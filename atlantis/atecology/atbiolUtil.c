/**
 * \file
 * \brief The biology util functions.
 * \ingroup atEcology
 *
 *
 *	Changes:
 *
 *	01-04-2009 Bec Gorton
 *
 *	Added code to intialise the FunctGroupArray[guild].dead[cohort] value in InitInvertProperties.
 *	Also added code to set FunctGroupArray[guild].deadGlobal[cohort] to 0 if it_count ==1. valgrind
 *	was spitting the dummy as these values were not intialised if fishing is not turned on.
 *
 *
 *	06-04-2009 Bec Gorton
 *	Added a line in the InitInvertProperties that will set the dead value to 0.
 *
 *	04-05-2010 Bec Gorton
 *	Added the new Ecology_Get_Species_LMort function that will return a species scaled linear mortality value.
 *
 *
 */
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>
#include <sjwlib.h>
#include "atecology.h"
#include "atExternalScalar.h"



/* Bec test comment */

/**
 * \brief Initialise the invertProps and processProps
 * for the given functional group.
 *
 * This is called for each invertebrate in the model
 * at the start of Water_Column_Box, Sediment_Box and Epibenthic_Box.
 *
 *
 *
 */
void Init_Invert_Properties(MSEBoxModel *bm){

	int guild;
	int cohort;

	for (guild = 0; guild < bm->K_num_tot_sp; guild++) {
		if (FunctGroupArray[guild].isVertebrate == FALSE){
            /* Primary Production Properties */
			FunctGroupArray[guild].chl = 0.0;
			//FunctGroupArray[guild].hN = 0.0;
			FunctGroupArray[guild].maxPhagotrophy = 0.0;

			FunctGroupArray[guild].uptakeDR = 0.0;
			FunctGroupArray[guild].uptakeDL = 0.0;

			FunctGroupArray[guild].nitrif = 0.0;
			FunctGroupArray[guild].remin = 0.0;
			FunctGroupArray[guild].brokenDown = 0.0;
			FunctGroupArray[guild].solDON = 0.0;

			/* Cohort dependant properties */
			for(cohort = 0; cohort < FunctGroupArray[guild].numCohortsXnumGenes; cohort++){
				FunctGroupArray[guild].CLEAR[cohort] = 0.0;
				FunctGroupArray[guild].dead[cohort] = 0.0;
				FunctGroupArray[guild].GrazeLive[cohort]= 0.0;
				FunctGroupArray[guild].growth[cohort] = 0.0;
				FunctGroupArray[guild].mortality[cohort]= 0.0;
				FunctGroupArray[guild].grazing[cohort]= 0.0;
				FunctGroupArray[guild].lysis[cohort] = 0.0;
				FunctGroupArray[guild].prodnDR[cohort]  = 0.0;
				FunctGroupArray[guild].prodnDL[cohort]  = 0.0;
				FunctGroupArray[guild].releaseNH[cohort] = 0.0;

				if(it_count == 1){
					FunctGroupArray[guild].deadGlobal[cohort] = 0.0;
				}

				FunctGroupArray[guild].transDR[cohort] = 0.0;
				FunctGroupArray[guild].uptakeNH[cohort] = 0.0;
			}
		}
	}
}

/**
 *  \brief Get initial values for vertebrate vectors.
 *
 *
 */
void Set_VERTinfo(MSEBoxModel *bm, double *localWCTracers, FILE *llogfp)
{
    int fgIndex, cohort;

    for(fgIndex = 0; fgIndex < bm->K_num_tot_sp; fgIndex++){
    	if(FunctGroupArray[fgIndex].speciesParams[flag_id]){
			if(FunctGroupArray[fgIndex].isVertebrate == TRUE){
				for(cohort = 0; cohort < FunctGroupArray[fgIndex].numCohortsXnumGenes; cohort++){
					VERTinfo[fgIndex][cohort][SN_id] = localWCTracers[FunctGroupArray[fgIndex].structNTracers[cohort]];
					VERTinfo[fgIndex][cohort][RN_id] = localWCTracers[FunctGroupArray[fgIndex].resNTracers[cohort]];
					VERTinfo[fgIndex][cohort][DEN_id] = localWCTracers[FunctGroupArray[fgIndex].NumsTracers[cohort]];
                    
					if ((it_count == 1) && (bm->debug == debug_biology_process) && (bm->dayt >= bm->checkstart) && (bm->dayt < bm->checkstop)) {
                    //if ((bm->which_check == fgIndex) && (bm->dayt == 117)){
						fprintf(llogfp, "Time: %e, box%d-%d, VERTinfo[%s][%d][SN_id] = %.20e\n", bm->dayt, bm->current_box, bm->current_layer, FunctGroupArray[fgIndex].groupCode, cohort, VERTinfo[fgIndex][cohort][SN_id]);
						fprintf(llogfp, "Time: %e, box%d-%d, VERTinfo[%s][%d][RN_id] = %.20e\n", bm->dayt, bm->current_box, bm->current_layer, FunctGroupArray[fgIndex].groupCode, cohort, VERTinfo[fgIndex][cohort][RN_id]);
						fprintf(llogfp, "Time: %e, box%d-%d, VERTinfo[%s][%d][DEN_id] = %.20e\n", bm->dayt, bm->current_box, bm->current_layer, FunctGroupArray[fgIndex].groupCode, cohort, VERTinfo[fgIndex][cohort][DEN_id]);
					}
				}
			}
    	}
    }
}

