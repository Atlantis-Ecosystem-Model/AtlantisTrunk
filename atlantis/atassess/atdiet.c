/**
 \file
 \brief Routines to calculate diet matrices
 \ingroup atassess

 File:           atdiet.c

 Created:        8/8/2004

 Author:         Beth Fulton

 Purpose:        Routines to calculate diet matrices

 Arguments:      bm    - Pointer to box model.

 Returns:        void

 Revisions:      8/8/2004 Created from southeast diet.c

 21/9/2004 Renamed FSL, FSO, FSF, FST and FSG to
 generic names so apply to other models/generic case.
 FSL -> FDC  (Ling -> Demersal_DC_Fish)
 FSO -> FDO  (Orange_Roughy -> Demersal_O_Fish)
 FSF -> FDB  (Flathead -> Demersal_B_Fish)
 FST -> FVO  (Bluefin_Tuna -> Pisciv_O_Fish)
 FSG -> SHB  (Gummy_Shark -> Shark_B)

 22/9/2004 Replaced numbers in switch statements with id
 guild numbers.

 1/10/2004 Removed switch case for KLP_XX, KUP_XX (replaced with read
 from bm->SP_prms array and moved YY_sc parameters to core boxmodel
 (this shift was done so	assessment and ecology libraries can both
 access this parameters with duplication)

 7/10/2004 Added diet preference read-in for bioloigcal groups
 (PWN, BFF, BML, FBP, FDE, FDF, FVB, SHR, SSK, REP, SP, WDG)

 22/10/2004 Replaced K_num_tot with K_num_tot_assess

 17/11/2004 Replaced use of read in preferences with call to
 operating model feeding parameter arrays

 12/09/2008 Bec Gorton
 Moved the species eat array data SPeat into the functional group array for
 each group.

 09-03-2010 Bec Gorton
 Resized the dietpref array.
 Removed the food array - this is not longer needed.
 *********************************************************************/

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <math.h>
#include <sjwlib.h>
#include "atsample.h"

int Doolittle_LU_Decomposition(double *A, int n);
int Doolittle_LU_Solve(double *LU, double B[], double x[], int n);
static void Multinomial(MSEBoxModel *bm, int n, double **p, int **count, int m, int numStages);
//static void Multinomial(MSEBoxModel *bm, int n, double *p, int **count, int m, int numStages);


static void Sample_Diet(MSEBoxModel *bm, double ****targetArray) {
	int pred, predStage, prey, preyStage;
	int maxPredStage;
	int K_num_stomaches_small = (int) (ceil(K_num_stomaches / 100.0));

	for (pred = 0; pred < bm->K_num_tot_sp; pred++) {
		if ((int)FunctGroupArray[pred].speciesParams[flag_id] && FunctGroupArray[pred].isAssessed == TRUE && FunctGroupArray[pred].isPredator == TRUE) {

			/* Note: Working with operating model-type age classes so use agemat */
			if (FunctGroupArray[pred].groupAgeType == AGE_STRUCTURED || FunctGroupArray[pred].groupAgeType == AGE_STRUCTURED_BIOMASS) {
				maxPredStage = 2;
			} else {
				maxPredStage = 1;
			}

			for (predStage = 0; predStage < maxPredStage; predStage++) {
				Util_Init_2D_Int(stomachs, bm->K_num_tot_sp, 2, 0);

				Multinomial(bm, K_num_stomaches, availfood[pred][predStage], stomachs, bm->K_num_tot_sp, K_num_invageclass);
				Multinomial(bm, K_num_stomaches_small, altavailfood[pred][predStage], stomachs, bm->K_num_tot_sp, K_num_invageclass);

				for (prey = 0; prey < bm->K_num_tot_sp; prey++) {
					if (FunctGroupArray[prey].isAssessed == TRUE) {

						for (preyStage = 0; preyStage < K_num_invageclass; preyStage++) {
							targetArray[pred][predStage][prey][preyStage] += stomachs[prey][preyStage];
						}
					}
				}
			}
		}
	}
}
/**
 * \brief This routine calculates the diet matrices
 */
