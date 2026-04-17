/**
 *  File:           atAssessTierSetup.c
 *  Author:         Beth Fulton
 *  Code to allocate and free the RBCstructure data structures.
 *
 *
 *
 */

#include <stdarg.h>
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <math.h>
#include <string.h>
#include <sjwlib.h>
#include "atsample.h"
#include <convertXML.h>
#include <atSS3LinkLib.h>

static double Assess_errfun (double xlim);
static void getMove(MSEBoxModel *bm, int species);
static void SetUpBiology(MSEBoxModel *bm, int species);

/**
 * Allocate the Tier assessment data structures.
 */
int Tier_Assessment_Allocate(MSEBoxModel *bm) {

	int groupIndex;
	int nYears;
	int nFuture;
	int nPast;
	int sp_maxAge;
	int sp_cCsel;
	int nLen;
    int AccumAge;
    int Ntriggers;
    int Nregions;
    int maxNYears = 0;
    
	//printf("Creating Tier Assessment arrays\n");

	/* Allocate the arrays in the RBCspeciesArray structure */
    bm->RBCestimation.nFuture = (int)(4 + (bm->tstop / 86400) / 365);  // As can't project beyond end of run but buffer for year+2 etc
    nFuture = bm->RBCestimation.nFuture;
    
    nPast = (int)bm->RBCestimation.HistYrMax - (int)bm->RBCestimation.HistYrMin + 1;
    nYears = nFuture + nPast;
    
    bm->RBCestimation.speciesMetierToRPFleet = Util_Alloc_Init_2D_Int(bm->K_num_fisheries, bm->K_num_tot_sp, 0);
    bm->RBCestimation.speciesRPFleetToMetier = Util_Alloc_Init_2D_Int(bm->K_num_tot_sp, bm->K_num_fisheries, 0);
    bm->RBCestimation.metierArray = (METarrays *) malloc(sizeof(METarrays) * (unsigned long int)bm->K_num_fisheries);
    
    bm->RBCestimation.F_actFleet = Util_Alloc_Init_2D_Double(bm->K_num_fisheries, bm->K_num_tot_sp, 0.0);
    bm->RBCestimation.FratioFleet = Util_Alloc_Init_2D_Double(bm->K_num_fisheries, bm->K_num_tot_sp, 0.0);
    
	for (groupIndex = 0; groupIndex < bm->K_num_tot_sp; groupIndex++) {
        
        printf("Doing Tier_Assessment_Allocate for %s\n", FunctGroupArray[groupIndex].groupCode);
        
        if ((FunctGroupArray[groupIndex].speciesParams[assess_flag_id] == FALSE) && (FunctGroupArray[groupIndex].isImpacted == FALSE))
            continue;
                
        bm->RBCestimation.RBCspeciesArray[groupIndex].regID = Util_Alloc_Init_1D_Int(bm->nbox, 0);     /* Mapping of boxes to assessment region */
        Nregions = (int)(bm->RBCestimation.RBCspeciesParam[groupIndex][NumRegions_id]);
        
		/* Length attributes */
		//bm->RBCestimation.RBCspeciesParam[groupIndex][Nlen_id] = bm->K_num_size;
		nLen = (int)bm->RBCestimation.RBCspeciesParam[groupIndex][Nlen_id];
        Ntriggers = bm->RBCestimation.NumTriggers;
        
        if (FunctGroupArray[groupIndex].numCohorts * FunctGroupArray[groupIndex].ageClassSize > bm->RBCestimation.RBCspeciesParam[groupIndex][MaxAge_id])
            bm->RBCestimation.RBCspeciesParam[groupIndex][MaxAge_id] = FunctGroupArray[groupIndex].numCohorts * FunctGroupArray[groupIndex].ageClassSize;

		bm->RBCestimation.RBCspeciesParam[groupIndex][HistYrMin_id] = bm->RBCestimation.HistYrMin;
		bm->RBCestimation.RBCspeciesParam[groupIndex][HistYrMax_id] = bm->RBCestimation.HistYrMax;
		nPast = (int)bm->RBCestimation.RBCspeciesParam[groupIndex][HistYrMax_id] - (int)bm->RBCestimation.RBCspeciesParam[groupIndex][HistYrMin_id] + 1;
		nYears = nFuture + nPast;
        if (nYears > maxNYears)
            maxNYears = nYears;
        
        bm->RBCestimation.RBCspeciesParam[groupIndex][Nyears_id] = nYears;
		bm->RBCestimation.RBCspeciesParam[groupIndex][MaxYr_id] = nYears;
        bm->RBCestimation.RBCspeciesParam[groupIndex][AccumAge_id] = bm->RBCestimation.RBCspeciesParam[groupIndex][MaxAge_id] + ceil(bm->RBCestimation.RBCspeciesParam[groupIndex][MaxAge_id] / 4.0); // accumulator age used in SS
        AccumAge = (int)bm->RBCestimation.RBCspeciesParam[groupIndex][AccumAge_id];

		/* Model structure */
		bm->RBCestimation.RBCspeciesArray[groupIndex].Ia = Util_Alloc_Init_2D_Int(bm->K_num_reg, bm->K_num_fisheries, 0);
		bm->RBCestimation.RBCspeciesArray[groupIndex].Future_catchprop = Util_Alloc_Init_1D_Double(bm->K_num_reg, 0.0);
        
		/* Biological parameters */
		
		/* Should be number of stock but in the SS3 info code the number of stock is always 1 */
		bm->RBCestimation.RBCspeciesArray[groupIndex].CvLA0 = Util_Alloc_Init_2D_Double(bm->K_num_sexes, bm->K_num_reg, 0.0);
        bm->RBCestimation.RBCspeciesArray[groupIndex].CvLAmax = Util_Alloc_Init_2D_Double(bm->K_num_sexes, bm->K_num_reg, 0.0);
		bm->RBCestimation.RBCspeciesArray[groupIndex].Fecund = Util_Alloc_Init_2D_Double(bm->RBCestimation.OverallMaxAge, bm->K_num_stocks_per_sp, 0.0);
        
        bm->RBCestimation.RBCspeciesArray[groupIndex].MeanLenAge =  Util_Alloc_Init_3D_Double(bm->RBCestimation.OverallMaxAge, bm->K_num_sexes, bm->K_num_stocks_per_sp, 0.0);
		bm->RBCestimation.RBCspeciesArray[groupIndex].MeanWtAge = Util_Alloc_Init_4D_Double(nYears+2, bm->RBCestimation.OverallMaxAge, bm->K_num_sexes, bm->K_num_stocks_per_sp, 0.0);
		bm->RBCestimation.RBCspeciesArray[groupIndex].Mzero = Util_Alloc_Init_3D_Double(bm->RBCestimation.OverallMaxAge, bm->K_num_sexes, bm->K_num_stocks_per_sp, 0.0);
		bm->RBCestimation.RBCspeciesArray[groupIndex].Mnat = Util_Alloc_Init_5D_Double(nYears, bm->RBCestimation.OverallMaxAge, bm->K_num_sexes, bm->K_num_reg, bm->K_num_stocks_per_sp, 0.0);
		bm->RBCestimation.RBCspeciesArray[groupIndex].WtLen = Util_Alloc_Init_3D_Double((int) bm->RBCestimation.RBCspeciesParam[groupIndex][Nlen_id], bm->K_num_sexes, bm->K_num_stocks_per_sp, 0.0);
		bm->RBCestimation.RBCspeciesArray[groupIndex].Wtlen_a = Util_Alloc_Init_2D_Double(bm->K_num_sexes, bm->K_num_stocks_per_sp, 0.0);
		bm->RBCestimation.RBCspeciesArray[groupIndex].Wtlen_b = Util_Alloc_Init_2D_Double(bm->K_num_sexes, bm->K_num_stocks_per_sp, 0.0);

		bm->RBCestimation.RBCspeciesArray[groupIndex].VBLinf = Util_Alloc_Init_2D_Double(bm->K_num_sexes, bm->K_num_stocks_per_sp, 0.0);  // Should it actually be this to be general?
		bm->RBCestimation.RBCspeciesArray[groupIndex].VBk = Util_Alloc_Init_2D_Double(bm->K_num_sexes, bm->K_num_stocks_per_sp, 0.0);
		bm->RBCestimation.RBCspeciesArray[groupIndex].VBt0 = Util_Alloc_Init_2D_Double(bm->K_num_sexes, bm->K_num_stocks_per_sp, 0.0);
		bm->RBCestimation.RBCspeciesArray[groupIndex].Xmove = Util_Alloc_Init_5D_Double(bm->RBCestimation.OverallMaxAge, bm->K_num_sexes, bm->K_num_reg, bm->K_num_reg, bm->K_num_stocks_per_sp, 0.0);

        /* Exploitation-related quantities */
        bm->RBCestimation.RBCspeciesArray[groupIndex].CatchCV = Util_Alloc_Init_1D_Double(bm->K_num_fisheries, 0.0);
        bm->RBCestimation.RBCspeciesArray[groupIndex].CatchData = Util_Alloc_Init_3D_Double(nYears, bm->K_num_reg+1, bm->K_num_fisheries, 0.0);  // +1 so have sum over all regions
        
        fprintf(bm->logFile,"%s using nYears: %d regions: %d fisheries: %d\n", FunctGroupArray[groupIndex].groupCode, nYears, bm->K_num_reg+1, bm->K_num_fisheries);
                
        bm->RBCestimation.RBCspeciesArray[groupIndex].EffortCV = Util_Alloc_Init_1D_Double(bm->K_num_fisheries, 0.0);
        bm->RBCestimation.RBCspeciesArray[groupIndex].EffortData = Util_Alloc_Init_3D_Double(nYears, bm->K_num_reg+1, bm->K_num_fisheries, 0.0);  // +1 so have sum over all regions
        bm->RBCestimation.RBCspeciesArray[groupIndex].EnviroData = Util_Alloc_Init_2D_Double(nYears, bm->K_num_reg+1, 0.0);  // +1 so have sum over all regions

		bm->RBCestimation.RBCspeciesArray[groupIndex].Uage = Util_Alloc_Init_5D_Double(nYears, bm->RBCestimation.OverallMaxAge, bm->K_num_sexes, bm->K_num_reg, bm->K_num_stocks_per_sp, 0.0);
		bm->RBCestimation.RBCspeciesArray[groupIndex].Ufleet = Util_Alloc_Init_3D_Double(nYears, bm->K_num_reg, bm->K_num_fisheries, 0.0);

        /* Selectivity */
		bm->RBCestimation.RBCspeciesArray[groupIndex].LoLenBin = Util_Alloc_Init_1D_Double(nLen, 0.0);

		/* Retention */
		bm->RBCestimation.RBCspeciesArray[groupIndex].DiscardType = Util_Alloc_Init_1D_Int(nLen, 0);
		bm->RBCestimation.RBCspeciesArray[groupIndex].LenRetain = Util_Alloc_Init_3D_Double(nYears, nLen, bm->K_num_fisheries, 0.0);

		/* Data generation */
        bm->RBCestimation.RBCspeciesArray[groupIndex].AgeFuture = Util_Alloc_Init_1D_Int(3, 0);          /* Flag for  future age  data (retained, whole, discard) */
        bm->RBCestimation.RBCspeciesArray[groupIndex].AgeN = Util_Alloc_Init_1D_Int(3, 0);               /* Sample size of age comps  (retained, whole, discard) */
		bm->RBCestimation.RBCspeciesArray[groupIndex].AgeYears = Util_Alloc_Init_2D_Int(3, nYears, 0);          /* Flags which years for which to generate age comps (retained, whole, discard) */
		bm->RBCestimation.RBCspeciesArray[groupIndex].AgeFltYr = Util_Alloc_Init_3D_Int(nYears, bm->K_num_fisheries, 3, 0);         /* Flags AF years, fleets in past (r,w,d) */
		bm->RBCestimation.RBCspeciesArray[groupIndex].AgeComp = Util_Alloc_Init_5D_Int(bm->RBCestimation.OverallMaxAge, 4, nYears, bm->K_num_sexes, bm->K_num_fisheries, 0);        /* Generated age composition data */
		bm->RBCestimation.RBCspeciesArray[groupIndex].AFss = Util_Alloc_Init_4D_Int(4, nYears, bm->K_num_sexes, bm->K_num_fisheries, 0);
        
        bm->RBCestimation.RBCspeciesArray[groupIndex].PGMSY_q = Util_Alloc_Init_1D_Double(bm->K_num_fisheries, 0);
        bm->RBCestimation.RBCspeciesArray[groupIndex].PGMSY_selcurve = Util_Alloc_Init_1D_Double(bm->K_num_fisheries, 0);
        bm->RBCestimation.RBCspeciesArray[groupIndex].PGMSY_sel_lsm = Util_Alloc_Init_1D_Double(bm->K_num_fisheries, 0);
        bm->RBCestimation.RBCspeciesArray[groupIndex].PGMSY_sel_sigma = Util_Alloc_Init_1D_Double(bm->K_num_fisheries, 0);
        bm->RBCestimation.RBCspeciesArray[groupIndex].Fhist = Util_Alloc_Init_2D_Double(nYears, bm->K_num_fisheries, 0.0);
        
        bm->RBCestimation.RBCspeciesArray[groupIndex].CPUEfuture = Util_Alloc_Init_1D_Int(bm->K_num_fisheries, 0);         /* Flag for future CPUE by fleet */
        
        bm->RBCestimation.RBCspeciesArray[groupIndex].CPUEyears = Util_Alloc_Init_3D_Int(nYears, bm->K_num_reg, bm->K_num_fisheries, 0);    /* Flags which years (by fleet, region) for which to generate cpue */
        bm->RBCestimation.RBCspeciesArray[groupIndex].CPUEqmu = Util_Alloc_Init_1D_Double(bm->K_num_fisheries, 0.0);
		bm->RBCestimation.RBCspeciesArray[groupIndex].CPUEcv = Util_Alloc_Init_1D_Double(bm->K_num_fisheries, 0.0);
		bm->RBCestimation.RBCspeciesArray[groupIndex].CPUEvar = Util_Alloc_Init_1D_Double(bm->K_num_fisheries, 0.0);
		bm->RBCestimation.RBCspeciesArray[groupIndex].CPUEcorr = Util_Alloc_Init_1D_Double(bm->K_num_fisheries, 0.0);
		bm->RBCestimation.RBCspeciesArray[groupIndex].CPUEgen = Util_Alloc_Init_3D_Double(nYears, bm->K_num_reg+1, bm->K_num_fisheries, 0.0);
		bm->RBCestimation.RBCspeciesArray[groupIndex].CPUEdevs = Util_Alloc_Init_3D_Double(nYears, bm->K_num_reg, bm->K_num_fisheries, 0.0);
		bm->RBCestimation.RBCspeciesArray[groupIndex].CPUEpow = Util_Alloc_Init_1D_Double(bm->K_num_fisheries, 0.0);
		bm->RBCestimation.RBCspeciesArray[groupIndex].CPUEq = Util_Alloc_Init_3D_Double(nYears, bm->K_num_reg, bm->K_num_fisheries, 0.0);
        bm->RBCestimation.RBCspeciesArray[groupIndex].DiscFuture = Util_Alloc_Init_1D_Int(bm->K_num_fisheries, 0);	     /* Flag for future discard by fleet */
        
        bm->RBCestimation.RBCspeciesArray[groupIndex].TotCatch = Util_Alloc_Init_3D_Double(nYears, bm->K_num_reg, bm->K_num_fisheries, 0.0);
		bm->RBCestimation.RBCspeciesArray[groupIndex].DiscData = Util_Alloc_Init_3D_Double(nYears, bm->K_num_reg+1, bm->K_num_fisheries, 0.0);  // +1 so can have sum over all regions
		bm->RBCestimation.RBCspeciesArray[groupIndex].DiscCV = Util_Alloc_Init_1D_Double(bm->K_num_fisheries, 0.0);
		bm->RBCestimation.RBCspeciesArray[groupIndex].DiscYears = Util_Alloc_Init_2D_Int(nYears, bm->K_num_fisheries, 0);		/* Flags which years (by fleet) for which to generate discards to feed to/from SS3 */
		
        bm->RBCestimation.RBCspeciesArray[groupIndex].LengthFuture = Util_Alloc_Init_1D_Int(4, 0);       /* Flag for future length data (retained, whole, discard) */
        bm->RBCestimation.RBCspeciesArray[groupIndex].LengthN = Util_Alloc_Init_1D_Int(4, 0);					  /* Sample size of length comps  (retained, whole, discard) */
		bm->RBCestimation.RBCspeciesArray[groupIndex].LengthYears = Util_Alloc_Init_2D_Int(nYears, 4, 0);
		bm->RBCestimation.RBCspeciesArray[groupIndex].LengthFltYr = Util_Alloc_Init_3D_Int(nYears, bm->K_num_fisheries, 4, 0);
		bm->RBCestimation.RBCspeciesArray[groupIndex].LenComp = Util_Alloc_Init_5D_Int(nLen, 4, nYears, bm->K_num_sexes, bm->K_num_fisheries, 0);        /* Generated length composition data */
		bm->RBCestimation.RBCspeciesArray[groupIndex].LFss = Util_Alloc_Init_4D_Int(4, nYears, bm->K_num_sexes, bm->K_num_fisheries, 0);
        
        bm->RBCestimation.RBCspeciesArray[groupIndex].mirrored_fleet = Util_Alloc_Init_1D_Int(bm->K_num_fisheries, 0);

		/* Assessment specifications */
		bm->RBCestimation.RBCspeciesArray[groupIndex].Ageing_error = Util_Alloc_Init_1D_Double(AccumAge+1, 0.0);     /* Aging error matrix. dimensions 0..AccumAge (SS data file) */
		bm->RBCestimation.RBCspeciesArray[groupIndex].SSMort = Util_Alloc_Init_2D_Double(bm->K_num_sexes, bm->K_num_stocks_per_sp, 0.0);

		bm->RBCestimation.RBCspeciesArray[groupIndex].Start_SelInflect = Util_Alloc_Init_1D_Double(bm->K_num_fisheries, 0.0);
		bm->RBCestimation.RBCspeciesArray[groupIndex].Start_SelWidth = Util_Alloc_Init_1D_Double(bm->K_num_fisheries, 0.0);
		bm->RBCestimation.RBCspeciesArray[groupIndex].Start_RetInflect = Util_Alloc_Init_1D_Double(bm->K_num_fisheries, 0.0);
		bm->RBCestimation.RBCspeciesArray[groupIndex].Start_RetSlope = Util_Alloc_Init_1D_Double(bm->K_num_fisheries, 0.0);
        
        bm->RBCestimation.RBCspeciesArray[groupIndex].Sel_Pattern = Util_Alloc_Init_1D_Int(bm->K_num_fisheries, 0);

		bm->RBCestimation.RBCspeciesArray[groupIndex].Tier3_Linf = Util_Alloc_Init_1D_Double(bm->K_num_sexes, 0.0);       /* Linf input for tier 3 (from table 1 in Neil's tier 3 document) */
		bm->RBCestimation.RBCspeciesArray[groupIndex].Tier3_k = Util_Alloc_Init_1D_Double(bm->K_num_sexes, 0.0);          /* k input for tier 3 */
		bm->RBCestimation.RBCspeciesArray[groupIndex].Tier3_t0 = Util_Alloc_Init_1D_Double(bm->K_num_sexes, 0.0);         /* t0 input for tier 3 */
		bm->RBCestimation.RBCspeciesArray[groupIndex].Tier5_Bstart = Util_Alloc_Init_1D_Double(4, 0.0);     /* Initial B for surplus production model */
		bm->RBCestimation.RBCspeciesArray[groupIndex].Tier5_r = Util_Alloc_Init_1D_Double(4, 0.0);          /* Initial r (and bounds) for surplus production model */
		bm->RBCestimation.RBCspeciesArray[groupIndex].T5_removal = Util_Alloc_Init_1D_Double(nYears, 0.0);       /* Total removals by year */
		bm->RBCestimation.RBCspeciesArray[groupIndex].TAC_mult = Util_Alloc_Init_1D_Double(nYears, 0.0); 		  /* Multiplier on the TAC - vector over future years */

		bm->RBCestimation.RBCspeciesArray[groupIndex].Varadj_CPUE = Util_Alloc_Init_1D_Double(bm->K_num_fisheries, 0.0);      /* Variance adjustment for CPUE (SS control file) */
		bm->RBCestimation.RBCspeciesArray[groupIndex].Varadj_discard = Util_Alloc_Init_1D_Double(bm->K_num_fisheries, 0.0);   /* Variance adjustment for discards (SS control file) */
		bm->RBCestimation.RBCspeciesArray[groupIndex].Varadj_length = Util_Alloc_Init_1D_Double(bm->K_num_fisheries, 0.0);    /* Variance adjustment for length (SS control file) */
		bm->RBCestimation.RBCspeciesArray[groupIndex].Varadj_age = Util_Alloc_Init_1D_Double(bm->K_num_fisheries, 0.0);       /* Variance adjustment for age (SS control file) */

		/* Derived quantities */
		sp_maxAge = (int)(bm->RBCestimation.RBCspeciesParam[groupIndex][MaxAge_id]);
		//bm->RBCestimation.RBCspeciesArray[groupIndex].Num = Util_Alloc_Init_5D_Double(nYears, sp_maxAge, bm->K_num_sexes, bm->K_num_reg, bm->K_num_stocks_per_sp, 0);         // Number of animals stock j, sex s, age a, region r

		bm->RBCestimation.RBCspeciesArray[groupIndex].RetVulBio = Util_Alloc_Init_3D_Double(nYears, bm->K_num_reg, bm->K_num_fisheries, 0.0); /* Retainable vulnerable biomass by fleet,region,time */

		bm->RBCestimation.RBCspeciesArray[groupIndex].SBioZero_shift = Util_Alloc_Init_2D_Double(bm->K_num_reg+1, bm->K_num_stocks_per_sp+1, 0.0);// +1 for region and stock so can keep overall values
		bm->RBCestimation.RBCspeciesArray[groupIndex].SBioZero = Util_Alloc_Init_2D_Double(bm->K_num_reg+1, bm->K_num_stocks_per_sp+1, 0.0);// +1 for region and stock so can keep overall values
		bm->RBCestimation.RBCspeciesArray[groupIndex].SpawnBio = Util_Alloc_Init_3D_Double(nYears, bm->K_num_reg+1, bm->K_num_stocks_per_sp+1, 0.0); // +1 for region and stock so can keep overall values
        
        /* HSF implementation */
		sp_maxAge = (int) bm->RBCestimation.RBCspeciesParam[groupIndex][Tier3_maxage_id];
		sp_cCsel = (int) bm->RBCestimation.RBCspeciesParam[groupIndex][CCsel_years_id];
		bm->RBCestimation.RBCspeciesArray[groupIndex].Obsageprop = Util_Alloc_Init_2D_Double(sp_maxAge, sp_cCsel, 0.0);      /* Global used to pass information to minimisation routine in Tier 3 calculations */
        
        bm->RBCestimation.RBCspeciesArray[groupIndex].TriggerPoints = Util_Alloc_Init_2D_Double(bm->K_num_reg+1, Ntriggers, 0.0);
        bm->RBCestimation.RBCspeciesArray[groupIndex].TriggerReached = Util_Alloc_Init_1D_Int(bm->K_num_reg+1, 0);
        bm->RBCestimation.RBCspeciesArray[groupIndex].AnnCPUE = Util_Alloc_Init_1D_Double(5, 0.0);  // As 5 years for the regression
        

        /* No longer needed as not using wayte population dynamics */
        // Recruitment
		//bm->RBCestimation.RBCspeciesArray[groupIndex].SigmaR = Util_Alloc_Init_1D_Double(nYears, 0.0);         // sigmaR at time t
        //bm->RBCestimation.RBCspeciesArray[groupIndex].CorRecDevs = Util_Alloc_Init_2D_Double(bm->K_num_reg, bm->K_num_reg, 0.0);     // Correlation of rec resids among regions
		//bm->RBCestimation.RBCspeciesArray[groupIndex].RecDevs = Util_Alloc_Init_2D_Double(nYears, bm->K_num_reg, 0.0);       // Recruitment deviations by region and time
		//bm->RBCestimation.RBCspeciesArray[groupIndex].Transition = Util_Alloc_Init_2D_Int(4, 4, 0.0);  		// Rec dev transition matrix for blue grenadier
		//bm->RBCestimation.RBCspeciesArray[groupIndex].TransCol = Util_Alloc_Init_1D_Int(4, 0.0);       		// Transition matrix column sums

		//bm->RBCestimation.RBCspeciesArray[groupIndex].MidWtAge = Util_Alloc_Init_4D_Double(nYears+2, bm->RBCestimation.OverallMaxAge, bm->K_num_sexes, bm->K_num_stocks_per_sp, 0.0);
		//bm->RBCestimation.RBCspeciesArray[groupIndex].Movrate = Util_Alloc_Init_3D_Double(bm->RBCestimation.OverallMaxAge, bm->K_num_sexes, bm->K_num_stocks_per_sp, 0.0);
		//bm->RBCestimation.RBCspeciesArray[groupIndex].SigmaLenAge = Util_Alloc_Init_4D_Double(nYears+2, bm->RBCestimation.OverallMaxAge, bm->K_num_sexes, bm->K_num_stocks_per_sp, 0.0);

        //bm->RBCestimation.RBCspeciesArray[groupIndex].MoveProb
		//bm->RBCestimation.RBCspeciesArray[groupIndex].Xsa = Util_Alloc_Init_4D_Double(bm->K_num_reg, bm->K_num_reg, bm->RBCestimation.OverallMaxAge, bm->K_num_sexes, 0.0);
		//bm->RBCestimation.RBCspeciesArray[groupIndex].Calc_catch = Util_Alloc_Init_3D_Double(nYears, bm->K_num_reg+1, bm->K_num_fisheries, 0.0);  // +1 so have sum over all regions
		//bm->RBCestimation.RBCspeciesArray[groupIndex].RetCatch = Util_Alloc_Init_3D_Double(nYears, bm->K_num_reg, bm->K_num_fisheries, 0.0);

		//bm->RBCestimation.RBCspeciesArray[groupIndex].CorSel = Util_Alloc_Init_1D_Double(bm->K_num_fisheries, 0.0);
		//bm->RBCestimation.RBCspeciesArray[groupIndex].CorDevSel = Util_Alloc_Init_3D_Double(nLen, nLen, bm->K_num_fisheries, 0.0);
		//bm->RBCestimation.RBCspeciesArray[groupIndex].DevSel = Util_Alloc_Init_3D_Double(nYears, nLen, bm->K_num_fisheries, 0.0);
		//bm->RBCestimation.RBCspeciesArray[groupIndex].FracLen = Util_Alloc_Init_5D_Double(nLen, nYears, bm->RBCestimation.OverallMaxAge, bm->K_num_sexes, bm->K_num_stocks_per_sp, 0.0);

		//bm->RBCestimation.RBCspeciesArray[groupIndex].SelAge = Util_Alloc_Init_5D_Double(nYears, bm->RBCestimation.OverallMaxAge, bm->K_num_sexes, bm->K_num_stocks_per_sp, bm->K_num_fisheries, 0.0);
		//bm->RBCestimation.RBCspeciesArray[groupIndex].SelLen = Util_Alloc_Init_3D_Double(nYears, nLen, bm->K_num_fisheries, 0.0);
		//bm->RBCestimation.RBCspeciesArray[groupIndex].VarDevSel = Util_Alloc_Init_1D_Double(nLen, 0.0);

		//bm->RBCestimation.RBCspeciesArray[groupIndex].AgeRetain = Util_Alloc_Init_5D_Double(nYears, bm->RBCestimation.OverallMaxAge, bm->K_num_sexes, bm->K_num_stocks_per_sp, bm->K_num_fisheries, 0.0);
		
        //bm->RBCestimation.RBCspeciesArray[groupIndex].RetGamma = Util_Alloc_Init_3D_Double(3, nYears, bm->K_num_fisheries, 0.0);
        
		// Recruitment
		//bm->RBCestimation.RBCspeciesArray[groupIndex].FracRec = Util_Alloc_Init_3D_Double(nYears, bm->K_num_reg, bm->K_num_reg, 0.0);       /* Expected fraction of age0's by stock assigned to regions through time */
		//bm->RBCestimation.RBCspeciesArray[groupIndex].HaveRecDev = Util_Alloc_Init_2D_Int(nYears, bm->K_num_reg, 0.0);    	/* Flag if have recruitment dev this year (1=y,0=n) */
        
		//bm->RBCestimation.RBCspeciesArray[groupIndex].Hsteep = Util_Alloc_Init_1D_Double(bm->K_num_stocks_per_sp, 0.0);         /* Steepness */
		//bm->RBCestimation.RBCspeciesArray[groupIndex].Recruits = Util_Alloc_Init_3D_Double(nYears, bm->K_num_reg, bm->K_num_stocks_per_sp, 0.0);     /* No of recuits by stock, area and time */
		//bm->RBCestimation.RBCspeciesArray[groupIndex].R0 = Util_Alloc_Init_1D_Double(bm->K_num_stocks_per_sp, 0.0);             /* Unfished recruitment by stock */
		//bm->RBCestimation.RBCspeciesArray[groupIndex].Rshift = Util_Alloc_Init_1D_Double(bm->K_num_stocks_per_sp, 0.0);         /* Equilibrium recruitment at regime shift */
        
        //SB0 = Util_Alloc_Init_1D_Double(nLen, 0.0);          /* Pre-exploitation spawning biomass - commented out by Sally */
		//bm->RBCestimation.RBCspeciesArray[groupIndex].VarRecDevs = Util_Alloc_Init_3D_Double(nYears, bm->K_num_reg, bm->K_num_reg, 0.0);    /* Variance-covariance matrix of the recruitment devs */
        
        //bm->RBCestimation.RBCspeciesArray[groupIndex].GrangerM = Util_Alloc_Init_2D_Double(500, 500, 0.0);

        //bm->RBCestimation.RBCspeciesArray[groupIndex].RetVulBio_byall = Util_Alloc_Init_6D_Int(nYears, bm->RBCestimation.OverallMaxAge, bm->K_num_sexes, bm->K_num_fisheries, bm->K_num_reg, bm->K_num_stocks_per_sp, 0);  /* Retainable vulnerable biomass by stock, region, fleet, sex, age, time */

        //bm->RBCestimation.RBCspeciesArray[groupIndex].LenBin = Util_Alloc_Init_1D_Double((int)bm->RBCestimation.RBCspeciesParam[groupIndex][Nlen_id], 0.0);
	    //bm->RBCestimation.RBCspeciesArray[groupIndex].Lambda0 = Util_Alloc_Init_2D_Double(bm->K_num_reg, bm->K_num_stocks_per_sp, 0.0);

        
		//bm->RBCestimation.RBCspeciesArray[groupIndex].VulBio = Util_Alloc_Init_3D_Double(nYears, bm->K_num_reg, bm->K_num_fisheries, 0.0);        /* Vulnerable biomass by fleet, region, time */
        
        bm->CloseKinEst[groupIndex].fec_expo_s = Util_Alloc_Init_1D_Double(bm->RBCestimation.RBCspeciesParam[groupIndex][Nsexes_id], 1.0);
        bm->CloseKinEst[groupIndex].fec_sa = Util_Alloc_Init_2D_Double(bm->RBCestimation.RBCspeciesParam[groupIndex][MaxAge_id], bm->RBCestimation.RBCspeciesParam[groupIndex][Nsexes_id], 0.0);
        bm->CloseKinEst[groupIndex].mature = Util_Alloc_Init_1D_Double(bm->RBCestimation.RBCspeciesParam[groupIndex][Nlen_id], 0.0);
        bm->CloseKinEst[groupIndex].lengths = Util_Alloc_Init_1D_Double(bm->RBCestimation.RBCspeciesParam[groupIndex][Nlen_id], 0.0);
        bm->CloseKinEst[groupIndex].samp_prop_to = Util_Alloc_Init_3D_Double(bm->RBCestimation.RBCspeciesParam[groupIndex][MaxAge_id], bm->CloseKinEst->last_sy, bm->RBCestimation.RBCspeciesParam[groupIndex][Nsexes_id], 0.0);
        
        bm->RBCestimation.RBCspeciesArray[groupIndex].SlopeMat = Util_Alloc_Init_1D_Double(FunctGroupArray[groupIndex].numStocks, 0.0);
        bm->RBCestimation.RBCspeciesArray[groupIndex].Mat50 = Util_Alloc_Init_1D_Double(FunctGroupArray[groupIndex].numStocks, 0.0);
        bm->RBCestimation.RBCspeciesArray[groupIndex].HiLenBin = Util_Alloc_Init_1D_Double(nLen, 0.0);
        bm->RBCestimation.RBCspeciesArray[groupIndex].FracLenS = Util_Alloc_Init_5D_Double(nLen, nYears, bm->RBCestimation.RBCspeciesParam[groupIndex][MaxAge_id], bm->RBCestimation.RBCspeciesParam[groupIndex][Nsexes_id], bm->K_num_stocks_per_sp, 0.0);
        // Used? bm->RBCestimation.RBCspeciesArray[groupIndex].FracLenM = Util_Alloc_Init_4D_Double(nYears, bm->RBCestimation.RBCspeciesParam[groupIndex][MaxAge_id], bm->RBCestimation.RBCspeciesParam[groupIndex][Nsexes_id], bm->K_num_stocks_per_sp, 0.0);
        bm->RBCestimation.RBCspeciesArray[groupIndex].MeanLenAgeM = Util_Alloc_Init_4D_Double(nYears, bm->RBCestimation.RBCspeciesParam[groupIndex][MaxAge_id], bm->RBCestimation.RBCspeciesParam[groupIndex][Nsexes_id], bm->K_num_stocks_per_sp, 0.0);
        bm->RBCestimation.RBCspeciesArray[groupIndex].SigmaLenAgeM = Util_Alloc_Init_4D_Double(nYears, bm->RBCestimation.RBCspeciesParam[groupIndex][MaxAge_id], bm->RBCestimation.RBCspeciesParam[groupIndex][Nsexes_id], bm->K_num_stocks_per_sp, 0.0);
        bm->RBCestimation.RBCspeciesArray[groupIndex].MeanWtAgeS = Util_Alloc_Init_4D_Double(nYears, bm->RBCestimation.RBCspeciesParam[groupIndex][MaxAge_id], bm->RBCestimation.RBCspeciesParam[groupIndex][Nsexes_id], bm->K_num_stocks_per_sp, 0.0);
        bm->RBCestimation.RBCspeciesArray[groupIndex].MeanWtAgeM = Util_Alloc_Init_4D_Double(nYears, bm->RBCestimation.RBCspeciesParam[groupIndex][MaxAge_id], bm->RBCestimation.RBCspeciesParam[groupIndex][Nsexes_id], bm->K_num_stocks_per_sp, 0.0);
        bm->RBCestimation.RBCspeciesArray[groupIndex].MeanLenAgeS = Util_Alloc_Init_4D_Double(nYears, bm->RBCestimation.RBCspeciesParam[groupIndex][MaxAge_id], bm->RBCestimation.RBCspeciesParam[groupIndex][Nsexes_id], bm->K_num_stocks_per_sp, 0.0);

        bm->RBCestimation.RBCspeciesArray[groupIndex].NblockperPatt = Util_Alloc_Init_1D_Int(bm->RBCestimation.RBCspeciesParam[groupIndex][NblockPattern_id], 0);
        bm->RBCestimation.RBCspeciesArray[groupIndex].Blocks = Util_Alloc_Init_2D_Double(bm->K_max_BlockPattern, bm->RBCestimation.RBCspeciesParam[groupIndex][NblockPattern_id], 0.0);
        
        bm->RBCestimation.RBCspeciesArray[groupIndex].discfleet = Util_Alloc_Init_1D_Int(bm->K_num_fisheries, 0);
        bm->RBCestimation.RBCspeciesArray[groupIndex].initialEquilCatch = Util_Alloc_Init_1D_Double(bm->K_num_fisheries, 0.0);
        bm->RBCestimation.RBCspeciesArray[groupIndex].R02 = Util_Alloc_Init_1D_Double(Nregions, 0.0);
        bm->RBCestimation.RBCspeciesArray[groupIndex].SSHsteep = Util_Alloc_Init_1D_Double(Nregions, 0.0);
        bm->RBCestimation.RBCspeciesArray[groupIndex].Start_Sel = Util_Alloc_Init_2D_Double(bm->K_num_tot_sp, bm->K_num_fisheries, 0.0);
        bm->RBCestimation.RBCspeciesArray[groupIndex].SelBlock = Util_Alloc_Init_2D_Double(bm->K_num_tot_sp, bm->K_num_fisheries, 0.0);
        bm->RBCestimation.RBCspeciesArray[groupIndex].Sel_Phase = Util_Alloc_Init_2D_Double(bm->K_num_tot_sp, bm->K_num_fisheries, 0.0);
        bm->RBCestimation.RBCspeciesArray[groupIndex].RBC_by_year = Util_Alloc_Init_1D_Double(nYears, 0.0);
        
        bm->RBCestimation.RBCspeciesArray[groupIndex].Mortality = Util_Alloc_Init_1D_Double(2, 0.0);
                
        bm->RBCestimation.RBCspeciesArray[groupIndex].Fupdated = Util_Alloc_Init_2D_Double(nYears, bm->K_num_fisheries, 0.0);
        bm->RBCestimation.RBCspeciesArray[groupIndex].RBCupdated = Util_Alloc_Init_2D_Double(nYears, bm->K_num_fisheries, 0.0);
        bm->RBCestimation.RBCspeciesArray[groupIndex].AvgCatFleet = Util_Alloc_Init_2D_Double(nYears, bm->K_num_fisheries, 0.0);

        bm->RBCestimation.RBCspeciesArray[groupIndex].CatchStore = Util_Alloc_Init_2D_Double(nYears, bm->K_num_fisheries, 0.0);
        bm->RBCestimation.RBCspeciesArray[groupIndex].CscalarMetier = Util_Alloc_Init_1D_Double(bm->K_num_fisheries, 0.0);
        bm->RBCestimation.RBCspeciesArray[groupIndex].FFs = Util_Alloc_Init_1D_Double(bm->RBCestimation.ProjYr, 0.0);
        
	}
    
    bm->RBCestimation.MULTISPestIDs = Util_Alloc_Init_1D_Int(bm->K_num_tot_sp, 0);
    bm->RBCestimation.Catch_by_Metier = Util_Alloc_Init_4D_Double(bm->K_num_fisheries, maxNYears, maxNYears, bm->K_num_tot_sp, 0.0);

    printf("Completed Tier_Assessment_Allocate\n");
    
	return 0;
}

