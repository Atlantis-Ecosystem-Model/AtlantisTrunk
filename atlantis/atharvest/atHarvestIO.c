/**
 \ingroup atHarvestLib
 \file  atHarvestIO.c
 \brief Functions that handle the fisheries management summary output.
 \author	Beth Fulton and Bec Gorton


 <b>Revisions:</b>

 20/7/2006 Beth Fulton
 Created from the write routines once stored in atannualharvest.c file


 28/11/2008 Bec Gorton
 Moved the static files into the harvestModule so that they can be freed at the
 end of the model run.

 28-10-2009 Bec Gorton
 Removed references to the K_num_fished_sp, K_num_fishedtac_sp and K_num_impacted_sp.
 Arrays that were this size have been resized to K_num_tot_sp.
 Also changed references to the arrays that have been moved into one of the modules.

 02-11-2009 Bec Gorton
 Removed more references to K_num_fished and K_num_impacted.

 **************/

/*  Open library routines */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "atHarvest.h"
#include "atHarvestIO.h"

/**
 * @name Annual catch output files. These are opened in Report_Annual_Fisheries_Stats
 * and closed in freeManageStruct
 */
//@{
FILE *anncatchfp;
FILE *annreccatchfp;
FILE *anncpffp;
FILE *anndpffp;
FILE *anndiscardfp;
FILE *anneffortfp;
FILE *displaceEffortfp;

static void writeAnnCatch(FILE *fid, FILE *fid2, FILE *fid3, MSEBoxModel *bm, FILE *llogfp);
static void writeAnnCatchPerFishery(FILE *fid, MSEBoxModel *bm, FILE *llogfp);
static void writeAnnDiscardPerFishery(FILE *fid, MSEBoxModel *bm, FILE *llogfp);
static void writeAnnEffort(FILE *fid, MSEBoxModel *bm, FILE *llogfp);
static void writeDisplaceEffort(FILE *fid, MSEBoxModel *bm, FILE *llogfp);

static FILE * initAnnCatchFile(MSEBoxModel *bm);
static FILE * initAnnCatchPerFisheryFile(MSEBoxModel *bm);
static FILE * initAnnDiscardPerFisheryFile(MSEBoxModel *bm);
static FILE * initAnnDiscardFile(MSEBoxModel *bm);
static FILE * initAnnEffortFile(MSEBoxModel *bm);
static FILE * initAnnRecCatchFile(MSEBoxModel *bm);
static FILE * initDisplacedEffortFile(MSEBoxModel *bm);

/*******************************************************************************************
 Routines to report the aggregate fisheries statistics to date - done on an annual basis,
 as only called from Annual_Fisheries_Mgmt()
 */

void Harvest_Report_Annual_Stats(MSEBoxModel *bm, FILE *llogfp) {

	if (verbose)
		fprintf(stderr, "Writing annual catch and effort output (instead of puting it in a logfile)\n");

	/* Initialise files if necessary */
	if (!anncatchfp)
		anncatchfp = initAnnCatchFile(bm);
	if (!annreccatchfp)
		annreccatchfp = initAnnRecCatchFile(bm);
	if (!anndiscardfp)
		anndiscardfp = initAnnDiscardFile(bm);
	if (!anncpffp)
		anncpffp = initAnnCatchPerFisheryFile(bm);
	if (!anndpffp)
		anndpffp = initAnnDiscardPerFisheryFile(bm);
	if (!anneffortfp)
		anneffortfp = initAnnEffortFile(bm);

	/* Write output */
	writeAnnCatch(anncatchfp, anndiscardfp, annreccatchfp, bm, llogfp);
    if(bm->thisyear)
        writeAnnCatchPerFishery(anncpffp, bm, llogfp);
    writeAnnDiscardPerFishery(anndpffp, bm, llogfp);
    writeAnnEffort(anneffortfp, bm, llogfp);

	return;
}

