/**
 \file
 \brief C file for effort allocation for economic models for Atlantis
 \ingroup ateconomic

 File:	ateconeffort.c
 Author:	Beth Fulton
 Created:	10/5/2006
 Purpose:	C file for effort allocation for economic models for Atlantis
 Revisions: 10/5/2006 Created the file (to split it away from ateconresponse.c)

 22/8/2006 Added check that only playing with effort allocation for
 fisheries using economically based effort model

 6/9/2006 To make loops faster skip over subfleets with no boats and fisheries
 where can't catch that actual group (i.e. when q == 0)

 7/9/2006 Initialisation of marginal rents and boatcatch necessary so not
 getting ridiculously small or large effort contributions

 8/9/2006 Made the currency of the economic model kg (which meant the TAC had
 to be updates as was previously in tonnes). This means that BlacBook values
 are to be in kg, saleprice is in $/kg and QuotaAlloc values are in kg.

 25/11/2006 Went from the option of rescaling all months to current month only.

 20/2/2007 Added Dan Holland effort allocation model - using bm->MultiPlanEffort
 to direct code to correct pathway. The bulk of the code is in Allocate_Final_Effort()

 14-04-2008 Beth Fulton
 Caught an sp call outside of a loop (so was greater than size of array) and initialisation
 that should have been in a loop that was being done just before it. Both in Dan Holland's code.

 15-04-2008 Beth Fulton
 When first calculating expected catch added += as = only set it to last value. Also
 removed (totland > maxland) check for BiTac cas of correcting for maximum allowable catch.

 16-04-2008 Beth Fulton
 In Dan Holland code: fixed marginal rent calculation (made price calc species specific contributions),
 made initial catch distribution in terms of kg not mg

 6-06-2008 Beth Fulton
 In Dan Holland code: capped the catch that can be expected based on hold size (quotas didn't work if
 species not under quota). Also changed ECONexprice calculation to market - quota price so don't
 get inappropriate compounding losses.

 28-10-2009 Bec Gorton
 Removed references to the K_num_fished_sp and K_num_impacted_sp. Arrays that were this
 size have been resized to K_num_tot_sp.

 04-11-2009 Bec Gorton
 Merged in Beths new bycatch incentive code.

 7-04-2010 Beth Fulton
 Taxes included in effort calculations
 **************/

/*  Open library routines */
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <sjwlib.h>
#include <atEconomic.h>

static void Multi_Plan_Effort_Final_Allocation(MSEBoxModel *bm, FILE *llogfp);
static void Holland_Effort_Final_Allocation(MSEBoxModel *bm, FILE *llogfp);

/**
 *
 *	\brief Monthly Effort Allocation
 *
 *	Resets any monthly stuff. Doesn't do it if we are replaying history.
 *
 *	Looks at what is the ratio of expected catch of rest of year and what its already caught
 *	Looks are targeting information - what group its going to target. What its catching the most of its going to more heavily weight.
 *
 *	Weights by the number of species you have. More species means effort can be spread.
 *
 *	Effort increased are capped to you can't go to high to quickly .
 *
 *	Will recalculated deemed values and taxes.
 *
 *	If you do need to rescale effort for the remaining months of year.
 *		For remaining months
 *		 	What is expected catch and bycatch and recalculates the effort for each month.
 *			This is similar to the annual effor calculations.
 *
 *	Tax and deemed value takes into account bycatch and catch over quota.
 *
 *	Then created new expected effort. What does this mean cost wise, will i be in too much debt.
 *
 *	Then checks for it need to do to rescale effort - just magnitude of effort per month.
 *	Expected profits, and check effort isn't too high.
 *	Is debt too high identify boats that are to high. If past amount of time debt is tolerated boat is take out
 *	of the fleet. Proportion of the fleet is being tied up not an actual boat. This is prorata step.
 *	Assumption is that boats will go out lots and some will tie up.
 *
 *
 *	Rescale spatial effort. Use same spatial proportion but rescale the values in each box.
 *
 *
 */
