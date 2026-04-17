/**
 \file atManageAnnual.c
 \brief Functions that are performed annually.
 \ingroup atManageLib
 \author Beth Fulton 5/10/2005

 <b>Revisions</b>
 16/6/2006 Added 	Report_Annual_Fisheries_Stats(), Report_Fisheries_Stats(),
 Harvest_Update_Index_Values(), Update_Management_Indx() and Output_Management_Indices(bm)
 for reporting aggregate fisheries statistics. Also expanded Annual_Fisheries_Mgmt()
 and Apply_Annual_Fisheries_Mgmt() to set up, execute or faciltate
 these new routines. Also added Make_Mgmt_Decisions() to handle dynamic TAC setting.

 1/7/2006 Added bm->FCtsCarryOver so can correct for any immediate shortfalls
 when applying timeseries of catch (needed as most time series are broken
 down from annual statistics and so day-to-day shortfalls or excesses can be created).

 8/8/2006 Removed extra Effort[b][nf] from annual effort adjusting calculations
 (as already had *= by having *Effort[b][nf] on the end was effectively
 squaring effort!)

 27/2/2007 Fixed the back-to-front companion TAC calculation code and added the
 flagdyn_coupdate option

 15-04-2008 Beth Fulton
 In Apply_Annual_Fisheries_Mgmt, for the in_quota check corrected id misread from nf to fishery_id

 26-05-2008 Bec Gorton
 Changed all references from bm->VERTind to the tracer arrays associated with
 each functional group.

 22-04-2009 Bec Gorton
 Got rid of the bm->SPid array. The isImpacted variable in the FunctGroupArray is not used to determine
 if a group is impacted by fishing.

 16/6/2009 Beth Fulton
 Replaced SP_of_Concern[sp] with FunctGroupArray[sp].speciesParams[sp_concern_id]

 28-10-2009 Bec Gorton
 Removed references to the K_num_impacted_sp and K_num_fished_sp variables.

 28-01-2010 Bec Gorton
 Renamed some functions to comply with the coding standard. Also remove all default variable initialisation to 0.
 Is a variable is used an not initialised (properly) then we want to know - valgrind will find.

 26-04-2010 Bec Gorton
 Fixed a couple of bugs in the Check_F_Harvest_Control_Rule code. Also added extensive
 debugging output for Penny. Will removed once she is happy with the output.


 05-05-2010 Bec Gorton
 Changed the broken-stick management strategy to store the current mFC scalar (in the new mFC_scale_id slot) instead of directly
 applying it to the actual mFC value. Directly applying it mean't that when the stock recovered fishing
 never resumed as it was trying to scale up the mFC value which was 0.

 03-10-2011 Beth Fulton
 Added OldCatchSum[sp], TotOldCumCatch[sp][n], TotOldCumDiscards[sp][n],
 tac_resetcount_id and tac_resetperiod_id to handle multi-year TAC setting


 */

/*  Open library routines */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <math.h>
#include <sjwlib.h>
#include "atManage.h"
#include "atHarvestLib.h"
#include "atAssessLib.h"

#ifdef RASSESS_LINK_ENABLED
    #include "atRlink.h"
#endif

/* Static prototypes delarations of private functions */

static void AMS_Tiered_Assessment(MSEBoxModel *bm, int sp, FILE *llogfp);
static void Do_MSY_Management(MSEBoxModel *bm, FILE *llogfp);
static void Annual_Effort_Scale(MSEBoxModel *bm, FILE *llogfp);
static void Make_Mgmt_Decisions(MSEBoxModel *bm, FILE *llogfp);
static void Check_Ecosystem_F_Harvest_Control_Rule(MSEBoxModel *bm, FILE *llogfp);
static void Check_F_Harvest_Control_Rule(MSEBoxModel *bm, FILE *llogfp);
static void Per_Sp_Frescale (MSEBoxModel *bm, FILE *llogfp, int sp);
static void Guild_Frescale (MSEBoxModel *bm, FILE *llogfp, int sp);
static void Ecosystem_Cap_Frescale(MSEBoxModel *bm, FILE *llogfp);

#ifdef RASSESS_LINK_ENABLED
    static void Check_R_management(MSEBoxModel *bm, FILE *llogfp);
#endif

/**
 *	\brief Annual management model
 */
void Manage_Annual_Calculations(MSEBoxModel *bm, FILE *llogfp) {

	/* Make management decisions */
	Make_Mgmt_Decisions(bm, llogfp);

	/* Update and output management peformance measures */
//	Update_Management_Indx(bm, llogfp);
//	Output_Management_Indices(bm);
//	Annual_Effort_Scale(bm, llogfp);

	return;
}

void Manage_Update_Indices(MSEBoxModel *bm, FILE *llogfp) {

	/* Update and output management peformance measures */
	Update_Management_Indx(bm, llogfp);
	Manage_Output_Indices(bm);
	Annual_Effort_Scale(bm, llogfp);
	return;
}
/**
 * \brief Rescale effort based on past years CPUE
 *
 *
 */
void Annual_Effort_Scale(MSEBoxModel *bm, FILE *llogfp) {

	int b, nf, flagspeffortmodel, sp;
	double effort_scale, toteffort, totcatch, ann_CPUE, newtoteffort, reset_yrs, lastEffort;

	/* If appropriate rescale effort based on past years CPUE */
	for (nf = 0; nf < bm->K_num_fisheries; nf++) {
		flagspeffortmodel = (int) (bm->FISHERYprms[nf][flageffortmodel_id]);
		effort_scale = bm->FISHERYprms[nf][CPUE_effort_scale_id];

		if ((flagspeffortmodel == alt_cpue_effort) || (flagspeffortmodel == alt_cpue_scale_effort)) {
			/* Get annual CPUE */
			totcatch = 0;
			for (sp = 0; sp < bm->K_num_tot_sp; sp++) {
				if (FunctGroupArray[sp].isFished == TRUE) {
					if (bm->FISHERYtarget[nf][sp]) {
						totcatch += Harvest_Get_TotCumCatch(sp, nf, bm->thisyear);
					}
				}
			}

			toteffort = 0;
			for (b = 0; b < bm->nbox; b++) {
				toteffort += bm->CumEffort[nf][b];
			}
			bm->TotOldCumEffort[nf] = toteffort;

			/* Convert catch to tonnes (effort already in days) */
			totcatch *= (bm->X_CN * mg_2_tonne);
			ann_CPUE = totcatch / (toteffort + small_num);

			/* Adjust effort based on CPUE - scale up or down and then dependence on
			 totOldEffort will see it filter through the fishery
			 */
			if (ann_CPUE > bm->FISHERYprms[nf][CPUE_effort_thresh_id]) {
				bm->totOldEffort[nf] *= (1.0 + effort_scale);
				for (b = 0; b < bm->nbox; b++) {
					bm->Effort[b][nf] *= (1.0 + effort_scale);
				}
				if ((bm->debug == debug_effort) && (bm->which_fleet == nf)) {
					fprintf(llogfp, "Time: %e, %s totOldEffort scaled by %e (effort_scale = %e)\n", bm->dayt, FisheryArray[nf].fisheryCode,
							(1.0 + effort_scale), effort_scale);
				}

			} else {
				bm->totOldEffort[nf] *= (1.0 - effort_scale / 2.0);
				for (b = 0; b < bm->nbox; b++) {
					bm->Effort[b][nf] *= (1.0 - effort_scale / 2.0);
				}
				if ((bm->debug == debug_effort) && (bm->which_fleet == nf)) {
					fprintf(llogfp, "Time: %e, %s totOldEffort scaled down by %e (effort_scale / 2 = %e)\n", bm->dayt, FisheryArray[nf].fisheryCode, (1.0
							- effort_scale / 2.0), effort_scale / 2.0);
				}

			}

			/* Check effort change isn't too radical - especially for use with pseudo assessment code */
			newtoteffort = 0;
			for (b = 0; b < bm->nbox; b++) {
				newtoteffort += bm->Effort[b][nf] * 365.0 * (86400.0 / bm->dt);
				/* Need to multiply by 365 to get daily rate to approximate annual rate */
			}

			if (fabs(1 - (newtoteffort / ((1.0 - effort_scale) * toteffort))) > buffer_rounding) {
				for (b = 0; b < bm->nbox; b++) {
					lastEffort = bm->Effort[b][nf];
					bm->Effort[b][nf] *= (((1.0 - effort_scale) * toteffort) / (newtoteffort + small_num));

					if ((bm->debug == debug_effort) && (bm->which_fleet == nf)) {
						fprintf(llogfp, "Time: %e  %s in %d newEffort = %e (effort_scale: %e, toteffort: %e, newtoteffort: %e, lastEffort: %e)\n", bm->dayt,
								FisheryArray[nf].fisheryCode, b, bm->Effort[b][nf], effort_scale, toteffort, newtoteffort, lastEffort);
					}

				}
			}

			/* Double check result of rescaling */
			newtoteffort = 0;
			for (b = 0; b < bm->nbox; b++) {
				newtoteffort += bm->Effort[b][nf] * 365.0 * (86400.0 / bm->dt);
			}
			if (fabs(1 - (newtoteffort / ((1.0 - effort_scale) * toteffort))) > buffer_rounding) {
				fprintf(llogfp,
						"Time: %e - Something is wrong in annual effort adjustment for %s as oldeffort: %e, neweffort: %e but expecting %e so resetting\n",
						bm->dayt, FisheryArray[nf].fisheryCode, toteffort, newtoteffort, ((1.0 - effort_scale) * toteffort));
				reset_yrs = bm->FISHERYprms[nf][reset_id];
				for (b = 0; b < bm->nbox; b++) {
					/* Need to use power representation of multiplicative decreases as if just multply decrease by reset_yrs end up with an increase */
					bm->Effort[b][nf] = bm->Effort_hdistrib[b][nf][bm->QofY] * bm->FISHERYprms[nf][EffortLevel_id] * pow((1.0 - effort_scale), reset_yrs);

					if ((bm->debug == debug_effort) && (bm->which_fleet == nf)) {
						fprintf(llogfp, "Time: %e  %s in %d newEffort = %e (hdistrib: %e, EffortLevel: %e, effort_scale: %e, reset_yrs: %e)\n", bm->dayt,
								FisheryArray[nf].fisheryCode, b, bm->Effort[b][nf], bm->Effort_hdistrib[b][nf][bm->QofY], bm->FISHERYprms[nf][EffortLevel_id],
								effort_scale, reset_yrs);
					}

				}
				bm->FISHERYprms[nf][reset_id]++;
			}

			if (bm->totOldEffort[nf] < 0)
				bm->totOldEffort[nf] = 0;

		}
	}
}
/**
 *	\brief This routine implements any formal decision rules to reach final management decisions
 *	(e.g. sets TACs here, but implemented in Apply_Annual_Fisheries_Mgmt()
 *
 *	FIX -- once know decision rules to implement, give option of using assessments not perfect
 *	knowledge in management
 */
