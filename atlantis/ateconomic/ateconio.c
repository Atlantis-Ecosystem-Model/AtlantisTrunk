/**
 \file
 \brief 	C file for output of economic indicators and information for Atlantis
 \ingroup ateconomic

 File:	ateconio.c
 Author:	Beth Fulton
 Created:	19/10/2006
 Purpose:	C file for output of economic indicators and information for Atlantis
 Revisions: 19/10/2006 Created the file from ateconomicindicator.c

 19/10/2006 Added output of bm->Trades[][]

 02-02-2009 Bec Gorton
 Removed all references to static input files. These have been moved into the atEconomicModule.

 29-04-2009 Bec Gorton
 Added the economicModule code to the svn repository.

 28-10-2009 Bec Gorton
 Removed references to the K_num_fished_sp and K_num_impacted_sp. Arrays that were this
 size have been resized to K_num_tot_sp.

 12-05-2010 Bec Gorton
 Added code to create a new deemed value output file for Olivier.


 **/

/*  Open library routines */
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <sjwlib.h>
#include <atEconomic.h>

/**
 * @name Economic indicators per subfleet output file
 */
//@{
static FILE *anneconindfp;

/* Quota and market prices */
static FILE *pricesfp;

/* Deemed values */
static FILE *deemedValuefp;

/* Effort schedule reporting output file*/
static FILE *efftschedfp;

/* Realised annual effort schedule output file */
static FILE *realefftschedfp;

/* Quota trade reporting output file */
static FILE *tradefp;

/* Economic performance measures output file */
static FILE *anneconfp;

/* Port Indices output file*/
static FILE *annportfp;

/* Quota Allocations output file - quota_per_fishery */
static FILE *quotaallocfp;

/* Trade matrix output file */
static FILE *tradematrixfp;

/* Monthly catch reporting output file*/
static FILE *tempcatchfp;

/* Expected catch per month output file */
static FILE *expectedcatchpermonthfp;

/* Static function declarations */
static FILE * Init_Econ_Ind_File(MSEBoxModel *bm);
static FILE * Init_Price_File(MSEBoxModel *bm);
static FILE * Init_Deemed_Value_File(MSEBoxModel *bm);
static FILE * Init_QuotaAlloc_File(MSEBoxModel *bm);
static FILE * Init_Trade_Matrix_File(MSEBoxModel *bm);
static FILE * Init_Effort_Schedule_File(MSEBoxModel *bm);
static FILE * Init_Real_Effort_Schedule_File(MSEBoxModel *bm);
static FILE * Init_Trades_File(MSEBoxModel *bm);
static FILE * Init_Econ_Meas_File(MSEBoxModel *bm);
static FILE * Init_Port_Meas_File(MSEBoxModel *bm);
static FILE * Init_Temporal_Catch_File(MSEBoxModel *bm);
static FILE * Init_Expected_CatchPerMonth_File(MSEBoxModel *bm);

/* write out functions */
static void Write_Econ_Ind(FILE *fid, MSEBoxModel *bm);
static void Write_Prices(FILE *fid, MSEBoxModel *bm);
static void Write_Deemed_Values(FILE *fid, MSEBoxModel *bm);
static void Write_QuotaAlloc(FILE *fid, MSEBoxModel *bm, FILE *llogfp);
static void Write_Trade_Matrix(FILE *fid, MSEBoxModel *bm, FILE *llogfp);
static void Write_Econ_Meas(FILE *fid, MSEBoxModel *bm);
static void Write_Port_Meas(FILE *fid, MSEBoxModel *bm);
static void Write_Effort_Schedule(FILE *fid, MSEBoxModel *bm);
static void Write_Real_Effort_Schedule(FILE *fid, MSEBoxModel *bm);
static void Write_Temporal_Catches(FILE *fid, MSEBoxModel *bm);
static void Write_Trades(FILE *fid, MSEBoxModel *bm);

void Open_Econ_Indx_Files(MSEBoxModel *bm) {

	anneconindfp = Init_Econ_Ind_File(bm);
	pricesfp = Init_Price_File(bm);
	deemedValuefp = Init_Deemed_Value_File(bm);
	quotaallocfp = Init_QuotaAlloc_File(bm);
	tradematrixfp = Init_Trade_Matrix_File(bm);
	efftschedfp = Init_Effort_Schedule_File(bm);
	realefftschedfp = Init_Real_Effort_Schedule_File(bm);
	tradefp = Init_Trades_File(bm);
	anneconfp = Init_Econ_Meas_File(bm);
	annportfp = Init_Port_Meas_File(bm);
	tempcatchfp = Init_Temporal_Catch_File(bm);
	expectedcatchpermonthfp = Init_Expected_CatchPerMonth_File(bm);
}

