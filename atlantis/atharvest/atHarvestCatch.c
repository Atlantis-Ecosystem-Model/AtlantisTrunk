/**
 * \file atHarvestCatch.c
 * \ingroup atHarvestLib
 *
 * \brief Functions used to calculate catch.
 *
 *	\author Bec Gorton and Beth Fulton
 *
 * This file currently contains functions to calculate the catch based on the following algorithms:
 *
 * 1. Dynamic Catch - the amount catch is dependent on a wide range of parameters that can change in time. See Get_Dynamic_Catch for more information.
 * 2. Imposed Catch - were catch values are loaded from provided time series (ts) files.
 * 3. Fishing Mortality - where a mortality percentage due to fishing is provided the input files.
 *
 *
 *	Changes:
 * 	05-05-2010 Bec Gorton
 *  Changed the code that used the mFC value to apply the mFC_scale value before its used. This mFC_scale
 *  slot is used to store any scaling that should be applied to implement the broken-stick management strategy.
 *
 */


/*  Open library routines */
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <math.h>
#include <atHarvest.h>

/* Static private function protypes */
static int 		Get_Dynamic_Catch(MSEBoxModel *bm, int guildcase, int chrt, int stage, int nf, int do_debug, int depend_dis, double li, double vert_scale, double Biom,
					double *gear_change_scale, int boxkey_id, double *SPtoFC, FILE *llogfp);

static double   Get_Aquaculture_Harvest(MSEBoxModel *bm, int sp, int chrt, int stage, int nf, double Biom, FILE *llogfp);
static double 	Get_Fishing_Mortality(MSEBoxModel *bm, int sp, int chrt, int stage, int nf, double Biom, double li, int do_debug, FILE *llogfp);
static double 	Get_Catchability(MSEBoxModel *bm, int sp, int chrt, int stage, int nf);
static double 	Get_Positional_Availability(MSEBoxModel *bm, int sp, int nf, int do_debug, FILE *llogfp);
static double 	Get_addlsm_Scale(MSEBoxModel *bm, int guildcase, int nf, double addlsm, double addsigma, int sel_curve, double li);
static double 	Get_Swept_Area(MSEBoxModel *bm, int nf, double *gear_change_scale);
static double 	Get_Fisheries_Escapment(MSEBoxModel *bm, int sp, int nf, double li, int dp_debug, FILE *llogfp);


/* Global Functions */

double Harvest_Get_TotCumCatch(int sp, int nf, int iy) {

	/* check stuff */
	return TotCumCatch[sp][nf][iy];
}

/**
 * \brief Get the percentage of sp species biomass caught in the current box.
 *
 */

int Get_Catch(MSEBoxModel *bm, int sp, int chrt, int stage, int nf, int do_debug, int flagimposecatch, int depend_dis, int boxkey_id, double Biom, double li,
		double vert_scale, double *gear_change_scale, double *loadFC, double *SPtoFC, double *mpa_losses, FILE *llogfp) {

	int flagF;
	int returnValue = 0;

	/**
	if(do_debug){
		fprintf(llogfp, "Time: %e box%d-%d Calculate Catch %s:%d, Biom = %.20e, stage = %d, fishery = %s, flagimposecatch = %d, depend_dis = %d, flagF: %d, q: %e, target: %d, fishery_active: %d\n",
				bm->dayt, bm->current_box, bm->current_layer, FunctGroupArray[sp].groupCode, chrt, Biom, stage, FisheryArray[nf].fisheryCode,
				flagimposecatch, depend_dis, (int)(bm->SP_FISHERYprms[sp][nf][flagF_id]), bm->SP_FISHERYprms[sp][nf][q_id], bm->FISHERYtarget[nf][sp],
				(int)(bm->FISHERYprms[nf][fisheriesactive_id]));
	}
	**/
    
	/* Check for use of fishing mortalities */
	flagF = (int) (bm->SP_FISHERYprms[sp][nf][flagF_id]);

    /* To short cut the loop when can't catch anything skip ahead - including when no incidental
	 mortality don't continue longer than is necessary for targeted fishing */
	if ((!flagimposecatch) && (!bm->SP_FISHERYprms[sp][nf][q_id]) && (!depend_dis) && (!flagF) && (!bm->flagincidmort && (bm->FISHERYtarget[nf][sp] < 1)))
		return -1;

	/**** Do actual fisheries calculations *******************************************/
	/* Check to see if fishery active in this box and at this time of day if
	 not go on to next fishery */

	if (!(int) (bm->FISHERYprms[nf][fisheriesactive_id])) {
		return -1;
	}
    
	/* Deal with aquaculture species first */
	if (FunctGroupArray[sp].isCultured){
		*SPtoFC = Get_Aquaculture_Harvest(bm, sp, chrt, stage, nf, Biom, llogfp);

	/* imposed time series catch */
	} else if (flagimposecatch) {

		if (do_debug) {
			fprintf(llogfp, "Calculate Catch - Time: %e, %s-%d (%d) box%d-%d %s flagimposecatch: %d, boxkey_id: %d\n", bm->dayt, FunctGroupArray[sp].groupCode, chrt, tscatchid[sp], bm->current_box, bm->current_layer, FisheryArray[nf].fisheryCode, flagimposecatch, boxkey_id);
		}
		*loadFC = Get_Imposed_Catch(bm, sp, chrt, stage, nf, do_debug, vert_scale, Biom, boxkey_id, mpa_losses, llogfp);

		/* To avoid handing around a pointer set SPtoFC = loadFC here */
		*SPtoFC = *loadFC;
	} else if (flagF) {

		/* Apply fishing mortality */
		*SPtoFC = Get_Fishing_Mortality(bm, sp, chrt, stage, nf, Biom, li, do_debug, llogfp);

	} else {
		/* Dynamic catch */
		returnValue = Get_Dynamic_Catch(bm, sp, chrt, stage, nf, do_debug, depend_dis, li, vert_scale, Biom, gear_change_scale, boxkey_id, SPtoFC, llogfp);
	}
    
    // Sanity checks
    if (*SPtoFC < 0.0)
        *SPtoFC = 0.0;
    
    if (returnValue < 0)
        returnValue = 0;

    return returnValue;
}


