/**
 \file
 \brief:	Contains routines that are used to set up the tracer arrays before the
 initial values are read in.
 \ingroup atEcology

 File:		atBuildTracer.c
 Created:	26/5/2008
 Author:		Bec Gorton,   CSIRO Marine Laboratories,  2008
 Based on work by Beth Fulton.





 Revisions: 	14-06-3008 Bec Gorton

 Added code to build the bm->dinfo array from the functional groups
 as specified in the functional group input file.
 This means that the diagnamelist array is no longer used, also the
 diagnostic indeices are also no longer used and have been deleted.

 03-02-2009 Bec Gorton
 Added a is not NULL check in the freeTracerArray code before the memory allocated
 for the tracer string is freed.

 14-04-2009 Bec Gorton
 Changed the order that they silicon tracers are set so they are in the same order as the new code.

 29-01-2010 Bec Gorton
 Added the Ecology_Add_FStat_Tracer so that the harvest library can build the list of fishery tracers.
 
 08-02-2012 Bec Gorton
 Removed the check that was only building tracers for groups that are turned on. If we do this
 some of the tracer are not reset to 0 when they should be and we get values written out
 to the netcdf file for groups that are not turned on.
 
 17-04-2013 Bec Gorton
 ATL-346 - Added support of tracking P:N:C ratios in groups.

 **/

/*  Open library routines */
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>
#include <sjwlib.h>
#include "atecology.h"

extern Namelist *trnamelist;
extern Namelist *epinamelist;
extern Namelist  *landnamelist;
extern int nEpiSize;
extern int nTrSize;
extern int nLandSize;

/**
 *\brief Function to add a tracer to a given tracer array
 *
 *
 *	@param tracerArray - the tracer array to new tracer to
 *	@param index - the next position in the tracer array
 *	@param name - the name of the new tracer
 *	@param tracerIndex - the pointer to the tracer index. This is used to reference the
 *	tracer in the various tracer arrays.
 *	@flux
 *	@tol
 *	@bio
 *
 */
static void Add_Tracer(Namelist **tracerArray, int index, char *name, int *tracerIndex, int flux, int tol, int bio, int contam) {
	if (name == NULL) {
		(*tracerArray)[index].index = NULL;
	} else {
		strcpy((*tracerArray)[index].name, name);
		(*tracerArray)[index].index = tracerIndex;
	}

	(*tracerArray)[index].Bio = bio;
	(*tracerArray)[index].Flux = flux;
	(*tracerArray)[index].Tol = tol;
    (*tracerArray)[index].Contam = contam;
    
}

/**
 * \brief Free the trnamelist tracer array.
 *
 *
 */
void Tracer_Array_Free(MSEBoxModel *bm) {

	free(trnamelist);
}

/**
 * \brief free the epinamelist tracer array including the name strings
 * that were allocated
 */

void Epi_Tracer_Array_Free(MSEBoxModel *bm) {
	int index;

	for (index = 0; index <= bm->nepi; index++) {
		if (!epinamelist[index].name)
			free(epinamelist[index].name);
	}
	free(epinamelist);
}

/**
 * \brief Free the trnamelist tracer array.
 *
 *
 */
void Land_Tracer_Array_Free(MSEBoxModel *bm) {

	free(landnamelist);
}

/*

 {"water", &Water_i, 0, 0, 0},
 {"salt", &Salinity_i, 0, 0, 0},
 {"Light_Adaptn_PL", &Light_Adaptn_PL_i, 0, 0, 0},
 {"Light_Adaptn_PS", &Light_Adaptn_PS_i, 0, 0, 0},
 {"Light_Adaptn_MB", &Light_Adaptn_MB_i, 0, 0, 0},

 {"NH3", &NH3_i, 0, 1, 0},
 {"NO3", &NO3_i, 0, 1, 0},
 {"DON", &DON_i, 0, 1, 0},
 {"MicroNut", &MicroNut_i, 0, 1, 1},
 {"Oxygen", &Oxygen_i, 0, 0, 0},
 {"Si",  &Si_i, 0, 1, 0},
 {"Det_Si",   &Det_Si_i, 0, 1, 0},
 {"Light", &Light_i, 0, 0, 0},
 {"Temp", &Temp_i, 0, 0, 0},
 {"Denitrifiction",    &Denitrification_i, 1, 0, 0},
 {"Nitrification",    &Nitrification_i, 1, 0, 0},
 {"Chl_a",    &Chl_a_i, 1, 0, 0},


 {"Stress", &Stress_i, 1, 0, 0},
 {"DiagNGain", &DiagNGain_i, 1, 0, 0},
 {"DiagNLoss", &DiagNLoss_i, 1, 0, 0},
 {"DiagNFlux", &DiagNFlux_i, 1, 0, 0},


 */

/**
 *	\brief Build the trenamelist tracer array.
 *
 *
 */
