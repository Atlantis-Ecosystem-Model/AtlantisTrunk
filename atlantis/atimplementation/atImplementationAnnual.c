/**
 *  \ingroup atImplementationLib
 * 	\file atAnnualImplementation.c
 *  \brief Management Implementation functions that are performed annually.
 * 	\author Beth Fulton 25/06/2009
 *
 * <b>Revisions</b>
 *
 * 25-06-2009 Bec Gorton
 * Split the management module into management, harvest and management implementation.
 *
 * 28-10-2009 Bec Gorton
 * Removed references to the K_num_fished_sp and K_num_impacted_sp. Arrays that were this
 * size have been resized to K_num_tot_sp.
 *
 * 02-11-2009 Bec Gorton
 * Removed more references to K_num_fished_sp and K_num_impacted_sp.
 *
 * 03-11-2009 Bec Gorton
 * Fixed a stuffed reference to the TACChange array - this has been moved into the harvestModule structure.
 *
 * 6/4/2010 Beth Fulton
 * Caught a [nf] array cell reference when should have been [fishery_id] in
 * Apply_Annual_Fisheries_Mgmt(. Removed extraneous loops over nf too
 * (as major loop is based on fishery_id and [nf] not actually used anywhere.
 * <NB: Maybe should change fishery_id to nf for consistency>
 *
 */

/*  Open library routines */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "atImplementation.h"
#include <atManageLib.h>

static void Apply_Annual_Fisheries_Mgmt(MSEBoxModel *bm, FILE *llogfp);

/**
 *	\brief Annual management implementation model
 */
void Annual_Mgmt_Implementation(MSEBoxModel *bm, FILE *llogfp) {

	Apply_Annual_Fisheries_Mgmt(bm, llogfp);
}

/**
 *	\brief Apply annual management decisions to get final form of management for the coming year of
 *	the model - concerned with setting TACS, dates for seasonal closures
 *	(currently only one closure per year implemented) and spatial closures for current year
 *
 *	Any management associated with perfect knowledge case is calculated and implemented in here
 *	(rather than having calculation steps in Make_Mgmt_Decisions() routine above
 */
