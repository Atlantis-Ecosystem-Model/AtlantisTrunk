/**

 \file	atHarvest.c
 \ingroup atHarvestLib
 \brief The main harvest functions.
 \author Beth Fulton 8/8/2004


 <b>Revisions:</b>

 8/8/2004 Created from the semanage.c file

 10/8/2004 moved fisheries activity staus to Set_Fishery_Active()

 20/8/2004 Updated values such as K_num_vert_sp to reflect now a part of
 the MSEBoxModel structure.

 22/8/2004 replaced group specific catch, discards arrays with
 general bm->FCcaught, bm->FCdiscard arrays

 16/8/2005 Put	in default case for all switch statements

 27/9/2005 Added catch and discard time series usage

 18/2/2006 Added scalars to and tsEvalEx so that didn't get under/over
 application of catch given different potential activity patterns
 (at night or in day or all the time etc). Also added CatchSum so
 can track total catch and discards

 17/8/2006 Allowed for case where no direct fishing pressure but catch
 dependent bycatch case (flagdiscard == 4) to run in historical
 fishing cases

 19/11/2006 Added forced F mortality option

 7/3/2007 Added ability to displace historical catch due to MPAs


 21/05/2008 Bec Gorton
 Added extra %e to fprintf on line 1439 to get rid of compiler warning.

 26-05-2008 Bec Gorton
 Changed all references from bm->VERTind to the tracer arrays associated with
 each functional group.

 6/6/2008 Added volume and timestep corrections to catch_at_age and discards_at_age
 stored values (so in correct units for use in the assessment model).

 1/7/2008 Beth Fulton
 To make fisheries code simpler to carry about moved all dt and cell_vol
 corrections on Catch and Discards moved to here (atharvest.c)


 24/11/2008 Bec Gorton
 Fixed a bug in Harvest_Skip_biology. The bm->Catch array was being updated
 in the epibenthic layer using the layer index of 5. I have changed this to 0. Also
 when the totCumCatch was being summed k=5 was being used. I have added a loop to
 sum over the wc layers.

 02-02-2009 Bec Gorton
 Fixed a fprintf statement in Fishing_and_Bycatch that was generating a warning.

 28-10-2009 Bec Gorton
 Removed references to the K_num_fished_sp, K_num_fishedtac_sp and K_num_impacted_sp.
 Arrays that were this size have been resized to K_num_tot_sp.
 Also changed references to the arrays that have been moved into one of the modules.

 02-11-2009 Bec Gorton
 Removed more references to K_num_fished and K_num_impacted.

 04-11-2009 Bec Gorton
 Finished rewriting the getSpeicesFished function.

 16-08-2011 Beth Fulton
 Added DependDiscardsTot so that discards dependent on catch could be represented without the
 full harvest model being used. ALso added the routines Do_Dependent_Discards(),
 Calc_Dependent_Discards_For_Species() and Calc_Dependent_Discards_For_Other_Species()
 to do the calculations

 03-10-2011 Beth Fulton
 Added TotOldCumCatch and TotOldCumDiscards so that can handle multi-year TAC


 ***********/

/*  Open library routines */
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include <math.h>
#include <atHarvest.h>
#include <atHarvestPrivate.h>

/**
 *	These routines are called from Box_Bio_Process() and initialise the fisheries arrays for use in
 *	that cell or layer
 */
void Harvest_Init_Box_Arrays(MSEBoxModel *bm, int ij, FILE *llogfp) {
	int n, sp;

	if (verbose > 1)
		printf("Initialise fisheries arrays\n");

	/* Discards are what interested in for bycatch as not landed so store it */
	for (sp = 0; sp < bm->K_num_tot_sp; sp++) {
		if (FunctGroupArray[sp].isFished == FALSE && FunctGroupArray[sp].isTAC) {
			for (n = 0; n < bm->K_num_fisheries; n++) {
				TotCumCatch[sp][n][bm->thisyear] += bm->Discards[ij][sp][n];

				/***/
				//if((sp == bm->which_check) &&  (bm->Discards[ij][sp][n] > 0))
				//					fprintf(llogfp, "Time: %e, %s %s box-%d TotCumCatch: %e, Discards: %e \n",
				//						bm->dayt, FunctGroupArray[sp].groupCode, FisheryArray[n].fisheryCode, ij,
				//						TotCumCatch[sp][n][bm->thisyear] * bm->X_CN * mg_2_kg,
				//						bm->Discards[ij][sp][n] * bm->X_CN * mg_2_kg);
				/**/

			}
		}
	}

	/* For target species its not a given that discards are included TAC accounting */
	if (bm->flagTACincludeDiscard) {
		for (sp = 0; sp < bm->K_num_tot_sp; sp++) {
			if (FunctGroupArray[sp].isFished == TRUE) {
				for (n = 0; n < bm->K_num_fisheries; n++) {
					TotCumCatch[sp][n][bm->thisyear] += bm->Discards[ij][sp][n];

					/**
					if((sp == bm->which_check) && (bm->Discards[ij][sp][n] > 0))
                      fprintf(llogfp, "Time: %e, %s %s box-%d TotCumCatch: %e Discards: %e \n",
                              bm->dayt, FunctGroupArray[sp].groupCode, FisheryArray[n].fisheryCode, ij, TotCumCatch[sp][n][bm->thisyear], bm->Discards[ij][sp][n]);
                    **/
				}
			}
		}
	}

	for (sp = 0; sp < bm->K_num_tot_sp; sp++) {
		if (FunctGroupArray[sp].isImpacted == TRUE) {
			for (n = 0; n < bm->K_num_fisheries; n++) {
				bm->CumDiscards[sp][n][ij] += bm->Discards[ij][sp][n];
				bm->TotCumDiscards[sp][n] += bm->Discards[ij][sp][n];
                
                /**
                if((sp == bm->which_check) && (bm->Discards[ij][sp][n] > 0))
                   fprintf(llogfp, "Time: %e, box %d %s %s TotCumDiscards %e discards: %e\n",
                           bm->dayt, ij, FunctGroupArray[sp].groupCode, FisheryArray[n].fisheryCode,
                           bm->TotCumDiscards[sp][n], bm->Discards[ij][sp][n]);
                **/

                bm->Discards[ij][sp][n] = 0.0;
                
			}
            
            for (n = 0; n < bm->K_num_fisheries; n++) {
                if(FisheryArray[n].isRec == TRUE){
                    TotCumCatch[sp][n][bm->thisyear] += bm->RecCatch[ij][sp][n];
				}

				/**
				//if((k == FPO_id) && (bm->RecCatch[n][k] > 0))
				//					fprintf(llogfp, "Time: %e, %s %s box-%d TotCumCatch: %e reccatch: %e \n",
				//						bm->dayt, FunctGroupArray[sp].groupCode,
				//						FisheryArray[recfish_id].fisheryCode, n,
				//						TotCumCatch[sp][recfish_id][bm->thisyear] * bm->X_CN * mg_2_kg,
				//						 bm->RecCatch[n][sp] * bm->X_CN * mg_2_kg);
				**/

			}
		}
	}

	for (sp = 0; sp < bm->K_num_tot_sp; sp++) {
		if (FunctGroupArray[sp].isVertebrate == TRUE) {
			for (n = 0; n < FunctGroupArray[sp].numCohortsXnumGenes; n++) {
				bm->FCcaught[sp][n] = 0.0;
				bm->FCdiscard[sp][n] = 0.0;
			}
		}
	}

	/* Rec-fisheries data */
	for (sp = 0; sp < bm->K_num_tot_sp; sp++) {
		if (FunctGroupArray[sp].isImpacted == TRUE) {
            for (n = 0; n < bm->K_num_fisheries; n++) {
                bm->TotCumRecCatch[sp][n] += bm->RecCatch[ij][sp][n];
                bm->RecCatch[ij][sp][n] = 0;
            }
		}
	}

	return;
}

/**
 * 	\brief Initialise the fishery layer arrays.
 *
 * This function is called in the biology module before the AdaptDiff function is called.
 *
 *
 *
 *
 */
void Harvest_Init_Layer_Arrays(MSEBoxModel *bm, int ij, int nl, FILE *llogfp) {
	int sp, n;

	if (verbose > 1)
		printf("Initialise fisheries arrays\n");
    
	for (sp = 0; sp < bm->K_num_tot_sp; sp++) {
		/* really shouldn't be including impacted species but had to to get vicMPA WDG to work */
		if (FunctGroupArray[sp].isFished == TRUE) {
			for (n = 0; n < bm->K_num_fisheries; n++) {

                if(bm->Catch[ij][sp][n][nl] > 0.0) {
                    bm->TripCatch[sp][n] += bm->Catch[ij][sp][n][nl];
                    TotCumCatch[sp][n][bm->thisyear] += bm->Catch[ij][sp][n][nl];
                    bm->TotCumBiCatch[sp][n] += bm->Catch[ij][sp][n][nl];
                    bm->CumCatch[sp][n][ij][nl] += bm->Catch[ij][sp][n][nl];
                } 

//				if(bm->CumCatch[sp][n][ij][nl] > 0)
//					fprintf(bm->logFile, "bm->CumCatch[%s][%s][ij][nl] = %e\n", FunctGroupArray[sp].groupCode,  FisheryArray[n].fisheryCode, bm->CumCatch[sp][n][ij][nl]);
				//fprintf(bm->logFile, "bm->TripCatch[%s][%d] = %e\n", FunctGroupArray[sp].groupCode, n, bm->TripCatch[sp][n]);
                
                /* Re-initialise */
				bm->Catch[ij][sp][n][nl] = 0.0;
			}
		}
	}
    
    //if(bm->flagecon_on && bm->newweek) {
    if(bm->newweek) {
        if ((bm->current_box == bm->first_box) && (bm->current_layer == bm->top_layer)){ // So only done once per week
            for (sp = 0; sp < bm->K_num_tot_sp; sp++) {
                for (n = 0; n < bm->K_num_fisheries; n++) {
                    bm->TripCatch[sp][n] = 0.0;
                }
            }
        }
        
    }
        
    
	return;
}
/**
 *
 *	\brief This function updates the LastCatch array (for use in calculating recent CPUE) and
 *  manages the CatchQueue (so can do a running average for LastCatch)
 *
 *	This function is called from the main loop after the all the biology is done.
 *
 *	Assumes all ids to be reset sit within the pBox->fstat vector
 */
void Harvest_Update_Temp_Catch_Array(MSEBoxModel *bm, FILE *llogfp) {
	int sp, n, ij, nl;
	int d = bm->K_num_catchqueue - 1;
	double list_length = (double) (bm->K_num_catchqueue);

	/* Obviously for the first k_length_catchqueue time steps you
	 really not taking anything off the queue, just building it up,
	 so fill it up initially with the first value over and over.

	 Also setting up first value of LastCatch to work from.
	 */

	if (bm->t < (2.0 * bm->dt)) {
		for (sp = 0; sp < bm->K_num_tot_sp; sp++) {
			if (FunctGroupArray[sp].isFished == TRUE) {
				for (n = 0; n < bm->K_num_fisheries; n++) {
					for (ij = 0; ij < bm->nbox; ij++) {

						/* Set up the catch queue */
						for (d = 0; d < bm->K_num_catchqueue; d++) {
							bm->CatchQueue[sp][n][ij][d] = 0;
							for (nl = 0; nl < bm->boxes[ij].nz; nl++) {
								bm->CatchQueue[sp][n][ij][d] += (bm->Catch[ij][sp][n][nl] / list_length);
							}
						}

						/* Store the first value of LastCatch (could simply sum over the catch
						 too, but hard to guess which is faster when both numlayers and
						 queue length can vary so stick with queue length here - once off
						 calculation so not too expensive anyway).
						 */
						bm->LastCatch[sp][n][ij] = 0.0;
						for (d = 0; d < bm->K_num_catchqueue; d++) {
							bm->LastCatch[sp][n][ij] += bm->CatchQueue[sp][n][ij][d];
						}
					}
				}
			}

		}

	} else {
		/* Standard case - once queue initialised */
		for (sp = 0; sp < bm->K_num_tot_sp; sp++) {
			if (FunctGroupArray[sp].isFished == TRUE) {
				for (n = 0; n < bm->K_num_fisheries; n++) {
					for (ij = 0; ij < bm->nbox; ij++) {
						bm->LastCatch[sp][n][ij] -= bm->CatchQueue[sp][n][ij][d];

						/* Shift everything along the CatchQueue one spot */
						for (d = 0; d < bm->K_num_catchqueue - 1; d++) {
							bm->CatchQueue[sp][n][ij][d + 1] = bm->CatchQueue[sp][n][ij][d];
						}
						/* Reinitialise first (most recent) spot in the queue as about to fill it */
						bm->CatchQueue[sp][n][ij][0] = 0;

						for (nl = 0; nl < bm->boxes[ij].nz; nl++) {
							bm->CatchQueue[sp][n][ij][0] += (bm->Catch[ij][sp][n][nl] / list_length);
						}

						/* Update LastCatch */
						bm->LastCatch[sp][n][ij] += bm->CatchQueue[sp][n][ij][0];

					}
				}
			}
		}
	}
	return;
}
/**
 * \brief This function is called from the main loop after the fisheries statistics have been output.
 *	It resets the pools so cumulative storage can be restarted.
 *
 *	Assumes all ids to be reset sit within the pBox->fstat vector
 */
void Harvest_Refresh_Fishing_Stats(MSEBoxModel *bm) {
	int b, id;

	if (verbose)
		printf("Resetting fisheries statistics\n");

	for (b = 0; b < bm->nbox; b++) {
		//for (id = bm->min_fstatid; id < bm->max_fstatid + 1; id++) {
		for (id = 0; id < bm->nfstat; id++) {
			bm->boxes[b].fishstat[id] = 0;
		}
	}
}