void Build_TrName_NameList(MSEBoxModel *bm) {
	int fgIndex, cohort, contamIndex;
	int size = 0;
	int index = 0;
	char str[STRLEN*2];

	/* work out the number of tracers that should be found */
	size = bm->K_num_physiochem + 2 + 1;

	for (fgIndex = 0; fgIndex < bm->K_num_tot_sp; fgIndex++) {
		// add the totNTracers
		if (FunctGroupArray[fgIndex].isVertebrate) {
			size += 1; /* _N */
			size += (FunctGroupArray[fgIndex].numCohortsXnumGenes * 3); /* StructN, ResN, Nums */
		} else {
			if (FunctGroupArray[fgIndex].groupType == SM_PHY || FunctGroupArray[fgIndex].groupType == LG_PHY
					|| FunctGroupArray[fgIndex].groupType == MICROPHTYBENTHOS) {
				size++; /* Light adaptation */
			}
			if (FunctGroupArray[fgIndex].groupType == DINOFLAG && FunctGroupArray[fgIndex].speciesParams[flag_id]) {
				size++; /* Light adaptation */
			}
			if (FunctGroupArray[fgIndex].habitatType != EPIFAUNA) {
				size += FunctGroupArray[fgIndex].numCohortsXnumGenes; /* _N */
			}
			if (FunctGroupArray[fgIndex].isSiliconDependant && (FunctGroupArray[fgIndex].groupType != SPONGE)) {
				size++; /* Silicon tracers */
			}
		}

	}

	/* If we are tracking P:N ratios add the correct number of tracers for each group that is turned on */
	if(bm->track_atomic_ratio == TRUE){
		for (fgIndex = 0; fgIndex < bm->K_num_tot_sp; fgIndex++) {
			if (FunctGroupArray[fgIndex].speciesParams[flag_id] == TRUE) {
				if (FunctGroupArray[fgIndex].habitatType != EPIFAUNA) {
					size += (FunctGroupArray[fgIndex].numCohortsXnumGenes * 2) ; /* P and C */
				}
			}
		}
	}

	/* If we are tracking contaiminants then add those as well */
	if(bm->track_contaminants == TRUE){
		for (fgIndex = 0; fgIndex < bm->K_num_tot_sp; fgIndex++) {
			if (FunctGroupArray[fgIndex].speciesParams[flag_id] == TRUE) {
				if (FunctGroupArray[fgIndex].habitatType != EPIFAUNA) {
					size += (FunctGroupArray[fgIndex].numCohorts * bm->num_contaminants * 2); /* a tracer per contaminant - entry for contaminant and for prop */
				}
			}
		}
		size = size + bm->num_contaminants;
	}
    
    if(bm->flag_pollutant_impacts == TRUE){
        size += 2;  // Adding light and noise
    }

	trnamelist = (Namelist *) malloc((size_t)size * sizeof(Namelist));
	nTrSize = size - 1;

	Add_Tracer(&trnamelist, index++, "water", &Water_i, 0, 0, 0, 0);
	Add_Tracer(&trnamelist, index++, "salt", &Salinity_i, 0, 0, 0, 0);
    
    if(bm->flag_pollutant_impacts == TRUE){
        Add_Tracer(&trnamelist, index++, "noise_pollution", &Noise_Pollution_i, 0, 0, 0, 0);
        Add_Tracer(&trnamelist, index++, "light_pollution", &Light_Pollution_i, 0, 0, 0, 0);
    }

	/* Light adaptation tracers */
	for (fgIndex = 0; fgIndex < bm->K_num_tot_sp; fgIndex++) {
		if (FunctGroupArray[fgIndex].groupType == SM_PHY || FunctGroupArray[fgIndex].groupType == LG_PHY
				|| FunctGroupArray[fgIndex].groupType == MICROPHTYBENTHOS) {
			sprintf(str, "Light_Adaptn_%s", FunctGroupArray[fgIndex].groupCode);
			Add_Tracer(&trnamelist, index++, str, &FunctGroupArray[fgIndex].Light_Adaptn_TracerID, 0, 0, 0, 0);
		}
		if (FunctGroupArray[fgIndex].groupType == DINOFLAG && FunctGroupArray[fgIndex].speciesParams[flag_id]) {
			sprintf(str, "Light_Adaptn_%s", FunctGroupArray[fgIndex].groupCode);
			Add_Tracer(&trnamelist, index++, str, &FunctGroupArray[fgIndex].Light_Adaptn_TracerID, 0, 0, 0, 0);

		}
	}

	/* Add the totN tracers */
	for (fgIndex = 0; fgIndex < bm->K_num_tot_sp; fgIndex++) {
		if (FunctGroupArray[fgIndex].habitatType != EPIFAUNA) {

			switch (FunctGroupArray[fgIndex].groupAgeType) {
			case AGE_STRUCTURED:
				sprintf(str, "%s_N", FunctGroupArray[fgIndex].name);

				Add_Tracer(&trnamelist, index++, str, &FunctGroupArray[fgIndex].totNTracers[0], 1, 0, 0, 0);

				break;
			case AGE_STRUCTURED_BIOMASS:
				for (cohort = 0; cohort < FunctGroupArray[fgIndex].numCohortsXnumGenes; cohort++) {
					sprintf(str, "%s_N%d", FunctGroupArray[fgIndex].name, cohort + 1);

					Add_Tracer(&trnamelist, index++, str, &FunctGroupArray[fgIndex].totNTracers[cohort], 0, FunctGroupArray[fgIndex].diagTol, 1, 0);
				}
				break;
			case BIOMASS:

				/* Bit of a hack for now so older models will work */
				//				if (FunctGroupArray[fgIndex].groupType == CEP || FunctGroupArray[fgIndex].groupType == PWN){
				//					sprintf(str, "%s_N2", FunctGroupArray[fgIndex].name);
				//					Add_Tracer(&trnamelist, index++, str, &FunctGroupArray[fgIndex].totNTracers[0], 0, FunctGroupArray[fgIndex].diagTol, 1);
				//
				//					sprintf(str, "%s_N1", FunctGroupArray[fgIndex].name);
				//					Add_Tracer(&trnamelist, index++, str, &FunctGroupArray[fgIndex].totNTracers[1], 0, FunctGroupArray[fgIndex].diagTol, 1);
				//					continue;
				//				}else{
				sprintf(str, "%s_N", FunctGroupArray[fgIndex].name);
				//}

				if (FunctGroupArray[fgIndex].groupType == CARRION) {
					Add_Tracer(&trnamelist, index++, str, &FunctGroupArray[fgIndex].totNTracers[0], 0, 3, 0, 0);

				} else if (FunctGroupArray[fgIndex].groupType == LAB_DET || FunctGroupArray[fgIndex].groupType == REF_DET) {
					Add_Tracer(&trnamelist, index++, str, &FunctGroupArray[fgIndex].totNTracers[0], 0, 1, 0, 0);
				} else {
					Add_Tracer(&trnamelist, index++, str, &FunctGroupArray[fgIndex].totNTracers[0], 0, FunctGroupArray[fgIndex].diagTol, 1, 0);
				}

				//printf(" Added tracer for %s\n", str);
				break;

			}
		}

	}

	/* Add the silicon tracers */
	for (fgIndex = 0; fgIndex < bm->K_num_tot_sp; fgIndex++) {
		if (FunctGroupArray[fgIndex].isSiliconDependant && (FunctGroupArray[fgIndex].groupType != SPONGE)) {
			sprintf(str, "%s_S", FunctGroupArray[fgIndex].name);
			Add_Tracer(&trnamelist, index++, str, &FunctGroupArray[fgIndex].secondNutrientTracerIndex, 0, 1, 1, 0);
			FunctGroupArray[fgIndex].secondNutrient = X_SiN;
		}

	}

	/* The rest of the vertebrate tracers
	 * Do in this order to the file structure is similar to the original format
	 */
	for (fgIndex = 0; fgIndex < bm->K_num_tot_sp; fgIndex++) {
		// add the totNTracers
		if (FunctGroupArray[fgIndex].isVertebrate == TRUE) {
			for (cohort = 0; cohort < FunctGroupArray[fgIndex].numCohortsXnumGenes; cohort++) {

				sprintf(str, "%s%d_StructN", FunctGroupArray[fgIndex].name, cohort + 1);
				Add_Tracer(&trnamelist, index++, str, &FunctGroupArray[fgIndex].structNTracers[cohort], 0, 2, 1, 0);
			}
		}

	}
	for (fgIndex = 0; fgIndex < bm->K_num_tot_sp; fgIndex++) {
		// add the totNTracers
		if (FunctGroupArray[fgIndex].isVertebrate == TRUE) {
			for (cohort = 0; cohort < FunctGroupArray[fgIndex].numCohortsXnumGenes; cohort++) {

				sprintf(str, "%s%d_ResN", FunctGroupArray[fgIndex].name, cohort + 1);
				Add_Tracer(&trnamelist, index++, str, &FunctGroupArray[fgIndex].resNTracers[cohort], 0, 2, 1, 0);
			}
		}

	}
	for (fgIndex = 0; fgIndex < bm->K_num_tot_sp; fgIndex++) {
		// add the totNTracers
		if (FunctGroupArray[fgIndex].isVertebrate == TRUE) {
			for (cohort = 0; cohort < FunctGroupArray[fgIndex].numCohortsXnumGenes; cohort++) {

				sprintf(str, "%s%d_Nums", FunctGroupArray[fgIndex].name, cohort + 1);
				Add_Tracer(&trnamelist, index++, str, &FunctGroupArray[fgIndex].NumsTracers[cohort], 0, 3, 1, 0);
			}
		}

	}

	/* add physical and nutrient tracers */
	for (fgIndex = 0; fgIndex < bm->K_num_physiochem; fgIndex++) {
		if (strcmp(PhysioChemArray[fgIndex].name, "") != 0) {
			sprintf(str, "%s", PhysioChemArray[fgIndex].name);
			Add_Tracer(&trnamelist, index++, str, PhysioChemArray[fgIndex].tracerIndex, PhysioChemArray[fgIndex].diagFlux, PhysioChemArray[fgIndex].diagTol,
					PhysioChemArray[fgIndex].diagBiol, 0);
		}
	}


	/* Add additional tracers for each group */
	if(bm->track_atomic_ratio == TRUE){
		for (fgIndex = 0; fgIndex < bm->K_num_tot_sp; fgIndex++) {
			if (FunctGroupArray[fgIndex].speciesParams[flag_id] == TRUE) {
				if (FunctGroupArray[fgIndex].habitatType != EPIFAUNA) {

					if((FunctGroupArray[fgIndex].numCohortsXnumGenes) > 1){
						for(cohort = 0; cohort < FunctGroupArray[fgIndex].numCohortsXnumGenes; cohort++){
							sprintf(str, "N_to_P_%s%d", FunctGroupArray[fgIndex].name, cohort + 1);
							Add_Tracer(&trnamelist, index++, str, &FunctGroupArray[fgIndex].addRatioTracers[cohort][p_id], 2, FunctGroupArray[fgIndex].diagTol, 0, 0);

							sprintf(str, "N_to_C_%s%d", FunctGroupArray[fgIndex].name, cohort + 1);
							Add_Tracer(&trnamelist, index++, str, &FunctGroupArray[fgIndex].addRatioTracers[cohort][c_id], 2, FunctGroupArray[fgIndex].diagTol, 0, 0);

						}
					}else{
						sprintf(str, "N_to_P_%s", FunctGroupArray[fgIndex].name);
						Add_Tracer(&trnamelist, index++, str, &FunctGroupArray[fgIndex].addRatioTracers[0][p_id], 2, FunctGroupArray[fgIndex].diagTol, 0, 0);

						sprintf(str, "N_to_C_%s", FunctGroupArray[fgIndex].name);
						Add_Tracer(&trnamelist, index++, str, &FunctGroupArray[fgIndex].addRatioTracers[0][c_id], 2, FunctGroupArray[fgIndex].diagTol, 0, 0);
					}
				}
			}
		}
	}

	/* Add contaminant tracers for each group */
	if (bm->track_contaminants == TRUE) {
        // Instead of using FunctGroupArray[fgIndex].diagTol use 0 as need to allow for spill shocks to occur
		for(contamIndex = 0; contamIndex < bm->num_contaminants; contamIndex++){
			for (fgIndex = 0; fgIndex < bm->K_num_tot_sp; fgIndex++) {
				if (FunctGroupArray[fgIndex].speciesParams[flag_id] == TRUE) {
					if (FunctGroupArray[fgIndex].habitatType != EPIFAUNA) {
						if (FunctGroupArray[fgIndex].numCohorts > 1) {

							for (cohort = 0; cohort < FunctGroupArray[fgIndex].numCohorts; cohort++) {
								sprintf(str, "%s%d_%s", FunctGroupArray[fgIndex].name, cohort + 1, bm->contaminantStructure[contamIndex]->contaminant_name);
								Add_Tracer(&trnamelist, index++, str, &FunctGroupArray[fgIndex].contaminantTracers[cohort][contamIndex], 0, FunctGroupArray[fgIndex].diagTol, 0, 1);
                                sprintf(str, "%s%d_Prop_%s", FunctGroupArray[fgIndex].name, cohort + 1, bm->contaminantStructure[contamIndex]->contaminant_name);
                                Add_Tracer(&trnamelist, index++, str, &FunctGroupArray[fgIndex].contamPropTracers[cohort][contamIndex], 0, FunctGroupArray[fgIndex].diagTol, 0, 1);
							}
						} else {
							sprintf(str, "%s_%s", FunctGroupArray[fgIndex].name, bm->contaminantStructure[contamIndex]->contaminant_name);
							Add_Tracer(&trnamelist, index++, str, &FunctGroupArray[fgIndex].contaminantTracers[0][contamIndex], 0, FunctGroupArray[fgIndex].diagTol, 0, 1);
                            sprintf(str, "%s_Prop_%s", FunctGroupArray[fgIndex].name, bm->contaminantStructure[contamIndex]->contaminant_name);
                            Add_Tracer(&trnamelist, index++, str, &FunctGroupArray[fgIndex].contamPropTracers[0][contamIndex], 0, FunctGroupArray[fgIndex].diagTol, 0, 1);

						}
					}
				}
			}
			Add_Tracer(&trnamelist, index++, bm->contaminantStructure[contamIndex]->contaminant_name, &bm->contaminantStructure[contamIndex]->contaminant_tracer, 0, 1, 0, 1);
		}
	}

	Add_Tracer(&trnamelist, index++, NULL, NULL, 0, 0, 0, 0);

	if ((index - 1) != nTrSize) {
		quit("ERROR: Build_TrName_NameList - Mismatch in the number of tracers we are expected and the number we have built. We are expecting %d, but we have built %d tracers\n", nTrSize, index);
	}

}

