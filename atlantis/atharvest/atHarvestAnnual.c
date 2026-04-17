/**
 * 	\file atHarvestAnnual.c
 *	\ingroup atHarvestLib
 *
 *	\brief Harvest functions that are performed annually.
 *
 *	\author Beth Fulton
 *	Created:	5/10/2005
 *
 *
 *	Revisions:
 *	25-06-2009 Bec Gorton
 *	Split the management module into management, harvest and management implementation.
 *
 * 	28-10-2009 Bec Gorton
 * 	Removed references to the K_num_fished_sp and K_num_impacted_sp. Arrays that were this
 * 	size have been resized to K_num_tot_sp.
 * 	Also changed references to the arrays that have been moved into one of the modules.
 *
 */

/*  Open library routines */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "atHarvest.h"

/* Local function prototypes */
static void Report_Annual_Harvest(MSEBoxModel *bm, FILE *llogfp);

void Harvest_Annual_Calculations(MSEBoxModel *bm, FILE *llogfp) {

	int nf;

	Report_Annual_Harvest(bm, llogfp);

	/* See if management active */
	for (nf = 0; nf < bm->K_num_fisheries; nf++) {
		bm->FISHERYprms[nf][manage_on_id] = 0;
		if ((bm->dayt >= bm->FISHERYprms[nf][start_manage_id]) && (bm->dayt < bm->FISHERYprms[nf][end_manage_id])) {
			bm->FISHERYprms[nf][manage_on_id] = 1;
		}
	}

	/* Update fisheries performance measures */
	//Update_Harvest_Index_Values(bm, llogfp);

	return;
}

/**
 *	A routine to report the aggregate fisheries statistics to date - called from
 *	Check_Gape() in atecology.lib
 */
void Harvest_Report_Fisheries_Stats(MSEBoxModel *bm, FILE *llogfp) {
	int sp = 0;

	/* Report catch statistics for current year to date */
	for (sp = 0; sp < bm->K_num_tot_sp; sp++) {
		if (FunctGroupArray[sp].isImpacted == TRUE) {
			fprintf(llogfp, "Time: %e, %s catch %e t discards %e t\n",
					bm->dayt, FunctGroupArray[sp].groupCode,
					CatchSum[sp][tscocatch_id] * bm->X_CN * mg_2_tonne,
					CatchSum[sp][tscodiscard_id] * bm->X_CN * mg_2_tonne);
		}
	}
}

/**
 *
 *
 *
 */
void Report_Annual_Harvest(MSEBoxModel *bm, FILE *llogfp) {
	int b, sp, did_printout;

	if (verbose > 0)
		printf("Annual fisheries management\n");

	/* Report catches in log file */
	Harvest_Report_Fisheries_Stats(bm, llogfp);

	/* Report on extra catch and discards and reset for the new year */
	for (sp = 0; sp < bm->K_num_tot_sp; sp++) {
		if (FunctGroupArray[sp].isImpacted == TRUE) {
			for (b = 0; b < bm->nbox; b++) {
				did_printout = 0;
				if (bm->FCtsCarryOver[b][tscocatch_id][sp] > 0) {
					did_printout = 1;
					fprintf(llogfp, "Time: %e, box %d %s had %e t undercatch", bm->dayt, b, FunctGroupArray[sp].groupCode,
							bm->FCtsCarryOver[b][tscocatch_id][sp] * bm->cell_vol * bm->X_CN * mg_2_tonne);
				}
				if (bm->FCtsCarryOver[b][tscodiscard_id][sp] > 0) {
					if (did_printout)
						fprintf(llogfp, " and %e t underdiscard", bm->FCtsCarryOver[b][tscodiscard_id][sp] * bm->cell_vol * bm->X_CN * mg_2_tonne);
					else
						fprintf(llogfp, "Time: %e, box %d %s had %e t underdiscard", bm->dayt, b, FunctGroupArray[sp].groupCode,
								bm->FCtsCarryOver[b][tscodiscard_id][sp] * bm->cell_vol * bm->dt * bm->X_CN * mg_2_tonne);
					did_printout = 1;
				}
				if (did_printout) {
					fprintf(llogfp, "\n");
				}
				bm->FCtsCarryOver[b][tscocatch_id][sp] = 0;
				bm->FCtsCarryOver[b][tscodiscard_id][sp] = 0;
			}
			/* Report and reset global totals */
			if ((bm->FCtsCarryOver[bm->nbox][tscocatch_id][sp] > 0) || (bm->FCtsCarryOver[bm->nbox][tscodiscard_id][sp] > 0)) {
				fprintf(llogfp, "Time: %e, %s had %e t undercatch and %e t underdiscard\n", bm->dayt, FunctGroupArray[sp].groupCode,
						bm->FCtsCarryOver[bm->nbox][tscocatch_id][sp] * bm->X_CN * mg_2_tonne, bm->FCtsCarryOver[bm->nbox][tscodiscard_id][sp] * bm->dt
								* bm->X_CN * mg_2_tonne);
			}
			bm->FCtsCarryOver[bm->nbox][tscocatch_id][sp] = 0;
			bm->FCtsCarryOver[bm->nbox][tscodiscard_id][sp] = 0;
		}
	}

	if(bm->thisyear >= 0){
		/* Report annual catch statistics */
		Harvest_Report_Annual_Stats(bm, llogfp);
	}

	/* Reset annual catch statistics */
	bm->OldCatchReset = 0;
	for(sp=0; sp<bm->K_num_tot_sp; sp++){
		if(FunctGroupArray[sp].isImpacted == TRUE){
			if(!bm->bulkTAC || (FunctGroupArray[sp].speciesParams[tac_resetcount_id] >= FunctGroupArray[sp].speciesParams[tac_resetperiod_id])){
				OldCatchSum[sp][tscocatch_id] = 0;
				OldCatchSum[sp][tscodiscard_id] = 0;
				OldCatchSum[sp][tsactcatch_id] = 0;
				bm->OldCatchReset = 1;
			}
			OldCatchSum[sp][tscocatch_id] += CatchSum[sp][tscocatch_id];
			OldCatchSum[sp][tscodiscard_id] += CatchSum[sp][tscodiscard_id];
			OldCatchSum[sp][tsactcatch_id] += CatchSum[sp][tsactcatch_id];
			CatchSum[sp][tscocatch_id] = 0;
			CatchSum[sp][tscodiscard_id] = 0;
			CatchSum[sp][tsactcatch_id] = 0;
		}
	}

	/* Update year of run */
	bm->renewTrade++;
}