int AtLoadArrays(MSEBoxModel *bm, int groupIndex) {
    int Nlen = (int) bm->RBCestimation.RBCspeciesParam[groupIndex][Nlen_id];
    int maxA, maxIT;   // For the arrays that came from atManageTier.c
    int Nfleets = (int)(bm->RBCestimation.RBCspeciesParam[groupIndex][NumFisheries_id]);
    int tier = (int) (FunctGroupArray[groupIndex].speciesParams[tier_id]);
	int nFtier3 = (int)(1000 * bm->RBCestimation.RBCspeciesParam[groupIndex][Tier3_maxF_id]);
    
    if ((FunctGroupArray[groupIndex].speciesParams[tier_id] == 0) || (FunctGroupArray[groupIndex].speciesParams[assess_flag_id] == FALSE))
        return 0;

    /* From atManageTier.c and those arrays called in Tier_Assessment_Setup() */
    if (bm->RBCestimation.RBCspeciesParam[groupIndex][Tier3_maxage_id] < bm->RBCestimation.RBCspeciesParam[groupIndex][MaxAge_id])
        maxA = (int)bm->RBCestimation.RBCspeciesParam[groupIndex][MaxAge_id];
    else
        maxA = (int)bm->RBCestimation.RBCspeciesParam[groupIndex][Tier3_maxage_id];

    bm->RBCestimation.RBCspeciesArray[groupIndex].agesel = Util_Alloc_Init_1D_Double(maxA, 0.0);
    bm->RBCestimation.RBCspeciesArray[groupIndex].mat = Util_Alloc_Init_1D_Double(maxA, 0.0);
    bm->RBCestimation.RBCspeciesArray[groupIndex].wt = Util_Alloc_Init_1D_Double(maxA, 0.0);
    bm->RBCestimation.RBCspeciesArray[groupIndex].fvalA = Util_Alloc_Init_1D_Double(maxA, 0.0);
    bm->RBCestimation.RBCspeciesArray[groupIndex].ageref = Util_Alloc_Init_1D_Double(maxA, 0.0);
    bm->RBCestimation.RBCspeciesArray[groupIndex].len = Util_Alloc_Init_1D_Double(maxA, 0.0);
    bm->RBCestimation.RBCspeciesArray[groupIndex].p_al = Util_Alloc_Init_2D_Double(Nlen, maxA, 0.0);

    bm->RBCestimation.RBCspeciesArray[groupIndex].yield = Util_Alloc_Init_1D_Double(maxA, 0.0);
    bm->RBCestimation.RBCspeciesArray[groupIndex].yield_mid = Util_Alloc_Init_1D_Double(maxA, 0.0);
    bm->RBCestimation.RBCspeciesArray[groupIndex].num = Util_Alloc_Init_1D_Double(maxA, 0.0);

    maxIT = maxA;
    if ((tier == tier3) && (Nfleets * 5 > maxIT))
        maxIT = Nfleets * 5;    // absolute max size needed
    bm->RBCestimation.RBCspeciesArray[groupIndex].z = Util_Alloc_Init_1D_Double(maxIT, 0.0);
    
    bm->RBCestimation.RBCspeciesArray[groupIndex].T5_Fval = Util_Alloc_Init_1D_Double(nFtier3, 0.0);          /* Fvals for Tier 5 avlen method */
    bm->RBCestimation.RBCspeciesArray[groupIndex].T5_Avlen_atF = Util_Alloc_Init_1D_Double(nFtier3, 0.0);    /* Average length at F for Tier 5 avlen method */

    bm->RBCestimation.RBCspeciesArray[groupIndex].sumcatch = Util_Alloc_Init_1D_Double(Nfleets, 0.0);

    return 0;
}