void Make_Mgmt_Decisions(MSEBoxModel *bm, FILE *llogfp) {
	int sp, nf, flag_sp, co_sp, co_sp2, co_TYPE, dont_scale2, do_assessing = 0, flagrecfish, in_quota, bim;
	double totTAC, FC2_ratio, spA_TAC, spC_TAC = 0, expect_spC = 0, avg_DAS, num_DAS;
	double spTotCumCatch, co_sp2_TotCumCatch;
	int year = (int)ceil(bm->dayt / 365);
    
    fprintf(llogfp,"Time: %e year %d\n", bm->dayt, year);

	if (verbose)
		printf("Making management decisions\n");

#ifdef RASSESS_LINK_ENABLED
	Check_R_management(bm, llogfp);
#endif
    
    /* If doing pseudo assessments go do them now */
    do_assessing = 1;
    if (bm->pseudo_assess || bm->useRBCTiers || bm->do_TACassessing) {
        do_assessing = 1;
    } else if (!bm->do_TACassessing) {  // used to be "else if (!bm->do_assessing)" but can have the case where want science collection but not TAC setting
        do_assessing = 0;
    }
    
    // Do not bother executing these in the first year
    if ((bm->thisyear > 0) || (!do_assessing)){
        Check_F_Harvest_Control_Rule(bm, llogfp);
        Check_Ecosystem_F_Harvest_Control_Rule(bm, llogfp);
    } else {
        // Make sure mFC_scale_id is set to 1.0
        for (sp = 0; sp < bm->K_num_tot_sp; sp++) {
            for (nf = 0; nf < bm->K_num_fisheries; nf++) {
                bm->SP_FISHERYprms[sp][nf][mFC_scale_id] = 1.0;
            }
        }
    }

	/* If not using assessment model then don't bother with this step */
	if (!do_assessing) {
		return;
	}

	/* If the first timestep and initial conditions still apply
	 for the TACs then don't bother with this step
	 */
	if (bm->dayt < 1) {
		Report_TACs(bm, llogfp);
		return;
	}

	/* Initialise TAC flags */
	for (nf = 0; nf < bm->K_num_fisheries; nf++) {
		bm->TAC_trigger[nf][triggered_scalar_id] = 1.0;
	}

	/* Do assessments */
	for (sp = 0; sp < bm->K_num_tot_sp; sp++) {
        // Under single species assessments - for single species only or PGMSY
        if((bm->UsingRAssess == 1) && (FunctGroupArray[sp].isTAC > 1)) {
#ifdef RASSESS_LINK_ENABLED
            Do_RAssess(bm, sp, year, llogfp);
#else
            quit("How get here as R link not active so set UsingRAssess to 0\n");
#endif
        } else if(bm->useRBCTiers) {
            CallTierAssessment(bm, sp, year, llogfp);
        } else {
            AMS_Tiered_Assessment(bm, sp, llogfp);
        }
	}
    
    // Allow for F based assessment rule her etoo, judt in case, especiallyfor Norway
    if ((bm->thisyear > 0) && do_assessing){
        Check_F_Harvest_Control_Rule(bm, llogfp);
        Check_Ecosystem_F_Harvest_Control_Rule(bm, llogfp);
    }

    fprintf(bm->logFile, "Time: %e year: %d Getting read to do DoMultiStockAssessment as useMultispAssess: %d\n", bm->dayt, year, bm->useMultispAssess);

    /* Do multispecies assessments if needed */
    if (bm->useMultispAssess) {
        if (bm->PGMSY_on) {
            Call_PGMSY(bm, year, llogfp);
        }
        DoMultiStockAssessment(bm, year, llogfp);
    }

	/* Set Companion TACs */
	for (sp = 0; sp < bm->K_num_tot_sp; sp++) {
		if (FunctGroupArray[sp].isFished == TRUE) {

            /* If only resetting Fs skip ahead now */
			if (FunctGroupArray[sp].speciesParams[flagFonly_id])
				continue;
            
            co_TYPE = (int) (FunctGroupArray[sp].speciesParams[coType_id]);
            spTotCumCatch = Harvest_Get_TotCumCatch(sp, nf, bm->thisyear);

            for (co_sp = 0; co_sp < FunctGroupArray[sp].speciesParams[max_co_sp_id]; co_sp++ ) {
                co_sp2 = FunctGroupArray[sp].co_sp[co_sp];
            
                /* If neither of the companion groups is fished then do nothing */
                if ((co_sp2 == -1 || FunctGroupArray[co_sp2].isFished == FALSE))
                    continue;

                if (co_sp2 > 0) {
                    fprintf(llogfp, "Time: %e, looking at %s vs co_sp: %s, co_sp2: %s, flagdyn_coupdate: %d\n", bm->dayt, FunctGroupArray[sp].groupCode, FunctGroupArray[co_sp].groupCode, FunctGroupArray[co_sp2].groupCode, bm->flagdyn_coupdate);
                } else {
                    fprintf(llogfp, "Time: %e, looking at %s vs co_sp: %s, flagdyn_coupdate: %d\n", bm->dayt, FunctGroupArray[sp].groupCode, FunctGroupArray[co_sp].groupCode, bm->flagdyn_coupdate);
                }

                for (nf = 0; nf < bm->K_num_fisheries; nf++) {

                    /* Only change management in fisheries where management active */
                    if (bm->FISHERYprms[nf][manage_on_id] < 1)
                        continue;

                    co_sp2_TotCumCatch = 0.0;
                    if (bm->flagdyn_coupdate) {
                        /* Update catch ratios */
                        if (co_sp2 != -1 && FunctGroupArray[co_sp2].isFished == TRUE) {
                            co_sp2_TotCumCatch = Harvest_Get_TotCumCatch(co_sp2, nf, bm->thisyear);
                            FC2_ratio = co_sp2_TotCumCatch / (spTotCumCatch + small_num);
                        } else {
                            FC2_ratio = 1.0;
                        }
                    } else {
                        /* Always use initial (read-in) catch ratios */
                        FC2_ratio = FunctGroupArray[sp].co_sp_catch[nf][co_sp2];
                    }
                    spA_TAC = bm->TACamt[sp][nf][now_id];

                    /* Cope with zero catches when quota still available */
                    if ((spTotCumCatch == 0) && spA_TAC) {
                        FC2_ratio = max(1.0, FunctGroupArray[sp].co_sp_catch[nf][co_sp2]);
                    }

                    if (co_sp2 != -1 && FunctGroupArray[co_sp2].isFished == TRUE) {
                        spC_TAC = bm->TACamt[co_sp2][nf][now_id];
                        expect_spC = FC2_ratio * spA_TAC; // What expected TAC of companion to A should be
                        dont_scale2 = 0;

                        /* For those cases where there is no actual catch despite wanting quota don't play with the TACs */
                        if (!co_sp2_TotCumCatch && spC_TAC) {
                            dont_scale2 = 1;
                        }
                    } else {
                        dont_scale2 = 1;
                    }

                    /* Reset TACs dependent on TAC of companion group */
                    switch (co_TYPE) {
                        case Weakest_Link: /* Set TAC based on weakest link in the pair */
                            if (!dont_scale2 && (spC_TAC > expect_spC)) {
                                bm->TACamt[co_sp2][nf][now_id] = expect_spC;
                                bm->TAC_trigger[nf][triggered_scalar_id] *= expect_spC / (spC_TAC + small_num);

                                fprintf(llogfp, "Time: %e, TAC for %s in %s was changed by %e (WLtrig_scalar: %e, expect_spB: %e, spC_Tac: %e) to %e\n", bm->dayt, FunctGroupArray[co_sp2].groupCode, FisheryArray[nf].fisheryCode, expect_spC / (spC_TAC + small_num), bm->TAC_trigger[nf][triggered_scalar_id], expect_spC, spC_TAC, bm->TACamt[co_sp2][nf][now_id]);
                            }
                            break;
                        case Strongest_link: /* Set TAC based on strongest link in the pair */
                            if (!dont_scale2 && (spC_TAC < expect_spC)) {
                                bm->TACamt[co_sp2][nf][now_id] = expect_spC;
                                bm->TAC_trigger[nf][triggered_scalar_id] *= expect_spC / (spC_TAC + small_num);

                                fprintf(llogfp, "Time: %e, TAC for %s in %s was changed by %e (SLtrig_scalar: %e, expect_spB: %e, spC_Tac: %e) to %e\n", bm->dayt, FunctGroupArray[co_sp2].groupCode, FisheryArray[nf].fisheryCode, expect_spC / (spC_TAC + small_num), bm->TAC_trigger[nf][triggered_scalar_id], expect_spC, spC_TAC, bm->TACamt[co_sp2][nf][now_id]);
                            }
                            break;
                        default:
                            quit("No such companion TAC option as yet. Must chose either weakest (0) or strongest (1) link\n");
                            break;
                    }

                    /* Output simple list of new TACs */
                    /*
                     if (!dont_scale2) {
                        fprintf(llogfp, "Time: %e, sp %s in %s coTAC = %e (%s-TAC: %e)\n", bm->dayt, FunctGroupArray[sp].groupCode, FisheryArray[nf].fisheryCode, bm->TACamt[sp][nf][now_id], FunctGroupArray[co_sp2].groupCode, bm->TACamt[co_sp2][nf][now_id]);
                     }
                     */
                }
			}
		}
	}

	/* Set for Dan Holland-like Bimonthly trip limits */
	if (!bm->MultiPlanEffort) {
		for (sp = 0; sp < bm->K_num_tot_sp; sp++) {
			if (FunctGroupArray[sp].isFished == TRUE) {

				/* If only resetting Fs skip ahead now */
				if (FunctGroupArray[sp].speciesParams[flagFonly_id])
					continue;

				for (nf = 0; nf < bm->K_num_fisheries; nf++) {

					/* Only change management in fisheries where management active */
					if (bm->FISHERYprms[nf][manage_on_id] < 1)
						continue;

					for (bim = 0; bim < 6; bim++) {
						/* Store last values */
						bm->BiTACamt[bim][sp][nf][old_id] = bm->BiTACamt[bim][sp][nf][now_id];

						/* Update with new values - use fixed read-in distribution as old could be reset to zero by dynamic management,
						 but need reset every year here.
						 */
						bm->BiTACamt[bim][sp][nf][now_id] = bm->BiTAC_sp[bim][totalreg_id][sp][old_id] * bm->TACamt[sp][nf][now_id];
					}
				}
			}
		}
	}

	/** Set Regional TAC **/

	/** Temporal management changes **/
	/* If on then handled in Apply_Annual_Fisheries_Mgmt()
	 by scaling the season by the same scalar as stored
	 in effort_scale.

	 TODO - In Future may want direct seasonal decisions made here.
	 */


	/* TODO - Put in decision model for more dynamic alternative management changes */
	/** Discards changed **/
	/** Gear changed - selectivity and swept area **/
	/** Spatial management changes **/

	/** Update Management stability index and give TAC messages **/
	/* Report changes in TACs */
	for (nf = 0; nf < bm->K_num_fisheries; nf++) {
		manageindx[nf][mgmtstability_id]++;

		/* Only change management in fisheries where management active */
		if (bm->FISHERYprms[nf][manage_on_id] < 1)
			continue;

		if (bm->TAC_trigger[nf][triggered_scalar_id] < 1.0) {
			fprintf(llogfp, "Time: %e, fishery %s has at least one TAC reduction of %e\n", bm->dayt, FisheryArray[nf].fisheryCode,
					bm->TAC_trigger[nf][triggered_scalar_id]);
			manageindx[nf][mgmtstability_id] = 0;
		} else if (bm->TAC_trigger[nf][triggered_scalar_id] > 1.0) {
			fprintf(llogfp, "Time: %e, fishery %s has at least one TAC increase  of %e\n", bm->dayt, FisheryArray[nf].fisheryCode,
					bm->TAC_trigger[nf][triggered_scalar_id]);
			manageindx[nf][mgmtstability_id] = 0;
		}
	}

	/* Check TAC values */
	//if(bm->debug == debug_assess){
	for (sp = 0; sp < bm->K_num_tot_sp; sp++) {
		if (FunctGroupArray[sp].isFished == TRUE) {

			/* If only resetting Fs skip ahead now */
			if (FunctGroupArray[sp].speciesParams[flagFonly_id])
				continue;

			flag_sp = (int) (FunctGroupArray[sp].speciesParams[flag_id]);
			if (flag_sp) {
				totTAC = 0;
				fprintf(llogfp, "Time: %e %s", bm->dayt, FunctGroupArray[sp].groupCode);
				for (nf = 0; nf < bm->K_num_fisheries; nf++) {
					flagrecfish = (int) (bm->FISHERYprms[nf][flagrecfish_id]);
					in_quota = bm->inQuota[nf][sp];
					if (!flagrecfish && in_quota) {
						/* Don't include recfishing in quota allocation for now (as often not quota-ed)
						 FIX - may have to change this if recfishing becomes quota allocated group
						 */
                        if(!FunctGroupArray[sp].isTAC || (bm->TACamt[sp][nf][now_id] < no_quota)) {
                            totTAC += bm->TACamt[sp][nf][now_id];
                        }
						fprintf(llogfp, " %s=%e", FisheryArray[nf].fisheryCode, bm->TACamt[sp][nf][now_id]);
					}
				}
				if (totTAC < 0)
					totTAC = 0;
				fprintf(llogfp, " TAC = %f (%e)", totTAC / 1000.0, totTAC / 1000.0); /* Convert kg to tonnes */
			}
			fprintf(llogfp, "\n");
		}
	}
	//}

	/* Report new TACs */
	Report_TACs(bm, llogfp);

	/* Reset Days-At-Sea caps */
	if (bm->DynDAS || bm->UsingEffortIncentive) {
		for (nf = 0; nf < bm->K_num_fisheries; nf++) {

			/* Only change management in fisheries where management active */
			if (bm->FISHERYprms[nf][manage_on_id] < 1)
				continue;

			avg_DAS = 0.0;
			num_DAS = 0.0;
			for (sp = 0; sp < bm->K_num_tot_sp; sp++) {
				if (FunctGroupArray[sp].isFished == TRUE) {
					if (FunctGroupArray[sp].speciesParams[sp_concern_id]) {
						avg_DAS += bm->DAScalc[sp];
						num_DAS++;
					}
				}
			}

			/* Rescale effort cap based on average ratio of RBC and last year's catch (across species) */
			if (num_DAS > 0) {
				avg_DAS /= num_DAS;
				bm->FISHERYprms[nf][cap_id] *= avg_DAS;
			} else {
				avg_DAS = 1.0;
			}
		}
	}

	Do_MSY_Management(bm, llogfp);

	return;

}

/**************************************************************************//**
 * AMS means of handling a tierd assessment
 *
 *****************************************************************************/
