/**
 \file
 \brief C file for process subroutines dealing with coral model developed by Mariska and Beth 2013
 \ingroup atEcology

 File:	atcoral.c
 Author:	Beth Fulton
 Created:	2/10/2013
 Purpose:	C file for process dealing with coral model developed by Mariska and Beth 2013
 Revisions:
            11/01/2019  Beth Fulton - Added sponge processes

 **/

/*  Open library routines */
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "atecology.h"

/*************************************************************************//**

Logic of the model was taken from work by Jess Melbourne-Thomas with additional turns elaborated as described below
See Appendix of
Melbourne-Thomas, J., Johnson, C.R., Fung, T., Seymour, R.M., Ch�rubin, L.M., Arias-Gonz�lez, J.E. and Fulton, E.A., 2011.
Regional-scale scenario modeling for coral reefs: a decision support tool to inform management of a complex system.
Ecological Applications 21: 1380�1398.

 */


/**
 *	\brief Coral photosynthetic growth
 *
 *	While not representing zooxanethellae separately, the concepts were checked against the highly cell detailed model by Gustafsson 2013
 *
 */
void Grow_Coral_Symbionts(MSEBoxModel *bm, BoxLayerValues *boxLayerInfo, FILE *llogfp, int guild, int cohort, int lim_case, int micro_case, double sp_biom,
		double DIN, double NH, double NO, double Si, double Fe, double P, double PRatio, double C, double CRatio,
		double IRR, double mum, double E_sp, double mL_other, double SPmax, double FDL_SP, double area_hab, double *spUptakeNO,
		double *spUptakeSi, double *spUptakeFe, double *spUptakeP,  double *spUptakeC, double *sphN) {

	double hN_sp, hI_sp, sp_grow, scale_uptake, uptakeNH, uptakeNO, uptakeSi, uptakeFe, uptakeP = 0, uptakeC = 0;
	double tot_N, host_resp_N_for_symbiont, SP_zooxanth_mort, SP_polyp_mort, SPmortNH_zx, SPmortNH_polyp;
	double mL_SP = FunctGroupArray[guild].cohortSpeciesParams[cohort][mL_id] + mL_other;
	double mQ_SP = FunctGroupArray[guild].cohortSpeciesParams[cohort][mQ_id];
	double mE_SP = FunctGroupArray[guild].cohortSpeciesParams[cohort][mE_id];
	double FDM_SP = FunctGroupArray[guild].speciesParams[FDMort_id];
	double Host_Resp_Remin_SP = FunctGroupArray[guild].speciesParams[HostRemin_id];
	double prop_daylight;
	double pH_lim = 1.0;
    int sp_id = (int) (FunctGroupArray[guild].speciesParams[coralID_id]);
	double local_mum = max(1.0, CORALREEF[sp_id].GrowShift[bm->current_box]) * mum;
	double over_growth_rate = 0.0;
	int sp;
	double zooxanth_biom;
	double polyp_biom;
	double P0 = FunctGroupArray[guild].speciesParams[P_min_internal_id];
	double P1 = FunctGroupArray[guild].speciesParams[P_max_internal_id];
    double mS_SP = Acidif_Mort(bm, guild);
    int is_sponge = 0;
    
    if ( FunctGroupArray[guild].groupType == SPONGE )
        is_sponge = 1;
    
    if (current_depth <= (-1.0 * FunctGroupArray[guild].speciesParams[threshdepth_id]))
        local_mum *= FunctGroupArray[guild].speciesParams[depmum_scalar_id];

    /* Find macralgal over_growth rate */
	for(sp = 0; sp < bm->K_num_tot_sp; sp++){
		if(FunctGroupArray[sp].groupType == PHYTOBEN) {
            if ( is_sponge )
                over_growth_rate += (boxLayerInfo->localEPITracers[FunctGroupArray[sp].CoverTracer] * FunctGroupArray[sp].speciesParams[sponge_overgrow_id]);
            else
                over_growth_rate += (boxLayerInfo->localEPITracers[FunctGroupArray[sp].CoverTracer] * FunctGroupArray[sp].speciesParams[coral_overgrow_id]);
		}
	}

	/* Symbiont biomass */
	zooxanth_biom = sp_biom * (FunctGroupArray[guild].speciesParams[prop_zooxanth_id] * CORALREEF[sp_id].PropUnBleached[cohort][bm->current_box]);
	polyp_biom = sp_biom - zooxanth_biom;

	/* Do mortality - zooxanthellae and polyp (ignoring bleaching as done explicitly below)
	 * For the purposes of tracking fate to detritus and nutrients need to keep polyp and zooxanthellae
	 * separate as some of the host is recycled */
	SP_zooxanth_mort = (mS_SP + mL_SP + mE_SP + mQ_SP * zooxanth_biom + over_growth_rate) * zooxanth_biom;
	SP_polyp_mort = (mL_SP + mE_SP + mQ_SP * polyp_biom + over_growth_rate) * polyp_biom;
	FunctGroupArray[guild].mortality[cohort] = SP_polyp_mort + SP_zooxanth_mort;   // As zooxanthellae stored inside the host tissue slot not differentiated
	bm->calcMLinearMort[guild][current_id] += (mL_SP * sp_biom);
	bm->calcELinearMort[guild][current_id] += ((mS_SP + mE_SP) * sp_biom);
	bm->calcMQuadMort[guild][current_id] += (mQ_SP * sp_biom * sp_biom);
	bm->calcTrackedMort[guild][cohort][0][ongoingM1_id] += (FunctGroupArray[guild].mortality[cohort] * FunctGroupArray[guild].speciesParams[Mdt_id]);

	if(bm->track_contaminants)
		Contaminant_Record_Death(bm, guild, cohort, sp_biom);
    
    /*
    fprintf(llogfp,"Time: %e box%d-%d %s has SP_zooxanth_mort: %e mL_SP: %e mE_SP: %e mQ_SP: %e zooxanth_biom: %e over_growth_rate: %e SP_polyp_mort: %e polyp_biom: %e totmort: %e\n",
            bm->dayt, bm->current_box, bm->current_layer, FunctGroupArray[guild].groupCode, SP_zooxanth_mort, mL_SP, mE_SP, mQ_SP, zooxanth_biom, over_growth_rate, SP_polyp_mort, polyp_biom, (SP_polyp_mort + SP_zooxanth_mort));
   
     */
    
	/* Convert into useful units for handing to nutrient and detritus pools */
	SPmortNH_zx = SP_zooxanth_mort / wcLayerThick;
	SP_zooxanth_mort /= smLayerThick;
	SPmortNH_polyp = SP_polyp_mort / wcLayerThick;
	SP_polyp_mort /= smLayerThick;

	/* Calculate limitation factors */
	host_resp_N_for_symbiont = SPmortNH_polyp * (1.0 - FDM_SP) * Host_Resp_Remin_SP;
	tot_N = DIN + host_resp_N_for_symbiont;
	hN_sp = Nutrient_Lim(bm, lim_case, micro_case, tot_N, Si, Fe, P, PRatio, P0, P1,
			FunctGroupArray[guild].speciesParams[KN_id], FunctGroupArray[guild].speciesParams[KS_id],
			FunctGroupArray[guild].speciesParams[KF_id], FunctGroupArray[guild].speciesParams[KP_id]);


	*sphN = hN_sp;

	prop_daylight = bm->boxes[bm->current_box].prop_light_time;
	hI_sp = Light_Lim(bm, bm->flaglight, IRR, FunctGroupArray[guild].speciesParams[KI_id], FunctGroupArray[guild].SP_IRR, prop_daylight, 0.0);

	/* TODO: Calculate pH limitation  - leave out for now as captured through changed growth rates
	k_pH = FunctGroupArray[guild].speciesParams[KpH_id];
	pH_lim = pH / (k_pH + pH);
	*/

	/* Calculate resulting growth of zooxanthellae - need to include area limitation here as host polyp will be area limited
	 * Note: Assume translocation of N and biomass to host cells rather than do translocation explicitly
	 * (so assume 90% of growth translocated, meaning zooxanthellae make up 30% of total biomass, which is lost if they bleach)
	 * Also assume that there is sufficient room in the host cell for any areal increase in zooxanthellae cells */
	sp_grow = zooxanth_biom * local_mum * hN_sp * hI_sp * pH_lim * min(1.0,max(0.0,(1.0 - sp_biom / (SPmax * area_hab + small_num))));

	/* Calculate nutrient uptake */
	uptakeNH = sp_grow * (NH / (FunctGroupArray[guild].speciesParams[KN_id] + NH)) * ((FunctGroupArray[guild].speciesParams[KN_id] + DIN) / DIN);
	uptakeNO = sp_grow * (NO / (FunctGroupArray[guild].speciesParams[KN_id] + DIN))
			* (FunctGroupArray[guild].speciesParams[KN_id]	/ (FunctGroupArray[guild].speciesParams[KN_id] + NH))
			* (1.0 + FunctGroupArray[guild].speciesParams[KN_id] / DIN);

	if (lim_case != one_nut_lim)
		uptakeSi = sp_grow * bm->X_SiN;
	else
		uptakeSi = 0.0;

	if (micro_case)
		uptakeFe = sp_grow * bm->X_FeN;
	else
		uptakeFe = 0.0;

	/* Cap nutrient uptake to what is available */
	scale_uptake = 1.0;
	if (uptakeNH > NH)
		scale_uptake = NH / uptakeNH;
	if (uptakeNO > NO)
		scale_uptake = min(scale_uptake, NO / uptakeNO);  // This used to say max
	if (uptakeSi > Si)
		scale_uptake = min(scale_uptake, Si / uptakeSi);  // This used to say max
	if ((Fe > bm->min_pool) && (uptakeFe > Fe)) // Only useful if MicroNut in play
		scale_uptake = min(scale_uptake, Fe / uptakeFe);  // This used to say max

	/* Assign uptake */
	FunctGroupArray[guild].uptakeNH[cohort] += uptakeNH * scale_uptake;   // As zooxanthellae stored inside the host tissue slot not differentiated

	if(bm->track_atomic_ratio == TRUE){

		/* Calculate P uptake */
		uptakeP = Calculate_Phosphorus_Uptake(bm, guild, sp_grow, PRatio);

		/* Now do carbon */
		uptakeC = Calculate_Carbon_Uptake(bm, guild, sp_grow, CRatio);
	}

	if(bm->ecotest > 1){
		if(!_finite(FunctGroupArray[guild].uptakeNH[cohort])){
			printf("group %s - uptakeNH = %e, scale_uptake = %e, DIN = %e, NH= %e, sp_grow= %e\n",
					FunctGroupArray[guild].groupCode, uptakeNH, scale_uptake, DIN, NH, sp_grow);
			quit("ERROR: uptakeNH is infinite.\n");
		}
	}
	*spUptakeNO = uptakeNO * scale_uptake;
	*spUptakeSi = uptakeSi * scale_uptake;
	*spUptakeFe = uptakeFe * scale_uptake;
	*spUptakeP = uptakeP * scale_uptake;
	*spUptakeC = uptakeC * scale_uptake;

	/* Make sure all diagnostics done */
	FunctGroupArray[guild].growth[cohort] = sp_grow;  /* Zooxanethellae only (host growth added below) - already in m-2 so no need to *smLayerThick
													      and zooxanthellae stored inside the host tissue slot not differentiated*/

	FunctGroupArray[guild].releaseNH[cohort] += (SPmortNH_polyp * (1.0 - FDM_SP) * (1.0 - Host_Resp_Remin_SP)) + SPmortNH_zx * (1.0 - FDM_SP);
	FunctGroupArray[guild].prodnDL[cohort] = SPmortNH_polyp * (1.0 - Host_Resp_Remin_SP) * FDM_SP * FDL_SP + SPmortNH_zx * FDM_SP * FDL_SP;
	FunctGroupArray[guild].prodnDR[cohort] = SPmortNH_polyp * (1.0 - Host_Resp_Remin_SP) * FDM_SP * (1.0 - FDL_SP) + SPmortNH_zx * FDM_SP * (1.0 - FDL_SP);

	return;
}


