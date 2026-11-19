/**
 \file atSS3DataDen.c
 \brief C file for cretaing data for used in harvest control rules
 \ingroup atSS3Link
 \author Beth Fulton
 
 Based on GenData.CPP from Sally Wayte
 
3/6/2014 Made sure all masses re in kg (not mg or tonne)
 
 
 ************************************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <math.h>
#include <ctype.h>
#include <time.h>
#include <string.h>
#include <sjwlib.h>
#include <atSS3Util.h>
#include <atlantisboxmodel.h>
#include <atUtilLib.h>
#include <atSS3LinkLib.h>

void GetCatch_n_DiscardData(MSEBoxModel *bm, int groupIndex, int iyr);
void GetFishCPUEData(MSEBoxModel *bm, int iyr, int groupIndex);
void GetFishLength_n_AgeData(MSEBoxModel *bm, int groupIndex, int iyr);
void GetEnviroData(MSEBoxModel *bm, int groupIndex, int iyr);
void GetBiomassData(MSEBoxModel *bm, int groupIndex, int iyr);
void GetCloseKinNum(MSEBoxModel *bm, int groupIndex, int iyr);
void RecordHistF(MSEBoxModel *bm, int sp, int iyr);

void FindRawData(MSEBoxModel *bm, int itype, int iyr, int groupIndex, double ****tempcatch, double *****rawnumdata, double *****rawsizedata);
void GetLenProps(MSEBoxModel *bm, int groupIndex, int id, int itype, int f, int r, int s, int iyr, double **lenprops, int *iseed);
void GetAgeProps(MSEBoxModel *bm, int groupIndex, int id, int itype, int f, int r, int s, int iyr, double **ageprops, int *iseed);
void GenSample(MSEBoxModel *bm, int groupIndex, int vecsize, int nf, int r_id, int s, int itype, int sample_size, double *comp, double *****rawnumdata, double *****rawsizedata, int *iseed);
/******************************************************************************
 *
 * Name:  GenData
 * Description:  generates the data for a given year
 *
 ******************************************************************************/
void GenData(MSEBoxModel *bm, int groupIndex, int yearIndex) {
    
    if (FunctGroupArray[groupIndex].speciesParams[tier_id] > 0) {

        printf("Find which data - %s\n", FunctGroupArray[groupIndex].groupCode);

        // find out which data to generate
        FindWhichData(bm, groupIndex, yearIndex);

        printf("Get biomass - %s\n", FunctGroupArray[groupIndex].groupCode);
        
        // spawning biomass data
        GetBiomassData(bm, groupIndex, yearIndex);

        printf("Find length-weight - %s\n", FunctGroupArray[groupIndex].groupCode);
        
        // length and weight at age data
        GetLengthWeightData(bm, groupIndex, yearIndex);

        printf("Find catch and discard data - %s\n", FunctGroupArray[groupIndex].groupCode);
        
        // fishery catch and discard data
        GetCatch_n_DiscardData(bm, groupIndex, yearIndex);

        printf("Find CPUE data - %s\n", FunctGroupArray[groupIndex].groupCode);
        
       // fishery CPUE data
        GetFishCPUEData(bm, groupIndex, yearIndex);

        printf("Find length and age data - %s\n", FunctGroupArray[groupIndex].groupCode);

        // fishery length and age data
        GetFishLength_n_AgeData(bm, groupIndex, yearIndex);

        printf("Find enviro data - %s\n", FunctGroupArray[groupIndex].groupCode);

        // get environmental data
        GetEnviroData(bm, groupIndex, yearIndex);
        
        // get close kin data if required
        if(bm->CloseKinEst->UseCloseKin && FunctGroupArray[groupIndex].isCloseKin) {
            GetCloseKinNum(bm, groupIndex, yearIndex);
            CKsimulator(bm, groupIndex, yearIndex);
            bm->CloseKinEst[groupIndex].nCKsampled++;
        }
            
        // Store historical F
        RecordHistF(bm, groupIndex, yearIndex);

    } else {
        fprintf(bm->logFile, "No assessment done for %s as tier = 0\n", FunctGroupArray[groupIndex].groupCode);
    }

}

/******************************************************************************
 *
 * Name:  FindWhichData
 * Description: sets up flags for which data to generate in projection
 *
 ******************************************************************************/
void FindWhichData(MSEBoxModel *bm, int groupIndex, int iyr) {
	int nf, f, r, it, b, ij;
	double tcatch, thiscatch, thisdiscard;
	int Nfleets = (int)(bm->RBCestimation.RBCspeciesParam[groupIndex][NumFisheries_id]);
	int Nregions = (int)(bm->RBCestimation.RBCspeciesParam[groupIndex][NumRegions_id]);
    int tier = (int) (FunctGroupArray[groupIndex].speciesParams[tier_id]);

	// Find if need CPUE and discards
	tcatch = 0.0;
    Util_Init_1D_Double(bm->RBCestimation.RBCspeciesArray[groupIndex].sumcatch, Nfleets, 0.0);
    
 	for (nf = 0; nf < bm->K_num_fisheries; nf++) {
        // map operating model fishery to assessment fishery for the species
        f = (int)bm->SP_FISHERYprms[groupIndex][nf][assess_nf_id];
		for (r = 0; r < Nregions; r++) {
			for(b=0; b<bm->nbox; b++) {
				if ((Nregions == 1) || (bm->RBCestimation.RBCspeciesArray[groupIndex].regID[b] == r)) {
                    thiscatch = 0.0;
                    thisdiscard = bm->CumDiscards[groupIndex][nf][b];
					for(ij=0; ij<bm->boxes[b].nz; ij++) {
						thiscatch += bm->CumCatch[groupIndex][nf][b][ij];
					}
                    
                    // CPUE to be stored for the current year? 1 = yes (possible if catch and effort exist and need CPUE data for the assessment)
                    //if((thiscatch > 0) && (bm->RBCestimation.RBCspeciesArray[groupIndex].CPUEfuture[f] != 0))
                    if(thiscatch > 0)
                        bm->RBCestimation.RBCspeciesArray[groupIndex].CPUEyears[f][r][iyr] = 1;
                    // Discards to be stored for the current year? 1 = yes (possible if discard data exists and need discards for the assessment)
                    //if ((thisdiscard > 0) && (bm->RBCestimation.RBCspeciesArray[groupIndex].DiscFuture[f] != 0))
                    if (thisdiscard > 0)
                        bm->RBCestimation.RBCspeciesArray[groupIndex].DiscYears[f][iyr] = 1;
                    tcatch += thiscatch;
                    bm->RBCestimation.RBCspeciesArray[groupIndex].sumcatch[f] += thiscatch * bm->X_CN * mg_2_kg;
                    
 				}
			}
        }
    }
    
    
	// Find if need length and or age frequencies - yes if have catch data for the fisheries and need length/age frequencies for the assessment
	if (tcatch > 0.0) {
		for (it = 0; it < 3; it++){
			//if (bm->RBCestimation.RBCspeciesArray[groupIndex].LengthFuture[it] != 0) {
            if ( tier == tier5 ) {
				bm->RBCestimation.RBCspeciesArray[groupIndex].LengthYears[it][iyr] = 1;
				for (f = 0; f < Nfleets; f++) {
					if (bm->RBCestimation.RBCspeciesArray[groupIndex].sumcatch[f] > 0.0)
						bm->RBCestimation.RBCspeciesArray[groupIndex].LengthFltYr[it][f][iyr] = 1;
  				}
			}
			//if (bm->RBCestimation.RBCspeciesArray[groupIndex].AgeFuture[it] != 0) {
            if ( tier == tier3 ) {
				bm->RBCestimation.RBCspeciesArray[groupIndex].AgeYears[iyr][it] = 1;   // Nov 2026: was [it][iyr] but AgeYears allocated [nYears][3] (ATS-01)
				for (f = 0; f < Nfleets; f++) {
					if (bm->RBCestimation.RBCspeciesArray[groupIndex].sumcatch[f] > 0.0)
						bm->RBCestimation.RBCspeciesArray[groupIndex].AgeFltYr[it][f][iyr] = 1;
 
				}
			}
		}
	}

	return;
}

/*****************************************************************************
 *
 * Name: GetBiomassData
 * Decription: gathers the spawning biomass value from operating model (converted to tonnes)
 *
 *****************************************************************************/
