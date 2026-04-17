/**
 \file
 \brief C file for all aging and reproduction related routines
 \ingroup atecology

 File:	atdemography.c
 Author:	Beth Fulton
 Created:	20/10/2011
 Purpose:	C file for all age structured subroutines for reproduction and aging
 Revisions: 20/10/2011 Created atdemography.c from the atvertprocesses.c and atprocess.c files

 20-10-2011 Beth Fulton
 Changed VERTembryo to EMBRYO (and brought all into one array rather than
 having one for current year and one for previous year)

 07-11-2011 Beth Fulton
 Removed INVpopratio array - each cohort now treated separately

 17-11-2011 Beth Fulton
 Changed boxVERTpopratio to boxPopRatio and tempVERTpopratio
 to tempPopRatio (so deals with vertebrates and invertebrates together)

 18-11-2011 Beth Fulton
 Get_Cohort_Stage() converted from static int to int so could be called from atverprocesses.c

 15-01-2013 Beth Fulton
 Replaced tempVERTpopratio with tempPopRatio

 10-12-2013 Beth Fulton
 Combined Update_Age_Distrib and Update_RecAge_Distrib() as effectively identical code

 ***************************************************************************************

 Note the kind of agent is stored in FunctGroupArray[].groupAgeType with
 BIOMASS = Biomass pool (in mg N m-3 unless epibenthic then mg N m-2), no age structure
 AGE_STRUCTURED_BIOMASS - Biomass pool, but with age stanzas
 AGE_STRUCTURED - numbers tracked (per box) as are the structural and reserve weights (weights in mg N per individual)

 *************/

/*  Open library routines */
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include <math.h>
#include <sjwlib.h>
#include "atecology.h"

/* Prototypes of routines */
void Apply_Semelparous_Deaths(MSEBoxModel *bm, int species, int do_debug, int semelparous_migrants, int semelparous_parents, FILE *llogfp);
void Ecology_Do_External_Age_Structured_Spawning(MSEBoxModel *bm, int species, int flagmother, int do_debug, int *semelparous_migrants, FILE *llogfp);
void Ecology_Do_Internal_Age_Structured_Spawning(MSEBoxModel *bm, int species, int flagmother, int do_debug, int *semelparous_parents, FILE *llogfp);
void Ecology_Find_Embryoes(MSEBoxModel *bm, int species, int stock_id, double plankton, double CHLa, int do_debug, FILE *llogfp);
void Ecology_Find_Invert_Spawn(MSEBoxModel *bm, int species, double spawning_biomass, int stock_id, double plankton, double CHLa, FILE *llogfp);
void Ecology_Init_Spawn_Params(MSEBoxModel *bm, int species, int maxstock_id, int wclayer, FILE *llogfp);
void Find_Final_Invert_Recruit_Distribtuion(MSEBoxModel *bm, int species, int ngene, int qid, int stock_id, double spawned_biomass, int wclayer, FILE *llogfp);
void Find_Final_Recruit_Distribution(MSEBoxModel *bm, int species, double enviro_scalar, double vertdistrib, int wclayer, int stock_id, int do_debug, FILE *llogfp);
void Get_Latest_Additions(MSEBoxModel *bm, int wclayer, int species, int maxstock_id, int stock_id, double *lostden_zero, int need_update, int do_debug, FILE *llogfp);
void Get_Recruits(MSEBoxModel *bm, int species, int stock_id, double plankton, FILE *llogfp);
void Get_Settlers(MSEBoxModel *bm, int species, int wclayer, int stock_id, int *recruits_arrive, int do_debug, FILE *llogfp);
void Init_Aging_Arrays(MSEBoxModel *bm, int species, int stock_id, int maxstock_id, FILE *llogfp);
void Prepare_Age_Distrib(MSEBoxModel *bm, int sp, FILE *llogfp);
void Sanity_Check_Recruits(MSEBoxModel *bm, int species, FILE *llogfp);
void Store_Recruitment_Diagnostics(MSEBoxModel *bm, int species, int use_aggregate, int do_debug, FILE *llogfp);
void Record_End_Num(MSEBoxModel *bm, int species);
void Reset_Mortality(MSEBoxModel *bm, int sp);
void Update_Age_Distrib(MSEBoxModel *bm, int sp, int stock_id, int sp_ddepend_move, int rec_related, FILE *llogfp);
void Update_Aging_Numbers(MSEBoxModel *bm, int species, int cohort, int stock_id, double dennow, double this_p_ageup, int do_debug, FILE *llogfp);
void Update_Migration_Array(MSEBoxModel *bm, int species, int cohort, int maxstock_id, int do_debug, FILE *llogfp);
void Update_Local_dPool(MSEBoxModel *bm, int species, int cohort, double Spawned_Matter, double *recruitDEN, int sp_numGeneTypes, double *localWCTracers, int do_debug, FILE *llogfp);
void Update_Age_Queue(MSEBoxModel *bm, int species, int cohort, int do_debug, FILE *llogfp);
void Update_Spawn_Queue(MSEBoxModel *bm, int species, int cohort, int maxstock_id, int do_debug, FILE *llogfp);
void Update_Invert_Cohorts(MSEBoxModel *bm, int species, int stock_id, int maxstock_id, FILE *llogfp);
void Update_Invert_Aging(MSEBoxModel *bm, double *localWCTracers, int species, int cohort, int sp_Age_Now, int stock_id, int ij, int wclayer, int do_debug, FILE *llogfp);

double Ecology_Do_External_InVert_Spawning(MSEBoxModel *bm, int species, int cohort, int do_debug, FILE *llogfp);
double Ecology_Find_Invert_Recruits(MSEBoxModel *bm, int species, int ngene, int stock_id, double plankton, FILE *llogfp);
double Ecology_Get_Plankton(MSEBoxModel *bm, double *localWCTracers, FILE *llogfp);
double Get_Enviro_Recruit_Forcing(MSEBoxModel *bm, int species, int do_debug, FILE *llogfp);
double Get_Invert_Settlers(MSEBoxModel *bm, int species, int ngene, int wclayer, int stock_id, int do_debug, FILE *llogfp);
double Get_Vertical_Recruit_Position(int species, int wclayer, int maxdeep, int totaldeep);
double Get_Numbers_Aging(MSEBoxModel *bm, int species, int cohort, int *active_den, double *lostden_zero, double *this_p_ageup, int do_debug, FILE *llogfp);
void Recruit_Migration(MSEBoxModel *bm, int species, int wclayer, int stock_id, int do_debug, FILE *llogfp);
double Larval_Mortality(MSEBoxModel *bm, int species, int stock_id, FILE *llogfp);

int Ecology_Sp_Active(MSEBoxModel *bm, int species, FILE *llogfp);

/*********************************************************************************************************************************************************
 * Age structured (vertebrate) Reproduction
 */

/**
 *
 *	Note that spawning and recruitment are envisaged as working like:
 *	1. Is date = to date spawn/mate? (Time_Spawn)
 *	2  Young spend Recruit_Time days in gestation + weaning or as larvae in the plankton
 *	3  After day (Time_Spawn + Recruit_Time) then for the next Recruit_Period days
 *	recruits come in with a density from vector KDENR (one entry for each day of
 *	the recruit period) - i.e. at the very least those represented explicitly
 *	are the young individuals with almsot adult behvaiour adult like behaviour
 *	(so born or settled etc etc no zygotes etc).
 *
 *	Note: that also added a little variation to this by adding random number
 *	to Time_Spawn (+/-14days) and Recruit_Time (+/- 5 days)
 *
 */
void Vertebrate_Reproduction(MSEBoxModel *bm, int wclayer, int maxdeep, int totaldeep, int nz, double *localWCTracers, FILE *llogfp) {
    int cohort, num_cohorts, species, flagSP, sp_active, i, need_update, flagmother, sp_ddepend_move, stock_id,
        maxstock_id, do_debug, need_recupdate, senesce_on, need_matupdate, qid, ngene,
        sp_numGeneTypes, basecohort, semelparous_migrants, semelparous_parents;
    int overall_checkday = (int) (floor(bm->dayt));
    int sp_checkday = 0;
    int done_something, recruits_arrive, cIndex;
    //double lostden = 0;
    double dennow, CHLa, KWSR_sp, KWRR_sp, enviro_scalar, this_prop_ageup,
            plankton, vertdistrib, starting_num, new_num, amt, sp_AgeClassSize;
    
    //printf("Created Vertebrate Reproduction arrays\n");

    if (verbose > 1)
        printf("Doing vertebrate reproduction\n");

    /******************** Initialisation ********************/
    for (species = 0; species < bm->K_num_tot_sp; species++) {
        if (FunctGroupArray[species].speciesParams[flag_id] == TRUE) {
            if (FunctGroupArray[species].isVertebrate == TRUE) {
                for (cohort = 0; cohort < FunctGroupArray[species].numCohortsXnumGenes; cohort++) {
                    for (i = 0; i < 3; i++)
                        shiftVERT[species][cohort][i] = 0.0;
                    shiftVERTON[species][cohort] = 0;

                }
            }
        }
    }

    /***** Find do_debug setting and do initial print if appropriate *****/
    if (bm->debug && bm->which_check < bm->K_num_tot_sp && ((FunctGroupArray[bm->which_check].isVertebrate == TRUE) && ((bm->debug == debug_general)
            || (bm->debug == debug_age) || (bm->debug == debug_spawn) || (bm->debug == debug_recruit)) && (bm->dayt >= bm->checkstart))) {
        do_debug = 1;
    } else {
        do_debug = 0;
    }
    
    /*******************************************************************************
     Get initial values for vertebrate vectors
     */
    Set_VERTinfo(bm, localWCTracers, llogfp);

    if (do_debug && (bm->debug != debug_recruit) && ((bm->current_box == bm->checkbox) || (bm->checkbox > bm->nbox))) {
        fprintf(llogfp, "day: %e, box: %d, layer %d\n", bm->dayt, bm->current_box, bm->current_layer);
        for (cohort = 0; cohort < FunctGroupArray[bm->which_check].numCohortsXnumGenes; cohort++) {
            fprintf(llogfp, "%s reprod vinfoSN%d %e vinfoRN%d %e vinfoDEN-%d %e\n", FunctGroupArray[bm->which_check].groupCode, cohort,
                    VERTinfo[bm->which_check][cohort][SN_id], cohort, VERTinfo[bm->which_check][cohort][RN_id], cohort,
                    VERTinfo[bm->which_check][cohort][DEN_id]);
        }
    }
    
    /*******************************************************************************
     Get plankton values in case have plankton based reproduction
     */
    CHLa = localWCTracers[Chl_a_i];

    /* Plankton in this sense represents the plankton groups that are really
     fish larvae or feed on by fish larvae */
    plankton = Ecology_Get_Plankton(bm, localWCTracers, llogfp);

    /**** Main loop through species ****/
    for (species = 0; species < bm->K_num_tot_sp; species++) {
        if ((FunctGroupArray[species].speciesParams[flag_id] == TRUE) && ((FunctGroupArray[species].isVertebrate == TRUE)
                || (FunctGroupArray[species].groupAgeType == AGE_STRUCTURED))) {
            
            // Reinitialise for this species
            Util_Init_1D_Int(active_den, bm->K_num_max_cohort * bm->K_num_max_genetypes, 0);
            
            if ((bm->which_check == species) && (((bm->current_box == bm->checkbox) || (bm->checkbox > bm->nbox)) && (bm->dayt >= bm->checkstart)))
                do_debug = 1;
            else
                do_debug = 0;
            
            //if ((FunctGroupArray[species].numSpawns > 1) || (FunctGroupArray[species].groupType == FISH_INVERT))
            //    do_debug = 1;
            
            //if (bm->which_check == species)
            //    do_debug = 1;

            //if ((species == 44) || (species == 64))
            //    do_debug = 1;
            
            /** Initialisation **/
            /* Define num_cohorts = FunctGroupArray[species].numCohorts so that when use it as index to call
             value from vector have already corrected for the fact index starts at 0 not 1.
             */
            num_cohorts = FunctGroupArray[species].numCohorts - 1;
            sp_numGeneTypes = (int) (FunctGroupArray[species].numGeneTypes);

            /* Parameter values */
            flagSP = (int) (FunctGroupArray[species].speciesParams[flag_id]);
            KWSR_sp = FunctGroupArray[species].speciesParams[KWSR_id];
            KWRR_sp = FunctGroupArray[species].speciesParams[KWRR_id];
            sp_AgeClassSize = (double)(FunctGroupArray[species].ageClassSize);
            flagmother = (int) (FunctGroupArray[species].speciesParams[flagmother_id]);
            maxstock_id = FunctGroupArray[species].numStocks;
            sp_ddepend_move = (int) (FunctGroupArray[species].speciesParams[ddepend_move_id]);

            sp_active = Ecology_Sp_Active(bm, species, llogfp);

            /* ID stock recruits to enter - assume stock fidelity (i.e. add to adult stock
             that spawned them */
            stock_id = bm->group_stock[species][bm->current_box][bm->current_layer];

            done_something = 0;
            need_update = 0;
            need_matupdate = 0;
            need_recupdate = 0;
            semelparous_migrants = 0;
            semelparous_parents = 0;
            
            /* If species in model and active then proceed */
            if (flagSP && sp_active) {
                EMBRYO[species].recruiting_now = 0;
                Ecology_Init_Spawn_Params(bm, species, maxstock_id, wclayer, llogfp);

                /** Spawning **/
                sp_checkday = EMBRYO[species].next_spawn_any_age;
                
                /**
                //if (do_debug && (bm->which_check == species))
                //if ((FunctGroupArray[species].groupType == MAMMAL) || (FunctGroupArray[species].groupType == FISH_INVERT))
                //if (do_debug) {
                 if (species == 52) {
                    fprintf(llogfp, "Time: %e overall_checkday: %d, spawn_check_day: %d, ageday: %d for sp %s sp_checkday: %d\n", bm->dayt, overall_checkday, sp_checkday, EMBRYO[species].next_age_any_age, FunctGroupArray[species].groupCode, sp_checkday);
                }
               **/
                
                if (overall_checkday == sp_checkday) {
                    
                    done_something = 1;
                    /* Initalise TotSpawn for new values from current time step - needs to be reset for every box-layer as is the total spawn for that box-layer and added to EMBRYO.Larvae[] */
                    for (ngene = 0; ngene < sp_numGeneTypes; ngene++) {
                        EMBRYO[species].TotSpawn[ngene] = 0.0;
                    }
                    if(bm->track_contaminants && bm->flag_contamMaternalTransfer){  // Reinitialise for new spawning
                        for (cIndex = 0; cIndex < bm->num_contaminants; cIndex++) {
                            FunctGroupArray[species].reprodContam[cIndex] = 0.0;
                            FunctGroupArray[species].reprodContamCount[cIndex] = 0.0;
                        }
                    }

                    /* Start evolution */
                    Find_Evolution_Stats(bm, species, llogfp, 0);
                    Find_Evolution_Shift(bm, species, llogfp);
                    Do_Evolution(bm, species, llogfp);
                    
                    /* External to model spawning */
                    Ecology_Do_External_Age_Structured_Spawning(bm, species, flagmother, do_debug, &semelparous_migrants, llogfp);

                    /* Internal to model spawning */
                    Ecology_Do_Internal_Age_Structured_Spawning(bm, species, flagmother, do_debug, &semelparous_parents, llogfp);

                    /* Determine number of embryoes created */
                    Ecology_Find_Embryoes(bm, species, stock_id, plankton, CHLa, do_debug, llogfp);

                    /* Apply an semelparous reproductive deaths */
                    Apply_Semelparous_Deaths(bm, species, do_debug, semelparous_migrants, semelparous_parents, llogfp);
                    
                    /* Now spawning is started turn the switch off (so any once off events not repeated) */
                    EMBRYO[species].readytospawn[stock_id] = 0;
                 } else {
                    for (cohort = ((FunctGroupArray[species].numCohortsXnumGenes) - 1); cohort >= 0; cohort--) {
                        EMBRYO[species].IndSpawn[cohort] = 0;
                    }
                }
                
                /* Reset spawning readiness - can't do it at the time due to multiple layer handling */
                if (overall_checkday == (sp_checkday + 1)) {
                    done_something = 1;
                    EMBRYO[species].readytospawn[stock_id] = 2; // Reset so ready for next spawning event (now this one is done)

                    /* Determine population spawning */
                    Determine_Stock_Prop(bm, species, maxstock_id, 1, &amt, llogfp);

                    /* Get base number of arrivals */
                    Get_Recruits(bm, species, stock_id, plankton, llogfp);

                    /* Check for environmental effects */
                    enviro_scalar = 1.0;
                    enviro_scalar = Get_Enviro_Recruit_Forcing(bm, species, do_debug, llogfp);

                    /* Get vertical position in the water column */
                    vertdistrib = Get_Vertical_Recruit_Position(species, wclayer, maxdeep, totaldeep);
                    
                    /* Check for final recruit distribution */
                    Find_Final_Recruit_Distribution(bm, species, enviro_scalar, vertdistrib, wclayer, stock_id, do_debug, llogfp);
                }
                
                /** Do record keeping and diagostic tracking **/
                if (overall_checkday == (sp_checkday + 2)) {
                    done_something = 1;
                    
                    qid = EMBRYO[species].next_larvae;
                    if(EMBRYO[species].SpawnRecruitOverlap && EMBRYO[species].CounterNotDone)
                        qid++;

                    /** Not used any more as was messing up the spawning queue with no real purpose **/
                    if (!bm->flag_sanity_check) {
                        for (ngene = 0; ngene < sp_numGeneTypes; ngene++) {
                            EMBRYO[species].Larvae[stock_id][ngene][qid] = totrecruit[species][stock_id][ngene];
                        }
                    }
                    /**/

                    if (recover_help[species][0] && (recover_help_set[species] <= bm->dayt - (recover_span * recover_subseq))) {
                        /* Reset the helper indicator so don't keep getting "help" */
                        recover_help[species][0] = 0;
                    }

                    if (bm->flag_sanity_check && (bm->current_box == bm->first_box) && (bm->current_layer == bm->top_layer)) {
                        Store_Recruitment_Diagnostics(bm, species, bm->store_aggregate_yoy, do_debug, llogfp);
                        Sanity_Check_Recruits(bm, species, llogfp);
                    }

                    /* Update spawning queue */
                    for (cohort = 0; cohort < (FunctGroupArray[species].numCohortsXnumGenes); cohort++) {
                        Update_Spawn_Queue(bm, species, cohort, maxstock_id, do_debug, llogfp);
                    }
                }
                
                /******* Aging and Settlement of new young ******/
                Util_Init_1D_Double(lostden_zero, bm->K_num_max_genetypes, 0.0);
                sp_checkday = EMBRYO[species].next_age_any_age;
                
                /* Find any recruits settling in this timestep */
                Get_Settlers(bm, species, wclayer, stock_id, &recruits_arrive, do_debug, llogfp);

                /*
                if (do_debug) {
                    fprintf(llogfp, "Time: %e box%d-%d %s has sp_checkday %d vs overall_checkday %d\n", bm->dayt, bm->current_box, bm->current_layer, FunctGroupArray[species].groupCode, sp_checkday, overall_checkday);
                }
                */
                
                /* Start aging */
                if (overall_checkday == sp_checkday) {
                    done_something = 1;
                    
                    Init_Aging_Arrays(bm, species, stock_id, maxstock_id, llogfp);
                    
                    /* Turn switch on to let groups age */
                    for (cohort = (int)floor((FunctGroupArray[species].numCohortsXnumGenes) - 1); cohort >= 0; cohort--) {
                        shiftVERTON[species][cohort]++;
                    }
                    
                    if (bm->flagsenesce && (sp_AgeClassSize >= 1.0)) {
                        /* If assuming senescence - not necessary if each age phase represents < 1 year */
                        senesce_on = 1;
                    } else {
                        /* The oldest age group doesn't age up. */
                        senesce_on = 0;
                    }
                    
                    /**
                    if (do_debug && (bm->which_check == species)) {
                        fprintf(llogfp, "Time: %e, %s senesce_on: %d\n", bm->dayt, FunctGroupArray[species].groupCode, senesce_on);
                    }
                    **/
                    
                    /* Iterate of cohorts and calculate number aging up - starting with evolutionary shifts */
                    Do_Phenotyic_Expression(bm, species, llogfp);
                    Do_Aging_Update_Trait(bm, species, llogfp);
                    
                    for (cohort = (int)floor((FunctGroupArray[species].numCohortsXnumGenes) - 1); cohort >= 0; cohort--) {
                        basecohort = (int)floor (cohort / sp_numGeneTypes);
                        
                        /**
                        if (do_debug && (bm->which_check == species)) {
                        //if ((FunctGroupArray[species].groupType == MAMMAL) || (FunctGroupArray[species].groupType == FISH_INVERT)) {
                            fprintf(llogfp, "Time: %e, %s-%d, overall_checkday: %d Age_Now: %d\n",
                                bm->dayt, FunctGroupArray[species].groupCode, cohort, overall_checkday, EMBRYO[species].Age_Now[cohort][EMBRYO[species].next_age]);
                        }
                        **/

                        if ((!senesce_on && (basecohort == num_cohorts))
                            || (overall_checkday != EMBRYO[species].Age_Now[cohort][EMBRYO[species].next_age])){
                            /* Skip to next cohort as oldest don't age away in this option or the cohort not aging now */
                            continue;
                        }
                        
                        dennow = Get_Numbers_Aging(bm, species, cohort, active_den, lostden_zero, &this_prop_ageup, do_debug, llogfp);
                        
                        /* Update numbers to age and local distribution */
                        Update_Aging_Numbers(bm, species, cohort, stock_id, dennow, this_prop_ageup, do_debug, llogfp);

                        /** Update Migration array **/
                        Update_Migration_Array(bm, species, cohort, maxstock_id, do_debug, llogfp);

                        /* Update the queue */
                        Update_Age_Queue(bm, species, cohort, do_debug, llogfp);

                    }
                    
                    /* Set flag to indicate need age distribution update */
                    need_update = 1;

                    /* Set flag to indicate need to update general spatial distribution and
                     age-structured information due to aging */
                    need_matupdate = 1;
                }
                
                /**
                //if (do_debug && (bm->which_check == species)) {
                //if ((FunctGroupArray[species].groupType == MAMMAL) || (FunctGroupArray[species].groupType == FISH_INVERT)) {
                //if ((species == 26) || (species == 35) || (species == 64)) {
                if (species == 52) {
                    fprintf(llogfp, "Time: %e, box%d-%d, %s overall_checkday: %d, recruit_now: %d\n", bm->dayt, bm->current_box, bm->current_layer,
                            FunctGroupArray[species].groupCode, overall_checkday, EMBRYO[species].recruiting_now);
                }
                **/

                /* If first day that recruits arriving calculate new age distribution
                 (anticipating all future arrivals so working with static figures not
                 moving target - not 100% correct probably but much easier to deal with) */
                if (EMBRYO[species].recruiting_now) {
                    done_something = 1;
                    
                    /**
                    if (do_debug && (bm->which_check == species)) {
                    //if ((FunctGroupArray[species].groupType == MAMMAL) || (FunctGroupArray[species].groupType == FISH_INVERT)) {
                        fprintf(llogfp, "Time: %e, box%d-%d, %s overall_checkday: %d, recruit_now: %d\n", bm->dayt, bm->current_box, bm->current_layer,
                                FunctGroupArray[species].groupCode, overall_checkday, EMBRYO[species].recruiting_now);
                    }
                    **/

                    Get_Latest_Additions(bm, wclayer, species, maxstock_id, stock_id, lostden_zero, need_update, do_debug, llogfp);
                    /* Check for final recruit distribution */
                    Recruit_Migration(bm, species, wclayer, stock_id, do_debug, llogfp);
                    /* Set flag to indicate need to update distributions due to recruitment */
                    need_recupdate = 1;

                    /* Set flag so know recruitment has been running sometime
                     so don't need to update age distribution */
                    if ((bm->current_box == bm->last_box) && (!bm->current_layer))
                        FunctGroupArray[species].speciesParams[firstrecday_id] = 0;
                    
                }
                
                /* Update flags */
                FunctGroupArray[species].speciesParams[need_update_id] = need_update;
                FunctGroupArray[species].speciesParams[need_recupdate_id] = need_recupdate;
                FunctGroupArray[species].speciesParams[need_matupdate_id] = need_matupdate;

                cohort = 0;
                /* Update sizes of smallest age class - calculate new sn and rn */
                
                if (bm->flag_modify_KWSR) {
                    KWSR_sp = Get_Recruit_Size_Forcing(bm, species, do_debug, llogfp);
                    KWRR_sp = KWSR_sp * FunctGroupArray[species].X_RS[cohort];
                }
                
                FunctGroupArray[species].speciesParams[suckler_contam_today_id] = 1;  // Only zeroed if recruits actively arriving
                if (recruits_arrive) {
                    /* Update sizes of smallest age class - calculate new sn and rn */
                    for(ngene = 0; ngene < sp_numGeneTypes; ngene++){
                        cohort = ngene;  // As all in youngest cohort here
                        new_num = EMBRYO[species].recruitSPden[ngene];
                        starting_num = VERTinfo[species][cohort][DEN_id] - lostden_zero[ngene]; // lostden is due to aging up
                        new_num += starting_num;
                        shiftVERT[species][cohort][SN_id] = (KWSR_sp * EMBRYO[species].recruitSPden[ngene] + VERTinfo[species][cohort][SN_id] * starting_num) / (new_num + small_num);
                        shiftVERT[species][cohort][RN_id] = (KWRR_sp * EMBRYO[species].recruitSPden[ngene] + (VERTinfo[species][cohort][RN_id] - EMBRYO[species].IndSpawn[cohort]) * starting_num) / (new_num + small_num);
                                                
                        if(bm->track_contaminants && bm->flag_contamMaternalTransfer){
                            Apply_Settler_Contaminants(bm, species, cohort, starting_num, new_num, EMBRYO[species].next_recruit);
                            FunctGroupArray[species].speciesParams[suckler_contam_today_id] = 0; // As getting transfer at birth so don't double up
                        }

                        // Also check if any den active for puroposes of updating the local pools
                        if((EMBRYO[species].recruitSPden[ngene] > 0) || (lostden_zero[ngene] != 0.0)){
                            active_den[cohort] = 1;
                        }
                        
                        /**
                        //if (do_debug && (bm->which_check == species) && (EMBRYO[species].recruitSPden[ngene] > 0) ) {
                            //fprintf(bm->logFile, "VERTinfo[species][cohort][DEN_id] = %e\n", VERTinfo[species][cohort][DEN_id]);
                        
                        if (do_debug && (EMBRYO[species].recruitSPden[ngene] > 0)) {
                        //if (EMBRYO[species].recruitSPden[ngene] > 0) {
                            fprintf(llogfp, "Time: %e, box%d %s ngene: %d shiftVERT-SN: %e, KWSR_sp: %e, recruitSPden: %e, VERTinfo-SN: %e, starting_num: %e, new_num: %e (VERTden: %e, lostdenzero: %e)\n", bm->dayt, bm->current_box, FunctGroupArray[species].groupCode, ngene, shiftVERT[species][cohort][SN_id], KWSR_sp, EMBRYO[species].recruitSPden[ngene], VERTinfo[species][cohort][SN_id], starting_num, new_num, VERTinfo[species][cohort][DEN_id], lostden_zero[ngene]);
                        }
                        **/
                    }
                }
                if(active_den[cohort])
                    done_something = 1;

                /**
                // Debugging check of shifts and recruitment
                // && (species == bm->which_check) && (bm->current_box == bm->checkbox || bm->checkbox > bm->nbox))
                if ((need_update || need_recupdate) && do_debug ) {
                    for (i = 0; i < (FunctGroupArray[species].numCohortsXnumGenes); i++) {
                        basecohort = (int)(floor (i / sp_numGeneTypes));
                        ngene = i - (basecohort * sp_numGeneTypes);
 
                        if (shiftVERT[species][i][DEN_id] > 0) {
                            fprintf(llogfp, "day: %e, %s-%d box: %d, layer %d, shiftDEN-%d %e\n", bm->dayt,
                                FunctGroupArray[species].groupCode, i, bm->current_box, wclayer, i, shiftVERT[species][i][DEN_id]);
                        }
                        if (EMBRYO[species].recruitSPden[ngene] > 0) {
                            fprintf(llogfp, "%s-%d recruitSPden: %e\n", FunctGroupArray[species].groupCode, ngene, EMBRYO[species].recruitSPden[ngene]);
                        }
                    }
                }
                **/
                
                /* Update local dPools */
                if(done_something){
                    for (i = 0; i < (FunctGroupArray[species].numCohortsXnumGenes); i++) {
                        Update_Local_dPool(bm, species, i, EMBRYO[species].IndSpawn[i], EMBRYO[species].recruitSPden, sp_numGeneTypes, localWCTracers, do_debug, llogfp);
                    }
                }
                
                /* Update age distributions - only do at end for typical group, but do it every box for site attached */
                need_update = (int) (FunctGroupArray[species].speciesParams[need_update_id]);
                if (bm->flagagestruct && need_update && (((bm->current_box == bm->last_box) && (!bm->current_layer))
                        || ((sp_ddepend_move == 1) && !bm->current_layer))) {
                    Update_Age_Distrib(bm, species, stock_id, sp_ddepend_move, 0, llogfp);  // 0 flag to indicate general aging
                }

                /* If no general aging but only new recruits still do update but of more limited extent (if general
                 aging occuring here too then skip this step as update already done */
                need_recupdate = (int) (FunctGroupArray[species].speciesParams[need_recupdate_id]);
                if (bm->flagagestruct && (need_update > -1) && need_recupdate && (((bm->current_box == bm->last_box) && !bm->current_layer)
                        || ((sp_ddepend_move == 1) && !bm->current_layer))) {

                    /*
                    //if (do_debug && (bm->which_check == species)) {
                    //if ((FunctGroupArray[species].groupType == MAMMAL) || (FunctGroupArray[species].groupType == FISH_INVERT)) {
                        fprintf(llogfp,"Time: %e, box: %d-%d flagagestruct: %d, need_update: %d, need_recupdate: %d, sp_ddepend_move: %d\n",
                            bm->dayt, bm->current_box, bm->current_layer, bm->flagagestruct, need_update, need_recupdate, sp_ddepend_move);
                    }
                    */

                    Update_Age_Distrib(bm, species, stock_id, sp_ddepend_move, 1, llogfp);  // 1 flag to indicate recreuitment related
                }
            }
        }
    }
    
    return;
}

/*****************************************************************************************************************
 * Reproduction helper routines
 */

/**
 *	\brief Vertebrate reproductive subroutines - find spawn for age structured groups
 *
 *	@returns The amount (N) spawned by an individual
 */
double Ecology_Age_Structured_Spawn(int species, double KSPA_SP, double FSP_SP, double RSprop_SP, double SN, double RN, double FSPB_SP, double X_RS, int repcost, FILE *llogfp) {
	double WforS, Wgt, Spwn, Wgtcheck, RepCost;
    double RN_buf = 0.999999999;
    
    if(repcost) {
        // NOTE: parameters are overloaded - FSP is constant for reproduction cost and KSPA is the exponent here
        RepCost = FSP_SP * pow(SN, KSPA_SP);
        Spwn = RSprop_SP * RN - RepCost;
        if (Spwn < 0.0)
            Spwn = 0.0;
    } else {
        WforS = (1.0 + X_RS) * SN;
        Wgt = SN + RN;
        Spwn = FSP_SP * WforS - KSPA_SP;
        Wgtcheck = FSPB_SP * RN;
        if (Wgt < WforS)
            Spwn = Spwn - (WforS - Wgt);
        if (Spwn < 0.0)
            Spwn = 0.0;

        if (Wgtcheck > RN) {
            /* This was adopted from ERSEM way back when, it should never actually happen
             as FSPB_SP <= 1 (or should be at any rate!!!!) */
            if (FSPB_SP != 0)
                Spwn = RN / FSPB_SP;
            else
                Spwn = RN;
        }
	}
    
    // Sanity check
    if (Spwn > RN)
        Spwn = RN * RN_buf;

    //fprintf(llogfp, "%s repcost %d SN: %e RN: %e KSPA_SP %e FSP_SP: %e RSprop_SP: %e FSPB_SP: %e X_RS: %e Spwn: %e\n", FunctGroupArray[species].groupCode, repcost, SN, RN, KSPA_SP, FSP_SP, RSprop_SP, FSPB_SP, X_RS, Spwn);
    
    return (Spwn);
}

/**
 *
 *	\brief Plankton in this sense represents the plankton groups that are really
 *  fish larvae or feed on by fish larvae
 *
 */
double Ecology_Get_Plankton(MSEBoxModel *bm, double *localWCTracers, FILE *llogfp) {
	double plankton = 0.0;
	int species;

	for (species = 0; species < bm->K_num_tot_sp; species++) {
		if (FunctGroupArray[species].speciesParams[flag_id] == TRUE) {
			/* Calculate the plankton value - only if an active group*/
			if (FunctGroupArray[species].groupType == LG_PHY || FunctGroupArray[species].groupType == LG_ZOO || FunctGroupArray[species].groupType == MED_ZOO
					|| FunctGroupArray[species].groupType == SM_ZOO) {
				plankton += localWCTracers[FunctGroupArray[species].totNTracers[0]];
			}
		}
	}

	return plankton;
}

/**
 *
 *  \brief Finding sp_active to see if spawning
 *
 */
int Ecology_Sp_Active(MSEBoxModel *bm, int species, FILE *llogfp) {
	int sp_active = (int) (FunctGroupArray[species].speciesParams[flagactive_id]);

	/* If species has no temporal preference then spawn and age update in day */
	if (sp_active == 2) {
		if (bm->flagday)
			sp_active = 1;
		else
			sp_active = 0;
	} else {
		/* Base it on when active */
		if(FunctGroupArray[species].groupAgeType == AGE_STRUCTURED) {
			if ((sp_active == bm->flagday) || (bm->dt >= 86400.0)) {
				sp_active = 1; // Or spawn when active
			} else {
				sp_active = 0;
			}
		} else if(FunctGroupArray[species].groupAgeType == AGE_STRUCTURED_BIOMASS) {
			sp_active = bm->flagday;
		}	
	}
	return sp_active;
}

/**
 *
 * \brief Initialising spawning related arrays
 *
 */
