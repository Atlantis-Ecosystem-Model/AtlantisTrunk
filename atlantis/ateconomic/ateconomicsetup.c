/**
 \file
 \brief C file for running the setup of economic models in Atlantis.
 \ingroup ateconomic

 File:		ateconomicsetup.c
 Created:	27/10/2004
 Author:		Beth Fulton,   CSIRO Marine Laboratories,  2004
 Purpose:	C file for running the setup of economic models in Atlantis.

 Note:

 Revisions:

 3/10/2005 Added contents of parameter readin routines
 initEconomic(), FreeEconParamArrays(), readModelEconParameter()
 and readEconValue()

 13/03/2008 Bec Gorton
 Added the code to convert the input file and parse the file based on
 tags.

 16-04-2008 Beth Fulton
 Fixed inQuota readin (in initEonomic) so ignores inactive subfleets and species
 with no quota restrictions


 02-02-2009 Bec Gorton
 Moved the freeing of the economic arrays in the bm structure into the Economic_Free
 function in this file instead of them being freed in the shutdownmodel function. This
 means that we can check to see if the conomic module is on - if not these
 arrays should not be freed.

 03-04-2009 Bec Gorton
 Changed the initEconomic function to not allocate any memory unless bm->flagecon_on is true.


 28-10-2009 Bec Gorton
 Removed references to the K_num_fished_sp and K_num_impacted_sp. Arrays that were this
 size have been resized to K_num_tot_sp.
 Also resized arrays any arrays allocated in this file that referred to either of these
 variables - these arrays are now K_num_tot_sp.

 04-11-2009 Bec Gorton
 Merged in Beths new bycatch incentive code - revision 961.

 27-08-2009 Beth Fulton
 Added option so clear whether TAC from harvest file is added to or superseded by infor from econ.prm

 30-09-3009 Bec Gorton
 Added some code to free up some new arrays correctly.

 04-05-2010 Bec Gorton
 Changed the allocation of the bm->blackBook array to use the K_num_BBook_prms value. This
 array was not being allocated the correct size resulting in a memory overwrite.

 15-11-2010 Bec Gorton
 Removed the readEconValue function. Changed the setup code to use the new XML input format.
 Renamed functions to match the new code format.
 Changed the indicator name setup functions to be local to this file - these are now called from Economics_Init not the main lib.

 26-11-2010 Beth Fulton
 Put in a more accurate (algebraically speaking) way of initialising discards

 11-02-2011 Bec Gorton
 Added code to read in the friendship network values from a csv file. Could probably
 be done in a more generic manner but this will do for now.

 11-10-2011 Beth Fulton
 Added a check whether bulkTAC and if it is (i.e. multiyear spanning TAC) then apply a multiplication of
 Allocation * tac_resetperiod_id length so that annual size allocation read-in converted to multi-year TAC


 *
 **/

/*  Open library routines */
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>
#include <sjwlib.h>

#include <atEconomic.h>
#include <atEconomicSetup.h>
#include "atHarvestLib.h"
#include "atManageLib.h"
#include <convertXML.h>

void Pre_Load_Array_Alloc(MSEBoxModel *bm);
void Post_Load_Array_Alloc(MSEBoxModel *bm);
static void Init_Econ_Indicator_Names(MSEBoxModel *bm);
static void Init_Entry_Names(MSEBoxModel *bm);
static void Init_Port_Index_Names(MSEBoxModel *bm);
static void Init_Econ_Index_Names(MSEBoxModel *bm);
int Read_FriendShip_Network(MSEBoxModel *bm, char *filePath);

/**
 *
 * \brief This routine is to initialise biological global data,
 * and is called by main() in transport module.
 * Initialisation.
 *
 * The routine Economic_Init() is called once at the beginning of
 * the run. It carries out the following steps.
 *
 * 1. Read economic parameter files
 *
 * 2. It allocates space for economic indicator vectors used in the economic models.
 *
 */