void GetBiomassData(MSEBoxModel *bm, int groupIndex, int iyr) {
    int ij, k, cohort, r_id, sn, rn, den, stock_id;
    int Nregions = (int)(bm->RBCestimation.RBCspeciesParam[groupIndex][NumRegions_id]);
    int Nstocks = FunctGroupArray[groupIndex].numStocks;
    double this_SSB = bm->tot_SSB[groupIndex] * bm->X_CN * mg_2_kg;
    int find_SSB = 0;
    int find_SSBshift = 0;
    double thistot = 0.0;
    
    /* Initialise */
    Util_Init_1D_Double(bm->RBCestimation.RBCspeciesArray[groupIndex].propR, Nregions, 0.0);
    Util_Init_2D_Double(bm->RBCestimation.RBCspeciesArray[groupIndex].propSR, Nregions, Nstocks, 0.0);
    for (cohort = 0; cohort < FunctGroupArray[groupIndex].numCohortsXnumGenes; cohort++) {
        bm->tot_cohort[groupIndex][cohort] = 0.0;
    }
    

    /* Find if need SSB_zero */
    if(bm->RBCestimation.RBCspeciesArray[groupIndex].SBioZero[Nstocks][Nregions] == 0.0)
        find_SSB = 1;
    if ((bm->RBCestimation.RBCspeciesParam[groupIndex][Regime_year_id] > 0)
        && (iyr == bm->RBCestimation.RBCspeciesParam[groupIndex][Regime_year_id])){
        find_SSBshift = 1;
    }
    
    /* Check to see if need to do live calc of SSB */
    if (this_SSB < 0.001) {  // i.e. less than 1kg
        this_SSB = 0.0;
        bm->tot_SSB[groupIndex] = 0.0;
        
        // Find SSB from scatch
        for (cohort = 0; cohort < FunctGroupArray[groupIndex].numCohortsXnumGenes; cohort++) {
            // Biomass outside the model domain
            if (FunctGroupArray[groupIndex].recruitType == external_recruit) {
                for (ij = MIGRATION[groupIndex].num_in_queue_done; ij < MIGRATION[groupIndex].num_in_queue; ij++) {
                    this_SSB += (MIGRATION[groupIndex].RN[cohort][ij] + MIGRATION[groupIndex].SN[cohort][ij]) * MIGRATION[groupIndex].DEN[cohort][ij] * bm->X_CN * mg_2_kg;
                    bm->tot_SSB[groupIndex] += FunctGroupArray[groupIndex].scaled_FSPB[cohort] * this_SSB;
                    bm->tot_cohort[groupIndex][cohort] += MIGRATION[groupIndex].DEN[cohort][ij];
                }
            }
        
            sn = FunctGroupArray[groupIndex].structNTracers[cohort];
            rn = FunctGroupArray[groupIndex].resNTracers[cohort];
            den = FunctGroupArray[groupIndex].NumsTracers[cohort];
            //bm->tot_Bcohort[groupIndex][cohort] = 0.0;
            
            // Biomass inside the model domain
            thistot = 0.0;
            for (ij = 0; ij < bm->nbox; ij++) {
                r_id = bm->regID[ij];
                if(r_id >= Nregions)
                    r_id = Nregions - 1;
                for (k = 0; k < bm->boxes[ij].nz; k++) {
					stock_id = bm->group_stock[groupIndex][ij][k];
                    
                    this_SSB = (bm->boxes[ij].tr[k][sn] + bm->boxes[ij].tr[k][rn]) * bm->boxes[ij].tr[k][den] * FunctGroupArray[groupIndex].habitatCoeffs[WC] * bm->X_CN * mg_2_kg;
                    bm->tot_SSB[groupIndex] += FunctGroupArray[groupIndex].scaled_FSPB[cohort] * this_SSB;
                    bm->RBCestimation.RBCspeciesArray[groupIndex].propR[r_id] += FunctGroupArray[groupIndex].scaled_FSPB[cohort] * this_SSB;
                    thistot += FunctGroupArray[groupIndex].scaled_FSPB[cohort] * this_SSB;
                    bm->RBCestimation.RBCspeciesArray[groupIndex].propSR[r_id][stock_id] += FunctGroupArray[groupIndex].scaled_FSPB[cohort] * this_SSB;
                    bm->tot_cohort[groupIndex][cohort] += bm->boxes[ij].tr[k][den];
                 }
            }
            
        }
    } else {
        thistot = 0.0;
        // Find regional breakdown
        
        for (cohort = 0; cohort < FunctGroupArray[groupIndex].numCohortsXnumGenes; cohort++) {
            sn = FunctGroupArray[groupIndex].structNTracers[cohort];
            rn = FunctGroupArray[groupIndex].resNTracers[cohort];
            den = FunctGroupArray[groupIndex].NumsTracers[cohort];
            //bm->tot_Bcohort[groupIndex][cohort] = 0.0;
        
            for (ij = 0; ij < bm->nbox; ij++) {
                r_id = bm->regID[ij];
                if(r_id >= Nregions)
                    r_id = Nregions - 1;
                
                for (k = 0; k < bm->boxes[ij].nz; k++) {

                    stock_id = bm->group_stock[groupIndex][ij][k];
                
                    this_SSB = (bm->boxes[ij].tr[k][sn] + bm->boxes[ij].tr[k][rn]) * bm->boxes[ij].tr[k][den] * FunctGroupArray[groupIndex].habitatCoeffs[WC] * bm->X_CN * mg_2_kg;
                    thistot += FunctGroupArray[groupIndex].scaled_FSPB[cohort] * this_SSB;
                    bm->RBCestimation.RBCspeciesArray[groupIndex].propR[r_id] += FunctGroupArray[groupIndex].scaled_FSPB[cohort] * this_SSB;
                    bm->RBCestimation.RBCspeciesArray[groupIndex].propSR[r_id][stock_id] += FunctGroupArray[groupIndex].scaled_FSPB[cohort] * this_SSB;
                    bm->tot_cohort[groupIndex][cohort] += bm->boxes[ij].tr[k][den];
                }
            }
        }
    }

    // Initialise before normalise
    bm->RBCestimation.RBCspeciesArray[groupIndex].SpawnBio[Nstocks][Nregions][iyr] = 0.0;
    if(find_SSB) {
        bm->RBCestimation.RBCspeciesArray[groupIndex].SBioZero[Nstocks][Nregions] = 0.0;
    }
    if (find_SSBshift){
        bm->RBCestimation.RBCspeciesArray[groupIndex].SBioZero_shift[Nstocks][Nregions] = 0.0;
    }
    
    // Normalise the regional contributions
    for (ij=0; ij < Nregions; ij++) {
        bm->RBCestimation.RBCspeciesArray[groupIndex].propR[ij] /= thistot;
    
        // Store spawning biomass for the current year
        for (stock_id=0; stock_id < Nstocks; stock_id++) {
            bm->RBCestimation.RBCspeciesArray[groupIndex].propSR[ij][stock_id] /= thistot;
            bm->RBCestimation.RBCspeciesArray[groupIndex].SpawnBio[stock_id][ij][iyr] = bm->tot_SSB[groupIndex] * bm->RBCestimation.RBCspeciesArray[groupIndex].propSR[ij][stock_id];
        }
        bm->RBCestimation.RBCspeciesArray[groupIndex].SpawnBio[Nstocks][ij][iyr] = bm->tot_SSB[groupIndex] * bm->RBCestimation.RBCspeciesArray[groupIndex].propR[ij];
        bm->RBCestimation.RBCspeciesArray[groupIndex].SpawnBio[Nstocks][Nregions][iyr] += bm->tot_SSB[groupIndex] * bm->RBCestimation.RBCspeciesArray[groupIndex].propR[ij];
        
        // If appropriate store the SSB_zero
        if(find_SSB) {
            for (stock_id=0; stock_id < Nstocks; stock_id++) {
                bm->RBCestimation.RBCspeciesArray[groupIndex].SBioZero[stock_id][ij] = bm->tot_SSB[groupIndex] * bm->RBCestimation.RBCspeciesArray[groupIndex].propSR[ij][stock_id];
            }
            bm->RBCestimation.RBCspeciesArray[groupIndex].SBioZero[Nstocks][ij] = bm->tot_SSB[groupIndex] * bm->RBCestimation.RBCspeciesArray[groupIndex].propR[ij];
            bm->RBCestimation.RBCspeciesArray[groupIndex].SBioZero[Nstocks][Nregions] += bm->tot_SSB[groupIndex] * bm->RBCestimation.RBCspeciesArray[groupIndex].propR[ij];
        }
        
        if (find_SSBshift){
            for (stock_id=0; stock_id < Nstocks; stock_id++) {
                bm->RBCestimation.RBCspeciesArray[groupIndex].SBioZero_shift[stock_id][ij] = bm->tot_SSB[groupIndex] * bm->RBCestimation.RBCspeciesArray[groupIndex].propSR[ij][stock_id];
            }
            bm->RBCestimation.RBCspeciesArray[groupIndex].SBioZero_shift[Nstocks][ij] = bm->tot_SSB[groupIndex] * bm->RBCestimation.RBCspeciesArray[groupIndex].propR[ij];
            bm->RBCestimation.RBCspeciesArray[groupIndex].SBioZero_shift[Nstocks][Nregions] += bm->tot_SSB[groupIndex] * bm->RBCestimation.RBCspeciesArray[groupIndex].propR[ij];
        }
    }
    
    return;
}

/*****************************************************************************
 *
 * Name: GetLengthWeightData
 * Decription: gathers the length and weight at age data
 *
 *****************************************************************************/