/**
 *	\brief This subroutine determines which consumer groups and fisheries are active,
 *	light limitation prevents the primary producers from being active at night.
 *	A 2 indicates no preference, 1 = day, 0 = night and 3 = fishery not implemented.
 *
 */
void Harvest_Set_Fishery_Active(MSEBoxModel *bm, FILE *llogfp) {
	int nf;

	for (nf = 0; nf < bm->K_num_fisheries; nf++) {
		if ((bm->FISHERYprms[nf][fisheriesflagactive_id] == 2) || (bm->flagday == bm->FISHERYprms[nf][fisheriesflagactive_id]))
			bm->FISHERYprms[nf][fisheriesactive_id] = 1;
		else
			bm->FISHERYprms[nf][fisheriesactive_id] = 0;
	}

	return;
}

/**
 *	Fishing mortality for target groups (vertebrate and invertebrate.
 *	This subroutine works out the pressure put on the groups
 *	impacted by fishing and then determines the numbers caught and discarded.
 *	It incorporates fishing induced incidental mortality on fish and invertebrates.
 */
int Harvest_Do_Fishing_And_ByCatch(MSEBoxModel *bm, FILE *llogfp, int guildcase, int chrt, double SC, double RC, double NUMS, double **FishingRes,
		double *numsdead, double *waste) {
	double li, loadDetFC, FCtoDR, SPtoFC, fishing, age_catch, discards, Wgt, Dens, Biom, deadnums, age_discard, vert_scale,
			gear_change_scale, discard_change_scale, survivors, quota, prop_dis_dead, flagrecfish, fishery_start, fishery_end, loadFC, mpa_losses,
            FCwaste, on_deck, waste_offloaded;
	int i, j, nf, sp, stage, flagspfish, flagimposecatch, boxkey_id;
	int do_debug = 0, do_debug_dis = 0, do_debug_orig = 0, do_debug_dis_orig = 0, do_debug_econ = 0, do_debug_econ_orig = 0;
	int depend_dis;

	/* Comment out the debug flags for speed - only uncomment as needed */
	if( ((bm->current_box == bm->checkbox) || (bm->checkbox > bm->nbox)) && (bm->dayt >= bm->checkstart) && (bm->dayt
			<= bm->checkstop) ){
		if (((bm->debug == debug_fishing) || (bm->debug == debug_histfishing)) && ((guildcase == bm->which_check) || (bm->which_check >= not_tracking_flux))) {
			do_debug_orig = 1;
		} else
			do_debug_orig = 0;

		if ((bm->debug == debug_discard) && ((guildcase == bm->which_check) || (bm->which_check >= not_tracking_flux))) {
			do_debug_dis_orig = 1;
		} else
			do_debug_dis_orig = 0;

		if ((((bm->debug == debug_econeffort) && (!bm->debug_it)) || (bm->debug == debug_econ)) && ((guildcase == bm->which_check) || (bm->which_check
				== not_tracking_flux))) {
			do_debug_econ_orig = 1;
		} else
			do_debug_econ_orig = 0;
	}

	//fprintf(llogfp,"Time: %e, %s, do_debug_orig: %d (debug: %d which_check: %s, checkbox: %d, checkstart: %e, checkstop: %e\n",
	//	bm->dayt, bm->spNAME[guildcase], do_debug_orig, bm->debug, bm->spNAME[bm->which_check], bm->checkbox, bm->checkstart, bm->checkstop);

	//if (verbose > 3)
	//	printf("Fishing and Bycatch\n");

	if (do_debug_orig || do_debug_dis_orig)
		fprintf(llogfp, "Fishing and bycatch %d:%d, SC = %.20e, RC = %.20e, NUMS = %.20e\n", guildcase, chrt, SC, RC, NUMS);

	flagspfish = 1;

	/* Determine local biomass for vertebrates (invertebrates assume SC entry = Biomass */
	if (FunctGroupArray[guildcase].isVertebrate == TRUE) {
		Wgt = SC + RC;
		if (Wgt < small_num)
			Wgt = small_num; /* To avoid divide by zero problems - Biom will
			 still be zero as NUMS will be zero in this instance too	*/
		Dens = NUMS / (bm->cell_vol + small_num);
		Biom = Wgt * Dens;
	} else if (FunctGroupArray[guildcase].habitatType == EPIFAUNA) {
		Biom = SC / wcLayerThick;
		Wgt = 1.0;
	} else {
		Biom = SC;
		Wgt = 1.0;
	}

    /* Initilise returns */
	for (i = 0; i < bm->K_num_fisheries; i++) {
		for (j = 0; j < DiscardsAtAge_result_id + 1; j++)
			FishingRes[i][j] = 0.0;
	}
	*waste = 0.0;
	*numsdead = 0.0;

	flagspfish = (int) (FunctGroupArray[guildcase].speciesParams[flagfish_id]);

	sp = guildcase;
	stage = FunctGroupArray[guildcase].cohort_stage[chrt];

	/* Convert model weight (mg AFDSW) into g wet weight and then length in cm */
	if (FunctGroupArray[guildcase].isVertebrate == TRUE) {
		li = Ecology_Get_Size(bm, guildcase, Wgt, chrt);

		vert_scale = bm->boxes[bm->current_box].vert_vdistrib[guildcase][stage][bm->current_layer];

	} else {
		li = Ecology_Get_Size(bm, guildcase, Wgt, chrt);

		if (FunctGroupArray[guildcase].isImpacted == TRUE)
			vert_scale = bm->boxes[bm->current_box].vert_vdistrib[guildcase][stage][bm->current_layer];
		else
			vert_scale = 1.0;
	}

	deadnums = 0.0;

	if (!bm->flagincidmort && !flagspfish && !bm->flagfinfish) {
		/* Nothing to actually do so stop now */
		return FALSE;
	}

	for (nf = 0; nf < bm->K_num_fisheries; nf++) {
        FCwaste = 0.0;

		fishery_end = bm->FISHERYprms[nf][tEnd_id];
		fishery_start = bm->FISHERYprms[nf][tStart_id];


		/* Check if fishery even allowed - fishery may not have started yet or may have been
		 ended by legislation (e.g. international fisheries kicked out of EEZ
		 */
		if ((bm->dayt < fishery_start) || (bm->dayt > fishery_end))
			continue;

		gear_change_scale = 1.0;
		discard_change_scale = 1.0;
		SPtoFC = 0;
		flagrecfish = (int) (bm->FISHERYprms[nf][flagrecfish_id]);
		//flagmanage = (int) (bm->FISHERYprms[nf][flagmanage_id]);
		prop_dis_dead = bm->SP_FISHERYprms[guildcase][nf][incidmort_id];

		if(do_debug_orig && (bm->which_fleet == nf || bm->which_fleet >= bm->K_num_fisheries)){
			do_debug = 1;
		}else{
			do_debug = 0;
		}

		if(do_debug_dis_orig && (bm->which_fleet == nf || bm->which_fleet >= bm->K_num_fisheries)){
			do_debug_dis = 1;
		}else{
			do_debug_dis = 0;
		}

		//if(do_debug_econ_orig && (bm->which_fleet < bm->K_num_fisheries)){
		if (do_debug_econ_orig && (bm->FISHERYprms[nf][flageffortmodel_id] > rec_econ_model)) {
			if (bm->which_fleet == nf)
				do_debug_econ = 1;
			else
				do_debug_econ = 0;
		} else if (do_debug_econ_orig) {
			/* As must mean which fleet >= K_num_fisheries */
			do_debug_econ = 1;
		}

		/* Check for imposed catch and dependent discards */
		if ((int) (bm->SP_FISHERYprms[sp][nf][flagdiscard_id]) == depend_discard)
			depend_dis = 1;
		else
			depend_dis = 0;

		flagimposecatch = (int) (bm->SP_FISHERYprms[sp][nf][flagimposecatch_id]);
		if ((bm->dayt < bm->SP_FISHERYprms[sp][nf][imposecatchstart_id]) || (bm->dayt > bm->SP_FISHERYprms[sp][nf][imposecatchend_id]))
			flagimposecatch = 0;

		if (flagimposecatch || depend_dis)
			boxkey_id = bm->BoxKeyMap[bm->current_box][catchkey_id];
		else
			boxkey_id = 0;

		if (flagimposecatch && FunctGroupArray[sp].isFished == FALSE) {
			quit("%s can not have catch imposed, use another option or recode\n", FunctGroupArray[sp].groupCode);
		}

		mpa_losses = 0.0;
		/* Get the fishing catch value */
		if (Get_Catch(bm, guildcase, chrt, stage, nf, do_debug, flagimposecatch, depend_dis, boxkey_id, Biom, li, vert_scale, &gear_change_scale, &loadFC, &SPtoFC, &mpa_losses, llogfp) < 0) {
			continue;
		}
        if(SPtoFC < 0.0)
            SPtoFC = 0.0;

		/*
		if(do_debug){
			//fprintf(llogfp, "Catch Outcomes %s:%d SPtoFC = %.20e\n", FunctGroupArray[sp].groupCode, chrt, SPtoFC);
			fprintf(llogfp, "Time: %e %s Catch Outcomes %s:%d SPtoFC = %.20e\n",
								bm->dayt, FisheryArray[nf].fisheryCode, FunctGroupArray[sp].groupCode, chrt, SPtoFC);
		}    
		*/    
        
		/* Fisheries discarding - assumes all discards are dead
		 * This is the proportion caught that is discarded
		 */
		loadDetFC = Get_Discards(bm, guildcase, chrt, stage, nf, do_debug, do_debug_econ, do_debug_dis, flagimposecatch, boxkey_id, Biom, li, vert_scale, loadFC, &SPtoFC, &quota, &FCwaste, mpa_losses, depend_dis, llogfp);

		/* Calculate total discards - minus any that are discarded and survive the experience */
		FCtoDR = loadDetFC * SPtoFC * prop_dis_dead;
        if(FCtoDR < 0.0)
            FCtoDR = 0.0;
        
		/* Calculate the proportion of the catch that is discarded that survive the experience */
		survivors = loadDetFC * SPtoFC * (1.0 - prop_dis_dead);
        if(survivors < 0.0)
            survivors = 0.0;

        /* Calculuate offal and other materials thrown overboard (there can be no survivors so split apart from general discards) */
        on_deck = SPtoFC - FCtoDR - survivors;
        waste_offloaded = FCwaste * on_deck;
        FCtoDR += waste_offloaded;
        
		/* Correct for those that survive discarding */
		(*waste) += FCtoDR;

		/**/
		if (!(_finite(*waste))) {
			fprintf(llogfp, "Time: %e, %d-%d %s-%d (sp%d-cat%d) %s waste: %e, loatDetFC final: %e (FCtoDR: %e, SPtoFC: %e, prop_dis_dead: %e)\n", bm->dayt,
					bm->current_box, bm->current_layer, FunctGroupArray[sp].groupCode, chrt, sp, guildcase, FisheryArray[nf].fisheryCode, (*waste), loadDetFC,
					FCtoDR, SPtoFC, prop_dis_dead);

		}
		/**/

		/* Total landed catch */
		fishing = SPtoFC - FCtoDR - survivors;

		if (fishing < 0.0)
			fishing = 0.0;

		/* Biomass discarded */
		discards = FCtoDR;
		if (discards < 0.0)
			discards = 0.0;

		/* Numbers caught and discarded (for age frequency of catch) */
		age_catch = fishing / Wgt;
		age_discard = discards / Wgt;

		/* Number of dead */
		deadnums += (SPtoFC - survivors) * bm->cell_vol / Wgt;

        /* Assign results - correcting for volume and timestep size (as sampled once per timestep) */
		FishingRes[nf][Catch_result_id] = fishing * bm->dt * bm->cell_vol;
		FishingRes[nf][CatchAtAge_result_id] = age_catch * bm->dt * bm->cell_vol;
		FishingRes[nf][Discards_result_id] = discards * bm->dt * bm->cell_vol;
		FishingRes[nf][DiscardsAtAge_result_id] = age_discard * bm->dt * bm->cell_vol;

		if (!flagrecfish) {
			CatchSum[guildcase][tscocatch_id] += fishing * bm->cell_vol * bm->dt;
			CatchSum[guildcase][tscodiscard_id] += discards * bm->cell_vol * bm->dt;
		}
        

        /**
         if((do_debug || do_debug_dis || do_debug_econ) && bm->newweek){
			fprintf(llogfp,"time: %e, sp: %s, chrt: %d, nf: %s, box: %d-%d, Catchsum: %e (vs quota: %e), DiscardSum: %e\n",
				bm->dayt, FunctGroupArray[sp].groupCode, chrt, FisheryArray[nf].fisheryCode, bm->current_box, bm->current_layer, CatchSum[sp][tscocatch_id], bm->TACamt[sp][nf][now_id] * kg_2_mg / bm->X_CN, CatchSum[sp][tscodiscard_id]);
			fprintf(llogfp,"time: %e, fishres: %e, totbiom: %e, SPtoFC: %e, Wgt: %e, bm->cell_vol: %e, fishing: %e, discards: %e, loadDetFC: %e prop_dis_dead: %e, survivors: %e, FCtoDR: %e\n",
				bm->dayt, FishingRes[nf][Catch_result_id], Biom * bm->cell_vol, SPtoFC, Wgt, bm->cell_vol, fishing, discards, loadDetFC, prop_dis_dead, survivors, FCtoDR);
			fprintf(llogfp,"(quota: %e, TripCatch: %e, catch_allowed: %e, TotCumCatch: %e, prop_within: %e) Effort[%d][%d]: %e\n",
				quota, bm->TripCatch[sp][nf], bm->SP_FISHERYprms[guildcase][nf][catch_allowed], TotCumCatch[sp][nf][bm->thisyear], bm->prop_within, bm->current_box, nf, bm->Effort[bm->current_box][nf]);
		 }
		 **/

		/* Update fisheries performance indices */
		harvestindx[nf][avg_sze_id] += fishing;
		harvestindx[nf][tot_num_caught_id] += age_catch;
		if (!harvestindx[nf][checkdone_id]) {
			harvestindx[nf][gearchange_id] += gear_change_scale;
			harvestindx[nf][discardchange_id] += discard_change_scale;
			harvestindx[nf][checkdone_id]++;
		}

		/* Update the discards values */
		bm->Discards[bm->current_box][guildcase][nf] += bm->FishingResults[nf][Discards_result_id];

		/* Update any impacts on rugosity */
        
        
	}

	/* Return total number dead */
	*numsdead = deadnums;

    /*
	if(guildcase == bm->which_check) {
		fprintf(llogfp,"time: %e box%d-%d sp: %s chrt: %d adding numsdead: %e\n",
                bm->dayt, bm->current_box, bm->current_layer, FunctGroupArray[guildcase].groupCode, chrt, deadnums);
    }
    */

	return TRUE;

}