void AMS_Tiered_Assessment(MSEBoxModel *bm, int sp, FILE *llogfp) {
	int cue_spatial_closure, assess_flag_sp, sp_basket_mem, tier, use_lever, nf, yr_id, flagrecfish, in_quota;
	double BrefA, BrefB, BrefC, BrefD, Bcurr, FrefA, Fcurr, M, Ccurr, Cslope, Num_yr, Blim, FrefC, RBC_scale, TACorig, FTARG, RBC,
		CPUE_ratio, CPUE_now, CPUE_start, K_est, r_est, FC_restrict, Fratio, totcatch, totTAC, RBC_alloc, TAC_prop, olddiff, newdiff,
		TAC_scale, end_date, start_date, period;

	if (FunctGroupArray[sp].isFished == TRUE) {
		cue_spatial_closure = 0;
		bm->DAScalc[sp] = 0;

		assess_flag_sp = (int) (FunctGroupArray[sp].speciesParams[assess_flag_id]);
		if (!assess_flag_sp)
			return;

		/* Set Basket TACs - No changes at present so skip to next species */
		/* FIX - in future make it dynamic too */
		sp_basket_mem = (int) (FunctGroupArray[sp].speciesParams[basketSP_id]);
		if (sp_basket_mem)
			return;

		/* If using pseudo assessment then can't change value of invert TACs
		 FIX - Make this more flexible!
		 */
		if (bm->pseudo_assess) {
            if (FunctGroupArray[sp].isVertebrate == TRUE) {
                Assess_Pseudo_Estimate_Prm(bm, sp, llogfp);
            } else {
                return;
            }
		}

		/* If only resetting Fs skip ahead now */
        if (FunctGroupArray[sp].speciesParams[flagFonly_id]) {
            return;
        }

		/* If multiyear TAC and not the correct year continue on */
		FunctGroupArray[sp].speciesParams[tac_resetnow_id] = 0;
        if(FunctGroupArray[sp].speciesParams[tac_resetcount_id] < FunctGroupArray[sp].speciesParams[tac_resetperiod_id]) {
            return;
        }
		FunctGroupArray[sp].speciesParams[tac_resetcount_id] = 0; // Reinitialise the counter if going to reset TAC
		FunctGroupArray[sp].speciesParams[tac_resetnow_id] = 1;


		/** Make decisions on effort and total quota **/
		/* Use assessment model output to determine changes needed.
		 Target reference point RefA is typically 40% and
		 Limit reference point RefB is typically 20%

		 Assumes all biomasses in tonnes until get final RBC which is then
		 converted to kg.

		 */
		RBC_scale = 1.0; /* recommended biological catch*/
		TACorig = FunctGroupArray[sp].speciesParams[TACorig_id];
		tier = (int) (FunctGroupArray[sp].speciesParams[tier_id]);

        
        if (bm->estBo[sp] > 0) {
            BrefA = FunctGroupArray[sp].speciesParams[BrefA_id] * bm->estBo[sp];
            BrefB = FunctGroupArray[sp].speciesParams[BrefB_id] * bm->estBo[sp];
            BrefC = FunctGroupArray[sp].speciesParams[BrefC_id] * bm->estBo[sp];
            BrefD = FunctGroupArray[sp].speciesParams[BrefD_id] * bm->estBo[sp];
            Blim = FunctGroupArray[sp].speciesParams[Blim_id] * bm->estBo[sp];
        } else {
            BrefA = FunctGroupArray[sp].speciesParams[BrefA_id] * bm->estinitpop[sp];
            BrefB = FunctGroupArray[sp].speciesParams[BrefB_id] * bm->estinitpop[sp];
            BrefC = FunctGroupArray[sp].speciesParams[BrefC_id] * bm->estinitpop[sp];
            BrefD = FunctGroupArray[sp].speciesParams[BrefD_id] * bm->estinitpop[sp];
            Blim = FunctGroupArray[sp].speciesParams[Blim_id] * bm->estinitpop[sp];
        }

		Bcurr = bm->NAssess[sp][est_med_stock_id];
		FrefA = bm->NAssess[sp][est_FrefA_id];
		FrefC = bm->NAssess[sp][est_FrefC_id];
		Fcurr = bm->NAssess[sp][est_Fcurr_id];
		M = bm->NAssess[sp][est_M_id];
		Ccurr = bm->NAssess[sp][est_Ccurr_id];
		Cslope = bm->NAssess[sp][est_Cslope_id];
		Num_yr = bm->NAssess[sp][est_numyr_id];
		CPUE_now = bm->NAssess[sp][est_CPUEnow_id];
		CPUE_start = bm->NAssess[sp][est_CPUEstart_id];

		switch (tier) {
		case tier_orig: /* Historical TAC setting based on CPUE trend.
		 If TAC changed reset est_numyr = 0 for next round
		 */
			CPUE_ratio = CPUE_now / (CPUE_start + small_num);
			if ((Cslope > 0) && (Num_yr >= bm->TAC_multiyr)) {
				RBC_scale = bm->prop_incTAC;
				bm->NAssess[sp][est_numyr_id] = 0;
			}
			if ((Cslope < 0) && (CPUE_ratio < bm->critCPUE) && (Num_yr >= bm->TAC_multiyr)) {
				RBC_scale = bm->prop_decTAC;
				bm->NAssess[sp][est_numyr_id] = 0;
			}

			RBC = 0.0; /* Deal with compiler warnings - RBC will be set later.*/
			break;
		case tier1: /* Tier 1 - Best quantitative assessment available */
			if (Bcurr >= BrefA) {
				/* Greater than BrefA (e.g. B48) so use F48 */
				FTARG = FrefA;
			} else if ((Bcurr < BrefA) && (Bcurr >= BrefB)) {
				/* Less than BrefA and greater than BrefB (e.g. B40), for stability remain at F48 */
				FTARG = FrefA;
			} else if ((Bcurr < BrefB) && (Bcurr > Blim)) {
				/* Less than BrefB and greater than Blim (e.g. B20) so reduce F rate */
				FTARG = FrefA * ((Bcurr - Blim) / (BrefB - Blim));
			} else {
				/* Less than Blim so set F = 0 */
				FTARG = 0;
			}
			RBC = Bcurr * (1.0 - exp(-FTARG));

			if (bm->debug == debug_assess) {
				fprintf(llogfp, "Time: %e %s tier: %d, RBC: %e, Bcurr: %e, FTARG: %e (BrefA: %e, BrefB: %e, Blim: %e)\n", bm->dayt,
						FunctGroupArray[sp].groupCode, tier, RBC, Bcurr, FTARG, BrefA, BrefB, Blim);
			}
			break;
		case tier2: /* Tier 2 - Out of date or lower quality analysis */
			if (Bcurr >= BrefC) {
				/* Greater than BrefC (e.g. B60) so use F60 */
				FTARG = FrefC;
			} else if ((Bcurr < BrefC) && (Bcurr >= BrefD)) {
				/* Less than BrefC and greater than BrefD (e.g. B50), for stability remain at F60 */
				FTARG = FrefC;
			} else if ((Bcurr < BrefD) && (Bcurr >= Blim)) {
				/* Less than BrefD and greater than Blim (e.g. B20) so reduce F rate */
				FTARG = FrefC * ((Bcurr - Blim) / (BrefD - Blim));
			} else {
				/* Less than Blim so set F = 0 */
				FTARG = 0;
			}
			RBC = Bcurr * (1.0 - exp(-FTARG));

			if (bm->debug == debug_assess) {
				fprintf(llogfp, "Time: %e %s tier: %d, RBC: %e, Bcurr: %e, FTARG: %e (BrefC: %e, BrefD: %e, Blim: %e)\n", bm->dayt,
						FunctGroupArray[sp].groupCode, tier, RBC, Bcurr, FTARG, BrefC, BrefD, Blim);
			}
			break;
		case tier3: /* Tier 3 - Catch based */
			if (Fcurr <= 0.5 * M)
				RBC = 1.2 * Ccurr;
			else if ((Fcurr <= 0.75 * M) && (Fcurr > 0.5 * M))
				RBC = 1.1 * Ccurr;
			else if ((Fcurr <= M) && (Fcurr > 0.75 * M))
				RBC = Ccurr;
			else if ((Fcurr <= 1.25 * M) && (Fcurr > M))
				RBC = 0.9 * Ccurr;
			else if ((Fcurr <= 1.5 * M) && (Fcurr > 1.25 * M))
				RBC = 0.8 * Ccurr;
			else if ((Fcurr < 2.0 * M) && (Fcurr > 1.5 * M))
				RBC = 0.5 * Ccurr;
			else
				RBC = 0;
			break;
		case tier4: /* Tier 4 - Catch trends */
			/* In assessment code first calculate:
			 Ccurr (average catch over last NC (e.g. 4) years)
			 Cslope (trend in CPUE over last NS (e.g. 4) years)
			 Now calculate RBC
			 */
			RBC = (1.0 + bm->RBCalpha * Cslope) * Ccurr;
			break;
		case tier5: /* Tier 5 - ERA-like analysis only. FIX - Finish (using ecological indicators) */
			RBC = 0.0; /* Deal with comiler warnings */
			quit("No actual code for this tier yet - %s tier5", FunctGroupArray[sp].groupCode);
			break;
		case tier6: /* US management system - based on Bmsy and Fmsy */
			r_est = bm->NAssess[sp][est_med_recruit_id];
			K_est = bm->NAssess[sp][est_med_prm2_id];

			RBC = Ccurr;
			bm->DAScalc[sp] = 1.0;

			/* MSY-based management updating */
			use_lever = (int) (FunctGroupArray[sp].speciesParams[use_lever_id]);
			for (nf = 0; nf < bm->K_num_fisheries; nf++) {
				FC_restrict = bm->FISHERYprms[nf][FC_restrict_id];
				Fratio = 1.0;

				FTARG = Fcurr;
				if (Bcurr < (0.5 * (K_est / 2.0))) {
					/* Overfished */
					FTARG = Fcurr * (1.0 - FC_restrict);
				} else {
					/* If not overfished then set F so not overfishing */
					FTARG = 0.5 * (r_est / 2.0);
				}

				if (Fcurr > (0.5 * (r_est / 2.0))) {
					/* Overfishing */
					FTARG = Fcurr * (1.0 - FC_restrict);
				}

				/* Find ratio of current F and desirable F - so can modify effort or catch accordingly */
				Fratio = 1.0;
				if (Fcurr != 0.0) {
					Fratio = FTARG / (Fcurr);
				}

				yr_id = (int) floor((bm->FISHERYprms[nf][FC_period_id] / 364.0) + 0.5);
				yr_id += bm->thisyear; /* Year management will take effect */
				switch (use_lever) {
				case use_none: /* Nothing to do */
					LeverQueue[sp][nf][yr_id] = 1.0;
					break;
				case use_effort: /* Effort change */
				case use_catch: /* Quota change */
				case use_space: /* Spatial management change */
				case use_season: /* Season change */
				case use_gear: /* Gear change */
					LeverQueue[sp][nf][yr_id] = Fratio;
					break;
				default:
					quit("No such lever for MSY based management (must be between %d for no lever to %d to for changing gear\n", use_none, use_gear);
					break;
				}
			}
			break;
		case tier7:
			RBC = 0.0;	/* Deal with comiler warnings - RBC will be used uninitialized later.*/
					// Do nothing as its for cross species rules see Check_Ecosystem_F_Harvest_Control_Rule();
			break;
        case tier8:
        case tier9:
        case tier13:
            RBC = 0.0;
            quit("Need to add code for this tier in AMS_Tiered_Assessment()\n");
            break;
		default:
			RBC = 0.0; /* Deal with comiler warnings */
			quit("No such management tier (%d) currently defined - species %s\n", tier, FunctGroupArray[sp].groupCode);
			break;
		}

		/* Convert RBC to kg from tonnes */
		RBC *= 1000.0;

		/* Get total quota for the previous year */
		totTAC = 0;
		totcatch = 0;
		for (nf = 0; nf < bm->K_num_fisheries; nf++) {

			/* Only change management in fisheries where management active */
			if (bm->FISHERYprms[nf][manage_on_id] < 1)
				continue;

			flagrecfish = (int) (bm->FISHERYprms[nf][flagrecfish_id]);
			in_quota = bm->inQuota[nf][sp];
			if (!flagrecfish) {
				if (in_quota) {
					/* Don't include recfishing in quota allocation for now (as often not quota-ed)
					 FIX - may have to change this if recfishing becomes quota allocated group
					 */

					/* Also update annual reporting and storage of old quotas */
					bm->TACamt[sp][nf][old_id] = bm->TACamt[sp][nf][now_id];

                    if(!FunctGroupArray[sp].isTAC || (bm->TACamt[sp][nf][now_id] < no_quota)) {
                        totTAC += bm->TACamt[sp][nf][now_id];
                    }
				}
				if (FunctGroupArray[sp].speciesParams[sp_concern_id]) {
					totcatch += Harvest_Get_TotCumCatch(sp, nf, bm->thisyear);//bm->TotCumCatch[sp][nf][bm->thisyear];
					if (bm->BycatchCountDiscards && (!bm->flagTACincludeDiscard && (FunctGroupArray[sp].isFished == TRUE))) {
						totcatch += (bm->TotCumDiscards[sp][nf] + bm->TotOldCumDiscards[sp][nf]);
					}
				}
			}
		}

		/* Sanity check */
		if (_isnan(RBC))
			RBC = totTAC;

		/* If historical TAC setting then scale current TAC based on long-term CPUE trends */
		if (tier == tier_orig) {
			RBC = totTAC * RBC_scale;
		} else {

			fprintf(bm->logFile, "totTAC = %e, bm->prop_incTAC = %e, bm->prop_decTAC = %e\n", totTAC, bm->prop_incTAC, bm->prop_decTAC);

			/* Else check if moderation of TAC change required */
			if (RBC > (bm->prop_incTAC * totTAC))
				RBC = bm->prop_incTAC * totTAC;
			else if (RBC < (bm->prop_decTAC * totTAC)) {
				RBC = bm->prop_decTAC * totTAC;
				if (bm->flagTradeTACvsMPA) {
					cue_spatial_closure = 1;
				}
			} else {
				/* Not changing by much and spatial closures already in place then they should continue */
				for (nf = 0; nf < bm->K_num_fisheries; nf++) {
					if (bm->SP_FISHERYprms[sp][nf][TACvsMPA_id])
						cue_spatial_closure = 1;
				}
			}

			fprintf(llogfp, "Time: %e, %s RBC: %e check before bounding\n", bm->dayt, FunctGroupArray[sp].groupCode, RBC);

			if (bm->flagQuotaBound) {
				if (RBC > (bm->TACbound * TACorig)) {
					RBC = bm->TACbound * TACorig;

					fprintf(llogfp, "Time: %e, %s RBC: %e, TACbound: %e, TACorig: %e\n", bm->dayt, FunctGroupArray[sp].groupCode, RBC, bm->TACbound,
							TACorig);
				}
			}
			if ((bm->flagQuotaBound > 1) && (bm->TACbound)) {
				if (RBC < (TACorig / bm->TACbound)) {
					RBC = TACorig / bm->TACbound;

					fprintf(llogfp, "Time: %e, %s RBC: %e, TACbound: %e, TACorig: %e\n", bm->dayt, FunctGroupArray[sp].groupCode, RBC, bm->TACbound,
							TACorig);
				}
			}
		}

		/* Handle multi-year TAC by multiplying annual allocation by the period of time it is to apply - if bulkTAC (i.e. one TAC to cover tac_reset number of years)
		 * If instead its a case where its an annual TAC, but only reset the TAC every tac_reset years then don't need the multiplier here
		 */
		if(bm->bulkTAC){
			RBC_scale = (double)(FunctGroupArray[sp].speciesParams[tac_resetperiod_id]);
			RBC *= RBC_scale;
		}

		/* When calculating days-at-sea cap resets only include species of concern and species where catch is actually taken */
		if (FunctGroupArray[sp].speciesParams[sp_concern_id] && (totcatch > 0)) {
			bm->DAScalc[sp] = RBC / (totcatch + small_num);
		}

		/* For all other tiers don't shift TAC unless sufficiently different from current TAC */
		if (fabs(RBC - totTAC) < bm->TAC_crit_change) {
			return;
		}

		/* TODO - dynamic phase out rules needed */
		for (nf = 0; nf < bm->K_num_fisheries; nf++) {

			/* Only change management in fisheries where management active */
			if (bm->FISHERYprms[nf][manage_on_id] < 1)
				continue;

			/* If phase out date elapsed turn off ability to have TAC for the group */
			if (bm->SP_FISHERYprms[sp][nf][phase_out_id] > bm->dayt) {
				bm->phased_out[sp][nf] = 0.0;
			} else if (bm->SP_FISHERYprms[sp][nf][phase_start_id] > bm->dayt) {
				end_date = bm->SP_FISHERYprms[sp][nf][phase_out_id];
				start_date = bm->SP_FISHERYprms[sp][nf][phase_start_id];
				period = end_date - start_date;
				TAC_scale = 1.0 - (bm->dayt - start_date) / period;
				bm->phased_out[sp][nf] = TAC_scale;
			} else {
				/* Do nothing so keep as 1.0 */
				bm->phased_out[sp][nf] = 1.0;
			}
		}

		/* Set total quota changes - checking for "phased out" fisheries.
		 Then allocate across the fisheries.
		 */
		RBC_alloc = 0;
		for (nf = 0; nf < bm->K_num_fisheries; nf++) {

			/* Only change management in fisheries where management active */
			if (bm->FISHERYprms[nf][manage_on_id] < 1)
				continue;

			flagrecfish = (int) (bm->FISHERYprms[nf][flagrecfish_id]);
			in_quota = bm->inQuota[nf][sp];
			if (!flagrecfish && in_quota) {
				/* Don't include recfishing in quota allocation for now (as often not quota-ed)
				 FIX - may have to change this if recfishing becomes quota allocated group
				 */

				TACorig = bm->TACamt[sp][nf][old_id];
				TAC_prop = TACorig * bm->phased_out[sp][nf] / (totTAC + small_num);
				bm->TACamt[sp][nf][now_id] = TAC_prop * RBC;
				TAC_scale = bm->TACamt[sp][nf][now_id] / (bm->TACamt[sp][nf][old_id] + small_num);
				RBC_alloc += bm->TACamt[sp][nf][now_id];

				/* Check for magnitude of TAC change */
				olddiff = fabs(1.0 - bm->TAC_trigger[nf][triggered_scalar_id]);
				newdiff = fabs(1.0 - TAC_scale);
				if (newdiff > olddiff)
					bm->TAC_trigger[nf][triggered_scalar_id] = TAC_scale;

				/* Output simple list of new TACs */
				fprintf(llogfp, "Time: %e, sp %s in %s new TAC = %e (TAC_prop: %e, RBC: %e, TAC_scale: %e, TACold: %e, RBC_alloc: %e)\n", bm->dayt,
						FunctGroupArray[sp].groupCode, FisheryArray[nf].fisheryCode, bm->TACamt[sp][nf][now_id], TAC_prop, RBC, TAC_scale,
						bm->TACamt[sp][nf][old_id], RBC_alloc);
			}
		}

		if (fabs(RBC_alloc - RBC) > buffer_rounding) {
			/* If some fisheries phased out will need to
			 rescale here so don't under allocate in total
			 */
			for (nf = 0; nf < bm->K_num_fisheries; nf++) {

				/* Only change management in fisheries where management active */
				if (bm->FISHERYprms[nf][manage_on_id] < 1)
					continue;

				flagrecfish = (int) (bm->FISHERYprms[nf][flagrecfish_id]);
				in_quota = bm->inQuota[nf][sp];
				if (!flagrecfish && in_quota) {
					/* Don't include recfishing in quota allocation for now (as often not quota-ed)
					 FIX - may have to change this if recfishing becomes quota allocated group
					 */
					bm->TACamt[sp][nf][now_id] *= RBC / (RBC_alloc + small_num);

					/* Output simple list of new TACs */
					fprintf(llogfp, "Time: %e, sp %s in %s new updated TAC = %e (RBC: %e, RBC_alloc: %e)\n", bm->dayt, FunctGroupArray[sp].groupCode,
							FisheryArray[nf].fisheryCode, bm->TACamt[sp][nf][now_id], RBC, RBC_alloc);
				}
			}
		}

		/* Output simple list of new TACs */
		fprintf(llogfp, "Time: %e, sp %s RBC (t): %e (old TAC (t): %e, RBC_alloc: %e)\n", bm->dayt, FunctGroupArray[sp].groupCode, RBC / 1000.0, totTAC
				/ 1000.0, RBC_alloc);

		/* If trading seasonal spatial closure of group for reduced TAC put in marker now */
		if (cue_spatial_closure) {
			for (nf = 0; nf < bm->K_num_fisheries; nf++) {
				bm->SP_FISHERYprms[sp][nf][TACvsMPA_id] = 1;
			}
		} else {
			for (nf = 0; nf < bm->K_num_fisheries; nf++) {
				bm->SP_FISHERYprms[sp][nf][TACvsMPA_id] = 0;
			}
		}

		// Iterate TAC counter (so for multiple years know if next year want to reset TAC (potentially depending on assessments)
		FunctGroupArray[sp].speciesParams[tac_resetcount_id] += 1.0;
	}

	return;
}

 /**************************************************************************//**
 *	\brief When using F rates for the fishing model, may still want a harvest control
 *  rule. In this case use this routine
 *
 *******************************************************************************/
