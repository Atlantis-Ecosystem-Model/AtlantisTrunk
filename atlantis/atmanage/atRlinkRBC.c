/**
 * 
 * Author: Beth Fulton
 *
 *  SS Based Multispecies Assessment R calls
 */

#define  _GNU_SOURCE
#include <stdio.h>
#include <string.h>
#include <math.h>
#include <sjwlib.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <errno.h>
#include <unistd.h>
#include <signal.h>
#include <ctype.h>
#include <time.h>
#include <stdarg.h>
#include "atManage.h"
#include "atRlink.h"

#ifdef RASSESS_LINK_ENABLED

#include <Rinternals.h>
#include <Rembedded.h>
#include <R_ext/Parse.h>

/**
 * Clean up directory before continuing with SS and PGMSY -s o aviod corruption between runs
 * TODO: FIX - check if more files need to be removed
 *
 */
void cleanPGMSYFiles(MSEBoxModel *bm) {
    char const *Metname = "Metier.dat";
    char const *outname = "MULTRBC.RBC";
    
    /* Remove PGMSY.R input file Metname */
    remove(Metname);
    
    /* Remove PGMSY.R output file *outname */
    remove(outname);
    
    return;
}


//static void create_r_object(int numSP, const char** names, int endYear);

/**
 * Setting an object variable in R - general version
 *
void create_r_object(int numSP, const char** names, int endYear)
{

    
}
*/

FILE * initMetierFile(MSEBoxModel *bm) {
    FILE *fp;
    char *fname = "Metier.dat";

    /* Create file */
    if ((fp = Util_fopen(bm, fname, "w")) == NULL) {
        quit("initMetierFile: Can't open %s\n", fname);
    }
    
    /* File content - nothing yet as added in another routine */

    /* Return file pointer */
    return (fp);
}

