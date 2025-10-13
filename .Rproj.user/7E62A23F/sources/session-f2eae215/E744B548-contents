/**
 *
 *  Contaminants within Atlantis.
 *
 *  Load the contaminants using the 'use_force_tracers' option in the forcing file.
 *  Atlantis will check to make sure that this option is set to true and values are provided for these tracers
 *  or it will quit.
 *
 *	Values of contaminants in the environment due to the following:
 *	 - concentration in the water etc - due to values read in from forcing netcdf files.
 *	 - dispersal, which comes for free with Hydro routines already
 *
 *
 *	Values in groups change due to the following:
 *
 *	- Update in groups due to contact with contaminants.
 *	- Transmission through food web
 *
 *	Impact of contaminants on groups
 *  - chronic effects on metabolic rates (as ill), which we can do in the same way as for Tcorr and pHcorr etc (i.e Q10 part of code).
 *  - mortality
 *
 *
 *
 */
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>
#include <sjwlib.h>
#include "atecology.h"

static FILE *contaminantContactFile;
/**
 * Free up the contaminant structure.
 *
 *
 *
 */
void Free_Contaminants(MSEBoxModel *bm) {
	int cIndex;

    printf("Freeing contaminant params\n");

	for (cIndex = 0; cIndex < bm->num_contaminants; cIndex++) {

        free4d(bm->contaminantStructure[cIndex]->expose_time);

        free(bm->contaminantStructure[cIndex]->sp_uptake_rate);
        free(bm->contaminantStructure[cIndex]->sp_uptake_option);
        free(bm->contaminantStructure[cIndex]->sp_decay_half_life);
        free2d(bm->contaminantStructure[cIndex]->sp_amount_decayed);
        free3d(bm->contaminantStructure[cIndex]->sp_uptake);
        free3d(bm->contaminantStructure[cIndex]->sp_transfer);
        free4d(bm->contaminantStructure[cIndex]->sp_transfer_global);

        free(bm->contaminantStructure[cIndex]->interact_coefft);
        free(bm->contaminantStructure[cIndex]->sp_LD50);
        free(bm->contaminantStructure[cIndex]->sp_LD100);
        free(bm->contaminantStructure[cIndex]->sp_LDChronic);
        free(bm->contaminantStructure[cIndex]->sp_LDslope);
        free(bm->contaminantStructure[cIndex]->sp_EC50);
        free(bm->contaminantStructure[cIndex]->sp_ECslope);
        free(bm->contaminantStructure[cIndex]->sp_EC50_r);
        free(bm->contaminantStructure[cIndex]->sp_ECslope_r);
        free2d(bm->contaminantStructure[cIndex]->gainedGlobal);
        free(bm->contaminantStructure[cIndex]->sp_maxConcentration);
        
        free(bm->contaminantStructure[cIndex]->gained);
        free4d(bm->contaminantStructure[cIndex]->sp_point);
        
        free1d(bm->contaminantStructure[cIndex]->sp_GrowthThresh);
        free1d(bm->contaminantStructure[cIndex]->sp_ReprodThresh);
        free2d(bm->contaminantStructure[cIndex]->gainedGlobal);
        free(bm->contaminantStructure[cIndex]->sp_maxConcentration);

        free(bm->contaminantStructure[cIndex]->gained);
        free4d(bm->contaminantStructure[cIndex]->sp_point);

        free(bm->contaminantStructure[cIndex]->sp_GrowthEffect);
        free1d(bm->contaminantStructure[cIndex]->sp_MoveEffect);
        free(bm->contaminantStructure[cIndex]->sp_ReprodEffect);
        free(bm->contaminantStructure[cIndex]->sp_ContamScalar);
        free(bm->contaminantStructure[cIndex]->sp_maternal_transfer);
        free(bm->contaminantStructure[cIndex]->sp_suckling_transfer);
        printf("About to free bm->instantDoseMortality\n");
        free(bm->contaminantStructure[cIndex]->sp_instantDoseMortality);
        free4d(bm->contaminantStructure[cIndex]->sp_maxDoseToDate);
        free(bm->contaminantStructure[cIndex]->sp_maxLethalConc);
        free(bm->contaminantStructure[cIndex]->sp_maxChronicConc);

        free(bm->contaminantStructure[cIndex]->sp_TimeToLD50);
        free(bm->contaminantStructure[cIndex]->sp_Cx);
        free(bm->contaminantStructure[cIndex]->sp_Cy);

        //free2d(bm->contaminantStructure[cIndex]->speciesMort);
	}

    printf("About to free bm->contaminantStructure\n");
	free(bm->contaminantStructure);

    return;
}

/**
 * Allocate the contaminant structure.
 *
 *
 *
 */
void Allocate_Contaiminants(MSEBoxModel *bm) {

    int cIndex;
    int max_chrt = bm->K_num_max_genetypes * bm->K_num_max_cohort;

	for (cIndex = 0; cIndex < bm->num_contaminants; cIndex++) {
		bm->contaminantStructure[cIndex]->sp_uptake_rate = Util_Alloc_Init_1D_Double(bm->K_num_tot_sp, 0.0);
		bm->contaminantStructure[cIndex]->sp_uptake_option = Util_Alloc_Init_1D_Int(bm->K_num_tot_sp, 0);
		bm->contaminantStructure[cIndex]->sp_uptake = Util_Alloc_Init_3D_Double(bm->num_active_habitats, max_chrt, bm->K_num_tot_sp, 0.0);
		bm->contaminantStructure[cIndex]->sp_transfer = Util_Alloc_Init_3D_Double(bm->num_active_habitats, max_chrt, bm->K_num_tot_sp, 0.0);
		bm->contaminantStructure[cIndex]->sp_transfer_global = Util_Alloc_Init_4D_Double(bm->num_active_habitats, bm->num_active_habitats, max_chrt, bm->K_num_tot_sp, 0.0);

        bm->contaminantStructure[cIndex]->expose_time =  Util_Alloc_Init_4D_Double((bm->wcnz+bm->sednz), bm->nbox, max_chrt, bm->K_num_tot_sp, 0.0);
		bm->contaminantStructure[cIndex]->sp_amount_decayed = Util_Alloc_Init_2D_Double(max_chrt, bm->K_num_tot_sp, 0.0);
        bm->contaminantStructure[cIndex]->sp_decay_half_life = Util_Alloc_Init_1D_Double(bm->K_num_tot_sp, 0.0);

        bm->contaminantStructure[cIndex]->sp_ContamScalar = Util_Alloc_Init_1D_Double(bm->K_num_tot_sp, 0.0);

        bm->contaminantStructure[cIndex]->sp_maternal_transfer = Util_Alloc_Init_1D_Double(bm->K_num_tot_sp, 0.0);
        bm->contaminantStructure[cIndex]->sp_suckling_transfer = Util_Alloc_Init_1D_Double(bm->K_num_tot_sp, 0.0);

        bm->contaminantStructure[cIndex]->sp_EC50 = Util_Alloc_Init_1D_Double(bm->K_num_tot_sp, 0.0);
        bm->contaminantStructure[cIndex]->sp_ECslope = Util_Alloc_Init_1D_Double(bm->K_num_tot_sp, 0.0);
        
        bm->contaminantStructure[cIndex]->sp_EC50_r = Util_Alloc_Init_1D_Double(bm->K_num_tot_sp, 0.0);
        bm->contaminantStructure[cIndex]->sp_ECslope_r = Util_Alloc_Init_1D_Double(bm->K_num_tot_sp, 0.0);

        bm->contaminantStructure[cIndex]->interact_coefft = Util_Alloc_Init_1D_Double(bm->num_contaminants, 0.0);

		bm->contaminantStructure[cIndex]->sp_LD50 = Util_Alloc_Init_1D_Double(bm->K_num_tot_sp, 0.0);
		bm->contaminantStructure[cIndex]->sp_LD100 = Util_Alloc_Init_1D_Double(bm->K_num_tot_sp, 0.0);
        bm->contaminantStructure[cIndex]->sp_LDChronic = Util_Alloc_Init_1D_Double(bm->K_num_tot_sp, 0.0);
        bm->contaminantStructure[cIndex]->sp_LDslope = Util_Alloc_Init_1D_Double(bm->K_num_tot_sp, 0.0);

		bm->contaminantStructure[cIndex]->gained = Util_Alloc_Init_1D_Double(bm->num_active_habitats, 0.0);

        bm->contaminantStructure[cIndex]->sp_maxConcentration = Util_Alloc_Init_1D_Double(bm->K_num_tot_sp, 0.0);
        bm->contaminantStructure[cIndex]->sp_maxChronicConc = Util_Alloc_Init_1D_Double(bm->K_num_tot_sp, 0.0);
        bm->contaminantStructure[cIndex]->sp_maxLethalConc = Util_Alloc_Init_1D_Double(bm->K_num_tot_sp, 0.0);

		bm->contaminantStructure[cIndex]->gainedGlobal = Util_Alloc_Init_2D_Double(bm->num_active_habitats, bm->num_active_habitats, 0.0);
		bm->contaminantStructure[cIndex]->sp_point = Util_Alloc_Init_4D_Double((bm->wcnz+bm->sednz), bm->nbox, max_chrt, bm->K_num_tot_sp, 0.0);

		bm->contaminantStructure[cIndex]->sp_GrowthThresh = Util_Alloc_Init_1D_Double(bm->K_num_tot_sp, 0.0);
		bm->contaminantStructure[cIndex]->sp_ReprodThresh = Util_Alloc_Init_1D_Double(bm->K_num_tot_sp, 0.0);
    bm->contaminantStructure[cIndex]->sp_GrowthEffect = Util_Alloc_Init_1D_Double(bm->K_num_tot_sp, 0.0);

        bm->contaminantStructure[cIndex]->sp_instantDoseMortality = Util_Alloc_Init_1D_Double(bm->K_num_tot_sp, 0);
		bm->contaminantStructure[cIndex]->sp_maxDoseToDate = Util_Alloc_Init_4D_Double((bm->wcnz+bm->sednz), bm->nbox, max_chrt, bm->K_num_tot_sp, 0.0);
		bm->contaminantStructure[cIndex]->sp_MoveEffect = Util_Alloc_Init_1D_Double(bm->K_num_tot_sp, 0.0);
        bm->contaminantStructure[cIndex]->sp_ReprodEffect = Util_Alloc_Init_1D_Double(bm->K_num_tot_sp, 0.0);

		bm->contaminantStructure[cIndex]->sp_TimeToLD50 = Util_Alloc_Init_1D_Double(bm->K_num_tot_sp, 0.0);
		bm->contaminantStructure[cIndex]->sp_Cx = Util_Alloc_Init_1D_Double(bm->K_num_tot_sp, 0.0);
		bm->contaminantStructure[cIndex]->sp_Cy = Util_Alloc_Init_1D_Double(bm->K_num_tot_sp, 0.0);
		bm->contaminantStructure[cIndex]->sp_L = Util_Alloc_Init_1D_Double(bm->K_num_tot_sp, 0.0);
		bm->contaminantStructure[cIndex]->sp_A = Util_Alloc_Init_1D_Double(bm->K_num_tot_sp, 0.0);
		bm->contaminantStructure[cIndex]->sp_B = Util_Alloc_Init_1D_Double(bm->K_num_tot_sp, 0.0);
		bm->contaminantStructure[cIndex]->sp_L_r = Util_Alloc_Init_1D_Double(bm->K_num_tot_sp, 0.0);
		bm->contaminantStructure[cIndex]->sp_A_r = Util_Alloc_Init_1D_Double(bm->K_num_tot_sp, 0.0);
		bm->contaminantStructure[cIndex]->sp_B_r = Util_Alloc_Init_1D_Double(bm->K_num_tot_sp, 0.0);

        bm->contaminantStructure[cIndex]->sp_avoid = Util_Alloc_Init_1D_Double(bm->K_num_tot_sp, 0.0);
        bm->contaminantStructure[cIndex]->sp_K_avoid = Util_Alloc_Init_1D_Double(bm->K_num_tot_sp, 0.0);

		//bm->contaminantStructure[cIndex]->speciesMort = Util_Alloc_Init_2D_Double(3, bm->K_num_tot_sp, 0.0);

	}
	Init_Contaminants(bm);
}
/**
 * Initialise the contaminants.
 *
 *
 */
void Init_Contaminants(MSEBoxModel *bm) {

	int found = FALSE;
	int cIndex, tracerIndex;

	/* Check that the netcdf files are provided for the contaminant tracers */

	if (bm->use_forceTracers) {

		for (cIndex = 0; cIndex < bm->num_contaminants; cIndex++) {
			/* Check each forcing tracers */
			for (tracerIndex = 0; tracerIndex < bm->numForceTracers; tracerIndex++) {
				if (strcmp(bm->forceTracerInput[tracerIndex].variableName, bm->contaminantStructure[cIndex]->contaminant_name) == 0) {
					found = TRUE;
				}
			}

			if (found == FALSE) {
				quit("You have defined a contaminant tracer '%s' but you have provided no forcing files for this tracer. See the wiki for more information. \n",
						bm->contaminantStructure[cIndex]->contaminant_name);

			}
		}
	} else {
		quit("You have defined some contaminant tracers but you have provided no forcing files. You need to provide forcing data for each contaminant tracer. See the wiki for more information. \n");
	}
}

/**************************************************************************************************************************************************************
 * Functions associated with the changes in contaminant levels within the water column or the sediment.
 ***************************************************************************************************************************************************************/

/*
 * Move to avoid contaminated waters
 *
 *
 */

double Avoid_Contaminants(MSEBoxModel *bm, int groupIndex, int cohort, int box, int layer) {
    double numScalar = 1.0;
    double contamEffect, cLevel, sp_avoid, K_const_sp, step1;
    int pid, cIndex;

    for (cIndex = 0; cIndex < bm->num_contaminants; cIndex++) {
        contamEffect = 1.0;
        pid = bm->contaminantStructure[cIndex]->contaminant_tracer;
        cLevel = bm->boxes[box].tr[layer][pid];
        sp_avoid = bm->contaminantStructure[cIndex]->sp_avoid[groupIndex];
        K_const_sp = bm->contaminantStructure[cIndex]->sp_K_avoid[groupIndex];
        step1 = K_const_sp * exp(sp_avoid - cLevel) / (K_const_sp + (exp(sp_avoid - cLevel) - 1.0));

        switch (bm->flag_contamMove) {
            case not_sensitive:  // Nothing to do
                break;
            case sensitive_heaviside_window:
                if (cLevel > sp_avoid) {
                    contamEffect = 0.0;
                }
                break;
            case sensitive_biologistic_window: // Gaussian shape
                contamEffect = step1 / K_const_sp;
                if(contamEffect > 1.0)
                    contamEffect = 1.0;
                if(contamEffect < 0.0)
                    contamEffect = 0.0;
                break;
            case sensitive_gradual_side: // Flat-top version
                if ( cLevel > sp_avoid ) {
                    contamEffect = step1;
                }
                if(contamEffect > 1.0)
                    contamEffect = 1.0;
                if(contamEffect < 0.0)
                    contamEffect = 0.0;
                break;
            default:
                break;

        }

        numScalar *= contamEffect;
    }


    return numScalar;
}

