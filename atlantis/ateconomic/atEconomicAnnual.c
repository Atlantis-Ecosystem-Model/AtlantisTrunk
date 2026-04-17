/*  Open library routines */
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <sjwlib.h>
#include <atEconomic.h>

static void Initialise_Annual_Effort(MSEBoxModel *bm, FILE *llogfp);
static double Calculate_Projected_Income(MSEBoxModel *bm, int nf, int ns, int do_debug, FILE *llogfp);
static void Identify_Target_Species(MSEBoxModel *bm, int nf, int ns, FILE *llogfp);
static void Dynamic_Effort_Allocation(MSEBoxModel *bm, int nf, int ns, double max_month_effort, int do_debug, FILE *llogfp);
static void Stocastic_Effort_Allocation(MSEBoxModel *bm, int nf, int ns, int *not_bounded, double totexpect_return, double max_month_effort, int do_debug,
		FILE *llogfp);
static void Calculate_Box_Allocation(MSEBoxModel *bm, int nf, int ns, int do_debug, FILE *llogfp);
static void Do_EconArray_Rescaling(MSEBoxModel *bm);

/**
 * \brief Annual effort schedule
 *
 *  Called annually.
 *
 *	Annually fisherman
 *	Annually
 *
 *	Initialisation
 *
 *	Loops over each fishery and then each subfleet in each fishery.
 *
 *
 * This is only applying to fisheries that are using effortmodel of 13.
 *
 *	Updates expectation based on black books
 *		Black books has all previous times fishing records.
 *		Some heavily weight different years.
 *
 *		When using deemed values and taxes it takes this into account.
 *
 *	Black book and effort data read in is the fishers black book.
 *	Black books are updated based on weighted av of previous years + year just gone on.
 *	Weightings are based on value read in from input file. Black book data is updated during the year.
 *	Some of them are innovative and some are conservative.
 *
 *	Flexibility of weighting - input parameter flexweight.
 *
 *	Expectation calculations are done over all species that can be fished not just the ones that
 *	they have fished in the past. Includes information about bycatch.
 *	Will also look at targetting different boxes so this is how it can decide to target a new species.
 *	They might try fishing in a different box if the CPUE in the current box is below a different threshold.
 *	This allows for target shift.
 *
 *	Expectation calculations take into account gear on the fleet. Each subfleet has perfect knowledge within the
 *	its subfleet. But no knowledge of expected catch. Only time its knows about other fleets is when its working
 *	out if it should switch and then it knows profits etc.
 *
 *	Once they have this expectations it goes through  and works out which species it should be targeting.
 *	It creates an ordered list. Ordered based on highest value return in gross $ return. Sale price * expected catch.
 *	Once ordered to highest to lowest.
 *	Ordered values are based on monthly black book value for that species * last years sale price for the matching month.

 *	Parameter ntargets in harvest that selects the number from the top of the list. Will focus on ntargets species.
 *	Will spit out a message to say which groups its targeting, so this is where it can switch targets. Will only tell you
 *	the switches not the full list of things that will be targetted. Species is really going from a bycatch to a target species.
 *
 *	There is the ability to change gear over time. This is is the Qchange values in the harvest input file. This will effect the
 *	catch data and thus the blackbook data. The expectation calculations don't explicitly take the gear into account.
 *
 *	If you are just replaying history don't bother resetting you efforts.
 *
 *	Then effort allocation
 *		Can replay history
 *		Stochastic
 *		- use probability until you have exhausted all of your lease.
 *			Allocate effort based on
 *
 *		Dynamic allocation
 *			- basically makes weighting of months expectation from annual expectation
 *			- allocates effort based on proportions
 *			- will upscale and downscale based on total potential return - tolerable debt.
 *			- 	If pot return < tolerable debt try to downscale effort.
 *			- effort is capped at a max effort you can apply. MaxMonthEffort. Can' fish more than number of days in the month - max_month_effort.
 *
 *		Then go's and checks that that amount of effort matches the quotas.
 *		What would i actually expect to get. Looks at existing quota, looks at how much could be traded based on what was traded in past year.
 *		If not possible - can't fish without quota and quota not big enough then effort will be scaled.
 *
 *	Once have effort per month it does a rough sketch of where its going to do the effort. Based on spatial black book and what it caught last year.
 *	Weights based on flexweight. How responsive you are - do you put effort where you have always put it or do you base on what you caught last year.
 *
 *	SpatialBlackBook - value per subfleet per month per box. days per month. No depth into but there is a min and max depth per fishery.
 *	Would have 0 effort in boxes that are too deep.
 *	Would
 *
 *	Once done effort allocation numbers are massaged to ensure values in arrays all add up.
 *
 *	At end it spits out the annual effort schedule. This allows you to compare its plans with what it actually does.
 *
 */
