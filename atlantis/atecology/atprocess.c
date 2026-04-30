/**
 \file
 \brief C file for process subroutines for south east ecological model
 ingroup atEcology

 File:	atprocess.c
 Author:	Beth Fulton
 Created:	21/9/2004
 Purpose:	C file for process subroutines for south east ecological model
 Revisions: 8/8/2004 Created mseecomproc.c from the seecomproc.c file

 11/8/2004 Time related routines move to msetime.c

 21/9/2004 Created atprocess.c from mseecomproc.c. And renamed
 FSL, FSO, FSF, FST and FSG to generic names so apply to
 other models/genericf case.
 FSL -> FDC  (Ling -> Demersal_DC_Fish)
 FSO -> FDO  (Orange_Roughy -> Demersal_O_Fish)
 FSF -> FDB  (Flathead -> Demersal_B_Fish)
 FST -> FVO  (Bluefin_Tuna -> Pisciv_O_Fish)
 FSH -> SHB  (Gummy_Shark -> Shark_B)

 22/9/2004 Replaced numbers in switch statements with id
 guild numbers. Added age structure to cephalopods; also
 and invertebrate aging and reproduction in Invertebrate_Reproduction()

 15/10/2004 Added BML and BFF.

 19/10/2004 Added Prawns

 26/10/2004 Added FBP, FVB, SP, FDE, FDF, SHR, SSK, REP, WDG

 8/4/2005 Added Invert_Activities() to handle mortality, growth,
 assimilation and waste equations in a general way

 10/4/2005 Added Primary_Production() to handle all primary production

 16/8/2005 Put in default case for all switch statements

 1/9/2005 Added multiple stock structure (horizontal and vertical)

 28/10/2005 Modified Invert_Activities() so mortality parameters read
 straight from parameter array not feed through routine call
 (so can allow for separate juvenile and adult mortality in
 vertebrates). Similarly modified Added Primary_Production()

 18/07/2008 Bec Gorton
 Changing the code within and called by Box_Bio_Process to use the temporary data
 stores in the EcologyModule instead of Pool and NetFLUX.

 13/08/2008 Bec Gorton

 Changed the invertebrate code to store calculated values in the
 invertProps structure instead of returning values.
 Changed Eat, PrimaryProduction and InvertActivites functions
 to store returned values in the invertProps structure.

 20-01-2009 Bec Gorton
 Changed the DIET_check array to store habitat level information. This
 array is now in the atEcologyStruct and its contents are output to
 a seperate file called DietCheck.txt if the flagdietcheck flag is TRUE.

 Added the age_structure_prey species param. If this is true then the prey availability
 value in the bm->pSPageeat array is used instead of the more generic pSPEat value.

 02-02-2009 Beth Fulton
 Changed the variable called small to small_num.

 03-02-2009 Bec Gorton
 Added additional debug statements to Eat and PrimaryProduction to
 allow easy testing of the species parameters input code.


 10-03-2009 Bec Gorton
 Re-wrote the prey_avail function to make it more generic. This also changed the diagnostic output.

 18-03-2009 Bec Gorton
 Changed the Eat function to scale the epibenthic prey and predators
 based on the habitat_type value instead of the invert_types. This makes this
 code more generic. This also highlighted a potential bug in the original code.
 The old code was not doing any scaling if the predator was CEP or PWN.

 23-03-2009 Bec Gorton
 Changed Eat to no longer expect KDEP_sp as this is never used.
 Also got rid of the KDEP_sp and spPREYInfo parameters in the Invert_activites function
 these were not used.

 02-04-2009 Bec Gorton
 Changed Nutrient_lim to expect the box model so that we can get rid of the gen_nut_case parameter.
 This was just set to bm->flagnut anyway.

 06-04-2009 Beth Fulton

 Changed irr_adapt_on case so actually get values < 1 as output
 (appears bug inherited from original ERSEM code on this one)

 10-06-2009 Bec Gorton
 Fixed a bug in the invertegrate_reproduction code.
 The j_sp and a_sp value were not being calculated when they should be.

 10-06-2009 Bec Gorton
 Added the SEAGRASS invertebrate type.

 02-11-2009 Bec Gorton
 Removed references to the bm->K_num_invcohorts value.

 03-11-2009 Bec Gorton
 Removed references to bm->SPtoCATid..

 05-11-2009 Bec Gorton
 Trunk merge 1064 - Changed the spawned_biomass calculation to use the species specific values
 calculated in Parameter_Q10.

 29-01-2010 Bec Gorton
 Updated the Calculate_Catch code to reflect the changes made to the harvest library.
 Also updated some other debugging output.

 28-04-2010 Bec Gorton
 Changed the group process functions to use the new Ecology_Get_Species_LMort function
 to get the linear mortality of a group - this function will calculate apply any scaling that should occur.

 10-05-2010 Bec Gorton
 Got rid of the Ecology_Get_Species_LMort function. These calculation are now done in ParameterQ10.

 26-03-2013 Bec Gorton
 Updated the invertebrate reproduction code to work for age structure epibenthic groups.

 17-04-2013 Bec Gorton
 ATL-346 - Added support of tracking P:N:C ratios in groups.

 21-04-2013 Beth Fulton
 Added overwintering and encystment
 **/

/*  Open library routines */
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "string.h"
#include "atecology.h"
#include <atHarvestLib.h>

/* Invertebrate and general routines ***********************************************/

/**
 *	\brief Nutrient Limitation
 *
 *
 *	Questions:
 *	How does this relate to upwelling or vertical flux? Is this the eddy strength?
 *
 *
 */
double Nutrient_Lim(MSEBoxModel *bm, int nut_case, int micro_case, double amt_din, double amt_si,
		double amt_fe, double amt_p, double PRatio, double P0, double P1, double KN_sp, double KS_sp, double KF_sp, double KP_sp) {
	double hN =0, xx;
	double PValue = 0.0;

	/* Do the Phosphorus calculation once */
	if(bm->track_atomic_ratio == TRUE){
		if(PRatio < P0)
			PValue = 0;
		else if(PRatio > P1)
			PValue = 0;
		else
			PValue = ((P1 - PRatio)/(P1 - P0)) * (amt_p/(KP_sp + amt_p));

		if(PRatio == 0){
			printf("nut_case = %d, micro_case = %d, amt_din = %.20e, amt_si = %.20e, amt_fe = %.20e, KN_sp = %.20e, KS_sp = %.20e, KF_sp = %.20e,KP_sp= %e, PValue= %e\n",
						nut_case, micro_case, amt_din, amt_si, amt_fe, KN_sp, KS_sp, KF_sp, KP_sp, PValue);

			printf("P1 = %e, P0 = %e, PRatio = %e\n", P1, P0, PRatio);
			abort();
		}
	}

	if ((bm->debug == debug_biology_process) && (bm->dayt >= bm->checkstart) && (bm->dayt < bm->checkstop)) {
		fprintf(bm->logFile, "nut_case = %d, micro_case = %d, amt_din = %.20e, amt_si = %.20e, amt_fe = %.20e, KN_sp = %.20e, KS_sp = %.20e, KF_sp = %.20e,KP_sp= %e, PValue= %e\n",
			nut_case, micro_case, amt_din, amt_si, amt_fe, KN_sp, KS_sp, KF_sp, KP_sp, PValue);
	}
	if (!micro_case) {
		/* Si and N limitation only */
		switch (nut_case) {
		case leibig_id:
			hN = min(amt_din/(KN_sp + amt_din), amt_si/(KS_sp + amt_si));
			if(bm->track_atomic_ratio == TRUE){
				hN = min(PValue, hN);
			}
			break;
		case mult_nut_id:
			xx = (amt_din / (KN_sp + amt_din)) * (amt_si / (KS_sp + amt_si));
			if(bm->track_atomic_ratio == TRUE){
				xx = xx * PValue;
			}
			hN = pow(xx, 0.5);
			break;
		case WQI_id:
			if(bm->track_atomic_ratio == TRUE){
				hN = 2.0 / ((KN_sp + amt_din) / amt_din + (KS_sp + amt_si) / amt_si + 1.0/PValue);
			}else{
				hN = 2.0 / ((KN_sp + amt_din) / amt_din + (KS_sp + amt_si) / amt_si);
			}
			break;
		case monod_nut_id:
			hN = amt_din / (KN_sp + amt_din);
			break;
		default:
			quit("No such nut_case defined (%d) - value must be between 0 and 3 currently\n", nut_case);
			break;
		}
	} else {
		/* Micro-nutrient limitation */
		switch (nut_case) {
		case leibig_id:
			xx = min(amt_din/(KN_sp + amt_din), amt_si/(KS_sp + amt_si));
			hN = min(amt_fe/(KF_sp + amt_fe), xx);
			if(bm->track_atomic_ratio == TRUE){
				hN = min(PValue, hN);
			}
			break;
		case mult_nut_id:
			xx = (amt_din / (KN_sp + amt_din)) * (amt_si / (KS_sp + amt_si)) * (amt_fe / (KF_sp + amt_fe));
			if(bm->track_atomic_ratio == TRUE){
				xx = xx * PValue;
			}
			hN = pow(xx, 0.5);
			break;
		case WQI_id:
			if(bm->track_atomic_ratio == TRUE){
				hN = 3.0 / ((KN_sp + amt_din) / amt_din + (KS_sp + amt_si) / amt_si + (KF_sp + amt_fe) / amt_fe + 1.0/PValue);
			}else{
				hN = 3.0 / ((KN_sp + amt_din) / amt_din + (KS_sp + amt_si) / amt_si + (KF_sp + amt_fe) / amt_fe);
			}
			break;
		case monod_nut_id: /* Uses nut_case setting to determine which nutrient limtation to use
		 when relying on N and micro-nutrient */
			switch (bm->flagnut) {
			case leibig_id:
				hN = min(amt_din/(KN_sp + amt_din), amt_fe/(KF_sp + amt_fe));
				if(bm->track_atomic_ratio == TRUE){
					hN = min(hN, PValue);
				}
				break;
			case mult_nut_id:
				xx = (amt_din / (KN_sp + amt_din)) * (amt_fe / (KF_sp + amt_fe));
				if(bm->track_atomic_ratio == TRUE){
					xx = xx * PValue;
				}
				hN = pow(xx, 0.5);
				break;
			case WQI_id:
				if(bm->track_atomic_ratio == TRUE){
					hN = 2.0 / ((KN_sp + amt_din) / amt_din + (KF_sp + amt_fe) / amt_fe + PValue);
				}else{
					hN = 2.0 / ((KN_sp + amt_din) / amt_din + (KF_sp + amt_fe) / amt_fe);
				}
				break;
			default:
				quit("No such bm->flagnut defined (%d) - value must be between 0 and 2 currently\n", bm->flagnut);
				break;
			}
			break;
		default:
			quit("No such nut_case defined (%d) - value must be between 0 and 3 currently\n", nut_case);
			break;
		}
	}
	return (hN);
}

/**
 *	\brief Light Limitation
 *
 */

double Light_Lim(MSEBoxModel *bm, int light_case, double amt_irr, double KI_sp, double opt_irr, double prop_daylight, double PBmax_D) {
	double hI = 0;
	double step1 = 0.0;
	double step2 = 0.0;


	switch (light_case) {
	case irr_adapt_off:
		hI = min(amt_irr/KI_sp, 1.0);
		break;
	case irr_adapt_on:
		hI = min(amt_irr/(KI_sp+opt_irr), 1.0);
		break;
	case irr_ice:
		/* From Griffith & Spitz 2009

		aB_D = 0.035		Photosynthetic efficiency Lavoie Resolute Passage 2005 (or 0.05 Smith and Herman 1991 (In situ))
							mapped to KI_sp here
		PBmax_D = 0.285		Lavoie Resolute Passage 2005 (or 0.44 Smith and Herman 1991 (In situ))
		Beta_D = 0.002		Photoinhibition  microg of C per mg of Chlorophyll a per hour per (Wm^-2)
							mapped to opt_irr here

		*/
		step1 = -1.0 * KI_sp / bm->X_CN; 	// C to N
		step2 = PBmax_D / bm->X_CN;

		// Light limitation with photoadaptation
		hI = (1.0 - exp(step1 * amt_irr / step2)) * exp(-1.0 * opt_irr * amt_irr / step2);
		break;
	default:
		quit("No such light_case defined (%d) - value must be between 0 and 1 currently\n", light_case);
		break;
	}


	/* Assume worrying about hours of daylight in the timestep,
	 if not prop_light_time setp = 1 when figuring out TimeODay()
	 */
	hI *= prop_daylight;

	return (hI);
}

/**
 * Calculate the available prey availability bringing in gape limitation
 *
 */
double Get_Gape_Lim_Prey(MSEBoxModel *bm, FILE *llogfp, int predatorID, int cohort, int chrtstage, int preyID, int prey_chrt, int habitat, double ***spPREYinfo)
{
	double prey_avail = 0.0;
	double SN;
    int preystage = FunctGroupArray[preyID].cohort_stage[prey_chrt];

	if(FunctGroupArray[predatorID].isVertebrate == TRUE){
		SN = VERTinfo[predatorID][cohort][SN_id];
	} else {
		SN = FunctGroupArray[predatorID].sn[cohort];
	}

	if(FunctGroupArray[preyID].isVertebrate == TRUE) {
		if(SN > 0.0)
			prey_avail = Avail_Fish(bm, predatorID, cohort, chrtstage, preyID, prey_chrt, SN, VERTinfo, llogfp);
	} else {
		if(bm->flag_macro_model && (FunctGroupArray[preyID].groupType == SEAGRASS)){
			/* If the prey is seagrass and the macro model is on get the prey availability term for this cohort */
			/* We could do this in many more clever ways but leave it simple for now
			 * Assumes:
			 * 		main_biomass_id = 0,     Leaves
			 *      below_ground_biomass_id, Roots
			 *	    epiphyte_biomass_id,     Epiphytes
			 */
			if(bm->pSPVERTeat[predatorID][preyID][chrtstage][preystage] > 0){
				prey_avail = FunctGroupArray[predatorID].pSP_SG_eat[habitat] * spPREYinfo[preyID][prey_chrt][habitat];
			}
		} else {
			if(!bm->flag_olddiet)
				prey_avail = bm->pSPVERTeat[predatorID][preyID][chrtstage][preystage] * spPREYinfo[preyID][prey_chrt][habitat];
			else
				prey_avail = spPreyAvail[predatorID][cohort][preyID][habitat] * spPREYinfo[preyID][prey_chrt][habitat];
		}
	}

	/**
	if ((predatorID == bm->which_check) && (preyID == 16)) {
		fprintf(llogfp,"Time: %e box%d-%d %s-%d eating %s-%d in habitat %d with preyinfo: %e, avail: %e (pSPVERT: %e, spPreyAvail: %e)\n",
				bm->dayt, bm->current_box, bm->current_layer, FunctGroupArray[predatorID].groupCode, cohort,
				FunctGroupArray[preyID].groupCode, prey_chrt, habitat, spPREYinfo[preyID][prey_chrt][habitat],
				prey_avail, bm->pSPVERTeat[predatorID][preyID][chrtstage][preystage],
				spPreyAvail[predatorID][cohort][preyID][habitat]);
	}
	**/

	return prey_avail;
}

/**
 * Calculate the available prey availability bringing in gape limitation
 *
 */
double Get_Catch_Prey(MSEBoxModel *bm, FILE *llogfp, int predatorID, int cohort, int chrtstage, int preyID, int prey_chrt, int habitat)
{
	double prey_avail = 0.0;
	double SN, step1;
	int fleet;

	if(habitat == SED)
		return 0.0; // As no fisheries inside the sediments

	if(FunctGroupArray[predatorID].isVertebrate == TRUE){
		SN = VERTinfo[predatorID][cohort][SN_id];
	} else {
		SN = FunctGroupArray[predatorID].sn[cohort];
	}

	if(FunctGroupArray[preyID].isVertebrate == TRUE) {
		if(SN > 0.0)
			prey_avail = Avail_Catch(bm, predatorID, cohort, chrtstage, preyID, prey_chrt, VERTinfo[predatorID][cohort][SN_id], VERTinfo, llogfp);
	} else {
		step1 = 0;
		for (fleet = 0; fleet < bm->K_num_fisheries; fleet++) {
			step1 += bm->FCcaughttemp[preyID][fleet][prey_chrt] * bm->pFLEET[predatorID][fleet] / (bm->cell_vol + small_num);
		}

		if(!bm->flag_olddiet)
			step1 *= bm->pSPFCeat[predatorID][preyID][cohort][prey_chrt];

		prey_avail += step1;
	}

	return prey_avail;
}