void Economic_Init(MSEBoxModel *bm, FILE *llogfp) {
	char convertedXMLFileName[STRLEN];
	double this_catch = 0.0, this_effort, max_month_effort, down_time, tot_quota_owned, tot_TAC, loadDetFC, avg_loadDetFC, dis_species_count;
	double divnumhr = 1.0 / 24.0;
	int b, nf, ns, porti, home_port, mth, sp, quitcheck, in_quota, flagspdiscard, chrt;
	/*int trade_sp; */
	int maxmonth = 12;
	int key_id = 0; // As only one economic property time series thus far - FIX make this read from array if get multiple time series

	printf("Initialise economics\n");

	if (!bm->flagecon_on) {
		/* Not using economics so skip it, set the arrays and leave */
		printf("Skipping economics as not active\n");

		return;
	}

	Init_Econ_Indicator_Names(bm);
	Init_Entry_Names(bm);
	/* Allocate memory for the box model arrays. */
	Pre_Load_Array_Alloc(bm);

	/* Build the converted filename */
	snprintf(convertedXMLFileName, sizeof(convertedXMLFileName), "%s", bm->econprmIfname);
	*(strstr(convertedXMLFileName, ".prm")) = '\0';
	strcat(convertedXMLFileName, ".xml");

	/* Convert the input file to XML */
	Convert_Economic_To_XML(bm, bm->econprmIfname, convertedXMLFileName);

	/* Now read in the input parameters */
	if (Read_Economic_Parameters(bm, convertedXMLFileName) == FALSE) {
		fprintf(llogfp, "Error in economics parameter file !! \n");
		quit("Error in economics parameter file !! \n");
	}

	/* if provided load the friendship file values */
	if(strlen(bm->friendship_file) > 0)
		Read_FriendShip_Network(bm, bm->friendship_file);

	/* Create economic arrays and set indices of all tracers and variables */
	Post_Load_Array_Alloc(bm);

	/* Populate arrays ***********************************************************/
	printf("Populate economic arrays\n");

	for (nf = 0; nf < bm->K_num_fisheries; nf++) {
		for (ns = 0; ns < bm->FISHERYprms[nf][nsubfleets_id]; ns++) {

			/* Correct unloading cost from per tonne to per kg */
			bm->SUBFLEET_ECONprms[nf][ns][unload_cost_id] /= 1000.0;
			/* Re4cord starting fleet size */
			bm->SUBFLEET_ECONprms[nf][ns][nboat_orig_id] = bm->SUBFLEET_ECONprms[nf][ns][nboat_id];

			bm->SUBFLEET_ECONprms[nf][ns][FishableLength_id] = bm->SUBFLEET_ECONprms[nf][ns][FishableLength_id] * divnumhr;
		}
	}

	/* Set up effective flex weights */
	if (!bm->hist_only) {
		for (nf = 0; nf < bm->K_num_fisheries; nf++) {
			for (ns = 0; ns < bm->FISHERYprms[nf][nsubfleets_id]; ns++) {
				bm->SUBFLEET_ECONprms[nf][ns][flexweight_id] = bm->SUBFLEET_ECONprms[nf][ns][flexweight_orig_id];
			}
		}
	} else {
		for (nf = 0; nf < bm->K_num_fisheries; nf++) {
			for (ns = 0; ns < bm->FISHERYprms[nf][nsubfleets_id]; ns++) {
				bm->SUBFLEET_ECONprms[nf][ns][flexweight_id] = 0.0;
			}
		}
	}

	/* If superseding TAC from harvest prm with that from econ prm warn the user */
	if (bm->UseEconQuotaDefine) {
		warn("Using TAC from econ.prm in place of that from harvest.prm\n");
	} else {
		warn("Adding quota from econ.prm to that from harvest.prm\n");
	}

	for (nf = 0; nf < bm->K_num_fisheries; nf++) {

		/* Convert quotas to kg from tonnes and update catch_allowed for the first time step */
		if (bm->FISHERYprms[nf][flageffortmodel_id] > rec_econ_model) {
			for (sp = 0; sp < bm->K_num_tot_sp; sp++) {
				if (FunctGroupArray[sp].isFished == TRUE) {
					bm->inQuota[nf][sp] = 0;
					bm->SP_FISHERYprms[sp][nf][catch_allowed] = 0;


					tot_quota_owned = 0;
					for (b = 0; b < bm->FISHERYprms[nf][nsubfleets_id]; b++) {
						if (bm->QuotaAlloc[nf][b][sp][owned_id] < no_quota)
							bm->inQuota[nf][sp] = 1;

						/* Convert from tonnes to kg */
						bm->QuotaAlloc[nf][b][sp][owned_id] *= 1000.0;


					   /* If using Bulk TAC then scale up allocation accordingly - so goes from annual to the multi-year period length */
					   if( bm->bulkTAC && (FunctGroupArray[sp].speciesParams[tac_resetperiod_id] > 1.0)) {
						   bm->QuotaAlloc[nf][b][sp][owned_id] *= FunctGroupArray[sp].speciesParams[tac_resetperiod_id];
					   }

					   // Get final sum
						tot_quota_owned += bm->QuotaAlloc[nf][b][sp][owned_id];
					}
					tot_TAC = 0;
					for (b = 0; b < bm->FISHERYprms[nf][nsubfleets_id]; b++) {
						/* Up date to include  "discard" quota contribution - stored in TACamt from harvest.prm
						 (typically when doing flagTACindcludDiscards so beefing up the total TAC to give room for discards) */
						if (!bm->UseEconQuotaDefine){ /* you can't have unallocated quota */
							bm->QuotaAlloc[nf][b][sp][owned_id] += (bm->TACamt[sp][nf][now_id] * bm->QuotaAlloc[nf][b][sp][owned_id] / (tot_quota_owned
									+ small_num));
						}
						bm->QuotaAlloc[nf][b][sp][oldquota_id] = bm->QuotaAlloc[nf][b][sp][owned_id];
                        if(!FunctGroupArray[sp].isTAC || (bm->TACamt[sp][nf][now_id] < no_quota)) {
                            tot_TAC += bm->QuotaAlloc[nf][b][sp][owned_id];
                        }

					}
					/* Update old TAC - make sure it the TAC value reflects the Quota and TAC values. */
					bm->TACamt[sp][nf][now_id] = tot_TAC;
					bm->TACamt[sp][nf][old_id] = bm->TACamt[sp][nf][now_id];

					fprintf(llogfp, "Quota on %s is %e kg for %s\n", FunctGroupArray[sp].groupCode, bm->TACamt[sp][nf][now_id], FisheryArray[nf].fisheryCode);

					/*trade_sp = (int) (bm->SP_FISHERYprms[sp][nf][flagquota_id]);
					if(trade_sp != bm->inQuota[nf][sp]){
					 printf("trade_sp = %d\n", trade_sp);
					 printf("sp = %d\n", sp);
					 printf("fishey =  %d\n", i);
					 quit("How can the species be under quota but not tradeable? There must be a mismatch between TAC_F?? in harvest.prm and OwnQuotaF?? in econ.prm\n");
					 }
					 */

					bm->SP_FISHERYprms[sp][nf][catch_allowed] = bm->TACamt[sp][nf][now_id] * kg_2_mg / bm->X_CN;
					if (bm->SP_FISHERYprms[sp][nf][catch_allowed] > bm->SP_FISHERYprms[sp][nf][trip_lim_id])
						bm->SP_FISHERYprms[sp][nf][catch_allowed] = bm->SP_FISHERYprms[sp][nf][trip_lim_id];
				}
				if (FunctGroupArray[sp].isFished == FALSE && FunctGroupArray[sp].isImpacted == TRUE) {
					bm->SP_FISHERYprms[sp][nf][catch_allowed] = MAXDOUBLE;
					bm->TACamt[sp][nf][now_id] = bm->SP_FISHERYprms[sp][nf][catch_allowed];
				}
			}
		}

		for (ns = 0; ns < bm->FISHERYprms[nf][nsubfleets_id]; ns++) {
		//for(ns=0; ns<bm->K_max_num_subfleet; ns++){ // Why would this need to occur beyond subfleets active in the fishery? everted to using subfleets for that fishery rather than max_num_subfleet
			this_catch = 0.0;
            avg_loadDetFC = 0.0;
            dis_species_count = 0.0;
			for (sp = 0; sp < bm->K_num_tot_sp; sp++) {
				if (FunctGroupArray[sp].isFished == TRUE) {
					/* Initialise subfleet targeting */
					bm->SUBFISHERYtarget[nf][ns][sp] = bm->FISHERYtarget[nf][sp];

					for (mth = 0; mth < 12; mth++) {
						/* Set initial BlackBook values for the starting year */
						if (flaglbs) {
							/* Convert from pounds if necessary */
							bm->BlackBook[nf][ns][sp][mth][hist_id] /= 2.2;
						}
						bm->BlackBook[nf][ns][sp][mth][expect_id] = bm->BlackBook[nf][ns][sp][mth][hist_id];
						bm->BlackBook[nf][ns][sp][mth][current_id] = bm->BlackBook[nf][ns][sp][mth][hist_id];
					}

					/* Initialise boatcatch - using average value per week in first month */
					bm->QuotaAlloc[nf][ns][sp][boatcatch_id] = bm->BlackBook[nf][ns][sp][0][hist_id] * (7.0 / 30.0);
					bm->QuotaAlloc[nf][ns][sp][cummonthcatch_id] = bm->BlackBook[nf][ns][sp][0][hist_id];

					/* Get expected discards */
					if (bm->TemporalBycatchAvoid || bm->SpatialBycatchAvoid) {
						loadDetFC = 0.0; /* Deal with compiler warning */
						flagspdiscard = (int) (bm->SP_FISHERYprms[sp][nf][flagdiscard_id]);
						chrt = (int) (FunctGroupArray[sp].speciesParams[age_mat_id]);
						switch (flagspdiscard) {
						case fixed_discard: /* Fixed proportion discarded */
							loadDetFC = bm->SP_FISHERYprms[sp][nf][FFCDR_id];
							break;
						case fixedage_discard: /* Fixed proportion per cohort discarded */
							loadDetFC = Harvest_Get_Fixed_Cohort_Discard(sp, nf, chrt, llogfp);
							break;
						case size_discard: /* Size based */
						case impose_discard: /* Read from time series */
						case depend_discard: /* Dependent on catch of other groups */
							/* Not coded as yet */
							quit(
									"No code for having bycatch taxation and this form of dicsarding (size-based, imposed or density dependent) - change flagdiscard or recode\n");
							break;
						}

						/* To get discards from catch you must multiply catch by (loadDetFC / (1 - loadDetFC))
						 * This is because
						 * 		Catch = Haul - Discards
						 * where Discards = Haul * loadDetFC
						 * so as Catch = Haul * (1 - loadDetFC)
						 * then Haul = Catch / (1 - loadDetFC)
						 * so substituting back in we get Discards = Catch * (loadDetFC / (1 - loadDetFC))
						 */
						bm->QuotaAlloc[nf][ns][sp][cummonthbycatch_id] = bm->BlackBook[nf][ns][sp][0][hist_id] * (loadDetFC / (small_num + (1.0 - loadDetFC))) * FunctGroupArray[sp].speciesParams[sp_concern_id];
						for(mth=0; mth<12; mth++){
							/* Set initial BlackBook bycatch values for the starting year */
							bm->BlackBook[nf][ns][sp][mth][bycatch_id] = bm->BlackBook[nf][ns][sp][mth][hist_id] * (loadDetFC / (small_num + (1.0 - loadDetFC))) * FunctGroupArray[sp].speciesParams[sp_concern_id];
							/*
							if(nf == bm->which_fleet){
								fprintf(llogfp,"Blackbook_bycatch for %s-%d on %s in mth: %d set = %e (hist: %e, loadDetFC: %e, spconcern: %e)\n",
									bm->fisheryNAME[nf], ns, bm->spNAME[sp], mth, bm->BlackBook[nf][ns][sp][mth][bycatch_id], bm->BlackBook[nf][ns][sp][mth][hist_id], loadDetFC, bm->SP_prms[sp][spconcern_id]);
							}
							*/
						}

                        /* Find average load to detritus - for discard calcualiton */
                        avg_loadDetFC += (loadDetFC / (small_num + (1.0 - loadDetFC))) * FunctGroupArray[sp].speciesParams[sp_concern_id];
                        if(FunctGroupArray[sp].speciesParams[sp_concern_id] > 0){
                            dis_species_count += 1.0;
                        }
					} else
						bm->QuotaAlloc[nf][ns][sp][cummonthbycatch_id] = 0;

					//fprintf(llogfp"bm->QuotaAlloc[%d][%d][%d][cummonthcatch_id] = %e\n", i, ns, sp, bm->QuotaAlloc[nf][ns][sp][cummonthcatch_id]);
					this_catch += bm->QuotaAlloc[nf][ns][sp][boatcatch_id];
				}
			}
			bm->SUBFLEET_ECONprms[nf][ns][CurrentCatch_id] = this_catch;
			//fprintf(llogfp, "this_catch = %.25e\n", this_catch);
			/* Give catch a starting values so can calculate a CPUE index for
			 initial effort allocation
			 */
            
            /* Get starting spatial discards */
            if(dis_species_count > 0.0){
                avg_loadDetFC /= dis_species_count;
            } else {
                avg_loadDetFC = 0.0;
            }
            for(b=0; b<bm->nbox; b++){
                bm->SpatialDisPUE[nf][ns][mth][b] += bm->SpatialCPUE[nf][ns][mth][b] * avg_loadDetFC;
            }

		}
	}

	/* Spatial CPUE and DisPUE - correct units (form pounds) for our American collegues */
	if (flaglbs) {
		for (b = 0; b < bm->nbox; b++) {
			for (nf = 0; nf < bm->K_num_fisheries; nf++) {
				for (ns = 0; ns < bm->FISHERYprms[nf][nsubfleets_id]; ns++) {
					for (mth = 0; mth < 12; mth++) {
						bm->SpatialCPUE[nf][ns][mth][b] /= 2.2;
						bm->SpatialDisPUE[nf][ns][mth][b] /= 2.2;
					}
				}
			}
		}
	}

	/* Deal with parameters specific to the Dan Holland economic model */
	if (!bm->MultiPlanEffort) {

		/* Set quota_trading = 0 as that is what is assumed by Holland model */
		if (bm->quota_trading) {
			warn("Resetting quota_trading = 0 as Holland economic model has no quota trading\n");
			bm->quota_trading = 0;
		}

		/* Warnings to do with assumptions behind Dan Holland model */
		warn("qscale not used in Holland economic effort allocation calculations, but may still be used in actual fishing code - so qs may not match exactly\n");
		warn("Regions for management identical over all species in model in Holland economic effort allocation model - location of regions set in biol.prm\n");
		warn("No change in vessel numbers through time in Holland economic effort allocation model\n");

		for (b = 0; b < bm->nbox; b++) {

			for (nf = 0; nf < bm->K_num_fisheries; nf++) {
				for (ns = 0; ns < bm->FISHERYprms[nf][nsubfleets_id]; ns++) {
					prop_tow_time[nf][ns][b] = prop_tow_time[nf][0][b];
					for (sp = 0; sp < bm->K_num_tot_sp; sp++) {
						if (FunctGroupArray[sp].isFished == TRUE) {
							//bm->ECONexprice[nf][ns][sp][b][hist_id] = bm->ECONexprice[midwcCEP_id][0][sp][b][hist_id];
							bm->ECONexprice[nf][ns][sp][b][expect_id] = bm->ECONexprice[nf][ns][sp][b][hist_id];
						}
					}
				}
			}
		}
	}

	/* Sanity check on whether effort is missing but still get catch */
	quitcheck = 0;
	for (nf = 0; nf < bm->K_num_fisheries; nf++) {
		if (bm->FISHERYprms[nf][flageffortmodel_id] > rec_econ_model) {
			for (ns = 0; ns < bm->FISHERYprms[nf][nsubfleets_id]; ns++) {
				for (mth = 0; mth < maxmonth; mth++) {
					this_effort = 0;
					for (b = 0; b < bm->nbox; b++) {
						this_effort += bm->SpatialBlackBook[nf][ns][mth][b][hist_id];
					}
					bm->BoxAlloc[nf][ns][mth][hist_id] = this_effort;
					for (sp = 0; sp < bm->K_num_tot_sp; sp++) {
						if (FunctGroupArray[sp].isFished == TRUE) {
							if (!this_effort && bm->BlackBook[nf][ns][sp][mth][hist_id]) {
								fprintf(llogfp, "%s-%d in month %d effort: %e %s catch: %e this is a problem - catch without effort\n",
										FisheryArray[nf].fisheryCode, ns, mth, this_effort, FunctGroupArray[sp].groupCode,
										bm->BlackBook[nf][ns][sp][mth][hist_id]);
								quitcheck = 1;
							}
						}
					}
				}
			}
		}
	}
	if (quitcheck)
		quit("Catch and effort mismatch - see log.txt\n");

	for (nf = 0; nf < bm->K_num_fisheries; nf++) {
		for (ns = 0; ns < bm->FISHERYprms[nf][nsubfleets_id]; ns++) {
			/* Sanity check on effort levels (in days at sea vs number of boats per subfleet) */
			down_time = bm->SUBFLEET_ECONprms[nf][ns][down_time_id];
			max_month_effort = bm->month_scalar * (1 - down_time) * bm->SUBFLEET_ECONprms[nf][ns][nboat_id];
			for (mth = 0; mth < maxmonth; mth++) {
				this_effort = 0;
				for (b = 0; b < bm->nbox; b++) {
					this_effort += bm->SpatialBlackBook[nf][ns][mth][b][hist_id] * (7.0 / 30.0);

				}
				if (this_effort > max_month_effort) {
					for (b= 0; b < bm->nbox; b++) {
						/* Rescale to maximum humanly possible */
						bm->SpatialBlackBook[nf][ns][mth][b][hist_id] *= (max_month_effort / (this_effort + small_num));

					}
				}
			}

			/* Initialise effort records */
			this_effort = 0;
			mth = 0;
			for (b = 0; b < bm->nbox; b++) {
				this_effort += bm->SpatialBlackBook[nf][ns][0][b][hist_id] * (7.0 / 30.0);
			}
			/* Give catch a starting values to effort so can calculate a CPUE index for
			 initial effort allocation - using average per week in first month of
			 historical data.
			 */
			bm->SUBFLEET_ECONprms[nf][ns][CurrentEffort_id] = this_effort;
		}
	}

	/* Now split off total Effort vs relative spatial distribution */
	for (nf = 0; nf < bm->K_num_fisheries; nf++) {
		for (ns = 0; ns < bm->FISHERYprms[nf][nsubfleets_id]; ns++) {
			for (mth = 0; mth < maxmonth; mth++) {
				/* Get total historical monthly effort */
				for (b = 0; b < bm->nbox; b++) {
					bm->EffortSchedule[nf][ns][mth][hist_id] += bm->SpatialBlackBook[nf][ns][mth][b][hist_id];
				}
				/* Now normalise historical spatial effort to make it relative */
				for (b = 0; b < bm->nbox; b++) {
					bm->SpatialBlackBook[nf][ns][mth][b][hist_id] /= (bm->EffortSchedule[nf][ns][mth][hist_id] + small_num);
					bm->SpatialBlackBook[nf][ns][mth][b][updated_id] = bm->SpatialBlackBook[nf][ns][mth][b][hist_id];
				}
			}
		}
	}

	/* Get final number of boats per fishery */
	for (nf = 0; nf < bm->K_num_fisheries; nf++) {
		bm->FISHERYprms[nf][nvessel_id] = 0;
		for (ns = 0; ns < bm->FISHERYprms[nf][nsubfleets_id]; ns++) {
            
            printf("Doing %d subfleet of %s\n", ns, FisheryArray[nf].fisheryCode);
            
			/* Total up the boats */
			bm->FISHERYprms[nf][nvessel_id] += bm->SUBFLEET_ECONprms[nf][ns][nboat_id];
            
            printf("Fishery %s has %e vessels as adding %e from subfleet %d\n", FisheryArray[nf].fisheryCode, bm->FISHERYprms[nf][nvessel_id], bm->SUBFLEET_ECONprms[nf][ns][nboat_id], ns);
		}
        // Check this total against the number of available licences - if more boats than licences then quit.
        if (bm->FISHERYprms[nf][nvessel_id] > bm->FISHERYprms[nf][nlicence_id])
            quit("The fishery %s has more vessels (%e) than licences (%e) - reset the number of licences or vessels.\n", FisheryArray[nf].fisheryCode, bm->FISHERYprms[nf][nvessel_id], bm->FISHERYprms[nf][nlicence_id]);
	}

	/* Initialise subfleet marginal rent and profit */
	Initialise_Subfleets(bm, llogfp);

	if(bm->flagecon_on){
		FisheryTimeSeries *this_ts;

		/* Read economic time series */
		Read_Econ_Time_Series(bm->inputFolder, bm->forceIfname, "Econ", bm->t_units, &bm->tsGDP);

		/* This test just so can have the FisheryTimeSeries definition here
		 (to get this far flagecon_on must be > 0 anyway */
		this_ts = &bm->tsGDP[key_id];

		/* Assign economic property ids (currently only GDP and fuel costs) */
		bm->GDP_id = -1;
		bm->FuelCost_id = -1;
		for (b = 0; b < this_ts->ts.nv; b++) {
			if (strcmp("GDP", this_ts->ts.varname[b]) == 0) {
				bm->GDP_id = b;
			} else if (strcmp("FuelCost", this_ts->ts.varname[b]) == 0) {
				bm->FuelCost_id = b;
			}
		}
		if (bm->GDP_id < 0)
			quit("Did not find time series of GDP in %s - check the ts files and retry\n", bm->tsGDP[key_id].ts.df->name);
		if (bm->FuelCost_id < 0)
			quit("Did not find time series of FuelCost in %s - check the ts files and retry\n", bm->tsGDP[key_id].ts.df->name);
	}

	/* Set port activity levels */
	for (porti = 0; porti < bm->K_num_ports; porti++) {
		portweight[porti] = 0;
	}

	for (nf = 0; nf < bm->K_num_fisheries; nf++) {
		for (ns = 0; ns < bm->FISHERYprms[nf][nsubfleets_id]; ns++) {
			/* If no boats in the subfleet currently skip ahead */
			if (!bm->SUBFLEET_ECONprms[nf][ns][nboat_id]) {
				continue;
			}

			home_port = (int) (bm->SUBFLEET_ECONprms[nf][ns][home_port_id]);
			if (home_port < bm->K_num_ports) {
				portweight[home_port] += bm->SUBFLEET_ECONprms[nf][ns][nboat_id];
			}
		}

//		fprintf(llogfp, "BycatchDiscount rate for %s-%d = %e\n", FisheryArray[nf].fisheryCode, ns,
//				bm->SUBFLEET_ECONprms[nf][ns][bycatchDiscount_id]);

	}

	for (porti = 0; porti < bm->K_num_ports; porti++) {
		if (!portweight[porti])
			portweight[porti] = 1.0;
		bm->Port_info[porti][new_act_id] = portweight[porti];
	}

	if (bm->hist_only) {
		warn("Using historical forcing of economic effort only with timeout %e\n", bm->hist_only_timeout);
		fprintf(llogfp, "Using historical forcing of economic effort only with timeout %e\n", bm->hist_only_timeout);
	}

	/* Rescale and printout starting quotas in effect */
	for (sp = 0; sp < bm->K_num_tot_sp; sp++) {
		if (FunctGroupArray[sp].isFished == TRUE) {
			tot_quota_owned = 0;
			tot_TAC = 0;
			for (nf = 0; nf < bm->K_num_fisheries; nf++) {
				in_quota = bm->inQuota[nf][sp];
				if (in_quota && (bm->FISHERYprms[nf][flagrecfish_id] < 1)) {
					/* Don't count recfishing in this as only want commercial quota */
                    if(!FunctGroupArray[sp].isTAC || (bm->TACamt[sp][nf][now_id] < no_quota)) {
                        tot_TAC += bm->TACamt[sp][nf][now_id];
                    }

					fprintf(llogfp, "%s %s TAC: %e (totTAC: %e)\n", FunctGroupArray[sp].groupCode, FisheryArray[nf].fisheryCode, bm->TACamt[sp][nf][now_id],
							tot_TAC);

					for (ns = 0; ns < bm->FISHERYprms[nf][nsubfleets_id]; ns++) {
						tot_quota_owned += bm->QuotaAlloc[nf][ns][sp][owned_id];

						fprintf(llogfp, "%s by %s-%d owned: %e\n", FunctGroupArray[sp].groupCode, FisheryArray[nf].fisheryCode, ns, bm->QuotaAlloc[nf][ns][sp][owned_id]);
					}
				}
			}
			//fprintf(llogfp,"%s TAC: %e with %e owned by starting vessels\n", FunctGroupArray[guild].groupCode[spp_id], mg_2_kg, tot_quota_owned);
			FunctGroupArray[sp].speciesParams[TACorig_id] = tot_TAC;
		}
	}
	for (sp = 0; sp < bm->K_num_tot_sp; sp++) {
	//	} else if (FunctGroupArray[sp].isImpacted == TRUE) {
		if (FunctGroupArray[sp].isFished == FALSE && FunctGroupArray[sp].isImpacted == TRUE) {
			for (nf = 0; nf < bm->K_num_fisheries; nf++) {
				if (bm->FISHERYprms[nf][flageffortmodel_id] > rec_econ_model) {
					bm->TACamt[sp][nf][now_id] = 1000000000.0;
					bm->inQuota[nf][sp] = 0;
//					for (ns = 0; ns < bm->FISHERYprms[nf][nsubfleets_id]; ns++) {
//						bm->QuotaAlloc[nf][ns][sp][owned_id] = 1000000000.0;
//					}
				}
			}
		}
	}


	/* To keep harvest and economic libaries in synch, find new max price off prices
	 read in from the economics parameter file
	 */
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

	bm->reset_month_val = 0;
	bm->renewTrade = 0;
	bm->TaxDone = 0;

	/* Initialise the tac reset values */
	for (sp = 0; sp < bm->K_num_tot_sp; sp++) {
		FunctGroupArray[sp].speciesParams[tac_resetnow_id] = 1;
	}


	for (sp = 0; sp < bm->K_num_tot_sp; sp++) {
		if (FunctGroupArray[sp].isImpacted == TRUE) {
			fprintf(llogfp, "%s spconcern: %e\n", FunctGroupArray[sp].groupCode, FunctGroupArray[sp].speciesParams[sp_concern_id]);
		}
	}

    /* If doing CPUE generation create the arrays - at some point this may need to be moved back to management lib */
    if(bm->flagStoreShotCPUE){
        CreateCPUEGenArrays(bm, llogfp);
        GenerateCPUEDistribution(bm, llogfp);
    }
    
	/* Setup the indicator strings */
	Init_Econ_Index_Names(bm);
	Init_Port_Index_Names(bm);

	/* Open the output files */
	Open_Econ_Indx_Files(bm);

	return;
}

