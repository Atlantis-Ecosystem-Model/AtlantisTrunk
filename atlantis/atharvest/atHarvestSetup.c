/**
 * \ingroup atHarvestLib
 *	\file atHarvestSetup.c
 *	\brief The C file containing the harvest setup code.
 *
 *	\author Bec Gorton 28/Nov/2008
 *
 *	<b>Revisions:</b>
 *
 *
 *	28-10-2009 Bec Gorton
 *	Finished rewriting the code to read in the input parameters from the XML file.
 *
 *	02-11-2009 Bec Gorton
 *	Added some function comments, changed where the allocateHarvestStruct function is called to ensure
 *	things are not allocated if the fisheries module is not 'on'.
 *
 */

/*  Open library routines */
/*  Open library routines */
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <assert.h>
#include <math.h>
#include <atHarvest.h>
#include <atEcologyLib.h>
#include <convertXML.h>
#include "atHarvestIO.h"

/* Functions that are private to this file */
static void Setup_Fishery_Param_Strings(MSEBoxModel *bm);
static void Build_Fishery_Species_Linkage(MSEBoxModel *bm, FILE *llogfp);
static void Calculate_Age_Vulnerability(MSEBoxModel *bm, FILE *llogfp);
static void Build_Fishery_Tracers(MSEBoxModel *bm);
static void Allocate_Harvest_Memory(MSEBoxModel *bm);

/**
 * \brief Initialise the harvest module.
 */
void Harvest_Init(MSEBoxModel *bm, FILE *llogfp) {
	int i, b, nf, sp;
	char convertedXMLFileName[STRLEN];

	if(verbose)
		printf("Starting fisheries param read\n");

	if (!bm->flag_fisheries_on) {

		/* If assessment is on then we do need to read in the fishery definition file */
		if(do_assess)
			if (Util_Read_Fisheries_XML(bm, bm->fisheryIfname, logfp) == FALSE)
				quit("Failed to find Fishery definition file %s\n", bm->fisheryIfname);
		return;
	}

	first_data_done = 0;
	Setup_Fishery_Param_Strings(bm);
	Allocate_Harvest_Memory(bm);

	/* Read in theFishery parameters */
	if (Util_Read_Fisheries_XML(bm, bm->fisheryIfname, logfp) == FALSE)
		quit("Failed to find Fishery definition file %s\n", bm->fisheryIfname);

//	if (!bm->flag_fisheries_on) {
//		return;
//	}

	Open_Harvest_Output_Files(bm);
	Set_Harvest_Index_Names(bm);

	if(strlen(bm->fishprmIfname) == 0){
		quit("Harvest_Init. Fisheries model is turned on but no input file has been provided\n");
	}
	/* Build the converted filename */
	snprintf(convertedXMLFileName, sizeof(convertedXMLFileName), "%s", bm->fishprmIfname);
	*(strstr(convertedXMLFileName, ".prm")) = '\0';
	strcat(convertedXMLFileName, ".xml");

	/* Convert the input file to XML */
	Convert_Harvest_To_XML(bm, bm->fishprmIfname, convertedXMLFileName);

	printf("Start reading fisheries parameters from %s.\n", convertedXMLFileName);
    
    /* Initialise mFC_end_age_id so can do check in Get_Fishing_Mortality() */
    for (sp = 0; sp < bm->K_num_tot_sp; sp++) {
        for (i = 0; i < bm->K_num_fisheries; i++) {
            bm->SP_FISHERYprms[sp][i][mFC_end_age_id] = FunctGroupArray[sp].numCohorts;
        }
    }
 
	/* Do the first pass of the input file */
	Read_Harvest_Parameters(bm, convertedXMLFileName);

	/* Constant selectivity parameters */
	for (sp = 0; sp < bm->K_num_tot_sp; sp++) {
		for (i = 0; i < bm->K_num_fisheries; i++) {
			for (b = 0; b < FunctGroupArray[sp].numStages; b++) {
				if (FunctGroupArray[sp].groupAgeType == BIOMASS && FunctGroupArray[sp].isImpacted == TRUE) {
					bm->selectivity[sp][i][b] = bm->SP_FISHERYprms[sp][i][sel_id];
				}
			}

			/* Reset checks */
			bm->FISHERYprms[i][reset_id] = 1;
		}
	}


	for (nf = 0; nf < bm->K_num_fisheries; nf++) {
		if (bm->FISHERYprms[nf][flagchangeSEL_id] && !bm->flagchangesel) {
			warn("As flagchangeSEL > 0 for a fishery then setting general flagchangesel = 1\n");
			bm->flagchangesel = 1;
		}

		if (bm->FISHERYprms[nf][flagchangeP_id] && !bm->flagchangep) {
			warn("As flagchangeP > 0 for a fishery then setting general flagchangep = 1\n");
			bm->flagchangep = 1;
		}

		if (bm->FISHERYprms[nf][flagchangeSWEPT_id] && !bm->flagchangeswept) {
			warn("As flagchangeSWEPT > 0 for a fishery then setting general flagchangeswept = 1\n");
			bm->flagchangeswept = 1;
		}

		if (bm->FISHERYprms[nf][flagchangeEFF_id] && !bm->flagchangeeffort) {
			warn("As flagchangeEFF > 0 for a fishery then setting general flagchangeeffort = 1\n");
			bm->flagchangeeffort = 1;
		}
	}
	/* Load the time series input files if required */
	Load_Imposed_Catch(bm, llogfp);
	Load_Imposed_Discards(bm);

	/* Calculate the 95% and 50% vulnerability */
	Calculate_Age_Vulnerability(bm, llogfp);

	/* Setup the linkage between the fisheries and the functional groups */
	Build_Fishery_Species_Linkage(bm, llogfp);
	Build_Fishery_Tracers(bm);

    bm->EffortModelsActive = 0;
	for (nf = 0; nf < bm->K_num_fisheries; nf++) {
		for (sp = 0; sp < bm->K_num_tot_sp; sp++) {
			if(FunctGroupArray[sp].groupType == MAMMAL){
				int flagF = (int) (bm->SP_FISHERYprms[sp][nf][flagF_id]);
				int flagimposecatch = (int) (bm->SP_FISHERYprms[sp][nf][flagimposecatch_id]);

				if(flagimposecatch || flagF)
					continue;

				if(FisherySpeciesCatchFlags[sp][nf] == TRUE && (int) (bm->FISHERYprms[nf][selcurve_id]) != 2){
					warn("Fishery %s, Species %s, The selectivity curve is tailing off for large animals, but mammals often get tangled even when large fish don't, you may want to modify this curve if you are concerned by mammal bycatch\n",
							FisheryArray[nf].fisheryCode, FunctGroupArray[sp].groupCode);
				}
			}
		}
        
        if((bm->FISHERYprms[nf][flageffortmodel_id] > 0) || (bm->FISHERYprms[nf][EffortLevel_id] > 0)) {
            bm->EffortModelsActive = 1;
        }
	}

}

