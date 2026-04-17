/**
 *	\brief The process functions for ice related processes
 *	\ingroup atecology
 *
 *
 *	15-03-2012 Beth Fulton
 *	Created the file
 */

/*  Open library routines */
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <sjwlib.h>
#include <atecology.h>

/**
 \file
 \brief The ice related ecological processes.

 **/

/***************************************** PHYSICAL ASPECTS ***********************************************************
 *
 * brief\ Day to day ice fluxes - like diffusion of nurtients into bottom ice layers
 * Taken from Nishi and Tabeta 2005 (Journal of Marine Systems 55)
 *
 */
void Box_Ice_Flux(MSEBoxModel *bm, Box *pBox, FILE *llogfp) {
	int wc_layer = 0;
	int tr_id = 0;
	int index = 0;
	double water_amt = 0.0;
	double ice_amt = 0.0;
	double brine_flux, freeze_flux, diffusive_flux, final_flux;
	double dt_scale = 86400.0 / bm->dt;
	double k_brine_diff = 0.0;
	IceModel *ice = &pBox->ice;
	int i = ice->currentnz - 1;
	double *Y1 = 0, // Water tracers
		   *Y2 = 0; // Ice tracers

	/* Get constants for the calculations */
	double igr = ice->ice_growth_rate * 100;     // * 100 so that it is in cm not metres

	// From Nishi and Tabeta 2005 Figure 3
	if (igr > 0.0)
		k_brine_diff = 72.0 * (9.667e-11 + 4.49e-8 * igr - 1.39e-9 * igr * igr);
	else
		k_brine_diff = 720.0 * (4.49e-6 * igr - 1.39e-9 * igr * igr);
    k_brine_diff *= dt_scale;   					// converting from per s to per day

	if(ice->currentnz < 1)
		return;	// No ice so skip ahead

	/* Get water layer of bottom of the ice */
	wc_layer = ice->wc_layer_match[i];

	Y1 = (double*) pBox->tr[wc_layer];
	Y2 = (double*) pBox->ice.tr[i];

	if(wc_layer > pBox->nz)
		quit("Ice layer mapped to water column layer (%d) that doesn't exist for this box (%d has %d water colum layers)\n", wc_layer, pBox->n, pBox->nz);

	/* Flux of nutrients into the ice - note given scale ignoring lateral melting flux */
	for(index=0; index<bm->K_num_physiochem; index++){
		tr_id = *PhysioChemArray[index].tracerIndex;

		water_amt = Y1[tr_id]; // This should really be the boundary layer amount but we're
							   // not following that so just use water value for now
		ice_amt = Y2[tr_id];

		/* Brine convection of nutrients and freezing flux */
		if(ice->is_freezing){
			brine_flux = k_brine_diff * (water_amt - ice_amt);
			freeze_flux = fabs(igr) * water_amt;
		} else {
			brine_flux = 0.0;
			freeze_flux = fabs(igr) * ice_amt;
		}

		/* Diffusive flux - this is a flux into the boundary layer
		 * which is always diffusive_flux = k_icebound_diff * (water_amt - ice_amt);
		 *
		 * However we're not explicitly following the boundary
		 * layer so leave it as zero here for now.
		 */
		diffusive_flux = 0.0;

		/* Final flux and nutrient concentration */
		final_flux = (brine_flux + freeze_flux + diffusive_flux) * bm->dt;

		//TODO BEC - Check with Beth - time is already taken into account in the ice growth rate.
		final_flux = (brine_flux + freeze_flux + diffusive_flux);// * bm->dt;

		/* Update the tracer fields */
		if(ice->is_freezing){
			Y1[tr_id] -= final_flux;
			Y2[tr_id] += final_flux;
		} else {
			Y1[tr_id] += final_flux;
			Y2[tr_id] -= final_flux;
		}

		/* Sanity check */
		if(Y1[tr_id] < 0.0) Y1[tr_id] = 0.0;
		if(Y2[tr_id] < 0.0) Y2[tr_id] = 0.0;
	}
    
	return;
}

/*
 * brief\ Temperature related dynamics in ice
 *
 * From Gary Griffiths matlab code for his ice algal model - Griffith & Spitz 2009
 */

void Box_Ice_Temperature_Related(MSEBoxModel *bm, Box *pBox, FILE *llogfp)
{
	int i;
	int slush_layer = pBox->ice.currentnz - 1;
	IceModel *ice = &pBox->ice;
	double *Y1 = 0;

	if(ice->currentnz < 1)
		return;	// No ice so skip ahead

	/* Set temperature for the different ice layers */
	for(i = 0; i < slush_layer; i++){
		Y1 = (double*) pBox->ice.tr[i];
		Y1[Temp_i] = 1.8;
	}
	/* And the temperature in the slush layer */
	Y1 = (double*) pBox->ice.tr[slush_layer];
	Y1[Temp_i] = 0.0;

}