void GetLengthWeightData(MSEBoxModel *bm, int groupIndex, int iyr) {
    int ij, k, cohort, sn, rn, den, stock_id = 0, basecohort, ia, last_wtai = 0, last_lai = 0, next_ai;
    int Nstocks = FunctGroupArray[groupIndex].numStocks;
    int Nages = (int)bm->RBCestimation.RBCspeciesParam[groupIndex][MaxAge_id];
    double Wgt, totnum, last_wtval = 0, last_lval = 0, nstep, diff_val, next_val;
    double nAgeCor = (double) (FunctGroupArray[groupIndex].ageClassSize);
    
    Util_Init_1D_Double(bm->RBCestimation.RBCspeciesArray[groupIndex].propCount, Nstocks, 0.0);
    if (bm->RBCestimation.RBCspeciesParam[groupIndex][flagLAdirect_id]) {
        Util_Init_3D_Double(bm->RBCestimation.RBCspeciesArray[groupIndex].MeanLenAge, Nstocks, bm->K_num_sexes, Nages, 0.0);
    }
    
    // Get size if appropriate
    for (cohort = 0; cohort < FunctGroupArray[groupIndex].numCohortsXnumGenes; cohort++) {
        sn = FunctGroupArray[groupIndex].structNTracers[cohort];
        rn = FunctGroupArray[groupIndex].resNTracers[cohort];
        den = FunctGroupArray[groupIndex].NumsTracers[cohort];
        basecohort = (int)floor (cohort / FunctGroupArray[groupIndex].numGeneTypes);
        ia = basecohort * FunctGroupArray[groupIndex].ageClassSize;
        
        if (bm->RBCestimation.RBCspeciesParam[groupIndex][flagWAdirect_id]) {
            for (stock_id = 0; stock_id < Nstocks; stock_id++) {
                for (ij = 0; ij < bm->K_num_sexes; ij++) {
                    bm->RBCestimation.RBCspeciesArray[groupIndex].MeanWtAge[stock_id][ij][ia][iyr] = 0.0;
                }
            }
        }
        
        Util_Init_1D_Double(bm->RBCestimation.RBCspeciesArray[groupIndex].propCount, Nstocks, 0.0);  // As need to reinitalise each for each cohort
        
        totnum  = 0.0;
        for (ij = 0; ij < bm->nbox; ij++) {
            for (k = 0; k < bm->boxes[ij].nz; k++) {
                stock_id = bm->group_stock[groupIndex][ij][k];

                if(bm->boxes[ij].tr[k][den] > bm->min_dens) {
                    Wgt = (bm->boxes[ij].tr[k][sn] + bm->boxes[ij].tr[k][rn]);
                    totnum += bm->boxes[ij].tr[k][den] * FunctGroupArray[groupIndex].habitatCoeffs[WC];
                    if (bm->RBCestimation.RBCspeciesParam[groupIndex][flagLAdirect_id]) {
                        bm->RBCestimation.RBCspeciesArray[groupIndex].MeanLenAge[stock_id][FEMALE][ia] += (Ecology_Get_Size(bm, groupIndex, Wgt, basecohort) * bm->boxes[ij].tr[k][den] * FunctGroupArray[groupIndex].habitatCoeffs[WC] * bm->X_CN * mg_2_kg);
                     }
                     if (bm->RBCestimation.RBCspeciesParam[groupIndex][flagWAdirect_id]) {
                         bm->RBCestimation.RBCspeciesArray[groupIndex].MeanWtAge[stock_id][FEMALE][ia][iyr] += ((bm->boxes[ij].tr[k][sn] + bm->boxes[ij].tr[k][rn])  * bm->boxes[ij].tr[k][den] * FunctGroupArray[groupIndex].habitatCoeffs[WC] * bm->X_CN * mg_2_kg);
                     }
                     bm->RBCestimation.RBCspeciesArray[groupIndex].propCount[stock_id] += 1.0;
                }
            }
        }
        // Get final average sizes (which have been weighted based on numbers per box contributing
        if ((bm->RBCestimation.RBCspeciesParam[groupIndex][flagLAdirect_id]) && (bm->RBCestimation.RBCspeciesArray[groupIndex].propCount[stock_id])){
            for (stock_id = 0; stock_id < Nstocks; stock_id++) {
                for (ij = 0; ij < bm->K_num_sexes; ij++) {
                    bm->RBCestimation.RBCspeciesArray[groupIndex].MeanLenAge[stock_id][FEMALE][ia] /= bm->RBCestimation.RBCspeciesArray[groupIndex].propCount[stock_id];
                }
            }
        }
        
        for (stock_id = 0; stock_id < Nstocks; stock_id++) {
                if ((bm->RBCestimation.RBCspeciesParam[groupIndex][flagWAdirect_id]) && (bm->RBCestimation.RBCspeciesArray[groupIndex].propCount[stock_id])){
                for (ij = 0; ij < bm->K_num_sexes; ij++) {
                    bm->RBCestimation.RBCspeciesArray[groupIndex].MeanWtAge[stock_id][FEMALE][ia][iyr] /= bm->RBCestimation.RBCspeciesArray[groupIndex].propCount[stock_id];
                }
            }
        }
    
    }
    
    // Now do a size check to fill holes
    for (ia=0; ia < bm->RBCestimation.RBCspeciesParam[groupIndex][MaxAge_id]; ia++) {
        for (stock_id = 0; stock_id < Nstocks; stock_id++) {
            for (ij = 0; ij < bm->K_num_sexes; ij++) {
                if ( bm->RBCestimation.RBCspeciesArray[groupIndex].MeanWtAge[stock_id][FEMALE][ia][iyr] ) {
                    last_wtval = bm->RBCestimation.RBCspeciesArray[groupIndex].MeanWtAge[stock_id][FEMALE][ia][iyr];
                    last_wtai = ia;
                }
                if ( bm->RBCestimation.RBCspeciesArray[groupIndex].MeanLenAge[stock_id][FEMALE][ia] ) {
                    last_lval = bm->RBCestimation.RBCspeciesArray[groupIndex].MeanLenAge[stock_id][FEMALE][ia];
                    last_lai = ia;
                }
                
                
                if (!bm->RBCestimation.RBCspeciesArray[groupIndex].MeanWtAge[stock_id][FEMALE][ia][iyr]) {
                    if (( ia > 0 ) && ( ia < bm->RBCestimation.RBCspeciesParam[groupIndex][MaxAge_id] - FunctGroupArray[groupIndex].ageClassSize)) {  // just average either side
                        
                        next_ai = last_wtai + FunctGroupArray[groupIndex].ageClassSize;
                        next_val = bm->RBCestimation.RBCspeciesArray[groupIndex].MeanWtAge[stock_id][FEMALE][next_ai][iyr];
                        
                        diff_val = (next_val - last_wtval) / nAgeCor;
                        nstep = (double) (ia - last_wtai);
                        bm->RBCestimation.RBCspeciesArray[groupIndex].MeanWtAge[stock_id][FEMALE][ia][iyr] = last_wtval + nstep * diff_val;
                        
                        
                    } else if ( ia > 0 ) {
                        // Test just to be sure, in actuality always has to be > 0 as should always have a value for a = 0 case
                        // due to fact basecohort = 0 -> ai = 0 case above
                        // So this is to pad out final age class
                        
                        bm->RBCestimation.RBCspeciesArray[groupIndex].MeanWtAge[stock_id][FEMALE][ia][iyr] = last_wtval;
                        
                    }
                }
                if (!bm->RBCestimation.RBCspeciesArray[groupIndex].MeanLenAge[stock_id][FEMALE][ia]) {
                    if (( ia > 0 ) && ( ia < bm->RBCestimation.RBCspeciesParam[groupIndex][MaxAge_id] - FunctGroupArray[groupIndex].ageClassSize)) {  // just average either side
                        
                        next_ai = last_lai + FunctGroupArray[groupIndex].ageClassSize;
                        next_val =bm->RBCestimation.RBCspeciesArray[groupIndex].MeanLenAge[stock_id][FEMALE][next_ai];
                        
                        diff_val = (next_val - last_lval) / nAgeCor;
                        nstep = (double) (ia - last_lai);
                        bm->RBCestimation.RBCspeciesArray[groupIndex].MeanLenAge[stock_id][FEMALE][ia] = last_lval + nstep * diff_val;
                        
                    } else if ( ia > 0 ) {
                        // Test just to be sure, in actuality always has to be > 0 as should always have a value for a = 0 case
                        // due to fact basecohort = 0 -> ai = 0 case above
                        // So this is to pad out final age class
                        
                        bm->RBCestimation.RBCspeciesArray[groupIndex].MeanLenAge[stock_id][FEMALE][ia] = last_lval;
                        
                    }
                }
            }
        }
    }
    
    return;
}




/******************************************************************************
 *
 * Name:  GetCatch_n_DiscardData
 * Description:  generates the catch, effort and discard rate for a given year
 *
 ******************************************************************************/
