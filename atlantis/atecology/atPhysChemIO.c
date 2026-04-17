/**
 *	\file
 *	\brief This file contains function associated with loading the physiochem group information.
 * \ingroup atEcology
 *
 * Revisions:
 *		09/09/2008 Bec Gorton : Added the diagnostic flux and tolerance values to the PhysioChemStruct structure.
 *			Created a function to add each of the tracers to the PhysioChemStruct array.
 *
 *      21-04-2009 Bec Gorton:
 *      Added a flux calculation function to the PhysioChemStruct array. This is now used
 *      to calculate the flux for each nutrient. The atEcology code now just loops over the PhysioChemStruct
 *      array and calls each function if not NULL.
 *
 *
 *		23-07-2009 Bec Gorton
 *		Added support for the new SED tracer - this is only used if the model is flagged as estuary (flagIsEstuary in the run.prm file)
 *		If this is set to true then an additional tracer SED is added to the tracer list and this new tracer must be found in the initial conditions
 *		input file. This is then used to attenuate light in the water column due to the presence of sediment. The tracer value in a
 *		water column layer is multiplied by the k_SED coefficient read in from the biology input file.
 */
/*  Open library routines */
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>
#include <ctype.h>
#include <sjwlib.h>
#include <netcdf.h>
#include "atecology.h"


static void Add_Phyio_Chem_Tracer(MSEBoxModel *bm, FILE *llogfp, int index,
		char *name, int *tracerIndex, int flux, int tol, int biol,
		int *debugIndex,
        void (*fp)(MSEBoxModel *bm, FILE *llogfp, HABITAT_TYPES habitatType, int index, BoxLayerValues *boxLayerInfo))
{
	strcpy(PhysioChemArray[index].name, name);

	PhysioChemArray[index].tracerIndex = tracerIndex;
	PhysioChemArray[index].diagTol = tol;
	PhysioChemArray[index].diagFlux = flux;
	PhysioChemArray[index].diagBiol = biol;
    PhysioChemArray[index].debugIndex = index + bm->K_num_tot_sp;
	if(debugIndex != NULL)
		*debugIndex = index + bm->K_num_tot_sp;

    PhysioChemArray[index].ROCFunction = fp;


}

/*
 * /brief This function sets up the PhysioChemArray.
 */