/*
* brief\ Get light in ice - from Griffith & Spitz 2009

 Extinction properties of light in Snow and ice m^-1
 Assumes that algal layer attenuates blue-green and red light equally.

 k_bs = 15.0     m^-1 blue-green light attenuation per metre in snow.
 k_bi = 2.0  	 m^-1 blue-green light attenuation per metre in ice.
 k_rs = 35.0 	 m^-1 red light attenuation per metre in snow.
 k_ri = 4.0      m^-1 red light attenuation per metre in ice.
 R_bi = 0.5  	 fraction of photosynthetically available light in the blue-green region

 k_ice = 0.45
 albedo_ice = 0.4    Walsh 0.7 or Grenfell & Perovich 1984 0.4
 albedo_snow = 0.6   Grenfell & Perovich 1984 0.6 to 0.81

 ka_star = 0.03		 Soohoo 1987 (neglects wavelength dependence)

*
*/
void Box_Ice_Light_Level(MSEBoxModel *bm, Box *pBox, FILE *llogfp) {
	int i, k, guild;
    //double icedepth;
	double snow_cover, ice_cover, lightbot, lighttop, k_a;
	IceModel *ice = &pBox->ice;
	double *Y1 = 0;
	double phytoBiomass = 0.0;
	double step1, step2;
	//double light_surf;

	if(ice->currentnz < 1)
		return;	// No ice so skip ahead

	/* Calculate light at the top of the ice */
	lighttop = pBox->swr;

	ice_cover = pBox->tot_ice_prop;

	//light_surf = k_ice * lighttop;	// At best PAR = 0.45 of incoming shortwave radiation
	lightbot = 0;

	/* Calculate irradiance in each layer */
	//icedepth = 0.0;
	for(i = ice->currentnz - 1; i >= 0 ; i--){
		Y1 = (double*) pBox->ice.tr[i];


		/* Get ice and snow cover first */
		ice_cover = 0.0;
		snow_cover = 0.0;
		for(k = 0; k < bm->K_num_ice_classes; k++){
			if(k != bm->snow_id)
				ice_cover += ice->ice_classes[i][k];
			else
				snow_cover += ice->ice_classes[i][k]; // Should be zero for all but top layer
		}
		/* Now get shading terms
		 *
		 * Get k_a for absorption in the algal layer, units are
		 * m^-1 light attenuation in algal layer per metre.
		 * lighttop
		 * Could just do a test on (FunctGroupArray[sp].isPrimaryProducer == TRUE)
		 * as don't expect macrophytes, but just in case do the following to
		 * be consistent with the open water form
		 *
		 * TODO: Check have a big enough tracer array to just simply do this
		 * */
		phytoBiomass = k_w_shallow;
		for (guild = 0; guild < bm->K_num_tot_sp; guild++) {
			/* Normal primary producer groups are not defined in ice - would need to change code if this is required. */
			if(FunctGroupArray[guild].habitatType == ICE_BASED && FunctGroupArray[guild].isPrimaryProducer == TRUE){
//			if (FunctGroupArray[guild].isPrimaryProducer == TRUE){
//				if (FunctGroupArray[guild].groupType == LG_PHY || FunctGroupArray[guild].groupType == SM_PHY || FunctGroupArray[guild].groupType == DINOFLAG
//						|| FunctGroupArray[guild].groupType == MICROPHTYBENTHOS) {
					if(Y1[FunctGroupArray[guild].totNTracers[0]] > bm->min_pool)
						phytoBiomass += Y1[FunctGroupArray[guild].totNTracers[0]];
				//}
			}
		}

		k_a = ka_star * phytoBiomass;// / ice->dz[i];

/*
		if(i == 0){
			lighttop = lightbot;	// Surface ice layer
		} else {
			lighttop = light_surf * (1.0 - albedo_ice * ice_cover) * exp(k_a * icedepth); // Irradiance at the top of the ice layer
		}

		icedepth += ice->dz[i];

		if(i <  (ice->currentnz - 1)){
			lighttop = light_surf * (1.0 - albedo_ice * ice_cover) * exp(- k_a * icedepth); // Irradiance at the top of the ice layer
		}
*/

		step1 = -1.0 * (k_bs * snow_cover + k_bi * ice_cover);
		step2 = -1.0 * (k_rs * snow_cover + k_ri * ice_cover);

		lightbot = lighttop * (R_bi * exp(step1) + (1 - R_bi) * exp(step2)) * exp(-k_a * ice->dz[i]); 	// Irradiance at the bottom of the layer

		switch (bm->flaglightopt) {
		case light_orig:
		case light_mid:
			if (ice->dz[i] != 0 && k_a > 1.0) {
				Y1[Light_i] = max(small_num,(lighttop-lightbot)/(k_a * ice->dz[i]));
			} else
				Y1[Light_i] = max(small_num,lighttop);

			break;
		case light_bott:
			Y1[Light_i] = max(small_num,lightbot);
			break;
		case light_top:
			Y1[Light_i] = max(small_num,lighttop);
			break;
		default:
			quit("No such flaglightopt setting\n");
			break;
		}
		lighttop = lightbot;
	}

	ice->bottom_light = lightbot;
}

