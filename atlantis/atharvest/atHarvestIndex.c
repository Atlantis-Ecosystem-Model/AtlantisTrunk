/**
 \ingroup atHarvestLib
 \file	atHarvestIndex.c
 \brief Functions to calculate the harvest performance indices. These are written to the harvestIndex output file
 with the management performance measures.
 \author Beth Fulton

 <b> Revisions:</b>

 01/12/2008 Bec Gorton
 Moved the management and fisheries performance measures output file into the
 atManageModule structure so that it can be correctly closed at the end of the
 model run.

 25-06-2009 Bec Gorton
 Split the management module into management, harvest and management implementation.

 02-11-2009 Bec Gorton
 Removed more references to K_num_fished and K_num_impacted.
 **************/

/*  Open library routines */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "atHarvest.h"
#include "atManageLib.h"

/* Variable definitions */
char **harvestindxNAME; /**< name of fisheries performance measures */
double **harvestindx; /**< fisheries performance measures */

/**
 * \brief Write out the names of the harvest index parameters to the provided outputFile.
 *
 */
void Harvest_Init_Index_Output(MSEBoxModel *bm, FILE *outputFile) {

	int index;

	/* Harvest performance measures (don't want to list harvest_indx update checker) */
	for (index = 0; index < K_num_harvest_indx - 1; index++) {
		fprintf(outputFile, " %s", harvestindxNAME[index]);
	}
}

/**
 * \brief Write out the values of each of the harvest index value for the given fishery to the
 * given outputFile.
 */
void Harvest_Write_Harvest_Index(MSEBoxModel *bm, int nf, FILE *outputFile) {
	int index;

	/* Harvest performance measures (don't want to list harvest_indx update checker)*/
	for (index = 0; index < K_num_harvest_indx - 1; index++) {
		fprintf(outputFile, " %e", harvestindx[nf][index]);
	}
}

void Harvest_Set_Harvest_Index(MSEBoxModel *bm, int nf, int index, double value) {
	harvestindx[nf][index] = value;
}
double Harvest_Get_Harvest_Index(MSEBoxModel *bm, int nf, int index) {
	return harvestindx[nf][index];
}

/**
 *
 * Routines to set names of harvest and management performance measures
 *
 *	\brief This sets up names of fisheries performance indices
 */
void Set_Harvest_Index_Names(MSEBoxModel *bm) {
    int paramlen = 20;
    
	snprintf(harvestindxNAME[avg_sze_id], paramlen, "%s", "AvgCatchSize");
	snprintf(harvestindxNAME[tot_land_id], paramlen, "%s", "TotLanding");
	snprintf(harvestindxNAME[tot_num_caught_id], paramlen, "%s", "TotNumCaught");
	snprintf(harvestindxNAME[tot_trash_id], paramlen, "%s", "TotDiscards");
	snprintf(harvestindxNAME[gearchange_id], paramlen, "%s", "GearChange");
	snprintf(harvestindxNAME[discardchange_id], paramlen, "%s", "DiscardChange");
	snprintf(harvestindxNAME[tepkill_id], paramlen, "%s", "TEPkills");
	snprintf(harvestindxNAME[habkill_id], paramlen, "%s", "HabImpact");
	snprintf(harvestindxNAME[tot_dayfishing_id], paramlen, "%s", "TotEffort");
	snprintf(harvestindxNAME[overallcpue_id], paramlen, "%s", "TotCPUE");
	snprintf(harvestindxNAME[sociallike_id], paramlen, "%s", "SocialAccept");
	snprintf(harvestindxNAME[realconflict_id], paramlen, "%s", "GearConflict");
	snprintf(harvestindxNAME[sp_comp_id], paramlen, "%s", "SpComp_Evenness");
	snprintf(harvestindxNAME[checkdone_id], paramlen, "%s", "UpdateCheck");

	return;
}

/**
 * \brief Routine to update harvest performance measures
 *
 */
