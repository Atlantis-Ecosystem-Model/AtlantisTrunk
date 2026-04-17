/**
 \file
 \brief C file for economic models for Atlantis
 \ingroup ateconomic

 File:	atquota.c
 Author:	Beth Fulton
 Created:	7/10/2005
 Purpose:	C file for economic models for Atlantis
 Revisions: 7/10/2005 Created the file

 22/8/2006 Added provision so quota allocation only happens between those
 fisheries with economically driven effort model

 8/9/2006 Put in test on boatcatch so only species actually caught taken
 into profit and cost calculations.

 28-10-2009
 Removed references to the K_num_fished_sp and K_num_impacted_sp. Arrays that were this
 size have been resized to K_num_tot_sp.

 04-11-2009 Bec Gorton
 Merged Beths new bycatch incentive code - revision 961.

 24/7/2009 Beth Fulton
 Merged trunk - 1136 - Added all the deemed value and other econoomic incentive calculations

 25/8/2009 Beth Fulton
 Merged trunk - 1136 - Added UseConstPrice so don't have to use market prices (for Dan Holland
 bioeconmic model using quota price model for instance)

 7/2/2010 Beth Fulton
 Merged trunk - 1821 Added deemed values and taxes so can have economic incentives against overcatch etc

 10-05-2010 Bec Gorton
 Changed the deemed value code to calculate the market price of each market independently
 and calculate the deemed value off that. The saleprice is then updated based on the marketprice - deemed value
 for that market. The original code calculated the deemed value based on the accumulative saleprice rather than
 just the saleprice of the current market.

 01-02-2011 Beth Fulton
 Removed the quit if no residuals, but put a warning in load up that residuals
 will be treated as a zero if not provided

 04-10-2011 Beth Fulton
 Added TotOldCumCatch[sp][nf] and TotCumDiscards[sp][nf] to allow for multiyear quotas


 **/

/*  Open library routines */
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <sjwlib.h>
#include <atEconomic.h>

static void Trade(MSEBoxModel *bm, int sp, int month, int buyernf, int buyerns, int sellernf, int sellerns, double demand, double supply, int buyerMonth,
		int sellerMonth, double max_month_effort, double *remain_demand, double *remain_supply, FILE *llogfp);
/**
 *
 *	\brief Get monthly price of each species in the markets - either major
 *	domestic or export markets.
 *
 *	Currently two markets used - FIX will have to redo weighting prm storage if > markets.
 *
 *	Called by Total_Catch_Value and Economic_Annual.
 *
 */
void Get_Fish_Prices(MSEBoxModel *bm, FILE *llogfp) {
	int sp, nmarket, ts_id, nf, do_debug, fdv, quota_trip, do_debug_base, ns, do_stuff;
	double market_wgt, market_intercept, trend_coefft, month_coefft, auto_coefft, resid, FinalDV, quota, catch_count, DVscale, calcTax, discard_count;
	double marketSalePrice;

	if (verbose > 1)
		printf("Calculating fish prices\n");

	if (bm->UseConstPrice) {
		/* Using prices read in at set-up from a ts file so skip in here. Look in your forcing input file. */
		return;
	}

	if (((bm->debug == debug_quota) || (bm->debug == debug_econeffort)) && (bm->dayt > bm->checkstart))
		do_debug_base = 1;
	else
		do_debug_base = 0;

	/* Initialise */
	for (sp = 0; sp < bm->K_num_tot_sp; sp++) {
		if (FunctGroupArray[sp].isFished == TRUE) {
			for (nf = 0; nf < bm->K_num_fisheries; nf++) {
				bm->SP_FISHERYprms[sp][nf][saleprice_id] = 0;
			}
		}
	}

	/* Find new prices */
	for (nmarket = 0; nmarket < bm->K_num_markets; nmarket++) {
		for (sp = 0; sp < bm->K_num_tot_sp; sp++) {
			if (FunctGroupArray[sp].isFished == TRUE) {

				ts_id = bm->tseconid[sp];

				for (nf = 0; nf < bm->K_num_fisheries; nf++) {
					if (do_debug_base && ((bm->which_fleet == nf) || (bm->which_fleet >= bm->K_num_fisheries))) {
						do_debug = 1;
					} else
						do_debug = 0;

					/* If can't catch the group skip ahead */
					if (!bm->SP_FISHERYprms[sp][nf][q_id])
						continue;
                    
					if (!nmarket)
						market_wgt = bm->SP_FISHERYprms[sp][nf][marketwgt_id];
					else
						market_wgt = 1.0 - bm->SP_FISHERYprms[sp][nf][marketwgt_id];

					/* Get weighted average over the autoregressive
					 regression models used to represent the prices
					 in each of the markets. This gives the final
					 saleprice - the weighting is to show preference
					 for one or other of the markets.
					 */
					market_intercept = bm->MARKETprms[nmarket][sp][market_intercept_id] * bm->SP_FISHERYprms[sp][nf][origprice_id];
					trend_coefft = bm->MARKETprms[nmarket][sp][trend_coefft_id];
					month_coefft = bm->MARKETmonth[nmarket][sp][bm->MofY];
					auto_coefft = bm->MARKETprms[nmarket][sp][auto_coefft_id];

					if(bm->ntsResid){
						/* Residual times series for use with autoregressive price model */
						FisheryTimeSeries *this_residual = &bm->tsResid[nmarket];
						resid = tsEval(&this_residual->ts, ts_id, bm->t);
					} else
						resid = 0.0;


					//						bm->SP_FISHERYprms[sp][nf][saleprice_id] += market_wgt * (market_intercept + trend_coefft * bm->thisyear
					//													+ month_coefft + auto_coefft * resid);

					marketSalePrice = market_wgt * (market_intercept + trend_coefft * bm->thisyear + month_coefft + auto_coefft * resid);

					if (do_debug) {
						fprintf(
								llogfp,
								"Time: %e %s by %s price: %e, market_wgt: %e, market_intercept: %e, trend_coefft: %e, thisyear: %d, month_coefft: %e, auto_coefft: %e, resid: %e (intercept: %e, origprice: %e)\n",
								bm->dayt, FunctGroupArray[sp].groupCode, FisheryArray[nf].fisheryCode, marketSalePrice, market_wgt, market_intercept,
								trend_coefft, bm->thisyear, month_coefft, auto_coefft, resid, bm->MARKETprms[nmarket][sp][market_intercept_id],
								bm->SP_FISHERYprms[sp][nf][origprice_id]);
					}

					/* If using deemed values implement them here */
					FinalDV = 0;
					quota = (bm->TACamt[sp][nf][now_id] * kg_2_mg) / bm->X_CN;
					catch_count = (Harvest_Get_TotCumCatch(sp, nf, bm->thisyear) + bm->TotOldCumCatch[sp][nf]);
					discard_count = (bm->TotCumDiscards[sp][nf] + bm->TotOldCumDiscards[sp][nf]);

					if (bm->BycatchCountDiscards && (!bm->flagTACincludeDiscard && (FunctGroupArray[sp].isFished == TRUE))) {
						/* Include discards in the count if they are a species of concern and they aren't already counted (don't want to double dip) */
						catch_count += FunctGroupArray[sp].speciesParams[sp_concern_id] * (bm->TotCumDiscards[sp][nf] + bm->TotOldCumDiscards[sp][nf]);

					}

					quota_trip = 0;
					do_stuff = 0;
					if ((!bm->flagTACincludeDiscard || bm->BycatchCountDiscards) && (catch_count > quota)) {
						/* Use deemed values if either (i) not counting discards and landings exceed quota or
						 (ii) counting bycatch and overall catch (landings and discard) exceeds quota
						 */
						do_stuff = 1;
					} else if (bm->flagTACincludeDiscard && ((catch_count - discard_count) > quota)) {
						/* Use deemed values if are counting discards in cumulative catch but only want to know
						 when cumulative landings exceeds quota
						 */
						do_stuff = 1;
					}

					if (bm->dayt > bm->DVstart) {

						/* Get the deemed value scale */
						DVscale = Get_Deemed_Value_Scale(bm, sp, nf);
						fdv = (int) (bm->FISHERYprms[nf][flagDV_id]);

						if (do_stuff) {
							quota_trip = 1;
							//FinalDV = bm->SP_FISHERYprms[sp][nf][saleprice_id] * DVscale;
							FinalDV = marketSalePrice * DVscale;

							/* For complex deemed values need to take into account marketing costs too */
							if (fdv == complexDV_id) {
								FinalDV -= bm->SUBFLEET_ECONprms[nf][0][fixed_cost_id];
							}

							/* Record that deemed values active by recording the scalar being used */

							for (ns = 0; ns < bm->FISHERYprms[nf][nsubfleets_id]; ns++) {
								bm->QuotaAlloc[nf][ns][sp][DVtriggered_id] = DVscale;
							}
						}

						/**/
						if (do_debug) {
							fprintf(
									llogfp,
									"Time: %e, %s  %s do_stuff: %d, deemed_value: %e (fdv: %d, fixed_cost: %e, quota_trip: %d, catch_count: %e,  quota: %e, saleprice: %e, DVscale: %e\n",
									bm->dayt, FisheryArray[nf].fisheryCode, FunctGroupArray[sp].groupCode, do_stuff, FinalDV, fdv,
									bm->SUBFLEET_ECONprms[nf][0][fixed_cost_id], quota_trip, catch_count, quota, marketSalePrice, DVscale);
						}
						/**/

						/* Just make sure deemed values don't start increasing the sale price */
						if (FinalDV < 0.0)
							FinalDV = 0.0;
					}

					//if(bm->dayt > bm->checkstart)
					if (do_debug) {
						fprintf(llogfp, "Time: %e, %s by %s has saleprice %e with FInalDV: %e ", bm->dayt, FunctGroupArray[sp].groupCode,
								FisheryArray[nf].fisheryCode, marketSalePrice, FinalDV);
					}

					bm->SP_FISHERYprms[sp][nf][saleprice_id] += marketSalePrice - FinalDV;
					//bm->SP_FISHERYprms[sp][nf][saleprice_id] -= FinalDV;
					bm->SP_FISHERYprms[sp][nf][deemprice_id] = FinalDV;

					if (do_debug) {
						fprintf(llogfp, "Time: %e, %s by %s has final saleprice %e with deemedprice: %e\n", bm->dayt, FunctGroupArray[sp].groupCode,
								FisheryArray[nf].fisheryCode, bm->SP_FISHERYprms[sp][nf][saleprice_id], bm->SP_FISHERYprms[sp][nf][deemprice_id]);
					}

					/* Or if using taxes and have exceeded the quota then the price becomes a tax instead */
					calcTax = 0.0;
					if (bm->TemporalBycatchAvoid || bm->SpatialBycatchAvoid) {

						//if(bm->dayt > bm->checkstart)
						//	fprintf(llogfp,"Time: %e, %s by %s has saleprice %e ", bm->dayt, FunctGroupArray[sp].groupCode, FisheryArray[nf].fisheryCode, bm->SP_FISHERYprms[sp][nf][saleprice_id]);

						if ((do_stuff && (bm->dayt > bm->TaxStart)) || ((bm->SP_FISHERYprms[sp][nf][tax_id] > 0.0)
								|| (bm->SP_FISHERYprms[sp][nf][FixedMinTax_id] > 0.0))) {
							calcTax
									= max(bm->SP_FISHERYprms[sp][nf][FixedMinTax_id], bm->SP_FISHERYprms[sp][nf][tax_id] * bm->SP_FISHERYprms[sp][nf][saleprice_id]);
							bm->SP_FISHERYprms[sp][nf][taxpaid_id] = calcTax;

							/* Record that deemed values active by recording the scalar being used */
							for (ns = 0; ns < bm->FISHERYprms[nf][nsubfleets_id]; ns++) {
								bm->QuotaAlloc[nf][ns][sp][TAXtriggered_id] = calcTax;
							}

							/*
							 if(bm->dayt > bm->checkstart)
							 fprintf(llogfp,"Time: %e, %s by %s has saleprice now %e as applied tax %e (FixedTax: %e, tax: %e, catch_count: %e, discard_count: %e, quota: %e)\n",
							 bm->dayt, FunctGroupArray[sp].groupCode, FisheryArray[nf].fisheryCode, bm->SP_FISHERYprms[sp][nf][saleprice_id], bm->SP_FISHERYprms[sp][nf][taxpaid_id], bm->SP_FISHERYprms[sp][nf][FixedMinTax_id], bm->SP_FISHERYprms[sp][nf][tax_id], catch_count, discard_count, quota);
							 */

						}

						//if(bm->dayt > bm->checkstart)
						//	fprintf(llogfp,"as catch_count: %e quota: %e\n", catch_count, quota);

					}
				}
			}
		}
	}

	/* Find new max price */
	if (bm->UpdateMaxSalePrice) {
		for (nf = 0; nf < bm->K_num_fisheries; nf++) {
			bm->FISHERYprms[nf][maxsaleprice_id] = 0;
		}
		for (sp = 0; sp < bm->K_num_tot_sp; sp++) {
			if (FunctGroupArray[sp].isFished == TRUE) {
				for (nf = 0; nf < bm->K_num_fisheries; nf++) {
					if (bm->SP_FISHERYprms[sp][nf][saleprice_id] > bm->FISHERYprms[nf][maxsaleprice_id]) {
						bm->FISHERYprms[nf][maxsaleprice_id] = bm->SP_FISHERYprms[sp][nf][saleprice_id];
					}
				}
			}
		}
	}

	return;
}

/**
 *
 *	\brief Calculate relative profit margin associated with each species
 *
 */