/*
 * brief\ Temperature corrections to parameters for ice dependent species
 *
 * Note that in Gary's Griffiths model (Griffith & Spitz 2009)
 * Ice dwelling species = Nitzschia with optimal temperature of 0.0
 * Sub-ice species = Melosira with optimal temperature pf 2.0
 *
 * Uses the peak growth rate calculations in Griffith & Spitz 2009
 * Tdep_di=log(2)*0.851*(1.066.^T_i).*exp(-((abs(T_i-T_opt)).^3)./1000)
 *
 *
 */

void Box_Ice_Q10(MSEBoxModel *bm, Box *pBox, int ice_layer, FILE *llogfp)
{
	int sp, cohort, stage;
	int sp_q10eff;
	double current_temp = 0.0;
	double local_current_corr = 1.0;
	double Tscalar = 0.0;
	IceModel *ice = &pBox->ice;

	/* Get current temperature */
	current_temp = ice->tr[ice_layer][Temp_i];

	/* Set ice dwelling species parameters - starting with the algae */
	for (sp = 0; sp < bm->K_num_tot_sp; sp++) {
		if ((FunctGroupArray[sp].habitatType == ICE_BASED) && (FunctGroupArray[sp].speciesParams[flag_id] == TRUE)){
			FunctGroupArray[sp].Tcorr = Get_Tcorr(bm, sp, current_temp, &local_current_corr);

			/* Include pH and salinity modifiers if desired
			 *
			 * TODO: Once know how to get salinity and alkalinity in the ice

			sensitive_sp = (int) (FunctGroupArray[sp].speciesParams[flagpHsensitive_id]);
			FunctGroupArray[sp].Scorr = Get_Scorr(bm, sp, sensitive_sp, current_SALT);
			FunctGroupArray[sp].pHcorr = Get_pHcorr(bm, sp, sensitive_sp, current_PH, current_box, current_layer);

			For now just make them 1.0
			*/
			FunctGroupArray[sp].Scorr = 1.0;
			FunctGroupArray[sp].pHcorr = 1.0;

            FunctGroupArray[sp].PolluteCorr = Get_Pollutant_Corrections(bm, sp, pBox->n, ice_layer);
            
			/* Correct the rate parameters for primary producers */
			if (FunctGroupArray[sp].isPrimaryProducer == TRUE){
				FunctGroupArray[sp].speciesParams[KI_id] = FunctGroupArray[sp].speciesParams[KI_T15_id] * FunctGroupArray[sp].Tcorr * FunctGroupArray[sp].Scorr * FunctGroupArray[sp].pHcorr;
			}
			/* For invertebrates */
			if (FunctGroupArray[sp].isVertebrate == FALSE && FunctGroupArray[sp].speciesParams[flag_id] == TRUE) {
				for (cohort = 0; cohort < FunctGroupArray[sp].numCohorts; cohort++) {

                    FunctGroupArray[sp].scaled_C[cohort] = FunctGroupArray[sp].SP_C[cohort] * FunctGroupArray[sp].Tcorr * FunctGroupArray[sp].Scorr * FunctGroupArray[sp].pHcorr * FunctGroupArray[sp].PolluteCorr;
					FunctGroupArray[sp].scaled_mum[cohort] = FunctGroupArray[sp].mum[cohort] * FunctGroupArray[sp].Tcorr * FunctGroupArray[sp].Scorr * FunctGroupArray[sp].pHcorr * FunctGroupArray[sp].PolluteCorr;

					if(!(_finite(FunctGroupArray[sp].scaled_C[cohort]))){
						quit("SP_C is not finite for species %s cohort %d\n", FunctGroupArray[sp].groupCode, cohort);
					}
					if(isnan(FunctGroupArray[sp].scaled_C[cohort])){
						quit("SP_C is NaN for species %s cohort %d\n", FunctGroupArray[sp].groupCode, cohort);
					}
				}
				FunctGroupArray[sp].speciesParams[mS_id] = FunctGroupArray[sp].speciesParams[mS_T15_id] * FunctGroupArray[sp].Tcorr * FunctGroupArray[sp].Scorr * FunctGroupArray[sp].pHcorr * FunctGroupArray[sp].PolluteCorr;
			} else {
			/* For vertebrates */
				FunctGroupArray[sp].speciesParams[vla_id] = FunctGroupArray[sp].speciesParams[vla_T15_id] * FunctGroupArray[sp].Tcorr * FunctGroupArray[sp].Scorr * FunctGroupArray[sp].pHcorr * FunctGroupArray[sp].PolluteCorr;

				sp_q10eff = (int) FunctGroupArray[sp].speciesParams[flagq10eff_id];
				if(FunctGroupArray[sp].Tcorr < 1.0)
					Tscalar = FunctGroupArray[sp].Tcorr;
				else
					Tscalar = 1.0 / FunctGroupArray[sp].Tcorr;

				if ((sp_q10eff == 1) && (local_current_corr < 0.0)) {
					FunctGroupArray[sp].speciesParams[E1_id] = FunctGroupArray[sp].speciesParams[E1orig_id] * Tscalar * FunctGroupArray[sp].Scorr * FunctGroupArray[sp].pHcorr;
					FunctGroupArray[sp].speciesParams[E2_id] = FunctGroupArray[sp].speciesParams[E2orig_id] * Tscalar * FunctGroupArray[sp].Scorr * FunctGroupArray[sp].pHcorr;
					FunctGroupArray[sp].speciesParams[E3_id] = FunctGroupArray[sp].speciesParams[E3orig_id] * Tscalar * FunctGroupArray[sp].Scorr * FunctGroupArray[sp].pHcorr;
					FunctGroupArray[sp].speciesParams[E4_id] = FunctGroupArray[sp].speciesParams[E4orig_id] * Tscalar * FunctGroupArray[sp].Scorr * FunctGroupArray[sp].pHcorr;
				} else if ((sp_q10eff == 2) && (local_current_corr > 0.0)) {
					FunctGroupArray[sp].speciesParams[E1_id] = FunctGroupArray[sp].speciesParams[E1orig_id] * Tscalar * FunctGroupArray[sp].Scorr * FunctGroupArray[sp].pHcorr;
					FunctGroupArray[sp].speciesParams[E2_id] = FunctGroupArray[sp].speciesParams[E2orig_id] * Tscalar * FunctGroupArray[sp].Scorr * FunctGroupArray[sp].pHcorr;
					FunctGroupArray[sp].speciesParams[E3_id] = FunctGroupArray[sp].speciesParams[E3orig_id] * Tscalar * FunctGroupArray[sp].Scorr * FunctGroupArray[sp].pHcorr;
					FunctGroupArray[sp].speciesParams[E4_id] = FunctGroupArray[sp].speciesParams[E4orig_id] * Tscalar * FunctGroupArray[sp].Scorr * FunctGroupArray[sp].pHcorr;
				}

//				sensitive_sp = (int) (FunctGroupArray[sp].speciesParams[flagpHsensitive_id]);
//				if(sensitive_sp) {
//					for(cohort = 0; cohort < FunctGroupArray[sp].numCohorts; cohort++){
//						FunctGroupArray[sp].scaled_C[cohort] *= (FunctGroupArray[sp].Tcorr * FunctGroupArray[sp].Scorr * FunctGroupArray[sp].pHcorr * FunctGroupArray[sp].PolluteCorr);
//						FunctGroupArray[sp].scaled_mum[cohort] *= (FunctGroupArray[sp].Tcorr * FunctGroupArray[sp].Scorr * FunctGroupArray[sp].pHcorr * FunctGroupArray[sp].PolluteCorr);
//					}
//				}
			}


			/* Mortality */
			switch (FunctGroupArray[sp].groupAgeType) {
			case BIOMASS: /* Intentional follow though */
			case AGE_STRUCTURED:
			case AGE_STRUCTURED_BIOMASS:
                for (stage = 0; stage < FunctGroupArray[sp].numStages; stage++){
                    FunctGroupArray[sp].cohortSpeciesParams[stage][mL_id] = FunctGroupArray[sp].cohortSpeciesParams[stage][mL_T15_id] * FunctGroupArray[sp].Tcorr * FunctGroupArray[sp].Scorr * FunctGroupArray[sp].pHcorr * FunctGroupArray[sp].PolluteCorr;
                    FunctGroupArray[sp].cohortSpeciesParams[stage][mQ_id] = FunctGroupArray[sp].cohortSpeciesParams[stage][mQ_T15_id] * FunctGroupArray[sp].Tcorr * FunctGroupArray[sp].Scorr * FunctGroupArray[sp].pHcorr * FunctGroupArray[sp].PolluteCorr;
                }
				break;
			}
		}
	}
}