void Annual_Effort_Schedule(MSEBoxModel *bm, FILE *llogfp) {
	int nf, ns, month, sp, not_bounded, do_debug, do_debug_base;
	double totexpect_return, max_month_effort, down_time;
	double min_month_effort = 1;

	/* If no yearly effort plan write last years effort schedule and skip ahead */
	if (!bm->MultiPlanEffort) {
		if (bm->dayt > 1) {
			Economic_Output_Realised_Schedule(bm);
		}
		return;
	}

	if (((bm->debug == debug_econeffort) || (bm->debug == debug_effort)) && (bm->dayt > bm->checkstart)) {
		do_debug_base = 1;
	} else
		do_debug_base = 0;

	if (verbose > 1)
		printf("Calculating annual effort allocation\n");

	/* Output previous year's trades (no need to do it on first day of run as nothing to report) */
	if (bm->dayt > 1) {
		Economic_Output_Trades(bm);
		Economic_Output_Realised_Schedule(bm);
	}

	/* Initialise expected catches and effort distribution */
	Initialise_Annual_Effort(bm, llogfp);

	/* Check for timeout of HistOnly option */
	if (bm->hist_only && ((bm->dayt + 7.0) > bm->hist_only_timeout)) {
		bm->hist_only = 0;
		for (nf = 0; nf < bm->K_num_fisheries; nf++) {
			for (ns = 0; ns < bm->FISHERYprms[nf][nsubfleets_id]; ns++) {
				bm->SUBFLEET_ECONprms[nf][ns][flexweight_id] = bm->SUBFLEET_ECONprms[nf][ns][flexweight_orig_id];
			}
		}
		fprintf(llogfp, "Time: %e hist_only timed out, dynamic effort allocation from now\n", bm->dayt);
        if(bm->rescale_post_burnin) {
            Do_EconArray_Rescaling(bm);
        }
            
	}

	/* Updated expectations - only for those using economically based effort model */
	for (nf = 0; nf < bm->K_num_fisheries; nf++) {
		if (bm->FISHERYprms[nf][flageffortmodel_id] > rec_econ_model) {
			if (do_debug_base && ((bm->which_fleet == nf) || (bm->which_fleet >= bm->K_num_fisheries))) {
				do_debug = 1;
			} else
				do_debug = 0;

			for (ns = 0; ns < bm->FISHERYprms[nf][nsubfleets_id]; ns++) {
				not_bounded = 1;

				/* If no boats in the subfleet currently skip ahead */
				if (!bm->SUBFLEET_ECONprms[nf][ns][nboat_id]) {
					continue;
				}

				totexpect_return = 0;
				down_time = bm->SUBFLEET_ECONprms[nf][ns][down_time_id];
				max_month_effort = bm->month_scalar * (1 - down_time) * bm->SUBFLEET_ECONprms[nf][ns][nboat_id];

				if (do_debug) {
					fprintf(llogfp, "Time: %e %s-%d max_effort %e, down_time: %e, nboat: %e\n", bm->dayt, FisheryArray[nf].fisheryCode, ns, max_month_effort,
							down_time, bm->SUBFLEET_ECONprms[nf][ns][nboat_id]);
				}

				if (max_month_effort < min_month_effort) {
					/* Nothing to do here so clear the decks and continue with the next subfleet */
					for (sp = 0; sp < bm->K_num_tot_sp; sp++) {
						if (FunctGroupArray[sp].isFished == TRUE) {
							/* If can't catch the group skip ahead */
							if (!bm->SP_FISHERYprms[sp][nf][q_id])
								continue;
							for (month = 0; month < 12; month++) {
								pi[nf][ns][month][sp] = 0;
								bm->MonthAlloc[nf][ns][sp][month] = 0;
								bm->EffortSchedule[nf][ns][month][expect_id] = 0;
								bm->EffortSchedule[nf][ns][month][current_id] = 0;
								bm->BoxAlloc[nf][ns][month][current_id] = 0;
							}
							bm->QuotaAlloc[nf][ns][sp][spexpectcatch_id] = 0;
						}
					}
					bm->SUBFLEET_ECONprms[nf][ns][ExpectedCatch_id] = 0;
					bm->SUBFLEET_ECONprms[nf][ns][totPlanEffort_id] = 0;
					bm->SUBFLEET_ECONprms[nf][ns][totPlanCatch_id] = 0;

					continue;
				}

				/* Calculate the projected income */
				totexpect_return = Calculate_Projected_Income(bm, nf, ns, do_debug, llogfp);

				/* Identify species to target - will find the top ntarget species in the list. */
				Identify_Target_Species(bm, nf, ns, llogfp);

				/** Allocate general effort level **/
				bm->SUBFLEET_ECONprms[nf][ns][ExpectedCatch_id] = 0;

				/** Using historical effort only **/
				if (bm->hist_only) {
					for (month = 0; month < 12; month++) {
						bm->EffortSchedule[nf][ns][month][expect_id] = bm->EffortSchedule[nf][ns][month][hist_id];

						if (do_debug) {
							fprintf(llogfp, "Time: %e, %s-%d mth: %d, effort schedule: %e\n", bm->dayt, FisheryArray[nf].fisheryCode, ns, month,
									bm->EffortSchedule[nf][ns][month][hist_id]);
						}
					}
					/** Using stochastic effort allocation **/
				} else if (bm->stochastic_effort) {
					Stocastic_Effort_Allocation(bm, nf, ns, &not_bounded, totexpect_return, max_month_effort, do_debug, llogfp);
				} else {
					/** Using more deterministic effort allocation - this is used in most models.**/
					Dynamic_Effort_Allocation(bm, nf, ns, max_month_effort, do_debug, llogfp);
				}
				/* Calculate the spatial allocations */
				Calculate_Box_Allocation(bm, nf, ns, do_debug, llogfp);
			}
		}
	}

	/* Write out schedule */
	Economic_Output_Effort_Schedule(bm);

	return;
}

/**
 * \brief Zero all of the annual effort values before we start calculating new values.
 */
void Initialise_Annual_Effort(MSEBoxModel *bm, FILE *llogfp) {
	int nf, ns, sp, month, ij;

	for (nf = 0; nf < bm->K_num_fisheries; nf++) {
		if (bm->FISHERYprms[nf][flageffortmodel_id] > rec_econ_model) {
			for (ns = 0; ns < bm->FISHERYprms[nf][nsubfleets_id]; ns++) {
				bm->SUBFLEET_ECONprms[nf][ns][ExpectedCatch_id] = 0;
				bm->SUBFLEET_ECONprms[nf][ns][AnnualEffort_id] = 0;
				bm->SUBFLEET_ECONprms[nf][ns][within_id] = 0;
				for (sp = 0; sp < bm->K_num_tot_sp; sp++) {
					if (FunctGroupArray[sp].isFished == TRUE) {
						/* If can't catch the group skip ahead */
						if (!bm->SP_FISHERYprms[sp][nf][q_id])
							continue;
						bm->SUBFLEET_ECONprms[nf][ns][AnnualCatch_id] += bm->QuotaAlloc[nf][ns][sp][cumboatcatch_id];
						bm->QuotaAlloc[nf][ns][sp][cumboatcatch_id] = 0; // Reset annual total catch
						bm->QuotaAlloc[nf][ns][sp][spexpectcatch_id] = 0;
						bm->QuotaAlloc[nf][ns][sp][within_id] = 0;
					}
				}
				for (month = 0; month < 12; month++) {
					bm->EffortSchedule[nf][ns][month][current_id] = 0;
				}
			}
			for (ij = 0; ij < bm->nbox; ij++) {
				for (month = 0; month < 12; month++) {
					bm->Effort_hdistrib[ij][nf][month] = 0.0;
				}
			}
		}
		for (sp = 0; sp < bm->K_num_tot_sp; sp++) {
			if (FunctGroupArray[sp].isFished == TRUE) {
				bm->FISHERYtarget[nf][sp] = 0;
				for (ij = 0; ij < 12; ij++) {
					bm->Trades[ij][sp] = 0;
				}
				//fprintf(llogfp, "Now %s is target of %s\n", FunctGroupArray[sp].groupCode, FisheryArray[nf].fisheryCode);
			}
		}
	}
}

/**
 * \brief Calculate the projected income.
 *
 * Will loop over each month and each species and calculate the projected profit of this
 * subfleet targetting that species.
 *
 */