void Marginal_Profit_Calc(MSEBoxModel *bm, FILE *llogfp) {
	int sp, nf, ns, do_debug, do_debug_base;
	double TAC, TACdenom, temp_correct;

	if (((bm->debug == debug_econeffort) || (bm->debug == debug_econ) || (bm->debug == debug_quota)) && (bm->dayt > bm->checkstart)) {
		do_debug_base = 1;
	} else
		do_debug_base = 0;

	if (verbose > 1)
		printf("Calculating marginal profit\n");

	/* Initalise rents */
	for (nf = 0; nf < bm->K_num_fisheries; nf++) {
		for (ns = 0; ns < bm->FISHERYprms[nf][nsubfleets_id]; ns++) {
			for (sp = 0; sp < bm->K_num_tot_sp; sp++) {
				if (FunctGroupArray[sp].isFished == TRUE) {
					bm->QuotaAlloc[nf][ns][sp][spmarg_rent_id] = 0;
				}
			}
		}
	}

	/* Calculate marginal profit and rent */
	for (nf = 0; nf < bm->K_num_fisheries; nf++) {
		for (ns = 0; ns < bm->FISHERYprms[nf][nsubfleets_id]; ns++) {
			/* If no boats in the subfleet currently skip ahead */
			if (!bm->SUBFLEET_ECONprms[nf][ns][nboat_id]) {
				continue;
			}

			if (do_debug_base && ((bm->which_fleet == nf) || (bm->which_fleet >= bm->K_num_fisheries))) {
				do_debug = 1;
			} else
				do_debug = 0;

			for (sp = 0; sp < bm->K_num_tot_sp; sp++) {
				if (FunctGroupArray[sp].isFished == TRUE) {
					/* If can't catch the group skip ahead */
					if (!bm->SP_FISHERYprms[sp][nf][q_id])
						continue;

					/* Initialise rent per species */
					bm->MargRent[nf][ns][sp][bm->MofY] = 0.0;
					/** Marginal profit - use bm->SUBFLEET_ECONprms[nf][ns][CurrentCumCatch_id] here instead
					 of bm->QuotaAlloc[nf][ns][sp][boatcatch_id] as the species are not caught in
					 isolation, but together with costs shared between them. If do them individually
					 never get any profits so don't get any fishing.
					 **/
					if (bm->QuotaAlloc[nf][ns][sp][boatcatch_id]) {
						/*
						 * per kilo marginal profit
						 */
						bm->QuotaAlloc[nf][ns][sp][spmarg_profit_id] = bm->SP_FISHERYprms[sp][nf][saleprice_id] - bm->SP_FISHERYprms[sp][nf][taxpaid_id]
								- bm->SUBFLEET_ECONprms[nf][ns][cost_ind_id] / (bm->SUBFLEET_ECONprms[nf][ns][CurrentCatch_id] + small_num);
					} else {
						bm->QuotaAlloc[nf][ns][sp][spmarg_profit_id] = 0;
					}
					bm->MargProfit[nf][ns][sp][bm->MofY] = bm->QuotaAlloc[nf][ns][sp][spmarg_profit_id];

					if (do_debug) {
						//if(bm->dayt > bm->checkstart){
						fprintf(
								llogfp,
								"Time: %e, MargProfit[%s][%d][%s][%d]: %e, bm->QuotaAlloc[spmarg_profit]: %e, SP_FISHERYprms[saleprice]: %e, cost_per_kg: %e (SUBFLEET_ECONprms[cost_ind]: %e, SUBFLEET_ECONprms[CumCatch]: %e)\n",
								bm->dayt, FisheryArray[nf].fisheryCode, ns, FunctGroupArray[sp].groupCode, bm->MofY, bm->MargProfit[nf][ns][sp][bm->MofY],
								bm->QuotaAlloc[nf][ns][sp][spmarg_profit_id], bm->SP_FISHERYprms[sp][nf][saleprice_id],
								bm->SUBFLEET_ECONprms[nf][ns][cost_ind_id] / (bm->SUBFLEET_ECONprms[nf][ns][CurrentCatch_id] + small_num),
								bm->SUBFLEET_ECONprms[nf][ns][cost_ind_id], bm->SUBFLEET_ECONprms[nf][ns][CurrentCatch_id]);
					}

					/** Rent - first find the temporal variation due to time of year
					 and then determine the final realised value (converting kg to mg) **/
					TAC = (bm->TACamt[sp][nf][now_id] * kg_2_mg) / bm->X_CN;
					TACdenom = TAC - (Harvest_Get_TotCumCatch(sp, nf, bm->thisyear) + bm->TotOldCumCatch[sp][nf]);

					if (!TACdenom)
						TACdenom = small_num;
					temp_correct = (TAC / TACdenom) * ((364.0 - bm->TofY) / 365.0);

					/* gross profit - take profit per kilo and mult by the catch to get profit in $ */
					bm->QuotaAlloc[nf][ns][sp][spmarg_rent_id] += temp_correct * bm->QuotaAlloc[nf][ns][sp][spmarg_profit_id]
							/ (bm->FISHERYprms[nf][nvessel_id] + small_num);

					if (do_debug) {
						fprintf(llogfp,
								"Time: %e, %s-%d bm->QuotaAlloc[%s][spmarg_rent_id]: %e temp_correct: %e, bm->QuotaAlloc[spmarg_profit_id]: %e, nvessel: %e\n",
								bm->dayt, FisheryArray[nf].fisheryCode, ns, FunctGroupArray[sp].groupCode, bm->QuotaAlloc[nf][ns][sp][spmarg_rent_id],
								temp_correct, bm->QuotaAlloc[nf][ns][sp][spmarg_profit_id], bm->FISHERYprms[nf][nvessel_id]);
					}
				}
			}
		}
	}

	/* Calculate final marginal rent */
	for (nf = 0; nf < bm->K_num_fisheries; nf++) {
		for (ns = 0; ns < bm->FISHERYprms[nf][nsubfleets_id]; ns++) {
			/* If no boats in the subfleet currently skip ahead */
			if (!bm->SUBFLEET_ECONprms[nf][ns][nboat_id]) {
				continue;
			}

			if (do_debug_base && ((bm->which_fleet == nf) || (bm->which_fleet >= bm->K_num_fisheries))) {
				do_debug = 1;
			} else
				do_debug = 0;

			for (sp = 0; sp < bm->K_num_tot_sp; sp++) {
				if (FunctGroupArray[sp].isFished == TRUE) {
					/* If can't catch the group skip ahead */
					if (!bm->SP_FISHERYprms[sp][nf][q_id])
						continue;

					//bm->QuotaAlloc[nf][ns][sp][spmarg_rent_id] = bm->QuotaAlloc[nf][ns][sp][spmarg_profit_id] - bm->QuotaAlloc[nf][ns][sp][spmarg_rent_id];
					bm->QuotaAlloc[nf][ns][sp][spmarg_rent_id] = bm->QuotaAlloc[nf][ns][sp][spmarg_profit_id];
					bm->MargRent[nf][ns][sp][bm->MofY] = bm->QuotaAlloc[nf][ns][sp][spmarg_rent_id];

					if (do_debug) {
						//if(bm->dayt > bm->checkstart){
						fprintf(
								llogfp,
								"Time: %e, MargRent[%s][%d][%s][%d]: %e, bm->QuotaAlloc[spmarg_rent_id]: %e, QuotaAlloc[spmarg_profit_id]: %e, bm->QuotaAlloc[spmarg_rent_id]: %e\n",
								bm->dayt, FisheryArray[nf].fisheryCode, ns, FunctGroupArray[sp].groupCode, bm->MofY, bm->MargRent[nf][ns][sp][bm->MofY],
								bm->QuotaAlloc[nf][ns][sp][spmarg_rent_id], bm->QuotaAlloc[nf][ns][sp][spmarg_profit_id],
								bm->QuotaAlloc[nf][ns][sp][spmarg_rent_id]);
					}

					/* Calculate final utility */
					bm->QuotaAlloc[nf][ns][sp][finalutility_id] = bm->QuotaAlloc[nf][ns][sp][spmarg_profit_id] - bm->QuotaAlloc[nf][ns][sp][quotaprice_id]
							* bm->QuotaAlloc[nf][ns][sp][leased_id];
				}
			}
		}
	}

	return;
}

/******************************************************************************************
 Calculate total annual margin rent for each species
 */
void Marginal_Rent_Calc(MSEBoxModel *bm, FILE *llogfp) {
	int sp, nf, ns;

	for (nf = 0; nf < bm->K_num_fisheries; nf++) {
		if (bm->FISHERYprms[nf][flageffortmodel_id] > rec_econ_model) {
			for (ns = 0; ns < bm->FISHERYprms[nf][nsubfleets_id]; ns++) {
				/* If no boats in the subfleet currently skip ahead */
				if (!bm->SUBFLEET_ECONprms[nf][ns][nboat_id]) {
					continue;
				}
				//tot_marg_rent[nf][ns] = 0;			Now initialised annually
				for (sp = 0; sp < bm->K_num_tot_sp; sp++) {
					if (FunctGroupArray[sp].isFished == TRUE) {
						/* If can't catch the group skip ahead */
						if (!bm->SP_FISHERYprms[sp][nf][q_id])
							continue;

						//for(month=0; month<12; month++){
						tot_marg_rent[nf][ns] += bm->MargRent[nf][ns][sp][bm->MofY] * bm->QuotaAlloc[nf][ns][sp][boatcatch_id];
						//tot_marg_rent[nf][ns] += bm->MargRent[nf][ns][sp][month];

						/*
						 if(bm->dayt > bm->checkstart)
						 fprintf(llogfp,"TIme: %e, %s-%d margrent: %e %s mth %d contrib: %e (rent: %e, catch: %e)\n",
						 bm->dayt, FisheryArray[nf].fisheryCode, ns, tot_marg_rent[nf][ns], FunctGroupArray[sp].groupCode, month, (bm->MargRent[nf][ns][sp][month] * bm->QuotaAlloc[nf][ns][sp][boatcatch_id]),  bm->MargRent[nf][ns][sp][month], bm->QuotaAlloc[nf][ns][sp][boatcatch_id]);
						 */

						//}

					}
				}
			}
		}
	}

	return;
}
/**
 *
 *	\brief Get overall quota prices
 *
 *	Called by Economic_Indicators.
 *
 */
void Total_Quota_Price(MSEBoxModel *bm, FILE *llogfp) {
	int nf, ns, sp, mth, in_quota;
	double totLeasePrice, current_GDP, price_coefft, price_coefftsq, cost_coefft, prev_fill_coefft, prev_fill_coefftsq, part_fill_coefft,
		part_fill_coefftsq, bind_coefft, tac_trend_coefft, const_coefft, past_cum_catch, this_cum_catch,
		price = 0, cost, last_catch, last_quota, this_quota,
		last_totQuota, this_totQuota, gdp_coefft, prop_olease, prop_tlease;

	/* double cumCatch; */
	int key_id = 0; // As only one economic property time series thus far - FIX make this read from array if get multiple time series
	FisheryTimeSeries *this_GDP = &bm->tsGDP[key_id];

	if (verbose > 1)
		printf("Calculating quota prices\n");

	/* Rich Little's lease price proxy */
	switch (bm->flagLease) {
	case RichModel:
		for (nf = 0; nf < bm->K_num_fisheries; nf++) {
			for (ns = 0; ns < bm->FISHERYprms[nf][nsubfleets_id]; ns++) {
				/* If no boats in the subfleet currently skip ahead */
				if (!bm->SUBFLEET_ECONprms[nf][ns][nboat_id]) {
					continue;
				}
				totLeasePrice = 0;
				for (sp = 0; sp < bm->K_num_tot_sp; sp++) {
					if (FunctGroupArray[sp].isFished == TRUE) {
						in_quota = bm->inQuota[nf][sp];

						if (!in_quota) {
							/* If not a participant in quota pool for this species don't include it in the calculations */
							bm->QuotaAlloc[nf][ns][sp][quotaprice_id] = 0;
						} else
							bm->QuotaAlloc[nf][ns][sp][quotaprice_id]
									= max(0.0,bm->QuotaAlloc[nf][ns][sp][spmarg_profit_id] * (1.0 + 1.0 / (bm->interest_rate + small_num)));

						totLeasePrice += bm->QuotaAlloc[nf][ns][sp][quotaprice_id] * bm->QuotaAlloc[nf][ns][sp][newleased_id];

						/* Reset the oldlease value so can calculate what is newly leased in for next time */
						bm->QuotaAlloc[nf][ns][sp][oldleased_id] = bm->QuotaAlloc[nf][ns][sp][leased_id];
					}
				}
				/* So cumulative through time - with some quota being cheaper than others to buy - depending on scarcity */
				bm->SUBFLEET_ECONprms[nf][ns][leased_Q_val_ind_id] += totLeasePrice;
			}
		}
		break;
	case NZmodel:
		/* Lease price calculation from Newell et al 2005 - NZ quota market paper */
		current_GDP = tsEval(&this_GDP->ts, bm->GDP_id, bm->t);
		if (!current_GDP)
			current_GDP = small_num;

		/* Coefficients set - FIX may need to make these subfleet specific,
		 for now just use single set from Newell et al
		 */
		price_coefft = bm->price_Coefft;
		price_coefftsq = bm->price_CoefftSq;
		cost_coefft = bm->cost_Coefft;
		prev_fill_coefft = bm->prev_fill_Coefft;
		prev_fill_coefftsq = bm->prev_fill_CoefftSq;
		part_fill_coefft = bm->part_fill_Coefft;
		part_fill_coefftsq = bm->part_fill_CoefftSq;
		bind_coefft = bm->bind_Coefft;
		tac_trend_coefft = bm->tac_trend_Coefft;
		const_coefft = bm->const_Coefft;
		gdp_coefft = bm->GDP_Coefft;

		for (nf = 0; nf < bm->K_num_fisheries; nf++) {
			for (ns = 0; ns < bm->FISHERYprms[nf][nsubfleets_id]; ns++) {
				/* If no boats in the subfleet currently skip ahead */
				if (!bm->SUBFLEET_ECONprms[nf][ns][nboat_id]) {
					continue;
				}
				totLeasePrice = 0;
				for (sp = 0; sp < bm->K_num_tot_sp; sp++) {
					if (FunctGroupArray[sp].isFished == TRUE) {
						in_quota = bm->inQuota[nf][sp];

						if (!in_quota) {
							/* If not a participant in quota pool for this species don't include it in the calculations */
							bm->QuotaAlloc[nf][ns][sp][quotaprice_id] = 0;
							continue;
						}

						past_cum_catch = 0;
						this_cum_catch = 0;

						for (mth = 0; mth <= bm->MofY; mth++) {
							past_cum_catch += bm->BlackBook[nf][ns][sp][mth][hist_id];
							this_cum_catch += bm->BlackBook[nf][ns][sp][mth][current_id];
						}

						/* Add small_num to anything that will be logged so that don't run the risk of trying to log zero */
                        price = bm->SP_FISHERYprms[sp][nf][saleprice_id];
						if (price < bm->minValue) {
							if (bm->UseMinValue) {
								/* Use a minimum value */
								price = bm->minValue;
							} else {
								/*Ignore price related terms if the fish has no landed value */
								price_coefft = 0;
								bind_coefft = 0;
								price = small_num;
							}
						}

						cost = bm->SUBFLEET_ECONprms[nf][ns][cost_ind_id] + small_num;
						last_catch = bm->QuotaAlloc[nf][ns][sp][oldcatch_id] + small_num;
						prop_olease = bm->QuotaAlloc[nf][ns][sp][permolease_id];
						prop_tlease = bm->QuotaAlloc[nf][ns][sp][templease_id];
						last_quota = bm->QuotaAlloc[nf][ns][sp][oldquota_id];
						this_quota = (1.0 - prop_olease - prop_tlease) * bm->QuotaAlloc[nf][ns][sp][owned_id] + bm->QuotaAlloc[nf][ns][sp][leased_id];
						/*cumCatch = bm->QuotaAlloc[nf][ns][sp][cumboatcatch_id]; // This can include discards if flagTACincludeDiscard is set to 1 */
						last_totQuota = bm->TACamt[sp][nf][old_id];
						this_totQuota = bm->TACamt[sp][nf][now_id];

						bm->QuotaAlloc[nf][ns][sp][quotaprice_id] = price_coefft * log(price) + price_coefftsq * log(price) * log(price) + cost_coefft * log(
								cost) + prev_fill_coefft * last_catch / (last_quota + small_num) + prev_fill_coefftsq * (last_catch / (last_quota + small_num))
								* (last_catch / (last_quota + small_num)) + part_fill_coefft * (this_cum_catch / (this_quota + small_num) - past_cum_catch
								/ (last_quota + small_num)) + part_fill_coefftsq * (this_cum_catch / (this_quota + small_num) - past_cum_catch / (last_quota
								+ small_num)) * (this_cum_catch / (this_quota + small_num) - past_cum_catch / (last_quota + small_num)) + bind_coefft * log(
								price * (last_catch / (last_quota + small_num))) + gdp_coefft * log(current_GDP) + tac_trend_coefft * last_totQuota
								/ (this_totQuota + small_num) + const_coefft;
						if (bm->QuotaAlloc[nf][ns][sp][quotaprice_id] < 0)
							bm->QuotaAlloc[nf][ns][sp][quotaprice_id] = small_num;
						/* Note: ignored SOI and fixed effects as values not stated in the Newell et al paper */
						totLeasePrice += bm->QuotaAlloc[nf][ns][sp][newleased_id] * bm->QuotaAlloc[nf][ns][sp][quotaprice_id];

						if ((bm->debug == debug_quota) && ((bm->which_fleet == nf) || (bm->which_fleet >= bm->K_num_fisheries))) {
							//if((sp == FPO_id) && (bm->newweek)){
							fprintf(
									llogfp,
									"Time: %e %s-%d %s last_quota: %e, last_totQuota: %e, this_quota: %e (polease: %e, ptlease: %e, owned: %e, leased: %e), this_totQuota: %e\n",
									bm->dayt, FisheryArray[nf].fisheryCode, ns, FunctGroupArray[sp].groupCode, last_quota, last_totQuota, this_quota,
									prop_olease, prop_tlease, bm->QuotaAlloc[nf][ns][sp][owned_id], bm->QuotaAlloc[nf][ns][sp][leased_id], this_totQuota);
							fprintf(llogfp, "Time: %e %s-%d %s quotaprice: %e, price_coefft: %e, price: %e, log(price): %e, price_coefftsq: %e\n", bm->dayt,
									FisheryArray[nf].fisheryCode, ns, FunctGroupArray[sp].groupCode, bm->QuotaAlloc[nf][ns][sp][quotaprice_id], price_coefft,
									price, (double)log(price), price_coefftsq);
							fprintf(llogfp,
									"Time: %e %s-%d %s cost_coefft: %e, cost: %e, log(cost): %e, prev_fill_coefft: %e, last_catch: %e, last_quota: %e\n",
									bm->dayt, FisheryArray[nf].fisheryCode, ns, FunctGroupArray[sp].groupCode, cost_coefft, cost, (double)log(cost), prev_fill_coefft,
									last_catch, last_quota);
							fprintf(llogfp,
									"Time: %e %s-%d %s prev_fill_coefftsq: %e, part_fill_coefft: %e, this_cum_catch: %e, this_quota: %e, past_cum_catch: %e\n",
									bm->dayt, FisheryArray[nf].fisheryCode, ns, FunctGroupArray[sp].groupCode, prev_fill_coefftsq, part_fill_coefft,
									this_cum_catch, this_quota, past_cum_catch);
							fprintf(
									llogfp,
									"Time: %e %s-%d %s part_fill_coefftsq: %e, bind_coefft: %e, log(p*lc/lq): %e, gdp_coefft: %e, GDP: %e, log(current_GDP): %e\n",
									bm->dayt, FisheryArray[nf].fisheryCode, ns, FunctGroupArray[sp].groupCode, part_fill_coefftsq, bind_coefft, (double)log(price
											* (last_catch / (last_quota + small_num))), gdp_coefft, current_GDP, (double)log(current_GDP));
							fprintf(llogfp, "Time: %e %s-%d %s tac_trend_coefft: %e, const_coefft: %e, ownq: %e, propl: %e, popo: %e, leased: %e\n", bm->dayt,
									FisheryArray[nf].fisheryCode, ns, FunctGroupArray[sp].groupCode, tac_trend_coefft, const_coefft,
									bm->QuotaAlloc[nf][ns][sp][owned_id], prop_olease, prop_tlease, bm->QuotaAlloc[nf][ns][sp][leased_id]);
						}

						/* Reset the oldlease value so can calculate what is newly leased in for next time */
						bm->QuotaAlloc[nf][ns][sp][oldleased_id] = bm->QuotaAlloc[nf][ns][sp][leased_id];

						/*
						 if((sp == FPO_id) && (bm->newweek))
						 fprintf(llogfp,"%s-%d %s this_quota: %e, this_totQuota: %e, cumCatch: %e, quota_price: %e\n",
						 FisheryArray[nf].fisheryCode, ns, FunctGroupArray[sp].groupCode, this_quota, this_totQuota, cumCatch, bm->QuotaAlloc[nf][ns][sp][quotaprice_id]);
						 */
					}
				}
				/* So cumulative through time - with some quota being cheaper than others to buy - depending on scarcity */
				bm->SUBFLEET_ECONprms[nf][ns][leased_Q_val_ind_id] += totLeasePrice;
			}
		}
		break;
	case NoPrice:
		for (nf = 0; nf < bm->K_num_fisheries; nf++) {
			for (ns = 0; ns < bm->FISHERYprms[nf][nsubfleets_id]; ns++) {
				/* If no boats in the subfleet currently skip ahead */
				if (!bm->SUBFLEET_ECONprms[nf][ns][nboat_id]) {
					continue;
				}
				for (sp = 0; sp < bm->K_num_tot_sp; sp++) {
					if (FunctGroupArray[sp].isFished == TRUE) {
						bm->QuotaAlloc[nf][ns][sp][quotaprice_id] = 0;
					}
				}
				bm->SUBFLEET_ECONprms[nf][ns][leased_Q_val_ind_id] = 0;
			}
		}
		break;
	default:
		quit("No default (%d) flagLease defined, please reset to an appropriate option\n");
		break;
	}

	return;
}