double Get_Fishery_Group_Change_Scale(MSEBoxModel *bm, int nf, int sp, int paramID, int numChangeParamID, double ***changeArray) {

	int flagchange, num_changes;
	double scale = 1.0;

	flagchange = (int) (bm->SP_FISHERYprms[sp][nf][paramID]);
	num_changes = (int) (bm->SP_FISHERYprms[sp][nf][numChangeParamID]);

	//fprintf(bm->logFile, "flagchange = %d\n",flagchange);
	if (flagchange){
		scale = Util_Get_Change_Scale(bm, num_changes, changeArray[nf]);
	}
	return scale;

}
double Get_Fishery_Change_Scale(MSEBoxModel *bm, int nf, int paramID, int numChangeParamID, double ***changeArray) {

	int flagchange, num_changes;
	double scale = 1.0;

	flagchange = (int) (bm->FISHERYprms[nf][paramID]);
	num_changes = (int) (bm->FISHERYprms[nf][numChangeParamID]);

	if (flagchange){
		scale = Util_Get_Change_Scale(bm, num_changes, changeArray[nf]);
	}
	return scale;

}

/** Private Functions */

/**
 * \brief Aquaculture harvest
 *
 * A very simple representation - as soon as old enough then harvestedat rate set by catchability parameter
 *
 */
double Get_Aquaculture_Harvest(MSEBoxModel *bm, int sp, int chrt, int stage, int nf, double Biom, FILE *llogfp) {
	double SPtoFC = 0.0;
	double prop_harvest = 0.0;
	double FCpressure = bm->Effort[bm->current_box][nf];
    int age_harvest = (int)(FunctGroupArray[sp].speciesParams[aquacult_age_harvest_id]);

	// get proportion harvested if pen ready
	if (chrt >= age_harvest)
		prop_harvest = Get_Catchability(bm, sp, chrt, stage, nf);

	// Calculate final amount caught - corrected to per second (as will be multipled up when integrate)
	SPtoFC = Biom * prop_harvest * (FCpressure / 86400.0);

    /**
     if(sp == bm->which_check)
        fprintf(llogfp,"Time: %e box%d-%d %s-%d has SPtoFC: %e Biom: %e prop_harvest: %e FCpressure: %e aquacult_age_harvest: %d\n",
			bm->dayt, bm->current_box, bm->current_layer, FunctGroupArray[sp].groupCode, chrt,
			SPtoFC, Biom, prop_harvest, FCpressure / 86400, age_harvest);
    **/

	return SPtoFC;
}

/**
 * \brief Get the mortality due to explicit mortality.
 *
 * This is the ForcedFishingMortality mortality values in the harvest input file. There is allowance for
 * scaling these values over time though the FishingMortalityChanges input parameters as well as scaling
 * to allow for a proportion of the box that is in an MPA and a proportion that cannot be fished due to an
 * infringement.
 *
 * @return
 * The fishing mortality.
 *
 * @param
 * bm: Pointer to the MSEBoxModel structure
 * sp: The index of the functional group
 * chrt: The cohort index
 * nf: The fishery index
 * Biom: The biomass of this cohort of the sp in the current box layer.
 * llogfp: Pointer to the current log file.
 *
 */