double Calculate_Projected_Income(MSEBoxModel *bm, int nf, int ns, int do_debug, FILE *llogfp) {
	int ntarg, month, sp;
	double ExpEffort = 0, ans, totexpect_return, calcTax;
	double TripCost = bm->SUBFLEET_ECONprms[nf][ns][pereffort_cost_ind_id];
	//TripCost = bm->SUBFLEET_ECONprms[nf][ns][cost_ind_id];	replaced with pereffort_costs as multiply by exepcted effort later on

	ntarg = (int) (bm->FISHERYprms[nf][ntargets_id]);
	if (ntarg < 1)
		ntarg = 1;

	totexpect_return = 0;
	for (month = 0; month < 12; month++) {
		/* Effort schedule is initially set by summing the spatial black book values over all boxes in the model */
		ExpEffort = bm->EffortSchedule[nf][ns][month][hist_id];
		for (sp = 0; sp < bm->K_num_tot_sp; sp++) {
			if (FunctGroupArray[sp].isFished == TRUE) {
				/* If can't catch the group skip ahead */
				if (!bm->SP_FISHERYprms[sp][nf][q_id])
					continue;

				/* Get the expected profit */
				ans = Get_Expected_Profit(bm, nf, ns, sp, month, ntarg, ExpEffort, "YearCheck", do_debug, llogfp);

				/* pi = projected income */
				pi[nf][ns][month][sp] = ans;
				totexpect_return += ans;

				if (do_debug) {
					fprintf(llogfp, "pi[%s][%d][%d][%s]: %e (totexpect_return: %e)\n", FisheryArray[nf].fisheryCode, ns, month, FunctGroupArray[sp].groupCode,
							pi[nf][ns][month][sp], totexpect_return);
				}
			}
		}

		if (!bm->OrigEconCalc) {
			totexpect_return -= (TripCost * ExpEffort);
			//totexpect_return -= TripCost;
		}
	}
	if (totexpect_return < 0)
		totexpect_return = 0;

	if (do_debug) {
		fprintf(llogfp, "Time: %e %s totexpect_return %e (TripCost: %e, ExpEffort: %e)\n", bm->dayt, FisheryArray[nf].fisheryCode, totexpect_return, TripCost,
				ExpEffort);
	}

	/* Normalise pi for stochastic cases so can use it in a probabilistic sense to get final
	 effort allocation contribution. Also determine new expected catch for the year based
	 on Black Book entries
	 */
	if (totexpect_return > 0) {
		for (sp = 0; sp < bm->K_num_tot_sp; sp++) {
			if (FunctGroupArray[sp].isFished == TRUE) {

				/* If can't catch the group skip ahead */
				if (!bm->SP_FISHERYprms[sp][nf][q_id])
					continue;
				all_pi[sp] = 0;
				orig_pi[sp] = 1;
				calcTax = max(bm->SP_FISHERYprms[sp][nf][FixedMinTax_id], bm->SP_FISHERYprms[sp][nf][tax_id] * bm->SP_FISHERYprms[sp][nf][saleprice_id]);
				for (month = 0; month < 12; month++) {

					/* if using stocastic effort normalise ppi */
					if (bm->stochastic_effort)
						pi[nf][ns][month][sp] /= (totexpect_return + small_num);
					all_pi[sp] += pi[nf][ns][month][sp];
					bm->QuotaAlloc[nf][ns][sp][spexpectcatch_id] += bm->BlackBook[nf][ns][sp][month][expect_id];
					bm->QuotaAlloc[nf][ns][sp][spexpectvalue_id] += bm->BlackBook[nf][ns][sp][month][expect_id] * (bm->SP_FISHERYprms[sp][nf][saleprice_id]
							- calcTax);
					bm->MonthAlloc[nf][ns][sp][month] = bm->BlackBook[nf][ns][sp][month][expect_id];

					if (do_debug) {
						fprintf(llogfp, "Time: %e %s-%d %s mth: %d QuotaAlloc-expected %e (BlackBook: %e and pi[%s][%d][%d][%s]: %e)\n", bm->dayt,
								FisheryArray[nf].fisheryCode, ns, FunctGroupArray[sp].groupCode, month, bm->QuotaAlloc[nf][ns][sp][spexpectcatch_id],
								bm->BlackBook[nf][ns][sp][month][expect_id], FisheryArray[nf].fisheryCode, ns, month, FunctGroupArray[sp].groupCode,
								pi[nf][ns][month][sp]);
					}

				}
				if (bm->stochastic_effort)
					raw_pi[sp] = all_pi[sp] / (totexpect_return + small_num);
			}
		}
	}
	return totexpect_return;
}
/**
 * \brief Identify species to target - will find the top ntarget species in the list.
 */