/*
 *
 Namelist  epinamelist[] = {
 {"Macroalgae_N", &Macroalgae_N_i, 0, 1, 1},
 {"Seagrass_N", &Seagrass_N_i, 0, 1, 1},
 {"Filter_Shallow_N", &Filter_Shallow_N_i, 0, 1, 1},
 {"Filter_Deep_N", &Filter_Deep_N_i, 0, 1, 1},
 {"Filter_Other_N", &Filter_Other_N_i, 0, 1, 1},
 {"Macrobenth_Shallow_N", &Macrobenth_Shallow_N_i, 0, 2, 1},
 {"Macrobenth_Deep_N", &Macrobenth_Deep_N_i, 0, 2, 1},
 {"Megazoobenthos_N", &Megazoobenthos_N_i, 0, 2, 1},
 {"Benthic_grazer_N", &Benthic_grazer_N_i, 0, 2, 1},
 {"Macroalgae_Cover", &Macroalgae_Cover_i, 1, 0, 0},
 {"Seagrass_Cover", &Seagrass_Cover_i, 1, 0, 0},
 {"MicroPB_Cover", &MicroPB_Cover_i, 1, 0, 0},
 {"Filter_Shallow_Cover", &Filter_Shallow_Cover_i, 1, 0, 0},
 {"Filter_Deep_Cover", &Filter_Deep_Cover_i, 1, 0, 0},
 {"Filter_Other_Cover", &Filter_Other_Cover_i, 1, 0, 0},
 {
 */
