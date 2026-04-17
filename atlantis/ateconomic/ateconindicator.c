/**
 \file
 \brief C file for economic models for Atlantis
 \ingroup ateconomic

 File:	ateconindicator.c
 Author:	Beth Fulton
 Created:	27/9/2005
 Purpose:	C file for economic models for Atlantis
 Revisions: 28/9/2005 Created the file from ateconomic.c

 29/9/2005 Added Economic_Indicators(),
 Simple_Econ_Indicators() and EconIndCalc() routines

 21/11/2006 removed "- cost" from calculation of
 bm->SUBFLEET_ECONprms[nf][ns][boat_cash_ind_id]
 in Dynamic_Econ_Indicators() as costs already
 included in calculation of the revenue term.

 02-02-2009 Bec Gorton
 Removed a stray semicolon in Dynamic_Econ_Indicators.

 29-10-2009 Bec Gorton
 Removed references to the K_num_fished_sp and K_num_impacted_sp. Arrays that were this
 size have been resized to K_num_tot_sp.
 Also changed the references to arrays that have been moved into the atHarvestModule.

 04-11-2009 Bec Gorton
 Merged in Beths new bycatch incentive code - revision 961.

 27-01-2010 Bec Gorton
 Replaced direct reading of the bm->harvestIndex values with Harvest_Get_Harvest_Index().
 Also change the code to use Harvest_Get_Num_Gear_Changes instead of directly calculating the
 number of possible gear changes.

 08-04-2010 Beth Fulton (for Bec Gorton)
 There were errant semicolons after loops in CalcSizeSpectra().
 They have been removed.


 12-05-2010 Bec Gorton
 Added a new slot in the QuotaAlloc array to store the deemed value for
 each species, fishery and subfleet. These values are now written out to a new
 output file called deemedValues.txt

 Changes to make:
 We might want to know about the crewshare - crews can be split across crews and owners.
 Crew payment is a proportion of the catch.

 **/

/*  Open library routines */
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <sjwlib.h>
#include <atEconomic.h>
#include "atHarvestLib.h"
#include <atAssessLib.h>

/**
 *	\brief Boxmodel economic indicators and statistics calculator
 *
 */
void Economic_Indicators(MSEBoxModel *bm, FILE *llogfp) {
	int nf, ns, flageconind;

	if (verbose > 1)
		printf("Calculating economic indicators\n");

	/* Get overall catch value */
	Total_Catch_Value(bm, llogfp);

	/* Calculate economic indicators */
	for (nf = 0; nf < bm->K_num_fisheries; nf++) {
		for (ns = 0; ns < bm->FISHERYprms[nf][nsubfleets_id]; ns++) {
			/* If no boats in the subfleet currently skip ahead */
			if (!bm->SUBFLEET_ECONprms[nf][ns][nboat_id]) {
				continue;
			}

			flageconind = (int) bm->SUBFLEET_ECONprms[nf][ns][flag_econind_id];

			switch (flageconind) {
			case simple_func:
				Simple_Econ_Indicators(bm, nf, ns, llogfp);
				break;
			case dynamic_func:
				Dynamic_Econ_Indicators(bm, nf, ns, llogfp);
				break;
			case expect_func:
				Expected_Indicators(bm, nf, ns, llogfp);
				break;
			default:
				quit("This flag_econind option is not coded yet, reset to %d\n", simple_func);
				break;
			}

		}
	}

	/* Calculate overall quota value */
	Total_Quota_Price(bm, llogfp);

	/* If appropriate, calculate profit margin dependent indicators */
	for (nf = 0; nf < bm->K_num_fisheries; nf++) {
		for (ns = 0; ns < bm->FISHERYprms[nf][nsubfleets_id]; ns++) {
			/* If no boats in the subfleet currently skip ahead */
			if (!bm->SUBFLEET_ECONprms[nf][ns][nboat_id]) {
				continue;
			}

			flageconind = (int) bm->SUBFLEET_ECONprms[nf][ns][flag_econind_id];

			if (flageconind == expect_func)
				Profit_Dependent_Indicators(bm, nf, ns, llogfp);
		}
	}

	return;
}

/**
 *
 *	\brief Simple functional responses mapping economic response to ecological
 *	indicators changes - data (in delphic sense) from Gerry Geen and Tom Kompas (ABARE)
 *
 *	@return Returns relative (normalised) indicators
 *
 *	Multiplicative and additive options given
 *
 */
void Simple_Econ_Indicators(MSEBoxModel *bm, int nf, int ns, FILE *llogfp) {
	int flageconindtype, ne, eind, neh, nehind;
	double ind_val;

	flageconindtype = (int) bm->SUBFLEET_ECONprms[nf][ns][flag_indtype_id];

	/******* Calculate indicators *******/
	/* Assumes the following order for the economic indicators
	 GVP is gross value of the product - which is the landed catch value
	 Revenue / ton landed
	 Revenue / day
	 Cost / day
	 Cash income = (revenue - costs)
	 Return on investment
	 Capital utilisation
	 Leased value of quota
	 Sale value of quota
	 Quota trade levels
	 Cost / ton
	 */

	ind_val = 0.0;
	for (ne = 0; ne < K_econ_indicators; ne++) {
		/* Initialise the economic indicator value */
		switch (flageconindtype) {
		case add_funcs:
			ind_val = 0.0;
			break;
		case mult_funcs:
			ind_val = 1.0;
			break;
		default:
			quit("Only multiplicative and additive options for economic indicators thus far. Reset to %d or %d\n", add_funcs, mult_funcs);
			break;
		}
		eind = ne + gvp_ind_id;

		/* Calculate individual economic indicator terms based on following
		 ecological or harvest values
		 CPUE
		 Yield
		 Size
		 Species composition
		 Spatial closures
		 Gear restrictions
		 Management costs
		 Research costs
		 Discards
		 */
		for (neh = 0; neh < K_ecolharvest_indicators; neh++) {
			nehind = cpue_ind_id + neh;
			switch (flageconindtype) {
			case add_funcs: /* Divide through by number of indicators so end with average result */
				ind_val += EconIndCalc(bm, nf, ns, ne, neh, nehind) / (K_ecolharvest_indicators + small_num);
				break;
			case mult_funcs:
				ind_val *= EconIndCalc(bm, nf, ns, ne, neh, nehind);
				break;
			default:
				quit("Only multiplicative and additive options for economic indicators thus far. Reset to %d or %d\n", add_funcs, mult_funcs);
				break;
			}
		}

		/* Assign the final value */
		bm->SUBFLEET_ECONprms[nf][ns][eind] = ind_val;
	}

	return;
}