void Check_F_Harvest_Control_Rule(MSEBoxModel *bm, FILE *llogfp) {
	int sp, nf, tier;
    
    // Per species HCR setting
    switch (bm->do_sumB_HCR) {
        case per_sp_rescale:
            for (sp = 0; sp < bm->K_num_tot_sp; sp++) {
               if (FunctGroupArray[sp].isFished == TRUE) {
               
                   if(bm->usingRedus_R_HCR) {   // Used to say < 1 but that would mean it wasn't in RedusR - also used to be at a species level using FunctGroupArray[sp].speciesParams[flagusingRedusR_HCR_id] but now assume it is all or nothing across all species in the model
                       continue; // As being done in R instead
                   }

                   /* Set the mFC scale value to 1.0 just in case */
                   for (nf = 0; nf < bm->K_num_fisheries; nf++) {
                       bm->SP_FISHERYprms[sp][nf][mFC_scale_id] = 1.0;
                   }

                   /* If not resetting (but doing TAC reset etc) skip ahead now */
                   if (!FunctGroupArray[sp].speciesParams[flagFonly_id])
                       continue;

                   /* Check to see that want to use a harvest control rule */
                   tier = (int) (FunctGroupArray[sp].speciesParams[tier_id]);
                   if (tier != tier_orig) {
                       /* Calculate Fcurr and update using the harvest control rule */
                        Per_Sp_Frescale(bm, llogfp, sp);
                   }
               }
            }
            break;
        case per_guild_rescale:
            //Reset done check
            for (sp = 0; sp < bm->K_num_tot_sp; sp++) {
                FunctGroupArray[sp].speciesParams[done_Co_sp_id] = 0;
                
                /* Set the mFC scale value to 1.0 just in case - can't do in next loop as could be reset as part of the guild */
                for (nf = 0; nf < bm->K_num_fisheries; nf++) {
                    bm->SP_FISHERYprms[sp][nf][mFC_scale_id] = 1.0;
                }
            }
            
            for (sp = 0; sp < bm->K_num_tot_sp; sp++) {
                if (FunctGroupArray[sp].isFished == TRUE) {
                    if(bm->usingRedus_R_HCR) {  // Used to say < 1 but that would mean it wasn't in RedusR - also used to be at a species level using FunctGroupArray[sp].speciesParams[flagusingRedusR_HCR_id] but now assume it is all or nothing across all species in the model
                        continue; // As being done in R instead
                    }

                    /* If not resetting (but doing TAC reset etc) skip ahead now */
                    if (!FunctGroupArray[sp].speciesParams[flagFonly_id])
                        continue;

                    /* Check to see that want to use a harvest control rule - assumes all species in the guild handled in teh same way */
                    int tier = (int) (FunctGroupArray[sp].speciesParams[tier_id]);
                    if ((tier != tier_orig) && (!FunctGroupArray[sp].speciesParams[done_Co_sp_id])) {
                        /* Calculate Fcurr and update using the harvest control rule applied at a guild level */
                        Guild_Frescale(bm, llogfp, sp);
                    }
                }
            }
            break;
        case per_system_cap:
            for (sp = 0; sp < bm->K_num_tot_sp; sp++) {
                if (FunctGroupArray[sp].isFished == TRUE) {
                    
                    if(bm->usingRedus_R_HCR || (!FunctGroupArray[sp].speciesParams[flagFonly_id]) || (!FunctGroupArray[sp].speciesParams[flag_systcap_sp_id]))
                        continue;
                    
                    /* Set the mFC scale value to 1.0 just in case */
                    for (nf = 0; nf < bm->K_num_fisheries; nf++) {
                        bm->SP_FISHERYprms[sp][nf][mFC_scale_id] = 1.0;
                    }
                }
            }
            /* Get F rescaled */
             Ecosystem_Cap_Frescale(bm, llogfp);
            break;
    }

	return;
}