/**
 *
 *	\brief Subfleet initialisation
 *
 *	Initialise marginal profit and rent per month for subfleets
 */
void Initialise_Subfleets(MSEBoxModel *bm, FILE *llogfp) {
	int nf, ns, sp, month;
	double rent_scale, tot_cash;

	for (month = 0; month < 12; month++) {
		for (nf = 0; nf < bm->K_num_fisheries; nf++) {
			if (bm->FISHERYprms[nf][flageffortmodel_id] > rec_econ_model) {
				for (ns = 0; ns < bm->FISHERYprms[nf][nsubfleets_id]; ns++) {
					tot_cash = 0;
					for (sp = 0; sp < bm->K_num_tot_sp; sp++) {
						if (FunctGroupArray[sp].isFished == TRUE) {
							/* If can't catch the group skip ahead */
							if (!bm->SP_FISHERYprms[sp][nf][q_id])
								continue;

							/* Put in average historical value (summed over all months) as
							 starting value of previous catch - for calculating quota prices etc */
							bm->QuotaAlloc[nf][ns][sp][cumboatcatch_id] += bm->BlackBook[nf][ns][sp][month][hist_id];

							/* Initial marginal profit - boatcatch is from first historic catches
							 (read in from param file) averaged over the year. Soon updated by actual
							 catches or end of historical catches so little impact on projection
							 dynamics. FIX - is this true or need to read in season catch for all? */
							bm->MargProfit[nf][ns][sp][month] = bm->SP_FISHERYprms[sp][nf][saleprice_id] - bm->SUBFLEET_ECONprms[nf][ns][cost_ind_id]
									/ (bm->QuotaAlloc[nf][ns][sp][boatcatch_id] + small_num);
							tot_cash += bm->MargProfit[nf][ns][sp][month] * bm->BlackBook[nf][ns][sp][month][hist_id];
						}
					}
					bm->SUBFLEET_ECONprms[nf][ns][tot_cash_id] = tot_cash;
				}
			}
		}
	}

	for (nf = 0; nf < bm->K_num_fisheries; nf++) {
		if (bm->FISHERYprms[nf][flageffortmodel_id] > rec_econ_model) {
			for (ns = 0; ns < bm->FISHERYprms[nf][nsubfleets_id]; ns++) {
				for (sp = 0; sp < bm->K_num_tot_sp; sp++) {
					if (FunctGroupArray[sp].isFished == TRUE) {

						/* If can't catch the group skip ahead */
						if (!bm->SP_FISHERYprms[sp][nf][q_id])
							continue;
						/* Initial marginal rent */
						for (month = 0; month < 12; month++) {
							bm->QuotaAlloc[nf][ns][sp][spmarg_rent_id] += bm->MargProfit[nf][ns][sp][month] / (bm->FISHERYprms[nf][nvessel_id] + small_num);
						}
						/* If single vessel fleet then rescale so some rent left from profit */
						rent_scale = 1.0;
						if (bm->FISHERYprms[nf][nvessel_id] == 1.0)
							rent_scale = 0.9;

						for (month = 0; month < 12; month++) {
							bm->MargRent[nf][ns][sp][month] = bm->MargProfit[nf][ns][sp][month] - rent_scale * bm->MargProfit[nf][ns][sp][month]
									/ (bm->FISHERYprms[nf][nvessel_id] + small_num);

							//fprintf(llogfp,"Time: %e, %s (%d) %s, mth: %d, MargProfit: %e, rent_scale: %e, nvessel: %e\n",
							//	bm->dayt, FisheryArray[nf].fisheryCode, ns, FunctGroupArray[sp].groupCode, month, bm->MargProfit[nf][ns][sp][month], rent_scale, bm->FISHERYprms[nf][nvessel_id]);

						}
					}
				}
			}
		}
	}

	return;
}