void Identify_Target_Species(MSEBoxModel *bm, int nf, int ns, FILE *llogfp) {
	int sp, nt, ntt, ntarg, newtarg, found_it, last_sp;
	double check_value;
	/** Identify species to target - will find the top ntarget species in the list. **/
	for (sp = 0; sp < bm->K_num_tot_sp; sp++) {
		if (FunctGroupArray[sp].isFished == TRUE) {
			tempTarget[sp] = -1;
			maxTargetCatch[sp] = 0;
		}
	}

	ntarg = 1;
	for (sp = 0; sp < bm->K_num_tot_sp; sp++) {
		if (FunctGroupArray[sp].isFished == TRUE) {

			/* If can't catch the group skip ahead */
			if (!bm->SP_FISHERYprms[sp][nf][q_id])
				continue;
			/* Find species providing highest expected catches */
			nt = 0;
			while (nt < ntarg) {
				if (bm->UseVPUE)
					check_value = bm->QuotaAlloc[nf][ns][sp][spexpectvalue_id];
				else
					check_value = bm->QuotaAlloc[nf][ns][sp][spexpectcatch_id];

				if (check_value > maxTargetCatch[nt]) {
					for (ntt = ntarg - 1; ntt > nt - 1; ntt--) {
						/* Shuffle existing records out of the way */
						maxTargetCatch[ntt + 1] = maxTargetCatch[ntt];
						tempTarget[ntt + 1] = tempTarget[ntt];
					}
					/* Add new record */
					maxTargetCatch[nt] = check_value;
					tempTarget[nt] = sp;
					ntarg++;
					/* As started from zero and checking in descending order
					 (as sorted in that order) then should be ok to bail
					 as soon as enter the new value
					 */
					break;
				}
				nt++;
			}
		}
	}
	/* Update new number of target groups */
	bm->FISHERYprms[nf][ntargets_id] = ntarg;
	fprintf(bm->logFile, "ntarg = %d\n", ntarg);

	/** Check for targeting shifts **/
	for (nt = 0; nt < ntarg; nt++) {
		newtarg = tempTarget[nt];
		/* New target identified */
		if (newtarg != -1) {
			if (!bm->SUBFISHERYtarget[nf][ns][newtarg]) {
				fprintf(llogfp, "Time: %e, %s subfleet %d switching to target %s\n", bm->dayt, FisheryArray[nf].fisheryCode, ns,
						FunctGroupArray[newtarg].groupCode);
			}
			bm->SUBFISHERYtarget[nf][ns][newtarg] = nt;
		}
	}
	/* Strip out old targets */
	for (sp = 0; sp < bm->K_num_tot_sp; sp++) {
		if (FunctGroupArray[sp].isFished == TRUE) {
			found_it = 0;
			for (nt = 0; nt < ntarg; nt++) {
				if (tempTarget[nt] == sp){
					found_it = 1;
					break;
				}
			}
			if (!found_it && (bm->SUBFISHERYtarget[nf][ns][sp] > 0)) {
				fprintf(llogfp, "Time: %e, %s subfleet %d switching away from %s\n",
						bm->dayt, FisheryArray[nf].fisheryCode, ns, FunctGroupArray[sp].groupCode);
				bm->SUBFISHERYtarget[nf][ns][sp] = 0;
			}
			/* Update overall fleet targetting list - for use in harvest model and quota trading model */
			if (found_it)
				bm->FISHERYtarget[nf][sp] = 1;
		}
	}
	/* Report current targets */
	ntarg = (int) (bm->FISHERYprms[nf][ntargets_id]);

	last_sp = -1;
	fprintf(llogfp, "Time: %e %s-%d, ntarg = %d, targets ", bm->dayt, FisheryArray[nf].fisheryCode, ns, ntarg);
	for (nt = 0; nt < ntarg; nt++) {
		sp = tempTarget[nt];
		/* As each group should only be entered once can kill the iteration once hit duplicate group */
		if (sp == last_sp){
			break;
		}

		last_sp = sp;

		/* If current species actually set to be beyond fished group list then let it go the
		 first time by skipping ahead as if happens again will be caught by the last_sp check
		 If can't catch the group skip ahead
		 */
		if (sp == -1 || FunctGroupArray[sp].isFished == FALSE || bm->SP_FISHERYprms[sp][nf][q_id] == FALSE)
			continue;

		fprintf(llogfp, "%s ", FunctGroupArray[sp].groupCode);
	}
	fprintf(llogfp, "\n");
}

/**
 * \brief More deterministic effort allocation - this is used in most models
 */
