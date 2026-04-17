/**
 * \file atannualbiology.c
 * \brief Biological processes that are done annually
 * \ingroup atEcology
 * \author Beth Fulton 	5/10/2005 CSIRO Marine Laboratories,  2005
 *
 *
 * 	Determines timing of annual events that will execute in main biological code
 *
 * <b>Revisions:</b>
 * 5/10/2005 Created from atannualbiology.c
 *
 * 12/10/2005 Corrected code to do with first round of wrapping
 * of reproduction (recruits in the pipeline), particularly
 * to do with those groups that have linear relationship
 * between pupping and adult abundance
 *
 * 24/2/2006 Rewrote vertebrate date determination in Annual_biology().
 * Added flagrandom so could switch off random factors. Added
 * initDATES and consistency checks to make sure don't have
 * artificial recruitment failure due to Wrap, WrapOld value mismatch.
 *
 * 3/4/2006 Moved handling of first year carry over recruits from
 * initBiology() to AnnualBiology()
 *
 * 26-05-2008 Bec Gorton
 * Changed all references from bm->VERTind to the tracer arrays associated with
 * each functional group.
 *
 * 02-10-2008 Bec Gorton
 * Removed references to jCEP and jPWN.
 *
 * 15/01/2009 Bec Gorton
 * Rewrote all code to loop over all the functional groups.
 * Created the Init_Vert_Spawning and Calculate_Migration_Dates functions.
 *
 * 16-01-2009 Bec Gorton
 * Added code to check the flagrandom flag in the invertegrate
 * migration date code. Previously this was not checked and a random
 * value was always added.
 *
 * 03-04-2009 Bec Gorton
 * Fixed a bug in a fprintf statement in Annual_biology.
 *
 * 03-06-2009 Bec Gorton
 * Added a additional debugging option to the Annual_biology function. When
 * bm->debug == debug_prey_biology_process is true do_debug will be set to true.
 *
 * 16-02-2010 Bec Gorton
 * Renamed the KMIGa array to KMIG.
 * 
 * 08-02-2012 Bec Gorton
 * Added a check on flagRandom before we add a random value to the invertebrate spawning and
 * aging dates. This makes testing easier.
 */
/*  Open library routines */
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>
#include <atecology.h>

/**
 * prototypes for static functions that are only defined in this file.
 */
//static void Init_Mortality_Estimates(MSEBoxModel *bm);
static void Init_Spawning(MSEBoxModel *bm, FILE *llogfp, int do_debug, int sp);
//static void Calculate_Migration_Dates(MSEBoxModel *bm, FILE *llogfp, int sp, int cohort, int migrationID, int do_debug);
static void CheckDates(MSEBoxModel *bm, int sp, int cohort, int ngene, int aid, int lid, int qid, int nspawn, int do_debug, int this_day, FILE *llogfp);
static void CheckSpawnDates(MSEBoxModel *bm, int sp, int cohort, int lid, int qid);
static void Pad_The_Demography_Queues(MSEBoxModel *bm, int sp, int cohort, int nspawn, int qid, int lid, int aid, FILE *llogfp);
static void PrintDates(MSEBoxModel *bm, int sp, int cohort, int ngene, int aid, int lid, int qid, int nspawn, int do_debug, int this_day, FILE *llogfp);
static void Scale_Group_Size(MSEBoxModel *bm, FILE *llogfp);
static void AlignVsMigration(MSEBoxModel *bm, int sp, int cohort, int lid, int qid);

static double Get_Init_Embryos(MSEBoxModel *bm, int species, int ngene, int stock_id, int qid, double first_spawn, int do_debug);

/**
 *	\brief This routine resets any annual biological processes (done 1st day of the year)
 *	It is called by main() in the transport module.
 *
 *	For each active vertebrate and immigrating invert:<br>
 *	<ul>
 *	<li>Find the spawning date</li>
 *	<li>Find start and end dates of recruitment and the recruitment period.</li>
 *	<li>Populate the MigrateAway and MigrateReturn arrays.</li>
 *	</ul>
 *	@param t  current time  (seconds)
 *	@param dt time range
 *	@param llogfp The model log file
 *
 *  Outputs :
 *  New spawning and migration times
 */
void Ecology_Annual(MSEBoxModel *bm, FILE *llogfp) {

    int sp;
    int do_debug;

	if (bm->debug && ((bm->debug == debug_migrate) || (bm->debug == debug_spawn) || (bm->debug == debug_age) || (bm->debug == debug_recruit)
			|| (bm->debug == debug_prey_biology_process))) {
		do_debug = 1;
	} else
		do_debug = 0;
    
	if (verbose > 0)
		printf("First day of year so annual events are done first\n");

	/* First day of the year stuff. Note years go from day 0 to day	364 NOT day 1 to day 365! */

	/* Setup new variable spawning and recruitment lengths and update old ones
	 if recruitment spans 1st day of the yr */
	if (verbose > 0) {
		printf("Spawning and migration dates are sorted out for vertebrates\n");
		//fprintf(llogfp, "Spawning and migration dates are sorted out for vertebrates\n");
	}

    /** Handle age structured migration and spawning */
	/* Done like this to get the drandom calls in the same order as the orginal code so that we get consistent results */
	for (sp = 0; sp < bm->K_num_tot_sp; sp++) {

		/* Check that the group is on */
		if ((int) (FunctGroupArray[sp].speciesParams[flag_id]) == TRUE) {            
            // Initialise diet update check
            FunctGroupArray[sp].updatedDiet = 0;
            
			switch (FunctGroupArray[sp].groupAgeType) {
			case AGE_STRUCTURED:
            case AGE_STRUCTURED_BIOMASS:

				/* Initialise the spawning dates - once per model run now */
                if (first_year) {
                    Init_Spawning(bm, llogfp, do_debug, sp);
                    Init_Migration_Age_Check(bm, llogfp, do_debug, sp);
                }
				break;
            case BIOMASS:
                /* Nothing to do here as migration done elsewhere and no spawning */
 				break;
			}
		}
	}
    
    /* Its no longer the first year to set the first_year flag to false */
	first_year = 0;

	/* Write out annual stock size file */
	Ecology_Calculate_Total_Abundance(bm, bm->dt, -1, llogfp);

	/* Initialse mortality estimate arrays  - now dealt with in Ecology_Update_Invert_Cohorts()*/
	//Init_Mortality_Estimates(bm);

	/* Scale the groups size (sn and rn values) */
	Scale_Group_Size(bm, llogfp);
    
    /* Check and update migration indices */
    Update_Migration_Index(bm, llogfp);

	return;
}

/**
 *	\brief This routine resets any mortality arrays so can estimate natural
 *  mortality for output. (NB: in some cases some of these already reset
 *  in Assess_Pseudo_Estimate_Prm())
 *
 *

static void Init_Mortality_Estimates(MSEBoxModel *bm) {
	int sp, pred;

	// Re-initialise mortality array for biomass pool invertebrates
	for (sp = 0; sp < bm->K_num_tot_sp; sp++) {
		// For biomass pool invertebrates store starting biomass
		if ((FunctGroupArray[sp].isVertebrate == FALSE) && (FunctGroupArray[sp].groupAgeType == BIOMASS)) {
			bm->calcMnum[sp][hist_id] = bm->calcMnum[sp][expect_id];
			bm->calcFnum[sp][hist_id] = bm->calcFnum[sp][expect_id];
			bm->calcNstart[sp][hist_id] = bm->calcNstart[sp][expect_id];
			bm->calcNstartPerPred[sp][hist_id] = bm->calcNstartPerPred[sp][expect_id];
			bm->calcMnum[sp][expect_id] = 0;
			bm->calcFnum[sp][expect_id] = 0;
			FunctGroupArray[sp].speciesParams[calcupdate_date_id] = 0.0;
			bm->calcNstart[sp][expect_id] = bm->totbiom[sp];
			bm->calcNstartPerPred[sp][expect_id] = bm->totbiom[sp];
			for (pred = 0; pred < bm->K_num_tot_sp; pred++) {
				bm->calcMnumPerPred[sp][pred][hist_id] = bm->calcMnumPerPred[sp][pred][expect_id];
				bm->calcMnumPerPred[sp][pred][expect_id] = 0;
			}
		}
	}
	return;
}
*/
/**
 * \brief Initialise the vertebrate spawning dates.
 *
 *
 *
 *
 */