static void Habitat_Overlap(MSEBoxModel *bm, int b, int sp, int stage, int nf, int check1, int check2, int check3, int check4) {

	double newvalue;
	double sp_likeREEF, sp_likeFLAT, sp_likeSOFT, sp_likeCANYON;
	double FC_likeREEF, FC_likeFLAT, FC_likeSOFT, FC_likeCANYON;

	/* Group's habitat preferences */
	sp_likeREEF = bm->HABITATlike[sp][stage][bm->REEFcover_id];
	sp_likeFLAT = bm->HABITATlike[sp][stage][bm->FLATcover_id];
	sp_likeSOFT = bm->HABITATlike[sp][stage][bm->SOFTcover_id];
	sp_likeCANYON = bm->HABITATlike[sp][stage][bm->CANYONcover_id];

	/* Fisheries' habitat preferences */
	FC_likeREEF = bm->FC_HABITATlike[nf][bm->REEFcover_id];
	FC_likeFLAT = bm->FC_HABITATlike[nf][bm->FLATcover_id];
	FC_likeSOFT = bm->FC_HABITATlike[nf][bm->SOFTcover_id];
	FC_likeCANYON = bm->FC_HABITATlike[nf][bm->CANYONcover_id];

	//	fprintf(bm->logFile, "sp_likeREEF = %d, sp_likeFLAT = %d, sp_likeSOFT = %d, sp_likeCANYON = %d\n", sp_likeREEF, sp_likeFLAT, sp_likeSOFT, sp_likeCANYON);
	//	fprintf(bm->logFile, "FC_likeREEF = %d, FC_likeFLAT = %d, FC_likeSOFT = %d, FC_likeCANYON = %d\n", FC_likeREEF, FC_likeFLAT, FC_likeSOFT, FC_likeCANYON);
	//	fprintf(bm->logFile, "check1 = %d, check2 = %d, check3 = %d, check4 = %d\n", check1, check2, check3, check4);
	//
	//	fprintf(bm->logFile, "b = %d, bm->coveramt[bm->FLATcover_id][b] = %e, bm->coveramt[bm->REEFcover_id][b] = %e, bm->coveramt[bm->SOFTcover_id][b] = %e, bm->coveramt[bm->CANYONcover_id][b] = %e\n", b, bm->coveramt[bm->FLATcover_id][b], bm->coveramt[bm->REEFcover_id][b], bm->coveramt[bm->SOFTcover_id][b], bm->coveramt[bm->CANYONcover_id][b]);
	newvalue = (sp_likeFLAT * FC_likeFLAT * bm->coveramt[bm->FLATcover_id][b]) + (sp_likeREEF * FC_likeREEF * bm->coveramt[bm->REEFcover_id][b]) + (sp_likeSOFT
			* FC_likeSOFT * bm->coveramt[bm->SOFTcover_id][b]);
	//		* 	(sp_likeCANYON * FC_likeCANYON * (1 + bm->coveramt[bm->CANYONcover_id][b]))
	//		* 	((sp_likeCANYON && !FC_likeCANYON) * (1.0 - bm->coveramt[bm->CANYONcover_id][b]))
	//		* 	((!sp_likeCANYON && FC_likeCANYON) * (1.0));


	newvalue = (sp_likeFLAT * FC_likeFLAT * bm->coveramt[bm->FLATcover_id][b]) + (!sp_likeFLAT * FC_likeFLAT * bm->coveramt[bm->FLATcover_id][b])
			+ (!sp_likeFLAT * !FC_likeFLAT * bm->coveramt[bm->FLATcover_id][b]) + (sp_likeREEF * FC_likeREEF * bm->coveramt[bm->REEFcover_id][b])
			+ (!sp_likeREEF * FC_likeREEF * bm->coveramt[bm->REEFcover_id][b]) + (!sp_likeREEF * !FC_likeREEF * bm->coveramt[bm->REEFcover_id][b])
			+ (sp_likeSOFT * FC_likeSOFT * bm->coveramt[bm->SOFTcover_id][b]) + (!sp_likeSOFT * FC_likeSOFT * bm->coveramt[bm->SOFTcover_id][b])
			+ (!sp_likeSOFT * !FC_likeSOFT * bm->coveramt[bm->SOFTcover_id][b]);

	if (sp_likeCANYON && FC_likeCANYON)
		newvalue = newvalue * (1.0 + bm->coveramt[bm->CANYONcover_id][b]);
	else if (sp_likeCANYON)
		newvalue = newvalue * (1.0 - bm->coveramt[bm->CANYONcover_id][b]);

	newvalue = min(1.0, newvalue);

	/* Both the group and the fishery operate over soft ground */
	/* Determine Overlap */
	if (check1) { /* Both on reefs */
		if (check2) { /* Both on flats */
			if (check3) { /* Both on soft */
				if (check4) { /* Both in canyons */
					p_fish_origi[b][sp][stage][nf] = 1.0;
				} else if (sp_likeCANYON) { /* Not conincident because fishery not in canyons */
					p_fish_origi[b][sp][stage][nf] = 1.0 - bm->coveramt[bm->CANYONcover_id][b];
				} else if (FC_likeCANYON) { /* Not conincident because fish not in canyons */
					p_fish_origi[b][sp][stage][nf] = 1.0;
				} else { /* Neither in canyons */
					p_fish_origi[b][sp][stage][nf] = 1.0;
				}
			} else if (sp_likeSOFT) { /* Not conincident because fishery not on soft */
				if (check4) { /* Both in canyons */
					p_fish_origi[b][sp][stage][nf] = (bm->coveramt[bm->REEFcover_id][b] + bm->coveramt[bm->FLATcover_id][b]) * (1.0
							+ bm->coveramt[bm->CANYONcover_id][b]);
				} else if (sp_likeCANYON) { /* Not conincident because fishery not in canyons */
					p_fish_origi[b][sp][stage][nf] = (bm->coveramt[bm->REEFcover_id][b] + bm->coveramt[bm->FLATcover_id][b]) * (1.0
							- bm->coveramt[bm->CANYONcover_id][b]);
				} else if (FC_likeCANYON) { /* Not conincident because fish not in canyons */
					p_fish_origi[b][sp][stage][nf] = bm->coveramt[bm->REEFcover_id][b] + bm->coveramt[bm->FLATcover_id][b];
				} else { /* Neither in canyons */
					p_fish_origi[b][sp][stage][nf] = bm->coveramt[bm->REEFcover_id][b] + bm->coveramt[bm->FLATcover_id][b];
				}
			} else if (FC_likeSOFT) { /* Not conincident because fish not on soft */
				if (check4) { /* Both in canyons */
					p_fish_origi[b][sp][stage][nf] = 1.0;
				} else if (sp_likeCANYON) { /* Not conincident because fishery not in canyons */
					p_fish_origi[b][sp][stage][nf] = 1.0 - bm->coveramt[bm->CANYONcover_id][b];
				} else if (FC_likeCANYON) { /* Not conincident because fish not in canyons */
					p_fish_origi[b][sp][stage][nf] = 1.0;
				} else { /* Neither in canyons */
					p_fish_origi[b][sp][stage][nf] = 1.0;
				}
			} else { /* Neither on soft */
				if (check4) { /* Both in canyons */
					p_fish_origi[b][sp][stage][nf] = 1.0;
				} else if (sp_likeCANYON) { /* Not conincident because fishery not in canyons */
					p_fish_origi[b][sp][stage][nf] = 1.0 - bm->coveramt[bm->CANYONcover_id][b];
				} else if (FC_likeCANYON) { /* Not conincident because fish not in canyons */
					p_fish_origi[b][sp][stage][nf] = 1.0;
				} else { /* Neither in canyons */
					p_fish_origi[b][sp][stage][nf] = 1.0;
				}
			}
		} else if (sp_likeFLAT) { /* Not conincident because fishery not on flats */
			if (check3) { /* Both on soft */
				if (check4) { /* Both in canyons */
					p_fish_origi[b][sp][stage][nf] = (bm->coveramt[bm->REEFcover_id][b] + bm->coveramt[bm->SOFTcover_id][b]) * (1.0
							+ bm->coveramt[bm->CANYONcover_id][b]);
				} else if (sp_likeCANYON) { /* Not conincident because fishery not in canyons */
					p_fish_origi[b][sp][stage][nf] = (bm->coveramt[bm->REEFcover_id][b] + bm->coveramt[bm->SOFTcover_id][b]) * (1.0
							- bm->coveramt[bm->CANYONcover_id][b]);
				} else if (FC_likeCANYON) { /* Not conincident because fish not in canyons */
					p_fish_origi[b][sp][stage][nf] = bm->coveramt[bm->REEFcover_id][b] + bm->coveramt[bm->SOFTcover_id][b];
				} else { /* Neither in canyons */
					p_fish_origi[b][sp][stage][nf] = bm->coveramt[bm->REEFcover_id][b] + bm->coveramt[bm->SOFTcover_id][b];
				}
			} else if (sp_likeSOFT) { /* Not conincident because fishery not on soft */
				if (check4) { /* Both in canyons */
					p_fish_origi[b][sp][stage][nf] = bm->coveramt[bm->REEFcover_id][b] * (1.0 + bm->coveramt[bm->CANYONcover_id][b]);
				} else if (sp_likeCANYON) { /* Not conincident because fishery not in canyons */
					p_fish_origi[b][sp][stage][nf] = bm->coveramt[bm->REEFcover_id][b] * (1.0 - bm->coveramt[bm->CANYONcover_id][b]);
				} else if (FC_likeCANYON) { /* Not conincident because fish not in canyons */
					p_fish_origi[b][sp][stage][nf] = bm->coveramt[bm->REEFcover_id][b];
				} else { /* Neither in canyons */
					p_fish_origi[b][sp][stage][nf] = bm->coveramt[bm->REEFcover_id][b];
				}
			} else if (FC_likeSOFT) { /* Not conincident because fish not on soft */
				if (check4) { /* Both in canyons */
					p_fish_origi[b][sp][stage][nf] = bm->coveramt[bm->REEFcover_id][b] * (1.0 + bm->coveramt[bm->CANYONcover_id][b]);
				} else if (sp_likeCANYON) { /* Not conincident because fishery not in canyons */
					p_fish_origi[b][sp][stage][nf] = bm->coveramt[bm->REEFcover_id][b] * (1.0 - bm->coveramt[bm->CANYONcover_id][b]);
				} else if (FC_likeCANYON) { /* Not conincident because fish not in canyons */
					p_fish_origi[b][sp][stage][nf] = bm->coveramt[bm->REEFcover_id][b];
				} else { /* Neither in canyons */
					p_fish_origi[b][sp][stage][nf] = bm->coveramt[bm->REEFcover_id][b];
				}
			} else { /* Neither on soft */
				if (check4) { /* Both in canyons */
					p_fish_origi[b][sp][stage][nf] = bm->coveramt[bm->REEFcover_id][b] * (1.0 + bm->coveramt[bm->CANYONcover_id][b]);
				} else if (sp_likeCANYON) { /* Not conincident because fishery not in canyons */
					p_fish_origi[b][sp][stage][nf] = bm->coveramt[bm->REEFcover_id][b] * (1.0 - bm->coveramt[bm->CANYONcover_id][b]);
				} else if (FC_likeCANYON) { /* Not conincident because fish not in canyons */
					p_fish_origi[b][sp][stage][nf] = bm->coveramt[bm->REEFcover_id][b];
				} else { /* Neither in canyons */
					p_fish_origi[b][sp][stage][nf] = bm->coveramt[bm->REEFcover_id][b];
				}
			}
		} else if (FC_likeFLAT) { /* Not conincident because fish not on flats */
			if (check3) { /* Both on soft */
				if (check4) { /* Both in canyons */
					p_fish_origi[b][sp][stage][nf] = (bm->coveramt[bm->REEFcover_id][b] + bm->coveramt[bm->SOFTcover_id][b]) * (1.0
							+ bm->coveramt[bm->CANYONcover_id][b]);
				} else if (sp_likeCANYON) { /* Not conincident because fishery not in canyons */
					p_fish_origi[b][sp][stage][nf] = (bm->coveramt[bm->REEFcover_id][b] + bm->coveramt[bm->SOFTcover_id][b]) * (1.0
							- bm->coveramt[bm->CANYONcover_id][b]);
				} else if (FC_likeCANYON) { /* Not conincident because fish not in canyons */
					p_fish_origi[b][sp][stage][nf] = bm->coveramt[bm->REEFcover_id][b] + bm->coveramt[bm->SOFTcover_id][b];
				} else { /* Neither in canyons */
					p_fish_origi[b][sp][stage][nf] = bm->coveramt[bm->REEFcover_id][b] + bm->coveramt[bm->SOFTcover_id][b];
				}
			} else if (sp_likeSOFT) { /* Not conincident because fishery not on soft */
				if (check4) { /* Both in canyons */
					p_fish_origi[b][sp][stage][nf] = bm->coveramt[bm->REEFcover_id][b] * (1.0 + bm->coveramt[bm->CANYONcover_id][b]);
				} else if (sp_likeCANYON) { /* Not conincident because fishery not in canyons */
					p_fish_origi[b][sp][stage][nf] = bm->coveramt[bm->REEFcover_id][b] * (1.0 - bm->coveramt[bm->CANYONcover_id][b]);
				} else if (FC_likeCANYON) { /* Not conincident because fish not in canyons */
					p_fish_origi[b][sp][stage][nf] = bm->coveramt[bm->REEFcover_id][b];
				} else { /* Neither in canyons */
					p_fish_origi[b][sp][stage][nf] = bm->coveramt[bm->REEFcover_id][b];
				}
			} else if (FC_likeSOFT) { /* Not conincident because fish not on soft */
				if (check4) { /* Both in canyons */
					p_fish_origi[b][sp][stage][nf] = 1.0;
				} else if (sp_likeCANYON) { /* Not conincident because fishery not in canyons */
					p_fish_origi[b][sp][stage][nf] = 1.0 - bm->coveramt[bm->CANYONcover_id][b];
				} else if (FC_likeCANYON) { /* Not conincident because fish not in canyons */
					p_fish_origi[b][sp][stage][nf] = 1.0;
				} else { /* Neither in canyons */
					p_fish_origi[b][sp][stage][nf] = 1.0;
				}
			} else { /* Neither on soft */
				if (check4) { /* Both in canyons */
					p_fish_origi[b][sp][stage][nf] = 1.0;
				} else if (sp_likeCANYON) { /* Not conincident because fishery not in canyons */
					p_fish_origi[b][sp][stage][nf] = 1.0 - bm->coveramt[bm->CANYONcover_id][b];
				} else if (FC_likeCANYON) { /* Not conincident because fish not in canyons */
					p_fish_origi[b][sp][stage][nf] = 1.0;
				} else { /* Neither in canyons */
					p_fish_origi[b][sp][stage][nf] = 1.0;
				}
			}
		} else { /* Neither on flats */
			if (check3) { /* Both on soft */
				if (check4) { /* Both in canyons */
					p_fish_origi[b][sp][stage][nf] = 1.0;
				} else if (sp_likeCANYON) { /* Not conincident because fishery not in canyons */
					p_fish_origi[b][sp][stage][nf] = 1.0 - bm->coveramt[bm->CANYONcover_id][b];
				} else if (FC_likeCANYON) { /* Not conincident because fish not in canyons */
					p_fish_origi[b][sp][stage][nf] = 1.0;
				} else { /* Neither in canyons */
					p_fish_origi[b][sp][stage][nf] = 1.0;
				}
			} else if (sp_likeSOFT) { /* Not conincident because fishery not on soft */
				if (check4) { /* Both in canyons */
					p_fish_origi[b][sp][stage][nf] = bm->coveramt[bm->REEFcover_id][b] * (1.0 + bm->coveramt[bm->CANYONcover_id][b]);
				} else if (sp_likeCANYON) { /* Not conincident because fishery not in canyons */
					p_fish_origi[b][sp][stage][nf] = bm->coveramt[bm->REEFcover_id][b] * (1.0 - bm->coveramt[bm->CANYONcover_id][b]);
				} else if (FC_likeCANYON) { /* Not conincident because fish not in canyons */
					p_fish_origi[b][sp][stage][nf] = bm->coveramt[bm->REEFcover_id][b];
				} else { /* Neither in canyons */
					p_fish_origi[b][sp][stage][nf] = bm->coveramt[bm->REEFcover_id][b];
				}
			} else if (FC_likeSOFT) { /* Not conincident because fish not on soft */
				if (check4) { /* Both in canyons */
					p_fish_origi[b][sp][stage][nf] = 1.0;
				} else if (sp_likeCANYON) { /* Not conincident because fishery not in canyons */
					p_fish_origi[b][sp][stage][nf] = 1.0 - bm->coveramt[bm->CANYONcover_id][b];
				} else if (FC_likeCANYON) { /* Not conincident because fish not in canyons */
					p_fish_origi[b][sp][stage][nf] = 1.0;
				} else { /* Neither in canyons */
					p_fish_origi[b][sp][stage][nf] = 1.0;
				}
			} else { /* Neither on soft */
				if (check4) { /* Both in canyons */
					p_fish_origi[b][sp][stage][nf] = 1.0;
				} else if (sp_likeCANYON) { /* Not conincident because fishery not in canyons */
					p_fish_origi[b][sp][stage][nf] = 1.0 - bm->coveramt[bm->CANYONcover_id][b];
				} else if (FC_likeCANYON) { /* Not conincident because fish not in canyons */
					p_fish_origi[b][sp][stage][nf] = 1.0;
				} else { /* Neither in canyons */
					p_fish_origi[b][sp][stage][nf] = 1.0;
				}
			}
		}
	} else if (sp_likeREEF) { /* Not conincident because fishery not on reefs */
		if (check2) { /* Both on flats */
			if (check3) { /* Both on soft */
				if (check4) { /* Both in canyons */
					p_fish_origi[b][sp][stage][nf] = (bm->coveramt[bm->SOFTcover_id][b] + bm->coveramt[bm->FLATcover_id][b]) * (1.0
							+ bm->coveramt[bm->CANYONcover_id][b]);
				} else if (sp_likeCANYON) { /* Not conincident because fishery not in canyons */
					p_fish_origi[b][sp][stage][nf] = (bm->coveramt[bm->SOFTcover_id][b] + bm->coveramt[bm->FLATcover_id][b]) * (1.0
							- bm->coveramt[bm->CANYONcover_id][b]);
				} else if (FC_likeCANYON) { /* Not conincident because fish not in canyons */
					p_fish_origi[b][sp][stage][nf] = bm->coveramt[bm->SOFTcover_id][b] + bm->coveramt[bm->FLATcover_id][b];
				} else { /* Neither in canyons */
					p_fish_origi[b][sp][stage][nf] = bm->coveramt[bm->SOFTcover_id][b] + bm->coveramt[bm->FLATcover_id][b];
				}
			} else if (sp_likeSOFT) { /* Not conincident because fishery not on soft */
				if (check4) { /* Both in canyons */
					p_fish_origi[b][sp][stage][nf] = bm->coveramt[bm->FLATcover_id][b] * (1.0 + bm->coveramt[bm->CANYONcover_id][b]);
				} else if (sp_likeCANYON) { /* Not conincident because fishery not in canyons */
					p_fish_origi[b][sp][stage][nf] = bm->coveramt[bm->FLATcover_id][b] * (1.0 - bm->coveramt[bm->CANYONcover_id][b]);
				} else if (FC_likeCANYON) { /* Not conincident because fish not in canyons */
					p_fish_origi[b][sp][stage][nf] = bm->coveramt[bm->FLATcover_id][b];
				} else { /* Neither in canyons */
					p_fish_origi[b][sp][stage][nf] = bm->coveramt[bm->FLATcover_id][b];
				}
			} else if (FC_likeSOFT) { /* Not conincident because fish not on soft */
				if (check4) { /* Both in canyons */
					p_fish_origi[b][sp][stage][nf] = bm->coveramt[bm->FLATcover_id][b] * (1.0 + bm->coveramt[bm->CANYONcover_id][b]);
				} else if (sp_likeCANYON) { /* Not conincident because fishery not in canyons */
					p_fish_origi[b][sp][stage][nf] = bm->coveramt[bm->FLATcover_id][b] * (1.0 - bm->coveramt[bm->CANYONcover_id][b]);
				} else if (FC_likeCANYON) { /* Not conincident because fish not in canyons */
					p_fish_origi[b][sp][stage][nf] = bm->coveramt[bm->FLATcover_id][b];
				} else { /* Neither in canyons */
					p_fish_origi[b][sp][stage][nf] = bm->coveramt[bm->FLATcover_id][b];
				}
			} else { /* Neither on soft */
				if (check4) { /* Both in canyons */
					p_fish_origi[b][sp][stage][nf] = bm->coveramt[bm->FLATcover_id][b] * (1.0 + bm->coveramt[bm->CANYONcover_id][b]);
				} else if (sp_likeCANYON) { /* Not conincident because fishery not in canyons */
					p_fish_origi[b][sp][stage][nf] = bm->coveramt[bm->FLATcover_id][b] * (1.0 - bm->coveramt[bm->CANYONcover_id][b]);
				} else if (FC_likeCANYON) { /* Not conincident because fish not in canyons */
					p_fish_origi[b][sp][stage][nf] = bm->coveramt[bm->FLATcover_id][b];
				} else { /* Neither in canyons */
					p_fish_origi[b][sp][stage][nf] = bm->coveramt[bm->FLATcover_id][b];
				}
			}
		} else if (sp_likeFLAT) { /* Not conincident because fishery not on flats */
			if (check3) { /* Both on soft */
				if (check4) { /* Both in canyons */
					p_fish_origi[b][sp][stage][nf] = bm->coveramt[bm->SOFTcover_id][b] * (1.0 + bm->coveramt[bm->CANYONcover_id][b]);
				} else if (sp_likeCANYON) { /* Not conincident because fishery not in canyons */
					p_fish_origi[b][sp][stage][nf] = bm->coveramt[bm->SOFTcover_id][b] * (1.0 - bm->coveramt[bm->CANYONcover_id][b]);
				} else if (FC_likeCANYON) { /* Not conincident because fish not in canyons */
					p_fish_origi[b][sp][stage][nf] = bm->coveramt[bm->SOFTcover_id][b];
				} else { /* Neither in canyons */
					p_fish_origi[b][sp][stage][nf] = bm->coveramt[bm->SOFTcover_id][b];
				}
			} else if (sp_likeSOFT) { /* Not conincident because fishery not on soft */
				if (check4) { /* Both in canyons */
					p_fish_origi[b][sp][stage][nf] = 0.0;
				} else if (sp_likeCANYON) { /* Not conincident because fishery not in canyons */
					p_fish_origi[b][sp][stage][nf] = 0.0;
				} else if (FC_likeCANYON) { /* Not conincident because fish not in canyons */
					p_fish_origi[b][sp][stage][nf] = 0.0;
				} else { /* Neither in canyons */
					p_fish_origi[b][sp][stage][nf] = 0.0;
				}
			} else if (FC_likeSOFT) { /* Not conincident because fish not on soft */
				if (check4) { /* Both in canyons */
					p_fish_origi[b][sp][stage][nf] = 0.0;
				} else if (sp_likeCANYON) { /* Not conincident because fishery not in canyons */
					p_fish_origi[b][sp][stage][nf] = 0.0;
				} else if (FC_likeCANYON) { /* Not conincident because fish not in canyons */
					p_fish_origi[b][sp][stage][nf] = 0.0;
				} else { /* Neither in canyons */
					p_fish_origi[b][sp][stage][nf] = 0.0;
				}
			} else { /* Neither on soft */
				if (check4) { /* Both in canyons */
					p_fish_origi[b][sp][stage][nf] = 0.0;
				} else if (sp_likeCANYON) { /* Not conincident because fishery not in canyons */
					p_fish_origi[b][sp][stage][nf] = 0.0;
				} else if (FC_likeCANYON) { /* Not conincident because fish not in canyons */
					p_fish_origi[b][sp][stage][nf] = 0.0;
				} else { /* Neither in canyons */
					p_fish_origi[b][sp][stage][nf] = 0.0;
				}
			}
		} else if (FC_likeFLAT) { /* Not conincident because fish not on flats */
			if (check3) { /* Both on soft */
				if (check4) { /* Both in canyons */
					p_fish_origi[b][sp][stage][nf] = bm->coveramt[bm->SOFTcover_id][b] * (1.0 + bm->coveramt[bm->CANYONcover_id][b]);
				} else if (sp_likeCANYON) { /* Not conincident because fishery not in canyons */
					p_fish_origi[b][sp][stage][nf] = bm->coveramt[bm->SOFTcover_id][b] * (1.0 - bm->coveramt[bm->CANYONcover_id][b]);
				} else if (FC_likeCANYON) { /* Not conincident because fish not in canyons */
					p_fish_origi[b][sp][stage][nf] = bm->coveramt[bm->SOFTcover_id][b];
				} else { /* Neither in canyons */
					p_fish_origi[b][sp][stage][nf] = bm->coveramt[bm->SOFTcover_id][b];
				}
			} else if (sp_likeSOFT) { /* Not conincident because fishery not on soft */
				if (check4) { /* Both in canyons */
					p_fish_origi[b][sp][stage][nf] = 0.0;
				} else if (sp_likeCANYON) { /* Not conincident because fishery not in canyons */
					p_fish_origi[b][sp][stage][nf] = 0.0;
				} else if (FC_likeCANYON) { /* Not conincident because fish not in canyons */
					p_fish_origi[b][sp][stage][nf] = 0.0;
				} else { /* Neither in canyons */
					p_fish_origi[b][sp][stage][nf] = 0.0;
				}
			} else if (FC_likeSOFT) { /* Not conincident because fish not on soft */
				if (check4) { /* Both in canyons */
					p_fish_origi[b][sp][stage][nf] = 0.0;
				} else if (sp_likeCANYON) { /* Not conincident because fishery not in canyons */
					p_fish_origi[b][sp][stage][nf] = 0.0;
				} else if (FC_likeCANYON) { /* Not conincident because fish not in canyons */
					p_fish_origi[b][sp][stage][nf] = 0.0;
				} else { /* Neither in canyons */
					p_fish_origi[b][sp][stage][nf] = 0.0;
				}
			} else { /* Neither on soft */
				if (check4) { /* Both in canyons */
					p_fish_origi[b][sp][stage][nf] = 0.0;
				} else if (sp_likeCANYON) { /* Not conincident because fishery not in canyons */
					p_fish_origi[b][sp][stage][nf] = 0.0;
				} else if (FC_likeCANYON) { /* Not conincident because fish not in canyons */
					p_fish_origi[b][sp][stage][nf] = 0.0;
				} else { /* Neither in canyons */
					p_fish_origi[b][sp][stage][nf] = 0.0;
				}
			}
		} else { /* Neither on flats */
			if (check3) { /* Both on soft */
				if (check4) { /* Both in canyons */
					p_fish_origi[b][sp][stage][nf] = bm->coveramt[bm->SOFTcover_id][b] * (1.0 + bm->coveramt[bm->CANYONcover_id][b]);
				} else if (sp_likeCANYON) { /* Not conincident because fishery not in canyons */
					p_fish_origi[b][sp][stage][nf] = bm->coveramt[bm->SOFTcover_id][b] * (1.0 - bm->coveramt[bm->CANYONcover_id][b]);
				} else if (FC_likeCANYON) { /* Not conincident because fish not in canyons */
					p_fish_origi[b][sp][stage][nf] = bm->coveramt[bm->SOFTcover_id][b];
				} else { /* Neither in canyons */
					p_fish_origi[b][sp][stage][nf] = bm->coveramt[bm->SOFTcover_id][b];
				}
			} else if (sp_likeSOFT) { /* Not conincident because fishery not on soft */
				if (check4) { /* Both in canyons */
					p_fish_origi[b][sp][stage][nf] = 0.0;
				} else if (sp_likeCANYON) { /* Not conincident because fishery not in canyons */
					p_fish_origi[b][sp][stage][nf] = 0.0;
				} else if (FC_likeCANYON) { /* Not conincident because fish not in canyons */
					p_fish_origi[b][sp][stage][nf] = 0.0;
				} else { /* Neither in canyons */
					p_fish_origi[b][sp][stage][nf] = 0.0;
				}
			} else if (FC_likeSOFT) { /* Not conincident because fish not on soft */
				if (check4) { /* Both in canyons */
					p_fish_origi[b][sp][stage][nf] = 0.0;
				} else if (sp_likeCANYON) { /* Not conincident because fishery not in canyons */
					p_fish_origi[b][sp][stage][nf] = 0.0;
				} else if (FC_likeCANYON) { /* Not conincident because fish not in canyons */
					p_fish_origi[b][sp][stage][nf] = 0.0;
				} else { /* Neither in canyons */
					p_fish_origi[b][sp][stage][nf] = 0.0;
				}
			} else { /* Neither on soft */
				if (check4) { /* Both in canyons */
					p_fish_origi[b][sp][stage][nf] = 0.0;
				} else if (sp_likeCANYON) { /* Not conincident because fishery not in canyons */
					p_fish_origi[b][sp][stage][nf] = 0.0;
				} else if (FC_likeCANYON) { /* Not conincident because fish not in canyons */
					p_fish_origi[b][sp][stage][nf] = 0.0;
				} else { /* Neither in canyons */
					p_fish_origi[b][sp][stage][nf] = 0.0;
				}
			}
		}
	} else if (FC_likeREEF) { /* Not conincident because fish not on reefs */
		if (check2) { /* Both on flats */
			if (check3) { /* Both on soft */
				if (check4) { /* Both in canyons */
					p_fish_origi[b][sp][stage][nf] = 1.0;
				} else if (sp_likeCANYON) { /* Not conincident because fishery not in canyons */
					p_fish_origi[b][sp][stage][nf] = 1.0 - bm->coveramt[bm->CANYONcover_id][b];
				} else if (FC_likeCANYON) { /* Not conincident because fish not in canyons */
					p_fish_origi[b][sp][stage][nf] = 1.0;
				} else { /* Neither in canyons */
					p_fish_origi[b][sp][stage][nf] = 1.0;
				}
			} else if (sp_likeSOFT) { /* Not conincident because fishery not on soft */
				if (check4) { /* Both in canyons */
					p_fish_origi[b][sp][stage][nf] = bm->coveramt[bm->FLATcover_id][b] * (1.0 + bm->coveramt[bm->CANYONcover_id][b]);
				} else if (sp_likeCANYON) { /* Not conincident because fishery not in canyons */
					p_fish_origi[b][sp][stage][nf] = bm->coveramt[bm->FLATcover_id][b] * (1.0 - bm->coveramt[bm->CANYONcover_id][b]);
				} else if (FC_likeCANYON) { /* Not conincident because fish not in canyons */
					p_fish_origi[b][sp][stage][nf] = bm->coveramt[bm->FLATcover_id][b];
				} else { /* Neither in canyons */
					p_fish_origi[b][sp][stage][nf] = bm->coveramt[bm->FLATcover_id][b];
				}
			} else if (FC_likeSOFT) { /* Not conincident because fish not on soft */
				if (check4) { /* Both in canyons */
					p_fish_origi[b][sp][stage][nf] = bm->coveramt[bm->FLATcover_id][b] * (1.0 + bm->coveramt[bm->CANYONcover_id][b]);
				} else if (sp_likeCANYON) { /* Not conincident because fishery not in canyons */
					p_fish_origi[b][sp][stage][nf] = bm->coveramt[bm->FLATcover_id][b] * (1.0 - bm->coveramt[bm->CANYONcover_id][b]);
				} else if (FC_likeCANYON) { /* Not conincident because fish not in canyons */
					p_fish_origi[b][sp][stage][nf] = bm->coveramt[bm->FLATcover_id][b];
				} else { /* Neither in canyons */
					p_fish_origi[b][sp][stage][nf] = bm->coveramt[bm->FLATcover_id][b];
				}
			} else { /* Neither on soft */
				if (check4) { /* Both in canyons */
					p_fish_origi[b][sp][stage][nf] = 1.0;
				} else if (sp_likeCANYON) { /* Not conincident because fishery not in canyons */
					p_fish_origi[b][sp][stage][nf] = 1.0 - bm->coveramt[bm->CANYONcover_id][b];
				} else if (FC_likeCANYON) { /* Not conincident because fish not in canyons */
					p_fish_origi[b][sp][stage][nf] = 1.0;
				} else { /* Neither in canyons */
					p_fish_origi[b][sp][stage][nf] = 1.0;
				}
			}
		} else if (sp_likeFLAT) { /* Not conincident because fishery not on flats */
			if (check3) { /* Both on soft */
				if (check4) { /* Both in canyons */
					p_fish_origi[b][sp][stage][nf] = bm->coveramt[bm->SOFTcover_id][b] * (1.0 + bm->coveramt[bm->CANYONcover_id][b]);
				} else if (sp_likeCANYON) { /* Not conincident because fishery not in canyons */
					p_fish_origi[b][sp][stage][nf] = bm->coveramt[bm->SOFTcover_id][b] * (1.0 - bm->coveramt[bm->CANYONcover_id][b]);
				} else if (FC_likeCANYON) { /* Not conincident because fish not in canyons */
					p_fish_origi[b][sp][stage][nf] = bm->coveramt[bm->SOFTcover_id][b];
				} else { /* Neither in canyons */
					p_fish_origi[b][sp][stage][nf] = bm->coveramt[bm->SOFTcover_id][b];
				}
			} else if (sp_likeSOFT) { /* Not conincident because fishery not on soft */
				if (check4) { /* Both in canyons */
					p_fish_origi[b][sp][stage][nf] = 0.0;
				} else if (sp_likeCANYON) { /* Not conincident because fishery not in canyons */
					p_fish_origi[b][sp][stage][nf] = 0.0;
				} else if (FC_likeCANYON) { /* Not conincident because fish not in canyons */
					p_fish_origi[b][sp][stage][nf] = 0.0;
				} else { /* Neither in canyons */
					p_fish_origi[b][sp][stage][nf] = 0.0;
				}
			} else if (FC_likeSOFT) { /* Not conincident because fish not on soft */
				if (check4) { /* Both in canyons */
					p_fish_origi[b][sp][stage][nf] = 0.0;
				} else if (sp_likeCANYON) { /* Not conincident because fishery not in canyons */
					p_fish_origi[b][sp][stage][nf] = 0.0;
				} else if (FC_likeCANYON) { /* Not conincident because fish not in canyons */
					p_fish_origi[b][sp][stage][nf] = 0.0;
				} else { /* Neither in canyons */
					p_fish_origi[b][sp][stage][nf] = 0.0;
				}
			} else { /* Neither on soft */
				if (check4) { /* Both in canyons */
					p_fish_origi[b][sp][stage][nf] = 0.0;
				} else if (sp_likeCANYON) { /* Not conincident because fishery not in canyons */
					p_fish_origi[b][sp][stage][nf] = 0.0;
				} else if (FC_likeCANYON) { /* Not conincident because fish not in canyons */
					p_fish_origi[b][sp][stage][nf] = 0.0;
				} else { /* Neither in canyons */
					p_fish_origi[b][sp][stage][nf] = 0.0;
				}
			}
		} else if (FC_likeFLAT) { /* Not conincident because fish not on flats */
			if (check3) { /* Both on soft */
				if (check4) { /* Both in canyons */
					p_fish_origi[b][sp][stage][nf] = 1.0;
				} else if (sp_likeCANYON) { /* Not conincident because fishery not in canyons */
					p_fish_origi[b][sp][stage][nf] = 1.0 - bm->coveramt[bm->CANYONcover_id][b];
				} else if (FC_likeCANYON) { /* Not conincident because fish not in canyons */
					p_fish_origi[b][sp][stage][nf] = 1.0;
				} else { /* Neither in canyons */
					p_fish_origi[b][sp][stage][nf] = 1.0;
				}
			} else if (sp_likeSOFT) { /* Not conincident because fishery not on soft */
				if (check4) { /* Both in canyons */
					p_fish_origi[b][sp][stage][nf] = 0.0;
				} else if (sp_likeCANYON) { /* Not conincident because fishery not in canyons */
					p_fish_origi[b][sp][stage][nf] = 0.0;
				} else if (FC_likeCANYON) { /* Not conincident because fish not in canyons */
					p_fish_origi[b][sp][stage][nf] = 0.0;
				} else { /* Neither in canyons */
					p_fish_origi[b][sp][stage][nf] = 0.0;
				}
			} else if (FC_likeSOFT) { /* Not conincident because fish not on soft */
				if (check4) { /* Both in canyons */
					p_fish_origi[b][sp][stage][nf] = 0.0;
				} else if (sp_likeCANYON) { /* Not conincident because fishery not in canyons */
					p_fish_origi[b][sp][stage][nf] = 0.0;
				} else if (FC_likeCANYON) { /* Not conincident because fish not in canyons */
					p_fish_origi[b][sp][stage][nf] = 0.0;
				} else { /* Neither in canyons */
					p_fish_origi[b][sp][stage][nf] = 0.0;
				}
			} else { /* Neither on soft */
				if (check4) { /* Both in canyons */
					p_fish_origi[b][sp][stage][nf] = 0.0;
				} else if (sp_likeCANYON) { /* Not conincident because fishery not in canyons */
					p_fish_origi[b][sp][stage][nf] = 0.0;
				} else if (FC_likeCANYON) { /* Not conincident because fish not in canyons */
					p_fish_origi[b][sp][stage][nf] = 0.0;
				} else { /* Neither in canyons */
					p_fish_origi[b][sp][stage][nf] = 0.0;
				}
			}
		} else { /* Neither on flats */
			if (check3) { /* Both on soft */
				if (check4) { /* Both in canyons */
					p_fish_origi[b][sp][stage][nf] = 1.0;
				} else if (sp_likeCANYON) { /* Not conincident because fishery not in canyons */
					p_fish_origi[b][sp][stage][nf] = 1.0 - bm->coveramt[bm->CANYONcover_id][b];
				} else if (FC_likeCANYON) { /* Not conincident because fish not in canyons */
					p_fish_origi[b][sp][stage][nf] = 1.0;
				} else { /* Neither in canyons */
					p_fish_origi[b][sp][stage][nf] = 1.0;
				}
			} else if (sp_likeSOFT) { /* Not conincident because fishery not on soft */
				if (check4) { /* Both in canyons */
					p_fish_origi[b][sp][stage][nf] = 0.0;
				} else if (sp_likeCANYON) { /* Not conincident because fishery not in canyons */
					p_fish_origi[b][sp][stage][nf] = 0.0;
				} else if (FC_likeCANYON) { /* Not conincident because fish not in canyons */
					p_fish_origi[b][sp][stage][nf] = 0.0;
				} else { /* Neither in canyons */
					p_fish_origi[b][sp][stage][nf] = 0.0;
				}
			} else if (FC_likeSOFT) { /* Not conincident because fish not on soft */
				if (check4) { /* Both in canyons */
					p_fish_origi[b][sp][stage][nf] = 0.0;
				} else if (sp_likeCANYON) { /* Not conincident because fishery not in canyons */
					p_fish_origi[b][sp][stage][nf] = 0.0;
				} else if (FC_likeCANYON) { /* Not conincident because fish not in canyons */
					p_fish_origi[b][sp][stage][nf] = 0.0;
				} else { /* Neither in canyons */
					p_fish_origi[b][sp][stage][nf] = 0.0;
				}
			} else { /* Neither on soft */
				if (check4) { /* Both in canyons */
					p_fish_origi[b][sp][stage][nf] = 0.0;
				} else if (sp_likeCANYON) { /* Not conincident because fishery not in canyons */
					p_fish_origi[b][sp][stage][nf] = 0.0;
				} else if (FC_likeCANYON) { /* Not conincident because fish not in canyons */
					p_fish_origi[b][sp][stage][nf] = 0.0;
				} else { /* Neither in canyons */
					p_fish_origi[b][sp][stage][nf] = 0.0;
				}
			}
		}
	} else { /* Neither on reefs */
		if (check2) { /* Both on flats */
			if (check3) { /* Both on soft */
				if (check4) { /* Both in canyons */
					p_fish_origi[b][sp][stage][nf] = 1.0;
				} else if (sp_likeCANYON) { /* Not conincident because fishery not in canyons */
					p_fish_origi[b][sp][stage][nf] = 1.0 - bm->coveramt[bm->CANYONcover_id][b];
				} else if (FC_likeCANYON) { /* Not conincident because fish not in canyons */
					p_fish_origi[b][sp][stage][nf] = 1.0;
				} else { /* Neither in canyons */
					p_fish_origi[b][sp][stage][nf] = 1.0;
				}
			} else if (sp_likeSOFT) { /* Not conincident because fishery not on soft */
				if (check4) { /* Both in canyons */
					p_fish_origi[b][sp][stage][nf] = bm->coveramt[bm->FLATcover_id][b] * (1.0 + bm->coveramt[bm->CANYONcover_id][b]);
				} else if (sp_likeCANYON) { /* Not conincident because fishery not in canyons */
					p_fish_origi[b][sp][stage][nf] = bm->coveramt[bm->FLATcover_id][b] * (1.0 - bm->coveramt[bm->CANYONcover_id][b]);
				} else if (FC_likeCANYON) { /* Not conincident because fish not in canyons */
					p_fish_origi[b][sp][stage][nf] = bm->coveramt[bm->FLATcover_id][b];
				} else { /* Neither in canyons */
					p_fish_origi[b][sp][stage][nf] = bm->coveramt[bm->FLATcover_id][b];
				}
			} else if (FC_likeSOFT) { /* Not conincident because fish not on soft */
				if (check4) { /* Both in canyons */
					p_fish_origi[b][sp][stage][nf] = bm->coveramt[bm->FLATcover_id][b] * (1.0 + bm->coveramt[bm->CANYONcover_id][b]);
				} else if (sp_likeCANYON) { /* Not conincident because fishery not in canyons */
					p_fish_origi[b][sp][stage][nf] = bm->coveramt[bm->FLATcover_id][b] * (1.0 - bm->coveramt[bm->CANYONcover_id][b]);
				} else if (FC_likeCANYON) { /* Not conincident because fish not in canyons */
					p_fish_origi[b][sp][stage][nf] = bm->coveramt[bm->FLATcover_id][b];
				} else { /* Neither in canyons */
					p_fish_origi[b][sp][stage][nf] = bm->coveramt[bm->FLATcover_id][b];
				}
			} else { /* Neither on soft */
				if (check4) { /* Both in canyons */
					p_fish_origi[b][sp][stage][nf] = 1.0;
				} else if (sp_likeCANYON) { /* Not conincident because fishery not in canyons */
					p_fish_origi[b][sp][stage][nf] = 1.0 - bm->coveramt[bm->CANYONcover_id][b];
				} else if (FC_likeCANYON) { /* Not conincident because fish not in canyons */
					p_fish_origi[b][sp][stage][nf] = 1.0;
				} else { /* Neither in canyons */
					p_fish_origi[b][sp][stage][nf] = 1.0;
				}
			}
		} else if (sp_likeFLAT) { /* Not conincident because fishery not on flats */
			if (check3) { /* Both on soft */
				if (check4) { /* Both in canyons */
					p_fish_origi[b][sp][stage][nf] = bm->coveramt[bm->SOFTcover_id][b] * (1.0 + bm->coveramt[bm->CANYONcover_id][b]);
				} else if (sp_likeCANYON) { /* Not conincident because fishery not in canyons */
					p_fish_origi[b][sp][stage][nf] = bm->coveramt[bm->SOFTcover_id][b] * (1.0 - bm->coveramt[bm->CANYONcover_id][b]);
				} else if (FC_likeCANYON) { /* Not conincident because fish not in canyons */
					p_fish_origi[b][sp][stage][nf] = bm->coveramt[bm->SOFTcover_id][b];
				} else { /* Neither in canyons */
					p_fish_origi[b][sp][stage][nf] = bm->coveramt[bm->SOFTcover_id][b];
				}
			} else if (sp_likeSOFT) { /* Not conincident because fishery not on soft */
				if (check4) { /* Both in canyons */
					p_fish_origi[b][sp][stage][nf] = 0.0;
				} else if (sp_likeCANYON) { /* Not conincident because fishery not in canyons */
					p_fish_origi[b][sp][stage][nf] = 0.0;
				} else if (FC_likeCANYON) { /* Not conincident because fish not in canyons */
					p_fish_origi[b][sp][stage][nf] = 0.0;
				} else { /* Neither in canyons */
					p_fish_origi[b][sp][stage][nf] = 0.0;
				}
			} else if (FC_likeSOFT) { /* Not conincident because fish not on soft */
				if (check4) { /* Both in canyons */
					p_fish_origi[b][sp][stage][nf] = 0.0;
				} else if (sp_likeCANYON) { /* Not conincident because fishery not in canyons */
					p_fish_origi[b][sp][stage][nf] = 0.0;
				} else if (FC_likeCANYON) { /* Not conincident because fish not in canyons */
					p_fish_origi[b][sp][stage][nf] = 0.0;
				} else { /* Neither in canyons */
					p_fish_origi[b][sp][stage][nf] = 0.0;
				}
			} else { /* Neither on soft */
				if (check4) { /* Both in canyons */
					p_fish_origi[b][sp][stage][nf] = 0.0;
				} else if (sp_likeCANYON) { /* Not conincident because fishery not in canyons */
					p_fish_origi[b][sp][stage][nf] = 0.0;
				} else if (FC_likeCANYON) { /* Not conincident because fish not in canyons */
					p_fish_origi[b][sp][stage][nf] = 0.0;
				} else { /* Neither in canyons */
					p_fish_origi[b][sp][stage][nf] = 0.0;
				}
			}
		} else if (FC_likeFLAT) { /* Not conincident because fish not on flats */
			if (check3) { /* Both on soft */
				if (check4) { /* Both in canyons */
					p_fish_origi[b][sp][stage][nf] = 1.0;
				} else if (sp_likeCANYON) { /* Not conincident because fishery not in canyons */
					p_fish_origi[b][sp][stage][nf] = 1.0 - bm->coveramt[bm->CANYONcover_id][b];
				} else if (FC_likeCANYON) { /* Not conincident because fish not in canyons */
					p_fish_origi[b][sp][stage][nf] = 1.0;
				} else { /* Neither in canyons */
					p_fish_origi[b][sp][stage][nf] = 1.0;
				}
			} else if (sp_likeSOFT) { /* Not conincident because fishery not on soft */
				if (check4) { /* Both in canyons */
					p_fish_origi[b][sp][stage][nf] = 0.0;
				} else if (sp_likeCANYON) { /* Not conincident because fishery not in canyons */
					p_fish_origi[b][sp][stage][nf] = 0.0;
				} else if (FC_likeCANYON) { /* Not conincident because fish not in canyons */
					p_fish_origi[b][sp][stage][nf] = 0.0;
				} else { /* Neither in canyons */
					p_fish_origi[b][sp][stage][nf] = 0.0;
				}
			} else if (FC_likeSOFT) { /* Not conincident because fish not on soft */
				if (check4) { /* Both in canyons */
					p_fish_origi[b][sp][stage][nf] = 0.0;
				} else if (sp_likeCANYON) { /* Not conincident because fishery not in canyons */
					p_fish_origi[b][sp][stage][nf] = 0.0;
				} else if (FC_likeCANYON) { /* Not conincident because fish not in canyons */
					p_fish_origi[b][sp][stage][nf] = 0.0;
				} else { /* Neither in canyons */
					p_fish_origi[b][sp][stage][nf] = 0.0;
				}
			} else { /* Neither on soft */
				if (check4) { /* Both in canyons */
					p_fish_origi[b][sp][stage][nf] = 0.0;
				} else if (sp_likeCANYON) { /* Not conincident because fishery not in canyons */
					p_fish_origi[b][sp][stage][nf] = 0.0;
				} else if (FC_likeCANYON) { /* Not conincident because fish not in canyons */
					p_fish_origi[b][sp][stage][nf] = 0.0;
				} else { /* Neither in canyons */
					p_fish_origi[b][sp][stage][nf] = 0.0;
				}
			}
		} else { /* Neither on flats */
			if (check3) { /* Both on soft */
				if (check4) { /* Both in canyons */
					p_fish_origi[b][sp][stage][nf] = 1.0;
				} else if (sp_likeCANYON) { /* Not conincident because fishery not in canyons */
					p_fish_origi[b][sp][stage][nf] = 1.0 - bm->coveramt[bm->CANYONcover_id][b];
				} else if (FC_likeCANYON) { /* Not conincident because fish not in canyons */
					p_fish_origi[b][sp][stage][nf] = 1.0;
				} else { /* Neither in canyons */
					p_fish_origi[b][sp][stage][nf] = 1.0;
				}
			} else if (sp_likeSOFT) { /* Not conincident because fishery not on soft */
				if (check4) { /* Both in canyons */
					p_fish_origi[b][sp][stage][nf] = 0.0;
				} else if (sp_likeCANYON) { /* Not conincident because fishery not in canyons */
					p_fish_origi[b][sp][stage][nf] = 0.0;
				} else if (FC_likeCANYON) { /* Not conincident because fish not in canyons */
					p_fish_origi[b][sp][stage][nf] = 0.0;
				} else { /* Neither in canyons */
					p_fish_origi[b][sp][stage][nf] = 0.0;
				}
			} else if (FC_likeSOFT) { /* Not conincident because fish not on soft */
				if (check4) { /* Both in canyons */
					p_fish_origi[b][sp][stage][nf] = 0.0;
				} else if (sp_likeCANYON) { /* Not conincident because fishery not in canyons */
					p_fish_origi[b][sp][stage][nf] = 0.0;
				} else if (FC_likeCANYON) { /* Not conincident because fish not in canyons */
					p_fish_origi[b][sp][stage][nf] = 0.0;
				} else { /* Neither in canyons */
					p_fish_origi[b][sp][stage][nf] = 0.0;
				}
			} else { /* Neither on soft */
				if (check4) { /* Both in canyons */
					p_fish_origi[b][sp][stage][nf] = 0.0;
				} else if (sp_likeCANYON) { /* Not conincident because fishery not in canyons */
					p_fish_origi[b][sp][stage][nf] = 0.0;
				} else if (FC_likeCANYON) { /* Not conincident because fish not in canyons */
					p_fish_origi[b][sp][stage][nf] = 0.0;
				} else { /* Neither in canyons */
					p_fish_origi[b][sp][stage][nf] = 0.0;
				}
			}
		}
	}
	//	fprintf(bm->logFile, "p_fish_origi[%d][%d][%d][%d] = %e\n", b, sp, stage, nf, p_fish_origi[b][sp][stage][nf]);
	//	fprintf(bm->logFile, "newvalue = %e\n", newvalue);
	//
	//	if(newvalue !=  p_fish_origi[b][sp][stage][nf])
	//		quit("Different value\n");


}