/***************************************** ECOLOGICAL ASPECTS ***********************************************************
 *
 * brief\ Day to day ice algal production
 *
 *	Kdi_N = 0.9	    Walsh (2005) & Jin (2006) mmol N m^-3 mapped to KN_id
 *	Wdi = 1.462     Nitrogen \PSI Eslinger et al 2001 mmol N m^-3
 *
 *	w_di = 6.6		Sinking rate of Nitzschia - metres per day from Haecky et al (1998)
 *
 *
 */
void Ice_PrimaryProduction(MSEBoxModel *bm, FILE *llogfp, int sp_id, int micro_case, int lim_case, int macro_producer, double sp, double DIN, double NH,
		double NO, double Si, double Fe, double P, double PRatio, double IRR, double pH, double E_sp, double mL_other, double SPmax, double area_hab, double *spUptakeNO, double *spUptakeSi,
		double *spUptakeFe, double *sphN) {

	double hICE_sp, amt_irr;
	double hN_sp, hI_sp, sp_grow, scale_uptake, uptakeNH, uptakeNO, uptakeSi, uptakeFe;
	double mL_sp = FunctGroupArray[sp_id].cohortSpeciesParams[0][mL_id] + mL_other;
    double mS_sp = Acidif_Mort(bm, sp_id);
	double prop_daylight;
	double P0 = FunctGroupArray[sp_id].speciesParams[P_min_internal_id];
	double P1 = FunctGroupArray[sp_id].speciesParams[P_max_internal_id];

	/* Check if enough light for photosynthesis
	 *
	 * Hourly average irradiance is above 0.8 umol m-2s-1
	 * The 4.56 converts Wm-2 to uEinst m-2s-1 (Rao & Platt, 1984)
	 */
	if((4.56 * IRR) > 0.8)
		amt_irr = IRR;
	else
		amt_irr = small_num;
	prop_daylight = bm->boxes[bm->current_box].prop_light_time;
	hI_sp = Light_Lim(bm, irr_ice, amt_irr, FunctGroupArray[sp_id].speciesParams[KI_id], FunctGroupArray[sp_id].speciesParams[Beta_D_id],
					prop_daylight, FunctGroupArray[sp_id].speciesParams[PBmax_D_id]);

	/* Nutrient limitation
	 * TODO: Add phosphorous limitation
	 */
	hN_sp = Nutrient_Lim(bm, lim_case, micro_case, DIN, Si, Fe, P, PRatio, P0, P1, FunctGroupArray[sp_id].speciesParams[KN_id], FunctGroupArray[sp_id].speciesParams[KS_id],
			FunctGroupArray[sp_id].speciesParams[KF_id], FunctGroupArray[sp_id].speciesParams[KP_id]);

	*sphN = hN_sp;

	/* Ice limitation - no ice then no ice algal growth */
	if(bm->boxes[bm->current_box].has_ice > 0){
		hICE_sp = 1.0;
	} else {
		hICE_sp = 0.0;
	}

	/* eddy_strength is a value per box. Think this is read in from the bgm file?
	 * Eddy_scale is an input parameter from the input biology file
	 */
	/* Calculate resulting growth */
	sp_grow = sp * FunctGroupArray[sp_id].scaled_mum[0] * hN_sp * hI_sp * hICE_sp;

	//*sp_growth = sp_grow;
	FunctGroupArray[sp_id].growth[0] = sp_grow;

	/* Calculate nutrient uptake */
	uptakeNH = sp_grow * (NH / (FunctGroupArray[sp_id].speciesParams[KN_id] + NH)) * ((FunctGroupArray[sp_id].speciesParams[KN_id] + DIN) / DIN);
	uptakeNO = sp_grow * (NO / (FunctGroupArray[sp_id].speciesParams[KN_id] + DIN)) * (FunctGroupArray[sp_id].speciesParams[KN_id]
			/ (FunctGroupArray[sp_id].speciesParams[KN_id] + NH)) * (1.0 + FunctGroupArray[sp_id].speciesParams[KN_id] / DIN);

	if (lim_case != one_nut_lim)
		uptakeSi = sp_grow * bm->X_SiN;
	else
		uptakeSi = 0.0;

	if (micro_case)
		uptakeFe = sp_grow * bm->X_FeN;
	else
		uptakeFe = 0.0;

	/* Cap nutrient uptake to what is available - this intentionally a max as it is rescaling the NH uptake based on which is the least limitign of the limiting nutients */
	scale_uptake = 1.0;
	if (uptakeNH > NH)
		scale_uptake = NH / uptakeNH;
	if (uptakeNO > NO)
		scale_uptake = max(scale_uptake, NO / uptakeNO);
	if (uptakeSi > Si)
		scale_uptake = max(scale_uptake, Si / uptakeSi);
	if (uptakeFe > Fe)
		scale_uptake = max(scale_uptake, Fe / uptakeFe);

	/* Assign uptake */
	FunctGroupArray[sp_id].uptakeNH[0] = uptakeNH * scale_uptake;
	*spUptakeNO = uptakeNO * scale_uptake;
	*spUptakeSi = uptakeSi * scale_uptake;
	*spUptakeFe = uptakeFe * scale_uptake;

	/* Calculate mortality and lysis */
	FunctGroupArray[sp_id].mortality[0] = (mL_sp + mS_sp) * sp;
	FunctGroupArray[sp_id].lysis[0] = FunctGroupArray[sp_id].speciesParams[KLYS_id] * sp / (hN_sp + 0.1);

	/* For dinoflagellates calculate maximum growth rate possible if nutrients non-limiting */
	if (FunctGroupArray[sp_id].groupType == DINOFLAG)
		FunctGroupArray[sp_id].maxPhagotrophy = sp * (FunctGroupArray[sp_id].scaled_mum[0] / E_sp) * hI_sp * (1.0 - hN_sp);
	else
		FunctGroupArray[sp_id].maxPhagotrophy = 0.0;
}

