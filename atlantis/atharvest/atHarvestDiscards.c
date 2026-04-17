/**
 * \file atHarvestDiscards.c
 * \brief Functions used to calculate the level of discarding by each fishery.
 * \ingroup atHarvestLib
 *
 *  \author: Bec Gorton and Beth Fulton 20/01/2010
 */


/*  Open library routines */
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <math.h>
#include <atHarvest.h>
#include <atEcologyLib.h>

/**** Routines only called from harvest.c ***************/
static void Do_Dependent_Discards(MSEBoxModel *bm, int impose_catch_depend_discard, int sp, int boxkey_id, int nf, int chrt, double vert_scale,
		double base_catch_sp, double *loadDetFC, double *SPtoFC, FILE *llogfp);
static double Calc_Dependent_Discards_For_Species(MSEBoxModel *bm, int sp, int boxkey_id, int nf, int chrt,
		double vert_scale, FILE *llogfp);
static double Calc_Dependent_Discards_For_Other_Species(MSEBoxModel *bm, int i, int ichrt, int sp, int chrt, int nf, double vert_scale,
		double base_catch_sp, FILE *llogfp);

/*
 * Prototypes of static 'private' functions local to this files
 */
static double Get_Imposed_Discards(MSEBoxModel *bm, int guildcase, int chrt, int stage, int nf, int this_stock, int do_debug, int do_debug_dis, int flagimposecatch, double li, double vert_scale, double Biom, int boxkey_id, double loadFC, double tsDetFC, double loadDetFC, double *SPtoFC, double mpa_losses, FILE *llogfp);


/**
 * \brief Get the fished cohort discard for the given fishery.
 *
 * @returns: The FFCDR value for the given cohort of the given species in the given fishery.
 *
 * This is called by the economics code during quota calculations.
 */
double Harvest_Get_Fixed_Cohort_Discard(int sp, int nf, int chrt, FILE *llogfp)
{
	double loadDetFC;

	/* Fixed proportion per cohort discarded */
	loadDetFC = FFCDR[sp][nf][chrt];

	return loadDetFC;
}


/**
 * \brief Calculate the biomass that should be discarded in this layer for this guild cohort by the given fishery.
 *
 *
 *
 *
 */
