/**
 \file
 \brief C file for running the calibration tools
 \ingroup atEcology

 File:		atcalibtools.c
 Created:	5/10/2005
 Author:		Beth Fulton,   CSIRO Marine Laboratories,  2005
 Created from previous atbiology.c file
 Purpose:	C file for running the calibration tools for use in Atlantis
 model calibration.
 Note:

 Revisions:

 26-05-2008 Bec Gorton
 Changed all references from bm->VERTind to the tracer arrays associated with
 each functional group. Changed testBalance to expect the boxModel as
 a parameter. This allows the code to be changed to loop over the functional groups
 and use the FunctGroupArray[fgIndex].totNTracers[0] tracer index instead of the
 old tracer indices that were defined separately for each vertebrate group
 (e.g. Planktiv_S_Fish_N_i)

 20-01-2009 Bec Gorton
 Changed the Check_Gape code to use bm_>K_num_to_sp instead of bm->K_num_vert_sp.
 The DietCheck values are now written to a separate output file instead of the log file.

 02-02-2009 Beth Fulton
 Change the variable called small to small_num.

 11-05-2009 Bec Gorton
 Moved the num_migrate_id parameter from the cohort species parameters to the normal species
 parameters.
 */
 
/*  Open library routines */
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>
#include "atecology.h"
#include <atHarvestLib.h>

/**
 *	\brief Routine to test that vertebrate abundance totals haven't
 *	increased erroneously
 *
 *  This routine totals up the numbers in each cohort for each vertebrate group.
 *
 *  Inputs:
 *   MSEBoxModel:	MSEBoxModel data structure
 *
 *  Outputs:
 *   localtotden:	tot numbers for each vertebrate group in each cohort
 *
 */