/**
 * \brief Free the harvest library.
 *
 *
 */
void Harvest_Free(MSEBoxModel *bm) {

	if (!bm->flag_fisheries_on) {
		return;
	}

	i_free2d(FisherySpeciesCatchFlags);

	/* Harvest Index arrays */
	c_free2d(harvestindxNAME);
	free2d(harvestindx);
	free3d(TotCumCatch);

	Close_Harvest_Output_Files(bm);

	/* Free the arrays */
	free3d(bm->selectivity);
	free2d(k_cover);
	free3d(FFCDR);
	free2d(CatchSum);
    free2d(OldCatchSum);
	i_free1d(checkedBox);

	free3d(SELchange);
	free3d(Pchange);
	free3d(SWEPTchange);
	free4d(Qchange);
	free4d(mFCchange);
	free4d(DISCRDchange);

	Harvest_Free_Time_Series(tsCatch, ntsCatch);
	i_free1d(tscatchid);
	i_free1d(tsdiscardid);

}

/**
 *	\brief Allocate memory for the diagnostic arrays.
 *
 *
 */
void Harvest_Allocate_FStat_Arrays(MSEBoxModel *bm) {

	int b, i;
	int fgIndex;
	int size = 1; /* Gear conflict */

	/* Work out how many tracers there will be - this will be max value possible.
	 * The actual tracers that a used is based on the relationship between fisheries and species.
	 * If there is no chance that a fishery can fish or impact a species then its not included
	 */
	for (fgIndex = 0; fgIndex < bm->K_num_tot_sp; fgIndex++) {

		if (FunctGroupArray[fgIndex].isVertebrate) {
			size += (FunctGroupArray[fgIndex].numCohortsXnumGenes * 2); /* Discards + Catch */
		}

		if (FunctGroupArray[fgIndex].isFished == TRUE) {
			size += 1; /* Tot Catch */
			size += bm->K_num_fisheries; /* Catch per fishery*/
		}

		if (FunctGroupArray[fgIndex].isImpacted == TRUE) {
			size += 2; /* discards  + total rec catch*/
			size += bm->K_num_fisheries; /* Discards per fishery*/

		}
	}
	size += (2 * bm->K_num_fisheries); /* Tot Effort and Avg Catch size*/

	bm->nfstat = size;

	/* Allocate memory for diagnostics */
	bm->fishstat = Util_Alloc_Init_2D_Double(bm->nfstat, bm->nbox, 0.0);

	/* Set up pointers to data from each box */
	for (b = 0; b < bm->nbox; b++) {
		bm->boxes[b].fishstat = bm->fishstat[b];

		/* Initialise values */
		for (i = 0; i < bm->nfstat; i++) {
			bm->fishstat[b][i] = 0.0;
		}
	}
}

