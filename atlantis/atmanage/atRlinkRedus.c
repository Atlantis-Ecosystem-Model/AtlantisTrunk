/**
 * 
 * Original redus Author: Parker Abercrombie <parker@pabercrombie.com>
 * Expansion and Modifications: Beth Fulton
 *
 *  This set of routines handles passing information to and from R harvest control rulea
 */

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

void Redus_Linkage_Start(MSEBoxModel *bm) {
    int sp, numyr;
    char **names;

    // Get species names (do it outside of redus
    printf("Redus_Linkage_Start: Preparing R data structure for %d species\n", bm->K_num_tot_sp);
    numyr = (int)floor(ROUNDGUARD + ((bm->tstop - bm->tstart)/(86400.0*365.0)));
    names = (char**) malloc(sizeof(char*) * bm->K_num_tot_sp + 1);

    for (sp = 0; sp < bm->K_num_tot_sp; sp++) {
        names[sp] = (char*) malloc(sizeof(char) * 5);
        strcpy(names[sp], FunctGroupArray[sp].groupCode);
    }

    names[bm->K_num_tot_sp] = (char*) malloc(sizeof(char) * 5);
    strcpy(names[sp], "");

    printf("Redus_Linkage_Start: Now creating R objects\n");
    create_r_redus_object(bm->K_num_tot_sp, (const char**)names, numyr);
    printf("Redus_Linkage_Start: Finish creating R objects\n");

    return;
}

/**
 * Parse a text statement
 */
int exec_r(const char* str)
{
  int error;
  SEXP code, expr = R_NilValue;
  ParseStatus status;
 
  PROTECT(code = Rf_allocVector(STRSXP, 1));
  SET_STRING_ELT(code, 0, Rf_mkChar(str));
 
  expr = PROTECT(R_ParseVector(code, -1, &status, R_NilValue));
 
  switch (status) {
  case PARSE_OK:
    break;
  case PARSE_INCOMPLETE:
    fprintf(stderr, "Expression incomplete! Missing brackets?\n");
    UNPROTECT(2);
    return -1;
  case PARSE_ERROR:
    fprintf(stderr, "Syntax error!\n");
    UNPROTECT(2);
    return -1;
  default:
    UNPROTECT(2);
    return -1;
  }
   
  int i;
  for (i=0; i<LENGTH(expr); i++) {
    R_tryEval(VECTOR_ELT(expr, i), R_GlobalEnv, &error);
  }
 
  UNPROTECT(2);
 
  return error;
}



/**
 * Setting an object variable in R - redus version
 */
void create_r_redus_object(int numSP, const char** names, int endYear)
{
    
	int sp;
	SEXP tmp;

	// Create biomass index list
	SEXP biomass = PROTECT(mkNamed(VECSXP, names));

	// Create assessment result list
	SEXP assessment = PROTECT(mkNamed(VECSXP, names));

	tmp = Rf_install("biomassIndex");
	Rf_defineVar(tmp, biomass, R_GlobalEnv);

	tmp = Rf_install("assessmentOutput");
	Rf_defineVar(tmp, assessment, R_GlobalEnv);

    tmp = Rf_install("hcrOutput");
    Rf_defineVar(tmp, assessment, R_GlobalEnv);

	for (sp = 0; sp < numSP; sp++) {
		char tmp2[255];

		snprintf(tmp2, sizeof(tmp2), "biomassIndex[[\"%s\"]] <- FLStock(FLQuant(NA, dimnames=list(age=\"all\", year=0:%d), units='t'))", names[sp], endYear);
		exec_r(tmp2);

		snprintf(tmp2, sizeof(tmp2), "assessmentOutput[[\"%s\"]] <- FLQuant(NA, dimnames=list(age=\"all\", year=0:%d))", names[sp], endYear);
		exec_r(tmp2);

		snprintf(tmp2, sizeof(tmp2), "hcrOutput[[\"%s\"]] <- FLQuant(NA, dimnames=list(age=\"all\", year=0:%d))", names[sp], endYear);
		exec_r(tmp2);
	}

	UNPROTECT(2);

	// Check interrupts
	R_CheckUserInterrupt();


	// Check objects
	exec_r("print(ls())");

}


/**
 * Setting an biomass for a species
 */
