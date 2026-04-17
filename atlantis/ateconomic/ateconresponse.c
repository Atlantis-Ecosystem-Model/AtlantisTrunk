/**
 \file
 \brief C file for economic models for Atlantis
 \ingroup ateconomic

 File:	ateconresponse.c
 Author:	Beth Fulton
 Created:	27/9/2005
 Purpose:	C file for economic models for Atlantis
 Revisions: 27/9/2005 Created the file (to split it away from file
 where calculate indicators)

 8/9/2006 Made the currency of the economic model kg - meant
 conversion from mg necessary when allocating catch

 20/2/2007 Added MultiPlanEffort check so that do NOT do quota allocation
 if using Dan Holland economic model (as not quota based). Likewise
 not updating of vessel numbers under that model.

 12/4/2007 Corrected decomissioning calculation (was back to front)

 28-10-2009 Bec Gorton
 Removed references to the K_num_fished_sp and K_num_impacted_sp. Arrays that were this
 size have been resized to K_num_tot_sp.

 04-11-2009 Bec Gorton
 Merged Beths bycatch incentive code - revision 961.
 **/

/*  Open library routines */
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <sjwlib.h>
#include <atEconomic.h>

/**
 *	\brief Boxmodel economics model - dynamics economics and response model (where make
 *	decisions on effort allocation in response to socio-economic drivers)
 *
 *	This is called once a month.
 *
 *****************************************************************************/
void Economic_Response(MSEBoxModel *bm, FILE *llogfp)
{

	/* So don't get divergence in reporting of statistics between
	 different economic models do some updating and reporting here
	 (Otherwise end up with monthly total marginal rent recorded
	 in one model and annual values in other models)

	 Update Profits and Rents
	 */

	Marginal_Profit_Calc(bm, llogfp);
	Marginal_Rent_Calc(bm, llogfp);

	/* Economic decision - decide on target groups could be done here,
	 but currently done in Annual_Effort_Schedule

	 Investment-Disinvestment called here - so can determine whether
	 boats have entered/left the fishery.
	 */
	Update_Vessel_Numbers(bm, llogfp);

	/* Social considerations - might need to reset the size of port as there might be too many vessels. */
	Port_Growth(bm, llogfp);

	return;
}

/**
 *	\brief Port population model - concerned with linking port population sizes to
 *	economics (profits) of fisheries and landings at each port location
 *
 */
void Port_Growth(MSEBoxModel *bm, FILE *llogfp) {
	int porti, nf, ns, home_port, flagfishhere, flagchangePOP, POP_num_changes, i, now_change, past_change;
	double POP_active_scale, pop_num, end_date, POPmultA, POPmultB, POPstart, POPperiod, POP_scale;

	flagfishhere = 0;

	if (verbose > 1)
		printf("Doing port activity\n");

	/* Initialise ports */
	for (porti = 0; porti < bm->K_num_ports; porti++) {
		portweight[porti] = 0;
	}

	/* Find fishing activity scalar */
	for (nf = 0; nf < bm->K_num_fisheries; nf++) {
		for (ns = 0; ns < bm->FISHERYprms[nf][nsubfleets_id]; ns++) {
			/* If no boats in the subfleet currently skip ahead */
			if (!bm->SUBFLEET_ECONprms[nf][ns][nboat_id]) {
				continue;
			}

			/* Find main home port of the subfleet and weight heavily for that */
			home_port = (int) (bm->SUBFLEET_ECONprms[nf][ns][home_port_id]);
			if (home_port < bm->K_num_ports) {
				portweight[home_port] += bm->SUBFLEET_ECONprms[nf][ns][nboat_id];
			}
		}
	}

	for (porti = 0; porti < bm->K_num_ports; porti++) {

        flagfishhere = 1;
        
		/* Check if currently active - only interested in active ports */
		if ((bm->dayt < bm->Port_info[porti][port_start_id]) || (bm->dayt > bm->Port_info[porti][port_end_id]))
			flagfishhere = 0;
        
		if (!flagfishhere)
			continue;

		/* If fishery uses the port then add this to the activity scalar */
		for (nf = 0; nf < bm->K_num_fisheries; nf++) {
			if (bm->Port_Fishery[nf][porti])
				portweight[porti] += 1;
		}

		/* Give a nominal value to a non-value so that not dividing by zero as this can blow up to
		 ridiculous levels in the new / old term below
		 */
		if (!portweight[porti])
			portweight[porti] = 0.9;

		/* Update activity index */
		bm->Port_info[porti][old_act_id] = bm->Port_info[porti][new_act_id];
		bm->Port_info[porti][new_act_id] = portweight[porti];
		POP_active_scale = bm->Port_info[porti][new_act_id] / (bm->Port_info[porti][old_act_id] + small_num);

		/* Basic Population Growth */
		pop_num = bm->Port_info[porti][population_id];
		flagchangePOP = (int) (bm->Port_info[porti][popchange_id]);
		POP_num_changes = (int) (bm->Port_info[porti][POP_num_changes_id]);
		if (flagchangePOP && (bm->dayt >= bm->POPchange[porti][0][start_id]) && (bm->POPchange[porti][0][start_id] != 0)) {
			now_change = 0;
			for (i = 0; i < POP_num_changes; i++) {
				if (bm->dayt >= bm->POPchange[porti][i][start_id])
					now_change = i;
			}
			end_date = (bm->POPchange[porti][now_change][start_id] + bm->POPchange[porti][now_change][period_id]);
			past_change = now_change - 1;
			POPmultA = bm->POPchange[porti][now_change][mult_id];
			if (now_change > 0)
				POPmultB = bm->POPchange[porti][past_change][mult_id];
			else
				POPmultB = 1;
			POPstart = bm->POPchange[porti][now_change][start_id];
			POPperiod = bm->POPchange[porti][now_change][period_id] + small_num;

			if (end_date < bm->dayt) {
				POP_scale = POPmultA;
			} else {
				if (POPmultA >= POPmultB) {
					POP_scale = POPmultB + (POPmultA - POPmultB) * (bm->dayt - POPstart) / POPperiod;
				} else {
					POP_scale = POPmultB - (POPmultB - POPmultA) * (bm->dayt - POPstart) / POPperiod;
				}
			}
		} else
			POP_scale = 1.0;

		bm->Port_info[porti][popactive_id] = pop_num * POP_scale * POP_active_scale;
	}

	return;
}

/**
 *
 *	\brief Update number of vessels per fleet (across subfleets)
 *	- also update market weighting
 *	-
 *	- Any vessel can go to any fleet. There must be a slot for it to fill.
 *	- There are also economic considerations.
 *	-
 *	- To make probability to switching zero drive up the input parameters.
 *	- Could put months of debt allow very high so they never have to sell out.
 *	-
 *	- We could put in a flag to not allow them to leave or switch.
 *	-
 *	- If a vessel is not profitable will it stop fishing?
 *	- 	Depends on the options you choose in your input files.
 *	-
 *	- Can allow them to tie up instead of leaving
 *	- 	would need to make onshore costs very low.
 *	-
 *	- Interesting values
 *	- propleave - set to 0 not boats will ever leave - there is a single probability for the entire model not a value per fishery.
 *	-
 *
 */
