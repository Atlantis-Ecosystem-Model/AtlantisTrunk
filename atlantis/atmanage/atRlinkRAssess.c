/**
 * 
 * Author: Beth Fulton
 *
 *  RAssess relevant R calls
 * Assumes Atlantis outputs a file that R script reads, does an assessment and returns the TAC.
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
#include <R_ext/Utils.h>

static void Populate_RAssessFile(FILE *fp, MSEBoxModel *bm, int species);
static void Read_RAssess_output(MSEBoxModel *bm, int species, int year, FILE *llogfp);
static void Run_RAssess(MSEBoxModel *bm, int species, FILE *RAssessfp);

static FILE * Init_RAssess_File(MSEBoxModel *bm, int species);

void Do_RAssess(MSEBoxModel *bm, int species, int year, FILE *llogfp) {
    int idnum = FunctGroupArray[species].RAssessFileNum;

    printf("Do RAssess calculations for %s\n", FunctGroupArray[species].groupCode);
    
    if (idnum < 0) {
        quit("Trying to do RAssess calculations for %s but it is not flagged as an age structured species under TAC management in groups.csv file or has a negative script number in the assess.prm file\n", FunctGroupArray[species].groupCode);
    }
    
    // Reuse SS data generation code
    GenData(bm, species, year);
    
    // Create and Popuate RAssessFile
    if(!bm->RAssess_initiated[species]) {
        bm->RAssessFnames[idnum] = Init_RAssess_File(bm, species);
    }
    
    // Write out for RAssess output structure
    Populate_RAssessFile(bm->RAssessFnames[idnum], bm, species);
    
    // Call R and run RAssess
    Run_RAssess(bm, species, bm->RAssessFnames[idnum]);
    
    // If not done already (which will be if using embedded calls), read TAC and rerun information for use in Atlantis
    if (bm->RAssessRuseScript){
        Read_RAssess_output(bm, species, year, llogfp);
    }

    return;
}

/* Intialising input file for RAssess */
FILE * Init_RAssess_File(MSEBoxModel *bm, int species) {
    FILE *fp;
    char fname[STRLEN];

    /** Create filename **/
    snprintf(fname, sizeof(fname), "%s_%s", FunctGroupArray[species].groupCode, bm->RAssessRinName);
    printf("Init_RAssess_File: Creating %s\n",fname);
    
    /* Create file */
    if ((fp = Util_fopen(bm, fname, "w")) == NULL)
        quit("Init_RAssess_File: Can't open %s\n", fname);

    fprintf(fp,"year%sage%scatch%scatch_length%scatch_weight%ssurvey_spring%sstock_weight%ssurvey_length%smaturity%ssurvey_autumn%sM%stotal_landings\n", bm->RassessColDelimiter, bm->RassessColDelimiter, bm->RassessColDelimiter, bm->RassessColDelimiter, bm->RassessColDelimiter, bm->RassessColDelimiter, bm->RassessColDelimiter, bm->RassessColDelimiter, bm->RassessColDelimiter, bm->RassessColDelimiter, bm->RassessColDelimiter);
    
    bm->RAssess_initiated[species] = 1;
    
    /* Return file pointer */
    return (fp);

}