void writeMetierFile(MSEBoxModel *bm, int year, FILE *fid) {
    int sp, m, yr, nreg, nf, Nregions;

    fprintf(fid, "# Number of species\n");
    fprintf(fid, "%d\n", bm->RBCestimation.MULTISPestNspecies);
    fprintf(fid, "# Key species\n");
    for (sp = 0; sp < bm->K_num_tot_sp; sp++) {
        if (FunctGroupArray[sp].isAssessed == TRUE) {
            fprintf(fid, "%d", (int)(bm->RBCestimation.RBCspeciesParam[sp][mgt_category_id]));
            if (sp < bm->RBCestimation.Lastsp){
                fprintf(fid, " ");
            } else {
                fprintf(fid, "\n");
            }
        }
    }
    
    fprintf(fid, "\n");
    fprintf(fid, "# Total number of metiers\n");
    fprintf(fid, "%d\n", bm->RBCestimation.Nmetiers);
    for (m = 0; m < bm->RBCestimation.Nmetiers; m++){
        fprintf(fid, "%s ", bm->RBCestimation.metierArray[m].metierCode);
        if (m < bm->RBCestimation.LastMetier){
            fprintf(fid, " ");
        } else {
            fprintf(fid, "\n");
        }
    }
    
    fprintf(fid, "\n");
    fprintf(fid, "# Links between fleet for assessed species\n");
    for (m = 0; m < bm->RBCestimation.Nmetiers; m++){
        fprintf(fid, "%d ", bm->RBCestimation.metierArray[m].PGMSYlinks);  // Is this a 1D or 2D array?
        if (m < bm->RBCestimation.LastMetier){
            fprintf(fid, " ");
        } else {
            fprintf(fid, "\n");
        }
    }
    
    fprintf(fid, "\n");
    fprintf(fid, "# Last data year\n");
    for (sp = 0; sp < bm->K_num_tot_sp; sp++) {
        if (FunctGroupArray[sp].isAssessed == TRUE) {
            fprintf(fid, "%d ", bm->RBCestimation.RBCspeciesArray[sp].mgt_dataYear);
            if (sp < bm->RBCestimation.Lastsp){
                fprintf(fid, " ");
            } else {
                fprintf(fid, "\n");
            }
        }
    }
    
    fprintf(fid, "# Assessment year\n");
    for (sp = 0; sp < bm->K_num_tot_sp; sp++) {
        if (FunctGroupArray[sp].isAssessed == TRUE) {
            fprintf(fid, "%d ", bm->RBCestimation.RBCspeciesArray[sp].mgt_assessYear);  // Is this the same as bm->RBCestimation.RBCspeciesArray[species].CurrentYear
            if (sp < bm->RBCestimation.Lastsp){
                fprintf(fid, " ");
            } else {
                fprintf(fid, "\n");
            }
        }
    }
    
    fprintf(fid, "# First RBC year\n");
    for (sp = 0; sp < bm->K_num_tot_sp; sp++) {
        if (FunctGroupArray[sp].isAssessed == TRUE) {
            fprintf(fid, "%d ", bm->RBCestimation.RBCspeciesArray[sp].mgt_rbcYear);
            if (sp < bm->RBCestimation.Lastsp){
                fprintf(fid, " ");
            } else {
                fprintf(fid, "\n");
            }
        }
    }
    
    fprintf(fid, "# Assessment\n");
    for (sp = 0; sp < bm->K_num_tot_sp; sp++) {
        if (FunctGroupArray[sp].isAssessed == TRUE) {
            fprintf(fid, "%d", (int)(FunctGroupArray[sp].speciesParams[tier_id]));
            if (sp < bm->RBCestimation.Lastsp){
                fprintf(fid, " ");
            } else {
                fprintf(fid, "\n");
            }
        }
    }
    
    fprintf(fid, "# Which fleet related to which metier\n");
    for (sp = 0; sp < bm->K_num_tot_sp; sp++) {
        if (FunctGroupArray[sp].isAssessed == TRUE) {
            for (m = 0; m < bm->RBCestimation.RBCspeciesParam[sp][NumFisheries_id]; m++) {
                fprintf(fid, "%d", bm->RBCestimation.speciesMetierToRPFleet[sp][m]);
                if ((sp == bm->RBCestimation.Lastsp) && (m == (bm->RBCestimation.RBCspeciesParam[sp][NumFisheries_id] - 1))) {
                    fprintf(fid, "\n");
                } else {
                    fprintf(fid, " ");
                }
            }
        }
    }
    
    fprintf(fid, "# Which metiers related to which fleets - fleet codes in metier order - for the number of fleets recognised as active for that fishery in the assessment\n");
    for (sp = 0; sp < bm->K_num_tot_sp; sp++) {
        if (FunctGroupArray[sp].isAssessed == TRUE) {
            for (m = 0; m < bm->RBCestimation.RBCspeciesParam[sp][NumFisheries_id]; m++) {
                fprintf(fid, "%d", bm->RBCestimation.speciesRPFleetToMetier[m][sp]);
                if ((sp == bm->RBCestimation.Lastsp) && (m == (bm->RBCestimation.RBCspeciesParam[sp][NumFisheries_id] - 1))) {
                    fprintf(fid, "\n");
                } else {
                    fprintf(fid, " ");
                }
            }
        }
    }
    
    fprintf(fid, "# Catches by fleet\n");
    //Was a loop from year-15, switching to 
    for (yr = bm->RBCestimation.HistYrMin; yr < year; yr++) {
        fprintf(fid, "%d ", yr);
        //fflush(fid);
        for (sp = 0; sp < bm->K_num_tot_sp; sp++) {
            if (FunctGroupArray[sp].isAssessed == TRUE) {
                Nregions = (int)(bm->RBCestimation.RBCspeciesParam[sp][NumRegions_id]);
                for (m = 0; m < bm->RBCestimation.RBCspeciesParam[sp][NumFisheries_id]; m++) {
                    for (nreg = 0; nreg < Nregions ; nreg++){
                        nf = bm->RBCestimation.speciesMetierToRPFleet[sp][m];
                        fprintf(fid, "%e", bm->RBCestimation.RBCspeciesArray[sp].CatchData[nf][nreg][yr]);
                        if ((sp == bm->RBCestimation.Lastsp) && (m == (bm->RBCestimation.RBCspeciesParam[sp][NumFisheries_id] - 1))) {
                            fprintf(fid, "\n");
                        } else {
                            fprintf(fid, " ");
                        }
                    }
                }
            }
        }
     }
    
    return;
}

