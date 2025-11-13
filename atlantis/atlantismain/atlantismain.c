/*********************************************************************

 File:           atlantismain.c

 Created:        21/9/2004

 Author:         Beth Fulton
 CSIRO Division of Marine Resaech

 Purpose:        Main routine for atlantis mse box model

 Arguments:      See Util_Usage(1) routine below for explanation
 of command line arguments

 Returns:        int - 0 if successful

 Revisions:      8/8/2004 msebm.c created from sebm.c

 11/8/2004 Time related routines move to from mseeco library

 13/8/2004 added assessment flag

 17/8/2004 added management and fisheries initialisation calls

 21/8/2004 moved creation and initialisation of arrays now
 in MSEBoxModel here from other libraries

 23/8/2004 added delete_to_halt_run check

 19/9/2004 added initHaltFile() so required file created on
 runtime (so people don't forget they need it)

 21/9/2004 created msegenbm.c from msebm.c

 27/10/2004 Renamed FSL, FSO, FSF, FST and FSG to generic names
 so apply to other models/generic case.
 FSL -> FDC  (Ling -> Demersal_DC_Fish)
 FSO -> FDO  (Orange_Roughy -> Demersal_O_Fish)
 FSF -> FDB  (Flathead -> Demersal_B_Fish)
 FST -> FVO  (Bluefin_Tuna -> Pisciv_O_Fish)
 FSG -> SHB  (Gummy_Shark -> Shark_B)
 Also added the additional biological groups (prawns, other filter
 feeders, megazoobenthos, estuarine demersal fish, flat deep demersal
 fish, benthopelagics, other piscivores, penguins, skates and rays,
 reptiles and dugongs).Also added economics model calls and updated
 head of log file to match new group and fisheries names.


 17/11/2004 EA Fulton
 Converted original routine definitions from
 void
 routine_name(blah,blahblah)
 int blah;
 double blahblah;

 to

 void routine_name(int blah, double blahblah)

 19/1/2005 EA Fulton
 Added Microphytobenthos (MB)

 29/3/2005 EA Fulton
 Added bm->DIN_id so can track "stock size" of DIN. Increased
 totbiom, totfishpop, totinitpop from bm->K_num_vert_sp + K_num_prey
 entries to bm->K_num_tot_sp to allow for the DIN entry.

 8/4/2005 EA Fulton
 Added Invert_Activities() to handle mortality, growth,
 assimilation and waste equations in a general way.

 9/4/2005 EA Fulton
 Added 	bm->INVERT_PREYeatenGlobal, bm->NutsProdGlobal,
 bm->NutsLostGlobal, bm->DetritusProdGlobal, bm->DetritusLostGlobal,
 bm->NutsProd, bm->NutsLost, bm->DetritusProd, bm->DetritusLost,
 created to simplify handling of handlgin of values spanning the
 it_count == 1 and subsequent iterations of the adaptive timestep.

 23/4/2005 EA Fulton
 Added option for stress to be read in from the initial conditions file

 13/7/2005 Added readkeyprm_darray_checked() and extended readValue() so
 that knows what data is expected so that can check on readin for
 obvious parameter errors

 06-07-2009 Bec Gorton
 First attempt to split the atmanage code into atharvest, atmanage and atImplementation.

 22-07-2009 Bec Gorton
 Got rid of lots of additional loggin that was being written to stdout.

 03-02-2010 Bec Gorton
 Changed the main code to use the updated function names in each of the
 libraraies.

 25-02-2010 Bec Gorton
 Moved the model setup code back into the atlantis main module. We no longer need to use the test interface that
 meant they had to separate.

 01-03-2010 Bec Gorton
 Added code in AllocateArrayMemory() to allocate memory for the bm->connectivityMatrix and
 bm->larvaldistrib arrays.

 14-05-2012 Beth Fulton
 Added stuff for Savina seagrass model - including totagepop so can get age structured overall output

 05-07-2012 Bec Gorton
 Moved the main timestep execution into a new runNextTimeStep function. This makes it easier for the linkage code to run the next timestep.
 The main function will just call this as per normal each timestep.

 21-04-2012 Beth Fulton
 Added overwintering and encystmewnt

 25-10-2013 Bec Gorton
 Added support for external scaling values from netcdf file.

 *********************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <time.h>
#include <math.h>
#include <process.h>
#include <string.h>
#include <sjwlib.h>
#include <netcdf.h>
#include <sys/stat.h>
#include <atlantisboxmodel.h>
#include <atUtilLib.h>
#include <atEcologyLib.h>
#include <atHarvestLib.h>
#include <atManageLib.h>
#include <atImplementationLib.h>
#include <atAssessLib.h>
#include <atEconomicLib.h>
#include <atSS3LinkLib.h>
#ifdef LINK_ENABLED
#include <atComms.h>
#endif
#ifdef BROKER_LINK_ENABLED
#include <LinkageInterface.h>
#endif

#ifdef CLAM_LINK_ENABLED
#include "atCLAMLinkLib.h"
#endif

#ifdef _WIN32
	#ifdef ATLANTIS_SVN
		#include "atlantisSVN.h"
	#else
		#define ATLANTIS_REVISION "Unknown"
	#endif
	#ifndef ATLANTIS_WCDATE
		#define ATLANTIS_WCDATE "Unknown"
	#endif
	#ifndef ATLANTIS_WCPATH
		#define ATLANTIS_WCPATH "Unknown"
	#endif
	#define FOLDER_SEP "\\"
#else
	#ifndef ATLANTIS_REVISION
		#define ATLANTIS_REVISION "Unknown"
	#endif
	#ifndef ATLANTIS_WCPATH
		#define ATLANTIS_WCPATH "Unknown"
	#endif
	#ifndef ATLANTIS_WCDATE
		#define ATLANTIS_WCDATE "Unknown"
	#endif
	#define FOLDER_SEP "/"
#endif


static void set_up_vvdistrib(MSEBoxModel *bm);
static void setupMSEBoxModel(int argc, char *argv[], MSEBoxModel *bm);
static void checknetCDFFiles(MSEBoxModel *bm);

void Util_Usage(int dummy);

FILE *logfp;
FILE *haltfp;

double ***newwctr; /* new water column tracer */
double ***newsedtr; /* new sediment tracer */
double ***newicetr; /* new ice tracers */
double **newlandtr; /* new land tracers */

//extern double *****DIET_check;

int verbose = 0;
int fishtest = 0;
int killed = 0;
int do_biology = 0;
int do_migration = 0;
int do_physics = 0;
int do_assess = 0;
int do_manage = 0;
int do_economics = 0;
int do_CLAMLinkage = 0;
int not_tracking_flux;
int do_BrokerLinkage = 0;	/* Link with the broker */


FunctionalGroupStruct *FunctGroupArray;
DemographicStruct *EMBRYO;
MigrationStruct *MIGRATION;
EvolutionStruct *DNA;
CoralStruct *CORALREEF;
PhysioChemStruct *PhysioChemArray;
FisheryStruct *FisheryArray;
AssessProjectionStruct *ASSESS_PROJECTION;

int it_count, waterboundary;
int sp;

double **dvol, ***dtr;
double **CatchSum;
int **FisherySpeciesCatchFlags; /* Array to hold a flag for each fishery/species combinations to indicate which species are fished by which fisheries */
double ****mFCchange;