/**
 *    \brief Factors limiting growth of corals
 *     Originally in the Coral_Consumer_Activities but moved separately so effects both hetertrophic and autotrophic growth
 *
 */
void Coral_Limiting_Growth_Factors(MSEBoxModel *bm, FILE *llogfp, int guild, int cohort, double sed_level) {
    double current_calcification, step1, step2, step3, smother_effect;
    double calcif_Tconst = FunctGroupArray[guild].speciesParams[calcifTconst_id];
    double calcif_Tcoefft = FunctGroupArray[guild].speciesParams[calcifTcoefft_id];
    double calcifTopt = FunctGroupArray[guild].speciesParams[calcifTopt_id];
    double calcif_lambda = FunctGroupArray[guild].speciesParams[calcifLambda_id];
    double calcif_ref_baseline = FunctGroupArray[guild].speciesParams[calcifRefBaseline_id];
    double calcification_scalar = 1.0;

    //Check to see calcification effects (so can bring in acidification) *
    step1 = calcif_Tconst - ((calcif_Tcoefft * (H2Otemp - calcifTopt) * (H2Otemp - calcifTopt)) / calcifTopt);
    step2 = calcif_lambda * H2Otemp / calcifTopt;
    if(current_ARAG < 1.0) {
        step3 = small_num;
        if(bm->newmonth)
            warn("aragonite has dropped below 1.0 in box %d-%d\n", bm->current_box, bm->current_layer);
    } else {
        step3 = pow((current_ARAG - 1.0), step2);
    }
    
    current_calcification = step1 * step3;
    calcification_scalar = current_calcification / calcif_ref_baseline;
    
    // Check for sediment smothering - energy going to cleaning self instead *
    if(sed_level > 0.0) {
        step1 = -1.0 * bm->Ksmother_coefft * log(sed_level) - bm->Ksmother_const;
        smother_effect = 1.0 + step1;
        //smother_effect = -bm->Ksmother_coefft * log10(sed_level) + bm->Ksmother_const;   // Old relationship
    } else
        smother_effect = 1.0;
    
    
    if(smother_effect < 0.0)  // Sanity check
        smother_effect = 0.0;
    if(smother_effect > 1.0)
        smother_effect = 1.0;
    
    FunctGroupArray[guild].growth[cohort] *= calcification_scalar * smother_effect;
    
    if(!_finite(FunctGroupArray[guild].growth[cohort])){
        printf("calcification_scalar= %e, smother_effect= %e\n", calcification_scalar, smother_effect);
        quit("Coral_Limiting_Growth_Factors - Coral Group %s has infinite growth in box %d.\n", FunctGroupArray[guild].groupCode, bm->current_box);
    }

    return;
}

/**
 *	\brief Coral metabolic, consumption and waste processes for heterotrophic feeding of Corals. Also contains sponge growth - including Si limitation of heterotrophic sponges
 *
 */