void Year_Of_Stomach(MSEBoxModel *bm, FILE *ofp) {
	int bb, b;
	double d;

	if (verbose)
		fprintf(stderr, "Calculating diet matrix\n");

	/* Intialise diets */
	Util_Init_4D_Double(inshorediet, bm->K_num_tot_sp, 2, bm->K_num_tot_sp, 2, 0.0);
	Util_Init_4D_Double(offshorediet, bm->K_num_tot_sp, 2, bm->K_num_tot_sp, 2, 0.0);
	Util_Init_4D_Double(totareadiet, bm->K_num_tot_sp, 2, bm->K_num_tot_sp, 2, 0.0);

	for (bb = 0; bb < bm->nsbox; bb++) {
		b = bm->nsboxes[bb];
		d = phys[wc_depth_id][b];

		Util_Init_2D_Int(stomachs, bm->K_num_tot_sp, 2, 0);

		/* This will set up the food available to each predator.
		 * The values per predator are then normalised to sum to 1.0
		 * So its basically the proportion that each prey group makes up the
		 * diet of each predator.
		 */
		Setup_Avail_Food(bm, b, ofp);

		if (d < bm->cdz) {
			/* Inshore diets */
			Sample_Diet(bm, inshorediet);
		} else {
			/* Offshore diets */
			Sample_Diet(bm, offshorediet);
		}

		/* Overall diet composition */
		Calculate_Overall_Diet_Composition(bm, totareadiet, inshorediet, offshorediet);
	}

	/* Normalise diet composition */
	Normalise_AvailFood(bm, inshorediet);
	Normalise_AvailFood(bm, offshorediet);
	Normalise_AvailFood(bm, totareadiet);

	Calculate_Trophic_Levels(bm, TL, inshorediet, offshorediet, totareadiet, ofp);

	return;

}
/**
 * \brief Set the invertebrate availfood array values for the given predator and stage.
 * This is called for consumer groups.
 */
static void Set_Species_Avail_Food(MSEBoxModel *bm, int b, int pred, int predStage) {
	int prey;
	for (prey = 0; prey < bm->K_num_tot_sp; prey++) {
		if (FunctGroupArray[prey].isVertebrate == FALSE && FunctGroupArray[prey].isAssessed == TRUE) {
			if (FunctGroupArray[prey].isDetritus == TRUE) {
				availfood[pred][predStage][prey][WC] = biolbiom[prey][b][WC] * dietpref[pred][predStage][prey][WC];
				availfood[pred][predStage][prey][SED] = biolbiom[prey][b][SED] * dietpref[pred][predStage][prey][SED];

			} else {
				availfood[pred][predStage][prey][WC] = biolbiom[prey][b][WC] * dietpref[pred][predStage][prey][WC];
			}
		}
	}
}

/**
 * \brief Normalise the given foodavail array.
 */
void Normalise_AvailFood(MSEBoxModel *bm, double ****availfoodArray) {
	int pred, predStage, prey, preyStage;
	double sum, sumCheck;

	/* Normalise the array so each column sums to one */
	for (pred = 0; pred < bm->K_num_tot_sp; pred++) {

		/* Only bother with this if the group is a consumer else the avail food values should all be 0 anyway so no
		 * point normalising
		 */
		if (FunctGroupArray[pred].isAssessed == TRUE && FunctGroupArray[pred].isPredator == TRUE) {
			for (predStage = 0; predStage < K_num_invageclass; predStage++) {
				sum = 0;
				for (prey = 0; prey < bm->K_num_tot_sp; prey++) {
					if (FunctGroupArray[prey].isAssessed == TRUE) {

						/* Add the possible combinations to allow for adult and juv vertebrate predators as well
						 * adult and juv prey and sediment and water column invertebrate prey
						 *
						 * If the values are not relevant then they will be 0 anyway.
						 */

						for (preyStage = 0; preyStage < K_num_invageclass; preyStage++) {
							sum += availfoodArray[pred][predStage][prey][preyStage];
						}
					}
				}

				/* Now normalise the values */
				sumCheck = 0;
				for (prey = 0; prey < bm->K_num_tot_sp; prey++) {
					if (FunctGroupArray[prey].isAssessed == TRUE) {

						for (preyStage = 0; preyStage < K_num_invageclass; preyStage++) {
							availfoodArray[pred][predStage][prey][preyStage] *= 1.0 / (sum + TINY);
							sumCheck = sumCheck + availfoodArray[pred][predStage][prey][preyStage];

//							if(availfoodArray[pred][predStage][prey][preyStage] > 0)
//								fprintf(bm->logFile, "inshorediet[%s][%d[%s][%d] = %.20e\n", FunctGroupArray[pred].groupCode, predStage,
//									FunctGroupArray[prey].groupCode, preyStage, availfoodArray[pred][predStage][prey][preyStage]);




						}
					}
				}
			}
		}
	}
}