/**
 * \brief Indicator calculator that returns the final value. The general form
 *	of functional response is implemented here and parameters stored in
 *	SUBFLEET_ECONprms and ECON_INDprms arrays
 *
 */
double EconIndCalc(MSEBoxModel *bm, int nf, int ns, int econind_id, int ecolharvind_id, int actharvind_id) {
	double ans, val, e_b, e_m;
	int ind_type;

	/* Initialise the ivalue of the term to be returned */
	ans = 1.0;

	val = bm->SUBFLEET_ECONprms[nf][ns][actharvind_id];
	ind_type = (int) (bm->ECON_INDprms[econind_id][ecolharvind_id][eindtype_ind]);

	switch (ind_type) {
	case simple_linear:
		ans = bm->ECON_INDprms[econind_id][ecolharvind_id][ecoefft_ind] + bm->ECON_INDprms[econind_id][ecolharvind_id][escale_ind] * val;
		break;
	case piecewise_linear:
		if (val > bm->ECON_INDprms[econind_id][ecolharvind_id][ethresh_ind])
			ans = bm->ECON_INDprms[econind_id][ecolharvind_id][ecoefft_ind] + bm->ECON_INDprms[econind_id][ecolharvind_id][escaleB_ind] * val;
		else
			ans = bm->ECON_INDprms[econind_id][ecolharvind_id][ecoefft_ind] + bm->ECON_INDprms[econind_id][ecolharvind_id][escale_ind] * val;
		break;	/* Bug fix - missing break statement */
	case asymptotic_curve:
		e_b = bm->ECON_INDprms[econind_id][ecolharvind_id][ecoefft_ind];
		e_m = bm->ECON_INDprms[econind_id][ecolharvind_id][escale_ind];
		ans = 1.0 / (1.0 + exp(-e_b * (val - e_m)));
		break;
	case humped_curve:
		ans = val * bm->ECON_INDprms[econind_id][ecolharvind_id][ecoefft_ind] * (1.0 - val / (bm->ECON_INDprms[econind_id][ecolharvind_id][escale_ind]
				+ small_num));
		break;
	}

	return ans;
}

/**
 *	\brief Dynamic economic indicator calculation based on simple equations
 *	relating costs and prices
 *
 *	@returns relative (normalised) indicators
 *
 */