void Coral_Consumer_Activities(MSEBoxModel *bm, HABITAT_TYPES habitatType, FILE *llogfp, int guild, int cohort, double SP, double SPmax, double IRR,
		double area_hab, double E_SP, double EDL_SP, double EDR_SP, double bact_DL, double bact_DR, double sedbact_DL, double sedbact_DR,
		double PB_scale, double BB_scale, double mL_other, double FDL_SP, double DL, double DR, double DLsed, double DRsed, double Si, double ***spGRAZEinfo) {

	double SPgrazePB, SPgrazeBB, SPgrazeDR, SPgrazeDL, SPprodnDET;
	double FDG_SP = FunctGroupArray[guild].speciesParams[FDG_id];
	double FDGDL_SP = FunctGroupArray[guild].speciesParams[FDGDL_id];
	double FDGDR_SP = FunctGroupArray[guild].speciesParams[FDGDR_id];
	double Feed_Light_Thresh = FunctGroupArray[guild].speciesParams[FeedLightThresh_id];
	double prop_light_feed = FunctGroupArray[guild].speciesParams[PropLightFeed_id];
	double prop_feeding = 1.0;
	double sp_GrazeFeed = 0.0;
    
    double Vmax_deltaSi = FunctGroupArray[guild].speciesParams[Vmax_deltaSi_id]; /* This and the below ones are for Si limitation */
    double Km_deltaSi = FunctGroupArray[guild].speciesParams[Km_deltaSi_id];
    double delta_Si = 1.0;

	char code[100];
	int habitat;

	if (FunctGroupArray[guild].groupAgeType == AGE_STRUCTURED_BIOMASS && cohort == 0)
		snprintf(code, sizeof(code), "j%s", FunctGroupArray[guild].groupCode);
	else
		snprintf(code, sizeof(code), "%s", FunctGroupArray[guild].groupCode);


    /*
	if (((bm->debug == debug_biology_process) && (bm->dayt >= bm->checkstart) && (bm->dayt < bm->checkstop))) {
		fprintf(llogfp,
				"Coral_Consumer_Activities, guild = %s, sp = %.20e, E_SP = %.20e, EDL_SP = %.20e, EDR_SP = %.20e, bact_DL = %.20e, bact_DR = %.20e, sedbact_DL = %.20e, sedbact_DR = %.20e, PB_scale = %.20e, BB_scale = %.20e, mL_other = %.20e, SPgrazeLive = %.20Le, FDL_SP = %.20e, DL = %.20e, DR = %.20e, DLsed = %.20e, DRsed = %.20e\n\n ",
				code, SP, E_SP, EDL_SP, EDR_SP, bact_DL, bact_DR, sedbact_DL, sedbact_DR, PB_scale, BB_scale, mL_other,
				FunctGroupArray[guild].GrazeLive[cohort], FDL_SP, DL, DR, DLsed, DRsed);
	}
     */
    
	SPgrazeDR = 0.0;
	SPgrazeDL = 0.0;

	for (habitat = WC; habitat <= EPIFAUNA; habitat++) {
		SPgrazeDR += spGRAZEinfo[RefDetIndex][0][habitat];
		SPgrazeDL += spGRAZEinfo[LabDetIndex][0][habitat];
	}

	/* Start with grazing on bacteria - in m-3 */
	SPgrazePB = (spGRAZEinfo[LabDetIndex][0][WC] * bact_DL / (DL + small_num) + spGRAZEinfo[RefDetIndex][0][WC] * bact_DR / (DR + small_num)) * PB_scale;

	SPgrazeBB = (spGRAZEinfo[LabDetIndex][0][SED] * sedbact_DL / (DLsed + small_num) + spGRAZEinfo[RefDetIndex][0][SED] * sedbact_DR / (DRsed + small_num))
			* BB_scale;

	FunctGroupArray[guild].GrazeLive[cohort] += SPgrazePB + SPgrazeBB;

	/* Handle grazing of aquaculture feed - unlikely to be an aquaculture species, but just in case */
	if(FunctGroupArray[guild].isCultured || FunctGroupArray[guild].isSupplemented)
		sp_GrazeFeed = spGRAZEinfo[AquacultFeedIndex][0][WC];
	else
		sp_GrazeFeed = 0.0;

	/* Do detritus production */
	SPprodnDET = ((double)FunctGroupArray[guild].GrazeLive[cohort] + sp_GrazeFeed) * (1.0 - E_SP) * FDG_SP + SPgrazeDR * (1.0 - EDR_SP) * FDGDR_SP
			+ SPgrazeDL * (1.0 - EDL_SP) * FDGDL_SP;

 	/* Return the final growth, waste etc values */
	spGRAZEinfo[pelagicBactIndex][0][WC] = SPgrazePB;
	spGRAZEinfo[SedBactIndex][0][SED] = SPgrazeBB;

	/* Growth - check to see if night as only really feed at night according to the texts so far
	 * - assume feeding at night and scale down if feeding in day time (as parameetrised).
	 * Note that space limitation has been taken into account in the scaling of mum for the Eat() routine */
	if(IRR > Feed_Light_Thresh)
		prop_feeding = prop_light_feed;
    
    /* Si limitation for sponge groups. The following should affect only heterotrophic sponges - which must be marked as isSiliconDep in the Groups.csv file.
     Phototrophic will be limited in photosynthesis calcualtions. Will need tests to see if this chokes them too much */
    if (FunctGroupArray[guild].isSiliconDependant) { /* Sponges should be the only Si-dependent group this is applied to */
        delta_Si = (Si * Vmax_deltaSi) / (Km_deltaSi + Si); /* following kinetics reported by Maldonado et al. 2011 Scientific Reports */
    }

	FunctGroupArray[guild].grazing[cohort] = (double)FunctGroupArray[guild].GrazeLive[cohort] + SPgrazeDR + SPgrazeDL + sp_GrazeFeed;
	FunctGroupArray[guild].growth[cohort] += (((E_SP * ((double)FunctGroupArray[guild].GrazeLive[cohort] + sp_GrazeFeed) + EDR_SP * SPgrazeDR + EDL_SP * SPgrazeDL)
			* prop_feeding * delta_Si) * smLayerThick);  // epibenthos need to set back to m-2


	if(!_finite(FunctGroupArray[guild].growth[cohort])){
		printf("prop_feeding = %e\n", prop_feeding);
		quit("Coral_Consumer_Activities - Coral Group %s has infinite growth in box %d.\n", FunctGroupArray[guild].groupCode, bm->current_box);
	}
		/**
	 Ammonia and detritus production by the host - other than by mortality (which is dealt with under the symbiont's growth so its available for remineralisation).
	 **/

	FunctGroupArray[guild].releaseNH[cohort] += ((double)FunctGroupArray[guild].GrazeLive[cohort] + sp_GrazeFeed) * (1.0 - E_SP) * (1.0 - FDG_SP)
			+ SPgrazeDR * (1.0 - EDR_SP) * (1.0 - FDGDR_SP) + SPgrazeDL * (1.0 - EDL_SP) * (1.0 - FDGDL_SP);

	FunctGroupArray[guild].prodnDL[cohort] += SPprodnDET * FDL_SP;
	FunctGroupArray[guild].prodnDR[cohort] += SPprodnDET * (1.0 - FDL_SP);

    /*
	if (((bm->debug == debug_biology_process) && (bm->dayt >= bm->checkstart) && (bm->dayt < bm->checkstop))) {
		fprintf(llogfp,
				"Time: %e, Coral_Consumer_Activities outcomes: guild = %s, SP = %.20e, SPgrazePB = %.20e, SPgrazeBB = %.20e, GrazeLive = %.20Le, mortality = %.20e, grazing = %.20e, growth = %.20e, releaseNH = %.20e, prodnDL = %.20e, prodnDR = %.20e\n",
				bm->dayt, code, SP, SPgrazePB, SPgrazeBB, FunctGroupArray[guild].GrazeLive[cohort],
				FunctGroupArray[guild].mortality[cohort], FunctGroupArray[guild].grazing[cohort],
				FunctGroupArray[guild].growth[cohort], FunctGroupArray[guild].releaseNH[cohort],
				FunctGroupArray[guild].prodnDL[cohort], FunctGroupArray[guild].prodnDR[cohort]);
	}
     */

	return;
}

/**
 *	\brief Coral bleaching
 *
 */