void Harvest_Report_Monthly_Stats(MSEBoxModel *bm, FILE *llogfp) {
    
    if (!bm->flagdisplace) {
        return;  // Nothing to report
    }
    
    if (!displaceEffortfp)
        displaceEffortfp = initDisplacedEffortFile(bm);
    
    /* Write output */
    writeDisplaceEffort(displaceEffortfp, bm, llogfp);
    
    return;
}

void Open_Harvest_Output_Files(MSEBoxModel *bm) {

	anncatchfp = initAnnCatchFile(bm);
	annreccatchfp = initAnnRecCatchFile(bm);
	anndiscardfp = initAnnDiscardFile(bm);
	anncpffp = initAnnCatchPerFisheryFile(bm);
	anndpffp = initAnnDiscardPerFisheryFile(bm);
	anneffortfp = initAnnEffortFile(bm);
    
    if (bm->flagdisplace) {
        displaceEffortfp = initDisplacedEffortFile(bm);
    }
}

void Close_Harvest_Output_Files(MSEBoxModel *bm) {

	Util_Close_Output_File(anncatchfp);
	Util_Close_Output_File(annreccatchfp);
	Util_Close_Output_File(anncpffp);
	Util_Close_Output_File(anndpffp);
	Util_Close_Output_File(anndiscardfp);
	Util_Close_Output_File(anneffortfp);
    
    if (bm->flagdisplace) {
        Util_Close_Output_File(displaceEffortfp);
    }
}

/******************************************************************************************
 File initialisation routines
 */

/**
 * \brief Routine to initialise catch information file
 *
 */
FILE * initAnnCatchFile(MSEBoxModel *bm) {
	FILE *fid;
	char fname[STRLEN];
	int sp, flag_sp;

	/** Create filename **/
	sprintf(fname, "%sCatch.txt", bm->startfname);
	printf("Creating %s\n", fname);

	/** Create file **/
	if ((fid = Util_fopen(bm, fname, "w")) == NULL)
		quit("initAnnCatchFile: Can't open %s\n", fname);

	/** Column definitions **/
	fprintf(fid, "Time");

	/* Species caught name */
	for (sp = 0; sp < bm->K_num_tot_sp; sp++) {
		if (FunctGroupArray[sp].isImpacted == TRUE) {
			flag_sp = (int) (FunctGroupArray[sp].speciesParams[flag_id]);
			if (flag_sp)
				fprintf(fid, " %s", FunctGroupArray[sp].groupCode);
		}
	}

	/* Repeat species caught name for record of actual catch in ts files */
	for (sp = 0; sp < bm->K_num_tot_sp; sp++) {
		if (FunctGroupArray[sp].isImpacted == TRUE) {
			flag_sp = (int) (FunctGroupArray[sp].speciesParams[flag_id]);
			if (flag_sp)
				fprintf(fid, " TsAct%s", FunctGroupArray[sp].groupCode);
		}
	}

	fprintf(fid, "\n");
	/* Return file pointer */
	return (fid);
}

/**
 * \brief Routine to initialise recreational catch information file
 *
 */
FILE * initAnnRecCatchFile(MSEBoxModel *bm) {
	FILE *fid;
	char fname[STRLEN];
	int sp, flag_sp;

	/** Create filename **/
	sprintf(fname, "%sRecCatch.txt", bm->startfname);
	printf("Creating %s\n", fname);

	/** Create file **/
	if ((fid = Util_fopen(bm, fname, "w")) == NULL)
		quit("initAnnRecCatchFile: Can't open %s\n", fname);

	/** Column definitions **/
	fprintf(fid, "Time ");

	/* Species caught name */
	for (sp = 0; sp < bm->K_num_tot_sp; sp++) {
		if (FunctGroupArray[sp].isImpacted == TRUE) {
			flag_sp = (int) (FunctGroupArray[sp].speciesParams[flag_id]);
			if (flag_sp)
				fprintf(fid, " %s", FunctGroupArray[sp].groupCode);
		}
	}

	fprintf(fid, "\n");
	/* Return file pointer */
	return (fid);
}

/**
 * \brief Routine to initialise discard information file
 */
