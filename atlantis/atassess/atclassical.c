/**
 \file
 \brief Routine to calculate classical assessment models
 \ingroup atassess

 File:           atclassical.c

 Created:        10/9/2004

 Author:         Beth Fulton

 Purpose:        Routine to calculate classical assessment models

 Arguments:      bm    - Pointer to box model.

 Returns:        void

 Revisions:      12/9/2004 added basic routine structure for assessment
 model routines.

 14/9/2004 added VPA_Assessment(), bm->EffortRecord and bm->CatchRecord
 and parameters and arrays needed in model minimisation routines

 16/9/2004 completed VPA routines adding BackCalc(), SolvePlus(),
 SolveBack(), VPAFunkVal() and VPA parts of funk()

 20/9/2004 added Schafer_Production_Assessment(), ProdCalc(). Also
 added ProdFunkVal(), production model parameter
 calls, production model parts of funk(), CalcPercentile(), and linear
 regression routines LinearRegression() and RegressionFunkVal().

 16/8/2005 Put in default case for all switch statements

 27-01-2009 Bec gorton
 Moved the vertebrate clearance and growth values into the processProps structure.

 29-01-2010 Bec Gorton
 Changed the direct references to the bm->TotCumCatch array in the harvest lib to use the function Harvest_Get_TotCumCatch.

 14-05-2010 Bec Gorton
 Merged in trunk changes 1558.

 28-04-2010 Bec Gorton
 Changed the Estimate_Other_AssessPrm function to use the new Ecology_Get_Species_LMort function
 to get the linear mortality of a group - this function will calculate apply any scaling that should occur.

 10-05-2010 Bec Gorton
 Got rid of the Ecology_Get_Species_LMort - this functionality has now been added to the ParameterQ10 function.

 *********************************************************************/

#include <process.h>
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <math.h>
#include <sjwlib.h>
#include <atsample.h>
#include "atHarvestLib.h"

static void Schafer_Production_Assessment(MSEBoxModel *bm, int sp, int funkflag, int bootstrap, double prm_sp, double top_pcnt, double bot_pcnt, FILE *ofp);
static void VPA_Assessment(MSEBoxModel *bm, int sp, int funkflag, int bootstrap, double prm_sp, double top_pcnt, double bot_pcnt, FILE *ofp);
static void MSVPA_Assessment(MSEBoxModel *bm, int sp, int funkflag, int bootstrap, double prm_sp, double top_pcnt, double bot_pcnt, FILE *ofp);
static void Integrated_Assessment(MSEBoxModel *bm, int sp, int funkflag, int bootstrap, double prm_sp, double top_pcnt, double bot_pcnt, FILE *ofp);
static void Assessment_mimic(MSEBoxModel *bm, int sp, int assess_flag_sp, int bootstrap, double prm_sp, double top_pcnt, double bot_pcnt, FILE *ofp);
static void PerfectKnowSummary(MSEBoxModel *bm, int sp, double assess_flag_sp, double top_pcnt, double bot_pcnt, FILE *ofp);
static void CalculateFref(MSEBoxModel *bm, int sp, FILE *ofp, int typecall);

/* Percentile calculation */
static void Calc_Percentile(double *EstToSort, double *SortedResults, int *SortRank, int nbs, int nchrt, int YrMax, FILE *ofp);

/**
 * \brief This routine calls the appropriate classical assessment method (if there is one) for each fished species
 */
void Classical_Assessment(MSEBoxModel *bm, FILE *ofp) {
	int sp, assess_flag_sp, assess_bootstrap_sp, flag_sp;
	double mort_sp, pmodel_sp, top_pcnt, bot_pcnt;

	for (sp = 0; sp < bm->K_num_tot_sp; sp++) {
		if (FunctGroupArray[sp].isFished == TRUE) {

			/* Initialise sums of squares */
			SSmin = MAXDOUBLE;

			flag_sp = (int) (FunctGroupArray[sp].speciesParams[flag_id]);

			/* Only bother if group in model */
			if (!flag_sp)
				continue;

			assess_flag_sp = (int) (FunctGroupArray[sp].speciesParams[assess_flag_id]);
			assess_bootstrap_sp = (int) (FunctGroupArray[sp].speciesParams[assess_bootstrap_id]);
			mort_sp = FunctGroupArray[sp].speciesParams[assess_nat_mort_id];
			pmodel_sp = FunctGroupArray[sp].speciesParams[flag_prod_model_id];
			top_pcnt = FunctGroupArray[sp].speciesParams[top_pcnt_id];
			bot_pcnt = FunctGroupArray[sp].speciesParams[bot_pcnt_id];

			switch (assess_flag_sp) {
			case no_assess: /* No assessment */
				break;
			case schafer_model: /* Schafer production model used in the assessment */
				Schafer_Production_Assessment(bm, sp, assess_flag_sp, assess_bootstrap_sp, pmodel_sp, top_pcnt, bot_pcnt, ofp);
				break;
			case VPA_model: /* ADAPT VPA used in the assessment */
				VPA_Assessment(bm, sp, assess_flag_sp, assess_bootstrap_sp, mort_sp, top_pcnt, bot_pcnt, ofp);
				break;
			case MSVPA_model: /* MSVPA used in the assessment */
				MSVPA_Assessment(bm, sp, assess_flag_sp, assess_bootstrap_sp, mort_sp, top_pcnt, bot_pcnt, ofp);
				break;
			case CAB_model: /* Integrated assessment model used in the assessment */
				Integrated_Assessment(bm, sp, assess_flag_sp, assess_bootstrap_sp, mort_sp, top_pcnt, bot_pcnt, ofp);
				break;
			case CAB_mimic: /* Assessment mimic - as time saver, uses perfect knowledge with error */
				Assessment_mimic(bm, sp, assess_flag_sp, assess_bootstrap_sp, mort_sp, top_pcnt, bot_pcnt, ofp);
				break;
            case SS3_model: /* True SS3 assessment - called through tiered assessment code for now */
                if (!bm->useRBCTiers)
                    quit("You must use useRBCTiers = 1 to access this SS3 assessment option for now\n");
                break;
            case PerfectKnow: /* Perfect knowledge case */
                PerfectKnowSummary(bm, sp, assess_flag_sp, top_pcnt, bot_pcnt, ofp);
                break;
			default:
				quit("No such assess_flag defined (i.e. value must be between %d and %d at present)\n", no_assess, PerfectKnow);
				break;
			}

			if (assess_flag_sp && (assess_flag_sp < PerfectKnow))
				Estimate_Other_AssessPrm(bm, sp, ofp);
		}
	}

	return;
}

/************************** Schafer Production Model ****************************/
/**
 * \brief This is a schafer produciton model taken from the model description in Chapter 10
 * of Modelling and Quantitative Methods in Fisheries by Malcolm Haddon.
 *
 * prm_sp in the flag indicating whether assymetry assumed in the production model
 * - if prm_sp == 0, p = 1 and schaefer model used
 * - if prm_sp == 1, p is free to vary and Pella and Tomlinson model used
 */