/**
 * Calculate the actual biomass that can be eaten of each group by this species/cohort.
 * Also calculates how much of the prey available is plant, living, ref detritus and lab detritus.
 *
 *
 */
static void Calculate_PreyAvail(MSEBoxModel *bm, FILE *llogfp, int predatorGuildID, int cohort, int chrtstage, int preyGuildID, int prey_chrt, double ***spPREYinfo, long double *plant_prey, long double *living_prey, long double *living_prey_sq, long double *refdet, long double *labdet) {
	int habitat, pHsensitive_sp, nut_val_sensitive_sp, max_hab;
	long double prey_eat, prey_active, prey_avail, catch_avail, pHscalar, catch_eat;
	int catcheater = (int) (FunctGroupArray[predatorGuildID].speciesParams[catcheater_id]);
	//int bcohort = floor(cohort / FunctGroupArray[predatorGuildID].numGeneTypes);
	// bcohort was used in place of cohort in spPreyAvail before moved to full gene expression (to allow for evolving diets and ontogeny)

	CATCHEATINGinfo[preyGuildID][prey_chrt] = 0; // incase never called again

	max_hab = bm->num_active_habitats - 1;
	if(FunctGroupArray[preyGuildID].isVertebrate == TRUE)
		max_hab = WC;

	/* for invertebrates and detritus need to include availability term */
	prey_active = FunctGroupArray[preyGuildID].speciesParams[flag_id];

	/* Ignore these - this is handled separately*/
	if (FunctGroupArray[preyGuildID].groupType == LAB_DET || FunctGroupArray[preyGuildID].groupType == REF_DET
			|| FunctGroupArray[preyGuildID].groupType == CARRION)
		prey_active = 1;

	if(prey_active == FALSE)
		return;

	/* pH effects on behaviour and exposure to predation - assume making
	 * it more available but that pHCorr < 1.0 as growth etc is falling
	 */
	pHsensitive_sp = (int) (FunctGroupArray[preyGuildID].speciesParams[flagpredavaileffect_id]);
	if(pHsensitive_sp){
		pHscalar = (1.0 / FunctGroupArray[preyGuildID].pHcorr);
	} else {
		pHscalar = 1.0;
	}

	/* Piggy back on the habitat loops for the seagrass cohorts */
	for (habitat = WC; habitat <= max_hab; habitat++) {


		/* Get the prey availability for the pre calculated array */
		prey_avail = Get_Gape_Lim_Prey(bm, llogfp, predatorGuildID, cohort, chrtstage, preyGuildID, prey_chrt, habitat, spPREYinfo);


		if (habitat == WC && catcheater && bm->flag_fisheries_on){
			catch_avail = Get_Catch_Prey(bm, llogfp, predatorGuildID, cohort, chrtstage, preyGuildID, prey_chrt, habitat);
			catch_eat = catch_avail * pHscalar;
			CATCHEATINGinfo[preyGuildID][prey_chrt] = (double)catch_eat;
		}

		/* determine biomass available */
		prey_eat =  prey_avail * pHscalar;
		EATINGinfo[preyGuildID][prey_chrt][habitat] = (double)prey_eat;

		/**
		if ((predatorGuildID == bm->which_check) || (preyGuildID == bm->which_check)){
			fprintf(llogfp,"Time: %e hab: %d, pred %s-%d on %s-%d has EATINGinfo: %.20e, CATCHEATINGinfo: %.20e, prey_avail: %.20e, pHscalar: %.20e\n",
				bm->dayt, habitat, FunctGroupArray[predatorGuildID].groupCode, cohort,
				FunctGroupArray[preyGuildID].groupCode, prey_chrt, EATINGinfo[preyGuildID][prey_chrt][habitat],
				CATCHEATINGinfo[preyGuildID][prey_chrt], prey_avail, pHscalar);
		}
		**/

		/* Check on the type of prey to work out if its plant/living etc */
		switch (FunctGroupArray[preyGuildID].groupType) {
		case LG_PHY:
		case SM_PHY:
		case DINOFLAG:
		case PHYTOBEN:
		case SEAGRASS:
		case MICROPHTYBENTHOS:
		case TURF:
			nut_val_sensitive_sp = (int) (FunctGroupArray[preyGuildID].speciesParams[flagnutvaleffect_id]);
			if ( nut_val_sensitive_sp ) {
				pHscalar = FunctGroupArray[preyGuildID].pHcorr;
			} else {
				pHscalar = 1.0;
			}

			*plant_prey += (EATINGinfo[preyGuildID][prey_chrt][habitat] * pHscalar);
			if (habitat == WC && catcheater && bm->flag_fisheries_on){
				*plant_prey += (CATCHEATINGinfo[preyGuildID][prey_chrt] * pHscalar);
			}
			break;
		case LAB_DET:
			*labdet += EATINGinfo[preyGuildID][prey_chrt][habitat];
			break;
		case CARRION:
			*living_prey += EATINGinfo[preyGuildID][prey_chrt][habitat];
			*living_prey_sq += EATINGinfo[preyGuildID][prey_chrt][habitat] * EATINGinfo[preyGuildID][prey_chrt][habitat];
			break;
		case REF_DET:
			*refdet += EATINGinfo[preyGuildID][prey_chrt][habitat];
			break;
		default: /* All the rest */
			*living_prey += EATINGinfo[preyGuildID][prey_chrt][habitat];
			*living_prey_sq += (EATINGinfo[preyGuildID][prey_chrt][habitat] * EATINGinfo[preyGuildID][prey_chrt][habitat]);
			if (habitat == WC && catcheater && bm->flag_fisheries_on){
				*living_prey += CATCHEATINGinfo[preyGuildID][prey_chrt];
				*living_prey_sq += (CATCHEATINGinfo[preyGuildID][prey_chrt] * CATCHEATINGinfo[preyGuildID][prey_chrt]);
			}
			break;
		}

	}
    
    return;
}

/**
 * \brief Calculate amount of competition between predators
 *
 * Calculate the amount of predator biomass and competiton for ratio dependent fucntional feeding responses
 *
 */
void Get_Predator_Competition(MSEBoxModel *bm, int sp_id, int cohort, long double *tot_pred, long double *tot_pred_comp_sp, FILE *llogfp) {
    double ans_tot_pred = 0.0;
    double ans_tot_pred_comp = 0.0;
    int predID, pid;
    
    //TODO: FIX the dim_corrections
    double epi_dim_correction = 1.0, sm_dim_correction = 1.0, wc_dim_correction = 1.0;
    
    if (!bm->need_predcomp_params ) { // Just need total predation but not competition
        for (predID = 0; predID < bm->K_num_tot_sp; predID++) {
            for (cohort = 0; cohort < FunctGroupArray[predID].numCohortsXnumGenes; cohort++) {
                pid = FunctGroupArray[predID].totNTracers[cohort];
            
                if ( !bm->current_layer ) { // Need sediment based predators
                    if ( FunctGroupArray[predID].isEpiFauna )
                        ans_tot_pred += bm->boxes[bm->current_box].epi[pid] * FunctGroupArray[sp_id].habitatCoeffs[EPIFAUNA] * epi_dim_correction;
                    ans_tot_pred += bm->boxes[bm->current_box].sm.tr[bm->current_layer][pid] * FunctGroupArray[sp_id].habitatCoeffs[SED] * sm_dim_correction;
                    ans_tot_pred += bm->boxes[bm->current_box].tr[bm->current_layer][pid] * FunctGroupArray[sp_id].habitatCoeffs[WC] * wc_dim_correction;
                } else {
                    ans_tot_pred += bm->boxes[bm->current_box].sm.tr[bm->current_layer][pid] * FunctGroupArray[sp_id].habitatCoeffs[SED] * sm_dim_correction;
                    ans_tot_pred += bm->boxes[bm->current_box].tr[bm->current_layer][pid] * FunctGroupArray[sp_id].habitatCoeffs[WC] * wc_dim_correction;
                }
            }
        }

    } else { // Need to include predator competition - where it is sum(competition param * pred biomass)
        for (predID = 0; predID < bm->K_num_tot_sp; predID++) {
            if (current_layer_sed > -1) { // Need sediment based predators
                    
            } else if (current_layer_sed > 0) {
                    
            } else {
                    
            }
        }
    }
    
    *tot_pred = ans_tot_pred_comp;
    return;
}

/**
 * \brief Aquaculture feed
 *
 * Calculate the amount of aquaculture feed available - is based on beinf fed a proportion of own body mass per day
 *
 */
void Get_Extra_Feed(MSEBoxModel *bm, FILE *llogfp, int sp_id, int cohort, double sp_Biomass, int flagcase, double KL_sp, long double *living_prey,
                    long double *living_prey_sq, double *denom_step) {
    int do_adult = 0;
    int do_juv = 0;
    int do_feeding = 0;
    int stage;
    double scalar = 1.0;
    
    if (FunctGroupArray[sp_id].isSupplemented > 0) {
        if (bm->current_layer < (bm->boxes[bm->current_box].nz - 1)) {
            // Can't do it unless at the surface layer
            scalar = 0.0;
            do_feeding = 0;
        } else {
            // So only occurs if supplemented and in the surface layer
            do_adult = 1;  // By default if supplement (set to 1), then supplement both age stages
            do_juv = 1;
            
            if (FunctGroupArray[sp_id].isSupplemented > 1) {  // If set to 2 then juveniles only
                do_adult = 0;
            }
            
            if (FunctGroupArray[sp_id].isSupplemented > 2) {  // If set to 3 then adults only
                do_juv = 0;
                do_adult = 1;
            }
            
            stage = FunctGroupArray[sp_id].cohort_stage[cohort];
            
            if ((stage < adult_id) && (do_juv)) {
                do_feeding = 1;
            }
            
            if ((stage > juv_id) && (do_adult)) {
                do_feeding = 1;
                scalar = SUPPdistrib[sp_id][bm->current_box];
            }
        }
        
    }
    
    if (FunctGroupArray[sp_id].isCultured ) { // If cultured feed both age stages
        do_feeding = 1;
        scalar = 1.0;
    }
    
    
    if ( do_feeding ) {
        EATINGinfo[AquacultFeedIndex][cohort][WC] = FunctGroupArray[sp_id].speciesParams[extra_feed_id] * sp_Biomass * scalar;
        *living_prey += EATINGinfo[AquacultFeedIndex][cohort][WC];
        *living_prey_sq += EATINGinfo[AquacultFeedIndex][cohort][WC] * EATINGinfo[AquacultFeedIndex][cohort][WC];
        
        if(flagcase == eat_minmax){
            FEEDinfo[AquacultFeedIndex][cohort][WC] += EATINGinfo[AquacultFeedIndex][cohort][WC] * Util_Mich_Ment(EATINGinfo[AquacultFeedIndex][cohort][WC], KL_sp);
            *denom_step += FEEDinfo[AquacultFeedIndex][cohort][WC];
        }
    }
    
    return;
    
}

/**
 * \brief Consumption
 *
 * Calculate the amount of a prey eaten. The values calculated have the units
 * mg N s-1. This change per second is later calculated by the size of the timestep
 * to get the total change in mg N.
 *
 */