static void Init_Spawning(MSEBoxModel *bm, FILE *llogfp, int do_debug, int sp) {
	double temp1, temp2, sumnzj1, sumnzj2, vertdistrib, AgeClassSize_sp;
	double ngenes = 0.0, thisday, tot_embryo, yr_scalar;
	int nboxes = bm->nbox;
	int nlayers = bm->wcnz;
	//int this_day = (int)(bm->dayt);
	int qid = 0;
	int aid = 0;
	int lid = 0;
	int startTofY = 0;
	int nspawn = 0;
	int i, j, k, layerk, Time_Spawn_sp, Recruit_Period_sp, Recruit_Time_sp, VarTime1,
		VarTime2, stock_id = 0, diffdeep, cohort, flagdem, diff, mid, den,
		basecohort, stage,  stid, stepsize, num_steps, day_to_check,
		temp_val, temp_val_start, temp_val_end, ngene, bcohort, chrt,
        start_i, countback_spawn, countback_recruit, prm_spawn, yr_counter;
	int larval_queue_extension = 0;
    int old_check = 0, days_lost = 0;
    int recruit_sp = (int) (FunctGroupArray[sp].speciesParams[flagrecruit_id]);
    double prod_scalar = 1.0;
    double maxstock = (double)(FunctGroupArray[sp].numStocks);
    double first_spawn = -1.0;
    
	AgeClassSize_sp = (double)(FunctGroupArray[sp].ageClassSize);
	flagdem = (int) (FunctGroupArray[sp].speciesParams[flagdem_id]);
    recruit_sp = (int) (FunctGroupArray[sp].speciesParams[flagrecruit_id]);

    do_debug = 0;
    if ((FunctGroupArray[sp].numSpawns > 1) || (FunctGroupArray[sp].groupType == FISH_INVERT))
        do_debug = 1;
    if (sp == bm->which_check)
    	do_debug = 1;
    
    if (verbose) {
        printf(" Doing Init_Spawning %s\n", FunctGroupArray[sp].groupCode);
    }
    
    Util_Init_1D_Double(adults_spawning, bm->K_num_stocks_per_sp, 0.0);
    
    //do_debug = 1;

    /* No longer needed as all done in one go below
    if (first_year) {
        // Set up RecruitPeriod array
        for (i=0; i < EMBRYO[sp].num_in_larval_queue; i++){
            EMBRYO[sp].RecruitPeriod[i] = (int) (FunctGroupArray[sp].speciesParams[Recruit_Period_id]);
            for(cohort = 0; cohort < FunctGroupArray[sp].numCohortsXnumGenes; cohort++) {
                EMBRYO[sp].Spawn_Now[cohort][i] = (int)(FunctGroupArray[sp].spawnSpeciesParams[nspawn][Time_Spawn_id]) + (i * spawn_increment);
                EMBRYO[sp].StartDay[cohort][i] = EMBRYO[sp].Spawn_Now[cohort][i] + (int)(FunctGroupArray[sp].speciesParams[Recruit_Time_id]);
                EMBRYO[sp].EndDay[cohort][i] = EMBRYO[sp].StartDay[cohort][i] + (int)EMBRYO[sp].RecruitPeriod[i];
                EMBRYO[sp].Age_Now[cohort][i] = EMBRYO[sp].StartDay[cohort][i] - 1;
                
                if ((do_debug) && bm->which_check)
                    fprintf(llogfp, "%s-%d i: %d StartDay: %d\n", FunctGroupArray[sp].groupCode, cohort, i, EMBRYO[sp].StartDay[cohort][i]);
            }
        }
        EMBRYO[sp].inter_yr_reprod_overlap = 0;
    }
    */
    
    /** Initialise adults_spawning entry, just in case needed **/
    /* If a vertebrate using fixed_linear_recruit need to get entire adult
     population to see total recruits produced - can't base it on local
     population as adults and juveniles spatial disjunct */
    if (recruit_sp != fixed_linear_recruit) {
        for (stock_id = 0; stock_id < bm->K_num_stocks_per_sp; stock_id++) {
            adults_spawning[stock_id] = 1.0;
        }
    } else if ( FunctGroupArray[sp].groupAgeType == AGE_STRUCTURED_BIOMASS) {
        for(cohort = 0; cohort < FunctGroupArray[sp].numCohortsXnumGenes; cohort++){
            den = FunctGroupArray[sp].totNTracers[cohort];
            for (i = 0; i < nboxes; i++) {
                stock_id = bm->group_stock[sp][i][0];
                if (FunctGroupArray[sp].habitatType == EPIFAUNA) {
                    adults_spawning[stock_id] += FunctGroupArray[sp].scaled_FSPB[cohort] * bm->boxes[i].epi[den];
                } else {
                    for (j = 0; j < bm->boxes[i].nz; j++) {
                        stock_id = bm->group_stock[sp][i][j];
                        adults_spawning[stock_id] += FunctGroupArray[sp].scaled_FSPB[cohort] * bm->boxes[i].tr[j][den];
                    }
                }
            }
        }

    } else {
        for (stock_id = 0; stock_id < bm->K_num_stocks_per_sp; stock_id++) {
            adults_spawning[stock_id] = 0.0;
        }
        for(cohort = 0; cohort < FunctGroupArray[sp].numCohortsXnumGenes; cohort++){
            /* Assume from first migrant group only */
            basecohort = (int)floor(cohort / FunctGroupArray[sp].numGeneTypes);
            stage = FunctGroupArray[sp].cohort_stage[basecohort];
            for(mid = 0; mid < FunctGroupArray[sp].num_migrate; mid++) {
                stock_id = (int) (MIGRATION[sp].returnstock_Prm[stage][mid]) - 1;
                if (stock_id >= bm->K_num_stocks_per_sp){
                    quit("return stock (%d) for group %s is not a valid value. Max valid value is %d\n", stock_id, FunctGroupArray[sp].groupCode, bm->K_num_stocks_per_sp - 1);
                }
                if (stock_id < 0) {
                    /* Homogeneously spread across stocks */
                    for (j = 0; j < FunctGroupArray[sp].numStocks; j++){
                        adults_spawning[j] += FunctGroupArray[sp].scaled_FSPB[cohort] * (MIGRATION[sp].InitDEN[cohort][mid] / maxstock);
                    }
                } else {
                    /* Directed to a specific stock */
                    
                    /**
                	fprintf(bm->logFile, "%s-%d scaled_FSPB: %e, InitDEN: %e, adults_spawning[stock %d]: %e\n", FunctGroupArray[sp].groupCode, cohort, FunctGroupArray[sp].scaled_FSPB[cohort], MIGRATION[sp].InitDEN[cohort][mid], stock_id, adults_spawning[stock_id]);
                    **/

                    adults_spawning[stock_id] += FunctGroupArray[sp].scaled_FSPB[cohort] * MIGRATION[sp].InitDEN[cohort][mid];
                }
            }
        
            /* Now do those inside the model domain */
            den = FunctGroupArray[sp].NumsTracers[cohort];
        
            for (i = 0; i < nboxes; i++) {
                for (j = 0; j < bm->boxes[i].nz; j++) {
                    stock_id = bm->group_stock[sp][i][j];
                    adults_spawning[stock_id] += FunctGroupArray[sp].scaled_FSPB[cohort] * bm->boxes[i].tr[j][den];
                }
            }
        }
    }
    
    EMBRYO[sp].SpawnRecruitOverlap = 0;
    EMBRYO[sp].CounterNotDone = 1;
    EMBRYO[sp].next_spawn = 0;
    EMBRYO[sp].next_larvae = 0;
    EMBRYO[sp].next_recruit = 0;
    EMBRYO[sp].next_age = 0;
    prm_spawn = -1;
    yr_scalar = 0.0;
    yr_counter = 0;
    
    for( nspawn = 0; nspawn < EMBRYO[sp].num_in_larval_queue; nspawn++){  // As all queues rhe same length
        prm_spawn++;
        
        if(prm_spawn > (FunctGroupArray[sp].numSpawns - 1)){  // clicked over so start again
            prm_spawn = 0;
        }
        
        if (yr_counter >= FunctGroupArray[sp].numSpawns){
            yr_scalar += 1.0;
            yr_counter = 0;
        }
        
        lid = nspawn;
        qid = nspawn;
        aid = nspawn;
        Time_Spawn_sp = (int) (FunctGroupArray[sp].spawnSpeciesParams[prm_spawn][Time_Spawn_id]);
        Recruit_Period_sp = (int) (FunctGroupArray[sp].speciesParams[Recruit_Period_id]);
        Recruit_Time_sp = (int) (FunctGroupArray[sp].speciesParams[Recruit_Time_id]);
        AgeClassSize_sp = (double)(FunctGroupArray[sp].ageClassSize);
        flagdem = (int) (FunctGroupArray[sp].speciesParams[flagdem_id]);
        recruit_sp = (int) (FunctGroupArray[sp].speciesParams[flagrecruit_id]);
        FunctGroupArray[sp].speciesParams[larval_queue_extension_id] = 0;

        // Assume the genetypes can go at different times but that all cohorts per genetype go at the same time
        // This is different to the old code where it was the cohort not the genetype that was being set
        for (ngene = 0; ngene < FunctGroupArray[sp].numGeneTypes; ngene++) {
            larval_queue_extension = 0;
            if (bm->flagrandom) {
                temp1 = drandom(-14.0, 14.0);
                if (temp1 < 0.0)
                    VarTime1 = (int) (ceil(temp1 - 0.5));
                else
                    VarTime1 = (int) (floor(temp1 + 0.5));
                temp2 = drandom(-7.0, 7.0);
                if (temp2 < 0.0)
                    VarTime2 = (int) (ceil(temp2 - 0.5));
                else
                    VarTime2 = (int) (floor(temp2 + 0.5));
            } else {
                VarTime1 = 0;
                VarTime2 = 0;
            }
		
            for(bcohort = 0; bcohort < FunctGroupArray[sp].numCohorts; bcohort++){
                cohort = (FunctGroupArray[sp].numGeneTypes * bcohort) + ngene;
                basecohort = (int)floor(cohort / FunctGroupArray[sp].numGeneTypes);
                /* Start with Migrants */
                stage = FunctGroupArray[sp].cohort_stage[basecohort];

                /* Find spawning date */
                EMBRYO[sp].Spawn_Now[cohort][qid] = Time_Spawn_sp + VarTime1;
                if (EMBRYO[sp].Spawn_Now[cohort][qid] < 2)
                    EMBRYO[sp].Spawn_Now[cohort][qid] = 2;
                else if (EMBRYO[sp].Spawn_Now[cohort][qid] >= 364)
                    EMBRYO[sp].Spawn_Now[cohort][qid] = 363;
                
                /**
                if (do_debug && (bm->which_check == sp)) {
                    fprintf(llogfp, "%s-%d qid: %d, SpawnNow: %d, Time_Spawn_sp: %d, VarTime: %d\n", FunctGroupArray[sp].groupCode, cohort, qid, EMBRYO[sp].Spawn_Now[cohort][qid], Time_Spawn_sp, VarTime1);
                }
                **/
                
                /* Find dates of recruitment (beginning and end) */
                EMBRYO[sp].StartDay[cohort][lid] = EMBRYO[sp].Spawn_Now[cohort][qid] + Recruit_Time_sp + VarTime2;
                
                /* Avoid the tick over (aging or startday) being on the first day of the year as can trip stuff
                 up so in those cases shuffle a few days further into the year
                 */
                thisday = (double)(EMBRYO[sp].StartDay[cohort][lid]);
                if ((thisday / 365.0) < 1.0)
                    startTofY = (int) (floor(thisday + 0.5));
                else
                    startTofY = (int) (floor((thisday / 365.0 - floor(thisday / 365.0)) * 365.0 + 0.5));

                if( startTofY < 2 ){
                    diff = 2 - startTofY;
                    EMBRYO[sp].StartDay[cohort][lid] += diff;
                }

                /*
                if (do_debug && (bm->which_check == sp)) {
                    fprintf(llogfp, "%s qid: %d, lid: %d, StartDay: %d, SpawnNow: %d, RecTime: %d, VarTime: %d\n", FunctGroupArray[sp].groupCode,
						qid, lid, EMBRYO[sp].StartDay[cohort][lid], EMBRYO[sp].Spawn_Now[cohort][qid], Recruit_Time_sp, VarTime2);
                }
                 */

                if (EMBRYO[sp].StartDay[cohort][lid] < EMBRYO[sp].Spawn_Now[cohort][qid])
                    EMBRYO[sp].StartDay[cohort][lid] = EMBRYO[sp].Spawn_Now[cohort][qid];
                
                EMBRYO[sp].RecruitPeriod[lid] = Recruit_Period_sp;
                EMBRYO[sp].EndDay[cohort][lid] = EMBRYO[sp].StartDay[cohort][lid] + (int)EMBRYO[sp].RecruitPeriod[lid];

                /*
                if (do_debug && (bm->which_check == sp)) {
                    fprintf(llogfp, "%s-%d lid: %d, EndDay: %d, StartDay: %d, RecPeriod: %f ", FunctGroupArray[sp].groupCode, cohort,
						lid, EMBRYO[sp].EndDay[cohort][lid], EMBRYO[sp].StartDay[cohort][lid], EMBRYO[sp].RecruitPeriod[lid]);

					fprintf(llogfp, "%s-%d has Spawn_now[%d]: %d, StartDay: %d, EndDay: %d, RecruitPeriod: %e\n",
						FunctGroupArray[sp].groupCode, cohort, qid, EMBRYO[sp].Spawn_Now[cohort][qid],
						EMBRYO[sp].StartDay[cohort][lid], EMBRYO[sp].EndDay[cohort][lid], EMBRYO[sp].RecruitPeriod[lid]);
                }
                 */
                
                // Check if larval period > 364 days (would have wrapped in old code, needed to set up values in first_year)
                // = 0;
                EMBRYO[sp].wraps[cohort] = 0;

                // Things get messy if end day falls write on the calendar rollover so trim as needed
                if ((EMBRYO[sp].EndDay[cohort][lid] > 363) && (EMBRYO[sp].EndDay[cohort][lid] < 366)){
                    if(EMBRYO[sp].StartDay[cohort][lid] == 363){
                        EMBRYO[sp].StartDay[cohort][lid] = 362; // Need to make sure they get one day to arrive in at least
                    }
                    EMBRYO[sp].EndDay[cohort][lid] = 363;  // Trim a bit short of the end of the year just to be safe.
                }
                // Otherwise can continue as normal by wrapping into the new year
                if (EMBRYO[sp].EndDay[cohort][lid] > 364){
                    EMBRYO[sp].wraps[cohort] = 1;
                }
                
                // Now have calendar sorted add it on to current day to give actual day of events
                EMBRYO[sp].Spawn_Now[cohort][qid] += (yr_scalar * 365.0);
                EMBRYO[sp].StartDay[cohort][lid] += (yr_scalar * 365.0);
                EMBRYO[sp].EndDay[cohort][lid] += (yr_scalar * 365.0);

                /*
                if (do_debug && (bm->which_check == sp)) {
                    fprintf(llogfp, "%s-%d qid: %d, EndDay: %d StartDay: %d RecPeriod: %f with Spawn_now[%d]: %d\n", FunctGroupArray[sp].groupCode, cohort, qid, EMBRYO[sp].EndDay[cohort][lid], EMBRYO[sp].StartDay[cohort][lid], EMBRYO[sp].RecruitPeriod[lid], qid, EMBRYO[sp].Spawn_Now[cohort][qid]);
               }
                */
            }
        }
        yr_counter++;
    }
    
    // Now shuffle based on needing to insert any from the spawns prior to the start of the model
    for( nspawn = 0; nspawn < FunctGroupArray[sp].numSpawns; nspawn++){  // As all queues rhe same length
        // Assume the genetypes can go at different times but that all cohorts per genetype go at the same time
        // This is different to the old code where it was the cohort not the genetype that was being set
        lid = nspawn;
        qid = nspawn;
        aid = nspawn;
        for (ngene = 0; ngene < FunctGroupArray[sp].numGeneTypes; ngene++) {
            larval_queue_extension = 0;
            for(bcohort = 0; bcohort < FunctGroupArray[sp].numCohorts; bcohort++){
                cohort = (FunctGroupArray[sp].numGeneTypes * bcohort) + ngene;
                basecohort = (int)floor(cohort / FunctGroupArray[sp].numGeneTypes);
                /* Start with Migrants */
                stage = FunctGroupArray[sp].cohort_stage[basecohort];

                /* Check for wrapping in first year so can insert recruits from pervious years */
                // Start with spawning countback
                countback_spawn = 0;
                temp_val = EMBRYO[sp].Spawn_Now[cohort][qid];
                if (first_spawn < 0) {
                    first_spawn = (double)(temp_val);
                }
                
                while ( temp_val > 364 ) {
                    temp_val -= 364;
                    countback_spawn++;
                }
            
                // Create the inserted countback cases
                if (countback_spawn) {
                    temp_val_start = EMBRYO[sp].Spawn_Now[cohort][qid];
                    start_i = 0;
                    // Start by finding the insertion point
                    for(i = 0; i < FunctGroupArray[sp].numSpawns; i++){
                        if(temp_val_start > EMBRYO[sp].Spawn_Now[cohort][i]) {
                            start_i = i;
                            break;
                        }
                    }
                
                    // Shuffle entries out of the way - including qid as will need that to lead into next year
                    for(i=EMBRYO[sp].num_in_spawn_queue - countback_spawn - 1; i >= start_i; i--){
                        EMBRYO[sp].Spawn_Now[cohort][i + countback_spawn] = EMBRYO[sp].Spawn_Now[cohort][i];
                    }
                
                    // Now insert the new values after countback
                    j = 0;
                    for(i = countback_spawn; i >= 1; i--){
                        EMBRYO[sp].Spawn_Now[cohort][start_i + j] = temp_val_start - 365 * i;
                        j++;
                    }
                }
            
                // Now do recruitment
                countback_recruit = 0;
                temp_val_end = EMBRYO[sp].EndDay[cohort][lid];
                while ( temp_val_end > 364 ) {
                    temp_val_end -= 364;
                    countback_recruit++;
                }
            
                //fprintf(llogfp, "%s-%d has countback_recruit: %d ", FunctGroupArray[sp].groupCode, cohort, countback_recruit);
                
                if (countback_recruit) {
                    if ( countback_recruit > larval_queue_extension )
                        larval_queue_extension = countback_recruit;
                
                    temp_val_start = EMBRYO[sp].StartDay[cohort][lid];
                    temp_val_end = EMBRYO[sp].EndDay[cohort][lid];
                
                    // New method = shuffle the spawns forward and insert those that would have occurred last year
                    // Start by finding the insertion point
                    start_i = 0;
                    for(i = 0; i < EMBRYO[sp].num_in_larval_queue; i++){
                        if(temp_val_start > EMBRYO[sp].StartDay[cohort][i]) {
                            start_i = i;
                            break;
                        }
                    }

                    // Shuffle entries out of the way - including lid as will need that to lead into next year
                    for(i=EMBRYO[sp].num_in_larval_queue - countback_recruit - 1; i >= start_i; i--){
                        EMBRYO[sp].StartDay[cohort][i + countback_recruit] = EMBRYO[sp].StartDay[cohort][i];
                        EMBRYO[sp].EndDay[cohort][i + countback_recruit] = EMBRYO[sp].EndDay[cohort][i];
                    }
                
                    // Now insert the new values for countback
                    j = 0;
                    for(i = countback_recruit; i >= 1; i--){
                        days_lost = 0;
                        EMBRYO[sp].StartDay[cohort][start_i + j] = temp_val_start - 365 * i;
                        EMBRYO[sp].EndDay[cohort][start_i + j] = temp_val_end - 365 * i;
                    
                        if(EMBRYO[sp].StartDay[cohort][start_i + j] < 2) {
                            days_lost = 2 - EMBRYO[sp].StartDay[cohort][start_i + j];
                            EMBRYO[sp].StartDay[cohort][start_i + j] = 2;
                        }
                    
                        /**
                        if (do_debug && (bm->which_check == sp)) {
                            fprintf(llogfp, "%s-%d after update, StartDay: %d (lid_startday: %d), EndDay: %d\n",
                                FunctGroupArray[sp].groupCode, cohort, EMBRYO[sp].StartDay[cohort][start_i + j], EMBRYO[sp].StartDay[cohort][lid],EMBRYO[sp].EndDay[cohort][start_i + j]);
                        }
                        **/
                        
                        j++;
                    }
                    
                    
                    /* For purposes of consistency with old runs allow for replication of old calenadar alignment juggle in first year */
                    if ((FunctGroupArray[sp].numSpawns < 2) && bm->flag_replicate_old_calendar) {
                        EMBRYO[sp].StartDay[cohort][1] += days_lost;
                    }
                }
            }

            /*
            if (sp == 9) {
                fprintf(llogfp, "and larval_queue_extension: %d\n", larval_queue_extension);
            }
            */
            
            /* Get actual number of recruits arriving and their settlement sites */
            if (larval_queue_extension) {
                FunctGroupArray[sp].speciesParams[larval_queue_extension_id] = larval_queue_extension;
                for (i = 0; i < nboxes; i++) {
                    diff = bm->wcnz - bm->boxes[i].nz;
                    for (j = 0; j < bm->boxes[i].nz; j++) {
                        if (bm->boxes[i].nz < nlayers) {
                            diffdeep = nlayers - bm->boxes[i].nz;
                            sumnzj1 = 0.0;
                            sumnzj2 = 0.0;
                            for (k = 0; k < bm->boxes[i].nz; k++) {
                                layerk = k + diffdeep - 1;
                                sumnzj1 += recruit_vdistrib[layerk][sp];
                                sumnzj2 += recruit_vdistrib[k][sp];
                            }
                            if (sumnzj1 < sumnzj2) {
                                vertdistrib = recruit_vdistrib[j][sp] / sumnzj2;
                            } else if (!sumnzj1) {
                                /* If demersal and on the bottom */
                                if (flagdem && (j == 0)) {
                                    vertdistrib = 1;
                                    /* If pelagic and at the surface */
                                } else if (!flagdem && (j == bm->boxes[i].nz - 1)) {
                                    vertdistrib = 1;
                                } else
                                    vertdistrib = 0;
                            } else {
                                layerk = j + diffdeep - 1;
                                vertdistrib = recruit_vdistrib[layerk][sp] / sumnzj1;
                            }
                        } else
                            vertdistrib = recruit_vdistrib[j][sp];

                        stock_id = bm->group_stock[sp][i][j];

                        /* If not fixed_linear_recruit then adults_spawning = 1.0
                         so effectively just spreading recruits read-in via KDENR
                         across the model domain (boxes and layers). If
                         fixed_linear_recruit is in use then adults_spawning
                         is the number spawning so scaling pups_per_adult by number
                         of adults to get the final numbers produced
                         */
                        if(FunctGroupArray[sp].isCultured)
                            prod_scalar = 1.0;
                        else
                            prod_scalar = adults_spawning[stock_id];
                        ngenes = (double)(FunctGroupArray[sp].numGeneTypes);
                        
                        /* Calculate the number of recruits for each spawning event that will happen before the start of the model */
                        for(k = 0; k < larval_queue_extension; k++){
                            
                            /* Update total number of potential recruits that were spawned last year (before model began)
                             and arriving this year (first year of model run). All potential recruits spawned year one will
                             happen explicitly in the run so no calculations needed here - or will end up with too many of them!!!! */
                            if(bm->flag_old_embryo_init) { // Old case where used KDENR regardless of standard stock recruit relationship
                                tot_embryo = KDENR[sp][stock_id];
                                EMBRYO[sp].num_recruits[i][j][ngene][k] = tot_embryo * prod_scalar * vertdistrib * bm->recruit_hdistrib[ngene][i][sp] / ngenes;
                                EMBRYO[sp].Larvae[stock_id][ngene][k] += EMBRYO[sp].num_recruits[i][j][ngene][k];
                                EMBRYO[sp].num_recruits_updating[i][j][ngene][k] = EMBRYO[sp].num_recruits[i][j][ngene][k];
                            } else {
                                tot_embryo = Get_Init_Embryos(bm, sp, ngene, stock_id, k, first_spawn, do_debug);
                                EMBRYO[sp].num_recruits[i][j][ngene][k] = tot_embryo * vertdistrib * bm->recruit_hdistrib[ngene][i][sp] / ngenes;  // Don't store the summation for new code as calculate Larvae in Get_Init_Embryos()
                                EMBRYO[sp].num_recruits_updating[i][j][ngene][k] = EMBRYO[sp].num_recruits[i][j][ngene][k];
                            }
                            
                            /**
                            //if(do_debug && (bm->which_check == sp)) {
                                    fprintf(llogfp, "%s, num_recruits-%d-%d: %e, flag_old_embryo_init: %d, VERTembryo%s-%d: %e adults_spawning: %e, prod_scalar: %e vertdistrib: %e, bm->recruit_hdistrib%d-%s: %e, ngenes: %e tot_embryo: %e\n", FunctGroupArray[sp].groupCode, i, j, EMBRYO[sp].num_recruits[i][j][ngene][k], bm->flag_old_embryo_init, FunctGroupArray[sp].groupCode, stock_id, EMBRYO[sp].Larvae[stock_id][ngene][k], adults_spawning[stock_id], prod_scalar, vertdistrib, i, FunctGroupArray[sp].groupCode, bm->recruit_hdistrib[ngene][i][sp], ngenes, tot_embryo);
                            }
                            **/
                            
                            EMBRYO[sp].next_larvae = k;
                            if (EMBRYO[sp].next_larvae > EMBRYO[sp].num_in_spawn_queue)
                                quit("EMBRYO[%s].next_larvae (%d, k %d) set > EMBRYO[].num_in_spawn_queue %d)\n", FunctGroupArray[sp].groupCode, EMBRYO[sp].next_larvae, k, EMBRYO[sp].num_in_spawn_queue);
                        }
                    }
                }
            }
        }
    }
    
    /* Find date of aging */
    for( nspawn = 0; nspawn < EMBRYO[sp].num_in_larval_queue; nspawn++){  // As all queues rhe same length
        lid = nspawn;
        qid = nspawn;
        aid = nspawn;
        for (ngene = 0; ngene < FunctGroupArray[sp].numGeneTypes; ngene++) {
            for(bcohort = 0; bcohort < FunctGroupArray[sp].numCohorts; bcohort++){
                cohort = (FunctGroupArray[sp].numGeneTypes * bcohort) + ngene;
                basecohort = (int)floor(cohort / FunctGroupArray[sp].numGeneTypes);
                stage = FunctGroupArray[sp].cohort_stage[basecohort];
 
                if (AgeClassSize_sp >= 1.0) {
                    EMBRYO[sp].Age_Now[cohort][aid] = EMBRYO[sp].StartDay[cohort][lid] - 1;

                    /*
                    if (do_debug && (bm->which_check == sp)){
                        fprintf(llogfp,"Aging %s-%d queue stid: %d aid: %d lid: %d age_now: %d StartDay: %d EndDay: %d\n",
                            FunctGroupArray[sp].groupCode, cohort, stid, aid, lid, EMBRYO[sp].Age_Now[cohort][aid+stid],
                            EMBRYO[sp].StartDay[cohort][lid+stid], EMBRYO[sp].EndDay[cohort][lid+stid]);
                    }
                     */
                } else {
            
                    /* TODO Actually allow for AgeClassSize < Cohort age span */
            
                    /* When doing it a new each time do it as for larger ageclass sizes,
                     *  but over whole queue when updating queue becareful that don't
                     *  lose entries that have wrapped into next calendar year
                     */

                    stepsize = (int) floor(ROUNDGUARD + (364.0 * AgeClassSize_sp));
                    num_steps = (int) ceil(364.0 / AgeClassSize_sp);

                    /* TODO: May need to put in a check that not accidentally wiping valid dates created in earlier years */
                    for (stid = aid; stid < aid + num_steps; stid++) {
                        // So start at spawning day and step forward through the year
                        EMBRYO[sp].Age_Now[cohort][stid] = EMBRYO[sp].StartDay[cohort][lid] - 1 + (stid - aid) * stepsize;
                    }
                }

                /* Check dates sensible */
                if(old_check){
                    Pad_The_Demography_Queues(bm, sp, cohort, nspawn, qid, lid, aid, llogfp);  // Not needed as do the whole queue at once now
                    CheckDates(bm, sp, cohort, ngene, aid, lid, qid, nspawn, do_debug, (yr_scalar * 365.0), llogfp); // Not needed as do the whole queue at once now
                }
                PrintDates(bm, sp, cohort, ngene, aid, lid, qid, nspawn, do_debug, (yr_scalar * 365.0), llogfp);
                CheckSpawnDates(bm, sp, cohort, lid, qid);
                
                if (!bcohort) {
                    AlignVsMigration(bm, sp, cohort, lid, qid);
                }
            }
        }
    }

    // Save final spawning and aging info
    EMBRYO[sp].next_age = 0;
    EMBRYO[sp].next_spawn = 0;
    EMBRYO[sp].next_recruit = 0;

    day_to_check = MAXINT;
    for(chrt = 0; chrt < FunctGroupArray[sp].numCohortsXnumGenes; chrt++) {
        if ((EMBRYO[sp].Spawn_Now[chrt][EMBRYO[sp].next_spawn] >= bm->dayt) && (day_to_check > EMBRYO[sp].Spawn_Now[chrt][EMBRYO[sp].next_spawn])) {
            day_to_check = EMBRYO[sp].Spawn_Now[chrt][EMBRYO[sp].next_spawn];
        }
        EMBRYO[sp].next_spawn_any_age = day_to_check;
        
    }
    
    day_to_check = MAXINT;
    for(chrt = 0; chrt < FunctGroupArray[sp].numCohortsXnumGenes; chrt++) {
        if ((EMBRYO[sp].Age_Now[chrt][EMBRYO[sp].next_age] >= bm->dayt) && (day_to_check > EMBRYO[sp].Age_Now[chrt][EMBRYO[sp].next_age])) {
            day_to_check = EMBRYO[sp].Age_Now[chrt][EMBRYO[sp].next_age];
        }
        EMBRYO[sp].next_age_any_age = day_to_check;
        
    }
    
    /* Sanity checks */
    if ((EMBRYO[sp].next_spawn_any_age < 0) || (EMBRYO[sp].next_spawn_any_age > ((365.0 * bm->tstop) / 86400.0)))
        quit("First spawning date for %s (%d) makes no sense - note testing against 0 and %e as tstop is : %e\n", FunctGroupArray[sp].groupCode, EMBRYO[sp].next_spawn_any_age, ((365.0 * bm->tstop) / 86400.0), bm->tstop);
    if ((EMBRYO[sp].next_age_any_age < 0) || (EMBRYO[sp].next_age_any_age > ((365.0 * bm->tstop) / 86400.0)))
        quit("First aging date for %s (%d) makes no sense - note testing against 0 and %e as tstop is : %e\n", FunctGroupArray[sp].groupCode, EMBRYO[sp].next_age_any_age, ((365.0 * bm->tstop) / 86400.0), bm->tstop);
    
	return;
}