/**
 *	Potential total area effected by fishing.
 */
void Basic_Habitat_Overlap(MSEBoxModel *bm, int b, FILE *llogfp) {
	int stage, nf, sp, check1, check2, check3, check4;
	double prop_reef, prop_flat, prop_soft, prop_canyon, prop_access, sp_likeREEF, sp_likeFLAT, sp_likeCANYON, FC_likeREEF, FC_likeFLAT, FC_likeCANYON, FC_likeSOFT, sp_likeSOFT;

	if (verbose > 1)
		printf("Determine basic habitat overlap\n");

	/* Determine area of each box available to each fishery as trawlable ground */
	for (nf = 0; nf < bm->K_num_fisheries; nf++) {
		/* Fisheries' habitat preferences */
		FC_likeREEF = bm->FC_HABITATlike[nf][bm->REEFcover_id];
		FC_likeFLAT = bm->FC_HABITATlike[nf][bm->FLATcover_id];
		FC_likeSOFT = bm->FC_HABITATlike[nf][bm->SOFTcover_id];
		FC_likeCANYON = bm->FC_HABITATlike[nf][bm->CANYONcover_id];

		/* Determine area of cell covered by the habitat types */
		prop_reef = bm->coveramt[bm->REEFcover_id][b];
		prop_flat = bm->coveramt[bm->FLATcover_id][b];
		prop_soft = bm->coveramt[bm->SOFTcover_id][b];
		prop_canyon = bm->coveramt[bm->CANYONcover_id][b];

		prop_access = 0.0;
		if (FC_likeREEF > 0.0)
			prop_access += prop_reef;
		if (FC_likeFLAT > 0.0)
			prop_access += prop_flat;
		if (FC_likeSOFT > 0.0)
			prop_access += prop_soft;
		if (FC_likeCANYON > 0.0)
			prop_access += prop_canyon;

		if (prop_access > 1.0)
			prop_access = 1.0;

		FC_hdistrib[b][nf] = prop_access;
	}

	/* Determine relative area in common with each species */
	for (sp = 0; sp < bm->K_num_tot_sp; sp++)
		if (FunctGroupArray[sp].isImpacted == TRUE)
			for (nf = 0; nf < bm->K_num_fisheries; nf++)
				for (stage = 0; stage < FunctGroupArray[sp].numStages; stage++)
					p_fish_origi[b][sp][stage][nf] = 0.0;

	for (sp = 0; sp < bm->K_num_tot_sp; sp++) {
		if (FunctGroupArray[sp].isImpacted == TRUE) {
			for (nf = 0; nf < bm->K_num_fisheries; nf++) {
				for (stage = 0; stage < FunctGroupArray[sp].numStages; stage++) {
					/* Group's habitat preferences */
					sp_likeREEF = bm->HABITATlike[sp][stage][bm->REEFcover_id];
					sp_likeFLAT = bm->HABITATlike[sp][stage][bm->FLATcover_id];
					sp_likeSOFT = bm->HABITATlike[sp][stage][bm->SOFTcover_id];
					sp_likeCANYON = bm->HABITATlike[sp][stage][bm->CANYONcover_id];

					/* Fisheries' habitat preferences */
					FC_likeREEF = bm->FC_HABITATlike[nf][bm->REEFcover_id];
					FC_likeFLAT = bm->FC_HABITATlike[nf][bm->FLATcover_id];
					FC_likeSOFT = bm->FC_HABITATlike[nf][bm->SOFTcover_id];
					FC_likeCANYON = bm->FC_HABITATlike[nf][bm->CANYONcover_id];

					/* Compare preferences */
					/* Both the group and the fishery operate over reefs */
					if (sp_likeREEF > 0.0 && FC_likeREEF > 0.0)
						check1 = 1;
					else
						check1 = 0;

					/* Both the group and the fishery operate over flat ground */
					if (sp_likeFLAT > 0.0 && FC_likeFLAT > 0.0)
						check2 = 1;
					else
						check2 = 0;

					if (sp_likeSOFT > 0.0 && FC_likeSOFT > 0.0)
						check3 = 1;
					else
						check3 = 0;

					/* Both the group and the fishery aggregate on canyons */
					if (sp_likeCANYON > 0.0 && FC_likeCANYON > 0.0)
						check4 = 1;
					else
						check4 = 0;

					Habitat_Overlap(bm, b, sp, stage, nf, check1, check2, check3, check4);
				}
			}
		}
	}

	return;
}

