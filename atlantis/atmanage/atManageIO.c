/**
 *
 * \ingroup atManageLib
 * \file	atManageIO.c
 * \brief C file for fisheries management summary output
 * \author	Beth Fulton 20/7/2006
 *
 *
 *
 * 	<b>Revisions</b>
 * 	20/7/2006 Created from the write routines once stored in atannualmanage.c file
 *
 * 	28/11/2008 Bec Gorton
 * 	Moved the static files into the manageModule so that they can be freed at the
 * 	end of the model run.
 *
 *	02-11-2009 Bec Gorton
 *	Removed references to the K_num_impacted_sp and K_num_fished_sp variables.
 *
 *
 *	 03-11-2009 Bec Gorton
 *	 Removed functions that were comments out that have been moved into the harvest or managementImplement
 *	 libs.
 *
 *	 27-04-2010 Bec Gorton
 *	 Added code to print out information about the brokenStick management strategy.
 */

/*  Open library routines */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "atManage.h"

/* The file to report annual TAC set */
FILE *anntacfp;
FILE *annBrokenfp;
FILE *annCapResultfp;
FILE *cpuefp;
FILE *grosscpuefp;

/* Static prototypes delarations of private functions */
static void Write_Annual_TAC(FILE *fid, MSEBoxModel *bm, FILE *llogfp);
static FILE * initAnnBrokenStickFile(MSEBoxModel *bm);

/**
 * \brief Open the management output text files.
 */
void Open_Manage_Output_Files(MSEBoxModel *bm) {

	anntacfp = initAnnTACFile(bm);
	annBrokenfp = initAnnBrokenStickFile(bm);
}
/**
 * \brief Close the management output text files.
 */
void Close_Manage_Output_Files(MSEBoxModel *bm) {

	/* Close the annual TAC reporting file */
	Util_Close_Output_File(anntacfp);
	/* Close the broken stick output file */
	Util_Close_Output_File(annBrokenfp);
    /* Close the ecosystem caps output file */
    Util_Close_Output_File(annCapResultfp);
}

/**
 * \brief A routine to report the annual TACs set
*/
void Report_TACs(MSEBoxModel *bm, FILE *llogfp){

	if( verbose )
		fprintf(stderr,"Writing annual TACs as output (instead of putting it in a logfile)\n");

	/* Initialise files if necessary */
	if( !anntacfp )
		anntacfp = initAnnTACFile(bm);

	/* Write output */
	Write_Annual_TAC(anntacfp,bm, llogfp);

	return;

}
/**
 * \brief Write out the annual TAC values
 */
void Write_Annual_TAC(FILE *fid, MSEBoxModel *bm, FILE *llogfp)
{
    int sp, flag_sp, nf, flagrecfish, in_quota;
	double totTAC = 0;

	if( verbose > 1)
		fprintf(stderr,"Write tac information\n");

	/* Write time */
	fprintf(fid,"%e", bm->dayt);

	/* Write TAC values */
	for(sp=0; sp<bm->K_num_tot_sp; sp++){
		if(FunctGroupArray[sp].isFished == TRUE){
			flag_sp = (int)(FunctGroupArray[sp].speciesParams[flag_id]);
			if(flag_sp){
				totTAC = 0;
				for(nf=0; nf<bm->K_num_fisheries; nf++){
					flagrecfish = (int)(bm->FISHERYprms[nf][flagrecfish_id]);
					in_quota = bm->inQuota[nf][sp];
					if(!flagrecfish && in_quota){
						/* Don't include recfishing in quota allocation for now (as often not quota-ed)
						FIX - may have to change this if recfishing becomes quota allocated group
						*/
						/* Convert kg to tonnes */
						totTAC += bm->TACamt[sp][nf][now_id] / 1000.0;

						//fprintf(llogfp,"%s TAC on %s is %e\n", FisheryArray[nf].fisheryCode, FunctGroupArray[sp].groupCode, bm->TACamt[sp][nf][now_id] / 1000.0);

					}
				}
				if(totTAC < 0)
					totTAC = 0;
				fprintf(fid," %f", totTAC);		/* No conversion necessary as already in tonnes */
			}
		}
	}
	fprintf(fid,"\n");
    
    fflush(fid);
    
	return;
}