void GetCatch_n_DiscardData(MSEBoxModel *bm, int groupIndex, int iyr) {
	int f, r, b, ij, nf, cohort, basecohort, sn, rn, den, stock_id = 0, r_id, k, s;
	double expdisc, sumcatch, sigma, norm, this_SSB, step1;
	int Nfleets = (int)(bm->RBCestimation.RBCspeciesParam[groupIndex][NumFisheries_id]);
	int Nregions = (int)(bm->RBCestimation.RBCspeciesParam[groupIndex][NumRegions_id]);
	int sumregion = Nregions;  // sum over regions
    int Nstocks = FunctGroupArray[groupIndex].numStocks;
    double toteffort = 0.0;
    double totcatch = 0.0;
    double totdisc = 0.0;
   
    Util_Init_2D_Double(bm->RBCestimation.RBCspeciesArray[groupIndex].actualcatch, Nfleets, Nregions, 0.0);
    Util_Init_2D_Double(bm->RBCestimation.RBCspeciesArray[groupIndex].actualeffort, Nfleets, Nregions, 0.0);
    Util_Init_2D_Double(bm->RBCestimation.RBCspeciesArray[groupIndex].actualdiscard, Nfleets, Nregions, 0.0);
    Util_Init_3D_Double(bm->RBCestimation.RBCspeciesArray[groupIndex].actualbiom, Nregions, Nstocks, FunctGroupArray[groupIndex].numCohorts, 0.0);
    Util_Init_3D_Double(bm->RBCestimation.RBCspeciesArray[groupIndex].actualcatchsize, Nregions, Nstocks, FunctGroupArray[groupIndex].numCohorts, 0.0);
    
    Util_Init_2D_Int(bm->RBCestimation.RBCspeciesArray[groupIndex].Ia, Nfleets, Nregions, 0);
  
	for (nf = 0; nf < bm->K_num_fisheries; nf++) {
        // map operating model fishery to assessment fishery for the species
        f = (int)bm->SP_FISHERYprms[groupIndex][nf][assess_nf_id];
		for (r = 0; r < Nregions; r++) {
			for(b=0; b<bm->nbox; b++) {
				if ((Nregions == 1)|| (bm->RBCestimation.RBCspeciesArray[groupIndex].regID[b] == r)) {
					bm->RBCestimation.RBCspeciesArray[groupIndex].actualeffort[f][r] += bm->CumEffort[nf][b];
					bm->RBCestimation.RBCspeciesArray[groupIndex].actualdiscard[f][r] += bm->CumDiscards[groupIndex][nf][b] * bm->X_CN * mg_2_kg;
					for(ij=0; ij<bm->boxes[b].nz; ij++) {
						bm->RBCestimation.RBCspeciesArray[groupIndex].actualcatch[f][r] += bm->CumCatch[groupIndex][nf][b][ij] * bm->X_CN * mg_2_kg;
 					}
				}
			}
        }
    }

 	for (f = 0; f < Nfleets; f++) {
        expdisc = 0.0;
        sumcatch = 0.0;
        toteffort = 0.0;
        totcatch = 0.0;
        totdisc = 0.0;
		for (r = 0; r < Nregions; r++) {
            
			// Get the catch data - always required
			sigma = sqrt(log(1 + bm->RBCestimation.RBCspeciesArray[groupIndex].CatchCV[f] * bm->RBCestimation.RBCspeciesArray[groupIndex].CatchCV[f])); //  log-normal sigma from cv
			norm = Util_xnorm(0.0, sigma, &bm->RBCestimation.Iseedz);
			step1 = bm->RBCestimation.RBCspeciesArray[groupIndex].actualcatch[f][r] * exp(norm - 0.5 * sigma * sigma);
            bm->RBCestimation.RBCspeciesArray[groupIndex].CatchData[f][r][iyr] = step1;
            totcatch += step1;
            if(bm->RBCestimation.RBCspeciesArray[groupIndex].CatchData[f][r][iyr] > 0) {  // Indicate catch present because then can do length sampling calculations etc
                bm->RBCestimation.RBCspeciesArray[groupIndex].Ia[f][r] = 1;
            }
            
			// Get the effort data - always required
			sigma = sqrt(log(1 + bm->RBCestimation.RBCspeciesArray[groupIndex].EffortCV[f] * bm->RBCestimation.RBCspeciesArray[groupIndex].EffortCV[f])); //  log-normal sigma from cv
			norm = Util_xnorm(0.0, sigma, &bm->RBCestimation.Iseedz);
			step1 = bm->RBCestimation.RBCspeciesArray[groupIndex].actualeffort[f][r] * exp(norm - 0.5 * sigma * sigma);
            bm->RBCestimation.RBCspeciesArray[groupIndex].EffortData[f][r][iyr] = step1;
            toteffort += step1;

			// Get the discard data and rate
			sigma = sqrt(log(1 + bm->RBCestimation.RBCspeciesArray[groupIndex].DiscCV[f] * bm->RBCestimation.RBCspeciesArray[groupIndex].DiscCV[f])); //  log-normal sigma from cv
			norm = Util_xnorm(0.0, sigma, &bm->RBCestimation.Iseedz);
			step1 = bm->RBCestimation.RBCspeciesArray[groupIndex].actualdiscard[f][r] * exp(norm - 0.5 * sigma * sigma);
            bm->RBCestimation.RBCspeciesArray[groupIndex].DiscData[f][r][iyr] = step1;
            totdisc += step1;
            
			if (bm->RBCestimation.RBCspeciesArray[groupIndex].DiscYears[f][iyr]) {
				expdisc += bm->RBCestimation.RBCspeciesArray[groupIndex].DiscData[f][r][iyr];
				sumcatch += bm->RBCestimation.RBCspeciesArray[groupIndex].TotCatch[f][r][iyr];
			}
            
		}
        
        if (bm->RBCestimation.RBCspeciesArray[groupIndex].DiscYears[f][iyr]) {
			if (bm->RBCestimation.DiscType == 2) {  // fraction of total
				expdisc = expdisc / (sumcatch + 0.0001);
                bm->RBCestimation.RBCspeciesArray[groupIndex].DiscData[f][sumregion][iyr] = expdisc
					+ Util_xnorm(0.0, bm->RBCestimation.RBCspeciesArray[groupIndex].DiscCV[f] * expdisc, &bm->RBCestimation.Iseedz);
            } else {
                bm->RBCestimation.RBCspeciesArray[groupIndex].DiscData[f][sumregion][iyr] = totdisc;   // Storing actual values
            }
		}
        
        bm->RBCestimation.RBCspeciesArray[groupIndex].CatchData[f][sumregion][iyr] = totcatch;
        bm->RBCestimation.RBCspeciesArray[groupIndex].EffortData[f][sumregion][iyr] = toteffort;
        
	}
    
    // Update total catch, effort and discard data
    
    
    // Get explotation rates - base data
    for (cohort = 0; cohort < FunctGroupArray[groupIndex].numCohortsXnumGenes; cohort++) {
        basecohort = (int)floor (cohort / FunctGroupArray[groupIndex].numGeneTypes);
        sn = FunctGroupArray[groupIndex].structNTracers[cohort];
        rn = FunctGroupArray[groupIndex].resNTracers[cohort];
        den = FunctGroupArray[groupIndex].NumsTracers[cohort];
        
        for (ij = 0; ij < bm->nbox; ij++) {
            r_id = bm->regID[ij];
            if(r_id >= Nregions)
                r_id = Nregions - 1;
            
            for (k = 0; k < bm->boxes[ij].nz; k++) {
                stock_id = bm->group_stock[groupIndex][ij][k];

                
                this_SSB = (bm->boxes[ij].tr[k][sn] + bm->boxes[ij].tr[k][rn]) * bm->boxes[ij].tr[k][den] * FunctGroupArray[groupIndex].habitatCoeffs[WC] * bm->X_CN * mg_2_kg;
                bm->RBCestimation.RBCspeciesArray[groupIndex].actualbiom[r_id][stock_id][basecohort] += this_SSB;
            }
 
            this_SSB = 0.0;
            for (f = 0; f < bm->K_num_fisheries; f++) {
                this_SSB += FunctGroupArray[groupIndex].SizeNumCaught[cohort][f][ij] + FunctGroupArray[groupIndex].SizeNumDiscard[cohort][f][ij];
            }
            this_SSB *= (bm->X_CN * mg_2_kg);
            bm->RBCestimation.RBCspeciesArray[groupIndex].actualcatchsize[r_id][stock_id][basecohort] += this_SSB;
        }
    }
    
    // Get the final explotation rates
    for (ij=0; ij < Nstocks; ij++) {
        for (r = 0; r < Nregions; r++) {
            for (cohort = 0; cohort < FunctGroupArray[groupIndex].numCohorts; cohort++) {
                for (s=0; s < bm->K_num_sexes; s++){
                    bm->RBCestimation.RBCspeciesArray[groupIndex].Uage[ij][r][s][cohort][iyr] = bm->RBCestimation.RBCspeciesArray[groupIndex].actualcatchsize[r][ij][cohort] / (bm->RBCestimation.RBCspeciesArray[groupIndex].actualbiom[r][ij][cohort] + small_num);
                }
            }
        }
    }
    
	return;
}


/******************************************************************************
 *
 * Name:  GetFishCPUEData
 * Description:  generates the fishery CPUE data for a given year
 *
 ******************************************************************************/
void GetFishCPUEData(MSEBoxModel *bm, int groupIndex, int iyr) {
	double norm, sigma, actualCPUE, catch, totcatch, this_Effort, cpue_dev, lastdev;
	int f, r;
	int Nfleets = (int) (bm->RBCestimation.RBCspeciesParam[groupIndex][NumFisheries_id]);
	int Nregions = (int) (bm->RBCestimation.RBCspeciesParam[groupIndex][NumRegions_id]);

	// Get CPUE
	for (f = 0; f < Nfleets; f++) {
		for (r = 0; r < Nregions; r++) {
			if (bm->RBCestimation.RBCspeciesArray[groupIndex].CPUEyears[f][r][iyr]) {
				this_Effort = bm->RBCestimation.RBCspeciesArray[groupIndex].EffortData[f][r][iyr];
				actualCPUE = bm->RBCestimation.RBCspeciesArray[groupIndex].CatchData[f][r][iyr] / (this_Effort + small_num);

				// random walk component
				sigma = sqrt(bm->RBCestimation.RBCspeciesArray[groupIndex].CPUEvar[f]);
				norm = Util_xnorm(0.0, sigma, &bm->RBCestimation.Iseedz);
				// Nov 2026: previous deviation taken as 0 in the first year (was reading index -1)
				if (iyr > 0)
					lastdev = bm->RBCestimation.RBCspeciesArray[groupIndex].CPUEdevs[f][r][iyr-1];
				else
					lastdev = 0.0;
				bm->RBCestimation.RBCspeciesArray[groupIndex].CPUEdevs[f][r][iyr] = bm->RBCestimation.RBCspeciesArray[groupIndex].CPUEcorr[f]
				        * lastdev
						+ norm * sqrt(1.0 - bm->RBCestimation.RBCspeciesArray[groupIndex].CPUEcorr[f] * bm->RBCestimation.RBCspeciesArray[groupIndex].CPUEcorr[f]);
                if(!bm->RBCestimation.RBCspeciesArray[groupIndex].CPUEdevs[f][r][iyr])
                    bm->RBCestimation.RBCspeciesArray[groupIndex].CPUEdevs[f][r][iyr] = 1.0;

				// error on CPUE
				sigma = sqrt(log(1 + bm->RBCestimation.RBCspeciesArray[groupIndex].CPUEcv[f] * bm->RBCestimation.RBCspeciesArray[groupIndex].CPUEcv[f])); //  log-normal sigma from cv
				norm = Util_xnorm(0.0, sigma, &bm->RBCestimation.Iseedz);
				cpue_dev = bm->RBCestimation.RBCspeciesArray[groupIndex].CPUEdevs[f][r][iyr];
				bm->RBCestimation.RBCspeciesArray[groupIndex].CPUEgen[f][r][iyr] = actualCPUE * cpue_dev * exp(norm - 0.5 * sigma * sigma);
                
                /*
                fprintf(bm->logFile, "Time: %e %s f: %d r: %d iyr: %d CPUEgen: %e actualCPUE: %e (catch: %e effort: %e) cpue_dev: %e norm: %e sigma: %e exp: %e, CPUEcv: %e\n",
                        bm->dayt, FunctGroupArray[groupIndex].groupCode, f, r, iyr, bm->RBCestimation.RBCspeciesArray[groupIndex].CPUEgen[f][r][iyr],
                        bm->RBCestimation.RBCspeciesArray[groupIndex].CatchData[f][r][iyr], this_Effort,
                        actualCPUE, cpue_dev, norm, sigma, exp(norm - 0.5 * sigma * sigma), bm->RBCestimation.RBCspeciesArray[groupIndex].CPUEcv[f]);
                 */
			}
		}
	}

	// Get weighted average over all regions
	for (f = 0; f < Nfleets; f++) {
		totcatch = 0.0;
		bm->RBCestimation.RBCspeciesArray[groupIndex].CPUEgen[f][Nregions][iyr] = 0.0;
		for (r = 0; r < Nregions; r++) {
			catch = bm->RBCestimation.RBCspeciesArray[groupIndex].CatchData[f][r][iyr];
			bm->RBCestimation.RBCspeciesArray[groupIndex].CPUEgen[f][Nregions][iyr] += catch * bm->RBCestimation.RBCspeciesArray[groupIndex].CPUEgen[f][r][iyr];
			totcatch += catch;
		}
		if (totcatch > 0.0) {
			bm->RBCestimation.RBCspeciesArray[groupIndex].CPUEgen[f][Nregions][iyr] = bm->RBCestimation.RBCspeciesArray[groupIndex].CPUEgen[f][Nregions][iyr] / (totcatch + small_num);
		} else {
			bm->RBCestimation.RBCspeciesArray[groupIndex].CPUEgen[f][Nregions][iyr] = 0.0;
		}
        
        //fprintf(bm->logFile, "Time: %e %s f: %d r: %d iyr: %d CPUEgen: %e totcatch: %e\n",
         //       bm->dayt, FunctGroupArray[groupIndex].groupCode, f, r, iyr, bm->RBCestimation.RBCspeciesArray[groupIndex].CPUEgen[f][r][iyr], totcatch);

	}

	return;
}