/**
 * \brief Build the structure of the fishery tracers based on the names provided in the input fishery definition file.
 */
void Build_Fishery_Tracers(MSEBoxModel *bm) {
	int fgIndex, cohort, nf;
	int diagIndex = 0;
	char name[BMSLEN];
	char longName[BMSLEN];
	int FCindex, recCount;

	/* Allocate space for statistics info */
	if ((bm->finfo = (FstatInfo *) malloc((long unsigned int)bm->nfstat * sizeof(FstatInfo))) == NULL)
		quit("atHarvest - Build_Fishery_Tracers: Can't allocate memory for info\n");

	/* Clear tracer info */
	memset(bm->finfo, 0, (long unsigned int)bm->nfstat * (int)sizeof(FstatInfo));

	/* Catch tracers */
	for (fgIndex = 0; fgIndex < bm->K_num_tot_sp; fgIndex++) {
		if (FunctGroupArray[fgIndex].isVertebrate == TRUE) {
			for (cohort = 0; cohort < FunctGroupArray[fgIndex].numCohortsXnumGenes; cohort++) {

				snprintf(name, sizeof(name), "%s%d_Catch", FunctGroupArray[fgIndex].name, cohort + 1);
				snprintf(longName, sizeof(longName), "Catch of age class %d %s", cohort + 1, FunctGroupArray[fgIndex].fullName);

				if( FunctGroupArray[fgIndex].numGeneTypes > 1 )
					snprintf(longName, sizeof(longName), "Catch of age class by gene type %d %s", cohort + 1, FunctGroupArray[fgIndex].fullName);

				Ecology_Add_FStat_Tracer(bm, diagIndex++, name, longName, "Numbers", 0, 3, &FunctGroupArray[fgIndex].CatchTracers[cohort], 0, 2, 0);
			}
		}
	}

	/* Discard tracers */
	for (fgIndex = 0; fgIndex < bm->K_num_tot_sp; fgIndex++) {
		if (FunctGroupArray[fgIndex].isVertebrate == TRUE) {
			for (cohort = 0; cohort < FunctGroupArray[fgIndex].numCohortsXnumGenes; cohort++) {

				snprintf(name, sizeof(name), "%s%d_Discards", FunctGroupArray[fgIndex].name, cohort + 1);
				snprintf(longName, sizeof(longName), "Discards of age class %d %s", cohort + 1, FunctGroupArray[fgIndex].fullName);

				if( FunctGroupArray[fgIndex].numGeneTypes > 1 )
					snprintf(longName, sizeof(longName), "Discards of age class by gene type %d %s", cohort + 1, FunctGroupArray[fgIndex].fullName);

				Ecology_Add_FStat_Tracer(bm, diagIndex++, name, longName, "Numbers", 0, 3, &FunctGroupArray[fgIndex].DiscardTracers[cohort], 0, 2, 0);

			}
		}
	}

	/* Species caught by fishery tracers */
	for (fgIndex = 0; fgIndex < bm->K_num_tot_sp; fgIndex++) {
		if (FunctGroupArray[fgIndex].isFished == TRUE) {

			for (nf = 0; nf < bm->K_num_fisheries; nf++) {

				if (FisherySpeciesCatchFlags[fgIndex][nf] == TRUE) {

					snprintf(name, sizeof(name), "%s_Catch_FC%d", FunctGroupArray[fgIndex].groupCode, nf + 1);
					snprintf(longName, sizeof(longName), "Total catch of %s by fishery %d (%s)", FunctGroupArray[fgIndex].groupCode, nf + 1, FisheryArray[nf].fisheryCode);

					Ecology_Add_FStat_Tracer(bm, diagIndex++, name, longName, "Tonnes", 0, 3, &FunctGroupArray[fgIndex].CaughtByFisheryTracers[nf], 0, 2, 0);
				}
			}
		}
	}
	/* Species discarded by fishery tracers */
	for (fgIndex = 0; fgIndex < bm->K_num_tot_sp; fgIndex++) {
		if (FunctGroupArray[fgIndex].isImpacted == TRUE) {
			for (nf = 0; nf < bm->K_num_fisheries; nf++) {

				if (FisherySpeciesCatchFlags[fgIndex][nf] == TRUE) {

					snprintf(name, sizeof(name), "%s_Discard_FC%d", FunctGroupArray[fgIndex].groupCode, nf + 1);
					snprintf(longName, sizeof(longName), "Total discards of %s by fishery %d (%s)", FunctGroupArray[fgIndex].groupCode, nf + 1, FisheryArray[nf].fisheryCode);

					Ecology_Add_FStat_Tracer(bm, diagIndex++, name, longName, "Tonnes", 0, 3, &FunctGroupArray[fgIndex].DiscardedByFisheryTracers[nf], 0, 2, 0);
				}
			}
		}
	}
	/* The total catch tracer */
	for (fgIndex = 0; fgIndex < bm->K_num_tot_sp; fgIndex++) {
		if (FunctGroupArray[fgIndex].isFished == TRUE) {
			snprintf(name, sizeof(name), "Tot_%s_Catch", FunctGroupArray[fgIndex].groupCode);
			snprintf(longName, sizeof(longName), "Total catch of %s", FunctGroupArray[fgIndex].fullName);

			Ecology_Add_FStat_Tracer(bm, diagIndex++, name, longName, "Tonnes", 0, 1, &FunctGroupArray[fgIndex].totCatchTracer, 0, 2, 0);
		}
	}
	/* The total REC catch tracer */
	for (fgIndex = 0; fgIndex < bm->K_num_tot_sp; fgIndex++) {
		if (FunctGroupArray[fgIndex].isImpacted == TRUE) {
			snprintf(name, sizeof(name), "Tot_%s_RecCatch", FunctGroupArray[fgIndex].groupCode);
			snprintf(longName, sizeof(longName), "Total recreational catch of %s", FunctGroupArray[fgIndex].fullName);

			Ecology_Add_FStat_Tracer(bm, diagIndex++, name, longName, "Tonnes", 0, 1, &FunctGroupArray[fgIndex].totRecCatchTracer, 0, 2, 0);
		}
	}
	/* The total Discards catch tracer */
	for (fgIndex = 0; fgIndex < bm->K_num_tot_sp; fgIndex++) {
		if (FunctGroupArray[fgIndex].isImpacted == TRUE) {
			snprintf(name, sizeof(name), "Tot_%s_Discards", FunctGroupArray[fgIndex].groupCode);
			snprintf(longName, sizeof(longName), "Total discards of %s", FunctGroupArray[fgIndex].fullName);

			Ecology_Add_FStat_Tracer(bm, diagIndex++, name, longName, "Tonnes", 0, 1, &FunctGroupArray[fgIndex].totDiscardsTracer, 0, 2, 0);
		}
	}

	/* The total Effort catch tracer */
	FCindex = 0;
	/* If there is only one rec fishery then leave things as they are. Otherwise print out a tracer per rec fishery */
	recCount = 0;
	for (nf = 0; nf < bm->K_num_fisheries; nf++) {
		if(FisheryArray[nf].isRec == TRUE){
			recCount++;
		}
	}
	for (nf = 0; nf < bm->K_num_fisheries; nf++) {
		if (FisheryArray[nf].isRec == TRUE) {
			if(recCount > 1){
				snprintf(name, sizeof(name), "Tot_Effort_Recfish%d", FCindex + 1);
				snprintf(longName, sizeof(longName), "Total effort of recreational fishery %d (%s)", FCindex + 1, FisheryArray[nf].fisheryCode);
				FCindex++;
			}else{
				snprintf(name, sizeof(name), "Tot_Effort_Recfish");
				snprintf(longName, sizeof(longName), "Total effort of recreational fishery");
			}
		} else {
			snprintf(name, sizeof(name), "Tot_Effort_FC%d", FCindex + 1);
			snprintf(longName, sizeof(longName), "Total effort of fishery %d (%s)", FCindex + 1, FisheryArray[nf].fisheryCode);
			FCindex++;
		}
		Ecology_Add_FStat_Tracer(bm, diagIndex++, name, longName, "Days", 0, 1, &FisheryArray[nf].totalEffortTracer, 0, 2, 0);
	}
	/* The average catch tracer */
	FCindex = 0;
	for (nf = 0; nf < bm->K_num_fisheries; nf++) {
		if (FisheryArray[nf].isRec == TRUE) {
			if(recCount > 1){
				snprintf(name, sizeof(name), "Avg_Catch_Sze_Recfish%d", FCindex + 1);
				snprintf(longName, sizeof(longName), "Total effort of recreational fishery %d (%s)", FCindex + 1, FisheryArray[nf].fisheryCode);
				FCindex++;
			}else{
				snprintf(name, sizeof(name), "Avg_Catch_Sze_Recfish");
				snprintf(longName, sizeof(longName), "Total effort of recreational fishery");
			}
		} else {
			snprintf(name, sizeof(name), "Avg_Catch_Sze_FC%d", FCindex + 1);

			/* This should be different but is not!*/
			snprintf(longName, sizeof(longName), "Total effort of fishery %d (%s)", FCindex + 1, FisheryArray[nf].fisheryCode);
			FCindex++;
		}
		Ecology_Add_FStat_Tracer(bm, diagIndex++, name, longName, "mg N", 0, 1, &FisheryArray[nf].averageCatchSizeTracer, 0, 2, 0);
	}

	/* Add the gear conflict tracer */
	Ecology_Add_FStat_Tracer(bm, diagIndex++, "Gear_Conflict", "Gear Conflict Index", "mg N", 0, 1, &bm->conflict_id, 0, 2, 0);

}