/**
 * \brief Build the EPI tracer array based on the values loaded from the functional group input file.
 *
 *
 */
void Build_EPI_NameList(MSEBoxModel *bm) {
	char str[STRLEN*2];
	int size = 1, fgIndex, index = 0, cohort, contamIndex;

	for (fgIndex = 0; fgIndex < bm->K_num_tot_sp; fgIndex++) {
		if (FunctGroupArray[fgIndex].habitatType == EPIFAUNA){
			size += FunctGroupArray[fgIndex].numCohortsXnumGenes;
		}

		if (FunctGroupArray[fgIndex].isCover == TRUE)
			size += 1;

	}
	if(bm->track_atomic_ratio == TRUE){
		for (fgIndex = 0; fgIndex < bm->K_num_tot_sp; fgIndex++) {
			if (FunctGroupArray[fgIndex].speciesParams[flag_id] == TRUE) {
				if (FunctGroupArray[fgIndex].habitatType == EPIFAUNA) {
					size += (FunctGroupArray[fgIndex].numCohortsXnumGenes * 2);
				}
			}
		}
	}
	if (bm->track_contaminants == TRUE) {
		for (fgIndex = 0; fgIndex < bm->K_num_tot_sp; fgIndex++) {
			if (FunctGroupArray[fgIndex].speciesParams[flag_id] == TRUE) {
				if (FunctGroupArray[fgIndex].habitatType == EPIFAUNA) {
					size += (FunctGroupArray[fgIndex].numCohorts * bm->num_contaminants * 2);
				}
			}
		}
	}
	epinamelist = (Namelist *) malloc((size_t)size * sizeof(Namelist));

	nEpiSize = size - 1;

	for (fgIndex = 0; fgIndex < bm->K_num_tot_sp; fgIndex++) {
		FunctGroupArray[fgIndex].CoverTracer = -1;
		if (FunctGroupArray[fgIndex].habitatType == EPIFAUNA) {
			switch (FunctGroupArray[fgIndex].groupAgeType) {
			case AGE_STRUCTURED_BIOMASS:
				// Allow for multiple cohorts so can have age structure reefs and multiple macrophyte pools
				for (cohort = 0; cohort < FunctGroupArray[fgIndex].numCohortsXnumGenes; cohort++) {
					sprintf(str, "%s_N%d", FunctGroupArray[fgIndex].name, cohort + 1);
					Add_Tracer(&epinamelist, index++, str, &FunctGroupArray[fgIndex].totNTracers[cohort], 0, FunctGroupArray[fgIndex].diagTol, 1, 0);
				}
				break;
			case BIOMASS:
				if(bm->flag_macro_model == 1 && FunctGroupArray[fgIndex].groupType == SEAGRASS){
					// Allow for multiple cohorts so can have age structure reefs and multiple macrophyte pools
					for (cohort = 0; cohort < FunctGroupArray[fgIndex].numCohortsXnumGenes; cohort++) {
						sprintf(str, "%s_N%d", FunctGroupArray[fgIndex].name, cohort + 1);
						Add_Tracer(&epinamelist, index++, str, &FunctGroupArray[fgIndex].totNTracers[cohort], 0, FunctGroupArray[fgIndex].diagTol, 1, 0);
					}
				}else{
					sprintf(str, "%s_N", FunctGroupArray[fgIndex].name);
					Add_Tracer(&epinamelist, index++, str, &FunctGroupArray[fgIndex].totNTracers[0], 0, FunctGroupArray[fgIndex].diagTol, 1, 0);
				}
				break;
			case AGE_STRUCTURED:
				/* Do nothing */
				break;
			}
		}

		if (FunctGroupArray[fgIndex].isCover == TRUE) {
			sprintf(str, "%s_Cover", FunctGroupArray[fgIndex].name);
			Add_Tracer(&epinamelist, index++, str, &FunctGroupArray[fgIndex].CoverTracer, 1, 0, 0, 0);
		}
	
	}

	if(bm->track_atomic_ratio == TRUE){
		for (fgIndex = 0; fgIndex < bm->K_num_tot_sp; fgIndex++) {
			if (FunctGroupArray[fgIndex].speciesParams[flag_id] == TRUE) {
				if (FunctGroupArray[fgIndex].habitatType == EPIFAUNA) {

					if((FunctGroupArray[fgIndex].numCohortsXnumGenes) > 1){
						for(cohort = 0; cohort < FunctGroupArray[fgIndex].numCohortsXnumGenes; cohort++){
							sprintf(str, "N_to_P_%s%d", FunctGroupArray[fgIndex].name, cohort + 1);
							Add_Tracer(&epinamelist, index++, str, &FunctGroupArray[fgIndex].addRatioTracers[cohort][p_id], 2, FunctGroupArray[fgIndex].diagTol, 0, 0);

							sprintf(str, "N_to_C_%s%d", FunctGroupArray[fgIndex].name, cohort + 1);
							Add_Tracer(&epinamelist, index++, str, &FunctGroupArray[fgIndex].addRatioTracers[cohort][c_id], 2, FunctGroupArray[fgIndex].diagTol, 0, 0);
						}
					}else{
						sprintf(str, "N_to_P_%s", FunctGroupArray[fgIndex].name);
						Add_Tracer(&epinamelist, index++, str, &FunctGroupArray[fgIndex].addRatioTracers[0][p_id], 2, FunctGroupArray[fgIndex].diagTol, 0, 0);

						sprintf(str, "N_to_C_%s", FunctGroupArray[fgIndex].name);
						Add_Tracer(&epinamelist, index++, str, &FunctGroupArray[fgIndex].addRatioTracers[0][c_id], 2, FunctGroupArray[fgIndex].diagTol, 0, 0);
					}
				}
			}
		}
	}

	if (bm->track_contaminants == TRUE) {
        // Instead of using FunctGroupArray[fgIndex].diagTol use 0 as need to allow for spill shocks to occur
		for(contamIndex = 0; contamIndex < bm->num_contaminants; contamIndex++){
			for (fgIndex = 0; fgIndex < bm->K_num_tot_sp; fgIndex++) {
                
                fprintf(bm->logFile, "Doing %s with habitatType: %d\n", FunctGroupArray[fgIndex].name, FunctGroupArray[fgIndex].habitatType);
                
				if (FunctGroupArray[fgIndex].speciesParams[flag_id] == TRUE && FunctGroupArray[fgIndex].habitatType == EPIFAUNA) {
					if (FunctGroupArray[fgIndex].numCohorts > 1) {

						for (cohort = 0; cohort < FunctGroupArray[fgIndex].numCohorts; cohort++) {
							sprintf(str, "%s%d_%s", FunctGroupArray[fgIndex].name, cohort + 1, bm->contaminantStructure[contamIndex]->contaminant_name);
							Add_Tracer(&epinamelist, index++, str, &FunctGroupArray[fgIndex].contaminantTracers[cohort][contamIndex], 0, FunctGroupArray[fgIndex].diagTol, 0, 1);
                            sprintf(str, "%s%d_Prop_%s", FunctGroupArray[fgIndex].name, cohort + 1, bm->contaminantStructure[contamIndex]->contaminant_name);
                            Add_Tracer(&epinamelist, index++, str, &FunctGroupArray[fgIndex].contamPropTracers[cohort][contamIndex], 0, FunctGroupArray[fgIndex].diagTol, 0, 1);
                            
                            fprintf(bm->logFile, "Added %s%d_Prop_%s with index %d\n", FunctGroupArray[fgIndex].name, cohort + 1, bm->contaminantStructure[contamIndex]->contaminant_name, index);
                            //printf("Added %s_Prop_%s with index %d\n", FunctGroupArray[fgIndex].name, bm->contaminantStructure[contamIndex]->contaminant_name, index);

						}
					} else {
						sprintf(str, "%s_%s", FunctGroupArray[fgIndex].name, bm->contaminantStructure[contamIndex]->contaminant_name);
						Add_Tracer(&epinamelist, index++, str, &FunctGroupArray[fgIndex].contaminantTracers[0][contamIndex], 0, FunctGroupArray[fgIndex].diagTol, 0, 1);
                        sprintf(str, "%s_Prop_%s", FunctGroupArray[fgIndex].name, bm->contaminantStructure[contamIndex]->contaminant_name);
                        Add_Tracer(&epinamelist, index++, str, &FunctGroupArray[fgIndex].contamPropTracers[0][contamIndex], 0, FunctGroupArray[fgIndex].diagTol, 0, 1);
                        
                        fprintf(bm->logFile, "Added %s_Prop_%s with index %d\n", FunctGroupArray[fgIndex].name, bm->contaminantStructure[contamIndex]->contaminant_name, index);
                        //printf("Added %s_Prop_%s with index %d\n", FunctGroupArray[fgIndex].name, bm->contaminantStructure[contamIndex]->contaminant_name, index);
					}
				}
			}
		}
	}


	Add_Tracer(&epinamelist, index++, NULL, NULL, 0, 0, 0, 0);

}