/**
 * \brief Pad out the queue in case age faster than have entries in the queue
 */
void Pad_The_Demography_Queues(MSEBoxModel *bm, int sp, int cohort, int nspawn, int qid, int lid, int aid, FILE *llogfp) {
	int i, step, maxage = 0, maxspawn = 0, maxstart = 0, maxend = 0, maxage_id = 0, maxspawn_id = 0, maxstart_id = 0, maxend_id = 0;
	int AgeClassSize_sp = FunctGroupArray[sp].ageClassSize;
    int stepsize;

	if (AgeClassSize_sp < 1.0)   // TODO: ALLOW FOR MULTIPLE SPAWNING PER YEAR - currently only handles when one spawning/aging per year
		return;
    
    /* Deal with one spawn per year case */
    if(FunctGroupArray[sp].numSpawns < 2) {
        // First up find the max date in each case
        for (i = 0; i < EMBRYO[sp].num_in_aging_queue; i++) {
            if(EMBRYO[sp].Age_Now[cohort][i] > maxage) {
                maxage = EMBRYO[sp].Age_Now[cohort][i];
                maxage_id = i;
            }
        }
        for (i = 0; i < EMBRYO[sp].num_in_spawn_queue; i++) {
            if(EMBRYO[sp].Spawn_Now[cohort][i] > maxspawn) {
                maxspawn = EMBRYO[sp].Spawn_Now[cohort][i];
                maxspawn_id = i;
            }
        }
        for (i = 0; i < EMBRYO[sp].num_in_larval_queue; i++) {
            if(EMBRYO[sp].StartDay[cohort][i] > maxstart) {
                maxstart = EMBRYO[sp].StartDay[cohort][i];
                maxstart_id = i;
            }
            if(EMBRYO[sp].EndDay[cohort][i] > maxend) {
                maxend = EMBRYO[sp].EndDay[cohort][i];
                maxend_id = i;
            }
        }

        // Now deal with oddness
        stepsize = (int)(365 / FunctGroupArray[sp].numSpawns);
        for (i = (maxage_id + 1); i < EMBRYO[sp].num_in_aging_queue; i++) {
            step = i - maxage_id;
            if (EMBRYO[sp].Age_Now[cohort][i] <= maxage) {
                EMBRYO[sp].Age_Now[cohort][i] = maxage + step * stepsize;
            }
        }
        for (i = (maxspawn_id + 1); i < EMBRYO[sp].num_in_spawn_queue; i++) {
            step = i - maxspawn_id;
            if(EMBRYO[sp].Spawn_Now[cohort][i] <= maxspawn) {
                EMBRYO[sp].Spawn_Now[cohort][i] = maxspawn + step * stepsize;
            }
        }
        for (i = (maxstart_id + 1); i < EMBRYO[sp].num_in_larval_queue; i++) {
            step = i - maxstart_id;
        
            //fprintf(llogfp,"i: %d maxstart: %d step: %d stepsize: %d StartDay begins: %d ", i, maxstart_id, step, stepsize, EMBRYO[sp].StartDay[cohort][i]);
        
            if(EMBRYO[sp].StartDay[cohort][i] <= maxstart) {
                EMBRYO[sp].StartDay[cohort][i] = maxstart + step * stepsize;
            }
        
            //fprintf(llogfp,"ends: %d\n", EMBRYO[sp].StartDay[cohort][i]);
        }
        for (i = (maxend_id + 1); i < EMBRYO[sp].num_in_larval_queue; i++) {
            step = i - maxend_id;
            if(EMBRYO[sp].EndDay[cohort][i] <= maxend) {
                EMBRYO[sp].EndDay[cohort][i] = maxend + step * stepsize;
            }
        }
    }
    /*
	if(sp == bm->which_check) {
		fprintf(llogfp, "Time: %e %s-%d maxage: %d maxspawn: %d maxstart: %d maxend: %d maxage_id: %d maxspawn_id: %d maxstart_id: %d maxend_id: %d\n",
				bm->dayt, FunctGroupArray[sp].groupCode, cohort, maxage, maxspawn, maxstart, maxend,
				maxage_id, maxspawn_id, maxstart_id, maxend_id);
	}
     */
    
	return;
}