/**
 *	\brief  Initialise parameter arrays
 *
 */
void InitialiseEconArrays(MSEBoxModel *bm) {
	int nf, ns, i, j, sp, b;
	int maxmonth = 12;
	int totpi = bm->K_max_num_subfleet * bm->K_num_fisheries * maxmonth;

	if (verbose)
		printf("Initialising economic arrays\n");

	for (nf = 0; nf < bm->K_num_fisheries; nf++) {
		for (ns = 0; ns < bm->K_max_num_subfleet; ns++) {
			net_return[nf][ns][immed_id] = 0;
			net_return[nf][ns][fut_id] = 0;
			for (i = 0; i < bm->K_num_fisheries; i++) {
				for (j = 0; j < bm->K_max_num_subfleet; j++) {
					friendship_netwk[nf][i][ns][j] = 1.0;
				}
			}
			for (i = 0; i < K_sub_fleet_params; i++) {
				bm->SUBFLEET_ECONprms[nf][ns][i] = 0;
			}
			for (sp = 0; sp < bm->K_num_tot_sp; sp++) {
				if (FunctGroupArray[sp].isFished == TRUE) {
					bm->SUBFISHERYtarget[nf][ns][sp] = 0;
					for (i = 0; i < maxmonth; i++) {
						bm->MargRent[nf][ns][sp][i] = 0;
						bm->MargProfit[nf][ns][sp][i] = 0;
						bm->MonthAlloc[nf][ns][sp][i] = 0;
						for (j = 0; j < 4; j++) {
							bm->BlackBook[nf][ns][sp][i][j] = 0;
						}
					}
					for (i = 0; i < K_num_sp_econ_prms; i++) {
						bm->QuotaAlloc[nf][ns][sp][i] = 0;
					}
					for (i = 0; i < bm->nbox; i++) {
						prop_tow_time[nf][ns][i] = 1.0;
						for (j = 0; j < 3; j++) {
							bm->ECONexprice[nf][ns][sp][i][j] = 0;
						}
					}
					for (i = 0; i < bm->K_num_reg; i++) {
						RegCatch[nf][ns][i][sp] = 0;
					}
				}
			}
			for (i = 0; i < maxmonth; i++) {
				for (j = 0; j < 3; j++) {
					bm->BoxAlloc[nf][ns][i][j] = 0;
					bm->EffortSchedule[nf][ns][i][j] = 0;
				}
				bm->EffortSchedule[nf][ns][i][3] = 0;
				for (b = 0; b < bm->nbox; b++) {
					for (j = 0; j < K_num_BBook_prms; j++) {
						bm->SpatialBlackBook[nf][ns][i][b][j] = 0;
					}
					bm->SpatialCPUE[nf][ns][i][b] = 0;

					bm->EffortTrades[nf][i][source_id] = 0;
					bm->EffortTrades[nf][i][sink_id] = 0;
				}
			}
			boats_free[nf][ns] = 0;
			tot_marg_rent[nf][ns] = 0;
		}
		best_subfleet[nf] = -1;
	}

	for (i = 0; i < totpi; i++) {
		ax[i] = 0;
		bx[i] = 0;
		cx[i] = 0;
		dx[i] = 0;
		ex[i] = 0;
	}

	for (sp = 0; sp < bm->K_num_tot_sp; sp++) {
		if (FunctGroupArray[sp].isFished == TRUE) {
			maxTargetCatch[sp] = 0;
			tempTarget[sp] = 0;
			all_pi[sp] = 0;
			orig_pi[sp] = 0;
			raw_pi[sp] = 0;
			any2spare[sp] = 0;
			max_demand[sp] = 0;
			tradeable_quota[sp] = 0;
			for (b = 0; b < maxmonth; b++) {
				bm->Trades[b][sp] = 0;
			}
			for (i = 0; i < bm->K_num_reg; i++) {
				totland[sp][i] = 0;
				tot_cumcatch[sp][i] = 0;
				quota_left[sp][i] = 0;
			}
		}
	}

	for (i = 0; i < bm->K_num_ports; i++) {
		portweight[i] = 0;
	}

	for (i = 0; i < K_econ_indicators; i++) {
		for (ns = 0; ns < K_ecolharvest_indicators; ns++) {
			for (sp = 0; sp < K_num_entry_indabc; sp++) {
				bm->ECON_INDprms[i][ns][sp] = 0;
			}
		}
	}

	for (i = 0; i < maxmonth; i++) {
		new_effort[i] = 0;
		old_effort[i] = 0;
		redo_effort[i] = 0;
	}

	return;
}