int runNextTimeStep(MSEBoxModel *bm){
	int b, halt = 0;
	double dtscale = 0;
	char keystrname[BMSLEN];

	if (verbose > 1)
		printf("Call checking time\n");
    
    /* Check the time */
	Ecology_Time_Check(bm, bm->t, bm->dt, logfp);
    
     /* Print current time */
	printf("t = %.10g, hd.t = %.10g, TofY: %d, %% Done: %f\n", bm->t / 86400.0, bm->hd.t / 86400.0, bm->TofY, 100.0 * bm->t/ bm->tstop);
	if (verbose > 1)
		fprintf(logfp, "t = %.10g, hd.t = %.10g, TofY: %d\n", bm->t / 86400.0, bm->hd.t / 86400.0, bm->TofY);

	dtscale = bm->dt / 86400.0; /* So that always output in daytime (so light has non-zero entry value) */

    if (bm->t + dtscale * bm->dt / 2.0 > bm->tout) {
		if (verbose > 0)
			printf("writing general output\n");

		/* Write general data */
		writeBMphysData(bm->ncOfid, bm->ncOfdump, bm, 0);
		writeBMTracerData(bm->ncOfid, bm->ncOfdump, bm, 0);
		writeBMEpiData(bm->ncOfid, bm->ncOfdump, bm, 0);
		if(bm->ice_on){
			writeBMIceData(bm->ncOfid, bm->ncOfdump, bm, 0);
		}
		if(bm->terrestrial_on){
			writeBMLandData(bm->ncOfid, bm->ncOfdump, bm, 0);
		}
        
		ncsync(bm->ncOfid);
		bm->ncOfdump++;
        
		/* Write summary data */
		writeBMSummaryData(bm->ncOsumfid, bm->ncOsumdump, bm);
		writeBMSummaryEpiData(bm->ncOsumfid, bm->ncOsumdump, bm);
		ncsync(bm->ncOsumfid);
		bm->ncOsumdump++;
        
		/* Write production/consumption data */
		writeBMphysData(bm->ncOpcfid, bm->ncOpcdump, bm, 2);
		writeBMDiagData(bm->ncOpcfid, bm->ncOpcdump, bm, 2);
		ncsync(bm->ncOpcfid);
		bm->ncOpcdump++;
        
        /* Write annual age structured output - if required */
        if (bm->flag_age_output > 1) {
            writeBMphysData(bm->ncOaafid, bm->ncOaadump, bm, 1);
            writeBMAnnAgeBioData(bm->ncOaafid, bm->ncOaadump, bm);
            bm->ncOaadump++;

        }
        
        /* Write spatial diet information - if required
        if (bm->flagdietcheck && (bm->which_check <= bm->K_num_tot_sp)) {
            writeBMDietData(bm->ncOdietfid, bm->ncOdietdump, bm);   // Now in txt file instead printed out in atbiolmessage.c
        }
        */

		/* Increment output time */
		while (bm->tout < bm->t + bm->dt / 2.0)
			bm->tout += bm->toutinc;
	}
    
    /* Write fisheries output if necessary */
	if (bm->fishout && (bm->t + bm->dt / 2.0 > bm->tfishout)) {
		if (verbose > 0)
			printf("writing fisheries output\n");

		/* Write totals */
		writeBMphysData(bm->ncOfishfid, bm->ncOfishdump, bm, 1);
		writeBMFisheriesData(bm->ncOfishfid, bm->ncOfishdump, bm, 1);
		ncsync(bm->ncOfishfid);
		bm->ncOfishdump++;
        
        /* Write details */
		writeBMphysData(bm->ncOdetfishfid, bm->ncOdetfishdump, bm, 3);
        
		writeBMFisheriesData(bm->ncOdetfishfid, bm->ncOdetfishdump, bm, 3);
        
		ncsync(bm->ncOdetfishfid);
		bm->ncOdetfishdump++;
        
        /* Write annual age structured output - if required */
        if (bm->flag_age_output > 1) {
            writeBMphysData(bm->ncOaacfid, bm->ncOaacdump, bm, 1);
            writeBMAnnAgeCatData(bm->ncOaacfid, bm->ncOaacdump, bm);
            bm->ncOaacdump++;

        }

        Harvest_Refresh_Fishing_Stats(bm);

		/* Increment output time */
		while (bm->tfishout < bm->t + bm->dt / 2.0)
			bm->tfishout += bm->toutfinc;
	}
    
    /* Copy MSEBoxModel values to new values - I can use memcpy
	 * here as the arrays are allocated with my routines which
	 * guarantee contiguous memory
	 */
	memcpy(newwctr[0][0], bm->wctr[0][0], sizeof(double) * (long unsigned int)bm->nbox * (long unsigned int)bm->wcnz * (long unsigned int)bm->ntracer);
	memcpy(newsedtr[0][0], bm->sedtr[0][0], sizeof(double) * (long unsigned int)bm->nbox * (long unsigned int)bm->sednz * (long unsigned int)bm->ntracer);
	if(bm->ice_on){
		memcpy(newicetr[0][0], bm->icetr[0][0], sizeof(double) * (long unsigned int)bm->nbox * (long unsigned int)bm->icenz * (long unsigned int)bm->ntracer);
	}
	if(bm->terrestrial_on){
		memcpy(newlandtr[0], bm->landtr[0], sizeof(double) * (long unsigned int)bm->nbox * (long unsigned int)bm->nland);
	}

    
    /* Calculate stock values for each group */
	Util_Calculate_StockID(bm);
    
    if (verbose > 1)
		printf("Call doing assessment \n");

	/* Do assessments if required */
	if (do_assess) {
		if ((bm->TofY == 0) && bm->flagday)
			Assess_Annual_Schedule(bm, logfp);
		Assess_Resources(bm, logfp);
        
#ifdef RASSESS_LINK_ENABLED
        if(bm->UsingRAssess == 1) {
            if (((bm->TofY == bm->RAssessSpringSurveyDay) || (bm->TofY == bm->RAssessAutumnSurveyDay)) && bm->flagday) {
                RAssessSurvey(bm, logfp);
                
                /*
                for (sp = 0; sp < bm->K_num_tot_sp; sp++) {
                    if (FunctGroupArray[sp].isTAC > 1) {
                        Do_RAssess(bm, sp, 0, bm->logFile);
                    }
                }
                
                quit("Test done\n");
                */
            }
        }
#endif
	}
    
    /* Annual fisheries management decisions (TACS and seasonal closure dates)
	 - has to occur after migrations as that's when total biomasses calculated
	 when using perfect knowledge managers */
	if (verbose > 1)
		printf("Annual management and economics \n");

	if ((bm->TofY == 0) && bm->flagday) {
        bm->thisyear++;

		if (verbose > 1)
			printf("Call annual fisheries, management and economics related things (if appropriate)\n");

		/* Output any natural mortality estimates before used and
		 reset in assessment cycle of Annual_Fisheries_Mgmt()
		 */
		Ecology_Output_Mort_Estimates(bm, logfp);
		Ecology_Output_Mort_Per_Pred_Estimates(bm, logfp);
        
		if (bm->flag_fisheries_on) {
			//printf("Call annual fisheries, management and economics related things (if appropriate)\n");

			Harvest_Annual_Calculations(bm, logfp);
			Manage_Annual_Calculations(bm, logfp);
			Annual_Mgmt_Implementation(bm, logfp);
            
			Update_Harvest_Index_Values(bm, logfp);
			Manage_Update_Indices(bm, logfp);

			//Annual_Fisheries_Mgmt(&bm, logfp);
			if (bm->flagecon_on)
				Economic_Annual(bm, logfp);
			Harvest_Annual_Reset(bm, logfp);
		}

	}
    
    /* Calculate total effort */
	if (verbose > 1)
		printf("Call effort allocation and economics (if appropriate)\n");


    if (do_manage && bm->flag_fisheries_on) {
		/* Reset bimonthly effort if need be */
		if (bm->Council_needed)
			Manage_Visit_Council(bm, logfp);

		if (bm->flagecon_on) {
			if (verbose > 1)
				printf("Call economics\n");
			Economics(bm, logfp);
		}
        
        if (bm->flagdisplace) { // Record displaced effort if using that effort allocation option
            if(bm->newmonth && bm->flagday) {
                Harvest_Report_Monthly_Stats(bm, logfp);
            }
        }
        
		Manage_Calculate_Total_Effort(bm, logfp);

		/* To increase speed in debugging use this to provide fisheries with catches
		 while skipping biology.
		 */
		if (!do_biology) {
			Harvest_Skip_biology(bm, logfp);
		}
	}
    
    if (do_biology) {

        /* Annual biological events such as spawning and migration dates set */
        if ((bm->TofY == 0) && bm->flagday) {

			if (verbose > 1)
				printf("Call annual biological activities");

			Ecology_Annual(bm, logfp);
		}
        
        if (verbose > 1)
			printf("Call update of cohorts\n");

        /* Do any maturity updating */
		Ecology_Update_Invert_Cohorts(bm, logfp);
        Ecology_Update_Vertebrate_Cohorts(bm, logfp);
        
        /* Update any scaled values such as growth rates */
		Ecology_Update_Scaled_Values(bm);
        
        if (verbose > 1)
            printf("Call migration of vertebrates\n");

        /* Calculate total vertebrates for the area and perform migrations */
        Ecology_Update_Move_Entry(bm, logfp);  // Not in fishmove test loop as might have inverts being updated
        if (bm->fishmove) {
            Ecology_Total_Verts_And_Migration(bm, bm->dt, logfp);
        }
        
		Ecology_Invading_Species(bm, bm->dt, logfp);
        
        if (fishtest) {
			Ecology_Test_Fish_Total(bm, newwctr, newlandtr, use_bm, "After vertebrate movement", logfp);
		}

		Ecology_Calculate_Total_Abundance(bm, bm->dt, 0, logfp);
        
		if (verbose > 1)
			printf("Call migration of invertebrates\n");

        /* Perform invertebrate migrations */
		Ecology_Invert_Migration(bm, bm->dt, logfp);

        if (verbose > 1) {
            printf("Call ecology\n");
            fflush(stdout);
            fflush(stderr);
        }
        
        /* Do biological processes - step through biology for each box */
		for (b = 0; b < bm->nbox; b++) {

            if(bm->boxes[b].type == LAND){

				/* Do the land stuff */
				/* Just want to allow groups that are present in the land to reproduce */

				Ecology_Land_Biology_Process(bm, &bm->boxes[b]);

			} else if (bm->boxes[b].type != BOUNDARY) {

				/* Update overlap between group and fisheries distributions */
				if (bm->flag_fisheries_on ) {
					Harvest_Update_Habitat_Overlap(bm, bm->boxes[b].n);
				}

				Ecology_Box_Biology(bm, &bm->boxes[b], bm->dt, logfp);

				/* Check fish abundance */
				if (fishtest && bm->checkbox) {
					sprintf(keystrname, "After box %d ecology", b);
					Ecology_Test_Fish_Total(bm, newwctr, newlandtr, use_tr, keystrname, logfp);
				}
            } else {
                /** Call external vertebrates code by Marie Savina **/

                /* The following will only be true if the bm->external_box value is set to true. Otherwse there is no way the
                 * boundarytype could be anything other than NORMAL_BOUND
                 *
                 * Only execute once so only when b = 0
                 */
                if(bm->external_populations && !b) {
                    External_Box_Ecology(bm, b, bm->dt, bm->logFile);
                }
            }
		}
        
		Ecology_Starve_Notice(bm, logfp);

		Harvest_Update_Temp_Catch_Array(bm, logfp);
	}
    
    /*
	 check_vals(&bm,newwctr,bm->wcnz,"water, after biology\n");
	 check_vals(&bm,newsedtr,bm->sednz,"sediment, after biology\n");
	 */
    
    if (verbose > 1)
		printf("Call physics\n");

	/* Step physics */
    if (do_physics) {
        physics(bm, newwctr, newsedtr, logfp);
		//physics(bm, newwctr, newsedtr, newicetr, newlandtr, logfp);
    }
    
    #ifdef CLAM_LINK_ENABLED
	//if(do_CLAMLinkage){
	//    CLAM_Check_Indicators(&bm);
	//}
	#endif
	/*
	 check_vals(&bm,newwctr,bm->wcnz,"water, after physics\n");
	 check_vals(&bm,newsedtr,bm->sednz,"sediment, after physics\n");
	 */

	if (verbose > 1)
		printf("Call dealing with boundaries\n");

	/* Boundary stuff */
	boundaries(bm, newwctr, newsedtr, newicetr, newlandtr, logfp);
    
	//fprintf(bm->logFile, "end of boundaries - Arsenic in wc 1:0 = %e\n", newwctr[1][0][1499]);
	//fprintf(bm->logFile, "end of boundaries - Arsenic in sed 1:0 = %e\n", newsedtr[1][0][1499]);

	/* Copy new values to MSEBoxModel */
	memcpy(bm->wctr[0][0], newwctr[0][0], sizeof(double) * (long unsigned int)bm->nbox * (long unsigned int)bm->wcnz * (long unsigned int)bm->ntracer);
	memcpy(bm->sedtr[0][0], newsedtr[0][0], sizeof(double) * (long unsigned int)bm->nbox * (long unsigned int)bm->sednz * (long unsigned int)bm->ntracer);

	if(bm->ice_on == TRUE){
		memcpy(bm->icetr[0][0], newicetr[0][0], sizeof(double) * (long unsigned int)bm->nbox * (long unsigned int)bm->icenz * (long unsigned int)bm->ntracer);
	}
    
//	if(bm->terrestrial_on){
//		memcpy(bm->landtr[0], newlandtr[0], sizeof(double) * bm->nbox * bm->nland);
//	}

	/* Perform any necessary vertical geometry adjustments */
	vertgeom(bm, logfp);

	/* Calculate updated sediment properties */
	sedprops(bm);
    
    /* Check contaminant balance */
    if(bm->track_contaminants && bm->flag_contam_sanity_check){
        Check_Contam_Totals(bm);
    }

    fflush(stdout);
    fflush(stderr);

    /* Update time and step number */
	bm->t += bm->dt;
	bm->nt++;
    
	/* Check to see if can continue run or need to come to premature stop.
	To do this check to see if can open dummy halt file in current directory */
	if( (haltfp=Util_fopen(bm, "delete_to_halt_run","r")) == NULL ){
		printf("warning: Can't open delete_to_halt_run file so ending the run\n");
		halt = 1;
	} else
		fclose(haltfp);

	logfp = checkLogFileSize(logfp, bm);
	/* Only check netcdf file size every 5 days */
	if(bm->TofY % 5 == 0)
		checknetCDFFiles(bm);
	return halt;
}
/****************************************************************************
 Main run routine


 At the start of each time step the tracers are copied into a temporary array called newwctr.
 The values


 */

int main(int argc, char *argv[]) {
	MSEBoxModel bm;
	//double dtscale = 0;
	//int b, halt;
	//char keystrname[BMSLEN];
	int halt;

	/* Set up the model */
	setupMSEBoxModel(argc, argv, &bm);

	if (logfp == NULL) {
		quit("logfp is null\n");
	}

#if 0
	/* Set signal to allow clean up on SIGTERM */
	signal(SIGTERM,setkill);
#endif

	/* Allocate memory for new values at each time step */
	/* Need to keep these */
	newwctr = (double ***) alloc3d(bm.ntracer, bm.wcnz, bm.nbox);
	newsedtr = (double ***) alloc3d(bm.ntracer, bm.sednz, bm.nbox);
	if(bm.ice_on){
		newicetr = (double ***) alloc3d(bm.ntracer, bm.icenz, bm.nbox);
	}
	if(bm.terrestrial_on){
		newlandtr = (double **) alloc2d(bm.nland, bm.nbox);
	}

	/* Store model start time */
	bm.tstart = bm.t;
	bm.nt = 0;
    bm.dtsz_stored = bm.dt;
    
#ifdef RASSESS_LINK_ENABLED
    
    if(bm.usingRedus_R_HCR){
        Redus_Linkage_Start(&bm);
    }
    
    if(bm.UsingRAssess){
        RRAssess_Linkage_Start(&bm);
    }
    
#endif
    
    /* First check to see if can continue run or need to come to premature stop.
	 To do this check to see if can open dummy halt file in current directory */
	if ((haltfp = Util_fopen(&bm, "delete_to_halt_run", "r")) == NULL) {
		printf("warning: Can't open delete_to_halt_run file so ending the run\n");
		halt = 1;
	} else {
		halt = 0;
		fclose(haltfp);
	}
	if(do_BrokerLinkage){
#ifdef BROKER_LINK_ENABLED
		/* If we are linking to the broker then we wait for the broker to tell us to start */
		Linkage_Start(&bm);
#else
		quit("You are trying to link to the broker but you must compile the code with the '--with-brokerlink' flag\n");
#endif
	} else {
		/* Loop for each time step */
		while (!killed && (bm.t < bm.tstop) && !halt) {
			halt = runNextTimeStep(&bm);
		}
	}

	/* Write final output dump and close the ouput files */
	writeBMphysData(bm.ncOfid, bm.ncOfdump, &bm, 0);
	writeBMTracerData(bm.ncOfid, bm.ncOfdump, &bm, 0);
	if(bm.ice_on){
		writeBMIceData(bm.ncOfid, bm.ncOfdump, &bm, 0);
	}
	writeBMEpiData(bm.ncOfid, bm.ncOfdump, &bm, 0);

	if(bm.terrestrial_on){
		writeBMLandData(bm.ncOfid, bm.ncOfdump, &bm, 0);
	}

	ncclose(bm.ncOfid);
	writeBMSummaryData(bm.ncOsumfid, bm.ncOsumdump, &bm);
	writeBMSummaryEpiData(bm.ncOsumfid, bm.ncOsumdump, &bm);
	ncclose(bm.ncOsumfid);
	writeBMphysData(bm.ncOpcfid, bm.ncOpcdump, &bm, 2);
	writeBMDiagData(bm.ncOpcfid, bm.ncOpcdump, &bm, 2);
	ncclose(bm.ncOpcfid);

    if(bm.flag_age_output > 1) {
        writeBMphysData(bm.ncOaafid, bm.ncOaadump, &bm, 1);
        writeBMAnnAgeBioData(bm.ncOaafid, bm.ncOaadump, &bm);
        ncclose(bm.ncOaafid);
    }

    /*
    if (bm.flagdietcheck && (bm.which_check <= bm.K_num_tot_sp)) {
        writeBMDietData(bm.ncOdietfid, bm.ncOdietdump, &bm);   // Now in txt file instead printed out in atbiolmessage.c
        ncclose(bm.ncOdietfid);
    }
    */

	if (bm.fishout) {

		writeBMphysData(bm.ncOfishfid, bm.ncOfishdump, &bm, 1);
		writeBMFisheriesData(bm.ncOfishfid, bm.ncOfishdump, &bm, 1);
		ncclose(bm.ncOfishfid);
		writeBMphysData(bm.ncOdetfishfid, bm.ncOdetfishdump, &bm, 3);
		writeBMFisheriesData(bm.ncOdetfishfid, bm.ncOdetfishdump, &bm, 3);
		ncclose(bm.ncOdetfishfid);

        if(bm.flag_age_output > 1) {
            writeBMphysData(bm.ncOaacfid, bm.ncOaacdump, &bm, 1);
            writeBMAnnAgeCatData(bm.ncOaacfid, bm.ncOaacdump, &bm);
            ncclose(bm.ncOaacfid);
        }

	}
    
    /* Close any R links */
#ifdef RASSESS_LINK_ENABLED
    if(bm.usingRedus_R_HCR){
        freeRedus();
    }
#endif
    if(bm.UsingRAssess){
#ifdef RASSESS_LINK_ENABLED
        // Close R link (if using embedded R)
        if(!bm.RAssessRuseScript) {
            freeRRAssess();
        }
        
        // Clean up associated filename arrays
        i_free1d(bm.RAssess_initiated);
        free(bm.RAssessFnames);
#endif
        c_free2d(bm.RAssessRscriptName);
    }

	/* Write out final comments to log and text files (so have summary of system state) */
	Textfile_Dump(&bm, logfp);

	/* Close the log file */
	fclose(logfp);

	logfp = 0;

	/* Shutdown model */
	modelshutdown(&bm);
	exit(0);
}