void Schafer_Production_Assessment(MSEBoxModel *bm, int sp, int funkflag, int bootstrap, double prm_sp, double top_pcnt, double bot_pcnt, FILE *ofp) {

	int i, j, nbs, icnt, ipnt, Yr, age, nfunk, Ilow, Ndim, low_p, high_p, med_p, indxm = 0, indxt = 0, indxb = 0;
	double yr_effort;
	//, r_est, K_est, B_est, q_est, MSY_med, BMSY, FMSY, Effort_MSY, Fmax, Effort_max;

	int dataid = (int) (FunctGroupArray[sp].speciesParams[assessDataStream_id]);

	//printf("Creating Schaefer Production arrays\n");

	/* Sums of squares */
	double SS = MAXDOUBLE;
	double GRD = 1.2;
	/* Current year since assessment started */
	int YrMax = (int) floor(ROUNDGUARD + ((bm->dayt - bm->tassessstart) / 365.0));
	CTData = Util_Alloc_Init_1D_Double(YrMax + 1, 0.0);
	ITData = Util_Alloc_Init_1D_Double(YrMax + 1, 0.0);

	//fprintf(ofp,"Doing %s with YrMax: %d, ROUNDGUARD: %e, dayt: %e, tassessstart: %e, bracket: %e\n", FunctGroupArray[sp].groupCode, YrMax, ROUNDGUARD, bm->dayt, bm->tassessstart, ((bm->dt - bm->tassessstart)/365.0));

	/* Number of dimensions in data arrays to be solved */
	if ((int) (prm_sp)) {
		Ndim = est_p_id + 1;
	} else
		Ndim = est_B0_id + 1;

	/* Specifiy data type to use */
	if ((FunctGroupArray[sp].groupType == BIRD) || (FunctGroupArray[sp].groupType == MAMMAL)) {
		/* FIX - Current code only allows for survey data of birds and mammals, this may need to change if have whaling etc */
		dataid = survey_id;
	}

	/* Data is loaded in the sampling routines FisheriesRecords() and FishEstimates() */
	/* Set up base data for the optimisation
	 FIX -- At present uses commerical cpue at age, when should really use survey
	 cpue at age, but how to define survey effort so can get cpue?? */
	for (Yr = 0; Yr < YrMax + 1; Yr++) {
		CTData[Yr] = 0;
		ITData[Yr] = 0;
		yr_effort = bm->EffortRecord[Yr][sp][dataid];
		for (age = 0; age < FunctGroupArray[sp].numCohortsXnumGenes; age++) {
			CTData[Yr] += bm->CatchRecord[Yr][sp][age][dataid];
			ITData[Yr] += bm->CatchRecord[Yr][sp][age][dataid] / (yr_effort + small_num);

			fprintf(ofp, "Time: %e, Yr: %d, CTData: %e, ITData: %e, Catch-%s-%d: %e, yr_effort: %e\n", bm->dayt, Yr, CTData[Yr], ITData[Yr],
					FunctGroupArray[sp].groupCode, age, bm->CatchRecord[Yr][sp][age][dataid], yr_effort);
		}
	}

	/* Bootstrap loop - nbs set to -1 so get one assessment loop even when not
	 bootstrapping */
	for (nbs = -1; nbs < bootstrap; nbs++) {
		/* Do projections and minimise sums of squares using amoeba
		 Assumes
		 X[est_r_id] = r
		 X[est_K_id] = K
		 X[est_B0_id] = B0
		 X[est_p_id] = p

		 Set them all about the same magnitude to speed up minimisation process, but correct
		 back to correct magnitudes when used in ProdCalc()
		 */
		for (i = est_r_id; i < est_B0_id + 1; i++) {
			X[i] = 20.0;
		}

		/** Fit the model using Amoeba **/
		/* Set up tolerances and gridding */
		for (i = 0; i < Ndim; i++) {
			for (j = 0; j < Ndim; j++) {
				P[i][j] = X[j];
				if (i - 1 == j)
					P[i][j] *= GRD;
			}
		}

		for (i = 0; i < Ndim; i++) {
			for (j = 0; j < Ndim; j++) {
				X[j] = P[i][j];
			}
			Y[i] = Funk(bm, funkflag, sp, (FunctGroupArray[sp].numCohortsXnumGenes), Ndim, YrMax, prm_sp, X, ofp, NULL);
		}

		Amoeba(bm, 1, sp, bm->dayt, FunctGroupArray[sp].groupCode, P, Y, Ndim, bm->Assess_Tol, bm->Assess_Max_Int, funkflag, (FunctGroupArray[sp].numCohortsXnumGenes), YrMax,
				prm_sp, &nfunk, &Ilow, ofp, NULL);

		if (nfunk <= bm->Assess_Max_Int) {
			for (j = 0; j < Ndim; j++) {
				X[j] = P[Ilow][j];
			}
			SS = Funk(bm, funkflag, sp, (FunctGroupArray[sp].numCohortsXnumGenes), Ndim, YrMax, prm_sp, X, ofp, NULL);
		} else
			SS = MAXDOUBLE;

		if (SS < MAXDOUBLE) {
			/* Assessment ok - store Nest results as stock assessment for use in further
			 management calculations and decisions (as no age structure store and use
			 parameter etimates and results in age zero 0-3) */
			for (i = 0; i < est_msy_id; i++) {
				if (i == est_B_id) {
					for (Yr = 0; Yr < YrMax + 1; Yr++) {
						NResult[Yr][i][nbs + 1] = NEst[Yr][i];

						fprintf(ofp, "Time: %e, i: %d, nbs: %d, NEst-%d: %e\n", bm->dayt, i, nbs, Yr, NEst[Yr][i]);
					}
				} else {
					NResult[YrMax][i][nbs + 1] = NEst[YrMax][i];

					fprintf(ofp, "Time: %e, i: %d, nbs: %d, NEst-YrMax: %e\n", bm->dayt, i, nbs, NEst[YrMax][i]);
				}
			}
		} else {
			/* Assessment fails - identify this via MAXDOUBLE do can trap below */
			for (i = 0; i < est_msy_id; i++) {
				NResult[YrMax][i][nbs + 1] = MAXDOUBLE;
			}
		}

		/* Store residuals - as no age structure carried, store results in entry zero (0) */
		icnt = 0;
		for (Yr = 0; Yr < YrMax + 1; Yr++) {
			BootResu[icnt] = Resu[Yr][0];
			icnt++;
		}

		/* Set up next round of bootstrap data */
		for (Yr = 0; Yr < YrMax + 1; Yr++) {
			ITData[Yr] = 0;
			yr_effort = bm->EffortRecord[Yr][sp][dataid];
			ipnt = (int) floor(ROUNDGUARD + (icnt * drandom(0.0, 1.0) + 1.0));
			for (age = 0; age < FunctGroupArray[sp].numCohortsXnumGenes; age++) {
				ITData[Yr] += (bm->CatchRecord[Yr][sp][age][dataid] / (yr_effort + small_num)) * exp(BootResu[ipnt]);

				fprintf(ofp, "Time: %e, Yr: %d, NewITData: %e, Catch-%s-%d: %e, yr_effort: %e, exp: %e, BootResu: %e\n", bm->dayt, Yr, ITData[Yr],
						FunctGroupArray[sp].groupCode, age, bm->CatchRecord[Yr][sp][age][dataid], yr_effort, (double)exp(BootResu[ipnt]), BootResu[ipnt]);

			}
		}
	}

	/* Determine median, and percentile results for decision support */
	/* First determine which entry in each list is low, median and high percentile */
	low_p = (int) floor(ROUNDGUARD + (nbs - (bot_pcnt / 100.0) * nbs));
	med_p = (int) floor(ROUNDGUARD + (0.5 * nbs));
	high_p = (int) floor(ROUNDGUARD + (nbs - (top_pcnt / 100.0) * nbs));

	if (bootstrap > 0) {
		/* Bootstraps to chose between */
		for (i = 0; i < est_msy_id; i++) {
			if ((NResult[YrMax][i][nbs + 1] < MAXDOUBLE) && ((i == est_r_id) || (i == est_K_id) || (i == est_B_id))) {
				for (j = 0; j < bootstrap; j++) {
					ResultToSort[j] = NResult[YrMax][i][j];
				}
				Calc_Percentile(ResultToSort, ResultSorted, nbsSort, nbs, (FunctGroupArray[sp].numCohortsXnumGenes), YrMax, ofp);

				/* Save the estimates for r, K, stock biomass estimate */
				/* Get index */
				switch (i) {
				case est_r_id:
					indxm = est_med_recruit_id;
					indxt = est_top_recruit_id;
					indxb = est_bot_recruit_id;
					break;
				case est_K_id:
					indxm = est_med_prm2_id;
					indxt = est_top_prm2_id;
					indxb = est_bot_prm2_id;
					break;
				case est_B_id:
					indxm = est_med_stock_id;
					indxt = est_top_stock_id;
					indxb = est_bot_stock_id;
					break;
				default:
					quit("No such estimatible parameter index (%d) - how did it get here?\n", i);
					break;
				}

				/* Median */
				bm->NAssess[sp][indxm] = ResultSorted[med_p];

				/* Lower x percentile */
				bm->NAssess[sp][indxb] = ResultSorted[low_p];

				/* Upper x percentile */
				bm->NAssess[sp][indxt] = ResultSorted[high_p];

				fprintf(ofp, "Time %e, %s (i: %d vs est_B: %d) indxm: %d (%e), indxb: %d (%e), indxt: %d (%e)\n", bm->dayt, FunctGroupArray[sp].groupCode, i, est_B_id, indxm, ResultSorted[med_p], indxb, ResultSorted[low_p], indxt, ResultSorted[high_p]);
			}

			/* If Assessment failed don't update the values - use last valid set */
		}
	} else {
		/* Single estimate - if Assessment failed don't	update the values,
		 but use last valid set */
		/* Recruitment */
		if (NResult[YrMax][est_r_id][0] < MAXDOUBLE) {
			bm->NAssess[sp][est_med_recruit_id] = NResult[YrMax][est_r_id][0];
			bm->NAssess[sp][est_top_recruit_id] = NResult[YrMax][est_r_id][0];
			bm->NAssess[sp][est_bot_recruit_id] = NResult[YrMax][est_r_id][0];
		}
		/* Carrying capacity */
		if (NResult[YrMax][est_K_id][0] < MAXDOUBLE) {
			bm->NAssess[sp][est_med_prm2_id] = NResult[YrMax][est_K_id][0];
			bm->NAssess[sp][est_top_prm2_id] = NResult[YrMax][est_K_id][0];
			bm->NAssess[sp][est_bot_prm2_id] = NResult[YrMax][est_K_id][0];
		}
		/* Total biomass */
		if (NResult[YrMax][est_B_id][0] < MAXDOUBLE) {
			bm->NAssess[sp][est_med_stock_id] = NResult[YrMax][est_B_id][0];
			bm->NAssess[sp][est_top_stock_id] = NResult[YrMax][est_B_id][0];
			bm->NAssess[sp][est_bot_stock_id] = NResult[YrMax][est_B_id][0];
		}

		fprintf(ofp, "Time %e, %s r %e, K %e, B %e\n", bm->dayt, FunctGroupArray[sp].groupCode, bm->NAssess[sp][est_med_recruit_id],
				bm->NAssess[sp][est_med_prm2_id], bm->NAssess[sp][est_med_stock_id]);

	}

	bm->NAssess[sp][est_SS_id] = SSmin;

	/* Assessment indices of interest */
	/*
	r_est = bm->NAssess[sp][est_med_recruit_id];
	K_est = bm->NAssess[sp][est_med_prm2_id];
	B_est = bm->NAssess[sp][est_med_stock_id];
	q_est = 1; // FIX - put in actual q estimate here
	MSY_med = r_est * K_est * 0.25;
	BMSY = K_est * 0.5;
	FMSY = r_est * 0.5;
	Effort_MSY = r_est / (2.0 * q_est + small_num);
	Fmax = r_est;
	Effort_max = r_est / (q_est + small_num);
	*/

	/* FIX - Repeat for bot and top estimates */

	free1d(CTData);
	free1d(ITData);
	return;
}


/************************** Virtual Population Analysis *************************/

/*
 * \brief This is an ADAPT Virtual Population Analysis taken from a macro in an excel spreadsheet
 * supplied by Andre Punt.
 * prm_sp in this case is natural mortality (M) for use in VPA calculations
 */