/**
 *	\brief Free economic arrays
 *
 */
void Economic_Free(MSEBoxModel *bm) {

	if (!bm->flagecon_on) {
		/* Not using economics so skip it */
		printf("Skip freeing assessment specific arrays\n");
		return;
	}

	Close_Econ_Indx_Files(bm);
	printf("Freeing economic specific arrays\n");
	c_free2d(EconIndicatorInputNames);
	c_free2d(entryInputNames);
	c_free2d(econInputNames);

	free1d(all_pi);
	free1d(any2spare);
	free1d(ax);
	i_free1d(best_subfleet);
	i_free2d(boats_free);

	i_free2d(boats_new);
	free1d(bx);
	free1d(cx);
	free1d(dx);
	free1d(ex);
	free1d(maxTargetCatch);
	free4d(friendship_netwk);
	free1d(max_demand);
	free3d(net_return);
	free1d(new_effort);
	free1d(old_effort);
	free1d(orig_pi);
	free4d(pi);
	free1d(portweight);
	free3d(prop_tow_time);
	free1d(raw_pi);
	i_free1d(redo_effort);
	i_free1d(tempTarget);
	free1d(tradeable_quota);
	free2d(tot_marg_rent);
	free4d(RegCatch);
	free2d(tot_cumcatch);
	free2d(quota_left);
	free2d(totland);
	free1d(quota_check);
    
	printf("Freeing economics arrays\n");
    
    // Clean up if haven't already
    if(bm->EffortReScale)
        free1d(bm->EffortReScale);
    if(bm->CatchReScale)
        free1d(bm->CatchReScale);
    
	free5d(bm->BlackBook);
	free4d(bm->BoxAlloc);
	free3d(bm->ECON_INDprms);
	free5d(bm->ECONexprice);
	free4d(bm->EffortSchedule);
	free4d(bm->MargProfit);
	free4d(bm->MargRent);
	free4d(bm->MonthAlloc);
	free4d(bm->QuotaAlloc);
	free4d(bm->QuotaTrade);
	free5d(bm->SpatialBlackBook);
	free4d(bm->SpatialCPUE);
	free4d(bm->SpatialDisPUE);
	free4d(bm->SpatialVPUE);
	i_free3d(bm->SUBFISHERYtarget);
	free3d(bm->SUBFLEET_ECONprms);
	i_free2d(bm->Trades);
	i_free3d(bm->EffortTrades);

	if (bm->tsGDP->ts.varname != NULL) {
		int i;
		for (i = 0; i < bm->tsGDP->ts.nv; ++i) {

			if (bm->tsGDP->ts.varname[i] != NULL)
				free(bm->tsGDP->ts.varname[i]);
			if (bm->tsGDP->ts.varunit[i] != NULL)
				free(bm->tsGDP->ts.varunit[i]);

		}
		free(bm->tsGDP->ts.varname);
		free(bm->tsGDP->ts.varunit);
	}

	dfFree(bm->tsGDP->ts.df);
	free(bm->tsGDP);
    
    free1d(bm->prop_hold);
    
	return;
}

/**
 * \brief Allocate the economic arrays prior to loading data from the
 *	economic input arrays.
 *
 *
 */