FILE * initAnnDiscardFile(MSEBoxModel *bm) {
	FILE *fid;
	char fname[STRLEN];
	int sp, flag_sp;

	/** Create filename **/
	sprintf(fname, "%sDiscard.txt", bm->startfname);
	printf("Creating %s\n", fname);

	/** Create file **/
	if ((fid = Util_fopen(bm, fname, "w")) == NULL)
		quit("initAnnDiscardFile: Can't open %s\n", fname);

	/** Column definitions **/
	fprintf(fid, "Time");

	/* Species caught name */
	for (sp = 0; sp < bm->K_num_tot_sp; sp++) {
		if (FunctGroupArray[sp].isImpacted == TRUE) {
			flag_sp = (int) (FunctGroupArray[sp].speciesParams[flag_id]);
			if (flag_sp)
				fprintf(fid, " %s", FunctGroupArray[sp].groupCode);
		}
	}
	fprintf(fid, "\n");

	/* Return file pointer */
	return (fid);
}

/**
 * \brief Routine to initialise catch information file
 */
FILE * initAnnCatchPerFisheryFile(MSEBoxModel *bm) {
	FILE *fid;
	char fname[STRLEN];
	int sp, flag_sp;

	/** Create filename **/
	sprintf(fname, "%sCatchPerFishery.txt", bm->startfname);
	printf("Creating %s\n", fname);

	/** Create file **/
	if ((fid = Util_fopen(bm, fname, "w")) == NULL)
		quit("initAnnCatchFile: Can't open %s\n", fname);

	/** Column definitions **/
	fprintf(fid, "Time Fishery");

	/* Species caught name */
	for (sp = 0; sp < bm->K_num_tot_sp; sp++) {
		if (FunctGroupArray[sp].isImpacted == TRUE) {
			flag_sp = (int) (FunctGroupArray[sp].speciesParams[flag_id]);
			if (flag_sp)
				fprintf(fid, " %s", FunctGroupArray[sp].groupCode);
		}
	}

	fprintf(fid, "\n");
	/* Return file pointer */
	return (fid);
}

FILE * initAnnDiscardPerFisheryFile(MSEBoxModel *bm)
/* Routine to initialise discard information file */
{
	FILE *fid;
	char fname[STRLEN];
	int sp, flag_sp;

	/** Create filename **/
	sprintf(fname, "%sDiscardPerFishery.txt", bm->startfname);
	printf("Creating %s\n", fname);

	/** Create file **/
	if ((fid = Util_fopen(bm, fname, "w")) == NULL)
		quit("initAnnDiscardFile: Can't open %s\n", fname);

	/** Column definitions **/
	fprintf(fid, "Time Fishery");

	/* Species caught name */
	for (sp = 0; sp < bm->K_num_tot_sp; sp++) {
		if (FunctGroupArray[sp].isImpacted == TRUE) {

			flag_sp = (int) (FunctGroupArray[sp].speciesParams[flag_id]);
			if (flag_sp)
				fprintf(fid, " %s", FunctGroupArray[sp].groupCode);
		}
	}

	fprintf(fid, "\n");
	/* Return file pointer */
	return (fid);
}
/**
 *
 * \brief Routine to initialise catch information file
 */
FILE * initAnnEffortFile(MSEBoxModel *bm) {
	FILE *fid;
	char fname[STRLEN];
	int nf;

	/** Create filename **/
	sprintf(fname, "%sEffort.txt", bm->startfname);
	printf("Creating %s\n", fname);

	/** Create file **/
	if ((fid = Util_fopen(bm, fname, "w")) == NULL)
		quit("initAnnEffortFile: Can't open %s\n", fname);

	/** Column definitions **/
	fprintf(fid, "Time");

	/* Fisheries name */
	for (nf = 0; nf < bm->K_num_fisheries; nf++) {
		fprintf(fid, " %s", FisheryArray[nf].fisheryCode);
	}
	fprintf(fid, "\n");

	/* Return file pointer */
	return (fid);
}
/**
 *
 * \brief Routine to initialise catch information file
 */