/**
 *
 *	\brief Get total value for use in revenue calculations
 *
 */
void Total_Catch_Value(MSEBoxModel *bm, FILE *llogfp) {
	int nf, ns, sp;
	double catchval;

	if (verbose > 1)
		printf("Calculating value of the catch\n");

	/* Calculate prices from market price model */
	Get_Fish_Prices(bm, llogfp);

	/* Calculate marginal rent, yields */
	for (nf = 0; nf < bm->K_num_fisheries; nf++) {
		//for (ns = 0; ns < bm->K_max_num_subfleet; ns++) {
        for (ns = 0; ns < bm->FISHERYprms[nf][nsubfleets_id]; ns++) { // Was K_max_num_subfleet but why need more subfleets than the fishery has?
			/* If no boats in the subfleet currently skip ahead */
			if (!bm->SUBFLEET_ECONprms[nf][ns][nboat_id]) {
				continue;
			}
			bm->SUBFLEET_ECONprms[nf][ns][yield_ind_id] = 0.0;
			catchval = 0;
			for (sp = 0; sp < bm->K_num_tot_sp; sp++) {
				if (FunctGroupArray[sp].isFished == TRUE) {
					//fprintf(bm->logFile, "%d:%d %s - bm->QuotaAlloc[nf][ns][sp][boatcatch_id] = %e\n", nf, ns, FunctGroupArray[sp].groupCode, bm->QuotaAlloc[nf][ns][sp][boatcatch_id]);
					catchval += bm->QuotaAlloc[nf][ns][sp][boatcatch_id] * bm->SP_FISHERYprms[sp][nf][saleprice_id];
					bm->SUBFLEET_ECONprms[nf][ns][yield_ind_id] += bm->QuotaAlloc[nf][ns][sp][boatcatch_id];
					//fprintf(bm->logFile, "bm->SUBFLEET_ECONprms[nf][ns][yield_ind_id] = %e\n", bm->SUBFLEET_ECONprms[nf][ns][yield_ind_id]);
				}
			}
			/* Get final average value of price of catch per kilo by dividing by total catch */
			bm->SUBFLEET_ECONprms[nf][ns][totcatch_saleprice_id] = catchval / (bm->SUBFLEET_ECONprms[nf][ns][yield_ind_id] + small_num);
			bm->SUBFLEET_ECONprms[nf][ns][boat_cash_ind_id] = catchval;
		}
	}

	return;
}

double Get_Buyer_Demand(MSEBoxModel *bm, int do_debug, int itc, int buyernf, int buyerns, int sp, double *ownQuota, double *leaseQuota) {
	double BUYEReffortschedule, BUYERexpectedCPUE, BUYERexpectedCatch, BUYERcumCatch, prop_olease, prop_tlease, BUYERownQuota, BUYERleaseQuota, sp_needed;
	/* Buyer needs */
	BUYEReffortschedule = bm->EffortSchedule[buyernf][buyerns][bm->MofY][expect_id];
	BUYERexpectedCPUE = bm->BlackBook[buyernf][buyerns][sp][bm->MofY][expect_id] / (bm->EffortSchedule[buyernf][buyerns][bm->MofY][hist_id] + small_num);
	BUYERexpectedCatch = BUYERexpectedCPUE * BUYEReffortschedule;
	BUYERcumCatch = bm->QuotaAlloc[buyernf][buyerns][sp][cumboatcatch_id];
	prop_olease = bm->QuotaAlloc[buyernf][buyerns][sp][permolease_id];
	prop_tlease = bm->QuotaAlloc[buyernf][buyerns][sp][templease_id];
	BUYERownQuota = (1.0 - prop_olease - prop_tlease) * bm->QuotaAlloc[buyernf][buyerns][sp][owned_id];
	BUYERleaseQuota = bm->QuotaAlloc[buyernf][buyerns][sp][leased_id];
	sp_needed = (BUYERexpectedCatch + BUYERcumCatch) - (BUYERownQuota + BUYERleaseQuota);

	if (sp_needed < 0)
		sp_needed = 0;

	*ownQuota = BUYERownQuota;
	*leaseQuota = BUYERleaseQuota;

	if (do_debug) {
		fprintf(
				bm->logFile,
				"Time: %e, itc: %d, %s, demand %e, MofY: %d (buyernf: %s, buyerns: %d, BUYERexpectedCatch: %e, BUYERcumCatch: %e, BUYERownQuota: %e, BUYERleaseQuota: %e - ptlease: %e, polease: %e, owned: %e, propcalc: %e)\n",
				bm->dayt, itc, FunctGroupArray[sp].groupCode, sp_needed, bm->MofY, FisheryArray[buyernf].fisheryCode, buyerns, BUYERexpectedCatch,
				BUYERcumCatch, BUYERownQuota, BUYERleaseQuota, prop_tlease, prop_olease, bm->QuotaAlloc[buyernf][buyerns][sp][owned_id], (1.0 - prop_olease
						- prop_tlease));
	}

	return sp_needed;
}

double Get_Seller_Supply(MSEBoxModel *bm, int sellernf, int sellerns, int sp, double *ownQuota, double *leaseQuota) {

	double SELLEReffortschedule, SELLERexpectedCPUE, SELLERexpectedCatch, SELLERcumCatch, prop_olease, prop_tlease, SELLERownQuota, SELLERleaseQuota, supply;

	/* Seller extra holdings */
	SELLEReffortschedule = bm->EffortSchedule[sellernf][sellerns][bm->MofY][expect_id];
	SELLERexpectedCPUE = bm->BlackBook[sellernf][sellerns][sp][bm->MofY][expect_id] / (bm->EffortSchedule[sellernf][sellerns][bm->MofY][hist_id] + small_num);
	SELLERexpectedCatch = SELLERexpectedCPUE * SELLEReffortschedule;
	SELLERcumCatch = bm->QuotaAlloc[sellernf][sellerns][sp][cumboatcatch_id];
	prop_olease = bm->QuotaAlloc[sellernf][sellerns][sp][permolease_id];
	prop_tlease = bm->QuotaAlloc[sellernf][sellerns][sp][templease_id];
	SELLERownQuota = (1.0 - prop_olease - prop_tlease) * bm->QuotaAlloc[sellernf][sellerns][sp][owned_id];
	SELLERleaseQuota = bm->QuotaAlloc[sellernf][sellerns][sp][leased_id];
	supply = (SELLERownQuota + SELLERleaseQuota) - (SELLERexpectedCatch + SELLERcumCatch);
	if (supply < 0) {
		supply = 0;
	}
	*ownQuota = SELLERownQuota;
	*leaseQuota = SELLERleaseQuota;

	return supply;

}
/**
 *
 *	\brief Boxmodel quota trading model - based on Rich Little's quota model
 *
 *	Price of quota not taken into account when working out what to trade.
 *	Its taken into account when working out if they want to buy or lease.
 *
 *	Trading takes into account friendship network and what they have to trade and what
 *	they can pick up.
 *
 */