void Dynamic_Effort_Allocation(MSEBoxModel *bm, int nf, int ns, double max_month_effort, int do_debug, FILE *llogfp) {

	double totppi, tot_expecteffort, tolerable_debt, potential_totppi, oldtotpi, stepE, this_profit, this_effort, rescale_effort, rescale_needed, prop_olease,
			prop_tlease, quota_avail, cumcatch, expectedCatch, ExpExposure, expectedEffort;
	int month, sp;
	double TripCost = bm->SUBFLEET_ECONprms[nf][ns][pereffort_cost_ind_id];
	double ExpEffort;
	double TOTexpectedCatch = 0.0;
	/** Using more deterministic effort allocation - this is used in most models.**/

	/* Find total and monthly potential profit contributions */
	totppi = 0;
	tot_expecteffort = 0;
	for (month = 0; month < 12; month++) {
		for (sp = 0; sp < bm->K_num_tot_sp; sp++) {
			if (FunctGroupArray[sp].isFished == TRUE) {
				/* If can't catch the group skip ahead */
				if (!bm->SP_FISHERYprms[sp][nf][q_id])
					continue;

				/* Weight based on contribution to total pi */
				totppi += pi[nf][ns][month][sp];

				/* Total expected effort */
				tot_expecteffort += bm->EffortSchedule[nf][ns][month][hist_id];

				if (do_debug) {
					fprintf(llogfp, "Time: %e, %s-%d for %s for mth: %d, add pi: %e totppi: %e\n", bm->dayt, FisheryArray[nf].fisheryCode, ns,
							FunctGroupArray[sp].groupCode, month, pi[nf][ns][month][sp], totppi);
				}
			}
		}
	}

	/* If using total catch - total cost model apply costs to get profitability */
	if (!bm->OrigEconCalc) {
		//totppi -= TripCost;
		tolerable_debt = -1.0 * bm->SUBFLEET_ECONprms[nf][ns][tol_debt_id];
		potential_totppi = totppi - (TripCost * tot_expecteffort);

		if ((bm->dayt < 1) || (bm->dayt < (bm->hist_only_timeout + 360.0)))
			oldtotpi = potential_totppi;
		else
			oldtotpi = bm->SUBFLEET_ECONprms[nf][ns][last_pi_id];

		if (potential_totppi < tolerable_debt) {
			/* Check to see if can contract effort and if still leads to too high debt
			 level then don't fish at all */

			//stepE = ExpEffort;
			stepE = tot_expecteffort;
			ExpEffort = (totppi - tolerable_debt) / (TripCost + small_num);
			if (ExpEffort < 0.0)
				ExpEffort = 0.0;

			totppi = oldtotpi * ExpEffort / (stepE + small_num);
		} else if (potential_totppi < 0.0) {
			/* Else just make sure contract effort appropriately */
			totppi *= (oldtotpi / (TripCost * tot_expecteffort));
		} else
			totppi -= (TripCost * tot_expecteffort);
	}

	/* Initialise last_pi if at model start or only just ended the historical period */
	if ((bm->dayt < 1) || (bm->dayt < (bm->hist_only_timeout + 360.0)))
		bm->SUBFLEET_ECONprms[nf][ns][last_pi_id] = totppi;

	bm->SUBFLEET_ECONprms[nf][ns][AnnualExpectPI_id] = totppi; // This is total expected returns (prices * catches - costs) that influences decisions

	/* Allocate effort in proportion to gross profitability of fishing in this month */
	for (month = 0; month < 12; month++) {
		/* Look at this year expected profitability vs last years and scale effort accordingly */
		this_profit = totppi / (bm->SUBFLEET_ECONprms[nf][ns][last_pi_id] + small_num);
		this_effort = bm->EffortSchedule[nf][ns][month][hist_id] * this_profit;

		/* Do initial allocation  */
		bm->EffortSchedule[nf][ns][month][expect_id] = min(max_month_effort, this_effort);

		if (do_debug) {
			fprintf(llogfp, "Time: %e, %s-%d month %d EffortSchedule: %e, this_profit: %e (totppi: %e, lastpi: %e), this_effort: %e, hist_effort: %e\n",
					bm->dayt, FisheryArray[nf].fisheryCode, ns, month, bm->EffortSchedule[nf][ns][month][expect_id], this_profit, totppi,
					bm->SUBFLEET_ECONprms[nf][ns][last_pi_id], this_effort, bm->EffortSchedule[nf][ns][month][hist_id]);
		}
	}
	/* Check whether this level of effort makes sense given current size of quotas */
	rescale_effort = 0;
	rescale_needed = 0;
	expectedEffort = 0;
	for (sp = 0; sp < bm->K_num_tot_sp; sp++) {
		if (FunctGroupArray[sp].isFished == TRUE) {

			/* If can't catch the group skip ahead */
			if (!bm->SP_FISHERYprms[sp][nf][q_id])
				continue;

			cumcatch = 0;
			for (month = 0; month < 12; month++) {
				/* New expected catch for the month selected = current effort * historical CPUE */
				expectedCatch
						= bm->EffortSchedule[nf][ns][month][expect_id] * bm->BlackBook[nf][ns][sp][month][expect_id]
								/ (bm->EffortSchedule[nf][ns][month][hist_id] + small_num);

				expectedEffort += bm->EffortSchedule[nf][ns][month][expect_id];
				TOTexpectedCatch += expectedCatch;

				if (bm->TemporalBycatchAvoid) {
					/* If taking temporal exposure to bycatch species (and discards) into account when scheduling effort */
					ExpExposure = FunctGroupArray[sp].speciesParams[sp_concern_id] * bm->BlackBook[nf][ns][sp][month][bycatch_id];
				} else
					ExpExposure = 0;

				cumcatch += (expectedCatch + ExpExposure);

			}

			/* Update quota_avail - take away new expected catch to see amount remaining */
			prop_olease = bm->QuotaAlloc[nf][ns][sp][permolease_id];
			prop_tlease = bm->QuotaAlloc[nf][ns][sp][templease_id];
			quota_avail = (1.0 - prop_olease - prop_tlease) * bm->QuotaAlloc[nf][ns][sp][owned_id] + bm->QuotaAlloc[nf][ns][sp][leased_id] - cumcatch;

			/* Add tradable quota to represent the potential (hope) for trade */
			quota_avail += tradeable_quota[sp];
			quota_avail *= 0.01; // Converted from kg to 100kg
			quota_avail = floor(quota_avail + 0.5); // So will not bother chasing final 50kg of quota

			/* If a target species and excess effort given the quota then scale back - in proportion
			 to the percieved value of that species to the fleet.
			 */
			if (!bm->fish_withoutQ && (quota_avail < 0)) {
				rescale_effort += (all_pi[sp] / (totppi + small_num)) * ((1.0 - prop_olease - prop_tlease) * bm->QuotaAlloc[nf][ns][sp][owned_id]
						+ bm->QuotaAlloc[nf][ns][sp][leased_id]) / (cumcatch + small_num);
				rescale_needed = 1;

                /*
				if (do_debug) {
					fprintf(
							llogfp,
							"Time: %e, %s-%d rescale_effort: %e, all_pi[%s]: %e, totppi: %e, quota: %e, cumcatch: %e, prop_olease: %e, prop_tlease: %e, bm->QuotaAlloc[owned_id]: %e, bm->QuotaAlloc[leased_id]: %e, tradeable_quota: %e\n",
							bm->dayt, FisheryArray[nf].fisheryCode, ns, rescale_effort, FunctGroupArray[sp].groupCode, all_pi[sp], totppi, ((1.0 - prop_olease
									- prop_tlease) * bm->QuotaAlloc[nf][ns][sp][owned_id] + bm->QuotaAlloc[nf][ns][sp][leased_id]), cumcatch, prop_olease,
							prop_tlease, bm->QuotaAlloc[nf][ns][sp][owned_id], bm->QuotaAlloc[nf][ns][sp][leased_id], tradeable_quota[sp]);
				}
                */

			} else {
				rescale_effort += (all_pi[sp] / (totppi + small_num));
			}
		}
	}

	bm->SUBFLEET_ECONprms[nf][ns][Expectedeffort_id] = expectedEffort;
	bm->SUBFLEET_ECONprms[nf][ns][ExpectedCatch_id] = TOTexpectedCatch;

	if (rescale_needed) {

		if (do_debug) {
			fprintf(llogfp, "Time: %e, %s-%d rescale_effort: %e\n", bm->dayt, FisheryArray[nf].fisheryCode, ns, rescale_effort);
		}

		for (month = 0; month < 12; month++) {
			bm->EffortSchedule[nf][ns][month][expect_id] *= rescale_effort;
		}
	}

	/* Set last_pi for use next year */
	bm->SUBFLEET_ECONprms[nf][ns][last_pi_id] = totppi;

}

/**
 * \brief Stochastic effort allocation
 */