void Dynamic_Econ_Indicators(MSEBoxModel *bm, int nf, int ns, FILE *llogfp) {
    int flagfcmpa, ij, tierid, sp;
    //int do_debug;
	double totMPA, numMPA, gear_change, numTAC, tier, reswork, fuel_cost_scalar, fuel_cost, capital_cost, gear_cost, fixed_cost, var_cost, revenue, gvp, current_catch, cost, trade_extent, numexchange, prop_olease, prop_tlease, lease_cost, quota_avail, invest, big_expenditure, fuel_intercept, in_quota, trend_coefft, month_coefft, auto_coefft, resid, unload_cost, catch_in_hold, maxlease_cost = 0, minrevenue, tax_revenue, quota, catch_count, dv_sum, dvprice, local_deemed_value;

	int key_id = 0; // As only one economic property time series thus far - FIX make this read from array if get multiple time series
	double totcatch = bm->SUBFLEET_ECONprms[nf][ns][yield_ind_id];
	double saleprice = bm->SUBFLEET_ECONprms[nf][ns][totcatch_saleprice_id];
	FisheryTimeSeries *this_fuel = &bm->tsGDP[key_id];

    /*
	if (((bm->debug == debug_econeffort) || (bm->debug == debug_effort)) && (bm->dayt > bm->checkstart) && ((bm->which_fleet == nf) || (bm->which_fleet
			>= bm->K_num_fisheries))) {
		do_debug = 1;
	} else
		do_debug = 0;
     */
    
	catch_in_hold = 0;
	for (sp = 0; sp < bm->K_num_tot_sp; sp++) {
		if (FunctGroupArray[sp].isFished == TRUE) {
			catch_in_hold += bm->QuotaAlloc[nf][ns][sp][boatcatch_id];
		}
	}

	/* Cost per day - fixed + variable costs (Note haven't put crew share in here, but it is in
	 operator's utility (calculated in Profit_Dependent_Indicators() - does it need to be here as well? */
	fuel_intercept = bm->FUELprms[fuel_intercept_id];
	trend_coefft = bm->FUELprms[trend_coefft_id];
	month_coefft = bm->FUELmonth[bm->MofY];
	auto_coefft = bm->FUELprms[auto_coefft_id];
	resid = tsEval(&this_fuel->ts, bm->FuelCost_id, bm->t);
	fuel_cost_scalar = bm->SUBFLEET_ECONprms[nf][ns][fuel_cost_id];
	fuel_cost = (fuel_intercept + trend_coefft * bm->thisyear + month_coefft + auto_coefft * resid) * fuel_cost_scalar / 100.0; /* Total fuel per day, converting cents to $ */

	/* cost of crew not really taken into account */

    /*
	if (do_debug) {
		//if(bm->dayt > bm->checkstart){
		fprintf(
				llogfp,
				"Time: %e %s-%d fuel_cost: %e, fuel_intercept: %e, trend_coefft: %e, thisyear: %d, month_coefft: %e, auto_coefft: %e, resid: %e, fuel_cost_scalar/100.0: %e\n",
				bm->dayt, FisheryArray[nf].fisheryCode, ns, fuel_cost, fuel_intercept, trend_coefft, bm->thisyear, month_coefft, auto_coefft, resid,
				fuel_cost_scalar / 100.0);
	}
    */
    
	capital_cost = bm->SUBFLEET_ECONprms[nf][ns][capital_cost_id];
	gear_cost = bm->SUBFLEET_ECONprms[nf][ns][gear_cost_id];
	fixed_cost = bm->SUBFLEET_ECONprms[nf][ns][fixed_cost_id] + (capital_cost * 30.0);
	//var_cost = (fuel_cost + gear_cost) * bm->EffortSchedule[nf][ns][bm->MofY][expect_id];
	var_cost = (fuel_cost + gear_cost) * bm->SUBFLEET_ECONprms[nf][ns][CurrentEffort_id];
	unload_cost = bm->SUBFLEET_ECONprms[nf][ns][unload_cost_id] * catch_in_hold;
	bm->SUBFLEET_ECONprms[nf][ns][var_cost_id] = var_cost;
	/* Cost per kilo */
	bm->SUBFLEET_ECONprms[nf][ns][cost_ind_id] = fixed_cost + var_cost + unload_cost;

	//fprintf(bm->logFile, "dynamic bm->SUBFLEET_ECONprms[nf][ns][cost_ind_id] = %e\n",bm->SUBFLEET_ECONprms[nf][ns][cost_ind_id]);

	cost = bm->SUBFLEET_ECONprms[nf][ns][cost_ind_id];

	/* Also update the per effort cost structure for use in the effort calculations */
	if (bm->immed_cost_only) {
		if (bm->SUBFLEET_ECONprms[nf][ns][CurrentEffort_id] > 0.0)
			bm->SUBFLEET_ECONprms[nf][ns][pereffort_cost_ind_id] = var_cost / bm->SUBFLEET_ECONprms[nf][ns][CurrentEffort_id];
		else
			bm->SUBFLEET_ECONprms[nf][ns][pereffort_cost_ind_id] = unload_cost;
	} else {
		if (bm->SUBFLEET_ECONprms[nf][ns][CurrentEffort_id] > 0.0)
			bm->SUBFLEET_ECONprms[nf][ns][pereffort_cost_ind_id] = bm->SUBFLEET_ECONprms[nf][ns][cost_ind_id] / bm->SUBFLEET_ECONprms[nf][ns][CurrentEffort_id];
		else
			bm->SUBFLEET_ECONprms[nf][ns][pereffort_cost_ind_id] = bm->SUBFLEET_ECONprms[nf][ns][cost_ind_id];
	}

    /*
	if (do_debug) {
		//if(bm->dayt > bm->checkstart){
		fprintf(
				llogfp,
				"Time: %e %s-%d totcost: %e, fixed: %e (fix: %e, capital %e), var: %e (fuel: %e, diesel: %e, gear: %e, effort: %e), unloaded: %e (unload: %e, catchinhold: %e)\n",
				bm->dayt, FisheryArray[nf].fisheryCode, ns, bm->SUBFLEET_ECONprms[nf][ns][cost_ind_id], fixed_cost,
				bm->SUBFLEET_ECONprms[nf][ns][fixed_cost_id], (capital_cost * 30.0), var_cost, fuel_cost, (fuel_intercept + trend_coefft * bm->thisyear
						+ month_coefft + auto_coefft * resid), gear_cost, bm->SUBFLEET_ECONprms[nf][ns][CurrentEffort_id], unload_cost,
				bm->SUBFLEET_ECONprms[nf][ns][unload_cost_id], catch_in_hold);
	}
     */
    
	/* Overall CPUE */
	if (bm->SUBFLEET_ECONprms[nf][ns][CurrentEffort_id] > 0)
		bm->SUBFLEET_ECONprms[nf][ns][cpue_ind_id] = bm->SUBFLEET_ECONprms[nf][ns][CurrentCatch_id] / (bm->SUBFLEET_ECONprms[nf][ns][CurrentEffort_id]
				+ small_num);
	else
		bm->SUBFLEET_ECONprms[nf][ns][cpue_ind_id] = 0;

    /*
	if (do_debug) {
		fprintf(llogfp, "Time: %e, %s-%d has cpue_ind = %e (catch: %e, effort: %e)\n", bm->dayt, FisheryArray[nf].fisheryCode, ns,
				bm->SUBFLEET_ECONprms[nf][ns][cpue_ind_id], bm->SUBFLEET_ECONprms[nf][ns][CurrentCatch_id], bm->SUBFLEET_ECONprms[nf][ns][CurrentEffort_id]);
	}
     */
    
	/* Average size of catch */
	bm->SUBFLEET_ECONprms[nf][ns][size_ind_id] = Assess_Add_Error(bm, flagcatch, Harvest_Get_Harvest_Index(bm, nf, avg_sze_id), k_avgcatch, k_varcatch);

	/* Composition of catch */
	bm->SUBFLEET_ECONprms[nf][ns][comp_ind_id] = Assess_Add_Error(bm, flagcatch, Harvest_Get_Harvest_Index(bm, nf, sp_comp_id), k_avgcatch, k_varcatch);

	/* Extent of spatial management */
	flagfcmpa = (int) (bm->FISHERYprms[nf][flagmpa_id]);
	totMPA = 0;
	numMPA = 0;
	if (flagfcmpa) {
		for (ij = 0; ij < bm->nbox; ij++) {
			totMPA += bm->MPA[ij][nf];
			if (bm->MPA[ij][nf] < 1.0)
				numMPA++;
		}
		totMPA /= bm->nbox;
	}
	bm->SUBFLEET_ECONprms[nf][ns][zone_ind_id] = totMPA;

	/* Number of gear changes (including selectivity, q, swept area, discarding, access and escapement).
	 Count only those in the last year.
	 */
	gear_change = Harvest_Get_Num_Gear_Changes(bm, nf, llogfp);

	bm->SUBFLEET_ECONprms[nf][ns][gear_ind_id] = gear_change;

	/* Loop over fished species and get all info needed to calculate management costs,
	 gvp, quota trading extent and total quota holdings
	 */
	reswork = 0;
	numTAC = 0;
	gvp = 0;
	tax_revenue = 0;
	dv_sum = 0;
	trade_extent = 0;
	numexchange = 0;
	bm->SUBFLEET_ECONprms[nf][ns][totQuota_id] = 0;
	bm->SUBFLEET_ECONprms[nf][ns][maxleased_Q_val_ind_id] = 0;

	for (sp = 0; sp < bm->K_num_tot_sp; sp++) {
		if (FunctGroupArray[sp].isFished == TRUE) {
			/* Management costs */
			tierid = (int) (FunctGroupArray[sp].speciesParams[tier_id]);
			if (tierid == tier_orig)
				tier = tier4;
			else
				tier = FunctGroupArray[sp].speciesParams[tier_id];
			in_quota = bm->inQuota[nf][sp];
			if (in_quota) {
				numTAC++;
				reswork += 1.0 / (tier + small_num);
			}

			/* GVP information - use cumulative monthly catch given done once per month */
			if (bm->MultiPlanEffort) {
				/* Basic value calculation used with base economic model */
				saleprice = bm->SP_FISHERYprms[sp][nf][saleprice_id];
				current_catch = bm->QuotaAlloc[nf][ns][sp][cummonthcatch_id];
				gvp += saleprice * current_catch;

				if (((bm->debug == debug_econeffort) || (bm->debug == debug_effort)) && (bm->dayt > bm->checkstart) && ((bm->which_fleet == nf)
						|| (bm->which_fleet >= bm->K_num_fisheries))) {
					//if(bm->dayt > bm->checkstart)
					fprintf(llogfp, "Time: %e, %s-%d %s value: %e, saleprice: %e, catch: %e, gvp: %e\n", bm->dayt, FisheryArray[nf].fisheryCode, ns,
							FunctGroupArray[sp].groupCode, (saleprice * current_catch), saleprice, current_catch, gvp);
				}
			} else {
				/* Dan Holland fleet revenue calculation - uses area specific
				 prices and catches, across all subfleets. Use proportional
				 distribution stored in SpatialBlackBook to map overall
				 monthly cumulative catch to box-level catch.
				 */
				for (ij = 0; ij < bm->nbox; ij++) {
					if (bm->boxes[ij].type != BOUNDARY) {
						/* Do this increment using market value ignoring quota pirces, this is not 100%
						 correct, but was getting ridiculous values if used the expected value

						 if(bm->QuotaAlloc[nf][ns][sp][quotaprice_id] < bm->high_price)

						 Only increment gvp if quota price not set to high_price (as that is an artificial
						 penalty imposed to condition the model not an actual economic fee to be paid.

						 Actually No can do it regardless as not using quotaprice in this calculation
						 */
						gvp += bm->ECONexprice[nf][ns][sp][ij][hist_id] * bm->SpatialBlackBook[nf][ns][bm->MofY][ij][current_id]
								* bm->QuotaAlloc[nf][ns][sp][cummonthcatch_id] / (bm->FISHERYprms[nf][nsubfleets_id] + small_num);

                        /*
						if (do_debug) {
							fprintf(llogfp, "Time: %e, box: %d, %s-%d %s mth: %d gvp: %e, price: %e, BBook: %e, Cumcatch: %e, nsub: %e\n", bm->dayt, ij,
									FisheryArray[nf].fisheryCode, ns, FunctGroupArray[sp].groupCode, bm->MofY, gvp, bm->ECONexprice[nf][ns][sp][ij][hist_id],
									bm->SpatialBlackBook[nf][ns][bm->MofY][ij][current_id], bm->QuotaAlloc[nf][ns][sp][cummonthcatch_id],
									bm->FISHERYprms[nf][nsubfleets_id]);
						}
                        */
					}
				}
			}

			/* Taxes - if using those as penalty for going over quota */
			quota = (bm->TACamt[sp][nf][now_id] * kg_2_mg) / bm->X_CN;
			//catch_count = bm->TotCumCatch[sp][nf][bm->thisyear];
			catch_count = Harvest_Get_TotCumCatch(sp, nf, bm->thisyear) + bm->TotOldCumCatch[sp][nf];


			//fprintf(llogfp,"Time: %e, %s-%d for %s taxpaid: %e, cumcatch: %e, cumbycatch: %e spconcern: %e (flagtemp: %d, flagspatial: %d, inquota: %d, tax1: %e, tax2: %e)\n",
			//	bm->dayt, FisheryArray[nf].fisheryCode, ns, FunctGroupArray[sp].groupCode, bm->SP_FISHERYprms[sp][nf][taxpaid_id], bm->QuotaAlloc[nf][ns][sp][cummonthcatch_id], bm->QuotaAlloc[nf][ns][sp][cummonthbycatch_id], bm->SP_prms[sp][spconcern_id],  bm->TemporalBycatchAvoid, bm->SpatialBycatchAvoid, bm->inQuota[nf][sp], bm->SP_FISHERYprms[spp_id][nf][tax_id], bm->SP_FISHERYprms[spp_id][nf][FixedMinTax_id]);

			if (bm->TemporalBycatchAvoid || bm->SpatialBycatchAvoid) {
				if (bm->BycatchCountDiscards && (!bm->flagTACincludeDiscard && (FunctGroupArray[sp].isFished == TRUE))) {
					/* Include discards in the count if they are a aspecies of concern and they aren't already counted (don't want to double dip) */
					catch_count += FunctGroupArray[sp].speciesParams[sp_concern_id] * (bm->TotCumDiscards[sp][nf] + bm->TotOldCumDiscards[sp][nf]);
				}
				if ((catch_count > quota) || ((bm->SP_FISHERYprms[sp][nf][tax_id] > 0.0) || (bm->SP_FISHERYprms[sp][nf][FixedMinTax_id] > 0.0))) {
					tax_revenue += bm->SP_FISHERYprms[sp][nf][taxpaid_id] * bm->QuotaAlloc[nf][ns][sp][cummonthcatch_id];
					if (bm->BycatchCountDiscards) {
						tax_revenue += bm->SP_FISHERYprms[sp][nf][taxpaid_id] * FunctGroupArray[sp].speciesParams[sp_concern_id]
								* bm->QuotaAlloc[nf][ns][sp][cummonthbycatch_id];
					}

					//fprintf(llogfp,"Time: %e, %s-%d for %s taxpaid: %e, cumcatch: %e, cumbycatch: %e spconcern: %e\n",
					//	bm->dayt, FisheryArray[nf].fisheryCode, ns, FunctGroupArray[sp].groupCode, bm->SP_FISHERYprms[sp][nf][taxpaid_id], bm->QuotaAlloc[nf][ns][sp][cummonthcatch_id], bm->QuotaAlloc[nf][ns][sp][cummonthbycatch_id], bm->SP_prms[sp][spconcern_id]);

				}
			}

			/* Deemed values */
            local_deemed_value = 0.0;
			if (catch_count > quota) {
				dvprice = bm->SP_FISHERYprms[sp][nf][deemprice_id];
				current_catch = bm->QuotaAlloc[nf][ns][sp][cummonthcatch_id];
				dv_sum += dvprice * current_catch;
                local_deemed_value = dvprice * current_catch;
			}

			/* Save the dv_sum for writing to output file */
			bm->QuotaAlloc[nf][ns][sp][deemed_value_id] += local_deemed_value;

			/* Magnitude of trading extent */
			prop_olease = bm->QuotaAlloc[nf][ns][sp][permolease_id];
			prop_tlease = bm->QuotaAlloc[nf][ns][sp][templease_id];
			trade_extent += (prop_olease + prop_tlease);
			numexchange++;

			/* Total quota */
			quota_avail = (1.0 - prop_olease - prop_tlease) * bm->QuotaAlloc[nf][ns][sp][owned_id] + bm->QuotaAlloc[nf][ns][sp][leased_id];
			bm->SUBFLEET_ECONprms[nf][ns][totQuota_id] += quota_avail;

			/* Get max lease cost - (price_of_lease_per_kg * kg_leased) */
			bm->SUBFLEET_ECONprms[nf][ns][maxleased_Q_val_ind_id] += bm->QuotaAlloc[nf][ns][sp][leased_id] * bm->QuotaAlloc[nf][ns][sp][quotaprice_id];
			maxlease_cost = bm->SUBFLEET_ECONprms[nf][ns][maxleased_Q_val_ind_id];
		}
	}

	/* Gross value */
	//fprintf(llogfp,"Day %e, %s-%d tax ind starts: %e", bm->dayt, FisheryArray[nf].fisheryCode, ns, bm->SUBFLEET_ECONprms[nf][ns][tax_ind_id]);
	bm->SUBFLEET_ECONprms[nf][ns][gvp_ind_id] += gvp - tax_revenue;
	//fprintf(llogfp," ends %e (tax_revenue: %e)\n", bm->SUBFLEET_ECONprms[nf][ns][tax_ind_id], tax_revenue);

	/* Gross tax revenue */
	bm->SUBFLEET_ECONprms[nf][ns][tax_ind_id] += tax_revenue;

	/* Sum of deemed values */
	bm->SUBFLEET_ECONprms[nf][ns][dv_ind_id] += dv_sum;

	/* Management costs */
	if (bm->MultiPlanEffort) {
		/* Australian version - function of gear restrictions, number of TACs and boxes with MPAs
		 (as all must be enforced or regulated)

		 FIX - incorporate the model from xls sheet
		 */
		bm->SUBFLEET_ECONprms[nf][ns][mgmt_cost_ind_id] = gear_change + numTAC + numMPA;
	} else {
		/* Dan Holland version - its the cost recovery costs only */
		bm->SUBFLEET_ECONprms[nf][ns][mgmt_cost_ind_id] = gvp * (1 - bm->SUBFLEET_ECONprms[nf][ns][mgmt_cost_id]);
	}

	/* Research costs - number of TACs * (1 / tier used) - FIX - anything else needed? */
	bm->SUBFLEET_ECONprms[nf][ns][res_cost_ind_id] = reswork;

	/* Revenue */
	lease_cost = bm->SUBFLEET_ECONprms[nf][ns][leased_Q_val_ind_id];
	revenue = gvp - lease_cost - cost;
	bm->SUBFLEET_ECONprms[nf][ns][revenue_ind_id] = revenue;

	minrevenue = gvp - maxlease_cost - cost; // When assume all leased fish paid for at most recent rate
	bm->SUBFLEET_ECONprms[nf][ns][minrev_ind_id] = minrevenue;

	/* Revenue per day */
	bm->SUBFLEET_ECONprms[nf][ns][rev_effort_ind_id] = revenue / (bm->SUBFLEET_ECONprms[nf][ns][CurrentEffort_id] + small_num);

	if (revenue < -1000000000.0) {
		fprintf(llogfp, "Revenue weirdness for %s-%d - revenue: %e, gvp: %e, lease_cost: %e, cost: %e\n", FisheryArray[nf].fisheryCode, ns, revenue, gvp, lease_cost, cost);
	}

	/* Revenue per ton landed - to avoid weirdness associated with small numbers assume minimum of one tonne
	 landed - if nothing landed little difference anyway in reality.
	 */
	if (bm->SUBFLEET_ECONprms[nf][ns][CurrentCatch_id] < 1.0)
		bm->SUBFLEET_ECONprms[nf][ns][CurrentCatch_id] = 1.0;
	bm->SUBFLEET_ECONprms[nf][ns][rev_land_ind_id] = revenue / (bm->SUBFLEET_ECONprms[nf][ns][CurrentCatch_id] + small_num);

	/* Return on investment - profit / total investment (costs and capital dollars put into it) */
	big_expenditure = bm->SUBFLEET_ECONprms[nf][ns][newboat_id] * bm->SUBFLEET_ECONprms[nf][ns][newboat_cost_id] + bm->SUBFLEET_ECONprms[nf][ns][switchboat_id]
			* bm->SUBFLEET_ECONprms[nf][ns][switch_cost_id];
	invest = lease_cost + cost + big_expenditure;
	bm->SUBFLEET_ECONprms[nf][ns][invest_return_ind_id] = revenue / (invest + small_num);

	/* Capital utilisation - profit / capital dollars invested */
	bm->SUBFLEET_ECONprms[nf][ns][cap_util_ind_id] = revenue / (big_expenditure + small_num);

	/* Quota trade levels */
	if (numexchange > 0)
		trade_extent /= numexchange;
	bm->SUBFLEET_ECONprms[nf][ns][Q_trade_ind_id] = trade_extent;

	/* Cost / ton */
	if (!totcatch)
		bm->SUBFLEET_ECONprms[nf][ns][cost_ton_ind_id] = cost;
	else
		bm->SUBFLEET_ECONprms[nf][ns][cost_ton_ind_id] = cost / (totcatch + small_num);

	/* Index of average marginal profit (boat cash income), also calc per species in quota model */
	bm->SUBFLEET_ECONprms[nf][ns][boat_cash_ind_id] = revenue;
	bm->SUBFLEET_ECONprms[nf][ns][tot_cash_id] += revenue;

    /*
	if (do_debug) {
		fprintf(llogfp, "Time: %e %s-%d totcash: %e,	revenue: %e, gvp: %e, lease_cost: %e cost: %e\n", bm->dayt, FisheryArray[nf].fisheryCode, ns,
				bm->SUBFLEET_ECONprms[nf][ns][tot_cash_id], revenue, gvp, lease_cost, cost);
	}
     */
    
	/* Sale value of quota (lease price calculated in Total_Quota_Price() so not repeated here) */
	/* Discards - calculated in Allocate_Catch() */

	return;
}