/*************************************************************************************
 Basic model initialisation
 */
void setupMSEBoxModel(int argc, char *argv[], MSEBoxModel *bm) {
	double cellvol, ask_t;
	int ij, traceid, den, rn, sn, fgIndex, i;
	int fid, fid2 = -1, fid3, fid4, fid5  =-1, fid6 = -1, fid7 = -1, b, k, totname, totnz;
    //int fid8 = -1;
	long n = 0;
	char *progname = argv[0];
	char endname[] = ".nc";
	char *pdest;
	char tempStr[STRLEN];
	char commands[STRLEN];
	char commandTempStr[STRLEN];

	logfp = NULL;
	haltfp = NULL;

#ifdef _STUDIO
	_CrtSetDbgFlag(_CRTDBG_CHECK_ALWAYS_DF);
#endif

	/* Clear the box model - this assumes that it doesn't
	 * already contain valid data. If so, allocated memory should
	 * be freed first by whoever is calling this routine.
	 */
	memset(bm, 0, sizeof(MSEBoxModel));
	bm->ncIfdump = -1;
    bm->PGMSY_on = FALSE;


	/* Process arguments - where specified output filename used as basis for all output netcdf file names */
	if (argc < 16){
		if(argc == 2){
			quit("Atlantis version %s\n", ATLANTIS_REVISION);
		}
		Util_Usage(1);

	}

	/* Prep the command line string to write out to the log file once its created */
	(void) sprintf(commands, "%s", argv[0]);
	for (i = 1; i < argc-1; i++) {
		sprintf(commandTempStr, " %s", argv[i]);
		strcat(commands, commandTempStr);
		//(void) sprintf(commands, "%s %s", commands,
	}
	sprintf(commandTempStr, " %s", argv[argc-1]);
	strcat(commands, commandTempStr);

	//(void) sprintf(commands, "%s %s",commands, argv[argc-1]);

	strcpy(bm->destFolder, "");
	strcpy(bm->inputFolder, "");
	while (--argc > 0) {
		if ((*++argv)[0] == '-') {
			switch ((*argv)[1]) {
			case 'i': // Input name
				if (argc < 3)
					Util_Usage(1);
				strncpy(bm->ncIfname, *++argv, BMSLEN);
				bm->ncIfdump = atoi(*++argv);
				argc -= 2;
				break;
			case 'o': // Output name
				if (argc < 2)
					Util_Usage(1);
				strncpy(bm->ncOfname, *++argv, BMSLEN);
				argc--;
				break;
			case 'r': // Run parameter file
				if (argc < 2)
					Util_Usage(1);
				strncpy(bm->runprmIfname, *++argv, BMSLEN);
				argc--;
				break;
			case 'f': // Forcing parameter file
				if (argc < 2)
					Util_Usage(1);
				strncpy(bm->forceIfname, *++argv, BMSLEN);
				argc--;
				break;
			case 'p': // Physics parameter file
				if (argc < 2)
					Util_Usage(1);
				strncpy(bm->physprmIfname, *++argv, BMSLEN);
				argc--;
				do_physics = 1;
				break;
			case 'b': // Biology parameter file
				if (argc < 2)
					Util_Usage(1);
				strncpy(bm->bioprmIfname, *++argv, BMSLEN);
				argc--;
				do_biology = 1;
				break;
			case 'h': // Harvest parameter file
				if (argc < 2)
					Util_Usage(1);
				strncpy(bm->fishprmIfname, *++argv, BMSLEN);
				argc--;
				do_biology = 1;
				do_manage = 1;
				break;
			case 'a': // Assessment parameter file
				if (argc < 2)
					Util_Usage(1);
				strncpy(bm->assessprmIfname, *++argv, BMSLEN);
				argc--;
				do_assess = 1;
				break;
			case 'e': // Economics parameter file
				if (argc < 2)
					Util_Usage(1);
				strncpy(bm->econprmIfname, *++argv, BMSLEN);
				argc--;
				do_economics = 1;
				break;
			case 'c':
				if (argc < 2)
					Util_Usage(1);
				strncpy(bm->cabspot, *++argv, BMSLEN);
				argc--;
				break;
			case 's': // Species config csv file
				if (argc < 2)
					Util_Usage(1);
				strncpy(bm->functionGroupIfname, *++argv, BMSLEN);
				argc--;
				break;

			case 'q': // Fisheries config csv file
				if (argc < 2)
					Util_Usage(1);
				strncpy(bm->fisheryIfname, *++argv, BMSLEN);
				argc--;
				break;
            case 'm':
                if (argc < 2)
                    Util_Usage(1);
                strncpy(bm->migrationIfname, *++argv, BMSLEN);
                argc--;
                do_migration = 1;
                break;
            case 'w':
				if (argc < 2)
					Util_Usage(1);
				do_CLAMLinkage = 1;
				strncpy(bm->CLAMLinkageprmIfname, *++argv, BMSLEN);
				argc--;
				break;
			case 'l':  // Broker config file
				if (argc < 2)
					Util_Usage(1);
				do_BrokerLinkage = 1;
				strncpy(bm->brokerLinkageprmIfname, *++argv, BMSLEN);
				argc--;
				break;
			case 'd': /* Destination folder */
				if( argc < 2 ) Util_Usage(1);
				strncpy(bm->destFolder, *++argv,BMSLEN);
				trim(bm->destFolder);
				strcat(bm->destFolder, FOLDER_SEP);

				/* Make the output folder */
				sprintf(tempStr, "mkdir %s", bm->destFolder);
				system(tempStr);

				argc--;
				break;
			case 't': /* Input Path folder */
				if( argc < 2 ) Util_Usage(1);
				strncpy(bm->inputFolder, *++argv,BMSLEN);
				trim(bm->inputFolder);
				strcat(bm->inputFolder, FOLDER_SEP);

				argc--;
				break;
			default:
				Util_Usage(1);
				break;
			}
		} else {
			Util_Usage(1);
		}
	}

	if (strcmp(bm->functionGroupIfname, "") == 0) {
		Util_Usage(1);
	}

	if (strlen(bm->forceIfname) == 0) {
		Util_Usage(1);
	}
	if (strlen(bm->ncOfname) == 0) {
		Util_Usage(1);
	}

	/* Get core part of output netcdf filenames */
	pdest = strstr(bm->ncOfname, endname);
	totname = (int) strlen(bm->ncOfname) - (int) strlen(pdest);

	// Create a string that stores the filename without the .nc part.
	strcpy(tempStr, bm->ncOfname);
	pdest = strstr(tempStr, endname);
	*pdest = '\0';

	strncpy(bm->ncOSumfname, bm->ncOfname, (size_t)totname);
	sprintf(bm->startfname, "%s", bm->ncOSumfname);
	//sprintf(bm->ncOSumfname, "%sTOT.nc", bm->ncOSumfname);
	strcat(bm->ncOSumfname, "TOT.nc");

	strncpy(bm->ncOPCfname, bm->ncOfname, (size_t)totname);
	//sprintf(bm->ncOPCfname, "%sPROD.nc", bm->ncOPCfname);
	strcat(bm->ncOPCfname, "PROD.nc");


	strncpy(bm->ncOFishfname, bm->ncOfname, (size_t)totname);
	//sprintf(bm->ncOFishfname, "%sTOTCATCH.nc", bm->ncOFishfname);
	strcat(bm->ncOFishfname, "TOTCATCH.nc");


	strncpy(bm->ncODetFishfname, bm->ncOfname, (size_t)totname);
    //sprintf(bm->ncODetFishfname, "%sCATCH.nc", bm->ncODetFishfname);
    strcat(bm->ncODetFishfname, "CATCH.nc");

    strncpy(bm->ncOAAfname, bm->ncOfname, (size_t)totname);
    //sprintf(bm->ncOAAfname, "%sANNAGEBIO.nc", bm->ncOAAfname);
    strcat(bm->ncOAAfname, "ANNAGEBIO.nc");

    strncpy(bm->ncOAACfname, bm->ncOfname, (size_t)totname);
    //sprintf(bm->ncOAACfname, "%sANNAGECATCH.nc", bm->ncOAACfname);
    strcat(bm->ncOAACfname, "ANNAGECATCH.nc");

    //strncpy(bm->ncODIETfname, bm->ncOfname, (size_t)totname);
    //sprintf(bm->ncODIETfname, "%sDIET.nc", bm->ncODIETfname);

	/* Check that all required arguments were present */
	if (!bm->ncIfname[0] || !bm->ncOfname[0] || !bm->runprmIfname[0] || !bm->forceIfname || !bm->fishprmIfname || !bm->assessprmIfname || !bm->econprmIfname
			|| !bm->ncODetFishfname[0] || !bm->ncOFishfname[0] || !bm->ncOSumfname[0] || !bm->ncOPCfname[0] || bm->ncIfdump < 0)
		Util_Usage(1);

	/* Write arguments to parameter string */
	sprintf(bm->params, "%s -i %s %d -o %s -s %s -g %s -c %s -d %s -r %s -f %s", progname, bm->ncIfname, bm->ncIfdump, bm->ncOfname, bm->ncOSumfname,
			bm->ncOPCfname, bm->ncOFishfname, bm->ncODetFishfname, bm->runprmIfname, bm->forceIfname);
	if (do_biology) {
		char s[STRLEN];
		sprintf(s, " -b %s", bm->bioprmIfname);
		strcat(bm->params, s);
		sprintf(s, " -h %s", bm->fishprmIfname);
		strcat(bm->params, s);
        
        if (do_migration) {
            sprintf(s, " -m %s", bm->migrationIfname);
            strcat(bm->params, s);
        }
        
		if (do_assess) {
			char sa[STRLEN];
			sprintf(sa, " -a %s", bm->assessprmIfname);
			strcat(bm->params, sa);
		}
		if (do_economics) {
			char sa[STRLEN];
			sprintf(sa, " -e %s", bm->econprmIfname);
			strcat(bm->params, sa);
		}
	}

	if (do_physics) {
		char s[STRLEN];
		sprintf(s, " -p %s", bm->physprmIfname);
		strcat(bm->params, s);
	}

	/* Create log file */
	if (!logfp)
		logfp = initLogFile(bm);

	fprintf(logfp, "%s\n",commands);

	/* Create halt file */
	initHaltFile(bm);

	/* Set up the species parameter strings */
	Util_Setup_Species_Param_Strings(bm);

	set_keyprm_errfn(quit);

	if(Util_Read_Run_XML(bm, bm->runprmIfname) == FALSE){
		quit("setupMSEBoxModel: Failed to load the model run parameter file %s\n", bm->runprmIfname);
	}

	/* Get Physiochemical properties */
	Ecology_Assign_Physio_Chem(bm, logfp);

	ncopts = NC_VERBOSE;

	/* Open input netCDF file */
    if(verbose) {
        printf("Open input file\n");
    }

	fid = Util_ncopen(bm->inputFolder, bm->ncIfname, NC_NOWRITE);
	if(fid < 0)
		quit("setupMSEBoxModel: Can't open initial conditions model input data file %s\n",bm->ncIfname);

	/* Read the model geometry */
	ncattget(fid, NC_GLOBAL, "geometry", bm->geomIfname);

    if(verbose) {
        printf("Read geometry file name\n");
    }

	readMSEBoxModelGeom(bm->geomIfname, bm);
	/* Echo geometry to stdout if verbose */
    if (verbose > 1) {
		writeMSEBoxModelGeom(stdout, bm);
    }

    if(verbose) {
        printf("Read box geometry\n");
    }

	/* Read the model variable info. Physical info must be first,
	 * as it reads the number of water column and sediment layers
	 * from the input netCDF file
	 */
	readBMphysInfo(fid, bm);
	readBMTracerInfo(fid,  bm->ncIfname, bm);
	readBMEpiInfo(fid, bm);
    if(bm->ice_on) {
		readBMIceInfo(fid,  bm->ncIfname, bm);
    }

    if(verbose) {
        printf("Read info\n");
    }

    if(bm->terrestrial_on) {
		readBMLandInfo(fid, bm);
    }

	/* Read in the functional group parameters. Need to have read in the number of layers and boxes to allocate arrays
	 * of the correct size.
	 */
	bm->flagOverwinter = 0;
	if (Util_Read_Functional_Group_XML(bm, bm->functionGroupIfname, logfp) == FALSE)
		quit("Failed to find functional group file\n");

    if(verbose) {
        printf("Read Functional Group XML\n");
    }

	/* Allocate memory for variables. Note that the casts (double ***)
	 * are only needed to supress compiler warnings. Note also that the
	 * allocation routines used here have parameter ordering opposite
	 * to the way that the arrays are accessed. Thus if we want a
	 * 3d array X, accessed X[k][j][i] (i varies fastest), then we say
	 * X = ...alloc3d(ni,nj,nk), where ni is the number of elements
	 * in the i dimension etc.
	 */
    if(verbose) {
        printf("Allocate arrays\n");
    }

	bm->nom_dz = (double **) alloc2d(bm->wcnz + bm->sednz, bm->nbox);
	bm->dz = (double **) alloc2d(bm->wcnz + bm->sednz, bm->nbox);
	bm->wctr = (double ***) alloc3d(bm->ntracer, bm->wcnz, bm->nbox);
	bm->sedtr = (double ***) alloc3d(bm->ntracer, bm->sednz, bm->nbox);
	if(bm->ice_on){
		bm->icedz = (double **) alloc2d(bm->icenz, bm->nbox);
		bm->icetr = (double ***) alloc3d(bm->ntracer, bm->icenz, bm->nbox);
	}
	bm->epi = (double **) alloc2d(bm->nepi, bm->nbox);

	if(bm->terrestrial_on){
		bm->landtr = (double **) alloc2d(bm->nland, bm->nbox);
	}

    if (bm->flagdietcheck) {
        // If age structured diets required
        bm->totDiet = (double *****) alloc5d(bm->K_num_tot_sp, bm->K_num_max_cohort, bm->K_num_tot_sp, bm->wcnz+bm->sednz, bm->nbox);
    }

    bm->init_pH = (double **) alloc2d(bm->wcnz + bm->sednz, bm->nbox);
	bm->vol = (double **) alloc2d(bm->wcnz + bm->sednz, bm->nbox);
	bm->por = (double **) alloc2d(bm->wcnz + bm->sednz, bm->nbox);
	bm->hddrain = (double **) alloc2d(bm->wcnz + bm->sednz, bm->nbox);
	bm->hdpool = (double **) alloc2d(bm->wcnz + bm->sednz, bm->nbox);
	bm->efluxes = (double **) alloc2d(bm->wcnz + bm->sednz, bm->nbox);
	bm->vfluxes = (double **) alloc2d(bm->wcnz + bm->sednz, bm->nbox);
	bm->vvdistrib = (double ****) alloc4d(bm->wcnz, bm->K_num_max_stages, bm->K_num_tot_sp, bm->nbox);

	/* Set up pointers to data from each box */
	for (b = 0; b < bm->nbox; b++) {
		bm->boxes[b].tr = bm->wctr[b];
		bm->boxes[b].sm.tr = bm->sedtr[b];
		bm->boxes[b].epi = bm->epi[b];
		if(bm->terrestrial_on)
			bm->boxes[b].land.tr = bm->landtr[b];

		bm->boxes[b].volume = bm->vol[b];
		bm->boxes[b].sm.volume = bm->vol[b] + bm->wcnz;
		bm->boxes[b].dz = bm->dz[b];
		bm->boxes[b].sm.dz = bm->dz[b] + bm->wcnz;
		bm->boxes[b].hdsource = bm->hddrain[b];
		bm->boxes[b].sm.hdsource = bm->hddrain[b] + bm->wcnz;
		bm->boxes[b].hdsink = bm->hdpool[b];
		bm->boxes[b].sm.hdsink = bm->hdpool[b] + bm->wcnz;
		bm->boxes[b].eflux = bm->efluxes[b];
		bm->boxes[b].vflux = bm->vfluxes[b];
		bm->boxes[b].sm.eflux = bm->efluxes[b] + bm->wcnz;
		bm->boxes[b].sm.vflux = bm->vfluxes[b] + bm->wcnz;
		bm->boxes[b].vert_vdistrib = bm->vvdistrib[b];
		for (k = 0; k < bm->wcnz; k++) {
			/* Set water column porosity values to 1 */
			bm->por[b][k] = 1.0;
		}
		/* Set sediment porosity */
		k = bm->boxes[b].sm.topk;
		bm->boxes[b].sm.porosity = bm->por[b] + bm->wcnz;

        /* Set up flux counters */
        bm->boxes[b].flush_in = Util_Alloc_Init_1D_Double(bm->wcnz, 0.0);
        bm->boxes[b].flush_out = Util_Alloc_Init_1D_Double(bm->wcnz, 0.0);


        if(bm->ice_on){
    		bm->boxes[b].ice.dz = bm->icedz[b];
    		bm->boxes[b].ice.tr = bm->icetr[b];
        }

    }
	totnz = bm->wcnz + bm->sednz;

    if(verbose) {
        printf("Finished allocating bm arrays\n");
    }

	/* Read data */
	readBMphysData(fid, bm->ncIfdump, bm, logfp);
	readBMTracerData(fid, bm->ncIfdump, bm);
	readBMEpiData(fid, bm->ncIfdump, bm);
	if(bm->ice_on){
		readBMIceData(fid, bm->ncIfdump, bm);
	}
	if(bm->terrestrial_on){
		readBMLandData(fid, bm->ncIfdump, bm);
	}
	/* Close input netCDF file */
	ncclose(fid);

    /* Initialise vert_vdistrib */
	set_up_vvdistrib(bm);

	/* Allocate the harvest FStat tracer arrays */
	Harvest_Allocate_FStat_Arrays(bm);

	/* Allocate the diagnostic arrays */
	Ecology_Allocate_Diag_Arrays(bm);

	/* Set output start time */
	bm->tout = bm->toutstart;
	bm->tfishout = bm->toutstart;

	/* Set starting conditions */
	bm->dayt = 0.0;
	bm->predayt = -1.0;
	bm->current_box = 0;
	bm->current_layer = 0;

	/* Test to see if can open output files */
	/* Open the general tracer output file */
	printf("Creating general output file\n");
	ncopts = 0;
	fid =  Util_ncopen(bm->destFolder, bm->ncOfname, NC_WRITE);
	if (fid == -1) {
		/* File doesn't exist so will have to create it later */
	} else if (!bm->flagreusefile) {
		/* File does exist and don't want to replace or re-use it */
		quit("file with name %s already exists, either delete, rename or set reuse param in run.prm to 1\n", bm->ncOfname);
	}

	/* Open the fisheries output file */
	if (bm->fishout) {
		printf("Create fisheries output file\n");

		/* Overall fisheries data */
		ncopts = 0;
		fid2 =  Util_ncopen(bm->destFolder, bm->ncOFishfname, NC_WRITE);
		if (fid2 == -1) {
			/* File doesn't exist so will have to create it later */
		} else if (!bm->flagreusefile) {
			/* File does exist and don't want to replace or re-use it */
			quit("file with name %s already exists, either delete, rename or set reuse param in run.prm to 1\n", bm->ncOFishfname);
		}

		/* Detailed fisheries output file */
		ncopts = 0;
		fid5 =  Util_ncopen(bm->destFolder, bm->ncODetFishfname, NC_WRITE);
		if (fid5 == -1) {
			/* File doesn't exist so will have to create it later */
		} else if (!bm->flagreusefile) {
			/* File does exist and don't want to replace or re-use it */
			quit("file with name %s already exists, either delete, rename or set reuse param in run.prm to 1\n", bm->ncODetFishfname);
		}
	}

	/* Open the summary output file */
	printf("Check summary output file %s\n", bm->ncOSumfname);
	ncopts = 0;
	fid3 =  Util_ncopen(bm->destFolder, bm->ncOSumfname, NC_WRITE);
	if (fid3 == -1) {
		/* File doesn't exist so will have to create it later */
	} else if (!bm->flagreusefile) {
		/* File does exist and don't want to replace or re-use it */
		quit("file with name %s already exists, either delete, rename or set reuse param in run.prm to 1\n", bm->ncOSumfname);
	}

	/* Open the growth/consumption output file */
	printf("Create growth and consumption output file\n");
	ncopts = 0;
	fid4 =  Util_ncopen(bm->destFolder, bm->ncOPCfname, NC_WRITE);
	if (fid4 == -1) {
		/* File doesn't exist so will have to create it later */
	} else if (!bm->flagreusefile) {
		/* File does exist and don't want to replace or re-use it */
		quit("file with name %s already exists, either delete, rename or set reuse param in run.prm to 1\n", bm->ncOPCfname);
	}

    /* Open annual age structured output files */
    if(bm->flag_age_output > 1) {
        printf("Create annual age structured output file\n");
        ncopts = 0;
        fid6 =  Util_ncopen(bm->destFolder, bm->ncOAAfname, NC_WRITE);
        if (fid6 == -1) {
            /* File doesn't exist so will have to create it later */
        } else if (!bm->flagreusefile) {
            /* File does exist and don't want to replace or re-use it */
            quit("file with name %s already exists, either delete, rename or set reuse param in run.prm to 1\n", bm->ncOAAfname);
        }

        if (bm->fishout) {
            printf("Create annual age structured output file\n");
            ncopts = 0;
            fid7 =  Util_ncopen(bm->destFolder, bm->ncOAACfname, NC_WRITE);
            if (fid7 == -1) {
                /* File doesn't exist so will have to create it later */
            } else if (!bm->flagreusefile) {
                /* File does exist and don't want to replace or re-use it */
                quit("file with name %s already exists, either delete, rename or set reuse param in run.prm to 1\n", bm->ncOAACfname);
            }
        }
    }

    /* Open the spatial diet output file - now relaced with text file version
    printf("Create spatial diet output file\n");
    ncopts = 0;
    fid8 =  Util_ncopen(bm->destFolder, bm->ncODIETfname, NC_WRITE);
    if (fid8 == -1) {
        // File doesn't exist so will have to create it later
    } else if (!bm->flagreusefile) {
        // File does exist and don't want to replace or re-use it
        quit("file with name %s already exists, either delete, rename or set reuse param in run.prm to 1\n", bm->ncODIETfname);
    }
     */

	/* Initialise the hydrodynamic inputs - this must be done before
	 * the source/sink inputs so that the hydrodynamic time units are known
	 */
	printf("Initialise hydrodynamics\n");

	hydro_init(bm);

    /* Create checking array */
	bm->checkedalready = (int **) i_alloc2d(totnz, bm->nbox);
	for (b = 0; b < bm->nbox; b++) {
		for (k = 0; k < totnz; k++) {
			bm->checkedalready[b][k] = 0;
		}
	}

	/* Read the water and tracer source/sink data */
	sourcesink_init(bm);

	/** Initialise the temperature and salinity inputs **/
	printf("Initialise temperature and salinity forcing\n");

	tempsalt_init(bm);
	printf("Initialise physics\n");

	/* Initialise the physics */

	//TODO: Move this all into one function.
	bm->HowFar = 0;
	bm->QofY = 0;

	if (do_physics) {
		initPhysics(bm);

		/* Set transport arrays for tracer and volume changes to zero */
		dvol = Util_Alloc_Init_2D_Double(bm->wcnz, bm->nbox, 0.0);
		dtr = Util_Alloc_Init_3D_Double(bm->ntracer, bm->wcnz, bm->nbox, 0.0);
	}

	/* Read the bottom stress data, hydrodynamic time units, from timeseries unless supplied
	 initial conditons file */
	if (!bm->supplied_stress) {

        if (bm->resuspension) {
            if(bm->ts_on_hydro_time)
                read_bm_ts(bm, "BottomStress", &bm->stress, bm->hd.t_units, "nxsbs", "1", &bm->stress_id, quit);
            else
                read_bm_ts(bm, "BottomStress", &bm->stress, bm->t_units, "nxsbs", "1", &bm->stress_id, quit);
		} else {
            if(bm->ts_on_hydro_time)
                read_bm_ts(bm, "BottomStress", &bm->stress, bm->hd.t_units, "nxsbs", "1", &bm->stress_id, warn);
            else
                read_bm_ts(bm, "BottomStress", &bm->stress, bm->t_units, "nxsbs", "1", &bm->stress_id, warn);
        }
	}
	bm->first_box = -1;
	bm->last_box = -1;
	for (b = 0; b < bm->nbox; b++) {
		/* Store initial stress values if not already loaded */

		if (!bm->supplied_stress) {
			/* FIX - if not supplied in initial conditions file then
			 need to have it so that if stress file is there than
			 read in otherwise set to zero  i.e. need to base it on
			 outcome of above call. */

            if (bm->ts_on_hydro_time)
                ask_t = bm->hd.t;
            else
                ask_t = bm->t;

			if (bm->resuspension)
				bm->boxes[b].stress = tsEvalXY(bm->stress, bm->stress_id, ask_t, bm->boxes[b].inside.x, bm->boxes[b].inside.y);
			else
				bm->boxes[b].stress = 0.0;
		}

		/* Get box id number for first non-boundasry box of the model */
		if ((bm->boxes[b].type != BOUNDARY) && (bm->first_box < 0)) {
			bm->first_box = b;
			bm->top_layer = bm->boxes[b].nz - 1;
		}

		if ((bm->boxes[b].type != BOUNDARY) && (b > bm->last_box)) {
			bm->last_box = b;
		}
	}

	/* Initialise remaining BoxModel arrays */
	AllocateArrayMemory(bm, logfp);

	/* Initialise year (set -1 here so update to 0 by first Annual_Fisheries_Mgmt) */
	bm->thisyear = -1;

    /* Initialise the biology */
	if (do_biology) {

		Ecology_Init(bm, logfp);

		/* Setup the harvest model */
		Harvest_Init(bm, logfp);

		/* Setup the management implementation model */
		Implementation_Init(bm, logfp);

        /* Initialise management model */
		Manage_Init(bm, logfp);

       if (bm->need_pops)
			Ecology_Init_Populations(bm, logfp);

        if (do_assess) {
			/* Initialise assessment model */
			Assess_Init(bm, logfp);
		}
        
        if (bm->flagecon_on) {
			/* Initialise economics model */
			Economic_Init(bm, logfp);
		}
	}
    
    /* Open the general tracer output file */
    printf("Create general output file with fid %d flagreusefile: %d\n", fid, bm->flagreusefile);
	ncopts = 0;
	if (fid == -1) {
		/* Create file */
		fid = createBMDataFile(bm->destFolder, bm->ncOfname, bm, 0);
		bm->ncOfdump = 0;
	} else if (bm->flagreusefile == 2) {
		/* Create file anew */
		ncopts = NC_CLOBBER;
		ncclose(fid);        
		fid = createBMDataFile(bm->destFolder, bm->ncOfname, bm, 0);
		bm->ncOfdump = 0;
	} else if (bm->flagreusefile == 1) {
		/* File does exist - check dimension lengths */
		ncopts = NC_VERBOSE;
		ncdiminq(fid, ncdimid(fid, "b"), NULL, &n);
		if (n != bm->nbox)
			quit("setupMSEBoxModel: general output file has wrong number of boxes\n");
		ncdiminq(fid, ncdimid(fid, "z"), NULL, &n);
		if (n != bm->wcnz + bm->sednz)
			quit("setupMSEBoxModel: general output file has wrong total number of layers\n");
		/* Check all variables are present */
		for (n = 0; n < bm->ntracer; n++) {
			if (!bm->tinfo[n].dtype) {
				if ((ncvarid(fid, bm->tinfo[n].name) < 0))
					quit("setupMSEBoxModel: general output file doesn't have %s\n", bm->tinfo[n].name);
			}
		}
		for (n = 0; n < bm->nepi; n++) {
			if (!bm->einfo[n].dtype) {
				if (ncvarid(fid, bm->einfo[n].name) < 0)
					quit("setupMSEBoxModel: general output file doesn't have %s\n", bm->einfo[n].name);
			}
		}

		/* Find out highest dump so far */
		ncdiminq(fid, ncdimid(fid, "t"), NULL, &n);
		bm->ncOfdump = (int)n;
	} else {
		quit("file with name %s already exists, either delete, rename or set reuse param in run.prm to 1\n", bm->ncOfname);
	}
	bm->ncOfid = fid;
    
    /* Open the fisheries output files */
	if (bm->fishout) {
		printf("Create fisheries output file\n");
        
		/* Overall fisheries output file */
		ncopts = 0;
		if (fid2 == -1) {
			fid2 = createBMDataFile(bm->destFolder, bm->ncOFishfname, bm, 1);
			bm->ncOfishdump = 0;
		} else if (bm->flagreusefile == 2) {
			/* Create file anew */
			ncopts = NC_CLOBBER;
			ncclose(fid2);
			fid2 = createBMDataFile(bm->destFolder, bm->ncOFishfname, bm, 1);
			bm->ncOfishdump = 0;
		} else if (bm->flagreusefile == 1) {
			/* File does exist - check dimension lengths */
			ncopts = NC_VERBOSE;
			ncdiminq(fid2, ncdimid(fid2, "b"), NULL, &n);
			if (n != bm->nbox)
				quit("setupMSEBoxModel: fisheries output file has wrong number of boxes\n");
			/* Check all variables are present */
			for (n = 0; n < bm->nfstat; n++) {
				if (bm->finfo[n].dtype == 1) {
					if (ncvarid(fid2, bm->finfo[n].name) < 0)
						quit("setupMSEBoxModel: fisheries output file doesn't have %s\n", bm->finfo[n].name);
				}
			}

			/* Find out highest dump so far */
			ncdiminq(fid2, ncdimid(fid2, "t"), NULL, &n);
			bm->ncOfishdump = (int)n;
		} else {
			quit("file with name %s already exists, either delete, rename or set reuse param in run.prm to 1\n", bm->ncOFishfname);
		}
		bm->ncOfishfid = fid2;
		ncopts = NC_VERBOSE | NC_FATAL;
        
 		/* Detailed fisheries output file */
		ncopts = 0;
		if (fid5 == -1) {
			fid5 = createBMDataFile(bm->destFolder, bm->ncODetFishfname, bm, 3);
			bm->ncOdetfishdump = 0;
		} else if (bm->flagreusefile == 2) {
			/* Create file anew */
			ncopts = NC_CLOBBER;
			ncclose(fid5);
			fid5 = createBMDataFile(bm->destFolder, bm->ncODetFishfname, bm, 3);
			bm->ncOdetfishdump = 0;
		} else if (bm->flagreusefile == 1) {
			/* File does exist - check dimension lengths */
			ncopts = NC_VERBOSE;
			ncdiminq(fid5, ncdimid(fid5, "b"), NULL, &n);
			if (n != bm->nbox)
				quit("setupMSEBoxModel: detailed fisheries output file has wrong number of boxes\n");
			/* Check all variables are present */
			for (n = 0; n < bm->nfstat; n++) {
				if (bm->finfo[n].dtype == 3) {
					if (ncvarid(fid5, bm->finfo[n].name) < 0)
						quit("setupMSEBoxModel: detailed fisheries output file doesn't have %s\n", bm->finfo[n].name);
				}
			}
			/* Find out highest dump so far */
			ncdiminq(fid5, ncdimid(fid5, "t"), NULL, &n);
			bm->ncOdetfishdump = (int)n;
		} else {
			quit("file with name %s already exists, either delete, rename or set reuse param in run.prm to 1\n", bm->ncODetFishfname);
		}
		bm->ncOdetfishfid = fid5;
		ncopts = NC_VERBOSE | NC_FATAL;
	}else{
		bm->ncOdetfishfid = -1;
		bm->ncOfishfid = -1;
	}

    /* Open the summary output file */
	printf("Create summary output file %s\n", bm->ncOSumfname);
	ncopts = 0;
	if (fid3 == -1) {
		fid3 = createBMSummaryDataFile(bm->destFolder, bm->ncOSumfname, bm);
		bm->ncOsumdump = 0;
	} else if (bm->flagreusefile == 2) {
		/* Create file anew */
		ncopts = NC_CLOBBER;
		ncclose(fid3);
		fid3 = createBMSummaryDataFile(bm->destFolder, bm->ncOSumfname, bm);
		bm->ncOsumdump = 0;
	} else if (bm->flagreusefile) {
		/* File does exist - check dimension lengths */
		ncopts = NC_VERBOSE;
		ncdiminq(fid3, ncdimid(fid3, "b"), NULL, &n);
		if (n != bm->nbox)
			quit("setupMSEBoxModel: summary output file has wrong number of boxes\n");

		/* Check all variables are present */
		for (n = 0; n < bm->ntracer; n++) {
			if (bm->tinfo[n].sumtype) {
				if (ncvarid(fid3, bm->tinfo[n].name) < 0)
					quit("setupMSEBoxModel: summary output file doesn't have %s\n", bm->tinfo[n].name);
			}
		}
		for (n = 0; n < bm->nepi; n++) {
			if (bm->einfo[n].sumtype) {
				if (ncvarid(fid3, bm->einfo[n].name) < 0)
					quit("setupMSEBoxModel: summary output file doesn't have %s\n", bm->einfo[n].name);
			}
		}

		/* Find out highest dump so far */
		ncdiminq(fid3, ncdimid(fid3, "t"), NULL, &n);
		bm->ncOsumdump = (int)n;
	} else {
		quit("file with name %s already exists, either delete, rename or set reuse param in run.prm to 1\n", bm->ncOSumfname);
	}
	bm->ncOsumfid = fid3;
	ncopts = NC_VERBOSE | NC_FATAL;
    
	/* Open the growth/consumption tracer output file */
	printf("Create growth and consumption output file\n");
	ncopts = 0;
	if (fid4 == -1) {
		/* Create file */
		fid4 = createBMDataFile(bm->destFolder, bm->ncOPCfname, bm, 2);
		bm->ncOpcdump = 0;

	} else if (bm->flagreusefile == 2) {
		/* Create file anew */
		ncopts = NC_CLOBBER;
		ncclose(fid4);
		fid4 = createBMDataFile(bm->destFolder, bm->ncOPCfname, bm, 2);
		bm->ncOpcdump = 0;
	} else if (bm->flagreusefile == 1) {
		/* File does exist - check dimension lengths */
		ncopts = NC_VERBOSE;
		ncdiminq(fid4, ncdimid(fid4, "b"), NULL, &n);
		if (n != bm->nbox)
			quit("setupMSEBoxModel: growth and consumption output file has wrong number of boxes\n");
		ncdiminq(fid4, ncdimid(fid4, "z"), NULL, &n);
		if (n != bm->wcnz + bm->sednz)
			quit("setupMSEBoxModel: growth and consumption output file has wrong total number of layers\n");
		/* Check all variables are present */
		for (n = 0; n < bm->ntracer; n++) {
			if (bm->tinfo[n].dtype == 2) {
				if ((ncvarid(fid4, bm->tinfo[n].name) < 0))
					quit("setupMSEBoxModel: general output file doesn't have %s\n", bm->tinfo[n].name);
			}
		}
		for (n = 0; n < bm->nepi; n++) {
			if (bm->einfo[n].dtype == 2) {
				if (ncvarid(fid4, bm->einfo[n].name) < 0)
					quit("setupMSEBoxModel: general output file doesn't have %s\n", bm->einfo[n].name);
			}
		}

		/* Find out highest dump so far */
		ncdiminq(fid4, ncdimid(fid4, "t"), NULL, &n);
		bm->ncOpcdump = (int)n;
	} else {
		quit("file with name %s already exists, either delete, rename or set reuse param in run.prm to 1\n", bm->ncOPCfname);
	}
	bm->ncOpcfid = fid4;
	ncopts = NC_VERBOSE | NC_FATAL;
    
    /* Open the annual age structured output files - if required */
    if(bm->flag_age_output > 1) {
        printf("Create annual age structured output file\n");

        if (!bm->ncOAAfname[0] || !bm->ncOAACfname[0])
            printf("Something went wrong in creating age structured output file names");

        ncopts = 0;
        if (fid6 == -1) {
            /* Create file */
            fid6 = createBMAnnAgeBioDataFile(bm->destFolder, bm->ncOAAfname, bm);
            bm->ncOaadump = 0;

        } else if (bm->flagreusefile == 2) {
            /* Create file anew */
            ncopts = NC_CLOBBER;
            ncclose(fid6);
            fid6 = createBMAnnAgeBioDataFile(bm->destFolder, bm->ncOAAfname, bm);
            bm->ncOaadump = 0;
        } else if (bm->flagreusefile == 1) {
            /* File does exist - check dimension lengths */
            ncopts = NC_VERBOSE;
            ncdiminq(fid6, ncdimid(fid6, "b"), NULL, &n);
            if (n != bm->nbox)
                quit("setupMSEBoxModel: age structured output file has wrong number of boxes\n");
            ncdiminq(fid6, ncdimid(fid6, "z"), NULL, &n);
            if (n != bm->wcnz + bm->sednz)
                quit("setupMSEBoxModel: age structured output file has wrong total number of layers\n");

            /* Find out highest dump so far */
            ncdiminq(fid6, ncdimid(fid6, "t"), NULL, &n);
            bm->ncOaadump = (int)n;

            // Should probably also check correct variables are stored in the file
        } else {
            quit("file with name %s already exists, either delete, rename or set reuse param in run.prm to 1\n", bm->ncOAAfname);
        }
        bm->ncOaafid = fid6;
        ncopts = NC_VERBOSE | NC_FATAL;

        if (bm->fishout) {
            printf("Create annual age structured catch and discards output file\n");
            ncopts = 0;
            if (fid7 == -1) {
                /* Create file */
                fid7 = createBMAnnAgeCatDataFile(bm->destFolder, bm->ncOAACfname, bm);
                bm->ncOaacdump = 0;

            } else if (bm->flagreusefile == 2) {
                /* Create file anew */
                ncopts = NC_CLOBBER;
                ncclose(fid7);
                fid7 = createBMAnnAgeCatDataFile(bm->destFolder, bm->ncOAACfname, bm);
                bm->ncOaacdump = 0;
            } else if (bm->flagreusefile == 1) {
                /* File does exist - check dimension lengths */
                ncopts = NC_VERBOSE;
                ncdiminq(fid7, ncdimid(fid7, "b"), NULL, &n);
                if (n != bm->nbox)
                    quit("setupMSEBoxModel: age structured catch and discards output file has wrong number of boxes\n");
                ncdiminq(fid7, ncdimid(fid7, "z"), NULL, &n);
                if (n != bm->wcnz + bm->sednz)
                    quit("setupMSEBoxModel: age structured catch and discards output file has wrong total number of layers\n");

                /* Find out highest dump so far */
                ncdiminq(fid7, ncdimid(fid7, "t"), NULL, &n);
                bm->ncOaacdump = (int)n;

                // Should probably also check correct variables are stored in the file
            } else {
                quit("file with name %s already exists, either delete, rename or set reuse param in run.prm to 1\n", bm->ncOAACfname);
            }
            bm->ncOaacfid = fid7;
            ncopts = NC_VERBOSE | NC_FATAL;
        }

    }
    
   /* Open the spatial diet output file - now in a text file
    printf("Create spatial output file\n");
    ncopts = 0;
    if (fid8 == -1) {
        // Create file
        fid8 = createBMDietDataFile(bm->destFolder, bm->ncODIETfname, bm);
        bm->ncOdietdump = 0;

    } else if (bm->flagreusefile == 2) {
        // Create file anew
        ncopts = NC_CLOBBER;
        ncclose(fid8);
        fid8 = createBMDietDataFile(bm->destFolder, bm->ncODIETfname, bm);
        bm->ncOpcdump = 0;
    } else if (bm->flagreusefile == 1) {
        // File does exist - check dimension lengths
        ncopts = NC_VERBOSE;
        ncdiminq(fid8, ncdimid(fid8, "b"), NULL, &n);
        if (n != bm->nbox)
            quit("setupMSEBoxModel: growth and consumption output file has wrong number of boxes\n");
        ncdiminq(fid8, ncdimid(fid8, "z"), NULL, &n);
        if (n != bm->wcnz + bm->sednz)
            quit("setupMSEBoxModel: growth and consumption output file has wrong total number of layers\n");

        // Find out highest dump so far
        ncdiminq(fid8, ncdimid(fid8, "t"), NULL, &n);
        bm->ncOdietdump = (int)n;
    } else {
        quit("file with name %s already exists, either delete, rename or set reuse param in run.prm to 1\n", bm->ncODIETfname);
    }
    bm->ncOdietfid = fid8;
    ncopts = NC_VERBOSE | NC_FATAL;
    */

	/* Check for cryptic biomass */
	Ecology_Boundary_Check(bm, logfp);

	/* Now everything initialised if you skip biology set do_biology and do_physics == 0 */
	if (bm->flag_skip_biol) {
		do_biology = 0;
	}
	if (bm->flag_skip_phys) {
		do_physics = 0;
	}
#ifdef BROKER_LINK_ENABLED
	if(do_BrokerLinkage){
		Linkage_Read_XML(bm, bm->brokerLinkageprmIfname);
	}
#endif
	/* Read in the CLAMLinkage input file */
#ifdef CLAM_LINK_ENABLED
	if(do_CLAMLinkage) {
		CLAM_Init(bm);
	}
	printf("going CLAM init\n");
#endif

	/* Before start model run make sure any updates to vertebrate numbers due to run.prm rescaling
	 are reflected in NTracers before printout first value
	 */

	for (fgIndex = 0; fgIndex < bm->K_num_tot_sp; fgIndex++) {
		if(FunctGroupArray[fgIndex].speciesParams[flag_id]){
			if (FunctGroupArray[fgIndex].isVertebrate == TRUE) {
				for (n = 0; n < FunctGroupArray[fgIndex].numCohortsXnumGenes; n++) {
					sn = FunctGroupArray[fgIndex].structNTracers[n];
					rn = FunctGroupArray[fgIndex].resNTracers[n];
					den = FunctGroupArray[fgIndex].NumsTracers[n];
					traceid = FunctGroupArray[fgIndex].totNTracers[0];

					for (ij = 0; ij < bm->nbox; ij++) {
						for (k = 0; k < bm->boxes[ij].nz; k++) {
							cellvol = bm->boxes[ij].area * bm->boxes[ij].dz[k];

							if (!n)
								bm->boxes[ij].tr[k][traceid] = 0;

							bm->boxes[ij].tr[k][traceid] += (bm->boxes[ij].tr[k][sn] + bm->boxes[ij].tr[k][rn]) * bm->boxes[ij].tr[k][den] * bm->X_CN * mg_2_tonne
									/ cellvol;
						}
					}
				}
			}
		}
	}
    return;
}