/**
 * Add code to scale linear mortality for each group in each box.
 *
 *	Max_Num_Mort_Changes
 * MortScale[sp][ageclass][b][index][start_id]
 * MortScale[sp][ageclass][b][index][period_id]
 * MortScale[sp][ageclass][b][index][end_id]
 */
void Scale_Group_Linear_Mortality(MSEBoxModel *bm, int speciesIndex, int cohort, int box, double start, double period, double mult, FILE *llogfp) {

	double *****tempArray;
	double *****tempArrayPointer;
	int i;

	fprintf(llogfp, "Scale_Linear_Mortality: Scaling %s:%d in box %d by mult %e from %e over %e\n", FunctGroupArray[speciesIndex].groupCode, cohort, box, mult, start, period);
    
	/* Increase the number of scaling events */
	numMortChanges[speciesIndex][cohort][box]++;

	printf("maxMortChange = %d\n", maxMortChange);
	if (((int) numMortChanges[speciesIndex][cohort][box]) > maxMortChange) {
		maxMortChange = numMortChanges[speciesIndex][cohort][box];
	}
	/* Allocate a new array */
	tempArray = Util_Alloc_Init_5D_Double(3, maxMortChange, bm->nbox, 2, bm->K_num_tot_sp, 0);

	/* Copy all existing values across */
//	memcpy(tempArray, LinearMortChange, sizeof(LinearMortChange));

	/* Now insert the new value in the correct place */
	Util_Copy_Change_Values(bm, LinearMortChange[speciesIndex][cohort][box],
			tempArray[speciesIndex][cohort][box],
			numMortChanges[speciesIndex][cohort][box],
			start, period, mult);

	/* Now do some pointer magic and free up the 'old' array*/
	tempArrayPointer = LinearMortChange;
	LinearMortChange = tempArray;
	free5d(tempArrayPointer);

	for(i = 0; i < ((int) numMortChanges[speciesIndex][cohort][box]); i++){
		fprintf(llogfp, "LinearMortChange[speciesIndex][cohort][box][%d][start_id] = %e\n", i, LinearMortChange[speciesIndex][cohort][box][i][start_id]);
		fprintf(llogfp, "LinearMortChange[speciesIndex][cohort][box][%d][period_id] = %e\n", i, LinearMortChange[speciesIndex][cohort][box][i][period_id]);
		fprintf(llogfp, "LinearMortChange[speciesIndex][cohort][box][%d][mult_id] = %e\n", i, LinearMortChange[speciesIndex][cohort][box][i][mult_id]);
	}

}



/**
 * \brief Scale the specified habitat.
 */