void Do_Bleaching(MSEBoxModel *bm, FILE *llogfp, int guild, int cohort, double sp_biom){
	int d;
    int time_since_last_shock = 0;
    int sp_id = (int) (FunctGroupArray[guild].speciesParams[coralID_id]);
    int recovery_periodA = (int)(FunctGroupArray[guild].speciesParams[bleach_periodA_id]);
	int recovery_periodB = (int)(FunctGroupArray[guild].speciesParams[bleach_periodB_id]);
    double timestep_correction = FunctGroupArray[guild].speciesParams[Mdt_id] / 86400;
	double amt_recovering, period_effect, prop_bleaching, lost_biom, dead_biom;
    double bleach_mort_rate = FunctGroupArray[guild].speciesParams[mBleach_id];
	double bleach_rate = FunctGroupArray[guild].speciesParams[bleaching_rate_id] * FunctGroupArray[guild].speciesParams[Mdt_id];
	double recovery_rate = FunctGroupArray[guild].speciesParams[bleach_recovery_rate_id];
	double temp_shift_rate = FunctGroupArray[guild].speciesParams[bleach_tempshift_id];
	double grow_shift_rate = FunctGroupArray[guild].speciesParams[bleach_growshift_id];
	double local_temp_thresh = FunctGroupArray[guild].speciesParams[bleach_temp_id] * CORALREEF[sp_id].TempShift[bm->current_box];
	double zooxanth_biom = sp_biom * (FunctGroupArray[guild].speciesParams[prop_zooxanth_id] * CORALREEF[sp_id].PropUnBleached[cohort][bm->current_box]);
	double polyp_biom = sp_biom - zooxanth_biom;
    double min_bleach_salt_sp = FunctGroupArray[guild].speciesParams[min_bleach_salt_id];
    double max_bleach_salt_sp = FunctGroupArray[guild].speciesParams[max_bleach_salt_id];
    //double min_bleach_temp_sp = FunctGroupArray[guild].speciesParams[min_bleach_temp_id];
    int do_DHW_now = 1;
    int sp_active = (int) (FunctGroupArray[guild].speciesParams[flagactive_id]);
    
    if (bm->dt < 86400) {  // Assumes 1 = day time, 0 = night time
        if (sp_active == 2) { /* No preference for night or day */
            if(!bm->flagday)
                do_DHW_now = 0;   // Don't do it at night - want it in the day time if have night and day
        } else if (sp_active != bm->flagday) {  /* Base it on when active */
            do_DHW_now = 0;
        }
    }
    
    if (cohort || (it_count > 1)) {
        // Only want it for the youngest - as then know done for all - and in the first iteration of the adaptive
        // timestep as don't need to do it for every age group as the answer is identical for all
        do_DHW_now = 0;
    }
    
	/* Look at past week of temperatures - if 1-2 deg C above threshold temperature then
	 * flag as "degree heating week", if get a month of those (i.e. 4 continuous weeks)
	 * then bleach with associated mortality and biomass (non-terminal) losses.
	 */

	/** Update the current temperature status **/
	if(do_DHW_now) {
        if (bleach_rate < 0.0) // Sanity check - needed here as used later in a couple of spots - for temperature and salinity so better than 2 tests there
            bleach_rate = 0.0;
        
        bleaching_has_occurred[guild] = 0; // Reinit
        
		//Update the sum by removing the first entry
		CORALREEF[sp_id].DHWsum[bm->current_box] -= CORALREEF[sp_id].DHWqueue[bm->current_box][0];  // so keeping the recorded value to whole days regardless of dt size;

		// Shuffle queue
		for (d=0; d<(bm->K_max_num_DHW - 1); d++){
			CORALREEF[sp_id].DHWqueue[bm->current_box][d] = CORALREEF[sp_id].DHWqueue[bm->current_box][d+1];
			if(CORALREEF[sp_id].DHWqueue[bm->current_box][d] > 0)
				time_since_last_shock = (bm->K_max_num_DHW - d);
		}

		// Add new entry to end of the queue
		d = bm->K_max_num_DHW - 1;
		if(H2Otemp > local_temp_thresh) {
			CORALREEF[sp_id].DHWqueue[bm->current_box][d] = H2Otemp - local_temp_thresh;
			time_since_last_shock = 0;
		} else {
			CORALREEF[sp_id].DHWqueue[bm->current_box][d] = 0;
		}

		//Update the sum by adding the new entry
		CORALREEF[sp_id].DHWsum[bm->current_box] += CORALREEF[sp_id].DHWqueue[bm->current_box][d];
	}
    
    /*
    if(bm->newweek) {
        fprintf(bm->logFile, "Time %e box %d, group %s-%d, it_count: %d do_DHW_now %d DHWsum: %e, DHW_thresh: %e H2Otemp: %e local_temp_thresh : %e\n", bm->dayt, bm->current_box, FunctGroupArray[guild].groupCode, cohort, it_count, do_DHW_now, CORALREEF[sp_id].DHWsum[bm->current_box], FunctGroupArray[guild].speciesParams[DHW_thresh_id], H2Otemp, local_temp_thresh);
    }
    */
    
	/* See if potential bleaching event triggered */
	if(CORALREEF[sp_id].DHWsum[bm->current_box] > FunctGroupArray[guild].speciesParams[DHW_thresh_id]) {

		/* Find proportion bleaching */
		period_effect = CORALREEF[sp_id].DHWsum[bm->current_box] - FunctGroupArray[guild].speciesParams[DHW_thresh_id]; // i.e. how much it exceeds the threshold by
		prop_bleaching = bleach_rate * (1.0 + period_effect);
        if(prop_bleaching < 0.0)
            prop_bleaching = 0.0;
        
		CORALREEF[sp_id].PropUnBleached[cohort][bm->current_box] -= prop_bleaching;
        if(CORALREEF[sp_id].PropUnBleached[cohort][bm->current_box] < 0.0)
            CORALREEF[sp_id].PropUnBleached[cohort][bm->current_box] = 0.0;
        
       // Sanity check - could be much greater > 1.0 from calculations if super high temperature
        if (prop_bleaching > 1.0)
            prop_bleaching = 1.0;
        
		/* Eject zooxanthellae from those bleaching */
		lost_biom = zooxanth_biom * prop_bleaching;

		/* Kill off some of the host colonies - killing living tissue not effecting rugosity */
		dead_biom = polyp_biom * prop_bleaching * bleach_mort_rate;

		/* Update diagnostics - so sum of effects of host cohort plus an zooxanthellae lost from within it */
		bm->calcMLinearMort[guild][current_id] += ((lost_biom + dead_biom) / FunctGroupArray[guild].speciesParams[Mdt_id]);  // As will be * dt at anotehr step
		FunctGroupArray[guild].mortality[cohort] += ((lost_biom + dead_biom) / FunctGroupArray[guild].speciesParams[Mdt_id]); // As will be * dt at anotehr step
		bm->calcTrackedMort[guild][cohort][0][ongoingM1_id] += (lost_biom + dead_biom); // As already * Mdt_id

        bleaching_has_occurred[guild] = 1;

		/**
		//if(bm->current_box == 8){

			fprintf(bm->logFile, "Time %e box %d, %s-%d, it_count = %d ", bm->dayt, bm->current_box, FunctGroupArray[guild].groupCode, cohort, it_count);
			fprintf(bm->logFile, "DHWsum = %e, period_effect= %e, prop_bleaching = %e, bleach_rate= %e ",
					CORALREEF[sp_id].DHWsum[bm->current_box], period_effect, prop_bleaching, bleach_rate);
			fprintf(bm->logFile, "lost_biom = %e, dead_biom =%e\n", lost_biom, dead_biom);
			fprintf(bm->logFile, "Time %e box %d, group %s, FunctGroupArray[guild].speciesParams[DHW_thresh_id] = %e ", bm->dayt, bm->current_box, FunctGroupArray[guild].groupCode, FunctGroupArray[guild].speciesParams[DHW_thresh_id]);
			fprintf(bm->logFile, "FunctGroupArray[%s].mortality[%d]= %e ", FunctGroupArray[guild].groupCode, cohort, FunctGroupArray[guild].mortality[cohort]);
			fprintf(bm->logFile, "PropUnBleached = %e ", CORALREEF[sp_id].PropUnBleached[cohort][bm->current_box]);
			fprintf(bm->logFile, "zooxanth_biom = %e polyp_biom = %e sp_biom = %e ", zooxanth_biom, polyp_biom, sp_biom);
			fprintf(bm->logFile, "timestep_correction = %e ", timestep_correction);
            fprintf(bm->logFile, "Mdt = %e bleaching_has_occurred: %d\n", FunctGroupArray[guild].speciesParams[Mdt_id], bleaching_has_occurred[guild]);
			//abort();
		//}
        **/
        
 
	}
    
    /* Salinity related bleaching
     
     TODO: Refine this based on literature
     
     */
    if ((current_SALT < min_bleach_salt_sp) || (current_SALT > max_bleach_salt_sp)) {
        
        /* Find proportion bleaching */
        prop_bleaching = bleach_rate;
        
        CORALREEF[sp_id].PropUnBleached[cohort][bm->current_box] -= prop_bleaching;
        if(CORALREEF[sp_id].PropUnBleached[cohort][bm->current_box] < 0.0)
            CORALREEF[sp_id].PropUnBleached[cohort][bm->current_box] = 0.0;
        
        // Sanity check - could be much greater > 1.0 from calculations if super high temperature
        if (prop_bleaching > 1.0)
            prop_bleaching = 1.0;
        
        /* Eject zooxanthellae from those bleaching */
        lost_biom = zooxanth_biom * prop_bleaching;
        
        /* Kill off some of the host colonies - killing living tissue not effecting rugosity */
        dead_biom = polyp_biom * prop_bleaching * bleach_mort_rate;
        
        /* Update diagnostics - so sum of effects of host cohort plus an zooxanthellae lost from within it */
        bm->calcMLinearMort[guild][current_id] += ((lost_biom + dead_biom) / FunctGroupArray[guild].speciesParams[Mdt_id]);  // As will be * dt at anotehr step
        FunctGroupArray[guild].mortality[cohort] += ((lost_biom + dead_biom) / FunctGroupArray[guild].speciesParams[Mdt_id]);  // As will be * dt at anotehr step
        bm->calcTrackedMort[guild][cohort][0][ongoingM1_id] += (lost_biom + dead_biom); // As already * Mdt_id
        
        bleaching_has_occurred[guild] = 1;
        
        /**
         //if(bm->current_box == bm->checkbox){
        fprintf(llogfp, "Time: %e box%d-%d %s-%d mort: %e lost_biom: %e dead_biom: %e current_SALT: %e min_bleach_salt_sp: %e, max_bleach_salt_sp: %e bleaching_has_occurred: %d\n", bm->dayt, bm->current_box, bm->current_layer, FunctGroupArray[guild].groupCode, cohort, FunctGroupArray[guild].mortality[cohort], lost_biom, dead_biom, current_SALT, min_bleach_salt_sp, max_bleach_salt_sp, bleaching_has_occurred[guild]);
         //}
         **/
    }

	/* Deal with any recovery of zooxanthellae by host polys post bleaching */
	if ((time_since_last_shock > 0) && (time_since_last_shock < recovery_periodA)) {
		// Quick recover so thermal toleerance and growth rates uneffected as have taken back in the same zooxanthellae clade
		CORALREEF[sp_id].PropUnBleached[cohort][bm->current_box] += recovery_rate * timestep_correction;
 	} else if (time_since_last_shock > recovery_periodB){
		// Slow recovery so thermal toleerance and growth rates are affected as a different zooxanthellae clade is taken in
		amt_recovering = (1.0 - CORALREEF[sp_id].PropUnBleached[cohort][bm->current_box]) * recovery_rate * timestep_correction;
		CORALREEF[sp_id].GrowShift[bm->current_box] -= grow_shift_rate * amt_recovering;
		CORALREEF[sp_id].TempShift[bm->current_box] += temp_shift_rate * amt_recovering;
		CORALREEF[sp_id].PropUnBleached[cohort][bm->current_box] += amt_recovering;
	}

	/* Sanity check */
	if(CORALREEF[sp_id].PropUnBleached[cohort][bm->current_box] > 1.0)
		CORALREEF[sp_id].PropUnBleached[cohort][bm->current_box] = 1.0;
	if(CORALREEF[sp_id].PropUnBleached[cohort][bm->current_box] < 0.0)
		CORALREEF[sp_id].PropUnBleached[cohort][bm->current_box] = 0.0;

	return;
}