void Ecology_Init_Spawn_Params(MSEBoxModel *bm, int species, int maxstock_id, int wclayer, FILE *llogfp) {
	int i = 0, ngene = 0, chrt = 0;
	int basecohort;
	int sp_numGeneTypes = (int) (FunctGroupArray[species].numGeneTypes);
	int overall_checkday = (int) (floor(bm->dayt));
    
    Util_Init_1D_Int(ngene_done, bm->K_num_max_genetypes, 0);
    Util_Init_1D_Int(not_finished, bm->K_num_max_genetypes, 0);

    //int do_debug = 0;

	//if((bm->current_box == bm->checkbox) && (bm->dayt > bm->checkstart) && (bm->which_check == species))
	//	do_debug = 1;

	//if((bm->dayt >= bm->checkstart) && (bm->which_check == species))
	//	do_debug = 1;
    
    //if ((FunctGroupArray[species].numSpawns > 1) || (FunctGroupArray[species].groupType == FISH_INVERT))
    //    do_debug = 1;

    //if (bm->which_check == species)
    //    do_debug = 1;
    
    //printf("Creating Init Spawn arrays\n");

	/* If no embryos there can be no recruits */
    for(chrt = 0; chrt < FunctGroupArray[species].numCohortsXnumGenes; chrt++) {
        shiftVERTON[species][chrt] = 0;
    }

	if ((bm->current_box == bm->last_box) && (!bm->current_layer)) {
		for(chrt = 0; chrt < FunctGroupArray[species].numCohortsXnumGenes; chrt++) {
            
			basecohort = (int)(floor (chrt / sp_numGeneTypes));
			ngene = chrt - (basecohort * sp_numGeneTypes);
            
            /**
            if (do_debug && (species == bm->which_check)) {
                fprintf(llogfp, "Time: %e %s-%d has overall_checkday: %d StartDay: %d EndDay: %d next_recruit: %d Larvae: %e\n", bm->dayt, FunctGroupArray[species].groupCode, chrt, overall_checkday, EMBRYO[species].StartDay[chrt][EMBRYO[species].next_recruit], EMBRYO[species].EndDay[chrt][EMBRYO[species].next_recruit], EMBRYO[species].next_recruit, EMBRYO[species].Larvae[0][0][EMBRYO[species].next_recruit]);
            }
            **/
            
			if(( overall_checkday > EMBRYO[species].StartDay[chrt][EMBRYO[species].next_recruit] ) && ( overall_checkday <= EMBRYO[species].EndDay[chrt][EMBRYO[species].next_recruit])) {
				if(ngene_done[ngene] < 1) {
					not_finished[ngene] = 0;
					for (i = 0; i < maxstock_id; i++) {
                        
                        /**
                        //if (do_debug && (species == bm->which_check)) {
                        if (species == 52) {
                            fprintf(llogfp, "Time: %e %s-%d ngene: %d stock %d Larvae: %e qid: %d\n", bm->dayt, FunctGroupArray[species].groupCode, chrt, ngene, i, EMBRYO[species].Larvae[i][ngene][EMBRYO[species].next_recruit], EMBRYO[species].next_recruit);
                        }
                        **/
						if (EMBRYO[species].Larvae[i][ngene][EMBRYO[species].next_recruit] > 0.0) {
							not_finished[ngene] += 1;
						}
					}
					ngene_done[ngene] = 1;  // Step 1 done
				}

				/* If settlers all done (i.e. larvae = 0.0 so not_finished = 0) shuffle everything along and reset */
				if (not_finished[ngene] < 1) {
					/* If all settlers arrived then need to reset */
					EMBRYO[species].EndDay[chrt][EMBRYO[species].next_recruit] = overall_checkday;
                    
                    /**
                    //if (do_debug && (species == bm->which_check)) {
                    if (species == 52) {
                        fprintf(llogfp, "Time: %e %s-%d end day RESET to: %d\n", bm->dayt, FunctGroupArray[species].groupCode, chrt, EMBRYO[species].EndDay[chrt][EMBRYO[species].next_recruit]);
                    }
                    **/
				}

				if (overall_checkday == EMBRYO[species].EndDay[chrt][EMBRYO[species].next_recruit]) {
					 /* Note this assumes all age classes for genetype ngene are reset at this time
					  * TODO: May want to relax this assumption into the future
					  */
					//EMBRYO[species].num_recruits[bm->current_box][wclayer][ngene][EMBRYO[species].next_recruit] = 0.0;  // Do not need to zero anymore as move on past it
					EMBRYO[species].recruitSPden[ngene] = 0.0;
				}
			}

            if ((overall_checkday == (EMBRYO[species].EndDay[chrt][EMBRYO[species].next_recruit] + 1)) && (!chrt)){ // Only bother doing this once per gene - so do it for chrt 0

                EMBRYO[species].next_recruit++;
				if(ngene_done[ngene] < 2){
					/* Zero last entry in the queue */
					EMBRYO[species].next_larvae++;
                    EMBRYO[species].CounterNotDone = 0;  // Set to zero here for cases where spawn about the time of teh end of recruitment so don't want to be +1 on slot ID

                    /**
					//if (do_debug && (species == bm->which_check)) {
                    if (species == 0) {
                        fprintf(llogfp, "Time: %e, %s has -- next_larvae now: %d and next_recruit: %d and CounterNotDone reset to 0\n",
							bm->dayt, FunctGroupArray[species].groupCode, EMBRYO[species].next_larvae, EMBRYO[species].next_recruit);
					}
                    **/
                    
					ngene_done[ngene] = 2; // Step 2 done
				}
			}
		}
	}

	/* Reset larval values - Only bother doing this once per gene, so do it for chrt 0 */
	if ((bm->current_box == bm->first_box) && (bm->current_layer == bm->top_layer)) {
        if (overall_checkday == (EMBRYO[species].next_spawn_any_age - 2)) {
            for (i = 0; i < maxstock_id; i++) {
                if (EMBRYO[species].readytospawn[i] > 1)
                    EMBRYO[species].readytospawn[i] = 1;
            }
            // Re-initialise SSB
            bm->tot_SSB[species] = 0.0;
            // Reinitialise CounterNotDone in case need to spawn mid recruitment
            EMBRYO[species].CounterNotDone = 1;
            
            //fprintf(llogfp, "Time: %e, %s has -- CounterNotDone reset to 1\n", bm->dayt, FunctGroupArray[species].groupCode);

		}
	}

    /*
    if (do_debug) {
        fprintf(llogfp, "Ending Init Spawns\n");
    }
    */
    
	return;
}

/**
 *
 * \brief Routine to update the aging queue for age structured groups
 *
 * TODO: Eventually generalise these queue updates (aging and spawning effectively identical)
 *
 */
void Update_Age_Queue(MSEBoxModel *bm, int species, int cohort, int do_debug, FILE *llogfp) {
    int chrt, day_to_check;
    
	if ((bm->current_box == bm->last_box) && (!bm->current_layer) && !cohort){
        /* Shuffle Aging Queue to next entry - only do once per species and as counter counting down do it for the youngest (as last) */
        EMBRYO[species].next_age++;

        day_to_check = MAXINT;
        for(chrt = 0; chrt < FunctGroupArray[species].numCohortsXnumGenes; chrt++) {
            
            if ((EMBRYO[species].Age_Now[chrt][EMBRYO[species].next_age] > bm->dayt) && (day_to_check > EMBRYO[species].Age_Now[chrt][EMBRYO[species].next_age]))
                day_to_check = EMBRYO[species].Age_Now[chrt][EMBRYO[species].next_age];
        }
        EMBRYO[species].next_age_any_age = day_to_check;
        
        /**
        //if (bm->which_check == species) {
            fprintf(llogfp, "Time: %e, %s-%d has next_age_id: %d next_age date: %d\n", bm->dayt,
                    FunctGroupArray[species].groupCode, cohort, EMBRYO[species].next_age, EMBRYO[species].Age_Now[cohort][EMBRYO[species].next_age]);
        }
        **/
	}

}

/**
 *
 * \brief Routine to update the soawning queue for age structured groups
 *
 */
void Update_Spawn_Queue(MSEBoxModel *bm, int species, int cohort, int maxstock_id, int do_debug, FILE *llogfp) {
    int i, ngene;
    int chrt, day_to_check;

	if ((bm->current_box == bm->last_box) && (!bm->current_layer) && (cohort == (FunctGroupArray[species].numCohortsXnumGenes - 1))){
        EMBRYO[species].next_spawn++;
        EMBRYO[species].CounterNotDone = 1;
        
        day_to_check = MAXINT;
        for(chrt = 0; chrt < FunctGroupArray[species].numCohortsXnumGenes; chrt++) {
            if ((EMBRYO[species].Spawn_Now[chrt][EMBRYO[species].next_spawn] > bm->dayt) && (day_to_check > EMBRYO[species].Spawn_Now[chrt][EMBRYO[species].next_spawn]))
                day_to_check = EMBRYO[species].Spawn_Now[chrt][EMBRYO[species].next_spawn];
        }
        EMBRYO[species].next_spawn_any_age = day_to_check;
        
        /* Zero statistic storing */
		for (ngene = 0; ngene < FunctGroupArray[species].numGeneTypes; ngene++){
			for (i = 0; i < maxstock_id; i++) {
				totrecruit[species][i][ngene] = 0.0;
			}
		}
        
        /*
        if (do_debug && (bm->which_check == species)){
            fprintf(llogfp, "Time: %e, %s-%d has next_spawn: %d\n", bm->dayt, FunctGroupArray[species].groupCode, cohort, EMBRYO[species].next_spawn);
		}
        */
	}

}


/**
 *
 * \brief External spawning (occurring by adults outside the model domain)
 *
 */
void Ecology_Do_External_Age_Structured_Spawning(MSEBoxModel *bm, int species, int flagmother, int do_debug, int *semelparous_migrants, FILE *llogfp) {
	double KSPA_sp = FunctGroupArray[species].speciesParams[KSPA_id];
	double FSP_sp = FunctGroupArray[species].speciesParams[FSP_id];
	double Wgt = 0.0;
	double li = 0.0;
	double MigIndSpawn = 0.0;
	double sp_spawn_start = 0.0;
	double sp_spawn_end = 0.0;
	double step1 = 0.0;
    double thisSSB = 0.0;
    double RSprop_sp = 0.0;
	int sp_numGeneTypes = (int) (FunctGroupArray[species].numGeneTypes);
	//int num_cohorts = FunctGroupArray[species].numCohorts - 1;
	int allowed_to_spawn = 0, ngene = 0, recieve_ngene = 0, mid;
	int cohort = 0;
	int overall_checkday = (int)(floor(bm->dayt));
	int basecohort = 0;
    int flag_allow_spawn = 1;
    *semelparous_migrants = 0;
    
    if(verbose)
        printf("Ecology_Do_External_Age_Structured_Spawning for %s\n", FunctGroupArray[species].groupCode);
    
    //fprintf(llogfp, "Time: %e, %s-%d box%d-%d Spawn_Now: %d (next_spawn: %d) overall_checkday: %d recruitType: %d\n", bm->dayt, FunctGroupArray[species].groupCode, cohort, bm->current_box, bm->current_layer, EMBRYO[species].Spawn_Now[cohort][EMBRYO[species].next_spawn], EMBRYO[species].next_spawn, overall_checkday, FunctGroupArray[sp].recruitType);
    
    /* If spawn outside model and in first dynamic box then add in spawning due to migrants */
	if ((FunctGroupArray[species].externalReproducer) && ((bm->current_box == bm->first_box) && (bm->current_layer == bm->top_layer))) {
		for (cohort = ((FunctGroupArray[species].numCohortsXnumGenes) - 1); cohort >= 0; cohort--) {
			basecohort = (int)(floor (cohort / sp_numGeneTypes));
			ngene = cohort - (basecohort * sp_numGeneTypes);
			recieve_ngene = ngene;

            //fprintf(llogfp, "Time: %e, %s-%d Spawn_Now: %d (next_spawn: %d) overall_checkday: %d\n", bm->dayt, FunctGroupArray[species].groupCode, cohort, EMBRYO[species].Spawn_Now[cohort][EMBRYO[species].next_spawn], EMBRYO[species].next_spawn, overall_checkday);
            
            if(EMBRYO[species].Spawn_Now[cohort][EMBRYO[species].next_spawn] != overall_checkday)
				continue;   // As not the cohort spawning

			allowed_to_spawn = 1;

			sp_spawn_start = EMBRYO[species].Spawn_Now[cohort][EMBRYO[species].next_spawn];
			sp_spawn_end = EMBRYO[species].Spawn_Now[cohort][EMBRYO[species].next_spawn] + FunctGroupArray[species].speciesParams[spawn_period_id];

			if ((overall_checkday >= sp_spawn_start) && (overall_checkday <= sp_spawn_end)) {
				allowed_to_spawn = 1;
			} else {
				allowed_to_spawn = 0;
			}
            
            //fprintf(llogfp, "Time: %e, %s-%d allowed_to_spawn A: %d\n", bm->dayt, FunctGroupArray[species].groupCode, cohort, allowed_to_spawn);

			if (!allowed_to_spawn) // If not big enough to spawn skip ahead
				continue;

            // Get dynamically evolving trait values if RSprop is an evolving trait
            if (bm->flag_do_evolution && DNA[species].trait_active[rsprop_trait_id]) {
                RSprop_sp = bm->evolTraitValues[species][cohort][rsprop_trait_id]; //NOTE that here cohort refers to the genotype
            } else {
                RSprop_sp = FunctGroupArray[species].speciesParams[RSprop_id]; // else just use the stable RSprop value (if there is one)
            }

            for (mid = MIGRATION[species].num_in_queue_done; mid < MIGRATION[species].num_in_queue; mid++) {
                flag_allow_spawn = MIGRATION[species].ReprodAllowed[cohort][mid];
                if (flag_allow_spawn && (MIGRATION[species].DEN[cohort][mid] > bm->min_dens)) {
                    
                    /* for those species with size-based age of maturity, check the age class is big enough to spawn */
                    Wgt = MIGRATION[species].SN[cohort][mid] + MIGRATION[species].RN[cohort][mid];
                    li = Ecology_Get_Size(bm, species, Wgt, cohort);
                    if (li < FunctGroupArray[species].speciesParams[min_li_mat_id]) {
                        allowed_to_spawn = 0;
                    }
            
                    //fprintf(llogfp, "Time: %e, %s cohort %d allowed_to_spawn B: %d\n", bm->dayt, FunctGroupArray[species].groupCode, cohort,  allowed_to_spawn);

                    if (!allowed_to_spawn) // If not big enough to spawn skip ahead
                        continue;
            
                    step1 = Ecology_Age_Structured_Spawn(species, KSPA_sp, FSP_sp, RSprop_sp, MIGRATION[species].SN[cohort][mid], MIGRATION[species].RN[cohort][mid], FunctGroupArray[species].scaled_FSPB[cohort], FunctGroupArray[species].X_RS[cohort], bm->flag_repcostSpawn, llogfp);
                    MigIndSpawn = FunctGroupArray[species].scaled_FSPB[cohort] * step1;
            
                    //fprintf(llogfp, "Time: %e, %s cohort %d mid: %d MigIndSpawn: %e FSPB: %e step1: %e\n", bm->dayt, FunctGroupArray[species].groupCode, cohort, mid, MigIndSpawn, FunctGroupArray[species].scaled_FSPB[cohort], step1);
            
                    thisSSB = (MIGRATION[species].RN[cohort][mid] + MIGRATION[species].SN[cohort][mid]) * MIGRATION[species].DEN[cohort][mid];
                        bm->tot_SSB[species] += FunctGroupArray[species].scaled_FSPB[cohort] * thisSSB;

                    if (bm->flag_do_evolution)
                        recieve_ngene = Do_Inheritance(bm, species, ngene, basecohort, cohort, llogfp); //# TODO: This may need to pass cohort rather cohort
                    EMBRYO[species].TotSpawn[recieve_ngene] += MigIndSpawn * MIGRATION[species].DEN[cohort][mid];
                    MIGRATION[species].RN[cohort][mid] -= MigIndSpawn; // Mass lost in spawning

                    
                    if(bm->track_contaminants && bm->flag_contamMaternalTransfer){
                        quit("Have not coded contaminant transfer for externally reproducing species - if you get this message contact teh developer group and ask them to sort this for you.... if it involves salmon Beth may swear, don't worry she will be fine after a bowl of ice cream\n");
                    }

                    /* If semelparous reproduction */
                    if (flagmother < 0) {
                        *semelparous_migrants = 1;
                    }

                    if (isnan(MIGRATION[species].RN[cohort][mid])) {
                        quit("Time %e - during external spawning of %s cohort %d mid: %d have got NAN RN: %e SN: %e MigIndSpawn: %e scaled_FSPB: %e step1: %e DEN: %e\n", bm->dayt, FunctGroupArray[species].groupCode, cohort, mid, MIGRATION[species].RN[cohort][mid], MIGRATION[species].SN[cohort][mid], MigIndSpawn, FunctGroupArray[species].scaled_FSPB[cohort], step1, MIGRATION[species].DEN[cohort][mid]);
                    }
                    
                    /**
                    //if (do_debug) {
                    if (species == 33) {
                        fprintf(llogfp, "Time: %e, %s cohort %d external spawned %e DEN: %e RN: %e semelparous_migrants: %d  TotSpawn: %e\n", bm->dayt, FunctGroupArray[species].groupCode, cohort, (MigIndSpawn * MIGRATION[species].DEN[cohort][mid]), MIGRATION[species].DEN[cohort][mid], MIGRATION[species].RN[cohort][mid], *semelparous_migrants,  EMBRYO[species].TotSpawn[recieve_ngene]);
                    }
                    **/
                }
            }
		}

        /**
		if (do_debug && (bm->which_check == species)) {
        //if (do_debug ) {
			fprintf(llogfp, "Time: %e, %s spawned (%e) - double check\n", bm->dayt, FunctGroupArray[species].groupCode, EMBRYO[species].TotSpawn[ngene]);
		}
        **/
	}

	return;
}

/**
 *
 * \brief External spawning (occurring by adults outside the model domain)
 *
 */
void Ecology_Do_Internal_Age_Structured_Spawning(MSEBoxModel *bm, int species, int flagmother, int do_debug, int *semelparous_parents, FILE *llogfp) {
	double KSPA_sp = FunctGroupArray[species].speciesParams[KSPA_id];
	double FSP_sp = FunctGroupArray[species].speciesParams[FSP_id];
	double Wgt = 0.0, li;
	double step1 = 0.0;
    double thisSSB = 0.0;
    double RSprop_sp = 0.0;
	int sp_numGeneTypes = (int) (FunctGroupArray[species].numGeneTypes);
	int allowed_to_spawn = 0, ngene = 0, recieve_ngene = 0;
	//int num_cohorts = FunctGroupArray[species].numCohorts - 1;
	int cohort;
	int basecohort = 0;
	int overall_checkday = (int)(floor(bm->dayt));
    *semelparous_parents = 0;

	for (cohort = ((FunctGroupArray[species].numCohortsXnumGenes) - 1); cohort >= 0; cohort--) {
		basecohort = (int)floor (cohort / sp_numGeneTypes);
		ngene = cohort - (basecohort * sp_numGeneTypes);
		recieve_ngene = ngene;

		if(EMBRYO[species].Spawn_Now[cohort][EMBRYO[species].next_spawn] != overall_checkday)
			continue;   // As not the cohort spawning

		allowed_to_spawn = 1;

		/* for those species with size-based age of maturity, check teh age class is big enough to spawn */
		Wgt = VERTinfo[species][cohort][SN_id] + VERTinfo[species][cohort][RN_id];
		li = Ecology_Get_Size(bm, species, Wgt, cohort);
		if (li < FunctGroupArray[species].speciesParams[min_li_mat_id]) {
			allowed_to_spawn = 0;
		}

		if (!allowed_to_spawn) // If not big enough to spawn skip ahead
			continue;

        // Get dynamically evolving trait values if RSprop is an evolving trait
        if (bm->flag_do_evolution && DNA[species].trait_active[rsprop_trait_id]) {
            RSprop_sp = bm->evolTraitValues[species][cohort][rsprop_trait_id]; //NOTE that here cohort refers to the genotype
        } else {
            RSprop_sp = FunctGroupArray[species].speciesParams[RSprop_id]; // else just use the stable RSprop value (if there is one)
        }

        /* Animals in this box spawn */
		step1 = Ecology_Age_Structured_Spawn(species, KSPA_sp, FSP_sp, RSprop_sp, VERTinfo[species][cohort][SN_id], VERTinfo[species][cohort][RN_id], FunctGroupArray[species].scaled_FSPB[cohort], FunctGroupArray[species].X_RS[cohort], bm->flag_repcostSpawn, llogfp);
		EMBRYO[species].IndSpawn[cohort] = FunctGroupArray[species].scaled_FSPB[cohort] * step1;
        
        thisSSB = (VERTinfo[species][cohort][RN_id] + VERTinfo[species][cohort][SN_id]) * VERTinfo[species][cohort][DEN_id];
        bm->tot_SSB[species] += FunctGroupArray[species].scaled_FSPB[cohort] * thisSSB;

        if(bm->flag_do_evolution) {
            recieve_ngene = Do_Inheritance(bm, species, ngene, basecohort, cohort, llogfp);
        }
        
        if(bm->track_contaminants && bm->flag_contamMaternalTransfer){
            if(EMBRYO[species].IndSpawn[cohort] > 0) {  // Only transfer contaminants if actually producing any spawn
                Get_Parental_Contaminants(bm, species, cohort, flagmother, VERTinfo[species][cohort][DEN_id]);
            }
        }
            
		EMBRYO[species].TotSpawn[recieve_ngene] += EMBRYO[species].IndSpawn[cohort] * VERTinfo[species][cohort][DEN_id];

        /**
		//if (do_debug && (bm->which_check == species)) {
        //if (do_debug ) {
        if (species == 33) {
			fprintf(llogfp, "Time: %e, %s cohort %d, internal_Spawning Box %d, layer %d, TotSpawn: %e, IndSpawn[%d]: %e, DEN: %e, FSPB: %e, SN: %e, RN: %e, FSP: %e, KSPA: %e step1: %e\n", bm->dayt,
					FunctGroupArray[species].groupCode,cohort, bm->current_box, bm->current_layer, EMBRYO[species].TotSpawn[recieve_ngene], cohort, EMBRYO[species].IndSpawn[cohort], VERTinfo[species][cohort][DEN_id],
					FunctGroupArray[species].scaled_FSPB[cohort], VERTinfo[species][cohort][SN_id], VERTinfo[species][cohort][RN_id], FSP_sp, KSPA_sp, step1);
		}
        **/
        
		/* If semelparous reproduction */
		if (flagmother < 0) {
            *semelparous_parents = 1;
		}
	}

	return;
}


/**
 * \brief Update the MIGRATION and VERT and shiftVERT arrays for semelparous deaths - needs to be here due to the way invertbrate reproduction happens
 *
 */
void Apply_Semelparous_Deaths(MSEBoxModel *bm, int species, int do_debug, int semelparous_migrants, int semelparous_parents, FILE *llogfp) {
    int cohort, qid;
    
    for (cohort = ((FunctGroupArray[species].numCohortsXnumGenes) - 1); cohort >= 0; cohort--) {
        
        if(FunctGroupArray[species].scaled_FSPB[cohort] > 0.0) {

            /* If semelparous external reproduction */
            if (semelparous_migrants) {
                for (qid = MIGRATION[species].num_in_queue_done; qid < MIGRATION[species].num_in_queue; qid++) {
                    fprintf(llogfp,"Time: %e %s-%d qid: %d has starting MIGRATION_den_expect: %e", bm->dayt, FunctGroupArray[species].groupCode, cohort, qid,  MIGRATION[species].DEN[cohort][qid]);
                
                    MIGRATION[species].DEN[cohort][qid] *= (1.0 - FunctGroupArray[species].scaled_FSPB[cohort]);
                
                    fprintf(llogfp," ending: %e as scaled_FSPB: %e\n", MIGRATION[species].DEN[cohort][qid], FunctGroupArray[species].scaled_FSPB[cohort]);
                }
            }

    
            /* If semelparous reproduction internal to the model domain */
            if (semelparous_parents) {
                fprintf(llogfp,"Time: %e %s-%d current_box: %d has DEN: %e and shiftDEN: %e ", bm->dayt, FunctGroupArray[species].groupCode, cohort, bm->current_box, VERTinfo[species][cohort][DEN_id], shiftVERT[species][cohort][DEN_id]);
            
                /* In case need to do aging correct adult abundance directly */
                VERTinfo[species][cohort][DEN_id] *= (1.0 - FunctGroupArray[species].scaled_FSPB[cohort]);
        
                /* If not at same time aging then need to update the shift values - no need to multiply by (1.0 - FunctGroupArray[species].scaled_FSPB[cohort]) here as already applied that above
                 */

                 shiftVERT[species][cohort][DEN_id] = VERTinfo[species][cohort][DEN_id];
                 shiftVERTON[species][cohort]++;
            
                fprintf(llogfp," DEN ending: %e shift ending: %e as scaled_FSPB: %e\n", VERTinfo[species][cohort][DEN_id], shiftVERT[species][cohort][DEN_id], FunctGroupArray[species].scaled_FSPB[cohort]);

            }
        }
    }

    return;
}

/**
 * \brief Update the EMBRYO array
 *
 */
void Ecology_Find_Embryoes(MSEBoxModel *bm, int species, int stock_id, double plankton, double CHLa, int do_debug, FILE *llogfp) {
	double KDENR_sp = KDENR[species][stock_id];
	double PP_sp = FunctGroupArray[species].speciesParams[PP_id];
	double sp_log_mult = FunctGroupArray[species].speciesParams[log_mult_id];
	int recruit_sp = (int) (FunctGroupArray[species].speciesParams[flagrecruit_id]);
	int tsrecruitid = (int) (FunctGroupArray[species].speciesParams[tsrecruit_id]);
	int sp_numGeneTypes = (int) (FunctGroupArray[species].numGeneTypes);
	double ng = (double)(sp_numGeneTypes);
	int ngene = 0, cohort = 0, bcohort = 0, boxkey_id;
    int stocking_sp, mid;
    //int qid = EMBRYO[species].next_spawn;
    int qid = EMBRYO[species].next_larvae;
    double stocked_larvae = 0.0;

    if(EMBRYO[species].SpawnRecruitOverlap && EMBRYO[species].CounterNotDone)
        qid++;
    if (qid > (EMBRYO[species].num_in_larval_queue - 1))
        qid = EMBRYO[species].num_in_larval_queue - 1;
 
    /**
    if (species == 33)
        fprintf(llogfp, "Time: %e %s has qid %d as next_larvae %d SpawnRecruitOverlap %d CounterNotDone %d and next_larve %d \n", bm->dayt, FunctGroupArray[species].groupCode, qid, EMBRYO[species].next_larvae, EMBRYO[species].SpawnRecruitOverlap, EMBRYO[species].CounterNotDone, EMBRYO[species].next_larvae);
    **/

    /* Larvae sanity check (in case of crossed wires in incrementing the queues */
     if (bm->flag_sanity_check && (bm->current_box == bm->first_box) && (bm->current_layer == bm->top_layer)) {
         if (EMBRYO[species].Larvae[stock_id][ngene][qid] > 0) {
            //EMBRYO[species].Larvae[stock_id][ngene][qid] = 0.0;
            fprintf(llogfp, "Time %e %s stock: %d ngene: %d cohort: %d (qid: %d next_spawn: %d) Larvae starts at: %e\n",
                   bm->dayt, FunctGroupArray[species].groupCode, stock_id, ngene, cohort, qid, EMBRYO[species].next_spawn, EMBRYO[species].Larvae[stock_id][ngene][qid]);
             warn("Time %e %s stock: %d ngene: %d cohort: %d (qid: %d next_spawn: %d) Larvae starts at: %e\n",
                  bm->dayt, FunctGroupArray[species].groupCode, stock_id, ngene, cohort, qid, EMBRYO[species].next_spawn, EMBRYO[species].Larvae[stock_id][ngene][qid]);
             EMBRYO[species].Larvae[stock_id][ngene][qid] = 0;
         }
     }
     
     /**/
     
    
    for(ngene = 0; ngene < sp_numGeneTypes; ngene++){
		switch (recruit_sp) {
		case no_recruit:
			quit("No such flagrecruit defined (i.e. value must be > 0)\n");
			break;
		case const_recruit: /* Fixed set of constants */
			if (EMBRYO[species].readytospawn[stock_id] == 1)
				EMBRYO[species].Larvae[stock_id][ngene][qid] = KDENR_sp / ng;
            
            /*
            if (species == 52) {
                fprintf(llogfp, "Time: %e box%d-%d %s larvae %e KDENR: %e ng: %e\n", bm->dayt, bm->current_box, bm->current_layer, FunctGroupArray[species].groupCode, EMBRYO[species].Larvae[stock_id][ngene][qid], KDENR_sp, ng);
            }
             */
            
			break;
		case chl_recruit: /* Proportional to primary productivity */
			EMBRYO[species].Larvae[stock_id][ngene][qid] += recSTOCK[species][stock_id] * PP_sp * bm->cell_vol * (CHLa / bm->ref_chl);
                
            //fprintf(llogfp, "Time: %e box%d-%d %s larvae %e P_sp: %e cell_vol: %e CHLa: %e ref_chl: %e\n", bm->dayt, bm->current_box, bm->current_layer, FunctGroupArray[species].groupCode, EMBRYO[species].Larvae[stock_id][ngene][qid], PP_sp, bm->cell_vol, CHLa, bm->ref_chl);
                
			break;
		case rand_recruit: /* Random - follows lognormal */
			if (EMBRYO[species].readytospawn[stock_id] == 1)
				EMBRYO[species].Larvae[stock_id][ngene][qid] = recSTOCK[species][stock_id] * sp_log_mult * Util_Logx_Result(-lognorm_mu, lognorm_sigma);
			break;
		case plank_recruit: /* Spawn is based on plankton levels (not just CHLa) */
			EMBRYO[species].Larvae[stock_id][ngene][qid] += recSTOCK[species][stock_id] * PP_sp * plankton / bm->ref_chl;
			break;
		case BevHolt_recruit: /* basical Atlantis Beverton-Holt stock-recruit relationship - so uses
		 spawn calculated above */
        case BevHolt_rand_recruit: /* Spawn is based on Beverton Holt with lognormal variation and
		 dependence on plankton levels  - so uses spawn calculated above */
        case BevHolt_num_recruit: /* Beverton-Holt stock-recruit relationship using numbers only - uses spawn calculated above */
		case recover_recruit: /* Beverton-Holt stock-recruit relationship - so uses
		 spawn calculated above */
		case force_recover_recruit: /* Beverton-Holt stock-recruit relationship - so uses
		 spawn calculated above */
		case Ricker_recruit: /* Ricker - so uses spawn calculated above */
		case baltic_ricker: /* Baltic version of the ricker */
        case SSB_ricker: /* SSB based ricker version - for senescent and short lived species */
		case SSB_BevHolt_recruit:/* Standard Beverton Holt */
		case linear_recruit:/* Pupping or calving linearly dependent on maternal condition */

			// Evolution only works for these spawning cases for now - TODO: Generalise so evolution works in all cases
			EMBRYO[species].Larvae[stock_id][ngene][qid] += EMBRYO[species].TotSpawn[ngene];

            /**
            //if (do_debug && (bm->which_check == species)) {
            if (species == 33) {
				fprintf(llogfp,"Doing %s stock %d, ngene: %d, qid: %d, TotSpawn: %e, Larvae: %e\n",
				FunctGroupArray[species].groupCode, stock_id, ngene, qid, EMBRYO[species].TotSpawn[ngene], 
				EMBRYO[species].Larvae[stock_id][ngene][qid]);
            }
            **/
			break;
		case fixed_linear_recruit:/* Pupping or calving a fixed number per adult spawning */
			for(bcohort = 0; bcohort < FunctGroupArray[species].numCohorts; bcohort++){
				cohort = (FunctGroupArray[species].numGeneTypes * bcohort) + ngene;

				EMBRYO[species].Larvae[stock_id][ngene][qid] += recSTOCK[species][stock_id] * KDENR_sp * FunctGroupArray[species].scaled_FSPB[cohort]
						* VERTinfo[species][cohort][DEN_id];

                /**
                //if (do_debug && (bm->which_check == species)) {
                if (species == 33) {
                    fprintf(llogfp,"Time: %e box%d-%d doing %s stock %d, ngene: %d, qid: %d, cohort: %d, recSTOCK: %e, KDENR: %e, scaledFSPB: %e, DEN: %e, Larvae: %e\n", bm->dayt, bm->current_box, bm->current_layer, FunctGroupArray[species].groupCode, stock_id, ngene, qid, cohort, recSTOCK[species][stock_id], KDENR_sp, FunctGroupArray[species].scaled_FSPB[cohort], VERTinfo[species][cohort][DEN_id], EMBRYO[species].Larvae[stock_id][ngene][qid]);
                }
                **/
			}
                
            if ((FunctGroupArray[species].externalReproducer) && ((bm->current_box == bm->first_box) && (bm->current_layer == bm->top_layer))) {
                for (cohort = ((FunctGroupArray[species].numCohortsXnumGenes) - 1); cohort >= 0; cohort--) {
                    for (mid = MIGRATION[species].num_in_queue_done; mid < MIGRATION[species].num_in_queue; mid++) {
                        if(MIGRATION[species].DEN[cohort][mid] > bm->min_dens){

                            EMBRYO[species].Larvae[stock_id][ngene][qid] += KDENR_sp * FunctGroupArray[species].scaled_FSPB[cohort] * MIGRATION[species].DEN[cohort][mid];
                            FunctGroupArray[species].speciesParams[recruit_qid_id] = qid;
                            
                            /**
                            if (do_debug && (bm->which_check == species)) {
                                fprintf(llogfp,"Time: %e %s ngene: %d, qid: %d, mid: %d, cohort: %d, KDENR: %e, scaledFSPB: %e, MIGRATIONden: %e, Larvae: %e\n", bm->dayt, FunctGroupArray[species].groupCode, ngene, qid, mid, cohort, KDENR_sp, FunctGroupArray[species].scaled_FSPB[cohort], MIGRATION[species].DEN[cohort][mid], EMBRYO[species].Larvae[stock_id][ngene][qid]);
                            }
                            **/
                        }
                    }
                }
            }

			break;
		case ts_recruit:/* Read in timeseries of recruitment */
			if (EMBRYO[species].readytospawn[stock_id] == 1) {
				EMBRYO[species].Larvae[stock_id][ngene][qid] = tsEval(bm->tsRecruit, tsrecruitid, bm->t);
                
                /**
                //if (do_debug && (bm->which_check == species))
                    fprintf(llogfp, "Time: %e %s ngene: %d stock: %d qid: %d with Larvae: %e and time t: %e\n", bm->dayt, FunctGroupArray[species].groupCode, ngene, stock_id, qid, EMBRYO[species].Larvae[stock_id][ngene][qid], bm->t);
                **/
			}
			break;
        case multiple_ts_recruit: /* Read in timeseries of recruitment - assume all genotypes get the same number of recruits */
            boxkey_id = bm->BoxKeyMap[bm->current_box][rects_key_id];
            EMBRYO[species].Larvae[stock_id][ngene][qid] +=  Get_Imposed_MultRecruits(bm, species, do_debug, boxkey_id, llogfp);
            break;
		case jackknife_recruit: /* Jackknife spawning function - sum up spawning stock biomass */
			for(bcohort = 0; bcohort < FunctGroupArray[species].numCohorts; bcohort++){
				cohort = (FunctGroupArray[species].numGeneTypes * bcohort) + ngene;
				EMBRYO[species].Larvae[stock_id][ngene][qid] += VERTinfo[species][cohort][DEN_id] * FunctGroupArray[species].scaled_FSPB[cohort] * (VERTinfo[species][cohort][SN_id] + VERTinfo[species][cohort][RN_id]) * bm->X_CN * mg_2_tonne;
			}
                
            if ((FunctGroupArray[species].externalReproducer) && ((bm->current_box == bm->first_box) && (bm->current_layer == bm->top_layer))) {
                for (cohort = ((FunctGroupArray[species].numCohortsXnumGenes) - 1); cohort >= 0; cohort--) {
                    for (mid = MIGRATION[species].num_in_queue_done; mid < MIGRATION[species].num_in_queue; mid++) {
                        if(MIGRATION[species].DEN[cohort][mid] > bm->min_dens){
                                
                            EMBRYO[species].Larvae[stock_id][ngene][qid] += MIGRATION[species].DEN[cohort][mid] * FunctGroupArray[species].scaled_FSPB[cohort] * (MIGRATION[species].SN[cohort][mid] + MIGRATION[species].RN[cohort][mid]) * bm->X_CN * mg_2_tonne;
                            FunctGroupArray[species].speciesParams[recruit_qid_id] = qid;
                        }
                    }
                }
            }
			break;

        case BevHolt_direct_num_recruit: /* Beverton-Holt stock-recruit relationship - numbers only case (when spawn set proprtional to numbers spawning) */
			for(bcohort = 0; bcohort < FunctGroupArray[species].numCohorts; bcohort++){
				cohort = (FunctGroupArray[species].numGeneTypes * bcohort) + ngene;

				EMBRYO[species].Larvae[stock_id][ngene][qid] += FunctGroupArray[species].scaled_FSPB[cohort] * VERTinfo[species][cohort][DEN_id];
			}
                
            if ((FunctGroupArray[species].externalReproducer) && ((bm->current_box == bm->first_box) && (bm->current_layer == bm->top_layer))) {
                for (cohort = ((FunctGroupArray[species].numCohortsXnumGenes) - 1); cohort >= 0; cohort--) {
                    for (mid = MIGRATION[species].num_in_queue_done; mid < MIGRATION[species].num_in_queue; mid++) {
                        if(MIGRATION[species].DEN[cohort][mid] > bm->min_dens){
                                
                            EMBRYO[species].Larvae[stock_id][ngene][qid] += FunctGroupArray[species].scaled_FSPB[cohort] * MIGRATION[species].DEN[cohort][mid];
                            FunctGroupArray[species].speciesParams[recruit_qid_id] = qid;
                        }
                    }
                }
            }

            break;

		default:
			quit("No such flagrecruit defined for vertebrates (i.e. value must be between 1 ands 10 at present)\n");
			break;
		}
        
        stocking_sp = (int) (FunctGroupArray[species].speciesParams[flagstocking_id]);
        if (stocking_sp) {
            /* Read in timeseries of number of recruits added due to stocking - assume all genotypes get the same number of recruits */
            boxkey_id = bm->BoxKeyMap[bm->current_box][rects_key_id];
            stocked_larvae = Get_Imposed_MultRecruits(bm, species, do_debug, boxkey_id, llogfp);
            EMBRYO[species].Larvae[stock_id][ngene][qid] += stocked_larvae;
        }

        /**
		//if (do_debug && (bm->which_check == species)) {
        //if (stocking_sp) {
        if (do_debug) {
            fprintf(llogfp, "species %s, case: %d, stock: %d, ngene: %d, qid: %d, stocked larvae: %e (tot new Larvae: %e), totspawn: %e recruit_sp: %d\n",
				FunctGroupArray[species].groupCode, recruit_sp, stock_id, ngene, qid, stocked_larvae,
				EMBRYO[species].Larvae[stock_id][ngene][qid], EMBRYO[species].TotSpawn[ngene], recruit_sp);
		}
        **/
        
        if(bm->track_contaminants && bm->flag_contamMaternalTransfer){
            Store_Recruit_Contaminants(bm, species, stock_id, ngene, qid);
        }
	}
    
    if (bm->flag_do_evolution){ /* Redistribute larvae across based on evolutiomn curve if necesary */
        Evolution_Curve(bm, species, stock_id, qid, do_debug, llogfp);
    }
    
    return;
}