/**************************************************************************//**
*    \brief Calculates current B and F state and how need to rescale - applied per species
*
*******************************************************************************/
void Per_Sp_Frescale (MSEBoxModel *bm, FILE *llogfp, int sp) {
    int cohort, ij, b, nf, flagF, nc, k;
    double FTARG, F_rescale, Fcurr, calcF, Fstep1, this_mFC, counter;
    //double M;
    //double calcM;
    
    int tier = (int) (FunctGroupArray[sp].speciesParams[tier_id]);
    int er_case = (int) (FunctGroupArray[sp].speciesParams[estError_id]);
    double est_bias = FunctGroupArray[sp].speciesParams[estBias_id];
    double est_cv = FunctGroupArray[sp].speciesParams[estCV_id];
    
    int maxstock = FunctGroupArray[sp].numStocks;
    
    double BrefA = FunctGroupArray[sp].speciesParams[BrefA_id] * bm->estBo[sp];
    double BrefB = FunctGroupArray[sp].speciesParams[BrefB_id] * bm->estBo[sp];
    double BrefE = FunctGroupArray[sp].speciesParams[BrefE_id] * bm->estBo[sp];
    double Blim = FunctGroupArray[sp].speciesParams[Blim_id] * bm->estBo[sp];
    double FrefA = FunctGroupArray[sp].speciesParams[FrefA_id];
    double FrefH = FunctGroupArray[sp].speciesParams[FrefH_id];
    double FrefLim = FunctGroupArray[sp].speciesParams[FrefLim_id];

    double Braw = 0.0, Bcurr;

    if(!do_assess) {  // Where do_assess set at atlantismain.c level as requires Assess_Resources() call in atasseess lib
        if (bm->flagSSBforHCR){
            /* Using SSB in the HCR - HAP 2024 - I wrote this using script from atSSBDataGen.c */
            for (cohort = 0; cohort < FunctGroupArray[sp].numCohortsXnumGenes; cohort++) {
                for (ij = 0; ij < bm->nbox; ij++) {
                    for (b = 0; b < bm->boxes[ij].nz; b++) {
                            Braw += ((bm->boxes[ij].tr[b][FunctGroupArray[sp].structNTracers[cohort]] + bm->boxes[ij].tr[b][FunctGroupArray[sp].resNTracers[cohort]]) *
                                    bm->boxes[ij].tr[b][FunctGroupArray[sp].NumsTracers[cohort]] *
                                    FunctGroupArray[sp].habitatCoeffs[WC] *
                                    bm->X_CN *
                                    mg_2_tonne) *
                                    FunctGroupArray[sp].scaled_FSPB[cohort];
                                // Note, HAP tried this withough FSPB and it produced the same value as bm->totfishpop[sp] * bm->X_CN * mg_2_tonne
                            }
                    }
                }
                    fprintf(llogfp, "Time: %e %s, Braw (total SSB) before Assess_Add_Error() - %e\n", bm->dayt, FunctGroupArray[sp].groupCode, Braw);
        } else {
            Braw = bm->totfishpop[sp] * bm->X_CN * mg_2_tonne;
            fprintf(llogfp, "Time: %e %s, Braw (total stock biomass) before Assess_Add_Error() - %e\n", bm->dayt, FunctGroupArray[sp].groupCode, Braw);
        }
        Bcurr = Assess_Add_Error(bm, er_case, Braw, est_bias, est_cv);
    } else {
        Bcurr = bm->NAssess[sp][est_med_stock_id];
        Fcurr = bm->NAssess[sp][est_Fcurr_id];
        //M = bm->NAssess[sp][est_M_id];
    }
    
    fprintf(llogfp, "Time: %e doing %s with tier %d\n", bm->dayt, FunctGroupArray[sp].groupCode, tier);
    fprintf(llogfp, "The HCR refernce poitns are as follows:\n");
    fprintf(llogfp, "BrefA: %e, BrefB: %e, Blim: %e, FrefLim: %e, FrefA: %e, FrefH: %e\n", BrefA, BrefB, Blim, FrefLim, FrefA, FrefH);
    
    /* Find Fcurr *
     update_date = (int) (FunctGroupArray[sp].speciesParams[calcupdate_date_id]);
     // Start with mortality estimates
     
     // This is the old method - not used anymore //
     
     // TODO: This should be F from calcTrackedMort
     if (update_date > 182) {
     // If restarted records within last 6 months use previous year's records as more complete
     start_N = bm->calcNstart[sp][hist_id];
     catch_N = bm->calcFnum[sp][hist_id];
     mort_scale = 1.0;
     } else {
     // If restart records early each year then current records should be sufficent
     start_N = bm->calcNstart[sp][expect_id];
     catch_N = bm->calcFnum[sp][expect_id];
     mort_scale = 365.0 / (365.0 - update_date);
     }
     
     // Sanity checks
     if (start_N < 0)
     start_N = 1;
     if (catch_N < 0)
     catch_N = 0;
     
     Fcurr = catch_N / (start_N + small_num);            // TODO: We may need to make this a direct read of mFC * mFC_scale rather than "estimate" F
     
     // Adjust for any partial year of data effects
     Fcurr *= mort_scale;
     
     */
    
    /** New perfect knowledge way of determining M and F */
    calcF = 0.0;
    //calcM = 0.0;
    counter = 0.0;
    for (nc = 0; nc < FunctGroupArray[sp].numCohorts; nc++) {
        for (k = 0; k < maxstock; k++) {
            //calcM += (bm->calcTrackedMort[sp][nc][k][finalM1_id] + bm->calcTrackedMort[sp][nc][k][finalM2_id]);
            calcF += bm->calcTrackedMort[sp][nc][k][finalF_id];
            counter++;
        }
    }
    //calcM /= counter;
    calcF /= counter;
    Fcurr = calcF;
    
    fprintf(llogfp, "Time: %e %s has Fcurr %e\n", bm->dayt, FunctGroupArray[sp].groupCode, Fcurr);
    
    switch (tier) {
        case tier0: // Intentional flow throgh for all these cases
        case tier1:
        case tier2:
        case tier3:
        case tier4:
        case tier5:
        case tier6:
        case tier7:
        case dyntier4:
        case dyntier1B0:
        case sp_rollover: // So not US (tier8) or Norway (tier 9) or Iceland (tier13)
            if (Bcurr >= BrefA) {
                /* Greater than BrefA (e.g. B48) so use F48 */
                FTARG = FrefA;
                //FTARG = Fcurr;
            } else if ((Bcurr < BrefA) && (Bcurr >= BrefB)) {
                /* Less than BrefA and greater than BrefB (e.g. B40), for stability remain at F48 */
                FTARG = FrefA;
            } else if ((Bcurr < BrefB) && (Bcurr > Blim)) {
                /* Less than BrefB and greater than Blim (e.g. B20) so reduce F rate */
                FTARG = FrefA * ((Bcurr - Blim) / (BrefB - Blim));
            } else {
                /* Less than Blim so set F = 0 */
                FTARG = 0;
            }
            break;
        case tier8: // Tier 8 version of the broken stick - how its applied in the US and Norway
        case tier9:
            if (Bcurr >= BrefA) {
                /* Greater than BrefA (e.g. B48) so fish at a higher rate */
                FTARG = FrefH;
            } else if ((Bcurr < BrefA) && (Bcurr >= BrefB)) {
                /* Less than BrefA and greater than BrefB (e.g. B40), for stability remain at F48 */
                FTARG = FrefA;
            } else if ((Bcurr < BrefB) && (Bcurr > Blim)) {
                /* Less than BrefB and greater than Blim (e.g. B20) so reduce F rate 
                   Formulate updated to allow for FrefLme to be non-zero */
                FTARG = ((FrefLim * (BrefB - Bcurr) + FrefA * (Bcurr - Blim)) / (BrefB - Blim));
            } else {
                /* Less than Blim so set F = FrefLim (typicall aboit 0.05) */
                FTARG = FrefLim;
            }
            break;
        case tier13: // Icelandic escapement approach
            if (Bcurr > Blim) {
                /* Bigger than Blim so set F rate */
                FTARG = 1 - (Blim / BrefB);
            } else {
                /* Less than Blim so set F = 0 */
                FTARG = 0;
            }
            break;
        case tier14: // Same as tier 1 but allows for truncating the descending limb and closing the fishery below BrefE, Alaska pollock and cod style
            if (Bcurr >= BrefA) {
                /* Greater than BrefA (e.g. B48) so use F48 */
                FTARG = FrefA;
                //FTARG = Fcurr;
            } else if ((Bcurr < BrefA) && (Bcurr >= BrefB)) {
                /* Less than BrefA and greater than BrefB (e.g. B40), for stability remain at F48 */
                FTARG = FrefA;
            } else if ((Bcurr < BrefB) && (Bcurr > BrefE)) {
                /* Less than BrefB and greater than BrefE (usually B20 for Alaska pollock and cod) so reduce F rate linearly towards Blim */
                FTARG = FrefA * ((Bcurr - Blim) / (BrefB - Blim));
            } else {
                /* Less than BrefE so set F = 0 */
               FTARG = 0;
            }
            break;
        default:
            quit("Per_Sp_Frescale: We do not have any code for tier %d\n", tier);
            break;
    }

    /* Rescale all F accordingly */
    if(!Fcurr && FTARG) {
        F_rescale = FunctGroupArray[sp].speciesParams[F_restart_id];
    } else {
        switch (tier) {
            case tier0:     // Intentional flow throgh for all these cases
            case tier1:
            case tier2:
            case tier3:
            case tier4:
            case tier5:
            case tier6:
            case tier7:
            case dyntier4:
            case dyntier1B0:
            case sp_rollover:
            case tier13:
            case tier14:
                Fstep1 = Fcurr / (FunctGroupArray[sp].speciesParams[maxmFC_id] * 365.0);  // As Fcurr is annual but mFC is daily
                F_rescale = Fstep1 * (FTARG / (Fcurr + small_num));  // Re-scale existing F
                break;
            case tier9: // Norwegian way of doing it - to avoid overcatch during spin-up phase
                Fstep1 = Fcurr / (FunctGroupArray[sp].speciesParams[maxmFC_id] * 365.0);  // As Fcurr is annual but mFC is daily
                F_rescale = Fstep1 * (FTARG / (FrefA + small_num));
                break;
            case tier8:
                /* In tier 8 version rescaling vs FrefA - although applied vs mFC
                 so just in case the user has not entered FrefA for mFC do a two step rescaling here */
                this_mFC = FunctGroupArray[sp].speciesParams[maxmFC_id] * 365.0;  // As FrefA is annual but mFC is daily
                Fstep1 = FrefA / this_mFC;
                F_rescale = Fstep1 * (FTARG / (FrefA + small_num));  // Re-scale the F
                break;
            default:
                quit("Per_Sp_Frescale: We do not have any code for tier %d\n", tier);
                break;
        }
    }
    
    if (bm->checkstart) {
        fprintf(llogfp, "Time: %e %s F_rescale: %e, FTARG: %e, Fcurr: %e)\n", bm->dayt, FunctGroupArray[sp].groupCode, F_rescale, FTARG, Fcurr);
    }

    /* If this is the first year then don't do the scaling */
    for (nf = 0; nf < bm->K_num_fisheries; nf++) {
        /* Only change management in fisheries where management active */
        if (bm->FISHERYprms[nf][manage_on_id] < 1)
            continue;

        flagF = (int) (bm->SP_FISHERYprms[sp][nf][flagF_id]);
        if (flagF) {

            if(!tier){
                bm->SP_FISHERYprms[sp][nf][mFC_scale_id] = 1.0;
            } else {
                bm->SP_FISHERYprms[sp][nf][mFC_scale_id] = F_rescale;
            }

            WriteAnnBrokenStickFile(bm, sp, nf, tier, FrefLim, FrefA, FrefH, Blim, BrefA, BrefB, Fcurr, FTARG, Bcurr, F_rescale);

            if (bm->checkstart) {
                fprintf(llogfp, "Time: %e %s mFC_scale: %e, F_rescale: %e)\n", bm->dayt, FunctGroupArray[sp].groupCode, bm->SP_FISHERYprms[sp][nf][mFC_scale_id], F_rescale);
            }
        }
    }
    
    return;
    
}

