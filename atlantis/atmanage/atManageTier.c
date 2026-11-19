/**
 \file atManageTier.c
 \brief C file for running the tiers for the harvest control rules
 \ingroup atManageLib
 \author Beth Fulton 19/5/2013

  Based on Assessment.CPP from Sally Wayte

  From Natalie Dowling's paper on tiered harvest strategies:

  Table 2
Tier 	Description																		Coded
 0 		Robust assessment of F and B based on fishery dependent AND independent data     No
 1 		Robust assessment of F and B based on fishery dependent data ONLY				 Yes
 2 		Assessment of F and B based on fishery dependent and/or fishery independ. data	 Yes
 3 		Empirical estimates of F based on size and/or age data						  	 Yes - Catch curves, F
 4 		Empirical estimates of
			(a) relative biomass based on fishery dependent data						 Yes - CPUE based
			(b) within season changes to relative biomass based on fishery depend. data
			(c) relative biomass based on fishery independent surveys
 5 		Empirical estimates of F based on spatial distribution of effort relative		 Yes - surplus production, average length
 	 	 	 to species distribution
 6 		No estimate of biomass and F; use of fishery dependent sp. specific triggers 	 No
 7 		No estimate of biomass and F; use of fishery dependent triggers for				 No
  	  	  	 groups of species

************************************************************************************************/

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>
#include <sys/stat.h>
#include <unistd.h>
#include "atManage.h"
#include "atHarvestLib.h"
#include <time.h>

#include "atSS3LinkLib.h"

/* Prototypes */
void Averaged_RBC(MSEBoxModel *bm, int species, int year, FILE *llogfp);
void Ratpack_Averaged_RBC(MSEBoxModel *bm, int species, int year, FILE *llogfp);
double Apply_MYTAC_Decay(MSEBoxModel *bm, int species, int year, double TACbase, FILE *llogfp);
void getF20_40_48(MSEBoxModel *bm, int species, double *agesel, double *wt, double *mat, double natM, double hsteep, FILE *llogfp);
void get_T5_F20_40_48(MSEBoxModel *bm, int species, double bstart, double k, double r, FILE *llogfp);
void newTier3Rule(MSEBoxModel *bm, int species, int year, double current_catch, FILE *llogfp, FILE *fid);
void newTierFour(MSEBoxModel *bm, int species, int year, double current_catch, FILE *llogfp);
void Tier_averages(MSEBoxModel *bm, int species, int year, int tier, double *current_catch, double *discard_rate, FILE *llogfp);
void TierOne(MSEBoxModel *bm, int species, int year, FILE *llogfp);
void TierOneMimic(MSEBoxModel *bm, int species, int year, FILE *llogfp);
void TierThree_CC(MSEBoxModel *bm, int species, int year, FILE *llogfp);
void TierThree_CCsel(MSEBoxModel *bm, int species, int year, FILE *llogfp);
void Tier3Rule(MSEBoxModel *bm, int species, int year, double current_catch, FILE *llogfp);
void TierFour(MSEBoxModel *bm, int species, int year, double current_catch, FILE *llogfp);
void TierFive_AvLen(MSEBoxModel *bm, int species, int year, FILE *llogfp);
void TierFive_SP(MSEBoxModel *bm, int species, int year, FILE *llogfp, FILE *fid);
void TierFiveDCAC(MSEBoxModel *bm, int species, int year, double current_catch, FILE *llogfp);
void TierFiveSAFE(MSEBoxModel *bm, int species, int year, double current_catch, FILE *llogfp);
void TierSixRUSS(MSEBoxModel *bm, int species, int year, FILE *llogfp);
void TierSevenRUSS(MSEBoxModel *bm, int species, int year, FILE *llogfp);
double Apply_RBC_Buffers(MSEBoxModel *bm, int species, int year, int tier, double TACbase, FILE *llogfp);

void DynamicTier4Assessment(MSEBoxModel *bm, int species, int year, FILE *llogfp);
void RDynamicTier4Assessment(MSEBoxModel *bm, int species, int year, FILE *llogfp);
void RatpackDynTier4Assessment(MSEBoxModel *bm, int species, int year, FILE *llogfp);
double Ratpack_Staleness_Buffer(MSEBoxModel *bm, int species, int year, FILE *llogfp);
double ACLIM_HCR(int type, double B2B0, double alpha, double gamma, double theta, double omega1, double omega2, double omega3, double cov,
                 double B2B0_lim, double B2B0_target, double Flim);
double Calc_ACLIM_Fmult(MSEBoxModel *bm, int species, double depletion, FILE *llogfp);
void WriteCPUE(MSEBoxModel *bm, int species, int region, int year, FILE *fid);
void WriteResults(MSEBoxModel *bm, int species, int region, int year, FILE *llogfp, FILE *fid);
void YPR(MSEBoxModel *bm, int species, double fval, double natM, double *spbpr, double *wt, double *mat, double *sel, FILE *llogfp);

void downhill_simplex(MSEBoxModel *bm, int funkflag, int species, double **simplex, double *func, int npar, double ftol, int iter, int *ok, FILE *llogfp, double *xpar);

double getSlope(double *x, double *y, int n, FILE *llogfp);
double TierThree_trueF(MSEBoxModel *bm, int species, int year, double agesel95, FILE *llogfp);
double Transform(double par, double constraint, FILE *llogfp);
double UnTransform(double transpar, double constraint);

double DoCPUE(MSEBoxModel *bm, int species, int year, FILE *llogfp);
double DoPrager(MSEBoxModel *bm, int species, int year, FILE *llogfp);

double regressionSlope(double *y, int ysize, int n, FILE *llogfp);

void Check_Indicator_Links(MSEBoxModel *bm, int year);
void CalculateNonIndicatorRBC(MSEBoxModel *bm, int sp, int isim, int year);

/* Output files */
FILE *histCPUEfp;
FILE *tier5fp;

static FILE * Init_tierRBC_File(MSEBoxModel *bm);
static FILE * Init_histCPUE_File(MSEBoxModel *bm);
static FILE * Init_tier5_File(MSEBoxModel *bm);

/********************************************************************************************************
 * \brief  Call the assessment code only if at start of "true assessment" period,
 * before then collate "historical data"
 *
 *  If doing the assessment then do stock assessment on historic data(up to year - 2)
 *  as assessment to calc this year's TAC happened last year on data from year before
 *  e.g. assessment performed in 2007 on data to end of 2006 is for 2008 TAC
 */
void CallTierAssessment(MSEBoxModel *bm, int species, int year, FILE *llogfp) {
	int nf;
	double TAC, RBC;
    int assess_flag_sp = (int) (FunctGroupArray[species].speciesParams[assess_flag_id]);
    
    if (!assess_flag_sp)
        return;

    // Check to see if historical data needed
	if(bm->RBCestimation.RBCspeciesParam[species][HistDataInit_id] == FALSE) {
		SS3Link_Init(bm, species);
	}
    
	bm->RBCestimation.RBCspeciesArray[species].CurrentYear = year;
    
	if(bm->RBCestimation.RBCspeciesParam[species][AssessStart_id] > year) {
        
 		/******* Historical data collection *******/

		// As Atlantis is providing the historical period then don't need HistProj()
		// Generate historical dataset (to HistYrMax) - calls GenData for each historical year
        
        printf("Generating data during the non-assessment period for %s\n", FunctGroupArray[species].groupCode);
        
		// generate the appropriate data
		GenData(bm, species, year);

		// As no actual assessment yet just roll over any TACs
		TAC = 0.0;
		RBC = 0.0;
		for (nf = 0; nf < bm->K_num_fisheries; nf++) {
            // save TAC value so can compare to next year - don't modify now_id cases
            bm->TACamt[species][nf][old_id] = bm->TACamt[species][nf][now_id];
            bm->TACamt[species][nf][RBCnow_id] = bm->TACamt[species][nf][now_id];;
            bm->TACamt[species][nf][RBCold_id] = bm->TACamt[species][nf][RBCnow_id];
            if(!FunctGroupArray[species].isTAC || (bm->TACamt[species][nf][now_id] < no_quota)) {
                TAC += bm->TACamt[species][nf][now_id];  // This is "old" as new one not set yet (and so old_id one is year before last)
                RBC += bm->TACamt[species][nf][RBCnow_id];
            }
		}
		  // For reporting
		bm->RBCestimation.RBCspeciesParam[species][TAC_old_id] = TAC;
		bm->RBCestimation.RBCspeciesParam[species][TACpost_id] = TAC;
		bm->RBCestimation.RBCspeciesParam[species][TACest_id] = TAC;
        bm->RBCestimation.RBCspeciesArray[species].RBC_by_year[year] = RBC;
        bm->RBCestimation.RBCspeciesArray[species].TAC_by_year[year] = TAC;
        
        
        printf("Finished data collection\n");

		// No actual assessment required so continue to the next species
		return;

	} else {
        
		/******* Actual assessment running *******/

        printf("Generating data during the assessment period for %s\n", FunctGroupArray[species].groupCode);
		// generate the appropriate data
		GenData(bm, species, year);
        
        printf("Entering assessment rourine for %s\n", FunctGroupArray[species].groupCode);
		// do the actual assessment
		DoTierAssessment(bm, species, year, llogfp);

        // store values from each sim for later analysis - for multispecies need to store it later (after multispecies adjustments)
        if (!bm->useMultispAssess) {
            Evaluation(bm, species, year, llogfp, bm->tierRBCfp);
        }
        
        printf("Finished assessment process for %s\n", FunctGroupArray[species].groupCode);

	}

	return;
}

/********************************************************************************************************
 * \brief  Do the assessment and get catches for next year
 *
 *  Routine to do the specified assessment on data up to end of year
 *  (i.e. assessment presumably happening in year on data to year-1), 
 *  and assessment set for coming year (may need to make delay 2 years)
 *  in which doing assessment (the input parameter 'year' here is the
 *  final year of data being collected and year-delay is used in assessment
 */
void DoTierAssessment(MSEBoxModel *bm, int species, int year, FILE *llogfp){
    
    double current_catch = 0, tac_change, cpue_ratio;
	double maxprop, tacAss, tacPost = 0, TAC, RBC, minCatch, lastHistCatch;
	double TAC_old, TACnf, RBCnf, TACbase, staleTACbase = -1.0;
	double discard_rate = bm->RBCestimation.RBCspeciesParam[species][est_discard_rate_id];   // Nov 2026: was read from speciesParams
	double inc_check = (bm->prop_incTAC - 1.0);
	double dec_check = (1.0 - bm->prop_decTAC);
    
    int Nregions = (int)(bm->RBCestimation.RBCspeciesParam[species][NumRegions_id]);
	int Nfleets = (int)(bm->RBCestimation.RBCspeciesParam[species][NumFisheries_id]);
	int AgeSel95 = (int)bm->RBCestimation.RBCspeciesParam[species][AgeSel95_id];
	int tier = (int) (FunctGroupArray[species].speciesParams[tier_id]);
	int tiertype = (int) (bm->RBCestimation.RBCspeciesParam[species][tiertype_id]);
	int nyears = year + 1;
    int nf, nreg, CPUEfleet, assess_flag_sp, nfishery, index1, index2;
    int delay = bm->RBCestimation.AssessDelay;
    int breakout = 0;
	//int do_debug = 0;
    
    if (year - delay < 0)
        delay = year;  // So delay is long as possible even if not as long as desired
    
    // Now reset year so its the final year of the assessment historical data series used
    year = year - delay;
        
    printf("DoTierAssessment %s with year: %d with nYears: %d Nregions: %d Nfleets: %d\n", FunctGroupArray[species].groupCode, year, nyears, Nregions, Nfleets);
    
    /*
	if((bm->debug == debug_quota) && (bm->which_check == species)){
		do_debug = 1;
	}
     */
    
    //if(verbose)
        printf("Doing Tiered Assessment for %s FunctGroupArray[species].groupCode with UseSS: %d\n", FunctGroupArray[species].groupCode, bm->RBCestimation.UseSS);

	// Check to see if worth continuing
	assess_flag_sp = (int) (FunctGroupArray[species].speciesParams[assess_flag_id]);
	if (!assess_flag_sp)
		return;

	/* If only resetting Fs skip ahead now */
	if (FunctGroupArray[species].speciesParams[flagFonly_id])
		return;

    bm->RBCestimation.RBCspeciesParam[species][AssessFail_id] = FALSE;  // For purposes of sending warning messages later
    
    /******* Find total old TAC *******/
	TAC_old = 0.0;
	for (nf = 0; nf < bm->K_num_fisheries; nf++){
        if (!bm->FISHERYprms[nf][flagrecfish_id] && bm->inQuota[nf][species])
            TAC_old += bm->TACamt[species][nf][now_id];  // this value is in kg
	}
	bm->RBCestimation.RBCspeciesParam[species][TAC_old_id] = TAC_old;
    
    // find final historical year catch
	lastHistCatch = 0.0;
	for (nf = 0; nf < Nfleets; nf++) {
		for (nreg = 0; nreg < Nregions; nreg++){
			lastHistCatch += bm->RBCestimation.RBCspeciesArray[species].CatchData[nf][nreg][year];
		}
	}
    
    printf("Doing Tiered Assessment - Deal with old TAcs and historical catches\n");
    
	/******* Calculate average current catch and discard rate - from catch data *******/
    if (tier > tier0)
        Tier_averages(bm, species, year, tier, &current_catch, &discard_rate, llogfp);

    /** Nov 2026: first year the new RBC applies (data year + 1), updated every year as in Ratpack mgt.rbcYear
        (set before the trigger check as that uses the catch in mgt_rbcYear - 1, i.e. the latest data year) **/
    bm->RBCestimation.RBCspeciesArray[species].mgt_rbcYear = year + 1;

    /** Check for indicator species status */
    if(bm->RBCestimation.UseTriggerMgmt && (year > bm->RBCestimation.RBCspeciesParam[species][AssessStart_id])) {
        
        // Only do this if after first year of the assessment period
        if(bm->RBCestimation.RBCspeciesParam[species][isTriggerSpecies_id]) {
            
            printf("Checking trigger breakout for %s\n", FunctGroupArray[species].groupCode);
            
            breakout = trigger_species_breakout(bm, species, year);
            if(breakout > 0) {
                // Nothing to do as uses tier from speciesParams[tier_id]
            } else {
                // Rollover so overwrite tier
                tier = sp_rollover;
            }
        }
    }

    printf("Doing Tiered Assessment - check for rollover\n");
    
    /** Check for MYTAC **/
    if (year < bm->RBCestimation.RBCspeciesParam[species][myTACend_id]) {
        // Rollover so overwrite tier
        tier = sp_rollover;
    } else {
        // Update timeline so ready for next time
        bm->RBCestimation.RBCspeciesParam[species][myTACend_id] = year + bm->RBCestimation.RBCspeciesParam[species][myTACperiod_sp_id];
    }

    
    printf("Doing Tiered Assessment - start assessment of species %s with tier: %d\n", FunctGroupArray[species].groupCode, tier);

    // Nov 2026: clear any RBC projection from the last assessment (only dynamic Tier 4 and SS3 fill it - review item 27)
    bm->RBCestimation.RBCspeciesArray[species].nRBC_proj = 0;

    /******* Do the tier analyses *******/
	switch (tier){
	case tier0: /* ----------- No future catches ---------- */
        bm->RBCestimation.RBCspeciesParam[species][AssessFail_id] = 1;
        RBC = bm->RBCestimation.RBCspeciesParam[species][RBCest_id] = TAC_old;
		break;
	case tier1: /* ----------- Tier 1 --------------------- */
            if (!bm->RBCestimation.UseSS) {
                TierOneMimic(bm, species, year, llogfp);   // calculate RBC from perfect information + error
            } else {
                TierOne(bm, species, year, llogfp);          // use SS to do Tier 1 analysis
            }
		break;
    case dyntier1B0: /* -------- Dynamic B0 Tier 1 Assessment --------------- */
        //TierOne_DynamicBo(bm, species, year, llogfp);
        quit("To be coded\n");
        break;
	case tier2: /* ----------- Tier 2 --------------------- */
		// No nothing for now so set it as an assess fail until rules for this tier are instituted
        bm->RBCestimation.RBCspeciesParam[species][AssessFail_id] = 1;
        RBC = bm->RBCestimation.RBCspeciesParam[species][RBCest_id] = TAC_old;
		break;
	case tier3: /* ------------ Tier 3 --------------------- */
		bm->RBCestimation.RBCspeciesParam[species][True_F_id] = TierThree_trueF(bm, species, year, AgeSel95, llogfp);
		bm->RBCestimation.RBCspeciesParam[species][True_F48_id] = bm->RBCestimation.RBCspeciesParam[species][F48_id];
		//-------- calculate current F  (global Fcurr)

		switch ((int)(bm->RBCestimation.RBCspeciesParam[species][Tier3_Fcalc_id])){
		case 1: // ----- catch curves (old T3)
			if ( AgeSel95 > (FunctGroupArray[species].numCohorts-1)) {
				quit("Catch curves will not work for this species because age at full selectivity > maximum age\n");
			} else {
				TierThree_CC(bm, species, year, llogfp);
			}
			break;
		case 2: // ----- catch curves with selectivity  (new T3)
			TierThree_CCsel(bm, species, year, llogfp);
			break;
		case 3:  // ----- true F
			bm->RBCestimation.RBCspeciesParam[species][Fcurr_id] = bm->RBCestimation.RBCspeciesParam[species][True_F_id];
			// if using true F use only final year's catch + true discard ***************
			current_catch = 0.0;
			for (nf = 0; nf < bm->K_num_fisheries; nf++)
				for (nreg = 0; nreg < Nregions; nreg++)
					current_catch += Harvest_Get_TotCumCatch(species, nf, bm->thisyear);   // True catchs and discards
			break;
		}

		// ---------  apply tier 3 rule to get RBC
		if (tiertype == orig_tier_rule) {   // original rule
			Tier3Rule(bm, species, year, current_catch, llogfp);
		} else if (tiertype == new_tier_rule) {    // Neils' new rule
			newTier3Rule(bm, species, year, current_catch, llogfp, tier5fp);
		}
		break;
	case tier4: // ----------------  tier 4 --------------------
		if (tiertype == orig_tier_rule) {
			TierFour(bm, species, year, current_catch, llogfp);
		} else if (tiertype == new_tier_rule) {
            newTierFour(bm, species, year, current_catch, llogfp);
		}
		break;
    case dyntier4: // -----------  dynamic tier 4 --------------------
        if (tiertype == orig_tier_rule) {
            RDynamicTier4Assessment(bm, species, year, llogfp);
        } else if (tiertype == ratpack_tier_rule) {
            RatpackDynTier4Assessment(bm, species, year, llogfp);   // Native C version of Ratpack dynamic Tier 4
        } else {
            DynamicTier4Assessment(bm, species, year, llogfp);
        }
        break;
    case tier14: // ------- perfect information + error with ACLIM harvest control rule (Ratpack Tier_5) --------
        TierOneMimic(bm, species, year, llogfp);
        break;
    case tier5: //  ---------------- tier 5 --------------------
            
		if (tiertype == orig_tier_rule) {         // Surplus production
			TierFive_SP(bm, species, year, llogfp, tier5fp);
		} else if (tiertype == new_tier_rule) {    // av length
			TierFive_AvLen(bm, species, year, llogfp);
			newTier3Rule(bm, species, year, current_catch, llogfp, tier5fp);
		} else if (tiertype == alt_tier_rule) {     // depletion corrected average catch
            TierFiveDCAC(bm, species, year, current_catch, llogfp);
        } else if (tiertype == safe_tier_rule) {     // depletion corrected average catch
            TierFiveSAFE(bm, species, year, current_catch, llogfp);
        }
		break;
	case tier6: //  ---------------- tier 6 --------------------
        TierSixRUSS(bm, species, year, llogfp);
		break;
	case tier7: //  ---------------- tier 7 --------------------
        TierSevenRUSS(bm, species, year, llogfp);
        break;
    case sp_rollover: // whether indicator spp or MYTAC rollover last year's value
        /* Nov 2026 (review item 31): the RBC is the RBC from the last successful assessment (it used to be set to
           last year's TAC, which already has the buffers in it). Last year's TAC is rolled over on the TAC path below.
           If there has been no successful assessment yet, carry forward last year's RBC. */
        if (bm->RBCestimation.RBCspeciesArray[species].lastAssessRBC > 0.0)
            bm->RBCestimation.RBCspeciesParam[species][RBCest_id] = bm->RBCestimation.RBCspeciesArray[species].lastAssessRBC;
        else if (year > 0)
            bm->RBCestimation.RBCspeciesParam[species][RBCest_id] = bm->RBCestimation.RBCspeciesArray[species].RBC_by_year[year - 1];
        else
            bm->RBCestimation.RBCspeciesParam[species][RBCest_id] = TAC_old;
        break;
    case tier8:  // US method
    case tier9:  // Norwegian method
    case tier13:  // Icealndic escapement method
        RBC = 0.0;
	default:
		printf("No such tier (%d)\n", tier);
		fprintf(llogfp,"No such tier (%d)\n", tier);
		quit("");
		break;

	}

    /** Ratpack staleness buffer in rollover years - relative to the last successful assessment.
        Nov 2026 (review item 31): applied to both (b) the RBC (last assessed RBC, stored in RBC_by_year) and
        (a) the TAC path (last assessment's buffered value, stored below once the buffers have been applied).
        Returns the TAC path value, or -1 if there has been no successful assessment yet. **/
    if ((tier == sp_rollover) && (bm->RBCestimation.RBCspeciesParam[species][Buffer_stale_rate_id] > 0.0)) {
        staleTACbase = Ratpack_Staleness_Buffer(bm, species, year, llogfp);
    }

    /** Replace RBC by average over a period if using RBC averaging (Nov 2026: only for actual assessments).
        Review item 27: UseRBCAveraging 1 = Atlantis moving average of this and previous RBCs,
        2 = Ratpack average over the first nRBCaverage years of this assessment's projected RBCs **/
    if (tier != sp_rollover) {
        switch ((int)(bm->RBCestimation.RBCspeciesParam[species][UseRBCAveraging_id])) {
        case no_RBC_avg:
            break;
        case moving_RBC_avg:
            Averaged_RBC(bm, species, year, llogfp);
            break;
        case ratpack_RBC_avg:
            Ratpack_Averaged_RBC(bm, species, year, llogfp);
            break;
        default:
            quit("DoTierAssessment: UseRBCAveraging for %s must be 0, 1 or 2 (currently %d)\n", FunctGroupArray[species].groupCode,
                 (int)(bm->RBCestimation.RBCspeciesParam[species][UseRBCAveraging_id]));
            break;
        }
    }

    // The RBC is the estimate from the assessment (after any averaging). It is not changed by the buffers below.
	RBC = bm->RBCestimation.RBCspeciesParam[species][RBCest_id];

    /** Nov 2026 (review item 31): buffers are applied after the RBC is calculated, on the way to the TAC.
        TACbase starts as the RBC and the buffers act on it; the TAC is then calculated from TACbase.
        (Previously the buffers overwrote RBCest - so the reported RBC was buffered - while the TAC was calculated
        from the unbuffered RBC, so the buffers never reached the TAC.) **/
    TACbase = RBC;

    /** Rollover years: the TAC path starts from the staleness-buffered value if using the Ratpack staleness
        buffer, otherwise from last year's TAC (which already has the buffers in it).
        Nov 2026 (review item 29): multi-year TAC decay - one more year of (1 - myTACbuffer) on last year's TAC in
        rollover years within a multi-year TAC. Now applied whether or not UseTierBuffers is set (it used to be
        inside Apply_RBC_Buffers so only worked with the tier buffers on). Not applied if using the Ratpack
        staleness buffer. **/
    if (tier == sp_rollover) {
        if (staleTACbase >= 0.0) {
            TACbase = staleTACbase;
        } else {
            TACbase = TAC_old;
            TACbase = Apply_MYTAC_Decay(bm, species, year, TACbase, llogfp);
        }
    }

    /******* Apply US style tier buffers (OFL to ABC, i.e. RBC towards TAC) ******/
    if (bm->RBCestimation.UseTierBuffers) {
        TACbase = Apply_RBC_Buffers(bm, species, year, tier, TACbase, llogfp);   // Nov 2026: pass effective tier so rollover years are recognised
    }

    /** Store the buffered value (after averaging and buffers) from an actual, successful assessment so the Ratpack
        staleness buffer in later rollover years starts from the same value as this year's TAC (discards are removed
        from both below) **/
    if ((tier != sp_rollover) && !bm->RBCestimation.RBCspeciesParam[species][AssessFail_id]) {
        bm->RBCestimation.RBCspeciesParam[species][Ratpack_baseRBC_id] = TACbase;
        bm->RBCestimation.RBCspeciesArray[species].lastAssessRBC = RBC;   // unbuffered RBC, carried through rollover years
    }
    
	/******* Now calculate TAC from RBC *******/
	if (tier == tier0 ) { //------------- no change in future catches ---------------
		RBC = 0.0;
		TAC = 0.0;
        bm->RBCestimation.RBCspeciesParam[species][TACpost_id] = 0.0;
        for (nf = 0; nf < bm->K_num_fisheries; nf++) {
            if (!bm->FISHERYprms[nf][flagrecfish_id] && bm->inQuota[nf][species] && (bm->TACamt[species][nf][now_id] < no_quota)) {
                RBC += bm->TACamt[species][nf][RBCold_id];
                TAC += bm->TACamt[species][nf][old_id];
                bm->RBCestimation.RBCspeciesParam[species][TACpost_id] += bm->TACamt[species][nf][old_id];
            }
        }
	} else {
        
        printf("Finished tier calculations and applying meta rules\n");
        
		//  if Tier 1 assessment and SS has not converged, ignore this assessment and set catches to
		//  those used in previous year
		// TODO: Ask Sally if AssessFail case should only apply to Tier1 as the note says
		if (bm->RBCestimation.RBCspeciesParam[species][AssessFail_id]) {
			RBC = 0.0;
			TAC = 0.0;
			bm->RBCestimation.RBCspeciesParam[species][TACpost_id] = 0.0;
			for (nf = 0; nf < bm->K_num_fisheries; nf++) {
                if (!bm->FISHERYprms[nf][flagrecfish_id] && bm->inQuota[nf][species] && (bm->TACamt[species][nf][now_id] < no_quota)) {
                    RBC += bm->TACamt[species][nf][RBCold_id];
                    TAC += bm->TACamt[species][nf][old_id];
                    bm->RBCestimation.RBCspeciesParam[species][TACpost_id] += bm->TACamt[species][nf][old_id];
                }
			}
            
            /*
            if (do_debug) {
                fprintf(llogfp, "As AssessFail: %e, Time: %e, %s, year: %d, RBC: %e, TAC: %e\n",
                        bm->RBCestimation.RBCspeciesParam[species][AssessFail_id], bm->dayt, FunctGroupArray[species].groupCode, year, RBC, TAC);
            }
             */
            
		} else {
			// remove discards (unless using Tier 1 perfect info 25/3/09 or Tier 1 SS as RBC is retained)
			// Nov 2026: also not in rollover years, as the rolled over value is last year's (retained) TAC.
			// The exception is the Ratpack staleness buffer, which is based on the last assessed RBC so still has discards in it.
			if ((tier == tier1) || ((tier == sp_rollover) && !(staleTACbase >= 0.0)))
				bm->RBCestimation.RBCspeciesParam[species][TACdisc_id] = TACbase;   // Nov 2026 (item 31): buffered value, was RBC
			else
				bm->RBCestimation.RBCspeciesParam[species][TACdisc_id] = TACbase - discard_rate * TACbase;

			// apply post-assessment CPUE indicator rule, use CPUE from next year of data
			if (bm->RBCestimation.RBCspeciesParam[species][PostRule_id] > 0) {
				tacAss = bm->RBCestimation.RBCspeciesParam[species][TACdisc_id];
				// which fleet has highest proportion of catch (av over last 5 years) ?
				maxprop = 0;
				CPUEfleet = 0;
				for (nf = 0; nf < Nfleets; nf++){
					if (bm->RBCestimation.RBCspeciesArray[species].avprop[nf] > maxprop) {
						maxprop = bm->RBCestimation.RBCspeciesArray[species].avprop[nf];
						CPUEfleet = nf;
					}
				}
                
                if(delay) {
                    index1 = year;
                    index2 = year+1;
                } else {
                    index1 = year-1;
                    index2 = year;
                }
                
				// Nov 2026: all-region CPUE (index Nregions) - was region 0 only
				cpue_ratio = bm->RBCestimation.RBCspeciesArray[species].CPUEgen[CPUEfleet][Nregions][index2] / bm->RBCestimation.RBCspeciesArray[species].CPUEgen[CPUEfleet][Nregions][index1];

				if (bm->RBCestimation.RBCspeciesParam[species][PostRule_id] == 1)
					tacPost = tacAss * (1.0 + bm->RBCestimation.RBCspeciesParam[species][CPUEmult_id] * (cpue_ratio - 1));
				else if (bm->RBCestimation.RBCspeciesParam[species][PostRule_id] == 2)
					tacPost = tacAss * (1.0 + bm->RBCestimation.RBCspeciesParam[species][CPUEmult_id] * log(cpue_ratio) / exp(1.0));
                
                bm->RBCestimation.RBCspeciesParam[species][TACpost_id] = tacPost;

                /*
				if (do_debug) {
					fprintf(llogfp, "Time: %e, %s, year: %d, RBC: %e, tacAss: %e, tacPost: %e, TACpost: %e, CPUEfleet: %d\n",
							bm->dayt, FunctGroupArray[species].groupCode, year, RBC, tacAss, tacPost, bm->RBCestimation.RBCspeciesParam[species][TACpost_id], CPUEfleet);
				}
                */
            } else {
				bm->RBCestimation.RBCspeciesParam[species][TACpost_id] = bm->RBCestimation.RBCspeciesParam[species][TACdisc_id];         
            }

			//  implement +/- constraints on TAC change (was 50% in sally code)
			//  Nov 2026: corrected so tac_change is positive for an increase and the increase (decrease)
			//  limit is applied to increases (decreases) - as in Ratpack _CalculateTAC. Previously a
			//  recommended decrease triggered an increase and vice versa.
			if (TAC_old > 0.0) {
				tac_change = (bm->RBCestimation.RBCspeciesParam[species][TACpost_id] - TAC_old) / TAC_old;

				if (tac_change > inc_check)
					TAC = TAC_old * (1.0 + inc_check);
				else if (tac_change < -dec_check)
					TAC = TAC_old * (1.0 - dec_check);
				else
					TAC = bm->RBCestimation.RBCspeciesParam[species][TACpost_id];
			} else {
				tac_change = 0.0;
				TAC = bm->RBCestimation.RBCspeciesParam[species][TACpost_id];
			}

			// implement no change if TAC differs by less than min threshold (set to 10% in Sally code)
			// (only when there is a previous TAC to compare to - as in Ratpack)
			// Nov 2026 (review item 28): not in rollover years, as otherwise a yearly multi-year TAC decay
			// (myTACbuffer or staleness rate) smaller than min_TACchange is cancelled and the TAC stays flat
            if ((tier != sp_rollover) && (TAC_old > 0.0) && (fabs(tac_change) < bm->RBCestimation.min_TACchange)) {
                TAC = TAC_old;
            }

			// TAC = TACpost;   // only if want no constraints
			minCatch = bm->RBCestimation.RBCspeciesParam[species][MinCatch_id] * lastHistCatch;   // Nov 2026: was read from speciesParams
            if (TAC < minCatch) {
                TAC = minCatch;   //	minCatch set to 5% of final year historic catch
            }
            
            /*
            if (do_debug) {
                fprintf(llogfp, "Time: %e, %s, year: %d TAC: %e tac_change: %e TAC_old: %e TACpost: %e  min_TACchange %e minCatch: %e MinCatch: %e lastHistCatch: %e inc_check: %e dec_check: %e prop_decTAC: %e min_TACchange: %e\n",
                        bm->dayt, FunctGroupArray[species].groupCode, year, TAC, tac_change, TAC_old, bm->RBCestimation.RBCspeciesParam[species][TACpost_id], bm->RBCestimation.min_TACchange, minCatch, bm->RBCestimation.RBCspeciesParam[species][MinCatch_id], lastHistCatch, inc_check, dec_check, bm->prop_decTAC, bm->RBCestimation.min_TACchange);
            }
            */
		}
	}

 	bm->RBCestimation.RBCspeciesParam[species][TACest_id] = TAC;  // For reporting
    if (bm->RBCestimation.RBCspeciesParam[species][TACest_id] < bm->discardTAC) {
        bm->RBCestimation.RBCspeciesParam[species][TACest_id] = bm->discardTAC;
    }
    
    /* Nov 2026: RBC_by_year now holds the RBC (it used to be given the TAC) and the TAC is stored in TAC_by_year.
       The RBC stored is the one from the assessment (after any averaging, before buffers and TAC meta-rules).
       In rollover years (multi-year TAC or trigger) it is the RBC from the last assessment, times the
       staleness buffer if that is being used (review item 31). */
    bm->RBCestimation.RBCspeciesArray[species].RBC_by_year[year] = RBC;
    bm->RBCestimation.RBCspeciesArray[species].TAC_by_year[year] = TAC;

	// allocate retained catch to fleets and regions, based on previous relative allocation - for commercial fisheries only (for now)
	for (nfishery = 0; nfishery < bm->K_num_fisheries; nfishery++) {
        if (!bm->FISHERYprms[nfishery][flagrecfish_id] && bm->inQuota[nfishery][species]) {
            
            // Proportion of the total TAC to provide this fleet
            TACnf = TAC * (bm->TACamt[species][nfishery][now_id] / (TAC_old + small_num));
            RBCnf = RBC * (bm->TACamt[species][nfishery][now_id] / (TAC_old + small_num));

            // save TAC value so can compare to next year
            bm->TACamt[species][nfishery][old_id] = bm->TACamt[species][nfishery][now_id];
            bm->TACamt[species][nfishery][RBCold_id] = bm->TACamt[species][nfishery][RBCnow_id];
            bm->TACamt[species][nfishery][now_id] = TACnf;
            bm->TACamt[species][nfishery][RBCnow_id] = RBCnf;
            
            if (bm->TACamt[species][nfishery][now_id] > no_quota)
                bm->TACamt[species][nfishery][now_id] = no_quota;
            
            fprintf(llogfp, "Time: %e %s fishery %d TAC %e (RBC %e) TAC_old: %e, RBC_old: %e\n",
                    bm->dayt, FunctGroupArray[species].groupCode, nfishery, TACnf, RBCnf, TAC_old, bm->TACamt[species][nfishery][RBCold_id]);
            fflush(bm->logFile);
        }
	}

    /*
	if (do_debug){
		fprintf(llogfp, "Time: %e, %s current_catch: %e, Fcurr: %e, RBC: %e, discard_rate: %e, TAC: %e\n",
				bm->dayt, FunctGroupArray[species].groupCode, current_catch,
				bm->RBCestimation.RBCspeciesParam[species][Fcurr_id], RBC, discard_rate, TAC);
	}
    */
    
    printf("Assessment finished\n");

    return;

}

/********************************************************************************************************
 * \brief  Tier_averages
 * Description: calculate average total catch  over last x (4) years for each fleet, sum over fleets
 *              get average discard rate combined over all fleets for last x (4) years
 *
 */