/**
 *
 *	\brief Indicator calculations based on expected values rather than on going
 *	values (as in model by Rich Little)
 *
 **/
void Expected_Indicators(MSEBoxModel *bm, int nf, int ns, FILE *llogfp) {
	int sp;
	int key_id = 0; // As only one economic property time series thus far - FIX make this read from array if get multiple time series
	double cost, fuel_cost_scalar, fuel_cost, capital_cost, gear_cost, fixed_cost, var_cost, fuel_intercept, trend_coefft, month_coefft, auto_coefft, resid,
			catch_in_hold, unload_cost;
	double CPUE = bm->SUBFLEET_ECONprms[nf][ns][ExpectCPUE_id];
	double TripLength = bm->SUBFLEET_ECONprms[nf][ns][max_trip_length_id];
	double saleprice = bm->SUBFLEET_ECONprms[nf][ns][totcatch_saleprice_id];
	FisheryTimeSeries *this_fuel = &bm->tsGDP[key_id];

	catch_in_hold = 0;
	for (sp = 0; sp < bm->K_num_tot_sp; sp++) {
		if (FunctGroupArray[sp].isFished == TRUE) {
			catch_in_hold += bm->QuotaAlloc[nf][ns][sp][boatcatch_id];
		}
	}

	/* Expected cost - fixed + variable costs (Note haven't put crew share in here, but it is in
	 operator's utility (calculated in Profit_Dependent_Indicators() - does it need to be here as well? */
	fuel_intercept = bm->FUELprms[fuel_intercept_id];
	trend_coefft = bm->FUELprms[trend_coefft_id];
	month_coefft = bm->FUELmonth[bm->MofY];
	auto_coefft = bm->FUELprms[auto_coefft_id];
	resid = tsEval(&this_fuel->ts, bm->FuelCost_id, bm->t);
	fuel_cost_scalar = bm->SUBFLEET_ECONprms[nf][ns][fuel_cost_id];
	fuel_cost = (fuel_intercept + trend_coefft * bm->thisyear + month_coefft + auto_coefft * resid) * fuel_cost_scalar;
	capital_cost = bm->SUBFLEET_ECONprms[nf][ns][capital_cost_id];
	gear_cost = bm->SUBFLEET_ECONprms[nf][ns][gear_cost_id];
	fixed_cost = bm->SUBFLEET_ECONprms[nf][ns][fixed_cost_id] + (capital_cost * 30.0);
	var_cost = (fuel_cost + gear_cost) * bm->EffortSchedule[nf][ns][bm->MofY][expect_id];
	unload_cost = bm->SUBFLEET_ECONprms[nf][ns][unload_cost_id] * catch_in_hold;
	bm->SUBFLEET_ECONprms[nf][ns][var_cost_id] = var_cost;
	bm->SUBFLEET_ECONprms[nf][ns][cost_ind_id] = fixed_cost + var_cost + unload_cost;

	//fprintf(bm->logFile, "dem bm->SUBFLEET_ECONprms[nf][ns][cost_ind_id] = %e\n",bm->SUBFLEET_ECONprms[nf][ns][cost_ind_id]);

	cost = bm->SUBFLEET_ECONprms[nf][ns][cost_ind_id];

	if (((bm->debug == debug_econeffort) || (bm->debug == debug_effort)) && (bm->dayt > bm->checkstart) && ((bm->which_fleet == nf) || (bm->which_fleet
			>= bm->K_num_fisheries))) {
		fprintf(
				llogfp,
				"Time: %e %s-%d totcost: %e, fixed: %e (fix: %e, capital %e), var: %e (fuel: %e, gear: %e, expeffort: %e), unloaded: %e (unload: %e, catchinhold: %e)\n",
				bm->dayt, FisheryArray[nf].fisheryCode, ns, bm->SUBFLEET_ECONprms[nf][ns][cost_ind_id], fixed_cost,
				bm->SUBFLEET_ECONprms[nf][ns][fixed_cost_id], (capital_cost * 30.0), var_cost, fuel_cost, gear_cost,
				bm->EffortSchedule[nf][ns][bm->MofY][expect_id], unload_cost, bm->SUBFLEET_ECONprms[nf][ns][unload_cost_id], catch_in_hold);
	}

	/* Cost / ton */
	bm->SUBFLEET_ECONprms[nf][ns][cost_ton_ind_id] = cost / (CPUE * TripLength + small_num);

	/* Marginal profit (boat cash income) given as saleprice per ton - cost per ton*/
	bm->SUBFLEET_ECONprms[nf][ns][boat_cash_ind_id] = saleprice - bm->SUBFLEET_ECONprms[nf][ns][cost_ton_ind_id];
	bm->SUBFLEET_ECONprms[nf][ns][tot_cash_id] += bm->SUBFLEET_ECONprms[nf][ns][boat_cash_ind_id];

	return;
}