/**
 * \brief Find the number of recruits
 *
 */
void Get_Recruits(MSEBoxModel *bm, int species, int stock_id, double plankton, FILE *llogfp) {
	double step1, step2, jack_SSB, jack_B, jack_a, larval_scalar;
    double current_ICE, prod_alpha, dd_beta1, dd_beta2, temp_coefft, rate_coefft, wind_coefft, recruit_var;
	double temprec = 0.0;
	double BHalpha_sp = FunctGroupArray[species].speciesParams[BHalpha_id];
	double BHbeta_sp = FunctGroupArray[species].speciesParams[BHbeta_id];
	double Ralpha_sp = FunctGroupArray[species].speciesParams[Ralpha_id];
	double Rbeta_sp = FunctGroupArray[species].speciesParams[Rbeta_id];
    double norm_sigma = FunctGroupArray[species].speciesParams[norm_sigma_id];
	double recover_mult_sp = FunctGroupArray[species].speciesParams[recover_mult_id];
	double recover_start_sp = FunctGroupArray[species].speciesParams[recover_start_id];
	//double KWSR_sp = FunctGroupArray[species].speciesParams[KWSR_id];
	//double KWRR_sp = FunctGroupArray[species].speciesParams[KWRR_id];
	int recruit_sp = (int) (FunctGroupArray[species].speciesParams[flagrecruit_id]);
	int sp_numGeneTypes = (int) (FunctGroupArray[species].numGeneTypes);
	int ngene = 0;
    double this_stock = 0.0;
	//int do_debug = 0;
    //int qid = EMBRYO[species].next_spawn;
    int qid = EMBRYO[species].next_larvae;

    if(EMBRYO[species].SpawnRecruitOverlap)  // Do not use the EMBRYO[species].CounterNotDone check here as it throws things out of whack for some species who spawn during the same period recruits arrive
        qid++;

    if (qid > (EMBRYO[species].num_in_larval_queue - 1))
        qid = EMBRYO[species].num_in_larval_queue - 1;

    /*
    if (species == 0) {
        fprintf(llogfp,"Time: %e %s has next_spawn %d and qid %d as EMBRYO[species].SpawnRecruitOverlap %d ",
                bm->dayt, FunctGroupArray[species].groupCode, EMBRYO[species].next_larvae, qid, EMBRYO[species].SpawnRecruitOverlap);
    }
    */

    /*
    if(((bm->current_box == bm->checkbox) || (bm->checkbox > bm->nbox)) && (bm->dayt >= bm->checkstart) && (bm->which_check == species))
		do_debug = 1;
    
    if ((FunctGroupArray[species].numSpawns > 1) || (FunctGroupArray[species].groupType == FISH_INVERT))
        do_debug = 1;

    if((bm->dayt >= bm->checkstart) && (bm->which_check == species)){
        do_debug = 1;
    }
    */

	for(ngene = 0; ngene < sp_numGeneTypes; ngene++){
		switch (recruit_sp) {
		case no_recruit:
			quit("No such flagrecruit defined (i.e. value must be > 0)\n");
			break;
		case const_recruit: /* Fixed set of constants */
		case chl_recruit: /* Proportional to primary productivity */
		case rand_recruit: /* Random - follows lognormal */
		case plank_recruit: /* Spawn is based on plankton levels (not just CHLa) */
		case linear_recruit:/* Pupping or calving linearly dependent on maternal condition */
		case fixed_linear_recruit:/* Pupping or calving a fixed number per adult spawning */
		case ts_recruit:/* Read in timeseries of recruitment */
        case multiple_ts_recruit: /* Read in timeseries of recruitment */
			temprec = EMBRYO[species].Larvae[stock_id][ngene][qid];
            
            /**
            //if (do_debug) {
             //if (species == 33) {
            if (species == 52) {
                fprintf( llogfp, "Time: %e, box%d-%d, species %s, ngene: %d, stock: %d, qid: %d, temprec: %e, Larvae: %e\n", bm->dayt, bm->current_box, bm->current_layer, FunctGroupArray[species].groupCode, ngene, stock_id, qid, temprec, EMBRYO[species].Larvae[stock_id][ngene][qid]);
            }
            **/
                
			break;
		case BevHolt_recruit: /* Beverton-Holt stock-recruit relationship - Atlantis basic version (mix numbers and biomass) */
			temprec = (recSTOCK[species][stock_id] * BHalpha_sp * EMBRYO[species].Larvae[stock_id][ngene][qid] / (BHbeta_sp + bm->totfishpop[species] * stock_prop[species][stock_id]));

            /**
			//if (do_debug && (bm->which_check == species)) {
            //if (do_debug) {
            if (species == 33) {
				fprintf( llogfp,
					"Time: %e, box%d-%d, species %s, ngene: %d, stock: %d, temprec: %e, recSTOCK: %e, BHalpha: %e, Larvae: %e, BHbeta: %e, totfish: %e, stock_prop: %e)\n",
						bm->dayt, bm->current_box, bm->current_layer, FunctGroupArray[species].groupCode, 
						ngene, stock_id, temprec, recSTOCK[species][stock_id], BHalpha_sp,
						EMBRYO[species].Larvae[stock_id][ngene][qid], BHbeta_sp, bm->totfishpop[species],
						stock_prop[species][stock_id]);
			}
            **/
			break;
        case BevHolt_direct_num_recruit: /* Beverton-Holt stock-recruit relationship - numbers only case (when spawn set proprtional to numbers spawning) */
        case BevHolt_num_recruit: /* Beverton-Holt stock-recruit relationship - numbers only case */
            temprec = (recSTOCK[species][stock_id] * BHalpha_sp * EMBRYO[species].Larvae[stock_id][ngene][qid] / (BHbeta_sp  + recSTOCK[species][stock_id] * EMBRYO[species].Larvae[stock_id][ngene][qid]));

            break;
        case BevHolt_rand_recruit: /* Spawn is based on Beverton Holt with lognormal variation and
		 dependence on plankton levels */
			step1 = Util_Logx_Result(-lognorm_mu, lognorm_sigma);
			step2 = (recSTOCK[species][stock_id] * BHalpha_sp * EMBRYO[species].Larvae[stock_id][ngene][qid] / (BHbeta_sp + bm->totfishpop[species] * stock_prop[species][stock_id])) * (plankton / bm->ref_chl);
			temprec = step2 * step1;
			break;
		case recover_recruit: /* Spawn is allowed a recovery encouraging boost of recruits
		 after "recovery_span" years of depressed stock levels */
			temprec = (recSTOCK[species][stock_id] * BHalpha_sp * EMBRYO[species].Larvae[stock_id][ngene][qid] / (BHbeta_sp + bm->totfishpop[species] * stock_prop[species][stock_id]));

			if (recover_help[species][0] && (recover_help_set[species] <= bm->dayt - (recover_span * recover_subseq))) {
				temprec *= recover_mult_sp;
				if (recover_help[species][1]) {
					fprintf(llogfp, "Time: %e, species %s has had a recovery event (temprec = %e)\n", bm->dayt, FunctGroupArray[species].groupCode, temprec);
					recover_help[species][1] = 0;
				}
			}
			break;
		case force_recover_recruit: /* Spawn has a pre-specified recovery encouraging boost of recruits */
			temprec = (recSTOCK[species][stock_id] * BHalpha_sp * EMBRYO[species].Larvae[stock_id][ngene][qid] / (BHbeta_sp + bm->totfishpop[species] * stock_prop[species][stock_id]));

			if ((bm->dayt >= recover_start_sp) && (bm->dayt <= (recover_start_sp + recover_subseq))) {
				temprec *= recover_mult_sp;
				fprintf(llogfp, "Time: %e, species %s has had a prescribed recovery event (temprec = %e)\n", bm->dayt, FunctGroupArray[species].groupCode, temprec);
			}
			break;
		case Ricker_recruit: /* Ricker */
			temprec = bm->totfishpop[species] * stock_prop[species][stock_id] * exp(recSTOCK[species][stock_id] * Ralpha_sp * (1.0 - bm->totfishpop[species] * stock_prop[species][stock_id] / Rbeta_sp));
           
            /**
            if (do_debug && (bm->which_check == species)) {
                fprintf( llogfp, "Time: %e, box%d-%d, species %s, ngene: %d, stock: %d, temprec: %e, totfishpop: %e, stock_prop: %e, recSTOCK: %e, Ralpha: %e, Larvae: %e, Rbeta: %e\n",
                    bm->dayt, bm->current_box, bm->current_layer, FunctGroupArray[species].groupCode,
                    ngene, stock_id, temprec, bm->totfishpop[species], stock_prop[species][stock_id], recSTOCK[species][stock_id], Ralpha_sp,
                    EMBRYO[species].Larvae[stock_id][ngene][qid], Rbeta_sp);
            }
            **/
                
			break;
		case SSB_BevHolt_recruit: /* Standard Beverton-Holt stock-recruit relationship */
			temprec = (recSTOCK[species][stock_id] * BHalpha_sp * bm->totfishpop[species] * stock_prop[species][stock_id] / (BHbeta_sp + bm->totfishpop[species] * stock_prop[species][stock_id]));
			break;
		case jackknife_recruit: /* Jackknife spawning function */
			jack_SSB = EMBRYO[species].Larvae[stock_id][ngene][qid];
			jack_B = FunctGroupArray[species].speciesParams[jack_b_id] * bm->totfishpop[species] * stock_prop[species][stock_id];
			jack_a = FunctGroupArray[species].speciesParams[jack_a_id];

			if (jack_SSB <  jack_B)
				temprec = jack_a * jack_SSB;
			else
				temprec = jack_a * jack_B;
			break;
		case baltic_ricker: /* Baltic version of the ricker */
            temprec = 1000 * recSTOCK[species][stock_id] * Ralpha_sp * bm->totfishpop[species] * bm->X_CN * mg_2_tonne
                * stock_prop[species][stock_id] * exp( -1.0 * Rbeta_sp * bm->totfishpop[species] * bm->X_CN * mg_2_tonne * stock_prop[species][stock_id]);
			break;
        case SSB_ricker: /* SSB based version of the ricker given short lived and senescent - e.g. for cephalopods */
                temprec = bm->tot_SSB[species] * stock_prop[species][stock_id] * exp(recSTOCK[species][stock_id] * Ralpha_sp * (1.0 - bm->tot_SSB[species] * stock_prop[species][stock_id] / Rbeta_sp));

                /*
                if (do_debug && (bm->which_check == species)) {
                    fprintf( llogfp, "Time: %e, box%d-%d, species %s, ngene: %d, stock: %d, temprec: %e, totfishpop: %e, stock_prop: %e, recSTOCK: %e, Ralpha: %e, Larvae: %e, Rbeta: %e\n",
                            bm->dayt, bm->current_box, bm->current_layer, FunctGroupArray[species].groupCode,
                            ngene, stock_id, temprec, bm->tot_SSB[species], stock_prop[species][stock_id], recSTOCK[species][stock_id], Ralpha_sp,
                            EMBRYO[species].Larvae[stock_id][ngene][expect_id], Rbeta_sp);
                }
                 */
            break;
        case ice_wimd_based: // For lake models - from Lynch et al (2015) Journal of Great Lakes Research 41: 415–422
            if(bm->ice_on) {
                // Load time series values for forcing values
                bm->airtemp_index = tsEvalR(bm->thermal_index, bm->tindex_id, bm->t, bm->tindex_rewindid);
                bm->airT_rate_index = tsEvalR(bm->rate_index, bm->rindex_id, bm->t, bm->rindex_rewindid);
                current_ICE = Get_Ice_Rating(bm, species);
                
                this_stock = bm->tot_SSB[species] * stock_prop[species][stock_id];

                prod_alpha = FunctGroupArray[species].speciesParams[prod_alpha_id];
                dd_beta1 = FunctGroupArray[species].speciesParams[den_depend_beta1_id];
                dd_beta2 = FunctGroupArray[species].speciesParams[den_depend_beta2_id];
                temp_coefft = FunctGroupArray[species].speciesParams[temp_coefft_id];
                rate_coefft = FunctGroupArray[species].speciesParams[rate_coefft_id];
                wind_coefft = FunctGroupArray[species].speciesParams[wind_coefft_id];
                
                recruit_var = FunctGroupArray[species].speciesParams[recruit_var_id];
                
                step1 = prod_alpha - dd_beta1 * this_stock * dd_beta2 * current_ICE + temp_coefft * bm->airtemp_index + rate_coefft * bm->airT_rate_index +  recruit_var * 0.5;
                if(bm->track_wind) {
                    step1 += (wind_coefft * current_WIND);
                }
                temprec = this_stock * exp(step1);
            }
            break;
		default:
			quit("No such flagrecruit defined for vertebrates (%d) - value must be between 0 and 10 currently\n", recruit_sp);
			break;
		}

        // Check for any larval processes
        if ( FunctGroupArray[species].speciesParams[intersp_depend_recruit_id] > 0) {
            larval_scalar = Larval_Mortality(bm, species, stock_id, llogfp);
            temprec *= larval_scalar;
        }
        
        // Check for any stochasticity added to this recruitment
        if ( FunctGroupArray[species].speciesParams[flag_recruit_stochastic_id] > 0) {
            step1 = temprec;
            temprec = Util_Normx_Result(step1, norm_sigma);
        }
        
        // Store the final number of recruits
		EMBRYO[species].BulkRecruits[ngene] = temprec;

        /**
        //if (do_debug && (bm->which_check == species)) {
        //if (do_debug) {
        //if (recruit_sp == fixed_linear_recruit) {
        if (species == 33) {
			fprintf( llogfp, "Time: %e, box%d-%d, species %s, ngene: %d, recruit_sp case: %d, BulkRecruits: %e, temprec: %e, Larvae: %e qid: %d)\n",
					bm->dayt, bm->current_box, bm->current_layer, FunctGroupArray[species].groupCode, ngene, recruit_sp,
					EMBRYO[species].BulkRecruits[ngene], temprec, EMBRYO[species].Larvae[stock_id][ngene][qid], qid);
		}
        **/
        
        if(bm->track_contaminants && bm->flag_contamMaternalTransfer){
            Get_Recruit_Contaminants(bm, species, ngene, stock_id, qid);
        }
        
	}
    
	return;
}

/**
 * \brief Apply any effects that happen during the larval period
 *
 */
double Larval_Mortality(MSEBoxModel *bm, int species, int stock_id, FILE *llogfp) {
    int depend_type = (int)(FunctGroupArray[species].speciesParams[intersp_depend_recruit_id]);
    int dependsp_id = (int)(FunctGroupArray[species].speciesParams[intersp_depend_sp_id]);
    double dependbiom = bm->totfishpop[dependsp_id];
    double depend_scalar = FunctGroupArray[species].speciesParams[intersp_depend_scale_id];
    double ans = 0.0;
    
    if (depend_type == intersp_linear) {  // Directly proportional
        ans = depend_scalar * dependbiom;
    } else if (depend_type == intersp_inverse) { // Indirectly proportional
        ans = depend_scalar / (dependbiom + small_num);
    }

    return ans;
}

/**
 * \brief Apply any environmental forcing or conditioning to the recruitment
 *
 */
double Get_Enviro_Recruit_Forcing(MSEBoxModel *bm, int species, int do_debug, FILE *llogfp) {
	int temp_sensitive_sp = (int) (FunctGroupArray[species].speciesParams[flagtempsensitive_id]);
	int salt_sensitive_sp = (int)(FunctGroupArray[species].speciesParams[flagSaltSensitive_id]);
	int pHsensitive_sp = (int) (FunctGroupArray[species].speciesParams[flagfecundsensitive_id]);
	int sp_q10receff = (int) (FunctGroupArray[species].speciesParams[flagq10receff_id]);
	int envforceid = (int) (FunctGroupArray[species].speciesParams[envforce_id]);
	int flagcontract_sp = (int)(FunctGroupArray[species].speciesParams[flagcontract_tol_id]);
	double min_spawntemp_sp = FunctGroupArray[species].speciesParams[min_spawn_temp_id];
	double max_spawntemp_sp = FunctGroupArray[species].speciesParams[max_spawn_temp_id];
	double min_spawnsalt_sp = FunctGroupArray[species].speciesParams[min_spawn_salt_id];
	double max_spawnsalt_sp = FunctGroupArray[species].speciesParams[max_spawn_salt_id];
	//double min_spawnpH_sp = FunctGroupArray[species].speciesParams[min_spawn_pH_id];
	//double max_spawnpH_sp = FunctGroupArray[species].speciesParams[max_spawn_pH_id];
	double contract_sp = FunctGroupArray[species].speciesParams[contract_tol_id];
	double enviro_force = 1.0;
	double enviro_scalar = 1.0;
	double pHscalar = 1.0;
	double Tscalar = 1.0;
    double recCcorr = 1.0;
    //int recruit_sp = (int) (FunctGroupArray[species].speciesParams[flagrecruit_id]);
    
    if(!bm->flagtempdepend_reprod) { // Have turned off temperature dependent movement
        if(temp_sensitive_sp && bm->newmonth) {
            warn("Time: %e Zeroing temperature sensitivity for reproduction of %s even though flagtempsensitive %d as flagtempdepend_move = %d\n", bm->dayt, FunctGroupArray[species].groupCode, temp_sensitive_sp, bm->flagtempdepend_move);
        }
        temp_sensitive_sp = 0;
    }


	/* pH contribution */
	pHscalar = (FunctGroupArray[species].pHcorr - 1.0);
	if( flagcontract_sp  && (pHscalar > 0.0)) {
		min_spawntemp_sp += contract_sp * pHscalar;
		max_spawntemp_sp -= contract_sp * pHscalar;
	}
	if(pHsensitive_sp){
		// Inverse as assume declines as pH drops but that pHCorr will
		// be higher for vertebrates as their availability to predators
		// will be higher
		pHscalar = (1.0 / FunctGroupArray[species].pHcorr);
	} else {
		pHscalar = 1.0;
	}

	if (temp_sensitive_sp) {

		/* Combined temperature effects */
		if((FunctGroupArray[species].Tcorr * pHscalar) < 1.0)
			Tscalar = (FunctGroupArray[species].Tcorr * pHscalar);
		else
			Tscalar = 1.0 / (FunctGroupArray[species].Tcorr * pHscalar);

		if (sp_q10receff == 1) {
			enviro_scalar *= Tscalar;
		} else if (sp_q10receff == 2) {
			enviro_scalar *= Tscalar;
		} else {
			/* simple pH effects */
			enviro_scalar *= FunctGroupArray[species].pHcorr;

			/* No such setting as yet
			if ( current_PH < min_spawnpH_sp)
				enviro_scalar *= 0.0;
			else if ( current_PH > max_spawnpH_sp)
				enviro_scalar *= 0.0;
			*/
		}

		/* Other temperature effects */
		if (H2Otemp < min_spawntemp_sp){
			enviro_scalar *= 0.0;
		}
		else if (H2Otemp > max_spawntemp_sp){
			enviro_scalar *= 0.0;
		}
	}

	/* Replicate old bec_dev results on existing models. We will remove this asap! */
	if(bm->flag_replicated_old == FALSE){

		if (salt_sensitive_sp){
			/* Salinity effects */
			enviro_scalar *= FunctGroupArray[species].Scorr;

			if ( current_SALT < min_spawnsalt_sp)
				enviro_scalar *= 0.0;
			else if ( current_SALT > max_spawnsalt_sp)
				enviro_scalar *= 0.0;
		}
	}

	/* If we have contaminant then take them into account as well. */
	if(bm->track_contaminants){
		/* Contaminant effects */
        recCcorr = Calculate_Contaminant_Repro_Scalar(bm, species);
		enviro_scalar *= recCcorr;
	}

	/* Environmental forcing */
	if ((bm->flagtsforcerecruit) && (envforceid > -1)) {
		enviro_force = tsEval(bm->env_force, envforceid, bm->t);
		enviro_scalar *= enviro_force;

        /*
        if (do_debug && (bm->which_check == species)) {
			fprintf(llogfp, "Time: %e, box%d-%d, species %s, enviro_force: %e\n", bm->dayt, bm->current_box, bm->current_layer,
					FunctGroupArray[species].groupCode, enviro_force);
		}
         */
	}

    /**
	//if (do_debug && (bm->which_check == species)) {
    if (species == 52) {
		fprintf(llogfp, "%s enviro_scalar: %e, has sensitive_sp: %d (saltsens: %d), pHscalar: %e, Tscalar: %e, flagtsforcerecruit: %d\n", FunctGroupArray[species].groupCode, enviro_scalar, temp_sensitive_sp, salt_sensitive_sp, pHscalar, Tscalar, bm->flagtsforcerecruit);
		fprintf(llogfp, "%s has H2Otemp: %e min_spawntemp_sp: %e max_spawntemp_sp: %e current_SALT: %e min_spawnsalt_sp: %e max_spawnsalt_sp: %e\n",
				FunctGroupArray[species].groupCode, H2Otemp, min_spawntemp_sp, max_spawntemp_sp, current_SALT, min_spawnsalt_sp, max_spawnsalt_sp);
	}
    **/
    
	return enviro_scalar;
}

double Get_Recruit_Size_Forcing(MSEBoxModel *bm, int species, int do_debug, FILE *llogfp) {
    int KWSRforceid = (int) (FunctGroupArray[species].speciesParams[KWSRforce_id]);
    double KWSR_sp = FunctGroupArray[species].speciesParams[KWSR_id];
    double KWSR_force = 0.0;
    
    /* Forcing */
    if (bm->flag_modify_KWSR && (KWSRforceid > -1)) {
        KWSR_force = tsEval(bm->KWSR_force, KWSRforceid, bm->t);
        
        if (bm->flag_modify_KWSR == scale_KWSR)
            KWSR_force *= KWSR_sp;
    }
    
    return KWSR_force;
}

void Recruit_Migration(MSEBoxModel *bm, int species, int wclayer, int stock_id, int do_debug, FILE *llogfp) {

	int sp_numGeneTypes = (int) (FunctGroupArray[species].numGeneTypes);
	int ngene = 0, mid, lid, recruit_outside, direct_recruit_entry;
	int thisday = 0;
    double oldDEN, oldSN, oldRN, embryo_recruits;
    //double orig_den;
	double KWSR_sp = FunctGroupArray[species].speciesParams[KWSR_id];
	double KWRR_sp = FunctGroupArray[species].speciesParams[KWRR_id];
    int larval_queue_extension = FunctGroupArray[species].speciesParams[larval_queue_extension_id];
    //int cohort_recruit_entry = FunctGroupArray[species].speciesParams[cohort_recruit_entry_id];
    
    //fprintf(bm->logFile, "Time: %e %s box%d-%d (vs first_box %d, top_layer %d) numGeneTypes: %d Recruit_Migration recruitType: %d num_in_queue: %d externalReproducer: %d\n",  bm->dayt, FunctGroupArray[species].groupCode, bm->current_box, bm->current_layer, bm->first_box, bm->top_layer, sp_numGeneTypes, FunctGroupArray[species].recruitType, MIGRATION[species].num_in_queue, FunctGroupArray[species].externalReproducer);

    if((FunctGroupArray[species].externalReproducer && (FunctGroupArray[species].recruitType != external_spawn_local_recruit)) || (FunctGroupArray[species].recruitType == external_recruit)) {
        if ((bm->current_box == bm->first_box) && (bm->current_layer == bm->top_layer)) {

            for(ngene = 0; ngene < sp_numGeneTypes; ngene++){
                for (mid = 0; mid < MIGRATION[species].num_in_queue; mid++) {
                    
                    fprintf(bm->logFile, "Time: %e %s RecruitQueueMatch: %d\n", bm->dayt, FunctGroupArray[species].groupCode, MIGRATION[species].RecruitQueueMatch[mid]);
                    
                    if (MIGRATION[species].RecruitQueueMatch[mid] > 0) {
                        if (bm->flag_modify_KWSR) {
                            KWSR_sp = Get_Recruit_Size_Forcing(bm, species, do_debug, llogfp);
                            KWRR_sp = KWSR_sp * FunctGroupArray[species].X_RS[ngene];
                        }

                        /* If recruit outside model area and in first dynamic box then update the Migration array */
                        recruit_outside = 0;

                        thisday = EMBRYO[species].StartDay[ngene][EMBRYO[species].next_recruit];
                    
                        /**
                        if(do_debug) {
                            fprintf(bm->logFile, "Time: %e %s mid: %d Return_Now = %d, Leave_Now = %d, thisday: %d\n",  bm->dayt, FunctGroupArray[species].groupCode, mid, MIGRATION[species].Return_Now[mid], MIGRATION[species].Leave_Now[mid], thisday);
                        }
                        **/

                        /**/
                        if (thisday < MIGRATION[species].Leave_Now[mid])
                            continue; // Skip ahead if haven't gotten this far in time yet
                        if ( thisday > MIGRATION[species].Return_Now[mid])
                            continue; // Skip ahead if time is alredy past the migration time
                        /**/
                    
                        /* Outside the model when recruitment happens */
                        if ((MIGRATION[species].Return_Now[mid] >= thisday) && (MIGRATION[species].Leave_Now[mid] <= thisday)) {
                            recruit_outside = 1;
                            FunctGroupArray[species].speciesParams[recruit_outside_id] = 1;
                            FunctGroupArray[species].speciesParams[direct_recruit_entry_id] = 0;
                            direct_recruit_entry = 1;
                        }
                                        
                        /**
                        //if(do_debug) {
                        if ((species == 26) || (species == 35) || (species == 64)) {
                            fprintf(bm->logFile, "Time: %e %s-%d mid: %d recruit_outside = %d, MIGRATION.recruit = %e (leave: %d return: %d start_return_chrt: %d direct_recruit_entry: %e)\n",  bm->dayt, FunctGroupArray[species].groupCode, ngene, mid, recruit_outside, MIGRATION[species].recruit[ngene][mid], MIGRATION[species].Leave_Now[mid], MIGRATION[species].Return_Now[mid], MIGRATION[species].start_return_chrt[mid], FunctGroupArray[species].speciesParams[direct_recruit_entry_id]);
                        }
                        **/
                    
                        if (recruit_outside && (!direct_recruit_entry)) {
                            oldDEN = MIGRATION[species].DEN[ngene][mid];
                            oldSN = MIGRATION[species].SN[ngene][mid];
                            oldRN = MIGRATION[species].RN[ngene][mid];
                        
                            embryo_recruits = 0.0;
                            if(larval_queue_extension) {
                                for (lid = 0; lid < larval_queue_extension; lid++) {
                                    embryo_recruits += EMBRYO[species].Larvae[stock_id][ngene][lid];
                                    EMBRYO[species].Larvae[stock_id][ngene][lid] = 0;
                                }
                                // Reset larval_queue_extension so not called in subsequent years
                                if (stock_id == (FunctGroupArray[species].numStocks - 1)) {
                                    FunctGroupArray[species].speciesParams[larval_queue_extension_id] = 0;
                                }
                            }

                            if((MIGRATION[species].recruit[ngene][mid] > 0.0) || (embryo_recruits)) {
                                //orig_den = MIGRATION[species].DEN[ngene][mid];
                                MIGRATION[species].DEN[ngene][mid] += (MIGRATION[species].recruit[ngene][mid] + embryo_recruits); // As all in youngest cohort maps to ngene not cohort in Migration
                                //MIGRATION[species].recruit[ngene][mid] = EMBRYO[species].BulkRecruits[ngene] * enviro_scalar;
                            
                                // Update size here too - used to reset to KMIG each year, but that is erroneous
                                MIGRATION[species].SN[ngene][mid] = (oldSN * oldDEN + KWSR_sp *	MIGRATION[species].recruit[ngene][mid]) /
                                    (MIGRATION[species].recruit[ngene][mid] + oldDEN + small_num);
                                MIGRATION[species].RN[ngene][mid] = (oldRN * oldDEN + KWRR_sp *	MIGRATION[species].recruit[ngene][mid]) /
                                    (MIGRATION[species].recruit[ngene][mid] + oldDEN + small_num);
                            
                                //fprintf(bm->logFile, "Time: %e %s-%d mid: %d now MigDEN: %e as MIGrecruit: %e MigYOY: %e embryo_recruits: %e with SN: %e and RN: %e\n", bm->dayt, FunctGroupArray[species].groupCode, ngene, mid, MIGRATION[species].DEN[ngene][mid], MIGRATION[species].recruit[ngene][mid], MIGRATION[species].MigYOY[ngene][mid], embryo_recruits, MIGRATION[species].SN[ngene][mid], MIGRATION[species].RN[ngene][mid]);

                                if(bm->track_contaminants && bm->flag_contamMaternalTransfer){
                                    Get_SettlerMigrant_Contaminants(bm, species, ngene, mid, oldDEN, MIGRATION[species].recruit[ngene][mid]);
                                }
                                
                                // Reset now passed across
                                MIGRATION[species].recruit[ngene][mid] = 0;
                            }
 						}
					}
				}
			}
		}
	}
    
    return;
}

/**
 * \brief Get final distribution of recruits across horizontal boxes and vertical layers (taking into account local recruitment)
 */