int Tier_Assessment_PostLoad_Allocate(MSEBoxModel *bm) {
    
	int groupIndex, Ncohorts, nselage, Tier3maxage, maxage;
	int Nfleets, Nregions, Nstocks, Nlen, Nages, vecsize;  // For the arrays that came from atSS3DataGen.c
    int maxA, tier;   // For the arrays that came from atManageTier.c
	int nYears, npar;
	int nPast;
	int Nsexes = (bm->K_num_sexes);
    int indexsize = max(bm->RBCestimation.GradientWindow, bm->RBCestimation.TriggerCheckPeriod);
    
    bm->RBCestimation.catchwghtCPUE = Util_Alloc_Init_1D_Double(indexsize, 0.0);
    bm->RBCestimation.catchwght = Util_Alloc_Init_1D_Double(indexsize, 0.0);
    bm->RBCestimation.count_sp_in_buffer = Util_Alloc_Init_1D_Int(K_num_indicator_tiers, 0);
    
	/* Allocate the arrays used in the assessment itself */
	for (groupIndex = 0; groupIndex < bm->K_num_tot_sp; groupIndex++) {
        
        printf("Doing tier postload for %s\n", FunctGroupArray[groupIndex].groupCode);
        
        if ((FunctGroupArray[groupIndex].speciesParams[tier_id] == 0) || (FunctGroupArray[groupIndex].speciesParams[assess_flag_id] == FALSE)) // Not needed as not assessed
            continue;
        
		nPast = (int)(bm->RBCestimation.RBCspeciesParam[groupIndex][HistYrMax_id] - bm->RBCestimation.RBCspeciesParam[groupIndex][HistYrMin_id]) + 1;
		nYears = bm->RBCestimation.nFuture + nPast;
        
        /* Files from atSS3DataGen.c */
        Nfleets = (int)(bm->RBCestimation.RBCspeciesParam[groupIndex][NumFisheries_id]);
        Nregions = (int)(bm->RBCestimation.RBCspeciesParam[groupIndex][NumRegions_id]);
        Nstocks = FunctGroupArray[groupIndex].numStocks;
        Nlen = (int) bm->RBCestimation.RBCspeciesParam[groupIndex][Nlen_id];
        Nages = (int)(bm->RBCestimation.RBCspeciesParam[groupIndex][MaxAge_id] + 1);
        vecsize = Nfleets * Nregions * Nsexes;
        tier = (int) (FunctGroupArray[groupIndex].speciesParams[tier_id]);
        Ncohorts = FunctGroupArray[groupIndex].numCohortsXnumGenes;
        nselage = (int)(bm->RBCestimation.RBCspeciesParam[groupIndex][Tier3_maxage_id] - bm->RBCestimation.RBCspeciesParam[groupIndex][AgeSel95_id]);
        
        if (nselage < 0)
            nselage = 0;
        
        Tier3maxage = (int)(bm->RBCestimation.RBCspeciesParam[groupIndex][Tier3_maxage_id]);
        maxage = (int)(bm->RBCestimation.RBCspeciesParam[groupIndex][MaxAge_id]);
        bm->RBCestimation.RBCspeciesParam[groupIndex][Nfint_id] = 1000 * bm->RBCestimation.RBCspeciesParam[groupIndex][Tier3_maxF_id];
        
        bm->RBCestimation.RBCspeciesArray[groupIndex].propR = Util_Alloc_Init_1D_Double(Nregions, 0.0);
        bm->RBCestimation.RBCspeciesArray[groupIndex].propSR = Util_Alloc_Init_2D_Double(Nstocks, Nregions, 0.0);
        bm->RBCestimation.RBCspeciesArray[groupIndex].propCount = Util_Alloc_Init_1D_Double(Nstocks, 0.0);
        
        bm->RBCestimation.RBCspeciesArray[groupIndex].actualcatch = Util_Alloc_Init_2D_Double(Nregions, Nfleets, 0.0);
        bm->RBCestimation.RBCspeciesArray[groupIndex].actualeffort = Util_Alloc_Init_2D_Double(Nregions, Nfleets, 0.0);
        bm->RBCestimation.RBCspeciesArray[groupIndex].actualdiscard = Util_Alloc_Init_2D_Double(Nregions, Nfleets, 0.0);
        bm->RBCestimation.RBCspeciesArray[groupIndex].actualbiom = Util_Alloc_Init_3D_Double(FunctGroupArray[groupIndex].numCohorts, Nstocks, Nregions, 0.0);
        bm->RBCestimation.RBCspeciesArray[groupIndex].actualcatchsize = Util_Alloc_Init_3D_Double(FunctGroupArray[groupIndex].numCohorts, Nstocks, Nregions, 0.0);
        
        bm->RBCestimation.RBCspeciesArray[groupIndex].rawdata = Util_Alloc_Init_4D_Double(3, bm->K_num_sexes, Nregions, Nfleets, 0.0);   // So data type by sex by region by fleet
        bm->RBCestimation.RBCspeciesArray[groupIndex].rawsizedata = Util_Alloc_Init_5D_Double(3, Ncohorts, bm->K_num_sexes, Nregions, Nfleets, 0.0);   // So size data type by age by sex by region by fleet
        bm->RBCestimation.RBCspeciesArray[groupIndex].rawnumdata = Util_Alloc_Init_5D_Double(3, Ncohorts, bm->K_num_sexes, Nregions, Nfleets, 0.0);   // So matching numbers data type by age by sex by region by fleet
        bm->RBCestimation.RBCspeciesArray[groupIndex].ndata = Util_Alloc_Init_4D_Double(2, bm->K_num_sexes, Nregions, Nfleets, 0.0);  // 2 is because there is age and length data to be handled
        
        bm->RBCestimation.RBCspeciesArray[groupIndex].lenprops = Util_Alloc_Init_2D_Double(Nlen, Nstocks + 1, 0.0);
        bm->RBCestimation.RBCspeciesArray[groupIndex].lencomp = Util_Alloc_Init_1D_Double(Nlen, 0.0);
        bm->RBCestimation.RBCspeciesArray[groupIndex].ageprops = Util_Alloc_Init_2D_Double(Nages + 1, Nstocks + 1, 0.0);  // Plus group for .... ???
        bm->RBCestimation.RBCspeciesArray[groupIndex].agecomp = Util_Alloc_Init_1D_Double(Nages + 1, 0.0);
        
        if (bm->useGenMnomial) {
            bm->RBCestimation.RBCspeciesArray[groupIndex].propvec = Util_Alloc_Init_2D_Double(vecsize, 3, 0.0);   // One entry for each data type and then another dimension to cover whether age or length being done
            bm->RBCestimation.RBCspeciesArray[groupIndex].newvec = Util_Alloc_Init_3D_Double(vecsize, 2, 3, 0.0);
        }
        
        bm->RBCestimation.RBCspeciesArray[groupIndex].props = Util_Alloc_Init_1D_Double((FunctGroupArray[groupIndex].numCohortsXnumGenes), 0.0);
        
        if(Nsexes < 2) {
            bm->RBCestimation.RBCspeciesArray[groupIndex].CKSurveyNum = Util_Alloc_Init_4D_Double(nYears, Nages, 2, Nstocks, 0.0);
        } else {
            bm->RBCestimation.RBCspeciesArray[groupIndex].CKSurveyNum = Util_Alloc_Init_4D_Double(nYears, Nages, Nsexes, Nstocks, 0.0);
        }
        
        /* Files from atManageTier.c */
        bm->RBCestimation.RBCspeciesArray[groupIndex].propcatch = Util_Alloc_Init_2D_Double(nYears, Nfleets, 0.0);
        bm->RBCestimation.RBCspeciesArray[groupIndex].propcatch_reg = Util_Alloc_Init_3D_Double(nYears, Nregions, Nfleets, 0.0);
        bm->RBCestimation.RBCspeciesArray[groupIndex].avprop = Util_Alloc_Init_1D_Double(Nfleets, 0.0);
        bm->RBCestimation.RBCspeciesArray[groupIndex].avprop_reg = Util_Alloc_Init_2D_Double(Nregions, Nfleets, 0.0);
        
        fprintf(bm->logFile, "Initiaised propcatch, propcatch_reg, avprop and avprop_reg for %s with nYears: %d Nregions: %d Nfleets: %d\n", FunctGroupArray[groupIndex].groupCode, nYears, Nregions, Nfleets);
        
        bm->RBCestimation.RBCspeciesArray[groupIndex].discrate = Util_Alloc_Init_1D_Double(nYears, 0.0);
        
        bm->RBCestimation.RBCspeciesArray[groupIndex].agesel_bysex = Util_Alloc_Init_2D_Double(bm->K_num_sexes, FunctGroupArray[groupIndex].numCohorts, 0.0);
        
        if (tier == tier1)  // As only used in tier1 routines
            bm->RBCestimation.RBCspeciesArray[groupIndex].fval = Util_Alloc_Init_2D_Double(bm->K_num_sexes, FunctGroupArray[groupIndex].numCohorts, 0.0);
        
        if (Tier3maxage < maxage)
            maxA = maxage;
        else
            maxA = Tier3maxage;
        
        npar = 3;
        if (tier == tier3) {  // As only used in tier3 routines
            
            bm->RBCestimation.RBCspeciesArray[groupIndex].numatage = Util_Alloc_Init_1D_Double(nselage, 0.0);
            bm->RBCestimation.RBCspeciesArray[groupIndex].ages = Util_Alloc_Init_1D_Double(nselage, 0.0);
            bm->RBCestimation.RBCspeciesArray[groupIndex].propgt05 = Util_Alloc_Init_1D_Int(nselage, 0);
            bm->RBCestimation.RBCspeciesArray[groupIndex].totnumatage = Util_Alloc_Init_1D_Double(Tier3maxage, 0.0);
            bm->RBCestimation.RBCspeciesArray[groupIndex].fvalF = Util_Alloc_Init_1D_Double(Nfleets, 0.0);
            bm->RBCestimation.RBCspeciesArray[groupIndex].chi = Util_Alloc_Init_1D_Double(Nfleets, 0.0);
            bm->RBCestimation.RBCspeciesArray[groupIndex].sel = Util_Alloc_Init_2D_Double(Tier3maxage, Nfleets, 0.0);
        }
        
        if ((tier == tier3) || (tier == tier5)){  // As only used in tier3 routines
            bm->RBCestimation.RBCspeciesArray[groupIndex].xunit = Util_Alloc_Init_2D_Double(npar+1, npar+1, 0.0);
            bm->RBCestimation.RBCspeciesArray[groupIndex].xpar = Util_Alloc_Init_1D_Double(npar+1, 0.0);
            bm->RBCestimation.RBCspeciesArray[groupIndex].simplex = Util_Alloc_Init_2D_Double(npar+1, npar+1, 0.0);
            bm->RBCestimation.RBCspeciesArray[groupIndex].scale = Util_Alloc_Init_1D_Double(npar+1, 1.0);
            bm->RBCestimation.RBCspeciesArray[groupIndex].func = Util_Alloc_Init_1D_Double(npar+1, 0.0);
        }

        if ((tier == tier1) || (tier == tier3)) { // As only used in tier1 and tier3 routines
            bm->RBCestimation.RBCspeciesArray[groupIndex].exp_prop = Util_Alloc_Init_1D_Double(Tier3maxage, 0.0);
            bm->RBCestimation.RBCspeciesArray[groupIndex].Funkz = Util_Alloc_Init_1D_Double(maxA, 0.0);
        }

        if (tier == tier4) {  // As only used in tier4 routines
            bm->RBCestimation.RBCspeciesArray[groupIndex].xyear = Util_Alloc_Init_1D_Double(4, 0.0);
            bm->RBCestimation.RBCspeciesArray[groupIndex].ycpue = Util_Alloc_Init_1D_Double(4, 0.0);
        }

        if (tier == tier5) {  // As only used in tier5 routines
            bm->RBCestimation.RBCspeciesArray[groupIndex].expectB = Util_Alloc_Init_1D_Double(nYears, 0.0);
            bm->RBCestimation.RBCspeciesArray[groupIndex].expectq = Util_Alloc_Init_1D_Double(nYears, 0.0);
        }
        
        if (tier == tier7) {
            bm->RBCestimation.RBCspeciesArray[groupIndex].CatchComp = Util_Alloc_Init_2D_Double(nYears+2, bm->K_num_tot_sp, 0.0);
            bm->RBCestimation.RBCspeciesArray[groupIndex].relareafished = Util_Alloc_Init_2D_Double(nYears+2, bm->K_num_tot_sp, 0.0);
            bm->RBCestimation.RBCspeciesArray[groupIndex].cpue = Util_Alloc_Init_1D_Double(nYears+2, 0.0);
        }
        
        bm->RBCestimation.RBCspeciesArray[groupIndex].TSbiomass = Util_Alloc_Init_1D_Double(1000, 0.0);
        
        // Finalise array contents for use in tiered assessments
		SetUpBiology(bm, groupIndex);       // TODO: need to do this dynamically?   Needed here or in other spot?
        
        /* As getting recruitment from Atlantis I'm not sure we need this
         mint = (int)(bm->RBCestimation.RBCspeciesParam[groupIndex][HistYrMin_id]);
         maxt = (int)(bm->RBCestimation.RBCspeciesParam[groupIndex][MaxYr_id]);
         // set up recruitment deviations
         SetUpRecDevs(bm, groupIndex, mint, maxt);
         
         // calc std error of recruitment deviations (mean is 0)
         ssq = 0;
         sum = 0;
         mint = (int)(bm->RBCestimation.RBCspeciesParam[groupIndex][RecDevMinYr_id]);
         maxt = (int)(bm->RBCestimation.RBCspeciesParam[groupIndex][RecDevMaxYr_id]);
         for (t = mint; t < maxt; t++) {
         ssq += bm->RBCestimation.RBCspeciesArray[groupIndex].RecDevs[r][t] * bm->RBCestimation.RBCspeciesArray[groupIndex].RecDevs[r][t];
         sum += bm->RBCestimation.RBCspeciesArray[groupIndex].RecDevs[r][t];
         }
         bm->RBCestimation.RBCspeciesParam[groupIndex][RecDevStdev_id] = sqrt((ssq - sum * sum)/ ((bm->RBCestimation.RBCspeciesParam[groupIndex][RecDevMaxYr_id] - bm->RBCestimation.RBCspeciesParam[groupIndex][RecDevMinYr_id])));
         */
                
	}
    
    /* Deal wit close kin related arrays - leave in atCloseKin.c for now, may move here later
    if (bm->CloseKinEst->UseCloseKin) {
        nsamps_y = Util_Alloc_Init_1D_Double(nsampy, 0.0);
        nsamps_sya = Util_Alloc_Init_3D_Double(bm->RBCestimation.RBCspeciesParam[sp][Nsexes_id], nsampy, bm->RBCestimation.RBCspeciesParam[sp][MaxAge_id], 0.0);

        x = Util_Alloc_Init_1D_Double(bm->RBCestimation.RBCspeciesParam[sp][MaxAge_id], 0.0);
        sx = Util_Alloc_Init_1D_Double(bm->RBCestimation.RBCspeciesParam[sp][Nsexes_id], 0.0);
        inv_totfec_sy = Util_Alloc_Init_2D_Double(bm->RBCestimation.RBCspeciesParam[sp][Nsexes_id], last_dy, 0.0);

        n_sya = Util_Alloc_Init_3D_Double(bm->RBCestimation.RBCspeciesParam[sp][Nsexes_id], last_dy, bm->RBCestimation.RBCspeciesParam[sp][MaxAge_id], 0.0);
        psurv_syay = Util_Alloc_Init_4D_Double(bm->RBCestimation.RBCspeciesParam[sp][Nsexes_id], last_dy, bm->RBCestimation.RBCspeciesParam[sp][MaxAge_id], last_dy, 0.0);
        
        Pr_PO_syaya = Util_Alloc_Init_5D_Double(bm->RBCestimation.RBCspeciesParam[sp][Nsexes_id], bm->CloseKinEst[sp].last_sy, bm->RBCestimation.RBCspeciesParam[sp][MaxAge_id], bm->CloseKinEst[sp].last_sy, bm->RBCestimation.RBCspeciesParam[sp][MaxAge_id], 0.0);
        Pr_HS_syaya = Util_Alloc_Init_5D_Double(bm->RBCestimation.RBCspeciesParam[sp][Nsexes_id], bm->CloseKinEst[sp].last_sy, bm->RBCestimation.RBCspeciesParam[sp][MaxAge_id], bm->CloseKinEst[sp].last_sy, bm->RBCestimation.RBCspeciesParam[sp][MaxAge_id], 0.0);

        Pr_GG_yaya = Util_Alloc_Init_4D_Double(bm->CloseKinEst[sp].last_sy, bm->RBCestimation.RBCspeciesParam[sp][MaxAge_id], bm->CloseKinEst[sp].last_sy, bm->RBCestimation.RBCspeciesParam[sp][MaxAge_id], 0.0);
        Pr_HS_yaya = Util_Alloc_Init_4D_Double(bm->CloseKinEst[sp].last_sy, bm->RBCestimation.RBCspeciesParam[sp][MaxAge_id], bm->CloseKinEst[sp].last_sy, bm->RBCestimation.RBCspeciesParam[sp][MaxAge_id], 0.0);

        ncomps_PO_syaya = Util_Alloc_Init_5D_Double(bm->RBCestimation.RBCspeciesParam[sp][Nsexes_id], bm->CloseKinEst[sp].last_sy, bm->RBCestimation.RBCspeciesParam[sp][MaxAge_id], bm->CloseKinEst[sp].last_sy, bm->RBCestimation.RBCspeciesParam[sp][MaxAge_id], 0.0);
        ncomps_HS_yaya = Util_Alloc_Init_4D_Double(bm->CloseKinEst[sp].last_sy, bm->RBCestimation.RBCspeciesParam[sp][MaxAge_id], bm->CloseKinEst[sp].last_sy, bm->RBCestimation.RBCspeciesParam[sp][MaxAge_id], 0.0);
        
        Ekin_HS_syaya = Util_Alloc_Init_5D_Double(bm->RBCestimation.RBCspeciesParam[sp][Nsexes_id], bm->CloseKinEst[sp].last_sy, bm->RBCestimation.RBCspeciesParam[sp][MaxAge_id], bm->CloseKinEst[sp].last_sy, bm->RBCestimation.RBCspeciesParam[sp][MaxAge_id], 0.0);
        Ekin_PO_syaya = Util_Alloc_Init_5D_Double(bm->RBCestimation.RBCspeciesParam[sp][Nsexes_id], bm->CloseKinEst[sp].last_sy, bm->RBCestimation.RBCspeciesParam[sp][MaxAge_id], bm->CloseKinEst[sp].last_sy, bm->RBCestimation.RBCspeciesParam[sp][MaxAge_id], 0.0);
        
        sim_nkin_PO = Util_Alloc_Init_5D_Double(bm->RBCestimation.RBCspeciesParam[sp][Nsexes_id], bm->CloseKinEst[sp].last_sy, bm->RBCestimation.RBCspeciesParam[sp][MaxAge_id], bm->CloseKinEst[sp].last_sy, bm->RBCestimation.RBCspeciesParam[sp][MaxAge_id], 0.0);
        sim_nkin_HS = Util_Alloc_Init_5D_Double(bm->RBCestimation.RBCspeciesParam[sp][Nsexes_id], bm->CloseKinEst[sp].last_sy, bm->RBCestimation.RBCspeciesParam[sp][MaxAge_id], bm->CloseKinEst[sp].last_sy, bm->RBCestimation.RBCspeciesParam[sp][MaxAge_id], 0.0);
    }
     
     */

        
	return 0;
}

