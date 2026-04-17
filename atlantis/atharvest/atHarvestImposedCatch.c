/**
 * \ingroup atHarvestLib
 * \file atHarvestImposedCatch.c
 * \brief Functions to deal with imposed catch and harvest.
 *
 *  \author Bec Gorton Created on: 25/01/2010
 *
 *
 *
 *
 *  <b>Revisions:</b>:
 *
 *  28-01-2010 Bec Gorton
 *  Moved the tsCatch and tsDiscard time series data out of the MSEBoxModel structure. This is now only defined in the harvest module.
 */

/*  Open library routines */
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <math.h>
#include <string.h>
#include <atHarvest.h>

/* Imposed Catch and Discard time series variables. */
int tsCatchwarned;

int ntsCatch; /**< Number of catch time series (should match one per box at most, but keep this int as check) */
int tsCatchtype; /**< Whether to use interpolated or last valid entries from time-series */
int *tscatchid; /**< Array matching boxmodel species ids to catch time series entry ids */
FisheryTimeSeries *tsCatch; /**< List of catch time series (one per box) */

int tsDiscardwarned;
int ntsDiscard; /**< Number of discard time series (should match one per box at most, but keep this int as check) */
int tsDiscardtype; /**< Whether to use interpolated or last valid entries from time-series */
int *tsdiscardid; /**< Array matching boxmodel species ids to discard time series entry ids */
FisheryTimeSeries *tsDiscard; /**< List of discard time series (one per box) */

//int first_data_done;

static double Get_Loaded_FC(MSEBoxModel *bm, int guildcase, int nf, int chrt, int stage, int boxkey_id, double vert_scale, double Biom, double *mpa_losses,
		int do_debug, FILE *llogfp);

/**
 * \brief Load the imposed catch time series data.
 *
 *
 *
 *
 */
int Load_Imposed_Catch(MSEBoxModel *bm, FILE *llogfp) {

	int nf, sp, b;
	int flagimposecatch;

	/* Initialise catch and effort timeseries - start with checking if needed */
	flagimposecatch = 0;
	for (nf = 0; nf < bm->K_num_fisheries; nf++) {
		for (sp = 0; sp < bm->K_num_tot_sp; sp++) {
			if (bm->SP_FISHERYprms[sp][nf][flagimposecatch_id] > 0)
				flagimposecatch = 1;
		}
	}

	/* Load catch time series */
	if (flagimposecatch) {
		tsCatchwarned = 0;
		Harvest_Read_Time_Series(bm->inputFolder, bm->forceIfname, "Catch", bm->t_units, &tsCatch, &ntsCatch, &tsCatchtype);

		if (ntsCatch) {
			FisheryTimeSeries *this_ts = &tsCatch[0];

			/* Assign species ids */
			for (sp = 0; sp < bm->K_num_tot_sp; sp++) {
				if (FunctGroupArray[sp].isFished == TRUE) {
					tscatchid[sp] = -1;
					for (b = 0; b < this_ts->ts.nv; b++) {
						if ((bm->debug == debug_fishing) || (bm->debug == debug_histfishing)) {
							fprintf(llogfp, "checking %s vs %s (sp = %d b = %d)\n", FunctGroupArray[sp].groupCode, this_ts->ts.varname[b], sp, b);
						}

						if (strcmp(FunctGroupArray[sp].groupCode, this_ts->ts.varname[b]) == 0) {
							tscatchid[sp] = b;
							if (bm->debug == debug_fishing) {
								fprintf(llogfp, "tscatchid for %s is %d\n", FunctGroupArray[sp].groupCode, tscatchid[sp]);
							}
							break;
						}
					}
					if (tscatchid[sp] < 0)
						quit("Did not find time series of catches for %s - check catch ts files contain it\n", FunctGroupArray[sp].groupCode);
				}
			}

			/* Assign box map ids */
			if (bm->flagimposeglobal == 1) {
				/* If a global entry then one entry for all */
				for (b = 0; b < bm->nbox; b++) {
					bm->BoxKeyMap[b][catchkey_id] = 0;
				}

				if (ntsCatch > 1)
					quit("flagimposeglobal = 1 assumes a single over all timeseries of catch being used, instead %d given\n", ntsCatch);

			} else {
				/* If explicit then have explicit IDs */
				for (b = 0; b < bm->nbox; b++) {
					for (sp = 0; sp < ntsCatch; sp++) {
						if (b == tsCatch[sp].b) {
							bm->BoxKeyMap[b][catchkey_id] = sp;
						}
					}
				}
			}
		} else
			quit("Expecting catch time series but didn't find any\n");
	} else {
		tsCatch = NULL;

		/* Set to -1 all round so that don't accidently try and use it even if no catch imposed */
		 for ( b=0; b<bm->nbox; b++ ){
			 bm->BoxKeyMap[b][catchkey_id] = -1;
		 }
	}

	return 0;
}