void Tier_averages(MSEBoxModel *bm, int species, int year, int tier, double *current_catch, double *discard_rate, FILE *llogfp)
{
	int nf, iy, sumregion, nreg, MinCyr = 0, MaxCyr = 0, DiscType = 0, nyears, startyear, diffyr;
	double ccurr, meancatch, avrate, curcatch, sumcatch, indexcount;
	double fltdiscbio = 0, totdiscbio, totretbio;
	int CCsel_years = (int)bm->RBCestimation.RBCspeciesParam[species][CCsel_years_id];
	int AgeSel50 = (int)bm->RBCestimation.RBCspeciesParam[species][AgeSel50_id];
	int Tier3_time = (int)bm->RBCestimation.RBCspeciesParam[species][Tier3_time_id];
	int Tier3_Fcalc = (int)bm->RBCestimation.RBCspeciesParam[species][Tier3_Fcalc_id];
	int Tier4_avtime = (int)bm->RBCestimation.RBCspeciesParam[species][Tier4_avtime_id];
	int Tier4_CPUEyrmin = (int)bm->RBCestimation.RBCspeciesParam[species][Tier4_CPUEyrmin_id];
	int Tier4_CPUEyrmax = (int)bm->RBCestimation.RBCspeciesParam[species][Tier4_CPUEyrmax_id];
	int Nregions = (int)(bm->RBCestimation.RBCspeciesParam[species][NumRegions_id]);
	int Nfleets = (int)(bm->RBCestimation.RBCspeciesParam[species][NumFisheries_id]);
    int nPast = (int)(bm->RBCestimation.RBCspeciesParam[species][HistYrMax_id] - bm->RBCestimation.RBCspeciesParam[species][HistYrMin_id]) + 1;
    int InitnYears = bm->RBCestimation.nFuture + nPast;
    
	double nyrs;

    printf("Doing Tier_averages - initalise popcatch and avprop\n");
    
    Util_Init_2D_Double(bm->RBCestimation.RBCspeciesArray[species].propcatch, Nfleets, InitnYears, 0.0);
    Util_Init_3D_Double(bm->RBCestimation.RBCspeciesArray[species].propcatch_reg, Nfleets, Nregions, InitnYears, 0.0);
    Util_Init_1D_Double(bm->RBCestimation.RBCspeciesArray[species].avprop, Nfleets, 0.0);
    Util_Init_2D_Double(bm->RBCestimation.RBCspeciesArray[species].avprop_reg, Nfleets, Nregions, 0.0);
    
    /*
    int do_debug = 0;
	if((bm->debug == debug_quota) && (bm->which_check == species)){
		do_debug = 1;
	}
     */
    
	sumregion = Nregions;  // sum over regions
    
	/* Sally deleted code snippet AA was here */

	// work out proportion of catch taken by each fleet and region in each of last 5 years of data in the assessment
	// assessments don't know about regions, but need to know the proportion of catch by region
	// and fleet so can allocate TACs
    startyear = year-4;
    if (startyear < 0) startyear = 0;
	for (iy = startyear; iy < year+1; iy++) {
		sumcatch = 0.0;
		for (nreg = 0; nreg < Nregions ; nreg++){
			for (nf = 0; nf< Nfleets; nf++){
				//sumcatch += Harvest_Get_TotCumCatch(species, nf, iy);   Use this if want true catch and for loop is over K_num_fisheries
                sumcatch += bm->RBCestimation.RBCspeciesArray[species].CatchData[nf][nreg][iy];  // Use this for "pseudo-data" in MSE and summing oonly over assessment NumFisheries
                if (bm->flagTACincludeDiscard) {
                    sumcatch += bm->RBCestimation.RBCspeciesArray[species].DiscData[nf][nreg][iy];
                }
			}
		}
        // Similarly to above could get true catch using Harvest_Get_TotCumCatch(), but makes more sense in MSE to use the "pseudo-data" instead
		for (nf = 0; nf< Nfleets; nf++){
            for (nreg = 0; nreg < Nregions ; nreg++){
                if (sumcatch > 0.0) {
                    if (bm->flagTACincludeDiscard) {
                        bm->RBCestimation.RBCspeciesArray[species].propcatch_reg[nf][nreg][iy] = (bm->RBCestimation.RBCspeciesArray[species].CatchData[nf][nreg][iy] + bm->RBCestimation.RBCspeciesArray[species].DiscData[nf][nreg][iy]) / (sumcatch + small_num);
                    } else
                        bm->RBCestimation.RBCspeciesArray[species].propcatch_reg[nf][nreg][iy] = bm->RBCestimation.RBCspeciesArray[species].CatchData[nf][nreg][iy] / (sumcatch + small_num);

                } else
                    bm->RBCestimation.RBCspeciesArray[species].propcatch_reg[nf][nreg][iy] = 0.0;
                bm->RBCestimation.RBCspeciesArray[species].propcatch[nf][iy] += bm->RBCestimation.RBCspeciesArray[species].propcatch_reg[nf][nreg][iy];
			}
		}
	}

	/* Sally deleted code snippet AB was here */

	// calculate average proportion of catch in each fleet and region over last 5 years of assessment data
    startyear = year-4;
    if (startyear < 0) startyear = 0;
	for (nf = 0; nf< Nfleets; nf++){
		for (nreg = 0; nreg < Nregions ; nreg++){
			for (iy = startyear; iy < year+1; iy++) {
				bm->RBCestimation.RBCspeciesArray[species].avprop_reg[nf][nreg] += bm->RBCestimation.RBCspeciesArray[species].propcatch_reg[nf][nreg][iy];
			}
			bm->RBCestimation.RBCspeciesArray[species].avprop_reg[nf][nreg] /= bm->RBCestimation.num_avg_year;
			bm->RBCestimation.RBCspeciesArray[species].avprop[nf] += bm->RBCestimation.RBCspeciesArray[species].avprop_reg[nf][nreg];
		}
	}

	// years over which to average catch
	switch(tier){
	case tier0:
	case tier1:
	case tier2:
    case tier6:
    case tier7:
    case dyntier4:
    case tier14:  /* nothing to do */
		MinCyr = year - 4;  // ***** was year
		MaxCyr = year;
		break;
	case tier3:
		if (Tier3_time == 1) { // most recent  years
			MinCyr = year-3;
			MaxCyr = year;
		}
		else if (Tier3_time == 2) {
			// CCsel_years/2.0 is subtracted because F is av over 5 most recent years CCs, want catch av to match
			MinCyr = year - ((FunctGroupArray[species].numCohorts-1) - AgeSel50) - (int)ceil(CCsel_years/2.0);
			MaxCyr = year - (int)ceil(CCsel_years/2.0);
		}
		if (Tier3_Fcalc == 3) {  // true
			MinCyr = year;
			MaxCyr = year;
		}
		break;
	case tier4:
		if (Tier4_avtime > 0) {
			MinCyr = year - Tier4_avtime + 1;
			MaxCyr = year;
		}
		else if (Tier4_avtime == 0) {
			MinCyr = Tier4_CPUEyrmin;
			MaxCyr = Tier4_CPUEyrmax;
		}
        diffyr = MaxCyr - MinCyr;
        if (MaxCyr > year) {
            MaxCyr = year;
            MinCyr = MaxCyr - diffyr;
        }
            
        fprintf(bm->logFile, "Time: %e %s year: %d Tier4_avtime: %d Tier4_CPUEyrmin: %d Tier4_CPUEyrmax: %d diffyr: %d MaxCyr: %d MinCyr: %d\n", bm->dayt, FunctGroupArray[species].groupCode, year, Tier4_avtime, Tier4_CPUEyrmin, Tier4_CPUEyrmax, diffyr, MaxCyr, MinCyr);
            
		break;
	case tier5:
		MinCyr = year - ((FunctGroupArray[species].numCohorts-1) - AgeSel50);
		MaxCyr = year;
		break;
	default:
		quit("No such tier");
		break;
	}

    // Sanity check
    if (MaxCyr < 1) MaxCyr = 1;
    if (MaxCyr > year) MaxCyr = year;
    if (MinCyr < 1) MinCyr = 1;
    if (MinCyr > MaxCyr) MaxCyr = MinCyr;
    
	nyears = MaxCyr - MinCyr + 1;
	nyrs = (double)(nyears);
    
    if (MaxCyr == 1) {
        MaxCyr = 2;  // Put here to trap case where start dat collection from run start so entry 0 is all zeroes so sets quote to zero as result, at least this way it will count the current years catch that first year
    }
    
    fprintf(bm->logFile, "Time: %e %s year: %d MaxCyr: %d MinCyr: %d\n", bm->dayt, FunctGroupArray[species].groupCode, year, MaxCyr, MinCyr);

// calculate average total catch (discard + retained) over last nyears years for each fleet, sum over fleets (for Tier 3 rule)
	curcatch = 0.0;
    Util_Init_1D_Double(bm->RBCestimation.RBCspeciesArray[species].discrate, nyears, 0.0);

	for (nf = 0; nf < Nfleets; nf++){
		DiscType = bm->RBCestimation.RBCspeciesArray[species].DiscardType[nf];

        // calculate weighted average discard rate over last 4 years by fleet
        startyear = year-3;
        if (startyear < 0) startyear = 0;
		for (iy = startyear; iy < year+1; iy++){
			if (bm->RBCestimation.RBCspeciesArray[species].DiscYears[nf][iy] > 0)   // presumably these will be generated if testing tier 3
			{
				if (DiscType == 1)
					bm->RBCestimation.RBCspeciesArray[species].discrate[iy] = bm->RBCestimation.RBCspeciesArray[species].DiscData[nf][sumregion][iy]
					               / (bm->RBCestimation.RBCspeciesArray[species].DiscData[nf][sumregion][iy]
					                  + bm->RBCestimation.RBCspeciesArray[species].CatchData[nf][sumregion][iy]);
				else if (DiscType == 2)
					bm->RBCestimation.RBCspeciesArray[species].discrate[iy] = bm->RBCestimation.RBCspeciesArray[species].DiscData[nf][sumregion][iy];
			}
            
            /**/
            //if(do_debug) {
                fprintf(llogfp,"Time: %e %s iy: %d DiscYears: %d, discrate: %e\n", bm->dayt, FunctGroupArray[species].groupCode, iy,
                    bm->RBCestimation.RBCspeciesArray[species].DiscYears[nf][iy], bm->RBCestimation.RBCspeciesArray[species].discrate[iy]);
            //}
            /**/
		}
		avrate = 8 * bm->RBCestimation.RBCspeciesArray[species].discrate[year];
        indexcount = 8.0;
        
        if ((year-1) > 0) {
            avrate += 4 * bm->RBCestimation.RBCspeciesArray[species].discrate[year-1];
            indexcount += 4.0;
        }
        if ((year-2) > 0) {
            avrate += 2 * bm->RBCestimation.RBCspeciesArray[species].discrate[year-2];
            indexcount += 2.0;
        }
        if ((year-3) > 0) {
            avrate += bm->RBCestimation.RBCspeciesArray[species].discrate[year-3];
            indexcount += 1.0;
        }
        avrate /= indexcount;

		// calculate av landed catch over last maxyr-minyr+1 years
		meancatch = 0;
		for (iy = MinCyr; iy< MaxCyr+1; iy++) {
			meancatch += bm->RBCestimation.RBCspeciesArray[species].CatchData[nf][sumregion][iy];
			// meancatch += bm->RBCestimation.RBCspeciesArray[species].CatchData[nf][sumregion][iy]
            //                + true_discards;  we would need to get this last term from Atlantis operating model array so getting using avrate
            
            //fprintf(llogfp, "Time: %e %s iy: %d nf: %d, catch: %e meancatch: %e\n", bm->dayt, FunctGroupArray[species].groupCode, iy, nf, bm->RBCestimation.RBCspeciesArray[species].CatchData[nf][sumregion][iy], meancatch);
            
		}
		meancatch = meancatch / nyrs;

        //fprintf(llogfp, "Time: %e %s iy: %d nf: %d, meancatch: %e nyrs: %e, Nfleets:%d\n", bm->dayt, FunctGroupArray[species].groupCode, iy, nf, meancatch, nyrs, Nfleets);

		// ideally would use past discard rates to adjust these historic catches, but mostly don't have them
		// so use recent av discard rate instead (could test this)

		ccurr = meancatch + avrate * meancatch / (1.0 - avrate);

		curcatch += ccurr;
        
        /**/
        //if(do_debug) {
            fprintf(llogfp,"Time: %e %s curcatch: %e ccurr: %e meancatch: %e avrate: %e meancatch: %e\n",
                bm->dayt, FunctGroupArray[species].groupCode, curcatch, ccurr, meancatch, avrate, meancatch);
        //}
        /**/
	}
	*current_catch = curcatch;


// get average discard rate combined over all fleets for last 4 years
    startyear = year-3;
    if (startyear < 0) startyear = 0;
	for (iy = startyear; iy < year+1; iy++){
		totdiscbio = 0.0;
		totretbio = 0.0;
		// sum over fleets
		for (nf = 0; nf < Nfleets; nf++){
			if (DiscType==1)    // biomass
				fltdiscbio = bm->RBCestimation.RBCspeciesArray[species].DiscData[nf][sumregion][iy];
			else if (DiscType==2)    // fraction of total
				fltdiscbio = bm->RBCestimation.RBCspeciesArray[species].DiscData[nf][sumregion][iy] * bm->RBCestimation.RBCspeciesArray[species].CatchData[nf][sumregion][iy] / (1.0 - bm->RBCestimation.RBCspeciesArray[species].DiscData[nf][sumregion][iy]);
			totdiscbio = totdiscbio + fltdiscbio;
			totretbio = totretbio + bm->RBCestimation.RBCspeciesArray[species].CatchData[nf][sumregion][iy];
		}
		bm->RBCestimation.RBCspeciesArray[species].discrate[iy] = totdiscbio / (totretbio + totdiscbio);
	}
    avrate = 8 * bm->RBCestimation.RBCspeciesArray[species].discrate[year];
    indexcount = 8.0;
    
    if ((year-1) > 0) {
        avrate += 4 * bm->RBCestimation.RBCspeciesArray[species].discrate[year-1];
        indexcount += 4.0;
    }
    if ((year-2) > 0) {
        avrate += 2 * bm->RBCestimation.RBCspeciesArray[species].discrate[year-2];
        indexcount += 2.0;
    }
    if ((year-3) > 0) {
        avrate += bm->RBCestimation.RBCspeciesArray[species].discrate[year-3];
        indexcount += 1.0;
    }
	*discard_rate = avrate / indexcount;

    /*
	if (do_debug){
		fprintf(llogfp,"Proportion of catch taken by each fleet in each of last 5 years\n");
        startyear = year-4;
        if (startyear < 0) startyear = 0;
		for (iy=startyear; iy< year+1; iy++){
			fprintf(llogfp, "Year: %d ", iy);
			for (nf = 0; nf < Nfleets; nf++)
				fprintf(llogfp, "nf: %d propcatch: %e\n", nf, bm->RBCestimation.RBCspeciesArray[species].propcatch[nf][iy]);
			fprintf(llogfp,"\n");
		}
		for (nf = 0; nf < Nfleets; nf++)
			fprintf(llogfp, "nf: %d avprop: %e\n", nf, bm->RBCestimation.RBCspeciesArray[species].avprop[nf]);
		fprintf(llogfp, "\n");
	}
    */

}

/******************************************************************************
 *
 * \brief  TierOne
 * Description: do a tier 1 assessment and get RBC for next year
 *
 * called by : DoAssessment
 * calls : WriteSSFiles
 * created  : off code from Sally Wayte
 */
/******************************************************************************/
void TierOne(MSEBoxModel *bm, int species, int year, FILE *llogfp)
{

    /* Deprecated from old catch cost risk SS efforts
	// Write SS dat, ctl and forecast and starter files from HistYrMin to ny-1
	WriteSSFiles(bm, species, year, llogfp);

	if (bm->RBCestimation.RBCspeciesParam[species][MaxConvergCrit_id] > 0.1){   // This was originally 0.01 but Sally had changed it to 0.1
		bm->RBCestimation.RBCspeciesParam[species][AssessFail_id] = 1;
	}
     
     */
    
    // New SS3 Tier one assessment calls
    SS330Assessment(bm, species, year, llogfp);

	return;
}
/******************************************************************************
 *
 \brief  TierOneMimic
 *	    TierOneMimic:   do a tier 1 assessment and get RBC for next year using perfect information + error (because SS3 takes too long)
 * Description: do a tier 1 assessment and get RBC for next year using perfect
 *              information + error (because SS takes too long)
 *
 *              Returns RBCest
 *
 * called by: DoAssessment
 * calls:
 * created: Beth Fulton
 ******************************************************************************/
void TierOneMimic(MSEBoxModel *bm, int species, int year, FILE *llogfp)
{
    double sb0, depletion, sigma, norm, depgen, fmax, yield, Hsteep, ftarg, natm, sf;
    double hcr_lim, hcr_brk, fref;
    int tier = (int) (FunctGroupArray[species].speciesParams[tier_id]);
    int hcr_option = (int)(bm->RBCestimation.RBCspeciesParam[species][Tier1_HCRoption_id]);
    int Nregions = (int)(bm->RBCestimation.RBCspeciesParam[species][NumRegions_id]);
    int Nstocks = FunctGroupArray[species].numStocks;
    double totcount = 0.0;
    double num_individuals = 0.0;
    double bodysize = 0.0;
    int s, a, ia, sn, rn, den, ij, k;
    int index1, index2;
   // int do_debug = 1;
    int maxage = (int)(bm->RBCestimation.RBCspeciesParam[species][MaxAge_id]);
    
    // So got meaningful data as assessing as go here
    if(bm->RBCestimation.AssessDelay > 1) {
        index1 = year+1;
        index2 = year+2;
    } else if (bm->RBCestimation.AssessDelay) {
        index1 = year+1;
        index2 = year+1;
    } else {
        index1 = year;
        index2 = year;
    }
    
    /* Nov 2026: arrays are [K_num_sexes][numCohorts] (ATS-02) so reset in that index order */
    Util_Init_2D_Double(bm->RBCestimation.RBCspeciesArray[species].agesel_bysex, bm->K_num_sexes, FunctGroupArray[species].numCohorts, 0.0);
    Util_Init_2D_Double(bm->RBCestimation.RBCspeciesArray[species].fval, bm->K_num_sexes, FunctGroupArray[species].numCohorts, 0.0);

    Util_Init_1D_Double(bm->RBCestimation.RBCspeciesArray[species].agesel, maxage, 0.0);
    Util_Init_1D_Double(bm->RBCestimation.RBCspeciesArray[species].mat, maxage, 0.0);
    Util_Init_1D_Double(bm->RBCestimation.RBCspeciesArray[species].wt, maxage, 0.0);

    // Find the level of depletion
	if ((bm->RBCestimation.RBCspeciesParam[species][Regime_year_id] > 0)
        && (year >= bm->RBCestimation.RBCspeciesParam[species][Regime_year_id])){
		sb0 = bm->RBCestimation.RBCspeciesArray[species].SBioZero_shift[Nstocks][Nregions];
    } else
		sb0 = bm->RBCestimation.RBCspeciesArray[species].SBioZero[Nstocks][Nregions];
    
	depletion = bm->RBCestimation.RBCspeciesArray[species].SpawnBio[Nstocks][Nregions][year] / sb0;
    
    /*
    if(do_debug) {
        fprintf(llogfp, "Time: %e (Year: %d), %s (Nstocks: %d) SpawnBio: %e sb0: %e depletion: %e\n",
                bm->dayt, year, FunctGroupArray[species].groupCode, Nstocks, bm->RBCestimation.RBCspeciesArray[species].SpawnBio[Nstocks][Nregions][year], sb0, depletion);
    }
    */
    
	sigma = sqrt(log(1.0 + bm->RBCestimation.Tier1_cv * bm->RBCestimation.Tier1_cv));
	norm = Util_xnorm(0, sigma, &bm->RBCestimation.Iseedy);
	depgen = depletion * exp(norm - 0.5 * sigma * sigma);
    
	// calculate F20, F40 and F48
	fmax = 0;
	for (s=0; s < bm->K_num_sexes; s++){
		for (a=0; a < FunctGroupArray[species].numCohorts; a++) {         //  should this be year+1?
			bm->RBCestimation.RBCspeciesArray[species].fval[s][a] = -log(1.0 - bm->RBCestimation.RBCspeciesArray[species].Uage[0][0][s][a][index1]);    //harvest rate to F. Uage r=0 is combined over fleets and regions
			if ((s == FEMALE) && (bm->RBCestimation.RBCspeciesArray[species].fval[s][a] > fmax))
				fmax = bm->RBCestimation.RBCspeciesArray[species].fval[s][a];
		}
		for (a=0; a <= (FunctGroupArray[species].numCohorts-1); a++) {
			bm->RBCestimation.RBCspeciesArray[species].agesel_bysex[s][a] = bm->RBCestimation.RBCspeciesArray[species].fval[s][a]/fmax;
            
            /*
            if(do_debug) {
                fprintf(llogfp, "Time: %e, %s-%d sex %d agesel: %e fval: %e fmax: %e Uage: %e\n",
                        bm->dayt, FunctGroupArray[species].groupCode, a, s, bm->RBCestimation.RBCspeciesArray[species].agesel_bysex[s][a], bm->RBCestimation.RBCspeciesArray[species].fval[s][a], fmax, bm->RBCestimation.RBCspeciesArray[species].Uage[0][0][s][a][index1]);
            }
             */
        }
	}
    
    
	for (a=0; a < bm->RBCestimation.RBCspeciesParam[species][MaxAge_id]; a++) {
        ia = (int)(a / FunctGroupArray[species].ageClassSize);
		bm->RBCestimation.RBCspeciesArray[species].wt[a] = bm->RBCestimation.RBCspeciesArray[species].MeanWtAge[0][FEMALE][a][year];
		bm->RBCestimation.RBCspeciesArray[species].mat[a] = bm->RBCestimation.RBCspeciesArray[species].Fecund[0][a];
		bm->RBCestimation.RBCspeciesArray[species].agesel[a] = bm->RBCestimation.RBCspeciesArray[species].agesel_bysex[FEMALE][ia];
        
        /*
        if(do_debug) {
            fprintf(llogfp, "Time: %e, %s-%d (ia: %d) wt: %e (%e %e) mat: %e (%e %e) agesel: %e (%e %e)\n",
                    bm->dayt, FunctGroupArray[species].groupCode, a, ia,
                    bm->RBCestimation.RBCspeciesArray[species].wt[a], bm->RBCestimation.RBCspeciesArray[species].MeanWtAge[0][FEMALE][a][year], bm->RBCestimation.RBCspeciesArray[species].MeanWtAge[0][FEMALE][a][year],
                    bm->RBCestimation.RBCspeciesArray[species].mat[a], bm->RBCestimation.RBCspeciesArray[species].Fecund[0][a], bm->RBCestimation.RBCspeciesArray[species].Fecund[1][a],
                    bm->RBCestimation.RBCspeciesArray[species].agesel[a], bm->RBCestimation.RBCspeciesArray[species].agesel_bysex[FEMALE][ia], bm->RBCestimation.RBCspeciesArray[species].agesel_bysex[FEMALE][ia]);
        }
        */
	}
    
	Hsteep = bm->RBCestimation.RBCspeciesParam[species][Hsteep_id];  // was Hsteep[1] in Sally's code
	getF20_40_48(bm, species, bm->RBCestimation.RBCspeciesArray[species].agesel, bm->RBCestimation.RBCspeciesArray[species].wt, bm->RBCestimation.RBCspeciesArray[species].mat, bm->RBCestimation.RBCspeciesArray[species].Mnat[0][0][0][0][year], Hsteep, llogfp);
    
	// now apply the harvest control rule
    if (tier == tier14)
        hcr_option = aclim_T1_hcr;   // Ratpack Tier_5 style perfect information assessment always uses the ACLIM rules

    switch (hcr_option) {
    case orig_T1_hcr:
        // Original 20:35:48 harvest control rule
        if (depgen < 0.2)
            ftarg = 0.0;
        else if (depgen < 0.35)
            ftarg = bm->RBCestimation.RBCspeciesParam[species][F40_id] * (depgen/0.2 - 1.0);
        else
            ftarg = bm->RBCestimation.RBCspeciesParam[species][F48_id];
        break;
    case ramp_T1_hcr:
        // Continuous ramp from zero at the limit to F at the target reference point (F48) at the break point
        hcr_lim = bm->RBCestimation.RBCspeciesParam[species][mgt_HCRlimit_id];
        hcr_brk = bm->RBCestimation.RBCspeciesParam[species][mgt_HCRbreak_id];
        if (hcr_brk <= hcr_lim)
            quit("TierOneMimic: for Tier1_HCRoption 1 the break point (BrefB = %e) must be greater than the limit (Blim = %e) for %s\n", hcr_brk, hcr_lim, FunctGroupArray[species].groupCode);
        if (depgen < hcr_lim)
            ftarg = 0.0;
        else if (depgen < hcr_brk)
            ftarg = bm->RBCestimation.RBCspeciesParam[species][F48_id] * (depgen - hcr_lim) / (hcr_brk - hcr_lim);
        else
            ftarg = bm->RBCestimation.RBCspeciesParam[species][F48_id];
        break;
    case aclim_T1_hcr:
        // ACLIM harvest control rule applied to the reference F
        if ((int)(bm->RBCestimation.RBCspeciesParam[species][HCR_Fref_id]) == 2)
            fref = bm->RBCestimation.RBCspeciesParam[species][F48_id];
        else
            fref = bm->RBCestimation.RBCspeciesParam[species][F40_id];
        ftarg = fref * Calc_ACLIM_Fmult(bm, species, depgen, llogfp);
        break;
    default:
        quit("TierOneMimic: Tier1_HCRoption %d not recognised for %s\n", hcr_option, FunctGroupArray[species].groupCode);
        break;
    }

    /*
    if(do_debug) {
        fprintf(llogfp, "Time: %e, %s depletion: %e depgen: %e ftarg: %e F40: %e, F48: %e\n",
                bm->dayt, FunctGroupArray[species].groupCode, depletion, depgen, ftarg, bm->RBCestimation.RBCspeciesParam[species][F40_id], bm->RBCestimation.RBCspeciesParam[species][F48_id]);
    }
     */
    
	// now calculate RBC - assume 1 region
	yield = 0;
	for (s=0 ; s < bm->K_num_sexes; s++){
		for (a=0; a < FunctGroupArray[species].numCohortsXnumGenes; a++){
            ia = (int)(a / FunctGroupArray[species].numGeneTypes);
            sn = FunctGroupArray[species].structNTracers[a];
            rn = FunctGroupArray[species].resNTracers[a];
            den = FunctGroupArray[species].NumsTracers[a];
            
            totcount = 0.0;
            num_individuals = 0.0;
            bodysize = 0.0;
            for (ij = 0; ij < bm->nbox; ij++) {
                for (k = 0; k < bm->boxes[ij].nz; k++) {
                    if (bm->boxes[ij].tr[k][den] > bm->min_dens) {
                        bodysize += (bm->boxes[ij].tr[k][sn] + bm->boxes[ij].tr[k][rn]) * bm->X_CN * mg_2_kg;
                        num_individuals += bm->boxes[ij].tr[k][den];
                        totcount += 1;
                    }
                }
            }
            if (totcount > 0.0) {  // Get the mean size
                bodysize /= totcount;
            }
            
            natm = bm->RBCestimation.RBCspeciesArray[species].Mnat[0][0][s][a][index2];
            sf = bm->RBCestimation.RBCspeciesArray[species].agesel_bysex[s][ia] * ftarg;
            if( natm || sf ) {
                yield += num_individuals * bodysize * (1.0 - exp(-(natm + sf))) * sf/(natm + sf);
            } else {
                yield += 0.0;
            }
            
            /**
            if(do_debug) {
                fprintf(llogfp, "Time: %e, %s sex: %d age: %d num: %e bodysize: %e rest: %e sel: %e natm: %e, sf: %e, ftarg: %e, yield: %e\n",
                        bm->dayt, FunctGroupArray[species].groupCode, s, a, num_individuals, bodysize, ((1.0 - exp(-(natm+sf))) * sf/(natm+sf)), bm->RBCestimation.RBCspeciesArray[species].agesel_bysex[s][ia], natm, sf, ftarg, yield);
            }
            **/
		}
	}

    /*
    if(do_debug)
        fprintf(llogfp, "Time: %e, %s yield: %e (%e)\n", bm->dayt, FunctGroupArray[species].groupCode, yield, yield / 1000.0);
     */

    bm->RBCestimation.RBCspeciesParam[species][RBCest_id] = yield;
	
	return;
    
}
/******************************************************************************
 *
\brief:  TierThree_CC
 *      TierThree_CC:     estimate Fcurr using catch curves
 * Description: estimate Fcurr using catch curves (old Tier3)
 *
 *
 * called by : DoAssessment
 * calls :  getSlope, Tier3_averages, Tier3rule
 * created  : From code by Sally Wayte
 *
******************************************************************************/
void TierThree_CC(MSEBoxModel *bm, int species, int year, FILE *llogfp)
{
	int s, nf, a, i, iy, nz, ia, isum05, sumnum;
	// int region;
	double sumz, meanz = 0, step1, sp_a, sp_b;
	int nselage, this_nselage;
	double age, len, temp;
	int Nfleets = (int)(bm->RBCestimation.RBCspeciesParam[species][NumFisheries_id]);
	int AgeSel95 = (int)(bm->RBCestimation.RBCspeciesParam[species][AgeSel95_id]);
    int maxage = (int)(bm->RBCestimation.RBCspeciesParam[species][MaxAge_id]);

    /*
    int do_debug = 1;
	if (do_debug)
		fprintf(llogfp,"Tier 3 catch curve calculations for %s assessment in year %d\n", FunctGroupArray[species].groupCode, year);
     */
    
    // This is the method Neil uses - use growth and selectivity params read in
	nselage = (int)(bm->RBCestimation.RBCspeciesParam[species][Tier3_maxage_id] - bm->RBCestimation.RBCspeciesParam[species][AgeSel95_id]);
	//region = 1;
    
    Util_Init_1D_Double(bm->RBCestimation.RBCspeciesArray[species].agesel, maxage, 0.0);
    Util_Init_1D_Double(bm->RBCestimation.RBCspeciesArray[species].mat, maxage, 0.0);
    Util_Init_1D_Double(bm->RBCestimation.RBCspeciesArray[species].wt, maxage, 0.0);
    
    Util_Init_1D_Double(bm->RBCestimation.RBCspeciesArray[species].numatage, nselage, 0.0);
    Util_Init_1D_Double(bm->RBCestimation.RBCspeciesArray[species].ages, nselage, 0.0);
    Util_Init_1D_Int(bm->RBCestimation.RBCspeciesArray[species].propgt05, nselage, 0);
    Util_Init_1D_Double(bm->RBCestimation.RBCspeciesArray[species].z, Nfleets * 5, 0.0);   // absolute max size needed

//  calc Z values for fleets X years where agecomps exist, there are enough fully selected ages
//  to calculate slope and where proportion of total catch of that fleet in that year is >=.3
	nz = -1;  // as ++ before use

	for (iy=year-4;iy<=year;iy++){
		for (nf = 0; nf < Nfleets; nf++){
			if((bm->RBCestimation.RBCspeciesArray[species].AgeFltYr[1][nf][iy] > 0) && (bm->RBCestimation.RBCspeciesArray[species].propcatch[nf][iy] > 0.3) && (nselage > 2)) {

                /*
                if (do_debug) {
					fprintf(llogfp,"Assessment %s iy: %d fleet: %d\n", FunctGroupArray[species].groupCode, iy, nf);
                }
                 */
                
				for (a = 0; a < nselage; a++) {
					bm->RBCestimation.RBCspeciesArray[species].numatage[a] = 0;
					bm->RBCestimation.RBCspeciesArray[species].ages[a] = 0;
					bm->RBCestimation.RBCspeciesArray[species].propgt05[a] = 0;
				}
				this_nselage = nselage;    //  sometimes use less ages

				// store numbers at age  - don't use values when get to a run of 3 values with < 5% total nos
				// this is to avoid having long tails in the regression
				ia = -1;  // as ++ before use
				sumnum = 0;
				for (a = AgeSel95; a <= bm->RBCestimation.RBCspeciesParam[species][Tier3_maxage_id]-1; a++){
					ia++;
					// combine sexes
					for (s=0; s< bm->K_num_sexes; s++){
						bm->RBCestimation.RBCspeciesArray[species].numatage[ia] += bm->RBCestimation.RBCspeciesArray[species].AgeComp[nf][s][iy][1][a];
					}
					// the following throws out the first nos at age if nos < nos at next age (to avoid -Z)
					//  make more comparable to doing this by eye
					if ( (a == AgeSel95 + 1 ) && ( bm->RBCestimation.RBCspeciesArray[species].numatage[1] < bm->RBCestimation.RBCspeciesArray[species].numatage[2] ) ){
						ia = ia - 1;
						bm->RBCestimation.RBCspeciesArray[species].numatage[1] = bm->RBCestimation.RBCspeciesArray[species].numatage[2];
						bm->RBCestimation.RBCspeciesArray[species].numatage[2] = 0;
						sumnum = sumnum - (int)bm->RBCestimation.RBCspeciesArray[species].numatage[1];
						this_nselage = nselage - 1;
					}
					sumnum += (int)bm->RBCestimation.RBCspeciesArray[species].numatage[ia];
					bm->RBCestimation.RBCspeciesArray[species].ages[ia] = a;
				}
				for ( ia = 0; ia <= this_nselage; ia++){
					if( (float)(bm->RBCestimation.RBCspeciesArray[species].numatage[ia] / sumnum) > 0.05)
						bm->RBCestimation.RBCspeciesArray[species].propgt05[ia] = 1;
                    
                    /*
                    if (do_debug) {
                        fprintf(llogfp,"Time: %e %s ages-%d: %e, numatage: %e, propt05: %d\n",
                                bm->dayt, FunctGroupArray[species].groupCode, ia, bm->RBCestimation.RBCspeciesArray[species].ages[ia], bm->RBCestimation.RBCspeciesArray[species].numatage[ia], bm->RBCestimation.RBCspeciesArray[species].propgt05[ia]);
                    }
                    */
                    
				}

				ia = 0;  // as ++ before use (was ia = 1 in Sally's code)
				do{
					ia++;
					isum05 = bm->RBCestimation.RBCspeciesArray[species].propgt05[ia-1] + bm->RBCestimation.RBCspeciesArray[species].propgt05[ia] + bm->RBCestimation.RBCspeciesArray[species].propgt05[ia+1];
				}
				while ((ia <= this_nselage-2) && (isum05 > 0));  // change from ia<nselage-2 && isum05>0   14-8-08
				if (isum05 == 0) ia = ia - 1;   // include first value in run of <0.05
				if (ia > this_nselage-2) ia = this_nselage;  // use all selected ages if don't have run of <0.05

				if (ia > 2){
					nz++;
					bm->RBCestimation.RBCspeciesArray[species].z[nz] = -1.0 * getSlope(bm->RBCestimation.RBCspeciesArray[species].ages, bm->RBCestimation.RBCspeciesArray[species].numatage, ia, llogfp);
                    
                    /*
                    if (do_debug) {
                        fprintf(llogfp,"Time: %e %s-%d Z: %e\n",
                                bm->dayt, FunctGroupArray[species].groupCode, ia, bm->RBCestimation.RBCspeciesArray[species].z[nz]);
                    }
                    */
                    
					// total mortality should be > natural mortality and positive
					if ((bm->RBCestimation.RBCspeciesArray[species].z[nz] < bm->RBCestimation.RBCspeciesParam[species][Tier3_M_id]) || (bm->RBCestimation.RBCspeciesArray[species].z[nz] < 0)) nz = nz - 1;
				}
			}
		}
	}

// average Z values over years and fleets
	sumz = 0.0;
	for (i=0; i < nz; i++)
		sumz += bm->RBCestimation.RBCspeciesArray[species].z[i];
	if (nz > 0){
		meanz = sumz / nz;
		bm->RBCestimation.RBCspeciesParam[species][Fcurr_id] =  meanz - bm->RBCestimation.RBCspeciesParam[species][Tier3_M_id] ;
	}
	else
		bm->RBCestimation.RBCspeciesParam[species][AssessFail_id] = TRUE;

    /*
    if (do_debug) {
        fprintf(llogfp,"Time: %e %s nz: %d meanz: %e Fcurr: %e\n",
                 bm->dayt, FunctGroupArray[species].groupCode, nz, meanz, bm->RBCestimation.RBCspeciesParam[species][Fcurr_id]);
    }
     */
    
// calculate Freference values
// calculate weight, length, selectivity and maturity at age for females
	for (a = 0; a<= (bm->RBCestimation.RBCspeciesParam[species][MaxAge_id]-1); a++){
		age = a + 0.5;
		step1 = -1.0 * bm->RBCestimation.RBCspeciesArray[species].Tier3_k[FEMALE] * (age - bm->RBCestimation.RBCspeciesArray[species].Tier3_t0[FEMALE]);
		len = bm->RBCestimation.RBCspeciesArray[species].Tier3_Linf[FEMALE] * (1.0 - exp(step1));
		sp_a = bm->RBCestimation.RBCspeciesParam[species][Tier3_a_id];
		sp_b = bm->RBCestimation.RBCspeciesParam[species][Tier3_b_id];
		bm->RBCestimation.RBCspeciesArray[species].wt[a] = sp_a * pow(len,sp_b);                 // weight in kg
		if ( len > bm->RBCestimation.RBCspeciesParam[species][Tier3_matlen_id])
			bm->RBCestimation.RBCspeciesArray[species].mat[a] = 1.0;
		else
			bm->RBCestimation.RBCspeciesArray[species].mat[a] = 0.0;      // knife-edge maturity at length = matlen

		temp = (len - bm->RBCestimation.RBCspeciesParam[species][Tier3_S50_id])
				/ (bm->RBCestimation.RBCspeciesParam[species][Tier3_S95_id] - bm->RBCestimation.RBCspeciesParam[species][Tier3_S50_id]);

		bm->RBCestimation.RBCspeciesArray[species].agesel[a] = 1.0/(1.0 + exp(-log(19.0) * temp));
	}

	getF20_40_48(bm, species, bm->RBCestimation.RBCspeciesArray[species].agesel, bm->RBCestimation.RBCspeciesArray[species].wt, bm->RBCestimation.RBCspeciesArray[species].mat, bm->RBCestimation.RBCspeciesParam[species][Tier3_M_id], bm->RBCestimation.RBCspeciesParam[species][Tier3_h_id], llogfp);

	return;

}



//******************************************************************************
//
// Name:  getSlope
//      getSlope:         calculate slope of ln(numbers) vs age
// Description: calculate slope of ln(numbers) vs age
//
//
// called by : TierThree_CC or TierFour
// calls :
// created  : Mar 2008 Sally
//
//******************************************************************************
double getSlope(double *x, double *y, int n, FILE *llogfp){
	int i, ni;
	double z, sumx = 0, sumy = 0, sumxy = 0, sumx2 = 0;

	ni = 0;       // number of non-zero obsns
	for (i=0; i<n; i++){
		if (y[i] != 0){
			sumx = sumx + x[i];
			sumy = sumy + log(y[i]);
			sumxy = sumxy + x[i] * log(y[i]);
			sumx2 = sumx2 + x[i] * x[i];
			ni++;
		}
	}

	z = (sumxy - sumx * sumy / ni) / (sumx2 - sumx * sumx / ni);

	return z;

}