void Eat(MSEBoxModel *bm, FILE *llogfp, int flagcase, int sp_id, int cohort, double sp, double C_sp, double mum_sp, double KL_sp, double KU_sp, double vl_sp, double ht_sp, double E1_sp, double E2_sp, double EDL_sp, double EDR_sp, int sp_feed_while_spawn, int sp_spawn_now, double chrt_mat, double ***spPREYinfo, double ***spGRAZEinfo, double **spCATCHGRAZEinfo, double sp_Biomass) {  // sp_Biomass is only used for aquaculture, sp is used for everything else
	int preyID, max_hab, prey_active;
	int kij = 0, fleet = 0;
	int habitat, thisID;
	long double living_prey, tot_prey, tot_prey_sq, tprey = 0, living_prey_sq,  labdet, refdet, CLEAR, step1, plant_prey, graze_live, pHscalar;
	long double rel_scalar, catch_addition, ratio, eat_amt;
	int ncohort, chrtstage;
	int is_overwintering;
	int zero_out = 0;
	int catcheater = (int) (FunctGroupArray[sp_id].speciesParams[catcheater_id]);
	double denom_step = 0.0;
    double sp_hvm = FunctGroupArray[sp_id].speciesParams[hvm_id]; // The coefficient of mutual interference for the total set of predators that interact on the prey targeted by species sp
    long double tot_pred_comp_sp = 0, tot_pred = 0;
	long double scalar = 1.0;
	long double scaled_clear;
    
    /* Get total predation biomass if needed */
    switch (flagcase) {
        case eat_parslow_holling2: /* Do not need predator biomass and competition */
        case eat_parslow_holling1:
        case eat_parslow_holling3:
        case eat_ecosim:
        case eat_minmax:
        case eat_holling3size:
        case eat_std_holling1:
        case eat_std_holling2:
        case eat_std_holling3:
        case eat_std_holling4:
            tot_pred_comp_sp = 1.0;
            tot_pred = sp;
            break;
        case eat_ratio_dependent: /* Ones that need predator biomass and competition */
        case eat_hassel_varley:
        case eat_crowley_martin:
            quit("This pred_case (%d) - needs more code to finish it\n", flagcase);
            Get_Predator_Competition(bm, sp_id, cohort, &tot_pred, &tot_pred_comp_sp, llogfp);
            break;
        default:
            quit("No such pred_case defined (%d) - value must be between 0 and 5 currently\n", flagcase);
            break;
            
    }


    
//	if ((bm->debug == debug_biology_process) && (bm->dayt >= bm->checkstart) && (bm->dayt < bm->checkstop)) {
//		fprintf(llogfp,
//				"EAT parameters: flagcase = %d, box %d:%d, sp_id = %s, cohort = %d, sp = %.20e, C_sp = %.20e, mum_sp = %.20e, KL_sp = %.20e, KU_sp = %.20e, vl_sp = %.20e, ht_sp = %.20e, E1_sp = %.20e, E2_sp = %.20e, EDL_sp = %.20e, EDR_sp = %.20e, sp_feed_while_spawn = %d,  sp_spawn_now = %d, chrt_mat = %.20e\n",
//				flagcase, bm->current_box, bm->current_layer,  FunctGroupArray[sp_id].groupCode, cohort, sp, C_sp, mum_sp, KL_sp, KU_sp, vl_sp, ht_sp, E1_sp, E2_sp, EDL_sp, EDR_sp, sp_feed_while_spawn, sp_spawn_now, chrt_mat);
//	}
	/* Initialise grazing terms and determine available biomass of each prey item */
	living_prey = 0.0;
	living_prey_sq = 0.0;
	plant_prey = 0.0;
	labdet = 0.0;
	refdet = 0.0;
	graze_live = 0.0;

	/* Get the cohort value = either adult or juv depending on the maturity age of the group */
	chrtstage = FunctGroupArray[sp_id].cohort_stage[cohort];

	if(FunctGroupArray[sp_id].groupAgeType == AGE_STRUCTURED_BIOMASS || FunctGroupArray[sp_id].groupAgeType == AGE_STRUCTURED){
		ncohort = cohort;
		// Sanity check
		if(ncohort >= FunctGroupArray[sp_id].numStages)
			ncohort = FunctGroupArray[sp_id].numStages - 1;
	} else {
		ncohort = 0;
	}

	// Check if this group feeds at all - won't eat if non-consumer, no suitable habitat for growth or if overwintering and not eating
	zero_out = 0;
	is_overwintering = (int)FunctGroupArray[sp_id].speciesParams[overwintering_id];
	if((FunctGroupArray[sp_id].cohortSpeciesParams[ncohort][flageat_id] <= 0)
			|| (!sp_feed_while_spawn && sp_spawn_now && (chrt_mat == 1.0))
			|| is_overwintering || (mum_sp == 0.0)){
		zero_out = 1;
		if(zero_out){

			for (preyID = 0; preyID < bm->K_num_tot_sp; preyID++) {
				for (kij = 0; kij < FunctGroupArray[preyID].numCohortsXnumGenes; kij++) {
					for(habitat = 0; habitat < bm->num_active_habitats; habitat++){
						spGRAZEinfo[preyID][kij][habitat] = 0.0;
					}
				}
			}

			FunctGroupArray[sp_id].CLEAR[cohort] = 0.0;
			FunctGroupArray[sp_id].GrazeLive[cohort] = 0.0;
			return;
		}
	}

	if (!sp_feed_while_spawn && sp_spawn_now) {
		if (chrt_mat < 1.0) {
			/* Adjust biomass down according to proportion of the population currently spawning */
			sp *= (1.0 - chrt_mat);
		}
	}

	for (preyID = 0; preyID < bm->K_num_tot_sp; preyID++) {
		for (kij = 0; kij < FunctGroupArray[preyID].numCohortsXnumGenes; kij++) {
			for(habitat = 0; habitat < bm->num_active_habitats; habitat++){
				spGRAZEinfo[preyID][kij][habitat] = 0.0;
			}
			spCATCHGRAZEinfo[preyID][kij] = 0.0;
			
            /* Calculate how much of each possible prey biomass is actually available for consumption by this group/cohort
			 * The results of these calculations are stored in the EATINGinfo array.
			 */
            
            if((FunctGroupArray[preyID].isOncePerDt == FALSE) || (it_count == 1)) {

            	/* for invertebrates and detritus need to include availability term */
				prey_active = (int)FunctGroupArray[preyID].speciesParams[flag_id];

				/* Ignore these - this is handled separately*/
				if (FunctGroupArray[preyID].isDetritus == TRUE)
					prey_active = 1;

				if(prey_active){
					Calculate_PreyAvail(bm, llogfp, sp_id, cohort, chrtstage, preyID, kij, spPREYinfo, &plant_prey, &living_prey, &living_prey_sq, &refdet, &labdet);
				}
			} else {
				EATINGinfo[preyID][kij][WC] = 0.0;
			}

			if (catcheater && bm->flag_fisheries_on){
				for (fleet = 0; fleet < bm->K_num_fisheries; fleet++) {
					bm->groupTotCatch[preyID][kij] = bm->FCcaughttemp[preyID][fleet][kij] * bm->pFLEET[sp_id][fleet];
				}
				if(!bm->flag_olddiet)
					bm->groupTotCatch[preyID][kij] *= bm->pSPFCeat[sp_id][preyID][cohort][kij];
			}

			if(flagcase == eat_minmax){
				max_hab = bm->num_active_habitats - 1;
				if(FunctGroupArray[preyID].isVertebrate == TRUE)
					max_hab = WC;
				for (habitat = WC; habitat <= max_hab; habitat++) {
					if(habitat == WC){
						catch_addition = CATCHEATINGinfo[preyID][kij];
					} else {
						catch_addition = 0.0;
					}
					eat_amt = (long double) EATINGinfo[preyID][kij][habitat] + catch_addition;
					FEEDinfo[preyID][kij][habitat] += ((double)eat_amt) * Util_Mich_Ment((double)eat_amt, KL_sp);
					denom_step += FEEDinfo[preyID][kij][habitat];
				}
			}
		}
	}

	if(FunctGroupArray[sp_id].isCultured || FunctGroupArray[sp_id].isSupplemented) {
		Get_Extra_Feed(bm, llogfp, sp_id, cohort, sp_Biomass, flagcase, KL_sp, &living_prey, &living_prey_sq, &denom_step);
	}

	tot_prey = living_prey + plant_prey + labdet + refdet + small_num;
    tot_prey_sq = living_prey_sq + plant_prey * plant_prey + labdet * labdet + refdet * refdet + small_num;

	CLEAR = 0.0;
	/* Now calculate the clearance rate - not dependant on prey so does not need to be in the prey loop */
	switch (flagcase) {
	case eat_parslow_holling2:
		/* Calculate the clearance rate */
		step1 = 1.0 + (living_prey * E1_sp + plant_prey * E2_sp + labdet * EDL_sp + refdet * EDR_sp) * C_sp / mum_sp;
		if(!step1) step1 = small_num;

		CLEAR  = sp * C_sp / step1;
		break;

	case eat_parslow_holling1:
		if ((C_sp * tot_prey) > (mum_sp / E1_sp)) {
			CLEAR = sp * mum_sp / E1_sp;

		} else {
			CLEAR = C_sp * sp;
		}
		break;
	case eat_parslow_holling3:
		CLEAR = sp * C_sp / (1.0 + (living_prey_sq * E1_sp + plant_prey * plant_prey * E2_sp + labdet * labdet * EDL_sp + refdet * refdet * EDR_sp) * C_sp / mum_sp);
		break;
	case eat_ecosim: /* Not used at present */
		quit("At this point case 3 (ecosim-based) feeding is not implemented reset the case and try again - or ask Beth to redress this\n");
		/* QBij=aijVij/(1+hjSiaijVij) where aij is the rate of effective search by predator j for
		 type i prey, hj is the predator handling time parameter, and Vij is the instantaneous density
		 of prey type i vulnerable to predator j.

		 At each simulation time step the program updates an estimate of the ratio of predator
		 search time to total time (this ratio is given by Psj=1/(1+hjSiaijVij)  in the Holling
		 formulation), using this update to provide an improved estimate of the Vij.

		 */
		break;
	case eat_minmax:
		CLEAR = sp * C_sp / (denom_step + KU_sp);
		break;
	case eat_holling3size:
		CLEAR = sp * vl_sp * tot_prey / (1.0 + vl_sp * ht_sp);
		break;
	case eat_ratio_dependent: /* See Abrams & Ginzburg paper plus Kinzey & Punt 2009  for more details */
        CLEAR = sp * C_sp * tot_prey / (1.0 + C_sp * ht_sp * tot_prey + tot_pred_comp_sp);
		break;
    case eat_std_holling1:
        CLEAR = sp * C_sp;
        break;
    case eat_std_holling2:
        CLEAR = sp * C_sp * tot_prey / ( 1.0 + C_sp * ht_sp * tot_prey );
        break;
    case eat_std_holling3:
        CLEAR = sp * C_sp * tot_prey_sq / ( 1.0 + C_sp * ht_sp * tot_prey_sq );
        break;
    case eat_std_holling4:
        CLEAR = sp * C_sp * tot_prey / ( 1.0 + C_sp * ht_sp * tot_prey_sq );
        break;
    case eat_hassel_varley:
        step1 = pow(tot_pred, sp_hvm);
        CLEAR = sp * C_sp * tot_prey / ( small_num + C_sp * ht_sp * tot_prey + step1 );
        break;
    case eat_crowley_martin:
        CLEAR = sp * C_sp * tot_prey / ((1.0 + C_sp * ht_sp * tot_prey) * (1.0 + tot_pred_comp_sp));
        break;
	default:
		quit("No such pred_case defined (%d) - value must be between 0 and 5 currently\n", flagcase);
		break;

	}

    if (bm->flag_satiation && (CLEAR > (sp * mum_sp / E1_sp))) {
        CLEAR = sp * mum_sp / E1_sp;
    }
    
	if (CLEAR < 0.0)
		CLEAR = 0.0;

	FunctGroupArray[sp_id].CLEAR[cohort] = (double)CLEAR;

	for (preyID = 0; preyID < bm->K_num_tot_sp; preyID++) {
		if((FunctGroupArray[preyID].isOncePerDt == FALSE) || (it_count == 1)) {
			//nut_val_sensitive_sp = (int) (FunctGroupArray[preyID].speciesParams[flagnutvaleffect_id]);
			if ( (int) (FunctGroupArray[preyID].speciesParams[flagnutvaleffect_id]) ) {
				pHscalar = FunctGroupArray[preyID].pHcorr;
			} else {
				pHscalar = 1.0;
			}
			scalar = pHscalar;
            scaled_clear = pHscalar * CLEAR;

			max_hab = bm->num_active_habitats - 1;
			if(FunctGroupArray[preyID].isVertebrate == TRUE)
				max_hab = WC;

            tprey = 1.0;
			for (kij = 0; kij < FunctGroupArray[preyID].numCohortsXnumGenes; kij++) {
                
				switch (flagcase) {  /* calculate the biomass actually eaten of each prey group */
				case eat_parslow_holling2:
					for (habitat = WC; habitat <= max_hab; habitat++) {
						spGRAZEinfo[preyID][kij][habitat] = EATINGinfo[preyID][kij][habitat] * (double)scaled_clear;

                        /**
                        if((bm->which_check == sp_id ) && (bm->checkbox == bm->current_box)) {
							fprintf(llogfp,"Time %e box%d-%d it_count: %d, %s-%d eating %s hab: %d, spGRAZEinfo: %e, EATINGinfo: %e CLEAR: %Lf pHscalar: %Lf ",
									bm->dayt, bm->current_box, bm->current_layer, it_count, FunctGroupArray[sp_id].groupCode, cohort, FunctGroupArray[preyID].groupCode, habitat, spGRAZEinfo[preyID][kij][habitat], EATINGinfo[preyID][kij][habitat], CLEAR, pHscalar);
							fprintf(llogfp,"CLEAR: %Lf, step1: %Lf, living_prey: %Lf, E1_sp: %e, plant_prey: %Lf, E2_sp: %e, labdet: %Lf, EDL_sp: %e, refdet: %Lf EDR_sp: %e, C_sp: %e, mum_sp: %e, sp: %e,\n",
									CLEAR, step1, living_prey, E1_sp, plant_prey, E2_sp, labdet, EDL_sp, refdet, EDR_sp, C_sp, mum_sp, sp);

						}
                        **/

					}
					spCATCHGRAZEinfo[preyID][kij] = CATCHEATINGinfo[preyID][kij] * (double)scaled_clear;
					break;
				case eat_parslow_holling1:
					if ((C_sp * tot_prey) > (mum_sp / E1_sp)) {

						/* calculate the amount actually eaten of each prey group */
						for (habitat = WC; habitat <= max_hab; habitat++) {
							spGRAZEinfo[preyID][kij][habitat] = EATINGinfo[preyID][kij][habitat] * (double)(scaled_clear / tot_prey);
						}
						spCATCHGRAZEinfo[preyID][kij] = CATCHEATINGinfo[preyID][kij] * (double)(scaled_clear / tot_prey);
						scalar = pHscalar / tot_prey;

					} else {

						for (habitat = WC; habitat <= max_hab; habitat++) {
							spGRAZEinfo[preyID][kij][habitat] = EATINGinfo[preyID][kij][habitat] * (double)scaled_clear;
						}
						spCATCHGRAZEinfo[preyID][kij] = CATCHEATINGinfo[preyID][kij] * (double)scaled_clear;
					}
					break;
				case eat_parslow_holling3:

					for (habitat = WC; habitat <= max_hab; habitat++) {
						spGRAZEinfo[preyID][kij][habitat] = EATINGinfo[preyID][kij][habitat] * EATINGinfo[preyID][kij][habitat] * (double)scaled_clear;
					}
					spCATCHGRAZEinfo[preyID][kij] = CATCHEATINGinfo[preyID][kij] * CATCHEATINGinfo[preyID][kij] * (double)scaled_clear;

					break;
				case eat_ecosim: /* Not used at present */
					quit("At this point case 3 (ecosim-based) feeding is not implemented reset the case and try again");
					/* QBij=aijVij/(1+hjSiaijVij) where aij is the rate of effective search by predator j for
					 type i prey, hj is the predator handling time parameter, and Vij is the instantaneous density
					 of prey type i vulnerable to predator j.

					 At each simulation time step the program updates an estimate of the ratio of predator
					 search time to total time (this ratio is given by Psj=1/(1+hjSiaijVij)  in the Holling
					 formulation), using this update to provide an improved estimate of the Vij.
					 */
					break;
				case eat_minmax:

					rel_scalar = 0.0;
					for (habitat = EPIFAUNA; habitat >= max_hab; habitat--) {
						if(habitat == WC){
							catch_addition = CATCHEATINGinfo[preyID][kij];
						} else {
							catch_addition = 0.0;
						}
						rel_scalar = EATINGinfo[preyID][kij][habitat] / (EATINGinfo[preyID][kij][habitat] + catch_addition);
						spGRAZEinfo[preyID][kij][habitat] = EATINGinfo[preyID][kij][habitat] * (double)scaled_clear * mum_sp * FEEDinfo[preyID][kij][habitat] * (double)rel_scalar;
					}
					spCATCHGRAZEinfo[preyID][kij] = CATCHEATINGinfo[preyID][kij] * (double)scaled_clear * mum_sp * FEEDinfo[preyID][kij][WC] * (double)(1.0 - rel_scalar);
					break;
				case eat_holling3size:
                case eat_ratio_dependent: /* See Abrams & Ginzburg paper plus Kinzey & Punt 2009  for more details */
                case eat_std_holling1: /* All as of Skalski & Gilliam 2001 */
                case eat_std_holling2:
                case eat_std_holling4:
                case eat_hassel_varley:
                case eat_crowley_martin:
					/* Calculate the amount actually eaten of each prey group - divide by tot_prey and then multiply with individual prey group biomasses */

					for (habitat = WC; habitat <= max_hab; habitat++) {
						spGRAZEinfo[preyID][kij][habitat] = EATINGinfo[preyID][kij][habitat] * (double)(scaled_clear / tot_prey);
					}
					spCATCHGRAZEinfo[preyID][kij] = CATCHEATINGinfo[preyID][kij] * (double)(scaled_clear / tot_prey);
					scalar = pHscalar / tot_prey;
                    tprey = tot_prey;
					break;
                case eat_std_holling3:
                        for (habitat = WC; habitat <= max_hab; habitat++) {
                            spGRAZEinfo[preyID][kij][habitat] = EATINGinfo[preyID][kij][habitat] * EATINGinfo[preyID][kij][habitat] * (double)(scaled_clear / tot_prey_sq);
                        }
                        spCATCHGRAZEinfo[preyID][kij] = CATCHEATINGinfo[preyID][kij] * CATCHEATINGinfo[preyID][kij] * (double)(scaled_clear / tot_prey);
                        scalar = pHscalar / tot_prey_sq;
                        tprey = tot_prey_sq;
                    break;
				default:
					quit("No such pred_case defined (%d) - value must be between 0 and 5 currently\n", flagcase);
					break;

				}
                
                /**
                
                if((bm->which_check == sp_id ) && (bm->checkbox == bm->current_box)) {
                    fprintf(bm->logFile, "In Eat for group %s, cohort %d in box%d-%d. Amount of %s-%d eaten is first %e\n",
                            FunctGroupArray[sp_id].groupCode, cohort, bm->current_box, bm->current_layer, FunctGroupArray[preyID].groupCode, kij, spGRAZEinfo[preyID][kij][WC]);
                }
                 
                 **/

				/* Corrections when dealing with epibenthic predators or prey */
				/* if not epibenthic predator convert epibenthic prey back to m-2 */
				if (FunctGroupArray[sp_id].habitatType != EPIFAUNA) {
					if (FunctGroupArray[preyID].habitatType == EPIFAUNA) {
						spGRAZEinfo[preyID][kij][EPIFAUNA] *= smLayerThick;
					}
				} else {
					if (FunctGroupArray[preyID].groupType != REF_DET) { /* not 100% sure about this - might be a bug in the original code */

						/* if epibenthic predator convert watercolumn and sediment prey */
						if (FunctGroupArray[preyID].habitatType != EPIFAUNA) {
							spGRAZEinfo[preyID][kij][WC] /= wcLayerThick;
							spGRAZEinfo[preyID][kij][SED] /= smLayerThick;
						}

					}
				}

                /**
                if((bm->which_check == sp_id ) && (bm->checkbox == bm->current_box)) {
                    fprintf(bm->logFile, "In Eat for group %s, cohort %d in box%d-%d. Amount of %s-%d eaten is %e wcLayerThick: %e\n", FunctGroupArray[sp_id].groupCode, cohort, bm->current_box, bm->current_layer, FunctGroupArray[preyID].groupCode, kij, spGRAZEinfo[preyID][kij][WC], wcLayerThick);
                }
                 
                 **/
                
                /** Update GrazeLive - units in m-3 **/

				/* Add the epibenthic prey to graze_live in m-3 */
				if (FunctGroupArray[preyID].groupType != MICROPHTYBENTHOS) {
					graze_live += spGRAZEinfo[preyID][kij][EPIFAUNA] / smLayerThick;
				}
				if (FunctGroupArray[preyID].groupType != LAB_DET && FunctGroupArray[preyID].groupType != REF_DET && FunctGroupArray[preyID].groupType != CARRION) {
					/* Add the wc and sed values */
					graze_live += spGRAZEinfo[preyID][kij][WC];
					graze_live += spGRAZEinfo[preyID][kij][SED];
					graze_live += spCATCHGRAZEinfo[preyID][kij];
				}

				/** Check for effects of rugosity **/
                for (habitat = WC; habitat <= EPIFAUNA; habitat++) {
                	if(bm->flag_adv_habitat){
                		if (spGRAZEinfo[preyID][kij][habitat] > 0) {
							if ((FunctGroupArray[preyID].groupType == CORAL) || (FunctGroupArray[preyID].groupType == SPONGE)) {
                                thisID = (int) (FunctGroupArray[preyID].speciesParams[coralID_id]);
                                
                                CORALREEF[thisID].RugosityEaten[kij][bm->current_box] += FunctGroupArray[thisID].cohortSpeciesParams[kij][rugosity_dec_id] * spGRAZEinfo[preyID][kij][habitat] * FunctGroupArray[sp_id].speciesParams[rugFeedScalar_id];
								// TODO: Potentially make this a nonlinear or other function of predator feeding type rather than a linear scalar
							}
						}
					}

				/** Add in a check to make sure things are still in control **/
					if(!_finite(spGRAZEinfo[preyID][kij][habitat] )){
						printf("In Eat for group %s, cohort %d in box%d-%d habitat %d. Amount of %s-%d eaten is infinite\n",
								FunctGroupArray[sp_id].groupCode, cohort, bm->current_box, bm->current_layer, habitat, FunctGroupArray[preyID].groupCode, kij);
                        fprintf(bm->logFile, "In Eat for group %s, cohort %d in box%d-%d habitat %d-%d. Amount of %s eaten is infinite\n", FunctGroupArray[sp_id].groupCode, cohort, bm->current_box, bm->current_layer, habitat, FunctGroupArray[preyID].groupCode, kij);
						quit("");
					}
				}

				/** Sort out any catch consumed **/
				if (catcheater && bm->flag_fisheries_on && (spCATCHGRAZEinfo[preyID][kij])){
					for (fleet = 0; fleet < bm->K_num_fisheries; fleet++) {
						ratio = bm->FCcaughttemp[preyID][fleet][kij] * bm->pFLEET[sp_id][fleet] / (bm->groupTotCatch[preyID][kij] + small_num);
						if(!bm->flag_olddiet)
							ratio *= bm->pSPFCeat[sp_id][preyID][cohort][kij];
                        if(FunctGroupArray[preyID].isVertebrate == TRUE) {
							bm->FCcaughttemp[preyID][fleet][kij] -= spCATCHGRAZEinfo[preyID][kij] * (double)ratio * bm->cell_vol / (VERTinfo[preyID][kij][SN_id] + VERTinfo[preyID][kij][RN_id]);  // as divide by bm->cell_vol in calculating CATCHEATINFO which SPCATCHGRAZEinfo comes from
                            if(!bm->flag_replicated_old) {
                                bm->Catch[bm->current_box][preyID][fleet][bm->current_layer] -= spCATCHGRAZEinfo[preyID][kij] * (double)ratio * bm->cell_vol;
                            }
                            
                        } else {
							bm->FCcaughttemp[preyID][fleet][kij] -= spCATCHGRAZEinfo[preyID][kij] * (double)ratio;
                        }
						if (bm->FCcaughttemp[preyID][fleet][kij] < 0.0)
							bm->FCcaughttemp[preyID][fleet][kij] = 0.0;
                        
                        if(!bm->flag_replicated_old) {
                            if(bm->Catch[bm->current_box][preyID][fleet][bm->current_layer] < 0.0)
                                bm->Catch[bm->current_box][preyID][fleet][bm->current_layer] = 0.0;
                        }
					}
				}
			}
		}
	}

	FunctGroupArray[sp_id].GrazeLive[cohort] = graze_live;

    // Handle consumption of aquaculture feed
    if(FunctGroupArray[sp_id].isCultured || FunctGroupArray[sp_id].isSupplemented)
        Do_Extra_Feed(bm, llogfp, sp_id, flagcase, (double)CLEAR, mum_sp, E1_sp, (double)scalar, tprey, spGRAZEinfo);
    else
        spGRAZEinfo[AquacultFeedIndex][0][WC] = 0.0;

//	if (((bm->debug == debug_biology_process) && (bm->which_check == sp_id) && (bm->dayt >= bm->checkstart) && (bm->dayt < bm->checkstop))) {
//		fprintf(llogfp,
//				"Eat outcomes: sp_id = %s:%d, tot_prey = %.20Le, living_prey = %.20Le, plant_prey = %.20Le, labdet = %.20Le, refdet = %.20Le, GrazeLive = %.20Le, CLEAR = %.20e\n",
//				FunctGroupArray[sp_id].groupCode, cohort, tot_prey, living_prey, plant_prey, labdet, refdet, FunctGroupArray[sp_id].GrazeLive[cohort], FunctGroupArray[sp_id].CLEAR[cohort]);
//	}

	return;

}