/**
 *	Adjust total area effected by fishing to reflect changes in actual area fished.
 */
void Harvest_Update_Habitat_Overlap(MSEBoxModel *bm, int b) {
	double P_scale, P_orig, newP;
	int stage, nf, sp;

	if (verbose > 1)
		printf("Updating habitat overlap information\n");

	for (nf = 0; nf < bm->K_num_fisheries; nf++) {
		//flagchangeP = (int) (bm->FISHERYprms[nf][flagchangeP_id]);
		//P_num_changes = (int) (bm->FISHERYprms[nf][P_num_changes_id]);

		/* If changing fishing spatial extent through time (gradual change).
			Note don't need to correct end_date to seconds here as bm->dayt
			 in days too */

		P_scale = Get_Fishery_Change_Scale(bm, nf, P_num_changes_id, P_num_changes_id, Pchange);

		for (sp = 0; sp < bm->K_num_tot_sp; sp++) {
			if (FunctGroupArray[sp].isImpacted == TRUE) {
				for (stage = 0; stage < FunctGroupArray[sp].numStages; stage++) {
					//fprintf(bm->logFile, "P_scale = %e\n", P_scale);
					P_orig = p_fish_origi[b][sp][stage][nf];
					newP = P_scale * P_orig;

					if (newP < 0.0)
						newP = 0.0;

					/* Is allowed to be > 1.0 as can represent concentration of fish and
					 boats at aggregation sites (e.g. canyons) */
					p_fishi[sp][stage][nf] = newP;
				}
			}
		}
	}

	return;
}

