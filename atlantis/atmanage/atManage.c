/**
 * \ingroup atManageLib
 * \file atManage.c
 * \brief Main managament functions.
 * 	\author	Beth Fulton 8/8/2004
 *
 *	<b>  Revisions</b>
 *	8/8/2004 Created from the semanage.c file
 *
 *	13/8/2004 Counters such as TofY updated to reflect now part of MSEBoxModel structure
 *
 *	16/8/2005 Put in default case for all switch statements
 *
 *	20/8/2005 Moved each step in management calculations into its own routine
 *	(for clarity as details are growing beyond manageable levels for a single routine).
 *
 *  24/8/2005 Modified handling MPAs so can have unlimited number that can
 *  change through time. Entire list is stored in MPAlist and brought over
 *  to current MPA array as required through time
 *
 * 	27/9/2005 Added effort time series usage
 *
 * 	14/04/2008 Beth Fulton
 * 	Added Kreg check in Manage_Visit_Council. Also corrected bim to bm->BiM.
 * 	Also moved "Redo regional ratios" into sp loop given it needs sp index.
 *
 *	22-04-2009 Bec Gorton
 *	Added the ingroup atmanage doxygen tag and replaced the code that calculated the
 *	currentStockID with a call to Util_Get_Current_Stock_Index().
 *
 *	16/6/2009 Beth Fulton
 *	Replaced SP_of_Concern[sp] with FunctGroupArray[sp].speciesParams[sp_concern_id]
 *
 *	28-10-2009 Bec Gorton
 *	Removed references to the K_num_impacted_sp and K_num_fished_sp variables.
 *	Also change references to a number of arrays that have been moved into a module structure.
 *
 *	27-01-2010 Bec Gorton
 *	Replaced the direct use of the harvest array bm->harvestindex with Harvest_Set_Harvest_Index.
 *
 *	28-01-2010 Bec Gorton
 *	Additional doxygen comments. Renamed functions to fit code style.
 *	Removed all the initialisation of variables when they are declared.
 *
 *  3/10/2011 Beth Fulton
 *  Added TotOldCumCatch[sp][nf] so could deal with multi-year quotas
 *
 */

/*  Open library routines */
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "atManage.h"
#include <atEcologyLib.h>
#include "atHarvestLib.h"

/* Static prototypes delarations of private functions */
static double TAC_Check(MSEBoxModel *bm, int fishery_id, int flagmanage, FILE *llogfp);
static double Effort_Restrict_Check(MSEBoxModel *bm, int fishery_id, int flagmanage, int *trigger_tripped, FILE *llogfp);
static double Endangered_Check(MSEBoxModel *bm, int fishery_id, double EFF_scale1, int *end_trigger_tripped, FILE *llogfp);
static void Check_For_Active_MPA(MSEBoxModel *bm, int fishery_id);
static double Forced_Effort_Change(MSEBoxModel *bm, int fishery_id, int flagspeffortmodel, FILE *llogfp);
static double Season_Check(MSEBoxModel *bm, int fishery_id, int ij, FILE *llogfp);
static double Allocate_Immediate_Effort(MSEBoxModel *bm, int fishery_id, int ij, int flagspeffortmodel, double prop_pop_fish, FILE *llogfp);

static void Effort_Displacement(MSEBoxModel *bm, int fishery_id, int ij, double orig_FCpressure, double *FCpressure, double *FCdisplaced, int *new_fish_loc,
		FILE *llogfp);

static void Check_CAP(MSEBoxModel *bm, FILE *llogfp);
static void Calculate_Port_Contrib(MSEBoxModel *bm, int fishery_id, int flagspeffortmodel, FILE *llogfp);
static void Update_Port_Population(MSEBoxModel *bm, FILE *llogfp);
static void GenerateCPUE(MSEBoxModel *bm, FILE *llogfp);
static void allocate_catch(MSEBoxModel *bm, int sp, int nf, int ij, int k, double *catch_unalloced, double *weighted_depth, double this_catch, FILE *llogfp);

int POP_max_num_changes,
	nshot;  // Used in shot-by-shot CPUE generation (needs to be global as used in generation and output writing)
double k_proprecfish;

/**
 * \brief Calculate the effort - this is run at each model timestep to determine what is the
 * current appropriate level of effort for each box for each fishery. Annual management options
 * are carried out in Annual_Fisheries_Mgmt()
 */
void Manage_Calculate_Total_Effort(MSEBoxModel *bm, FILE *llogfp) {
	double EFF_scale1 = 0.0, EFF_scale2 = 0.0, EFF_scale3 = 0.0, EFF_scale4, FCpressure, orig_FCpressure, fish_infringe, FCdisplaced, prop_pop_fish = 0.0, localcell_vol, FC_likeREEF, FC_likeFLAT, FC_likeSOFT, FC_dempel, reef_area, flat_area, soft_area, otherFC_likeREEF, otherFC_likeFLAT, totconflict, otherFC_likeSOFT, otherFC_dempel, dempel_match, K_GearConflict, conflict_contrib, active_scale, step1_cpue, totcatch, dummy, mpa_scale, mpa_infringe, F_displaced, F_rescale;
	int ij, k, sp, nf, flagspeffortmodel, fishery_id, flagmanage, end_trigger_tripped, new_fish_loc = 0, nstock, flagfcmpa,
    crunch_id;
    //int do_debug_nf;
    //int do_debug;
	int ncells = bm->nbox;
	//double record_period = (bm->t - (bm->tfishout - bm->toutfinc)) + 1;
	double EFF_scale0 = 0.0;
	int trigger_tripped;

    /*
	if (bm->debug == debug_effort) {
		fprintf(llogfp, "time: %e record_period: %e, t: %e, tfishout: %e, toutfinc: %e\n", bm->dayt, record_period, bm->t, bm->tfishout, bm->toutfinc);
		do_debug = 1;
	} else {
		do_debug = 0;
     }
     */
    
	if (verbose > 1)
		printf("Determine total effort\n");

    /* Store any CPUE information - do this before shift Effort values around */
    if(bm->flagStoreShotCPUE) {
        GenerateCPUE(bm, llogfp);
    }
    if(bm->flagStoreCPUE) {
        Write_CPUE(bm, llogfp);
    }
    
    /* Initialise local arrays */
    if(bm->EffortModelsActive) {
        for (nf = 0; nf < bm->K_num_fisheries; nf++) {
            Harvest_Set_Harvest_Index(bm, nf, checkdone_id, 0);
            for (ij = 0; ij < ncells; ij++) {
                bm->CumEffort[nf][ij] += bm->Effort[ij][nf];
                bm->OldEffort[ij][nf] = bm->Effort[ij][nf];
                bm->totOldEffort[nf] += bm->OldEffort[ij][nf];
                bm->Effort[ij][nf] = 0.0;
                bm->TempCPUE[ij][nf] = 0.0;
                
                /*
                 if (do_debug && (nf == bm->which_fleet)) {
                 fprintf(llogfp, "Time: %e, %s-%d, effort: %e (oldeffort: %e with totOldEffort: %e)\n", bm->dayt, FisheryArray[nf].fisheryCode, ij,
                 bm->Effort[ij][nf], bm->OldEffort[ij][nf], bm->totOldEffort[nf]);
                 }
                 */
                
                
            }
            scale_effort[nf] = 0;
            bm->totCPUE[nf] = 0;
        }
    }
    
    // If displaceing effort need to clean up the record keeping
    if (bm->flagdisplace) {
        for (nf = 0; nf < bm->K_num_fisheries; nf++) {
            for (ij = 0; ij < ncells; ij++) {
                if(bm->newmonth && bm->flagday) {
                    bm->CumDisplaceEffort[ij][nf] = 0.0;
                } else {
                    bm->CumDisplaceEffort[ij][nf] += bm->Effort[ij][nf];
                }
            }
        }
    }
    
    for (sp = 0; sp < bm->K_num_tot_sp; sp++) {
        if (FunctGroupArray[sp].isTAC) {
            for (ij = 0; ij < bm->K_num_fisheries; ij++) {
                for (k = 0; k < bm->K_num_basket; k++) {
					bm->TAC_over[ij][sp][k] = 0;
                }
            }
        }
    }

    /* Find distribution of catch across regions (for regional TAC based management).*/
	for (sp = 0; sp < bm->K_num_tot_sp; sp++) {
		if (FunctGroupArray[sp].isFished == TRUE) {

			/* Initialise regional catch contributions */
			totcatch = 0.0;
            for (nstock = 0; nstock < FunctGroupArray[sp].numStocks; nstock++) {
                bm->RegionalData[sp][nstock][reg_catch_id] = 0;
            }

			/* Get regional catch info */
			for (nf = 0; nf < bm->K_num_fisheries; nf++) {
				for (ij = 0; ij < bm->nbox; ij++) {
					if (bm->boxes[ij].type != BOUNDARY) {
						for (k = 0; k < bm->boxes[ij].nz; k++) {
							/* Identify appropriate stock */

							if (!bm->flagecon_on || bm->MultiPlanEffort) {
								/* Base model - regional management areas match stocks */
								if (FunctGroupArray[sp].isVertebrate == FALSE) {
									/* No stock structure for invert stocks yet */
									nstock = 0;
								} else {
									nstock = bm->group_stock[sp][ij][k];
								}
							} else {
								/* Dan Holland's model where regional management areas needn't match stocks and is
								 assume constant over all stocks.
								 The id name nstock used as name of id here as that's the id used from base model,
								 but actual value passed is the region id
								 */
								nstock = bm->regID[ij];
							}

							bm->RegionalData[sp][nstock][reg_catch_id] += bm->CumCatch[sp][nf][ij][k];
							totcatch += bm->CumCatch[sp][nf][ij][k];
						}
					}
				}
			}
			/* Now normalise */
			for (nstock = 0; nstock < FunctGroupArray[sp].numStocks; nstock++) {
				bm->RegionalData[sp][nstock][reg_catch_id] /= (totcatch + small_num);
			}
		}
	}
    
    /* Find average CPUE for each box over the last fisheries recording period
	 (becasue catch is cumulative you can't just divide the total catch recorded
	 for the square by the effort from yesterday. As a first pass, reduce catch from
	 yesterday to average catch from the period and then divide by the effort from
	 yesterday.
     
	 NOTE: that CPUE for recreational fishing will be very overstated
	 as catch is lumped for inverts, pelagics, demersals, but is counted for each
	 component species. This was done for ease of computation and will be ok so long
	 as dynamic effort allocation is NOT used for the recreational fishing. A warning
	 to this effect has been added to the model initialisation code. */

    /*
	if (do_debug && (bm->which_fleet < bm->K_num_fisheries)) {
		fprintf(llogfp, "Time: %e, %s totCPUE starts %e\n", bm->dayt, FisheryArray[bm->which_fleet].fisheryCode, bm->totCPUE[bm->which_fleet]);
	}
    */
    
	/* Update Port population if not using dynamic economic drivers */
    if (bm->flagForceRec) {
		Update_Port_Population(bm, llogfp);
    }
     
    if(!bm->EffortModelsActive) {
        // Check MPAs active or not then jump out - need MPA check as used by some catch only models
        if (bm->dayt != bm->predayt) {
            for (fishery_id = 0; fishery_id < bm->K_num_fisheries; fishery_id++) {
                Check_For_Active_MPA(bm, fishery_id);
                
                if (bm->flagdisplace) {
                    for (ij = 0; ij < bm->nbox; ij++) {
                        orig_FCpressure = 1.0;
                        Effort_Displacement(bm, fishery_id, ij, orig_FCpressure, &FCpressure, &FCdisplaced, &new_fish_loc, llogfp);
                        if (FCdisplaced < 0.0)
                            FCdisplaced = 0.0;
                        
                        /* Find the proportion retained and displaced */
                        F_rescale = 1.0 - (FCdisplaced / orig_FCpressure);
                        F_displaced = FCdisplaced / orig_FCpressure;
                        
                        /* Updating scalars - overload via the use of effort array */
                        bm->Effort[ij][fishery_id] *= F_rescale; /* this is so that accumulate additional fishing mortakity pressure due to displaced effort */
                        bm->Effort[new_fish_loc][fishery_id] += F_displaced;
                    }
                }
            }
        }
        return;
    }
        
    for (ij = 0; ij < bm->nbox; ij++) {
		if (bm->boxes[ij].type != BOUNDARY) {
			for (nf = 0; nf < bm->K_num_fisheries; nf++) {
                /* Determine effort model type */
                flagspeffortmodel = (int) (bm->FISHERYprms[nf][flageffortmodel_id]);

                if (!flagspeffortmodel) {
                    continue;
                }
                
                // If have active effort model then continue
				if (!bm->totOldEffort[nf]) {
					bm->TempCPUE[ij][nf] = 0.0;
				} else {
					step1_cpue = 0.0;
					for (sp = 0; sp < bm->K_num_tot_sp; sp++) {
						if (FunctGroupArray[sp].isFished == TRUE) {

							localcell_vol = 0;
							for (k = 0; k < bm->boxes[ij].nz; k++) {
								localcell_vol += bm->boxes[ij].dz[k] * ceil(Effort_vdistrib[ij][k][nf]);
							}
							localcell_vol *= bm->boxes[ij].area;

							//step1_cpue += ((bm->boxes[ij].fishstat[catid] / (localcell_vol + small_num)) / record_period);
							if (bm->FISHERYprms[nf][flaguseall_id]) {
								step1_cpue += bm->LastCatch[sp][nf][ij];
							} else {
								step1_cpue += (bm->LastCatch[sp][nf][ij] * bm->FISHERYtarget[nf][sp]);
							}

                            /*
							if ((bm->newmonth) && (bm->debug == debug_effort) && (nf == bm->which_fleet)) {
								//if(bm->newweek && (nf == trapBMS_id)){
								fprintf(llogfp, "Time: %e, %s in Box%d step1: %e (%e on %s)\n", bm->dayt, FisheryArray[nf].fisheryCode, ij, step1_cpue,
										bm->LastCatch[sp][nf][ij], FunctGroupArray[sp].groupCode);
							}
                             */
						}

					}
					/* Get recent CPUE based on the catch taken in the memory period (used to be the last week of catch) */
					bm->TempCPUE[ij][nf] += step1_cpue / (bm->OldEffort[ij][nf] + small_num);

					/**
					 if(bm->newweek && (nf == trapBMS_id)){
					 fprintf(llogfp, "Time: %e, %s biox%d CPUE contrib by BML: %e\n",
					 bm->dayt, FisheryArray[nf].fisheryCode, ij, bm->LastCatch[BMLguild_id][nf][ij]/(bm->TempCPUE[ij][nf] + small_num));
					 }

					if (do_debug && (bm->which_fleet == nf)) {
						fprintf(llogfp, "Time: %e, %s tempCPUE-%d, %e catch: %e, OldEffort: %e\n", bm->dayt,
								FisheryArray[nf].fisheryCode, ij, bm->TempCPUE[ij][nf], step1_cpue, bm->OldEffort[ij][nf]);
					}
                    **/

				}

				bm->totCPUE[nf] += bm->TempCPUE[ij][nf];

                /*
				if (do_debug && (bm->which_fleet == nf)) {
					fprintf(llogfp, "Time: %e, %s totCPUE %e tempCPUE-%d %e\n", bm->dayt, FisheryArray[nf].fisheryCode, bm->totCPUE[nf], ij,
							bm->TempCPUE[ij][nf]);
				}
                */
			}
		}
	}
    
    for (k = 0; k < bm->K_num_fisheries; k++) {
		if (bm->totOldEffort[k] < small_num)
			bm->totOldEffort[k] = 0.0;

        /*
		if (do_debug && (bm->which_fleet == k)) {
			//if((k == bm->which_fleet) && ((!bm->TofY) || (bm->TofY == 91) || (bm->TofY == 182) || (bm->TofY == 273))){
			for (ij = 0; ij < bm->nbox; ij++) {
				fprintf(llogfp, "Time: %e, %s totCPUE %e tempCPUE-%d %e\n", bm->dayt, FisheryArray[k].fisheryCode, bm->totCPUE[k], ij, bm->TempCPUE[ij][k]);
			}
		}
        */
	}
    
    /**** Calculate new distribution of effort *******************************************/
	for (fishery_id = 0; fishery_id < bm->K_num_fisheries; fishery_id++) {

        /*
        if (do_debug && (bm->which_fleet == fishery_id)) {
			do_debug_nf = 1;
		} else {
			do_debug_nf = 0;
         }
        */
        /* Before do  management (spatially explicit or otherwise) identify active MPAs - needs to be hear as might want to use dynamic MOAs even without having an explicit effort model on - so need the routine here so it id activated regardless and isn't missed if the effort model is off and the 'continue' triggered */
        if (bm->dayt != bm->predayt) {
            Check_For_Active_MPA(bm, fishery_id);
        }

		/* Determine effort model type */
		flagspeffortmodel = (int) (bm->FISHERYprms[fishery_id][flageffortmodel_id]);
        
        //fprintf(llogfp, "Time: %e %s flagspeffortmodel: %d\n", bm->dayt, FisheryArray[fishery_id].fisheryCode, flagspeffortmodel);

        
        /* Check if fishery activated and if no set to zero and only continue if fishery active this time step */
		if (((!flagspeffortmodel) && ((!mEff[fishery_id][bm->NextQofY]) && (!mEff[fishery_id][bm->QofY]))) || (!bm->FISHERYprms[fishery_id][fisheriesactive_id])) {
			for (ij = 0; ij < bm->nbox; ij++) {
				bm->Effort[ij][fishery_id] = 0;
			}
			continue;
		}
        
        /* If active determine what proportion of the entire day it is active
		 and then compare that against dt, so can scale realised effort accordingly
		 */
		if (bm->FISHERYprms[fishery_id][fisheriesflagactive_id] < 2)
            active_scale = bm->dt / 43200.0;
		else
            active_scale = 1.0;
        
        if (flagspeffortmodel != readts_effort) {

            /* Effort management */
			flagmanage = (int) (bm->FISHERYprms[fishery_id][flagmanage_id]);
			EFF_scale0 = Effort_Restrict_Check(bm, fishery_id, flagmanage, &trigger_tripped, llogfp);
            
            /* TAC-based adjustment to effort */
			EFF_scale1 = TAC_Check(bm, fishery_id, flagmanage, llogfp);
            
            /* Protected-endangered-threatened species management */
			EFF_scale2 = Endangered_Check(bm, fishery_id, EFF_scale1, &end_trigger_tripped, llogfp);
            
            /* If general change in fishing pressure through time (gradual change) calculate
			 new levels here. */
			EFF_scale3 = Forced_Effort_Change(bm, fishery_id, flagspeffortmodel, llogfp);
            
            /* If recreational fishery determine human population actually fishing */
            if(FisheryArray[fishery_id].isRec) {
				prop_pop_fish = k_proprecfish;
            } else {
				prop_pop_fish = 1.0;
            }

            /* Before do spatially explicit management identify active MPAs - moved earlier so available even if not using dynamic effort
			if (bm->dayt != bm->predayt)
				Check_For_Active_MPA(bm, fishery_id);
             */
        } else if (flagmanage > stock_adapt_mgmt){
            /* TAC-check needed so discard if over TAC amount - dummy not used but TAC_over calculated within the routine is */
            dummy = TAC_Check(bm, fishery_id, flagmanage, llogfp);
        }

        /* Check port status and contribution */
        if (bm->FISHERYprms[fishery_id][fisheries_need_port_id]) {
			Calculate_Port_Contrib(bm, fishery_id, flagspeffortmodel, llogfp);
        }

        /* Allocate effort */
		for (ij = 0; ij < bm->nbox; ij++) {
			if (bm->boxes[ij].type != BOUNDARY) {

				if (flagspeffortmodel != readts_effort) {

					FCpressure = Allocate_Immediate_Effort(bm, fishery_id, ij, flagspeffortmodel, prop_pop_fish, llogfp);

					/* Seasonal closures */
					EFF_scale4 = Season_Check(bm, fishery_id, ij, llogfp);

					/* Adjust based on spatially independent effort management (or changes) above */
					FCpressure = EFF_scale0 * EFF_scale1 * EFF_scale2 * EFF_scale3 * EFF_scale4 * FCpressure;

					/* Set the flag allowing an effort drop to on if management
					 is forcing an effort drop */
					if ((EFF_scale1 < 1) || (EFF_scale2 < 1) || (EFF_scale3 < 1))
						flagdropeffort[fishery_id] = 1;

					scale_effort[fishery_id] = EFF_scale0 * EFF_scale1 * EFF_scale2 * EFF_scale3 * EFF_scale4;

                    /**
					//if (do_debug_nf) {
						fprintf(llogfp, "Time: %e box %d %s in %d FCpressure: %e, EFF_scale0: %e, EFF_scale1: %e, EFF_scale2: %e, EFF_scale3: %e, EFF_scale4: %e\n", bm->dayt,
								ij, FisheryArray[fishery_id].fisheryCode, ij, FCpressure, EFF_scale0, EFF_scale1, EFF_scale2, EFF_scale3, EFF_scale4);
					//}
                     **/

					/* Spatial management */
					orig_FCpressure = FCpressure;
					FCpressure = FCpressure * bm->MPA[ij][fishery_id];
                    
                    /**
					//if (do_debug_nf) {
						//if(fishery_id == trapBMS_id)
						fprintf(llogfp, "Time: %e, box %d %s FCpressure: %e (with MPA_scale: %e)\n", bm->dayt, ij, FisheryArray[fishery_id].fisheryCode,
								FCpressure, bm->MPA[ij][fishery_id]);
					//}
                    **/

					/* Potential infringement (if using spatial management) is calculated
					 as a percentage of the fishing that would occur if the area wasn't spatially managed */
					if (bm->flaginfringe) {
						/* Apply in infringement (end result can't be greater than if no spatial management
						 at all) */
						fish_infringe = bm->FISHERYprms[fishery_id][infringe_id] * orig_FCpressure;
						FCpressure += fish_infringe;
						if (orig_FCpressure < FCpressure)
							FCpressure = orig_FCpressure;
					}

					/* Effort displacement - this can be due to the institution of an
					 MPA without any reduction int total effort; or it can be due to
					 effort transfer when local stocks are exhausted */
					Effort_Displacement(bm, fishery_id, ij, orig_FCpressure, &FCpressure, &FCdisplaced, &new_fish_loc, llogfp);

					if (FCpressure < 0.0)
						FCpressure = 0.0;

					if (FCdisplaced < 0.0)
						FCdisplaced = 0.0;

				} else {
					/* Load effort to impose */
					FisheryTimeSeries *this_tsEffort = &bm->tsEffort[ij];

					if (bm->tsEfforttype == interpolate_id)
						FCpressure = tsEval(&this_tsEffort->ts, bm->tseffortid[fishery_id], bm->t);
					else
						FCpressure = tsEvalEx(&this_tsEffort->ts, bm->tseffortid[fishery_id], bm->t);
                    
                    /* Spatial management */
                    flagfcmpa = (int) (bm->FISHERYprms[fishery_id][flagmpa_id]);
                    if (flagfcmpa) {
                        mpa_scale = bm->MPA[bm->current_box][fishery_id];
                        
                        /* Allow for infringement */
                        if (bm->flaginfringe) {
                            mpa_infringe = bm->FISHERYprms[fishery_id][infringe_id];
                            if (mpa_infringe > mpa_scale)
                                mpa_scale = mpa_infringe;
                        }
                    } else {
                        mpa_scale = 1.0;
                    }
                    
                    FCpressure = FCpressure * mpa_scale;
                    
                    if (bm->flagdisplace) {
                        orig_FCpressure = FCpressure;
                        Effort_Displacement(bm, fishery_id, ij, orig_FCpressure, &FCpressure, &FCdisplaced, &new_fish_loc, llogfp);
                        
                        if (FCpressure < 0.0)
                            FCpressure = 0.0;

                        if (FCdisplaced < 0.0)
                            FCdisplaced = 0.0;
                    } else {
                        FCdisplaced = 0.0;
                    }
				}

				//fprintf(bm->logFile, "FCpressure = %.20e, active_scale = %.20e\n",FCpressure, active_scale);
				//fprintf(llogfp, "Fishery = %d, box - %d, FCpressure = %e, active_scale = %e, flagspeffortmodel = %d\n", fishery_id, ij, FCpressure, active_scale, flagspeffortmodel);
				
                /* Store effort in terms of effort per active time step rather than per day per se.
				 If only active in part of the day then only count that period
				 */
				bm->Effort[ij][fishery_id] += FCpressure * active_scale; /* this is the value thats actually used in the harvest code */
				bm->Effort[new_fish_loc][fishery_id] += FCdisplaced * active_scale;

				/* Keep track of effort units expended for individual effort quotas - so can use
				 days-at-sea limit and differential effort costs per location as incentive-based
				 means of dictating effort distributions.
				 */
				bm->GhostEffort[ij][fishery_id] += FCpressure * active_scale * bm->EffortPenalty[ij][fishery_id];

                /*
				if (do_debug_nf) {
					//if((fishery_id == bm->which_fleet) && ((!bm->TofY) || (bm->TofY == 91) || (bm->TofY == 182) || (bm->TofY == 273))){
					fprintf(llogfp, "time: %e, %s bm->Effort[%d] = %.20e, displaced: %f to bm->Effort[%d][%d]: %e\n", bm->dayt,
							FisheryArray[fishery_id].fisheryCode, ij, bm->Effort[ij][fishery_id], FCdisplaced, new_fish_loc, fishery_id,
							bm->Effort[new_fish_loc][fishery_id]);
				}
                */
			}
		}
	}
    
    /* If overall effort declines not allowed for that fishery at that time rescale effort */
	for (fishery_id = 0; fishery_id < bm->K_num_fisheries; fishery_id++) {
        
		for (ij = 0; ij < ncells; ij++) {
			bm->totNewEffort[fishery_id] += bm->Effort[ij][fishery_id];
		}
        
		flagspeffortmodel = (int) (bm->FISHERYprms[fishery_id][flageffortmodel_id]);
		if (flagspeffortmodel == wgt_effort || flagspeffortmodel == dist_effort) {
			if (bm->flagbuffereffort) {
				if (flagspeffortmodel == array_effort || flagspeffortmodel == dist_effort) {
					if (scale_effort[fishery_id] > 1.0) {
						if (bm->totNewEffort[fishery_id] > scale_effort[fishery_id] * bm->totOldEffort[fishery_id]) {
							bm->totNewEffort[fishery_id] = scale_effort[fishery_id] * bm->totOldEffort[fishery_id];
							flagdropeffort[fishery_id] = 0;
						}
					} else {
						if (bm->totNewEffort[fishery_id] < scale_effort[fishery_id] * bm->totOldEffort[fishery_id]) {
							bm->totNewEffort[fishery_id] = scale_effort[fishery_id] * bm->totOldEffort[fishery_id];
							flagdropeffort[fishery_id] = 0;
						}
					}
				}
			}
			if (!flagdropeffort[fishery_id]) {
				if (bm->totNewEffort[fishery_id] < bm->totOldEffort[fishery_id]) {
					if (!bm->totNewEffort[fishery_id]) {
						bm->totNewEffort[fishery_id] = 0.0;
						for (ij = 0; ij < ncells; ij++) {
							bm->totNewEffort[fishery_id] += bm->OldEffort[ij][fishery_id];
						}

					}

					for (ij = 0; ij < ncells; ij++)
						bm->Effort[ij][fishery_id] *= bm->totOldEffort[fishery_id] / (bm->totNewEffort[fishery_id] + small_num);
				}
			}
		}
        
		if (flagspeffortmodel == alt_cpue_effort && bm->dayt > 0) {
			for (ij = 0; ij < ncells; ij++) {

				if (!bm->TofY && !bm->totOldEffort[fishery_id]){
					/* If no previous effort leave as is so that can get exploratory fishing */
					//bm->Effort[ij][fishery_id] = bm->Effort[ij][fishery_id];
					;
				}else{
					/* If there was previous effort then redistribute it based on above
					 bm->TempCPUE calcs */
					bm->Effort[ij][fishery_id] *= scale_effort[fishery_id] * bm->totOldEffort[fishery_id] / (bm->totNewEffort[fishery_id] + small_num);
				}
			}
		}
	}
    
    /* Calculate final effort and any gear conflict */
	crunch_id = bm->conflict_id;
    for (ij = 0; ij < bm->nbox; ij++) {
		bm->boxes[ij].fishstat[crunch_id] = 0;
    }
	totconflict = 0;
    
    /* Find a gear conflict indicator - how many fisheries and having conflicts with gear *
	 * This has no impact on the dynamics at all its just an output indicator. */
	for (fishery_id = 0; fishery_id < bm->K_num_fisheries; fishery_id++) {

		/* Fisheries' habitat preferences */
		FC_likeREEF = bm->FC_HABITATlike[fishery_id][bm->REEFcover_id];
		FC_likeFLAT = bm->FC_HABITATlike[fishery_id][bm->FLATcover_id];
		FC_likeSOFT = bm->FC_HABITATlike[fishery_id][bm->SOFTcover_id];
		FC_dempel = bm->FISHERYprms[fishery_id][flagdempelfishery_id];
		bm->totNewEffort[fishery_id] = 0.0;

		bm->FISHERYprms[fishery_id][conflict_contrib_id] = 0;
		for (ij = 0; ij < ncells; ij++) {
			bm->totNewEffort[fishery_id] += bm->Effort[ij][fishery_id];

			/* Find gear overlap */
			if (!bm->FISHERYprms[fishery_id][flagconflict_id]) {
				if (!fishery_id) {
					/* Initialise the value */
					bm->boxes[ij].fishstat[crunch_id] = 0;
				}
				continue;
			}

			reef_area = bm->boxes[ij].reef;
			flat_area = bm->boxes[ij].flat;
			soft_area = bm->boxes[ij].soft;
			if (!fishery_id) {
				/* Initialise the value */
				bm->boxes[ij].fishstat[crunch_id] = 0;
			}
			/* Calculate simple conflict proxy */
			for (k = 0; k < bm->K_num_fisheries; k++) {
				otherFC_likeREEF = bm->FC_HABITATlike[k][bm->REEFcover_id];
				otherFC_likeFLAT = bm->FC_HABITATlike[k][bm->FLATcover_id];
				otherFC_likeSOFT = bm->FC_HABITATlike[k][bm->SOFTcover_id];
				otherFC_dempel = bm->FISHERYprms[k][flagdempelfishery_id];

				dempel_match = FC_dempel * otherFC_dempel;

				K_GearConflict = gear_conflict[fishery_id][k];
				conflict_contrib = dempel_match * K_GearConflict * (FC_likeREEF * otherFC_likeREEF * reef_area + FC_likeFLAT * otherFC_likeFLAT * flat_area
						+ FC_likeSOFT * otherFC_likeSOFT * soft_area);
				bm->boxes[ij].fishstat[crunch_id] += conflict_contrib;

				bm->FISHERYprms[fishery_id][conflict_contrib_id] += conflict_contrib;
				totconflict += conflict_contrib;
			}
		}
	}
    
    /* Check fishing effort cap */
	Check_CAP(bm, llogfp);
    
    for (sp = 0; sp < bm->K_num_fisheries; sp++) {
		/* Final relative conflict (as running average) */
		bm->FISHERYprms[sp][relconflict_id] = bm->FISHERYprms[sp][conflict_contrib_id] / (totconflict + small_num);

		/* Reset Effort */
		bm->totOldEffort[sp] = 0.0;
		bm->totNewEffort[sp] = 0.0;
	}
    
	return;
}

