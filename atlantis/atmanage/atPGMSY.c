/**
 \file atPGMSY.c
 \brief C file for running the PGMSY harvest control rules
 \ingroup atManageLib
 \author Beth Fulton 13/9/2023

  Based on R script from Andre Punt

************************************************************************************************/

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>
#include <sys/stat.h>
#include <unistd.h>
#include "atManage.h"
#include "atHarvestLib.h"
#include <time.h>

/* Prototypes */
static FILE * Init_PGMSY_File(MSEBoxModel *bm);
static FILE * PGMSYfp;

double Get_Projection_Selectivity(MSEBoxModel *bm, int sp, double li, int sel_curve, double lsm, double sigma);
double RunTier4Projection(MSEBoxModel *bm, int species, int this_year, double **Fmatrix);

void Initialise_PGMSY(MSEBoxModel *bm);
void Free_PGMSY(MSEBoxModel *bm);
void GetMultipliers(MSEBoxModel *bm, int sp, int this_year, double **Fmatrix);
void RunPGMSYProjection(MSEBoxModel *bm, int species, int this_year, double **Fmatrix);
void WritePGMSYStep(FILE *fid, MSEBoxModel *bm, int sp, int this_year, int iteration, double **Fmatrix) ;


/************************************************************************************************************
 *
 *    \brief Initialise PGMSY arrays and Create PGMSY output file
 *
 */
void Initialise_PGMSY(MSEBoxModel *bm) {
    int maxage, sp;
    
    ASSESS_PROJECTION = (AssessProjectionStruct *) malloc(sizeof(AssessProjectionStruct) * (size_t)(bm->K_num_tot_sp + 1));
    
    for (sp = 0; sp < bm->K_num_tot_sp; sp++) {
        if( FunctGroupArray[sp].isImpacted == TRUE) {
            maxage = FunctGroupArray[sp].ageClassSize * FunctGroupArray[sp].numCohortsXnumGenes;
            
            ASSESS_PROJECTION[sp].DEN = Util_Alloc_Init_2D_Double(bm->RBCestimation.ProjYr, maxage, 0.0);
            ASSESS_PROJECTION[sp].LENGTH = Util_Alloc_Init_1D_Double(maxage, 0);
            ASSESS_PROJECTION[sp].WEIGHT = Util_Alloc_Init_1D_Double(maxage, 0);
            ASSESS_PROJECTION[sp].BIO = Util_Alloc_Init_2D_Double(bm->RBCestimation.ProjYr, maxage, 0.0);
            ASSESS_PROJECTION[sp].totavailB = Util_Alloc_Init_1D_Double(bm->K_num_fisheries, 0);
            ASSESS_PROJECTION[sp].Catch = Util_Alloc_Init_2D_Double(bm->RBCestimation.ProjYr, bm->K_num_fisheries, 0.0);
            ASSESS_PROJECTION[sp].SSB = Util_Alloc_Init_1D_Double(maxage, 0);
            ASSESS_PROJECTION[sp].Depleted = Util_Alloc_Init_1D_Double(maxage, 0);
            ASSESS_PROJECTION[sp].Fout = Util_Alloc_Init_2D_Double(bm->RBCestimation.ProjYr, bm->K_num_fisheries, 0.0);
            ASSESS_PROJECTION[sp].Fused = Util_Alloc_Init_1D_Double(bm->K_num_fisheries, 0);
            ASSESS_PROJECTION[sp].RBC = Util_Alloc_Init_1D_Double(bm->K_num_fisheries, 0);
        }
    }
    
    PGMSYfp = Init_PGMSY_File(bm);
    
    return;
}

/************************************************************************************************************
 *
 *    \brief Free PGMSY
 *
 */
void Free_PGMSY(MSEBoxModel *bm) {
    int sp;
    
    for (sp = 0; sp < bm->K_num_tot_sp; sp++) {
        if( FunctGroupArray[sp].isImpacted == TRUE) {
            free2d(ASSESS_PROJECTION[sp].DEN);
            free(ASSESS_PROJECTION[sp].LENGTH);
            free(ASSESS_PROJECTION[sp].WEIGHT);
            free2d(ASSESS_PROJECTION[sp].BIO);
            free(ASSESS_PROJECTION[sp].totavailB);
            free2d(ASSESS_PROJECTION[sp].Catch);
            free(ASSESS_PROJECTION[sp].SSB);
            free(ASSESS_PROJECTION[sp].Depleted);
            free2d(ASSESS_PROJECTION[sp].Fout);
            free(ASSESS_PROJECTION[sp].Fused);
            free(ASSESS_PROJECTION[sp].RBC);
        }
    }
    
    free(ASSESS_PROJECTION);
    
    return;
}