int Load_Imposed_Discards(MSEBoxModel *bm) {
	int sp, nf, b;

	int flagimposediscard;

	flagimposediscard = 0;
	for (nf = 0; nf < bm->K_num_fisheries; nf++) {
		for (sp = 0; sp < bm->K_num_tot_sp; sp++) {
			if (bm->SP_FISHERYprms[sp][nf][flagdiscard_id] == impose_discard)
				flagimposediscard = 1;
		}
	}

	/* Load discard time series */
	if (flagimposediscard) {
		tsDiscardwarned = 0;
		Harvest_Read_Time_Series(bm->inputFolder,bm->forceIfname, "Discard", bm->t_units, &tsDiscard, &ntsDiscard, &tsDiscardtype);

		if (ntsDiscard) {
			FisheryTimeSeries *this_ts = &tsDiscard[0];

			/* Assign species ids */
			for (sp = 0; sp < bm->K_num_tot_sp; sp++) {
				if (FunctGroupArray[sp].isFished == TRUE) {
					tsdiscardid[sp] = -1;
					for (b = 0; b < this_ts->ts.nv; b++) {
						if (strcmp(FunctGroupArray[sp].groupCode, this_ts->ts.varname[b]) == 0)
							tsdiscardid[sp] = b;
					}
					if (tsdiscardid[sp] < 0)
						quit("Did not find time series of discards for %s - check discard ts files contain it\n", FunctGroupArray[sp].groupCode);
				}
			}

			/* Assign box map ids */
			if (bm->flagimposeglobal == 1) {
				/* If a global entry then one entry for all */
				for (b = 0; b < bm->nbox; b++) {
					bm->BoxKeyMap[b][discardkey_id] = 0;
				}
			} else {
				/* If explicit then have explicit IDs */
				for (b = 0; b < bm->nbox; b++) {
					for (sp = 0; sp < ntsDiscard; sp++) {
						if (b == tsDiscard[sp].b) {
							bm->BoxKeyMap[b][discardkey_id] = sp;
						}
					}
				}
			}
		} else
			quit("Expecting discard time series but didn't find any\n");
	} else
		tsDiscard = NULL;

	return 0;
}

/**
 * \brief Get the imposed catch of this cohort of the guildcase functional group by the nf'th fishery.
 *
 * This is used when there is set of time series catch data.
 *
 *
 *
 */
double Get_Imposed_Catch(MSEBoxModel *bm, int guildcase, int chrt, int stage, int nf, int do_debug, double vert_scale, double Biom, int boxkey_id,
		double *mpa_losses, FILE *llogfp) {

	double loadFC;
	/* Assumes one time series per box - FIX generalise so checks for time series to use
	 so don't have to have dummy fields in unfished boxes */

	if (boxkey_id > -1) {

		loadFC = Get_Loaded_FC(bm, guildcase, nf, chrt, stage, boxkey_id, vert_scale, Biom, mpa_losses, do_debug, llogfp);

	} else {
		if (!tsCatchwarned) {
			fprintf(llogfp, "expecting imposed catch in box %d (and maybe others), but never got it - check catch timeseries definitions\n", bm->current_box);
			tsCatchwarned++;
		}
		loadFC = 0;
	}

	return loadFC;
}