/**
 * \brief This sets up matrices of available food
 * availfood = probabiltity of encounter based on prey biomass in the ecosystem
 * altavailfood = probability of encountering rare prey is increased
 */
void Setup_Avail_Food(MSEBoxModel *bm, int b, FILE *ofp) {
	int pred, chrt, prey = 0;
	int predChrt;
	double contrib, KLP_sp, KUP_sp;
	int predStage, preyStage;//, i, j, k, l;
	double thisarea = bm->boxes[b].area;

	if (verbose > 2)
		fprintf(bm->logFile, "Setup_Avail_Food in box %d\n", b);
	/* Initialise the food arrays */
	Util_Init_4D_Double(availfood, bm->K_num_tot_sp, 2, bm->K_num_tot_sp, 2, 0.0);
	Util_Init_4D_Double(altavailfood, bm->K_num_tot_sp, 2, bm->K_num_tot_sp, 2, 0.0);

	/* Could possibly just allocate this here instead of it being a global */
	Util_Init_2D_Double(contribvert, 2, bm->K_num_tot_sp, 0.0);

	/* Determine vertebrate biomass */
	for (pred = 0; pred < bm->K_num_tot_sp; pred++) {
		if (FunctGroupArray[pred].isVertebrate == TRUE) {
			for (chrt = 0; chrt < FunctGroupArray[pred].numCohortsXnumGenes; chrt++) {
				/* Note: Working with operating model-type age classes so use agemat */
				predStage = FunctGroupArray[pred].cohort_stage[chrt];
				contribvert[pred][predStage] += biolVERTinfo[bstocknums_id][pred][chrt][b];
			}
		}
	}

	/* Set up usual case where encounter matches relative biomass in the ecosystem */
	/* Those predator-prey interactions where gape size is ignored */

	for (pred = 0; pred < bm->K_num_tot_sp; pred++) {
		if (FunctGroupArray[pred].isAssessed == TRUE){
			if(FunctGroupArray[pred].isPredator == TRUE) {

				/* Don't bother with groups that aren't consumers - the array is set to 0 by default above */

				/* If the consumer is a vertebrate then we need to set the diet preference for adult and juv, else just set the first slot */
				/* This will just set the values for the invertebrates - vertebrates are handled below */
				if (FunctGroupArray[pred].isVertebrate == TRUE) {
					Set_Species_Avail_Food(bm, b, pred, juv_id);
					Set_Species_Avail_Food(bm, b, pred, adult_id);
				} else {
					Set_Species_Avail_Food(bm, b, pred, 0);
				}
			}
		}
	}

	/* Those predator-prey interactions where gape size is taken into account
	 - vertebrates as prey */

	for (prey = 0; prey < bm->K_num_tot_sp; prey++) {
		if (FunctGroupArray[prey].isVertebrate == TRUE) {
			for (chrt = 0; chrt < FunctGroupArray[prey].numCohortsXnumGenes; chrt++) {

//				double preyBiomass = (biolVERTinfo[bstruct_id][prey][chrt][b] + biolVERTinfo[bres_id][prey][chrt][b])
//										* biolVERTinfo[bstocknums_id][prey][chrt][b];
				/* Note: Working with operating model-type age classes so use agemat */
				preyStage = FunctGroupArray[prey].cohort_stage[chrt];

				/* Now look at the various predators */
				for (pred = 0; pred < bm->K_num_tot_sp; pred++) {
					if (FunctGroupArray[pred].isVertebrate == FALSE) {

						if(FunctGroupArray[pred].isPredator == TRUE){
							if(FunctGroupArray[pred].groupAgeType == AGE_STRUCTURED_BIOMASS)
								predStage = adult_id;
							else
								predStage = juv_id;

							if (biolVERTinfo[bstruct_id][prey][chrt][b] >= (FunctGroupArray[pred].sn[predStage]
									* FunctGroupArray[pred].speciesParams[KLP_id])) {
								if (biolVERTinfo[bstruct_id][prey][chrt][b] <= (FunctGroupArray[pred].sn[predStage]
										* FunctGroupArray[pred].speciesParams[KUP_id])) {

									availfood[pred][0][prey][preyStage] += (biolVERTinfo[bstruct_id][prey][chrt][b] + biolVERTinfo[bres_id][prey][chrt][b]) * biolVERTinfo[bstocknums_id][prey][chrt][b] * dietpref[pred][0][prey][preyStage] / (thisarea + TINY);

								}
							}
						}
					} else {
						/* Vertebrate food - skip planktivores and start with first piscivores */
						if (!FunctGroupArray[pred].speciesParams[flagplankfish_id]) {

							/* Set the gap limitations to the default and re-set
							 where necessary using the swtich statement */
							KLP_sp = FunctGroupArray[pred].speciesParams[KLP_id];
							KUP_sp = FunctGroupArray[pred].speciesParams[KUP_id];

							for (predChrt = 0; predChrt < FunctGroupArray[pred].numCohortsXnumGenes; predChrt++) {
								/* Note: Working with operating model-type age classes so use agemat */
								predStage = FunctGroupArray[pred].cohort_stage[predChrt];
								contrib = biolVERTinfo[bstocknums_id][pred][predChrt][b] / (contribvert[pred][predStage] + TINY);

								/* Determine biomass (m-2) of available food */
								if (biolVERTinfo[bstruct_id][prey][chrt][b] >= biolVERTinfo[bstruct_id][pred][predChrt][b] * KLP_sp) {
									if (biolVERTinfo[bstruct_id][prey][chrt][b] <= biolVERTinfo[bstruct_id][pred][predChrt][b] * KUP_sp) {

										availfood[pred][predStage][prey][preyStage] += (biolVERTinfo[bstruct_id][prey][chrt][b]
										        + biolVERTinfo[bres_id][prey][chrt][b])	* biolVERTinfo[bstocknums_id][prey][chrt][b]
												* dietpref[pred][predStage][prey][preyStage] * contrib / (thisarea + TINY);

									}
								}
							}
						}
					}
				}
			}
		}
	}

	/* Normalise the array so each column sums to one */
	Normalise_AvailFood(bm, availfood);

	/* Set up case where rare dietary components are encountered more often */
	for (pred = 0; pred < bm->K_num_tot_sp; pred++) {

		if (FunctGroupArray[pred].isAssessed == TRUE && FunctGroupArray[pred].isPredator == TRUE) {
			for (prey = 0; prey < bm->K_num_tot_sp; prey++) {
				if (FunctGroupArray[prey].isAssessed == TRUE) {
					for (predStage = 0; predStage < K_num_invageclass; predStage++) {
						for (preyStage = 0; preyStage < K_num_invageclass; preyStage++) {
							if ((availfood[pred][predStage][prey][preyStage] < 0.01) && (availfood[pred][predStage][prey][preyStage] > 0.0))
								altavailfood[pred][predStage][prey][preyStage] = 0.1;
							else
								altavailfood[pred][predStage][prey][preyStage] = availfood[pred][predStage][prey][preyStage];
						}
					}
				}
			}
		}
	}

	/* Now normalise the altavailfood array */
	Normalise_AvailFood(bm, altavailfood);

	/**
	if (verbose > 1) {
		for (pred = 0; pred < bm->K_num_tot_sp; pred++) {
			if (FunctGroupArray[pred].isAssessed == TRUE) {
				for (predStage = 0; predStage < K_num_invageclass; predStage++){
					for (prey = 0; prey < bm->K_num_tot_sp; prey++) {
						if (FunctGroupArray[prey].isAssessed == TRUE) {

							for (preyStage = 0; preyStage < K_num_invageclass; preyStage++){
								if( availfood[pred][predStage][prey][preyStage] > 0 || altavailfood[pred][predStage][prey][preyStage] > 0)
								fprintf(ofp, "availfood[%s][%d][%s][%d]: %.20e, altavailfood[%d][%d][%d][%d]: %e\n", FunctGroupArray[pred].groupCode, predStage,
										FunctGroupArray[prey].groupCode, preyStage, availfood[pred][predStage][prey][preyStage], pred, predStage, prey,
										preyStage, altavailfood[pred][predStage][prey][preyStage]);
							}
						}
					}
				}
			}
		}
	}
	**/

	return;

}
/**
 * \brief generating a sample from a multinomial
 */