/**
 *
 *	\brief Profit dependent indicator calculations
 *
 */
void Profit_Dependent_Indicators(MSEBoxModel *bm, int nf, int ns, FILE *llogfp) {
	double boat_cash = bm->SUBFLEET_ECONprms[nf][ns][boat_cash_ind_id]; /* revenue */
	double Quota_lease_value = bm->SUBFLEET_ECONprms[nf][ns][leased_Q_val_ind_id];
	double effort = bm->SUBFLEET_ECONprms[nf][ns][CurrentEffort_id];
	double cost = bm->SUBFLEET_ECONprms[nf][ns][cost_ind_id];
	double TripLength = bm->SUBFLEET_ECONprms[nf][ns][max_trip_length_id];
	double purchased_quota = bm->SUBFLEET_ECONprms[nf][ns][Q_trade_ind_id];
	double crew_share = bm->SUBFLEET_ECONprms[nf][ns][crewshare_id];

	/* effort is measured in days */
	/* boat cost is total cost of the current trip they are on. total revenue.*/
	/* Marginal rent given as profit - tot lease price */
	/* boat_cash - cash flow. Net income. */
	bm->SUBFLEET_ECONprms[nf][ns][marg_rent_id] = boat_cash - Quota_lease_value;

	/* Utility - rent at present, need to add social features to make it a true measure of utility
	 * or make it stochastic based on level of effort vs conservative bent (and if conservative and
	 * little variation in effort applied then happy and have high utility) */
    bm->SUBFLEET_ECONprms[nf][ns][AnnualUtility_id] = boat_cash - effort * (cost / (TripLength + small_num))
		- Quota_lease_value * purchased_quota;

	/* Operator share  didn't include min wage constraint.*/
	bm->SUBFLEET_ECONprms[nf][ns][OpUtility_id] = (1.0 - crew_share) * bm->SUBFLEET_ECONprms[nf][ns][AnnualUtility_id];

	return;
}