/* Write RAssess input file */
void Populate_RAssessFile(FILE *fp, MSEBoxModel *bm, int groupIndex){
    int cohort, istock, isex, r, b, nf, this_age;
    int iyr = (int)(bm->dayt/365.0);
    int Nregions = (int)(bm->RBCestimation.RBCspeciesParam[groupIndex][NumRegions_id]);
    int this_year = (int)(bm->dayt/365.0) + bm->RAssessRefYear - 1;  // Assumes that as happens on Day 365 it would count first year as RefYear + 1 so need to -1 to get it to RefYear
    double catchnum, discardnum, totlandings, catchsize, catchlen, counter1, counter2, this_size, this_length,
        numspring, numautumn, calcM;

    for (cohort = 0; cohort < FunctGroupArray[groupIndex].numCohortsXnumGenes; cohort++) {
        this_age = cohort * FunctGroupArray[groupIndex].ageClassSize;
        fprintf(fp,"%d%s", this_year, bm->RassessColDelimiter); // calendar year - could be model year if needed
        fprintf(fp,"%d%s", this_age, bm->RassessColDelimiter); // age – in years
        
        // TODO: Talk to RAssess about implications of cohort being multipe years
        
        catchnum = 0.0;
        discardnum = 0.0;
        totlandings = 0.0;
        catchsize = 0.0;
        counter1 = 0.0;
        counter2 = 0.0;
        this_size = 0.0;
        this_length = 0.0;
        
        for (istock = 0; istock < FunctGroupArray[groupIndex].numStocks; istock++){
            for (isex = 0; isex < bm->K_num_sexes; isex++){
                this_size += bm->RBCestimation.RBCspeciesArray[groupIndex].MeanWtAge[istock][isex][cohort][iyr] * 1000.0; // convert weight to g from kg
                this_length += bm->RBCestimation.RBCspeciesArray[groupIndex].MeanLenAge[istock][isex][cohort];
                counter1 += 1.0;
            }
        }
        this_size /= counter1;
        this_length /= counter1;
        
        for (nf = 0; nf < bm->K_num_fisheries; nf++) {
            for (r = 0; r < Nregions; r++) {
                totlandings += bm->RBCestimation.RBCspeciesArray[groupIndex].CatchData[nf][r][iyr] / 1000.0;  // As going kg to tonnes
            }
            for (b = 0; b < bm->nbox; b++) {
                catchnum += FunctGroupArray[groupIndex].SizeNumCaught[cohort][nf][b]; //
                discardnum += FunctGroupArray[groupIndex].SizeNumDiscard[cohort][nf][b];
                catchsize += FunctGroupArray[groupIndex].SizeCaught[cohort][nf][b];
                counter2 += 1.0;
            }
        }
        catchsize /= counter2;
        catchlen = Get_Length(bm, catchsize, groupIndex);
        
        numspring = 0.0;
        numautumn = 0.0;
        for (b = 0; b < bm->nbox; b++) {
            numspring += FunctGroupArray[groupIndex].RAssessSpringSurvey[cohort][b];
            numautumn += FunctGroupArray[groupIndex].RAssessAutumnSurvey[cohort][b];
        }
        
        fprintf(fp,"%f%s", catchnum + discardnum, bm->RassessColDelimiter);  // total catch (in numbers at age)
        fprintf(fp,"%f%s", catchlen, bm->RassessColDelimiter); // mean length at age in the catch in cm
        fprintf(fp,"%f%s", catchsize, bm->RassessColDelimiter); // mean weight at age in the catch in grams
        fprintf(fp,"%f%s", numspring, bm->RassessColDelimiter); // area swept abundance by age
        fprintf(fp,"%f%s", this_size, bm->RassessColDelimiter); // weight at age, in grams
        fprintf(fp,"%f%s", this_length, bm->RassessColDelimiter); // length at age in cm
        fprintf(fp,"%f%s", FunctGroupArray[groupIndex].scaled_FSPB[cohort], bm->RassessColDelimiter); // proportion mature by age
        fprintf(fp,"%f%s", numautumn, bm->RassessColDelimiter); // area swept abundance by age
        
        if(bm->RAssessFixedM) {
            // If a parameter setting
            fprintf(fp,"%f%s", FunctGroupArray[groupIndex].speciesParams[assess_nat_mort_id], bm->RassessColDelimiter);
        } else {
            // If using dynamic value
            calcM = 0.0;
            for (istock = 0; istock < FunctGroupArray[groupIndex].numStocks; istock++){
                calcM += (bm->calcTrackedMort[groupIndex][cohort][istock][finalM1_id] + bm->calcTrackedMort[groupIndex][cohort][istock][finalM2_id]);
            }
            fprintf(fp,"%f%s", calcM, bm->RassessColDelimiter);
        }
        
        fprintf(fp,"%f", totlandings);  // In tonnes
        fprintf(fp,"\n");
    }
    fflush(fp);
    
    return;
}