/**
 *
 * \brief Allocate the memory needed by the harvest module.
 *
 * These arrays are freed in Harvest_Free().
 *
 */
void Allocate_Harvest_Memory(MSEBoxModel *bm) {
	int max_year = (int)(bm->tstop / (365.0 * 86400));
	max_year++;  // So can allow for +1 steps in some of the annual updating code

	/* If fisheries isn't on don't allocate memory that we won't use */
	if (!bm->flag_fisheries_on) {
		return;
	}

	//printf("Creating harvest arrays\n");

	FisherySpeciesCatchFlags = Util_Alloc_Init_2D_Int(bm->K_num_fisheries, bm->K_num_tot_sp, 0);
	/* Harvest Index*/
	harvestindxNAME = (char **) c_alloc2d(20, K_num_harvest_indx);
	harvestindx = Util_Alloc_Init_2D_Double(K_num_harvest_indx, bm->K_num_fisheries, 0.0);
	TotCumCatch = Util_Alloc_Init_3D_Double(max_year, bm->K_num_fisheries, bm->K_num_tot_sp, 0.0);

	/* Allocate the arrays*/
	k_cover = Util_Alloc_Init_2D_Double(bm->nbox, bm->K_num_fisheries, 0.0);
	bm->selectivity = Util_Alloc_Init_3D_Double(bm->K_num_max_stages, bm->K_num_fisheries, bm->K_num_tot_sp, 0.0);

	FFCDR = Util_Alloc_Init_3D_Double(bm->K_num_max_cohort * bm->K_num_max_genetypes, bm->K_num_fisheries, bm->K_num_tot_sp, 0.0);

	CatchSum = Util_Alloc_Init_2D_Double(3, bm->K_num_tot_sp, 0.0);
	checkedBox = (int *) i_alloc1d(bm->nbox);

    OldCatchSum = Util_Alloc_Init_2D_Double ( 3, bm->K_num_tot_sp , 0.0);
    qSTOCK = Util_Alloc_Init_3D_Double(bm->K_num_max_stages, bm->K_num_stocks_per_sp, bm->K_num_tot_sp, 1.0);

	/* Time series catch and discard data */
	tscatchid = Util_Alloc_Init_1D_Int(bm->K_num_tot_sp, 0);
	tsdiscardid = Util_Alloc_Init_1D_Int(bm->K_num_tot_sp, 0);

}