/**
 * \brief Check on dates so that they make sense and queue not twisted
 */
void CheckDates(MSEBoxModel *bm, int sp, int cohort, int ngene, int aid, int lid, int qid, int nspawn, int do_debug, int this_day, FILE *llogfp) {
	int i;
	double AgeClassSize_sp = (double)(FunctGroupArray[sp].ageClassSize);
	int age_check, startday_check, spawn_check, endday_check;
	int age_id = expect_id;
	int spawn_id = expect_id;
	int days_id = expect_id;

	/* Find which dates to check */
	age_check = EMBRYO[sp].Age_Now[cohort][expect_id];     // as is just a simple check vs next_age when calling Update_Age_Queue()

	if ((EMBRYO[sp].EndDay[cohort][expect_id] + 1) <= this_day) {
		startday_check = EMBRYO[sp].StartDay[cohort][expect_id + 1];  // As need to check the coming date, once Ecology_Init_Spawn_Params() updates, is ok
		endday_check = EMBRYO[sp].EndDay[cohort][expect_id + 1];
		days_id += 1;
	} else {
		startday_check = EMBRYO[sp].StartDay[cohort][expect_id];  // As no weirdness with wrapping over the end of the year so a simple check
		endday_check = EMBRYO[sp].EndDay[cohort][expect_id];
	}

	if ((EMBRYO[sp].Spawn_Now[cohort][expect_id] + 2) <= this_day) {
		spawn_check = EMBRYO[sp].Spawn_Now[cohort][expect_id + 1];    // As need to check the coming date, once Update_Spawn_Queue() updates, is ok
		spawn_id += 1;
	} else
		spawn_check = EMBRYO[sp].Spawn_Now[cohort][expect_id];  // As no weirdness with wrapping over the end of the year so a simple check

	if ((do_debug && (bm->which_check == sp)) || ((age_check < this_day) || (startday_check < this_day) || (spawn_check < this_day))) {
		fprintf(llogfp, "%s-%d day: %f first_year: %d, AgeClassSize_sp: %f, num_in_yr: %d, Spawn_Now: %d, StartDay: %d, RecruitPeriod: %f, EndDay: %d, Age_now: %d\n",
				FunctGroupArray[sp].groupCode, cohort, bm->dayt, first_year, AgeClassSize_sp,
				EMBRYO[sp].num_in_yr, EMBRYO[sp].Spawn_Now[cohort][qid],
				EMBRYO[sp].StartDay[cohort][days_id], EMBRYO[sp].RecruitPeriod[days_id],
				EMBRYO[sp].EndDay[cohort][days_id], EMBRYO[sp].Age_Now[cohort][age_id]);

		for (i = 0; i< EMBRYO[sp].num_in_aging_queue; i++)
			fprintf(llogfp, "%s-%d has Age_now[%d]: %d\n", FunctGroupArray[sp].groupCode, cohort, i, EMBRYO[sp].Age_Now[cohort][i]);

		for (i = 0; i< EMBRYO[sp].num_in_spawn_queue; i++) {
			fprintf(llogfp, "%s-%d has Spawn_now[%d]: %d, StartDay: %d, EndDay: %d, RecruitPeriod: %e\n",
				FunctGroupArray[sp].groupCode, cohort, i, EMBRYO[sp].Spawn_Now[cohort][i],
				EMBRYO[sp].StartDay[cohort][i], EMBRYO[sp].EndDay[cohort][i], EMBRYO[sp].RecruitPeriod[i]);
		}

		/*
		if(cohort == ((bm->K_num_max_cohort * bm->K_num_max_genetypes) - 1)){
			for (i = 0; i< EMBRYO[sp].num_in_larval_queue; i++) {
				fprintf(llogfp, "%s has Larvae[%d]: %e\n", FunctGroupArray[sp].groupCode, i, EMBRYO[sp].Larvae[0][0][i]);
				for (b = 0; b < bm->nbox; b++) {
					for ( k = 0; k < bm->wcnz; k++) {
						fprintf(llogfp, "%s has num_rec[box: %d][layer: %d][%d]: %e\n",
							FunctGroupArray[sp].groupCode, b, k, i, EMBRYO[sp].num_recruits[b][k][ngene][i]);
					}
				}
			}
		}
		*/

		if((age_check < this_day) || (endday_check < this_day) || (spawn_check < this_day)){
			fflush(llogfp);
			quit("There was a spawning/aging date foul up in Annual_biology for %s see log file for details (age_now: %d start_day: %d, end_day: %d spawn_now: %d this_day: %d)\n",
					FunctGroupArray[sp].groupCode, EMBRYO[sp].Age_Now[cohort][age_id], EMBRYO[sp].StartDay[cohort][days_id],
					EMBRYO[sp].EndDay[cohort][days_id], EMBRYO[sp].Spawn_Now[cohort][spawn_id], this_day);
		}
	}
    
    return;
}