void Monthly_Effort_Schedule(MSEBoxModel *bm, FILE *llogfp) {
	int nf, ns, month, sp, ij, nt, do_debug_base, do_debug, boats_tied_up, boats_semi_active, redo_needed = 0, ntarg;
	double max_eff_ratio, max_month_effort, cumcatch, tolerable_debt = 0, expectedcatch, eff_ratio, effort_adjust, down_time, choice_buffer,
			effort_correction, check_time, step1 = 0, ans, ExpEffort, oldtotpi = 0, this_profit, totppi, TripCost, potential_totppi = 0, stepE;

	/* If no monthly effort plan skip ahead */
	if (!bm->MultiPlanEffort)
		return;

	if (((bm->debug == debug_econeffort) || (bm->debug == debug_effort)) && (bm->dayt > bm->checkstart)) {
		do_debug_base = 1;
	} else
		do_debug_base = 0;

	if (verbose > 1)
		printf("Calculating monthly effort allocation\n");

	/* Updated effort expectations - beginning in second month of the year.
	 If start in first month there is nothing to update
	 */
	if ((bm->dayt > 1) && (!bm->hist_only)) {
		for (nf = 0; nf < bm->K_num_fisheries; nf++) {
			if (bm->FISHERYprms[nf][flageffortmodel_id] > rec_econ_model) {
				if (do_debug_base && ((bm->which_fleet == nf) || (bm->which_fleet >= bm->K_num_fisheries))) {
					do_debug = 1;
				} else
					do_debug = 0;

				max_eff_ratio = 0;
				for (month = bm->MofY; month < 12; month++) {
					old_effort[month] = 0;
					new_effort[month] = 0;
					redo_effort[month] = 0;
				}

				redo_needed = 0;
				for (ns = 0; ns < bm->FISHERYprms[nf][nsubfleets_id]; ns++) {
					/* If no boats in the subfleet currently skip ahead */
					if (!bm->SUBFLEET_ECONprms[nf][ns][nboat_id]) {
						continue;
					}

					//TripCost = bm->SUBFLEET_ECONprms[nf][ns][cost_ind_id];		replaced with pereffort_costs as multiply by exepcted effort later on
					TripCost = bm->SUBFLEET_ECONprms[nf][ns][pereffort_cost_ind_id];
					down_time = bm->SUBFLEET_ECONprms[nf][ns][down_time_id];
					max_month_effort = bm->month_scalar * (1 - down_time) * bm->SUBFLEET_ECONprms[nf][ns][nboat_id];

					ntarg = (int) (bm->FISHERYprms[nf][ntargets_id]);
					/* Determine of actual catch matching expected catch for targetted groups */
					for (sp = 0; sp < bm->K_num_tot_sp; sp++) {

						/* If current species actually set to be beyond fished group list then let it go the
						 first time by skipping ahead as if happens again will be caught by the last_sp check */

						if (FunctGroupArray[sp].isFished == FALSE || bm->SP_FISHERYprms[sp][nf][q_id] == FALSE)
							continue;

						/** First to see where expected catch and rent is sitting **/
						/* Actually caught */
						cumcatch = bm->QuotaAlloc[nf][ns][sp][cumboatcatch_id];

						/* Expected to catch */
						expectedcatch = 0;
						for (month = 0; month < bm->MofY; month++) {
							expectedcatch += bm->MonthAlloc[nf][ns][sp][month];

							if (do_debug) {
								fprintf(llogfp, "Time: %e, %s-%d %s mth: %d, MonthAlloc: %e expectedcatch: %e\n", bm->dayt, FisheryArray[nf].fisheryCode, ns,
										FunctGroupArray[sp].groupCode, month, bm->MonthAlloc[nf][ns][sp][month], expectedcatch);
							}
						}

						/* Are we at where we expected to be in the year */
						/* Scaling ratio */
						if (!cumcatch)
							eff_ratio = 1;
						else
							eff_ratio = expectedcatch / (cumcatch + small_num);

						/* Correct for targeting priority - primary target weighted highly, bycatch (groups lower on target
						 list have very low weighting
						 */
						nt = bm->SUBFISHERYtarget[nf][ns][sp];
						//if(!nt)
						//	nt = ntarg;  This test is no longer required as 0 is the best ranked species so resetting to ntarg does not make sense

						eff_ratio *= (1 / ((nt + 1.0) * (nt + 1.0)));

                        /*
						if (do_debug) {
							fprintf(llogfp, "Time: %e, %s-%d %s mth: %d, eff_ratio: %e, expectedcatch: %e, cumcatch: %e\n", bm->dayt,
									FisheryArray[nf].fisheryCode, ns, FunctGroupArray[sp].groupCode, bm->MofY, eff_ratio, expectedcatch, cumcatch);
						}
                        */
                        
						/* Find maximum scalar */
						if (eff_ratio > max_eff_ratio)
							max_eff_ratio = eff_ratio;

						/* If there are deemed values then check to see if need to update expectations */
						if (bm->QuotaAlloc[nf][ns][sp][DVtriggered_id] || bm->QuotaAlloc[nf][ns][sp][TAXtriggered_id]) {
							redo_needed = 1;
						}
					}

					/** Redo profit calculations with deemed values - if necessary **/
					this_profit = 1.0;
					if (redo_needed) {
						for (month = bm->MofY; month < 12; month++) {
							totppi = 0; /* Total expected profit */
                            if(bm->MonthlyProfitUpdate) {
                                oldtotpi = 0; /* Update monthly, without this on it is only upating annually */
                            }
							ExpEffort = bm->EffortSchedule[nf][ns][month][expect_id];
							for (sp = 0; sp < bm->K_num_tot_sp; sp++) {
								if (FunctGroupArray[sp].isFished == TRUE) {
									/* If can't catch the group skip ahead */
									if (!bm->SP_FISHERYprms[sp][nf][q_id])
										continue;

									ans = Get_Expected_Profit(bm, nf, ns, sp, month, ntarg, ExpEffort, "MonthCheck", do_debug, llogfp);

									oldtotpi += pi[nf][ns][month][sp]; // Get the sum of the old values before you update them (one at a time)
									pi[nf][ns][month][sp] = ans;
									totppi += pi[nf][ns][month][sp]; // Sum of new values
								}
							}

							if (do_debug) {
								fprintf(llogfp, "Time: %e, %s-%d month %d starting totppi: %e ", bm->dayt, FisheryArray[nf].fisheryCode, ns, month, totppi);
							}

							/* If using total catch - total cost model apply costs to get profitability */
							if (!bm->OrigEconCalc) {

								tolerable_debt = -1.0 * bm->SUBFLEET_ECONprms[nf][ns][tol_debt_id];
								potential_totppi = totppi - (TripCost * ExpEffort);

								if (potential_totppi < tolerable_debt) {
									/* Check to see if can contract effort and if still leads to too high debt
									 level then don't fish at all */

									if (do_debug) {
										fprintf(llogfp, "Time: %e %s-%d contracting ExpEffort from %e ", bm->dayt, FisheryArray[nf].fisheryCode, ns, ExpEffort);
									}
									stepE = ExpEffort;
									ExpEffort = (totppi - tolerable_debt) / (TripCost + small_num);
									if (ExpEffort < 0.0)
										ExpEffort = 0.0;

									totppi = oldtotpi * ExpEffort / (stepE + small_num);

								} else if (potential_totppi < 0.0) {
									/* Else just make sure contract effort appropriately */
									totppi *= (oldtotpi / (TripCost * ExpEffort));
								} else
									totppi -= (TripCost * ExpEffort);
							}

							/* Calculate the prop of this profit compared to past profit */
							this_profit = totppi / (oldtotpi + small_num);

							if (do_debug) {
								fprintf(
										llogfp,
										"ends with totppi: %e (TripCost: %e, ExpEffort: %e, totcost: %e, tolerable_debt: %e, potential_totppi: %e) lastpi: %e, this_profit: %e\n",
										totppi, TripCost, ExpEffort, TripCost * ExpEffort, tolerable_debt, potential_totppi, oldtotpi, this_profit);
							}

						}
					}
					redo_needed = 0;

					/* Only reduce effort if applicable to model. Unless regulated otherwise, some models
					 assume fishers will always fish their hardest (max_eff_ratio > 1 always - set choice_buffer = 1),
					 or at least only reduce their effort to a certain degree (max_eff_ratio > choice_buffer),
					 rather than reduce their effort to fully compensate for any observed catch > expected catch.

					 */
					choice_buffer = (int) (bm->SUBFLEET_ECONprms[nf][ns][choicebuffer_id]);
					if (max_eff_ratio < (1 - choice_buffer))
						max_eff_ratio = (1 - choice_buffer);

					/** Check for rescaling **/
					for (month = bm->MofY; month < 12; month++) {
						/* If only rescaling current month skip ahead once that has been done */
						if ((month > bm->MofY) && !bm->flagallmonths)
							break;

						old_effort[month] += bm->EffortSchedule[nf][ns][month][expect_id];

						if (do_debug) {
							fprintf(
									llogfp,
									"Time: %e, %s-%d month %d EffortSchedule: %e, max_effort_ratio: %e, max_month_effort: %e (downtime: %e nboat: %e), this_profit: %e\n",
									bm->dayt, FisheryArray[nf].fisheryCode, ns, month, bm->EffortSchedule[nf][ns][month][expect_id], max_eff_ratio,
									max_month_effort, down_time, bm->SUBFLEET_ECONprms[nf][ns][nboat_id], this_profit);
						}

						/* Future effort increase to compensate for current under achievement - if appropriate  */
						if (max_eff_ratio > 0) {
							step1 = min(max_month_effort, max_eff_ratio * bm->EffortSchedule[nf][ns][month][expect_id]);
							redo_needed = 1;

							if (do_debug) {
								fprintf(llogfp, "Time: %e, %s-%d mth %d step1 %e, max_month_effort: %e, max_eff_ratio: %e, expect_effort: %e\n", bm->dayt,
										FisheryArray[nf].fisheryCode, ns, month, step1, max_month_effort, max_eff_ratio,
										bm->EffortSchedule[nf][ns][month][expect_id]);
							}
						}

						/* Reacting to deemed value adjustment overrides all else */
						if (this_profit != 1.0) {
							step1 = min(max_month_effort, this_profit * bm->EffortSchedule[nf][ns][month][expect_id]);
							redo_needed = 1;

							if (do_debug) {
								fprintf(llogfp, "Time: %e, %s-%d mth %d step1 %e, max_month_effort: %e, this_profit: %e, expect_effort: %e\n", bm->dayt,
										FisheryArray[nf].fisheryCode, ns, month, step1, max_month_effort, this_profit,
										bm->EffortSchedule[nf][ns][month][expect_id]);
							}
						}

						if (redo_needed) {
							/* Adjust the expected profit ?*/
							bm->EffortSchedule[nf][ns][month][expect_id] = step1;
							new_effort[month] += bm->EffortSchedule[nf][ns][month][expect_id];
							redo_effort[month] = 1;

							if (bm->EffortSchedule[nf][ns][month][expect_id] < 0) {
								fprintf(llogfp, "Time: %e, %s-%d month %d zeroed EffortSchedule: %e, max_effort_ratio: %e\n", bm->dayt,
										FisheryArray[nf].fisheryCode, ns, month, bm->EffortSchedule[nf][ns][month][expect_id], max_eff_ratio);
								bm->EffortSchedule[nf][ns][month][expect_id] = 0;
							}

							if (max_eff_ratio < 1) {
								fprintf(llogfp, "Time: %e, %s-%d month %d lowering EffortSchedule: %e, max_effort_ratio: %e\n", bm->dayt,
										FisheryArray[nf].fisheryCode, ns, month, bm->EffortSchedule[nf][ns][month][expect_id], max_eff_ratio);
							}

							if (do_debug) {
								fprintf(llogfp, "Time: %e, %s-%d mth %d new_effort: %e step1 %e\n", bm->dayt, FisheryArray[nf].fisheryCode, ns, month,
										new_effort[month], step1);
							}

						} else {
							new_effort[month] += bm->EffortSchedule[nf][ns][month][expect_id];
						}

						/* Check for sanity of effort level */
						if (bm->EffortSchedule[nf][ns][month][expect_id] > max_month_effort) {
							effort_adjust = bm->EffortSchedule[nf][ns][month][expect_id] - max_month_effort;
							bm->EffortSchedule[nf][ns][month][expect_id] = max_month_effort;
							new_effort[month] -= effort_adjust;
							redo_effort[month] = 1;

							if (do_debug) {
								fprintf(llogfp, "Time: %e, %s-%d mth %d new_effort: %e effort_adjust %e\n", bm->dayt, FisheryArray[nf].fisheryCode, ns, month,
										new_effort[month], effort_adjust);
							}
						}

					}

					/** If profit less than tolerable loss (i.e. negative rent) then don't fish this month */
					tolerable_debt = -1.0 * bm->SUBFLEET_ECONprms[nf][ns][tol_debt_id];
					if (tot_marg_rent[nf][ns] < tolerable_debt) {
						effort_correction = bm->effort_reduction * bm->EffortSchedule[nf][ns][bm->MofY][expect_id];
						bm->EffortSchedule[nf][ns][bm->MofY][expect_id] *= (1 - bm->effort_reduction);
						new_effort[bm->MofY] -= effort_correction;
						bm->SUBFLEET_ECONprms[nf][ns][months_crippled_id] += 1;
						redo_effort[bm->MofY] = 1;

						if (do_debug) {
							fprintf(llogfp, "Time: %e, %s-%d month :%d, tot_marg_rent: %e so new_effort %e, effort_correction: %e\n", bm->dayt,
									FisheryArray[nf].fisheryCode, ns, bm->MofY, tot_marg_rent[nf][ns], new_effort[bm->MofY], effort_correction);
						}

					} else {
						if (bm->SUBFLEET_ECONprms[nf][ns][months_crippled_id] > bm->cripple_period) {
							bm->SUBFLEET_ECONprms[nf][ns][cripples_lost_id] += 1;
						}
						bm->SUBFLEET_ECONprms[nf][ns][months_crippled_id] = 0;
					}

					/** Rescale so final spatial effort reset based on trying to play catch up while being
					 constrained to one day fished per boat per day in the fishery **/
					for (month = bm->MofY; month < 12; month++) {
						/* If only rescaling current month skip ahead once that has been done */
						if ((month > bm->MofY) && !bm->flagallmonths)
							break;

						if (do_debug) {
							fprintf(llogfp, "Time: %e, %s-%d mth: %d, overall old_effort: %e new_effort: %e\n", bm->dayt, FisheryArray[nf].fisheryCode, ns,
									month, old_effort[month], new_effort[month]);
						}

						if (redo_effort[month]) {
							for (ij = 0; ij < bm->nbox; ij++) {
								if (bm->boxes[ij].type != BOUNDARY) {
									bm->Effort_hdistrib[ij][nf][month] *= (new_effort[month] / (old_effort[month] + small_num));

									if (do_debug) {
										fprintf(llogfp, "Time: %e, %s-%d mth: %d, Effort_hdistrib-%d: %e, old_effort: %e new_effort: %e\n", bm->dayt,
												FisheryArray[nf].fisheryCode, ns, month, ij, bm->Effort_hdistrib[ij][nf][month], old_effort[month],
												new_effort[month]);
									}
								}
							}
						}
					}

					/* Check for tied-up boats */
					check_time = min((3.0 * bm->SUBFLEET_ECONprms[nf][ns][nboat_id]), max_month_effort);
					if (bm->EffortSchedule[nf][ns][bm->MofY][expect_id] < check_time) {
						/* Find number of single trips possible */
						step1 = bm->EffortSchedule[nf][ns][bm->MofY][expect_id] / (bm->SUBFLEET_ECONprms[nf][ns][max_trip_length_id] + small_num);
						boats_semi_active = (int) (ceil(step1));
						/* Boats tied up are those that couldn't even get a single trip in,
						 with a minimum of 1 once check tripped
						 */
						boats_tied_up = (int) (bm->SUBFLEET_ECONprms[nf][ns][nboat_id]) - boats_semi_active;
						if (boats_tied_up < 1)
							boats_tied_up = 1;
						if (!bm->MofY) {
							bm->SUBFLEET_ECONprms[nf][ns][tied_up_id] = boats_tied_up;
							bm->SUBFLEET_ECONprms[nf][ns][mth_tiedup_id] = 1;
						} else {
							bm->SUBFLEET_ECONprms[nf][ns][tied_up_id] = (bm->SUBFLEET_ECONprms[nf][ns][tied_up_id] + boats_tied_up) / 2.0;
							bm->SUBFLEET_ECONprms[nf][ns][mth_tiedup_id] += 1;
						}
					}
				}
			}
		}
	}

	/* Write out updated schedule */
	Economic_Output_Effort_Schedule(bm);

	return;
}