void Ecology_Test_Fish_Total(MSEBoxModel *bm, double ***valtr, double **landtr, int calltype, char *spotcall, FILE *llogfp) {
	int ij, k, n, sp, den;
	double diff, buffsize, totold, totnew;
	double **localtotden = Util_Alloc_Init_2D_Double(bm->K_num_max_cohort * bm->K_num_max_genetypes, bm->K_num_tot_sp, 0.0);


	/* Initialise the localtotden matrix */
	for (sp = 0; sp < bm->K_num_tot_sp; sp++) {
		if (FunctGroupArray[sp].isVertebrate == TRUE) {
			for (n = 0; n < FunctGroupArray[sp].numCohortsXnumGenes; n++) {
				localtotden[sp][n] = 0.0;
			}
		}
	}

	/* Determine tot numbers per vertebrate group */
	if (!calltype) {
		/* Use box model stored values */
		for (ij = 0; ij < bm->nbox; ij++) {
			if (bm->boxes[ij].type == LAND) {
				for (sp = 0; sp < bm->K_num_tot_sp; sp++) {
					if (FunctGroupArray[sp].isVertebrate == TRUE && FunctGroupArray[sp].habitatCoeffs[LAND_BASED]) {
						for (n = 0; n < FunctGroupArray[sp].numCohortsXnumGenes; n++) {
							den = FunctGroupArray[sp].NumsTracers[n];
							localtotden[sp][n] += bm->boxes[ij].tr[0][den];
						}
					}
				}
			}else if (bm->boxes[ij].type != BOUNDARY) {
				for (k = 0; k < bm->boxes[ij].nz; k++) {
					for (sp = 0; sp < bm->K_num_tot_sp; sp++) {
						if (FunctGroupArray[sp].isVertebrate == TRUE) {
							for (n = 0; n < FunctGroupArray[sp].numCohortsXnumGenes; n++) {
								den = FunctGroupArray[sp].NumsTracers[n];
								localtotden[sp][n] += bm->boxes[ij].tr[k][den];
							}
						}
					}
				}
			}
		}
	} else {
		/* Use new updated values */
		for (ij = 0; ij < bm->nbox; ij++) {
			if (bm->boxes[ij].type == LAND) {
				for (sp = 0; sp < bm->K_num_tot_sp; sp++) {
					if (FunctGroupArray[sp].isVertebrate == TRUE && FunctGroupArray[sp].habitatCoeffs[LAND_BASED] > 0) {
						for (n = 0; n < FunctGroupArray[sp].numCohortsXnumGenes; n++) {
							den = FunctGroupArray[sp].NumsTracers[n];

							if (ij <= bm->current_box) {
								/* piggy back on normal tracers */
								localtotden[sp][n] += valtr[ij][0][den];

								/*
								 if(bm->debug && ((bm->debug > debug_stock) && bm->current_box == bm->checkbox && sp == bm->which_check && (bm->dayt > bm->checkstart))){
								 fprintf(llogfp,"%s in %d-%d contribs %.16f to localtotden %e\n",
								 FunctGroupArray[sp].groupCode, ij, k, valtr[ij][k][den], localtotden[sp][n]);
								 }
								 */

							} else {
								localtotden[sp][n] += bm->boxes[ij].tr[0][den];

								/*
								 if(bm->debug && ((bm->debug > debug_stock) && bm->current_box == bm->checkbox && sp == bm->which_check && (bm->dayt > bm->checkstart))){
								 fprintf(llogfp,"%s in %d-%d contribs %.16f to localtotden %e\n",
								 FunctGroupArray[sp].groupCode, ij, k, bm->boxes[ij].tr[k][den], localtotden[sp][n]);
								 }
								 */
							}
						}
					}
				}
			}else if (bm->boxes[ij].type != BOUNDARY) {
				for (k = 0; k < bm->boxes[ij].nz; k++) {
					for (sp = 0; sp < bm->K_num_tot_sp; sp++) {
						if (FunctGroupArray[sp].isVertebrate == TRUE) {
							for (n = 0; n < FunctGroupArray[sp].numCohortsXnumGenes; n++) {
								den = FunctGroupArray[sp].NumsTracers[n];

								if (ij <= bm->current_box) {
									localtotden[sp][n] += valtr[ij][k][den];

									/*
									 if(bm->debug && ((bm->debug > debug_stock) && bm->current_box == bm->checkbox && sp == bm->which_check && (bm->dayt > bm->checkstart))){
									 fprintf(llogfp,"%s in %d-%d contribs %.16f to localtotden %e\n",
									 FunctGroupArray[sp].groupCode, ij, k, valtr[ij][k][den], localtotden[sp][n]);
									 }
									 */

								} else {
									localtotden[sp][n] += bm->boxes[ij].tr[k][den];

									/*
									 if(bm->debug && ((bm->debug > debug_stock) && bm->current_box == bm->checkbox && sp == bm->which_check && (bm->dayt > bm->checkstart))){
									 fprintf(llogfp,"%s in %d-%d contribs %.16f to localtotden %e\n",
									 FunctGroupArray[sp].groupCode, ij, k, bm->boxes[ij].tr[k][den], localtotden[sp][n]);
									 }
									 */
								}
							}
						}
					}
				}
			}
		}
	}

	/* Output results */
	totold = 0;
	totnew = 0;
	for (sp = 0; sp < bm->K_num_tot_sp; sp++) {

		if (FunctGroupArray[sp].isVertebrate == TRUE) {
			totold = 0; // Not sure if this is the correct place for these to get set.
			totnew = 0;
			for (n = 0; n < FunctGroupArray[sp].numCohortsXnumGenes; n++) {
				totold += VERTabund_check[sp][n];
				totnew += localtotden[sp][n];

				/* Update the abundances */
				VERTabund_check[sp][n] = localtotden[sp][n];
			}

			diff = totnew - totold;
			buffsize = small_num * totold;
			if ((diff > buffsize) && (diff > small_num))
				fprintf(llogfp, "time: %e, %s %s gained in size (%e vs now %e so gained %e individuals)\n", bm->dayt, spotcall, FunctGroupArray[sp].groupCode, totold, totnew, diff);

		}
	}

	free2d(localtotden);

	return;
}