/**
 * \brief Work out which species are fished by which fisheries.
 *
 *
 *
 */
void Build_Fishery_Species_Linkage(MSEBoxModel *bm, FILE *llogfp) {
	int sp, nf;
	int flagimposecatch, flagF;
	double q;

	for (nf = 0; nf < bm->K_num_fisheries; nf++) {
		for (sp = 0; sp < bm->K_num_tot_sp; sp++) {

			if (FunctGroupArray[sp].isFished == TRUE) {

				/* Default is that this species is not fished by this fishery */
				FisherySpeciesCatchFlags[sp][nf] = FALSE;

				/* First test for imposed catch */
				flagimposecatch = (int) (bm->SP_FISHERYprms[sp][nf][flagimposecatch_id]);

				/* Check for use of fishing mortalities */
				flagF = (int) (bm->SP_FISHERYprms[sp][nf][flagF_id]);

				/* Dynamic catch - catability is used to test for this */
				q = bm->SP_FISHERYprms[sp][nf][q_id];

				//printf("species %d, nf = %d, flagimposecatch = %d, flagF = %d, q = %e\n", sp, nf, flagimposecatch, flagF, q);

				if (flagimposecatch > 0 || flagF > 0 || q > 0) {
					FisherySpeciesCatchFlags[sp][nf] = TRUE;
					//printf("FisherySpeciesCatchFlags[%d][%d] = %d\n", sp, nf, FisherySpeciesCatchFlags[sp][nf]);
				}

				/* Nasty special case - leave in for now untill i talk to Beth *
				 * REMOVED SPECIAL CASES AS FISHERIES NO LONGER MAP TO HARD NUMBERS
				if (FunctGroupArray[sp].habitatType == EPIFAUNA && nf == 21)
					FisherySpeciesCatchFlags[sp][nf] = TRUE;

				if (FunctGroupArray[sp].groupType == PHYTOBEN && nf == 31)
					FisherySpeciesCatchFlags[sp][nf] = TRUE;
				if ((FunctGroupArray[sp].groupType == LG_ZOO  || FunctGroupArray[sp].groupType == MED_ZOO )&& nf == 27)
					FisherySpeciesCatchFlags[sp][nf] = TRUE;
				*/
			}
		}
	}
}