/**
 * \brief Apply the aquaculture feeding or feeding on things from outside the model
 */
void Do_Extra_Feed(MSEBoxModel *bm, FILE *llogfp, int sp_id, int flagcase, double CLEAR, double mum_sp, double E_sp, double scalar, double tprey, double ***spGRAZEinfo) {
    switch (flagcase) {
        case eat_parslow_holling2:
            spGRAZEinfo[AquacultFeedIndex][0][WC] = EATINGinfo[AquacultFeedIndex][0][WC] * CLEAR * scalar;
            break;
        case eat_parslow_holling3:
            spGRAZEinfo[AquacultFeedIndex][0][WC] = EATINGinfo[AquacultFeedIndex][0][WC] * EATINGinfo[AquacultFeedIndex][0][WC] * CLEAR * scalar;
            break;
        case eat_ecosim: /* Not used at present */
            quit("At this point case 3 (ecosim-based) feeding is not implemented reset the case and try again");
            break;
        case eat_minmax:
            spGRAZEinfo[AquacultFeedIndex][0][WC] = EATINGinfo[AquacultFeedIndex][0][WC] * CLEAR * mum_sp * FEEDinfo[AquacultFeedIndex][0][WC] * scalar;
            break;
        case eat_parslow_holling1:
        case eat_holling3size:
        case eat_ratio_dependent:
        case eat_std_holling1:
        case eat_std_holling2:
        case eat_std_holling4:
        case eat_hassel_varley:
        case eat_crowley_martin:
            spGRAZEinfo[AquacultFeedIndex][0][WC] = EATINGinfo[AquacultFeedIndex][0][WC] * CLEAR * scalar / (tprey + small_num);
            break;
        case eat_std_holling3:
            spGRAZEinfo[AquacultFeedIndex][0][WC] = EATINGinfo[AquacultFeedIndex][0][WC] * EATINGinfo[AquacultFeedIndex][0][WC] * CLEAR * scalar / (tprey + small_num);
            break;
        default:
            quit("No such pred_case defined (%d) - value must be between 0 and 5 currently\n", flagcase);
            break;
            
    }
    
    return;
}

/**
 *	\brief Invertebrate metabolic, consumption and waste processes
 *
 */
void Invert_Activities(MSEBoxModel *bm, BoxLayerValues *boxLayerInfo, HABITAT_TYPES habitatType, FILE *llogfp, int guild, int cohort, double SP, double area_hab, double E_SP, double EDL_SP, double EDR_SP, double bact_DL,
		double bact_DR, double sedbact_DL, double sedbact_DR, double PB_scale, double BB_scale, double mL_other, double FDL_SP, double DL, double DR,
		double DLsed, double DRsed, double ***spGRAZEinfo) {
	int chrtstage = FunctGroupArray[guild].cohort_stage[cohort];
	double SPmort, SPmortNH, SPgrazePB, SPgrazeBB, SPgrazeDR, SPgrazeDL, SPprodnDET;
	double mL_SP = Ecology_Get_Linear_Mortality(bm, guild, cohort, chrtstage) + mL_other;
	double mQ_SP = FunctGroupArray[guild].cohortSpeciesParams[chrtstage][mQ_id];
	double mE_SP = FunctGroupArray[guild].cohortSpeciesParams[chrtstage][mE_id];
    double mS_SP = Acidif_Mort(bm, guild);
	double FDM_SP = FunctGroupArray[guild].speciesParams[FDMort_id];
	double FDG_SP = FunctGroupArray[guild].speciesParams[FDG_id];
	double FDGDL_SP = FunctGroupArray[guild].speciesParams[FDGDL_id];
	double FDGDR_SP = FunctGroupArray[guild].speciesParams[FDGDR_id];
	double cyst_growth = 0.0;
	double cyst_transfer = 0.0;
	double dummy = MAXDOUBLE;
	double sp_GrazeFeed = 0.0;
    double aquacult_fry = 0.0;
	char code[100];
	int habitat;
	double scalar;
	double mortality_scalar;

	/* If there is a scalar to apply then grab it now */
	if(bm->scale_all_mortality == TRUE){
		mortality_scalar = Ecology_Get_Mortality_Scalar(bm, guild, cohort);
	}else{
		mortality_scalar = 1.0;
	}

	if (FunctGroupArray[guild].groupAgeType == AGE_STRUCTURED_BIOMASS && cohort == 0)
		sprintf(code, "j%s", FunctGroupArray[guild].groupCode);
	else
		sprintf(code, "%s", FunctGroupArray[guild].groupCode);


    /*
	//if (((bm->debug == debug_biology_process) && (bm->dayt >= bm->checkstart) && (bm->dayt < bm->checkstop))) {
    if (FunctGroupArray[guild].habitatType == EPIFAUNA) {
		fprintf(
				llogfp,
				"Invert_Activities, sp_id = %s, sp = %.20e, E_SP = %.20e, EDL_SP = %.20e, EDR_SP = %.20e, bact_DL = %.20e, bact_DR = %.20e, sedbact_DL = %.20e, sedbact_DR = %.20e, PB_scale = %.20e, BB_scale = %.20e, mL_other = %.20e, SPgrazeLive = %.20Le, FDL_SP = %.20e, DL = %.20e, DR = %.20e, DLsed = %.20e, DRsed = %.20e, mL_SP = %e, mQ_SP = %e\n\n ",
				code, SP, E_SP, EDL_SP, EDR_SP, bact_DL, bact_DR, sedbact_DL, sedbact_DR, PB_scale, BB_scale, mL_other,
				FunctGroupArray[guild].GrazeLive[cohort], FDL_SP, DL, DR, DLsed, DRsed, mL_SP, mQ_SP);
	}
     */
    
	SPgrazeDR = 0.0;
	SPgrazeDL = 0.0;

	for (habitat = WC; habitat <  bm->num_active_habitats; habitat++) {
		SPgrazeDR += spGRAZEinfo[RefDetIndex][0][habitat];
		SPgrazeDL += spGRAZEinfo[LabDetIndex][0][habitat];
	}

	/* Start with grazing on bacteria - in m-3 */
	SPgrazePB = (spGRAZEinfo[LabDetIndex][0][WC] * bact_DL / (DL + small_num) + spGRAZEinfo[RefDetIndex][0][WC] * bact_DR / (DR + small_num)) * PB_scale;

	SPgrazeBB = (spGRAZEinfo[LabDetIndex][0][SED] * sedbact_DL / (DLsed + small_num) + spGRAZEinfo[RefDetIndex][0][SED] * sedbact_DR / (DRsed + small_num)) * BB_scale;

	FunctGroupArray[guild].GrazeLive[cohort] += SPgrazePB + SPgrazeBB;

    /* Handle grazing of aquaculture feed */
    if(FunctGroupArray[guild].isCultured || FunctGroupArray[guild].isSupplemented)
        sp_GrazeFeed = GRAZEinfo[AquacultFeedIndex][0][WC];
    else
        sp_GrazeFeed = 0.0;
    
    //* Check whether overwintering or not */
	FunctGroupArray[guild].speciesParams[overwintering_id] = Check_OverWintering(bm, llogfp, guild);
	if (FunctGroupArray[guild].speciesParams[overwintering_id] > 0)
		Encystment(bm, llogfp, guild, SP, dummy, &cyst_growth, &cyst_transfer);  // Enter nutrients as MAXDOUBLE as invertebrate consumers don't trigger on this
	else {
		cyst_growth = 0.0;
		cyst_transfer = 0.0;
	}

    /* Get Aquaculture fry additions - correcting from total biomass to mg/m3 */
    if (FunctGroupArray[guild].isCultured){
        aquacult_fry = FunctGroupArray[guild].speciesParams[aquacult_fry_id] * area_hab * bm->recruit_hdistrib[0][bm->current_box][guild] * tonne_2_mg / (bm->boxes[bm->current_box].area * wcLayerThick);
    }
    
	/* Do mortality */
	bm->calcMLinearMort[guild][current_id] += (mortality_scalar * (mL_SP * SP + cyst_transfer));
	bm->calcELinearMort[guild][current_id] += (mortality_scalar * ((mS_SP + mE_SP) * SP));
	bm->calcMQuadMort[guild][current_id] += (mortality_scalar * (mQ_SP * SP * SP));
	SPmort = mortality_scalar * ((mS_SP + mL_SP + mE_SP + mQ_SP * SP) * SP + cyst_transfer);

	FunctGroupArray[guild].mortality[cohort] = SPmort;
	bm->calcTrackedMort[guild][cohort][0][ongoingM1_id] += (FunctGroupArray[guild].mortality[cohort] * FunctGroupArray[guild].speciesParams[Mdt_id]);

    /*
    if((guild == bm->which_check) && (bm->current_box == bm->checkbox)) {
        fprintf(llogfp, "Time: %e box-%d-%d %s has mort: %e as mortality_scalar: %e mS_SP: %e mL_SP: %e mE_SP: %e mQ_SP: %e SP: %e cyst_transfer: %e\n",
                bm->dayt, bm->current_box, bm->current_layer, FunctGroupArray[guild].groupCode, SPmort, mortality_scalar, mS_SP, mL_SP, mE_SP, mQ_SP, SP, cyst_transfer);
    }
    */

	if(bm->track_contaminants){
		Contaminant_Record_Death(bm, guild, cohort, SP);
	}

	/* If epibenthos need to get in m-3 for the rest of detritus and NH release calculations */
	if (FunctGroupArray[guild].groupType == SED_EP_FF) {
		SPmortNH = SPmort / wcLayerThick;
		SPmort /= smLayerThick;
	} else if (FunctGroupArray[guild].groupType == SED_EP_OTHER || FunctGroupArray[guild].groupType == MOB_EP_OTHER) {
		SPmortNH = SPmort / smLayerThick;
		SPmort /= smLayerThick;
	} else {
		SPmortNH = SPmort;
	}

	/* Do detritus production */
	SPprodnDET = ((double)FunctGroupArray[guild].GrazeLive[cohort] + sp_GrazeFeed) * (1.0 - E_SP) * FDG_SP + SPgrazeDR * (1.0 - EDR_SP) * FDGDR_SP + SPgrazeDL * (1.0 - EDL_SP) * FDGDL_SP + SPmort * FDM_SP;

 	/* Return the final growth, waste etc values */
	spGRAZEinfo[pelagicBactIndex][0][WC] = SPgrazePB;
	spGRAZEinfo[SedBactIndex][0][SED] = SPgrazeBB;

	/* Growth */
	FunctGroupArray[guild].grazing[cohort] = (double)FunctGroupArray[guild].GrazeLive[cohort] + SPgrazeDR + SPgrazeDL + sp_GrazeFeed;
	FunctGroupArray[guild].growth[cohort] = (E_SP * ((double)FunctGroupArray[guild].GrazeLive[cohort] + sp_GrazeFeed) + EDR_SP * SPgrazeDR + EDL_SP * SPgrazeDL + cyst_growth + aquacult_fry);
	if(bm->track_atomic_ratio == TRUE){
		if (FunctGroupArray[guild].groupType == SED_EP_FF || FunctGroupArray[guild].groupType == SED_EP_OTHER || FunctGroupArray[guild].groupType
					== MOB_EP_OTHER)
			scalar = smLayerThick;
		else
			scalar = 1.0;

		if (FunctGroupArray[guild].groupType == CEP)
			Transfer_To_Pred(bm, boxLayerInfo, spGRAZEinfo, habitatType, guild, cohort, scalar, E_SP, EDR_SP, EDL_SP, WC);
		else
			Transfer_To_Pred(bm, boxLayerInfo, spGRAZEinfo, habitatType, guild, cohort, scalar, E_SP, EDR_SP, EDL_SP, habitatType);

	}

	/**
	 Ammonia and detritus production by grazers.

	 There are two ways in which grazers can contribute to both the ammonia pool
	 and the labile detritus pool. Part of the losses which occur between grazing
	 and growth, and are summarised by the growth efficiency E, can be attributed
	 to production of particulate detritus either through inefficient feeding or
	 faecal production. The remainder are due to respiration, and excretion of the
	 associated nitrogen as ammonia. The fraction allocated to detritus is
	 denoted by FDG. In the model, only one size class of labile detritus is
	 used, and this sinks comparatively rapidly. However, the particulate
	 material produced by small_num phytoplankton and microzooplankton is very small,
	 and should remain in the water column. To correct for this, a larger
	 fraction of the losses from microzooplankton are directly recycled as ammonia.
	 The other losses which occur from the grazer pool are due to mortality.
	 Again, these are partitioned between detritus production and ammonia
	 production, with a fraction FDM going to detritus. It follows that:
	 **/

	FunctGroupArray[guild].releaseNH[cohort] = SPmortNH * (1.0 - FDM_SP) + ((double)FunctGroupArray[guild].GrazeLive[cohort] + sp_GrazeFeed) * (1.0 - E_SP)
			* (1.0 - FDG_SP) + SPgrazeDR * (1.0 - EDR_SP) * (1.0 - FDGDR_SP) + SPgrazeDL * (1.0 - EDL_SP) * (1.0 - FDGDL_SP);

	FunctGroupArray[guild].prodnDL[cohort] = SPprodnDET * FDL_SP;
	FunctGroupArray[guild].prodnDR[cohort] = SPprodnDET * (1.0 - FDL_SP);

	/* If epibenthos need to set back to m-2 */
	if (FunctGroupArray[guild].groupType == SED_EP_FF || FunctGroupArray[guild].groupType == SED_EP_OTHER || FunctGroupArray[guild].groupType
			== MOB_EP_OTHER)
		FunctGroupArray[guild].growth[cohort] *= smLayerThick;

    /*
	if ((bm->which_check == guild) && (bm->current_box == bm->checkbox)) {
		fprintf(llogfp,
				"Time: %e, Invert_Activities outcomes: sp_id = %s, SP = %.20e, SPgrazePB = %.20e, SPgrazeBB = %.20e, GrazeLive = %.20Le, mortality = %.20e, grazing = %.20e, growth = %.20e, releaseNH = %.20e, prodnDL = %.20e, prodnDR = %.20e\n",
				bm->dayt, code, SP, SPgrazePB, SPgrazeBB, FunctGroupArray[guild].GrazeLive[cohort],
				FunctGroupArray[guild].mortality[cohort], FunctGroupArray[guild].grazing[cohort],
				FunctGroupArray[guild].growth[cohort], FunctGroupArray[guild].releaseNH[cohort],
				FunctGroupArray[guild].prodnDL[cohort], FunctGroupArray[guild].prodnDR[cohort]);
	}
    */
    
    if(bm->track_atomic_ratio == TRUE){
        /* P release due to respiration and excretion*/
        Calculate_Element_Release(bm, boxLayerInfo, guild, FunctGroupArray[guild].releaseNH[cohort], WC, WC); // For now assume all releases to the Water column
    }

	return;
}