/* Calibration helper messages */
void CheckSpawnDates(MSEBoxModel *bm, int sp, int cohort, int lid, int qid) {
    if((EMBRYO[sp].Spawn_Now[cohort][qid] >= EMBRYO[sp].StartDay[cohort][lid]) && (EMBRYO[sp].Spawn_Now[cohort][qid] <= EMBRYO[sp].EndDay[cohort][lid]))
        EMBRYO[sp].SpawnRecruitOverlap = 1;
    
    return;
}

/* Calibration helper messages */
void PrintDates(MSEBoxModel *bm, int sp, int cohort, int ngene, int aid, int lid, int qid, int nspawn, int do_debug, int this_day, FILE *llogfp) {
    int k;
    
    //do_debug = 1;
    
	//if (do_debug && (bm->which_check == sp)) {
        fprintf(llogfp, "%s-%d SpawnDay: %d AgeDay: %d StartDay: %d EndDay: %d ",
            FunctGroupArray[sp].groupCode, cohort, EMBRYO[sp].Spawn_Now[cohort][qid], EMBRYO[sp].Age_Now[cohort][aid], EMBRYO[sp].StartDay[cohort][lid], EMBRYO[sp].EndDay[cohort][lid]);
            
        for (k = 0; k < FunctGroupArray[sp].numStocks; k++) {
            fprintf(llogfp, " VERTembryo-%d %e", k, EMBRYO[sp].Larvae[k][ngene][lid]);
        }

        fprintf(llogfp, "\n");

	//}

	return;
}