//static void Multinomial(MSEBoxModel *bm, int n, double **p, int **count, int m, int numStages) {
//
//	int i, bin, j;//, sp;
//	double u, lower, upper, swap;
//	int maxNumStages;
//	int flag = 0;
//
//	Util_Init_2D(lower, bm->K_num_tot_sp, 2, 0.0);
//	Util_Init_2D(upper, bm->K_num_tot_sp, 2, 0.0);
//
//
//	if (FunctGroupArray[bin].isAssessed == TRUE) {
//		if (FunctGroupArray[bin].isVertebrate == TRUE || FunctGroupArray[bin].isDetritus == TRUE)
//			maxNumStages = numStages;
//		else
//			maxNumStages = 1;
//
//		for (j = 0; j < maxNumStages; j++) {
//			if(p[bin][j] > 0.0){
//				/* Locate subinterval, of length p[bin], that contains the variate
//				 and increment the number in that bin */
//				lower[bin][j] = swap;
//				upper[bin][j] += p[bin][j];
//				swap = upper[bin][j];
//			}
//		}
//	}
//	/* Generate n uniform variates in the interval [0,1] */
//	for (i = 0; i < n; i++) {
//
//		u = drandom(0.0, 1.0);
//
//		//lower = upper = 0.0;
//
//		//for (sp = 0; sp < m; sp++) {
//		bin = m - 1;
//		flag = 0;
//		while(flag < 1 && bin >= 0){
//		//for (bin = m - 1,flag = 0; bin >= 0, flag < 1; bin--) {
//
////#ifdef ORIGINAL_GROUP_ORDER
////			bin = TLorder2[sp];
////#else
////			bin = sp;
////#endif
//
//			//bin = TLorder3[sp];
//
//			if (FunctGroupArray[bin].isAssessed == TRUE) {
//				if (FunctGroupArray[bin].isVertebrate == TRUE || FunctGroupArray[bin].isDetritus == TRUE)
//					maxNumStages = numStages;
//				else
//					maxNumStages = 1;
//
//				for (j = 0; j < maxNumStages; j++) {
//					if(p[bin][j] > 0.0){
//						/* Locate subinterval, of length p[bin], that contains the variate
//						 and increment the number in that bin */
//						lower = upper;
//						upper += p[bin][j];
//
//						if ((lower <= u) && (u < upper)) {
//							count[bin][j]++;
//							flag = 1;
//							break;
//						}
//					}
//				}
//			}
//			bin--;
//		}
//	}
//
//	return;
//}