/**
 *
 *	Weekly Effort Allocation
 *
 *	Does final effort allocation for the trip.
 *	Only does for subfleet that have boats.
 *	Caps effort so you can only fish num of days in month.
 *
 *	Find species where there is quota left.
 *	Is expected effort all done - stop fishing for month.
 *
 *	If there is quota in hand and there is effort left and last week in month then it will
 *	decide to fish all week.
 *
 *	Then allocate effort
 *		looks at CPUE, discards per UF and value per UE.
 *
 *		Then to actually
 *			either uses catch or effort.
 *			Gets initially allocated  = relative CPUE in that box in that month - immediate value.
 *			Decides where to put effort and scales by MPA - prop of box open to fishing.
 *			Compliance is taken into account in harvest not at this stage.
 *
 *			If using spatial non-compliance. Hook penalty. This is when that will come in. Will try to weight away from
 *			places where we are getting lots of discards. If there is lots of discards then will have to pay deemed values or taxes.
 *			places
 *
 *		Once done allocation look at fuel costs. Will downweight locations a long distance away. They have to take into account steaming
 *		and costs. Its assumes that they will fish along the way.
 *
 *		Things are then normalised to later.
 *		Stores into for later.
 *
 *		Effort units are days per day. (Days allows for multiple boats in a subfleet).
 */
void Allocate_Final_Effort(MSEBoxModel *bm, FILE *llogfp) {
	int nf, ij, ns, do_debug_base, do_debug, mth;
	double toteffort;

	if (((bm->debug == debug_econeffort) || (bm->debug == debug_effort)) && (bm->dayt > bm->checkstart)) {
		do_debug_base = 1;
	} else
		do_debug_base = 0;

	if (verbose > 1)
		printf("Calculating final effort allocation\n");

	/** Base economic model **/
	if (bm->MultiPlanEffort) {
		Multi_Plan_Effort_Final_Allocation(bm, llogfp);
	} else {
		/** Dan Holland effort model - originally developed for the northern california current **/
		/* First up find cumulative catch per species in participating fisheries */
		Holland_Effort_Final_Allocation(bm, llogfp);
	}

	/* Calculate split-up of relative effort within each box for each subfleet */
	for (ij = 0; ij < bm->nbox; ij++) {

		if (bm->boxes[ij].type != BOUNDARY) {
			for (nf = 0; nf < bm->K_num_fisheries; nf++) {
				toteffort = 0;

				if (do_debug_base && ((bm->which_fleet == nf) || (bm->which_fleet >= bm->K_num_fisheries))) {
					do_debug = 1;
				} else
					do_debug = 0;

				if (bm->FISHERYprms[nf][flageffortmodel_id] > rec_econ_model) {
					for (ns = 0; ns < bm->FISHERYprms[nf][nsubfleets_id]; ns++) {
						toteffort += bm->SpatialBlackBook[nf][ns][bm->MofY][ij][current_id];

						//fprintf(llogfp,"Time: %e, ij: %d, %s-%d, mth: %d, toteffort: %e, bm->SpatialBlackBook[current_id: %e\n",
						//	bm->dayt, ij, FisheryArray[nf].fisheryCode, ns, bm->MofY, toteffort, bm->SpatialBlackBook[nf][ns][bm->MofY][ij][current_id]);

					}

					/* Normalise relative effort per subfleet in each box (for use in Allocate_Catch()) */
					for (ns = 0; ns < bm->FISHERYprms[nf][nsubfleets_id]; ns++) {
						bm->SpatialBlackBook[nf][ns][bm->MofY][ij][current_id] /= (toteffort + small_num);

						if (do_debug) {
							fprintf(llogfp, "Time: %e, ij: %d, %s-%d, mth: %d, bm->SpatialBlackBook[current_id: %e, toteffort: %e\n", bm->dayt, ij,
									FisheryArray[nf].fisheryCode, ns, bm->MofY, bm->SpatialBlackBook[nf][ns][bm->MofY][ij][current_id], toteffort);
						}

						if (bm->MultiPlanEffort) {
							/* Do nothing as current effort updated continuously without discontinuity */
						} else {
							/* For Dan Holland code need to flow the current month's effort through the rest of the year
							 (i.e. fill to end) as it calculates gvp off "current_id" but does it at the start of
							 each month before calculating effort so need the flow through of effort values so have
							 values for effort for gvp to work with
							 */
							for (mth = bm->MofY + 1; mth < 12; mth++) {
								bm->SpatialBlackBook[nf][ns][mth][ij][current_id] = bm->SpatialBlackBook[nf][ns][bm->MofY][ij][current_id];
							}
						}
					}
				}
			}
		}
	}

	return;
}