/**
 *	\brief Whether overwintering (encysting or not) - based on Samuelsen et al (2009) MEPS 386: 163�180
 *
 */
double Check_OverWintering(MSEBoxModel *bm, FILE *llogfp, int sp_id)
{
	int ans = 0;
	int overStartDay = 0;
	int overEndDay = 0;
	double overStartTemp = 0.0;
	double overEndTemp = 0.0;

	switch(FunctGroupArray[sp_id].isOverWinter) {
	case no_overwintering:
		ans = 0;
		break;
	case date_overwintering:
		overStartDay = (int)(FunctGroupArray[sp_id].speciesParams[overwinterStartTofY_id]);
		overEndDay = (int)(FunctGroupArray[sp_id].speciesParams[overwinterEndTofY_id]);

		if(overStartDay <= overEndDay){
			if ((bm->TofY >= overStartDay) && (bm->TofY < overEndDay)){
				ans = 1;
			} else {
				ans = 0;
			}
		} else {
			if ((bm->TofY >= overStartDay) || (bm->TofY < overEndDay)){
				ans = 1;
			} else {
				ans = 0;
			}
		}
		break;
	case enviro_overwintering:
		overStartTemp = (int)(FunctGroupArray[sp_id].speciesParams[overwinterStartTemp_id]);
		overEndTemp = (int)(FunctGroupArray[sp_id].speciesParams[overwinterEndTemp_id]);

		if(H2Otemp < overStartTemp)
			ans = 1;
		if(H2Otemp > overEndTemp)
			ans = 0;

		break;
	default:
		ans = 0;
		break;
	}

	return ans;
}

/**
 *	\brief Primary productivity
 *
 */
void Primary_Production(MSEBoxModel *bm, FILE *llogfp, int sp_id, int micro_case, int lim_case, int macro_producer, double sp_biom,
		double DIN, double NH, double NO, double Si, double Fe, double P, double PRatio, double C, double CRatio,
		double IRR, double mum, double E_sp, double mL_other, double SPmax,
		double area_hab, double *spUptakeNO,
		double *spUptakeSi, double *spUptakeFe, double *spUptakeP,  double *spUptakeC, double *sphN) {

	double hN_sp, hI_sp, sp_grow, scale_uptake, uptakeNH, uptakeNO, uptakeSi, uptakeFe, uptakeP = 0, uptakeC = 0;
	double mL_sp = Ecology_Get_Linear_Mortality(bm, sp_id, 0, 0) + mL_other;
	double mE_sp = FunctGroupArray[sp_id].cohortSpeciesParams[0][mE_id];
    double mS_sp = Acidif_Mort(bm, sp_id);
	double cyst_transfer = 0.0;
	double prop_daylight;
	double pH_lim;
	double P0 = FunctGroupArray[sp_id].speciesParams[P_min_internal_id];
	double P1 = FunctGroupArray[sp_id].speciesParams[P_max_internal_id];
	double mortality_scalar;
    
    /**/
	//if ((bm->debug == debug_biology_process) && (bm->dayt >= bm->checkstart) && (bm->dayt < bm->checkstop)) {
    if((bm->dayt > 3633) && (FunctGroupArray[sp_id].groupType == TURF) && (bm->current_box == 24)) {
		fprintf(
			llogfp,
			"Primary_Production parameters: sp_id = %d, micro_case = %d, lim_case = %d, macro_producer = %d, sp = %.20e, DIN = %.20e, NH = %.20e, NO = %.20e, Si = %.20e, Fe = %.20e, KN_sp = %.20e, KS_sp = %.20e, KF_sp = %.20e, KI_sp = %.20e, IRR = %.20e, OPT_IRR_sp = %.20e, mum_sp = %.20e, E_sp = %.20e, KLYS_sp = %.20e, mL_other = %.20e, SPmax = %.20e, area_hab = %.20e, mL_sp = %.20e\n",
			sp_id, micro_case, lim_case, macro_producer, sp_biom, DIN, NH, NO, Si, Fe, FunctGroupArray[sp_id].speciesParams[KN_id],
			FunctGroupArray[sp_id].speciesParams[KS_id], FunctGroupArray[sp_id].speciesParams[KF_id], FunctGroupArray[sp_id].speciesParams[KI_id], IRR,
			FunctGroupArray[sp_id].SP_IRR, mum, E_sp, FunctGroupArray[sp_id].speciesParams[KLYS_id],
			mL_other, SPmax, area_hab, mL_sp);

	}
    /**/
    
	/* Loop over strata here */

	/* Calculate limitation factors */
	hN_sp = Nutrient_Lim(bm, lim_case, micro_case, DIN, Si, Fe, P, PRatio, P0, P1,
			FunctGroupArray[sp_id].speciesParams[KN_id], FunctGroupArray[sp_id].speciesParams[KS_id],
			FunctGroupArray[sp_id].speciesParams[KF_id], FunctGroupArray[sp_id].speciesParams[KP_id]);

	if(hN_sp <= small_num){

		fprintf(bm->logFile, "group %s is nutrient limited in box %d, layer %d, DIN (NO + NH) = %e, Si = %e, Fe = %e, P = %e\n",  FunctGroupArray[sp_id].groupCode, bm->current_box, bm->current_layer,
						DIN, Si, Fe, P);
		warn("group %s is nutrient limited in box %d, layer %d, DIN (NO + NH) = %e, Si = %e, Fe = %e, P = %e\n",  FunctGroupArray[sp_id].groupCode, bm->current_box, bm->current_layer,
				DIN, Si, Fe, P);
	}

	*sphN = hN_sp;

	prop_daylight = bm->boxes[bm->current_box].prop_light_time;
	hI_sp = Light_Lim(bm, bm->flaglight, IRR, FunctGroupArray[sp_id].speciesParams[KI_id], FunctGroupArray[sp_id].SP_IRR, prop_daylight, 0.0);

	/* Calculate pH limitation  - leave out for now as captured through changed growth rates
	k_pH = FunctGroupArray[sp_id].speciesParams[KpH_id];
	pH_lim = pH / (k_pH + pH);
	*/
	pH_lim = 1.0;

	/* eddy_strength is a value per box. Think this is read in from the bgm file?
	 * Eddy_scale is an input parameter from the input biology file
	 */
	/* Calculate resulting growth */
	if (!macro_producer){
		sp_grow = sp_biom * mum * hN_sp * hI_sp * eddy_strength * bm->eddy_scale * pH_lim;
	} else {
		sp_grow = sp_biom * mum * hN_sp * hI_sp * eddy_strength * bm->eddy_scale * pH_lim
				* min(1.0,max(0.0,(1.0 - sp_biom / (SPmax * area_hab + small_num))));

		if(area_hab == 0  && it_count == 1){
            if (bm->newmonth)
                warn("Group %s has no suitable substrate habitats in box %d, layer %d. Will result in 0 growth.\n",
					FunctGroupArray[sp_id].groupCode, bm->current_box, bm->current_layer);
		}
	}

	/* Check for encystment
	 *
	 * Cysts "hatching" = add to growth
	 * Cysts forming and sinking = local additional mortality with cysts being moved into a global to settle on the bottom
	 *
	 */
	FunctGroupArray[sp_id].speciesParams[overwintering_id] = Check_OverWintering(bm, llogfp, sp_id);
	if (FunctGroupArray[sp_id].speciesParams[overwintering_id] > 0){
		Encystment(bm, llogfp, sp_id, sp_biom, DIN, &sp_grow, &cyst_transfer);
	}

	FunctGroupArray[sp_id].growth[0] = sp_grow;

	/* End loop over strata */

	/* Calculate nutrient uptake */
	uptakeNH = sp_grow * (NH / (FunctGroupArray[sp_id].speciesParams[KN_id] + NH)) * ((FunctGroupArray[sp_id].speciesParams[KN_id] + DIN) / DIN);
	uptakeNO = sp_grow * (NO / (FunctGroupArray[sp_id].speciesParams[KN_id] + DIN))
			* (FunctGroupArray[sp_id].speciesParams[KN_id]	/ (FunctGroupArray[sp_id].speciesParams[KN_id] + NH))
			* (1.0 + FunctGroupArray[sp_id].speciesParams[KN_id] / DIN);

	if (lim_case != one_nut_lim)
		uptakeSi = sp_grow * X_SiN;
	else
		uptakeSi = 0.0;

	if (micro_case)
		uptakeFe = sp_grow * X_FeN;
	else
		uptakeFe = 0.0;

	/* Cap nutrient uptake to what is available */
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

	/* If there is a scalar to apply then grab it now */
	if(bm->scale_all_mortality == TRUE){
		mortality_scalar = Ecology_Get_Mortality_Scalar(bm, sp_id, 0);
	}else{
		mortality_scalar = 1.0;
	}

	/* Calculate mortality and lysis - do before P and C calcs.*/
	FunctGroupArray[sp_id].mortality[0] = mortality_scalar * ((mS_sp + mL_sp + mE_sp) * sp_biom + cyst_transfer);
	FunctGroupArray[sp_id].lysis[0] = mortality_scalar * (FunctGroupArray[sp_id].speciesParams[KLYS_id] * sp_biom / (hN_sp + 0.1));

	if(bm->track_atomic_ratio == TRUE && sp_biom > bm->min_pool){

		/* Calculate P uptake */
		uptakeP = Calculate_Phosphorus_Uptake(bm, sp_id, sp_grow, PRatio);

		/* Now do carbon */
		uptakeC = Calculate_Carbon_Uptake(bm, sp_id, sp_grow, CRatio);
	}

	if(bm->ecotest > 1){
		if(!_finite(FunctGroupArray[sp_id].uptakeNH[0])){
			printf("group %s - uptakeNH = %e, scale_uptake = %e, DIN = %e, NH= %e, sp_grow= %e\n",
					FunctGroupArray[sp_id].groupCode, uptakeNH, scale_uptake, DIN, NH, sp_grow);
			quit("ERROR: uptakeNH is infinite.\n");
		}
	}
	*spUptakeNO = uptakeNO * scale_uptake;
	*spUptakeSi = uptakeSi * scale_uptake;
	*spUptakeFe = uptakeFe * scale_uptake;
	*spUptakeP = uptakeP * scale_uptake;
	*spUptakeC = uptakeC * scale_uptake;

	if(bm->track_atomic_ratio == TRUE){
		PP_uptake(bm, sp_id, FunctGroupArray[sp_id].uptakeNH[0] + *spUptakeNO, *spUptakeP , *spUptakeC);
	}

	bm->calcMLinearMort[sp_id][current_id] += (mortality_scalar * (mL_sp * sp_biom + cyst_transfer));
	bm->calcELinearMort[sp_id][current_id] += (mortality_scalar * (mE_sp * sp_biom));
	//bm->calcMQuadMort[sp_id][current_id] += (mQ_sp * sp_biom * sp_biom);

	bm->calcTrackedMort[sp_id][0][0][ongoingM1_id] += (mortality_scalar * (FunctGroupArray[sp_id].mortality[0] * FunctGroupArray[sp_id].speciesParams[Mdt_id]));

	if(bm->track_contaminants){
		Contaminant_Record_Death(bm, sp_id, 0, sp_biom);
	}

	/* For dinoflagellates calculate maximum growth rate possible if nutrients non-limiting */
	if (FunctGroupArray[sp_id].groupType == DINOFLAG){
		if(E_sp > 0)
			FunctGroupArray[sp_id].maxPhagotrophy = sp_biom * (mum / E_sp) * hI_sp * (1.0 - hN_sp);
		else
			FunctGroupArray[sp_id].maxPhagotrophy = 0.0;
	}
	else{
		FunctGroupArray[sp_id].maxPhagotrophy = 0.0;
	}

	/**
	 if(bm->debug && ((bm->debug > debug_stock) && (sp_id == bm->which_check))){
	 fprintf(llogfp,"sp: %s, uptakeNH: %e, uptakeNO: %e, uptakeSi: %e\n",
	 FunctGroupArray[sp_id].groupCode, *sp_uptakeNH,  *sp_uptakeNO,  *sp_uptakeSi);
	 fprintf(llogfp,"uptakeFE: %e, growth: %e, mort: %e, lysis: %e, scale_uptake: %e\n",
	 *sp_uptakeFe, *sp_growth, *sp_mortality, *sp_lysis, scale_uptake);
	 }
	*/
     //if (((bm->debug == debug_biology_process) && (bm->dayt >= bm->checkstart) && (bm->dayt < bm->checkstop))){
    //if ((bm->current_box == bm->checkbox) && (bm->dayt >= bm->checkstart) && (bm->dayt < bm->checkstop) && (bm->which_check == sp_id)){
     if((bm->dayt > 3633) && (FunctGroupArray[sp_id].groupType == TURF) && (bm->current_box == 24)) {
		fprintf(
			llogfp,
			"Time: %e Primary_Production outcomes: sp_id = %s, box%d-%d, growth = %.20e, uptakeNH = %.20e, uptakeNO = %.20e, uptakeSi = %.20e, uptakeFe = %.20e, mortality = %.20e, lysis = %.20e, maxPhagotrophy = %.20e, hN_sp = %.20e\n",
			bm->dayt, FunctGroupArray[sp_id].groupCode, bm->current_box, bm->current_layer, FunctGroupArray[sp_id].growth[0], FunctGroupArray[sp_id].uptakeNH[0], *spUptakeNO, *spUptakeSi, *spUptakeFe,
			FunctGroupArray[sp_id].mortality[0], FunctGroupArray[sp_id].lysis[0], FunctGroupArray[sp_id].maxPhagotrophy,
			*sphN);

		fprintf(llogfp,"Time: %e box%d-%d, %s sp_grow: %e, sp_biom: %e, mum: %e, hN_sp: %e, hI_sp: %e, eddy_strength: %e, eddy_scale: %e, min: %e, (SPmax = %e, area_hab = %e)\n",
				bm->dayt, bm->current_box, bm->current_layer, FunctGroupArray[sp_id].groupCode, sp_grow, sp_biom, mum, hN_sp,
					hI_sp, eddy_strength, bm->eddy_scale, min(1.0,max(0.0,(1.0 - sp_biom / (SPmax * area_hab + small_num)))), SPmax, area_hab);

	}
    /**/
    
	return;
}