FILE * initAnnTACFile(MSEBoxModel *bm) {
	FILE *fid;
	char fname[STRLEN];
	int sp, flag_sp;

	/** Create filename **/
	sprintf(fname, "%sTAC.txt", bm->startfname);
	printf("Creating %s\n", fname);

	/** Create file **/
	if ((fid = Util_fopen(bm, fname, "w")) == NULL)
		quit("initAnnTACFile: Can't open %s\n", fname);

	/** Column definitions **/
	fprintf(fid, "Time");

	/* Species caught name */
	for (sp = 0; sp < bm->K_num_tot_sp; sp++) {
		if (FunctGroupArray[sp].isFished == TRUE) {
			flag_sp = (int) (FunctGroupArray[sp].speciesParams[flag_id]);
			if (flag_sp)
				fprintf(fid, " %s", FunctGroupArray[sp].groupCode);
		}
	}

	fprintf(fid, "\n");
	/* Return file pointer */
	return (fid);
}

/**
 *
 * \brief Routine to initialise displaced effort information file
 */
FILE * initDisplacedEffortFile(MSEBoxModel *bm) {
    FILE *fid;
    char fname[STRLEN];
    int nf;

    /** Create filename **/
    sprintf(fname, "%sDisplaceEffort.txt", bm->startfname);
    printf("Creating %s\n", fname);

    /** Create file **/
    if ((fid = Util_fopen(bm, fname, "w")) == NULL)
        quit("initDisplacedEffortFile: Can't open %s\n", fname);

    /** Column definitions **/
    fprintf(fid, "Time Box");

    /* Fisheries name */
    for (nf = 0; nf < bm->K_num_fisheries; nf++) {
        fprintf(fid, " %s", FisheryArray[nf].fisheryCode);
    }
    fprintf(fid, "\n");

    /* Return file pointer */
    return (fid);
}

/**
 * \brief File writing routines
 *
 */
void writeAnnCatch(FILE *fid, FILE *fid2, FILE *fid3, MSEBoxModel *bm, FILE *llogfp) {
	int sp, flag_sp, fishery_id;
	double catch;

	if (verbose > 1)
        printf( "Write total catch information\n");

	/* Write time */
	fprintf(fid, "%e", bm->dayt);
	fprintf(fid2, "%e", bm->dayt);
	fprintf(fid3, "%e", bm->dayt);

	/* Write catch values */
	for (sp = 0; sp < bm->K_num_tot_sp; sp++) {
		if (FunctGroupArray[sp].isImpacted == TRUE) {
			flag_sp = (int) (FunctGroupArray[sp].speciesParams[flag_id]);
			if (flag_sp) {
				fprintf(fid, " %f", CatchSum[sp][tscocatch_id] * bm->X_CN * mg_2_tonne);
			}
		}
	}

	/* Write actual catch values from ts files */
	for (sp = 0; sp < bm->K_num_tot_sp; sp++) {
		if (FunctGroupArray[sp].isImpacted == TRUE) {
			flag_sp = (int) (FunctGroupArray[sp].speciesParams[flag_id]);
			if (flag_sp)
				fprintf(fid, " %f", CatchSum[sp][tsactcatch_id] * bm->X_CN * mg_2_tonne);
		}
	}
	fprintf(fid, "\n");
    fflush(fid);

	/* Write discard values */
	for (sp = 0; sp < bm->K_num_tot_sp; sp++) {
		if (FunctGroupArray[sp].isImpacted == TRUE) {
			flag_sp = (int) (FunctGroupArray[sp].speciesParams[flag_id]);
			if (flag_sp)
				fprintf(fid2, " %f", CatchSum[sp][tscodiscard_id] * bm->X_CN * mg_2_tonne);
		}
	}

	fprintf(fid2, "\n");
    fflush(fid2);

	/* Write recreational fisheries catches */
	for (sp = 0; sp < bm->K_num_tot_sp; sp++) {
		if (FunctGroupArray[sp].isImpacted == TRUE) {
			flag_sp = (int) (FunctGroupArray[sp].speciesParams[flag_id]);
			if (flag_sp){
				catch = 0;
				for (fishery_id = 0; fishery_id < bm->K_num_fisheries; fishery_id++) {
					catch = catch + bm->TotCumRecCatch[sp][fishery_id];
				}
				fprintf(fid3, " %f", catch * bm->X_CN * mg_2_tonne);
			}
		}
	}
	fprintf(fid3, "\n");
    fflush(fid3);

	return;
}