/**
 *	\brief Routine to initialise vert_vdistrib box properties
 */
static void set_up_vvdistrib(MSEBoxModel *bm) {
	int b, k, sp, ij;

	/* Initialise vert_vdistrib */
	for (b = 0; b < bm->nbox; b++) {
		for (k = 0; k < bm->wcnz; k++) {
			/* Set water column porosity values to 1 */

			for (sp = 0; sp < bm->K_num_tot_sp; sp++) {
				/* The mobile groups */
				if (FunctGroupArray[sp].isMobile == TRUE) {
					for (ij = 0; ij < FunctGroupArray[sp].numStages; ij++) {
						if (bm->boxes[b].type != BOUNDARY) {
							bm->boxes[b].vert_vdistrib[sp][ij][k] = 1.0 / bm->boxes[b].nz;
						} else {
							bm->boxes[b].vert_vdistrib[sp][ij][k] = 1.0;
						}
					}
					/* Now the rest of the cohort slots are set to 0 just in case */
					for (ij = FunctGroupArray[sp].numStages; ij < bm->K_num_max_stages; ij++) {
						bm->boxes[b].vert_vdistrib[sp][ij][k] = 0;
					}
				} else {
					for (ij = 0; ij < FunctGroupArray[sp].numStages; ij++) {
						/* Put everything else in bottom of the water column as sediment associated */
						if (!k)
							bm->boxes[b].vert_vdistrib[sp][ij][k] = 1.0;
						else
							bm->boxes[b].vert_vdistrib[sp][ij][k] = 0.0;
					}
				}
			}
		}
	}
	return;
}