/**
 *	\brief Encystment of primary producers - based on model by Hense and Beckmann (2010) Ecological Modelling 221:2330�2338
 *
 */
void Encystment(MSEBoxModel *bm, FILE *llogfp, int sp_id, double sp_biom, double DIN, double *sp_grow, double *cyst_transfer)
{
	int encyst_rollover;
	double this_grow = *sp_grow;
	double encyst_rate = FunctGroupArray[sp_id].speciesParams[encyst_rate_id];
	double hatch_rate = FunctGroupArray[sp_id].speciesParams[hatch_rate_id];
	double crit_growth = FunctGroupArray[sp_id].speciesParams[crit_mum_id];
	double crit_nut = FunctGroupArray[sp_id].speciesParams[crit_nut_id];
	double crit_temp = FunctGroupArray[sp_id].speciesParams[crit_temp_id];
	int hatch_TofY = (int)(FunctGroupArray[sp_id].speciesParams[overwinterEndTofY_id]);
	int encyst_TofY = (int)(FunctGroupArray[sp_id].speciesParams[overwinterStartTofY_id]);
	int encyst_period = (int)(FunctGroupArray[sp_id].speciesParams[encyst_period_id]);
	int flagencyst = (int)(FunctGroupArray[sp_id].speciesParams[flagencyst_id]);

	double this_cyst_hatch = 0.0;
	double this_encyst = 0.0;

	/* Deal with wrapping event */
	if((encyst_TofY + encyst_period) > 365) {
		encyst_rollover = (encyst_TofY + encyst_period) - 365;
		encyst_TofY = 0;
		encyst_period = encyst_rollover;
	}

	/* Form cysts */
	switch(flagencyst){
	case no_overwintering: // Do nothing
		break;
	case date_overwintering:
		if((encyst_TofY <= bm->TofY) && (bm->TofY <= (encyst_TofY + encyst_period))) {
			this_encyst = encyst_rate * sp_biom;
			cysts[sp_id][bm->current_box][cystenter_id] += this_encyst;
		}
		break;
	case enviro_overwintering:
		if(this_grow < crit_growth){
			this_encyst = encyst_rate * sp_biom;
			cysts[sp_id][bm->current_box][cystenter_id] += this_encyst;
		}
		break;
	default: // Do nothing
		break;
	}

	/* Deal with wrapping event */
	if((hatch_TofY + encyst_period) > 365) {
		encyst_rollover = (hatch_TofY + encyst_period) - 365;
		hatch_TofY = 0;
		encyst_period = encyst_rollover;
	}

	/* Leave cyst state */
	switch(flagencyst){
	case no_overwintering: // Do nothing
		break;
	case date_overwintering:
		if((hatch_TofY <= bm->TofY) && (bm->TofY <= (hatch_TofY + encyst_period))) {
			this_cyst_hatch = hatch_rate * cysts[sp_id][bm->current_box][cystenter_id];
			cysts[sp_id][bm->current_box][cystexit_id] += this_cyst_hatch;
			cysts[sp_id][bm->current_box][cystenter_id] -= this_cyst_hatch;
		}
		break;
	case enviro_overwintering:
		if((DIN < crit_nut) || (H2Otemp > crit_temp)) {
			this_cyst_hatch = hatch_rate * cysts[sp_id][bm->current_box][cystenter_id];
			cysts[sp_id][bm->current_box][cystexit_id] += this_cyst_hatch;
			cysts[sp_id][bm->current_box][cystenter_id] -= this_cyst_hatch;
		}
		break;
	default: // Do nothing
		break;
	}

	// Sanity checks
	if(cysts[sp_id][bm->current_box][cystenter_id] < 0.0)
		cysts[sp_id][bm->current_box][cystenter_id] = 0.0;
	if(cysts[sp_id][bm->current_box][cystexit_id] < 0.0)
		cysts[sp_id][bm->current_box][cystexit_id] = 0.0;

	if(bm->current_layer == (bm->boxes[bm->current_box].nz-1)){
		// Add hatched cysts to population growth and ten zero out the cyst pool
		*sp_grow += cysts[sp_id][bm->current_box][cystexit_id];
		cysts[sp_id][bm->current_box][cystexit_id] = 0.0;
	}

	*cyst_transfer = this_encyst; // To be applied as mortality back in the calling routines routine

	return;
}

/**
 *	\brief Availability of sediment fauna to epibenthos and pelagos
 *
 */
void Calculate_Sediment_Prey_Avail(MSEBoxModel *bm, BoxLayerValues *boxLayerInfo, int guild, double ***spPREYinfo,
		double *avail_BB) {
	double depth_scalar = 1.0;
	double biomass, avail, corrected_avail;
	int prey, prey_chrt;
	double KDEP_sp = FunctGroupArray[guild].speciesParams[KDEP_id];

	*avail_BB = 0.0;
	if (KDEP_sp < O2depth)
		depth_scalar = (O2depth - KDEP_sp) / (O2depth + small_num);
	else
		depth_scalar = 1.0;

	//	if ((bm->debug == debug_biology_process) && (bm->dayt >= bm->checkstart) && (bm->dayt < bm->checkstop)) {
	//		fprintf(bm->logFile, "Prey_Avail parameters: guild = %d, KDEP_sp = %e, sDLscale = %e, sDRscale = %e, sDCscale = %e, depth_scalar = %e\n", guild, KDEP_sp,
	//				boxLayerInfo->sDLscale, boxLayerInfo->sDRscale, boxLayerInfo->sDCscale, depth_scalar);
	//	}

	for (prey = 0; prey < bm->K_num_tot_sp; prey++) {
		if (FunctGroupArray[prey].speciesParams[flag_id] == TRUE) {

			for (prey_chrt = 0; prey_chrt < FunctGroupArray[prey].numCohortsXnumGenes; prey_chrt++) {

				if (FunctGroupArray[prey].groupType == LG_INF ||
						FunctGroupArray[prey].groupType == SM_INF ||
						FunctGroupArray[prey].groupType == MICROPHTYBENTHOS ||
						prey == SedBactIndex ||
						prey == LabDetIndex ||
						prey == RefDetIndex ||
						prey == CarrionIndex) {

					biomass = boxLayerInfo->localSEDTracers[FunctGroupArray[prey].totNTracers[prey_chrt]];
					avail = 1.0;

					/* Find availability */
					Avail(0, biomass, KDEP_sp, &avail);

					switch (FunctGroupArray[prey].groupType) {
					case SED_BACT:
						*avail_BB = depth_scalar;
						//				if ((bm->debug == debug_biology_process) && (bm->dayt >= bm->checkstart) && (bm->dayt < bm->checkstop)) {
						//					fprintf(llogfp, "Prey_Avail avail_BB = %e\n", *avail_BB);
						//				}
						continue;
						break;
					case CARRION:
						spPREYinfo[prey][prey_chrt][SED] = avail * boxLayerInfo->sDCscale;
						break;
					case LAB_DET:
						spPREYinfo[prey][prey_chrt][SED] = avail * boxLayerInfo->sDLscale;
						break;
					case REF_DET:
						spPREYinfo[prey][prey_chrt][SED] = avail * boxLayerInfo->sDRscale;
						break;
					default:
						/* Depth correct */
						corrected_avail = avail * depth_scalar;

						/* Update prey availability */
						spPREYinfo[prey][prey_chrt][SED] = corrected_avail;

						break;
					}
				}
			}
		}
	}
	return;
}
/**
 *	\brief Availability in the sediment
 *
 */
void Avail(int aerob_case, double sp, double Depth, double *avail_sp) {
	double BotDpth, TopDpth, PercentDpth;
    
    if (current_layer_sed < 0)
        quit("Avail: What are we doing in the sediment if current_layer_sed < 0?\n");

    /* Get the top and bottom depth*/
	BotDpth = current_layer_sed * smLayerThick + smLayerThick;
	TopDpth = current_layer_sed * smLayerThick;
	if (TopDpth < 0.0)
		TopDpth = 0.0;
	PercentDpth = (Depth - TopDpth) / smLayerThick;

	switch (aerob_case) {
	/*** Aerobic case ***/
	case aerobic_sed:
		/* Layer completely above depth down to which DR completely available to that
		 species then all detritus is available. */
		if (BotDpth <= Depth)
			*avail_sp = sp;
		else if (TopDpth < Depth)
			/* Layer spans the depth down to which DR completely available to that species
			 then only a % of the detritus is available. (=% layer above exploitation depth).
			 */
			*avail_sp = sp * PercentDpth;
		else
			/* Layer below the depth down to which DR completely available to that species
			 then no detritus is available. */
			*avail_sp = 0.0;
		break;

		/*** Anaerobic case ***/
	case anaerobic_sed:
		/* Layer completely above depth down to which DR becomes available to that
		 species then no detritus is available. */
		if (BotDpth <= Depth)
			*avail_sp = 0.0;
		else if (TopDpth < Depth)
			/* Layer spans the depth down to which DR becomes available to that species
			 then only a % of the detritus is available. (=% layer below exploitation depth).
			 */
			*avail_sp = sp * (1.0 - PercentDpth);
		else
			/* Layer below the depth down to which DR becomes available to that species
			 then all detritus is available. */
			*avail_sp = sp;
		break;
	default:
		quit("No such aerob_case defined (%d) - value must be between 0 and 1 currently\n", aerob_case);
		break;
	}

	return;

}

/**
 *	\brief Effects of oxygen (limitation)
 *
 */
double Oxygen(int O2_case, double oxy, double lethal, double lim, double mD_sp) {
	double hO = 0, xx, mm, effect, hO1, hO2;

	switch (O2_case) {
	case O2_ambient_id:
		xx = (oxy - lethal) * (oxy - lethal);
		mm = (lim - lethal) * (lim - lethal);
		hO = Util_Mich_Ment(xx, mm);
		/* If statement used to deal with low/absent oxygen as then don't get positive effect
		 of negative oxygen, that would have got just using quadratic Michelis-Mentin. IGEBM
		 used cubic Michelis-Mentin, but this would actually give negative growth effect (hO < 0).
		 */
		if (oxy < 0.0)
			hO = 0.0;
		break;
	case O2_depth_id:
		hO = O2depth / (O2depth + mD_sp + small_num);
		break;
	case O2_IGBEM_id:
		effect = 1.0;
		/* Was a formulation in ERSEM, but never reached a point where less than 1.0 anyway */
		hO = effect / (lethal + effect);
		break;
	case O2_quad_id:
		xx = (oxy - lethal) * (oxy - lethal);
		mm = (lim - lethal) * (lim - lethal);
		hO1 = Util_Mich_Ment(xx, mm);
		/* If statement used to deal with low/absent oxygen as then don't get positive effect
		 of negative oxygen, that would have got just using quadratic Michelis-Mentin. IGEBM
		 used cubic Michelis-Mentin, but this would actually give negative growth effect (hO < 0).
		 */
		if (oxy < 0.0)
			hO1 = 0.0;
		hO2 = O2depth / (O2depth + mD_sp + small_num);
		hO = max(hO1, hO2);
		break;
	default:
		quit("No such O2_case defined (%d) - value must be between 0 and 3 currently\n", O2_case);
		break;
	}

	if(isnan(hO)){
		quit("Oxygen: resulted in a nan value for h0 (%e). O2_case = %d, oxy = %e, lethal = %e, lim = %e, mD_sp = %e\n", hO, O2_case, oxy, lethal, lim, mD_sp);
	}
	return (hO);
}
/*
 *	\brief Biophysical activity in the sediment
 *
 */
void Irrig_and_Turb(MSEBoxModel *bm, double *Irrig, double *Turb) {
	double Turb_enhance, Irr_enhance;

	/* Must multiply by number in the day as only working it out once now not every iteration */
	Turb_enhance = 1.0 + K_MAX_TUR * Util_Mich_Ment(Turbatn_contribs * bm->dt, K_TUR);
	*Turb = (*Turb + Turb_enhance) / 2.0;

	Irr_enhance = K_MIN_IRR + K_MAX_IRR * Util_Mich_Ment(Irrig_contribs * bm->dt, K_IRR);
	*Irrig = (*Irrig + Irr_enhance) / 2.0;

	return;
}
/**
 *
 * \brief Function used to update the catch and discard arrays.
 *
 * The divideValue and multValue values are used in the epibenthic layer.
 *
 * This is used for both invertebrates and vertebrates. For inverts the biomass is passed in SN, RN is set to 0 and NUMS will be 1.0.
 *
 *
 *
 */