//******************************************************************************
//
// Name:  TierThree_CCsel
//      TierThree_CCsel:  calculate current F using Andre's YPR catch curve with selectivity
// Description: calculate current F using Andre's yield per recruit (new Tier 3)
//               catch curves with selectivity (now calling this Equilibrium F method)
//
//
// called by : DoAssessment
// calls :  downhill_simplex, Transform, UnTransform
// created  : April 2008 Sally
//
//******************************************************************************
void TierThree_CCsel(MSEBoxModel *bm, int species, int year, FILE *llogfp){

	int s, a, iy, i, j, nF, nf, nFit, NCA = 0;
	int iter = bm->RBCestimation.Tier_Max_Int;
	double sumage, init_S95, init_S50, step1;
	double ftol;
	double sumF, sumchi;
	double linf,kpar,t0;
	double age,len;
	double Tier3_M = bm->RBCestimation.RBCspeciesParam[species][Tier3_M_id];
	double Tier3_h = bm->RBCestimation.RBCspeciesParam[species][Tier3_h_id];
	double Tier3_a = bm->RBCestimation.RBCspeciesParam[species][Tier3_a_id];
	double Tier3_b = bm->RBCestimation.RBCspeciesParam[species][Tier3_b_id];
	double Tier3_F = bm->RBCestimation.RBCspeciesParam[species][Tier3_F_id];
	double Tier3_matlen = bm->RBCestimation.RBCspeciesParam[species][Tier3_matlen_id];
	double Tier3_S50 = bm->RBCestimation.RBCspeciesParam[species][Tier3_S50_id];
	double Tier3_S95 = bm->RBCestimation.RBCspeciesParam[species][Tier3_S95_id];
	double Nsexes = (double)(bm->K_num_sexes);
	int CCsel_years = (int)bm->RBCestimation.RBCspeciesParam[species][CCsel_years_id];
	int Nfleets = (int)(bm->RBCestimation.RBCspeciesParam[species][NumFisheries_id]);
	int ok, startyear;
    int npar = 3;
    int Tier3maxage = (int)(bm->RBCestimation.RBCspeciesParam[species][Tier3_maxage_id]);
    int CCselyr = (int)(bm->RBCestimation.RBCspeciesParam[species][CCsel_years_id]);

    /*
    int do_debug = 0;
    if((bm->debug == debug_ss3) && (bm->which_check == species)) {
		do_debug = 1;
    }
	if (do_debug) {
		fprintf(llogfp, "Time: %e Tier 3 equilibrium F calculations for %s assessment\n", bm->dayt,FunctGroupArray[species].groupCode);
    }
    */
    
	/* Initialise */
	ftol = 1.0e-4;
    
    Util_Init_2D_Double(bm->RBCestimation.RBCspeciesArray[species].sel, Nfleets, Tier3maxage, 0.0);
    Util_Init_1D_Double(bm->RBCestimation.RBCspeciesArray[species].chi, Nfleets, 0.0);
    Util_Init_1D_Double(bm->RBCestimation.RBCspeciesArray[species].fvalF, Nfleets, 0.0);
    
    npar = 3;
    Util_Init_2D_Double(bm->RBCestimation.RBCspeciesArray[species].xunit, npar+1, npar+1, 0.0);
    Util_Init_1D_Double(bm->RBCestimation.RBCspeciesArray[species].xpar, npar+1, 0.0);
    Util_Init_2D_Double(bm->RBCestimation.RBCspeciesArray[species].simplex, npar+1, npar+1, 0.0);
    Util_Init_1D_Double(bm->RBCestimation.RBCspeciesArray[species].scale, npar+1, 1.0);
    Util_Init_1D_Double(bm->RBCestimation.RBCspeciesArray[species].func, npar+1, 0.0);
    
    Util_Init_1D_Double(bm->RBCestimation.RBCspeciesArray[species].agesel, Tier3maxage, 0.0);
    Util_Init_1D_Double(bm->RBCestimation.RBCspeciesArray[species].mat, Tier3maxage, 0.0);
    Util_Init_1D_Double(bm->RBCestimation.RBCspeciesArray[species].wt, Tier3maxage, 0.0);
    
// average growth parameters over sexes
	linf = 0.0;
	kpar = 0.0;
	t0 = 0.0;
	for (s = 0; s < bm->K_num_sexes; s++){
		linf += bm->RBCestimation.RBCspeciesArray[species].Tier3_Linf[s];
		kpar += bm->RBCestimation.RBCspeciesArray[species].Tier3_k[s];
		t0 += bm->RBCestimation.RBCspeciesArray[species].Tier3_t0[s];
	}
	linf = linf/Nsexes;
	kpar = kpar/Nsexes;
	t0 = t0/Nsexes;

// calc initial age selectivity params from length selectivity and growth
	init_S50 = (-1.0 * log(1-Tier3_S50/linf)/kpar + t0);
	init_S95 = (-1.0 * log(1-Tier3_S95/linf)/kpar + t0);

	nF = -1;  // As starting base zero
	for (nf = 0; nf < Nfleets; nf++){
		// Initalise
        Util_Init_1D_Double(bm->RBCestimation.RBCspeciesArray[species].totnumatage, Tier3maxage, 0.0);
        Util_Init_2D_Double(bm->RBCestimation.RBCspeciesArray[species].Obsageprop, CCselyr, Tier3maxage, 0.0);
        
        startyear = year - CCsel_years + 1;
        if(startyear < 0) startyear = 0;
        bm->RBCestimation.RBCspeciesParam[species][Ncage_id] = -1;  // So then first entry is entry 0
        for (iy = startyear; iy <= year; iy++){
 			if((bm->RBCestimation.RBCspeciesArray[species].AgeFltYr[1][nf][iy]>0) && (bm->RBCestimation.RBCspeciesArray[species].propcatch[nf][iy]> 0.3)) {
				bm->RBCestimation.RBCspeciesParam[species][Ncage_id] += 1;     // number of years of catch-at-age data for this fleet
				NCA = (int)bm->RBCestimation.RBCspeciesParam[species][Ncage_id];
				// store numbers at age
				sumage = 0;
				for (a = 0; a < Tier3maxage; a++){
					// calc num at age over all years used for this fleet
					bm->RBCestimation.RBCspeciesArray[species].totnumatage[a] += bm->RBCestimation.RBCspeciesArray[species].AgeComp[nf][FEMALE][iy][1][a];
					sumage += bm->RBCestimation.RBCspeciesArray[species].AgeComp[nf][FEMALE][iy][1][a];
				}
				for (a = 0; a < Tier3maxage; a++){
					bm->RBCestimation.RBCspeciesArray[species].Obsageprop[NCA][a] = bm->RBCestimation.RBCspeciesArray[species].AgeComp[nf][FEMALE][iy][1][a] / (sumage + small_num);
                    /*
					if (do_debug){
						fprintf(llogfp, "Time: %e, %s f: %d, iy: %d a: %d Obsageprop[%d][%d]: %e sumage: %e (%e)\n",
								bm->dayt, FunctGroupArray[species].groupCode, nf, iy, a, NCA, a, bm->RBCestimation.RBCspeciesArray[species].Obsageprop[NCA][a],
                                sumage, bm->RBCestimation.RBCspeciesArray[species].totnumatage[a]);
                   }
                    */
				}
			}
		}
        
        /*
        if (do_debug) {
            fprintf(llogfp, "Time %e %s NCA: %d\n",
                bm->dayt, FunctGroupArray[species].groupCode, NCA);
       }
        */
        
		if (NCA >= 0){    // only do the minimisation if have data from this fleet
			// call powell to estimate, S50 and S95

			/* Sally deleted code snippet CA was here */

			bm->RBCestimation.RBCspeciesArray[species].simplex[0][0] = Transform(Tier3_F, 0.01, llogfp);
			bm->RBCestimation.RBCspeciesArray[species].simplex[0][1] = init_S95;
			bm->RBCestimation.RBCspeciesArray[species].simplex[0][2] = init_S50;

            Util_Init_2D_Double(bm->RBCestimation.RBCspeciesArray[species].xunit, npar+1, npar+1, 0.0);

			for (i=0; i < npar; i++) {
				for (j=0; j < npar; j++){
					// Initialise
					bm->RBCestimation.RBCspeciesArray[species].xunit[i][j] = 0.0;
					if (i == j) bm->RBCestimation.RBCspeciesArray[species].xunit[i][j] = 1.0;
					bm->RBCestimation.RBCspeciesArray[species].simplex[i+1][j] = bm->RBCestimation.RBCspeciesArray[species].simplex[0][j] + bm->RBCestimation.RBCspeciesArray[species].scale[j] * bm->RBCestimation.RBCspeciesArray[species].xunit[i][j];
				}
			}
            
 			for (i=0; i < npar+1; i++){
				for (j=0; j < npar; j++) {
					bm->RBCestimation.RBCspeciesArray[species].xpar[j] = bm->RBCestimation.RBCspeciesArray[species].simplex[i][j];
				}
                
 				bm->RBCestimation.RBCspeciesArray[species].func[i] = EquilF_Funk(bm, species, bm->RBCestimation.RBCspeciesArray[species].xpar, llogfp);
			}

			downhill_simplex(bm, EquilF_Funk_model, species, bm->RBCestimation.RBCspeciesArray[species].simplex, bm->RBCestimation.RBCspeciesArray[species].func, npar, ftol, iter, &ok, llogfp, bm->RBCestimation.RBCspeciesArray[species].xpar);
			//powell(bm->RBCestimation.RBCspeciesArray[species].xpar,bm->RBCestimation.RBCspeciesArray[species].xunit,npar,ftol,&iter,&ss,EquilF_Funk);

			nF++;
            
            /*
            if (do_debug){
                fprintf(llogfp, "Time: %e %s has nF %d\n", bm->dayt, FunctGroupArray[species].groupCode, nF);
            }
            */
            
			bm->RBCestimation.RBCspeciesArray[species].fvalF[nF] = UnTransform(bm->RBCestimation.RBCspeciesArray[species].simplex[0][0], 0.01);

			if (bm->RBCestimation.RBCspeciesArray[species].fvalF[nF] < 0.05){
				// restart with larger F to ensure not in local minimum
				// very brute-force method !!
				bm->RBCestimation.RBCspeciesArray[species].simplex[0][0] = Transform(Tier3_F+2.0, 0.01, llogfp);
				bm->RBCestimation.RBCspeciesArray[species].simplex[0][1] = init_S95;
				bm->RBCestimation.RBCspeciesArray[species].simplex[0][2] = init_S50;

				for (i=0; i < npar; i++){
					for (j=0; j < npar; j++){
						bm->RBCestimation.RBCspeciesArray[species].simplex[i+1][j] = bm->RBCestimation.RBCspeciesArray[species].simplex[0][j] + bm->RBCestimation.RBCspeciesArray[species].scale[j] * bm->RBCestimation.RBCspeciesArray[species].xunit[i][j];
					}
				}
                
 				for (i=0; i < npar+1; i++){
					for (j=0; j < npar; j++)
						bm->RBCestimation.RBCspeciesArray[species].xpar[j] = bm->RBCestimation.RBCspeciesArray[species].simplex[i][j];
 					bm->RBCestimation.RBCspeciesArray[species].func[i] = EquilF_Funk(bm, species, bm->RBCestimation.RBCspeciesArray[species].xpar, llogfp);
				}

				downhill_simplex(bm, EquilF_Funk_model, species, bm->RBCestimation.RBCspeciesArray[species].simplex, bm->RBCestimation.RBCspeciesArray[species].func, npar, ftol, iter, &ok, llogfp, bm->RBCestimation.RBCspeciesArray[species].xpar);

				bm->RBCestimation.RBCspeciesArray[species].fvalF[nF] = UnTransform(bm->RBCestimation.RBCspeciesArray[species].simplex[0][0], 0.01);   // Nov 2026: was simplex[1][1] (S95 of a non-best vertex)
 
			}

			if (bm->RBCestimation.RBCspeciesArray[species].fvalF[nF] > 7.0)
				bm->RBCestimation.RBCspeciesArray[species].fvalF[nF] = 7.0;
            
 			// calculate selectivity vector for this fleet
			for (a=0; a < Tier3maxage; a++){
				bm->RBCestimation.RBCspeciesArray[species].sel[nF][a] = 1.0 / (1.0 + exp(-log(19) * (a - bm->RBCestimation.RBCspeciesArray[species].simplex[0][2])/(bm->RBCestimation.RBCspeciesArray[species].simplex[0][1] - bm->RBCestimation.RBCspeciesArray[species].simplex[0][2])));
            }

			for (a=0; a < Tier3maxage; a++) {
				if (bm->RBCestimation.RBCspeciesArray[species].sel[nF][a] != 0.0) bm->RBCestimation.RBCspeciesArray[species].chi[nF] += bm->RBCestimation.RBCspeciesArray[species].totnumatage[a] / bm->RBCestimation.RBCspeciesArray[species].sel[nF][a];
            }

		}  // end - (if this fleet has data)
	}  // end (fleets)
    
    // average F values over  fleets that have agecomp data (because each F is estimating same thing)
    
    // Nov 2026: nF is the index of the last fleet fitted (starts at -1), so the number of fleets fitted is nF + 1.
    // Previously looped i < nF and divided by nF, so the last fleet was left out (and Fcurr = 0 with one fleet)
    nFit = nF + 1;
    if(nF < 0) nF = 0;  // reset here to make sense for following calculations, a -1 is not useful and a 0 means a "nothing happened" response here
	sumF = 0.0;
	sumchi = 0.0;
	for (i = 0; i < nFit; i++){
		sumF += bm->RBCestimation.RBCspeciesArray[species].fvalF[i];
		sumchi += bm->RBCestimation.RBCspeciesArray[species].chi[i];
        
        /*
        if (do_debug){
            printf("Time: %e, %s year: %d fleet: %d sumF: %e, sumchi: %e (Fcurr: %e)\n",
                    bm->dayt, FunctGroupArray[species].groupCode, year, i, sumF, sumchi, sumF/(nF + small_num));
            fprintf(llogfp, "Time: %e, %s year: %d fleet: %d sumF: %e, sumchi: %e (Fcurr: %e)\n",
                    bm->dayt, FunctGroupArray[species].groupCode, year, i, sumF, sumchi, sumF/(nF + small_num));
        }
        */
	}
	bm->RBCestimation.RBCspeciesParam[species][Fcurr_id] = sumF/(nFit + small_num);
    
    fprintf(llogfp, "Time %e %s year: %d Fcurr: %e sumF: %e nFit: %d\n",
            bm->dayt, FunctGroupArray[species].groupCode, year, bm->RBCestimation.RBCspeciesParam[species][Fcurr_id], sumF, nFit);

    // combine selectivity vectors over fleets
	for (i = 0; i < nFit; i++){
		bm->RBCestimation.RBCspeciesArray[species].chi[i] = bm->RBCestimation.RBCspeciesArray[species].chi[i] / (sumchi + small_num);
	}
	for (a = 0; a < bm->RBCestimation.RBCspeciesParam[species][Tier3_maxage_id]; a++){
		for (i = 0; i < nFit; i++){
			bm->RBCestimation.RBCspeciesArray[species].agesel[a] += bm->RBCestimation.RBCspeciesArray[species].chi[i] * bm->RBCestimation.RBCspeciesArray[species].sel[i][a];
            
            /*
            if (do_debug){
                printf("Time: %e, %s year: %d a: %d and agesel: %e chi: %e, sel: %e\n", bm->dayt, FunctGroupArray[species].groupCode, year, a, bm->RBCestimation.RBCspeciesArray[species].agesel[a], bm->RBCestimation.RBCspeciesArray[species].chi[i], bm->RBCestimation.RBCspeciesArray[species].sel[i][a]);
                fprintf(llogfp, "Time: %e, %s year: %d a: %d and agesel: %e chi: %e, sel: %e\n", bm->dayt, FunctGroupArray[species].groupCode, year, a, bm->RBCestimation.RBCspeciesArray[species].agesel[a], bm->RBCestimation.RBCspeciesArray[species].chi[i], bm->RBCestimation.RBCspeciesArray[species].sel[i][a]);
            }
            */
		}
	}

    // calculate weight, length, and maturity at age for females
	for (a = 0; a < bm->RBCestimation.RBCspeciesParam[species][Tier3_maxage_id]; a++){
		age = a + 0.5;
		step1 = -1.0 * bm->RBCestimation.RBCspeciesArray[species].Tier3_k[FEMALE] * (age - bm->RBCestimation.RBCspeciesArray[species].Tier3_t0[FEMALE]);
		len = bm->RBCestimation.RBCspeciesArray[species].Tier3_Linf[FEMALE] * (1.0 - exp(step1));
		bm->RBCestimation.RBCspeciesArray[species].wt[a] = Tier3_a * pow(len, Tier3_b);                 // weight in kg
		if ( len > Tier3_matlen )
			bm->RBCestimation.RBCspeciesArray[species].mat[a] = 1.0;
		else
			bm->RBCestimation.RBCspeciesArray[species].mat[a] = 0.0;      // knife-edge maturity at length = matlen
	}

    // calculate Freference values
	getF20_40_48(bm, species, bm->RBCestimation.RBCspeciesArray[species].agesel, bm->RBCestimation.RBCspeciesArray[species].wt, bm->RBCestimation.RBCspeciesArray[species].mat, Tier3_M, Tier3_h, llogfp);



}
//******************************************************************************
//
// Name:  Transform
//      Transform:        transform the input parameter so it is constrained to be >=constraint
// Description: transform the input parameter so it is constrained to be >=constraint
//
//
// called by :  TierThree_CCsel and TierFive_SP
// calls :
// created: April 2008  Sally
// generalised : May 2010
//
//******************************************************************************
double Transform(double par, double constraint, FILE *llogfp)
{
	double transpar;

	transpar = log(par - constraint);

	return transpar;
}

//******************************************************************************
//
// Name:  UnTransform
//      UnTransform :     transforms the input parameter back to original scale
// Description: transforms the input parameter back to original scale
//
//
// called by :  TierThree_CCsel, EquilF_Funk,TierFive_SP, SurplusProduction
// calls :
// created: April 2008  Sally
// generalised : May 2010
//
//******************************************************************************
double UnTransform(double transpar, double constraint)
{
	double par;

	par = exp(transpar) + constraint;

	return par;
}


//******************************************************************************
//
// Name:  EquilF_Funk
//      EquilF_Funk:      the objective function for solving the CCsel
// Description: the objective function for solving the CCsel
//
//  xpar(1) = F, xpar(2) = S95, xpar(3) = S50
//
// called by : powell or downhill_simplex from TierThree_CCsel
// calls :
// created: April 2008  Sally
//
//******************************************************************************

double EquilF_Funk(MSEBoxModel *bm, int species, double *xpar, FILE *llogfp){
	int a,iy;
	double fmort, sumyield, sel, ss;
	double inF, inS95, inS50;
    int Tier3maxage = (int)(bm->RBCestimation.RBCspeciesParam[species][Tier3_maxage_id]);

    /*
    int do_debug = 0;
    if((bm->debug == debug_ss3) && (bm->which_check == species)) {
		do_debug = 1;
    }
     */
    
	inF = xpar[0];
	inS95 = xpar[1];
	inS50 = xpar[2];
    
    Util_Init_1D_Double(bm->RBCestimation.RBCspeciesArray[species].yield, Tier3maxage, 0.0);
    Util_Init_1D_Double(bm->RBCestimation.RBCspeciesArray[species].exp_prop, Tier3maxage, 0.0);
    
    Util_Init_1D_Double(bm->RBCestimation.RBCspeciesArray[species].num, Tier3maxage, 0.0);
    Util_Init_1D_Double(bm->RBCestimation.RBCspeciesArray[species].Funkz, Tier3maxage, 0.0);


	fmort = UnTransform(inF, 0.01);
	bm->RBCestimation.RBCspeciesArray[species].num[0] = 1.0;
	sumyield = 0.0;
	for (a=0; a< bm->RBCestimation.RBCspeciesParam[species][Tier3_maxage_id]; a++){
		sel = 1.0 / (1.0 + exp(-log(19) * (a - inS50) / (inS95 - inS50)));
		bm->RBCestimation.RBCspeciesArray[species].Funkz[a] = bm->RBCestimation.RBCspeciesParam[species][Tier3_M_id] + sel * fmort;
		if ((a < bm->RBCestimation.RBCspeciesParam[species][Tier3_maxage_id]) && (a > 0))
			bm->RBCestimation.RBCspeciesArray[species].num[a] = bm->RBCestimation.RBCspeciesArray[species].num[a-1] * exp(-bm->RBCestimation.RBCspeciesArray[species].Funkz[a-1]);
		else if (a == bm->RBCestimation.RBCspeciesParam[species][Tier3_maxage_id])
			bm->RBCestimation.RBCspeciesArray[species].num[a] = bm->RBCestimation.RBCspeciesArray[species].num[a-1] * exp(-bm->RBCestimation.RBCspeciesArray[species].Funkz[a-1]) / (1.0 - exp(-bm->RBCestimation.RBCspeciesArray[species].Funkz[a-1]));  // the last z(a-1) should be z(a)
		bm->RBCestimation.RBCspeciesArray[species].yield[a] = bm->RBCestimation.RBCspeciesArray[species].num[a] * sel * fmort / bm->RBCestimation.RBCspeciesArray[species].Funkz[a] * (1 - exp(-bm->RBCestimation.RBCspeciesArray[species].Funkz[a]));  // will be equal anyway if fully-selected
//      catch nos a = numbers * fraction dead due to fishing * total no dead
		sumyield += bm->RBCestimation.RBCspeciesArray[species].yield[a];
	}

	for (a=0; a < bm->RBCestimation.RBCspeciesParam[species][Tier3_maxage_id]; a++)
		bm->RBCestimation.RBCspeciesArray[species].exp_prop[a] = bm->RBCestimation.RBCspeciesArray[species].yield[a] / (sumyield + small_num);

	ss = 0.0;
	for (iy=0; iy < bm->RBCestimation.RBCspeciesParam[species][Ncage_id]; iy++){
		for (a=0; a < bm->RBCestimation.RBCspeciesParam[species][Tier3_maxage_id]; a++) {
 			if (bm->RBCestimation.RBCspeciesArray[species].Obsageprop[iy][a] != 0.0)
				ss += bm->RBCestimation.RBCspeciesArray[species].Obsageprop[iy][a] * log( bm->RBCestimation.RBCspeciesArray[species].exp_prop[a] / bm->RBCestimation.RBCspeciesArray[species].Obsageprop[iy][a]);
		}
	}

    /*
	if (do_debug) {
		fprintf(llogfp,"Time: %e %s s50: %e s95: %e F: %e ss: %e\n",
				bm->dayt, FunctGroupArray[species].groupCode, inS50, inS95, fmort, ss);
	}
     */
    
	return -ss;

}

//******************************************************************************
//
// Name:  TierThree_trueF
//      TierThree_trueF:  calculate current almost true F
// Description: calculate current true F (used to test new rule)
//
//
// called by : DoAssessment
// calls :
// created  : June 2008 Sally
//
//******************************************************************************
double TierThree_trueF(MSEBoxModel *bm, int species, int year, double agesel95, FILE *llogfp)
{
	int a;
	double fmax;
    int maxage = (int)(bm->RBCestimation.RBCspeciesParam[species][MaxAge_id]);
    
    Util_Init_1D_Double(bm->RBCestimation.RBCspeciesArray[species].agesel, maxage, 0.0);
    Util_Init_1D_Double(bm->RBCestimation.RBCspeciesArray[species].mat, maxage, 0.0);
    Util_Init_1D_Double(bm->RBCestimation.RBCspeciesArray[species].wt, maxage, 0.0);
    
	fmax = 0;
	for (a=0; a < bm->RBCestimation.RBCspeciesParam[species][MaxAge_id]; a++){
		bm->RBCestimation.RBCspeciesArray[species].fvalA[a] = -log(1.0 - bm->RBCestimation.RBCspeciesArray[species].Uage[0][0][FEMALE][a][year]);    //harvest rate to F. Uage r=0 is combined over region and fleet
		if (bm->RBCestimation.RBCspeciesArray[species].fvalA[a] > fmax) fmax = bm->RBCestimation.RBCspeciesArray[species].fvalA[a];
	}

	//Fcurr = fmax;

	//  calculate F20, F40, F48
	for (a=0; a < bm->RBCestimation.RBCspeciesParam[species][MaxAge_id]; a++){
		bm->RBCestimation.RBCspeciesArray[species].wt[a] = bm->RBCestimation.RBCspeciesArray[species].MeanWtAge[0][FEMALE][a][year];
		bm->RBCestimation.RBCspeciesArray[species].mat[a] = bm->RBCestimation.RBCspeciesArray[species].Fecund[0][a];
		bm->RBCestimation.RBCspeciesArray[species].agesel[a] = bm->RBCestimation.RBCspeciesArray[species].fvalA[a] / fmax;   // this assumes that fish subject to fmax are fully-selected, but for whiting they aren't for trawl fleet so does this work ?
	}

	getF20_40_48(bm, species, bm->RBCestimation.RBCspeciesArray[species].agesel, bm->RBCestimation.RBCspeciesArray[species].wt, bm->RBCestimation.RBCspeciesArray[species].mat, bm->RBCestimation.RBCspeciesArray[species].Mnat[0][0][0][0][year], bm->RBCestimation.RBCspeciesParam[species][Hsteep_id], llogfp);

	return fmax;

}


//******************************************************************************
//
// Name:  Tier3rule
//      Tier3rule:        calculate RBC using the original Tier 3 rule, given Fcurr, Ccur and M
// Description: calculate RBC using the original Tier 3 rule, given Fcurr, Ccur and M
//
//
// called by : DoAssessment
// calls :
// created  : Mar 2008 Sally
//
// Note replaced the fixed constants with variable parameters so for reference
//  tier3RuleA = 0.0295
//  tier3RuleB = 0.2313
//  tier3RuleC = 1.209
//  tier3RuleD = 1.0413
//  tier3RuleE = 0.283
//
//******************************************************************************
void Tier3Rule(MSEBoxModel *bm, int species, int year, double current_catch, FILE *llogfp)
{
	double p,r,mnat;

	mnat = bm->RBCestimation.RBCspeciesArray[species].Mnat[0][0][0][0][year];
	r = bm->RBCestimation.RBCspeciesParam[species][Fcurr_id] / mnat;
	p = bm->RBCestimation.tier3RuleA + (-1.0 * bm->RBCestimation.tier3RuleB * r) + (bm->RBCestimation.tier3RuleC * r * r) + (-1.0 * bm->RBCestimation.tier3RuleD * r * r * r)
			+ (bm->RBCestimation.tier3RuleE * r * r * r * r) + (-1.0 * bm->RBCestimation.tier3RuleF * r * r * r * r * r);
	if (p < 0.0) p = 0.0;
	bm->RBCestimation.RBCspeciesParam[species][RBCest_id] = p * current_catch;

}
//******************************************************************************
//
// Name:  newTier3rule
//      newTier3rule:     calculate RBC using Neil's Tier 3 rule, given Fcurr, Ccur, F20, F40
// Description: calculate RBC using Neil's Tier 3 rule, given Fcurr, Ccur ,F20,F40
//
//
// called by : DoAssessment
// calls :
// created  : May 2008 Sally
//
//******************************************************************************
void newTier3Rule(MSEBoxModel *bm, int species, int year, double current_catch, FILE *llogfp, FILE *fid)
{
	double frbc, step1;
	//int do_debug = 0;

    if (bm->dayt > 10584)
        printf("Applying final step of calc\n");

    /*
    if((bm->debug == debug_ss3) && (bm->which_check == species)) {
		do_debug = 1;
    }
     */
    
	if (bm->RBCestimation.RBCspeciesParam[species][Fcurr_id] >= bm->RBCestimation.RBCspeciesParam[species][F20_id])
		frbc = 0.0;
	else if (bm->RBCestimation.RBCspeciesParam[species][Fcurr_id] <= bm->RBCestimation.RBCspeciesParam[species][F40_id])
		frbc = bm->RBCestimation.RBCspeciesParam[species][F48_id];
	else if (bm->RBCestimation.RBCspeciesParam[species][Tier3_replicate_old_id] > 0) {
		// Original equation - kept so old simulations can be replicated (Tier3_replicate_old_XXX = 1).
		// Note it is not a ramp (units of 1/F), e.g. F20 = 0.4, F40 = 0.2, F48 = 0.15, Fcurr = 0.3 gives frbc = 7.5
		step1 = (bm->RBCestimation.RBCspeciesParam[species][F40_id] - bm->RBCestimation.RBCspeciesParam[species][F20_id])
				* (bm->RBCestimation.RBCspeciesParam[species][Fcurr_id] - bm->RBCestimation.RBCspeciesParam[species][F20_id]);
		frbc = bm->RBCestimation.RBCspeciesParam[species][F48_id] / step1;
	} else {
		// Nov 2026: SESSF Tier 3 ramp - linear from F48 at Fcurr = F40 down to zero at Fcurr = F20
		frbc = bm->RBCestimation.RBCspeciesParam[species][F48_id]
				* (bm->RBCestimation.RBCspeciesParam[species][F20_id] - bm->RBCestimation.RBCspeciesParam[species][Fcurr_id])
				/ (bm->RBCestimation.RBCspeciesParam[species][F20_id] - bm->RBCestimation.RBCspeciesParam[species][F40_id] + small_num);
	}

	bm->RBCestimation.RBCspeciesParam[species][Tier3_mult_id] = (1.0-exp(-frbc)) / (1.0-exp(-bm->RBCestimation.RBCspeciesParam[species][Fcurr_id]));
	// Sally's note: maybe don't need all these restrictions any more ?  2 April 2009
	if (bm->RBCestimation.RBCspeciesParam[species][Tier3_mult_id] > 3.0)
		bm->RBCestimation.RBCspeciesParam[species][Tier3_mult_id] = 3.0;       // do need this one
//	if (bm->RBCestimation.RBCspeciesParam[species][Tier3_mult_id] < 0.25)
//		bm->RBCestimation.RBCspeciesParam[species][Tier3_mult_id] = 0.25;     // mincatch = 5% initial catch
	bm->RBCestimation.RBCspeciesParam[species][RBCest_id] = bm->RBCestimation.RBCspeciesParam[species][Tier3_mult_id] * current_catch;

    /*
	if (do_debug){
		fprintf(llogfp,"Time: %e %s F20: %e F40: %e F48: %e Fcurr: %e frbc: %e RBC: %e current catch : %e\n",
				bm->dayt, FunctGroupArray[species].groupCode, bm->RBCestimation.RBCspeciesParam[species][F20_id],
				bm->RBCestimation.RBCspeciesParam[species][F40_id], bm->RBCestimation.RBCspeciesParam[species][F48_id],
				bm->RBCestimation.RBCspeciesParam[species][Fcurr_id], frbc,
				bm->RBCestimation.RBCspeciesParam[species][RBCest_id], current_catch);
	}
    */
    
    // Write the output to store the result
    fprintf(fid,"%e %s %d %e %e %e %e %e %e %e %e %e %e\n",
            bm->dayt, FunctGroupArray[species].groupCode, year, 0.0, 0.0, 0.0, 0.0,
            bm->RBCestimation.RBCspeciesParam[species][F48_id], bm->RBCestimation.RBCspeciesParam[species][F40_id],
            bm->RBCestimation.RBCspeciesParam[species][F20_id], frbc,
            0.0, bm->RBCestimation.RBCspeciesParam[species][RBCest_id]);

}

//******************************************************************************
//
// Name:  getF20_40_48
//      getF40_F40_F48:   calculate equilibrium F20, F40 and F48 for Tier3
// Description: calculate equilibrium F20, F40 and F48
//
//
// called by : TierThree_CCsel
// calls :
// created  : May 2008 Sally
//
//******************************************************************************
void getF20_40_48(MSEBoxModel *bm, int species, double *agesel, double *wt, double *mat, double natM, double hsteep, FILE *llogfp)
{
	double alpha, beta;
	double fval, spbpr;
    int i, nfint;
    //int a;
    double spb0 = 0, spfrac, spbf, spbf0 = 0;
    //double rec_prog;
	double rec_phd;
	
    /*
    int do_debug = 0;
    if((bm->debug == debug_ss3) && (bm->which_check == species)) {
		do_debug = 1;
    }
     */
    
	alpha = (1.0 - hsteep) / (4.0 * hsteep);    // this is (usual alpha) / B0
	beta = (5.0 * hsteep - 1.0) / (4.0 * hsteep);

    /*
	if (do_debug){
		fprintf(llogfp, "Time %e %s Calculate F20, F40, F48 - steepness: %e B-H stock-recruit alpha: %e B-H stock-recruit beta : %e (natM: %e)\n",
				bm->dayt, FunctGroupArray[species].groupCode, hsteep, alpha, beta, natM);
		for (a=0; a < FunctGroupArray[species].numCohorts; a++) {
			fprintf(llogfp, "Time: %e %s a: %d wt: %e mat: %e agesel: %e\n", bm->dayt, FunctGroupArray[species].groupCode, a, wt[a], mat[a], agesel[a]);
		}
	}
     */
    
	fval = 0.0;
	nfint = (int)(1000 * bm->RBCestimation.RBCspeciesParam[species][Tier3_maxF_id]);   // no of F's over which to calculate YPR

	for (i=0; i < nfint; i++){  // f = i/1000  f goes from 0 to 2
		YPR(bm, species, fval, natM, &spbpr, wt, mat, agesel, llogfp);

		if (i == 0) spb0 = spbpr;
		spfrac = spbpr / spb0;

		// rec_prog = spfrac / (alpha + beta * spfrac); // Used 2008 - 2015
		rec_phd = (spfrac - alpha) / (beta * spfrac);

		// spbf = spbpr * rec_prog; // Used 2008 - 2015
		spbf = spbpr * rec_phd;

		if (i == 0) spbf0 = spbf;

		// F20, F40 F48 - originally 0.2, 0.4, 0.48 was hard wired but now have Blim, BrefB, BrefA used instead so can do flexible ref points under new Australian Harvest Strategies
		if (spbf > (spbf0*FunctGroupArray[species].speciesParams[Blim_id])) bm->RBCestimation.RBCspeciesParam[species][F20_id] = fval;
		if (spbf > (spbf0*FunctGroupArray[species].speciesParams[BrefB_id])) bm->RBCestimation.RBCspeciesParam[species][F40_id] = fval;
		if (spbf > (spbf0*FunctGroupArray[species].speciesParams[BrefA_id])) bm->RBCestimation.RBCspeciesParam[species][F48_id] = fval;

		/*
		if (do_debug){
			fprintf(llogfp, "Time: %e %s fval: %e yieldpr: %e rec_prog: %e rec_phd: %e spbf_prog: %e spbf_phd: %e\n",
				bm->dayt, FunctGroupArray[species].groupCode, fval, yieldpr, rec_prog, rec_phd, spbf_prog, spbf_phd);
		}
		 */
		fval = i / 1000.0;
	}

    /*
	if (do_debug){
		fprintf(llogfp,"Time: %e %s F20: %e F40: %e F48: %e\n",
			bm->dayt, FunctGroupArray[species].groupCode, bm->RBCestimation.RBCspeciesParam[species][F20_id],
			bm->RBCestimation.RBCspeciesParam[species][F40_id],bm->RBCestimation.RBCspeciesParam[species][F48_id]);
	}
     */
}

//******************************************************************************
//
// Name:  YPR
//      YPR:              yield-per-recruit calculations for given F
// Description: yield-per-recruit calculations for given F
//
//
// called by : getF20_40_48
// calls :
// created  : May 2008 Sally
//
//******************************************************************************
void YPR(MSEBoxModel *bm, int species, double fval, double natM, double *spbpr, double *wt, double *mat, double *sel, FILE *llogfp){

	int a;
    
    Util_Init_1D_Double(bm->RBCestimation.RBCspeciesArray[species].num, FunctGroupArray[species].numCohorts, 0.0);
    Util_Init_1D_Double(bm->RBCestimation.RBCspeciesArray[species].Funkz, FunctGroupArray[species].numCohorts, 0.0);

	*spbpr = 0.0;

	for (a=0; a < FunctGroupArray[species].numCohorts; a++)
		bm->RBCestimation.RBCspeciesArray[species].Funkz[a] = natM + sel[a] * fval;

	bm->RBCestimation.RBCspeciesArray[species].num[0] = 1.0;

    //  calculate numbers at age relative to age 0
	for (a=1; a < FunctGroupArray[species].numCohorts-1; a++)
		bm->RBCestimation.RBCspeciesArray[species].num[a] = bm->RBCestimation.RBCspeciesArray[species].num[a-1] * exp(-bm->RBCestimation.RBCspeciesArray[species].Funkz[a-1]);
	// plus group
	bm->RBCestimation.RBCspeciesArray[species].num[FunctGroupArray[species].numCohorts-1] = bm->RBCestimation.RBCspeciesArray[species].num[FunctGroupArray[species].numCohorts-2] * exp(-bm->RBCestimation.RBCspeciesArray[species].Funkz[FunctGroupArray[species].numCohorts-2])/(1.0 - exp(-bm->RBCestimation.RBCspeciesArray[species].Funkz[FunctGroupArray[species].numCohorts-1]));

    // YPR
	for (a=0; a< FunctGroupArray[species].numCohorts; a++)
		*spbpr = *spbpr + bm->RBCestimation.RBCspeciesArray[species].num[a] * wt[a] * mat[a];

}