/************************************************************************************************************
 *
 *    \brief Routine to initialise the diet check output file.
 *
 */
static FILE * Init_PGMSY_File(MSEBoxModel *bm) {
    FILE *fid;
    char fname[STRLEN];

    /** Create filename **/
    snprintf(fname, sizeof(fname), "%sPGMSY.txt", bm->startfname);

    /** Create file **/
    if ((fid = Util_fopen(bm, fname, "w")) == NULL)
        quit("Init_PGMSY_File: Can't open %s\n", fname);

    /** Column definitions **/
    fprintf(fid,"Time Species Iteration Fleet AssessF Factual Fupdate FratioFleet RBCupdate Depletion ProjectedDepletion\n");
    
    /* Return file pointer */
    return (fid);
}

/************************************************************************************************************
 *
 *    \brief Routine to iwrite out the data to the PGMSY file.
 *
 */
void WritePGMSYStep(FILE *fid, MSEBoxModel *bm, int sp, int this_year, int iteration, double **Fmatrix) {
    int nf, f;
    int iYr = bm->RBCestimation.ProjYr;
    
    for (nf = 0; nf < bm->K_num_fisheries; nf++) {
        f = (int)bm->SP_FISHERYprms[sp][nf][assess_nf_id];
        fprintf(fid,"%d %s %d %s %d %e %e %e %e %e %e %e\n", this_year, FunctGroupArray[sp].groupCode, iteration, FisheryArray[nf].fisheryCode, f, bm->RBCestimation.F_actFleet[sp][f], Fmatrix[f][this_year], bm->RBCestimation.FratioFleet[sp][f], bm->RBCestimation.RBCspeciesArray[sp].RBCupdated[nf][this_year], bm->RBCestimation.RBCspeciesArray[sp].CatchStore[f][this_year], ASSESS_PROJECTION[sp].Depleted[1], ASSESS_PROJECTION[sp].Depleted[iYr]);
    }
}

/********************************************************************************************************
 * \brief  Getting multipliers across fleets
 * Called from Do_Atlantis_PGMSY()
 */
void GetMultipliers(MSEBoxModel *bm, int sp, int this_year, double **Fmatrix) {
    double nfleets = 0;
    int nf;
    
    // Find the mean F (by fleet) for the last five years of the assessment
    for (nf = 0; nf < bm->K_num_fisheries; nf++) {
        // Find current fishable biomass so can find F per fishery
        bm->RBCestimation.F_actFleet[sp][nf] = Fmatrix[nf][this_year];
        
        if ( bm->SP_FISHERYprms[sp][nf][q_id] > 0 ) {
            bm->RBCestimation.RBCspeciesParam[sp][AveF_id] += bm->RBCestimation.F_actFleet[sp][nf];
            nfleets += 1.0;
        }
    }
    bm->RBCestimation.RBCspeciesParam[sp][AveF_id] /= nfleets;
    if (bm->RBCestimation.RBCspeciesParam[sp][AveF_id] > 0) {
        for (nf = 0; nf < bm->K_num_fisheries; nf++) {
            bm->RBCestimation.FratioFleet[sp][nf] = bm->RBCestimation.F_actFleet[sp][nf] / bm->RBCestimation.RBCspeciesParam[sp][AveF_id];
        }
    }

    return;
 }

/**==================================================================================
 \brief  Get_Selectivity
    Called from RunPGMSYProjection()
 **/