double Get_Fishing_Mortality(MSEBoxModel *bm, int sp, int chrt, int stage, int nf, double Biom, double li, int do_debug, FILE *llogfp) {

	double mFC, mFC_scale, mpa_scale, mpa_infringe, SPtoFC, sel, quota;
	double gear_change_scale = 0, Displace_rescale = 1.0;
	int mFC_start_age, mFC_end_age, flagfcmpa, sel_curve;
	int basechrt = chrt / FunctGroupArray[sp].numGeneTypes;

	mpa_infringe = 1.0;

	/* Get fishing mortality - corrected from (day-1) to (s-1) */
	mFC_start_age = (int) (bm->SP_FISHERYprms[sp][nf][mFC_start_age_id]);
    mFC_end_age = (int) (bm->SP_FISHERYprms[sp][nf][mFC_end_age_id]);
	mFC = bm->SP_FISHERYprms[sp][nf][mFC_id] / 86400.0;
	/* Apply the broken stick scaler is there is any  - default will be 1.0 */
	mFC = mFC * bm->SP_FISHERYprms[sp][nf][mFC_scale_id];


	if(FunctGroupArray[sp].isFished == FALSE){
		if ((FunctGroupArray[sp].groupAgeType == BIOMASS) && ((mFC_start_age != 0) || (mFC_end_age < FunctGroupArray[sp].numCohorts)))
			quit("Get_Fishing_Mortality. Group %s, Fishery %s, Shouldn't be in here for impacted only groups. Some parameters including mFC_start_age_id and mFC_end_age_id will be undefined.\n", FunctGroupArray[sp].groupCode, FisheryArray[nf].fisheryCode);

	}
            
	if(do_debug){
		fprintf(llogfp,"Get_Fishing_Mortality - Time: %e %s %s origmFC: %e, mFC_scale: %e, mFC: %e)\n",
			bm->dayt, FisheryArray[nf].fisheryCode, FunctGroupArray[sp].groupCode, bm->SP_FISHERYprms[sp][nf][mFC_id] / 86400.0, bm->SP_FISHERYprms[sp][nf][mFC_scale_id], mFC);
	}

	/* Get scenario scalars */
	if (bm->flagchangeF){
		mFC_scale = Get_Fishery_Group_Change_Scale(bm, nf,sp, mFC_num_changes_id, mFC_num_changes_id, mFCchange[sp]);
	}else{
		mFC_scale = 1.0;
	}

	/* Correct for presence of mpas */
	flagfcmpa = (int) (bm->FISHERYprms[nf][flagmpa_id]);
	if (flagfcmpa)
		mpa_scale = bm->MPA[bm->current_box][nf];
	else
		mpa_scale = 1.0;

	/* Allow for infringement */
	if (bm->flaginfringe) {
		mpa_infringe = bm->FISHERYprms[nf][infringe_id];
		if (mpa_infringe > mpa_scale)
			mpa_scale = mpa_infringe;
	}


	/* In the case where using selectivity to determine which ages suffer the fishing mortality */

	sel_curve = (int) (bm->FISHERYprms[nf][selcurve_id]);

    if( bm->flag_sel_with_mFC) {

        /* Calculate selectivity - size based only applies to vertebrates not the biomass pool invertebrates */
		//sel = Get_Selectivity(bm, sp, stage, nf, li, sel_curve, 0.0, 0.0); // Replaced with Get_Catch_Selectivity() for consistency with other fishing options
        sel = Get_Catch_Selectivity(bm, sp, stage, nf, li, &gear_change_scale, &sel_curve);
        
	} else {
		sel = 1.0;

		/* Only put this filter on if not using selectivity as the filter */
		if(FunctGroupArray[sp].isVertebrate == TRUE || FunctGroupArray[sp].groupAgeType == AGE_STRUCTURED_BIOMASS){
			if((basechrt < mFC_start_age) || (basechrt >= mFC_end_age))
				/* Actually too young to be caught so don't apply the mortality here */
				mFC = 0;
		}
	}

    
    if ( bm->flag_stop_F_tac ) {
        quota = bm->TACamt[sp][nf][now_id] * kg_2_mg / bm->X_CN;
        if(((TotCumCatch[sp][nf][bm->thisyear] + bm->TotOldCumCatch[sp][nf]) > bm->prop_within * quota) || (bm->TripCatch[sp][nf] > bm->SP_FISHERYprms[sp][nf][catch_allowed]))
            mFC = 0.0;
    }
    
    Displace_rescale = 1.0;
    if (bm->flagdisplace) {
       // Pulling in effort displacement that is calculated previously - this uses the overloaded Effort array (even though effort not active)
        Displace_rescale = bm->Effort[bm->current_box][nf];
    }
    
	/* Apply fishing mortality */
	SPtoFC = mFC * mpa_scale * mFC_scale * Displace_rescale * Biom * sel;

	if(do_debug){
		fprintf(llogfp,"Time: %e, %d-%d %s-%d %s SPtoFC orig: %e (with mFC: %e mFC-param: %e, mFCscal_prm: %e, and Biom: %e, mpa_scale:%e, mFC_scale: %e, sel: %e)\n",
			bm->dayt, bm->current_box, bm->current_layer, FunctGroupArray[sp].groupCode,
			chrt, FisheryArray[nf].fisheryCode, SPtoFC, mFC, bm->SP_FISHERYprms[sp][nf][mFC_id] / 86400.0, bm->SP_FISHERYprms[sp][nf][mFC_scale_id], Biom, mpa_scale, mFC_scale, sel);
	}

	return SPtoFC;
}

/**
 *
 * \brief Calculate the dynamic catch for this cohort of the given functional group by the nf fishery.
 *
 *
 * 	The dynamic catch amount is based on the total effort in the current box (bm->Effort) and the vertical distribution of the effort in the water column (Effort_vdistrib).
 *	These two values are calculated in the management module in Manage_Calculate_Total_Effort().
 *
 *
 *
 */