/**
 * \brief Effort constraint checking - management agency reigning in effort based on stock size
 *
 * @return Returns a scalar to adjust realised effort by to take into account effort restrictions based on stock size
 */
double Effort_Restrict_Check(MSEBoxModel *bm, int fishery_id, int flagmanage, int *trigger_tripped, FILE *llogfp) {
	int sp_no_adapt_mgmt, sp_stock_adapt_mgmt;
	double EFF_scale1 = 1.0;

	sp_no_adapt_mgmt = 0;
	sp_stock_adapt_mgmt = 0;

	if (!flagmanage) {
		sp_no_adapt_mgmt = 1;
    } else if (flagmanage == stock_adapt_mgmt) {
		sp_stock_adapt_mgmt = 1;
    }

	/* Also apply stock based management if using effort based MSY levers */
    if (bm->FISHERYprms[fishery_id][use_msy_effort_id]) {
		sp_stock_adapt_mgmt = 1;
    }

	/* No adaptive management - do nothing */
	if (sp_no_adapt_mgmt) {
		EFF_scale1 = 1.0;
		return EFF_scale1;
	}

	/* Simple adaptive management responds to the size of the target
	 species for this fishery. If a population has fallen to critical
	 % of virgin stock size overall effort is reduced.
	 The assessments are made in Annual_Fisheries_Mgmt() and applied here */
	if (sp_stock_adapt_mgmt) {
		EFF_scale1 = effort_scale[fishery_id][target_tac_id];
		if (EFF_scale1 != 1.0) {
			*trigger_tripped = 1;
			/* If first time for the year record the fact */
			if (effort_scale[fishery_id][target_message_id]) {
				fprintf(llogfp, "time: %e, Stock size trigger set off by fishery %s\n", bm->dayt, FisheryArray[fishery_id].fisheryCode);
				effort_scale[fishery_id][target_message_id] = 0;
			}
		} else
			*trigger_tripped = 0;
	}

	return EFF_scale1;
}
/**
 * \brief TAC checking - management agency checking landings so can restrict, close fishery if TAC is filled
 *
 * @return Returns a scalar to adjust realised effort by to take into account TAC issues
 */