void Update_Harvest_Index_Values(MSEBoxModel *bm, FILE *llogfp) {
	int b, sp, nf, is_cute, is_shark, is_hab, ij;
	double totconflict, catch_ratio, sp_catch, max_ratio;
	double dtscale = 86400.0 / bm->dt;
	int managementCost;

	if (verbose > 0)
		printf("Updating harvest performance measures\n");

	for (nf = 0; nf < bm->K_num_fisheries; nf++) {
		/* Initialisations */
		harvestindx[nf][tot_land_id] = 0;
		harvestindx[nf][tot_trash_id] = 0;
		harvestindx[nf][tepkill_id] = 0;
		harvestindx[nf][habkill_id] = 0;

		/* Average size of catch */
		harvestindx[nf][avg_sze_id] /= (harvestindx[nf][tot_num_caught_id] + small_num);

		/* Total effort */
		harvestindx[nf][tot_dayfishing_id] = 0;
		for (b = 0; b < bm->nbox; b++) {
			harvestindx[nf][tot_dayfishing_id] += bm->CumEffort[nf][b];
		}

		/* Gear and discarding changes */
		harvestindx[nf][gearchange_id] /= (365.0 * dtscale);
		harvestindx[nf][discardchange_id] /= (365.0 * dtscale);

	}

	for (sp = 0; sp < bm->K_num_tot_sp; sp++) {
		if(FunctGroupArray[sp].isFished == TRUE || FunctGroupArray[sp].isImpacted == TRUE){
			/* Identify is species is one the public cares about */
			if (FunctGroupArray[sp].groupType == BIRD || FunctGroupArray[sp].groupType == MAMMAL)
				is_cute = 1;
			else
				is_cute = 0;

			if (FunctGroupArray[sp].groupType == SHARK)
				is_shark = 1;
			else
				is_shark = 0;

			//TODO: Replace with a simple check of the invert type
			if (FunctGroupArray[sp].speciesParams[isbiogenhab_id])
				is_hab = 1;
			else
				is_hab = 0;

			for (nf = 0; nf < bm->K_num_fisheries; nf++) {
				for (b = 0; b < bm->nbox; b++) {
					/* Total discards */
					harvestindx[nf][tot_trash_id] += bm->CumDiscards[sp][nf][b] * bm->X_CN * mg_2_tonne;
                    
                    /* Social acceptance contribution */
                    if (is_cute) {
                        if (FunctGroupArray[sp].isImpacted == TRUE)
                            harvestindx[nf][tepkill_id] += bm->tepcoefft * bm->CumDiscards[sp][nf][b] * bm->X_CN * mg_2_tonne;
                    }
                    if (is_shark) {
                        if (FunctGroupArray[sp].isImpacted == TRUE)
                            harvestindx[nf][tepkill_id] += bm->sharkcoefft * bm->CumDiscards[sp][nf][b] * bm->X_CN * mg_2_tonne;
                    }
                    if (is_hab) {
                        if (FunctGroupArray[sp].isImpacted == TRUE)
                            harvestindx[nf][habkill_id] += bm->CumDiscards[sp][nf][b] * bm->X_CN * mg_2_tonne;
                    }

					for (ij = 0; ij < bm->boxes[b].nz; ij++) {
						/* Total landings */
						harvestindx[nf][tot_land_id] += bm->CumCatch[sp][nf][b][ij] * bm->X_CN * mg_2_tonne;

						/* Social acceptance contribution */
						if (is_cute) {
							if (FunctGroupArray[sp].isFished == TRUE)
								harvestindx[nf][tepkill_id] += bm->tepcoefft * bm->CumCatch[sp][nf][b][ij] * bm->X_CN * mg_2_tonne;
						}
						if (is_shark) {
							if (FunctGroupArray[sp].isFished == TRUE)
								harvestindx[nf][tepkill_id] += bm->sharkcoefft * bm->CumCatch[sp][nf][b][ij] * bm->X_CN * mg_2_tonne;
						}
						if (is_hab) {
							if (FunctGroupArray[sp].isFished == TRUE)
								harvestindx[nf][habkill_id] += bm->CumCatch[sp][nf][b][ij] * bm->X_CN * mg_2_tonne;
						}
					}
				}
			}
		}
	}

	/* Overall gear conflict */
	totconflict = 0;
	for (b = 0; b < bm->nbox; b++) {
		totconflict += bm->boxes[b].fishstat[bm->conflict_id];
	}

	/* Species catch composition evenness - actually largest proportion of catch due to any one group */
	for (nf = 0; nf < bm->K_num_fisheries; nf++) {

		/* Total CPUE */
		harvestindx[nf][overallcpue_id] = harvestindx[nf][tot_land_id] / (harvestindx[nf][tot_dayfishing_id] + small_num);

		/* Final social acceptance */
		harvestindx[nf][sociallike_id] = 1.0 / (harvestindx[nf][tepkill_id] + bm->habcoefft * harvestindx[nf][habkill_id] + small_num);

		/* Gear conflict */
		harvestindx[nf][realconflict_id] = bm->FISHERYprms[nf][relconflict_id] * totconflict;

		managementCost = 0;
		max_ratio = 0;

		/* Species catch composition evenness - actually largest proportion of catch due to any one group */
		for (sp = 0; sp < bm->K_num_tot_sp; sp++) {
			if (FunctGroupArray[sp].isFished == TRUE) {
				sp_catch = 0;
				for (b = 0; b < bm->nbox; b++) {
					for (ij = 0; ij < bm->boxes[b].nz; ij++) {
						sp_catch += bm->CumCatch[sp][nf][b][ij] * bm->X_CN * mg_2_tonne;
					}
				}
				catch_ratio = sp_catch / (harvestindx[nf][tot_land_id] + small_num);
				if (catch_ratio > max_ratio)
					max_ratio = catch_ratio;

				/* Calculate management costs */
				if ((TotCumCatch[sp][nf][bm->thisyear] > 0) && (bm->SP_FISHERYprms[sp][nf][TAC_id] < no_quota))
					managementCost++;
			}
		}
		harvestindx[nf][sp_comp_id] = max_ratio;
		/* Set the management performance indicator */
		Manage_Set_Manage_Index(bm, nf, mgmtcost_id, managementCost);
	}
	return;
}

