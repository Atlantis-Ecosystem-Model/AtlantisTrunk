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
    //GenData(bm, species, year); // Not working for RAssess parameterization
    
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
    sprintf(fname,"%s_%s",FunctGroupArray[species].groupCode, bm->RAssessRinName);
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
void Populate_RAssessFile(FILE *fp, MSEBoxModel *bm, int species){
  int cohort, this_age, b, nf;
  int this_year = (int)(bm->dayt/365.0) + bm->RAssessRefYear - 1; 
  double catchnum, discardnum, catchsize, catchlen, counter1, this_size, this_length, numspring, numautumn, calcM; 
  
  
  for (cohort = 0; cohort < FunctGroupArray[species].numCohortsXnumGenes; cohort++) {
    
    this_age = cohort * FunctGroupArray[species].ageClassSize;
    fprintf(fp,"%d%s", this_year, bm->RassessColDelimiter); // calendar year - could be model year if needed
    fprintf(fp,"%d%s", this_age, bm->RassessColDelimiter); // age – in years
    
    // TODO: Talk to RAssess about implications of cohort being multipe years
    
    catchnum = 0.0;
    discardnum = 0.0;
    catchsize = 0.0;
    catchlen = 0.0;
    counter1 = 0.0;
    this_size = 0.0;
    this_length = 0.0;

    
    for (nf = 0; nf < bm->K_num_fisheries; nf++) {
      for (b = 0; b < bm->nbox; b++) {
        catchnum += FunctGroupArray[species].SizeNumCaught[cohort][nf][b]; //Numbers caught
        discardnum += FunctGroupArray[species].SizeNumDiscard[cohort][nf][b]; //Numbers discarded
        catchsize += FunctGroupArray[species].SizeCaught[cohort][nf][b]; //Biomass caught
      }
    }
    if (catchnum > 0.0) {
      catchsize /= catchnum;
    } else {
      catchsize = 0.0;
    }
    
    catchlen = Ecology_Get_Size(bm, species, catchsize, cohort);
    catchsize = catchsize * bm->X_CN * mg_2_g; //mg_2_g (0.02) includes wet weight
    
    numspring = 0.0;
    numautumn = 0.0;
    for (b = 0; b < bm->nbox; b++) {
      numspring += FunctGroupArray[species].RAssessSpringSurvey[cohort][b];
      numautumn += FunctGroupArray[species].RAssessAutumnSurvey[cohort][b];
      
      if (FunctGroupArray[species].RAssessSpringSurveySize[cohort][b] > 0.0) {
        this_size += FunctGroupArray[species].RAssessSpringSurveySize[cohort][b];
        counter1 += 1.0;
      }
    }
    
    if (counter1 > 0.0) {
      this_size /= counter1;
    } else {
      this_size = 0.0;
    }
    
    this_length = Ecology_Get_Size(bm, species, this_size, cohort);
    this_size = this_size * bm->X_CN * mg_2_g;
    
    fprintf(fp,"%f%s", catchnum + discardnum, bm->RassessColDelimiter);  // total catch (in numbers at age)
    fprintf(fp,"%f%s", catchlen, bm->RassessColDelimiter); // mean length at age in the catch in cm
    fprintf(fp,"%f%s", catchsize, bm->RassessColDelimiter); // mean weight at age in the catch in grams
    fprintf(fp,"%f%s", numspring, bm->RassessColDelimiter); // area swept abundance by age
    fprintf(fp,"%f%s", this_size, bm->RassessColDelimiter); // weight at age, in grams
    fprintf(fp,"%f%s", this_length, bm->RassessColDelimiter); // length at age in cm
    fprintf(fp,"%f%s", FunctGroupArray[species].scaled_FSPB[cohort], bm->RassessColDelimiter); // proportion mature by age
    fprintf(fp,"%f%s", numautumn, bm->RassessColDelimiter); // area swept abundance by age
    
    if(bm->RAssessFixedM) {
      // If a parameter setting
      fprintf(fp,"%f%s", FunctGroupArray[species].speciesParams[assess_nat_mort_id], bm->RassessColDelimiter);
    } else {
      // If using dynamic value
      calcM = 0.0;
      fprintf(fp,"%f%s", calcM, bm->RassessColDelimiter);
    }
    
    fprintf(fp,"%f", catchsize*catchnum/1e+6);  // Landings in tonnes
    fprintf(fp,"\n");
  }
  fflush(fp);
  
  return;
}