void Pre_Load_Array_Alloc(MSEBoxModel *bm) {
	//printf("Creating econ PreLoad arrays\n");

	bm->BlackBook = Util_Alloc_Init_5D_Double(K_num_BBook_prms, 12, bm->K_num_tot_sp, bm->K_max_num_subfleet, bm->K_num_fisheries, 0.0);
	bm->BoxAlloc = Util_Alloc_Init_4D_Double(3, 12, bm->K_max_num_subfleet, bm->K_num_fisheries, 0.0);
	bm->ECON_INDprms = Util_Alloc_Init_3D_Double(K_num_entry_indabc, K_ecolharvest_indicators, K_econ_indicators, 0.0);
	bm->ECONexprice = Util_Alloc_Init_5D_Double(3, bm->nbox, bm->K_num_tot_sp, bm->K_max_num_subfleet, bm->K_num_fisheries, 0.0);
	bm->EffortSchedule = Util_Alloc_Init_4D_Double(4, 12, bm->K_max_num_subfleet, bm->K_num_fisheries, 0.0);
	bm->MargProfit = Util_Alloc_Init_4D_Double(12, bm->K_num_tot_sp, bm->K_max_num_subfleet, bm->K_num_fisheries, 0.0);
	bm->MargRent = Util_Alloc_Init_4D_Double(12, bm->K_num_tot_sp, bm->K_max_num_subfleet, bm->K_num_fisheries, 0.0);
	bm->MonthAlloc = Util_Alloc_Init_4D_Double(12, bm->K_num_tot_sp, bm->K_max_num_subfleet, bm->K_num_fisheries, 0.0);
	bm->QuotaAlloc = Util_Alloc_Init_4D_Double(K_num_sp_econ_prms, bm->K_num_tot_sp, bm->K_max_num_subfleet, bm->K_num_fisheries, 0.0);
	bm->QuotaTrade = Util_Alloc_Init_4D_Double(2, bm->K_num_tot_sp, bm->K_num_fisheries, bm->K_num_fisheries, 0.0);
	bm->SpatialBlackBook = Util_Alloc_Init_5D_Double(K_num_BBook_prms, bm->nbox, 12, bm->K_max_num_subfleet, bm->K_num_fisheries, 0.0);
	bm->SpatialCPUE = Util_Alloc_Init_4D_Double(bm->nbox, 12, bm->K_max_num_subfleet, bm->K_num_fisheries, 0.0);
	bm->SpatialVPUE = Util_Alloc_Init_4D_Double(bm->nbox, 12, bm->K_max_num_subfleet, bm->K_num_fisheries, 0.0);
	bm->SpatialDisPUE = Util_Alloc_Init_4D_Double(bm->nbox, 12, bm->K_max_num_subfleet, bm->K_num_fisheries, 0.0);
	bm->SUBFISHERYtarget = (int ***) Util_Alloc_Init_3D_Double(bm->K_num_tot_sp, bm->K_max_num_subfleet, bm->K_num_fisheries, 0.0);
	bm->SUBFLEET_ECONprms = Util_Alloc_Init_3D_Double(K_sub_fleet_params, bm->K_max_num_subfleet, bm->K_num_fisheries, 0.0);
	bm->Trades = Util_Alloc_Init_2D_Int(bm->K_num_tot_sp, 12, 0);
	bm->EffortTrades = Util_Alloc_Init_3D_Int(bm->K_num_fisheries, 12, 2, 0);
	friendship_netwk = Util_Alloc_Init_4D_Double(bm->K_max_num_subfleet, bm->K_max_num_subfleet, bm->K_num_fisheries, bm->K_num_fisheries, 1.0);
	prop_tow_time = Util_Alloc_Init_3D_Double(bm->nbox, bm->K_max_num_subfleet, bm->K_num_fisheries, 1.0);
    
    bm->EffortReScale = Util_Alloc_Init_1D_Double(bm->K_num_fisheries, 0.0);
    bm->CatchReScale = Util_Alloc_Init_1D_Double(bm->K_num_tot_sp, 0.0);
    
}

/**
 * \brief Allocate the economic arrays after loading data from the
 *	economic input arrays.
 *
 */
void Post_Load_Array_Alloc(MSEBoxModel *bm) {
	int maxmonth = 12;
	int totpi = bm->K_max_num_subfleet * bm->K_num_fisheries * maxmonth;

	//printf("Creating econ PostLoad arrays\n");

	/* Create economic arrays */
	all_pi = Util_Alloc_Init_1D_Double(bm->K_num_tot_sp, 0.0);
	any2spare = Util_Alloc_Init_1D_Double(bm->K_num_tot_sp, 0.0);
	ax = Util_Alloc_Init_1D_Double(totpi, 0.0);
	best_subfleet = Util_Alloc_Init_1D_Int(bm->K_num_fisheries, 0);

	boats_free = Util_Alloc_Init_2D_Int(bm->K_max_num_subfleet, bm->K_num_fisheries, 0);
	boats_new = Util_Alloc_Init_2D_Int(bm->K_max_num_subfleet, bm->K_num_fisheries, 0);

	bx = Util_Alloc_Init_1D_Double(totpi, 0.0);
	cx = Util_Alloc_Init_1D_Double(totpi, 0.0);
	dx = Util_Alloc_Init_1D_Double(totpi, 0.0);
	ex = Util_Alloc_Init_1D_Double(totpi, 0.0);

	max_demand = Util_Alloc_Init_1D_Double(bm->K_num_tot_sp, 0.0);
	maxTargetCatch = Util_Alloc_Init_1D_Double(bm->K_num_tot_sp, 0.0);
	net_return = Util_Alloc_Init_3D_Double(2, bm->K_max_num_subfleet, bm->K_num_fisheries, 0.0);
	new_effort = Util_Alloc_Init_1D_Double(12, 0.0);
	old_effort = Util_Alloc_Init_1D_Double(12, 0.0);
	orig_pi = Util_Alloc_Init_1D_Double(bm->K_num_tot_sp, 0.0);
	pi = Util_Alloc_Init_4D_Double(bm->K_num_tot_sp, maxmonth, bm->K_max_num_subfleet, bm->K_num_fisheries, 0.0);
	portweight = Util_Alloc_Init_1D_Double(bm->K_num_ports, 0.0);
	quota_left = Util_Alloc_Init_2D_Double(bm->K_num_reg, bm->K_num_tot_sp, 0.0);
	quota_check = Util_Alloc_Init_1D_Double(bm->K_num_tot_sp, 0.0);
	raw_pi = Util_Alloc_Init_1D_Double(bm->K_num_tot_sp, 0.0);
	redo_effort = Util_Alloc_Init_1D_Int(12, 0);
	RegCatch = Util_Alloc_Init_4D_Double(bm->K_num_tot_sp, bm->K_num_reg, bm->K_max_num_subfleet, bm->K_num_fisheries, 0.0);
	tempTarget = Util_Alloc_Init_1D_Int(bm->K_num_tot_sp, 0);
	tradeable_quota = Util_Alloc_Init_1D_Double(bm->K_num_tot_sp, 0.0);
	tot_cumcatch = Util_Alloc_Init_2D_Double(bm->K_num_reg, bm->K_num_tot_sp, 0.0);
	totland = Util_Alloc_Init_2D_Double(bm->K_num_reg, bm->K_num_tot_sp, 0.0);
	tot_marg_rent = Util_Alloc_Init_2D_Double(bm->K_max_num_subfleet, bm->K_num_fisheries, 0.0);
    
    // For CPUE gen purproses
    bm->prop_hold = Util_Alloc_Init_1D_Double(bm->K_max_num_subfleet, 0.0);

}

void Init_Econ_Indicator_Names(MSEBoxModel *bm) {
    int paramLen = 15;
	EconIndicatorInputNames = (char **) c_alloc2d(paramLen, K_sub_fleet_params);

	snprintf(EconIndicatorInputNames[nboat_id], paramLen, "%s", "nboat");
	snprintf(EconIndicatorInputNames[boat_size_id], paramLen, "%s", "boat_size");
	snprintf(EconIndicatorInputNames[crew_size_id], paramLen, "%s", "crewsize");
	snprintf(EconIndicatorInputNames[home_port_id], paramLen, "%s", "home_port");
	snprintf(EconIndicatorInputNames[FCwgtscale_id], paramLen, "%s", "FCwgtscale");
	snprintf(EconIndicatorInputNames[max_trip_length_id], paramLen, "%s", "triplength(days)");
	snprintf(EconIndicatorInputNames[FishableLength_id], paramLen, "%s", "FishableLength");
	snprintf(EconIndicatorInputNames[down_time_id], paramLen, "%s", "DownTime");
	snprintf(EconIndicatorInputNames[choicebuffer_id], paramLen, "%s", "choicebuffer");
	snprintf(EconIndicatorInputNames[effortDiscount_id], paramLen, "%s", "EffortDiscount");
	snprintf(EconIndicatorInputNames[bycatchDiscount_id], paramLen, "%s", "BycatchDiscount");
	snprintf(EconIndicatorInputNames[var_cost_id], paramLen, "%s", "varcost");
	snprintf(EconIndicatorInputNames[supp_cost_id], paramLen, "%s", "suppcost");
	snprintf(EconIndicatorInputNames[switch_cost_id], paramLen, "%s", "switchcost");
	snprintf(EconIndicatorInputNames[newboat_cost_id], paramLen, "%s", "newcost");
	snprintf(EconIndicatorInputNames[tol_debt_id], paramLen, "%s", "TolDebt");
	snprintf(EconIndicatorInputNames[fuel_cost_id], paramLen, "%s", "fuelcost");
	snprintf(EconIndicatorInputNames[gear_cost_id], paramLen, "%s", "gearcost");
	snprintf(EconIndicatorInputNames[unload_cost_id], paramLen, "%s", "unloadcost");
	snprintf(EconIndicatorInputNames[fixed_cost_id], paramLen, "%s", "fixedcost");
	snprintf(EconIndicatorInputNames[capital_cost_id], paramLen, "%s", "capitalcost");
	snprintf(EconIndicatorInputNames[crewshare_id], paramLen, "%s", "crewshare");
	snprintf(EconIndicatorInputNames[mgmt_cost_id], paramLen, "%s", "mgmtcost");
	snprintf(EconIndicatorInputNames[flexweight_orig_id], paramLen, "%s", "flexweight");
	snprintf(EconIndicatorInputNames[resale_id], paramLen, "%s", "resale");
	snprintf(EconIndicatorInputNames[buyout_id], paramLen, "%s", "buyout");
	snprintf(EconIndicatorInputNames[propbuyback_id], paramLen, "%s", "propbuyback");
	snprintf(EconIndicatorInputNames[flag_econind_id], paramLen, "%s", "flageconind");
	snprintf(EconIndicatorInputNames[flag_indtype_id], paramLen, "%s", "flagtypeind");
	snprintf(EconIndicatorInputNames[betarev_id], paramLen, "%s", "betarev");
	snprintf(EconIndicatorInputNames[minprob_id], paramLen, "%s", "minprob");
	snprintf(EconIndicatorInputNames[down_time_id], paramLen, "%s", "minDownTime");
	snprintf(EconIndicatorInputNames[hold_capacity_id], paramLen, "%s", "hold_capacity");

}

