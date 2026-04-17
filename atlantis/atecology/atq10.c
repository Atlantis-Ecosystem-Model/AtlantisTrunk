/**
 \file
 \brief C file for process subroutines for south east ecological model
 \ingroup atEcology

 Temperature dependence
 All rate parameters p are assumed to depend on temperature T, according to a
 prescribed Q10. The prescribed value is assumed to apply at 15 C, so that
 the correction factor is

 Tcorr = Q10**((T-15)/10.)

 The default Q10 for all parameters is 2.0. We only compute Tcorr once, and
 apply it to all parameters by default. We treat parameters for which Q10 is
 not 2 as a special case.

 File:	atq10.c
 Author:	Beth Fulton
 Created:	21/9/2004
 Purpose:	C file for process subroutines for south east ecological model
 Revisions: 8/8/2004 Created mseq10.c from the seecomproc.c file

 21/9/2004 Created atq10.c from mseq10.c

 22/9/2004 Added calculation of temp_influence for temperature
 effects on spawning (particularly of invertebrates)

 15/10/2004 Added BML and BFF.

 19/10/2004 Added Prawns

 26/4/2005 Added Dinoflagellates

 10-06-2009 Bec Gorton
 Added the SEAGRASS invertebrate type.

 05-11-2009 Bec Gorton
 Trunk Merge 1064 - Added support to calculate temperature correction value for each functional
 group separately. A new species parameter is read in from the input biology file and stored
 in the q10_id slot in the speciesParams array. This is used in the tcorr calulation. The tcorr value
 for each functional group is stored in the FunctGroupArray structure - this value is used in the
 reproduction calculations if the species reproduction corrected for temperature.

 05-05-2010 Bec Gorton
 Added code for Gary Griffiths new temperature correction algorithm. See the wiki
 for information about how this works.

 10-05-2010 Bec Gorton
 Added the Scale_Linear_Mortality function to scale the linear mortality values.

 14-05-2012 Beth Fulton
 Added stuff for Savina seagrass model - including totagepop so can get age structured overall output

 **/

/* Revisions
 * 31 Aug 2012 - adjusted the temperature model based on Griffith, had the scalar outside the exp(), which was wrong
 *   Had reversed current_temp vs coefficient in pow() in step 1 too, but reversed it as was correct originally.
 */

/*  Open library routines */
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <sjwlib.h>
#include "atecology.h"

static void Scale_Cohort_Linear_Mortality(MSEBoxModel *bm, FILE *llogfp, int speciesIndex, int ageClass, int boxID) {
	int mL_scale_index = -1;
	double environScaler = 1.0, scalingFactor = 1.0, scaledmL, origmL;

	mL_scale_index = (int)FunctGroupArray[speciesIndex].cohortSpeciesParams[ageClass][mL_scale_id];
    origmL = FunctGroupArray[speciesIndex].cohortSpeciesParams[ageClass][mL_id];
    scaledmL = 1.0 * origmL;

	//printf("sp = %d, cohort = %d, mL_scale_index = %d\n", speciesIndex, ageClass, mL_scale_index);
	/* Apply environmental forcing to this group */
	if (mL_scale_index != -1) {
		environScaler = tsEval(bm->tslinearMort, mL_scale_index, bm->t);
	}

	/* Now get the scaling factor to apply */
	if (numMortChanges[speciesIndex][ageClass][boxID] > 0) {
		scalingFactor = Util_Get_Accumulative_Change_Scale(bm, numMortChanges[speciesIndex][ageClass][boxID], LinearMortChange[speciesIndex][ageClass][boxID]);
	}
    scaledmL = origmL * environScaler * scalingFactor;
    FunctGroupArray[speciesIndex].cohortSpeciesParams[ageClass][mL_id] = scaledmL;
}

/**
 * \brief Scale the linear mortality based on either values in the LinearMortChange array or values
 * in the time series ts file.
 *
 *
 */
static void Scale_Linear_Mortality(MSEBoxModel *bm, FILE *llogfp, int boxID) {
	int speciesIndex, stage;

	/* Now also allow for other factors that effect mortality such as changes due to environmental forcing
	 * and any changes specified in the change array
	 */
	for (speciesIndex = 0; speciesIndex < bm->K_num_tot_sp; speciesIndex++) {

		if (FunctGroupArray[speciesIndex].groupAgeType == BIOMASS) {
			Scale_Cohort_Linear_Mortality(bm, llogfp, speciesIndex, 0, boxID);
		} else {
			for(stage = 0; stage < FunctGroupArray[speciesIndex].numStages; stage++)
				Scale_Cohort_Linear_Mortality(bm, llogfp, speciesIndex, stage, boxID);
		}
	}
}