void VPA_Assessment(MSEBoxModel *bm, int sp, int funkflag, int bootstrap, double prm_sp, double top_pcnt, double bot_pcnt, FILE *ofp) {
	int chrt, i, j, nbs, icnt, ipnt, Yr, age, nfunk, Ilow, low_p, high_p, med_p, indxm = 0, indxt = 0, indxb = 0, pchrt, assess_failed;
	double yr_effort, top_biom, bot_biom, med_biom;

	int dataid = (int) (FunctGroupArray[sp].speciesParams[assessDataStream_id]);

	/* Sums of squares */
	double SS = MAXDOUBLE;
	double GRD = 1.2;
	/* Number of dimensions in data arrays to be solved */
	int Ndim = FunctGroupArray[sp].numCohortsXnumGenes;
	/* Current year since assessment started */
	int YrMax = (int) floor(ROUNDGUARD + ((bm->dayt - bm->tassessstart) / 365.0));

	/* Specify data type to use */
	if ((FunctGroupArray[sp].groupType == BIRD) || (FunctGroupArray[sp].groupType == MAMMAL)) {
		/* FIX - Current code only allows for survey data of birds and mammals, this may need to change if have whaling etc */
		dataid = survey_id;
	}

	/* Data is loaded in the sampling routines FisheriesRecords() and FishEstimates() */
	/* Set up base data for the optimisation
	 FIX -- At present uses commerical cpue at age, when should really use survey
	 cpue at age, but how to define survey effort so can get cpue?? */
	for (Yr = 0; Yr < YrMax + 1; Yr++) {
		yr_effort = bm->EffortRecord[Yr][sp][dataid];
		for (age = 0; age < FunctGroupArray[sp].numCohortsXnumGenes; age++) {
			CData[Yr][age] = bm->CatchRecord[Yr][sp][age][dataid];
			IData[Yr][age] = bm->CatchRecord[Yr][sp][age][dataid] / (yr_effort + small_num);

			fprintf(ofp, "Time: %e, Yr: %d, CData: %e, IData: %e, Catch-%s-%d: %e, yr_effort: %e\n", bm->dayt, Yr, CData[Yr][age], IData[Yr][age],
					FunctGroupArray[sp].groupCode, age, bm->CatchRecord[Yr][sp][age][dataid], yr_effort);

		}
	}

	/* Bootstrap loop - nbs set to -1 so get one assessment loop even when not
	 bootstrapping */
	for (nbs = -1; nbs < bootstrap; nbs++) {
		/* Do projections and minimise sums of squares using amoeba */
		for (chrt = 0; chrt < FunctGroupArray[sp].numCohortsXnumGenes; chrt++) {
			X[chrt] = 5;
		}

		/** Fit the model using Amoeba **/
		/* Set up tolerances and gridding */
		for (i = 0; i < Ndim; i++) {
			for (j = 0; j < Ndim; j++) {
				P[i][j] = X[j];
				if (i - 1 == j)
					P[i][j] *= GRD;
			}
		}

		for (i = 0; i < Ndim; i++) {
			for (j = 0; j < Ndim; j++) {
				X[j] = P[i][j];
			}
			Y[i] = Funk(bm, funkflag, sp, (FunctGroupArray[sp].numCohortsXnumGenes), Ndim, YrMax, prm_sp, X, ofp, NULL);
		}

		Amoeba(bm, 1, sp, bm->dayt, FunctGroupArray[sp].groupCode, P, Y, Ndim, bm->Assess_Tol, bm->Assess_Max_Int, funkflag, (FunctGroupArray[sp].numCohortsXnumGenes), YrMax,
				prm_sp, &nfunk, &Ilow, ofp, NULL);

		if (nfunk <= bm->Assess_Max_Int) {
			for (j = 0; j < Ndim; j++) {
				X[j] = P[Ilow][j];
			}
			SS = Funk(bm, funkflag, sp, (FunctGroupArray[sp].numCohortsXnumGenes), Ndim, YrMax, prm_sp, X, ofp, NULL);
		} else
			SS = MAXDOUBLE;

		fprintf(ofp, "Time: %e, %s nfunk: %d, Max_Int: %d, SS: %e\n", bm->dayt, FunctGroupArray[sp].groupCode, nfunk, bm->Assess_Max_Int, SS);

		if (SS < MAXDOUBLE) {
			/* Assessment ok - store Nest results as stock assessment for use in further
			 management calculations and decisions */
			for (Yr = 0; Yr < YrMax + 1; Yr++) {
				for (age = 0; age < FunctGroupArray[sp].numCohortsXnumGenes; age++) {
					NResult[Yr][age][nbs + 1] = NEst[Yr][age];
				}
			}
		} else {
			/* Assessment fails - use MAXDOUBLE to indicate this so can trap for it
			 in percentile calculation section */
			for (age = 0; age < FunctGroupArray[sp].numCohortsXnumGenes; age++) {
				NResult[YrMax][age][nbs + 1] = MAXDOUBLE;
			}
		}

		/* Store residuals */
		icnt = 0;
		for (Yr = 0; Yr < YrMax + 1; Yr++) {
			for (age = 0; age < FunctGroupArray[sp].numCohortsXnumGenes; age++) {
				BootResu[icnt] = Resu[Yr][age];
				icnt++;
			}
		}

		/* Set up next round of bootstrap data */
		for (Yr = 0; Yr < YrMax + 1; Yr++) {
			yr_effort = bm->EffortRecord[Yr][sp][dataid];
			for (age = 0; age < FunctGroupArray[sp].numCohortsXnumGenes; age++) {
				ipnt = (int) floor(ROUNDGUARD + (icnt * drandom(0.0, 1.0) + 1.0));
				IData[Yr][age] = (bm->CatchRecord[Yr][sp][age][dataid] / (yr_effort + small_num)) * exp(BootResu[ipnt]);

				fprintf(ofp, "Time: %e, Yr: %d, NewIData: %e, Catch-%s-%d: %e, yr_effort: %e, exp: %e, BootResu: %e\n", bm->dayt, Yr, IData[Yr][age],
						FunctGroupArray[sp].groupCode, age, bm->CatchRecord[Yr][sp][age][dataid], yr_effort, (double)exp(BootResu[ipnt]), BootResu[ipnt]);

			}
		}
	}

	/* Determine median, and percentile results for decision support */
	/* First determine which entry in each list is low, median and high percentile */
	low_p = (int) floor(ROUNDGUARD + (nbs - (bot_pcnt / 100.0) * nbs));
	med_p = (int) floor(ROUNDGUARD + (0.5 * nbs));
	high_p = (int) floor(ROUNDGUARD + (nbs - (top_pcnt / 100.0) * nbs));
	pchrt = (FunctGroupArray[sp].numCohortsXnumGenes) - 1;

	if (bootstrap > 0) {
		/* Bootstraps to chose between */
		top_biom = 0;
		bot_biom = 0;
		med_biom = 0;
		assess_failed = 0;
		for (i = 0; i < FunctGroupArray[sp].numCohortsXnumGenes; i++) {

			/* FIX -- Only want latest estimates or over the entire assessment period ?*/
			if (NResult[YrMax][i][nbs + 1] < MAXDOUBLE) {
				for (j = 0; j < bootstrap; j++) {
					ResultToSort[j] = NResult[YrMax][i][j];
				}

				Calc_Percentile(ResultToSort, ResultSorted, nbsSort, nbs, (FunctGroupArray[sp].numCohortsXnumGenes), YrMax, ofp);

				/* Save the estimates for r, K, stock biomass estimate */
				if ((i == 0) || (i == pchrt)) {
					/* Get index */
					if (i == 0) { /* Recruits */
						indxm = est_med_recruit_id;
						indxt = est_top_recruit_id;
						indxb = est_bot_recruit_id;
					} else if (i == pchrt) { /* Plus Group */
						indxm = est_med_prm2_id;
						indxt = est_top_prm2_id;
						indxb = est_bot_prm2_id;
					}

					/* Median */
					bm->NAssess[sp][indxm] = ResultSorted[med_p];

					/* Lower x percentile */
					bm->NAssess[sp][indxb] = ResultSorted[low_p];

					/* Upper x percentile */
					bm->NAssess[sp][indxt] = ResultSorted[high_p];
				}

				top_biom += ResultSorted[high_p];
				med_biom += ResultSorted[med_p];
				bot_biom += ResultSorted[low_p];
			} else
				assess_failed = 1;

			/* If assessment valid save biomasses */
			if (!assess_failed) {
				/* Median */
				bm->NAssess[sp][est_med_stock_id] = med_biom;

				/* Lower x percentile */
				bm->NAssess[sp][est_bot_stock_id] = bot_biom;

				/* Upper x percentile */
				bm->NAssess[sp][est_top_stock_id] = top_biom;

			}

			/* If assessment failed don't update the values - use last valid set */
		}
	} else {
		/* Single estimate - if Assessment failed don't	update the values,
		 but use last valid set */
		/* Recruitment */
		if (NResult[YrMax][est_r_id][0] < MAXDOUBLE) {
			bm->NAssess[sp][est_med_recruit_id] = NResult[YrMax][est_r_id][0];
			bm->NAssess[sp][est_top_recruit_id] = NResult[YrMax][est_r_id][0];
			bm->NAssess[sp][est_bot_recruit_id] = NResult[YrMax][est_r_id][0];
		}
		/* Plus group */
		if (NResult[YrMax][pchrt][0] < MAXDOUBLE) {
			bm->NAssess[sp][est_med_prm2_id] = NResult[YrMax][pchrt][0];
			bm->NAssess[sp][est_top_prm2_id] = NResult[YrMax][pchrt][0];
			bm->NAssess[sp][est_bot_prm2_id] = NResult[YrMax][pchrt][0];
		}
		/* Total biomass */
		top_biom = 0;
		for (i = 0; i < FunctGroupArray[sp].numCohortsXnumGenes; i++) {
			top_biom += NResult[YrMax][i][0];
		}
		if (top_biom < MAXDOUBLE) {
			bm->NAssess[sp][est_med_stock_id] = top_biom;
			bm->NAssess[sp][est_top_stock_id] = top_biom;
			bm->NAssess[sp][est_bot_stock_id] = top_biom;
		}
	}

	fprintf(ofp, "Time %e, %s r %e, p %e, B %e\n", bm->dayt, FunctGroupArray[sp].groupCode, bm->NAssess[sp][est_med_recruit_id],
			bm->NAssess[sp][est_med_prm2_id], bm->NAssess[sp][est_med_stock_id]);

	bm->NAssess[sp][est_SS_id] = SSmin;

	return;
}


/********************** MultiSpecies Virtual Population Analysis ***************/
void MSVPA_Assessment(MSEBoxModel *bm, int sp, int funkflag, int bootstrap, double prm_sp, double top_pcnt, double bot_pcnt, FILE *ofp)
/* This is a MultiSpecies Virtual Population Analysis taken from
 */
{
	return;
}

/********************** Integrated Assessment **********************************/
/**
 * \brief  This is a standard integrated assessment model taken from Andre Punt - links to CAB by
 * spawning child process to CAB and sucking in CAB output. Note that under Linux compilations
 * a file containing _spawnlp() is needed. In that file define the _spawnlp() routine using
 * something like:
 *
 *	i = fork()
 *	if(i == 0)
 *	execlp(bm->cabspot);
 *	else
 *  wait(i);
 */
void Integrated_Assessment(MSEBoxModel *bm, int sp, int funkflag, int bootstrap, double prm_sp, double top_pcnt, double bot_pcnt, FILE *ofp)

{

	/* Write CAB input file - called cab.dat and cab.ctl */

	/* Run CAB */
	_spawnlp(_P_WAIT, bm->cabspot, NULL);

	/* Readin CAB output file - called cab.rep */

	return;
}

/********************** CAB mimic (perfect knowledge with error) ***************/
/**
 * \brief This is a time saving exercise - just uses perfect knowledge and adds lognormal error (but using sampled data)
 *
 */
void Assessment_mimic(MSEBoxModel *bm, int sp, int assess_flag_sp, int bootstrap, double prm_sp, double top_pcnt, double bot_pcnt, FILE *ofp) {
    
	int z, i;
	double med_recruit, bot_recruit, top_recruit, med_biom, bot_biom, top_biom, biom_yoy;

    if (!assess_flag_sp)
        return;

    /** Initialise estimates **/
	med_recruit = 0;
	bot_recruit = 0;
	top_recruit = 0;
	med_biom = 0;
	bot_biom = 0;
	top_biom = 0;
    
    /** Find new recruitment and biomass estimates **/
	for (z = 0; z < bm->nfzones + 1; z++) {
        
		/** Get biomass estimate **/
		med_biom += stockinfo[sbiomass_id][sp][z][sample_id];
		bot_biom += (bot_pcnt / 100.0) * stockinfo[sbiomass_id][sp][z][sample_id];
		top_biom += (top_pcnt / 100.0) * stockinfo[sbiomass_id][sp][z][sample_id];
        
		/** Get recruitment estimate **/
		/* Get Biomass of young-of-year */
		biom_yoy = 0;
		for (i = 0; i < bm->K_num_size; i++) {
			biom_yoy += individVERTinfo[ibiomass_id][sp][i][z][sample_id] * agelengthkey[i][0][sp][z][sample_id];
		}
		med_recruit += biom_yoy;
		bot_recruit += (bot_pcnt / 100.0) * biom_yoy;
		top_recruit += (top_pcnt / 100.0) * biom_yoy;
	}
    
	/** Save recruitmemt **/
	/* Median */
	bm->NAssess[sp][est_med_recruit_id] = med_recruit;
    
	/* Lower x percentile */
	bm->NAssess[sp][est_bot_recruit_id] = bot_recruit;
    
	/* Upper x percentile */
	bm->NAssess[sp][est_top_recruit_id] = top_recruit;
    
	/** Set prm2 results to zero as nothing else to estimate as mimic only */
	bm->NAssess[sp][est_med_prm2_id] = 0.0;
	bm->NAssess[sp][est_top_prm2_id] = 0.0;
	bm->NAssess[sp][est_bot_prm2_id] = 0.0;
	bm->NAssess[sp][est_SS_id] = 0.0;
    
	/** Save biomasses **/
	/* Median */
	bm->NAssess[sp][est_med_stock_id] = med_biom;
    
	/* Lower x percentile */
	bm->NAssess[sp][est_bot_stock_id] = bot_biom;
    
	/* Upper x percentile */
	bm->NAssess[sp][est_top_stock_id] = top_biom;
    

	return;
}