void Quota_trade(MSEBoxModel *bm, FILE *llogfp) {
	int nf, ns, sp, month, quota_sp, b, s, step1, buyernf = 0, buyerns = 0, buyermonth = 0, sellernf = 0, sellerns = 0, sellermonth = 0, m,
		within_min, any_spare, itc, idd, nsort, idp, still_trading, found_any, do_debug, do_debug_base;
	int tnf = bm->K_num_fisheries;
	int maxmonth = 12;
	int nm = maxmonth - bm->MofY;
	int ascendflag = 0;
	int totpi = bm->K_max_num_subfleet * bm->K_num_fisheries * maxmonth;
	double scheduledEffort, TripLength, TripCost, cumCatch, expectedCatch = 0, ownQuota, leaseQuota, priceQuota, fishprice, expectedCPUE, aggDemand, aggSupply,
			demand, supply, ExpCatch, ExpEffort, expectedIncreasedCatch, expectedDecreaseCatch, remainderQ, sp_wgt, totPlannedEffort, totPlannedCatch,
			effThresh, max_month_effort = 0, prop_olease, prop_tlease, friend_weight, tot_match, this_max_demand, BUYERexpectedCatch, BUYERcumCatch, BUYERownQuota,
			BUYERleaseQuota, sp_needed, SELLERexpectedCatch, SELLERcumCatch, SELLERownQuota, SELLERleaseQuota, sp_avail, spareend, remain_demand,
			remain_supply, down_time, BUYEReffortschedule, BUYERexpectedCPUE, SELLEReffortschedule, SELLERexpectedCPUE;

	/* If not allowing for quota trading then skip ahead as nothing to do */
	if (!bm->quota_trading)
		return;

	if (bm->debug == debug_quota)
		do_debug_base = 1;
	else
		do_debug_base = 0;

	//checkmonth = 0;

	if (verbose > 1)
		printf("Trading quota\n");

	//fprintf(llogfp, "Trading quota\n");

	nsort = 0;

	/** Trading **/
	nsort = 0;

	/* If single species match-up rather than packages then do buying and selling */
	/* FIX STEP THROUGH RICH'S DOCUMENT AND SEE WHAT IN CODE MATCHES WHAT IN TEXT */
	if (bm->sp_by_sp) {
		for (sp = 0; sp < bm->K_num_tot_sp; sp++) {
			if (FunctGroupArray[sp].isFished == TRUE) {
				quota_sp = 0;
				aggDemand = 0;
				aggSupply = 0;
				within_min = 0;
				any_spare = 0;

				/* Initialise linearPI */
				for (ns = 0; ns < totpi; ns++) {
					linearPI[sp][linPI_id][ns] = 0;
					linearPI[sp][idd_id][ns] = 0;
					ax[ns] = 0;
					bx[ns] = 0;
				}

				idp = 0;
				for (nf = 0; nf < bm->K_num_fisheries; nf++) {
					if (bm->FISHERYprms[nf][flageffortmodel_id] > rec_econ_model) { /* Only commerical fisheries trade quota */
						/* Identify quota mix and rank subfleets/fisheries */
						/* Only bother continuing if a quota species */
						if (bm->SP_FISHERYprms[sp][nf][flagquota_id]) {
							quota_sp = 1;
							for (ns = 0; ns < bm->FISHERYprms[nf][nsubfleets_id]; ns++) {
								/* If no boats in the subfleet currently skip ahead */
								if (!bm->SUBFLEET_ECONprms[nf][ns][nboat_id]) {
									continue;
								}
								TripLength = bm->SUBFLEET_ECONprms[nf][ns][max_trip_length_id];
								TripCost = bm->SUBFLEET_ECONprms[nf][ns][cost_ind_id];

								/* Identify if anyone actually needs to trade this species rightly now */
								if (bm->QuotaAlloc[nf][ns][sp][within_id])
									within_min = 1;

								/* Identify if anyone actually has some to spare */
								if (bm->QuotaAlloc[nf][ns][sp][spare2sell_id])
									any_spare = 1;

								/* Calculate profit (pi) */
								cumCatch = bm->QuotaAlloc[nf][ns][sp][cumboatcatch_id];
								prop_olease = bm->QuotaAlloc[nf][ns][sp][permolease_id];
								prop_tlease = bm->QuotaAlloc[nf][ns][sp][templease_id];
								ownQuota = (1.0 - prop_olease - prop_tlease) * bm->QuotaAlloc[nf][ns][sp][owned_id];
								leaseQuota = bm->QuotaAlloc[nf][ns][sp][leased_id];
								priceQuota = bm->QuotaAlloc[nf][ns][sp][spmarg_profit_id] * (1.0 + 1.0 / (bm->interest_rate + small_num));
								for (month = bm->MofY; month < 12; month++) {
									expectedCatch = bm->MonthAlloc[nf][ns][sp][month];
									scheduledEffort = bm->EffortSchedule[nf][ns][month][expect_id];
									fishprice = bm->SP_FISHERYprms[sp][nf][saleprice_id];
									expectedCPUE = bm->BlackBook[nf][ns][sp][bm->MofY][expect_id] / (bm->EffortSchedule[nf][ns][bm->MofY][hist_id] + small_num);
									pi[nf][ns][month][sp] = scheduledEffort * ((fishprice * expectedCPUE) - (TripCost / (TripLength + small_num)));
									pi[nf][ns][month][sp] -= priceQuota * (cumCatch + expectedCatch - ownQuota - leaseQuota);

									idd = ns * tnf * nm + nm * nf + month - bm->MofY;
									linearPI[sp][linPI_id][idd] = bm->QuotaAlloc[nf][ns][sp][finalutility_id];
									linearPI[sp][idd_id][idd] = idd;

									if (bm->QuotaAlloc[nf][ns][sp][within_id] || bm->QuotaAlloc[nf][ns][sp][spare2sell_id]) {
										/* To keep sort times to a minimum only include those
										 fishery components that are actually buying or selling
										 at this time.
										 */
										ax[idp] = linearPI[sp][linPI_id][idd];
										bx[idp] = linearPI[sp][idd_id][idd];
										cx[idp] = 1.0; // Not needed in this case
										dx[idp] = 1.0; // Not needed in this case
										ex[idp] = 1.0; // Not needed in this case
										idp++;
									}
								}

								/** Calculate aggregate demand and supply **/
								/* Aggregate demand - is sum of the differences between the cumulative catch
								 + expected future catch and the quota held already */
								aggDemand += cumCatch + expectedCatch - ownQuota - leaseQuota;

								/* Aggregate supply - here purchase quota is sold quota so need is negative */
								aggSupply += ownQuota + leaseQuota - cumCatch;

							}
						}
					}
				}

				/* Only continue if quota species that someone has nearly filled their quota on
				 and someone else has some spare to sell. Also don't bother continuing if not
				 using the sp_by_sp code. More complex case is handled below.
				 */
				if (!quota_sp || !within_min || !any_spare || !bm->sp_by_sp)
					continue;

				/* Sort participating subfleet and fisheries based on marginal rent only - set nsort first
				 (so don't waste time sorting non-participating members */
				nsort = idp;
				Quicksort_Dir(ax, bx, cx, dx, ex, nsort, ascendflag);

				/* Copy ax and bx across to linearPIP for use below */
				for (ns = 0; ns < totpi; ns++) {
					linearPI[sp][linPI_id][ns] = ax[ns];
					linearPI[sp][idd_id][ns] = bx[ns];
				}

				b = 0;
				s = nsort;
				while ((s >= 0) && (b < nsort && b >= 0)) {
					b = 0;
					demand = 1;
					while ((demand > 0) && (b < nsort && b >= 0)) {
						demand = 0;
						/* Deconstructing ids from single sorted array - assuming ids in
						 the sorted array are of the form ns*tnf*(12 - month) + (12 - month)*nf + month - bm->MofY */
						step1 = (int) floor(ROUNDGUARD + (linearPI[sp][idd_id][b] / (nm + small_num))); /* First step in finding fishery_id and subfleet_id */
						buyerns = (int) floor(ROUNDGUARD + (step1 / (tnf + small_num))); /* Finding subfleet_id */
						buyernf = step1 % tnf; /* Finding the fishery_id */
						buyermonth = (int) floor(ROUNDGUARD + (linearPI[sp][idd_id][b])) % nm; /* Finding the month_id */
						down_time = bm->SUBFLEET_ECONprms[buyernf][buyerns][down_time_id];
						max_month_effort = bm->month_scalar * (1 - down_time) * bm->SUBFLEET_ECONprms[buyernf][buyerns][nboat_id];

						/* The expected increase in catch by filling the current month */
						if ((max_month_effort - bm->EffortSchedule[buyernf][buyerns][buyermonth][expect_id] >= 1)
								&& (bm->MargRent[buyernf][buyerns][sp][buyermonth] >= 0)) {
							scheduledEffort = bm->EffortSchedule[buyernf][buyerns][buyermonth][expect_id];
							ExpCatch = bm->BlackBook[buyernf][buyerns][sp][buyermonth][expect_id];
							ExpEffort = bm->EffortSchedule[buyernf][buyerns][buyermonth][hist_id];
							expectedIncreasedCatch = (max_month_effort - scheduledEffort) * ExpCatch / (ExpEffort + small_num);
							expectedCatch = 0;
							prop_olease = bm->QuotaAlloc[buyernf][buyerns][sp][permolease_id];
							prop_tlease = bm->QuotaAlloc[buyernf][buyerns][sp][templease_id];
							ownQuota = (1.0 - prop_olease - prop_tlease) * bm->QuotaAlloc[buyernf][buyerns][sp][owned_id];
							leaseQuota = bm->QuotaAlloc[buyernf][buyerns][sp][leased_id];
							cumCatch = bm->QuotaAlloc[buyernf][buyerns][sp][cumboatcatch_id];
							for (m = bm->MofY; m < 12; m++) {
								scheduledEffort = bm->EffortSchedule[buyernf][buyerns][m][expect_id];
								ExpCatch = bm->BlackBook[buyernf][buyerns][sp][m][expect_id];
								ExpEffort = bm->EffortSchedule[buyernf][buyerns][m][hist_id];
								expectedCatch += scheduledEffort * ExpCatch / (ExpEffort + small_num);
							}
							demand = cumCatch + expectedCatch + expectedIncreasedCatch - ownQuota - leaseQuota;
						}
						if (demand <= 0)
							b++;
					}

					s = nsort;
					supply = 0;
					while (supply <= 0 && s >= 0) {
						/* Deconstructing ids from single sorted array - assuming ids in
						 the sorted array are of the form ns*tnf*(12 - month) + (12 - month)*nf + month - bm->MofY */
						step1 = (int) floor(ROUNDGUARD + (linearPI[sp][idd_id][s] / (nm + small_num))); /* First step in finding fishery_id and subfleet_id */
						sellerns = (int) floor(ROUNDGUARD + (step1 / (tnf + small_num))); /* Finding subfleet_id */
						sellernf = step1 % tnf; /* Finding the fishery_id */
						sellermonth = (int) floor(ROUNDGUARD + (linearPI[sp][idd_id][s])) % nm; /* Finding the month_id */

						/* Expected annual quota leftover if no fishing in this month */
						if ((bm->EffortSchedule[sellernf][sellerns][sellermonth][expect_id] > 0) && (bm->MargRent[sellernf][sellerns][sp][sellermonth] < 0)) {
							scheduledEffort = bm->EffortSchedule[sellernf][sellerns][sellermonth][expect_id];
							ExpCatch = bm->BlackBook[sellernf][sellerns][sp][sellermonth][expect_id];
							ExpEffort = bm->EffortSchedule[sellernf][sellerns][sellermonth][hist_id];
							expectedDecreaseCatch = scheduledEffort * ExpCatch / (ExpEffort + small_num);
							prop_olease = bm->QuotaAlloc[sellernf][sellerns][sp][permolease_id];
							prop_tlease = bm->QuotaAlloc[sellernf][sellerns][sp][templease_id];
							ownQuota = (1.0 - prop_olease - prop_tlease) * bm->QuotaAlloc[sellernf][sellerns][sp][owned_id];
							leaseQuota = bm->QuotaAlloc[sellernf][sellerns][sp][leased_id];
							cumCatch = bm->QuotaAlloc[sellernf][sellerns][sp][cumboatcatch_id];
							/* Need supply to cover the rest of the year */
							expectedCatch = 0;
							for (m = bm->MofY; m < 12; m++) {
								scheduledEffort = bm->EffortSchedule[sellernf][sellerns][m][expect_id];
								ExpCatch = bm->BlackBook[sellernf][sellerns][sp][m][expect_id];
								ExpEffort = bm->EffortSchedule[sellernf][sellerns][m][hist_id];
								expectedCatch += scheduledEffort * ExpCatch / (ExpEffort + small_num);
							}
							remainderQ = ownQuota + leaseQuota - cumCatch - (expectedCatch - expectedDecreaseCatch);
							supply = min(expectedDecreaseCatch, remainderQ);
						}
						if (supply <= 0)
							s--;
					}

					if ((demand <= bm->recon_buffer) || (supply <= bm->recon_buffer))
						break;

					down_time = bm->SUBFLEET_ECONprms[buyernf][buyerns][down_time_id];
					max_month_effort = bm->month_scalar * (1 - down_time) * bm->SUBFLEET_ECONprms[buyernf][buyerns][nboat_id];
					Trade(bm, sp, bm->MofY, buyernf, buyerns, sellernf, sellerns, demand, supply, buyermonth, sellermonth, max_month_effort, &remain_demand,
							&remain_supply, llogfp);

					/* Need to recalculate expected catch and planned effort as just changed schedule in
					 quota trading routine trade() */
					bm->SUBFLEET_ECONprms[sellernf][sellerns][totPlanEffort_id] = 0;
					bm->SUBFLEET_ECONprms[sellernf][sellerns][totPlanCatch_id] = 0;
					for (m = bm->MofY; m < 12; m++) {
						scheduledEffort = bm->EffortSchedule[sellernf][sellerns][m][expect_id];
						ExpCatch = bm->BlackBook[sellernf][sellerns][sp][m][expect_id];
						ExpEffort = bm->EffortSchedule[sellernf][sellerns][m][hist_id];
						bm->SUBFLEET_ECONprms[sellernf][sellerns][totPlanEffort_id] += scheduledEffort;
						bm->SUBFLEET_ECONprms[sellernf][sellerns][totPlanCatch_id] += scheduledEffort * ExpCatch / ExpEffort;
					}

					bm->SUBFLEET_ECONprms[buyernf][buyerns][totPlanEffort_id] = 0;
					bm->SUBFLEET_ECONprms[buyernf][buyerns][totPlanCatch_id] = 0;
					for (m = bm->MofY; m < 12; m++) {
						scheduledEffort = bm->EffortSchedule[buyernf][buyerns][m][expect_id];
						ExpCatch = bm->BlackBook[buyernf][buyerns][sp][m][expect_id];
						ExpEffort = bm->EffortSchedule[buyernf][buyerns][m][hist_id];
						bm->SUBFLEET_ECONprms[buyernf][buyerns][totPlanEffort_id] += scheduledEffort;
						bm->SUBFLEET_ECONprms[buyernf][buyerns][totPlanCatch_id] += scheduledEffort * ExpCatch / ExpEffort;
					}

					/* Get new aggregate supply and demand */
					aggDemand = 0;
					aggSupply = 0;
					for (nf = 0; nf < bm->K_num_fisheries; nf++) {
						for (ns = 0; ns < bm->FISHERYprms[nf][nsubfleets_id]; ns++) {
							/* If no boats in the subfleet currently skip ahead */
							if (!bm->SUBFLEET_ECONprms[nf][ns][nboat_id]) {
								continue;
							}
							cumCatch = bm->QuotaAlloc[nf][ns][sp][cumboatcatch_id];
							expectedCatch = bm->MonthAlloc[nf][ns][sp][bm->MofY];
							prop_olease = bm->QuotaAlloc[nf][ns][sp][permolease_id];
							prop_tlease = bm->QuotaAlloc[nf][ns][sp][templease_id];
							ownQuota = (1.0 - prop_olease - prop_tlease) * bm->QuotaAlloc[nf][ns][sp][owned_id];
							leaseQuota = bm->QuotaAlloc[nf][ns][sp][leased_id];

							/** Calculate aggregate demand and supply **/
							/* Aggregate demand - is sum of the differences between the cumulative catch
							 + expected future catch and the quota held already */
							aggDemand += cumCatch + expectedCatch - ownQuota - leaseQuota;

							/* Aggregate supply - here purchase quota is sold quota so need is negative */
							aggSupply += ownQuota + leaseQuota - cumCatch;
						}
					}
				}

				/* Step 2 see if there is anyone with full effort scedule but is expected to not fill their quota */

				s = nsort;
				b = 0;

				while (s >= 0 && (b < nsort && b >= 0)) {
					s = nsort;
					supply = 0;
					while ((supply <= 0) && (s >= 0)) {
						/* Deconstructing ids from single sorted array - assuming ids in
						 the sorted array are of the form ns*tnf*(12 - month) + (12 - month)*nf + month - bm->MofY */
						step1 = (int) floor(ROUNDGUARD + (linearPI[sp][idd_id][s] / (nm + small_num))); /* First step in finding fishery_id and subfleet_id */
						sellerns = (int) floor(ROUNDGUARD + (step1 / (tnf + small_num))); /* Finding subfleet_id */
						sellernf = step1 % tnf; /* Finding the fishery_id */

						/* Use totPlanEffort and totPlanCatch calculated elsewhere */
						supply = 0;
						totPlannedEffort = bm->SUBFLEET_ECONprms[sellernf][sellerns][totPlanEffort_id];
						totPlannedCatch = bm->SUBFLEET_ECONprms[sellernf][sellerns][totPlanCatch_id];
						cumCatch = bm->QuotaAlloc[sellernf][sellerns][sp][cumboatcatch_id];
						expectedCatch = bm->MonthAlloc[sellernf][sellerns][sp][bm->MofY];
						prop_olease = bm->QuotaAlloc[sellernf][sellerns][sp][permolease_id];
						prop_tlease = bm->QuotaAlloc[sellernf][sellerns][sp][templease_id];
						ownQuota = (1.0 - prop_olease - prop_tlease) * bm->QuotaAlloc[sellernf][sellerns][sp][owned_id];
						leaseQuota = bm->QuotaAlloc[sellernf][sellerns][sp][leased_id];
						effThresh = 30.0; /* Effort threshold as max can actually do
						 - if effort = days fishing then 30 is thresh,
						 if seconds then 86400*30 etc */
						if ((totPlannedEffort >= (12.0 - bm->MofY) * effThresh) && ((totPlannedCatch + cumCatch) < (ownQuota + leaseQuota))) {
							supply = (ownQuota + leaseQuota) - (totPlannedCatch + cumCatch);
						}

						if (supply <= 0)
							s--;
					}

					b = 0;
					demand = 0;
					while ((demand <= 0) && (b < nsort && b >= 0)) {
						demand = 0;
						/* Deconstructing ids from single sorted array - assuming ids in
						 the sorted array are of the form ns*tnf*(12 - month) + (12 - month)*nf + month - bm->MofY */
						step1 = (int) floor(ROUNDGUARD + (linearPI[sp][idd_id][b] / (nm + small_num))); /* First step in finding fishery_id and subfleet_id */
						buyerns = (int) floor(ROUNDGUARD + (step1 / (tnf + small_num))); /* Finding subfleet_id */
						buyernf = step1 % tnf; /* Finding the fishery_id */
						buyermonth = (int) floor(ROUNDGUARD + (linearPI[sp][idd_id][b])) % nm; /* Finding the month_id */

						/* The expected increase in catch by filling the current month */
						if ((max_month_effort - bm->EffortSchedule[buyernf][buyerns][buyermonth][expect_id] >= 1)
								&& (bm->MargRent[buyernf][buyerns][sp][buyermonth] >= 0)) {
							scheduledEffort = bm->EffortSchedule[buyernf][buyerns][buyermonth][expect_id];
							ExpCatch = bm->BlackBook[buyernf][buyerns][sp][buyermonth][expect_id];
							ExpEffort = bm->EffortSchedule[buyernf][buyerns][buyermonth][hist_id];
							prop_olease = bm->QuotaAlloc[buyernf][buyerns][sp][permolease_id];
							prop_tlease = bm->QuotaAlloc[buyernf][buyerns][sp][templease_id];
							ownQuota = (1.0 - prop_olease - prop_tlease) * bm->QuotaAlloc[buyernf][buyerns][sp][owned_id];
							leaseQuota = bm->QuotaAlloc[buyernf][buyerns][sp][leased_id];
							cumCatch = bm->QuotaAlloc[buyernf][buyerns][sp][cumboatcatch_id];
							expectedIncreasedCatch = (max_month_effort - scheduledEffort) * ExpCatch / (ExpEffort + small_num);
							expectedCatch = 0;
							for (m = bm->MofY; m < 12; m++) {
								scheduledEffort = bm->EffortSchedule[buyernf][buyerns][m][expect_id];
								ExpCatch = bm->BlackBook[buyernf][buyerns][sp][m][expect_id];
								ExpEffort = bm->EffortSchedule[buyernf][buyerns][m][hist_id];
								expectedCatch += scheduledEffort * ExpCatch / (ExpEffort + small_num);
							}
							demand = cumCatch + expectedCatch + expectedIncreasedCatch - ownQuota - leaseQuota;

						}
						if (demand <= 0)
							b++;
					}

					if ((demand <= bm->recon_buffer) || (supply <= bm->recon_buffer))
						break;

					down_time = bm->SUBFLEET_ECONprms[buyernf][buyerns][down_time_id];
					max_month_effort = bm->month_scalar * (1 - down_time) * bm->SUBFLEET_ECONprms[buyernf][buyerns][nboat_id];
					Trade(bm, sp, bm->MofY, buyernf, buyerns, sellernf, sellerns, demand, supply, buyermonth, sellermonth, max_month_effort, &remain_demand,
							&remain_supply, llogfp);

					/* Use totPlanEffort and totPlanCatch calculated elsewhere
					 FIX - AS NOT UPDATING EFFORT ETC HERE RIGHT?? WHY NEED TO CALCULATE IT AGAIN? */

				}
			}
		}
	}

	/* If dealing with packages not single species quota trading */
	if (!bm->sp_by_sp) {
		itc = 0;
		still_trading = 1;

		while (still_trading) {
			idp = 0;
			/* See whose sale package is the closest match */
			for (buyernf = 0; buyernf < bm->K_num_fisheries; buyernf++) {
				/* won't attempt to trade non-dynamic fisheries */
				if (bm->FISHERYprms[buyernf][flageffortmodel_id] > rec_econ_model) { /* Only commerical fisheries trade quota */

					if (do_debug_base && ((bm->which_fleet == buyernf) || (bm->which_fleet >= bm->K_num_fisheries))) {
						do_debug = 1;
					} else
						do_debug = 0;

					for (buyerns = 0; buyerns < bm->FISHERYprms[buyernf][nsubfleets_id]; buyerns++) {
						/* If no boats in the subfleet then can't trade and only
						 want to bother with those in the trading market as a buyer
						 */

						if (do_debug) {
							fprintf(llogfp, "Time: %e, itc: %d, buyer = %s-%d nboat: %e, within: %e\n", bm->dayt, itc, FisheryArray[buyernf].fisheryCode,
									buyerns, bm->SUBFLEET_ECONprms[buyernf][buyerns][nboat_id], bm->SUBFLEET_ECONprms[buyernf][buyerns][within_id]);
						}

						/* TODO: create a seperate # define for the within_id in the subfleet econ params */
						/* if not close to exceeding quota they are not looking to buy stuff in */
						if((!bm->SUBFLEET_ECONprms[buyernf][buyerns][nboat_id]) ||
						   (!bm->SUBFLEET_ECONprms[buyernf][buyerns][within_id])){
							continue;
						}

						/* Identify what's on the table and its relative worth to the buyer */
						for (sellernf = 0; sellernf < bm->K_num_fisheries; sellernf++) {
							if (bm->FISHERYprms[sellernf][flageffortmodel_id] > rec_econ_model) { /* Only commerical fisheries trade quota */
								for (sellerns = 0; sellerns < bm->FISHERYprms[sellernf][nsubfleets_id]; sellerns++) {
									/* First off if no boats in the subfleet currently skip ahead */
									if (!bm->SUBFLEET_ECONprms[sellernf][sellerns][nboat_id]) {
										continue;
									}

									/* If buyer and seller have same identity skip ahead (as needn't trade to self) */
									if ((sellernf == buyernf) && (sellerns == buyerns))
										continue;

									/* Only bother with those in the trading market as a seller */
									found_any = 0;
									for (sp = 0; sp < bm->K_num_tot_sp; sp++) {
										if (FunctGroupArray[sp].isFished == TRUE) {
											if (bm->QuotaAlloc[buyernf][buyerns][sp][within_id] && bm->QuotaAlloc[sellernf][sellerns][sp][spare2sell_id])
												found_any = 1;

											/*
											 if(sp == FDB_id)
											 fprintf(llogfp,"Time: %e, buyer-%s, seller-%s, within %e for %s target: %d, QuotaAlloc: %e, found_any: %d\n",
											 bm->dayt, FisheryArray[buyernf].fisheryCode, FisheryArray[sellernf].fisheryCode, bm->QuotaAlloc[buyernf][buyerns][sp][within_id], FunctGroupArray[sp].groupCode, bm->FISHERYtarget[buyernf][sp], bm->QuotaAlloc[sellernf][sellerns][sp][spare2sell_id], found_any);
											 */
										}
									}
									if (!found_any)
										continue;

									/* Get ideal quota mix */
									friend_weight = friendship_netwk[buyernf][sellernf][buyerns][sellerns];
									tot_match = 0;
									//any_target = 3;
									this_max_demand = 0;

									for (sp = 0; sp < bm->K_num_tot_sp; sp++) {
										if (FunctGroupArray[sp].isFished == TRUE) {
											/* Weight based on inverse to targeting preference (as want to minimise
											 final result so small_numer is more desirable - will place a higher importance on targeted species */
											sp_wgt = 1.0;
											if (bm->FISHERYtarget[buyernf][sp])
												sp_wgt = 1.0 / (bm->targetscale + small_num);

											/* Buyer needs */
											//sp_needed = Get_Buyer_Demand(bm, do_debug, itc, buyernf, buyerns, sp, &BUYERownQuota, &BUYERleaseQuota);
											BUYEReffortschedule = bm->EffortSchedule[buyernf][buyerns][bm->MofY][expect_id];
											BUYERexpectedCPUE = bm->BlackBook[buyernf][buyerns][sp][bm->MofY][expect_id]
													/ (bm->EffortSchedule[buyernf][buyerns][bm->MofY][hist_id] + small_num);
											BUYERexpectedCatch = BUYERexpectedCPUE * BUYEReffortschedule;
											BUYERcumCatch = bm->QuotaAlloc[buyernf][buyerns][sp][cumboatcatch_id];
											prop_olease = bm->QuotaAlloc[buyernf][buyerns][sp][permolease_id];
											prop_tlease = bm->QuotaAlloc[buyernf][buyerns][sp][templease_id];
											BUYERownQuota = (1.0 - prop_olease - prop_tlease) * bm->QuotaAlloc[buyernf][buyerns][sp][owned_id];
											BUYERleaseQuota = bm->QuotaAlloc[buyernf][buyerns][sp][leased_id];
											sp_needed = (BUYERexpectedCatch + BUYERcumCatch) - (BUYERownQuota + BUYERleaseQuota);
											if (sp_needed < 0)
												sp_needed = 0;

											if (sp_needed > this_max_demand)
												this_max_demand = sp_needed;

											if (do_debug) {
												//if(buyernf == dseineFDB_id){
												fprintf(
														llogfp,
														"Time: %e, itc: %d, %s, sp_needed: %e, (this_max_demand: %e), buyer = %s-%d BUYERexpectedCatch: %e, BUYERcumCatch: %e, BUYERownQuota: %e, BUYERleaseQuota: %e\n",
														bm->dayt, itc, FunctGroupArray[sp].groupCode, sp_needed, this_max_demand,
														FisheryArray[buyernf].fisheryCode, buyerns, BUYERexpectedCatch, BUYERcumCatch, BUYERownQuota,
														BUYERleaseQuota);
											}

											if (!sp_needed) {
												/* If don't need to trade skip it */
												continue;
											}

											/* Seller extra holdings */

											//sp_avail = Get_Seller_Supply(bm, sellernf, sellerns, sp, &SELLERownQuota, &SELLERleaseQuota);
											SELLEReffortschedule = bm->EffortSchedule[sellernf][sellerns][bm->MofY][expect_id];
											SELLERexpectedCPUE = bm->BlackBook[sellernf][sellerns][sp][bm->MofY][expect_id]
													/ (bm->EffortSchedule[sellernf][sellerns][bm->MofY][hist_id] + small_num);
											SELLERexpectedCatch = SELLERexpectedCPUE * SELLEReffortschedule;
											SELLERcumCatch = bm->QuotaAlloc[sellernf][sellerns][sp][cumboatcatch_id];
											prop_olease = bm->QuotaAlloc[sellernf][sellerns][sp][permolease_id];
											prop_tlease = bm->QuotaAlloc[sellernf][sellerns][sp][templease_id];
											SELLERownQuota = (1.0 - prop_olease - prop_tlease) * bm->QuotaAlloc[sellernf][sellerns][sp][owned_id];
											SELLERleaseQuota = bm->QuotaAlloc[sellernf][sellerns][sp][leased_id];
											sp_avail = (SELLERownQuota + SELLERleaseQuota) - (SELLERexpectedCatch + SELLERcumCatch);
											if ((sp_avail < 0) || (!bm->QuotaAlloc[sellernf][sellerns][sp][spare2sell_id])) {
												sp_avail = 0;
												sp_wgt = 1.0;
											}
											/* Closer to end of year then more likely to sell what extra remains */
											spareend = bm->prop_spareend + (1 - bm->prop_spareend) * (bm->MofY / 12.0);
											if (sp_avail < (1.0 - spareend) * (SELLERownQuota + SELLERleaseQuota)) {
												sp_avail = 0;
												sp_wgt = 1.0;
											}

											if (do_debug) {
												fprintf(
														llogfp,
														"Time: %e, itc: %d, %s, sp_avail: %e, (spare2sell: %e), seller = %s-%d SELLERexpectedCatch: %e, SELLERcumCatch: %e, SELLERownQuota: %e, SELLERleaseQuota: %e\n",
														bm->dayt, itc, FunctGroupArray[sp].groupCode, sp_avail,
														bm->QuotaAlloc[sellernf][sellerns][sp][spare2sell_id], FisheryArray[sellernf].fisheryCode, sellerns,
														SELLERexpectedCatch, SELLERcumCatch, SELLERownQuota, SELLERleaseQuota);
											}

											/* No point trading if no target to trade - removed this code as bycatch species can be driving the trades
											 if(!sp_avail && bm->FISHERYtarget[buyernf][sp])
											 any_target--;
											 if(any_target < 1)
											 break;
											 */

											/* Final match value - this is a function of species targeting preference (sp_wgt)
											 the amount available (and how close it is to what's desired) and a friendship measure.
											 As this match is to be sorted based on minimisation of difference between
											 desired and available then any weighting should be inverse to strength of
											 desire or friendship network (so more desirable get smaller scalar so picked
											 out preferentially in the minimisation

											 TODO: Reintroduce explicit read-in for the friendship network

											 */
											tot_match += sp_wgt * (sp_needed - sp_avail) / (friend_weight + small_num);

											if (do_debug) {
												fprintf(
														llogfp,
														"Time: %e, itc: %d, %s, tot_match = %e, sp_wgt: %e, sp_needed: %e, sp_avail: %e, friend_weight: %e (buyernf: %s, buyerns: %d, sellernf: %s, sellerns: %d, idp: %d)\n",
														bm->dayt, itc, FunctGroupArray[sp].groupCode, tot_match, sp_wgt, sp_needed, sp_avail, friend_weight,
														FisheryArray[buyernf].fisheryCode, buyerns, FisheryArray[sellernf].fisheryCode, sellerns, idp);
											}

											if (do_debug) {
												fprintf(
														llogfp,
														"Time: %e, itc: %d, %s, tot_match = %e, sp_wgt: %e, sp_needed: %e, sp_avail: %e, friend_weight: %e (buyernf: %d, buyerns: %d, sellernf: %d, sellerns: %d, idp: %d)\n",
														bm->dayt, itc, FunctGroupArray[sp].groupCode, tot_match, sp_wgt, sp_needed, sp_avail, friend_weight,
														buyernf, buyerns, sellernf, sellerns, idp);
											}
										}
									}

									/* Don't bother trading if less than 10kg to make-up at most */
									if (this_max_demand < bm->recon_buffer)
										continue;

									ax[idp] = tot_match;
									bx[idp] = buyernf;
									cx[idp] = buyerns; // buyer id
									dx[idp] = sellernf;
									ex[idp] = sellerns; // seller id
									idp++;
								}
							}
						}
					}
				}
			}
			/* Check matches *
			 for(idd=0; idd<idp; idd++){
			 a = ax[idd];
			 bb = (int)(bx[idd]);
			 c = (int)(cx[idd]);
			 d = (int)(dx[idd]);
			 e = (int)(ex[idd]);

			 fprintf(llogfp, "Time: %e %s-%d has %e totmatch with %s-%d\n",
			 bm->dayt, bm->fisheryNAME[bb], c, a, bm->fisheryNAME[d], e);
			 }
			 */

			/* Sort participating subfleet and fisheries based on size of matches
			 (want to sort based on minimising the size of difference between
			 what is wanted and what is available)
			 */
			Quicksort_Dir(ax, bx, cx, dx, ex, nsort, ascendflag);

			for (sp = 0; sp < bm->K_num_tot_sp; sp++) {
				if (FunctGroupArray[sp].isFished == TRUE) {
					max_demand[sp] = 0;
					any2spare[sp] = 0;
				}
			}

			/* Check sorted matches *
			 for(idd=0; idd<idp; idd++){
			 a = ax[idd];
			 bb = (int)(bx[idd]);
			 c = (int)(cx[idd]);
			 d = (int)(dx[idd]);
			 e = (int)(ex[idd]);

			 fprintf(llogfp, "Time: %e %s-%d has %e sorted totmatch with %s-%d\n",
			 bm->dayt, bm->fisheryNAME[bb], c, a, bm->fisheryNAME[d], e);
			 }
			 */
			for (idd = 0; idd < idp; idd++) {
				buyernf = (int) (bx[idd]); /* Finding the buyer fishery_id */
				buyerns = (int) (cx[idd]); /* Finding the buyer subfleet_id */
				sellernf = (int) (dx[idd]); /* Finding the seller fishery_id */
				sellerns = (int) (ex[idd]); /* Finding the seller subfleet_id */

				if (do_debug_base && ((bm->which_fleet == buyernf) || (bm->which_fleet == sellernf) || (bm->which_fleet >= bm->K_num_fisheries))) {
					do_debug = 1;
				} else
					do_debug = 0;

				for (sp = 0; sp < bm->K_num_tot_sp; sp++) {
					if (FunctGroupArray[sp].isFished == TRUE) {
						if (do_debug) {
							fprintf(
									llogfp,
									"Time: %e, idd: %d, itc: %d, %s, MofY: %d (buyernf: %s, buyerns: %d, BUYERexpectedEffort: %e, BUYERexpectedHist: %e, BlackBookExpect: %e) - seller %s-%d\n",
									bm->dayt, idd, itc, FunctGroupArray[sp].groupCode, bm->MofY, FisheryArray[buyernf].fisheryCode, buyerns,
									bm->EffortSchedule[buyernf][buyerns][bm->MofY][expect_id], bm->EffortSchedule[buyernf][buyerns][bm->MofY][hist_id],
									bm->BlackBook[buyernf][buyerns][sp][bm->MofY][expect_id], FisheryArray[sellernf].fisheryCode, sellerns);
						}

						/* Find remaing supply and demand */
						//demand = Get_Buyer_Demand(bm, do_debug, itc, buyernf, buyerns, sp, &BUYERownQuota, &BUYERleaseQuota);

						BUYEReffortschedule = bm->EffortSchedule[buyernf][buyerns][bm->MofY][expect_id];
						BUYERexpectedCPUE = bm->BlackBook[buyernf][buyerns][sp][bm->MofY][expect_id] / (bm->EffortSchedule[buyernf][buyerns][bm->MofY][hist_id]
								+ small_num);
						BUYERexpectedCatch = BUYERexpectedCPUE * BUYEReffortschedule;
						BUYERcumCatch = bm->QuotaAlloc[buyernf][buyerns][sp][cumboatcatch_id];
						prop_olease = bm->QuotaAlloc[buyernf][buyerns][sp][permolease_id];
						prop_tlease = bm->QuotaAlloc[buyernf][buyerns][sp][templease_id];
						BUYERownQuota = (1.0 - prop_olease - prop_tlease) * bm->QuotaAlloc[buyernf][buyerns][sp][owned_id];
						BUYERleaseQuota = bm->QuotaAlloc[buyernf][buyerns][sp][leased_id];
						demand = (BUYERexpectedCatch + BUYERcumCatch) - (BUYERownQuota + BUYERleaseQuota);
						if (demand < 0)
							demand = 0;

						if (do_debug) {
							fprintf(
									llogfp,
									"Time: %e, itc: %d, %s, demand %e, MofY: %d (buyernf: %s, buyerns: %d, BUYERexpectedCatch: %e, BUYERcumCatch: %e, BUYERownQuota: %e, BUYERleaseQuota: %e - ptlease: %e, polease: %e, owned: %e, propcalc: %e)\n",
									bm->dayt, itc, FunctGroupArray[sp].groupCode, demand, bm->MofY, FisheryArray[buyernf].fisheryCode, buyerns,
									BUYERexpectedCatch, BUYERcumCatch, BUYERownQuota, BUYERleaseQuota, prop_tlease, prop_olease,
									bm->QuotaAlloc[buyernf][buyerns][sp][owned_id], (1.0 - prop_olease - prop_tlease));
						}

						//TODO : This should probably be in the loop above.
						if (demand < bm->recon_buffer) {
							/* No need to trade if no demand - consider demand of 1kg or less as trivial */
							demand = 0;
							continue;
						}

						/* Seller extra holdings */
						//supply = Get_Seller_Supply(bm, sellernf, sellerns, sp, &SELLERownQuota, &SELLERleaseQuota);

						SELLEReffortschedule = bm->EffortSchedule[sellernf][sellerns][bm->MofY][expect_id];
						SELLERexpectedCPUE = bm->BlackBook[sellernf][sellerns][sp][bm->MofY][expect_id]
								/ (bm->EffortSchedule[sellernf][sellerns][bm->MofY][hist_id] + small_num);
						SELLERexpectedCatch = SELLERexpectedCPUE * SELLEReffortschedule;
						SELLERcumCatch = bm->QuotaAlloc[sellernf][sellerns][sp][cumboatcatch_id];
						prop_olease = bm->QuotaAlloc[sellernf][sellerns][sp][permolease_id];
						prop_tlease = bm->QuotaAlloc[sellernf][sellerns][sp][templease_id];
						SELLERownQuota = (1.0 - prop_olease - prop_tlease) * bm->QuotaAlloc[sellernf][sellerns][sp][owned_id];
						SELLERleaseQuota = bm->QuotaAlloc[sellernf][sellerns][sp][leased_id];
						supply = (SELLERownQuota + SELLERleaseQuota) - (SELLERexpectedCatch + SELLERcumCatch);
						if (supply < 0) {
							supply = 0;
						}

						/* Closer to end of year then more likely to sell what extra remains - intentionally keep less so happy to sell so don't zero out supply as easily */
						spareend = bm->prop_spareend + (((1.0 / bm->prop_spareend) - 1.0) * bm->prop_spareend - 1.0) * (bm->MofY / 12.0);
						if (supply < spareend * (SELLERownQuota + SELLERleaseQuota)) {
							/* Want to hold on to their last little bit of quota.
							 Can't just use spare2sell as need to recalculate it after trading has reset quota holdings.
							 */
							supply = 0;
						}

						if (do_debug) {
							fprintf(
									llogfp,
									"Time: %e, itc: %d, %s, supply %e, spareend: %e, MofY: %d (sellernf: %s, sellerns: %d, SELLERexpectedCatch: %e, SELLERcumCatch: %e, SELLERownQuota: %e, SELLERleaseQuota: %e, prop_tlease: %e, prop_olease: %e)\n",
									bm->dayt, itc, FunctGroupArray[sp].groupCode, supply, spareend, bm->MofY, FisheryArray[sellernf].fisheryCode, sellerns,
									SELLERexpectedCatch, SELLERcumCatch, SELLERownQuota, SELLERleaseQuota, prop_tlease, prop_olease);
						}

						if (supply < bm->recon_buffer) {
							/* Nothing of substance to trade so skip ahead - consider demand of 1kg or less as trivial */
							supply = 0;
							continue;
						}

						/* Do the trading - if anything available */
						down_time = bm->SUBFLEET_ECONprms[buyernf][buyerns][down_time_id];
						max_month_effort = bm->month_scalar * (1 - down_time) * bm->SUBFLEET_ECONprms[buyernf][buyerns][nboat_id];
						remain_demand = 0;
						remain_supply = 0;
						if (supply && demand) {
							//fprintf(llogfp,"Time: %e %s-%d trading %s with %s-%d (demand: %e, supply: %e\n",
							//	bm->dayt, FisheryArray[sellernf].fisheryCode, sellerns, FunctGroupArray[sp].groupCode, FisheryArray[buyernf].fisheryCode, buyerns, demand, supply);
							Trade(bm, sp, bm->MofY, buyernf, buyerns, sellernf, sellerns, demand, supply, bm->MofY, bm->MofY, max_month_effort, &remain_demand,
									&remain_supply, llogfp);
						}

						if (do_debug) {
							fprintf(
									llogfp,
									"Time: %e, itc: %d, %s, demand %e, supply: %e, remain_demand: %e, remain_supply: %e, MofY: %d (buyernf: %d, buyerns: %d, sellernf: %d, sellerns: %d, max_month_effort: %e)\n",
									bm->dayt, itc, FunctGroupArray[sp].groupCode, demand, supply, remain_demand, remain_supply, bm->MofY, buyernf, buyerns,
									sellernf, sellerns, max_month_effort);
						}

						/* Check for remaining supply and demand */
						if (remain_demand > max_demand[sp])
							max_demand[sp] = remain_demand;

						if (bm->QuotaAlloc[sellernf][sellerns][sp][spare2sell_id] > 0)
							any2spare[sp] += remain_supply;

						if (do_debug) {
							fprintf(llogfp, "Time: %e, itc: %d, %s, maxdemand %e, anyspare: %e, spare2sell: %e (remain_demand: %e, remain_supply: %e)\n",
									bm->dayt, itc, FunctGroupArray[sp].groupCode, max_demand[sp], any2spare[sp],
									bm->QuotaAlloc[sellernf][sellerns][sp][spare2sell_id], remain_demand, remain_supply);
						}

					}
				}
			}

			/* Reset while loop trigger appropriately - keep trading if demand and supply for any target is > 10kg */
			still_trading = 0;
			for (sp = 0; sp < bm->K_num_tot_sp; sp++) {
				if (FunctGroupArray[sp].isFished == TRUE) {
					if ((max_demand[sp] > bm->recon_buffer) && (any2spare[sp] > bm->recon_buffer))
						still_trading = 1;

					if (do_debug_base) {
						fprintf(llogfp, "Time: %e, itc: %d, %s still_trading: %d, max_demand: %e, any2spare: %e)\n", bm->dayt, itc,
								FunctGroupArray[sp].groupCode, still_trading, max_demand[sp], any2spare[sp]);
					}
				}
			}

			itc++;

			/* TOD: Change this to an input parameter */
			if (itc > 30)
				still_trading = 0;
		}
	}

	/** Update what is newly leased in (since last time calculated the lease prices) **/
	for (nf = 0; nf < bm->K_num_fisheries; nf++) {
		for (ns = 0; ns < bm->FISHERYprms[nf][nsubfleets_id]; ns++) {
			for (sp = 0; sp < bm->K_num_tot_sp; sp++) {
				if (FunctGroupArray[sp].isFished == TRUE) {
					if (bm->QuotaAlloc[nf][ns][sp][leased_id] > bm->QuotaAlloc[nf][ns][sp][oldleased_id]) {
						bm->QuotaAlloc[nf][ns][sp][newleased_id] = bm->QuotaAlloc[nf][ns][sp][leased_id] - bm->QuotaAlloc[nf][ns][sp][oldleased_id];
					}
				}
			}
		}
	}

	/** Consolidation of holdings **/
	Consolidate_Allowed_Catches(bm, llogfp);

	return;
}