double Get_Projection_Selectivity(MSEBoxModel *bm, int sp, double li, int sel_curve, double lsm, double sigma) {

    double sel = 0.0, sel_b, sel_lsm, step1, step2, sel_sigma;

    switch (sel_curve) {
    case q_const_id: /* Group specific constant */
        sel = lsm;
        break;
    case q_ageconst_id: /* Constant proportion of maturity stage - also used for stages in cephalopods */
        sel = lsm;
        break;
    case q_logistic_id: /* Dynamic so selectivity based on size, logistic */
        sel_b = sigma;
        sel_lsm = lsm;
        sel = 1.0 / (1.0 + exp(-sel_b * (li - sel_lsm)));
        break;
    case q_norm_id: /* Dynamic so selectivity based on size, normal */
        sel_lsm = lsm;
        sel_sigma = sigma;
        step1 = li - sel_lsm;
        step2 = -(step1 * step1) / (2.0 * sel_sigma * sel_sigma + small_num);
        sel = exp(step2);
        break;
    case q_lognorm_id: /* Dynamic so selectivity based on size, lognormal */
        sel_lsm = lsm;
        sel_sigma = sigma;
        sel = Util_Lognorm_Distrib(sel_lsm, sel_sigma, li);
        break;
    case q_gamma_id: /* Dynamic so selectivity based on size, gamma */
        sel_lsm = lsm + small_num;
        sel_sigma = sigma;
        step1 = sel_lsm * sel_lsm + 4.0 * sel_sigma * sel_sigma;
        step2 = (sqrt(step1) - sel_lsm) / 2.0 + small_num;
        sel = pow((li / sel_lsm), (sel_lsm / step2)) * exp((sel_lsm - li) / step2);
        break;
    case q_knife_id: /* Knife edged */
        if (FunctGroupArray[sp].isVertebrate == TRUE) {
            /* For vertebrates check to see if over minimum length */
            sel_lsm = lsm;
            
            if (li < sel_lsm)
                sel = 0.0;
            else
                sel = sigma;
        } else {
            /* For invertebrates assume all equally available as no size check possible */
            sel = sigma;
        }
        break;
    case q_bimodal_id: /* Bimodal "normal" selectivity curve based on size */
    case q_binormal_id: /* Bimodal "normal" selectivity curve based on size */
    default:
        quit("No such selectivity curve defined (%d) for porjection assessments - value must be between 0 and %d currently\n", sel_curve, q_knife_id);
        break;
    }

    /* Ensure selecitivity is bounded between zero and one */
    if (sel > 1.0)
        sel = 1.0;
    if (sel < 0.0)
        sel = 0.0;

    return sel;
}


/**==================================================================================
 \brief  RunPGMSYProgection for PGMSY calculations
    Called from Do_Atlantis_PGMSY()
 **/

