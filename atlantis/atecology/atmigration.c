/**
 * \file atmigration.c
 * \brief Processes for representing migration outside the model domain
 * \ingroup atEcology
 * \author Beth Fulton 	18/12/2016 CSIRO Marine Laboratories,  2005
 *
 *
 * 	Determines timing of migration that will execute in main biological code
 *
 * <b>Revisions:</b>
 */
/*  Open library routines */
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>
#include <atecology.h>

/*
* prototypes for static functions that are only defined in this file.
*/


/**
 *	\brief This routine resets annual or multiyear migration processes (done 1st day of the year)
 *	It is called by main() in the transport module.
 *
 *	For each active vertebrate and immigrating invert:<br>
 *	<ul>
 *	<li>Populate the MigrateAway and MigrateReturn arrays.</li>
 *	</ul>
 *	@param t  current time  (seconds)
 *	@param dt time range
 *	@param llogfp The model log file
 *
 *  Outputs :
 *  New migration times
 *
 * Calculate the migration dates of the given species, cohort and migrationID combination.
 * The resulting values are stored in the Leave_now and Return_Now arrays (used to be MigrateAway and MigrateBack).
 * If the species is the bm->which_Check species and we are debuging this code messages
 * are written to the log file.
 *
 *
 
 */

void Init_Migration(MSEBoxModel *bm, FILE *llogfp, int do_debug, int sp) {
    int startyr = 0;
    int endyr = (int) (ceil((bm->tstop - bm->tstart) / (365.0 * 86400.0)));
    double temp3, temp4, real_mig_length;
    int yr, migrationID, currentID, stage, sp_migrate, sp_Migrate_Time, sp_Migrate_Return, sp_Migrate_Time_orig, start_with_return, sp_Migrate_Years, this_YearsAway, mig_window, counter, this_currentID =0, potential_end_time, time_return, VarTime3, VarTime4, max_Return_Now, cohort, flagmother, motherstage, sp_Migrate_Time_chk, sp_Migrate_Return_chk, migID, b, ij, last_currentID;
    //int matage;
    //int last_return = -1;
    int nagemat = (int)(FunctGroupArray[sp].speciesParams[age_mat_id]);
    int dummy_return = (int)(bm->tstop / 86400.0);
    int Highest_date;
    
    if ( verbose > 0 )
        printf("Doing Init_Migration for %s with num_mig: %d\n", FunctGroupArray[sp].groupCode, FunctGroupArray[sp].num_migrate);
    
    /* Check for maternal care species */
    flagmother = (int) (FunctGroupArray[sp].speciesParams[flagmother_id]);
    motherstage = (FunctGroupArray[sp].numStages - 1);
    if (flagmother > 0) {
        for (counter = 0; counter < FunctGroupArray[sp].num_migrate; counter++) {
            for (stage = 0; stage < (FunctGroupArray[sp].numStages - 1); stage++) {
                if (MIGRATION[sp].StartDay_Prm[stage][counter] != MIGRATION[sp].StartDay_Prm[motherstage][counter])
                    quit("Juvenile %s not leaving on migration with their mother despite having maternal care - reset migration StartDay for stage %d or reset reproduction parameters\n", FunctGroupArray[sp].groupCode, stage);
                if (MIGRATION[sp].EndDay_Prm[stage][counter] != MIGRATION[sp].EndDay_Prm[motherstage][counter])
                    warn("Juvenile %s not returning from migration with their mother despite having maternal care - may need to reset migration StartDay for stage %d or reset reproduction parameters\n", FunctGroupArray[sp].groupCode, stage);
            }
        }
    }
    
    /* Check complexity of migrations */
    FunctGroupArray[sp].isComplexMigrator = FALSE;
    
    for (stage = 0; stage < FunctGroupArray[sp].numStages; stage++) {
        for(migrationID = 0; migrationID < FunctGroupArray[sp].num_migrate; migrationID++){
            sp_Migrate_Years = (int) (MIGRATION[sp].MinYearsAway_Prm[stage][migrationID]);
            //matage = (int) (FunctGroupArray[sp].speciesParams[age_mat_id] * FunctGroupArray[sp].ageClassSize);
            
            sp_Migrate_Time = (int) (MIGRATION[sp].StartDay_Prm[stage][migrationID]);
            sp_Migrate_Return = (int) (MIGRATION[sp].EndDay_Prm[stage][migrationID]);
            
            // Get it so all in linear time so if Return < Start due to looping calendar unwind
            sp_Migrate_Time_orig = sp_Migrate_Time;
            if (sp_Migrate_Time > sp_Migrate_Return)
                sp_Migrate_Return += 365;
            
            for(migID = 0; migID < FunctGroupArray[sp].num_migrate; migID++){
                if (migrationID == migID)
                    continue;  // Do't check against same ID entry
                
                sp_Migrate_Time_chk = (int) (MIGRATION[sp].StartDay_Prm[stage][migID]);
                sp_Migrate_Return_chk = (int) (MIGRATION[sp].EndDay_Prm[stage][migID]);
                
                // Get it so all in linear time so if Return < Start due to looping calendar unwind
                if (sp_Migrate_Time_chk > sp_Migrate_Return_chk)
                    sp_Migrate_Return_chk += 365;
                
                fprintf(llogfp, "%s stage %d migrationID %d sp_Migrate_Time %d sp_Migrate_Return %d (orig %d) num_mig: %d migID: %d sp_Migrate_Time_chk: %d sp_Migrate_Return_chk: %d checkLHS: %d checkRHS: %d\n", FunctGroupArray[sp].groupCode, stage, migrationID, sp_Migrate_Time, sp_Migrate_Return, (int)(MIGRATION[sp].EndDay_Prm[stage][migrationID]), FunctGroupArray[sp].num_migrate, migID, sp_Migrate_Time_chk, sp_Migrate_Return_chk, (sp_Migrate_Time_orig && sp_Migrate_Time), sp_Migrate_Return_chk);
                
                
                // Now check each case
                if ((sp_Migrate_Time_orig > sp_Migrate_Return_chk) && (sp_Migrate_Time > sp_Migrate_Return_chk)) {
                    
                    /* This is case where the second entry happens before first entry
                     1.               |-----|
                     2.    |-----|
                     This is more sensibly handled by reversing the order in the parameter file - will run faster and less chance of a cock up
                     */
                    quit("The migration entry %d (starting %d) for %s-%d happens completely before migration entry %d (return starting %d EndDay %d). Please fix that in the parameter file as it will help the code run faster and there is less chance something will go wrong\n", migrationID, sp_Migrate_Time, FunctGroupArray[sp].groupCode, stage, migID, sp_Migrate_Return_chk, MIGRATION[sp].EndDay_Prm[stage][migID]);
                    
                    FunctGroupArray[sp].isComplexMigrator = TRUE;
                }
                if ((sp_Migrate_Time < sp_Migrate_Time_chk) && (sp_Migrate_Return > sp_Migrate_Return_chk)) {
                    
                    /* This is case where the second entry happens within the time period of the first entry
                     1.     |---------------|
                     2.         |-----|
                     */
                    FunctGroupArray[sp].isComplexMigrator = TRUE;
                }
                if (sp_Migrate_Return > sp_Migrate_Time_chk) {
                    
                    /* This is case where the first entry end overlaps with the start time period of the second  entry
                     1.     |-----|
                     2.         |-----|
                     */
                    FunctGroupArray[sp].isComplexMigrator = TRUE;
                }
                if ( sp_Migrate_Time < sp_Migrate_Return_chk) {
                    
                    /* This is case where the first entry end overlaps with the start time period of the second  entry
                     1.         |-----|
                     2.     |-----|
                     */
                    FunctGroupArray[sp].isComplexMigrator = TRUE;
                }
                if ((sp_Migrate_Time > sp_Migrate_Time_chk) && (sp_Migrate_Return < sp_Migrate_Return_chk)) {
                    
                    /* This is case where the first entry happens within the time period of the second entry
                     1.         |-----|
                     2.     |---------------|
                     */
                    FunctGroupArray[sp].isComplexMigrator = TRUE;
                }
            }
        }
    }
    
    /* Pre-plan all migrations */
    max_Return_Now = -1;
    currentID = -1;
    last_currentID = -1;
    for (yr = startyr; yr <= endyr; yr++) {
        for (stage = 0; stage < FunctGroupArray[sp].numStages; stage++) {
            sp_migrate = FALSE;
             MIGRATION[sp].ActualMigrator[stage] = 0;
            for(migrationID = 0; migrationID < FunctGroupArray[sp].num_migrate; migrationID++){
                /* First check if even for real */
                if(!MIGRATION[sp].StartDay_Prm[stage][migrationID] && !MIGRATION[sp].EndDay_Prm[stage][migrationID]){
                    break;
                }

                // If yes then we can continue
                if(last_currentID < 0) {
                    currentID++;
                } else {
                    currentID = last_currentID + 1;
                }
                
                printf("Doing migration entry %d for %s\n", currentID, FunctGroupArray[sp].groupCode);
                
                // Set parameters
                MIGRATION[sp].IsAnnualMigration[currentID] = MIGRATION[sp].IsAnnualMigration_Prm[stage][migrationID];
                MIGRATION[sp].Leave_Period[currentID] = MIGRATION[sp].Leave_Period_Prm[stage][migrationID];
                MIGRATION[sp].Return_Period[currentID] = MIGRATION[sp].Return_Period_Prm[stage][migrationID];
                MIGRATION[sp].MinYearsAway[currentID] = MIGRATION[sp].MinYearsAway_Prm[stage][migrationID];
                MIGRATION[sp].MaxYearsAway[currentID] = MIGRATION[sp].MaxYearsAway_Prm[stage][migrationID];
                MIGRATION[sp].survival[currentID] = MIGRATION[sp].survival_Prm[stage][migrationID];
                MIGRATION[sp].growth[currentID] = MIGRATION[sp].growth_Prm[stage][migrationID];
                MIGRATION[sp].returnstock[currentID] = MIGRATION[sp].returnstock_Prm[stage][migrationID];
                
                MIGRATION[sp].IsPartialMigration[currentID] = MIGRATION[sp].IsPartialMigration_Prm[stage][migrationID];
                MIGRATION[sp].PartialMinAge[currentID] = MIGRATION[sp].PartialMigration_MinPrm[stage][migrationID];
                MIGRATION[sp].PartialMaxAge[currentID] = MIGRATION[sp].PartialMigration_MaxPrm[stage][migrationID];
                
                if(!stage) {
                    MIGRATION[sp].start_cohort[currentID] = 0;
                    for(cohort = 0; cohort < nagemat; cohort++) {
                        MIGRATION[sp].cohort_migrating[cohort][currentID] = 1; // Really only used in Leave_Now so ok if don't set to 1 for age classes might age into while away
                        MIGRATION[sp].ReprodAllowed[cohort][currentID] = MIGRATION[sp].ReprodAllowedPrm[stage][migrationID];
                    }
                } else {
                    MIGRATION[sp].start_cohort[currentID] = nagemat;
                    for(cohort = nagemat; cohort < FunctGroupArray[sp].numCohortsXnumGenes; cohort++) {
                        MIGRATION[sp].cohort_migrating[cohort][currentID] = 1;
                    }
                }
                
                for(cohort = 0; cohort < FunctGroupArray[sp].numCohortsXnumGenes; cohort++){
                    MIGRATION[sp].all_go[cohort][currentID] = MIGRATION[sp].all_go[cohort][migrationID];
                    MIGRATION[sp].ReprodAllowed[cohort][currentID] = MIGRATION[sp].ReprodAllowedPrm[stage][migrationID];
                }
                
                for(ij=0; ij<bm->nbox; ij++) {
                    MIGRATION[sp].Box[ij][currentID] = MIGRATION[sp].Box_Prm[ij][stage][migrationID];
                }
                
                MIGRATION[sp].Stagger[currentID][stagger_years_id] = MIGRATION[sp].Stagger_Prm[stage][migrationID][stagger_years_id];
                MIGRATION[sp].Stagger[currentID][return_stagger_id] = MIGRATION[sp].Stagger_Prm[stage][migrationID][return_stagger_id];
                
                /* If the period of this migration is not positive then move on to the next migration */
                if (MIGRATION[sp].Return_Period[currentID] > 0) {
                    sp_migrate = TRUE;
                    MIGRATION[sp].ActualMigrator[stage] = 1;
                }
                
                /*
                if(sp == bm->which_check) {
                    fprintf(llogfp, "%s-%d migrationID: %d currentID: %d sp_migrate: %d as period: %d\n", FunctGroupArray[sp].groupCode, stage, migrationID, currentID, sp_migrate, MIGRATION[sp].Return_Period[currentID]);
                }
                */
                
                /* Check if age while away */
                sp_Migrate_Time = (int) (MIGRATION[sp].StartDay_Prm[stage][migrationID]);
                sp_Migrate_Return = (int) (MIGRATION[sp].EndDay_Prm[stage][migrationID]);

                if (yr == startyr) {
                    if ((sp_migrate == TRUE) && (sp_Migrate_Time == 0) && (sp_Migrate_Return == 364)) {
                        warn("%s stage %d has Migrate_Time of 0 and Migrate_Return of 364 - this will mean that this stage of this group is NEVER in the model. Did you really mean that? If not (i.e. you just don't want that stage to migrate at all) please set Migrate_Period to 0 (zero)\n", FunctGroupArray[sp].groupCode, stage);
                    }
                }
                
                /* Check for zero length migrations */
                if (sp_Migrate_Time < sp_Migrate_Return) {
                    real_mig_length = sp_Migrate_Return - sp_Migrate_Time;
                    start_with_return = 0;
                } else if (sp_Migrate_Time > sp_Migrate_Return) {
                    real_mig_length = sp_Migrate_Return + 364 - sp_Migrate_Time;
                    start_with_return = 1;
                } else {
                    real_mig_length = 0;
                    start_with_return = 0;
                }

                /**
                if(sp == bm->which_check) {
                    fprintf(llogfp, "%s Stage %d real_mig_length: %e start_with_return: %d sp_Migrate_Time: %d sp_Migrate_Return: %d\n", FunctGroupArray[sp].groupCode, stage, real_mig_length, start_with_return, sp_Migrate_Time, sp_Migrate_Return);
                }
                **/
                
                if(FunctGroupArray[sp].multiyr_mig) {
                    sp_Migrate_Years = (int) (MIGRATION[sp].MinYearsAway[currentID]);
                    this_YearsAway = sp_Migrate_Years;
                    if(!this_YearsAway)
                        this_YearsAway = 1;
                } else {
                    sp_Migrate_Years = 0;
                    this_YearsAway = 1;
                            
                    /* If single year migrations and real_mig_length value is not greater than one move onto the next migration */
                    if (fabs(real_mig_length) <= 1) {
                        sp_migrate = FALSE;
                        MIGRATION[sp].ActualMigrator[stage] = 0;
                    }
                }
                
                /* Update check on real_mig_length and start_with_return */
                if (sp_Migrate_Years) {
                    start_with_return = 1;
                }
                
                if (sp_migrate == TRUE) {
                    mig_window = 14;

                    /* Add wobble to dates if required */
                    if (bm->flagrandom) {
                        temp3 = drandom(-mig_window, mig_window);
                        if (temp3 < 0.0)
                            VarTime3 = (int) (ceil(temp3 - 0.5));
                        else
                            VarTime3 = (int) (floor(temp3 + 0.5));
                        
                        temp4 = drandom(-mig_window, mig_window);
                        if (temp4 < 0.0)
                            VarTime4 = (int) (ceil(temp4 - 0.5));
                        else
                            VarTime4 = (int) (floor(temp4 + 0.5));
                    } else {
                        VarTime3 = 0;
                        VarTime4 = 0;
                    }
                    
                    this_currentID = currentID;
                    if (start_with_return && !yr) {
                        // Start with returns so have those come in first then proceed as normal
                        for (counter = 0; counter < this_YearsAway; counter++) {
                            this_currentID = currentID + counter * ((int)(FunctGroupArray[sp].num_migrate));
                            
                            printf("Step1 Doing currentID: %d this_currentID: %d for %s\n", currentID, this_currentID, FunctGroupArray[sp].groupCode);
                            
                            MIGRATION[sp].Leave_Period[this_currentID] = MIGRATION[sp].Leave_Period[currentID];
                            MIGRATION[sp].Return_Period[this_currentID] = MIGRATION[sp].Return_Period[currentID];
                            MIGRATION[sp].MinYearsAway[this_currentID] = MIGRATION[sp].MinYearsAway[currentID];
                            MIGRATION[sp].survival[this_currentID] = MIGRATION[sp].survival[currentID];
                            MIGRATION[sp].growth[this_currentID] = MIGRATION[sp].growth[currentID];
                            MIGRATION[sp].returnstock[this_currentID] = MIGRATION[sp].returnstock[currentID];
                            MIGRATION[sp].Stagger[this_currentID][stagger_years_id] = MIGRATION[sp].Stagger[currentID][stagger_years_id];
                            MIGRATION[sp].Stagger[this_currentID][return_stagger_id] = MIGRATION[sp].Stagger[currentID][return_stagger_id];
                            
                            for(cohort = 0; cohort < FunctGroupArray[sp].numCohortsXnumGenes; cohort++){
                                MIGRATION[sp].all_go[cohort][this_currentID] = MIGRATION[sp].all_go[cohort][currentID];
                                MIGRATION[sp].cohort_migrating[cohort][this_currentID] = MIGRATION[sp].cohort_migrating[cohort][currentID];
                            }
                            
                            for(ij=0; ij<bm->nbox; ij++) {
                                MIGRATION[sp].Box[ij][this_currentID] = MIGRATION[sp].Box[ij][currentID];
                            }

                            MIGRATION[sp].Leave_Now[this_currentID] = sp_Migrate_Time - 364; // As already away
                            MIGRATION[sp].Return_Now[this_currentID] = sp_Migrate_Return + VarTime4 + counter * 365; // Assume all the initial multi-yr ones come in on same day of year but with additional years
                            
                            if (MIGRATION[sp].Return_Now[this_currentID] > max_Return_Now )
                                max_Return_Now = MIGRATION[sp].Return_Now[this_currentID];
                            
                            /* Initalise stagger for those starting outside the model */
                            if (MIGRATION[sp].Leave_Now[this_currentID] < 0) {
                                for(cohort = 0; cohort < FunctGroupArray[sp].numCohortsXnumGenes; cohort++){
                                    MIGRATION[sp].num_stagger[this_currentID] = 0;
                                }
                                if(FunctGroupArray[sp].multiyr_mig) {
                                    if ((MIGRATION[sp].MinYearsAway[this_currentID] > 1) && (MIGRATION[sp].Stagger[this_currentID][return_stagger_id])) {
                                        for(cohort = 0; cohort < FunctGroupArray[sp].numCohortsXnumGenes; cohort++){
                                            MIGRATION[sp].num_stagger[this_currentID] = MIGRATION[sp].Stagger[this_currentID][stagger_years_id];
                                        }
                                    }
                                }
                            }
                            /**
                            if ((do_debug && (sp == bm->which_check)) || (bm->debug == debug_prey_biology_process && bm->dayt >= bm->checkstart && bm->dayt < bm->checkstop)) {
                                fprintf(llogfp, "VERSION 0 CHECK A Time: %e, sp: %s-%d, this_currentID: %d LeaveNow: %d, ReturnNow: %d (sp_Migrate_Time: %d, VarTime4: %d, counter: %d)\n", bm->dayt, FunctGroupArray[sp].groupCode, stage, this_currentID, MIGRATION[sp].Leave_Now[this_currentID], MIGRATION[sp].Return_Now[this_currentID], sp_Migrate_Time, VarTime4, counter);
                            }
                            **/
                        }
                    } else {
                        /*
                        if (start_with_return ) {
                            this_currentID = currentID + FunctGroupArray[sp].num_migrate * (this_YearsAway - 1) - 1; // So have made room for the returns in first year (-1 to correct for 0 start index)
                            
                            printf("Step2 Doing currentID: %d this_currentID: %d for %s\n", currentID, this_currentID, FunctGroupArray[sp].groupCode);
                            
                            if (this_currentID <= last_currentID) {
                                this_currentID = last_currentID + 1;
                            }
                            
                            MIGRATION[sp].Leave_Period[this_currentID] = MIGRATION[sp].Leave_Period[currentID];
                            MIGRATION[sp].Return_Period[this_currentID] = MIGRATION[sp].Return_Period[currentID];
                            MIGRATION[sp].MinYearsAway[this_currentID] = MIGRATION[sp].MinYearsAway[currentID];
                            MIGRATION[sp].survival[this_currentID] = MIGRATION[sp].survival[currentID];
                            MIGRATION[sp].growth[this_currentID] = MIGRATION[sp].growth[currentID];
                            MIGRATION[sp].returnstock[this_currentID] = MIGRATION[sp].returnstock[currentID];
                            MIGRATION[sp].Stagger[this_currentID][stagger_years_id] = MIGRATION[sp].Stagger[currentID][stagger_years_id];
                            MIGRATION[sp].Stagger[this_currentID][return_stagger_id] = MIGRATION[sp].Stagger[currentID][return_stagger_id];
                            
                            for(cohort = 0; cohort < FunctGroupArray[sp].numCohortsXnumGenes; cohort++){
                                MIGRATION[sp].all_go[cohort][this_currentID] = MIGRATION[sp].all_go[cohort][currentID];
                                MIGRATION[sp].cohort_migrating[cohort][this_currentID] = MIGRATION[sp].cohort_migrating[cohort][currentID];
                            }
                            
                            for(ij=0; ij<bm->nbox; ij++) {
                                MIGRATION[sp].Box[ij][this_currentID] = MIGRATION[sp].Box[ij][currentID];
                            }

                        } else {
                            this_currentID = currentID;
                            if (this_currentID <= last_currentID) {
                                this_currentID = last_currentID + 1;
                            }
                        }
                        */
                        
                        MIGRATION[sp].Leave_Now[this_currentID] = sp_Migrate_Time + VarTime3 + yr * 365;  // So start day for year being dealt with

                        if( FunctGroupArray[sp].multiyr_mig ) {
                            potential_end_time = sp_Migrate_Time + VarTime3 + yr * 365 + sp_Migrate_Years * 365;
                            time_return = (int)(365 * floor(potential_end_time/365)) + sp_Migrate_Return + VarTime4;  // So get year return and then add on the return day of year to get the final return date
                            
                        } else {
                            //time_return = sp_Migrate_Time + VarTime3 + yr * 365 + real_mig_length; // Annual migrations so its start time + period away
                            time_return = sp_Migrate_Return + VarTime3 + yr * 365;
                        }
                        MIGRATION[sp].Return_Now[this_currentID] = time_return;

                        if (start_with_return) {
                            MIGRATION[sp].Leave_Now[this_currentID] = MIGRATION[sp].Leave_Now[this_currentID] - 365;
                            MIGRATION[sp].Return_Now[this_currentID] = MIGRATION[sp].Return_Now[this_currentID] - 365;
                            
                        }

                        if (MIGRATION[sp].Return_Now[this_currentID] > max_Return_Now ) {
                            max_Return_Now = MIGRATION[sp].Return_Now[this_currentID];
                        }
                        
                        if ( MIGRATION[sp].Leave_Now[this_currentID] > MIGRATION[sp].Return_Now[this_currentID]) {  // Sanity check so come back
                            MIGRATION[sp].Return_Now[this_currentID] += (365 * this_YearsAway);
                        }
                        
                        /**
                        if (sp == 39) {
                            fprintf(llogfp, "CHECK B sp: %s-%d, yr: %d leave_now = %d, return_now = %d, this_currentID = %d, start_with_return = %d, multiyr_mig: %d, this_YearsAway: %d, VarTime3: %d, sp_Migrate_Time: %d, sp_Migrate_Return: %d, VarTime4: %d, return_term1: %d\n", FunctGroupArray[sp].groupCode, stage, yr, MIGRATION[sp].Leave_Now[this_currentID], MIGRATION[sp].Return_Now[this_currentID], this_currentID, start_with_return, FunctGroupArray[sp].multiyr_mig, this_YearsAway, VarTime3, sp_Migrate_Time, sp_Migrate_Return, VarTime4, (int)(365 * floor(potential_end_time/365)));
                        }
                        **/
                    }
                }
                
                last_currentID = this_currentID;
            }
        }

        /* Reinitialise */
        //last_return = -1;
    }
    
    /* Set up the proportion aging - defalt values (can be updated as model runs) */
    for (counter = 0; counter < MIGRATION[sp].num_in_queue; counter++) {
        for(cohort = 0; cohort < FunctGroupArray[sp].numCohortsXnumGenes; cohort++){
            for (b = 0; b < FunctGroupArray[sp].ageClassSize; b++) {
                MIGRATION[sp].pop_ratio[counter][cohort][b] = 1.0 / ((double)(FunctGroupArray[sp].ageClassSize));
                MIGRATION[sp].current_pop_ratio[cohort][b] = 1.0 / ((double)(FunctGroupArray[sp].ageClassSize));
            }
        }
        
        /* Final checks */
        if(MIGRATION[sp].Return_Now[counter] > (MAXINT - 1)) {
            MIGRATION[sp].Return_Now[counter] = dummy_return + 1;  // Needed for purposes of record keeping, never executed
        
            if( MIGRATION[sp].Return_Now[counter] < max_Return_Now) {
                MIGRATION[sp].Return_Now[counter] = max_Return_Now + 1;  // Needed for purposes of record keeping, never executed
            }
        }

        if(( MIGRATION[sp].Return_Now[counter] < MAXINT) && (MIGRATION[sp].Leave_Now[counter] > (MAXINT - 1)))
            MIGRATION[sp].Leave_Now[counter] = MIGRATION[sp].Return_Now[counter]; // Needed for purposes of record keeping, never executed
        
        /**
        if (sp == 39) {
            fprintf(llogfp, "CHECK C sp: %s counter %d, leave_now = %d, return_now = %d\n", FunctGroupArray[sp].groupCode, counter, MIGRATION[sp].Leave_Now[counter], MIGRATION[sp].Return_Now[counter]);
        }
        **/
       
        /** Error check printout **
        if ((sp == 33) || ((sp > 8) && (sp < 12))) {
            for(cohort = 0; cohort < FunctGroupArray[sp].numCohortsXnumGenes; cohort++){
                fprintf(llogfp, "Counter: %d, sp: %s-%d MigrateAway: %d, MigrateBack: %d num_migrate: %d multiyr_mig: %d MinYearsAway: %d cohort_migrating: %d leave_period: %d return_period: %d\n", counter,  FunctGroupArray[sp].groupCode, cohort, MIGRATION[sp].Leave_Now[counter], MIGRATION[sp].Return_Now[counter], FunctGroupArray[sp].num_migrate, FunctGroupArray[sp].multiyr_mig, MIGRATION[sp].MinYearsAway[counter], MIGRATION[sp].cohort_migrating[cohort][counter], MIGRATION[sp].Leave_Period[counter], MIGRATION[sp].Return_Period[counter]);
            }
        }
        **/
    }
    
    /** FInal sanity check **/
    Highest_date = -MAXINT;
    for (counter = 0; counter < MIGRATION[sp].num_in_queue; counter++) {
        if (MIGRATION[sp].Leave_Now[counter] > Highest_date) {
            Highest_date = MIGRATION[sp].Leave_Now[counter];
        }
        
        if (MIGRATION[sp].Leave_Now[counter] < Highest_date) {
            FunctGroupArray[sp].isComplexMigrator = TRUE;
        }
    }
    
    return;

}