/*
 * Move and so change proportion contaminanted per cell
 *
 *
 */

void Move_Vert_Contaminated(MSEBoxModel *bm, int sp, int cohort, double ****this_currentden) {
    int ij, k, cIndex, cid, pid, clayer, next_box, nextij;
    double num_possible, new_num_in_box;
    double **diffden, **num_contam, ***localcontam, ***num_contam_moved;
    int den = FunctGroupArray[sp].NumsTracers[cohort];
    
    if(verbose)
        printf("Doing Move_Vert_Contaminated for %s\n", FunctGroupArray[sp].groupCode);

    diffden = Util_Alloc_Init_2D_Double(bm->wcnz, bm->nbox, 0.0);
    num_contam = Util_Alloc_Init_2D_Double(bm->wcnz, bm->nbox, 0.0);
    localcontam = Util_Alloc_Init_3D_Double(bm->num_contaminants, bm->wcnz, bm->nbox, 0.0);
    num_contam_moved = Util_Alloc_Init_3D_Double(bm->num_contaminants, bm->wcnz, bm->nbox, 0.0);

    for (cIndex = 0; cIndex < bm->num_contaminants; cIndex++) {
        cid = bm->contaminantStructure[cIndex]->contaminant_tracer;
        pid = FunctGroupArray[sp].contamPropTracers[cohort][cIndex];
        
        for (ij = 0; ij < bm->nbox; ij++) {
            if (bm->boxes[ij].type != BOUNDARY) {
                for (k = 0; k < bm->boxes[ij].nz; k++) {
                    // Change in numbers
                    diffden[ij][k] = this_currentden[sp][cohort][k][ij] - bm->boxes[ij].tr[k][den];

                    //fprintf(bm->logFile, "Time: %e box%d-%d %s-%d for %s has diffden: %e\n", bm->dayt, ij, k, FunctGroupArray[sp].groupCode, cohort, bm->contaminantStructure[cIndex]->contaminant_name, diffden[ij][k]);

                    // Number of contaminanted that must have moved
                    localcontam[ij][k][cIndex] = bm->boxes[ij].tr[k][cid];
                    if (diffden[ij][k] < 0.0) {
                        num_contam_moved[ij][k][cIndex] = floor(bm->boxes[bm->current_box].tr[bm->current_layer][pid] * -1.0 * diffden[ij][k]);
                    }
                }
            }
        }

        /* Now reallocate */
        for (ij = 0; ij < bm->nbox; ij++) {
            if (bm->boxes[ij].type != BOUNDARY) {
                for (k = 0; k < bm->boxes[ij].nz; k++) {
                    // Move contaminated individuals
                    if (num_contam_moved[ij][k][cIndex] < 1.0) {
                        continue; //Nothing to do as nothing moved
                    }

                    /* Individuals have moved away - so move them to nearest box with new additional individuals */
                    // Do layers in same box
                    for (clayer = 0; clayer < bm->wcnz; clayer++) {
                        if ( clayer == k )
                            continue;

                        if (diffden[ij][clayer] > 0.0) {
                            num_possible = min(num_contam_moved[ij][k][cIndex], diffden[ij][clayer]);

                            // New contam level - average of what is there and what moved in
                            bm->boxes[ij].tr[clayer][cid] = (bm->boxes[ij].tr[clayer][cid] * bm->boxes[ij].tr[clayer][pid] * this_currentden[sp][cohort][clayer][ij] + num_possible * localcontam[ij][k][cIndex]) / (num_possible + bm->boxes[ij].tr[clayer][pid] * this_currentden[sp][cohort][clayer][ij] + small_num);

                            // Adjust local proportion
                            new_num_in_box = bm->boxes[ij].tr[clayer][pid] * this_currentden[sp][cohort][clayer][ij] + num_possible;
                            bm->boxes[ij].tr[clayer][pid] = new_num_in_box / (bm->boxes[ij].tr[k][den] + small_num);

                            //fprintf(bm->logFile, "Time %e box%d-%d %s %d %s has after move prop: %e with new_num_in_box %e den: %e\n", bm->dayt, ij, clayer, FunctGroupArray[sp].groupCode, cohort, bm->contaminantStructure[cIndex]->contaminant_name, bm->boxes[ij].tr[clayer][pid], new_num_in_box, bm->boxes[ij].tr[k][den]);

                            // Sanity check
                            if (bm->boxes[ij].tr[clayer][pid] < 0.0) {
                                bm->boxes[ij].tr[clayer][pid] = 0.0;
                            }
                            if (bm->boxes[ij].tr[clayer][pid] > 1.0) {
                                bm->boxes[ij].tr[clayer][pid] = 1.0;
                            }

                            // Update the counters - both of source and sink
                            num_contam_moved[ij][k][cIndex] -= num_possible;
                            diffden[ij][clayer] -= num_possible;
                        }

                    }

                    // Other boxes
                    nextij = 0;
                    while ((num_contam_moved[ij][k][cIndex] > 1.0) && (nextij < bm->nbox)) {
                        next_box = bm->boxes[ij].distID[nextij];

                        if(next_box != ij) {
                            for (clayer = 0; clayer < bm->wcnz; clayer++) {
                                if (diffden[next_box][clayer] > 0.0) {
                                    //Space for entrants
                                    num_possible = min(num_contam_moved[ij][k][cIndex], diffden[next_box][clayer]);

                                    // New contam level - average of what is there and what moved in
                                    bm->boxes[next_box].tr[clayer][cid] = (bm->boxes[next_box].tr[clayer][cid] * bm->boxes[next_box].tr[clayer][pid] * this_currentden[sp][cohort][clayer][next_box] + num_possible * localcontam[ij][k][cIndex]) / (num_possible + bm->boxes[next_box].tr[clayer][pid] * this_currentden[sp][cohort][clayer][next_box] + small_num);

                                    // Adjust local proportion
                                    new_num_in_box = bm->boxes[next_box].tr[clayer][pid] * this_currentden[sp][cohort][clayer][next_box] + num_possible;
                                    bm->boxes[next_box].tr[clayer][pid] =  new_num_in_box / (bm->boxes[next_box].tr[clayer][den] + small_num);

                                    //fprintf(bm->logFile, "Time %e next_box%d-%d %s %d %s has after move prop: %e with  new_num_in_box %e den: %e\n", bm->dayt, next_box, clayer, FunctGroupArray[sp].groupCode, cohort, bm->contaminantStructure[cIndex]->contaminant_name, bm->boxes[next_box].tr[clayer][pid], new_num_in_box, bm->boxes[next_box].tr[clayer][den]);


                                    // Sanity check
                                    if (bm->boxes[next_box].tr[clayer][pid] < 0.0) {
                                        bm->boxes[next_box].tr[clayer][pid] = 0.0;
                                    }
                                    if (bm->boxes[next_box].tr[clayer][pid] > 1.0) {
                                        bm->boxes[next_box].tr[clayer][pid] = 1.0;
                                    }

                                    // Update the counters - both of source and sink
                                    num_contam_moved[ij][k][cIndex] -= num_possible;
                                    diffden[next_box][clayer] -= num_possible;
                                }
                            }
                        }
                        nextij++;
                    }
                }
            }
        }
    }
    
    free2d(diffden);
    free2d(num_contam);
    free3d(localcontam);
    free3d(num_contam_moved);

    return;
}

/*
 * Call all the routines for changing contaminant levels. Do it here in one go so not doing multiple unnecessary loops
 *
 *
 */

void Change_Contaminant_Levels(MSEBoxModel *bm, BoxLayerValues *boxLayerInfo, HABITAT_TYPES habitat, double dtsz, int offset) {
    int cIndex;
    double cLevel;
    double *tracerArray = getTracerArray(boxLayerInfo, habitat);

    for (cIndex = 0; cIndex < bm->num_contaminants; cIndex++) {
        /* Grab the level in the water column or the sediment */
        cLevel = tracerArray[bm->contaminantStructure[cIndex]->contaminant_tracer];

        Calculate_Species_Contaminant_Decay(bm, boxLayerInfo, habitat, dtsz, cIndex, cLevel);
        Species_Contaminant_Uptake(bm, boxLayerInfo, habitat, dtsz, cIndex);
        Degrade_Contaminants(bm, boxLayerInfo, habitat, dtsz, cIndex, cLevel);
    }
    Calculate_Contaminants_Flux(bm, boxLayerInfo, habitat);

    return;
}

/**
 * Degrade the contaiminants in the water colummn and the sediment.
 *
 *
 */
int Degrade_Contaminants(MSEBoxModel *bm, BoxLayerValues *boxLayerInfo, HABITAT_TYPES habitat, double dtsz, int cIndex, double cLevel) {
	double newValue;

	newValue = cLevel * pow(0.5, dtsz / bm->contaminantStructure[cIndex]->half_life);
    bm->contaminantStructure[cIndex]->amount_decayed = (cLevel - newValue) / dtsz;

	/**
    if(bm->contaminantStructure[cIndex]->amount_decayed){
        fprintf(bm->logFile, "Box %d, layer %d, amount_decayed  = %e, cLevel= %e, habitat= %d\n", bm->current_box, bm->current_layer, bm->contaminantStructure[cIndex]->amount_decayed, cLevel, habitat);
    }
    **/

    return 0;
}

/**
 * Calculate the flux in the given habitat. This is called by Calculate_Contaminants_Flux().
 *
 *
 */
static void Calculate_Containinant_Flux_Habitat(MSEBoxModel *bm, BoxLayerValues *boxLayerInfo, double *fluxArray, HABITAT_TYPES habitat, int add){

	int cIndex, groupIndex, cohort;
	double flux, uptakeSum = 0, cLevel = 0, amt_after_decay;
    double *tracerArray = getTracerArray(boxLayerInfo, habitat);

	for (cIndex = 0; cIndex < bm->num_contaminants; cIndex++) {

        cLevel = tracerArray[bm->contaminantStructure[cIndex]->contaminant_tracer];
		uptakeSum = 0;

		/* loss of flux is sum of decay and all the uptake groups */
		for (groupIndex = 0; groupIndex < bm->K_num_tot_sp; groupIndex++) {
			for (cohort = 0; cohort < FunctGroupArray[groupIndex].numCohortsXnumGenes; cohort++) {
				if (FunctGroupArray[groupIndex].speciesParams[flag_id] == TRUE) {
                    uptakeSum += bm->contaminantStructure[cIndex]->sp_uptake[groupIndex][cohort][habitat];
					//uptakeSum += (bm->contaminantStructure[cIndex]->sp_uptake[groupIndex][cohort][habitat] * bm->dtsz_stored); // This line included as rates of uptake can be high enough it can out strip total standing stock once get total flux so need to account for total flux here

                    /**
                    if (uptakeSum > 0) {
                        fprintf(bm->logFile, "Calculate_Containinant_Flux_Habitat - Time: %e %s flux in Box %d, layer %d, habitat == %d, uptakeSum: %e sp_uptake[%s-%d][hab %d]: %e dtsz_stored: %e\n", bm->dayt, bm->contaminantStructure[cIndex]->contaminant_name, bm->current_box, bm->current_layer, habitat, uptakeSum, FunctGroupArray[groupIndex].groupCode, cohort, habitat, bm->contaminantStructure[cIndex]->sp_uptake[groupIndex][cohort][habitat], bm->dtsz_stored);
                    }
                    **/
				}
			}
		}

        
        /**
        if (uptakeSum > 0) {
            fprintf(bm->logFile, "Calculate_Containinant_Flux_Habitat - Time: %e %s flux in Box %d, layer %d, habitat == %d, uptakeSum: %e cLevel: %e\n", bm->dayt, bm->contaminantStructure[cIndex]->contaminant_name, bm->current_box, bm->current_layer, habitat, uptakeSum, cLevel);
        }
        **/

        
        /* Check whether uptake exceeds presence of material and then pro-rata uptake if necessary */
        if ((uptakeSum + bm->contaminantStructure[cIndex]->amount_decayed) * bm->dtsz_stored > cLevel) {
            
            amt_after_decay = (cLevel / bm->dtsz_stored) - bm->contaminantStructure[cIndex]->amount_decayed;  // was just clevel but Raisha time adjusted it for unit consistency
            
            for (groupIndex = 0; groupIndex < bm->K_num_tot_sp; groupIndex++) {
                for (cohort = 0; cohort < FunctGroupArray[groupIndex].numCohortsXnumGenes; cohort++) {
                    if (FunctGroupArray[groupIndex].speciesParams[flag_id] == TRUE) {
                        if(!cLevel) {
                            bm->contaminantStructure[cIndex]->sp_uptake[groupIndex][cohort][habitat] = 0.0;
                        } else {
                            bm->contaminantStructure[cIndex]->sp_uptake[groupIndex][cohort][habitat] *= amt_after_decay / uptakeSum;
                            
                           //fprintf(bm->logFile, "Calculate_Containinant_Flux_Habitat - Time: %e %s flux in Box %d, layer %d, habitat == %d, now has sp_uptake: %e\n", bm->dayt, bm->contaminantStructure[cIndex]->contaminant_name, bm->current_box, bm->current_layer, habitat, bm->contaminantStructure[cIndex]->sp_uptake[groupIndex][cohort][habitat]);

                        }
                    }
                }
            }
            uptakeSum = amt_after_decay;
        }
        
        
		flux = bm->contaminantStructure[cIndex]->gained[habitat] - (bm->contaminantStructure[cIndex]->amount_decayed + uptakeSum);

		if(add)
			fluxArray[bm->contaminantStructure[cIndex]->contaminant_tracer] += flux;
		else
			fluxArray[bm->contaminantStructure[cIndex]->contaminant_tracer] = flux;

		/**
        if(flux != 0) {
			fprintf(bm->logFile, "Calculate_Containinant_Flux_Habitat - Time: %e %s flux in Box %d, layer %d, habitat == %d, case: %d, flux = %e, uptakeSum = %e, gained = %e, amount_decays = %e dtsz_stored: %e\n", bm->dayt, bm->contaminantStructure[cIndex]->contaminant_name, bm->current_box, bm->current_layer, habitat, add, fluxArray[bm->contaminantStructure[cIndex]->contaminant_tracer], uptakeSum, bm->contaminantStructure[cIndex]->gained[habitat], bm->contaminantStructure[cIndex]->amount_decayed, bm->dtsz_stored);
        }
        **/

		bm->contaminantStructure[cIndex]->gained[habitat] = 0;

	}
}