void RunPGMSYProjection(MSEBoxModel *bm, int species, int this_year, double **Fmatrix) {
    double linf, Kbert, tzero, li_a, li_b, pR_sp, age_mat, Length, SSB, lsm, sigma,
        nums, ageclasssize, Density, age_class_part, totF, sel, q, catch, recruitNum,
        AveF = 0.0, AveC = 0.0, ncount = 0.0;
    int cohort, age, nf, iYr, Nfleets, stage, base_chort, max_base_cohort, f, sel_curve;
    double BHalpha_sp = bm->RBCestimation.RBCspeciesParam[species][PGMSYBHalpha_id];
    double BHbeta_sp = bm->RBCestimation.RBCspeciesParam[species][PGMSYBHbeta_id];
    double nat_mort = FunctGroupArray[species].speciesParams[assess_nat_mort_id];
    int AveYr = (int)bm->RBCestimation.RBCspeciesParam[species][Tier4_avtime_id];

    //int pid;

    Density = 1.0;
    linf = FunctGroupArray[species].speciesParams[linf_id];
    Kbert = FunctGroupArray[species].speciesParams[Kbert_id];
    tzero = FunctGroupArray[species].speciesParams[tzero_id];
    li_a = FunctGroupArray[species].speciesParams[li_a_id];
    li_b = FunctGroupArray[species].speciesParams[li_b_id];
    pR_sp = FunctGroupArray[species].speciesParams[pR_id];
    age_mat = FunctGroupArray[species].speciesParams[age_mat_id];
    ageclasssize = FunctGroupArray[species].ageClassSize;
    age_class_part = 1.0 / ((double)(ageclasssize));
    Nfleets = (int)(bm->RBCestimation.RBCspeciesParam[species][NumFisheries_id]);

    // Set up starting population
    for (cohort = 0; cohort < FunctGroupArray[species].numCohortsXnumGenes; cohort++) {
        age = ageclasssize * cohort;
        ASSESS_PROJECTION[species].DEN[age][0] = bm->tot_cohort[species][cohort] * age_class_part;
        ASSESS_PROJECTION[species].LENGTH[age] = Length = linf - (linf * exp(-1.0 * Kbert * (age - tzero)));
        ASSESS_PROJECTION[species].WEIGHT[age] = li_a * pow(Length,li_b) * 1000.0 / (bm->k_wetdry * bm->X_CN);
        ASSESS_PROJECTION[species].BIO[age][0] = ASSESS_PROJECTION[species].DEN[age][0] * ASSESS_PROJECTION[species].WEIGHT[age];
    }

    for (nf = 0; nf < Nfleets; nf++) {
        ASSESS_PROJECTION[species].RBC[nf] = 0.0;
    }

    for (nf = 0; nf < bm->K_num_fisheries; nf++) {
        f = (int)bm->SP_FISHERYprms[species][nf][assess_nf_id];
        if (bm->RBCestimation.RBCspeciesArray[species].RBCupdated[nf][this_year] < no_quota) {
            ASSESS_PROJECTION[species].RBC[f] += bm->RBCestimation.RBCspeciesArray[species].RBCupdated[nf][this_year];
        }
    }
    for (f = 0; f < Nfleets; f++) {
        ASSESS_PROJECTION[species].Fused[f] = Fmatrix[f][this_year];
        ASSESS_PROJECTION[species].Catch[f][0] = bm->RBCestimation.RBCspeciesArray[species].AvgCatFleet[this_year][f];
    }

    // Project forward
    for (iYr = 1; iYr < bm->RBCestimation.ProjYr ; iYr++) {
        SSB = 0.0;
        totF = 0.0;
        for (f=0; f < Nfleets; f++) {
            ASSESS_PROJECTION[species].totavailB[f] = 0.0;
            ASSESS_PROJECTION[species].Catch[f][iYr] = 0.0;
        }
        for (cohort = 0; cohort < FunctGroupArray[species].numCohortsXnumGenes; cohort++) {
            //  Natural Mortality calculation
            if (cohort < age_mat) {
                stage = juv_id;
            } else {
                stage = adult_id;
            }
            base_chort = cohort * ageclasssize;
            max_base_cohort = (cohort + 1) * ageclasssize;

            for (age = base_chort; age < max_base_cohort; age++) {
                nums = ASSESS_PROJECTION[species].DEN[age][iYr-1];
                
                // Get projected catch by applying F
                for (nf = 0; nf < Nfleets; nf++) {
                    lsm = bm->RBCestimation.RBCspeciesArray[species].PGMSY_sel_lsm[nf];
                    sigma = bm->RBCestimation.RBCspeciesArray[species].PGMSY_sel_sigma[nf];
                    sel_curve = (int)(bm->RBCestimation.RBCspeciesArray[species].PGMSY_selcurve[nf]);
                    
                    // get selectivity based on size at age
                    sel = Get_Projection_Selectivity(bm, species, ASSESS_PROJECTION[species].LENGTH[age], sel_curve, lsm, sigma);
                    
                    // get final catch
                    q = bm->RBCestimation.RBCspeciesArray[species].PGMSY_q[nf];
                    ASSESS_PROJECTION[species].totavailB[nf] += ASSESS_PROJECTION[species].BIO[age][iYr] * sel * q;
                    catch = ASSESS_PROJECTION[species].BIO[age][iYr] * sel * q * ASSESS_PROJECTION[species].Fused[nf];
                    totF += ASSESS_PROJECTION[species].Fused[nf];
                    
                    if (catch > ASSESS_PROJECTION[species].RBC[nf])
                        catch = ASSESS_PROJECTION[species].RBC[nf];
                    ASSESS_PROJECTION[species].Catch[nf][iYr] += catch;
                }
        
                // Final numbers
                if (age > 0) {
                    ASSESS_PROJECTION[species].DEN[age][iYr] = ASSESS_PROJECTION[species].DEN[age-1][iYr-1] * exp(-1.0 * (nat_mort + totF));
                } else {
                    // Get info for recruiment
                    SSB += FunctGroupArray[species].scaled_FSPB[cohort] * ASSESS_PROJECTION[species].DEN[age][iYr];
                }
            }
        }
        // Do recruitment
        ASSESS_PROJECTION[species].SSB[iYr] = SSB;
        recruitNum = BHalpha_sp * SSB / (BHbeta_sp + SSB);
        ASSESS_PROJECTION[species].DEN[0][iYr] = recruitNum;
        ASSESS_PROJECTION[species].Depleted[iYr] = SSB / bm->estBo[species];
        
        // Update F
        for (nf = 0; nf < Nfleets; nf++) {
            ASSESS_PROJECTION[species].Fout[nf][iYr] = ASSESS_PROJECTION[species].Catch[nf][iYr] / ASSESS_PROJECTION[species].totavailB[nf];
        }
    }
    
    // Store updated F
    for (nf = 0; nf < Nfleets; nf++) {
        bm->RBCestimation.RBCspeciesArray[species].RBCupdated[nf][this_year] = 0.0;
        AveF = 0.0;
        AveC = 0.0;
        ncount = 0.0;
        for (iYr = 1; iYr < AveYr ; iYr++) {
            AveF += ASSESS_PROJECTION[species].Fout[nf][iYr];
            AveC += ASSESS_PROJECTION[species].Catch[nf][iYr];
            bm->RBCestimation.RBCspeciesArray[species].RBCupdated[nf][this_year] += ASSESS_PROJECTION[species].Catch[nf][iYr];
            ncount += 1.0;
        }
        AveF /= ncount;
        AveC /= ncount;
        bm->RBCestimation.RBCspeciesArray[species].Fupdated[nf][this_year] = AveF;
        bm->RBCestimation.RBCspeciesArray[species].CatchStore[nf][this_year] = AveC;
        
        // Store RBCs (by fleet)
        bm->RBCestimation.RBCspeciesArray[species].RBCupdated[nf][this_year] /= ncount;
    }

    return;

}