/* Do survey data collection for purposes of RAssess estimate */
void RAssessSurvey(MSEBoxModel *bm, FILE *llogfp) {
    int springSurvey = 0;
    int sp, cohort, bb, b, den, sn, rn, k;
    double sample_num, survey_num, q, avail, rawwgt, rawlngth, rawn;
    int fishery_id = bm->RAssessFisheryID;

    if (bm->TofY == bm->RAssessSpringSurveyDay) {
        springSurvey = 1;
    }
    
    // Survey the species
    for (sp = 0; sp < bm->K_num_tot_sp; sp++) {
        if (FunctGroupArray[sp].isTAC > 1) {
            
          fprintf(bm->logFile, "Doing RAssess survey (springSurvey = %d) for %s\n",springSurvey, FunctGroupArray[sp].groupCode);
          fflush(bm->logFile);
            
            for (cohort = 0; cohort < FunctGroupArray[sp].numCohortsXnumGenes; cohort++) {
                for (bb = 0; bb < bm->nsbox; bb++) {
                    b = bm->nsboxes[bb];

                    den = FunctGroupArray[sp].NumsTracers[cohort];
                    sn = FunctGroupArray[sp].structNTracers[cohort];
                    rn = FunctGroupArray[sp].resNTracers[cohort];
                
                    // Just use max size as do mixing in movement so unless sedentary then all same size anyways
                    rawwgt = 0.0;
                    rawn = 0.0;		
                    for (k=0; k < bm->boxes[b].nz; k++) {
                      rawn += bm->boxes[b].tr[k][den];
                      if ((bm->boxes[b].tr[k][sn] + bm->boxes[b].tr[k][rn]) > rawwgt) {
                        rawwgt = bm->boxes[b].tr[k][sn] + bm->boxes[b].tr[k][rn];
                      }
                    }
                    
                    // This sampling method needs checking, i.e adding error to weight
                    rawlngth = Ecology_Get_Size(bm, sp, rawwgt, cohort);
                    avail = bm->SP_FISHERYprms[sp][fishery_id][avail_id];
                    q = Selectivity(bm, rawlngth, fishery_id, avail, 0, bm->logFile);  // Also accounts for availability
                    sample_num = Assess_Add_Error(bm, flagcount, rawn, k_avgcount, k_varcount);
                    survey_num = sample_num * q;
                
                    if (springSurvey) {
                        FunctGroupArray[sp].RAssessSpringSurvey[cohort][b] = survey_num;
                        FunctGroupArray[sp].RAssessSpringSurveySize[cohort][b] = rawwgt;
                    } else {
                        FunctGroupArray[sp].RAssessAutumnSurvey[cohort][b] = survey_num;
                        FunctGroupArray[sp].RAssessAutumnSurveySize[cohort][b] = rawwgt;
                    }
                    
                    fprintf(bm->logFile, "RAssess Survey: survey_num = %e, rawwgt = %e for ageclass = %d in box = %d\n", survey_num, rawwgt, cohort, b);
                    fflush(bm->logFile);
                }
            }
        }
    }
    
    return;
}
           
/** Reading in the file defining the TAC or Effort. 

The TAC file should have the following format
 
 fishery_id TAC
 ID XX
 
 where ID is the index of fisheries as in fishery.csv file
 and XX is the value of the TAC e.g.
 
 fishery_id TAC
 0 2e+07       
 1 0e+00          
 2 0e+00        
 3 0e+00   
 
 The Effort file should have the following format matching the parameters needed for effortmodel = 3
 meff and effort are the same value for each box per fishery
 
 box	fishery_id	Effort_hdistrib1	Effort_hdistrib2	Effort_hdistrib3	Effort_hdistrib4	meff1	meff2	meff3	meff4	effort
 0	1	0	0	0	0	1	1	0	1.3	20000
 1	1	0.2	0.2	0.2	0.2	1	1	0	1.3	20000
 2	1	0.3	0.3	0.3	0.3	1	1	0	1.3	20000
 0	3	0	0	0	0	1	1	0	2	5000
 1	3	0.2	0.2	0.2	0.2	1	1	0	2	5000
 2	3	0.3	0.3	0.3	0.3	1	1	0	2	5000
 
 */

