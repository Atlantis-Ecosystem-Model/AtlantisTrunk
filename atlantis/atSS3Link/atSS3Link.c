/*
 * atSS3Link.c
 *
 *  Created on: May 2, 2013
 *      Author: bec
 */
#include <sys/types.h>

#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <math.h>
#include <ctype.h>
#include <time.h>
#include <string.h>
#ifdef _WIN32
#include <windows.h>
#include <direct.h>
#else
#include <unistd.h>
#endif
#include <sjwlib.h>
#include <atlantisboxmodel.h>
#include <atUtilLib.h>
#include <atSS3LinkPrivate.h>

static FILE *histfp;
static FILE * Init_Hist_File(MSEBoxModel *bm);
static void WriteHistoryFile(FILE *fid, MSEBoxModel *bm);

int SS3Link_Init(MSEBoxModel *bm, int species){

	//SS3_Test_Init(bm);

	if(!bm->RBCestimation.initHistFileDone)
		histfp = Init_Hist_File(bm);

	return 0;
}
void SS3Link_Free(MSEBoxModel *bm){

    printf("Closing Link OutputFile\n");
    
	Util_Close_Output_File(histfp);

}

/* Spawn a child process running a new program.  PROGRAM is the name
   of the program to run; the path will be searched for this program.
   ARG_LIST is a NULL-terminated list of character strings to be
   passed as the program's argument list.  Returns the process id of
   the spawned process.  */

int spawn (char* program, unsigned int timeout, unsigned int startTime, unsigned int sleepTime, char** arg_list) {
#ifndef _WIN32
  pid_t child_pid;
  int started = FALSE;

  unsigned int time = 0;

  sleep(2);

  /* Duplicate this process.  */
  child_pid = fork ();
  if (child_pid != 0){
    /* This is the parent process.  */

	  /* First check that SS3 has started */
	  while(time < startTime){

		  sleep(sleepTime);
		  time += sleepTime;

		  /* Check for presence of runnumber.ss */
		  if( access( "runnumber.ss", F_OK ) != -1 ) {
			  started = TRUE;
			  printf("Successfully started SS3 model\n");
			  break;
		  }
	  }

	  if(started == TRUE){
	  /* First check that SS3 has started */
		  while(time < timeout){

			  sleep(sleepTime);
			  time += sleepTime;

			  /* Check for presence of ss3.log */
			  if( access( "ss3.log", F_OK ) != -1 ) {
				  break;
			  }
		  }
	  }else{
		  printf("SS3 model failed to start\n");
	  }

	  /* Check for presence of 'ss3.log' file */
    return child_pid;
  }else {
    /* Now execute PROGRAM, searching for it in the path.  */
    execvp (program, arg_list);
    /* The execvp function returns only if an error occurs.  */
    fprintf (stderr, "an error occurred in execvp\n");
    abort ();
  }
  sleep(2);
#else
  return 0;
#endif
}

//void Start_SS3_Link(MSEBoxModel *bm, char *folderName) {
void WriteSSFiles(MSEBoxModel *bm, int species, int year, FILE *llogfp){
	static unsigned int timeout = 60;
	static unsigned int sleepTime = 1;
	static unsigned int startTime = 10;
	int result;
	char *folderName = "SS3LinkFolder";
	char dataFileName[STRLEN];
	char controlFileName[STRLEN];
    //char SSFileName[STRLEN];

	snprintf(dataFileName, sizeof(dataFileName), "%sDatFile.dat", FunctGroupArray[species].groupCode);
	snprintf(controlFileName, sizeof(controlFileName), "%sControlFile.ctl", FunctGroupArray[species].groupCode);
    //snprintf(SSFileName, sizeof(SSFileName), "%s.ss", FunctGroupArray[species].groupCode);

	/* First rm all the contents of the folder */
	result = chdir(folderName);

	if(result >=0){
		system("rm *.*");

		chdir("..");

		//system("cp Simple/simple.dat TestFolder");
		//system("cp Simple/simple.ctl TestFolder");
		//system("cp Simple/forecast.ss TestFolder");
	}

	Create_Starter_File(bm, folderName, species, 1);
	Write_Forecast_File(bm, folderName, 2007 - 1915, species, 1);

	printf("bm->thisyear = %d\n", bm->thisyear);


	/* Sample years/files only - need to test with real generated data */
	Write_SS_Data_File(bm, folderName, dataFileName, 2007 - 1915, species, 1);
	Write_SS_Control_File(bm, folderName, controlFileName, 2007 - 1915, species, 1);

	//abort();
	spawn("./child", timeout, startTime, sleepTime, NULL);

	result = chdir(folderName);

	if(result >=0){
		bm->RBCestimation.RBCspeciesParam[species][RBCest_id] = Read_SS3_Report_File(bm, species, year, folderName);
		printf("RBC = %e\n", bm->RBCestimation.RBCspeciesParam[species][RBCest_id]);
        bm->RBCestimation.RBCspeciesParam[species][MaxConvergCrit_id] = Read_SS3_Par_File(bm, species, year, folderName);
		chdir("..");
	}
}