int Get_Dynamic_Catch(MSEBoxModel *bm, int guildcase, int chrt, int stage, int nf, int do_debug, int depend_dis, double li, double vert_scale, double Biom,
		double *gear_change_scale, int boxkey_id, double *SPtoFC, FILE *llogfp) {

	double vertdistrib, FCpressure, sel, q, swept_area, FCvol, FCescape;
	double q_pos_scale, habitat_scale;
	int sel_curve;
	//int basechrt = chrt / FunctGroupArray[guildcase].numGeneTypes;

	/* Determine how much of the effort in the box is applied actually at this depth.
	 If considering a an area that is too shallow to have all
	 the water column layers present	rescale (done in set-up, result read-in here */
	vertdistrib = Effort_vdistrib[bm->current_box][bm->current_layer][nf];

	/* Read total effort for the box from effort matrix and adjust for depth using
	 vertdistrib value from above. Also check if pressure is now zero, if it is skip to next
	 fishery */
	FCpressure = bm->Effort[bm->current_box][nf] * vertdistrib;

	/**/
	if((do_debug) && (bm->newweek)){
		fprintf(llogfp, "Time %e box%d-%d guildcase: %s:%d, nf: %s, Effort[%d][%s]: %.20e, FCpressure: %.20e, vertdistrib: %e, current_layer: %d\n",
				bm->dayt, bm->current_box, bm->current_layer, FunctGroupArray[guildcase].groupCode, chrt, FisheryArray[nf].fisheryCode, bm->current_box, FisheryArray[nf].fisheryCode, bm->Effort[bm->current_box][nf], FCpressure, vertdistrib, bm->current_layer);
	}
	/**/


	if (!FCpressure && !depend_dis) {
		return -1;
	}

	/* Calculate selectivity - size based only applies to vertebrates not
	 the biomass pool invertebrates */
	sel = Get_Catch_Selectivity(bm, guildcase, stage, nf, li, gear_change_scale, &sel_curve);
    
	/* Correct for gear selectivity and check if pressure is now zero, if it is skip to next
	 fishery */
	FCpressure = FCpressure * sel;

	/**/
	if (do_debug && bm->newweek) {
		fprintf(llogfp, "Fishing after selectivity - guildcase: %s-%d, box%d FCpressure: %.20e, sel: %e, sel_curve: %d, sel[%s][%d]: %e\n",
				FunctGroupArray[guildcase].groupCode, chrt, bm->current_box, FCpressure,
				sel, sel_curve, FisheryArray[nf].fisheryCode, selcurve_id, bm->FISHERYprms[nf][selcurve_id]);
	}
	/**/
	if (!FCpressure && !depend_dis) {
		return -1;
	}

	/* Calculate catchability = result will be scaled to allow for seasonal closures. */
	q = Get_Catchability(bm, guildcase, chrt, stage, nf);

	if (do_debug && bm->newweek) {
		fprintf(llogfp,"time: %e, sp: %s, chrt: %d, nf: %s, box: %d-%d, q: %e, q_id: %e)\n",
				bm->dayt, FunctGroupArray[guildcase].groupCode, chrt, FisheryArray[nf].fisheryCode, bm->current_box, bm->current_layer, q, bm->SP_FISHERYprms[guildcase][nf][q_id]);
	 }

	/* If can't catch the group skip ahead */
	if (!q) {
		return -1;
	}

	/* Availability - take account of position of fishing in the water column.
	 If the gear's position doesn't match the position of the vertebrates
	 then reduce the catch accordingly */
	q_pos_scale = Get_Positional_Availability(bm, guildcase, nf, do_debug, llogfp);
	FCpressure = FCpressure * q_pos_scale;
	if (do_debug && bm->newweek) {
		fprintf(llogfp, "Fishing Pressure after Availability - FCpressure = %e\n", FCpressure);
	}

	/* Availability - take account of area fished */
	habitat_scale = Harvest_Get_Habitat_Availability(bm, guildcase, stage, nf, q, do_debug, llogfp);

	FCpressure = FCpressure * habitat_scale;
	if (do_debug && bm->newweek) {
		fprintf(llogfp, "Fishing Pressure after area fished - FCpressure = %e, habitat_scale: %e\n", FCpressure, habitat_scale);
	}

	/* Area correct the effort estimate - effort read in as number of events
	 per day per box, now multiply by swept area of each event and
	 divide by area the fishery can access to get it into m-3 so matches biomass units */

	/* Calculate swept_area */
	swept_area = Get_Swept_Area(bm, nf, gear_change_scale);

	/* Scale fishing pressure by area fished vs entire area that could be fished */
	FCvol = bm->cell_vol * FC_hdistrib[bm->current_box][nf];
	FCpressure *= swept_area / (FCvol + small_num);

	/**/
	if (do_debug && bm->newweek) {
		fprintf(llogfp, "Fishing correct for swept area - guildcase: %s, by %s in box%d, FCpressure: %e, swept_area: %e\n", FunctGroupArray[guildcase].groupCode,
				FisheryArray[nf].fisheryCode, bm->current_box, FCpressure, swept_area);
	}
	/**/

	/* Check if pressure is now zero, if it is skip to next fishery */
	if (!FCpressure && !depend_dis) {
		return -1;
	}

	/* Fisheries escapement - those fish that escape alive and so are effectively
	 never caught */
	FCescape = Get_Fisheries_Escapment(bm, guildcase, nf, li, do_debug, llogfp);
	FCpressure = FCpressure * (1.0 - FCescape);

	/**/
	if (do_debug && bm->newweek) {
		fprintf(llogfp, "Fishing correct for escapement - guildcase: %s, by %s in box%d, FCpressure: %e, FCescape: %e\n",
				FunctGroupArray[guildcase].groupCode, FisheryArray[nf].fisheryCode, bm->current_box, FCpressure, FCescape);
	}
	/**/


	if (!FCpressure && !depend_dis) {
		return -1;
	}

	/* Correct into seconds */
	FCpressure = FCpressure / 86400.0;

	if (FCpressure > 1)
		FCpressure = 1;

	/* Fluxes to fishing - species biomass (or numbers) lost to fishing*/
	*SPtoFC = FCpressure * Biom;

	/**/
	if (!(_finite(*SPtoFC))) {
		fprintf(llogfp, "Time: %e, sp: %d, chrt: %d, nf: %d, box: %d, dynamic catch Biom: %e, FCpressure: %e, SPtoFC: %e\n", bm->dayt, guildcase, chrt, nf, bm->current_box,
				Biom, FCpressure, *SPtoFC);
	}
	/**/

	return 0;
}