double TAC_Check(MSEBoxModel *bm, int fishery_id, int flagmanage, FILE *llogfp) {
	int sp, nstock = 0, max_sp, taccount, sp_basket_mem, regtaccount, nreg, co_basket_sp, flagTACparticipate, sp_reg, sp_TAC_mgmt, sp_coTAC_mgmt, nf,
    cumTripLim_mgmt, Right_To_Count, sp_basketTAC_mgmt, sp_regionalTAC_mgmt, flag_commonpool_TAC, in_quota;
    //int do_debug;
	double TAC, basketCatch, regTAC_scale, regC_scale, baseEFF_scale1 = 1.0, baskEFF_scale1, regEFF_scale1, CumCatch, totCumCatch;
	double EFF_scale1 = 1.0;

	sp_TAC_mgmt = 0;
	sp_coTAC_mgmt = 0;
	sp_basketTAC_mgmt = 0;
	sp_regionalTAC_mgmt = 0;
	cumTripLim_mgmt = 0;
	if (flagmanage == TAC_mgmt) {
		sp_TAC_mgmt = 1;
	} else if ((flagmanage == coTAC_mgmt) || (flagmanage == coBTAC_mgmt) || (flagmanage == coBRTAC_mgmt)) {
		sp_coTAC_mgmt = 1;
    } else if (flagmanage == cumTACclosure_mgmt) {
		cumTripLim_mgmt = 1;
    }

	if ((flagmanage == basketTAC_mgmt) || (flagmanage == coBTAC_mgmt) || (flagmanage == RbasketTAC_mgmt)) {
		sp_basketTAC_mgmt = 1;
	}

	if ((flagmanage == regionalTAC_mgmt) || (flagmanage == coBRTAC_mgmt) || (flagmanage == RbasketTAC_mgmt) || cumTripLim_mgmt) {
		sp_regionalTAC_mgmt = 1;
	}

	flag_commonpool_TAC = (int) (bm->FISHERYprms[fishery_id][flagTACpartipcate_id]);
	max_sp = (int) (bm->FISHERYprms[fishery_id][max_num_sp_id]);

    /*
	if (((bm->debug == debug_fishing) || (bm->debug == debug_effort)) && ((bm->which_fleet == fishery_id) || (bm->which_fleet >= bm->K_num_fisheries))
			&& (bm->dayt >= bm->checkstart) && (bm->dayt <= bm->checkstop))
		//	if(((bm->which_fleet == fishery_id) || (bm->which_fleet >= bm->K_num_fisheries)) && (bm->dayt >= bm->checkstart) && (bm->dayt <= bm->checkstop))
		do_debug = 1;
	else
		do_debug = 0;
    //do_debug = 1;

	if (do_debug) {
		fprintf(llogfp, "Time: %e, %s has sp_TAC_mgmt: %d, sp_coTAC_mgmt: %d, cumTripLim_mgmt: %d, sp_basketTAC_mgmt: %d, sp_regionalTAC_mgmt: %d\n", bm->dayt,
				FisheryArray[fishery_id].fisheryCode, sp_TAC_mgmt, sp_coTAC_mgmt, cumTripLim_mgmt, sp_basketTAC_mgmt, sp_regionalTAC_mgmt);
        printf("Time: %e, %s has sp_TAC_mgmt: %d, sp_coTAC_mgmt: %d, cumTripLim_mgmt: %d, sp_basketTAC_mgmt: %d, sp_regionalTAC_mgmt: %d\n", bm->dayt,
                FisheryArray[fishery_id].fisheryCode, sp_TAC_mgmt, sp_coTAC_mgmt, cumTripLim_mgmt, sp_basketTAC_mgmt, sp_regionalTAC_mgmt);
	}
     */
    
	if (!flag_commonpool_TAC) {
		/* Simple or companion TACs per fishery in place */
		/* Companion TACs - in effect the same as simple except how calculated
		 in the first place */
		if (sp_TAC_mgmt || sp_coTAC_mgmt) {
			/* TAC per fleet imposed */
			taccount = 0;
			for (sp = 0; sp < bm->K_num_tot_sp; sp++) {
				if (FunctGroupArray[sp].isTAC) {
					in_quota = bm->inQuota[fishery_id][sp];

					/* Convert from kg wet weight to mg N */
					TAC = (bm->TACamt[sp][fishery_id][now_id] * kg_2_mg) / bm->X_CN;
					totCumCatch = Harvest_Get_TotCumCatch(sp, fishery_id, bm->thisyear); //bm->TotCumCatch[sp][fishery_id];
					CumCatch = totCumCatch + bm->TotOldCumCatch[sp][fishery_id];

					/* Bycatch quota sensitivity for species of concern */
					if (bm->BycatchCountDiscards && (!bm->flagTACincludeDiscard && (FunctGroupArray[sp].isFished == TRUE))) {
						/* Include discards in the count if they are a aspecies of concern and they aren't already counted (don't want to double dip) */
						CumCatch += FunctGroupArray[sp].speciesParams[sp_concern_id] * (bm->TotCumDiscards[sp][fishery_id] + bm->TotOldCumDiscards[sp][fishery_id]);

					}

                    /*
					if (do_debug) {
						fprintf(llogfp, "Time: %e %s for %s for mgmt in_quota: %d, CumCatch: %e, TAC: %e (base TAC: %e)\n", bm->dayt,
								FisheryArray[fishery_id].fisheryCode, FunctGroupArray[sp].groupCode, in_quota, CumCatch, TAC,
								bm->TACamt[sp][fishery_id][now_id]);
                        printf("Time: %e %s for %s for mgmt in_quota: %d, CumCatch: %e, TAC: %e (base TAC: %e)\n", bm->dayt,
                                FisheryArray[fishery_id].fisheryCode, FunctGroupArray[sp].groupCode, in_quota, CumCatch, TAC,
                                bm->TACamt[sp][fishery_id][now_id]);
					}
                    */

					if (in_quota && (CumCatch > TAC)) {
						bm->TAC_over[sp][fishery_id][0] = 1;
						/*
						if (bm->dayt > bm->checkstart)
							fprintf(llogfp, "time: %e, Cumulative catch of %s (Cumcatch %e - totcum_t: %e totcumdis_t %e) is over TAC (%e) for fishery %s\n",
									bm->dayt, FunctGroupArray[sp].groupCode, CumCatch, totCumCatch * bm->X_CN * mg_2_tonne, bm->TotCumDiscards[sp][fishery_id]
											* bm->X_CN * mg_2_tonne, TAC, FisheryArray[fishery_id].fisheryCode);
						*/
						taccount++;
					}
				}
			}
            /*
			if (do_debug) {
				fprintf(llogfp, "Time: %e %s comgmt taccount: %d, max_sp: %d\n", bm->dayt, FisheryArray[fishery_id].fisheryCode, taccount, max_sp);
                printf("Time: %e %s comgmt taccountA: %d, max_sp: %d\n", bm->dayt, FisheryArray[fishery_id].fisheryCode, taccount, max_sp);
			}
            */

			if (taccount >= max_sp) {
				baseEFF_scale1 = 0.0;
				if (!bm->FISHERYprms[fishery_id][closenotice_id]) {
					fprintf(llogfp, "time: %e, due to number of species over TAC (%d vs max_allowed %d) fishery %s closed\n", bm->dayt, taccount, max_sp,
							FisheryArray[fishery_id].fisheryCode);
					bm->FISHERYprms[fishery_id][closenotice_id] = 1;
				}
			} else
				baseEFF_scale1 = 1.0;
		} else
			baseEFF_scale1 = 1.0;

		/* Do rest outside switch so can have combos of regional, companion and basket TACs */
		/* Basket TACs */
		if (sp_basketTAC_mgmt) {
			taccount = 0;
			for (sp = 0; sp < bm->K_num_tot_sp; sp++) {
				if (FunctGroupArray[sp].isTAC) {
                    
                    sp_basket_mem = (int) (FunctGroupArray[sp].speciesParams[basketSP_id]);
					if (!sp_basket_mem) /* Only include those members in the basket */
						continue;
					/* Check to see if basket exceeded already */
					if (bm->TAC_over[sp][fishery_id][0])
						continue;

                    /* Convert from kg wet weight to mg N */
					TAC = (bm->TACamt[sp][fishery_id][now_id] * kg_2_mg) / bm->X_CN;

					/* Get total catch of the basket */
					/* Leading group */
					basketCatch = Harvest_Get_TotCumCatch(sp, fishery_id, bm->thisyear) + bm->TotOldCumCatch[sp][fishery_id];

                    /* All other groups */
					nstock = (int) (FunctGroupArray[sp].speciesParams[basket_size_id]);

                    for (nreg = 0; nreg < nstock; nreg++) {
                        co_basket_sp = bm->sp_basket[sp][nreg];
                        
						/* Don't try and do anything for groups that aren't fished
						 - should never actually be tripped, but here just to be safe */
						if (co_basket_sp == -1 || FunctGroupArray[co_basket_sp].isFished == FALSE)
							continue;
						//basketCatch += bm->TotCumCatch[co_basket_sp][fishery_id][bm->thisyear];
                               
						basketCatch += (Harvest_Get_TotCumCatch(co_basket_sp, fishery_id, bm->thisyear) + bm->TotOldCumCatch[co_basket_sp][fishery_id]);
						TAC += (bm->TACamt[co_basket_sp][fishery_id][now_id] * kg_2_mg) / bm->X_CN;
					}

                    if (basketCatch > TAC) {
						/* Set the bm->TAC_over flag for all groups in the basket */
						/* Leading group */
						bm->TAC_over[sp][fishery_id][0] = 1;

						/* All other groups */
						nstock = (int) (FunctGroupArray[sp].speciesParams[basket_size_id]);
						for (nreg = 0; nreg < nstock; nreg++) {
							co_basket_sp = bm->sp_basket[sp][nreg];
							/* Don't try and do anything for groups that aren't fished
							 - should never actually be tripped, but here just to be safe */
							if (co_basket_sp == -1 || FunctGroupArray[co_basket_sp].isFished == FALSE)
								continue;
							bm->TAC_over[co_basket_sp][fishery_id][0] = 1;
						}

                        /*
						if (do_debug) {
							fprintf(llogfp, "time: %e, Cumulative basket catch of %s (%e) is over TAC (%e) for fishery %s\n", bm->dayt,
									FunctGroupArray[sp].groupCode, basketCatch, TAC, FisheryArray[fishery_id].fisheryCode);
						}
                        */
						taccount += nstock;
					}
				}
			}
			max_sp = (int) (bm->FISHERYprms[fishery_id][max_num_sp_id]);

            /*
			if (do_debug) {
				fprintf(llogfp, "Time: %e %s comgmt taccount: %d, max_sp: %d\n", bm->dayt, FisheryArray[fishery_id].fisheryCode, taccount, max_sp);
                printf("Time: %e %s comgmt taccountB: %d, max_sp: %d closenotice: %e\n", bm->dayt, FisheryArray[fishery_id].fisheryCode, taccount, max_sp, bm->FISHERYprms[fishery_id][closenotice_id]);
			}
            */
            
			if (taccount >= max_sp) {
				baskEFF_scale1 = 0.0;
				if (!bm->FISHERYprms[fishery_id][closenotice_id]) {
					fprintf(llogfp, "time: %e, due to number of basket species over TAC (%d vs max_allowed %d) fishery %s closed\n", bm->dayt, taccount,
							max_sp, FisheryArray[fishery_id].fisheryCode);
					bm->FISHERYprms[fishery_id][closenotice_id] = 1;
				}
            } else {
				baskEFF_scale1 = 1.0;
            }
        } else {
			baskEFF_scale1 = 1.0;
        }

		/* Regional TACs */
		if (sp_regionalTAC_mgmt) {
			taccount = 0;
			for (sp = 0; sp < bm->K_num_tot_sp; sp++) {
				if (FunctGroupArray[sp].isTAC) {
					sp_reg = (int) (FunctGroupArray[sp].speciesParams[regionalSP_id]);
					in_quota = bm->inQuota[fishery_id][sp];
					totCumCatch = Harvest_Get_TotCumCatch(sp, fishery_id, bm->thisyear);

					if (bm->manage_reg)
						nstock = bm->K_num_active_reg;
					else
						nstock = FunctGroupArray[sp].numStocks;

					if (!in_quota || (!sp_reg && (nstock < 1))) /* Only include those groups under regional TACs with seperate stocks */
						continue;

					for (nreg = 0; nreg < nstock; nreg++) {
						/* Get regional scaling */
						regTAC_scale = bm->RegionalData[sp][nreg][reg_tac_id];
						regC_scale = bm->RegionalData[sp][nreg][reg_catch_id];

						/* Convert from tonnes wet weight to mg N */
						TAC = (regTAC_scale * bm->TACamt[sp][fishery_id][now_id] * kg_2_mg) / bm->X_CN;
						if(((totCumCatch + bm->TotOldCumCatch[sp][fishery_id]) * regC_scale) > TAC){
							bm->TAC_over[sp][fishery_id][nreg] = 1;

                            /*
							if (do_debug) {
								fprintf(llogfp, "time: %e, Cumulative regional catch of %s (%e) is over TAC (%e) for fishery %s in region %d\n", bm->dayt,
										FunctGroupArray[sp].groupCode, ((totCumCatch + bm->TotOldCumCatch[sp][fishery_id]) * regC_scale), TAC, FisheryArray[fishery_id].fisheryCode, nreg);

							}
                            */
						}
					}
				}
			}
			/* Check for complete fishery closure everywhere */
			for (sp = 0; sp < bm->K_num_tot_sp; sp++) {
				if (FunctGroupArray[sp].isTAC) {
					regtaccount = 0;
					nstock = FunctGroupArray[sp].numStocks;
					for (nreg = 0; nreg < nstock; nreg++) {
						if (bm->TAC_over[sp][fishery_id][nreg])
							regtaccount++;
					}
					if (regtaccount == nstock) {
						taccount++;
					}
				}
			}
			max_sp = (int) (bm->FISHERYprms[fishery_id][max_num_sp_id]);

            /*
			if (do_debug) {
				fprintf(llogfp, "Time: %e %s regional taccount: %d, max_sp: %d\n", bm->dayt, FisheryArray[fishery_id].fisheryCode, taccount, max_sp);
                printf("Time: %e %s regional taccount: %d, max_sp: %d\n", bm->dayt, FisheryArray[fishery_id].fisheryCode, taccount, max_sp);
			}
             */
            
			if (taccount >= max_sp) {
				regEFF_scale1 = 0.0;
				if (!bm->FISHERYprms[fishery_id][closenotice_id]) {
					fprintf(llogfp, "time: %e, due to number of regional species over TAC (%d vs max_allowed %d) in every region fishery %s closed\n",
							bm->dayt, taccount, max_sp, FisheryArray[fishery_id].fisheryCode);
					bm->FISHERYprms[fishery_id][closenotice_id] = 1;
				}
            } else {
				regEFF_scale1 = 1.0;
            }
        } else {
			regEFF_scale1 = 1.0;
        }
	} else {
		/* Cumulative TACs */
		/* FIX - should be able to generalise simplify this code to get simpler code */
		taccount = 0;
		if (sp_TAC_mgmt || sp_coTAC_mgmt) {
			for (sp = 0; sp < bm->K_num_tot_sp; sp++) {
				if (FunctGroupArray[sp].isTAC) {
					TAC = 0;
					CumCatch = 0;
					Right_To_Count = 0;
					if (!bm->flagTACincludeDiscard && (FunctGroupArray[sp].isFished == TRUE))
						Right_To_Count = 1;

					for (nf = 0; nf < bm->K_num_fisheries; nf++) {
						flagTACparticipate = (int) (bm->FISHERYprms[nf][flagTACpartipcate_id]);
						if (flagTACparticipate) {
							/* Convert from kg wet weight to mg N */
							TAC += (bm->TACamt[sp][nf][now_id] * kg_2_mg) / bm->X_CN;
							CumCatch += (Harvest_Get_TotCumCatch(sp, nf, bm->thisyear) + bm->TotOldCumCatch[sp][nf]);


							/* Bycatch quota sensitivity for species of concern */
							if (bm->BycatchCountDiscards && Right_To_Count) {
								/* Include discards in the count if they are a aspecies of concern and they aren't already counted (don't want to double dip) */
								CumCatch += FunctGroupArray[sp].speciesParams[sp_concern_id] * (bm->TotCumDiscards[sp][nf] + bm->TotOldCumDiscards[sp][nf]);
							}
						}
					}
					if (CumCatch > TAC) {
						for (nf = 0; nf < bm->K_num_fisheries; nf++) {
							flagTACparticipate = (int) (bm->FISHERYprms[nf][flagTACpartipcate_id]);
							if (flagTACparticipate) {
								bm->TAC_over[sp][nf][0] = 1;
								//fprintf(llogfp,"time: %e, Cumulative catch of %s (%e) is over common pool TAC (%e) for fisheries including %s\n", bm->dayt, FunctGroupArray[sp].groupCode, bm->TotCumCatch[sp][fishery_id][bm->thisyear], TAC, FisheryArray[fishery_id].fisheryCode);
								taccount++;
							}
						}
					}
				}
			}

            /*
			if (do_debug) {
				fprintf(llogfp, "Time: %e %s comon pool TAC taccount: %d, max_sp: %d\n", bm->dayt, FisheryArray[fishery_id].fisheryCode, taccount, max_sp);
                printf("Time: %e %s comon pool TAC taccount: %d, max_sp: %d\n", bm->dayt, FisheryArray[fishery_id].fisheryCode, taccount, max_sp);
			}
             */
            
			if (taccount >= max_sp) {
				baseEFF_scale1 = 0.0;
				if (!bm->FISHERYprms[fishery_id][closenotice_id]) {
					fprintf(llogfp, "time: %e, due to number of species over TAC (%d vs max_allowed %d) fishery %s closed\n", bm->dayt, taccount, max_sp,
							FisheryArray[fishery_id].fisheryCode);
					bm->FISHERYprms[fishery_id][closenotice_id] = 1;
				}
			}
        } else {
			baseEFF_scale1 = 1.0;
        }

		if (sp_basketTAC_mgmt) {
			baskEFF_scale1 = 0.0;
			quit("No code for baskets and cumulative TAC at present - possible if apply TAC per fishery rather than common pool\n");
        } else {
			baskEFF_scale1 = 1.0;
        }

		if (cumTripLim_mgmt || sp_regionalTAC_mgmt) {
			taccount = 0;
			for (sp = 0; sp < bm->K_num_tot_sp; sp++) {
				if (FunctGroupArray[sp].isTAC) {
					sp_reg = (int) (FunctGroupArray[sp].speciesParams[regionalSP_id]);

                    if (bm->manage_reg) {
						nstock = bm->K_num_active_reg;
                    } else {
						nstock = FunctGroupArray[sp].numStocks;
                    }

                    if (!sp_reg && (nstock < 1)) {
                        /* Only include those groups under regional TACs with seperate stocks */
						continue;
                    }

					for (nreg = 0; nreg < nstock; nreg++) {
						/* Get regional scaling */
						regTAC_scale = bm->RegionalData[sp][nreg][reg_tac_id];
						regC_scale = bm->RegionalData[sp][nreg][reg_catch_id];

						TAC = 0;
						CumCatch = 0;
						for (nf = 0; nf < bm->K_num_fisheries; nf++) {
							flagTACparticipate = (int) (bm->FISHERYprms[nf][flagTACpartipcate_id]);
							if (flagTACparticipate) {
								/* Convert from tonnes wet weight to mg N */
								TAC += (regTAC_scale * bm->TACamt[sp][nf][now_id] * kg_2_mg) / bm->X_CN;
								CumCatch += (Harvest_Get_TotCumCatch(sp, nf, bm->thisyear) + bm->TotOldCumCatch[sp][nf]) * regC_scale;
							}
						}
						if (CumCatch > TAC) {
							for (nf = 0; nf < bm->K_num_fisheries; nf++) {
								flagTACparticipate = (int) (bm->FISHERYprms[nf][flagTACpartipcate_id]);
								if (flagTACparticipate) {
									bm->TAC_over[sp][nf][nreg] = 1;

									/*
									if (do_debug) {
										double totCumCatch = Harvest_Get_TotCumCatch(sp, nf, bm->thisyear);
										fprintf(llogfp, "time: %e, Cumulative regional catch of %s (%e) is over TAC (%e) for fishery %s in region %d\n",
												bm->dayt, FunctGroupArray[sp].groupCode, ((totCumCatch + bm->TotOldCumCatch[sp][fishery_id])* regC_scale), TAC, FisheryArray[fishery_id].fisheryCode,
												nreg);

									}
                                    */
								}
							}
						}
					}
				}
			}
			/* Check for complete fishery closure everywhere */
			for (sp = 0; sp < bm->K_num_tot_sp; sp++) {
				if (FunctGroupArray[sp].isTAC) {
					regtaccount = 0;
					for (nreg = 0; nreg < FunctGroupArray[sp].numStocks; nreg++) {
						if (bm->TAC_over[sp][fishery_id][nreg])
							regtaccount++;
					}
					if (regtaccount == nstock) {
						taccount++;
					}
				}
			}
			max_sp = (int) (bm->FISHERYprms[fishery_id][max_num_sp_id]);

            /*
			if (do_debug) {
				fprintf(llogfp, "Time: %e %s comgmt taccount: %d, max_sp: %d\n", bm->dayt, FisheryArray[fishery_id].fisheryCode, taccount, max_sp);
                printf("Time: %e %s comgmt taccountC: %d, max_sp: %d closenotice: %e\n", bm->dayt, FisheryArray[fishery_id].fisheryCode, taccount, max_sp, bm->FISHERYprms[fishery_id][closenotice_id]);
			}
             */

			if (taccount >= max_sp) {
				regEFF_scale1 = 0.0;
				if (!bm->FISHERYprms[fishery_id][closenotice_id]) {
					fprintf(llogfp, "time: %e, due to number of regional species over TAC (%d vs max_allowed %d) in every region fishery %s closed\n",
							bm->dayt, taccount, max_sp, FisheryArray[fishery_id].fisheryCode);
                    
                    printf("time: %e, due to number of regional species over TAC (%d vs max_allowed %d) in every region fishery %s closed\n",
                            bm->dayt, taccount, max_sp, FisheryArray[fishery_id].fisheryCode);
                    
					bm->FISHERYprms[fishery_id][closenotice_id] = 1;
				}
				if (cumTripLim_mgmt) {
					/* NOTE: For US style cumulative trip limits (related to overall TAC) that trigger area closures,
					 the TAC_check equivalent occurs in US_TripLimMgmt() NOT here - this is because it needs to
					 play with the MPAlist and because it is across many fleets and it only happens at council meetings
					 (with delays). This makes it difficult to mesh in here, so simply dealt with separately.
					 */
					regEFF_scale1 = 1.0;
				}
            } else {
				regEFF_scale1 = 1.0;
            }
        } else {
			regEFF_scale1 = 1.0;
        }
	}

	EFF_scale1 = (baseEFF_scale1 * baskEFF_scale1 * regEFF_scale1);

    /*
	if (do_debug) {
		fprintf(llogfp, "Time: %e, %s has EFF_scale1: %e, baseEFF_scale1: %e, baskEFF_scale1: %e, regEFF_scale1: %e\n", bm->dayt,
				FisheryArray[fishery_id].fisheryCode, EFF_scale1, baseEFF_scale1, baskEFF_scale1, regEFF_scale1);
        printf("Time: %e, %s has EFF_scale1: %e, baseEFF_scale1: %e, baskEFF_scale1: %e, regEFF_scale1: %e\n", bm->dayt,
                FisheryArray[fishery_id].fisheryCode, EFF_scale1, baseEFF_scale1, baskEFF_scale1, regEFF_scale1);
	}
    */
    
	return EFF_scale1;
}