/**
 *
 *	\brief Economic performance measures - update and output
 *
 */
void Update_Econ_Indx(MSEBoxModel *bm, FILE *llogfp) {
	int nf, ns, sp, ij, flagrecfish;
	double numexchange, totcatch, prop_olease, prop_tlease, toteffort;

	for (nf = 0; nf < bm->K_num_fisheries; nf++) {
		/* Initialise for the first year */
		if (bm->dayt < 1.0) {
			bm->econindx[nf][fleet_switch_id] = 0;
			bm->econindx[nf][overallrent_id] = 0;
			bm->econindx[nf][SubfleetCash_id] = 0;
			bm->econindx[nf][LeaseVal_id] = 0;
			bm->econindx[nf][InvestReturn_id] = 0;
			bm->econindx[nf][GrossVal_id] = 0;
			bm->econindx[nf][GrossTax_id] = 0;
			bm->econindx[nf][GrossDV_id] = 0;
		}
		/* These reinitialised each time as cumulative values used to calculate it */
		bm->econindx[nf][TradeExtent_id] = 0;
		bm->econindx[nf][avgboatsze_id] = 0;
		numexchange = 0;
		totcatch = 0;

		flagrecfish = (int) (bm->FISHERYprms[nf][flagrecfish_id]);

		/* Number of boats */
		bm->econindx[nf][fleetnum_id] = bm->FISHERYprms[nf][nvessel_id];

		for (ns = 0; ns < bm->FISHERYprms[nf][nsubfleets_id]; ns++) {
			/* If no boats in the subfleet currently skip ahead */
			if (!bm->SUBFLEET_ECONprms[nf][ns][nboat_id]) {
				continue;
			}

			/* Fleet changes */
			bm->econindx[nf][fleet_switch_id] += boats_free[nf][ns];

			/* Average boat size */
			bm->econindx[nf][avgboatsze_id] += bm->SUBFLEET_ECONprms[nf][ns][boat_size_id] * bm->SUBFLEET_ECONprms[nf][ns][nboat_id]
					/ (bm->FISHERYprms[nf][nvessel_id] + small_num);

			/* Rent (profit) */
			bm->econindx[nf][overallrent_id] += tot_marg_rent[nf][ns];

			if (((bm->debug == debug_econeffort) || (bm->debug == debug_effort)) && (bm->dayt > bm->checkstart) && ((bm->which_fleet == nf) || (bm->which_fleet
					>= bm->K_num_fisheries))) {
				//if(bm->dayt > bm->checkstart)
				fprintf(llogfp, "Time: %e, %s totrent: %e %s-%d contrib: %e\n", bm->dayt, FisheryArray[nf].fisheryCode, bm->econindx[nf][overallrent_id],
						FisheryArray[nf].fisheryCode, ns, tot_marg_rent[nf][ns]);
			}

			/* Gross value */
			bm->econindx[nf][GrossVal_id] += bm->SUBFLEET_ECONprms[nf][ns][gvp_ind_id];

			//if(bm->dayt > bm->checkstart)
			if (((bm->debug == debug_econeffort) || (bm->debug == debug_effort)) && (bm->dayt > bm->checkstart) && ((bm->which_fleet == nf) || (bm->which_fleet
					>= bm->K_num_fisheries))) {
				fprintf(llogfp, "Time: %e, %s totGVP: %e %s-%d contrib: %e\n", bm->dayt, FisheryArray[nf].fisheryCode, bm->econindx[nf][GrossVal_id],
						FisheryArray[nf].fisheryCode, ns, bm->SUBFLEET_ECONprms[nf][ns][gvp_ind_id]);
			}

			/* Gross value */
			bm->econindx[nf][GrossTax_id] += bm->SUBFLEET_ECONprms[nf][ns][tax_ind_id];
			//fprintf(llogfp, "Day %e %s-%d GrossTax: %e tax: %e \n", bm->dayt, FisheryArray[nf].fisheryCode, ns, bm->econindx[nf][GrossTax_id], bm->SUBFLEET_ECONprms[nf][ns][tax_ind_id]);


			/* Gross deemed value contributions */
			bm->econindx[nf][GrossDV_id] += bm->SUBFLEET_ECONprms[nf][ns][dv_ind_id];

			/* Boat cash */
			bm->econindx[nf][SubfleetCash_id] += bm->SUBFLEET_ECONprms[nf][ns][boat_cash_ind_id];

			/* Leased quota value - only if commercial */
			if (!flagrecfish)
				bm->econindx[nf][LeaseVal_id] += bm->SUBFLEET_ECONprms[nf][ns][leased_Q_val_ind_id];
			else
				bm->econindx[nf][LeaseVal_id] = 0;

			/* Return on investment */
			bm->econindx[nf][InvestReturn_id] += bm->SUBFLEET_ECONprms[nf][ns][invest_return_ind_id];

			for (sp = 0; sp < bm->K_num_tot_sp; sp++) {
				if (FunctGroupArray[sp].isFished == TRUE) {
					/* Revenue per tonne */
					totcatch += bm->QuotaAlloc[nf][ns][sp][cumboatcatch_id];

					/* Trading extent */
					prop_olease = bm->QuotaAlloc[nf][ns][sp][permolease_id];
					prop_tlease = bm->QuotaAlloc[nf][ns][sp][templease_id];

					bm->econindx[nf][TradeExtent_id] += (prop_olease + prop_tlease);
					numexchange++;
				}
			}
		}
		if (!totcatch)
			bm->econindx[nf][RevPerT_id] = 0;
		else
			bm->econindx[nf][RevPerT_id] = bm->econindx[nf][SubfleetCash_id] / (totcatch + small_num);
		if (!numexchange)
			bm->econindx[nf][TradeExtent_id] = 0;
		else
			bm->econindx[nf][TradeExtent_id] /= (numexchange + small_num);

		/* Revenue per effort */
		toteffort = 0;
		for (ij = 0; ij < bm->nbox; ij++) {
			toteffort += bm->CumEffort[nf][ij];
		}
		if (!toteffort)
			bm->econindx[nf][RevPerEffort_id] = 0;
		else
			bm->econindx[nf][RevPerEffort_id] = bm->econindx[nf][SubfleetCash_id] / (toteffort + small_num);

	}
	return;
}