/**
 * \brief Routine to initialise box model arrays
 */
void AllocateArrayMemory(MSEBoxModel *bm, FILE *llogfp) {
	int i, b, nyr, nid, totfluxout, nreg;
	//int numRunYears = ceil(bm->tstop / 86400) / 365.0 + 1;

	printf("flag_fine_ontogenetic_diets: %d initarray\n", bm->flag_fine_ontogenetic_diets);

	/* Initialise MSEBoxModel arrays */
    bm->BoxKeyMap = Util_Alloc_Init_2D_Int(3, bm->nbox, 0);

	bm->pSPageeat = Util_Alloc_Init_3D_Double(bm->K_num_tot_sp, bm->K_num_max_cohort * bm->K_num_max_genetypes, bm->K_num_tot_sp, 0.0);
	bm->pSPFCeat = Util_Alloc_Init_4D_Double(bm->K_num_max_stages, bm->K_num_max_stages, bm->K_num_tot_sp, bm->K_num_tot_sp, 0.0);
	bm->pSPVERTeat = Util_Alloc_Init_4D_Double(bm->K_num_max_stages, bm->K_num_max_stages, bm->K_num_tot_sp, bm->K_num_tot_sp, 0.0);
	bm->pFLEET = Util_Alloc_Init_2D_Double(bm->K_num_fisheries, bm->K_num_tot_sp, 0.0);

	/* Larval dispersion information */
	bm->mS = Util_Alloc_Init_3D_Double(2, 4, bm->K_num_tot_sp, 0.0);
	bm->recruit_hdistrib = Util_Alloc_Init_3D_Double(bm->K_num_tot_sp, bm->nbox, bm->K_num_max_genetypes, 0.0);
	bm->recruit_hdistrib_orig = Util_Alloc_Init_3D_Double(bm->K_num_tot_sp, bm->nbox, bm->K_num_max_genetypes, 0.0);
	bm->refuge_status = Util_Alloc_Init_3D_Double(2, bm->nbox, bm->K_num_tot_sp, 1.0);
    bm->turbid_effect = Util_Alloc_Init_4D_Double(2, bm->wcnz, bm->nbox, bm->K_num_tot_sp, 1.0);
    bm->scaling_indices = Util_Alloc_Init_3D_Int(bm->K_num_max_cohort * bm->K_num_max_genetypes * bm->K_num_max_stages, bm->K_num_tot_sp, num_scaling_indicies, -1);

	bm->HABITATlike = Util_Alloc_Init_3D_Double(bm->K_num_cover_types, 2, bm->K_num_tot_sp, 0);
	if(bm->ice_on == TRUE){
		bm->ICE_HABITATlike = Util_Alloc_Init_3D_Int(bm->K_num_ice_classes, 2, bm->K_num_tot_sp, 0);
		bm->ICE_HABITATlikeReprod = Util_Alloc_Init_3D_Int(bm->K_num_ice_classes, 2, bm->K_num_tot_sp, 0);
	}

	/* Stock structure information */
	bm->stock_struct = Util_Alloc_Init_2D_Int(bm->K_num_tot_sp, bm->nbox, 0);
	bm->v_stock_struct = Util_Alloc_Init_2D_Int(bm->K_num_tot_sp, bm->wcnz, 0);
	bm->group_stock = Util_Alloc_Init_3D_Int(bm->wcnz, bm->nbox, bm->K_num_tot_sp, 0);

	bm->coveramt = Util_Alloc_Init_2D_Double(bm->nbox, bm->K_num_cover_types, 0.0);

	bm->totinitpop = Util_Alloc_Init_1D_Double(bm->K_num_tot_sp + 1, 0.0); // The +1 is for DIN
	bm->totfishpop = Util_Alloc_Init_1D_Double(bm->K_num_tot_sp + 1, 0.0); // The +1 is for DIN
	bm->totagepop = Util_Alloc_Init_2D_Double(bm->K_num_max_cohort * bm->K_num_max_genetypes, bm->K_num_tot_sp + 1, 0.0);
	bm->boxBiomass = Util_Alloc_Init_2D_Double(bm->K_num_tot_sp + 1, bm->nbox, 0.0);

	/* Set up group and fisheries name arrays - the 15 is the length of the string and would
	 need to be increased if the fisheries or group code ids increased in size */
	bm->fisheryParamNAME = (char **) c_alloc2d(25, tot_fisheries_prms);

	/* Overall fishery parameter arrays - values are passed to these directly
	 during parameter readin so can't be set further down with all the other
	 arrays (whose size is dependent on parameters, so can't do those until
	 after the parameter read */
	/* Fishery specific parameters */
	bm->FISHERYprms = Util_Alloc_Init_2D_Double(tot_fisheries_prms, bm->K_num_fisheries, 0.0);
	/* Species specific fishery parameters */
	bm->SP_FISHERYprms = Util_Alloc_Init_3D_Double(tot_sp_specif_fishing_prms, bm->K_num_fisheries, bm->K_num_tot_sp, 0.0);
   
	/* Set up arrays for fisheries processes and parameters. */
	if (bm->K_num_reg > bm->K_num_stocks_per_sp)
		nreg = bm->K_num_reg;
	else
		nreg = bm->K_num_stocks_per_sp;

	nid = DiscardsAtAge_result_id + 1;
	bm->estBo = 0; // Set this way here so read in correctly in readModelFCParameter(), which needs zeroed pointed to allocate correctly

	/* Fishing results */
	bm->FishingResults = Util_Alloc_Init_2D_Double(nid, bm->K_num_fisheries, 0.0);
	bm->FCcaught = (double **) alloc2d(bm->K_num_max_cohort * bm->K_num_max_genetypes, bm->K_num_tot_sp);
	bm->FCcaughttemp = (double ***) alloc3d(bm->K_num_max_cohort * bm->K_num_max_genetypes, bm->K_num_fisheries, bm->K_num_tot_sp);
	bm->FCcocatch = Util_Alloc_Init_2D_Double(bm->K_num_tot_sp, bm->K_num_tot_sp, 0.0);
	bm->FCdiscard = (double **) alloc2d(bm->K_num_max_cohort * bm->K_num_max_genetypes, bm->K_num_tot_sp);
	bm->FCtsCarryOver = Util_Alloc_Init_3D_Double(bm->K_num_tot_sp, 2, bm->nbox + 1 + bm->K_num_stocks_per_sp, 0.0);

	/* Mortality tracking - used to generate the mort output files */
	bm->calcMnum = Util_Alloc_Init_2D_Double(3, bm->K_num_tot_sp, 0.0);
	bm->calcMnumPerPred = Util_Alloc_Init_3D_Double(3, bm->K_num_tot_sp, bm->K_num_tot_sp, 0.0);
	bm->calcMLinearMort = Util_Alloc_Init_2D_Double(3, bm->K_num_tot_sp, 0.0);
	bm->calcMQuadMort = Util_Alloc_Init_2D_Double(3, bm->K_num_tot_sp, 0.0);
	bm->calcMPredMort = Util_Alloc_Init_2D_Double(3, bm->K_num_tot_sp, 0.0);

	bm->calcELinearMort = Util_Alloc_Init_2D_Double(3, bm->K_num_tot_sp, 0.0);

	bm->calcFnum = Util_Alloc_Init_2D_Double(3, bm->K_num_tot_sp, 0.0);
	bm->calcNstart = Util_Alloc_Init_2D_Double(2, bm->K_num_tot_sp, 0.0);
	bm->calcNstartPerPred = Util_Alloc_Init_2D_Double(2, bm->K_num_tot_sp, 0.0);
	bm->calcTrackedMort = Util_Alloc_Init_4D_Double(K_num_mort_counter, bm->K_num_stocks_per_sp, bm->K_num_max_cohort * bm->K_num_max_genetypes, bm->K_num_tot_sp, 0.0);
	bm->calcTrackedPredMort = Util_Alloc_Init_5D_Double(2, bm->K_num_tot_sp, bm->K_num_stocks_per_sp, bm->K_num_max_cohort * bm->K_num_max_genetypes, bm->K_num_tot_sp, 0.0);
	bm->Catch = Util_Alloc_Init_4D_Double(bm->wcnz, bm->K_num_fisheries, bm->K_num_tot_sp, bm->nbox, 0.0);
	bm->CatchQueue = Util_Alloc_Init_4D_Double(bm->K_num_catchqueue, bm->nbox, bm->K_num_fisheries, bm->K_num_tot_sp, 0.0);

	bm->CumCatch = Util_Alloc_Init_4D_Double(bm->wcnz, bm->nbox, bm->K_num_fisheries, bm->K_num_tot_sp, 0.0);
	bm->CumDiscards = Util_Alloc_Init_3D_Double(bm->nbox, bm->K_num_fisheries, bm->K_num_tot_sp, 0.0);
	bm->CumEffort = Util_Alloc_Init_2D_Double(bm->nbox, bm->K_num_fisheries, 0.0);
    bm->CumDisplaceEffort = Util_Alloc_Init_2D_Double(bm->K_num_fisheries, bm->nbox, 0.0);
    
	bm->CatchTS_agedistrib = Util_Alloc_Init_3D_Double(bm->K_num_max_cohort, bm->K_num_tot_sp, bm->K_num_fisheries, 0.0);  // intentionally left as only per age cohort
	bm->CatchTS_agedistribOrig = Util_Alloc_Init_3D_Double(bm->K_num_max_cohort, bm->K_num_tot_sp, bm->K_num_fisheries, 0.0);  // intentionally left as only per age cohort
	bm->DAScalc = Util_Alloc_Init_1D_Double(bm->K_num_tot_sp, 0.0);
	bm->DiscardTS_agedistrib = Util_Alloc_Init_3D_Double(bm->K_num_max_cohort, bm->K_num_tot_sp, bm->K_num_fisheries, 0.0);  // intentionally left as only per age cohort
	bm->DiscardTS_agedistribOrig = Util_Alloc_Init_3D_Double(bm->K_num_max_cohort, bm->K_num_tot_sp, bm->K_num_fisheries, 0.0);  // intentionally left as only per age cohort
	bm->Discards = Util_Alloc_Init_3D_Double(bm->K_num_fisheries, bm->K_num_tot_sp, bm->nbox, 0.0);
	bm->DependDiscardsTot = Util_Alloc_Init_3D_Double(bm->K_num_max_cohort * bm->K_num_max_genetypes * bm->K_num_max_genetypes, bm->K_num_tot_sp, bm->K_num_fisheries, 0.0);

	bm->Effort = Util_Alloc_Init_2D_Double(bm->K_num_fisheries, bm->nbox, 0.0);
	bm->Effort_hdistrib = Util_Alloc_Init_3D_Double(K_num_effort_entries, bm->K_num_fisheries, bm->nbox, 0.0); // 12 months + one entry for today_effort and one entry for calc_effort
	bm->EffortPenalty = Util_Alloc_Init_2D_Double(bm->K_num_fisheries, bm->nbox, 0.0);
	bm->estinitpop = Util_Alloc_Init_1D_Double(bm->K_num_tot_sp, 0.0);

    bm->InvaderStartNums = Util_Alloc_Init_1D_Double(bm->K_num_max_cohort, 0.0);

	bm->GhostEffort = Util_Alloc_Init_2D_Double(bm->K_num_fisheries, bm->nbox, 0.0);
	bm->LastCatch = Util_Alloc_Init_3D_Double(bm->nbox, bm->K_num_fisheries, bm->K_num_tot_sp, 0.0);

	bm->FC_HABITATlike = Util_Alloc_Init_2D_Double(bm->K_num_cover_types, bm->K_num_fisheries, 0);

	bm->FISHERYtarget = Util_Alloc_Init_2D_Int(bm->K_num_tot_sp, bm->K_num_fisheries, 0);
	bm->OldCumEffort = Util_Alloc_Init_2D_Double(bm->nbox, bm->K_num_fisheries, 0.0);
	bm->OldEffort = Util_Alloc_Init_2D_Double(bm->K_num_fisheries, bm->nbox, 0.0);

	bm->Port_Fishery = Util_Alloc_Init_2D_Int(bm->K_num_ports, bm->K_num_fisheries, 0);
	bm->Port_info = Util_Alloc_Init_2D_Double(K_num_port_prm, bm->K_num_ports, 0.0);
	bm->Port_distance = Util_Alloc_Init_2D_Double(bm->nbox, bm->K_num_ports, 0.0);
	bm->PortContrib = Util_Alloc_Init_3D_Double(2, bm->nbox, bm->K_num_fisheries, 0.0);
	bm->totPortContrib = Util_Alloc_Init_2D_Double(2, bm->K_num_fisheries, 0.0);

	// Not initialised.
	bm->RecCatch = Util_Alloc_Init_3D_Double(bm->K_num_fisheries, bm->K_num_tot_sp, bm->nbox, 0.0);
	bm->RegionalData = Util_Alloc_Init_3D_Double(2, nreg, bm->K_num_tot_sp, 1.0);

	// The following are used but were never set.
	//bm->SizeNumCatch = Util_Alloc_Init_4D_Double(bm->nbox, bm->K_num_fisheries, bm->K_num_max_cohort * bm->K_num_max_genetypes, bm->K_num_tot_sp, 0.0);
	//bm->SizeNumDiscard = Util_Alloc_Init_4D_Double(bm->nbox, bm->K_num_fisheries, bm->K_num_max_cohort * bm->K_num_max_genetypes, bm->K_num_tot_sp, 0.0);

	bm->targetspbiom = Util_Alloc_Init_2D_Double(bm->nbox, bm->K_num_tot_sp, 0.0);
	bm->TAC_trigger = Util_Alloc_Init_2D_Double(3, bm->K_num_fisheries, 0.0);
	bm->TempCPUE = Util_Alloc_Init_2D_Double(bm->K_num_fisheries, bm->nbox, 0.0);
	bm->totcatch = Util_Alloc_Init_1D_Double(bm->K_num_tot_sp, 0.0);
	bm->totCPUE = Util_Alloc_Init_1D_Double(bm->K_num_fisheries, 0.0);
	bm->totdiscards = Util_Alloc_Init_1D_Double(bm->K_num_tot_sp, 0.0);
	bm->totNewEffort = Util_Alloc_Init_1D_Double(bm->K_num_fisheries, 0.0);
	bm->TotOldCumEffort = Util_Alloc_Init_1D_Double(bm->K_num_fisheries, 0.0);
	bm->totOldEffort = Util_Alloc_Init_1D_Double(bm->K_num_fisheries, 0.0);

	// Not initialised.
	bm->tseffortid = (int *) i_alloc1d(bm->K_num_fisheries);
	bm->tseconid = (int *) i_alloc1d(bm->K_num_tot_sp);
	bm->tsMPAid = (int *) i_alloc1d(bm->K_num_fisheries);

	bm->pSPcheck = Util_Alloc_Init_2D_Int(bm->K_num_max_cohort * bm->K_num_max_genetypes, bm->K_num_tot_sp, 0);

	/* Initialise fishery parameter arrays */

	for (b = 0; b < bm->K_num_fisheries; b++) {
		for (i = 0; i < 2; i++) {
			bm->TAC_trigger[b][i] = 0;
		}
		bm->TAC_trigger[b][triggered_scalar_id] = 1.0;
	}

	/* Set-up market arrays */
	bm->MARKETprms = Util_Alloc_Init_3D_Double(K_num_market_prm, bm->K_num_tot_sp, bm->K_num_markets, 0.0);
	bm->MARKETmonth = Util_Alloc_Init_3D_Double(12, bm->K_num_tot_sp, bm->K_num_markets, 0.0);

	/* Set-up fuel arrays */
	bm->FUELprms = Util_Alloc_Init_1D_Double(K_num_fuel_prm, 0.0);
	bm->FUELmonth = Util_Alloc_Init_1D_Double(12, 0.0);

	/* Set-up assessment arrays */
	if (bm->tassessstart > bm->tstop)
		nyr = 1;
	else
		nyr = (int) floor(ROUNDGUARD + (((bm->tstop / 86400.0) - bm->tassessstart) / 365.0 + 1));

	//fprintf(llogfp,"nyr: %d, ROUNDGUARD: %e, bm->tstop: %e, bm->tassessstart: %e, bracket: %e\n", nyr, ROUNDGUARD , bm->tstop, bm->tassessstart, ((bm->tstop - bm->tassessstart)/365.0 + 1));

	/* Initialise assessment arrays */
	bm->EffortRecord = Util_Alloc_Init_3D_Double(2, bm->K_num_tot_sp, nyr, 0.0);
	bm->CatchRecord = Util_Alloc_Init_4D_Double(2, bm->K_num_max_cohort * bm->K_num_max_genetypes, bm->K_num_tot_sp, nyr, 0.0);
	bm->NAssess = Util_Alloc_Init_2D_Double(K_num_assess_prms, bm->K_num_tot_sp, 0.0);

	bm->NAssessNAME = (char **) c_alloc2d(20, K_num_assess_prms);
	Assess_Set_Name_ID(bm);

	/* Set-up debugging arrays */
	totfluxout = bm->K_num_tot_sp + num_nut_flux_id; // Extra entries for nutrient fluxes
	not_tracking_flux = totfluxout;

	if (totfluxout != not_tracking_flux)
		quit("Need to recode so that the defined value of not_tracking_flux (%d) equals K_num_tot_sp (%d) + num_nut_flux_id (%d)\n", not_tracking_flux,
				bm->K_num_tot_sp, num_nut_flux_id);

	/* Set up performance measure arrays */
	bm->ecolindx = Util_Alloc_Init_1D_Double(K_num_ecol_indx, 0.0);
	bm->econindx = Util_Alloc_Init_2D_Double(K_num_econ_indx, bm->K_num_fisheries, 0.0);
	bm->portindx = Util_Alloc_Init_2D_Double(K_num_port_indx, bm->K_num_ports, 0.0);

	/* Set up performance measure name arrays - the 20 is the length of the string and would
	 need to be increased if the names of the code ids increased in size */
	bm->ecolindxNAME = (char **) c_alloc2d(20, K_num_ecol_indx);
	bm->econindxNAME = (char **) c_alloc2d(20, K_num_econ_indx);
	bm->SUBFLEETeconindxNAME = (char **) c_alloc2d(20, K_sub_fleet_params);
	bm->portindxNAME = (char **) c_alloc2d(20, K_num_port_indx);

	Ecology_Set_Index_Name_ID(bm);

	/* Allocate memory for P:N:C tracer stuff */
	if(bm->track_atomic_ratio){
		Ecology_Initialise_Atomic_Info(bm);
	}

	if (verbose > 3){
	  printf(" Finish AllocateArrayMemory\n");
	}
	return;
}
/**
 *
 *	\brief Routine to initialise log file
 */