void Update_Vessel_Numbers(MSEBoxModel *bm, FILE *llogfp) {
	double max_marg_rent, tot_net_return1, avg_net_return1, final_net_return1, resale1, crew_size, final_VNR1, p_decom, switch_cost, p_md, p_switch, rndnum,
			totportweight, totmarketwgt, marketwgt, rndnum2, step2, new_cost, p_new, overshot, months_of_debt, bank_vault, prop_buyback, final_VNR,
			tot_net_return0, avg_net_return0, final_net_return0, final_VNR0, resale0, supp_cost = 0, rent_ratio, max_rent_ratio, p_supp, boats_switching = 0,
			maxp_flexgear = 0, p_flexgear;
	int ns, nf, sp, month, nyr, num_moving, fishery_id, subf_id, this_fishery, this_subfleet, new_boats, porti, home_port, old_boats, nbuyout, do_rescale,
			num_new, tot_new, debt_lost, do_debug_base, do_debug = 0;
	int maxmonth = 12;
    int orig_max_num_boats, boat_count;

	if (((bm->debug == debug_econeffort) || (bm->debug == debug_econ)) && (bm->dayt > bm->checkstart)) {
		do_debug_base = 1;
	} else
		do_debug_base = 0;

	if (verbose > 1)
		printf("Doing vessel investment, disinvestment and fleet switching\n");

	/* If using Dan Holland model skip ahead as fleet size doesn't change in absolute
	 size through time in that model
	 */
	if (!bm->MultiPlanEffort)
		return;

	/* Initialise */
	for (nf = 0; nf < bm->K_num_fisheries; nf++) {
		if (bm->FISHERYprms[nf][flageffortmodel_id] > rec_econ_model) {
			for (ns = 0; ns < bm->FISHERYprms[nf][nsubfleets_id]; ns++) {
				boats_free[nf][ns] = 0;
				if (!bm->TofY) {
					bm->SUBFLEET_ECONprms[nf][ns][newboat_id] = 0;
					bm->SUBFLEET_ECONprms[nf][ns][switchboat_id] = 0;
					bm->SUBFLEET_ECONprms[nf][ns][lasttot_cash_id] = bm->SUBFLEET_ECONprms[nf][ns][tot_cash_id];
					bm->SUBFLEET_ECONprms[nf][ns][tot_cash_id] = 0;
					bm->SUBFLEET_ECONprms[nf][ns][lostboat_id] = 0;

					/* Bound on number of boats leaving the fishery in one year independent of forced buyouts */
					bm->SUBFLEET_ECONprms[nf][ns][fleet_thresh_id] = bm->SUBFLEET_ECONprms[nf][ns][nboat_id] * (bm->prop_leave * 12.0);
				}
			}
		}
	}
    orig_max_num_boats = bm-> K_max_num_boats;
    
	/** Boats leaving or entering the fishery - based on models by Thebaud et al 2006 and Guyader 2002 */
	for (nf = 0; nf < bm->K_num_fisheries; nf++) {
		if (bm->FISHERYprms[nf][flageffortmodel_id] > rec_econ_model) {
			if (do_debug_base && ((bm->which_fleet == nf) || (bm->which_fleet >= bm->K_num_fisheries))) {
				do_debug = 1;
			} else
				do_debug = 0;

			bm->FISHERYprms[nf][nvessel_id] = 0;
			max_marg_rent = 0;
			best_subfleet[nf] = -1;
			tot_net_return0 = 0;
			tot_net_return1 = 0;

			/** Check for boats leaving fishery **/
			for (ns = 0; ns < bm->FISHERYprms[nf][nsubfleets_id]; ns++) {
				/* If no boats in the subfleet currently skip ahead */
				if (!bm->SUBFLEET_ECONprms[nf][ns][nboat_id]) {
					continue;
				}
				/* Use current net returns (tot_marg_rent) over a year to the fishery as
				 assumed rate of returns into future.
				 */

				/* Find the net returns for use in relative probabilties below */
				net_return[nf][ns][immed_id] = 0;
				net_return[nf][ns][fut_id] = 0;
				for (nyr = 0; nyr < bm->yr_horizon; nyr++) {
					net_return[nf][ns][fut_id] += ((tot_marg_rent[nf][ns] / (pow((1.0 + bm->interest_rate), nyr) + small_num))
							/ (bm->SUBFLEET_ECONprms[nf][ns][boat_size_id] + small_num));
					net_return[nf][ns][immed_id] += ((tot_marg_rent[nf][ns] / (pow((1.0 + bm->interest_rate), 0) + small_num))
							/ (bm->SUBFLEET_ECONprms[nf][ns][boat_size_id] + small_num));
				}
				tot_net_return1 += net_return[nf][ns][fut_id];
				tot_net_return0 += net_return[nf][ns][immed_id];

				/* Check for most profitable subfleet as new boats (if any) will enter here */
                if (tot_marg_rent[nf][ns] > max_marg_rent){
                    best_subfleet[nf] = ns;
                    max_marg_rent = tot_marg_rent[nf][ns];
                }
			}

			avg_net_return1 = tot_net_return1 / (bm->FISHERYprms[nf][nsubfleets_id] + small_num);
			avg_net_return0 = tot_net_return0 / (bm->FISHERYprms[nf][nsubfleets_id] + small_num);
			prop_buyback = 0;

			for (ns = 0; ns < bm->FISHERYprms[nf][nsubfleets_id]; ns++) {
				/* If no boats in the subfleet currently skip ahead */
				if (!bm->SUBFLEET_ECONprms[nf][ns][nboat_id]) {
					continue;
				}

				/** Find relative probabilities of decomissioning and switching of fleets **/
				final_net_return1 = net_return[nf][ns][fut_id] / (avg_net_return1 + small_num);
				resale1 = log((bm->SUBFLEET_ECONprms[nf][ns][resale_id] / (pow((1.0 + bm->interest_rate), bm->yr_horizon) + small_num))
						/ (bm->SUBFLEET_ECONprms[nf][ns][boat_size_id] + small_num));
				final_net_return0 = net_return[nf][ns][immed_id] / (avg_net_return0 + small_num);
				resale0 = log((bm->SUBFLEET_ECONprms[nf][ns][resale_id] / (pow((1.0 + bm->interest_rate), 0) + small_num))
						/ (bm->SUBFLEET_ECONprms[nf][ns][boat_size_id] + small_num));
				crew_size = log(bm->SUBFLEET_ECONprms[nf][ns][crew_size_id]);

				/* Decomissioning probabilty */
				final_VNR1 = bm->decom_return_coefft * final_net_return1 + bm->resale_coeff * resale1 - bm->decom_crew_coefft * crew_size; // sell later
				final_VNR0 = bm->decom_return_coefft * final_net_return0 + bm->resale_coeff * resale0 - bm->decom_crew_coefft * crew_size; // sell now
				final_VNR = final_VNR0 - final_VNR1;
				p_decom = 1.0 / (1.0 + exp(-final_VNR));

				//fprintf(llogfp,"Time: %e, %s-%d p_decom: %e, finalVNR: %e, final_VNR1: %e, finNetReturn1: %e resale1: %e, final_VNR0: %e, finNetReturn0: %e resale0: %e, tot_marg_rent: %e\n",
				//	bm->dayt, FisheryArray[nf].fisheryCode, ns, p_decom, final_VNR, final_VNR1, final_net_return1, resale1, final_VNR0, final_net_return0, resale0, tot_marg_rent[nf][ns]);

				/* Switching probability - if allowed to switch */
                if (bm->flagswitch) {
                    switch_cost = bm->SUBFLEET_ECONprms[nf][ns][switch_cost_id];
                    final_VNR = bm->switch_return_coefft * final_net_return1 - bm->switch_coefft * switch_cost - bm->decom_crew_coefft * crew_size;
                    p_switch = 1.0 / (1.0 + exp(-final_VNR));
                } else
                    p_switch = 0.0;

				//fprintf(llogfp,"Time: %e, p_switch: %e, finalVNR: %e\n", bm->dayt, p_switch, final_VNR);

				/* Supplementing probability - where those that can used mixed gears */
				p_flexgear = 0;
				max_rent_ratio = 0;
				if (bm->flagsupp_allowed) {
					supp_cost = bm->SUBFLEET_ECONprms[nf][ns][supp_cost_id];
					for (fishery_id = 0; fishery_id < bm->K_num_fisheries; fishery_id++) {
						for (subf_id = 0; subf_id < bm->FISHERYprms[fishery_id][nsubfleets_id]; subf_id++) {
							/* If value of being in current fleet is less than if you try a different gear
							 then do that instead
							 */
							if (tot_marg_rent[nf][ns] < (tot_marg_rent[fishery_id][subf_id] - supp_cost)) {
								/* Only allowed to swap with other dynamic fisheries */
								if (bm->FISHERYprms[fishery_id][flageffortmodel_id] > rec_econ_model) {
									rent_ratio = tot_marg_rent[fishery_id][subf_id] / (tot_marg_rent[nf][ns] - supp_cost + small_num);
									if (rent_ratio > max_rent_ratio) {
										max_rent_ratio = rent_ratio;
									}

									/* Case when opt for any more profitable than the current one */
									if (rent_ratio > 1.0)
										p_flexgear += (1.0 - 1.0 / (rent_ratio + small_num));
								} else {
									fprintf(llogfp, "Time: %e, %s-%d eyeing off %s-%d\n", bm->dayt, FisheryArray[nf].fisheryCode, ns,
											FisheryArray[fishery_id].fisheryCode, subf_id);
								}
							}
						}
					}
					/* Case when all opt for the most profitable */
					p_supp = 1.0 - 1.0 / (max_rent_ratio + small_num);
				} else
					p_supp = 0.0;

				/* Check that general redistribution hasn't exceeded current subfleet size */
				maxp_flexgear = p_flexgear;
				if (p_flexgear > 1.0)
					p_flexgear = 1.0;

				if (bm->flagsupp_allowed < 2)
					p_flexgear = 0.0;

				num_moving = 0;
				rndnum = drandom(0.0, 1.0);

				/* Supplementing gear */
				if ((rndnum < p_flexgear) || (rndnum < p_supp)) {
					rndnum2 = drandom(0.0, bm->prop_supp) * bm->SUBFLEET_ECONprms[nf][ns][nboat_id];
					num_moving = (int) (min(1.0, floor(rndnum2)));  // Curently capped at 1 boat a month moving.... could set ghis to max to allow more
					boats_free[nf][ns] += num_moving;

					fprintf(llogfp, "Time: %e %d boats supplementing from %s-%d (num_moving; %d, boats_free: %d)\n", bm->dayt, num_moving,
							FisheryArray[nf].fisheryCode, ns, num_moving, boats_free[nf][ns]);

				} else if (rndnum < p_switch) {
					/* Simply switching fleets */
					rndnum2 = drandom(0.0, bm->prop_switch) * bm->SUBFLEET_ECONprms[nf][ns][nboat_id];
					num_moving = (int) (min(1.0, floor(rndnum2)));
					boats_free[nf][ns] += num_moving;

					fprintf(llogfp, "Time: %e %d boats switching from %s-%d (num_moving; %d, boats_free: %d)\n", bm->dayt, num_moving,
							FisheryArray[nf].fisheryCode, ns, num_moving, boats_free[nf][ns]);

				} else if (rndnum < p_decom) {
					/* Actual decomissioning - nothing to do actually as will be lost below
					 as boats_free not being reset
					 */
					rndnum2 = drandom(0.0, bm->prop_leave) * bm->SUBFLEET_ECONprms[nf][ns][nboat_id];

					/* If don't just want one ticking over every time trigger tripped, do second
					 check vs month of year
					 */
					p_md = 0;
					if (bm->flagslowsell && (rndnum2 < 1.0)) {
						p_md = (int) (floor(rndnum2 * 12.0));
						if (p_md < bm->MofY)
							num_moving = 1;
						else
							num_moving = 0;
					} else {
						/* If trigger tripped at least one has to go */
						step2 = floor(rndnum2);
						num_moving = (int) (max(1.0, step2));
					}

					fprintf(llogfp, "Time: %e %d boats decomissioned in %s-%d (rnd2: %e, p_md: %e, MofY: %d)\n", bm->dayt, num_moving,
							FisheryArray[nf].fisheryCode, ns, rndnum2, p_md, bm->MofY);

				} else {
					/* None leaving the fishery */
					num_moving = 0;
				}
				old_boats = (int) (bm->SUBFLEET_ECONprms[nf][ns][nboat_id]);

				/* Find those leaving due to crippling debt */
				months_of_debt = bm->SUBFLEET_ECONprms[nf][ns][months_crippled_id];
				debt_lost = 0;
				if (months_of_debt > bm->cripple_period) {
					/* Lose current debt cripples and any lost to earlier "bad runs" in the year */
					debt_lost
							= (int) (ceil(bm->SUBFLEET_ECONprms[nf][ns][nboat_id] * bm->cripple_nboat * (1 + bm->SUBFLEET_ECONprms[nf][ns][cripples_lost_id])));
					num_moving += debt_lost;

					fprintf(llogfp, "Time: %e %d boats leaving %s-%d due to crippling debt (num_moving = %d, months_of_debt: %e, cripples_lost: %e)\n",
							bm->dayt, debt_lost, FisheryArray[nf].fisheryCode, ns, num_moving, months_of_debt, bm->SUBFLEET_ECONprms[nf][ns][cripples_lost_id]);

					bm->SUBFLEET_ECONprms[nf][ns][cripples_lost_id] = 0;
				}

				/* Find those who have run out of cash flow - except in first year of run
				 (to avoid initialisation issues)
				 */
				bank_vault = bm->SUBFLEET_ECONprms[nf][ns][lasttot_cash_id];
				if (!debt_lost && (bank_vault < bm->shorecost) && (bm->dayt > 364.0)) {
					rndnum2 = drandom(0.0, 1.0) * bm->SUBFLEET_ECONprms[nf][ns][nboat_id];
					debt_lost = (int) (min(1.0, floor(rndnum2))); // Again allowing 1 vessel at a time... if want more then use max
					num_moving += debt_lost;

					fprintf(llogfp, "Time: %e %d boats leaving %s-%d as bank vault dry (num_moving = %d)\n", bm->dayt, debt_lost, FisheryArray[nf].fisheryCode,
							ns, num_moving);

				}

				/* Check could actually lose that many boats */
				if (bm->SUBFLEET_ECONprms[nf][ns][lostboat_id] > bm->SUBFLEET_ECONprms[nf][ns][fleet_thresh_id])
					num_moving = 0;
				if (bm->SUBFLEET_ECONprms[nf][ns][nboat_id] < num_moving) {
					num_moving = old_boats;
				}
				if ((bm->SUBFLEET_ECONprms[nf][ns][nboat_id] - num_moving) < ceil(bm->SUBFLEET_ECONprms[nf][ns][nboat_orig_id] * bm->prop_boatbound)) {
					num_moving = (int) (bm->SUBFLEET_ECONprms[nf][ns][nboat_id] - ceil(bm->SUBFLEET_ECONprms[nf][ns][nboat_orig_id] * bm->prop_boatbound));
				}
				if (num_moving < 0)
					num_moving = 0;

				/* Directed (forced) buyback */
				nbuyout = 0;
				if ((bm->dayt > (bm->FISHERYprms[nf][buybackdate_id] + 7.0)) && (bm->FISHERYprms[nf][buybackdate_id] > (bm->dayt - 365.0))) {
					nbuyout = (int) (ceil(bm->SUBFLEET_ECONprms[nf][ns][nboat_id] * bm->SUBFLEET_ECONprms[nf][ns][propbuyback_id]));
					if (num_moving < nbuyout)
						num_moving = nbuyout;
					prop_buyback += bm->SUBFLEET_ECONprms[nf][ns][propbuyback_id];

					fprintf(llogfp, "Time: %e %d boats leaving %s-%d in buyout (num_moving = %d)\n", bm->dayt, nbuyout, FisheryArray[nf].fisheryCode, ns,
							num_moving);
				}

				/* Check allowed to lose boats at all */
				if (bm->hist_only)
					num_moving = 0;

				/* Total up the boats */
				bm->SUBFLEET_ECONprms[nf][ns][nboat_id] -= num_moving;
				bm->FISHERYprms[nf][nvessel_id] += bm->SUBFLEET_ECONprms[nf][ns][nboat_id];
				bm->SUBFLEET_ECONprms[nf][ns][lostboat_id] += num_moving;

				if (num_moving)
					fprintf(llogfp, "Time: %e %d boats leaving %s-%d\n", bm->dayt, num_moving, FisheryArray[nf].fisheryCode, ns);

				/* New boat probability */
				new_cost = bm->SUBFLEET_ECONprms[nf][ns][newboat_cost_id];
				final_VNR = bm->new_return_coefft * final_net_return1 - bm->new_coefft * new_cost;
				p_new = 1.0 / (1.0 + exp(-final_VNR));
				rndnum = drandom(0.0, 1.0);
				if (rndnum < p_new) {
					rndnum2 = drandom(0.0, 1.0) * (bm->FISHERYprms[nf][nlicence_id] - bm->FISHERYprms[nf][nvessel_id]);
					num_new = (int) (min(1.0, floor(rndnum2)));
					boats_new[nf][ns] += num_new;
				}
				if (boats_new[nf][ns] < 0)
					boats_new[nf][ns] = 0;

				if (boats_new[nf][ns])
					fprintf(llogfp, "Time: %e %d new boats joining %s-%d\n", bm->dayt, boats_new[nf][ns], FisheryArray[nf].fisheryCode, ns);
			}

			/* Update licences as result of forced buybacks */
			if (bm->FISHERYprms[nf][nsubfleets_id] && (bm->dayt > (bm->FISHERYprms[nf][buybackdate_id] + 7.0)) && (bm->FISHERYprms[nf][buybackdate_id]
					> (bm->dayt - 365.0))) {
				prop_buyback /= bm->FISHERYprms[nf][nsubfleets_id];
				bm->FISHERYprms[nf][nlicence_id] *= (1.0 - prop_buyback);
			}
		}
	}

	/* Move boats switching fisheries */
	for (nf = 0; nf < bm->K_num_fisheries; nf++) {
		if (bm->FISHERYprms[nf][flageffortmodel_id] > rec_econ_model) {
			if (do_debug_base && ((bm->which_fleet == nf) || (bm->which_fleet >= bm->K_num_fisheries))) {
				do_debug = 1;
			} else
				do_debug = 0;

			for (ns = 0; ns < bm->FISHERYprms[nf][nsubfleets_id]; ns++) {
				if (!boats_free[nf][ns]) {
					/* If none to swap skip it */
					continue;
				}

				/* If having flexible gear use distribute over more profitable methods */
				if (bm->flagsupp_allowed > 1) {
					/* Find best compatible subfleets in another fishery (based on vessel size) */
					for (fishery_id = 0; fishery_id < bm->K_num_fisheries; fishery_id++) {
						for (subf_id = 0; subf_id < bm->FISHERYprms[fishery_id][nsubfleets_id]; subf_id++) {
							if ((bm->SUBFLEET_ECONprms[nf][ns][boat_size_id] > bm->min_size_buffer * bm->SUBFLEET_ECONprms[fishery_id][subf_id][boat_size_id])
									&& (bm->SUBFLEET_ECONprms[nf][ns][boat_size_id] < bm->max_size_buffer
											* bm->SUBFLEET_ECONprms[fishery_id][subf_id][boat_size_id])) {
								/* Only allowed to swap with other dynamic fisheries */
								if (bm->FISHERYprms[fishery_id][flageffortmodel_id] > rec_econ_model) {
									rent_ratio = tot_marg_rent[fishery_id][subf_id] / (tot_marg_rent[nf][ns] - supp_cost + small_num);
									if (rent_ratio > 1.0) {
										p_flexgear = ((1.0 - 1.0 / (rent_ratio + small_num)) / (maxp_flexgear + small_num));
										boats_switching = floor(p_flexgear * boats_free[nf][ns] + 0.5); // 0.5 so rounding to nearest
										bm->SUBFLEET_ECONprms[fishery_id][subf_id][nboat_id] += boats_switching;
										bm->FISHERYprms[fishery_id][nvessel_id] += boats_switching;
										bm->SUBFLEET_ECONprms[fishery_id][subf_id][switchboat_id] += boats_switching;
									}
								}
							}
						}
					}

					/* If not enough better offers than stay where they are for now */
					fprintf(llogfp, "Time: %e, %s-%d had %d boats wishing to switch, but only %e found a location so stay where they are\n", bm->dayt,
							FisheryArray[nf].fisheryCode, ns, boats_free[nf][ns], boats_switching);
					boats_free[nf][ns] -= (int) (boats_switching);
					if (boats_free[nf][ns] < 0)
						boats_free[nf][ns] = 0;
					bm->SUBFLEET_ECONprms[nf][ns][nboat_id] += boats_free[nf][ns];
				} else {
					this_fishery = -1;
					this_subfleet = -1;

					/* First find best compatible subfleet in another fishery (based on vessel size) */
					max_marg_rent = 0;
					for (fishery_id = 0; fishery_id < bm->K_num_fisheries; fishery_id++) {
						for (subf_id = 0; subf_id < bm->FISHERYprms[fishery_id][nsubfleets_id]; subf_id++) {

							if ((bm->SUBFLEET_ECONprms[nf][ns][boat_size_id] > bm->min_size_buffer * bm->SUBFLEET_ECONprms[fishery_id][subf_id][boat_size_id])
									&& (bm->SUBFLEET_ECONprms[nf][ns][boat_size_id] < bm->max_size_buffer
											* bm->SUBFLEET_ECONprms[fishery_id][subf_id][boat_size_id])) {
								if (tot_marg_rent[fishery_id][subf_id] > max_marg_rent) {
									max_marg_rent = tot_marg_rent[fishery_id][subf_id];
									this_fishery = fishery_id;
									this_subfleet = subf_id;
								}
							}

						}
					}

					/* Otherwise switch boats to best alternative compatible fleet */
					if ((this_fishery > -1) && (this_subfleet > -1)) {
						bm->SUBFLEET_ECONprms[this_fishery][this_subfleet][nboat_id] += boats_free[nf][ns];
						bm->FISHERYprms[this_fishery][nvessel_id] += boats_free[nf][ns];
						bm->SUBFLEET_ECONprms[this_fishery][this_subfleet][switchboat_id] += boats_free[nf][ns];

						if (do_debug || boats_free[nf][ns]) {
							fprintf(llogfp, "Time: %e, %s-%d nboat: %e switchboat: %d\n", bm->dayt, FisheryArray[this_fishery].fisheryCode, this_subfleet,
									bm->SUBFLEET_ECONprms[this_fishery][this_subfleet][nboat_id], boats_free[nf][ns]);
						}

					} else if (boats_free[nf][ns] > 0) {
						/* If no better offer stay where they are for now */
						fprintf(llogfp, "Time: %e, %s-%d had %d boats wishing to switch, but no where for them to go (size: %e, marg_rent: %e)\n", bm->dayt,
								FisheryArray[nf].fisheryCode, ns, boats_free[nf][ns], bm->SUBFLEET_ECONprms[nf][ns][boat_size_id], max_marg_rent);
						bm->SUBFLEET_ECONprms[nf][ns][nboat_id] += boats_free[nf][ns];
					}
				}
			}
		}
	}

	/* Check for new boats - if new boats are allowed to enter */
    bm-> K_max_num_boats = 0;
	for (nf = 0; nf < bm->K_num_fisheries; nf++) {
		if (bm->FISHERYprms[nf][flageffortmodel_id] > rec_econ_model) {

			if (do_debug_base && ((bm->which_fleet == nf) || (bm->which_fleet >= bm->K_num_fisheries))) {
				do_debug = 1;
			} else
				do_debug = 0;

			tot_new = 0;
			overshot = 0;
			if (bm->flagnewboat) {
				if (bm->FISHERYprms[nf][nvessel_id] < bm->FISHERYprms[nf][nlicence_id]) {
					for (ns = 0; ns < bm->FISHERYprms[nf][nsubfleets_id]; ns++) {
						tot_new += boats_new[nf][ns];
					}
					if ((tot_new + bm->FISHERYprms[nf][nvessel_id]) > bm->FISHERYprms[nf][nlicence_id]) {
						overshot = (tot_new + bm->FISHERYprms[nf][nvessel_id]) - bm->FISHERYprms[nf][nlicence_id];
					}
					for (ns = 0; ns < bm->FISHERYprms[nf][nsubfleets_id]; ns++) {
						boats_new[nf][ns] -= (int) (ceil(overshot * boats_new[nf][ns] / (tot_new + small_num)));
						new_boats = boats_new[nf][ns];
						bm->FISHERYprms[nf][nvessel_id] += new_boats;
						bm->SUBFLEET_ECONprms[nf][ns][nboat_id] += new_boats;
						bm->SUBFLEET_ECONprms[nf][ns][newboat_id] += new_boats;

						if (do_debug || new_boats) {
							fprintf(llogfp, "Time: %e, %s-%d nboat: %e newboat: %d\n", bm->dayt, FisheryArray[nf].fisheryCode, ns,
									bm->SUBFLEET_ECONprms[nf][ns][nboat_id], new_boats);
						}
					}
				}
			}
		}

		/** In preparation for the next year, update effort BlackBook to reflect new number
		 of vessels - any new entrants are gifted the existing BlackBook distribution
		 and don't have to learn it for themselves. Thus can use simple rescaling.

		 Use > 10 test on month as faster than == 11 test.
		 **/
		if (bm->MofY > 10) {
			for (ns = 0; ns < bm->FISHERYprms[nf][nsubfleets_id]; ns++) {
				new_boats = (int) (bm->SUBFLEET_ECONprms[nf][ns][nboat_id]);
				old_boats = (int) (bm->SUBFLEET_ECONprms[nf][ns][nboat_id] - (bm->SUBFLEET_ECONprms[nf][ns][newboat_id]
						+ bm->SUBFLEET_ECONprms[nf][ns][switchboat_id]) + bm->SUBFLEET_ECONprms[nf][ns][lostboat_id]);
				this_subfleet = ns;
				do_rescale = 1;
				if (!old_boats) {
					if (new_boats > 0) {
						/* If new subfleet opening up default to distribution of 1st subfleet in the list,
						 if that doesn't exist fail (for now at least) */
						this_subfleet = 0;
						if (!ns)
							quit("New subfleet opening up but no distribution to call upon and no means of getting one so fail\n");
					} else {
						/* No rescaling necessary of new and old boat numbers are both less than 1 */
					}
					do_rescale = 0;
				}
				if ((new_boats < 1) && (old_boats > 0)) {
					/* So don't lose knowledge if fishery dwindles temporarily keep record as if 1 boat left,
					 otherwise if set all records to zero would lose it for good into the future given use
					 rescaling.
					 */
					new_boats = 1;
				}
				if (new_boats == old_boats)
					do_rescale = 0;

				/* Do the rescaling - if necessary */
				if (do_rescale) {
					for (month = 0; month < maxmonth; month++) {
						/* Rescale to maximum humanly possible - don't rescale the spatial effort
						 as that is already proportions for resistributing the bulk effort across the boxes */
						bm->EffortSchedule[nf][ns][month][hist_id] *= (new_boats / (old_boats + small_num));
					}
				}
			}
		}

		/** Update market weighting - to reflect which markets are being fed **/
		/* Initialise ports */
		for (porti = 0; porti < bm->K_num_ports; porti++) {
			portweight[porti] = 0;
		}

		/* Find new weighting */
		totportweight = 0;
		for (ns = 0; ns < bm->FISHERYprms[nf][nsubfleets_id]; ns++) {
			/* If no boats in the subfleet currently skip ahead */
			if (!bm->SUBFLEET_ECONprms[nf][ns][nboat_id]) {
				continue;
			}

			/* Find main home port of the subfleet and weight heavily for that */
			home_port = (int) (bm->SUBFLEET_ECONprms[nf][ns][home_port_id]);
			if (home_port < bm->K_num_ports) {
				portweight[home_port] += bm->SUBFLEET_ECONprms[nf][ns][nboat_id];
				totportweight += bm->SUBFLEET_ECONprms[nf][ns][nboat_id];
			}
		}

		/* Check of fishery uses the port and that the port is active */
		for (porti = 0; porti < bm->K_num_ports; porti++) {
			if (bm->Port_Fishery[nf][porti] && ((bm->dayt > bm->Port_info[porti][port_start_id]) && (bm->dayt < bm->Port_info[porti][port_end_id]))) {
				portweight[porti]++;
				totportweight++;
			}
		}

		/* Normalise and allocate weighting from ports to markets */
		if (bm->K_num_markets > 2) {
			quit("Code assumes only two markets\n");
		}
		totmarketwgt = 0;
		for (porti = 0; porti < bm->K_num_ports; porti++) {
			/* Find weight for market 0 as just assume weight for other market = 1 - value for market 0 */
			if (bm->Port_info[porti][prime_market_id])
				marketwgt = 1.0;  //Used to be 0
			else
				marketwgt = 0.0;  //Used to be 1
			totmarketwgt += marketwgt * (portweight[porti] / (totportweight + small_num));

			if (do_debug) {
				fprintf(llogfp, "Time: %e, %s porti: %d, totmarketwgt: %e, marketwgt: %e, portweight: %e, totportweight: %e\n", bm->dayt,
						FisheryArray[nf].fisheryCode, porti, totmarketwgt, marketwgt, portweight[porti], totportweight);
			}
		}

		for (sp = 0; sp < bm->K_num_tot_sp; sp++) {
			if (FunctGroupArray[sp].isFished == TRUE) {
				bm->SP_FISHERYprms[sp][nf][marketwgt_id] = totmarketwgt;

				if (do_debug) {
					fprintf(llogfp, "Time: %e, %s by %s marketwgt: %e\n", bm->dayt, FunctGroupArray[sp].groupCode, FisheryArray[nf].fisheryCode,
							bm->SP_FISHERYprms[sp][nf][marketwgt_id]);
				}
			}
		}
        
        // Check on max_num_boats
        boat_count = 0;
        if (bm->FISHERYprms[nf][flageffortmodel_id] > rec_econ_model) {
            for (ns = 0; ns < bm->FISHERYprms[nf][nsubfleets_id]; ns++) {
               boat_count += (int)bm->SUBFLEET_ECONprms[nf][ns][nboat_id];
            }
            if (boat_count > bm->K_max_num_boats )
                bm->K_max_num_boats = boat_count;
        }
	}
    
    // Sanity check on max_num_boats
    if(( bm->K_max_num_boats > orig_max_num_boats) && bm->flagStoreShotCPUE)
        quit ("K_max_num_boats has increased so will get an array over write in the CPUEgeneration generalise the code (so get realloc based on new num of boats) and redo\n");
    
    return;
}