/********************** Perfect knowledge with error) ***************/
/**
 * \brief This is a time saving exercise - just uses perfect knowledge (pure Atlantis) and adds lognormal error
 */
void PerfectKnowSummary(MSEBoxModel *bm, int sp, double assess_flag_sp, double top_pcnt, double bot_pcnt, FILE *ofp) {
	int b, nc, k, sn, rn, den;
	double med_recruit, bot_recruit, top_recruit, med_biom, bot_biom, top_biom, biom_yoy, counter, calcF, calcM;
    int Ncohorts = FunctGroupArray[sp].numCohorts;
    int maxstock = FunctGroupArray[sp].numStocks;
    
	/** Initialise estimates **/
	med_recruit = 0;
	bot_recruit = 0;
	top_recruit = 0;
	med_biom = 0;
	bot_biom = 0;
	top_biom = 0;
	biom_yoy = 0;
    
	/** Find new recruitment and biomass estimates **/
	for (b = 0; b < bm->nbox; b++) {
        if (bm->boxes[b].type != BOUNDARY) {
            for (nc = 0; nc < Ncohorts; nc++) {
                sn = FunctGroupArray[sp].structNTracers[nc];
                rn = FunctGroupArray[sp].resNTracers[nc];
                den = FunctGroupArray[sp].NumsTracers[nc];
                for (k=0; k < bm->boxes[b].nz; k++) {
                    med_biom += (bm->boxes[b].tr[k][sn] + bm->boxes[b].tr[k][rn]) * bm->boxes[b].tr[k][den] * FunctGroupArray[sp].habitatCoeffs[WC] * bm->X_CN * mg_2_kg;
                    
                    if(!nc)
                        biom_yoy += (bm->boxes[b].tr[k][sn] + bm->boxes[b].tr[k][rn]) * bm->boxes[b].tr[k][den] * FunctGroupArray[sp].habitatCoeffs[WC] * bm->X_CN * mg_2_kg;
                }
            }
        }
        
    }
    
    /** Get biomass estimate **/
    //med_biom = med_biom;
    bot_biom += (bot_pcnt / 100.0) * med_biom;
    top_biom += (top_pcnt / 100.0) * med_biom;
    
    /** Get recruitment estimate **/
    med_recruit += biom_yoy;
	bot_recruit += (bot_pcnt / 100.0) * biom_yoy;
	top_recruit += (top_pcnt / 100.0) * biom_yoy;
    
	/** Save recruitmemt **/
	/* Median */
	bm->NAssess[sp][est_med_recruit_id] = med_recruit;
    
	/* Lower x percentile */
	bm->NAssess[sp][est_bot_recruit_id] = bot_recruit;
    
	/* Upper x percentile */
	bm->NAssess[sp][est_top_recruit_id] = top_recruit;
    
	/** Set prm2 results to zero as nothing else to estimate as mimic only */
	bm->NAssess[sp][est_med_prm2_id] = 0.0;
	bm->NAssess[sp][est_top_prm2_id] = 0.0;
	bm->NAssess[sp][est_bot_prm2_id] = 0.0;
	bm->NAssess[sp][est_SS_id] = 0.0;
    
	/** Save biomasses **/
	/* Median */
	bm->NAssess[sp][est_med_stock_id] = med_biom;
    
	/* Lower x percentile */
	bm->NAssess[sp][est_bot_stock_id] = bot_biom;
    
	/* Upper x percentile */
	bm->NAssess[sp][est_top_stock_id] = top_biom;
    
    /** Work out M and F */
    calcM = 0.0;
	calcF = 0.0;
    counter = 0.0;
    for (nc = 0; nc < Ncohorts; nc++) {
        for (k = 0; k < maxstock; k++) {
            calcM += (bm->calcTrackedMort[sp][nc][k][finalM1_id] + bm->calcTrackedMort[sp][nc][k][finalM2_id]);
            calcF += bm->calcTrackedMort[sp][nc][k][finalF_id];
            counter++;
        }
    }
    calcM /= counter;
    calcF /= counter;
    
    bm->NAssess[sp][est_M_id] = calcM;
	bm->NAssess[sp][est_Fcurr_id] = calcF;
    
    /* Calculate reference points */
    CalculateFref(bm, sp, ofp, 1);
	return;
}


/********************** Estimating other assessment parameters *****************
 BFM
 FrefA
 Fcurr
 M
 Ccurr
 Cslope
 numyr
 CPUEnow
 CPUEstart
 */