void writeAnnCatchPerFishery(FILE *fid, MSEBoxModel *bm, FILE *llogfp) {
	int sp, flag_sp, fishery_id;
	double catch_2_print = 0;

	if (verbose > 1)
        printf( "Write catch per fishery information\n");
	fprintf(llogfp, "Write catch per fishery information\n");

	/* Write catch values */
	for (fishery_id = 0; fishery_id < bm->K_num_fisheries; fishery_id++) {
		fprintf(fid, "%e %s", bm->dayt, FisheryArray[fishery_id].fisheryCode);
		for (sp = 0; sp < bm->K_num_tot_sp; sp++) {
			if (FunctGroupArray[sp].isImpacted == TRUE) {
				flag_sp = (int) (FunctGroupArray[sp].speciesParams[flag_id]);
				if (flag_sp) {
					if(!FisheryArray[fishery_id].isRec){
						catch_2_print = TotCumCatch[sp][fishery_id][bm->thisyear-1] * bm->X_CN * mg_2_tonne;
						fprintf(fid, " %e", catch_2_print);
					} else {
						catch_2_print = bm->TotCumRecCatch[sp][fishery_id] * bm->X_CN * mg_2_tonne;
						fprintf(fid, " %e", catch_2_print);
					}
				}
			}
		}
		fprintf(fid, "\n");
	}

	return;
}

void writeAnnDiscardPerFishery(FILE *fid, MSEBoxModel *bm, FILE *llogfp) {
	int sp, flag_sp, fishery_id;
	double discard_2_print = 0;

	if (verbose > 1)
        printf( "Write discards per fishery information\n");

	/* Write discard values */
	for (fishery_id = 0; fishery_id < bm->K_num_fisheries; fishery_id++) {
		fprintf(fid, "%e %s", bm->dayt, FisheryArray[fishery_id].fisheryCode);
		for (sp = 0; sp < bm->K_num_tot_sp; sp++) {
			if (FunctGroupArray[sp].isImpacted == TRUE) {
				flag_sp = (int) (FunctGroupArray[sp].speciesParams[flag_id]);
				if (flag_sp) {
					discard_2_print = bm->TotCumDiscards[sp][fishery_id] * bm->X_CN * mg_2_tonne;
					fprintf(fid, " %f", discard_2_print);
				}
			}
		}
		fprintf(fid, "\n");
	}

	return;
}
void writeAnnEffort(FILE *fid, MSEBoxModel *bm, FILE *llogfp) {
	int nf, b;
	double toteffort = 0;

	if (verbose > 1)
        printf( "Write total effort information\n");

	/* Write time */
	fprintf(fid, "%e", bm->dayt);

	/* Write effort values */
	for (nf = 0; nf < bm->K_num_fisheries; nf++) {
		toteffort = 0;
		for (b = 0; b < bm->nbox; b++) {
			toteffort += bm->CumEffort[nf][b];
		}
		fprintf(fid, " %f", toteffort);
	}
	fprintf(fid, "\n");

	return;
}

void writeDisplaceEffort(FILE *fid, MSEBoxModel *bm, FILE *llogfp) {
    int nf, b;
    double toteffort = 0;

    if (verbose > 1)
        printf( "Write total effort information\n");

    /* Write time */
    for (b = 0; b < bm->nbox; b++) {
        fprintf(fid, "%e %d", bm->dayt, b);
        
        /* Write effort values */
        for (nf = 0; nf < bm->K_num_fisheries; nf++) {
            fprintf(fid, " %f", bm->CumDisplaceEffort[b][nf]);
        }
        fprintf(fid, "\n");
    }

    return;
}