void Apply_Annual_Fisheries_Mgmt(MSEBoxModel *bm, FILE *llogfp) {
	int fishery_id, flagADJUSTmanage, trigger_tripped, sp, nstock, nreg, ij, k, end_trigger_tripped, flagTACchange, flagSEASONchange,
			fishstartday, fishendday, season_length, new_length, high_trigger_tripped, high_end_trigger_tripped, chrt, TAC_num_changes, n, sn, rn, den, pid,
			pseudo_survey_trip, in_quota, guild;

	double FC_restrict, FC_period, FC_period2, EFF_scale, FC_thresh, startpop, fishnowpop, step1, TACorig, TAC_scale, SEASON_scale, tempendday, newfishendday,
			FC_high_thresh, regtot;
	double biomass;

	if (verbose)
		printf("Annual fisheries management\n");

	/* Apply management decisions */
	for (fishery_id = 0; fishery_id < bm->K_num_fisheries; fishery_id++) {
		/* Only change management in fisheries where management active */
		if (bm->FISHERYprms[fishery_id][manage_on_id] < 1)
			continue;

		flagADJUSTmanage = (int) (bm->FISHERYprms[fishery_id][flagADJUSTmanage_id]);

		/* If using stock based management, determine if any group trips the trigger */
		FC_restrict = bm->FISHERYprms[fishery_id][FC_restrict_id];
		FC_period = bm->FISHERYprms[fishery_id][FC_period_id] + small_num;
		FC_period2 = bm->FISHERYprms[fishery_id][FC_period2_id];
		EFF_scale = 1.0;
		high_trigger_tripped = 1;
		if (flagADJUSTmanage > 0) {
			trigger_tripped = 0;
			for (sp = 0; sp < bm->K_num_tot_sp; sp++) {
				if (FunctGroupArray[sp].isFished == TRUE) {
					FC_thresh = bm->SP_FISHERYprms[sp][fishery_id][FC_thresh_id];
					FC_high_thresh = bm->SP_FISHERYprms[sp][fishery_id][FC_high_thresh_id];
					startpop = bm->totinitpop[sp];
					fishnowpop = bm->totfishpop[sp];

					/* Using original simple adaptive management with perfect knowledge model
					 (explicit TAC model handed in Make_Mgmt_Decisions())
					 */
					if (!bm->do_assessing) {
						if (fishnowpop < FC_thresh * startpop) {
							trigger_tripped = 1;
							fprintf(llogfp, "Time: %e, species %s (targeted by fishery %s) is at critical population levels (%.2f)\n", bm->dayt,
									FunctGroupArray[sp].groupCode, FisheryArray[fishery_id].fisheryCode, fishnowpop / startpop);

							break;
						} else if (fishnowpop < FC_high_thresh * startpop) {
							high_trigger_tripped = 0;
							break;
						} else {
							if (verbose > 1 && bm->debug < debug_biology_process)
								fprintf(llogfp, "Time: %e, species %s population (targeted by fishery %s) is larger than upper reference level (%.2f)\n",
										bm->dayt, FunctGroupArray[sp].groupCode, FisheryArray[fishery_id].fisheryCode, fishnowpop / startpop);
						}
					} else {
						/* Use assessment model output */
						if (bm->TAC_trigger[fishery_id][triggered_scalar_id] < 1.0) {
							trigger_tripped = 1;
						} else if (bm->TAC_trigger[fishery_id][triggered_scalar_id] > 1.0) {
							/* Nothing to do here */
						} else {
							high_trigger_tripped = 0;
						}
					}
					/*
					 if((sp == ZLcat_id) && (fishery_id == midwcZL_id)){
					 fprintf(llogfp, "time: %e, species: %s, trigger_tripped: %d, startpop: %e, fishnowpop: %e, FC_thresh: %e\n",
					 bm->dayt, FunctGroupArray[nid].groupCode, trigger_tripped, startpop, fishnowpop, FC_thresh);
					 }
					 */
				}
			}

			/* If trigger tripped by low reference value, there is a reduction in effort,
			 but that reduction is re-evaluated after the period FC_period2. So the
			 trigger system is reset, if the system is still poor the trigger will go off again */
			if (trigger_tripped) {
				/* Set the management stability management index */
				Manage_Set_Manage_Index(bm, fishery_id, mgmtstability_id, 0);

				if ((bm->TAC_trigger[fishery_id][target_day_id] > 0.0) && ((bm->dayt > (bm->TAC_trigger[fishery_id][target_day_id] + FC_period2))
						|| (bm->TAC_trigger[fishery_id][triggered_scalar_id] != 1.0)))
					bm->TAC_trigger[fishery_id][target_day_id] = 0.0;
				if (bm->TAC_trigger[fishery_id][target_day_id] == 0.0)
					bm->TAC_trigger[fishery_id][target_day_id] = bm->dayt;

				if (!bm->do_assessing) {
					/* Using original simple adaptive management with perfect knowledge model */
					if (bm->dayt < (bm->TAC_trigger[fishery_id][target_day_id] + FC_period))
						step1 = (bm->dayt + 1.0 - bm->TAC_trigger[fishery_id][target_day_id]) / FC_period;
					else
						step1 = 1.0;
					EFF_scale = 1.0 - (step1 * (1 - FC_restrict));
				} else
					/* Use assessment model output */
					EFF_scale = bm->TAC_trigger[fishery_id][triggered_scalar_id];

				/*
				 if(fishery_id == midwcZL_id){
				 fprintf(llogfp,"%e %s bm->TAC_trigger[target_day_i]: %e (trigger_tripped: %d), step1: %e, FC_restrict: %e, EFF_scale: %e\n",
				 bm->dayt, FisheryArray[fishery_id].fisheryCode, bm->TAC_trigger[fishery_id][target_day_id], trigger_tripped, step1, FC_restrict, EFF_scale);
				 }
				 */
			} else {
				bm->TAC_trigger[fishery_id][target_day_id] = 0.0;
				EFF_scale = 1.0;
			}
		}

		if ((high_trigger_tripped) && (!bm->do_assessing)) {
			/* If in recovery mode let it rebuild */
			step1 = (bm->dayt + 1.0 - bm->TAC_trigger[fishery_id][target_day_id]) / FC_period;
			EFF_scale = 1.0 - (step1 * (1 + FC_restrict));
		}

		/* If have stock-based management (flagADJUSTmanage > 0) then this EFF_scale may have a value other than 1.0
		 Note that it is stored in effort_scale[fishery_id][target_tac_id] in all cases (whether adjusting
		 gear, season, spatial management, effort or TACs). For gear and season this is applied below. For
		 spatial management it is done in Check_For_Active_MPA. For effort it is applied in Effort_Restrict_check().
		 And for TACs it is applied below at TAC_scale
		 */
		effort_scale[fishery_id][target_tac_id] = EFF_scale;

		/* Set flag so management action recorded */
		if (effort_scale[fishery_id][target_tac_id] != 1.0)
			effort_scale[fishery_id][target_message_id] = 1;

		//fprintf(llogfp, "time: %e, effort_scale[%d][0]: %e\n",
		//	bm->dayt, fishery_id, effort_scale[fishery_id][target_tac_id]);

		/* If using stock management that is sensitive to the population levels of
		 endangered groups, determine if any group trips the trigger */
		FC_restrict = bm->FISHERYprms[fishery_id][FC_restrict_endangered_id];
		FC_period = bm->FISHERYprms[fishery_id][FC_endanger_period_id] + small_num;
		FC_period2 = bm->FISHERYprms[fishery_id][FC_period2_id];
		EFF_scale = 1.0;
		high_end_trigger_tripped = 1;
		pseudo_survey_trip = 0;

		if (bm->flagendangered) {
			end_trigger_tripped = 0;
			for (sp = 0; sp < bm->K_num_tot_sp; sp++) {
				if ((FunctGroupArray[sp].isImpacted == TRUE) || ((FunctGroupArray[sp].isImpacted == FALSE) && (FunctGroupArray[sp].speciesParams[assess_flag_id] < 0))){
					if (FunctGroupArray[sp].speciesParams[sp_concern_id]) {
						FC_thresh = bm->SP_FISHERYprms[sp][fishery_id][FC_thresh_id];
						FC_high_thresh = bm->SP_FISHERYprms[sp][fishery_id][FC_high_thresh_id];
						startpop = bm->totinitpop[sp];
						fishnowpop = bm->totfishpop[sp];

						/* Using original simple adaptive management with perfect knowledge model */
                        if (fishnowpop < FC_thresh * startpop) {
                            end_trigger_tripped = 1;
                            fprintf(llogfp, "Time: %e, endangered species %s (impacted by fishery %s) is at critical population levels (%.2f)\n", bm->dayt,
                                    FunctGroupArray[sp].groupCode, FisheryArray[fishery_id].fisheryCode, fishnowpop / startpop);
                            break;
                        } else if (fishnowpop < FC_high_thresh * startpop) {
                            high_end_trigger_tripped = 0;
                            break;
                        } else {
                            if (verbose > 1)
                                fprintf(
                                        llogfp,
                                        "Time: %e, endangered species %s population (impacted by fishery %s) is larger than upper reference level (%.2f)\n",
                                        bm->dayt, FunctGroupArray[sp].groupCode, FisheryArray[fishery_id].fisheryCode, fishnowpop / startpop);
                        }
                        pseudo_survey_trip = 1;
                    } else {
                        /* Use assessment model output */
                        if (bm->TAC_trigger[fishery_id][triggered_scalar_id] < 1.0 && bm->TAC_trigger[fishery_id][endanger_is_trigger_id]) {
                            end_trigger_tripped = 1;
                            fprintf(
                                    llogfp,
                                    "Time: %e, endangered species %s (impacted by fishery %s) is at critical population levels so fishery effort impacted (%.2f)\n",
                                    bm->dayt, FunctGroupArray[sp].groupCode, FisheryArray[fishery_id].fisheryCode, fishnowpop / startpop);
                            break;
                        } else if (bm->TAC_trigger[fishery_id][triggered_scalar_id] > 1.0 && bm->TAC_trigger[fishery_id][endanger_is_trigger_id]) {
                            if (verbose > 1)
                                fprintf(llogfp, "Time: %e, endangered species %s population (impacted by fishery %s) is not endangered any more (%.2f)\n",
                                        bm->dayt, FunctGroupArray[sp].groupCode, FisheryArray[fishery_id].fisheryCode, fishnowpop / startpop);
                        } else {
                            high_end_trigger_tripped = 0;
                        }
                    }
				}
			}
			if (end_trigger_tripped) {
				Manage_Set_Manage_Index(bm, fishery_id, mgmtstability_id, 0);

				if ((bm->TAC_trigger[fishery_id][endangered_day_id] > 0.0) && (bm->dayt > (bm->TAC_trigger[fishery_id][endangered_day_id] + FC_period2)))
					bm->TAC_trigger[fishery_id][endangered_day_id] = 0.0;
				if (bm->TAC_trigger[fishery_id][endangered_day_id] == 0.0)
					bm->TAC_trigger[fishery_id][endangered_day_id] = bm->dayt;

				if (pseudo_survey_trip) {
					/* Using original simple adaptive management with perfect knowledge model */
					if (bm->dayt < (bm->TAC_trigger[fishery_id][endangered_day_id] + FC_period))
						step1 = (bm->dayt - bm->TAC_trigger[fishery_id][endangered_day_id]) / FC_period;
					else
						step1 = 1.0;
					EFF_scale = 1.0 - (step1 * (1 - FC_restrict));
				} else
					/* Use assessment model output */
					EFF_scale = bm->TAC_trigger[fishery_id][triggered_scalar_id];
			} else {
				bm->TAC_trigger[fishery_id][endangered_day_id] = 0.0;
				EFF_scale = 1.0;
			}
		}

		if ((high_end_trigger_tripped) && (!bm->do_assessing)) {
			/* If in recovery mode let it rebuild */
			step1 = (bm->dayt - bm->TAC_trigger[fishery_id][endangered_day_id]) / FC_period;
			EFF_scale = 1.0 - (step1 * (1 + FC_restrict));
		}

		effort_scale[fishery_id][endangered_tac_id] = EFF_scale;

		/* Set flag so management action recorded */
		if (effort_scale[fishery_id][endangered_tac_id] != 1.0)
			effort_scale[fishery_id][endangered_message_id] = 1;

		/*
         if(bm->dayt > 1090 && (fishery_id == 14 || fishery_id == 13 || fishery_id == 17))
		 fprintf(llogfp, "time: %e, effort_scale[%d][0]: %e, effort_scale[%d][1]: %e, trigger_tripped: %d, end_trigger_tripped: %d\n",
		 bm->dayt, fishery_id, effort_scale[fishery_id][target_tac_id], fishery_id, effort_scale[fishery_id][endangered_tac_id], trigger_tripped, end_trigger_tripped);
         */
		
        /* If using minimum biomass based triggers - in terms of absolute biomass not relative biomass
         Reuse the FC_high_thresh parameter
         */
        if (bm->FISHERYprms[fishery_id][flag_framebased_id]) {
            for (sp = 0; sp < bm->K_num_tot_sp; sp++) {
                if (FunctGroupArray[sp].isTAC) {
                    /* Update annual reporting and storage of old quotas */
                    in_quota = bm->inQuota[fishery_id][sp];
                    if (!in_quota) {
                        /* If not participating in the quota pool skip on */
                        continue;
                    }
                    bm->TACamt[sp][fishery_id][old_id] = bm->TACamt[sp][fishery_id][now_id];
                    FC_high_thresh = bm->SP_FISHERYprms[sp][fishery_id][FC_high_thresh_id];
                    startpop = bm->totinitpop[sp];
                    fishnowpop = bm->totfishpop[sp];
            
                    /* If the biomass as above the threshold then use the fixed quota level - probably fairly unconstrained */
                    if (fishnowpop > (FC_high_thresh * startpop)) {
                        bm->TACamt[sp][fishery_id][now_id] = bm->SP_FISHERYprms[sp][fishery_id][TAC_id];
                    } else {
                        bm->TACamt[sp][fishery_id][now_id] = 0.0;
                    }
                }
            }
        }

        /* If using scenario painting (i.e. not using assessments) then set new TACs if need be */
		flagTACchange = (int) (bm->FISHERYprms[fishery_id][TACchange_id]);
		if (flagTACchange > 0) {
            if (!bm->do_assessing) {
                Manage_Set_Manage_Index(bm, fishery_id, mgmtstability_id, 0);
                //manageindx[fishery_id][mgmtstability_id] = 0;
            
                /* Check to see which scalar (one based on stock or one based on endangered groups)
                 is required to calculate the new TAC adjustment */
                if (effort_scale[fishery_id][target_tac_id] < effort_scale[fishery_id][endangered_tac_id]) {
                    TAC_scale = effort_scale[fishery_id][target_tac_id];
                } else {
                    TAC_scale = effort_scale[fishery_id][endangered_tac_id];
                }

                /* Apply TAC adjustment to give the new TAC */
                for (sp = 0; sp < bm->K_num_tot_sp; sp++) {
                    if (FunctGroupArray[sp].isTAC) {

                        /* Update annual reporting and storage of old quotas */
                        in_quota = bm->inQuota[fishery_id][sp];
                        if (!in_quota) {
                            /* If not participating in the quota pool skip on */
                            continue;
                        }
                        bm->TACamt[sp][fishery_id][old_id] = bm->TACamt[sp][fishery_id][now_id];

                        /* Get forced species specific TAC scaling */
                        if (flagTACchange == change_forced) {
                            TAC_num_changes = (int) (bm->SP_FISHERYprms[sp][fishery_id][TAC_num_changes_id]);
                            TAC_scale = Util_Get_Change_Scale(bm, TAC_num_changes, TACchange[sp][fishery_id]);

                        }

                        /* Get new TAC */
                        if (bm->dayt < 366) // high_trigger cases handled explicitly above;
                            TACorig = bm->SP_FISHERYprms[sp][fishery_id][TAC_id];
                        else
                            TACorig = bm->TACamt[sp][fishery_id][old_id];
                        bm->TACamt[sp][fishery_id][now_id] = TACorig * TAC_scale;

                        fprintf(llogfp, "%e, %s TAC set to %e (TACorig: %e, TAC_scale: %e\n", bm->dayt, FunctGroupArray[sp].groupCode,
							bm->TACamt[sp][fishery_id][now_id], TACorig, TAC_scale);
                    }
				}
			}
            
            /* Disable TAC check */
            if(bm -> flagdisableTAC) {
                for (sp = 0; sp < bm->K_num_tot_sp; sp++) {
                    if (FunctGroupArray[sp].isTAC) {
                        if (flagTACchange == disable_tac ) {
                            TAC_num_changes = (int) (bm->SP_FISHERYprms[sp][fishery_id][TAC_num_changes_id]);
                            TAC_scale = Util_Get_Change_Scale(bm, TAC_num_changes, TACchange[sp][fishery_id]);
                        }
                    }
                }
            }
		}

		/* Set regional TAC - can't have regional TACs or stocks for invertebrates as yet
		 FIX - allow multiple stocks and regional TACs for all groups
		 */
        if (!bm->flagecon_on || bm->MultiPlanEffort) {
            if (bm->manage_reg) {
                for (sp = 0; sp < bm->K_num_tot_sp; sp++) {
                    if (FunctGroupArray[sp].isVertebrate == TRUE) {
						/* As not using stocks figure out relative biomass in each area so can put in relative proportion of TAC */
						nstock = bm->K_num_active_reg;

						for (nreg = 0; nreg < bm->K_num_active_reg; nreg++) {
							bm->lastreg_prop[sp][nreg] = bm->RegionalData[sp][nreg][reg_tac_id];
							bm->initreg_prop[sp][nreg] = 0.0;
						}

						for (ij = 0; ij < bm->nbox; ij++) {
							nreg = bm->regID[ij];
							if (bm->boxes[ij].type != BOUNDARY) {

								for (k = 0; k < bm->boxes[ij].nz; k++) { // loop through each layer in this box.
									for (guild = 0; guild < bm->K_num_tot_sp; guild++) {
										switch (FunctGroupArray[guild].groupAgeType) {
										case AGE_STRUCTURED:
											for (n = 0; n < FunctGroupArray[guild].numCohortsXnumGenes; n++) {
												sn = FunctGroupArray[guild].structNTracers[n];
												rn = FunctGroupArray[guild].resNTracers[n];
												den = FunctGroupArray[guild].NumsTracers[n];
												biomass = (bm->boxes[ij].tr[k][sn] + bm->boxes[ij].tr[k][rn]) * bm->boxes[ij].tr[k][den]
														* FunctGroupArray[guild].habitatCoeffs[WC];

												bm->initreg_prop[guild][nreg] += biomass;

											}
											break;
										case AGE_STRUCTURED_BIOMASS:
										case BIOMASS:
											if (FunctGroupArray[guild].groupType != REF_DET && FunctGroupArray[guild].groupType != LAB_DET) {
												for (n = 0; n < FunctGroupArray[guild].numCohortsXnumGenes; n++) {
													pid = FunctGroupArray[guild].totNTracers[n];
													biomass = bm->boxes[ij].tr[k][pid] * bm->boxes[ij].dz[k] * bm->boxes[ij].area
															* FunctGroupArray[guild].habitatCoeffs[WC];

													bm->initreg_prop[guild][nreg] += biomass;
												}
											}
											break;
										}
									}

									pid = NH3_i; /* Water column DIN - use hardwired references as above and
									 beyond dynamic numbers used for prey elsewhere in the code */
									//nid = bm->DIN_id;
									bm->initreg_prop[bm->DIN_id][nreg] += bm->boxes[ij].tr[k][pid] * bm->boxes[ij].dz[k] * bm->boxes[ij].area;

									pid = NO3_i;
									//nid = bm->DIN_id;
									bm->initreg_prop[bm->DIN_id][nreg] += bm->boxes[ij].tr[k][pid] * bm->boxes[ij].dz[k] * bm->boxes[ij].area;

								}

								/* Loop through each sediment layer */
								for (k = 0; k < bm->boxes[ij].sm.nz; k++) {
									for (guild = 0;  guild < bm->K_num_tot_sp; guild++) {
										switch (FunctGroupArray[guild].groupAgeType) {
										case AGE_STRUCTURED:
										case AGE_STRUCTURED_BIOMASS:
											/* Do nothing  - this is not yet supported.*/
											break;
										case BIOMASS:
											//TODO: Fix this.
											if (FunctGroupArray[guild].groupType != LG_PHY && FunctGroupArray[guild].groupType != SM_PHY) {

												pid = FunctGroupArray[guild].totNTracers[0];
												/* Calculate the biomass of this group in this cell */
												biomass = bm->boxes[ij].sm.tr[k][pid] * bm->boxes[ij].sm.dz[k] * bm->boxes[ij].area
														* FunctGroupArray[guild].habitatCoeffs[SED];

												bm->initreg_prop[guild][nreg] += biomass;
											}
											break;
										}
									}
								}

								/* The eipbenthic layer */
								for (guild = 0; guild < bm->K_num_tot_sp; guild++) {
									switch (FunctGroupArray[guild].groupAgeType) {
									case AGE_STRUCTURED:
										/* Do nothing  - this is not yet supported.*/
										break;
									case AGE_STRUCTURED_BIOMASS:	/* Intentional */
									case BIOMASS:
										// Allow for multiple cohorts so that can have age structured reefs and multiple biomass pools in macrophytes
										for (n = 0; n < FunctGroupArray[guild].numCohortsXnumGenes; n++) {
											pid = FunctGroupArray[guild].totNTracers[n];
											/* Calculate the biomass of this group in this cell */
											if (FunctGroupArray[guild].habitatType == EPIFAUNA){
												biomass = bm->boxes[ij].epi[pid] * bm->boxes[ij].area;
											}else{
												biomass = 0.0;
											}

											bm->initreg_prop[guild][nreg] += biomass;

										}
										break;
									}
								}
							}
						}

						regtot = 0;
						for (nreg = 0; nreg < bm->K_num_active_reg; nreg++) {
							regtot += bm->initreg_prop[sp][nreg];
						}
						/* Make reg_prop trully proportions */
						for (nreg = 0; nreg < bm->K_num_active_reg; nreg++) {
							if (bm->lastreg_prop[sp][nreg] < no_quota) { // So can have regions where no TAC used
								bm->RegionalData[sp][nreg][reg_tac_id] = bm->initreg_prop[sp][nreg] / (regtot + small_num);
							}
						}

					} else {
						nstock = FunctGroupArray[sp].numStocks;
						for (nreg = 0; nreg < nstock; nreg++) {
							/* Get regional scaling */
							for (chrt = 0; chrt < FunctGroupArray[sp].numCohortsXnumGenes; chrt++) {
								bm->RegionalData[sp][nreg][reg_tac_id] += bm->stock_struct_prop[sp][chrt][nreg] / (FunctGroupArray[sp].numCohortsXnumGenes);
							}
						}
					}
				}
			}
		} else {
            /* Dan Holland's model currently assumes US system, which this caricatures */
            Manage_Visit_Council(bm, llogfp);
        }

		/* Adjust seasonal closures if need be */
		//flagseasonal = (int) (bm->FISHERYprms[fishery_id][flagseasonal_id]);
		flagSEASONchange = (int) (bm->FISHERYprms[fishery_id][flagchangeseason_id]);
		fishstartday = (int) (bm->FISHERYprms[fishery_id][seasonopen_id]);
		fishendday = (int) (bm->FISHERYprms[fishery_id][seasonclose_id]);
		SEASONAL[fishery_id][0] = fishstartday;
		if (fishstartday < fishendday)
			season_length = fishendday - fishstartday;
		else
			season_length = fishendday + 364 - fishstartday;
		if (flagSEASONchange) {
			/* Check to see whether the season length is changing */
			if (effort_scale[fishery_id][target_tac_id] < effort_scale[fishery_id][endangered_tac_id]) {
				SEASON_scale = effort_scale[fishery_id][target_tac_id];
			} else
				SEASON_scale = effort_scale[fishery_id][endangered_tac_id];

			if (SEASON_scale != 1.0) {
				Manage_Set_Manage_Index(bm, fishery_id, mgmtstability_id, 0);
			}

			new_length = (int) floor(ROUNDGUARD + (season_length * SEASON_scale));

			newfishendday = fishstartday + new_length;
			/* If season starts in one year and finish in the next */
			if (newfishendday > 364)
				newfishendday = newfishendday - 364;
			tempendday = oldFishEndDay[fishery_id];
			if (tempendday > 0) {
				SEASONAL[fishery_id][1] = tempendday;
				oldFishEndDay[fishery_id] = newfishendday;
			} else {
				/* If season all in one year */
				SEASONAL[fishery_id][1] = newfishendday;
				oldFishEndDay[fishery_id] = 0;
			}
		} else {
			/* If it isn't just sort out whether the season starts in one
			 year and finish in the next... */
			if (fishendday > 364)
				fishendday = fishendday - 364;
			tempendday = oldFishEndDay[fishery_id];
			if (tempendday > 0) {
				SEASONAL[fishery_id][1] = tempendday;
				oldFishEndDay[fishery_id] = fishendday;
			} else {
				/*... or if season all in one year */
				SEASONAL[fishery_id][1] = fishendday;
				oldFishEndDay[fishery_id] = 0;
			}
		}

		/* Do gear shifts - based on stock size */
		if (bm->FISHERYprms[fishery_id][flagchangeSEL_id] == change_dynamic) {
			/* To do a rapid change irrespective of the shape of the gear, actually apply it to the q per species */
			for (sp = 0; sp < bm->K_num_tot_sp; sp++) {
				if (FunctGroupArray[sp].isImpacted == TRUE) {
					bm->SP_FISHERYprms[sp][fishery_id][q_id] *= effort_scale[fishery_id][target_tac_id];
				}
			}
		}

		/* Reset notice switches */
		bm->FISHERYprms[fishery_id][closenotice_id] = 0;
	}
	return;
}