void Estimate_Other_AssessPrm(MSEBoxModel *bm, int sp, FILE *ofp) {
    double calcM, calcF, calcZ, spawn_biom1, weight, M, num_rec1, calcB1, minend_N, maxend_N, lngth,
    clear, this_N, E1_sp = 0, start_N, end_N, catch_N,
    popratio, totDead_N, calcE, calcB, calcC, step1, lngth_end, max_yrs_included, wgt, wgt_end, yrs_included;
	int chrt, z, ai, na, nc, nyr, nc_remain, minnc, min_remain, maxnc, max_remain, thisai, oldest_age,
    b, youngest_age, testai, all_done, pred, nc_end, chrt_size, end_nc, nc_remain_end, size_nc, size_nc_end, size_nc_start, stage;

    double dsp_ageClassSize = FunctGroupArray[sp].ageClassSize;
	double numsec = 86400.0; /* number of seconds in a day */
	double Balpha = bm->NAssess[sp][est_balpha_id];
	double Bbeta = bm->NAssess[sp][est_bbeta_id];
	double KWSR_sp = FunctGroupArray[sp].speciesParams[KWSR_id];
	double KWRR_sp = FunctGroupArray[sp].speciesParams[KWRR_id];
	double mL = FunctGroupArray[sp].cohortSpeciesParams[0][mL_id];
	double mQ = FunctGroupArray[sp].cohortSpeciesParams[0][mQ_id];
	int nc_fishery = 0; // dummy variable that needs to be read into Sort_Length_Weight, but never used as don't use the flag value that needs it
	int va50 = (int) (FunctGroupArray[sp].speciesParams[Age50pcntV_id]);
	int va95 = (int) (FunctGroupArray[sp].speciesParams[Age95pcntV_id]);
	int nageclass = FunctGroupArray[sp].numCohortsXnumGenes * FunctGroupArray[sp].ageClassSize;
	int nagemat = (int)(FunctGroupArray[sp].speciesParams[age_mat_id]);
	int sp_AgeClassSize = FunctGroupArray[sp].ageClassSize;
	int cmaxsize = bm->K_num_size - 1;

	//double step2 = 0;

    /*
	if (bm->debug && (bm->debug == debug_assess))
		do_debug = 1;
	else
		do_debug = 0;
    */
    
	nagemat *= sp_AgeClassSize;

	/** Natural mortality (M) and current fishing mortality (Fcurr) estimation **/

	/* start_N = numbers in first cohort available to the fishery (use age given by va95) */
	nc = va95 * FunctGroupArray[sp].numGeneTypes;
	nc_remain = 0;
	start_N = 0;
	end_N = 0;
	catch_N = 0;
	minend_N = MAXDOUBLE;
	minnc = 0;
	min_remain = 0;
	maxend_N = 0;
	maxnc = 0;
	max_remain = 0;

	/* TODO: Check with Beth about these values - don't think they are supposed to be 0.*/
	size_nc = 0;
	end_nc = 0;

	max_yrs_included = 0;
	for (z = 0; z < bm->nfzones; z++) {
		if (nc < nagemat)
			stage = 0;
		else
			stage = 1;
		b = bm->rep_box_of_zone[sp][z][stage];
		/* Determine size of first capture (equivalent to youngest) */
		wgt = biolVERTinfo[bstruct_id][sp][nc][b] + biolVERTinfo[bres_id][sp][nc][b];
		lngth = Get_Length(bm, wgt, sp);
		Sort_Length_Weight(bm, 0, 1, sizestocknums_id, z, sp, nc_fishery, lngth, 0, wgt, cmaxsize, sample_id, &size_nc_start, ofp);
		popratio = zoneVERTpopratio[sp][nc][nc_remain][z];
		start_N += individVERTinfo[istocknums_id][sp][size_nc_start][z][sample_id] * stockinfo[sstocknums_id][sp][z][sample_id] * popratio;

		/* Find oldest fish - Iterate through the size classes if top one empty */
		nc_remain_end = sp_AgeClassSize - 1;
		nc_end = (FunctGroupArray[sp].numCohortsXnumGenes) - 1;
		while ((!end_N) && (nc_end > nc)) {
			/* Determine maxsize (equivalent to oldest) */
			if (nc_end < nagemat)
				stage = 0;
			else
				stage = 1;
			b = bm->rep_box_of_zone[sp][z][stage];
			wgt_end = biolVERTinfo[bstruct_id][sp][nc_end][b] + biolVERTinfo[bres_id][sp][nc_end][b];
			lngth_end = Get_Length(bm, wgt_end, sp);
			Sort_Length_Weight(bm, 0, 1, sizestocknums_id, z, sp, nc_fishery, lngth_end, 0, wgt_end, cmaxsize, sample_id, &size_nc_end, ofp);
			popratio = zoneVERTpopratio[sp][nc_end][nc_remain_end][z];
			end_N += individVERTinfo[istocknums_id][sp][size_nc_end][z][sample_id] * stockinfo[sstocknums_id][sp][z][sample_id] * popratio;

			if (end_N && (max_yrs_included < ((nc_end - va95) * sp_AgeClassSize))) {
				max_yrs_included = (nc_end - va95) * sp_AgeClassSize;
			}
			nc_end--;

		}

		if (!end_N) {
			/* If still none then all died out within one ageclass */
			max_yrs_included = sp_AgeClassSize;
		}

		for (chrt = 0; chrt < FunctGroupArray[sp].numCohortsXnumGenes; chrt++) {
			for (ai = 0; ai < sp_AgeClassSize; ai++) {
				thisai = chrt * sp_AgeClassSize + ai;
				popratio = zoneVERTpopratio[sp][chrt][ai][z];

				wgt = biolVERTinfo[bstruct_id][sp][chrt][b] + biolVERTinfo[bres_id][sp][chrt][b];
				lngth = Get_Length(bm, wgt, sp);
				Sort_Length_Weight(bm, 0, 1, sizestocknums_id, z, sp, nc_fishery, lngth, 0, wgt, cmaxsize, sample_id, &chrt_size, ofp);
				if (thisai >= va95) {
					catch_N += individVERTinfo[icatchnums_id][sp][chrt_size][z][sample_id] * stockinfo[scatchnums_id][sp][z][sample_id] * popratio;
				}
				this_N = individVERTinfo[icatchnums_id][sp][chrt_size][z][sample_id] * stockinfo[scatchnums_id][sp][z][sample_id] * popratio;

				if (this_N < minend_N) {
					minnc = chrt;
					min_remain = ai;
					minend_N = this_N;
				}

				if (this_N > maxend_N) {
					maxnc = chrt;
					max_remain = ai;
					maxend_N = this_N;
				}
			}
		}
	}
	if (start_N - end_N > 0) {
		/* Easy case */
		/* TODO: Check with Beth about this */
		yrs_included = 1.0;
		totDead_N = start_N - end_N;
		calcZ = (1.0 / yrs_included) * (log(start_N / (start_N - catch_N - totDead_N + small_num)));
		calcM = (1.0 / yrs_included) * (log(start_N / (start_N - totDead_N + small_num)));
		calcF = calcZ - calcM;
        
        fprintf(bm->logFile, "Time %e %s calcZ: %e calcM: %e calcF: %e start_N: %e catch_N: %e totDead_N: %e yrs_included: %e\n",
                bm->dayt, FunctGroupArray[sp].groupCode, calcZ, calcM, calcF, start_N, catch_N, totDead_N, yrs_included);
        
	} else {
		/* More complex case - redo with smallest and largest ageclasses */
		oldest_age = maxnc * sp_AgeClassSize + max_remain;
		youngest_age = minnc * sp_AgeClassSize + min_remain;

		if (oldest_age > youngest_age) {
			if (oldest_age >= va95)
				testai = va95;
			else if (oldest_age >= va50)
				testai = va50;
			else
				testai = 0;

			start_N = 0;
			end_N = 0;
			for (z = 0; z < bm->nfzones; z++) {

				/*if(sp == FPO_id){
				 fprintf(ofp, "individVERTinfo[istocknums_id][sp][maxnc][z][sample_id] = %e\n",individVERTinfo[istocknums_id][sp][maxnc][z][sample_id]);
				 fprintf(ofp, "stockinfo[sstocknums_id][sp][z][sample_id] = %e\n",stockinfo[sstocknums_id][sp][z][sample_id]);
				 fprintf(ofp, "popratio = %e\n", popratio);
				 }
				 */
				if (maxnc < nagemat)
					stage = 0;
				else
					stage = 1;
				b = bm->rep_box_of_zone[sp][z][stage];
				popratio = zoneVERTpopratio[sp][maxnc][max_remain][z];

				wgt = biolVERTinfo[bstruct_id][sp][maxnc][b] + biolVERTinfo[bres_id][sp][maxnc][b];
				lngth = Get_Length(bm, wgt, sp);
				Sort_Length_Weight(bm, 0, 1, sizestocknums_id, z, sp, nc_fishery, lngth, 0, wgt, cmaxsize, sample_id, &chrt_size, ofp);
				start_N += individVERTinfo[istocknums_id][sp][chrt_size][z][sample_id] * stockinfo[sstocknums_id][sp][z][sample_id] * popratio;

				if (minnc < nagemat)
					stage = 0;
				else
					stage = 1;
				b = bm->rep_box_of_zone[sp][z][stage];
				popratio = zoneVERTpopratio[sp][minnc][min_remain][z];
				wgt = biolVERTinfo[bstruct_id][sp][minnc][b] + biolVERTinfo[bres_id][sp][minnc][b];
				lngth = Get_Length(bm, wgt, sp);
				Sort_Length_Weight(bm, 0, 1, sizestocknums_id, z, sp, nc_fishery, lngth, 0, wgt, cmaxsize, sample_id, &chrt_size, ofp);
				end_N += individVERTinfo[istocknums_id][sp][chrt_size][z][sample_id] * stockinfo[sstocknums_id][sp][z][sample_id] * popratio;
				catch_N = 0;
				all_done = 0;
				for (chrt = 0; chrt < FunctGroupArray[sp].numCohortsXnumGenes; chrt++) {
					if (all_done) // Stop if already done oldest ageclass
						break;
					for (ai = 0; ai < sp_AgeClassSize; ai++) {
						thisai = chrt * sp_AgeClassSize + ai;
						if (thisai > oldest_age) { // Stop if already done oldest ageclass
							all_done = 1;
							break;
						}
						popratio = zoneVERTpopratio[sp][chrt][ai][z];
						wgt = biolVERTinfo[bstruct_id][sp][chrt][b] + biolVERTinfo[bres_id][sp][chrt][b];
						lngth = Get_Length(bm, wgt, sp);
						Sort_Length_Weight(bm, 0, 1, sizestocknums_id, z, sp, nc_fishery, lngth, 0, wgt, cmaxsize, sample_id, &chrt_size, ofp);
						if (thisai >= testai) {
							catch_N += individVERTinfo[icatchnums_id][sp][chrt_size][z][sample_id] * stockinfo[scatchnums_id][sp][z][sample_id] * popratio;
						}
						this_N = individVERTinfo[icatchnums_id][sp][chrt_size][z][sample_id] * stockinfo[scatchnums_id][sp][z][sample_id] * popratio;
					}
				}
			}
			yrs_included = (double)(oldest_age - youngest_age);
			totDead_N = start_N - end_N;
			calcZ = (1.0 / yrs_included) * (log(start_N / (start_N - catch_N - totDead_N + small_num)));
			calcM = (1.0 / yrs_included) * (log(start_N / (start_N - totDead_N + small_num)));
			calcF = calcZ - calcM;

			fprintf(
					ofp,
					"Time: %e, %s, yrs_included: %e, oldest: %d, youngest: %d, start_N: %e, totDead_N: %e, catch_N: %e, end_N: %e, calcZ: %e, calcM: %e, calcF: %e\n",
					bm->dayt, FunctGroupArray[sp].groupCode, yrs_included, oldest_age, youngest_age, start_N, totDead_N, catch_N, end_N, calcZ, calcM, calcF);

		} else {
			/* All else fails use perfect knowledge */
			fprintf(ofp, "Time: %e, had to cheat for %s M estimation\n", bm->dayt, FunctGroupArray[sp].groupCode);
			/* Do natural and fishing mortality estimate based on the first year adults */
			calcE = 0;
			calcC = 0;
			calcB = 0;
			nc = nagemat * FunctGroupArray[sp].numGeneTypes;
			nc_remain = 0;
			for (z = 0; z < bm->nfzones; z++) {
				if (nc < nagemat)
					stage = 0;
				else
					stage = 1;
				b = bm->rep_box_of_zone[sp][z][stage];
				popratio = zoneVERTpopratio[sp][nc][nc_remain][z];
				wgt = biolVERTinfo[bstruct_id][sp][nc][b] + biolVERTinfo[bres_id][sp][nc][b];
				lngth = Get_Length(bm, wgt, sp);
				Sort_Length_Weight(bm, 0, 1, sizestocknums_id, z, sp, nc_fishery, lngth, 0, wgt, cmaxsize, sample_id, &chrt_size, ofp);
				calcB += individVERTinfo[istocknums_id][sp][chrt_size][z][sample_id] * stockinfo[sstocknums_id][sp][z][sample_id] * popratio;
				calcC += individVERTinfo[icatchnums_id][sp][chrt_size][z][sample_id] * stockinfo[scatchnums_id][sp][z][sample_id] * popratio;

				fprintf( bm->logFile, "Time: %e %s b: %d, z: %d, chrt_size: %d, lngth: %e, wgt: %e (nc: %d, SN: %e, RN: %e), individVERTinfo: %e, stockinfo: %e, individCatch: %e, stockcatch: %e, popratio: %e, \n",
						bm->dayt, FunctGroupArray[sp].groupCode, b, z, chrt_size, lngth, wgt, nc, biolVERTinfo[bstruct_id][sp][nc][b],
						biolVERTinfo[bres_id][sp][nc][b], individVERTinfo[istocknums_id][sp][chrt_size][z][sample_id],
						stockinfo[sstocknums_id][sp][z][sample_id], individVERTinfo[icatchnums_id][sp][chrt_size][z][sample_id],
						stockinfo[scatchnums_id][sp][z][sample_id], popratio);

				//step1 = (mL + mQ * calcB) * calcB;
				//step2 = calcB * calcB / (step1 + small_num);
				//calcE += (mL + mQ * step2) * step2;	
				/* Other mortality */

				calcE += (mL + mQ * calcB) * calcB;
				for (pred = 0; pred < bm->K_num_tot_sp; pred++) {
					E1_sp= 0;
					step1 = 0;
					clear = 0;
					if (FunctGroupArray[pred].isVertebrate == TRUE) {
						/* Predation mortality */
						E1_sp = FunctGroupArray[pred].speciesParams[E1_id];
						step1 = bm->pSPVERTeat[pred][sp][adult_id][adult_id] * stockinfo[scatchnums_id][sp][z][sample_id] * popratio;
						//	clear = bm->C_spVERT[sp][nagemat] / (1.0 + step1 * E1_sp * bm->C_spVERT[sp][nagemat] / bm->mum_spVERT[sp][nagemat]);
						/* Don't worry about the numsec in the later part of the equation as they will cancel out */
						clear = FunctGroupArray[sp].scaled_C[nagemat] * numsec / (1.0 + step1 * E1_sp * FunctGroupArray[sp].scaled_C[nagemat]
								/ FunctGroupArray[sp].scaled_mum[nagemat]);
						calcE += clear * step1;
					}

					//fprintf(ofp, "Time: %e %s on %s E1: %e, step1: %e, clear: %e, calcE: %e (+ %e) and mL: %e, mQ: %e, step2: %e\n", bm->dayt,
					//		FunctGroupArray[pred].groupCode, FunctGroupArray[sp].groupCode, E1_sp, step1, clear, calcE, (clear * step1), mL, mQ, step2);
				}

			}
			start_N = calcC + calcB + calcE;
			calcF = log(start_N / (start_N - calcC + small_num));
			calcM = log(start_N / (start_N - calcE + small_num));

			fprintf(bm->logFile, "Time: %e, %s, start_N: %e (vs %e), calcC: %e, calcE: %e, calcB: %e, calcF: %e, calcM: %e\n", bm->dayt, FunctGroupArray[sp].groupCode, start_N, bm->calcNstart[sp][expect_id], calcC, calcE, calcB, calcF, calcM);
		}
	}
	bm->NAssess[sp][est_M_id] = calcM;
	bm->NAssess[sp][est_Fcurr_id] = calcF;
	M = calcM;

	/** Fishing mortality parameters **/
	/* Calculate virgin biomass estimate */
	for (na = 0; na < nageclass; na++) {
		nc = (int) (floor(na / dsp_ageClassSize));
		num_nyr[na][est1_id] = initVERTinfo[sp][nc][DEN_id] / dsp_ageClassSize;
	}
	for (nyr = 0; nyr < bm->assessnyr; nyr++) {
		spawn_biom1 = 0;
		for (na = nageclass - 1; na > 0; na--) {
			num_nyr[na][est1_id] = num_nyr[na - 1][est1_id] * exp(-M); // Unfished case
			nc = (int) (floor(na / dsp_ageClassSize));
			weight = initVERTinfo[sp][nc][SN_id] + initVERTinfo[sp][nc][RN_id];
			if (na > nagemat - 1) {
				spawn_biom1 += num_nyr[na][est1_id] * weight;
			}
		}
		num_rec1 = (0.5 * spawn_biom1 * Balpha / (Bbeta + 0.5 * spawn_biom1)) / (KWRR_sp + KWSR_sp);
		num_nyr[0][est1_id] = num_rec1;
	}
	calcB1 = 0;
	for (na = 0; na < nageclass; na++) {
		nc = (int) (floor(na / dsp_ageClassSize));
		weight = initVERTinfo[sp][nc][SN_id] + initVERTinfo[sp][nc][RN_id];
		calcB1 += num_nyr[na][est1_id] * weight;
	}
	/* Convert to tonnes */
	bm->estinitpop[sp] = calcB1 * bm->X_CN * mg_2_tonne;

	/* Calculate FrefA and FrefC */
	CalculateFref(bm, sp, ofp, 0);

	return;
}