void Stocastic_Effort_Allocation(MSEBoxModel *bm, int nf, int ns, int *not_bounded, double totexpect_return, double max_month_effort, int do_debug,
		FILE *llogfp) {

	int last_sp, nt, sp, month, itc, checkmonth, do_extra;
	double prop_olease, prop_tlease, quota_avail, cumcatch, pi_bound = 0, final_toteffort;
	double rndmax, rndnum, cum_pi, lost_pi, new_bound, expectedCatch, ExpExposure;
	double totneweffort, totoldeffort, totallpi, totppi, allocedeffort, remain_effort, totpi;
	double TripLength = bm->SUBFLEET_ECONprms[nf][ns][max_trip_length_id];
	int ntarg = (int)bm->FISHERYprms[nf][ntargets_id];

	last_sp = -1;
	for (nt = 0; nt < ntarg; nt++) { // TO FIX - This may need to change to species loop if targetting is off
		sp = tempTarget[nt];
		/* As each group should only be entered once can kill the iteration once hit duplicate group */
		if (sp == last_sp)
			break;
		last_sp = sp;

		/* If current species actually set to be beyond fished group list then let it go the
		 first time by skipping ahead as if happens again will be caught by the last_sp check
		 If can't catch the group skip ahead
		 */
		if (sp == -1 || FunctGroupArray[sp].isFished == FALSE || bm->SP_FISHERYprms[sp][nf][q_id] == FALSE)
			continue;

		/* Find pi_bound for use later on */
		if (*not_bounded) {
			pi_bound = 0;
			for (month = 0; month < 12; month++) {
				pi_bound += pi[nf][ns][month][sp];
			}
			*not_bounded = 0;
		}

		// As no quota used caught yet available quota is currently held quota
		prop_olease = bm->QuotaAlloc[nf][ns][sp][permolease_id];
		prop_tlease = bm->QuotaAlloc[nf][ns][sp][templease_id];
		quota_avail = (1.0 - prop_olease - prop_tlease) * bm->QuotaAlloc[nf][ns][sp][owned_id] + bm->QuotaAlloc[nf][ns][sp][leased_id];
		quota_avail *= 0.01; // Converted from kg to 100kg
		quota_avail = floor(quota_avail + 0.5); // So will not bother chasing final 50kg of quota

		if (do_debug) {
			fprintf(
					llogfp,
					"Setup annual effort allocation for %s-%d sp: %s - quota_avail: %e prop_olease: %e, prop_tlease: %e, owned: %e, leased: %e, all_pi: %e, pi_bound: %e)\n",
					FisheryArray[nf].fisheryCode, ns, FunctGroupArray[sp].groupCode, quota_avail, prop_olease, prop_tlease,
					bm->QuotaAlloc[nf][ns][sp][owned_id], bm->QuotaAlloc[nf][ns][sp][leased_id], all_pi[sp], pi_bound);
		}

		if (all_pi[sp] < small_num) {
			continue;
		}

		itc = 0;
		cumcatch = 0;
		while ((((!bm->fish_withoutQ) && (quota_avail > 0)) || totexpect_return > 0) && (all_pi[sp] > 0)) {
			itc++;
			/* Adjust range of rand number in relation to pi_bound so in cases where all pi small as lots
			 of targets with about even catch composition aren't stuck with range (0,1) and missing
			 small pi values in rndnum comparison.
			 */
			rndmax = min(1.0, pi_bound);
			rndnum = drandom(0.0, rndmax);
			checkmonth = -1;
			cum_pi = 0;
			for (month = 0; month < 12; month++) {
				/* Find first month where can expect to catch this proportion of the annual catch */
				cum_pi += pi[nf][ns][month][sp];

				if (do_debug) {
					fprintf(llogfp, "Allocating effort for %s-%d sp: %s - checking month %d (rndnum: %e vs pi: %e)\n", FisheryArray[nf].fisheryCode, ns,
							FunctGroupArray[sp].groupCode, month, rndnum, cum_pi);
				}

				if (rndnum < cum_pi) {
					checkmonth = month;

					if (do_debug) {
						fprintf(llogfp, "Allocating to month: %d\n", checkmonth);
					}
					break;
				}
			}

			if (itc > 1000) {
				fprintf(llogfp, "Had to bail on 100th iteration for %s-%d sp: %s\n", FisheryArray[nf].fisheryCode, ns, FunctGroupArray[sp].groupCode);
				quota_avail = 0;
				totexpect_return = 0;
				break;
			}

			if (checkmonth < 0)
				continue;

			/* Calculate in effort days and then correct to units used in the harvest library */
			bm->EffortSchedule[nf][ns][checkmonth][expect_id] += min(TripLength, max_month_effort - bm->EffortSchedule[nf][ns][checkmonth][expect_id]);

			if ((bm->EffortSchedule[nf][ns][checkmonth][expect_id] > 1e10) || (bm->EffortSchedule[nf][ns][checkmonth][expect_id] < 0) || (do_debug)) {
				fprintf(llogfp, "Allocating effort for %s-%d sp: %s month: %d (pi: %e) - expect_effort %e vs max_effort: %e\n", FisheryArray[nf].fisheryCode,
						ns, FunctGroupArray[sp].groupCode, checkmonth, pi[nf][ns][month][sp], bm->EffortSchedule[nf][ns][checkmonth][expect_id],
						max_month_effort);
			}

			if (bm->EffortSchedule[nf][ns][checkmonth][expect_id] < 0)
				bm->EffortSchedule[nf][ns][checkmonth][expect_id] = 0;

			/* If now have a full month of effort for checkmonth then reset pi so don't try this month again */
			if (bm->EffortSchedule[nf][ns][checkmonth][expect_id] >= max_month_effort) {
				lost_pi = pi[nf][ns][checkmonth][sp];
				orig_pi[sp] -= pi[nf][ns][checkmonth][sp] / all_pi[sp];
				all_pi[sp] = 0;
				pi[nf][ns][checkmonth][sp] = 0;
				totexpect_return = 0;
				new_bound = 0;
				for (month = 0; month < 12; month++) {
					/* Find remnants of totexpect_return */
					totexpect_return += pi[nf][ns][month][sp];
					/* Renormalise pi */
					pi[nf][ns][month][sp] /= (1.0 - lost_pi + small_num);
					all_pi[sp] += pi[nf][ns][month][sp];
					new_bound += pi[nf][ns][month][sp];
				}
				if (new_bound > pi_bound)
					pi_bound = new_bound;
			}

			/* New expected catch for the month selected = current effort * historical CPUE */
			expectedCatch = bm->EffortSchedule[nf][ns][checkmonth][expect_id] * bm->BlackBook[nf][ns][sp][checkmonth][expect_id]
					/ (bm->EffortSchedule[nf][ns][checkmonth][hist_id] + small_num);

			if (bm->TemporalBycatchAvoid) {
				/* If taking temporal exposure to bycatch species (and discards) into account when scheduling effort */
				ExpExposure = FunctGroupArray[sp].speciesParams[sp_concern_id] * bm->BlackBook[nf][ns][sp][checkmonth][bycatch_id];
			} else
				ExpExposure = 0;

			cumcatch += (expectedCatch + ExpExposure);

			/* Update quota_avail - take away new expected catch to see amount remaining */
			quota_avail = (1.0 - prop_olease - prop_tlease) * bm->QuotaAlloc[nf][ns][sp][owned_id] + bm->QuotaAlloc[nf][ns][sp][leased_id] - cumcatch;
			quota_avail *= 0.01; // Converted from kg to 100kg
			quota_avail = floor(quota_avail + 0.5); // So will not bother chasing final 50kg of quota

            /*
			if (do_debug) {
				fprintf(
						llogfp,
						"Allocating effort for %s-%d sp: %s - new_cum: %e, (EffortSchedule[checkmonth - %d][expect]: %e, BlackBook: %e, EffortSchedule[hist]: %e, quota_avail: %e, prop_olease: %e, prop_tlease: %e, QuotaAlloc[owned]: %e, QuotaAlloc[leased_id]: %e\n",
						FisheryArray[nf].fisheryCode, ns, FunctGroupArray[sp].groupCode, cumcatch, checkmonth,
						bm->EffortSchedule[nf][ns][checkmonth][expect_id], bm->BlackBook[nf][ns][sp][checkmonth][expect_id],
						bm->EffortSchedule[nf][ns][checkmonth][hist_id], quota_avail, prop_olease, prop_tlease, bm->QuotaAlloc[nf][ns][sp][owned_id],
						bm->QuotaAlloc[nf][ns][sp][leased_id]);
			}
            */

		}

		/* Interim expected catches - as planned effort * historical CPUE per month - for finding new target species
		 so don't include bycatch incentives here (as will end up targetting the wrong things!) */
		expectedCatch = 0;
		for (month = 0; month < 12; month++) {
			expectedCatch
					+= bm->EffortSchedule[nf][ns][month][expect_id] * bm->BlackBook[nf][ns][sp][month][expect_id]
							/ (bm->EffortSchedule[nf][ns][month][hist_id] + small_num);  // Denominator previously used MofY not month
		}
		bm->QuotaAlloc[nf][ns][sp][spexpectcatch_id] = expectedCatch;
	}

	/** Do deterministic allocation of remaining effort - for the pi missed above as too small_num, but
	 can sum to weighty percentages
	 **/
	/* Start with determining magnitude of effort to play with */
	totneweffort = 0;
	totoldeffort = 0;
	for (month = 0; month < 12; month++) {
		totneweffort += bm->EffortSchedule[nf][ns][month][expect_id];
		totoldeffort += bm->EffortSchedule[nf][ns][month][hist_id];

		if (do_debug) {
			fprintf(llogfp, "Time: %e, %s-%d month %d start EffortSchedule: %e (vs old: %e - old/max: %e)\n", bm->dayt, FisheryArray[nf].fisheryCode, ns,
					month, bm->EffortSchedule[nf][ns][month][expect_id], bm->EffortSchedule[nf][ns][month][hist_id], bm->EffortSchedule[nf][ns][month][hist_id]
							/ max_month_effort);
		}

	}
	totallpi = 0;
	totppi = 0;
	last_sp = -1;
	for (nt = 0; nt < ntarg; nt++) { // TO FIX - This may need to change to a species loop if targetting skew-if
		sp = tempTarget[nt];
		/* As each group should only be entered once can kill the iteration once hit duplicate group */
		if ((sp == last_sp) || (!bm->FISHERYprms[nf][flagMultiSpEffort_id] && nt))
			break;
		last_sp = sp;

		/* If current species actually set to be beyond fished group list then let it go the
		 first time by skipping ahead as if happens again will be caught by the last_sp check
		 If can't catch the group skip ahead
		 */
		if (sp == -1 || FunctGroupArray[sp].isFished == FALSE || bm->SP_FISHERYprms[sp][nf][q_id] == FALSE)
			continue;

		/* Make sure this is making sense */
		if (orig_pi[sp] < 0)
			orig_pi[sp] = 0;

		/* Look at weighted contributions to final state */
		totallpi += (orig_pi[sp] * raw_pi[sp]);
		for (month = 0; month < 12; month++) {
			totppi += (pi[nf][ns][month][sp] * raw_pi[sp]);
		}
	}

	/* If not primary target and no effort yet allocated for this lower order spp just skip
	 ahead unless specifically interested
	 */
	do_extra = 1;
	if (!totallpi) {
		/* None to do so skip ahead here */
		do_extra = 0;
	}

	if (do_extra) {
		if (totallpi > 1)
			quit("How got more than 1 for a normalised totallpi value?\n");

		/* So that numerics don't get enormous (as dividing by very small numbers) below
		 assume at least 1% of the pi related effort already allocated.
		 */
		if (totallpi > 0.99)
			totallpi = 0.99;

		if (totneweffort) {
			/* Find remaining effort based on remaining pi and existing effort levels (if any) */
			allocedeffort = totneweffort;
		} else if (totoldeffort) {
			/* Otherwise allocate effort based on remaining pi and historical effort levels (if any) */
			allocedeffort = totoldeffort;
		} else
			allocedeffort = 0;

		final_toteffort = allocedeffort / (1 - totallpi);
		remain_effort = final_toteffort - allocedeffort;

		/* Allocate effort across months - based on sums of remaining pi */
		if (remain_effort > 0) {
			for (month = 0; month < 12; month++) {
				totpi = 0;
				last_sp = -1;
				for (nt = 0; nt < ntarg; nt++) { // TO FIX - This may need to change to a species loop if targetting off
					sp = tempTarget[nt];
					/* As each group should only be entered once can kill the iteration once hit duplicate group */
					if ((sp == last_sp) || (!bm->FISHERYprms[nf][flagMultiSpEffort_id] && nt))
						break;
					last_sp = sp;

					/* If current species actually set to be beyond fished group list then let it go the
					 first time by skipping ahead as if happens again will be caught by the last_sp check
					 If can't catch the group skip ahead
					 */
					if (sp == -1 || FunctGroupArray[sp].isFished == FALSE || bm->SP_FISHERYprms[sp][nf][q_id] == FALSE)
						continue;

					/* Weight based on contribution to total pi */
					totpi += (pi[nf][ns][month][sp] * raw_pi[sp]);
				}
				/* Allocate remaining effort in proportion to gross profitability of fishing in this month */
				bm->EffortSchedule[nf][ns][month][expect_id] += (totpi / (totppi + small_num)) * remain_effort;

				if (bm->EffortSchedule[nf][ns][month][expect_id] < 0) {
					fprintf(
							llogfp,
							"Time: %e, %s-%d month %d EffortSchedule: %e, totpi: %e, totppi: %e, totallpi: %e, remain_effort: %e (ratio: %e, final_toteffort: %e, allocedeffort: %e)\n",
							bm->dayt, FisheryArray[nf].fisheryCode, ns, month, bm->EffortSchedule[nf][ns][month][expect_id], totpi, totppi, totallpi,
							remain_effort, (totpi / (totppi + small_num)), final_toteffort, allocedeffort);
					bm->EffortSchedule[nf][ns][month][expect_id] = 0;
				}

				if (bm->EffortSchedule[nf][ns][month][expect_id] > 1e10) {
					fprintf(
							llogfp,
							"Time: %e, %s-%d month %d EffortSchedule: %e, totpi: %e, totppi: %e, totallpi: %e, remain_effort: %e (ratio: %e, final_toteffort: %e, allocedeffort: %e)\n",
							bm->dayt, FisheryArray[nf].fisheryCode, ns, month, bm->EffortSchedule[nf][ns][month][expect_id], totpi, totppi, totallpi,
							remain_effort, (totpi / (totppi + small_num)), final_toteffort, allocedeffort);
					quit("EffortSchedule muckup in pi alloc\n");
				}

				if (do_debug) {
					fprintf(
							llogfp,
							"Time: %e, %s-%d month %d final EffortSchedule: %e, totpi: %e, totppi: %e, totallpi: %e, remain_effort: %e (ratio: %e, final_toteffort: %e, allocedeffort: %e)\n",
							bm->dayt, FisheryArray[nf].fisheryCode, ns, month, bm->EffortSchedule[nf][ns][month][expect_id], totpi, totppi, totallpi,
							remain_effort, (totpi / (totppi + small_num)), final_toteffort, allocedeffort);
				}
			}
		}
	}
}