void Scale_Habitat(MSEBoxModel *bm, char *name, int box, double start, double period, double mult, FILE *llogfp) {

	int habitatIndex = -1;
	double ***tempArray;
	double ***tempArrayPointer;
	int i;

	fprintf(llogfp, "Scale_Habitat: Scaling %s in box %d by mult %e from %e over %e\n", name, box, mult, start, period);

	/* First - work out which habitat we are changing */
	if (strcmp(name, "REEF") == 0) {
		habitatIndex = reef_id;
	} else if (strcmp(name, "FLAT") == 0) {
		habitatIndex = flat_id;
	} else if (strcmp(name, "SOFT") == 0) {
		habitatIndex = soft_id;
	} else {
		quit("Scale_Habitat: Habitat %s is not recognised. Must be either REEF, FLAT or SOFT\n", name);
	}

	/* Turn on habitat degradation in this box */
	Box_degradedi[box] = TRUE;
	bm->flagdegrade = TRUE;

	/* Check to see if there is just a single dummy value in the array - if so just overwrite it */
	if(BEDchange_max_num == 1 && BEDchange[habitatIndex][0][start_id] == 0){
		BEDchange[habitatIndex][0][start_id] = start + bm->dayt;
		BEDchange[habitatIndex][0][period_id] = period;
		BEDchange[habitatIndex][0][mult_id] = mult;
		return;
	}
	/* Otherwise increase the size and insert it into the correct location */
	BEDchange_max_num++;

	fprintf(llogfp, "Scale_Habitat: Scaling %s:%d in box %d by mult %e from %e over %e\n", name, habitatIndex, box, mult,
				start, period);

	/* Now allocate a temp array to store the updated values */
	tempArray = Util_Alloc_Init_3D_Double(3, BEDchange_max_num, bm->K_num_bed_types, 0);

	/* Now copy the values across inserting the new value in the correct place.*/
	Util_Copy_Change_Values(bm, BEDchange[habitatIndex], tempArray[habitatIndex],
			BEDchange_max_num, start, period, mult);

	/* Now do some pointer magic and free up the 'old' array*/
	tempArrayPointer = BEDchange;
	BEDchange = tempArray;
	free3d(tempArrayPointer);

	for(i = 0; i < BEDchange_max_num; i++){
		fprintf(llogfp, "BEDchange[habitatIndex][%d][start_id] = %e\n", i, BEDchange[habitatIndex][i][start_id]);
		fprintf(llogfp, "BEDchange[habitatIndex][%d][period_id] = %e\n", i, BEDchange[habitatIndex][i][period_id]);
		fprintf(llogfp, "BEDchange[habitatIndex][%d][mult_id] = %e\n", i, BEDchange[habitatIndex][i][mult_id]);
	}

}

/**
 * \brief Get the biomass correction to add to the biomass of a group eaten when its added to the calcMnumPerPred array.
 *
 */
double Get_Biomass_Correction(MSEBoxModel *bm, int sp, HABITAT_TYPES habitatType){
	double biomass_correction = 0.0;

	switch(habitatType) {
		case WC:
			biomass_correction =  bm->boxes[bm->current_box].dz[bm->current_layer] * bm->boxes[bm->current_box].area;
			break;
		case SED:
			biomass_correction = bm->boxes[bm->current_box].sm.dz[bm->current_layer] * bm->boxes[bm->current_box].area;
			break;
		case EPIFAUNA:
			biomass_correction = bm->boxes[bm->current_box].area;
			break;
		case LAND_BASED:
			biomass_correction = bm->boxes[bm->current_box].area;
			break;
		case ICE_BASED:
			biomass_correction =  bm->boxes[bm->current_box].ice.dz[bm->current_icelayer] * bm->boxes[bm->current_box].area;
			break;
        case MIXED:
            quit("How did we get here as should come through a primary habitat\n");
            break;
        default:
            quit("How did we get here as this isn't even a habitat type slot but nTotHabTypes\n");
            break;
	}
	return biomass_correction;
}


/**
 *
 *	\brief A general way of grabbing the current tracers.
 *
 *
 *
 */
double *getTracerArray(BoxLayerValues *boxLayerInfo, HABITAT_TYPES habitatType) {
	double *tracerArray = NULL;

	switch (habitatType) {
	case WC:
		tracerArray = boxLayerInfo->localWCTracers;
		break;
	case SED:
		tracerArray = boxLayerInfo->localSEDTracers;
		break;
	case EPIFAUNA:
		tracerArray = boxLayerInfo->localEPITracers;
		break;
	case LAND_BASED:
		tracerArray = boxLayerInfo->localLANDTracers;
		break;
	case ICE_BASED:
		tracerArray = boxLayerInfo->localICETracers;
		break;
    case MIXED:
        quit("How did we get here as should come through a primary habitat\n");
        break;
    default:
        quit("How did we get here as this isn't even a habitat type slot but nTotHabTypes\n");
        break;
	}
	return tracerArray;

}


/**
 * Get the linear mortality of a group. If external scaling is enable then the returned value will be scaled.
 * 
 * 
 */