/**
 *    \brief This routine updates count of how many migrations have been completed so don't have to search the whole queue each time (ignore ones already finished).
 *
 **/
void Update_Migration_Index(MSEBoxModel *bm, FILE *llogfp){
    int species, counter;
    int check_day = (int)(bm->dayt);
    int migdone = 0, last_mig_done = 0;
    
    for (species = 0; species < bm->K_num_tot_sp; species++) {
        for (counter = MIGRATION[species].num_in_queue_done; counter < MIGRATION[species].num_in_queue; counter++) {
            last_mig_done = migdone;
            migdone = 0;
            if (check_day > (MIGRATION[species].Return_Now[counter] +  MIGRATION[species].Return_Period[counter] + 2)) { // +2 so have time for stagger to happen
                migdone++;
                
                    
                //fprintf(llogfp, "Time: %e %s Migdone %d as dayt: %e vs Return_Now: %d Period: %d\n", bm->dayt, FunctGroupArray[species].groupCode, migdone, bm->dayt, MIGRATION[species].Return_Now[counter], MIGRATION[species].Return_Period[counter]);
            }
            
            if (!migdone && (last_mig_done > 0))
                break;  // As got some weird shit order happening so can't skip any

            if(!migdone && !last_mig_done) // Lots left to do in the queue
                break;
            
            if (migdone) { // Can happily iterate the counter past ones already done
                MIGRATION[species].num_in_queue_done++;  // All done with this entry so can truncate searchable list
                
                //fprintf(llogfp, "Time: %e %s updated num_in_queue_done (now %d) as migdone: %d and numStages: %d\n", bm->dayt, FunctGroupArray[species].groupCode, MIGRATION[species].num_in_queue_done, migdone, FunctGroupArray[species].numStages);
            }
        }
    }
    return;
}