double Get_Discards(MSEBoxModel *bm, int guildcase, int chrt, int stage, int nf, int do_debug, int do_debug_econ, int do_debug_dis, int flagimposecatch, int boxkey_id, double Biom, double li, double vert_scale, double loadFC, double *SPtoFC, double *quota, double *FCwaste, double mpa_losses, int depend_dis, FILE *llogfp) {

	double tsDetFC;
	double discard_scale1, discard_scale2, discard_scale3, thresh_li, FCillegal, waste_scale;
	double loadDetFC = 0;
	double age_scale_catch, active_scale, tsEvaled;
	double saleprice, maxsaleprice;
	double catch_step = 0.0;
	int flagchangeDISCRD, DISCRD_num_changes, i, now_change = 0, mustkeep, flagspdiscard;
	int ts_id;
	double ts_dtscale = 86400.0 / bm->dt; // As timeseries generating code assumes timestep of 86400 timestep
	int flagmanage = (int) (bm->FISHERYprms[nf][flagmanage_id]);
	int this_stock, desired_chrt, larger_extant;
	int impose_catch_depend_discard = 0;
	int basechrt = chrt / FunctGroupArray[guildcase].numGeneTypes;

	//double loadDetFCorig, discard_change_scale
	*quota = 0.0;
//	if ((bm->debug == debug_discard) && ((guildcase == bm->which_check) || (bm->which_check >= not_tracking_flux)) && ((bm->current_box == bm->checkbox)
//			|| (bm->checkbox > bm->nbox)) && (bm->dayt >= bm->checkstart) && (bm->dayt <= bm->checkstop)) {
//		do_debug_dis = 1;
//	} else
//		do_debug_dis = 0;
    
	if(do_debug_dis){
		fprintf(llogfp, "Time %e %s by %s Calculating Discard - %s:%d. Biom = %.20e, SPtoFC = %.20e\n",
			bm->dayt, FunctGroupArray[guildcase].groupCode, FisheryArray[nf].fisheryCode,
			FunctGroupArray[guildcase].groupCode, chrt, Biom, *SPtoFC);
	}

	/* Identify the stock under consideration */
	if (FunctGroupArray[guildcase].isVertebrate == FALSE) {
		/* No stock structure for invert stocks yet */
		this_stock = 0;
	} else {
		/* Vertebrates can have stock structure */
		this_stock = bm->group_stock[guildcase][bm->current_box][bm->current_layer];
	}
    
	/* Check for imposed catch and dependent discards */
	flagspdiscard = (int) (bm->SP_FISHERYprms[guildcase][nf][flagdiscard_id]);
	/* Fisheries discarding - assumes all discards are dead */
	tsDetFC = 0;

	flagchangeDISCRD = (int) (bm->SP_FISHERYprms[guildcase][nf][flagchangeDISCRD_id]);
	DISCRD_num_changes = (int) (bm->SP_FISHERYprms[guildcase][nf][DISCRD_num_changes_id]);
	if (flagchangeDISCRD && (bm->dayt >= DISCRDchange[guildcase][nf][0][start_id]) && (DISCRDchange[guildcase][nf][0][start_id] != 0)) {
		for (i = 0; i < DISCRD_num_changes; i++) {
			if (bm->dayt >= DISCRDchange[guildcase][nf][i][start_id])
				now_change = i;
		}
		discard_scale1 = DISCRDchange[guildcase][nf][now_change][thresh_mult_id];
		discard_scale2 = DISCRDchange[guildcase][nf][now_change][retain_mult_id];
		discard_scale3 = DISCRDchange[guildcase][nf][now_change][discard_mult_id];
        waste_scale = DISCRDchange[guildcase][nf][now_change][waste_mult_id];
	} else {
		discard_scale1 = 1.0;
		discard_scale2 = 1.0;
        discard_scale3 = 1.0;
        waste_scale = 1.0;
	}
	//discard_change_scale = discard_scale1 * discard_scale2 * discard_scale3;

	/* Check to see if allowed to discard at all */
	if (bm->FISHERYprms[nf][landallTAC_sp_id] && bm->SP_FISHERYprms[guildcase][nf][flagquota_id])
		mustkeep = 1;
	else
		mustkeep = 0;

	/* Discarding takes into account whether its targeted fishing or bycatch and whether
	 any is kept as byproduct */
	thresh_li = 0;
	FCillegal = 0;
	impose_catch_depend_discard = 0;

	/*
	 *  What proportion of the catch is thrown over board
	 *
	 */
	switch (flagspdiscard) {
	case fixed_discard: /* Fixed proportion discarded */
		loadDetFC = bm->SP_FISHERYprms[guildcase][nf][FFCDR_id] * discard_scale3;
		break;
	case fixedage_discard: /* Fixed proportion per cohort discarded */
		loadDetFC = FFCDR[guildcase][nf][chrt] * discard_scale3;

		/**
		if((do_debug || do_debug_dis) && bm->newweek){
			fprintf(llogfp, "Time: %e, %d-%d %s-%d %s loadDetFC initial: %e (with FFCDR: %e, discard_scale: %e)\n", bm->dayt, bm->current_box,
					bm->current_layer, FunctGroupArray[guildcase].groupCode, basechrt, FisheryArray[nf].fisheryCode, loadDetFC, FFCDR[guildcase][nf][chrt], discard_scale3);
		}
		**/

		break;
	case size_discard: /* Size based */
		/* Convert model weight (mg AFDSW) into g wet weight and then length in cm */
		thresh_li = bm->SP_FISHERYprms[guildcase][nf][FCthreshli_id] * discard_scale1;
		FCillegal = bm->SP_FISHERYprms[guildcase][nf][k_retain_id] * discard_scale2;
		if (li < thresh_li) /* Discard under size */
			loadDetFC = (1.0 - FCillegal);
		else
			loadDetFC = 0.0;
		break;
	case impose_discard: /* Read from time series */
		boxkey_id = bm->BoxKeyMap[bm->current_box][discardkey_id];

		if (boxkey_id > -1) {
			FisheryTimeSeries *this_tsDiscard = &tsDiscard[boxkey_id];
			ts_id = tsdiscardid[guildcase];

			/* Load size of discards to impose - currently assume one dump for all
			 fisheries,	FIX - will need to allow for this to be fishery specific
			 (by doing (i*j + i) kind of index allocation) */
			if (FunctGroupArray[guildcase].groupAgeType == AGE_STRUCTURED || FunctGroupArray[guildcase].groupAgeType == AGE_STRUCTURED_BIOMASS)
				age_scale_catch = bm->DiscardTS_agedistrib[nf][guildcase][basechrt];
			else
				age_scale_catch = 1.0;

			/* If always active (vs only diurnally active) then adjust accordingly */
			if (bm->FISHERYprms[nf][fisheriesflagactive_id] == 2)
				active_scale = 86400.0 / bm->dt;
            else {
                active_scale = 1.0;
            }
				

			/* Correct for any mismatch in model and tsgenerator dt */
			active_scale *= ts_dtscale;

			/* Load total discards - assumed to be in mgs-1 */
			if (tsDiscardtype == interpolate_id)
				tsEvaled = tsEval(&this_tsDiscard->ts, ts_id, bm->t);
			else
				tsEvaled = tsEvalEx(&this_tsDiscard->ts, ts_id, bm->t);
			tsDetFC = active_scale * vert_scale * age_scale_catch * tsEvaled / (bm->cell_vol + small_num);
			loadDetFC = tsDetFC / (tsDetFC + loadFC);
		} else {
			if (!tsDiscardwarned) {
				fprintf(llogfp, "expecting imposed catch in box %d (and maybe others), but never got it - check catch timeseries definitions\n",
						bm->current_box);
				tsDiscardwarned++;
			}
			loadDetFC = 0;
		}
		break;
	case depend_discard:
		/* Dependent on catch of other groups - typically used
		 in conjunction with forced catch time series */

		if(flagimposecatch){  // Getting discards of current species due to the catch of other species - used when imposing catch time series
			impose_catch_depend_discard = 1;
			loadDetFC = 1.0;
		} else
			impose_catch_depend_discard = 0; // Getting discard of other species due to the catch of the current species - used for any fishing case

		catch_step = 0.0;
		Do_Dependent_Discards(bm, impose_catch_depend_discard, guildcase, boxkey_id, nf, basechrt, vert_scale, *SPtoFC, &loadDetFC, &catch_step, llogfp);
		*SPtoFC += catch_step;  // In case any extra catch occurs as a result - due to catch of other species

		if((do_debug || do_debug_dis) && bm->newweek){
			fprintf(llogfp,"Time: %e, %d-%d %s-%d %s loadDetFC initial: %e (with SPtoFC = %e)\n",
				bm->dayt, bm->current_box, bm->current_layer, FunctGroupArray[guildcase].groupCode,
				basechrt, FisheryArray[nf].fisheryCode, loadDetFC, *SPtoFC);
            
		}

		impose_catch_depend_discard = 1;
		break;

	default:
		quit("No such discard option defined (%d) - value must be between 0 and 2 currently\n", flagspdiscard);
		break;
	}

	//loadDetFCorig = loadDetFC;
    
	/* Reset if not allowed to discard this group */
	if (mustkeep)
		loadDetFC = 0.0;

	/**/
	if((do_debug || do_debug_dis || do_debug_econ) && bm->newweek){
		fprintf(llogfp, "Time: %e, %d-%d %s-%d %s loadDetFC initial: %e (flagspdiscard = %d)\n", bm->dayt, bm->current_box, bm->current_layer,
				FunctGroupArray[guildcase].groupCode, basechrt, FisheryArray[nf].fisheryCode, loadDetFC, flagspdiscard);
	}
	/**/

	/* If using TAC and exceed TAC for the species then dump all of the catch as
	 discards */
	if (flagmanage > stock_adapt_mgmt) {
		if (FunctGroupArray[guildcase].isTAC) {
			if (bm->TAC_over[guildcase][nf][this_stock] && !mustkeep)
				loadDetFC = 1.0;
		}
	}

	/* Check no discarding more than caught or sucking more in (negative) */
	loadDetFC = min(1.0,max(0.0,loadDetFC));

	/**/
	if((do_debug || do_debug_dis || do_debug_econ) && bm->newweek){
		fprintf(llogfp, "Time: %e, %d-%d %s-%d %s loadDetFC after flagmanage: %e\n", bm->dayt, bm->current_box, bm->current_layer, FunctGroupArray[guildcase].groupCode, basechrt,
				FisheryArray[nf].fisheryCode, loadDetFC);
	}
	/**/
    
	/* If loaded catch from time series calculate total initial catch (landed and discards) */
	if (flagimposecatch) {
		loadDetFC = Get_Imposed_Discards(bm, guildcase, basechrt, stage, nf, this_stock, do_debug, do_debug_dis, flagimposecatch, li, vert_scale, Biom, boxkey_id, loadFC, tsDetFC, loadDetFC, SPtoFC, mpa_losses, llogfp);
	}

	/**/
	if ((do_debug || do_debug_dis) && bm->newweek) {
		fprintf(llogfp, "Time: %e, %d-%d %s-%d %s loadDetFC after carry-over: %e, SPtoFC: %e\n", bm->dayt, bm->current_box, bm->current_layer,
				FunctGroupArray[guildcase].groupCode, basechrt, FisheryArray[nf].fisheryCode, loadDetFC, *SPtoFC);
	}
	/**/

	/* High grade */
	*quota = bm->TACamt[guildcase][nf][now_id] * kg_2_mg / bm->X_CN;
	desired_chrt = (int) (bm->SP_FISHERYprms[guildcase][nf][desired_chrt_id]);
	larger_extant = (int)(bm->SP_FISHERYprms[guildcase][nf][larger_extant_id]);
	if(bm->flaghighgrading && !mustkeep && ((TotCumCatch[guildcase][nf][bm->thisyear] + bm->TotOldCumCatch[guildcase][nf]) > bm->prop_within * *quota) && ((basechrt < desired_chrt) && (larger_extant)) && (!flagimposecatch))
		loadDetFC = 1.0;

	if((do_debug || do_debug_dis || do_debug_econ) && bm->newweek){
		fprintf(llogfp,"Time: %e, %d-%d %s-%d %s loadDetFC after high grading: %e\n",
			bm->dayt, bm->current_box, bm->current_layer, FunctGroupArray[guildcase].groupCode, basechrt, FisheryArray[nf].fisheryCode, loadDetFC);
	}

	/* Check whether allowed to retain the catch (i.e. check if over quota) */
	if (!mustkeep && (bm->TripCatch[guildcase][nf] > bm->SP_FISHERYprms[guildcase][nf][catch_allowed]) && (!flagimposecatch)) {
		loadDetFC = 1.0;
		//fprintf(llogfp,"Time: %e %s has exceeded allowed catch of %s (allowed: %e, TripCatch: %e, nf = %d, guildcase = %d\n",
		//	bm->dayt, FisheryArray[nf].fisheryCode, FunctGroupArray[sp].groupCode, bm->SP_FISHERYprms[guildcase][nf][catch_allowed], bm->TripCatch[sp][nf], nf, guildcase);
	}

	if((do_debug || do_debug_dis || do_debug_econ) && bm->newweek){
		fprintf(llogfp, "Time: %e, %d-%d %s-%d %s loadDetFC after trip limits: %e (tripcatch: %e, catch_allowed: %e)\n", bm->dayt, bm->current_box,
				bm->current_layer, FunctGroupArray[guildcase].groupCode, basechrt, FisheryArray[nf].fisheryCode, loadDetFC, bm->TripCatch[guildcase][nf],
				bm->SP_FISHERYprms[guildcase][nf][catch_allowed]);
	}

	/* Market based discarding */
	saleprice = bm->SP_FISHERYprms[guildcase][nf][saleprice_id];
	maxsaleprice = bm->FISHERYprms[nf][maxsaleprice_id];
	if (bm->flagmarketdiscard && !mustkeep && (saleprice < bm->salethresh * maxsaleprice) && (!flagimposecatch)) {
		loadDetFC = 1.0;
	}

	if((do_debug || do_debug_dis || do_debug_econ) && bm->newweek){
		fprintf(llogfp, "Time: %e, %d-%d %s-%d %s loadDetFC after market-based discarding: %e\n", bm->dayt, bm->current_box, bm->current_layer,
				FunctGroupArray[guildcase].groupCode, basechrt, FisheryArray[nf].fisheryCode, loadDetFC);
	}
    
    /* Guts and other waste that are thrown overboard */
    *FCwaste = bm->SP_FISHERYprms[guildcase][nf][k_waste_id] * waste_scale;

	return loadDetFC;
}