/**
 * Calculate the flux of each contaminant in the current layer and the flux in each group.
 *
 *	For Epi layers we calculate the flux in both the water column and the sediment layer as normal.
 *
 */
int Calculate_Contaminants_Flux(MSEBoxModel *bm, BoxLayerValues *boxLayerInfo, HABITAT_TYPES habitatType){

	double *groupFluxArray = NULL;
	double flux = 0;
	int cIndex, groupIndex, cohort;
    //int pid;

	if (verbose > 1){
	  printf("Calculate_Contaminants_Flux\n");
	}
	switch(habitatType){
	case WC:
		Calculate_Containinant_Flux_Habitat(bm, boxLayerInfo, boxLayerInfo->localWCFlux, WC, 0);
		groupFluxArray = boxLayerInfo->localWCFlux;
		break;
	case SED:
		Calculate_Containinant_Flux_Habitat(bm, boxLayerInfo, boxLayerInfo->localSEDFlux, SED, 0);
		groupFluxArray = boxLayerInfo->localSEDFlux;

		break;
	case EPIFAUNA:
		Calculate_Containinant_Flux_Habitat(bm, boxLayerInfo, boxLayerInfo->localWCFlux, WC, 1);
		Calculate_Containinant_Flux_Habitat(bm, boxLayerInfo, boxLayerInfo->localSEDFlux, SED, 1);
		groupFluxArray = boxLayerInfo->localEPIFlux;
		break;
	case ICE_BASED:
		quit("Calculate_Contaminants_Flux not yet handling ICE\n");
		break;
	case LAND_BASED:
		quit("Calculate_Contaminants_Flux not yet handling Land\n");
		break;
    case MIXED:
        quit("How did we get here as should come through a primary habitat\n");
        break;
    default:
        quit("Got to default case with no entry in Contaminats_Flux");
        break;
	}

	/* Now do each group */
    for (groupIndex = 0; groupIndex < bm->K_num_tot_sp; groupIndex++) {
        if (FunctGroupArray[groupIndex].speciesParams[flag_id] == TRUE && FunctGroupArray[groupIndex].habitatCoeffs[habitatType] > 0) {
            if ((FunctGroupArray[groupIndex].isOncePerDt == FALSE || (it_count == 1 && FunctGroupArray[groupIndex].isOncePerDt == TRUE))) {
                for (cohort = 0; cohort < FunctGroupArray[groupIndex].numCohortsXnumGenes; cohort++) {
                    switch (FunctGroupArray[groupIndex].groupAgeType) {
                        case AGE_STRUCTURED:
                            FunctGroupArray[groupIndex].dead[cohort] += FunctGroupArray[groupIndex].contaminantSpMort[cohort];
                            break;
                        case AGE_STRUCTURED_BIOMASS:
                        case BIOMASS:
                             FunctGroupArray[groupIndex].mortality[cohort] += FunctGroupArray[groupIndex].contaminantSpMort[cohort];
                            break;
                    }
                    for (cIndex = 0; cIndex < bm->num_contaminants; cIndex++) {
						flux = bm->contaminantStructure[cIndex]->sp_uptake[groupIndex][cohort][habitatType] +
								bm->contaminantStructure[cIndex]->sp_transfer[groupIndex][cohort][habitatType] -
								bm->contaminantStructure[cIndex]->sp_amount_decayed[groupIndex][cohort];

                        groupFluxArray[FunctGroupArray[groupIndex].contaminantTracers[cohort][cIndex]] = flux;
                        
                        /**/
						if (isnan(flux)) {
							fprintf(bm->logFile, "Calculate_Contaminants_Flux - Group %s, cohort %d, in box %d, layer %d, contaiminant %s flux is nan (uptake = %e, transfer = %e,  amt_decayed: %e mort: %e)\n",
									FunctGroupArray[groupIndex].groupCode, cohort, bm->current_box, bm->current_layer,
									bm->contaminantStructure[cIndex]->contaminant_name, bm->contaminantStructure[cIndex]->sp_uptake[groupIndex][cohort][habitatType],
									bm->contaminantStructure[cIndex]->sp_transfer[groupIndex][cohort][habitatType], bm->contaminantStructure[cIndex]->sp_amount_decayed[groupIndex][cohort], FunctGroupArray[groupIndex].contaminantSpMort[cohort]);
							fprintf(stderr, "Calculate_Contaminants_Flux - Group %s, cohort %d, in box %d, layer %d, contaiminant %s flux is nan (uptake = %e, transfer = %e, amt_decayed: %e mort: %e)\n",
									FunctGroupArray[groupIndex].groupCode, cohort, bm->current_box, bm->current_layer,
									bm->contaminantStructure[cIndex]->contaminant_name, bm->contaminantStructure[cIndex]->sp_uptake[groupIndex][cohort][habitatType],
									bm->contaminantStructure[cIndex]->sp_transfer[groupIndex][cohort][habitatType], bm->contaminantStructure[cIndex]->sp_amount_decayed[groupIndex][cohort], FunctGroupArray[groupIndex].contaminantSpMort[cohort]);
							quit("Flux is nan");
						}

                        if (!(_finite(flux))) {
                            fprintf(bm->logFile, "Calculate_Contaminants_Flux - Group %s, cohort %d, in box %d, layer %d, contaiminant %s flux is inf (uptake = %e, transfer = %e,  amt_decayed: %e mort: %e)\n",
                                    FunctGroupArray[groupIndex].groupCode, cohort, bm->current_box, bm->current_layer,
                                    bm->contaminantStructure[cIndex]->contaminant_name, bm->contaminantStructure[cIndex]->sp_uptake[groupIndex][cohort][habitatType],
                                    bm->contaminantStructure[cIndex]->sp_transfer[groupIndex][cohort][habitatType], bm->contaminantStructure[cIndex]->sp_amount_decayed[groupIndex][cohort], FunctGroupArray[groupIndex].contaminantSpMort[cohort]);
                            fprintf(stderr, "Calculate_Contaminants_Flux - Group %s, cohort %d, in box %d, layer %d, contaiminant %s flux is inf (uptake = %e, transfer = %e, amt_decayed: %e mort: %e)\n",
                                    FunctGroupArray[groupIndex].groupCode, cohort, bm->current_box, bm->current_layer,
                                    bm->contaminantStructure[cIndex]->contaminant_name, bm->contaminantStructure[cIndex]->sp_uptake[groupIndex][cohort][habitatType],
                                    bm->contaminantStructure[cIndex]->sp_transfer[groupIndex][cohort][habitatType], bm->contaminantStructure[cIndex]->sp_amount_decayed[groupIndex][cohort], FunctGroupArray[groupIndex].contaminantSpMort[cohort]);
                            quit("Flux is inf");
                        }

                        /**/
                        
						/**
                        if(flux != 0.0){
                            fprintf(bm->logFile, "Calculate_Contaminants_Flux - Time %e box %d-%d %s-%d for %s flux %e uptake: %e Transfer: %e amt_decayed: %e mort: %e\n", bm->dayt, bm->current_box, bm->current_layer, FunctGroupArray[groupIndex].groupCode, cohort, bm->contaminantStructure[cIndex]->contaminant_name, groupFluxArray[FunctGroupArray[groupIndex].contaminantTracers[cohort][cIndex]],  bm->contaminantStructure[cIndex]->sp_uptake[groupIndex][cohort][habitatType], bm->contaminantStructure[cIndex]->sp_transfer[groupIndex][cohort][habitatType], bm->contaminantStructure[cIndex]->sp_amount_decayed[groupIndex][cohort], FunctGroupArray[groupIndex].contaminantSpMort[cohort]);
                        }
						**/
					}

				}
			}
		}
	}
    
    /*
    pid = FunctGroupArray[8].contamPropTracers[3][0];
    
    fprintf(bm->logFile, "Time: %e box%d-%d running - test propContam %s-%d for %s in box%d-%d: %e\n", bm->dayt, bm->current_box, bm->current_layer, FunctGroupArray[8].groupCode, 3, bm->contaminantStructure[0]->contaminant_name, 3, 2, bm->boxes[3].tr[2][pid]);
     printf("Time: %e box%d-%d running - test propContam %s-%d for %s in box%d-%d: %e\n", bm->dayt, bm->current_box, bm->current_layer, FunctGroupArray[8].groupCode, 3, bm->contaminantStructure[0]->contaminant_name, 3, 2, bm->boxes[3].tr[2][pid]);
     */

	return 0;
}

/**
 * Contaminants are released from groups back into the pools.
 *
 *
 */
int Gain_Contaminants(MSEBoxModel *bm, BoxLayerValues *boxLayerInfo, HABITAT_TYPES globalHabitat, HABITAT_TYPES habitat, int species, int cohort,
		double amountLost, double ***spSPinfo, double initialBiomass) {

	int cIndex;
	//double *tracerArray = getTracerArray(boxLayerInfo, globalHabitat);
    double *tracerArray;
	double cGroupLevel, transfer, totalBiomass, propLost = 1.0;
	int isGlobal = (FunctGroupArray[species].diagTol == 2 && it_count == 1);

    if(!amountLost)
        return 0;

    if(!bm->flag_dissolved_pollutants)
        return 0;

    if(FunctGroupArray[species].habitatType == EPIFAUNA){
      tracerArray = getTracerArray(boxLayerInfo, EPIFAUNA);
    } else {
      tracerArray = getTracerArray(boxLayerInfo, globalHabitat);
    }

	/* Work out the proportion eaten */
	if (FunctGroupArray[species].groupAgeType == AGE_STRUCTURED) {
		totalBiomass = (spSPinfo[species][cohort][SN_id] + spSPinfo[species][cohort][RN_id]) * spSPinfo[species][cohort][DEN_id] / bm->cell_vol;
		propLost = amountLost / (totalBiomass + small_num);
	} else {
		//totalBiomass = tracerArray[FunctGroupArray[species].totNTracers[cohort]];

        if (!initialBiomass)
            return 0;

        totalBiomass = initialBiomass;
		propLost = amountLost / (totalBiomass + small_num);
	}

    /**/
    if (propLost > 1.0) {
        fprintf(bm->logFile, "Gain_Contaminants prop > 1 Time: %e %s-%d box %d-%d propLost: %e amountLost: %e, totalBiomass: %e\n", bm->dayt, FunctGroupArray[species].groupCode, cohort, bm->current_box, bm->current_layer, propLost, amountLost, totalBiomass);
        quit("propLost > 1.0\n");
    }
    /**/

	for (cIndex = 0; cIndex < bm->num_contaminants; cIndex++) {
		/* The current concentration in the group */
		cGroupLevel = tracerArray[FunctGroupArray[species].contaminantTracers[cohort][cIndex]];

		transfer = cGroupLevel * propLost * bm->contaminantStructure[cIndex]->dissolv_coefft / (small_num + bm->dtsz_stored);  // Need time step correction so flux makes sense

		bm->contaminantStructure[cIndex]->gained[habitat] += transfer;
        bm->contaminantStructure[cIndex]->sp_transfer[species][cohort][habitat] -= transfer;

		/**/
        if(transfer > 0.0){
            fprintf(bm->logFile, "Gain_Contam transfer > 0 -- Time: %e %s-%d in box %d-%d, habitat %d, transfer = %e propLost: %e dissolv_coefft: %e cGroupLevel: %e dtsz_stored: %e\n", bm->dayt, FunctGroupArray[species].groupCode, cohort, bm->current_box, bm->current_layer, habitat, transfer, propLost, bm->contaminantStructure[cIndex]->dissolv_coefft, cGroupLevel, bm->dtsz_stored);
        }
        /**/
        if(!(_finite(transfer))) {
			quit("Time: %e %s-%d in box %d-%d, habitat %d, transfer = %e propLost: %e dissolv_coefft: %e cGroupLevel: %e dtsz_stored: %e\n", bm->dayt, FunctGroupArray[species].groupCode, cohort, bm->current_box, bm->current_layer, habitat, transfer, propLost, bm->contaminantStructure[cIndex]->dissolv_coefft, cGroupLevel, bm->dtsz_stored);
		}
        /**/

		if (isGlobal == TRUE) {
			bm->contaminantStructure[cIndex]->gainedGlobal[globalHabitat][habitat] += transfer;
		}
	}

	return 0;
}

/**************************************************************************************************************************************************************
 * Functions associated with a change in contaminant levels in a group.
 ***************************************************************************************************************************************************************/

/**
 * When a species is exposed to a contaminant there is an uptake.
 * The value is stored for the habitat. This contact can occur in all actives groups not just primary producers.
 *
 */