/******************************************************************************
 *
 * Name:  GetFishLength_n_AgeData
 * Description:  generates the fishery length and age composition data for a given year
 *
 ******************************************************************************/
void GetFishLength_n_AgeData(MSEBoxModel *bm, int groupIndex, int iyr) {
	int it, f, r, s, l, jj, aa, a, vecsize, sex_samp, nage_sample, nlen_sample;
 	int Nfleets = (int)(bm->RBCestimation.RBCspeciesParam[groupIndex][NumFisheries_id]);
	int Nregions = (int)(bm->RBCestimation.RBCspeciesParam[groupIndex][NumRegions_id]);
    int Nages = (int)(bm->RBCestimation.RBCspeciesParam[groupIndex][MaxAge_id]);
	int Nlen = (int) bm->RBCestimation.RBCspeciesParam[groupIndex][Nlen_id];
    int Nstocks = FunctGroupArray[groupIndex].numStocks;
    double ns = (double)(FunctGroupArray[groupIndex].numStocks);
    int Ncohorts = FunctGroupArray[groupIndex].numCohortsXnumGenes;
	int Nsexes = bm->K_num_sexes;
    int NageVec = Nages+1;
    int NlenVec = Nlen;
    double tempsum;
    int a_id = 0;  // For use with propvec and newvec
    int l_id = 1;
    //double Nsex_samp = 1;
    vecsize = Nfleets * Nregions * Nsexes;

    printf("Doing GetFishLength_n_AgeData\n");
    
    Util_Init_4D_Double(bm->RBCestimation.RBCspeciesArray[groupIndex].rawdata, Nfleets, Nregions, bm->K_num_sexes, 3, 0.0);   // So data type by sex by region by fleet
    Util_Init_5D_Double(bm->RBCestimation.RBCspeciesArray[groupIndex].rawsizedata, Nfleets, Nregions, bm->K_num_sexes, Ncohorts, 3, 0.0);   // So size data type by age by sex by region by fleet
    Util_Init_5D_Double(bm->RBCestimation.RBCspeciesArray[groupIndex].rawnumdata, Nfleets, Nregions, bm->K_num_sexes, Ncohorts, 3, 0.0);   // So matching numbers data type by age by sex by region by fleet
    Util_Init_4D_Double(bm->RBCestimation.RBCspeciesArray[groupIndex].ndata, Nfleets, Nregions, bm->K_num_sexes, 2, 0.0);  // 2 is because there is age and length data to be handled

    // Shouldn't need to re-init AFss and LFss as already per year entry
    
    Util_Init_2D_Double(bm->RBCestimation.RBCspeciesArray[groupIndex].lenprops, Nstocks+1, Nlen, 0.0);
    Util_Init_2D_Double(bm->RBCestimation.RBCspeciesArray[groupIndex].ageprops, Nstocks+1, Nages+1, 0.0);
    
    if(bm->useGenMnomial) {
        Util_Init_2D_Double(bm->RBCestimation.RBCspeciesArray[groupIndex].propvec, 3, vecsize, 0.0);   // One entry for each data type and then another dimension to cover whether age or length being done
        Util_Init_3D_Double(bm->RBCestimation.RBCspeciesArray[groupIndex].newvec, 3, 2, vecsize, 0.0);
    }
    
   // Get the actual value - aggregate over boxes but record the number of
    // samples per region so can figure out how many to include in the final
    // sample from each region
    
    // First get the raw data in an easily accessible form for manipulation
    for (it = 0; it < 3; it++) {
        
        //printf("Doing iteration %d and about to Find Data\n", it);
        
        FindRawData(bm, it, iyr, groupIndex, bm->RBCestimation.RBCspeciesArray[groupIndex].rawdata, bm->RBCestimation.RBCspeciesArray[groupIndex].rawnumdata, bm->RBCestimation.RBCspeciesArray[groupIndex].rawsizedata);
    
        if(bm->useGenMnomial) { // If using GenMnomial - get expected proportions
            for (f = 0; f < Nfleets; f++) {
                for (r = 0; r < FunctGroupArray[groupIndex].numStocks; r++) {
                    for (s=0; s < bm->K_num_sexes; s++){
                        GetAgeProps(bm, groupIndex, 1, it, f, r, s, iyr, bm->RBCestimation.RBCspeciesArray[groupIndex].ageprops, &bm->RBCestimation.Iseedx);
                        GetLenProps(bm, groupIndex, 1, it, f, r, s, iyr, bm->RBCestimation.RBCspeciesArray[groupIndex].lenprops, &bm->RBCestimation.Iseedx);
                    }
                }
            }
            
            // Get overall len and age props
            for (a = 0; a < Nages; a++) {
                bm->RBCestimation.RBCspeciesArray[groupIndex].ageprops[Nstocks][a] = 0.0;
                for (r = 0; r < FunctGroupArray[groupIndex].numStocks; r++) {
                    bm->RBCestimation.RBCspeciesArray[groupIndex].ageprops[Nstocks][a] += bm->RBCestimation.RBCspeciesArray[groupIndex].ageprops[r][a];
                }
                bm->RBCestimation.RBCspeciesArray[groupIndex].ageprops[Nstocks][a] /= ns;
            }
        }
    }
    
     // Now for each data type, sum and find proportions
    for (it = 0; it < 3; it++) {
        nage_sample = bm->RBCestimation.RBCspeciesArray[groupIndex].AgeN[it];
        nlen_sample = bm->RBCestimation.RBCspeciesArray[groupIndex].LengthN[it];
        Util_Init_1D_Double(bm->RBCestimation.RBCspeciesArray[groupIndex].lencomp, Nlen, 0.0);
        Util_Init_1D_Double(bm->RBCestimation.RBCspeciesArray[groupIndex].agecomp, Nages+1, 0.0);
        
        //printf("Doing iteration %d and about to Find Data\n", it);
 
        if(bm->useGenMnomial) {
            tempsum = 0.0;
            jj = -1;  // as ++ before first use
            for (f = 0; f < Nfleets; f++){
                for (r = 0; r < Nregions; r++){
                    for (s = 0; s < bm->K_num_sexes; s++){
                        tempsum += bm->RBCestimation.RBCspeciesArray[groupIndex].rawdata[f][r][s][it];
                    }
                }
            }
            for (f = 0; f < Nfleets; f++){
                for (r = 0; r < Nregions; r++){
                    for (s=0; s < bm->K_num_sexes; s++){
                        bm->RBCestimation.RBCspeciesArray[groupIndex].rawdata[f][r][s][it] /= (tempsum + small_num);
                        jj++;
                        
                       bm->RBCestimation.RBCspeciesArray[groupIndex].propvec[it][jj] = bm->RBCestimation.RBCspeciesArray[groupIndex].rawdata[f][r][s][it];
                    }
                }
            }
    
            // Figure out the proportion coming from each fleet and region etc
            GenMnomial(bm->RBCestimation.RBCspeciesArray[groupIndex].propvec[it], vecsize, nage_sample, bm->RBCestimation.RBCspeciesArray[groupIndex].newvec[it][a_id], &bm->RBCestimation.Iseedx, 0);
            GenMnomial(bm->RBCestimation.RBCspeciesArray[groupIndex].propvec[it], vecsize, nlen_sample, bm->RBCestimation.RBCspeciesArray[groupIndex].newvec[it][l_id], &bm->RBCestimation.Iseedx, 0);
    
            jj = -1;  // as ++ before first use
            for (f = 0; f < Nfleets; f++){
                for (r = 0; r < Nregions; r++){
                    for (s=0; s < bm->K_num_sexes; s++){
                        jj++;
                        bm->RBCestimation.RBCspeciesArray[groupIndex].ndata[f][r][s][a_id] = (int)(bm->RBCestimation.RBCspeciesArray[groupIndex].newvec[it][a_id][jj]);
                        bm->RBCestimation.RBCspeciesArray[groupIndex].ndata[f][r][s][l_id] = (int)(bm->RBCestimation.RBCspeciesArray[groupIndex].newvec[it][l_id][jj]);
                    }
                }
            }
        }

    // Now get the samples
        for (f = 0; f < Nfleets; f++) {
            for (r = 0; r < Nregions; r++) {
                for (s = 0; s < Nsexes; s++) {
                    sex_samp = s;
                    //if (Nsex_samp == 1)
                    //    sex_samp = 1;   // this combines samples over sexes - removed as using a single sex in Atlantis
                 
                    // Add the error
                    if(bm->useGenMnomial) {
                        // Save the new data
                        nage_sample = (int)bm->RBCestimation.RBCspeciesArray[groupIndex].ndata[f][r][s][a_id];
                        nlen_sample = (int)bm->RBCestimation.RBCspeciesArray[groupIndex].ndata[f][r][s][l_id];

                        if(nage_sample)
                            GenMnomial(bm->RBCestimation.RBCspeciesArray[groupIndex].ageprops[Nstocks], NageVec, nage_sample, bm->RBCestimation.RBCspeciesArray[groupIndex].agecomp, &bm->RBCestimation.Iseedx, 0);
                        if(nlen_sample)
                            GenMnomial(bm->RBCestimation.RBCspeciesArray[groupIndex].lenprops[Nstocks], NlenVec, nlen_sample, bm->RBCestimation.RBCspeciesArray[groupIndex].lencomp, &bm->RBCestimation.Iseedz, 0);
                    } else {
                        // Use method consistent with ecological indicators work
                        
                        //printf("Fleet %d region %d sex %d and generating %d age samples\n", f, r, s, nage_sample);
 
                        if(nage_sample) {
                            GenSample(bm, groupIndex, NageVec, f, r, s, it, nage_sample, bm->RBCestimation.RBCspeciesArray[groupIndex].agecomp, bm->RBCestimation.RBCspeciesArray[groupIndex].rawnumdata, bm->RBCestimation.RBCspeciesArray[groupIndex].rawsizedata, &bm->RBCestimation.Iseedx);
                        }
                        //printf("Fleet %d region %d sex %d and generating %d length samples\n", f, r, s, nlen_sample);
                        
                        if(nlen_sample){
                            GenSample(bm, groupIndex, NlenVec, f, r, s, it, nlen_sample, bm->RBCestimation.RBCspeciesArray[groupIndex].lencomp, bm->RBCestimation.RBCspeciesArray[groupIndex].rawnumdata, bm->RBCestimation.RBCspeciesArray[groupIndex].rawsizedata, &bm->RBCestimation.Iseedz);
                        }
                    }
                
                    // Store length data first
                    
                   // printf("Storing %d length samples in %d bins\n", nlen_sample, Nlen);
                    
                     if (bm->RBCestimation.RBCspeciesArray[groupIndex].LengthYears[it][iyr] == 1) {
                                
                        // keep track of sample sizes of length records
                        bm->RBCestimation.RBCspeciesArray[groupIndex].LFss[f][sex_samp][iyr][it] += nlen_sample;

                        // keep track of the final length data generated
                        for (l = 0; l < Nlen; l++) {
                            bm->RBCestimation.RBCspeciesArray[groupIndex].LenComp[f][sex_samp][iyr][it][l] += (int)bm->RBCestimation.RBCspeciesArray[groupIndex].lencomp[l];
                            
                            //fprintf(bm->logFile, "Time: %e %s iyr: %d, it: %d, f: %d, l: %d sex_samp: %d LenComp: %d, lencomp: %e\n",
                            //        bm->dayt, FunctGroupArray[groupIndex].groupCode, iyr, it, f, l, sex_samp, bm->RBCestimation.RBCspeciesArray[groupIndex].LenComp[f][sex_samp][iyr][it][l], bm->RBCestimation.RBCspeciesArray[groupIndex].lencomp[l]);
                        }
                    }

                    //printf("Storing %d length samples in %d bins\n", nage_sample, Nages);

                     // Now store age data
                    if (bm->RBCestimation.RBCspeciesArray[groupIndex].AgeYears[iyr][it] == 1){   // Nov 2026: was [it][iyr] (ATS-01)
                        // keep track of sample sizes of age records
                        bm->RBCestimation.RBCspeciesArray[groupIndex].AFss[f][s][iyr][it] += nage_sample;
                            
                        // keep track of the final age data generated
                        for (aa=0; aa < Nages + 1; aa++) {
                            bm->RBCestimation.RBCspeciesArray[groupIndex].AgeComp[f][sex_samp][iyr][it][aa] += (int)bm->RBCestimation.RBCspeciesArray[groupIndex].agecomp[aa];
                            
                            //fprintf(bm->logFile, "Time: %e %s iyr: %d, it: %d, f: %d, aa: %d sex_samp: %d AgeComp: %d, agecomp: %e\n",
                            //       bm->dayt, FunctGroupArray[groupIndex].groupCode, iyr, it, f, aa, sex_samp, bm->RBCestimation.RBCspeciesArray[groupIndex].AgeComp[f][sex_samp][iyr][it][aa], bm->RBCestimation.RBCspeciesArray[groupIndex].agecomp[aa]);
                        }
                    }
                    
                }
            }
        }
    }
}