double Get_Catchability(MSEBoxModel *bm, int sp, int chrt, int stage, int nf) {
	double q, q_scale, prop_spawn, prop_adjust;
	int flagfcmpa;
    int stock_id = bm->group_stock[sp][bm->current_box][bm->current_layer];

    prop_spawn = 0.0;
    prop_adjust = 0.0;
	q = 0.0;

	/* Calculate catchability */
	q_scale = Get_Fishery_Group_Change_Scale(bm, nf, sp, flagQchange_id, Q_num_changes_id, Qchange[sp]);

	q = bm->SP_FISHERYprms[sp][nf][q_id] * q_scale;
    
    /* Find the stock differential for catchability */
    q *= qSTOCK[sp][stock_id][stage];
    
    //if(!sp)
    //    fprintf(bm->logFile,"Time: %e %s-%d q_id: %e q_scale: %e q: %e ", bm->dayt, FunctGroupArray[sp].groupCode, chrt, bm->SP_FISHERYprms[sp][nf][q_id], q_scale, q);

	/* If have seasonal spawning spatial closures or trading off TAC and spatial closures
	 and closures in place adjust q to mimic effects
	 */
	flagfcmpa = (int) (bm->FISHERYprms[nf][flagmpa_id]);
	if (bm->SP_FISHERYprms[sp][nf][TACvsMPA_id] || (flagfcmpa == mix_fix_rolling_mpa) || (flagfcmpa == spawn_closure) || bm->SP_FISHERYprms[sp][nf][spawn_closure_id]) {
		if (EMBRYO[sp].Spawn_Now[chrt][expect_id]) {
			/* In the spawning season then adjust q if a reproductively mature adult age class */
			if (FunctGroupArray[sp].isVertebrate == TRUE)
				prop_spawn = FunctGroupArray[sp].scaled_FSPB[chrt];
			else if (stage)
				prop_spawn = 1;
			else
				prop_spawn = 0;
			prop_spawn *= bm->SP_FISHERYprms[sp][nf][prop_spawn_close_id];
			prop_adjust = (1 - prop_spawn);
			q *= prop_adjust;
		}
	}
    
    /* Sanity check */
    if (q > 1.0)
        q = 1.0;
    
    //if(!sp)
    //    fprintf(bm->logFile,"after spawning closure q: %e\n", q);
                                                                                            
                                                                                                
	return q;
}


double Get_Catch_Selectivity(MSEBoxModel *bm, int sp, int stage, int nf, double li, double *gear_change_scale, int *sel_curve) {

	double sel, addlsm, addsigma;
	int flagchangeSEL, SEL_num_changes, i, now_change = 0, past_change, sel_curveB;

	/* Calculate selectivity - size based only applies to vertebrates not
	 the biomass pool invertebrates */
	flagchangeSEL = (int) (bm->FISHERYprms[nf][flagchangeSEL_id]);
	SEL_num_changes = (int) (bm->FISHERYprms[nf][SEL_num_changes_id]);
	*sel_curve = (int) (bm->FISHERYprms[nf][selcurve_id]);

	if (flagchangeSEL && (bm->dayt >= SELchange[nf][0][start_id]) && (SELchange[nf][0][start_id] != 0)) {
		for (i = 0; i < SEL_num_changes; i++) {
			if (bm->dayt >= SELchange[nf][i][start_id])
				now_change = i;
		}
		*sel_curve = (int) (SELchange[nf][now_change][selcurvechange_id]);
		past_change = now_change - 1;
		if (now_change > 0)
			sel_curveB = (int) (SELchange[nf][past_change][selcurvechange_id]);
		else
			sel_curveB = (int) (bm->FISHERYprms[nf][selcurve_id]);
		if (*sel_curve == sel_curveB) {
			addlsm = SELchange[nf][now_change][addlsm_id];
			addsigma = SELchange[nf][now_change][addsigma_id];
		} else {
			addlsm = 0.0;
			addsigma = 0.0;
		}
	} else {
		addlsm = 0.0;
		addsigma = 0.0;
	}


	/* Scale the addlsm value */
	addlsm = Get_addlsm_Scale(bm, sp, nf, addlsm, addsigma, *sel_curve, li);

	// TODO: Get rid of this special condition - getting rid of this changes output in the AMS model!
	/* Make sure use logistic curve for nets interacting with large mammals */
	if (FunctGroupArray[sp].groupType == MAMMAL){
		*sel_curve = 2;
	}

	sel = Get_Selectivity(bm, sp, stage, nf, li, *sel_curve, addlsm, addsigma);

	if (addlsm != 0)
		(*gear_change_scale) *= 1.0 / (fabs(addlsm) + small_num);

	return sel;
}



double Get_Positional_Availability(MSEBoxModel *bm, int sp, int nf, int do_debug, FILE *llogfp) {
	int fishery_dempel, sp_dempel, access_thru_wc;
	double q_pos_scale;

	access_thru_wc = (int) (FunctGroupArray[sp].speciesParams[access_thru_wc_id]);

	/* Availability - take account of position of fishing in the water column.
	 If the gear's position doesn't match the position of the vertebrates
	 then reduce the catch accordingly */
	fishery_dempel = (int) (bm->FISHERYprms[nf][flagdempelfishery_id]);
	sp_dempel = (int) (FunctGroupArray[sp].speciesParams[flagdem_id]);
	if ((sp_dempel != fishery_dempel) && (access_thru_wc < 1))
		q_pos_scale = k_mismatch;
	else
		q_pos_scale = 1.0;

	/**/
	if (do_debug && bm->newweek) {
		fprintf(llogfp, "Correct for positional avail - %s by %s in %d fishery_dempel: %d, access_thru_wc: %d, k_mismatch: %e, q_pos_scale: %e\n",
				FunctGroupArray[sp].groupCode, FisheryArray[nf].fisheryCode, bm->current_box, fishery_dempel, access_thru_wc, k_mismatch, q_pos_scale);
	}
	/**/

	return q_pos_scale;
}