/**
 * Free the Tier assessment data structures.
 */
int Tier_Assessment_Free(MSEBoxModel *bm) {

	int groupIndex, tier;
    
    free1d(bm->RBCestimation.catchwghtCPUE);
    free1d(bm->RBCestimation.catchwght);
    i_free1d(bm->RBCestimation.count_sp_in_buffer);
    
    if(bm->RBCestimation.speciesMetierToRPFleet)
        i_free2d(bm->RBCestimation.speciesMetierToRPFleet);
    if(bm->RBCestimation.speciesRPFleetToMetier)
        i_free2d(bm->RBCestimation.speciesRPFleetToMetier);
    
	for (groupIndex = 0; groupIndex < bm->K_num_tot_sp; groupIndex++) {
        tier = (int) (FunctGroupArray[groupIndex].speciesParams[tier_id]);
        
        if ((FunctGroupArray[groupIndex].speciesParams[assess_flag_id] == FALSE) && (FunctGroupArray[groupIndex].isFished == FALSE))
            continue;
        
        printf("Doing %s assess free\n", FunctGroupArray[groupIndex].groupCode);

        i_free1d(bm->RBCestimation.RBCspeciesArray[groupIndex].regID);
        i_free2d(bm->RBCestimation.RBCspeciesArray[groupIndex].Ia);
        free1d(bm->RBCestimation.RBCspeciesArray[groupIndex].Future_catchprop);
        
        free2d(bm->RBCestimation.RBCspeciesArray[groupIndex].CvLA0);
        free2d(bm->RBCestimation.RBCspeciesArray[groupIndex].CvLAmax);
        free2d(bm->RBCestimation.RBCspeciesArray[groupIndex].Fecund);
        
        free3d(bm->RBCestimation.RBCspeciesArray[groupIndex].MeanLenAge);
        free4d(bm->RBCestimation.RBCspeciesArray[groupIndex].MeanWtAge);
        free3d(bm->RBCestimation.RBCspeciesArray[groupIndex].Mzero);
        free5d(bm->RBCestimation.RBCspeciesArray[groupIndex].Mnat);
        
        free3d(bm->RBCestimation.RBCspeciesArray[groupIndex].WtLen);
        free2d(bm->RBCestimation.RBCspeciesArray[groupIndex].Wtlen_a);
        free2d(bm->RBCestimation.RBCspeciesArray[groupIndex].Wtlen_b);
        
        free2d(bm->RBCestimation.RBCspeciesArray[groupIndex].VBLinf);
        free2d(bm->RBCestimation.RBCspeciesArray[groupIndex].VBk);
        free2d(bm->RBCestimation.RBCspeciesArray[groupIndex].VBt0);
        
        free1d(bm->RBCestimation.RBCspeciesArray[groupIndex].CatchCV);
        free3d(bm->RBCestimation.RBCspeciesArray[groupIndex].CatchData);
        free1d(bm->RBCestimation.RBCspeciesArray[groupIndex].EffortCV);
        free3d(bm->RBCestimation.RBCspeciesArray[groupIndex].EffortData);
        free2d(bm->RBCestimation.RBCspeciesArray[groupIndex].EnviroData);

        free5d(bm->RBCestimation.RBCspeciesArray[groupIndex].Uage);
        free3d(bm->RBCestimation.RBCspeciesArray[groupIndex].Ufleet);

        free1d(bm->RBCestimation.RBCspeciesArray[groupIndex].LoLenBin);

        free3d(bm->RBCestimation.RBCspeciesArray[groupIndex].LenRetain);

        i_free1d(bm->RBCestimation.RBCspeciesArray[groupIndex].AgeFuture);
        i_free1d(bm->RBCestimation.RBCspeciesArray[groupIndex].AgeN);
        i_free2d(bm->RBCestimation.RBCspeciesArray[groupIndex].AgeYears);
        i_free3d(bm->RBCestimation.RBCspeciesArray[groupIndex].AgeFltYr);
        
        free1d(bm->RBCestimation.RBCspeciesArray[groupIndex].PGMSY_q);
        free1d(bm->RBCestimation.RBCspeciesArray[groupIndex].PGMSY_selcurve);
        free1d(bm->RBCestimation.RBCspeciesArray[groupIndex].PGMSY_sel_lsm);
        free1d(bm->RBCestimation.RBCspeciesArray[groupIndex].PGMSY_sel_sigma);

        free2d(bm->RBCestimation.RBCspeciesArray[groupIndex].Fhist);
        
        i_free1d(bm->RBCestimation.RBCspeciesArray[groupIndex].CPUEfuture);
        i_free3d(bm->RBCestimation.RBCspeciesArray[groupIndex].CPUEyears);
        free3d(bm->RBCestimation.RBCspeciesArray[groupIndex].CPUEdevs);

        i_free1d(bm->RBCestimation.RBCspeciesArray[groupIndex].DiscFuture);

        free1d(bm->RBCestimation.RBCspeciesArray[groupIndex].CPUEcv);
        free1d(bm->RBCestimation.RBCspeciesArray[groupIndex].CPUEvar);
        free1d(bm->RBCestimation.RBCspeciesArray[groupIndex].CPUEcorr);
        free3d(bm->RBCestimation.RBCspeciesArray[groupIndex].CPUEgen);
        free1d(bm->RBCestimation.RBCspeciesArray[groupIndex].CPUEpow);
        free1d(bm->RBCestimation.RBCspeciesArray[groupIndex].CPUEqmu);
        
        free3d(bm->RBCestimation.RBCspeciesArray[groupIndex].CPUEq);
        free3d(bm->RBCestimation.RBCspeciesArray[groupIndex].TotCatch);
        free3d(bm->RBCestimation.RBCspeciesArray[groupIndex].DiscData);
        free1d(bm->RBCestimation.RBCspeciesArray[groupIndex].DiscCV);
        i_free2d(bm->RBCestimation.RBCspeciesArray[groupIndex].DiscYears);
        
        i_free1d(bm->RBCestimation.RBCspeciesArray[groupIndex].LengthFuture);
        i_free1d(bm->RBCestimation.RBCspeciesArray[groupIndex].LengthN);
        i_free2d(bm->RBCestimation.RBCspeciesArray[groupIndex].LengthYears);
        i_free3d(bm->RBCestimation.RBCspeciesArray[groupIndex].LengthFltYr);
        i_free4d(bm->RBCestimation.RBCspeciesArray[groupIndex].LFss);
        
        i_free1d(bm->RBCestimation.RBCspeciesArray[groupIndex].mirrored_fleet);

        free1d(bm->RBCestimation.RBCspeciesArray[groupIndex].Ageing_error);
        free2d(bm->RBCestimation.RBCspeciesArray[groupIndex].SSMort);

        free1d(bm->RBCestimation.RBCspeciesArray[groupIndex].Start_SelInflect);
        free1d(bm->RBCestimation.RBCspeciesArray[groupIndex].Start_SelWidth);
        free1d(bm->RBCestimation.RBCspeciesArray[groupIndex].Start_RetInflect);
        free1d(bm->RBCestimation.RBCspeciesArray[groupIndex].Start_RetSlope);
        
        i_free1d(bm->RBCestimation.RBCspeciesArray[groupIndex].Sel_Pattern);

        free1d(bm->RBCestimation.RBCspeciesArray[groupIndex].Varadj_CPUE);
        free1d(bm->RBCestimation.RBCspeciesArray[groupIndex].Varadj_discard);
        free1d(bm->RBCestimation.RBCspeciesArray[groupIndex].Varadj_length);
        free1d(bm->RBCestimation.RBCspeciesArray[groupIndex].Varadj_age);

        free3d(bm->RBCestimation.RBCspeciesArray[groupIndex].RetVulBio);

        free2d(bm->RBCestimation.RBCspeciesArray[groupIndex].SBioZero_shift);
        free2d(bm->RBCestimation.RBCspeciesArray[groupIndex].SBioZero);
        free3d(bm->RBCestimation.RBCspeciesArray[groupIndex].SpawnBio);

        free2d(bm->RBCestimation.RBCspeciesArray[groupIndex].TriggerPoints);
        i_free1d(bm->RBCestimation.RBCspeciesArray[groupIndex].TriggerReached);
        free1d(bm->RBCestimation.RBCspeciesArray[groupIndex].AnnCPUE);
        
        //if (bm->RBCestimation.RBCspeciesArray[groupIndex].SelLen)
        //    free3d(bm->RBCestimation.RBCspeciesArray[groupIndex].SelLen);
        
        //if (bm->RBCestimation.RBCspeciesArray[groupIndex].CPUEqmu)
        //    free1d(bm->RBCestimation.RBCspeciesArray[groupIndex].CPUEqmu);
        
        //if (bm->RBCestimation.RBCspeciesArray[groupIndex].Hsteep)
        //    free1d(bm->RBCestimation.RBCspeciesArray[groupIndex].Hsteep);

        //if (bm->RBCestimation.RBCspeciesArray[groupIndex].SigmaR)
        //    free1d(bm->RBCestimation.RBCspeciesArray[groupIndex].SigmaR);
        
        //if (bm->RBCestimation.RBCspeciesArray[groupIndex].CorRecDevs)
        //    free2d(bm->RBCestimation.RBCspeciesArray[groupIndex].CorRecDevs);
        
        //if (bm->RBCestimation.RBCspeciesArray[groupIndex].Transition)
        //    i_free2d(bm->RBCestimation.RBCspeciesArray[groupIndex].Transition);
        
        //if (bm->RBCestimation.RBCspeciesArray[groupIndex].TransCol)
        //    i_free1d(bm->RBCestimation.RBCspeciesArray[groupIndex].TransCol);
        
        if ((FunctGroupArray[groupIndex].speciesParams[tier_id] == 0) || (FunctGroupArray[groupIndex].speciesParams[assess_flag_id] == FALSE)) {
            // Nothing to do so skip ahead
        } else {
            free1d(bm->RBCestimation.RBCspeciesArray[groupIndex].agesel);
            free1d(bm->RBCestimation.RBCspeciesArray[groupIndex].mat);
            free1d(bm->RBCestimation.RBCspeciesArray[groupIndex].wt);
            free1d(bm->RBCestimation.RBCspeciesArray[groupIndex].fvalA);
        
            free1d(bm->RBCestimation.RBCspeciesArray[groupIndex].ageref);
            free1d(bm->RBCestimation.RBCspeciesArray[groupIndex].len);
            free2d(bm->RBCestimation.RBCspeciesArray[groupIndex].p_al);
        
            free1d(bm->RBCestimation.RBCspeciesArray[groupIndex].yield);
            free1d(bm->RBCestimation.RBCspeciesArray[groupIndex].yield_mid);
            free1d(bm->RBCestimation.RBCspeciesArray[groupIndex].num);
        
            free1d(bm->RBCestimation.RBCspeciesArray[groupIndex].z);
            free1d(bm->RBCestimation.RBCspeciesArray[groupIndex].sumcatch);
        
            free1d(bm->RBCestimation.RBCspeciesArray[groupIndex].T5_Fval);
            free1d(bm->RBCestimation.RBCspeciesArray[groupIndex].T5_Avlen_atF);
        }
        
        if ((FunctGroupArray[groupIndex].speciesParams[tier_id] == 0) || (FunctGroupArray[groupIndex].speciesParams[assess_flag_id] == FALSE)) // Not needed as not assessed
            continue;
        
        free1d(bm->RBCestimation.RBCspeciesArray[groupIndex].propR);
        free2d(bm->RBCestimation.RBCspeciesArray[groupIndex].propSR);
        free1d(bm->RBCestimation.RBCspeciesArray[groupIndex].propCount);
        
        free2d(bm->RBCestimation.RBCspeciesArray[groupIndex].actualcatch);
        free2d(bm->RBCestimation.RBCspeciesArray[groupIndex].actualeffort);
        free2d(bm->RBCestimation.RBCspeciesArray[groupIndex].actualdiscard);
        free3d(bm->RBCestimation.RBCspeciesArray[groupIndex].actualbiom);
        free3d(bm->RBCestimation.RBCspeciesArray[groupIndex].actualcatchsize);

        free4d(bm->RBCestimation.RBCspeciesArray[groupIndex].rawdata);
        free5d(bm->RBCestimation.RBCspeciesArray[groupIndex].rawsizedata);
        free5d(bm->RBCestimation.RBCspeciesArray[groupIndex].rawnumdata);
        free4d(bm->RBCestimation.RBCspeciesArray[groupIndex].ndata);
        
        free2d(bm->RBCestimation.RBCspeciesArray[groupIndex].lenprops);
        free1d(bm->RBCestimation.RBCspeciesArray[groupIndex].lencomp);
        free2d(bm->RBCestimation.RBCspeciesArray[groupIndex].ageprops);
        free1d(bm->RBCestimation.RBCspeciesArray[groupIndex].agecomp);
        
        if (bm->useGenMnomial) {
            free2d(bm->RBCestimation.RBCspeciesArray[groupIndex].propvec);
            free3d(bm->RBCestimation.RBCspeciesArray[groupIndex].newvec);
        }
        
        free1d(bm->RBCestimation.RBCspeciesArray[groupIndex].props);
        free4d(bm->RBCestimation.RBCspeciesArray[groupIndex].CKSurveyNum);
        
        /* Files from atManageTier.c */
        free2d(bm->RBCestimation.RBCspeciesArray[groupIndex].propcatch);
        free3d(bm->RBCestimation.RBCspeciesArray[groupIndex].propcatch_reg);
        free1d(bm->RBCestimation.RBCspeciesArray[groupIndex].avprop);
        free2d(bm->RBCestimation.RBCspeciesArray[groupIndex].avprop_reg);

        free1d(bm->RBCestimation.RBCspeciesArray[groupIndex].discrate);
        free2d(bm->RBCestimation.RBCspeciesArray[groupIndex].agesel_bysex);
        
        if (tier == tier1) {  // As only used in tier1 routines
            free2d(bm->RBCestimation.RBCspeciesArray[groupIndex].fval);
        }
        
        if (tier == tier3) {  // As only used in tier3 routines
            free1d(bm->RBCestimation.RBCspeciesArray[groupIndex].numatage);
            free1d(bm->RBCestimation.RBCspeciesArray[groupIndex].ages);
            i_free1d(bm->RBCestimation.RBCspeciesArray[groupIndex].propgt05);
            free1d(bm->RBCestimation.RBCspeciesArray[groupIndex].totnumatage);
            free1d(bm->RBCestimation.RBCspeciesArray[groupIndex].fvalF);
            free1d(bm->RBCestimation.RBCspeciesArray[groupIndex].chi);
            free2d(bm->RBCestimation.RBCspeciesArray[groupIndex].sel);
        }
        if ((tier == tier3) || (tier == tier5)){  // As only used in tier3 routines
            free2d(bm->RBCestimation.RBCspeciesArray[groupIndex].xunit);
            free1d(bm->RBCestimation.RBCspeciesArray[groupIndex].xpar);
            free2d(bm->RBCestimation.RBCspeciesArray[groupIndex].simplex);
            free1d(bm->RBCestimation.RBCspeciesArray[groupIndex].scale);
            free1d(bm->RBCestimation.RBCspeciesArray[groupIndex].func);
        }

        if ((tier == tier1) || (tier == tier3)) { // As only used in tier1 and tier3 routines
            free1d(bm->RBCestimation.RBCspeciesArray[groupIndex].exp_prop);
            free1d(bm->RBCestimation.RBCspeciesArray[groupIndex].Funkz);
        }

        if (tier == tier4) {  // As only used in tier4 routines
            free1d(bm->RBCestimation.RBCspeciesArray[groupIndex].xyear);
            free1d(bm->RBCestimation.RBCspeciesArray[groupIndex].ycpue);
        }
        
        if (tier == tier5) {  // As only used in tier5 routines
            free1d(bm->RBCestimation.RBCspeciesArray[groupIndex].expectB);
            free1d(bm->RBCestimation.RBCspeciesArray[groupIndex].expectq);
        }
        
        if (tier ==tier7) {
            if (bm->RBCestimation.RBCspeciesArray[groupIndex].CatchComp)
                free2d(bm->RBCestimation.RBCspeciesArray[groupIndex].CatchComp);
            if (bm->RBCestimation.RBCspeciesArray[groupIndex].relareafished)
                free2d(bm->RBCestimation.RBCspeciesArray[groupIndex].relareafished);
            if (bm->RBCestimation.RBCspeciesArray[groupIndex].cpue)
                free1d(bm->RBCestimation.RBCspeciesArray[groupIndex].cpue);
        }
        
        
        if(bm->CloseKinEst[groupIndex].fec_expo_s)
            free1d(bm->CloseKinEst[groupIndex].fec_expo_s);
        if(bm->CloseKinEst[groupIndex].fec_sa)
            free2d(bm->CloseKinEst[groupIndex].fec_sa);
        if(bm->CloseKinEst[groupIndex].mature)
            free1d(bm->CloseKinEst[groupIndex].mature);
        if(bm->CloseKinEst[groupIndex].lengths)
            free1d(bm->CloseKinEst[groupIndex].lengths);
        if(bm->CloseKinEst[groupIndex].samp_prop_to)
            free3d(bm->CloseKinEst[groupIndex].samp_prop_to);

        if (bm->RBCestimation.RBCspeciesArray[groupIndex].TSbiomass) {
            free1d(bm->RBCestimation.RBCspeciesArray[groupIndex].TSbiomass);
        }
        
        if(bm->RBCestimation.RBCspeciesArray[groupIndex].SlopeMat)
            free1d(bm->RBCestimation.RBCspeciesArray[groupIndex].SlopeMat);
        if(bm->RBCestimation.RBCspeciesArray[groupIndex].Mat50)
            free1d(bm->RBCestimation.RBCspeciesArray[groupIndex].Mat50);
        if(bm->RBCestimation.RBCspeciesArray[groupIndex].HiLenBin)
            free1d(bm->RBCestimation.RBCspeciesArray[groupIndex].HiLenBin);
        if(bm->RBCestimation.RBCspeciesArray[groupIndex].FracLenS)
            free5d(bm->RBCestimation.RBCspeciesArray[groupIndex].FracLenS);
        if(bm->RBCestimation.RBCspeciesArray[groupIndex].MeanLenAgeM)
            free4d(bm->RBCestimation.RBCspeciesArray[groupIndex].MeanLenAgeM);
        if(bm->RBCestimation.RBCspeciesArray[groupIndex].SigmaLenAgeM)
            free4d(bm->RBCestimation.RBCspeciesArray[groupIndex].SigmaLenAgeM);
        if(bm->RBCestimation.RBCspeciesArray[groupIndex].MeanWtAgeS)
            free4d(bm->RBCestimation.RBCspeciesArray[groupIndex].MeanWtAgeS);
        if(bm->RBCestimation.RBCspeciesArray[groupIndex].MeanWtAgeM)
            free4d(bm->RBCestimation.RBCspeciesArray[groupIndex].MeanWtAgeM);
        if(bm->RBCestimation.RBCspeciesArray[groupIndex].NblockperPatt)
            i_free1d(bm->RBCestimation.RBCspeciesArray[groupIndex].NblockperPatt);
        if(bm->RBCestimation.RBCspeciesArray[groupIndex].discfleet)
            i_free1d(bm->RBCestimation.RBCspeciesArray[groupIndex].discfleet);

        if(bm->RBCestimation.RBCspeciesArray[groupIndex].initialEquilCatch)
            free1d(bm->RBCestimation.RBCspeciesArray[groupIndex].initialEquilCatch);
        if(bm->RBCestimation.RBCspeciesArray[groupIndex].R02)
            free1d(bm->RBCestimation.RBCspeciesArray[groupIndex].R02);
        if(bm->RBCestimation.RBCspeciesArray[groupIndex].SSHsteep)
            free1d(bm->RBCestimation.RBCspeciesArray[groupIndex].SSHsteep);
        if(bm->RBCestimation.RBCspeciesArray[groupIndex].Start_Sel)
            free2d(bm->RBCestimation.RBCspeciesArray[groupIndex].Start_Sel);
        if(bm->RBCestimation.RBCspeciesArray[groupIndex].SelBlock)
            free2d(bm->RBCestimation.RBCspeciesArray[groupIndex].SelBlock);
        if(bm->RBCestimation.RBCspeciesArray[groupIndex].Sel_Phase)
            free2d(bm->RBCestimation.RBCspeciesArray[groupIndex].Sel_Phase);
        if(bm->RBCestimation.RBCspeciesArray[groupIndex].RBC_by_year)
            free1d(bm->RBCestimation.RBCspeciesArray[groupIndex].RBC_by_year);
        
        if(bm->RBCestimation.RBCspeciesArray[groupIndex].Mortality)
            free1d(bm->RBCestimation.RBCspeciesArray[groupIndex].Mortality);
                
        if(bm->RBCestimation.RBCspeciesArray[groupIndex].Fupdated)
            free2d(bm->RBCestimation.RBCspeciesArray[groupIndex].Fupdated);
        if(bm->RBCestimation.RBCspeciesArray[groupIndex].RBCupdated)
            free2d(bm->RBCestimation.RBCspeciesArray[groupIndex].RBCupdated);
        if(bm->RBCestimation.RBCspeciesArray[groupIndex].CatchStore)
            free2d(bm->RBCestimation.RBCspeciesArray[groupIndex].CatchStore);
        if(bm->RBCestimation.RBCspeciesArray[groupIndex].CscalarMetier)
            free1d(bm->RBCestimation.RBCspeciesArray[groupIndex].CscalarMetier);
        
        if(bm->RBCestimation.RBCspeciesArray[groupIndex].AvgCatFleet)
            free2d(bm->RBCestimation.RBCspeciesArray[groupIndex].AvgCatFleet);
        if(bm->RBCestimation.RBCspeciesArray[groupIndex].FFs)
            free1d(bm->RBCestimation.RBCspeciesArray[groupIndex].FFs);

	}

    if(bm->RBCestimation.Catch_by_Metier)
        free4d(bm->RBCestimation.Catch_by_Metier);

	if (bm->RBCestimation.RBCspeciesArray)
		free(bm->RBCestimation.RBCspeciesArray);

	if (bm->RBCestimation.RBCspeciesParam)
		free2d(bm->RBCestimation.RBCspeciesParam);
    
    if (bm->RBCestimation.metierArray)
        free(bm->RBCestimation.metierArray);
    
    if (bm->RBCestimation.F_actFleet)
        free2d(bm->RBCestimation.F_actFleet);
    
    if (bm->RBCestimation.FratioFleet)
        free2d(bm->RBCestimation.FratioFleet);

	return 0;
}