void Read_RAssess_output(MSEBoxModel *bm, int species, int year, FILE *llogfp) {
  char outname[STRLEN];
  FILE *fp;
  char buffer[STRLEN];
  
  
  sprintf(outname, "%s_%s",
          FunctGroupArray[species].groupCode,
          bm->RAssessRoutName);
  
  if ((fp = Open_Input_File(bm->destFolder, outname, "rt")) == NULL) {
    quit("Cannot open R generated TAC output file %s\n", outname);
  }
  
  /* Read header line */
  if (fgets(buffer, sizeof(buffer), fp) == NULL) {
    fprintf(llogfp,
            "ERROR-RAssess: Empty R generated TAC output file %s\n",
            outname);
    fflush(llogfp);
    fclose(fp);
    quit("Error: empty R generated TAC output file %s\n", outname);
  }
  
  

  /* If header includes "TAC" the TAC values are read in */
  if (strstr(buffer, "TAC") != NULL) {
    int fishery_id;
    double tac_val;
    double total_tac = 0.0;
    
    fprintf(llogfp,
            "RAssess: TAC format detected in %s for species %s\n",
            outname,
            FunctGroupArray[species].groupCode);
  
  
  while (fgets(buffer, sizeof(buffer), fp) != NULL) {
    if (buffer[0] == '\n' || buffer[0] == '\0') {
      continue;
    }
    
    /* Checking format and input of the output file */
    if (sscanf(buffer, "%d%lf", &fishery_id, &tac_val) != 2) {
      fprintf(llogfp,
              "WARNING-RAssess: Skipping malformed TAC line in %s: %s\n", outname, buffer);
      fflush(llogfp);
      continue;
    }
    
    if (fishery_id < 0 || fishery_id >= bm->K_num_fisheries) {
      fprintf(llogfp,
              "WARNING-RAssess: Fishery index %d out of range in %s (valid range 0-%d)\n",
              fishery_id, outname, bm->K_num_fisheries - 1);
      fflush(llogfp);
      continue;
    }
    
    if (tac_val < 0.0) {
      fprintf(llogfp,
              "WARNING-RAssess: Negative TAC value %.3f for species %s, fishery %d in %s\n",
              tac_val,
              FunctGroupArray[species].groupCode,
              fishery_id,
              outname);
    }
    /* Read TAC into array that is used to scale mFC and Effort */
    bm->TACamt[species][fishery_id][now_id] = tac_val;
    total_tac += tac_val;
    
    fprintf(llogfp,
            "RAssess: Updated TAC for species %s, fishery %d to %.3f for year %d\n",
            FunctGroupArray[species].groupCode, fishery_id, tac_val, year);
  }
  
  /* Store total TAC across fisheries for this species */
  bm->RBCestimation.RBCspeciesParam[species][RBCest_id] = total_tac;
  
  fprintf(llogfp,
          "RAssess: %s total TAC across fisheries set to %.3f\n",
          FunctGroupArray[species].groupCode, total_tac);
  
  } else if (strstr(buffer, "Effort") != NULL || strstr(buffer, "effort") != NULL) {
    
    int b, fishery_id;
    double Effort_hdistrib1, Effort_hdistrib2;
    double Effort_hdistrib3, Effort_hdistrib4;
    double meff1, meff2, meff3, meff4;
    double effort;
    
    fprintf(llogfp,
            "RAssess: Effort format detected in %s for species %s\n",
            outname,
            FunctGroupArray[species].groupCode);
    
    while (fgets(buffer, sizeof(buffer), fp) != NULL) {
      if (buffer[0] == '\n' || buffer[0] == '\0') {
        continue;
      }
      
      if (sscanf(buffer, "%d%d%lf%lf%lf%lf%lf%lf%lf%lf%lf",
                 &b, &fishery_id,
                 &Effort_hdistrib1, &Effort_hdistrib2,
                 &Effort_hdistrib3, &Effort_hdistrib4,
                 &meff1, &meff2, &meff3, &meff4,
                 &effort) != 11) {
                 
                 fprintf(llogfp,
                         "WARNING-RAssess: Skipping malformed effort line in %s: %s",
                         outname, buffer);
        fflush(llogfp);
        continue;
      }
      
      if (b < 0 || b >= bm->nbox) {
        fprintf(llogfp,
                "WARNING-RAssess: Box index %d out of range in %s "
                "(valid range 0-%d)\n",
                b, outname, bm->nbox - 1);
        fflush(llogfp);
        continue;
      }
      
      if (fishery_id < 0 || fishery_id >= bm->K_num_fisheries) {
        fprintf(llogfp,
                "WARNING-RAssess: Fishery index %d out of range in %s "
                "(valid range 0-%d)\n",
                fishery_id, outname, bm->K_num_fisheries - 1);
        fflush(llogfp);
        continue;
      }
      
      bm->Effort_hdistrib[b][fishery_id][0] = Effort_hdistrib1;
      bm->Effort_hdistrib[b][fishery_id][1] = Effort_hdistrib2;
      bm->Effort_hdistrib[b][fishery_id][2] = Effort_hdistrib3;
      bm->Effort_hdistrib[b][fishery_id][3] = Effort_hdistrib4;
      
      mEff[fishery_id][0] = meff1;
      mEff[fishery_id][1] = meff2;
      mEff[fishery_id][2] = meff3;
      mEff[fishery_id][3] = meff4;
      
      bm->FISHERYprms[fishery_id][EffortLevel_id] = effort;
      
      fprintf(llogfp,
              "RAssess: Updated effort for box %d, fishery %d: "
              "hdistrib for seasons 1-4 = (%.3f %.3f %.3f %.3f) "
              "meff for seasons 1-4 = (%.3f %.3f %.3f %.3f) effort = %.3f\n",
              b,
              fishery_id,
              Effort_hdistrib1,
              Effort_hdistrib2,
              Effort_hdistrib3,
              Effort_hdistrib4,
              meff1,
              meff2,
              meff3,
              meff4,
              effort);
    }
  } else {
    fprintf(llogfp,
            "RAssess: No TAC or Effort found in header of %s for species %s. "
            "No values were read.\n",
            outname,
            FunctGroupArray[species].groupCode);
  }
  
  fflush(llogfp);
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
        sprintf(R_ScriptName,"Rscript %s_%s",FunctGroupArray[species].groupCode, bm->RAssessRscriptName[idR]);
        system(R_ScriptName);
    } else {
        // Using embedded R call
        sprintf(inScriptName,"%s_%s",FunctGroupArray[species].groupCode, bm->RAssessRscriptName[0]);
        sprintf(outScriptName,"%s_%s",FunctGroupArray[species].groupCode, bm->RAssessRoutName);
        
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