/********************** Estimating F reference points *****************
 Calculating Fref for Perfect knowledge and other cases
 */
void CalculateFref(MSEBoxModel *bm, int sp, FILE *ofp, int typecall) {
    
    int do_debug, reset_yes, nyr, ij, ny, not_done, nf, counter, nc, na, den, b, k;
    double tempF, tbiom1, tbiom2, sp_SN, sp_RN, current_slope, this_CPUE, currentcatch, currenteffort, reldiff, ratiodiff, BrefA, BrefC, calcB2, num_rec2, V, M, tempF2, spawn_biom1, spawn_biom2, this_num, weight, num_rec1, calcB1, Fcurr;
    double diff = 0.0;
    
    int va50 = (int) (FunctGroupArray[sp].speciesParams[Age50pcntV_id]);
	int va95 = (int) (FunctGroupArray[sp].speciesParams[Age95pcntV_id]);
	int nageclass = (int) (FunctGroupArray[sp].numCohorts * FunctGroupArray[sp].ageClassSize);
	int nagemat = (int) (FunctGroupArray[sp].speciesParams[age_mat_id]);
    
    double KSPA_sp = FunctGroupArray[sp].speciesParams[KSPA_id];
	double FSP_sp = FunctGroupArray[sp].speciesParams[FSP_id];
	double Balpha = FunctGroupArray[sp].speciesParams[BHalpha_id];
	double Bbeta = FunctGroupArray[sp].speciesParams[BHbeta_id];
	double KWSR_sp = FunctGroupArray[sp].speciesParams[KWSR_id];
	double KWRR_sp = FunctGroupArray[sp].speciesParams[KWRR_id];
    double nasize = (double)(FunctGroupArray[sp].ageClassSize);
    
    if (bm->debug && (bm->debug == debug_assess))
		do_debug = 1;
	else
		do_debug = 0;
    
	not_done = 1;
	tempF = bm->NAssess[sp][est_Fcurr_id];
	tempF2 = bm->NAssess[sp][est_Fcurr_id];
    if (bm->estBo[sp] > 0) {
        BrefA = bm->targ_refA * bm->estBo[sp];
        BrefC = bm->targ_refC * bm->estBo[sp];
    } else {
        BrefA = bm->targ_refA * bm->estinitpop[sp];
        BrefC = bm->targ_refC * bm->estinitpop[sp];
    }
    M = bm->NAssess[sp][est_M_id];
    Fcurr = bm->NAssess[sp][est_Fcurr_id];
    
	counter = 0;
	while (not_done) {
		/* Run model forward to see what biomass is reached with current F and M values and then scale F until
		 end up with desired Fref points */

        if(!typecall) {
            for (na = 0; na < nageclass; na++) {
            	nc = (int) (floor(na / nasize));
                num_nyr[na][est1_id] = initVERTinfo[sp][nc][DEN_id] / nasize;
                num_nyr[na][est2_id] = initVERTinfo[sp][nc][DEN_id] / nasize;
            }
		} else {
            for (na = 0; na < nageclass; na++) {
                den = FunctGroupArray[sp].NumsTracers[na];
                this_num = 0;
                
                for (b = 0; b < bm->nbox; b++) {
                    if (bm->boxes[b].type != BOUNDARY) {
                        for (k=0; k < bm->boxes[b].nz; k++) {
                            this_num += bm->boxes[b].tr[k][den];
                        }
                    }
                }
                
                num_nyr[na][est1_id] = this_num / nasize;
                num_nyr[na][est2_id] = this_num / nasize;
            }
        }
        
		for (nyr = 0; nyr < bm->assessnyr; nyr++) {
			spawn_biom1 = 0;
			spawn_biom2 = 0;
			tbiom1 = 0;
			tbiom2 = 0;
			for (na = nageclass - 1; na > 0; na--) {
				V = 1.0 / (1.0 + exp(-log(19) * ((na - va50) / (va95 - va50 + small_num))));
				num_nyr[na][est1_id] = num_nyr[na - 1][est1_id] * exp(-M - V * tempF);
				num_nyr[na][est2_id] = num_nyr[na - 1][est2_id] * exp(-M - V * tempF2);
				nc = (int) (floor(na / nasize));
				sp_SN = initVERTinfo[sp][nc][SN_id];
				sp_RN = initVERTinfo[sp][nc][RN_id];
				weight = sp_SN + sp_RN;
                
                if (na > nagemat - 1) {
                    spawn_biom1 += num_nyr[na][est1_id] * FunctGroupArray[sp].scaled_FSPB[nc] * Ecology_Spawn(sp, KSPA_sp, FSP_sp, sp_SN, sp_RN, FunctGroupArray[sp].scaled_FSPB[nc], FunctGroupArray[sp].X_RS[nc], ofp);
                    spawn_biom2 += num_nyr[na][est2_id] * FunctGroupArray[sp].scaled_FSPB[nc] * Ecology_Spawn(sp, KSPA_sp, FSP_sp, sp_SN, sp_RN, FunctGroupArray[sp].scaled_FSPB[nc], FunctGroupArray[sp].X_RS[nc], ofp);
                }
				tbiom1 += num_nyr[na][est1_id] * weight;
				tbiom2 += num_nyr[na][est2_id] * weight;
			}
			num_rec1 = (0.5 * spawn_biom1 * Balpha / (Bbeta + 0.5 * tbiom1)) / (KWRR_sp + KWSR_sp);
			num_rec2 = (0.5 * spawn_biom2 * Balpha / (Bbeta + 0.5 * tbiom2)) / (KWRR_sp + KWSR_sp);
			num_nyr[0][est1_id] = num_rec1;
			num_nyr[0][est2_id] = num_rec2;
		}
		calcB1 = 0;
		calcB2 = 0;
		for (na = 0; na < nageclass; na++) {
			nc = (int) (floor(na / nasize));
			weight = initVERTinfo[sp][nc][SN_id] + initVERTinfo[sp][nc][RN_id];
			calcB1 += num_nyr[na][est1_id] * weight;
			calcB2 += num_nyr[na][est2_id] * weight;
		}
		calcB1 *= bm->X_CN * mg_2_tonne; // Convert to tonnes
		calcB2 *= bm->X_CN * mg_2_tonne; // Convert to tonnes
		/* Check against desired reference level - BrefA */
		reldiff = fabs(calcB1 - BrefA) / (BrefA + small_num);
		ratiodiff = calcB1 / (BrefA + small_num);
		if (reldiff > bm->Assess_Btol) {
			not_done = 1;
			if (!tempF && (ratiodiff > 1.0)) {
				tempF = M;
				if (do_debug) {
					fprintf(bm->logFile, "Time: %e %s reset tempF = M (%e)\n", bm->dayt, FunctGroupArray[sp].groupCode, M);
				}
			} else {
				diff = 1.0 - log(1.0 / ratiodiff);
				tempF *= diff;
			}
		} else {
			not_done = 0;
			diff = 1;
		}
        
		if (do_debug) {
			fprintf(bm->logFile, "Time: %e %s calcB1: %e, BrefA: %e, reldiff: %e, ratiodiff: %e, diff: %e, nageclass: %d\n", bm->dayt, FunctGroupArray[sp].groupCode, calcB1, BrefA, reldiff, ratiodiff, diff, nageclass);
		}
        
		/* Check against desired reference level - BrefC */
		reldiff = fabs(calcB2 - BrefC) / (BrefC + small_num);
		ratiodiff = calcB2 / (BrefC + small_num);
		if (reldiff > bm->Assess_Btol) {
			not_done = 1;
			if (!tempF2 && (ratiodiff > 1.0)) {
				tempF2 = M;
				if (do_debug) {
					fprintf(bm->logFile, "Time: %e %s reset tempF2 = M (%e)\n", bm->dayt, FunctGroupArray[sp].groupCode, M);
				}
			} else {
				diff = 1.0 - log(1.0 / ratiodiff);
				tempF2 *= diff;
			}
		} else {
			not_done = 0;
			diff = 1;
		}
        
		if (do_debug) {
			fprintf(bm->logFile, "Time: %e %s calcB2: %e, BrefC: %e, reldiff: %e, ratiodiff: %e, diff: %e, nageclass: %d\n", bm->dayt, FunctGroupArray[sp].groupCode, calcB2, BrefC, reldiff, ratiodiff, diff, nageclass);
		}
        
		if (_isnan(tempF)) {
			if (_isnan(tempF2)) {
				tempF = Fcurr;
				tempF2 = 0.5 * Fcurr;
				fprintf(bm->logFile, "Time: %e, %s was assessed as needing INF F - clearly wrong! - so used tempF = Fcurr, tempF2 = 0.5 * Fcurr\n", bm->dayt, FunctGroupArray[sp].groupCode);
				//quit("Time: %e, %s was assessed as needing INF F - clearly wrong!\n", bm->dayt, FunctGroupArray[sp].groupCode);
			} else
				tempF = tempF2;
		}
        
		counter++;
        
		if (counter > 3)
			not_done = 0;
	}
	bm->NAssess[sp][est_FrefA_id] = tempF;
	bm->NAssess[sp][est_FrefC_id] = tempF2;
    
    fprintf(bm->logFile, "Time: %e %s FrefA: %e FrefC: %e\n", bm->dayt, FunctGroupArray[sp].groupCode, tempF, tempF2);
        
	/** Estimate CPUE slope parameters **/
	currentcatch = 0;
	currenteffort = 0;
    
	for (nf = 0; nf < bm->K_num_fisheries; nf++) {
		currentcatch += Harvest_Get_TotCumCatch(sp, nf, bm->thisyear); //bm->TotCumCatch[sp][nf];
		for (ij = 0; ij < bm->nbox; ij++) {
			currenteffort += bm->CumEffort[nf][ij];
		}
	}
	this_CPUE = currentcatch / (currenteffort + small_num);
	bm->NAssess[sp][est_Ccurr_id] = currentcatch;
    
	if (bm->thisyear) {
		/* Check slope */
		current_slope = bm->NAssess[sp][est_Cslope_id];
        
		reset_yes = 0;
		if ((current_slope > 0) && (this_CPUE < bm->NAssess[sp][est_CPUEstart_id]))
			reset_yes = 1;
		if ((current_slope < 0) && (this_CPUE > bm->NAssess[sp][est_CPUEstart_id]))
			reset_yes = 1;
	} else {
		reset_yes = 1;
	}
    
	if (reset_yes) {
		bm->NAssess[sp][est_CPUEstart_id] = this_CPUE;
		bm->NAssess[sp][est_CPUEnow_id] = this_CPUE;
		bm->NAssess[sp][est_Cslope_id] = 0;
		bm->NAssess[sp][est_numyr_id] = 1;
		CPUEtrend[sp][0] = this_CPUE;
	} else {
		bm->NAssess[sp][est_CPUEnow_id] = this_CPUE;
		ny = (int) (bm->NAssess[sp][est_numyr_id]);
		CPUEtrend[sp][ny] = this_CPUE;
		bm->NAssess[sp][est_numyr_id]++;
        
		/* Estimate slope of CPUE trend - FIX enable linear regression (check how to initialise it)
		 for(i=0; i<bm->NAssess[nid][est_numyr_id]; i++){
		 XX[0] = CPUEtrend[nid][i];
		 XX[1] = i;
		 }
		 for(i=bm->NAssess[nid][est_numyr_id]; i<floor(bm->tstop / 365.0); i++){
		 XX[0] = 0;
		 XX[1] = 1;
		 }
		 Linear_Regression(CPUE_linear_regress, sp, XX, 2, PP, YY, bm->NAssess[nid][est_numyr_id], 0, llogfp);
         
		 As interim calculation method use rise over run
		 */
		bm->NAssess[sp][est_Cslope_id] = (bm->NAssess[sp][est_CPUEnow_id] - bm->NAssess[sp][est_CPUEstart_id]) / bm->NAssess[sp][est_numyr_id];
        
	}
    
	return;
}

