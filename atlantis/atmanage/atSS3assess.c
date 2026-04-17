/**
 * \ingroup atManageLib
 * \file atSS3assess.c
 * \brief Subroutines for setting up the SS input files
 *     \author    Beth Fulton 8/3/2022
 
 * Original close kin Author: Rich Little
 * Expansion and Modifications: Beth Fulton
 *
 *    <b>  Revisions</b>
 *
 *  This set of routines handles doing latest SS assessment
 *
 *  //  Functions in this file :
 *     WriteSSFiles:   write the files for input to SS
 *     WriteSSDat:     write the generated data to data file for input to SS filename.dat
 *     WriteSSCtl:     write the control file for input to SS filename.ctl
 *     WriteSSFor:     write the forecast file for input to SS
 *
 */

#include <stdio.h>
#include <string.h>
#include <math.h>
#include <sjwlib.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <errno.h>
#include <unistd.h>
#include <limits.h>

#include "atManage.h"

#define MYLEN 72

int Create_Dir_SS3(MSEBoxModel *bm, int sp, int year, FILE *llogfp);
void Run_SS3(MSEBoxModel *bm, char *run_dir);

//******************************************************************************
//
// Name:  Create_Dir_SS3
// Creating a directory for the SS activity - done here to try to force it to
// occur before the write occurs
//
// called by: SS330Assessment
// created: Aug 2023 Beth Fuliton
//
//******************************************************************************
int Create_Dir_SS3(MSEBoxModel *bm, int sp, int year, FILE *llogfp) {

    char dirName[STRLEN];
    int ret = 0;

    struct stat st = {0};
    
    /* Create directory */
    snprintf(dirName, sizeof(dirName), "%s_SS3_sim_%d_year_%d", FunctGroupArray[sp].groupCode, bm->RBCestimation.sim, year);
    
    if (stat(dirName, &st) == -1) {
        printf("Creating %s\n", dirName);
        ret = mkdir(dirName, 0777);  // Was S_IRWXU instead of 0777
    }

    /* Change directory into the new directory - lets skip this for now and write into the directory instead
    //pass your path in the function
    ret = chdir(dirName);
    // if the change of directory was successful it will print successful otherwise it will print not successful
    if (ret < 0)
        quit("SS330Assessment: chdir change of directory to %s not successful\n", dirName);
    else
        printf("SS330Assessment: chdir change of directory to %s successful", dirName);
     
     */
    
    if (ret < 0) {
        printf("Error: %s\n", strerror(errno));
        quit("Create_Dir_SS3: make directory to %s not successful as ret: %d\n", dirName, ret);
    }
    
    return ret;
}