void Close_Econ_Indx_Files(MSEBoxModel *bm) {
	Util_Close_Output_File(anneconindfp);
	Util_Close_Output_File(pricesfp);
	Util_Close_Output_File(deemedValuefp);
	Util_Close_Output_File(efftschedfp);
	Util_Close_Output_File(realefftschedfp);
	Util_Close_Output_File(tradefp);
	Util_Close_Output_File(anneconfp);
	Util_Close_Output_File(annportfp);
	Util_Close_Output_File(quotaallocfp);
	Util_Close_Output_File(tradematrixfp);
	Util_Close_Output_File(tempcatchfp);
	Util_Close_Output_File(expectedcatchpermonthfp);
}

/**
 *
 *	\brief Write out economic indicators per subfleet
 *
 */

void Economic_Output_Indices(MSEBoxModel *bm, FILE *llogfp) {

	if (verbose)
		fprintf(stderr, "Writing annual economic indices\n");

	/* Write output */
	Write_Econ_Ind(anneconindfp, bm);
	Write_Prices(pricesfp, bm);
	Write_Deemed_Values(deemedValuefp, bm);
	Write_QuotaAlloc(quotaallocfp, bm, llogfp);
	Write_Trade_Matrix(tradematrixfp, bm, llogfp);

	return;
}

/**
 * \brief Routine to initialise fishery performance measure information file
 *
 */
FILE * Init_Econ_Ind_File(MSEBoxModel *bm) {
	FILE *fid;
	char fname[STRLEN];
	int sp = 0;

	/** Create filename **/
	snprintf(fname, sizeof(fname),"%sEconSubIndx.txt", bm->startfname);
	printf("Creating %s\n", fname);

    /** Create file **/
    if( (fid=Util_fopen(bm, fname, "w")) == NULL )
    	quit("Init_Econ_Meas_File: Can't open %s\n",fname);

    /** Column definitions **/
	fprintf(fid, "Time Fishery Subfleet");

	/* Economic performance measures */
	printf("K_sub_fleet_reporting = %d\n", K_sub_fleet_reporting);
	for (sp = 0; sp < K_sub_fleet_reporting; sp++) {
		fprintf(fid, " %s", bm->SUBFLEETeconindxNAME[sp]);
	}

	fprintf(fid, "\n");

	/* Return file pointer */
	return (fid);
}

void Write_Econ_Ind(FILE *fid, MSEBoxModel *bm) {
	int sp, nf, ns;

	if (verbose > 1)
        printf( "Write subfleet economic indices\n");

	/* Write catch values */
	for (nf = 0; nf < bm->K_num_fisheries; nf++) {
		for (ns = 0; ns < bm->FISHERYprms[nf][nsubfleets_id]; ns++) {
			/* Write time, fishery and subfleet */
			fprintf(fid, "%e %s %d", bm->dayt, FisheryArray[nf].fisheryCode, ns);

			/* Economic indicators */
			for (sp = 0; sp < K_sub_fleet_reporting; sp++) {
				fprintf(fid, " %e", bm->SUBFLEET_ECONprms[nf][ns][sp]);
			}
			fprintf(fid, "\n");
		}
	}

	return;
}

/**
 * \brief Effort schedule reporting
 *
 */
void Economic_Output_Effort_Schedule(MSEBoxModel *bm) {
	if (verbose)
		fprintf(stderr, "Writing annual effort schedule\n");

	/* Write output */
	Write_Effort_Schedule(efftschedfp, bm);

	return;
}

void Economic_Output_Realised_Schedule(MSEBoxModel *bm) {

	if (verbose)
		fprintf(stderr, "Writing realised annual effort schedule\n");

	/* Write output */
	Write_Real_Effort_Schedule(realefftschedfp, bm);

	return;
}

/**
 *	\brief Routine to initialise effort schedule reporting file
 *
 */