//******************************************************************************
//
// Name: Call_Tier3
//
// called by: Tiers 6-7 (RUSS style)
// calls: tier3 routines
// created: Oct 2014 Beth Fulton
//
//******************************************************************************
void Call_Tier3(MSEBoxModel *bm, int species, int year, FILE *llogfp) {
    int AgeSel95 = (int)bm->RBCestimation.RBCspeciesParam[species][AgeSel95_id];
    int Nregions = (int)(bm->RBCestimation.RBCspeciesParam[species][NumRegions_id]);
	int tiertype = (int) (bm->RBCestimation.RBCspeciesParam[species][tiertype_id]);
    double current_catch = 0;
    int nf, nreg;

    bm->RBCestimation.RBCspeciesParam[species][True_F_id] = TierThree_trueF(bm, species, year, AgeSel95, llogfp);
    bm->RBCestimation.RBCspeciesParam[species][True_F48_id] = bm->RBCestimation.RBCspeciesParam[species][F48_id];
    //-------- calculate current F  (global Fcurr)
    
    switch ((int)(bm->RBCestimation.RBCspeciesParam[species][Tier3_Fcalc_id])){
		case 1: // ----- catch curves (old T3)
			if ( AgeSel95 > (FunctGroupArray[species].numCohorts-1)) {
				quit("Catch curves will not work for this species because age at full selectivity > maximum age\n");
			} else {
				TierThree_CC(bm, species, year, llogfp);
			}
			break;
		case 2: // ----- catch curves with selectivity  (new T3)
			TierThree_CCsel(bm, species, year, llogfp);
			break;
		case 3:  // ----- true F
			bm->RBCestimation.RBCspeciesParam[species][Fcurr_id] = bm->RBCestimation.RBCspeciesParam[species][True_F_id];
			// if using true F use only final year's catch + true discard ***************
			current_catch = 0.0;
			for (nf = 0; nf < bm->K_num_fisheries; nf++)
				for (nreg = 0; nreg < Nregions; nreg++)
					current_catch += Harvest_Get_TotCumCatch(species, nf, bm->thisyear);   // True catchs and discards
			break;
    }
    
    // ---------  apply tier 3 rule to get RBC
    if (tiertype == orig_tier_rule) {   // original rule
        Tier3Rule(bm, species, year, current_catch, llogfp);
    } else if (tiertype == new_tier_rule) {    // Neils' new rule
        newTier3Rule(bm, species, year, current_catch, llogfp, tier5fp);
    }
    
    return;
}

//******************************************************************************
//
// Name:  TierFour
//      TierFour:         do an old-style tier 4 assessment and get RBC for next year
// Description: do a tier 4 assessment and get RBC for next year (old T4)
//
//
// called by : DoAssessment
// calls :
// created  : May 2008 Sally
//
//******************************************************************************
void TierFour (MSEBoxModel *bm, int species, int year, double current_catch, FILE *llogfp)
{
	int iy, nf, maxfleet, ii;
	double slope, maxprop;
	//int do_debug = 0;
	int Nfleets = (int)(bm->RBCestimation.RBCspeciesParam[species][NumFisheries_id]);
	int Nregions = (int)(bm->RBCestimation.RBCspeciesParam[species][NumRegions_id]);   // Index Nregions holds the catch weighted all-region CPUE

    Util_Init_1D_Double(bm->RBCestimation.RBCspeciesArray[species].xyear, 4, 0.0);
    Util_Init_1D_Double(bm->RBCestimation.RBCspeciesArray[species].ycpue, 4, 0.0);

    /*
    if((bm->debug == debug_ss3) && (bm->which_check == species)) {
		do_debug = 1;
    }
    */
     
// which fleet has highest proportion of catch (av over last 5 years) ?
	// 17 May 2010 - should check whether CPUE exists for these years and fleets
	maxprop = bm->RBCestimation.RBCspeciesArray[species].avprop[1];
	maxfleet = 1;
	for (nf = 0; nf < Nfleets; nf++)
		if (bm->RBCestimation.RBCspeciesArray[species].avprop[nf] > maxprop){
			maxprop = bm->RBCestimation.RBCspeciesArray[species].avprop[nf];
			maxfleet = nf;
		}

// get slope of last 4 years Ln(CPUE)
	ii = -1;  // as ++ before used
	for (iy = year-3; iy <= year; iy++){
		ii++;
		bm->RBCestimation.RBCspeciesArray[species].xyear[ii] = ii;
		bm->RBCestimation.RBCspeciesArray[species].ycpue[ii] = bm->RBCestimation.RBCspeciesArray[species].CPUEgen[maxfleet][Nregions][iy];   // Nov 2026: was region 0
	}
	slope = getSlope(bm->RBCestimation.RBCspeciesArray[species].xyear, bm->RBCestimation.RBCspeciesArray[species].ycpue, 4, llogfp);

	bm->RBCestimation.RBCspeciesParam[species][RBCest_id] = (1 + bm->RBCestimation.RBCspeciesParam[species][Tier4_alpha_id] * slope) * current_catch;

    /*
	if (do_debug){
		fprintf(llogfp,"Time: %e %s Fleet for cpue: %d\n", bm->dayt, FunctGroupArray[species].groupCode, maxfleet);
		for (ii=0; ii < 4; ii++)
			fprintf(llogfp,"Time: %e %s ii: %d ycpue: %e\n", bm->dayt, FunctGroupArray[species].groupCode, ii, bm->RBCestimation.RBCspeciesArray[species].ycpue[ii]);
		fprintf(llogfp,"Time: %e %s current catch: %e cpue slope: %e\n",
			bm->dayt, FunctGroupArray[species].groupCode, current_catch, slope);
	}
    */
}

//******************************************************************************
//
// Name:  newTierFour
//      newTierFour:      do a tier 4 assessment using Rich's new rule and get RBC for next year
// Description: do a tier 4 assessment using Rich's new rule and get RBC for next year
//
//
// called by : DoAssessment
// calls :
// created  : May 2008 Sally
//
//******************************************************************************
void newTierFour (MSEBoxModel *bm, int species, int year, double current_catch, FILE *llogfp){
	int iy, region, r, nf, MinCyr, MaxCyr, nsum;
	double cpuesum, maxprop, mult, step1, denom;
	int fleetid, startyear;
	int Nregions = (int)(bm->RBCestimation.RBCspeciesParam[species][NumRegions_id]);
	int Nfleets = (int)(bm->RBCestimation.RBCspeciesParam[species][NumFisheries_id]);

    /*
    int do_debug = 0;
    if((bm->debug == debug_ss3) && (bm->which_check == species)) {
		do_debug = 1;
    }
    */
        
	region = Nregions;   // weighted sum over regions (stored at index Nregions), assessment doesn't know about regions - Nov 2026: was 0 (i.e. region 0 only)
	// which fleet has highest proportion of catch (av over last 5 years) ?
	// 22.3.2011 only do this at start of projection
	if (year == (bm->RBCestimation.RBCspeciesParam[species][HistYrMax_id] - 1)){
		maxprop = 0;
		bm->RBCestimation.RBCspeciesParam[species][CPUEfleet_id] = 0;
		for (nf = 0; nf < Nfleets; nf++)
			if (bm->RBCestimation.RBCspeciesArray[species].avprop[nf] > maxprop){
				maxprop = bm->RBCestimation.RBCspeciesArray[species].avprop[nf];
				bm->RBCestimation.RBCspeciesParam[species][CPUEfleet_id] = nf;
			}
	}
	fleetid = (int)(bm->RBCestimation.RBCspeciesParam[species][CPUEfleet_id]);

	// calculate CPUE target and limit
	cpuesum = 0.0;
    
    MinCyr = (int)(bm->RBCestimation.RBCspeciesParam[species][Tier4_CPUEyrmin_id]);
    MaxCyr = (int)(bm->RBCestimation.RBCspeciesParam[species][Tier4_CPUEyrmax_id]);
    // Sanity check
    if (MaxCyr < 1) MaxCyr = 1;
    if (MaxCyr > year) MaxCyr = year;
    if (MinCyr < 1) MinCyr = 1;
    if (MinCyr > MaxCyr) MaxCyr = MinCyr;

    nsum = 0;
	for (iy = MinCyr; iy < MaxCyr + 1; iy++){   // Nov 2026: now includes MaxCyr (was iy < MaxCyr but divided by the full number of years)
		cpuesum += bm->RBCestimation.RBCspeciesArray[species].CPUEgen[fleetid][region][iy];
        nsum++;
        
        /**/
        //if (do_debug) {
            fprintf(llogfp, "Time: %e %s iy: %d cpuesum: %e cpuegen: %e (fleet: %d, region: %d)\n",
                    bm->dayt, FunctGroupArray[species].groupCode, year, cpuesum, bm->RBCestimation.RBCspeciesArray[species].CPUEgen[fleetid][region][iy], fleetid, region);
        //}
        /**/
	}
	step1 = (double)nsum;   // Number of years actually summed
	bm->RBCestimation.RBCspeciesParam[species][CPUEtarg_id] = bm->RBCestimation.RBCspeciesParam[species][Tier4_Bo_correct_id] * cpuesum / (step1 + small_num);

	bm->RBCestimation.RBCspeciesParam[species][CPUElim_id] = bm->RBCestimation.CPUElim * bm->RBCestimation.RBCspeciesParam[species][CPUEtarg_id];

    /*
    if (do_debug) {
        fprintf(llogfp, "Time: %e %s iy: %d step1: %e CPUEtarg: %e, CPUElim: %e (baselim: %e)\n",
                bm->dayt, FunctGroupArray[species].groupCode, year, step1, bm->RBCestimation.RBCspeciesParam[species][CPUEtarg_id], bm->RBCestimation.RBCspeciesParam[species][CPUElim_id], bm->RBCestimation.CPUElim);
    }
    */
    
	bm->RBCestimation.RBCspeciesParam[species][Cmax_id] = current_catch * (1.0 + bm->RBCestimation.RBCspeciesParam[species][Tier4_Cmaxmult_id]);

	// calculate average CPUE over last m years
	cpuesum = 0.0;
    startyear = year - (int)bm->RBCestimation.RBCspeciesParam[species][Tier4_m_id] + 1;   // Nov 2026: +1 restored so m years are used (was m+1 years divided by m)
    if (startyear < 0) startyear = 0;
	for (iy = startyear; iy < year + 1; iy++){
		cpuesum += bm->RBCestimation.RBCspeciesArray[species].CPUEgen[fleetid][region][iy];
       
        /**/
        //if (do_debug) {
            fprintf(llogfp, "newTierFour Time: %e %s iy: %d year: %d cpuesum: %e cpuegen: %e (fleet: %d, region: %d)\n",
                    bm->dayt, FunctGroupArray[species].groupCode, iy, year, cpuesum, bm->RBCestimation.RBCspeciesArray[species].CPUEgen[fleetid][region][iy], fleetid, region);
        //}
        /**/
	}
    denom = (double)(year - startyear + 1);   // Number of years summed

    bm->RBCestimation.RBCspeciesParam[species][CPUEav_id] = cpuesum / (denom + small_num);

	// RBC
	if (bm->RBCestimation.RBCspeciesParam[species][CPUEav_id] < bm->RBCestimation.RBCspeciesParam[species][CPUElim_id]) {
		mult = 0.0;
        step1 = 0.0;
	} else {
		step1 = bm->RBCestimation.RBCspeciesParam[species][CPUEtarg_id] - bm->RBCestimation.RBCspeciesParam[species][CPUElim_id];
		mult = (bm->RBCestimation.RBCspeciesParam[species][CPUEav_id] - bm->RBCestimation.RBCspeciesParam[species][CPUElim_id]) / (step1 + small_num);
	}

	bm->RBCestimation.RBCspeciesParam[species][RBCest_id] = current_catch * mult;

    // Optional Ratpack scaling of RBC by the HCR target relative to the default 0.48 (Ratpack _TierFour)
    if (bm->RBCestimation.RBCspeciesParam[species][Tier4_UseHCRtarget_id] > 0) {
        bm->RBCestimation.RBCspeciesParam[species][RBCest_id] *= bm->RBCestimation.RBCspeciesParam[species][mgt_HCRtarget_id] / 0.48;
    }

	if (bm->RBCestimation.RBCspeciesParam[species][RBCest_id] > bm->RBCestimation.RBCspeciesParam[species][Cmax_id])
		bm->RBCestimation.RBCspeciesParam[species][RBCest_id] = bm->RBCestimation.RBCspeciesParam[species][Cmax_id];

    // Optional Ratpack absolute maximum catch cap (Run_Tier4b Max.catch)
    if ((bm->RBCestimation.RBCspeciesParam[species][Tier4_UseMaxCatch_id] > 0) &&
        (bm->RBCestimation.RBCspeciesParam[species][RBCest_id] > bm->RBCestimation.RBCspeciesParam[species][Tier4_MaxCatch_id])) {
        bm->RBCestimation.RBCspeciesParam[species][RBCest_id] = bm->RBCestimation.RBCspeciesParam[species][Tier4_MaxCatch_id];
    }

    bm->RBCestimation.RBCspeciesParam[species][TACpost_id] = bm->RBCestimation.RBCspeciesParam[species][RBCest_id];

    /**/
    //if (do_debug) {
    //if (isnan(bm->RBCestimation.RBCspeciesParam[species][TACpost_id])) {
		fprintf(llogfp, "newTierFour Time: %e %s year %d RBCest: %e current_catch: %e mult: %e step1: %e CPUEav: %e CPUElim: %e CPUEtarg: %e Cmax: %e cpuesum: %e, denom: %e\n",
                bm->dayt, FunctGroupArray[species].groupCode, year, bm->RBCestimation.RBCspeciesParam[species][RBCest_id], current_catch, mult, step1,
                bm->RBCestimation.RBCspeciesParam[species][CPUEav_id], bm->RBCestimation.RBCspeciesParam[species][CPUElim_id],
                bm->RBCestimation.RBCspeciesParam[species][CPUEtarg_id], bm->RBCestimation.RBCspeciesParam[species][Cmax_id], cpuesum, denom);
    //}
    /**/
    
	// write CPUE history for each region. region=0 is sum over regions
    if (year == (bm->RBCestimation.RBCspeciesParam[species][HistYrMax_id]-1)) {
        for (r=0; r < Nregions; r++) {
			WriteCPUE(bm, species, r, year, histCPUEfp);
        }
    }

	// for log file
	bm->RBCestimation.RBCspeciesParam[species][Cmult_id] = mult;
	bm->RBCestimation.RBCspeciesParam[species][CCcur_id] = current_catch;

    /*
	if (do_debug) {
		fprintf(llogfp, "Time: %e %s CPUEfleet: %e CPUElim: %e CPUEtarg: %e Cmax: %e CPUEav: %e mult: %e current_catch: %e RBC: %e\n",
				bm->dayt, FunctGroupArray[species].groupCode,bm->RBCestimation.RBCspeciesParam[species][CPUEfleet_id],
				bm->RBCestimation.RBCspeciesParam[species][CPUElim_id], bm->RBCestimation.RBCspeciesParam[species][CPUEtarg_id],
				bm->RBCestimation.RBCspeciesParam[species][Cmax_id], bm->RBCestimation.RBCspeciesParam[species][CPUEav_id],
				mult, current_catch, bm->RBCestimation.RBCspeciesParam[species][RBCest_id]);
	}
    */
}




//******************************************************************************
//
// Name:  WriteCPUE
//      WriteCPUE:        write CPUE by region for plotting
// Description: write CPUE by region for plotting

//
// called by : newTierFour
// calls :
// created  : Nov 2010 Sally
//
// Note fid is the HistCPUE file TODO: create this file
//
//******************************************************************************

void WriteCPUE(MSEBoxModel *bm, int species, int region, int year, FILE *fid)
{
	int iy;
	double step1;
	int fleetid = (int)(bm->RBCestimation.RBCspeciesParam[species][CPUEfleet_id]);

    int MinCyr = (int)(bm->RBCestimation.RBCspeciesParam[species][Tier4_CPUEyrmin_id]);
    int MaxCyr = (int)(bm->RBCestimation.RBCspeciesParam[species][Tier4_CPUEyrmax_id]);
    // Sanity check
    if (MaxCyr < 1) MaxCyr = 1;
    if (MaxCyr > year) MaxCyr = year;
    if (MinCyr < 1) MinCyr = 1;
    if (MinCyr > MaxCyr) MaxCyr = MinCyr;

	for (iy = MinCyr; iy < MaxCyr; iy++){
		step1 = bm->RBCestimation.RBCspeciesArray[species].CPUEgen[fleetid][region][iy];
		fprintf(fid, "%e %s %d %d %e %e %e\n",
			bm->dayt, FunctGroupArray[species].groupCode, region, iy, step1,
			bm->RBCestimation.RBCspeciesParam[species][CPUEfleet_id], bm->RBCestimation.RBCspeciesParam[species][CPUEtarg_id]);
	}
}


//******************************************************************************
//
// Name: TierFive_SP:      do a tier 5 assessment - surplus production model - and get RBC for next year
// Description: do a tier 5 assessment - surplus production model
//              and get RBC for next year
//
// created  : Apr 2010 Sally
//
// note fid is the tier_RBC reporting file TODO: Create this file
//
//******************************************************************************
void TierFive_SP(MSEBoxModel *bm, int species, int year, FILE *llogfp, FILE *fid)
{
	int i, j, npar, iy, sumregion, minyr, nf;
	int iter = bm->RBCestimation.Tier_Max_Int;
	int ok;
    double ftol;
    //double step1;
	double bstart, k, r, current_depletion, ftarg;
	double discy, catchy;
	//double funcy;
	//int Tier5_flt = (int)(bm->RBCestimation.RBCspeciesParam[species][Tier5_flt_id]);
	//int Tier5_reg = (int)(bm->RBCestimation.RBCspeciesParam[species][Tier5_reg_id]);
	int Nfleets = (int)(bm->RBCestimation.RBCspeciesParam[species][NumFisheries_id]);
    int Nregions = (int)(bm->RBCestimation.RBCspeciesParam[species][NumRegions_id]);

    /*
    int do_debug = 1;
    if((bm->debug == debug_ss3) && (bm->which_check == species)) {
		do_debug = 1;
    }
    */
    
	sumregion = Nregions;  // sum over regions
    
	// inputs for downhill simplex
	// parameters are Bstart, K, r

	npar = 3;

    Util_Init_2D_Double(bm->RBCestimation.RBCspeciesArray[species].xunit, npar+1, npar+1, 0.0);
    Util_Init_1D_Double(bm->RBCestimation.RBCspeciesArray[species].xpar, npar+1, 0.0);
    Util_Init_2D_Double(bm->RBCestimation.RBCspeciesArray[species].simplex, npar+1, npar+1, 0.0);
    Util_Init_1D_Double(bm->RBCestimation.RBCspeciesArray[species].scale, npar+1, 1.0);
    Util_Init_1D_Double(bm->RBCestimation.RBCspeciesArray[species].func, npar+1, 0.0);
    
    bm->RBCestimation.RBCspeciesArray[species].scale[0] = 1000;
	bm->RBCestimation.RBCspeciesArray[species].scale[1] = 1000;
	bm->RBCestimation.RBCspeciesArray[species].scale[2] = 0.1;
	ftol = 1.0e-4;

// get total removals if first year of this simulation
// same within simulation, but can be different between sims because DiscData is generated
	if (bm->RBCestimation.RBCspeciesArray[species].CurrentYear == (bm->RBCestimation.RBCspeciesParam[species][HistYrMax_id]-1))
		minyr = (int)bm->RBCestimation.RBCspeciesParam[species][HistYrMin_id];
	else
		minyr = bm->RBCestimation.RBCspeciesArray[species].CurrentYear - 1;
    
    fprintf(llogfp, "Time: %e %s year: %d minyr: %d currentyear %d\n",
            bm->dayt, FunctGroupArray[species].groupCode, year, minyr, bm->RBCestimation.RBCspeciesArray[species].CurrentYear);
    
	for (iy=minyr; iy < (bm->RBCestimation.RBCspeciesArray[species].CurrentYear + 1); iy++){
		// get total removals for this year
		catchy = 0;
		discy = 0;
		for (nf = 0; nf < Nfleets; nf++){
			if (bm->RBCestimation.RBCspeciesParam[species][DiscType_id] == 1)      // discard data is biomass
				discy += bm->RBCestimation.RBCspeciesArray[species].DiscData[nf][sumregion][iy];
			else if (bm->RBCestimation.RBCspeciesParam[species][DiscType_id] == 2)  // discard data is rate
				discy += bm->RBCestimation.RBCspeciesArray[species].DiscData[nf][sumregion][iy] * bm->RBCestimation.RBCspeciesArray[species].CatchData[nf][sumregion][iy] / (1.0 - bm->RBCestimation.RBCspeciesArray[species].DiscData[nf][sumregion][iy]);
			catchy += bm->RBCestimation.RBCspeciesArray[species].CatchData[nf][sumregion][iy];
		}
		bm->RBCestimation.RBCspeciesArray[species].T5_removal[iy] = discy + catchy;
		
        /*
        if (do_debug){
			step1 = bm->RBCestimation.RBCspeciesArray[species].CPUEgen[Tier5_flt][Tier5_reg][iy];
			fprintf(llogfp, "Time: %e %s iy: %d T5_removal: %e step1 %e\n",
				bm->dayt, FunctGroupArray[species].groupCode, iy, bm->RBCestimation.RBCspeciesArray[species].T5_removal[iy], step1);
		}
        */
	}

// set up initial values of parameters
	bm->RBCestimation.RBCspeciesArray[species].simplex[0][0] = bm->RBCestimation.RBCspeciesArray[species].Tier5_Bstart[0];
	bm->RBCestimation.RBCspeciesArray[species].simplex[0][1] = bm->RBCestimation.RBCspeciesArray[species].Tier5_Bstart[0];
	bm->RBCestimation.RBCspeciesArray[species].simplex[0][2] = Transform(bm->RBCestimation.RBCspeciesArray[species].Tier5_r[0], 0.0, llogfp);     // transform to constrain r > 0

	for (i=0; i < npar; i++){
		for (j=0; j < npar; j++){
			if (i==j) bm->RBCestimation.RBCspeciesArray[species].xunit[i][j] = 1.0;
			bm->RBCestimation.RBCspeciesArray[species].simplex[i+1][j] = bm->RBCestimation.RBCspeciesArray[species].simplex[0][j] + bm->RBCestimation.RBCspeciesArray[species].scale[j] * bm->RBCestimation.RBCspeciesArray[species].xunit[i][j];
		}
	}
	for (i=0; i < npar+1; i++){
		for (j=0; j < npar; j++)
			bm->RBCestimation.RBCspeciesArray[species].xpar[j] = bm->RBCestimation.RBCspeciesArray[species].simplex[i][j];
		bm->RBCestimation.RBCspeciesArray[species].func[i] = SurplusProduction(bm, species, bm->RBCestimation.RBCspeciesArray[species].xpar, llogfp);
	}


// call simplex to estimate Bstart, K, r
	downhill_simplex(bm, SurplusProduction_model, species, bm->RBCestimation.RBCspeciesArray[species].simplex, bm->RBCestimation.RBCspeciesArray[species].func, npar, ftol, iter, &ok, llogfp, bm->RBCestimation.RBCspeciesArray[species].xpar);

	/* Nov 2026: K is now its own parameter (simplex[0][1]). Original handling, in case need to switch back:
	   K was set equal to Bstart, i.e.  k = simplex[0][0]  (and xpar[1] below = simplex[0][0], in_K = xpar[0] in SurplusProduction) */
	bstart = bm->RBCestimation.RBCspeciesArray[species].simplex[0][0];
	k = bm->RBCestimation.RBCspeciesArray[species].simplex[0][1];
	r = UnTransform(bm->RBCestimation.RBCspeciesArray[species].simplex[0][2], 0.0);

	bm->RBCestimation.RBCspeciesArray[species].xpar[0] = bm->RBCestimation.RBCspeciesArray[species].simplex[0][0];
	bm->RBCestimation.RBCspeciesArray[species].xpar[1] = bm->RBCestimation.RBCspeciesArray[species].simplex[0][1];   // Nov 2026: was simplex[0][0] (K = Bstart)
	bm->RBCestimation.RBCspeciesArray[species].xpar[2] = bm->RBCestimation.RBCspeciesArray[species].simplex[0][2];
	//funcy = SurplusProduction(bm, species, xpar, llogfp);

	if (!ok || (r <= bm->RBCestimation.RBCspeciesArray[species].Tier5_r[1])
			|| (r >= bm->RBCestimation.RBCspeciesArray[species].Tier5_r[2])
			|| (bstart <= bm->RBCestimation.RBCspeciesArray[species].Tier5_Bstart[1])
			|| (bstart >= bm->RBCestimation.RBCspeciesArray[species].Tier5_Bstart[2])){
		bm->RBCestimation.RBCspeciesParam[species][AssessFail_id] = TRUE;
	}
	current_depletion = bm->RBCestimation.RBCspeciesParam[species][BcurrSP_id] / k;

	// calculate F20, F40 and F48
	get_T5_F20_40_48(bm, species, bstart, k, r, llogfp);

	// now apply the 20:35:48 harvest control rule
	if (current_depletion<0.2)
		ftarg = 0.0;
	else if (current_depletion<0.35)
		ftarg = bm->RBCestimation.RBCspeciesParam[species][F40_id] * (current_depletion / 0.2 -1.0);
	else
		ftarg = bm->RBCestimation.RBCspeciesParam[species][F48_id];

	// now calculate RBC
	bm->RBCestimation.RBCspeciesParam[species][RBCest_id] = bm->RBCestimation.RBCspeciesParam[species][BcurrSP_id] * (1-exp(-ftarg));
    
	fprintf(llogfp, "Time %e %s year %d bstart %e k %e r %e current_depletion %e F48 %e F40 %e F20 %e ftarg %e BcurrSP %e RBCest %e\n",
            bm->dayt, FunctGroupArray[species].groupCode, year, bstart, k, r,
            current_depletion, bm->RBCestimation.RBCspeciesParam[species][F48_id],
            bm->RBCestimation.RBCspeciesParam[species][F40_id], bm->RBCestimation.RBCspeciesParam[species][F20_id],
            ftarg, bm->RBCestimation.RBCspeciesParam[species][BcurrSP_id],
            bm->RBCestimation.RBCspeciesParam[species][RBCest_id]);
    

	// Write the output to store the result
	fprintf(fid,"%e %s %d %e %e %e %e %e %e %e %e %e %e\n",
		bm->dayt, FunctGroupArray[species].groupCode, year, bstart, k, r,
		current_depletion, bm->RBCestimation.RBCspeciesParam[species][F48_id],
		bm->RBCestimation.RBCspeciesParam[species][F40_id], bm->RBCestimation.RBCspeciesParam[species][F20_id],
		ftarg, bm->RBCestimation.RBCspeciesParam[species][BcurrSP_id],
		bm->RBCestimation.RBCspeciesParam[species][RBCest_id]);

	return;
}
//******************************************************************************
//
// Name:  SurplusProduction: the objective function for the Surplus production model
// Description: the objective function for the Surplus production model
//  can go to start of Currentyear+1 becuase know catch in Currentyear
//
//  xpar[0] = Bstart, xpar[1] = K, xpar[2] = r
//
// called by : powell or downhill_simplex from TierFive_SP
// created: April 2010  Sally
//
//******************************************************************************
double SurplusProduction(MSEBoxModel *bm, int species, double *xpar, FILE *llogfp)
{
	int iy, nq, index1M, index2M;
	//int sumregion = 0;  // index for sum of catches over regions
	double in_Bstart, in_K, in_r, step1, step2;
	double bt,q,sigma,diff2,expect_cpue,sumq,ll;
	int Tier5_flt = (int)(bm->RBCestimation.RBCspeciesParam[species][Tier5_flt_id]);
	int Tier5_reg = (int)(bm->RBCestimation.RBCspeciesParam[species][Tier5_reg_id]);
    int index1, index2;

    // So got meaningful data as assessing as go here
    if(bm->RBCestimation.AssessDelay > 1) {
        index1 = bm->RBCestimation.RBCspeciesArray[species].CurrentYear+1;
        index2 = bm->RBCestimation.RBCspeciesArray[species].CurrentYear+2;
    } else if (bm->RBCestimation.AssessDelay) {
        index1 = bm->RBCestimation.RBCspeciesArray[species].CurrentYear+1;
        index2 = bm->RBCestimation.RBCspeciesArray[species].CurrentYear+1;
    } else {
        index1 = bm->RBCestimation.RBCspeciesArray[species].CurrentYear;
        index2 = bm->RBCestimation.RBCspeciesArray[species].CurrentYear;
    }
    
    index1M = index1;
    if(!index1)
        index1M = 1;
    index2M = index2;
    if(!index2)
        index2M = 1;

    Util_Init_1D_Double(bm->RBCestimation.RBCspeciesArray[species].expectB, index2M, 0.0);
	Util_Init_1D_Double(bm->RBCestimation.RBCspeciesArray[species].expectq, index1M, 0.0);

	nq = 0;
	sumq = 0.0;
	sigma = 0.0;

	in_Bstart = xpar[0];
	in_K = xpar[1];   // Nov 2026: was xpar[0], i.e. K = Bstart (switch back to xpar[0] to replicate the original handling)
	in_r = UnTransform(xpar[2], 0);

	// can only do this to CurrentYear because only have CPUE up to this year
	for (iy=(int)bm->RBCestimation.RBCspeciesParam[species][HistYrMin_id]; iy < (int)bm->RBCestimation.RBCspeciesArray[species].CurrentYear; iy++){
		if (iy==bm->RBCestimation.RBCspeciesParam[species][HistYrMin_id])
			bm->RBCestimation.RBCspeciesArray[species].expectB[iy] = in_Bstart;
		else {
			bt = bm->RBCestimation.RBCspeciesArray[species].expectB[iy-1];
			/* Nov 2026: removals now subtracted after the production term (Pella-Tomlinson B + (r/p)B(1-(B/K)^p) - C).
			   Original handling, in case need to switch back - removals were inside the production term:
			     step1 = (1.0 - exp(p * log(bt/in_K))) - T5_removal[iy-1];
			     expectB[iy] = bt + bt * in_r / p * step1;  */
			step1 = 1.0 - exp(bm->RBCestimation.RBCspeciesParam[species][Tier5_p_id] * log(bt/in_K));
			bm->RBCestimation.RBCspeciesArray[species].expectB[iy] = bt + bt * in_r / bm->RBCestimation.RBCspeciesParam[species][Tier5_p_id] * step1 - bm->RBCestimation.RBCspeciesArray[species].T5_removal[iy-1];
			if (bm->RBCestimation.RBCspeciesArray[species].expectB[iy] < 75.0) bm->RBCestimation.RBCspeciesArray[species].expectB[iy] = 75.0;
		}
		// if there is a cpue for this year
		step1 = bm->RBCestimation.RBCspeciesArray[species].CPUEyears[Tier5_flt][Tier5_reg][iy];
		if (step1 == 1){
			step1 = bm->RBCestimation.RBCspeciesArray[species].CPUEgen[Tier5_flt][Tier5_reg][iy];
			bm->RBCestimation.RBCspeciesArray[species].expectq[iy] = log(step1 / bm->RBCestimation.RBCspeciesArray[species].expectB[iy]);
			sumq = sumq + bm->RBCestimation.RBCspeciesArray[species].expectq[iy];
			nq++;
		}
	}

	// now calculate average of all the ln(cpue/B) and exponentiate to get q est
	q = exp(sumq / nq);

	// then can loop over years again and calc expected cpue , compare to obs cpue, calc ll
	for (iy= (int) bm->RBCestimation.RBCspeciesParam[species][HistYrMin_id]; iy < (int)bm->RBCestimation.RBCspeciesArray[species].CurrentYear; iy++){
		step1 = bm->RBCestimation.RBCspeciesArray[species].CPUEyears[Tier5_flt][Tier5_reg][iy];
		if (step1==1){
			expect_cpue = q * bm->RBCestimation.RBCspeciesArray[species].expectB[iy];
			step2 = bm->RBCestimation.RBCspeciesArray[species].CPUEgen[Tier5_flt][Tier5_reg][iy];
			diff2 = (log(step2) - log(expect_cpue)) * (log(step2) - log(expect_cpue));
			sigma = sigma + diff2;
		}
	}

	sigma = sqrt(sigma / nq);

	ll = log(sigma);     // ignore constants and multipliers in ll

	// project biomass to start of year+2 (because know catch in year+1 (year of assessment)
	for (iy=bm->RBCestimation.RBCspeciesArray[species].CurrentYear; iy < index2; iy++){
		bt = bm->RBCestimation.RBCspeciesArray[species].expectB[iy-1];
		bm->RBCestimation.RBCspeciesArray[species].expectB[iy] = bt + bt * in_r / bm->RBCestimation.RBCspeciesParam[species][Tier5_p_id] * (1.0 - exp(bm->RBCestimation.RBCspeciesParam[species][Tier5_p_id] * log(bt / in_K)))
				- bm->RBCestimation.RBCspeciesArray[species].T5_removal[iy-1];
		if (bm->RBCestimation.RBCspeciesArray[species].expectB[iy] < 75.0) bm->RBCestimation.RBCspeciesArray[species].expectB[iy] = 75.0;
	}

	// BcurrSP is biomass at start of CurrentYear+2  (CurrentYear is last year of data, but do have catch for CurrentYear+1)
	bm->RBCestimation.RBCspeciesParam[species][BcurrSP_id] = bm->RBCestimation.RBCspeciesArray[species].expectB[index1];    // save in global for use when solution found

	return ll;      // minimise negative ll
}

//******************************************************************************
//
// Name:  get_T5_F20_40_48
//      get_T5_F20_40_48:  calculate equilibrium F20, F40 and F48 for surplus production model
// Description: calculate equilibrium F20, F40 and F48 for surplus production model
//
//
// called by : TierFive_SP
// calls :
// created  : April 2010 Sally
//
//******************************************************************************
void get_T5_F20_40_48(MSEBoxModel *bm, int species, double bstart, double k, double r, FILE *llogfp)
{
	double fval, bt, longterm_dep, step1;
	int i, iy, nfint;

	Util_Init_1D_Double(bm->RBCestimation.RBCspeciesArray[species].TSbiomass, 1000, 0.0);

	bm->RBCestimation.RBCspeciesParam[species][F20_id] = 0.0;
	bm->RBCestimation.RBCspeciesArray[species].TSbiomass[0] = bstart;

	nfint = (int)(1000 * bm->RBCestimation.RBCspeciesParam[species][Tier3_maxF_id]);   // max no of F's over which to calculate YPR
	//bm->RBCestimation.RBCspeciesParam[species][Nfint_id] = nfint;  // As set globally at the start
	i = 0;

	// loop over Fs until find F40 or get to max F
	do{
		i++;
		fval = i / 1000.0;
		iy = 0;
		// project the population with this F until it stabilises
		do {
			iy++;
			bt = bm->RBCestimation.RBCspeciesArray[species].TSbiomass[iy-1];
			step1 = (1.0 - exp(bm->RBCestimation.RBCspeciesParam[species][Tier5_p_id] * log(bt/k)));
			bm->RBCestimation.RBCspeciesArray[species].TSbiomass[iy] = bt + bt * r / bm->RBCestimation.RBCspeciesParam[species][Tier5_p_id] * step1 - bt * (1.0 - exp(-1.0 * fval));
		}  while ((fabs(bm->RBCestimation.RBCspeciesArray[species].TSbiomass[iy] - bm->RBCestimation.RBCspeciesArray[species].TSbiomass[iy-1]) > 1.0) && (iy < 1000.0));

		longterm_dep = bm->RBCestimation.RBCspeciesArray[species].TSbiomass[iy] / k;
		if (longterm_dep >= 0.48) bm->RBCestimation.RBCspeciesParam[species][F48_id] = fval;
		if (longterm_dep >= 0.40) bm->RBCestimation.RBCspeciesParam[species][F40_id] = fval;
		//if (longterm_dep >= 0.20) F20 = fval;

	}  while ((longterm_dep >= 0.40)  && (i < nfint));

}