/*
 * brief\ Ice related habitat state
 *
 */
void Ice_HabitatState(MSEBoxModel *bm, FILE *llogfp) {


	/* Currently read in from external driving file - read in within physics library
	 * Individual species scaling of habitat attractiveness
	 * is based on their habitat needs
	 *
	 * TODO: Other specific ice processes of ecological relevance?
	 *
	 * Currently stored in bm->boxes[box_id].ice.ice_classes[layer_id][iceclass_id]
	 *
	 * assuming 0.0 means that iceclass not present and value >0.0 is habitat
	 * presence and quality scalar
	 */


	return;
}

/*
 * brief\ Ice related habitat use by age structured groups
 *
 */
double Get_Ice_Rating(MSEBoxModel *bm, int sp) {
    double ans = 0.0;
    double tot_ice = 0.0, tot_ice_possible = 0.0;
    int i, z, b;
    int ice_dependent = 0;
    double sp_likeICE;

    tot_ice = 0.0;
    tot_ice_possible = 0.0;
    for(i=0; i < bm->K_num_ice_classes; i++){
        sp_likeICE = (double)(bm->ICE_HABITATlike[sp][adult_id][i]);
        for(b=0; b<bm->nbox; b++) {
            if (bm->boxes[b].type != BOUNDARY) {
                for(z=0; z<bm->icenz; z++){
                    tot_ice += bm->boxes[b].ice.ice_classes[z][i] * sp_likeICE;
                    tot_ice_possible += (1.0 * sp_likeICE);
                }
            }
        }
    }
    
    if(!tot_ice_possible){
        tot_ice_possible = 1.0;
    }

    ans = tot_ice / tot_ice_possible;
    return ans;
}