int Species_Contaminant_Uptake(MSEBoxModel *bm, BoxLayerValues *boxLayerInfo, HABITAT_TYPES habitat, double dtsz, int cIndex) {

	int sp, pid, cohort = 0;
	double cLevel = 0, uptake_rate = 0, cUptake = 0, cGroupLevel = 0, Cnew = 0, propLevel = 0, avoidLevel = 0, chronicLevel = 0, testLevel = 0, propContam = 0, time_step = 0;
	double *tracerArray;
	CONTAMINANT_UPTAKE_OPTION uptake_option;

	if (habitat == EPIFAUNA) {
		tracerArray = getTracerArray(boxLayerInfo, WC);
	} else {
		tracerArray = getTracerArray(boxLayerInfo, habitat);
	}

	/* For each contaminant calculate the uptake */

    /* Grab the level in the water column or the sediment */
    cLevel = tracerArray[bm->contaminantStructure[cIndex]->contaminant_tracer];
    if(cLevel > bm->min_pool){
        //fprintf(bm->logFile, "time %e, box %d, layer %d, cLevel = %e\n", bm->dayt, bm->current_box, bm->current_layer, cLevel);
        for (sp = 0; sp < bm->K_num_tot_sp; sp++) {

            /* Contact can occur for all active groups that are present in this habitat */
            if ((FunctGroupArray[sp].speciesParams[flag_id] == TRUE) && (FunctGroupArray[sp].isDetritus == FALSE) && (FunctGroupArray[sp].habitatCoeffs[habitat] > 0)) {
                
                uptake_option = (CONTAMINANT_UPTAKE_OPTION)bm->contaminantStructure[cIndex]->sp_uptake_option[sp];
                uptake_rate = bm->contaminantStructure[cIndex]->sp_uptake_rate[sp];

                for(cohort = 0; cohort < FunctGroupArray[sp].numCohortsXnumGenes; cohort++){
                    /* The current concentration in the group */
                    cGroupLevel = tracerArray[FunctGroupArray[sp].contaminantTracers[cohort][cIndex]];
                    cUptake = 0;

                    switch (uptake_option) {
                    case linear_contaminant_uptake_id: /* The linear contaminant formula accounting for internal content */
                        //cUptake = uptake_rate * cLevel;
                        cUptake = uptake_rate * (cLevel - cGroupLevel);
                        if (cUptake < 0) {
                            cUptake = 0;
                        }
                        break;
                    case flatlinear_contaminant_uptake_id:  /* The linear contaminant formula */
                    	cUptake = uptake_rate * cLevel;
                    	break;
                    case sigmoidal_uptake_id: /* The Richards sigmoidal uptake formula */
                        if (FunctGroupArray[sp].isOncePerDt == FALSE) {
                            time_step = dtsz;
                        } else if ((it_count == 1) && (FunctGroupArray[sp].isOncePerDt == TRUE)) {
                            time_step = bm->dt;
                        } else {
                            time_step = 0;
                        }

                        if ((time_step > 0) && (uptake_rate > 0)) {
                            /* Have the calculate the actual new tracer value and from this we calculate the flux values 
                             contam_sig_uptake_const set to 0.99 by default
                             */
                            
                            //fprintf(bm->logFile,"Time: %e box %d-%d contam_sig_uptake_const: %e\n", bm->dayt, bm->current_box, bm->current_layer, bm->contam_sig_uptake_const);
                            
                            Cnew = pow((pow(cLevel,(1.0 - bm->contam_sig_uptake_const))- (pow(cLevel,(1.0 - bm->contam_sig_uptake_const)) - pow(cGroupLevel, (1.0 - bm->contam_sig_uptake_const))) * exp(-uptake_rate * time_step * (bm->contam_sig_uptake_const + 1.0))),(1.0 / (1.0 - bm->contam_sig_uptake_const)));
                            cUptake = (Cnew - cGroupLevel) / time_step;
                            //cUptake = 0;
                        } else {
                            cUptake = 0.0;
                        }
                        break;
                    case invitro_sigmoid_id: /* The InVitro version of the sigmoidal uptake formula */
                        if (FunctGroupArray[sp].isOncePerDt == FALSE) {
                            time_step = dtsz;
                        } else if ((it_count == 1) && (FunctGroupArray[sp].isOncePerDt == TRUE)) {
                            time_step = bm->dt;
                        } else {
                            time_step = 0;
                        }

                        if ((time_step > 0) && (uptake_rate > 0)) {
                            /* Have the calculate the actual new tracer value and from this we calculate the flux values */
                            Cnew = cLevel / (cGroupLevel + (cLevel - cGroupLevel) * exp(-uptake_rate * time_step));
                            cUptake = (Cnew - cGroupLevel) / time_step;
                            //cUptake = 0;
                        } else {
                            cUptake = 0.0;
                        }
                        break;
                    case piecewise_linear_id:
                        quit("No idea what the Invitro code did here. Doco doesn't make much sense. Code if required.");
                        break;
                    default:
                        quit("Unrecognised uptake option %d for group %s", uptake_option, FunctGroupArray[sp].groupCode);
                        break;
                    }

                    
                    //fprintf(bm->logFile,"Time: %e box%d-%d %s-%d vs %s has uptake_option: %d cUptake: %e uptake_rate: %e cLevel: %e\n", bm->dayt, bm->current_box, bm->current_layer, FunctGroupArray[sp].groupCode, cohort, bm->contaminantStructure[cIndex]->contaminant_name, uptake_option, cUptake, uptake_rate, cLevel);
                    
                    bm->contaminantStructure[cIndex]->sp_uptake[sp][cohort][habitat] = cUptake;

                    // Initiate exposure time
                    if (!cGroupLevel && cUptake) {
                        bm->contaminantStructure[cIndex]->expose_time[sp][cohort][bm->current_box][bm->current_layer] = bm->dayt;
                    }
                        
                    if(!cUptake) {
                        continue;
                    }

                    // Proportion of individals contaminated
                    chronicLevel = bm->contaminantStructure[cIndex]->sp_maxChronicConc[sp];  // chronic level
                    avoidLevel = bm->contaminantStructure[cIndex]->sp_avoid[sp]; // avoidance level
                    testLevel = min(chronicLevel, avoidLevel);
                    propLevel = cUptake / (testLevel + small_num);
                    if (propLevel > 1.0) {
                        propLevel = 1.0;
                    }

                    propContam = drandom(0.0, propLevel);
                    pid = FunctGroupArray[sp].contamPropTracers[cohort][cIndex];

                    switch(habitat) {
                        case WC:
                            bm->boxes[bm->current_box].tr[bm->current_layer][pid] += propContam;
                            if (bm->boxes[bm->current_box].tr[bm->current_layer][pid] > 1.0) {
                                bm->boxes[bm->current_box].tr[bm->current_layer][pid] = 1.0;
                            }

                            //fprintf(bm->logFile, "Time %e box%d-%d %s %d %s has after uptake prop: %e with propContam %e chronicLevel: %e avoidLevel: %e propLevel: %e\n", bm->dayt, bm->current_box, bm->current_layer, FunctGroupArray[sp].groupCode, cohort, bm->contaminantStructure[cIndex]->contaminant_name, bm->boxes[bm->current_box].tr[bm->current_layer][pid], propContam, chronicLevel, avoidLevel, propLevel);

                            break;
                        case SED:
                            bm->boxes[bm->current_box].sm.tr[bm->current_layer][pid] += propContam;
                            if (bm->boxes[bm->current_box].sm.tr[bm->current_layer][pid] > 1.0) {
                                bm->boxes[bm->current_box].sm.tr[bm->current_layer][pid] = 1.0;
                            }
                            //fprintf(bm->logFile, "Time %e sed box%d-%d %s %d %s has after uptake prop: %e with propContam %e chronicLevel: %e avoidLevel: %e propLevel: %e\n", bm->dayt, bm->current_box, bm->current_layer, FunctGroupArray[sp].groupCode, cohort, bm->contaminantStructure[cIndex]->contaminant_name, bm->boxes[bm->current_box].sm.tr[bm->current_layer][pid], propContam, chronicLevel, avoidLevel, propLevel);

                            break;
                        case EPIFAUNA:
                            bm->boxes[bm->current_box].epi[pid] += propContam;
                            if (bm->boxes[bm->current_box].epi[pid] > 1.0) {
                                bm->boxes[bm->current_box].epi[pid] = 1.0;
                            }

                            //fprintf(bm->logFile, "Time %e epi box%d-%d %s %d %s has after uptake prop: %e with propContam %e chronicLevel: %e avoidLevel: %e propLevel: %e\n", bm->dayt, bm->current_box, bm->current_layer, FunctGroupArray[sp].groupCode, cohort, bm->contaminantStructure[cIndex]->contaminant_name, bm->boxes[bm->current_box].epi[pid], propContam, chronicLevel, avoidLevel, propLevel);

                            break;
                        default:
                            break;
                    }

                    /**
                    if(((cUptake > 10.0) || (sp == 54)) && (cIndex == 3)) {
                        fprintf(bm->logFile, "%s uptake = %e, box %d, layer %d, cGroupLevel= %e, uptake_option: %d, uptake_rate: %e, cUptake: %e, cLevel: %e, dtsz: %e, it_count = %d\n", FunctGroupArray[sp].groupCode, bm->contaminantStructure[cIndex]->sp_uptake[sp][cohort][habitat], bm->current_box, bm->current_layer, cGroupLevel, uptake_option, uptake_rate, cUptake, cLevel, dtsz, it_count);
                    }
                    **/

				}
			}
		}
	}
    
    /*
    pid = FunctGroupArray[8].contamPropTracers[3][0];
    fprintf(bm->logFile, "Time: %e at end of Species_Contam_Uptake for box%d-%d - test propContam %s-%d for %s in box%d-%d: %e\n", bm->dayt, bm->current_box, bm->current_layer, FunctGroupArray[8].groupCode, 3, bm->contaminantStructure[0]->contaminant_name, 3, 2, bm->boxes[3].tr[2][pid]);
    printf("Time: %e end of Species_Contam_Uptake for box%d-%d - test propContam %s-%d for %s in box%d-%d: %e\n", bm->dayt, bm->current_box, bm->current_layer, FunctGroupArray[8].groupCode, 3, bm->contaminantStructure[0]->contaminant_name, 3, 2, bm->boxes[3].tr[2][pid]);
     */

	return 0;
}

/**
 * Initialise the contaminant values at the start of each timestep.
 *
 *
 */
int Init_Contaminant_Transfer_Values(MSEBoxModel *bm) {
	int cIndex, sp, hab, cohort, globalHab;

	for (cIndex = 0; cIndex < bm->num_contaminants; cIndex++) {

		for(sp = 0; sp < bm->K_num_tot_sp; sp++){
			for(hab = 0; hab < bm->num_active_habitats; hab++){
				for(cohort = 0; cohort < FunctGroupArray[sp].numCohortsXnumGenes; cohort++){
					bm->contaminantStructure[cIndex]->sp_uptake[sp][cohort][hab] = 0;
					bm->contaminantStructure[cIndex]->sp_transfer[sp][cohort][hab] = 0;
					for(globalHab = 0; globalHab < bm->num_active_habitats; globalHab++){
						bm->contaminantStructure[cIndex]->sp_transfer_global[sp][cohort][hab][globalHab] = 0;
					}
				}
			}
		}
	}

	return 0;
}

/**
 * Transfer contaminant to the predator or detritus group.
 *
 */
int Group_Transfer_Contaminant(MSEBoxModel *bm, BoxLayerValues *boxLayerInfo, HABITAT_TYPES globalHabitat, HABITAT_TYPES habitat, int toGuild, int toCohort, int fromGuild, int fromCohort, double amountTransfer, double ***spSPinfo, double initialBiomass, double dtsz, int need_prop, int caseGTC) {

	int cIndex, pid;
	double *tracerArray;
    double cGroupLevel = 0, transfer, totalBiomass, amt_exchanged, toGuild_totalBiomass = 0.0, prop_exchanged, propContam, min_num, this_num;
	int isGlobal = (FunctGroupArray[toGuild].diagTol == 2 && it_count == 1);

	/* If the amount transfered is zero don't do anything
	 * Not ideal to do this check inside the function - for speed purposes we should do it outside the function but that results in a huge amount of additional code.
	 */
	if (amountTransfer == 0.0)
		return 0;
    
    if(isnan(dtsz)){
        quit("Group_Transfer_Contaminant - from group %s-%d, to group %s-%d, dtsz: %e\n", FunctGroupArray[fromGuild].groupCode, fromCohort, FunctGroupArray[toGuild].groupCode, toCohort, dtsz);
    }

	if(FunctGroupArray[fromGuild].habitatType == EPIFAUNA){
	  tracerArray = getTracerArray(boxLayerInfo, EPIFAUNA);
	} else {
	  tracerArray = getTracerArray(boxLayerInfo, habitat);
	}

	/* Work out the proportion transfered */
	if (FunctGroupArray[fromGuild].groupAgeType == AGE_STRUCTURED) {
        totalBiomass = (spSPinfo[fromGuild][fromCohort][SN_id] + spSPinfo[fromGuild][fromCohort][RN_id]) * spSPinfo[fromGuild][fromCohort][DEN_id] / bm->cell_vol;
        amountTransfer = amountTransfer / (totalBiomass + small_num);
	} else {
		//totalBiomass = tracerArray[FunctGroupArray[fromGuild].totNTracers[fromCohort]];

        if(!initialBiomass)  // No actual transfer possible
            return 0;

        totalBiomass = initialBiomass;
        amountTransfer = amountTransfer / (totalBiomass + small_num);
	}

    /* Sanity check */
    if (amountTransfer > 1.0) {
        amountTransfer = 1.0;
    }
    
    /**
	if ((isnan(propTransfer)) || (propTransfer > 1.0)) {
		fprintf(stderr, "Group_Transfer_Contaminant group propTransfer level is NAN or prop > 1 - From %s-%d, in box %d-%d, propTransfer: %e amountTransfer = %e, totalBiomass = %e\n", FunctGroupArray[fromGuild].groupCode, fromCohort, bm->current_box, bm->current_layer, propTransfer, amountTransfer, totalBiomass);
		quit("Group_Transfer_Contaminant nan or prop > 1\n");
	}
    **/

    if (need_prop) {
        if (FunctGroupArray[toGuild].groupAgeType == AGE_STRUCTURED) {
            toGuild_totalBiomass = (spSPinfo[toGuild][toCohort][SN_id] + spSPinfo[toGuild][toCohort][RN_id]) * spSPinfo[toGuild][toCohort][DEN_id] / bm->cell_vol;
            
            if(isnan(toGuild_totalBiomass)) {
                quit("Time: %e %s-%d returning nan biomass in box%d-%d for Group_Transfer_Contaminant - SN: %e RN: %e DEN: %e cell_vol: %e\n", bm->dayt, FunctGroupArray[toGuild].groupCode, toCohort, bm->current_box, bm->current_layer, spSPinfo[toGuild][toCohort][SN_id], spSPinfo[toGuild][toCohort][RN_id], spSPinfo[toGuild][toCohort][DEN_id], bm->cell_vol);
            }
        } else {
            toGuild_totalBiomass = tracerArray[FunctGroupArray[toGuild].totNTracers[toCohort]];
            
            if(isnan(toGuild_totalBiomass)) {
                quit("Time: %e %s-%d returning nan biomass in box%d-%d for Group_Transfer_Contaminant\n", bm->dayt, FunctGroupArray[toGuild].groupCode, toCohort, bm->current_box, bm->current_layer);
            }
        }
    }
    

	for (cIndex = 0; cIndex < bm->num_contaminants; cIndex++) {
		/* The current concentration in the group */
		cGroupLevel = tracerArray[FunctGroupArray[fromGuild].contaminantTracers[fromCohort][cIndex]];

		if (cGroupLevel > bm->min_pool){

            /**
            if ((isnan(propTransfer)) || (propTransfer > 1.0)) {
                fprintf(stderr, "Time: %e box %d-%d case %d Group_Transfer_Contaminant group propTransfer level is NAN or prop > 1 - From %s-%d to %s-%d propTransfer: %e amountTransfer = %e totalBiomass = %e %s cGroupLevel: %e\n", bm->dayt, bm->current_box, bm->current_layer, caseGTC, FunctGroupArray[fromGuild].groupCode, fromCohort, FunctGroupArray[toGuild].groupCode, toCohort, propTransfer, amountTransfer, totalBiomass, bm->contaminantStructure[cIndex]->contaminant_name, cGroupLevel);
                quit("Group_Transfer_Contaminant nan or prop > 1\n");
            }
            **/

			//transfer = cGroupLevel * amountTransfer / dtsz;  // Need time step correction so flux makes sense
            transfer = cGroupLevel * amountTransfer;  // Actually already a rate so ok?

            /**/
			//if(isnan(transfer)){
            if(!(_finite(transfer))) {
                fflush(bm->logFile);
				quit("Group_Transfer_Contaminant - from group %s-%d, to group %s-%d, transfer is nan or inf, cGroupLevel= %e, amountTransfer = %e dtsz: %e\n",
						FunctGroupArray[fromGuild].groupCode, fromCohort, FunctGroupArray[toGuild].groupCode, toCohort, cGroupLevel, amountTransfer, dtsz);
			}
            /**/

			bm->contaminantStructure[cIndex]->sp_transfer[toGuild][toCohort][habitat] += transfer;
			bm->contaminantStructure[cIndex]->sp_transfer[fromGuild][fromCohort][habitat] -= transfer;

            /**
            //if((((toGuild == 54) || (fromGuild == 54)) && (bm->contaminantStructure[cIndex]->sp_transfer[toGuild][toCohort][habitat] > 0.0)) && (cIndex == 3)) {
                    fprintf(bm->logFile, "prey = %s, to %s-%d gaining %e, cGroupLevel= %e, propTransfer= %e, totalTransfer = %e amountTransfer = %e, totalBiomass= %e\n", FunctGroupArray[fromGuild].groupCode, FunctGroupArray[toGuild].groupCode, toCohort, transfer, cGroupLevel, propTransfer, bm->contaminantStructure[cIndex]->sp_transfer[toGuild][toCohort][habitat], amountTransfer, totalBiomass);
            //}
            **/

			if (isGlobal == TRUE) {
				bm->contaminantStructure[cIndex]->sp_transfer_global[toGuild][toCohort][globalHabitat][habitat] += transfer;
				bm->contaminantStructure[cIndex]->sp_transfer_global[fromGuild][fromCohort][globalHabitat][habitat] -= transfer;

			}

            if (need_prop && transfer) {
                /* Set or update the proportion contaminated */
                amt_exchanged = amountTransfer * dtsz;
                prop_exchanged = amt_exchanged / (toGuild_totalBiomass + small_num);

                if( prop_exchanged > 1.0)
                    prop_exchanged = 1.0;
                

                min_num = 0.0;
                if(FunctGroupArray[toGuild].groupAgeType == AGE_STRUCTURED ){
                    this_num = tracerArray[FunctGroupArray[toGuild].NumsTracers[toCohort]];
                    min_num = 1.0 / (this_num + small_num);
                } else {
                    min_num = bm->min_pool;  // Just using this as a proxy for a small number here
                }

                
                propContam = drandom(min_num, prop_exchanged);
                pid = FunctGroupArray[toGuild].contamPropTracers[toCohort][cIndex];

                switch(habitat) {
                    case WC:
                        bm->boxes[bm->current_box].tr[bm->current_layer][pid] += propContam;
                        if (bm->boxes[bm->current_box].tr[bm->current_layer][pid] > 1.0) {
                            bm->boxes[bm->current_box].tr[bm->current_layer][pid] = 1.0;
                        }

                        //fprintf(bm->logFile, "Time %e box%d-%d %s %d %s has after transfer prop: %e with propContam %e amt_exchanged: %e toGuild_totalBiomass: %e\n", bm->dayt, bm->current_box, bm->current_layer, FunctGroupArray[toGuild].groupCode, toCohort, bm->contaminantStructure[cIndex]->contaminant_name, bm->boxes[bm->current_box].tr[bm->current_layer][pid], propContam, amt_exchanged, toGuild_totalBiomass);

                        break;
                    case SED:
                        bm->boxes[bm->current_box].sm.tr[bm->current_layer][pid] += propContam;
                        if(bm->boxes[bm->current_box].sm.tr[bm->current_layer][pid] > 1.0){
                            bm->boxes[bm->current_box].sm.tr[bm->current_layer][pid] = 1.0;
                        }

                        //fprintf(bm->logFile, "Time %e box%d-%d %s %d %s has after transfer prop: %e with propContam %e amt_exchanged: %e toGuild_totalBiomass: %e\n", bm->dayt, bm->current_box, bm->current_layer, FunctGroupArray[toGuild].groupCode, toCohort, bm->contaminantStructure[cIndex]->contaminant_name, bm->boxes[bm->current_box].sm.tr[bm->current_layer][pid], propContam, amt_exchanged, toGuild_totalBiomass);

                        break;
                    case EPIFAUNA:
                        bm->boxes[bm->current_box].epi[pid] += propContam;
                        if (bm->boxes[bm->current_box].epi[pid] > 1.0) {
                            bm->boxes[bm->current_box].epi[pid] = 1.0;
                        }

                        //fprintf(bm->logFile, "Time %e box%d-%d %s %d %s has after transfer prop: %e with propContam %e amt_exchanged: %e toGuild_totalBiomass: %e\n", bm->dayt, bm->current_box, bm->current_layer, FunctGroupArray[toGuild].groupCode, toCohort, bm->contaminantStructure[cIndex]->contaminant_name, bm->boxes[bm->current_box].epi[pid], propContam, amt_exchanged, toGuild_totalBiomass);

                        break;
                    default:
                        break;
                }

            }
		}
	}

	return 0;
}