/**
 * Routines to handle catch dependent discarding
 */

void Do_Dependent_Discards(MSEBoxModel *bm, int impose_catch_depend_discard, int guildcase, int boxkey_id, int nf, int chrt, double vert_scale,
		double base_catch_sp, double *loadDetFC, double *SPtoFC, FILE *llogfp)
{
	int i, bcohort, icohort;
    double step1;
	*SPtoFC = 0.0;  // Just in case...

	if(impose_catch_depend_discard){
		*SPtoFC = Calc_Dependent_Discards_For_Species(bm, guildcase, boxkey_id, nf, chrt, vert_scale, llogfp);
		*loadDetFC = 1.0;
	} else {
		for(i=0; i<bm->K_num_tot_sp; i++){
            
            if(bm->FCcocatch[guildcase][i] > 0) {
                for(bcohort=0; bcohort<FunctGroupArray[i].numCohortsXnumGenes; bcohort++){
                    icohort = (int)floor(bcohort / FunctGroupArray[i].numGeneTypes);
                    step1 = Calc_Dependent_Discards_For_Other_Species(bm, i, icohort, guildcase, chrt, nf, vert_scale, base_catch_sp, llogfp);
                    bm->DependDiscardsTot[nf][i][bcohort] += step1;
                }
			}
		}
	}

}