FILE * initLogFile(MSEBoxModel *bm) {
	FILE *fp;
	char *fname = "log.txt";
	int index;

	strcpy(bm->logFileName, "log.txt");
	bm->logFileIndex = 1;

	/* Create file */
	if ((fp = Util_fopen(bm, fname, "w")) == NULL)
		quit("initLogFile: Can't open %s\n", fname);

	bm->logFile = fp;

	fprintf(fp, "#\n#Atlantis2 Updated to Version %s\n#\n#\n", ATLANTIS_REVISION);
	fprintf(fp, "#\n#Atlantis path %s\n#\n#\n", ATLANTIS_WCPATH);
	fprintf(fp, "#\n#Atlantis SVN Last Change Date %s\n\n", ATLANTIS_WCDATE);


	/* File title */
	fprintf(fp, "# Log file for run of Atlantis - only ecological errors and important events are recorded here #\n");
	fprintf(fp, "# Key to indices used in Atlantis, which may appear in logged errors here:\n");

	/* Write out the group code and name. */
	//fprintf(fp, "# group   name\n");
	for(index = 0; index < bm->K_num_tot_sp; index++){
		fprintf(fp, "# %s     %s\n", FunctGroupArray[index].groupCode, FunctGroupArray[index].fullName);
	}

	/* Write out the fishery code and name. */
	for(index = 0; index < bm->K_num_fisheries; index++){
		fprintf(fp, "# %s     %s\n", FisheryArray[index].fisheryCode, FisheryArray[index].name);
	}

	/* Return file pointer */
	return (fp);
}