/**
 *
 *	\brief Routine to trade quota
 *
 */
void Trade(MSEBoxModel *bm, int sp, int month, int buyernf, int buyerns, int sellernf, int sellerns, double demand, double supply, int buyermonth,
		int sellermonth, double max_month_effort, double *rdemand, double *rsupply, FILE *llogfp) {
	double scheduledEffort, expectCPUE_b, prop_olease, prop_tlease, ownQuota, final_VNR, final_VNRA, p_buy, p_perm, rndnum,
		old_perm, new_perm, old_temp, new_temp, remain_demand, remain_supply, orig_supply, orig_demand, amt_supplied,
		demand_used, supply_used, tot_2_lease;
	double ROUNDGUARD_TRADE = 0.0000001;
	double keep_trading = 1000000000.0; // IN HERE AS CLUDGE UNTIL SORT OUT QUOTA PRICES BETTER OTHERWISE ALL JUST SELL QUOTA
	double NRA_thresh = -100.0;
	//int nosupply;
	//double expectCPUE_s, seller_scheduledEffort;
	int nodemand;
	int do_debug;

	if ((bm->debug == debug_quota) && ((bm->which_fleet == buyernf) || (bm->which_fleet == sellernf) || (bm->which_fleet >= bm->K_num_fisheries))) {
		do_debug = 1;
	} else
		do_debug = 0;

	/* No need to trade */
	if ((demand == 0) || (supply == 0)) {
		fprintf(llogfp, "Time: %e %s-%d trading %s with %s-%d - how get zero demand or supply? (demand: %e, supply: %e)\n", bm->dayt,
				FisheryArray[sellernf].fisheryCode, sellerns, FunctGroupArray[sp].groupCode, FisheryArray[buyernf].fisheryCode, buyerns, demand, supply);
		return;
	}

	scheduledEffort = bm->EffortSchedule[buyernf][buyerns][buyermonth][expect_id];
	//seller_scheduledEffort = bm->EffortSchedule[sellernf][sellerns][sellermonth][expect_id];
	expectCPUE_b = bm->BlackBook[buyernf][buyerns][sp][buyermonth][expect_id] / (bm->EffortSchedule[buyernf][buyerns][buyermonth][hist_id] + small_num);
	//expectCPUE_s = bm->BlackBook[sellernf][sellerns][sp][sellermonth][expect_id] / (bm->EffortSchedule[sellernf][sellerns][sellermonth][hist_id] + small_num);

	orig_supply = supply;
	orig_demand = demand;

	//nosupply = 0;
	*rsupply = 0;
	*rdemand = 0;

	/* Calculate relevant probabiltities of buying versus leasing */
	prop_olease = bm->QuotaAlloc[buyernf][buyerns][sp][permolease_id];
	prop_tlease = bm->QuotaAlloc[buyernf][buyerns][sp][templease_id];
	ownQuota = (1.0 - prop_olease - prop_tlease) * bm->QuotaAlloc[buyernf][buyerns][sp][owned_id];

	/* Add leased quota to buyers information. First determine if bought, permanent ot
	 temporary lease. Also adjust sellers information accordingly.
	 VNRA - net present value.
	 */

	/* quotaprice_id is calculated before we work out what to trade.
	 * Assumes perfect knowledge of price does not take into account friendships etc.
	 * Same price is assumed per subfleet.
	 * Prices don't vary vessel to vessel.
	 *
	 * If subfleet is large then price will be across the entire subfleet.
	 *
	 * No random values are added to price.
	 *
	 * Perm lease or lease per year
	 * 	- similar equation to deciding if you decommission vessel.
	 *
	 * Buy - buy for ever. Costs about 10 times more
	 * Perm lease - hand shake lease.  about 2 times more expensive
	 * Lease - short term lease within the single year.
	 *
	 * The costs coeff for the different options is read in from the input files.
	 *
	 * Most of the quota trading is bycatch quota not target species.
	 * Trading often happens with different fisheries.
	 */
	final_VNRA = net_return[buyernf][buyerns][fut_id] - (bm->buy_coefft * bm->QuotaAlloc[buyernf][buyerns][sp][quotaprice_id] * demand + keep_trading);
	if (final_VNRA < NRA_thresh)
		final_VNRA = NRA_thresh;
	p_buy = 1.0 / (1.0 + exp(-final_VNRA));
	final_VNR = net_return[buyernf][buyerns][fut_id] - (bm->perm_coefft * bm->QuotaAlloc[buyernf][buyerns][sp][quotaprice_id] * demand + keep_trading);
	if (final_VNR < NRA_thresh)
		final_VNR = NRA_thresh;
	p_perm = 1.0 / (1.0 + exp(-final_VNR));

	rndnum = drandom(0.0, 1.0);

	if (do_debug) {
		fprintf(
				llogfp,
				"Time: %e %s-%d-%d trading %s to %s-%d-%d  (p: %e, p_buy: %e, p_perm: %e final_VNRA: %e expA: %e final_VNR: %e exp: %e, net_return: %e buy_coefft: %e quotaprice: %e perm_coefft: %e, demand: %e)\n",
				bm->dayt, FisheryArray[sellernf].fisheryCode, sellerns, sellermonth, FunctGroupArray[sp].groupCode, FisheryArray[buyernf].fisheryCode, buyerns,
				buyermonth, rndnum, p_buy, p_perm, final_VNRA, (double)exp(-final_VNRA), final_VNR, (double)exp(-final_VNR), net_return[buyernf][buyerns][fut_id],
				bm->buy_coefft, bm->QuotaAlloc[buyernf][buyerns][sp][quotaprice_id], bm->perm_coefft, demand);
		fprintf(llogfp, "Time: %e %s-%d-%d trading %s to %s-%d-%d  (Original supply: %e, original demand: %e)\n", bm->dayt, FisheryArray[sellernf].fisheryCode,
				sellerns, sellermonth, FunctGroupArray[sp].groupCode, FisheryArray[buyernf].fisheryCode, buyerns, buyermonth, orig_supply, orig_demand);
		fprintf(llogfp, "Original supply: %e, original demand: %e\n", orig_supply, orig_demand);
	}

	if (demand > supply) {
		/* Don't worry about rescaling based on expected effort in this case as it will be demand driven */

		/* Update holdings */
		if (rndnum < p_buy) {
			/* Bought quota */
			if (supply > bm->QuotaAlloc[sellernf][sellerns][sp][owned_id])
				supply = bm->QuotaAlloc[sellernf][sellerns][sp][owned_id];

			bm->QuotaAlloc[buyernf][buyerns][sp][owned_id] += supply;
			bm->QuotaAlloc[sellernf][sellerns][sp][owned_id] -= supply;
			supply_used = supply;

			bm->QuotaTrade[sellernf][buyernf][sp][amttrade_id] += supply;
			bm->QuotaTrade[sellernf][buyernf][sp][numtrade_id] += 1.0;

			/*
			 if(sp == FDB_id)
			 fprintf(llogfp,"Time: %e, %s-%d sold %e %s to %s-%d (seller owned: %e vs buyer owned: %e)\n",
			 bm->dayt, FisheryArray[sellernf].fisheryCode, sellerns, supply, FunctGroupArray[sp].groupCode, FisheryArray[buyernf].fisheryCode, buyerns, bm->QuotaAlloc[sellernf][sellerns][sp][owned_id], bm->QuotaAlloc[buyernf][buyerns][sp][owned_id]);
			 */

		} else if (rndnum < p_perm) {
			/* Check to see how much can be leased out */
			prop_olease = bm->QuotaAlloc[sellernf][sellerns][sp][permolease_id];
			prop_tlease = bm->QuotaAlloc[sellernf][sellerns][sp][templease_id];
			tot_2_lease = bm->QuotaAlloc[sellernf][sellerns][sp][permilease_id] * bm->QuotaAlloc[sellernf][sellerns][sp][leased_id] + (1.0 - prop_olease
					- prop_tlease) * bm->QuotaAlloc[sellernf][sellerns][sp][owned_id];
			if (tot_2_lease < 0)
				tot_2_lease = 0;

			if (supply > tot_2_lease)
				supply = tot_2_lease;

			/* Adding permanent lease to buyers holdings */
			old_perm = bm->QuotaAlloc[buyernf][buyerns][sp][permilease_id] * bm->QuotaAlloc[buyernf][buyerns][sp][leased_id];
			new_perm = (old_perm + supply) / (bm->QuotaAlloc[buyernf][buyerns][sp][leased_id] + supply);
			bm->QuotaAlloc[buyernf][buyerns][sp][permilease_id] = new_perm;
			bm->QuotaAlloc[buyernf][buyerns][sp][leased_id] += supply;
			supply_used = supply;

			bm->QuotaTrade[sellernf][buyernf][sp][amttrade_id] += supply;
			bm->QuotaTrade[sellernf][buyernf][sp][numtrade_id] += 1.0;

			/* Noting changes in sellers holdings */
			old_perm = prop_olease * bm->QuotaAlloc[sellernf][sellerns][sp][owned_id];
			if (supply > ((1.0 - prop_olease - prop_tlease) * bm->QuotaAlloc[sellernf][sellerns][sp][owned_id])) {
				/** Exhaust owned and dip in to leases too **/

				/* Update owned proportions */
				amt_supplied = (1.0 - prop_olease - prop_tlease) * bm->QuotaAlloc[sellernf][sellerns][sp][owned_id];
				new_perm = (amt_supplied + old_perm) / (bm->QuotaAlloc[sellernf][sellerns][sp][owned_id] + small_num);
				bm->QuotaAlloc[sellernf][sellerns][sp][permolease_id] = new_perm;

				/* Dip into own leased in quota */
				supply -= amt_supplied;
				bm->QuotaAlloc[sellernf][sellerns][sp][leased_id] -= supply;
			} else {
				/** All covered by owned quota - so update permanently leased out quota **/
				new_perm = (supply + old_perm) / (bm->QuotaAlloc[sellernf][sellerns][sp][owned_id] + small_num);
				bm->QuotaAlloc[sellernf][sellerns][sp][permolease_id] = new_perm;
			}

			/*
			 if(sp == FDB_id)
			 fprintf(llogfp,"Time: %e, %s-%d pleased %e (%e) %s to %s-%d (seller owned: %e, pleased: %e, leased: %e vs buyer owned: %e, pleased: %e, leased: %e)\n",
			 bm->dayt, FisheryArray[sellernf].fisheryCode, sellerns, supply_used, supply, FunctGroupArray[sp].groupCode, FisheryArray[buyernf].fisheryCode, buyerns, bm->QuotaAlloc[sellernf][sellerns][sp][owned_id], bm->QuotaAlloc[sellernf][sellerns][sp][permolease_id], bm->QuotaAlloc[sellernf][sellerns][sp][leased_id], bm->QuotaAlloc[buyernf][buyerns][sp][owned_id], bm->QuotaAlloc[buyernf][buyerns][sp][permilease_id], bm->QuotaAlloc[buyernf][buyerns][sp][leased_id]);
			 */

		} else {
			/* Check to see how much can be leased out */
			prop_olease = bm->QuotaAlloc[sellernf][sellerns][sp][permolease_id];
			prop_tlease = bm->QuotaAlloc[sellernf][sellerns][sp][templease_id];
			tot_2_lease = bm->QuotaAlloc[sellernf][sellerns][sp][leased_id] + (1.0 - prop_olease - prop_tlease)
					* bm->QuotaAlloc[sellernf][sellerns][sp][owned_id];
			if (tot_2_lease < 0)
				tot_2_lease = 0;

			/*
			 if(sp == FDB_id)
			 fprintf(llogfp,"Time: %e, %s leased: %e, prop_olease: %e, prop_tlease: %e, owned: %e, owned_contrib: %e, tot_2_lease: %e (supply: %e)\n",
			 bm->dayt, FunctGroupArray[sp].groupCode, bm->QuotaAlloc[sellernf][sellerns][sp][leased_id], prop_olease, prop_tlease, bm->QuotaAlloc[sellernf][sellerns][sp][owned_id], (1.0 - prop_olease - prop_tlease) * bm->QuotaAlloc[sellernf][sellerns][sp][owned_id], tot_2_lease, supply);
			 */

			if (supply > tot_2_lease)
				supply = tot_2_lease;

			/* Update buyer's holdings */
			bm->QuotaAlloc[buyernf][buyerns][sp][leased_id] += supply;
			supply_used = supply;

			bm->QuotaTrade[sellernf][buyernf][sp][amttrade_id] += supply;
			bm->QuotaTrade[sellernf][buyernf][sp][numtrade_id] += 1.0;

			/*
			 if(sp == FDB_id)
			 fprintf(llogfp, "Time: %e, %s supply_used: %e added to %s-%d tleased: %e (amttrade: %e, numtrade: %e)\n",
			 bm->dayt, FunctGroupArray[sp].groupCode, supply_used, FisheryArray[buyernf].fisheryCode, buyerns, bm->QuotaAlloc[buyernf][buyerns][sp][leased_id], bm->QuotaTrade[sellernf][buyernf][sp][amttrade_id], bm->QuotaTrade[sellernf][buyernf][sp][numtrade_id]);
			 */

			/* Temporary leases only - exhaust own leases before touch owned quota */
			if (supply < bm->QuotaAlloc[sellernf][sellerns][sp][leased_id]) {
				bm->QuotaAlloc[sellernf][sellerns][sp][leased_id] -= supply;
			} else {
				/* Hand over all leased quota first */
				supply -= bm->QuotaAlloc[sellernf][sellerns][sp][leased_id];
				bm->QuotaAlloc[sellernf][sellerns][sp][leased_id] = 0;

				/* Now go into owned quota */
				amt_supplied = (1.0 - prop_olease - prop_tlease) * bm->QuotaAlloc[sellernf][sellerns][sp][owned_id];

				if (amt_supplied < supply)
					supply = amt_supplied;

				old_temp = bm->QuotaAlloc[sellernf][sellerns][sp][templease_id] * bm->QuotaAlloc[sellernf][sellerns][sp][owned_id];
				new_temp = (old_temp + supply) / (bm->QuotaAlloc[sellernf][sellerns][sp][owned_id] + small_num);

				bm->QuotaAlloc[sellernf][sellerns][sp][templease_id] = new_temp;
			}

			/*
			 if(sp == FDB_id)
			 fprintf(llogfp,"Time: %e, %s-%d tleased %e (%e) %s to %s-%d (seller owned: %e, tleased: %e, leased: %e vs buyer owned: %e, pleased: %e, leased: %e)\n",
			 bm->dayt, FisheryArray[sellernf].fisheryCode, sellerns, supply_used, supply, FunctGroupArray[sp].groupCode, FisheryArray[buyernf].fisheryCode, buyerns, bm->QuotaAlloc[sellernf][sellerns][sp][owned_id], bm->QuotaAlloc[sellernf][sellerns][sp][templease_id], bm->QuotaAlloc[sellernf][sellerns][sp][leased_id], bm->QuotaAlloc[buyernf][buyerns][sp][owned_id], bm->QuotaAlloc[buyernf][buyerns][sp][permilease_id], bm->QuotaAlloc[buyernf][buyerns][sp][leased_id]);
			 */

		}

		bm->Trades[month][sp] += 1;
		remain_demand = demand - supply_used;
		remain_supply = 0;

		if (do_debug) {
			fprintf(llogfp, "Time: %e %s-%d-%d trading %e %s to %s-%d-%d  (%e demand remaining)\n", bm->dayt, FisheryArray[sellernf].fisheryCode, sellerns,
					sellermonth, orig_supply, FunctGroupArray[sp].groupCode, FisheryArray[buyernf].fisheryCode, buyerns, buyermonth, remain_demand);
		}

	} else {
		/* supply > demand */

		/* Check if demand needs to be rescaled based on realistic effort levels */
		if (bm->EconLimDemand && ((scheduledEffort + demand / (expectCPUE_b + small_num)) > max_month_effort)) {
			demand = (max_month_effort - scheduledEffort) * expectCPUE_b;
			nodemand = 0;
			if (demand < 0)
				demand = 0;
			if (!demand)
				nodemand = 1;

			if (nodemand) {
				/*No trade necessary as would require unrealistic levels of effort to use */
				fprintf(llogfp,
						"TIme: %e, Rescaling %s demand to %e based on realistic effort (max: %e, schedule: %e, expect: %e) - now no demand so don't trade\n",
						bm->dayt, FunctGroupArray[sp].groupCode, demand, max_month_effort, scheduledEffort, expectCPUE_b);
				return;
			}
		}

		/* Update holdings */
		if (rndnum < p_buy) {
			/* Bought quota */
			bm->QuotaAlloc[buyernf][buyerns][sp][owned_id] += demand;
			bm->QuotaAlloc[sellernf][sellerns][sp][owned_id] -= demand;
			demand_used = demand;

			bm->QuotaTrade[sellernf][buyernf][sp][amttrade_id] += demand;
			bm->QuotaTrade[sellernf][buyernf][sp][numtrade_id] += 1.0;

			/*
			 if(sp == FDB_id)
			 fprintf(llogfp,"Time: %e, %s-%d sold %e %s to %s-%d (seller owned: %e vs buyer owned: %e)\n",
			 bm->dayt, FisheryArray[sellernf].fisheryCode, sellerns, demand, FunctGroupArray[sp].groupCode, FisheryArray[buyernf].fisheryCode, buyerns, bm->QuotaAlloc[sellernf][sellerns][sp][owned_id], bm->QuotaAlloc[buyernf][buyerns][sp][owned_id]);
			 */

		} else if (rndnum < p_perm) {
			/* Check to see how much can be leased out */
			prop_olease = bm->QuotaAlloc[sellernf][sellerns][sp][permolease_id];
			prop_tlease = bm->QuotaAlloc[sellernf][sellerns][sp][templease_id];
			tot_2_lease = bm->QuotaAlloc[sellernf][sellerns][sp][permilease_id] * bm->QuotaAlloc[sellernf][sellerns][sp][leased_id] + (1.0 - prop_olease
					- prop_tlease) * bm->QuotaAlloc[sellernf][sellerns][sp][owned_id];
			if (tot_2_lease < 0)
				tot_2_lease = 0;

			if (demand > tot_2_lease)
				demand = tot_2_lease;

			/* Adding permanent lease to buyers holdings */
			old_perm = bm->QuotaAlloc[buyernf][buyerns][sp][permilease_id] * bm->QuotaAlloc[buyernf][buyerns][sp][leased_id];
			new_perm = (old_perm + demand) / (bm->QuotaAlloc[buyernf][buyerns][sp][leased_id] + demand);
			bm->QuotaAlloc[buyernf][buyerns][sp][permilease_id] = new_perm;
			bm->QuotaAlloc[buyernf][buyerns][sp][leased_id] += demand;
			demand_used = demand;

			bm->QuotaTrade[sellernf][buyernf][sp][amttrade_id] += demand;
			bm->QuotaTrade[sellernf][buyernf][sp][numtrade_id] += 1.0;

			/*
			 if(sp == FDB_id)
			 fprintf(llogfp, "Time: %e, %s demand_used: %e added to %s-%d pleased: %e (old_perm: %e, new_perm: %e)\n",
			 bm->dayt, FunctGroupArray[sp].groupCode, demand_used, FisheryArray[buyernf].fisheryCode, buyerns, bm->QuotaAlloc[buyernf][buyerns][sp][leased_id], old_perm, new_perm);
			 */

			/* Noting changes in sellers holdings */
			old_perm = prop_olease * bm->QuotaAlloc[sellernf][sellerns][sp][owned_id];
			if (demand > ((1.0 - prop_olease - prop_tlease) * bm->QuotaAlloc[sellernf][sellerns][sp][owned_id])) {
				/** Exhaust owned and dip in to leases too **/

				/* Update owned proportions */
				amt_supplied = (1.0 - prop_olease - prop_tlease) * bm->QuotaAlloc[sellernf][sellerns][sp][owned_id];
				new_perm = (amt_supplied + old_perm) / (bm->QuotaAlloc[sellernf][sellerns][sp][owned_id] + small_num);
				bm->QuotaAlloc[sellernf][sellerns][sp][permolease_id] = new_perm;

				/* Dip into own leased in quota */
				demand -= amt_supplied;
				bm->QuotaAlloc[sellernf][sellerns][sp][leased_id] -= demand;

			} else {
				/* All covered by owned quota */
				new_perm = (demand + old_perm) / (bm->QuotaAlloc[sellernf][sellerns][sp][owned_id] + small_num);
				bm->QuotaAlloc[sellernf][sellerns][sp][permolease_id] = new_perm;
			}

			/*
			 if(sp == FDB_id)
			 fprintf(llogfp,"Time: %e, %s-%d pleased %e (%e) %s to %s-%d (seller owned: %e, pleased: %e, leased: %e vs buyer owned: %e, pleased: %e, leased: %e)\n",
			 bm->dayt, FisheryArray[sellernf].fisheryCode, sellerns, demand_used, demand, FunctGroupArray[sp].groupCode, FisheryArray[buyernf].fisheryCode, buyerns, bm->QuotaAlloc[sellernf][sellerns][sp][owned_id], bm->QuotaAlloc[sellernf][sellerns][sp][permolease_id], bm->QuotaAlloc[sellernf][sellerns][sp][leased_id], bm->QuotaAlloc[buyernf][buyerns][sp][owned_id], bm->QuotaAlloc[buyernf][buyerns][sp][permilease_id], bm->QuotaAlloc[buyernf][buyerns][sp][leased_id]);
			 */

		} else {
			/** Temporary leases only - exhaust own leases before touch owned quota **/
			/* Check to see how much can be leased out */
			prop_olease = bm->QuotaAlloc[sellernf][sellerns][sp][permolease_id];
			prop_tlease = bm->QuotaAlloc[sellernf][sellerns][sp][templease_id];
			tot_2_lease = bm->QuotaAlloc[sellernf][sellerns][sp][leased_id] + (1.0 - prop_olease - prop_tlease)
					* bm->QuotaAlloc[sellernf][sellerns][sp][owned_id];
			if (tot_2_lease < 0)
				tot_2_lease = 0;

			if (demand > tot_2_lease) {
				demand = tot_2_lease;

				/*
				 if(sp == FDB_id)
				 fprintf(llogfp,"Time: %e %s now with demand = %e as tot_2_lease = %e\n",
				 bm->dayt, FunctGroupArray[sp].groupCode, demand, tot_2_lease);
				 */
			}

			/* Update buyer's holdings */
			bm->QuotaAlloc[buyernf][buyerns][sp][leased_id] += demand;
			demand_used = demand;

			bm->QuotaTrade[sellernf][buyernf][sp][amttrade_id] += demand;
			bm->QuotaTrade[sellernf][buyernf][sp][numtrade_id] += 1.0;

			/*
			 if(sp == FDB_id)
			 fprintf(llogfp, "Time: %e, %s demand_used: %e (%e) added to %s-%d tleased: %e (sleased: %e, sowned: %e, po: %e, pt: %e, tot_2_lease: %e)\n",
			 bm->dayt, FunctGroupArray[sp].groupCode, demand_used, orig_demand, FisheryArray[buyernf].fisheryCode, buyerns, bm->QuotaAlloc[buyernf][buyerns][sp][leased_id], bm->QuotaAlloc[sellernf][sellerns][sp][leased_id], bm->QuotaAlloc[sellernf][sellerns][sp][owned_id], prop_olease, prop_tlease, tot_2_lease);
			 */

			if (demand < bm->QuotaAlloc[sellernf][sellerns][sp][leased_id]) {
				bm->QuotaAlloc[sellernf][sellerns][sp][leased_id] -= demand;
			} else {
				/* Hand over all leased quota first */
				demand -= bm->QuotaAlloc[sellernf][sellerns][sp][leased_id];
				bm->QuotaAlloc[sellernf][sellerns][sp][leased_id] = 0;

				/* Now go into owned quota */
				amt_supplied = (1.0 - prop_olease - prop_tlease) * bm->QuotaAlloc[sellernf][sellerns][sp][owned_id];
				if (amt_supplied < demand)
					demand = amt_supplied;

				old_temp = bm->QuotaAlloc[sellernf][sellerns][sp][templease_id] * bm->QuotaAlloc[sellernf][sellerns][sp][owned_id];
				new_temp = (old_temp + demand) / (bm->QuotaAlloc[sellernf][sellerns][sp][owned_id] + small_num);
				bm->QuotaAlloc[sellernf][sellerns][sp][templease_id] = new_temp;

				// Do a rounding error check
				if ((new_temp > 1.0) && (new_temp < (1.0 + small_num)))
					new_temp = 1.0;

				if (new_temp > 1.0)
					fprintf(llogfp, "Time: %e, %s from %s-%d to %s-%d new_temp: %e, old_temp: %e, owned: %e, tlease: %e, demand: %e\n", bm->dayt,
							FunctGroupArray[sp].groupCode, FisheryArray[sellernf].fisheryCode, sellerns, FisheryArray[buyernf].fisheryCode, buyerns, new_temp,
							old_temp, bm->QuotaAlloc[sellernf][sellerns][sp][owned_id], bm->QuotaAlloc[sellernf][sellerns][sp][templease_id], demand);

			}

			/*
			 if(sp == FDB_id)
			 fprintf(llogfp,"Time: %e, %s-%d tleased %e (%e) %s to %s-%d (seller owned: %e, tleased: %e, leased: %e vs buyer owned: %e, pleased: %e, leased: %e)\n",
			 bm->dayt, FisheryArray[sellernf].fisheryCode, sellerns, demand_used, demand, FunctGroupArray[sp].groupCode, FisheryArray[buyernf].fisheryCode, buyerns, bm->QuotaAlloc[sellernf][sellerns][sp][owned_id], bm->QuotaAlloc[sellernf][sellerns][sp][templease_id], bm->QuotaAlloc[sellernf][sellerns][sp][leased_id], bm->QuotaAlloc[buyernf][buyerns][sp][owned_id], bm->QuotaAlloc[buyernf][buyerns][sp][permilease_id], bm->QuotaAlloc[buyernf][buyerns][sp][leased_id]);
			 */
		}

		bm->Trades[month][sp] += 1;
		remain_demand = 0;
		remain_supply = supply - demand_used;

		if (do_debug) {
			fprintf(llogfp, "Time: %e %s-%d-%d trading %e %s to %s-%d-%d  (%e demand remaining)\n", bm->dayt, FisheryArray[sellernf].fisheryCode, sellerns,
					sellermonth, orig_demand, FunctGroupArray[sp].groupCode, FisheryArray[buyernf].fisheryCode, buyerns, buyermonth, remain_demand);
		}
	}

	/* Check whether demand still exists for this buyer */
	if (remain_demand < bm->recon_buffer)
		bm->QuotaAlloc[buyernf][buyerns][sp][within_id] = 0;
	else {
		bm->QuotaAlloc[buyernf][buyerns][sp][within_id] = 1;
		bm->SUBFLEET_ECONprms[buyernf][buyerns][within_id] = 1;
	}

	/* Check whether this seller still has surplus to shift */
	if (remain_supply < bm->recon_buffer)
		bm->QuotaAlloc[sellernf][sellerns][sp][spare2sell_id] = 0;
	else
		bm->QuotaAlloc[sellernf][sellerns][sp][spare2sell_id] = 1;

	if (do_debug) {
		fprintf(llogfp, "Time: %e within: %e, spare2sell: %e, demand: %e, supply: %e, remain_demand: %e, remain_supply: %e)\n", bm->dayt,
				bm->QuotaAlloc[buyernf][buyerns][sp][within_id], bm->QuotaAlloc[sellernf][sellerns][sp][spare2sell_id], orig_demand, orig_supply,
				remain_demand, remain_supply);
	}

	*rsupply = remain_supply;
	*rdemand = remain_demand;

	/* Do a rounding error check */
	prop_olease = bm->QuotaAlloc[buyernf][buyerns][sp][permolease_id];
	if ((prop_olease > 1.0) && (prop_olease < (1.0 + ROUNDGUARD_TRADE)))
		bm->QuotaAlloc[buyernf][buyerns][sp][permolease_id] = 1.0;
	prop_tlease = bm->QuotaAlloc[buyernf][buyerns][sp][templease_id];
	if ((prop_tlease > 1.0) && (prop_tlease < (1.0 + ROUNDGUARD_TRADE)))
		bm->QuotaAlloc[buyernf][buyerns][sp][templease_id] = 1.0;
	prop_olease = bm->QuotaAlloc[sellernf][sellerns][sp][permolease_id];
	if ((prop_olease > 1.0) && (prop_olease < (1.0 + ROUNDGUARD_TRADE)))
		bm->QuotaAlloc[sellernf][sellerns][sp][permolease_id] = 1.0;
	prop_tlease = bm->QuotaAlloc[sellernf][sellerns][sp][templease_id];
	if ((prop_tlease > 1.0) && (prop_tlease < (1.0 + ROUNDGUARD_TRADE)))
		bm->QuotaAlloc[sellernf][sellerns][sp][templease_id] = 1.0;

	/* Sanity check on quota holdings */
	prop_olease = bm->QuotaAlloc[buyernf][buyerns][sp][permolease_id];
	prop_tlease = bm->QuotaAlloc[buyernf][buyerns][sp][templease_id];
	ownQuota = (1.0 - prop_olease - prop_tlease) * bm->QuotaAlloc[buyernf][buyerns][sp][owned_id];
	if ((ownQuota < 0.0) || (bm->QuotaAlloc[buyernf][buyerns][sp][leased_id] < 0.0)) {
		fprintf(llogfp, "Time: %e %s-%d has negative quota holdings - ownQuota: %e (p_olease: %e, p_tlease: %e, owned: %e), leased: %e\n", bm->dayt,
				FisheryArray[buyernf].fisheryCode, buyerns, ownQuota, prop_olease, prop_tlease, bm->QuotaAlloc[buyernf][buyerns][sp][owned_id],
				bm->QuotaAlloc[buyernf][buyerns][sp][leased_id]);
		quit("Negative quota");
	}
	prop_olease = bm->QuotaAlloc[sellernf][sellerns][sp][permolease_id];
	prop_tlease = bm->QuotaAlloc[sellernf][sellerns][sp][templease_id];
	ownQuota = (1.0 - prop_olease - prop_tlease) * bm->QuotaAlloc[sellernf][sellerns][sp][owned_id];
	if ((ownQuota < 0.0) || (bm->QuotaAlloc[sellernf][sellerns][sp][leased_id] < 0.0)) {
		fprintf(llogfp, "Time: %e %s-%d has negative quota holdings - ownQuota: %e (p_olease: %e, p_tlease: %e, owned: %e), leased: %e\n", bm->dayt,
				FisheryArray[sellernf].fisheryCode, sellerns, ownQuota, prop_olease, prop_tlease, bm->QuotaAlloc[sellernf][sellerns][sp][owned_id],
				bm->QuotaAlloc[sellernf][sellerns][sp][leased_id]);
		quit("Negative quota");
	}

	if (do_debug) {
		fprintf(llogfp, "Time: %e, %s-%d now has amttrade: %e (numtrade: %e) for %s\n", bm->dayt, FisheryArray[buyernf].fisheryCode, buyerns,
				bm->QuotaTrade[sellernf][buyernf][sp][amttrade_id], bm->QuotaTrade[sellernf][buyernf][sp][numtrade_id], FunctGroupArray[sp].groupCode);
	}

	return;
}