FILE * Init_Effort_Schedule_File(MSEBoxModel *bm) {
	FILE *fid;
	char fname[STRLEN];

	/** Create filename **/
	snprintf(fname, sizeof(fname),"%sEffortSchedIndx.txt", bm->startfname);
	printf("Creating %s\n", fname);

    /** Create file **/
    if( (fid=Util_fopen(bm, fname, "w")) == NULL )
	    quit("Init_Effort_Schedule_File: Can't open %s\n",fname);

    /** Column definitions **/
	fprintf(fid, "Time Fishery Subfleet Jan Feb Mar Apr May Jun Jul Aug Sep Oct Nov Dec\n");

	/* Return file pointer */
	return (fid);
}

/**
 *
 *	\brief Routine to initialise effort schedule reporting file
 *
 **/
FILE * Init_Real_Effort_Schedule_File(MSEBoxModel *bm) {
	FILE *fid;
	char fname[STRLEN];

	/** Create filename **/
	snprintf(fname, sizeof(fname),"%sRealEffortSchedIndx.txt", bm->startfname);
	printf("Creating %s\n", fname);

    /** Create file **/
    if( (fid=Util_fopen(bm, fname, "w")) == NULL )
	    quit("Init_Real_Effort_Schedule_File: Can't open %s\n",fname);

	/** Column definitions **/
	fprintf(fid, "Time Fishery Subfleet Jan Feb Mar Apr May Jun Jul Aug Sep Oct Nov Dec\n");

	/* Return file pointer */
	return (fid);
}

void Write_Effort_Schedule(FILE *fid, MSEBoxModel *bm) {
	int nf, ns, month;

	if (verbose > 1)
        printf( "Write gross effort schedule\n");

	/* Write effort schedule values */
	for (nf = 0; nf < bm->K_num_fisheries; nf++) {
		for (ns = 0; ns < max(1,bm->FISHERYprms[nf][nsubfleets_id]); ns++) {

			/* Write time and port */
			fprintf(fid, "%e %s %d", bm->dayt, FisheryArray[nf].fisheryCode, ns);

			/* Economic effort schedule */
			for (month = 0; month < 12; month++) {
				fprintf(fid, " %e", bm->EffortSchedule[nf][ns][month][expect_id]);
			}
			fprintf(fid, "\n");
		}
	}

	fflush(fid);
	return;
}

void Write_Real_Effort_Schedule(FILE *fid, MSEBoxModel *bm) {
	int nf, ns, month;

	if (verbose > 1)
        printf( "Write realised gross effort schedule\n");

	/* Write predicted effort schedule values */
	for (nf = 0; nf < bm->K_num_fisheries; nf++) {
		for (ns = 0; ns < max(1,bm->FISHERYprms[nf][nsubfleets_id]); ns++) {

			/* Write time and port */
			fprintf(fid, "%e %s %d", bm->dayt, FisheryArray[nf].fisheryCode, ns);

			/* Economic effort schedule */
			for (month = 0; month < 12; month++) {
				fprintf(fid, " %e", bm->EffortSchedule[nf][ns][month][expect_id]);
			}
			fprintf(fid, "\n");
		}
	}

	/* Write realised effort schedule values */
	for (nf = 0; nf < bm->K_num_fisheries; nf++) {
		for (ns = 0; ns < max(1,bm->FISHERYprms[nf][nsubfleets_id]); ns++) {

			/* Write time and port */
			fprintf(fid, "%e %s %d", bm->dayt, FisheryArray[nf].fisheryCode, ns);

			/* Realised effort schedule */
			for (month = 0; month < 12; month++) {
				fprintf(fid, " %e", bm->EffortSchedule[nf][ns][month][current_id]);
			}
			fprintf(fid, "\n");
		}
	}
	fflush(fid);

	return;
}

/**
 *
 *	\brief Quota trade reporting
 *
 */
void Economic_Output_Trades(MSEBoxModel *bm) {
	//static FILE *tradefp = NULL;

	if (verbose)
		fprintf(stderr, "Writing quota trading statistics\n");

	/* Write output */
	Write_Trades(tradefp, bm);

	return;
}

/**
 *	\brief Routine to initialise effort schedule reporting file
 *
 */
FILE * Init_Trades_File(MSEBoxModel *bm) {
	FILE *fid;
	char fname[STRLEN];

	/** Create filename **/
	snprintf(fname, sizeof(fname),"%sTradeIndx.txt", bm->startfname);
	printf("Creating %s\n", fname);

    /** Create file **/
    if( (fid=Util_fopen(bm, fname, "w")) == NULL )
    	quit("Init_Trades_File: Can't open %s\n",fname);

    /** Column definitions **/
	fprintf(fid, "Time Group Jan Feb Mar Apr May Jun Jul Aug Sep Oct Nov Dec\n");

	/* Return file pointer */
	return (fid);
}