/**
 * \brief Figure out which MigID in MIGRATION Array that this set of spawn and recruit array entries
*
*
*
*/
void AlignVsMigration(MSEBoxModel *bm, int sp, int cohort, int lid, int qid) {
    int counter;

    // First check that the fucntional group migrates
    if (FunctGroupArray[sp].num_migrate > 0) {
        
        // Now for for that cohort check the MIGRATION queue entries - for overlapping dates and for whtehr or not the youngest cohorts (the cohorts making this call) is migrating at that time
        for (counter = 0; counter < MIGRATION[sp].num_in_queue; counter++) {
            if ((EMBRYO[sp].StartDay[cohort][lid] >= MIGRATION[sp].Leave_Now[counter]) && (MIGRATION[sp].Return_Now[counter] >= EMBRYO[sp].StartDay[cohort][lid])) {
                if (MIGRATION[sp].cohort_migrating[cohort][counter]) {
                    if (FunctGroupArray[sp].recruitType == external_recruit) {
                        EMBRYO[sp].migIDmatch[lid] = counter;
                        MIGRATION[sp].RecruitQueueMatch[counter] = 1;  // So can trigger adding recruits in demography code
                        FunctGroupArray[sp].speciesParams[recruit_outside_id] = 1;
                    } else {
                        if(FunctGroupArray[sp].recruitType != external_spawn_local_recruit) {
                            warn("%s has RecruitType %d cohort: %d lid: %d counter: %d StartDay: %d Leave_Now: %d Return_Now: %d so the settlers will arrive while the youngest cohort is outside the model domain - check the paramters\n", FunctGroupArray[sp].groupCode, FunctGroupArray[sp].recruitType, cohort, lid, counter, EMBRYO[sp].StartDay[cohort][lid], MIGRATION[sp].Leave_Now[counter], MIGRATION[sp].Return_Now[counter]);
                        }
                        EMBRYO[sp].migIDmatch[lid] = -1;  // So trigger things to go into recruit array instead
                    }
                }
            }
        }
    }
    
    return;
}