/**
 *  Routine to calculate discards of the current species due to catch of other species - when imposing catch time series
 */

double Calc_Dependent_Discards_For_Species(MSEBoxModel *bm, int sp, int boxkey_id, int nf, int chrt, double vert_scale, FILE *llogfp)
{
	int guildcase = sp;
	int ts_id = 0;
	double age_scale_catch = 1.0;
	double active_scale = 1.0;
	double distrib_scale = 1.0;
	double tsEvaled = 0.0;
	double SPtoFC = 0.0;
	int i;
	int basechrt = chrt / FunctGroupArray[guildcase].numGeneTypes;

	/* Load size of discards to impose - currently assume one dump for all
	fisheries,	FIX - will need to allow for this to be fishery specific
	(by doing (i*j + i) kind of index allocation) */
	if(FunctGroupArray[sp].groupAgeType == AGE_STRUCTURED || FunctGroupArray[sp].groupAgeType == AGE_STRUCTURED_BIOMASS)
	//if((sp < bm->K_num_vert_sp) || (sp == CEPguild_id) || (sp == PWNguild_id))
		age_scale_catch = bm->DiscardTS_agedistrib[nf][guildcase][basechrt];
	else
		age_scale_catch = 1.0;

	/* If always active (vs only diurnally active) then adjust accordingly
	- should really be whether active fisheries (i.e. target species to be
	checked has fishery that is dirunal or not, but for now this will do
	as all catch in time series grouped per species so aggregate answers
	are fine so long as take account of activity of "bycatch species fishery"
	doing the calling here - i.e. if it calls twice then must halve the result */
	if(bm->FISHERYprms[nf][fisheriesflagactive_id] == 2)
		active_scale = 86400.0 / bm->dt;
    else {
        active_scale = 1.0;
        if (bm->newmonth && (bm->dt < 43200.0 ))
            warn("Imposing discards but dt %e doesn't match assumptions of 12-24 hours per timestep\n", bm->dt);
    }

	for(i=0; i<bm->K_num_tot_sp; i++){
		if(FunctGroupArray[i].isFished == TRUE){
			if(bm->FCcocatch[sp][i] > 0){
				if(boxkey_id > -1){
					FisheryTimeSeries *this_tsCatch = &tsCatch[boxkey_id];
					ts_id = tscatchid[i];

					/* Load catch of target species - assumed to be in mgs-1 */
					if(tsCatchtype == interpolate_id)
						tsEvaled = tsEval(&this_tsCatch->ts, ts_id, bm->t);
					else
						tsEvaled = tsEvalEx(&this_tsCatch->ts, ts_id, bm->t);

					switch(bm->flagimposeglobal){
					case no_impose:
						distrib_scale = 0.0;
						break;
					case spatial_impose:
					case stock_impose:
					case adjacent_impose:
						/* Spatially explicit time series loaded so nothing to do here */
						distrib_scale = 1.0;
						break;
					case global_impose:
						/* Global time series loaded and then corrected by current spatial distribution */
						distrib_scale = bm->targetspbiom[i][bm->current_box] / (bm->totbiom[i] + small_num);
						break;
					default:
						quit("Fishing for %s, no such case (%d) for flagimposeglobal - reset to 0 to 4\n", FunctGroupArray[i].groupCode, bm->flagimposeglobal);
						break;
					}

					// bm->FCcocatch[spcat][i] used here as worried about catch of sp (spcat) due to catch of the other i species
					// Assuming under this parameter selection that the spcat column of the array covers the dependent catch, with the row
					// being defined for the ith species
					SPtoFC += bm->FCcocatch[sp][i] * distrib_scale * active_scale * age_scale_catch * vert_scale * tsEvaled / (bm->cell_vol + small_num);

				}
			}
		}
	}

	return SPtoFC;
}

/**
 *  When getting dicards of other species dependent on catch of current species
 */