/**
 * \brief Endangered species checking
 *
 * @return  Returns a scalar to adjust realised effort by to take into account Protected-Endangered-Threatened species issues
 */
double Endangered_Check(MSEBoxModel *bm, int fishery_id, double EFF_scale1, int *end_trigger_tripped, FILE *llogfp) {
	double EFF_scale2 = 1.0;

	if (bm->flagendangered) {
		/* Adaptive management responds to the size of the stock and any
		 charismatic or endangered species of concern. In each case if population
		 has fallen to critical % of virgin stock size reduce effort
		 (done gradually over a specified time period rather than as a sudden drop).
		 Once again assessment made in Annual_Fisheries_Mgmt() and the result is
		 applied here*/
		EFF_scale2 = effort_scale[fishery_id][endangered_tac_id];
		if (EFF_scale2 != 1.0) {
			*end_trigger_tripped = 1;
			/* If first time for the year record the fact */
			if (effort_scale[fishery_id][endangered_message_id]) {
				fprintf(llogfp, "time: %e, endangered species trigger set off by fishery %s\n", bm->dayt, FisheryArray[fishery_id].fisheryCode);
				effort_scale[fishery_id][endangered_message_id] = 0;
			}
        } else {
			*end_trigger_tripped = 0;
        }

		/* Check to see if effort already sufficiently reduced or whether
		 more reductions are necessary due to pressure on endangered groups */
		if (EFF_scale1 < EFF_scale2) {
			EFF_scale2 = 1.0;
        } else {
			/* As multiplicative adjustments must make sure adjust this contribution
			 so in combination with any stock inspired drops you reach the right spot */
			EFF_scale2 = EFF_scale2 / (EFF_scale1 + small_num);
        }
    } else {
		EFF_scale2 = 1.0;
    }

	return EFF_scale2;
}

/**
 * \brief Institute perscribed effort changes. Note don't need to correct end_date to seconds
 * here as bm->dayt is in days too
 *
 * @return Returns scalar scaling effort to represent "drawn scenario" changes
 */
double Forced_Effort_Change(MSEBoxModel *bm, int fishery_id, int flagspeffortmodel, FILE *llogfp) {
	int flagspchangeEFF, EFF_num_changes, i, past_change, flagpulsefish, now_change = 0, flag_use_preveffortmodel;
	double step1, EFFperiod, EFFmultB, EFFmultA, EFFstart, end_date;
	double EFF_scale3 = 1.0;

	flagspchangeEFF = (int) (bm->FISHERYprms[fishery_id][flagchangeEFF_id]);
	flag_use_preveffortmodel = (int) (bm->FISHERYprms[fishery_id][flaguse_preveffort_id]);
	EFF_num_changes = (int) (bm->FISHERYprms[fishery_id][EFF_num_changes_id]);
	if (flagspchangeEFF && (bm->dayt >= EFFORTchange[fishery_id][0][start_id]) && (EFFORTchange[fishery_id][0][start_id] != 0)) {
		for (i = 0; i < EFF_num_changes; i++) {
			if (bm->dayt >= EFFORTchange[fishery_id][i][start_id])
				now_change = i;
		}
		end_date = (EFFORTchange[fishery_id][now_change][start_id] + EFFORTchange[fishery_id][now_change][period_id]);
		past_change = now_change - 1;
		EFFmultA = EFFORTchange[fishery_id][now_change][mult_id];
		if (now_change > 0)
			EFFmultB = EFFORTchange[fishery_id][past_change][mult_id];
		else
			EFFmultB = 1;
		EFFstart = EFFORTchange[fishery_id][now_change][start_id];
		EFFperiod = EFFORTchange[fishery_id][now_change][period_id] + small_num;

		/* For those fisheries which depend on previous effort to calculate new effort
		 each little change in effort is cumulative */
		if (flag_use_preveffortmodel) {
			if (end_date < bm->dayt) {
				/* Change over */
				flagpulsefish = (int) (EFFORTchange[fishery_id][now_change][pulse_id]);
				if (flagpulsefish && (end_date < bm->dayt + 1.0)) {
					/* If only pulse of change, divide by level of change to return to
					 normal for fisheries dependent on previous effort levels - correction only needed for the first day after the end of the pulse */
					EFF_scale3 = 1.0 / EFFmultA;
				} else {
					/* Otherwise change is complete no more scaling necessary */
					EFF_scale3 = 1.0;
				}
			} else {
				/* Change still underway - increment change in effort only for
				 that due to current day */
				if (EFFmultA >= EFFmultB) {
					step1 = prev_mult[fishery_id] + ((EFFmultA - EFFmultB) / EFFmultB) / (EFFperiod * (86400.0 / bm->dt));
					EFF_scale3 = step1 / prev_mult[fishery_id];
					prev_mult[fishery_id] = step1;
					flagdropeffort[fishery_id] = 0;
				} else {
					step1 = prev_mult[fishery_id] - ((EFFmultB - EFFmultA) / EFFmultB) / (EFFperiod * (86400.0 / bm->dt));
					EFF_scale3 = step1 / prev_mult[fishery_id];
					prev_mult[fishery_id] = step1;
					flagdropeffort[fishery_id] = 1;
				}
			}
		} else {
			/* For those fisheries using simple perscribed effort models original effort levels
			 are simply scaled to reach new levels at each timestep, there is no cumulative
			 change everything remains relative to the original ("baseline") level */
			if (end_date < bm->dayt) {
				/* Change over */
				flagpulsefish = (int) (EFFORTchange[fishery_id][now_change][pulse_id]);
				if (flagpulsefish) {
					/* If only a pulse of change then remove scaling of simple effort to
					 return to normal */
					EFF_scale3 = 1.0;
				} else {
					/* Otherwise change is complete is simply multiple by final level of
					 change desired */
					EFF_scale3 = EFFmultA;
				}
			} else {
				/* Change still underway - calculate scaling based on time elapsed
				 since change started */
				if (EFFmultA >= EFFmultB) {
					EFF_scale3 = EFFmultB + (EFFmultA - EFFmultB) * (bm->dayt - EFFstart) / EFFperiod;
					flagdropeffort[fishery_id] = 0;
				} else {
					EFF_scale3 = EFFmultB - (EFFmultB - EFFmultA) * (bm->dayt - EFFstart) / EFFperiod;
					flagdropeffort[fishery_id] = 1;
				}
			}
		}
	} else {
		EFF_scale3 = 1.0;
		EFFperiod = 1.0;
		flagdropeffort[fishery_id] = (int) (bm->FISHERYprms[fishery_id][flagdropEFF_id]);
	}

	return EFF_scale3;
}

/**
 * \brief Seasonal fisheries checking
 *
 *	@return Returns a scalar to adjust realised effort to take into account seasonal issues
 */
double Season_Check(MSEBoxModel *bm, int fishery_id, int ij, FILE *llogfp) {
	int flagseasonal, nreg, use_reg;
	double fishstartday, fishendday;
	double EFF_scale4 = 1.0;

	flagseasonal = (int) (bm->FISHERYprms[fishery_id][flagseasonal_id]);

	if (bm->K_num_active_reg > 1) {
		nreg = bm->regID[ij];
		use_reg = bm->reg_season[nreg];
	} else
		use_reg = 1;

	if (flagseasonal && use_reg) {
		fishstartday = SEASONAL[fishery_id][0];
		fishendday = SEASONAL[fishery_id][1];
		if (fishstartday < fishendday) {
			/* The entire season is within in a single calendar year */
			if ((bm->TofY >= fishstartday) && (bm->TofY <= fishendday))
				EFF_scale4 = 1.0;
			else
				EFF_scale4 = 0.0;
		} else {
			/* Or the season runs over from one year into the next */
			if ((bm->TofY > fishendday) && (bm->TofY < fishstartday))
				EFF_scale4 = 0.0;
			else
				EFF_scale4 = 1.0;
		}
	} else
		EFF_scale4 = 1.0;

	return EFF_scale4;
}

/**
 * \brief Effort determination and allocation
 *
 * @return Returns fishing pressure in box ij
 */