/**
 *	\brief Coral final rugosity contribution by the species
 *
 *	Calculated using
 *	Blackwood et al 2011. A model-based approach to determine the long-term effects
 *  of multiple interacting stressors on coral reefs. Ecological Applications, 21: 2722�2733
 *
 *	equation d(rugosity)/dt = rate_rugosity_prod_by_growing_corals * living_coral_cover * (3.0 - rugosity)
 *	                          - rate_rugosity_lost_bioreosion * (1.0 - living_coral_cover) * (rugosity - 1.0)
 *
 *	Rugosity should be an EPI tracer, but at present all physiochemical properties are stored as straight
 *	tracers so pretend it is in the lowest layer of the water column for now.
 *
 *  Updated to allow for bioerosion by sponges
 *
 */
double Calculate_Rugosity(MSEBoxModel *bm, int guild, int cohort, FILE *llogfp, int sp_level_calc){
    int sp, this_stage;
	double rug_growth, rug_erode, rug_space, rug_correction, step1, step2, step3, pa_reef, sa_reef, cdiam,
        cheight, cheight_a, reef_index, rate_rug_growth;
    int sp_id = (int) (FunctGroupArray[guild].speciesParams[coralID_id]);
    int stage = FunctGroupArray[guild].cohort_stage[cohort];
    
    double ans = 0;
    double delta_rug = 0.0; // this is the increase in rugosity
    double rug_erode_grazers = 0.0; // these are for the bioerosion bit
    double rug_erode_sponges = 0.0;
    double rate_rug_erode, rate_rug_erode_sponge, cover_boring_sponges, rug_erode_other, OA_enhancer;
    
    //fprintf(llogfp, "Time %e box%d-%d rate_rug_growth: %e\n", bm->dayt, bm->current_box, bm->current_layer, rate_rug_growth);

	rug_growth = 0;
	rug_erode = 0;
    
    if(!sp_level_calc) {
        switch (bm->flag_rugosity_model) {
        case cover_sum_rugosity:  /* Simple empirical relationship based on area cover */
            rug_space = 0.0;
            rug_correction = 0.0;
            for(sp = 0; sp < bm->K_num_tot_sp; sp++){
                if (FunctGroupArray[sp].isCover == TRUE) {
                    if((FunctGroupArray[sp].groupType == PHYTOBEN) ||
                       (FunctGroupArray[sp].groupType == SED_EP_FF) ||
                       (FunctGroupArray[sp].groupType == CORAL) ||
                       (FunctGroupArray[sp].groupType == SPONGE)  ||
                       (FunctGroupArray[sp].groupType == SEAGRASS) ||
                       (FunctGroupArray[sp].groupType == TURF)) {
                        rug_space += bm->coveramt[FunctGroupArray[sp].coverID][bm->current_box] * FunctGroupArray[sp].speciesParams[rug_factor_id];
                        rug_correction += FunctGroupArray[sp].speciesParams[rug_factor_id];
                    }
                }
            }
            
            rug_correction /= bm->max_rugosity;

            // Final rugosity relationship from area index - note will be directly setting the rugosity not handling it as a flux
            ans = rug_space / (rug_correction + small_num);
            
            if (bm->flag_georugosity)
                ans +=  bm->wgt_georugosity * bm->boxes[bm->current_box].base_rugosity;
            break;
        case bozec_rugosity: /* Based on equations from Bozec et al 2014 GCB */
            // Get 2D and 3D area of the reef(s) in the box
            pa_reef = 0.0;
            sa_reef = 0.0;
            
            for(sp = 0; sp < bm->K_num_tot_sp; sp++){
                if ((FunctGroupArray[sp].isCover == TRUE) && ((FunctGroupArray[sp].groupType == CORAL) || (FunctGroupArray[sp].groupType == SPONGE))) {
                    pa_reef += bm->coveramt[FunctGroupArray[sp].coverID][bm->current_box] * FunctGroupArray[sp].speciesParams[max_colony_diam_id];

                    /*
                    if(bm->newmonth)
                        fprintf(llogfp, "Time: %e box %d %s pa_reef: %e coveramt: %e max_colony_diam: %e\n",
                            bm->dayt, bm->current_box, FunctGroupArray[sp].groupCode, pa_reef, bm->coveramt[FunctGroupArray[sp].coverID][bm->current_box], FunctGroupArray[sp].speciesParams[max_colony_diam_id]);
                    */
                    
                    /* Note moved fixed values to variables so can tinker easier
                    bm->rugosity_bozec_a = 16.0
                    bm->rugosity_bozec_b = 1.5 (was 0.66)
                    bm->rugosity_bozec_c = 8.0
                    bm->rugosity_bozec_d = 3.0
                    */
                    
                    for (cohort = 0; cohort < FunctGroupArray[sp].numCohortsXnumGenes; cohort++) {
                        this_stage = FunctGroupArray[sp].cohort_stage[cohort];
                        
                        cheight_a = FunctGroupArray[sp].speciesParams[colony_ha_id];
                        cdiam = FunctGroupArray[sp].cohortSpeciesParams[this_stage][colony_diam_id];
                        
                        if (cdiam > (bm->coveramt[FunctGroupArray[sp].coverID][bm->current_box] * FunctGroupArray[sp].speciesParams[max_colony_diam_id]))
                            cdiam = (bm->coveramt[FunctGroupArray[sp].coverID][bm->current_box] * FunctGroupArray[sp].speciesParams[max_colony_diam_id]);
                        
                        cheight = cdiam * cheight_a;
                        
                        step1 = cdiam * cdiam / (bm->rugosity_bozec_a * cheight + small_num);
                        step2 = (bm->rugosity_bozec_a * cheight * cheight / (cdiam * cdiam + small_num)) + 1.0;
                        step2 = pow(step2, bm->rugosity_bozec_b) - 1.0;
                        step3 = (bm->rugosity_bozec_c * 3.141592654 / bm->rugosity_bozec_d) * step1 * step1 * step2;
                        sa_reef += step3;
                        
                        /*
                        if(bm->newmonth)
                            fprintf(llogfp, "Time: %e box %d %s-%d cheight_a: %e cheight: %e cdiam: %e step1 %e step2 %e step3 %e sa_reef: %e\n",
                                bm->dayt, bm->current_box, FunctGroupArray[sp].groupCode, cohort, cheight_a, cheight, cdiam, step1, step2, step3, sa_reef);
                        */
                    }
                }
            }
        
            // Get reef index (surface-area-index)
            reef_index = sa_reef / (pa_reef + small_num);
            
            // Final rugosity relationship from reef_index - note will be directly setting the rugosity not handling it as a flux
            ans = bm->rugosity_const * reef_index;
            
            if (bm->flag_georugosity)
                ans +=  bm->wgt_georugosity * bm->boxes[bm->current_box].base_rugosity;
            
            /*
            if(bm->newmonth)
                fprintf(llogfp, "Time: %e box %d rugosity: %e rugosity_const: %e reef_index: %e sa_reef: %e pa_reef %e\n",
                    bm->dayt, bm->current_box, ans, bm->rugosity_const, reef_index, sa_reef, pa_reef);
            */
                
            break;
        default:
            break;
        }
    } else {
        if (!bm->current_layer) {
            switch (bm->flag_rugosity_model) {
                case mechanistic_rugosity: /* Model details from Blackwood et al 2011 */
                rate_rug_growth = FunctGroupArray[guild].cohortSpeciesParams[stage][rugosity_inc_id] * 86400.0; // Note that the 86400.0 is used here as the rug_growth_rate is s-1 but growth rate is too so was getting s-2 in effect - TODO: May need some more thought
                
                // Rugosity increase due to growth, related to size-based cohort so get greater increase of rugosity due to growth of largest (height wise) coral stages
                rug_growth = FunctGroupArray[guild].growth[cohort] * rate_rug_growth;
                
                // Erosion by cryptic species (just related to area of coral cover) + any due to direct consumption
                rug_erode = CORALREEF[sp_id].PropUnBleached[cohort][bm->current_box] * FunctGroupArray[guild].speciesParams[rug_erode_id] + CORALREEF[sp_id].RugosityEaten[cohort][bm->current_box] + (1.0 - CORALREEF[sp_id].PropUnBleached[cohort][bm->current_box] + FunctGroupArray[guild].mortality[cohort] * 86400.0) * FunctGroupArray[guild].speciesParams[rug_bleacherode_id];
                
                // The * 86400.0 on the mortality rate is so it is no longer per second as the rug_bleacherode is, we can't remove the per second from the rug_bleacherode as it also multiples the PropUnBleached which is not a per second value (so the rug_bleacherode must be per second)
                
                ans = (rug_growth - rug_erode);

                /**
                if(bm->newmonth){
                    fprintf(llogfp, "Time %e box%d-%d rugosity: %e rug_growth: %e rug_erode: %e\n", bm->dayt, bm->current_box, bm->current_layer,
                           bm->boxes[bm->current_box].base_rugosity, rug_growth, rug_erode);
                    fprintf(llogfp, "Time %e box%d-%d growth: %e rate_rug_growth: %e rug_erode: %e, rug_eaten: %e, propunbleached: %e erode_param: %e mort: %e mort_erodparam: %e\n", bm->dayt, bm->current_box, bm->current_layer,
                            FunctGroupArray[guild].growth[cohort], rate_rug_growth,
                            CORALREEF[sp_id].PropUnBleached[cohort][bm->current_box] * FunctGroupArray[guild].speciesParams[rug_erode_id],
                            CORALREEF[sp_id].RugosityEaten[cohort][bm->current_box], CORALREEF[sp_id].PropUnBleached[cohort][bm->current_box], FunctGroupArray[guild].speciesParams[rug_erode_id], FunctGroupArray[guild].mortality[cohort], FunctGroupArray[guild].speciesParams[rug_bleacherode_id]);
                }
                **/
                break;
            case blackwood_rugosity:  /* Model details from Blackwood et al 2011 */
                rate_rug_growth = FunctGroupArray[guild].cohortSpeciesParams[stage][rugosity_inc_id];
                // Note no rate_rug_growth * 86400.0 here (unlike above) as not being multiple by the coral growth rate so does need to be a per second rate
                
                // Rugosity increase
                rug_growth = rate_rug_growth * CORALREEF[sp_id].PropUnBleached[cohort][bm->current_box] * (bm->max_rugosity - LocalRugosity);
                
                // Erosion by cryptic species (just related to area of coral cover) + any due to direct consumption
                rug_erode =  FunctGroupArray[guild].speciesParams[rug_erode_id] * (1.0 - CORALREEF[sp_id].PropUnBleached[cohort][bm->current_box]) * (LocalRugosity - 1.0);
                
                ans = (rug_growth - rug_erode);
                break;
            case bioerosion_rugosity: /* Blackwood model et al 2011 updated for sponge bioeroders */
                rate_rug_growth = FunctGroupArray[guild].cohortSpeciesParams[stage][rugosity_inc_id]; // rugosity growth for this species
                rate_rug_erode = FunctGroupArray[guild].speciesParams[rug_erode_id]; // amount of rugosity contributed by this species eroded by everything that is not a sponge or a grazer (e.g. worms, microborers, bivalves)
                rate_rug_erode_sponge = FunctGroupArray[guild].speciesParams[rug_erode_sponge_id]; // amount of rugosity contributed by this species eroded by sponges
                
                // Rugosity increase
                rug_growth = rate_rug_growth * CORALREEF[sp_id].PropUnBleached[cohort][bm->current_box] * bm->coveramt[FunctGroupArray[guild].coverID][bm->current_box] * (bm->max_rugosity - LocalRugosity); //  this attempts to include cover (C in Blackwood)
                
                // Bioerosion from scrapers and sponges
                cover_boring_sponges = bm->coveramt[FunctGroupArray[bm->sp_boring_sponges].coverID][bm->current_box];
                rug_erode_sponges = rate_rug_erode_sponge * cover_boring_sponges * (1.0 - (CORALREEF[sp_id].PropUnBleached[cohort][bm->current_box] * bm->coveramt[FunctGroupArray[guild].coverID][bm->current_box])) * (LocalRugosity - 1.0);  // this scales total bioerosion rate by the abundance of bioeroding sponges
                
                rug_erode_other = rate_rug_erode * (1.0 - (CORALREEF[sp_id].PropUnBleached[cohort][bm->current_box] * bm->coveramt[FunctGroupArray[guild].coverID][bm->current_box])) * (LocalRugosity - 1.0); // baseline erosion from other borers, not scaled
                rug_erode_grazers = CORALREEF[sp_id].RugosityEaten[cohort][bm->current_box]; // rugosity of this species eaten by all its grazers combined as resolved in Eat(). ** Not a rate like the other two **
                // now sum all terms together
                
                // bring in Ocean acidification enhancer as 1 / pHCorr, this will depend on pHCorr's shape.
                OA_enhancer = (1.0 / FunctGroupArray[bm->sp_boring_sponges].pHcorr);
                
                /* See Schoenberg et al (2017). Bioerosion is expected to increase linearly (with thresholds). Hence we apply the same coefficient calculated
                 for sponges to both sponge bioerosion and other bioerosion.
                 
                 Bioerosion from scraping is not expected to significantly increase (Schoenberg et al. 2017) */
                
                rug_erode = (rug_erode_sponges + rug_erode_other) * OA_enhancer;
                delta_rug = rug_growth - rug_erode;
                
                /* Find final state - bringing in grazers */
                //ans = (delta_rug + LocalRugosity) - rug_erode_grazers;
                ans = delta_rug - rug_erode_grazers;   // Exclude LocalRugosity as returning a flux so that will be added to LocalRugosity anyways
                break;
            case bozec_rugosity: /* Based on equations from Bozec et al 2014 GCB */
                // Get 2D and 3D area of the reef(s) in the box
                pa_reef = 0.0;
                sa_reef = 0.0;
                    
                for(sp = 0; sp < bm->K_num_tot_sp; sp++){
                    if ((FunctGroupArray[sp].isCover == TRUE) && ((FunctGroupArray[sp].groupType == CORAL) || (FunctGroupArray[sp].groupType == SPONGE))) {
                        pa_reef += bm->coveramt[FunctGroupArray[sp].coverID][bm->current_box] * FunctGroupArray[sp].speciesParams[max_colony_diam_id];
                        
                        /*
                        if(bm->newmonth)
                        fprintf(llogfp, "Time: %e CASE2 box %d-%d %s pa_reef: %e coveramt: %e max_colony_diam: %e\n", bm->dayt, bm->current_box, bm->current_layer, FunctGroupArray[sp].groupCode, pa_reef, bm->coveramt[FunctGroupArray[sp].coverID][bm->current_box], FunctGroupArray[sp].speciesParams[max_colony_diam_id]);
                        */
                        
                        /* Note moved fixed values to variables so can tinker easier
                         bm->rugosity_bozec_a = 16.0
                         bm->rugosity_bozec_b = 1.5 (was 0.66)
                         bm->rugosity_bozec_c = 8.0
                         bm->rugosity_bozec_d = 3.0
                         */
                        for (cohort = 0; cohort < FunctGroupArray[sp].numCohortsXnumGenes; cohort++) {
                            this_stage = FunctGroupArray[sp].cohort_stage[cohort];
                                
                            cheight_a = FunctGroupArray[sp].speciesParams[colony_ha_id];
                            cdiam = FunctGroupArray[sp].cohortSpeciesParams[this_stage][colony_diam_id];
                                
                            if (cdiam > (bm->coveramt[FunctGroupArray[sp].coverID][bm->current_box] * FunctGroupArray[sp].speciesParams[max_colony_diam_id]))
                                cdiam = (bm->coveramt[FunctGroupArray[sp].coverID][bm->current_box] * FunctGroupArray[sp].speciesParams[max_colony_diam_id]);
                                
                            cheight = cdiam * cheight_a;
                                
                            step1 = cdiam * cdiam / (bm->rugosity_bozec_a * cheight + small_num);
                            step2 = (bm->rugosity_bozec_a * cheight * cheight / (cdiam * cdiam + small_num)) + 1.0;
                            step2 = pow(step2, bm->rugosity_bozec_b) - 1.0;
                            step3 = (bm->rugosity_bozec_c * 3.141592654 / bm->rugosity_bozec_d) * step1 * step1 * step2;
                            sa_reef += step3;
                            
                            /*
                            if(bm->newmonth)
                            fprintf(llogfp, "Time: %e CASE 2 box %d-%d %s-%d cheight_a: %e cheight: %e cdiam: %e step1 %e step2 %e step3 %e sa_reef: %e\n", bm->dayt, bm->current_box, bm->current_layer, FunctGroupArray[sp].groupCode, cohort, cheight_a, cheight, cdiam, step1, step2, step3, sa_reef);
                            */
                        }
                    }
                }
                    
                // Get reef index (surface-area-index)
                reef_index = sa_reef / (pa_reef + small_num);
                    
                // Final rugosity relationship from reef_index - note will be directly setting the rugosity not handling it as a flux
                ans = bm->rugosity_const * reef_index;
                    
                if (bm->flag_georugosity)
                    ans += bm->wgt_georugosity * bm->boxes[bm->current_box].base_rugosity;
                
                /*
                if(bm->newmonth)
                fprintf(llogfp, "Time: %e CASE2 box %d-%d rugosity: %e rugosity_const: %e reef_index: %e sa_reef: %e pa_reef %e\n", bm->dayt, bm->current_box, bm->current_layer, ans, bm->rugosity_const, reef_index, sa_reef, pa_reef);
                */
                    
                break;
            default:
                break;
            }
        
            
            if (bm->flag_georugosity) {
                if(ans < (bm->wgt_georugosity * bm->boxes[bm->current_box].base_rugosity))
                    ans = bm->wgt_georugosity * bm->boxes[bm->current_box].base_rugosity;
            }
        }
    }

	return ans;
}