//******************************************************************************
//
// Name:  TierFive_AvLen
//      TierFive_AvLen:   do a tier 5 assessment - average length - and get RBC for next year
// Description: do a tier 5 assessment - av length
//              and get RBC for next year
//
//
// called by : DoAssessment
// calls :
// created  : June 2010 Sally
//
//******************************************************************************
void TierFive_AvLen(MSEBoxModel *bm, int species, int year, FILE *llogfp)
{
	int i, it, l, fleet;
	double avlen, nolen, totlen, len = 0;
	// double avlen1;
    //int do_debug = 1;
    int nfint = (int)(bm->RBCestimation.RBCspeciesParam[species][Nfint_id]);

	it = 1;     // retained
	avlen = 0.0;
	nolen = totlen = 0;

	/* Replaced Sally snippet EA here as wanted to generalise */
	fleet = (int)(bm->RBCestimation.RBCspeciesParam[species][Tier5_flt_id]);
    
    /*
    if (do_debug) {
        fprintf(llogfp, "Time: %e %s fleet %d year %d, LengthFltYr: %d\n",
                bm->dayt, FunctGroupArray[species].groupCode, fleet, year, bm->RBCestimation.RBCspeciesArray[species].LengthFltYr[it][fleet][year]);
    }
    */

// calculate female avlen (above given length) of chosen fleet if have data
	if (bm->RBCestimation.RBCspeciesArray[species].LengthFltYr[it][fleet][year] > 0){
        if (bm->dayt > 10584)
            printf("Working out avlen etc\n");

        
		for (l=0; l < bm->RBCestimation.RBCspeciesParam[species][Nlen_id]; l++){
			len = bm->RBCestimation.RBCspeciesArray[species].LoLenBin[l] + (bm->RBCestimation.RBCspeciesParam[species][Lbin_id] / 2.0);
			totlen += bm->RBCestimation.RBCspeciesArray[species].LenComp[fleet][FEMALE][year][it][l];
            
            /*
            if (do_debug) {
                fprintf(llogfp, "Time: %e %s it: %d year: %d, l: %d len: %e totlen: %e lencomp: %d, LoLenBin: %e, lbin: %e nlen: %e Tier5len: %e\n",
                        bm->dayt, FunctGroupArray[species].groupCode, it, year, l, len, totlen,
                        bm->RBCestimation.RBCspeciesArray[species].LenComp[fleet][FEMALE][year][it][l],
                        bm->RBCestimation.RBCspeciesArray[species].LoLenBin[l],
                        bm->RBCestimation.RBCspeciesParam[species][Lbin_id],
                        bm->RBCestimation.RBCspeciesParam[species][Nlen_id],
                        bm->RBCestimation.RBCspeciesParam[species][Tier5_length_id]);
            }
            */
            
			if (len >= bm->RBCestimation.RBCspeciesParam[species][Tier5_length_id]){
				avlen += bm->RBCestimation.RBCspeciesArray[species].LenComp[fleet][FEMALE][year][it][l] * len;
				nolen += bm->RBCestimation.RBCspeciesArray[species].LenComp[fleet][FEMALE][year][it][l];
                
                /*
                if (do_debug) {
                    fprintf(llogfp, "Time: %e %s has avlen: %e nolen: %e len: %e lencomp: %d\n",
                            bm->dayt, FunctGroupArray[species].groupCode, avlen, nolen, len,
                            bm->RBCestimation.RBCspeciesArray[species].LenComp[fleet][FEMALE][year][it][l]);
                }
                */
			}
		}
		if (nolen > 0.00001)
			avlen = avlen / nolen;
		else
			avlen = 0.0;
        
        /*
        if (do_debug) {
            fprintf(llogfp, "Time: %e %s year: %d avlen: %e nolen: %e len: %e\n", bm->dayt, FunctGroupArray[species].groupCode, year, avlen, nolen, len);
        }
        */
	} else {
		//quit("No length data for the chosen fleet\n");
        bm->RBCestimation.RBCspeciesParam[species][AssessFail_id] = 1;  // As no length data for the chosen fleet
	}

	/* Removed Sally snippet EB here as wanted to generalise */

// find the F that corresponds to this avlen
    if (bm->dayt > 10584)
        printf("Working out T5_avlen with Nfint: %d\n", nfint);

    
    i = 1;
	do { i = i+1; }
	while ((avlen < bm->RBCestimation.RBCspeciesArray[species].T5_Avlen_atF[i])
			&& (i < nfint));

	bm->RBCestimation.RBCspeciesParam[species][Avlen_id] = avlen;
	bm->RBCestimation.RBCspeciesParam[species][Fcurr_id] = bm->RBCestimation.RBCspeciesArray[species].T5_Fval[i-1];
	bm->RBCestimation.RBCspeciesParam[species][PropAvlen_id] = nolen / (totlen + small_num);    // proportion of catch > reference length
    
    /*
    if (do_debug) {
        fprintf(llogfp, "Time: %e %s year: %d avlen: %e Fcurr: %e nolen: %e totlen: %e PropAvlen: %e i: %d T5_Avlen_atF: %e Nfint: %e\n",
                bm->dayt, FunctGroupArray[species].groupCode, year,
                bm->RBCestimation.RBCspeciesParam[species][Avlen_id], bm->RBCestimation.RBCspeciesParam[species][Fcurr_id],
                nolen, totlen, bm->RBCestimation.RBCspeciesParam[species][PropAvlen_id], i,
                bm->RBCestimation.RBCspeciesArray[species].T5_Avlen_atF[i], bm->RBCestimation.RBCspeciesParam[species][Nfint_id]);
    }
    */
}


//******************************************************************************
//
// Name:  TierFiveDCAC
//      TierFiveDCAC:   do a tier 5 assessment using Alec McCall's
//                        depletion corrected average catch
//
// called by : DoAssessment
// calls :
// created  : Oct 2014 Beth Fulton
//
//******************************************************************************
void TierFiveDCAC(MSEBoxModel *bm, int species, int year, double current_catch, FILE *llogfp) {
    int startyear = 0;
    int Nregions = (int)(bm->RBCestimation.RBCspeciesParam[species][NumRegions_id]);
    int Nstocks = FunctGroupArray[species].numStocks;
    int Nfleets = (int)(bm->RBCestimation.RBCspeciesParam[species][NumFisheries_id]);
    double sb0 = bm->RBCestimation.RBCspeciesArray[species].SBioZero[Nstocks][Nregions];
    double num_years = (double)(year + 1);
    double depletion, WYopt, step1, nat_mort_est, t_agemat, sumcatch;
    int iy, nreg, nf;
    
    // Get simple estimate of natural mortality
    t_agemat = FunctGroupArray[species].speciesParams[age_mat_id] * FunctGroupArray[species].ageClassSize;
    if(!t_agemat)
        t_agemat = small_num;
    nat_mort_est = bm->RBCestimation.tier5JensenConst / t_agemat;
    
    depletion = (bm->RBCestimation.RBCspeciesArray[species].SpawnBio[Nstocks][Nregions][startyear] - bm->RBCestimation.RBCspeciesArray[species].SpawnBio[Nstocks][Nregions][year]) / sb0;
    WYopt = depletion / (bm->RBCestimation.tier5DCACscalar * bm->RBCestimation.tier5DCACconst * nat_mort_est);
    if ((num_years + WYopt) < 0.0)
        step1 = 1.0;
    else
        step1 = num_years + WYopt;
    
    sumcatch = 0.0;
    for (iy = 0; iy < year+1; iy++) {
		sumcatch = 0.0;
		for (nreg = 0; nreg < Nregions ; nreg++){
			for (nf = 0; nf< Nfleets; nf++){
				sumcatch += bm->RBCestimation.RBCspeciesArray[species].CatchData[nf][nreg][iy];  // Use this for "pseudo-data" in MSE and summing oonly over assessment NumFisheries
                if (bm->flagTACincludeDiscard) {
                    sumcatch += bm->RBCestimation.RBCspeciesArray[species].DiscData[nf][nreg][iy];
                }
			}
		}
    }
    
    bm->RBCestimation.RBCspeciesParam[species][RBCest_id] = sumcatch / step1;
    
    return;
}

//******************************************************************************
//
// Name:  TierFiveSAFE
//      TierFiveSAFE:   do a tier 5 assessment using SAFE assessment methods
//      Details from Zhou et al (2011) - Fisheries Research 112: 168– 178
//
// called by : DoAssessment
// calls :
// created  : Oct 2014 Beth Fulton
//
//******************************************************************************
void TierFiveSAFE(MSEBoxModel *bm, int species, int year, double current_catch, FILE *llogfp) {
    double totfishedarea = 0.0;
    double Fest = 0.0;
    int do_debug = 0;
    int b, biomhere, cohort, den, ij, nf, stage;
    double fishedarea, maxsel, maxq, maxhab, habitat_scale, prop_dis_dead, t_agemat, nat_mort_est, frbc, step1;
    double areasp = 0.0;
    int Nregions = (int)(bm->RBCestimation.RBCspeciesParam[species][NumRegions_id]);
    
    for(b=0; b<bm->nbox; b++) {
        biomhere = 0;
        fishedarea = 0.0;
        for (cohort = 0; cohort < FunctGroupArray[species].numCohortsXnumGenes; cohort++) {
            den = FunctGroupArray[species].NumsTracers[cohort];
            for(ij=0; ij<bm->boxes[b].nz; ij++) {
                if ( bm->boxes[b].tr[ij][den] > bm->min_dens)
                    biomhere = 1;
            }
        }
        
        maxsel = bm->RBCestimation.RBCspeciesParam[species][Tier5sel_id];
        maxq = bm->RBCestimation.RBCspeciesParam[species][Tier5q_id];

        for (nf = 0; nf < bm->K_num_fisheries; nf++) {
            prop_dis_dead = bm->SP_FISHERYprms[species][nf][incidmort_id];
            maxhab = 0.0;
            for (stage = 0; stage < FunctGroupArray[species].numStages; stage++) {
                habitat_scale = Harvest_Get_Habitat_Availability(bm, species, stage, nf, maxq, do_debug, llogfp);
                if (habitat_scale > maxhab)
                    maxhab = habitat_scale;
            }
            fishedarea += prop_dis_dead * maxhab * maxq * maxsel * bm->CumEffort[nf][b] * bm->FISHERYprms[nf][swept_area_id];
        }
        
        if (fishedarea > bm->boxes[b].area)
            fishedarea = bm->boxes[b].area;
        totfishedarea += fishedarea;

        if(biomhere)
            areasp += bm->boxes[b].area;
    }

    Fest = totfishedarea / (areasp + small_num);
    bm->RBCestimation.RBCspeciesParam[species][Fcurr_id] = Fest;

    // Modified from tier3 - modifications based on Zhou et al (2011) - especially option 6 for reference point calculation
    t_agemat = FunctGroupArray[species].speciesParams[age_mat_id] * FunctGroupArray[species].ageClassSize;
    if(!t_agemat)
        t_agemat = small_num;
    nat_mort_est = bm->RBCestimation.tier5JensenConst / t_agemat;
    bm->RBCestimation.RBCspeciesParam[species][F20_id] = bm->RBCestimation.tier5limscalar * bm->RBCestimation.tier5omega * nat_mort_est;
    bm->RBCestimation.RBCspeciesParam[species][F40_id] = bm->RBCestimation.tier5omega * nat_mort_est;
    bm->RBCestimation.RBCspeciesParam[species][F48_id] = 0.8 * bm->RBCestimation.RBCspeciesParam[species][F40_id];
    
    if (bm->RBCestimation.RBCspeciesParam[species][Fcurr_id] >= bm->RBCestimation.RBCspeciesParam[species][F20_id]){
		frbc = 0.0;
        bm->RBCestimation.RBCspeciesArray[species].TriggerReached[Nregions] = 2;   // As less than target
	} else if (bm->RBCestimation.RBCspeciesParam[species][Fcurr_id] <= bm->RBCestimation.RBCspeciesParam[species][F40_id])
		frbc = bm->RBCestimation.RBCspeciesParam[species][F48_id];
	else {
		step1 = (bm->RBCestimation.RBCspeciesParam[species][F40_id] - bm->RBCestimation.RBCspeciesParam[species][F20_id])
        * (bm->RBCestimation.RBCspeciesParam[species][Fcurr_id] - bm->RBCestimation.RBCspeciesParam[species][F20_id]);
		frbc = bm->RBCestimation.RBCspeciesParam[species][F48_id] / step1;
        bm->RBCestimation.RBCspeciesArray[species].TriggerReached[Nregions] = 2;  // As less than target
	}
    
	bm->RBCestimation.RBCspeciesParam[species][Tier3_mult_id] = (1.0-exp(-frbc)) / (1.0-exp(-bm->RBCestimation.RBCspeciesParam[species][Fcurr_id]));
	if (bm->RBCestimation.RBCspeciesParam[species][Tier3_mult_id] > 3.0)  // Adopted from tier3
		bm->RBCestimation.RBCspeciesParam[species][Tier3_mult_id] = 3.0;
	bm->RBCestimation.RBCspeciesParam[species][RBCest_id] = bm->RBCestimation.RBCspeciesParam[species][Tier3_mult_id] * current_catch;
    
    
    return;
}


//******************************************************************************
//
// Name:  TierSixRUSS
//      TierSizRUSS:   catch trigger - see
//
// called by : DoAssessment
// calls :
// created  : Oct 2014 Beth Fulton
//
//******************************************************************************
void TierSixRUSS(MSEBoxModel *bm, int species, int year, FILE *llogfp) {
    
	int Nfleets = (int)(bm->RBCestimation.RBCspeciesParam[species][NumFisheries_id]);
    int Nregions = (int)(bm->RBCestimation.RBCspeciesParam[species][NumRegions_id]);
	int nf, nreg, nt, ij, nd, iy;
    int TriggerScen = (int)(bm->RBCestimation.RBCspeciesParam[species][TriggerResponseScen_id]);
    int MaxTrigger = 0;
    int FisheryClosed = (int)(bm->RBCestimation.RBCspeciesParam[species][Closed_id]);
    int Ntriggers = bm->RBCestimation.NumTriggers;
    double regsumcatch, totsumcatch, adjTAC, prev_RBC, localcatch;
    double HistCatch = 0.0;

	localcatch = 0.0;
    
    // Get historical level of catch
    for (iy = 0; iy < bm->RBCestimation.RBCspeciesParam[species][HistYrMax_id]; iy++){
        for (nf = 0; nf< Nfleets; nf++){
            localcatch += bm->RBCestimation.RBCspeciesArray[species].CatchData[nf][Nregions][iy];
        }
        if (localcatch > HistCatch)
            HistCatch = localcatch;
    }

    // First, see if any triggers have been reached for any level in the previous year:
    for (nreg = 0; nreg < Nregions; nreg++) {
        regsumcatch = 0.0;
        for (nf = 0; nf< Nfleets; nf++){
            regsumcatch += bm->RBCestimation.RBCspeciesArray[species].CatchData[nf][nreg][year];
        }
        bm->RBCestimation.RBCspeciesParam[species][TriggerReached_id] = 0;
        for (nt=0; nt < Ntriggers; nt++) {
            if (regsumcatch > (bm->RBCestimation.RBCspeciesArray[species].TriggerPoints[nt][nreg] * HistCatch))
                 bm->RBCestimation.RBCspeciesArray[species].TriggerReached[nreg] = nt;
        }
    }
    totsumcatch = 0.0;
    for (nf = 0; nf< Nfleets; nf++){
        totsumcatch += bm->RBCestimation.RBCspeciesArray[species].CatchData[nf][Nregions][year];
    }
    for (nt=0; nt < Ntriggers; nt++) {
        if (totsumcatch > (bm->RBCestimation.RBCspeciesArray[species].TriggerPoints[nt][Nregions] * HistCatch))
             bm->RBCestimation.RBCspeciesArray[species].TriggerReached[Nregions] = nt;
    }

    // Find maximal response per zone or across the board (depending on trigger rule)
    MaxTrigger = 0;
    switch (TriggerScen) {
        case NoTrigger:  // No adjustment to previous year's quota or spatial management
        case TotTrigger:   // Trigger system independent of zone
            MaxTrigger = bm->RBCestimation.RBCspeciesArray[species].TriggerReached[Nregions];
            break;
        case ZoneTrigger:  // Trigger by zone
        case LaxZoneTrigger:  // Trigger by zone but no response to lowest level trigger being tripped
        case TightZoneTrigger: // Trigger y zone and if Level 2 or 3 trigger reached in a zone then zone is closed
            for (nreg = 0; nreg < Nregions; nreg++) {
                if (bm->RBCestimation.RBCspeciesArray[species].TriggerReached[nreg] > MaxTrigger)
                    MaxTrigger = bm->RBCestimation.RBCspeciesArray[species].TriggerReached[nreg];
            }
            break;
        default:
            quit("No such Trigger Response Scenario defined for Tier 6 please reset TriggerResponseScen for %s (currently %d)\n", FunctGroupArray[species].groupCode, TriggerScen);
            break;
    }
    bm->RBCestimation.RBCspeciesParam[species][TriggerReached_id] = MaxTrigger;
	
    /* Now respond according to the highest trigger level reached in any zone and the trigger rules in place */
    adjTAC = 1.0;
    prev_RBC = bm->RBCestimation.RBCspeciesParam[species][TAC_old_id];
    switch (MaxTrigger) {
        case 0:
            if (FisheryClosed) {      // Do assessment if no trigger was because there was no fishing in previous year(s)
                
                if(bm->RBCestimation.tier6_to_tier4) {
                    adjTAC = DoCPUE(bm, year, species, llogfp); // To get average CPUE
                    adjTAC = DoPrager(bm, year, species, llogfp);

                    // If we adjust from last existing level, we will likely end up at trigger level 3 again almost immediately so to be safe apply an addition reduction factor (RBCadjscalar = 0.3 in Nat's code)
                    adjTAC *= (1.0 - bm->RBCestimation.RBCspeciesParam[species][RBCadjscalar_id]);
                    prev_RBC = bm->RBCestimation.RBCspeciesParam[species][lastRBC_id];
                
                } else {
                    // Do length based assessment
                    Call_Tier3(bm, year, species, llogfp);
                    
                    // map back to local variables so don't need to make code below conditional on method used
                    adjTAC = 1.0;
                    prev_RBC = bm->RBCestimation.RBCspeciesParam[species][RBCest_id];
                }
            }
            bm->RBCestimation.RBCspeciesParam[species][Closed_id] = 0;
            break;
        case 1:  // Level 1 trigger reached in at least one zone so react accordingly (unless not responding to level 1 triggers) - either with a CPUE-based empirical Tier 4 rule or a length-based Tier 3
            if (TriggerScen != LaxZoneTrigger) {
                if(bm->RBCestimation.tier6_to_tier4)
                    // Do CPUE trend analysis
                    adjTAC = DoCPUE(bm, year, species, llogfp);
                else {
                    // Do SAFE assessment
                    TierFiveSAFE(bm, year, species, totsumcatch, llogfp);
                    
                    // map back to local variables so don't need to make code below conditional on method used
                    adjTAC = 1.0;
                    prev_RBC = bm->RBCestimation.RBCspeciesParam[species][RBCest_id];
                }
            }
            bm->RBCestimation.RBCspeciesParam[species][Closed_id] = 0;
            break;
        case 2: // Level 2 trigger reached in at least one zone so do a stock assesssment
            if(bm->RBCestimation.tier6_to_tier4) {
                // do production assessment
                adjTAC = DoCPUE(bm, year, species, llogfp); // To get average CPUE
                adjTAC = DoPrager(bm, year, species, llogfp);  // Do Production estimate
            } else {
                // Do length based assessment
                Call_Tier3(bm, year, species, llogfp);
                
                // map back to local variables so don't need to make code below conditional on method used
                adjTAC = 1.0;
                prev_RBC = bm->RBCestimation.RBCspeciesParam[species][RBCest_id];
            }
            bm->RBCestimation.RBCspeciesParam[species][Closed_id] = 0;
            break;
        case 3: // Level 3 trigger reached in at least one zone
            // i.e. no fishing this year - store current TAC for later use and close fishery by zeroing TAC
            adjTAC = 0.0;
            bm->RBCestimation.RBCspeciesParam[species][lastRBC_id] = bm->RBCestimation.RBCspeciesParam[species][TAC_old_id];
            bm->RBCestimation.RBCspeciesParam[species][Closed_id] = 1;
    }
    bm->RBCestimation.RBCspeciesParam[species][RBCest_id] = prev_RBC * adjTAC;
    
    
    /* Now do spatial management */
    switch (TriggerScen) {
        case NoTrigger:  // No adjustment to previous year's quota or spatial management
        case ZoneTrigger:  // Trigger by zone
        case TotTrigger:   // Trigger system independent of zone
        case LaxZoneTrigger:  // Trigger by zone but no response to lowest level trigger being tripped
            // No extra spatial management to do as done via TAC or finer zone closures ignored
            break;
        case TightZoneTrigger: // If Level 2 or 3 trigger reached in a zone then zone is closed
            if (MaxTrigger > 1) {
                for (ij=0; ij < bm->nbox; ij++) {
                    for (nreg = 0; nreg < Nregions; nreg++) {
                        if ((Nregions == 1) || (bm->RBCestimation.RBCspeciesArray[species].regID[ij] == nreg)) {
                            for (nf = 0; nf < bm->K_num_fisheries; nf++) {
                                // If target the species then close the fishery
                                if(bm->FISHERYtarget[nf][species] || (bm->SP_FISHERYprms[species][nf][q_id] > bm->RBCestimation.RBCspeciesParam[species][trigq_id])) {
                                    for (nd = 0; nd < 365; nd++) {
                                        bm->MPAlist[year+1][nd][ij][nf] = 0;
                                    }
                                }
                            }
                        }
                    }
                }
            }
            break;
        default:
            quit("No such Trigger Response Scenario defined for Tier 6 please reset TriggerResponseScen for %s (currently %d)\n", FunctGroupArray[species].groupCode, TriggerScen);
            break;
    }

    return;
}

//******************************************************************************
//
// Name: DoCPUE
//
// called by: Tiers 6
// calls:
// created: Oct 2014 Beth Fulton
//
//******************************************************************************
double DoCPUE(MSEBoxModel *bm, int species, int year, FILE *llogfp) {
    double Adjust;
	int iy, nyrs, f;
    int is_done = 0;
	int j = 0;
	int Rnum = 0;
	double Alpha, Beta, SumX, SumXX, SumXY, SumY, totEff, totC, AvgCPUE;
    int Nfleets = (int)(bm->RBCestimation.RBCspeciesParam[species][NumFisheries_id]);
	int Nregions = (int)(bm->RBCestimation.RBCspeciesParam[species][NumRegions_id]);
    
	// Calculate CPUE for the last x years, beginning with latest non-zero effort year
	// Nov 2026: loop now counts back (was iy++ so ran forward off the end of the arrays) and j is the year found
	for(iy = year-1; iy >= 0; iy--) { // establish most recent non-zero effort year
        for (f = 0; f < Nfleets; f++) {
            if (bm->RBCestimation.RBCspeciesArray[species].EffortData[f][Nregions][iy] > 0.0) {
                is_done = 1;
                break;
            }
        }
        if (is_done) {
            j = iy;
            break;
        }
	}
    
    nyrs = 4;
    if (j < nyrs)
        nyrs = j;
    
    for (iy = nyrs; iy >= 0; iy--) {
        totEff = 0.0;
        totC = 0.0;
        for (f = 0; f < Nfleets; f++) {
            totEff += bm->RBCestimation.RBCspeciesArray[species].EffortData[f][Nregions][j-iy];
            totC += bm->RBCestimation.RBCspeciesArray[species].CatchData[f][Nregions][j-iy];
        }
        bm->RBCestimation.RBCspeciesArray[species].AnnCPUE[5-iy] = totC / totEff;
	}
    
	// Fit linear regression to the last five CPUE values
	SumX = 0.0;
	SumXX = 0.0;
	SumXY = 0.0;
	SumY = 0;
	for (iy = 0; iy < 5; iy++) {
		if (bm->RBCestimation.RBCspeciesArray[species].AnnCPUE[iy] > 0) {
			Rnum ++;
			SumX += iy;
			SumXX += iy * iy;
			SumY += bm->RBCestimation.RBCspeciesArray[species].AnnCPUE[iy];
			SumXY += bm->RBCestimation.RBCspeciesArray[species].AnnCPUE[iy] * iy;
		}
	}
    
	AvgCPUE = SumY / Rnum;
    
	Beta = (SumXY - SumX * SumY / Rnum) / (SumXX - SumX * SumX / Rnum);
	Alpha = SumY / Rnum - Beta * SumX / Rnum;
    
	Adjust = 1.0 +  bm->RBCestimation.delTAE * Beta;
    
	fprintf(llogfp, "Time: %e for %s: AvgCPUE = %9.4f  Alpha = %9.4f  Beta = %9.4f AdjTAE = %9.4f\n",
           bm->dayt, FunctGroupArray[species].groupCode, AvgCPUE, Alpha, Beta, Adjust);
	
	return Adjust;

}

//******************************************************************************
//
// Name: DoPrager
//
// called by: Tiers 6-7 (RUSS style)
// calls:
// created: Oct 2014 Beth Fulton
//
//******************************************************************************
double DoPrager(MSEBoxModel *bm, int species, int year, FILE *llogfp) {
    double ans = 0.0;

    quit("Ask Beth to code this in as not yet complete - ask her to look at Nat Dowlings RUSS code\n");
    
    return ans;
}


//******************************************************************************
//
// Name:  TierSevenRUSS
//      SevenRUSS:   group based assessment
//          Primary member of group called then do assessment (i.e. set only one
//          species as tiered 7 assessed and then it will assess that species and
//          its companion species (as defined by co_sp array)
//
// called by : DoAssessment
// calls :
// created  : Oct 2014 Beth Fulton
//
//******************************************************************************
void TierSevenRUSS(MSEBoxModel *bm, int species, int year, FILE *llogfp) {
    double this_catch, last_catch, totcatch, localcatch, toteffort = 0.0, HistCatch, CoHistCatch, diff;
    int Nregions = (int)(bm->RBCestimation.RBCspeciesParam[species][NumRegions_id]);
    int Ntriggers = bm->RBCestimation.NumTriggers;
    int iy, nt, sp, cosp, nf, b, biomhere, ij, cohort, den;
    int startyear = year - (int)bm->RBCestimation.tier7period;
    double areafished = 0.0;
    double areasp = 0.0;
    double relarea = 0.0;
    double maxrelcatch = 0.0;
    double tot_co_catch = 0.0;
    double hist_co_catch = 0.0;

	if (startyear < 0) startyear = 0;

    /* Check peak catch */
    for (sp = 0; sp < bm->K_max_co_sp; sp++) {
        
        cosp = FunctGroupArray[sp].co_sp[sp];
        if ((cosp < 0) || (cosp > bm->K_max_impacted_sp))
            continue;
        
        // Get historical level of catch
        HistCatch = 0.0;
        for (iy = 0; iy < bm->RBCestimation.RBCspeciesParam[species][HistYrMax_id]; iy++){
            localcatch = 0.0;
            for (nf = 0; nf< bm->K_num_fisheries; nf++){
                localcatch += Harvest_Get_TotCumCatch(cosp, nf, iy);
            }
            if (localcatch > HistCatch)
                HistCatch = localcatch;
        }
        
        this_catch = 0.0;
        for (nf = 0; nf < bm->K_num_fisheries; nf++){
            this_catch += Harvest_Get_TotCumCatch(cosp, nf, year);
        }
        tot_co_catch += this_catch;
        bm->RBCestimation.RBCspeciesArray[species].CatchComp[cosp][year] = this_catch;
        
        // Do peak catch check
        for (nt=0; nt < Ntriggers; nt++) {
            if (this_catch > (bm->RBCestimation.RBCspeciesArray[species].TriggerPoints[nt][Nregions] * HistCatch))
                bm->RBCestimation.RBCspeciesArray[species].TriggerReached[Nregions] = nt;
        }
    }
    
    /* Aggregate catch over the group and then apply similar triggers to tier 6 */
    CoHistCatch = 0.0;
    for (iy = 0; iy < bm->RBCestimation.RBCspeciesParam[species][HistYrMax_id]; iy++){
        hist_co_catch = 0.0;
        for (sp = 0; sp < bm->K_max_co_sp; sp++) {
            cosp = FunctGroupArray[sp].co_sp[sp];
            if ((cosp < 0) || (cosp > bm->K_max_impacted_sp))
                continue;
            for (nf = 0; nf< bm->K_num_fisheries; nf++){
                hist_co_catch += Harvest_Get_TotCumCatch(cosp, nf, iy);
            }
        }
        if( hist_co_catch > CoHistCatch)
            CoHistCatch = hist_co_catch;
    }
    for (nt=0; nt < Ntriggers; nt++) {
        if (tot_co_catch > (bm->RBCestimation.RBCspeciesArray[species].TriggerPoints[nt][Nregions] * CoHistCatch))
            bm->RBCestimation.RBCspeciesArray[species].TriggerReached[Nregions] = nt;
    }

    
    /* Check to see if proportional composition of the catch of any species
     changes > x% (either in 1 step in 1 yr or cumulatively over 3 yrs) */
    for (sp = 0; sp < bm->K_max_co_sp; sp++) {
        cosp = FunctGroupArray[sp].co_sp[sp];
        if ((cosp < 0) || (cosp > bm->K_max_impacted_sp))
            continue;
        
        bm->RBCestimation.RBCspeciesArray[species].CatchComp[cosp][year] /= (tot_co_catch + small_num);
        // Check against previous tier7period (e.g. 3) years
        nt = 0;
        last_catch = 0.0;
        for (iy = startyear; iy < year; iy++) {
            diff = fabs(bm->RBCestimation.RBCspeciesArray[species].CatchComp[cosp][year] - bm->RBCestimation.RBCspeciesArray[species].CatchComp[cosp][iy]);
            this_catch = bm->RBCestimation.RBCspeciesArray[species].CatchComp[cosp][iy];
            if (diff > bm->RBCestimation.tier7maxcompchange) {  // Check for proportional change (has it changed by more than y% overall?)
                if (bm->RBCestimation.RBCspeciesArray[species].TriggerReached[Nregions] < 1)
                    bm->RBCestimation.RBCspeciesArray[species].TriggerReached[Nregions] = 1;
            }
            diff = last_catch - this_catch;  // Check annual change (has it declined by > x%)
            if (diff > bm->RBCestimation.tier7maxannchange)
                nt++;
            last_catch = this_catch;
        }
        if (nt >= bm->RBCestimation.tier7period) {
            if (bm->RBCestimation.RBCspeciesArray[species].TriggerReached[Nregions] < 1)
                bm->RBCestimation.RBCspeciesArray[species].TriggerReached[Nregions] = 1;
        }
        
    /* Now check if the proportion of the area fished changes > y% */
    /* Also check if the proportion of the catch from any one location is > z% */
        relarea = 0.0;
        maxrelcatch = 0.0;
        for(b=0; b<bm->nbox; b++) {
            biomhere = 0;
            totcatch = 0.0;
            for (nf = 0; nf < bm->K_num_fisheries; nf++){
                totcatch += Harvest_Get_TotCumCatch(cosp, nf, year);
            }
            localcatch = 0.0;
            for(ij=0; ij<bm->boxes[b].nz; ij++) {
                for (nf = 0; nf < bm->K_num_fisheries; nf++) {
                    if ((bm->CumCatch[cosp][nf][b][ij] + bm->CumDiscards[cosp][nf][b]) > 0.0){
                        localcatch += bm->CumCatch[cosp][nf][b][ij];
                    }
                }
                for (cohort = 0; cohort < FunctGroupArray[cosp].numCohorts * FunctGroupArray[cosp].numGeneTypes; cohort++) {
                    den = FunctGroupArray[cosp].NumsTracers[cohort];
                    if ( bm->boxes[b].tr[ij][den] > bm->min_dens)
                        biomhere = 1;
                }
            }

            if (bm->SP_FISHERYprms[cosp][nf][q_id] > 0.0)
                toteffort += bm->CumEffort[nf][b];

            if((localcatch / (totcatch + small_num)) > maxrelcatch)
                maxrelcatch = localcatch / (totcatch + small_num);
            
            if(localcatch)
                areafished += bm->boxes[b].area;
            if(biomhere)
                areasp += bm->boxes[b].area;
            relarea = areafished / (areasp + small_num);
        }
        bm->RBCestimation.RBCspeciesArray[species].relareafished[cosp][year] = relarea;
        for (iy = 0; iy < year; iy++) {
            diff = fabs(bm->RBCestimation.RBCspeciesArray[species].relareafished[cosp][year] - bm->RBCestimation.RBCspeciesArray[species].relareafished[cosp][iy]);
            if (diff > bm->RBCestimation.tier7areachange) {
                if (bm->RBCestimation.RBCspeciesArray[species].TriggerReached[Nregions] < 1)
                    bm->RBCestimation.RBCspeciesArray[species].TriggerReached[Nregions] = 1;
            }
        }
        
        if (relarea > bm->RBCestimation.tier7relarea) {
            if (bm->RBCestimation.RBCspeciesArray[species].TriggerReached[Nregions] < 1)
                bm->RBCestimation.RBCspeciesArray[species].TriggerReached[Nregions] = 1;
        }
        
    }
        
    /* Check CPUE - i.e. whether CPUE decline > n% */
    bm->RBCestimation.RBCspeciesArray[species].cpue[year] = tot_co_catch / (toteffort + small_num);
    diff = bm->RBCestimation.RBCspeciesArray[species].cpue[year] / (bm->RBCestimation.RBCspeciesArray[species].cpue[startyear] + small_num);
    if (( diff < 1.0) && ( diff > bm->RBCestimation.tier7cpuechange)) {
        if (bm->RBCestimation.RBCspeciesArray[species].TriggerReached[Nregions] < 1)
            bm->RBCestimation.RBCspeciesArray[species].TriggerReached[Nregions] = 1;
    }
    if ( diff <= bm->RBCestimation.tier7cpuechange ) { // Trigger level 2 for all constituent species
        for (sp = 0; sp < bm->K_max_co_sp; sp++) {
            cosp = FunctGroupArray[sp].co_sp[sp];
            if ((cosp < 0) || (cosp > bm->K_max_impacted_sp))
                continue;
            
            if (bm->RBCestimation.RBCspeciesArray[cosp].TriggerReached[Nregions] < 2)
                bm->RBCestimation.RBCspeciesArray[cosp].TriggerReached[Nregions] = 2;
        }
    }
    
    
    /* Level 1 trigger - do a SAFE assessment per species */
    if (bm->RBCestimation.RBCspeciesArray[species].TriggerReached[Nregions] == 1) {
        for (sp = 0; sp < bm->K_max_co_sp; sp++) {
            cosp = FunctGroupArray[sp].co_sp[sp];
            if ((cosp < 0) || (cosp > bm->K_max_impacted_sp))
                continue;
            
            totcatch = 0.0;
            for (nf = 0; nf < bm->K_num_fisheries; nf++){
                totcatch += Harvest_Get_TotCumCatch(cosp, nf, year);
            }
            
            TierFiveSAFE(bm, cosp, year, totcatch, llogfp);
        }
    }
    
    /* Level 2 trigger - do a length based assessment per species */
    for (sp = 0; sp < bm->K_max_co_sp; sp++) {
        cosp = FunctGroupArray[sp].co_sp[sp];
        if ((cosp < 0) || (cosp > bm->K_max_impacted_sp))
            continue;
        
        if (bm->RBCestimation.RBCspeciesArray[cosp].TriggerReached[Nregions] == 2)
            Call_Tier3(bm, year, species, llogfp);
        
        bm->RBCestimation.RBCspeciesParam[cosp][TriggerReached_id] = bm->RBCestimation.RBCspeciesArray[cosp].TriggerReached[Nregions];

    }
    bm->RBCestimation.RBCspeciesParam[species][TriggerReached_id] = bm->RBCestimation.RBCspeciesArray[species].TriggerReached[Nregions];
    
    
    return;
}