/**
 * Transfer the global values across.
 *
 *
 */

int Reconcile_Global_Contaminant_Values(MSEBoxModel *bm, HABITAT_TYPES habitatType) {

	int cIndex, pred, pred_chrt, hab;

	for (hab = WC; hab < bm->num_active_habitats; hab++) {

		for (cIndex = 0; cIndex < bm->num_contaminants; cIndex++) {
			for (pred = 0; pred < bm->K_num_tot_sp; pred++) {
				for (pred_chrt = 0; pred_chrt < FunctGroupArray[pred].numCohortsXnumGenes; pred_chrt++) {
					bm->contaminantStructure[cIndex]->sp_transfer[pred][pred_chrt][hab] =
							bm->contaminantStructure[cIndex]->sp_transfer_global[pred][pred_chrt][habitatType][hab];
				}
			}
		}
	}

	return 0;
}
/**
 * Calculate the contaminant decay in this timestep.
 *
 *
 */
int Calculate_Species_Contaminant_Decay(MSEBoxModel *bm, BoxLayerValues *boxLayerInfo, HABITAT_TYPES habitat, double dtsz, int cIndex, double cGroupLevel) {
	int cohort, sp;
	double newValue, time_step;
    double *tracerArray;
    //double newValue;
    double local_cGroupLevel, this_half_life;
    //double decay_constant;

	for (sp = 0; sp < bm->K_num_tot_sp; sp++) {
        
        tracerArray = getTracerArray(boxLayerInfo, habitat);
        
        if(bm->flag_contam_halflife_spbased) {
            this_half_life = bm->contaminantStructure[cIndex]->sp_decay_half_life[sp];
        } else {
            this_half_life = bm->contaminantStructure[cIndex]->half_life;
        }

		/* Contact can occur for all active groups that are present in this habitat */
		if (FunctGroupArray[sp].speciesParams[flag_id] == TRUE && FunctGroupArray[sp].habitatCoeffs[habitat] > 0) {

            if (FunctGroupArray[sp].isOncePerDt == FALSE) {
                time_step = dtsz;
            } else if ((it_count == 1) && (FunctGroupArray[sp].isOncePerDt == TRUE)) {
                time_step = bm->dt;
            } else {
                time_step = 0;
            }
            
            if(!time_step)
                continue;
            
            // Apply the effect
            for(cohort = 0; cohort < FunctGroupArray[sp].numCohortsXnumGenes; cohort++){
                
                local_cGroupLevel = tracerArray[FunctGroupArray[sp].contaminantTracers[cohort][cIndex]];  // Need to use local cGroupLevel as needs to be per cohort not at species level

                /* The current concentration in the group */
                
                newValue = local_cGroupLevel * pow(0.5, time_step / this_half_life);
                bm->contaminantStructure[cIndex]->sp_amount_decayed[sp][cohort] = (local_cGroupLevel - newValue) / time_step;
                
                /* Alternative code -- Using the decay constant instead of the half life
                decay_constant = (log(2))/bm->contaminantStructure[cIndex]->half_life;
                newValue = local_cGroupLevel * decay_constant * time_step;
                bm->contaminantStructure[cIndex]->sp_amount_decayed[sp][cohort] = (local_cGroupLevel - newValue) / time_step;
                */

                //fprintf(bm->logFile,"Time: %e %s-%d contaminant: %s cGroupLevel: %e decay_constant: %e\n", bm->dayt, FunctGroupArray[sp].groupCode, cohort, bm->contaminantStructure[cIndex]->contaminant_name, cGroupLevel, decay_constant);

                
			}
		}
	}
	return 0;

}

/**************************************************************************************************************************************************************
 * Functions associated the impact a contaminant has on a group.
 ***************************************************************************************************************************************************************/

//#define Tmortality(conc, ld, mc, ldt, dt) min(1.0, max(((log(ld/mc)/ldt)*(dt)/(2.0*log(conc/mc))), 0.0))

/**
 * Conc = Concentration in environment or group.
 * ld = LD50
 * mc = Max concentration - LD100
 * ldt = time to LD50
 * dt = current dt
 *
 */

double Tmortality(MSEBoxModel *bm, double conc, double ld, double mc, double ldt, double dt){
	double x;

	dt = 1.0;	/* We want mortality per second */

	x =  max((log(ld/mc)/ldt)*(dt)/(2.0*log(conc/mc)), 0.0);

	//fprintf(bm->logFile, "x = %e, dt = %e, ldt= %e\n", x, dt, ldt);

	return min(1.0, x);
	//return min(1.0, max(((log(ld/mc)/ldt)*(dt)/(2.0*log(conc/mc))), 0.0));
}

/**
 *
 * Get the current population - gets the current multiple contaminantion point of the species in this location
 *
 */
double CurrentPopContam(MSEBoxModel *bm, int species, int cohort) {
    double r = 1;
    int i = 0;

    for (i = 0; i < bm->num_contaminants; i++) {
        r *= (1.0 - bm->contaminantStructure[i]->sp_point[species][cohort][bm->current_box][bm->current_layer]);
    }

    return r;
}


/***
 *
 * Calculate the mortality due to contaminants.
 * The acute mortality is due to water column values.
 * The chronic mortality is due to levels in the group - not the values in the environment.
 *
 * Mortality sources across multiple contaminants if InVitro.

 *
 */