void Write_Trades(FILE *fid, MSEBoxModel *bm) {
	int sp, month;

	if (verbose > 1)
        printf( "Write gross trades\n");

	/* Write Trades */
	for (sp = 0; sp < bm->K_num_tot_sp; sp++) {
		if (FunctGroupArray[sp].isFished == TRUE) {
			/* Write time and group */
			fprintf(fid, "%e %s", bm->dayt, FunctGroupArray[sp].groupCode);

			/* Trades per month */
			for (month = 0; month < 12; month++) {
				fprintf(fid, " %d", bm->Trades[month][sp]);
			}
			fprintf(fid, "\n");
		}
	}
	fflush(fid);
	return;
}

/**
 *
 *	\brief Economic performance measures output
 *
 */
void Economic_Output_Meas(MSEBoxModel *bm) {
	//static FILE *anneconfp = NULL;

	if (verbose)
		fprintf(stderr, "Writing annual economic indices\n");

	/* Write output */
	Write_Econ_Meas(anneconfp, bm);
	return;
}
/**
 *
 *	\brief Routine to initialise fishery performance measure information file
 *
 */
FILE * Init_Econ_Meas_File(MSEBoxModel *bm) {
	FILE *fid;
	char fname[STRLEN];
	int sp = 0;

	/** Create filename **/
	snprintf(fname, sizeof(fname),"%sEconIndx.txt", bm->startfname);
	printf("Creating %s\n", fname);

    /** Create file **/
    if( (fid=Util_fopen(bm, fname, "w")) == NULL )
    	quit("Init_Econ_Meas_File: Can't open %s\n",fname);

    /** Column definitions **/
	fprintf(fid, "Time Fishery");

	/* Economic performance measures */
	for (sp = 0; sp < K_num_econ_indx; sp++) {
		fprintf(fid, " %s", bm->econindxNAME[sp]);
	}

	fprintf(fid, "\n");

	/* Return file pointer */
	return (fid);
}

void Write_Econ_Meas(FILE *fid, MSEBoxModel *bm) {
	int sp, nf;

	if (verbose > 1)
        printf( "Write economic indices\n");

	/* Write catch values */
	for (nf = 0; nf < bm->K_num_fisheries; nf++) {
		/* Write time and fishery */
		fprintf(fid, "%e %s", bm->dayt, FisheryArray[nf].fisheryCode);

		/* Economic performance measures */
		for (sp = 0; sp < K_num_econ_indx; sp++) {
			fprintf(fid, " %e", bm->econindx[nf][sp]);
		}
		fprintf(fid, "\n");
	}
	fflush(fid);

	return;
}

/**
 *
 *	\brief Port indices output
 *
 */
void Update_Port_Indx(MSEBoxModel *bm, FILE *llogfp) {
	int porti = 0;

	for (porti = 0; porti < bm->K_num_ports; porti++) {
		bm->portindx[porti][portwgt_id] = portweight[porti];
		bm->portindx[porti][portpop_id] = bm->Port_info[porti][popactive_id];
	}

	return;
}

void Economic_Output_Port_Meas(MSEBoxModel *bm) {
	if (verbose)
		fprintf(stderr, "Writing annual port indices\n");

	/* Write output */
	Write_Port_Meas(annportfp, bm);
	return;
}

/**
 *
 *	\brief Routine to initialise fishery performance measure information file
 *
 */
FILE * Init_Port_Meas_File(MSEBoxModel *bm) {
	FILE *fid;
	char fname[STRLEN];
	int sp = 0;

	/** Create filename **/
	snprintf(fname, sizeof(fname),"%sPortIndx.txt", bm->startfname);
	printf("Creating %s\n", fname);

    /** Create file **/
    if( (fid=Util_fopen(bm, fname, "w")) == NULL )
        quit("Init_Port_Meas_File: Can't open %s\n",fname);

    /** Column definitions **/
	fprintf(fid, "Time Port");

	/* Economic performance measures */
	for (sp = 0; sp < K_num_port_indx; sp++) {
		fprintf(fid, " %s", bm->portindxNAME[sp]);
	}

	fprintf(fid, "\n");

	/* Return file pointer */
	return (fid);
}