/**
 * \brief Computing temperature sensitive parameters based on time
 *
 */
void Parameter_Q10(MSEBoxModel *bm, Box *pBox, double dayt, int numwclayer, double cdepth, int clayer, double midpoint,
		int flagmodel, FILE *llogfp) {

	int sp;
	//if (verbose > 0)
	//	printf("Doing param Q10\n");

	/* get the temperature at this depth */
	Properties_At_Depth(bm, pBox, dayt, numwclayer, cdepth, clayer, midpoint, flagmodel, llogfp);


	/* Calculate the default Tcorr value and current temperature for the species Tcorr calculations
     Note the line was bm->current_corr = (H2Otemp - 15.0) / 10.0; 
     but generalised to baseline_temp
     
     */
    bm->current_corr = (H2Otemp - bm->baseline_temp) / 10.0;
	if (bm->flagq10) {
		bm->Tcorr = pow(Q10, bm->current_corr);
	} else
		bm->Tcorr = 1.0;

	/**
	 if(verbose)
	 fprintf(llogfp,"Checking for temperature corrections in box: %d, (day: %e) with temp %e\n",
	 bm->current_box, bm->dayt, H2Otemp);

	 **/
	/* Temperature influence on recruitment */
	bm->temp_influence = bm->Tcorr;

	/* Calculate the TCorr value for each group */
	for (sp = 0; sp < bm->K_num_tot_sp; sp++) {
		FunctGroupArray[sp].Tcorr = Get_Tcorr(bm, sp, H2Otemp, &bm->current_corr);
        FunctGroupArray[sp].TcorrEff = FunctGroupArray[sp].Tcorr;
        
		/** Include pH and salinity modifiers if desired **/
		FunctGroupArray[sp].Scorr = Get_Scorr(bm, sp, current_SALT);
		FunctGroupArray[sp].pHcorr = Get_pHcorr(bm, sp, current_PH, pBox->n, clayer);
        
        FunctGroupArray[sp].PolluteCorr = Get_Pollutant_Corrections(bm, sp, pBox->n, clayer);
	}

	/** Apply parameter corrections **/
	/* If we are not tracking contaminants then do q10 calcs now - otherwise they will be done in WaterColumnBox etc each iteration to allow for changes in contaminant levels */
	if(bm->track_contaminants == FALSE){
		Apply_Q10_Corrections(bm);
	}

	/* Now also allow for other factors that effect mortality such as changes due to environmental forcing
	 * and any changes specified in the change array
	 */
	Scale_Linear_Mortality(bm, llogfp, pBox->n);

	return;
}

/*
 * \brief Get the temperature corrections to apply to ecophysiology given current water temperature
 * Note that from Griffiths original model fitting exercise
 * K_temp_const_A = 0.851
 * K_temp_const_B = 1.066
 *
 * Equation from Gary G - Tdep_di=log(2)*0.851*(1.066.^T_i).*exp(-((abs(T_i-T_opt_nitzschia)).^3)./1000);
 */