/**
 * \brief Build the land tracer array based on the values loaded from the functional group input file.
 *
 *
 */
void Build_LAND_NameList(MSEBoxModel *bm) {
	int size = 2;
	int index = 0;

	landnamelist = (Namelist *) malloc((size_t)size * sizeof(Namelist));

	nLandSize = size - 1;

	Add_Tracer(&landnamelist, index++, "Soil", &Soil_i, 0, 0, 0, 0);
	Add_Tracer(&landnamelist, index++, NULL, NULL, 0, 0, 0, 0);
}

/**
 * \brief Add a new diagnostic tracer to the bm->dinfo array.
 */
static void Add_Diagnostic_Tracer(MSEBoxModel *bm, int index, char *name, char *longName, char *units, int sumType, int dtype, int *tracerIndex, int flux,
		int tol, int bio) {
	strcpy(bm->dinfo[index].name, name);
	strcpy(bm->dinfo[index].long_name, longName);
	bm->dinfo[index].sumtype = sumType;
	bm->dinfo[index].dtype = dtype;
	strcpy(bm->dinfo[index].units, units);
	bm->dinfo[index].flagid = 1;
	*tracerIndex = index;
	DiagFluxflag[index] = flux;
	DiagTolflag[index] = tol;
	DiagBioflag[index] = bio;
}
/**
 * \brief Add a new diagnostic tracer to the bm->finfo array.
 */