void Write_Port_Meas(FILE *fid, MSEBoxModel *bm) {
	int sp, nm;

	if (verbose > 1)
        printf( "Write market performance measures\n");

	/* Write catch values */
	for (nm = 0; nm < bm->K_num_ports; nm++) {

		/* Write time and port */
		fprintf(fid, "%e %d", bm->dayt, nm);

		/* Economic performance measures */
		for (sp = 0; sp < K_num_port_indx; sp++) {
			fprintf(fid, " %e", bm->portindx[nm][sp]);
		}
		fprintf(fid, "\n");
	}
	fflush(fid);

	return;
}

/**
 *
 *	\brief Routine to initialise fishery price measure information file
 *
 */
FILE * Init_Price_File(MSEBoxModel *bm) {
	FILE *fid;
	char fname[STRLEN];
	int sp;

	/** Create filename **/
	snprintf(fname, sizeof(fname),"%sPrices.txt", bm->startfname);
	printf("Creating %s\n", fname);

    /** Create file **/
    if( (fid=Util_fopen(bm, fname, "w")) == NULL )
        quit("Init_Price_File: Can't open %s\n",fname);

    /** Column definitions **/
	fprintf(fid, "Time Fleet Subfleet");

	/* Sale prices */
	for (sp = 0; sp < bm->K_num_tot_sp; sp++) {
		if (FunctGroupArray[sp].isFished == TRUE) {
			fprintf(fid, " SalePrice-%s", FunctGroupArray[sp].groupCode);
		}
	}

	/* Deemed values */
	for (sp = 0; sp < bm->K_num_tot_sp; sp++) {
		if (FunctGroupArray[sp].isFished == TRUE) {
			fprintf(fid, " DeemVal-%s", FunctGroupArray[sp].groupCode);
		}
	}

	/* Quota prices */
	for (sp = 0; sp < bm->K_num_tot_sp; sp++) {
		if (FunctGroupArray[sp].isFished == TRUE) {
			fprintf(fid, " QuotaPrice-%s", FunctGroupArray[sp].groupCode);
		}
	}

	fprintf(fid, "\n");

	/* Return file pointer */
	return (fid);
}

void Write_Prices(FILE *fid, MSEBoxModel *bm) {
	int sp, nf, ns;

	if (verbose > 1)
        printf( "Write market performance measures\n");

	/* Write prices */
	for (nf = 0; nf < bm->K_num_fisheries; nf++) {
		for (ns = 0; ns < bm->FISHERYprms[nf][nsubfleets_id]; ns++) {

			/* Write time and fleet info */
			fprintf(fid, "%e %s %d", bm->dayt, FisheryArray[nf].fisheryCode, ns);

			/* Sale prices */
			for (sp = 0; sp < bm->K_num_tot_sp; sp++) {
				if (FunctGroupArray[sp].isFished == TRUE) {
					fprintf(fid, " %e", bm->SP_FISHERYprms[sp][nf][saleprice_id]);
				}
			}

			/* Deemed values */
			for (sp = 0; sp < bm->K_num_tot_sp; sp++) {
				if (FunctGroupArray[sp].isFished == TRUE) {
					fprintf(fid, " %e", bm->SP_FISHERYprms[sp][nf][deemprice_id]);
				}
			}

			/* Quota prices */
			for (sp = 0; sp < bm->K_num_tot_sp; sp++) {
				if (FunctGroupArray[sp].isFished == TRUE) {
					fprintf(fid, " %e", bm->QuotaAlloc[nf][ns][sp][quotaprice_id]);
				}
			}
			fprintf(fid, "\n");
		}
	}

	fflush(fid);
	return;
}

/**************************************************************************//**
 *
 *	\brief Routine to initialise fishery deemed value output file.
 *
 *****************************************************************************/
FILE * Init_Deemed_Value_File(MSEBoxModel *bm) {
	FILE *fid;
	char fname[STRLEN];
	int sp = 0;

	/** Create filename **/
	snprintf(fname, sizeof(fname),"%sDeemedValue.txt", bm->startfname);
	printf("Creating %s\n", fname);

    /** Create file **/
    if( (fid=Util_fopen(bm, fname, "w")) == NULL )
        quit("Init_Deemed_Value_File: Can't open %s\n",fname);

    /** Column definitions **/
	fprintf(fid, "Time Fleet Subfleet");

	/* Deemed values */
	for (sp = 0; sp < bm->K_num_tot_sp; sp++) {
		if (FunctGroupArray[sp].isFished == TRUE) {
			fprintf(fid, " %s", FunctGroupArray[sp].groupCode);
		}
	}

	fprintf(fid, "\n");

	/* Return file pointer */
	return (fid);
}