/**************************************************************************//**
*    \brief Calculates current B and F state and how need to rescale - applied per guild
*    Triggered by the first species in the list for the guild - reusing the co_sp_FXX parameters
*
*******************************************************************************/
void Guild_Frescale (MSEBoxModel *bm, FILE *llogfp, int sp) {
    int nf, flagF, nc, k, groupIndex, othersp;
    double FTARG, F_rescale, Fstep1, this_mFC;
    double *calcM_sp = Util_Alloc_Init_1D_Double(bm->K_max_co_sp + 1, 0.0); // The +1 is for the primary species
    double *calcF_sp = Util_Alloc_Init_1D_Double(bm->K_max_co_sp + 1, 0.0); // The +1 is for the primary species
    double *counter_sp = Util_Alloc_Init_1D_Double(bm->K_max_co_sp + 1, 0.0); // The +1 is for the primary species
    double *Fcurr_sp = Util_Alloc_Init_1D_Double(bm->K_max_co_sp + 1, 0.0); // The +1 is for the primary species
    double counter, Fcurr;

    int tier = (int) (FunctGroupArray[sp].speciesParams[tier_id]);
    int er_case = (int) (FunctGroupArray[sp].speciesParams[estError_id]);
    double est_bias = FunctGroupArray[sp].speciesParams[estBias_id];
    double est_cv = FunctGroupArray[sp].speciesParams[estCV_id];
    int maxstock = FunctGroupArray[sp].numStocks;
    double Braw = bm->totfishpop[sp] * bm->X_CN * mg_2_tonne;
    double Bcurr = Assess_Add_Error(bm, er_case, Braw, est_bias, est_cv);
    double BrefA = FunctGroupArray[sp].speciesParams[BrefA_id] * bm->estBo[sp];
    double BrefB = FunctGroupArray[sp].speciesParams[BrefB_id] * bm->estBo[sp];
    double Blim = FunctGroupArray[sp].speciesParams[Blim_id] * bm->estBo[sp];
    double FrefA = FunctGroupArray[sp].speciesParams[FrefA_id];
    double FrefH = FunctGroupArray[sp].speciesParams[FrefH_id];
    double FrefLim = FunctGroupArray[sp].speciesParams[FrefLim_id];
    
    fprintf(llogfp, "Time: %e doing %s with tier %d\n", bm->dayt, FunctGroupArray[sp].groupCode, tier);
    fprintf(llogfp, "The HCR refernce poitns are as follows:\n");
    fprintf(llogfp, "BrefA: %e, BrefB: %e, Blim: %e, FrefLim: %e, FrefA: %e, FrefH: %e\n", BrefA, BrefB, Blim, FrefLim, FrefA, FrefH);

    for (groupIndex = 0; groupIndex < FunctGroupArray[sp].speciesParams[max_co_sp_id]; groupIndex++ ) {
        othersp = FunctGroupArray[sp].co_sp[groupIndex];
    
        Braw += bm->totfishpop[othersp] * bm->X_CN * mg_2_tonne;
        Bcurr += Assess_Add_Error(bm, er_case, Braw, est_bias, est_cv);
    
        BrefA += FunctGroupArray[othersp].speciesParams[BrefA_id] * bm->estBo[othersp];
        BrefB += FunctGroupArray[othersp].speciesParams[BrefB_id] * bm->estBo[othersp];
        Blim += FunctGroupArray[othersp].speciesParams[Blim_id] * bm->estBo[othersp];
    }
    FrefA = FunctGroupArray[sp].speciesParams[FrefA_id];
    FrefH = FunctGroupArray[sp].speciesParams[FrefH_id];
    
    /** Using perfect knowledge way of determining M and F - averaging over the species in the guild */
    for (groupIndex = 0; groupIndex < FunctGroupArray[sp].speciesParams[max_co_sp_id]; groupIndex++ ) {
        if (groupIndex < bm->K_max_co_sp) {
            othersp = FunctGroupArray[sp].co_sp[groupIndex];
        } else {
            othersp = sp;
        }
        
        calcF_sp[groupIndex] = 0.0;
        calcM_sp[groupIndex] = 0.0;
        counter_sp[groupIndex] = 0.0;
        Fcurr_sp[groupIndex] = 0.0;
        maxstock = FunctGroupArray[othersp].numStocks;
        
        for (nc = 0; nc < FunctGroupArray[othersp].numCohorts; nc++) {
            for (k = 0; k < maxstock; k++) {
                calcM_sp[groupIndex] += (bm->calcTrackedMort[othersp][nc][k][finalM1_id] + bm->calcTrackedMort[othersp][nc][k][finalM2_id]);
                calcF_sp[groupIndex] += bm->calcTrackedMort[othersp][nc][k][finalF_id];
                counter_sp[groupIndex] += 1.0;
            }
        }
        calcM_sp[groupIndex] /= counter_sp[groupIndex];
        calcF_sp[groupIndex] /= counter_sp[groupIndex];
        Fcurr_sp[groupIndex] = calcF_sp[groupIndex];
        
        fprintf(llogfp, "Time: %e %s has Fcurr %e\n", bm->dayt, FunctGroupArray[othersp].groupCode, Fcurr_sp[groupIndex]);

    }
    
    // Calculate the average Fcurr
    Fcurr = 0.0;
    counter = (double)(bm->K_max_co_sp + 1);
    for (groupIndex = 0; groupIndex < bm->K_max_co_sp + 1; groupIndex++) {
        Fcurr += Fcurr_sp[groupIndex];
    }
    Fcurr_sp[groupIndex] /= counter;

    if (tier < tier8) {  // So not US (tier8) or Norway (tier 9)
        /* Tier 1 - Best quantitative assessment available */
        if (Bcurr >= BrefA) {
            /* Greater than BrefA (e.g. B48) so use F48 */
            FTARG = FrefA;
            //FTARG = Fcurr;
        } else if ((Bcurr < BrefA) && (Bcurr >= BrefB)) {
            /* Less than BrefA and greater than BrefB (e.g. B40), for stability remain at F48 */
            FTARG = FrefA;
        } else if ((Bcurr < BrefB) && (Bcurr > Blim)) {
            /* Less than BrefB and greater than Blim (e.g. B20) so reduce F rate */
            FTARG = FrefA * ((Bcurr - Blim) / (BrefB - Blim));
        } else {
            /* Less than Blim so set F = 0 */
            FTARG = 0;
        }
    } else {
        /* Tier 8 version of the broken stick - how its applied in the US and Norway */
        if (Bcurr >= BrefA) {
            /* Greater than BrefA (e.g. B48) so fish at a higher rate */
            FTARG = FrefH;
        } else if ((Bcurr < BrefA) && (Bcurr >= BrefB)) {
            /* Less than BrefA and greater than BrefB (e.g. B40), for stability remain at F48 */
            FTARG = FrefA;
        } else if ((Bcurr < BrefB) && (Bcurr > Blim)) {
            /* Less than BrefB and greater than Blim (e.g. B20) so reduce F rate
               Formulate updated to allow for FrefLme to be non-zero */
            FTARG = ((FrefLim * (BrefB - Bcurr) + FrefA * (Bcurr - Blim)) / (BrefB - Blim));
        } else {
            /* Less than Blim so set F = FrefLim (typicall aboit 0.05) */
            FTARG = FrefLim;
        }
    }

    /* Rescale all F accordingly - base it on primary species - TODO: FIX should this be done per species? */
    if(!Fcurr && FTARG) {
        F_rescale = FunctGroupArray[sp].speciesParams[F_restart_id];
    } else if (tier != tier8) {
        Fstep1 = Fcurr / (FunctGroupArray[sp].speciesParams[maxmFC_id] * 365.0);  // As Fcurr is annual but mFC is daily
        if (tier != tier9) {
            F_rescale = Fstep1 * (FTARG / (Fcurr + small_num));  // Re-scale existing F
        } else {
            F_rescale = Fstep1 * (FTARG / (FrefA + small_num));  // Norwegian way of doing it - to avoid overcatch during spin-up phase
        }
    } else {
        /* In tier 8 version rescaling vs FrefA - although applied vs mFC
         so just in case the user has not entered FrefA for mFC do a two step rescaling here */
        this_mFC = FunctGroupArray[sp].speciesParams[maxmFC_id] * 365.0;  // As FrefA is annual but mFC is daily
        Fstep1 = FrefA / this_mFC;
        F_rescale = Fstep1 * (FTARG / (FrefA + small_num));  // Re-scale the F
    }
    
    
    /* Apply rescaling to each species in the guild */
    for (groupIndex = 0; groupIndex < FunctGroupArray[sp].speciesParams[max_co_sp_id]; groupIndex++ ) {

        if (groupIndex < bm->K_max_co_sp) {
            othersp = FunctGroupArray[sp].co_sp[groupIndex];
        } else {
            othersp = sp;
        }

        if (bm->checkstart) {
            fprintf(llogfp, "Time: %e %s F_rescale: %e, FTARG: %e, Fcurr: %e)\n", bm->dayt, FunctGroupArray[othersp].groupCode, F_rescale, FTARG, Fcurr);
        }

        /* Write output */
        for (nf = 0; nf < bm->K_num_fisheries; nf++) {
            /* Only change management in fisheries where management active */
            if (bm->FISHERYprms[nf][manage_on_id] < 1)
                continue;

            flagF = (int) (bm->SP_FISHERYprms[othersp][nf][flagF_id]);
            if (flagF) {
                bm->SP_FISHERYprms[othersp][nf][mFC_scale_id] = F_rescale;
                WriteAnnBrokenStickFile(bm, othersp, nf, tier, FrefLim, FrefA, FrefH, Blim, BrefA, BrefB, Fcurr, FTARG, Bcurr, F_rescale);

                if (bm->checkstart) {
                    fprintf(llogfp, "Time: %e %s mFC_scale: %e, F_rescale: %e)\n", bm->dayt, FunctGroupArray[othersp].groupCode,
                            bm->SP_FISHERYprms[othersp][nf][mFC_scale_id], F_rescale);
                }
            }
        }
        
        // Set check so don't redo again later
        FunctGroupArray[othersp].speciesParams[done_Co_sp_id] = 1;
    }
    
    return;
    
}

/**************************************************************************//**
*    \brief Calculates current B and F state and how need to rescale if to meet a catch cap
*
*******************************************************************************/