/**
 *	\brief Calibration tool used to check prey availability given gape limitation
 *	(what prey are actually being caught)
 *
 *	This routine checks gape vs prey for each cohort for each vertebrate group.
 *
 *  Inputs:
 *   MSEBoxModel:	MSEBoxModel data structure
 *   FILE:      log file
 *
 */
void Check_Gape(MSEBoxModel *bm, FILE *llogfp) {
	char keywithin[BMSLEN*2];
	char keyless[BMSLEN*2];
	char keymore[BMSLEN*2];
	char key[BMSLEN*2];

	int ij, k, n, sn, rn, den, sp, prey, preyage, chrtstage, preychrt, flag_sp, maxstock_id, i, indx, qidi;
    int qid, recdate, spmigrate;
    //int pid;
    double sp_abund, sum_prop, totrec, totrecold, sp_AgeClassSize, relSN, relRN, relDEN;

	if (!bm->flaggape && !bm->flagchecksize && !bm->fishout && !bm->coming_to_end)
		return;
    
    //if (verbose > 1)
    //    printf("Check_Gape\n");
    
	/* Initialise VERTinfo array */
	for (sp = 0; sp < bm->K_num_tot_sp; sp++) {
		if (FunctGroupArray[sp].isVertebrate == TRUE) {
			for (n = 0; n < FunctGroupArray[sp].numCohortsXnumGenes; n++) {
				for (ij = 0; ij < 3; ij++) {
					VERTinfo[sp][n][ij] = 0.0;
				}
			}
		}
	}
    
    /* Set up biomasses to check against */
	for (sp = 0; sp < bm->K_num_tot_sp; sp++) {
		if (FunctGroupArray[sp].isVertebrate == TRUE) {
			flag_sp = (int) (FunctGroupArray[sp].speciesParams[flag_id]);
			if (flag_sp) {

				for (n = 0; n < FunctGroupArray[sp].numCohortsXnumGenes; n++) {

					sn = FunctGroupArray[sp].structNTracers[n];
					rn = FunctGroupArray[sp].resNTracers[n];
					den = FunctGroupArray[sp].NumsTracers[n];

					/* Initialise min and max per species */
					sizeMinMax[min_SN_id] = MAXDOUBLE; // Minimum SN
					sizeMinMax[max_SN_id] = 0; // Maximum SN
					sizeMinMax[min_RN_id] = MAXDOUBLE; // Minimum RN
					sizeMinMax[max_RN_id] = 0; // Maximum RN

					/* Initalise abundance */
					sp_abund = 0;

					for (ij = 0; ij < bm->nbox; ij++) {
						if (bm->boxes[ij].type != BOUNDARY) {
							for (k = 0; k < bm->boxes[ij].nz; k++) {
								sp_abund += bm->boxes[ij].tr[k][den];

								/**/
								if (bm->debug && (((bm->debug == debug_age) || (bm->debug == debug_migrate)) && (sp == bm->which_check) && (bm->dayt
										> bm->checkstart))) {
									fprintf(llogfp, "Time: %e, %s-%d box%d-%d added abund: %e\n", bm->dayt, FunctGroupArray[sp].groupCode, n, ij, k,
											bm->boxes[ij].tr[k][den]);
								}
								/**/
								if (bm->boxes[ij].tr[k][den] > bm->min_pool) {
									/* Check minimum SN */
									if (bm->boxes[ij].tr[k][sn] < sizeMinMax[min_SN_id])
										sizeMinMax[min_SN_id] = bm->boxes[ij].tr[k][sn];
									/* Check maximum SN */
									if (bm->boxes[ij].tr[k][sn] > sizeMinMax[max_SN_id])
										sizeMinMax[max_SN_id] = bm->boxes[ij].tr[k][sn];
									/* Check minimum RN */
									if (bm->boxes[ij].tr[k][rn] < sizeMinMax[min_RN_id])
										sizeMinMax[min_RN_id] = bm->boxes[ij].tr[k][rn];
									/* Check maximum RN */
									if (bm->boxes[ij].tr[k][rn] > sizeMinMax[max_RN_id])
										sizeMinMax[max_RN_id] = bm->boxes[ij].tr[k][rn];
								}
							}
						}
					}

					/* Add in migrating individuals to abundance estimate */
					for (qid = 0; qid < MIGRATION[sp].num_in_queue; qid++) {
						sp_abund += MIGRATION[sp].DEN[n][qid];

						/**/
						if (bm->debug && (((bm->debug == debug_age) || (bm->debug == debug_migrate)) && (sp == bm->which_check) && (bm->dayt > bm->checkstart))) {
							fprintf(llogfp, "Time: %e, %s-%d mig-%d added abund: %e\n", bm->dayt, FunctGroupArray[sp].groupCode, n, qid,
									MIGRATION[sp].DEN[n][qid]);
						}
						/**/

						if (MIGRATION[sp].DEN[n][qid] > bm->min_pool) {
							/* Check minimum SN */
							if (MIGRATION[sp].SN[n][qid] < sizeMinMax[min_SN_id])
								sizeMinMax[min_SN_id] = MIGRATION[sp].SN[n][qid];
							/* Check maximum SN */
							if (MIGRATION[sp].SN[n][qid] > sizeMinMax[max_SN_id])
								sizeMinMax[max_SN_id] = MIGRATION[sp].SN[n][qid];
							/* Check minimum RN */
							if (MIGRATION[sp].RN[n][qid] < sizeMinMax[min_RN_id])
								sizeMinMax[min_RN_id] = MIGRATION[sp].RN[n][qid];
							/* Check maximum RN */
							if (MIGRATION[sp].RN[n][qid] > sizeMinMax[max_RN_id])
								sizeMinMax[max_RN_id] = MIGRATION[sp].RN[n][qid];
						}
					}

					/* If no individuals left check size again with relaxed density constraints */
					if (((sp_abund < bm->min_pool) || (sizeMinMax[min_SN_id] > (MAXDOUBLE / 4.0)))) {
						for (ij = 0; ij < bm->nbox; ij++) {
							if (bm->boxes[ij].type != BOUNDARY) {
								for (k = 0; k < bm->boxes[ij].nz; k++) {
									//sp_abund += bm->boxes[ij].tr[k][den];

									/* Check minimum SN */
									if (bm->boxes[ij].tr[k][sn] < sizeMinMax[min_SN_id])
										sizeMinMax[min_SN_id] = bm->boxes[ij].tr[k][sn];
									/* Check maximum SN */
									if (bm->boxes[ij].tr[k][sn] > sizeMinMax[max_SN_id])
										sizeMinMax[max_SN_id] = bm->boxes[ij].tr[k][sn];
									/* Check minimum RN */
									if (bm->boxes[ij].tr[k][rn] < sizeMinMax[min_RN_id])
										sizeMinMax[min_RN_id] = bm->boxes[ij].tr[k][rn];
									/* Check maximum RN */
									if (bm->boxes[ij].tr[k][rn] > sizeMinMax[max_RN_id])
										sizeMinMax[max_RN_id] = bm->boxes[ij].tr[k][rn];
								}
							}
						}
					}

					/* Determine average size for each species */
					VERTinfo[sp][n][SN_id] = (sizeMinMax[min_SN_id] + sizeMinMax[max_SN_id]) / 2.0;
					VERTinfo[sp][n][RN_id] = (sizeMinMax[min_RN_id] + sizeMinMax[max_RN_id]) / 2.0;

					//TODO: Check this - will never get here is the group is not active.
					if (!FunctGroupArray[sp].speciesParams[flag_id])
						VERTinfo[sp][n][DEN_id] = initVERTinfo[sp][n][DEN_id] + small_num;
					else
						VERTinfo[sp][n][DEN_id] = sp_abund;
				}
			}
		}
	}
    
    if (bm->flagchecksize || bm->coming_to_end) {
        
        for (sp = 0; sp < bm->K_num_tot_sp; sp++) {
            
            if (FunctGroupArray[sp].isVertebrate == TRUE) {
				flag_sp = (int) (FunctGroupArray[sp].speciesParams[flag_id]);
				sp_AgeClassSize = (double)(FunctGroupArray[sp].ageClassSize);
				maxstock_id = FunctGroupArray[sp].numStocks;
				spmigrate = FunctGroupArray[sp].num_migrate;

				if (flag_sp) {
					/* Recruitment check */
					totrec = 0;
					totrecold = 0;
					qid = EMBRYO[sp].next_larvae;
                    
                    if(EMBRYO[sp].SpawnRecruitOverlap)
                        qid++;
                    qidi = qid - 1;
                    if(qidi < 0)
                        qidi = 0;

                    //printf("Last step Check_Gape W-%d %s qid: %d (SpawnRecruitOverlap: %d, next_larvae: %d)\n", sp, FunctGroupArray[sp].groupCode, qid, EMBRYO[sp].SpawnRecruitOverlap, EMBRYO[sp].next_larvae);
                    
                    if (EMBRYO[sp].next_larvae > EMBRYO[sp].num_in_spawn_queue)
                        quit("EMBRYO[%s].next_larvae (%d) > EMBRYO[].num_in_spawn_queue %d)\n", FunctGroupArray[sp].groupCode, EMBRYO[sp].next_larvae, EMBRYO[sp].num_in_spawn_queue);

                    
                    for (n = 0; n < FunctGroupArray[sp].numCohortsXnumGenes; n++) {
                        
                        if (n < FunctGroupArray[sp].numGeneTypes) {
							for (k = 0; k < FunctGroupArray[sp].numStocks; k++) {
								totrec += EMBRYO[sp].Larvae[k][n][qid];
								totrecold += EMBRYO[sp].Larvae[k][n][qidi];

                                //printf("Last step Check_Gape W-%d-%d %s k: %d qid: %d qidi: %d\n", sp, n, FunctGroupArray[sp].groupCode, k, qid, qidi);
                                
								if(EMBRYO[sp].Larvae[k][n][qid] || EMBRYO[sp].Larvae[k][n][expect_id])
									fprintf(llogfp,"%s stock %d, gene: %d, VERTembryo: %e, VERTembryoOld: %e\n",
											FunctGroupArray[sp].groupCode, k, n, EMBRYO[sp].Larvae[k][n][qid], EMBRYO[sp].Larvae[k][n][qidi]);
                                
                                
							}
						} else if ((n == FunctGroupArray[sp].numGeneTypes) && (totrec || totrecold)){
                            indx = EMBRYO[sp].next_spawn - 1;
                            if(indx < 0) {
                                indx = 0;
                            }
                            
                            //printf("Last step Check_Gape W-%d-%d-b %s indx: %d\n", sp, n, FunctGroupArray[sp].groupCode, indx);
                            
							fprintf(llogfp, "%s has %e (%e + %e old) larvae in watercolumn - spawndate: %d, settledate: %d\n",
									FunctGroupArray[sp].groupCode, totrec + totrecold, totrec, totrecold,
									EMBRYO[sp].Spawn_Now[n][indx], EMBRYO[sp].StartDay[n][EMBRYO[sp].next_recruit]);
                            
						}
                        
                        /* Migration check */
						//pid = FunctGroupArray[sp].cohort_stage[n];
                        //printf("Last step Check_Gape W-%d-%d-b %s pid: %d\n", sp, n, FunctGroupArray[sp].groupCode, pid);
                        
                        for (qid = MIGRATION[sp].num_in_queue_done; qid < MIGRATION[sp].num_in_queue; qid++) {                            
                            //printf("Last step Check_Gape W-%d-%d-d %s qid: %d\n", sp, n, FunctGroupArray[sp].groupCode, qid);
                            
                            if (spmigrate && (bm->dayt >= MIGRATION[sp].Leave_Now[qid]) && (bm->dayt < MIGRATION[sp].Return_Now[qid] + MIGRATION[sp].Return_Period[qid])){
                                relSN = MIGRATION[sp].SN[n][qid] / (initVERTinfo[sp][n][SN_id] + small_num);
                                relRN = MIGRATION[sp].RN[n][qid] / (initVERTinfo[sp][n][RN_id] + small_num);
                                relDEN = MIGRATION[sp].DEN[n][qid] / (initVERTinfo[sp][n][DEN_id] + small_num);

                                //printf("Last step Check_Gape W-%d qid: %d %s, cohort %d - migration %d underway (relsn: %e relrn: %e relden: %e as sn: %e rn: %e den: %e and return %d)\n", sp, qid, FunctGroupArray[sp].groupCode, n, qid, relSN, relRN, relDEN, MIGRATION[sp].SN[n][qid], MIGRATION[sp].RN[n][qid], MIGRATION[sp].DEN[n][qid], MIGRATION[sp].Return_Now[qid]);

                                fprintf(llogfp, "%s, cohort %d - migration %d underway (relsn: %e relrn: %e relden: %e as sn: %e rn: %e den: %e and return %d)\n", FunctGroupArray[sp].groupCode, n, qid, relSN, relRN, relDEN, MIGRATION[sp].SN[n][qid], MIGRATION[sp].RN[n][qid], MIGRATION[sp].DEN[n][qid], MIGRATION[sp].Return_Now[qid]);
                            }
						}

                        /* Size structure */
						fprintf(llogfp, "%s-%d sn/init_sn %e, rn/init_rn %e, den/init_den %e (SN: %e, RN: %e, DEN: %e vs initSN: %e, initRN: %e, initDEN: %e)\n",
								FunctGroupArray[sp].groupCode, n, VERTinfo[sp][n][SN_id] / (initVERTinfo[sp][n][SN_id] + small_num),
								VERTinfo[sp][n][RN_id] / (initVERTinfo[sp][n][RN_id] + small_num), VERTinfo[sp][n][DEN_id] / (initVERTinfo[sp][n][DEN_id]
										+ small_num), VERTinfo[sp][n][SN_id], VERTinfo[sp][n][RN_id], VERTinfo[sp][n][DEN_id], initVERTinfo[sp][n][SN_id], initVERTinfo[sp][n][RN_id], initVERTinfo[sp][n][DEN_id]);
					}

                    /* Age distribution summary */
					if (bm->flagagecheck || bm->coming_to_end) {
						qid = EMBRYO[sp].next_larvae;

                        //printf("Last step Check_Gape agedistrib W-%d %s qid: %d\n", sp, FunctGroupArray[sp].groupCode, qid);
                        
                        // TODO: Fx this for new queues
						for (n = 0; n < FunctGroupArray[sp].numCohortsXnumGenes; n++) {
							recdate = EMBRYO[sp].StartDay[n][qid];
                            
							fprintf(llogfp, "d: %d, AgeDate: %d, SpawnDate: %d, RecruitDate:%d ageclass_size: %e\n", bm->TofY,
									EMBRYO[sp].Age_Now[n][expect_id], EMBRYO[sp].Spawn_Now[n][expect_id], recdate, sp_AgeClassSize);

							for (i = 0; i < maxstock_id; i++) {
								sum_prop = 0;
								fprintf(llogfp, "%s-%d stock: %d", FunctGroupArray[sp].groupCode, n, i);
								for (k = 0; k < sp_AgeClassSize; k++) {
									sum_prop += bm->tempPopRatio[i][sp][n][k];
									fprintf(llogfp, " nc-%d: %e", k, bm->tempPopRatio[i][sp][n][k]);
								}
								fprintf(llogfp, " sumprop: %e\n", sum_prop);
                                //printf(" %s sumprop: %e\n", FunctGroupArray[sp].groupCode, sum_prop);
							}
						}
					}
				}
			}
		}
	}
    
    if (!bm->flaggape && !bm->fishout && !bm->coming_to_end)
		return;

    //printf("Last step Check_Gape Y \n");
    
	/* Check gape match */
	if (bm->flaggape) {
		for (sp = 0; sp < bm->K_num_tot_sp; sp++) {
			if (FunctGroupArray[sp].isVertebrate == TRUE) {

				flag_sp = (int) (FunctGroupArray[sp].speciesParams[flag_id]);
				if (flag_sp) {
					for (n = 0; n < FunctGroupArray[sp].numCohortsXnumGenes; n++) { // Predator age groups
						chrtstage = FunctGroupArray[sp].cohort_stage[n];

						/* Print output - predator name */
                        fprintf(llogfp, "Time: %e Predator: %s cohort: %d, KLP: %e, KUP: %e\n", bm->dayt, FunctGroupArray[sp].groupCode, n,
								FunctGroupArray[sp].speciesParams[KLP_id], FunctGroupArray[sp].speciesParams[KUP_id]);

						//TODO: Check to see if i can change this to loop over the sp then cohorts instead.
						for (preychrt = 0; preychrt < bm->K_num_max_cohort * bm->K_num_max_genetypes; preychrt++) { // Prey age groups
							//snprintf(keywithin, sizeof(keywithin), " ");
							//snprintf(keymore, sizeof(keymore), " ");
							//snprintf(keyless, sizeof(keyless)," ");
							strcpy(keywithin, " ");
							strcpy(keymore, " ");
							strcpy(keyless, " ");
							for (prey = 0; prey < bm->K_num_tot_sp; prey++) { // Prey
								if (FunctGroupArray[prey].isVertebrate == TRUE && preychrt < FunctGroupArray[prey].numCohortsXnumGenes) {
									preyage = FunctGroupArray[prey].cohort_stage[preychrt];

									/* Check to see if groups are active and if link is non-zero */
									if (FunctGroupArray[sp].speciesParams[flag_id] && FunctGroupArray[prey].speciesParams[flag_id]
											&& (bm->pSPVERTeat[sp][prey][chrtstage][preyage] > 0.0)) {

										/* Check to see if fit in gape */
										if (VERTinfo[prey][preychrt][SN_id] >= FunctGroupArray[sp].speciesParams[KLP_id] * VERTinfo[sp][n][SN_id]) {
											/* Above lower gape */
											if (VERTinfo[prey][preychrt][SN_id] <= FunctGroupArray[sp].speciesParams[KUP_id] * VERTinfo[sp][n][SN_id]) {
												/* Less than upper gape */

												//snprintf(key, sizeof(key), "%s %s-%d", keywithin, FunctGroupArray[prey].groupCode, preychrt);
												//snprintf(keywithin,sizeof(keywithin), "%s", key);
												snprintf(key, sizeof(key), " %s-%d", FunctGroupArray[prey].groupCode, preychrt);
												strcat(keywithin, key);
											} else {
												/* Above upper gape */
												if (VERTinfo[prey][preychrt][SN_id] > VERTinfo[sp][n][SN_id]) {
													//snprintf(key, sizeof(key), "%s %s-%d (>SN %e)", keymore, FunctGroupArray[prey].groupCode, preychrt,
													//		VERTinfo[prey][preychrt][SN_id] / VERTinfo[sp][n][SN_id]);
													//snprintf(keymore, sizeof(keymore), "%s", key);

													snprintf(key, sizeof(key), " %s-%d (>SN %e)", FunctGroupArray[prey].groupCode, preychrt,
															VERTinfo[prey][preychrt][SN_id] / VERTinfo[sp][n][SN_id]);

													strcat(keymore, key);

												} else {
													//snprintf(key, sizeof(key), "%s %s-%d (%e)", keymore, FunctGroupArray[prey].groupCode, preychrt,
													//		VERTinfo[prey][preychrt][SN_id] / VERTinfo[sp][n][SN_id]);
													//snprintf(keymore, sizeof(keymore), "%s", key);

													snprintf(key, sizeof(key), " %s-%d (%e)", FunctGroupArray[prey].groupCode, preychrt,
															VERTinfo[prey][preychrt][SN_id] / VERTinfo[sp][n][SN_id]);
													strcat(keymore, key);

												}
											}
										} else {
											/* Less than lower gape */
											//snprintf(key, sizeof(key), "%s %s-%d (%e)", keyless, FunctGroupArray[prey].groupCode, preychrt, VERTinfo[prey][preychrt][SN_id]
											//		/ VERTinfo[sp][n][SN_id]);
											//snprintf(keyless, sizeof(keyless), "%s", key);

											snprintf(key, sizeof(key), " %s-%d (%e)", FunctGroupArray[prey].groupCode, preychrt, VERTinfo[prey][preychrt][SN_id]
													/ VERTinfo[sp][n][SN_id]);
											strcat(keyless, key);

										}
									}
								}
							}

							/* Print output - predator name */
							if (strcmp(keywithin, " ") == 0)
								fprintf(llogfp, "Time: %e Predator: %s-%d Preychrt: %d, Nothing available\n", bm->dayt, FunctGroupArray[sp].groupCode, n,preychrt);
							else
								fprintf(llogfp, "Time: %e Predator: %s-%d Preychrt: %d, Can eat %s\n", bm->dayt, FunctGroupArray[sp].groupCode, n,preychrt, keywithin);
							if (strcmp(keyless, " ") == 0) {
								/* Nothing to do - don't print out as no info content */
							} else
								fprintf(llogfp, "Time: %e Predator: %s-%d Preychrt: %d, Too small %s\n", bm->dayt, FunctGroupArray[sp].groupCode, n,preychrt, keyless);
							if (strcmp(keymore, " ") == 0) {
								/* Nothing to do - don't print out as no info content */
							} else
								fprintf(llogfp, "Time: %e Predator: %s-%d Preychrt: %d, Too large %s\n", bm->dayt, FunctGroupArray[sp].groupCode, n,preychrt, keymore);

						}
					}
				}
			}
		}
	}
    
    //printf("Last step Check_Gape Z \n");
    
    if (!bm->fishout && !bm->coming_to_end)
		return;

	/* Report cumulative total fisheries for the year to date */
	if (bm->fishout)
		Harvest_Report_Fisheries_Stats(bm, llogfp);

    //printf("Last step Check_Gape\n");
    
	return;
}

void Ecology_Check_VertAbund(MSEBoxModel *bm, double ***valtr, FILE *llogfp, int instance) {
    int ij, k, n, den;
    //double totden = 0.0;
    int sp = bm->which_check;
    
    //sp = 23;

    for (n = 0; n < FunctGroupArray[sp].numCohortsXnumGenes; n++) {
        totdenCheck[n][0] = 0.0;
        totdenCheck[n][1] = 0.0;
        for (ij = 0; ij < bm->nbox; ij++) {
            if (bm->boxes[ij].type != BOUNDARY) {
                for (k = 0; k < bm->boxes[ij].nz; k++) {
                    den = FunctGroupArray[sp].NumsTracers[n];
                    totdenCheck[n][0] += bm->boxes[ij].tr[k][den];
                    totdenCheck[n][1] += valtr[ij][k][den];
                }
            }
        }
        
        fprintf(llogfp, "Time: %e %d %s-%d instance %d totdenCheck: %e newvalCheck: %e\n",
                bm->dayt, bm->current_box, FunctGroupArray[sp].groupCode, n, instance, totdenCheck[n][0], totdenCheck[n][1]);

    }
        
    return;
}