void Find_Final_Recruit_Distribution(MSEBoxModel *bm, int species, double enviro_scalar, double vertdistrib, int wclayer, int stock_id, int do_debug, FILE *llogfp) {
    int recruit_outside = 0;
	int qid = EMBRYO[species].next_recruit;
	//int thisday = 0;
	int sp_numGeneTypes = (int) (FunctGroupArray[species].numGeneTypes);
	int ngene = 0;
	int mid = 0;
	double recruitment_scalar;
	double hab_scalar = Get_Habitat_Mediated_Recruit_Scalar(bm, species, llogfp);
	enviro_scalar *= hab_scalar;
    
    //double KWSR_sp = FunctGroupArray[species].speciesParams[KWSR_id];
    //double KWRR_sp = FunctGroupArray[species].speciesParams[KWRR_id];
    //double oldDEN, oldSN, oldRN;
    
    if(EMBRYO[species].SpawnRecruitOverlap && EMBRYO[species].CounterNotDone)
        qid++;

    /* Recruitment scaling due to external files - added for Cam for the GOC model */
	recruitment_scalar = Ecology_Get_Recruitment_Scalar(bm, species);
    
    /**
	if (bm->debug == debug_external_scaling && (bm->which_check == species)) {
		fprintf(bm->logFile, "Time: %e, External recruitment scaling of group %s in box %d:%d. Scaling - %e. Spawning numbers before scaling = %e, after = %e\n",
				bm->dayt, FunctGroupArray[species].groupCode, bm->current_box, bm->current_layer, recruitment_scalar, enviro_scalar, enviro_scalar * recruitment_scalar );
	}
    
    if (species == 0){
        fprintf(llogfp, "Time: %e %s has qid %d as next_recruit %d SpawnRecruitOverlap %d CounterNotDone %d\n", bm->dayt, FunctGroupArray[species].groupCode, qid, EMBRYO[species].next_recruit, EMBRYO[species].SpawnRecruitOverlap, EMBRYO[species].CounterNotDone);
    }
     **/

	enviro_scalar *= recruitment_scalar;

	for(ngene = 0; ngene < sp_numGeneTypes; ngene++){

        /**
        if (do_debug && (bm->which_check == species)) {
			fprintf( llogfp,
					"Final recruit distrib Time: %e, box%d-%d, species %s ngene: %d, recruit_Type: %d, BulkRecruits: %e, enviro_scalar: %e, hab_scalar: %e, vertdistrib: %e, recruit_hdistrib[box%d][%s]: %e, recryut_type: %d (EMBRYO[%s][%d]: %e)\n",
					bm->dayt, bm->current_box, bm->current_layer, FunctGroupArray[species].groupCode, ngene,FunctGroupArray[species].recruitType, EMBRYO[species].BulkRecruits[ngene],
					enviro_scalar, hab_scalar, vertdistrib, bm->current_box, FunctGroupArray[species].groupCode, bm->recruit_hdistrib[ngene][bm->current_box][species],
                    FunctGroupArray[species].recruitType, FunctGroupArray[species].groupCode, stock_id, EMBRYO[species].Larvae[stock_id][ngene][qid]);
		}
        **/

		switch (FunctGroupArray[species].recruitType) {
		case independent_recruit_distrib:
		case at_parent_location:
        case external_spawn_local_recruit:
			/* If recruitment occurs within the model domain, assign final values
			 - checks to make sure distributions don't lead to artifical reseeding are
			 done on model initialisation on whenever the're reset (e.g. by movement, if
			 have localised recruitment) */
			EMBRYO[species].num_recruits[bm->current_box][wclayer][ngene][qid] = EMBRYO[species].BulkRecruits[ngene] * enviro_scalar * vertdistrib * bm->recruit_hdistrib[ngene][bm->current_box][species];
			totrecruit[species][stock_id][ngene] += EMBRYO[species].num_recruits[bm->current_box][wclayer][ngene][qid];

            /**
            if (do_debug && (species == bm->which_check)) {
            //if (species < 6) {
                fprintf(llogfp,"Time: %e, %s box%d-%d ngene: %d, qid: %d, totrecruit: %e num_rec: %e (Bulkrecruits: %e, enviro_scalar: %e, vdistrib: %e, hdistrib: %e)\n", bm->dayt, FunctGroupArray[species].groupCode, bm->current_box, wclayer, ngene, qid, totrecruit[species][stock_id][ngene], EMBRYO[species].num_recruits[bm->current_box][wclayer][ngene][qid], EMBRYO[species].BulkRecruits[ngene], enviro_scalar, vertdistrib, bm->recruit_hdistrib[ngene][bm->current_box][species]);
            }
            **/
                
			break;
		case external_recruit:
			if ((bm->current_box == bm->first_box) && (bm->current_layer == bm->top_layer)) {
				/* If recruit outside model area and in first dynamic box then update
				 the Migration array */
                mid = EMBRYO[species].migIDmatch[qid];
                recruit_outside = 1;
                if (mid < 0) { // See AlignVsMigration() for how this is set
                    recruit_outside = 0;
                }
                
                        
                /**
                thisday = EMBRYO[species].StartDay[ngene][qid];

                if ((species == 64) || (species == 69)) {
                    fprintf(bm->logFile, "Time: %e %s ngene: %d mid: %d thisday: %d Start: %d Return: %d recruit_outside: %d\n",
                            bm->dayt, FunctGroupArray[species].groupCode, ngene, mid, thisday, MIGRATION[species].Leave_Now[mid], MIGRATION[species].Return_Now[mid], recruit_outside);
                }
                **/
                
                if (recruit_outside) {
                    //oldDEN = MIGRATION[species].DEN[ngene][mid];
                    //oldSN = MIGRATION[species].SN[ngene][mid];
                    //oldRN = MIGRATION[species].RN[ngene][mid];

                    if(EMBRYO[species].BulkRecruits[ngene] * enviro_scalar > 0.0) {
                        //MIGRATION[species].DEN[ngene][mid] += EMBRYO[species].BulkRecruits[ngene] * enviro_scalar; // As all in youngest cohort maps to ngene not cohort in Migration
                        MIGRATION[species].recruit[ngene][mid] = EMBRYO[species].BulkRecruits[ngene] * enviro_scalar;
                        FunctGroupArray[species].speciesParams[direct_recruit_entry_id] = 1;

                        // Update size here too - used to reset to KMIG each year, but that is erroneous
                        //MIGRATION[species].SN[ngene][mid] = (oldSN * oldDEN + KWSR_sp *	EMBRYO[species].BulkRecruits[ngene] * enviro_scalar) / (EMBRYO[species].BulkRecruits[ngene] * enviro_scalar + oldDEN + small_num);
                        //MIGRATION[species].RN[ngene][mid] = (oldRN * oldDEN + KWRR_sp *	EMBRYO[species].BulkRecruits[ngene] * enviro_scalar) / (EMBRYO[species].BulkRecruits[ngene] * enviro_scalar + oldDEN + small_num);
                        
                        /**
                        //if ((species == 64) || (species == 69)) {
                        if (species == 52) {
                            fprintf(bm->logFile, "Time: %e %s ngene: %d mid: %d MIGRATIONrecruit: %e BulkRecruits: %e enviro_scalar: %e\n",
                                bm->dayt, FunctGroupArray[species].groupCode, ngene, mid, MIGRATION[species].recruit[ngene][mid], EMBRYO[species].BulkRecruits[ngene], enviro_scalar);
                        }
                        **/
                    }
                } else {
                    MIGRATION[species].recruit[ngene][mid] = 0;
                    EMBRYO[species].num_recruits[bm->current_box][wclayer][ngene][qid] = EMBRYO[species].BulkRecruits[ngene] * enviro_scalar * vertdistrib * bm->recruit_hdistrib[ngene][bm->current_box][species];
                    totrecruit[species][stock_id][ngene] += EMBRYO[species].num_recruits[bm->current_box][wclayer][ngene][qid];
                    
                    /**
                    //if ((species == 64) || (species == 69)) {
                        fprintf(bm->logFile, "Time: %e %s ngene: %d qid: %d box: %d-%d num_recruit: %e BulkRecruits: %e enviro_scalar: %e vertdistrib: %e hdistrib: %e\n",
                            bm->dayt, FunctGroupArray[species].groupCode, ngene, qid, bm->current_box, wclayer, EMBRYO[species].num_recruits[bm->current_box][wclayer][ngene][qid], EMBRYO[species].BulkRecruits[ngene], enviro_scalar, vertdistrib, bm->recruit_hdistrib[ngene][bm->current_box][species]);
                    }
                    **/
                }
				totrecruit[species][stock_id][ngene] = 0.0;
			}
			break;
		case larval_dispersal:
			EMBRYO[species].num_recruits[bm->current_box][wclayer][ngene][qid] = EMBRYO[species].BulkRecruits[ngene] * enviro_scalar * vertdistrib * bm->larvaldistrib[bm->current_box][species];
			totrecruit[species][stock_id][ngene] += EMBRYO[species].num_recruits[bm->current_box][wclayer][ngene][qid];

			/* Takes larvae and distributes based on read in time series of distributions (updated in physical library)
			 num_recruits[bm->current_box][wclayer][species] = BulkRecruits * bm->larvaldistrib[bm->current_box][wclayer][ngene][d]

			 where BulkRecruits = TotSpawn * array_survivourship (conditioned on plankton and time in watercolumn to get from site A to B)
			 and larvaldistrib is proportion of spawn starting at site A that reaches site B after recruit period of time

			 For expanded case of larval_dispersal don't sum over layers and boxes to get BulkRecruits,
			 do num_recruits iterative allocation at time calc TotSpawn

			 May also want to include super expanded case where have Larvae as life history stage
			 and explicitly consider mortality and growth at each time step (so predation and
			 temperature dependent mortality explicitly represented)

			 totrecruit[species][stock_id] += num_recruits[bm->current_box][wclayer][ngene][latest_in_larval_queue] for the species;
			 */
			break;
		default:
			quit("No such recruitType defined for vertebrates (%d) - value must be between 0 and 3 currently\n", FunctGroupArray[species].recruitType);
			break;
		}

        EMBRYO[species].num_recruits_updating[bm->current_box][wclayer][ngene][qid] = EMBRYO[species].num_recruits[bm->current_box][wclayer][ngene][qid];
        
        
        /* Now find the contaminant content */
        if(bm->track_contaminants && bm->flag_contamMaternalTransfer){
            Set_Recruit_Final_Contaminants(bm, wclayer, species, ngene, mid, qid, recruit_outside);
        }
        
        /**
        //if (do_debug && (bm->which_check == species)) {
        //if (species == 33) {
        //if (species == 52) {
        if (species < 6) {
            fprintf(llogfp, "Time: %e %s-%d box %d-%d totrecruit: %e, num_recruit: %e, qid: %d recruitType: %d\n", bm->dayt, FunctGroupArray[species].groupCode, ngene, bm->current_box, wclayer, totrecruit[species][stock_id][ngene], EMBRYO[species].num_recruits[bm->current_box][wclayer][ngene][qid], qid, FunctGroupArray[species].recruitType);
		}
        **/
	}

	return;
}

/*
 * \brief Helper routine to determines vertical position in the water column
 *
 */

double Get_Vertical_Recruit_Position(int species, int wclayer, int maxdeep, int totaldeep) {
	double sumnzj1, sumnzj2;
	int layerk, diffdeep;
	int flagdem = (int) (FunctGroupArray[species].speciesParams[flagdem_id]);
	int k = 0;

	double vertdistrib = 1.0;

	/* If considering a vertebrate group in an area that is too shallow to have
	 all the water column layers present rescale */
	if (totaldeep < maxdeep) {
		diffdeep = maxdeep - totaldeep;
		sumnzj1 = 0.0;
		sumnzj2 = 0.0;
		for ( k = 0; k < totaldeep; k++) {
			layerk = k + diffdeep;
			sumnzj1 += recruit_vdistrib[layerk][species];
			sumnzj2 += recruit_vdistrib[k][species];
		}
		if (sumnzj1 < sumnzj2) {
			vertdistrib = recruit_vdistrib[wclayer][species] / sumnzj2;
		} else if (!sumnzj1) {
			if (flagdem && (wclayer == 0)) {
				/* If demersal and on the bottom */
				vertdistrib = 1;
			} else if (!flagdem && (wclayer == totaldeep - 1)) {
				/* If pelagic and at the surface */
				vertdistrib = 1;
			} else
				vertdistrib = 0;
		} else {
			layerk = wclayer + diffdeep;
			vertdistrib = recruit_vdistrib[layerk][species] / sumnzj1;
		}
	} else
		vertdistrib = recruit_vdistrib[wclayer][species];

	return vertdistrib;
}

/**
 * \brief Sanity check the numbers beingreturned - especially for linear recruitment
 *
 */
void Sanity_Check_Recruits(MSEBoxModel *bm, int species, FILE *llogfp){
    int i, ij, k, n, ngene;
    double scalar, KDENR_sp;
    int recruit_sp = (int) (FunctGroupArray[species].speciesParams[flagrecruit_id]);
    double tot_adult_den = 0.0;
    int maxstock_id = FunctGroupArray[species].numStocks;
    int sp_numGeneTypes = (int) (FunctGroupArray[species].numGeneTypes);
    int qid = EMBRYO[species].next_larvae;
    
    if (recruit_sp == fixed_linear_recruit) {
        for(ij=0; ij < bm->nbox; ij++){
            if(bm->boxes[ij].type != BOUNDARY) {
                for (k = 0; k < bm->boxes[ij].nz; k++) {
                    for (n = 0; n < FunctGroupArray[species].numCohorts; n++) {
                        tot_adult_den += FunctGroupArray[species].scaled_FSPB[n] * bm->boxes[ij].tr[k][FunctGroupArray[species].NumsTracers[n]];
                    }
                }
            }
        }
        
        for ( i = 0; i < maxstock_id; i++) {
            for(ngene = 0; ngene < sp_numGeneTypes; ngene++){
                KDENR_sp = KDENR[species][i];
                scalar = KDENR_sp;
                if (KDENR_sp < 1.0)
                    scalar = 1.0;
                if (EMBRYO[species].Larvae[i][ngene][qid] > (tot_adult_den * scalar)) {
                    fprintf(llogfp, "Trimming %s offspring as have too many infants per adults - %e larvae vs %e reproductive adults (tot_adult_den: %e scalar: %e) ", FunctGroupArray[species].groupCode, EMBRYO[species].Larvae[i][ngene][qid], (tot_adult_den * scalar), tot_adult_den, scalar);
                    EMBRYO[species].Larvae[i][ngene][qid] = (tot_adult_den * scalar);
                    fprintf(llogfp,"- now setting infants to %e\n", EMBRYO[species].Larvae[i][ngene][qid]);
                }
            }
        }
    }
    
    return;
}

/**
 * \brief Store diagnostics to do with recruitment or estimating natural mortality
 *
 */
void Store_Recruitment_Diagnostics(MSEBoxModel *bm, int species, int use_aggregate, int do_debug, FILE *llogfp) {
	int pred = 0;
	int qid = EMBRYO[species].next_larvae;
	int maxstock_id = FunctGroupArray[species].numStocks;
    //double yoysum = 0.0;
    double biomass, chrt_biomass;
	double KWSR_sp = FunctGroupArray[species].speciesParams[KWSR_id];
	double KWRR_sp = FunctGroupArray[species].speciesParams[KWRR_id];
	int sp_numGeneTypes = (int) (FunctGroupArray[species].numGeneTypes);
	int ngene = 0, i = 0, stock_id = 0, den, k, mid, thisday, recruit_outside = 0;
    //int stage;
	int fished_chrt = 0;
    
    Util_Init_1D_Double(yoy, bm->K_num_max_genetypes, 0.0);

    /* yoy = young of the year */
	for ( i = 0; i < maxstock_id; i++) {
		for(ngene = 0; ngene < sp_numGeneTypes; ngene++){
            if (bm->flag_modify_KWSR) {
                KWSR_sp = Get_Recruit_Size_Forcing(bm, species, do_debug, llogfp);
                KWRR_sp = KWSR_sp * FunctGroupArray[species].X_RS[ngene];
            }
            
			//yoysum += EMBRYO[species].Larvae[i][ngene][qid];
			yoy[ngene] += EMBRYO[species].Larvae[i][ngene][qid];
			if (use_aggregate) {
                tot_yoy[species][i] = (totrecruit[species][i][ngene] * (KWRR_sp + KWSR_sp));
			} else {
				tot_yoy[species][i] = (EMBRYO[species].Larvae[i][ngene][qid] * (KWRR_sp + KWSR_sp));
			}
            
            if(FunctGroupArray[species].externalReproducer){
                for (mid = MIGRATION[species].num_in_queue_done; mid < MIGRATION[species].num_in_queue; mid++) {
                    recruit_outside = 0;
                        
                    thisday = EMBRYO[species].StartDay[ngene][EMBRYO[species].next_recruit];
                    //stage = FunctGroupArray[species].cohort_stage[ngene];
                    
                    /* Outside the model when recruitment happens */
                    if ((MIGRATION[species].Return_Now[mid] >= thisday) && (MIGRATION[species].Leave_Now[mid] <= thisday)) {
                        recruit_outside = 1;
                    }
                        
                    if (recruit_outside) {
                        tot_yoy[species][i] += MIGRATION[species].recruit[ngene][mid] * (KWRR_sp + KWSR_sp);
                    }
                }
            }
		}
 	}

	for(ngene = 0; ngene < sp_numGeneTypes; ngene++){
        if (bm->flag_modify_KWSR) {
            KWSR_sp = Get_Recruit_Size_Forcing(bm, species, do_debug, llogfp);
            KWRR_sp = KWSR_sp * FunctGroupArray[species].X_RS[ngene];
        }

		fprintf(llogfp, "At %e species %s genotype %d has %e YOY (biom = %e) which should arrive %d to %d\n", bm->dayt, FunctGroupArray[species].groupCode, ngene, yoy[ngene], yoy[ngene] * (KWRR_sp + KWSR_sp) * bm->X_CN * mg_2_tonne, EMBRYO[species].StartDay[ngene][EMBRYO[species].next_recruit], EMBRYO[species].EndDay[ngene][EMBRYO[species].next_recruit]);
	}

	/* Update Pseudo assessment estimates */
	fished_chrt = (int) (FunctGroupArray[species].speciesParams[Age95pcntV_id]);
	if (fished_chrt < (FunctGroupArray[species].numCohorts / 2))
		fished_chrt = (FunctGroupArray[species].numCohorts / 2);

	/* If the fished cohort for this group is the first cohort then we want to
	 * count the young of year (yoy) as the start numbers
	 * Don't think this can ever happen due to check above!
	 */

	if (fished_chrt < sp_numGeneTypes) {
		/* Store last year's records in case they're needed in the assessment */
		bm->calcMnum[species][hist_id] = bm->calcMnum[species][expect_id];
		bm->calcFnum[species][hist_id] = bm->calcFnum[species][expect_id];
		bm->calcNstart[species][hist_id] = bm->calcNstart[species][expect_id];
		FunctGroupArray[species].speciesParams[calcupdate_date_id] = bm->TofY;
		FunctGroupArray[species].speciesParams[calcPerPredupdate_date_id] = bm->TofY;

		bm->calcNstartPerPred[species][hist_id] = bm->calcNstartPerPred[species][expect_id];
		for (pred = 0; pred < bm->K_num_tot_sp; pred++) {
			bm->calcMnumPerPred[species][pred][hist_id] = bm->calcMnumPerPred[species][pred][expect_id];
			bm->calcMnumPerPred[species][pred][expect_id] = 0;
		}

		/* Update the linear mortality, quad mortality and mortality due to explict birds and mammals */
		bm->calcMLinearMort[species][hist_id] = bm->calcMLinearMort[species][expect_id];
		bm->calcMLinearMort[species][expect_id] = 0;

		bm->calcMQuadMort[species][hist_id] = bm->calcMQuadMort[species][expect_id];
		bm->calcMQuadMort[species][expect_id] = 0;

		bm->calcMPredMort[species][hist_id] = bm->calcMPredMort[species][expect_id];
		bm->calcMPredMort[species][expect_id] = 0;

		/* Initialise mortality trackers for pseudo assessment */
		bm->calcMnum[species][expect_id] = 0;
		bm->calcFnum[species][expect_id] = 0;

		/* Set starting cohort size for pseduo assessment - Zero for update below (otherwise done in recruitment) */
		bm->calcNstart[species][expect_id] = 0;

		//fprintf(llogfp, "%s set startN to %e\n", FunctGroupArray[species].groupCode, yoysum);
	}

	// Now initialise the calcN and starting numbers in bm->calcTrackedMort
	biomass = 0.0;
	for(ngene = 0; ngene < sp_numGeneTypes; ngene++){

		if(bm->track_contaminants ){
			Contaminant_Update_ContactMort_Record(bm, species, ngene);
		}

		for (k = 0; k < maxstock_id; k++) {
			/* Sort out the mortality estimates and initialise for next year */
			bm->calcTrackedMort[species][ngene][k][finalM1_id] = bm->calcTrackedMort[species][ngene][k][ongoingM1_id] / (bm->calcTrackedMort[species][ngene][k][start_id] + small_num);
			bm->calcTrackedMort[species][ngene][k][finalM2_id] = bm->calcTrackedMort[species][ngene][k][ongoingM2_id] / (bm->calcTrackedMort[species][ngene][k][start_id] + small_num);
			bm->calcTrackedMort[species][ngene][k][finalF_id] = bm->calcTrackedMort[species][ngene][k][ongoingF_id] / (bm->calcTrackedMort[species][ngene][k][start_id] + small_num);

			for(pred=0; pred < bm->K_num_tot_sp; pred++){
				bm->calcTrackedPredMort[species][ngene][k][pred][final_id] = bm->calcTrackedPredMort[species][ngene][k][pred][ongoing_id] / (bm->calcTrackedMort[species][ngene][k][start_id] + small_num);
				bm->calcTrackedPredMort[species][ngene][k][pred][ongoing_id] = 0.0;
			}

			bm->calcTrackedMort[species][ngene][k][ongoingM1_id] = 0.0;
			bm->calcTrackedMort[species][ngene][k][ongoingM2_id] = 0.0;
			bm->calcTrackedMort[species][ngene][k][ongoingF_id] = 0.0;

			bm->calcTrackedMort[species][ngene][k][start_id] = 0.0;
            
            if (!k) {
                FunctGroupArray[species].min_B[ngene] = MAXDOUBLE;
                FunctGroupArray[species].max_B[ngene] = MINDOUBLE;
                
                if(FunctGroupArray[species].groupAgeType == AGE_STRUCTURED) {
                    FunctGroupArray[species].min_wgt[ngene] = MAXDOUBLE;
                    FunctGroupArray[species].max_wgt[ngene] = MINDOUBLE;
                }
            }
		}
		// Now initialise the calcN and starting numbers in bm->calcTrackedMort
		if (FunctGroupArray[species].groupAgeType == AGE_STRUCTURED) {
			den = FunctGroupArray[species].NumsTracers[ngene];
			for (i = 0; i < bm->nbox; i++) {
				if (bm->boxes[i].type != BOUNDARY) {
					for (k = 0; k < bm->boxes[i].nz; k++) {
						if (fished_chrt == ngene )
							biomass += bm->boxes[i].tr[k][den];
						stock_id = bm->group_stock[species][i][k];

						bm->calcTrackedMort[species][ngene][stock_id][start_id] += bm->boxes[i].tr[k][den];
						biomass += bm->boxes[i].tr[k][den];
					}
				}
			}
			for (i = 0; i < maxstock_id; i++) {
				bm->calcTrackedMort[species][ngene][i][start_id] += EMBRYO[species].Larvae[i][ngene][qid];
				biomass += EMBRYO[species].Larvae[i][ngene][qid];
			}
		} else {
			den = FunctGroupArray[species].totNTracers[ngene];
			for (i = 0; i < bm->nbox; i++) {
				// Loop through each pelagic layer
				for (k = 0; k < bm->boxes[i].nz; k++) {
					stock_id = bm->group_stock[species][i][k];
					biomass += bm->boxes[i].tr[k][den] * bm->boxes[i].dz[k] * bm->boxes[i].area * FunctGroupArray[species].habitatCoeffs[WC];
					chrt_biomass = bm->boxes[i].tr[k][den] * bm->boxes[i].dz[k] * bm->boxes[i].area * FunctGroupArray[species].habitatCoeffs[WC];
					bm->calcTrackedMort[species][ngene][stock_id][start_id] += chrt_biomass;
				}
				// Get stocks based on assuming dealing with deepest water column layer
				// layer 0
				stock_id = bm->group_stock[species][i][0];
				// Loop through each sediment layer
				for (k = 0; k < bm->boxes[i].sm.nz; k++) {
					biomass += bm->boxes[i].sm.tr[k][den] * bm->boxes[i].sm.dz[k] * bm->boxes[i].area * FunctGroupArray[species].habitatCoeffs[SED];
					chrt_biomass = bm->boxes[i].sm.tr[k][den] * bm->boxes[i].sm.dz[k] * bm->boxes[i].area * FunctGroupArray[species].habitatCoeffs[SED];
					bm->calcTrackedMort[species][ngene][stock_id][start_id] += chrt_biomass;
				}
				// Loop over epibenthic layer
				if (FunctGroupArray[species].habitatCoeffs[EPIFAUNA] > 0) {
					biomass += bm->boxes[i].epi[den] * bm->boxes[i].area * FunctGroupArray[species].habitatCoeffs[EPIFAUNA];
					chrt_biomass = bm->boxes[i].epi[den] * bm->boxes[i].area * FunctGroupArray[species].habitatCoeffs[EPIFAUNA];
					bm->calcTrackedMort[species][ngene][stock_id][start_id] += chrt_biomass;
				}
			}
		}
	}
	/* Set starting cohort size for pseduo assessment */
	bm->calcNstart[species][expect_id] = biomass;
	bm->calcNstartPerPred[species][expect_id] = biomass;
    
    // Mark diet updated
    FunctGroupArray[species].updatedDiet = 1;

	return;

}

/**
 * \brief Get any settlers coming in on the day
 *
 * Note: expect_id is the zeroth entry in the array, so top of the queue
 */
void Get_Settlers(MSEBoxModel *bm, int species, int wclayer, int stock_id, int *recruits_arrive, int do_debug, FILE *llogfp) {
	double Dayj = 0.0;
	double temprec = 1.0;
	double step1 = 0.0;
    double recruits_coming = 0.0;
	int sp_numGeneTypes = (int) (FunctGroupArray[species].numGeneTypes);
    int ngene;
    int this_ID = 0;
    //int do_debug2;
	int sp_checkday = (int)(bm->dayt);
    *recruits_arrive = 0;
    
    if(verbose) {
        printf("Doing Get_Settlers for %s\n", FunctGroupArray[species].groupCode);
    }
    
	/* If recruit within the same year do so now */
	for (ngene = 0; ngene < sp_numGeneTypes; ngene ++) {
        /**
        //if (do_debug && (bm->which_check == species) && (bm->dayt >= bm->checkstart)) {
        if ((species == 33) || (species == 32)) {
            fprintf(llogfp, "Time: %e, %s get settlers ngene: %d, next-recruit: %d StartDay: %d, EndDay: %d, recruit sp_checkday: %d larvae starts: %e\n",
					bm->dayt, FunctGroupArray[species].groupCode, ngene, EMBRYO[species].next_recruit, EMBRYO[species].StartDay[ngene][EMBRYO[species].next_recruit], EMBRYO[species].EndDay[ngene][EMBRYO[species].next_recruit], sp_checkday, EMBRYO[species].num_recruits_updating[bm->current_box][wclayer][ngene][EMBRYO[species].next_recruit]);
		}
        **/
    
		if ((sp_checkday >= EMBRYO[species].StartDay[ngene][EMBRYO[species].next_recruit]) && (sp_checkday < EMBRYO[species].EndDay[ngene][EMBRYO[species].next_recruit])) {
			Dayj = bm->dayt - EMBRYO[species].StartDay[ngene][EMBRYO[species].next_recruit];
            
            /* Flat recruitment distribution through recruitment period */
            if (flagtrecruitdistrib) {
                recruits_coming = EMBRYO[species].num_recruits[bm->current_box][wclayer][ngene][EMBRYO[species].next_recruit] / EMBRYO[species].RecruitPeriod[EMBRYO[species].next_recruit];
                temprec = 1.0;

                /**
                //if (do_debug && (bm->which_check == species)) {
                 if ((species == 33) || (species == 32)) {
                    fprintf(llogfp, "Time: %e, get settlers %s box%d wclayer: %d flagt case num_recruits: %e, period: %e, recruits_coming: %e\n", bm->dayt, FunctGroupArray[species].groupCode, bm->current_box, wclayer, EMBRYO[species].num_recruits[bm->current_box][wclayer][ngene][EMBRYO[species].next_recruit], EMBRYO[species].RecruitPeriod[EMBRYO[species].next_recruit], recruits_coming);
                }
                **/
            } else {
                /* Humped recruitment distribution through recruitment period */
                step1 = Dayj / EMBRYO[species].RecruitPeriod[EMBRYO[species].next_recruit];
                temprec = Util_Lognorm_Distrib(rec_m, rec_sigma, step1);
                recruits_coming = EMBRYO[species].num_recruits[bm->current_box][wclayer][ngene][EMBRYO[species].next_recruit] * temprec;

                /**
                //if (do_debug && (bm->which_check == species)) {
                if ((species == 33) || (species == 32)) {
                    fprintf(llogfp, "Time: %e, get settlers %s humped case step1: %e, temprec: %e, rec_m: %e, rec_sigma: %e, num_recruits: %e, recruitSPden: %e\n", bm->dayt, FunctGroupArray[species].groupCode, step1, temprec, rec_m, rec_sigma, EMBRYO[species].num_recruits[bm->current_box][wclayer][ngene][EMBRYO[species].next_recruit], EMBRYO[species].recruitSPden[ngene]);
                }
                **/
            }
            
            if (FunctGroupArray[species].recruitType == external_recruit) {
                /* Recruiting into te MIGRATION array */
                this_ID = EMBRYO[species].migIDmatch[EMBRYO[species].next_recruit];
                MIGRATION[species].recruit[ngene][this_ID] = recruits_coming;
                FunctGroupArray[species].speciesParams[direct_recruit_entry_id] = 1;
                
                /**
                if (species == 33) {
                fprintf(llogfp, "Time: %e, get settlers %s box%d wclayer: %d num_recruits: %e, period: %e, recruits_coming: %e MIGRATION.recruit[this_ID: %d]: %e\n", bm->dayt, FunctGroupArray[species].groupCode, bm->current_box, wclayer, EMBRYO[species].num_recruits[bm->current_box][wclayer][ngene][EMBRYO[species].next_recruit], EMBRYO[species].RecruitPeriod[EMBRYO[species].next_recruit], recruits_coming, EMBRYO[species].recruitSPden[ngene], this_ID, MIGRATION[species].recruit[ngene][this_ID]);
                }
                **/

            } else {
                /* Recruiting within the model domain */
                EMBRYO[species].recruitSPden[ngene] = recruits_coming;
                
                /**
                if (species == 33) {
                    fprintf(llogfp, "Time: %e, get settlers %s box%d wclayer: %d num_recruits: %e, period: %e, recruits_coming: %e recruitSPden: %e\n", bm->dayt, FunctGroupArray[species].groupCode, bm->current_box, wclayer, EMBRYO[species].num_recruits[bm->current_box][wclayer][ngene][EMBRYO[species].next_recruit], EMBRYO[species].RecruitPeriod[EMBRYO[species].next_recruit], recruits_coming, EMBRYO[species].recruitSPden[ngene]);
                }
               **/
            }

            /* Update numbers of recruits yet to settle */
            if (EMBRYO[species].recruitSPden[ngene] > 0) {
                //EMBRYO[species].Larvae[stock_id][ngene][EMBRYO[species].next_recruit] -= recruits_coming;
                EMBRYO[species].num_recruits_updating[bm->current_box][wclayer][ngene][EMBRYO[species].next_recruit] -= recruits_coming;
                *recruits_arrive = 1;
                
                //fprintf(llogfp, "Time: %e, %s box%d-%d stock %d ngene %d next_reruit: %d setting remaining Larvae: %e\n", bm->dayt, FunctGroupArray[species].groupCode, bm->current_box, wclayer, stock_id, ngene, EMBRYO[species].next_recruit, EMBRYO[species].num_recruits_updating[bm->current_box][wclayer][ngene][EMBRYO[species].next_recruit]);

            }

            /**
            //if (do_debug && (bm->which_check == species)) {
            if ((species == 33) || (species == 32)) {
                fprintf(llogfp, "Time: %e, %s ngene: %d, qid (next_recruit): %d StartDay: %d, EndDay: %d, sp_checkday: %d, recruitSPden: %e, remaining larvae: %e, num_recruits%d-%d: %e, temprec: %e, RecruitPeriod: %e, flagtrecruitdistrib: %d\n", bm->dayt, FunctGroupArray[species].groupCode, ngene, EMBRYO[species].next_recruit, EMBRYO[species].StartDay[ngene][EMBRYO[species].next_recruit], EMBRYO[species].EndDay[ngene][EMBRYO[species].next_recruit], sp_checkday, EMBRYO[species].recruitSPden[ngene], EMBRYO[species].num_recruits_updating[bm->current_box][wclayer][ngene][EMBRYO[species].next_recruit], bm->current_box, wclayer, EMBRYO[species].num_recruits[bm->current_box][wclayer][ngene][EMBRYO[species].next_recruit], temprec, EMBRYO[species].RecruitPeriod[EMBRYO[species].next_recruit], flagtrecruitdistrib);
            }
            **/
            
            if (EMBRYO[species].num_recruits_updating[bm->current_box][wclayer][ngene][EMBRYO[species].next_recruit] < 0.0) {
                    
                /**
                //if (do_debug && (bm->which_check == species)) {
                if ((species == 33) || (species == 32)) {
                    fprintf(llogfp,"Time: %e %s stock: %d, ngene: %d, out of larvae in box%d-%d as larvae : %e so larvae RESET\n", bm->dayt, FunctGroupArray[species].groupCode, stock_id, ngene, bm->current_box, bm->current_layer, EMBRYO[species].num_recruits_updating[bm->current_box][wclayer][ngene][EMBRYO[species].next_recruit]);
                }
                **/
                
                /** Reset EMBRYO information and finalise settling**/
                EMBRYO[species].num_recruits_updating[bm->current_box][wclayer][ngene][EMBRYO[species].next_recruit] = 0.0;
                EMBRYO[species].EndDay[ngene][EMBRYO[species].next_recruit] = bm->dayt + 1;
                
            }

            if (FunctGroupArray[species].speciesParams[firstrecday_id]) {
                EMBRYO[species].recruiting_now = 1;
            }
                
		} else {
			FunctGroupArray[species].speciesParams[firstrecday_id] = 1;
			EMBRYO[species].recruitSPden[ngene] = 0.0;

            /**
			if (do_debug && (bm->which_check == species)) {
				fprintf(llogfp,"Time: %e %s has no recruits as out of session\n", bm->dayt, FunctGroupArray[species].groupCode);
			}
            **/

		}
        
        if(bm->track_contaminants && bm->flag_contamMaternalTransfer){
            Get_Settler_Contaminants(bm, wclayer, species, ngene, EMBRYO[species].next_recruit, EMBRYO[species].recruitSPden[ngene]);
        }

	}
	return;
}

/****************************************************************************************************************************
 Age related helper routines
 */

/**
 * \brief Initialising age related arrays
 *
 */
void Init_Aging_Arrays(MSEBoxModel *bm, int species, int stock_id, int maxstock_id, FILE *llogfp) {
	int cohort, i, nc;
	int sp_ddepend_move = (int)(FunctGroupArray[species].speciesParams[ddepend_move_id]);
	int sp_AgeClassSize = FunctGroupArray[species].ageClassSize;
    int qid;
    
    /*
    if(verbose)
        printf(" Doing Init_Aging_Arrays - %s current_box %d first_box %d current_layer %d top_layer %d\n", FunctGroupArray[species].groupCode, bm->current_box, bm->first_box, bm->current_layer, bm->top_layer);
    */
    
    /* Initialise aging arrays - once if no site attachment, per box if there is site_attachment */
	if ((bm->current_box == bm->first_box) && (bm->current_layer == bm->top_layer)) {
		for (cohort = 0; cohort < FunctGroupArray[species].numCohortsXnumGenes; cohort++) {
			for (i = 0; i < maxstock_id; i++) {
				if (FunctGroupArray[species].groupAgeType == AGE_STRUCTURED) {
					FunctGroupArray[species].agingVERT[cohort][i][DEN_id] = 0.0;
					FunctGroupArray[species].agingVERT[cohort][i][SN_id] = 0.0;
					FunctGroupArray[species].agingVERT[cohort][i][RN_id] = 0.0;
                    
                    /*
                     if (species == 22) {
                        fprintf(llogfp,"Time: %e %s-%d stock %d agingVERT zeroed\n", bm->dayt, FunctGroupArray[species].groupCode, cohort, i);
                    }
                     */
				}
                
				if (sp_ddepend_move != 1) {
					for (nc = 0; nc < sp_AgeClassSize; nc++) {
						bm->tempPopRatio[i][species][cohort][nc] = 0.0;
					}
				}
			}
            
           for (qid = MIGRATION[species].num_in_queue_done; qid < MIGRATION[species].num_in_queue; qid++) {
                MIGRATION[species].aging[cohort][qid] = 0.0;
            }
		}

		/* Prepare for update of age distributions */
		if (bm->flagagestruct) {
			Prepare_Age_Distrib(bm, species, llogfp);
		}
	}

	if ((sp_ddepend_move == 1) && (bm->current_layer == bm->boxes[bm->current_box].nz - 1)) {
		for (cohort = 0; cohort < FunctGroupArray[species].numCohortsXnumGenes; cohort++) {
			for (nc = 0; nc < sp_AgeClassSize; nc++) {
				bm->tempPopRatio[stock_id][species][cohort][nc] = 0.0;
			}
		}
	}
	return;
}

/**
 * \brief Get the proportion of the age class aging into the next age class
 * For groups where the age class represents > 1 year then this need not be = 1.0
 */
double Get_Proportion_Aging(MSEBoxModel *bm, int species, int cohort, int do_debug, FILE *llogfp) {
	int i = 0;
	double p_ageup = 1.0;
	double sp_AgeSize = (double)(FunctGroupArray[species].ageClassSize);

	/* Proportion to age on, uniform distribution case */
	if (!bm->flagagestruct) {
		p_ageup = 1.0 / sp_AgeSize;
		if (p_ageup > 1.0)
			p_ageup = 1.0;
        
	} else {
		/* If non-homogeneous distribution, determine proportion to age-up */
		i = (int) (sp_AgeSize - 1);
		if (i < 0)
			i = 0;
		p_ageup = FunctGroupArray[species].boxPopRatio[bm->current_box][bm->current_layer][cohort][i];

		if((FunctGroupArray[species].groupType == CORAL) || (FunctGroupArray[species].groupType == SPONGE)){
			p_ageup += Coral_Variable_Transitions(bm, species, cohort, do_debug, llogfp);
		}

        /**
        if (do_debug && (bm->which_check == species)) {
            fprintf(llogfp, "Time: %e p_ageup: %e sp: %s cohort: %d boxPopRatio: %e sp_AgeSize: %e\n", bm->dayt, p_ageup, FunctGroupArray[species].groupCode, cohort, FunctGroupArray[species].boxPopRatio[bm->current_box][bm->current_layer][cohort][i], sp_AgeSize);
        **/
        
		if ((p_ageup < 0) || (!_finite(p_ageup)) || (p_ageup > 1)){
			fprintf(llogfp, "p_ageup: %e, sp: %s, cohort: %d, i: %d is IMPOSSIBLE?! (boxPopRatio[%d][%d][%d][%d]: %e)\n", p_ageup, FunctGroupArray[species].groupCode, cohort, i, bm->current_box, bm->current_layer, cohort, i, FunctGroupArray[species].boxPopRatio[bm->current_box][bm->current_layer][cohort][i]);
			quit("p_ageup: %e, sp: %s, cohort: %d, i: %d is IMPOSSIBLE?!  (boxPopRatio[%d][%d][%d][%d]: %e)\n", p_ageup, FunctGroupArray[species].groupCode, cohort, i,
					bm->current_box, bm->current_layer, cohort, i, FunctGroupArray[species].boxPopRatio[bm->current_box][bm->current_layer][cohort][i]);
		}
	}

	return p_ageup;
}

/**
 * \brief Get the numbers shifting age classes
 */