/*
 * brief\ Ice related habitat use by age structured groups
 *
 */
double Get_Ice_Vertebrate_Habitat_Rating(MSEBoxModel *bm, int guildcase, int stage, int boxin) {
	double ans = 0.0;
	double tot_ice = 0.0;
	int i, z;
	int ice_dependent = 0;
	int sp_likeICE;

	for(i=0; i < bm->K_num_ice_classes; i++){
		sp_likeICE = bm->ICE_HABITATlike[guildcase][stage][i];
		if(sp_likeICE){
			ice_dependent++;
		}

		tot_ice = 0.0; // Local total for ice of that class across layers in the box
		for(z=0; z<bm->icenz; z++){
			tot_ice += bm->boxes[boxin].ice.ice_classes[z][i];
		}

		ans += tot_ice * sp_likeICE;
	}

	if(!ice_dependent)
		ans = 1.0;		// As not ice dependent so don't downscale any refuge status

	return ans;
}

/*
 * brief\ Get movement habitat dependency
 *
 */
double Get_Ice_Presence(MSEBoxModel *bm, int sp, int stage, int ij, int k, int ***HABlike){
	int like_ice = 0, ii;
	int sp_likeICE = 0;
	double some_ice = 0.0;
	double ans = 1.0;
	double current_enviro;

	for(ii=0; ii<bm->K_num_ice_classes; ii++){
		sp_likeICE = HABlike[sp][stage][ii];
		current_enviro = bm->boxes[ij].ice_prop[k][ii];

		if(sp_likeICE > 0)
			like_ice ++;
		if((sp_likeICE > 0) && (current_enviro > 0.0)){
			some_ice += current_enviro;
		}
	}
	if(like_ice && (!some_ice)){
		ans = 0.0;
	}

	return ans;
}

/**
 *	\brief Sediment Bacteria partitioning
 */
void Calculate_IceBact_Scale(MSEBoxModel *bm, HABITAT_TYPES habitatType, BoxLayerValues *boxLayerInfo) {
	double PB, DR, DL;
	int flag_sp = (int) (FunctGroupArray[pelagicBactIndex].speciesParams[flag_id]);

	DL = boxLayerInfo->localICETracers[FunctGroupArray[LabDetIndex].totNTracers[0]];
	DR = boxLayerInfo->localICETracers[FunctGroupArray[RefDetIndex].totNTracers[0]];
	PB = boxLayerInfo->localICETracers[FunctGroupArray[IceBactIndex].totNTracers[0]];

	if ((flag_sp) && (habitatType == ICE_BASED)) {
		boxLayerInfo->ICEB_DL = PB * XPB_DL * DL / (XPB_DL * DL + XPB_DR * DR + small_num);
		boxLayerInfo->ICEB_DR = PB * XPB_DR * DR / (XPB_DL * DL + XPB_DR * DR + small_num);
	} else {
		boxLayerInfo->ICEB_DL = 0.0;
		boxLayerInfo->ICEB_DR = 0.0;
	}
}