double Allocate_Immediate_Effort(MSEBoxModel *bm, int fishery_id, int ij, int flagspeffortmodel, double prop_pop_fish, FILE *llogfp) {

	int porti, chkbox, adjacent_to_effort, flagexplore, sp, nb;
    //int do_debug;
	double mEff_offset, mEffscale, mEff_max, mEff_a, FCweight, tempFCpressure, boxarea, FCtestpressure, maxdepth_constraint, ntarget, totFCweight, FCpressure,
			grow_effort, current_CPUE, CPUEshift, port_bit, cpue_bit, CPUE_and_port, mindepth_constraint;
	double active_scale = 86400.0 / bm->dt;
	double minday = 1.0;
	int ncells = bm->nbox - bm->nboundary - bm->nland;  // So only counting dynamic boxes

	// So no body trying to do anything but basic effort on day 1
	if(bm->tburnday < 1.0)
		minday = 1.0;
	else
		minday = bm->tburnday;

    /*
	if ((((bm->debug == debug_effort) || (bm->debug == debug_econeffort)) && ((bm->which_fleet == fishery_id) || (bm->which_fleet >= bm->K_num_fisheries)))
			&& (bm->dayt > bm->checkstart)) {
		do_debug = 1;
	} else {
		do_debug = 0;
     }
    */
    
	boxarea = bm->boxes[ij].area;
	flagexplore = (int) (bm->FISHERYprms[fishery_id][flagexplore_id]);
	mindepth_constraint = bm->FISHERYprms[fishery_id][minFCdepth_id];
	maxdepth_constraint = bm->FISHERYprms[fishery_id][maxFCdepth_id];

	/* Fishing effort - this is where effort management should be dealt with */
	FCweight = 1.0;
	switch (flagspeffortmodel) {
	case const_effort: /* Constant fishing pressure per quarter per cell
	 (evenly distributed across cells) */
		FCpressure = (bm->HowFar * (mEff[fishery_id][bm->NextQofY] - mEff[fishery_id][bm->QofY]) + mEff[fishery_id][bm->QofY]) * bm->FISHERYprms[fishery_id][EffortLevel_id] / ncells;
		break;
	case const_qrt_effort: /* Constant fishing pressure per quarter per cell (adjusted by relative
	 area of the cell) */
		FCpressure = (bm->HowFar * (mEff[fishery_id][bm->NextQofY] - mEff[fishery_id][bm->QofY]) + mEff[fishery_id][bm->QofY]) * (boxarea / tot_dyn_sea_area)
				* bm->FISHERYprms[fishery_id][EffortLevel_id];
		break;
	case cpue_effort: /* bm->Effort scaled based on location of previous day's cpue */
		if (bm->dayt < minday) {
			FCpressure = bm->Effort_hdistrib[ij][fishery_id][bm->QofY] * bm->FISHERYprms[fishery_id][EffortLevel_id];
		} else {
			FCpressure = (bm->HowFar * (mEff[fishery_id][bm->NextQofY] - mEff[fishery_id][bm->QofY]) + mEff[fishery_id][bm->QofY])
					* (bm->TempCPUE[ij][fishery_id] / (bm->totCPUE[fishery_id] + small_num)) * bm->FISHERYprms[fishery_id][EffortLevel_id];

			/* Assumes some effort should be happening so make sure it ticks over */
			if (!bm->totCPUE[fishery_id])
				FCpressure = bm->FISHERYprms[fishery_id][EffortLevel_id] / (ncells + small_num);

			/* Once a year do exploratory fish - in case low stocks have seen fishery
			 close and now no CPUE to base dynamics on */
			if (flagexplore && (bm->boxes[ij].botz < maxdepth_constraint) && (bm->boxes[ij].botz > mindepth_constraint)) {
				if (bm->TofY == 0 && bm->totOldEffort[fishery_id])
					FCtestpressure = bm->FISHERYprms[fishery_id][mEff_testfish_id];
				else
					FCtestpressure = 1.0;

				if (!FCpressure)
					FCpressure = FCtestpressure;
			}
		}

        /*
		if (do_debug) {
			fprintf(llogfp, "Time: %e %s box %d, FCpressure: %e, level: %e, cpue_ratio: %e (TempCPUE: %e totCPUE: %e), mEff: %e\n", bm->dayt,
					FisheryArray[fishery_id].fisheryCode, ij, FCpressure, bm->FISHERYprms[fishery_id][EffortLevel_id], (bm->TempCPUE[ij][fishery_id]
							/ (bm->totCPUE[fishery_id] + small_num)), bm->TempCPUE[ij][fishery_id], bm->totCPUE[fishery_id], ((bm->HowFar
							* (mEff[fishery_id][bm->NextQofY] - mEff[fishery_id][bm->QofY]) + mEff[fishery_id][bm->QofY])));
		}
        */

		break;
	case array_effort: /* Prescribed spatial effort matrices */
		if (bm->QofY < 3)
			FCpressure = bm->HowFar * (bm->Effort_hdistrib[ij][fishery_id][bm->QofY + 1] - bm->Effort_hdistrib[ij][fishery_id][bm->QofY])
					+ bm->Effort_hdistrib[ij][fishery_id][bm->QofY];
		else
			FCpressure = bm->HowFar * (bm->Effort_hdistrib[ij][fishery_id][0] - bm->Effort_hdistrib[ij][fishery_id][bm->QofY])
					+ bm->Effort_hdistrib[ij][fishery_id][bm->QofY];
        FCpressure *= (bm->HowFar * (mEff[fishery_id][bm->NextQofY] - mEff[fishery_id][bm->QofY]) + mEff[fishery_id][bm->QofY]);  // Add so consistent with the other const effort options
		FCpressure *= bm->FISHERYprms[fishery_id][EffortLevel_id];

        /*
		if (do_debug) {
			fprintf(llogfp, "Time: %e, %s %d FCpressure: %e (Effort_hdistrib: %e and Level: %e)\n", bm->dayt, FisheryArray[fishery_id].fisheryCode, ij,
					FCpressure, bm->Effort_hdistrib[ij][fishery_id][bm->QofY], bm->FISHERYprms[fishery_id][EffortLevel_id]);
		}
        */

		break;
	case wgt_effort: /* Weights effort based on effort of previous day */
		mEff_max = bm->FISHERYprms[fishery_id][mEff_max_id];
		mEff_a = bm->FISHERYprms[fishery_id][mEff_a_id];
		mEff_offset = bm->FISHERYprms[fishery_id][mEff_offset_id];
		tempFCpressure = mEff_max / (1.0 + exp(-mEff_a * bm->TempCPUE[ij][fishery_id])) - mEff_offset;

		// TODO: This should perhaps be a min() not max()
		FCpressure = max(1.0,(bm->Speed_boat * bm->dt / bm->width)) * (tempFCpressure - bm->OldEffort[ij][fishery_id]) + bm->OldEffort[ij][fishery_id];
            
        /* Once a year do exploratory fish */
        if (flagexplore && (bm->boxes[ij].botz < maxdepth_constraint) && (bm->boxes[ij].botz > mindepth_constraint)) {
            if (bm->TofY == 0 && bm->totOldEffort[fishery_id])
                FCtestpressure = bm->FISHERYprms[fishery_id][mEff_testfish_id];
            else
                FCtestpressure = 1.0;
                
            if (!FCpressure)
                FCpressure = FCtestpressure;
        }

		break;
	case dist_effort: /* Fleet dynamics model based on - distance to port considered in the calculations */
		/* For the burn-in use some inital distributions */
		if (bm->dayt < minday) {
			FCpressure = bm->Effort_hdistrib[ij][fishery_id][bm->QofY] * bm->FISHERYprms[fishery_id][EffortLevel_id];
		} else {
			mEff_max = bm->FISHERYprms[fishery_id][mEff_max_id];
			mEff_a = bm->FISHERYprms[fishery_id][mEff_a_id];
			mEff_offset = bm->FISHERYprms[fishery_id][mEff_offset_id];
			tempFCpressure = mEff_max / (1.0 + exp(-mEff_a * bm->TempCPUE[ij][fishery_id])) - mEff_offset;

			/* Get relative contribution from each port */
			FCweight = bm->PortContrib[fishery_id][ij][pvalue_id];

			/* Take distance based weights and previous days CPUE distribution
			 and use these to weight new effort allocation */
			FCpressure = FCweight * (tempFCpressure - bm->OldEffort[ij][fishery_id]) + bm->OldEffort[ij][fishery_id];

			/* Once a year do exploratory fish - in case low stocks have seen fishery
			 close and now no CPUE to base dynamics on */
			if (flagexplore && (bm->boxes[ij].botz < maxdepth_constraint) && (bm->boxes[ij].botz > mindepth_constraint)) {
				if (bm->TofY == 0 && bm->totOldEffort[fishery_id])
					FCtestpressure = bm->FISHERYprms[fishery_id][mEff_testfish_id];
				else
					FCtestpressure = 1.0;

				if (!FCpressure)
					FCpressure = FCtestpressure;
			}

		}
		break;
	case rec_effort: /* Recreational fishing where effort is based on the size of the human population */
	case rec_econ_model: /* Recreational fishing effort model - when using economic drivers */
		FCweight = 0;
		for (porti = 0; porti < bm->K_num_ports; porti++) {
			/* No check if active here as needn't be a commerical port to have recreational fishing pressure */

			/* Get population of port fishing */
			mEffscale = prop_pop_fish * bm->Port_info[porti][popactive_id];

			/* If an active port than determine contribution to effort from that port
			 use the scalar mEffscale to heighten or lessen effects of distance
			 - so can also capture social and economic forces which may push
			 affect a sites attractiveness */
			FCweight += mEffscale * bm->Speed_recboat * bm->dt / bm->Port_distance[porti][ij];
		}
		/* Use population-based effort weight to scale effort in coastal boxes */
		if (bm->QofY < 3)
			FCpressure = FCweight * (bm->HowFar * (bm->Effort_hdistrib[ij][fishery_id][bm->QofY + 1] - bm->Effort_hdistrib[ij][fishery_id][bm->QofY])
					+ bm->Effort_hdistrib[ij][fishery_id][bm->QofY]);
		else
			FCpressure = FCweight * (bm->HowFar * (bm->Effort_hdistrib[ij][fishery_id][0] - bm->Effort_hdistrib[ij][fishery_id][bm->QofY])
					+ bm->Effort_hdistrib[ij][fishery_id][bm->QofY]);
		FCpressure *= bm->FISHERYprms[fishery_id][EffortLevel_id];

        /*
		if (do_debug) {
			fprintf(llogfp, "Time: %e, %s-%d, FCpressure: %e, FCweight: %e, EffortLevel: %e, hdistrib: %e\n", bm->dayt, FisheryArray[fishery_id].fisheryCode,
					ij, FCpressure, FCweight, bm->FISHERYprms[fishery_id][EffortLevel_id], bm->Effort_hdistrib[ij][fishery_id][bm->QofY]);
		}
        */
            
		break;
	case alt_cpue_effort: /* Alternative fleet dynamics model - effort distribution is drawn
	 toward CPUE distribution of previous day */
		/* For the burn-in use some inital distributions */
		if (bm->dayt < minday) {
			FCpressure = bm->Effort_hdistrib[ij][fishery_id][bm->QofY] * bm->FISHERYprms[fishery_id][EffortLevel_id];
		} else {
			tempFCpressure = (bm->TempCPUE[ij][fishery_id] / (bm->totCPUE[fishery_id] + small_num)) * bm->totOldEffort[fishery_id];

			/* Get relative contribution from each port */
			FCweight = bm->PortContrib[fishery_id][ij][pvalue_id];

			/* Take distance based weights and previous days CPUE distribution
			 and use these to weight new effort allocation */
			FCpressure = FCweight * (tempFCpressure - bm->OldEffort[ij][fishery_id]) + bm->OldEffort[ij][fishery_id];

			/* Once a year do exploratory fish - in case low stocks have seen fishery
			 close and now no CPUE to base dynamics on */
			if (flagexplore && (bm->boxes[ij].botz < maxdepth_constraint) && (bm->boxes[ij].botz > mindepth_constraint)) {
				if (bm->TofY == 0 && !FCpressure) {
					if (bm->totOldEffort[fishery_id])
						FCtestpressure = bm->FISHERYprms[fishery_id][mEff_testfish_id];
					else
						FCtestpressure = 1.0;

                    if (!FCpressure)
                        FCpressure = FCtestpressure;
				}
			}
            /*
			if (do_debug) {
				fprintf(llogfp, "\nFCpressure: %e, FCweight: %e, tempFCpressure: %e, bm->OldEffort[%d][%d]: %e, bm->TempCPUE/bm->totCPUE: %e\n", FCpressure,
						FCweight, tempFCpressure, ij, fishery_id, bm->OldEffort[ij][fishery_id], bm->TempCPUE[ij][fishery_id] / bm->totCPUE[fishery_id]);
			}
            */
		}
		break;
	case alt_cpue_scale_effort:
		/* Alternative fleet dynamics model weighted by prescribed
		 spatial effort matrices and CPUE distribution of previous day */
		/* For the burn-in period use some initial distributions */
		if (bm->dayt < minday) {
			FCpressure = bm->Effort_hdistrib[ij][fishery_id][bm->QofY] * bm->FISHERYprms[fishery_id][EffortLevel_id];

            /*
			if (do_debug) {
				fprintf(llogfp, "Time: %e, %s-%d, FCpressure: %e, hdistrib: %e, EffortLevel: %e\n", bm->dayt, FisheryArray[fishery_id].fisheryCode, ij,
						FCpressure, bm->Effort_hdistrib[ij][fishery_id][bm->QofY], bm->FISHERYprms[fishery_id][EffortLevel_id]);
			}
            */
		} else {
			/** Old code - no longer used as fleet movement model verified **
			 if(bm->pseudo_assess && !bm->dynanyway){
			 // If using pseudo assessment then use this as fast proxy until can double check fleet movement model *
			 das_effort = bm->totOldEffort[fishery_id];
			 if(das_effort < (bm->FISHERYprms[fishery_id][reset_id] * (1.0 - bm->FISHERYprms[fishery_id][CPUE_effort_scale_id]) * bm->FISHERYprms[fishery_id][EffortLevel_id]))
			 das_effort = bm->FISHERYprms[fishery_id][reset_id] * (1.0 - bm->FISHERYprms[fishery_id][CPUE_effort_scale_id]) * bm->FISHERYprms[fishery_id][EffortLevel_id];
			 FCpressure = bm->Effort_hdistrib[ij][fishery_id][bm->QofY] * das_effort;

			 if (do_debug){
             fprintf(llogfp,"Time: %e, %s-%d, das_effort: %e, reset: %e, CPUE_effort_scale: %e, EffortLevel: %e, oldeffort: %e\n",
			 bm->dayt, FisheryArray[fishery_id].fisheryCode, ij, das_effort, bm->FISHERYprms[fishery_id][reset_id], bm->FISHERYprms[fishery_id][CPUE_effort_scale_id], bm->FISHERYprms[fishery_id][EffortLevel_id], bm->totOldEffort[fishery_id]);
			 }
			 } else {
			 **/
			/** Relic code of some original steps involved - now use compound step calculated in CalcPortContrib
			 tempFCpressure *= bm->Effort_hdistrib[ij][fishery_id][bm->QofY];
			 tempFCpressure = (bm->TempCPUE[ij][fishery_id]/(bm->totCPUE[fishery_id] + small_num)) * bm->totOldEffort[fishery_id] * active_scale;
			 FCweight = bm->PortContrib[fishery_id][ij][pvalue_id] / (bm->totPortContrib[fishery_id][simple_id] + small_num);
			 **/

			/* Get relative port and CPUE contributions */
			port_bit = bm->PortContrib[fishery_id][ij][pvalue_id] / (bm->totPortContrib[fishery_id][simple_id] + small_num);
			cpue_bit = bm->TempCPUE[ij][fishery_id] / (bm->totCPUE[fishery_id] + small_num);

			CPUE_and_port = port_bit * cpue_bit;

			/* Calculate new potential effort distribution */
			tempFCpressure = (CPUE_and_port / (bm->totPortContrib[fishery_id][compound_id] + small_num)) * bm->totOldEffort[fishery_id];

			/* Get speed weighting so can't teleport from one distribution to the next,
			 based around distance between peaks of old and new distributions

			 Trying to capture steaming time - in crude way admittedly
			 */
			FCweight = bm->Speed_boat * bm->dt / (DistPeak[fishery_id] + small_num);
			if (FCweight > 1.0)
				FCweight = 1.0;

			/* Take distance based weights and use these to interpolate from the old to the new effort allocations */
			FCpressure = FCweight * (tempFCpressure - bm->OldEffort[ij][fishery_id]) + bm->OldEffort[ij][fishery_id];
			FCweight = bm->Speed_boat * bm->dt / (DistPeak[fishery_id] + small_num);

			/* Multiply by active_scale so that get it in terms of whole days as will rescale to dt level later */
			FCpressure *= active_scale;

			/* On about trip-length scale (i.e. weekly) rescale effort based on whether investment/disinvestment
			 has been triggered by CPUE rates
			 */
			grow_effort = 1.0;
			if (bm->newweek) {
				current_CPUE = bm->totCPUE[fishery_id] * bm->X_CN * mg_2_kg;
				CPUEshift = bm->FISHERYprms[fishery_id][mEff_shift_id] / 52.0;
				// Correct to weekly incremental change or get explosive spirals
				if (current_CPUE > bm->FISHERYprms[fishery_id][mEff_thresh_top_id]) {
					/* Buy in new effort */
					grow_effort = 1.0 + CPUEshift;

					//fprintf(llogfp,"Time: %e, Box%d growing effort by %e (cpue: %e vs thresh: %e)\n",
					//	bm->dayt, ij, grow_effort, current_CPUE, bm->FISHERYprms[fishery_id][mEff_thresh_top_id]);

				} else if (current_CPUE < bm->FISHERYprms[fishery_id][mEff_thresh_id]) {
					/* Sell out of fishery - or tie up longer - and reduce effort */
					grow_effort = 1.0 - CPUEshift;
					if (grow_effort < 0.0)
						grow_effort = 0.0;

					//fprintf(llogfp,"Time: %e, Box%d shrinking effort by %e (cpue: %e vs thresh: %e)\n",
					//	bm->dayt, ij, grow_effort, current_CPUE, bm->FISHERYprms[fishery_id][mEff_thresh_id]);

				}
			}
			FCpressure *= grow_effort;

			/* If a fishery that explores and at appropriate depths then once
			 a year do exploratory fish - in case low stocks have seen fishery
			 close and now no CPUE to base dynamics on */
			if (flagexplore && (bm->boxes[ij].botz < maxdepth_constraint) && (bm->boxes[ij].botz > mindepth_constraint)) {
				if (bm->TofY == 0 && !FCpressure) {
					if (bm->totOldEffort[fishery_id])
						FCtestpressure = bm->FISHERYprms[fishery_id][mEff_testfish_id];
					else
						FCtestpressure = 1.0;

                    if (!FCpressure)
                        FCpressure = FCtestpressure;
				}
			}
            /*
			if (do_debug) {
				fprintf(
						llogfp,
						"time: %e, %s %d FCpressure: %e, scalar: %e, FCweight: %e, tempFCpressure: %e, bm->OldEffort: %e, grow_effort: %e, CPUE_and_port: %e, totPortContrib: %e, totOldEffort: %e, active_scale: %e\n",
						bm->dayt, FisheryArray[fishery_id].fisheryCode, ij, FCpressure, (grow_effort * port_bit * cpue_bit
								/ (bm->totPortContrib[fishery_id][compound_id] + small_num)), FCweight, tempFCpressure, bm->OldEffort[ij][fishery_id],
						grow_effort, CPUE_and_port, bm->totPortContrib[fishery_id][compound_id], bm->totOldEffort[fishery_id], active_scale);
			}
             */
		}
		//}		Relic for old if statement above
		break;
	case ideal_distrib_effort: /* bm->Effort scaled based on ideal knowledge of target fish distributions */
		FCweight = 0;
        /* For the burn in period use the iniital distributions */
		if (bm->dayt < minday) {
			FCpressure = bm->Effort_hdistrib[ij][fishery_id][bm->QofY] * bm->FISHERYprms[fishery_id][EffortLevel_id];

            /*
			if (do_debug) {
				fprintf(llogfp, "Time: %e, %s-%d, FCpressure: %e, hdistrib: %e, EffortLevel: %e\n", bm->dayt, FisheryArray[fishery_id].fisheryCode, ij,
						FCpressure, bm->Effort_hdistrib[ij][fishery_id][bm->QofY], bm->FISHERYprms[fishery_id][EffortLevel_id]);
			}
            */

		} else {
			/* Get distribution based on distribution of target groups */
			FCweight = 0;
			totFCweight = 0;
			ntarget = 0;
			for (sp = 0; sp < bm->K_num_tot_sp; sp++) {
				if (FunctGroupArray[sp].isFished == TRUE) {
					if (bm->FISHERYtarget[fishery_id][sp]) {
						FCweight += bm->targetspbiom[sp][ij] / (bm->totbiom[sp] + small_num);
						totFCweight += bm->totbiom[sp];
						ntarget++;

						/*
						 if(do_debug){
						 fprintf(llogfp,"Time %e %s in %d - sp %s contrib: %e, ntarget %e\n", bm->dayt, FisheryArray[fishery_id].fisheryCode, ij, FunctGroupArray[guild].groupCode[spcat], bm->targetspbiom[sp][ij] / (bm->totbiom[spcat] + small_num), ntarget);
						 }
						 */
					}
				}
			}
			FCweight /= ntarget;

			/* If no target left then may be targeting by-product so distrbute based on perscribed distributions */
			if (!totFCweight) {
				FCweight = (bm->HowFar * (bm->Effort_hdistrib[ij][fishery_id][bm->QofY + 1] - bm->Effort_hdistrib[ij][fishery_id][bm->QofY])
						+ bm->Effort_hdistrib[ij][fishery_id][bm->QofY]);
			}

			FCpressure = FCweight * bm->FISHERYprms[fishery_id][EffortLevel_id];

			/* Once a year do exploratory fish - in case low stocks have seen fishery
			 close and now no CPUE to base dynamics on */
			if (flagexplore && (bm->boxes[ij].botz < maxdepth_constraint) && (bm->boxes[ij].botz > mindepth_constraint)) {
				if (bm->TofY == 0 && bm->totOldEffort[fishery_id])
					FCtestpressure = bm->FISHERYprms[fishery_id][mEff_testfish_id];
				else
					FCtestpressure = 1.0;

				if (!FCpressure)
					FCpressure = FCtestpressure;
			}

            /*
			if (do_debug) {
				fprintf(llogfp, "time: %e, %s %d FCpressure: %e, FCweight: %e, EffortLevel: %e\n", bm->dayt, FisheryArray[fishery_id].fisheryCode, ij,
						FCpressure, FCweight, bm->FISHERYprms[fishery_id][EffortLevel_id]);
			}
            */
		}
		break;
	case cpue_dyneffort: /* bm->Effort scaled based on location of previous day's cpue - except during the burn-in period */
		if (bm->dayt < minday) {
			FCpressure = bm->Effort_hdistrib[ij][fishery_id][bm->QofY] * bm->FISHERYprms[fishery_id][EffortLevel_id];
		} else {
			FCpressure = (bm->HowFar * (mEff[fishery_id][bm->NextQofY] - mEff[fishery_id][bm->QofY]) + mEff[fishery_id][bm->QofY])
					* (bm->TempCPUE[ij][fishery_id] / (bm->totCPUE[fishery_id] + small_num)) * bm->totOldEffort[fishery_id];

			/* Once a year do exploratory fish - in case low stocks have seen fishery
			 close and now no CPUE to base dynamics on */
			if (flagexplore && (bm->boxes[ij].botz < maxdepth_constraint) && (bm->boxes[ij].botz > mindepth_constraint)) {
				if (bm->TofY == 0 && bm->totOldEffort[fishery_id])
					FCtestpressure = bm->FISHERYprms[fishery_id][mEff_testfish_id];
				else
					FCtestpressure = 1.0;

				if (!FCpressure)
					FCpressure = FCtestpressure;
			}
		}
		break;
	case readts_effort: /* Effort time series read-in */

		FCpressure = 0.0; /* Deal with compiler warnings */
		quit("Don't calculate effort, read it from time series so should never get here, check code path\n");
		break;
	case econ_model_effort: /* Effort time series as calculated in the economics model (in atecon.lib) */
		FCpressure = bm->Effort_hdistrib[ij][fishery_id][today_effort];

		/* If a fishery that explores and at appropriate depths then once a year do exploratory
		 fish - in case low stocks have seen fishery close and now no CPUE to base dynamics
		 on. FIX - This could be made once a quarter.

		 First find if adjacent to a cell fished in previous year. If is nearby then do exploratory fishing.
		 */
		if (flagexplore && (bm->boxes[ij].botz < maxdepth_constraint) && (bm->boxes[ij].botz > mindepth_constraint)) {
//			if (fishery_id == dlineFDE_id)
//				fprintf(llogfp, "Found ground in box %d at depth: %e between %e and %e\n", ij, bm->boxes[ij].botz, maxdepth_constraint, mindepth_constraint);

			if (((bm->TofY == 0) || (bm->newmonth)) && (!FCpressure)) {
				adjacent_to_effort = 0;
				for (chkbox = 0; chkbox < bm->boxes[ij].nconn; chkbox++) {
					nb = bm->boxes[ij].ibox[chkbox];
					if ((bm->OldCumEffort[fishery_id][nb] / (bm->TotOldCumEffort[fishery_id] + small_num)) > bm->explore_thresh_effort) {
						adjacent_to_effort = 1;
					}
				}
				if (adjacent_to_effort)
					FCpressure = bm->FISHERYprms[fishery_id][mEff_testfish_id];
			}
		}

        /*
		if (do_debug) {
			fprintf(llogfp, "Time: %e, %s %d applied FCpressure: %e (Effort_hdistrib: %e vs forced: %e (QofY: %e, EffortLevel: %e))\n", bm->dayt,
					FisheryArray[fishery_id].fisheryCode, ij, FCpressure, bm->Effort_hdistrib[ij][fishery_id][today_effort],
					bm->Effort_hdistrib[ij][fishery_id][bm->QofY] * bm->FISHERYprms[fishery_id][EffortLevel_id], bm->Effort_hdistrib[ij][fishery_id][bm->QofY],
					bm->FISHERYprms[fishery_id][EffortLevel_id]);
		}
        */

		break;
	default:
		FCpressure = 0.0; /* Deal with compiler warnings */
		quit("No such effort model defined (%d) - value must be between 0 and 7 currently\n", flagspeffortmodel);
		break;
	}


	//fprintf(bm->logFile, "%s has flagspeffortmodel = %d, FCpressure = %.20e\n", FisheryArray[fishery_id].fisheryCode, flagspeffortmodel, FCpressure);
	return FCpressure;
}

