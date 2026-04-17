/**
 \file
 \brief C file for process subroutines dealing with seargrass model developed by Marie Savina-Rolland 2012
 \ingroup atEcology

 File:	atmacrophyte.c
 Author:	Beth Fulton
 Created:	11/5/2012
 Purpose:	C file for process dealing with seargrass model developed by Marie Savina-Rolland 2012
 Revisions:

 17-04-2013 Bec Gorton
 ATL-346 - Added support of tracking P:N:C ratios in groups.
 Probably needs more work if being used with new seagrass code.

 **/

/*  Open library routines */
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "atecology.h"

/*************************************************************************//**

 */
void Grow_Macrophytes(MSEBoxModel *bm, FILE *llogfp, int sp_id, int cohort, int micro_case, int lim_case, int macro_producer,
		double sp_biom, double sp_biom_sed, double sp_biom_epi,
		double DINs, double NHs, double NOs, double DIN, double NH, double NO, double Si, double Fe, double P, double C, double IRR,
		double E_sp, double mL_other, double SPmax, double area_hab,
		double *spUptakeNO, double *sphN, double PRatio, double CRatio, double *spUptakeP, double *spUptakeC) {

	double mL_sp = 0.0, this_biom = 0.0, step1 = 0.0, step2 = 0.0, hI_sp = 0.0, hN_sp = 0.0, hSPACE_sp = 0.0,
			sp_grow = 0.0, uptakeNH = 0.0, uptakeNO = 0.0, uptakeP = 0.0, uptakeC = 0.0, KN_term = 0.0;
	double ratio = 1.0;
	double prop_daylight = 1.0;
	double scale_uptake = 1.0;
	double trans = FunctGroupArray[sp_id].speciesParams[Ktrans_id];
	double this_mum = 0;
	double mE_sp = 1.0;
    double mS_sp = Acidif_Mort(bm, sp_id);
	double P0 = FunctGroupArray[sp_id].speciesParams[P_min_internal_id];
	double P1 = FunctGroupArray[sp_id].speciesParams[P_max_internal_id];

	prop_daylight = bm->boxes[bm->current_box].prop_light_time;
	IRR = IRR * prop_daylight;

	/* do some checks of the values we have got */

	switch (cohort ) {
	case main_biomass_id:  /** Above-ground biomass - i.e. Leaves **/
		this_biom = sp_biom;
		this_mum = FunctGroupArray[sp_id].scaled_mum[cohort];
		mL_sp = FunctGroupArray[sp_id].cohortSpeciesParams[main_biomass_id][mL_id];
		mE_sp = FunctGroupArray[sp_id].cohortSpeciesParams[main_biomass_id][mE_id];

		// Light limitation (epiphyte shading formulation: Fong and Harwell 94)
		ratio = sp_biom_epi / (sp_biom + small_num);
		step1 = exp(-FunctGroupArray[sp_id].speciesParams[Kext_id] * ratio) * IRR;
		step2 = step1 / FunctGroupArray[sp_id].speciesParams[KI_L_id];
		hI_sp = min(step2,1.0);

		if ((bm->debug == debug_biology_process) && (bm->dayt >= bm->checkstart) && (bm->dayt < bm->checkstop)) {

				fprintf(bm->logFile, "sp_biom_epi = %.20e, sp_biom= %.20e\n", sp_biom_epi, sp_biom);

				fprintf(bm->logFile, "Grow_Macrophytes, %s:%d, ratio = %.20e, step1= %.20e, step2= %.20e, hI_sp= %.20e\n",
									FunctGroupArray[sp_id].groupCode, cohort, ratio, step1, step2, hI_sp);

				fprintf(bm->logFile, "Kext_id  = %.20e, IRR= %.20e, prop_daylight= %.20e\n",
						FunctGroupArray[sp_id].speciesParams[Kext_id], IRR, prop_daylight);
		}

		// Nutrient limitation
		hN_sp = Nutrient_Lim(bm, lim_case, micro_case, DINs, Si, Fe, P, PRatio, P0, P1,
				FunctGroupArray[sp_id].speciesParams[KN_id], FunctGroupArray[sp_id].speciesParams[KS_id],
				FunctGroupArray[sp_id].speciesParams[KF_id], FunctGroupArray[sp_id].speciesParams[KP_id]);
		*sphN = hN_sp;

		// Space limitation
		ratio = sp_biom_sed / (sp_biom + small_num);
		hSPACE_sp = (1.0 - exp(-FunctGroupArray[sp_id].speciesParams[Ksub_id] * ratio)); //changed (1.0-trans) deleted
        
        // If allow for extra space limitation not just due to roots vs leaves
        if (FunctGroupArray[sp_id].speciesParams[flag_lim_id] == simple_ben_lim) {
            hSPACE_sp *= min(1.0, max(0.0, (1.0 - sp_biom / (SPmax * area_hab + small_num))));
        } else if (FunctGroupArray[sp_id].speciesParams[flag_lim_id] == ersem_ben_lim) {
            quit("This is not coded for %s group types as yet - see Beth for a fix or change type of %s\n", FunctGroupArray[sp_id].groupType, FunctGroupArray[sp_id].groupCode);
        }

		// Growth
		FunctGroupArray[sp_id].growth[cohort] = (this_biom * this_mum * hI_sp * hN_sp * hSPACE_sp * (1.0-trans)); // changed (1-trans) added

		if ((bm->debug == debug_biology_process) && (bm->dayt >= bm->checkstart) && (bm->dayt < bm->checkstop)) {
			fprintf(bm->logFile, "Grow_Macrophytes, %s:%d, this_biom  = %.20e, this_mum =%.20e, hI_sp = %.20e, hN_sp = %.20e, hSPACE_sp = %.20e, trans = %.20e\n",
					FunctGroupArray[sp_id].groupCode, cohort, this_biom , this_mum, hI_sp, hN_sp, hSPACE_sp, trans);
		}

		if(!_finite(hSPACE_sp) || isnan(hSPACE_sp)){
			fprintf(bm->logFile, "sp_biom = %e\n", sp_biom);
			fprintf(bm->logFile, "FunctGroupArray[sp_id].invertProps->Ksub = %e, ratio = %e, trans = %e\n",
					FunctGroupArray[sp_id].speciesParams[Ksub_id], ratio, trans);
			quit("");
		}

		break;
	case epiphyte_biomass_id:  /** Epiphytes **/
		ratio = sp_biom / (sp_biom_epi + small_num);
		this_biom = sp_biom_epi;
		this_mum = FunctGroupArray[sp_id].scaled_mum[cohort];

		mL_sp = ( FunctGroupArray[sp_id].cohortSpeciesParams[epiphyte_biomass_id][mL_id] + FunctGroupArray[sp_id].cohortSpeciesParams[main_biomass_id][mL_id] );
		mE_sp = ( FunctGroupArray[sp_id].cohortSpeciesParams[epiphyte_biomass_id][mE_id] + FunctGroupArray[sp_id].cohortSpeciesParams[main_biomass_id][mE_id] );
		hI_sp = Light_Lim(bm, bm->flaglight, IRR, FunctGroupArray[sp_id].speciesParams[KI_id], FunctGroupArray[sp_id].SP_IRR, prop_daylight, 0.0);

		// Nutrient limitation

		hN_sp = Nutrient_Lim(bm, lim_case, micro_case, DIN, Si, Fe, P, PRatio, P0, P1, FunctGroupArray[sp_id].speciesParams[KNepi_id], FunctGroupArray[sp_id].speciesParams[KS_id], FunctGroupArray[sp_id].speciesParams[KF_id], FunctGroupArray[sp_id].speciesParams[KP_id]);

		// Space limitation (from Bartleson et al)
		// Alternatively (from Plus et al)
		// hSPACE_sp = 1.0 - (FunctGroupArray[sp_id].growth[main_biomass_id]/(FunctGroupArray[sp_id].growth[main_biomass_id]+Ksub));
		hSPACE_sp = 1.0 - exp(-FunctGroupArray[sp_id].speciesParams[KsubEpi_id] * ratio);
            
        if ((sp_biom_epi / sp_biom) > FunctGroupArray[sp_id].speciesParams[KsubEpi_id])
            hSPACE_sp = 0.0;  // To constrain epiphytes wheich were growing uncontrollably compared to leaf biomass

		// Growth
		FunctGroupArray[sp_id].growth[cohort] = (this_biom * this_mum * hI_sp * hN_sp * hSPACE_sp);

		break;
	case below_ground_biomass_id:  /** Below ground biomass - i.e. Roots **/
		this_mum = 0;	/* Added in to supress warnings */
		this_biom = sp_biom_sed;
		mL_sp = FunctGroupArray[sp_id].cohortSpeciesParams[below_ground_biomass_id][mL_id];
		mE_sp = FunctGroupArray[sp_id].cohortSpeciesParams[below_ground_biomass_id][mE_id];
		hI_sp = 1.0;  // As not relevant
		hN_sp = 1.0;  // As not relevant

		// Space limitation - the main relevant process other than mortality
		if(sp_biom_sed < FunctGroupArray[sp_id].speciesParams[max_id]){
			hSPACE_sp = (fabs(1.0 - sp_biom_sed / FunctGroupArray[sp_id].speciesParams[max_id])); //changed trans deleted
		}
		/* Growth in the roots is related to growth of leaves - no max growth rate for roots */
		FunctGroupArray[sp_id].growth[cohort] = (this_biom * FunctGroupArray[sp_id].growth[main_biomass_id] * hI_sp * hN_sp * hSPACE_sp*trans/(1-trans));
		//changed trans added and also divided by (1-trans) because FunctGroupArray[sp_id].processProps[main_biomass_id].growth has this *(1-trans) component that we want to get rid of to calculate the root growth
		break;

	}
	// Growth
	sp_grow = FunctGroupArray[sp_id].growth[cohort];

	if(!_finite(FunctGroupArray[sp_id].growth[cohort]) || isnan(FunctGroupArray[sp_id].growth[cohort])){
		fprintf(bm->logFile, "cohort = %d, this_biom = %e, this_mum = %e, hI_sp = %e, hN_sp = %e, hSPACE_sp = %e\n", cohort, this_biom, this_mum, hI_sp, hN_sp, hSPACE_sp);

		fprintf(bm->logFile, "FunctGroupArray[sp_id].processProps[cohort].growth = %e\n", FunctGroupArray[sp_id].growth[cohort]);

		quit("Seagrass growth is out of control!.");

	}

	// Mortality
	FunctGroupArray[sp_id].mortality[cohort] = (mL_sp + mE_sp + mS_sp) * this_biom;
	bm->calcTrackedMort[sp_id][cohort][0][ongoingM1_id] += (FunctGroupArray[sp_id].mortality[cohort] * FunctGroupArray[sp_id].speciesParams[Mdt_id]);

	/** Handle total nutrient uptake **/
	if (cohort != epiphyte_biomass_id) {
		/* Calculate sediment nutrient uptake - sediment nutrients */
        KN_term = FunctGroupArray[sp_id].speciesParams[KN_id] * sp_biom_sed / (sp_biom + sp_biom_sed + small_num);
        
		uptakeNH = sp_grow * (NHs / (KN_term + NHs)) * ((KN_term + DINs) / DINs);
		uptakeNO = sp_grow * (NOs / (KN_term + DINs)) * (KN_term / (KN_term + NHs)) * (1.0 + KN_term / DINs);

		/* Cap nutrient uptake to what is available - this intentionally a max as it is rescaling the NH uptake based on which is the least limitign of the limiting nutients */
		scale_uptake = 1.0;
		if (uptakeNH > NHs)
			scale_uptake = NHs / uptakeNH;
		if (uptakeNO > NOs)
			scale_uptake = max(scale_uptake, NOs / uptakeNO);

	} else {
		/* Calculate nutrient uptake - water column nutrients */
		uptakeNH = sp_grow * (NH / (FunctGroupArray[sp_id].speciesParams[KN_id] + NH)) * ((FunctGroupArray[sp_id].speciesParams[KN_id] + DIN) / DIN);
		uptakeNO = sp_grow * (NO / (FunctGroupArray[sp_id].speciesParams[KN_id] + DIN)) * (FunctGroupArray[sp_id].speciesParams[KN_id]
				/ (FunctGroupArray[sp_id].speciesParams[KN_id] + NH)) * (1.0 + FunctGroupArray[sp_id].speciesParams[KN_id] / DIN);

		/* Cap nutrient uptake to what is available - this intentionally a max as it is rescaling the NH uptake based on which is the least limitign of the limiting nutients */
		scale_uptake = 1.0;
		if (uptakeNH > NH)
			scale_uptake = NH / uptakeNH;
		if (uptakeNO > NO)
			scale_uptake = max(scale_uptake, NO / uptakeNO);

		if(bm->track_atomic_ratio == TRUE){

			/* Calculate P uptake */
			uptakeP = Calculate_Phosphorus_Uptake(bm, sp_id, sp_grow, PRatio);
			/*
			if(bm->current_box == 5 && bm->current_layer == 5){
				fprintf(bm->logFile, "\n\nCalculate_Phosphorus_Uptake - %s - FunctGroupArray[guild].addRatioFluxes[0][p_id] = %e\n",
					FunctGroupArray[sp_id].groupCode, FunctGroupArray[sp_id].addRatioFluxes[0][p_id]);
			}*/

			/* Now do carbon */
			uptakeC = Calculate_Carbon_Uptake(bm, sp_id, sp_grow,CRatio);

		}
	}

	/* Assign uptake */
	FunctGroupArray[sp_id].uptakeNH[cohort] = uptakeNH * scale_uptake;
	*spUptakeNO = uptakeNO * scale_uptake;
	*spUptakeP = uptakeP;
	*spUptakeC = uptakeC;

	if(bm->track_atomic_ratio == TRUE){
        // This used to be uptakeNH[0] - most macrophytes only have 1 cohort so likely no big effect on dynamics
		PP_uptake(bm, sp_id, FunctGroupArray[sp_id].uptakeNH[cohort], *spUptakeP , *spUptakeC);
	}

	if ((bm->debug == debug_biology_process) && (bm->dayt >= bm->checkstart) && (bm->dayt < bm->checkstop)) {
		fprintf(bm->logFile, "Grow_Macrophytes outcomes. Group %s:%d, grow = %.20e, uptakeNH= %.20e, uptakeNO= %.20e, mortality= %.20e\n", FunctGroupArray[sp_id].groupCode, cohort, sp_grow, uptakeNH, uptakeNO, FunctGroupArray[sp_id].mortality[cohort]);
	}

	return;
}


/*
c Opening the result and diag files
       open(999,file='results.txt')
       write(999,*) 'time   EPI   LEAVES   ROOTS'
       write(999,*) datedepart, EPI, LEAVES, ROOTS
       open(997,file='diagE.txt')
       write(997,*) 'time limL  limN limS'
       open(996,file='diagL.txt')
       write(996,*) 'time limN limL limS '
       open(995,file='diagR.txt')
       write(995,*) 'time limS'
       open(994, file='environment.txt')
       write(994,*)'time NutS NutW IRR'

*/