double Get_Numbers_Aging(MSEBoxModel *bm, int species, int cohort, int *active_den, double *lostden_zero, double *this_p_ageup, int do_debug, FILE *llogfp) {
	double p_ageup = 0;
	double dennow = 0.0;
	double num_aging = 0.0;
	double lostden = 0.0;
	int ng;
	int sp_numGeneTypes = (int)(FunctGroupArray[species].numGeneTypes);
	int basecohort = (int)floor (cohort / sp_numGeneTypes);
	ng = cohort - basecohort;

	/* Find numbers shifting and shift numbers lost - ones added done in aging update */
	if (bm->flag_do_evolution && bm->flagagestruct) {
		// Number of individuals aging out of the age-gene bin in total
		num_aging = DNA[species].phenotype_aging_up[cohort];
		shiftVERT[species][cohort][DEN_id] = VERTinfo[species][cohort][DEN_id] - num_aging;

		/* Store numbers lost so can be used in calculate size after new recruits arrive */
		lostden = num_aging;
		if (cohort < sp_numGeneTypes)
			lostden_zero[cohort] = num_aging;

        /**
        //if (species == 22) {
		if (do_debug && (bm->which_check == species)) {
			fprintf(llogfp, "Time: %e, %s-%d shiftVERT[den]: %e, num_aging: %e, VERTinfo[den]: %e\n", bm->dayt,
					FunctGroupArray[species].groupCode, cohort, shiftVERT[species][cohort][DEN_id], num_aging, VERTinfo[species][cohort][DEN_id]);
		}
        **/
        
		// Find number aging into the age-gene bin (well it will be age+1 but the +1 transition is done in Update_Aging_Numbers() to be consistent with non-evolution case
		dennow = 0.0;
		for (ng = 0; ng < FunctGroupArray[species].numGeneTypes; ng++){
			dennow += DNA[species].phenotype_transition[cohort][ng];

            /**
            if (do_debug && (bm->which_check == species)) {
            //if (species == 22) {
				fprintf(llogfp,"Time: %e box%d-%d %s-%d, ng %d has dennow: %e, phenotype_transition: %e\n",
						bm->dayt, bm->current_box, bm->current_layer, FunctGroupArray[species].groupCode, cohort, ng, dennow, DNA[species].phenotype_transition[cohort][ng]);
            }
            **/
		}

	} else {
		p_ageup = Get_Proportion_Aging(bm, species, cohort, do_debug, llogfp);
		*this_p_ageup = p_ageup;
		dennow = p_ageup * VERTinfo[species][cohort][DEN_id];
		shiftVERT[species][cohort][DEN_id] = VERTinfo[species][cohort][DEN_id] - dennow;

		/* Store numbers lost so can be used in calculate size after new recruits arrive */
		lostden = dennow;
		if (cohort < sp_numGeneTypes)
			lostden_zero[cohort] = lostden;

        /**
		if (do_debug && (bm->which_check == species)) {
			fprintf(llogfp, "Time: %e, %s-%d box %d shiftVERT[den]: %e, p_ageup: %e, VERTinfo[den]: %e (shiftVERTON: %d, dennow: %e)\n", bm->dayt,
                    FunctGroupArray[species].groupCode, cohort, bm->current_box, shiftVERT[species][cohort][DEN_id], p_ageup, VERTinfo[species][cohort][DEN_id], shiftVERTON[species][cohort], dennow);
		}
        **/

	}

    /**
	if (do_debug && (bm->which_check == species)) {
		fprintf(llogfp, "Time: %e, %s-%d shiftVERT[den]: %e, p_ageup: %e, VERTinfo[den]: %e (shiftVERTON: %d)\n", bm->dayt,
				FunctGroupArray[species].groupCode, cohort, shiftVERT[species][cohort][DEN_id], p_ageup, VERTinfo[species][cohort][DEN_id],
				shiftVERTON[species][cohort]);
	}
    **/
    
	// Used to assume that as iterating down last lostden was for the youngest ageclass,
	// now storing it explicitly, just to be safe (and storing one value per gene type - see lostden_zero).
	if(lostden > 0)
		active_den[cohort] = 1;


	return dennow;
}

/**
 * \brief Updating numbers of aging up through age classes. Also updating the local distributions
 *
 */
void Update_Aging_Numbers(MSEBoxModel *bm, int species, int cohort, int stock_id, double dennow, double this_p_ageup, int do_debug, FILE *llogfp) {
	int sp_AgeClassSize = FunctGroupArray[species].ageClassSize;
	int i = 0;
	int sp_numGeneTypes = FunctGroupArray[species].numGeneTypes;
	int num_cohorts = (FunctGroupArray[species].numCohortsXnumGenes) - 1;
	int nextcid = 0;

	nextcid = cohort + sp_numGeneTypes;

	if (bm->flagagestruct) {
		/* If non-homogeneous distribution, update local distribution
		 - youngest group (i=0) already zeroed */
		for ( i = (int) (sp_AgeClassSize - 1); i > 0; i--) {
			bm->tempPopRatio[stock_id][species][cohort][i] += VERTinfo[species][cohort][DEN_id]
					* FunctGroupArray[species].boxPopRatio[bm->current_box][bm->current_layer][cohort][i - 1];

			if((!_finite(bm->tempPopRatio[stock_id][species][cohort][i])) || ((species == bm->which_check) && do_debug)){
				fprintf(llogfp,"Time: %e %s-%d in Box%d-%d has tempPopRatio %e for stock %d subclass %d as DEN: %e boxPopRatio: %e\n",
						bm->dayt, FunctGroupArray[species].groupCode, cohort, bm->current_box, bm->current_layer,
						bm->tempPopRatio[stock_id][species][cohort][i], stock_id, i, VERTinfo[species][cohort][DEN_id],
						FunctGroupArray[species].boxPopRatio[bm->current_box][bm->current_layer][cohort][i - 1]);
			}
		}
		if (cohort <= (num_cohorts - sp_numGeneTypes)) {
			bm->tempPopRatio[stock_id][species][nextcid][0] += dennow;

			if((!_finite(bm->tempPopRatio[stock_id][species][cohort][i])) || (species == bm->which_check)){
				fprintf(llogfp,"Time: %e %s-%d in Box%d-%d has tempPopRatio %e for stock %d subclass %d as dennow: %e\n",
						bm->dayt, FunctGroupArray[species].groupCode, nextcid, bm->current_box, bm->current_layer,
						bm->tempPopRatio[stock_id][species][nextcid][0], stock_id, 0, dennow);
			}

		}

        /**
        if (do_debug && (bm->which_check == species)) {
			for ( i = 0; i < sp_AgeClassSize; i++) {
				fprintf(llogfp,"Time %e, box%d-%d %s-%d has tempPopRatio[%d][%d]: %e as VERTINFOden: %e, boxoPopRatio: %e, dennow: %e\n",
						bm->dayt, bm->current_box, bm->current_layer, FunctGroupArray[species].groupCode, cohort, stock_id, i, bm->tempPopRatio[stock_id][species][cohort][i],
						VERTinfo[species][cohort][DEN_id], FunctGroupArray[species].boxPopRatio[bm->current_box][bm->current_layer][cohort][i], dennow);
			}
		}
        **/
	}

	/* Store numbers and sizes to carry over in aging updating of stocks */
	if (cohort <= (num_cohorts - sp_numGeneTypes)) {
        FunctGroupArray[species].agingVERT[nextcid][stock_id][SN_id] += VERTinfo[species][cohort][SN_id] * dennow;
		FunctGroupArray[species].agingVERT[nextcid][stock_id][RN_id] += (VERTinfo[species][cohort][RN_id] - EMBRYO[species].IndSpawn[cohort]) * dennow;
		FunctGroupArray[species].agingVERT[nextcid][stock_id][DEN_id] += dennow;

        /**
		//if (do_debug && (bm->which_check == species)) {
        if (species == 35) {
			fprintf(llogfp, "Time: %e, box%d-%d %s-chrt_aged_into %d stock: %d, agingDEN: %e, agingSN: %e, agingRN: %e dennow: %e\n", bm->dayt, bm->current_box, bm->current_layer, FunctGroupArray[species].groupCode, nextcid, stock_id, FunctGroupArray[species].agingVERT[nextcid][stock_id][DEN_id], FunctGroupArray[species].agingVERT[nextcid][stock_id][SN_id], FunctGroupArray[species].agingVERT[nextcid][stock_id][RN_id], dennow);
		}
        **/
        
        if(bm->track_contaminants){
            Age_Contaminants_Store(bm, species, cohort, nextcid, dennow, this_p_ageup);
        }
	}
	return;
}

/**
 * \brief Update Migration array for aging
 *
 *  So the logic of what aging is done is
 *  A. Both outside the model domain then update only the MIGRATION array [ Done in this routine ]
 *  B. Juveniles outside the domain and adults inside then add juveniles via update FunctGroupArray[].agingVERT
 *      so they have dribbled back into the doman [ Done in this routine ]
 *  C. Juveniles inside the domain and adults outside then handle this in Ecology_Update_Vert_Cohorts()
 *      where you (a) update the MIGRATION array (so any juveniles outside the domain in the MIGRATION array.... which should
 *      be negligible... are just added to adults outside and will return with them next time round)
 *      and assume (b) that any juveniles updated to adults normally within the model domain will migrate normally
 *      with the rest of the adults in subseqent years
 *  D. Both adults and juveniles in the model domain then that is straightforward aging handled in Ecology_Update_Vert_Cohorts()
 *
 */
void Update_Migration_Array(MSEBoxModel *bm, int species, int cohort, int maxstock_id, int do_debug, FILE *llogfp) {
	int spmigrate = MIGRATION[species].num_in_queue;
    int num_cohorts = (FunctGroupArray[species].numCohortsXnumGenes) - 1;
    int qid = 0;
    double p_ageup, oldden, oldSN, oldRN, newSN, newRN, num_aging, growth_period, sizeScalar;
    int nextcid = cohort + FunctGroupArray[species].numGeneTypes;
    //int sp_numGeneTypes = FunctGroupArray[species].numGeneTypes;
    //int age_mat = (int) (FunctGroupArray[species].speciesParams[age_mat_id]);
	//int cid = (int)(floor(cohort / sp_numGeneTypes));
    //int nmig;
    //int stage = FunctGroupArray[species].cohort_stage[cohort];
    //double mxs = (double)(maxstock_id);

    //do_debug = 1;
    
    if ((bm->current_box != bm->first_box) || (bm->current_layer != bm->top_layer)){
        return;  // As only want to do this once
    }
    
    /* Have replaced previous detailed and somewhat painful handling of aging in the MIGRATION array by having single entry per migration queue rather than trying to overload per cohort */
    if (spmigrate) {
        if (FunctGroupArray[species].isVertebrate == TRUE){
            p_ageup = Get_Proportion_Aging(bm, species, cohort, do_debug, llogfp);  // TODO: Need to allow for these to have evolution act
        } else {
            p_ageup = shiftVERT[species][cohort][RN_id]; // p_ageup stored here for biomass pool age structured groups
        }

        if (cohort < num_cohorts) {
            for (qid = MIGRATION[species].num_in_queue_done; qid < MIGRATION[species].num_in_queue; qid++) {
                if (FunctGroupArray[species].isVertebrate == TRUE){
                    p_ageup = Get_Proportion_Aging(bm, species, cohort, do_debug, llogfp);  // TODO: Need to allow for these to have evolution act
                } else {
                    p_ageup = shiftVERT[species][cohort][RN_id]; // p_ageup stored here for biomass pool age structured groups
                }
                
                oldden = MIGRATION[species].DEN[nextcid][qid];
                oldSN = MIGRATION[species].SN[nextcid][qid];
                oldRN = MIGRATION[species].RN[nextcid][qid];
                
                newSN = MIGRATION[species].SN[cohort][qid];
                newRN = MIGRATION[species].RN[cohort][qid];
                num_aging = (p_ageup * MIGRATION[species].DEN[cohort][qid]);

                /* Update size
                 
                 Start with size scalar - only needed if not using external pop code to update size explicitly
                 */
                sizeScalar = 1.0;
                if (FunctGroupArray[species].isExternal == FALSE) {
                    if(!MIGRATION[species].IsAnnualMigration[qid]) {
                        growth_period = (double)(MIGRATION[species].MinYearsAway[qid]);
                        sizeScalar = 1.0 + (MIGRATION[species].growth[qid] / growth_period);
                    }
                }
                
                MIGRATION[species].SN[nextcid][qid] = (oldden * oldSN + num_aging * newSN * sizeScalar) / (oldden + num_aging + small_num);
                MIGRATION[species].RN[nextcid][qid] = (oldden * oldRN + num_aging * newRN * sizeScalar) / (oldden + num_aging + small_num);
                
                if(bm->track_contaminants){
                    Age_MigrantContaminants_Update(bm, species, nextcid, cohort, qid, oldden, num_aging);
                }
                
                /* Update numbers */
                MIGRATION[species].DEN[nextcid][qid] += num_aging;
                MIGRATION[species].DEN[cohort][qid] -= num_aging;
                
                MIGRATION[species].aging[nextcid][qid] = num_aging;
                
                if (MIGRATION[species].DEN[nextcid][qid] > 0) {
                    MIGRATION[species].cohort_migrating[nextcid][qid] = 1;
                } 
                
                /* Not doing this as getting double migration - as enter ok, and not checked when leave this is proabably ok
                if (!MIGRATION[species].cohort_migrating[nextcid][qid]) {
                    MIGRATION[species].cohort_migrating[nextcid][qid] = 1; // Update so allowed to migrate in the movement code
                }
                 */
                
                /**
                //if (do_debug) {
                 if (species == 33) {
                    fprintf(llogfp,"Time: %e box%d-%d %s-%d qid: %d p_ageup: %e, DEN: %e nextDEN: %e (num_in_queue: %d)\n", bm->dayt, bm->current_box, bm->current_layer, FunctGroupArray[species].groupCode, cohort, qid, p_ageup, MIGRATION[species].DEN[cohort][qid], MIGRATION[species].DEN[nextcid][qid], MIGRATION[species].num_in_queue);
                }
                **/
                
            }
        } else {
            for (qid = MIGRATION[species].num_in_queue_done; qid < MIGRATION[species].num_in_queue; qid++) {
                
                if( (FunctGroupArray[species].speciesParams[flagkeep_plusgroup_id] > 0) && (bm->dayt < MIGRATION[species].Return_Now[qid])) {
                    // Do nothing as want to keep a plus group that hasn't returned to the model yet
                } else {
                    MIGRATION[species].DEN[cohort][qid] *= (1.0 - p_ageup);  // So have aging out of system handled
                }
                
                /**
                 if (species == 33) {
                    fprintf(llogfp,"Time: %e box%d-%d %s-%d qid: %d p_ageup: %e, DEN: %e\n", bm->dayt, bm->current_box, bm->current_layer, FunctGroupArray[species].groupCode, cohort, qid, p_ageup, MIGRATION[species].DEN[cohort][qid]);
                }
                **/
            }
        }
    }

    return;
}

/**
 * \brief Handle new settlers and prepare for their addition to the youngest age class
 *
 */
void Get_Latest_Additions(MSEBoxModel *bm, int wclayer, int species, int maxstock_id, int stock_id, double *lostden_zero, int need_update, int do_debug, FILE *llogfp) {
    int i = 0, nc = 0, cohort = 0, migstock_id = 0, ngene = 0, qid = 0, mxk = 0;
    //int stage = 0;
	double sp_AgeClassSize = (double)(FunctGroupArray[species].ageClassSize);
	int sp_numGeneTypes = (int) (FunctGroupArray[species].numGeneTypes);    
    Util_Init_1D_Double(coming_SPden, bm->K_num_max_genetypes, 0.0);
    
	/* Get total settlers that are in the process of arriving */
	for ( ngene = 0; ngene < sp_numGeneTypes; ngene++ ){
		coming_SPden[ngene] = EMBRYO[species].num_recruits[bm->current_box][wclayer][ngene][EMBRYO[species].next_recruit];
	}

	/* If non-homogeneous distribution, update distribution for new recruits */
	/* If not already in terms of absolute numbers make it so */
	if (!need_update) {
		/* Initialise in first cell */
		if ((bm->current_box == bm->first_box) && (bm->current_layer == bm->top_layer)) {
			for ( ngene = 0; ngene < sp_numGeneTypes; ngene++ ){
				for (nc = 0; nc < sp_AgeClassSize; nc++) {
					recVERTpopratio[stock_id][species][ngene][nc] = 0.0;
				}
			}
		}

		for ( cohort = 0; cohort < sp_numGeneTypes; cohort++ ){
			for (i = (int) (sp_AgeClassSize - 1); i >= 0; i--) {
				recVERTpopratio[stock_id][species][cohort][i] += VERTinfo[species][cohort][DEN_id]
						* FunctGroupArray[species].boxPopRatio[bm->current_box][bm->current_layer][cohort][i];
			}
			recVERTpopratio[stock_id][species][cohort][0] += coming_SPden[cohort]; // Add to the youngest age class component
		}

	} else {
		for ( ngene = 0; ngene < sp_numGeneTypes; ngene++ ){
			bm->tempPopRatio[stock_id][species][ngene][0] += coming_SPden[ngene]; //Used to say cohort for tempPopRatio

			if((!_finite(bm->tempPopRatio[stock_id][ngene][cohort][0])) || (species == bm->which_check)){
				fprintf(llogfp,"Time: %e %s-%d in Box%d-%d (ngene %d) has tempPopRatio %e for stock %d subclass %d as dennow: %e\n",
						bm->dayt, FunctGroupArray[species].groupCode, cohort, ngene, bm->current_box, bm->current_layer,
						bm->tempPopRatio[stock_id][species][ngene][0], stock_id, 0, coming_SPden[ngene]);
			}

		}
	}

	/* Add any recruits due to arrival in migration array */
	if ((bm->current_box == bm->first_box) && (bm->current_layer == bm->top_layer)) {
		for ( ngene = 0; ngene < sp_numGeneTypes; ngene++ ){
			for ( qid = MIGRATION[species].num_in_queue_done; qid < MIGRATION[species].num_in_queue; qid++) {
				if (MIGRATION[species].recruit[ngene][qid] > 0) {
					/* Split across stocks */
					migstock_id = (int) (MIGRATION[species].returnstock[qid]) - 1;
					if (migstock_id < 0) {
						/* Homogeneously spread across stocks */
						for ( mxk = 0; mxk < maxstock_id; mxk++) {
							bm->tempPopRatio[mxk][species][ngene][0] += (MIGRATION[species].recruit[ngene][qid] / (double)(maxstock_id));

							if(!_finite(bm->tempPopRatio[mxk][species][ngene][0])){
								fprintf(llogfp,"Time: %e %s-%d in Box%d-%d (ngene %d) has tempPopRatio %e for stock %d subclass %d as MIGrecruit[%d]: %e, maxstock: %d\n",
										bm->dayt, FunctGroupArray[species].groupCode, cohort, ngene, bm->current_box, bm->current_layer,
										bm->tempPopRatio[mxk][species][ngene][0], mxk, 0, qid, MIGRATION[species].recruit[ngene][qid], maxstock_id);
							}

						}
					} else {
						/* Directed to a specific stock */
						bm->tempPopRatio[migstock_id][species][ngene][0] += MIGRATION[species].recruit[ngene][qid];

						if(!_finite(bm->tempPopRatio[migstock_id][species][ngene][0])){
							fprintf(llogfp,"Time: %e %s-%d in Box%d-%d (ngene %d) has tempPopRatio %e for stock %d subclass %d as MIGrecruit[%d]: %e\n",
									bm->dayt, FunctGroupArray[species].groupCode, cohort, ngene, bm->current_box, bm->current_layer,
									bm->tempPopRatio[migstock_id][species][ngene][0], migstock_id, 0, qid, MIGRATION[species].recruit[ngene][qid]);
						}

					}
				}

				/* Reset MIGRATION[species].recruit[ngene] */
				//MIGRATION[species].recruit[ngene][qid] = 0;
			}
		}
	}
	
	return;
}

/**
 * \brief Update the Local tracers - the cohort index used to be i in the original code when was all in one routine
 *
 */
void Update_Local_dPool(MSEBoxModel *bm, int species, int cohort, double Spawned_Matter, double *recruitDEN, 
		int sp_numGeneTypes, double *localWCTracers, int do_debug, FILE *llogfp) {
	int vids = FunctGroupArray[species].structNTracers[cohort];
	int vidr = FunctGroupArray[species].resNTracers[cohort];
	int vidd = FunctGroupArray[species].NumsTracers[cohort];

	// Sanity check before we start
	if(localWCTracers[vidd] < 0.0)
		localWCTracers[vidd] = 0.0;

    /**
	if (do_debug && (bm->which_check == species)) {
		fprintf(llogfp, "Time: %e, %s-%d starting Pool: %e with SN: %e, RN: %e with shiftden: %e and Spawned_Matter: %e\n", bm->dayt,
			FunctGroupArray[species].groupCode, cohort, 
			localWCTracers[vidd], localWCTracers[vids], localWCTracers[vidr], shiftVERT[species][cohort][DEN_id], Spawned_Matter);
	}
    **/

	/* Update any loses due to spawning */
	localWCTracers[vidr] -= Spawned_Matter;

	/* Update numbers remaining after aging up - numbers added occurs in update */
	if (shiftVERTON[species][cohort] > 0) {
		localWCTracers[vidd] = shiftVERT[species][cohort][DEN_id];
	}

	/* For the youngest age class update all characteristics for each geneotype */
	if (cohort < sp_numGeneTypes) {
		if(recruitDEN[cohort] > 0){
			localWCTracers[vids] = shiftVERT[species][cohort][SN_id];
			localWCTracers[vidr] = shiftVERT[species][cohort][RN_id];
			localWCTracers[vidd] += recruitDEN[cohort];   // As actually the different geneotypes as all in youngest cohort
		}

		/**
		if ((recruitDEN[cohort] > 0) && do_debug && (species == bm->which_check)) {
        //if (recruitDEN[cohort] > 0) {
            fprintf(llogfp, "Time: %e, box%d %s-%d newden: %e, recruitSPden: %e (SN: %e, RN: %e)\n", bm->dayt, bm->current_box, FunctGroupArray[species].groupCode, cohort,
					localWCTracers[vidd], recruitDEN[cohort], localWCTracers[vids], localWCTracers[vidr]);
		}
		**/

		if ((localWCTracers[vids] < 0.0) || (localWCTracers[vidr] < 0.0) || (localWCTracers[vidd] < 0.0) || (!_finite(localWCTracers[vidd]))) {
			fprintf(llogfp, "Time: %e, box%d %s-%d newden: %e, recruitSPden: %e, shiftden: %e, newsn: %e, shift-sn: %e, newrn: %e, shift-rn: %e\n",
					bm->dayt, bm->current_box, FunctGroupArray[species].groupCode, cohort, localWCTracers[vidd], recruitDEN[cohort], shiftVERT[species][cohort][DEN_id], localWCTracers[vids], shiftVERT[species][cohort][SN_id], localWCTracers[vidr], shiftVERT[species][cohort][RN_id]);
			printf("Time: %e, box%d %s-%d newden: %e, recruitSPden: %e, shift-den: %e, newsn: %e, shift-sn: %e, newrn: %e, shift-rn: %e\n",
					bm->dayt, bm->current_box, FunctGroupArray[species].groupCode, cohort, localWCTracers[vidd], recruitDEN[cohort], shiftVERT[species][cohort][DEN_id],
					localWCTracers[vids], shiftVERT[species][cohort][SN_id], localWCTracers[vidr], shiftVERT[species][cohort][RN_id]);
			quit("%s-%d had negative size or numbers after aging/recruitment\n", FunctGroupArray[species].groupCode, cohort);
		}

	}
    
    /**
    if ((do_debug && (shiftVERT[species][cohort][DEN_id] > 0.0)) || (!_finite(localWCTracers[vidd]))) {
    	fprintf(llogfp, "Time: %e %s-%d newPool: %e (%e) with SN: %e RN: %e (Spawned_Matter: %e shiftDEN: %e)\n",
				bm->dayt, FunctGroupArray[species].groupCode, cohort,
				localWCTracers[vidd], shiftVERT[species][cohort][DEN_id],
				localWCTracers[vids], localWCTracers[vidr],
				Spawned_Matter, shiftVERT[species][cohort][DEN_id]);
	}
    **/
	if((!_finite(localWCTracers[vidd]))){
		quit("Update_Local_dPool %s-%d has nan numbers in box %d-%d\n", FunctGroupArray[species].groupCode, cohort, bm->current_box, bm->current_layer);
	}

	return;
}

/*******************
 * Static Functions - were static in atvertprocess.c, but void here (at least for now).
 *
 */

/**
 *	\brief Routine to update overall population age distribution arrays (due to aging or recruitment)
 *	for homogeneous populations that smoothes local variations and resets for the coming year
 *
 */
void Update_Age_Distrib(MSEBoxModel *bm, int species, int stock_id, int sp_ddepend_move, int rec_related, FILE *llogfp) {

	int cohort, k, classize, maxstock_id, need_recupdate, i, ij, b, sp_recruitpeak;
	double maxvalue, sp_recruitRange, sp_AgeClassSize;
	int s_id, spmigrate, is_outside, qid, do_test;
    //int stage;
    //int do_debug = 0;
	int NumCohort = FunctGroupArray[species].numCohortsXnumGenes;

    Util_Init_1D_Int(stock_done, bm->K_num_stocks_per_sp, 0);
    Util_Init_1D_Double(totsum, bm->K_num_stocks_per_sp, 0.0);
    Util_Init_1D_Double(totksum, bm->K_num_stocks_per_sp, 0.0);
    
	/*
     
     if(((bm->nbox < bm->checkbox) || (bm->current_box == bm->checkbox)) && (bm->dayt > bm->checkstart) && (bm->which_check == species))
        do_debug = 1;
    
    if ((FunctGroupArray[species].numSpawns > 1) || (FunctGroupArray[species].groupType == FISH_INVERT))
        do_debug = 1;

    if (bm->which_check == species)
        do_debug = 1;
     
     if((bm->which_check == species) && (bm->dayt >= bm->checkstart))
     do_debug = 1;

     if (!species)
         do_debug = 1;

     */

	if(rec_related) {
		NumCohort = FunctGroupArray[species].numGeneTypes; // As only the youngest age class being dealt with
	}

	sp_AgeClassSize = (double)(FunctGroupArray[species].ageClassSize);
	maxstock_id = FunctGroupArray[species].numStocks;
	need_recupdate = (int) (FunctGroupArray[species].speciesParams[need_recupdate_id]);
	sp_recruitpeak = (int) (FunctGroupArray[species].speciesParams[flagrecpeak_id]);

	if (sp_recruitpeak)
		sp_recruitRange = bm->recruitRange;
	else
		sp_recruitRange = bm->recruitRangeFlat;

	if ((int) (sp_AgeClassSize) == 0)
		classize = 1;
	else
		classize = (int) (sp_AgeClassSize);

    if (classize == 1) {
		if((bm->current_box == bm->last_box && !bm->current_layer)) {
			/* If an age class is a year class then should always just have a transference ratio of 1.0 */
			for (cohort = 0; cohort < NumCohort; cohort++) {
				for (i = 0; i < maxstock_id; i++) {
					bm->tempPopRatio[i][species][cohort][0] = 1.0;
				}
				if (sp_ddepend_move == 1) {
					/* If site attached just update one box - doesn't effect calculations, but done for speed
					 (remove redundant loops)
					 */
					FunctGroupArray[species].boxPopRatio[bm->current_box][bm->current_layer][cohort][0] = 1.0;
				} else {
					for (ij = 0; ij < bm->nbox; ij++) {
						for (k = 0; k < bm->boxes[ij].nz; k++) {
							FunctGroupArray[species].boxPopRatio[ij][k][cohort][0] = 1.0;
						}
					}
				}
			}
		}
	} else {
		is_outside = 0;
		spmigrate = MIGRATION[species].num_in_queue;  // So if migrate at all
		if(spmigrate) {
			for (cohort = 0; cohort < NumCohort; cohort++) {
				for (qid = MIGRATION[species].num_in_queue_done; qid < MIGRATION[species].num_in_queue; qid++) {
					// If all leave the arena then see if currently gone
					if ( MIGRATION[species].all_go[cohort][qid] && (MIGRATION[species].Return_Period[qid] > 0.0)) {
						if ((MIGRATION[species].Leave_Now[qid] <= bm->dayt) && (bm->dayt <= MIGRATION[species].Return_Now[qid])){
							is_outside = 1;
						}
					}
				}
			}
		}


		/* Check to see if animal has site attachment and non homogeneous populations */
		if (!bm->flaghomog_sp) {
			/* Nothing to do as want to keep local distributions unchanged */
		}  else if (is_outside && ((bm->current_box == bm->last_box && !bm->current_layer))) {

			/* TODO: Figure out a better way of doing this for species outside the model-domain:
			 * At present it just assumes the same distribution for ever, but in reality should
			 * allow for more the same variability in cohort strength to flow through
			 */
            for (cohort = 0; cohort < NumCohort; cohort++) {
				//stage = FunctGroupArray[species].cohort_stage[cohort];
				for (qid = MIGRATION[species].num_in_queue_done; qid < MIGRATION[species].num_in_queue; qid++) {
                    is_outside = 0;
					// If all leave the arena then see if currently gone
					if ( MIGRATION[species].all_go[cohort][qid] && (MIGRATION[species].Return_Period[qid] > 0.0)) {
						if ((MIGRATION[species].Leave_Now[qid] <= bm->dayt) && (bm->dayt <= MIGRATION[species].Return_Now[qid])){
							is_outside = 1;
						}
					}
					if ( is_outside ){
						/* Get a representative value for each stock */
						for (i = 0; i < maxstock_id; i++) {
							for (ij = 0; ij < bm->nbox; ij++) {
								if (bm->boxes[ij].type != BOUNDARY) {
									for (k = 0; k < bm->boxes[ij].nz; k++) {
										/* Identify the stock under consideration */
										s_id = bm->group_stock[species][ij][k];

										if(!stock_done[s_id]){
											for (b = 1; b < classize; b++) {
												bm->tempPopRatio[s_id][species][cohort][b] += (FunctGroupArray[species].boxPopRatio[ij][k][cohort][b-1] * MIGRATION[species].DEN[cohort][qid]);
												if(rec_related)
													recVERTpopratio[stock_id][species][cohort][b] += (FunctGroupArray[species].boxPopRatio[ij][k][cohort][b-1] * MIGRATION[species].DEN[cohort][qid]);

												if((!_finite(FunctGroupArray[species].boxPopRatio[ij][k][cohort][b])) || (!_finite(bm->tempPopRatio[s_id][species][cohort][b]))){
													quit("tempPopRatio for %s-%d in box%d-%d is NAN for stock_id %d as boxPopRatio[%d] %e, MIGRATIONden[%d]: %e\n",
															FunctGroupArray[species].groupCode, cohort, ij, k, s_id, b-1, FunctGroupArray[species].boxPopRatio[ij][k][cohort][b-1],
															qid, MIGRATION[species].DEN[cohort][qid]);
												}

											} // Note boxPopRatio itself is never updated as assumed constant through time for now
											stock_done[s_id] = 1;
										}
									}
								}
							}
						}
					}
				}
			}
		} else if (sp_ddepend_move == 1) {
			/* Site attached case done box by box */
			for (cohort = 0; cohort < NumCohort; cohort++) {
				totsum[stock_id] = 0;
				for (k = 0; k < classize; k++) {
					if(rec_related)
						bm->tempPopRatio[stock_id][species][cohort][k] = recVERTpopratio[stock_id][species][cohort][k];
					totsum[stock_id] += bm->tempPopRatio[stock_id][species][cohort][k];
				}

				if (!totsum[stock_id]) {
					totsum[stock_id] = small_num;
					//quit("%s-%d had ltotsum = 0 in box: %d-%d \n", FunctGroupArray[species].groupCode, cohort, bm->current_box, bm->current_layer);
				}

				/* Normalise and find maximum entry */
				maxvalue = -MAXDOUBLE;
				for (k = 0; k < classize; k++) {
					bm->tempPopRatio[stock_id][species][cohort][k] /= (totsum[stock_id] + small_num);

					if (bm->tempPopRatio[stock_id][species][cohort][k] > maxvalue)
						maxvalue = bm->tempPopRatio[stock_id][species][cohort][k];

					if(!_finite(bm->tempPopRatio[stock_id][species][cohort][k])){
						fprintf(llogfp,"Time: %e %s-%d in Box%d-%d has tempPopRatio %e for stock %d subclass %d as totsum: %e\n",
								bm->dayt, FunctGroupArray[species].groupCode, cohort, bm->current_box, bm->current_layer,
								bm->tempPopRatio[stock_id][species][cohort][k], stock_id, k, totsum[stock_id]);
					}

				}

				/* Re-adjust (if necessary) - assuming that very populus
				 cohorts are disproportinately lost and that not all individuals grow
				 equally fast so some will sneak ahead/lag behind anyway */
				totksum[stock_id] = 0;
				do_test = 0;
				if(rec_related) do_test = 1;
				else if(cohort || (!cohort && k)) do_test = 1;
				for (k = 0; k < classize; k++) {
					if (do_test && (bm->tempPopRatio[stock_id][species][cohort][k] < (maxvalue / sp_recruitRange))) {
						bm->tempPopRatio[stock_id][species][cohort][k] = maxvalue / sp_recruitRange;
					}

					totksum[stock_id] += bm->tempPopRatio[stock_id][species][cohort][k];
				}
				if(!totksum[stock_id]){
					totksum[stock_id] = small_num;
					fprintf(llogfp,"Time %e box%d-%d %s-%d having to apply a smooth age distribution across subclasses as sum zero\n",
							bm->dayt, bm->current_box, bm->current_layer, FunctGroupArray[species].groupCode, cohort);

					// Apply smoother distribution
					for (k = 0; k < classize; k++) {
						bm->tempPopRatio[stock_id][species][cohort][k] = 1.0 / sp_AgeClassSize;
					}
					totksum[stock_id] = 1;  // So not renormalised again
				}

				if (totksum[stock_id] != 1) {
					/* Renormalise */
					for (k = 0; k < classize; k++) {
						bm->tempPopRatio[stock_id][species][cohort][k] /= totksum[stock_id];
					}
				}
                
                /* If site attached update local age distribution */
				for (k = 0; k < classize; k++) {
					FunctGroupArray[species].boxPopRatio[bm->current_box][bm->current_layer][cohort][k] = bm->tempPopRatio[stock_id][species][cohort][k];

					if(!_finite(FunctGroupArray[species].boxPopRatio[bm->current_box][bm->current_layer][cohort][k])){
						quit("boxPopRatio for %s-%d in box%d-%d is NAN as tempPopRatio for stock_id %d = %e, totksum: %e totsum: %e maxvalue: %e sp_recruitRange: %e\n",
								FunctGroupArray[species].groupCode, cohort, bm->current_box, bm->current_layer, stock_id, bm->tempPopRatio[stock_id][species][cohort][k],
								totksum[stock_id], totsum[stock_id], maxvalue, sp_recruitRange);
					}

				}

				/* If last box then save average value for the entire stock */
				if (bm->current_box == bm->last_box && !bm->current_layer) {
					for (i = 0; i < maxstock_id; i++) {
						for (k = 0; k < classize; k++) {
							sumSTOCK[i][k] = 0;
							nSTOCK[i][k] = 0;
						}
					}

					for (ij = 0; ij < bm->nbox; ij++) {
						if (bm->boxes[ij].type != BOUNDARY) {
							for (k = 0; k < bm->boxes[ij].nz; k++) {

								/* Identify the stock under consideration */
								s_id = bm->group_stock[species][ij][k];

								for (b = 0; b < classize; b++) {
									sumSTOCK[s_id][b] += FunctGroupArray[species].boxPopRatio[ij][k][cohort][b];
									nSTOCK[s_id][b]++;

								}
							}
						}
					}

					for (i = 0; i < maxstock_id; i++) {
						for (k = 0; k < classize; k++) {
							bm->tempPopRatio[i][species][cohort][k] = sumSTOCK[i][k] / (nSTOCK[i][k] + small_num);
                            /*
							if(do_debug){
								fprintf(llogfp,"Time: %e %s-%d stock %d ageclass %d tempPopRatio: %e sumSTOCK: %e nStock: %d\n",
										bm->dayt, FunctGroupArray[species].groupCode, cohort, i, k, bm->tempPopRatio[i][species][cohort][k], sumSTOCK[i][k], nSTOCK[i][k]);
							}
                            */
						}
					}
				}
			}
		} else {
			/* Global population done simultaneously */
			if((bm->current_box == bm->last_box && !bm->current_layer)) {
				for (cohort = 0; cohort < NumCohort; cohort++) {
					for (i = 0; i < maxstock_id; i++) {
						totsum[i] = 0;
						for (k = 0; k < classize; k++) {
							if(rec_related)
								bm->tempPopRatio[i][species][cohort][k] = recVERTpopratio[i][species][cohort][k];
							totsum[i] += bm->tempPopRatio[i][species][cohort][k];
                            
                            /**
                            if(do_debug){
								fprintf(llogfp,"Time: %e step 1 %s-%d stock %d k: %d tempPopRatio: %e (recVERTpopratio: %e) totsum %e allgone: %d\n",
										bm->dayt, FunctGroupArray[species].groupCode, cohort, i, k, bm->tempPopRatio[i][species][cohort][k], recVERTpopratio[i][species][cohort][k], totsum[i], FunctGroupArray[species].allgone[cohort]);
							}
                            **/

						}
                        if(!rec_related) {
                            for (qid = MIGRATION[species].num_in_queue_done; qid < MIGRATION[species].num_in_queue; qid++) {
                                totsum[i] += MIGRATION[species].aging[cohort][qid];
                            }
                        }

						if (!totsum[i]){
							totsum[i] = small_num;
							if(!FunctGroupArray[species].allgone[cohort]){
								warn("%s-%d had totsum = 0\n", FunctGroupArray[species].groupCode, cohort);
								fprintf(llogfp,"time: %e %s-%d stock %d had totsum = 0\n", bm->dayt, FunctGroupArray[species].groupCode, cohort, i);
							}
						}

						/* Normalise and find maximum entry */
						maxvalue = -MAXDOUBLE;
						for (k = 0; k < classize; k++) {
							bm->tempPopRatio[i][species][cohort][k] /= (totsum[i] + small_num);

							if (bm->tempPopRatio[i][species][cohort][k] > maxvalue)
								maxvalue = bm->tempPopRatio[i][species][cohort][k];
                            
                            /**
							if(do_debug){
								fprintf(llogfp,"Time: %e step 2 %s-%d stock %d k: %d tempPopRatio: %e totsum %e\n",
										bm->dayt, FunctGroupArray[species].groupCode, cohort, i, k, bm->tempPopRatio[i][species][cohort][k], totsum[i]);
							}
                            **/

						}

						/* Re-adjust (if necessary) - assuming that very populus
						 cohorts are disproportinately lost and that not all individuals grow
						 equally fast so some will sneak ahead/lag behind anyway */
						totksum[i] = 0;
						do_test = 0;
						if(rec_related) do_test = 1;
						else if(cohort || (!cohort && k)) do_test = 1;
						for (k = 0; k < classize; k++) {
							if (do_test && (bm->tempPopRatio[i][species][cohort][k] < (maxvalue / sp_recruitRange))) {
								bm->tempPopRatio[i][species][cohort][k] = maxvalue / sp_recruitRange;
                                /**
								if(do_debug){
									fprintf(llogfp,"Time: %e step 3 %s-%d stock %d classize %d tempPopRatio: %e maxvalue: %e sp_recruitRange %e\n",
											bm->dayt, FunctGroupArray[species].groupCode, cohort, i, k, bm->tempPopRatio[i][species][cohort][k], maxvalue, sp_recruitRange);
								}
                                **/

							}

							totksum[i] += bm->tempPopRatio[i][species][cohort][k];
                            
                            /*
							if(do_debug){
								fprintf(llogfp,"Time: %e step 4 %s-%d stock %d classize: %d tempPopRatio: %e totksum %e\n",
										bm->dayt, FunctGroupArray[species].groupCode, cohort, i, k, bm->tempPopRatio[i][species][cohort][k], totksum[i]);
							}
                             */

						}

						if (!totksum[i]){
							totksum[i] = small_num;
							if(!FunctGroupArray[species].allgone[cohort]){
								warn("%s-%d had totsum = 0\n", FunctGroupArray[species].groupCode, cohort);
								fprintf(llogfp,"time: %e %s-%d stock %d had totsum = 0\n", bm->dayt, FunctGroupArray[species].groupCode, cohort, i);
							}
						}

						if (totksum[i] != 1) {
							
                            /* Renormalise */
							for (k = 0; k < classize; k++) {
								bm->tempPopRatio[i][species][cohort][k] /= totksum[i];
                            
                                /**
								if(do_debug){
									fprintf(llogfp,"Time: %e step 5 renormalising %s-%d stock %d classize %d tempPopRatio: %e totksum %e\n",
											bm->dayt, FunctGroupArray[species].groupCode, cohort, i, k, bm->tempPopRatio[i][species][cohort][k], totksum[i]);
								}
                                **/
							}
						}
					}


					/* Update local age distribution */
					for (ij = 0; ij < bm->nbox; ij++) {
						if (bm->boxes[ij].type != BOUNDARY) {
							for (k = 0; k < bm->boxes[ij].nz; k++) {

								/* Identify the stock under consideration */
								stock_id = bm->group_stock[species][ij][k];

								/* If in box matching current stock id then update local age distribution with
								 overall stock value */
								for (b = 0; b < classize; b++) {
									FunctGroupArray[species].boxPopRatio[ij][k][cohort][b] = bm->tempPopRatio[stock_id][species][cohort][b];

                                    
                                    if(!_finite(FunctGroupArray[species].boxPopRatio[ij][k][cohort][b])){
										quit("boxPopRatio for %s-%d in box%d-%d sub-chort (b) %d is NAN as tempPopRatio for stock_id %d = %e MIGRATIONaging may hold more\n",
												FunctGroupArray[species].groupCode, cohort, ij, k, b, stock_id, bm->tempPopRatio[stock_id][species][cohort][b]);
									}
                                    /**
									if(do_debug){
										fprintf(llogfp,"Time: %e boxPopRatio for %s-%d sub-chort (b) %d in box%d-%d tempPopRatio for stock_id %d = %e MIGRATIONaging: may hold more\n",
												bm->dayt, FunctGroupArray[species].groupCode, cohort, ij, k, b, stock_id, bm->tempPopRatio[stock_id][species][cohort][b]);
									}
                                    **/
									if(FunctGroupArray[species].boxPopRatio[ij][k][cohort][b] > 1.0){
										quit("Time: %e boxPopRatio for %s-%d in box%d-%d sub-chort (b) %d tempPopRatio for stock_id %d = %e MIGRATIONaging may hold more\n",
												bm->dayt, FunctGroupArray[species].groupCode, cohort, ij, k, b, stock_id, bm->tempPopRatio[stock_id][species][cohort][b]);
									}
								}
							}
						}
					}
				}
			}
		}
	}
    
    // Update migrators so nhave an appropriate proportion assigned to aging event on return
    if (MIGRATION[species].num_in_queue > 0) {
        for (cohort = 0; cohort < NumCohort; cohort++) {
            for (b = 0; b < classize; b++) {
                MIGRATION[species].current_pop_ratio[cohort][b] = 0.0;
                for (i = 0; i < maxstock_id; i++) {
                    MIGRATION[species].current_pop_ratio[cohort][b] += bm->tempPopRatio[i][species][cohort][b];
                }
                MIGRATION[species].current_pop_ratio[cohort][b] /= ((double)(FunctGroupArray[species].numStocks));  // Take an average as so rare to have > one stock of migrators
                //TODO: Refine in future

            }
        }
    }
    

	/* If aging alone then ended, if has to recruit also then set flag to -1 to signal already
	 done age distribution recalculation */
	if (need_recupdate)
		FunctGroupArray[species].speciesParams[need_update_id] = -1;
	else
		FunctGroupArray[species].speciesParams[need_update_id] = 0;

	if(rec_related) {
		FunctGroupArray[species].speciesParams[need_update_id] = 0;
		FunctGroupArray[species].speciesParams[need_recupdate_id] = 0;
	}

	return;
}