//static void Multinomial(MSEBoxModel *bm, int n, double **p, int **count, int m, int numStages) {
//
//	int i, bin, j;//, sp;
//	//double u,/* lower, upper, */swap;
//	int maxNumStages, index;
//	double swap, u;
//
//	Util_Init_1D_Double(lower, bm->K_num_tot_sp* 2, 0.0);
//	Util_Init_1D_Double(upper, bm->K_num_tot_sp* 2, 0.0);
//
//	index = 0;
//	swap = 0.0;
//	for(bin = m - 1; bin >= 0; bin--){
//		if (FunctGroupArray[bin].isAssessed == TRUE) {
//			if (FunctGroupArray[bin].isVertebrate == TRUE || FunctGroupArray[bin].isDetritus == TRUE)
//				maxNumStages = numStages;
//			else
//				maxNumStages = 1;
//
//			for (j = 0; j < maxNumStages; j++) {
//				if(p[bin][j] > 0.0){
//					/* Locate subinterval, of length p[bin], that contains the variate
//					 and increment the number in that bin */
//					lower[index] = swap;
//					upper[index] += p[bin][j];
//					swap = upper[index];
//
//					MultiNom_binIndex[index] = bin;
//					MultiNom_cohortIndex[index] = j;
//					index++;
//
//				}
//			}
//		}
//	}
//	/* Generate n uniform variates in the interval [0,1] */
//	for (i = 0; i < n; i++) {
//
//		u = drandom(0.0, 1.0);
//
//		//lower = upper = 0.0;
//
//		//for (sp = 0; sp < m; sp++) {
//		for(bin = 0 ; bin < index; bin++ ){
//			if ((lower[bin] <= u) && (u < upper[bin])) {
//				count[MultiNom_binIndex[bin]][MultiNom_cohortIndex[bin]]++;
//				break;
//			}
//		}
//	}
//
//	return;
//}