double Harvest_Get_Habitat_Availability(MSEBoxModel *bm, int sp, int stage, int nf, double q, int do_debug, FILE *llogfp) {
	double scaler = 1.0, p_fished, k_patchcover, pop_prop;
	int flagsphabitat;

	/* Availability - take account of area fished */
	flagsphabitat = (int) (bm->SP_FISHERYprms[sp][nf][flaghabitat_id]);
	p_fished = p_fishi[sp][stage][nf];

	switch (flagsphabitat) {
	case homog_habitat_pressure: /* Fishery and species distributions homogeneous, only area of
	 overlap determines availability. Include catchability q in expression here
	 too as can't include it separately above as then would be double
	 accounting of it if use other flagsphabitat case (where its used inside
	 the availability calculations */
		scaler = p_fished * q;
		break;
	case ellis_habitat_pressure: /* Use Ellis and Pantus two stage model to determine proportion of population
	 fished	(as assuming habitat and fishing can be patchy) where
	 p_fished = Probability that trawl point is within the population's area
	 k_patchcover = level of cover in the patch
	 k_pattern = distribution of fishing (so 0 if Poission, -1 if uniform)
	 k_patches = total number of patches per box. Currently assumed to = 2
	 (fished and not fished)
	 pop_prop = proportion untouched by fishing (i.e. 1 - availability) */
		k_patchcover = k_cover[nf][bm->current_box] / (p_fished + small_num);
		pop_prop = p_fished * exp(-k_patchcover * log(1.0 + q * k_pattern) / (k_pattern + small_num)) + 1.0 - p_fished;
		scaler = (1.0 - min(1.0, pow(pop_prop, k_patches)));
		break;
    case ep_habitat_pressure: /* Use Ellis and Pantus (2001, 2014) version of two stage model to determine proportion of population
      fished (as assuming habitat and fishing can be patchy) where
                                   
      prop_removed = 1 - (EP_lambda)^k_patches
                                   
      with
      EP_lambda = EP_d      if EP_beta = 0.0
      EP_lambda = log(1 + EP_beta * EP_d) / EP_beta    if EP_beta != 0
    
      and EP_d = q
                                   
      p_fished = Probability that trawl point is within the population's area
      k_patchcover = level of cover in the patch vs area fished
      k_patches = total number of patches per box. Currently assumed to = 2 (fished and not fished)
      */
      k_patchcover = min(1.0, (k_cover[nf][bm->current_box] / (p_fished + small_num)));
      if (k_pattern == 0) pop_prop = q * k_patchcover;
      else pop_prop = log(1.0 + q * k_patchcover * k_pattern) / k_pattern;
      scaler = (1.0 - min(1.0, pow(pop_prop, k_patches)));
      break;

	default:
		quit("No such habitat patchiness algorithm defined (%d) - value must be between 0 and 1 currently\n", flagsphabitat);
		break;

	}

	/**/
	if (do_debug && bm->newweek) {
		fprintf(llogfp, "Correct for habitat avail - %s by %s in %d flagsphabitat: %d, p_fished: %e, q = %e, areaScaler: %e\n", FunctGroupArray[sp].groupCode,
				FisheryArray[nf].fisheryCode, bm->current_box, flagsphabitat, p_fished, q, scaler);
	}
	/**/

	return scaler;
}


