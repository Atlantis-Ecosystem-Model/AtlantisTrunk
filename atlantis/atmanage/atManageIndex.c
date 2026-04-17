/**
 * \ingroup atManageLib
 * \file atManageIndex.c
 * \brief Functions to calculate and write out performance measures.
 * \author Beth Fulton 16/5/2006
 *
 *
 * <b>Revisions:</b>
 *
 *
 * 01/12/2008 Bec Gorton
 * Moved the management and fisheries performance measures output file into the
 * atManageModule structure so that it can be correctly closed at the end of the
 * model run.
 *
 * 03-11-2009 Bec Gorton
 * Removed commented out code that has been moved into the harvest or managementImplementation
 * modules.
 *
 * 27-01-2010 Bec Gorton
 * Removed direct references to the harvestindx array. This is now private within the harvest lib so
 * is accessed via functions defined in atHarvestLib.h
 *
 * 01-02-2010 Bec Gorton
 * Moved the management index and index name arrays into the management library. These are now private to this
 * lib but can be set/get by public functions prototyped in atManageLib.h
 *
 */

/*  Open library routines */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "atManage.h"
#include "atHarvestLib.h"

/* File containing management and fisheries performance measures */
static FILE *annhmindxfp;
char **manageindxNAME; /**< name of management performance measures */
double **manageindx; /**< management performance measures */

static void Write_Performance_Measures(FILE *fid, MSEBoxModel *bm);
static FILE * Init_Performance_Measures_File(MSEBoxModel *bm);
/**
 * \brief  This sets up names of fisheries management performance indices
 */
void Set_Manage_Index_Names(MSEBoxModel *bm) {
    int paramLen = 20;
    
	snprintf(manageindxNAME[mgmtstability_id], paramLen, "%s", "mgmtStability");
	snprintf(manageindxNAME[mgmtaccess_id], paramLen, "%s", "mgmtAccess");
	snprintf(manageindxNAME[mgmtcost_id], paramLen, "%s", "mgmtCost");
	snprintf(manageindxNAME[frstindx_id], paramLen, "%s", "1stindx");

	return;
}

void Open_Management_Index_File(MSEBoxModel *bm) {

	annhmindxfp = Init_Performance_Measures_File(bm);
}

void Close_Management_Index_File(MSEBoxModel *bm) {
	/* Close the management and fisheries reporting file */
	Util_Close_Output_File(annhmindxfp);
}

void Manage_Set_Manage_Index(MSEBoxModel *bm, int nf, int index, double value) {
	manageindx[nf][index] = value;
}
double Manage_Get_Manage_Index(MSEBoxModel *bm, int nf, int index) {
	return manageindx[nf][index];
}

/**
 * \brief Routine to update management performance measures
 */

void Update_Management_Indx(MSEBoxModel *bm, FILE *llogfp) {
	int fishery_id, flagseasonal, ij, first_indx;
	double season_impact, TAC_impact, MPA_impact, TEP_impact, newindx, oldindx, mpa_enforce;

	for (fishery_id = 0; fishery_id < bm->K_num_fisheries; fishery_id++) {
		/* Access - running average of largest change in TAC for the fishery,
		 adjusted for large changes in gear and impacts of spatial management
		 */
		flagseasonal = (int) (bm->FISHERYprms[fishery_id][flagseasonal_id]);

		if (flagseasonal)
			season_impact = fabs(SEASONAL[fishery_id][0] - SEASONAL[fishery_id][1]) / 365.0;
		else
			season_impact = 1.0;
		TAC_impact = bm->TAC_trigger[fishery_id][triggered_scalar_id];
		MPA_impact = 0;
		mpa_enforce = 0;
		for (ij = 0; ij < bm->nbox; ij++) {
			MPA_impact += bm->MPA[ij][fishery_id] / bm->nbox;
			if (bm->MPA[ij][fishery_id] < 1)
				mpa_enforce++;
		}
		TEP_impact = effort_scale[fishery_id][endangered_tac_id];
		newindx = season_impact * TAC_impact * MPA_impact * TEP_impact;

		first_indx = (int) (manageindx[fishery_id][frstindx_id]);
		if (!first_indx)
			oldindx = manageindx[fishery_id][mgmtaccess_id];
		else {
			oldindx = newindx;
			manageindx[fishery_id][frstindx_id] = 0;
		}

		manageindx[fishery_id][mgmtaccess_id] = (oldindx + newindx) / 2.0;

		/* Finalise management costs */
		manageindx[fishery_id][mgmtcost_id] += mpa_enforce * bm->enforce_coefft;
	}

	return;
}

/**
 * \brief Routines to report the management and fisheries performance measures.
 */
void Manage_Output_Indices(MSEBoxModel *bm) {
	if (verbose)
		fprintf(stderr, "Writing annual catch output (instead of puting it in a logfile)\n");

	/* Initialise files if necessary */
	if (!annhmindxfp)
		quit("Open_Management_Index_File must be called before Output_Management_Indices \n");

	/* Write output */
	Write_Performance_Measures(annhmindxfp, bm);
	return;
}

/**
 * \brief Routine to initialise fishery performance measure information file
 */
FILE * Init_Performance_Measures_File(MSEBoxModel *bm) {
	FILE *fid;
	char fname[STRLEN];
	int sp;

	/** Create filename **/
	snprintf(fname, sizeof(fname), "%sHarvestIndx.txt", bm->startfname);
	printf("Creating %s\n", fname);

	/** Create file **/
	if ((fid = Util_fopen(bm, fname, "w")) == NULL)
		quit("Init_Performance_Measures_File: Can't open %s\n", fname);

	/** Column definitions **/
	fprintf(fid, "Time Fishery");

	Harvest_Init_Index_Output(bm, fid);

	/* Management performance measures */
	for (sp = 0; sp < K_num_manage_indx; sp++) {
		fprintf(fid, " %s", manageindxNAME[sp]);
	}

	fprintf(fid, "\n");

	/* Return file pointer */
	return (fid);
}
/**
 * \brief Write out the harvest and management performance indices.
 */
void Write_Performance_Measures(FILE *fid, MSEBoxModel *bm) {
	int sp, nf;

	if (verbose > 1)
		fprintf(stderr, "Write total biomass information\n");

	/* Write catch values */
	for (nf = 0; nf < bm->K_num_fisheries; nf++) {
		/* Write time */
		fprintf(fid, "%e %s", bm->dayt, FisheryArray[nf].fisheryCode);

		/* Write out the harvest performance measures */
		Harvest_Write_Harvest_Index(bm, nf, fid);

		/* Management performance measures */
		for (sp = 0; sp < K_num_manage_indx; sp++) {
			fprintf(fid, " %e", manageindx[nf][sp]);
		}

		fprintf(fid, "\n");
	}

	return;
}