//******************************************************************************
//
// Name:  Evaluation
//      Evaluation:       write results for later evaluation
// Description: write results for later evaluation
//
//
// called by : main
// calls : WriteResults
// created  : Mar 2008 Sally
//
//******************************************************************************
void Evaluation(MSEBoxModel *bm, int species, int year, FILE *llogfp, FILE *fid) {
	int  r;
	double sb0;
	int tier = (int) (FunctGroupArray[species].speciesParams[tier_id]);
	int tiertype = (int) (bm->RBCestimation.RBCspeciesParam[species][tiertype_id]);   // Nov 2026: was read from speciesParams
	double step1;
	int fleetid = (int)(bm->RBCestimation.RBCspeciesParam[species][CPUEfleet_id]);
	int Nregions = (int)(bm->RBCestimation.RBCspeciesParam[species][NumRegions_id]);
    int Nstocks = FunctGroupArray[species].numStocks;

    //printf("Doing Evaluation for %s with tier %d\n", FunctGroupArray[species].groupCode, tier);
    
    for (r=0; r < Nregions; r++) {
		WriteResults(bm, species, r, year, llogfp, fid);
    }

	if (bm->RBCestimation.RBCspeciesParam[species][Regime_year_id] > 0)
		sb0 = bm->RBCestimation.RBCspeciesArray[species].SBioZero_shift[0][0];
	else
		sb0 = bm->RBCestimation.RBCspeciesArray[species].SBioZero[0][0];

	switch (tier){
	case tier0: /* ----------- No future catches ---------- */
		break;
	case tier1: /* ----------- Tier 1 --------------------- */
		fprintf(llogfp, "Time: %e %s year: %d MaxConvergCrit: %e sb0: %e, EstB0: %e, SpawnBio: %e, EstBcurr: %e, relSpawnBio: %e, EstDepletion: %e, SaveIseedx: %d, SaveIseedz: %d\n",
			bm->dayt, FunctGroupArray[species].groupCode, year,
			bm->RBCestimation.RBCspeciesParam[species][MaxConvergCrit_id], sb0,
			bm->RBCestimation.RBCspeciesParam[species][EstB0_id],
			bm->RBCestimation.RBCspeciesArray[species].SpawnBio[Nstocks][Nregions][year],
			bm->RBCestimation.RBCspeciesParam[species][EstBcurr_id],
			bm->RBCestimation.RBCspeciesArray[species].SpawnBio[Nstocks][Nregions][year] / sb0,
			bm->RBCestimation.RBCspeciesParam[species][EstDepletion_id],
			bm->RBCestimation.SaveIseedx, bm->RBCestimation.SaveIseedz);
		break;
    case tier2: /* ------------ Tier 2 --------------------- */
        break;
	case tier3: /* ------------ Tier 3 --------------------- */
		fprintf(llogfp, "Time: %e %s year: %d F20: %e F40: %e F48: %e Fcurr: %e Tier3_mult: %e MinCyr: %e MaxCyr: %e\n",
				bm->dayt, FunctGroupArray[species].groupCode, year,
				bm->RBCestimation.RBCspeciesParam[species][F20_id], bm->RBCestimation.RBCspeciesParam[species][F40_id],
				bm->RBCestimation.RBCspeciesParam[species][F48_id], bm->RBCestimation.RBCspeciesParam[species][Fcurr_id],
				bm->RBCestimation.RBCspeciesParam[species][Tier3_mult_id],
				bm->RBCestimation.RBCspeciesParam[species][MinCyr_id], bm->RBCestimation.RBCspeciesParam[species][MaxCyr_id]);
		break;
	case tier4: /* ------------ Tier 4 --------------------- */
		fprintf(llogfp, "Time: %e %s year: %d CPUEfleet: %e CPUElim: %e CPUEtarg: %e Cmax: %e CPUEav: %e Cmult: %e CCcur: %e RBC: %e\n",
				bm->dayt, FunctGroupArray[species].groupCode, year,
				bm->RBCestimation.RBCspeciesParam[species][CPUEfleet_id],
				bm->RBCestimation.RBCspeciesParam[species][CPUElim_id], bm->RBCestimation.RBCspeciesParam[species][CPUEtarg_id],
				bm->RBCestimation.RBCspeciesParam[species][Cmax_id], bm->RBCestimation.RBCspeciesParam[species][CPUEav_id],
				bm->RBCestimation.RBCspeciesParam[species][Cmult_id], bm->RBCestimation.RBCspeciesParam[species][CCcur_id],
				bm->RBCestimation.RBCspeciesParam[species][RBCest_id]);
		step1 = bm->RBCestimation.RBCspeciesArray[species].CPUEgen[fleetid][r][year];
		for (r=0; r < Nregions; r++)
			fprintf(llogfp, "%e %s %d %d %e %e %e\n",
				bm->dayt, FunctGroupArray[species].groupCode, r, year, step1,
				bm->RBCestimation.RBCspeciesParam[species][CPUEfleet_id], bm->RBCestimation.RBCspeciesParam[species][CPUEtarg_id]);
		break;
	case tier5: /* ------------ Tier 5 --------------------- */
		if (tiertype == new_tier_rule) {  //**********************************
			fprintf(llogfp, "Time: %e %s year: %d F20: %e F40: %e F48: %e Avlen: %e PropAvlen: %e Fcurr: %e True_F: %e Tier3_mult: %e  MinCyr: %e MaxCyr: %e\n",
					bm->dayt, FunctGroupArray[species].groupCode, year,
					bm->RBCestimation.RBCspeciesParam[species][F20_id], bm->RBCestimation.RBCspeciesParam[species][F40_id],
					bm->RBCestimation.RBCspeciesParam[species][F48_id], bm->RBCestimation.RBCspeciesParam[species][Avlen_id],
					bm->RBCestimation.RBCspeciesParam[species][PropAvlen_id], bm->RBCestimation.RBCspeciesParam[species][Fcurr_id],
					bm->RBCestimation.RBCspeciesParam[species][True_F_id], bm->RBCestimation.RBCspeciesParam[species][Tier3_mult_id],
					bm->RBCestimation.RBCspeciesParam[species][MinCyr_id], bm->RBCestimation.RBCspeciesParam[species][MaxCyr_id]);
			if (year == bm->RBCestimation.RBCspeciesParam[species][MaxYr_id]){
				fprintf(llogfp,"Time: %e %s\n", bm->dayt, FunctGroupArray[species].groupCode);
			}
		}
		break;
    case tier6:
    case tier7:
        fprintf(llogfp, "Time: %e %s year: %d trigeached: %e\n",
					bm->dayt, FunctGroupArray[species].groupCode, year, bm->RBCestimation.RBCspeciesParam[species][TriggerReached_id]);

        break;
    case dyntier4: /* ------------ Dynamic Tier 4 --------------------- */
        fprintf(llogfp, "Time: %e %s year: %d sb0: %e SpawnBio: %e relSpawnBio: %e EstB0: %e EstBcurr: %e EstDepletion: %e r: %e K: %e z: %e MSY: %e BMSY: %e nll: %e maxgrad: %e RBC: %e\n",
                bm->dayt, FunctGroupArray[species].groupCode, year, sb0,
                bm->RBCestimation.RBCspeciesArray[species].SpawnBio[Nstocks][Nregions][year],
                bm->RBCestimation.RBCspeciesArray[species].SpawnBio[Nstocks][Nregions][year] / sb0,
                bm->RBCestimation.RBCspeciesParam[species][EstB0_id], bm->RBCestimation.RBCspeciesParam[species][EstBcurr_id],
                bm->RBCestimation.RBCspeciesParam[species][EstDepletion_id], bm->RBCestimation.RBCspeciesParam[species][DT4_r_id],
                bm->RBCestimation.RBCspeciesParam[species][DT4_K_id], bm->RBCestimation.RBCspeciesParam[species][DT4_z_id],
                bm->RBCestimation.RBCspeciesParam[species][DT4_MSY_id], bm->RBCestimation.RBCspeciesParam[species][DT4_BMSY_id],
                bm->RBCestimation.RBCspeciesParam[species][DT4_nll_id], bm->RBCestimation.RBCspeciesParam[species][DT4_maxgrad_id],
                bm->RBCestimation.RBCspeciesParam[species][RBCest_id]);
        break;
    case tier14: /* ------------ Perfect information ACLIM --------------------- */
        fprintf(llogfp, "Time: %e %s year: %d sb0: %e SpawnBio: %e relSpawnBio: %e HCRtype: %e rebuilding: %e F40: %e F48: %e RBC: %e\n",
                bm->dayt, FunctGroupArray[species].groupCode, year, sb0,
                bm->RBCestimation.RBCspeciesArray[species].SpawnBio[Nstocks][Nregions][year],
                bm->RBCestimation.RBCspeciesArray[species].SpawnBio[Nstocks][Nregions][year] / sb0,
                bm->RBCestimation.RBCspeciesParam[species][HCRtype_id], bm->RBCestimation.RBCspeciesParam[species][HCR_rebuilding_id],
                bm->RBCestimation.RBCspeciesParam[species][F40_id], bm->RBCestimation.RBCspeciesParam[species][F48_id],
                bm->RBCestimation.RBCspeciesParam[species][RBCest_id]);
        break;
	default:
		printf("No such tier (%d)\n", tier);
		fprintf(llogfp,"No such tier (%d)\n", tier);
		quit("");
		break;
	}

	//printf("Ending Evaluation\n");
}

//******************************************************************************
//
// Name:  WriteResults
// WriteResults:     write results by region for later evaluation
// Description: write results by region for later evaluation
//
//
// called by : Evaluation
// calls :
// created  : Nov 2010 Sally
//
// note fid is the tier_RBC_eval reporting file TODO: Create this file
//
//******************************************************************************

void WriteResults(MSEBoxModel *bm, int species, int region, int year, FILE *llogfp, FILE *fid)
{
	int nf;
	//int r;
	double sb0;
	int Nfleets = (int)(bm->RBCestimation.RBCspeciesParam[species][NumFisheries_id]);
    int Nstock = FunctGroupArray[species].numStocks;

	if ((bm->RBCestimation.RBCspeciesParam[species][Regime_year_id] > 0)
        && (year >= bm->RBCestimation.RBCspeciesParam[species][Regime_year_id]))
		sb0 = bm->RBCestimation.RBCspeciesArray[species].SBioZero_shift[Nstock][region];
	else
		sb0 = bm->RBCestimation.RBCspeciesArray[species].SBioZero[Nstock][region];
    

    // where region subscript = 0, the value is summed over regions
	// Write the output to store the result
	fprintf(fid,"%e %s %d %d %e %e %e %e %e %e %e %e",
		bm->dayt, FunctGroupArray[species].groupCode, year, region,
		sb0, bm->RBCestimation.RBCspeciesArray[species].SpawnBio[Nstock][region][year],
		bm->RBCestimation.RBCspeciesArray[species].SpawnBio[Nstock][region][year] / sb0,
		bm->RBCestimation.RBCspeciesParam[species][AssessFail_id],
		bm->RBCestimation.RBCspeciesParam[species][RBCest_id] / 1000.0,
		bm->RBCestimation.RBCspeciesParam[species][TACdisc_id] / 1000.0,
		bm->RBCestimation.RBCspeciesParam[species][TACpost_id] / 1000.0,
        bm->RBCestimation.RBCspeciesParam[species][TACest_id] / 1000.0);

	for (nf = 0; nf < Nfleets; nf++)
		fprintf(fid, " %e", bm->RBCestimation.RBCspeciesArray[species].CatchData[nf][region][year]);
    for (nf = Nfleets; nf < bm->K_num_fisheries; nf++)
        fprintf(fid, " 0");

    for (nf = 0; nf < Nfleets; nf++)
		fprintf(fid, " %e", bm->RBCestimation.RBCspeciesArray[species].DiscData[nf][region][year]);  // TODO: FIX I think this was real discards in Sally's code

    for (nf = Nfleets; nf < bm->K_num_fisheries; nf++)
        fprintf(fid, " 0");

    /* Sally deleted code snippet GA was here */

	fprintf(fid,"\n");

	return;

}

/*********************************************************************************
 * Calls amoeba search (also known as downhill_simplex) - from numerical recipes
 *
 * Nov 2026: Amoeba() now uses all npar+1 vertices and returns the best point in row 0.
 * On return:
 *   - simplex[0] holds the best point and func[0] its objective value
 *   - xpar is left at the best point (and the objective evaluated there)
 *   - ok is TRUE if converged within iter function evaluations, FALSE otherwise
*********************************************************************************/
void downhill_simplex(MSEBoxModel *bm, int funkflag, int species, double **simplex, double *func, int npar,
		double ftol, int iter, int *ok, FILE *llogfp, double *xpar) {

	int Ilow = 0;
	int nfunk = 0;
	int nyears = (int)(ceil(bm->dayt/365.0));

	// Point to the amoeba routine in the atassess library
	Amoeba(bm, 0, species, bm->dayt, FunctGroupArray[species].groupCode, simplex, func, npar, ftol, iter, funkflag,
			FunctGroupArray[species].numCohorts, nyears, 0, &nfunk, &Ilow, llogfp, xpar);

	if (nfunk < iter)
		*ok = TRUE;
	else
		*ok = FALSE;

	/* Leave xpar (and any model state set by the objective function) at the best point */
	func[0] = Funk(bm, funkflag, species, FunctGroupArray[species].numCohorts, npar, nyears, 0, simplex[0], llogfp, xpar);

	return;
}


/*********************************************************************************
 * Handles output files - creating and closing
 *********************************************************************************/
void Open_Tiered_Output_File(MSEBoxModel *bm) {
    
    bm->tierRBCfp = Init_tierRBC_File(bm);
    histCPUEfp = Init_histCPUE_File(bm);
    tier5fp = Init_tier5_File(bm);
    
}

void Close_Tiered_Output_Files(MSEBoxModel *bm) {

    Util_Close_Output_File(bm->tierRBCfp);
    Util_Close_Output_File(histCPUEfp);
    Util_Close_Output_File(tier5fp);
        
}

/**
 *	\brief Routines to initialise the v Biomass information file
 *
 */
static FILE * Init_tierRBC_File(MSEBoxModel *bm) {
	FILE *fid;
	char fname[STRLEN];
	int nf;
    int Nfleets = bm->K_num_fisheries;
    
	/** Create filename **/
	snprintf(fname, sizeof(fname), "%sTierRBC.txt", bm->startfname);
    
    /** Create file **/
    if ( (fid=Util_fopen(bm, fname, "w")) == NULL )
        quit("Init_tierRBC_File: Can't open %s\n",fname);
    
    /** Column definitions **/
	fprintf(fid, "Time Group year region sb0 SpawnBio RelSpawnBio AssessFail RBCest TACdisc TACpost TACest");
    
    for (nf = 0; nf < Nfleets; nf++)
		fprintf(fid, " fleet%d-catch", nf);
    
	for (nf = 0; nf < Nfleets; nf++)
		fprintf(fid, " fleet%d-discard", nf);
    
    fprintf(fid, "\n");
    
	/* Return file pointer */
	return (fid);
}

/**
 *	\brief Routines to initialise the v Biomass information file
 *
 */
static FILE * Init_histCPUE_File(MSEBoxModel *bm) {
	FILE *fid;
	char fname[STRLEN];
    
	/** Create filename **/
	snprintf(fname, sizeof(fname), "%sHistCPUE.txt", bm->startfname);
    
    /** Create file **/
    if ( (fid=Util_fopen(bm, fname, "w")) == NULL )
        quit("Init_tierRBC_File: Can't open %s\n",fname);
    
    /** Column definitions **/
	fprintf(fid, "Time Group year region CPUEgen CPUEfleet CPUEtarg\n");
    
	/* Return file pointer */
	return (fid);
}

/**
 *	\brief Routines to initialise the v Biomass information file
 *
 */
static FILE * Init_tier5_File(MSEBoxModel *bm) {
	FILE *fid;
	char fname[STRLEN];
    
	/** Create filename **/
	snprintf(fname, sizeof(fname), "%sTier5.txt", bm->startfname);
    
    /** Create file **/
    if ( (fid=Util_fopen(bm, fname, "w")) == NULL )
        quit("Init_tier5_File: Can't open %s\n",fname);
    

    /** Column definitions **/
	fprintf(fid, "Time Group year bstart k r depletion F48 F40 F20 ftarg Bcurr RBCest\n");
    
	/* Return file pointer */
	return (fid);
}

/*************************************************************************************
 *	\brief Routines to implement US style tier buffering between OFL (RBC) and ABC/AFL (TAC)
 *  Nov 2026 (review item 31): returns the buffered value used to calculate the TAC; the RBC (RBCest) is left unchanged
 *
 *  Nov 2026: tier is now the effective tier passed in from DoTierAssessment (so rollover years
 *  are recognised rather than re-applying the tier buffer to last year's TAC), and the buffer
 *  scalars are read from RBCspeciesParam (were read from the biological speciesParams array).
 */
double Apply_RBC_Buffers(MSEBoxModel *bm, int species, int year, int tier, double TACbase, FILE *llogfp) {
    double sig1 = bm->RBCestimation.RBCspeciesParam[species][Tier1Sig_id];
    double sig2 = bm->RBCestimation.RBCspeciesParam[species][Tier2Sig_id];
    double sig3 = bm->RBCestimation.RBCspeciesParam[species][Tier3Sig_id];
    double RBC = TACbase;

	switch (tier){
        case tier1: /* ----------- Tier 1 --------------------- */
        case dyntier1B0:
            RBC = sig1 * TACbase;
            break;
        case tier2: /* ----------- Tier 2 --------------------- */
            RBC = sig2 * TACbase;
            break;
        case tier3: /* ------------ Tier 3 --------------------- */
            RBC = sig3 * TACbase;
            break;
        case tier4: // ----------------  tier 4 --------------------
        case dyntier4:
            RBC = sig3 * TACbase;
            break;
        case tier0: /* ----------- No future catches ---------- */
        case tier5: //  ---------------- tier 5 --------------------
        case tier6: //  ---------------- tier 6 --------------------
        case tier7: //  ---------------- tier 7 --------------------
        case tier8: //  ---------------- tier 8 --------------------
        case tier9: //  ---------------- tier 9 --------------------
        case tier13: //  ---------------- tier 13 Iceland escapement --------------------
        case tier14: //  ---------------- tier 14 perfect information ACLIM --------------------
            // Nothing to do for these cases as yet
            break;
        case sp_rollover:
            // Nov 2026 (review item 29): the multi-year TAC decay (myTACbuffer) is now applied in Apply_MYTAC_Decay,
            // called from DoTierAssessment whether or not UseTierBuffers is set, so nothing to do here.
            break;
        default:
            printf("No such tier (%d)\n", tier);
            fprintf(llogfp,"No such tier (%d)\n", tier);
            quit("");
            break;
            
	}
    if (RBC != TACbase) {
        fprintf(llogfp, "Time: %e %s year: %d tier buffer applied (tier %d): RBC %e becomes %e on the way to the TAC\n",
                bm->dayt, FunctGroupArray[species].groupCode, year, tier, TACbase, RBC);
    }

    // Nov 2026 (review item 31): returned rather than overwriting RBCest, so the RBC stays the assessment estimate
    return RBC;
}

/*************************************************************************************
 *	\brief Multi-year TAC decay in rollover years (Nov 2026, review item 29)
 *  RBCest is last year's TAC, which already includes the decay for earlier rollover years,
 *  so apply one more year of the MYTAC buffer (giving (1 - myTACbuffer)^years since assessment overall).
 *  Acts on (and returns) the value used to calculate the TAC; the RBC is not changed.
 *  Only within a multi-year TAC (buff_yr > 0) - trigger rollovers outside a multi-year TAC are not decayed.
 *  Not applied if the Ratpack staleness buffer is being used instead (already applied to RBCest).
 *  Used to be part of Apply_RBC_Buffers, so only happened when UseTierBuffers was set.
 */
double Apply_MYTAC_Decay(MSEBoxModel *bm, int species, int year, double TACbase, FILE *llogfp) {
    int endyr = (int)(bm->RBCestimation.RBCspeciesParam[species][myTACend_id]);
    double buff_yr = (double)(bm->RBCestimation.RBCspeciesParam[species][myTACperiod_sp_id] - (endyr - year));
    double newTACbase;

    if (bm->RBCestimation.myTACbuffer <= 0.0)
        return TACbase;
    if (bm->RBCestimation.RBCspeciesParam[species][Buffer_stale_rate_id] > 0.0)
        return TACbase;
    if (buff_yr <= 0)
        return TACbase;

    newTACbase = (1.0 - bm->RBCestimation.myTACbuffer) * TACbase;

    fprintf(llogfp, "Time: %e %s year: %d multi-year TAC decay (1 - myTACbuffer): %e (years since assessment: %e) %e becomes %e on the way to the TAC\n",
            bm->dayt, FunctGroupArray[species].groupCode, year, (1.0 - bm->RBCestimation.myTACbuffer), buff_yr, TACbase, newTACbase);

    return newTACbase;
}

/*************************************************************************************
 *	\brief Ratpack staleness buffer (SetBuffer in Ratpack RatpackR.R)
 *  During multi-year TAC (or trigger) rollover years the staleness buffer
 *  max(Buffer_min, 1 - (years since assessment) * Buffer_stale_rate) is applied relative to the last
 *  successful assessment. Only called when Buffer_stale_rate > 0 and the species is in a rollover year.
 *  Nov 2026 (review item 31): applied to both
 *   (b) the RBC: RBCest = buffer * RBC from the last assessment (unbuffered estimate, stored in RBC_by_year)
 *   (a) the TAC path: returns buffer * the last assessment's buffered value (Ratpack_baseRBC), from which the
 *       new TAC is calculated. Since the tier buffers are multipliers this is the same as re-applying them
 *       to the staleness adjusted RBC.
 *  Returns -1 if there has been no successful assessment yet (TAC path then rolls over last year's TAC).
 */
double Ratpack_Staleness_Buffer(MSEBoxModel *bm, int species, int year, FILE *llogfp) {
    int endyr = (int)(bm->RBCestimation.RBCspeciesParam[species][myTACend_id]);
    double buff_yr = (double)(bm->RBCestimation.RBCspeciesParam[species][myTACperiod_sp_id] - (endyr - year));
    double stale_rate = bm->RBCestimation.RBCspeciesParam[species][Buffer_stale_rate_id];
    double stale_buffer;

    if (buff_yr < 0.0)
        buff_yr = 0.0;

    stale_buffer = 1.0 - buff_yr * stale_rate;
    if (stale_buffer < bm->RBCestimation.RBCspeciesParam[species][Buffer_min_id])
        stale_buffer = bm->RBCestimation.RBCspeciesParam[species][Buffer_min_id];

    if (!(bm->RBCestimation.RBCspeciesArray[species].lastAssessRBC > 0.0) || !(bm->RBCestimation.RBCspeciesParam[species][Ratpack_baseRBC_id] > 0.0)) {
        fprintf(llogfp, "Time: %e %s year: %d WARNING - no successful assessment yet so Ratpack staleness buffer not applied (last year's TAC rolled over)\n",
                bm->dayt, FunctGroupArray[species].groupCode, year);
        return -1.0;
    }

    // (b) the RBC
    bm->RBCestimation.RBCspeciesParam[species][RBCest_id] = stale_buffer * bm->RBCestimation.RBCspeciesArray[species].lastAssessRBC;

    fprintf(llogfp, "Time: %e %s year: %d Ratpack staleness buffer: %e (years since assessment: %e) last assessed RBC: %e RBC: %e buffered base: %e TAC base: %e\n",
            bm->dayt, FunctGroupArray[species].groupCode, year, stale_buffer, buff_yr,
            bm->RBCestimation.RBCspeciesArray[species].lastAssessRBC, bm->RBCestimation.RBCspeciesParam[species][RBCest_id],
            bm->RBCestimation.RBCspeciesParam[species][Ratpack_baseRBC_id], stale_buffer * bm->RBCestimation.RBCspeciesParam[species][Ratpack_baseRBC_id]);

    // (a) the TAC path
    return stale_buffer * bm->RBCestimation.RBCspeciesParam[species][Ratpack_baseRBC_id];
}

/********************************************************************************************************
 * \brief  Do PGMSY assessment across species
 *
 *  Routine to call PGMSY R scripts
 */
void Call_PGMSY(MSEBoxModel *bm, int year, FILE *llogfp){

    if(bm->RBCestimation.earliestYr > year) {
       // Do nothing as no assessments running yet
    } else {
        
        if (bm->RBCestimation.UseAtlantisPGMSY) {
            Do_Atlantis_PGMSY(bm, year);
        } else {
        
#ifdef RASSESS_LINK_ENABLED
        
            printf("Attempting to run PGMSY as year: %d and earliestYr: %d\n", year, bm->RBCestimation.earliestYr);
        
            Do_PGMSY_R(bm, year, llogfp);
#endif
        }
    }
    
    return;
}

/********************************************************************************************************
 * \brief  Do multispecies assessment and get updated TAC
 *
 *  Routine to do the specified muiltspecies assessment adjustments
 */
void DoMultiStockAssessment(MSEBoxModel *bm, int year, FILE *llogfp){
    int species, assessType;

    printf("DoMultiStockAssessment Time: %e year %d vs earliestYr: %d ReviewYr: %d\n", bm->dayt, year, bm->RBCestimation.earliestYr, bm->RBCestimation.ReviewYr);
    
    fprintf(bm->logFile, "DoMultiStockAssessment Time: %e year %d vs earliestYr: %d ReviewYr: %d\n", bm->dayt, year, bm->RBCestimation.earliestYr, bm->RBCestimation.ReviewYr);
    
    if(bm->RBCestimation.earliestYr > year) {
        // Do nothing as no assessments running yet
    } else {
    
        if (year == bm->RBCestimation.ReviewYr) {
            Check_Indicator_Links(bm, year);
            bm->RBCestimation.ReviewYr = year + bm->RBCestimation.ReviewPeriod;
        }
    
        for (species = 0; species < bm->K_num_tot_sp; species++) {
            if (FunctGroupArray[species].isTAC > 1) {
                assessType = (int) (bm->RBCestimation.RBCspeciesParam[species][MultispAssessType_id]);

                printf("DoMultiStockAssessment Time: %e year %d doing species %s with assessType: %d\n", bm->dayt, year, FunctGroupArray[species].groupCode, assessType);
            
                fprintf(bm->logFile, "DoMultiStockAssessment Time: %e year %d doing species %s with assessType: %d\n", bm->dayt, year, FunctGroupArray[species].groupCode, assessType);
            
                switch(assessType) {
                    case MultiSpProd:
                        // Need to add call to multispecies production model
                        break;
                    case IndicatorSp:
                    case IndicatorSpPGMSY:
                        // Indicator species assessed in main tier code, this spot does the non indicator species
                        CalculateNonIndicatorRBC(bm, species, bm->RBCestimation.sim, year);
                        break;
                    case PGMSY:
                    case SingleSpOnly:
                    case NoAssess:
                    default: // Do nothing (when set to 0 or PGMSY as have flow through from PGMSY)
                        break;
                }
                
    
                // Store the results - do we need separate loops for this?
                if (assessType > NoAssess) {
                    Evaluation(bm, species, year, llogfp, bm->tierRBCfp);
                }
             }
        }
    }
    
    return;
}

/********************************************************************************************************
 * \brief  Trigger species breakout routine
 */
int trigger_species_breakout(MSEBoxModel *bm, int sp, int this_year){

    //TODO: set up this_year = bm->RBCestimation.RBCspeciesParam[sp].mgt.rbcYear
    
    int f = 0, r = 0, y = 0;
    //int LTcount = 0;
    int LTincr = FALSE;
    int STincr = FALSE;
    int TACcaptured = FALSE;
    double totCatch = 0;
    double lastCPUE = 0;
    int yy = 0;
    int lag = 4;
    double lastC = 0;
    double ans = 0;
    
    printf("Checking if %s is prevAssessment %e\n", FunctGroupArray[sp].groupCode, bm->RBCestimation.RBCspeciesParam[sp][prevAssessment_id]);

    if (!bm->RBCestimation.RBCspeciesParam[sp][prevAssessment_id]){
        //no previous assessment has been done AND it is a trigger species, best to do an assessment
         fprintf(bm->logFile, "Time: %e %s TRIGGER species BREAKOUT: no previous assessment\n", bm->dayt, FunctGroupArray[sp].groupCode);
         return 1;
    }
    
    // MARKET TEST
    // if recent catches are greater than long-term catches, breakout and do an assessment
    // the problem with testing this breakout rule however is that we pretty much assume that
    // all TACs are taken, i.e. there is no market effects ...
    lastC = 0;
    for (r = 0; r < bm->RBCestimation.RBCspeciesParam[sp][NumRegions_id]; r++) {
        for (f = 0; f < bm->RBCestimation.RBCspeciesParam[sp][NumFisheries_id]; f++) {
            
            printf("Doing test for %s - r %d f %d rbcYear: %d (%d)\n", FunctGroupArray[sp].groupCode, r, f, bm->RBCestimation.RBCspeciesArray[sp].mgt_rbcYear, bm->RBCestimation.RBCspeciesArray[sp].mgt_rbcYear - 1);

            lastC += bm->RBCestimation.RBCspeciesArray[sp].CatchData[f][r][bm->RBCestimation.RBCspeciesArray[sp].mgt_rbcYear - 1];
        }
    }
    
    // it is more than likely that this test will be triggered.
    // Nov 2026: compared to last year's TAC (TAC_by_year), as in Ratpack, since RBC_by_year now holds RBCs
    if (lastC > (bm->RBCestimation.RBCspeciesParam[sp][trigger_threshold_id] * bm->RBCestimation.RBCspeciesArray[sp].TAC_by_year[this_year - 1])) {
        TACcaptured = TRUE;
    }

    fprintf(bm->logFile, "Time: %e %s CATCH TRIGGER %f %f\n", bm->dayt, FunctGroupArray[sp].groupCode, lastC, bm->RBCestimation.RBCspeciesArray[sp].TAC_by_year[this_year - 1]);
    fprintf(bm->logFile, "Time: %e %s TRIGGER species: last catch: %f threshold * last TAC: %f  TACcaptured: %d\n", bm->dayt, FunctGroupArray[sp].groupCode, lastC, bm->RBCestimation.RBCspeciesParam[sp][trigger_threshold_id] * bm->RBCestimation.RBCspeciesArray[sp].TAC_by_year[this_year - 1], TACcaptured);

    if (TACcaptured) {
        fprintf(bm->logFile, "Time: %e %s TRIGGER species BREAKOUT: CATCH / MARKET test\n", bm->dayt, FunctGroupArray[sp].groupCode);
        bm->RBCestimation.RBCspeciesParam[sp][Assess_id] = TRUE;
        return 1;
    }

    // STOCK TEST
    // if recent CPUE is less than long-term catches, breakout and do an assessment
    // use a catch weighted CPUE ?
    // 1. calculate catch weighted CPUE for past 10 years
    // 2. calculate slope of regression line
    // 3. breakout if the regression line slope has been <0 for past 4 years
    lastCPUE = 0;
    totCatch = 0;
    for (r = 0; r < bm->RBCestimation.RBCspeciesParam[sp][NumRegions_id]; r++) {
        for (f = 0; f < bm->RBCestimation.RBCspeciesParam[sp][NumFisheries_id]; f++) {
            totCatch += bm->RBCestimation.RBCspeciesArray[sp].CatchData[f][r][this_year - 1];
        }
    }
    for (r = 0; r < bm->RBCestimation.RBCspeciesParam[sp][NumRegions_id]; r++) {
        for (f = 0; f < bm->RBCestimation.RBCspeciesParam[sp][NumFisheries_id]; f++) {
            //catch weighted CPUE
            lastCPUE += (bm->RBCestimation.RBCspeciesArray[sp].CatchData[f][r][this_year - 1] / totCatch) * bm->RBCestimation.RBCspeciesArray[sp].CPUEgen[f][r][this_year - 1];
        }
    }
    
    /* (long-term) calculate 10-year trend */
    /* over past 4 years */
    Util_Init_1D_Double(bm->RBCestimation.catchwghtCPUE, bm->RBCestimation.TriggerCheckPeriod, 0.0);
    for (lag = bm->RBCestimation.LagPeriod; lag >= 0; lag--){
        for (y = (bm->RBCestimation.RBCspeciesArray[sp].mgt_rbcYear - bm->RBCestimation.TriggerCheckPeriod - lag); y < (this_year - 1 - lag); y++){
            totCatch = 0;
            for (r = 0; r < bm->RBCestimation.RBCspeciesParam[sp][NumRegions_id]; r++) {
                for (f = 0; f < bm->RBCestimation.RBCspeciesParam[sp][NumFisheries_id]; f++) {
                    totCatch += bm->RBCestimation.RBCspeciesArray[sp].CatchData[f][r][y];
                }
            }
        
            yy = y - (this_year - bm->RBCestimation.TriggerCheckPeriod) + 1 + lag;
            for (r = 0; r < bm->RBCestimation.RBCspeciesParam[sp][NumRegions_id]; r++) {
                for (f = 0; f < bm->RBCestimation.RBCspeciesParam[sp][NumFisheries_id]; f++) {
                
                    bm->RBCestimation.catchwghtCPUE[yy] += (bm->RBCestimation.RBCspeciesArray[sp].CatchData[f][r][y] / totCatch) * bm->RBCestimation.RBCspeciesArray[sp].CPUEgen[f][r][y];
                    //fprintf(bm->logFile, "LAG %d catched weighted CPUE %d --- %d %f\n" lag, y, yy, catchwghtCPUE[yy]);
                }
            }
        }
        fprintf(bm->logFile, "Time: %e %s CPUE TRIGGER : TRIGGER species Long-term trend\n", bm->dayt, FunctGroupArray[sp].groupCode);
        for (y = 0; y < bm->RBCestimation.TriggerCheckPeriod; y++){
            fprintf(bm->logFile, "Time: %e %s y: %d catchwghtCPUE: %f\n", bm->dayt, FunctGroupArray[sp].groupCode, y, bm->RBCestimation.catchwghtCPUE[y]);
        }
        fprintf(bm->logFile, "Time: %e %s regressionSlope: %f\n", bm->dayt, FunctGroupArray[sp].groupCode, regressionSlope(bm->RBCestimation.catchwghtCPUE,bm->RBCestimation.TriggerCheckPeriod,bm->RBCestimation.TriggerCheckPeriod,bm->logFile));
            
        if (regressionSlope(bm->RBCestimation.catchwghtCPUE,bm->RBCestimation.TriggerCheckPeriod,bm->RBCestimation.TriggerCheckPeriod,bm->logFile) > 0) LTincr = TRUE;

        /* calculate (short-term) 1-year trend */
        if (lag == 0){
            // CPUE in last 4 years
            if (regressionSlope(bm->RBCestimation.catchwghtCPUE,bm->RBCestimation.TriggerCheckPeriod,bm->RBCestimation.LagPeriod,bm->logFile) > 0) STincr = TRUE;
            ans  = regressionSlope(bm->RBCestimation.catchwghtCPUE,bm->RBCestimation.TriggerCheckPeriod,bm->RBCestimation.LagPeriod,bm->logFile);
            fprintf(bm->logFile, "Time: %e %s regressionSlope %e\n", bm->dayt, FunctGroupArray[sp].groupCode, ans);
        }
    }

    if (!STincr && !LTincr) {
        fprintf(bm->logFile, "Time: %e %s TRIGGER species BREAKOUT: CPUE test\n", bm->dayt, FunctGroupArray[sp].groupCode);
        fprintf(bm->logFile, "Time: %e %s STincr: %d LTincr: %d\n", bm->dayt, FunctGroupArray[sp].groupCode, STincr, LTincr);
        bm->RBCestimation.RBCspeciesParam[sp][Assess_id] = TRUE;
        return 1;
    } else {
        printf("For %s no incr check and return\n", FunctGroupArray[sp].groupCode);
        
        fprintf(bm->logFile, "Time: %e %s TRIGGER species ROLLOVER\n", bm->dayt, FunctGroupArray[sp].groupCode);
        bm->RBCestimation.RBCspeciesParam[sp][Assess_id] = FALSE;
        return 0;
    }
    
}


//******************************************************************************
//
// Name:  CalculateNonIndicatorRBC
// Description: Calculates the RBC for unassessed, non-indicator species
//
// created  : Dec 2021 Rich Little, translated to Atlantis by Beth Fulton
//
//******************************************************************************
void CalculateNonIndicatorRBC(MSEBoxModel *bm, int sp, int isim, int year){
    int i = 0, nf;
    double avgInd_deltaRBC = 0, TAC, TAC_old, TACnf;
    int nInds = 0;
    
    if (bm->RBCestimation.UseCategory) {
        // If using entire category
        bm->RBCestimation.RBCspeciesParam[sp][TACpost_id] = bm->RBCestimation.RBCspeciesParam[sp][TACest_id]; // Last year's TAC
        for (i = 0; i < bm->K_num_tot_sp; i++) {
            if (sp == i || (FunctGroupArray[sp].isImpacted == FALSE) || bm->RBCestimation.RBCspeciesParam[sp][mgt_category_id] != bm->RBCestimation.RBCspeciesParam[i][mgt_category_id] || !bm->RBCestimation.RBCspeciesParam[i][mgt_indicator_id]) continue;
            nInds++;
            
            if (!bm->RBCestimation.RBCspeciesArray[i].RBC_by_year[year - 1]) {
                avgInd_deltaRBC += 1.0;
            } else {
                avgInd_deltaRBC += bm->RBCestimation.RBCspeciesArray[i].RBC_by_year[year] / bm->RBCestimation.RBCspeciesArray[i].RBC_by_year[year - 1];
            }
        }
        
        if (nInds > 0) {
            avgInd_deltaRBC /= nInds;
        } else {
            quit("ERROR: No Indicators for %s\n", FunctGroupArray[sp].groupCode);
        }
    
        bm->RBCestimation.RBCspeciesParam[sp][avgInd_deltaRBC_id] = avgInd_deltaRBC;
    } else {
        // Only one paired species
        i = bm->RBCestimation.RBCspeciesParam[sp][mgt_sp_id];
        
        if (!bm->RBCestimation.RBCspeciesArray[i].RBC_by_year[year - 1]) {
            avgInd_deltaRBC = 1.0;
        } else {
            avgInd_deltaRBC = bm->RBCestimation.RBCspeciesArray[i].RBC_by_year[year] / bm->RBCestimation.RBCspeciesArray[i].RBC_by_year[year - 1];
        }
        bm->RBCestimation.RBCspeciesParam[sp][avgInd_deltaRBC_id] = avgInd_deltaRBC;
        
    }
    
    // New value is previous TAC * delta (Based on indicator species TAC change)
    bm->RBCestimation.RBCspeciesParam[sp][TACest_id] = bm->RBCestimation.RBCspeciesParam[sp][TACpost_id] * bm->RBCestimation.RBCspeciesParam[sp][avgInd_deltaRBC_id];
    
    // Allocate the new TAC's per fleet
    TAC = bm->RBCestimation.RBCspeciesParam[sp][TACest_id];
    TAC_old = bm->RBCestimation.RBCspeciesParam[sp][TAC_old_id];
    
    if(isnan(TAC)){
        quit("TAC is nan for %s in CalculateNonIndicatorRBC as TACpost: %e, avgInd_delta: %e\n", FunctGroupArray[sp].groupCode, bm->RBCestimation.RBCspeciesParam[sp][TACpost_id], bm->RBCestimation.RBCspeciesParam[sp][avgInd_deltaRBC_id]);
    }
        
    for (nf = 0; nf < bm->K_num_fisheries; nf++) {
        if (!bm->FISHERYprms[nf][flagrecfish_id] && bm->inQuota[nf][sp]) {
            
            // Proportion of the total TAC to provide this fleet
            TACnf = TAC * (bm->TACamt[sp][nf][now_id] / (TAC_old + small_num));
            
            // save TAC value so can compare to next year
            bm->TACamt[sp][nf][old_id] = bm->TACamt[sp][nf][now_id];
            bm->TACamt[sp][nf][now_id] = TACnf;
         }
    }    
    return;
}