double Get_Tcorr(MSEBoxModel *bm, int sp, double current_temp, double *current_corr) {
	double ans = 1.0;
	double step1, step2, step3, step4, stepA, stepB, stepC;
	double opt_temp;
	double temp_const_A = 0.0, temp_const_B = 0.0;
    int q10flag = (int)(FunctGroupArray[sp].speciesParams[q10_method_id]);

	if (bm->flagq10) {
		/* Now check the type of q10 correction that we are doing for this group */
		switch (q10flag) {
            case base_q10_id: /* Basic q10 relationship */
                ans = (double)pow(FunctGroupArray[sp].speciesParams[q10_id], (*current_corr));
                break;
            case humped_griffith_q10_id: /* Humped shape from Hary Griffith */
                // Equation from Gary G - Tdep_di=log(2)*0.851*(1.066.^T_i).*exp(-((abs(T_i-T_opt_nitzschia)).^3)./1000);
                temp_const_A = (double) FunctGroupArray[sp].speciesParams[temp_coefftA_id];
                opt_temp = (double) FunctGroupArray[sp].speciesParams[q10_optimal_temp_id];
                step1 = log(2) * temp_const_A * pow(bm->temp_const_B,current_temp);
                step2 = exp(-bm->temp_const_C * (pow(fabs(current_temp - opt_temp), bm->temp_const_D) / FunctGroupArray[sp].speciesParams[q10_correction_id]));
                *current_corr = current_temp - opt_temp;

                ans = step1 * step2;
                
                //                fprintf(bm->logFile, "FunctGroupArray[sp].speciesParams[q10_correction_id] = %e, opt_temp = %e\n",FunctGroupArray[sp].speciesParams[q10_correction_id], opt_temp);
                //                fprintf(llogfp, "ParameterQ10 - opt_temp = %e, current_temp, = %e, FunctGroupArray[%d].Tcorr  = %e, step1 = %e, step2 = %e\n",opt_temp, current_temp, sp, FunctGroupArray[sp].Tcorr, step1, step2);
                break;
            case Heinichen_q10_id:
                /* Equation 4 from Heinichen et al. referencing Blanchard et al. 2012 referencing Gillooly et al. 2001; & Brown et al. 2004 */
                // Tau = 2.71^(25.55-(0.63/((0.0000863*TinK[i]))))
                step1 = 0.0000863 * (current_temp + 273.15); // Caren added: These constants are: Boltzman constant (0.0000863) & temperature conversion from Celsius to Kelvin (+273.15)
                step2 = 25.55 - (0.63 / step1); // Caren added: These are values for activation energy constant (0.63) and a constant: c1=25.55
                      
                step3 = exp(step2);

                /*
                if(sp == 45) {
                  fprintf(bm->logFile, "Tau check: Day: %e, Species: %s, current_temp: %e, step1: %e, step2: %e, step3: %e, ans: %e", bm->dayt, FunctGroupArray[sp].groupCode, current_temp, step1, step2, step3);
                }
                 */
               
                opt_temp = FunctGroupArray[sp].speciesParams[q10_optimal_temp_id];
               
                // Equation #5 from Heinichen et al. 2022
                stepA = 0.0000863 * (opt_temp + 273.15);
                stepB = 25.55 - (0.63 / stepA);
                stepC = exp(stepB);
               
                ans = 1.0 / (step3 / stepC); //scaled Tau
                
                /*
                if(sp == 45) {
                  fprintf(bm->logFile, "Scaled Tau check: Day: %e, Species: %s, opt_temp: %e, stepA: %e, stepB: %e, stepC: %e, ans: %e", bm->dayt, FunctGroupArray[sp].groupCode, opt_temp, stepA, stepB, stepC, ans);
                }
                */

                break;
            case CEATTLE_q10_id:
                // Equation from Wisconsin model used in CEATTLE
                temp_const_A = (double) FunctGroupArray[sp].speciesParams[temp_coefftA_id];
                opt_temp = (double) FunctGroupArray[sp].speciesParams[q10_optimal_temp_id];
                temp_const_B = (double) FunctGroupArray[sp].speciesParams[q10_correction_id];
                
                step1 = log(temp_const_A)*(temp_const_B - opt_temp + 2);
                step2 = log(temp_const_A)*(temp_const_B - opt_temp);
                step3 = (pow(step2, 2) * pow((1 + pow((1 + 40/step1), 0.5)), 2)) / 400;
                step4 = (temp_const_B - current_temp)/(temp_const_B - opt_temp);
                
                ans = pow(step4, step3) * exp(step3 * (1 - step4));

                break;
            default:
                quit("How got here as code option not possible - q10 effect method can only be 0-2 and you have %d\n", (int)(FunctGroupArray[sp].speciesParams[q10_method_id]));
		}
	} else {
		ans = 1.0;
	}

    // Always has to be non-negative, but don't make it zero as have to divide by it
    if((ans < 0.0) || (!ans) || isnan(ans))
      ans = small_num;

	return ans;
}


/*
 * \brief Get the salinity based corrections to apply to ecophysiology given current water temperature
 *
 */
double Get_Scorr(MSEBoxModel *bm, int sp, double current_salt) {
	double ans = 1.0;
	double min_salt = FunctGroupArray[sp].speciesParams[min_move_salt_id];
	double max_salt = FunctGroupArray[sp].speciesParams[max_move_salt_id];

	int sensitive_sp = (int) (FunctGroupArray[sp].speciesParams[flagSaltSensitive_id]);
	if(sensitive_sp){
		// TODO: Find out if knife edge vs humped optimum form
		// For now assume knife edge

		if((current_salt < min_salt) || (current_salt > max_salt)) {
			ans = FunctGroupArray[sp].speciesParams[salt_correction_id];
		} else
			ans = 1.0;
	} else {
		ans = 1.0;
	}

	return ans;
}