void Write_Deemed_Values(FILE *fid, MSEBoxModel *bm) {
	int sp = 0, nf = 0, ns = 0;

	if (verbose > 1)
        printf( "Write market performance measures\n");

	/* Write prices */
	for (nf = 0; nf < bm->K_num_fisheries; nf++) {
		for (ns = 0; ns < bm->FISHERYprms[nf][nsubfleets_id]; ns++) {

			/* Write time and fleet info */
			fprintf(fid, "%e %s %d", bm->dayt, FisheryArray[nf].fisheryCode, ns);

			/* Sale prices */
			for (sp = 0; sp < bm->K_num_tot_sp; sp++) {
				if (FunctGroupArray[sp].isFished == TRUE) {
					fprintf(fid, " %e", bm->QuotaAlloc[nf][ns][sp][deemed_value_id]);
				}
			}

			fprintf(fid, "\n");
		}
	}

	fflush(fid);
	return;
}

/**************************************************************************//**
 *
 *	\brief Routine to initialise fishery quota allocation information file
 *
 *****************************************************************************/
FILE * Init_QuotaAlloc_File(MSEBoxModel *bm) {
	FILE *fid;
	char fname[STRLEN];
	int sp, flag_sp;

	/** Create filename **/
	snprintf(fname, sizeof(fname),"%sQuotaAlloc.txt", bm->startfname);
	printf("Creating %s\n", fname);

    /** Create file **/
    if( (fid=Util_fopen(bm, fname, "w")) == NULL )
        quit("Init_QuotaAlloc_File: Can't open %s\n",fname);

    /** Write column headers **/
	fprintf(fid, "Time Fishery ");

	/* Species quota name */
	for (sp = 0; sp < bm->K_num_tot_sp; sp++) {
		if (FunctGroupArray[sp].isFished == TRUE) {
			flag_sp = (int) (FunctGroupArray[sp].speciesParams[flag_id]);
			if (flag_sp)
				fprintf(fid, " Quota-%s", FunctGroupArray[sp].groupCode);
		}
	}

	/* Species remaining quota name */
	for (sp = 0; sp < bm->K_num_tot_sp; sp++) {
		if (FunctGroupArray[sp].isFished == TRUE) {
			flag_sp = (int) (FunctGroupArray[sp].speciesParams[flag_id]);
			if (flag_sp)
				fprintf(fid, " Remain-%s", FunctGroupArray[sp].groupCode);
		}
	}

	fprintf(fid, "\n");

	/* Return file pointer */
	return (fid);

}

void Write_QuotaAlloc(FILE *fid, MSEBoxModel *bm, FILE *llogfp) {
	int sp, fishery_id, ns, flag_sp;
	double value_2_print, prop_olease, prop_tlease;

	if (verbose > 1)
        printf( "Write quotas\n");

	/* Write Quota Allocations */
	for (fishery_id = 0; fishery_id < bm->K_num_fisheries; fishery_id++) {
		fprintf(fid, "%e %s", bm->dayt, FisheryArray[fishery_id].fisheryCode);
		/* Quota held */
		for (sp = 0; sp < bm->K_num_tot_sp; sp++) {
			if (FunctGroupArray[sp].isFished == TRUE) {
				flag_sp = (int) (FunctGroupArray[sp].speciesParams[flag_id]);
				if (flag_sp) {
					if(!FisheryArray[fishery_id].isRec){
						value_2_print = 0.0;
						for (ns = 0; ns < bm->FISHERYprms[fishery_id][nsubfleets_id]; ns++) {
							prop_olease = bm->QuotaAlloc[fishery_id][ns][sp][permolease_id];
							prop_tlease = bm->QuotaAlloc[fishery_id][ns][sp][templease_id];
							value_2_print += (1.0 - prop_olease - prop_tlease) * bm->QuotaAlloc[fishery_id][ns][sp][owned_id];
							value_2_print += bm->QuotaAlloc[fishery_id][ns][sp][leased_id];


							//							if(fishery_id == dtrawlFDB_id)
							//								fprintf(llogfp,"Time: %e, %s by %s-%d prop_olease: %e, prop_tlease: %e, own: %e, leased: %e, final_value: %e\n",
							//									bm->dayt, FunctGroupArray[sp].groupCode, FisheryArray[fishery_id].fisheryCode, ns, prop_olease, prop_tlease, bm->QuotaAlloc[fishery_id][ns][sp][owned_id], bm->QuotaAlloc[fishery_id][ns][sp][leased_id], value_2_print);
						}
						fprintf(fid, " %f", value_2_print);
					} else {
						value_2_print = 0.0;
						fprintf(fid, " %f", value_2_print);
					}
				}
			}
		}

		/* Quota remaining */
		for (sp = 0; sp < bm->K_num_tot_sp; sp++) {
			if (FunctGroupArray[sp].isFished == TRUE) {
				flag_sp = (int) (FunctGroupArray[sp].speciesParams[flag_id]);
				if (flag_sp) {
					if(!FisheryArray[fishery_id].isRec){
						fprintf(fid, " %f", bm->SP_FISHERYprms[sp][fishery_id][totquota_remain]);
					} else {
						value_2_print = 0.0;
						fprintf(fid, " %f", value_2_print);
					}
				}
			}
		}

		fprintf(fid, "\n");
	}

	fflush(fid);
	return;
}
/**************************************************************************//**
 *
 *	\brief Routine to initialise trade matrix file
 *
 *****************************************************************************/