void Ecology_Assign_Physio_Chem(MSEBoxModel *bm, FILE *llogfp)
{
	int  i = 0;
    
    if(verbose){
        printf("Start reading Ecology_Assign_Physio_Chem\n");
    }
    
	bm->K_num_physiochem = 17;
	if(bm->flagIsEstuary || bm->flagpassivetracer)
		bm->K_num_physiochem++;
	if (bm->track_pH) {
		bm->K_num_physiochem++;
	}
	if(bm->track_atomic_ratio == TRUE){
		bm->K_num_physiochem += 14;  // The raw C and P nutrient tracers
	}
	if(bm->track_rugosity_arag) {
		bm->K_num_physiochem += 2;
	}
    
    if (bm->track_wind) {
        bm->K_num_physiochem++;
    }

	PhysioChemArray = (PhysioChemStruct *)malloc(sizeof(PhysioChemStruct) * (size_t)bm->K_num_physiochem);

    Add_Phyio_Chem_Tracer(bm, llogfp, i++, "NH3", &NH3_i, 0, 1, 0, NULL, Ammonium_ROC);
    if(bm->flagIsEstuary) {
        Add_Phyio_Chem_Tracer(bm, llogfp, i++, "SED", &SED_i, 0, 1, 0, NULL, NULL);
    }
    if(bm->flagpassivetracer) { // Intentionally overloading SED_i tracer
        Add_Phyio_Chem_Tracer(bm, llogfp, i++, "SED", &SED_i, 0, 0, 0, NULL, NULL);
    }
    Add_Phyio_Chem_Tracer(bm, llogfp, i++, "NO3", &NO3_i, 0, 1, 0, NULL, Nitrate_ROC);
    Add_Phyio_Chem_Tracer(bm, llogfp, i++, "DON", &DON_i, 0, 1, 0, NULL, DON_ROC);
    Add_Phyio_Chem_Tracer(bm, llogfp, i++, "MicroNut", &MicroNut_i, 0, 1, 0, NULL, Micronutrient_ROC);
    Add_Phyio_Chem_Tracer(bm, llogfp, i++, "Oxygen", &Oxygen_i, 0, 0, 0, NULL, Oxygen_ROC);
    Add_Phyio_Chem_Tracer(bm, llogfp, i++, "Si", &Si_i, 0, 1, 0, NULL, Dissolved_Silica_ROC);
    Add_Phyio_Chem_Tracer(bm, llogfp, i++, "Det_Si", &Det_Si_i, 0, 1, 0, NULL, Detrital_Silica_ROC);

	Add_Phyio_Chem_Tracer(bm, llogfp, i++, "Light", &Light_i, 0, 0, 0, NULL, NULL);
    Add_Phyio_Chem_Tracer(bm, llogfp, i++, "DayLight", &DayLight_i, 0, 0, 0, NULL, NULL);
	Add_Phyio_Chem_Tracer(bm, llogfp, i++, "Temp", &Temp_i, 0, 0, 0, NULL, NULL);
	if (bm->track_pH) {
		Add_Phyio_Chem_Tracer(bm, llogfp, i++, "pH", &pH_i, 0, 0, 0, NULL, NULL);
	} else {
		/* Set to -1 so we will get valgrind issues if used without being defined */
		pH_i = -1;
	}

	Add_Phyio_Chem_Tracer(bm, llogfp, i++, "Denitrification", &Denitrification_i, 1, 0, 0, NULL, NULL);
	Add_Phyio_Chem_Tracer(bm, llogfp, i++, "Nitrification", &Nitrification_i, 1, 0, 0, NULL, NULL);
	Add_Phyio_Chem_Tracer(bm, llogfp, i++, "Chl_a", &Chl_a_i, 1, 0, 0, NULL, NULL);
	Add_Phyio_Chem_Tracer(bm, llogfp, i++, "Stress", &Stress_i, 1, 0, 0, NULL, NULL);
	Add_Phyio_Chem_Tracer(bm, llogfp, i++, "DiagNGain", &DiagNGain_i, 1, 0, 0, NULL, NULL);
	Add_Phyio_Chem_Tracer(bm, llogfp, i++, "DiagNLoss", &DiagNLoss_i, 1, 0, 0, NULL, NULL);
	Add_Phyio_Chem_Tracer(bm, llogfp, i++, "DiagNFlux", &DiagNFlux_i, 1, 0, 0, NULL, NULL);

	if(bm->track_atomic_ratio == TRUE){
		Add_Phyio_Chem_Tracer(bm, llogfp, i++, "Phosphorus", &P_i, 0, 1, 0, NULL, Phosphorus_ROC);
		Add_Phyio_Chem_Tracer(bm, llogfp, i++, "TOP", &TOP_i, 0, 1, 0, NULL, TOP_ROC);
		Add_Phyio_Chem_Tracer(bm, llogfp, i++, "Carbon", &C_i, 0, 1, 0, NULL, Carbon_ROC);
		Add_Phyio_Chem_Tracer(bm, llogfp, i++, "CO2", &CO2_i, 0, 1, 0, NULL, NULL);
        Add_Phyio_Chem_Tracer(bm, llogfp, i++, "Tfactor", &Tfactor_i, 0, 1, 0, NULL, NULL);
        
        Add_Phyio_Chem_Tracer(bm, llogfp, i++, "Pads_r", &Pads_r_i, 0, 1, 0, NULL, NULL);
        Add_Phyio_Chem_Tracer(bm, llogfp, i++, "r_immob_PIP", &r_immob_PIP_i, 0, 1, 0, NULL, NULL);
        Add_Phyio_Chem_Tracer(bm, llogfp, i++, "DIP", &DIP_i, 0, 1, 0, NULL, NULL);
        Add_Phyio_Chem_Tracer(bm, llogfp, i++, "EFI", &EFI_i, 0, 1, 0, NULL, NULL);
        Add_Phyio_Chem_Tracer(bm, llogfp, i++, "PIP", &PIP_i, 0, 1, 0, NULL, NULL);
        Add_Phyio_Chem_Tracer(bm, llogfp, i++, "PIPI", &PIPI_i, 0, 1, 0, NULL, NULL);
        Add_Phyio_Chem_Tracer(bm, llogfp, i++, "TIP", &TIP_i, 0, 1, 0, NULL, NULL);
        Add_Phyio_Chem_Tracer(bm, llogfp, i++, "PIP_Dust", &PIP_Dust_i, 0, 1, 0, NULL, NULL);
        Add_Phyio_Chem_Tracer(bm, llogfp, i++, "Dust", &Dust_i, 0, 1, 0, NULL, NULL);
        
	}

	if(bm->track_rugosity_arag == TRUE) {
		Add_Phyio_Chem_Tracer(bm, llogfp, i++, "Rugosity", &Rugosity_i, 0, 0, 0, NULL, NULL);
		Add_Phyio_Chem_Tracer(bm, llogfp, i++, "AragoniteSaturation", &AragoniteSat_i, 0, 0, 0, NULL, NULL);
	}
    
    if (bm->track_wind) {
        Add_Phyio_Chem_Tracer(bm, llogfp, i++, "Wind", &Wind_i, 0, 0, 0, NULL, NULL);
    }
    
	return;
}

/**
 *\brief Free up the PhysioChemArray memory.
 *
 *	Frees up each of the tracer arrays and then the PhysioChemArray array.
 */
void Ecology_PhysioChem_Free(MSEBoxModel *bm)
{
    free(PhysioChemArray);
}