double Calc_Dependent_Discards_For_Other_Species(MSEBoxModel *bm, int i, int ichrt, int sp, int chrt, int nf, double vert_scale,
		double base_catch_sp, FILE *llogfp)
{
	double SPtoFC = 0.0;
	double age_scale_catch = 1.0;
	//int spcat = bm->SPtoCATid[i];
	int ibasechrt = ichrt / FunctGroupArray[i].numGeneTypes;

	/* Load size of discards to impose - currently assume one dump for all
	fisheries,	FIX - will need to allow for this to be fishery specific
	(by doing (i*j + i) kind of index allocation) */
	if(FunctGroupArray[i].groupAgeType == AGE_STRUCTURED || FunctGroupArray[i].groupAgeType == AGE_STRUCTURED_BIOMASS)
	//if((i < bm->K_num_vert_sp) || (i == CEPguild_id) || (i == PWNguild_id))
		age_scale_catch = bm->DiscardTS_agedistrib[nf][i][ibasechrt];
	else
		age_scale_catch = 1.0;

	/* Get amount of discards of the bycatch species i due to the catch of species sp */
	// bm->FCcocatch[sp][spcat] used here as worried about catch of spcat due to catch of species sp and
	// assuming under this parameter selection that the sp row defines the catches of all the other species (spcat)
	// due to the catch of sp
	SPtoFC = bm->FCcocatch[sp][i] * base_catch_sp * age_scale_catch;
    
	return SPtoFC;

}

/**
 * \brief Get the imposed discards for the given chrt of the functional groups sp by the fishery nf.
 *
 */