/********************************************************************************
 *
 * Name:  Prop_LatA
 * Description: calculate proportion of fish of age a in length bin l for avlen calcs
 *
 * called by : ReadAssessSpecs
 * created  : July 2010 Sally
 *
 ********************************************************************************/
void Prop_LatA(MSEBoxModel *bm, int species, double *len, double **p_al){
	int a, l, nlen;
	double xlim, accum, integral, meanlen, siglen;

	// calc proportion of age a in length bin l
	for (a = 0; a < bm->RBCestimation.RBCspeciesParam[species][MaxAge_id]; a++) {
		meanlen = len[a];
		siglen = bm->RBCestimation.RBCspeciesParam[species][Tier5_cv_id] * meanlen;
		xlim = (bm->RBCestimation.RBCspeciesArray[species].LoLenBin[1] - meanlen) / siglen;
		accum = Assess_errfun(xlim);
		p_al[a][0] = accum;
        
        //fprintf(bm->logFile,"a: %d meanlen: %e siglen: %e cv: %e xlim: %e p_al[0] and accum: %e\n", a, meanlen, siglen, bm->RBCestimation.RBCspeciesParam[species][Tier5_cv_id], xlim, accum);
        
		for (l = 1; l < bm->RBCestimation.RBCspeciesParam[species][Nlen_id] - 1; l++){
			xlim = (bm->RBCestimation.RBCspeciesArray[species].LoLenBin[l+1] - meanlen) / siglen;
			integral = Assess_errfun(xlim);
			p_al[a][l] = integral - accum;
			accum = integral;
            
            //fprintf(bm->logFile,"a: %d l: %d xlim: %e integral: %e accum: %e p_al[l]: %e\n", a, l, xlim, integral, accum, p_al[a][l]);
            
		}
		nlen = (int)(bm->RBCestimation.RBCspeciesParam[species][Nlen_id]);
        nlen--; // So don't over-write the end of the array
		p_al[a][nlen] = 1.0 - accum;
        
        //fprintf(bm->logFile,"a: %d l: %d accum: %e p_al[l]: %e\n", a, nlen, accum, p_al[a][nlen]);
        
	}
}