/**
 *
 *	\brief Routine to prepare for aging (if done after process is begun then can
 *	have zero entries so update fails as not completed again up so can't
 *	determine correct distribution.
 *
 */
void Prepare_Age_Distrib(MSEBoxModel *bm, int species, FILE *llogfp) {
	int ij, k, stock_id, cohort, den = -1;

	int maxstock = FunctGroupArray[species].numStocks;

	/* Initialise */
	for (cohort = 0; cohort < FunctGroupArray[species].numCohortsXnumGenes; cohort++) {
		totden[species][cohort] = 0;
		for (k = 0; k < maxstock; k++) {
			AGE_stock_struct_prop[species][cohort][k] = 0;
		}
		for (k = 0; k < bm->wcnz; k++) {
			for (ij = 0; ij < bm->nbox; ij++) {
				FunctGroupArray[species].AGEnewden[cohort][k][ij] = 0.0;
			}
		}
	}

	/* Get spatial distributions */
	for (ij = 0; ij < bm->nbox; ij++) {
		if (bm->boxes[ij].type == LAND) {
			/* Identify the stock under consideration */
			stock_id = bm->group_stock[species][ij][0];

			/* Update density and current stock distribution */
			for (cohort = 0; cohort < FunctGroupArray[species].numCohortsXnumGenes; cohort++) {

				switch (FunctGroupArray[species].groupAgeType) {
					case AGE_STRUCTURED:
						den = FunctGroupArray[species].NumsTracers[cohort];
						break;
					case AGE_STRUCTURED_BIOMASS:
					case BIOMASS:
						/* Do nothing */
						quit("How did we get here for a biomass or age structured group only group?\n");
						break;
				}
				totden[species][cohort] += bm->boxes[ij].tr[0][den];
				AGE_stock_struct_prop[species][cohort][stock_id] += bm->boxes[ij].tr[0][den];

				/* Begin constructing spatial distribution */
				FunctGroupArray[species].AGEnewden[cohort][0][ij] = bm->boxes[ij].tr[0][den];

			}
		}
		else if (bm->boxes[ij].type != BOUNDARY) {
			for (k = 0; k < bm->boxes[ij].nz; k++) {

				/* Identify the stock under consideration */
				stock_id = bm->group_stock[species][ij][k];

				/* Update density and current stock distribution */
				for (cohort = 0; cohort < FunctGroupArray[species].numCohortsXnumGenes; cohort++) {

					switch (FunctGroupArray[species].groupAgeType) {
						case AGE_STRUCTURED:
							den = FunctGroupArray[species].NumsTracers[cohort];
							break;
						case AGE_STRUCTURED_BIOMASS:
							den = FunctGroupArray[species].totNTracers[cohort];
							break;
						case BIOMASS:
							/* Do nothing */
							quit("How did we get here for a biomass only group?\n");
							break;
					}
					totden[species][cohort] += bm->boxes[ij].tr[k][den];
					AGE_stock_struct_prop[species][cohort][stock_id] += bm->boxes[ij].tr[k][den];

					/* Begin constructing spatial distribution */
					FunctGroupArray[species].AGEnewden[cohort][k][ij] = bm->boxes[ij].tr[k][den];

				}
			}
		}
	}

	/* Normalise distributions */
	for (cohort = 0; cohort < FunctGroupArray[species].numCohortsXnumGenes; cohort++) {
		for (ij = 0; ij < bm->nbox; ij++) {

			if (bm->boxes[ij].type == LAND) {
				FunctGroupArray[species].AGEnewden[cohort][0][ij] /= (totden[species][cohort] + small_num);
			}else{
				for (k = 0; k < bm->boxes[ij].nz; k++) {
					FunctGroupArray[species].AGEnewden[cohort][k][ij] /= (totden[species][cohort] + small_num);
				}
			}
		}
	}

	/* Get stock structure */
	for (k = 0; k < maxstock; k++) {
		for (cohort = 0; cohort < FunctGroupArray[species].numCohortsXnumGenes; cohort++) {
			AGE_stock_struct_prop[species][cohort][k] /= (totden[species][cohort] + small_num);
			if (!AGE_stock_struct_prop[species][cohort][k])
				AGE_stock_struct_prop[species][cohort][k] = small_num;
		}
	}

	return;
}

/**
 *	Routine to calculate spawning stock size when multiple stocks per group
 *
 */
void Determine_Stock_Prop(MSEBoxModel *bm, int species, int maxstock_id, int who_call, double *remnant, FILE *llogfp) {
	int n, stock_id;
	double scale, amt;
    double overall_tot = 0.0;

	for (n = 0; n < maxstock_id; n++)
		stock_prop[species][n] = 0.0;

	for (n = 0; n < FunctGroupArray[species].numCohortsXnumGenes; n++) {
		/* If spawning return only spawners in population */
		if (!who_call)
			scale = 1.0;
		else
			scale = FunctGroupArray[species].scaled_FSPB[n];

		for (stock_id = 0; stock_id < maxstock_id; stock_id++) {
			stock_prop[species][stock_id] += bm->stock_struct_prop[species][n][stock_id] * scale;
            overall_tot += bm->stock_struct_prop[species][n][stock_id] * scale;
		}
        
	}

	/* Find total amount */
	amt = 0;
    for (n = 0; n < maxstock_id; n++){
        if (!bm->flag_replicated_old) {
            /* Normalise */
            stock_prop[species][n] /= (overall_tot + small_num);
        }
		amt += stock_prop[species][n];
    }
	*remnant = amt;

	/* Error tracking *
	 if((bm->debug == debug_age) && ((sp == bm->which_check) || (sp == bm->move_check)) && (bm->dayt > bm->checkstart)){
	 for(n=0; n<maxstock_id; n++){
	 fprintf(llogfp,"%s stock%d %e\n",
	 FunctGroupArray[species].groupCode, n, stock_prop[species][n]);
	 }
	 }
	 */

	return;
}

/**
 * \brief Get The cohort stage of this group.
 *
 * If this group is a vertebrate group then compare the given cohort value to the age when
 * the group matures. If its greater than this age then its a adult else its a juvenile.
 * For the rest of the groups just return the given cohort value.
 *
 *
 * The age structured test used to be
 *
		if (chrt < FunctGroupArray[guildcase].speciesParams[age_mat_id])
			chrtstage = 0;
		else
			chrtstage = 1;
 *
 *
 */
//inline int Get_Cohort_Stage(MSEBoxModel *bm, int guildcase, int cohort) {
//	int chrtstage = 0;
//	int chrt = floor(cohort / FunctGroupArray[guildcase].numGeneTypes);
//	int pid;
//
//
//	if ((FunctGroupArray[guildcase].groupAgeType == AGE_STRUCTURED)
//			|| (FunctGroupArray[guildcase].groupAgeType == AGE_STRUCTURED_BIOMASS)){
//
//		for (pid = 0; pid < FunctGroupArray[guildcase].numStages; pid++) {
//			if (chrt >= FunctGroupArray[guildcase].age_mat[pid]){
//				chrtstage = pid;
//			}
//		}
//	} else {
//		chrtstage = chrt;
//	}
//
//	if(chrtstage != FunctGroupArray[guildcase].cohort_stage[cohort]){
//
//		printf("group %s, cohort %d, cohort_stage = %d, chrtstage= %d\n", FunctGroupArray[guildcase].groupCode, cohort, FunctGroupArray[guildcase].cohort_stage[cohort], chrtstage);
//		abort();
//	}
//	return chrtstage;
//}

/**
 *	\brief Routine to handle aging (and subsequent distribution of individuals into new age class
 *	- do this before movement so matches existing distribution with no discontinuities.
 *
 */
void Ecology_Update_Vertebrate_Cohorts(MSEBoxModel *bm, FILE *llogfp) {
    int ij, k, stock_id, species, cohort, age_mat, sn, rn, den, maxstock, need_update, sp_mignum, wk, do_debug, qid, cid, nextcid;
    //int stage, next_stage;
	double dennow, denup, snnow, rnnow, nextden, sp_AgeSize, oldden, oldSN, oldRN, finalmig, avgsn, avgrn, totmig, num_in_MIG_total, num_in_MIG;
	// double totsum, sp_AgeClassSize;
    //double diff;
	//double totalDen = 0.0;

	if (((bm->debug == debug_age) || (bm->debug == debug_general)) && (bm->dayt >= bm->checkstart))
		do_debug = 1;
	else
		do_debug = 0;

	for (species = 0; species < bm->K_num_tot_sp; species++) {
        //do_debug = 0;
        //if(species == 7)
        //    do_debug = 1;
        
		if (FunctGroupArray[species].speciesParams[flag_id] == TRUE && FunctGroupArray[species].isVertebrate == TRUE) {
            Util_Init_1D_Double(tot_new_mat, bm->K_num_max_cohort * bm->K_num_max_genetypes, 0.0);
            //Util_Init_1D_Int(mig_status, bm->K_num_max_stages, 0);
            
            /**
			if((bm->dayt >= bm->checkstart) && (sp == bm->which_check))
				do_debug = 1;
			else
				do_debug = 0;
            
            if (bm->which_check == species)
                do_debug = 1;
            
            if ((FunctGroupArray[species].numSpawns > 1) || (FunctGroupArray[species].groupType == FISH_INVERT))
                do_debug = 1;
             
             if((bm->dayt >= bm->checkstart) && (species == bm->which_check))
             do_debug = 1;

             if ((species == 64) || (species == 69))
             do_debug = 1;
            
            if((bm->dayt >= bm->checkstart) && (species == bm->which_check))
                do_debug = 1;
             **/

			need_update = (int) (FunctGroupArray[species].speciesParams[need_matupdate_id]);
			sp_AgeSize = (double)(FunctGroupArray[species].ageClassSize);
			maxstock = FunctGroupArray[species].numStocks;
			sp_mignum = MIGRATION[species].num_in_queue;
			age_mat = (int) (FunctGroupArray[species].speciesParams[age_mat_id]);

			if (need_update) {
                /* Record final numbers if needed */
                if(bm->M_est_method == Z_and_F_based) {
                    Record_End_Num(bm, species);
                }
                
				/* Initialise aging in general */
				for (cohort = 0; cohort < FunctGroupArray[species].numCohortsXnumGenes; cohort++) {
					for (k = 0; k < maxstock; k++) {

						/* tot_new_mat = sum of vertebrates aging in this cohort */
						tot_new_mat[cohort] += FunctGroupArray[species].agingVERT[cohort][k][DEN_id];

						/* Correct SN and RN to average rather than cumulative value */
						FunctGroupArray[species].agingVERT[cohort][k][SN_id] /= (FunctGroupArray[species].agingVERT[cohort][k][DEN_id] + small_num);
						FunctGroupArray[species].agingVERT[cohort][k][RN_id] /= (FunctGroupArray[species].agingVERT[cohort][k][DEN_id] + small_num);

						/**
						if (do_debug && (bm->which_check == species)) {
                        //if ((FunctGroupArray[species].groupType == MAMMAL) || (FunctGroupArray[species].groupType == FISH_INVERT)) {
							fprintf(llogfp, "Time: %e, Update_Cohorts %s-chrt_aged_into %d stock: %d, agingDEN: %e, agingSN: %e, agingRN: %e\n", bm->dayt,
									FunctGroupArray[species].groupCode, cohort, k, FunctGroupArray[species].agingVERT[cohort][k][DEN_id],
									FunctGroupArray[species].agingVERT[cohort][k][SN_id], FunctGroupArray[species].agingVERT[cohort][k][RN_id]);
						}
						**/
					}

					if (!tot_new_mat[cohort])
						tot_new_mat[cohort] = small_num;
				}

				/* Get stock structure */
				for (k = 0; k < maxstock; k++) {
					for (cohort = 0; cohort < FunctGroupArray[species].numCohortsXnumGenes; cohort++) {
						/* proportion of aging vertebrates aging in this stock */
						/* Proportion of total vertebrates aging into this cohort that are in this stock */
						spSTOCKprop[k][cohort] = FunctGroupArray[species].agingVERT[cohort][k][DEN_id] / tot_new_mat[cohort];
					}
				}

				/* Determine proportion of the population that is currently outside the model domain */
				if (sp_mignum) {
                    
                    // Was for (cohort = (((FunctGroupArray[species].numCohorts - 1) * FunctGroupArray[species].numGeneTypes) - 1); cohort > -1 ; cohort--)
                    // I think can actually be as written below but keeping it here for reference just in case
                    
                    
					for (cohort = FunctGroupArray[species].numCohortsXnumGenes - 1; cohort > -1 ; cohort--) { // Running backwards so don't need embedded stage check to see different migration status for mig_status (if run forward then get wrong answer for young_in_old_out as not done the later stages when do the younger
						finalmig = 0;
						avgsn = 0;
						avgrn = 0;
						MIGRATION[species].totprop_mig[cohort] = 0.0;
						cid = (int)(floor(cohort / FunctGroupArray[species].numGeneTypes));
						nextcid = cid + FunctGroupArray[species].numGeneTypes;
                        //stage = FunctGroupArray[species].cohort_stage[cohort];
                        //next_stage = FunctGroupArray[species].cohort_stage[nextcid];
                        
                        // For each migration group?
                        num_in_MIG_total = 0.0;
						for (qid = MIGRATION[species].num_in_queue_done; qid < MIGRATION[species].num_in_queue; qid++) {
                            num_in_MIG = MIGRATION[species].DEN[cohort][qid];
                            num_in_MIG_total += MIGRATION[species].DEN[cohort][qid];
                            
                            // Does prop_mig[cohort] have to be handled as migration_den[cohort+1] - as put against agingDEN which already cast to [cohort+1]? I don't think so as called form main loop so aging will have happened already on previous iteration of the model
                            // #TODO: check this assumption re cohort vs cohrot+1
                            MIGRATION[species].prop_mig[cohort][qid] = num_in_MIG / (totden[species][cohort] + num_in_MIG + small_num);
                        }
                        
                        MIGRATION[species].totprop_mig[cohort] += num_in_MIG_total / (totden[species][cohort] + num_in_MIG_total + small_num);
                            
                        if (MIGRATION[species].totprop_mig[cohort] > 1.0)
                            MIGRATION[species].totprop_mig[cohort] = 1.0;

                        /* Allocate those who would age-up into an age class outside the model domain
                        to the Migration array. Only do this if juveniles becoming adults and one away
                        while the other isn't
                         
                         Do allocation of aging into Migration array only when appropriate
                          - i.e. juveniles in model domain and adults not. If both outside model
                          domain then code in Update_Migration_Array() will handle it. Also if
                          juveniles are outside the domain and the adults are inside then this
                          is dealt with in Update_Migration_Array() too - see logic discussion
                          in Update_Migration_Array()
                         
                         By default mat_transition_thresh should be something high like 0.99 and juv_transition_thresh something lower like 0.1 or 0.5 etc, so this only comes into play when the majority of the adult popualtion is outside the model and juveniles not, but made them variables so more easily adjusted as needed
                         
                         */
                        if ((cohort < age_mat) && (nextcid >= age_mat)) {
                            if ((MIGRATION[species].totprop_mig[cohort] < bm->juv_transition_thresh) && (MIGRATION[species].totprop_mig[nextcid] > bm->mat_transition_thresh)) {
                                for (qid = MIGRATION[species].num_in_queue_done; qid < MIGRATION[species].num_in_queue; qid++) {

                                    /**
                                     if(bm->which_check == species){
                                        fprintf(llogfp, "Time: %e, Looking at %s-%d with totprop_mig-%d (juv) %e totprop_mig-%d (mat): %e juv_transition_thresh: %e mat_transition_thresh: %e\n", bm->dayt, FunctGroupArray[species].groupCode, cohort, MIGRATION[species].totprop_mig[cohort], MIGRATION[species].totprop_mig[nextcid],bm->juv_transition_thresh,  bm->mat_transition_thresh);
                                     }
                                     **/
                                
                                    // #TODO: Do we need to use MIGRATION[species].all_go[cohort][qid] here?
                                    if (FunctGroupArray[species].agingVERT[cohort][k][DEN_id] > 0) {
                                        warn("Juveniles have to suddenly flee domain as age for %s (time: %e)\n", FunctGroupArray[species].groupCode, bm->dayt);  // Warngin as doesn't really make sense to have to do this
                                    
                                        for (k = 0; k < maxstock; k++) {
                                            oldden = MIGRATION[species].DEN[nextcid][qid];
                                            oldSN = MIGRATION[species].SN[nextcid][qid];
                                            oldRN = MIGRATION[species].RN[nextcid][qid];
                                            MIGRATION[species].DEN[nextcid][qid] += MIGRATION[species].prop_mig[cohort][qid] * FunctGroupArray[species].agingVERT[cohort][k][DEN_id];
                                            MIGRATION[species].SN[nextcid][qid] = (oldden * oldSN + MIGRATION[species].prop_mig[cohort][qid]
												* FunctGroupArray[species].agingVERT[cohort][k][DEN_id] * FunctGroupArray[species].agingVERT[cohort][k][SN_id])
												/ (MIGRATION[species].DEN[nextcid][qid] + small_num);
                                            MIGRATION[species].RN[nextcid][qid] = (oldden * oldRN + MIGRATION[species].prop_mig[cohort][qid]
												* FunctGroupArray[species].agingVERT[cohort][k][DEN_id] * FunctGroupArray[species].agingVERT[cohort][k][RN_id])
												/ (MIGRATION[species].DEN[nextcid][qid] + small_num);
                                            totmig = MIGRATION[species].prop_mig[cohort][qid] * FunctGroupArray[species].agingVERT[cohort][k][DEN_id];
                                            finalmig += MIGRATION[species].prop_mig[cohort][qid] * FunctGroupArray[species].agingVERT[cohort][k][DEN_id];
                                            avgsn = (avgsn * (finalmig - totmig) + MIGRATION[species].SN[nextcid][qid] * totmig) / (finalmig + small_num);
                                            avgrn = (avgrn * (finalmig - totmig) + MIGRATION[species].RN[nextcid][qid] * totmig) / (finalmig + small_num);

                                            /**
                                            if (bm->which_check == species) {
                                                fprintf(llogfp,"Time: %e, %s-%d aged up: %e (prop: %e, agingVERT[%s][%d][%d]: %e)\n",
										 	 	 bm->dayt, FunctGroupArray[species].groupCode, cohort, (MIGRATION[species].prop_mig[cohort][qid] * FunctGroupArray[species].agingVERT[cohort][k][DEN_id]),
										 	 	 MIGRATION[species].prop_mig[cohort][qid], FunctGroupArray[species].groupCode, cohort, k, FunctGroupArray[species].agingVERT[cohort][k][DEN_id]);
                                                fprintf(llogfp,"%s-%d nextcid: %d, qid: %d, migSN: %e, oldden: %e, oldSN: %e, prop_mig: %e, agingVERTden: %e, agingVERTsn: %e, MIGden: %e\n",
												 FunctGroupArray[species].groupCode, cohort, nextcid, qid, MIGRATION[species].SN[nextcid][qid], oldden, oldSN, MIGRATION[species].prop_mig[cohort][qid],
												 FunctGroupArray[species].agingVERT[cohort][k][DEN_id], FunctGroupArray[species].agingVERT[cohort][k][SN_id], (MIGRATION[species].DEN[nextcid][qid] + small_num));
                                             }
                                             **/
                                        }
									}
								}
							}
							/* Update box zero so have record of actions in cdf */
							sn = FunctGroupArray[species].structNTracers[nextcid];
							rn = FunctGroupArray[species].resNTracers[nextcid];
							den = FunctGroupArray[species].NumsTracers[nextcid];

							for (wk = 0; wk < bm->boxes[0].nz; wk++) {
								bm->boxes[0].tr[wk][sn] = avgsn;
								bm->boxes[0].tr[wk][rn] = avgrn;
								bm->boxes[0].tr[wk][den] = finalmig;
							}
						}

                        /**
						if (do_debug && (bm->which_check == species)) {
                        //if ((FunctGroupArray[species].groupType == MAMMAL) || (FunctGroupArray[species].groupType == FISH_INVERT)) {
							fprintf(llogfp, "Time: %e, %s-%d had prop %e of population update to migration box\n", bm->dayt, FunctGroupArray[species].groupCode,
									cohort, MIGRATION[species].totprop_mig[cohort]);
						}
                        **/
					}
				}
                
				/* Apply newly mature individuals accordingly */
				//totsum = 0;
				for (cohort = 1; cohort < FunctGroupArray[species].numCohortsXnumGenes; cohort++) {

                    sn = FunctGroupArray[species].structNTracers[cohort];
					rn = FunctGroupArray[species].resNTracers[cohort];
					den = FunctGroupArray[species].NumsTracers[cohort];

					for (ij = 0; ij < bm->nbox; ij++) {
						if (bm->boxes[ij].type == LAND) {
							/* Identify the stock under consideration */
							stock_id = bm->group_stock[species][ij][0];

							denup = (1.0 - MIGRATION[species].totprop_mig[cohort]) * FunctGroupArray[species].agingVERT[cohort][stock_id][DEN_id]
									* FunctGroupArray[species].AGEnewden[cohort][0][ij] * spSTOCKprop[stock_id][cohort]
									/ AGE_stock_struct_prop[species][cohort][stock_id];
                            
                            if (denup < 0.0)
                                denup = 0.0;

							nextden = bm->boxes[ij].tr[0][den] + denup + small_num;
							dennow = bm->boxes[ij].tr[0][den];
							snnow = bm->boxes[ij].tr[0][sn];
							rnnow = bm->boxes[ij].tr[0][rn];

							if (nextden > 0 && MIGRATION[species].totprop_mig[cohort] < 0.99) {
								bm->boxes[ij].tr[0][sn] = (dennow * snnow + FunctGroupArray[species].agingVERT[cohort][stock_id][SN_id] * denup) / nextden;
								bm->boxes[ij].tr[0][rn] = (dennow * rnnow + FunctGroupArray[species].agingVERT[cohort][stock_id][RN_id] * denup) / nextden;
							}
							bm->boxes[ij].tr[0][den] = nextden;
                            
                            if ((do_debug) || (!_finite(bm->boxes[ij].tr[0][den]))) {
								fprintf(llogfp, "Update1-Land Time: %e, box%d-%d %s-chrt%d, dennow: %e, snnow: %e, aging-stock%d-SN: %e, denup: %e, nextden: %e\n",
										bm->dayt, ij, 0, FunctGroupArray[species].groupCode, cohort, dennow, snnow, stock_id,
										FunctGroupArray[species].agingVERT[cohort][stock_id][SN_id], denup, nextden);
								fprintf(llogfp, "Time: %e, box: %d-%d %s-chrt%d den: %e (dennow: %e, agingDEN: %e, AGEnewden: %e, stockratio: %e)\n",
										bm->dayt, ij, 0, FunctGroupArray[species].groupCode, cohort, bm->boxes[ij].tr[0][den], dennow,
										FunctGroupArray[species].agingVERT[cohort][stock_id][DEN_id], FunctGroupArray[species].AGEnewden[cohort][0][ij],
										spSTOCKprop[stock_id][cohort] / AGE_stock_struct_prop[species][cohort][stock_id]);
								fprintf(llogfp, "Time: %e, box%d-%d %s-chrt%d, denratio: %e, snratio: %e, rnratio: %e\n", bm->dayt, ij, 0,
										FunctGroupArray[species].groupCode, cohort, bm->boxes[ij].tr[0][den] / (dennow + small_num), bm->boxes[ij].tr[0][sn]
												/ snnow, bm->boxes[ij].tr[0][rn] / rnnow);
							}
                             
							if((!_finite(bm->boxes[ij].tr[0][den]))){
								quit("Ecology_Update_Vert_Cohorts %s-%d has nan numbers in box %d-%d\n", FunctGroupArray[species].groupCode, cohort, ij, 0);
							}


							/* For those groups where the age class is less than a year long, for
							 those age records where there would be a "zero" entry (the individuals
							 aging out to the next phase already) keep the structural and reserve
							 weights of the previous members for record keeping purposes (avoid
							 zigzag netcdf files). This should have little effect on the aging calcs
							 as multiple by density (very close to zero) so negligible effect
							 when are actually aging things rather than "fiddling with zeros" */
							if (sp_AgeSize < 1 && bm->boxes[ij].tr[0][sn] < bm->min_pool * 10.0)
								bm->boxes[ij].tr[0][sn] = snnow;
							if (sp_AgeSize < 1 && bm->boxes[ij].tr[0][rn] < bm->min_pool * 10.0)
								bm->boxes[ij].tr[0][rn] = rnnow;
                            
                            /* Note that age distributions have already been updated in Vertebrate_Reproduction */

						} else if (bm->boxes[ij].type != BOUNDARY) {
							for (k = 0; k < bm->boxes[ij].nz; k++) {

								/* Identify the stock under consideration */
								stock_id = bm->group_stock[species][ij][k];


								/* Update relevant pool values - needn't do youngest age class as can't age up
								 into it	(their settlement is all handed back in Vertebrate_Reproduction()).
								 Note that totprop_mig is the proportion of the updated ages that need
								 to go into the Migration Array instead of these boxes here */

								/*
								 * 	FunctGroupArray[species].agingVERT[cohort][stock_id][DEN_id] = number of vertebrates in this stock that are aging into
								 *  the this cohort.
								 *
								 *
								 *  FunctGroupArray[species].AGEnewden[cohort][k][ij] = proportion of the vertebrates in this box/layer.
								 *
								 *  spSTOCKprop[stock_id][cohort] =  Proportion of total vertebrates aging into this cohort that are in this stock
								 *
								 *
								 *  AGE_stock_struct_prop = total number of vertebrates in cohort ageclass that are in this stock.
								 *
								 *
								 */
								denup = (1.0 - MIGRATION[species].totprop_mig[cohort]) * FunctGroupArray[species].agingVERT[cohort][stock_id][DEN_id]
										* FunctGroupArray[species].AGEnewden[cohort][k][ij] * spSTOCKprop[stock_id][cohort]
										/ AGE_stock_struct_prop[species][cohort][stock_id];

                                if (denup < 0.0)
                                    denup = 0.0;
                                
                                nextden = bm->boxes[ij].tr[k][den] + denup + small_num;
								dennow = bm->boxes[ij].tr[k][den];
								snnow = bm->boxes[ij].tr[k][sn];
								rnnow = bm->boxes[ij].tr[k][rn];

								if (nextden > 0 && MIGRATION[species].totprop_mig[cohort] < 0.99) {
									bm->boxes[ij].tr[k][sn] = (dennow * snnow + FunctGroupArray[species].agingVERT[cohort][stock_id][SN_id] * denup) / nextden;
									bm->boxes[ij].tr[k][rn] = (dennow * rnnow + FunctGroupArray[species].agingVERT[cohort][stock_id][RN_id] * denup) / nextden;
                                    
                                    if(bm->track_contaminants){
                                        Age_Contaminants_Update(bm, species, cohort, denup, dennow, nextden, ij, k);
                                    }
								}
								bm->boxes[ij].tr[k][den] = nextden;

								if ((do_debug) || (!_finite(bm->boxes[ij].tr[k][den])) || (bm->boxes[ij].tr[k][den] < 0.0)) {
                                    fprintf(llogfp, "Update2-Marine Time: %e, box%d-%d %s-chrt%d, dennow: %e, snnow: %e, aging-stock%d-SN: %e, totprop_mig: %e, denup: %e, nextden: %e\n",
											bm->dayt, ij, k, FunctGroupArray[species].groupCode, cohort, dennow, snnow, stock_id,
											FunctGroupArray[species].agingVERT[cohort][stock_id][SN_id], MIGRATION[species].totprop_mig[cohort], denup, nextden);
									fprintf(llogfp, "Time: %e, box: %d-%d %s-chrt%d den: %e (dennow: %e, agingDEN: %e, AGEnewden: %e, stockratio: %e)\n",
											bm->dayt, ij, k, FunctGroupArray[species].groupCode, cohort, bm->boxes[ij].tr[k][den], dennow,
											FunctGroupArray[species].agingVERT[cohort][stock_id][DEN_id], FunctGroupArray[species].AGEnewden[cohort][k][ij],
											spSTOCKprop[stock_id][cohort] / AGE_stock_struct_prop[species][cohort][stock_id]);
									fprintf(llogfp, "Time: %e, box%d-%d %s-chrt%d, denratio: %e, snratio: %e, rnratio: %e\n", bm->dayt, ij, k,
											FunctGroupArray[species].groupCode, cohort, bm->boxes[ij].tr[k][den] / (dennow + small_num), bm->boxes[ij].tr[k][sn] / snnow, bm->boxes[ij].tr[k][rn] / rnnow);
								}
								if((!_finite(bm->boxes[ij].tr[k][den])) || (bm->boxes[ij].tr[k][den] < 0.0)){
									quit("Ecology_Update_Vert_Cohorts %s-%d has nan numbers or is a negative in box %d-%d\n", FunctGroupArray[species].groupCode, cohort, ij, k);
								}
                                
                                /**
                                //if ((species == 64) || (species == 69)) {
                                //if (species == 22) {
                                if (species == 35) {
                                    fprintf(llogfp, "Update2-Marine Time: %e, box%d-%d %s-chrt%d, dennow: %e, snnow: %e, aging-stock%d-SN: %e, totprop_mig: %e, denup: %e, nextden: %e\n",
                                            bm->dayt, ij, k, FunctGroupArray[species].groupCode, cohort, dennow, snnow, stock_id,
                                            FunctGroupArray[species].agingVERT[cohort][stock_id][SN_id], MIGRATION[species].totprop_mig[cohort], denup, nextden);
                                    fprintf(llogfp, "Time: %e, box: %d-%d %s-chrt%d den: %e (dennow: %e, agingDEN: %e, AGEnewden: %e, stockratio: %e)\n",
                                            bm->dayt, ij, k, FunctGroupArray[species].groupCode, cohort, bm->boxes[ij].tr[k][den], dennow,
                                            FunctGroupArray[species].agingVERT[cohort][stock_id][DEN_id], FunctGroupArray[species].AGEnewden[cohort][k][ij],
                                            spSTOCKprop[stock_id][cohort] / AGE_stock_struct_prop[species][cohort][stock_id]);
                                    fprintf(llogfp, "Time: %e, box%d-%d %s-chrt%d, denratio: %e, snratio: %e, rnratio: %e\n", bm->dayt, ij, k,
                                            FunctGroupArray[species].groupCode, cohort, bm->boxes[ij].tr[k][den] / (dennow + small_num), bm->boxes[ij].tr[k][sn] / snnow, bm->boxes[ij].tr[k][rn] / rnnow);
                                }

                                **/


								/* For those groups where the age class is less than a year long, for
								 those age records where there would be a "zero" entry (the individuals
								 aging out to the next phase already) keep the structural and reserve
								 weights of the previous members for record keeping purposes (avoid
								 zigzag netcdf files). This should have little effect on the aging calcs
								 as multiple by density (very close to zero) so negligible effect
								 when are actually aging things rather than "fiddling with zeros" */
								if (sp_AgeSize < 1 && bm->boxes[ij].tr[k][sn] < bm->min_pool * 10.0)
									bm->boxes[ij].tr[k][sn] = snnow;
								if (sp_AgeSize < 1 && bm->boxes[ij].tr[k][rn] < bm->min_pool * 10.0)
									bm->boxes[ij].tr[k][rn] = rnnow;
                                
                                /* Note that age distributions have already been updated in Vertebrate_Reproduction */
							}
						}
					}
				}

				/**
				if (species == bm->which_check) {
					double grandTotal = 0.0;
					for (cohort = 0; cohort < FunctGroupArray[species].numCohortsXnumGenes; cohort++) {
						totalDen = 0.0;
						den = FunctGroupArray[species].NumsTracers[cohort];
						for (ij = 0; ij < bm->nbox; ij++) {
							if (bm->boxes[ij].type != BOUNDARY) {
								//diff = bm->wcnz - bm->boxes[ij].nz;
								for (k = 0; k < bm->boxes[ij].nz; k++) {
									totalDen += bm->boxes[ij].tr[k][den];
								}
							}
						}

						grandTotal += totalDen;
						fprintf(llogfp, "Update_Aged_Cohort %s;%d = %e\n", FunctGroupArray[species].groupCode, cohort, totalDen);
					}
					fprintf(llogfp, "Update_Aged_Cohort %s = %e\n", FunctGroupArray[species].groupCode, grandTotal);
				}
				**/

				/* Reset update */
				FunctGroupArray[species].speciesParams[need_matupdate_id] = 0;

				/** Reset the mortality estimates **/
				Reset_Mortality(bm, species);

			}
		}
	}

	if (do_debug)
		Check_Gape(bm, llogfp);

	return;

}

