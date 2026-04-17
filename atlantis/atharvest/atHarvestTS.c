/**
 *	\file atHarvestTS.c
 * 	\ingroup atHarvestLib
 *	\brief Functions to deal with imposed catch and discard data that is read in from time series files.
 *
 *	\author Beth Fulton
 *
 *	<b>Revisions</b>
 *
 *	29-06-2009 Bec Gorton
 *	Moved out of the management library into the new harvest module.
 *
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "atHarvest.h"

/********//**
 *	Reads a list of time series of catch or effort from an ascii file.
 *	The time series are specified as shown below.
 *	fishtsnn means ts followed by the integer box number time series is in nn
 *	(with the minimum number of digits needed).
 *	Here, S is a string (which must not contain whitespace),
 *	N is an integer and X, Y, Z and A are floating point numbers.
 *
 *	This routine depends on the existence of a global variable:
 *	verbose -   Sets level of messages printed
 *
 \begin{verbatim
 # Number of fisheries time series (supposed to be one per box)
 nfishts     N

 # Parameters for each fisheries time series
 # Point source/sink name
 fishtsnn.name    S

 # Location ( x y )
 fishtsnn.location X Y

 # Box_location
 fishtsnn.boxid B

 # Data - the next line is a time series definition
 # as used by my timeseries routines in sjwlib. The example
 # below assumes that the data is in an ascii or netCDF file.
 fishtsnn.data  filename
 \end{verbatim}
 *
 *	Input variables and parameters:
 *	@input name:		Name of ascii file containing list of fisheries time series
 *	@input tsname:		Name of times series (catch or effort) being read
 *	@input t_units:	Time units to be used for time series
 *	@input fishts:		Returned pointer to fisheries time series list
 *  @input nts:		Returned number of fisheries time series
 */
void Harvest_Read_Time_Series(char *folderPath, char *name, char *tsname, char *t_units, FisheryTimeSeries **ts, int *nts, int *typets) {
	FILE *fp;
	char keyn[BMSLEN];
	int nnts = 0;
	int typts = 0;
	FisheryTimeSeries *fts = NULL;
	int i = 0;

	/* Open the file */
	if ((fp = Open_Input_File(folderPath, name, "r")) == NULL)
		quit("read%sTimeSeries: Can't open %s%s\n", tsname, folderPath, name);

	/* Print a heading on stderr if verbose enough */
	if (verbose > 1)
        printf( "Reading %s Time Series\n", tsname);

	/* Get the number of time series */
	snprintf(keyn, sizeof(keyn), "n%sts", tsname);
	readkeyprm_i(fp, keyn, &nnts);

	/* Get the type of time series */
	snprintf(keyn, sizeof(keyn), "type%sts", tsname);
	readkeyprm_i(fp, keyn, &typts);

	/* Allocate memory for list of time series */
	if ((fts = (FisheryTimeSeries *) malloc((size_t)nnts * sizeof(FisheryTimeSeries))) == NULL)
		quit("Harvest readTimeSeries: Can't allocate memory for %s time series list\n", tsname);

	/* Read each fisheries time series input */
	for (i = 0; i < nnts; i++) {
		char key[MAXLINELEN];
		char buf[MAXLINELEN];

		/* Location */
		snprintf(key, sizeof(key), "%sts%d.location", tsname, i);
		readkeyprm_s(fp, key, buf);
		if (sscanf(buf, "%lf %lf %d", &fts[i].x, &fts[i].y, &fts[i].b) != 3)
			quit("Incorrect information specified for %s, need x y box format", key);

		/* Data */
		snprintf(key, sizeof(key), "%sts%d.data", tsname, i);

		fprintf(stderr, "Loading %s for box %d\n", key, fts[i].b);

		readkeyprm_s(fp, key, buf);
		tsRead(folderPath, buf, &fts[i].ts);
		if (verbose > 1)
			tsPrintInfo(&fts[i].ts, stderr);

		fprintf(stderr, "Loaded %s\n", fts[i].ts.df->name);

		/* Check data time units */
		if (strcmp(fts[i].ts.t_units, t_units) != 0) {

			warn("TS file %s time data units (%s) does not match the initial conditions units (%s). If this is correct then ignore but please check.\n",
					tsname, fts[i].ts.t_units,
					t_units);
			if (verbose > 1) {
				fprintf(stderr, "Converting %s time series time units\n", tsname);
				fprintf(stderr, "    Was %s\n", fts[i].ts.t_units);
				fprintf(stderr, "    Now %s\n", t_units);
			}
			tsNewTimeUnits(&fts[i].ts, t_units);
		}
	}

	/* Close the file */
	fclose(fp);

	/* Store pointer to list of point source/sinks */
	*ts = fts;
	*nts = nnts;
	*typets = typts;

	return;
}