/*
 * \brief Setup the names of the fishery parameters.
 *
 *
 *
 */
void Setup_Fishery_Param_Strings(MSEBoxModel *bm) {
	int i;
    int paramlen = 25;

	for (i = 0; i < tot_fisheries_prms; i++) {
		snprintf(bm->fisheryParamNAME[i], paramlen, "%s", "");
	}

	snprintf(bm->fisheryParamNAME[flagdempelfishery_id], paramlen, "%s", "_flagdempelfishery");
	snprintf(bm->fisheryParamNAME[tStart_id], paramlen, "%s", "_tStart");
	snprintf(bm->fisheryParamNAME[tEnd_id], paramlen, "%s", "_tEnd");
	snprintf(bm->fisheryParamNAME[start_manage_id], paramlen, "%s", "_start_manage");
	snprintf(bm->fisheryParamNAME[end_manage_id], paramlen, "%s", "_end_manage");
	snprintf(bm->fisheryParamNAME[flageffortmodel_id], paramlen, "%s", "_effortmodel");
	snprintf(bm->fisheryParamNAME[flagexplore_id], paramlen, "%s", "_explore");
	snprintf(bm->fisheryParamNAME[flagdropEFF_id], paramlen, "%s", "_effortdrop");
	snprintf(bm->fisheryParamNAME[selcurve_id], paramlen, "%s", "_selcurve");
	snprintf(bm->fisheryParamNAME[flagmanage_id], paramlen, "%s", "_flagmanage");
	snprintf(bm->fisheryParamNAME[flagTACpartipcate_id], paramlen, "%s", "flagTACparticipate_");

	snprintf(bm->fisheryParamNAME[flagcap_id], paramlen, "%s", "_flagcap");
	snprintf(bm->fisheryParamNAME[flagcap_peak_id], paramlen, "%s", "_flagcap_peak");
	snprintf(bm->fisheryParamNAME[flaguseall_id], paramlen, "%s", "_flaguseall");

	snprintf(bm->fisheryParamNAME[flagseasonal_id], paramlen, "%s", "_flagseasonal");
    snprintf(bm->fisheryParamNAME[flag_framebased_id], paramlen, "%s", "_flag_framebased");
	snprintf(bm->fisheryParamNAME[flagmpa_id], paramlen, "%s", "_flagmpa");
	snprintf(bm->fisheryParamNAME[flagchangeSEL_id], paramlen, "%s", "_changeSEL");

	snprintf(bm->fisheryParamNAME[flagchangeP_id], paramlen, "%s", "_changeP");
	snprintf(bm->fisheryParamNAME[flagchangeSWEPT_id], paramlen, "%s", "_changeSWEPT");
	snprintf(bm->fisheryParamNAME[flagchangeEFF_id], paramlen, "%s", "_changeEFF");
	snprintf(bm->fisheryParamNAME[flagchangeseason_id], paramlen, "%s", "_changeseason");
	snprintf(bm->fisheryParamNAME[TACchange_id], paramlen, "%s", "_TACchange");
	snprintf(bm->fisheryParamNAME[minFCdepth_id], paramlen, "%s", "_mindepth");
	snprintf(bm->fisheryParamNAME[maxFCdepth_id], paramlen, "%s", "_maxdepth");
	snprintf(bm->fisheryParamNAME[sel_b_id], paramlen, "%s", "sel_b");

	snprintf(bm->fisheryParamNAME[sel_lsm_id], paramlen, "%s", "sel_lsm");
	snprintf(bm->fisheryParamNAME[sel_normsigma_id], paramlen, "%s", "sel_normsigma");
	snprintf(bm->fisheryParamNAME[sel_normlsm_id], paramlen, "%s", "sel_normlsm");
	snprintf(bm->fisheryParamNAME[sel_lognormlsm_id], paramlen, "%s", "sel_lognormlsm");
	snprintf(bm->fisheryParamNAME[sel_lognormsigma_id], paramlen, "%s", "sel_lognormsigma");
	snprintf(bm->fisheryParamNAME[sel_gammasigma_id], paramlen, "%s", "sel_gammasigma");
	snprintf(bm->fisheryParamNAME[sel_gammalsm_id], paramlen, "%s", "sel_gammalsm");
	snprintf(bm->fisheryParamNAME[sel_bisigma_id], paramlen, "%s", "sel_bisigma");
	snprintf(bm->fisheryParamNAME[sel_bisigma2_id], paramlen, "%s", "sel_bisigma2");
	snprintf(bm->fisheryParamNAME[sel_ampli_id], paramlen, "%s", "sel_ampli");

	snprintf(bm->fisheryParamNAME[sel_bilsm1_id], paramlen, "%s", "sel_bilsm1");
	snprintf(bm->fisheryParamNAME[sel_bilsm2_id], paramlen, "%s", "sel_bilsm2");
	snprintf(bm->fisheryParamNAME[swept_area_id], paramlen, "%s", "_sweptarea");
	snprintf(bm->fisheryParamNAME[maxsaleprice_id], paramlen, "%s", "_maxsaleprice");
	snprintf(bm->fisheryParamNAME[mEff_max_id], paramlen, "%s", "_mEff_max");
	snprintf(bm->fisheryParamNAME[mEff_a_id], paramlen, "%s", "_mEff_a");
	snprintf(bm->fisheryParamNAME[mEff_offset_id], paramlen, "%s", "_mEff_offset");
	snprintf(bm->fisheryParamNAME[mEff_testfish_id], paramlen, "%s", "_mEff_testfish");

	snprintf(bm->fisheryParamNAME[mEff_thresh_id], paramlen, "%s", "_mEff_thresh");
	snprintf(bm->fisheryParamNAME[mEff_thresh_top_id], paramlen, "%s", "_mEff_thresh_top");
	snprintf(bm->fisheryParamNAME[mEff_shift_id], paramlen, "%s", "_mEff_shift");
	snprintf(bm->fisheryParamNAME[mFCscale_id], paramlen, "%s", "_mFCscale");
	snprintf(bm->fisheryParamNAME[EffortLevel_id], paramlen, "%s", "_Effort");
	snprintf(bm->fisheryParamNAME[CPUE_effort_thresh_id], paramlen, "%s", "CPUEthresh");
	snprintf(bm->fisheryParamNAME[CPUE_effort_scale_id], paramlen, "%s", "CPUEscale");
	snprintf(bm->fisheryParamNAME[cap_id], paramlen, "%s", "_cap");
	snprintf(bm->fisheryParamNAME[seasonopen_id], paramlen, "%s", "_seasonopen");
	snprintf(bm->fisheryParamNAME[seasonclose_id], paramlen, "%s", "_seasonclose");
	snprintf(bm->fisheryParamNAME[mFCpeak_id], paramlen, "%s", "_mFCpeak");

	snprintf(bm->fisheryParamNAME[use_min_lever_id], paramlen, "%s", "_use_min_lever");
	snprintf(bm->fisheryParamNAME[landallTAC_sp_id], paramlen, "%s", "_landallTAC_sp");
	snprintf(bm->fisheryParamNAME[max_num_sp_id], paramlen, "%s", "_max_num_sp");
	snprintf(bm->fisheryParamNAME[infringe_id], paramlen, "%s", "_infringe");
	snprintf(bm->fisheryParamNAME[FC_restrict_id], paramlen, "%s", "_FC_restrict");
	snprintf(bm->fisheryParamNAME[FC_period_id], paramlen, "%s", "_FC_period");
	snprintf(bm->fisheryParamNAME[FC_period2_id], paramlen, "%s", "_FC_period2");

	snprintf(bm->fisheryParamNAME[FC_restrict_endangered_id], paramlen, "%s", "_FC_restrict_endangered");
	snprintf(bm->fisheryParamNAME[FC_endanger_period_id], paramlen, "%s", "_FC_endanger_period");
	snprintf(bm->fisheryParamNAME[EFF_num_changes_id], paramlen, "%s", "_num_changes");
	snprintf(bm->fisheryParamNAME[CAP_num_changes_id], paramlen, "%s", "_cap_changes");
	snprintf(bm->fisheryParamNAME[SEL_num_changes_id], paramlen, "%s", "_sel_changes");
	snprintf(bm->fisheryParamNAME[SWEPT_num_changes_id], paramlen, "%s", "_swept_changes");

	snprintf(bm->fisheryParamNAME[P_num_changes_id], paramlen, "%s", "_p_changes");
	snprintf(bm->fisheryParamNAME[fisheriesflagactive_id], paramlen, "%s", "flag");
	snprintf(bm->fisheryParamNAME[flagDV_id], paramlen, "%s", "_flagDV");
	snprintf(bm->fisheryParamNAME[nsubfleets_id], paramlen, "%s", "_nsubfleets");
	snprintf(bm->fisheryParamNAME[nlicence_id], paramlen, "%s", "_nlicence");
	snprintf(bm->fisheryParamNAME[flagMultiSpEffort_id], paramlen, "%s", "_flagMultiSpEffort");
	snprintf(bm->fisheryParamNAME[buybackdate_id], paramlen, "%s", "_buybackdate");

}
/**
 *
 * \brief Calculate the age50 and age95 value for each species.
 *
 */