/**
 * \brief Effort displacement - due to MPA locaiton or depletion of local stocks
 */
void Effort_Displacement(MSEBoxModel *bm, int fishery_id, int ij, double orig_FCpressure, double *FCpressure, double *FCdisplaced, int *new_fish_loc,
		FILE *llogfp) {
	int nb, k, sp;
	double prop_displaced, MPA_check, fishthere, maxfishthere, fishhere, mEff_thresh;
	//int debug = 0;

	/* Initialise displaced effort magnitude and location  */
	*FCdisplaced = 0;
	*new_fish_loc = ij;
	fishhere = 0;

	/*if ((bm->debug == debug_effort) && (bm->dayt >= bm->checkstart) && (bm->dayt < bm->checkstop)) {
		debug = 1;
	}
	*/

	if (bm->flagdisplace) {
		mEff_thresh = bm->FISHERYprms[fishery_id][mEff_thresh_id];
		/* Test to see if CPUE is high enough */
		if (bm->TempCPUE[ij][fishery_id] < mEff_thresh) {
			/* Find target stock in current cell */
			for (sp = 0; sp < bm->K_num_tot_sp; sp++) {
				if (FunctGroupArray[sp].isFished == TRUE) {
					if (bm->FISHERYtarget[fishery_id][sp])
						fishhere += bm->targetspbiom[sp][ij];
				}
			}
			/* See which of the adjacent cells has the highest potential target stock
			 and which have MPAs */
			maxfishthere = 0;
			for (k = 0; k < bm->boxes[ij].nconn; k++) {
				nb = bm->boxes[ij].ibox[k];
				fishthere = 0;
				/* Determine potential stock */
				for (sp = 0; sp < bm->K_num_tot_sp; sp++) {
					if (FunctGroupArray[sp].isFished == TRUE) {
						if (bm->FISHERYtarget[fishery_id][sp]){
							fishthere += bm->targetspbiom[sp][nb];
							/*
							if(debug){
								fprintf(bm->logFile, "Effort_Displacement fishery %s, box %d, bm->targetspbiom[%d][%d] = %e, fishthere = %e\n",
										FisheryArray[fishery_id].fisheryCode, ij, sp, nb, bm->targetspbiom[sp][nb], fishthere);
							}
							*/
						}
					}
				}
				/*
				if(debug){
					fprintf(bm->logFile, "Effort_Displacement fishery %s, box %d, fishthere = %e\n",
						FisheryArray[fishery_id].fisheryCode, ij, fishthere);
				}
				*/
				/* Check MPA status */
				MPA_check = 1.0 - bm->MPA[nb][fishery_id];

				/* Scale stock by MPA access scalar to get final potentail stock */
				fishthere *= MPA_check;

				/* If its the best so far record it */
				if (fishthere > maxfishthere) {
					maxfishthere = fishthere;
					*new_fish_loc = nb;
				}
			}

			/*
			if(debug){
				fprintf(bm->logFile, "Effort_Displacement fishery %s, box %d, maxfishthere = %e, fishthere = %e, FCpressure = %e, new_fish_loc = %d, mEff_thresh = %e\n",
						FisheryArray[fishery_id].fisheryCode, ij, maxfishthere, fishthere, *FCpressure, *new_fish_loc, mEff_thresh);

			}
			*/
			/* If stock in an adjacent box is better than in the current box displace
			 some effort to that location */
			if (maxfishthere > fishhere) {
				/* If displacement is due to poor stocks the effort displaced matches
				 the relative stock sizes in the current and adjacent box */
				prop_displaced = maxfishthere / (maxfishthere + fishhere);
				*FCdisplaced = *FCpressure * prop_displaced;
				*FCpressure -= *FCdisplaced;

				/* If displacement is due to an MPA, the effort displaced
				 is the difference between the full effort for the box and
				 that left after MPA restrictions imposed */
				*FCdisplaced = orig_FCpressure - *FCpressure;
			}
		}
	}

	return;
}

/**
 * \brief  Distance from each port to centre of each box in the model domain.
 */
void Distance_to_Port(MSEBoxModel *bm) {
	double xdiff, ydiff, step1;
	int i, b, fishery_id;
    double avg_dist = 0.0;
    double counter = 0;
    
	for (i = 0; i < bm->K_num_ports; i++) {
		for (b = 0; b < bm->nbox; b++) {
			xdiff = fabs(bm->Port_info[i][portx_id] - bm->boxes[b].inside.x);
			ydiff = fabs(bm->Port_info[i][porty_id] - bm->boxes[b].inside.y);
			step1 = xdiff * xdiff + ydiff * ydiff;
			bm->Port_distance[i][b] = sqrt(step1);
            avg_dist += bm->Port_distance[i][b];
            counter += 1.0;
		}
	}
    avg_dist /= counter;
    if(!avg_dist)
        avg_dist = small_num;
    
    /* Also rescale the Effort scalars (mFCscale_id) accordingly */
    for (fishery_id = 0; fishery_id < bm->K_num_fisheries; fishery_id++) {
        bm->FISHERYprms[fishery_id][mFCscale_id] *= avg_dist;
    }
    

	return;
}

/**
 * \brief Check for which MPAs are active on current day of the year
 */
void Check_For_Active_MPA(MSEBoxModel *bm, int fishery_id) {
	int flagfcmpa, ij, k, sp, cohort, cIndex, ad, adbox;
	double temp_scale, temp_scale2, catch_check, cGroupLevel, thresh_level;
	int trigger_tripped, end_trigger_tripped;
    double *tracerArray;

	if (effort_scale[fishery_id][target_tac_id] != 1.0)
		trigger_tripped = 1;
	else
		trigger_tripped = 0;
	if (effort_scale[fishery_id][endangered_tac_id] != 1.0)
		end_trigger_tripped = 1;
	else
		end_trigger_tripped = 0;

	flagfcmpa = (int) (bm->FISHERYprms[fishery_id][flagmpa_id]);

	for (ij = 0; ij < bm->nbox; ij++) {        
		switch (flagfcmpa) {
		case no_mpa: /* No spatial management - so set = 1.0 */
			bm->MPA[ij][fishery_id] = 1.0;
			break;
		case fix_mpa: /* Spatial management fixed at initial distribution */
		case cycle_mpa: /* Rotational spatial management */
		case mix_fix_rolling_mpa: /* Mix of fixed and rolling spatial management */
		case mix_f_r_spawn_mpa: /* Mix of fixed, rolling and seasonal spawning closures used in management */
		case depth_stock_mpa: /* US-like depth closures triggered when TAC exceeded (if list needed updating that
		 has been done in the Manage_Visit_Council() routine) */
		case council_stock_mpa: /* US-like closures (but based on biom distrib of groups) triggered when TAC exceeded
		 (if list needed updating that has been done in the Manage_Visit_Council() routine) */
			bm->MPA[ij][fishery_id] = bm->MPAlist[bm->thisyear][bm->TofY][ij][fishery_id];
			break;
		case stock_mpa: /* Spatial management responds to the size of the target
		 species for this fishery (using fixed mpas) */
		case cycle_stock_mpa: /* Spatial management responds to the size of the target
		 species for this fishery (using changing mpa locations) */
			if (trigger_tripped) {
				bm->MPA[ij][fishery_id] = bm->MPAlist[bm->thisyear][bm->TofY][ij][fishery_id];
				manageindx[fishery_id][mgmtstability_id] = 0;
			}
			break;
		case pet_mpa: /* Spatial management responds to the size of
		 charismatic species population (using fixed mpas) */
		case cycle_pet_mpa: /* Spatial management responds to the size of
		 charismatic species population (using changing mpa locations) */
			if (end_trigger_tripped) {
				bm->MPA[ij][fishery_id] = MPAendangeredlist[bm->TofY][ij][fishery_id];
				manageindx[fishery_id][mgmtstability_id] = 0;
			}
			break;
		case stock_pet_mpa: /* Spatial management responds to the size of the target
		 species for this fishery and the charismatic species populations
		 (using fixed mpas) */
		case cycle_stock_pet_mpa: /* Spatial management responds to the size of the target
		 species for this fishery and the charismatic species populations
		 (using changing mpa locations) */
			if (trigger_tripped) {
				temp_scale = bm->MPAlist[bm->thisyear][bm->TofY][ij][fishery_id];
				manageindx[fishery_id][mgmtstability_id] = 0;
			} else
				temp_scale = 1.0;
			if (end_trigger_tripped) {
				temp_scale2 = MPAendangeredlist[bm->TofY][ij][fishery_id];
				manageindx[fishery_id][mgmtstability_id] = 0;
			} else
				temp_scale2 = 1.0;
			if (temp_scale < temp_scale2)
				bm->MPA[ij][fishery_id] = temp_scale;
			else
				bm->MPA[ij][fishery_id] = temp_scale2;
			break;
        case catch_mpa: /* Catch based spatal closures */
            for (sp = 0; sp < bm->K_num_tot_sp; sp++) {
                catch_check = 0.0;
                for (k = 0; k < bm->boxes[ij].nz; k++)
                    catch_check += bm->CumCatch[sp][fishery_id][ij][k];
                // Check cumulative catch vs trip limit and trigger spatial closure of that box
                if (catch_check > bm->SP_FISHERYprms[sp][fishery_id][trip_lim_id])
                    bm->MPA[ij][fishery_id] = 0.0;
            }
            break;
		default:
			quit("No such mpa case defined (%d) - value must be between %d and %d currently\n", flagfcmpa, no_mpa, cycle_stock_pet_mpa);
			break;
		}

        /* Check for contaminant based closures */
        if(bm->track_contaminants) {
        
            switch (bm->flag_contam_fisheries_mgmt) {
                case no_closures: // Nothing to do
                    break;
                case set_closures: // Closed for a set period after the spill
                    if((bm->dayt >= bm->contam_fishery_closure_day) && (bm->dayt <= (bm->contam_fishery_closure_day + bm->contam_fishery_closure_period))) {
                        bm->MPA[ij][fishery_id] = 1.0 - bm->ContamClosed[ij];  // Assumes ContamClosed has 1 if closed due to contaminant and 0 if left open - so easier for people entering the parameters to udnerstand what is being done)
                    }
                    break;
                case conc_based: // Based on concentration in species
                    tracerArray = boxLayerInfo->localWCTracers;
                    for (sp = 0; sp < bm->K_num_tot_sp; sp++) {
                        for(cohort = 0; cohort < FunctGroupArray[sp].numCohortsXnumGenes; cohort++){
                            for (cIndex = 0; cIndex < bm->num_contaminants; cIndex++) {
                                cGroupLevel = tracerArray[FunctGroupArray[sp].contaminantTracers[cohort][cIndex]];
                                
                                thresh_level = bm->contaminantStructure[cIndex]->fishery_thresh_level;
                                if (cGroupLevel > thresh_level) {
                                    if (!bm->contam_fishery_closure_option) {
                                        bm->MPA[ij][fishery_id] = 0.0;  // Close only the immediate box
                                    } else { // Close immediate box and adjactent boxes
                                        bm->MPA[ij][fishery_id] = 0.0;
                                        for (ad = 0; ad < bm->boxes[ij].nconn; ad++) {
                                            adbox = bm->boxes[ij].ibox[ad];
                                            bm->MPA[adbox][fishery_id] = 0.0;
                                        }
                                    }
                                    
                                    // Break loop as already closed now
                                    sp = bm->K_num_tot_sp;
                                }
                            }
                        }
                    }
                    break;
                default:
                    quit("This (%d) setting for flag_contam_fisheries_mgmt is not yet coded - please use %d (no closures), %d (set closures) or %d (based on concentration in stocks)\n", bm->flag_contam_fisheries_mgmt, no_closures, set_closures, conc_based);
                    break;
            }
        }
    }
    
	return;
}

/**
 * \brief Check effort cap
 */
void Check_CAP(MSEBoxModel *bm, FILE *llogfp) {
	int flagchangeCAP, fishery_id, CAP_num_changes, b, flagcap, flagspeffortmodel;
	double CAP_scale, cap, toteffort, allow_effort, effort_rescale, effort_expended, totghosteffort;
	//int do_debug, do_debug_base;

	/*
    if (bm->debug == debug_effort) {
		do_debug_base = 1;
	} else {
		do_debug_base = 0;
    }
    */

	CAP_scale = 1.0;
	for (fishery_id = 0; fishery_id < bm->K_num_fisheries; fishery_id++) {

		/*if (do_debug_base && ((bm->which_fleet == fishery_id) || (bm->which_fleet >= bm->K_num_fisheries))) {
			do_debug = 1;
		} else {
			do_debug = 0;
         }
        */

		cap = bm->FISHERYprms[fishery_id][cap_id];
		flagcap = (int) (bm->FISHERYprms[fishery_id][flagcap_id]);
		flagspeffortmodel = (int) (bm->FISHERYprms[fishery_id][flageffortmodel_id]);

		/* Only finish the check if caps in place */
		if (!flagcap)
			continue;

		flagchangeCAP = (int) (bm->FISHERYprms[fishery_id][flagchangeP_id]);
		CAP_num_changes = (int) (bm->FISHERYprms[fishery_id][P_num_changes_id]);

		if (flagchangeCAP) {
			CAP_scale = Util_Get_Change_Scale(bm, CAP_num_changes, CAPchange[fishery_id]);

		}

		cap *= CAP_scale;

		/* Total shots to date */
		toteffort = 0;
		totghosteffort = 0;
		for (b = 0; b < bm->nbox; b++) {
			toteffort += bm->CumEffort[fishery_id][b];
			totghosteffort += bm->GhostEffort[b][fishery_id];
		}

		if (bm->UsingEffortIncentive)
			effort_expended = totghosteffort;
		else
			effort_expended = (toteffort + bm->totNewEffort[fishery_id]);

		/* Reset effort to days fishing not shots and compare vs the cap */
		if (effort_expended > cap) {
			/* If cumulative effort plus projected effort > cap then scale back so can't exceed the cap */
			allow_effort = cap - toteffort;
			if (allow_effort < 0)
				allow_effort = 0;

			/* Reset effort */
			for (b = 0; b < bm->nbox; b++) {
				bm->Effort[b][fishery_id] *= allow_effort / (bm->totNewEffort[fishery_id] + small_num);

                /*
				if(do_debug){
                    fprintf(llogfp, "Time: %e, %s-%d neweffort: %e (due to cap being exceeded)\n", bm->dayt, FisheryArray[fishery_id].fisheryCode, b,
						bm->Effort[b][fishery_id]);
				}
                */
			}
		} else if (flagspeffortmodel < readts_effort) {
			/* For aggregate fleet models rescale so effort spread through the year rather than flooding
			 in year start and then sitting idle for majority of the year
			 */
			if ((bm->totNewEffort[fishery_id] * 365.0) > cap) {
				/* Reset effort so spread through the year */

				//fprintf(llogfp, "Time: %e %s effort restreamed to avoid effective cap\n", bm->dayt, FisheryArray[fishery_id].fisheryCode);

				effort_rescale = cap / (bm->totNewEffort[fishery_id] * 365.0 + small_num);
				for (b = 0; b < bm->nbox; b++) {
					bm->Effort[b][fishery_id] *= effort_rescale;

                    /*
					if(do_debug){
                        fprintf(llogfp, "Time: %e, %s-%d neweffort: %e (due to cap rescaling)\n", bm->dayt, FisheryArray[fishery_id].fisheryCode, b,
							bm->Effort[b][fishery_id]);
					}
                    */

				}
			}
		}
	}

	return;
}

/**
 * \brief Port activity corrections - for when not using economic drivers (as that case is dealt with
 * explicitly in the economics library
 */
void Update_Port_Population(MSEBoxModel *bm, FILE *llogfp) {
	int porti, flagchangePOP, POP_num_changes;
	double POP_scale = 1.0, start_pop;

	for (porti = 0; porti < bm->K_num_ports; porti++) {
		/* No check if active here as needn't be a commerical port to have recreational fishing pressure */

		/* Get population of port fishing and scale it for growth if necessary */
		start_pop = bm->Port_info[porti][population_id];
		flagchangePOP = (int) (bm->Port_info[porti][popchange_id]);
		POP_num_changes = (int) (bm->Port_info[porti][POP_num_changes_id]);

		if (flagchangePOP) {
			POP_scale = Util_Get_Change_Scale(bm, POP_num_changes, bm->POPchange[porti]);
		}
		bm->Port_info[porti][popactive_id] = start_pop * POP_scale;
	}

	return;
}

/**
 * \brief Contribution per active port to fisheries effort distributions
 * And for the scaled effort model find the compound contribution of CPUE and ports
 */