/*
 * \brief Get the pH based corrections to apply to ecophysiology given current water temperature
 * For the nonlinear case information is from Hinga using Griffiths temperature formulation form.
 * Based on this form appropriate parameter forms to reproduce Hinga is
 * pH_const_A  1.0
 * pH_const_B  1.18
 * optimal_pH  8.0
 * pH_correction 15.0
 *
 */
double Get_pHcorr(MSEBoxModel *bm, int sp, double current_ph, int cbox, int clayer) {
	double ans = 1.0;
	double step1, step2;
	double opt_pH = 0;
	double KN_pH = 0;
	double pH_const_A = 0;
	double pH_const_B = 0;
    double pH_const_C = 0;
    double min_pH = 0;
    double max_pH = 0;
    double delta_H_ion = 0;
    double initial_ph = current_ph;
    int pH_sensitivity_model = (int) (FunctGroupArray[sp].speciesParams[pHsensitive_model_id]);
    
    if(bm->flag_use_deltaH) {
        // delta_H_ion [H+] being an increase in acidity/decrease in pH
        
        // Positive changes in
        if(bm->dayt == 0.0) {
            bm->init_pH[cbox][clayer] = current_ph;
        }
        initial_ph = bm->init_pH[cbox][clayer];
        
        step1 = (-1.0 * current_ph);
        step2 = (-1.0 * initial_ph);
        delta_H_ion = pow(10, step1) - pow(10, step2);
        
        // Overload current_ph here so can use existing code
        current_ph = delta_H_ion;
    }
    
    switch(pH_sensitivity_model){
	case no_pH_effect_id:   
		ans = 1.0;
		break;
	case monod_pH_id:
		pH_const_A = FunctGroupArray[sp].speciesParams[pH_constA_id];
		pH_const_B = FunctGroupArray[sp].speciesParams[pH_constB_id];
		KN_pH = FunctGroupArray[sp].speciesParams[KN_pH_id];
		if(current_ph < pH_const_A)
			step1 = 0.0;
		else
			step1 = pH_const_B * (current_ph - pH_const_A) / ((KN_pH - pH_const_A) + (current_ph - pH_const_A));
		ans = step1;
		break;
	case nonlin_pH_id:
		// Equation from Hinga
		opt_pH = FunctGroupArray[sp].speciesParams[optimal_pH_id];
		pH_const_A = FunctGroupArray[sp].speciesParams[pH_constA_id];
		pH_const_B = FunctGroupArray[sp].speciesParams[pH_constB_id];
		step1 = log10(2) * pH_const_A * pow(pH_const_B,current_ph);
		step2 = exp(-1.0 * (pow(fabs(current_ph - opt_pH), 3.0) / FunctGroupArray[sp].speciesParams[pH_correction_id]));
		ans = step1 * step2;
		break;
	case lin_pH_id:
		// Equation based on meta-analysis results of acidification
		pH_const_A = FunctGroupArray[sp].speciesParams[pH_constA_id];
		pH_const_B = FunctGroupArray[sp].speciesParams[pH_constB_id];
		ans = pH_const_A + current_ph * pH_const_B;
		break;
    case piecewise_pH_id:
        // Equation based on piece-wise linear functional shape
        /* So shape produced is:
                ______
               /
              /
         ____/
         
         Where pH_const_A if less then left hand inflection at min_pH, 
         and pH_const_B if greater than right hand inflection at max_pH
         
        */
        pH_const_A = FunctGroupArray[sp].speciesParams[pH_constA_id];
        pH_const_B = FunctGroupArray[sp].speciesParams[pH_constB_id];
        min_pH = FunctGroupArray[sp].speciesParams[min_pH_id];
        max_pH = FunctGroupArray[sp].speciesParams[max_pH_id];
        
        if(current_ph <= min_pH){
            ans = pH_const_A;
        } else if (current_ph >= max_pH){
            ans = pH_const_B;
        } else {
            ans = pH_const_A + (current_ph - min_pH) * (pH_const_B - pH_const_A) / (max_pH - min_pH); // This was just current_ph but that was an error in the interpolation
        }
        break;
    case quadratic_pH_id:
        // Equation based on meta-analysis by researchers in NOAA - Hawaii
        pH_const_A = FunctGroupArray[sp].speciesParams[pH_constA_id];
        pH_const_B = FunctGroupArray[sp].speciesParams[pH_constB_id];
        pH_const_C = FunctGroupArray[sp].speciesParams[pH_constC_id];

        ans = pH_const_A * current_ph * current_ph + pH_const_B * current_ph + pH_const_C;
        break;
	default:
		quit("No such pH model type (%d vs monod: %d, nonlinear: %d or linear: %d)\n", pH_sensitivity_model, monod_pH_id, nonlin_pH_id, lin_pH_id);
		break;
	}

	// Always has to be non-negative, but don't make it zero as have to divide by it
	if((ans < 0.0) || (!ans))
		ans = small_num;

	return ans;
}