void redus_update_biomass(int year, char* groupCode, double biomass)
{
    char test[255];

	snprintf(test, sizeof(test), "stock(biomassIndex[[\"%s\"]])[, \"%d\"] <- %f", groupCode, year, biomass);
	//printf("%s\n", test);

	exec_r(test);

	//Check interrupts
	R_CheckUserInterrupt();

}


/**
 * Invokes the command source().
 */
void source(const char *name)
{
    SEXP e;

    PROTECT(e = lang2(install("source"), mkString(name)));
    R_tryEval(e, R_GlobalEnv, NULL);
    UNPROTECT(1);
}

/**
 * Invokes the command HCR.
 */
double redus_do_hcr(int curyearC, char* groupCodeC)
{
    double Rret;

    SEXP groupCode = PROTECT(allocVector(STRSXP, 1));
    SET_STRING_ELT(groupCode, 0, mkChar(groupCodeC));

    SEXP curyear = PROTECT(allocVector(INTSXP, 1));
    INTEGER(curyear)[0] = curyearC;

    // Setup a call to the R  doHCR function
    SEXP hcr_call;
    PROTECT(hcr_call = lang3(install("doHCR"), curyear, groupCode));

    // Execute the function
    int errorOccurred;
    SEXP ret = R_tryEval(hcr_call, R_GlobalEnv, &errorOccurred);

    if (!errorOccurred)
    {
        printf("REDUS: R returned F target value of: ");
        Rret = REAL(ret)[0];
        printf("%0.5f, ", Rret);
        printf("\n");
    }
    else
    {
        printf("Error occurred calling R\n");
        Rret = -1;
    }

    UNPROTECT(3);

    // Check interrupts
    R_CheckUserInterrupt();

    return Rret;
}

/**
 * Invokes the command assessment.
 */
void redus_do_assessment(int minyearC, int maxyearC, char* groupCodeC)
{
    SEXP groupCode = PROTECT(allocVector(STRSXP, 1)); 
    SET_STRING_ELT(groupCode, 0, mkChar(groupCodeC));

    SEXP maxyear = PROTECT(allocVector(INTSXP, 1));
    INTEGER(maxyear)[0] = maxyearC;

    SEXP minyear = PROTECT(allocVector(INTSXP, 1));
    INTEGER(minyear)[0] = minyearC;

    // Setup a call to the R  doAssess function
    SEXP assess_call;
    PROTECT(assess_call = lang4(install("doAssess"), minyear, maxyear, groupCode));

    // Execute the function
    int errorOccurred;
    R_tryEval(assess_call, R_GlobalEnv, &errorOccurred);

    if (errorOccurred)
    {
        printf("Error occurred calling R\n");
    }

    UNPROTECT(4);

    // Check interrupts
    R_CheckUserInterrupt();
     
}

/**
 * Initialize R environment for REDUS
 */
int initRedus(int ans)
{
    // Intialize the R environment.
    int r_argc = 2;
    char *r_argv[] = { "R", "--silent" };
    Rf_initEmbeddedR(r_argc, r_argv);
    
    // Load R functions script
    source("redus.R");
    
    return 0;
}

int freeRedus()
{
    printf("Releasing R for Redus\n");

    // Try to print out all redus objects

    exec_r("print(\"One\n\");print(currentYear)");
    exec_r("print(\"Two\n\");print(biomassIndex)");
    exec_r("print(\"Final: \n\");print(assessmentVector)");

    // Release R environment
    Rf_endEmbeddedR(0);
    
    return 0;
}


void intRedus(int dummy)
{
   // Check interrupts
   R_CheckUserInterrupt();
}


int redus_getRintObject(char *name) {

   SEXP tmp, ret;
   tmp = Rf_install(name);
   ret = Rf_findVar(tmp, R_GlobalEnv);

   return(Rf_asInteger(ret));
    
    return 0;
}

const char** redus_getRvecObject(char *name, int *length) {
    const char **result = NULL;
   int i;
   
   SEXP tmp, ret;
   tmp = Rf_install(name);
   ret = Rf_findVar(tmp, R_GlobalEnv);

   *length = Rf_length(ret);

   result = (const char**) malloc(sizeof(const char**) * *length);

   for(i = 0; i < *length; i++) {
	result[i] = CHAR(STRING_ELT(ret, i));
   }

   return(result);
}