/**
 *
 * \brief Check the size of the log file. If its too big create a now one.
 *
 *	This is critical code - can't be multithreaded.
 *
 */
FILE *checkLogFileSize(FILE *llogfp, MSEBoxModel *bm) {

	struct stat buffer;
	FILE *fp;
	char fname[STRLEN];

	sprintf(fname, "%s%s", bm->destFolder, bm->logFileName);

	/* Might need to close the file and then open it after we have checked its size */
	/* Check the size of the diff file. */
	if(stat(fname, &buffer) < 0){
		quit("Unable to get information about file %s\n", fname);
	}

	if (buffer.st_size > MAX_LOG_FILE_SIZE) {

		fflush(llogfp);
		fclose(llogfp);

		bm->logFileIndex++;

		sprintf(bm->logFileName, "log%d.txt", bm->logFileIndex);

		if ((fp = Util_fopen(bm, bm->logFileName, "w")) == NULL)
			quit("initLogFile: Can't open %s\n", bm->logFileName);

		bm->logFile = fp;
		return fp;
	}
	bm->logFile = llogfp;
	return llogfp;
}

void checknetCDFFiles(MSEBoxModel *bm) {

	ncopts = NC_CLOBBER;

	if(bm->t == 0){
		bm->ncOfIndex = 0;
		bm->ncOfishIndex = 0;
		bm->ncOsumIndex = 0;
		bm->ncOpcIndex = 0;
        bm->ncOdetfishIndex = 0;
        bm->ncOaaIndex = 0;
        bm->ncOaacIndex = 0;
	}

	bm->ncOfid = Util_Check_NetCDF_Size(bm, bm->ncOfid, &bm->ncOfdump, bm->ncOfname, &bm->ncOfIndex, 0);
	bm->ncOfishfid = Util_Check_NetCDF_Size(bm, bm->ncOfishfid, &bm->ncOfishdump, bm->ncOFishfname, &bm->ncOfishIndex, 1);
	bm->ncOsumfid = Util_Check_NetCDF_Size(bm, bm->ncOsumfid, &bm->ncOsumdump, bm->ncOSumfname, &bm->ncOsumIndex, -1);
	bm->ncOpcfid = Util_Check_NetCDF_Size(bm, bm->ncOpcfid, &bm->ncOpcdump, bm->ncOPCfname, &bm->ncOpcIndex, 2);
	bm->ncOdetfishfid = Util_Check_NetCDF_Size(bm, bm->ncOdetfishfid, &bm->ncOdetfishdump, bm->ncODetFishfname, &bm->ncOdetfishIndex, 3);

    if(bm->flag_age_output > 1) {
        bm->ncOaafid = Util_Check_NetCDF_Size(bm, bm->ncOaafid, &bm->ncOaadump, bm->ncOAAfname, &bm->ncOaaIndex, 4);
        if (bm->fishout) {
            bm->ncOaacfid = Util_Check_NetCDF_Size(bm, bm->ncOaacfid, &bm->ncOaacdump, bm->ncOAACfname, &bm->ncOaacIndex, 5);
        }
    }

    //bm->ncOdietfid = Util_Check_NetCDF_Size(bm, bm->ncOdietfid, &bm->ncOdietdump, bm->ncODIETfname, &bm->ncOdietIndex, 6);
}