double Ecology_Get_Linear_Mortality(MSEBoxModel *bm, int sp, int cohort, int isAdult){
	double scalar = 1.0;
	double mL = FunctGroupArray[sp].cohortSpeciesParams[isAdult][mL_id];
    
	if(bm->track_contaminants){
		mL = mL + FunctGroupArray[sp].contaminantSpMort[cohort];
		/*if(FunctGroupArray[sp].contaminantSpMort[cohort] != 0)
			fprintf(bm->logFile, "FunctGroupArray[%s].contaminantSpMort[%d] = %e\n", FunctGroupArray[sp].groupCode, cohort, FunctGroupArray[sp].contaminantSpMort[cohort]);*/
		
	}

	if (bm->use_external_scaling && bm->scale_all_mortality == FALSE) {
		if(bm->externalBiologyInput->Species_Cohort_Lookup[sp][cohort][MORT_INDEX_ID] != -1){
			scalar = Get_Group_Scalar(bm, bm->externalBiologyInput, bm->externalBiologyInput->Species_Cohort_Lookup[sp][cohort][MORT_INDEX_ID], bm->current_box, bm->current_layer);

			if(bm->mortality_addition == TRUE){
				mL = mL + (scalar/86400.0);	/* Divide by 86400 to get value per seconds */

                //fprintf(bm->logFile, "Time: %e box %d, layer %d, mL = %e, addition= %e\n", bm->dayt, bm->current_box, bm->current_layer, mL, scalar);

			} else {
				mL = mL * scalar;
                
                //fprintf(bm->logFile, "Time: %e %s-%d box %d, layer %d, mL = %e (vs %e), scalar= %e\n", bm->dayt, FunctGroupArray[sp].groupCode, cohort, bm->current_box, bm->current_layer, mL, FunctGroupArray[sp].cohortSpeciesParams[isAdult][mL_id], scalar);
                
			}
		}
	}
    
	return mL;
}


/**
 * Get the linear mortality of a group. If external scaling is enable then the returned value will be scaled.
 *
 *
 */
double Ecology_Get_Mortality_Scalar(MSEBoxModel *bm, int sp, int cohort){
	double scalar = 1.0;

	if (bm->use_external_scaling && bm->scale_all_mortality == TRUE) {
		if(bm->externalBiologyInput->Species_Cohort_Lookup[sp][cohort][MORT_INDEX_ID] != -1){
			scalar = Get_Group_Scalar(bm, bm->externalBiologyInput, bm->externalBiologyInput->Species_Cohort_Lookup[sp][cohort][MORT_INDEX_ID], bm->current_box, bm->current_layer);

			return scalar;
		}
	}
	return 1.0;
}

/**
 * Get the growth scalar for this group. If no external scaling is enabled the return value is 1.0.
 * 
 */
double Ecology_Get_Growth_Scalar(MSEBoxModel *bm, int sp, int cohort){
	double scalar = 1.0;

	if (bm->use_external_scaling) {

		if(bm->externalBiologyInput->Species_Cohort_Lookup[sp][cohort][GROWTH_INDEX_ID] != -1){
			scalar = Get_Group_Scalar(bm, bm->externalBiologyInput, bm->externalBiologyInput->Species_Cohort_Lookup[sp][cohort][GROWTH_INDEX_ID], bm->current_box, bm->current_layer);

			/*if(bm->current_box == 1 && bm->current_layer == 0 && it_count == 1 && sp == 78)
				fprintf(bm->logFile, "growth rate scalar - time = %f, %s:%d, box %d:%d - scalar = %e,  it_count = %d\n",
						bm->dayt, FunctGroupArray[sp].groupCode, cohort, bm->current_box, bm->current_layer, scalar, it_count);

						*/

		}
	}
	return scalar;
}

/**
 * Get the recruitment scalar for this group. If no external scaling is enabled the return value is 1.0.
 * 
 */
double Ecology_Get_Recruitment_Scalar(MSEBoxModel *bm, int sp){
	double scalar = 1.0;

	if (bm->use_external_scaling) {
		if(bm->externalBiologyInput->Species_Cohort_Lookup[sp][0][RECRUITMENT_INDEX_ID] != -1){
			scalar = Get_Group_Scalar(bm, bm->externalBiologyInput, bm->externalBiologyInput->Species_Cohort_Lookup[sp][0][RECRUITMENT_INDEX_ID], bm->current_box, bm->current_layer);
		}
	}
	return scalar;
}