/************************************************************************************************************************************
 * Resetting the mortality
 */
void Record_End_Num(MSEBoxModel *bm, int species) {
    int ij, k, cohort, stock_id, den, fished_chrt;
    double chrt_biomass;
    double biomass = 0.0;
    
    for(cohort = FunctGroupArray[species].numGeneTypes; cohort<FunctGroupArray[species].numCohortsXnumGenes; cohort++){
        fished_chrt = (int) (FunctGroupArray[species].speciesParams[Age95pcntV_id]);
        if (fished_chrt < ((FunctGroupArray[species].numCohortsXnumGenes) / 2))
            fished_chrt = ((FunctGroupArray[species].numCohortsXnumGenes) / 2);

        // Now initialise the calcN and starting numbers in bm->calcTrackedMort - Zero here... Recruits done in reproduction step
        for (stock_id = 0; stock_id < FunctGroupArray[species].numStocks; stock_id++) {
            bm->calcTrackedMort[species][cohort][stock_id][endNum_id] = 0.0;
        }
        
        if(FunctGroupArray[species].groupAgeType == AGE_STRUCTURED) {
            den = FunctGroupArray[species].NumsTracers[cohort];
            for (ij = 0; ij < bm->nbox; ij++) {
                if (bm->boxes[ij].type != BOUNDARY) {
                    for (k = 0; k < bm->boxes[ij].nz; k++) {
                        if (fished_chrt == cohort )
                            biomass += bm->boxes[ij].tr[k][den];
                        
                        stock_id = bm->group_stock[species][ij][k];
                        
                        bm->calcTrackedMort[species][cohort][stock_id][endNum_id] += bm->boxes[ij].tr[k][den];
                    }
                }
            }
        } else {
            den = FunctGroupArray[species].totNTracers[cohort];
            for (ij = 0; ij < bm->nbox; ij++) {
                // Loop through each pelagic layer
                for (k = 0; k < bm->boxes[ij].nz; k++) {
                    
                    stock_id = bm->group_stock[species][ij][k];
                    
                    chrt_biomass = bm->boxes[ij].tr[k][den] * bm->boxes[ij].dz[k] * bm->boxes[ij].area * FunctGroupArray[species].habitatCoeffs[WC];
                    bm->calcTrackedMort[species][cohort][stock_id][endNum_id] += chrt_biomass;
                }
                // Set relk for sediment and epibenthos by assuming its the one in deepest vertical layer
                stock_id = bm->group_stock[species][ij][0]; // As assuming k = 0
                
                // Loop through each sediment layer
                for (k = 0; k < bm->boxes[ij].sm.nz; k++) {
                    chrt_biomass = bm->boxes[ij].sm.tr[k][den] * bm->boxes[ij].sm.dz[k] * bm->boxes[ij].area * FunctGroupArray[species].habitatCoeffs[SED];
                    bm->calcTrackedMort[species][cohort][stock_id][endNum_id] += chrt_biomass;
                }
                // Loop over epibenthic layer
                if (FunctGroupArray[species].habitatCoeffs[EPIFAUNA] > 0) {
                    chrt_biomass = bm->boxes[ij].epi[den] * bm->boxes[ij].area * FunctGroupArray[species].habitatCoeffs[EPIFAUNA];
                    bm->calcTrackedMort[species][cohort][stock_id][endNum_id] += chrt_biomass;
                }
            }
        }
    }
    
    return;
}

/************************************************************************************************************************************
 * Resetting the mortality
 */
void Reset_Mortality(MSEBoxModel *bm, int species) {
	int pred, fished_chrt, cohort, k, ij, stock_id, den;
	int maxstock = FunctGroupArray[species].numStocks;
	double biomass, chrt_biomass;

	/* Get starting numbers for pseduo assessment */
	fished_chrt = (int) (FunctGroupArray[species].speciesParams[Age95pcntV_id]);
	if (fished_chrt < ((FunctGroupArray[species].numCohortsXnumGenes) / 2))
		fished_chrt = ((FunctGroupArray[species].numCohortsXnumGenes) / 2);

	if (fished_chrt >= FunctGroupArray[species].numGeneTypes) {
		/* Store last year's records in case they're needed in the assessment */
		bm->calcMnum[species][hist_id] = bm->calcMnum[species][expect_id];
		bm->calcFnum[species][hist_id] = bm->calcFnum[species][expect_id];
		bm->calcNstart[species][hist_id] = bm->calcNstart[species][expect_id];
		FunctGroupArray[species].speciesParams[calcupdate_date_id] = bm->TofY;
		FunctGroupArray[species].speciesParams[calcPerPredupdate_date_id] = bm->TofY;

		bm->calcNstartPerPred[species][hist_id] = bm->calcNstartPerPred[species][expect_id];
		for (pred = 0; pred < bm->K_num_tot_sp; pred++) {
			bm->calcMnumPerPred[species][pred][hist_id] = bm->calcMnumPerPred[species][pred][expect_id];
			bm->calcMnumPerPred[species][pred][expect_id] = 0;
		}

		/* Update the linear mortality, quad mortality and mortality due to explict birds and mammals */
		bm->calcMLinearMort[species][hist_id] = bm->calcMLinearMort[species][expect_id];
		bm->calcMLinearMort[species][expect_id] = 0;

		bm->calcMQuadMort[species][hist_id] = bm->calcMQuadMort[species][expect_id];
		bm->calcMQuadMort[species][expect_id] = 0;

		bm->calcMPredMort[species][hist_id] = bm->calcMPredMort[species][expect_id];
		bm->calcMPredMort[species][expect_id] = 0;

		/* Initialise mortality trackers for pseudo assessment */
		bm->calcMnum[species][expect_id] = 0;
		bm->calcFnum[species][expect_id] = 0;

		/* Set starting cohort size for pseduo assessment - Zero for update below (otherwise done in recruitment) */
		bm->calcNstart[species][expect_id] = 0;
	}

	/* For purposes of PerPred calculations sum on all these new individuals too */
	biomass = 0;
	for(cohort = FunctGroupArray[species].numGeneTypes; cohort<FunctGroupArray[species].numCohortsXnumGenes; cohort++){

		if(bm->track_contaminants){
			Contaminant_Update_ContactMort_Record(bm, species, cohort);
		}

		for (k = 0; k < maxstock; k++) {
			/* Sort out the mortality estimates and initialise for next year */
			bm->calcTrackedMort[species][cohort][k][finalM1_id] = bm->calcTrackedMort[species][cohort][k][ongoingM1_id] / (bm->calcTrackedMort[species][cohort][k][start_id] + small_num);
			bm->calcTrackedMort[species][cohort][k][finalM2_id] = bm->calcTrackedMort[species][cohort][k][ongoingM2_id] / (bm->calcTrackedMort[species][cohort][k][start_id] + small_num);
			bm->calcTrackedMort[species][cohort][k][finalF_id] = bm->calcTrackedMort[species][cohort][k][ongoingF_id] / (bm->calcTrackedMort[species][cohort][k][start_id] + small_num);

			for(pred=0; pred < bm->K_num_tot_sp; pred++){
				bm->calcTrackedPredMort[species][cohort][k][pred][final_id] = bm->calcTrackedPredMort[species][cohort][k][pred][ongoing_id] / (bm->calcTrackedMort[species][cohort][k][start_id] + small_num);
				bm->calcTrackedPredMort[species][cohort][k][pred][ongoing_id] = 0.0;
			}
            
			bm->calcTrackedMort[species][cohort][k][ongoingM1_id] = 0.0;
			bm->calcTrackedMort[species][cohort][k][ongoingM2_id] = 0.0;
			bm->calcTrackedMort[species][cohort][k][ongoingF_id] = 0.0;

			bm->calcTrackedMort[species][cohort][k][start_id] = 0.0;
            
            if (!k) {
                FunctGroupArray[species].min_B[cohort] = MAXDOUBLE;
                FunctGroupArray[species].max_B[cohort] = MINDOUBLE;
                
                if(FunctGroupArray[species].groupAgeType == AGE_STRUCTURED) {
                    FunctGroupArray[species].min_wgt[cohort] = MAXDOUBLE;
                    FunctGroupArray[species].max_wgt[cohort] = MINDOUBLE;
                }
            }
		}

		// Now initialise the calcN and starting numbers in bm->calcTrackedMort - Zero here... Recruits done in reproduction step
		if(FunctGroupArray[species].groupAgeType == AGE_STRUCTURED) {
			den = FunctGroupArray[species].NumsTracers[cohort];
			for (ij = 0; ij < bm->nbox; ij++) {
				if (bm->boxes[ij].type != BOUNDARY) {
					for (k = 0; k < bm->boxes[ij].nz; k++) {
						if (fished_chrt == cohort )
							biomass += bm->boxes[ij].tr[k][den];

						stock_id = bm->group_stock[species][ij][k];

						bm->calcTrackedMort[species][cohort][stock_id][start_id] += bm->boxes[ij].tr[k][den];
					}
				}
			}
		} else {
			den = FunctGroupArray[species].totNTracers[cohort];
			for (ij = 0; ij < bm->nbox; ij++) {
				// Loop through each pelagic layer
				for (k = 0; k < bm->boxes[ij].nz; k++) {

					stock_id = bm->group_stock[species][ij][k];

					biomass += bm->boxes[ij].tr[k][den] * bm->boxes[ij].dz[k] * bm->boxes[ij].area * FunctGroupArray[species].habitatCoeffs[WC];
					chrt_biomass = bm->boxes[ij].tr[k][den] * bm->boxes[ij].dz[k] * bm->boxes[ij].area * FunctGroupArray[species].habitatCoeffs[WC];
					bm->calcTrackedMort[species][cohort][stock_id][start_id] += chrt_biomass;
				}
				// Set relk for sediment and epibenthos by assuming its the one in deepest vertical layer
				stock_id = bm->group_stock[species][ij][0]; // As assuming k = 0

				// Loop through each sediment layer
				for (k = 0; k < bm->boxes[ij].sm.nz; k++) {
					biomass += bm->boxes[ij].sm.tr[k][den] * bm->boxes[ij].sm.dz[k] * bm->boxes[ij].area * FunctGroupArray[species].habitatCoeffs[SED];
					chrt_biomass = bm->boxes[ij].sm.tr[k][den] * bm->boxes[ij].sm.dz[k] * bm->boxes[ij].area * FunctGroupArray[species].habitatCoeffs[SED];
					bm->calcTrackedMort[species][cohort][stock_id][start_id] += chrt_biomass;
				}
				// Loop over epibenthic layer
				if (FunctGroupArray[species].habitatCoeffs[EPIFAUNA] > 0) {
					biomass += bm->boxes[ij].epi[den] * bm->boxes[ij].area * FunctGroupArray[species].habitatCoeffs[EPIFAUNA];
					chrt_biomass = bm->boxes[ij].epi[den] * bm->boxes[ij].area * FunctGroupArray[species].habitatCoeffs[EPIFAUNA];
					bm->calcTrackedMort[species][cohort][stock_id][start_id] += chrt_biomass;
				}
			}
		}
	}
	bm->calcNstart[species][expect_id] = biomass;
	bm->calcNstartPerPred[species][expect_id] = biomass;
    
    FunctGroupArray[species].updatedDiet = 2;

	return;
}

/*********************************************************************************************************************************************************
 * Invertebrate Reproduction
 */

/**
 *	\brief Invertebrate aging and reproduction in age structured vertebrates
 *
 */
void Invertebrate_Reproduction(MSEBoxModel *bm, int wclayer, int maxdeep, int totaldeep, double *localWCTracers, FILE *llogfp) {

	double biomass_newly_mature, spawned_biomass, spawn_active, spawning_biomass, plankton, CHLa, vertdistrib,
		p_ageup, this_biom, amt, prop_biom_spawn_lost, spawning_biomass_contrib;
	double enviro_scalar = 1.0;
	int species, sp_Spawn_Now, sp_Age_Now, sp_active, flagSP, do_debug, additional_day, maxstock_id, did_something, num_cohorts, stock_id, sp_checkday,
		flagmother, ngene, stk, sp_spawnday, stuff_to_do;
	int ij = bm->current_box;
	int cohort, qid, i;
	int overall_checkday = (int) (floor(bm->dayt));
	int basecohort;
	double recruitment_scalar;

	/* Local biomass values */
	CHLa = localWCTracers[Chl_a_i];

	/* Plankton in this sense represents the plankton groups that are really
	 squid larvae or feed on by squid larvae */
	plankton = Ecology_Get_Plankton(bm, localWCTracers, llogfp);

	for (species = 0; species < bm->K_num_tot_sp; species++) {
		if ((FunctGroupArray[species].speciesParams[flag_id] == TRUE) && (FunctGroupArray[species].groupAgeType == AGE_STRUCTURED_BIOMASS)) {
			/* Define num_cohorts = FunctGroupArray[species].numCohorts so that when use it as index to call
			 value from vector have already corrected for the fact index starts at 0 not 1.
			 */
			num_cohorts = FunctGroupArray[species].numCohorts - 1;

			if (bm->debug && (((bm->debug == debug_spawn) || (bm->debug == debug_age) || (bm->debug == debug_general)) && (bm->which_check == species))) {
				do_debug = 1;
			} else
				do_debug = 0;
			
			//if(((bm->current_box == bm->checkbox) || (bm->dayt >= bm->checkstart)) && (bm->which_check == species))
			//	do_debug = 1;
            
            //if (bm->which_check == species)
            //    do_debug = 1;


			/******************** Initialisation ********************/
			if (FunctGroupArray[species].speciesParams[flag_id] == TRUE) {
                for (cohort = 0; cohort < FunctGroupArray[species].numCohortsXnumGenes; cohort++) {
                    for (i = 0; i < 3; i++)
                        shiftVERT[species][cohort][i] = 0.0;
                    shiftVERTON[species][cohort] = 0;
                }
			}

			/* Get parameters */
			flagSP = (int) (FunctGroupArray[species].speciesParams[flag_id]);
			sp_active = (int) (FunctGroupArray[species].speciesParams[flagactive_id]);
			spawn_active = FunctGroupArray[species].speciesParams[FSP_id];
			maxstock_id = FunctGroupArray[species].numStocks;
			flagmother = (int) (FunctGroupArray[species].speciesParams[flagmother_id]);
	        prop_biom_spawn_lost = FunctGroupArray[species].speciesParams[prop_spawn_lost_id];

			sp_active = Ecology_Sp_Active(bm, species, llogfp);

			/* ID stock recruits to enter - assume stock fidelity (i.e. add to adult stock
			 that spawned them */
			stock_id = bm->group_stock[species][bm->current_box][wclayer];

			if (!sp_active || !flagSP) {
				/* Not active now so move onto next group*/
				continue;
			}

			/* Initialisation */
			sp_Spawn_Now = 0;
			sp_Age_Now = 0;
			biomass_newly_mature = 0;
			spawned_biomass = 0;
			spawning_biomass = 0;
			spawning_biomass_contrib = 0;
			additional_day = 0;
			did_something = 0;
			p_ageup = 0;
			Ecology_Init_Spawn_Params(bm, species, maxstock_id, wclayer, llogfp);

			/* Determine if time to spawn or age */
			for (cohort = 0; cohort < FunctGroupArray[species].numCohortsXnumGenes; cohort++) {
				basecohort = (int)(floor(cohort / FunctGroupArray[species].numGeneTypes));
				ngene = cohort - (basecohort * FunctGroupArray[species].numGeneTypes);
				this_biom = localWCTracers[FunctGroupArray[species].totNTracers[cohort]];

				if (FunctGroupArray[species].recruitType)
					additional_day = 0;
				else
					additional_day = 1;

				sp_checkday = EMBRYO[species].Spawn_Now[cohort][EMBRYO[species].next_spawn];
				qid = EMBRYO[species].next_spawn;
				if (overall_checkday == sp_checkday) {
					sp_Spawn_Now = 1;
				}
				sp_spawnday = sp_checkday;

				sp_checkday = EMBRYO[species].Age_Now[cohort][EMBRYO[species].next_age];
				if (overall_checkday == sp_checkday) {
					sp_Age_Now = 1;
				}

				stuff_to_do = 0;
				if((overall_checkday >= sp_spawnday) && (overall_checkday <= (sp_spawnday + (additional_day * 2.0) + 1))){
					stuff_to_do = 1;
				}

                /*
				if (do_debug && (bm->which_check == species))
					fprintf(llogfp, "day %d, %s-%d box: %d-%d starting_biom: %e, sp_Spawn_Now = %d, sp_Age_Now: %d, spawn_day: %d, age_day: %d, overall_checkday: %d, sp_spawnday: %d, additional_day: %d, stuff_to_do: %d\n", 
						overall_checkday, FunctGroupArray[species].groupCode, cohort, bm->current_box, bm->current_layer, this_biom, sp_Spawn_Now, sp_Age_Now, EMBRYO[species].Spawn_Now[cohort][EMBRYO[species].next_spawn], EMBRYO[species].Age_Now[cohort][EMBRYO[species].next_age], overall_checkday, sp_spawnday, additional_day, stuff_to_do);
                */
                
				if (!sp_Spawn_Now && !sp_Age_Now && !stuff_to_do) {
					/* Not spawning or aging now so don't do anything */
					continue;
				}

				/* Initialise YOY sum - including aggregate one stored in maxstock_id slot */
				if (sp_Spawn_Now && ((bm->current_box == bm->first_box) && (bm->current_layer == bm->top_layer))) {
					for ( stk = 0; stk < maxstock_id + 1; stk++) {
						tot_yoy[species][stk] = 0.0;
					}
				}

				/**** Spawn *****/
				if (sp_Spawn_Now) {
					/* Adult spawning biomass */
					spawning_biomass = spawn_active * this_biom * FunctGroupArray[species].scaled_FSPB[cohort];

					/* Spawning - external to model domain (by migrating adults) */
					spawning_biomass += Ecology_Do_External_InVert_Spawning(bm, species, cohort, do_debug, llogfp);

                    /*
					if (do_debug && (bm->which_check == species))
						fprintf(llogfp, "%s-%d box: %d-%d, spawning_biomass = %e spawn_active: %e this_biom: %e, scaled_FSPB: %e\n", FunctGroupArray[species].groupCode, cohort, bm->current_box, bm->current_layer, spawning_biomass, spawn_active, this_biom, FunctGroupArray[species].scaled_FSPB[cohort]);
                     */
                    
					/* Spawning - internal to model domain */
					Ecology_Find_Invert_Spawn(bm, species, spawning_biomass, stock_id, plankton, CHLa, llogfp);

					/* Now spawning is started turn the switch off (so any once off events not repeated) */
					EMBRYO[species].readytospawn[stock_id] = 0;

					/* Did something */
					did_something = 1;
				}

                /*
				if (do_debug && (bm->which_check == species))
					fprintf(llogfp, "%s-%d box: %d-%d, overall_checkday: %d, sp_spawnday: %d, additional_day: %d\n", FunctGroupArray[species].groupCode, cohort, bm->current_box, bm->current_layer, overall_checkday, sp_spawnday, additional_day);
                 */
                
				/* Reset spawning readiness - can't do it at the time due to multiple layer handling */
				if (overall_checkday == (sp_spawnday + additional_day)) {
					EMBRYO[species].readytospawn[stock_id] = 2; // Reset so ready for next spawning event (now this one is done)

					/* Determine population spawning */
					Determine_Stock_Prop(bm, species, maxstock_id, 1, &amt, llogfp);

					/* Get base level for recruitment */
					spawned_biomass = Ecology_Find_Invert_Recruits(bm, species, ngene, stock_id, plankton, llogfp);

                    /*
					if (do_debug && (bm->which_check == species))
						fprintf(llogfp, "%s-%d box: %d-%d, spawned_biomass = %e ", FunctGroupArray[species].groupCode, cohort, bm->current_box, bm->current_layer, spawned_biomass);
                     */
                    
					/* Check for environmental effects */
					enviro_scalar = 1.0;
					enviro_scalar = Get_Enviro_Recruit_Forcing(bm, species, do_debug, llogfp);
					spawned_biomass *= enviro_scalar;

                    /*
					if (do_debug && (bm->which_check == species))
						fprintf(llogfp, "enviro_scalar = %e ", enviro_scalar);
                     */
                    
					/* Check for vertical distribution of new recruits */
					if (FunctGroupArray[species].recruitType) {
						vertdistrib = 1.0;
					} else {
						vertdistrib = Get_Vertical_Recruit_Position(species, wclayer, maxdeep, totaldeep);
					}
					spawned_biomass *= vertdistrib;

					/* Recruitment scaling due to external files - added for Cam for the GOC model */
					recruitment_scalar = Ecology_Get_Recruitment_Scalar(bm, species);
					if (bm->debug == debug_external_scaling && (bm->which_check == species)) {
						fprintf(bm->logFile, "Time: %e, External recruitment scaling of group %s in box %d:%d. Scaling - %e. Spawning numbers before scaling = %e, after = %e\n",
								bm->dayt, FunctGroupArray[species].groupCode, bm->current_box, bm->current_layer, recruitment_scalar, spawned_biomass, spawned_biomass * recruitment_scalar );
					}
					spawned_biomass *= recruitment_scalar;

                    /*
                    if (do_debug && (bm->which_check == species))
						fprintf(llogfp, "vertdistrib = %e\n", vertdistrib);
                     */
                    
					Find_Final_Invert_Recruit_Distribtuion(bm, species, ngene, qid, stock_id, spawned_biomass, wclayer, llogfp);

					/* Update yoy sum - only if localised recruit as all other cases done in Store_Recruitment_Diagnostics() */
					if (FunctGroupArray[species].recruitType) {
						tot_yoy[species][maxstock_id] += spawned_biomass;
						tot_yoy[species][stock_id] += spawned_biomass;
						bm->calcTrackedMort[species][0][stock_id][start_id] += spawned_biomass;  // TODO: Is this covered in general update anyways?
					}

					/* Did something */
					did_something = 1;
				}
				if (overall_checkday == (sp_spawnday + (additional_day * 2.0))) {
					if ((bm->current_box == bm->last_box) && (!bm->current_layer)){
						if(cohort == (FunctGroupArray[species].numCohortsXnumGenes - 1) ){
							Store_Recruitment_Diagnostics(bm, species, FunctGroupArray[species].recruitType, do_debug, llogfp);
						}

						/*
						fprintf(llogfp, "At %e species %s has %e YOY (avgbiom = %e)\n", bm->dayt,
								FunctGroupArray[species].groupCode, yoysum, yoysum / bm->nbox);
						*/

						/* Update the spawning queue */
						Update_Spawn_Queue(bm, species, cohort, maxstock_id, do_debug, llogfp);
					}
				}

				/* If semelparous reproduction - update biomass to be lost (i.e. all adults spawning rather than just spawn produced)
				 * TODO: May need to distinguish when only external stocks reproduce not internal stocks, in which case don't need to
				 * do this step as already sorted in Ecology_Do_External_InVert_Spawning()
				 */
				if (spawning_biomass && (flagmother < 0)) {
					spawning_biomass_contrib = this_biom * (1.0 - FunctGroupArray[species].scaled_FSPB[cohort]);
				} else if (spawning_biomass) {
					spawning_biomass_contrib = spawning_biomass * prop_biom_spawn_lost;  // This used to be zero, as spawn found now and don't want adult population reduced
				} else {
					spawning_biomass_contrib = 0.0;
				}
				shiftVERT[species][cohort][SN_id] = spawning_biomass_contrib;

				/******* Aging and Settlement of new young ******/

				/* Aging - used to assume age of maturity is about 12 months and total life span is about 24 months.
				 * Now should be assumption free
				 */

				// TODO: Make this work for more than two age stages for biomass pools
				if (sp_Age_Now) {
                    /* Record final numbers if needed */
                    if(bm->M_est_method == Z_and_F_based) {
                        Record_End_Num(bm, species);
                    }
                    
					Init_Aging_Arrays(bm, species, stock_id, maxstock_id, llogfp);

					/* Add the cohort that is maturing (if senescent they will just age out of the system and die) */
					p_ageup = Get_Proportion_Aging(bm, species, cohort, do_debug, llogfp);
					if ((basecohort == num_cohorts) && (!bm->flagsenesce)) {
						/* The oldest age group doesn't age up. */
						p_ageup = 0.0;
					}

					biomass_newly_mature = this_biom * p_ageup;
					shiftVERT[species][cohort][DEN_id] = biomass_newly_mature;
					shiftVERT[species][cohort][RN_id] = p_ageup;

                    /*
					if (do_debug && (bm->which_check == species))
						fprintf(llogfp," %s-%d (bc %d vs nc %d) biomass_newly_mature: %e, this_biom: %e, p_ageup: %e\n", 
							FunctGroupArray[species].groupCode, cohort, basecohort, num_cohorts, biomass_newly_mature, this_biom, p_ageup);
					*/
                    
					/* Did something */
					did_something = 1;
				}
			}

			for (cohort = (FunctGroupArray[species].numCohortsXnumGenes) - 1; cohort >= 0; cohort--) {
				sp_checkday = (int) (floor(EMBRYO[species].Age_Now[cohort][EMBRYO[species].next_age]));
				if (overall_checkday == sp_checkday) {
					sp_Age_Now = 1;
				}

				if (did_something) {

					/* tag that we need to at a mat update - this will force a call to Reset_Mortality */
					FunctGroupArray[species].speciesParams[need_matupdate_id] = 1;

					/** Update Pools - shifting biomass agins and lost to reproduction and adding in any new recruits **/
					Update_Invert_Aging(bm, localWCTracers, species, cohort, sp_Age_Now, stock_id, ij, wclayer, do_debug, llogfp);

					/** Update Migration array **/
					Update_Migration_Array(bm, species, cohort, maxstock_id, do_debug, llogfp);

					/** Update final box-age distributions **/
					if(cohort == 0){
						Update_Invert_Cohorts(bm, species, stock_id, maxstock_id, llogfp);
					}
				}

				if (sp_Age_Now ){
					/* Update the aging queue */
					Update_Age_Queue(bm, species, cohort, do_debug, llogfp);
				}
			}
		}
	}
	return;
}

/**
 * \brief: Invertebrate spawning function
 *
 */
void Ecology_Find_Invert_Spawn(MSEBoxModel *bm, int species, double spawning_biomass, int stock_id, double plankton, double CHLa, FILE *llogfp) {
	int recruit_sp = (int) (FunctGroupArray[species].speciesParams[flagrecruit_id]);
	int qid = EMBRYO[species].next_larvae;
	int tsrecruitid = (int) (FunctGroupArray[species].speciesParams[tsrecruit_id]);
	double local_spawn_biomass = 0.0;
	double KSPAWN_sp = KDENR[species][stock_id];
	double PP_sp = FunctGroupArray[species].speciesParams[PP_id];
	double sp_log_mult = FunctGroupArray[species].speciesParams[log_mult_id];
	int sp_numGeneTypes = (int) (FunctGroupArray[species].numGeneTypes);
	int ngene, boxkey_id;
	double recruitscale1, recruitscale2, recruitconst, stocking_sp;
    int do_debug = 0;

	for (ngene = 0; ngene < sp_numGeneTypes; ngene++) {
		switch (recruit_sp) {
		case no_recruit:
			quit("No such flagrecruit defined (i.e. value must be > 0)\n");
			break;
		case const_recruit: /* Fixed set of constants */
            local_spawn_biomass = KSPAWN_sp;
            break;
		case fixed_linear_recruit: /* Reproduction a fixed number per adult spawning */
			local_spawn_biomass = KSPAWN_sp * spawning_biomass;
			break;
		case chl_recruit: /* Proportional to primary productivity */
			local_spawn_biomass = PP_sp * CHLa / bm->ref_chl;
			break;
		case rand_recruit: /* Random - follows lognormal */
			local_spawn_biomass = sp_log_mult * Util_Logx_Result(-lognorm_mu, lognorm_sigma);
			break;
		case plank_recruit: /* Spawn is based on plankton levels (not just CHLa) */
			local_spawn_biomass = PP_sp * plankton / bm->ref_chl;
			break;
		case BevHolt_recruit: /* Beverton-Holt stock-recruit relationship */
		case SSB_BevHolt_recruit:
		case BevHolt_rand_recruit:
        case BevHolt_num_recruit:
        case BevHolt_direct_num_recruit:
		case recover_recruit:
		case force_recover_recruit:
		case Ricker_recruit: /* Ricker */
		case baltic_ricker: /* Baltic version of the ricker */
        case SSB_ricker: /* SSB based ricker version - for senescent and short lived species */
		case jackknife_recruit: /* Jackknife spawning function - sum up spawning stock biomass */
			local_spawn_biomass = spawning_biomass;
			break;
		case ts_recruit:/* Read in timeseries of recruitment */
			if (EMBRYO[species].readytospawn[stock_id] == 1) {
				EMBRYO[species].Larvae[stock_id][ngene][qid] = tsEval(bm->tsRecruit, tsrecruitid, bm->t);
			}
			break;
        case multiple_ts_recruit: /* Read in timeseries of recruitment */
            boxkey_id = bm->BoxKeyMap[bm->current_box][rects_key_id];
            EMBRYO[species].Larvae[stock_id][ngene][qid] +=  Get_Imposed_MultRecruits(bm, species, do_debug, boxkey_id, llogfp);
            break;
		case coral_recruit: /* Recruitment of corals and similar organisms - mix of brooding and broadcast */
			recruitscale1 = FunctGroupArray[species].speciesParams[CrecruitA_id];
			recruitconst = FunctGroupArray[species].speciesParams[CrecruitB_id];
			recruitscale2 = FunctGroupArray[species].speciesParams[CrecruitC_id];
			local_spawn_biomass = KSPAWN_sp + recruitscale1 * max(0.0,(recruitconst - exp(-recruitscale2 * spawning_biomass)));
			break;
		default:
			quit("No such flagrecruit defined for invertebrates - value must be between %d and %d or %d or %d\n", const_recruit, BevHolt_rand_recruit, Ricker_recruit, baltic_ricker);
			break;
		}

        stocking_sp = (int) (FunctGroupArray[species].speciesParams[flagstocking_id]);
        if (stocking_sp) {
            /* Read in timeseries of number of recruits added due to stocking - assume all genotypes get the same number of recruits */
            boxkey_id = bm->BoxKeyMap[bm->current_box][rects_key_id];
            EMBRYO[species].Larvae[stock_id][ngene][qid] +=  Get_Imposed_MultRecruits(bm, species, do_debug, boxkey_id, llogfp);
        }
        
		if (FunctGroupArray[species].recruitType)
			EMBRYO[species].Larvae[stock_id][ngene][qid] = local_spawn_biomass; // Local biomass sets the number of recruits
		else
			EMBRYO[species].Larvae[stock_id][ngene][qid] += local_spawn_biomass; // Population biomass cumulatively contributes to the number of recruits

		//fprintf(llogfp,"%s has spawned_biomass: %e, local_spawn_biomass: %e (recruit_sp: %d, KSPAWN_sp: %e, spawning_biomass: %e, stock: %d)\n", FunctGroupArray[species].groupCode, EMBRYO[species].Larvae[stock_id][ngene][qid], local_spawn_biomass, recruit_sp, KSPAWN_sp, spawning_biomass, stock_id);
	}
	return;
}