/* Routine to initialise catch information file */
FILE * initAnnBrokenStickFile(MSEBoxModel *bm)
{
    FILE *fid;
    char fname[STRLEN];

	/** Create filename **/
	snprintf(fname, sizeof(fname),"%s_BrokenStick.txt",bm->startfname);
	printf("Creating %s\n",fname);

    /** Create file **/
    if( (fid=Util_fopen(bm, fname,"w")) == NULL )
    	quit("initAnnBrokenStickFile: Can't open %s\n",fname);

    /** Column definitions **/
    fprintf(fid,"Time ");
    fprintf(fid,"SpeciesName ");
    fprintf(fid,"FisheryName ");
    fprintf(fid,"Tier ");
    fprintf(fid,"FrefLim ");
    fprintf(fid,"FrefA ");
    fprintf(fid,"FrefH ");
    fprintf(fid,"Blim ");
    fprintf(fid,"BrefB ");
    fprintf(fid,"BrefA ");
    fprintf(fid,"FCurr ");
    fprintf(fid,"FTARG ");
    fprintf(fid,"CurrentBiomass ");
    fprintf(fid,"BrokenStickScaler ");
	fprintf(fid,"\n");
    /* Return file pointer */
    return(fid);
}
/**
 * \brief Write out info to the broken stick output file.
 */
void WriteAnnBrokenStickFile(MSEBoxModel *bm,  int species, int nf, int tier, double FrefLim, double FrefA, double FrefH, double Blim, double BrefA, double BrefB, double FCurr, double FTARG, double Bcurr, double scalar)
{
	if(annBrokenfp == NULL){
		annBrokenfp = initAnnBrokenStickFile(bm);
	}
	fprintf(annBrokenfp, "%e %s %s %d %e %e %e %e %e %e %e %e %e %e\n", bm->dayt, FunctGroupArray[species].groupCode,  FisheryArray[nf].fisheryCode, tier, FrefLim, FrefA, FrefH, Blim, BrefA, BrefB, FCurr, FTARG, Bcurr, scalar);
}

/* Routine to initialise catch information file */
FILE * initCapResultStickFile(MSEBoxModel *bm)
{
    FILE *fid;
    char fname[STRLEN];

    /** Create filename **/
    snprintf(fname, sizeof(fname),"%s_EcosystemCapResult.txt",bm->startfname);
    printf("Creating %s\n",fname);

    /** Create file **/
    if( (fid=Util_fopen(bm, fname,"w")) == NULL )
        quit("initCapResultStickFile: Can't open %s\n",fname);

    /** Column definitions **/
    fprintf(fid,"Time ");
    fprintf(fid,"SpeciesName ");
    fprintf(fid,"FisheryName ");
    fprintf(fid,"SpBased_Frescale ");
    fprintf(fid,"PostSystCap_Frescale ");
    fprintf(fid,"OrigExpectedCatch ");
    fprintf(fid,"Deductions ");
    fprintf(fid,"\n");
    /* Return file pointer */
    return(fid);
}

/**
 * \brief Write out info to do with the systems cap results output file.
 */
void WriteAnnCapResultFile(MSEBoxModel *bm, int species, int nf)
{
    if(annCapResultfp == NULL){
        annCapResultfp = initCapResultStickFile(bm);
    }
    fprintf(annCapResultfp, "%e %s %s %e %e %e %e\n", bm->dayt, FunctGroupArray[species].groupCode, FisheryArray[nf].fisheryCode, bm->SP_FISHERYprms[species][nf][orig_mFC_scale_id], bm->SP_FISHERYprms[species][nf][mFC_scale_id], FunctGroupArray[species].speciesParams[sp_fishery_expected_catch_id], FunctGroupArray[species].speciesParams[sp_fishery_deduction_id]);
}


/***************************************************************************************************************************************************************
 *  \brief Routine to initialise generated CPUE data file
 */
FILE * initCPUEGenFile(MSEBoxModel *bm) {
	FILE *fid;
	char fname[STRLEN];
    
    printf("Actually creating CPUE file\n");
    
	/** Create filename **/
	snprintf(fname, sizeof(fname), "%sCPUEgen.txt", bm->startfname);
	printf("Creating %s\n", fname);
    
	/** Create file **/
	if ((fid = Util_fopen(bm, fname, "w")) == NULL)
		quit("initCPUEGenFile: Can't open %s\n", fname);
    
	/** Column definitions **/
	fprintf(fid, "Time Species EffecClass Box Fishery Catch Discards Effort Depth FinalPropAlloc\n");
    
	/* Return file pointer */
	return (fid);
}

/***************************************************************************************************************************************************************
 * \brief Write out the generated CPUE data
 */