/**
 *	\brief Bound rugosity between 1 (flat) and n (usually 3-5; highly branched - like branching coral)
 *
 */
void BoundRugosity(MSEBoxModel *bm, double *tracerArray) {

	if (tracerArray[Rugosity_i] < bm->min_rugosity)
		tracerArray[Rugosity_i] = bm->min_rugosity;
	if (tracerArray[Rugosity_i] > bm->max_rugosity)
		tracerArray[Rugosity_i] = bm->max_rugosity;
    
	return;
}

/**
 *	\brief Destruction of rugosity by fishing
 *
 */
void Destroy_Rugosity(MSEBoxModel *bm, BoxLayerValues *boxLayerInfo, FILE *llogfp, int guild, int cohort, int fishery, double dead_biom){
	double rate_rug_crumble = bm->FISHERYprms[fishery][rugosity_effect_id] * FunctGroupArray[guild].cohortSpeciesParams[cohort][rugosity_dec_id];

	boxLayerInfo->localWCFlux[Rugosity_i] -= (dead_biom * rate_rug_crumble);
    
    
    fprintf(llogfp,"Destroy_Rugosity: Time: %e box%d-%d Rugosity: %e dead_biom: %e rate_rug_crumble: %e\n", bm->dayt, bm->current_box, bm->current_layer, boxLayerInfo->localWCFlux[Rugosity_i], dead_biom, rate_rug_crumble);

}