/*
 * Reads in the file MULTRBC.RBC output by PGMSY.R
 *
 * THis assumes the ID of the species is noted and then each line of the projection putput is stored (read in)m as
 *
 * Year RBC allocations per metiers (sum to RBC)
 *
 */

void Read_PGMSY_output(MSEBoxModel *bm, int year, FILE *llogfp) {
    double TheRBC, AveRBC, StateRBC, StateCatchOrig;
    char const *outname = "MULTRBC.RBC";
    FILE *fp;
    char buffer[STRLEN];
    int ThisSpID,spID, i, j, m, nf, sp;
    int nrow = bm->assessnyr;
    char seps[] = " ,\t";
    int size = bm->RBCestimation.Nmetiers + 2;
    double *values = (double *) malloc((size_t)size * sizeof(double));
    char *line_buf = NULL;
    //char dirname[STRLEN];
    //char *varStr;
    //double val, lastval = 0.0;
    //char *valueCopy = (char *)malloc(sizeof(char) *STRLEN);
    //int line_count = 0;
    size_t line_buf_size = 0;
    size_t line_size;

    //valueCopy  = strcpy(valueCopy, valueStr);

    printf(" Read_PGMSY_output: reading results back in\n");

    if(verbose) {
        printf(" Read_PGMSY_output: reading results back in\n");
    }
    
    if ((fp = fopen(outname, "rt")) == NULL) {
        quit("Cannot open PGMSY output file %s\n", outname);
    }
    
    // Get the first line of the file
    line_size = getline(&line_buf, &line_buf_size, fp);
    if(line_size != 4) {
        quit("Error in %s - expected header row not present\n", outname);
    }
    
    // Loop through the file
    
    for (int sp = 0; sp < bm->RBCestimation.MULTISPestNspecies; sp++) {

        // Read in species of interest
        line_size = getline(&line_buf, &line_buf_size, fp);
        if(line_size == 1) {
            ThisSpID = atoi(line_buf);
            spID = bm->RBCestimation.MULTISPestIDs[ThisSpID];
        }
        
        // Read in data lines
        // TODO: Will need to fix this
        i = 0;
        while (fgets(buffer, sizeof buffer, fp) && (i < nrow)) {
            const char* tok;
            j = 0;

            // Assign values
            for (tok = strtok(buffer, seps); tok && *tok; j++, tok = strtok(NULL, "\n")) {
                values[j] = atof(tok);
                //fprintf(bm->logFile, "%f\t", values[j]);
            }
            //fprintf(bm->logFile,"\n");

            // Apply values to correct arrays - assuming the following format for values
            // TheYear TheRBC  and all the rest are the catch per metier
            
            // Assign projection RBC
            bm->RBCestimation.RBCspeciesArray[sp].RBC_by_year[year] = values[1];  // Nead all years (i) as used in calculating Average RBC below
            if (year == values[0]) {
                // Store current RBC
                bm->RBCestimation.RBCspeciesParam[spID][RBCest_id] = values[1];
            }
            // Assign metier catches
            for (m = 0; m < bm->RBCestimation.RBCspeciesParam[spID][NumFisheries_id]; m++) {
                nf = bm->RBCestimation.speciesRPFleetToMetier[m][spID];
                bm->RBCestimation.Catch_by_Metier[spID][year][i][nf] = values[2+m];
            }
            
            i++;
        }
    }

    // Read in Depletion - #TODO
    
    /* Handle the RBC */
    for (sp = 0; sp < bm->K_num_tot_sp; sp++) {
        if ((FunctGroupArray[sp].isAssessed == TRUE) && (bm->RBCestimation.RBCspeciesParam[sp][UseRBCAveraging_id])){
            // Calculate the average RBC
            AveRBC = 0;
            for (int yr = bm->RBCestimation.RBCspeciesArray[sp].mgt_rbcYear; yr < bm->RBCestimation.RBCspeciesArray[sp].mgt_rbcYear + bm->RBCestimation.nRBCaverage - 1; yr++) {
                AveRBC += bm->RBCestimation.RBCspeciesArray[sp].RBC_by_year[yr];
            }
            AveRBC /= ((float)(bm->RBCestimation.nRBCaverage));

            // Replace the RBCs
            bm->RBCestimation.RBCspeciesArray[sp].RBC_by_year[year] = AveRBC;
            //fprintf(bm->logFile, "Time: %e Final RBC (PGMSY) for %s now has RBC_by_year: %e\n", bm->dayt, FunctGroupArray[sp].groupCode, bm->RBCestimation.RBCspeciesArray[sp].RBC_by_year[year]);
        }
    }
    
    free1d(values);
    fclose(fp);

    return;
}