/**
 *	Use this to fake catch data while skipping biology - so runs faster.
 *
 */
void Harvest_Skip_biology(MSEBoxModel *bm, FILE *llogfp) {
	int ij, sp, pid, n, k, nf, sn, rn, den, stage;
	double spbiom, q, spfishing, swept_area, vertdistrib, FCpressure, sel, loadDetFC;

	/* Initialise values */
	for (ij = 0; ij < bm->nbox; ij++) {
		for (sp = 0; sp < bm->K_num_tot_sp; sp++) {
			if (FunctGroupArray[sp].isImpacted == TRUE) {
				for (nf = 0; nf < bm->K_num_fisheries; nf++) {
					bm->Discards[ij][sp][nf] = 0;
					for (k = 0; k < bm->boxes[ij].nz; k++) {
						bm->Catch[ij][sp][nf][k] = 0.0;
					}
				}
			}
		}
	}

	/* Assign fake catches */
	for (ij = 0; ij < bm->nbox; ij++) {
		if (bm->boxes[ij].type != BOUNDARY) {
			for (k = 0; k < bm->boxes[ij].nz; k++) {
				bm->cell_vol = bm->boxes[ij].area * bm->boxes[ij].dz[k];

				/* Get total biomass - summing over each cohort */
				for (nf = 0; nf < bm->K_num_fisheries; nf++) {
					swept_area = bm->FISHERYprms[nf][swept_area_id] / (bm->cell_vol * FC_hdistrib[ij][nf] + small_num);
					vertdistrib = Effort_vdistrib[ij][k][nf];
					FCpressure = bm->Effort[ij][nf] * vertdistrib / 86400.0;

					/** Do vertebrate catches first **/
					for (sp = 0; sp < bm->K_num_tot_sp; sp++) {
						switch (FunctGroupArray[sp].groupAgeType) {
						case AGE_STRUCTURED:
							for (n = 0; n < FunctGroupArray[sp].numCohortsXnumGenes; n++) {
								sn = FunctGroupArray[sp].structNTracers[n];
								rn = FunctGroupArray[sp].resNTracers[n];
								den = FunctGroupArray[sp].NumsTracers[n];
								stage = FunctGroupArray[sp].cohort_stage[n];

								spbiom = (bm->boxes[ij].tr[k][sn] + bm->boxes[ij].tr[k][rn]) * bm->boxes[ij].tr[k][den];

								/* Get catch per fishery */
								sel = bm->selectivity[sp][nf][stage];
								q = bm->SP_FISHERYprms[sp][nf][q_id];
								loadDetFC = bm->SP_FISHERYprms[sp][nf][FFCDR_id];
								spfishing = FCpressure * sel * q * spbiom * (1.0 - loadDetFC) * swept_area;

								bm->Catch[ij][sp][nf][k] += spfishing * bm->cell_vol * bm->dt;
								CatchSum[sp][tscocatch_id] += spfishing * bm->cell_vol * bm->dt;
							}
							break;
						case AGE_STRUCTURED_BIOMASS:
							for (n = 0; n < FunctGroupArray[sp].numCohortsXnumGenes; n++) {
								pid = FunctGroupArray[sp].totNTracers[n];
								spbiom = bm->boxes[ij].tr[k][pid];
                                stage = FunctGroupArray[sp].cohort_stage[n];

								/* Get catch per fishery */
								sel = bm->selectivity[sp][nf][stage];
								q = bm->SP_FISHERYprms[sp][nf][q_id];
								loadDetFC = bm->SP_FISHERYprms[sp][nf][FFCDR_id];
								spfishing = FCpressure * sel * q * spbiom * (1.0 - loadDetFC) * swept_area;

								bm->Catch[ij][sp][nf][k] += spfishing * bm->cell_vol * bm->dt;
								CatchSum[sp][tscocatch_id] += spfishing * bm->cell_vol * bm->dt;

							}
							break;
						case BIOMASS:
							/* Do nothing */
							break;
						}
					}
				}
			}
			/* Epibenthos - Get catch per fishery */
			for (nf = 0; nf < bm->K_num_fisheries; nf++) {
				bm->cell_vol = bm->boxes[ij].area * bm->boxes[ij].dz[0];
				swept_area = bm->FISHERYprms[nf][swept_area_id] / (bm->cell_vol * FC_hdistrib[ij][nf] + small_num);
				vertdistrib = Effort_vdistrib[ij][0][nf];
				FCpressure = bm->Effort[ij][nf] * vertdistrib / 86400.0;

				for (sp = 0; sp < bm->K_num_tot_sp; sp++) {
					if (FunctGroupArray[sp].habitatType == EPIFAUNA) {
						for (n = 0; n < FunctGroupArray[sp].numCohortsXnumGenes; n++) {
							pid = FunctGroupArray[sp].totNTracers[n];
							spbiom = bm->boxes[ij].epi[pid];
                            stage = FunctGroupArray[sp].cohort_stage[n];

							/* Get catch per fishery */
							sel = bm->selectivity[sp][nf][stage];
							q = bm->SP_FISHERYprms[sp][nf][q_id];
							loadDetFC = bm->SP_FISHERYprms[sp][nf][FFCDR_id];
							spfishing = FCpressure * sel * q * spbiom * (1.0 - loadDetFC) * swept_area;

							bm->Catch[ij][sp][nf][0] += spfishing * bm->cell_vol * bm->dt;
							CatchSum[sp][tscocatch_id] += spfishing * bm->cell_vol * bm->dt;

						}
					}
				}
			}

			/* Update catch per fishery records */
			for (k = 0; k < bm->boxes[ij].nz; k++) {
				for (sp = 0; sp < bm->K_num_tot_sp; sp++) {
					if (FunctGroupArray[sp].isImpacted == TRUE) {
						for (nf = 0; nf < bm->K_num_fisheries; nf++) {
							TotCumCatch[sp][nf][bm->thisyear] += bm->Catch[ij][sp][nf][k];
						}
					}
				}
			}

			/* Save effort */
			/* Total Effort tracers */
			for (nf = 0; nf < bm->K_num_fisheries; nf++) {
				bm->boxes[ij].fishstat[FisheryArray[nf].totalEffortTracer] += bm->Effort[ij][nf];
			}
		}
	}
	return;
}