//******************************************************************************
//
// Name:  GetEnviroData
// Description:  generate environmental data
//
// created  : April 2014 Beth Fulton
//
//*****************************************************************************
void GetEnviroData(MSEBoxModel *bm, int groupIndex, int iyr) {
    int r, b, this_nz;
    double ans, tot, num;
    int Nregions = (int)(bm->RBCestimation.RBCspeciesParam[groupIndex][NumRegions_id]);
    
    if(verbose){
        printf("GetEnviroData\n");
    }
    
    if(bm->RBCestimation.RBCspeciesParam[groupIndex][num_enviro_obs_id] > 0) {
        bm->RBCestimation.RBCspeciesParam[groupIndex][num_enviro_obs_id] = iyr + 1;
        
        for (r=0; r < Nregions; r++) {
            num = 0;
            tot = 0;
            for (b = 0; b < bm->nbox; b++) {
                this_nz = bm->boxes[b].nz;
                if ((Nregions == 1)|| (bm->RBCestimation.RBCspeciesArray[groupIndex].regID[b] == r)) {
                    // Get surface temperature as proxy - as likely what record in reality (i.e. for layer nz-1
                    tot += bm->boxes[b].tr[this_nz-1][Temp_i];
                    num += 1.0;
                }
            }
            ans = tot / (num + small_num);
            bm->RBCestimation.RBCspeciesArray[groupIndex].EnviroData[r][iyr] = ans;
        }
    }
    
    if(verbose){
        printf("End GetEnviroData\n");
    }
    
    return;
}