static void Multinomial(MSEBoxModel *bm, int n, double **p, int **count, int m, int numStages) {

	int i, bin, j, checkdone = 0;
	double u, lower, upper;


	/* Generate n uniform variates in the interval [0,1] */
	for (i = 0; i < n; i++) {

		u = drandom(0.0, 1.0);

		lower = upper = 0.0;

        checkdone = 0;
		for(bin = 0 ; bin < m; bin++ ){
			for (j = 0; j < numStages; j++) {

			/* Locate subinterval, of length p[bin], that contains the variate
				and increment the number in that bin */
				lower = upper;
				upper += p[bin][j];

				if ((lower <= u) && (u < upper)){
					count[bin][j]++;
                    checkdone = 1;
					break;
				}
                if (checkdone) {
                    break;
                }
			}
		}
	}

	return;
}

//
//static void Multinomial(MSEBoxModel *bm, int n, double **p, int **count, int m, int numStages) {
//
//	int i, bin, j, sp;
//	double u, lower, upper;
//	int maxNumStages;
//	int flag = 0;
//
//	/* Generate n uniform variates in the interval [0,1] */
//	for (i = 0; i < n; i++) {
//
//		u = drandom(0.0, 1.0);
//
//		lower = 0.0;
//		upper = 0.0;
//
//		for (sp = 0; sp < m; sp++) {
//#ifdef ORIGINAL_GROUP_ORDER
//			bin = TLorder2[sp];
//#else
//			bin = sp;
//#endif
//			flag = 0;
//			if (FunctGroupArray[bin].isAssessed == TRUE) {
//				if (FunctGroupArray[bin].isVertebrate == TRUE || FunctGroupArray[bin].isDetritus == TRUE)
//					maxNumStages = numStages;
//				else
//					maxNumStages = 1;
//
//				for (j = 0; j < maxNumStages; j++) {
//					/* Locate subinterval, of length p[bin], that contains the variate
//					 and increment the number in that bin */
//					lower = upper;
//					upper += p[bin][j];
//
//					if ((lower <= u) && (u < upper)) {
//						count[bin][j]++;
//						flag = 1;
//						break;
//					}
//				}
//
//				if (flag == 1)
//					break;
//			}
//		}
//	}
//
//	return;
//}