/**
 *	\brief Coral and sponge space competition with phytobenthos
 *
 * Routine modified to capture 3-way interactions between sponges, corals and algae. This compounds the space limitation
 * calculated by Get_Species_Area_Hab(), by capturing specific relationships between dominant (or focal) benthic groups. Main additions:
 *
 *      1. Sponges can limit coral growth, at a rate decided by XXX_coral_compete.
 *      2. Corals are aided by turf and also CCA
 *      3. Macroalgae can limit sponge growth, according to the parameter XXX_sponge_compete
 *      4. Corals can limit sponge growth, also using the parameter XXX_sponge_compete
 *      5. Sponges are not aided by turf, but they may be aided by CCA (Whalan et al. 2012). The more CCA, the more space available for sponges to recruit
 *
 */

double Coral_Space_Competition(MSEBoxModel *bm, BoxLayerValues *boxLayerInfo, FILE *llogfp, int guild, int cohort, double mum) {
	double ans = mum;
	double compete_space = 0.0;
	double facilitate_space = 0.0;
	int sp;
    int stage = FunctGroupArray[guild].cohort_stage[cohort];
    int is_sponge = 0;
    
    if ( FunctGroupArray[guild].groupType == SPONGE ){
        is_sponge = 1;
    }
	/* Find macralgal competitors */
	for(sp = 0; sp < bm->K_num_tot_sp; sp++){
		if (FunctGroupArray[sp].isCover == TRUE) {
			if(FunctGroupArray[sp].groupType == PHYTOBEN) {
                if( is_sponge )
                    compete_space += (boxLayerInfo->localEPITracers[FunctGroupArray[sp].CoverTracer] * FunctGroupArray[sp].speciesParams[sponge_compete_id]);
                else
                    compete_space += (boxLayerInfo->localEPITracers[FunctGroupArray[sp].CoverTracer] * FunctGroupArray[sp].speciesParams[coral_compete_id]);

                /*
                fprintf(llogfp, "Time: %e box%d-%d %s-%d compete_space %e as %s cover: %e coral_compete: %e\n",
                        bm->dayt, bm->current_box, bm->current_layer, FunctGroupArray[guild].groupCode, cohort, compete_space, FunctGroupArray[sp].groupCode,
                       boxLayerInfo->localEPITracers[FunctGroupArray[sp].CoverTracer], FunctGroupArray[sp].speciesParams[coral_compete_id]);
                 */
                
			} else if(bm->flag_competing_epiff && (bm->HABITATlike[guild][stage][FunctGroupArray[sp].coverID] <= 0.0)){
                compete_space += (boxLayerInfo->localEPITracers[FunctGroupArray[sp].CoverTracer] * fabs(bm->HABITATlike[guild][stage][FunctGroupArray[sp].coverID]));

                /*
                fprintf(llogfp, "Time: %e box%d-%d %s-%d compete_space %e as %s cover: %e splike: %e\n",
                        bm->dayt, bm->current_box, bm->current_layer, FunctGroupArray[guild].groupCode, cohort, compete_space, FunctGroupArray[sp].groupCode, boxLayerInfo->localEPITracers[FunctGroupArray[sp].CoverTracer], fabs(bm->HABITATlike[guild][stage][FunctGroupArray[sp].coverID]));
                */
            }
                
            
	/* Turf facilitation */
			if(bm->containsTurf && (FunctGroupArray[sp].groupType == TURF)) {
                if( is_sponge )
                    facilitate_space += (boxLayerInfo->localEPITracers[FunctGroupArray[sp].CoverTracer] * FunctGroupArray[sp].speciesParams[sponge_compete_id]);
                else
                    facilitate_space += (boxLayerInfo->localEPITracers[FunctGroupArray[sp].CoverTracer] * FunctGroupArray[sp].speciesParams[coral_compete_id]);

                /*
                fprintf(llogfp, "Time: %e box%d-%d %s-%d facilitate_space %e as %s cover: %e coral_compete: %e\n",
                        bm->dayt, bm->current_box, bm->current_layer, FunctGroupArray[guild].groupCode, cohort, facilitate_space, FunctGroupArray[sp].groupCode,
                        boxLayerInfo->localEPITracers[FunctGroupArray[sp].CoverTracer], FunctGroupArray[sp].speciesParams[coral_compete_id]);
                */

			} else if(bm->flag_competing_epiff && (bm->HABITATlike[guild][stage][FunctGroupArray[sp].coverID] > 0.0)){
                facilitate_space += (boxLayerInfo->localEPITracers[FunctGroupArray[sp].CoverTracer] * bm->HABITATlike[guild][stage][FunctGroupArray[sp].coverID]);
                
                /*
                fprintf(llogfp, "Time: %e box%d-%d %s-%d facilitate_space %e as %s cover: %e splike: %e\n",
                        bm->dayt, bm->current_box, bm->current_layer, FunctGroupArray[guild].groupCode, cohort, facilitate_space, FunctGroupArray[sp].groupCode,
                        boxLayerInfo->localEPITracers[FunctGroupArray[sp].CoverTracer], bm->HABITATlike[guild][stage][FunctGroupArray[sp].coverID]);
                */
            }

		}
	}

	if(!bm->containsTurf) {
		facilitate_space = 1.0;
	}

	/* Sanity check */
	if(compete_space > 1.0)
		compete_space = 1.0;
	if(facilitate_space > 1.0)
		facilitate_space = 1.0;

	ans = mum * (1.0 - compete_space) * (1.0 + facilitate_space);

	if ((bm->which_check == guild) && (bm->current_box == bm->checkbox)) {
		fprintf(llogfp,"Time: %e box%d-%d %s-%d ans: %e mum: %e compete_space: %e facilitate_space: %e\n",
				bm->dayt, bm->current_box, bm->current_layer, FunctGroupArray[guild].groupCode, cohort,
				ans, mum, compete_space, facilitate_space);
	}

	return ans;

}