/**
 *
 * \brief External invertebrate spawning (occurring by adults outside the model domain)
 *
 */
double Ecology_Do_External_InVert_Spawning(MSEBoxModel *bm, int species, int cohort, int do_debug, FILE *llogfp) {
	double MigSpawn = 0.0;
	int qid;
	int flagmother = (int) (FunctGroupArray[species].speciesParams[flagmother_id]);
	double prop_mat = FunctGroupArray[species].scaled_FSPB[cohort];
	double spawn_active = FunctGroupArray[species].speciesParams[FSP_id];
    double flag_allow_spawn = 1.0;

	/* If spawn outside model and in first dynamic box then add in spawning due to migrants */
	if ((FunctGroupArray[species].externalReproducer) && ((bm->current_box == bm->first_box) && (bm->current_layer == bm->top_layer))) {
		for (qid = MIGRATION[species].num_in_queue_done; qid < MIGRATION[species].num_in_queue; qid++) {
            flag_allow_spawn = (double)(MIGRATION[species].ReprodAllowed[cohort][qid]);
			MigSpawn = flag_allow_spawn * spawn_active * prop_mat * MIGRATION[species].DEN[cohort][qid]; // In this case DEN is the total biomass of the migrating group

			/* Update biomass left in migrating cohort */
			if (flagmother < 0) {
				// If semelparous then all that spawned die
				MIGRATION[species].DEN[cohort][qid] *= (1.0 - spawn_active);
			} else {
				// Only matter spawned lost
				MIGRATION[species].DEN[cohort][qid] -= MigSpawn;
			}
		}
	}

	return MigSpawn;
}

/**
 * \brief: Invertebrate recruit function
 *
 */
double Ecology_Find_Invert_Recruits(MSEBoxModel *bm, int species, int ngene, int stock_id, double plankton, FILE *llogfp) {
	int recruit_sp = (int) (FunctGroupArray[species].speciesParams[flagrecruit_id]);
	int qid = EMBRYO[species].next_recruit;
	int do_debug = 0;
	double step1, step2, jack_SSB, jack_B, jack_a;
	double spawned_biomass = 0.0;
	double spawning_biomass = EMBRYO[species].Larvae[stock_id][ngene][qid];
	double pop_spawning_biomass = spawning_biomass;
	double stock_scalar1 = 1.0;
	double BHalpha_sp = FunctGroupArray[species].speciesParams[BHalpha_id];
	double BHbeta_sp = FunctGroupArray[species].speciesParams[BHbeta_id];
	double Ralpha_sp = FunctGroupArray[species].speciesParams[Ralpha_id];
	double Rbeta_sp = FunctGroupArray[species].speciesParams[Rbeta_id];
    double norm_sigma = FunctGroupArray[species].speciesParams[norm_sigma_id];
	double recover_mult_sp = FunctGroupArray[species].speciesParams[recover_mult_id];
	double recover_start_sp = FunctGroupArray[species].speciesParams[recover_start_id];

	if(((bm->current_box == bm->checkbox) || (bm->dayt >= bm->checkstart)) && (bm->which_check == species))
		do_debug = 1;

	if (FunctGroupArray[species].recruitType) {
		pop_spawning_biomass = spawning_biomass;
	} else {
		stock_scalar1 = recSTOCK[species][stock_id];
		pop_spawning_biomass = bm->totfishpop[species] * stock_prop[species][stock_id];
	}
    
    //if (bm->which_check == species)
    //    do_debug = 1;


    switch (recruit_sp) {
	case no_recruit:
		quit("No such flagrecruit defined (i.e. value must be > 0)\n");
		break;
	case const_recruit: /* Fixed set of constants */
	case fixed_linear_recruit: /* Reproduction a fixed number per adult spawning - not really valid here */
	case chl_recruit: /* Proportional to primary productivity */
	case rand_recruit: /* Random - follows lognormal */
	case plank_recruit: /* Spawn is based on plankton levels (not just CHLa) */
	case ts_recruit: /* Read in timeseries of recruitment */
    case multiple_ts_recruit: /* Read in timeseries of recruitment from multpile individual boxes */
	case coral_recruit: /* Recruitment of corals and simialr organisms - mix of brooding and broadcast */
		spawned_biomass = EMBRYO[species].Larvae[stock_id][ngene][qid];
		break;
	case BevHolt_recruit: /* Beverton-Holt stock-recruit relationship */
    case BevHolt_num_recruit:
	case SSB_BevHolt_recruit:
    case BevHolt_direct_num_recruit:
		spawned_biomass = (stock_scalar1 * BHalpha_sp * spawning_biomass) / (BHbeta_sp + pop_spawning_biomass);
		break;
    case BevHolt_rand_recruit: /* Spawn is based on Beverton Holt with lognormal variation and
	 dependence on plankton levels  - so uses spawn calculated above */
		step1 = Util_Logx_Result(-lognorm_mu, lognorm_sigma);
		step2 = ((stock_scalar1 * BHalpha_sp * spawning_biomass) / (BHbeta_sp + pop_spawning_biomass));
		spawned_biomass = step1 * step2 * (plankton / bm->ref_chl);
		break;
	case recover_recruit: /* Spawn is allowed a recovery encouraging boost of recruits
	 after "recovery_span" years of depressed stock levels */
		spawned_biomass = (stock_scalar1 * BHalpha_sp * spawning_biomass / (BHbeta_sp + pop_spawning_biomass));

		if (recover_help[species][0] && (recover_help_set[species] <= bm->dayt - (recover_span * recover_subseq))) {
			spawned_biomass *= recover_mult_sp;
			if (recover_help[species][1]) {
				fprintf(llogfp, "Time: %e, species %s has had a recovery event (spawned_biomass = %e)\n", bm->dayt, FunctGroupArray[species].groupCode, spawned_biomass);
				recover_help[species][1] = 0;
			}
		}
		break;
	case force_recover_recruit: /* Spawn has a pre-specified recovery encouraging boost of recruits */
		spawned_biomass = (stock_scalar1 * BHalpha_sp * spawning_biomass / (BHbeta_sp + pop_spawning_biomass));

		if ((bm->dayt >= recover_start_sp) && (bm->dayt <= (recover_start_sp + recover_subseq))) {
			spawned_biomass *= recover_mult_sp;
			fprintf(llogfp, "Time: %e, species %s has had a prescribed recovery event (spawned_biomass = %e)\n", bm->dayt, FunctGroupArray[species].groupCode, spawned_biomass);
		}

		//quit("No such flagrecruit defined for invertebrates (i.e. value must be > 0 and a value other than %d or %d)\n", recover_recruit,
		//		force_recover_recruit);
		break;
	case Ricker_recruit: /* Ricker */
    case SSB_ricker: /* SSB based ricker version - for senescent and short lived species */            
		spawned_biomass = pop_spawning_biomass * exp(stock_scalar1 * Ralpha_sp * (1.0 - pop_spawning_biomass / Rbeta_sp));
		break;
	case jackknife_recruit: /* Jackknife spawning function - sum up spawning stock biomass */
		jack_SSB = EMBRYO[species].Larvae[stock_id][ngene][qid];  // As stored spawner biomass in this above
		jack_B = FunctGroupArray[species].speciesParams[jack_b_id] * pop_spawning_biomass;
		jack_a = FunctGroupArray[species].speciesParams[jack_a_id];

		if (jack_SSB <  jack_B)
			spawned_biomass = jack_a * jack_SSB;
		else
			spawned_biomass = jack_a * jack_B;
		break;
	case baltic_ricker: /* Baltic version of the ricker */
		spawned_biomass = Ralpha_sp * pop_spawning_biomass * exp( -1.0 * Rbeta_sp * pop_spawning_biomass);
		break;
	default:
		quit("No such flagrecruit defined for invertebrates - value must be between %d and %d or %d or %d\n", const_recruit, BevHolt_rand_recruit, Ricker_recruit, baltic_ricker);
		break;
	}

    // Check for any stochasticity added to this recruitment
    if ( FunctGroupArray[species].speciesParams[flag_recruit_stochastic_id] > 0) {
        step1 = spawned_biomass;
        spawned_biomass = Util_Normx_Result(step1, norm_sigma);
    }
    
	if (do_debug && (bm->which_check == species))
		fprintf(llogfp,"%s has recruit_sp: %d, spawned_biomass %e\n", FunctGroupArray[species].groupCode, recruit_sp, spawned_biomass);

	return spawned_biomass;

}

/**
 *	Routine to handle aging and spawning update for invertebrates -
 *	needs to be a global action so don't get erroneous weighting if migrating
 *
 */
void Ecology_Update_Invert_Cohorts(MSEBoxModel *bm, FILE *llogfp) {
	//int spawnupdate, ageupdate, done_something, cohort
	int species, ij, k, n, pid;
	double biomass, chrt_biomass;
	int pred, stock_id;

	/* Updating starting biomass for age structured invertebrate pools here */
	for (species = 0; species < bm->K_num_tot_sp; species++) {
		int maxstock = FunctGroupArray[species].numStocks;
		if ((int) (FunctGroupArray[species].speciesParams[flag_id]) == TRUE) {
			//if ((FunctGroupArray[species].isVertebrate == FALSE) && (FunctGroupArray[species].groupAgeType == AGE_STRUCTURED_BIOMASS)) {
			if ((FunctGroupArray[species].groupAgeType == BIOMASS) && (!bm->TofY)){
				bm->calcMnum[species][hist_id] = bm->calcMnum[species][expect_id];
				bm->calcFnum[species][hist_id] = bm->calcFnum[species][expect_id];
				bm->calcNstart[species][hist_id] = bm->calcNstart[species][expect_id];
				bm->calcNstartPerPred[species][hist_id] = bm->calcNstartPerPred[species][expect_id];
				bm->calcMnum[species][expect_id] = 0;
				bm->calcFnum[species][expect_id] = 0;
				FunctGroupArray[species].speciesParams[calcupdate_date_id] = bm->dayt;

				for (pred = 0; pred < bm->K_num_tot_sp; pred++) {
					bm->calcMnumPerPred[species][pred][hist_id] = bm->calcMnumPerPred[species][pred][expect_id];
					bm->calcMnumPerPred[species][pred][expect_id] = 0;
				}

				/* Calculate total biomass */
				biomass = 0;
				for (n = 0; n < FunctGroupArray[species].numCohortsXnumGenes; n++) {
					pid = FunctGroupArray[species].totNTracers[n];

					if(bm->track_contaminants){
						Contaminant_Update_ContactMort_Record(bm, species, n);
					}

					for (k = 0; k < maxstock; k++) {
						bm->calcTrackedMort[species][n][k][finalM1_id] = bm->calcTrackedMort[species][n][k][ongoingM1_id] / (bm->calcTrackedMort[species][n][k][start_id] + small_num);
						bm->calcTrackedMort[species][n][k][finalM2_id] = bm->calcTrackedMort[species][n][k][ongoingM2_id] / (bm->calcTrackedMort[species][n][k][start_id] + small_num);
						bm->calcTrackedMort[species][n][k][finalF_id] = bm->calcTrackedMort[species][n][k][ongoingF_id] / (bm->calcTrackedMort[species][n][k][start_id] + small_num);

						//fprintf(llogfp,"Time: %e %s-%d has finalM1: %e ongoingM1: %e start: %e\n",
						//		bm->dayt, FunctGroupArray[species].groupCode, n, bm->calcTrackedMort[species][n][k][finalM1_id], bm->calcTrackedMort[species][n][k][ongoingM1_id], (bm->calcTrackedMort[species][n][k][start_id] + small_num));

						for(pred=0; pred < bm->K_num_tot_sp; pred++){
							bm->calcTrackedPredMort[species][n][k][pred][final_id] = bm->calcTrackedPredMort[species][n][k][pred][ongoing_id] / (bm->calcTrackedMort[species][n][k][start_id] + small_num);
							bm->calcTrackedPredMort[species][n][k][pred][ongoing_id] = 0.0;
                        }

						bm->calcTrackedMort[species][n][k][ongoingM1_id] = 0.0;
						bm->calcTrackedMort[species][n][k][ongoingM2_id] = 0.0;
						bm->calcTrackedMort[species][n][k][ongoingF_id] = 0.0;
						bm->calcTrackedMort[species][n][k][start_id] = 0.0;
                        
                        FunctGroupArray[species].min_B[n] = MAXDOUBLE;
                        FunctGroupArray[species].max_B[n] = MINDOUBLE;

					}
					for (ij = 0; ij < bm->nbox; ij++) {
		                chrt_biomass = 0.0;
						// Loop through each pelagic layer
						for (k = 0; k < bm->boxes[ij].nz; k++) {
							stock_id = bm->group_stock[species][ij][k];

							biomass += bm->boxes[ij].tr[k][pid] * bm->boxes[ij].dz[k] * bm->boxes[ij].area * FunctGroupArray[species].habitatCoeffs[WC];
							chrt_biomass = bm->boxes[ij].tr[k][pid] * bm->boxes[ij].dz[k] * bm->boxes[ij].area * FunctGroupArray[species].habitatCoeffs[WC];
				            bm->calcTrackedMort[species][n][stock_id][start_id] += chrt_biomass;
						}
						// Set relk for sediment and epibenthos by assuming its the one in deepest vertical layer
						stock_id = bm->group_stock[species][ij][0];  // As assuming k = 0

						// Loop through each sediment layer
						for (k = 0; k < bm->boxes[ij].sm.nz; k++) {
							biomass += bm->boxes[ij].sm.tr[k][pid] * bm->boxes[ij].sm.dz[k] * bm->boxes[ij].area * FunctGroupArray[species].habitatCoeffs[SED];
							chrt_biomass = bm->boxes[ij].sm.tr[k][pid] * bm->boxes[ij].sm.dz[k] * bm->boxes[ij].area * FunctGroupArray[species].habitatCoeffs[SED];
				            bm->calcTrackedMort[species][n][stock_id][start_id] += chrt_biomass;
						}
						// Loop over epibenthic layer
						if (FunctGroupArray[species].habitatType == EPIFAUNA){
							biomass += bm->boxes[ij].epi[pid] * bm->boxes[ij].area * FunctGroupArray[species].habitatCoeffs[EPIFAUNA];
							chrt_biomass = bm->boxes[ij].epi[pid] * bm->boxes[ij].area * FunctGroupArray[species].habitatCoeffs[EPIFAUNA];
				            bm->calcTrackedMort[species][n][stock_id][start_id] += chrt_biomass;
						}
					}
				}
				bm->calcNstart[species][expect_id] = biomass;
				bm->calcNstartPerPred[species][expect_id] = biomass;
                
                FunctGroupArray[species].updatedDiet = 1;

			}
			else if(FunctGroupArray[species].groupAgeType == AGE_STRUCTURED_BIOMASS){

				/* Do we need to update the mortality output values */
				if(FunctGroupArray[species].speciesParams[need_matupdate_id] == 1){
					Reset_Mortality(bm, species);
					FunctGroupArray[species].speciesParams[need_matupdate_id]  = 0;
				}
			}
            
		}
	}
    

	return;
}

/*
 * \brief Spatial distribution of invertebrate new recruits
 *
 */
void Find_Final_Invert_Recruit_Distribtuion(MSEBoxModel *bm, int species, int ngene, int qid, int stock_id, double spawned_biomass, int wclayer, FILE *llogfp) {
	int recruit_outside = 0, cohort = 0, bcohort = 0;
	//int qid = EMBRYO[species].latest_in_larval_queue;
    int mid = 0;
    //int pid;
	double thisday = 0;
	double hab_scalar = Get_Habitat_Mediated_Recruit_Scalar(bm, species, llogfp);

	for( ngene = 0; ngene < FunctGroupArray[species].numGeneTypes; ngene++){
		switch (FunctGroupArray[species].recruitType) {
		case independent_recruit_distrib:
        case external_spawn_local_recruit:
			/* If recruitment occurs within the model domain, assign final values
			 - checks to make sure distributions don't lead to artificial reseeding are
			 done on model initialisation on whenever the're reset (e.g. by movement, if
			 have localised recruitment) */

			EMBRYO[species].num_recruits[bm->current_box][wclayer][ngene][qid] = hab_scalar * spawned_biomass * bm->recruit_hdistrib[ngene][bm->current_box][species];
			totrecruit[species][stock_id][ngene] += EMBRYO[species].num_recruits[bm->current_box][wclayer][ngene][qid];
            
            /*
			if(EMBRYO[species].num_recruits[bm->current_box][wclayer][ngene][qid] > 0){
				fprintf(llogfp,"Time: %e box%d-%d %s ngene: %d, qid: %d, num_rec: %e, hab_scalar: %e, spawned_biomass: %e, hdistrib: %e, totrecruit: %e\n",
						bm->dayt, bm->current_box, wclayer, FunctGroupArray[species].groupCode, ngene, qid, EMBRYO[species].num_recruits[bm->current_box][wclayer][ngene][qid],
						hab_scalar, spawned_biomass, bm->recruit_hdistrib[ngene][bm->current_box][species], totrecruit[species][stock_id][ngene]);
			}
            */
			break;
		case at_parent_location:
			EMBRYO[species].num_recruits[bm->current_box][wclayer][ngene][qid] = hab_scalar * spawned_biomass;
			totrecruit[species][stock_id][ngene] += EMBRYO[species].num_recruits[bm->current_box][wclayer][ngene][qid];
              
            /*
            fprintf(llogfp,"Time: %e box%d-%d %s ngene: %d, qid: %d, num_rec: %e, hab_scalar: %e, spawned_biomass: %e, totrecruit: %e\n",
                bm->dayt, bm->current_box, wclayer, FunctGroupArray[species].groupCode, ngene, qid, EMBRYO[species].num_recruits[bm->current_box][wclayer][ngene][qid],
                hab_scalar, spawned_biomass, totrecruit[species][stock_id][ngene]);
            */
			break;
		case external_recruit:
			if ((bm->current_box == bm->first_box) && (bm->current_layer == bm->top_layer)) {
				/* If recruit outside model area and in first dynamic box then update
				 the Migration array */
				for (mid = MIGRATION[species].num_in_queue_done; mid < MIGRATION[species].num_in_queue; mid++) {
					recruit_outside = 0;

					for(bcohort = 0; bcohort < FunctGroupArray[species].numCohorts; bcohort++){
						cohort = (FunctGroupArray[species].numGeneTypes * bcohort) + ngene;
						//thisday = (double)(EMBRYO[species].Spawn_Now[cohort][EMBRYO[species].next_spawn]);
                        thisday = (double)EMBRYO[species].StartDay[cohort][EMBRYO[species].next_recruit];
						//pid = FunctGroupArray[species].cohort_stage[cohort];
                        
						/* Recruit while outside the model */
						if ((MIGRATION[species].Return_Now[mid] >= thisday) && (MIGRATION[species].Leave_Now[mid] <= thisday)) {
							recruit_outside = 1;
						}
					}
					if (recruit_outside) {
						MIGRATION[species].DEN[ngene][mid] += hab_scalar * spawned_biomass;
						MIGRATION[species].recruit[ngene][mid] = hab_scalar * spawned_biomass;
                    } else {
						MIGRATION[species].recruit[ngene][mid] = 0;
                    }
                    
                    /**
                    if ((species == 26) || (species == 35) || (species == 64)) {
                        fprintf(llogfp,"Time: %e box%d-%d %s ngene: %d, mid: %d, MIGden: %e, MIGrecruit: %e, hab_scalar: %e, spawned_biomass: %e, totrecruit: %e\n",
                                bm->dayt, bm->current_box, wclayer, FunctGroupArray[species].groupCode, ngene, mid, MIGRATION[species].DEN[ngene][mid], MIGRATION[species].recruit[ngene][mid], hab_scalar, spawned_biomass, totrecruit[species][stock_id][ngene]);
                    }
                    **/
				}
				totrecruit[species][stock_id][ngene] = 0.0;
			}
			break;
		case larval_dispersal:
			EMBRYO[species].num_recruits[bm->current_box][wclayer][ngene][qid] = hab_scalar * spawned_biomass * bm->larvaldistrib[bm->current_box][species];
			totrecruit[species][stock_id][ngene] += EMBRYO[species].num_recruits[bm->current_box][wclayer][ngene][qid];

			/* Takes larvae and distributes based on read in time series of distributions (updated in physical library)
			 * for each species we have
			 num_recruits[bm->current_box][wclayer][latest_in_larval_queue] = spawned_biomass * bm->larvaldistrib[bm->current_box][wclayer][d]

			 where temprec = totspawn * array_survivourship (conditioned on plankton and time in watercolumn to get from site A to B)
			 and larvaldistrib is proportion of spawn starting at site A that reaches site B after recruit period of time

			 For expanded case of larval_dispersal don't sum over layers and boxes to get temprec,
			 do num_recruits iterative allocation at time calc TotSpawn

			 May also want to include super expanded case where have Larvae as life history stage
			 and explicitly consider mortality and growth at each time step (so predation and
			 temperature dependent mortality explicitly represented)

			 totrecruit[species][stock_id][ngene] += num_recruits[bm->current_box][wclayer][ngene][latest_in_larval_queue]; for the species
			 */
			break;
		default:
			quit("No such recruitType defined for vertebrates (%d) - value must be between 0 and 3 currently\n", FunctGroupArray[species].recruitType);
			break;
		}
		/*
		fprintf(llogfp,"%s case: %d, qid: %d, spawned_biomass: %e, num_rec: %e\n", 
			FunctGroupArray[species].groupCode,FunctGroupArray[species].recruitType, qid, spawned_biomass, 
			EMBRYO[species].num_recruits[bm->current_box][wclayer][ngene][qid]);
		*/
	}
	return;
}

/**
 * \brief Reporting newly spawned biomass to add to youngest cohort
 */
double Get_Invert_Settlers(MSEBoxModel *bm, int species, int ngene, int wclayer, int stock_id, int do_debug, FILE *llogfp) {
	double ans = 0.0;

	ans = EMBRYO[species].num_recruits[bm->current_box][wclayer][ngene][EMBRYO[species].next_recruit];

	return ans;

}

/**
 * \brief Updating numbers of aging up through age classes. Also updating teh local distributions
 *
 */
void Update_Invert_Aging(MSEBoxModel *bm, double *localWCTracers, int species, int cohort, int sp_Age_Now, int stock_id, int ij, int wclayer, int do_debug, FILE *llogfp) {
	double base_biomass = localWCTracers[FunctGroupArray[species].totNTracers[cohort]];
	int sp_AgeClassSize = FunctGroupArray[species].ageClassSize;
	int num_cohorts = FunctGroupArray[species].numCohorts - 1;
	int sp_numGeneTypes = (int) (FunctGroupArray[species].numGeneTypes);
	int basecohort = (int)(floor( cohort / sp_numGeneTypes ));
	int nextcid = cohort + sp_numGeneTypes;
	int i;
	int ngene = cohort - (basecohort * FunctGroupArray[species].numGeneTypes);
	double biomass_terminal_class = 0.0;
	double mT_sp = FunctGroupArray[species].speciesParams[mT_id];
	double tracer_correction = 0.0;

	/* Get info */
	double new_settled_biomass = Get_Invert_Settlers(bm, species, ngene, wclayer, stock_id, do_debug, llogfp);
	double spawning_biomass_contrib = shiftVERT[species][cohort][SN_id];
	double biomass_newly_mature = shiftVERT[species][cohort][DEN_id];

	/*
	for (i = 0; i < FunctGroupArray[species].numCohortsXnumGenes; i++) {
		fprintf(llogfp, "Time: %e %s-%d box%d-%d biom_start: %e\n", 
			bm->dayt, FunctGroupArray[species].groupCode, i, bm->current_box, bm->current_layer, localWCTracers[FunctGroupArray[species].totNTracers[i]]);
	}
	*/

	if(basecohort == num_cohorts)
		nextcid = cohort;

	if(basecohort > 0)
		new_settled_biomass = 0;

	// Correct from /m2 to total
	base_biomass *= bm->boxes[ij].area;
	new_settled_biomass *= bm->boxes[ij].area;
	biomass_newly_mature *= bm->boxes[ij].area;
	spawning_biomass_contrib *= bm->boxes[ij].area;
	if (FunctGroupArray[species].habitatType != EPIFAUNA){
		/* If originally /m3 then still have to * dz */
		base_biomass *= bm->boxes[ij].dz[wclayer];
		new_settled_biomass *= bm->boxes[ij].dz[wclayer];
		biomass_newly_mature *= bm->boxes[ij].dz[wclayer];
		spawning_biomass_contrib *= bm->boxes[ij].dz[wclayer];
	}

	/* Do some sanity checks */
	if ( base_biomass < 0.0 )
		base_biomass = 0.0;
	if ( spawning_biomass_contrib < 0.0 )
		spawning_biomass_contrib = 0.0;
	if ( new_settled_biomass < 0.0 )
		new_settled_biomass = 0.0;
	if ( biomass_newly_mature < 0.0 )
		biomass_newly_mature = 0.0;

	/* Update age distributions within cohorts. Get absolute biomasses for
	 updating cohort biomass ratios (ignoring delays due to larval periods).
	 Actually update of PopRatio is done after all cells are complete and before next
	 time step by calling Update_Invert_Cohorts() from the main routine.
	 Also note that only include in the update if nonzero contributions.
	 */

	/* First age and spawn simultaneously */
	if (bm->flagagestruct) {
		/* Get terminal cohort aging out of the systenm - if appropriate */
		if (sp_Age_Now && bm->flagsenesce && sp_AgeClassSize && (basecohort == (FunctGroupArray[species].numCohorts - 1))) {
			i = sp_AgeClassSize - 1;
            if (i < 0)
                i = 0;
			biomass_terminal_class = mT_sp * FunctGroupArray[species].boxPopRatio[bm->current_box][bm->current_layer][cohort][i] * base_biomass;
		} else {
			biomass_terminal_class = 0.0;
		}

		/* If non-homogeneous distribution, update local distribution
		 - youngest group (i=0) already zeroed */
		for ( i = (sp_AgeClassSize - 1); i > 0; i--) {
			bm->tempPopRatio[stock_id][species][cohort][i] += (base_biomass - biomass_newly_mature - spawning_biomass_contrib)
					* FunctGroupArray[species].boxPopRatio[bm->current_box][bm->current_layer][cohort][i - 1];
		}
		// Now add newly arrived from recruitment or aging
		bm->tempPopRatio[stock_id][species][nextcid][0] += biomass_newly_mature;
		bm->tempPopRatio[stock_id][species][ngene][0] += new_settled_biomass;
	}

	/* Now update final tracer pool - adjusting everything back to /m2 and /m3 etc */
	tracer_correction = bm->boxes[ij].area;
	if (FunctGroupArray[species].habitatType != EPIFAUNA){
		/* So that we're dealing in /m3 */
		tracer_correction *= bm->boxes[ij].dz[wclayer];
	}

    /*
    if(do_debug) {
		fprintf(llogfp, "Time: %e %s box%d-%d cohort: %d, ngene: %d (%d) nextcid: %d\n",
                bm->dayt, FunctGroupArray[species].groupCode, bm->current_box, bm->current_layer, cohort, ngene, sp_numGeneTypes, nextcid);
		for (i = 0; i < FunctGroupArray[species].numCohortsXnumGenes; i++) {
			fprintf(llogfp, "Time: %e %s-%d box%d-%d biom_before_addition: %e\n",
                    bm->dayt, FunctGroupArray[species].groupCode, i, bm->current_box, bm->current_layer, localWCTracers[FunctGroupArray[species].totNTracers[i]]);
		}
	}
    */
    
	/* Lose those aging up and any contributions to spawning */
	localWCTracers[FunctGroupArray[species].totNTracers[cohort]] -= ((biomass_newly_mature + spawning_biomass_contrib) / tracer_correction);
	/* Gain newly aged up
	 * Note: Originally had no terminal entry below just skipped this addition if (basecohort == num_cohorts)
	 * However added in terminal age class step below so can have multi-year age cohorts for biomass pools 
	 */
	localWCTracers[FunctGroupArray[species].totNTracers[nextcid]] += (biomass_newly_mature / tracer_correction);
	/* Gain new recruits */
	localWCTracers[FunctGroupArray[species].totNTracers[ngene]] += (new_settled_biomass / tracer_correction);
	/* Lose any that die from senescence - terminal age cohort only (value will be zero for all other age classes) */
	localWCTracers[FunctGroupArray[species].totNTracers[cohort]] -= (biomass_terminal_class / tracer_correction);

	/* Sanity check */
	if(localWCTracers[FunctGroupArray[species].totNTracers[cohort]] < 0.0)
		localWCTracers[FunctGroupArray[species].totNTracers[cohort]] = 0.0;

	/**
	//if (bm->debug && (((bm->debug == debug_spawn) || (bm->debug == debug_age) || (bm->debug == debug_general)) && (bm->which_check == species))) {
	if(do_debug) {
		fprintf(llogfp, "Time: %e %s box%d-%d cohort: %d, ngene: %d (%d) nextcid: %d, new_settled_biomass: %e, biomass_newly_mature: %e, biomass_terminal_class: %e, spawning_biomass_contrib: %e\n",
		bm->dayt, FunctGroupArray[species].groupCode, bm->current_box, bm->current_layer, cohort, ngene, sp_numGeneTypes, nextcid,
		new_settled_biomass/tracer_correction, biomass_newly_mature/tracer_correction, biomass_terminal_class/tracer_correction, spawning_biomass_contrib/tracer_correction);
		for (i = 0; i < FunctGroupArray[species].numCohortsXnumGenes; i++) {
			fprintf(llogfp, "Time: %e %s-%d box%d-%d biom_end: %e\n", 
				bm->dayt, FunctGroupArray[species].groupCode, i, bm->current_box, bm->current_layer, localWCTracers[FunctGroupArray[species].totNTracers[i]]);
		}
	}
	**/

	return;
}

/*
 *	\brief Routine to handle aging update for invertebrates.
 *
 * TODO: May need to further update to be more like the "vertebrate" case
 */
void Update_Invert_Cohorts(MSEBoxModel *bm, int species, int stock_id, int maxstock_id, FILE *llogfp) {
	int i = 0, k = 0, classize = 1, sp_recruitpeak = 0, cohort = 0, qid;
	double totksum = 0, sp_recruitRange = 0, this_totsum = 0, maxvalue = 0;
	double sp_AgeClassSize = (double)(FunctGroupArray[species].ageClassSize);
    
    if ((int) (sp_AgeClassSize) == 0)
		classize = 1;
	else
		classize = (int) (sp_AgeClassSize);

	sp_recruitpeak = (int) (FunctGroupArray[species].speciesParams[flagrecpeak_id]);

	if (sp_recruitpeak)
		sp_recruitRange = bm->recruitRange;
	else
		sp_recruitRange = bm->recruitRangeFlat;


	for ( cohort = 0; cohort < FunctGroupArray[species].numCohortsXnumGenes; cohort++) {
		/* Update cohort biomass ratios - ignoring delays due to larval periods */

		if (classize == 1) {
			/* If an age class is a year class then should always just have a transference ratio of 1.0 */
			for (i = 0; i < maxstock_id; i++) {
				bm->tempPopRatio[i][species][cohort][0] = 1.0;
			}
			FunctGroupArray[species].boxPopRatio[bm->current_box][bm->current_layer][cohort][0] = 1.0;
		} else {
			/* Check to see if animal has site attachment and non homogeneous populations */
			if (!bm->flaghomog_sp) {
				/* Nothing to do as want to keep local distributions unchanged */
			} else {
				/* Site attached case done box by box */
				this_totsum = 0;
				for (k = 0; k < classize; k++) {
                    this_totsum += bm->tempPopRatio[stock_id][species][cohort][k] ;
				}
                for (qid = MIGRATION[species].num_in_queue_done; qid < MIGRATION[species].num_in_queue; qid++) {
                    this_totsum += MIGRATION[species].aging[cohort][qid];
                }

				if (!this_totsum){
					this_totsum = small_num;
					//quit("%s-%d had ltotsum = 0\n", FunctGroupArray[species].groupCode, cohort);
				}
				fprintf(bm->logFile, "species %s, totsum = %e\n",FunctGroupArray[species].groupCode, this_totsum);

				/* Normalise and find maximum entry */
				maxvalue = -MAXDOUBLE;
				for (k = 0; k < classize; k++) {
					if (!k) {
						/* If first slice of the ageclass then may have entrants from
						 the Migration array to consider */
                        for (qid = MIGRATION[species].num_in_queue_done; qid < MIGRATION[species].num_in_queue; qid++) {
                            bm->tempPopRatio[stock_id][species][cohort][k] += MIGRATION[species].aging[cohort][qid];
                        }
                        bm->tempPopRatio[stock_id][species][cohort][k] /= (this_totsum + small_num);
                    } else {
                        bm->tempPopRatio[stock_id][species][cohort][k] /= (this_totsum + small_num);
                    }
                    
                    if (bm->tempPopRatio[stock_id][species][cohort][k] > maxvalue) {
                        maxvalue = bm->tempPopRatio[stock_id][species][cohort][k];
                    }
				}

				/* Re-adjust (if necessary) - assuming that very populus
				 cohorts are disproportinately lost and that not all individuals grow
				 equally fast so some will sneak ahead/lag behind anyway */
				totksum = 0;
				for (k = 0; k < classize; k++) {
					if(bm->flag_replicated_old == FALSE){
						if ((cohort || (!cohort && k)) && (bm->tempPopRatio[stock_id][species][cohort][k] < (maxvalue / sp_recruitRange))) {
							bm->tempPopRatio[stock_id][species][cohort][k] = maxvalue / sp_recruitRange;
						}
					}
					totksum += bm->tempPopRatio[stock_id][species][cohort][k];
				}

				if (totksum != 1) {
					/* Renormalise */
					for (k = 0; k < classize; k++) {
						bm->tempPopRatio[stock_id][species][cohort][k] /= totksum;
					}
				}

				/* Set final age distribution */
				for (k = 0; k < classize; k++) {
					FunctGroupArray[species].boxPopRatio[bm->current_box][bm->current_layer][cohort][k] = bm->tempPopRatio[stock_id][species][cohort][k];
				}
			}
		}
	}

	return;

}

/*
 *	\brief Routine for including habitat mediated settlement
 *
 */
double Get_Habitat_Mediated_Recruit_Scalar(MSEBoxModel *bm, int species, FILE *llogfp) {
	int flag_recruit_habdepend = (int)FunctGroupArray[species].speciesParams[rec_HabDepend_id];
	int rec_stage = FunctGroupArray[species].numStages;  // As recruit stage stored after all other stages (as usually implict not explicit)
	int sp;
	int like_coral = 0;
	double sp_like;
	double hab_scalar = 1.0;
	double Rcoefft = bm->RugCover_Coefft;
	double Rconst = bm->RugCover_Const;

	if (flag_recruit_habdepend){
		for(sp = 0; sp < bm->K_num_tot_sp; sp++){
			if (FunctGroupArray[species].isCover == TRUE) {
				sp_like = bm->HABITATlike[species][rec_stage][FunctGroupArray[species].coverID];
				hab_scalar += sp_like * bm->coveramt[FunctGroupArray[species].coverID][bm->current_box];   // This covers larvae liking turf etc and no bare sediments
				if((FunctGroupArray[species].isCover == CORAL) || (FunctGroupArray[species].isCover == SPONGE)) {
					like_coral = 1;
				}
			}
		}
	}


	switch(bm->flag_refuge_model){
	case no_refuge_model:
	case orig_refuge_model:
		/* Do nothing */
		break;
	case rugosity_refuge_model:
		if(like_coral) {
			hab_scalar *= (Rcoefft * log(LocalRugosity) + Rconst);
		}
		break;
	default:
		quit("No such refuge model set flag_refuge_model to 0, 1 or 2 (current;y %d)\n", bm->flag_refuge_model);
		break;
	}

	// Sanity check
	if(hab_scalar > 1.0)
		hab_scalar = 1.0;

	return hab_scalar;
}