/**
 *  \brief The process function for the PelagicBacteria functional group.
 *
 *
 ***
 Ice bacteria

 Based on pelagic bacteria for now
 *
 *
 */
int Ice_Bacteria_Process(MSEBoxModel *bm, FILE *llogfp, HABITAT_TYPES habitatType, int guild, int cohort, BoxLayerValues *boxLayerInfo) {
	double *tracerArray;
	double NH, PB, DL, DR, O2, hO_SP, potential_PB;
	double mL_sp = Ecology_Get_Linear_Mortality(bm, guild, cohort, cohort);
    double mS_sp = Acidif_Mort(bm, guild);

	if ((int) (FunctGroupArray[guild].speciesParams[flag_id])) {

		tracerArray = getTracerArray(boxLayerInfo, habitatType);

		O2 = tracerArray[Oxygen_i];
		NH = tracerArray[NH3_i];
		DL = tracerArray[FunctGroupArray[LabDetIndex].totNTracers[0]];
		DR = tracerArray[FunctGroupArray[RefDetIndex].totNTracers[0]];
		PB = tracerArray[FunctGroupArray[guild].totNTracers[0]];

		if (bm->debug == debug_prey_biology_process && bm->dayt >= bm->checkstart && bm->dayt < bm->checkstop) {
			fprintf(
					llogfp,
					"Ice_Bacteria_Process parameters: mL_sp = %e, FunctGroupArray[guild].speciesParams[mO_id] = %e, FunctGroupArray[guild].speciesParams[E3_id] = %e, FunctGroupArray[guild].speciesParams[FDMort_id] = %e\n",
					mL_sp, FunctGroupArray[guild].speciesParams[mO_id], FunctGroupArray[guild].speciesParams[E3_id],
					FunctGroupArray[guild].speciesParams[FDMort_id]);
		}

		hO_SP = Oxygen(2, O2, FunctGroupArray[guild].speciesParams[KO2_id], FunctGroupArray[guild].speciesParams[KO2LIM_id],
				FunctGroupArray[guild].speciesParams[mD_id]);


		if (flagkdrop)
			potential_PB = FunctGroupArray[guild].scaled_mum[cohort] * (boxLayerInfo->ICEB_DL * (1.0 - pow(boxLayerInfo->ICEB_DL / (XPB_DL * DL), 3.0))
					+ boxLayerInfo->ICEB_DR * (1.0 - pow(boxLayerInfo->ICEB_DR / (XPB_DR * DR), 3.0)));
		else {
			potential_PB = FunctGroupArray[guild].scaled_mum[cohort] * PB * pow(max(0.0,(1.0 - PB / (XPB_DL * DL
									+ XPB_DR * DR + small_num))), k_PB);
		}

		FunctGroupArray[guild].growth[cohort] = max(0.0, potential_PB);

		FunctGroupArray[guild].uptakeDL = FunctGroupArray[guild].growth[cohort] * boxLayerInfo->ICEB_DL / (PB
				* FunctGroupArray[guild].speciesParams[E3_id] + small_num);
		FunctGroupArray[guild].uptakeDR = FunctGroupArray[guild].growth[cohort] * boxLayerInfo->ICEB_DR / (PB
				* FunctGroupArray[guild].speciesParams[E4_id] + small_num);
		FunctGroupArray[guild].mortality[cohort] = ((mL_sp + mS_sp) + (1.0 - hO_SP) * FunctGroupArray[guild].speciesParams[mO_id]) * PB;

		FunctGroupArray[guild].prodnDR[cohort] = (FunctGroupArray[guild].uptakeDL * (1.0
				- FunctGroupArray[guild].speciesParams[E3_id]) + FunctGroupArray[guild].mortality[cohort]
				* FunctGroupArray[guild].speciesParams[FDMort_id]) * FPB_DR;

		FunctGroupArray[guild].prodnDON = (FunctGroupArray[guild].uptakeDL * (1.0 - FunctGroupArray[guild].speciesParams[E3_id])
				+ FunctGroupArray[guild].uptakeDR * (1.0 - FunctGroupArray[guild].speciesParams[E4_id])
				+ FunctGroupArray[guild].mortality[cohort] * FunctGroupArray[guild].speciesParams[FDMort_id]) * FPB_DON * (1.0 - FPB_DR);
		FunctGroupArray[guild].releaseNH[cohort] = (FunctGroupArray[guild].uptakeDL * (1.0
				- FunctGroupArray[guild].speciesParams[E3_id])) * (1.0 - FPB_DR - FPB_DON * (1.0 - FPB_DR))
				+ FunctGroupArray[guild].uptakeDR * (1.0 - FunctGroupArray[guild].speciesParams[E4_id]) * (1.0 - FPB_DON * (1.0 - FPB_DR))
				+ FunctGroupArray[guild].mortality[cohort] * (1.0 - FunctGroupArray[guild].speciesParams[FDMort_id] * FPB_DR
						- FunctGroupArray[guild].speciesParams[FDMort_id] * FPB_DON * (1.0 - FPB_DR));

		FunctGroupArray[guild].nitrif = K_nit * NH * Susp_Sed / (K_conc + small_num);

		/* Diagnostic information storage */
		boxLayerInfo->DebugInfo[guild][ICE_BASED][DiagnostNH_id] = FunctGroupArray[guild].releaseNH[cohort]
				- FunctGroupArray[guild].nitrif;
		boxLayerInfo->DebugInfo[guild][ICE_BASED][DiagnostDL_id] = -FunctGroupArray[guild].uptakeDL;
		boxLayerInfo->DebugInfo[guild][ICE_BASED][DiagnostDR_id] = FunctGroupArray[guild].prodnDR[cohort]
				- FunctGroupArray[guild].uptakeDR;

		boxLayerInfo->DebugFluxInfo[guild][habitatType][gain_id] = FunctGroupArray[guild].growth[cohort];
		boxLayerInfo->DebugFluxInfo[guild][habitatType][loss_id] = FunctGroupArray[guild].mortality[cohort];

		if (bm->debug == debug_prey_biology_process && bm->dayt >= bm->checkstart && bm->dayt < bm->checkstop) {
			fprintf(
					llogfp,
					"Ice_Bacteria_Process outcomes: growth = %e, uptakeDL = %e, uptakeDR = %e, mortality = %e, prodnDR = %e, prodnDON = %e, releaseNH = %e, nitrif = %e \n",
					FunctGroupArray[guild].growth[cohort], FunctGroupArray[guild].uptakeDL,
					FunctGroupArray[guild].uptakeDR, FunctGroupArray[guild].mortality[cohort],
					FunctGroupArray[guild].prodnDR[cohort], FunctGroupArray[guild].prodnDON,
					FunctGroupArray[guild].releaseNH[cohort], FunctGroupArray[guild].nitrif);
		}

	}
	return TRUE;
}
/**
 *	\brief Availability in the Ice
 *
 */