FILE * Init_Trade_Matrix_File(MSEBoxModel *bm) {
	FILE *fid;
	char fname[STRLEN];
	int nf;

	/** Create filename **/
	snprintf(fname, sizeof(fname),"%sTradeMatrix.txt", bm->startfname);
	printf("Creating %s\n", fname);

    /** Create file **/
    if( (fid=Util_fopen(bm, fname, "w")) == NULL )
        quit("Init_Trade_Matrix_File: Can't open %s\n",fname);

    /** Write column headers **/
	fprintf(fid, "Time Seller Species ");

	/* Fishery name for amount traded */
	for (nf = 0; nf < bm->K_num_fisheries; nf++) {
		fprintf(fid, " Amt-%s", FisheryArray[nf].fisheryCode);
	}

	/* Fishery name for amount traded */
	for (nf = 0; nf < bm->K_num_fisheries; nf++) {
		fprintf(fid, " Num-%s", FisheryArray[nf].fisheryCode);
	}
	fprintf(fid, "\n");

	/* Return file pointer */
	return (fid);

}

void Write_Trade_Matrix(FILE *fid, MSEBoxModel *bm, FILE *llogfp) {
	int sp, sellernf, buyernf, flag_sp, nf, nf2;

	if (verbose > 1)
        printf( "Write trade matrix measures\n");

	/* Write trades */
	for (sellernf = 0; sellernf < bm->K_num_fisheries; sellernf++) {
		for (sp = 0; sp < bm->K_num_tot_sp; sp++) {
			if (FunctGroupArray[sp].isTAC) {
				flag_sp = (int) (FunctGroupArray[sp].speciesParams[flag_id]);
				if (flag_sp) {
					fprintf(fid, "%e %s %s", bm->dayt, FisheryArray[sellernf].fisheryCode, FunctGroupArray[sp].groupCode);
					/* Write amount traded */
					for (buyernf = 0; buyernf < bm->K_num_fisheries; buyernf++) {
						fprintf(fid, " %f", bm->QuotaTrade[sellernf][buyernf][sp][amttrade_id]);
					}
					/* Write number of trades */
					for (buyernf = 0; buyernf < bm->K_num_fisheries; buyernf++) {
						fprintf(fid, " %f", bm->QuotaTrade[sellernf][buyernf][sp][numtrade_id]);
					}
					fprintf(fid, "\n");
				}
			}
		}
	}

	if (bm->renewTrade) {
		/* Reinitialise the trade matrix */
		for (nf = 0; nf < bm->K_num_fisheries; nf++) {
			for (nf2 = 0; nf2 < bm->K_num_fisheries; nf2++) {
				for (sp = 0; sp < bm->K_num_tot_sp; sp++) {
					if (FunctGroupArray[sp].isFished == TRUE) {
						bm->QuotaTrade[nf][nf2][sp][amttrade_id] = 0.0;
						bm->QuotaTrade[nf][nf2][sp][numtrade_id] = 0.0;
					}
				}
			}
		}
		bm->renewTrade = 0;
	}
	fflush(fid);
	return;
}

/**************************************************************************//**
 *
 *	\brief Routine to initialise fishery price measure information file
 *
 *****************************************************************************/
void Economic_Output_Temporal_Catches(MSEBoxModel *bm) {
	if (verbose)
		fprintf(stderr, "Writing annual effort schedule\n");

	/* Initialise files if necessary */
	if (!tempcatchfp)
		tempcatchfp = Init_Temporal_Catch_File(bm);

	/* Write output */
	Write_Temporal_Catches(tempcatchfp, bm);

	return;
}

