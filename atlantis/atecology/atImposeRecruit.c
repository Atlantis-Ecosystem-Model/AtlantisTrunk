/**
 * \ingroup atEcologyLib
 * \file atImposeRecruit.c
 * \brief Functions to deal with imposed recruitment from multiple sources rather than a single box
 *
 *  \author Beth Fulton Created on: 08/05/2014
 *
 *
 *  <b>Revisions:</b>:
 *
 *  08-05-2014 Beth Fulton
 *  Based on how Bec did time series for imposed catches
 */

/*  Open library routines */
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <math.h>
#include <string.h>
#include <atecology.h>

/* Imposed recruit time series variables. */
int tsRecruitswarned;
int ntsRecruits; /**< Number of recruit time series (should match one per box at most, but keep this int as check) */
int tsRecruitstype; /**< Whether to use interpolated or last valid entries from time-series */
int *tsRecruitsid; /**< Array matching boxmodel species ids to recruits time series entry ids */
FisheryTimeSeries *tsRecruits; /**< List of recruit time series (one per box) - use a FisheryTimeSeries as want a list of time series not just one time series */

static double Get_Loaded_MultRecruits(MSEBoxModel *bm, int guildcase, int boxkey_id, int do_debug, FILE *llogfp);

/**
 * \brief Load the imposed recruitment time series data.
 *
 * Note use FisheryTimeSeries as that is a list of time series
 */
int Load_Imposed_Recruits(MSEBoxModel *bm, FILE *llogfp) {

	int sp, b;
	int flagimposerec;
    int recruit_sp, stocking_sp;

	/* Initialise recruitment timeseries - start with checking if needed */
	flagimposerec = 0;
    for (sp = 0; sp < bm->K_num_tot_sp; sp++) {
        recruit_sp = (int) (FunctGroupArray[sp].speciesParams[flagrecruit_id]);
        stocking_sp = (int) (FunctGroupArray[sp].speciesParams[flagstocking_id]);
        if ((recruit_sp == multiple_ts_recruit) || (stocking_sp)){
            flagimposerec = 1;
		}
	}

	/* Load catch time series */
	if (flagimposerec) {
		tsRecruitswarned = 0;
		Harvest_Read_Time_Series(bm->inputFolder, bm->forceIfname, "MultRec", bm->t_units, &tsRecruits, &ntsRecruits, &tsRecruitstype); // Reuse this as populates a list of time series

		if (ntsRecruits) {
			FisheryTimeSeries *this_ts = &tsRecruits[0];

			/* Assign species ids */
			for (sp = 0; sp < bm->K_num_tot_sp; sp++) {
				if ((FunctGroupArray[sp].groupAgeType == AGE_STRUCTURED) || (FunctGroupArray[sp].groupAgeType == AGE_STRUCTURED_BIOMASS)) {
					tsRecruitsid[sp] = -1;
					for (b = 0; b < this_ts->ts.nv; b++) {
						if (strcmp(FunctGroupArray[sp].groupCode, this_ts->ts.varname[b]) == 0) {
							tsRecruitsid[sp] = b;
							break;
						}
					}
					if (tsRecruitsid[sp] < 0)
						quit("Did not find time series of recruits for %s - check MultRec ts files contain it\n", FunctGroupArray[sp].groupCode);
				}
			}

			/* Assign box map ids */
            for (b = 0; b < bm->nbox; b++) {
                for (sp = 0; sp < ntsRecruits; sp++) {
                    if (b == tsRecruits[sp].b) {
                        bm->BoxKeyMap[b][rects_key_id] = sp;
                    }
                }
            }
		} else
			quit("Expecting recruit time series but didn't find any\n");
	} else {
		tsRecruits = NULL;

		/* Set to -1 all round so that don't accidently try and use it even if no catch imposed */
		 for ( b=0; b<bm->nbox; b++ ){
			 bm->BoxKeyMap[b][rects_key_id] = -1;
		 }
	}

	return 0;
}

/**
 * \brief Get the imposed recruits
 *
 * This is used when there is set of time series catch data.
 *
 *
 */
double Get_Imposed_MultRecruits(MSEBoxModel *bm, int guildcase, int do_debug, int boxkey_id, FILE *llogfp) {
	double loadFC;
	/* Assumes one time series per box - FIX generalise so checks for time series to use
	 so don't have to have dummy fields in unfished boxes */

	if (boxkey_id > -1) {

		loadFC = Get_Loaded_MultRecruits(bm, guildcase, boxkey_id, do_debug, llogfp);

	} else {
		if (!tsRecruitswarned) {
			fprintf(llogfp, "expecting imposed recruits in box %d (and maybe others), but never got it - check catch timeseries definitions\n", bm->current_box);
			tsRecruitswarned++;
		}
		loadFC = 0;
	}

	return loadFC;
}

double Get_Loaded_MultRecruits(MSEBoxModel *bm, int guildcase, int boxkey_id, int do_debug, FILE *llogfp) {
	FisheryTimeSeries *this_tsRecruits = &tsRecruits[boxkey_id];
	int ts_id = tsRecruitsid[guildcase];
    double tsEvaled = 0.0;
    
    /* Load recruits */
    if (tsRecruitstype == interpolate_id)
        tsEvaled = tsEval(&this_tsRecruits->ts, ts_id, bm->t);
    else
        tsEvaled = tsEvalEx(&this_tsRecruits->ts, ts_id, bm->t);

    return tsEvaled;
}

/**
 * Free up the Imposed Recruits Time Series.
 * 
 * 
 */
void Free_Imposed_Recruits(MSEBoxModel *bm){

	if(tsRecruits){
		Harvest_Free_Time_Series(tsRecruits, ntsRecruits);


	}
}