static double Ice_Avail(MSEBoxModel *bm, double sp, double Depth) {
	double BotDpth, TopDpth, PercentDpth, avail_sp;
	int i;

	BotDpth = 0;
	for(i = 0; i < bm->current_icelayer; i++){
		BotDpth += bm->boxes[bm->current_box].ice.dz[i];
	}
	TopDpth = BotDpth + bm->boxes[bm->current_box].ice.dz[bm->current_icelayer];
	if (TopDpth < 0.0)
		TopDpth = 0.0;

	PercentDpth = (Depth - TopDpth) / bm->boxes[bm->current_box].ice.dz[bm->current_icelayer];

	/* Layer completely above depth down to which DR completely available to that
	 species then all detritus is available. */
	if (BotDpth <= Depth)
		avail_sp = sp;
	else if (TopDpth < Depth)
		/* Layer spans the depth down to which DR completely available to that species
		 then only a % of the detritus is available. (=% layer above exploitation depth).
		 */
		avail_sp = sp * PercentDpth;
	else
		/* Layer below the depth down to which DR completely available to that species
		 then no detritus is available. */
		avail_sp = 0.0;

	return avail_sp;
}

/**
 *	\brief Availability of ice groups to predators.
 *
 */
void Calculate_Ice_Prey_Avail(MSEBoxModel *bm, BoxLayerValues *boxLayerInfo,  int guild, double ***spPREYinfo, double *avail_IceBact) {
	double biomass;
	double ICE_KDEP_sp = FunctGroupArray[guild].speciesParams[ICE_KDEP_id];
	int prey;

	*avail_IceBact = 0;

	for (prey = 0; prey < bm->K_num_tot_sp; prey++) {
		if (FunctGroupArray[prey].speciesParams[flag_id] == TRUE) {

			if(FunctGroupArray[prey].habitatCoeffs[ICE_BASED] > 0){

				if(FunctGroupArray[prey].groupType == ICE_BACT){
					/* Assume for now that bacteria in ice is not dependant on depth of nutrient */
					*avail_IceBact = 1.0;
					continue;
				}else{
					biomass = boxLayerInfo->localICETracers[FunctGroupArray[prey].totNTracers[0]];

					/* Find availability */
					spPREYinfo[prey][0][ICE_BASED] = Ice_Avail(bm, biomass, ICE_KDEP_sp);
				}

			}
		}
	}
	return;
}