void Multi_Plan_Effort_Final_Allocation(MSEBoxModel *bm, FILE *llogfp) {

	int ij, nf, ns, sp, end_run, days_left, do_debug_base, do_debug, any_quota, all_done, porti, flagfishhere;
	double down_time, max_month_effort, cumcatch, prop_olease, prop_tlease, totCPUE, totDIS = 0, totVPUE = 0, toteffort, step1, CPUEbased, FCweight, Costscale,
			tempEffort, FCpressure, this_effort, old_realised, permisible_left;

	if (((bm->debug == debug_econeffort) || (bm->debug == debug_effort)) && (bm->dayt > bm->checkstart)) {
		do_debug_base = 1;
	} else
		do_debug_base = 0;

	/* Initialise effort distribution */
	for (ij = 0; ij < bm->nbox; ij++) {
		for (nf = 0; nf < bm->K_num_fisheries; nf++) {
			if (bm->FISHERYprms[nf][flageffortmodel_id] > rec_econ_model) {
				bm->Effort_hdistrib[ij][nf][today_effort] = 0;
				bm->Effort_hdistrib[ij][nf][new_today_effort] = 0;
			}
		}
	}

	/* Determine how close to the end of the month or year currently are */
	if ((bm->TofY > 330) || (bm->DofM > 23)) {
		end_run = 1;
		days_left = 30 - bm->DofM;
		if (days_left < 0)
			days_left = 1;
	} else {
		end_run = 0;
        days_left = 0;
    }

	for (nf = 0; nf < bm->K_num_fisheries; nf++) {
		if (bm->FISHERYprms[nf][flageffortmodel_id] > rec_econ_model) {
			if (do_debug_base && ((bm->which_fleet == nf) || (bm->which_fleet >= bm->K_num_fisheries))) {
				do_debug = 1;
			} else
				do_debug = 0;

			for (ns = 0; ns < bm->FISHERYprms[nf][nsubfleets_id]; ns++) {
				/* If no boats in the subfleet currently skip ahead */
				if (!bm->SUBFLEET_ECONprms[nf][ns][nboat_id]) {
					continue;
				}

				down_time = bm->SUBFLEET_ECONprms[nf][ns][down_time_id];
				max_month_effort = bm->month_scalar * (1 - down_time) * bm->SUBFLEET_ECONprms[nf][ns][nboat_id];

				/* Identify species with quota remaining - for each species
				 check if the subfleet has any remaining quota available.
				 If no quota is available it can't target that group and if
				 there is no available quota for any group then they will
				 not fish.
				 */

				any_quota = 0;
				if (!bm->fish_withoutQ) {
					for (sp = 0; sp < bm->K_num_tot_sp; sp++) {
						if (FunctGroupArray[sp].isFished == TRUE) {
							/* If can't catch the group skip ahead */
							if (!bm->SP_FISHERYprms[sp][nf][q_id])
								continue;
							cumcatch = bm->QuotaAlloc[nf][ns][sp][cumboatcatch_id];

							prop_olease = bm->QuotaAlloc[nf][ns][sp][permolease_id];
							prop_tlease = bm->QuotaAlloc[nf][ns][sp][templease_id];
							if (cumcatch > ((1.0 - prop_olease - prop_tlease) * bm->QuotaAlloc[nf][ns][sp][owned_id] + bm->QuotaAlloc[nf][ns][sp][leased_id])) {
								bm->QuotaAlloc[nf][ns][sp][over_id] = 1;
							} else {
								bm->QuotaAlloc[nf][ns][sp][over_id] = 0;
								any_quota = 1;
							}
						}
					}
				} else
					any_quota = 1;

				/* Check if effort scheduled for the current month and if it is
				 whether the effort scheduled for the month has already been exhausted.
				 If scheduled effort remains continue allocating it.
				 Also if expected return less than fixed rate tie up y% of the subfleet.
				 */
				if (bm->EffortSchedule[nf][ns][bm->MofY][expect_id] < bm->EffortSchedule[nf][ns][bm->MofY][current_id]) {
					all_done = 1;
				} else {
					all_done = 0;
				}

				if (do_debug) {
					fprintf(llogfp, "Time: %e, %s-%d mth: %d all_done: %d, EffortSchedule[expect_id]: %e, EffortSchedule[current_id]: %e\n", bm->dayt,
							FisheryArray[nf].fisheryCode, ns, bm->MofY, all_done, bm->EffortSchedule[nf][ns][bm->MofY][expect_id],
							bm->EffortSchedule[nf][ns][bm->MofY][current_id]);
				}

				/* If quota remains (or quota management not in use) allocate
				 effort level based on expected returns. If this is the last
				 week of the month (or last month of the year) and total catch
				 is still less than allowed quota then the vessel will go
				 beyond what was scheduled
				 */
				if (all_done && any_quota && end_run && (!bm->hist_only)) {
					bm->EffortSchedule[nf][ns][bm->MofY][expect_id] += days_left;
					all_done = 0;
				}

				if (do_debug) {
					fprintf(llogfp, "Time: %e, %s-%d mth: %d all_done: %d, any_quota: %d\n", bm->dayt, FisheryArray[nf].fisheryCode, ns, bm->MofY, all_done,
							any_quota);
				}

				if (!any_quota || all_done) {
					/* If run out of quota in total (over all groups) or done for the month skip ahead */
					continue;
				}

				/* Find current total CPUE so can distribute effort - cpue_ind is insufficient
				 as that is current CPUE only with no spatial memory so get get erroneously
				 low CPUEbased below. Thus need separate sum here.
				 */
				totCPUE = 0;
				for (ij = 0; ij < bm->nbox; ij++) {
					if (bm->boxes[ij].type != BOUNDARY) {
						totCPUE += bm->SpatialCPUE[nf][ns][bm->MofY][ij];
						totDIS += bm->SpatialDisPUE[nf][ns][bm->MofY][ij];
						totVPUE += bm->SpatialVPUE[nf][ns][bm->MofY][ij];
					}
				}

				/* Determine effort location - dependent on trip length, costs,
				 catch plans at higher temporal steps, spatial distribution
				 of target groups
				 */
				toteffort = 0;
				for (ij = 0; ij < bm->nbox; ij++) {
					if (bm->boxes[ij].type != BOUNDARY) {

						/* This calc_effort entry is being reinitialised and reused with each
						 subfleet as final calculated effort is stored in bm->Effort_hdistrib[ij][nf][today_effort]
						 */
						bm->Effort_hdistrib[ij][nf][calc_effort] = 0;

						/* Start by finding optimal map based on CPUE (or potentially VPUE)
						 - weighted by targetting preferences - and constrained by zoning (spatial management).

						 Note that for cases where using incentives-based effort modification
						 (where there is an effort cap and a differential cost for fishing in specific
						 locations) the effort cost of a particular cell is already accounted for
						 in the SpatialCPUE (as penalised effort used in its calculation).
						 */

						if (bm->UseVPUE)
							step1 = bm->SpatialVPUE[nf][ns][bm->MofY][ij] / (totVPUE + small_num);
						else
							step1 = bm->SpatialCPUE[nf][ns][bm->MofY][ij] / (totCPUE + small_num);

						CPUEbased = bm->MPA[ij][nf] * bm->EffortSchedule[nf][ns][bm->MofY][expect_id] * step1;

						if (bm->SpatialBycatchAvoid) {
							/* If wanting to use non-penalty/non-cap incentives to steer them away from areas where they discard the most */
							CPUEbased *= (1 - (bm->SpatialDisPUE[nf][ns][bm->MofY][ij] / (totDIS + small_num)));
						}

						if (do_debug) {
							fprintf(
									llogfp,
									"Time: %e %s in %d CPUEbased: %e, MPA: %e, SpatialCPUE[%s][%d][%d][%d]: %e, SUBFLEET_ECONprms[cpue_ind]: %e, EffortSchedule[expect]: %e\n",
									bm->dayt, FisheryArray[nf].fisheryCode, ij, CPUEbased, bm->MPA[ij][nf], FisheryArray[nf].fisheryCode, ns, bm->MofY, ij,
									bm->SpatialCPUE[nf][ns][bm->MofY][ij], bm->SUBFLEET_ECONprms[nf][ns][cpue_ind_id],
									bm->EffortSchedule[nf][ns][bm->MofY][expect_id]);
						}

						/* Find petrol cost implications - as for alt_cpue_effort fleet dynamics
						 models in atmanage.lib
						 */
						FCweight = 0;
						Costscale = bm->SUBFLEET_ECONprms[nf][ns][var_cost_id];

						if (!Costscale)
							Costscale = 1.0;

						for (porti = 0; porti < bm->K_num_ports; porti++) {

							/* See if ever an active port for this fishery */
							flagfishhere = bm->Port_Fishery[nf][porti];

							/* Check if currently active */
							if ((bm->dayt < bm->Port_info[porti][port_start_id]) || (bm->dayt > bm->Port_info[porti][port_end_id])) {
								flagfishhere = 0;
							}

							/* If an active port than determine contribution to effort from that port
							 use the scalar Costscale to heighten or lessen effects of distance
							 - so can also capture social and economic forces which may push
							 fishers to either stay close to home or go far out to sea even when
							 not economically efficent to do so */
							if (flagfishhere) {
								FCweight += (bm->Speed_boat * bm->dt) / (bm->Port_distance[porti][ij] * Costscale * bm->K_num_ports + small_num);

							}
							//fprintf(llogfp,"Time: %e %s-%d port %d  FCweight: %e Costscale: %e Speed_boat: %e dt: %e Port_dist: %e (flagfishhere: %d, Port_Fishery: %d, port_start: %e, port_end: %e)\n",
							//	bm->dayt, FisheryArray[nf].fisheryCode, ns, porti, FCweight, Costscale, bm->Speed_boat, bm->dt, bm->Port_distance[porti][ij], flagfishhere, bm->Port_Fishery[nf][porti], bm->Port_info[porti][port_start_id], bm->Port_info[porti][port_end_id]);

						}

						FCweight *= bm->SUBFLEET_ECONprms[nf][ns][FCwgtscale_id];

						/* Interpolate between planned effort locations and CPUE-based ones (so shift across
						 as information spreads), but constrained by flexibility.
						 */
						tempEffort = bm->SUBFLEET_ECONprms[nf][ns][flexweight_id] * (CPUEbased - bm->Effort_hdistrib[ij][nf][bm->MofY])
								+ bm->Effort_hdistrib[ij][nf][bm->MofY];

						if (tempEffort < 0)
							tempEffort = 0;

						if (do_debug) {
							fprintf(llogfp,
									"Time: %e %s-%d in %d tempEffort: %e, SUBFLEET_ECONprms[flexweight_id]: %e, CPUEbased: %e, bm->Effort_hdistrib[%d]: %e\n",
									bm->dayt, FisheryArray[nf].fisheryCode, ns, ij, tempEffort, bm->SUBFLEET_ECONprms[nf][ns][flexweight_id], CPUEbased,
									bm->MofY, bm->Effort_hdistrib[ij][nf][bm->MofY]);
						}

						/* For final effort distribution take distribution based on plan update by immediate CPUE
						 results (the ideal distribution effectively) and interpolate it with the current
						 effort map, constrained by costs and trip length behaviour
						 */
						FCpressure = FCweight * (max(1.0,(bm->Speed_boat * bm->dt / bm->width)) * (tempEffort - bm->OldEffort[ij][nf]) + bm->OldEffort[ij][nf]);

						bm->SpatialBlackBook[nf][ns][bm->MofY][ij][current_id] = FCpressure;
						bm->Effort_hdistrib[ij][nf][calc_effort] += FCpressure;
						toteffort += bm->Effort_hdistrib[ij][nf][calc_effort];

						if (do_debug) {
							fprintf(
									llogfp,
									"Time: %e %s-%d in %d bm->Effort_hdistrib[calc_effort]: %e, FCpressure: %e, FCweight: %e, max: %e, Speed_boat: %e, dt/width: %e, tempEffort: %e, OldEffort: %e, toteffort: %e\n",
									bm->dayt, FisheryArray[nf].fisheryCode, ns, ij, bm->Effort_hdistrib[ij][nf][calc_effort], FCpressure, FCweight,
									(max(1.0,(bm->Speed_boat * bm->dt / bm->width))), bm->Speed_boat, (bm->dt / bm->width), tempEffort,
									bm->OldEffort[ij][nf], toteffort);
						}
					}
				}

				/* Normalise calc_effort and calculate final realised effort per day per
				 fishery. Multiply by shot length per day to get final realised effort for
				 use in Allocate_Immediate_Effort() in atmanage.lib
				 */
				if (do_debug) {
					fprintf(llogfp, "Time: %e, %s-%d toteffort: %e\n", bm->dayt, FisheryArray[nf].fisheryCode, ns, toteffort);
				}

				this_effort = 0;
				if (toteffort > 0) {
					for (ij = 0; ij < bm->nbox; ij++) {
						if (bm->boxes[ij].type != BOUNDARY) {
							FCpressure = bm->SUBFLEET_ECONprms[nf][ns][FishableLength_id] * (bm->EffortSchedule[nf][ns][bm->MofY][expect_id] / 30.0)
									* (bm->Effort_hdistrib[ij][nf][calc_effort] / (toteffort + small_num));
							/* Effort_hdistrib - value per fishery not a subfleet value */
							bm->Effort_hdistrib[ij][nf][new_today_effort] += FCpressure;
							this_effort += FCpressure;

							if (do_debug) {
								fprintf(
										llogfp,
										"Time: %e %s-%d Effort_hdistrib-box%d: %e, level: %e, fishablelength: %e, Effort_hdistrib: %e (effort per boat per day: %e - nboat: %e), this_effort: %e\n",
										bm->dayt, FisheryArray[nf].fisheryCode, ns, ij, FCpressure, (bm->EffortSchedule[nf][ns][bm->MofY][expect_id] / 30.0),
										bm->SUBFLEET_ECONprms[nf][ns][FishableLength_id], (bm->Effort_hdistrib[ij][nf][calc_effort] / (toteffort + small_num)),
										((bm->EffortSchedule[nf][ns][bm->MofY][expect_id] / 30.0) / bm->SUBFLEET_ECONprms[nf][ns][nboat_id]),
										bm->SUBFLEET_ECONprms[nf][ns][nboat_id], this_effort);
							}
						}
					}
				}
				/* Store realised effort - *7 as updated once per week */
				old_realised = bm->EffortSchedule[nf][ns][bm->MofY][current_id];
				bm->EffortSchedule[nf][ns][bm->MofY][current_id] += (this_effort * 7);

				/* Check whether have exceeded maximum total effort */
				if (do_debug) {
					fprintf(llogfp,
							"Time: %e %s-%d mth: %d check EffortSchedule: %e (e: %e), max_month_effort: %e, this_effort: %e, toteffort: %e old_realised: %e\n",
							bm->dayt, FisheryArray[nf].fisheryCode, ns, bm->MofY, bm->EffortSchedule[nf][ns][bm->MofY][current_id],
							bm->EffortSchedule[nf][ns][bm->MofY][expect_id], max_month_effort, this_effort, toteffort, old_realised);
				}

				if ((!bm->hist_only) && (bm->EffortSchedule[nf][ns][bm->MofY][current_id] > max_month_effort)) {
					permisible_left = max_month_effort - old_realised;
					if (permisible_left < 0)
						permisible_left = 0;
					for (ij = 0; ij < bm->nbox; ij++) {
						if (bm->boxes[ij].type != BOUNDARY) {
							bm->Effort_hdistrib[ij][nf][today_effort] += bm->SUBFLEET_ECONprms[nf][ns][FishableLength_id]
									* (bm->EffortSchedule[nf][ns][bm->MofY][expect_id] / 30.0) * (bm->Effort_hdistrib[ij][nf][calc_effort] / (toteffort
									+ small_num)) * permisible_left / (this_effort + small_num);

							if (do_debug) {
								fprintf(llogfp, "Time: %e %s-%d new rescaled Effort_hdistrib-%d: %e, permissible: %e this_effort: %e, max_month_effort: %e\n",
										bm->dayt, FisheryArray[nf].fisheryCode, ns, ij, bm->Effort_hdistrib[ij][nf][today_effort], permisible_left,
										this_effort, max_month_effort);
							}
						}
					}
					/* Store new realised effort */
					bm->EffortSchedule[nf][ns][bm->MofY][current_id] = max_month_effort;
				} else {
					/* Update final effort distribution */
					for (ij = 0; ij < bm->nbox; ij++) {
						if (bm->boxes[ij].type != BOUNDARY) {
							bm->Effort_hdistrib[ij][nf][today_effort] = bm->Effort_hdistrib[ij][nf][new_today_effort];

							if (do_debug) {
								fprintf(llogfp, "Time: %e %s-%d new setting Effort_hdistrib-%d: %e\n", bm->dayt, FisheryArray[nf].fisheryCode, ns, ij,
										bm->Effort_hdistrib[ij][nf][today_effort]);
							}
						}
					}
				}
			}
		}
	}
}