/******************************************************************************
 *
 * Name:  YPRlen
 * Description: yield-per-recruit and average length calculations for given F
 * called by : getF20_40_48_len
******************************************************************************/
void YPRlen(MSEBoxModel *bm, int species, double fval, double natM, double *spbpr,
		double *avlen_atF, double *wt, double *len, double *mat, double *sel, double **p_al){

	int a,l;
	double yp,ypl;
	int maxage = (int)(bm->RBCestimation.RBCspeciesParam[species][MaxAge_id] - 1);

	//printf("Creating YPRlen arrays\n");

	*spbpr = 0.0;
	*avlen_atF = 0.0;

	for (a = 0; a < maxage + 1; a++)
		bm->RBCestimation.RBCspeciesArray[species].z[a] = natM + sel[a] * fval;
	bm->RBCestimation.RBCspeciesArray[species].num[0] = 1.0;

	// calculate numbers at age relative to age 0
	for (a = 1; a < maxage; a++)
		bm->RBCestimation.RBCspeciesArray[species].num[a] = bm->RBCestimation.RBCspeciesArray[species].num[a-1] * exp(-1.0 * bm->RBCestimation.RBCspeciesArray[species].z[a-1]);

	// plus group
	bm->RBCestimation.RBCspeciesArray[species].num[maxage] = bm->RBCestimation.RBCspeciesArray[species].num[maxage-1] * exp(-1.0 * bm->RBCestimation.RBCspeciesArray[species].z[maxage-1]) / (1.0-exp(-1.0 * bm->RBCestimation.RBCspeciesArray[species].z[maxage]));

	// YPR
	for (a = 0 ; a <maxage + 1; a++) {
		*spbpr += bm->RBCestimation.RBCspeciesArray[species].num[a] * wt[a] * mat[a];
		bm->RBCestimation.RBCspeciesArray[species].yield[a] = bm->RBCestimation.RBCspeciesArray[species].num[a] * sel[a] * fval / bm->RBCestimation.RBCspeciesArray[species].z[a] * (1-exp(-1.0 * bm->RBCestimation.RBCspeciesArray[species].z[a]));
        
        //fprintf(bm->logFile, "a: %d sbpr: %e num: %e wt: %e mat: %e yield: %e sel: %e fval: %e, z: %e lastterm: %e\n", a, *spbpr, bm->RBCestimation.RBCspeciesArray[species].num[a], wt[a], mat[a], bm->RBCestimation.RBCspeciesArray[species].yield[a], sel[a], fval, bm->RBCestimation.RBCspeciesArray[species].z[a], (1-exp(-1.0 * bm->RBCestimation.RBCspeciesArray[species].z[a])));
	}

	// calc avlen above ref len
	yp = ypl = 0.0;
	if (bm->RBCestimation.RBCspeciesParam[species][Tier5_cv_id] > 0.0) {
		for (l=0 ;l < bm->RBCestimation.RBCspeciesParam[species][Nlen_id]; l++){
			if ((bm->RBCestimation.RBCspeciesArray[species].LoLenBin[l] + (bm->RBCestimation.RBCspeciesParam[species][Lbin_id] / 2.0)) >= bm->RBCestimation.RBCspeciesParam[species][Tier5_length_id]){
				for (a = 0; a < maxage + 1; a++){
					yp += bm->RBCestimation.RBCspeciesArray[species].yield[a] * p_al[a][l];
					ypl += bm->RBCestimation.RBCspeciesArray[species].yield[a] * p_al[a][l] * (bm->RBCestimation.RBCspeciesArray[species].LoLenBin[l] + (bm->RBCestimation.RBCspeciesParam[species][Lbin_id] / 2.0));
                    
                    //fprintf(bm->logFile, "a: %d l: %d yp: %e yield: %e p_al: %e LoLen: %e Lbin: %e ypl: %e size: %e\n", a, l, yp, bm->RBCestimation.RBCspeciesArray[species].yield[a], p_al[a][l], bm->RBCestimation.RBCspeciesArray[species].LoLenBin[l], bm->RBCestimation.RBCspeciesParam[species][Lbin_id], ypl, (bm->RBCestimation.RBCspeciesArray[species].LoLenBin[l] + (bm->RBCestimation.RBCspeciesParam[species][Lbin_id] / 2.0)));
				}
			}
		}
	} else {
		for (a = 0; a < maxage + 1; a++){
			if (len[a] >= bm->RBCestimation.RBCspeciesParam[species][Tier5_length_id]){
				yp += bm->RBCestimation.RBCspeciesArray[species].yield[a];
				ypl += bm->RBCestimation.RBCspeciesArray[species].yield[a] * len[a];
			}
		}
	}

	if (yp > 0.0000000001) *avlen_atF = ypl / yp;

	return;
}



/*****************************************************************************************
 *  Calculate the initial reference levels
 ****************************************************************************************/