/**
 *	Free Fishery TS
 */
void Harvest_Free_Time_Series(FisheryTimeSeries *fts, int nnts) {
	int i = 0;

	for (i = 0; i < nnts; i++) {
		fts[i].x = 0;
		fts[i].y = 0;
		fts[i].b = -1;
		tsFree(&fts[i].ts);
	}
	free(fts);
	return;
}

/**
 *	Routine to recalculate CatchTS_agedistrib and DiscardTS_agedistrib when
 *	age classes are exhausted or reintroduced.
 *
 *	Essentially compact the distribution onto top most age class with any
 *	individuals left. Reset discarding of this top age class to be fished
 *	to that of more preferable age class under original distribution
 *	(to reflect changing size preferences).
 **/
void Harvest_Recalc_Time_Series_Distrib(MSEBoxModel *bm, double **sptotden, int sp, int decdis, FILE *llogfp) {
	int ij, bij, ncum, do_debug, ngene;
	int fishery_id = 0;
	int flagimposecatch = (int) (FunctGroupArray[sp].speciesParams[flagimpose_id]);
	double cumtot, sumtotc, sumtotd, step1;

	if ((bm->which_check == sp) && (bm->debug == debug_fishing)) {
		do_debug = 1;
	} else
		do_debug = 0;

    //do_debug = 1;
    
	sumtotc = 0;
	sumtotd = 0;
	/* Contracting the distribution - so use current distribution and squeeze it top down */
	if (decdis) {
		cumtot = 0;
		ncum = 0;

		// Not using "* FunctGroupArray[sp].numGeneTypes" here as harvest code refers back to age classes only for this parameter 
		for (ij = (FunctGroupArray[sp].numCohorts) - 1; ij >= 0; ij--) {
			if (do_debug) {
				fprintf(llogfp, "Time: %e, %s-%d startcatchTS: %e ", bm->dayt, FunctGroupArray[sp].groupCode, ij, bm->CatchTS_agedistrib[fishery_id][sp][ij]);
			}

			step1 = 0.0;
			for(ngene = 0; ngene < FunctGroupArray[sp].numGeneTypes; ngene++){
				bij = (FunctGroupArray[sp].numGeneTypes * ij) + ngene;
				step1 += sptotden[sp][bij];
			}
			if (step1 < (bm->min_dens * bm->nbox * 2)) {
				cumtot += bm->CatchTS_agedistrib[fishery_id][sp][ij];
				bm->CatchTS_agedistrib[fishery_id][sp][ij] = 0;
				ncum++;
				bm->pSPcheck[sp][ij] = 1;
            } else if (bm->flagkeepZeroCatchTS) {
                if (bm->CatchTS_agedistribOrig[fishery_id][sp][ij] > 0) {
                    bm->CatchTS_agedistrib[fishery_id][sp][ij] += cumtot;
                } else {
                    bm->CatchTS_agedistrib[fishery_id][sp][ij] = bm->CatchTS_agedistribOrig[fishery_id][sp][ij];
                }
                if (bm->DiscardTS_agedistribOrig[fishery_id][sp][ij] > 0) {
                    bm->DiscardTS_agedistrib[fishery_id][sp][ij] = bm->DiscardTS_agedistribOrig[fishery_id][sp][ij + ncum];
                } else {
                    bm->DiscardTS_agedistrib[fishery_id][sp][ij] = bm->DiscardTS_agedistribOrig[fishery_id][sp][ij];
                }
                cumtot = 0;
                bm->pSPcheck[sp][ij] = 0;
            } else {
				bm->CatchTS_agedistrib[fishery_id][sp][ij] += cumtot;
				bm->DiscardTS_agedistrib[fishery_id][sp][ij] = bm->DiscardTS_agedistribOrig[fishery_id][sp][ij + ncum];
				cumtot = 0;
				bm->pSPcheck[sp][ij] = 0;
			}

			sumtotc += bm->CatchTS_agedistrib[fishery_id][sp][ij];
			sumtotd += bm->DiscardTS_agedistrib[fishery_id][sp][ij];

			if (do_debug) {
				fprintf(llogfp, "contracts to: %e (sumtotc: %e)\n", bm->CatchTS_agedistrib[fishery_id][sp][ij], sumtotc);
			}
		}
	} else {
		/* Expanding the distribution - so use original distribution and squeeze it top down in case
		 some ages still depleted
		 */
		cumtot = 0;
		ncum = 0;
		
		// Not using "* FunctGroupArray[sp].numGeneTypes" here as harvest code refers back to age classes only for this parameter 
		for (ij = (FunctGroupArray[sp].numCohorts) - 1; ij >= 0; ij--) {
			if (do_debug) {
				fprintf(llogfp, "Time: %e, %s-%d startcatchTS: %e ", bm->dayt, FunctGroupArray[sp].groupCode, ij, bm->CatchTS_agedistrib[fishery_id][sp][ij]);
			}

			step1 = 0.0;
			for(ngene = 0; ngene < FunctGroupArray[sp].numGeneTypes; ngene++){
				bij = (FunctGroupArray[sp].numGeneTypes * ij) + ngene;
				step1 += sptotden[sp][bij];
			}
			if ((step1 < (bm->min_dens * bm->nbox * 2)) || (bm->flagkeepZeroCatchTS && !bm->CatchTS_agedistribOrig[fishery_id][sp][ij])){
				cumtot += bm->CatchTS_agedistribOrig[fishery_id][sp][ij];
				bm->CatchTS_agedistrib[fishery_id][sp][ij] = 0;
				ncum++;
				bm->pSPcheck[sp][ij] = 1;
            } else {
				bm->CatchTS_agedistrib[fishery_id][sp][ij] += bm->CatchTS_agedistribOrig[fishery_id][sp][ij] + cumtot;
				bm->DiscardTS_agedistrib[fishery_id][sp][ij] = bm->DiscardTS_agedistribOrig[fishery_id][sp][ij + ncum];
				cumtot = 0;
				bm->pSPcheck[sp][ij] = 0;
			}

			sumtotc += bm->CatchTS_agedistrib[fishery_id][sp][ij];
			sumtotd += bm->DiscardTS_agedistrib[fishery_id][sp][ij];

			if (do_debug) {
				fprintf(llogfp, "expands to: %e (sumtotc: %e)\n", bm->CatchTS_agedistrib[fishery_id][sp][ij], sumtotc);
			}
		}
	}

	// Not using "* FunctGroupArray[sp].numGeneTypes" here as harvest code refers back to age classes only for this parameter 
	/* Normalise resulting distributions and determine desired size/age classes (for high grading) */
	for (ij = (FunctGroupArray[sp].numCohorts) - 1; ij >= 0; ij--) {
		bm->CatchTS_agedistrib[fishery_id][sp][ij] /= (sumtotc + small_num);
		bm->DiscardTS_agedistrib[fishery_id][sp][ij] /= (sumtotd + small_num);

		/* Do the for loop in descending order so last entry in desired_chrt_id is smallest acceptable */
		if (bm->CatchTS_agedistrib[fishery_id][sp][ij] > bm->highgrade_thresh) {
			bm->SP_FISHERYprms[sp][fishery_id][desired_chrt_id] = ij;
		}

		if (do_debug || (bm->CatchTS_agedistrib[fishery_id][sp][0] > 0)) {
			fprintf(llogfp, "Time: %e, %s-%d new_catchTSdistrib: %e\n", bm->dayt, FunctGroupArray[sp].groupCode, ij, bm->CatchTS_agedistrib[fishery_id][sp][ij]);
		}
	}

	/* Let all fisheries impacting the group know the new desired size for high grading */
	if (bm->flaghighgrading) {
		for (fishery_id = 1; fishery_id < bm->K_num_fisheries; fishery_id++) {
			bm->SP_FISHERYprms[sp][fishery_id][desired_chrt_id] = bm->SP_FISHERYprms[sp][0][desired_chrt_id];
		}
	}

	/* If for imposed catch assign to all other fisheries (not necessary if only for high grading) */
	/* FIX - if ever get fishery specific patterns then make above loop per fishery not one size fits all */
	if (flagimposecatch) {
		for (fishery_id = 1; fishery_id < bm->K_num_fisheries; fishery_id++) {
			// Not using "* FunctGroupArray[sp].numGeneTypes" here as harvest code refers back to age classes only for this parameter 
			for (ij = 0; ij < FunctGroupArray[sp].numCohorts; ij++) {
				bm->CatchTS_agedistrib[fishery_id][sp][ij] = bm->CatchTS_agedistrib[0][sp][ij];
				bm->DiscardTS_agedistrib[fishery_id][sp][ij] = bm->DiscardTS_agedistrib[0][sp][ij];
			}
		}
	}
	return;
}