/**
 * \brief Dan Holland effort model - originally developed for the northern california current
 */
void Holland_Effort_Final_Allocation(MSEBoxModel *bm, FILE *llogfp) {
	int nf, ij, ns, sp, do_debug_base, do_debug = 0, regid, trip_length, do_biTAC, mth, nss, use_history, need_to_init, do_consoldation = 0;
	double cumcatch, toteffort, FCpressure, spbiom, q, tolerable_debt, TotCC, use_minprob, minprob, maxprob, totallowedcatch, totscore, max_hold, regTAC_scale, totTAC, catchleft, totcatch, regcatch_nf, regcatch, diff_catch, max_quota_left, totmaxland, totland_overall, maxland, catch_here, hold_size;

	if (((bm->debug == debug_econeffort) || (bm->debug == debug_effort)) && (bm->dayt > bm->checkstart)) {
		do_debug_base = 1;
	} else
		do_debug_base = 0;
	/** Dan Holland effort model - originally developed for the northern california current **/
	/* First up find cumulative catch per species in participating fisheries */
	for (regid = 0; regid < bm->K_num_active_reg; regid++) {
		for (sp = 0; sp < bm->K_num_tot_sp; sp++) {
			if (FunctGroupArray[sp].isFished == TRUE) {

				regTAC_scale = bm->RegionalData[sp][regid][reg_tac_id];
				tot_cumcatch[sp][regid] = 0;
				totTAC = 0;
				TotCC = 0;
				for (nf = 0; nf < bm->K_num_fisheries; nf++) {
					if (do_debug_base && ((bm->which_fleet == nf) || (bm->which_fleet >= bm->K_num_fisheries))) {
						do_debug = 1;
					} else
						do_debug = 0;

					for (ns = 0; ns < bm->FISHERYprms[nf][nsubfleets_id]; ns++) {
						tot_cumcatch[sp][regid] += RegCatch[nf][ns][regid][sp];

						/*
						 if(sp == FPO_id)
						 fprintf(llogfp,"Time: %e, %s %s totcumcatch: %e, regcatch: %e\n",
						 bm->dayt, FunctGroupArray[sp].groupCode, FisheryArray[nf].fisheryCode, tot_cumcatch[sp][regid][bm->thisyear], RegCatch[nf][ns][regid][sp]);
						 */
					}
					/* Get total current TAC - leave in kg as tot_cumcatch is in kg here */
                    if(FunctGroupArray[sp].isTAC && (bm->TACamt[sp][nf][now_id] < no_quota)) {
                        totTAC += regTAC_scale * bm->TACamt[sp][nf][now_id];
                    }

					if (do_debug) {
						fprintf(llogfp, "Time: %e, reg: %d, %s regTACscale: %e, %s TAC: %e kg\n", bm->dayt, regid, FunctGroupArray[sp].groupCode, regTAC_scale,
								FisheryArray[nf].fisheryCode, bm->TACamt[sp][nf][now_id]);
					}
					//TotCC += bm->TotCumCatch[sp][nf][bm->thisyear];
					TotCC += (Harvest_Get_TotCumCatch(sp, nf, bm->thisyear) + bm->TotOldCumCatch[sp][nf]);
				}
				quota_left[sp][regid] = totTAC - tot_cumcatch[sp][regid];
				if (quota_left[sp][regid] < 0)
					quota_left[sp][regid] = 0;

                /*
				if (do_debug) {
					//if((sp == FPO_id) && (!bm->newweek)){
					fprintf(llogfp,
							"Time: %e, reg: %d, %s quota_left: %e, totTAC: %e, cumcatch: %e (TotCumCatch: %e, regTAC_scale: %e, K_num_active_reg: %d)\n",
							bm->dayt, regid, FunctGroupArray[sp].groupCode, quota_left[sp][regid], totTAC, tot_cumcatch[sp][regid], TotCC * bm->X_CN * mg_2_kg,
							regTAC_scale, bm->K_num_active_reg);
				}
                */
			}
		}

		/* Initialise effort distribution - if need be (don't otherwise or will zero effort mid trip) */
		for (nf = 0; nf < bm->K_num_fisheries; nf++) {
			if (bm->FISHERYprms[nf][flageffortmodel_id] > rec_econ_model) {
				need_to_init = 0;
				for (ns = 0; ns < bm->FISHERYprms[nf][nsubfleets_id]; ns++) {
					trip_length = (int) (bm->SUBFLEET_ECONprms[nf][ns][max_trip_length_id]);
					if (trip_length == 0)
						trip_length = 1;

					if ((bm->TofY % trip_length) == 0) {
						need_to_init = 1;
					}
				}

				if (need_to_init) {
					for (ij = 0; ij < bm->nbox; ij++) {
						bm->Effort_hdistrib[ij][nf][today_effort] = 0;
						bm->Effort_hdistrib[ij][nf][new_today_effort] = 0;
					}
				}
			}
		}
	}

	/* Now start calculating the effort - if the trip is on */
	for (nf = 0; nf < bm->K_num_fisheries; nf++) {
		if (bm->FISHERYprms[nf][flageffortmodel_id] > rec_econ_model) {
			if (do_debug_base && ((bm->which_fleet == nf) || (bm->which_fleet >= bm->K_num_fisheries))) {
				do_debug = 1;
			} else
				do_debug = 0;

			if (bm->FISHERYprms[nf][flagmanage_id] == cumTACclosure_mgmt)
				do_biTAC = 1; /* US-like cumulative trip limits (Dan Holland version) */
			else
				do_biTAC = 0; /* Use Dan holland version of ITQ */

            do_consoldation = 0;
            for (ns = 0; ns < bm->FISHERYprms[nf][nsubfleets_id]; ns++) {
                /* If no boats in the subfleet currently skip ahead */
                if (!bm->SUBFLEET_ECONprms[nf][ns][nboat_id]) {
                    continue;
                }
                
                /* Initalise total monthly effort tracker */
                if (bm->newmonth)
                    bm->EffortSchedule[nf][ns][bm->MofY][current_id] = 0;
                
                /* Check time to update (check to make sure just happens once per day is done back
                 in Economics()
                 */
                trip_length = (int) (bm->SUBFLEET_ECONprms[nf][ns][max_trip_length_id]);
                if (trip_length == 0)
                    trip_length = 1;
                
                if ((bm->TofY % trip_length) == 0) {
                    
                    /* Initialise values to update */
                    tot_marg_rent[nf][ns] = 0;
                    toteffort = 0;
                    totscore = 0;
                    maxprob = 0;
                    minprob = MAXDOUBLE;
                    
                    /* Check if new bimonthly period */
                    if (bm->BiM != bm->LastBiM) {
                        
                        /* Reset prices */
                        for (sp = 0; sp < bm->K_num_tot_sp; sp++) {
                            if (FunctGroupArray[sp].isFished == TRUE) {
                                for (ij = 0; ij < bm->nbox; ij++) {
                                    bm->ECONexprice[nf][ns][sp][ij][expect_id] = bm->ECONexprice[nf][ns][sp][ij][hist_id];
                                }
                                for (ij = 0; ij < bm->K_num_active_reg; ij++) {
                                    totland[sp][ij] = 0;
                                }
                            }
                        }
                        
                    }
                    
                    /* Initialise quota_check */
                    for (sp = 0; sp < bm->K_num_tot_sp; sp++) {
                        if (FunctGroupArray[sp].isFished == TRUE) {
                            quota_check[sp] = 0;
                        }
                    }
                    
                    for (ij = 0; ij < bm->nbox; ij++) {
                        /* More initialisation */
                        bm->Effort_hdistrib[ij][nf][today_effort] = 0;
                        bm->Effort_hdistrib[ij][nf][calc_effort] = 0;
                        bm->Effort_hdistrib[ij][nf][new_today_effort] = 0;
                        bm->SpatialBlackBook[nf][ns][bm->MofY][ij][expect_id] = 0;
                        if (bm->boxes[ij].type != BOUNDARY) {
                            regid = bm->regID[ij];
                            
                            /* In case where using ITQ look at whether exhausted quota or not. If exhausted
                             set quota prices to an (arbitrary) high_price to prevent or deter fleets from
                             sending effort to locations where a lot of that species has been traditionally caught.
                             
                             At present as Dan assumes quota allocated where needed then cover regional component
                             of total quota holding in the quota allocation step not here.
                             */
                            for (sp = 0; sp < bm->K_num_tot_sp; sp++) {
                                if (FunctGroupArray[sp].isFished == TRUE) {
                                    if (!do_biTAC && !quota_check[sp]) {
                                        cumcatch = bm->QuotaAlloc[nf][ns][sp][cumboatcatch_id];
                                        
                                        /* Update how much quota has been leased */
                                        if (cumcatch > bm->QuotaAlloc[nf][ns][sp][owned_id]) {
                                            diff_catch = cumcatch - bm->QuotaAlloc[nf][ns][sp][owned_id];
                                            if (diff_catch > bm->SP_FISHERYprms[sp][nf][catch_allowed])
                                                bm->QuotaAlloc[nf][ns][sp][leased_id] += (bm->SP_FISHERYprms[sp][nf][catch_allowed] * bm->X_CN * mg_2_kg);
                                            else
                                                bm->QuotaAlloc[nf][ns][sp][leased_id] += diff_catch;
                                        }
                                        
                                        /* Store amount of total leased quota that was leased in the last round of calculations - since calculating lease prices */
                                        if (bm->QuotaAlloc[nf][ns][sp][leased_id] > bm->QuotaAlloc[nf][ns][sp][oldleased_id]) {
                                            bm->QuotaAlloc[nf][ns][sp][newleased_id] = bm->QuotaAlloc[nf][ns][sp][leased_id]
                                            - bm->QuotaAlloc[nf][ns][sp][oldleased_id];
                                        }
                                        
                                        /* See if any quota remaining anywhere - if not set quotaprice to high_price so try to avoid it */
                                        if (quota_left[sp][regid] == 0) {
                                            bm->QuotaAlloc[nf][ns][sp][quotaprice_id] = bm->high_price;
                                            
                                            //fprintf(llogfp,"Time: %e, %s-%d on %s quotaprice set to %e\n",
                                            //	bm->dayt, FisheryArray[nf].fisheryCode, ns, FunctGroupArray[sp].groupCode, bm->QuotaAlloc[nf][ns][sp][quotaprice_id]);
                                        }
                                        quota_check[sp] = 1;
                                        
                                        /*
                                         if((sp == FPO_id) && (ij == 1) && (!bm->newweek))
                                         fprintf(llogfp,"Time: %e, %s-%d on %s quotaleft %e, quotaprice: %e\n",
                                         bm->dayt, FisheryArray[nf].fisheryCode, ns, FunctGroupArray[sp].groupCode, quota_left[sp][regid], bm->QuotaAlloc[nf][ns][sp][quotaprice_id]);
                                         */
                                    }
                                    
                                    /* Determine prices for each functional group and fleet/subfleet. These are
                                     set to zero if the cumulative trip limit has been reached for that
                                     combination of fleet/subfleet and functional group.
                                     */
                                    if (do_biTAC && ((bm->TotCumBiCatch[sp][nf] * bm->RegionalData[sp][regid][reg_catch_id])
                                                     > (bm->BiTAC_sp[bm->BiM][regid][sp][now_id] * kg_2_mg / bm->X_CN))) {
                                        /* When using cumulative trip limits set expected price to zero to discourage further targetting */
                                        bm->ECONexprice[nf][ns][sp][ij][expect_id] = 0.0;
                                    } else {
                                        /* When using ITQ get net price as market price - quota price for each species
                                         NB if just took quotaprice from expected price then that may happen muliple times in
                                         bimonth period (between resets) and would have compounding losses, which is not
                                         appropriate
                                         */
                                        bm->ECONexprice[nf][ns][sp][ij][expect_id] = (bm->ECONexprice[nf][ns][sp][ij][hist_id]
                                                                                      - bm->QuotaAlloc[nf][ns][sp][quotaprice_id]);
                                        
                                        //fprintf(llogfp,"Time: %e, %s for %s-%d box: %d, ECONexprice: %e, [quotaprice_id]: %e\n",
                                        //	bm->dayt, FunctGroupArray[sp].groupCode, FisheryArray[nf].fisheryCode, ns, ij, bm->ECONexprice[nf][ns][sp][ij][expect_id], bm->QuotaAlloc[nf][ns][sp][quotaprice_id]);
                                    }
                                    /* Determine catch expectations as:
                                     
                                     CPUE_for_area * expected_total_tow_time
                                     
                                     with tow_time calculated as
                                     
                                     prop_day_fishing * trip_length
                                     
                                     For now, these are calculated using the true CPUE given the current state
                                     of the stock and the catchability coefficient(i.e., fleet has perfect information).
                                     It would be possible to add noise to this, or build an expectation based on
                                     previous catch history. In Dan Holland's opinion the latter is problematic because
                                     of holes in the data (specifically: what is the expectation for areas not fished
                                     during period used to model expectations?)
                                     
                                     */
                                    spbiom = bm->targetspbiom[sp][ij] * bm->X_CN * mg_2_kg;
                                    q = bm->SP_FISHERYprms[sp][nf][q_id];
                                    /* Get expected catches - store in SpatialBlackBook as
                                     it will become the basis of the effort distribution
                                     
                                     Accumulate over species so not just set by the final
                                     species considered (which could zero it again).
                                     */
                                    catch_here = spbiom * q * prop_tow_time[nf][ns][ij];
                                    bm->SpatialBlackBook[nf][ns][bm->MofY][ij][expect_id] += catch_here;
                                    totland[sp][regid] += catch_here;
                                    
                                    //fprintf(llogfp,"Time: %e, %s-%d mth: %d box: %d SpatialBlackBook: %e, catch_here: %e, spbiom: %e, q: %e, prop_tow_time: %e\n",
                                    //	bm->dayt, FisheryArray[nf].fisheryCode, ns, bm->MofY, ij, bm->SpatialBlackBook[nf][ns][bm->MofY][ij][expect_id], catch_here, spbiom, q, prop_tow_time[nf][ns][ij]);
                                }
                            }
                        }
                    }
                    
                    /* Get the maximum catch possible for the trip */
                    for (ij = 0; ij < bm->nbox; ij++) {
                        if (bm->boxes[ij].type != BOUNDARY) {
                            regid = bm->regID[ij];
                            totmaxland = 0;
                            totland_overall = 0;
                            for (sp = 0; sp < bm->K_num_tot_sp; sp++) {
                                if (FunctGroupArray[sp].isFished == TRUE) {
                                    /* Used to be if(do_biTAC && (totland[sp][regid] > maxland)) */
                                    if (do_biTAC) {
                                        /* If using cumulative trip limits, expected landings can not exceed the difference
                                         between cumulative landings and the bimonthly trip limit.
                                         */
                                        catchleft = (bm->BiTAC_sp[bm->BiM][regid][sp][now_id] * kg_2_mg / bm->X_CN) - (bm->TotCumBiCatch[sp][nf]
                                                                                                                       * bm->RegionalData[sp][regid][reg_catch_id]);
                                        maxland = min(totland[sp][regid],catchleft);
                                        
                                        //fprintf(llogfp, "Time: %e, %s regid: %d, maxland: %e, totland: %e, catchleft: %e, BiTAC: %e, CumBiCatch: %e, RegData: %e\n",
                                        //	bm->dayt, FunctGroupArray[sp].groupCode, regid, maxland, totland[sp][regid], catchleft, (bm->BiTAC_sp[bm->BiM][regid][sp][now_id] * kg_2_mg / bm->X_CN), bm->TotCumBiCatch[sp][nf], bm->RegionalData[sp][regid][reg_catch_id]);
                                        
                                    } else {
                                        /* If using quotas the expected landings can't exceed the quota remaining */
                                        if (bm->ITQ_think_ahead) {
                                            /* With forethought spread the remaining quota through the year in proportion to
                                             the catch per month
                                             */
                                            catchleft = 0;
                                            for (mth = bm->MofY; mth < 12; mth++) {
                                                catchleft += bm->BlackBook[nf][ns][sp][mth][expect_id];
                                            }
                                            maxland
                                            = min(totland[sp][regid],(quota_left[sp][regid] * bm->BlackBook[nf][ns][sp][bm->MofY][expect_id] / (catchleft + small_num)));
                                            
                                            /* Cap based on hold capacity - using full hold for every boat every day as maximum bound physically possible
                                             Converting from tonnes to kg as we go (totland etc has been calculated in kg).
                                             */
                                            hold_size = bm->SUBFLEET_ECONprms[nf][ns][hold_capacity_id] * 1000.0;
                                            max_hold = hold_size * bm->SUBFLEET_ECONprms[nf][ns][nboat_id] * 30.0;
                                            maxland = min(maxland, max_hold);
                                            
                                            //fprintf(llogfp,"Time: %e, %s regid: %d, maxland: %e, totland: %e, quota_left: %e, BlackBook: %e, catchleft: %e, max_hold: %e\n",
                                            //	bm->dayt, FunctGroupArray[sp].groupCode, regid, maxland, totland[sp][regid], quota_left[sp][regid], bm->BlackBook[nf][ns][sp][bm->MofY][expect_id], catchleft, max_hold);
                                            
                                        } else {
                                            /* In the case with no forethought then the only check on catch is quota left */
                                            maxland = min(totland[sp][regid],quota_left[sp][regid]);
                                            
                                            /* Cap based on hold capacity - using full hold for every boat every day as maximum bound physically possible
                                             Converting from tonnes to kg as we go (totland etc has been calculated in kg).
                                             */
                                            hold_size = bm->SUBFLEET_ECONprms[nf][ns][hold_capacity_id] * 1000.0;
                                            max_hold = hold_size * bm->SUBFLEET_ECONprms[nf][ns][nboat_id] * 30.0;
                                            maxland = min(maxland, max_hold);
                                            
                                            /**
                                             if(sp == FVD_id){
                                             fprintf(llogfp,"Time: %e, %s regid: %d, maxland: %e, totland: %e, quota_left: %e, max_hold: %e\n",
                                             bm->dayt, FunctGroupArray[sp].groupCode, regid, maxland, totland[sp][regid], quota_left[sp][regid], max_hold);
                                             }
                                             **/
                                        }
                                    }
                                    /* Cumulative maxland value (based on what is allowed catch from outstanding quotas for each species) */
                                    totmaxland += maxland;
                                    totland_overall += totland[sp][regid];
                                }
                            }
                            
                            //fprintf(llogfp,"Time: %e box-%d %s-%d SpatialBlackBook[expect] starts: %e ", bm->dayt, ij, FisheryArray[nf].fisheryCode, ns, bm->SpatialBlackBook[nf][ns][bm->MofY][ij][expect_id]);
                            
                            bm->SpatialBlackBook[nf][ns][bm->MofY][ij][expect_id] = (bm->SpatialBlackBook[nf][ns][bm->MofY][ij][expect_id] / (totland_overall
                                                                                                                                              + small_num)) * totmaxland;
                            
                            //fprintf(llogfp,"ends: %e, totland: %e, totmaxland: %e\n", bm->SpatialBlackBook[nf][ns][bm->MofY][ij][expect_id], totland_overall, totmaxland);
                            
                            /* Calculate expected revenue for all areas and subfleets by multiplying
                             the expected catch by prices and summing over groups.
                             Prices are zero if trip limits have been met.
                             
                             Stored the value in total marginal rent array here as effectively the same thing and
                             plays the same role in the alternate economics model so rather than duplicate arrays used
                             the old name here.
                             
                             To do the calculation on a per species basis we need to multiply spatial total catch by:
                             species price * proportion of the total catch made up by the species
                             */
                            tot_marg_rent[nf][ns] = 0;
                            for (sp = 0; sp < bm->K_num_tot_sp; sp++) {
                                if (FunctGroupArray[sp].isFished == TRUE) {
                                    tot_marg_rent[nf][ns] += bm->SpatialBlackBook[nf][ns][bm->MofY][ij][expect_id] * bm->ECONexprice[nf][ns][sp][ij][expect_id]
                                    * (totland[sp][regid] / (totland_overall + small_num));
                                    
                                    //fprintf(llogfp, "Time: %e, %s (%d) mth: %d, ij: %d, tot_marg_rent: %e, SpatialBlackBook: %e, ECONexprice: %e, totland[%s][%d]: %e, totland_overall: %e\n",
                                    //	bm->dayt, FisheryArray[nf].fisheryCode, ns, bm->MofY, ij, tot_marg_rent[nf][ns], bm->SpatialBlackBook[nf][ns][bm->MofY][ij][expect_id], bm->ECONexprice[nf][ns][sp][ij][expect_id], FunctGroupArray[sp].groupCode, regid, totland[sp][regid], totland_overall);
                                }
                            }
                            /* Create net expected revenue for each choice by subtracting costs from total expected revenue
                             Note that: Observer costs are included in fixed costs of trip and mgmt_cost_id is the % of
                             revenue taken as recovered costs of management.
                             */
                            tot_marg_rent[nf][ns] *= (1 - bm->SUBFLEET_ECONprms[nf][ns][mgmt_cost_id]);
                            tot_marg_rent[nf][ns] -= bm->SUBFLEET_ECONprms[nf][ns][fixed_cost_id];
                            
                            //fprintf(llogfp, "Time: %e, %s (%d) mgmt_cost_correction: %e, fixed_cost: %e\n",
                            //	bm->dayt, FisheryArray[nf].fisheryCode, ns, (1 - bm->SUBFLEET_ECONprms[nf][ns][mgmt_cost_id]), bm->SUBFLEET_ECONprms[nf][ns][fixed_cost_id]);
                            
                            /* Determine for each potential fishing choice whether the expected
                             profits are high enough to justify the trip. Effort reset to zero for that
                             box if returns (profit) not expected to be high enough.
                             */
                            tolerable_debt = -1.0 * bm->SUBFLEET_ECONprms[nf][ns][tol_debt_id];
                            if (tot_marg_rent[nf][ns] < tolerable_debt) {
                                bm->SpatialBlackBook[nf][ns][bm->MofY][ij][expect_id] = 0;
                                tot_marg_rent[nf][ns] = 0;
                            }
                            /* Calculate expected revenue per unit effort across areas for each fleet.
                             Store the revenue map in the Effort_hdistrib as it is a draft effort
                             allocation map.
                             */
                            bm->Effort_hdistrib[ij][nf][new_today_effort] = tot_marg_rent[nf][ns] / (bm->SUBFLEET_ECONprms[nf][ns][max_trip_length_id]
                                                                                                     + small_num);
                            toteffort += bm->Effort_hdistrib[ij][nf][new_today_effort];
                        }
                    }
                    
                    for (ij = 0; ij < bm->nbox; ij++) {
                        if (bm->boxes[ij].type != BOUNDARY) {
                            
                            /* Normalise the effort distribution */
                            bm->Effort_hdistrib[ij][nf][new_today_effort] /= (toteffort + small_num);
                            
                            /* Calculate a score for each area based on expected revenue and spatial closures */
                            bm->Effort_hdistrib[ij][nf][calc_effort] = bm->MPA[ij][nf] * exp(bm->SUBFLEET_ECONprms[nf][ns][betarev_id]
                                                                                             * bm->Effort_hdistrib[ij][nf][new_today_effort]);
                            totscore += bm->Effort_hdistrib[ij][nf][calc_effort];
                            
                            //fprintf(llogfp,"Time: %e, %s-%d box-%d calceffort: %e, MPA: %e, exprev: %e, betarev: %e, newtodayeffort: %e\n",
                            //	bm->dayt, FisheryArray[nf].fisheryCode, ns, ij, bm->Effort_hdistrib[ij][nf][calc_effort], bm->MPA[ij][nf], exp(bm->SUBFLEET_ECONprms[nf][ns][betarev_id] * bm->Effort_hdistrib[ij][nf][new_today_effort]), bm->SUBFLEET_ECONprms[nf][ns][betarev_id], bm->Effort_hdistrib[ij][nf][new_today_effort]);
                        }
                    }
                    
                    for (ij = 0; ij < bm->nbox; ij++) {
                        if (bm->boxes[ij].type != BOUNDARY) {
                            /* Calculates probabilities for each location by normalising scores stored in Effort_hdistrib */
                            bm->Effort_hdistrib[ij][nf][calc_effort] /= (totscore + small_num);
                            
                            /* Find maximum and minimum realised probabilities */
                            if (maxprob < bm->Effort_hdistrib[ij][nf][calc_effort])
                                maxprob = bm->Effort_hdistrib[ij][nf][calc_effort];
                            
                            if (minprob > bm->Effort_hdistrib[ij][nf][calc_effort])
                                minprob = bm->Effort_hdistrib[ij][nf][calc_effort];
                        }
                    }
                    
                    /* To make sure at least one area is chosen, the threshold probability is set equal
                     to the maximum realised probabilty score (if that is below the prespecified threshold)
                     */
                    
                    if (maxprob < bm->SUBFLEET_ECONprms[nf][ns][minprob_id])
                        use_minprob = maxprob;
                    else
                        use_minprob = bm->SUBFLEET_ECONprms[nf][ns][minprob_id];
                    
                    //fprintf(llogfp, "%e %s-%d min: %e, max: %e, usemin: %e\n",
                    //	bm->dayt, FisheryArray[nf].fisheryCode, ns, maxprob, bm->SUBFLEET_ECONprms[nf][ns][minprob_id], use_minprob);
                    
                    totscore = 0;
                    for (ij = 0; ij < bm->nbox; ij++) {
                        if (bm->boxes[ij].type != BOUNDARY) {
                            /* Eliminate choices with probabilties below minimum probabilty threshold.*/
                            
                            //fprintf(llogfp, "%e %s-%d in %d calceffort: %e vs usemin: %e\n", bm->dayt, FisheryArray[nf].fisheryCode, ns, ij, bm->Effort_hdistrib[ij][nf][calc_effort], use_minprob);
                            
                            if (use_minprob > bm->Effort_hdistrib[ij][nf][calc_effort]) {
                                bm->Effort_hdistrib[ij][nf][calc_effort] = 0;
                                
                                //fprintf(llogfp, "%e %s-%d in %d newcalceffort = 0\n", bm->dayt, FisheryArray[nf].fisheryCode, ns, ij);
                                
                            } else {
                                /* Calculate last draft of location choices bringing in expected profit,
                                 distribution of probabilitis vs minimum threshold and pattern of closed areas
                                 */
                                bm->Effort_hdistrib[ij][nf][calc_effort] *= bm->SpatialBlackBook[nf][ns][bm->MofY][ij][expect_id] * bm->MPA[ij][nf];
                                
                                //fprintf(llogfp, "%e %s-%d in %d newcalceffort: %e (expect: %e, MPA: %e)\n",
                                //	bm->dayt, FisheryArray[nf].fisheryCode, ns, ij, bm->Effort_hdistrib[ij][nf][calc_effort], bm->SpatialBlackBook[nf][ns][bm->MofY][ij][expect_id], bm->MPA[ij][nf]);
                            }
                            totscore += bm->Effort_hdistrib[ij][nf][calc_effort];
                        }
                    }
                    
                    for (ij = 0; ij < bm->nbox; ij++) {
                        if (bm->boxes[ij].type != BOUNDARY) {
                            
                            /* Renormalize so that available effort is spread across the remaining trips
                             in proportion to choice scores that total 1 unless no areas are fished
                             */
                            bm->Effort_hdistrib[ij][nf][calc_effort] /= (totscore + small_num);
                            
                            /* Calculate applied effort as
                             
                             prop_fleet_in_area * fleet_size * prop_time_towing * prob_fleet_not_layover
                             
                             Note all calculations above lead to final proportional distribution, which is
                             stored in bm->SpatialBlackBook[nf][ns][bm->MofY][ij][current_id]
                             */
                            FCpressure = bm->Effort_hdistrib[ij][nf][calc_effort] * bm->SUBFLEET_ECONprms[nf][ns][nboat_id] * prop_tow_time[nf][ns][ij] * (1
                                                                                                                                                           - bm->SUBFLEET_ECONprms[nf][ns][down_time_id]);
                            
                            bm->Effort_hdistrib[ij][nf][today_effort] += FCpressure;
                            
                            /*
                             if(nf == trapBMS_id)
                             fprintf(llogfp, "Time: %e, %s box-%d, FCpressure: %e, today_effort: %e\n",
                             bm->dayt, FisheryArray[nf].fisheryCode, ij, FCpressure, bm->Effort_hdistrib[ij][nf][today_effort]);
                             */
                            
                            bm->SpatialBlackBook[nf][ns][bm->MofY][ij][current_id] = FCpressure;
                            
                            if (do_debug) {
                                fprintf(llogfp, "%e %s-%d final effort-%d: %e (calc_effort: %e, nboat: %e, prop_tow_time: %e, down_time: %e\n", bm->dayt,
                                        FisheryArray[nf].fisheryCode, ns, ij, FCpressure, bm->Effort_hdistrib[ij][nf][calc_effort],
                                        bm->SUBFLEET_ECONprms[nf][ns][nboat_id], prop_tow_time[nf][ns][ij], bm->SUBFLEET_ECONprms[nf][ns][down_time_id]);
                            }
                            
                            /* Store realised final overall effort */
                            bm->EffortSchedule[nf][ns][bm->MofY][current_id] += FCpressure;
                        }
                    }
                    
                    /* Consolidate allowed catch per fleet so can't land fish above limits */
                    
                    if (do_biTAC) {
                        /* Consolidation such that don't exceed	bimonthly trip limit - will discard any over catch */
                        for (sp = 0; sp < bm->K_num_tot_sp; sp++) {
                            if (FunctGroupArray[sp].isFished == TRUE) {
                                for (regid = 0; regid < bm->K_num_active_reg; regid++) {
                                    totallowedcatch
                                    = max(0.0,((bm->BiTAC_sp[bm->BiM][regid][sp][now_id] * kg_2_mg / bm->X_CN) - (bm->TotCumBiCatch[sp][nf] * bm->RegionalData[sp][regid][reg_catch_id])));
                                    bm->SP_FISHERYprms[sp][nf][catch_allowed] = totallowedcatch;
                                    
                                    /* Check for individual trip limits */
                                    if (bm->SP_FISHERYprms[sp][nf][catch_allowed] > bm->SP_FISHERYprms[sp][nf][trip_lim_id])
                                        bm->SP_FISHERYprms[sp][nf][catch_allowed] = bm->SP_FISHERYprms[sp][nf][trip_lim_id];
                                    
                                    if (do_debug) {
                                        fprintf(llogfp, "Time: %e, reg: %d, %s on %s totallowedcatch: %e, BiTAC_sp: %e, TotCumBiCatch: %e, RegionalData: %e\n",
                                                bm->dayt, regid, FisheryArray[nf].fisheryCode, FunctGroupArray[sp].groupCode, totallowedcatch,
                                                (bm->BiTAC_sp[bm->BiM][regid][sp][now_id] * kg_2_mg / bm->X_CN), bm->TotCumBiCatch[sp][nf],
                                                bm->RegionalData[sp][regid][reg_catch_id]);
                                    }
                                }
                            }
                        }
                        do_consoldation = 0;
                    } else {
                        /* Consolidate vs held quota */
                        do_consoldation = 1;
                    }
                }
            }
            if (do_consoldation) { // Consolidation used to be done within ns loop meaning only the final subfleet was setting it
                for (sp = 0; sp < bm->K_num_tot_sp; sp++) {
                    if (FunctGroupArray[sp].isFished == TRUE) {
                        totcatch = 0;
                        use_history = 0;
                        for (regid = 0; regid < bm->K_num_active_reg; regid++) {
                            totcatch += tot_cumcatch[sp][regid];
                        }

                        if (!totcatch) {
                            /* No catch taken from any of the regions could be because its before statistics
                             are stored for the year (i.e. early in first quarter) or there has been a
                             closure previously. To reset the calculations try historical catch distribution
                             instead */
                            use_history = 1;
                            for (nss = 0; nss < bm->FISHERYprms[nf][nsubfleets_id]; nss++) {
                                for (ij = 0; ij < bm->nbox; ij++) {
                                    if (bm->boxes[ij].type != BOUNDARY) {
                                        totcatch += bm->SpatialBlackBook[nf][nss][bm->MofY][ij][hist_id] * bm->BlackBook[nf][nss][sp][bm->MofY][hist_id];
                                    }
                                }
                            }
                        }
                        for (regid = 0; regid < bm->K_num_active_reg; regid++) {
                            /* In each region find proportion of catch taken by the current fleet
                             (summing across all its subfleets)
                             */
                            regcatch_nf = 0;
                            if (use_history) {
                                /* If using historical data (as there has been a recent closure or because its before
                                 statistics for the new year have been stored) */
                                for (nss = 0; nss < bm->FISHERYprms[nf][nsubfleets_id]; nss++) {
                                    for (ij = 0; ij < bm->nbox; ij++) {
                                        if (bm->boxes[ij].type != BOUNDARY) {
                                            if (bm->regID[ij] == regid) {
                                                regcatch_nf += bm->SpatialBlackBook[nf][nss][bm->MofY][ij][hist_id] * bm->BlackBook[nf][nss][sp][bm->MofY][hist_id];
                                                
                                                if (do_debug) {
                                                    //if(sppid == FPO_id){
                                                    fprintf(llogfp,
                                                            "Time: %e, reg: %d (box: %d), %s-%d on %s in month %d regcatch: %e, SpatialBB: %e, BB: %e)\n",
                                                            bm->dayt, regid, ij, FisheryArray[nf].fisheryCode, ns, FunctGroupArray[sp].groupCode, bm->MofY,
                                                            regcatch_nf, bm->SpatialBlackBook[nf][nss][bm->MofY][ij][hist_id],
                                                            bm->BlackBook[nf][nss][sp][bm->MofY][hist_id]);
                                                }
                                            }
                                        }
                                    }
                                }
                            } else {
                                /* If past the start of the year and fishery open (so have had a chance to catch something) */
                                for (nss = 0; nss < bm->FISHERYprms[nf][nsubfleets_id]; nss++) {
                                    regcatch_nf += RegCatch[nf][nss][regid][sp];
                                }
                            }
                            regcatch = regcatch_nf / (totcatch + small_num);

                            /* Distribute quota according to "relative need", based on proportion of catch they've taken - and convert from kg to mg */
                            max_quota_left = regcatch * quota_left[sp][regid] * (kg_2_mg / bm->X_CN);

                            /* Make sure the fishery can't take in more than its share of the remaining quota (in mg) */
                            bm->SP_FISHERYprms[sp][nf][catch_allowed] = max_quota_left;

                            if (do_debug) {
                                //if(sppid == FPO_id){
                                fprintf(
                                        llogfp,
                                        "Time: %e, reg: %d, %s on %s totallowedcatch: %e, max_quota_left: %e (regcatch: %e, quota_left: %e, use_history: %d)\n",
                                        bm->dayt, regid, FisheryArray[nf].fisheryCode, FunctGroupArray[sp].groupCode,
                                        bm->SP_FISHERYprms[sp][nf][catch_allowed], max_quota_left, regcatch, quota_left[sp][regid], use_history);
                            }
						}
					}
				}
			}
		}
	}
}