double Get_Imposed_Discards(MSEBoxModel *bm, int guildcase, int chrt, int stage, int nf, int this_stock, int do_debug, int do_debug_dis, int flagimposecatch,
		double li, double vert_scale, double Biom, int boxkey_id, double loadFC, double tsDetFC, double loadDetFC, double *SPtoFC, double mpa_losses,
		FILE *llogfp) {

	double step1, step2, FCdiff, step3, biomCarryOver = 0.0, disCarryOver = 0.0;
    double age_scale_discard = 1.0, age_scale_catch = 1.0;
	int flagspdiscard, adbox, chkbox;

	flagspdiscard = (int) (bm->SP_FISHERYprms[guildcase][nf][flagdiscard_id]);

	if (flagspdiscard == impose_discard) {
		*SPtoFC = loadFC + tsDetFC;
	} else {
		*SPtoFC = loadFC / (1.0 - loadDetFC + small_num);
	}

    if (FunctGroupArray[guildcase].numCohortsXnumGenes > 1) {
        age_scale_discard = bm->DiscardTS_agedistrib[nf][guildcase][chrt];
        age_scale_catch = bm->CatchTS_agedistrib[nf][guildcase][chrt];
    }
    
	if (do_debug) {
		fprintf(llogfp,"Get_Imposed_Discards - sp %d:%d stage = %d, fishery %s, li = %e, vert_scale = %e, Biom = %e, boxkey_id = %d, loadFC = %e, loadFC = %e, loadDetFC = %e, SPtoFC = %e\n", guildcase, chrt, stage, FisheryArray[nf].fisheryCode, li, vert_scale, Biom, boxkey_id, loadFC, tsDetFC, loadDetFC, *SPtoFC);
	}
	if (!(_finite(*SPtoFC))) {
		fprintf(llogfp, "Time: %e, %d-%d %s-%d %s SPtoFC: %e, loadFC: %e, loadDetFC: %e\n", bm->dayt, bm->current_box, bm->current_layer,
				FunctGroupArray[guildcase].groupCode, chrt, FisheryArray[nf].fisheryCode, *SPtoFC, loadFC, loadDetFC);
	}

	/* Check whether enough biomass present to be caught and discarded */
	if (*SPtoFC > Biom) {
		/* Catch vs Available Biomass */
		if (loadFC > Biom) {

			FCdiff = loadFC - Biom;
			bm->FCtsCarryOver[bm->current_box][tscocatch_id][guildcase] += FCdiff * bm->cell_vol;
			bm->FCtsCarryOver[bm->current_box][tscodiscard_id][guildcase] += loadDetFC * *SPtoFC * bm->cell_vol;
			bm->FCtsCarryOver[bm->nbox][tscocatch_id][guildcase] += FCdiff * bm->cell_vol;
			bm->FCtsCarryOver[bm->nbox][tscodiscard_id][guildcase] += loadDetFC * *SPtoFC * bm->cell_vol;
			bm->FCtsCarryOver[bm->nbox + this_stock + 1][tscocatch_id][guildcase] += FCdiff * bm->cell_vol;
			bm->FCtsCarryOver[bm->nbox + this_stock + 1][tscodiscard_id][guildcase] += loadDetFC * *SPtoFC * bm->cell_vol;

			/**
			if (do_debug || do_debug_dis) {
            //if (!chrt && (*SPtoFC > 0)) {
                fprintf(llogfp, "Time: %e, %s-%d box%d reset loadDetFC = 0, cOver %e catch (now %e stock%dv %e) %e discards (now %e stock%dv %e) age_scale_catch: %e\n", bm->dayt,
						FunctGroupArray[guildcase].groupCode, chrt, bm->current_box, FCdiff, bm->FCtsCarryOver[bm->current_box][tscocatch_id][guildcase], this_stock,
						bm->FCtsCarryOver[bm->nbox + this_stock + 1][tscocatch_id][guildcase], loadDetFC * (*SPtoFC),
						bm->FCtsCarryOver[bm->current_box][tscodiscard_id][guildcase], this_stock,
						bm->FCtsCarryOver[bm->nbox + this_stock + 1][tscodiscard_id][guildcase], age_scale_catch);
			}
			**/

			/* All available biomass is caught */
			*SPtoFC = Biom;
			/* No discards possible */
			loadDetFC = 0.0;

		} else {
			/* Sufficient Biomass for Catch, but too much discards */
			FCdiff = *SPtoFC - Biom;
			bm->FCtsCarryOver[bm->current_box][tscodiscard_id][guildcase] += FCdiff * bm->cell_vol;
			bm->FCtsCarryOver[bm->nbox][tscodiscard_id][guildcase] += FCdiff * bm->cell_vol;
			bm->FCtsCarryOver[bm->nbox + this_stock + 1][tscodiscard_id][guildcase] += FCdiff * bm->cell_vol;

			/* Reset discards */
			step1 = loadDetFC * (*SPtoFC) - FCdiff;
			*SPtoFC = Biom;
			loadDetFC = step1 / ((*SPtoFC) + small_num);

			/**
			if (do_debug || do_debug_dis) {
            //if (!chrt && (*SPtoFC > 0)) {
                fprintf(llogfp, "Time: %e, %s-%d box%d reducing loadDetFC to %e, cOver %e discards (now %e or stock%dv: %e) age_scale_catch: %e\n", bm->dayt,
						FunctGroupArray[guildcase].groupCode, chrt, bm->current_box, loadDetFC, FCdiff, bm->FCtsCarryOver[bm->current_box][tscodiscard_id][guildcase], this_stock, bm->FCtsCarryOver[bm->nbox + this_stock + 1][tscodiscard_id][guildcase], age_scale_catch);
			}
			**/
		}
	}

	/* Check if must account for displacement due to an MPA */
	if (mpa_losses > 0) {
		bm->FCtsCarryOver[bm->current_box][tscocatch_id][guildcase] += mpa_losses * bm->cell_vol;
		bm->FCtsCarryOver[bm->current_box][tscodiscard_id][guildcase] += loadDetFC * (*SPtoFC) * bm->cell_vol;
		bm->FCtsCarryOver[bm->nbox][tscocatch_id][guildcase] += mpa_losses * bm->cell_vol;
		bm->FCtsCarryOver[bm->nbox][tscodiscard_id][guildcase] += loadDetFC * (*SPtoFC) * bm->cell_vol;
		bm->FCtsCarryOver[bm->nbox + this_stock + 1][tscocatch_id][guildcase] += mpa_losses * bm->cell_vol;
		bm->FCtsCarryOver[bm->nbox + this_stock + 1][tscodiscard_id][guildcase] += loadDetFC * (*SPtoFC) * bm->cell_vol;
	}

	/* Check to see whether there is any carry over to deal with */
    if (!age_scale_discard && !age_scale_catch)  // This size isn't caught so it can't be discarded
        return 0;
    
    switch (flagimposecatch) {
	case spatial_impose:
		biomCarryOver = bm->FCtsCarryOver[bm->current_box][tscocatch_id][guildcase] / (bm->cell_vol + small_num);
		disCarryOver = bm->FCtsCarryOver[bm->current_box][tscodiscard_id][guildcase] / (bm->cell_vol + small_num);
		break;
	case global_impose:
		biomCarryOver = bm->FCtsCarryOver[bm->nbox][tscocatch_id][guildcase] / (bm->cell_vol + small_num);
		disCarryOver = bm->FCtsCarryOver[bm->nbox][tscodiscard_id][guildcase] / (bm->cell_vol + small_num);
		break;
	case stock_impose:
		biomCarryOver = bm->FCtsCarryOver[bm->nbox + this_stock + 1][tscocatch_id][guildcase] / (bm->cell_vol + small_num);
		disCarryOver = bm->FCtsCarryOver[bm->nbox + this_stock + 1][tscodiscard_id][guildcase] / (bm->cell_vol + small_num);
		break;
	case adjacent_impose:
		biomCarryOver = 0;
		disCarryOver = 0;

		biomCarryOver += bm->FCtsCarryOver[bm->current_box][tscocatch_id][guildcase] / (bm->cell_vol + small_num);
		disCarryOver += bm->FCtsCarryOver[bm->current_box][tscodiscard_id][guildcase] / (bm->cell_vol + small_num);
		for (adbox = 0; adbox < bm->boxes[bm->current_box].nconn; adbox++) {
			chkbox = bm->boxes[bm->current_box].ibox[adbox];
			checkedBox[chkbox] = 0;
		}
		for (adbox = 0; adbox < bm->boxes[bm->current_box].nconn; adbox++) {
			chkbox = bm->boxes[bm->current_box].ibox[adbox];
			if (!checkedBox[chkbox]) {
				if (bm->boxes[chkbox].type != BOUNDARY) {
					biomCarryOver += bm->FCtsCarryOver[chkbox][tscocatch_id][guildcase] / (bm->cell_vol + small_num);
					disCarryOver += bm->FCtsCarryOver[chkbox][tscodiscard_id][guildcase] / (bm->cell_vol + small_num);
				}
				checkedBox[chkbox] = 1;
			}
		}
		break;
	default:
		quit("No such case (%d) for flagimposecatch for %s - reset to 0, 1 or 2\n", flagimposecatch, FunctGroupArray[guildcase].groupCode);
		break;
	}
    biomCarryOver *= age_scale_catch;
    disCarryOver *= age_scale_discard;

	if (!(_finite((*SPtoFC)))) {
		fprintf(llogfp, "Time: %e, %d-%d %s-%d %s SPtoFC: %e, Biom: %e\n", bm->dayt, bm->current_box, bm->current_layer, FunctGroupArray[guildcase].groupCode, chrt, FisheryArray[nf].fisheryCode, (*SPtoFC), Biom);
	}

	if (biomCarryOver > 0) {
		step1 = Biom - (*SPtoFC);
		/****/
		if (do_debug) {
			fprintf(llogfp, "Time %e %s-%d box%d %e catch to catchup from spare %e biom\n", bm->dayt, FunctGroupArray[guildcase].groupCode, chrt, bm->current_box, biomCarryOver, step1);
		}
		/**/

		/* Can only take extra if there is anything to take in the first place */
		if (step1 > 0) {
			/* More extra catch than available biomass */
			if (biomCarryOver > step1) {
				/* Calculate the amount of extra that can be soaked up
				 by the available biomass */
				loadFC += step1;
				step2 = loadDetFC * (*SPtoFC);
				(*SPtoFC) += step1;
				loadDetFC = step2 / ((*SPtoFC) + small_num);
				/* Reset stored values of extra catch */
				if (flagimposecatch != adjacent_impose) {
					/** Update local, global or stock level cases **/
					bm->FCtsCarryOver[bm->current_box][tscocatch_id][guildcase] -= step1 * bm->cell_vol;
					bm->FCtsCarryOver[bm->nbox][tscocatch_id][guildcase] -= step1 * bm->cell_vol;
					bm->FCtsCarryOver[bm->nbox + this_stock + 1][tscocatch_id][guildcase] -= step1 * bm->cell_vol;

					/* Reset values */
					if (bm->FCtsCarryOver[bm->nbox][tscocatch_id][guildcase] < 0)
						bm->FCtsCarryOver[bm->nbox][tscocatch_id][guildcase] = 0;
					if (bm->FCtsCarryOver[bm->nbox + this_stock + 1][tscocatch_id][guildcase] < 0)
						bm->FCtsCarryOver[bm->nbox + this_stock + 1][tscocatch_id][guildcase] = 0;
					if (bm->FCtsCarryOver[bm->current_box][tscocatch_id][guildcase] < 0)
						bm->FCtsCarryOver[bm->current_box][tscocatch_id][guildcase] = 0;

					/**/
					if (!(_finite((*SPtoFC)))) {
						fprintf(llogfp, "Time %e %s-%d box%d caught up %e catch (cOver now %e gcOver: %e stock%dOver: %e)\n", bm->dayt, FunctGroupArray[guildcase].groupCode,
								chrt, bm->current_box, step1, bm->FCtsCarryOver[bm->current_box][tscocatch_id][guildcase],
								bm->FCtsCarryOver[bm->nbox][tscocatch_id][guildcase], this_stock,
								bm->FCtsCarryOver[bm->nbox + this_stock + 1][tscocatch_id][guildcase]);
					}
					/**/
				} else {
					/** Update case when checking adjacent cells **/
					/* First update global record */
					bm->FCtsCarryOver[bm->nbox][tscocatch_id][guildcase] -= step1 * bm->cell_vol;

					/* Update current cell */
					bm->FCtsCarryOver[bm->current_box][tscocatch_id][guildcase] -= step1 * bm->cell_vol
							* bm->FCtsCarryOver[bm->current_box][tscocatch_id][guildcase] / (biomCarryOver + small_num);
					if (bm->FCtsCarryOver[bm->current_box][tscocatch_id][guildcase] < 0)
						bm->FCtsCarryOver[bm->current_box][tscocatch_id][guildcase] = 0;
					/* Update adjacent cells */
					for (adbox = 0; adbox < bm->boxes[bm->current_box].nconn; adbox++) {
						chkbox = bm->boxes[bm->current_box].ibox[adbox];
						checkedBox[chkbox] = 0;
					}
					for (adbox = 0; adbox < bm->boxes[bm->current_box].nconn; adbox++) {
						chkbox = bm->boxes[bm->current_box].ibox[adbox];
						if (!checkedBox[chkbox]) {
							bm->FCtsCarryOver[chkbox][tscocatch_id][guildcase] -= step1 * bm->cell_vol * bm->FCtsCarryOver[chkbox][tscocatch_id][guildcase]
									/ (biomCarryOver + small_num);
							if (bm->FCtsCarryOver[chkbox][tscocatch_id][guildcase] < 0)
								bm->FCtsCarryOver[chkbox][tscocatch_id][guildcase] = 0;
							checkedBox[chkbox] = 1;

							/**
							 if(do_debug){
							 fprintf(llogfp, "Time %e %s-%d box%d caught up %e catch (cOver now %e)\n",
							 bm->dayt, FunctGroupArray[sp].groupCode, chrt, chkbox, step1, bm->FCtsCarryOver[chkbox][tscocatch_id][guildcase]);
							 }
							 **/
						}
					}

					/**
					 if(do_debug){
					 fprintf(llogfp, "Time %e %s-%d box%d caught up %e catch (cOver now %e gcOver: %e stock%dOver: %e)\n",
					 bm->dayt, FunctGroupArray[sp].groupCode, chrt, bm->current_box, step1, bm->FCtsCarryOver[bm->current_box][tscocatch_id][guildcase], bm->FCtsCarryOver[bm->nbox][tscocatch_id][guildcase]);
					 }
					 **/

				}
			} else {
				/* Enough available biomass to soak up all extra catch */
				/* Calculate amount leftover for extra discards */
				step2 = step1 - biomCarryOver;
				/* Reset catch and discard amounts*/
				loadFC += biomCarryOver;
				step3 = min(disCarryOver, step2);
				tsDetFC += step3;
				step1 = loadDetFC * (*SPtoFC) + step3;
				(*SPtoFC) += biomCarryOver + step3;
				loadDetFC = step1 / ((*SPtoFC) + small_num);
				/* Reset stored values of extra catch and discards */

				if (flagimposecatch != adjacent_impose) {
					/** Update local, global or stock level cases **/
					bm->FCtsCarryOver[bm->nbox][tscocatch_id][guildcase] -= biomCarryOver * bm->cell_vol;
					bm->FCtsCarryOver[bm->nbox + this_stock + 1][tscocatch_id][guildcase] -= biomCarryOver * bm->cell_vol;
					bm->FCtsCarryOver[bm->current_box][tscocatch_id][guildcase] -= biomCarryOver * bm->cell_vol;
					bm->FCtsCarryOver[bm->nbox][tscodiscard_id][guildcase] -= step3 * bm->cell_vol;
					bm->FCtsCarryOver[bm->nbox + this_stock + 1][tscodiscard_id][guildcase] -= step3 * bm->cell_vol;
					bm->FCtsCarryOver[bm->current_box][tscodiscard_id][guildcase] -= step3 * bm->cell_vol;

					/**/
					assert((_finite((*SPtoFC))));
					if (!(_finite((*SPtoFC)))) {
						fprintf(
								llogfp,
								"Time %e %s-%d box%d caught up %e catch (cOver now %e and gcOver: %e and stock%dOver) and %e discards (now %e and %e and %e), %e\n",
								bm->dayt, FunctGroupArray[guildcase].groupCode, chrt, bm->current_box, biomCarryOver,
								bm->FCtsCarryOver[bm->current_box][tscocatch_id][guildcase], bm->FCtsCarryOver[bm->nbox][tscocatch_id][guildcase], this_stock,
								bm->FCtsCarryOver[bm->nbox + this_stock + 1][tscocatch_id][guildcase], step3,
								bm->FCtsCarryOver[bm->current_box][tscodiscard_id][guildcase], bm->FCtsCarryOver[bm->nbox][tscodiscard_id][guildcase],
								bm->FCtsCarryOver[bm->nbox + this_stock + 1][tscodiscard_id][guildcase]);
					}
					/**/

					/* Reset values */
					if (bm->FCtsCarryOver[bm->nbox][tscocatch_id][guildcase] < 0)
						bm->FCtsCarryOver[bm->nbox][tscocatch_id][guildcase] = 0;
					if (bm->FCtsCarryOver[bm->nbox + this_stock + 1][tscocatch_id][guildcase] < 0)
						bm->FCtsCarryOver[bm->nbox + this_stock + 1][tscocatch_id][guildcase] = 0;
					if (bm->FCtsCarryOver[bm->current_box][tscocatch_id][guildcase] < 0)
						bm->FCtsCarryOver[bm->current_box][tscocatch_id][guildcase] = 0;
					if (bm->FCtsCarryOver[bm->nbox][tscodiscard_id][guildcase] < 0)
						bm->FCtsCarryOver[bm->nbox][tscodiscard_id][guildcase] = 0;
					if (bm->FCtsCarryOver[bm->nbox + this_stock + 1][tscodiscard_id][guildcase] < 0)
						bm->FCtsCarryOver[bm->nbox + this_stock + 1][tscodiscard_id][guildcase] = 0;
					if (bm->FCtsCarryOver[bm->current_box][tscodiscard_id][guildcase] < 0)
						bm->FCtsCarryOver[bm->current_box][tscodiscard_id][guildcase] = 0;

				} else {
					/** Update case when checking adjacent cells **/
					/* First update global record */
					bm->FCtsCarryOver[bm->nbox][tscocatch_id][guildcase] -= biomCarryOver * bm->cell_vol;
					bm->FCtsCarryOver[bm->nbox][tscodiscard_id][guildcase] -= step3 * bm->cell_vol;

					/* Update current cell */
					bm->FCtsCarryOver[bm->current_box][tscocatch_id][guildcase] = 0;
					bm->FCtsCarryOver[bm->current_box][tscodiscard_id][guildcase] -= step3 * bm->cell_vol
							* bm->FCtsCarryOver[bm->current_box][tscodiscard_id][guildcase] / (disCarryOver + small_num);
					if (bm->FCtsCarryOver[bm->current_box][tscocatch_id][guildcase] < 0)
						bm->FCtsCarryOver[bm->current_box][tscocatch_id][guildcase] = 0;
					if (bm->FCtsCarryOver[bm->current_box][tscodiscard_id][guildcase] < 0)
						bm->FCtsCarryOver[bm->current_box][tscodiscard_id][guildcase] = 0;
					/* Update adjacent cells */
					for (adbox = 0; adbox < bm->boxes[bm->current_box].nconn; adbox++) {
						chkbox = bm->boxes[bm->current_box].ibox[adbox];
						checkedBox[chkbox] = 0;
					}
					for (adbox = 0; adbox < bm->boxes[bm->current_box].nconn; adbox++) {
						chkbox = bm->boxes[bm->current_box].ibox[adbox];
						if (!checkedBox[chkbox]) {
							bm->FCtsCarryOver[chkbox][tscocatch_id][guildcase] = 0;
							bm->FCtsCarryOver[chkbox][tscodiscard_id][guildcase] -= step3 * bm->cell_vol * bm->FCtsCarryOver[chkbox][tscodiscard_id][guildcase] / (disCarryOver + small_num);
							if (bm->FCtsCarryOver[chkbox][tscocatch_id][guildcase] < 0)
								bm->FCtsCarryOver[chkbox][tscocatch_id][guildcase] = 0;
							if (bm->FCtsCarryOver[chkbox][tscodiscard_id][guildcase] < 0)
								bm->FCtsCarryOver[chkbox][tscodiscard_id][guildcase] = 0;
							checkedBox[chkbox] = 1;
						}
					}
				}
			}
		}
	}
	return loadDetFC;
}


