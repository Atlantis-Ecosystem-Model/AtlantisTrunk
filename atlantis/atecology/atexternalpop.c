/**
 \file
 \brief C file for process subroutines dealing with an external population model for vertebrates - originally developed by Marie Savina
 \ingroup atEcology

 File:	atexternalpop.c
 Author:	Beth Fulton
 Created:	13/12/2016
 Purpose:	C file for processes dealing with an external vertebrate populaiton (rather than being in status when outside the model domain)
 Revisions:


 **/

/*  Open library routines */
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <sjwlib.h>
#include "atecology.h"

/*************************************************************************//**

 */

void External_Box_Ecology(MSEBoxModel *bm, int b, double dt, FILE *llogfp) {
    double linf, Kbert, tzero, li_a, li_b, pR_sp, age_mat, age, old_age, Length, old_Length, lengthgrowth ;
    double nums, Growth, SN, RN, FRC_sp, mLext, mPext, mort, ageclasssize, X_RS, Density, Resp = 0, avail_intake,
        Growth_increment, old_Growth, prop_year_elapsed;
    int species, cohort, mid, Time_Spawn_sp, time_elapsed;
    //int pid;
    double dt_step = (bm->dt / 86400.0) / 365.0; // Divide by 365.0 because old_age is in years, and dt_step is in days, so need to divide divide dt by seconds in a day (to get to days) and then days in a year to get it into years.
    // Work on the MIGRATION array and only when box zero executing
    if (!b) {
        
        /**
        if(bm->debug == debug_external){
            fprintf(bm->logFile, "External_Box_Ecology running for box %d\n", bm->current_box);
        }
         **/
        
        Parameter_Q10(bm, &bm->boxes[b], bm->dt, 1, 0, 0, bm->boxes[b].inside.y, WC, bm->logFile);
    
        // Per cohort
        for (species = 0; species < bm->K_num_tot_sp; species++) {
            if(FunctGroupArray[species].speciesParams[flag_id] == TRUE){
                if (FunctGroupArray[species].isVertebrate == TRUE) {
                    if (FunctGroupArray[species].isExternal == TRUE) {

                        if (FunctGroupArray[species].numSpawns > 1) {
                            quit("External migration code does not work if multiple spawns per year (%s has %d) - contact coders\n", FunctGroupArray[species].groupCode, FunctGroupArray[species].numSpawns);
                        }
                        Time_Spawn_sp = (int) (FunctGroupArray[species].spawnSpeciesParams[0][Time_Spawn_id]);
                        if (bm->TofY < Time_Spawn_sp) {
                            time_elapsed = 365 - Time_Spawn_sp + bm->TofY;
                        } else {
                            time_elapsed = bm->TofY - Time_Spawn_sp;
                        }
                        prop_year_elapsed = (double)(time_elapsed) / 365.0;
                        
                        Density = 1.0;
                        linf = FunctGroupArray[species].speciesParams[linf_id];
                        Kbert = FunctGroupArray[species].speciesParams[Kbert_id];
                        tzero = FunctGroupArray[species].speciesParams[tzero_id];
                        li_a = FunctGroupArray[species].speciesParams[li_a_id];
                        li_b = FunctGroupArray[species].speciesParams[li_b_id];
                        pR_sp = FunctGroupArray[species].speciesParams[pR_id];
                        age_mat = FunctGroupArray[species].speciesParams[age_mat_id];
                        ageclasssize = FunctGroupArray[species].ageClassSize;

                        for (cohort = 0; cohort < FunctGroupArray[species].numCohortsXnumGenes; cohort++) {
                            X_RS = FunctGroupArray[species].X_RS[cohort];
                            //pid = FunctGroupArray[species].cohort_stage[cohort];
                            for (mid = MIGRATION[species].num_in_queue_done; mid < MIGRATION[species].num_in_queue; mid++) {
                                if ((bm->dayt >= MIGRATION[species].Leave_Now[mid]) && (bm->dayt < (MIGRATION[species].Return_Now[mid] + MIGRATION[species].Return_Period[mid]))) {
                                    nums = MIGRATION[species].DEN[cohort][mid];
                        
                                    /* Only do if there are actually numbers in this box */
                                    if(nums > bm->min_dens){
                            
                                        /**/
                                        if(bm->debug == debug_external){
                                            fprintf(bm->logFile, "Group %s-%d in migration array (mid: %d) extern pop with nums = %e", FunctGroupArray[species].groupCode, cohort, mid, nums);
                                        }
                                        /**/
                            
                                        //		Growth calculation
                                        //      ------------------
                                        if ( !bm->flag_extpop_growth_option ) {  // Marie's equations
                                            //			calculation of age
                                            age = ageclasssize * cohort + ageclasssize / 2.0;

                                            //			calculation of length
                                            Length = linf - (linf * exp(-Kbert * (age - tzero)));
                                
                                            //			daily growth in length (cm)
                                            lengthgrowth = Kbert * (linf - Length) / 365.0;
                                            //			daily growth in weight (mg N)
                                            Growth = li_a * pow(lengthgrowth,li_b) * 1000.0 / (bm->k_wetdry * bm->X_CN);
                                            Growth_increment = Growth;
                                        } else { // Isaac's equations
                                        // Alternative - get length at age and resulting mass and then difference that to get growth increment
                                            //            calculation of age
                                            old_age = ageclasssize * cohort + prop_year_elapsed - dt_step;
                                            if(old_age < 0.0)
                                                old_age = 0.0;
                                            
                                            age = ageclasssize * cohort + prop_year_elapsed;
                                            
                                            //            calculation of length (in cm)
                                            Length = linf - (linf * exp(-1.0 * Kbert * (age - tzero)));
                                            old_Length = linf - (linf * exp(-1.0 * Kbert * (old_age - tzero)));
                                            
                                            if(Length < 0.0)
                                                Length = small_num;
                                            if (old_Length < 0.0)
                                                old_Length = small_num;
                                            
                                            //            mass at age
                                            Growth = li_a * pow(Length,li_b) * 1000.0 / (bm->k_wetdry * bm->X_CN);
                                            old_Growth = li_a * pow(old_Length, li_b) * 1000.0 / (bm->k_wetdry * bm->X_CN);
                                            Growth_increment = Growth - old_Growth;
                                            if (Growth_increment < 0)
                                                Growth_increment = 0.0;
                                            
                                            if (isnan(Growth_increment)) {
                                                fprintf(llogfp, "Time: %e %s-%d has age: %e, old_age: %e, length: %e, old_length: %e, growth %e, old_growth %e, Growth_increment: %e as linf: %e, Kbert: %e, tzero: %e, li_a: %e, li_b: %e\n", bm->dayt, FunctGroupArray[species].groupCode, cohort, age, old_age, Length, old_Length, Growth, old_Growth, Growth_increment, linf, Kbert, tzero, li_a, li_b);
                                                quit("Time: %e %s-%d has age: %e, old_age: %e, length: %e, old_length: %e, growth %e, old_growth %e, Growth_increment: %e as linf: %e, Kbert: %e, tzero: %e, li_a: %e, li_b: %e\n", bm->dayt, FunctGroupArray[species].groupCode, cohort, age, old_age, Length, old_Length, Growth, old_Growth, Growth_increment, linf, Kbert, tzero, li_a, li_b);
                                            }
                                            
                                            /**
                                            if(bm->debug == debug_external){
                                                fprintf(llogfp, "Time: %e %s-%d has age: %e, old_age: %e, length: %e, old_length: %e, growth %e, old_growth %e, Growth_increment: %e as linf: %e, Kbert: %e, tzero: %e, li_a: %e, li_b: %e\n", bm->dayt, FunctGroupArray[species].groupCode, cohort, age, old_age, Length, old_Length, Growth, old_Growth, Growth_increment, linf, Kbert, tzero, li_a, li_b);
                                             }
                                            **/
                                        }
                                        //            calculation of the proportion of growth going into SN and RN
                                        SN = MIGRATION[species].SN[cohort][mid];
                                        RN = MIGRATION[species].RN[cohort][mid];

                                        // Include respiration in case of dynamic size structuring and respiration
                                        switch (flagresp) { //ASTAFIX - make sure all respirations cases are read, because now it just assumes boolean flag
                                            case no_resp:
                                                Resp = 0.0; /* Turning off respiration means the fish model is essentially/very similar to a Deriso-Schnute */
                                                break;
                                            case base_resp:
                                                Fish_Respiration(bm, species, SN, RN, X_RS, Density, &Resp);
                                                break;
                                            case deb_resp: // DEB model for respitaiton
                                                Fish_Respiration_DEB(bm, species, SN, RN, X_RS, Density, &Resp);
                                                break;
                                        }
                                        avail_intake = Growth_increment - Resp;
                                        
                                        /*
                                        if(bm->debug == debug_external){
                                            fprintf(bm->logFile, "growth Kbert: %e, linf: %e, Length: %e, tzero: %e ",
                                                Kbert, linf, Length, tzero);
                                            fprintf(bm->logFile, "li_a: %e, lengthgrowth: %e, li_b: %e with ", li_a, lengthgrowth, li_b);
                                        }
                                        */
                                        
                                        if (bm->flag_dynamicXRS) {  // Test needs to be here for consistency with old code
                                            Partition_Weight_Dynamic(bm, species, SN, RN, X_RS, &FRC_sp, avail_intake, llogfp);
                                        }
                                        else {
                                            Partition_Weight(bm, species, pR_sp, SN, RN, X_RS, &FRC_sp, llogfp);
                                        }

                                        /**
                                        //if(bm->debug == debug_external){
                                            fprintf(bm->logFile, " start_sn: %e start_rn: %e ",
                                                MIGRATION[species].SN[cohort][mid], MIGRATION[species].RN[cohort][mid]);
                                        //}
                                        **/

                                        
                                        //			allocation to SN and RN
                                        // SN AND RN was  "* dt" but not required as being done at dt sized steps anyway in incrementing mass to get Growth_increment
                                        MIGRATION[species].SN[cohort][mid] += (FRC_sp * Growth_increment);
                                        MIGRATION[species].RN[cohort][mid] += (Growth_increment - (FRC_sp * Growth_increment));
                            
                                        /**
                                        //if(bm->debug == debug_external){
                                            fprintf(bm->logFile, "Growth_increment: %e, new_sn: %e new_rn: %e\n",
                                                Growth_increment, MIGRATION[species].SN[cohort][mid], MIGRATION[species].RN[cohort][mid]);
                                        //}
                                        **/
                            
                                        //		Mortality calculation
                                        //		---------------------
                                        if (cohort < age_mat) {
                                            mLext = FunctGroupArray[species].cohortSpeciesParams[juv_id][mLext_id];
                                            mPext = FunctGroupArray[species].cohortSpeciesParams[juv_id][mPext_id];
                                        } else {
                                            mLext = FunctGroupArray[species].cohortSpeciesParams[adult_id][mLext_id];
                                            mPext = FunctGroupArray[species].cohortSpeciesParams[adult_id][mPext_id];
                                        }
                                        mort = (mLext + mPext) * nums;
                            
                                        MIGRATION[species].DEN[cohort][mid] -= (mort * dt);
                            
                            
                                        /*
                                        if(bm->debug == debug_external){
                                            fprintf(bm->logFile, "New_Density: %e, mort: %e, dt: %e ",
                                                MIGRATION[species].DEN[cohort][mid], mort, dt);
                                
                                            fprintf(bm->logFile, "mLext: %e, mPext: %e, nums: %e, mort: %e\n", mLext, mPext, nums, mort);
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
    }
    
    return;
    
}