/* Do survey data collection for purposes of RAssess estimate */
void RAssessSurvey(MSEBoxModel *bm, FILE *llogfp) {
    int springSurvey = 0;
    int sp, cohort, bb, b, den, sn, rn, k;
    double sample_num, survey_num, q, avail, swept_area, rawwgt, rawlngth, rawn, this_wgt;
    int fishery_id = bm->RAssessFisheryID;

    if (bm->TofY == bm->RAssessSpringSurveyDay) {
        springSurvey = 1;
    }
    
    // Survey the species
    for (sp = 0; sp < bm->K_num_tot_sp; sp++) {
        if (FunctGroupArray[sp].isTAC > 1) {
            
            printf("Doing RAssess Survey for %s\n", FunctGroupArray[sp].groupCode);
            
            for (cohort = 0; cohort < FunctGroupArray[sp].numCohortsXnumGenes; cohort++) {
                for (bb = 0; bb < bm->nsbox; bb++) {
                    b = bm->nsboxes[bb];

                    den = FunctGroupArray[sp].NumsTracers[cohort];
                    sn = FunctGroupArray[sp].structNTracers[cohort];
                    rn = FunctGroupArray[sp].resNTracers[cohort];
                
                    // Just use max size as do mixing in movement so undless sedentary then all same size anyways
                    for (k = 0; k < bm->boxes[b].nz; k++){
                        this_wgt = (bm->boxes[b].tr[k][sn] + bm->boxes[b].tr[k][rn]);
                        if (this_wgt > rawwgt) {
                            rawwgt = this_wgt;
                        }
                    }
                
                    rawlngth = Get_Length(bm, rawwgt, sp);
                    avail = bm->SP_FISHERYprms[sp][fishery_id][avail_id];
                    q = Selectivity(bm, rawlngth, fishery_id, avail, 0, bm->logFile);  // Also accounts for availability
                    swept_area = bm->FISHERYprms[fishery_id][swept_area_id] / (bm->boxes[b].area * bm->boxes[b].botz);
                    rawn = bm->boxes[b].tr[k][den];
                    sample_num = Assess_Add_Error(bm, flagcount, rawn, k_avgcount, k_varcount);
                    survey_num = sample_num * q * swept_area;
                
                    if (springSurvey) {
                        FunctGroupArray[sp].RAssessSpringSurvey[cohort][b] = survey_num;
                    } else {
                        FunctGroupArray[sp].RAssessAutumnSurvey[cohort][b] = survey_num;
                    }
                }
            }
        }
    }
    
    return;
}
           
/** Reading in the file defining the TAC. Has an assumed format of
 
 TAC
 XX
 
 where XX is the value of the TAC e.g.
 
 TAC
 10000
 */
void Read_RAssess_output(MSEBoxModel *bm, int species, int year, FILE *llogfp) {
    char outname[STRLEN];
    char ch;
    FILE *fp;
    char *valueStr;
    char buffer[STRLEN];
    char seps[] = " ,\t";
    int size = 100;  // TODO: Set this to sensible value
    int buflen = 200;
    double *values = (double *) malloc((size_t)size * sizeof(double));
    char *line_buf = NULL;
    size_t line_buf_size = 0;
    size_t line_size;
    
    if(verbose) {
        printf("Read_RAssess_output: reading results back in for %s\n", FunctGroupArray[species].groupCode);
    }

    /** Create filename - was _Rassess.out**/
    snprintf(outname, sizeof(outname), "%s_%s", FunctGroupArray[species].groupCode, bm->RAssessRoutName);
    
    if ((fp = Open_Input_File(bm->destFolder,outname, "rt")) == NULL) {
        quit("Cannot open R generated output file %s\n", outname);
    }
    
    // Get the first line of the file
    line_size = getline(&line_buf, &line_buf_size, fp);
    /*
    if(line_size > 2) {
        quit("Error in %s - expected header row not present\n", outname);
    }
    */
    
    // Read in data lines
    /* Loop for all lines */
    fseek(fp, 0L, 0);
    while (fgets(buffer, buflen, fp) != NULL) {
        ch = buffer[0];

        if (ch == '\n')
            continue;

        if (strcmp(buffer, "TAC") == 0){
            // Do nothing as header row
        } else {
            valueStr = strtok(buffer, seps);
            if(strlen(valueStr) != 0) {
                bm->RBCestimation.RBCspeciesParam[species][RBCest_id] = atof(valueStr);
            }
        }
    }
    fprintf(bm->logFile, "%s RAssessTAC set to %e\n", FunctGroupArray[species].groupCode, bm->RBCestimation.RBCspeciesParam[species][RBCest_id]);
    fflush(bm->logFile);
    
    free1d(values);
    fclose(fp);

    return;
}