/*
 * \brief Get correction factors based on light and noise pollution
 *
 */
double Get_Pollutant_Corrections(MSEBoxModel *bm, int sp, int b, int clayer) {
    double ans = 1.0;
    
    if (!FunctGroupArray[sp].isNoiseEffected && !FunctGroupArray[sp].isLightEffected)
        return ans;
    
    if (FunctGroupArray[sp].isNoiseEffected && (bm->boxes[b].tr[clayer][Noise_Pollution_i] > 0)) {
        ans = FunctGroupArray[sp].speciesParams[noise_coefft_id] / bm->boxes[b].tr[clayer][Noise_Pollution_i];
    }
    
    if (FunctGroupArray[sp].isLightEffected && (bm->boxes[b].tr[clayer][Light_Pollution_i] > 0)) {
        ans *= FunctGroupArray[sp].speciesParams[light_coefft_id] / bm->boxes[b].tr[clayer][Light_Pollution_i];
    }
    
    /*
    if ((bm->newmonth) && (sp == 1)){
        fprintf(bm->logFile, "Time: %e box-layer: %d-%d %s ans: %e, isNoiseEffected: %d noise_coefft: %e, Noise_Pollution: %e, isLightEffected: %d, light_coefft: %e Light_Pollution: %e\n",
                bm->dayt, b, clayer, FunctGroupArray[sp].groupCode, ans, FunctGroupArray[sp].isNoiseEffected, FunctGroupArray[sp].speciesParams[noise_coefft_id], bm->boxes[b].tr[clayer][Noise_Pollution_i], FunctGroupArray[sp].isLightEffected, FunctGroupArray[sp].speciesParams[light_coefft_id], bm->boxes[b].tr[clayer][Light_Pollution_i]);
    }
    */
    
    return ans;
}

/*
 * \brief update parameters given the combined environmental scalars
 *
 */