void Ecology_Add_FStat_Tracer(MSEBoxModel *bm, int index, char *name, char *longName, char *units, int sumType, int dtype, int *tracerIndex, int flux, int tol,
		int bio) {

	strcpy(bm->finfo[index].name, name);
	strcpy(bm->finfo[index].long_name, longName);
	bm->finfo[index].sumtype = sumType;
	bm->finfo[index].dtype = dtype;
	strcpy(bm->finfo[index].units, units);
	bm->finfo[index].flagid = 1;
	*tracerIndex = index;
	DiagFluxflag[numdiagvar + index] = flux;
	DiagTolflag[numdiagvar + index] = tol;
	DiagBioflag[numdiagvar + index] = bio;
}

/**
 *	\brief Allocate memory for the diagnostic arrays.
 *
 *
 */
void Ecology_Allocate_Diag_Arrays(MSEBoxModel *bm) {

	int b, i;
	int fgIndex;
	int size = 9; // Invertebrates.

	for (fgIndex = 0; fgIndex < bm->K_num_tot_sp; fgIndex++) {
		if (FunctGroupArray[fgIndex].isVertebrate) {
			size += (FunctGroupArray[fgIndex].numCohortsXnumGenes * 2); /* Growth + Eat */
			size += 1; /* Spawn Size */
		} else {
			if (FunctGroupArray[fgIndex].isProducer)
				size += 1;
			if (FunctGroupArray[fgIndex].isGrazer)
				size += 1;
		}
	}

	bm->ndiag = size;

	/* Allocate memory for diagnostics */
	//    /bm->diagnost = (double **)alloc2d(bm->ndiag,bm->nbox);
	bm->diagnost = Util_Alloc_Init_2D_Double(bm->ndiag, bm->nbox, 0.0);

	/* Set up pointers to data from each box */
	for (b = 0; b < bm->nbox; b++) {
		bm->boxes[b].diagnost = bm->diagnost[b];
	}

	/* Initialise values */
	for (i = 0; i < bm->ndiag; i++) {
		for (b = 0; b < bm->nbox; b++) {
			bm->diagnost[b][i] = 0.0;
		}
	}
}
/**
 * \brief Build the diagnostic tracer name list.
 *
 */