void Get_ContamMortEffects(MSEBoxModel *bm, double cEnvLevel, double cGroupLevel, int species, int cohort, int cIndex, double cPop, double dtsz) {
    double contam_survivor_prop = 1.0;
    double LC50 = bm->contaminantStructure[cIndex]->sp_LD50[species];
    double maxLevel, step1, step2, chronicLevel, contam_pt, contamDelta, ci, cj;
    double final_mort_rate_mod = 0.0;
    double epsilon = 0, exposure_time = 0;

    // If LD50 is temperature and exposure time dependent then update it here - from French-McCay et al 2004 (Environ. Toxicol. Chem.)
    if ( bm->contaminantStructure[cIndex]->contam_temp_depend ) {
        if (bm->flag_contamMinTemp > H2Otemp) {
            epsilon = 0.5 * bm->contam_tau * bm->flag_contamMinTemp * bm->flag_contamMinTemp;
        } else {
            epsilon = 0.5 * bm->contam_tau * H2Otemp * H2Otemp;
        }
        exposure_time = bm->dayt - bm->contaminantStructure[cIndex]->expose_time[species][cohort][bm->current_box][bm->current_layer];
        if(!exposure_time) {
            exposure_time = 1.0;
        }
        LC50 = bm->contaminantStructure[cIndex]->sp_LD50[species] * (1.0 - exp(-1.0 * epsilon * exposure_time));
    }

    if ( bm->flag_contamMortModel ) {
        /* Simple model from Laender et al 2008 (Chemosphere) and French-McCay et al 2004 (Environ. Toxicol. Chem.) */

        if ((FunctGroupArray[species].groupType == BIRD) || (FunctGroupArray[species].groupType == MAMMAL) || (FunctGroupArray[species].groupType == CORAL)
            || (FunctGroupArray[species].groupType == SPONGE) || (FunctGroupArray[species].groupType == SED_EP_FF)) {

            // Smothering = main source of mortality - as of French-McCay et al 2004
            maxLevel = bm->contaminantStructure[cIndex]->sp_maxLethalConc[species];  // lethal level
            if (cEnvLevel >= maxLevel) {
                FunctGroupArray[species].contaminantSpMort[cohort] += bm->contaminantStructure[cIndex]->sp_instantDoseMortality[species];  // Probability of mortality once covered with the contaminant (like oil)

                if(!(_finite(FunctGroupArray[species].contaminantSpMort[cohort]))) {
                    fflush(bm->logFile);
                    quit("Time: %e %s-%d contaminantSpMor: %e sp_instantDoseMortality: %e\n", bm->dayt, FunctGroupArray[species].groupCode, cohort, FunctGroupArray[species].contaminantSpMort[cohort], bm->contaminantStructure[cIndex]->sp_instantDoseMortality[species]);
                }
            }
        } else {
            // Internal tissue content is the killer and uses LC_50 - from Laender et al 2008
            step1 = cGroupLevel / LC50;
            step2 = pow(step1, bm->contaminantStructure[cIndex]->sp_LDslope[species]);

            FunctGroupArray[species].contaminantSpMort[cohort] += log(1.0 + step2) / bm->contaminantStructure[cIndex]->sp_TimeToLD50[species];

            if(!(_finite(FunctGroupArray[species].contaminantSpMort[cohort]))) {
                fflush(bm->logFile);
                quit("Time: %e %s-%d contaminantSpMor: %e step2: %e step1: %e LC50: %e sp_LDslope: %e cGroupLevel: %e LD50 %e exp: %e epsilon: %e exposure_time: %e tau: %e H20temp: %e\n", bm->dayt, FunctGroupArray[species].groupCode, cohort, FunctGroupArray[species].contaminantSpMort[cohort], step2, step1, LC50, bm->contaminantStructure[cIndex]->sp_LDslope[species], cGroupLevel, bm->contaminantStructure[cIndex]->sp_LD50[species], (1.0 - exp(-1.0 * epsilon * exposure_time)), epsilon, exposure_time, bm->contam_tau, H2Otemp);
            }


        }

        //fprintf(bm->logFile,"Time: %e %s-%d contaminantSpMor: %e\n", bm->dayt, FunctGroupArray[species].groupCode, cohort, FunctGroupArray[species].contaminantSpMort[cohort]);

    } else {
        /** InVitro multiple contaminant model **/

        /* The current concentration in the group */
        maxLevel = bm->contaminantStructure[cIndex]->sp_maxLethalConc[species];  // lethal level
        chronicLevel = bm->contaminantStructure[cIndex]->sp_maxChronicConc[species];  // chronic level

        //if(cEnvLevel > 0 && cGroupLevel > 0)
        //    fprintf(bm->logFile, "Group %s, cGroupLevel = %e, cEnvLevel= %e, maxLevel= %e\n", FunctGroupArray[species].groupCode, cGroupLevel, cEnvLevel, maxLevel);

        /* Get acute mortality due to what's in the water column */
        if (cEnvLevel >= maxLevel) {
            // First look at contaminant level equivalents responding to (note actual uptake handled elsewhere)
            contam_pt = bm->contaminantStructure[cIndex]->sp_point[species][cohort][bm->current_box][bm->current_layer] + cEnvLevel * (1.0 - bm->contaminantStructure[cIndex]->sp_point[species][cohort][bm->current_box][bm->current_layer]);

            if (contam_pt > 1.0)
                contam_pt = 1.0;

            contam_survivor_prop *= (1.0 - contam_pt);
        } else if (cEnvLevel > 0) {
            step1 = Tmortality(bm, cEnvLevel, LC50, maxLevel, bm->contaminantStructure[cIndex]->sp_TimeToLD50[species], dtsz) - bm->contaminantStructure[cIndex]->sp_point[species][cohort][bm->current_box][bm->current_layer];
            if (step1 < 0.0)
                step1 = 0.0;
            contam_survivor_prop *= (1.0 - step1);
        }


        /* Chronic mortality due to tissue level -- chronic effects occur when tissue level exceeds water level */
        contamDelta = 0;
        if (cGroupLevel > chronicLevel) {
            if (cGroupLevel > maxLevel) {
                contamDelta = 1.0 - bm->contaminantStructure[cIndex]->sp_point[species][cohort][bm->current_box][bm->current_layer];
            } else if (cGroupLevel > 0){
                contamDelta = Tmortality(bm, cGroupLevel, LC50, maxLevel, bm->contaminantStructure[cIndex]->sp_TimeToLD50[species], dtsz) - bm->contaminantStructure[cIndex]->sp_point[species][cohort][bm->current_box][bm->current_layer];
            } else {
                contamDelta = 0;
            }
        }

        if (contamDelta < 0)
            contamDelta = 0.0;

        // This means that plonking a thing in the ld50 concentration will give you the correct number of dead in ldT, and exponentially decaying  death rates
        contamDelta *= (1.0 - bm->contaminantStructure[cIndex]->sp_point[species][cohort][bm->current_box][bm->current_layer]);

        /* Also update the internal dose levels so ready for future chronic effects */
        if (bm->contaminantStructure[cIndex]->sp_point[species][cohort][bm->current_box][bm->current_layer] > 1.0) {
            bm->contaminantStructure[cIndex]->sp_point[species][cohort][bm->current_box][bm->current_layer] = 1.0;
        } else {
            ci = cGroupLevel /maxLevel;

            if (cGroupLevel > bm->contaminantStructure[cIndex]->sp_maxDoseToDate[species][cohort][bm->current_box][bm->current_layer]) {
                bm->contaminantStructure[cIndex]->sp_maxDoseToDate[species][cohort][bm->current_box][bm->current_layer] = cGroupLevel;
                // do the two part piecewise interpolation here -- this defaults to a simple linear mortality
                //piecewise(ci, Cx, Cy) = ((Cx >= 1.0 ? Cy : (ci >= Cx ? (1-Cy)/(1-Cx) * (ci - Cx) + Cy : (Cy/Cx * ci))))
                // cj is the portion of the axis that lC[i] represents.
                if (bm->contaminantStructure[cIndex]->sp_Cx[species] >= 1.0) {
                    cj = bm->contaminantStructure[cIndex]->sp_Cy[species];
                } else {
                    if(ci >= bm->contaminantStructure[cIndex]->sp_Cx[species]){
                        cj = (1 - bm->contaminantStructure[cIndex]->sp_Cy[species]) / (1 - bm->contaminantStructure[cIndex]->sp_Cx[species]) * (ci - bm->contaminantStructure[cIndex]->sp_Cx[species]) +  bm->contaminantStructure[cIndex]->sp_Cy[species];
                    } else {
                        cj= ( bm->contaminantStructure[cIndex]->sp_Cy[species] / bm->contaminantStructure[cIndex]->sp_Cx[species] * ci);
                    }
                }

                bm->contaminantStructure[cIndex]->sp_point[species][cohort][bm->current_box][bm->current_layer] = max(bm->contaminantStructure[cIndex]->sp_point[species][cohort][bm->current_box][bm->current_layer], cj);
            }
        }

        final_mort_rate_mod = (1.0 - contam_survivor_prop) + contamDelta;
        FunctGroupArray[species].contaminantSpMort[cohort] += final_mort_rate_mod;

    }

}

/**
 *
 * Calculate growth effects
 *
 */
void Get_ContamGrowthEffects(MSEBoxModel *bm, double cGroupLevel, int species, int cohort, int cIndex) {
    double step1, L_sp, a_sp, b_sp;

    switch (bm->flag_contamGrowthModel){
        case NoGrowthEffects: // No growth effects
            FunctGroupArray[species].C_growth_corr[cohort] = 1.0;
            break;
        case InVitro_model:  // Growth effects as of InVitro
            if (cGroupLevel > bm->contaminantStructure[cIndex]->sp_GrowthThresh[species]) {
                if ((FunctGroupArray[species].isPrimaryProducer == TRUE) && (FunctGroupArray[species].isMicroFauna == TRUE)){
                    step1 = 1.0 + pow((cGroupLevel / bm->contaminantStructure[cIndex]->sp_EC50[species]), bm->contaminantStructure[cIndex]->sp_ECslope[species]);
                    FunctGroupArray[species].C_growth_corr[cohort] *= 1.0 / step1; // As of Laender et al 2008
                } else {
                    FunctGroupArray[species].C_growth_corr[cohort] *= bm->contaminantStructure[cIndex]->sp_GrowthEffect[species]; // As of Invitro and French-McCay et al 2004
                }
            }
            break;
        case Salmon_logistic_model: // Growth effects defined using a logistic (as defined for salmon work)
            L_sp = bm->contaminantStructure[cIndex]->sp_L[species];
            b_sp = bm->contaminantStructure[cIndex]->sp_B[species];
            a_sp = bm->contaminantStructure[cIndex]->sp_A[species];
            FunctGroupArray[species].C_growth_corr[cohort] = 1.0 - L_sp / (1.0 + exp(-1.0 * a_sp * (cGroupLevel - b_sp)));

            break;
    }

    //fprintf(bm->logFile,"Time: %e %s-%d C_growth_corr: %e\n", bm->dayt, FunctGroupArray[species].groupCode, cohort, FunctGroupArray[species].C_growth_corr[cohort]);


    return;

}

/**
 *
 * Reproduction effects - calculating the scalar to apply to settlers to take into account effect of contaminants
 * on the number of settlers.
 *
 * The contaiminant effects are due to the level of contaminant levels in the animal not the level
 * in the surrounding water columns.
 *
 */

void Get_ContamReproductionEffects(MSEBoxModel *bm, double cGroupLevel, int species, int cohort, int cIndex) {
  double chronicLevel = bm->contaminantStructure[cIndex]->sp_maxChronicConc[species];  // chronic level
  int age_mat = (int) (FunctGroupArray[species].speciesParams[age_mat_id]);
  double step1, step2, L_sp_r, b_sp_r, a_sp_r;
  
  if ( cohort < age_mat ) // To young for it to matter.
    return;
  
  switch (bm->flag_contamReprodModel){
  case NoReprodEffects: // No repro effects
    FunctGroupArray[species].C_reprod_corr[cohort] = 1.0;
    break;
  case Lovindeeretal: //Equation 13 Lovideer et al. paper  
    if (cGroupLevel > chronicLevel) {
      step1 = bm->contaminantStructure[cIndex]->sp_ReprodEffect[species] * cGroupLevel;
      if (step1 > 1.0)
        step1 = 1.0;
      step2 = 1.0 - step1;
      if (step2 < FunctGroupArray[species].C_reprod_corr[cohort]) // As taking the minimum value only
        FunctGroupArray[species].C_reprod_corr[cohort] *= step2;
    }
    break;
  case InVitro_model_r:  // Reprod effects as of InVitro
  //  printf("Going into Invitro Model");
    
    if (cGroupLevel > bm->contaminantStructure[cIndex]->sp_ReprodThresh[species]) {
      step1 = 1.0 + pow((cGroupLevel / bm->contaminantStructure[cIndex]->sp_EC50_r[species]), bm->contaminantStructure[cIndex]->sp_ECslope_r[species]);
      FunctGroupArray[species].C_reprod_corr[cohort] *= 1.0 / step1; // As of Laender et al 2008
 //     printf("C_reprod_corr: %e\n", FunctGroupArray[species].C_reprod_corr[cohort]);
      //
    }
    break;
  case Salmon_logistic_model_r: // Repro effects defined using a logistic (as defined for salmon work)
    L_sp_r = bm->contaminantStructure[cIndex]->sp_L_r[species];
    b_sp_r = bm->contaminantStructure[cIndex]->sp_B_r[species];
    a_sp_r = bm->contaminantStructure[cIndex]->sp_A_r[species];
    FunctGroupArray[species].C_reprod_corr[cohort] = 1.0 - L_sp_r / (1.0 + exp(-1.0 * a_sp_r * (cGroupLevel - b_sp_r)));
    
    break;
    
    
  }
  
  // fprintf(bm->logFile,"Time: %e %s C_reprod_corr: %e\n", bm->dayt, FunctGroupArray[species].groupCode, cGroupLevel);
  // fprintf(bm->logFile,"Time: %e %s C_reprod_corr: %e\n", bm->dayt, FunctGroupArray[species].groupCode, FunctGroupArray[species].C_reprod_corr[cohort]);
  // 
  
  
  
  
  return;
}

/**
 *
 * Movement effects - based on comments in Dell'Omo 2002 (Behavioural Ecotoxicology)
 * animals effected by contaminants can't move as well
 *
 */
void Get_ContamMoveEffects(MSEBoxModel *bm, int species, int cohort, int box, int layer) {
    double step1, step2, chronicLevel = 0, cGroupLevel = 0;
    int contamIndex, pid = 0;

    FunctGroupArray[species].C_move_corr[cohort] = 1.0;

    for (contamIndex = 0; contamIndex < bm->num_contaminants; contamIndex++) {
        chronicLevel = bm->contaminantStructure[contamIndex]->sp_maxChronicConc[species];  // chronic level
        pid = FunctGroupArray[species].contaminantTracers[cohort][contamIndex];
        cGroupLevel = bm->boxes[box].tr[layer][pid];

        step2 = 1.0;
        if (cGroupLevel > chronicLevel) {
            step1 = bm->contaminantStructure[contamIndex]->sp_MoveEffect[species] * cGroupLevel;
            if (step1 > 1.0) {
                step1 = 1.0;
            }
            step2 = 1.0 - step1;
            if(step2 < 0.0) {
                step2 = 0.0;
            }
        }
        FunctGroupArray[species].C_move_corr[cohort] *= step2;

    }

    //fprintf(bm->logFile,"Time: %e %s-%d C_move_corr: %e\n", bm->dayt, FunctGroupArray[species].groupCode, cohort, FunctGroupArray[species].C_move_corr[cohort]);


    return;
}

/**
 *
 * Cross contaminant interactions and amplification - simple matrix of interaction coefficients, assumed to be multiplicative/cumulative
 *
 */

double Get_Contam_Amplification(MSEBoxModel *bm, int species, int cohort, int cIndex, double *tracerArray) {
    double cum_contam_scalar = 1.0;
    int contamIndex;
    double contam_interaction_coefft = 0, contam_scalar = 1.0;

    if (!bm->flag_contamInteractModel) {
        return cum_contam_scalar;  // As no interaction scalar is set to 1.0
    }

    // Loop over other contaminants to check for interactions
    for (contamIndex = 0; contamIndex < bm->num_contaminants; contamIndex++) {
       contam_interaction_coefft = bm->contaminantStructure[cIndex]->interact_coefft[contamIndex];

        contam_scalar = contam_interaction_coefft * (tracerArray[FunctGroupArray[species].contaminantTracers[cohort][contamIndex]] / bm->contaminantStructure[contamIndex]->sp_LD50[species]);

        switch (bm->flag_contamInteractModel) {
            case no_contam_interact:
                // Nothing to do - code should never get here
                break;
            case additive_contam_interact:
                cum_contam_scalar += contam_scalar;
                break;
            case mult_contam_interact:
                cum_contam_scalar *= contam_scalar;
                break;
            case most_lim_contam_interact: // That with greatest effect dictates the ultimate outcome
                if (contam_scalar > cum_contam_scalar)
                    cum_contam_scalar = contam_scalar;
                break;
            default:
                quit("No such contaminant interaction option (%d), must be no interaciton (0), additive (1), multiplicative (2), most limiting (3)\n");
                break;
        }

        /**
        if (cum_contam_scalar > 10) {
            fprintf(bm->logFile,"Time: %e  box%d-%d %s vs %s cum_contam_scalar: %e contam_scalar: %e contam_interaction_coefft: %e %s tracer: %e tracerID %d\n", bm->dayt, bm->current_box, bm->current_layer, bm->contaminantStructure[cIndex]->contaminant_name, bm->contaminantStructure[contamIndex]->contaminant_name, cum_contam_scalar, contam_scalar, contam_interaction_coefft, bm->contaminantStructure[contamIndex]->contaminant_name, tracerArray[FunctGroupArray[species].contaminantTracers[cohort][contamIndex]], FunctGroupArray[species].contaminantTracers[cohort][cIndex]);
        }
        **/
    }

    if (bm->flag_contamOnlyAmplify && (cum_contam_scalar < 1.0)) {
        cum_contam_scalar = 1.0;
    }

    return cum_contam_scalar;
}

/**
 *
 * Not yet tested. Issues with the Invitro way of working - that was a value per agent - we really need to keep track of these values per box/layer as well and then
 * there are issues with movement - be better if we can calculate an instant mortality rather than having to keep track of past mortality values as per the sp_point code.
 *
 *
 *
 */