double Get_addlsm_Scale(MSEBoxModel *bm, int guildcase, int nf, double addlsm, double addsigma, int sel_curve, double li) {

	double sel_b, sel_lsm, sel_temp, sel, sel_sigma, sel_ampli;
	double step1, step2, sel_temp1, sel_temp2, sel1, sel2;
	double returnValue = addlsm;

	/* Make sure invertebrates restricted to constant selectivity options */
	if ((FunctGroupArray[guildcase].isVertebrate == FALSE) && (sel_curve > 1)) {
		/* Scale back addlsm if necessary */
		if (addlsm != 0.0) {
			switch (sel_curve) {
			case q_const_id: /* Group specific constant - should be nothing to do here as sel_curve > 1 */
			case q_ageconst_id: /* Constant proportion of age class - - should be nothing to do
			 here as sel_curve > 1 */
			case q_knife_id: /* Knife edge selectivity */
                returnValue = addlsm;
				break;
			case q_logistic_id: /* Dynamic so selectivity based on size, logistic */
				sel_b = bm->FISHERYprms[nf][sel_b_id];
				sel_lsm = bm->FISHERYprms[nf][sel_lsm_id];
				sel_temp = 1.0 / (1.0 + exp(-sel_b * (li - sel_lsm)));
				sel_b = bm->FISHERYprms[nf][sel_b_id] + addsigma;
				sel_lsm = bm->FISHERYprms[nf][sel_lsm_id] + addlsm;
				sel = 1.0 / (1.0 + exp(-sel_b * (li - sel_lsm)));
				returnValue = sel - sel_temp;
				break;
			case q_norm_id: /* Dynamic so selectivity based on size, normal */
				sel_lsm = bm->FISHERYprms[nf][sel_normlsm_id];
				sel_sigma = bm->FISHERYprms[nf][sel_normsigma_id];
				step1 = li - sel_lsm;
				step2 = -(step1 * step1) / (2.0 * sel_sigma * sel_sigma + small_num);
				sel_temp = exp(step2);
				sel_lsm = bm->FISHERYprms[nf][sel_normlsm_id] + addlsm;
				sel_sigma = bm->FISHERYprms[nf][sel_normsigma_id] + addsigma;
				step1 = li - sel_lsm;
				step2 = -(step1 * step1) / (2.0 * sel_sigma * sel_sigma + small_num);
				sel = exp(step2);
				returnValue = sel - sel_temp;
				break;
			case q_lognorm_id: /* Dynamic so selectivity based on size, lognormal */
				sel_lsm = bm->FISHERYprms[nf][sel_lognormlsm_id];
				sel_sigma = bm->FISHERYprms[nf][sel_lognormsigma_id];
				sel_temp = Util_Lognorm_Distrib(sel_lsm, sel_sigma, li);
				sel_lsm = bm->FISHERYprms[nf][sel_lognormlsm_id] + addlsm;
				sel_sigma = bm->FISHERYprms[nf][sel_lognormsigma_id] + addsigma;
				sel = Util_Lognorm_Distrib(sel_lsm, sel_sigma, li);
				returnValue = sel - sel_temp;
				break;
			case q_gamma_id: /* Dynamic so selectivity based on size, gamma */
				sel_lsm = bm->FISHERYprms[nf][sel_gammalsm_id] + small_num;
				sel_sigma = bm->FISHERYprms[nf][sel_gammasigma_id];
				step1 = sel_lsm * sel_lsm + 4.0 * sel_sigma * sel_sigma;
				step2 = (sqrt(step1) - sel_lsm) / 2.0 + small_num;
				sel_temp = pow((li / sel_lsm), (sel_lsm / step2)) * exp((sel_lsm - li) / step2);
				sel_lsm = bm->FISHERYprms[nf][sel_gammalsm_id] + addlsm + small_num;
				sel_sigma = bm->FISHERYprms[nf][sel_gammasigma_id] + addsigma;
				step1 = sel_lsm * sel_lsm + 4.0 * sel_sigma * sel_sigma;
				step2 = (sqrt(step1) - sel_lsm) / 2.0 + small_num;
				sel = pow((li / sel_lsm), (sel_lsm / step2)) * exp((sel_lsm - li) / step2);
				returnValue = sel - sel_temp;
				break;
			case q_bimodal_id: /* Bimodal assymetric "normal" selectivity curve based on size */
				sel_lsm = bm->FISHERYprms[nf][sel_bilsm1_id];
				sel_sigma = bm->FISHERYprms[nf][sel_bisigma_id];
				step1 = li - sel_lsm;
				step2 = -(step1 * step1) / (2.0 * sel_sigma * sel_sigma + small_num);
				sel_temp1 = exp(step2);
				sel_lsm = bm->FISHERYprms[nf][sel_bilsm2_id] + addlsm;
				step1 = li - sel_lsm;
				step2 = -(step1 * step1) / (2.0 * sel_sigma * sel_sigma + small_num);
				sel_temp2 = exp(step2);
				sel_temp = max(sel_temp1, sel_temp2);
				sel_lsm = bm->FISHERYprms[nf][sel_bilsm1_id] + addlsm;
				sel_sigma = bm->FISHERYprms[nf][sel_bisigma_id] + addsigma;
				step1 = li - sel_lsm;
				step2 = -(step1 * step1) / (2.0 * sel_sigma * sel_sigma + small_num);
				sel1 = exp(step2);
				sel_lsm = bm->FISHERYprms[nf][sel_bilsm2_id] + addlsm;
				step1 = li - sel_lsm;
				step2 = -(step1 * step1) / (2.0 * sel_sigma * sel_sigma + small_num);
				sel2 = exp(step2);
				sel = max(sel1, sel2);
				returnValue = sel - sel_temp;
                break;
            case q_binormal_id: /* Bimodal assymetric "normal" selectivity curve based on size */
                sel_lsm = bm->FISHERYprms[nf][sel_bilsm1_id];
                sel_sigma = bm->FISHERYprms[nf][sel_bisigma_id];
                step1 = li - sel_lsm;
                step2 = -(step1 * step1) / (2.0 * sel_sigma * sel_sigma + small_num);
                sel_temp1 = exp(step2);
                sel_lsm = bm->FISHERYprms[nf][sel_bilsm2_id] + addlsm;
                sel_sigma = bm->FISHERYprms[nf][sel_bisigma2_id] + addsigma;
                sel_ampli = bm->FISHERYprms[nf][sel_ampli_id];
                step1 = li - sel_lsm;
                step2 = -(step1 * step1) / (2.0 * sel_sigma * sel_sigma + small_num);
                sel_temp2 = sel_ampli * exp(step2);
                sel_temp = max(sel_temp1, sel_temp2);
                sel_lsm = bm->FISHERYprms[nf][sel_bilsm1_id] + addlsm;
                sel_sigma = bm->FISHERYprms[nf][sel_bisigma_id] + addsigma;
                step1 = li - sel_lsm;
                step2 = -(step1 * step1) / (2.0 * sel_sigma * sel_sigma + small_num);
                sel1 = exp(step2);
                sel_lsm = bm->FISHERYprms[nf][sel_bilsm2_id] + addlsm;
                sel_sigma = bm->FISHERYprms[nf][sel_bisigma2_id] + addsigma;
                sel_ampli = bm->FISHERYprms[nf][sel_ampli_id];
                step1 = li - sel_lsm;
                step2 = -(step1 * step1) / (2.0 * sel_sigma * sel_sigma + small_num);
                sel2 = sel_ampli * exp(step2);
                sel = max(sel1, sel2);
                returnValue = sel - sel_temp;
				break;
			default:
				quit("No such selectivity curve defined (%d) - value must be between 0 and 6 currently\n", sel_curve);
				break;
			}
		}
	}
	return returnValue;
}
double Get_Swept_Area(MSEBoxModel *bm, int nf, double *gear_change_scale) {
	/* Area correct the effort estimate - effort read in as number of events
	 per day per box, now multiply by swept area of each event and
	 divide by area the fishery can access to get it into m-3 so matches biomass units */

	double swept_area, SWEPT_scale;
	/* Calculate swept_area */
	SWEPT_scale = Get_Fishery_Change_Scale(bm, nf, flagchangeSWEPT_id, flagchangeSWEPT_id, SWEPTchange);
	swept_area = bm->FISHERYprms[nf][swept_area_id];

	(*gear_change_scale) *= SWEPT_scale;
	swept_area *= SWEPT_scale;
	return swept_area;
}