/**
 *     Check the migration data for each group. This must be called after the cohort stages are calculated.
 *
 */
void Check_Migration(MSEBoxModel *bm){
    //int endyr = 1 + (int) (ceil((bm->tstop - bm->tstart) / (365.0 * 86400.0)));
    int sp, cohort, stage, done_it, qid, b, spmigrate, num_away, num_mig, max_years, this_num_migs, do_debug, counter,
    leave_day, return_day, num_cohorts_away;
    //int kmig_check;
    //int age_mat;
    int nyears = (int) (ceil((bm->tstop - bm->tstart) / (365.0 * 86400.0))) + 1;
    double ngenes;
    
    for (sp = 0; sp < bm->K_num_tot_sp; sp++) {
        if (FunctGroupArray[sp].isVertebrate == FALSE && FunctGroupArray[sp].isMobile == TRUE)
            for (cohort = 0; cohort < FunctGroupArray[sp].numCohorts; cohort++){
                stage = FunctGroupArray[sp].cohort_stage[cohort];
                Check_Migration_Data(bm, sp, stage);
            }
    }
    
    /* Set up Migration matrices for those starting outside model domain */
    // Removed if(FunctGroupArray[sp].isVertebrate == TRUE) check as apply for all species
    for (sp = 0; sp < bm->K_num_tot_sp; sp++) {
        do_debug = 0;
        //if(sp == bm->which_check)
        //    do_debug = 1;

        MIGRATION[sp].num_in_yr = (int)FunctGroupArray[sp].num_migrate;
        this_num_migs = 0;
        if(FunctGroupArray[sp].num_migrate > 0){
            this_num_migs = (FunctGroupArray[sp].numStages * nyears * (int)FunctGroupArray[sp].num_migrate) + 1;
            
            if(FunctGroupArray[sp].multiyr_mig) {
                this_num_migs = (nyears * FunctGroupArray[sp].multiyr_mig * FunctGroupArray[sp].numStages) * (FunctGroupArray[sp].num_migrate + 1);
            }
        }
        MIGRATION[sp].num_in_queue = this_num_migs;
        MIGRATION[sp].num_in_queue_done = 0;
        
        //fprintf(logfp,"%s has num_in_queue = %d\n", FunctGroupArray[sp].groupCode, MIGRATION[sp].num_in_queue);
        
        done_it = 0;
        spmigrate = FunctGroupArray[sp].num_migrate;
        ngenes = (double) (FunctGroupArray[sp].numGeneTypes);
        num_mig = FunctGroupArray[sp].num_migrate;
        //age_mat = (int) (FunctGroupArray[sp].speciesParams[age_mat_id]);
        
        if (!spmigrate)
            continue;

        Init_Migration(bm, bm->logFile, do_debug, sp); // Inialise timing so can do appropriate density mapping below given potential for multiyr migrations

        for (b = 0; b < FunctGroupArray[sp].numCohortsXnumGenes; b++) {
            num_away = FunctGroupArray[sp].num_migrate;
            stage = FunctGroupArray[sp].cohort_stage[b];
            cohort = (int)floor (b / FunctGroupArray[sp].numGeneTypes);
            
            /* Check to make sure that the period is 1 if kmigslow is 0 */
            if(k_migslow == 0){
                for(qid = 0; qid < FunctGroupArray[sp].num_migrate; qid++){
                    if( MIGRATION[sp].Return_Period_Prm[stage][qid] > 1){
                        quit("Error: You have set the k_migslow to 0 indicating you want all your groups to migrate in a single day but the migration period for %s (ID %d), stage %d migration %d, is greater than 1 day (%d)\n", FunctGroupArray[sp].groupCode, sp, stage, qid, MIGRATION[sp].Return_Period_Prm[stage][qid]);
                    }
                }
            }
            
            if (FunctGroupArray[sp].multiyr_mig){
                max_years = -MAXINT;
                for(qid = 0; qid < num_mig; qid++){
                    if (MIGRATION[sp].IsAnnualMigration_Prm[stage][qid] < 1){
                        if (MIGRATION[sp].MaxYearsAway_Prm[stage][qid] > max_years) {
                            max_years = MIGRATION[sp].MaxYearsAway_Prm[stage][qid];
                        }
                    } else {
                        max_years = 1;
                    }
                }
                if (max_years < MAXINT) {
                    max_years = 1;
                }
                num_away *= max_years;
                
                // Check if needed due to aging in multiyear migration
                num_cohorts_away = ceil(b + (max_years / FunctGroupArray[sp].ageClassSize));
                if (num_cohorts_away > FunctGroupArray[sp].numCohortsXnumGenes) {
                    num_cohorts_away = FunctGroupArray[sp].numCohortsXnumGenes;
                }
                for(qid = b; qid < num_cohorts_away; qid++){
                    FunctGroupArray[sp].needed_for_age_away[qid] = 1;
                }
                
                if (!num_away && FunctGroupArray[sp].needed_for_age_away[b] && (MIGRATION[sp].InitDEN[b][0] > 0.0)) {
                    if (!max_years) {
                        for(qid = 0; qid < num_mig; qid++){
                            printf("Something is not right with multi yr migrations - qid: %d - for %s you have MultiyrMigrate set to %d, with %d migration(s) per year, you also have %e as the KMIGa entry for age class %d, but for this life history stage (%d) for YearsAway you have Min %d, Max: %d (%d)\n", qid, FunctGroupArray[sp].groupCode, FunctGroupArray[sp].multiyr_mig, num_mig, MIGRATION[sp].InitDEN[b][0], b, stage, MIGRATION[sp].MinYearsAway_Prm[stage][qid], MIGRATION[sp].MaxYearsAway_Prm[stage][qid], max_years);
                        }
                        quit("\n");
                    }
                }
            }
            
            //fprintf(bm->logFile, "Doing %s-%d (stage: %d, age_mat: %d, spmigrate: %d, num_away: %d, num_mig: %d)\n", FunctGroupArray[sp].groupCode, b, stage, age_mat, spmigrate, num_away, num_mig);

            /* Deal migrations spanning years end - assume flat numbers returning per year for those outside the system */
            //kmig_check = 0;
            for (counter = 0; counter < MIGRATION[sp].num_in_queue; counter++) {
                leave_day = MIGRATION[sp].Leave_Now[counter];
                return_day = MIGRATION[sp].Return_Now[counter];
                
                // Load in KMIG values if (a) starting outside model as migrating in from previous migraiton events or because accidentally loaded in KMIG array instead of nc
                if (( leave_day <= 0 ) && ( return_day > 0 )){  // Do not require  MIGRATION[sp].cohort_migrating[b][counter] > 0 as need values for older ages in case of aging before enter the model domain
                    
                    // TODO: Think if this is the best way of handling all the migration options - all away or only a cohort at a time
                    MIGRATION[sp].SN[b][counter] = MIGRATION[sp].InitSN[b][0] / ngenes;   // As all KMIG saved Init [0] entry on read in
                    MIGRATION[sp].RN[b][counter] = MIGRATION[sp].InitRN[b][0] / ngenes;
                    MIGRATION[sp].DEN[b][counter] = MIGRATION[sp].InitDEN[b][0] / ngenes;

                    /**
                    //if (bm->debug && (bm->which_check == sp)) {
                    if (sp == 33) {
                        fprintf(bm->logFile, "%s-%d Migration array [counter %d] num set to %e (SN: %e RN: %e) as ngenes: %e\n", FunctGroupArray[sp].groupCode, b, counter,
                            MIGRATION[sp].DEN[b][counter], MIGRATION[sp].SN[b][counter], MIGRATION[sp].RN[b][counter], ngenes);
                    }
                    **/
                    
                    done_it++;
                }
            }
            if(!done_it) {
                //kmig_check = 1;
                for(qid = 0; qid < num_away; qid++){
                    if (!FunctGroupArray[sp].multiyr_mig && (MIGRATION[sp].InitDEN[b][qid] > 0) && (MIGRATION[sp].StartDay_Prm[stage][qid] <= MIGRATION[sp].EndDay_Prm[stage][qid])) {
                        warn("%s-%d had %e migrants in KMIG array but its migration (%d) dates (T: %d R: %d) say that it shouldn't be away at the start of the model run\n",
                                 FunctGroupArray[sp].groupCode, b, MIGRATION[sp].InitDEN[b][qid], qid, MIGRATION[sp].StartDay_Prm[stage][qid], MIGRATION[sp].EndDay_Prm[stage][qid]);
                        fprintf(bm->logFile, "%s-%d had %e migrants in KMIG array but its migration (%d) dates (T: %d R: %d) say that it shouldn't be away at the start of the model run\n", FunctGroupArray[sp].groupCode, b, MIGRATION[sp].InitDEN[b][qid], qid, MIGRATION[sp].StartDay_Prm[stage][qid], MIGRATION[sp].EndDay_Prm[stage][qid]);
                    }
                }
            }
            
        }
        
        /** Output the results **/
        if(sp == bm->which_check) {
            for (counter = 0; counter < MIGRATION[sp].num_in_queue; counter++) {
                for (b = 0; b < FunctGroupArray[sp].numCohortsXnumGenes; b++) {
                    leave_day = MIGRATION[sp].Leave_Now[counter];
                    return_day = MIGRATION[sp].Return_Now[counter];
            
                    //fprintf(bm->logFile, "Counter: %d, sp: %s-%d counter: %d DEN: %e SN: %e RN: %e leave_day: %d return_day: %d\n", counter, FunctGroupArray[sp].groupCode, b, counter, MIGRATION[sp].DEN[b][counter], MIGRATION[sp].SN[b][counter], MIGRATION[sp].RN[b][counter], leave_day, return_day);
                }
            }
        }
        /**/
        
    }
    
    return;
}