/**==================================================================================
 \brief  RunPGMSYProjection for PGMSY calculations
    Called from Do_Atlantis_PGMSY()
 **/

double RunTier4Projection(MSEBoxModel *bm, int species, int this_year, double **Fmatrix) {
    double minDepletion = 1.0;
    double AveF = 0.0;
    double ncount = 0.0;
    double totcatch = 0.0;
    double thistotcatch = 0.0;
    double step1, step2;
    int iYr, nf, f;
    int AveYr = (int)bm->RBCestimation.RBCspeciesParam[species][Tier4_avtime_id];
    int Nfleets = (int)(bm->RBCestimation.RBCspeciesParam[species][NumFisheries_id]);
    
    /* This re-uses information from the dynamic tier 4 assessment:
     FFs
     Tier4_r_id
     Tier4_Carry_id
     Tier4_z_id
     EstBcurr_id
     */

    // Compute average F over the last 10 years
    for (iYr = 1; iYr < AveYr ; iYr++) {
        AveF += 1.0 / (1.0 + exp(-1.0 * bm->RBCestimation.RBCspeciesArray[species].FFs[iYr]));
        for (nf = 0; nf < Nfleets; nf++) {
            totcatch += bm->RBCestimation.RBCspeciesArray[species].AvgCatFleet[iYr][nf];
        }
        ncount += 1.0;
    }
    AveF /= ncount;

    // project forward - use age 0 slot for convenience
    ASSESS_PROJECTION[species].BIO[0][0] = bm->RBCestimation.RBCspeciesParam[species][EstBcurr_id];
    for (nf = 0; nf < Nfleets; nf++) {
        ASSESS_PROJECTION[species].RBC[nf] = 0.0;
    }
    for (nf = 0; nf < bm->K_num_fisheries; nf++) {
        f = (int)bm->SP_FISHERYprms[species][nf][assess_nf_id];
        if (bm->RBCestimation.RBCspeciesArray[species].RBCupdated[nf][this_year] < no_quota) {
            ASSESS_PROJECTION[species].RBC[f] += bm->RBCestimation.RBCspeciesArray[species].RBCupdated[nf][this_year];
        }
    }
    for (nf = 0; nf < Nfleets; nf++) {
        ASSESS_PROJECTION[species].Catch[nf][0] = AveF * ASSESS_PROJECTION[species].BIO[0][0] * bm->RBCestimation.RBCspeciesArray[species].AvgCatFleet[0][nf] / (totcatch + small_num);
        if (ASSESS_PROJECTION[species].RBC[nf] > ASSESS_PROJECTION[species].Catch[nf][0])
            ASSESS_PROJECTION[species].RBC[nf] = ASSESS_PROJECTION[species].Catch[nf][0];
    }
    
    minDepletion = 1.0;
    for (iYr = 1; iYr < bm->RBCestimation.ProjYr ; iYr++) {
        thistotcatch = 0.0;
        for (nf = 0; nf < Nfleets; nf++) {
            ASSESS_PROJECTION[species].Catch[nf][iYr] = AveF * ASSESS_PROJECTION[species].BIO[0][iYr-1] * bm->RBCestimation.RBCspeciesArray[species].AvgCatFleet[iYr][nf] / (totcatch + small_num);
            if (ASSESS_PROJECTION[species].Catch[nf][iYr] > ASSESS_PROJECTION[species].RBC[nf])
                ASSESS_PROJECTION[species].Catch[nf][iYr] = ASSESS_PROJECTION[species].RBC[nf];
            thistotcatch += ASSESS_PROJECTION[species].Catch[nf][iYr];
        }
        step1 = pow(ASSESS_PROJECTION[species].BIO[0][iYr-1]/ bm->RBCestimation.RBCspeciesParam[species][Tier4_Carry_id], bm->RBCestimation.RBCspeciesParam[species][Tier4_z_id]);
        step2 = ASSESS_PROJECTION[species].BIO[0][iYr-1] + bm->RBCestimation.RBCspeciesParam[species][Tier4_r_id] * ASSESS_PROJECTION[species].BIO[0][iYr-1] * ( 1.0 - step1 ) - thistotcatch;
        ASSESS_PROJECTION[species].BIO[0][iYr] = max(step2,0.000001);
        ASSESS_PROJECTION[species].Depleted[iYr] = ASSESS_PROJECTION[species].BIO[0][iYr] / bm->RBCestimation.RBCspeciesParam[species][Tier4_Carry_id];
        
        if (minDepletion > ASSESS_PROJECTION[species].Depleted[iYr]) {
            minDepletion = ASSESS_PROJECTION[species].Depleted[iYr];
        }
    }
    
    return minDepletion;
}