double Get_Loaded_FC(MSEBoxModel *bm, int guildcase, int nf, int chrt, int stage, int boxkey_id, double vert_scale, double Biom, double *mpa_losses,
		int do_debug, FILE *llogfp) {

	FisheryTimeSeries *this_tsCatch = &tsCatch[boxkey_id];
	int ts_id;
	double age_scale_catch, active_scale, distrib_scale, tsEvaled = 0.0, mgmt_scale, report_scale, mpa_scale, mpa_infringe;
	double ts_dtscale = 86400.0 / bm->dt; // As timeseries generating code assumes timestep of 86400 timestep
	double loadFC;
	int flagimposecatch = (int) (bm->SP_FISHERYprms[guildcase][nf][flagimposecatch_id]);
	int i, flagmanage, flagfcmpa;
	int basechrt = (int)floor(chrt / FunctGroupArray[guildcase].numGeneTypes);

	flagmanage = (int) (bm->FISHERYprms[nf][flagmanage_id]);
	ts_id = tscatchid[guildcase];
    
	/* Load size of catch to impose - currently assume one dump for all
	 fisheries,	FIX - will need to allow for this to be fishery specific
	 (by doing (i*j + i) kind of index allocation) */
    if (FunctGroupArray[guildcase].groupAgeType == AGE_STRUCTURED || FunctGroupArray[guildcase].groupAgeType == AGE_STRUCTURED_BIOMASS) {
		age_scale_catch = bm->CatchTS_agedistrib[nf][guildcase][basechrt];
        
        /*
        if (!chrt) {
            fprintf(llogfp, "Time: %e, %s-%d (basechrt %d) age_scale_catch: %e (Orig: %e)\n", bm->dayt, FunctGroupArray[guildcase].groupCode, chrt, basechrt, age_scale_catch, bm->CatchTS_agedistribOrig[nf][guildcase][basechrt]);
        }
        */
         
    } else {
		age_scale_catch = 1.0;
        
        /*
        if (!chrt) {
            fprintf(llogfp, "Time: %e, %s-%d (basechrt %d) age_scale_catch: %e as not age structured\n", bm->dayt, FunctGroupArray[guildcase].groupCode, chrt, basechrt, age_scale_catch);
        }
         */
    }

	/* If always active (vs only diurnally active) then adjust accordingly */
	if (bm->FISHERYprms[nf][fisheriesflagactive_id] > 1)
		active_scale = bm->dt / 86400.0;
    else {
        active_scale = 1.0;
        if (bm->newmonth && (bm->dt < 43200.0 ))
            warn("Imposing catch but dt %e doesn't match assumptions of 12-24 hours per timestep\n", bm->dt);
    }

	/* Correct for any mismatch in model and tsgenerator dt */
	active_scale *= ts_dtscale;

	switch (flagimposecatch) {
	case no_impose:
		distrib_scale = 0.0;
		tsEvaled = 0.0;
		break;
	case spatial_impose:
	case stock_impose:
	case adjacent_impose:
		/* Spatially explicit time series loaded so nothing to do here
		 (if stock-based case any short fall may be siphoned off rest of stock
		 in carryover section)
		 */
		distrib_scale = 1.0;

		/* Load catch - assumed to be in mgs-1 */
		if (tsCatchtype == interpolate_id)
			tsEvaled = tsEval(&this_tsCatch->ts, ts_id, bm->t);
		else
			tsEvaled = tsEvalEx(&this_tsCatch->ts, ts_id, bm->t);

		break;
	case global_impose:
		/* Global time series loaded and then corrected by current spatial distribution */
		distrib_scale = bm->targetspbiom[guildcase][bm->current_box] / (bm->totbiom[guildcase] + small_num);

		/* Load catch - assumed to be in mgs-1 */
		tsEvaled = 0;
        this_tsCatch = &tsCatch[boxkey_id];
        ts_id = tscatchid[guildcase];
		if (tsCatchtype == interpolate_id) {
			tsEvaled += tsEval(&this_tsCatch->ts, ts_id, bm->t);
		} else {
			tsEvaled += tsEvalEx(&this_tsCatch->ts, ts_id, bm->t);
		}
            
        /**
        if (do_debug) {
            fprintf(llogfp, "Time: %e, box%d, boxkey: %d, tsid: %d, tsEval: %e, guildcase= %s\n", bm->dayt, i, boxkey_id, ts_id, tsEvaled, FunctGroupArray[guildcase].groupCode);
        }
        **/

		break;
	default:
		tsEvaled = 0.0;
		distrib_scale = 0.0;	/* Deal with compiler warnings */
		quit("No such case (%d) for flagimposecatch for %s - reset to a number 0 to 4\n", flagimposecatch, FunctGroupArray[guildcase].groupCode);
		break;
	}

	/* Correct for simple adaptive management (if appropriate) */
	if (flagmanage == stock_adapt_mgmt)
		mgmt_scale = effort_scale[nf][target_tac_id];
	else
		mgmt_scale = 1.0;

	/* Correction for under reporting */
	report_scale = bm->SP_FISHERYprms[guildcase][nf][FC_reportscale_id];

	/* Correct for presence of mpas */
	flagfcmpa = (int) (bm->FISHERYprms[nf][flagmpa_id]);
	if (flagfcmpa)
		mpa_scale = bm->MPA[bm->current_box][nf];
	else
		mpa_scale = 1.0;
    
    /* Allow for infringement */
    if (bm->flaginfringe) {
        mpa_infringe = bm->FISHERYprms[nf][infringe_id];
        if (mpa_infringe > mpa_scale)
            mpa_scale = mpa_infringe;
    }

	loadFC = mgmt_scale * report_scale * distrib_scale * active_scale * vert_scale * age_scale_catch * tsEvaled / (bm->cell_vol + small_num);

	/* Find any losses (displaced catch) due to MPAs */
	*mpa_losses = (1 - mpa_scale) * loadFC;
	loadFC *= mpa_scale;

	/*
	 Store actual potential catch for records
	 */
	CatchSum[guildcase][tsactcatch_id] += loadFC * bm->cell_vol * bm->dt;

	/**
	if (do_debug) {
    //if (!chrt && loadFC) {
		fprintf(
				llogfp,
                "Time: %e, %s-%d (basechrt %d tscatchid: %d) box%d-%d (%d), biom: %e, histcatch: %e (totscale: %e, tsEval: %e, mpa_scale: %e, active_scale: %e, vert_scale: %e (vert_vdistrib[%d-%d]: %e), age_scale_catch: %e (Orig: %e), distrib_scale: %e, mgmt_scale: %e, report_scale: %e, ts_dtscale: %e, bm->cell_vol: %e)\n",
				bm->dayt, FunctGroupArray[guildcase].groupCode, chrt, basechrt, tscatchid[guildcase], bm->current_box, bm->current_layer, boxkey_id, Biom, loadFC, (loadFC
						* bm->cell_vol / (tsEvaled + small_num)), tsEvaled, mpa_scale, active_scale, vert_scale, bm->current_box, bm->current_layer,
				bm->boxes[bm->current_box].vert_vdistrib[guildcase][stage][bm->current_layer], age_scale_catch, bm->CatchTS_agedistribOrig[nf][guildcase][basechrt], distrib_scale, mgmt_scale, report_scale,
				ts_dtscale, bm->cell_vol);
	}
	**/

	if (!first_data_done && (tsEvaled > 0)) {
		fprintf(llogfp, "First hist catch data used at %e in box%d-%d on %s\n", bm->dayt, bm->current_box, bm->current_layer, FunctGroupArray[guildcase].groupCode);
		first_data_done = 1;
	}

	return loadFC;
}