void getF20_40_48_init_len(MSEBoxModel *bm, int species){
	int a, i;
	double step1;
	double thisalpha, thisbeta, thisfval, spbpr, spfrac, spbf, rec_prog, avlen_atF;
    double spb0 = small_num;
    double spbf0 = small_num;
    int nfint = (int)(1000 * bm->RBCestimation.RBCspeciesParam[species][Tier3_maxF_id]);
    double nfintt = (double)(nfint);
    double age;
	//printf("Creating getF20_40_48_init_len arrays\n");

	// calculate weight, length, selectivity and maturity at age for females
	for (a=0; a< bm->RBCestimation.RBCspeciesParam[species][MaxAge_id]; a++){
		age = a + 0.5;
		step1 = 1.0 - exp(- 1.0 * bm->RBCestimation.RBCspeciesArray[species].Tier3_k[FEMALE] * (age - bm->RBCestimation.RBCspeciesArray[species].Tier3_t0[FEMALE]));
		bm->RBCestimation.RBCspeciesArray[species].len[a] = bm->RBCestimation.RBCspeciesArray[species].Tier3_Linf[FEMALE] * step1;
		bm->RBCestimation.RBCspeciesArray[species].wt[a] = bm->RBCestimation.RBCspeciesParam[species][Tier3_a_id] * pow(bm->RBCestimation.RBCspeciesArray[species].len[a], bm->RBCestimation.RBCspeciesParam[species][Tier3_b_id]);                 // weight in kg
		if (bm->RBCestimation.RBCspeciesArray[species].len[a] < bm->RBCestimation.RBCspeciesParam[species][Tier5_S50_id])
			bm->RBCestimation.RBCspeciesArray[species].agesel[a] = 0.0;
		else
			bm->RBCestimation.RBCspeciesArray[species].agesel[a] = 1.0;
		if (bm->RBCestimation.RBCspeciesArray[species].len[a] < bm->RBCestimation.RBCspeciesParam[species][Tier5_length_id])
			bm->RBCestimation.RBCspeciesArray[species].ageref[a] = 0.0;
		else
			bm->RBCestimation.RBCspeciesArray[species].ageref[a] = 1.0;
		if ( bm->RBCestimation.RBCspeciesArray[species].len[a] > bm->RBCestimation.RBCspeciesParam[species][Tier3_matlen_id])
			bm->RBCestimation.RBCspeciesArray[species].mat[a] = 1.0;
		else
			bm->RBCestimation.RBCspeciesArray[species].mat[a] = 0.0;      // knife-edge maturity at length = matlen
	}

	// calculate proportion of age a in length bin l if length at age cv > 0
	if (bm->RBCestimation.RBCspeciesParam[species][Tier5_cv_id] > 0.0)
		Prop_LatA(bm, species, bm->RBCestimation.RBCspeciesArray[species].len, bm->RBCestimation.RBCspeciesArray[species].p_al);

	// calculate Freference values and av length at F
	thisalpha = (1.0 - bm->RBCestimation.RBCspeciesParam[species][Tier3_h_id]) / (4.0 * bm->RBCestimation.RBCspeciesParam[species][Tier3_h_id]);    // this is (usual alpha)/B0
	thisbeta = (5.0 * bm->RBCestimation.RBCspeciesParam[species][Tier3_h_id] - 1.0) / (4.0 * bm->RBCestimation.RBCspeciesParam[species][Tier3_h_id]);
	thisfval = 0.0;

	for (i = 0; i < nfint; i++){  // f = i/1000  f goes from 0 to Tier3_maxF
        bm->RBCestimation.RBCspeciesArray[species].T5_Fval[i] = thisfval;

		YPRlen(bm, species, thisfval, bm->RBCestimation.RBCspeciesParam[species][Tier3_M_id], &spbpr, &avlen_atF, bm->RBCestimation.RBCspeciesArray[species].wt, bm->RBCestimation.RBCspeciesArray[species].len, bm->RBCestimation.RBCspeciesArray[species].mat, bm->RBCestimation.RBCspeciesArray[species].agesel, bm->RBCestimation.RBCspeciesArray[species].p_al);

        bm->RBCestimation.RBCspeciesArray[species].T5_Avlen_atF[i] = avlen_atF;
        
        //fprintf(bm->logFile,"i: %d avlen_atF: %e T5_val: %e\n", i, avlen_atF, bm->RBCestimation.RBCspeciesArray[species].T5_Fval[i]);

		if (i == 0) spb0 = spbpr + small_num;
		spfrac = spbpr / spb0;
		rec_prog = spfrac/(thisalpha + thisbeta * spfrac + small_num);
		spbf = spbpr * rec_prog;

		if (i == 0) spbf0 = spbf;

		// F20, F40 F48
		if (spbf > (spbf0 * 0.2)) bm->RBCestimation.RBCspeciesParam[species][F20_id] = thisfval;
		if (spbf > (spbf0 * 0.4)) bm->RBCestimation.RBCspeciesParam[species][F40_id] = thisfval;
		if (spbf > (spbf0 * 0.48)) bm->RBCestimation.RBCspeciesParam[species][F48_id] = thisfval;

		thisfval = i / nfintt;
	}

}

/******************************************************************************
 * Name:  SetUpBiology
 * Description: calculate mean length-at-age, sd of length-at-age, mean weight-at-age,
 *              mean weight-at-length, and fill in vectors for time-varying parameters
 *              currently there is no variation over time
 *
 ******************************************************************************/
void SetUpBiology(MSEBoxModel *bm, int species)
{
    int j, s, a, t, r;
    double step1, step2, step3;
    int ssize = (int)(FunctGroupArray[species].speciesParams[samplesize_id]);
    
    int yr1 = 0;
    int yr2 = (int)bm->RBCestimation.RBCspeciesParam[species][Nyears_id];

  // Set up M matrix
	for (j = 0; j < FunctGroupArray[species].numStocks; j++){
		for (s = 0; s < bm->K_num_sexes; s++){
			for (r = 0; r < bm->K_num_reg; r++){
				for (a = 0; a < bm->RBCestimation.RBCspeciesParam[species][MaxAge_id]; a++){
					bm->RBCestimation.RBCspeciesArray[species].Mnat[j][r][s][a][yr1] = bm->RBCestimation.RBCspeciesArray[species].Mzero[j][s][a];
					for (t = yr1+1; t < yr2; t++){
						bm->RBCestimation.RBCspeciesArray[species].Mnat[j][r][s][a][t] = bm->RBCestimation.RBCspeciesArray[species].Mnat[j][r][s][a][t-1];
					}
 				}
			}
		}
	}

    // set up movement matrix
   getMove(bm, species);

   // Get the starting sizes
   if (bm->RBCestimation.RBCspeciesParam[species][flagLAdirect_id]) {
       GetLengthWeightData(bm, species, 0);
   }
    
   // Calculate weight at age in first year if wt-age parameters provided (eqn 1.16)
	if (!bm->RBCestimation.RBCspeciesParam[species][flagWAdirect_id]) {
		for (j=0; j < FunctGroupArray[species].numStocks; j++){
			for (s=0; s < bm->K_num_sexes; s++){
				for (a=0; a< bm->RBCestimation.RBCspeciesParam[species][MaxAge_id]; a++){
					step1 =  bm->RBCestimation.RBCspeciesArray[species].Wtlen_a[j][s];
					step2 = bm->RBCestimation.RBCspeciesArray[species].Wtlen_b[j][s];
					step3 = bm->RBCestimation.RBCspeciesArray[species].MeanLenAge[j][s][a];
					bm->RBCestimation.RBCspeciesArray[species].MeanWtAge[j][s][a][yr1] = step1 * exp(step2 * log(step3));
				}
			}
		}
	}

   // Propagate these vectors constantly through time (as currently time-varying growth not implememted)
   // If weight at age parameters provided, potential for wt-at-age to change with changes in mean length-at-age
	for (j=0; j < FunctGroupArray[species].numStocks; j++){
		for (s=0; s < bm->K_num_sexes; s++){
			for (a=0; a < bm->RBCestimation.RBCspeciesParam[species][MaxAge_id]; a++){
				for (t=yr1+1; t < yr2; t++){
					bm->RBCestimation.RBCspeciesArray[species].MeanWtAge[j][s][a][t] = bm->RBCestimation.RBCspeciesArray[species].MeanWtAge[j][s][a][t-1];
				}
			}
		}
	}

    
    for (j=0; j < 3; j++){
        bm->RBCestimation.RBCspeciesArray[species].AgeN[j] = ssize;
        bm->RBCestimation.RBCspeciesArray[species].LengthN[j] = ssize;
        
        fprintf(bm->logFile, "%s %d has samplesize %d (AgeN: %d, LengthN: %d\n", FunctGroupArray [species].groupCode, j, ssize, bm->RBCestimation.RBCspeciesArray[species].AgeN[j], bm->RBCestimation.RBCspeciesArray[species].LengthN[j]);
    }
}

/*******************************************************************************
 *
 * Name:  getMove
 * Description: get the values for the movement matrix Xmove, currently no movement
 *
 * Created : Oct 2007  Sally
 * Modified for RUSS  Feb 2010
 ******************************************************************************/
void getMove(MSEBoxModel *bm, int species)
{

	/* Not converted as not actually required as yet
	int j,s,a,r,r2;
	d4_array sumX;

	sumX.chaSize(1,Nstocks,1,Nsexes,0,MaxAge,1,Nregions);

	for (j=1;j<=Nstocks;j++)
		for (s=1;s<=Nsexes;s++)
			for (a=0;a<=MaxAge;a++)
				for (r=1;r<=Nregions;r++)
					for (r2=1;r2<=Nregions;r2++)
					{
						if (r!= r2)
						{
							Xmove(j,r,r2,s,a) = T(j,s,r,r2)*Movrate(j,s,a);
							sumX(j,s,a,r) = sumX(j,s,a,r) + Xmove(j,r,r2,s,a);
						}
					}
	for (j=1;j<=Nstocks;j++)
		for (s=1;s<=Nsexes;s++)
			for (a=0;a<=MaxAge;a++)
				for (r=1;r<=Nregions;r++)
					for (r2=1;r2<=Nregions;r2++)
					{
						if (r == r2)
							Xmove(j,r,r2,s,a) = 1 - sumX(j,s,a,r);
					}

	*/

	return;
}




/******************************************************************************
 *
 * Name:  getNewDev
 * Description: get the next autocorrelated dev for blue grenadier
 * called by : SetUpRecDevs
 * Created : June 2011  Sally
 *
******************************************************************************
void getNewDev(MSEBoxModel *bm, int species, int from, double *newdev, double *means, double **tt, double *sg){
	int rannum;
	int nd, rd;

	rannum = ceil(rand() * bm->RBCestimation.RBCspeciesArray[species].TransCol[from]);  // generate random no between 1 and 'from' column total

	if (rannum < bm->RBCestimation.RBCspeciesArray[species].Transition[from][0]) nd = 1;
	else if ((rannum > bm->RBCestimation.RBCspeciesArray[species].Transition[from][0]) && (rannum < (bm->RBCestimation.RBCspeciesArray[species].Transition[from][0] + bm->RBCestimation.RBCspeciesArray[species].Transition[from][1]))) nd = 2;
	else if ((rannum > (bm->RBCestimation.RBCspeciesArray[species].Transition[from][0] + bm->RBCestimation.RBCspeciesArray[species].Transition[from][1]))
			&& (rannum < (bm->RBCestimation.RBCspeciesArray[species].Transition[from][0] + bm->RBCestimation.RBCspeciesArray[species].Transition[from][1] + bm->RBCestimation.RBCspeciesArray[species].Transition[from][2]))) nd = 3;
	else nd = 4;

	// generate rec devs until get one the required size
	do {
		Util_GenMnorm(newdev, means, &bm->RBCestimation.Iseedz, bm->K_num_reg, tt, sg);
		// determine what rec dev is in relation to stdev
		if (newdev[0] >= bm->RBCestimation.RBCspeciesParam[species][RecDevStdev_id]) rd = 1;                      // > 1 stdev
		else if (newdev[0] < bm->RBCestimation.RBCspeciesParam[species][RecDevStdev_id] && (newdev[0] >= 0)) rd = 2; // between and 0 and 1 srdev
		else if (newdev[0] < - bm->RBCestimation.RBCspeciesParam[species][RecDevStdev_id]) rd = 4;                // <-1 stdev
		else rd = 3;  // between 0 and -1 stdev
	} while (rd != nd);

}
*/
/******************************************************************************
 *
 * Name:  SetUpRecDevs
 * Description: get the time series of recruitment deviations (history and future)
 *
 ******************************************************************************

void SetUpRecDevs(MSEBoxModel *bm, int species, int yr1, int yr2)
{
	double rannum;
	int ireg,jreg,t,year1,from;
	double *means, *devs, *sg;
	double **tt;

	//printf("Creating SetUpRecDevs arrays\n");

	means = Util_Alloc_Init_1D_Double(bm->K_num_reg, 0.0);
	devs = Util_Alloc_Init_1D_Double(bm->K_num_reg, 0.0);
	sg = Util_Alloc_Init_1D_Double(bm->K_num_reg, 0.0);
	tt = Util_Alloc_Init_2D_Double(bm->K_num_reg, bm->K_num_reg, 0.0);

// get time series of variance-covariance matrix and sigmaRs
	for (t = yr1; t < yr2; t++){
		if (t < bm->RBCestimation.RBCspeciesParam[species][RecDevMaxYr_id])
			bm->RBCestimation.RBCspeciesArray[species].SigmaR[t] = bm->RBCestimation.RBCspeciesParam[species][SigmaR1_id];
		else
			bm->RBCestimation.RBCspeciesArray[species].SigmaR[t] = bm->RBCestimation.RBCspeciesParam[species][SigmaR_future_id];
		rannum = rand();
		if (rannum > bm->RBCestimation.RBCspeciesParam[species][PSigmaR1_id]) bm->RBCestimation.RBCspeciesArray[species].SigmaR[t] = bm->RBCestimation.RBCspeciesParam[species][SigmaR2_id];
        
        // This doesn't seem to be used anywhere so commented out for now
		//for (ireg=0; ireg < bm->K_num_reg; ireg++){
		//	for (jreg=1;jreg < bm->K_num_reg; jreg++) {
		//		bm->RBCestimation.RBCspeciesArray[species].VarRecDevs[ireg][jreg][t] = bm->RBCestimation.RBCspeciesArray[species].SigmaR[t] * bm->RBCestimation.RBCspeciesArray[species].SigmaR[t] * bm->RBCestimation.RBCspeciesArray[species].CorRecDevs[ireg][jreg];
		//	}
		//}
	}

// set up past rec devs to be 0 if ones read in start later than HistYrMin
	if (bm->RBCestimation.RBCspeciesParam[species][RecDevFlag_id]){
		for (ireg=0; ireg < bm->K_num_reg; ireg++){
			for (t=yr1; t < bm->RBCestimation.RBCspeciesParam[species][RecDevMinYr_id]-1; t++){
				bm->RBCestimation.RBCspeciesArray[species].RecDevs[ireg][t] = 0.0;
				//bm->RBCestimation.RBCspeciesArray[species].HaveRecDev[ireg][t] = 0;
			}
		}
	}

// set up rec devs in past if not read in and for future projection (past rec devs are read in if RecDevFlag=true)
	if (bm->RBCestimation.RBCspeciesParam[species][RecDevFlag_id])
		year1 = bm->RBCestimation.RBCspeciesParam[species][RecDevMaxYr_id] + 1;
	else
		year1 = yr1;

	for (t=year1;t<=yr2;t++){
		for (ireg=0; ireg < bm->K_num_reg; ireg++){
			sg[ireg] = bm->RBCestimation.RBCspeciesArray[species].SigmaR[t];
			for (jreg=0; jreg < bm->K_num_reg; jreg++){
				tt[ireg][jreg] = bm->RBCestimation.RBCspeciesArray[species].CorRecDevs[ireg][jreg];
			}
		}
		if (bm->RBCestimation.RBCspeciesParam[species][AutoCorRecDev_id] && (bm->K_num_reg == 0)){     // generate auto-correlated rec devs  (only if one region)
			/// determine what last rec dev was
			if (bm->RBCestimation.RBCspeciesArray[species].RecDevs[0][t-1] >= bm->RBCestimation.RBCspeciesParam[species][RecDevStdev_id]) from = 0;                      // > 1 stdev
			else if ((bm->RBCestimation.RBCspeciesArray[species].RecDevs[0][t-1] < bm->RBCestimation.RBCspeciesParam[species][RecDevStdev_id]) && (bm->RBCestimation.RBCspeciesArray[species].RecDevs[0][t-1] >= 0)) from = 1; // between and 0 and 1 srdev
			else if (bm->RBCestimation.RBCspeciesArray[species].RecDevs[0][t-1] < - bm->RBCestimation.RBCspeciesParam[species][RecDevStdev_id]) from = 3;                // <-1 stdev
			else from = 2;                                                   // between 0 and -1 stdev

			// generate next rec dev according to observed past probs
			getNewDev(bm, species, from, devs, means, tt, sg);
		} else
			Util_GenMnorm(devs, means, &bm->RBCestimation.Iseedz, bm->K_num_reg, tt, sg);


		for (ireg = 0; ireg < bm->K_num_reg; ireg++){
			if ((bm->RBCestimation.RBCspeciesParam[species][tier_id] == 0) || (bm->RBCestimation.RBCspeciesArray[species].SigmaR[t] == 0.0)) { // no future fishing(test for convergence to SB0) or no rec dev this year
				bm->RBCestimation.RBCspeciesArray[species].RecDevs[ireg][t] = 0.0;
				//bm->RBCestimation.RBCspeciesArray[species].HaveRecDev[ireg][t] = 0;
			} else {
				// RecDevs(ireg,t) = devs(ireg);    // for different rec devs by region - commented out by Sally
				bm->RBCestimation.RBCspeciesArray[species].RecDevs[ireg][t] = devs[0];         // for rec devs same in all regions
				//bm->RBCestimation.RBCspeciesArray[species].HaveRecDev[ireg][t] = 1;
			}
		}
	}

	// clean-up
	free1d(means);
	free1d(devs);
	free1d(sg);
	free2d(tt);

	return;

}
*/