void Ecosystem_Cap_Frescale(MSEBoxModel *bm, FILE *llogfp) {
    int sp, nf, nc, cohort, ij, b, k, flagF, tier, er_case, maxstock, mFC_end_age, mFC_start_age, flagfcmpa, sel_curve, stage, basechrt;
    double max_mFC, F_rescale, FTARG, Bcurr, calcM, survival, Fcurr, calcF, Fstep1, this_mFC, M, est_bias, est_cv, BrefA, BrefB, BrefE, Blim, FrefA, FrefH, FrefLim, Braw, sel, this_expect_catch, sp_fishery_pref_weight, w_inv, tot_w_inv, counter, mFC, mFC_change_scale, mpa_scale, mpa_infringe, Wgt, li, gear_change_scale, this_Num, this_start, this_end, this_Biom, Z_Est, expectF, Catch_Eqn_Denom, orig_expected_catch, excess, deductions, new_expected_catch, rescale_scalar,tot_area, fishable_area;
    //double calcM;
    
    /* Initialise weights if has not been done previously */
    if (!bm->sp_pref_inv_norm_done) {
        tot_w_inv = 0.0;
        for (sp = 0; sp < bm->K_num_tot_sp; sp++) {
            if (FunctGroupArray[sp].isFished == TRUE) {
                
                // Follow-through for sp that are not part of OY
                if((!FunctGroupArray[sp].speciesParams[flagFonly_id]) || (!FunctGroupArray[sp].speciesParams[flag_systcap_sp_id]))
                    continue;
                
                /* Weigths on each species - (e.g., as set by Council)  - with 1 is the least important, n > 1 more  important */
                sp_fishery_pref_weight = FunctGroupArray[sp].speciesParams[sp_fishery_pref_id];
                
                /* Use inverse weights to attribute a share of the excess to each stock */
                w_inv = 1.0 / sp_fishery_pref_weight;
                tot_w_inv += w_inv;
            }
        }

        /* Normalize w_inv */
        for (sp = 0; sp < bm->K_num_tot_sp; sp++) {
            if (FunctGroupArray[sp].isFished == TRUE) {
                
                // Follow-through for sp that are not part of OY
                if((!FunctGroupArray[sp].speciesParams[flagFonly_id]) || (!FunctGroupArray[sp].speciesParams[flag_systcap_sp_id]))
                    continue;
                
                sp_fishery_pref_weight = FunctGroupArray[sp].speciesParams[sp_fishery_pref_id];
                w_inv = 1.0 / sp_fishery_pref_weight;
                FunctGroupArray[sp].speciesParams[sp_fishery_pref_norm_id] = w_inv / tot_w_inv;
            }
        }
        bm->sp_pref_inv_norm_done = 1;
    }
    
    /* Prep the mpa calcs needed - as required per fleet not species so do up front - only update if need to */
    
    for (nf = 0; nf < bm->K_num_fisheries; nf++) {
        
        /* Correct for presence of mpas */
        flagfcmpa = (int) (bm->FISHERYprms[nf][flagmpa_id]);
        if (flagfcmpa) {
            tot_area = 0.0;
            fishable_area = 0.0;
            for (b = 0; b < bm->nbox; b++) {
                tot_area += bm->boxes[b].area;
                fishable_area += bm->boxes[b].area * bm->MPA[b][nf];
            }
            bm->FISHERYprms[nf][mpascale_cap_id] = fishable_area / tot_area;
        } else {
            bm->FISHERYprms[nf][mpascale_cap_id] = 1.0;
        }
            
    }


    /* Steps:
    1. Determine FTARG using single species approach (so how rescale mFC so to be in line with single species harvest cnotrol rule)
    2. Apply catch equation to find catch expected with that FTARG
    3. Sum the expected catches and compare against the system cap
    3. Redo mFc_scale to make sure not breaching system cap when aggregate across species
    */
    
    //Step 1: apply catch equation
    double tot_expect_catch = 0.0;
    for (sp = 0; sp < bm->K_num_tot_sp; sp++) {
        if (FunctGroupArray[sp].isFished == TRUE) {

            // Follow-through for sp that are not part of OY
            if((!FunctGroupArray[sp].speciesParams[flagFonly_id]) || (!FunctGroupArray[sp].speciesParams[flag_systcap_sp_id]))
                continue;

            gear_change_scale = 0;
            tier = (int) (FunctGroupArray[sp].speciesParams[tier_id]);
            er_case = (int) (FunctGroupArray[sp].speciesParams[estError_id]);
            est_bias = FunctGroupArray[sp].speciesParams[estBias_id];
            est_cv = FunctGroupArray[sp].speciesParams[estCV_id];
            maxstock = FunctGroupArray[sp].numStocks;
            BrefA = FunctGroupArray[sp].speciesParams[BrefA_id] * bm->estBo[sp];
            BrefB = FunctGroupArray[sp].speciesParams[BrefB_id] * bm->estBo[sp];
            BrefE = FunctGroupArray[sp].speciesParams[BrefE_id] * bm->estBo[sp];
            Blim = FunctGroupArray[sp].speciesParams[Blim_id] * bm->estBo[sp];
            FrefA = FunctGroupArray[sp].speciesParams[FrefA_id];
            FrefH = FunctGroupArray[sp].speciesParams[FrefH_id];
            FrefLim = FunctGroupArray[sp].speciesParams[FrefLim_id];
            
            if(!do_assess) {  // Where do_assess set at atlantismain.c level as requires Assess_Resources() call in atasseess lib
                if (bm->flagSSBforHCR){
                    /* Using SSB in the HCR - HAP 2024 - I wrote this using script from atSSBDataGen.c */
                    for (cohort = 0; cohort < FunctGroupArray[sp].numCohortsXnumGenes; cohort++) {
                        for (ij = 0; ij < bm->nbox; ij++) {
                            for (b = 0; b < bm->boxes[ij].nz; b++) {
                                 Braw += ((bm->boxes[ij].tr[b][FunctGroupArray[sp].structNTracers[cohort]] + bm->boxes[ij].tr[b][FunctGroupArray[sp].resNTracers[cohort]]) *
                                 bm->boxes[ij].tr[b][FunctGroupArray[sp].NumsTracers[cohort]] *
                                 FunctGroupArray[sp].habitatCoeffs[WC] *
                                 bm->X_CN *
                                 mg_2_tonne) *
                                 FunctGroupArray[sp].scaled_FSPB[cohort];
                                // Note, HAP tried this withough FSPB and it produced the same value as bm->totfishpop[sp] * bm->X_CN * mg_2_tonne
                            }
                        }
                    }
                    fprintf(llogfp, "Time: %e %s, Braw (total SSB) before Assess_Add_Error() - %e\n", bm->dayt, FunctGroupArray[sp].groupCode, Braw);
                } else {
                    Braw = bm->totfishpop[sp] * bm->X_CN * mg_2_tonne;
                    fprintf(llogfp, "Time: %e %s, Braw (total stock biomass) before Assess_Add_Error() - %e\n", bm->dayt, FunctGroupArray[sp].groupCode, Braw);
                }
                Bcurr = Assess_Add_Error(bm, er_case, Braw, est_bias, est_cv);
            } else {
                Bcurr = bm->NAssess[sp][est_med_stock_id];
                Fcurr = bm->NAssess[sp][est_Fcurr_id];
                M = bm->NAssess[sp][est_M_id];
            }

            max_mFC = 365.0 * FunctGroupArray[sp].speciesParams[maxmFC_id];
            //max_F = -365.0 * log(1.0 - max_mFC); // turn max_mFC to corresponding max F experienced by this species. BUT this breaks internal consistency with Fstep1
            
            /** New perfect knowledge way of determining M and F */
            calcF = 0.0;
            //calcM = 0.0;
            counter = 0.0;
            for (nc = 0; nc < FunctGroupArray[sp].numCohorts; nc++) {
                for (k = 0; k < maxstock; k++) {
                    //calcM += (bm->calcTrackedMort[sp][nc][k][finalM1_id] + bm->calcTrackedMort[sp][nc][k][finalM2_id]);
                    calcF += bm->calcTrackedMort[sp][nc][k][finalF_id];
                    counter++;
                }
            }
            //calcM /= counter;
            calcF /= counter;
            Fcurr = calcF;
            
            fprintf(llogfp, "Time: %e %s has Fcurr %e\n", bm->dayt, FunctGroupArray[sp].groupCode, Fcurr);
            
            switch (tier) {
                case tier0: // Intentional flow throgh for all these cases
                case tier1:
                case tier2:
                case tier3:
                case tier4:
                case tier5:
                case tier6:
                case tier7:
                case dyntier4:
                case dyntier1B0:
                case sp_rollover: // So not US (tier8) or Norway (tier 9) or Iceland (tier13)
                    if (Bcurr >= BrefA) {
                        /* Greater than BrefA (e.g. B48) so use F48 */
                        FTARG = FrefA;
                        //FTARG = Fcurr;
                    } else if ((Bcurr < BrefA) && (Bcurr >= BrefB)) {
                        /* Less than BrefA and greater than BrefB (e.g. B40), for stability remain at F48 */
                        FTARG = FrefA;
                    } else if ((Bcurr < BrefB) && (Bcurr > Blim)) {
                        /* Less than BrefB and greater than Blim (e.g. B20) so reduce F rate */
                        FTARG = FrefA * ((Bcurr - Blim) / (BrefB - Blim));
                    } else {
                        /* Less than Blim so set F = 0 */
                        FTARG = 0;
                    }
                    break;
                case tier8: // Tier 8 version of the broken stick - how its applied in the US and Norway
                case tier9:
                    if (Bcurr >= BrefA) {
                        /* Greater than BrefA (e.g. B48) so fish at a higher rate */
                        FTARG = FrefH;
                    } else if ((Bcurr < BrefA) && (Bcurr >= BrefB)) {
                        /* Less than BrefA and greater than BrefB (e.g. B40), for stability remain at F48 */
                        FTARG = FrefA;
                    } else if ((Bcurr < BrefB) && (Bcurr > Blim)) {
                        /* Less than BrefB and greater than Blim (e.g. B20) so reduce F rate
                           Formulate updated to allow for FrefLme to be non-zero */
                        FTARG = ((FrefLim * (BrefB - Bcurr) + FrefA * (Bcurr - Blim)) / (BrefB - Blim));
                    } else {
                        /* Less than Blim so set F = FrefLim (typicall aboit 0.05) */
                        FTARG = FrefLim;
                    }
                    break;
                case tier13: // Icelandic escapement approach
                    if (Bcurr > Blim) {
                        /* Bigger than Blim so set F rate */
                        FTARG = 1 - (Blim / BrefB);
                    } else {
                        /* Less than Blim so set F = 0 */
                        FTARG = 0;
                    }
                    break;
               case tier14: // Same as tier 1 but allows for truncating the descending limb and closing the fishery below BrefE, Alaska pollock and cod style
                    if (Bcurr >= BrefA) {
                        /* Greater than BrefA (e.g. B48) so use F48 */
                        FTARG = FrefA;
                        //FTARG = Fcurr;
                    } else if ((Bcurr < BrefA) && (Bcurr >= BrefB)) {
                        /* Less than BrefA and greater than BrefB (e.g. B40), for stability remain at F48 */
                        FTARG = FrefA;
                    } else if ((Bcurr < BrefB) && (Bcurr > BrefE)) {
                        /* Less than BrefB and greater than BrefE (usually B20 for Alaska pollock and cod) so reduce F rate linearly towards Blim */
                        FTARG = FrefA * ((Bcurr - Blim) / (BrefB - Blim));
                    } else {
                        /* Less than BrefE so set F = 0 */
                        FTARG = 0;
                    }
                    break;
                default:
                    quit("Per_Sp_Frescale: We do not have any code for tier %d\n", tier);
                    break;
            }

            /* Rescale all F accordingly to be in line with single species expectations - basically F to deliver the Acceptable Biological Catch */
            if(!Fcurr && FTARG) {
                F_rescale = FunctGroupArray[sp].speciesParams[F_restart_id];
            } else {
                switch (tier) {
                    case tier0:     // Intentional flow throgh for all these cases
                    case tier1:
                    case tier2:
                    case tier3:
                    case tier4:
                    case tier5:
                    case tier6:
                    case tier7:
                    case dyntier4:
                    case dyntier1B0:
                    case sp_rollover:
                    case tier13:
                    case tier14:
                        Fstep1 = Fcurr / (FunctGroupArray[sp].speciesParams[maxmFC_id] * 365.0);  // As Fcurr is annual but mFC is daily
                        F_rescale = Fstep1 * (FTARG / (Fcurr + small_num));  // Re-scale existing F (need to do vs mFC rather than just Fcurr as code applies it against mFC)
                        break;
                    case tier9: // Norwegian way of doing it - to avoid overcatch during spin-up phase
                        Fstep1 = Fcurr / (FunctGroupArray[sp].speciesParams[maxmFC_id] * 365.0);  // As Fcurr is annual but mFC is daily
                        F_rescale = Fstep1 * (FTARG / (FrefA + small_num));
                        break;
                    case tier8:
                        /* In tier 8 version rescaling vs FrefA - although applied vs mFC
                         so just in case the user has not entered FrefA for mFC do a two step rescaling here */
                        this_mFC = FunctGroupArray[sp].speciesParams[maxmFC_id] * 365.0;  // As FrefA is annual but mFC is daily
                        Fstep1 = FrefA / this_mFC;
                        F_rescale = Fstep1 * (FTARG / (FrefA + small_num));  // Re-scale the F
                        break;
                    default:
                        quit("Per_Sp_Frescale: We do not have any code for tier %d\n", tier);
                        break;
                }
            }
            
            if (bm->checkstart) {
                fprintf(llogfp, "Time: %e %s F_rescale: %e, FTARG: %e, Fcurr: %e)\n", bm->dayt, FunctGroupArray[sp].groupCode, F_rescale, FTARG, Fcurr);
            }

            /* If this is the first year then don't do the scaling */
            for (nf = 0; nf < bm->K_num_fisheries; nf++) {
                /* Only change management in fisheries where management active */
                if (bm->FISHERYprms[nf][manage_on_id] < 1)
                    continue;

                flagF = (int) (bm->SP_FISHERYprms[sp][nf][flagF_id]);
                if (flagF) {

                    if(!tier){
                        bm->SP_FISHERYprms[sp][nf][mFC_scale_id] = 1.0;
                    } else {
                        bm->SP_FISHERYprms[sp][nf][mFC_scale_id] = F_rescale;
                    }
                    
                    
                    /* Write out end result of original species focused assessment */
                    WriteAnnBrokenStickFile(bm, sp, nf, tier, FrefLim, FrefA, FrefH, Blim, BrefA, BrefB, Fcurr, FTARG, Bcurr, F_rescale);
                    
                    /* Get fishing mortality - corrected from (day-1) to (s-1) */
                    mFC_start_age = (int) (bm->SP_FISHERYprms[sp][nf][mFC_start_age_id]);
                    mFC_end_age = (int) (bm->SP_FISHERYprms[sp][nf][mFC_end_age_id]);
                    mFC = bm->SP_FISHERYprms[sp][nf][mFC_id] * 365.0;
                    mFC *= bm->SP_FISHERYprms[sp][nf][mFC_scale_id];  // Apply the broken stick scalar from above
                    
                    /* Get scenario scalars */
                    if (bm->flagchangeF){
                        mFC_change_scale = Get_Fishery_Group_Change_Scale(bm, nf,sp, mFC_num_changes_id, mFC_num_changes_id, mFCchange[sp]);
                    } else {
                        mFC_change_scale = 1.0;
                    }
                    
                    /* Correct for presence of mpas */
                    flagfcmpa = (int) (bm->FISHERYprms[nf][flagmpa_id]);
                    if (flagfcmpa)
                        mpa_scale = bm->FISHERYprms[nf][mpascale_cap_id];
                    else
                        mpa_scale = 1.0;

                    /* Allow for infringement */
                    if (bm->flaginfringe) {
                        mpa_infringe = bm->FISHERYprms[nf][infringe_id];
                        if (mpa_infringe > mpa_scale)
                            mpa_scale = mpa_infringe;
                    }

                    sel_curve = (int) (bm->FISHERYprms[nf][selcurve_id]);
                    FunctGroupArray[sp].speciesParams[sp_fishery_expected_catch_id] = 0;
                    for (nc = 0; nc < FunctGroupArray[sp].numCohortsXnumGenes; nc++) {
                        basechrt = nc / FunctGroupArray[sp].numGeneTypes;
                        
                        /* Convert model weight (mg AFDSW) into g wet weight and then length in cm */
                        if (FunctGroupArray[sp].isVertebrate == TRUE) {
                            if (!bm->use_time_avg_wgt) {
                                if (bm->dayt>0 && FunctGroupArray[sp].min_wgt[nc] > MAXDOUBLE) {
                                    quit("Ecosystem_Cap_Frescale: Time %e min_wgt for %s-%d is larger than is feasible (%e) something has gone wrong\n", bm->dayt, FunctGroupArray[sp].groupCode, nc, FunctGroupArray[sp].min_wgt[nc]);
                                }
                                if (bm->dayt>0 && FunctGroupArray[sp].max_wgt[nc] < MINDOUBLE) {
                                    quit("Ecosystem_Cap_Frescale: Time %e max_wgt for %s-%d is smaller than is feasible (%e) something has gone wrong\n", bm->dayt, FunctGroupArray[sp].groupCode, nc, FunctGroupArray[sp].max_wgt[nc]);
                                }
                                Wgt = (FunctGroupArray[sp].min_wgt[nc] + FunctGroupArray[sp].max_wgt[nc]) / 2.0;
                            } else {
                                Wgt = FunctGroupArray[sp].rolling_wgt[nc][bm->K_rolling_cap_num];
                            }
                            if (Wgt < small_num)
                                Wgt = small_num; /* To avoid divide by zero problems  */
                            li = Ecology_Get_Size(bm, sp, Wgt, nc);
                        } else {
                            Wgt = 1.0;
                            li = Ecology_Get_Size(bm, sp, Wgt, nc);
                        }
                        
                        stage = FunctGroupArray[sp].cohort_stage[nc];
                        /* In the case where using selectivity to determine which ages suffer the fishing mortality */
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
                        
                        /* Find what the predicted catch would be by applying fishing mortality */
                        if (!bm->syst_cap_calc_method) {
                            // Bring in M
                            switch (bm->M_est_method) {
                                case fixed_input_M:
                                    calcM = FunctGroupArray[sp].speciesParams[assess_nat_mort_id];
                                    break;
                                case Z_and_F_based:
                                    this_start = 0.0;
                                    this_end = 0.0;
                                    if (FunctGroupArray[sp].isVertebrate == TRUE) {
                                        this_Num = 0.0;
                                        for (k = 0; k < maxstock; k++) {
                                            this_start += bm->calcTrackedMort[sp][nc][k][start_id];
                                            this_end += bm->calcTrackedMort[sp][nc][k][endNum_id];
                                        }
                                    } else {
                                        this_Biom = 0.0;
                                        for (k = 0; k < maxstock; k++) {
                                            this_start += bm->calcTrackedMort[sp][nc][k][start_id];
                                            this_end += bm->calcTrackedMort[sp][nc][k][endNum_id];
                                        }
                                    }
                                    Z_Est = 1.0 - (this_end / (this_start + small_num));
                                    if (Z_Est < Fcurr) {
                                        warn("Time: %e for %s - Z calculation too small versus number caught so set to F as default in Ecosystem_Cap_Frescale\n", bm->dayt, FunctGroupArray[sp].groupCode);
                                        Z_Est = Fcurr;
                                    }
                                    calcM = Z_Est - Fcurr;
                                    break;
                                case full_dynamic_M_est:
                                    calcM = 0.0;
                                    counter = 0.0;
                                    for (k = 0; k < maxstock; k++) {
                                        calcM += (bm->calcTrackedMort[sp][nc][k][finalM1_id] + bm->calcTrackedMort[sp][nc][k][finalM2_id]);
                                        counter++;
                                    }
                                    
                                    calcM /= counter;
                                    break;
                                case assess_M_est:
                                    if(!do_assess) {
                                        quit("Ecosystem_Cap_Frescale: For %s Can't use option %d for M_est_method if do_assess = 0\n", FunctGroupArray[sp].groupCode, assess_M_est);
                                    } else {
                                        M = bm->NAssess[sp][est_M_id];
                                    }
                                default:
                                    quit("No such option for syst_cap_calc_method. Please set to 0 for fixed M (read in from harvest.prm), 1 for M = Z-F, 2 for fully dynamic estimation of M given in model predation, or 3 for M estimated in an assessment\n");
                            }
                            
                            
                            
                            // Apply Baranov catch equation by cohort
                            expectF = mFC * mpa_scale * mFC_change_scale * sel;
                            Catch_Eqn_Denom = expectF + calcM;
                            survival = 1.0 - exp(-1.0 * Catch_Eqn_Denom);
                            
                            // Assume all stocks and genetypes being pooled to get the Numbers at age
                            // TODO: Do per Stock and geneotype separately
                            if(expectF > 0) { // Only do this if non-zero F to avoid time wasting loops
                                if (FunctGroupArray[sp].isVertebrate == TRUE) {
                                    this_Num = 0.0;
                                    for (k = 0; k < maxstock; k++) {
                                        this_Num += bm->calcTrackedMort[sp][nc][k][start_id];
                                    }
                                    this_Biom = (Wgt * bm->X_CN * mg_2_tonne) * this_Num;
                                } else {
                                    this_Biom = 0.0;
                                    for (k = 0; k < maxstock; k++) {
                                        this_Biom = bm->calcTrackedMort[sp][nc][k][start_id] * bm->X_CN * mg_2_tonne;
                                    }
                                }
                                this_expect_catch = this_Biom * (expectF / Catch_Eqn_Denom) * survival;
                                tot_expect_catch += this_expect_catch;
                            }
                        } else {
                            if (!bm->use_time_avg_biom) {
                                if (bm->dayt>0 && FunctGroupArray[sp].min_B[nc] > MAXDOUBLE) {
                                    quit("Ecosystem_Cap_Frescale: Time %e min_B for %s-%d is larger than is feasible (%e) something has gone wrong\n", bm->dayt, FunctGroupArray[sp].groupCode, nc, FunctGroupArray[sp].min_B[nc]);
                                }
                                if (bm->dayt>0 && FunctGroupArray[sp].max_B[nc] < MINDOUBLE) {
                                    quit("Ecosystem_Cap_Frescale: Time %e max_B for %s-%d is smaller than is feasible (%e) something has gone wrong\n", bm->dayt, FunctGroupArray[sp].groupCode, nc, FunctGroupArray[sp].max_B[nc]);
                                }
                                this_Biom = (FunctGroupArray[sp].min_B[nc] + FunctGroupArray[sp].max_B[nc]) / 2.0;
                            } else {
                                this_Biom = FunctGroupArray[sp].rolling_B[nc][bm->K_rolling_cap_num];
                            }
                            this_expect_catch = mFC * mpa_scale * mFC_change_scale * sel * this_Biom * bm->X_CN * mg_2_tonne;
                            tot_expect_catch += this_expect_catch;
                        }
                        FunctGroupArray[sp].speciesParams[sp_fishery_expected_catch_id] += this_expect_catch;
                    }
                }
            }
        }
    }
    
    /* Compare tot_expect_catch vs the total system expected catch - if in excess then rescale using preferential weighting */
    if (tot_expect_catch > bm->Ecosystm_Cap_tonnes ) {
        excess = tot_expect_catch - bm->Ecosystm_Cap_tonnes;
        
        for (sp = 0; sp < bm->K_num_tot_sp; sp++) {
            if (FunctGroupArray[sp].isFished == TRUE) {
                
                // Follow-through for sp that are not part of OY
                if((!FunctGroupArray[sp].speciesParams[flagFonly_id]) || (!FunctGroupArray[sp].speciesParams[flag_systcap_sp_id]))
                    continue;
                /* Rescale excess based on these weigths * */
                deductions = excess * FunctGroupArray[sp].speciesParams[sp_fishery_pref_norm_id];
                FunctGroupArray[sp].speciesParams[sp_fishery_deduction_id] = deductions;
                
                orig_expected_catch = FunctGroupArray[sp].speciesParams[sp_fishery_expected_catch_id];
                if (orig_expected_catch) { // only both of expected catch is non-zero
                    new_expected_catch = orig_expected_catch - deductions;
                    rescale_scalar = (new_expected_catch / orig_expected_catch);
                    for (nf = 0; nf < bm->K_num_fisheries; nf++) {
                        bm->SP_FISHERYprms[sp][nf][orig_mFC_scale_id] = bm->SP_FISHERYprms[sp][nf][mFC_scale_id]; // For reporting purposes
                        bm->SP_FISHERYprms[sp][nf][mFC_scale_id] *= rescale_scalar;
                    }
                }
            }
        }
    }
    
    /* Write out end result */
    for (sp = 0; sp < bm->K_num_tot_sp; sp++) {
        if (FunctGroupArray[sp].isFished == TRUE) {
            for (nf = 0; nf < bm->K_num_fisheries; nf++) {
                flagF = (int) (bm->SP_FISHERYprms[sp][nf][flagF_id]);
                if (flagF) {
                    WriteAnnCapResultFile(bm, sp, nf);
                }
            }
        }
    }
    
    return;
}