void Init_Entry_Names(MSEBoxModel *bm) {
    int paramLen = 15;
	entryInputNames = (char **) c_alloc2d(paramLen, K_num_entry_indabc);

	snprintf(entryInputNames[ecoefft_ind], paramLen, "%s", "ecoefft");
	snprintf(entryInputNames[escale_ind], paramLen, "%s", "escale");
	snprintf(entryInputNames[escaleB_ind], paramLen, "%s", "escaleB");
	snprintf(entryInputNames[ethresh_ind], paramLen, "%s", "ethresh");
	snprintf(entryInputNames[eindtype_ind], paramLen, "%s", "eindtype");

	econInputNames = (char **) c_alloc2d(paramLen, K_econ_indicators);

	snprintf(econInputNames[igvp_id], paramLen, "%s", "GVP");
	snprintf(econInputNames[irevland_id], paramLen, "%s", "RevLand");
	snprintf(econInputNames[irevenue_id], paramLen, "%s", "Revenue");
	snprintf(econInputNames[icost_id], paramLen, "%s", "Cost");
	snprintf(econInputNames[icash_id], paramLen, "%s", "Cash");
	snprintf(econInputNames[iinvest_id], paramLen, "%s", "Invest");
	snprintf(econInputNames[icaputil_id], paramLen, "%s", "CapUtil");
	snprintf(econInputNames[ileaseQ_id], paramLen, "%s", "LeaseQ");
	snprintf(econInputNames[isaleQ_id], paramLen, "%s", "SaleQ");
	snprintf(econInputNames[iQtrade_id], paramLen, "%s", "TradeQ");
	snprintf(econInputNames[icostton_id], paramLen, "%s", "CostTon");
}


/**
 *
 *	\brief Economic Index names set
 *	This sets up names of economics performance indices
 *
 */
void Init_Econ_Index_Names(MSEBoxModel *bm) {
    int paramLen = 20;
    
	/* Overall performance measures */
	snprintf(bm->econindxNAME[fleetnum_id], paramLen, "%s", "FleetSize");
	snprintf(bm->econindxNAME[fleet_switch_id], paramLen, "%s", "GearSwitching");
	snprintf(bm->econindxNAME[avgboatsze_id], paramLen, "%s", "AvgBoatSize");
	snprintf(bm->econindxNAME[overallrent_id], paramLen, "%s", "TotalRent");
	snprintf(bm->econindxNAME[GrossVal_id], paramLen, "%s", "GrossValue");
	snprintf(bm->econindxNAME[GrossTax_id], paramLen, "%s", "GrossTax");
	snprintf(bm->econindxNAME[GrossDV_id], paramLen, "%s", "GrossDV");
	snprintf(bm->econindxNAME[SubfleetCash_id], paramLen, "%s", "CashPerSubfleet");
	snprintf(bm->econindxNAME[LeaseVal_id], paramLen, "%s", "LeaseValue");
	snprintf(bm->econindxNAME[InvestReturn_id], paramLen, "%s", "InvestReturn");
	snprintf(bm->econindxNAME[RevPerT_id], paramLen, "%s", "RevenuePerTonne");
	snprintf(bm->econindxNAME[TradeExtent_id], paramLen, "%s", "TradeExtent");
	snprintf(bm->econindxNAME[RevPerEffort_id], paramLen, "%s", "RevenuePerEffort");

	/* Subfleet parameters and performance measures */
	snprintf(bm->SUBFLEETeconindxNAME[crew_size_id], paramLen, "%s", "CrewSize");
	snprintf(bm->SUBFLEETeconindxNAME[capital_cost_id], paramLen, "%s", "CapCost");
	snprintf(bm->SUBFLEETeconindxNAME[max_trip_length_id], paramLen, "%s", "TripLength");
	snprintf(bm->SUBFLEETeconindxNAME[var_cost_id], paramLen, "%s", "VarCost");
	snprintf(bm->SUBFLEETeconindxNAME[boat_size_id], paramLen, "%s", "BoatSize");
	snprintf(bm->SUBFLEETeconindxNAME[home_port_id], paramLen, "%s", "HomePort");
	snprintf(bm->SUBFLEETeconindxNAME[nboat_id], paramLen, "%s", "FleetSize");
	snprintf(bm->SUBFLEETeconindxNAME[flag_econind_id], paramLen, "%s", "EconInd");
	snprintf(bm->SUBFLEETeconindxNAME[cpue_ind_id], paramLen, "%s", "CPUEInd");
	snprintf(bm->SUBFLEETeconindxNAME[yield_ind_id], paramLen, "%s", "YieldInd");
	snprintf(bm->SUBFLEETeconindxNAME[size_ind_id], paramLen, "%s", "SizeInd");
	snprintf(bm->SUBFLEETeconindxNAME[comp_ind_id], paramLen, "%s", "CompInd");
	snprintf(bm->SUBFLEETeconindxNAME[zone_ind_id], paramLen, "%s", "ZoneInd");
	snprintf(bm->SUBFLEETeconindxNAME[gear_ind_id], paramLen, "%s", "GearInd");
	snprintf(bm->SUBFLEETeconindxNAME[mgmt_cost_ind_id], paramLen, "%s", "MgmtCost");
	snprintf(bm->SUBFLEETeconindxNAME[res_cost_ind_id], paramLen, "%s", "ResCost");
	snprintf(bm->SUBFLEETeconindxNAME[discards_ind_id], paramLen, "%s", "DiscrdInd");
	snprintf(bm->SUBFLEETeconindxNAME[gvp_ind_id], paramLen, "%s", "GVPInd");
	snprintf(bm->SUBFLEETeconindxNAME[tax_ind_id], paramLen, "%s", "TaxInd");
	snprintf(bm->SUBFLEETeconindxNAME[dv_ind_id], paramLen, "%s", "DVInd");
	snprintf(bm->SUBFLEETeconindxNAME[AnnualExpectPI_id], paramLen, "%s", "AnnualExpectedProfitInd");
	snprintf(bm->SUBFLEETeconindxNAME[rev_land_ind_id], paramLen, "%s", "RevLandInd");
	snprintf(bm->SUBFLEETeconindxNAME[rev_effort_ind_id], paramLen, "%s", "RevEffortInd");
	snprintf(bm->SUBFLEETeconindxNAME[revenue_ind_id], paramLen, "%s", "RevenueInd");
	snprintf(bm->SUBFLEETeconindxNAME[minrev_ind_id], paramLen, "%s", "MinRevInd");
	snprintf(bm->SUBFLEETeconindxNAME[cost_ind_id], paramLen, "%s", "CostInd");
	snprintf(bm->SUBFLEETeconindxNAME[boat_cash_ind_id], paramLen, "%s", "BoatCashInd");
	snprintf(bm->SUBFLEETeconindxNAME[invest_return_ind_id], paramLen, "%s", "InvestReturn");
	snprintf(bm->SUBFLEETeconindxNAME[cap_util_ind_id], paramLen, "%s", "CapUtil");
	snprintf(bm->SUBFLEETeconindxNAME[leased_Q_val_ind_id], paramLen, "%s", "QuotaLeaseVal");
	snprintf(bm->SUBFLEETeconindxNAME[maxleased_Q_val_ind_id], paramLen, "%s", "MaxQuotaLeaseVal");
	snprintf(bm->SUBFLEETeconindxNAME[sale_Q_val_ind_id], paramLen, "%s", "QuotaSaleVal");
	snprintf(bm->SUBFLEETeconindxNAME[Q_trade_ind_id], paramLen, "%s", "QuotaTradeInd");
	snprintf(bm->SUBFLEETeconindxNAME[cost_ton_ind_id], paramLen, "%s", "CostPTon");
	snprintf(bm->SUBFLEETeconindxNAME[flag_indtype_id], paramLen, "%s", "FlagIndType");
	snprintf(bm->SUBFLEETeconindxNAME[Expectedeffort_id], paramLen, "%s", "ExpectEffort");
	snprintf(bm->SUBFLEETeconindxNAME[marg_rent_id], paramLen, "%s", "MargRent");
	snprintf(bm->SUBFLEETeconindxNAME[totPlanEffort_id], paramLen, "%s", "PlanEffort");
	snprintf(bm->SUBFLEETeconindxNAME[totPlanCatch_id], paramLen, "%s", "PlanCatch");
	snprintf(bm->SUBFLEETeconindxNAME[AnnualCatch_id], paramLen, "%s", "AnnCatch");
	snprintf(bm->SUBFLEETeconindxNAME[CurrentCatch_id], paramLen, "%s", "CurrCatch");
	snprintf(bm->SUBFLEETeconindxNAME[FishableLength_id], paramLen, "%s", "FishableLength");
	snprintf(bm->SUBFLEETeconindxNAME[effortDiscount_id], paramLen, "%s", "effortDiscount");
	snprintf(bm->SUBFLEETeconindxNAME[bycatchDiscount_id], paramLen, "%s", "bycatchDiscount");
	snprintf(bm->SUBFLEETeconindxNAME[CurrentEffort_id], paramLen, "%s", "CurrEffort");
	snprintf(bm->SUBFLEETeconindxNAME[AnnualEffort_id], paramLen, "%s", "AnnEffort");
	snprintf(bm->SUBFLEETeconindxNAME[AnnualUtility_id], paramLen, "%s", "AnnUtil");
	snprintf(bm->SUBFLEETeconindxNAME[ExpectCPUE_id], paramLen, "%s", "ExpectCPUE");
	snprintf(bm->SUBFLEETeconindxNAME[totcatch_saleprice_id], paramLen, "%s", "totSalePrice");
	snprintf(bm->SUBFLEETeconindxNAME[totQuota_id], paramLen, "%s", "totQuota");
	snprintf(bm->SUBFLEETeconindxNAME[ExpectedCatch_id], paramLen, "%s", "ExpectCatch");
	snprintf(bm->SUBFLEETeconindxNAME[OpUtility_id], paramLen, "%s", "OpUtil");
	snprintf(bm->SUBFLEETeconindxNAME[crewshare_id], paramLen, "%s", "CrewShare");
	snprintf(bm->SUBFLEETeconindxNAME[OldQuota_id], paramLen, "%s", "OldQuota");
	snprintf(bm->SUBFLEETeconindxNAME[flexweight_id], paramLen, "%s", "FlexWgt");
	snprintf(bm->SUBFLEETeconindxNAME[resale_id], paramLen, "%s", "ResaleVal");
	snprintf(bm->SUBFLEETeconindxNAME[switch_cost_id], paramLen, "%s", "SwitchCost");
	snprintf(bm->SUBFLEETeconindxNAME[fuel_cost_id], paramLen, "%s", "FuelCost");
	snprintf(bm->SUBFLEETeconindxNAME[gear_cost_id], paramLen, "%s", "GearCost");
	snprintf(bm->SUBFLEETeconindxNAME[unload_cost_id], paramLen, "%s", "UnloadCost");
	snprintf(bm->SUBFLEETeconindxNAME[fixed_cost_id], paramLen, "%s", "FixedCost");
	snprintf(bm->SUBFLEETeconindxNAME[buyout_id], paramLen, "%s", "BuyoutVal");
	snprintf(bm->SUBFLEETeconindxNAME[newboat_id], paramLen, "%s", "NewBoats");
	snprintf(bm->SUBFLEETeconindxNAME[switchboat_id], paramLen, "%s", "Switching");
	snprintf(bm->SUBFLEETeconindxNAME[newboat_cost_id], paramLen, "%s", "NewBoatCost");
	snprintf(bm->SUBFLEETeconindxNAME[lostboat_id], paramLen, "%s", "LostBoats");
	snprintf(bm->SUBFLEETeconindxNAME[down_time_id], paramLen, "%s", "minDownTime");
	snprintf(bm->SUBFLEETeconindxNAME[tot_cash_id], paramLen, "%s", "YearlyIncome");
	snprintf(bm->SUBFLEETeconindxNAME[tied_up_id], paramLen, "%s", "NumTiedUp");
	snprintf(bm->SUBFLEETeconindxNAME[mth_tiedup_id], paramLen, "%s", "MonthsTiedUp");

	return;
}