int Calculate_Species_Contaminant_Effects(MSEBoxModel *bm, int box, int clayer, double dtsz, HABITAT_TYPES habitatType) {

	int cIndex, species, cohort;
	double *tracerArray = NULL;
	double *elementTracerArray = NULL;
    //double step1, step2;
    double cGroupLevel = 0, cEnvLevel, conc_amplif = 1.0, cPop;
    //double chronicLevel;

    // Reinit the scalars
    for (species = 0; species < bm->K_num_tot_sp; species++) {
        for(cohort = 0; cohort < FunctGroupArray[species].numCohortsXnumGenes; cohort++){
            FunctGroupArray[species].C_growth_corr[cohort] = 1.0; // Assumed multiplicative effects
            FunctGroupArray[species].C_move_corr[cohort] = 1.0; // Assumed multiplicative effects
            FunctGroupArray[species].contaminantSpMort[cohort] = 0.0;  // So zeroed with each new layer and box and calculated fresh in each case - assumed additive effects.
            FunctGroupArray[species].C_reprod_corr[cohort]  = 1.0; // Assumed multiplicative effects
        }
    }

    switch (habitatType) {
	case WC:
		tracerArray = (double*) bm->boxes[box].tr[clayer];
		elementTracerArray = tracerArray;
		break;
	case SED:
		tracerArray = (double*) bm->boxes[box].sm.tr[clayer];
		elementTracerArray = tracerArray;

		break;
	case EPIFAUNA:
		tracerArray = (double*) bm->boxes[box].epi;
		//tracerArray = (double*) bm->boxes[box].tr[0];
		elementTracerArray = (double*) bm->boxes[box].tr[0];
		break;
    case LAND_BASED:
    case ICE_BASED:
        quit("Get developers to add ice/land case for contaminants\n");
        break;
    case MIXED:
        quit("How did we get here as should come through a primary habitat\n");
        break;
	default:
		abort();
		break;
	}

    for (cIndex = 0; cIndex < bm->num_contaminants; cIndex++) {

        //printf("Doing contam %s (%d vs %d)\n", bm->contaminantStructure[cIndex]->contaminant_name, cIndex, bm->num_contaminants);

        for (species = 0; species < bm->K_num_tot_sp; species++) {

            //printf("Doing %s (%d vs %d)\n", FunctGroupArray[species].groupCode, species,bm->K_num_tot_sp);


            if ((FunctGroupArray[species].speciesParams[flag_id] == TRUE) && (FunctGroupArray[species].isDetritus == FALSE) && (FunctGroupArray[species].habitatCoeffs[habitatType] > 0)) {

                for(cohort = 0; cohort < FunctGroupArray[species].numCohortsXnumGenes; cohort++){
                    /* Get amplification factor due to interaction of contaminants */

                    //printf("Doing %s-%d contam %s - A\n", FunctGroupArray[species].groupCode, cohort, bm->contaminantStructure[cIndex]->contaminant_name);

                    conc_amplif = Get_Contam_Amplification(bm, species, cohort, cIndex, tracerArray);

                    cPop = CurrentPopContam(bm, species, cohort);

                    /* Grab the level in the water column or the sediment */
                    cEnvLevel = elementTracerArray[bm->contaminantStructure[cIndex]->contaminant_tracer];

                    /* The current concentration in the group */
                    cGroupLevel = tracerArray[FunctGroupArray[species].contaminantTracers[cohort][cIndex]];
                    cGroupLevel *= conc_amplif;  // Amplification due to interaction with other contaminants (such as marine debris allowing for higher concentrations of contaminants than seawater or tissues

                    /**
                    if(cGroupLevel > 0) {
                        fprintf(bm->logFile, "Time: %e box %d layer %d doing %s-%d contam %s - cGroupLevel: %e (tracer %e) tracerID: %d, conc_amplif: %e tracer %s, cEnvLevel: %e cPop: %e\n", bm->dayt, box, clayer, FunctGroupArray[species].groupCode, cohort, bm->contaminantStructure[cIndex]->contaminant_name, cGroupLevel, tracerArray[FunctGroupArray[species].contaminantTracers[cohort][cIndex]],  FunctGroupArray[species].contaminantTracers[cohort][cIndex], conc_amplif, bm->tinfo[FunctGroupArray[species].contaminantTracers[cohort][cIndex]].name, cEnvLevel, cPop);
                    }
                    **/

                    if (cGroupLevel <= 0) {
                        // Nothing to worry about
                        cGroupLevel = 0;
                        continue;
                    }

                    // Growth effects
                    Get_ContamGrowthEffects(bm, cGroupLevel, species, cohort, cIndex);

                    // Reproduction effects
                    Get_ContamReproductionEffects(bm, cGroupLevel, species, cohort, cIndex);

                    // Movement effects - now moved to atmovement.c
                    //Get_ContamMoveEffects(bm, species, cohort);

                    // Mortality effects
                    Get_ContamMortEffects(bm, cEnvLevel, cGroupLevel, species, cohort, cIndex, cPop, dtsz);

                    //printf("Doing %s-%d contam %s - B2\n", FunctGroupArray[species].groupCode, cohort, bm->contaminantStructure[cIndex]->contaminant_name);

                }
            }

            /*
            // Generic effects of contaminants on other physiological parameters - TODO: Make this contaminant effect more than just a linear equation
            chronicLevel = bm->contaminantStructure[cIndex]->sp_maxChronicConc[species];  // chronic level
            
            if (cGroupLevel > chronicLevel) {
                step1 = bm->contaminantStructure[cIndex]->sp_ContamScalar[species] * cGroupLevel;
                if (step1 > 1.0)
                    step1 = 1.0;
                step2 = 1.0 - step1;

                FunctGroupArray[species].Ccorr *= step2;

            }
            */

        }
    }

    // Sanity check on mortality
    for (species = 0; species < bm->K_num_tot_sp; species++) {
        for(cohort = 0; cohort < FunctGroupArray[species].numCohortsXnumGenes; cohort++){
            if (FunctGroupArray[species].contaminantSpMort[cohort] > 1.0)
                FunctGroupArray[species].contaminantSpMort[cohort] = 1.0;
        }
    }

    return 0;
}


/**
 * Calculate the impact the species current contaminant level has a spawning etc.
 *
 * This has to be done differently to how the temperature and salinity effects are calculated.
 * The contaiminant effects are due to the level of contaminant levels in the animal not the level in the surrounding water columns.
 *
 * So it needs to be calculated based on tracer values.
 */


void Calculate_Contaminant_Q10_Corrections(MSEBoxModel *bm, BoxLayerValues *boxLayerInfo, HABITAT_TYPES habitat) {
	int sp;
    //int cohort;
	//double sum;

	/* Update parameters */
	for (sp = 0; sp < bm->K_num_tot_sp; sp++) {

		/* Set the default value to 1.0 in case we don't set it otherwise. */
		FunctGroupArray[sp].Ccorr = 1.0;

        // TODO: Finish this as mortality and growth already added directly so what else to do?
		/* check group is active and present in this habitat.*
		if (FunctGroupArray[sp].speciesParams[flag_id] == TRUE && FunctGroupArray[sp].habitatCoeffs[habitat] > 0 ){
			FunctGroupArray[sp].Ccorr = 1.0;

			sum = 0;
			// I don't think this is going to work - need to work out a better way to do this *
			for(cohort = 0; cohort < FunctGroupArray[sp].numCohortsXnumGenes; cohort++){
				sum = sum + FunctGroupArray[sp].contaminantSpMort[cohort];
			}
			FunctGroupArray[sp].Ccorr = sum/FunctGroupArray[sp].numCohortsXnumGenes;
		}
         */
	}
}

/**
 * Placeholder for calculating the scalar to apply to settlers to take into account effect of contaminants
 * on the number of settlers.
 *
 * This might be a function that returns the scalar for a given group or we could calculate it for all groups.
 *
 */
double Calculate_Contaminant_Repro_Scalar(MSEBoxModel *bm, int species) {
    double recCcorr = 1.0;
    
    if (!bm->flag_contamMaternalTransfer) {
        /* Set the default value to 1.0 in case we don't set it otherwise. */
        recCcorr = 1.0;

        // What else to do - could be rescaling based on parental levels or water column levels to cause additional mortality
        // TODO: Finish this
    }
    
    return recCcorr;
}

void Age_Contaminants_Store(MSEBoxModel *bm, int sp, int cohort, int nextcid, double dennow, double this_p_ageup) {
    int cIndex, cid;
    double cGroupLevel;
    
    for (cIndex = 0; cIndex < bm->num_contaminants; cIndex++) {
        cid = FunctGroupArray[sp].contaminantTracers[cohort][cIndex];
        cGroupLevel = bm->boxes[bm->current_box].tr[bm->current_layer][cid];
        FunctGroupArray[sp].agingContam[nextcid][cIndex][bm->current_box][bm->current_layer] = cGroupLevel * this_p_ageup;
        // bm->boxes[bm->current_box].tr[bm->current_layer][cid] -= (cGroupLevel * this_p_ageup);
        // Store it here (but do not remove it yet) so transferred properly to next class with aging
        
    }
}

void Age_Contaminants_Update(MSEBoxModel *bm, int sp, int cohort, double denup, double dennow, double nextden, int ij, int k) {
    int cIndex, cid, cid_prev;
    //double cGroupLevel;
    
    for (cIndex = 0; cIndex < bm->num_contaminants; cIndex++) {
        cid = FunctGroupArray[sp].contaminantTracers[cohort][cIndex];
        cid_prev = FunctGroupArray[sp].contaminantTracers[cohort-1][cIndex];
        bm->boxes[ij].tr[k][cid] += FunctGroupArray[sp].agingContam[cohort][cIndex][ij][k];
        bm->boxes[ij].tr[k][cid_prev] -= FunctGroupArray[sp].agingContam[cohort][cIndex][ij][k];
        // Add and subtract stuff being transferred (need to add and subtract here as not moved elsewhere anymore for clarity sake)
        //TODO: Need to think about how to handle this if the new age group isn't present in this water column and layer...
    }
}

/* Contaminant transfer for migrants during aging */
void Age_MigrantContaminants_Update(MSEBoxModel *bm, int sp, int nextcid, int cohort, int mid, double oldden, double num_aging) {
    int cIndex;
    double origContam, newContam;
    
    // For species without live birth and maternal care
    for (cIndex = 0; cIndex < bm->num_contaminants; cIndex++) {
        origContam = MIGRATION[sp].contam[nextcid][mid][cIndex];
        newContam = MIGRATION[sp].contam[cohort][mid][cIndex];
        if(oldden > 0) {
            MIGRATION[sp].contam[nextcid][mid][cIndex] = (origContam * oldden + newContam * num_aging) / (num_aging + oldden + small_num);
        } else {
            MIGRATION[sp].contam[nextcid][mid][cIndex] = newContam;
        }
    }
}

/* Migration return of contaminants */
double ContaminantDirectRectuitMigrants(MSEBoxModel *bm, int sp, int n, int mid, double oldden, double num_returning) {
    double ans = 0.0;
    
    return ans;
}

/* Migration return of contaminants */
double ContaminantMigrationIn(MSEBoxModel *bm, int sp, int n, int mid, double oldden, double num_returning) {
    double ans = 0.0;
    
    return ans;
}

/* Migration of contaminants as animals leave */
double ContaminantMigrationOut(MSEBoxModel *bm, int sp, int cohort, int mid, double oldden, double num_leaving) {
    int cIndex;
    double newContam = 0.0, origContam;
    double ans = 0.0;
    
    // For species without live birth and maternal care
    for (cIndex = 0; cIndex < bm->num_contaminants; cIndex++) {
        origContam = MIGRATION[sp].contam[cohort][mid][cIndex];
        //newContam = ;
        if(oldden > 0) {
            MIGRATION[sp].contam[cohort][mid][cIndex] = (origContam * oldden + newContam * num_leaving) / (num_leaving + oldden + small_num);
        } else {
            MIGRATION[sp].contam[cohort][mid][cIndex] = newContam;
        }
    }
    return ans;
}


/* Larval contaminant transfer - for non mammals
 
 MTR = maternal_transfer_rate
 
 Contaminant_load_for_offpring = Contaminant_load_mother * MTR

 OR
 
 Contaminant_load_offpring = (Contaminant_conc_mother / weight_mother) * weight_offspring * MTR

*/
void Get_Parental_Contaminants(MSEBoxModel *bm, int sp, int cohort, int flagmother, double this_den) {
    int cIndex, cid;
    double cGroupLevel, maternal_transfer_rate;

   if (flagmother < 1) {  // Only those not using maternal care transfer here
       for (cIndex = 0; cIndex < bm->num_contaminants; cIndex++) {
           maternal_transfer_rate = bm->contaminantStructure[cIndex]->sp_maternal_transfer[sp];
           cid = FunctGroupArray[sp].contaminantTracers[cohort][cIndex];
           cGroupLevel = bm->boxes[bm->current_box].tr[bm->current_layer][cid];
           FunctGroupArray[sp].reprodContam[cIndex] += (cGroupLevel * maternal_transfer_rate);
           FunctGroupArray[sp].reprodContamCount[cIndex] += this_den;
           bm->boxes[bm->current_box].tr[bm->current_layer][cid] -= (cGroupLevel * maternal_transfer_rate);
           // As lost to offspring instantaneously (as would be in external spawners)
       }
    }
}

/* Store contaminants per offspring until needed */
void Store_Recruit_Contaminants(MSEBoxModel *bm, int sp, int stock_id, int ngene, int qid) {
    int cIndex;
    int flagmother = (int) (FunctGroupArray[sp].speciesParams[flagmother_id]);
    
    if (bm->flag_contamMaternalTransfer && (flagmother < 1)) {  // Only those not using maternal care transfer here
        for (cIndex = 0; cIndex < bm->num_contaminants; cIndex++) {
            EMBRYO[sp].Contam[stock_id][ngene][qid][cIndex] = FunctGroupArray[sp].reprodContam[cIndex] / FunctGroupArray[sp].reprodContamCount[cIndex];  // Average contributed across the entire spawning population
        }
    }
 
    return;
}

/* Now allocate to recruits */
void Get_Recruit_Contaminants(MSEBoxModel *bm, int sp, int stock_id, int ngene, int qid) {
    int cIndex;
    int flagmother = (int) (FunctGroupArray[sp].speciesParams[flagmother_id]);
    
    if (bm->flag_contamMaternalTransfer && (flagmother < 1)) {  // Only those not using maternal care transfer here
        for (cIndex = 0; cIndex < bm->num_contaminants; cIndex++) {
            EMBRYO[sp].AverageContam[ngene][cIndex] = EMBRYO[sp].Contam[stock_id][ngene][qid][cIndex];
        }
    }
    
    return;
}

/* Now spatially allocate contamination along with recruits - doing this one step at a time so stays
 * in sync with spawning and recruitment queues
 */