void Build_Diagnostic_NameList(MSEBoxModel *bm) {
	int fgIndex, cohort;
	int diagIndex = 0;
	char name[BMSLEN];
	char longName[BMSLEN];

	/* Allocate space for diagnostic info */
	if ((bm->dinfo = (DiagInfo *) malloc((size_t)bm->ndiag * sizeof(DiagInfo))) == NULL)
		quit("readBMDiagInfo: Can't allocate memory for info\n");

	/* Clear tracer info */
	memset(bm->dinfo, 0, (size_t)bm->ndiag * sizeof(DiagInfo));

	for (fgIndex = 0; fgIndex < bm->K_num_tot_sp; fgIndex++) {
		// add the production Tracers
		if (FunctGroupArray[fgIndex].isVertebrate == FALSE) {
			if (FunctGroupArray[fgIndex].isProducer) {

				sprintf(name, "%sProdn", FunctGroupArray[fgIndex].name);
				sprintf(longName, "%s production", FunctGroupArray[fgIndex].name);

				Add_Diagnostic_Tracer(bm, diagIndex++, name, longName, "mg N m-3 d-1", 0, 2, &FunctGroupArray[fgIndex].prodnTracers[0], 0,
						FunctGroupArray[fgIndex].diagTol == 2 ? 2 : 0, 0);
			}
		}
	}

	for (fgIndex = 0; fgIndex < bm->K_num_tot_sp; fgIndex++) {
		// add the totNTracers
		if (FunctGroupArray[fgIndex].isVertebrate == FALSE) {
			if (FunctGroupArray[fgIndex].isGrazer) {

				sprintf(name, "%sGrazing", FunctGroupArray[fgIndex].name);
				sprintf(longName, "%s grazing", FunctGroupArray[fgIndex].name);

				Add_Diagnostic_Tracer(bm, diagIndex++, name, longName, "mg N m-3 d-1", 0, 2, &FunctGroupArray[fgIndex].GrazingTracers[0], 0,
						FunctGroupArray[fgIndex].diagTol == 2 ? 2 : 0, 0);
			}
		}
	}

	Add_Diagnostic_Tracer(bm, diagIndex++, "PercentDenitri", "PercentDenitrification", "percent", 0, 2, &PercentDenitri_i, 0, 0, 0);
	Add_Diagnostic_Tracer(bm, diagIndex++, "Nitrif_eff", "Nitrification efficiency", " ", 0, 2, &Nitrif_eff_i, 0, 0, 0);
	Add_Diagnostic_Tracer(bm, diagIndex++, "Denitrif_eff", "Denitrification efficiency", " ", 0, 2, &Denitrif_eff_i, 0, 0, 0);
	Add_Diagnostic_Tracer(bm, diagIndex++, "Recyc_N", "Recycled Nitrogen", "mg N m-3 d-1", 0, 2, &Recyc_N_i, 0, 0, 0);
	Add_Diagnostic_Tracer(bm, diagIndex++, "Don_Prod", "Recycled Don Production", "mg N m-3 d-1", 0, 2, &Don_Prod_i, 0, 0, 0);

	for (fgIndex = 0; fgIndex < bm->K_num_tot_sp; fgIndex++) {
		// add the totNTracers
		if (FunctGroupArray[fgIndex].isVertebrate == TRUE) {
			for (cohort = 0; cohort < FunctGroupArray[fgIndex].numCohortsXnumGenes; cohort++) {

				sprintf(name, "%s%d_Growth", FunctGroupArray[fgIndex].name, cohort + 1);
				sprintf(longName, "Growth of age class %d %s", cohort + 1, FunctGroupArray[fgIndex].name);
				Add_Diagnostic_Tracer(bm, diagIndex++, name, longName, "mg N d-1", 0, 2, &FunctGroupArray[fgIndex].GrowthTracers[cohort], 0,
						FunctGroupArray[fgIndex].diagTol, 0);

			}
		}
	}

	for (fgIndex = 0; fgIndex < bm->K_num_tot_sp; fgIndex++) {
		// add the totNTracers
		if (FunctGroupArray[fgIndex].isVertebrate == TRUE) {
			for (cohort = 0; cohort < FunctGroupArray[fgIndex].numCohortsXnumGenes; cohort++) {

				sprintf(name, "%s%d_Eat", FunctGroupArray[fgIndex].name, cohort + 1);
				sprintf(longName, "Consumption of age class %d %s", cohort + 1, FunctGroupArray[fgIndex].name);
				Add_Diagnostic_Tracer(bm, diagIndex++, name, longName, "mg N m-3 d-1", 0, 2, &FunctGroupArray[fgIndex].EatTracers[cohort], 0,
						FunctGroupArray[fgIndex].diagTol, 0);

			}

			/* Add the Spawn Size */
			sprintf(name, "%s_SpwnSze", FunctGroupArray[fgIndex].name);
			sprintf(longName, "%s average size spawning", FunctGroupArray[fgIndex].name);
			Add_Diagnostic_Tracer(bm, diagIndex++, name, longName, "mg N", 0, 2, &FunctGroupArray[fgIndex].SpawnSizeTracer, 0, FunctGroupArray[fgIndex].diagTol,
					0);
		}
	}

	Add_Diagnostic_Tracer(bm, diagIndex++, "PelDem_ratio", "Pelagic vs Demersal fish biomass", " ", 0, 2, &PelDem_ratio_i, 0, 2, 0);
	Add_Diagnostic_Tracer(bm, diagIndex++, "PiscPlank_ratio", "Piscivorous vs planktivorous pelagic fish biomass", " ", 0, 2, &PiscPlank_ratio_i, 0, 2, 0);
	Add_Diagnostic_Tracer(bm, diagIndex++, "InfEpi_ratio", "Infauna vs Epifauna biomass", " ", 0, 2, &InfEpi_ratio_i, 0, 2, 0);
	Add_Diagnostic_Tracer(bm, diagIndex++, "DivCount", "Diversity index", " ", 0, 2, &DivCount_i, 0, 2, 0);
}