//******************************************************************************
//
// Name:  Check_Indicator_Links
// Description: Checks which indicator species the non-indicator species is linked to
//
// created  : April 2023 Beth Fulton
//
//******************************************************************************
void Check_Indicator_Links(MSEBoxModel *bm, int year) {
    int y, yy, r, f, lag, sp, ind_sp, ref_sp, ref_spB, my_category, that_category, itier, maxTier,
        maxCount = 0, starty = 0, endy = 0;
    double diff, closest_slope, closest_slopeB, buffer, totCatch, step1, step2;
    
    printf("Doing Check_Indicator_Links\n");
    
    // First year so use the initial pairing
    for (sp = 0; sp < bm->K_num_tot_sp; sp++) {
        if (FunctGroupArray[sp].isImpacted == TRUE) {
            if( year == bm->RBCestimation.RBCspeciesParam[sp][AssessStart_id]) {
                bm->RBCestimation.RBCspeciesParam[sp][mgt_sp_id] = bm->RBCestimation.RBCspeciesParam[sp][init_mgt_sp_id];
                bm->RBCestimation.RBCspeciesParam[sp][mgt_category_id] = bm->RBCestimation.RBCspeciesParam[sp][init_mgt_category_id];
            }
        }
    }
    
    printf("Doing Check_Indicator_Links step A\n");
    
    // Check index change over "running" reference period - use CPUE for now but could be B or F
    // Including check direction of change (increase or decrease in catch) - could use B, CPUE as alternatives
    for (sp = 0; sp < bm->K_num_tot_sp; sp++) {
        if (FunctGroupArray[sp].isImpacted == TRUE) {
            
            printf("Doing Check_Indicator_Links Doing species %s ", FunctGroupArray[sp].groupCode);
            
            Util_Init_1D_Double(bm->RBCestimation.catchwghtCPUE, bm->RBCestimation.GradientWindow, 0.0);
            Util_Init_1D_Double(bm->RBCestimation.catchwght, bm->RBCestimation.GradientWindow, 0.0);
            for (lag = bm->RBCestimation.GradientPeriod; lag >= 0; lag--){
                
                printf("Doing Check_Indicator_Links Doing species %s lag %d (RBCestimation.GradientPeriod: %d)\n", FunctGroupArray[sp].groupCode, lag, bm->RBCestimation.GradientPeriod);
                
                starty = (bm->RBCestimation.RBCspeciesArray[sp].mgt_rbcYear - bm->RBCestimation.GradientWindow - lag);
                
                if (starty < 0)
                    starty = 0;
                endy = (year - 1 - lag);
                if (endy < 0)
                    endy = 0;
                
                for (y = starty; y < endy; y++){
                    totCatch = 0;
                    for (r = 0; r < bm->RBCestimation.RBCspeciesParam[sp][NumRegions_id]; r++) {
                        for (f = 0; f < bm->RBCestimation.RBCspeciesParam[sp][NumFisheries_id]; f++) {
                            totCatch += bm->RBCestimation.RBCspeciesArray[sp].CatchData[f][r][y];
                        }
                    }
            
                    yy = y - (year - bm->RBCestimation.GradientPeriod) + 1 + lag;  // TODO: Check whether this should be a different period - is it ok like this?
                    
                    if (yy < 0)
                        yy = 0;

                    for (r = 0; r < bm->RBCestimation.RBCspeciesParam[sp][NumRegions_id]; r++) {
                        for (f = 0; f < bm->RBCestimation.RBCspeciesParam[sp][NumFisheries_id]; f++) {
                            bm->RBCestimation.catchwghtCPUE[yy] += (bm->RBCestimation.RBCspeciesArray[sp].CatchData[f][r][y] / totCatch) * bm->RBCestimation.RBCspeciesArray[sp].CPUEgen[f][r][y];
                            bm->RBCestimation.catchwght[yy] += bm->RBCestimation.RBCspeciesArray[sp].CatchData[f][r][y] / totCatch;
                        }
                    }
                }
            
                /*
                 fprintf(bm->logFile, "Time: %e %s CPUE CHECK : Check direction of change for species to understadn classification in indicator species harvest strategy\n", bm->dayt, FunctGroupArray[sp].groupCode);
                 for (y = 0; y < bm->RBCestimation.GradientWindow; y++){
                    fprintf(bm->logFile, "Time: %e %s y: %d catchwghtCPUE: %f\n", bm->dayt, FunctGroupArray[sp].groupCode, y, bm->RBCestimation.catchwghtCPUE[y]);
                 }
                 fprintf(bm->logFile, "Time: %e %s regressionSlope: %f\n", bm->dayt, FunctGroupArray[sp].groupCode, regressionSlope(bm->RBCestimation.catchwghtCPUE,bm->RBCestimation.GradientWindow,bm->RBCestimation.GradientWindow,bm->logFile));
                 */
            
                bm->RBCestimation.RBCspeciesParam[sp][catchtrend_id] = regressionSlope(bm->RBCestimation.catchwght,bm->RBCestimation.GradientWindow,bm->RBCestimation.GradientWindow,bm->logFile);
                bm->RBCestimation.RBCspeciesParam[sp][CPUEtrend_id] = regressionSlope(bm->RBCestimation.catchwghtCPUE,bm->RBCestimation.GradientWindow,bm->RBCestimation.GradientWindow,bm->logFile);
                buffer = bm->RBCestimation.GradientBuffer * bm->RBCestimation.RBCspeciesParam[sp][CPUEtrend_id];
                if (bm->RBCestimation.RBCspeciesParam[sp][CPUEtrend_id] > (0 + buffer)) {
                    bm->RBCestimation.RBCspeciesParam[sp][CPUEdir_id] = 1;
                } else if (bm->RBCestimation.RBCspeciesParam[sp][CPUEtrend_id] < (0 - buffer)) {
                    bm->RBCestimation.RBCspeciesParam[sp][CPUEdir_id] = -1;
                } else {
                    bm->RBCestimation.RBCspeciesParam[sp][CPUEtrend_id] = 0;
                }
            }
        }
    }
    
    printf("Doing Check_Indicator_Links step B\n");
    
    /** Align with closest species - Based on CPUE **/
    for (sp = 0; sp < bm->K_num_tot_sp; sp++) {
        if (FunctGroupArray[sp].isImpacted == TRUE) {
            Util_Init_1D_Int(bm->RBCestimation.count_sp_in_buffer, K_num_indicator_tiers, 0);
            closest_slope = MAXDOUBLE;
            ref_sp = -1;
            buffer = bm->RBCestimation.GradientBuffer * bm->RBCestimation.RBCspeciesParam[sp][CPUEtrend_id];
            my_category = bm->RBCestimation.RBCspeciesParam[sp][mgt_category_id];
            
            // First check only those species of same management type
            for (ind_sp = 0; ind_sp < bm->K_num_tot_sp; ind_sp++) {
                if( FunctGroupArray[ind_sp].isImpacted == TRUE) {
                    if (bm->RBCestimation.RBCspeciesParam[sp][mgt_category_id] == bm->RBCestimation.RBCspeciesParam[ind_sp][mgt_category_id]) {
                        diff = fabs(bm->RBCestimation.RBCspeciesParam[sp][CPUEtrend_id] - bm->RBCestimation.RBCspeciesParam[ind_sp][CPUEtrend_id]);
                        if (diff < closest_slope) {
                            ref_sp = ind_sp;
                        }
                        if (diff < buffer) {
                            bm->RBCestimation.count_sp_in_buffer[my_category]++;
                        }
                    }
                }
            }
            
            // Now check other categories
            closest_slopeB = MAXDOUBLE;
            ref_spB = -1;
            for (ind_sp = 0; ind_sp < bm->K_num_tot_sp; ind_sp++) {
                if( FunctGroupArray[ind_sp].isImpacted == TRUE) {
                    that_category = bm->RBCestimation.RBCspeciesParam[ind_sp][mgt_category_id];
                    if (bm->RBCestimation.RBCspeciesParam[sp][mgt_category_id] != bm->RBCestimation.RBCspeciesParam[ind_sp][mgt_category_id]) {
                        diff = bm->RBCestimation.RBCspeciesParam[sp][CPUEtrend_id] - bm->RBCestimation.RBCspeciesParam[ind_sp][CPUEtrend_id];
                        if (diff < closest_slopeB) {
                            ref_spB = ind_sp;
                        }
                        if (diff < buffer) {
                            bm->RBCestimation.count_sp_in_buffer[that_category]++;
                        }
                    }
                }
            }
            
            // Check for absolute closest
            if (bm->RBCestimation.UseClosest) {
                // Use closest regardless of management category
                if ( closest_slope < closest_slopeB ){
                    bm->RBCestimation.RBCspeciesParam[sp][mgt_sp_id] = ref_sp; // Species from the same category
                } else {
                    bm->RBCestimation.RBCspeciesParam[sp][mgt_sp_id] = ref_spB; // Species from another category
                }
                
                maxTier = mgt_category_id;
                maxCount = 0;
                for (itier = 0; itier < K_num_indicator_tiers; itier++ ){
                    if (bm->RBCestimation.count_sp_in_buffer[itier] > maxCount) {
                        maxCount = bm->RBCestimation.count_sp_in_buffer[itier];
                        maxTier = itier;
                    }
                }
                bm->RBCestimation.RBCspeciesParam[sp][mgt_category_id] = maxTier;
            } else {
                // Use closest of own category
                bm->RBCestimation.RBCspeciesParam[sp][mgt_category_id] = bm->RBCestimation.RBCspeciesParam[sp][init_mgt_category_id];
                bm->RBCestimation.RBCspeciesParam[sp][mgt_sp_id] = ref_sp;
            }
        }
    }
    
    printf("Doing Check_Indicator_Links step C\n");
    
    /** Align with closest species - Based on Catch and CPUE**/
    for (sp = 0; sp < bm->K_num_tot_sp; sp++) {
        if (FunctGroupArray[sp].isImpacted == TRUE) {
            Util_Init_1D_Int(bm->RBCestimation.count_sp_in_buffer, K_num_indicator_tiers, 0);
            closest_slope = MAXDOUBLE;
            ref_sp = -1;
            buffer = bm->RBCestimation.GradientBuffer * bm->RBCestimation.RBCspeciesParam[sp][CPUEtrend_id];
            my_category = bm->RBCestimation.RBCspeciesParam[sp][mgt_category_id];
            
            // First check only those species of same management type
            for (ind_sp = 0; ind_sp < bm->K_num_tot_sp; ind_sp++) {
                if( FunctGroupArray[ind_sp].isImpacted == TRUE) {
                    if (bm->RBCestimation.RBCspeciesParam[sp][mgt_category_id] == bm->RBCestimation.RBCspeciesParam[ind_sp][mgt_category_id]) {
                        step1 = (bm->RBCestimation.RBCspeciesParam[sp][CPUEtrend_id] - bm->RBCestimation.RBCspeciesParam[ind_sp][CPUEtrend_id]) * (bm->RBCestimation.RBCspeciesParam[sp][CPUEtrend_id] - bm->RBCestimation.RBCspeciesParam[ind_sp][CPUEtrend_id]);
                        step2 = (bm->RBCestimation.RBCspeciesParam[sp][catchtrend_id] - bm->RBCestimation.RBCspeciesParam[ind_sp][catchtrend_id]) * (bm->RBCestimation.RBCspeciesParam[sp][catchtrend_id] - bm->RBCestimation.RBCspeciesParam[ind_sp][catchtrend_id]);
                        diff = sqrt(step1 + step2);
                        if (diff < closest_slope) {
                            ref_sp = ind_sp;
                        }
                        if (diff < buffer) {
                            bm->RBCestimation.count_sp_in_buffer[my_category]++;
                        }
                    }
                }
            }
            
            // Now check other categories
            closest_slopeB = MAXDOUBLE;
            ref_spB = -1;
            for (ind_sp = 0; ind_sp < bm->K_num_tot_sp; ind_sp++) {
                if( FunctGroupArray[ind_sp].isImpacted == TRUE) {
                    that_category = bm->RBCestimation.RBCspeciesParam[ind_sp][mgt_category_id];
                    if (bm->RBCestimation.RBCspeciesParam[sp][mgt_category_id] != bm->RBCestimation.RBCspeciesParam[ind_sp][mgt_category_id]) {
                        step1 = (bm->RBCestimation.RBCspeciesParam[sp][CPUEtrend_id] - bm->RBCestimation.RBCspeciesParam[ind_sp][CPUEtrend_id]) * (bm->RBCestimation.RBCspeciesParam[sp][CPUEtrend_id] - bm->RBCestimation.RBCspeciesParam[ind_sp][CPUEtrend_id]);
                        step2 = (bm->RBCestimation.RBCspeciesParam[sp][catchtrend_id] - bm->RBCestimation.RBCspeciesParam[ind_sp][catchtrend_id]) * (bm->RBCestimation.RBCspeciesParam[sp][catchtrend_id] - bm->RBCestimation.RBCspeciesParam[ind_sp][catchtrend_id]);
                        diff = sqrt(step1 + step2);
                        if (diff < closest_slopeB) {
                            ref_spB = ind_sp;
                        }
                        if (diff < buffer) {
                            bm->RBCestimation.count_sp_in_buffer[that_category]++;
                        }
                    }
                }
            }
            
            // Check for absolute closest
            if (bm->RBCestimation.UseClosest) {
                // Use closest regardless of management category
                if ( closest_slope < closest_slopeB ){
                    bm->RBCestimation.RBCspeciesParam[sp][mgt_sp_id] = ref_sp; // Species from the same category
                } else {
                    bm->RBCestimation.RBCspeciesParam[sp][mgt_sp_id] = ref_spB; // Species from another category
                }
                
                maxTier = mgt_category_id;
                maxCount = 0;
                for (itier = 0; itier < K_num_indicator_tiers; itier++ ){
                    if (bm->RBCestimation.count_sp_in_buffer[itier] > maxCount) {
                        maxCount = bm->RBCestimation.count_sp_in_buffer[itier];
                        maxTier = itier;
                    }
                }
                bm->RBCestimation.RBCspeciesParam[sp][mgt_category_id] = maxTier;
            } else {
                // Use closest of own category
                bm->RBCestimation.RBCspeciesParam[sp][mgt_category_id] = bm->RBCestimation.RBCspeciesParam[sp][init_mgt_category_id];
                bm->RBCestimation.RBCspeciesParam[sp][mgt_sp_id] = ref_sp;
            }
        }
    }
    
    printf("Doing Check_Indicator_Links step D\n");
    
    return;
}

//******************************************************************************
//
// Name:  regressionSlope
// called by: trigger_species_breakout
//
//******************************************************************************
double regressionSlope(double *y, int ysize, int n, FILE *llogfp) {
    int i = 0;
    double sumx = 0, sumy = 0;
    double avgx = 0, avgy = 0;
    double ssxx = 0, ssxy = 0;

    //n is the number of observation from the end of the y array

    for (i = ysize-n; i < ysize; i++){
        //fprintf(llogfp,"%d %e\n", i, y[i]);
        sumy += y[i];
        sumx += i;
    }
    avgx = sumx / n;
    avgy = sumy / n;
    for (i = ysize-n; i < ysize; i++){
        ssxx += pow((i - avgx),2);
        ssxy += (y[i] - avgy) * (i - avgx);
    }
    //fprintf(llogfp,"** ssxy / ssxx %e\n", ssxy / ssxx);
    return (ssxy / ssxx);
}

/*********************************************************************************************************************************
Dynamic Tuer 4 routines
 ********************************************************************************************************************************/

//******************************************************************************
//
// Name:  DynamicTier4Assessment
// called by: DoAssessment()
//
// moved from R code so no system calls required
//
//******************************************************************************
void DynamicTier4Assessment(MSEBoxModel *bm, int species, int year, FILE *llogfp) {



    return;
}

/*********************************************************************************************************************************
 Ratpack (PuntRatpackR) routines - ACLIM harvest control rules and native C dynamic Tier 4
 ********************************************************************************************************************************/

//******************************************************************************
//
// Name:  ACLIM_HCR
// Description: ACLIM / GOA-CLIM alternative harvest control rules (types 1-10).
//              Direct port of ACLIM_HCR() in PuntRatpackR "MISC HCR/'HCR_ACLIM.R"
//              (K. Holsman, June 2025). Returns maxFabc, which is an F multiplier when Flim = 1.
//
//              Differences from the R version:
//              - gamma and theta are passed on the natural scale (R takes log_gamma and log_theta)
//              - inv.logit(x) is coded as 1/(1+exp(-x))
//
//  B2B0 = current depletion (B/B0)
//  alpha = B/Btarget at which F reaches zero on the ramp
//  gamma = sensitivity parameter (types 5, 6, 9 and 10)
//  theta = scalar on SSB (type 8)
//  omega1-3 = covariate effects on F, Btarget and Blim (type 7)
//  cov = scaled covariate (types 7 and 9)
//  B2B0_lim, B2B0_target = limit and target depletion
//  Flim = F to which the multiplier is applied
//
// called by : Calc_ACLIM_Fmult
// created  : Nov 2026 Beth Fulton (ported from Ratpack)
//
//******************************************************************************
double ACLIM_HCR(int type, double B2B0, double alpha, double gamma, double theta, double omega1, double omega2, double omega3, double cov,
                 double B2B0_lim, double B2B0_target, double Flim) {
    double B2B40 = 0.0;
    double maxFabc = 0.0;
    double invlogit;

    switch (type) {
        case 1:
        case 2:
        case 3:
        case 4:
            B2B40 = B2B0 / B2B0_target;
            if (B2B40 > 1.0) {
                maxFabc = Flim;
            } else {
                if (alpha < B2B40)
                    maxFabc = Flim * ((B2B40 - alpha) / (1.0 - alpha));
                else
                    maxFabc = 0.0;
            }
            break;
        case 5:
        case 6:
            B2B40 = B2B0 / B2B0_target;
            if (B2B40 > 1.0) {
                maxFabc = Flim * exp(-gamma * (B2B40 - 1.0));
            } else {
                if (alpha < B2B40)
                    maxFabc = Flim * ((B2B40 - alpha) / (1.0 - alpha));
                else
                    maxFabc = 0.0;
            }
            break;
        case 7:
            // omega 1 and 2 are environmental effect parameters, cov is the scaled covariate
            B2B0_target = B2B0_target * exp(-1.0 * omega2 * cov);
            B2B0_lim = B2B0_lim * exp(-1.0 * omega3 * cov);
            B2B40 = B2B0 / B2B0_target;
            if (B2B40 > 1.0) {
                maxFabc = Flim;
            } else {
                if (alpha < B2B40)
                    maxFabc = Flim * ((B2B40 - alpha) / (1.0 - alpha));
                else
                    maxFabc = 0.0;
            }
            maxFabc = maxFabc * exp(omega1 * cov);
            break;
        case 8:
            B2B40 = (theta * B2B0) / B2B0_target;
            if (B2B40 > 1.0) {
                maxFabc = Flim;
            } else {
                if (alpha < B2B40)
                    maxFabc = Flim * ((B2B40 - alpha) / (1.0 - alpha));
                else
                    maxFabc = 0.0;
            }
            break;
        case 9:
            B2B40 = B2B0 / B2B0_target;
            if (B2B40 > 1.0) {
                invlogit = 1.0 / (1.0 + exp(cov));   // inv.logit(-1 * cov)
                maxFabc = Flim * exp(-gamma * (invlogit / 0.5) * (B2B40 - 1.0));
            } else {
                if (alpha < B2B40)
                    maxFabc = Flim * ((B2B40 - alpha) / (1.0 - alpha));
                else
                    maxFabc = 0.0;
            }
            break;
        case 10:
            // decay inverse to SSB/Btarget above 1 + gamma
            B2B40 = B2B0 / B2B0_target;
            if (B2B40 > 1.0) {
                maxFabc = Flim;
                if (B2B40 >= (1.0 + gamma)) {
                    maxFabc = Flim / (B2B40 / (1.0 + gamma));
                }
            } else {
                if (alpha < B2B40)
                    maxFabc = Flim * ((B2B40 - alpha) / (1.0 - alpha));
                else
                    maxFabc = 0.0;
            }
            break;
        default:
            quit("ACLIM_HCR: HCR type %d not recognised - must be an integer between 1 and 10\n", type);
            break;
    }

    // Hard limit
    if (B2B40 <= (B2B0_lim / B2B0_target))
        maxFabc = 0.0;

    return maxFabc;
}

//******************************************************************************
//
// Name:  Calc_ACLIM_Fmult
// Description: Collects the species' ACLIM HCR parameters and returns the F multiplier
//              for the given depletion level.
//
//              HCR 2 (lagged recovery): once depletion falls below HCR_Blim the stock is
//              flagged as rebuilding until depletion exceeds HCR_Btarget. While rebuilding
//              HCR_alpha is used (ACLIM white paper value 0.30); otherwise the standard
//              ACLIM alpha of 0.05 is used.
//              HCR 4 (marine heatwave): alpha = HCR_alpha + 0.09 * MHW category, where the
//              category is given by HCR_cov (ACLIM white paper).
//
// called by : TierOneMimic, RatpackDynTier4Assessment
// created  : Nov 2026 Beth Fulton (ported from Ratpack)
//
//******************************************************************************
double Calc_ACLIM_Fmult(MSEBoxModel *bm, int species, double depletion, FILE *llogfp) {
    int type = (int)(bm->RBCestimation.RBCspeciesParam[species][HCRtype_id]);
    double alpha = bm->RBCestimation.RBCspeciesParam[species][HCR_alpha_id];
    double gamma = bm->RBCestimation.RBCspeciesParam[species][HCR_gamma_id];
    double theta = bm->RBCestimation.RBCspeciesParam[species][HCR_theta_id];
    double omega1 = bm->RBCestimation.RBCspeciesParam[species][HCR_omega1_id];
    double omega2 = bm->RBCestimation.RBCspeciesParam[species][HCR_omega2_id];
    double omega3 = bm->RBCestimation.RBCspeciesParam[species][HCR_omega3_id];
    double cov = bm->RBCestimation.RBCspeciesParam[species][HCR_cov_id];
    double Blim = bm->RBCestimation.RBCspeciesParam[species][HCR_Blim_id];
    double Btarget = bm->RBCestimation.RBCspeciesParam[species][HCR_Btarget_id];
    double Fmult;

    if (Btarget <= 0.0)
        quit("Calc_ACLIM_Fmult: HCR_Btarget for %s must be > 0\n", FunctGroupArray[species].groupCode);

    switch (type) {
        case 2:  // Lagged recovery
            if (depletion < Blim) {
                bm->RBCestimation.RBCspeciesParam[species][HCR_rebuilding_id] = 1;
            } else if (depletion > Btarget) {
                bm->RBCestimation.RBCspeciesParam[species][HCR_rebuilding_id] = 0;
            }
            if (!bm->RBCestimation.RBCspeciesParam[species][HCR_rebuilding_id])
                alpha = 0.05;   // Standard ACLIM alpha when not rebuilding
            break;
        case 4:  // Marine heatwave category adjusts the slope
            alpha = alpha + 0.09 * cov;
            break;
        default:
            // Nothing extra to do
            break;
    }

    Fmult = ACLIM_HCR(type, depletion, alpha, gamma, theta, omega1, omega2, omega3, cov, Blim, Btarget, 1.0);

    fprintf(llogfp, "Time: %e %s ACLIM HCR type: %d depletion: %e alpha: %e Blim: %e Btarget: %e rebuilding: %e Fmult: %e\n",
            bm->dayt, FunctGroupArray[species].groupCode, type, depletion, alpha, Blim, Btarget,
            bm->RBCestimation.RBCspeciesParam[species][HCR_rebuilding_id], Fmult);

    return Fmult;
}

/* Data and settings passed to the Ratpack dynamic Tier 4 objective function */
typedef struct {
    int nyr;               // Number of data years
    int nidx;              // Number of CPUE series
    int EstR;              // Estimate r (1) or derive it from median catch (0)
    int EstZ;              // Estimate z (1) or derive it from MSYL (0)
    int ProcessError;      // 0 = no process error, 1 = penalised deviations
    int MSYRY1;            // First year index of period assumed to be at BMSY
    int MSYRY2;            // Last year index of period assumed to be at BMSY
    int npar;              // Number of estimated parameters
    double MSYINPUT;       // Median catch used to derive r when not estimated
    double MSYL;           // Input MSYL
    double z;              // Shape parameter derived from MSYL
    double CVMSYL;         // CV of MSYL penalty
    double PriorMeanR;     // Prior mean r
    double PriorSDr;       // Prior sd r
    double SigmaR;         // Process error sd (fixed)
    double *catch_obs;     // Total removals by year [nyr]
    double **cpue;         // CPUE by series and year [nidx][nyr] (<= 0 means missing)
    double *B;             // Biomass at the start of each year [nyr+1]
} DT4DataStruct;

//******************************************************************************
//
// Name:  DT4_FindZ
// Description: Find the Pella-Tomlinson shape parameter z that gives the requested MSYL,
//              i.e. solve 1 - (z+1)*MSYL^z = 0 by bisection on [0.0001, 10]
//              (uniroot(FindZ) in Ratpack Assessment.R)
//
//******************************************************************************
static double DT4_FindZ(double msyl) {
    double zlo = 0.0001, zhi = 10.0, zmid = 1.0, flo, fmid;
    int i;

    flo = 1.0 - (zlo + 1.0) * pow(msyl, zlo);
    for (i = 0; i < 200; i++) {
        zmid = 0.5 * (zlo + zhi);
        fmid = 1.0 - (zmid + 1.0) * pow(msyl, zmid);
        if ((fmid > 0.0) == (flo > 0.0)) {
            zlo = zmid;
            flo = fmid;
        } else {
            zhi = zmid;
        }
        if ((zhi - zlo) < 1.0e-10)
            break;
    }
    return zmid;
}

//******************************************************************************
//
// Name:  DT4_Derived
// Description: Get K, r, z, MSYL and BMSY from the parameter vector
//              x[0] = log(K), then log(r) if EstR, then log(z) if EstZ, then process deviations
//
//******************************************************************************
static void DT4_Derived(DT4DataStruct *d, double *x, double *K, double *r, double *z, double *MSYLout, double *BMSY) {
    int ip = 1;
    double logR = 0.0;

    *K = exp(x[0]);
    if (d->EstR) {
        logR = x[ip];
        ip++;
    }
    if (d->EstZ) {
        *z = exp(x[ip]);
        *MSYLout = pow(1.0 / (*z + 1.0), 1.0 / *z);
    } else {
        *z = d->z;
        *MSYLout = d->MSYL;
    }
    *BMSY = (*MSYLout) * (*K);
    if (d->EstR)
        *r = exp(logR);
    else
        *r = d->MSYINPUT / ((*BMSY) * (1.0 - pow(*MSYLout, *z)));

    return;
}

//******************************************************************************
//
// Name:  DT4_Objective
// Description: Negative log-likelihood for the Ratpack dynamic Tier 4 Pella-Tomlinson model
//              (dynTier4.cpp / f2() in Ratpack Assessment.R)
//
//  Differences to the Ratpack version:
//  - catch is conditioned on (exploitation rate = C/B) rather than estimating an exploitation
//    rate for every year and fitting catch with sd = 0.05. If C/B exceeds 0.95 the exploitation
//    rate is capped and the Ratpack catch likelihood (sd = 0.05) is applied to the shortfall.
//  - catchability and sigma of each CPUE series are concentrated out (analytical MLEs)
//
//******************************************************************************
static double DT4_Objective(DT4DataStruct *d, double *x) {
    int t, i, n, ndev;
    double K, r, z, MSYLout, BMSY, Bt, Bnext, expl, dev, nll = 0.0, pen = 0.0;
    double resid, sumresid, sumsq, logq, sigma2, bioave, minB;

    DT4_Derived(d, x, &K, &r, &z, &MSYLout, &BMSY);
    if (!isfinite(K) || !isfinite(r) || !isfinite(z) || (K <= 0.0) || (r <= 0.0) || (z <= 0.0))
        return 1.0e20;

    ndev = d->npar - d->nyr;   // index of first process deviation (only used if ProcessError)
    minB = 1.0e-6 * K;

    // Population dynamics
    d->B[0] = K;
    for (t = 0; t < d->nyr; t++) {
        Bt = d->B[t];
        expl = d->catch_obs[t] / Bt;
        if (expl > 0.95) {
            pen += (log(d->catch_obs[t]) - log(0.95 * Bt)) * (log(d->catch_obs[t]) - log(0.95 * Bt)) / (2.0 * 0.05 * 0.05);
            expl = 0.95;
        }
        Bnext = Bt + r * Bt * (1.0 - pow(Bt / K, z)) - expl * Bt;
        if (d->ProcessError) {
            dev = x[ndev + t];
            Bnext = Bnext * exp(dev * d->SigmaR - 0.5 * d->SigmaR * d->SigmaR);
            nll += 0.5 * dev * dev;
        }
        if (Bnext < minB) {
            pen += 1000.0 * ((minB - Bnext) / K) * ((minB - Bnext) / K);
            Bnext = minB;
        }
        d->B[t + 1] = Bnext;
    }

    // CPUE likelihood - lognormal with concentrated q and sigma per series
    for (i = 0; i < d->nidx; i++) {
        n = 0;
        sumresid = 0.0;
        for (t = 0; t < d->nyr; t++) {
            if (d->cpue[i][t] > 0.0) {
                sumresid += log(d->cpue[i][t]) - log(0.5 * (d->B[t] + d->B[t + 1]));
                n++;
            }
        }
        if (n < 2)
            continue;
        logq = sumresid / (double)n;
        sumsq = 0.0;
        for (t = 0; t < d->nyr; t++) {
            if (d->cpue[i][t] > 0.0) {
                resid = log(d->cpue[i][t]) - log(0.5 * (d->B[t] + d->B[t + 1])) - logq;
                sumsq += resid * resid;
            }
        }
        sigma2 = sumsq / (double)n;
        if (sigma2 < 1.0e-6)
            sigma2 = 1.0e-6;
        nll += 0.5 * (double)n * log(sigma2) + 0.5 * (double)n;
    }

    // MSYL penalty - biomass over the specified period assumed to be around BMSY
    if ((d->CVMSYL > 0.0) && (d->MSYRY2 >= d->MSYRY1) && (d->MSYRY1 >= 0) && (d->MSYRY2 < (d->nyr + 1))) {
        bioave = 0.0;
        for (t = d->MSYRY1; t < (d->MSYRY2 + 1); t++)
            bioave += d->B[t];
        bioave /= (double)(d->MSYRY2 - d->MSYRY1 + 1);
        nll += (log(bioave) - log(BMSY)) * (log(bioave) - log(BMSY)) / (d->CVMSYL * d->CVMSYL);
    }

    // Normal prior on r
    if (d->PriorSDr > 0.0)
        nll += 0.5 * ((r - d->PriorMeanR) / d->PriorSDr) * ((r - d->PriorMeanR) / d->PriorSDr);

    nll += pen;
    if (!isfinite(nll))
        return 1.0e20;

    return nll;
}

//******************************************************************************
//
// Name:  DT4_Gradient
// Description: Central difference gradient of DT4_Objective
//
//******************************************************************************
static void DT4_Gradient(DT4DataStruct *d, double *x, double *grad) {
    int i;
    double h, xsave, fplus, fminus;

    for (i = 0; i < d->npar; i++) {
        xsave = x[i];
        h = 1.0e-5 * (1.0 + fabs(xsave));
        x[i] = xsave + h;
        fplus = DT4_Objective(d, x);
        x[i] = xsave - h;
        fminus = DT4_Objective(d, x);
        x[i] = xsave;
        grad[i] = (fplus - fminus) / (2.0 * h);
    }
    return;
}

//******************************************************************************
//
// Name:  DT4_Minimise
// Description: Quasi-Newton (BFGS) minimisation of DT4_Objective with numerical gradients
//              and a backtracking line search. Restarts (from the best point so far) up to
//              8 times, mirroring the nlminb restarts used in Ratpack.
//              Returns the minimum objective value; x holds the best parameters and
//              maxgrad the largest absolute gradient component at the solution.
//
//******************************************************************************
static double DT4_Minimise(DT4DataStruct *d, double *x, int maxit, double *maxgrad) {
    int npar = d->npar;
    int i, j, it, irestart, nhalf;
    double f, fnew, fstart, step, slope, sy, rho, pmax, hy, yhy;
    double *g = Util_Alloc_Init_1D_Double(npar, 0.0);
    double *gnew = Util_Alloc_Init_1D_Double(npar, 0.0);
    double *p = Util_Alloc_Init_1D_Double(npar, 0.0);
    double *s = Util_Alloc_Init_1D_Double(npar, 0.0);
    double *y = Util_Alloc_Init_1D_Double(npar, 0.0);
    double *xnew = Util_Alloc_Init_1D_Double(npar, 0.0);
    double *Hy = Util_Alloc_Init_1D_Double(npar, 0.0);
    double **H = Util_Alloc_Init_2D_Double(npar, npar, 0.0);

    f = DT4_Objective(d, x);

    for (irestart = 0; irestart < 9; irestart++) {
        fstart = f;

        // Reset the inverse Hessian approximation to the identity
        for (i = 0; i < npar; i++) {
            for (j = 0; j < npar; j++)
                H[i][j] = 0.0;
            H[i][i] = 1.0;
        }
        DT4_Gradient(d, x, g);

        for (it = 0; it < maxit; it++) {
            // Search direction
            slope = 0.0;
            for (i = 0; i < npar; i++) {
                p[i] = 0.0;
                for (j = 0; j < npar; j++)
                    p[i] -= H[i][j] * g[j];
                slope += p[i] * g[i];
            }
            if (slope >= 0.0) {
                // Not a descent direction so use steepest descent
                for (i = 0; i < npar; i++) {
                    for (j = 0; j < npar; j++)
                        H[i][j] = 0.0;
                    H[i][i] = 1.0;
                    p[i] = -g[i];
                }
                slope = 0.0;
                for (i = 0; i < npar; i++)
                    slope += p[i] * g[i];
            }

            // Limit step size (parameters are on log scale or are N(0,1) deviations)
            pmax = 0.0;
            for (i = 0; i < npar; i++) {
                if (fabs(p[i]) > pmax)
                    pmax = fabs(p[i]);
            }
            step = 1.0;
            if (pmax * step > 2.0)
                step = 2.0 / pmax;

            // Backtracking line search (Armijo condition)
            fnew = f;
            for (nhalf = 0; nhalf < 40; nhalf++) {
                for (i = 0; i < npar; i++)
                    xnew[i] = x[i] + step * p[i];
                fnew = DT4_Objective(d, xnew);
                if (fnew <= (f + 1.0e-4 * step * slope))
                    break;
                step *= 0.5;
            }
            if (!(fnew < f))
                break;   // No improvement possible along this direction

            DT4_Gradient(d, xnew, gnew);

            // BFGS update of inverse Hessian
            sy = 0.0;
            for (i = 0; i < npar; i++) {
                s[i] = xnew[i] - x[i];
                y[i] = gnew[i] - g[i];
                sy += s[i] * y[i];
            }
            if (sy > 1.0e-12) {
                rho = 1.0 / sy;
                yhy = 0.0;
                for (i = 0; i < npar; i++) {
                    hy = 0.0;
                    for (j = 0; j < npar; j++)
                        hy += H[i][j] * y[j];
                    Hy[i] = hy;
                    yhy += y[i] * hy;
                }
                for (i = 0; i < npar; i++) {
                    for (j = 0; j < npar; j++) {
                        H[i][j] += (1.0 + rho * yhy) * rho * s[i] * s[j] - rho * (Hy[i] * s[j] + s[i] * Hy[j]);
                    }
                }
            }

            for (i = 0; i < npar; i++) {
                x[i] = xnew[i];
                g[i] = gnew[i];
            }

            if (fabs(f - fnew) < 1.0e-12 * (1.0 + fabs(f))) {
                f = fnew;
                break;
            }
            f = fnew;

            *maxgrad = 0.0;
            for (i = 0; i < npar; i++) {
                if (fabs(g[i]) > *maxgrad)
                    *maxgrad = fabs(g[i]);
            }
            if (*maxgrad < 1.0e-6)
                break;
        }

        // Stop restarting if the last restart gave no real improvement
        if ((irestart > 0) && (fabs(fstart - f) < 1.0e-8 * (1.0 + fabs(f))))
            break;
    }

    DT4_Gradient(d, x, g);
    *maxgrad = 0.0;
    for (i = 0; i < npar; i++) {
        if (fabs(g[i]) > *maxgrad)
            *maxgrad = fabs(g[i]);
    }

    free1d(g);
    free1d(gnew);
    free1d(p);
    free1d(s);
    free1d(y);
    free1d(xnew);
    free1d(Hy);
    free2d(H);

    return f;
}