void Calculate_Port_Contrib(MSEBoxModel *bm, int fishery_id, int flagspeffortmodel, FILE *llogfp) {
	int porti, b, flagfishhere, peak_effort, peak_cpue_port;
	double CPUE_and_port, max_effort, max_cpue_port, step1, xdiff, ydiff, port_bit, cpue_bit, newport;
	double mEffscale = bm->FISHERYprms[fishery_id][mFCscale_id];

	bm->totPortContrib[fishery_id][simple_id] = 0.0;
	for (b = 0; b < bm->nbox; b++) {
		bm->PortContrib[fishery_id][b][pvalue_id] = 0.0;
		for (porti = 0; porti < bm->K_num_ports; porti++) {
			/* See if ever an active port for this fishery */
			flagfishhere = bm->Port_Fishery[fishery_id][porti];

			/* Check if currently active */
			if ((bm->dayt < bm->Port_info[porti][port_start_id]) || (bm->dayt > bm->Port_info[porti][port_end_id]))
				flagfishhere = 0;

			/*
			 if(do_debug){
			 fprintf(llogfp, "Time: %e, %s port-%d has flagfishhere = %d (port_start: %d port_end: %d)\n",
			 bm->dayt, FisheryArray[fishery_id].fisheryCode, porti, flagfishhere, bm->Port_info[porti][port_start_id], bm->Port_info[porti][port_end_id]);
			 }
			 */

			/* If an active port than determine contribution to effort from that port
			 use the scalar mEffscale to heighten or lessen effects of distance
			 - so can also capture social and economic forces which may push
			 fishers to either stay close to home or go far out to sea even when
			 not economically efficent to do so */
			if (flagfishhere) {
				bm->PortContrib[fishery_id][b][pvalue_id] += mEffscale / bm->Port_distance[porti][b];

				/*
				 if(do_debug){
				 fprintf(llogfp, "Time: %e, %s in %d has FCweight: %e, mEffscale: %e, port_dist-%d: %e\n",
				 bm->dayt, FisheryArray[fishery_id].fisheryCode, ij, FCweight, mEffscale, porti, bm->Port_distance[porti][ij]);
				 }
				 */

			}
		}
		bm->totPortContrib[fishery_id][simple_id] += bm->PortContrib[fishery_id][b][pvalue_id];
	}

	/* Check port contributions and cap any excessive peaks if appropriate */
	if (bm->FISHERYprms[fishery_id][flagcap_peak_id]) {
		newport = 0;

		//fprintf(llogfp,"Time: %e, %s portcontrib start: %e\n",
		//	bm->dayt, FisheryArray[fishery_id].fisheryCode, bm->totPortContrib[fishery_id][simple_id]);

		for (b = 0; b < bm->nbox; b++) {
			port_bit = bm->PortContrib[fishery_id][b][pvalue_id] / (bm->totPortContrib[fishery_id][simple_id] + small_num);
			bm->PortContrib[fishery_id][b][clipped_id] = 0;
			if (port_bit > bm->FISHERYprms[fishery_id][mFCpeak_id]) {
				bm->PortContrib[fishery_id][b][pvalue_id] = bm->FISHERYprms[fishery_id][mFCpeak_id] * bm->totPortContrib[fishery_id][simple_id];
				bm->PortContrib[fishery_id][b][clipped_id] = 1;

				//fprintf(llogfp, "Time: %e, box: %d, %s portcontrib clipped to: %e (from %e)\n",
				//	bm->dayt, ij, FisheryArray[fishery_id].fisheryCode, bm->PortContrib[fishery_id][ij][pvalue_id], port_bit);
			}
			newport += bm->PortContrib[fishery_id][b][pvalue_id];
		}
		/* Rescale if necessary */
		for (b = 0; b < bm->nbox; b++) {
			if (bm->PortContrib[fishery_id][b][clipped_id] < 1) {
				bm->PortContrib[fishery_id][b][pvalue_id] *= (bm->totPortContrib[fishery_id][simple_id]) / (newport + small_num);

				//fprintf(llogfp, "Time: %e, box: %d, %s final portcontrib: %e\n",
				//	bm->dayt, ij, FisheryArray[fishery_id].fisheryCode, bm->PortContrib[fishery_id][ij][pvalue_id]);
			}

		}
	}

	/* For models using CPUE calculate compound port and CPUE contributions */
	if (bm->FISHERYprms[fishery_id][fisheries_need_port_id]) {
		CPUE_and_port = 0;
		max_effort = 0;
		max_cpue_port = 0;
		peak_effort = 0;
		peak_cpue_port = 0;
		for (b = 0; b < bm->nbox; b++) {
			port_bit = bm->PortContrib[fishery_id][b][pvalue_id] / (bm->totPortContrib[fishery_id][simple_id] + small_num);

			cpue_bit = bm->TempCPUE[b][fishery_id] / (bm->totCPUE[fishery_id] + small_num);
			CPUE_and_port += (port_bit * cpue_bit);

			/* Find peak sites - for distance calculation */
			if (max_effort < bm->OldEffort[b][fishery_id]) {
				peak_effort = b;
				max_effort = bm->OldEffort[b][fishery_id];
			}
			if (max_cpue_port < (port_bit * cpue_bit)) {
				peak_cpue_port = b;
				max_cpue_port = port_bit * cpue_bit;
			}
		}
		bm->totPortContrib[fishery_id][compound_id] = CPUE_and_port;

		/* Calculate peak distance */
		xdiff = fabs(bm->boxes[peak_effort].inside.x - bm->boxes[peak_cpue_port].inside.x);
		ydiff = fabs(bm->boxes[peak_effort].inside.y - bm->boxes[peak_cpue_port].inside.y);
		step1 = xdiff * xdiff + ydiff * ydiff;
		DistPeak[fishery_id] = sqrt(step1);
	}

	return;
}
/**
 * \brief  US management stype - cumualtive trip limits based off TAC, and once TAC exhausted then trigger spatial management actions
 */
void Manage_Visit_Council(MSEBoxModel *bm, FILE *llogfp) {
	int do_stuff, nf, nreg, sp, tripped, overfished_sp, delay, implement_BiM, bim, ij, correct_reg, nd, QuarterOfYear, in_box, flagTACparticipate, co_sp, co_sp2, companion_ok, min_delay, flagfcmpa, Kreg;
	double regTAC_scale, regC_scale, biom_ratio, FCperiod, other_biTAC_reg, TAC, CumCatch, catch_ratio, regTAC_scale_cosp = 0, regC_scale_cosp = 0, TAC_cosp, CumCatch_cosp, prop_yr_changed, catch_ratio_cosp;

	do_stuff = 0;
	TAC_cosp = 0;
	CumCatch_cosp = 0;
	implement_BiM = 0;
	/* If no council needed do not enter this routine */
	if (!bm->Council_needed){
		fprintf(bm->logFile, "return council not needed\n");
		return;
	}
	/** Only continue if relevant and in new bi-month period **/
	if (bm->newmonth) {
		switch (bm->MofY) {
		case 0:
		case 2:
		case 4:
		case 6:
		case 8:
		case 10:
			/* Play with cumulative trip limits */
			do_stuff = 1;
			break;
		case 1:
		case 3:
		case 5:
		case 7:
		case 9:
		case 11:
			/* Don't do anything */
			do_stuff = 0;
			break;
		default:
			quit("How the hell did you get more than 12 months in a year - %d\n", bm->BiM);
			break;
		}
	}

	/* Only continue if actually need to visit the council */
	if (!do_stuff){
		return;
	}

	/* FIX - need to generalise regional TAC so don't need to use this 2 region - north-south - assumption check */
	if (bm->K_num_active_reg > 2)
		Kreg = 2;
	else
		Kreg = bm->K_num_active_reg;

	/* Check status vs TAC  and check vs list of overfished spp */
	for (nreg = 0; nreg < Kreg; nreg++) {
		for (sp = 0; sp < bm->K_num_tot_sp; sp++) {
			if (FunctGroupArray[sp].isFished == TRUE) {
				regTAC_scale = bm->RegionalData[sp][nreg][reg_tac_id];
				regC_scale = bm->RegionalData[sp][nreg][reg_catch_id];

				/* Check whether regional TAC (cumulative tip limits) have been exceeded */
				tripped = 0;
				for (nf = 0; nf < bm->K_num_fisheries; nf++) {
					if (bm->TAC_over[sp][nf][nreg])
						tripped = 1;
				}

				/* Check whether regional species is overfished */
				overfished_sp = 0;
				if (FunctGroupArray[sp].speciesParams[sp_concern_id]) {
					biom_ratio = (bm->totfishpop[sp] * bm->X_CN * mg_2_tonne) / (bm->estinitpop[sp] + small_num);
					if (biom_ratio < bm->lim_ref) {
						overfished_sp = 1;
					}
				}

				/* Reset cumulative trip limts */
				if (tripped) {
					for (nf = 0; nf < bm->K_num_fisheries; nf++) {
						/* Only participate if using appropriate economically based effort model */
						if (bm->FISHERYprms[nf][visitcouncil_id]) {

							/* Political process delay vs bimonthly council metting steps - to see when action implemented */
							flagfcmpa = (int) (bm->FISHERYprms[nf][flagmpa_id]);
							FCperiod = bm->FISHERYprms[nf][FC_period_id];
							delay = (int) (ceil(FCperiod / 60.0));
							implement_BiM = bm->BiM + delay;
							if (implement_BiM > 5) {
								/* Would happen after end of year so no actual effect as would be superceded by next years' managememt */
								continue;
							}

							/* Rescale */
							for (bim = implement_BiM; bim < 6; bim++) {
								bm->BiTAC_sp[bim][nreg][sp][now_id] = 0.0;
								other_biTAC_reg = (1.0 - regTAC_scale) * bm->BiTACamt[bim][sp][nf][now_id];
								bm->BiTACamt[bim][sp][nf][now_id] = other_biTAC_reg;

								printf("Time1: %e, %s by %s, bim: %d, BiTACamt: %e\n", bm->dayt, FunctGroupArray[sp].groupCode, FisheryArray[nf].fisheryCode,
										bim, bm->BiTACamt[bim][sp][nf][now_id]);

							}

							/* Adjust MPA if necessary */
							for (ij = 0; ij < bm->nbox; ij++) {

								/* Check box is in the region */
								correct_reg = 1;
								if (bm->regID[ij] != nreg)
									correct_reg = 0;

								if (correct_reg) {
									switch (flagfcmpa) {
									case depth_stock_mpa: /* Depth-based (fixed) closures */
										for (nd = (implement_BiM * 60); nd < 365; nd++) {
											bm->MPAlist[bm->thisyear][nd][ij][nf] = MPAoverfishedlist[ij][sp];
										}
										break;
									case council_stock_mpa: /* Biom based closures */
										for (nd = (implement_BiM * 60); nd < 365; nd++) {
											/* Check the groups distribution */

											if (nd <= 91) {
												QuarterOfYear = 0;
											} else if ((nd > 90) && (nd <= 181)) {
												QuarterOfYear = 1;
											} else if ((nd > 181) && (nd <= 272)) {
												QuarterOfYear = 2;
											} else {
												QuarterOfYear = 3;
											}

											in_box = 0;
											if (FunctGroupArray[sp].isVertebrate == TRUE) {
												/* Vertebrates */
												if (FunctGroupArray[sp].distrib[ij][QuarterOfYear][adult_id] || FunctGroupArray[sp].distrib[ij][QuarterOfYear][juv_id]) {
													in_box = 1;
												}
											} else {
												/* Invertebrates - FIX should probably use distribINVERT */
												if (bm->targetspbiom[sp][ij])
													in_box = 1;
											}

											/* Put MPAs in all boxes the group is found in */
											if (in_box)
												bm->MPAlist[bm->thisyear][nd][ij][nf] = 0;
										}
										break;
									default:
										quit("The code for Visiting councils currently only supports options for depth (%d) or biom (%d) based options\n",
												depth_stock_mpa, council_stock_mpa);
										break;
									}
								}
							}
						}
					}
				} else {
					/* TAC not exceeded, but may need to rescale cumulative trip limits based on realised catch vs the trip limit expectations */

					TAC = 0;
					CumCatch = 0;
					for (nf = 0; nf < bm->K_num_fisheries; nf++) {
						flagTACparticipate = (int) (bm->FISHERYprms[nf][flagTACpartipcate_id]);
						if (flagTACparticipate) {
							/* Convert from tonnes wet weight to mg N */
							TAC += (regTAC_scale * bm->TACamt[sp][nf][now_id] * kg_2_mg) / bm->X_CN;
							CumCatch += (Harvest_Get_TotCumCatch(sp, nf, bm->thisyear) + bm->TotOldCumCatch[sp][nf]) * regC_scale; //bm->TotCumCatch[sp][nf][bm->thisyear] * regC_scale;
						}
					}
					catch_ratio = CumCatch / (TAC + small_num);
					if (catch_ratio < 1.0) {
						/* Rescaling necessary - but only if companion species would be worse off, so check companions.*/
                        
                        for (co_sp = 0; co_sp < FunctGroupArray[sp].speciesParams[max_co_sp_id]; co_sp++ ) {
                            co_sp2 = FunctGroupArray[sp].co_sp[co_sp];
                            
                            if ((co_sp2 < 0) || (co_sp2 > bm->K_max_impacted_sp) || (FunctGroupArray[co_sp2].isFished == FALSE)) {
                                continue;
                            }

                            regTAC_scale_cosp = bm->RegionalData[co_sp2][nreg][reg_tac_id];
							regC_scale_cosp = bm->RegionalData[co_sp2][nreg][reg_catch_id];
							TAC_cosp = 0;
                            CumCatch_cosp = 0.0;
                            companion_ok = 1;
                            
							for (nf = 0; nf < bm->K_num_fisheries; nf++) {
								flagTACparticipate = (int)(bm->FISHERYprms[nf][flagTACpartipcate_id]);
								if (flagTACparticipate) {
									/* Convert from tonnes wet weight to mg N */
									TAC_cosp += (regTAC_scale_cosp * bm->TACamt[co_sp2][nf][now_id] * kg_2_mg) / bm->X_CN;
                                    CumCatch_cosp += (Harvest_Get_TotCumCatch(co_sp2, nf, bm->thisyear)  + bm->TotOldCumCatch[co_sp2][nf]) * regC_scale_cosp;
								}
							}
							/* Check if scaled up CumCatch for companions would exceed TAC */
							prop_yr_changed = (1.0 - implement_BiM / 6.0);
							CumCatch_cosp *= 1.0 + ((1 / (catch_ratio + small_num)) * prop_yr_changed);
							catch_ratio_cosp = CumCatch_cosp / (TAC_cosp + small_num);

                            if (catch_ratio_cosp > 1.0) {
                                companion_ok = 0;
                            }


                            /* Do rescaling */
                            if (companion_ok) {
                                min_delay = MAXINT;
                                for (nf = 0; nf < bm->K_num_fisheries; nf++) {
                                    /* Political process delay vs bimonthly council metting steps _ to see when action implemented */
                                    FCperiod = bm->FISHERYprms[nf][FC_period_id];
                                    delay = (int) (ceil(FCperiod / 60.0));
                                    if (delay < min_delay)
                                        min_delay = delay;
                                    implement_BiM = bm->BiM + delay;
                                    if (implement_BiM > 5) {
                                        /* Would happen after end of year so no actual effect as would be superceded by next years' managememt */
                                        continue;
                                    }
                                    if (!overfished_sp) {
                                        /* Only rescale if not overfished. If overfished consider yourself lucky and take the opportunity for extra rebuilding.
                                         */
                                        for (bim = implement_BiM; bim < 6; bim++) {
                                            bm->BiTAC_sp[bim][nreg][sp][now_id] = regTAC_scale * bm->BiTACamt[bim][sp][nf][now_id] * (1 / (catch_ratio + small_num));
                                            other_biTAC_reg = (1.0 - regTAC_scale) * bm->BiTACamt[bim][sp][nf][now_id];
                                            bm->BiTACamt[bim][sp][nf][now_id] = bm->BiTAC_sp[bim][nreg][sp][now_id] + other_biTAC_reg;

                                            printf("Time2: %e, %s by %s, bim: %d, BiTACamt: %e\n", bm->dayt, FunctGroupArray[sp].groupCode,
												FisheryArray[nf].fisheryCode, bim, bm->BiTACamt[bim][sp][nf][now_id]);
                                        }
									}
								}
							}
						}
					} else {
						/* This should have been caught by "tripped"m section above */
						quit("How can catch/TAC be > 1 and not trip management? Code goof!!\n");
					}
				}
			}
		}
	}

	for (sp = 0; sp < bm->K_num_tot_sp; sp++) {
		if (FunctGroupArray[sp].isImpacted == TRUE) {
			/* Total combined % of total TAC */
			bm->BiTAC_sp[bm->BiM][totalreg_id][sp][now_id] = bm->BiTAC_sp[bm->BiM][0][sp][now_id] + bm->BiTAC_sp[bm->BiM][1][sp][now_id];

			/* Redo regional ratios - assumes only two regions and assumes using cumTAC management (US-like system) */
			for (nf = 0; nf < bm->K_num_fisheries; nf++) {
				if (bm->FISHERYprms[nf][flagmanage_id] == cumTACclosure_mgmt) {
					for (nreg = 0; nreg < 2; nreg++) {
						bm->RegionalData[sp][nreg][reg_tac_id] = bm->BiTAC_sp[bm->BiM][nreg][sp][now_id] / (bm->BiTAC_sp[bm->BiM][totalreg_id][sp][now_id]
								+ small_num);
					}
				}
				/* Reset closure notices */
				bm->FISHERYprms[nf][closenotice_id] = 0;
			}
		}
	}

	return;
}

/**
 *	\brief Find the maximum bm->FISHERYprms value for the given parameter.
 *	The value returned will be the maximum of 1 and the max value + 1.
 *
 */
int Manage_Get_Max_Fishery_Param(MSEBoxModel *bm, int paramIndex) {
	int i;
	int maxValue = 1;

	for (i = 0; i < bm->K_num_fisheries; i++) {
		if (bm->FISHERYprms[i][paramIndex] > maxValue)
			maxValue = (int) (bm->FISHERYprms[i][paramIndex] + 1);
	}
	return maxValue;

}
/**
 *	\brief Find the maximum bm->SP_FISHERYprms value for the given parameter.
 *	The value returned will be the maximum of 1 and the max value + 1.
 *
 */
int Manage_Get_Max_Species_Fishery_Param(MSEBoxModel *bm, int paramIndex) {
	int maxValue, sp, i;

	maxValue = 1;
	for (sp = 0; sp < bm->K_num_tot_sp; sp++) {
		for (i = 0; i < bm->K_num_fisheries; i++) {
			maxValue = max ( maxValue, ( int ) bm->SP_FISHERYprms[sp][i][paramIndex] );
		}
	}
	return maxValue;
}


/***************************************************************************************************************************************************************
 *  \brief generate and store shot-by-shot cpue - this assumes units of effort are in days fished
 *
 */