//******************************************************************************
//
// Name:  FindRawData
// Description:  find the retained, discarded or whole catch by fleet, region and sex for a particular year
//
//
// Equations : not quite denominator of eqn 1.38 ? ufleet, wtlen extra
// called by : GetFishLengthData
// calls :
// created  : Nov 2007 Sally
//
// Note that tempcatch is the rawdata array
//
//*****************************************************************************
void FindRawData(MSEBoxModel *bm, int itype, int iyr, int groupIndex, double ****tempcatch, double *****rawnumdata, double *****rawsizedata) {
	int f, s, nc, r_id, nf, sn, rn, k, b, totnum, have_catch, have_discards;
	int Nsexes = bm->K_num_sexes;
    int Ncohorts = FunctGroupArray[groupIndex].numCohortsXnumGenes;
	int Nregions = (int)(bm->RBCestimation.RBCspeciesParam[groupIndex][NumRegions_id]);
    double step1, step2, new_mean_l, Wgt, li;
    int sampsize = (int)(FunctGroupArray[groupIndex].speciesParams[samplesize_id]);
    
    totnum = 0;
	for (nf = 0; nf < bm->K_num_fisheries; nf++) {
        // map operating model fishery to assessment fishery for the species
        f = (int)bm->SP_FISHERYprms[groupIndex][nf][assess_nf_id];
		for (b = 0; b < bm->nbox; b++) {
            if (bm->boxes[b].type != BOUNDARY) {
                // map the box to the assessment region
                r_id = bm->regID[b];
                if(r_id >= Nregions)
                    r_id = Nregions - 1;
                have_catch = 0;
                have_discards = 0;
               
                if( bm->CumDiscards[groupIndex][nf][b] > 0)
                        have_discards = 1;
                
                for (s = 0; s < Nsexes; s++) {
                    for (nc = 0; nc < Ncohorts; nc++) {
                        
                        //printf("Doing fishery %d box %d sex %d cohort %d\n", nf, b, s, nc);
                        
                        sn = FunctGroupArray[groupIndex].structNTracers[nc];
                        rn = FunctGroupArray[groupIndex].resNTracers[nc];
                        Wgt = 0.0;
                        for (k=0; k < bm->boxes[b].nz; k++) {
                            if ((bm->boxes[b].tr[k][sn] + bm->boxes[b].tr[k][rn]) > Wgt) {
                                Wgt = bm->boxes[b].tr[k][sn] + bm->boxes[b].tr[k][rn];
                            }
                            if( bm->CumCatch[groupIndex][nf][b][k] > 0 )
                                have_catch = 1;
                        }
                        li = Ecology_Get_Size(bm, groupIndex, Wgt, nc);
                    
                        // retained
                        if ((itype == retained_id) && have_catch) {
                            tempcatch[f][r_id][s][itype] += FunctGroupArray[groupIndex].SizeNumCaught[nc][nf][b];
                            
                            step1 = rawnumdata[f][r_id][s][nc][itype] * rawsizedata[f][r_id][s][nc][itype];
                            step2 = li * FunctGroupArray[groupIndex].SizeNumCaught[nc][nf][b];
                            new_mean_l = (step1 + step2) / (rawnumdata[f][r_id][s][nc][itype] + FunctGroupArray[groupIndex].SizeNumCaught[nc][nf][b] + small_num);
 
                            rawnumdata[f][r_id][s][nc][itype] += FunctGroupArray[groupIndex].SizeNumCaught[nc][nf][b];
                            rawsizedata[f][r_id][s][nc][itype] = new_mean_l;
                            totnum += (int)FunctGroupArray[groupIndex].SizeNumCaught[nc][nf][b];
                            
                            //printf("rawnum: %e rawsize: %e totnum: %d SizeNumCaught: %e\n", rawnumdata[f][r_id][s][nc][itype], rawsizedata[f][r_id][s][nc][itype], totnum, FunctGroupArray[groupIndex].SizeNumCaught[nc][nf][b]);
                            
                        }

                        // whole catch
                        if ((itype == whole_id) && (have_catch || have_discards)) {
                            tempcatch[f][r_id][s][itype] += FunctGroupArray[groupIndex].SizeNumCaught[nc][nf][b] + FunctGroupArray[groupIndex].SizeNumDiscard[nc][nf][b];

                            step1 = rawnumdata[f][r_id][s][nc][itype] * rawsizedata[f][r_id][s][nc][itype];
                            step2 = li * (FunctGroupArray[groupIndex].SizeNumCaught[nc][nf][b] + FunctGroupArray[groupIndex].SizeNumDiscard[nc][nf][b]);
                            new_mean_l = (step1 + step2) / (rawnumdata[f][r_id][s][nc][itype] + FunctGroupArray[groupIndex].SizeNumCaught[nc][nf][b] + FunctGroupArray[groupIndex].SizeNumDiscard[nc][nf][b] + small_num);
 
                            rawnumdata[f][r_id][s][nc][itype] += FunctGroupArray[groupIndex].SizeNumCaught[nc][nf][b] + FunctGroupArray[groupIndex].SizeNumDiscard[nc][nf][b];
                            rawsizedata[f][r_id][s][nc][itype] = new_mean_l;
                            totnum += (int)FunctGroupArray[groupIndex].SizeNumCaught[nc][nf][b] + (int)FunctGroupArray[groupIndex].SizeNumDiscard[nc][nf][b];
                            
                            //printf("rawnum: %e rawsize: %e totnum: %d SizeNumCaught: %e SizeNumDiscard: %e \n", rawnumdata[f][r_id][s][nc][itype], rawsizedata[f][r_id][s][nc][itype], totnum, FunctGroupArray[groupIndex].SizeNumCaught[nc][nf][b], FunctGroupArray[groupIndex].SizeNumDiscard[nc][nf][b]);

                            
                        }

                        // discarded
                        if ((itype == discarded_id) && have_discards) {
                            tempcatch[f][r_id][s][itype] += FunctGroupArray[groupIndex].SizeNumDiscard[nc][nf][b];
                            
                            step1 = rawnumdata[f][r_id][s][nc][itype] * rawsizedata[f][r_id][s][nc][itype];
                            step2 = li * FunctGroupArray[groupIndex].SizeNumDiscard[nc][nf][b];
                            new_mean_l = (step1 + step2) / (rawnumdata[f][r_id][s][nc][itype] + FunctGroupArray[groupIndex].SizeNumDiscard[nc][nf][b] + small_num);
 
                            rawnumdata[f][r_id][s][nc][itype] += FunctGroupArray[groupIndex].SizeNumDiscard[nc][nf][b];
                            rawsizedata[f][r_id][s][nc][itype] = new_mean_l;
                            totnum += (int)FunctGroupArray[groupIndex].SizeNumDiscard[nc][nf][b];
                            
                            //printf("rawnum: %e rawsize: %e totnum: %d SizeNumDiscard: %e \n", rawnumdata[f][r_id][s][nc][itype], rawsizedata[f][r_id][s][nc][itype], totnum, FunctGroupArray[groupIndex].SizeNumDiscard[nc][nf][b]);
                            
                            
                        }
                    }
				}
			}
		}
	}

    if (totnum > FunctGroupArray[groupIndex].speciesParams[samplesize_id]) {
        bm->RBCestimation.RBCspeciesArray[groupIndex].AgeN[itype] = sampsize;
        bm->RBCestimation.RBCspeciesArray[groupIndex].LengthN[itype] = sampsize;
    } else {
        bm->RBCestimation.RBCspeciesArray[groupIndex].AgeN[itype] = totnum;
        bm->RBCestimation.RBCspeciesArray[groupIndex].LengthN[itype] = totnum;
    }

}

//******************************************************************************
//
// Name:  GetLenProps
// Description:  find the length comp sample proportions for a given fleet/survey, region and sex
//
// created  : Nov 2007 Sally
//
//*****************************************************************************
void GetLenProps(MSEBoxModel *bm, int groupIndex, int id, int itype, int f, int r, int s, int iyr, double **lenprops, int *iseed) {
	double sumlen;
	int l, b, k, nc, sn, rn, stock_id;
	int Nlen = (int) bm->RBCestimation.RBCspeciesParam[groupIndex][Nlen_id];
	int Ncohorts = FunctGroupArray[groupIndex].numCohortsXnumGenes;
    int bin_size = (int)(FunctGroupArray[groupIndex].speciesParams[allometic_bin_size_id]);
    double li, Wgt, samplecatch, samplediscard;
    int reflayer = 0;
    
	// check for allocation of fleet to this region
	if ((id == 1) && (bm->RBCestimation.RBCspeciesArray[groupIndex].Ia[f][r] == 0))
		return;

    // initialise
    for (l = 0; l < Nlen; l++) {
        lenprops[r][l] = 0.0;
    }
    
    // find numbers per bin in raw data
	for (nc = 0; nc < Ncohorts; nc++) {
        sn = FunctGroupArray[groupIndex].structNTracers[nc];
        rn = FunctGroupArray[groupIndex].resNTracers[nc];
		for (b = 0; b < bm->nbox; b++) {
            stock_id = bm->group_stock[groupIndex][b][reflayer];
            if (r != stock_id)
                continue;

            Wgt = 0.0;
            for (k=0; k < bm->boxes[b].nz; k++) {
                stock_id = bm->group_stock[groupIndex][b][k];
                if (r != stock_id) {
                    // DO nothing as not right stock
                } else {
                    if ((bm->boxes[b].tr[k][sn] + bm->boxes[b].tr[k][rn]) > Wgt) {
                        Wgt = bm->boxes[b].tr[k][sn] + bm->boxes[b].tr[k][rn];
                    }
                }
            }
            li = Ecology_Get_Size(bm, groupIndex, Wgt, nc);
            l = (int)floor(li / bin_size);
            
			for (f = 0; f < bm->K_num_fisheries; f++) {
                //fishery
                if (id == 1) {
                    samplecatch = FunctGroupArray[groupIndex].SizeNumCaught[nc][f][b];
                    samplediscard = FunctGroupArray[groupIndex].SizeNumDiscard[nc][f][b];
                    
                    if ((itype == retained_id) || (itype == whole_id)) {
                        lenprops[r][l] += samplecatch;
                    }
                    if ((itype == discarded_id) || (itype == whole_id)) {
                        lenprops[r][l] += samplediscard;
                    }
                }

                //survey - not included as yet
            }
        }
	}

	sumlen = 0.0;
	for (l = 0; l < Nlen; l++) {
		sumlen += lenprops[r][l];
    }
    if (sumlen == 0.0) sumlen = small_num;

    // normalise to get proportions
	for (l = 0; l < Nlen; l++) {
		lenprops[r][l] /= sumlen;
    }

}


//******************************************************************************
//
// Name:  GetAgeProps
// Description:  find the age comp sample proportions for a given fleet/survey, region and sex
//
// created  : Nov 2007 Sally
//
//*****************************************************************************
void GetAgeProps(MSEBoxModel *bm, int groupIndex, int id, int itype, int f, int r, int s, int iyr, double **ageprops, int *iseed){
	double sumage, p_agestep, p_a, max_prop, samplecatch, samplediscard;
    int Nages = (int)(bm->RBCestimation.RBCspeciesParam[groupIndex][MaxAge_id] + 1);
	int Ncohorts = FunctGroupArray[groupIndex].numCohortsXnumGenes;
    int sp_AgeSize = FunctGroupArray[groupIndex].ageClassSize;
    double num_ann_class = (double)(FunctGroupArray[groupIndex].ageClassSize);
    int a, nc, aa, b, k, stock_id;
    int reflayer = 0;

	// check for allocation of fleet to this region
	if ((id==1) && (bm->RBCestimation.RBCspeciesArray[groupIndex].Ia[f][r] == 0))
		return;

    // initialise
    for (a = 0; a < Nages; a++) {
        ageprops[r][a] = 0.0;
    }
    
    // find numbers per bin in raw data
	for (nc = 0; nc < Ncohorts; nc++) {
        // basecohort * sp_AgeSize_per_age_cohort first
        a = (int)(floor(nc / FunctGroupArray[groupIndex].numGeneTypes)) * sp_AgeSize;

		for (b = 0; b < bm->nbox; b++) {
            stock_id = bm->group_stock[groupIndex][b][reflayer];
            if (r != stock_id)
                continue;
            
            // find annual increment
            if(sp_AgeSize > 1){
                p_a = ran3(iseed);
                p_agestep = 0.0;
                for (aa = 0; aa < sp_AgeSize; aa++) {
                    if (!bm->flagagestruct) {
                        p_agestep += 1.0 / num_ann_class;
                    } else {
                        max_prop = 0.0;
                        for (k=0; k < bm->boxes[b].nz; k++) {
                            stock_id = bm->group_stock[groupIndex][b][k];
                            if(r != stock_id) {
                                // Do nothing as not relevant
                            } else {
                                if(FunctGroupArray[groupIndex].boxPopRatio[b][k][nc][aa] > max_prop){
                                    max_prop = FunctGroupArray[groupIndex].boxPopRatio[b][k][nc][aa];
                                }
                            }
                        }
                        p_agestep += max_prop;
                    }
                    
                    if (p_a < p_agestep) {  // Found the candidate annual step so increment a appropriately and break
                        a += aa;
                        break;
                    }
                }
            } else {
                // Nothing to do as already annual a
            }
       
			for (f = 0; f < bm->K_num_fisheries; f++) {
                //fishery
                if (id == 1) {
                    samplecatch = FunctGroupArray[groupIndex].SizeNumCaught[nc][f][b];
                    samplediscard = FunctGroupArray[groupIndex].SizeNumDiscard[nc][f][b];
                    
                    if ((itype == retained_id) || (itype == whole_id)) {
                        ageprops[r][a] += samplecatch;
                    }
                    if ((itype == discarded_id) || (itype == whole_id)) {
                        ageprops[r][a] += samplediscard;
                    }
                }
                
                //survey - not included as yet
            }
        }
	}

	sumage = 0.0;
	for (a=0; a < Nages; a++)
		sumage += ageprops[r][a];
    if (sumage == 0.0) sumage = small_num;
    
	// normalise to get proportions
    for (a=0; a < Nages; a++) {
		ageprops[r][a] /= sumage;
    }
    return;
}