double Get_Selectivity(MSEBoxModel *bm, int sp, int stage, int nf, double li, int sel_curve, double addlsm, double addsigma) {

	double sel = 0.0, sel_b, sel_lsm, step1, step2, sel_ampli;
	double sel_sigma, sel1, sel2;

	switch (sel_curve) {
	case q_const_id: /* Group specific constant */
		sel = bm->SP_FISHERYprms[sp][nf][sel_id] + addlsm;
		break;
	case q_ageconst_id: /* Constant proportion of maturity stage - also used for stages in cephalopods */
		sel = bm->selectivity[sp][nf][stage] + addlsm;
		break;
	case q_logistic_id: /* Dynamic so selectivity based on size, logistic */
		sel_b = bm->FISHERYprms[nf][sel_b_id] + addsigma;
		sel_lsm = bm->FISHERYprms[nf][sel_lsm_id] + addlsm;
		sel = 1.0 / (1.0 + exp(-sel_b * (li - sel_lsm)));
		break;
	case q_norm_id: /* Dynamic so selectivity based on size, normal */
		sel_lsm = bm->FISHERYprms[nf][sel_normlsm_id] + addlsm;
		sel_sigma = bm->FISHERYprms[nf][sel_normsigma_id] + addsigma;
		step1 = li - sel_lsm;
		step2 = -(step1 * step1) / (2.0 * sel_sigma * sel_sigma + small_num);
		sel = exp(step2);
		break;
	case q_lognorm_id: /* Dynamic so selectivity based on size, lognormal */
		sel_lsm = bm->FISHERYprms[nf][sel_lognormlsm_id] + addlsm;
		sel_sigma = bm->FISHERYprms[nf][sel_lognormsigma_id] + addsigma;
		sel = Util_Lognorm_Distrib(sel_lsm, sel_sigma, li);
		break;
	case q_gamma_id: /* Dynamic so selectivity based on size, gamma */
		sel_lsm = bm->FISHERYprms[nf][sel_gammalsm_id] + addlsm + small_num;
		sel_sigma = bm->FISHERYprms[nf][sel_gammasigma_id] + addsigma;
		step1 = sel_lsm * sel_lsm + 4.0 * sel_sigma * sel_sigma;
		step2 = (sqrt(step1) - sel_lsm) / 2.0 + small_num;
		sel = pow((li / sel_lsm), (sel_lsm / step2)) * exp((sel_lsm - li) / step2);
		break;
	case q_knife_id: /* Knife edged */
		if (FunctGroupArray[sp].isVertebrate == TRUE) {
			/* For vertebrates check to see if over minimum length */
			sel_lsm = bm->selectivity[sp][nf][juv_id];
			
			if (li < sel_lsm)
				sel = 0.0;
			else
				sel = bm->selectivity[sp][nf][adult_id] + addlsm;
		} else {
			/* For invertebrates assume all equally available as no size check possible */
			sel = bm->SP_FISHERYprms[sp][nf][sel_id] + addlsm;
		}
		break;
	case q_bimodal_id: /* Bimodal "normal" selectivity curve based on size */
		sel_lsm = bm->FISHERYprms[nf][sel_bilsm1_id] + addlsm;
		sel_sigma = bm->FISHERYprms[nf][sel_bisigma_id] + addsigma;
		step1 = li - sel_lsm;
		step2 = -(step1 * step1) / (2.0 * sel_sigma * sel_sigma + small_num);
		sel1 = exp(step2);
		sel_lsm = bm->FISHERYprms[nf][sel_bilsm2_id] + addlsm;
		step1 = li - sel_lsm;
		step2 = -(step1 * step1) / (2.0 * sel_sigma * sel_sigma + small_num);
		sel2 = exp(step2);
		sel = max(sel1, sel2);
		break;
    case q_binormal_id: /* Bimodal "normal" selectivity curve based on size */
        sel_lsm = bm->FISHERYprms[nf][sel_bilsm1_id] + addlsm;
        sel_sigma = bm->FISHERYprms[nf][sel_bisigma_id] + addsigma;
        step1 = li - sel_lsm;
        step2 = -(step1 * step1) / (2.0 * sel_sigma * sel_sigma + small_num);
        sel1 = exp(step2);
        sel_lsm = bm->FISHERYprms[nf][sel_bilsm2_id] + addlsm;
        sel_sigma = bm->FISHERYprms[nf][sel_bisigma2_id] + addsigma;
        sel_ampli = bm->FISHERYprms[nf][sel_ampli_id];
        step1 = li - sel_lsm;
        step2 = -(step1 * step1) / (2.0 * sel_sigma * sel_sigma + small_num);
        sel2 = sel_ampli * exp(step2);
        sel = max(sel1, sel2);
        break;
	default:
		quit("No such selectivity curve defined (%d) - value must be between 0 and 6 currently\n", sel_curve);
		break;
	}

	/* Ensure selecitivity is bounded between zero and one */
	if (sel > 1.0)
		sel = 1.0;
	if (sel < 0.0)
		sel = 0.0;

	return sel;
}