/**
 * \brief Calculate the spatial allocations
 */
void Calculate_Box_Allocation(MSEBoxModel *bm, int nf, int ns, int do_debug, FILE *llogfp) {
	int month, ij, sp;
	double expectedEffort, expectedCatch, rescale_ratio;

	expectedEffort = 0;
	expectedCatch = 0;
	for (month = 0; month < 12; month++) {
		/** Determine general effort location. Interpolate between historical map of CPUE and
		 last years map of CPUE - recorded at monthly level. Do this by finding difference
		 between historical and last years total CPUE map for this subfleet, weighting
		 it based on strength of memory (or flexibility to change) and add that weighted
		 amount to the historical mapvto get the new map
		 **/

		if (do_debug) {
			fprintf(llogfp, "%s-%d month %d expectedeffort = %e hist: %e flexweight: %e\n", FisheryArray[nf].fisheryCode, ns, month,
					bm->EffortSchedule[nf][ns][month][expect_id], bm->BoxAlloc[nf][ns][month][hist_id], bm->SUBFLEET_ECONprms[nf][ns][flexweight_id]);
		}

		bm->BoxAlloc[nf][ns][month][expect_id] = bm->EffortSchedule[nf][ns][month][expect_id];
		bm->BoxAlloc[nf][ns][month][current_id] = bm->SUBFLEET_ECONprms[nf][ns][flexweight_id] * (bm->BoxAlloc[nf][ns][month][expect_id]
				- bm->BoxAlloc[nf][ns][month][hist_id]) + bm->BoxAlloc[nf][ns][month][hist_id];
		if (bm->BoxAlloc[nf][ns][month][current_id] < 0)
			bm->BoxAlloc[nf][ns][month][current_id] = 0;

		/* Consolidate effort allocation */
		for (ij = 0; ij < bm->nbox; ij++) {
			if (bm->boxes[ij].type != BOUNDARY) {
				bm->Effort_hdistrib[ij][nf][month] += bm->BoxAlloc[nf][ns][month][current_id] * bm->SpatialBlackBook[nf][ns][month][ij][updated_id];

				if (do_debug) {
					fprintf(llogfp, "%s-%d month: %d Effort_hdistrib-%d = %e (Boxalloc: %e, SpatialBlackBook: %e)\n", FisheryArray[nf].fisheryCode, ns, month,
							ij, bm->Effort_hdistrib[ij][nf][month], bm->BoxAlloc[nf][ns][month][current_id], bm->SpatialBlackBook[nf][ns][month][ij][hist_id]);
				}
			}
		}
		expectedEffort += bm->BoxAlloc[nf][ns][month][current_id];
		/* Update expected effort - expectation is always toward larger value */
		if (bm->BoxAlloc[nf][ns][month][current_id] > bm->EffortSchedule[nf][ns][month][expect_id])
			bm->EffortSchedule[nf][ns][month][expect_id] = bm->BoxAlloc[nf][ns][month][current_id];

		if (do_debug) {
			fprintf(llogfp, "Time: %e %s-%d month %d totexpectedeffort = %e (vs current: %e vs hist: %e and aggregate: %e)\n", bm->dayt,
					FisheryArray[nf].fisheryCode, ns, month, expectedEffort, bm->BoxAlloc[nf][ns][month][current_id], bm->BoxAlloc[nf][ns][month][hist_id],
					bm->FISHERYprms[nf][EffortLevel_id]);
		}

		/* Finalise expected catches - as planned effort * historical CPUE per month */
		for (sp = 0; sp < bm->K_num_tot_sp; sp++) {
			if (FunctGroupArray[sp].isFished == TRUE) {
				/* If can't catch the group skip ahead */
				if (!bm->SP_FISHERYprms[sp][nf][q_id])
					continue;

				expectedCatch += bm->EffortSchedule[nf][ns][month][expect_id] * bm->BlackBook[nf][ns][sp][month][expect_id]
						/ (bm->EffortSchedule[nf][ns][bm->MofY][hist_id] + small_num);

				/* Rescale monthly expected catch too */
				rescale_ratio = bm->BoxAlloc[nf][ns][month][current_id] / (bm->EffortSchedule[nf][ns][month][expect_id] + small_num);
				if (rescale_ratio > 1)
					bm->MonthAlloc[nf][ns][sp][month] *= rescale_ratio;
			}
		}
	}
	/* Calculate total scheduled effort and expected catch */
	bm->SUBFLEET_ECONprms[nf][ns][totPlanEffort_id] = expectedEffort;
	bm->SUBFLEET_ECONprms[nf][ns][totPlanCatch_id] = expectedCatch;
}