void Calculate_Catch(MSEBoxModel *bm, BoxLayerValues *boxLayerInfo, FILE *llogfp, int guild, int cohort, double SN, double RN, double NUMS,
		double propSediment, double propWater) {
	double biomassToDL, numDead, numInvDead;
	int nf, stock_id;

	//FunctGroupArray[guild].dead[cohort] = 0.0;  This removed as was zeroing out density dependent fishing discard effects

	/* Calculate the biomass that is lost due to fishing - either though catch (stored in the bm->FishingResults array), and the amount that is discarded dead
	 * that will be added to the DL biomass pool
	 */
	if(Harvest_Do_Fishing_And_ByCatch(bm, llogfp, guild, cohort, SN, RN, NUMS, bm->FishingResults, &numDead, &biomassToDL)){

		if (FunctGroupArray[guild].isVertebrate == TRUE) {
			FunctGroupArray[guild].dead[cohort] += numDead;  // This used to be in the loop, but to no erroneous effect as it is an = not a +=
			// Invertebrates dealt with explicitly below using FishingResults call
            // Also now += as should already be zeroed elsewhere (and if not its because not used yet)
            
            /**
            if (guild == 33) {
                fprintf(llogfp,"Time: %e %s has dead: %e and numDead from fishing: %e\n", bm->dayt, FunctGroupArray[guild].groupCode, FunctGroupArray[guild].dead[cohort], numDead);
            }
             **/
		}
		
		boxLayerInfo->DetritusProd[WC][DCdet_id] += biomassToDL;   // This differs to the original code, which added it with each fishery but that was a bug

		stock_id = bm->group_stock[guild][bm->current_box][bm->current_layer];

		bm->calcTrackedMort[guild][cohort][stock_id][ongoingF_id] += (numDead * bm->dt);

		for (nf = 0; nf < bm->K_num_fisheries; nf++) {
			if (!FisheryArray[nf].isRec) {
				bm->Catch[bm->current_box][guild][nf][bm->current_layer] += bm->FishingResults[nf][Catch_result_id];
				bm->FCcaughttemp[guild][nf][cohort] += bm->FishingResults[nf][CatchAtAge_result_id];
				if (FunctGroupArray[guild].isVertebrate == TRUE) {
					FunctGroupArray[guild].SizeNumCaught[cohort][nf][bm->current_box] += bm->FishingResults[nf][CatchAtAge_result_id];
					FunctGroupArray[guild].SizeNumDiscard[cohort][nf][bm->current_box] += bm->FishingResults[nf][DiscardsAtAge_result_id];
					FunctGroupArray[guild].SizeCaught[cohort][nf][bm->current_box] += bm->FishingResults[nf][Catch_result_id];
				}
			} else {
				bm->RecCatch[bm->current_box][guild][nf] += bm->FishingResults[nf][Catch_result_id];
			}

			if (FunctGroupArray[guild].isVertebrate == TRUE) {
				bm->FCdiscard[guild][cohort] += bm->FishingResults[nf][DiscardsAtAge_result_id];
			} else {
				FunctGroupArray[guild].dead[cohort] += (bm->FishingResults[nf][Catch_result_id] * propWater / (bm->cell_vol * bm->dt));
				FunctGroupArray[guild].dead[cohort] += (bm->FishingResults[nf][Discards_result_id] * propWater / (bm->cell_vol * bm->dt));
				numInvDead = (bm->FishingResults[nf][Catch_result_id] * propWater / (bm->cell_vol * bm->dt)) + (bm->FishingResults[nf][Discards_result_id] * propWater / (bm->cell_vol * bm->dt));
				bm->calcTrackedMort[guild][cohort][stock_id][ongoingF_id] += (numInvDead * bm->dt);

				//if(bm->which_check == guild)
				//	fprintf(llogfp,"%s-%d dead: %e due to catch\n", FunctGroupArray[guild].groupCode, cohort, FunctGroupArray[guild].dead[cohort]);

				if ((FunctGroupArray[guild].groupType == CORAL) || (FunctGroupArray[guild].groupType == SPONGE)) {
					Destroy_Rugosity(bm, boxLayerInfo, llogfp, guild, cohort, nf, numInvDead);
				}

			}

			/**/
			if ((bm->debug == debug_biology_process) && (bm->dayt >= bm->checkstart) && (bm->dayt < bm->checkstop)) {
				if(bm->FishingResults[nf][Catch_result_id] > 0){
					fprintf(llogfp, "Group %s:%d - bm->FishingResults[%s][Catch_result_id] = %.20e\n", FunctGroupArray[guild].groupCode, cohort, FisheryArray[nf].fisheryCode, bm->FishingResults[nf][Catch_result_id]);
					fprintf(llogfp, "bm->FishingResults[%s][Discards_result_id] = %e\n", FisheryArray[nf].fisheryCode, bm->FishingResults[nf][Discards_result_id]);
				}
			}
			/**/
		}
	}
	if (FunctGroupArray[guild].isVertebrate == FALSE) {
		boxLayerInfo->DetritusProdGlobal[WC][WC][DLdet_id] += biomassToDL / propSediment;

		if (FunctGroupArray[guild].groupAgeType == BIOMASS)
			FunctGroupArray[guild].deadGlobal[cohort] = FunctGroupArray[guild].dead[cohort];
	}
}

/**
 * \brief Get the area habitat of the given species/cohort.
 *
 *
 */
double Get_Species_Area_Hab(MSEBoxModel *bm, int guild, int cohort, BoxLayerValues *boxLayerInfo) {
	double sp_likeREEF, sp_likeFLAT, sp_likeSOFT, sp_likeCANYON;
	double area_hab, sp_like;
    double compete_space = 0.0;
    double remaining_space = 0.0;
    int sp, do_competition;
    int stage = FunctGroupArray[guild].cohort_stage[cohort];
    
	sp_likeREEF = bm->HABITATlike[guild][stage][bm->REEFcover_id];
	sp_likeFLAT = bm->HABITATlike[guild][stage][bm->FLATcover_id];
	sp_likeSOFT = bm->HABITATlike[guild][stage][bm->SOFTcover_id];
    sp_likeCANYON = bm->HABITATlike[guild][stage][bm->CANYONcover_id];

	area_hab = (sp_likeREEF * area_reef * BED_scale[reef_id]) + (sp_likeFLAT * area_flat * BED_scale[flat_id]) + (sp_likeSOFT * area_soft * BED_scale[soft_id]);

    /*
     fprintf(bm->logFile,"Time: %e %s box%d-%d physical area_hab: %e\n",
            bm->dayt, FunctGroupArray[guild].groupCode, bm->current_box, bm->current_layer, area_hab);
    */
    
    if((bm->debug == debug_biology_process) && (bm->dayt >= bm->checkstart) && (bm->dayt < bm->checkstop)){
    	fprintf(bm->logFile, "Group %s cohort %d area_hab = %e\n", FunctGroupArray[guild].groupCode, cohort, area_hab);
    	fprintf(bm->logFile, "sp_likeREEF - %e, sp_likeFLAT = %e, sp_likeSOFT= %e\n",sp_likeREEF, sp_likeFLAT, sp_likeSOFT);
    }
    if(area_hab > 1.0)   // Check needed as sp_like now no longer binary
    	area_hab = 1.0;
    
    if ((FunctGroupArray[guild].groupType == CORAL) || (FunctGroupArray[guild].groupType == SPONGE))  // Don't do any more for corals as they are treated explicitly in Coral_Space_Competition()
        return area_hab;
    
    // Now see how much of that potential space is actually open for settlement, colonisation or growth
    if(bm->flag_competing_epiff || bm->flag_invert_biohab) {
        do_competition = 0;
        if((FunctGroupArray[guild].groupType == PHYTOBEN) ||
              (FunctGroupArray[guild].groupType == SED_EP_FF) ||
              (FunctGroupArray[guild].groupType == SEAGRASS) ||
              (FunctGroupArray[guild].groupType == TURF))
            do_competition = 1;
        for(sp = 0; sp < bm->K_num_tot_sp; sp++){
            if (FunctGroupArray[sp].isCover == TRUE) {
                if (do_competition) {
                    if((FunctGroupArray[sp].groupType == PHYTOBEN) ||
                       (FunctGroupArray[sp].groupType == SED_EP_FF) ||
                       (FunctGroupArray[sp].groupType == CORAL) ||
                       (FunctGroupArray[sp].groupType == SPONGE) ||
                       (FunctGroupArray[sp].groupType == SEAGRASS) ||
                       (FunctGroupArray[sp].groupType == TURF)) {
                        compete_space += bm->coveramt[FunctGroupArray[sp].coverID][bm->current_box];
                        
                        /* fprintf(bm->logFile,"Time: %e %s box%d-%d compete with %s cover %e compete_space: %e\n",
                                bm->dayt, FunctGroupArray[guild].groupCode, bm->current_box, bm->current_layer,
                                FunctGroupArray[sp].groupCode, bm->coveramt[FunctGroupArray[sp].coverID][bm->current_box], compete_space);
                         */
                        
                    }
                }
                if (bm->flag_invert_biohab) {
                    sp_like = bm->HABITATlike[guild][stage][FunctGroupArray[sp].coverID];
                    
                    if (sp_like > 0.0) {
                        area_hab += (sp_like * bm->coveramt[FunctGroupArray[sp].coverID][bm->current_box]);
                        
                        /*
                            fprintf(bm->logFile,"Time: %e %s box%d-%d physical area_hab: %e as %s has cover %e and sp_like: %e \n",
                                bm->dayt, FunctGroupArray[guild].groupCode, bm->current_box, bm->current_layer, area_hab,
                                FunctGroupArray[sp].groupCode, bm->coveramt[FunctGroupArray[sp].coverID][bm->current_box], sp_like);
                         */
                        
                    }
                }
            }
        }
        // Allow for canyons
        if (bm->flag_invert_biohab)
            area_hab *= (1.0 + sp_likeCANYON * bm->boxes[bm->current_box].canyon);

        /*
            fprintf(bm->logFile,"Time: %e %s box%d-%d physical area_hab: %e after canyons\n",
                bm->dayt, FunctGroupArray[guild].groupCode, bm->current_box, bm->current_layer, area_hab);
         */
        
        // Adjust the area_hab down to reflect the remaining open space
        remaining_space = bm->max_available_habitat - compete_space;
        if ( remaining_space < 0.0 )
            remaining_space = 0.0;
        if ( area_hab > remaining_space )
            area_hab = remaining_space;

        /*
            fprintf(bm->logFile,"Time: %e %s box%d-%d physical area_hab: %e remaining_space: %e compete_space: %e\n",
                bm->dayt, FunctGroupArray[guild].groupCode, bm->current_box, bm->current_layer, area_hab, remaining_space, compete_space);
         */
    }

	return area_hab;
}

/**
 *	\brief Pelagic Bacteria partitioning
 */
void Calculate_PelagicBact_Scale(MSEBoxModel *bm, BoxLayerValues *boxLayerInfo) {
	double PB, DR, DL;
	int flag_sp = (int) (FunctGroupArray[pelagicBactIndex].speciesParams[flag_id]);


	if (flag_sp) {

		DL = boxLayerInfo->localWCTracers[FunctGroupArray[LabDetIndex].totNTracers[0]];
		DR = boxLayerInfo->localWCTracers[FunctGroupArray[RefDetIndex].totNTracers[0]];
		PB = boxLayerInfo->localWCTracers[FunctGroupArray[pelagicBactIndex].totNTracers[0]];

		boxLayerInfo->PB_DL = PB * XPB_DL * DL / (XPB_DL * DL + XPB_DR * DR + small_num);
		boxLayerInfo->PB_DR = PB * XPB_DR * DR / (XPB_DL * DL + XPB_DR * DR + small_num);
	} else {
		boxLayerInfo->PB_DL = 0.0;
		boxLayerInfo->PB_DR = 0.0;
	}
}
/**
 *	\brief Sediment Bacteria partitioning
 */
void Calculate_SedBact_Scale(MSEBoxModel *bm, HABITAT_TYPES habitatType, BoxLayerValues *boxLayerInfo) {
	double BB, O2, DLsed, DRsed, hO_BB;
	int flag_sp = (int) (FunctGroupArray[SedBactIndex].speciesParams[flag_id]);

	if (habitatType == SED) {
		O2 = boxLayerInfo->localSEDTracers[Oxygen_i];
		DLsed = 1.0;
		DRsed = 1.0;
	} else {
		O2 = boxLayerInfo->localWCTracers[Oxygen_i];
		DLsed = boxLayerInfo->localSEDTracers[FunctGroupArray[LabDetIndex].totNTracers[0]];
		DRsed = boxLayerInfo->localSEDTracers[FunctGroupArray[RefDetIndex].totNTracers[0]];
	}
	if (flag_sp) {
		BB = boxLayerInfo->localSEDTracers[FunctGroupArray[SedBactIndex].totNTracers[0]];

		hO_BB = Oxygen(1, O2, 0.0, 0.0, FunctGroupArray[SedBactIndex].speciesParams[mD_id]);
		boxLayerInfo->BB_DL = BB * Bact_stim * XBB_DL * hO_BB / (Bact_stim * XBB_DL * DLsed * hO_BB + Bact_stim * XBB_DR * DRsed * hO_BB + small_num);
		boxLayerInfo->BB_DR = BB * Bact_stim * XBB_DR * hO_BB / (Bact_stim * XBB_DL * DLsed * hO_BB + Bact_stim * XBB_DR * DRsed * hO_BB + small_num);
	} else {
		boxLayerInfo->BB_DL = 0.0;
		boxLayerInfo->BB_DR = 0.0;
	}
}

/**
 * \brief Update the tracked mortality record after Eat()
 *
 *
 */