/******************************************************************************************
 * Setup up some of the indices required during read-in
 *
 ******************************************************************************************/
void PreAllocate_Index_Setting(MSEBoxModel *bm) {
    /* Now read-in from file
    int groupIndex;
    
    for (groupIndex = 0; groupIndex < bm->K_num_tot_sp; groupIndex++) {
        bm->RBCestimation.RBCspeciesParam[groupIndex][NumRegions_id] = (int) (FunctGroupArray[groupIndex].numStocks);
    }
    */
    
}


/******************************************************************************************
 * Setup up the tiered assessment structures and parameters not read in form assess.prm
 *
 ******************************************************************************************/

int Tier_Assessment_Setup(MSEBoxModel *bm) {

	int groupIndex, n, s, tot_nf, b, nreg, nf, m;
    // int mint, maxt, t;
	//int r = 1;   // Sally's work assumes one region used to do the recruitment deviations calculation
	double totTAC = 0.0;
    double reg_area = 0.0;
    double regscale = 1.0;
    double totarea = 0.0;
	//double ssq, sum;
    double linf, kpar, t0;
	double Nsexes = (double)(bm->K_num_sexes);
    int nLen, nt, mid;
    int Nregions;
    int Ntriggers;
    int Nfleet;
    
    bm->RBCestimation.LastMetier = -1;
    bm->RBCestimation.sim = 0; // In theory could loop over but deterministic so won't bother

    /* Transfer data from Atlantis data structures into RBC data structures */
	for (groupIndex = 0; groupIndex < bm->K_num_tot_sp; groupIndex++) {
        
        if (FunctGroupArray[groupIndex].isAssessed == TRUE) {
            bm->RBCestimation.Lastsp = groupIndex;
        }
        printf("Doing tier assessment setup for %s\n", FunctGroupArray[groupIndex].groupCode);
        fflush(stdout);
        fflush(stderr);

        Ntriggers = bm->RBCestimation.NumTriggers;
        Nregions = (int)(bm->RBCestimation.RBCspeciesParam[groupIndex][NumRegions_id]);
        nLen = (int)(bm->RBCestimation.RBCspeciesParam[groupIndex][Nlen_id]);

        if ((FunctGroupArray[groupIndex].speciesParams[tier_id] == 0) || (FunctGroupArray[groupIndex].speciesParams[assess_flag_id] == 0)) { // Not needed as not assessed
            
            printf("%s with tier %f assess_flag: %f so no further RBC set up required\n", FunctGroupArray[groupIndex].groupCode, FunctGroupArray[groupIndex].speciesParams[tier_id], FunctGroupArray[groupIndex].speciesParams[assess_flag_id]);
            continue;
        }
        
        if ( (FunctGroupArray[groupIndex].speciesParams[tier_id] > 0) && (bm->RBCestimation.RBCspeciesParam[groupIndex][MaxAge_id] > bm->RBCestimation.OverallMaxAge )) {
            quit("MaxAge (%e) for %s is > RBCestimation.OverallMaxAge (%d), please reset RBCestimation.OverallMaxAge to the same value to MaxAge or numcohorts * ageclass size (as listed in the Groups.csv file) which ever is larger",
                 bm->RBCestimation.RBCspeciesParam[groupIndex][MaxAge_id], FunctGroupArray[groupIndex].groupCode, bm->RBCestimation.OverallMaxAge);
        }

        for (n = 0; n < nLen; n++) {
			bm->RBCestimation.RBCspeciesArray[groupIndex].LoLenBin[n] = FunctGroupArray[groupIndex].speciesParams[allometic_bin_start_id]
					+ FunctGroupArray[groupIndex].speciesParams[allometic_bin_size_id] * n;
            
            //fprintf(bm->logFile, "n: %d, LoLenBin: %e bin_start: %e bin_size: %e\n", n, bm->RBCestimation.RBCspeciesArray[groupIndex].LoLenBin[n], FunctGroupArray[groupIndex].speciesParams[allometic_bin_start_id], FunctGroupArray[groupIndex].speciesParams[allometic_bin_size_id]);
		}
        
        Nfleet = (int)(bm->RBCestimation.RBCspeciesParam[groupIndex][NumFisheries_id]);
        if (bm->useMultispAssess) {
            for (m = 0; m < Nfleet; m++) {
                mid = bm->RBCestimation.speciesRPFleetToMetier[m][groupIndex];  // metier for this fishery
                bm->RBCestimation.speciesMetierToRPFleet[groupIndex][mid] = m;
                strcpy(bm->RBCestimation.metierArray[mid].metierCode, FisheryArray[m].fisheryCode);
                bm->RBCestimation.metierArray[mid].PGMSYlinks = PGMSYLinksInputs[mid];
            }
        }
        
        for (m = 0; m < Nfleet; m++) {
            nf = bm->RBCestimation.speciesRPFleetToMetier[m][groupIndex];
            if (nf > bm->RBCestimation.LastMetier) {
                bm->RBCestimation.LastMetier = nf;
            }
        }
        
        bm->RBCestimation.RBCspeciesParam[groupIndex][mgt_HCRtarget_id] = FunctGroupArray[groupIndex].speciesParams[BrefA_id];
        bm->RBCestimation.RBCspeciesParam[groupIndex][mgt_HCRbreak_id] = FunctGroupArray[groupIndex].speciesParams[BrefB_id];
        bm->RBCestimation.RBCspeciesParam[groupIndex][mgt_HCRlimit_id] = FunctGroupArray[groupIndex].speciesParams[Blim_id];
        bm->RBCestimation.RBCspeciesParam[groupIndex][mgt_category_id] = whichrefi[groupIndex];
        
		// Find original TAC and number of active fisheries for the species
		tot_nf = 0;
		totTAC = 0.0;
		for (n = 0; n < bm->K_num_fisheries; n++){
            if(FunctGroupArray[groupIndex].isTAC || (bm->TACamt[groupIndex][n][now_id] < no_quota)) {
                totTAC += bm->TACamt[groupIndex][n][now_id];
            }
            if ((int)(bm->SP_FISHERYprms[groupIndex][n][assess_nf_id]) > tot_nf) {
                tot_nf = (int)bm->SP_FISHERYprms[groupIndex][n][assess_nf_id];
            }
		}
        tot_nf++;
        
        bm->RBCestimation.RBCspeciesParam[groupIndex][TACinitial_id] = totTAC;
		bm->RBCestimation.RBCspeciesParam[groupIndex][RBC_old_id] = totTAC;
		bm->RBCestimation.RBCspeciesParam[groupIndex][EstB0_id] = bm->estBo[groupIndex];
		bm->RBCestimation.RBCspeciesParam[groupIndex][EstBcurr_id] = bm->RBCestimation.RBCspeciesParam[groupIndex][EstB0_id] * bm->RBCestimation.RBCspeciesParam[groupIndex][EstDepletion_id];
        
        if (tot_nf > bm->RBCestimation.RBCspeciesParam[groupIndex][NumFisheries_id]) {
            quit("max number of fisheries from assess_nf (%d) exceeds defined NumFisheries (%d), please correct in assess.prm\n", tot_nf, bm->RBCestimation.RBCspeciesParam[groupIndex][NumFisheries_id]);
        } else {
            bm->RBCestimation.RBCspeciesParam[groupIndex][NumFisheries_id] = tot_nf;
            // To save iteration space
        }
        
        // average growth parameters over sexes
		bm->RBCestimation.RBCspeciesParam[groupIndex][Tier3_S95_id] = bm->RBCestimation.RBCspeciesParam[groupIndex][Tier3_S50_id] - (log(1.0/0.95 - 1.0)/log(3.0)) * (bm->RBCestimation.RBCspeciesParam[groupIndex][Tier3_S50_id] - bm->RBCestimation.RBCspeciesParam[groupIndex][Tier3_S25_id]);

		linf = 0.0;
		kpar = 0.0;
		t0 = 0.0;
		for (s = 0; s < bm->K_num_sexes; s++){
			linf += bm->RBCestimation.RBCspeciesArray[groupIndex].Tier3_Linf[s];
			kpar += bm->RBCestimation.RBCspeciesArray[groupIndex].Tier3_k[s];
			t0 += bm->RBCestimation.RBCspeciesArray[groupIndex].Tier3_t0[s];
		}
		linf = linf / Nsexes;
		kpar = kpar / Nsexes;
		t0 = t0 / Nsexes;
        
        // calculate ages at 25 %  50% and 95 % selectivity
		bm->RBCestimation.RBCspeciesParam[groupIndex][AgeSel95_id] = ceil(-1.0 * log(1.0 - bm->RBCestimation.RBCspeciesParam[groupIndex][Tier3_S95_id] / linf) / kpar + t0);
		bm->RBCestimation.RBCspeciesParam[groupIndex][AgeSel50_id] = ceil(-1.0 * log(1.0 - bm->RBCestimation.RBCspeciesParam[groupIndex][Tier3_S50_id] / linf) / kpar + t0);
		bm->RBCestimation.RBCspeciesParam[groupIndex][AgeSel25_id] = ceil(-1.0 * log(1.0 - bm->RBCestimation.RBCspeciesParam[groupIndex][Tier3_S25_id] / linf) / kpar + t0);

		bm->RBCestimation.RBCspeciesParam[groupIndex][Tier3_a_id] = FunctGroupArray[groupIndex].speciesParams[allometic_li_a_id];
		bm->RBCestimation.RBCspeciesParam[groupIndex][Tier3_b_id] = FunctGroupArray[groupIndex].speciesParams[allometic_li_b_id];
        
        // do AtLoadArrays() - basically those that don't get dimenional info until after load but needed in setup before post load routines
        AtLoadArrays(bm, groupIndex);
        
		// calculate Freference values and av length at F
		getF20_40_48_init_len(bm, groupIndex);
        
        // set flag that need to generate historical data
		bm->RBCestimation.RBCspeciesParam[groupIndex][HistDataInit_id] = FALSE;
        
		// identify the ID of the group that is the last one with a tiered assessment
		if((!FunctGroupArray[groupIndex].speciesParams[flagFonly_id]) && (FunctGroupArray[groupIndex].speciesParams[assess_flag_id] > 0)
				 && (groupIndex > bm->RBCestimation.lastTierSp))
			bm->RBCestimation.lastTierSp = groupIndex;

        totarea = 0.0;
        
        // Set up the area weighting for harvest strategies using triggers by fishery region (zone)
        for (nt=0; nt < Ntriggers; nt++) {
            bm->RBCestimation.RBCspeciesArray[groupIndex].TriggerPoints[nt][Nregions] = TrigPts[nt];
        }
        
        for (nreg = 0; nreg < Nregions; nreg++) {
            reg_area = 0.0;
            if (Nregions == 1) {
                regscale = 1.0;
            } else {
                for (b=0 ; b < bm->nbox; b++) {
                    if (bm->RBCestimation.RBCspeciesArray[groupIndex].regID[b] == nreg)
                        reg_area += bm->boxes[b].area;
                    if (!nreg)
                        totarea += bm->boxes[b].area;
                }
                if( totarea )
                    regscale = reg_area / totarea;
                else
                    regscale = 1.0;
            }
            
            for (nt=0; nt < Ntriggers; nt++) {
                bm->RBCestimation.RBCspeciesArray[groupIndex].TriggerPoints[nt][nreg] = bm->RBCestimation.RBCspeciesArray[groupIndex].TriggerPoints[nt][Nregions] * regscale;
            }
        }
        
	}
    
    if (bm->RBCestimation.Lastsp > 0) {
        bm->RBCestimation.Lastsp++;  // So don't cut off the final species
    }
    
    if (bm->RBCestimation.LastMetier > -1) {
        bm->RBCestimation.LastMetier++; // So don't cut off the final fleet
    }
    
    /* Clean up unneeded readin vectors */
    if(TrigPts) {
        free1d(TrigPts);
    }
    if(PGMSYLinksInputs) {
        i_free1d(PGMSYLinksInputs);
    }
    
	return 0;
}

/******************************************************************************
 *
 *  Name: errfun
 *  Description: calculate integration of normal dsn from xlim to negative infinity
 *                derived from  Numerical Recipes sec 6.2
 *                (complementary err fn converted to normal)
 *                got this version from Robin
 *
 ******************************************************************************/
static double Assess_errfun (double xlim){
    double dxval, dzval, dt, dans, i_ans;

    dxval = xlim / sqrt(2.0);
    dzval = sqrt(dxval * dxval);
    dt = 1.0/(1.0 + 0.5 * dzval);
    dans = dt * exp(-dzval * dzval - 1.26551223 + dt * (1.00002368 + dt *
           (0.37409196 + dt * (0.09678418 + dt * (-0.18628806 + dt *
           (0.27886807 + dt * (-1.13520398 + dt * (1.48851587 + dt *
           (-0.82215223 + dt * 0.17087277)))))))));
    if (dxval < 0) dans = 2.0 - dans;
    i_ans = 1 - 0.5 * dans;

	return i_ans;
}