/* Run RAssess from R */
void Run_RAssess(MSEBoxModel *bm, int species, FILE *RAssessfp) {
    char R_ScriptName[150];
    char inScriptName[150];
    char outScriptName[150];
    double Rret;
    int errorOccurred = 0;
    int idR = FunctGroupArray[species].speciesParams[whichRAssess_id];
    
    printf("Run_RAssess for species %s idR %d RAssessRuseScript %d\n", FunctGroupArray[species].groupCode, idR, bm->RAssessRuseScript);
    
    if(idR < 0) {
        quit("whichRAssess is negative for %s so can't complete Run_RAssess\n", FunctGroupArray[species].groupCode);
    }
    
    if (bm->RAssessRuseScript) {
        // Using a system call
        snprintf(R_ScriptName, sizeof(R_ScriptName), "Rscript %s_%s", FunctGroupArray[species].groupCode, bm->RAssessRscriptName[idR]);
        system(R_ScriptName);
    } else {
        // Using embedded R call
        snprintf(inScriptName, sizeof(inScriptName), "%s_%s", FunctGroupArray[species].groupCode, bm->RAssessRscriptName[0]);
        snprintf(outScriptName, sizeof(outScriptName), "%s_%s", FunctGroupArray[species].groupCode, bm->RAssessRoutName);
        
        SEXP infile = PROTECT(allocVector(STRSXP, 1));
        SET_STRING_ELT(infile, 0, mkChar(inScriptName));

        SEXP outfile = PROTECT(allocVector(STRSXP, 1));
        SET_STRING_ELT(outfile, 0, mkChar(outScriptName));
        
        SEXP outdir = PROTECT(allocVector(STRSXP, 1));
        SET_STRING_ELT(outdir, 0, mkChar(bm->destFolder));

        // Setup a call to the R RAssess function, will need other function calls embedded in that?
        SEXP r_call;
        PROTECT(r_call = lang4(install("doRAssess"), infile, outfile, outdir));

        // Execute the function
        SEXP ret = R_tryEval(r_call, R_GlobalEnv, &errorOccurred);

        if (!errorOccurred) {
            printf("RRAssess: R returned TAC value of: ");
            Rret = REAL(ret)[0];
            printf("%0.5f, ", Rret);
            printf("\n");
        } else {
            printf("Error occurred calling R\n");
            Rret = -1;
        }

        UNPROTECT(3);

        // Check interrupts
        R_CheckUserInterrupt();

        // Assign the TAC value
        if(Rret > -1) {
            bm->RBCestimation.RBCspeciesParam[species][RBCest_id] = Rret;
            fprintf(bm->logFile, "%s RAssessTAC set to %e\n", FunctGroupArray[species].groupCode, bm->RBCestimation.RBCspeciesParam[species][RBCest_id]);
        } else {
            fprintf(bm->logFile, "%s RAssessTAC not set as had R error\n", FunctGroupArray[species].groupCode);
        }
        fflush(bm->logFile);

    }
    

    return;
}

/**
 * Initialize R environment and associated arrays
 */

void RRAssess_Linkage_Start(MSEBoxModel *bm) {
    int i;
    int num_files_needed = 0;
    int r_argc = 2;
    char *r_argv[] = { "R", "--silent" };
    
    if(!bm->RAssessRuseScript) {
        printf("RRAssess_Linkage_Start: initialise R\n");
        
        // load R if not doing a system call direct
        Rf_initEmbeddedR(r_argc, r_argv);

        // Load R functions script
        source(bm->RAssessRscriptName[0]);
    }
    
    bm->RAssess_initiated = (int *) i_alloc1d(bm->K_num_tot_sp);
    for (i=0; i < bm->K_num_tot_sp; i++) {
        bm->RAssess_initiated[i] = 0;
        
        // Count up species that need a RAssess file
        if (FunctGroupArray[i].isTAC > 1) {
            FunctGroupArray[i].RAssessFileNum = num_files_needed;
            num_files_needed++;
        }
    }
    bm->RAssessFnames = malloc( num_files_needed * sizeof(FILE*)); // Ideally only allocate what need so might need to revise this after

    return;
}

/**
 * Release R environment
 */
int freeRRAssess(){
    printf("Releasing R for RRAssess\n");

    // Release R environment
    Rf_endEmbeddedR(0);

    return 0;
}

#endif