/* Routine to determine which cohort animals return to */
void Init_Migration_Age_Check(MSEBoxModel *bm, FILE *llogfp, int do_debug, int sp) {
    int cohort, age_away, qid, aid, ageing_time, qqid, max_num_event, diff_nc, sp_Migrate_Years, nextcid, start_return_chrt;
    int spmigrate = FunctGroupArray[sp].num_migrate;
    double p_ageup;

    
    if (!spmigrate)
        return;
    
    if (verbose)
        printf("Doing Init_Migration_Age_Check for %s\n", FunctGroupArray[sp].groupCode);

    max_num_event = 0;
    // For each migration check how many aging events occur while away
    for (qid = 0; qid < MIGRATION[sp].num_in_queue; qid++) {
        if (MIGRATION[sp].Stagger[qid][return_stagger_id] && !MIGRATION[sp].Stagger[qid][stagger_years_id]) {
                quit("For %s StaggerYears cannot be set to %d if stagger_return is set to %d, please fix the parametersation\n", FunctGroupArray[sp].groupCode, (int)(FunctGroupArray[sp].speciesParams[stagger_years_id]), MIGRATION[sp].Stagger[qid][return_stagger_id]);
        }
        
        age_away = 0;
        cohort = MIGRATION[sp].start_cohort[qid];
        for (aid = 0; aid < EMBRYO[sp].num_in_aging_queue; aid++) {
            ageing_time = EMBRYO[sp].Age_Now[cohort][aid];
            if ((ageing_time >= MIGRATION[sp].Leave_Now[qid]) && (ageing_time < (MIGRATION[sp].Return_Now[qid] + MIGRATION[sp].Return_Period[qid]))){
                    age_away++;
            }
        }
        
        /**
        if (sp == 33) {
            fprintf(llogfp,"%s-%d age_away: %d vs num_aging_event: %f\n", FunctGroupArray[sp].groupCode, cohort, age_away, MIGRATION[sp].num_aging_event[qid]);
        }
        **/

        if (age_away > MIGRATION[sp].num_aging_event[qid])
            MIGRATION[sp].num_aging_event[qid] = age_away;
            
        if (max_num_event < MIGRATION[sp].num_aging_event[qid]) {
            max_num_event = MIGRATION[sp].num_aging_event[qid];
        }
            
        // Then depending on ageclasssize determine how split up recruits will be
        start_return_chrt = ((int) floor(MIGRATION[sp].num_aging_event[qid] / FunctGroupArray[sp].ageClassSize)) + MIGRATION[sp].start_cohort[qid];

        if (start_return_chrt > MIGRATION[sp].start_return_chrt[qid]) {
            MIGRATION[sp].start_return_chrt[qid] = start_return_chrt;
            if (MIGRATION[sp].start_return_chrt[qid] > (FunctGroupArray[sp].numCohortsXnumGenes - 1)) {
                MIGRATION[sp].start_return_chrt[qid] = (FunctGroupArray[sp].numCohorts - 1) * FunctGroupArray[sp].numGeneTypes;
            }
        }
                
        for (qqid = 0; qqid < MIGRATION[sp].num_in_queue; qqid++) {
            if (qid != qqid) {
                if ((MIGRATION[sp].Leave_Now[qqid] >= MIGRATION[sp].Return_Now[qid]) && (MIGRATION[sp].Leave_Now[qqid] <= (MIGRATION[sp].Return_Now[qid] + MIGRATION[sp].Return_Period[qid]))){
                    MIGRATION[sp].Leave_Now[qqid] += 1;  // So have returned before leave again
                    fprintf(llogfp, "%s qqid %d had clash of one group returning as next group leaves so pushing out leaving so there and ready to go (now day %d)\n", FunctGroupArray[sp].groupCode, qqid, MIGRATION[sp].Leave_Now[qqid]);
                        warn("%s had clash of one group returning as next group leaves so pushing out leaving so there and ready to go\n", FunctGroupArray[sp].groupCode);
                }
            }
        }
    }
        
    // Sort the aging that happens before the run starts for all those starting outside the model
    for (qid = 0; qid < MIGRATION[sp].num_in_queue; qid++) {
        for (cohort = (int)floor((FunctGroupArray[sp].numCohortsXnumGenes) - 1); cohort >= 0; cohort--) {
            if (MIGRATION[sp].Leave_Now[qid] < 0) {
                diff_nc = (int)ceil((MIGRATION[sp].Return_Now[qid] - MIGRATION[sp].Leave_Now[qid]) / 365);
                sp_Migrate_Years = max(1,((int)(MIGRATION[sp].MinYearsAway[qid])));
                MIGRATION[sp].yrs_to_age_pre_model[qid] = sp_Migrate_Years - diff_nc;
                MIGRATION[sp].end_pt[qid] = cohort + MIGRATION[sp].yrs_to_age_pre_model[qid];
                
                if (MIGRATION[sp].end_pt[qid] > (FunctGroupArray[sp].numCohortsXnumGenes - 1)) {
                    MIGRATION[sp].end_pt[qid] = (FunctGroupArray[sp].numCohortsXnumGenes - 1);
                }
            }
        }
    }
                                
    for (qid = 0; qid < MIGRATION[sp].num_in_queue; qid++) {
        for (qqid = 0; qqid < MIGRATION[sp].yrs_to_age_pre_model[qid]; qqid++) {
            for (cohort = MIGRATION[sp].end_pt[qid] - 1; cohort >= MIGRATION[sp].start_cohort[qid]; cohort--) {
                
                nextcid = cohort + FunctGroupArray[sp].numGeneTypes;
                if(nextcid > (FunctGroupArray[sp].numCohortsXnumGenes - 1)) {
                    nextcid = (FunctGroupArray[sp].numCohortsXnumGenes - 1);
                }
                
                if (FunctGroupArray[sp].isVertebrate == TRUE){
                    p_ageup = Get_Proportion_Aging(bm, sp, cohort, do_debug, llogfp);  // TODO: Need to allow for these to have evolution act
                } else {
                    p_ageup = shiftVERT[sp][cohort][RN_id]; // p_ageup stored here for biomass pool age structured groups
                }
                
                MIGRATION[sp].DEN[nextcid][qid] += (p_ageup * MIGRATION[sp].DEN[cohort][qid]);
                MIGRATION[sp].DEN[cohort][qid] -= (p_ageup * MIGRATION[sp].DEN[cohort][qid]);
                
                /**
                //if ((sp == 48) || (sp == 63) || (sp == 64)) {
                if (sp == 33) {
                    fprintf(bm->logFile, "Init_Migration_Age_Check: %s-%d qid %d now has MIGden: %e MIGsn: %e, MIGrn: %e\n", FunctGroupArray[sp].groupCode, nextcid, qid, MIGRATION[sp].DEN[nextcid][qid], MIGRATION[sp].SN[nextcid][qid], MIGRATION[sp].RN[nextcid][qid]);
                }
                **/

                if((MIGRATION[sp].DEN[nextcid][qid] > 0) && (isnan(MIGRATION[sp].SN[nextcid][qid]) || isnan(MIGRATION[sp].RN[nextcid][qid]))) {
                    fprintf(bm->logFile, "Init_Migration_Age_Check Error: %s-%d qid %d now has MIGden: %e MIGsn: %e, MIGrn: %e please supply size values in KMIG arrays for sn and rn due to aging of migrants that occurs prior to the model start\n", FunctGroupArray[sp].groupCode, nextcid, qid, MIGRATION[sp].DEN[nextcid][qid], MIGRATION[sp].SN[nextcid][qid], MIGRATION[sp].RN[nextcid][qid]);
                    quit("Init_Migration_Age_Check Error: %s-%d qid %d now has NAN for SN ro RN after pre-model aging\n", FunctGroupArray[sp].groupCode, nextcid, qid);
                }
                
            }
        }
    }
    
    return;
}