//******************************************************************************
//
// Name:  RatpackDynTier4Assessment
// Description: Native C version of the Ratpack (PuntRatpackR) dynamic Tier 4 assessment
//              (Run_Tier4a in Ratpack Assessment.R; dynTier4.cpp in MISC HCR).
//              Fits a Pella-Tomlinson production model to total removals and CPUE then
//              projects forward using the hockey stick control rule (or an ACLIM rule if
//              HCRtype > 0), or a constant harvest rate, to give the RBC.
//
//              Selected with tier = dyntier4 (10) and tiertype = ratpack_tier_rule (5).
//              Year indices used by the parameters are data year indices (0 = first data year),
//              biomass B[t] is biomass at the start of data year t.
//
// called by : DoTierAssessment
// created  : Nov 2026 Beth Fulton (ported from Ratpack)
//
//******************************************************************************
void RatpackDynTier4Assessment(MSEBoxModel *bm, int species, int year, FILE *llogfp) {
    DT4DataStruct d;
    int Nfleets = (int)(bm->RBCestimation.RBCspeciesParam[species][NumFisheries_id]);
    int Nregions = (int)(bm->RBCestimation.RBCspeciesParam[species][NumRegions_id]);
    int Nstocks = FunctGroupArray[species].numStocks;
    int CPUEfleet = (int)(bm->RBCestimation.RBCspeciesParam[species][DT4_CPUEfleet_id]);
    int ABCoption = (int)(bm->RBCestimation.RBCspeciesParam[species][DT4_ABCoption_id]);
    int HCRtype = (int)(bm->RBCestimation.RBCspeciesParam[species][HCRtype_id]);
    int MSYPriorY1 = (int)(bm->RBCestimation.RBCspeciesParam[species][DT4_MSYPriorY1_id]);
    int MSYPriorY2 = (int)(bm->RBCestimation.RBCspeciesParam[species][DT4_MSYPriorY2_id]);
    double DT4alpha = bm->RBCestimation.RBCspeciesParam[species][DT4_alpha_id];
    double DT4beta = bm->RBCestimation.RBCspeciesParam[species][DT4_beta_id];
    double DT4target = bm->RBCestimation.RBCspeciesParam[species][DT4_target_id];
    double PGMSYlimit = bm->RBCestimation.RBCspeciesParam[species][DT4_PGMSYlimit_id];
    double K_init_mult = bm->RBCestimation.RBCspeciesParam[species][DT4_K_init_mult_id];
    int nf, iy, i, j, nyr, nidx, ipar, nmedian;
    int AssessFail = FALSE;
    double catchy, discy, nll, maxgrad = 0.0, K, r, z, MSYLout, BMSY, MSY, Bproj, Depl, Ftarg, Fval, RBC = 0.0, Kinit, sb0;
    double *medcatch, *x;
    int *fleetidx;

    nyr = year + 1;   // data years 0 .. year (year is the final year of data after any assessment delay)
    if (nyr < 2)
        quit("RatpackDynTier4Assessment: not enough years of data for %s (year %d)\n", FunctGroupArray[species].groupCode, year);

    if (bm->RBCestimation.RBCspeciesParam[species][DT4_ProcessError_id] > 1) {
        fprintf(llogfp, "Time: %e %s WARNING: DT4_ProcessError 2 (random effects) is only available in the R version (tiertype 1). Using penalised process deviations (option 1) instead\n",
                bm->dayt, FunctGroupArray[species].groupCode);
        warn("RatpackDynTier4Assessment: DT4_ProcessError 2 (random effects) is only available in the R version - using option 1 for %s\n", FunctGroupArray[species].groupCode);
    }

    /* Which fleets supply CPUE indices */
    fleetidx = (int *)i_alloc1d(Nfleets);
    nidx = 0;
    for (nf = 0; nf < Nfleets; nf++) {
        if ((CPUEfleet < 0) || (CPUEfleet == nf)) {
            fleetidx[nidx] = nf;
            nidx++;
        }
    }
    if (nidx < 1)
        quit("RatpackDynTier4Assessment: DT4_CPUEfleet (%d) for %s does not match any assessment fleet\n", CPUEfleet, FunctGroupArray[species].groupCode);

    /* Set up the data */
    d.nyr = nyr;
    d.nidx = nidx;
    d.EstR = (int)(bm->RBCestimation.RBCspeciesParam[species][DT4_EstR_id]);
    d.EstZ = (int)(bm->RBCestimation.RBCspeciesParam[species][DT4_EstZ_id]);
    d.ProcessError = (bm->RBCestimation.RBCspeciesParam[species][DT4_ProcessError_id] > 0) ? 1 : 0;
    d.MSYRY1 = (int)(bm->RBCestimation.RBCspeciesParam[species][DT4_MSYRY1_id]);
    d.MSYRY2 = (int)(bm->RBCestimation.RBCspeciesParam[species][DT4_MSYRY2_id]);
    d.MSYL = bm->RBCestimation.RBCspeciesParam[species][DT4_MSYL_id];
    d.CVMSYL = bm->RBCestimation.RBCspeciesParam[species][DT4_CVMSYL_id];
    d.PriorMeanR = bm->RBCestimation.RBCspeciesParam[species][DT4_PriorMeanR_id];
    d.PriorSDr = bm->RBCestimation.RBCspeciesParam[species][DT4_PriorSDr_id];
    d.SigmaR = 0.1;   // Fixed as in Ratpack for penalised deviations
    d.catch_obs = Util_Alloc_Init_1D_Double(nyr, 0.0);
    d.B = Util_Alloc_Init_1D_Double(nyr + 1, 0.0);
    d.cpue = Util_Alloc_Init_2D_Double(nyr, nidx, 0.0);

    if ((d.MSYL <= 0.0) || (d.MSYL >= 1.0))
        quit("RatpackDynTier4Assessment: DT4_MSYL for %s must be between 0 and 1 (currently %e)\n", FunctGroupArray[species].groupCode, d.MSYL);
    d.z = DT4_FindZ(d.MSYL);
    d.MSYL = pow(1.0 / (d.z + 1.0), 1.0 / d.z);

    // Total removals (retained catch plus discards - so RBC is on the same basis as other tiers) and CPUE
    for (iy = 0; iy < nyr; iy++) {
        catchy = 0.0;
        discy = 0.0;
        for (nf = 0; nf < Nfleets; nf++) {
            if (bm->RBCestimation.RBCspeciesParam[species][DiscType_id] == 1)      // discard data is biomass
                discy += bm->RBCestimation.RBCspeciesArray[species].DiscData[nf][Nregions][iy];
            else if (bm->RBCestimation.RBCspeciesParam[species][DiscType_id] == 2)  // discard data is rate
                discy += bm->RBCestimation.RBCspeciesArray[species].DiscData[nf][Nregions][iy] * bm->RBCestimation.RBCspeciesArray[species].CatchData[nf][Nregions][iy] / (1.0 - bm->RBCestimation.RBCspeciesArray[species].DiscData[nf][Nregions][iy] + small_num);
            catchy += bm->RBCestimation.RBCspeciesArray[species].CatchData[nf][Nregions][iy];
        }
        d.catch_obs[iy] = catchy + discy;
        for (i = 0; i < nidx; i++)
            d.cpue[i][iy] = bm->RBCestimation.RBCspeciesArray[species].CPUEgen[fleetidx[i]][Nregions][iy];   // Nregions index holds the all-region value
    }

    // Median catch for MSY input (used when r is not estimated)
    if ((MSYPriorY2 < 1) || (MSYPriorY2 > (nyr - 1)))
        MSYPriorY2 = nyr - 1;
    if ((MSYPriorY1 < 0) || (MSYPriorY1 > MSYPriorY2))
        MSYPriorY1 = 0;
    nmedian = MSYPriorY2 - MSYPriorY1 + 1;
    medcatch = Util_Alloc_Init_1D_Double(nmedian, 0.0);
    for (iy = MSYPriorY1; iy < (MSYPriorY2 + 1); iy++)
        medcatch[iy - MSYPriorY1] = d.catch_obs[iy];
    for (i = 1; i < nmedian; i++) {   // insertion sort
        catchy = medcatch[i];
        j = i - 1;
        while ((j >= 0) && (medcatch[j] > catchy)) {
            medcatch[j + 1] = medcatch[j];
            j--;
        }
        medcatch[j + 1] = catchy;
    }
    if (nmedian % 2)
        d.MSYINPUT = medcatch[nmedian / 2];
    else
        d.MSYINPUT = 0.5 * (medcatch[nmedian / 2 - 1] + medcatch[nmedian / 2]);
    free1d(medcatch);

    /* Parameters: log(K), log(r) if EstR, log(z) if EstZ, process deviations if ProcessError */
    d.npar = 1 + d.EstR + d.EstZ + d.ProcessError * nyr;
    x = Util_Alloc_Init_1D_Double(d.npar, 0.0);

    // Initial values - K from true SB0 if K_init_mult > 0 (as in Ratpack), otherwise from catch (as in standalone dynTier4)
    if ((bm->RBCestimation.RBCspeciesParam[species][Regime_year_id] > 0) && (year >= bm->RBCestimation.RBCspeciesParam[species][Regime_year_id]))
        sb0 = bm->RBCestimation.RBCspeciesArray[species].SBioZero_shift[Nstocks][Nregions];
    else
        sb0 = bm->RBCestimation.RBCspeciesArray[species].SBioZero[Nstocks][Nregions];
    r = exp(bm->RBCestimation.RBCspeciesParam[species][DT4_logr_init_id]);
    if ((K_init_mult > 0.0) && (sb0 > 0.0))
        Kinit = sb0 * K_init_mult;
    else
        Kinit = (d.MSYINPUT + small_num) / (r * d.MSYL * (1.0 - pow(d.MSYL, d.z)));
    if (!(Kinit > 0.0))
        quit("RatpackDynTier4Assessment: could not set an initial value of K for %s - check catch data and DT4_K_init_mult\n", FunctGroupArray[species].groupCode);

    ipar = 0;
    x[ipar] = log(Kinit);
    ipar++;
    if (d.EstR) {
        x[ipar] = bm->RBCestimation.RBCspeciesParam[species][DT4_logr_init_id];
        ipar++;
    }
    if (d.EstZ) {
        x[ipar] = log(d.z);
        ipar++;
    }
    // Process deviations start at zero

    /* Fit the model */
    nll = DT4_Minimise(&d, x, 500, &maxgrad);
    DT4_Derived(&d, x, &K, &r, &z, &MSYLout, &BMSY);
    nll = DT4_Objective(&d, x);   // Refill biomass trajectory for the best parameters
    MSY = r * BMSY * (1.0 - pow(MSYLout, z));

    if (!isfinite(nll) || (nll >= 1.0e19) || !isfinite(K) || !isfinite(r) || (K <= 0.0) || (r <= 0.0))
        AssessFail = TRUE;

    /* Harvest control rule to get the RBC (ABC in Ratpack). Ratpack projects 50 years to give a
       vector of future TACs. The first projection year is the RBC (multi-year TACs are handled by the
       Atlantis rollover code). Nov 2026 (review item 27): if using Ratpack style RBC averaging
       (UseRBCAveraging = 2) the projection is continued for nRBCaverage years and stored in RBC_proj
       so the average can be taken over the projected RBCs, as in Ratpack */
    if (!AssessFail) {
        int iproj, nproj = 1;
        double RBCproj;

        if (((int)(bm->RBCestimation.RBCspeciesParam[species][UseRBCAveraging_id]) == ratpack_RBC_avg) && (bm->RBCestimation.nRBCaverage > 1)) {
            nproj = bm->RBCestimation.nRBCaverage;
            if (nproj > MAX_RBC_PROJ)
                nproj = MAX_RBC_PROJ;
        }

        Bproj = d.B[nyr];
        for (iproj = 0; iproj < nproj; iproj++) {
            Depl = Bproj / K;
            if (ABCoption == DT4_constF_abc) {
                Ftarg = r * (1.0 - pow(PGMSYlimit, z));
                Fval = Ftarg;
            } else {
                Ftarg = r * (1.0 - pow(DT4target, z));   // Exploitation rate whose equilibrium depletion is DT4target
                if (HCRtype > 0) {
                    Fval = Ftarg * Calc_ACLIM_Fmult(bm, species, Depl, llogfp);
                } else {
                    // Hockey stick (hard version of the logistic-smoothed rule in Ratpack)
                    if (Depl <= DT4alpha)
                        Fval = 0.0;
                    else if (Depl < DT4beta)
                        Fval = Ftarg * (Depl - DT4alpha) / (DT4beta - DT4alpha);
                    else
                        Fval = Ftarg;
                }
            }
            RBCproj = Bproj * Fval;
            if (!isfinite(RBCproj) || (RBCproj < 0.0))
                RBCproj = 0.0;
            if (iproj == 0)
                RBC = RBCproj;
            bm->RBCestimation.RBCspeciesArray[species].RBC_proj[iproj] = RBCproj;

            // Project forward (as in Ratpack runDynTier4: catch = RBC, biomass floored at 1)
            Bproj = Bproj + r * Bproj * (1.0 - pow(Bproj / K, z)) - RBCproj;
            if (Bproj < 1.0)
                Bproj = 1.0;
        }
        bm->RBCestimation.RBCspeciesArray[species].nRBC_proj = nproj;
    }

    /* Store results */
    bm->RBCestimation.RBCspeciesParam[species][AssessFail_id] = AssessFail;
    if (!AssessFail) {
        bm->RBCestimation.RBCspeciesParam[species][RBCest_id] = RBC;
        bm->RBCestimation.RBCspeciesParam[species][EstB0_id] = K;
        bm->RBCestimation.RBCspeciesParam[species][EstBcurr_id] = d.B[nyr];
        bm->RBCestimation.RBCspeciesParam[species][EstDepletion_id] = d.B[nyr] / K;
    }
    bm->RBCestimation.RBCspeciesParam[species][DT4_r_id] = r;
    bm->RBCestimation.RBCspeciesParam[species][DT4_K_id] = K;
    bm->RBCestimation.RBCspeciesParam[species][DT4_z_id] = z;
    bm->RBCestimation.RBCspeciesParam[species][DT4_MSY_id] = MSY;
    bm->RBCestimation.RBCspeciesParam[species][DT4_BMSY_id] = BMSY;
    bm->RBCestimation.RBCspeciesParam[species][DT4_nll_id] = nll;
    bm->RBCestimation.RBCspeciesParam[species][DT4_maxgrad_id] = maxgrad;

    fprintf(llogfp, "Time: %e %s Ratpack dynamic Tier 4 year: %d nyr: %d nidx: %d npar: %d nll: %e maxgrad: %e r: %e K: %e z: %e MSYL: %e BMSY: %e MSY: %e MSYINPUT: %e Bcurr: %e depletion: %e RBC: %e AssessFail: %d\n",
            bm->dayt, FunctGroupArray[species].groupCode, year, nyr, nidx, d.npar, nll, maxgrad, r, K, z, MSYLout, BMSY, MSY, d.MSYINPUT,
            d.B[nyr], d.B[nyr] / K, RBC, AssessFail);
    for (iy = 0; iy < (nyr + 1); iy++) {
        fprintf(llogfp, "Time: %e %s Ratpack dynamic Tier 4 trajectory year: %d B: %e catch: %e\n",
                bm->dayt, FunctGroupArray[species].groupCode, iy, d.B[iy], (iy < nyr) ? d.catch_obs[iy] : 0.0);
    }

    free1d(x);
    free1d(d.catch_obs);
    free1d(d.B);
    free2d(d.cpue);
    i_free1d(fleetidx);

    return;
}

//******************************************************************************
//
// Name:  DynamicTier4Assessment - moved to atRlink as uses R
// Description:
//
//
// called by : DoAssessment()
// created  : Oct 2019 Rich Little
// updated  : Dec 2020 Maciej Golebiewski
// converted to Atlantis: Jan 2022
//
//******************************************************************************

void RDynamicTier4Assessment(MSEBoxModel *bm, int species, int year, FILE *llogfp) {
    char dirname[STRLEN];
    char R_ScriptName[STRLEN * 3];
    char scriptName[STRLEN * 2];
    int THIS_PATH_MAX = 1024; // Needed as linux doesn't have PATH_MAX defined like Mac does
    char cwd[THIS_PATH_MAX];
    int ret = 0;
    int idR = 1;

    /* Nov 2026: remember the current directory so can change back after the assessment (previously never changed back) */
    if (getcwd(cwd, sizeof(cwd)) != NULL) {
        // Do nothing has have remembered current directory
    } else {
        quit("RDynamicTier4Assessment: getcwd() error\n");
    }

    /* Nov 2026: R script name - use RAssessRscriptName1 if given, otherwise runDynTier4onRTMB.R in the run directory
       (RAssessRscriptName is only read in for multispecies assessments). Relative names are relative to the run directory. */
    if ((bm->RAssessRscriptName != NULL) && (bm->NumRAssessScriptNames > idR)) {
        if (bm->RAssessRscriptName[idR][0] == '/')
            snprintf(scriptName, sizeof(scriptName), "%s", bm->RAssessRscriptName[idR]);
        else
            snprintf(scriptName, sizeof(scriptName), "%s/%s", cwd, bm->RAssessRscriptName[idR]);
    } else {
        snprintf(scriptName, sizeof(scriptName), "%s/%s", cwd, "runDynTier4onRTMB.R");
    }

    /* Create directory */
    snprintf(dirname, sizeof(dirname), "DynTier4_sim_%d_year_%d", bm->RBCestimation.sim, year);
    ret = mkdir(dirname, S_IRWXU);

    /* Change directory into the new directory */
    //pass your path in the function
    ret = chdir(dirname);
    /*if the change of directory was successful it will print successful otherwise it will print not successful*/
    if (ret < 0)
        quit("runDynamicTier4Assessment: chdir change of directory to %s not successful\n", dirname);
    else
        printf("runDynamicTier4Assessment: chdir change of directory to %s successful", dirname);
    
    /* Create and run bat files - Nov 2026: new files each time as each assessment is in its own directory
       (previously the file pointers were closed but not reset so later assessments wrote to closed files) */
    bm->Dyn4File = initDynTier4File(bm);
    bm->Dyn4CTLFile = initDynTier4CTLFile(bm);
    writeDynTier4inputfile(bm, species, year, bm->Dyn4File, bm->Dyn4CTLFile);
    Util_Close_Output_File(bm->Dyn4File);
    Util_Close_Output_File(bm->Dyn4CTLFile);
    bm->Dyn4File = NULL;
    bm->Dyn4CTLFile = NULL;
    
    /* Commands to run the assessment */
    printf("Running %s\n", scriptName);
    snprintf(R_ScriptName, sizeof(R_ScriptName), "Rscript \"%s\" > dynTier4_R.log 2>&1", scriptName);
    ret = system(R_ScriptName);
    if (ret != 0) {
        fprintf(llogfp, "Time: %e %s Rscript return code: %d - possibly an error? See %s/dynTier4_R.log\n", bm->dayt, FunctGroupArray[species].groupCode, ret, dirname);
    }

    setRBC(bm, species, year, llogfp);

    /* go back to previous directory */
    ret = chdir(cwd);
    if (ret < 0)
        quit("RDynamicTier4Assessment: chdir change of directory back to %s not successful\n", cwd);

    return;

}

FILE * initDynTier4File(MSEBoxModel *bm) {
    FILE *fp;
    char *fname = "dynTier4.dat";

    /* Create file */
    if ((fp = Util_fopen(bm, fname, "w")) == NULL)
        quit("initDynTier4File: Can't open %s\n", fname);

    /* File content - nothing yet as added in another routine */
    

    /* Return file pointer */
    return (fp);
}

FILE * initDynTier4CTLFile(MSEBoxModel *bm) {
    FILE *fp;
    char *fname = "dynTier4.ctl";

    /* Create file */
    if ((fp = Util_fopen(bm, fname, "w")) == NULL)
        quit("initDynTier4CTLFile: Can't open %s\n", fname);

    /* File content - nothing yet as added in another routine */
    

    /* Return file pointer */
    return (fp);
}

//******************************************************************************
//
// Name:  writeDynTier4inputfile
// Description:
//
//
// called by : DynamicTier4Assessment
// calls :
// created  : Oct 2019 Rich Little
// converted to Atlantis: Jan 2022
//
//******************************************************************************
void writeDynTier4inputfile(MSEBoxModel *bm, int sp, int year, FILE *datfid, FILE *ctlfid) {
    int iy, nf;
    int Nregions = (int)(bm->RBCestimation.RBCspeciesParam[sp][NumRegions_id]);
    int sumregion = Nregions;  // sum over regions
    int Nfleets = (int)(bm->RBCestimation.RBCspeciesParam[sp][NumFisheries_id]);
    int CPUEfleet = (int)(bm->RBCestimation.RBCspeciesParam[sp][DT4_CPUEfleet_id]);
    double catchy, discy, cpue;

    /* Nov 2026: the control values now come from the DT4_ parameters (the mgt_tier4_ values were never set),
       the CPUE is written (previously the catch was written twice), and the year column is the data year
       index 0..year so the year based settings match the native C version (tiertype 5). Removals are
       catch plus discards, as for the C version. */
    
    // First write the content of the CTL file
    fprintf(ctlfid, "# MSYL\n");
    fprintf(ctlfid, "%e\n", bm->RBCestimation.RBCspeciesParam[sp][DT4_MSYL_id]);
    fprintf(ctlfid, "# EstR\n");
    fprintf(ctlfid, "%d\n", (int)(bm->RBCestimation.RBCspeciesParam[sp][DT4_EstR_id]));
    fprintf(ctlfid, "# EstZ\n");
    fprintf(ctlfid, "%d\n", (int)(bm->RBCestimation.RBCspeciesParam[sp][DT4_EstZ_id]));
    fprintf(ctlfid, "# BMSY Year range\n");
    fprintf(ctlfid, "%d %d\n", (int)(bm->RBCestimation.RBCspeciesParam[sp][DT4_MSYRY1_id]), (int)(bm->RBCestimation.RBCspeciesParam[sp][DT4_MSYRY2_id]));
    fprintf(ctlfid, "# Target depletion\n");
    fprintf(ctlfid, "%e\n", bm->RBCestimation.RBCspeciesParam[sp][DT4_target_id]);
    fprintf(ctlfid, "# MSY Year range\n");
    fprintf(ctlfid, "%d %d\n", (int)(bm->RBCestimation.RBCspeciesParam[sp][DT4_MSYPriorY1_id]), (int)(bm->RBCestimation.RBCspeciesParam[sp][DT4_MSYPriorY2_id]));
    fprintf(ctlfid, "# CV(MSYL)\n");
    fprintf(ctlfid, "%e\n", bm->RBCestimation.RBCspeciesParam[sp][DT4_CVMSYL_id]);
    fprintf(ctlfid, "# R(prior)\n");
    fprintf(ctlfid, "%e %e\n", bm->RBCestimation.RBCspeciesParam[sp][DT4_PriorMeanR_id], bm->RBCestimation.RBCspeciesParam[sp][DT4_PriorSDr_id]);
    fprintf(ctlfid, "# Process error\n");
    fprintf(ctlfid, "%d\n", (int)(bm->RBCestimation.RBCspeciesParam[sp][DT4_ProcessError_id]));
    fprintf(ctlfid, "# Hockey stick alpha beta\n");
    fprintf(ctlfid, "%e %e\n", bm->RBCestimation.RBCspeciesParam[sp][DT4_alpha_id], bm->RBCestimation.RBCspeciesParam[sp][DT4_beta_id]);
    
    // Now the dat file
    fprintf(datfid, "# assessment using Tier 4 of %s\n", FunctGroupArray[sp].name);
    fprintf(datfid, "# Rlibrary path\n");
    if (strlen(bm->RBCestimation.Rlibpath) > 0)
        fprintf(datfid, "%s\n", bm->RBCestimation.Rlibpath);
    else
        fprintf(datfid, "none\n");
    fprintf(datfid, "# Number of fleets\n");
    fprintf(datfid, "%d\n", Nfleets);
    fprintf(datfid, "# Range of years\n");
    fprintf(datfid, "%d %d\n", 0, year);

    fprintf(datfid,  "# Year, Catch and CPUE\n");
    for (iy = 0; iy < year + 1; iy++) {
        fprintf(datfid,  "%d ", iy);

        // Removals by fleet
        for (nf = 0; nf < Nfleets; nf++) {
            catchy = bm->RBCestimation.RBCspeciesArray[sp].CatchData[nf][sumregion][iy];
            discy = 0.0;
            if (bm->RBCestimation.RBCspeciesParam[sp][DiscType_id] == 1)      // discard data is biomass
                discy = bm->RBCestimation.RBCspeciesArray[sp].DiscData[nf][sumregion][iy];
            else if (bm->RBCestimation.RBCspeciesParam[sp][DiscType_id] == 2)  // discard data is rate
                discy = bm->RBCestimation.RBCspeciesArray[sp].DiscData[nf][sumregion][iy] * catchy / (1.0 - bm->RBCestimation.RBCspeciesArray[sp].DiscData[nf][sumregion][iy] + small_num);
            fprintf(datfid,"%e ", catchy + discy);
        }

        // CPUE by fleet (all-region value), -1 = missing or fleet not used
        for (nf = 0; nf < Nfleets; nf++) {
            cpue = bm->RBCestimation.RBCspeciesArray[sp].CPUEgen[nf][sumregion][iy];
            if ((cpue <= 0.0) || ((CPUEfleet >= 0) && (CPUEfleet != nf)))
                cpue = -1.0;
            fprintf(datfid,"%e ", cpue);
        }
        fprintf(datfid, "\n");
    }
            
    return;
}

//******************************************************************************
//
// Name:  setRBC
// Description:
//
//
// called by : DynamicTier4Assessment()
// calls :
// created  : Oct 2019 Rich Little
// converted to Atlantis: Jan 2022
//
//******************************************************************************
void setRBC(MSEBoxModel *bm, int species, int year, FILE *llogfp) {
    char const *outname = "dynTier4.rep";
    //char str[1000];
    char *str = NULL;
    double d0;
    int count = 0;
    //char *line_buf = NULL;
    //int line_count = 0;
    size_t line_buf_size = 1000;
    ssize_t line_size;
    FILE *fp;
    char *ret;

    printf("setting dynTier4 RBC for species %s\n", FunctGroupArray[species].name);

    // Check that this file has been created - otherwise ss3 has failed
    if ((fp = fopen(outname, "rt")) == NULL) {
        quit("runDynTier4.R failed to generate %s output file\n", outname);
    }

    count = 0;
    line_size = getline(&str, &line_buf_size, fp);
    
    while (strncmp(str,"\"Maximum gradient component:",28) !=0 && count < 1000) {
        count++;
        //printf("%d %s\n", count, str);
        line_size = getline(&str, &line_buf_size, fp);
    }
    if (count >= 1000) {
        quit("setRBC: Can't find Maximum gradient component in %s output file\n", outname);
    } else {
        printf("str **** %s \n", str);
        ret = strrchr(str, '\"'); // get everything after last  ' " '
        //printf("ret **** %s \n", ret);
        //printf("ret %s \n", (ret+2));
        bm->RBCestimation.RBCspeciesArray[species].MaxConvergCrit = atof((ret+2)); // +2 to skip the ' " ' and the space.
        //printf("Max Converge %f %f\n", bm->RBCestimation.RBCspeciesArray[species].MaxConvergCrit, atof((ret+2)));
    }

    // Only continue if all is when
    if (bm->RBCestimation.RBCspeciesArray[species].MaxConvergCrit > 100.1) {  // RateMSE code found had to use 0.1 not 0.01
        fprintf(llogfp, "Tier 4 assessment failed due to %s MaxConvergCrit: %e\n", FunctGroupArray[species].groupCode, bm->RBCestimation.RBCspeciesArray[species].MaxConvergCrit);
        bm->RBCestimation.RBCspeciesParam[species][AssessFail_id] = TRUE;
        bm->RBCestimation.RBCspeciesArray[species].RBC_by_year[year] = 0.0;              // temporary value, will be reset in DoAssessment
        bm->RBCestimation.RBCspeciesParam[species][EstB0_id] = 999.9;
        bm->RBCestimation.RBCspeciesParam[species][EstBinit_id] = 999.9;
        bm->RBCestimation.RBCspeciesParam[species][EstBcurr_id] = 999.9;
    } else {
        //MSY
        
        fseek(fp, 0, SEEK_SET); // Move the file pointer to the start.
        count = 0;
        line_size = getline(&str, &line_buf_size, fp);
        while (strncmp(str,"\"MSY",4) !=0 && count < 1000) {
            count++;
            //printf("%d %s\n", count, str);
            line_size = getline(&str, &line_buf_size, fp);
        }
        if (count >= 1000) {
            quit("setRBC: Can't find MSY in %s output file\n", outname);
        } else {
            printf("str **** %s \n", str);
            ret = strrchr(str, '\"'); // get everything after last  ' " '
            bm->RBCestimation.RBCspeciesArray[species].MSY = atof((ret+2)); // +2 to skip the ' " ' and the space.
            //printf("MSY %f %f\n", bm->RBCestimation.RBCspeciesArray[species].MSY, atof((ret+2)));
        }

        // k
        fseek(fp, 0, SEEK_SET); // Move the file pointer to the start.
        count = 0;
        line_size = getline(&str, &line_buf_size, fp);
        while (strncmp(str,"\"k",2) !=0 && count < 1000) {
            count++;
            line_size = getline(&str, &line_buf_size, fp);
        }
        if (count >= 1000) {
            quit("setRBC: Can't find k in %s output file\n", outname);
        } else {
            printf("str **** %s \n", str);
            ret = strrchr(str, '\"'); // get everything after last  ' " '
            bm->RBCestimation.RBCspeciesParam[species][EstB0_id] = atof((ret+2)); // +2 to skip the ' " ' and the space.
            //printf("B0 %f %f\n",bm->RBCestimation.RBCspeciesParam[species][EstB0_id], atof((ret+2)));
        }

        // Binit
        fseek(fp, 0, SEEK_SET); // Move the file pointer to the start.
        count = 0;
        line_size = getline(&str, &line_buf_size, fp);
        while (strncmp(str,"\"Binit",6) !=0 && count < 1000) {
            count++;
            line_size = getline(&str, &line_buf_size, fp);
        }
        if (count >= 1000) {
            quit("setRBC: Can't find Binit in %s output file\n", outname);
        } else {
            printf("str **** %s \n", str);
            ret = strrchr(str, '\"'); // get everything after last  ' " '
            bm->RBCestimation.RBCspeciesParam[species][EstBinit_id] = atof((ret+2)); // +2 to skip the ' " ' and the space.
            //printf("Binit %f %f\n", bm->RBCestimation.RBCspeciesParam[species][EstBinit_id], atof((ret+2)));
        }

        // Bcurr
        fseek(fp, 0, SEEK_SET); // Move the file pointer to the start.
        count = 0;
        line_size = getline(&str, &line_buf_size, fp);
        while (strncmp(str,"\"Bcurr",6) !=0 && count < 1000) {
            count++;
            line_size = getline(&str, &line_buf_size, fp);
        }
        if (count >= 1000) {
            quit("setRBC: Can't find Bcurr in %s output file\n", outname);
        } else {
            printf("str **** %s \n", str);
            ret = strrchr(str, '\"'); // get everything after last  ' " '
            bm->RBCestimation.RBCspeciesParam[species][EstBcurr_id] = atof((ret+2)); // +2 to skip the ' " ' and the space.
            //printf("Bcurr %f %f\n", bm->RBCestimation.RBCspeciesArray[species].EstBcurr, atof((ret+2)));
        }

        // RBC
        fseek(fp, 0, SEEK_SET); // Move the file pointer to the start.
        count = 0;
        line_size = getline(&str, &line_buf_size, fp);
        while (strncmp(str,"\"RBC",4) !=0 && count < 1000) {
            count++;
            line_size = getline(&str, &line_buf_size, fp);
        }
        if (count >= 1000) {
            quit("setRBC: Can't find RBC in %s output file\n", outname);
        } else {
            printf("str **** %s \n", str);
            ret = strrchr(str, '\"'); // get everything after last  ' " '
            printf("%s\n", str);
            // Nov 2026: read the RBC (was hard set to 0). Only the first projection year is used as multi-year
            // TACs are handled by the Atlantis rollover code
            d0 = atof((ret+2)); // +2 to skip the ' " ' and the space.
            bm->RBCestimation.RBCspeciesArray[species].RBC_by_year[year] = d0;
            bm->RBCestimation.RBCspeciesParam[species][RBCest_id] = d0;
        }

        /* Nov 2026 (review item 27): if using Ratpack style RBC averaging read the projected RBCs
           (the "Catch projection" block written by runDynTier4onRTMB.R, one value per line) */
        if (((int)(bm->RBCestimation.RBCspeciesParam[species][UseRBCAveraging_id]) == ratpack_RBC_avg) && (bm->RBCestimation.nRBCaverage > 1)) {
            int nproj_max = bm->RBCestimation.nRBCaverage;
            int nproj = 0;
            char *endp;

            if (nproj_max > MAX_RBC_PROJ)
                nproj_max = MAX_RBC_PROJ;

            fseek(fp, 0, SEEK_SET);
            count = 0;
            line_size = getline(&str, &line_buf_size, fp);
            while ((line_size > 0) && (strstr(str, "Catch projection") == NULL) && (count < 1000)) {
                count++;
                line_size = getline(&str, &line_buf_size, fp);
            }
            if ((line_size > 0) && (count < 1000)) {
                while ((nproj < nproj_max) && ((line_size = getline(&str, &line_buf_size, fp)) > 0)) {
                    d0 = strtod(str, &endp);
                    if (endp == str)
                        break;   // Not a number so end of the projection block
                    bm->RBCestimation.RBCspeciesArray[species].RBC_proj[nproj] = d0;
                    nproj++;
                }
            } else {
                fprintf(llogfp, "Time %e %s WARNING - setRBC: no Catch projection in %s so no Ratpack style RBC averaging this year\n", bm->dayt, FunctGroupArray[species].groupCode, outname);
            }
            bm->RBCestimation.RBCspeciesArray[species].nRBC_proj = nproj;
        }
    }
    fclose(fp);
    
    if ((bm->RBCestimation.RBCspeciesArray[species].RBC_by_year[year] < 0.0) || (bm->RBCestimation.RBCspeciesArray[species].RBC_by_year[year] > 999999.9)) {
        fprintf(llogfp, "Time %e %s SS assessment failed due to RBC = %e\n", bm->dayt, FunctGroupArray[species].groupCode, bm->RBCestimation.RBCspeciesArray[species].RBC_by_year[year]);

        bm->RBCestimation.RBCspeciesParam[species][AssessFail_id] = TRUE;
        bm->RBCestimation.RBCspeciesArray[species].RBC_by_year[year] = 0.0;               // temporary value, will be reset in DoAssessment        bm->RBCestimation.RBCspeciesParam[species][EstB0_id] = 999.9;
        bm->RBCestimation.RBCspeciesParam[species][EstBinit_id] = 999.9;
        bm->RBCestimation.RBCspeciesParam[species][EstBcurr_id] = 999.9;
    }

    if (bm->RBCestimation.RBCspeciesParam[species][AssessFail_id] == TRUE) {
        bm->RBCestimation.RBCspeciesArray[species].NassessFail++;
    } else {
        bm->RBCestimation.RBCspeciesArray[species].NassessFail = 0;
    }

    bm->RBCestimation.RBCspeciesParam[species][EstDepletion_id] = bm->RBCestimation.RBCspeciesParam[species][EstBcurr_id] / bm->RBCestimation.RBCspeciesParam[species][EstB0_id];

    //fprintf(llogfp, "Time %e %s dataYear: %d EstDepletion: %e RBC_by_year: %e\n", bm->dayt, FunctGroupArray[species].groupCode, bm->RBCestimation.RBCspeciesArray[species].mgt_dataYear, bm->RBCestimation.RBCspeciesParam[species][EstDepletion_id], bm->RBCestimation.RBCspeciesArray[species].RBC_by_year[year]);
    
    return;

}

/* Needed if using ratpack style RBC averaging over years */
void Averaged_RBC(MSEBoxModel *bm, int species, int year, FILE *llogfp) {
    int yr, startyr, nyr = 0;
    double AveRBC = 0;

    /* Nov 2026: average the most recent nRBCaverage RBCs ending with the current assessment.
       Previously summed nRBCaverage-1 values starting at the (never set) mgt_rbcYear, divided by
       nRBCaverage and wrote to RBC_by_year[year] on every loop pass. */

    // Store the current RBC so it is included in the average
    bm->RBCestimation.RBCspeciesArray[species].RBC_by_year[year] = bm->RBCestimation.RBCspeciesParam[species][RBCest_id];

    startyr = year - bm->RBCestimation.nRBCaverage + 1;
    if (startyr < 0)
        startyr = 0;

    // Calculate the average RBC
    for (yr = startyr; yr < year + 1; yr++) {
        AveRBC += bm->RBCestimation.RBCspeciesArray[species].RBC_by_year[yr];
        nyr++;
    }
    if (nyr > 0)
        AveRBC /= (double)nyr;

    // Replace the RBC
    bm->RBCestimation.RBCspeciesArray[species].RBC_by_year[year] = AveRBC;
    bm->RBCestimation.RBCspeciesParam[species][RBCest_id] = AveRBC;

    fprintf(llogfp, "Time: %e (Year: %d) %s RBC averaged over %d years (%d to %d): %e\n", bm->dayt, year, FunctGroupArray[species].groupCode, nyr, startyr, year, AveRBC);

    return;
}

/* Nov 2026 (review item 27): Ratpack style RBC averaging (UseRBCAveraging = 2), as in ratpackmse Assessment.cpp
   DoAssessment. The RBC is replaced by the average of the first nRBCaverage years of this assessment's projected
   RBCs (RBC_proj, index 0 = first year the RBC applies). Only dynamic Tier 4 (tiertypes 1 and 5) and SS3 give a
   projection. Other tiers give a single RBC, which Ratpack would treat as a constant projection, so it is unchanged.
   Ratpack uses the average for each of the nRBCaverage years; in Atlantis the next assessment (or the multi-year TAC
   rollover) takes over from there. */
void Ratpack_Averaged_RBC(MSEBoxModel *bm, int species, int year, FILE *llogfp) {
    int yr, nyr = bm->RBCestimation.RBCspeciesArray[species].nRBC_proj;
    double AveRBC = 0;

    if (bm->RBCestimation.RBCspeciesParam[species][AssessFail_id])
        return;
    if (nyr < 1)
        return;   // Single RBC from this tier, so nothing to average
    if (nyr > bm->RBCestimation.nRBCaverage)
        nyr = bm->RBCestimation.nRBCaverage;
    if (nyr < 1)
        return;

    for (yr = 0; yr < nyr; yr++)
        AveRBC += bm->RBCestimation.RBCspeciesArray[species].RBC_proj[yr];
    AveRBC /= (double)nyr;

    if (nyr < bm->RBCestimation.nRBCaverage) {
        fprintf(llogfp, "Time: %e (Year: %d) %s WARNING - only %d projected RBCs available for Ratpack RBC averaging (nRBCaverage %d)\n",
                bm->dayt, year, FunctGroupArray[species].groupCode, nyr, bm->RBCestimation.nRBCaverage);
    }

    // Replace the RBC
    bm->RBCestimation.RBCspeciesArray[species].RBC_by_year[year] = AveRBC;
    bm->RBCestimation.RBCspeciesParam[species][RBCest_id] = AveRBC;

    fprintf(llogfp, "Time: %e (Year: %d) %s Ratpack RBC averaging over the first %d projected years: %e\n", bm->dayt, year, FunctGroupArray[species].groupCode, nyr, AveRBC);

    return;
}