void Calculate_Location_Trophic_Levels(MSEBoxModel *bm, double **tl, double ****diet, FILE *llogfp) {
	int pred, predStage, prey, preyStage;
	//double ans = 0;
	double *rhsmat;
	double **sumDiet;
	double **lhsmat;
	int num_to_check;
	int predMaxStage;
	double *t;

	//printf("Creating Trophic Level arrays\n");

	/* Allocate and init the local arrays */
	t = Util_Alloc_Init_1D_Double(2 * bm->K_num_tot_sp, 1.0);

	/* Start the LU decomposition by setting up Ax = b */
	rhsmat = Util_Alloc_Init_1D_Double(2 * bm->K_num_tot_sp, 1.0);
	lhsmat = Util_Alloc_Init_2D_Double(2 * bm->K_num_tot_sp, 2 * bm->K_num_tot_sp, 0.0);

	/* Really need to change this to be like the old array so we can use the solve functions without having to change them too much */
	sumDiet = Util_Alloc_Init_2D_Double(2, bm->K_num_tot_sp, 0.0);

	/** Set up matrices **/
	/* Initialise matrices */
	for (pred = 0; pred < bm->K_num_tot_sp; pred++) {
		if (FunctGroupArray[pred].isAssessed == TRUE) {
			if (FunctGroupArray[pred].isVertebrate == TRUE)
				predMaxStage = 2;
			else
				predMaxStage = 1;

			for (predStage = 0; predStage < predMaxStage; predStage++) {
				/* Initialise everything */
				tl[pred][predStage] = 1.0;

				/* Check the diet sums to 1.0 */
				sumDiet[pred][predStage] = 0.0;
				for (prey = 0; prey < bm->K_num_tot_sp; prey++) {
					if (FunctGroupArray[prey].isAssessed == TRUE) {
						for (preyStage = 0; preyStage < K_num_invageclass; preyStage++) {
							sumDiet[pred][predStage] += diet[pred][predStage][prey][preyStage];
						}
					}
				}

				/**
				if (fabs(sumDiet[pred][predStage] - 1.0) > TINY)
					fprintf(llogfp, "Warning: sumdiet (%e) for %s doesn't add to 1.0, diff = %e\n", sumDiet[pred][predStage], FunctGroupArray[pred].groupCode,
							sumDiet[pred][predStage] - 1.0);
							**/
			}
		}
	}

	for (pred = 0; pred < bm->K_num_tot_sp; pred++) {
		for (predStage = 0; predStage < FunctGroupArray[pred].numStages; predStage++) {
			lhsmat[(pred * FunctGroupArray[pred].numStages) + predStage][(pred * FunctGroupArray[pred].numStages) + predStage] = 1.0;
		}
	}

	/* Set up the matrix */
	for (pred = 0; pred < bm->K_num_tot_sp; pred++) {
		if (FunctGroupArray[pred].isAssessed == TRUE && FunctGroupArray[pred].isDetritus == FALSE) {
			for (predStage = 0; predStage < K_num_invageclass; predStage++) {
				for (prey = 0; prey < bm->K_num_tot_sp; prey++) {
					if (FunctGroupArray[prey].isAssessed == TRUE) {
						for (preyStage = 0; preyStage < K_num_invageclass; preyStage++) {
							lhsmat[(pred * 2) + predStage][(prey * 2) + preyStage] = -diet[pred][predStage][prey][preyStage];
						}
					}
				}
				lhsmat[(pred * 2) + predStage][(pred * 2) + predStage] = 1.0 - diet[pred][predStage][pred][predStage];
			}
		}
	}

	/* Find the solution */
	num_to_check = bm->K_num_tot_sp * 2;
	Doolittle_LU_Decomposition(*lhsmat, num_to_check);
	Doolittle_LU_Solve(*lhsmat, rhsmat, t, num_to_check);

	for (pred = 0; pred < bm->K_num_tot_sp; pred++) {
		if (FunctGroupArray[pred].isAssessed == TRUE) {

			for (predStage = 0; predStage < K_num_invageclass; predStage++) {
				tl[pred][predStage] = t[(pred * 2) + predStage];
			}
		}
	}

	free2d(lhsmat);
	free1d(rhsmat);
	free2d(sumDiet);
	free(t);

}

/**
 * \brief Routine to calculate trophic levels for the various groups
 * calculated as TLi = 1+sum_over_j(DCij .* TLj)
 * Note TLplant = 1, TLdetrit = 1
 *
 * Assumes coefftmat[pred][prey] and diet[pred][prey]
 */
void Calculate_Trophic_Levels(MSEBoxModel *bm, double ***tl, double ****indiet, double ****offdiet, double ****totdiet, FILE *ofp) {

	if (verbose)
		fprintf(stderr, "Calculating inshore trophic levels\n");

	/* Inshore */
	Calculate_Location_Trophic_Levels(bm, tl[0], indiet, ofp);

	if (verbose)
		fprintf(stderr, "Calculating offdiet trophic levels\n");

	/* Offshore */
	Calculate_Location_Trophic_Levels(bm, tl[1], offdiet, ofp);

	if (verbose)
		fprintf(stderr, "Calculating totdiet trophic levels\n");

	/* Total area */
	Calculate_Location_Trophic_Levels(bm, tl[2], totdiet, ofp);
	return;
}

/*************************** Model Set-up Routines ****************************/
/**
 * \brief This routine constructs the preference matrix used to calculate
 * the diet matrices (order of prey inverts_pred_id, juvenile and adult of each
 * vertebrate group and then the detritus groups. Note: for vertebrates there
 * is a diet entry for adults and another for juveniles
 */