void Do_PGMSY_R(MSEBoxModel *bm, int year, FILE *llogfp) {
    //int sp = 0;
    //int fleet = 0;
    //int yr = 0;
    //int usef = 0;
    //char str[1000];
    //double Checkcatch;
    //double AveRBC;
    char R_ScriptName[150];
    char dirname[STRLEN];
    int ret = 0;
    int idR = 0;

    /* Create directory */
    snprintf(dirname, sizeof(dirname), "%s/MultispeciesAssessment_sim_%d_year_%d", bm->destFolder, bm->RBCestimation.sim, year);
    ret = mkdir(dirname, 0777);  //Was S_IRWXU but was failing to allow file creation
    
    if (ret == -1) {
        switch (errno) {
            case EACCES :
                quit("Do_PGMSY_R: the parent directory does not allow write\n");
                break;
            case EEXIST:
                warn("Do_PGMSY_R: pathname %s already exists so cleaning up before starting\n", dirname);
                cleanPGMSYFiles(bm);
                break;
            case ENAMETOOLONG:
                quit("Do_PGMSY_R: pathname %s is too long\n ", dirname);
                break;
            default:
                quit("Do_PGMSY_R: mkdir failed for %s\n", dirname);
                break;
        }
    }
    
    /* multiSpeciesAssess_R.bat */
    if(!bm->metierFile) {
        printf("Attempting to create MetierFile\n");
        bm->metierFile = initMetierFile(bm);
    }
    writeMetierFile(bm, year, bm->metierFile);
    Util_Close_Output_File(bm->metierFile);

    /* Change directory into the output directory to run PGMSY*/
    ret = chdir(bm->destFolder);
    
    // if the change of directory was successful it will print successful otherwise it will print not successful
    if (ret < 0)
        quit("Do_PGMSY_R: chdir change of directory to %s not successful\n", bm->destFolder);
    else
        printf("Do_PGMSY_R: chdir change of directory to %s successful\n", bm->destFolder);
    /**/
     
    /* Commands to run PGMSY */
    printf("Running PGMSY.R - filename: %s\n", bm->RAssessRscriptName[idR]);
    
    snprintf(R_ScriptName, sizeof(R_ScriptName), "Rscript %s", bm->RAssessRscriptName[idR]);
    system(R_ScriptName);  // Actual call to run PGMSY in R

    /* Read output back into RBCestimation */
    Read_PGMSY_output(bm, year, llogfp);
    
    /*** Clean up ***/
    // Change directory back up to input directory
    ret = chdir(bm->inputFolder);
    // if the change of directory was successful it will print successful otherwise it will print not successful
    if (ret < 0)
        quit("Do_PGMSY_R: chdir change of directory to %s not successful\n", bm->inputFolder);
    else
        printf("Do_PGMSY_R: chdir change of directory to %s successful\n", bm->inputFolder);
     /**/
    
    printf("PGMSY.R finished\n");

    
}

#endif