void Calculate_Age_Vulnerability(MSEBoxModel *bm, FILE *llogfp) {

	double Wgt, li, sel;
	int sp, nf, age50, age95, chrt, sn, rn, sel_curve, stage, basechrt;

	/* Calculate age of 50% and 95% capture defaults */
	for (sp = 0; sp < bm->K_num_tot_sp; sp++) {
		if (FunctGroupArray[sp].groupAgeType == AGE_STRUCTURED) {
			if(FunctGroupArray[sp].speciesParams[flag_id]){
				age50 = FunctGroupArray[sp].numCohorts - 1;
				age95 = FunctGroupArray[sp].numCohorts - 1;
				for (chrt = 0; chrt < FunctGroupArray[sp].numCohortsXnumGenes; chrt++) {
					basechrt = chrt / FunctGroupArray[sp].numGeneTypes;
					stage = FunctGroupArray[sp].cohort_stage[chrt];
					sn = FunctGroupArray[sp].structNTracers[chrt];
					rn = FunctGroupArray[sp].resNTracers[chrt];

					Wgt = bm->boxes[0].tr[0][sn] + bm->boxes[0].tr[0][rn];
					li = Ecology_Get_Size(bm, sp, Wgt, basechrt);

					for (nf = 0; nf < bm->K_num_fisheries; nf++) {

						sel_curve = (int) (bm->FISHERYprms[nf][selcurve_id]);

						sel = Get_Selectivity(bm, sp, stage, nf, li, sel_curve, 0.0, 0.0);

						/* 50% vulnerability to the fisheries */
						if ((sel > 0.50) && (basechrt < age50)) {
							FunctGroupArray[sp].speciesParams[Age50pcntV_id] = chrt;
							age50 = basechrt;
						}
						/* 95% vulnerability to the fisheries */
						if ((sel > 0.95) && (basechrt < age95)) {
							FunctGroupArray[sp].speciesParams[Age95pcntV_id] = chrt;
							age95 = basechrt;
						}
					}
				}
				if (verbose) {
					fprintf(llogfp, "For %s setting Vage50 = %e, Vage95 = %e\n", FunctGroupArray[sp].groupCode, FunctGroupArray[sp].speciesParams[Age50pcntV_id],
							FunctGroupArray[sp].speciesParams[Age95pcntV_id]);
				}
			}
		}
	}
}