//******************************************************************************
//
// Name:  GetCloseKinNum
// Description:  generate close kin survey data
//
// created  : March 2023 Beth Fulton
//
//*****************************************************************************
void GetCloseKinNum(MSEBoxModel *bm, int groupIndex, int iyr) {
    double samplesize = FunctGroupArray[groupIndex].speciesParams[samplesize_id];
    double sexratio_var = drandom(-bm->RBCestimation.sexratio_cv, bm->RBCestimation.sexratio_cv);
    double sex_ratio = bm->RBCestimation.RBCspeciesParam[groupIndex][femsexratio_id] * (1.0 + sexratio_var);
    int ns, a;
    
    printf("Doing GetCloseKinNum\n");

    for (ns = 0; ns < FunctGroupArray[groupIndex].numStocks; ns++){
        for (a = 0; a < bm->RBCestimation.RBCspeciesParam[groupIndex][MaxAge_id]; a++){
            bm->RBCestimation.RBCspeciesArray[groupIndex].CKSurveyNum[ns][FEMALE_ID][a][iyr] = (int)(floor(bm->RBCestimation.RBCspeciesArray[groupIndex].ageprops[ns][a] * samplesize * sex_ratio));
            bm->RBCestimation.RBCspeciesArray[groupIndex].CKSurveyNum[ns][MALE_ID][a][iyr] = (int)(floor(bm->RBCestimation.RBCspeciesArray[groupIndex].ageprops[ns][a] * samplesize * (1.0 - sex_ratio)));
        }
    }
    
    return;
}

//******************************************************************************
//
// Name:  RecordHistF
// Description:  store F based on recorded catch and effort data
//               and get average of the 5 most recent years
//
// created  : September 2023 Beth Fulton
//
//*****************************************************************************
void RecordHistF(MSEBoxModel *bm, int sp, int iyr) {
    int minYr, ij, k, nf, n, r, sn, rn, den, stage, pid;
    //int maxYr;
    double avgPeriod = 0.0;
    double fishableBiomass = 0.0, avgF = 0.0, avgC = 0.0, spbiom, q, sel;
    int Nfleets = (int)(bm->RBCestimation.RBCspeciesParam[sp][NumFisheries_id]);
    
    if(verbose) {
        printf("RecordHistF for %s\n", FunctGroupArray[sp].groupCode);
    }
    
    for (ij = 0; ij < bm->nbox; ij++) {
        if (bm->boxes[ij].type != BOUNDARY) {
            for (k = 0; k < bm->boxes[ij].nz; k++) {
                bm->cell_vol = bm->boxes[ij].area * bm->boxes[ij].dz[k];
                
                switch (FunctGroupArray[sp].groupAgeType) {
                case AGE_STRUCTURED:
                    for (n = 0; n < FunctGroupArray[sp].numCohortsXnumGenes; n++) {
                        sn = FunctGroupArray[sp].structNTracers[n];
                        rn = FunctGroupArray[sp].resNTracers[n];
                        den = FunctGroupArray[sp].NumsTracers[n];
                        stage = FunctGroupArray[sp].cohort_stage[n];
                        spbiom = (bm->boxes[ij].tr[k][sn] + bm->boxes[ij].tr[k][rn]) * bm->boxes[ij].tr[k][den];
                        
                        /* Consideration per fishery */
                        for (nf = 0; nf < bm->K_num_fisheries; nf++) {
                            sel = bm->selectivity[sp][nf][stage];
                            q = bm->SP_FISHERYprms[sp][nf][q_id];
                            
                            fishableBiomass += sel * q * spbiom;
                        }
                    }
                    break;
                case AGE_STRUCTURED_BIOMASS:
                    for (n = 0; n < FunctGroupArray[sp].numCohortsXnumGenes; n++) {
                        pid = FunctGroupArray[sp].totNTracers[n];
                        stage = FunctGroupArray[sp].cohort_stage[n];
                        spbiom = bm->boxes[ij].tr[k][pid];
                        
                        /* Consideration per fishery */
                        for (nf = 0; nf < bm->K_num_fisheries; nf++) {
                            sel = bm->selectivity[sp][nf][stage];
                            q = bm->SP_FISHERYprms[sp][nf][q_id];
                            
                            fishableBiomass += sel * q * spbiom;
                        }
                    }
                    break;
                case BIOMASS:
                    /* Do nothing */
                    pid = FunctGroupArray[sp].totNTracers[0];
                    spbiom = bm->boxes[ij].tr[k][pid];
                        
                    /* Consideration per fishery */
                    for (nf = 0; nf < bm->K_num_fisheries; nf++) {
                        sel = bm->selectivity[sp][nf][0];
                        q = bm->SP_FISHERYprms[sp][nf][q_id];
                    }
                    fishableBiomass += sel * q * spbiom;

                    break;
                }
            }
        }
    }
    
    for (nf = 0; nf < Nfleets; nf++) {
        bm->RBCestimation.RBCspeciesArray[sp].Fhist[nf][iyr] = 0.0;
        for (r = 0; r < bm->RBCestimation.RBCspeciesParam[sp][NumRegions_id]; r++) {
            bm->RBCestimation.RBCspeciesArray[sp].Fhist[nf][iyr] += (bm->RBCestimation.RBCspeciesArray[sp].CatchData[nf][r][iyr] + bm->RBCestimation.RBCspeciesArray[sp].DiscData[nf][r][iyr]) / fishableBiomass;

        }
    }
    
    // Calculate the averages - over teh review period, which is typically 5 years
    minYr = iyr - bm->RBCestimation.ReviewPeriod;
    if (minYr < 0) {
        minYr = 0;
    }
    
    avgF = 0.0;
    avgC = 0.0;
    avgPeriod = 0.0;
    for (ij = minYr; ij < (iyr + 1); ij++) {
        for (nf = 0; nf < Nfleets; nf++) {
            avgF += bm->RBCestimation.RBCspeciesArray[sp].Fhist[nf][ij];
            for (r = 0; r < bm->RBCestimation.RBCspeciesParam[sp][NumRegions_id]; r++) {
                avgC += bm->RBCestimation.RBCspeciesArray[sp].CatchData[nf][r][ij];
            }
        }
        avgPeriod += 1.0;
    }
    avgF /= avgPeriod;
    avgC /= avgPeriod;
    
    bm->RBCestimation.RBCspeciesParam[sp][F_5yrAvg_id] = avgF;
    bm->RBCestimation.RBCspeciesParam[sp][Catch_5yrAvg_id] = avgC;
    
    return;
}


//******************************************************************************
//
// Name:  GenSample
// Description:  alternative means of finding data on catch, discards etc -  for a given fleet/survey, region and sex
//
// created  : April 2014 Beth Fulton
//
//*****************************************************************************
void GenSample(MSEBoxModel *bm, int groupIndex, int vecsize, int nf, int r_id, int s, int itype, int sample_size, double *comp, double *****rawnumdata, double *****rawsizedata, int *iseed){
    int ns, nc, l;
    int Ncohorts = FunctGroupArray[groupIndex].numCohortsXnumGenes;
    int bin_size = (int)(FunctGroupArray[groupIndex].speciesParams[allometic_bin_size_id]);
    double this_totsum = 0.0;
    double p_a, sum_p, li;
    
    Util_Init_1D_Double(bm->RBCestimation.RBCspeciesArray[groupIndex].props, (FunctGroupArray[groupIndex].numCohortsXnumGenes), 0.0);
    Util_Init_1D_Double(comp, vecsize, 0.0);

    // Find proportions to act as probabilities of selection
    for (nc = 0; nc < Ncohorts; nc++) {
        bm->RBCestimation.RBCspeciesArray[groupIndex].props[nc] += rawnumdata[nf][r_id][s][nc][itype];
        this_totsum += rawnumdata[nf][r_id][s][nc][itype];
    }
    
    // Normal use to get into proprtons
    for (nc = 0; nc < Ncohorts; nc++) {
        bm->RBCestimation.RBCspeciesArray[groupIndex].props[nc] /= (this_totsum + small_num);
    }
    if (!this_totsum) {
        fprintf(bm->logFile, "No fish to sample for %s fishery class %d region %d sex %d itype %d - despite wanting to collect %d samples\n", FunctGroupArray[groupIndex].groupCode, nf, r_id, s, itype, sample_size);
        return;
    }
    
    // Need samples per fishery, regions and sexes
    ns  = 0;
    l = 0;
    while (ns < sample_size) {
        
        //printf("ns %d vs sample_size %d\n", ns, sample_size);
        
        p_a = ran3(iseed);
        sum_p = 0.0;
        
        for (nc = 0; nc < Ncohorts; nc++) {
            sum_p += bm->RBCestimation.RBCspeciesArray[groupIndex].props[nc];
            
             //printf("Doing cohort %d p_a %e sum_p %e\n", nc, p_a, sum_p);

            if (sum_p > p_a) {
                // assign sample to the composition results
                li = rawsizedata[nf][r_id][s][nc][itype];
                l = (int)floor(li / bin_size);
                
                // sanity check
                if(l >= vecsize)
                    l = vecsize - 1;
                
                comp[l] += 1;
                ns++;
               
                break;
            }
        }
    }
}