/**
 * \brief Routine to initialise halt file
 *
 */
void initHaltFile(MSEBoxModel *bm) {
	FILE *fp;
	char *fname = "delete_to_halt_run";

	/* Create file */
	if ((fp = Util_fopen(bm, fname, "w")) == NULL)
		quit("initHaltFile: Can't open %s\n", fname);

	/* Close file so can be used by main loop */
	fclose(fp);

	return;
}

/**
 * \brief This routine calls the shutdown procedures
 *
 */
void modelshutdown(MSEBoxModel *bm) {
    int b = 0;
	printf("Freeing general box arrays\n");

    /* Free flux trackers */
    for (b = 0; b < bm->nbox; b++) {
        free1d(bm->boxes[b].flush_in);
        free1d(bm->boxes[b].flush_out);

        if(bm->boxes[b].uniqueAdjBoxes){
        	free(bm->boxes[b].uniqueAdjBoxes);
        }

        free(bm->boxes[b].distID);
    }

	/* Free Memory for top level boxmodel arrays */
    i_free1d((int*) bm->is_boundary);
	free2d((double**) bm->nom_dz);
	free2d((double**) bm->dz);
	free3d((double***) bm->wctr);
	free3d((double***) bm->sedtr);
	free2d((double**) bm->epi);
	free2d((double**) bm->vol);
	free2d((double**) bm->por);
	free2d((double**) bm->hddrain);
	free2d((double**) bm->hdpool);
	free2d((double**) bm->efluxes);
	free2d((double**) bm->vfluxes);
	free4d((double****) bm->vvdistrib);
	free3d((double***) newwctr);
	free3d((double***) newsedtr);
	if(bm->ice_on){
		free3d((double***) newicetr);
		free3d((double***) bm->icetr);
		free2d((double**) bm->icedz);
	}
	if(bm->terrestrial_on){
		free2d((double**) newlandtr);
		free2d((double**) bm->landtr);
	}

	i_free2d(bm->checkedalready);
	free2d(dvol);
	free3d(dtr);

	 if (bm->flagdietcheck) {
		// If age structured diets required then free up the array.
		free5d(bm->totDiet);
	}

	free1d(bm->init_scalar);
	if(bm->estBo)
		free1d(bm->estBo);
	free2d(bm->portindx);
	c_free2d(bm->portindxNAME);

	free1d(bm->estinitpop);
	free2d(bm->diagnost);

	/* Free memory allocated for model variable info */
	printf("Freeing model variables\n");

	c_free2d(paramStrings);
	c_free2d(cohortParamStrings);
	c_free2d(spawnParamStrings);
	/* Shouldn't really allocate this unless we are using RBC tier code but leave for now */
	if(RBCParamStrings)
		c_free2d(RBCParamStrings);
    
    c_free2d(bm->SP_FISHERYprmsName);

	freeBMTracerInfo(bm);
	freeBMphysInfo(bm);
	freeBMFisheriesInfo(bm);
	freeBMEpiInfo(bm);
	freeBMDiagInfo(bm);
    
    if(bm->ice_on){
		freeBMIceInfo(bm);
	}
	if(bm->terrestrial_on)
		freeBMLandInfo(bm);
    
    /* Free library specific arrays */
	if (do_assess){
		SS3Link_Free(bm);
		Assess_Free(bm);
	}

#ifdef BROKER_LINK_ENABLED
	if(do_BrokerLinkage){
		Linkage_Free(bm);
	}
#endif
    
	Ecology_Free(bm);
	Manage_Free(bm);
	Harvest_Free(bm);
	freePhysics(bm);
	Economic_Free(bm);
	Implementation_Free(bm);
    
	/* If defined free up the CLAM module */
#ifdef CLAM_LINK_ENABLED
	if(do_CLAMLinkage) {
		CLAM_Free(bm);
	}
#endif
	/* Free memory allocated for the geometry. */
	printf("Freeing box geometry\n");
	freeMSEBoxModelGeom(bm);

	/* Free biological arrays */
	printf("Freeing biological arrays\n");

    i_free2d(bm->BoxKeyMap);
	free3d(bm->pSPageeat);
	free4d(bm->pSPFCeat);
	free4d(bm->pSPVERTeat);
	free2d(bm->pFLEET);
	//free2d(bm->VERT_PREYeaten);
	//free2d(bm->FSPB);

	free3d(bm->mS);

	if(bm->larvaldistrib)
		free2d(bm->larvaldistrib);
	if(bm->connectivitymatrix)
		free4d(bm->connectivitymatrix);

	free3d(bm->recruit_hdistrib);
    free3d(bm->recruit_hdistrib_orig);
	free3d(bm->refuge_status);
    free4d(bm->turbid_effect);
    i_free3d(bm->scaling_indices);

	free3d(bm->HABITATlike);
	if(bm->ice_on == TRUE){
		i_free3d(bm->ICE_HABITATlike);
		i_free3d(bm->ICE_HABITATlikeReprod);
	}
	i_free2d(bm->stock_struct);
	free3d(bm->stock_struct_prop);
	i_free2d(bm->v_stock_struct);
	i_free3d(bm->group_stock);
	free2d(bm->coveramt);
	free4d(bm->tempPopRatio);
	free1d(bm->totinitpop);
	free1d(bm->totfishpop);
	free2d(bm->boxBiomass);
	free2d(bm->totagepop);
	//free2d(bm->agingMIG);
	free1d(bm->totbiom);
	free2d(bm->groupTotCatch);
	free1d(bm->diagnosticBiom);
	i_free1d(bm->regID);
	free2d(bm->reg_prop);
	free2d(bm->initreg_prop);
	free2d(bm->lastreg_prop);

	i_free1d(bm->tseconid);

	if(bm->track_atomic_ratio){
		Ecology_Free_Atomic_Info(bm);
	}

	/* Free name arrays */
	printf("Freeing names\n");
	c_free2d(bm->fisheryParamNAME);

	/* Free fishery arrays  */
	printf("Freeing fisheries arrays\n");

	free2d(bm->calcMnum);
	free3d(bm->calcMnumPerPred);

	free2d(bm->calcMLinearMort);
	free2d(bm->calcMQuadMort);
	free2d(bm->calcMPredMort);

	free2d(bm->calcELinearMort);

	free2d(bm->calcFnum);
	free2d(bm->calcNstart);
	free2d(bm->calcNstartPerPred);
	free4d(bm->calcTrackedMort);
	free5d(bm->calcTrackedPredMort);

	free2d(bm->FISHERYprms);
	i_free2d(bm->FISHERYtarget);

	free3d(bm->SP_FISHERYprms);
	free2d(bm->FishingResults);
	free2d(bm->FCcaught);
	free2d(bm->FCdiscard);

	free3d(bm->FCcaughttemp);
	free2d(bm->FCcocatch);
	free3d(bm->FCtsCarryOver);
	free4d(bm->Catch);
	free4d(bm->CatchQueue);
	free4d(bm->CumCatch);
	free3d(bm->CumDiscards);
	free2d(bm->CumEffort);
    free2d(bm->CumDisplaceEffort);
    
	free3d(bm->CatchTS_agedistrib);
	free3d(bm->CatchTS_agedistribOrig);
	free3d(bm->DiscardTS_agedistrib);
	free3d(bm->DiscardTS_agedistribOrig);
	free3d(bm->Discards);
	free3d(bm->DependDiscardsTot);
	free2d(bm->Effort);
	free3d(bm->Effort_hdistrib);
	free2d(bm->EffortPenalty);
	free2d(bm->GhostEffort);
	free3d(bm->LastCatch);
	free2d(bm->fishstat);
	free1d(bm->DAScalc);

    free1d(bm->InvaderStartNums);

	free2d(bm->FC_HABITATlike);

	/* These arrays are only allocated if the fisheries module is in use */
	if (bm->flag_fisheries_on) {
		free2d(bm->MPA);
		free4d(bm->MPAlist);
		free3d(bm->POPchange);
	}

	free2d(bm->OldCumEffort);
	free2d(bm->OldEffort);
	i_free2d(bm->Port_Fishery);

	free2d(bm->Port_info);
	free2d(bm->Port_distance);
	free3d(bm->PortContrib);
	free2d(bm->totPortContrib);

	free3d(bm->RecCatch);
	free3d(bm->RegionalData);
	free2d(bm->targetspbiom);
	free2d(bm->TAC_trigger);
	free2d(bm->TempCPUE);
	free1d(bm->totcatch);
	free1d(bm->totCPUE);
	free1d(bm->TotOldCumEffort);
	free1d(bm->totdiscards);
	free1d(bm->totNewEffort);
	free1d(bm->totOldEffort);

	i_free1d(bm->tseffortid);
	i_free1d(bm->tsMPAid);
	i_free2d(bm->pSPcheck);
	i_free2d(bm->sp_basket);

	i_free3d(bm->TAC_over);

	/* Free market arrays */
	printf("Freeing market arrays\n");

	free3d(bm->MARKETprms);
	free3d(bm->MARKETmonth);

	/* Free market arrays */
	printf("Freeing fuel arrays\n");

	free1d(bm->FUELprms);
	free1d(bm->FUELmonth);

	/* Free assessment arrays */
	printf("Freeing assessment arrays\n");

	free3d(bm->EffortRecord);
	free4d(bm->CatchRecord);
	free2d(bm->NAssess);
	c_free2d(bm->NAssessNAME);

	/* Free performance measure arrays */
	printf("Freeing performance measures\n");

	free1d(bm->ecolindx);

	free2d(bm->econindx);

	c_free2d(bm->ecolindxNAME);

	c_free2d(bm->econindxNAME);
	c_free2d(bm->SUBFLEETeconindxNAME);

	printf("Freeing physiochem property memory\n");
	Ecology_PhysioChem_Free(bm);

	/* Free up the structures allocated by the util lib */
	Util_Free(bm);

	return;

}
/**
 \brief This is the error message given if the input command is incorrect it also shows the correct
 command line form
 -i for input file (input.nc)
 dump indicates where in input file to start (spot zero)
 -o for biological output file (output.nc)
 -d for fisheries output file (fishing.nc)
 -s for summary output file (fishing.nc)
 -r for run definition parameter file (run.prm)
 -f for forcing file with paths to forcing files listed (force.prm)
 -p for physical parameter file (physics.prm)
 -b for biological parameter file (biology.prm)
 -h for harvest parameter file (harvest.prm)
 -a for assessment model parameter file (assess.prm)
 -e for economics model parameter file (assess.prm)

 Square brackets indicate optional model components, without these files the model will run,
 but without that part of the model (biology or physics)
 */

void Util_Usage(int dummy) {

	printf("\nAtlantis SVN Updated to Version %s\n", ATLANTIS_REVISION);
	printf("Atlantis SVN Path %s\n", ATLANTIS_WCPATH);
	printf("Atlantis SVN Last Change Date %s\n\n", ATLANTIS_WCDATE);


	printf("Util_Usage: atlantis -i input.nc dump -o output.nc -r run.prm -f force.prm -p physics.prm -b biology.prm -m migration.csv -h harvest.prm -a assess.prm -e economics.prm -s functionGroupFile.xml -q fisheries.xml [-d destinationFolder] [-t inputFolder]\n");
	printf("\nDestinationFolder - An optional parameter. If provided a new folder with this name will be create and all output files generated by Atlantis will be placed in this folder.\n");
	printf("\nFurther information about running Atlantis can be found in the Atlantis manual or Atlantis wiki site.\n\n");
	exit(0);
}