double Get_Fisheries_Escapment(MSEBoxModel *bm, int sp, int nf, double li, int do_debug, FILE *llogfp) {
	int flagspescapement;
	double FCescape = 0, Ka_escape, Kb_escape;

	/* Fisheries escapement - those fish that escape alive and so are effectively
	 never caught */
	flagspescapement = (int) (bm->SP_FISHERYprms[sp][nf][flagescapement_id]);
	switch (flagspescapement) {
	case no_escape: /* No escapement all returns are discards and are dead */
		FCescape = 0;
		break;
	case fixed_escape: /* Fixed proportion escape alive */
		FCescape = bm->SP_FISHERYprms[sp][nf][p_escape_id];
		break;
	case size_escape: /* Size based */
		Ka_escape = bm->SP_FISHERYprms[sp][nf][Ka_escape_id];
		Kb_escape = bm->SP_FISHERYprms[sp][nf][Kb_escape_id];
		FCescape = Ka_escape * li + Kb_escape;
		if (FCescape < 0.0)
			FCescape = 0;
		else if (FCescape > 1.0)
			FCescape = 1.0;
		break;
	default:
		quit("No such escapement option defined (%d) - value must be between 0 and 2 currently\n", flagspescapement);
		break;
	}

	/* Check if pressure is now zero, if it is skip to next fishery */

	/**/
	if (do_debug && bm->newweek) {
		fprintf(llogfp, "guildcase: %s, by %s in box%d FCescape: %e, flagspescapement: %d\n", FunctGroupArray[sp].groupCode, FisheryArray[nf].fisheryCode, bm->current_box, FCescape,
				flagspescapement);
	}
	/**/
	return FCescape;
}


/**
 *
 * \brief Check on access to species for each fishery - for use in output of annual age specific values.
 *
 *
 */
double Harvest_How_Much_Fishery_Access(MSEBoxModel *bm, int species, int cohort, int nf, double Wgt) {
    int do_debug = 0;
    int basechrt = (int)floor(cohort / FunctGroupArray[species].numGeneTypes);
    int stage = FunctGroupArray[species].cohort_stage[cohort];
    int sel_curve = (int) (bm->FISHERYprms[nf][selcurve_id]);
    int flagF = (int) (bm->SP_FISHERYprms[species][nf][flagF_id]);
    int flagimposecatch = (int) (bm->SP_FISHERYprms[species][nf][flagimposecatch_id]);
    double gear_change_scale = 1.0;
    double li = Ecology_Get_Size(bm, species, Wgt, basechrt);
    double sel = Get_Catch_Selectivity(bm, species, stage, nf, li, &gear_change_scale, &sel_curve);
    double q = Get_Catchability(bm, species, cohort, stage, nf);
    double q_pos_scale = Get_Positional_Availability(bm, species, nf, do_debug, bm->logFile);
    double habitat_scale = Harvest_Get_Habitat_Availability(bm, species, stage, nf, q, do_debug, bm->logFile);
    double ans;
    
    if (FunctGroupArray[species].isCultured) {
        ans = 1.0;
    } else if (flagimposecatch) {
        if (FunctGroupArray[species].groupAgeType == AGE_STRUCTURED || FunctGroupArray[species].groupAgeType == AGE_STRUCTURED_BIOMASS)
            ans = bm->CatchTS_agedistrib[nf][species][basechrt];
        else
            ans = 1.0;
    } else if (flagF) {
        if( bm->flag_sel_with_mFC) {
            sel = Get_Selectivity(bm, species, stage, nf, li, sel_curve, 0.0, 0.0);
            ans = 1.0;
        } else {
            ans = 1.0;
            
        }
    } else {
        ans = sel * q * q_pos_scale * habitat_scale;
    }
    
    /*
    if((species == 2) && (nf == 2))
        fprintf(bm->logFile, "How_Much_Access: %s-%d (stage %d) fishery %d Wgt: %e li: %e selcurve: %d sel: %e q: %e q_pos_scale: %e habitat_scale: %e\n", FunctGroupArray[species].groupCode, cohort, stage, nf, Wgt, li, sel_curve, sel, q, q_pos_scale, habitat_scale);
    */
    
    return ans;
}