void GenerateCPUE(MSEBoxModel *bm, FILE *llogfp) {
    
    int sp, nf, ij, k, ns, this_shot, nc, just_spotted, this_gun, this_catchbin;
    double tot_nboat, tot_hold, shotcount, step1, max_prob, weighted_depth, catch_unalloced, final_shot_catch, abscatch, x,
        min_shots, max_shots, tot_effort, sum_prob, effort_contrib, tot_effort_contrib, this_catch, catch_left, this_discards,
        prop_discard, min_catch, max_catch, diff_Catch, final_shot_discards, min_effort, this_effort, final_shot_effort, totC;
    double hr_per_dt = (bm->dt / 86400) * 24.0;  // So converting dt in to per day rate and then multiple by 24 to see number of hours.
    int num_shots_per_day = (int)(ceil(24 / bm->K_min_shotlength));
    int max_poss_shots = bm->K_max_num_subfleet * bm->K_max_num_boats * num_shots_per_day + bm->nbox + 1;
    int step_tolerance = 10;
    int numstep = 0;
    //int do_debug = 1;
    
    if(bm->dayt < 1.0)
        return;
    
    if(!bm->K_max_num_boats)
        quit("K_max_num_boats has been set to zero. Make sure that econ is turned on in run.prm\n");
    
    for (nf = 0; nf < bm->K_num_fisheries; nf++) {
        if(bm->FISHERYprms[nf][flagneed_shots_id]) {
            //Only continue of can actually need to
            if (bm->FISHERYprms[nf][flageffortmodel_id] > rec_econ_model) {
                /* Find probability (based on realtive hold capacity per subfleet) that catch is in hold of boat in subfleet ns
                    Also find min and max number of potential shots per day possible given number of boats, fishable time and
                    minimum down time
                */
                Util_Init_3D_Double(bm->ShotData, max_poss_shots, bm->K_num_tot_sp, K_num_shot_data_entries, 0.0);
                Util_Init_1D_Int(bm->box_done, bm->nbox, 0);
                Util_Init_1D_Double(bm->alloced_effort, bm->nbox, 0.0);
                Util_Init_3D_Double(bm->alloced_catch, bm->K_num_tot_sp, bm->nbox, bm->wcnz, 0.0);
                Util_Init_2D_Double(bm->alloced_discard, bm->K_num_tot_sp, bm->nbox, 0.0);
                tot_nboat = 0.0;
                tot_hold = 0.0;
                min_shots = 0.0;
                max_shots = 0.0;
                nshot = 0;
                Util_Init_1D_Double(bm->prop_hold, bm->K_max_num_subfleet, 0.0);
                for (ns = 0; ns < bm->FISHERYprms[nf][nsubfleets_id]; ns++) {
                    tot_nboat += bm->SUBFLEET_ECONprms[nf][ns][nboat_id];
                    bm->prop_hold[ns] += bm->SUBFLEET_ECONprms[nf][ns][hold_capacity_id] * bm->SUBFLEET_ECONprms[nf][ns][nboat_id];
                    tot_hold += bm->SUBFLEET_ECONprms[nf][ns][hold_capacity_id];
                    min_shots += bm->SUBFLEET_ECONprms[nf][ns][nboat_id] * bm->SUBFLEET_ECONprms[nf][ns][down_time_id] * bm->SUBFLEET_ECONprms[nf][ns][FishableLength_id] ;  // So a minimum of one shot each for active vessels
                    max_shots += bm->SUBFLEET_ECONprms[nf][ns][nboat_id] * bm->SUBFLEET_ECONprms[nf][ns][down_time_id] * bm->SUBFLEET_ECONprms[nf][ns][FishableLength_id] * (hr_per_dt / (bm->SUBFLEET_ECONprms[nf][ns][ShotLength_id] + small_num));
                }
                for (ns = 0; ns < bm->FISHERYprms[nf][nsubfleets_id]; ns++) {
                    bm->prop_hold[ns] /= (tot_hold + small_num);
                }
                
                // Find number of shots to take
                if (min_shots > max_shots)
                    max_shots = min_shots;
                step1 = drandom(0.0, min_shots);  // As getting too many shots so reset starting point a bit lower
                min_shots = step1;
                shotcount = ceil(max_shots - min_shots);
                if(shotcount)
                    step1 = drandom(0.0, shotcount);
                step1 += min_shots;
                nshot = (int)ceil(step1);
                
                // Find the locaiton of the shots
                tot_effort = 0.0;
                Util_Init_1D_Double(bm->prop_effort, bm->nbox, 0.0);
                Util_Init_1D_Int(bm->shot_loc, max_poss_shots, 0);
                for (ij = 0; ij < bm->nbox; ij++) {
                    bm->prop_effort[ij] += bm->Effort_hdistrib[ij][nf][today_effort];
                    tot_effort += bm->Effort_hdistrib[ij][nf][today_effort];
                }
                for (ij = 0; ij < bm->nbox; ij++) {
                    bm->prop_effort[ij] /= (tot_effort + small_num);
                }
                
                this_shot = 0;
                max_prob = 1.0;
                Util_Init_1D_Int(bm->shot_loc, max_poss_shots, 0);
                while (this_shot < nshot) {
                    step1 = drandom(0.0, max_prob);
                    sum_prob = 0.0;
                    for (ij = 0; ij < bm->nbox; ij++) {
                        sum_prob += bm->prop_effort[ij];
                        if (step1 <= sum_prob) {
                            bm->shot_loc[this_shot] = ij;
                            bm->box_done[ij] = 1;
                            this_shot++;
                            break;
                        }
                    }
                }
                // Make sure all the boxes with catch are included - if not pad out the list now
                for (ij = 0; ij < bm->nbox; ij++) {
                    if(bm->box_done[ij] < 1) {
                        just_spotted = 0;
                        for (k = 0; k < bm->boxes[ij].nz; k++) {
                            if(!just_spotted) {
                                for (sp = 0; sp < bm->K_num_tot_sp; sp++) {
                                    if (bm->Catch[ij][sp][nf][k] > 0.0) {
                                        bm->shot_loc[this_shot] = ij;
                                        this_shot++;
                                        nshot++;
                                        just_spotted++;
                                        break;
                                    }
                                }
                            } else
                                break;
                        }
                    }
                }
                
                /* Get catch and effort */
                this_shot = 0;
                while (this_shot < nshot) {
                    abscatch = 0.0;
                    ij = bm->shot_loc[this_shot];  // location of the shot
                    
                 // Find whether a gun skipper or a mediocre one - assumes "good" and "poor" fisherman - for now base it on flexweight
                // Use flexweight with threshold at 0.2 >= this = gun less than = poor
                    effort_contrib = 0.0;
                    tot_effort_contrib = 0.0;
                    for (ns = 0; ns < bm->FISHERYprms[nf][nsubfleets_id]; ns++) {
                        // If no boats in the subfleet currently skip ahead
                        if (!bm->SUBFLEET_ECONprms[nf][ns][nboat_id]) {
                            continue;
                        }
                        tot_effort_contrib += bm->SpatialBlackBook[nf][ns][bm->MofY][ij][current_id];
                        if (bm->SUBFLEET_ECONprms[nf][ns][flexweight_id] < bm->gun_flexthresh)
                            effort_contrib += bm->SpatialBlackBook[nf][ns][bm->MofY][ij][current_id];
                    }
                    effort_contrib /= (tot_effort_contrib + small_num);
                    step1 = drandom(0.0, 1.0);
                    if (step1 < effort_contrib)  // Drawn someone who is not a gun skipper
                        this_gun = 0;
                    else                        // Gun skipper picked
                        this_gun = 1;
                    
                // Calculate the contents of each shot
                    for (sp = 0; sp < bm->K_num_tot_sp; sp++) {
                        if ((FunctGroupArray[sp].isFished == FALSE) || (bm->SP_FISHERYprms[sp][nf][q_id] == FALSE) || (bm->SP_FISHERYprms[sp][nf][flagPerShotCPUE_id] < 1))
                            continue;  // As not taken by this fishery
                        
                        // Get total catch (in kg) for the species in this box
                        this_catch = 0;
                        Util_Init_1D_Double(bm->prop_depth, bm->wcnz, 0.0);
                        for (k = 0; k < bm->boxes[ij].nz; k++) {
                            catch_left = (bm->Catch[ij][sp][nf][k] * bm->X_CN * mg_2_kg) - bm->alloced_catch[sp][ij][k];
                            if (catch_left < 0.0)
                                catch_left = 0.0;
                            this_catch += catch_left;
                            bm->prop_depth[k] += catch_left;
                            
                            /*
                            if(do_debug) {
                            //if(bm->newmonth) {
                                fprintf(llogfp, "Time %e box%d-%d this_catch: %e catch_left: %e (catch %e allocated_catch: %e)\n",
                                    bm->dayt, ij, k, this_catch, catch_left, (bm->Catch[ij][sp][nf][k] * bm->X_CN * mg_2_kg), bm->alloced_catch[sp][ij][k]);
                            }
                            */
                        }
                        for (k = 0; k < bm->boxes[ij].nz; k++) {
                            bm->prop_depth[k] /= (this_catch + small_num);
                        }
                        // Get total discards (in kg)
                        this_discards = (bm->Discards[ij][sp][nf] * bm->X_CN * mg_2_kg) - bm->alloced_discard[sp][ij];
                        if (this_discards < 0.0)
                            this_discards = 0.0;
                        prop_discard = this_discards / (this_catch + small_num);
                        
                        // Now get the size of the catch - draw from a negative bionomial
                        step1 = drandom(0.0, 1.0);
                        sum_prob = 0.0;
                        this_catchbin = 0;
                        for (nc = 0; nc < bm->K_num_catchbin; nc++){
                            sum_prob = cpue_cdf[sp][nc][this_gun];   // Could use a calculated negative binomal cdf, but reading it in from pre-generated set for now (as cpue_cdf is cumualtive then set sum_prob = not += )
                            
                            //fprintf(llogfp,"gun %d step1: %e nc: %d cpue_cdf: %e sum_prob %e\n", this_gun, step1, nc, cpue_cdf[nc][this_gun], sum_prob);
                            
                            if (step1 < sum_prob) {
                                this_catchbin = nc;
                                break;
                            }
                        }
                        min_catch = (this_catchbin - 1) * bm->size_catchbin;  // in kg
                        if(min_catch < 0.0)
                            min_catch = 0.0;
                        max_catch = this_catchbin * bm->size_catchbin;
                        diff_Catch = max_catch - min_catch;
                        step1 = drandom(0.0, diff_Catch);
                        final_shot_catch = step1 + min_catch;
                        if(final_shot_catch > this_catch)
                            final_shot_catch = this_catch;
                        abscatch += final_shot_catch;
                        
                        /*
                        if(do_debug) {
                            //if(bm->newmonth) {
                            fprintf(llogfp, "Time: %e %s shot %d box %d min_catch: %e max_catch: %e diff_catch: %e final_shot_catch: %e (step1: %e) this_catch: %e this_catchbin: %d size_catchbin: %e\n", bm->dayt, FisheryArray[nf].fisheryCode, this_shot, ij, min_catch, max_catch, diff_Catch, final_shot_catch, step1, this_catch, this_catchbin, bm->size_catchbin);
                        }
                        */
                        
                        // Find corresponding discards
                        step1 = final_shot_catch * prop_discard;
                        final_shot_discards = Assess_Add_Error(bm, bm->CPUEshotfdis, step1, bm->CPUEshotavgdis, bm->CPUEshotvartdis);
                        if(final_shot_discards > this_discards)
                            final_shot_discards = this_discards;
                        
                        // Allocated catch and discards per depth so don't get double counting - also gives average depth for reporting
                        bm->alloced_discard[sp][ij] -= final_shot_discards;
                        weighted_depth = 0.0;
                        catch_unalloced = final_shot_catch;
                        sum_prob = 0.0;
                        
                        // Start with random allocation
                        numstep = 0;
                        while ((catch_unalloced > 0.0) && (numstep < step_tolerance)) {
                            step1 = drandom(0.0, 1.0);
                            for (k = 0; k < bm->boxes[ij].nz; k++) {
                                sum_prob += bm->prop_depth[k];
                                if (step1 < sum_prob) {
                                    allocate_catch(bm, sp, nf, ij, k, &catch_unalloced, &weighted_depth, this_catch, llogfp);
                                }
                            }
                            numstep++;
                        }
                        // Now iteratively allocate any remaining
                        if (catch_unalloced > 0.0) {
                            for (k = 0; k < bm->boxes[ij].nz; k++) {
                                allocate_catch(bm, sp, nf, ij, k, &catch_unalloced, &weighted_depth, this_catch, llogfp);
                            }
                        }
                        
                        weighted_depth /= (final_shot_catch + small_num);
                        
                        // Store results
                        bm->ShotData[this_shot][sp][wdepth_id] = weighted_depth;
                        bm->ShotData[this_shot][sp][tdiscard_id] = final_shot_discards;
                        bm->ShotData[this_shot][sp][tcatch_id] = final_shot_catch;
                        bm->ShotData[this_shot][sp][guru_id] = this_gun;
                        
                        /*
                        if(do_debug) {
                        //if(bm->newmonth) {
                            fprintf(llogfp, "Time: %e %s shot %d %s tcatch: %e tdiscard: %e wdepth: %e\n", bm->dayt, FisheryArray[nf].fisheryCode, this_shot, FunctGroupArray[sp].groupCode, bm->ShotData[this_shot][sp][tcatch_id], bm->ShotData[this_shot][sp][tdiscard_id], bm->ShotData[this_shot][sp][wdepth_id]);
                        }
                        */
                    }
                    
                    // Find out how much effort used per shot
                    x = (abscatch / bm->size_catchbin);
                    min_effort = bm->min_effort_coefft * x + bm->min_effort_const;
                    if(min_effort < 0.0)
                        min_effort = 0.0;
                    this_effort = bm->Effort_hdistrib[ij][nf][today_effort] * 24.0;  // As originally in days and want hours
                    step1 = this_effort - min_effort;
                    
                    final_shot_effort = Assess_Add_Error(bm, bm->CPUEshotfeff, step1, bm->CPUEshotavgeff, bm->CPUEshotvareff) + min_effort;
                    if (final_shot_effort > this_effort)
                        final_shot_effort = this_effort;
                    if (final_shot_effort < 0.0)
                        final_shot_effort = 0.0;
                    
                    /*
                    if(do_debug) {
                        //if(bm->newmonth) {
                        fprintf(llogfp, "Time: %e %s shot %d box %d min_effort: %e x: %e this_effort: %e final_shot_effort: %e\n",
                                bm->dayt, FisheryArray[nf].fisheryCode, this_shot, ij, min_effort, x, this_effort, final_shot_effort);
                    }
                     */
                    
                    for (sp = 0; sp < bm->K_num_tot_sp; sp++) {
                        bm->ShotData[this_shot][sp][teffort_id] = final_shot_effort;
                        bm->ShotData[this_shot][sp][tloc_id] = ij;
                        
                        /*
                        if(do_debug) {
                        //if(bm->newmonth) {
                            fprintf(llogfp, "Time: %e %s shot %d %s teffort: %e tloc: %e\n", bm->dayt, FisheryArray[nf].fisheryCode, this_shot, FunctGroupArray[sp].groupCode, bm->ShotData[this_shot][sp][teffort_id], bm->ShotData[this_shot][sp][tloc_id]);
                        }
                        */
                    }
                    this_shot++;

                }
            
                /* Get the left overs */
                if(bm->flagfullCPUEreport){
                    for (sp = 0; sp < bm->K_num_tot_sp; sp++) {
                        weighted_depth = 0.0;
                        bm->ShotData[this_shot][sp][tcatch_id] = 0.0;
                        for (ij=0; ij < bm->nbox; ij++) {
                            bm->ShotData[this_shot][sp][tloc_id] = ij;
                            bm->ShotData[this_shot][sp][teffort_id] += ((bm->Effort_hdistrib[ij][nf][today_effort] / 24.0 ) - bm->alloced_effort[ij]);
                            for (k = 0; k < bm->boxes[ij].nz; k++) {
                                bm->ShotData[this_shot][sp][wdepth_id] += (bm->Catch[ij][sp][nf][k] * bm->X_CN * mg_2_kg) - bm->alloced_catch[sp][ij][k];
                                bm->ShotData[this_shot][sp][tcatch_id] += (bm->Catch[ij][sp][nf][k] * bm->X_CN * mg_2_kg) - bm->alloced_catch[sp][ij][k];
                                bm->ShotData[this_shot][sp][tdiscard_id] += (bm->Discards[ij][sp][nf] * bm->X_CN * mg_2_kg) - bm->alloced_discard[sp][ij];
                            }
                        }
                        bm->ShotData[this_shot][sp][wdepth_id] /= (bm->ShotData[this_shot][sp][tcatch_id] + small_num);
                    }
                }
                
                /* Find amount allocated overall for repoprting purposes */
                for (sp = 0; sp < bm->K_num_tot_sp; sp++) {
                    tot_alloced[sp] = 0.0;
                    totC = 0.0;
                    for (ij=0; ij < bm->nbox; ij++) {
                        for (k = 0; k < bm->boxes[ij].nz; k++) {
                            totC += (bm->Catch[ij][sp][nf][k] * bm->X_CN * mg_2_kg);
                            tot_alloced[sp] += bm->alloced_catch[sp][ij][k];
                        }
                    }
                    if ( totC > 0.0 )
                        tot_alloced[sp] /= totC;
                    else
                        tot_alloced[sp] = 1.0;  // As nocatch to allocate so all allocated
                }
                
                /* Report the results */
                Write_CPUEreport(bm, nf, llogfp, tot_alloced);
                
            } else {
                quit("Asked for CPUE per shot to be generated for %s but this is only possible for fisheries with effortmodel setting >%d. Reset effortfleet or add code\n", FisheryArray[nf].name, rec_econ_model);
            }
        }
    }

    return;
 
}

/*
 * \brief Allocating geenrated catch to actual depths in selected box 
 */
void allocate_catch(MSEBoxModel *bm, int sp, int nf, int ij, int k, double *catch_unalloced, double *weighted_depth, double this_catch, FILE *llogfp) {
    
    double prop_used, new_sum;
    double catch_left = (bm->Catch[ij][sp][nf][k] * bm->X_CN * mg_2_kg) - bm->alloced_catch[sp][ij][k];
    
    if (catch_left > 0.0) {
        bm->alloced_catch[sp][ij][k] += catch_left;
        *catch_unalloced -= catch_left;
        *weighted_depth += bm->boxes[ij].dz[k] * catch_left;
        prop_used = catch_left / (this_catch + small_num);
        bm->prop_depth[k] -= prop_used;
        if (bm->prop_depth[k] < 0.0)
            bm->prop_depth[k] = 0.0;
        new_sum = 1.0 - prop_used;
        if (new_sum > 0.0) {
            for (ij = 0; ij < bm->nbox; ij++) {
                bm->prop_effort[ij] /= (new_sum + small_num);
            }
        } else {
            *catch_unalloced = 0.0;
        }
    }
    
    return;
}