/**************************************************************************************************************************
*
* \brief  Rescale initial conditions at the end of the historical period
*
*  This is so can go form light burn-in exploitation to high depletion rates for depletion experiements
*
***********/
void Do_EconArray_Rescaling(MSEBoxModel *bm) {
    int nf, ns, month, sp;

	for (nf = 0; nf < bm->K_num_fisheries; nf++) {
        for (ns = 0; ns < bm->FISHERYprms[nf][nsubfleets_id]; ns++) {
            for (month = 0; month < 12; month++) {
                bm->EffortSchedule[nf][ns][month][hist_id] *= bm->EffortReScale[nf];
                bm->EffortSchedule[nf][ns][month][expect_id] *= bm->EffortReScale[nf];
                for (sp = 0; sp < bm->K_num_tot_sp; sp++) {
                    bm->BlackBook[nf][ns][sp][month][hist_id] *= bm->CatchReScale[sp];
                    bm->BlackBook[nf][ns][sp][month][expect_id] *= bm->CatchReScale[sp];
                }
            }
            for (sp = 0; sp < bm->K_num_tot_sp; sp++) {
                bm->QuotaAlloc[nf][ns][sp][permolease_id] *= bm->CatchReScale[sp];
                bm->QuotaAlloc[nf][ns][sp][templease_id] *= bm->CatchReScale[sp];
                bm->QuotaAlloc[nf][ns][sp][owned_id] *= bm->CatchReScale[sp];
            }
        }
    }
        
    return;
}