//******************************************************************************
//
// Name:  SS330Assessment
// Description: do a tier 1 assessment and get RBC for next year
//
// called by:
// calls:
// created:  Oct 2019 Rich Little
// Modified: Oct 2020 Maciej Golebiewski
//     new Linux implementation that runs ss3 directly, without creating a script
//
//     Beth Fulton brought it into Atantis
//
//******************************************************************************
void SS330Assessment(MSEBoxModel *bm, int sp, int year, FILE *llogfp) {

    char dirName[STRLEN];
    char fileName[STRLEN];
    int ret = 0;

    printf("Starting SS330Assessment");
    snprintf(dirName, sizeof(dirName), "%s_SS3_sim_%d_year_%d", FunctGroupArray[sp].groupCode, bm->RBCestimation.sim, year);
    
    
    /* Create directory */
    ret = Create_Dir_SS3(bm, sp, year, llogfp);  // Failure to create handled within that routine
        
    snprintf(fileName, sizeof(fileName), "WriteSS330Files");
    WriteSS330Files(bm, sp, year, dirName, fileName);
    
    Run_SS3(bm, dirName);  //TODO: Check this will really run SS3 or do I need to call a script?

    printf("End SS3 Assessment for %s\n", FunctGroupArray[sp].groupCode);

        // read max convergence criterion
    bm->RBCestimation.RBCspeciesParam[sp][MaxConvergCrit_id] = Read_SS3_Par_File(bm, sp, year, dirName);
    if (bm->RBCestimation.RBCspeciesParam[sp][MaxConvergCrit_id] > bm->RBCestimation.MaxCritConvergeValue){   // This was originally 0.01 but Sally had changed it to 0.1, RatPack had a value of 10.0
        bm->RBCestimation.RBCspeciesParam[sp][AssessFail_id] = 1;
        fprintf(llogfp, "yr: %d WARNING - SS assessment failed as MaxConvergCrit for %s is %e (vs %e)\n", year, FunctGroupArray[sp].groupCode, bm->RBCestimation.RBCspeciesParam[sp][MaxConvergCrit_id], bm->RBCestimation.MaxCritConvergeValue);
    } else {
    // read next year+1 catches from report.sso
        Read_SS3_Report_File(bm, sp, year, dirName);
    }

    if (bm->RBCestimation.RBCspeciesArray[sp].RBC_by_year[year] < 0.0 || bm->RBCestimation.RBCspeciesArray[sp].RBC_by_year[year] > 999999.9) {
        fprintf(llogfp, "Time %e %s SS assessment failed due to RBC = %e\n ", bm->dayt, FunctGroupArray[sp].groupCode, bm->RBCestimation.RBCspeciesArray[sp].RBC_by_year[year]);
        bm->RBCestimation.RBCspeciesParam[sp][AssessFail_id] = 1;
        bm->RBCestimation.RBCspeciesArray[sp].RBC_by_year[year] = 0.0;              // temporary value, will be reset in DoAssessment
        bm->RBCestimation.RBCspeciesParam[sp][RBCest_id] = 0.0;
        bm->RBCestimation.RBCspeciesParam[sp][EstB0_id] = 999.9;
        bm->RBCestimation.RBCspeciesParam[sp][EstBinit_id] = 999.9;
        bm->RBCestimation.RBCspeciesParam[sp][EstBcurr_id] = 999.9;
    }

    bm->RBCestimation.RBCspeciesParam[sp][EstDepletion_id] = bm->RBCestimation.RBCspeciesParam[sp][EstBcurr_id] / (bm->RBCestimation.RBCspeciesParam[sp][EstB0_id] + small_num);

    //fprintf(llogfp, "Time %e %s dataYear: %d EstDepletion: %e RBC: %e\n", bm->dayt, FunctGroupArray[sp].groupCode, bm->RBCestimation.RBCspeciesArray[sp].mgt_dataYear, bm->RBCestimation.RBCspeciesParam[sp][EstDepletion_id], bm->RBCestimation.RBCspeciesArray[sp].RBC_by_year[year]);

    if (bm->RBCestimation.RBCspeciesParam[sp][AssessFail_id]) {
        bm->RBCestimation.RBCspeciesArray[sp].NassessFail++;
    } else {
        bm->RBCestimation.RBCspeciesArray[sp].NassessFail = 0;
    }
}

//******************************************************************************
//
// Name:  Run_SS3
// Description: execute ss3_exe in directory run_dir
//
// Parameters:
//    ss3_exe - path to the executable: can be absolute or relative, actual
//          file or filesystem link
//      run_dir - path to the working directory for this run
//
// Created: Original in Ratpack MSE by Maciej Golebiewski
// Modifications: Beth Fulton
//
void Run_SS3(MSEBoxModel *bm, char *run_dir) {

    char *ss3_abs;
    char ss3_cmd[STRLEN];
    int THIS_PATH_MAX = 1024; // Needed as linux doesn't have PATH_MAX defined like Mac does
    char cwd[THIS_PATH_MAX];
    char buf[THIS_PATH_MAX];
    
    if (getcwd(cwd, sizeof(cwd)) != NULL) {
        // Do nothing has have remembered current directory
    } else {
        quit("getcwd() error\n");
    }

    // resolve link or relative path to an absolute path
    ss3_abs = realpath(bm->ss3Name, buf);
    if (ss3_abs) {
        // Real path found
    } else {
        // print explanatory error message
        int errc = errno;
        char *errStr = strerror(errno);
        quit("Failed to resolve: %s - Error: %d %s\n", bm->ss3Name, errc, errStr);
    }

    // build command string to pass to system
    snprintf(ss3_cmd, sizeof(ss3_cmd), "%s -nohess > mse.junk", ss3_abs);

    // change to rundir and execute the ss3 binary
    chdir(run_dir);
    printf("Running %s in dir %s\n", ss3_cmd, run_dir);
    int ret = system(ss3_cmd);

    if (ret != 0) {
        printf("ss3 return code: %d - possibly an error?\n", ret);
    }

    // go back to previous directory
    chdir(cwd);
    free(ss3_abs);
    //free(cwd);  // TODO: This correct or cause dump?

} // Run_SS3