void Set_Preference_Matrix(MSEBoxModel *bm, FILE *ofp) {
	double sum;
	int pred, predStage, prey, preyStage, allStage;
	int habitat;

	/* Initialise */
	for (pred = 0; pred < bm->K_num_tot_sp; pred++) {
		for (predStage = 0; predStage < FunctGroupArray[pred].numStages; predStage++) {
			for (prey = 0; prey < bm->K_num_tot_sp; prey++) {
				for (preyStage = 0; preyStage < FunctGroupArray[prey].numStages; preyStage++) {
					dietpref[pred][predStage][prey][preyStage] = 0.0;
				}
			}
		}
	}

	/* Consumer diets */
	for (pred = 0; pred < bm->K_num_tot_sp; pred++) {
		switch (FunctGroupArray[pred].groupAgeType) {

		case AGE_STRUCTURED:
			for (predStage = 0; predStage < FunctGroupArray[pred].numStages; predStage++) {
				for (prey = 0; prey < bm->K_num_tot_sp; prey++) {
					switch (FunctGroupArray[prey].groupAgeType) {
					case AGE_STRUCTURED: /* intentional follow through */
						//case AGE_STRUCTURED_BIOMASS:
						for (preyStage = 0; preyStage < FunctGroupArray[prey].numStages; preyStage++) {
							dietpref[pred][predStage][prey][preyStage] = bm->pSPVERTeat[pred][prey][predStage][preyStage];

						}
						break;
					case BIOMASS:
					case AGE_STRUCTURED_BIOMASS:
						allStage = 0;
						if (FunctGroupArray[prey].isDetritus == TRUE) {
							/* allow for the sediment and wc values */
							for (habitat = WC; habitat <= SED; habitat++) {
								dietpref[pred][allStage][prey][habitat] = FunctGroupArray[pred].pSPEat[allStage][prey][habitat];
							}
						} else {
							dietpref[pred][predStage][prey][WC] = FunctGroupArray[pred].pSPEat[predStage][prey][WC];
						}

						break;
					}
				}
			}
			break;

		case AGE_STRUCTURED_BIOMASS:

			for (prey = 0; prey < bm->K_num_tot_sp; prey++) {
				switch (FunctGroupArray[prey].groupAgeType) {
				case AGE_STRUCTURED: /* intentional follow through */
					for (preyStage = 0; preyStage < FunctGroupArray[prey].numStages; preyStage++) {
						/* Get the summation of the cohorts for this age structured biomass group */
						sum = 0.0;
						allStage = 0;
						for (predStage = 0; predStage < FunctGroupArray[pred].numStages; predStage++) {
							sum += FunctGroupArray[pred].pSPEat[predStage][prey][preyStage];
						}
						dietpref[pred][allStage][prey][preyStage] = sum / 2;
					}
					break;
				case BIOMASS:
				case AGE_STRUCTURED_BIOMASS:
					sum = 0.0;
					allStage = 0;
					if (FunctGroupArray[prey].isDetritus == TRUE) {
						/* allow for the sediment and wc values */
						for (habitat = WC; habitat <= SED; habitat++) {
							sum = 0.0;
							/* Need to allow for the adult and juv age_sturctured_biomass prey values */
							for (predStage = 0; predStage < FunctGroupArray[pred].numStages; predStage++) {
								sum += FunctGroupArray[pred].pSPEat[predStage][prey][habitat];
							}
							dietpref[pred][allStage][prey][habitat] = sum / 2;
						}
					} else {
						sum = 0.0;
						allStage = 0;
						for (predStage = 0; predStage < FunctGroupArray[pred].numStages; predStage++) {
							sum += FunctGroupArray[pred].pSPEat[predStage][prey][WC];
						}
						dietpref[pred][allStage][prey][allStage] = sum / 2;
					}

					break;
				}

			}

			break;
		case BIOMASS:
			/* Need to remember that detritus will not be a predator so we don't need to worry about the
			 * location of the detritus predators.
			 */
			for (prey = 0; prey < bm->K_num_tot_sp; prey++) {
				/* If this is a detritus group then we need to allow for sediment and water column values */

				switch (FunctGroupArray[prey].groupAgeType) {

				case AGE_STRUCTURED: /* intentional follow through */
				case AGE_STRUCTURED_BIOMASS:
					predStage = 0;
					dietpref[pred][predStage][prey][juv_id] = FunctGroupArray[pred].pSPEat[predStage][prey][WC];
					dietpref[pred][predStage][prey][adult_id] = FunctGroupArray[pred].pSPEat[predStage][prey][WC];
					break;
				case BIOMASS:
					predStage = 0;
					if (FunctGroupArray[prey].isDetritus == TRUE) {
						/* allow for the sediment and wc values */
						for (habitat = WC; habitat <= SED; habitat++) {
							dietpref[pred][predStage][prey][habitat] = FunctGroupArray[pred].pSPEat[predStage][prey][habitat];
						}
					} else {
						dietpref[pred][predStage][prey][0] = FunctGroupArray[pred].pSPEat[predStage][prey][WC];
					}
					break;
				}
			}
			break;

		}
	}
	return;
}