/**
 *	Annual fisheries reset
 */
void Harvest_Annual_Reset(MSEBoxModel *bm, FILE *llogfp) {
	int k, n, b, ij;
    //double totcumcatch = 0;

	/* Reset catch and cumulative statistics for the year */
	for (n = 0; n < bm->K_num_fisheries; n++) {

		for (b = 0; b < bm->nbox; b++) {
			bm->OldCumEffort[n][b] = bm->CumEffort[n][b];
			bm->CumEffort[n][b] = 0;
			bm->GhostEffort[b][n] = 0;
		}
		for (b = 0; b < K_num_harvest_indx; b++) {
			harvestindx[n][b] = 0;
		}
        
		for (k = 0; k < bm->K_num_tot_sp; k++) {
			if (FunctGroupArray[k].isImpacted == TRUE) {
				for (b = 0; b < bm->nbox; b++) {
					bm->CumDiscards[k][n][b] = 0;
					for (ij = 0; ij < bm->wcnz; ij++) {
                        /*
                        if( k == 17 ) {
                            totcumcatch = Harvest_Get_TotCumCatch(k, n, bm->thisyear);
                            fprintf(llogfp, "Time: %e %s %s in box %d-%d reset CumCatch of %e (ToCumCatch %e CatchSum: %e)\n", bm->dayt,
                                    FunctGroupArray[k].groupCode, FisheryArray[n].name, b, ij, bm->CumCatch[k][n][b][ij], totcumcatch, CatchSum[k][tscocatch_id]);
                        }
                        */

						bm->Catch[b][k][n][ij] = 0.0;
						bm->CumCatch[k][n][b][ij] = 0.0;
					}
                    
                    if ((FunctGroupArray[k].speciesParams[flag_id] == TRUE) && (FunctGroupArray[k].groupAgeType == AGE_STRUCTURED)){
                        for (ij = 0; ij < FunctGroupArray[k].numCohortsXnumGenes; ij++) {
                            FunctGroupArray[k].SizeNumCaught[ij][n][b] = 0.0;
                            FunctGroupArray[k].SizeNumDiscard[ij][n][b] = 0.0;
                        }
                    }

				}
				bm->TotCumDiscards[k][n] = 0.0;
				if(bm->OldCatchReset){
					bm->TotOldCumCatch[k][n] = 0; // If just do test above then will fail as will have new TAC setting in between which resets the counters
				}
				bm->TotOldCumCatch[k][n] += TotCumCatch[k][n][bm->thisyear];  // Could use TotCumCatch[k][n][bm->thisyear-1] instead of TotOldCumCatch, but then would have to add exception checks so keep TotOldCumCatch (at least for now)
				if(!bm->bulkTAC)
					bm->TotOldCumCatch[k][n] = 0.0; // As not summing over years, so don't count previous years in cumulative sums
			}

			if (FunctGroupArray[k].isTAC) {
				TotCumCatch[k][n][bm->thisyear+1] = 0.0;  // Ready for the new year (just to be sure)
				bm->TotCumRecCatch[k][n] = 0.0;
			}
            bm->TripCatch[k][n] = 0.0; // Reset here incase economic module not on, but still want the behaviour
            
		}
	}
	return;
}