void Set_Recruit_Final_Contaminants(MSEBoxModel *bm, int wclayer, int sp, int ngene, int mid, int qid, int recruit_outside) {
    int cIndex;
    int flagmother = (int) (FunctGroupArray[sp].speciesParams[flagmother_id]);
    
    if (bm->flag_contamMaternalTransfer && (flagmother < 1)) {  // Only those not using maternal care transfer here
        for (cIndex = 0; cIndex < bm->num_contaminants; cIndex++) {
            if(!recruit_outside) {
                // Recruit inside model domain
                EMBRYO[sp].RecruitContam[bm->current_box][wclayer][ngene][qid][cIndex] = EMBRYO[sp].AverageContam[ngene][cIndex];
            } else {
                // Recruit into migration array
                MIGRATION[sp].RecruitContam[ngene][mid][cIndex] = EMBRYO[sp].AverageContam[ngene][cIndex];
            }
        }
    }
    return;
}

/* Neonate contaminant transfer for migrants */
void Get_SettlerMigrant_Contaminants(MSEBoxModel *bm, int sp, int ngene, int mid, double oldden, double newden) {
    int cIndex;
    double origContam, newContam;
    
    // For species without live birth and maternal care
    for (cIndex = 0; cIndex < bm->num_contaminants; cIndex++) {
        origContam = MIGRATION[sp].contam[ngene][mid][cIndex];
        newContam = MIGRATION[sp].RecruitContam[ngene][mid][cIndex];
        if(oldden > 0) {
            MIGRATION[sp].contam[ngene][mid][cIndex] = (origContam * oldden + newContam * newden) / (newden + oldden + small_num);
        } else {
            MIGRATION[sp].contam[ngene][mid][cIndex] = newContam;
        }
    }
}



/* Neonate contaminant transfer - likely not much use for fish so use contaminant scalar on recruits for them
 Contaminant_load_offpring = Contaminant load in mother * MTR
*/

void Get_Settler_Contaminants(MSEBoxModel *bm, int wclayer, int sp, int ngene, int qid, double recruitSPden) {
    int cIndex, cid;
    int flagmother = (int) (FunctGroupArray[sp].speciesParams[flagmother_id]);
    double this_reprodContam, maternal_transfer_rate, cGroupLevel;
    double start_n = FunctGroupArray[sp].speciesParams[age_mat_id];
    double end_n = (double)FunctGroupArray[sp].numCohortsXnumGenes;
    int cohort = (int)(floor(drandom(start_n, end_n))); // Find a random adult cohort to use
    
    if (flagmother > 0) {
        for (cIndex = 0; cIndex < bm->num_contaminants; cIndex++) {
            maternal_transfer_rate = bm->contaminantStructure[cIndex]->sp_maternal_transfer[sp];
            cid = FunctGroupArray[sp].contaminantTracers[cohort][cIndex];
            cGroupLevel = bm->boxes[bm->current_box].tr[bm->current_layer][cid];
            this_reprodContam = (cGroupLevel * maternal_transfer_rate); // As local transfer from mother to neonate at point of birth (to save book keeping)
            bm->boxes[bm->current_box].tr[bm->current_layer][cid] -= this_reprodContam;
            // Add to neonate
            EMBRYO[sp].SettlerContam[ngene][cIndex] = this_reprodContam;
        }
    } else {
        // For species without live birth and maternal care
        for (cIndex = 0; cIndex < bm->num_contaminants; cIndex++) {
            if (recruitSPden > 0) {
                EMBRYO[sp].SettlerContam[ngene][cIndex]= EMBRYO[sp].RecruitContam[bm->current_box][wclayer][ngene][qid][cIndex];
            }
        }
    }
}

/* Now update contaminant scofres for recruits */
void Apply_Settler_Contaminants(MSEBoxModel *bm, int sp, int cohort, double starting_num, double new_num, int qid){
    int cIndex, cid;
    double old_contam, diff_num;
    
    for (cIndex = 0; cIndex < bm->num_contaminants; cIndex++) {
        cid = FunctGroupArray[sp].contaminantTracers[cohort][cIndex];
        old_contam = bm->boxes[bm->current_box].tr[bm->current_layer][cid];
        diff_num = (new_num - starting_num);
        if (diff_num < 0.0) {
            diff_num = 0.0;
        }
        bm->boxes[bm->current_box].tr[bm->current_layer][cid] = (old_contam * starting_num + diff_num * EMBRYO[sp].SettlerContam[cohort][cIndex]) / (new_num + small_num);
    }
    return;
}

/* Contaminant transfer to offspring - for mammals due to sucking young
 Contaminant_load_offpring = Contaminant load in mother * MTR_milk
 */
void Get_Suckling_Contaminants(MSEBoxModel *bm, int sp, int ad_cohort, int juv_cohort) {
    int cIndex, ad_cid, juv_cid;
    double milk_transfer_rate, cGroupLevel, this_Contam;
    
    for (cIndex = 0; cIndex < bm->num_contaminants; cIndex++) {
        milk_transfer_rate = bm->contaminantStructure[cIndex]->sp_suckling_transfer[sp];
        ad_cid = FunctGroupArray[sp].contaminantTracers[ad_cohort][cIndex];
        cGroupLevel = bm->boxes[bm->current_box].tr[bm->current_layer][ad_cid];
        this_Contam += (cGroupLevel * milk_transfer_rate); // As local transfer from mother to neonate at point of birth (to save book keeping)
        bm->boxes[bm->current_box].tr[bm->current_layer][ad_cid] -= this_Contam;
        // Add to juvenile
        juv_cid = FunctGroupArray[sp].contaminantTracers[juv_cohort][cIndex];
        bm->boxes[bm->current_box].tr[bm->current_layer][juv_cid] += this_Contam;
    }
}



/**********************************************************************************************************************************************
 *
 *
 * Functions to record contact and deaths associated with a contaminant
 *
 *
 *
 ***************************************************************************************************************************************************************/

/**
 * Record a a death in a group due to a contaminant.
 *
 */
void Contaminant_Record_Death(MSEBoxModel *bm, int sp, int cohort, double amount){

    FunctGroupArray[sp].calcCLinearMort[cohort][ongoingC_id] += FunctGroupArray[sp].contaminantSpMort[cohort] * amount * FunctGroupArray[sp].speciesParams[Mdt_id];

//    if(FunctGroupArray[sp].calcCLinearMort[cohort][ongoingC_id] > 0 && sp == 41)
//        fprintf(bm->logFile, "%s - current = %e, amount = %e\n",
//            FunctGroupArray[sp].groupCode, FunctGroupArray[sp].calcCLinearMort[cohort][ongoingC_id], amount);

}

void Contaminant_Update_ContactMort_Record(MSEBoxModel *bm, int sp, int cohort){
    int k;
    int maxstock = FunctGroupArray[sp].numStocks;
    double totstock = small_num;

    for(k = 0; k < maxstock; k++){
        totstock += (bm->calcTrackedMort[sp][cohort][k][start_id]);
    }

    FunctGroupArray[sp].calcCLinearMort[cohort][finalC_id] += (FunctGroupArray[sp].calcCLinearMort[cohort][ongoingC_id]/totstock);
    FunctGroupArray[sp].calcCLinearMort[cohort][ongoingC_id] = 0.0;

}

void Contaminant_Init_Contact_Record(MSEBoxModel *bm){

    char fname[STRLEN];
    int sp, cohort;

    /** Create filename **/
    sprintf(fname, "%sContamContact.txt", bm->startfname);

    if(verbose){
      printf("Doing Contaminant_Init_Contact_Record for file %s\n", fname);
    }
    /** Create file **/
    if ((contaminantContactFile=Util_fopen(bm, fname, "w")) == NULL ){
      quit("Contaminant_Init_Contact_Record: Can't open %s\n",fname);
    }
    /** Column definitions **/
    fprintf(contaminantContactFile, "Time");

    /* Each group that is turned on.
     */
    for (sp = 0; sp < bm->K_num_tot_sp; sp++) {
        if (FunctGroupArray[sp].speciesParams[flag_id]) {
            for(cohort = 0; cohort < FunctGroupArray[sp].numCohortsXnumGenes; cohort++){
                fprintf(contaminantContactFile, " %s-%d", FunctGroupArray[sp].groupCode, cohort);
            }
        }
    }

    fprintf(contaminantContactFile, "\n");

}


void Contaminant_Write_Contact_Record(MSEBoxModel *bm){
    int sp, cohort;

    if(verbose){
      printf("Doing Contaminant_Write_Contact_Record\n");
    }

    if(!contaminantContactFile){
      Contaminant_Init_Contact_Record(bm);
    }

    fprintf(contaminantContactFile, "%e", bm->dayt);
            
    for (sp = 0; sp < bm->K_num_tot_sp; sp++) {
        if (FunctGroupArray[sp].speciesParams[flag_id]) {
            for(cohort = 0; cohort < FunctGroupArray[sp].numCohortsXnumGenes; cohort++){
                fprintf(contaminantContactFile, " %e", FunctGroupArray[sp].calcCLinearMort[cohort][finalC_id]);
            }

        }
    }
    fprintf(contaminantContactFile, "\n");
}

/**
 * Close the contaminant file.
 */
void Contaminant_Close_Contact_Record(MSEBoxModel *bm){
    Util_Close_Output_File(contaminantContactFile);

}

/**********************************************************************************************************************************************
 * Routine to check fux of contaminants - diagnostic
 */

void Check_Contam_Totals(MSEBoxModel *bm) {
    int cIndex, ij, k, sp, cohort, cid, pid;
    double amount_check, surface_layer;
    
    if(verbose){
      printf("Checking contaminant totals\n");
    }
    
    for (cIndex = 0; cIndex < bm->num_contaminants; cIndex++) {
        
        /**/
        //if (strcmp(bm->contaminantStructure[cIndex]->contaminant_name, "Phenanthrene") == 0) {
            fprintf(bm->logFile, "Time %e %s has total_input: %e\n", bm->dayt, bm->contaminantStructure[cIndex]->contaminant_name, bm->forceTracerInput[cIndex].total_input);
        //}
        /**/
        
        if(!bm->forceTracerInput[cIndex].total_input) {
            continue;  // Nothing to do so skip ahead
        }
        
        amount_check = 0;
        cid = bm->contaminantStructure[cIndex]->contaminant_tracer;
        
        for (ij = 0; ij < bm->nbox; ij++) {
            if (bm->boxes[ij].type != BOUNDARY) {
                surface_layer = bm->boxes[ij].sm.dz[0];
                
                /* Check freely moving amounts */
                for (k = 0; k < bm->boxes[ij].nz; k++) {
                    amount_check += bm->boxes[ij].tr[k][cid];
                    
                    /*
                    if (bm->boxes[ij].tr[k][cid] && (strcmp(bm->contaminantStructure[cIndex]->contaminant_name, "Phenanthrene") == 0)) {
                        fprintf(bm->logFile, "Time %e box%d-%d %s wc amt: %e (vs amount_check: %e)\n", bm->dayt, ij, k, bm->contaminantStructure[cIndex]->contaminant_name, bm->boxes[ij].tr[k][cid], amount_check);
                    }
                    */
                }
                for (k = 0; k < bm->boxes[ij].sm.nz; k++) {
                    amount_check += bm->boxes[ij].sm.tr[k][cid];
                    
                    /*
                    if (bm->boxes[ij].sm.tr[k][cid] && (strcmp(bm->contaminantStructure[cIndex]->contaminant_name, "Phenanthrene") == 0)) {
                            fprintf(bm->logFile, "Time %e box%d-%d %s sed amt: %e (vs amount_check: %e)\n", bm->dayt, ij, k, bm->contaminantStructure[cIndex]->contaminant_name, bm->boxes[ij].tr[k][cid], amount_check);
                    }
                    */
                }

                /* Check amount per species */
                for (sp = 0; sp < bm->K_num_tot_sp; sp++) {
                    if (FunctGroupArray[sp].speciesParams[flag_id]) {
                        for(cohort = 0; cohort < FunctGroupArray[sp].numCohortsXnumGenes; cohort++){
                            pid = FunctGroupArray[sp].contaminantTracers[cohort][cIndex];
                            
                            if (FunctGroupArray[sp].habitatType == EPIFAUNA) {
                                amount_check += (bm->boxes[ij].epi[pid] / (surface_layer + small_num));
                                
                                /*
                                if (bm->boxes[ij].epi[pid] && (strcmp(bm->contaminantStructure[cIndex]->contaminant_name, "Phenanthrene") == 0)) {
                                    fprintf(bm->logFile, "Time %e box%d %s in %s-%d epi amt: %e surface_layer: %e (vs amount_check: %e)\n", bm->dayt, ij, bm->contaminantStructure[cIndex]->contaminant_name, FunctGroupArray[sp].groupCode, cohort, bm->boxes[ij].epi[pid], surface_layer, amount_check);
                                }
                                 */
                                
                            } else {
                                for (k = 0; k < bm->boxes[ij].nz; k++) {
                                    amount_check += bm->boxes[ij].tr[k][pid];
                                    
                                    /*
                                    if (bm->boxes[ij].tr[k][pid] && (strcmp(bm->contaminantStructure[cIndex]->contaminant_name, "Phenanthrene") == 0)) {
                                        fprintf(bm->logFile, "Time %e box%d-%d %s in %s-%d wc amt: %e (vs amount_check: %e)\n", bm->dayt, ij, k, bm->contaminantStructure[cIndex]->contaminant_name, FunctGroupArray[sp].groupCode, cohort, bm->boxes[ij].tr[k][pid], amount_check);
                                    }
                                     */

                                }
                                for (k = 0; k < bm->boxes[ij].sm.nz; k++) {
                                    amount_check += bm->boxes[ij].sm.tr[k][pid];
                                    
                                    /*
                                    if (bm->boxes[ij].sm.tr[k][pid] && (strcmp(bm->contaminantStructure[cIndex]->contaminant_name, "Phenanthrene") == 0)) {
                                        fprintf(bm->logFile, "Time %e box%d-%d %s in %s-%d sed amt: %e (vs amount_check: %e)\n", bm->dayt, ij, k, bm->contaminantStructure[cIndex]->contaminant_name, FunctGroupArray[sp].groupCode, cohort, bm->boxes[ij].sm.tr[k][pid], amount_check);
                                    }
                                     */
                                }
                            }
                        }
                    }
                }
            }
        }
        
        if (amount_check > (bm->forceTracerInput[cIndex].total_input * buffer_tol)) {
        //if (amount_check > (bm->forceTracerInput[cIndex].total_input) && (strcmp(bm->contaminantStructure[cIndex]->contaminant_name, "Phenanthrene") == 0)) {
            fprintf(bm->logFile, "Time %e %s total_input: %e amount_check: %e\n", bm->dayt, bm->contaminantStructure[cIndex]->contaminant_name, bm->forceTracerInput[cIndex].total_input, amount_check);
            fflush(bm->logFile);
            quit("Check_Contam_Totals - %s amount (%e) has exceeded input (%e)\n",  bm->contaminantStructure[cIndex]->contaminant_name, amount_check, bm->forceTracerInput[cIndex].total_input);
            
        }
    }

    return;

}