void UpdateTrackedMort(MSEBoxModel *bm, FILE *llogfp, int guildcase, int cohort, HABITAT_TYPES level_id, HABITAT_TYPES habitat, int prey, int prey_chrt,
		BoxLayerValues *boxLayerInfo, double scalar, int global_contrib) {
	double biomass_correction, step1;
    int preystock, stock_id, this_layer;
    //int sn, rn, den;
	double scalar2 = (double)(global_contrib);
	int isGlobal = (FunctGroupArray[guildcase].diagTol == 2 && it_count == 1);
	/* Replicate old bec_dev results on existing models. We will remove this asap! */
    double KUP_SN, KLP_SN, Kmax_coefft, rel_size, sizeScalar = 1.0, SN;
    // double pred_bio;
	int test = 1;
	double mortality_scalar;
	if(GRAZEinfo[prey][prey_chrt][habitat] == 0)
		return;

	/* Update the invertebrate eaten values  - the vertebrates have already been handled in Partition_Grazed_Fish. */
	preystock = 0;
    stock_id = 0;

	/* If there is a scalar to apply then grab it now - remember this is the scaler to apply to the prey. */
	if(bm->scale_all_mortality == TRUE){
		mortality_scalar = Ecology_Get_Mortality_Scalar(bm, prey, prey_chrt);
	}else{
		mortality_scalar = 1.0;
	}

    //if (prey == bm->which_check)
    //    fprintf(llogfp,"Time: %e box%d-%d doing %s\n", bm->dayt, bm->current_box, bm->current_layer, FunctGroupArray[prey].groupCode);

	preystock = bm->group_stock[prey][bm->current_box][bm->current_layer];
	
	// Need this for the diet data

	stock_id = bm->group_stock[guildcase][bm->current_box][bm->current_layer];
	
	step1 = 0;
	if(FunctGroupArray[prey].groupAgeType == AGE_STRUCTURED){
		/* Sort out numbers eaten for age structured (e.g. vertebrate) groups */
		step1 = GRAZEinfo[prey][prey_chrt][habitat] * bm->cell_vol / (VERTinfo[prey][prey_chrt][SN_id] + VERTinfo[prey][prey_chrt][RN_id] + small_num);

/*
 * This very ugly code has been added to help people that were using the old bec_dev code to migrate to the new trunk code.
 * Turning this on will hopefully result in very similar results.
 */
		if(bm->flag_replicated_old == TRUE){

			/**
			 * The bec-dev code was checking the UseHardFeedingWindow flag in the Avail_Fish function only,
             * it was not checking again in Partition_Grazed_Fish().
			 * If UseHardFeedingWindow was set to 0 then the sizeScalar is being used in the Avail_Fish code,
             * but not in the Partition_Grazed_Fish code. So the amount of fish eaten was being scaled incorrectly.
			 *
			 * As we can't change the bec_dev version of the code we need to replicate these
             * values here - so we divide the step1 value by the sizeScalar value and we get the same
			 * value as the bec_dev code. NOT good but it works.
			 *
			 *
			 */
			if(!bm->UseHardFeedingWindow){
				if(FunctGroupArray[guildcase].isVertebrate == TRUE){
					SN = VERTinfo[guildcase][cohort][SN_id];
				} else {
					SN = FunctGroupArray[guildcase].sn[cohort];
				}

				KUP_SN = FunctGroupArray[guildcase].speciesParams[KUP_id] * SN;
				KLP_SN = FunctGroupArray[guildcase].speciesParams[KLP_id] * SN;
				Kmax_coefft = FunctGroupArray[guildcase].speciesParams[Kmax_coefft_id];
				rel_size = VERTinfo[prey][prey_chrt][SN_id] / ((SN * KUP_SN) + small_num);
				sizeScalar = rel_size * exp(Kmax_coefft * (1.0 - rel_size));
				if(sizeScalar > 1.0)
					sizeScalar = 1.0;
				if(sizeScalar < 0.0)
					sizeScalar = 0.0;
				step1 = step1 / sizeScalar;


				/* The bec_dev code was checking the UseHardFeedingWindow flag in the avail_fish function,
                 * but not in the Partition_Grazed_Fish function.
				 * So atlantis was saying some groups could be eaten and added them to avail_fish but,
                 * then when it was time to divy up the amount of fish eaten into the actual fish groups
                 * to set the preyEaten values the code was saying thet were not in the correct size range.
				 * The UseHardFeedingWindow flag should be checked in both Avail_Fish and Partition_Grazed_Fish
                 * to get the same results as the trunk code.
				 * This will not impact on models where the UseHardFeedingWindow flag is set to 1.
				 */
				if(FunctGroupArray[prey].isVertebrate == TRUE){
					if (VERTinfo[prey][prey_chrt][SN_id] >= KLP_SN &&  VERTinfo[prey][prey_chrt][SN_id] <= KUP_SN) {
						test = 1;
					}else{
						test = 0;
					}
				}
			}

		}

		if(test){
			FunctGroupArray[prey].preyEaten[prey_chrt][habitat] += (mortality_scalar * step1 * scalar);
			FunctGroupArray[prey].preyEatenGlobal[prey_chrt][level_id][habitat] += (mortality_scalar * step1 * scalar2);
		}
	} else {
		FunctGroupArray[prey].preyEaten[prey_chrt][habitat] += (mortality_scalar * GRAZEinfo[prey][prey_chrt][habitat] * scalar);
		FunctGroupArray[prey].preyEatenGlobal[prey_chrt][level_id][habitat] += (mortality_scalar * GRAZEinfo[prey][prey_chrt][habitat] * scalar2);
	}

	/**
	if ((FunctGroupArray[guildcase].groupAgeType == AGE_STRUCTURED_BIOMASS) && (bm->current_box == 1)){
		fprintf(llogfp,"Time: %e box%d-%d guildcase %s-%d eating %s-%d habitat: %d preyEaten: %Le preyEatenGlobal: %Le grazeinfo: %.20e step1: %.20e scalar: %.20e\n", bm->dayt, bm->current_box, bm->current_layer, FunctGroupArray[guildcase].groupCode, cohort, FunctGroupArray[prey].groupCode, prey_chrt, habitat, FunctGroupArray[prey].preyEaten[prey_chrt][habitat], FunctGroupArray[prey].preyEatenGlobal[prey_chrt][level_id][habitat], GRAZEinfo[prey][prey_chrt][habitat], step1, scalar);
	}
	**/

	if(bm->track_atomic_ratio == TRUE){
		if(FunctGroupArray[guildcase].habitatType == EPIFAUNA) {
			Loose_Element_From_Prey(bm, boxLayerInfo, habitat, guildcase, 0, GRAZEinfo[prey][prey_chrt][habitat], EPIFAUNA, isGlobal);
		}else{
			if (FunctGroupArray[guildcase].groupType == CEP)
				/* Only account for loss in element in prey - gain in predator is handled when we calculate the growth value. */
				Loose_Element_From_Prey(bm, boxLayerInfo, habitat, guildcase, 0, (GRAZEinfo[prey][prey_chrt][habitat] * scalar), WC, isGlobal);
			else
				Loose_Element_From_Prey(bm, boxLayerInfo, habitat, guildcase, 0, (GRAZEinfo[prey][prey_chrt][habitat] * scalar), level_id, isGlobal);
		}
	}

	if(bm->track_contaminants){
        
        //fprintf(llogfp, "Time: %e box %d-%d %s-%d eaten by %s-%d habitat %d GRAZEinfo: %e scalar: %e EATINGinfo: %e\n", bm->dayt, bm->current_box, bm->current_layer, FunctGroupArray[prey].groupCode, prey_chrt, FunctGroupArray[guildcase].groupCode, cohort, habitat, GRAZEinfo[prey][prey_chrt][habitat], scalar, EATINGinfo[prey][prey_chrt][habitat]);
        
        if(FunctGroupArray[guildcase].isOncePerDt == TRUE) {
            
            Group_Transfer_Contaminant(bm, boxLayerInfo, habitat, habitat, guildcase, cohort, prey, prey_chrt, (GRAZEinfo[prey][prey_chrt][habitat] * scalar), VERTinfo, EATINGinfo[prey][prey_chrt][habitat], bm->dtsz_stored, 1, 31);
        } else {
            
            Group_Transfer_Contaminant(bm, boxLayerInfo, habitat, habitat, guildcase, cohort, prey, prey_chrt, (GRAZEinfo[prey][prey_chrt][habitat] * scalar), VERTinfo, EATINGinfo[prey][prey_chrt][habitat], bm->dt, 1, 32);
        }
	}

	biomass_correction = Get_Biomass_Correction(bm, prey, level_id);

	bm->calcMnumPerPred[prey][guildcase][current_id] += (mortality_scalar * GRAZEinfo[prey][prey_chrt][habitat] * biomass_correction);
	bm->calcTrackedMort[prey][prey_chrt][preystock][ongoingM2_id] += (mortality_scalar * GRAZEinfo[prey][prey_chrt][habitat] * biomass_correction * FunctGroupArray[prey].speciesParams[Mdt_id]);
	bm->calcTrackedPredMort[prey][prey_chrt][preystock][guildcase][ongoing_id] += (mortality_scalar * GRAZEinfo[prey][prey_chrt][habitat] * biomass_correction * FunctGroupArray[prey].speciesParams[Mdt_id]);
	DIET_check[guildcase][cohort][stock_id][prey][ongoing_id] += (mortality_scalar * GRAZEinfo[prey][prey_chrt][habitat] * biomass_correction * FunctGroupArray[guildcase].speciesParams[Mdt_id]);
    
    if(bm->flagdietcheck) {
        if (habitat == SED) {
            this_layer = bm->current_layer + bm->wcnz;
        } else {
            this_layer = bm->current_layer;
        }

        bm->totDiet[bm->current_box][this_layer][guildcase][cohort][prey] += (mortality_scalar * GRAZEinfo[prey][prey_chrt][habitat] * biomass_correction * FunctGroupArray[guildcase].speciesParams[Mdt_id]);
        
        
        /*
        if (guildcase == 2) {
            sn = FunctGroupArray[guildcase].structNTracers[cohort];
            rn = FunctGroupArray[guildcase].resNTracers[cohort];
            den = FunctGroupArray[guildcase].NumsTracers[cohort];

            pred_bio = (bm->boxes[bm->current_box].tr[bm->current_layer][sn] + bm->boxes[bm->current_box].tr[bm->current_layer][rn]) * bm->boxes[bm->current_box].tr[bm->current_layer][den] * bm->X_CN * mg_2_tonne;

            fprintf(llogfp,"Time: %e box%d-%d %s-%d eating %s totDiet: %e vs %e (ratio: %e)\n", bm->dayt, bm->current_box, this_layer, FunctGroupArray[guildcase].groupCode, cohort, FunctGroupArray[prey].groupCode, bm->totDiet[bm->current_box][this_layer][guildcase][cohort][prey] * mg_2_tonne * bm->X_CN, pred_bio, (bm->totDiet[bm->current_box][this_layer][guildcase][cohort][prey] * mg_2_tonne * bm->X_CN) / pred_bio);
            
        }
        */
    }
    
    /**
     if (!prey && (!prey_chrt) && (FunctGroupArray[guildcase].groupAgeType == AGE_STRUCTURED) && bm->newmonth) {
         fprintf(llogfp,"Time: %e box%d-%d guildcase %s-%d eating %s-%d habitat: %d preyEaten: %Le preyEatenGlobal: %Le grazeinfo: %.20e step1: %.20e scalar: %.20e\n",bm->dayt, bm->current_box, bm->current_layer, FunctGroupArray[guildcase].groupCode, cohort, FunctGroupArray[prey].groupCode, prey_chrt,habitat, FunctGroupArray[prey].preyEaten[prey_chrt][habitat], FunctGroupArray[prey].preyEatenGlobal[prey_chrt][level_id][habitat], GRAZEinfo[prey][prey_chrt][habitat], step1, scalar);
    }
    **/

    
	return;
}

/**
 * \brief constructing prey info arrays
 *
 */
void Construct_Prey_Info(MSEBoxModel *bm, FILE *llogfp, BoxLayerValues *boxLayerInfo, int habitat_type)
{
	int guild, cohort;

	for (guild = 0; guild < bm->K_num_tot_sp; guild++) {
		if(FunctGroupArray[guild].speciesParams[flag_id]) {
			switch (FunctGroupArray[guild].groupAgeType) {
			case AGE_STRUCTURED:
				/* Nothing to do here - it will be done as assessed in Calculate_PreyAvail() */
				break;
			case AGE_STRUCTURED_BIOMASS:
				for (cohort = 0; cohort < FunctGroupArray[guild].numCohortsXnumGenes; cohort++) {
					PREYinfo[guild][cohort][habitat_type] = boxLayerInfo->localWCTracers[FunctGroupArray[guild].totNTracers[cohort]] * FunctGroupArray[guild].habitatCoeffs[habitat_type];
				}
				break;
			case BIOMASS:
				if ((FunctGroupArray[guild].groupType != LAB_DET && FunctGroupArray[guild].groupType != REF_DET)) {
					PREYinfo[guild][0][habitat_type] = boxLayerInfo->localWCTracers[FunctGroupArray[guild].totNTracers[0]] * FunctGroupArray[guild].habitatCoeffs[habitat_type];
				}
				break;
			}
            
            if(bm->ice_on) {
                if (FunctGroupArray[guild].habitatCoeffs[MIXED] > 0 ) {
                    if ( habitat_type == WC) {
                        switch (FunctGroupArray[guild].groupAgeType) {
                        case AGE_STRUCTURED:
                            /* Nothing to do here - it will be done as assessed in Calculate_PreyAvail() */
                            break;
                        case AGE_STRUCTURED_BIOMASS:
                            for (cohort = 0; cohort < FunctGroupArray[guild].numCohortsXnumGenes; cohort++) {
                                PREYinfo[guild][cohort][habitat_type] = boxLayerInfo->localWCTracers[FunctGroupArray[guild].totNTracers[cohort]] * FunctGroupArray[guild].habitatCoeffs[MIXED];
                            }
                            break;
                        case BIOMASS:
                            if ((FunctGroupArray[guild].groupType != LAB_DET && FunctGroupArray[guild].groupType != REF_DET)) {
                                PREYinfo[guild][0][habitat_type] = boxLayerInfo->localWCTracers[FunctGroupArray[guild].totNTracers[0]] * FunctGroupArray[guild].habitatCoeffs[MIXED];
                            }
                            break;
                        }

                    }
                }
            }
		}
	}

	return;
}

/**
 * \brief Get mortality due to acidification
 *
 */
double Acidif_Mort(MSEBoxModel *bm, int species) {
    double ans = 0.0;
    double sp_pHmortstart = FunctGroupArray[species].speciesParams[pHmortstart_id];
    double sp_pHmortA = FunctGroupArray[species].speciesParams[pHmortA_id];
    double sp_pHmortB = FunctGroupArray[species].speciesParams[pHmortB_id];
    double sp_pHmortmid = FunctGroupArray[species].speciesParams[pHmortmid_id];
    double annualSurvivalNoAcid, scalarOfSurvival, annualSurvivalScaled, annualSurvivalDecrement, mortDueToAcidPerDay;
    int pHmortcase = bm->flagPHmortcase;

    if (current_PH < sp_pHmortstart ) {
        switch (pHmortcase) {
            case ph_no_mort:
                ans = 0.0;
                break;
            case ph_sigmoid_mort:
                sp_pHmortA /= 86400.0;  // Converting to per second as not corrected on read in due to overloading for the other case
                ans = sp_pHmortA / ( 1.0 + exp(-1.0 * sp_pHmortB * (sp_pHmortmid - current_PH)));
                break;
            case ph_McElhany_Bush_mort:
                annualSurvivalNoAcid = exp(-sp_pHmortB); // This is exp(-PB) if using Ecopath PB estimate
                scalarOfSurvival = 1.0 - sp_pHmortA * (sp_pHmortstart - current_PH); // Apply slope from Paul and shallin. Gives a value between 0 and 1.
                annualSurvivalScaled = annualSurvivalNoAcid * scalarOfSurvival;     // get new annual survival rate under acidification
                annualSurvivalDecrement =  (annualSurvivalNoAcid - annualSurvivalScaled);   // will be some small positive value, annual decline in survival.
                mortDueToAcidPerDay = -1.0 * (log(1.0 - annualSurvivalDecrement)) / 86400.0;  // convert to per second rate. Note log is natural log.
                ans = mortDueToAcidPerDay;
                break;
            default:
                quit("No such pH mortality case\n");
                break;
        }
    }
    
    return ans;
    
}

/**
 *	\brief This routine will calculate initial dynamic X_RS ratio for species and age groups.
 The midpoint of RN/SN curve is in days, so we need this_age parameter to get the age of individuals in days
 *
 */

void Create_Update_XRS(MSEBoxModel *bm,  FILE *llogfp) {
    int sp, chrt, sp_numGeneTypes, basecohort, this_age, sid, aid, nspawn;
    double tmp, last_spawn;
    
    for (sp = 0; sp < bm->K_num_tot_sp; sp++) {
        if (FunctGroupArray[sp].groupAgeType == AGE_STRUCTURED || FunctGroupArray[sp].groupAgeType == AGE_STRUCTURED_BIOMASS){
            sp_numGeneTypes = (int) (FunctGroupArray[sp].numGeneTypes);
            for (chrt = 0; chrt < FunctGroupArray[sp].numCohortsXnumGenes; chrt++) {
                basecohort = (int)floor(chrt / sp_numGeneTypes); // calculate the age in years, as the loop goes through all genotypes
                sid = EMBRYO[sp].next_spawn - 1;  // So looking at the previous event
                if (sid < 0)
                    sid = 0;
                if (bm->dayt < EMBRYO[sp].Spawn_Now[chrt][sid]) {
                    // When initialising everything so in the first year
                    nspawn = FunctGroupArray[sp].numSpawns - 1; // Shortest time since spawn (relevant if have multiple spawns per year)
                    last_spawn = FunctGroupArray[sp].spawnSpeciesParams[nspawn][Time_Spawn_id];
                } else {
                    last_spawn = EMBRYO[sp].Spawn_Now[chrt][sid];
                }
                aid = EMBRYO[sp].next_age - 1; // So looking at the previous event
                if (aid < 0)
                    aid = 0;
                if (bm->dayt < EMBRYO[sp].Age_Now[chrt][aid]) {
                    // When before first modelling ageing event - including on initialisation
                    this_age = basecohort * FunctGroupArray[sp].ageClassSize * 365.0 + (365.0 - last_spawn); // age in days since spawn as this is when they age up
                    if(this_age < 0.0)
                        quit("Something has gone wrong for %s-%d as this_age < 0 in Create_Update_XRS (last_spawn: %e)\n", FunctGroupArray[sp].groupCode, chrt, last_spawn);
                    
                } else {
                    this_age = basecohort * FunctGroupArray[sp].ageClassSize * 365.0 + (bm->dayt - EMBRYO[sp].Age_Now[chrt][aid]);
                }
                
                //tmp = FunctGroupArray[sp].speciesParams[RSslope_id] * (this_age - FunctGroupArray[sp].speciesParams[RSmid_id]);
                if (bm->flag_do_evolution && DNA[sp].trait_active[rsmid_trait_id]) {
                    tmp = FunctGroupArray[sp].speciesParams[RSslope_id] * (this_age - bm->evolTraitOriginalValues[sp][chrt][rsmid_trait_id]);
                } else {
                    tmp = FunctGroupArray[sp].speciesParams[RSslope_id] * (this_age - FunctGroupArray[sp].speciesParams[RSmid_id]);
                }
                if (tmp < (-1.0 * bm->XRS_cap)) 	// Added bounds to prevent numerical issues
                    tmp = (-1.0 * bm->XRS_cap);
                
                if (tmp > bm->XRS_cap)
                    tmp = bm->XRS_cap;  // Set XRS_cap to 20.0
                
                FunctGroupArray[sp].X_RS[chrt] = FunctGroupArray[sp].speciesParams[RSmax_id] * (exp(tmp)) / (1 + exp(tmp)) ;
                // TODO: - for now all genotypes in a cohort will have the same X_RS. At which point will they be updated? If RSmid evolves, it is probably stored in some other array?
                if (FunctGroupArray[sp].X_RS[chrt] < 0 )
                    quit("Check RSmid, RSmax and RSslope parameters, the X_RS ratio is negative for species %s\n", FunctGroupArray[sp].groupCode);
                
            }
        }
    }
    
    
    return;
}