/**
 *
 *	\brief Port Index names set
 *
 *	This sets up names of port performance indices
 */
void Init_Port_Index_Names(MSEBoxModel *bm) {
    int paramLen = 20;
    
	snprintf(bm->portindxNAME[portwgt_id], paramLen, "%s", "PortActivity");
	snprintf(bm->portindxNAME[portpop_id], paramLen, "%s", "PortEffectivePop");

	return;
}


/**
 * \brief Read in the friendship network cvs file.
 *
 * This is a csv file of size ( K_num_fisheries * K_num_subfleets) * ( K_num_fisheries * K_num_subfleets)
 *
 * Each row represents a friendship network for a  fleet within a fishery with other fleets.
 *
 *
 */
int Read_FriendShip_Network(MSEBoxModel *bm, char *filePath){

	FILE *inputFP;
	int		buflen = 2000;
	char	buf[2000], seps[] = ",";
	int index, count, fisheryIndex, rowIndex;
	char *indexStr, *varStr;
	double value;

	int numExpectedTokens = (bm->K_num_fisheries * bm->K_max_num_subfleet);
	int **columnIndexValues = Util_Alloc_Init_2D_Int(2, numExpectedTokens, 0);


	if( (inputFP=Open_Input_File(bm->inputFolder, filePath, "r")) == NULL )
		quit("Read_FriendShip_Network: Can't open %s%s\n",bm->inputFolder,filePath);

	/* This is a wee bit hard coded for this file structure due to time but would be
	* easy to generalise if required later.
	*
	* So we know this file structure is ( K_num_fisheries * K_num_subfleets) * ( K_num_fisheries * K_num_subfleets)
	*/

	fseek ( inputFP, 0L, 0 );

	/* Get the first line */
	fgets ( buf, buflen, inputFP );

	/* First we need to check that the number of tokens is correct */
	varStr = strtok ( buf, seps );
	count = 0;
	while((varStr = strtok ( NULL, seps )) != NULL){

		/* get rid of a \n in case there is one */
		if(strchr(varStr, '\n') != NULL)
			*(strchr(varStr, '\n')) = '\0';
		/* Parse the values */
		/* grab the fleet index off the end */
		indexStr = varStr + strlen(varStr) - 1;

		index = atoi(indexStr);
		*(indexStr) = '\0';

		/* get the index of the fishery */
		fisheryIndex = Util_Get_Fishery_Index(bm, varStr);
		if(fisheryIndex == -1)
			quit("ERROR: The friendship network input file %s contains an unrecognised fishery name %s\n\n", filePath, varStr);

		columnIndexValues[count][0] = fisheryIndex;
		columnIndexValues[count][1] = index;
		count++;
	}

	if(count != numExpectedTokens){
		quit("ERROR: The friendship network input file %s does not have the correct number of columns %d it has %d columns\n\n", filePath, numExpectedTokens, count);
	}


	while ( fgets ( buf, buflen, inputFP ) != NULL )
	{
		if(strstr(buf, "FisheryGroup") != NULL)
			continue;

		//printf("buf = %s\n", buf);
		varStr = strtok ( buf, seps );

		/* Parse the values */
		indexStr = varStr + strlen(varStr) - 1;
		//printf("indexStr = %s\n", indexStr);
		index = atoi(indexStr);
		*(indexStr) = '\0';
		fisheryIndex = Util_Get_Fishery_Index(bm, varStr);
		if(fisheryIndex == -1)
			quit("ERROR: The friendship network input file %s contains an unrecognised fishery name %s\n\n", filePath, varStr);

		/* Now parse all of the values */
		rowIndex = 0;
		while((varStr = strtok ( NULL, seps )) != NULL){
			if(strcmp(varStr, "-") == 0){
				rowIndex++;
				continue;	/* the friendship value will default to 1.0 */
			}
			value = atof(varStr);

			if(value <= 1.0){
				warn("Read_FriendShip_Network. You have tried to set the friendship index between %s:%d and %s:%d to less than the min value 1.0. It will default back to 1.0\n",
						FisheryArray[fisheryIndex].fisheryCode, index, FisheryArray[columnIndexValues[rowIndex][0]].fisheryCode, columnIndexValues[rowIndex][1]);
				value = 1.0;
			}
			friendship_netwk[fisheryIndex][columnIndexValues[rowIndex][0]][index][columnIndexValues[rowIndex][1]] = value;
			//fprintf(bm->logFile, "friendship_netwk[%s][%s][%d][%d] = %e\n", bm->fisheryNAME[fisheryIndex], bm->fisheryNAME[columnIndexValues[rowIndex][0]], index, columnIndexValues[rowIndex][1], friendship_netwk[fisheryIndex][columnIndexValues[rowIndex][0]][index][columnIndexValues[rowIndex][1]]);

			rowIndex++;
		}
		if(rowIndex != numExpectedTokens){
			quit("ERROR: The friendship network input file %s does not have the correct number of columns %d it has %d columns\n\n", filePath, numExpectedTokens, rowIndex);
		}
	}
	i_free2d(columnIndexValues);
	fclose(inputFP);
	return 0;

}