/**************************************************************************//**
 *	When using F rates for the fishing model, may still want a harvest control
 *  rule based on other species in the system. In this case use this routine
 *
 *****************************************************************************/

void Check_Ecosystem_F_Harvest_Control_Rule(MSEBoxModel *bm, FILE *llogfp){
	int sp, i, tier, nf, flagF, co_sp;
	double FC_thresh, fishnowpop, F_rescale;

	if(bm->flagendangered){
		// Don't need to reset mFC_scale_id to 1.0 as done in Check_F_Harvest_Control_Rule()

		for(sp=0; sp<bm->K_num_tot_sp; sp++){
			if(FunctGroupArray[sp].isImpacted == TRUE){
				if(FunctGroupArray[sp].speciesParams[sp_concern_id]){
					fishnowpop = (bm->totfishpop[sp] * bm->X_CN * mg_2_tonne) / (bm->estinitpop[sp] + small_num);

					for(nf=0; nf<bm->K_num_fisheries; nf++){
						/* Only change management in fisheries where management active */
						if(bm->FISHERYprms[nf][manage_on_id] < 1)
							continue;

						FC_thresh = bm->SP_FISHERYprms[sp][nf][FC_thresh_id];

						// If species of concern less than threshold value down scale effort of relevant fisheries
						if(fishnowpop < FC_thresh){
                            for (i = 0; i < FunctGroupArray[sp].speciesParams[max_co_sp_id]; i++ ) {
								co_sp = (int)(FunctGroupArray[sp].co_sp[i]);
                                
                                if ((co_sp < 0) || (co_sp > bm->K_num_tot_sp))
                                    continue;

								/* Check to see that want to use a harvest control rule */
								tier = (int)(FunctGroupArray[co_sp].speciesParams[tier_id]);
								flagF = (int)(bm->SP_FISHERYprms[co_sp][nf][flagF_id]);
								if((flagF) && (tier == tier7)){
									/* if this is the first year then don't do the scaling */
                                    F_rescale = bm->FISHERYprms[nf][FC_restrict_id];
                                    bm->SP_FISHERYprms[co_sp][nf][mFC_scale_id] *= F_rescale;
                                    // Do the multiplication here so don't overwrite anything done in Check_F_Harvest_Control_Rule()
                                    // May need to change this to make it minimum required change not cumulative change
								}
							}
						}
					}
				}
			}
		}
	}

	return;
}

/**
 * \brief Apply any changes in management levers - for US-like MSY-based (tier6) fisheries
 * All relevant info stored in LeverQueue.
 */
void Do_MSY_Management(MSEBoxModel *bm, FILE *llogfp) {
	int sp, nf, use_lever, update_scale = 0, update_season = 0, fishstartday, fishendday, season_length, newfishendday, tempendday;
	double new_scale = 0.0, avg_scale = 0.0, num_scale = 0.0, min_scale = 0.0, lever_scale = 0.0;

	/* If not doing MSY related management leave routine */
	if (!bm->do_MSY)
		return;

	/* MSY-based management updating */
	for (nf = 0; nf < bm->K_num_fisheries; nf++) {
		new_scale = 1.0;
		avg_scale = 0.0;
		num_scale = 0.0;
		min_scale = MAXDOUBLE;
		bm->FISHERYprms[nf][use_msy_effort_id] = 0;
		for (sp = 0; sp < bm->K_num_tot_sp; sp++) {
			if (FunctGroupArray[sp].isFished == TRUE) {
				use_lever = (int) (FunctGroupArray[sp].speciesParams[use_lever_id]);
				lever_scale = LeverQueue[sp][nf][bm->thisyear];

				switch (use_lever) {
				case use_none: /* Nothing to do */
					break;
				case use_effort: /* Effort change */
				case use_season: /* Season change */
				case use_space: /* Spatial management change */
					break;
				case use_catch: /* Quota change */
					bm->TACamt[sp][nf][now_id] *= lever_scale;
					break;
				case use_gear: /* Gear change */
					bm->SP_FISHERYprms[sp][nf][q_id] *= lever_scale;
					break;
				default:
					quit("No such lever for MSY based management (must be between %d for no lever to %d to for changing gear\n", use_none, use_gear);
					break;
				}
				switch (use_lever) {
				case use_none: /* Nothing to do */
				case use_catch: /* Quota change */
				case use_gear: /* Gear change */
					break;
				case use_effort: /* Effort change */
					bm->FISHERYprms[nf][use_msy_effort_id] = 1;
					update_scale = 1;
					break;
				case use_season: /* Season change */
					update_season = 1;
					break;
				case use_space: /* Spatial management change */
					update_scale = 1;
					break;
				default:
					quit("No such lever for MSY based management (must be between %d for no lever to %d to for changing gear\n", use_none, use_gear);
					break;
				}
			}

		}
		/* Find average rescale necessary */
		if (num_scale > 0.0)
			avg_scale /= num_scale;

		if (bm->FISHERYprms[nf][use_min_lever_id])
			new_scale = min_scale;
		else
			new_scale = avg_scale;

		/* Update effort and spatial management */
		if (update_scale) {
			effort_scale[nf][target_tac_id] = new_scale;
		}

		/* Update season */
		if (update_season) {
			fishstartday = (int) (bm->FISHERYprms[nf][seasonopen_id]);
			fishendday = (int) (bm->FISHERYprms[nf][seasonclose_id]);
			SEASONAL[nf][0] = fishstartday;
			if (fishstartday < fishendday)
				season_length = fishendday - fishstartday;
			else
				season_length = fishendday + 364 - fishstartday;

			newfishendday = (int) floor(ROUNDGUARD + (season_length * new_scale));
			newfishendday += fishstartday;

			/* If season starts in one year and finish in the next */
			if (newfishendday > 364)
				newfishendday = newfishendday - 364;
			tempendday = (int) (oldFishEndDay[nf]);
			if (tempendday > 0) {
				SEASONAL[nf][1] = tempendday;
				oldFishEndDay[nf] = newfishendday;
			} else {
				/* If season all in one year */
				SEASONAL[nf][1] = newfishendday;
				oldFishEndDay[nf] = 0;
			}
		}
	}
	return;
}


#ifdef RASSESS_LINK_ENABLED
void Check_R_management(MSEBoxModel *bm, FILE *llogfp) {

	if(bm->usingRedus_R_HCR) {
        REDUS_management(bm, llogfp);
	}

	return;
}
#endif