/**
 *	\brief Coral growth - allowing for growth that skips across sub-bins within size/age class
 *
 *	 i.e. Lets say we have 3 size/age bins each with 3 sub-bins then
 *
 *	 |     ||     ||     |
 *	 |_|_|_||_|_|_||_|_|_|
 *	  A B C  D E F  G H I
 *
 * The  A, B, or C could transition into D, but they couldn't skip to G
 * (must go through at least one of the sub-bins in the second size/age bin first)
 *
 */
double Coral_Variable_Transitions(MSEBoxModel *bm, int species, int cohort, int do_debug, FILE *llogfp){
	int i;
	int sp_AgeClassSize = FunctGroupArray[species].ageClassSize;
	double avail_i, trans_i;
	double ans = 0.0;
	double max_accel_transition = FunctGroupArray[species].speciesParams[coral_max_accel_trans_id];
	double accel_transA = FunctGroupArray[species].speciesParams[coral_max_accelA_id];
	double accel_transB = FunctGroupArray[species].speciesParams[coral_max_accelB_id];

	for ( i = (int) (sp_AgeClassSize - 2); i > 0; i--) {
		avail_i = FunctGroupArray[species].boxPopRatio[bm->current_box][bm->current_layer][cohort][i];
		trans_i = min(avail_i, max_accel_transition / (1 + exp(-accel_transA * (i - accel_transB))));  // Used to say max, but can't transition more than what's available for transer so needs to be a min so avail_i is max possible
		FunctGroupArray[species].boxPopRatio[bm->current_box][bm->current_layer][cohort][i] -= trans_i;
		ans += trans_i;
	}

	return ans;
}