/**
 * \brief Scale the group size based on scaler values read in from the SizeChange ts forcing input file.
 *
 *
 *
 */
void Scale_Group_Size(MSEBoxModel *bm, FILE *llogfp){

	int sp;
	int size_scale_index;
	int cohort;
	int b, k, sn, rn;
	double sizeScaler;

	if(verbose > 2)
		printf("Scaling group size\n");
	for(sp = 0; sp < bm->K_num_tot_sp; sp++){
		if(FunctGroupArray[sp].isVertebrate == TRUE){

			for(cohort = 0; cohort < FunctGroupArray[sp].numCohortsXnumGenes; cohort++){
				size_scale_index = bm->scaling_indices[size_scale_id][sp][cohort];

				if(size_scale_index > -1){
					sizeScaler = tsEval(bm->tsSizeChange, size_scale_index, bm->t);


					if(fabs(sizeScaler - 1.0) > 1e-100){
						sn = FunctGroupArray[sp].structNTracers[cohort];
						rn = FunctGroupArray[sp].resNTracers[cohort];

						for(b = 0; b < bm->nbox; b++){

							if(bm->boxes[b].type == LAND && FunctGroupArray[sp].habitatCoeffs[LAND_BASED] > 0){
								/* Cover the wc tracers first */
								fprintf(llogfp, "Before - sizeScaler = %e\n", sizeScaler);
								fprintf(llogfp, "%s:%d bm->boxes[%d].tr[0][sn] = %e\n", FunctGroupArray[sp].groupCode, cohort, b, bm->boxes[b].tr[0][sn]);
								fprintf(llogfp, "%s:%d bm->boxes[%d].tr[0][rn] = %e\n", FunctGroupArray[sp].groupCode, cohort, b, bm->boxes[b].tr[0][rn]);
								bm->boxes[b].tr[0][sn] = bm->boxes[b].tr[0][sn] * sizeScaler;
								bm->boxes[b].tr[0][rn] = bm->boxes[b].tr[0][rn] * sizeScaler;
								fprintf(llogfp, "After\n");
								fprintf(llogfp, "%s:%d bm->boxes[%d].tr[0][sn] = %e\n", FunctGroupArray[sp].groupCode, cohort, b, bm->boxes[b].tr[0][sn]);
								fprintf(llogfp, "%s:%d bm->boxes[%d].tr[0][rn] = %e\n", FunctGroupArray[sp].groupCode, cohort, b, bm->boxes[b].tr[0][rn]);

								/* No support for SED or EPI yet */

							} else if(bm->boxes[b].type != BOUNDARY){
                                /* Cover the wc tracers first */

                                // Loop through each wc layer
                                for (k=0; k<bm->boxes[b].nz; k++) {
                                    fprintf(llogfp, "Before - sizeScaler = %e\n", sizeScaler);
                                    fprintf(llogfp, "%s:%d bm->boxes[%d].tr[%d][sn] = %e\n", FunctGroupArray[sp].groupCode, cohort, b, k, bm->boxes[b].tr[k][sn]);
                                    fprintf(llogfp, "%s:%d bm->boxes[%d].tr[%d][rn] = %e\n", FunctGroupArray[sp].groupCode, cohort, b, k, bm->boxes[b].tr[k][rn]);
                                    bm->boxes[b].tr[k][sn] = bm->boxes[b].tr[k][sn] * sizeScaler;
                                    bm->boxes[b].tr[k][rn] = bm->boxes[b].tr[k][rn] * sizeScaler;
                                    fprintf(llogfp, "After\n");
                                    fprintf(llogfp, "%s:%d bm->boxes[%d].tr[%d][sn] = %e\n", FunctGroupArray[sp].groupCode, cohort, b, k, bm->boxes[b].tr[k][sn]);
                                    fprintf(llogfp, "%s:%d bm->boxes[%d].tr[%d][rn] = %e\n", FunctGroupArray[sp].groupCode, cohort, b, k, bm->boxes[b].tr[k][rn]);
                                }
                                /* No support for SED or EPI yet */
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
 * \brief The initial number of embryos if start with larvae outside the model - uses same stock-recruitment relationship as in atdemogrpahy code
 *
 */
static double Get_Init_Embryos(MSEBoxModel *bm, int species, int ngene, int stock_id, int qid, double first_spawn, int do_debug) {
    double ans = 0.0;
    double step1, step2, jack_SSB, jack_B, jack_a, SN, RN, DEN, IndSpawn, RSprop_sp;
    double temprec = 0.0;
    double BHalpha_sp = FunctGroupArray[species].speciesParams[BHalpha_id];
    double BHbeta_sp = FunctGroupArray[species].speciesParams[BHbeta_id];
    double Ralpha_sp = FunctGroupArray[species].speciesParams[Ralpha_id];
    double Rbeta_sp = FunctGroupArray[species].speciesParams[Rbeta_id];
    double ng = (double)(FunctGroupArray[species].numGeneTypes);
    double KDENR_sp = KDENR[species][stock_id];
    double PP_sp = FunctGroupArray[species].speciesParams[PP_id];
    double sp_log_mult = FunctGroupArray[species].speciesParams[log_mult_id];    
    double norm_sigma = FunctGroupArray[species].speciesParams[norm_sigma_id];
    double thisSSB = 0.0;
    double KSPA_sp = FunctGroupArray[species].speciesParams[KSPA_id];
    double FSP_sp = FunctGroupArray[species].speciesParams[FSP_id];
    double CHLa = bm->ref_chl;
    double plankton = bm->ref_chl;
    double amt = 0.0;
    double this_t, this_larvae;
    double this_stock = 0.0;
    double prod_alpha, dd_beta1, dd_beta2, temp_coefft, rate_coefft, wind_coefft, recruit_var, current_ICE;
    
    int maxstock_id = FunctGroupArray[species].numStocks;
    int recruit_sp = (int) (FunctGroupArray[species].speciesParams[flagrecruit_id]);
    int tsrecruitid = (int) (FunctGroupArray[species].speciesParams[tsrecruit_id]);
    int cohort = 0, bcohort = 0, boxkey_id;
    
    /* Get total spawn */
    EMBRYO[species].TotSpawn[ngene] = 0.0;
    EMBRYO[species].Larvae[stock_id][ngene][qid] = 0.0;
    RSprop_sp = FunctGroupArray[species].speciesParams[RSprop_id];
    for(bcohort = 0; bcohort < FunctGroupArray[species].numCohorts; bcohort++){
        cohort = (FunctGroupArray[species].numGeneTypes * bcohort) + ngene;
        SN = initVERTinfo[species][cohort][SN_id];
        RN = initVERTinfo[species][cohort][RN_id];
        DEN = initVERTinfo[species][cohort][DEN_id];

        IndSpawn = Ecology_Age_Structured_Spawn(species, KSPA_sp, FSP_sp, RSprop_sp, SN, RN, FunctGroupArray[species].scaled_FSPB[cohort], FunctGroupArray[species].X_RS[cohort], bm->flag_repcostSpawn, bm->logFile);
        thisSSB = (RN + SN) * DEN * FunctGroupArray[species].scaled_FSPB[cohort];   // This was originally incorrectly += likely only an issue for first round of spawning ?
        bm->tot_SSB[species] += thisSSB;
        EMBRYO[species].TotSpawn[ngene] += (FunctGroupArray[species].scaled_FSPB[cohort] * IndSpawn * DEN);
        
        //fprintf(bm->logFile, "%s-%d (bcohort: %d) SN: %e RN: %e IndSpawn: %e, thisSSB: %e DEN: %e FSPB: %e TotSpawn: %e\n", FunctGroupArray[species].groupCode, cohort, bcohort, SN, RN, IndSpawn, thisSSB, DEN, FunctGroupArray[species].scaled_FSPB[cohort], EMBRYO[species].TotSpawn[ngene]);
    }

    Determine_Stock_Prop(bm, species, maxstock_id, 1, &amt, bm->logFile);
    
    /* First get larvae */
    switch (recruit_sp) {
        case no_recruit:
            quit("No such flagrecruit defined (i.e. value must be > 0)\n");
            break;
        case const_recruit: /* Fixed set of constants */
            if (EMBRYO[species].readytospawn[stock_id] == 1)
                EMBRYO[species].Larvae[stock_id][ngene][qid] = KDENR_sp / ng;
            
            if(species == 9) {
                fprintf(bm->logFile,"Init for %s qid: %d ngene: %d Larvae: %e KDENR_sp: %e ng: %e\n", FunctGroupArray[species].groupCode, qid, ngene,  EMBRYO[species].Larvae[stock_id][ngene][qid], KDENR_sp, ng);
            }
            
            break;
        case chl_recruit: /* Proportional to primary productivity */
            EMBRYO[species].Larvae[stock_id][ngene][qid] += recSTOCK[species][stock_id] * PP_sp * bm->avg_cell_vol * (CHLa / bm->ref_chl);
            break;
        case rand_recruit: /* Random - follows lognormal */
            if (EMBRYO[species].readytospawn[stock_id] == 1)
                EMBRYO[species].Larvae[stock_id][ngene][qid] = recSTOCK[species][stock_id] * sp_log_mult * Util_Logx_Result(-lognorm_mu, lognorm_sigma);
            break;
        case plank_recruit: /* Spawn is based on plankton levels (not just CHLa) */
            EMBRYO[species].Larvae[stock_id][ngene][qid] += recSTOCK[species][stock_id] * PP_sp * plankton / bm->ref_chl;
            break;
        case BevHolt_recruit: /* basical Atlantis Beverton-Holt stock-recruit relationship - so uses spawn calculated above */
        case BevHolt_rand_recruit: /* Spawn is based on Beverton Holt with lognormal variation and dependence on plankton levels  - so uses spawn calculated above */
        case BevHolt_num_recruit: /* Beverton-Holt stock-recruit relationship using numbers only - uses spawn calculated above */
        case recover_recruit: /* Beverton-Holt stock-recruit relationship - so uses spawn calculated above */
        case force_recover_recruit: /* Beverton-Holt stock-recruit relationship - so uses spawn calculated above */
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
                fprintf(bm->logFile,"Doing %s stock %d, ngene: %d, qid: %d, TotSpawn: %e, Larvae: %e\n",
                FunctGroupArray[species].groupCode, stock_id, ngene, qid, EMBRYO[species].TotSpawn[ngene],
                EMBRYO[species].Larvae[stock_id][ngene][qid]);
            }
            **/
            
            break;
        case fixed_linear_recruit:/* Pupping or calving a fixed number per adult spawning */
            for(bcohort = 0; bcohort < FunctGroupArray[species].numCohorts; bcohort++){
                cohort = (FunctGroupArray[species].numGeneTypes * bcohort) + ngene;
                DEN = initVERTinfo[species][cohort][DEN_id];
                EMBRYO[species].Larvae[stock_id][ngene][qid] += recSTOCK[species][stock_id] * KDENR_sp * FunctGroupArray[species].scaled_FSPB[cohort] * DEN;
            }
            break;
        case ts_recruit:/* Read in timeseries of recruitment */
            this_t = first_spawn * 86400.0; // Assume it is equivalent to what happens on the first spawn_day
            this_larvae = tsEval(bm->tsRecruit, tsrecruitid, this_t);
            EMBRYO[species].Larvae[stock_id][ngene][qid] = this_larvae;
            break;
        case multiple_ts_recruit: /* Read in timeseries of recruitment - assume all genotypes get the same number of recruits */
            boxkey_id = bm->BoxKeyMap[bm->current_box][rects_key_id];
            EMBRYO[species].Larvae[stock_id][ngene][qid] +=  Get_Imposed_MultRecruits(bm, species, do_debug, boxkey_id, bm->logFile);
            break;
        case jackknife_recruit: /* Jackknife spawning function - sum up spawning stock biomass */
            for(bcohort = 0; bcohort < FunctGroupArray[species].numCohorts; bcohort++){
                cohort = (FunctGroupArray[species].numGeneTypes * bcohort) + ngene;
                SN = initVERTinfo[species][cohort][SN_id];
                RN = initVERTinfo[species][cohort][RN_id];
                DEN = initVERTinfo[species][cohort][DEN_id];

                EMBRYO[species].Larvae[stock_id][ngene][qid] += DEN * FunctGroupArray[species].scaled_FSPB[cohort] * (SN + RN) * bm->X_CN * mg_2_tonne;
            }
            break;
        case BevHolt_direct_num_recruit: /* Beverton-Holt stock-recruit relationship - numbers only case (when spawn set proprtional to numbers spawning) */
            for(bcohort = 0; bcohort < FunctGroupArray[species].numCohorts; bcohort++){
                cohort = (FunctGroupArray[species].numGeneTypes * bcohort) + ngene;
                DEN = initVERTinfo[species][cohort][DEN_id];
                EMBRYO[species].Larvae[stock_id][ngene][qid] += FunctGroupArray[species].scaled_FSPB[cohort] * DEN;
            }
            break;
        default:
            quit("No such flagrecruit defined for vertebrates (i.e. value must be between 1 ands 10 at present)\n");
            break;
    }

    /* Apply stock-recruit relationship */
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
            break;
        case BevHolt_recruit: /* Beverton-Holt stock-recruit relationship - Atlantis basic version (mix numebrs and biomass) */
            temprec = (recSTOCK[species][stock_id] * BHalpha_sp * EMBRYO[species].Larvae[stock_id][ngene][qid] / (BHbeta_sp + bm->totfishpop[species] * stock_prop[species][stock_id]));
            break;
        case BevHolt_direct_num_recruit: /* Beverton-Holt stock-recruit relationship using spawn directly proportional to number of spawners */
        case BevHolt_num_recruit: /* Beverton-Holt stock-recruit relationship - numbers only case */
            temprec = (recSTOCK[species][stock_id] * BHalpha_sp * EMBRYO[species].Larvae[stock_id][ngene][qid] / (BHbeta_sp  + recSTOCK[species][stock_id] * EMBRYO[species].Larvae[stock_id][ngene][qid]));
            break;
        case BevHolt_rand_recruit: /* Spawn is based on Beverton Holt with lognormal variation and dependence on plankton levels */
            step1 = Util_Logx_Result(-lognorm_mu, lognorm_sigma);
            step2 = (recSTOCK[species][stock_id] * BHalpha_sp * EMBRYO[species].Larvae[stock_id][ngene][qid] / (BHbeta_sp + bm->totfishpop[species] * stock_prop[species][stock_id])) * (plankton / bm->ref_chl);
            temprec = step2 * step1;
            break;
        case recover_recruit: /* Spawn is allowed a recovery encouraging boost of recruits after "recovery_span" years of depressed stock levels */
            temprec = (recSTOCK[species][stock_id] * BHalpha_sp * EMBRYO[species].Larvae[stock_id][ngene][qid] / (BHbeta_sp + bm->totfishpop[species] * stock_prop[species][stock_id]));
            break;
        case force_recover_recruit: /* Spawn has a pre-specified recovery encouraging boost of recruits */
            temprec = (recSTOCK[species][stock_id] * BHalpha_sp * EMBRYO[species].Larvae[stock_id][ngene][qid] / (BHbeta_sp + bm->totfishpop[species] * stock_prop[species][stock_id]));
            break;
        case Ricker_recruit: /* Ricker */
            temprec = bm->totfishpop[species] * stock_prop[species][stock_id] * exp(recSTOCK[species][stock_id] * Ralpha_sp * (1.0 - bm->totfishpop[species] * stock_prop[species][stock_id] / Rbeta_sp));
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
            temprec = 1000 * recSTOCK[species][stock_id] * Ralpha_sp * bm->totfishpop[species] * bm->X_CN * mg_2_tonne * stock_prop[species][stock_id] * exp( -1.0 * Rbeta_sp * bm->totfishpop[species] * bm->X_CN * mg_2_tonne * stock_prop[species][stock_id]);
            break;
        case SSB_ricker: /* SSB based version of the ricker given short lived and senescent - e.g. for cephalopods */
            temprec = bm->tot_SSB[species] * stock_prop[species][stock_id] * exp(recSTOCK[species][stock_id] * Ralpha_sp * (1.0 - bm->tot_SSB[species] * stock_prop[species][stock_id] / Rbeta_sp));
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
    
    // Check for any stochasticity added to this recruitment
    if ( FunctGroupArray[species].speciesParams[flag_recruit_stochastic_id] > 0) {
        step1 = temprec;
        temprec = Util_Normx_Result(step1, norm_sigma);
    }

    ans = temprec;
    
    /**
    //if (do_debug && (bm->which_check == species)) {
    if (species == 33) {
        fprintf(bm->logFile, "Initialisation, species %s, recruit_sp case: %d, BulkRecruits: %e, temprec: %e, Larvae: %e qid: %d)\n", FunctGroupArray[species].groupCode, recruit_sp, temprec, EMBRYO[species].Larvae[stock_id][ngene][qid], qid);
    }
    **/
    
    // Reset SSB in case spawn very soon after
    bm->tot_SSB[species] = 0.0;

    return ans;
}