void Apply_Q10_Corrections(MSEBoxModel *bm) {
	int sp, cohort, sp_q10eff;
	double Tscalar = 1.0;
    double TscalarEff = 1.0;
	int sensitive_sp = 0;
	int pHsensitive_sp = 0;
	int stage = 0;
	double pHscalar = 1.0;
	double growth_scalar, contamScalar = 1.0, contamGrowthScalar = 1.0;

	/* Get pHCorr value - to apply to the nitrification - from Huesemann et al 2002 */
	double pHCorr = 1.0;
    if(bm->flagmodelpH)
        pHCorr = -1.5 + 0.3 * current_PH;

    /* Update parameters */
	for (sp = 0; sp < bm->K_num_tot_sp; sp++) {
		if (FunctGroupArray[sp].speciesParams[flag_id] == TRUE) {

			// Check if growth or non-predation mortality effected
			pHsensitive_sp = (int) (FunctGroupArray[sp].speciesParams[flagpHsensitive_id]);
			if(pHsensitive_sp) {
				pHscalar = FunctGroupArray[sp].pHcorr;
			} else {
				pHscalar = 1.0;
			}

			if(bm->track_contaminants){
				contamScalar = FunctGroupArray[sp].Ccorr;
                contamGrowthScalar = FunctGroupArray[sp].C_growth_corr[0];
			} else {
                contamScalar = 1.0;
                contamGrowthScalar = 1.0;
            }
            
            // Check for sensitivity
            if ( FunctGroupArray[sp].speciesParams[flagtempsensitive_id] || FunctGroupArray[sp].speciesParams[flagSaltSensitive_id] || pHsensitive_sp || (FunctGroupArray[sp].PolluteCorr != 1.0) || (contamScalar != 1.0)) {
                sensitive_sp = 1;
            } else {
                sensitive_sp = 0;
            }
            
			/* Primary producers */
			if (FunctGroupArray[sp].isPrimaryProducer == TRUE){
				FunctGroupArray[sp].speciesParams[KI_id] = FunctGroupArray[sp].speciesParams[KI_T15_id] * FunctGroupArray[sp].Tcorr * FunctGroupArray[sp].Scorr * pHscalar * contamScalar;
				// TODO: May need to allow for different irradiance params for epiphytes in macrophyte model
				// FunctGroupArray[sp].speciesParams[KIepi_id] = FunctGroupArray[sp].speciesParams[KIepi_T15_id] * FunctGroupArray[sp].Tcorr * FunctGroupArray[sp].Scorr * pHscalar;
			}

			/* Invertebrate growth parameters */
			if (FunctGroupArray[sp].isVertebrate == FALSE) {
				for (cohort = 0; cohort < FunctGroupArray[sp].numCohortsXnumGenes; cohort++) {

					FunctGroupArray[sp].scaled_C[cohort] = FunctGroupArray[sp].SP_C[cohort] * FunctGroupArray[sp].Tcorr * FunctGroupArray[sp].Scorr * pHscalar * contamScalar * FunctGroupArray[sp].PolluteCorr;
					FunctGroupArray[sp].scaled_mum[cohort] = FunctGroupArray[sp].mum[cohort] * FunctGroupArray[sp].Tcorr * FunctGroupArray[sp].Scorr * pHscalar * contamGrowthScalar * FunctGroupArray[sp].PolluteCorr;
                    
					growth_scalar = Ecology_Get_Growth_Scalar(bm, sp, cohort);

					FunctGroupArray[sp].scaled_mum[cohort] = FunctGroupArray[sp].scaled_mum[cohort] * growth_scalar;


					if(!(_finite(FunctGroupArray[sp].scaled_C[cohort]))){
						quit("SP_C is not finite for species %s cohort %d\n", FunctGroupArray[sp].groupCode, cohort);
					}
					if(isnan(FunctGroupArray[sp].scaled_C[cohort])){
						quit("SP_C is NaN for species %s cohort %d\n", FunctGroupArray[sp].groupCode, cohort);
					}

				}
			} else {
			/* Vertebrate parameters */
				FunctGroupArray[sp].speciesParams[vla_id] = FunctGroupArray[sp].speciesParams[vla_T15_id] * FunctGroupArray[sp].Tcorr * FunctGroupArray[sp].Scorr * pHscalar * contamScalar * FunctGroupArray[sp].PolluteCorr;

				sp_q10eff = (int) FunctGroupArray[sp].speciesParams[flagq10eff_id];
                if((FunctGroupArray[sp].Tcorr * pHscalar) < 1.0) {
                    Tscalar = FunctGroupArray[sp].Tcorr * pHscalar;
                } else {
                    Tscalar = 1.0 / (FunctGroupArray[sp].Tcorr * pHscalar);
                }
                
                TscalarEff = 1.0 / (FunctGroupArray[sp].TcorrEff); // added Caren & Albi -- it will only be applied for temps warmer than the baseline so when TcorrEff is positive

				if ((sp_q10eff == poorer_when_cool) && (bm->current_corr < 0.0)) {
					FunctGroupArray[sp].speciesParams[E1_id] = FunctGroupArray[sp].speciesParams[E1orig_id] * Tscalar * FunctGroupArray[sp].Scorr * contamScalar;
					FunctGroupArray[sp].speciesParams[E2_id] = FunctGroupArray[sp].speciesParams[E2orig_id] * Tscalar * FunctGroupArray[sp].Scorr * contamScalar;
					FunctGroupArray[sp].speciesParams[E3_id] = FunctGroupArray[sp].speciesParams[E3orig_id] * Tscalar * FunctGroupArray[sp].Scorr * contamScalar;
					FunctGroupArray[sp].speciesParams[E4_id] = FunctGroupArray[sp].speciesParams[E4orig_id] * Tscalar * FunctGroupArray[sp].Scorr * contamScalar;
				} else if ((sp_q10eff == poorer_when_warm) && (bm->current_corr > 0.0)) {
					FunctGroupArray[sp].speciesParams[E1_id] = FunctGroupArray[sp].speciesParams[E1orig_id] * Tscalar * FunctGroupArray[sp].Scorr * contamScalar;
					FunctGroupArray[sp].speciesParams[E2_id] = FunctGroupArray[sp].speciesParams[E2orig_id] * Tscalar * FunctGroupArray[sp].Scorr * contamScalar;
					FunctGroupArray[sp].speciesParams[E3_id] = FunctGroupArray[sp].speciesParams[E3orig_id] * Tscalar * FunctGroupArray[sp].Scorr * contamScalar;
					FunctGroupArray[sp].speciesParams[E4_id] = FunctGroupArray[sp].speciesParams[E4orig_id] * Tscalar * FunctGroupArray[sp].Scorr * contamScalar;
				} else if ((sp_q10eff == versus_baseline) && (bm->current_corr > 0.0)) { // added by Albi
                    FunctGroupArray[sp].speciesParams[E1_id] = FunctGroupArray[sp].speciesParams[E1orig_id] * TscalarEff * FunctGroupArray[sp].Scorr * contamScalar;
                    FunctGroupArray[sp].speciesParams[E2_id] = FunctGroupArray[sp].speciesParams[E2orig_id] * TscalarEff * FunctGroupArray[sp].Scorr * contamScalar;
                    FunctGroupArray[sp].speciesParams[E3_id] = FunctGroupArray[sp].speciesParams[E3orig_id] * TscalarEff * FunctGroupArray[sp].Scorr * contamScalar;
                    FunctGroupArray[sp].speciesParams[E4_id] = FunctGroupArray[sp].speciesParams[E4orig_id] * TscalarEff * FunctGroupArray[sp].Scorr * contamScalar;
                }


				//sensitive_sp = (int) (FunctGroupArray[sp].speciesParams[flagtempsensitive_id]);
				if(sensitive_sp) {
					if(bm->tsGrowthRateChange != NULL) {
						quit("Code can't have temperature rescaling growth and time series rescaling growth - please contact Atlantis development team for code fix\n");
					}
					/* Replicate old bec_dev results - these will be removed asap */
					if((bm->flag_replicated_old == FALSE) || (bm->flag_inheritance)){
						for(cohort = 0; cohort < FunctGroupArray[sp].numCohortsXnumGenes; cohort++){
							FunctGroupArray[sp].scaled_C[cohort] = FunctGroupArray[sp].SP_C[cohort] * (FunctGroupArray[sp].Tcorr * FunctGroupArray[sp].Scorr * pHscalar  * contamScalar * FunctGroupArray[sp].PolluteCorr);
							FunctGroupArray[sp].scaled_mum[cohort] = FunctGroupArray[sp].mum[cohort] * (FunctGroupArray[sp].Tcorr * FunctGroupArray[sp].Scorr * pHscalar  * contamGrowthScalar * FunctGroupArray[sp].PolluteCorr);
                            
                            //if(bm->newmonth)
                                //fprintf(bm->logFile,"Time: %e box %d-%d %s-%d has scaled_mum: %e with mum %e Tcorr: %e Scorr: %e pHscalar: %e contamGrowthScalar: %e\n", bm->dayt, bm->current_box, bm->current_layer, FunctGroupArray[sp].groupCode, cohort, FunctGroupArray[sp].scaled_mum[cohort], FunctGroupArray[sp].mum[cohort], FunctGroupArray[sp].Tcorr, FunctGroupArray[sp].Scorr, pHscalar, contamGrowthScalar);
                            

						}
					}

				} else {
					for(cohort = 0; cohort < FunctGroupArray[sp].numCohortsXnumGenes; cohort++){
						FunctGroupArray[sp].scaled_C[cohort] = FunctGroupArray[sp].SP_C[cohort];
					}
				}
			}

			/* Mortality - no additional contaminant mortality here as added directly in linear mortality call */
			FunctGroupArray[sp].speciesParams[mS_id] = FunctGroupArray[sp].speciesParams[mS_T15_id] * FunctGroupArray[sp].Tcorr * FunctGroupArray[sp].Scorr * (1.0 / pHscalar) * (1.0 / FunctGroupArray[sp].PolluteCorr);
			switch (FunctGroupArray[sp].groupAgeType) {
			case AGE_STRUCTURED_BIOMASS:
				for(stage = 0; stage < FunctGroupArray[sp].numStages; stage++) {
					FunctGroupArray[sp].cohortSpeciesParams[stage][mL_id] = FunctGroupArray[sp].cohortSpeciesParams[stage][mL_T15_id] * FunctGroupArray[sp].Tcorr * FunctGroupArray[sp].Scorr * (1.0 / pHscalar) * (1.0 / FunctGroupArray[sp].PolluteCorr);
					FunctGroupArray[sp].cohortSpeciesParams[stage][mQ_id] = FunctGroupArray[sp].cohortSpeciesParams[stage][mQ_T15_id] * FunctGroupArray[sp].Tcorr * FunctGroupArray[sp].Scorr * (1.0 / pHscalar) * (1.0 / FunctGroupArray[sp].PolluteCorr);
					FunctGroupArray[sp].cohortSpeciesParams[stage][mE_id] = 0; //1e-05;
				}
				break;
			case BIOMASS: /* Intentional follow though */
			case AGE_STRUCTURED: 
				for(stage = 0; stage < FunctGroupArray[sp].numStages; stage++) {
					FunctGroupArray[sp].cohortSpeciesParams[stage][mL_id] = FunctGroupArray[sp].cohortSpeciesParams[stage][mL_T15_id] * FunctGroupArray[sp].Tcorr * FunctGroupArray[sp].Scorr * (1.0 / pHscalar) * (1.0 / FunctGroupArray[sp].PolluteCorr);
					FunctGroupArray[sp].cohortSpeciesParams[stage][mQ_id] = FunctGroupArray[sp].cohortSpeciesParams[stage][mQ_T15_id] * FunctGroupArray[sp].Tcorr * FunctGroupArray[sp].Scorr * (1.0 / pHscalar) * (1.0 / FunctGroupArray[sp].PolluteCorr);
                    
					/* Test values */
					FunctGroupArray[sp].cohortSpeciesParams[stage][mE_id] = 0; //1e-05;
                    
                    /*
                    if((sp == bm->which_check) && (bm->current_box == bm->checkbox)) {
                        fprintf(logfp, "Time: %e box%d-%d mQ %e as mq_T15: %e Tcorr: %e Scorr: %e pHscalar: %e PolluteCorr: %e\n", bm->dayt, bm->current_box, bm->current_layer,  FunctGroupArray[sp].cohortSpeciesParams[stage][mQ_id], FunctGroupArray[sp].cohortSpeciesParams[stage][mQ_T15_id], FunctGroupArray[sp].Tcorr, FunctGroupArray[sp].Scorr, (1.0 / pHscalar), (1.0 / FunctGroupArray[sp].PolluteCorr));
                    }
                     */
				}
				break;
			}


			// Sanity checks
			for(cohort = 0; cohort < FunctGroupArray[sp].numCohortsXnumGenes; cohort++){
				if(FunctGroupArray[sp].SP_C[cohort] < 0.0)
					FunctGroupArray[sp].SP_C[cohort] = 0.0;
				if(FunctGroupArray[sp].mum[cohort]< 0.0)
					FunctGroupArray[sp].mum[cohort] = 0.0;
				if(FunctGroupArray[sp].scaled_C[cohort] < 0.0)
					FunctGroupArray[sp].scaled_C[cohort] = 0.0;
				if(FunctGroupArray[sp].scaled_mum[cohort] < 0.0)
					FunctGroupArray[sp].scaled_mum[cohort] = 0.0;
			}
			if(FunctGroupArray[sp].speciesParams[E1_id] < 0.0)
				FunctGroupArray[sp].speciesParams[E1_id] = 0.0;
			if(FunctGroupArray[sp].speciesParams[E2_id] < 0.0)
				FunctGroupArray[sp].speciesParams[E2_id] = 0.0;
			if(FunctGroupArray[sp].speciesParams[E3_id] < 0.0)
				FunctGroupArray[sp].speciesParams[E3_id] = 0.0;
			if(FunctGroupArray[sp].speciesParams[E4_id] < 0.0)
				FunctGroupArray[sp].speciesParams[E4_id] = 0.0;

			for(stage = 0; stage < FunctGroupArray[sp].numStages; stage++) {
				if(FunctGroupArray[sp].cohortSpeciesParams[stage][mL_id] < 0.0)
					FunctGroupArray[sp].cohortSpeciesParams[stage][mL_id] = 0.0;
				if(FunctGroupArray[sp].cohortSpeciesParams[stage][mQ_id] < 0.0)
					FunctGroupArray[sp].cohortSpeciesParams[stage][mQ_id] = 0.0;
				if(FunctGroupArray[sp].cohortSpeciesParams[stage][mE_id] < 0.0)
					FunctGroupArray[sp].cohortSpeciesParams[stage][mE_id] = 0.0;
			}

		}
	}
	
	/* Physical parameters */
	r_DL = r_DL_T15 * bm->Tcorr;
	r_DC = r_DC_T15 * bm->Tcorr;
	r_DR = r_DR_T15 * bm->Tcorr;
	r_DON = r_DON_T15 * bm->Tcorr;
	r_DSi = r_DSi_T15 * bm->Tcorr;

	K_nit = K_nit_T15 * bm->Tcorr * pHCorr;

	R_0 = R_0_T15;
	R_D = R_D_T15;

}