/**==================================================================================
 \brief  Undertake PGMSY calculations
 **/

void Do_Atlantis_PGMSY(MSEBoxModel *bm, int this_year) {
    int tier = -1;
    int Iteration = 0;
    int Finished = FALSE;
    int StartIt = FALSE;
    int Nfleets, Iyr, sp, nf, f, r;
    double minDepletion = 1.0, this_minDepletion, MultBest, TAC_old;
    double MultMin = 0.0;
    double MultMax = 1.0;
    double TAC = 0.0;

    int MinCyr = this_year - bm->RBCestimation.TriggerCheckPeriod;
    int MaxCyr = this_year;
    double numYear = (double)(MaxCyr - MinCyr);

    // Sanity check on years
    if (!numYear) {
        numYear = 1 ;
    }
    
    /* Main parameters
      =================
       Fleet numbers year range with data (NumFisheries)
       Length of data time series (Yr1 = Min, Yr2 = Yr3 = Max)
       Final year of the assessment
       Number of sexes included
       Maximum age considered (Nage)
     
       Also need estimate from model of SSB unfished - SSB0
     
     
     Summary table dimensions [nf][nyears] for Catch and F (so time series per fleet)
        - will need to record F in "data collection" so can do "perfect knowlegde assessment" of it if not using SS
        - will calculate depletion as part of output
     ==============
    */


    /* Don't seem to be used
    // Create the summary table
    for (Iyr = MinCyr; Iyr < MaxCyr; Iyr++) {
        for (sp = 0; sp < bm->K_num_tot_sp; sp++) {
            if (FunctGroupArray[sp].isFished == TRUE) {
                Nfleets = (int)(bm->RBCestimation.RBCspeciesParam[sp][NumFisheries_id]);

                // Get SSB and hence depletion
                bm->RBCestimation.RBCspeciesParam[sp].Depletion[currentSSB_id][iyr] = bm->totSSBpop[sp];   // Grab from start of run reference point
                bm->RBCestimation.RBCspeciesParam[sp].Depletion[BrelSSB0_id][iyr] = bm->totSSBpop[sp] / bm->totinitpop[sp];   // Grab from start of run reference point
                bm->RBCestimation.RBCspeciesParam[sp].Depletion[Brelstart_id][iyr] = bm->totSSBpop[sp] / bm->estBo[sp]; // SSB0 read in from prm file based on nofishing run
                 
                // Get total catch
                bm->RBCestimation.DeadCatch[sp] = 0;
                bm->RBCestimation.RetainedCatch[sp] = 0;
                for (nf = 0; nf < Nfleets; nf++) {
                    for (r = 0; r < bm->RBCestimation.RBCspeciesParam[sp][NumRegions_id]; r++) {
                        bm->RBCestimation.DeadCatch[sp] += bm->RBCestimation.RBCspeciesArray[sp].CatchData[nf][r][iyr];
                        bm->RBCestimation.RetainedCatch[sp] += (bm->RBCestimation.RBCspeciesArray[sp].CatchData[nf][r][iyr] + bm->RBCestimation.RBCspeciesArray[groupIndex].DiscData[nf][r][iyr]);
                    }
                }
            }
        }
    }
     */
        
    // In R script woudl assign Fval oer yr here but already stored in bm->RBCestimation.FperFleet[sp][nf] in this case
    // and catch is stored in bm->RBCestimation.RBCspeciesArray[sp].CatchData[f][r][iyr]

    /* Extract the relevant output

     In R script store ctahc and F as "Fold" and "Cold" as will modify in following steps, where
 
     Fhist = Fold in 5 years before specified reference year (like year we use as virgin yeear)
     FactFleet = Fold time series ending with msot recent year

     */
    
    // Average catch per fleet over past x (typically 9) years (or start of time series, which ever is shorter)
    for (sp = 0; sp < bm->K_num_tot_sp; sp++) {
        if (FunctGroupArray[sp].isFished == TRUE) {
            for (nf = 0; nf < bm->K_num_fisheries; nf++) { // Initlaise RBCupdated so ready to go for the first projection
                bm->RBCestimation.RBCspeciesArray[sp].RBCupdated[nf][this_year] = bm->TACamt[sp][nf][now_id];
            }
            
            Nfleets = (int)(bm->RBCestimation.RBCspeciesParam[sp][NumFisheries_id]);
            for (nf = 0; nf < Nfleets; nf++) {
                // Reinitilise depletion output vector
                bm->RBCestimation.RBCspeciesArray[sp].AvgCatFleet[this_year][nf] = 0.0;

                for (Iyr = MinCyr; Iyr < MaxCyr; Iyr++ ) {
                    for (r = 0; r < bm->RBCestimation.RBCspeciesParam[sp][NumRegions_id]; r++) {
                        bm->RBCestimation.RBCspeciesArray[sp].AvgCatFleet[this_year][nf] += bm->RBCestimation.RBCspeciesArray[sp].CatchData[nf][r][Iyr];
                    }
                }
                bm->RBCestimation.RBCspeciesArray[sp].AvgCatFleet[this_year][nf] /= numYear;
                bm->RBCestimation.RBCspeciesArray[sp].CatchStore[nf][this_year] = 0.0;
                for (r = 0; r < bm->RBCestimation.RBCspeciesParam[sp][NumRegions_id]; r++) {
                    bm->RBCestimation.RBCspeciesArray[sp].CatchStore[nf][this_year] += bm->RBCestimation.RBCspeciesArray[sp].CatchData[nf][r][this_year];
                }
            }
        }
    }
 
 
    // Extract the information from the Tier 1 assessments
    for (sp = 0; sp < bm->K_num_tot_sp; sp++) {
        if (FunctGroupArray[sp].isFished == TRUE) {
            tier = (int)(FunctGroupArray[sp].speciesParams[tier_id]);
            if (tier == tier1) {
 
                // Get multipliers and depletion from the basic run - Depletion calculated above
                GetMultipliers(bm, sp, this_year, bm->RBCestimation.RBCspeciesArray[sp].Fhist); // Get multipliers from current round of assessments
                WritePGMSYStep(PGMSYfp, bm, sp, this_year, 0, bm->RBCestimation.RBCspeciesArray[sp].Fhist);
            }
        }
    }
    
    // Run projections - based on fixed Fs and not a HCR - use External_Box_Ecology() variant to achieve this?
    for (sp = 0; sp < bm->K_num_tot_sp; sp++) {
        if (FunctGroupArray[sp].isFished == TRUE) {  // TODO: Could this be made to be isImpacted?

            RunPGMSYProjection(bm, sp, this_year, bm->RBCestimation.RBCspeciesArray[sp].Fhist);
    
            // Get multipliers and depletion from the revised results and then write out the results
            GetMultipliers(bm, sp, this_year, bm->RBCestimation.RBCspeciesArray[sp].Fupdated);
            WritePGMSYStep(PGMSYfp, bm, sp, this_year, 1, bm->RBCestimation.RBCspeciesArray[sp].Fupdated);
            
            // Skip RBCupdated adjustment done below as not a real intertion, just a step to switch from dynamic to fixed F
        }
    }

    // Now have F based starting point iterate until criteria met
    Iteration = 2;
    Finished = FALSE;
    StartIt = FALSE;
    while ( Finished == FALSE) {
        // Do Tier 1 assessments on appropriate species
        for (sp = 0; sp < bm->K_num_tot_sp; sp++) {
            if (FunctGroupArray[sp].isFished == TRUE) {  // TODO: Could this be made to be isImpacted?
                tier = (int) (FunctGroupArray[sp].speciesParams[tier_id]);
            
                if (tier != tier1) {
                    continue;
                }

                RunPGMSYProjection(bm, sp, this_year, bm->RBCestimation.RBCspeciesArray[sp].Fupdated);
            
                // Get multipliers and depletion from the basic run
                GetMultipliers(bm, sp, this_year, bm->RBCestimation.RBCspeciesArray[sp].Fupdated);
                WritePGMSYStep(PGMSYfp, bm, sp, this_year, Iteration, bm->RBCestimation.RBCspeciesArray[sp].Fupdated);
            
                // Adjust RBCs for average catch species
                for (nf = 0; nf < bm->K_num_fisheries; nf++) {
                    f = (int)bm->SP_FISHERYprms[sp][nf][assess_nf_id];
                    bm->RBCestimation.RBCspeciesArray[sp].CscalarMetier[nf] = bm->RBCestimation.RBCspeciesArray[sp].CatchStore[f][this_year] / (bm->RBCestimation.RBCspeciesArray[sp].CatchData[f][r][this_year] + small_num);
                
                    bm->RBCestimation.RBCspeciesArray[sp].RBCupdated[nf][this_year] = bm->RBCestimation.RBCspeciesArray[sp].CscalarMetier[f] * bm->TACamt[sp][nf][now_id];
                }
            }
        }
        
        //Extract the information from the Tier 4 assessments
        minDepletion = 1.0;
        for (sp = 0; sp < bm->K_num_tot_sp; sp++) {
            if (FunctGroupArray[sp].isFished == TRUE) {  // TODO: Could this be made to be isImpacted?
                tier = (int) (FunctGroupArray[sp].speciesParams[tier_id]);

                if (tier != tier4) {
                    continue;
                }
        
                this_minDepletion = RunTier4Projection(bm, sp, this_year, bm->RBCestimation.RBCspeciesArray[sp].Fupdated);

                if (minDepletion > this_minDepletion) {
                    minDepletion = this_minDepletion;
                }
            }
            
        }

        // Now tune as needed across species
        if ((StartIt == FALSE) && (minDepletion >= bm->RBCestimation.ThresholdDepletion)) {
            Finished = TRUE;
        }
        if ((StartIt == TRUE) || ((StartIt == FALSE) && (minDepletion < bm->RBCestimation.ThresholdDepletion))) {
            printf("Time: %e At least one of the species is failing in PGMSY\n", bm->dayt);
            StartIt = TRUE;
            if (Iteration == 2) {
                MultMin = 0.0;
                MultMax = 1.0;
            } else {
                if (minDepletion < bm->RBCestimation.ThresholdDepletion) {
                    MultMax = MultBest;
                } else {
                    MultMin = MultBest;
                }
            }
            if (fabs(minDepletion - bm->RBCestimation.ThresholdDepletion) < bm->RBCestimation.ThresholdBound) {
                Finished = TRUE;
            }
            if (Finished == FALSE) {
                MultBest = (MultMax + MultMin) / 2.0;
                
                Nfleets = (int)(bm->RBCestimation.RBCspeciesParam[sp][NumFisheries_id]);
                for (nf = 0; nf < Nfleets; nf++) {
                    bm->RBCestimation.RBCspeciesArray[sp].Fupdated[nf][this_year] *= MultBest;
                }
                Iteration += 1;
                RunPGMSYProjection(bm, sp, this_year, bm->RBCestimation.RBCspeciesArray[sp].Fupdated);  // So ready for the new iteration
            }
        }
        // Check if too many iterations so break out - not in R but require it here just in case
        if (Iteration > bm->RBCestimation.MaxIteration) {
            Finished = TRUE;
        }
        
        // Iteration end
    }
    
    // Store updated RCBs and TACs
    for (sp = 0; sp < bm->K_num_tot_sp; sp++) {
        if (FunctGroupArray[sp].isFished == TRUE) {  // TODO: Could this be made to be isImpacted?
            TAC = 0.0;
            for (nf = 0; nf < bm->K_num_fisheries; nf++) {
                TAC += bm->RBCestimation.RBCspeciesArray[sp].RBCupdated[nf][this_year];
            }
            bm->RBCestimation.RBCspeciesArray[sp].RBC_by_year[this_year] = TAC;

            for (nf = 0; nf < bm->K_num_fisheries; nf++) {
                if (!bm->FISHERYprms[nf][flagrecfish_id] && bm->inQuota[nf][sp]) {
                    TAC_old = bm->TACamt[sp][nf][now_id];
                    bm->TACamt[sp][nf][now_id] = bm->RBCestimation.RBCspeciesArray[sp].RBCupdated[nf][this_year];
                    bm->TACamt[sp][nf][RBCnow_id] = bm->RBCestimation.RBCspeciesArray[sp].RBCupdated[nf][this_year];
                
                    if (bm->TACamt[sp][nf][now_id] > no_quota) {
                        bm->TACamt[sp][nf][now_id] = no_quota;
                    }
            
                    fprintf(bm->logFile, "Time: %e %s fishery %s PGMSY updated TAC to %e from %e\n", bm->dayt, FunctGroupArray[sp].groupCode, FisheryArray[nf].fisheryCode, bm->TACamt[sp][nf][now_id], TAC_old);
                }
            }
        }
    }

 
    return;
}