/**
 *	\brief Annual Effort Allocation routine
 *
 */
void Annual_Quota_Allocation(MSEBoxModel *bm, FILE *llogfp) {

	int sp, nf, ns, flagrecfish, mth;
	double last_totQuota, last_owned_prop, owned_quota, totTACnew, totTACold, last_owned;

	if (verbose > 1)
		printf("Doing annual quota allocation\n");

	/* If no quotas to trade then skip ahead - Dan Holland US council model assumed NOT to be quota trading */
	if (!bm->MultiPlanEffort && bm->Council_needed)
		return;

	/* Initialise indices */
	for (nf = 0; nf < bm->K_num_fisheries; nf++) {
		for (ns = 0; ns < bm->FISHERYprms[nf][nsubfleets_id]; ns++) {
			bm->SUBFLEET_ECONprms[nf][ns][OldQuota_id] = 0;
		}
	}

	for (sp = 0; sp < bm->K_num_tot_sp; sp++) {
		if (FunctGroupArray[sp].isFished == TRUE) {

			if(!FunctGroupArray[sp].speciesParams[tac_resetnow_id])
				continue;

			/* Set some checks */
			tradeable_quota[sp] = 0;
			owned_quota = 0;
			totTACnew = 0;
			totTACold = 0;
			for (nf = 0; nf < bm->K_num_fisheries; nf++) {
				totTACnew += bm->TACamt[sp][nf][now_id];
				totTACold += bm->TACamt[sp][nf][old_id];
				for (ns = 0; ns < bm->FISHERYprms[nf][nsubfleets_id]; ns++) {
					if (!bm->stochastic_effort)
						tradeable_quota[sp] += bm->QuotaAlloc[nf][ns][sp][templease_id] + bm->QuotaAlloc[nf][ns][sp][leased_id];
					owned_quota += bm->QuotaAlloc[nf][ns][sp][owned_id];
				}
			}

			/* Find spare quota traded for each species */
			if (!bm->stochastic_effort) {
				if (totTACnew < totTACold) {
					if ((owned_quota + tradeable_quota[sp]) > totTACnew) {
						tradeable_quota[sp] = totTACnew - (owned_quota + tradeable_quota[sp]);
						if (tradeable_quota[sp] < 0)
							tradeable_quota[sp] = 0;
					}
				}
				if (bm->dayt < 1)
					tradeable_quota[sp] = MAXDOUBLE;
			}

			/* Now do quota allocation */
			owned_quota = 0.0; // Reinit owned quota so you can do a sanity check later
			for (nf = 0; nf < bm->K_num_fisheries; nf++) {
				flagrecfish = (int) (bm->FISHERYprms[nf][flagrecfish_id]);
				if (!flagrecfish) {
					/* Don't include recfishing in quota allocation for now (as often not quota-ed)
					 FIX - may have to change this if recfishing becomes quota allocated group
					 */
					last_totQuota = bm->TACamt[sp][nf][old_id];

					for (ns = 0; ns < bm->FISHERYprms[nf][nsubfleets_id]; ns++) {
						/* Zero catch sum */
						bm->QuotaAlloc[nf][ns][sp][oldcatch_id] = 0;

						/* Store Old quota */
						bm->QuotaAlloc[nf][ns][sp][oldquota_id] = bm->QuotaAlloc[nf][ns][sp][owned_id];
						bm->SUBFLEET_ECONprms[nf][ns][OldQuota_id] += bm->QuotaAlloc[nf][ns][sp][owned_id];

						/* Find old proportion */
						last_owned = bm->QuotaAlloc[nf][ns][sp][owned_id];

						if (last_totQuota)
							last_owned_prop = bm->QuotaAlloc[nf][ns][sp][owned_id] / (last_totQuota + small_num);
						else
							last_owned_prop = 0.0;

						/* Set new allocations - assume that permanent leases scale up and down
						 with owned quota (for now don't worry about fixed tonage leases, instead
						 assume they lease 2% of the sellers quota etc, which is itself a
						 percentage of the total TAC rather than fixed tonages.
						 */
						bm->QuotaAlloc[nf][ns][sp][owned_id] = last_owned_prop * bm->TACamt[sp][nf][now_id];

						/* Sum up owned quota for sanity check */
						owned_quota += bm->QuotaAlloc[nf][ns][sp][owned_id];

						if ((((bm->debug == debug_econeffort) || (bm->debug == debug_effort) || (bm->debug == debug_quota)) && ((bm->which_fleet == nf)
								|| (bm->which_fleet >= bm->K_num_fisheries))) && (bm->dayt > bm->checkstart)) {
							fprintf(
									llogfp,
									"Time: %e, %s-%d QuotaAlloc[%s][owned]: %e, TACamt: %e, last_owned_prop: %e, last_owned: %e, last_totQuota: %e, oldTACamt: %e\n",
									bm->dayt, FisheryArray[nf].fisheryCode, ns, FunctGroupArray[sp].groupCode, bm->QuotaAlloc[nf][ns][sp][owned_id],
									bm->TACamt[sp][nf][now_id], last_owned_prop, last_owned, last_totQuota, bm->TACamt[sp][nf][old_id]);
						}

						/* Reset temporary leases */
						bm->QuotaAlloc[nf][ns][sp][templease_id] = 0; // Temporarily leased out
						bm->QuotaAlloc[nf][ns][sp][leased_id] = 0; // Temporarily leased in
						bm->QuotaAlloc[nf][ns][sp][newleased_id] = 0;
						bm->QuotaAlloc[nf][ns][sp][oldleased_id] = 0;

						/* Find last year's total catch and reset it */
						for (mth = 0; mth < 12; mth++) {
							bm->QuotaAlloc[nf][ns][sp][oldcatch_id] += bm->BlackBook[nf][ns][sp][mth][current_id];
							bm->BlackBook[nf][ns][sp][mth][current_id] = 0;
						}
					}
				}
			}

			/* Do sanity checks on amount of quota owned so can't exceed actual total (cumulative) quota */
			if (owned_quota && (owned_quota > totTACnew)) {
				for (nf = 0; nf < bm->K_num_fisheries; nf++) {
					for (ns = 0; ns < bm->FISHERYprms[nf][nsubfleets_id]; ns++) {
						bm->QuotaAlloc[nf][ns][sp][owned_id] *= totTACnew / owned_quota;

						fprintf(llogfp, "Time %e %s by %s-%d owned now: %e (totTACnew: %e, owned_quota: %e)\n", bm->dayt, FunctGroupArray[sp].groupCode,
								FisheryArray[nf].fisheryCode, ns, bm->QuotaAlloc[nf][ns][sp][owned_id], totTACnew, owned_quota);
					}
				}
			}
		}
	}

	return;
}