void REDUS_management(MSEBoxModel *bm, FILE *llogfp) {
    double FTARG, Fcurr, F_rescale;
    int year = (int)ceil(bm->dayt / 365);
    int sp, i, nc, k, nf;
	double calcF = 0.0;
    double calcM = 0.0;
    double counter = 0.0;

    /* Conf global (Get from R objects) */
    int collectStart = redus_getRintObject("collectStart");
    int assessStart = redus_getRintObject("assessStart");

    int assessSPnum = 0;
    const char **assessSP = 0;
    assessSP = redus_getRvecObject("assessSpecies", &assessSPnum);
    /* Conf END */

	printf("REDUS: We are at year %d\n", year);

    /* Data collection */
    if( year >= collectStart ) {
        for (sp = 0; sp < bm->K_num_tot_sp; sp++) {
            printf("REDUS: Collect biomass for %s at year %d\n", FunctGroupArray[sp].groupCode, year);
            redus_update_biomass(year, FunctGroupArray[sp].groupCode, bm->totfishpop[sp] * bm->X_CN * mg_2_tonne);
            fprintf(llogfp, "REDUS: Biomass at Time: %e (year: %d) for %s is %e\n", bm->dayt, year, FunctGroupArray[sp].groupCode,  bm->totfishpop[sp] * bm->X_CN * mg_2_tonne);
        }
    }

    /* Do management */
    if( year >= assessStart ) {
        for (sp = 0; sp < bm->K_num_tot_sp; sp++) {
            for (i = 0; i < assessSPnum; i++) {
                printf("REDUS: Is %s (%s) going to be assessed? %d\n", FunctGroupArray[sp].groupCode, assessSP[i], strncmp(FunctGroupArray[sp].groupCode, assessSP[i], 3));
                if (!strncmp(FunctGroupArray[sp].groupCode, assessSP[i], 3)) {
                    printf("REDUS: Doing assessment for %s at year %d\n", FunctGroupArray[sp].groupCode, year);
                    redus_do_assessment(collectStart, year, FunctGroupArray[sp].groupCode);
                    printf("REDUS: Doing HCR for %s at year %d\n", FunctGroupArray[sp].groupCode, year);
                    FTARG = redus_do_hcr(year, FunctGroupArray[sp].groupCode);

                    // Find Fcurr
                    calcF = 0.0;
                    calcM = 0.0;
                    counter = 0.0;
                    for (nc = 0; nc < FunctGroupArray[sp].numCohorts; nc++) {
                        for (k = 0; k < FunctGroupArray[sp].numStocks; k++) {
                            calcM += (bm->calcTrackedMort[sp][nc][k][finalM1_id] + bm->calcTrackedMort[sp][nc][k][finalM2_id]);
                            calcF += bm->calcTrackedMort[sp][nc][k][finalF_id];
                            counter++;
                        }
                    }
                     calcM /= counter;
                    calcF /= counter;
                    Fcurr = calcF;

                    // Apply F rescale
                    //FrefA = FunctGroupArray[sp].speciesParams[FrefA_id];

                    if(Fcurr <= 0 || FTARG <= 0) {
                        F_rescale = 1;
                    } else {
                        //this_mFC = FunctGroupArray[sp].speciesParams[maxmFC_id] * 365.0;  // As FrefA is annual but mFC is daily
                        //Fstep1 = Fcurr / this_mFC;
                        //F_rescale = Fstep1 * (FTARG / (Fcurr + small_num));
                        F_rescale = FTARG / Fcurr;
                        //fprintf(llogfp, "REDUS: FTARG: %e, FrefA: %e, Fcurr: %e, this_mFC: %e, Fstep1: %e\n", FTARG, FrefA, Fcurr, this_mFC, Fstep1);
                    }

                    for (nf = 0; nf < bm->K_num_fisheries; nf++) {
                        bm->SP_FISHERYprms[sp][nf][mFC_scale_id] = F_rescale;
                    }

                    // REDUS log
                    fprintf(llogfp, "REDUS: HCR at Time: %e for %s F_rescale: %e, FTARG: %e, Fcurr: %e\n", bm->dayt, FunctGroupArray[sp].groupCode, F_rescale, FTARG, Fcurr);
                }
            }
        }
    }

    // Free up memory
    free(assessSP);
}

#endif