/**************/
void Calc_Percentile(double *EstToSort, double *SortedResults, int *SortRank, int nbs, int nchrt, int YrMax, FILE *ofp) {
	int i, j, rank, indx;

	/* Sort in descending order */
	/* Initialise the sort */
	for (j = 0; j < nbs; j++) {
		SortRank[j] = 0;
	}

	/* Do the sort */
	for (j = 0; j < nbs; j++) {
		rank = 0;
		for (i = 0; i < nbs; i++) {
			if (EstToSort[j] > EstToSort[i])
				rank++;
		}
		SortRank[j] = nbs - 1 - rank;
	}

	/* Save sorted results */
	for (j = 0; j < nbs; j++) {
		indx = SortRank[j];
		SortedResults[indx] = EstToSort[j];
	}

	return;
}

/**
 * \brief Pseudo assessent for use when not using full assessment survey code, but still want harvest rules in dynamic effort model
 */
void Assess_Pseudo_Estimate_Prm(MSEBoxModel *bm, int sp, FILE *ofp) {
	int na, nc, nyr, not_done, nf, ij, ny, reset_yes, nreset1, nreset2, do_debug, nid, counter;
	double tempF, spawn_biom2, weight, V, M, num_rec2, calcZ, calcF, spawn_biom1, num_rec1, calcB2, Fcurr, BrefA, reldiff, ratiodiff, diff, natDead_N, sp_SN,
			sp_RN, calcB1, currentcatch, currenteffort, this_CPUE, current_slope, start_N, calcM, tbiom1, catch_N, temp,
			mort_scale, tempF2, BrefC, tbiom2, prevF, prevF2, prevR, prevR2, avgF;

	/* Parameters */
	int tier = (int) (FunctGroupArray[sp].speciesParams[tier_id]);
	double Balpha = FunctGroupArray[sp].speciesParams[BHalpha_id];
	double Bbeta = FunctGroupArray[sp].speciesParams[BHbeta_id];
	double KWSR_sp = FunctGroupArray[sp].speciesParams[KWSR_id];
	double KWRR_sp = FunctGroupArray[sp].speciesParams[KWRR_id];
	double KSPA_sp = FunctGroupArray[sp].speciesParams[KSPA_id];
	double FSP_sp = FunctGroupArray[sp].speciesParams[FSP_id];
    double dsp_AgeClassSize = (double)(FunctGroupArray[sp].ageClassSize);
	int va50 = (int) (FunctGroupArray[sp].speciesParams[Age50pcntV_id]);
	int va95 = (int) (FunctGroupArray[sp].speciesParams[Age95pcntV_id]);
	int nageclass = FunctGroupArray[sp].numCohortsXnumGenes * FunctGroupArray[sp].ageClassSize;
	int nagemat = (int) (FunctGroupArray[sp].speciesParams[age_mat_id]);
	int sp_AgeClassSize = FunctGroupArray[sp].ageClassSize;
	int update_date = (int) (FunctGroupArray[sp].speciesParams[calcupdate_date_id]);

	nagemat *= sp_AgeClassSize;

    if (bm->debug && (bm->debug == debug_assess))
		do_debug = 1;
	else
		do_debug = 0;
    
	nid = sp;
    
	printf("Assessing %s\n", FunctGroupArray[sp].groupCode);
    

	/* Biomass - in wet weight */
	bm->NAssess[sp][est_med_stock_id] = bm->totfishpop[sp] * bm->X_CN * mg_2_tonne;
	bm->estinitpop[sp] = bm->estBo[sp];  // Asumes estBo input in tonnes

	if (bm->debug == debug_assess)
		fprintf(ofp, "Time: %e %s stock: %e Bo: %e\n", bm->dayt, FunctGroupArray[sp].groupCode, bm->NAssess[sp][est_med_stock_id], bm->estinitpop[sp]);


	fprintf(bm->logFile, "bm->calcMnum[sp][hist_id] = %e\n", bm->calcMnum[sp][hist_id]);
	fprintf(bm->logFile, "bm->calcMnum[sp][expect_id] = %e\n", bm->calcMnum[sp][expect_id]);
	fprintf(bm->logFile, "bm->rand[sp][calcM_id][bm->thisyear] = %e\n", bm->rand[sp][calcM_id][bm->thisyear]);

	/* Mortality estimates */
	if (update_date > 182) {
		/* If restarted records within last 6 months use previous
		 year's records as more complete */
		start_N = bm->calcNstart[sp][hist_id] * bm->rand[sp][startN_id][bm->thisyear];
		catch_N = bm->calcFnum[sp][hist_id] * bm->rand[sp][calcF_id][bm->thisyear];
		natDead_N = bm->calcMnum[sp][hist_id] * bm->rand[sp][calcM_id][bm->thisyear];
		mort_scale = 1.0;
	} else {
		/* If restart records early each year then current records should be sufficent */
		start_N = bm->calcNstart[sp][expect_id] * bm->rand[sp][startN_id][bm->thisyear];
		catch_N = bm->calcFnum[sp][expect_id] * bm->rand[sp][calcF_id][bm->thisyear];
		natDead_N = bm->calcMnum[sp][expect_id] * bm->rand[sp][calcM_id][bm->thisyear];
		mort_scale = 365.0 / (365.0 - update_date);
	}

	/** Sanity checks **/
	if (start_N < 0)
		start_N = 1;
	if (catch_N < 0)
		catch_N = 0;
	if (natDead_N < 0)
		natDead_N = 0;

	if (catch_N > start_N) {
		/* Sanity check so can not record catching more than started with
		 - do pro-rated correction and down scaling */
		temp = catch_N;
		catch_N = start_N * temp / (temp + natDead_N);
		natDead_N = start_N - catch_N;
	}

	/* This should get used rarely - if not rethink this as basically capping M at about 0.8 */
	if (start_N < natDead_N) {
		start_N = 1.25 * natDead_N;
		fprintf(ofp, "Time: %e had to reset start_N to 1.2 * natDead_N (so now %e)\n", bm->dayt, start_N);
	}

	calcZ = (log(start_N / (start_N - catch_N - natDead_N + small_num)));
	calcM = (log(start_N / (start_N - natDead_N + small_num)));
	calcF = calcZ - calcM;

	/* Adjust for any partial year of data effects */
	calcZ *= mort_scale;
	calcM *= mort_scale;
	calcF *= mort_scale;

	if (do_debug) {
		fprintf(ofp, "Time: %e %s calcM: %e calcZ: %e, calcF: %e, start_N: %e, catch_N: %e, natDead_N: %e (mort_scale: %e)\n", bm->dayt,
				FunctGroupArray[sp].groupCode, calcM, calcZ, calcF, start_N, catch_N, natDead_N, mort_scale);
	}

	bm->NAssess[sp][est_M_id] = calcM;
	bm->NAssess[sp][est_Fcurr_id] = calcF;
	Fcurr = calcF;
	M = calcM;

	/* Sanity check on M */
	if (M < 0.001) {
		M *= 100.0;
		if (M > 0.3)
			M *= 0.5;
		fprintf(ofp, "Time: %e rescaled %s M est to %e\n", bm->dayt, FunctGroupArray[sp].groupCode, M);
	} else if (M < 0.01) {
		M *= 10.0;
		if (M > 0.3)
			M *= 0.5;
		fprintf(ofp, "Time: %e rescaled %s M est to %e\n", bm->dayt, FunctGroupArray[sp].groupCode, M);
	}

	/** Fishing mortality parameters **/
	/* Calculate FrefA and FrefC */
	if (tier == 1) {
		not_done = 1;
		tempF = Fcurr + small_num;
		tempF2 = Fcurr + small_num;
		prevF = tempF;
		prevF2 = tempF2;
		prevR = MAXDOUBLE;
		prevR2 = MAXDOUBLE;
		BrefA = bm->targ_refA * bm->estinitpop[nid];
		BrefC = bm->targ_refC * bm->estinitpop[nid];
		counter = 0;
		nreset1 = 0;
		nreset2 = 0;

		while (not_done) {
			/* Run model forward to see what biomass is reached with current F and M values */
			for (na = 0; na < nageclass; na++) {
				nc = (int) (floor(na / dsp_AgeClassSize));
				num_nyr[na][est1_id] = initVERTinfo[sp][nc][DEN_id] / dsp_AgeClassSize;
				num_nyr[na][est2_id] = initVERTinfo[sp][nc][DEN_id] / dsp_AgeClassSize;

				if (do_debug) {
					fprintf(ofp, "Time: %e %s-%d (nc: %d, nageclass: %d) initDEN: %e\n", bm->dayt, FunctGroupArray[sp].groupCode, na, nc,
							FunctGroupArray[sp].ageClassSize, num_nyr[na][est2_id]);
				}

			}
			for (nyr = 0; nyr < bm->assessnyr; nyr++) {
				spawn_biom1 = 0;
				spawn_biom2 = 0;
				tbiom1 = 0;
				tbiom2 = 0;
				for (na = nageclass - 1; na > 0; na--) {
					V = 1.0 / (1.0 + exp(-log(19) * ((na - va50) / (va95 - va50 + small_num))));
					num_nyr[na][est1_id] = num_nyr[na - 1][est1_id] * exp(-M - V * tempF); // Fished case
					num_nyr[na][est2_id] = num_nyr[na - 1][est2_id] * exp(-M - V * tempF2); // Fished case
					nc = (int) (floor(na / dsp_AgeClassSize));
					sp_SN = initVERTinfo[sp][nc][SN_id];
					sp_RN = initVERTinfo[sp][nc][RN_id];
					weight = sp_SN + sp_RN;

					if (na > nagemat - 1) {
                        spawn_biom1 += num_nyr[na][est1_id] * FunctGroupArray[sp].scaled_FSPB[nc] * Ecology_Spawn(sp, KSPA_sp, FSP_sp, sp_SN, sp_RN, FunctGroupArray[sp].scaled_FSPB[nc], FunctGroupArray[sp].X_RS[nc], ofp);
                        spawn_biom2 += num_nyr[na][est2_id] * FunctGroupArray[sp].scaled_FSPB[nc] * Ecology_Spawn(sp, KSPA_sp, FSP_sp, sp_SN, sp_RN, FunctGroupArray[sp].scaled_FSPB[nc], FunctGroupArray[sp].X_RS[nc], ofp);
					}
					tbiom1 += num_nyr[na][est1_id] * weight;
					tbiom2 += num_nyr[na][est2_id] * weight;

					if (do_debug) {
						fprintf(
								ofp,
								"Time: %e %s-%d it: %d, nyr: %d, nend: %e, nstart: %e, mortexp: %e, wgt: %e, spbiom2: %e tbiom2 : %e, (V: %e, M: %e, tempF2: %e, nagemat: %d)\n",
								bm->dayt, FunctGroupArray[sp].groupCode, na, counter, nyr, num_nyr[na][est2_id], num_nyr[na - 1][est2_id], (double) exp(-M - V
										* tempF), weight, spawn_biom2, tbiom2, V, M, tempF2, nagemat);
					}

				}
				num_rec1 = (0.5 * spawn_biom1 * Balpha / (Bbeta + 0.5 * tbiom1)) / (KWRR_sp + KWSR_sp);
				num_rec2 = (0.5 * spawn_biom2 * Balpha / (Bbeta + 0.5 * tbiom2)) / (KWRR_sp + KWSR_sp);
				num_nyr[0][est1_id] = num_rec1;
				num_nyr[0][est2_id] = num_rec2;
				if (do_debug) {
					fprintf(ofp, "Time: %e %s-%d nyr: %d, numrec2: %e (Balpha: %e, Bbeta: %e, recwgt: %e)\n", bm->dayt, FunctGroupArray[sp].groupCode, na, nyr,
							num_rec2, Balpha, Bbeta, (KWRR_sp + KWSR_sp));
				}
			}
			calcB1 = 0;
			calcB2 = 0;
			for (na = 0; na < nageclass; na++) {
				nc = (int) (floor(na / dsp_AgeClassSize));
				weight = initVERTinfo[sp][nc][SN_id] + initVERTinfo[sp][nc][RN_id];
				calcB1 += num_nyr[na][est1_id] * weight;
				calcB2 += num_nyr[na][est2_id] * weight;
				if (do_debug) {
					fprintf(ofp, "Time: %e %s-%d wgt: %e, calcB1: %e calcB2: %e)\n", bm->dayt, FunctGroupArray[sp].groupCode, na, weight, calcB1, calcB2);
				}
			}
			calcB1 *= bm->X_CN * mg_2_tonne; // Convert to tonnes
			calcB2 *= bm->X_CN * mg_2_tonne; // Convert to tonnes

			/* Check against desired reference level - BrefA */
			reldiff = fabs(calcB1 - BrefA) / (BrefA + small_num);
			ratiodiff = calcB1 / (BrefA + small_num);
			if (reldiff > bm->Assess_Btol) {
				not_done = 1;
				if ((tempF < buffer_rounding) && (ratiodiff > 1.0)) {
					nreset1++;
					prevF = tempF;
					prevR = ratiodiff;
					tempF = M / nreset1;
					diff = -1;
					if (do_debug) {
						fprintf(ofp, "Time: %e %s reset tempF ~ M (%e)\n", bm->dayt, FunctGroupArray[sp].groupCode, tempF);
					}
				} else {
					diff = max(small_num,1.0 - log(1 / ratiodiff));
					if ((diff == small_num) && (prevR > 1.0) && (ratiodiff < 1.0)) {
						avgF = (prevF / prevR + tempF / (ratiodiff + small_num)) / 2.0;
						prevF = tempF;
						prevR = ratiodiff;
						tempF = avgF;
					} else {
						prevF = tempF;
						prevR = ratiodiff;
						tempF *= diff;
					}
				}
			} else {
				not_done = 0;
				diff = 1;
			}

			if (do_debug) {
				fprintf(ofp, "Time: %e %s it %d, calcB1: %e, BrefA: %e, reldiff: %e, ratiodiff: %e, diff: %e, nageclass: %d, tempF: %e\n", bm->dayt,
						FunctGroupArray[sp].groupCode, counter, calcB1, BrefA, reldiff, ratiodiff, diff, nageclass, tempF);
			}

			/* Check against desired reference level - BrefC */
			reldiff = fabs(calcB2 - BrefC) / (BrefC + small_num);
			ratiodiff = calcB2 / (BrefC + small_num);
			if (reldiff > bm->Assess_Btol) {
				not_done = 1;
				if ((tempF2 < buffer_rounding) && (ratiodiff > 1.0)) {
					nreset2++;
					prevF2 = tempF2;
					prevR2 = ratiodiff;
					tempF2 = M / nreset2;
					diff = -1;
					if (do_debug) {
						fprintf(ofp, "Time: %e %s reset tempF2 ~ M (%e)\n", bm->dayt, FunctGroupArray[sp].groupCode, tempF2);
					}
				} else {
					diff = max(small_num,1.0 - log(1.0 / ratiodiff));
					if ((diff == small_num) && (prevR2 > 1.0) && (ratiodiff < 1.0)) {
						avgF = (prevF2 / prevR2 + tempF2 / (ratiodiff + small_num)) / 2.0;
						prevF2 = tempF2;
						prevR2 = ratiodiff;
						tempF2 = avgF;
					} else {
						prevF2 = tempF2;
						prevR2 = ratiodiff;
						tempF2 *= diff;
					}
				}
			} else {
				not_done = 0;
				diff = 1;
			}

			if (do_debug) {
				fprintf(ofp, "Time: %e %s it %d calcB2: %e, BrefC: %e, reldiff: %e, ratiodiff: %e, diff: %e, nageclass: %d, tempF2: %e\n", bm->dayt,
						FunctGroupArray[sp].groupCode, counter, calcB2, BrefC, reldiff, ratiodiff, diff, nageclass, tempF2);
			}

			if (_isnan(tempF)) {
				if (_isnan(tempF2)) {
					tempF = Fcurr;
					tempF2 = 0.5 * Fcurr;
					fprintf(ofp, "Time: %e, %s was assessed as needing INF F - clearly wrong! - so used tempF = Fcurr, tempF2 = 0.5 * Fcurr\n", bm->dayt,
							FunctGroupArray[sp].groupCode);
					//quit("Time: %e, %s was assessed as needing INF F - clearly wrong!\n", bm->dayt, FunctGroupArray[sp].groupCode);
				} else
					tempF = tempF2;
			}

			counter++;

			if (counter > 10)
				not_done = 0;
		}
		bm->NAssess[nid][est_FrefA_id] = tempF;
		bm->NAssess[nid][est_FrefC_id] = tempF2;
	} else {
		bm->NAssess[nid][est_FrefA_id] = Fcurr * 0.8;
		bm->NAssess[nid][est_FrefC_id] = Fcurr * 0.5;
	}

	if (bm->debug == debug_assess)
		fprintf(ofp, "Time: %e %s est_refA: %e\n", bm->dayt, FunctGroupArray[sp].groupCode, bm->NAssess[nid][est_FrefA_id]);

	/** Estimate CPUE slope parameters **/
	currentcatch = 0;
	currenteffort = 0;
	for (nf = 0; nf < bm->K_num_fisheries; nf++) {
		//currentcatch += bm->TotCumCatch[sp][nf][bm->thisyear];
		currentcatch += Harvest_Get_TotCumCatch(sp, nf, bm->thisyear);
		for (ij = 0; ij < bm->nbox; ij++) {
			currenteffort += bm->CumEffort[nf][ij];
		}
	}
	this_CPUE = currentcatch / (currenteffort + small_num);
	bm->NAssess[nid][est_Ccurr_id] = currentcatch;

	if (bm->thisyear) {
		/* Check slope */
		current_slope = bm->NAssess[nid][est_Cslope_id];

		reset_yes = 0;
		if ((current_slope > 0) && (this_CPUE < bm->NAssess[nid][est_CPUEstart_id]))
			reset_yes = 1;
		if ((current_slope < 0) && (this_CPUE > bm->NAssess[nid][est_CPUEstart_id]))
			reset_yes = 1;
	} else {
		reset_yes = 1;
	}

	if (reset_yes) {
		bm->NAssess[nid][est_CPUEstart_id] = this_CPUE;
		bm->NAssess[nid][est_CPUEnow_id] = this_CPUE;
		bm->NAssess[nid][est_Cslope_id] = 0;
		bm->NAssess[nid][est_numyr_id] = 1;
		CPUEtrend[nid][0] = this_CPUE;
	} else {
		bm->NAssess[nid][est_CPUEnow_id] = this_CPUE;
		ny = (int) (bm->NAssess[nid][est_numyr_id]);
		CPUEtrend[nid][ny] = this_CPUE;
		bm->NAssess[nid][est_numyr_id]++;

		/* Estimate slope of CPUE trend */
		bm->NAssess[nid][est_Cslope_id] = (bm->NAssess[nid][est_CPUEnow_id] - bm->NAssess[nid][est_CPUEstart_id]) / bm->NAssess[nid][est_numyr_id];

	}

	if (do_debug) {
		fprintf(ofp, "Time: %e %s est_CPUEstart: %e, est_CPUEnow: %e, est_Cslope: %e, est_numyr: %e\n", bm->dayt, FunctGroupArray[sp].groupCode,
				bm->NAssess[nid][est_CPUEstart_id], bm->NAssess[nid][est_CPUEnow_id], bm->NAssess[nid][est_Cslope_id], bm->NAssess[nid][est_numyr_id]);
	}

	return;
}