/**
 *
 *	\brief Catch allocation routines (also cumulative effort updating
 *
 *	Distribute realised catch amongst subfleets
 */
void Allocate_Catch(MSEBoxModel *bm, FILE *llogfp) {
    int nf, ns, sp, b, ij, do_debug, do_debug_base, nreg;
   // int do_debug_quota = 0;
	double cpueval, totcatch, thiscatch, oldcpue, newcpue, target_scale, sp_catchkg, cumCatch, prop_olease, prop_tlease, ownQuota, leaseQuota, sp_avail,
			propstop, spareend, tottotcatch, totdiscard, totthisspcatch, thisspcatch, sp_bycatchkg, oldDISpue, newDISpue, thisspdiscard,
			totthisspdiscard, newVpue, oldVpue, totval;
	/*double oldspcatch;*/

	if (((bm->debug == debug_econeffort) || (bm->debug == debug_econ) || (bm->debug == debug_quota)) && (bm->dayt > bm->checkstart)) {
		do_debug_base = 1;
	} else
		do_debug_base = 0;

    /*
	if ((bm->debug == debug_quota) && (bm->dayt > bm->checkstart)) {
		do_debug_quota = 1;
	} else
		do_debug_quota = 0;
     */
    
	if (verbose > 1) {
		printf("Allocating catch to subfleets\n");
		fprintf(llogfp, "Time: %e allocating catch to subfleets\n", bm->dayt);
	}

	totval = 0;
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

				tottotcatch = 0;
				totdiscard = 0;
				/* Allocate catch from fleet amongst subfleets */
				for (b = 0; b < bm->nbox; b++) {
					totcatch = 0;
			
					/*
					 if(sp == FPO_id)
					 fprintf(llogfp,"Time: %e, %s box-%d today_effort: %e\n",
					 bm->dayt, FisheryArray[nf].fisheryCode, b, bm->Effort_hdistrib[b][nf][today_effort]);
					 */

					if (bm->Effort_hdistrib[b][nf][today_effort]) {
						for (sp = 0; sp < bm->K_num_tot_sp; sp++) {
							if (FunctGroupArray[sp].isFished == TRUE) {
								/* If can't catch the group skip ahead */
								if (!bm->SP_FISHERYprms[sp][nf][q_id])
									continue;

								totthisspcatch = 0;
								totthisspdiscard = 0;

								target_scale = 1.0;
								if (bm->FISHERYtarget[nf][sp])
									target_scale = bm->targetscale;

								sp_bycatchkg = bm->Discards[b][sp][nf] * bm->X_CN * mg_2_kg;

								for (ij = 0; ij < bm->boxes[b].nz; ij++) {
									nreg = bm->regID[ij];
									if ((bm->Catch[b][sp][nf][ij]) || ((bm->flagTACincludeDiscard) && (!ij) && (bm->Discards[b][sp][nf]))) {
										/*oldspcatch = totthisspcatch;*/

										/* Convert from mg N to kg wet weight */
										sp_catchkg = bm->Catch[b][sp][nf][ij] * bm->X_CN * mg_2_kg;

										/* Get relative contribution due to this subfleet (the SpatialBlackBook current
										 entry here is the normalised value so that over the sum of the boxes and
										 subfleets get total value without double counting).

										 As catch stored at per-box level then bm->SpatialBlackBook is relative
										 contribution by each subfleet.
										 */
										thisspcatch = sp_catchkg * bm->SpatialBlackBook[nf][ns][bm->MofY][b][current_id];
										thiscatch = target_scale * thisspcatch;
										totthisspcatch += thisspcatch;
										totcatch += thiscatch;
										tottotcatch += thisspcatch;
										RegCatch[nf][ns][nreg][sp] += thisspcatch;
										totval += thiscatch * bm->SP_FISHERYprms[sp][nf][saleprice_id];

										/* If appropriate add in discards (only do it in layer 0 as don't
										 have layer information, but know layer 0 will always happen
										 */
										thisspdiscard = 0;
										if (!ij) {
											if (bm->flagTACincludeDiscard) {
												thisspdiscard = bm->X_CN * mg_2_kg * bm->Discards[b][sp][nf];
												RegCatch[nf][ns][nreg][sp] += thisspdiscard;
												totthisspdiscard += thisspdiscard;
											}
											totdiscard += sp_bycatchkg * FunctGroupArray[sp].speciesParams[sp_concern_id];
										}

										/**
										 if(do_debug)
										 fprintf(llogfp,"Time: %e, %s-%d month: %d box%d-%d, totthisspcatch: %e (%e), thisspcatch: %e, sp_catchkg: %e SpatialBlackBook[current_id]: %e (%s catch: %e, XCN: %e, mg2kg: %e, target_scale: %e, tottotcatch: %e)\n",
										 bm->dayt, FisheryArray[nf].fisheryCode, ns, bm->MofY, b, ij, totthisspcatch, oldspcatch, thisspcatch, sp_catchkg, bm->SpatialBlackBook[nf][ns][bm->MofY][b][current_id], FunctGroupArray[sp].groupCode, bm->Catch[b][sp][nf][ij], bm->X_CN, (double)mg_2_kg, target_scale, tottotcatch);
										 **/
									}
									/**
									 if(do_debug)
									 fprintf(llogfp,"Time: %e, %s-%d month: %d box%d-%d, RegCatch-%d: %e, thisspcatch: %e, , thisspdiscard: %e, totthisspcatch: %e (%e), sp_catchkg: %e SpatialBlackBook[current_id]: %e (%s catch: %e, XCN: %e, mg2kg: %e, target_scale: %e, tottotcatch: %e)\n",
									 bm->dayt, FisheryArray[nf].fisheryCode, ns, bm->MofY, b, ij, nreg, RegCatch[nf][ns][nreg][sp], thisspcatch, thisspdiscard, totthisspcatch, oldspcatch, sp_catchkg, bm->SpatialBlackBook[nf][ns][bm->MofY][b][current_id], bm->spNAME[nid], bm->Catch[b][sp][nf][ij], bm->X_CN, mg_2_kg, target_scale, tottotcatch);
									 **/
								}
								bm->SUBFLEET_ECONprms[nf][ns][CurrentCatch_id] += totthisspcatch;
								bm->QuotaAlloc[nf][ns][sp][boatcatch_id] += totthisspcatch; // Landed catch only as used to calculate cash flow for the boat
								bm->QuotaAlloc[nf][ns][sp][cumboatcatch_id] += totthisspcatch + totthisspdiscard;   // Catch and discards as compared against quota
								bm->QuotaAlloc[nf][ns][sp][cummonthcatch_id] += totthisspcatch; // Landed catch only as used to calculate revenue
								bm->BlackBook[nf][ns][sp][bm->MofY][current_id] += totthisspcatch;

								/* Update bycatch information too - only store it for species of concern however */
								bm->QuotaAlloc[nf][ns][sp][cummonthbycatch_id] += sp_bycatchkg * bm->SpatialBlackBook[nf][ns][bm->MofY][b][current_id]
										* FunctGroupArray[sp].speciesParams[sp_concern_id];

								/**
								 if((nf==bm->which_fleet) && (sp == FDB_id)){
								 fprintf(llogfp,"Time: %e, %s-%d month: %d totthisspcatch: %e cummonthcatch: %e\n",
								 bm->dayt,  FisheryArray[nf].fisheryCode, ns, bm->MofY, totthisspcatch, bm->QuotaAlloc[nf][ns][sp][cummonthcatch_id]);
								 }

								 if((nf==bm->which_fleet) && (sp == FDB_id) && (do_debug)){
								 fprintf(llogfp,"Time: %e, %s-%d month: %d cummonthbycatch: %e, sp_bycatchkg: %e, SpatialBlackBook: %e, spconcern: %e (Discards: %e)\n",
								 bm->dayt,  FisheryArray[nf].fisheryCode, ns, bm->MofY, bm->QuotaAlloc[nf][ns][sp][cummonthbycatch_id], sp_bycatchkg, bm->SpatialBlackBook[nf][ns][bm->MofY][b][current_id], bm->SP_prms[sp][spconcern_id], bm->Discards[b][nid][nf] * bm->X_CN * mg_2_kg);
								 }
								 **/
							}
						}
					}
					/* Store effort estimate for cpue calcs */
					bm->SUBFLEET_ECONprms[nf][ns][CurrentEffort_id] += bm->Effort_hdistrib[b][nf][today_effort];

					/* Update spatial CPUE - including effort penalty if need be for days-at-sea incentive schemes
					 (set penalty = 1.0 if not using it
					 */
					oldcpue = bm->SpatialCPUE[nf][ns][bm->MofY][b];
					newcpue = totcatch / (bm->Effort_hdistrib[b][nf][today_effort] * bm->EffortPenalty[b][nf] + small_num);
					bm->SpatialCPUE[nf][ns][bm->MofY][b] = bm->SUBFLEET_ECONprms[nf][ns][flexweight_id] * (newcpue - oldcpue) + oldcpue;

					/* Update spatial Discards */
					oldDISpue = bm->SpatialDisPUE[nf][ns][bm->MofY][b];
					newDISpue = totdiscard / (bm->Effort_hdistrib[b][nf][today_effort] + small_num);
					bm->SpatialDisPUE[nf][ns][bm->MofY][b] = bm->SUBFLEET_ECONprms[nf][ns][flexweight_id] * (newDISpue - oldDISpue) + oldDISpue;

					/* Update spatial Value Per Unit Effort - for use with deemed value behavioural changes */
					oldVpue = bm->SpatialVPUE[nf][ns][bm->MofY][b];
					newVpue = totval / (bm->Effort_hdistrib[b][nf][today_effort] + small_num);

					bm->SpatialVPUE[nf][ns][bm->MofY][b] = bm->SUBFLEET_ECONprms[nf][ns][flexweight_id] * (newVpue - oldVpue) + oldVpue;

				}
				/* Allocate catch from fleet amongst subfleets */
				for (sp = 0; sp < bm->K_num_tot_sp; sp++) {
					if (FunctGroupArray[sp].isFished == TRUE) {

						if (!bm->SP_FISHERYprms[sp][nf][q_id])
							continue;

						/* Check catch vs holdings */
						cumCatch = bm->QuotaAlloc[nf][ns][sp][cumboatcatch_id];
						prop_olease = bm->QuotaAlloc[nf][ns][sp][permolease_id];
						prop_tlease = bm->QuotaAlloc[nf][ns][sp][templease_id];
						ownQuota = (1.0 - prop_olease - prop_tlease) * bm->QuotaAlloc[nf][ns][sp][owned_id];
						leaseQuota = bm->QuotaAlloc[nf][ns][sp][leased_id];
						sp_avail = ownQuota + leaseQuota - cumCatch;

						/* Check whether demand still exists for this - with the
						 threshold of "if we're this close we'd better get extra quota"
						 shifting higher as the year progresses so don't buy extra late
						 in year that then don't use it, so left with a surplus */
						propstop = bm->prop_within + (1.0 - bm->prop_within) * (bm->MofY / 12.0);
						if (sp_avail < (1.0 - propstop) * (ownQuota + leaseQuota)) {
							bm->QuotaAlloc[nf][ns][sp][within_id] = 1;
							bm->SUBFLEET_ECONprms[nf][ns][within_id] = 1;
						} else
							bm->QuotaAlloc[nf][ns][sp][within_id] = 0;

						/* Check whether this still has surplus to shift - with the
						 threshold of "beyond this point hold onto quota" shifting
						 lower as the year progresses to minimise surplus
						 */
						spareend = bm->prop_spareend + (((1.0 / bm->prop_spareend) - 1.0) * bm->prop_spareend - 1.0) * (bm->MofY / 12.0);
						if (sp_avail > spareend * (ownQuota + leaseQuota))
							bm->QuotaAlloc[nf][ns][sp][spare2sell_id] = 1;
						else
							bm->QuotaAlloc[nf][ns][sp][spare2sell_id] = 0;

                        /*
						if (do_debug_quota) {
							fprintf(
									llogfp,
									"Time: %e, %s-%d for %s sp_avail: %e, propstop: %e, testprop: %e, within: %e, spareend: %e, sparetest: %e, spare2sell: %e (M/12: %e, ownQuota: %e, leaseQuota:%e, cumCatch: %e)\n",
									bm->dayt, FisheryArray[nf].fisheryCode, ns, FunctGroupArray[sp].groupCode, sp_avail, propstop, ((1.0 - propstop)
											* (ownQuota + leaseQuota)), bm->QuotaAlloc[nf][ns][sp][within_id], spareend, (spareend * (ownQuota + leaseQuota)),
									bm->QuotaAlloc[nf][ns][sp][spare2sell_id], (double)(bm->MofY / 12.0), ownQuota, leaseQuota, cumCatch);
                         }
                         */

					}
				}
				bm->SUBFLEET_ECONprms[nf][ns][discards_ind_id] += totdiscard * (bm->SUBFLEET_ECONprms[nf][ns][CurrentCatch_id] / (tottotcatch + small_num));

				/* If a new month update values */
				if (bm->newmonth) {
					/* Update cumulative realised effort expenditure */
					bm->SUBFLEET_ECONprms[nf][ns][AnnualEffort_id] += bm->SUBFLEET_ECONprms[nf][ns][CurrentEffort_id];

					/* Update expected historical effort */
					if (bm->EffortSchedule[nf][ns][bm->lastMofY][expect_id] > 0) {
						bm->EffortSchedule[nf][ns][bm->lastMofY][hist_id] = (bm->EffortSchedule[nf][ns][bm->lastMofY][hist_id]
								* bm->SUBFLEET_ECONprms[nf][ns][effortDiscount_id] + bm->EffortSchedule[nf][ns][bm->lastMofY][expect_id]) / (1
								+ bm->SUBFLEET_ECONprms[nf][ns][effortDiscount_id]);

					}

					/* Update expected catch, discards and CPUE */
					cpueval = 0;

					if (bm->SUBFLEET_ECONprms[nf][ns][CurrentCatch_id] > 0) {
						for (sp = 0; sp < bm->K_num_tot_sp; sp++) {
							if (FunctGroupArray[sp].isFished == TRUE) {
								/* If can't catch the group skip ahead */
								if (!bm->SP_FISHERYprms[sp][nf][q_id]){
									continue;
								}

								/**/
								if(do_debug){
									fprintf(llogfp, "Time: %e, %s-%d %s BlackBookExpect: %e effDis: %e, BoatCatch: %e\n", bm->dayt,
											FisheryArray[nf].fisheryCode, ns, FunctGroupArray[sp].groupCode,
											bm->BlackBook[nf][ns][sp][bm->lastMofY][expect_id], bm->SUBFLEET_ECONprms[nf][ns][effortDiscount_id],
											bm->QuotaAlloc[nf][ns][sp][boatcatch_id]);
								}
								/**/
								bm->BlackBook[nf][ns][sp][bm->lastMofY][expect_id] = bm->SUBFLEET_ECONprms[nf][ns][effortDiscount_id]
										* (bm->QuotaAlloc[nf][ns][sp][cummonthcatch_id] - bm->BlackBook[nf][ns][sp][bm->lastMofY][expect_id])
										+ bm->BlackBook[nf][ns][sp][bm->lastMofY][expect_id];
								cpueval += bm->BlackBook[nf][ns][sp][bm->lastMofY][expect_id] / bm->EffortSchedule[nf][ns][bm->lastMofY][hist_id];

								/* For species of concern update bycatch in the BlackBook */
								bm->BlackBook[nf][ns][sp][bm->lastMofY][bycatch_id] = (bm->SUBFLEET_ECONprms[nf][ns][bycatchDiscount_id]
										* (bm->QuotaAlloc[nf][ns][sp][cummonthbycatch_id] - bm->BlackBook[nf][ns][sp][bm->lastMofY][bycatch_id])
										+ bm->BlackBook[nf][ns][sp][bm->lastMofY][bycatch_id]) * FunctGroupArray[sp].speciesParams[sp_concern_id];

								if (do_debug) {
									fprintf(llogfp, "Time: %e, %s-%d %s mth: %d, BlackBookBycatch: %e, bycatchDis: %e, cumBycatch: %e, spconcern: %e\n",
											bm->dayt, FisheryArray[nf].fisheryCode, ns, FunctGroupArray[sp].groupCode, bm->lastMofY,
											bm->BlackBook[nf][ns][sp][bm->lastMofY][bycatch_id], bm->SUBFLEET_ECONprms[nf][ns][bycatchDiscount_id],
											bm->QuotaAlloc[nf][ns][sp][cummonthbycatch_id], FunctGroupArray[sp].speciesParams[sp_concern_id]);
								}

								/* Also update the spatial allocation of effort - not that any incentives-based
								 avoidance of bycatch will already be reflected in the current spatial
								 distribution and need not be explicitly included again here (would be quasi double
								 counting) */
								for (b = 0; b < bm->nbox; b++) {
									bm->SpatialBlackBook[nf][ns][bm->lastMofY][b][updated_id] = bm->SUBFLEET_ECONprms[nf][ns][effortDiscount_id]
											* (bm->SpatialBlackBook[nf][ns][bm->lastMofY][b][current_id]
													- bm->SpatialBlackBook[nf][ns][bm->lastMofY][b][updated_id])
											+ bm->SpatialBlackBook[nf][ns][bm->lastMofY][b][updated_id];
								}
							}
						}
					}

					bm->SUBFLEET_ECONprms[nf][ns][ExpectCPUE_id] = cpueval;

					/* Initialise bimonthly accumulated catch if a new bimonthly period */
					if (bm->LastBiM != bm->BiM) {
						for (sp = 0; sp < bm->K_num_tot_sp; sp++) {
							if (FunctGroupArray[sp].isFished == TRUE) {
								bm->TotCumBiCatch[sp][nf] = 0;
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
 *
 *	\brief Routine calculating how much of each group each fleet may catch
 *
 */
void Consolidate_Allowed_Catches(MSEBoxModel *bm, FILE *llogfp) {
	int nf, ns, sp, do_debug, do_debug_base, fdv;
	double totallowedcatch, quotabit = 0, prop_olease, prop_tlease;
	double totCumCatch = 0;

	if (verbose > 1)
		printf("Calculating allowed catches\n");

	if (((bm->debug == debug_econeffort) || (bm->debug == debug_econ) || (bm->debug == debug_quota)) && (bm->dayt > bm->checkstart)) {
		do_debug_base = 1;
	} else
		do_debug_base = 0;

	for (sp = 0; sp < bm->K_num_tot_sp; sp++) {
		if (FunctGroupArray[sp].isFished == TRUE) {
			for (nf = 0; nf < bm->K_num_fisheries; nf++) {
				if (do_debug_base && ((bm->which_fleet == nf) || (bm->which_fleet >= bm->K_num_fisheries))) {
					do_debug = 1;
				} else
					do_debug = 0;

				if (bm->FISHERYprms[nf][flageffortmodel_id] > rec_econ_model) {
					/* If can't catch the group skip ahead */
					if (!bm->SP_FISHERYprms[sp][nf][q_id])
						continue;
					totallowedcatch = 0;
					if (bm->fish_withoutQ) {
						totallowedcatch = no_quota;
					} else {
						for (ns = 0; ns < bm->FISHERYprms[nf][nsubfleets_id]; ns++) {
							/* If no boats in the subfleet currently skip ahead */
							if (!bm->SUBFLEET_ECONprms[nf][ns][nboat_id]) {
								continue;
							}

							/* Total per group */
							prop_olease = bm->QuotaAlloc[nf][ns][sp][permolease_id];
							prop_tlease = bm->QuotaAlloc[nf][ns][sp][templease_id];
							bm->QuotaAlloc[nf][ns][sp][totalallowed_id] = (1.0 - prop_olease - prop_tlease) * bm->QuotaAlloc[nf][ns][sp][owned_id]
									+ bm->QuotaAlloc[nf][ns][sp][leased_id];

							/* Allocating to fleet */
							totallowedcatch += bm->QuotaAlloc[nf][ns][sp][totalallowed_id];
						}

						/* Converting into mg N - as this is what catches are stored in */
						totallowedcatch *= (kg_2_mg / bm->X_CN);
						quotabit = totallowedcatch;

						/* Accounting for catch already taken */
						//totallowedcatch -= bm->TotCumCatch[sp][nf][bm->thisyear];
                        totCumCatch = Harvest_Get_TotCumCatch(sp, nf, bm->thisyear);
						totallowedcatch -= (totCumCatch + bm->TotOldCumCatch[sp][nf]);
						
						if (totallowedcatch < 0.0)
							totallowedcatch = 0.0;
					}
					bm->SP_FISHERYprms[sp][nf][totquota_remain] = totallowedcatch * bm->X_CN * mg_2_kg; /* Convert back so in comparable units to Quota for reporting in output */

					/* If deemed values in use then ignore limits as will land it for deemed value instead potentially - unless high grade */
					if (bm->dayt > bm->DVstart) {
						fdv = (int) (bm->FISHERYprms[nf][flagDV_id]);
						switch (fdv) {
						case noDV_id: /* No use of deemed values so do nothing */
							break;
						case simpleDV_id: /* Deemed value is simple proportion of price, using constant proportions per species per fishery */
						case complexDV_id: /* Deemed value is a proportion estimated explicitly from costs of landing, price of stock, crew share. */
							totallowedcatch = no_quota * bm->X_CN * mg_2_kg;
							break;
						default:
							quit("No such deemed value option, they must be between (none = %d and compound = %d - reset for %s)\n", noDV_id, complexDV_id,
									FisheryArray[nf].fisheryCode);
							break;
						}
					}

					bm->SP_FISHERYprms[sp][nf][catch_allowed] = totallowedcatch;
					if (bm->SP_FISHERYprms[sp][nf][catch_allowed] > bm->SP_FISHERYprms[sp][nf][trip_lim_id])
						bm->SP_FISHERYprms[sp][nf][catch_allowed] = bm->SP_FISHERYprms[sp][nf][trip_lim_id];

					/* Reset TripCatch for the new trip */
					bm->TripCatch[sp][nf] = 0.0;

					/* */
					 if(do_debug){
						 fprintf(llogfp, "Time: %e, fishery: %s, sp: %s, totallowedcatch: %e (orig: %e), TotCumCatch: %e, catch_allowed: %e, trip_lim: %e (tripcatch: %e)\n",
								 bm->dayt, FisheryArray[nf].fisheryCode, FunctGroupArray[sp].groupCode, totallowedcatch, quotabit, totCumCatch, bm->SP_FISHERYprms[sp][nf][catch_allowed], bm->SP_FISHERYprms[sp][nf][trip_lim_id], bm->TripCatch[sp][nf]);
					 }
					/* */
				}
			}
		}

		/** Also turn taxes on if appropriate **/
		if (!bm->TaxDone) {
			if (bm->dayt > bm->TaxStart) {
				for (nf = 0; nf < bm->K_num_fisheries; nf++) {
					bm->SP_FISHERYprms[sp][nf][tax_id] = bm->SP_FISHERYprms[sp][nf][origtax_id];
				}
			}
		}
	}

	if ((!bm->TaxDone) && (bm->dayt > bm->TaxStart)) {
		bm->TaxDone = 1;
	}

	return;
}