void Write_CPUEreport(MSEBoxModel *bm, int nf, FILE *llogfp, double *tot_alloced)
{
    int sp, ns, flag_sp;

    
    // Initialise files if necessary
    if( !cpuefp ) {
        printf("Creating CPUE file\n");
        
        cpuefp = initCPUEGenFile(bm);
        
    }

    for (sp = 0; sp < bm->K_num_tot_sp; sp++) {
        flag_sp = (int)(FunctGroupArray[sp].speciesParams[flag_id]);
        if(!flag_sp || (FunctGroupArray[sp].isFished == FALSE) || (bm->SP_FISHERYprms[sp][nf][q_id] == FALSE) || (bm->SP_FISHERYprms[sp][nf][flagPerShotCPUE_id] < 1))
           continue;  // As no data recorded
        else {
            for(ns=0; ns < nshot; ns++) {
                fprintf(cpuefp,"%e %s %d %d %s %e %e %e %e %e\n",
                    bm->dayt, FunctGroupArray[sp].groupCode, (int)(bm->ShotData[ns][sp][guru_id]), (int)(bm->ShotData[ns][sp][tloc_id]), FisheryArray[nf].fisheryCode,
                    bm->ShotData[ns][sp][tcatch_id], bm->ShotData[ns][sp][tdiscard_id],
                    bm->ShotData[ns][sp][teffort_id], bm->ShotData[ns][sp][wdepth_id],
                    tot_alloced[sp]);
            }
        }
    }
    
    return;
}

/***************************************************************************************************************************************************************
 *  \brief Routine to initialise per time step CPUE data file
 */
FILE * initCPUEFile(MSEBoxModel *bm) {
	FILE *fid;
	char fname[STRLEN];
    
    printf("Creating CPUE file\n");
    
	/** Create filename **/
	snprintf(fname, sizeof(fname), "%sCPUE.txt", bm->startfname);
	printf("Creating %s\n", fname);
    
	/** Create file **/
	if ((fid = Util_fopen(bm, fname, "w")) == NULL)
		quit("initCPUEFile: Can't open %s\n", fname);
    
	/** Column definitions **/
	fprintf(fid, "Time Species Box Depth Fishery Catch DiscardRate Effort\n");
    
	/* Return file pointer */
	return (fid);
}

/***************************************************************************************************************************************************************
 * \brief Write out the per time step CPUE data
 */
void Write_CPUE(MSEBoxModel *bm, FILE *llogfp)
{
    int sp, ij, nf, k, flag_sp;
    double totcatch, discard_rate, botz;
    
    
    // Initialise files if necessary
    if( !grosscpuefp ) {
        printf("Calling create CPUE file\n");
        
        grosscpuefp = initCPUEFile(bm);
        
    }
    
    for (sp = 0; sp < bm->K_num_tot_sp; sp++) {
        for (nf = 0; nf < bm->K_num_fisheries; nf++) {
            flag_sp = (int)(FunctGroupArray[sp].speciesParams[flag_id]);
            if(!flag_sp || (FunctGroupArray[sp].isFished == FALSE) || (bm->SP_FISHERYprms[sp][nf][q_id] == FALSE) || (bm->SP_FISHERYprms[sp][nf][flagRecordCPUE_id] < 1))
                continue;  // As no data recorded
            else {
                for (ij = 0; ij < bm->nbox; ij++) {
                    if (bm->Effort[ij][nf] > 0.0) {
                        totcatch = 0.0;
                        discard_rate = 0.0;
                        for (k = 0; k < bm->boxes[ij].nz; k++) {
                            totcatch += bm->Catch[ij][sp][nf][k];
                        }
                        discard_rate = bm->Discards[ij][sp][nf] / (totcatch + bm->Discards[ij][sp][nf] + small_num);
                        botz = bm->boxes[ij].botz;
                        if (botz < bm->maxwcbotz)
                            botz = bm->maxwcbotz;  // Open water boundary for deep boxes
                        botz = -1.0 * botz; // So a =ve number for recording purposes
                        for (k = 0; k < bm->boxes[ij].nz; k++) {
                            if((bm->Effort[ij][nf] * Effort_vdistrib[ij][k][nf]) > 0.0) {
                                fprintf(grosscpuefp,"%e %s %d %e %s %e %e %e\n",
                                        bm->dayt, FunctGroupArray[sp].groupCode, ij, botz, FisheryArray[nf].fisheryCode, bm->Catch[ij][sp][nf][k] * bm->X_CN * mg_2_kg, discard_rate, bm->Effort[ij][nf] * Effort_vdistrib[ij][k][nf]);
                            }
                            botz -= (bm->boxes[ij].dz[k]);   // Update for next layer - assuming all fishing at the bottom of each layer
                            if( botz < 0.0)  // Sanity check
                                botz = 0.0;
                        }
                    }
                }
            }
        }
    }
    
    return;
}