//******************************************************************************
//
// Name:  Initiate the history file
// Description: write initial history - biomass by region
//
// created  : Nov 2010 Sally
//
//******************************************************************************
static FILE * Init_Hist_File(MSEBoxModel *bm) {
	FILE *fid;
	char fname[STRLEN];
	int sp;

	/** Create filename **/
	snprintf(fname, sizeof(fname), "%sHist.txt", bm->startfname);

	/** Create file **/
	if ( (fid=Util_fopen(bm, fname, "w")) == NULL )
		quit("Init_Hist_File: Can't open %s\n",fname);

	/** Column definitions **/
	fprintf(fid, "Time Region");

	/* Biomasses */
	for (sp = 0; sp < bm->K_num_tot_sp; sp++) {
		if(FunctGroupArray[sp].isFished || FunctGroupArray[sp].isImpacted)
			fprintf(fid, " %s", FunctGroupArray[sp].groupCode);
	}

	/* Catches */
	for (sp = 0; sp < bm->K_num_tot_sp; sp++) {
		if(FunctGroupArray[sp].isFished || FunctGroupArray[sp].isImpacted)
			fprintf(fid, " %s-Catch", FunctGroupArray[sp].groupCode);
	}

	/* Discards */
	for (sp = 0; sp < bm->K_num_tot_sp; sp++) {
		if(FunctGroupArray[sp].isFished || FunctGroupArray[sp].isImpacted)
			fprintf(fid, " %s-Discard", FunctGroupArray[sp].groupCode);
	}

	fprintf(fid, "\n");

	/* Return file pointer */
	bm->RBCestimation.initHistFileDone = 1;
	return (fid);


}

//******************************************************************************
//
// Name:  WriteHistory
// Description: write history by region for plotting
//
// created  : Nov 2010 Sally
//
//******************************************************************************
void WriteHistory(MSEBoxModel *bm, int groupIndex) {

	// Only print it out of its the last species with tiered assessment
	if(groupIndex == bm->RBCestimation.lastTierSp)
		WriteHistoryFile(histfp, bm);

	return;
}

void WriteHistoryFile(FILE *fid, MSEBoxModel *bm)
{
	int iy, f, groupIndex, region, Nfleets = 0, Nstocks;
	double totcatch, totdisc, sb0;

	for (iy = bm->RBCestimation.HistYrMin; iy < bm->RBCestimation.HistYrMax; iy++){
		for(region = 0; region < bm->K_num_reg; region++) {
			for(groupIndex = 0; groupIndex < bm->K_num_tot_sp; groupIndex++){
                Nstocks = FunctGroupArray[groupIndex].numStocks;
				if(FunctGroupArray[groupIndex].isFished || FunctGroupArray[groupIndex].isImpacted) {
					Nfleets = (int)(bm->RBCestimation.RBCspeciesParam[groupIndex][NumFisheries_id]);

					if (bm->RBCestimation.RBCspeciesParam[groupIndex][Regime_year_id] > 0)
						sb0 = bm->RBCestimation.RBCspeciesArray[groupIndex].SBioZero_shift[Nstocks][region];
					else
						sb0 = bm->RBCestimation.RBCspeciesArray[groupIndex].SBioZero[Nstocks][region];

					fprintf(fid, "%d %d %e", iy, region, (bm->RBCestimation.RBCspeciesArray[groupIndex].SpawnBio[Nstocks][region][iy] / sb0));
				}
			}
			for(groupIndex = 0; groupIndex < bm->K_num_tot_sp; groupIndex++){
				if(FunctGroupArray[groupIndex].isFished || FunctGroupArray[groupIndex].isImpacted) {
					totcatch = 0;
					for (f = 0; f < Nfleets; f++){
						totcatch += bm->RBCestimation.RBCspeciesArray[groupIndex].CatchData[f][region][iy];
					}
					fprintf(fid, " %e ", totcatch);
				}
			}
			for(groupIndex = 0; groupIndex < bm->K_num_tot_sp; groupIndex++){
				if(FunctGroupArray[groupIndex].isFished || FunctGroupArray[groupIndex].isImpacted) {
					totdisc = 0;
					for (f = 0; f < Nfleets; f++){
						totdisc += bm->RBCestimation.RBCspeciesArray[groupIndex].DiscData[f][region][iy];
					}
					fprintf(fid, " %e", totdisc);
				}
			}
			fprintf(fid, "\n");
		}
	}
	return;
}