FILE * Init_Temporal_Catch_File(MSEBoxModel *bm) {
	FILE *fid;
	char fname[STRLEN];
	int sp = 0;

	/** Create filename **/
	snprintf(fname, sizeof(fname),"%sCatchPerMonth.txt", bm->startfname);
	printf("Creating %s\n", fname);

    /** Create file **/
    if( (fid=Util_fopen(bm, fname, "w")) == NULL )
        quit("Init_Temporal_Catch_File: Can't open %s\n",fname);

    /** Column definitions **/
	fprintf(fid, "Time Fleet Subfleet Month");

	/* Catches */
	for (sp = 0; sp < bm->K_num_tot_sp; sp++) {
		if (FunctGroupArray[sp].isFished == TRUE) {
			fprintf(fid, " Catch-%s", FunctGroupArray[sp].groupCode);
		}
	}

	fprintf(fid, "\n");

	/* Return file pointer */
	return (fid);
}

void Write_Temporal_Catches(FILE *fid, MSEBoxModel *bm) {
	int sp, nf, ns;

	if (verbose > 1)
        printf( "Write market performance measures\n");

	/* Write catches */
	for (nf = 0; nf < bm->K_num_fisheries; nf++) {
		for (ns = 0; ns < bm->FISHERYprms[nf][nsubfleets_id]; ns++) {

			/* Write time and fleet info */
			fprintf(fid, "%e %s %d %d", bm->dayt, FisheryArray[nf].fisheryCode, ns, bm->MofY);

			/* Monthly Catches */
			for (sp = 0; sp < bm->K_num_tot_sp; sp++) {
				if (FunctGroupArray[sp].isFished == TRUE) {
					fprintf(fid, " %e", bm->QuotaAlloc[nf][ns][sp][cummonthcatch_id]);
				}
			}

			fprintf(fid, "\n");
		}
	}

	fflush(fid);
	return;
}

FILE * Init_Expected_CatchPerMonth_File(MSEBoxModel *bm)
{
    FILE *fid;
    char fname[STRLEN];
	int sp = 0;

	/** Create filename **/
	snprintf(fname, sizeof(fname), "%sExpectedCatchPerMonth.txt",bm->startfname);
	printf("Creating %s\n",fname);

    /** Create file **/
    if( (fid=Util_fopen(bm, fname, "w")) == NULL )
	quit("initExpectedCatchPerMonthFile: Can't open %s\n",fname);

    /** Column definitions **/
    fprintf(fid,"Time Fleet Subfleet Month");

	/* Catches */
    for(sp=0; sp<bm->K_num_tot_sp; sp++){
		if (FunctGroupArray[sp].isFished == TRUE) {
			fprintf(fid," ExpectedCatch-%s", FunctGroupArray[sp].groupCode);
		}
	}

	fprintf(fid,"\n");

    /* Return file pointer */
    return(fid);
}


void Write_Expected_CatchPerMonth(FILE *fid, MSEBoxModel *bm)
{
    int sp = 0, nf = 0, ns = 0;
	if( verbose > 1)
		fprintf(stderr,"Write market performance measures\n");

	/* Write catches */
	for(nf=0; nf<bm->K_num_fisheries; nf++){
		for(ns=0; ns<bm->FISHERYprms[nf][nsubfleets_id]; ns++){


			/* Write time and fleet info */
			fprintf(fid,"%e %s %d %d", bm->dayt - 30.0, FisheryArray[nf].fisheryCode, ns, bm->lastMofY);

			/* Monthly Catches */
			for(sp=0; sp<bm->K_num_tot_sp; sp++){
				if (FunctGroupArray[sp].isFished == TRUE) {
					fprintf(fid," %e", bm->BlackBook[nf][ns][sp][bm->lastMofY][expect_id]);
				}
			}

			fprintf(fid,"\n");
		}
	}

	fflush(fid);
	return;
}

void Economic_Output_ExpectedCatchPerMonth(MSEBoxModel *bm)
{

	if( verbose )
		fprintf(stderr,"Writing realised monthly expected catch\n");

	/* Initialise files if necessary */
	if( !expectedcatchpermonthfp)
		expectedcatchpermonthfp = Init_Expected_CatchPerMonth_File(bm);

	/* Write output */
	Write_Expected_CatchPerMonth(expectedcatchpermonthfp, bm);

	return;
}