/**
 * \brief Get the maximum number of gear changes that are possible for the given fishery
 *
 */
int Harvest_Get_Num_Gear_Changes(MSEBoxModel *bm, int nf, FILE *llogfp) {

	int i, sp;
	int gear_change = 0;
	int Num_changes;

	/* Number of gear changes (including selectivity, q, swept area, discarding, access and escapement).
	 Count only those in the last year.
	 */
	Num_changes = (int) (bm->FISHERYprms[nf][SEL_num_changes_id]);
	//if ((int) (bm->FISHERYprms[nf][flagchangeSEL_id])) {
		for (i = 0; i < Num_changes; i++) {
			if ((SELchange[nf][i][start_id] > (bm->dayt - 365)) && (SELchange[nf][i][start_id] < bm->dayt + 1)) {
				gear_change++;
			}
		}
	//}

	//if ((int) (bm->FISHERYprms[nf][flagQchange_id])) {
		for (sp = 0; sp < bm->K_num_tot_sp; sp++) {
			if (FunctGroupArray[sp].isImpacted == TRUE) {
				Num_changes = (int) (bm->SP_FISHERYprms[sp][nf][Q_num_changes_id]);
				for (i = 0; i < Num_changes; i++) {
					if ((Qchange[sp][nf][i][start_id] > (bm->dayt - 365)) && (Qchange[sp][nf][i][start_id] < bm->dayt + 1)) {
						gear_change++;
					}
				}
			}
		}
	//}

	Num_changes = (int) (bm->FISHERYprms[nf][SWEPT_num_changes_id]);
	//if ( (int) (bm->FISHERYprms[nf][flagchangeSWEPT_id])) {
		for (i = 0; i < Num_changes; i++) {
			if ((SWEPTchange[nf][i][start_id] > (bm->dayt - 365)) && (SWEPTchange[nf][i][start_id] < bm->dayt + 1)) {
				gear_change++;
			}
		}
	//}

	//if ((int) (bm->FISHERYprms[nf][flagchangeDISCRD_id])) {
		for (sp = 0; sp < bm->K_num_tot_sp; sp++) {
			if (FunctGroupArray[sp].isImpacted == TRUE) {
				Num_changes = (int) (bm->SP_FISHERYprms[sp][nf][DISCRD_num_changes_id]);
				for (i = 0; i < Num_changes; i++) {
					if ((DISCRDchange[sp][nf][i][start_id] > (bm->dayt - 365)) && (DISCRDchange[sp][nf][i][start_id] < bm->dayt + 1)) {
						gear_change++;
					}
				}
			}
		}
	//}

	Num_changes = (int) (bm->FISHERYprms[nf][P_num_changes_id]);
	//if ((int) (bm->FISHERYprms[nf][flagchangeP_id])) {
		for (i = 0; i < Num_changes; i++) {
			if ((Pchange[nf][i][start_id] > (bm->dayt - 365)) && (Pchange[nf][i][start_id] < bm->dayt + 1)) {
				gear_change++;
			}
		}
	//}

	return gear_change;
}

/**
 * \brief Update the totCatch, totdiscards values and then update the related fishery tracers.
 *
 * 	This is called from within ecology - Water_Column_Box and EpibethicBox to update the tracers based on calculations in Harvest_Do_Fishing_And_ByCatch
 *
 *
 *
 */
void Harvest_Update_Total_Catch(MSEBoxModel *bm, double *FishTracers, int habitat, FILE *llogfp) {
	int guild, nf, cohort;

	/*** Update total catch and discards ***/
	for (guild = 0; guild < bm->K_num_tot_sp; guild++) {

		if (habitat == WC || (habitat == EPIFAUNA && (FunctGroupArray[guild].isVertebrate == FALSE && FunctGroupArray[guild].habitatType != WC))) {
			if (FunctGroupArray[guild].isImpacted == TRUE) {
				bm->totdiscards[guild] = 0;
				for (nf = 0; nf < bm->K_num_fisheries; nf++) {
					bm->totdiscards[guild] += bm->Discards[bm->current_box][guild][nf];
				}
			}
			if (FunctGroupArray[guild].isFished == TRUE) {
				bm->totcatch[guild] = 0;
				for (nf = 0; nf < bm->K_num_fisheries; nf++) {
					bm->totcatch[guild] += bm->Catch[bm->current_box][guild][nf][bm->current_layer];
				}
			}
		}
	}
    
	/**
	 Fishery landings - record catch, discards and bycatch and age profiles here for CEP, PWN, ZG, ZL and vertebrates.
	 All the rest are done in epibenthic layer (as sediment associated).

	 Accumulate total catch and effort for the record period.

	 Some of these use to be stored in the sediments or overloaded, but now promoted many to individual
	 entries so get output data of correct resolution

	 All weights converted from mg N to t.
	 */

	for (guild = 0; guild < bm->K_num_tot_sp; guild++) {
		if (habitat == WC || (habitat == EPIFAUNA && (FunctGroupArray[guild].isVertebrate == FALSE && FunctGroupArray[guild].habitatType != WC))) {
			if (FunctGroupArray[guild].isFished == TRUE) {
				for (nf = 0; nf < bm->K_num_fisheries; nf++) {
					/* Catch by fishery tracers */
					FishTracers[FunctGroupArray[guild].CaughtByFisheryTracers[nf]] += bm->Catch[bm->current_box][guild][nf][bm->current_layer] * bm->X_CN * mg_2_tonne;
                    
					/* Total Rec Catch tracers */
					//TODO: Sum over all rec fisheries.
					FishTracers[FunctGroupArray[guild].totRecCatchTracer] += bm->RecCatch[bm->current_box][guild][nf] * bm->X_CN * mg_2_tonne;
				}
				/* Total Catch tracers */
				FishTracers[FunctGroupArray[guild].totCatchTracer] += bm->totcatch[guild] * bm->X_CN * mg_2_tonne;
			}
			if (FunctGroupArray[guild].isImpacted == TRUE) {
				for (nf = 0; nf < bm->K_num_fisheries; nf++) {
					/* Discards by fishery tracers */
					FishTracers[FunctGroupArray[guild].DiscardedByFisheryTracers[nf]] += bm->Discards[bm->current_box][guild][nf] * bm->X_CN * mg_2_tonne;
				}

				/* Total Discards */
				FishTracers[FunctGroupArray[guild].totDiscardsTracer] += bm->totdiscards[guild] * bm->X_CN * mg_2_tonne;

			}
		}
	}
	if (!bm->current_layer && (habitat == WC)) {  // So read in at the end of every box as FCcaught reinitialised before new box calculations done
		/* Total Effort tracers */
		for (nf = 0; nf < bm->K_num_fisheries; nf++) {
			FishTracers[FisheryArray[nf].totalEffortTracer] += bm->Effort[bm->current_box][nf];
		}
		/**
		 Size/age distributions for catch, discards (and bycatch)
		 **/
		for (guild = 0; guild < bm->K_num_tot_sp; guild++) {
			if (FunctGroupArray[guild].isVertebrate) {
				for (cohort = 0; cohort < FunctGroupArray[guild].numCohorts * FunctGroupArray[guild].numGeneTypes; cohort++) {
					FishTracers[FunctGroupArray[guild].CatchTracers[cohort]] += bm->FCcaught[guild][cohort]; // Already * dt in Harvest_Do_Fishing_And_ByCatch when first calculated as FishingRes, which becomes FCcaughttemp, which becomes FCcaught
					FishTracers[FunctGroupArray[guild].DiscardTracers[cohort]] += bm->FCdiscard[guild][cohort]; // As for FCcaught
                    
                    /*
                    if (guild == 2) {
                        fprintf(llogfp,"Time: %e box%d-%d %s-%d has CatchTracers: %e FCcaught: %e\n", bm->dayt, bm->current_box, bm->current_layer, FunctGroupArray[guild].groupCode, cohort, FishTracers[FunctGroupArray[guild].CatchTracers[cohort]], bm->FCcaught[guild][cohort]);
                    }
                    */
                }
            }
        }
    }
}

/**
 *
 * Change the Catchability of a group for a fishery.
 *
 * At present the way things work is the same changes in catchability are applied to all fisheries for each groups.
 * This is a single set of changes in catchability are read in for each group and these changes are applied across all fisheries
 * provide the num changes is correct.
 *
 *
 */
void Harvest_Scale_Q(MSEBoxModel *bm, int species, int fishery, double start, double period, double mult, FILE *llogfp) {
	double ****tempArray;
	double ****tempArrayPointer;

	/* Make sure this species and fishery combination is scaled */
	bm->SP_FISHERYprms[species][fishery][flagQchange_id] = TRUE;

	/* Increase the number of scaling events */
	bm->SP_FISHERYprms[species][fishery][Q_num_changes_id]++;

	if (((int) bm->SP_FISHERYprms[species][fishery][Q_num_changes_id]) > Q_max_num_changes) {
		Q_max_num_changes = (int)bm->SP_FISHERYprms[species][fishery][Q_num_changes_id];
	}

	/* Check to see if there is just a single dummy value in the array - if so just overwrite it */
	if (Q_max_num_changes == 1 && Qchange[species][fishery] == 0) {
		Qchange[species][fishery][0][start_id] = start + bm->dayt;
		Qchange[species][fishery][0][period_id] = period;
		Qchange[species][fishery][0][mult_id] = mult;
		return;
	}

	/* Allocate a new array */
	tempArray = Util_Alloc_Init_4D_Double(3, Q_max_num_changes, bm->K_num_fisheries, bm->K_num_tot_sp, 0.0);

	/* Now insert the new value in the correct place */
	Util_Copy_Change_Values(bm, Qchange[species][fishery], tempArray[species][fishery], (int)bm->SP_FISHERYprms[species][fishery][Q_num_changes_id], start, period,
			mult);

	/* Now do some pointer magic and free up the 'old' array*/
	tempArrayPointer = Qchange;
	Qchange = tempArray;
	free4d(tempArrayPointer);
}

/**
 *
 * \brief Add a new mFC scaling event.
 *
 *
 */
void Harvest_Scale_mFC(MSEBoxModel *bm, int species, int fishery, double start, double period, double mult, FILE *llogfp) {
	double ****tempArray;
	double ****tempArrayPointer;

	/* Make sure this species and fishery combination is scaled */
	bm->flagchangeF = TRUE;

	/* Increase the number of scaling events */
	bm->SP_FISHERYprms[species][fishery][mFC_num_changes_id]++;

	if (((int) bm->SP_FISHERYprms[species][fishery][mFC_num_changes_id]) > mFC_max_num_changes) {
		mFC_max_num_changes = (int)bm->SP_FISHERYprms[species][fishery][mFC_num_changes_id];
	}

	/* Check to see if there is just a single dummy value in the array - if so just overwrite it */
	if (mFC_max_num_changes == 1 && mFCchange[species][fishery] == 0) {
		mFCchange[species][fishery][0][start_id] = start + bm->dayt;
		mFCchange[species][fishery][0][period_id] = period;
		mFCchange[species][fishery][0][mult_id] = mult;
		return;
	}

	/* Allocate a new array */
	tempArray = Util_Alloc_Init_4D_Double(3, Q_max_num_changes, bm->K_num_fisheries, bm->K_num_tot_sp, 0.0);

	/* Now insert the new value in the correct place */
	Util_Copy_Change_Values(bm, mFCchange[species][fishery], tempArray[species][fishery], (int)bm->SP_FISHERYprms[species][fishery][mFC_num_changes_id], start,
			period, mult);

	/* Now do some pointer magic and free up the 'old' array*/
	tempArrayPointer = mFCchange;
	mFCchange = tempArray;
	free4d(tempArrayPointer);
}

