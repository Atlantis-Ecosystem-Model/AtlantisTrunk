/* Revisions:
 * Beth Fulton - 8/7/2014
 * Converted all MALE to FEMALE as not likely to have stored MALE data as single sex usually used in Atlantis and FEMALE = 0 be definition
 */

#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <math.h>
#include <ctype.h>
#include <time.h>
#include <string.h>
#include <unistd.h>
#include <sjwlib.h>
#include <atlantisboxmodel.h>
#include <atUtilLib.h>

/* Assessment and fisheries data storage */
#define commerical_id 0
#define survey_id 1
#define sample_id 0

#define EstBcurrIndex 5
#define depletionIndex 8
#define maxConvergCritIndex 16
#define startOfFleetData 10

static void WriteSSDat(MSEBoxModel *bm, FILE *fid, int sp, int maxyr);
/**
 *	Read in the output from the SS3 run. Will read in the RBC (recommended biological catch) and B estimate values.
 *
 *
 */
double Read_SS3_Report_File(MSEBoxModel *bm, int species, int year, char *folderName) {

	char outputFileName[] = "Report.sso";

	FILE *fid;
	int buflen = STRLEN;
	char buf[STRLEN];
    char buffer[1000]; // TODO: Set this to sensible value
	char seps[] = " ";
    int n, j, thisID;
    //int yy;
	char *varStr;
	int fleetIndex;
	double catch;
	double RBC;
    //char *line_buf = NULL;
    //size_t line_buf_size = 0;
    //size_t line_size;
    //int found_LAA = 0;
    int size = 100;  // TODO: Set this to sensible value
    double Len_mid, Len_SD, wt_beg, wt_mid;

    double *values = (double *) malloc((size_t)size * sizeof(double));

	_getcwd(buf, (int)STRLEN);
	printf("getcwd = %s\n", buf);

    if ((fid = fopen(outputFileName, "r")) == NULL) {
		fprintf(bm->logFile,"Read_SS3_Report_File: Can't open %s\n", outputFileName);
        fprintf(bm->logFile, "yr: %d WARNING - Read_SS3_Report_File - SS3 failed to generate a Report.sso output file for %s\n", year, FunctGroupArray[species].groupCode);
        printf("yr: %d WARNING - Read_SS3_Report_File - SS3 failed to generate a Report.sso output file for %s\n", year, FunctGroupArray[species].groupCode);
        bm->RBCestimation.RBCspeciesParam[species][AssessFail_id] = 1;
    }

	/* Need to read in the RBC and total catch*/
	RBC = 0;

	/* Find line that starts with 'FORECAST:_With_F_to_match_adjusted_catch' */
	while (fgets(buf, buflen, fid) != NULL) {

		if (strstr(buf, "FORECAST:_With_F_to_match_adjusted_catch") != NULL) {

			/* ignore the next line */
			fgets(buf, buflen, fid);

			/* grab the data from the next line */
			fgets(buf, buflen, fid);

			varStr = strtok(buf, seps);
			n = 0;

			while (varStr != NULL) {

				varStr = strtok(NULL, seps);
				n++;
                
                /*
                if ( n == year) {
                    yy = atof(varStr);
                }
                 */
                
				/* The 'bio-all' column */
				if (n == EstBcurrIndex) {
					bm->RBCestimation.RBCspeciesParam[species][EstBcurr_id] = atof(varStr);
				}

				/* The 'Depletion' column */
				if (n == depletionIndex) {
					bm->RBCestimation.RBCspeciesParam[species][EstDepletion_id] = atof(varStr);
				}

				if (n >= startOfFleetData)
					break;
			}
            
            /* TODO: Sort this out
            if (yy == (bm->RBCestimation.RBCspeciesParam[species][HistYrMin_id] - 2))
                bm->RBCestimation.RBCspeciesParam[species][EstB0_id];

            if (yy == (bm->RBCestimation.RBCspeciesParam[species][HistYrMin_id] - 2))
                bm->RBCestimation.RBCspeciesParam[species][EstBinit_id];

			//bm->RBCestimation.RBCspeciesParam[species][EstB0_id] = bm->RBCestimation.RBCspeciesParam[species][EstBcurr_id] / bm->RBCestimation.RBCspeciesParam[species][EstDepletion_id];
             */

			fleetIndex = 0;
			n = 0;
			RBC = 0;
			/* Now deal with the fleets */
			while (varStr != NULL) {

				/* Want the 'Retain_B-1' column */
				if (n == 2) {
					catch = atof(varStr);
					RBC += catch;
				}

				varStr = strtok(NULL, seps);
				n++;

				if (n >= 8) {
					fleetIndex++;
					n = 0;
				}
                
                bm->RBCestimation.RBCspeciesArray[species].RBC_by_year[year] = RBC;
                bm->RBCestimation.RBCspeciesParam[species][RBCest_id] = RBC;
			}
        }
        
        // Check on fit
        if (strstr(buf, "Biology_at_age_in_endyr_with_CV=f(LAA)") != NULL) {
            //found_LAA = 1;
            
            for (int sex = 0; sex < bm->RBCestimation.RBCspeciesParam[species][Nsexes_id]; sex++) {
                for (int age = 0; age < bm->RBCestimation.RBCspeciesParam[species][AccumAge_id]; age++) {
                
                    const char* tok;
                    thisID = (int)(bm->RBCestimation.RBCspeciesParam[species][HistYrMin_id]);
                    j = 0;

                    // Assign values
                    for (tok = strtok(buffer, seps); tok && *tok; j++, tok = strtok(NULL, "\n")) {
                        values[j] = atof(tok);
                        //fprintf(bm->logFile, "%f\t", values[j]);
                    }

                    Len_mid = values[12];
                    Len_SD = values[14];
                    wt_beg = values[15];
                    wt_mid = values[16];
                    
                    fprintf(bm->logFile,"Time: %e %s sex %d age %d Len_mid: %e Len_SD: %e wt_beg: %e  wt_mid: %e\n",bm->dayt, FunctGroupArray[species].groupCode, sex, age, Len_mid, Len_SD, wt_beg, wt_mid);
                    
                    if ((fabs(bm->RBCestimation.RBCspeciesArray[species].MeanLenAgeM[0][sex][age][thisID] - Len_mid) / Len_mid) > bm->RBCestimation.ConvergeThresh) {
                        fprintf(bm->logFile,"Time: %e %s sex %d age %d - problem with  Len_mid: %e vs MeanLenAge: %e\n", bm->dayt, FunctGroupArray[species].groupCode, sex, age, Len_mid, bm->RBCestimation.RBCspeciesArray[species].MeanLenAgeM[0][sex][age][thisID]);
                    }
                    if ((fabs(bm->RBCestimation.RBCspeciesArray[species].SigmaLenAgeM[0][sex][age][thisID] - Len_SD) / Len_SD) > bm->RBCestimation.ConvergeThresh) {
                        fprintf(bm->logFile,"Time: %e %s sex %d age %d - problem with  Len_SD: %e vs SigmaLenAgeM: %e\n", bm->dayt, FunctGroupArray[species].groupCode, sex, age, Len_SD, bm->RBCestimation.RBCspeciesArray[species].SigmaLenAgeM[0][sex][age][thisID]);
                    }
                    if ((fabs(bm->RBCestimation.RBCspeciesArray[species].MeanWtAgeS[0][sex][age][thisID] - wt_beg) / wt_beg) > bm->RBCestimation.ConvergeThresh) {
                        fprintf(bm->logFile,"Time: %e %s sex %d age %d - problem with  wt_beg: %e vs MeanWtAgeS: %e\n", bm->dayt, FunctGroupArray[species].groupCode, sex, age, wt_beg, bm->RBCestimation.RBCspeciesArray[species].MeanWtAgeS[0][sex][age][thisID]);
                    }
                    if ((fabs(bm->RBCestimation.RBCspeciesArray[species].MeanWtAgeM[0][sex][age][thisID] - wt_mid) / wt_mid) > bm->RBCestimation.ConvergeThresh) {
                        fprintf(bm->logFile,"Time: %e %s sex %d age %d - problem with  wt_mid: %e vs MeanWtAgeM: %e\n", bm->dayt, FunctGroupArray[species].groupCode, sex, age, wt_mid, bm->RBCestimation.RBCspeciesArray[species].MeanWtAgeM[0][sex][age][thisID]);
                    }
                }
            }
            break;
        }
	}
    
    printf("%s Something has gone wrong reading in the SS3 report file. Look at the Report.sso: Biology_at_age_in_endyr_with_CV=f(LAA)\n", FunctGroupArray[species].groupCode);
    fprintf(bm->logFile,"Time: %e %s Something has gone wrong reading in the SS3 report file. Look at the Report.sso: Biology_at_age_in_endyr_with_CV=f(LAA)\n", bm->dayt, FunctGroupArray[species].groupCode);

	fprintf(bm->logFile,"Time: %e %s EstB0 = %e, RBC = %e\n", bm->dayt, FunctGroupArray[species].groupCode, bm->RBCestimation.RBCspeciesParam[species][EstB0_id], RBC);

    free1d(values);
    
	return RBC;
}

/**
 *    Read in the output from the SS3 run. Will read in the RBC (recommended biological catch) and B estimate values.
 *        Version written by Bec that only picks up EstB0, RBC etc
 *
 */
double Read_SS3_Report_File_EstBo_Only(MSEBoxModel *bm, int species, int year, char *folderName) {

    char outputFileName[] = "Report.sso";

    FILE *fid;
    int buflen = STRLEN;
    char buf[STRLEN];
    char seps[] = " ";
    int n;
    char *varStr;
    int fleetIndex;
    double catch;
    double RBC;

    _getcwd(buf, (int)STRLEN);
    printf("getcwd = %s\n", buf);

    if ((fid = fopen(outputFileName, "r")) == NULL) {
        fprintf(bm->logFile,"Read_SS3_Report_File: Can't open %s\n", outputFileName);
        fprintf(bm->logFile, "yr: %d WARNING - Read_SS3_Report_File - SS3 failed to generate a Report.sso output file for %s\n", year, FunctGroupArray[species].groupCode);
        printf("yr: %d WARNING - Read_SS3_Report_File - SS3 failed to generate a Report.sso output file for %s\n", year, FunctGroupArray[species].groupCode);
        bm->RBCestimation.RBCspeciesParam[species][AssessFail_id] = 1;
    }

    /* Need to read in the RBC and total catch*/
    RBC = 0;

    /* Find line that starts with 'FORECAST:_With_F_to_match_adjusted_catch' */
    while (fgets(buf, buflen, fid) != NULL) {

        if (strstr(buf, "FORECAST:_With_F_to_match_adjusted_catch") != NULL) {

            /* ignore the next line */
            fgets(buf, buflen, fid);

            /* grab the data from the next line */
            fgets(buf, buflen, fid);

            varStr = strtok(buf, seps);
            n = 0;

            while (varStr != NULL) {

                varStr = strtok(NULL, seps);
                n++;

                /* The 'bio-all' column */
                if (n == EstBcurrIndex) {
                    bm->RBCestimation.RBCspeciesParam[species][EstBcurr_id] = atof(varStr);
                }

                /* The 'Depletion' column */
                if (n == depletionIndex) {
                    bm->RBCestimation.RBCspeciesParam[species][EstDepletion_id] = atof(varStr);
                }

                if (n >= startOfFleetData)
                    break;
            }

            bm->RBCestimation.RBCspeciesParam[species][EstB0_id] = bm->RBCestimation.RBCspeciesParam[species][EstBcurr_id] / bm->RBCestimation.RBCspeciesParam[species][EstDepletion_id];


            fleetIndex = 0;
            n = 0;
            RBC = 0;
            /* Now deal with the fleets */
            while (varStr != NULL) {

                /* Want the 'Retain_B-1' column */
                if (n == 2) {
                    catch = atof(varStr);
                    RBC += catch;
                }

                varStr = strtok(NULL, seps);
                n++;

                if (n >= 8) {
                    fleetIndex++;
                    n = 0;
                }
            }
            break;
        }
    }

    printf("EstB0 = %e, RBC = %e\n", bm->RBCestimation.RBCspeciesParam[species][EstB0_id], RBC);
    printf("Read_SS3_Report_File\n");

    return RBC;
}

/**
 *	Read in the output par from the SS3 run, to check for convergence
 *
 *
 */
double Read_SS3_Par_File(MSEBoxModel *bm, int sp, int year, char *folderName) {
    
	char outputFileName[] = "ss3.par";
    
	FILE *fid;
	int buflen = STRLEN;
	char buf[STRLEN];
    char str[STRLEN];
	char seps[] = " ";
	char *varStr;
	double ans = 0.0;
    
	_getcwd(buf, STRLEN);
	printf("getcwd = %s\n", buf);
    
    if ((fid = fopen(outputFileName, "r")) == NULL) {
		fprintf(bm->logFile,"Read_SS3_Report_File: Can't open %s\n", outputFileName);
        ans = 1000.0;
        fprintf(bm->logFile, "yr: %d WARNING - Read_SS3_Par_File - SS3 failed to generate an ss3.par output file for %s\n", year, FunctGroupArray[sp].groupCode);
        printf("yr: %d WARNING - Read_SS3_Par_File - SS3 failed to generate an ss3.par output file for %s\n", year, FunctGroupArray[sp].groupCode);
        return ans;
    }
    
    snprintf(str, sizeof(str), "MGparm[%d]", maxConvergCritIndex);
    
	/* Find line that starts with 'MGparm[16]' - as that marks the convergence result */
	while (fgets(buf, buflen, fid) != NULL) {
        
		if (strstr(buf, str) != NULL) {
            
			/* ignore the next line */
			fgets(buf, buflen, fid);
            
			/* grab the data from the next line */
			fgets(buf, buflen, fid);
            
			varStr = strtok(buf, seps);
			ans = atof(varStr);
		}
	}
    
	printf("MaxConvergCrit = %e for %s\n", ans, FunctGroupArray[sp].groupCode);
    
	//printf("Read_SS3_Par_File\n");
    
	return ans;
}

/**
 *	Create the SS3 run. starter file
 *
 *
 */

void Create_Starter_File(MSEBoxModel *bm, char *dirName, int groupIndex, int versionID) {

	FILE *fid;
    char fileName[STRLEN];
    
	snprintf(fileName, sizeof(fileName), "%s/%sstarter.ss", dirName, FOLDER_SEP);

	printf("fileName = %s\n", fileName);

	if ((fid = fopen(fileName, "w")) == NULL)
		quit("Create_Starter_File: Can't open %s\n", fileName);

	printf("versionID = %d\n", versionID);
	/* Now start writing out the data */

	fprintf(fid, "# SS starter file for %s\n", FunctGroupArray[groupIndex].groupCode);
    fprintf(fid, "#C starter comment here\n");
	//fprintf(fid, "atlantisSS3_%d.dat\n", versionID);
	//fprintf(fid, "atlantisSS3_%d.ctl\n", versionID);

	fprintf(fid, "%s%s.dat\n", FunctGroupArray[groupIndex].groupCode, fileName);
	fprintf(fid, "%s%s.ctl\n", FunctGroupArray[groupIndex].groupCode, fileName);

	fprintf(fid, "0 # 0=use init values in control file; 1=use ss3.par\n");
    fprintf(fid, "0 # run display detail (0,1,2)\n"); // if detailed output (below) is set to 0 no report file is generated.
    
    fprintf(fid, "1 # detailed output (0=minimal for data-limited, 1=high (w/ wtatage.ss_new), 2=brief)\n");
    fprintf(fid, "0 # write 1st iteration details to echoinput.sso file (0,1) \n");
    fprintf(fid, "0 # write parm values to ParmTrace.sso (0=no,1=good,active; 2=good,all; 3=every_iter,all_parms; 4=every,active)\n");
    fprintf(fid, "0 # write to cumreport.sso (0=no,1=like&timeseries; 2=add survey fits)\n");
    fprintf(fid, "1 # Include prior_like for non-estimated parameters (0,1)\n");
    fprintf(fid, "1 # Use Soft Boundaries to aid convergence (0,1) (recommended)\n");
    fprintf(fid, "1 # Number of bootstrap datafiles to produce\n");
    fprintf(fid, "10 # Turn off estimation for parameters entering after this phase\n");
    fprintf(fid, "0 # MCeval burn interval\n");
    fprintf(fid, "1 # MCeval thin interval\n");
    fprintf(fid, "0 # jitter initial parm value by this fraction\n");
    fprintf(fid, "-1 # min yr for sdreport outputs (-1 for styr)\n");
    fprintf(fid, "-2 # max yr for sdreport outputs (-1 for endyr; -2 for endyr+Nforecastyrs\n");
    fprintf(fid, "0 # N individual STD years\n");
    fprintf(fid, "#vector of year values \n");
    fprintf(fid, "%e # final convergence criteria (e.g. 1.0e-04)\n", bm->RBCestimation.SSTol);  // 0.0001
    fprintf(fid, "0 # retrospective year relative to end year (e.g. -4)\n");
    fprintf(fid, "1 # min age for calc of summary biomass\n");
    fprintf(fid, "1 # min age for calc of summary biomass\n");
    fprintf(fid, "%d # Depletion basis:  denom is: 0=skip; 1=rel X*B0; 2=rel X*Bmsy; 3=rel X*B_styr\n", bm->RBCestimation.SSDepletionBasis); //1
    fprintf(fid, "%e # Fraction (X) for Depletion denominator (e.g. 0.4)\n", bm->RBCestimation.SSFractX); // 1.0  neil had 0.41 here
    fprintf(fid, "%d # (1-SPR)_reporting:  0=skip; 1=rel(1-SPR); 2=rel(1-SPR_MSY); 3=rel(1-SPR_Btarget); 4=notrel\n", bm->RBCestimation.SS_SPRreport); //4
    fprintf(fid, "1 # # Annual_F_units: 0=skip; 1=exploitation(Bio); 2=exploitation(Num); 3=sum(Apical_F's); 4=true F for range of ages; 5=unweighted avg. F for range of ages\n");
    fprintf(fid, "#COND 10 15 #_min and max age over which average F will be calculated with F_reporting=4 or 5\n");
    fprintf(fid, "3 # F_std_basis: 0=raw_annual_F; 1=F/Fspr; 2=F/Fmsy ; 3=F/Fbtgt; where F means annual_F\n");
    fprintf(fid, "0 # MCMC output detail: integer part (0=default; 1=adds obj func components); and decimal part (added to SR_LN(R0) on first call to mcmc)\n");
    fprintf(fid, "0 # ALK tolerance (example 0.0001)\n");
    fprintf(fid, "-1 # random number seed for bootstrap data (-1 to use long(time) as seed): # 1585083947\n");
    fprintf(fid, "3.30 # check value for end of file and for version control\n");

	fclose(fid);
}

/**
 *    Create the SS3 run. starter file
 *
 *
 */

void Create_Starter_File_Orig(MSEBoxModel *bm, char *dirName, int groupIndex, char *fileName, int versionID) {

    FILE *fid;

    snprintf(fileName, sizeof(fileName), "%s/%sstarter.ss", dirName, FOLDER_SEP);

    printf("fileName = %s\n", fileName);

    if ((fid = fopen(fileName, "w")) == NULL)
        quit("Create_Starter_File_Orig: Can't open %s\n", fileName);

    printf("versionID = %d\n", versionID);
    /* Now start writing out the data */

    fprintf(fid, "# SS starter file for %s\n", FunctGroupArray[groupIndex].groupCode);
    fprintf(fid, "#C starter comment here\n");
    //fprintf(fid, "atlantisSS3_%d.dat\n", versionID);
    //fprintf(fid, "atlantisSS3_%d.ctl\n", versionID);

    fprintf(fid, "%s%s.dat\n", FunctGroupArray[groupIndex].groupCode, fileName);
    fprintf(fid, "%s%s.ctl\n", FunctGroupArray[groupIndex].groupCode, fileName);

    fprintf(fid, "0 # 0=use init values in control file; 1=use ss3.par\n");
    fprintf(fid, "2 # run display detail (0,1,2)\n");
    fprintf(fid, "0 # detailed age-structured reports in REPORT.SSO (0,1)\n");
    fprintf(fid, "1 # write detailed checkup.sso file (0,1)\n");
    fprintf(fid, "4 # write parm values to ParmTrace.sso (0=no,1=good,active; 2=good,all; 3=every_iter,all_parms; 4=every,active)\n");
    fprintf(fid, "0 # write to cumreport.sso (0=no,1=like&timeseries; 2=add survey fits)\n");
    fprintf(fid, "1 # Include prior_like for non-estimated parameters (0,1)\n");
    fprintf(fid, "1 # Use Soft Boundaries to aid convergence (0,1) (recommended)\n");
    fprintf(fid, "1 # Number of bootstrap datafiles to produce\n");
    fprintf(fid, "6 # Turn off estimation for parameters entering after this phase\n");
    fprintf(fid, "10 # MCMC burn interval\n");
    fprintf(fid, "2 # MCMC thin interval\n");
    fprintf(fid, "0 # jitter initial parm value by this fraction\n");
    fprintf(fid, "-1 # min yr for sdreport outputs (-1 for styr)\n");
    fprintf(fid, "-2 # max yr for sdreport outputs (-1 for endyr; -2 for endyr+Nforecastyrs\n");
    fprintf(fid, "0 # N individual STD years\n");
    fprintf(fid, "%e # final convergence criteria (e.g. 1.0e-04)\n", bm->RBCestimation.SSTol);  // 0.0001
    fprintf(fid, "0 # retrospective year relative to end year (e.g. -4)\n");
    fprintf(fid, "1 # min age for calc of summary biomass\n");
    fprintf(fid, "%d # Depletion basis:  denom is: 0=skip; 1=rel X*B0; 2=rel X*Bmsy; 3=rel X*B_styr\n", bm->RBCestimation.SSDepletionBasis); //1
    fprintf(fid, "%e # Fraction (X) for Depletion denominator (e.g. 0.4)\n", bm->RBCestimation.SSFractX); // 1.0  neil had 0.41 here
    fprintf(fid, "%d # (1-SPR)_reporting:  0=skip; 1=rel(1-SPR); 2=rel(1-SPR_MSY); 3=rel(1-SPR_Btarget); 4=notrel\n", bm->RBCestimation.SS_SPRreport); //4
    fprintf(fid, "%d # F_std reporting: 0=skip; 1=exploit(Bio); 2=exploit(Num); 3=sum(frates)\n", bm->RBCestimation.SS_Freport); //1
    fprintf(fid, "%d # F_report_basis: 0=raw; 1=rel Fspr; 2=rel Fmsy ; 3=rel Fbtgt\n", bm->RBCestimation.SSFreportBasis);
    fprintf(fid, "999 # check value for end of file\n");

    fclose(fid);
}


void Write_Forecast_File(MSEBoxModel *bm, char *dirName, int maxyr, int groupIndex, int versionID) {

	//	int f,allregion;
	//	double last_catch;
	FILE *fid;
	char fileName[STRLEN];
	double catch, last_catch;
	int f;
	//int sumregion = 0; /* This isn't going to work - needs to be numRegions + 1 */
    int Nregions = (int)(bm->RBCestimation.RBCspeciesParam[groupIndex][NumRegions_id]);
    int sumregion = Nregions;  // sum over regions

	snprintf(fileName, sizeof(fileName), "%s%sforecast.ss", dirName, FOLDER_SEP);

	printf("fileName = %s\n", fileName);

	if ((fid = fopen(fileName, "w")) == NULL)
		quit("Write_Forecast_File: Can't open %s\n", fileName);

	printf("versionID = %d\n", versionID);
	/* Now start writing out the data */

	fprintf(fid, "#V3.24f\n");
	fprintf(fid, "#C  generic forecast file\n");
	fprintf(fid, "# for all year entries except rebuilder; enter either: actual year, -999 for styr, 0 for endyr, neg number for rel. endyr\n");
	fprintf(fid, "1 # Benchmarks: 0=skip; 1=calc F_spr,F_btgt,F_msy \n");
	fprintf(fid, "2 # MSY: 1= set to F(SPR); 2=calc F(MSY); 3=set to F(Btgt); 4=set to F(endyr) \n");
	fprintf(fid, "%e # SPR target (e.g. 0.40)\n", bm->targ_refA);
	fprintf(fid, "%e # Biomass target (e.g. 0.40)\n", bm->targ_refA);
	fprintf(fid, "#_Bmark_years: beg_bio, end_bio, beg_selex, end_selex, beg_relF, end_relF (enter actual year, or values of 0 or -integer to be rel. endyr)\n");
	fprintf(fid, " 0 0 0 0 0 0\n");
	fprintf(fid, "#  2001 2001 2001 2001 2001 2001 # after processing \n");
	fprintf(fid, "\n");
	fprintf(fid, "1 #Bmark_relF_Basis: 1 = use year range; 2 = set relF same as forecast below#\n");

	fprintf(fid, "%d # Forecast: 0=none; 1=F(SPR); 2=F(MSY) 3=F(Btgt); 4=Ave F (uses first-last relF yrs); 5=input annual F scalar\n", bm->RBCestimation.SSForecastType);  //3
	fprintf(fid, "%d # N forecast years \n", bm->RBCestimation.nFuture);  //2
	fprintf(fid, "%e # F scalar (only used for Do_Forecast==5)\n", bm->RBCestimation.SSFscalar);  //0.2

	fprintf(fid, "#_Fcast_years:  beg_selex, end_selex, beg_relF, end_relF  (enter actual year, or values of 0 or -integer to be rel. endyr)\n");
	fprintf(fid, " 0 0 -1 0\n");
	fprintf(fid, "#  2001 2001 1991 2001 # after processing \n");

	fprintf(fid, "%d # Control rule method (1=catch=f(SSB) west coast; 2=F=f(SSB) ) \n", bm->RBCestimation.SSControlRule); //2
	fprintf(fid, "%e # Control rule Biomass level for constant F (as frac of Bzero, e.g. 0.40); (Must be > the no F level below) \n", bm->RBCestimation.SSControlRuleB); //0.35
	fprintf(fid, "%e # Control rule Biomass level for no F (as frac of Bzero, e.g. 0.10) \n", bm->lim_ref);  // 0.2
	fprintf(fid, "%e # Control rule target as fraction of Flimit (e.g. 0.75) \n", bm->RBCestimation.SSControlRuleTargF);  // 1
	fprintf(fid, "%d #_N forecast loops (1=OFL only; 2=ABC; 3=get F from forecast ABC catch with allocations applied)\n", bm->RBCestimation.SSLoop); //3
	fprintf(fid, "%d #_First forecast loop with stochastic recruitment\n", bm->RBCestimation.SSLoopWithRandRec);  //3
	fprintf(fid, "0 #_Forecast loop control #3 (reserved for future bells&whistles) \n");
	fprintf(fid, "0 #_Forecast loop control #4 (reserved for future bells&whistles) \n");
	fprintf(fid, "0 #_Forecast loop control #5 (reserved for future bells&whistles) \n");
	fprintf(fid, "%d  #FirstYear for caps and allocations (should be after years with fixed inputs) \n", (int)(bm->RBCestimation.RBCspeciesParam[groupIndex][HistYrMax_id] + 1));  // 2010
	fprintf(fid, "0 # stddev of log(realized catch/target catch) in forecast (set value>0.0 to cause active impl_error)\n");
	fprintf(fid, "0 # Do West Coast gfish rebuilder output (0/1) \n");
	fprintf(fid, "%d # Rebuilder:  first year catch could have been set to zero (Ydecl)(-1 to set to 1999)\n", (int)(bm->RBCestimation.RBCspeciesParam[groupIndex][HistYrMax_id] + 1));  // 1999
	fprintf(fid, "-1 # Rebuilder:  year for current age structure (Yinit) (-1 to set to endyear+1)\n");
	fprintf(fid, "1 # fleet relative F:  1=use first-last alloc year; 2=read seas(row) x fleet(col) below\n");

	fprintf(fid, "\n");

	fprintf(fid, "# Note that fleet allocation is used directly as average F if Do_Forecast=4 \n");
	fprintf(fid, "1 # basis for fcast catch tuning and for fcast catch caps and allocation  (2=deadbio; 3=retainbio; 5=deadnum; 6=retainnum)\n");
	fprintf(fid, "# Conditional input if relative F choice = 2\n");
	fprintf(fid, "# Fleet relative F:  rows are seasons, columns are fleets\n");
	fprintf(fid, "#_Fleet:  FISHERY1\n");
	fprintf(fid, "#  1\n");
	fprintf(fid, "# max totalcatch by fleet (-1 to have no max) must enter value for each fleet\n");
	fprintf(fid, " -1\n");
	fprintf(fid, "# max totalcatch by area (-1 to have no max); must enter value for each fleet \n");
	fprintf(fid, " -1\n");
	fprintf(fid, "# fleet assignment to allocation group (enter group ID# for each fleet, 0 for not included in an alloc group)\n");
	fprintf(fid, " 0\n");
	fprintf(fid, "#_Conditional on >1 allocation group\n");
	fprintf(fid, "# allocation fraction for each of: 0 allocation groups\n");
	fprintf(fid, "# no allocation groups\n");
	fprintf(fid, "0 # Number of forecast catch levels to input (else calc catch from forecast F) \n");
	fprintf(fid, "2 # basis for input Fcast catch:  2=dead catch; 3=retained catch; 99=input Hrate(F) (units are from fleetunits; note new codes in SSV3.20)\n");
	fprintf(fid, "# Input fixed catch values\n");
	fprintf(fid, "#Year Seas Fleet Catch(or_F) \n");

	last_catch = 0;
    for (f = 0; f < bm->RBCestimation.RBCspeciesParam[groupIndex][NumFisheries_id]; f++) {
		last_catch = last_catch + bm->RBCestimation.RBCspeciesArray[groupIndex].CatchData[f][sumregion][maxyr];
    }

	for (f = 0; f < bm->RBCestimation.RBCspeciesParam[groupIndex][NumFisheries_id]; f++) {
		catch = bm->RBCestimation.RBCspeciesArray[groupIndex].CatchData[f][sumregion][maxyr] * bm->RBCestimation.RBCspeciesParam[groupIndex][TAC_old_id]
				/ last_catch;
		fprintf(fid, " %d 1 %d %f\n", (int) bm->RBCestimation.RBCspeciesParam[groupIndex][HistYrMin_id] + maxyr + 1, f, catch);
	}

	fprintf(fid, "\n#\n");
	fprintf(fid, "999 # verify end of input \n");

	fclose(fid);

}

//******************************************************************************
//
// Name:  Write_SS_Data_File
// Description: write the generated data to data file for input to SS filename.dat
//              Have data up to maxyr, need TAC for maxyr+2, so estimate catch for maxyr+1
//              This is for an assessment in maxyr+1
//
//
// called by : WriteSSFiles
// calls :
// created  : Nov 2007 Sally
// updated:   Feb 2011 Sally
//
//******************************************************************************
void Write_SS_Data_File(MSEBoxModel *bm, char *dirName, char *fileName, int maxyr, int groupIndex, int versionID) {
	FILE *fid;
	char str[STRLEN];
	//char tempStr[STRLEN];
	//char line1[STRLEN], char line2[STRLEN];
	int fleetIndex, i, t, f, a, iy;
	//double value;
	int nseas = 1;
	//int nAgeBins;
	//int Nsexes = bm->K_num_sexes;
	//int AccumAge = (int)FunctGroupArray[groupIndex].speciesParams[AccumAge_id];  // TODO: Or should this be bm->RBCestimation.RBCspeciesParam[groupIndex][AccumAge_id] ?
	//int HistYrMin;
	int num, it, s, part = 0, gender, l, nfltdisc;
    //int numYears;
	double cpcv, disccv;
	double *emptySex;
	//double Nsex_samp = 1;
    int allregion = (int)(bm->RBCestimation.RBCspeciesParam[groupIndex][NumRegions_id]);

    /*
    if (bm->RBCestimation.RBCspeciesArray[groupIndex].CurrentYear == (bm->RBCestimation.RBCspeciesParam[groupIndex][HistYrMax_id] - 1)) {
		HistYrMin = (int)bm->RBCestimation.RBCspeciesParam[groupIndex][HistYrMin_id];
        //HistYrMin = 0;
    } else {
		HistYrMin = (int)bm->RBCestimation.RBCspeciesArray[groupIndex].CurrentYear + 1 - (int)bm->RBCestimation.RBCspeciesParam[groupIndex][HistYrMin_id];
    }
     */
    
	//printf(bm->logFile, "maxyr = %d\n", maxyr);
	//numYears = maxyr + 1;	// - HistYrMin + 1;

	//fprintf(bm->logFile, "bm->RBCestimation.RBCspeciesParam[groupIndex][HistYrMax_id] = %d\n", (int) bm->RBCestimation.RBCspeciesParam[groupIndex][HistYrMax_id]);
	//printf(bm->logFile, "bm->RBCestimation.RBCspeciesArray[groupIndex].CurrentYear = %d\n", bm->RBCestimation.RBCspeciesArray[groupIndex].CurrentYear);
	//printf(bm->logFile, "numYears = %d\n", numYears);

	//sscanf(bm->t_units, "seconds since %d-%s", &startYear, str);

	snprintf(str, sizeof(str), "%s%s%s", dirName, FOLDER_SEP, fileName);

	if ((fid = fopen(str, "w")) == NULL)
		quit("Write_SS_Data_File: Can't open %s\n", str);

	/* Need the following data:
	 *
	 * bm->CatchRecord[Yr][gropuIndex][age][dataid]
	 *
	 */
    
	fprintf(fid, "SS v3.30.15.03 data file for %s written by Atlantis Start_Time\n", FunctGroupArray[groupIndex].groupCode);
	
    fprintf(fid, "# Model dimensions\n");
	fprintf(fid, "%d #_StartYr\n", (int)(bm->RBCestimation.RBCspeciesParam[groupIndex][HistYrMin_id]));    // This has to have an AD year value (e.g. 1980)
	fprintf(fid, "%d #_EndYr\n", maxyr);
	fprintf(fid, "%d #_Nseas\n", nseas);
	fprintf(fid, "12 #_months/season (currently fixed at 12)\n");
    fprintf(fid, "2 #_Nsubseasons (even number, minimum is 2)\n");
	fprintf(fid, "1 #_spawn_month\n");
	
    fprintf(fid, "%d #_Ngenders: 1, 2, -1  (use -1 for 1 sex setup with SSB multiplied by female_frac parameter)\n", (int)(bm->RBCestimation.RBCspeciesParam[groupIndex][Nsexes_id]));
    //* check the definition of the accumulator age hasn't changed between 3.24 and 3.30

    fprintf(fid, "%d #_Nages=accumulator age, first age is always age 0\n",(int)(bm->RBCestimation.RBCspeciesParam[groupIndex][AccumAge_id]));
    
    fprintf(fid, "1 #_Nareas\n"); // SS will only use 1 area, even if more in the operating model

    //* check SS still only permits 1 area
    fprintf(fid, "%d #_Nfleets (including surveys)\n", (int)(bm->RBCestimation.RBCspeciesParam[groupIndex][NumSurvey_id] +  bm->RBCestimation.RBCspeciesParam[groupIndex][NumFisheries_id]));
    fprintf(fid, "#_fleet_type: 1=catch fleet; 2=bycatch only fleet; 3=survey; 4=ignore\n");
    fprintf(fid, "#_sample_timing: -1 for fishing fleet to use season-long catch-at-age for observations, or 1 to use observation month;  (always 1 for surveys)\n");
    fprintf(fid, "#_fleet_area:  area the fleet/survey operates in \n");
    fprintf(fid, "#_units of catch:  1=bio; 2=num (ignored for surveys; their units read later)\n");
    fprintf(fid, "#_catch_mult: 0=no; 1=yes\n");
    fprintf(fid, "#_rows are fleets\n");
    fprintf(fid, "#_fleet_type timing area units need_catch_mult fleetname\n");
    
    // loop over fleets and surveys
    // We assume that fleets always come before surveys
	for (fleetIndex = 0; fleetIndex < (bm->RBCestimation.RBCspeciesParam[groupIndex][NumFisheries_id] + bm->RBCestimation.RBCspeciesParam[groupIndex][NumSurvey_id]); fleetIndex++) {
        
        //add the fleet type
        if (fleetIndex < bm->RBCestimation.RBCspeciesParam[groupIndex][NumFisheries_id]) {
            if (bm->RBCestimation.speciesRPFleetToMetier[fleetIndex][groupIndex] > -1) {
                fprintf(fid, " 1  -1  1  1  0  FLEET%d # %d in metier %s\n", fleetIndex, fleetIndex, FisheryArray[fleetIndex].fisheryCode);
            } else {
                fprintf(fid, " 1  -1  1  1  0  FLEET%d # %d\n", fleetIndex, fleetIndex);
            }
        } else {
            fprintf(fid, " 3   1  1  2  0  SURVEY%d # %d\n", fleetIndex, fleetIndex - (int)(bm->RBCestimation.RBCspeciesParam[groupIndex][NumFisheries_id]));
        }
	}

    // Bycatch fleet not currently used
    fprintf(fid, "#Bycatch_fleet_input_goes_next\n");
    fprintf(fid, "#a:  fleet index\n");
    fprintf(fid, "#b:  1=include dead bycatch in total dead catch for F0.1 and MSY optimizations and forecast ABC; 2=omit from total catch for these purposes (but still include the mortality)\n");
    fprintf(fid, "#c:  1=Fmult scales with other fleets; 2=bycatch F constant at input value; 3=bycatch F from range of years\n");
    fprintf(fid, "#d:  F or first year of range\n");
    fprintf(fid, "#e:  last year of range\n");
    fprintf(fid, "#f:  not used\n");
    fprintf(fid, "# a   b   c   d   e   f \n");
    // Header for the catch data
    fprintf(fid, "#_Catch data: yr, seas, fleet, catch, catch_se\n");
    fprintf(fid, "#_catch_se:  standard error of log(catch)\n");
    fprintf(fid, "#_NOTE:  catch data is ignored for survey fleets\n");

    // loop over the fleets only as catch data is ignored for survey fleets
	for (fleetIndex = 0; fleetIndex < bm->RBCestimation.RBCspeciesParam[groupIndex][NumFisheries_id]; fleetIndex++) {

        fprintf(fid, "-999 1 %d %e 0.01\n", fleetIndex, bm->RBCestimation.RBCspeciesArray[groupIndex].initialEquilCatch[fleetIndex]);
       //next is a loop over the years
       // t is the year, f is fleet
       for (t = bm->RBCestimation.RBCspeciesParam[groupIndex][HistYrMin_id]; t < maxyr; t++) {
         fprintf(fid, "%d 1 %d ", t, fleetIndex);
            // Apply proportion catch rule for closed area scenarios
            if (bm->RBCestimation.RBCspeciesArray[groupIndex].mgt_Retro_catch && (bm->RBCestimation.RBCspeciesParam[groupIndex][PropClosed_id] > 0) && (t < bm->RBCestimation.RBCspeciesParam[groupIndex][HistYrMax_id])) {   // allocate historic catches to open area by proportion open
                   fprintf(fid,"%e ", ((1.0 - bm->RBCestimation.RBCspeciesArray[groupIndex].PropClosed) * bm->RBCestimation.RBCspeciesArray[groupIndex].CatchData[fleetIndex][allregion][t]));
            } else {
                   fprintf(fid, " %e", bm->RBCestimation.RBCspeciesArray[groupIndex].CatchData[fleetIndex][allregion][t]);   // region=0 is sum over regions
            }
         fprintf(fid, " 0.01\n");
       }
	}

    fprintf(fid, "-9999 0 0 0 0 # terminator for catches\n");
    // end the block of catch data
    // CPUE abundance indices
    fprintf(fid, "#\n");
    fprintf(fid, " #_CPUE_and_surveyabundance_observations\n");
    fprintf(fid, "#_Units:  0=numbers; 1=biomass; 2=F; 30=spawnbio; 31=recdev; 32=spawnbio*recdev; 33=recruitment; 34=depletion(&see Qsetup); 35=parm_dev(&see Qsetup)\n");
    fprintf(fid, "#_Errtype:  -1=normal; 0=lognormal; >0=T\n");
    fprintf(fid, "#_SD_Report: 0=no sdreport; 1=enable sdreport\n");
    fprintf(fid, "#_Fleet Units Errtype SD_Report\n");
    // loop over fleets and surveys
    for (i = 0; i < (bm->RBCestimation.RBCspeciesParam[groupIndex][NumFisheries_id] + bm->RBCestimation.RBCspeciesParam[groupIndex][NumSurvey_id]); i++){
        //add the fleet type
        if(i < bm->RBCestimation.RBCspeciesParam[groupIndex][NumFisheries_id]){
            if (bm->RBCestimation.speciesRPFleetToMetier[i][groupIndex] > -1) {
                fprintf(fid, "%d  1  0  0  # FLEET%d in metier %s\n", i, i, FisheryArray[i].fisheryCode);
            } else {
                fprintf(fid, "%d 1  0  0  # FLEET%d\n", i, i);
            }
        } else {
            fprintf(fid, "%d 1  0  0  # SURVEY%d\n", (int)(i -  bm->RBCestimation.RBCspeciesParam[groupIndex][NumFisheries_id]), i);
        }
    }

    // abundance indices
    //* could add fleet names as comment at the end of each row
    fprintf(fid, "#_yr month fleet obs stderr\n");
    fprintf(fid, "%e %e\n", bm->RBCestimation.RBCspeciesParam[groupIndex][NumFisheries_id], bm->RBCestimation.RBCspeciesParam[groupIndex][NumSurvey_id]);

    for (f = 0; f < (bm->RBCestimation.RBCspeciesParam[groupIndex][NumFisheries_id] + bm->RBCestimation.RBCspeciesParam[groupIndex][NumSurvey_id]); f++) {
        for (t = bm->RBCestimation.RBCspeciesParam[groupIndex][HistYrMin_id]; t < maxyr; t++) {
            if (bm->RBCestimation.RBCspeciesArray[groupIndex].CPUEgen[f][allregion][t] > 0) {
                if (!bm->RBCestimation.RBCspeciesArray[groupIndex].CPUEcv[f]) {   // generate cpue with no error, but still want cv in SS
                    cpcv = 0.2;
                } else {
                    cpcv = bm->RBCestimation.RBCspeciesArray[groupIndex].CPUEcv[f];
                }

                if (bm->RBCestimation.RBCspeciesArray[groupIndex].CPUEgen[f][allregion][t] < 0.01) {
                    bm->RBCestimation.RBCspeciesArray[groupIndex].CPUEgen[f][allregion][t] = 0.01; // this is because SS fails if abund index<=0
                }
                
                fprintf(fid, " %d 7 %e %e\n", t, bm->RBCestimation.RBCspeciesArray[groupIndex].CPUEgen[f][allregion][t], cpcv);
            }
        }
    }
    fprintf(fid, "-9999 1 1 1 1 # terminator for CPUE and survey observations\n");
    fprintf(fid, "#\n");

    // setup for discards
    //* check this works for an assessment with discards
    //* Jemery suggested the flathead and school whiting examples
    Util_Init_1D_Int(bm->RBCestimation.RBCspeciesArray[groupIndex].discfleet, bm->RBCestimation.RBCspeciesParam[groupIndex][NumFisheries_id], 0.0);
    
    nfltdisc = 0;
    num = 0;
    for (t = bm->RBCestimation.RBCspeciesParam[groupIndex][HistYrMin_id]; t < maxyr; t++) {
        for (f = 0; f < bm->RBCestimation.RBCspeciesParam[groupIndex][NumFisheries_id]; f++) {
            if (bm->RBCestimation.RBCspeciesArray[groupIndex].DiscData[f][allregion][t] > 0) {
                num++;
                bm->RBCestimation.RBCspeciesArray[groupIndex].discfleet[f] = 1;
            }
        }
    }
    for (f = 0; f < bm->RBCestimation.RBCspeciesParam[groupIndex][NumFisheries_id]; f++){
        nfltdisc += bm->RBCestimation.RBCspeciesArray[groupIndex].discfleet[f];
    }
    
	fprintf(fid, "%d #_N_fleets_with_discard\n", (int) bm->RBCestimation.RBCspeciesParam[groupIndex][NumFisheries_id]);
	fprintf(fid, "#_discard_units (1=same_as_catchunits(bio/num); 2=fraction; 3=numbers)\n");
	fprintf(fid, "#_discard_errtype:  >0 for DF of T-dist(read CV below); 0 for normal with CV; -1 for normal with se; -2 for lognormal; -3 for trunc normal with CV\n");

    fprintf(fid, "# note, only have units and errtype for fleets with discard\n");
    fprintf(fid, "#Fleet Units Err_type\n");
	for (f = 0; fleetIndex < bm->RBCestimation.RBCspeciesParam[groupIndex][NumFisheries_id]; f++) {
        
        if (bm->RBCestimation.RBCspeciesArray[groupIndex].discfleet[f]) {
            fprintf(fid, "%d %d 0\n", f, (int)(bm->RBCestimation.RBCspeciesParam[groupIndex][DiscType_id]));
        }
    }
    
    //** Number of discard observations doesn't appear to be an input for 3.30
    //*   fprintf(fid, "%d  # number of obsns\n", num);
    for (f = 0; f < bm->RBCestimation.RBCspeciesParam[groupIndex][NumFisheries_id]; f++) {
        for (t = bm->RBCestimation.RBCspeciesParam[groupIndex][HistYrMin_id]; t < maxyr; t++) {
            if (bm->RBCestimation.RBCspeciesArray[groupIndex].DiscData[f][allregion][t] > 0) {
                if (bm->RBCestimation.RBCspeciesArray[groupIndex].DiscCV[f] == 0.0)   {  // generate discards with no error, but still want cv in SS
                    disccv = 0.1;
                } else {
                    disccv = bm->RBCestimation.RBCspeciesArray[groupIndex].DiscCV[f];
                }
                
                fprintf(fid, "1 %d %e %e\n", f, bm->RBCestimation.RBCspeciesArray[groupIndex].DiscData[f][allregion][t], disccv);
            }
        }
    }
    
    // Only write the discard terminator if there are discards
    if(nfltdisc > 0){
        fprintf(fid, "-9999 0 0 0.0 0.0 # terminator for discard data\n");
    } else {
        fprintf(fid, "#-9999 0 0 0.0 0.0 # terminator for discard data\n");
    }
    fprintf(fid, "\n");
    //TODO: Mean body size - needs additional coding if it is going to be used
    fprintf(fid, "0 #_use meanbodysize_data (0/1)  \n");
    fprintf(fid, "#_COND_0 #_DF_for_meanbodysize_T-distribution_like \n");
    fprintf(fid, "# note:  use positive partition value for mean body wt, negative partition for mean body length   \n");

    fprintf(fid, "#_yr month fleet part obs stderr  \n");
    fprintf(fid, "#  -9999 0 0 0 0 0 # terminator for mean body size data   \n");
    fprintf(fid, "#\n");
    //* length data - needs additional coding if methods 2 or 3 are to be used
    fprintf(fid, "# set up population length bin structure (note - irrelevant if not using size data and using empirical wtatage  \n");
    fprintf(fid, "1 # length bin method: 1=use databins; 2=generate from binwidth,min,max below; 3=read vector  \n");
    fprintf(fid, "# binwidth for population size comp (no additional input for option 1)  \n");
    fprintf(fid, "# minimum size in the population (lower edge of first bin and size at age 0.00)  \n");
    fprintf(fid, "# # maximum size in the population (lower edge of last bin)  \n");
    fprintf(fid, "1 # use length composition data (0/1)  \n");
    fprintf(fid, "#_mintailcomp: upper and lower distribution for females and males separately are accumulated until exceeding this level.  \n");
    fprintf(fid, "#_addtocomp:  after accumulation of tails; this value added to all bins  \n");
    fprintf(fid, "#_males and females treated as combined gender below this bin number   \n");
    fprintf(fid, "#_compressbins: accumulate upper tail by this number of bins; acts simultaneous with mintailcomp; set=0 for no forced accumulation  \n");
    fprintf(fid, "#_Comp_Error:  0=multinomial, 1=dirichlet  \n");
    fprintf(fid, "#_Comp_Error2:  parm number  for dirichlet  \n");
    fprintf(fid, "#_minsamplesize: minimum sample size; set to 1 to match 3.24, minimum value is 0.001  \n");
    fprintf(fid, "#_mintailcomp addtocomp combM+F CompressBins CompError ParmSelect minsamplesize  \n");
    
    // loop over fleets and surveys
    for (i = 0; i < bm->RBCestimation.RBCspeciesParam[groupIndex][NumFisheries_id] + bm->RBCestimation.RBCspeciesParam[groupIndex][NumSurvey_id]; i++){
        if(i < bm->RBCestimation.RBCspeciesParam[groupIndex][NumFisheries_id]){
            if(bm->RBCestimation.speciesRPFleetToMetier[i][groupIndex] > -1) {
                fprintf(fid, " 0 1e-007 0 0 0 0 1 # FLEET%d in metier %s\n", i,   FisheryArray[i].fisheryCode);
            } else {
                fprintf(fid, " 0 1e-007 0 0 0 0 1 # FLEET%d\n", i);
            }
        } else {
            fprintf(fid, " 0 1e-007 0 0 0 0 1 # SURVEY%d\n", (int)(i - bm->RBCestimation.RBCspeciesParam[groupIndex][NumFisheries_id]));
        }
    }
    fprintf(fid, "# sex codes:  0=combined; 1=use female only; 2=use male only; 3=use both as joint sexxlength distribution \n");
    fprintf(fid, "# partition codes:  (0=combined; 1=discard; 2=retained  \n");
    
    // setup number of length bins
    // Initialise - TODO: Is emptysex dealt with properly for lengths?
    emptySex = Util_Alloc_Init_1D_Double(bm->RBCestimation.RBCspeciesParam[groupIndex][Nlen_id], 0.0);

    fprintf(fid, "%d #_N_LengthBins; then enter lower edge of each length bin\n", ((int)bm->RBCestimation.RBCspeciesParam[groupIndex][Nlen_id]));
    for (l = 0; l < bm->RBCestimation.RBCspeciesParam[groupIndex][Nlen_id]; l++){
        fprintf(fid, "%e ", bm->RBCestimation.RBCspeciesArray[groupIndex].LoLenBin[l]);
    }
    fprintf(fid, "\n");
    //* add this comment #_yr month fleet sex part Nsamp datavector(female-male)
    //* Code chunk below may be redundant after removing # no LF samples below. Check this with a stock that uses length data
    num = 0;
    for (t = bm->RBCestimation.RBCspeciesParam[groupIndex][HistYrMin_id]; t < bm->RBCestimation.RBCspeciesParam[groupIndex][MaxYr_id]; t++) {
        for (f = 0; f < (bm->RBCestimation.RBCspeciesParam[groupIndex][NumFisheries_id] + bm->RBCestimation.RBCspeciesParam[groupIndex][NumSurvey_id]); f++) {
            for (it = 0; it < 3; it++) {
                for (s = 0; s < bm->RBCestimation.RBCspeciesParam[groupIndex][Nsex_samp_id]; s++) {
                    if (bm->RBCestimation.RBCspeciesArray[groupIndex].LengthFltYr[it][f][t] > 0) {
                        if (bm->RBCestimation.RBCspeciesArray[groupIndex].LFss[f][s][t][it] > bm->RBCestimation.RBCspeciesParam[groupIndex][LFSSlim_id]) {
                            num++;
                        } else {
                            fprintf(fid, "#  sample size %d year %d fleet %d type %d sex %d\n", (int)(bm->RBCestimation.RBCspeciesParam[groupIndex][LFSSlim_id]), t, f, it, s);
                        }
                    }
                }
            }
        }
    }
    // Array of length data
    fprintf(fid, "#_yr month fleet sex part Nsamp datavector(female-male)\n");
    for (f = 0; f < (bm->RBCestimation.RBCspeciesParam[groupIndex][NumFisheries_id] + bm->RBCestimation.RBCspeciesParam[groupIndex][NumSurvey_id]); f++) {
        for (t = bm->RBCestimation.RBCspeciesParam[groupIndex][HistYrMin_id]; t < maxyr; t++) {
            for (it = 0; it < 3; it++) {
                for (s = 0; s < bm->RBCestimation.RBCspeciesParam[groupIndex][Nsex_samp_id]; s++) {
                    if ((bm->RBCestimation.RBCspeciesArray[groupIndex].LengthFltYr[it][f][t] > 0) && (bm->RBCestimation.RBCspeciesArray[groupIndex].LFss[f][s][t][it] > bm->RBCestimation.RBCspeciesParam[groupIndex][LFSSlim_id])) {
                        if (!it) {
                            part = 2;   // retained
                        } else if (it == 1) {
                            part = 0;   // whole
                        } else {
                            part = 1;   // discarded
                        }
                        if (bm->RBCestimation.RBCspeciesParam[groupIndex][Nsex_samp_id] == 1) {
                            gender = 0;   //combined
                        } else {
                            gender = s;
                        }

                        fprintf(fid, "%d 7 %d %d %d %d", t, f, gender, part, bm->RBCestimation.RBCspeciesArray[groupIndex].LFss[f][s][t][it]);
                        
                        // write the vector of lengths dependent on value specified for sex
                        if (!s) {    // females or combined - if 2 gender model write female LFs then male (ignored)
                            for (l = 0; l < bm->RBCestimation.RBCspeciesParam[groupIndex][Nlen_id]; l++) {
                                fprintf(fid, " %d", bm->RBCestimation.RBCspeciesArray[groupIndex].LenComp[f][s][t][it][l]);
                            }
                            if (bm->RBCestimation.RBCspeciesParam[groupIndex][Nsexes_id] > 1) {
                                for (l = 0; l < bm->RBCestimation.RBCspeciesParam[groupIndex][Nlen_id]; l++) {
                                    fprintf(fid, " %e", emptySex[l]);
                                }
                            }
                        } else {    // males - write female lfs (ignored) then male
                            for (l = 0; l < bm->RBCestimation.RBCspeciesParam[groupIndex][Nlen_id]; l++) {
                                fprintf(fid, " %e", emptySex[l]);
                            }
                            for (l = 0; l < bm->RBCestimation.RBCspeciesParam[groupIndex][Nlen_id]; l++) {
                                fprintf(fid, " %d", bm->RBCestimation.RBCspeciesArray[groupIndex].LenComp[f][s][t][it][l]);
                            }
                        }
                        fprintf(fid, "\n");
                    }
                }
            }
        }
    }
    // terminator for the length data
    fprintf(fid, "-9999 ");
    // loop to create 2*n_len_bins +5 zeroes
    for (i=0; i < (2 * bm->RBCestimation.RBCspeciesParam[groupIndex][Nlen_id]) + 5; i++){
        fprintf(fid, "0 ");
    }
    fprintf(fid, "# terminator for Length composition\n");
    // Start of the age composition data
    fprintf(fid, "# Age composition\n");
    
    free(emptySex);
    emptySex = Util_Alloc_Init_1D_Double(bm->RBCestimation.RBCspeciesParam[groupIndex][MaxAge_id], 0.0);
    // Initialise - TODO: Is emptysex dealt with properly for these ages?
    
    fprintf(fid, "%d #_N_age_bins\n", (int)(bm->RBCestimation.RBCspeciesParam[groupIndex][MaxAge_id]+1));
    
    // Note: 3.30.15 requires ages to start from 1 - but note from RL suggest this no longer true as of July 2021
    for (a = 0; a < bm->RBCestimation.RBCspeciesParam[groupIndex][MaxAge_id]; a++){
        fprintf(fid, "%d ", a);
    }
    fprintf(fid, "1   #_N_ageerror_definitions\n");
    for (a = 0; a < bm->RBCestimation.RBCspeciesParam[groupIndex][AccumAge_id]; a++) {    // bm->RBCestimation.RBCspeciesParam[groupIndex][MaxAge_id] +6 is accumulator age + 1
        fprintf(fid, "%f ", a+0.5);
    }
    fprintf(fid, "\n");
    for (a = 0; a < bm->RBCestimation.RBCspeciesParam[groupIndex][AccumAge_id]; a++) {
        fprintf(fid, "%f ", bm->RBCestimation.RBCspeciesArray[groupIndex].Ageing_error[a]);
    }
    fprintf(fid, "\n");
    num = 0;
    // _mintailcomp ...
    fprintf(fid, "#_mintailcomp: upper and lower distribution for females and males separately are accumulated until exceeding this level.  \n");
    fprintf(fid, "#_addtocomp:  after accumulation of tails; this value added to all bins  \n");
    fprintf(fid, "#_males and females treated as combined gender below this bin number   \n");
    fprintf(fid, "#_compressbins: accumulate upper tail by this number of bins; acts simultaneous with mintailcomp; set=0 for no forced accumulation  \n");
    fprintf(fid, "#_Comp_Error:  0=multinomial, 1=dirichlet  \n");
    fprintf(fid, "#_Comp_Error2:  parm number  for dirichlet  \n");
    fprintf(fid, "#_minsamplesize: minimum sample size; set to 1 to match 3.24, minimum value is 0.001  \n");
    fprintf(fid, "#_mintailcomp addtocomp combM+F CompressBins CompError ParmSelect minsamplesize  \n");
    // loop over fleets and surveys
                   
    for (i = 0; i < bm->RBCestimation.RBCspeciesParam[groupIndex][NumFisheries_id] + bm->RBCestimation.RBCspeciesParam[groupIndex][NumSurvey_id]; i++){
        if(i < bm->RBCestimation.RBCspeciesParam[groupIndex][NumFisheries_id]){
            if(bm->RBCestimation.speciesRPFleetToMetier[i][groupIndex] > -1) {
                fprintf(fid, "0 1e-007 1 0 0 0 1  # FLEET%d in metier %s\n", i, FisheryArray[i].fisheryCode);
            } else {
                fprintf(fid, "0 1e-007 1 0 0 0 1  # FLEET%d\n", i);
            }
        } else {
            fprintf(fid, "0 1e-007 1 0 0 0 1  # SURVEY%d\n", (i - (int)(bm->RBCestimation.RBCspeciesParam[groupIndex][NumFisheries_id])));
        }
    }

    for (t = bm->RBCestimation.RBCspeciesParam[groupIndex][HistYrMin_id]; t < maxyr; t++) {
        for (f = 0; f < bm->RBCestimation.RBCspeciesParam[groupIndex][NumFisheries_id];f++) {
            for (it = 0; it < 3; it++) {
                for ( s = 0; s < bm->RBCestimation.RBCspeciesParam[groupIndex][Nsex_samp_id]; s++) {
                    if (bm->RBCestimation.RBCspeciesArray[groupIndex].AgeFltYr[it][f][t] > 0) {
                        if (bm->RBCestimation.RBCspeciesArray[groupIndex].AFss[f][s][t][it] > bm->RBCestimation.RBCspeciesParam[groupIndex][AFSSlim_id]) {
                            num++;
                        } else {
                            fprintf(fid, "#  sample size %d year %d fleet %d type %d sex %d\n", (int)(bm->RBCestimation.RBCspeciesParam[groupIndex][AFSSlim_id]), t, f, it, s);
                        }
                    }
                }
            }
        }
    }

    //fprintf(fid, "%d  #  no age compositions   female then male\n", num);
    fprintf(fid, "2 #_Lbin_method_for_Age_Data: 1=poplenbins; 2=datalenbins; 3=lengths\n");
    //fprintf(fid, "1    #  combine males into females at or below this bin number\n");
    //fprintf(fid, "# Yr sea flt sex typ aer lb1 lb2   ss\n");

    fprintf(fid, "# sex codes:  0=combined; 1=use female only; 2=use male only; 3=use both as joint sex length distribution\n");
    fprintf(fid, "# partition codes:  (0=combined; 1=discard; 2=retained)\n");
    fprintf(fid, "#_yr month fleet sex part ageerr Lbin_lo Lbin_hi Nsamp datavector(female-male)\n");
    /* old headers were fprintf(fid, "#Yr sea flt sex typ aer lb1 lb2   ss     0     1   2 ...\n");
    // loop over the age data
    for (s = 0; s < bm->RBCestimation.RBCspeciesParam[groupIndex][Nsexes_id]; s++) {
        for (a=0; a < bm->RBCestimation.RBCspeciesParam[groupIndex][MaxAge_id]; a++) {
            fprintf(fid, "%d ", a);
        }
        fprintf(fid, "\n");
    }
    */
    //* this might need to be over Nfleets + Nsurveys, do surveys collect age data?
    for (f = 0; f < bm->RBCestimation.RBCspeciesParam[groupIndex][NumFisheries_id]; f++) {
        for (t = bm->RBCestimation.RBCspeciesParam[groupIndex][HistYrMin_id]; t < maxyr; t++) {
            for (it = 0; it < 3; it++) {
                for ( s = 0; s < bm->RBCestimation.RBCspeciesParam[groupIndex][Nsex_samp_id]; s++) {
                    if ((bm->RBCestimation.RBCspeciesArray[groupIndex].AgeFltYr[it][f][t] > 0) && bm->RBCestimation.RBCspeciesArray[groupIndex].AFss[f][s][t][it] > bm->RBCestimation.RBCspeciesParam[groupIndex][AFSSlim_id]) {  //   make ss limit >0 ?
                        if (!it) {
                            part = 2;   // retained
                        } else if (it==1) {
                            part = 0;   // whole
                        } else {
                            part = 1;   // discarded
                        }
                        
                        if (bm->RBCestimation.RBCspeciesParam[groupIndex][Nsex_samp_id] == 1) {
                            gender = 0;   //combined
                        } else {
                            gender = s;
                        }
                        
                        //* this has the lo and hi length bins set to -1
                        fprintf(fid, "%d   7 %d %d %d   1  -1  -1 %d", t, f, gender, part, bm->RBCestimation.RBCspeciesArray[groupIndex].AFss[f][s][t][it]); // sample size
                        
                        if (!s) {    // combined or females - if 2 gender model write female LFs then male (ignored)
                            for (a = 0; a < bm->RBCestimation.RBCspeciesParam[groupIndex][MaxAge_id]; a++) {
                                fprintf(fid, " %d ", bm->RBCestimation.RBCspeciesArray[groupIndex].AgeComp[f][s][t][it][a]);
                            }
                            if (bm->RBCestimation.RBCspeciesParam[groupIndex][Nsexes_id] > 1) {
                                for (a = 0; a < bm->RBCestimation.RBCspeciesParam[groupIndex][MaxAge_id]; a++) {
                                       fprintf(fid, "%e ", emptySex[a]);
                                }
                            }
                        } else {  // males - write female lfs (ignored) then male
                            for (a = 0; a < bm->RBCestimation.RBCspeciesParam[groupIndex][MaxAge_id]; a++) {
                                fprintf(fid, "%e ", emptySex[a]);
                            }
                            for (a = 0; a < bm->RBCestimation.RBCspeciesParam[groupIndex][MaxAge_id]; a++) {
                                fprintf(fid,"%d ", bm->RBCestimation.RBCspeciesArray[groupIndex].AgeComp[f][s][t][it][a]);
                            }
                        }
                        //* I don't think the line below is needed - I need an end line statement though
                        fprintf(fid, "     #### AgeComp(f,s,t,it,a)\n");
                    }
                }
            }
        }
    }

    // terminator for the age data
    fprintf(fid, "-9999 ");
    // loop to create 2*n_age_bins +9 zeroes, where n_age_bins=MaxAge+1 **update the equation**
    for (i=0; i < ( 2 * bm->RBCestimation.RBCspeciesParam[groupIndex][MaxAge_id]) + 10; i++){
        fprintf(fid, "0 ");
    }
    fprintf(fid, "# terminator for Age composition\n");
    fprintf(fid, "#\n");
    // mean size at age obs not currently implemented
    fprintf(fid, "0 #_Use_MeanSize-at-Age_obs (0/1)\n");
    fprintf(fid, "# sex codes:  0=combined; 1=use female only; 2=use male only; 3=use both as joint sexxlength distribution\n");
    fprintf(fid, "# partition codes:  (0=combined; 1=discard; 2=retained\n");
    fprintf(fid, "# ageerr codes:  positive means mean length-at-age; negative means mean bodywt_at_age\n");
    fprintf(fid, "#_yr month fleet sex part ageerr ignore datavector(female-male)\n");
    fprintf(fid, "#                                          samplesize(female-male)\n");
    fprintf(fid, "#\n");
    // Environmental variables (for regime shift)
    if (bm->RBCestimation.RBCspeciesParam[groupIndex][Regime_year_assess_id] > 0) {  //  for  regime shift
        fprintf(fid, "1    #  number of environmental variables: year, variable, value\n");
        //nobs = bm->RBCestimation.RBCspeciesParam[groupIndex][Regime_year_assess_id] - bm->RBCestimation.RBCspeciesParam[groupIndex][HistYrMin_id] + 1;                           //  +1 changed in ratpack 2014
        //fprintf(fid, "%d     #  enviromental observations\n", nobs);
        for (iy = bm->RBCestimation.RBCspeciesParam[groupIndex][HistYrMin_id] - 1; iy < bm->RBCestimation.RBCspeciesParam[groupIndex][Regime_year_assess_id] - 1; iy++)  {  // was bm->RBCestimation.RBCspeciesParam[groupIndex][HistYrMin_id]  changed in ratpack 2014
               fprintf(fid, "%d  1  1\n", iy);
        }
        fprintf(fid, "-9999  0  0\n");
    } else {
        fprintf(fid, "0 #_N_environ_variables\n");
        fprintf(fid, "#Yr Variable Value\n");
    }
    fprintf(fid, "#\n");
    // size frequency not currently implemented
    fprintf(fid, "0 #    N sizefreq methods to read\n");
    fprintf(fid, "#\n");
    // tagging data not currently implemented
    fprintf(fid, "0 #    do tags (0/1)\n");
    fprintf(fid, "#\n");
    // morphcomp not currently implemented
    fprintf(fid, "0 #    morphcomp data (0/1)\n");
    fprintf(fid, "#  Nobs, Nmorphs, mincomp\n");
    fprintf(fid, "#  yr, seas, type, partition, Nsamp, datavector_by_Nmorphs\n");
    fprintf(fid, "#\n");
    // selectivty priors not yet implemented for SS 3.30.15
    fprintf(fid, "0  #  Do dataread for selectivity priors(0/1)\n");
    fprintf(fid, "# Yr, Seas, Fleet,  Age/Size,  Bin,  selex_prior,  prior_sd\n");
    fprintf(fid, "# feature not yet implemented\n");
    fprintf(fid, "#\n");
    fprintf(fid, "999\n");
    fprintf(fid, "\n");
    fprintf(fid, "ENDDATA\n");
    fprintf(fid, "\n");

	fclose(fid);
    
    free(emptySex);
    return;

}

//******************************************************************************
//
// Name:  WriteSS330Files
// Description: write the files for input to SS v3.30.15.03
//
//
// called by : DoAssessment
// calls : WriteSSDat, WriteSSCtl, WriteSSFor
// created  :
// updated  : Oct 2020 Maciej Golebiewski
//     on Linux run mkdir directly, no script, to allow parallel execution
//
//******************************************************************************

void WriteSS330Files(MSEBoxModel *bm, int sp, int maxyr, char *baseFolder, char *fileName) {
    FILE *fiddat;
    FILE *fidctl;
    FILE *fidss;
    
    //char ssdat[STRLEN];
    //char ssctl[STRLEN];
    //char ssstart[STRLEN];
    //char ssfor[STRLEN];
    
    char outputFileName[STRLEN];
    int versionID = 0;
    
    // SS dat file
    snprintf(outputFileName, sizeof(outputFileName), "%s/%s%s.dat", baseFolder, FunctGroupArray[sp].groupCode, fileName);
    if ((fiddat = fopen(outputFileName, "w")) == NULL) {
        quit("Error opening dat file for  %s\n", FunctGroupArray[sp].groupCode);
    }

    WriteSSDat(bm, fiddat, sp, maxyr);
    fclose(fiddat);

    // SS control file
    snprintf(outputFileName, sizeof(outputFileName), "%s/%s%s.ctl", baseFolder,FunctGroupArray[sp].groupCode, fileName);
    if ((fidctl = fopen(outputFileName, "w")) == NULL) {
        quit("Error opening ctl control file for  %s\n", FunctGroupArray[sp].groupCode);
    }
    WriteSSCtl(bm, fidctl, sp, maxyr);
    fclose(fidctl);

    //  SS forecast file
    snprintf(outputFileName, sizeof(outputFileName), "%s/forecast.ss", baseFolder);

    if ((fidss = fopen(outputFileName, "w")) == NULL) {
        quit("Error opening forecast file for  %s\n", FunctGroupArray[sp].groupCode);
    }
    WriteSSFor(bm, fidss, sp, maxyr);
    fclose(fidss);

    // SS starter file
    Create_Starter_File(bm, baseFolder, sp, versionID);
    
    return;
}

//******************************************************************************
//
// Name:  WriteSSDat
// Description: write the files for input to SS v3.30.15.03
//
//
// called by : WriteSSFiles
// calls :
// created  : Nov 2007 Sally
// updated:   Feb 2011 Sally
// updated  : Nov 2012 Sally for SS v3.24
//
//******************************************************************************
void WriteSSDat(MSEBoxModel *bm, FILE *fid, int sp, int maxyr) {
    int t, f, s, a, l, i, num, part, it, allregion, gender, nfltdisc, iy;
    double *emptySex;
    double cpcv, disccv;

    //allregion = 0;    // uses weighted sum over regions, assume assessment doesn't know about regions
    allregion = (int)(bm->RBCestimation.RBCspeciesParam[sp][NumRegions_id]);

    //  ------  Dimensions
    fprintf(fid, "# SS v3.30.15.03 data file for %s written by Atlantis Version\n", FunctGroupArray[sp].name);
    fprintf(fid, "# Model dimensions\n");
    fprintf(fid, "%d #_StartYr\n", (int)(bm->RBCestimation.RBCspeciesParam[sp][HistYrMin_id]));
    fprintf(fid, "%d #_EndYr\n", maxyr);
    fprintf(fid, "1 #_Nseas\n");
    fprintf(fid, "12 #_months/season (currently fixed at 12)\n");
    fprintf(fid, "2 #_Nsubseasons (even number, minimum is 2)\n");
    fprintf(fid, "1 #_spawn_month\n");
    fprintf(fid, "%d #_Ngenders: 1, 2, -1  (use -1 for 1 sex setup with SSB multiplied by female_frac parameter)\n", (int)(bm->RBCestimation.RBCspeciesParam[sp][Nsexes_id]));
    //* check the definition of the accumulator age hasn't changed between 3.24 and 3.30
    fprintf(fid, "%d #_Nages=accumulator age, first age is always age 0\n", (int)(bm->RBCestimation.RBCspeciesParam[sp][AccumAge_id]));
    //* check SS still only permits 1 area
    fprintf(fid, "1 #_Nareas\n");    // SS will only use 1 area, even if more in the operating model
    fprintf(fid, "%d #_Nfleets (including surveys)\n", (int)(bm->RBCestimation.RBCspeciesParam[sp][NumFisheries_id] + bm->RBCestimation.RBCspeciesParam[sp][NumSurvey_id]));
    fprintf(fid, "#_fleet_type: 1=catch fleet; 2=bycatch only fleet; 3=survey; 4=ignore\n");
    fprintf(fid, "#_sample_timing: -1 for fishing fleet to use season-long catch-at-age for observations, or 1 to use observation month;  (always 1 for surveys)\n");
    fprintf(fid, "#_fleet_area:  area the fleet/survey operates in \n");
    fprintf(fid, "#_units of catch:  1=bio; 2=num (ignored for surveys; their units read later)\n");
    fprintf(fid, "#_catch_mult: 0=no; 1=yes\n");
    fprintf(fid, "#_rows are fleets\n");
    fprintf(fid, "#_fleet_type timing area units need_catch_mult fleetname\n");
    // loop over fleets and surveys
    // We assume that fleets always come before surveys
    for (i = 0; i < bm->RBCestimation.RBCspeciesParam[sp][NumFisheries_id] + bm->RBCestimation.RBCspeciesParam[sp][NumSurvey_id]; i++){
        //add the fleet type
        if(i < bm->RBCestimation.RBCspeciesParam[sp][NumFisheries_id]){
            if (bm->RBCestimation.speciesRPFleetToMetier[i][sp] > -1) {
                fprintf(fid, " 1  -1  1  1  0  FLEET%d in metier %s\n", i, FisheryArray[i].fisheryCode);
            } else {
                fprintf(fid, " 1  -1  1  1  0  FLEET%d\n", i);
            }
        } else {
            fprintf(fid, " 3   1  1  2  0  SURVEY%d # \n", (i - (int)(bm->RBCestimation.RBCspeciesParam[sp][NumFisheries_id])));
        }
    }
    // Bycatch fleet not currently used
    fprintf(fid, "#Bycatch_fleet_input_goes_next\n");
    fprintf(fid, "#a:  fleet index\n");
    fprintf(fid, "#b:  1=include dead bycatch in total dead catch for F0.1 and MSY optimizations and forecast ABC; 2=omit from total catch for these purposes (but still include the mortality)\n");
    fprintf(fid, "#c:  1=Fmult scales with other fleets; 2=bycatch F constant at input value; 3=bycatch F from range of years\n");
    fprintf(fid, "#d:  F or first year of range\n");
    fprintf(fid, "#e:  last year of range\n");
    fprintf(fid, "#f:  not used\n");
    fprintf(fid, "# a   b   c   d   e   f \n");
    // Header for the catch data
    fprintf(fid, "#_Catch data: yr, seas, fleet, catch, catch_se\n");
    fprintf(fid, "#_catch_se:  standard error of log(catch)\n");
    fprintf(fid, "#_NOTE:  catch data is ignored for survey fleets\n");

    // loop over the fleets only as catch data is ignored for survey fleets
    for (f = 0; f < bm->RBCestimation.RBCspeciesParam[sp][NumFisheries_id]; f++){
        fprintf(fid, "-999 1 %d %e 0.01\n", f, bm->RBCestimation.RBCspeciesArray[sp].initialEquilCatch[f]);
        //next is a loop over the years
        // t is the year, f is fleet
        for (t = bm->RBCestimation.RBCspeciesParam[sp][HistYrMin_id]; t < maxyr; t++){
            fprintf(fid, "%d 1 %d", t, f);
            // Apply proportion catch rule for closed area scenarios
            if ((bm->RBCestimation.RBCspeciesArray[sp].mgt_Retro_catch && (bm->RBCestimation.RBCspeciesParam[sp][PropClosed_id] > 0)) && (t < bm->RBCestimation.RBCspeciesParam[sp][HistYrMax_id])){   // allocate historic catches to open area by proportion open
                fprintf(fid, "%e ", (1.0 - bm->RBCestimation.RBCspeciesArray[sp].PropClosed) * bm->RBCestimation.RBCspeciesArray[sp].CatchData[f][allregion][t]);
            } else {
                fprintf(fid, "%e ", bm->RBCestimation.RBCspeciesArray[sp].CatchData[f][allregion][t]);   // region=0 is sum over regions
            }
         fprintf(fid, " 0.01\n");
       }
    }
    
    fprintf(fid, "-9999 0 0 0 0 # terminator for catches\n");
    // end the block of catch data
    // CPUE abundance indices
    fprintf(fid, "#\n");
    fprintf(fid, " #_CPUE_and_surveyabundance_observations\n");
    fprintf(fid, "#_Units:  0=numbers; 1=biomass; 2=F; 30=spawnbio; 31=recdev; 32=spawnbio*recdev; 33=recruitment; 34=depletion(&see Qsetup); 35=parm_dev(&see Qsetup)\n");
    fprintf(fid, "#_Errtype:  -1=normal; 0=lognormal; >0=T\n");
    fprintf(fid, "#_SD_Report: 0=no sdreport; 1=enable sdreport\n");
    fprintf(fid, "#_Fleet Units Errtype SD_Report\n");
    
    // loop over fleets and surveys
    for (i = 0; i < bm->RBCestimation.RBCspeciesParam[sp][NumFisheries_id] + bm->RBCestimation.RBCspeciesParam[sp][NumSurvey_id]; i++){
        //add the fleet type
        if(i < bm->RBCestimation.RBCspeciesParam[sp][NumFisheries_id]){
            if (bm->RBCestimation.speciesRPFleetToMetier[i][sp] > -1)
                fprintf(fid, " %d 1  0  0  # FLEET%d in metier %s\n", i, i, FisheryArray[i].fisheryCode);
            else
                fprintf(fid, " %d 1  0  0  # FLEET%d", i, i);
        } else {
            fprintf(fid, " %d 1  0  0  # SURVEY%d", i, i - (int)(bm->RBCestimation.RBCspeciesParam[sp][NumFisheries_id]));
        }
    }
    // abundance indices
    //* could add fleet names as comment at the end of each row
    fprintf(fid, "#_yr month fleet obs stderr\n");
                        
    //fprintf(bm->logFile, "Time %e %s %d %d\n", bm->dayt, FunctGroupArray[sp].groupCode, bm->RBCestimation.RBCspeciesParam[sp][NumFisheries_id], bm->RBCestimation.RBCspeciesParam[sp][NumSurvey_id]);

    for (f = 0; f < bm->RBCestimation.RBCspeciesParam[sp][NumFisheries_id] + bm->RBCestimation.RBCspeciesParam[sp][NumSurvey_id]; f++) {
        for (t = bm->RBCestimation.RBCspeciesParam[sp][HistYrMin_id]; t < maxyr; t++) {
            if (bm->RBCestimation.RBCspeciesArray[sp].CPUEgen[f][allregion][t] > 0.0) {
                if (!bm->RBCestimation.RBCspeciesArray[sp].CPUEcv[f]) {   // generate cpue with no error, but still want cv in SS
                    cpcv = 0.2;
                } else {
                    cpcv = bm->RBCestimation.RBCspeciesArray[sp].CPUEcv[f];
                }
            
                if (bm->RBCestimation.RBCspeciesArray[sp].CPUEgen[f][allregion][t] < 0.01) { bm->RBCestimation.RBCspeciesArray[sp].CPUEgen[f][allregion][t] = 0.01; // this is because SS fails if abund index<=0
                }
                fprintf(fid, "%d 7 %d %e %e", t, f, bm->RBCestimation.RBCspeciesArray[sp].CPUEgen[f][allregion][t], cpcv);
            }
        }
    }
    
    fprintf(fid, "-9999 1 1 1 1 # terminator for CPUE and survey observations\n");
    fprintf(fid, "#\n");

    // setup for discards
    //* check this works for an assessment with discards
    //* Jemery suggested the flathead and school whiting examples
    Util_Init_1D_Int(bm->RBCestimation.RBCspeciesArray[sp].discfleet, bm->RBCestimation.RBCspeciesParam[sp][NumFisheries_id], 0.0);
    nfltdisc = 0;
    num = 0;
    for (t = bm->RBCestimation.RBCspeciesParam[sp][HistYrMin_id]; t < maxyr; t++) {
        for (f = 0; f < bm->RBCestimation.RBCspeciesParam[sp][NumFisheries_id]; f++) {
            if (bm->RBCestimation.RBCspeciesArray[sp].DiscData[f][allregion][t] > 0) {
                num++;
                bm->RBCestimation.RBCspeciesArray[sp].discfleet[f] = 1;
            }
        }
    }
    for (f = 0; f < bm->RBCestimation.RBCspeciesParam[sp][NumFisheries_id]; f++){
        nfltdisc += bm->RBCestimation.RBCspeciesArray[sp].discfleet[f];
    }
    // discards
    fprintf(fid, "%d #_N_fleets_with_discard\n", nfltdisc);
    fprintf(fid, "#_discard_units (1=same_as_catchunits(bio/num); 2=fraction; 3=numbers)\n");
    fprintf(fid, "#_discard_errtype:  >0 for DF of T-dist(read CV below); 0 for normal with CV; -1 for normal with se; -2 for lognormal; -3 for trunc normal with CV\n");
    fprintf(fid, "# note, only have units and errtype for fleets with discard \n");
     fprintf(fid, "#Fleet Units Err_type\n");
    for (f = 0; f < bm->RBCestimation.RBCspeciesParam[sp][NumFisheries_id]; f++) {
        if (bm->RBCestimation.RBCspeciesArray[sp].discfleet[f]) {
            fprintf(fid, "%d %d      0\n", f, (int)(bm->RBCestimation.RBCspeciesParam[sp][DiscType_id]));
        }
    }
    //** Number of discard observations doesn't appear to be an input for 3.30
    //*   fprintf(fid, "%d  # number of obsns\n", num);
    for (f = 0; f < bm->RBCestimation.RBCspeciesParam[sp][NumFisheries_id]; f++) {
        for (t = bm->RBCestimation.RBCspeciesParam[sp][HistYrMin_id]; t < maxyr; t++) {
            if (bm->RBCestimation.RBCspeciesArray[sp].DiscData[f][allregion][t] > 0) {
                if (bm->RBCestimation.RBCspeciesArray[sp].DiscCV[f] == 0.0) {   // generate discards with no error, but still want cv in SS
                    disccv = 0.1;
                } else {
                    disccv = bm->RBCestimation.RBCspeciesArray[sp].DiscCV[f];
                }
                fprintf(fid, "%d   1 %d %e %e", t, f, bm->RBCestimation.RBCspeciesArray[sp].DiscData[f][allregion][t], disccv);
            }
        }
    }
    
    // Only write the discard terminator if there are discards
    if(nfltdisc > 0){
      fprintf(fid, "-9999 0 0 0.0 0.0 # terminator for discard data\n");
    } else {
      fprintf(fid, "#-9999 0 0 0.0 0.0 # terminator for discard data\n");
    }
    fprintf(fid, "#\n");
    
    //* mean body size - needs additional coding if it is going to be used
    fprintf(fid, "0 #_use meanbodysize_data (0/1)  \n");
    fprintf(fid, "#_COND_0 #_DF_for_meanbodysize_T-distribution_like  \n");
    fprintf(fid, "# note:  use positive partition value for mean body wt, negative partition for mean body length   \n");
    fprintf(fid, "#_yr month fleet part obs stderr  \n");
    fprintf(fid, "#  -9999 0 0 0 0 0 # terminator for mean body size data   \n");
    fprintf(fid, "#\n");
    //* length data - needs additional coding if methods 2 or 3 are to be used
    fprintf(fid, "# set up population length bin structure (note - irrelevant if not using size data and using empirical wtatage  \n");
    fprintf(fid, "1 # length bin method: 1=use databins; 2=generate from binwidth,min,max below; 3=read vector  \n");
    fprintf(fid, "# binwidth for population size comp (no additional input for option 1)  \n");
    fprintf(fid, "# minimum size in the population (lower edge of first bin and size at age 0.00)  \n");
    fprintf(fid, "# # maximum size in the population (lower edge of last bin)  \n");
    fprintf(fid, "1 # use length composition data (0/1)  \n");
    fprintf(fid, "#_mintailcomp: upper and lower distribution for females and males separately are accumulated until exceeding this level.  \n");
    fprintf(fid, "#_addtocomp:  after accumulation of tails; this value added to all bins  \n");
    fprintf(fid, "#_males and females treated as combined gender below this bin number   \n");
    fprintf(fid, "#_compressbins: accumulate upper tail by this number of bins; acts simultaneous with mintailcomp; set=0 for no forced accumulation  \n");
    fprintf(fid, "#_Comp_Error:  0=multinomial, 1=dirichlet  \n");
    fprintf(fid, "#_Comp_Error2:  parm number  for dirichlet  \n");
    fprintf(fid, "#_minsamplesize: minimum sample size; set to 1 to match 3.24, minimum value is 0.001  \n");
    fprintf(fid, "#_mintailcomp addtocomp combM+F CompressBins CompError ParmSelect minsamplesize  \n");
    
    // loop over fleets and surveys
    for (i = 0; i < bm->RBCestimation.RBCspeciesParam[sp][NumFisheries_id] + bm->RBCestimation.RBCspeciesParam[sp][NumSurvey_id]; i++){
        if (i < bm->RBCestimation.RBCspeciesParam[sp][NumFisheries_id]){
            if (bm->RBCestimation.speciesRPFleetToMetier[i][sp] > -1) {
                fprintf(fid, " 0 1e-007 0 0 0 0 1 # FLEET%d in metier %s\n", i, FisheryArray[i].fisheryCode);
            } else {
                fprintf(fid, " 0 1e-007 0 0 0 0 1 # FLEET%d\n", i);
            }
        } else {
            fprintf(fid, " 0 1e-007 0 0 0 0 1 # SURVEY%d\n", i - (int)(bm->RBCestimation.RBCspeciesParam[sp][NumFisheries_id]));
        }
    }
    fprintf(fid, "# sex codes:  0=combined; 1=use female only; 2=use male only; 3=use both as joint sexxlength distribution \n");
    fprintf(fid, "# partition codes:  (0=combined; 1=discard; 2=retained  \n");
    
    // setup number of length bins

    emptySex = Util_Alloc_Init_1D_Double(bm->RBCestimation.RBCspeciesParam[sp][Nlen_id], 0.0);
    // Initialise - TODO: Is emptysex dealt with properly?
    
    fprintf(fid, "%d #_N_LengthBins; then enter lower edge of each length bin\n", (int)(bm->RBCestimation.RBCspeciesParam[sp][Nlen_id]));
    for (l = 0; l < bm->RBCestimation.RBCspeciesParam[sp][Nlen_id]; l++){
       fprintf(fid, "%e ", bm->RBCestimation.RBCspeciesArray[sp].LoLenBin[l]);
    }
    fprintf(fid, "\n");
    //* add this comment #_yr month fleet sex part Nsamp datavector(female-male)
    //* Code chunk below may be redundant after removing # no LF samples below. Check this with a stock that uses length data
    num = 0;
    for (t = bm->RBCestimation.RBCspeciesParam[sp][HistYrMin_id]; t < maxyr; t++) {
        for (f = 0; f < (bm->RBCestimation.RBCspeciesParam[sp][NumFisheries_id] + bm->RBCestimation.RBCspeciesParam[sp][NumSurvey_id]); f++) {
            for (it = 0; it < 3; it++) {
                for (s = 0; s < bm->RBCestimation.RBCspeciesParam[sp][Nsex_samp_id]; s++) {
                    if ((bm->RBCestimation.RBCspeciesArray[sp].LengthFltYr[it][f][t] > 0)) {
                        if (bm->RBCestimation.RBCspeciesArray[sp].LFss[f][s][t][it] > bm->RBCestimation.RBCspeciesParam[sp][LFSSlim_id]) {
                            num++;
                        } else {
                            fprintf(fid, "#  sample size %d year %d fleet %d type %d sex %d\n", (int)(bm->RBCestimation.RBCspeciesParam[sp][LFSSlim_id]), t, f, it, s);
                        }
                    }
                }
            }
        }
    }
    
    // Array of length data
    fprintf(fid, "#_yr month fleet sex part Nsamp datavector(female-male)\n");
    for (f = 0; f < (bm->RBCestimation.RBCspeciesParam[sp][NumFisheries_id] + bm->RBCestimation.RBCspeciesParam[sp][NumSurvey_id]); f++) {
        for (t = bm->RBCestimation.RBCspeciesParam[sp][HistYrMin_id]; t < maxyr; t++) {
            for (it = 0; it < 3; it++) {
                for (s = 0; s < bm->RBCestimation.RBCspeciesParam[sp][Nsex_samp_id]; s++) {
                    if ((bm->RBCestimation.RBCspeciesArray[sp].LengthFltYr[it][f][t] > 0) && (bm->RBCestimation.RBCspeciesArray[sp].LFss[f][s][t][it] > bm->RBCestimation.RBCspeciesParam[sp][LFSSlim_id])) {
                        if (!it) {
                            part = 2;   // retained
                        } else if (it==1) {
                            part = 0;   // whole
                        } else {
                            part = 1;   // discarded
                        }
                        if (bm->RBCestimation.RBCspeciesParam[sp][Nsex_samp_id] == 1) {
                            gender = 0;   //combined
                        } else {
                            gender = s;
                        }
                        
                        fprintf(fid, "%d 7 %d %d %d %d", t, f, gender, part,   bm->RBCestimation.RBCspeciesArray[sp].LFss[f][s][t][it]);
                    
                        // write the vector of lengths dependent on value specified for sex
                        if (!s) {    // females or combined - if 2 gender model write female LFs then male (ignored)
                            for (l = 0; l < bm->RBCestimation.RBCspeciesParam[sp][Nlen_id]; l++) {
                                fprintf(fid, " %d", bm->RBCestimation.RBCspeciesArray[sp].LenComp[f][s][t][it][l]);
                            }
                            if (bm->RBCestimation.RBCspeciesParam[sp][Nsexes_id] > 1) {
                                for (l = 0; l < bm->RBCestimation.RBCspeciesParam[sp][Nlen_id]; l++) {
                                    fprintf(fid, " %e", emptySex[l]);
                                }
                            }
                        } else {    // males - write female lfs (ignored) then male
                            for (l = 0; l < bm->RBCestimation.RBCspeciesParam[sp][Nlen_id]; l++) {
                                fprintf(fid, " %e", emptySex[l]);
                            }
                            for (l = 0; l < bm->RBCestimation.RBCspeciesParam[sp][Nlen_id]; l++) {
                                fprintf(fid, " %d", bm->RBCestimation.RBCspeciesArray[sp].LenComp[f][s][t][it][l]);
                            }
                        }
                        fprintf(fid, "\n");
                    }
                }
            }
        }
    }
    // terminator for the length data
    fprintf(fid, "-9999 ");
    // loop to create 2*n_len_bins +5 zeroes
    for (i = 0; i < 2 * bm->RBCestimation.RBCspeciesParam[sp][Nlen_id] + 5; i++){
        fprintf(fid, "0 ");
    }
    fprintf(fid, "# terminator for Length composition\n");
    // Start of the age composition data
    fprintf(fid, "# Age composition\n");
                    
    free(emptySex);
    emptySex = Util_Alloc_Init_1D_Double(bm->RBCestimation.RBCspeciesParam[sp][MaxAge_id], 0.0);
    // Initialise - TODO: Is emptysex dealt with properly?

    fprintf(fid, "%d   #_N_age_bins\n", (int)(bm->RBCestimation.RBCspeciesParam[sp][MaxAge_id]+1));
    // Note: 3.30.15 requires ages to start from 1 - but note from RL suggest this no longer true as of July 2021
    for (a = 0; a < bm->RBCestimation.RBCspeciesParam[sp][MaxAge_id]; a++){
        fprintf(fid, "%d ", a);
    }
    fprintf(fid, "\n");
    fprintf(fid, "1   #_N_ageerror_definitions\n");
    for (a = 0; a < bm->RBCestimation.RBCspeciesParam[sp][AccumAge_id]; a++) {     // bm->RBCestimation.RBCspeciesParam[sp][MaxAge_id] +6 is accumulator age + 1
        fprintf(fid, "%f ", a+0.5);
    }
    fprintf(fid, "\n");
    for (a = 0; a < bm->RBCestimation.RBCspeciesParam[sp][AccumAge_id]; a++) {
        fprintf(fid, "%f ", bm->RBCestimation.RBCspeciesArray[sp].Ageing_error[a]);
    }
    fprintf(fid, "\n");
    num = 0;
    
    // _mintailcomp ...
    fprintf(fid, "#_mintailcomp: upper and lower distribution for females and males separately are accumulated until exceeding this level.  \n");
    fprintf(fid, "#_addtocomp:  after accumulation of tails; this value added to all bins  \n");
    fprintf(fid, "#_males and females treated as combined gender below this bin number   \n");
    fprintf(fid, "#_compressbins: accumulate upper tail by this number of bins; acts simultaneous with mintailcomp; set=0 for no forced accumulation  \n");
    fprintf(fid, "#_Comp_Error:  0=multinomial, 1=dirichlet  \n");
    fprintf(fid, "#_Comp_Error2:  parm number  for dirichlet  \n");
    fprintf(fid, "#_minsamplesize: minimum sample size; set to 1 to match 3.24, minimum value is 0.001  \n");
    fprintf(fid, "#_mintailcomp addtocomp combM+F CompressBins CompError ParmSelect minsamplesize  \n");
    // loop over fleets and surveys
    for (i = 0; i < bm->RBCestimation.RBCspeciesParam[sp][NumFisheries_id] + bm->RBCestimation.RBCspeciesParam[sp][NumSurvey_id]; i++){
        if(i < bm->RBCestimation.RBCspeciesParam[sp][NumFisheries_id]){
            if(bm->RBCestimation.speciesRPFleetToMetier[i][sp] > -1) {
                fprintf(fid, "0 1e-007 1 0 0 0 1  # FLEET%d in metier %s\n", i, FisheryArray[i].fisheryCode);
            } else {
                fprintf(fid, " 0 1e-007 1 0 0 0 1  # FLEET %d\n", i);
            }
        } else {
            fprintf(fid, " 0 1e-007 1 0 0 0 1  # SURVEY%d\n", i - (int)(bm->RBCestimation.RBCspeciesParam[sp][NumFisheries_id]));
        }
    }

    for (t = bm->RBCestimation.RBCspeciesParam[sp][HistYrMin_id]; t < maxyr; t++) {
        for (f = 0; f < bm->RBCestimation.RBCspeciesParam[sp][NumFisheries_id]; f++) {
            for (it = 0; it < 3; it++) {
                for (s = 0; s < bm->RBCestimation.RBCspeciesParam[sp][Nsex_samp_id]; s++) {
                    if (bm->RBCestimation.RBCspeciesArray[sp].AgeFltYr[it][f][t] > 0) {
                        if (bm->RBCestimation.RBCspeciesArray[sp].AFss[f][s][t][it] > bm->RBCestimation.RBCspeciesParam[sp][AFSSlim_id]) {
                            num++;
                        } else {
                            fprintf(fid, "#  sample size %d year %d fleet %d type %d sex %d\n", (int)(bm->RBCestimation.RBCspeciesParam[sp][AFSSlim_id]), t, f, it, s);
                        }
                    }
                }
            }
        }
    }
    //fprintf(fid, "%d #  no age compositions   female then male\n", num);
    fprintf(fid, "2 #_Lbin_method_for_Age_Data: 1=poplenbins; 2=datalenbins; 3=lengths\n");
    //fprintf(fid, "1    #    combine    males    into    females    at    or    below    this    bin    number\n");
    //fprintf(fid, "# Yr sea flt sex typ aer lb1 lb2   ss";

    fprintf(fid, "# sex codes:  0=combined; 1=use female only; 2=use male only; 3=use both as joint sex length distribution\n");
    fprintf(fid, "# partition codes:  (0=combined; 1=discard; 2=retained)\n");
    fprintf(fid, "#_yr month fleet sex part ageerr Lbin_lo Lbin_hi Nsamp datavector(female-male)\n");
    //   old headers were fprintf(fid, "#Yr sea flt sex typ aer lb1 lb2   ss     0     1   2 ...\n");
    // loop over the age data
    // for (s=1;s<=bm->RBCestimation.RBCspeciesParam[sp][Nsexes_id];s++)
    //     for (a=0;a<=bm->RBCestimation.RBCspeciesParam[sp][MaxAge_id];a++)
    //            fprintf(fid, "std::setw(6)<<a;
    //    fprintf(fid, "\n");
    //* this might need to be over Nfleets + Nsurveys, do surveys collect age data?
    for (f = 0; f < bm->RBCestimation.RBCspeciesParam[sp][NumFisheries_id]; f++) {
        for (t = bm->RBCestimation.RBCspeciesParam[sp][HistYrMin_id]; t < maxyr; t++) {
            for (it = 0; it < 3; it++) {
                for (s = 0; s < bm->RBCestimation.RBCspeciesParam[sp][Nsex_samp_id]; s++) {
                    if (bm->RBCestimation.RBCspeciesArray[sp].AgeFltYr[it][f][t] > 0) {
                        if (bm->RBCestimation.RBCspeciesArray[sp].AFss[f][s][t][it] > bm->RBCestimation.RBCspeciesParam[sp][AFSSlim_id]) { //   make ss limit >0 ?
                            if (it==1)
                                part = 2;   // retained
                            else if (it==2)
                                part = 0;   // whole
                            else if (it==3)
                                part = 1;   // discarded
                            if (bm->RBCestimation.RBCspeciesParam[sp][Nsex_samp_id] == 1)
                                gender = 0;   //combined
                            else
                                gender = s;

                            // this has the lo and hi length bins set to -1
                            // break this over multiple lines
                            fprintf(fid, "%d   7 %d %d %d   1  -1  -1 %d", t, f, gender, part, bm->RBCestimation.RBCspeciesArray[sp].AFss[f][s][t][it]); // final bit is the sample size

                            if (!s) {    // combined or females - if 2 gender model write female LFs then male (ignored)
                                for (a = 0; a< bm->RBCestimation.RBCspeciesParam[sp][MaxAge_id]; a++) {
                                   fprintf(fid, "%d ", bm->RBCestimation.RBCspeciesArray[sp].AgeComp[f][s][t][it][a]);
                                }
                                if (bm->RBCestimation.RBCspeciesParam[sp][Nsexes_id] > 1) {
                                    for (a = 0; a < bm->RBCestimation.RBCspeciesParam[sp][MaxAge_id]; a++) {
                                        fprintf(fid, " %e", emptySex[a]);
                                    }
                                }
                            } else { // males - write female lfs (ignored) then male
                                for (a = 0; a < bm->RBCestimation.RBCspeciesParam[sp][MaxAge_id]; a++) {
                                    fprintf(fid, " %e", emptySex[a]);
                                }
                                for (a = 0; a < bm->RBCestimation.RBCspeciesParam[sp][MaxAge_id]; a++) {
                                    fprintf(fid, "%d ", bm->RBCestimation.RBCspeciesArray[sp].AgeComp[f][s][t][it][a]);
                                }
                            }
                            //* I don't think the line below is needed - I need an end line statement though
                            fprintf(fid, "      #### AgeComp(f,s,t,it,a)\n");
                        }
                    }
                }
            }
        }
    }

    // terminator for the age data
    fprintf(fid, "-9999 ");
    // loop to create 2*n_age_bins +9 zeroes, where n_age_bins=MaxAge+1 **update the equation**
    for (i = 0; i < 2*bm->RBCestimation.RBCspeciesParam[sp][MaxAge_id]+10; i++) {
        fprintf(fid, "0 ");
    }
    fprintf(fid, "# terminator for Age composition\n");
    fprintf(fid, "#\n");
    // mean size at age obs not currently implemented
    fprintf(fid, "0 #_Use_MeanSize-at-Age_obs (0/1)\n");
    fprintf(fid, "# sex codes:  0=combined; 1=use female only; 2=use male only; 3=use both as joint sexxlength distribution\n");
    fprintf(fid, "# partition codes:  (0=combined; 1=discard; 2=retained\n");
    fprintf(fid, "# ageerr codes:  positive means mean length-at-age; negative means mean bodywt_at_age\n");
    fprintf(fid, "#_yr month fleet sex part ageerr ignore datavector(female-male)\n");
    fprintf(fid, "#                                          samplesize(female-male)\n");
    fprintf(fid, "#\n");
    // Environmental variables (for regime shift)
    if (bm->RBCestimation.RBCspeciesParam[sp][Regime_year_assess_id] > 0) {  //  for  regime shift
        fprintf(fid, "1    #  number of environmental variables: year, variable, value\n");
        //nobs = bm->RBCestimation.RBCspeciesParam[sp][Regime_year_assess_id] - bm->RBCestimation.RBCspeciesParam[sp][HistYrMin_id] + 1;
        //fprintf(fid, "%d    #  enviromental observations\n", nobs);
        for (iy = bm->RBCestimation.RBCspeciesParam[sp][HistYrMin_id]-1; iy < bm->RBCestimation.RBCspeciesParam[sp][Regime_year_assess_id] - 1; iy++) {
               fprintf(fid, "%d  1  1\n", iy);
        }
        fprintf(fid, "-9999  0  0\n");
    } else {
        fprintf(fid, "0 #_N_environ_variables\n");
        fprintf(fid, "#Yr Variable Value\n");
    }
    fprintf(fid, "#\n");
    // size frequency not currently implemented
    fprintf(fid, "0 #    N sizefreq methods to read\n");
    fprintf(fid, "#\n");
    // tagging data not currently implemented
    fprintf(fid, "0 #    do tags (0/1)\n");
    fprintf(fid, "#\n");
    // morphcomp not currently implemented
    fprintf(fid, "0 #    morphcomp data (0/1)\n");
    fprintf(fid, "#  Nobs, Nmorphs, mincomp\n");
    fprintf(fid, "#  yr, seas, type, partition, Nsamp, datavector_by_Nmorphs\n");
    fprintf(fid, "#\n");
    // selectivty priors not yet implemented for SS 3.30.15
    fprintf(fid, "0  #  Do dataread for selectivity priors(0/1)\n");
    fprintf(fid, "# Yr, Seas, Fleet,  Age/Size,  Bin,  selex_prior,  prior_sd\n");
    fprintf(fid, "# feature not yet implemented\n");
    fprintf(fid, "#\n");
    fprintf(fid, "999\n");
    fprintf(fid, "\n");
    fprintf(fid, "ENDDATA\n");
    fprintf(fid, "\n");
                       
    free(emptySex);
    return;

}

//******************************************************************************
//
// Name:  WriteSSCtl
// Description: write the files for input to SS v3.30.15.03
//
//
// called by : _WriteSSFiles
// calls :
// created  : Nov 2007 Sally
// updated  : Feb 2011 to write SSv3.1
// updated  : Nov 2012 Sally for SS v3.24
// updated  : FEb 2020 PB for SS v3.30
//
//******************************************************************************
void WriteSSCtl(MSEBoxModel *bm, FILE *fid, int sp, int maxyr) {
    int f, disc, t, i, j, phase, nsp, block_fxn, ageindex;
    int yindex = (int)(bm->RBCestimation.RBCspeciesParam[sp][HistYrMin_id]);
    int thisIndexL2 = (int)(bm->RBCestimation.RBCspeciesParam[sp][Growthage_L2_id]);
    int thisIndexL1 = (int)(bm->RBCestimation.RBCspeciesParam[sp][Growthage_L1_id]);
    double lminf, lminm, lmaxf, lmaxm, cvest, vbk;
    double sv, mval, lo, hi;
    int *dflt;
    int allregion = 0;    // uses weighted sum over regions, assume assessment doesn't know about regions
    
    // initial header block
    fprintf(fid, "# SS v3.30.15.03 control file for %s written by Atlantis Version\n", FunctGroupArray[sp].name);
    // start of settings / parameters
    fprintf(fid, "0  # 0 means do not read wtatage.ss; 1 means read and use wtatage.ss and also read and use growth parameters\n");
    fprintf(fid, "1  #_N_Growth_Patterns  (Growth Patterns, Morphs, Bio Patterns, GP are terms used interchangeably in SS)\n");
    fprintf(fid, "1 #_N_platoons_Within_GrowthPattern\n");
    fprintf(fid, "#_Cond 1 #_Platoon_within/between_stdev_ratio (no read if N_platoons=1)\n");
    fprintf(fid, "#_Cond  1 #vector_platoon_dist_(-1_in_first_val_gives_normal_approx)\n");
    fprintf(fid, "# \n");
    fprintf(fid, "2 # recr_dist_method for parameters:  2=main effects for GP, Settle timing, Area; 3=each Settle entity; 4=none when N_GP*Nsettle*pop==1\n");
    fprintf(fid, "1 # not yet implemented; Future usage: Spawner-Recruitment: 1=global; 2=by area\n");
    fprintf(fid, "1 #  number of recruitment settlement assignments \n");
    fprintf(fid, "0 # unused option\n");
    fprintf(fid, "#GPattern month  area  age (for each settlement assignment)\n");
    fprintf(fid, "1 1 1 0\n");
    fprintf(fid, "#\n");
    fprintf(fid, "#_Cond 0 # N_movement_definitions goes here if Nareas > 1\n");
    fprintf(fid, "#_Cond 1.0 # first age that moves (real age at begin of season, not integer) also cond on do_migration>0\n");
    fprintf(fid, "#_Cond 1 1 1 2 4 10 # example move definition for seas=1, morph=1, source=1 dest=2, age1=4, age2=10\n");
    fprintf(fid, "#\n");
    
    if (bm->RBCestimation.RBCspeciesParam[sp][NblockPattern_id] > 0){
        fprintf(fid, "%d #_Nblock_Patterns\n", (int)(bm->RBCestimation.RBCspeciesParam[sp][NblockPattern_id]));
        for (i = 0; i < bm->RBCestimation.RBCspeciesParam[sp][NblockPattern_id]; i++) {
            fprintf(fid, "%d  ", bm->RBCestimation.RBCspeciesArray[sp].NblockperPatt[i]);
        }
        fprintf(fid, "        # Blocks per pattern\n");
        fprintf(fid, "# begin and end years of Blocks\n");
        for (i=0; i < bm->RBCestimation.RBCspeciesParam[sp][NblockPattern_id]; i++) {
            for (j=0; j< bm->RBCestimation.RBCspeciesArray[sp].NblockperPatt[i]-1; j++) {
                fprintf(fid, "%f %f\n", bm->RBCestimation.RBCspeciesArray[sp].Blocks[i][j*2+1], bm->RBCestimation.RBCspeciesArray[sp].Blocks[i][j*2+2]);
            }
        }
    } else {
        fprintf(fid, "1 #_Nblock_Patterns\n");
        fprintf(fid, "1 #_blocks_per_pattern\n");
        fprintf(fid, "%d  %d\n", (int)(bm->RBCestimation.RBCspeciesParam[sp][HistYrMin_id]), (int)(bm->RBCestimation.RBCspeciesParam[sp][HistYrMin_id]));
    }
    fprintf(fid, "#\n");
    fprintf(fid, "# controls for all timevary parameters \n");
    fprintf(fid, "1 #_env/block/dev_adjust_method for all time-vary parms (1=warn relative to base parm bounds; 3=no bound check)\n");
    fprintf(fid, "#\n");
    fprintf(fid, "#  autogen: 1st element for biology, 2nd for SR, 3rd for Q, 4th reserved, 5th for selex\n");
    fprintf(fid, "0 0 0 0 0 # autogen: \n");
    fprintf(fid, "# where: 0 = autogen all time-varying parms; 1 = read each time-varying parm line; 2 = read then autogen if parm min==-12345\n");
    fprintf(fid, "# \n");
    fprintf(fid, "#_Available timevary codes\n");
    fprintf(fid, "#_Block types: 0: P_block=P_base*exp(TVP); 1: P_block=P_base+TVP; 2: P_block=TVP; 3: P_block=P_block(-1) + TVP\n");
    fprintf(fid, "#_Block_trends: -1: trend bounded by base parm min-max and parms in transformed units (beware); -2: endtrend and infl_year direct values; -3: end and infl as fraction of base range\n");
    fprintf(fid, "#_EnvLinks:  1: P(y)=P_base*exp(TVP*env(y));  2: P(y)=P_base+TVP*env(y);  3: null;  4: P(y)=2.0/(1.0+exp(-TVP1*env(y) - TVP2))\n");
    fprintf(fid, "#_DevLinks:  1: P(y)*=exp(dev(y)*dev_se;  2: P(y)+=dev(y)*dev_se;  3: random walk;  4: zero-reverting random walk with rho;  21-24 keep last dev for rest of years\n");
    fprintf(fid, "#\n");
    fprintf(fid, "#_Prior_codes:  0=none; 6=normal; 1=symmetric beta; 2=CASAL's beta; 3=lognormal; 4=lognormal with biascorr; 5=gamma\n");
    fprintf(fid, "#\n");
    fprintf(fid, "# setup for M, growth, maturity, fecundity, recruitment distibution, movement \n");
    fprintf(fid, "# \n");
    fprintf(fid, "0 # natM_type:_0=1Parm; 1=N_breakpoints;_2=Lorenzen;_3=agespecific;_4=agespec_withseasinterpolate \n");
    fprintf(fid, "  # no_additional_input_for_selected_M_option; read 1P per morph \n");
    fprintf(fid, "1 # GrowthModel: 1=vonBert with L1&L2; 2=Richards with L1&L2; 3=age_specific_K_incr; 4=age_specific_K_decr; 5=age_specific_K_each; 6=NA; 7=NA; 8=growth cessation\n");
    fprintf(fid, "%d #_Age(post-settlement)_for_L1;linear growth below this\n", (int)(bm->RBCestimation.RBCspeciesParam[sp][Growthage_L1_id]));
    fprintf(fid, "%d #_Growth_Age_for_L2 (999 to use as Linf)\n", (int)(bm->RBCestimation.RBCspeciesParam[sp][Growthage_L2_id]));
    fprintf(fid, "-999 #_exponential decay for growth above maxage (value should approx initial Z; -999 replicates 3.24; -998 to not allow growth above maxage)\n");
    fprintf(fid, "0  # placeholder for future growth feature \n");
    fprintf(fid, "# \n");
    fprintf(fid, "0 #_SD_add_to_LAA (set to 0.1 for SS2 V1.x compatibility) \n");
    fprintf(fid, "0 #_CV_Growth_Pattern:  0 CV=f(LAA); 1 CV=F(A); 2 SD=F(LAA); 3 SD=F(A); 4 logSD=F(A) \n");
    fprintf(fid, "# \n");
    fprintf(fid, "1 #_maturity_option:  1=length logistic; 2=age logistic; 3=read age-maturity matrix by growth_pattern; 4=read age-fecundity; 5=disabled; 6=read length-maturity \n");
    fprintf(fid, "1 #_First_Mature_Age \n");
    fprintf(fid, "1 #_fecundity option:(1)eggs=Wt*(a+b*Wt);(2)eggs=a*L^b;(3)eggs=a*Wt^b; (4)eggs=a+b*L; (5)eggs=a+b*W \n");
    fprintf(fid, "0 #_hermaphroditism option:  0=none; 1=female-to-male age-specific fxn; -1=male-to-female age-specific fxn \n");
    fprintf(fid, "3 #_parameter_offset_approach (1=none, 2= M, G, CV_G as offset from female-GP1, 3=like SS2 V1.x) \n");
    fprintf(fid, "# \n");
    //* growth parameters: some of the fixed values may be different in 3.30
    fprintf(fid, "#_growth_parms \n");
    fprintf(fid, "#_ LO HI INIT PRIOR PR_SD PR_type PHASE env_var&link dev_link dev_minyr dev_maxyr dev_PH Block Block_Fxn\n");
    // female parameters (female's appear to be sex = 1 in arrays)
    fprintf(fid, "# Sex: 1  BioPattern: 1  NatMort\n");
    mval = bm->RBCestimation.RBCspeciesArray[sp].Mortality[0];
    fprintf(fid, "%f %f %f %f       0.8  0   -3       0       0         0         0        0     0     0     # NatM_p_1_Fem_GP_1\n", mval/2.0, mval*2.0, mval, mval);
    fprintf(fid, "# Sex: 1  BioPattern: 1  Growth\n");
    ageindex = (int)(bm->RBCestimation.RBCspeciesParam[sp][Growthage_L1_id]);
    lminf = bm->RBCestimation.RBCspeciesArray[sp].MeanLenAgeS[0][0][ageindex][yindex];
     fprintf(fid, "%f %f %f %f       10   0   -3       0       0         0         0        0     0     0     # L_at_Amin_Fem_GP_1\n", lminf/2.0, lminf*2.0, lminf, lminf);

    if (bm->RBCestimation.RBCspeciesParam[sp][Growthage_L2_id] < 999) {
        lmaxf = bm->RBCestimation.RBCspeciesArray[sp].MeanLenAgeS[0][0][thisIndexL2][yindex];
    } else {
        lmaxf = bm->RBCestimation.RBCspeciesArray[sp].VBLinf[0][0];     // use Linf
    }
    //lmax = bm->RBCestimation.RBCspeciesArray[sp].MeanLenAgeS[0][0][bm->RBCestimation.RBCspeciesParam[sp][MaxAge_id]][bm->RBCestimation.RBCspeciesParam[sp][HistYrMin_id]];
     fprintf(fid, "%f %f %f %f       10   0   -3       0       0         0         0        0     0     0     # L_at_Amax_Fem_GP_1\n", lmaxf/2.0, lmaxf*2.0, lmaxf, lmaxf);

    vbk = bm->RBCestimation.RBCspeciesArray[sp].VBk[0][0];
     fprintf(fid, "%f %f %f %f       0.8  0   -3       0       0         0         0        0     0     0     # VonBert_K_Fem_GP_1\n", vbk/3.0, vbk*3.0, vbk, vbk);

    cvest = bm->RBCestimation.RBCspeciesArray[sp].CvLA0[0][0];

     fprintf(fid, "%f %f %f %f       0.8  0   -2       0       0         0         0        0     0     0     # CV_young_Fem_GP_1\n", cvest/3.0, cvest*3.0, cvest, cvest);

    if (bm->RBCestimation.RBCspeciesParam[sp][MG_offset_id] > 2){
        if (fabs(bm->RBCestimation.RBCspeciesArray[sp].CvLAmax[0][0])/fabs(bm->RBCestimation.RBCspeciesArray[sp].CvLA0[0][0]) <= 0 ) {
            quit("Error in setting CvLAmax, CvLA0\n");
        }
        cvest = log(fabs(bm->RBCestimation.RBCspeciesArray[sp].CvLAmax[0][0])/fabs(bm->RBCestimation.RBCspeciesArray[sp].CvLA0[0][0]));
        lo = -1.0;
        hi = 1.0;
    } else {
        cvest = fabs(bm->RBCestimation.RBCspeciesArray[sp].CvLAmax[0][0]);
        lo = cvest/3.0;
        hi = cvest*3.0;
    }

    fprintf(fid, "%f %f %f %f       0.8   0  -3       0       0         0         0        0     0     0     # CV_old_Fem_GP_1\n", lo, hi, cvest, cvest);
    fprintf(fid, "# Sex: 1  BioPattern: 1  WtLen\n");
    fprintf(fid, "    -3     3 %f %f     0.8    0  -3       0       0         0         0        0     0     0     # Wtlen_1_Fem\n", bm->RBCestimation.RBCspeciesArray[sp].Wtlen_a[0][FEMALE], bm->RBCestimation.RBCspeciesArray[sp].Wtlen_a[0][FEMALE]);
    fprintf(fid, "     0     6 %f %f     0.8    0  -3       0       0         0         0        0     0     0     # Wtlen_2_Fem\n",bm->RBCestimation.RBCspeciesArray[sp].Wtlen_b[0][FEMALE], bm->RBCestimation.RBCspeciesArray[sp].Wtlen_b[0][FEMALE]);
    fprintf(fid, "# Sex: 1  BioPattern: 1  Maturity&Fecundity\n");
    fprintf(fid, "%f %f %f %f     10    0   -3       0       0         0         0        0     0     0     # Mat50_Fem\n", bm->RBCestimation.RBCspeciesParam[sp][Maturity_Inflect_id]/2.0, bm->RBCestimation.RBCspeciesParam[sp][Maturity_Inflect_id]*2.0, bm->RBCestimation.RBCspeciesParam[sp][Maturity_Inflect_id], bm->RBCestimation.RBCspeciesParam[sp][Maturity_Inflect_id]);
    fprintf(fid, "    -3     3 %f %f %f     0.8    0   -3       0       0         0         0        0     0     0     # Mat_slope_Fem\n", bm->RBCestimation.RBCspeciesParam[sp][Maturity_Slope_id], bm->RBCestimation.RBCspeciesParam[sp][Maturity_Slope_id], bm->RBCestimation.RBCspeciesParam[sp][Maturity_Slope_id]);
    //* the next two parameters were labeled egg/gm in the 3.24 version, howeverit doesn't look like they're doing anything important
     fprintf(fid, "    -3     3        1        1       0.8   0   -3       0       0         0         0        0     0     0     #   Eggs/kg_inter_Fem\n");
     fprintf(fid, "    -3     3        0        0       0.8   0   -3       0       0         0         0        0     0     0     #   Eggs/kg_slope_wt_Fem\n");
    // male parameters (when there are two sexes)
    if (bm->RBCestimation.RBCspeciesParam[sp][Nsexes_id] > 1) {
        fprintf(fid, "# Sex: 2  BioPattern: 1  NatMort\n");
        if (bm->RBCestimation.RBCspeciesParam[sp][MG_offset_id] > 2) {
            mval = log(fabs(bm->RBCestimation.RBCspeciesArray[sp].Mortality[1])/fabs(bm->RBCestimation.RBCspeciesArray[sp].Mortality[0]));
            lo = -1.0;
            hi = 1.0;
        } else {
            mval = fabs(bm->RBCestimation.RBCspeciesArray[sp].Mortality[1]);
            lo = mval/2.0;
            hi = mval*2.0;
        }

         fprintf(fid, "%f %f %f %f        0.8   0   -3       0       0         0         0        0     0     0     # NatM_p_1_Mal_GP_1\n", lo, hi, mval, mval);
    
        if (bm->RBCestimation.RBCspeciesParam[sp][MG_offset_id] > 2) {
            lminm = bm->RBCestimation.RBCspeciesArray[sp].MeanLenAgeS[0][1][thisIndexL1][yindex];
            lminm = log(lminm/lminf);
            //lmin = log(bm->RBCestimation.RBCspeciesParam[sp].MeanLenAge[0][1][bm->RBCestimation.RBCspeciesParam[sp][Growthage_L1_id][bm->RBCestimation.RBCspeciesParam[sp][HistYrMin_id]])/bm->RBCestimation.RBCspeciesParam[sp].MeanLenAge[0][0][bm->RBCestimation.RBCspeciesParam[sp][Growthage_L1_id][,]bm->RBCestimation.RBCspeciesParam[sp][HistYrMin_id]));
            lo = -1.0;
            hi = 1.0;
        } else {
            lminm = bm->RBCestimation.RBCspeciesArray[sp].MeanLenAgeS[0][1][thisIndexL1][yindex];
            lo = lminm/2.0;
            hi = lminm*2.0;
        }
                                                                            
        fprintf(fid, "# Sex: 2  BioPattern: 1  Growth\n");
        fprintf(fid, "%f %f %f %f     10   0   -3       0       0         0         0        0     0     0     # L_at_Amin_Mal_GP_1\n", lo, hi, lminm, lminm);

        if (bm->RBCestimation.RBCspeciesParam[sp][Growthage_L2_id] < 999) {
            lmaxm = bm->RBCestimation.RBCspeciesArray[sp].MeanLenAgeS[0][1][thisIndexL2][yindex];
        } else {
            lmaxm = bm->RBCestimation.RBCspeciesArray[sp].VBLinf[0][1];
        }
        if (bm->RBCestimation.RBCspeciesParam[sp][MG_offset_id] > 2) {
            lmaxm = log(lmaxm/lmaxf);
            //lmaxm = log(bm->RBCestimation.RBCspeciesParam[sp].MeanLenAge[1][2][bm->RBCestimation.RBCspeciesParam[sp][MaxAge_id]][bm->RBCestimation.RBCspeciesParam[sp][HistYrMin_id]])/bm->RBCestimation.RBCspeciesParam[sp].MeanLenAge[0][0][bm->RBCestimation.RBCspeciesParam[sp][MaxAge_id]][bm->RBCestimation.RBCspeciesParam[sp][HistYrMin_id]]);
            lo = -1.0;
            hi = 1.0;
        } else {
            //lmax = bm->RBCestimation.RBCspeciesParam[sp].MeanLenAge[1][2][bm->RBCestimation.RBCspeciesParam[sp][MaxAge_id]][bm->RBCestimation.RBCspeciesParam[sp][HistYrMin_id]];
            lo = lmaxm/2.0;
            hi = lmaxm*2.0;
        }
        fprintf(fid, "%f %f %f %f      10   0   -3       0       0         0         0        0     0     0     # L_at_Amax_Mal_GP_1\n", lo, hi, lmaxm, lmaxm);
        if (bm->RBCestimation.RBCspeciesParam[sp][MG_offset_id] > 2) {
            vbk = log(bm->RBCestimation.RBCspeciesArray[sp].VBk[0][1])/bm->RBCestimation.RBCspeciesArray[sp].VBk[0][0];
            lo = -1.0;
            hi = 1.0;
        } else {
            vbk = bm->RBCestimation.RBCspeciesArray[sp].VBk[0][1];
            lo = vbk/2.0;
            hi = vbk*2.0;
        }
        fprintf(fid, "%f %f %f %f     0.8   0   -3       0       0         0         0        0     0     0     # VonBert_K_Mal_GP_1\n", lo, hi, vbk, vbk);

        if (bm->RBCestimation.RBCspeciesParam[sp][MG_offset_id] > 2) {
            cvest = log(bm->RBCestimation.RBCspeciesArray[sp].CvLA0[0][1]/bm->RBCestimation.RBCspeciesArray[sp].CvLA0[0][0]);  // offset from F
            lo = -1.0;
            hi = 1.0;
        } else {
            cvest = bm->RBCestimation.RBCspeciesArray[sp].CvLA0[0][1];
            lo = cvest/3.0;
            hi = cvest*3.0;
        }
         fprintf(fid, "%f %f %f %f      0.8   0  -3       0       0         0         0        0     0     0     # CV_young_Mal_GP_1\n", lo, hi, cvest, cvest);

        if (bm->RBCestimation.RBCspeciesParam[sp][MG_offset_id] > 2) {     // prior to 21.5.13 was wrongly setting this to cv old F
            cvest = log(bm->RBCestimation.RBCspeciesArray[sp].CvLAmax[0][1]/bm->RBCestimation.RBCspeciesArray[sp].CvLA0[0][1]);   // offset from young M
            lo = -1.0;
            hi = 1.0;
        } else {
            cvest = bm->RBCestimation.RBCspeciesArray[sp].CvLAmax[0][1];
            lo = cvest/3.0;
            hi = cvest*3.0;
        }
         fprintf(fid, "%f %f %f %f       0.8   0  -3        0       0         0         0        0     0     0     # CV_old_Mal_GP_1\n", lo, hi, cvest, cvest);
    }
    // male weight length
    if (bm->RBCestimation.RBCspeciesParam[sp][Nsexes_id]> 1) {
         fprintf(fid, "    -3     3 %f %f     0.8   0    -3       0       0         0         0        0     0     0     # Wtlen_1_Mal\n", bm->RBCestimation.RBCspeciesArray[sp].Wtlen_a[0][MALE], bm->RBCestimation.RBCspeciesArray[sp].Wtlen_a[0][MALE]);
         fprintf(fid, "     0     6 %f %f     0.8   0    -3       0       0         0         0        0     0     0     # Wtlen_2_Mal\n", bm->RBCestimation.RBCspeciesArray[sp].Wtlen_b[0][MALE], bm->RBCestimation.RBCspeciesArray[sp].Wtlen_b[0][MALE]);
    }
    fprintf(fid, "# Hermaphroditism\n");
    fprintf(fid, "#  Recruitment Distribution  \n");
    fprintf(fid, "     0     0        0        0      0    0   -3       0       0         0         0        0       0     0     # RecrDist_GP_1\n");
    fprintf(fid, "     0     0        0        0      0    0   -3       0       0         0         0        0       0     0     # RecrDist_Area_1\n");
    fprintf(fid, "     0     0        0        0      0    0   -3       0       0         0         0        0       0     0     # RecrDist_timing_1\n");
    fprintf(fid, "#  Cohort growth dev base\n");
    fprintf(fid, "     1     1        1        1      0    1   -3       0       0         0         0        0       0     0     # CohortGrowDev\n");
    //* check this in the manual
    fprintf(fid, "#  Movement\n");
    fprintf(fid, "#  Age Error from parameters\n");
    fprintf(fid, "#  catch multiplier\n");
    fprintf(fid, "#  fraction female, by GP    \n");
    fprintf(fid, "     0.000001    0.999999    0.5    0.5    0.5       0   -99      0       0         0         0        0       0     0     # FracFemale_GP_1\n");
    // end of growth parameter block
    fprintf(fid, "#\n");
    fprintf(fid, "#_no timevary MG parameters\n");
    fprintf(fid, "#\n");
    fprintf(fid, "#_seasonal_effects_on_biology_parms\n");
    fprintf(fid, "0 0 0 0 0 0 0 0 0 0 #_femwtlen1,femwtlen2,mat1,mat2,fec1,fec2,Malewtlen1,malewtlen2,L1,K\n");
    fprintf(fid, "#_ LO HI INIT PRIOR PR_SD PR_type PHASE\n");
    fprintf(fid, "#_Cond -2 2 0 0 -1 99 -2 #_placeholder when no seasonal MG parameters\n");
    fprintf(fid, "#\n");
    fprintf(fid, "3 #_Spawner-Recruitment; Options: 1=NA; 2=Ricker; 3=std_B-H; 4=SCAA; 5=Hockey; 6=B-H_flattop; 7=survival_3Parm; 8=Shepherd_3Parm; 9=RickerPower_3parm\n");
    fprintf(fid, "0  # 0/1 to use steepness in initial equ recruitment calculation\n");
    fprintf(fid, "0  #  future feature:  0/1 to make realized sigmaR a function of SR curvature\n");
    fprintf(fid, "#_          LO            HI          INIT         PRIOR         PR_SD       PR_type      PHASE    env-var    use_dev   dev_mnyr   dev_mxyr     dev_PH      Block    Blk_Fxn #  parm_name\n");
    if (bm->RBCestimation.RBCspeciesParam[sp][Regime_year_assess_id] > 0){
        if (bm->RBCestimation.RBCspeciesParam[sp][SRBlock_id] < 0 ) {
            quit("SR Regime shift specified but no SR time varying block specified.\n");
        }
        fprintf(fid, "     3    31 %f %f       10    0   1 0    0    0    0    0  %d    1   # SR_LN(R0)\n", log(bm->RBCestimation.RBCspeciesArray[sp].R02[0]), log(bm->RBCestimation.RBCspeciesArray[sp].R02[0]), (int)(bm->RBCestimation.RBCspeciesParam[sp][SRBlock_id]));
    } else {
      fprintf(fid, "     3    31 %f %f       10    0   1  0    0    0    0    0    0    0  # SR_LN(R0)\n", log(bm->RBCestimation.RBCspeciesArray[sp].R02[0]), log(bm->RBCestimation.RBCspeciesArray[sp].R02[0]));
    }
    fprintf(fid, "   0.2     1 %f      0.7    0.2   0 %f  0    0    0    0    0    0    0  # SR_BH_steep\n", bm->RBCestimation.RBCspeciesArray[sp].SSHsteep[0], bm->RBCestimation.RBCspeciesParam[sp][T1_steep_phase_id]);
    fprintf(fid, "     0     2 %f %f        0.8   0    -2 0    0    0    0    0    0    0  # SR_sigmaR\n", bm->RBCestimation.RBCspeciesParam[sp][SigmaR1_id], bm->RBCestimation.RBCspeciesParam[sp][SigmaR1_id]);
    //  for  regime shift
    //if (bm->RBCestimation.RBCspeciesParam[sp][Regime_year_assess_id] > 0){
    //    if (bm->RBCestimation.RBCspeciesParam[sp][SRBlock_id] < 0 ) {
    //        quit("SR Regime shift specified but no SR time varying block specified.\n");
    //    }
    //    fprintf(fid, "    -5     5        0        0       1     0    -3   0    0    0    0    0    %f    1   # SR_regime\n", bm->RBCestimation.RBCspeciesParam[sp][SRBlock_id]);
    //}
    //else
    //    fprintf(fid, "    -5     5        0        0       1     0   -3   0    0    0    0    0    0    0   # SR_regime\n");
    fprintf(fid, "    -5     5        0        0       1     0   -3   0    0    0    0    0    0    0   # SR_regime\n");
    fprintf(fid, "    0     0        0        0       0    0    -99  0     0    0    0    0    0    0   # SR_autocorr\n");
    fprintf(fid, "#_no timevary SR parameters\n");
    // recruitment
    fprintf(fid, "2 #do_recdev:  0=none; 1=devvector (R=F(SSB)+dev); 2=deviations (R=F(SSB)+dev); 3=deviations (R=R0*dev; dev2=R-f(SSB)); 4=like 3 with sum(dev2) adding penalty\n");
    if (!bm->RBCestimation.RBCspeciesParam[sp][assRecDevMinYear_id]) {
         fprintf(fid, "%d    # first year of main recr-devs\n", (int)(bm->RBCestimation.RBCspeciesParam[sp][RecDevMinYr_id]));
    } else {
         fprintf(fid, "%d    # first year of main recr-devs\n", (int)(bm->RBCestimation.RBCspeciesParam[sp][assRecDevMinYear_id]));
    }
    fprintf(fid, "%d # last_year_of_main_rec_devs; forecast devs start in following year\n", maxyr-(int)(bm->RBCestimation.RBCspeciesParam[sp][RecDevBack_id]));
    fprintf(fid, "2 #3 #_recdev phase \n");
    fprintf(fid, "1 # (0/1) to read 13 advanced options\n");   // need this so can set lambda to 1000
    fprintf(fid, "0 #_recdev_early_start  (0=none; neg value makes relative to recdev_start)\n");    // don't use bias adjustment ramps as not in operating model
    fprintf(fid, "-4 #_recdev_early_phase\n");
    fprintf(fid, "-1 #_forecast_recruitment_phase (incl. late recr) (0 value resets to maxphase+1)\n");
    fprintf(fid, "1 #_lambda_for_Fcast_recr_like occurring before endyr+1\n");
    //* check the manual for when to start the bias ramp
    fprintf(fid, "%d #_last_yr_nobias_adj_in_MPD; begin of ramp\n", (int)(bm->RBCestimation.RBCspeciesParam[sp][HistYrMin_id]-50));
    if (!bm->RBCestimation.RBCspeciesParam[sp][assRecDevMinYear_id]) {
         fprintf(fid, "%d #_first_yr_fullbias_adj_in_MPD; begin of plateau\n", (int)(bm->RBCestimation.RBCspeciesParam[sp][RecDevMinYr_id]));
    } else {
         fprintf(fid, "%d #_first_yr_fullbias_adj_in_MPD; begin of plateau\n", (int)(bm->RBCestimation.RBCspeciesParam[sp][assRecDevMinYear_id]));
    }
    fprintf(fid, "%d #_last_yr_fullbias_adj_in_MPD\n", maxyr-(int)(bm->RBCestimation.RBCspeciesParam[sp][RecDevBack_id]));
    fprintf(fid, "%d #_end_yr_for_ramp_in_MPD (can be in forecast to shape ramp, but SS sets bias_adj to 0.0 for fcast yrs)\n", maxyr-(int)(bm->RBCestimation.RBCspeciesParam[sp][RecDevBack_id]+1));
    fprintf(fid, "1 #_max_bias_adj_in_MPD (typical ~0.8; -3 sets all years to 0.0; -2 sets all non-forecast yrs w/ estimated recdevs to 1.0; -1 sets biasadj=1.0 for all yrs w/ recdevs)\n");
    fprintf(fid, "0 #_period_of_cycles_in_recruitment (N parms read below)\n");
    fprintf(fid, "-5 #min_rec-dev\n");
    fprintf(fid, "5 #max_rec-dev\n");
    fprintf(fid, "0 # read_rec-devs\n");
    fprintf(fid, "#_end_of_advanced_SR_options\n");
    fprintf(fid, "#\n");
    fprintf(fid, "#_placeholder for full parameter lines for recruitment cycles\n");
    fprintf(fid, "# read specified recr devs\n");
    fprintf(fid, "#_Yr Input_value\n");
    fprintf(fid, "#\n");
    fprintf(fid, "# all recruitment deviations\n");
    fprintf(fid, "# Example file prints year1R, year2R ...\n");
    fprintf(fid, "# Example file prints rec devs by year\n");
    fprintf(fid, "#\n");
    fprintf(fid, "# implementation error by year in forecast:  0 0 0 0 0 0 0 0 0 0\n");
    fprintf(fid, "#\n");
    fprintf(fid, "#Fishing_Mortality_info \n");
    //* consider adding these parameters to the input files
    fprintf(fid, "0.2 # F ballpark value in units of annual_F\n");    // FLT was 0.3, MOW was 0.1, default 0.2
    fprintf(fid, "2000 # F ballpark year (neg value to disable)\n");   // FLT 2001, MOW 1999
    fprintf(fid, "3 # F_Method:  1=Pope; 2=instan. F; 3=hybrid (hybrid is recommended)\n");
    fprintf(fid, "4 # max F or harvest rate, depends on F_Method (0.9 recommended for F_Method 1 and 4 for F_Method 2 or 3)\n");
    fprintf(fid, "# no additional F input needed for Fmethod 1\n");
    fprintf(fid, "# if Fmethod=2; read overall start F value; overall phase; N detailed inputs to read\n");
    fprintf(fid, "# if Fmethod=3; read N iterations for tuning for Fmethod 3\n");
    fprintf(fid, "5  # N iterations for tuning F in hybrid method (recommend 3 to 7)\n");
    fprintf(fid, "#\n");
    fprintf(fid, "#_initial_F_parms; count = 0\n");
    fprintf(fid, "#_ LO HI INIT PRIOR PR_SD  PR_type  PHASE\n");
    for (f = 0; f < bm->RBCestimation.RBCspeciesParam[sp][NumFisheries_id]; f++){
        if (bm->RBCestimation.RBCspeciesArray[sp].initialEquilCatch[f] > 0) {
            fprintf(fid,  "0   3   0.1   0   99   0   1 #InitF fleet %d\n", f);
        }
    }

    fprintf(fid, "# F rates by fleet\n");
    fprintf(fid, "# Yr: ...    Not implemented in ratpack\n");
    fprintf(fid, "# seas: ...   Not implemented in ratpack\n");
    fprintf(fid, "# FISHERY ... Not implemented in ratpack\n");
    fprintf(fid, "#\n");
    fprintf(fid, "#_Q_setup for fleets with cpue or survey data\n");
    fprintf(fid, "#_1:  fleet number\n");
    fprintf(fid, "#_2:  link type: (1=simple q, 1 parm; 2=mirror simple q, 1 mirrored parm; 3=q and power, 2 parm; 4=mirror with offset, 2 parm)\n");
    fprintf(fid, "#_3:  extra input for link, i.e. mirror fleet# or dev index number\n");
    fprintf(fid, "#_4:  0/1 to select extra sd parameter\n");
    fprintf(fid, "#_5:  0/1 for biasadj or not\n");
    fprintf(fid, "#_6:  0/1 to float\n");
    fprintf(fid, "#_   fleet      link link_info  extra_se   biasadj     float  #  fleetname\n");
    
    // Q's for each fishery and survey with an index
    //* Do all fleets in ratpack have abundance indices? If not then below needs modification
    for (f = 0; f < (bm->RBCestimation.RBCspeciesParam[sp][NumFisheries_id] + bm->RBCestimation.RBCspeciesParam[sp][NumSurvey_id]); f++) {
        //guard
        if (bm->RBCestimation.RBCspeciesArray[sp].CPUEqmu[f] <= 0) continue;
    
        if( f < bm->RBCestimation.RBCspeciesParam[sp][NumFisheries_id]){
            if (bm->RBCestimation.speciesRPFleetToMetier[f][sp+1] > -1) {
                fprintf(fid,  "%d   1   0   0   0   0  # FLEET %d in metier %s\n", f, f, FisheryArray[f].fisheryCode);
            } else {
                fprintf(fid, "%d  1   0   0   0   0  # FLEET %d\n", f, f);
            }
        } else {
            fprintf(fid, "%d  1   0   0   0   0  # SURVEY %d\n", f, (int)(f - bm->RBCestimation.RBCspeciesParam[sp][NumFisheries_id]));
        }
    }
    fprintf(fid, "-9999 0 0 0 0 0 # terminator_for_Qs\n");
    fprintf(fid, "#\n");
    // For Q_params, CPUEqmu is used and no extra SD added
    fprintf(fid, "#_Q_parms(if_any);Qunits_are_ln(q)\n");
    fprintf(fid, "#_          LO            HI          INIT         PRIOR         PR_SD       PR_type      PHASE    env-var    use_dev   dev_mnyr   dev_mxyr     dev_PH      Block    Blk_Fxn  #  parm_name\n");

    for (f = 0; f < (bm->RBCestimation.RBCspeciesParam[sp][NumFisheries_id] + bm->RBCestimation.RBCspeciesParam[sp][NumSurvey_id]); f++) {
        //guard
        if (bm->RBCestimation.RBCspeciesArray[sp].CPUEqmu[f] <= 0) continue;

        if( f < bm->RBCestimation.RBCspeciesParam[sp][NumFisheries_id]){
            fprintf(fid,  "  -15             15 %f  0             1             0          1          0          0          0          0          0          0          0    # LnQ_base_FLEET %d (%d)\n",  log(bm->RBCestimation.RBCspeciesArray[sp].CPUEqmu[f]), f, f);
        } else {
            fprintf(fid,  "  -15             15  %f 0             1             0          1          0          0          0          0          0          0          0    # LnQ_base_SURVEY %d (%d)\n", log(bm->RBCestimation.RBCspeciesArray[sp].CPUEqmu[f]), f, (int)(f - bm->RBCestimation.RBCspeciesParam[sp][NumFisheries_id]));
        }
    }
    fprintf(fid, "# \n");
    fprintf(fid, "#_size_selex_patterns\n");
    fprintf(fid, "#Pattern:_0; parm=0; selex=1.0 for all sizes\n");
    fprintf(fid, "#Pattern:_1; parm=2; logistic; with 95 percent width specification\n");
    fprintf(fid, "#Pattern:_5; parm=2; mirror another size selex; PARMS pick the min-max bin to mirror\n");
    fprintf(fid, "#Pattern:_15; parm=0; mirror another age or length selex\n");
    fprintf(fid, "#Pattern:_6; parm=2+special; non-parm len selex\n");
    fprintf(fid, "#Pattern:_43; parm=2+special+2;  like 6, with 2 additional param for scaling (average over bin range)\n");
    fprintf(fid, "#Pattern:_8; parm=8; New doublelogistic with smooth transitions and constant above Linf option\n");
    fprintf(fid, "#Pattern:_9; parm=6; simple 4-parm double logistic with starting length; parm 5 is first length; parm 6=1 does desc as offset\n");
    fprintf(fid, "#Pattern:_21; parm=2+special; non-parm len selex, read as pairs of size, then selex\n");
    fprintf(fid, "#Pattern:_22; parm=4; double_normal as in CASAL\n");
    fprintf(fid, "#Pattern:_23; parm=6; double_normal where final value is directly equal to sp(6) so can be >1.0\n");
    fprintf(fid, "#Pattern:_24; parm=6; double_normal with sel(minL) and sel(maxL), using joiners\n");
    fprintf(fid, "#Pattern:_25; parm=3; exponential-logistic in size\n");
    fprintf(fid, "#Pattern:_27; parm=3+special; cubic spline \n");
    fprintf(fid, "#Pattern:_42; parm=2+special+3;\n");
    fprintf(fid, "#_discard_options:_0=none;_1=define_retention;_2=retention&mortality;_3=all_discarded_dead;_4=define_dome-shaped_retention \n");
    fprintf(fid, "#_Pattern Discard Male Special \n");
                    
    // for each fleet see if have any discard data in any years, if so can estimate retention
    dflt = Util_Alloc_Init_1D_Int(bm->RBCestimation.RBCspeciesParam[sp][NumFisheries_id], 0.0);
                    
    for (t = bm->RBCestimation.RBCspeciesParam[sp][HistYrMin_id]; t < maxyr; t++) {
        for (f = 0; f < bm->RBCestimation.RBCspeciesParam[sp][NumFisheries_id]; f++) {
            if (bm->RBCestimation.RBCspeciesArray[sp].DiscData[f][allregion][t] > 0) {
                dflt[f] = 1;
            }
        }
    }
    for (f = 0; f < (bm->RBCestimation.RBCspeciesParam[sp][NumFisheries_id] + bm->RBCestimation.RBCspeciesParam[sp][NumSurvey_id]); f++)  {
        disc = 0;    // survey has no discards
        if (f < bm->RBCestimation.RBCspeciesParam[sp][NumFisheries_id]) {
            if (dflt[f] > 0) {
                disc = 1;
            } else {
                disc = 0;
            }
        }
        // write the selectivity pattern to file
        if (bm->RBCestimation.RBCspeciesArray[sp].Sel_Pattern[f] != 5){
            fprintf(fid, "%d %d 0 0\n", bm->RBCestimation.RBCspeciesArray[sp].Sel_Pattern[f], disc);
        } else {
            if (bm->RBCestimation.RBCspeciesArray[sp].mirrored_fleet[f] < 0 ){
                quit(" --- Error --- \n  Len Selectivity pattern 5 specified for fleet %d\n but no fleet specified to mirror, \n i.e. bm->RBCestimation.RBCspeciesArray[sp].mirrored_fleet[f].\n ---------------\n", f);
            } else {
                fprintf(fid, "%d %d 0 %d ", bm->RBCestimation.RBCspeciesArray[sp].Sel_Pattern[f], disc, bm->RBCestimation.RBCspeciesArray[sp].mirrored_fleet[f]);
            }
        }
    }
    fprintf(fid, "#\n");
    fprintf(fid, "#_age_selex_patterns\n");
    fprintf(fid, "#Pattern:_0; parm=0; selex=1.0 for ages 0 to maxage\n");
    fprintf(fid, "#Pattern:_10; parm=0; selex=1.0 for ages 1 to maxage\n");
    fprintf(fid, "#Pattern:_11; parm=2; selex=1.0  for specified min-max age\n");
    fprintf(fid, "#Pattern:_12; parm=2; age logistic\n");
    fprintf(fid, "#Pattern:_13; parm=8; age double logistic\n");
    fprintf(fid, "#Pattern:_14; parm=nages+1; age empirical\n");
    fprintf(fid, "#Pattern:_15; parm=0; mirror another age or length selex\n");
    fprintf(fid, "#Pattern:_16; parm=2; Coleraine - Gaussian\n");
    fprintf(fid, "#Pattern:_17; parm=nages+1; empirical as random walk  N parameters to read can be overridden by setting special to non-zero\n");
    fprintf(fid, "#Pattern:_41; parm=2+nages+1; // like 17, with 2 additional param for scaling (average over bin range)\n");
    fprintf(fid, "#Pattern:_18; parm=8; double logistic - smooth transition\n");
    fprintf(fid, "#Pattern:_19; parm=6; simple 4-parm double logistic with starting age\n");
    fprintf(fid, "#Pattern:_20; parm=6; double_normal,using joiners\n");
    fprintf(fid, "#Pattern:_26; parm=3; exponential-logistic in age\n");
    fprintf(fid, "#Pattern:_27; parm=3+special; cubic spline in age\n");
    fprintf(fid, "#Pattern:_42; parm=2+special+3; // cubic spline; with 2 additional param for scaling (average over bin range)\n");
    fprintf(fid, "#Age patterns entered with value >100 create Min_selage from first digit and pattern from remainder\n");
    fprintf(fid, "#_Pattern Discard Male Special\n");
    for (i = 0; i < (bm->RBCestimation.RBCspeciesParam[sp][NumFisheries_id] + bm->RBCestimation.RBCspeciesParam[sp][NumSurvey_id]); i++){
        if (i < bm->RBCestimation.RBCspeciesParam[sp][NumFisheries_id]) {
            if (bm->RBCestimation.speciesRPFleetToMetier[i][sp+1] > -1) {
                fprintf(fid, "%d  0  0  0  # FLEET %d in metier %s", (int)(bm->RBCestimation.RBCspeciesParam[sp][Agesel_Pattern_id]), i, FisheryArray[i].fisheryCode);
            } else {
                fprintf(fid, "%d  0  0  0  # FLEET %d in metier", (int)(bm->RBCestimation.RBCspeciesParam[sp][Agesel_Pattern_id]), i);
            }
        } else {
            fprintf(fid, "%d  0  0  0  # SURVEY %d\n", (int)(bm->RBCestimation.RBCspeciesParam[sp][Agesel_Pattern_id]), (int)(i - bm->RBCestimation.RBCspeciesParam[sp][NumFisheries_id]));
        }
    }
    fprintf(fid, "# Selectivity_parameters\n");
    //* check this
    //* Prior SD seems too high here (hard coded to 99) perhaps a calculated value
    fprintf(fid, "#_ LO    HI     INIT    PRIOR PR_SD    PR_type  PHASE env-var use_dev dev_mnyr  dev_mxyr  dev_PH     Block Blk_Fxn  #  parm_name\n");
    for (f = 0; f < (bm->RBCestimation.RBCspeciesParam[sp][NumFisheries_id] + bm->RBCestimation.RBCspeciesParam[sp][NumSurvey_id]); f++) {
        // selectivity
        //* it might not be worth making this look like the 3.30 file format
         fprintf(fid, "# Length selectivity  Fleet %d\n", f);
        if (bm->RBCestimation.RBCspeciesArray[sp].Sel_Pattern[f] == 1)          // logistic
            nsp = 2;
        else if (bm->RBCestimation.RBCspeciesArray[sp].Sel_Pattern[f] == 24)    // double-normal
            nsp = 6;
        else if (bm->RBCestimation.RBCspeciesArray[sp].Sel_Pattern[f] == 5){        // mirrored
            nsp = 2;
        } else  {                            // stop because not coded yet
            quit("SS selectivity pattern (%d) for %s fleet %d not coded in\n", bm->RBCestimation.RBCspeciesArray[sp].Sel_Pattern[f], FunctGroupArray[sp].groupCode, f);
        }

        if (bm->RBCestimation.RBCspeciesArray[sp].Sel_Pattern[f] == 5) { // mirror
            fprintf(fid, "-10 100 1 1 99 -1 -5 0 0 0 0 0.5 0 0 # mirrored fleet %d  parm 1\n", bm->RBCestimation.RBCspeciesArray[sp].mirrored_fleet[f]);

            fprintf(fid, "-10 100 %d %d 99 -1 -5 0 0 0 0 0.5 0 0 # mirrored fleet %d parm 2\n", (int)(bm->RBCestimation.RBCspeciesParam[sp][Nlen_id]), (int)(bm->RBCestimation.RBCspeciesParam[sp][Nlen_id]), (int)(bm->RBCestimation.RBCspeciesArray[sp].mirrored_fleet[f]));
        } else {
            for (i = 0; i < nsp; i++) {
                if (bm->RBCestimation.RBCspeciesArray[sp].Start_Sel[f][i] <0.0) {
                    lo = bm->RBCestimation.RBCspeciesArray[sp].Start_Sel[f][i] * 2.0;
                    hi = bm->RBCestimation.RBCspeciesArray[sp].Start_Sel[f][i] / 2.0;
                } else {
                    lo = bm->RBCestimation.RBCspeciesArray[sp].Start_Sel[f][i] / 2.0;
                    hi = bm->RBCestimation.RBCspeciesArray[sp].Start_Sel[f][i] * 2.0;
                }
            
                if (bm->RBCestimation.RBCspeciesParam[sp][NblockPattern_id] > 0){
                    if (bm->RBCestimation.RBCspeciesArray[sp].SelBlock[f][i]>0) {
                        block_fxn = 2;
                    } else {
                        block_fxn = 0;
                    }
                    fprintf(fid, "%f %f %f %f       99  0 %f       0       0         0         0          0 %f %d\n", lo, hi, bm->RBCestimation.RBCspeciesArray[sp].Start_Sel[f][i], bm->RBCestimation.RBCspeciesArray[sp].Start_Sel[f][i], bm->RBCestimation.RBCspeciesArray[sp].Sel_Phase[f][i], bm->RBCestimation.RBCspeciesArray[sp].SelBlock[f][i], block_fxn);
                } else {
                    fprintf(fid, "%f %f %f %f        99  0 %f       0       0         0         0          0     0   0\n", lo, hi, bm->RBCestimation.RBCspeciesArray[sp].Start_Sel[f][i], bm->RBCestimation.RBCspeciesArray[sp].Start_Sel[f][i], bm->RBCestimation.RBCspeciesArray[sp].Sel_Phase[f][i]);
                }
            }
        }

        // retention
        if (f < bm->RBCestimation.RBCspeciesParam[sp][NumFisheries_id] ) {  // no discards for survey
            if (dflt[f] > 0) {  // if discard data
                fprintf(fid, "\n# Retention parameters fleet %d\n", f);
                sv = bm->RBCestimation.RBCspeciesArray[sp].Start_RetInflect[f];
                phase = 3;
                if (sv<0) {
                    sv = - sv;
                    phase = -3;
                }
                fprintf(fid, "%f %f %f %f        99  0 %d        0       0         0         0          0     0   0  # inflection for logistic retention\n", sv/2.0, sv*2.0, sv, sv, phase);
                sv = bm->RBCestimation.RBCspeciesArray[sp].Start_RetSlope[f];
                phase = 4;
                if (sv < 0) {
                    sv = - sv;
                    phase = -4;
                }
                fprintf(fid, "   0.1 %f %f %f       99  0 %d       0       0         0         0          0     0   0  # slope for logistic retention\n", sv*2.0, sv, sv, phase);
                fprintf(fid, " 0.001     1      1.0      0.1       99   0   -3       0       0         0   0          0     0         0\n");
                fprintf(fid, "   -10    10        0        0       99   0   -3       0       0         0   0          0     0         0\n\n");
            }
        }
    }

    /*
    if (bm->RBCestimation.RBCspeciesParam[sp][NblockPattern_id] > 0) {
         fprintf(fid, "1     # custom sel block setup\n");
         for (i = 0; i < bm->RBCestimation.RBCspeciesParam[sp][Ncustom;i++) {
             for (j = 0; j < 7; j++) {
                  fprintf(fid, bm->RBCestimation.RBCspeciesParam[sp].CustomSelBlock[i][j];
             }
             fprintf(fid, "\n");"
         }
         fprintf(fid, "1   # env/block dev adjust method\n");
    }
    */

    if (bm->RBCestimation.RBCspeciesParam[sp][Agesel_Pattern_id] == 11) {
        fprintf(fid, "# Age parameters\n");
        for (f = 0; f < (bm->RBCestimation.RBCspeciesParam[sp][NumFisheries_id] + bm->RBCestimation.RBCspeciesParam[sp][NumSurvey_id]); f++) {
             fprintf(fid, "# Fleet %d\n", f);
             fprintf(fid, "     0 %d       0.1      0.1       99   0   -3       0       0         0         0          0     0         0  # min age\n", (int)(bm->RBCestimation.RBCspeciesParam[sp][AccumAge_id]));
             fprintf(fid, "     0 %d %d %d       99  0   -3       0       0         0         0          0     0         0  # max age\n", (int)(bm->RBCestimation.RBCspeciesParam[sp][AccumAge_id]), (int)(bm->RBCestimation.RBCspeciesParam[sp][AccumAge_id]), (int)(bm->RBCestimation.RBCspeciesParam[sp][AccumAge_id]));
        }
        fprintf(fid,"\n");
    }
    // end of selectivity parameters
    fprintf(fid, "#_no timevary selex parameters\n");
    fprintf(fid, "#\n");
    fprintf(fid, "0   # use_2D_AR1_selectivity(0/1)\n");
    fprintf(fid, "#_no 2D_AR1 selex offset used\n");
    fprintf(fid, "#\n");
    fprintf(fid, "# Tag loss and Tag reporting parameters go next\n");
    fprintf(fid, "0   # TG_custom:  0=no read and autogen if tag data exist; 1=read\n");
    fprintf(fid, "# Cond -6 6 1 1 2 0.01 -4 0 0 0 0 0 0 0  #_placeholder if no parameters\n");
    fprintf(fid, "#\n");
    fprintf(fid, "# no_timevary_parameters\n");
    fprintf(fid, "#\n");
    fprintf(fid, "#\n");
    fprintf(fid, "# Input variance adjustments factors: \n");
    fprintf(fid, " #_1=add_to_survey_CV\n");
    fprintf(fid, " #_2=add_to_discard_stddev\n");
    fprintf(fid, " #_3=add_to_bodywt_CV\n");
    fprintf(fid, " #_4=mult_by_lencomp_N\n");
    fprintf(fid, " #_5=mult_by_agecomp_N\n");
    fprintf(fid, " #_6=mult_by_size-at-age_N\n");
    fprintf(fid, " #_7=mult_by_generalized_sizecomp\n");
    fprintf(fid, "#_Factor  Fleet  Value\n");
                
    // add variance adjustment factors
    for (f = 0; f < (bm->RBCestimation.RBCspeciesParam[sp][NumFisheries_id] + bm->RBCestimation.RBCspeciesParam[sp][NumSurvey_id]); f++) {
        if (bm->RBCestimation.RBCspeciesArray[sp].Varadj_CPUE[f]) {
             fprintf(fid, "1 %d %f\n", f, bm->RBCestimation.RBCspeciesArray[sp].Varadj_CPUE[f]);
        }
    }
    for (f = 0; f < (bm->RBCestimation.RBCspeciesParam[sp][NumFisheries_id] + bm->RBCestimation.RBCspeciesParam[sp][NumSurvey_id]); f++) {
        if (bm->RBCestimation.RBCspeciesArray[sp].Varadj_discard[f]) {
             fprintf(fid, "2 %d %f\n", f, bm->RBCestimation.RBCspeciesArray[sp].Varadj_discard[f]);
        }
    }
    for (f = 0; f < (bm->RBCestimation.RBCspeciesParam[sp][NumFisheries_id] + bm->RBCestimation.RBCspeciesParam[sp][NumSurvey_id]); f++) {
        if (bm->RBCestimation.RBCspeciesArray[sp].Varadj_length[f]) {
             fprintf(fid, "4 %d %f\n", f, bm->RBCestimation.RBCspeciesArray[sp].Varadj_length[f]);
        }
    }
    for (f = 0; f < (bm->RBCestimation.RBCspeciesParam[sp][NumFisheries_id] + bm->RBCestimation.RBCspeciesParam[sp][NumSurvey_id]); f++) {
        if (bm->RBCestimation.RBCspeciesArray[sp].Varadj_age[f]) {
             fprintf(fid, "5 %d %f\n", f, bm->RBCestimation.RBCspeciesArray[sp].Varadj_age[f]);
        }
    }
    //
    fprintf(fid, " -9999   1    0  # terminator for variance adjustment factors\n");
    fprintf(fid, "#\n");
    fprintf(fid, "1 #_maxlambdaphase\n");
    fprintf(fid, "1 #_sd_offset; must be 1 if any growthCV, sigmaR, or survey extraSD is an estimated parameter\n");
    fprintf(fid, "# read 3 changes to default Lambdas (default value is 1.0)\n");
    fprintf(fid, "# Like_comp codes:  1=surv; 2=disc; 3=mnwt; 4=length; 5=age; 6=SizeFreq; 7=sizeage; 8=catch; 9=init_equ_catch; \n");
    fprintf(fid, "# 10=recrdev; 11=parm_prior; 12=parm_dev; 13=CrashPen; 14=Morphcomp; 15=Tag-comp; 16=Tag-negbin; 17=F_ballpark; 18=initEQregime\n");
    fprintf(fid, "#like_comp fleet  phase  value  sizefreq_method\n");
    fprintf(fid, "# lambdas are yet to be implemented in ratpack\n");
    fprintf(fid, "18  1 1 0 1\n");
    fprintf(fid, " -9999  1  1  1  1  #  terminator for lambdas\n");
    fprintf(fid, "#\n");
    fprintf(fid, "# lambdas (for info only; columns are phases)\n");
    fprintf(fid, "# lambdas are not printed by ratpack\n");
    fprintf(fid, "0 # (0/1/2) read specs for more stddev reporting: 0 = skip, 1 = read specs for reporting stdev for selectivity, size, and numbers, 2 = mortality\n");
    fprintf(fid, "# Not implemented in ratpack\n");
    fprintf(fid, "999 # terminator for the ctl file\n");
    fprintf(fid, "\n");
    fprintf(fid, "\n");
    
    i_free1d(dflt);
    
    fclose(fid);// close the ctl file
}

//******************************************************************************
//
// Name:  WriteSSFor
// Description: write the files for input to SS v3.30.15.03
//
//
// called by : _WriteSSFiles
// calls :
// created  : Feb 2011 Sally
// updated  : Nov 2012 Sally for SS v3.24
// updated  : Sep 2013 MSY=3 not 2,Fcast years = 0 0 0 0, not 0 0 -5 0
// updated  : Feb 2020 SS v3.30
//
//******************************************************************************
void WriteSSFor(MSEBoxModel *bm, FILE *fid, int sp, int maxyr) {
    int f,allregion;
    double last_catch;
    double *cat;

    // initial header block
    fprintf(fid, "# SS v3.30.15.03 forcast file for %s written by Atlantis\n", FunctGroupArray[sp].name);

    fprintf(fid, "1 # Benchmarks: 0=skip; 1=calc F_spr,F_btgt,F_msy; 2=calc F_spr,F0.1,F_msy \n");
    fprintf(fid, "3 # MSY: 1= set to F(SPR); 2=calc F(MSY); 3=set to F(Btgt) or F0.1; 4=set to F(endyr) \n");
    fprintf(fid, "%f    # SPR target\n", bm->RBCestimation.RBCspeciesParam[sp][mgt_HCRtarget_id]);
    fprintf(fid, "%f    # Biomass target\n", bm->RBCestimation.RBCspeciesParam[sp][mgt_HCRtarget_id]);
    //* the values below may need to be the appropriate years rather than zeroes. The manual isn't informative but provides an example of 10 zeroes
    fprintf(fid, "#   Bmark_years: beg_bio, end_bio, beg_selex, end_selex, beg_relF, end_relF, beg_recr_dist, end_recr_dist, beg_SRparm, end_SRparm (enter actual year, or values of 0 or -integer to be rel. endyr)\n");
    fprintf(fid, " 0 0 0 0 0 0 0 0 0 0\n");
    fprintf(fid, "1 # Bmark_relF_Basis: 1 = use year range; 2 = set relF same as forecast below\n");
    fprintf(fid, "#\n");
    fprintf(fid, "3 # Forecast: -1=none; 0=simple_1yr; 1=F(SPR); 2=F(MSY) 3=F(Btgt) or F0.1; 4=Ave F (uses first-last relF yrs); 5=input annual F scalar\n");
    fprintf(fid, "# where none and simple require no input after this line; simple sets forecast F same as end year F \n");
    fprintf(fid, "51 # N forecast years\n");
    fprintf(fid, "0 # Fmult (only used for Do_Forecast==5) such that apical_F(f)=Fmult*relF(f)\n");
    fprintf(fid, "#_Fcast_years:  beg_selex, end_selex, beg_relF, end_relF, beg_mean recruits, end_recruits  (enter actual year, or values of 0 or -integer to be rel. endyr)\n");
    //* the example in the manual is 6 zeroes, the values below were used in 2019 SESSF assessments
    fprintf(fid, "0 0 0 0 -999 0\n");
    fprintf(fid, "0 # Forecast_selectivity (0=fcast selex is mean from year range; 1=fcast selectivity from annual time-vary parms)\n");
    fprintf(fid, "2 # Control rule method (1: ramp does catch=f(SSB), buffer on F; 2: ramp does F=f(SSB), buffer on F; 3: ramp does catch=f(SSB), buffer on catch; 4: ramp does F=f(SSB), buffer on catch) \n");
    fprintf(fid, "%f # Control rule Biomass level for constant F (as frac of Bzero, e.g. 0.40)\n", bm->RBCestimation.RBCspeciesParam[sp][mgt_HCRbreak_id]);
    /*
    if(0) {
        fprintf(bm->logFile," ------------------------------\n limit reference point  = 0.01 \n------------------------------\n");
        fprintf(fid, "0.01   # Control_rule_Biomass_level_for_no_F (as frac of Bzero, e.g. 0.10)\n");
    } else {
    */
    fprintf(fid, "%f # Control_rule_Biomass_level_for_no_F (as frac of Bzero, e.g. 0.10)\n", bm->RBCestimation.RBCspeciesParam[sp][mgt_HCRlimit_id]);
    //}
    fprintf(fid, "1 # Buffer:  enter Control rule target as fraction of Flimit (e.g. 0.75), negative value invokes list of [year, scalar] with filling from year to YrMax \n");
    fprintf(fid, "3 # N_forecast_loops (1=OFL only; 2=ABC; 3=get F from forecast ABC catch with allocations applied)\n");
    fprintf(fid, "3 # First_forecast_loop_with_stochastic_recruitment\n");
    fprintf(fid, "0 #_Forecast recruitment:  0= spawn_recr (<=SS3.30.10); 1=value*spawn_recr_fxn; 2=value*VirginRecr; 3=recent mean from yr range above (need to set phase to -1 in control to get constant recruitment in MCMC)\n");
    fprintf(fid, "1 # value is multiplier of SRR (if 1 or 2 selected above then this is a scalar applied to recruitment. If 3 = number of years to average recruitment. If 0 then ignored.\n");
    fprintf(fid, "0 #_Forecast loop control #5 (reserved for future bells&whistles) \n");
    fprintf(fid, "%d # FirstYear_for_caps_and_allocations (should be after years with fixed inputs) \n", maxyr+2);
    fprintf(fid, "0 # stddev_of_log (realized F/target F) in forecast (set value>0.0 to cause active impl_error)\n");
    fprintf(fid, "0 # Do_West_Coast_gfish_rebuilder output (0/1)\n");
    fprintf(fid, "0 # Rebuilder:  first year catch could have been set to zero (Ydecl)(-1 to set to 1999)\n");
    fprintf(fid, "0 # Rebuilder:  year for current age structure (Yinit) (-1 to set to endyear+1)\n");
    fprintf(fid, "1 # fleet relative F:  1=use first-last alloc year; 2=read seas, fleet, alloc list below\n");
    fprintf(fid, "# Note that fleet allocation is used directly as average F if Do_Forecast=4 \n");
    fprintf(fid, "3 # basis for fcast catch tuning and for fcast catch caps and allocation  (2=deadbio; 3=retainbio; 5=deadnum; 6=retainnum); NOTE: same units for all fleets\n");
    fprintf(fid, "# Conditional input if relative F choice = 2\n");
    fprintf(fid, "# Fleet_relative_F:  rows are seasons, columns are fleets\n");
    fprintf(fid, "# relative F not implemented in ratpack\n");
    fprintf(fid, "# enter list of fleet number and max for fleets with max annual catch; terminate with fleet=-9999\n");
    fprintf(fid, "-9999 -1\n");
    fprintf(fid, "# enter list of area ID and max annual catch; terminate with area=-9999\n");
    fprintf(fid, "-9999 -1\n");
    fprintf(fid, "# enter list of fleet number and allocation group assignment, if any; terminate with fleet=-9999\n");
    fprintf(fid, "-9999 -1\n");
    fprintf(fid, "#_if N allocation groups >0, list year, allocation fraction for each group\n");
    fprintf(fid, "# list sequentially because read values fill to end of N forecast\n");
    fprintf(fid, "# terminate with -9999 in year field\n");
    fprintf(fid, "# no allocation groups\n");
    fprintf(fid, "3 # basis for input Fcast catch: -1=read basis with each obs; 2=dead catch; 3=retained catch; 99=input apical_F; NOTE: bio vs num based on fleet's catchunits\n");
    fprintf(fid, "#enter list of Fcast catches; terminate with line having year=-9999\n");
    fprintf(fid, "#_Yr Seas Fleet Catch(or_F)\n");
    // need to add an extra year's catch, so can get TAC for maxyr+2
    // estimate from last year's catch (maxyr) X this years TAC (bm->RBCestimation.RBCspeciesParam[sp].mgt.TAC_old)
    allregion = 0;
    
    cat = Util_Alloc_Init_1D_Double(bm->RBCestimation.RBCspeciesParam[sp][NumFisheries_id], 0.0);
    last_catch = 0;
    for (f = 0; f < bm->RBCestimation.RBCspeciesParam[sp][NumFisheries_id]; f++) {
        last_catch += bm->RBCestimation.RBCspeciesArray[sp].CatchData[f][allregion][maxyr];
    }
    for (f = 0; f < bm->RBCestimation.RBCspeciesParam[sp][NumFisheries_id]; f++) {
        cat[f] = bm->RBCestimation.RBCspeciesArray[sp].CatchData[f][allregion][maxyr] * bm->RBCestimation.RBCspeciesArray[sp].mgtTAC_old / last_catch;
         fprintf(fid, "%d 1 %d %f\n,", maxyr+1, f, cat[f]);
    }
    fprintf(fid, "-9999 1 1 0  # terminator_for_Fcast_catches\n");
    fprintf(fid, "#\n");
    fprintf(fid, "999     #  terminator_for_the_for_file \n");

    fclose(fid);
}

void Write_SS_Control_File(MSEBoxModel *bm, char *dirName, char *fileName, int maxyr, int groupIndex, int versionID) {

	FILE *fid;
	int disc, phase;
	int _Nblock_Patterns = 0;
	int fleetIndex, yearIndex;
	double mortality, lminf, lmaxf, vbk, cvest, hi, lo, mval, lminm, lmaxm, sv;
	char str[STRLEN];
	int startYear, fisheryIndex;
	int HistYrMin;
    //int Growthage_L1 = (int)(bm->RBCestimation.RBCspeciesParam[groupIndex][Growthage_L1_id]);
	double Nsexes = (double) (bm->K_num_sexes);
	int *dflt = malloc(sizeof(int) * (size_t)bm->RBCestimation.RBCspeciesParam[groupIndex][NumFisheries_id]);
	int allregion = 0;    // uses weighted sum over regions, assume assessment doesn't know about regions
	int recruit_sp;

	if (bm->RBCestimation.RBCspeciesArray[groupIndex].CurrentYear == (bm->RBCestimation.RBCspeciesParam[groupIndex][HistYrMax_id] - 1))
		HistYrMin = (int)bm->RBCestimation.RBCspeciesParam[groupIndex][HistYrMin_id];
        //HistYrMin = 0;
	else
		HistYrMin = (int)bm->RBCestimation.RBCspeciesArray[groupIndex].CurrentYear + 1 - (int)bm->RBCestimation.RBCspeciesParam[groupIndex][HistYrMin_id];

	sscanf(bm->t_units, "seconds since %d-%s", &startYear, str);

	recruit_sp = (int) (FunctGroupArray[groupIndex].speciesParams[flagrecruit_id]);

	snprintf(str, sizeof(str), "%s%s%s", dirName, FOLDER_SEP, fileName);

	printf("fileName = %s\n", str);

	if ((fid = fopen(str, "w")) == NULL)
		quit("Create_Control_File: Can't open %s\n", str);

	printf("versionID = %d\n", versionID);

	fprintf(fid, "#V3.24f\n");
	fprintf(fid, "#C growth parameters are estimated\n");
	fprintf(fid, "#C spawner-recruitment bias adjustment Not tuned For optimality\n");
	fprintf(fid, "#_data_and_control_files: simple.dat // simple.ctl\n");
	fprintf(fid, "#_SS-V3.24f-safe-Win64;_08/03/2012;_Stock_Synthesis_by_Richard_Methot_(NOAA)_using_ADMB_11\n");
	fprintf(fid, "%d #_N_Growth_Patterns\n", bm->RBCestimation.SSnumGrowthPatterns);
	fprintf(fid, "%d #_N_Morphs_Within_GrowthPattern\n", (int) (bm->RBCestimation.RBCspeciesParam[groupIndex][num_growth_morphs_id]));

	fprintf(fid, "#_Cond 1 #_Morph_between/within_stdev_ratio (no read if N_morphs=1)\n");
	fprintf(fid, "#_Cond  1 #vector_Morphdist_(-1_in_first_val_gives_normal_approx)\n");
	fprintf(fid, "#\n");

	fprintf(fid, "#_Cond 0  #  N recruitment designs goes here if N_GP*nseas*area>1\n");
	fprintf(fid, "#_Cond 0  #  placeholder for recruitment interaction request\n");
	fprintf(fid, "#_Cond 1 1 1  # example recruitment design element for GP=1, seas=1, area=1\n");
	fprintf(fid, "#\n");
	fprintf(fid, "#_Cond 0 # N_movement_definitions goes here if N_areas > 1\n");
	fprintf(fid, "#_Cond 1.0 # first age that moves (real age at begin of season, not integer) also cond on do_migration>0\n");
	fprintf(fid, "#_Cond 1 1 1 2 4 10 # example move definition for seas=1, morph=1, source=1 dest=2, age1=4, age2=10\n");
	fprintf(fid, "#\n");

	fprintf(fid, "%d #_Nblock_Patterns \n", _Nblock_Patterns);

	fprintf(fid, "#_Cond 0 #_blocks_per_pattern\n");
	fprintf(fid, "# begin and end years of blocks\n");
	fprintf(fid, "#\n");
	fprintf(fid, "%e #_fracfemale \n", (double)(1.0 / bm->K_num_sexes));

	fprintf(fid, "0 #_natM_type:_0=1Parm;1=N_breakpoints;_2=Lorenzen;_3=agespecific;_4=agespec_withseasinterpolate\n");
	fprintf(fid, "  #_no additional input for selected M option; read 1P per morph\n");
	fprintf(fid, "1 # GrowthModel: 1=vonBert with L1&L2; 2=Richards with L1&L2; 3=age_speciific_K; 4=not implemented\n");

	/* These should perhaps be read in from an input file */
	fprintf(fid, "%d #_Growth_Age_for_L1\n", (int) (bm->RBCestimation.RBCspeciesParam[groupIndex][Growthage_L1_id]));
	fprintf(fid, "%d #_Growth_Age_for_L2 (999 to use as Linf)\n", (int) (bm->RBCestimation.RBCspeciesParam[groupIndex][Growthage_L2_id]));
	fprintf(fid, "0 #_SD_add_to_LAA (set to 0.1 for SS2 V1.x compatibility)\n");
	fprintf(fid, "0 #_CV_Growth_Pattern:  0 CV=f(LAA); 1 CV=F(A); 2 SD=F(LAA); 3 SD=F(A); 4 logSD=F(A)\n");
	fprintf(fid, "1 #_maturity_option:  1=length logistic; 2=age logistic; 3=read age-maturity matrix by growth_pattern; 4=read age-fecundity; 5=read fec and wt from wtatage.ss\n");
	fprintf(fid, "#_placeholder for empirical age-maturity by growth pattern\n");
	fprintf(fid, "%d #_First_Mature_Age\n",(int)(FunctGroupArray[groupIndex].speciesParams[age_mat_id] * FunctGroupArray[groupIndex].ageClassSize));
	fprintf(fid, "1 #_fecundity option:(1)eggs=Wt*(a+b*Wt);(2)eggs=a*L^b;(3)eggs=a*Wt^b; (4)eggs=a+b*L; (5)eggs=a+b*W\n");
	fprintf(fid, "0 #_hermaphroditism option:  0=none; 1=age-specific fxn\n");
	fprintf(fid, "%d #_parameter_offset_approach (1=none, 2= M, G, CV_G as offset from female-GP1, 3=like SS2 V1.x)\n",
			(int) (bm->RBCestimation.RBCspeciesParam[groupIndex][MG_offset_id]));
	fprintf(fid, "1 #_env/block/dev_adjust_method (1=standard; 2=logistic transform keeps in base parm bounds; 3=standard w/ no bound check)\n");

	fprintf(fid, "#\n");
	fprintf(fid, "#_growth_parms\n");
	fprintf(fid, "#_LO HI INIT PRIOR PR_type SD PHASE env-var use_dev dev_minyr dev_maxyr dev_stddev Block Block_Fxn\n");

	mortality = bm->RBCestimation.RBCspeciesArray[groupIndex].SSMort[0][FEMALE];

	printf("HistYrMin = %d\n", HistYrMin);

	fprintf(fid, " %f %f %f %f 0 0.8 -3 0 0 0 0 0.5 0 0 # NatM_p_1_Fem_GP_1\n", mortality / 2.0, mortality * 2.0, mortality, mortality);

	lminf = bm->RBCestimation.RBCspeciesArray[groupIndex].VBLinf[0][FEMALE]
                * (1.0 - exp( -1.0 * bm->RBCestimation.RBCspeciesArray[groupIndex].VBk[0][FEMALE]
                                * (bm->RBCestimation.RBCspeciesParam[groupIndex][Growthage_L1_id]
                                - bm->RBCestimation.RBCspeciesArray->VBt0[0][FEMALE])));

	fprintf(fid, " %f %f %f %f 0 10 -3 0 0 0 0 0.5 0 0 # L_at_Amin_Fem_GP_1\n", lminf / 2.0, lminf * 2.0, lminf, lminf);

	if (bm->RBCestimation.RBCspeciesParam[groupIndex][Growthage_L2_id] < 999)
		lmaxf = bm->RBCestimation.RBCspeciesArray[groupIndex].VBLinf[0][FEMALE]
				* (1.0 - exp( -1.0 * bm->RBCestimation.RBCspeciesArray[groupIndex].VBk[0][FEMALE]
                                * (bm->RBCestimation.RBCspeciesParam[groupIndex][Growthage_L2_id]
                                    - bm->RBCestimation.RBCspeciesArray->VBt0[0][FEMALE])));
	else
		lmaxf = bm->RBCestimation.RBCspeciesArray[groupIndex].VBLinf[0][FEMALE];     // use Linf

	fprintf(fid, " %f %f %f %f 0 10 -3 0 0 0 0 0.5 0 0 # L_at_Amax_Fem_GP_1\n", lmaxf / 2.0, lmaxf * 2.0, lmaxf, lmaxf);

	vbk = bm->RBCestimation.RBCspeciesArray[groupIndex].VBk[0][FEMALE];
	fprintf(fid, " %f %f %f %f 0 0.8 -3 0 0 0 0 0.5 0 0 # VonBert_K_Fem_GP_1\n", vbk / 3.0, vbk * 3.0, vbk, vbk);

	cvest = bm->RBCestimation.RBCspeciesArray[groupIndex].CvLA0[0][0];
	fprintf(fid, " %f %f %f %f 0 0.8 -2 0 0 0 0 0.5 0 0 # CV_young_Fem_GP_1\n", cvest / 3.0, cvest * 3.0, cvest, cvest);

	if (bm->RBCestimation.RBCspeciesParam[groupIndex][MG_offset_id] == 3) {
		cvest = log(fabs(bm->RBCestimation.RBCspeciesArray[groupIndex].CvLAmax[0][0]) / fabs(bm->RBCestimation.RBCspeciesArray[groupIndex].CvLA0[0][0]));
		lo = -1.0;
		hi = 1.0;
	} else {
		cvest = fabs(bm->RBCestimation.RBCspeciesArray[groupIndex].CvLAmax[0][0]);
		lo = cvest / 3.0;
		hi = cvest * 3.0;
	}

	fprintf(fid, " %f %f %f %f 0 0.8 -3 0 0 0 0 0.5 0 0 #CV_old_Fem_GP_1\n", lo, hi, cvest, cvest);

	if (Nsexes == 2) {
		if (bm->RBCestimation.RBCspeciesParam[groupIndex][MG_offset_id] == 3) {
			mval = log(fabs(bm->RBCestimation.RBCspeciesArray[groupIndex].SSMort[0][FEMALE]) / fabs(bm->RBCestimation.RBCspeciesArray[groupIndex].SSMort[0][FEMALE]));
			lo = -1.0;
			hi = 1.0;
		} else {
			mval = fabs(bm->RBCestimation.RBCspeciesArray[groupIndex].SSMort[0][FEMALE]);
			lo = mval / 2.0;
			hi = mval * 2.0;
		}

		fprintf(fid, " %f %f %f %f 0 0.8 -3 0 0 0 0 0.5 0 0 # NatM_p_1_Male_GP_1\n", lo, hi, mval, mval);

		if (bm->RBCestimation.RBCspeciesParam[groupIndex][MG_offset_id] == 3) {
			lminm = bm->RBCestimation.RBCspeciesArray[groupIndex].VBLinf[0][FEMALE]
					* (1.0 - exp( -1.0 * bm->RBCestimation.RBCspeciesArray[groupIndex].VBk[0][FEMALE]
                                    * (bm->RBCestimation.RBCspeciesParam[groupIndex][Growthage_L1_id]
                                    - bm->RBCestimation.RBCspeciesArray[groupIndex].VBt0[0][FEMALE])));
			lminm = log(lminm / lminf);
			//lmin = log(MeanLenAge[0][FEMALE][Growthage_L1])/MeanLenAge[0][FEMALE][Growthage_L1]);
			lo = -1.0;
			hi = 1.0;
		} else {
			lminm = bm->RBCestimation.RBCspeciesArray[groupIndex].VBLinf[0][FEMALE]
					* (1.0 - exp( -1.0 * bm->RBCestimation.RBCspeciesArray[groupIndex].VBk[0][FEMALE]
                                    * (bm->RBCestimation.RBCspeciesParam[groupIndex][Growthage_L1_id]
                                    - bm->RBCestimation.RBCspeciesArray[groupIndex].VBt0[0][FEMALE])));
			//lmin = MeanLenAge[0][FEMALE][Growthage_L1];
			lo = lminm / 2.0;
			hi = lminm * 2.0;
		}
		fprintf(fid, " %f %f %f %f 0 10 -3 0 0 0 0 0.5 0 0 # L_at_Amin_Male_GP_1\n", lo, hi, lminm, lminm);

		if (bm->RBCestimation.RBCspeciesParam[groupIndex][Growthage_L2_id] < 999)
			lmaxm = bm->RBCestimation.RBCspeciesArray[groupIndex].VBLinf[0][FEMALE]
					* (1.0 - exp(-1.0 * bm->RBCestimation.RBCspeciesArray[groupIndex].VBk[0][FEMALE]
                                    * (bm->RBCestimation.RBCspeciesParam[groupIndex][Growthage_L2_id]
                                    - bm->RBCestimation.RBCspeciesArray[groupIndex].VBt0[0][FEMALE])));
		else
			lmaxm = bm->RBCestimation.RBCspeciesArray[groupIndex].VBLinf[0][FEMALE];

		if (bm->RBCestimation.RBCspeciesParam[groupIndex][MG_offset_id] == 3) {
			lmaxm = log(lmaxm / lmaxf);
			//lmaxm = log(MeanLenAge[0][FEMALE][MaxAge][HistYrMin]/MeanLenAge[0][FEMALE][MaxAge][HistYrMin]);
			lo = -1.0;
			hi = 1.0;
		} else {
			//lmax = MeanLenAge[0][FEMALE][MaxAge][HistYrMin];
			lo = lmaxm / 2.0;
			hi = lmaxm * 2.0;
		}
		fprintf(fid, " %f %f %f %f 0 10 -3 0 0 0 0 0.5 0 0 # L_at_Amax_Male_GP_1\n", lo, hi, lmaxm, lmaxm);

		if (bm->RBCestimation.RBCspeciesParam[groupIndex][MG_offset_id] == 3) {
			vbk = log(bm->RBCestimation.RBCspeciesArray[groupIndex].VBk[0][FEMALE] / bm->RBCestimation.RBCspeciesArray[groupIndex].VBk[0][FEMALE]);
			lo = -1.0;
			hi = 1.0;
		} else {
			vbk = bm->RBCestimation.RBCspeciesArray[groupIndex].VBk[0][FEMALE];
			lo = vbk / 2.0;
			hi = vbk * 2.0;
		}

		fprintf(fid, " %f %f %f %f 0 0.8 -3 0 0 0 0 0.5 0 0 # VonBert_K_Male_GP_1\n", lo, hi, vbk, vbk);

		if (bm->RBCestimation.RBCspeciesParam[groupIndex][MG_offset_id] == 3) {
			cvest = log(bm->RBCestimation.RBCspeciesArray[groupIndex].CvLA0[0][1] / bm->RBCestimation.RBCspeciesArray[groupIndex].CvLA0[0][0]);
			lo = -1.0;
			hi = 1.0;
		} else {
			cvest = bm->RBCestimation.RBCspeciesArray[groupIndex].CvLA0[0][1];
			lo = cvest / 3.0;
			hi = cvest * 3.0;
		}

		fprintf(fid, " %f %f %f %f 0 0.8 -3 0 0 0 0 0.5 0 0 # CV_young_Male_GP_1\n", lo, hi, cvest, cvest);

		if (bm->RBCestimation.RBCspeciesParam[groupIndex][MG_offset_id] == 3) {
			cvest = log(bm->RBCestimation.RBCspeciesArray[groupIndex].CvLAmax[0][0] / bm->RBCestimation.RBCspeciesArray[groupIndex].CvLA0[0][0]);
			lo = -1.0;
			hi = 1.0;
		} else {
			cvest = bm->RBCestimation.RBCspeciesArray[groupIndex].CvLAmax[0][0];
			lo = cvest / 3.0;
			hi = cvest * 3.0;
		}
		fprintf(fid, " %f %f %f %f 0 0.8 -3 0 0 0 0 0.5 0 0 # CV_old_Male_GP_1\n", lo, hi, cvest, cvest);

	}

	fprintf(fid, "# wt-len and mat-len parameters\n");

	fprintf(fid, "#   LO HI INIT PRIOR PR_TYPE SD PHASE env-var use-dev dev_minyr dev_maxyr dev_stddev block block_fxn\n");

	fprintf(fid, " -3 3 %f %f 0 10 -3 0 0 0 0 0.5 0 0 # Wtlen_1_Fem\n", bm->RBCestimation.RBCspeciesArray[groupIndex].Wtlen_a[0][FEMALE],
			bm->RBCestimation.RBCspeciesArray[groupIndex].Wtlen_a[0][FEMALE]);

	fprintf(fid, " 0 6 %f %f 0 10 -3 0 0 0 0 0.5 0 0 # Wtlen_2_Fem\n", bm->RBCestimation.RBCspeciesArray[groupIndex].Wtlen_b[0][FEMALE],
			bm->RBCestimation.RBCspeciesArray[groupIndex].Wtlen_b[0][FEMALE]);

	fprintf(fid, " 0 6 %f %f 0 10 -3 0 0 0 0 0.5 0 0 # Wtlen_2_Fem\n", bm->RBCestimation.RBCspeciesArray[groupIndex].Wtlen_b[0][FEMALE],
			bm->RBCestimation.RBCspeciesArray[groupIndex].Wtlen_b[0][FEMALE]);

	fprintf(fid, " %f %f %f %f 0 10 -3 0 0 0 0 0.5 0 0 # Mat50_Fem\n", bm->RBCestimation.RBCspeciesParam[groupIndex][Maturity_Inflect_id] / 3.0,
			bm->RBCestimation.RBCspeciesParam[groupIndex][Maturity_Inflect_id] * 3.0, bm->RBCestimation.RBCspeciesParam[groupIndex][Maturity_Inflect_id],
			bm->RBCestimation.RBCspeciesParam[groupIndex][Maturity_Inflect_id]);

	fprintf(fid, " -3 3 %f %f 0 10 -3 0 0 0 0 0.5 0 0 # Mat_slope_Fem\n", bm->RBCestimation.RBCspeciesParam[groupIndex][Maturity_Slope_id],
			bm->RBCestimation.RBCspeciesParam[groupIndex][Maturity_Slope_id]);

	fprintf(fid, " -3 3 1 1 0 10 -3 0 0 0 0 0.5 0 0 # Eggs/kg_inter_Fem\n");
	fprintf(fid, " -3 3 0 0 0 10 -3 0 0 0 0 0.5 0 0 # Eggs/kg_slope_wt_Fem\n");

	if (Nsexes > 1) {
		fprintf(fid, " -3 3 %f %f  0 10 -3 0 0 0 0 0.5 0 0 # Wtlen_1_Fem\n", bm->RBCestimation.RBCspeciesArray[groupIndex].Wtlen_a[0][FEMALE],
				bm->RBCestimation.RBCspeciesArray[groupIndex].Wtlen_a[0][FEMALE]);

		fprintf(fid, " 0 6 %f %f  0 10 -3 0 0 0 0 0.5 0 0 # Wtlen_2_Fem\n", bm->RBCestimation.RBCspeciesArray[groupIndex].Wtlen_b[0][FEMALE],
				bm->RBCestimation.RBCspeciesArray[groupIndex].Wtlen_b[0][FEMALE]);

	}

	fprintf(fid, " 0 0 0 0 -1 0 -3 0 0 0 0 0 0 0 # RecrDist_GP_1\n");
	fprintf(fid, " 0 0 0 0 -1 0 -3 0 0 0 0 0 0 0 # RecrDist_Area_1\n");
	fprintf(fid, " 0 0 0 0 -1 0 -3 0 0 0 0 0 0 0 # RecrDist_Seas_1\n");
	fprintf(fid, " 1 1 1 1 -1 0 -3 0 0 0 0 0 0 0 # CohortGrowDev\n");

	fprintf(fid, "#\n");
	fprintf(fid, "#_Cond 0  #custom_MG-env_setup (0/1)\n");
	fprintf(fid, "#_Cond -2 2 0 0 -1 99 -2 #_placeholder when no MG-environ parameters\n");

	fprintf(fid, "#\n");
	fprintf(fid, "#_Cond 0  #custom_MG-block_setup (0/1)\n");
	fprintf(fid, "#_Cond -2 2 0 0 -1 99 -2 #_placeholder when no MG-block parameters\n");
	fprintf(fid, "#_Cond No MG parm trends \n");
	fprintf(fid, "#\n");
	fprintf(fid, "#_seasonal_effects_on_biology_parms\n");
	fprintf(fid, " 0 0 0 0 0 0 0 0 0 0 #_femwtlen1,femwtlen2,mat1,mat2,fec1,fec2,Malewtlen1,malewtlen2,L1,K\n");
	fprintf(fid, "#_Cond -2 2 0 0 -1 99 -2 #_placeholder when no seasonal MG parameters\n");
	fprintf(fid, "#\n");
	fprintf(fid, "#_Cond -4 #_MGparm_Dev_Phase\n");
	fprintf(fid, "#\n");

	fprintf(fid, "#_Spawner-Recruitment\n");
	if (recruit_sp == BevHolt_recruit) {

		fprintf(fid, "3 #_SR_function: 2=Ricker; 3=std_B-H; 4=SCAA; 5=Hockey; 6=B-H_flattop; 7=survival_3Parm\n");
		fprintf(fid, "#_LO HI INIT PRIOR PR_type SD PHASE\n");
		fprintf(fid, " 3 31 9.5 9.3 0 10 1 # SR_LN(R0)\n");

		fprintf(fid, " 0.2 1 %f 0.7 0 0.2 %f  1 # SR_BH_steep\n", bm->RBCestimation.RBCspeciesParam[groupIndex][Hsteep_id],
				bm->RBCestimation.RBCspeciesParam[groupIndex][T1_steep_phase_id]);

		fprintf(fid, " 0 2 %f %f 0 0.8 -2 # SR_sigmaR\n", bm->RBCestimation.RBCspeciesParam[groupIndex][SigmaR1_id],
				bm->RBCestimation.RBCspeciesParam[groupIndex][SigmaR1_id]);

		fprintf(fid, " -5 5 0.1 0 -1 1 -3 # SR_envlink\n");
		fprintf(fid, " -5 5 0 0 -1 1 -4 # SR_R1_offset\n");
		fprintf(fid, " 0 0 0 0 -1 0 -99 # SR_autocorr\n");

	} else {
		quit("recruit option not supported \n");
	}

	if (bm->RBCestimation.RBCspeciesParam[groupIndex][Regime_shift_assess_id])  //  for morwong regime shift
	{
		fprintf(fid, "1    # index of environmental variable");
		fprintf(fid, "2    # SR env target 0=1,1=devs,2=R0,3=steepness");
	} else {
		fprintf(fid, "0    # index of environmental variable");
		fprintf(fid, "0    # SR env target 0=1,1=devs,2=R0,3=steepness");
	}

	fprintf(fid, "0 #_SR_env_link\n");
	fprintf(fid, "0 #_SR_env_target_0=none;1=devs;_2=R0;_3=steepness\n");

	fprintf(fid, "1 #do_recdev:  0=none; 1=devvector; 2=simple deviations\n");

	fprintf(fid, "%d # first year of main recr_devs; early devs can preceed this era\n", (int) (bm->RBCestimation.RBCspeciesParam[groupIndex][RecDevMinYr_id]));
	fprintf(fid, "%d # last year of main recr_devs; forecast devs start in following year\n",
			maxyr - (int) bm->RBCestimation.RBCspeciesParam[groupIndex][RecDevBack_id]);

	fprintf(fid, "3 #_recdev phase \n");
	fprintf(fid, "1 # (0/1) to read 13 advanced options\n");
	fprintf(fid, " 0 #_recdev_early_start (0=none; neg value makes relative to recdev_start)\n");
	fprintf(fid, " -4 #_recdev_early_phase\n");
	fprintf(fid, " 0 #_forecast_recruitment phase (incl. late recr) (0 value resets to maxphase+1)\n");
	fprintf(fid, " 1000 #_lambda for Fcast_recr_like occurring before endyr+1\n");
	fprintf(fid, " %d #_last_early_yr_nobias_adj_in_MPD\n", HistYrMin - bm->RBCestimation.SSnoBiasAdj);
	fprintf(fid, " %d #_first_yr_fullbias_adj_in_MPD\n", (int) bm->RBCestimation.RBCspeciesParam[groupIndex][RecDevMinYr_id]);

	fprintf(fid, " %d #last_yr_fullbias_adj_in_MPD\n", maxyr - (int) bm->RBCestimation.RBCspeciesParam[groupIndex][RecDevBack_id]);

	fprintf(fid, " %d #_first_recent_yr_nobias_adj_in_MPD\n", maxyr - (int) bm->RBCestimation.RBCspeciesParam[groupIndex][RecDevBack_id] + 1);
	fprintf(fid, " 1 #_max_bias_adj_in_MPD (-1 to override ramp and set biasadj=1.0 for all estimated recdevs)\n");
	fprintf(fid, " 0 #_period of cycles in recruitment (N parms read below)\n");

	fprintf(fid, " -5 #min rec_dev\n");
	fprintf(fid, " 5 #max rec_dev\n");
	fprintf(fid, " 0 #_read_recdevs\n");
	fprintf(fid, "#_end of advanced SR options\n");
	fprintf(fid, "#\n");

// Done/

	fprintf(fid, "#\n");
	fprintf(fid, "#Fishing Mortality info \n");
	fprintf(fid, "%e # F ballpark for tuning early phases\n", bm->RBCestimation.RBCspeciesParam[groupIndex][BallParkF_id]); //0.2
	fprintf(fid, "%d # F ballpark year (neg value to disable)\n", (int)(bm->RBCestimation.RBCspeciesParam[groupIndex][BallParkYr_id]));
	fprintf(fid, "3 # F_Method:  1=Pope; 2=instan. F; 3=hybrid (hybrid is recommended)\n");
	fprintf(fid, "4 # max F or harvest rate, depends on F_Method\n");
	fprintf(fid, "# no additional F input needed for Fmethod 1\n");

	fprintf(fid, "# if Fmethod=2; read overall start F value; overall phase; N detailed inputs to read\n");
	fprintf(fid, "# if Fmethod=3; read N iterations for tuning for Fmethod 3\n");
	fprintf(fid, "5  # N iterations for tuning F in hybrid method (recommend 3 to 7)\n");
	fprintf(fid, "#\n");
	fprintf(fid, "#_initial_F_parms\n");
	fprintf(fid, "#_LO HI INIT PRIOR PR_type SD PHASE\n");
	for (fisheryIndex = 0; fisheryIndex < bm->RBCestimation.RBCspeciesParam[groupIndex][NumFisheries_id]; fisheryIndex++) {
		fprintf(fid, " 0 1 0 0.01 0 99 -1 # InitF_1FISHERY%d\n", fisheryIndex + 1);
	}
	fprintf(fid, "#\n");

	fprintf(fid, "#_Q_setup\n");
	fprintf(fid,
			" # Q_type options:  <0=mirror, 0=float_nobiasadj, 1=float_biasadj, 2=parm_nobiasadj, 3=parm_w_random_dev, 4=parm_w_randwalk, 5=mean_unbiased_float_assign_to_parm\n");
	fprintf(fid, "#_for_env-var:_enter_index_of_the_env-var_to_be_linked\n");
	fprintf(fid, "#_Den-dep  env-var  extra_se  Q_type\n");

	for (fleetIndex = 0; fleetIndex < bm->RBCestimation.RBCspeciesParam[groupIndex][NumFisheries_id]; fleetIndex++) {
		fprintf(fid, " 0 0 0 0 # 1 FISHERY%d\n", fleetIndex + 1);
	}

	fprintf(fid, "\n#\n");

	//done.
//
//	fprintf(fid, "#_Cond 0 #_If q has random component, then 0=read one parm for each fleet with random q; 1=read a parm for each year of index\n");
//	fprintf(fid, "#_Q_parms(if_any)\n");
//	fprintf(fid, "# LO HI INIT PRIOR PR_type SD PHASE\n");
//	fprintf(fid, " 0 0.5 0 0.05 1 0 -4 # Q_extraSD_2_SURVEY1\n");
//	fprintf(fid, " -7 5 0.515263 0 -1 1 1 # Q_base_2_SURVEY1\n");
//	fprintf(fid, "\n\n");

	fprintf(fid, "#\n");
	fprintf(fid, "#_size_selex_types\n");

	fprintf(fid, "#discard_options:_0=none;_1=define_retention;_2=retention&mortality;_3=all_discarded_dead\n");
	fprintf(fid, "#_Pattern Discard Male Special\n");

	for (yearIndex = HistYrMin; yearIndex <= maxyr; yearIndex++)
		for (fleetIndex = 0; fleetIndex < bm->RBCestimation.RBCspeciesParam[groupIndex][NumFisheries_id]; fleetIndex++)
			if (bm->RBCestimation.RBCspeciesArray[groupIndex].DiscData[fleetIndex][allregion][yearIndex] > 0)
				dflt[fleetIndex] = 1;
			else
				dflt[fleetIndex] = 0;
	for (fleetIndex = 0; fleetIndex < bm->RBCestimation.RBCspeciesParam[groupIndex][NumFisheries_id]; fleetIndex++) {
		if (dflt[fleetIndex] > 0)
			disc = 1;
		else
			disc = 0;

		fprintf(fid, " 1 %d 0 0 # FISHERY%d\n", disc, fleetIndex + 1);

	}
//
//	for (fleetIndex = 0; fleetIndex < numfleets; fleetIndex++) {
//		fprintf(fid, " 1 0 0 0 # 1 FISHERY%d\n", fleetIndex + 1);
//	}
//	for (surveyIndex = 0; surveyIndex < numsurveys; surveyIndex++) {
//		fprintf(fid, " 1 0 0 0 # 2 SURVEY%d\n", surveyIndex + 1);
//	}

	fprintf(fid, "\n");
	fprintf(fid, "#\n");
	fprintf(fid, "#_age_selex_types\n");
	fprintf(fid, "#_Pattern ___ Male Special\n");

	for (fleetIndex = 0; fleetIndex < bm->RBCestimation.RBCspeciesParam[groupIndex][NumFisheries_id]; fleetIndex++) {
		fprintf(fid, " %d 0 0 0 # FISHERY %d\n", (int) (bm->RBCestimation.RBCspeciesParam[groupIndex][Agesel_Pattern_id]), fleetIndex + 1);
	}

	//Done
	fprintf(fid, "#Selectivity parameters\n");
	fprintf(fid, "#_LO HI INIT PRIOR PR_type SD PHASE env-var use_dev dev_minyr dev_maxyr dev_stddev Block Block_Fxn\n");

	for (fleetIndex = 0; fleetIndex < bm->RBCestimation.RBCspeciesParam[groupIndex][NumFisheries_id]; fleetIndex++) {

		fprintf(fid, " Fleet %d", fleetIndex + 1);

		sv = bm->RBCestimation.RBCspeciesArray[groupIndex].Start_SelInflect[fleetIndex];
		phase = 2;
		if (sv < 0) {
			sv = -sv;
			phase = -2;
		}
		fprintf(fid, " %f %f %f %f 0 99 %d 0 0 0 0 0 0 0 # inflection for logistic", sv / 2.0, sv * 2.0, sv, sv, phase);

		sv = bm->RBCestimation.RBCspeciesArray[groupIndex].Start_SelWidth[fleetIndex];
		phase = 3;
		if (sv < 0) {
			sv = -sv;
			phase = -3;
		}

		fprintf(fid, " %f %f %f %f 0 99 %d 0 0 0 0 0 0 0 # width for logistic", sv / 2.0, sv * 2.0, sv, sv, phase);

		if (dflt[fleetIndex] > 0) { // if discard data

			sv = bm->RBCestimation.RBCspeciesArray[groupIndex].Start_RetInflect[fleetIndex];
			phase = 3;
			if (sv < 0) {
				sv = -sv;
				phase = -3;
			}

			fprintf(fid, " %f %f %f %f 0 99 %d 0 0 0 0 0 0 0 #  inflection for logistic retention", sv / 2.0, sv * 2.0, sv, sv, phase);

			sv = bm->RBCestimation.RBCspeciesArray[groupIndex].Start_RetSlope[fleetIndex];
			phase = 4;
			if (sv < 0) {
				sv = -sv;
				phase = -4;
			}

			fprintf(fid, " 0.2 %f %f %f 0 99 %d 0 0 0 0 0 0 0 #  slope for logistic retention", sv * 2.0, sv, sv, phase);

			fprintf(fid, " 0.001 1 1.0 0.1 0 99 -3 0 0 0 0 0 0 0\n");

			fprintf(fid, " -10 10 0 1 0 99 -3 0 0 0 0 0 0 0\n");

		}
	}

	if (bm->RBCestimation.RBCspeciesParam[groupIndex][Agesel_Pattern_id] == 11) {

		fprintf(fid, "# Age parameters");

		for (fleetIndex = 0; fleetIndex < bm->RBCestimation.RBCspeciesParam[groupIndex][NumFisheries_id]; fleetIndex++) {

			fprintf(fid, "# Fleet %d\n", fleetIndex + 1);

			fprintf(fid, " 0 %d 0.1 0.1 0 99 -3 0 0 0 0 0 0 0 #min age", (int) bm->RBCestimation.RBCspeciesParam[groupIndex][AccumAge_id]);

			fprintf(fid, " 0 %d %d %d 0 99 -3 0 0 0 0 0 0 0 #max age", (int) bm->RBCestimation.RBCspeciesParam[groupIndex][AccumAge_id],
					(int) bm->RBCestimation.RBCspeciesParam[groupIndex][AccumAge_id], (int) bm->RBCestimation.RBCspeciesParam[groupIndex][AccumAge_id]);

		}
		fprintf(fid, "\n");
	}

	fprintf(fid, "#_Cond 0 #_custom_sel-env_setup (0/1) \n");
	fprintf(fid, "#_Cond -2 2 0 0 -1 99 -2 #_placeholder when no enviro fxns\n");
	fprintf(fid, "#_Cond 0 #_custom_sel-blk_setup (0/1) \n");
	fprintf(fid, "#_Cond -2 2 0 0 -1 99 -2 #_placeholder when no block usage\n");
	fprintf(fid, "#_Cond No selex parm trends \n");
	fprintf(fid, "#_Cond -4 # placeholder for selparm_Dev_Phase\n");
	fprintf(fid, "#_Cond 0 #_env/block/dev_adjust_method (1=standard; 2=logistic trans to keep in base parm bounds; 3=standard w/ no bound check)\n");
	fprintf(fid, "#\n");
	fprintf(fid, "# Tag loss and Tag reporting parameters go next\n");
	fprintf(fid, "0  # TG_custom:  0=no read; 1=read if tags exist\n");
	fprintf(fid, "#_Cond -6 6 1 1 2 0.01 -4 0 0 0 0 0 0 0  #_placeholder if no parameters\n");
	fprintf(fid, "#\n");
	fprintf(fid, "1 #_Variance_adjustments_to_input_values\n");

	// done.

	for (fleetIndex = 0; fleetIndex < bm->RBCestimation.RBCspeciesParam[groupIndex][NumFisheries_id]; fleetIndex++) {
		fprintf(fid, " %f", bm->RBCestimation.RBCspeciesArray[groupIndex].Varadj_CPUE[fleetIndex]);
	}
	fprintf(fid, "      # add to CPUE CV\n");

	for (fleetIndex = 0; fleetIndex < bm->RBCestimation.RBCspeciesParam[groupIndex][NumFisheries_id]; fleetIndex++) {
		fprintf(fid, " %f", bm->RBCestimation.RBCspeciesArray[groupIndex].Varadj_discard[fleetIndex]);
	}
	fprintf(fid, "      # add to discard stdev\n");
	for (fleetIndex = 0; fleetIndex < bm->RBCestimation.RBCspeciesParam[groupIndex][NumFisheries_id]; fleetIndex++) {
		fprintf(fid, " 0");
	}
	fprintf(fid, "      # add to mean bodywt CV\n");
	for (fleetIndex = 0; fleetIndex < bm->RBCestimation.RBCspeciesParam[groupIndex][NumFisheries_id]; fleetIndex++) {
		fprintf(fid, " %f", bm->RBCestimation.RBCspeciesArray[groupIndex].Varadj_length[fleetIndex]);
	}
	fprintf(fid, "      # mult by length comp\n");
	for (fleetIndex = 0; fleetIndex < bm->RBCestimation.RBCspeciesParam[groupIndex][NumFisheries_id]; fleetIndex++) {
		fprintf(fid, " %f", bm->RBCestimation.RBCspeciesArray[groupIndex].Varadj_age[fleetIndex]);
	}
	fprintf(fid, "      # mult by age comp\n");
	for (fleetIndex = 0; fleetIndex < bm->RBCestimation.RBCspeciesParam[groupIndex][NumFisheries_id]; fleetIndex++) {
		fprintf(fid, " 1");
	}
	fprintf(fid, "     # mult by mean size at age\n");

	fprintf(fid, "#\n");
	fprintf(fid, "4 #_maxlambdaphase\n");
	fprintf(fid, "1 #_sd_offset\n");
	fprintf(fid, "#\n");

   if (bm->RBCestimation.RBCspeciesParam[groupIndex][NumChangeLambda_id] > 0) {

		fprintf(fid, "#Lambdas\n");
		fprintf(fid, "%d      #  number of changes to make to default lambdas\n", (int)(bm->RBCestimation.RBCspeciesParam[groupIndex][NumChangeLambda_id]));
        fprintf(fid,"# Like_comp codes:  1=surv; 2=disc; 3=mnwt; 4=length; 5=age; 6=SizeFreq; 7=sizeage; 8=catch;\n");
        fprintf(fid, "# 9=init_equ_catch; 10=recrdev; 11=parm_prior; 12=parm_dev; 13=CrashPen; 14=Morphcomp; 15=Tag-comp; 16=Tag-negbin\n");

		fprintf(fid, "#component  fleet phase lambda sizefreq_meth\n");
		for (fleetIndex = 0; fleetIndex < bm->RBCestimation.RBCspeciesParam[groupIndex][NumFisheries_id]; fleetIndex++) {
			fprintf(fid, " 4 %d 1 0.1 1\n", fleetIndex + 1);
		}
		for (fleetIndex = 0; fleetIndex < bm->RBCestimation.RBCspeciesParam[groupIndex][NumFisheries_id]; fleetIndex++) {
			fprintf(fid, " 5 %d 1 0.1 1\n", fleetIndex + 1);
		}
	} else {
		fprintf(fid, "0   #  number of changes to make to default lambdas\n");
	}

	fprintf(fid, "# lambdas (for info only; columns are phases)\n");
	fprintf(fid, "#  0 0 0 0 #_CPUE/survey:_1\n");
	fprintf(fid, "#  1 1 1 1 #_CPUE/survey:_2\n");
	fprintf(fid, "#  1 1 1 1 #_lencomp:_1\n");
	fprintf(fid, "#  1 1 1 1 #_lencomp:_2\n");
	fprintf(fid, "#  1 1 1 1 #_agecomp:_1\n");
	fprintf(fid, "#  1 1 1 1 #_agecomp:_2\n");
	fprintf(fid, "#  1 1 1 1 #_size-age:_1\n");
	fprintf(fid, "#  1 1 1 1 #_size-age:_2\n");
	fprintf(fid, "#  1 1 1 1 #_init_equ_catch\n");
	fprintf(fid, "#  1 1 1 1 #_recruitments\n");
	fprintf(fid, "#  1 1 1 1 #_parameter-priors\n");
	fprintf(fid, "#  1 1 1 1 #_parameter-dev-vectors\n");
	fprintf(fid, "#  1 1 1 1 #_crashPenLambda\n");
	fprintf(fid, "0 # (0/1) read specs for more stddev reporting \n");

	fprintf(fid, "\n");

	fprintf(fid, "999\n");
	free(dflt);

	fclose(fid);
}


//******************************************************************************
//      Original dat and control files for SS assess,ets
//
// Name:  WriteSSDat
// Description: write the generated data to data file for input to SS filename.dat
            //  Have data up to maxyr, need TAC for maxyr+2, so estimate catch for maxyr+1
//  This is for an assessment in maxyr+1
//
//
// called by : WriteSSFiles
// calls :
// created  : Nov 2007 Sally
// updated:   Feb 2011 Sally
//
//******************************************************************************
void Write_SS_Data_File_Orig(MSEBoxModel *bm, char *dirName, char *fileName, int maxyr, int groupIndex, int versionID) {
    FILE *fid;
    char str[STRLEN];
    char tempStr[STRLEN];
    char line1[STRLEN], line2[STRLEN], line3[STRLEN];
    int seasonIndex, yearIndex, fleetIndex;
    int index;
    double value;
    int nseas = 1;
    int nAgeBins;
    int Nsexes = bm->K_num_sexes;
    int AccumAge = (int)FunctGroupArray[groupIndex].speciesParams[AccumAge_id];  // TODO: Or should this be bm->RBCestimation.RBCspeciesParam[groupIndex][AccumAge_id] ?
    int HistYrMin;
    int numYears, allregion, nf, num, it, s, part = 0, gender, l, ageIndex;
    double cpcv, disccv;
    char *emptySex;
    double Nsex_samp = 1;
    int emptySexSize;

    if (bm->RBCestimation.RBCspeciesArray[groupIndex].CurrentYear == (bm->RBCestimation.RBCspeciesParam[groupIndex][HistYrMax_id] - 1))
        HistYrMin = (int)bm->RBCestimation.RBCspeciesParam[groupIndex][HistYrMin_id];
        //HistYrMin = 0;
    else
        HistYrMin = (int)bm->RBCestimation.RBCspeciesArray[groupIndex].CurrentYear + 1 - (int)bm->RBCestimation.RBCspeciesParam[groupIndex][HistYrMin_id];

    printf("maxyr = %d\n", maxyr);
    numYears = maxyr + 1;    // - HistYrMin + 1;

    printf("bm->RBCestimation.RBCspeciesParam[groupIndex][HistYrMax_id] = %d\n", (int) bm->RBCestimation.RBCspeciesParam[groupIndex][HistYrMax_id]);
    printf("bm->RBCestimation.RBCspeciesArray[groupIndex].CurrentYear = %d\n", bm->RBCestimation.RBCspeciesArray[groupIndex].CurrentYear);
    printf("numYears = %d\n", numYears);

    //sscanf(bm->t_units, "seconds since %d-%s", &startYear, str);

    snprintf(str, sizeof(str), "%s%s%s", dirName, FOLDER_SEP, fileName);

    if ((fid = fopen(str, "w")) == NULL)
        quit("Write_SS_Data_File_Orig: Can't open %s\n", str);

    /* Need the following data:
     *
     * bm->CatchRecord[Yr][groupIndex][age][dataid]
     *
     */
    
    fprintf(fid, "#V3.24f\n#_SS-V3.24f-safe-Win64;_08/03/2012;_Stock_Synthesis_by_Richard_Methot_(NOAA)_using_ADMB_11\n");
    fprintf(fid, "#_Start_time: Fri Aug 03 16:47:29 2012\n#_Number_of_datafiles: 3\n#C data file for simple example\n#_observed data: \n");
    fprintf(fid, "%d #_styr\n", HistYrMin);    // This has to have an AD year value (e.g. 1980)
    fprintf(fid, "%d #_endyr\n", maxyr);
    fprintf(fid, "%d #_nseas\n", nseas);
    fprintf(fid, "12 #_months/season\n");
    fprintf(fid, "1 #_spawn_seas\n");
    fprintf(fid, "%d #_Nfleet\n", (int) bm->RBCestimation.RBCspeciesParam[groupIndex][NumFisheries_id]);
    fprintf(fid, "0 #_Nsurveys\n");
    fprintf(fid, "1 #_N_areas\n");

    strcpy(line1, "");
    strcpy(line2, "");
    strcpy(line3, "");

    for (fleetIndex = 0; fleetIndex < bm->RBCestimation.RBCspeciesParam[groupIndex][NumFisheries_id]; fleetIndex++) {
        snprintf(tempStr, sizeof(tempStr), "FISHERY%d", fleetIndex + 1);
        strcat(line1, tempStr);
        //snprintf(line1, sizeof(line1), "%sFISHERY%d", line1, fleetIndex + 1);
        strcat(line2, "0.5 ");
        //snprintf(line2, sizeof(line2), "%s0.5 ", line2);
        strcat(line3, "1 ");

        //snprintf(line3, sizeof(line3), "%s1 ", line3);

        if (fleetIndex < (bm->RBCestimation.RBCspeciesParam[groupIndex][NumFisheries_id] - 1)){
            //snprintf(line1, sizeof(line1), "%s%%", line1);
            strcat(line1, "%%");
        }
    }

    fprintf(fid, "%s\n", line1);
    fprintf(fid, "%s #_surveytiming_in_season\n", line2);
    fprintf(fid, "%s #_area_assignments_for_each_fishery_and_survey\n", line3);

    strcpy(line1, "");
    strcpy(line2, "");

    for (fleetIndex = 0; fleetIndex < bm->RBCestimation.RBCspeciesParam[groupIndex][NumFisheries_id]; fleetIndex++) {
        //snprintf(line1, sizeof(line1), "%s 1", line1);
        //snprintf(line2, sizeof(line2), "%s 1", line2);
        strcat(line1, " 1");
        strcat(line2, " 1");
    }
    fprintf(fid, "%s #_units of catch: 1=bio; 2=num\n", line1);
    fprintf(fid, "%s #_se of log(catch) only used for init_eq_catch and for Fmethod 2 and 3; use -1 for discard only fleets\n", line2);

    fprintf(fid, "%d #_Ngenders\n", Nsexes);
    fprintf(fid, "%d #_Nages\n", AccumAge);
    
    for (fleetIndex = 0; fleetIndex < bm->RBCestimation.RBCspeciesParam[groupIndex][NumFisheries_id]; fleetIndex++) {
        snprintf(line1, sizeof(line1), "0 ");
    }
    fprintf(fid, "#_init_equil_catch_for_each_fishery\n");

    fprintf(fid, "%d #_N_lines_of_catch_to_read\n", numYears);

    fprintf(fid, "#_catch_biomass(mtons):_columns_are_fisheries,year,season\n");

    seasonIndex = 1;
    allregion = 0;    // uses weighted sum over regions, assume assessment doesn't know about regions

    /* Biomass data
     bm->CatchRecord[Yr][groupIndex][chrt][survey_id]

     Catch data
     bm->CatchRecord[Yr][groupIndex][chrt][commerical_id]
     */

    //bm->CatchRecord[Yr][groupIndex][age][dataid]
    for (yearIndex = 0; yearIndex < numYears; yearIndex++) {
        for (nf = 0; nf < bm->RBCestimation.RBCspeciesParam[groupIndex][NumFisheries_id]; nf++) {

            fprintf(fid, " %.1f", bm->RBCestimation.RBCspeciesArray[groupIndex].CatchData[nf][allregion][yearIndex]);
        }

        fprintf(fid, " %d %d\n", ((int) bm->RBCestimation.RBCspeciesParam[groupIndex][HistYrMin_id] + yearIndex), seasonIndex);
    }

    fprintf(fid, "\n#\n");

    num = 0;
    for (nf = 0; nf < (int) bm->RBCestimation.RBCspeciesParam[groupIndex][NumFisheries_id]; nf++) {
        for (yearIndex = 0; yearIndex < numYears; yearIndex++) {
            if (bm->RBCestimation.RBCspeciesArray[groupIndex].CPUEgen[nf][allregion][yearIndex] > 0.0) {
                    num = num + 1;
            }
        }
    }

    fprintf(fid, "%d #_N_cpue_and_surveyabundance_observations\n", num);
    fprintf(fid, "#_Units: 0=numbers; 1=biomass; 2=F\n");
    fprintf(fid, "#_Errtype: -1=normal; 0=lognormal; >0=T\n");

    fprintf(fid, "#_Fleet Units Errtype\n");

    index = 1;

    for (fleetIndex = 0; fleetIndex < bm->RBCestimation.RBCspeciesParam[groupIndex][NumFisheries_id]; fleetIndex++) {
        fprintf(fid, " %d %d %d # FISHERY%d\n", index, 1, 0, fleetIndex + 1);
        index++;
    }

    fprintf(fid, "##_year seas index obs err\n");

    /*
     * #_year seas index obs err
     1977 1 2 339689 0.3 # SURVEY1
     1980 1 2 193353 0.3 # SURVEY1
     1983 1 2 151984 0.3 # SURVEY1

     bm->CatchRecord[Yr][groupIndex][age][dataid]
     */

    index = 1;
    for (nf = 0; nf < (int) bm->RBCestimation.RBCspeciesParam[groupIndex][NumFisheries_id]; nf++) {
        for (yearIndex = 0; yearIndex < numYears; yearIndex++) {
            if (bm->RBCestimation.RBCspeciesArray[groupIndex].CPUEgen[nf][allregion][yearIndex] > 0.0) {
                    if (bm->RBCestimation.RBCspeciesArray[groupIndex].CPUEcv[nf] == 0.0) // generate cpue with no error, but still want cv in SS
                        cpcv = 0.2;
                    else
                        cpcv = bm->RBCestimation.RBCspeciesArray[groupIndex].CPUEcv[nf];

                    fprintf(fid, " %d %d %d %.2f %.2f #FISHERY%d\n", ((int) bm->RBCestimation.RBCspeciesParam[groupIndex][HistYrMin_id] + yearIndex), seasonIndex, index, bm->RBCestimation.RBCspeciesArray[groupIndex].CPUEgen[nf][allregion][yearIndex], cpcv, (fleetIndex + 1));

                }
            }
            index++;
        }
        fprintf(fid, "\n");

    num = 0;
    for (yearIndex = 0; yearIndex < numYears; yearIndex++) {
        for (fleetIndex = 0; fleetIndex < bm->RBCestimation.RBCspeciesParam[groupIndex][NumFisheries_id]; fleetIndex++) {
            if (bm->RBCestimation.RBCspeciesArray[groupIndex].DiscData[fleetIndex][allregion][yearIndex] > 0) {
                            num = num + 1;
            }
        }
    }

    fprintf(fid, "%d #_N_fleets_with_discard\n", (int) bm->RBCestimation.RBCspeciesParam[groupIndex][NumFisheries_id]);
    fprintf(fid, "#_discard_units (1=same_as_catchunits(bio/num); 2=fraction; 3=numbers)\n");
    fprintf(fid, "#_discard_errtype: >0 for DF of T-dist(read CV below); 0 for normal with CV; -1 for normal with se; -2 for lognormal\n");

    fprintf(fid, "#Fleet Disc_units err_type\n");
    for (fleetIndex = 0; fleetIndex < bm->RBCestimation.RBCspeciesParam[groupIndex][NumFisheries_id]; fleetIndex++) {
         fprintf(fid, " %d %d %d # FISHERY%d\n", index, 1, 0, fleetIndex + 1);
        index++;
    }
    
    fprintf(fid, "%d #N discard obs\n", num);
    fprintf(fid, "#_year seas index obs err\n");

    for (fleetIndex = 0; fleetIndex < bm->RBCestimation.RBCspeciesParam[groupIndex][NumFisheries_id]; fleetIndex++) {
        for (yearIndex = 0; yearIndex < numYears; yearIndex++) {
            if (bm->RBCestimation.RBCspeciesArray[groupIndex].DiscData[fleetIndex][allregion][yearIndex] > 0) {

                if (bm->RBCestimation.RBCspeciesArray[groupIndex].DiscCV[fleetIndex] == 0.0) // generate discards with no error, but still want cv in SS
                    disccv = 0.1;
                else
                    disccv = bm->RBCestimation.RBCspeciesArray[groupIndex].DiscCV[fleetIndex];

                fprintf(fid, " %d %d %d %.5f %.2f #FISHERY%d\n", ((int) bm->RBCestimation.RBCspeciesParam[groupIndex][HistYrMin_id] + yearIndex), seasonIndex, index, bm->RBCestimation.RBCspeciesArray[groupIndex].DiscData[fleetIndex][allregion][yearIndex], disccv, (fleetIndex + 1));

            }
        }
    }
    fprintf(fid, "#\n");

    fprintf(fid, "0 #_N_meanbodywt_obs\n");
    fprintf(fid, "30 #_DF_for_meanbodywt_T-distribution_like\n");

    emptySexSize = sizeof(char) * 2 * (size_t) bm->RBCestimation.RBCspeciesParam[groupIndex][Nlen_id];
    emptySex = (char *) malloc(emptySexSize);
    //strcpy(emptySex, "");

    for (index = 0; index < (int) bm->RBCestimation.RBCspeciesParam[groupIndex][Nlen_id]; index++) {
        if (index == 0){
            snprintf(emptySex, emptySexSize, "0");
        } else {
            //snprintf(emptySex, emptySexSize, "%s 0", emptySex);
            strcat(emptySex, " 0");
        }
    }

    fprintf(fid, "\n");
    fprintf(fid, "1 # length bin method: 1=use databins; 2=generate from binwidth,min,max below; 3=read vector\n");
    fprintf(fid, "%d # binwidth for population size comp \n", (int) FunctGroupArray[groupIndex].speciesParams[allometic_bin_size_id]);

    // May not be required - not in flathead example file from Wayte
    fprintf(fid, "%d # minimum size in the population (lower edge of first bin and size at age 0.00) \n", (int) bm->RBCestimation.RBCspeciesArray[groupIndex].LoLenBin[0]);
    // May not be required - not in flathead example file from Wayte
    fprintf(fid, "%d # maximum size in the population (lower edge of last bin) \n", (int) (bm->RBCestimation.RBCspeciesArray[groupIndex].LoLenBin[(int) bm->RBCestimation.RBCspeciesParam[groupIndex][Nlen_id] - 1]));

    fprintf(fid, "\n");
    fprintf(fid, "0 #_comp_tail_compression\n");
    fprintf(fid, "0.0001 #_add_to_comp\n");
    fprintf(fid, "0 #_combine males into females at or below this bin number\n");
    fprintf(fid, "%d #_N_LengthBins\n", (int) (bm->RBCestimation.RBCspeciesParam[groupIndex][Nlen_id]));

    index = 0;
    for (index = 0; index < bm->RBCestimation.RBCspeciesParam[groupIndex][Nlen_id]; index++) {
        fprintf(fid, " %d", (int) bm->RBCestimation.RBCspeciesArray[groupIndex].LoLenBin[index]);
    }
    fprintf(fid, " \n");

    it = 0;
    printf("retained\n");
    for (yearIndex = 0; yearIndex < numYears; yearIndex++) {
        printf("%d ", ((int) bm->RBCestimation.RBCspeciesParam[groupIndex][HistYrMin_id] + yearIndex));
        for (fleetIndex = 0; fleetIndex < (int) bm->RBCestimation.RBCspeciesParam[groupIndex][NumFisheries_id]; fleetIndex++) {
            printf(" %d", bm->RBCestimation.RBCspeciesArray[groupIndex].LengthFltYr[it][fleetIndex][yearIndex]);
        }
        printf("\n");
    }

    it = 2;
    printf("discarded\n");
    for (yearIndex = 0; yearIndex < numYears; yearIndex++) {
        printf("%d ", ((int) bm->RBCestimation.RBCspeciesParam[groupIndex][HistYrMin_id] + yearIndex));
        for (fleetIndex = 0; fleetIndex < (int) bm->RBCestimation.RBCspeciesParam[groupIndex][NumFisheries_id]; fleetIndex++) {
            printf(" %d", bm->RBCestimation.RBCspeciesArray[groupIndex].LengthFltYr[it][fleetIndex][yearIndex]);
        }
        printf("\n");

    }

    for (yearIndex = 0; yearIndex < numYears; yearIndex++) {
        for (fleetIndex = 0; fleetIndex < (int) bm->RBCestimation.RBCspeciesParam[groupIndex][NumFisheries_id]; fleetIndex++) {
            for (it = 0; it < 3; it++) {
                for (s = 0; s < Nsex_samp; s++) {
                    if ((bm->RBCestimation.RBCspeciesArray[groupIndex].LengthFltYr[it][fleetIndex][yearIndex] > 0) && (bm->RBCestimation.RBCspeciesArray[groupIndex].LFss[fleetIndex][s][yearIndex][it] > 10)) {
                        num = num + 1;
                    }
                }
            }
        }
    }

    fprintf(fid, "%d #_N_Length_obs\n", num);
    fprintf(fid, "#Yr Seas Flt/Svy Gender Part Nsamp datavector(female-male)\n");

    for (fleetIndex = 0; fleetIndex < (int) bm->RBCestimation.RBCspeciesParam[groupIndex][NumFisheries_id]; fleetIndex++) {
        for (yearIndex = 0; yearIndex < numYears; yearIndex++) {
            for (it = 0; it < 3; it++) {
                for (s = 0; s < Nsex_samp; s++) {
                    if ((bm->RBCestimation.RBCspeciesArray[groupIndex].LengthFltYr[it][fleetIndex][yearIndex] > 0) && (bm->RBCestimation.RBCspeciesArray[groupIndex].LFss[fleetIndex][s][yearIndex][it] > 10)) {
                        if (it == 0)
                            part = 2;   // retained
                        else if (it == 1)
                            part = 0;   // whole
                        else if (it == 2)
                            part = 1;   // discarded
                        if (Nsex_samp == 1)
                            gender = 0;   //combined
                        else
                            gender = s;

                        fprintf(fid, " %d 1 %d %d %d %d", (HistYrMin + yearIndex), fleetIndex + 1, gender, part,
                                bm->RBCestimation.RBCspeciesArray[groupIndex].LFss[fleetIndex][s][yearIndex][it]);

                        if (s == 1) {   // females or combined - write female LFs then male (ignored)

                            for (l = 0; l < bm->RBCestimation.RBCspeciesParam[groupIndex][Nlen_id]; l++)
                                fprintf(fid, " %d", bm->RBCestimation.RBCspeciesArray[groupIndex].LenComp[fleetIndex][s][yearIndex][it][l]);

                            //for (l = 0; l < bm->RBCestimation.RBCspeciesParam[groupIndex][Nlen_id]; l++)
                            fprintf(fid, " %s", emptySex);
                        } else if (s == 2) {    // males - write female lfs (ignored) then male

                            //for (l = 0; l < bm->RBCestimation.RBCspeciesParam[groupIndex][Nlen_id]; l++)
                            fprintf(fid, " %s", emptySex);
                            for (l = 0; l < bm->RBCestimation.RBCspeciesParam[groupIndex][Nlen_id]; l++)
                                fprintf(fid, " %d", bm->RBCestimation.RBCspeciesArray[groupIndex].LenComp[fleetIndex][s][yearIndex][it][l]);
                        }
                        fprintf(fid, " \n");
                    }
                }
            }

        }
    }

    fprintf(fid, "\n");

    free(emptySex);
    
    emptySexSize = sizeof(char) * 2 * (size_t) bm->RBCestimation.RBCspeciesParam[groupIndex][MaxAge_id];
    emptySex = (char *) malloc(emptySexSize);
    strcpy(emptySex, "");

    for (index = 0; index < (int) bm->RBCestimation.RBCspeciesParam[groupIndex][MaxAge_id]; index++) {
        if (index == 0){
            snprintf(emptySex, emptySexSize, "0");
        } else {
            //snprintf(emptySex, emptySexSize, "%s 0", emptySex);
            strcat(emptySex, " 0");

        }
    }

    /* Need high resolution age sampling data */
    nAgeBins = (int)bm->RBCestimation.RBCspeciesParam[groupIndex][MaxAge_id];

    fprintf(fid, "%d #_N_age_bins\n", (int) (bm->RBCestimation.RBCspeciesParam[groupIndex][MaxAge_id]));

    for (ageIndex = 0; ageIndex < nAgeBins; ageIndex++) {
        fprintf(fid, " %d", ageIndex + 1);
    }
    fprintf(fid, "\n");

    fprintf(fid, "1 #_N_ageerror_definitions\n");
    for (index = 0; index < bm->RBCestimation.RBCspeciesParam[groupIndex][AccumAge_id]; index++) {
        value = index + 0.5;
        fprintf(fid, " %2.1f", value);

    }
    fprintf(fid, "\n");

    //fprintf(fid, "1 #_N_ageerror_definitions\n");
    for (index = 0; index < bm->RBCestimation.RBCspeciesParam[groupIndex][AccumAge_id]; index++) {
        fprintf(fid, " %2.1f", bm->RBCestimation.RBCspeciesArray[groupIndex].Ageing_error[index]);

    }
    fprintf(fid, "\n");

    for (yearIndex = 0; yearIndex < numYears; yearIndex++) {
        for (fleetIndex = 0; fleetIndex < bm->RBCestimation.RBCspeciesParam[groupIndex][NumFisheries_id]; fleetIndex++) {
            for (it = 0; it < 3; it++) {
                for (s = 0; s < Nsex_samp; s++) {
                    if ((bm->RBCestimation.RBCspeciesArray[groupIndex].AgeFltYr[it][fleetIndex][yearIndex] > 0) && (bm->RBCestimation.RBCspeciesArray[groupIndex].AFss[fleetIndex][s][yearIndex][it] > 10))
                        num = num + 1;
                }
            }
        }
    }

    fprintf(fid, "\n%d #_N_Agecomp_obs\n", num);

    fprintf(fid, "1 #_Lbin_method: 1=poplenbins; 2=datalenbins; 3=lengths\n");
    fprintf(fid, "1 #_combine males into females at or below this bin number\n");
    fprintf(fid, "#Yr Seas Flt/Svy Gender Part Ageerr Lbin_lo Lbin_hi Nsamp datavector(female-male)\n");

    for (fleetIndex = 0; fleetIndex < bm->RBCestimation.RBCspeciesParam[groupIndex][NumFisheries_id]; fleetIndex++) {
        for (yearIndex = 0; yearIndex < numYears; yearIndex++) {
            for (it = 0; it < 3; it++) {
                for (s = 0; s < Nsex_samp; s++) {
                    if ((bm->RBCestimation.RBCspeciesArray[groupIndex].AgeFltYr[it][fleetIndex][yearIndex] > 0) && (bm->RBCestimation.RBCspeciesArray[groupIndex].AFss[fleetIndex][s][yearIndex][it] > 10)) {
                        if (it == 1)
                            part = 2;   // retained
                        else if (it == 2)
                            part = 0;   // whole
                        else if (it == 3)
                            part = 1;   // discarded
                        if (Nsex_samp == 1)
                            gender = 0;   //combined
                        else
                            gender = s;

                        fprintf(fid, " %d 1 %d %d %d 1 -1 1 %d", (HistYrMin + yearIndex), fleetIndex + 1, gender, part, bm->RBCestimation.RBCspeciesArray[groupIndex].AFss[fleetIndex][s][yearIndex][it]);

                        if (s == 1) {    // combined or females - write female LFs then male (ignored)

                            for (ageIndex = 0; ageIndex <= bm->RBCestimation.RBCspeciesParam[groupIndex][MaxAge_id]; ageIndex++)
                                fprintf(fid, " %d", bm->RBCestimation.RBCspeciesArray[groupIndex].AgeComp[fleetIndex][s][yearIndex][it][ageIndex]);

                            //for (ageIndex = 0; ageIndex <= bm->RBCestimation.RBCspeciesParam[groupIndex][MaxAge_id]; ageIndex++)
                            fprintf(fid, " %s", emptySex);
                        } else if (s == 2) {
                            // males - write female lfs (ignored) then male

                            //for (ageIndex = 0; ageIndex <= bm->RBCestimation.RBCspeciesParam[groupIndex][MaxAge_id]; ageIndex++)
                            fprintf(fid, " %s", emptySex);
                            for (ageIndex = 0; ageIndex <= bm->RBCestimation.RBCspeciesParam[groupIndex][MaxAge_id]; ageIndex++)
                                fprintf(fid, " %d", bm->RBCestimation.RBCspeciesArray[groupIndex].AgeComp[fleetIndex][s][yearIndex][it][ageIndex]);
                        }
                        fprintf(fid, "\n");
                    }
                }
            }
        }
    }

    fprintf(fid, "\n");

    fprintf(fid, "0 #_N_MeanSize-at-Age_obs\n");
    /*
     
     TODO: Create SizeRecord if N_MeanSize-at-Age_obs is ever non-zero
     
    fprintf(fid, "#Yr Seas Flt/Svy Gender Part Ageerr Ignore datavector(female-male)\n");
    fprintf(fid, "#                                          samplesize(female-male)\n");
    for (yearIndex = 0; yearIndex < numYears; yearIndex++) {
        fprintf(fid, " %d", (HistYrMin + yearIndex));
        fprintf(fid, " 1 1 0 0 1 999");
        for (index = 0; index < nAges; index++) {
            fprintf(fid, " %f", bm->SizeRecord[yearIndex][groupIndex][index]);
        }
        for (index = 0; index < nAges; index++) {
            fprintf(fid, " %d"bm->age_sample_size);
        }
        fprintf(fid, "\n");
    }
    fprintf(fid, "\n\n");
    */

    if (bm->RBCestimation.RBCspeciesParam[groupIndex][Regime_shift_assess_id] == 1)  //  for morwong regime shift
            {
        fprintf(fid, "1 #_N_environ_variables\n");

        fprintf(fid, "%d #_N_environ_obs\n", (int)(bm->RBCestimation.RBCspeciesParam[groupIndex][num_enviro_obs_id]));   // was 74
        for (index = 0; index < bm->RBCestimation.RBCspeciesParam[groupIndex][num_enviro_obs_id]; index++)
            fprintf(fid, " %d %e", index + 1, bm->RBCestimation.RBCspeciesArray[groupIndex].EnviroData[0][index]);  // was 1913 + ....  but now set relative to start of model and report only for the first region

    } else {
        fprintf(fid, "0 #_N_environ_variables\n");
        fprintf(fid, "0 #_N_environ_obs\n");
    }

    fprintf(fid, "0 #_N_environ_variables\n");
    fprintf(fid, "0 #_N_environ_obs\n");
    fprintf(fid, "0 # N sizefreq methods to read\n");   // If need this to be non-zero look to 3fish_3seas.dat for layout

    fprintf(fid, "0 # no tag data\n");

    fprintf(fid, "0 # no morphcomp data\n");

    fprintf(fid, "999\n");

    fclose(fid);

}
    
void Write_SS_Control_File_Orig(MSEBoxModel *bm, char *dirName, char *fileName, int maxyr, int groupIndex, int versionID) {

    FILE *fid;
    int disc, phase;
    int _Nblock_Patterns = 0;
    int fleetIndex, yearIndex;
    double mortality, lminf, lmaxf, vbk, cvest, hi, lo, mval, lminm, lmaxm, sv;
    char str[STRLEN];
    int startYear, fisheryIndex;
    int HistYrMin;
    //int Growthage_L1 = (int)(bm->RBCestimation.RBCspeciesParam[groupIndex][Growthage_L1_id]);
    double Nsexes = (double) (bm->K_num_sexes);
    int *dflt = malloc(sizeof(int) * (size_t)bm->RBCestimation.RBCspeciesParam[groupIndex][NumFisheries_id]);
    int allregion = 0;    // uses weighted sum over regions, assume assessment doesn't know about regions
    int recruit_sp;

    if (bm->RBCestimation.RBCspeciesArray[groupIndex].CurrentYear == (bm->RBCestimation.RBCspeciesParam[groupIndex][HistYrMax_id] - 1))
        HistYrMin = (int)bm->RBCestimation.RBCspeciesParam[groupIndex][HistYrMin_id];
        //HistYrMin = 0;
    else
        HistYrMin = (int)bm->RBCestimation.RBCspeciesArray[groupIndex].CurrentYear + 1 - (int)bm->RBCestimation.RBCspeciesParam[groupIndex][HistYrMin_id];

    sscanf(bm->t_units, "seconds since %d-%s", &startYear, str);

    recruit_sp = (int) (FunctGroupArray[groupIndex].speciesParams[flagrecruit_id]);

    snprintf(str, sizeof(str), "%s%s%s", dirName, FOLDER_SEP, fileName);

    printf("fileName = %s\n", str);

    if ((fid = fopen(str, "w")) == NULL)
        quit("Create_Control_File: Can't open %s\n", str);

    printf("versionID = %d\n", versionID);

    fprintf(fid, "#V3.24f\n");
    fprintf(fid, "#C growth parameters are estimated\n");
    fprintf(fid, "#C spawner-recruitment bias adjustment Not tuned For optimality\n");
    fprintf(fid, "#_data_and_control_files: simple.dat // simple.ctl\n");
    fprintf(fid, "#_SS-V3.24f-safe-Win64;_08/03/2012;_Stock_Synthesis_by_Richard_Methot_(NOAA)_using_ADMB_11\n");
    fprintf(fid, "%d #_N_Growth_Patterns\n", bm->RBCestimation.SSnumGrowthPatterns);
    fprintf(fid, "%d #_N_Morphs_Within_GrowthPattern\n", (int) (bm->RBCestimation.RBCspeciesParam[groupIndex][num_growth_morphs_id]));

    fprintf(fid, "#_Cond 1 #_Morph_between/within_stdev_ratio (no read if N_morphs=1)\n");
    fprintf(fid, "#_Cond  1 #vector_Morphdist_(-1_in_first_val_gives_normal_approx)\n");
    fprintf(fid, "#\n");

    fprintf(fid, "#_Cond 0  #  N recruitment designs goes here if N_GP*nseas*area>1\n");
    fprintf(fid, "#_Cond 0  #  placeholder for recruitment interaction request\n");
    fprintf(fid, "#_Cond 1 1 1  # example recruitment design element for GP=1, seas=1, area=1\n");
    fprintf(fid, "#\n");
    fprintf(fid, "#_Cond 0 # N_movement_definitions goes here if N_areas > 1\n");
    fprintf(fid, "#_Cond 1.0 # first age that moves (real age at begin of season, not integer) also cond on do_migration>0\n");
    fprintf(fid, "#_Cond 1 1 1 2 4 10 # example move definition for seas=1, morph=1, source=1 dest=2, age1=4, age2=10\n");
    fprintf(fid, "#\n");

    fprintf(fid, "%d #_Nblock_Patterns \n", _Nblock_Patterns);

    fprintf(fid, "#_Cond 0 #_blocks_per_pattern\n");
    fprintf(fid, "# begin and end years of blocks\n");
    fprintf(fid, "#\n");
    fprintf(fid, "%e #_fracfemale \n", (double)(1.0 / bm->K_num_sexes));

    fprintf(fid, "0 #_natM_type:_0=1Parm;1=N_breakpoints;_2=Lorenzen;_3=agespecific;_4=agespec_withseasinterpolate\n");
    fprintf(fid, "  #_no additional input for selected M option; read 1P per morph\n");
    fprintf(fid, "1 # GrowthModel: 1=vonBert with L1&L2; 2=Richards with L1&L2; 3=age_speciific_K; 4=not implemented\n");

    /* These should perhaps be read in from an input file */
    fprintf(fid, "%d #_Growth_Age_for_L1\n", (int) (bm->RBCestimation.RBCspeciesParam[groupIndex][Growthage_L1_id]));
    fprintf(fid, "%d #_Growth_Age_for_L2 (999 to use as Linf)\n", (int) (bm->RBCestimation.RBCspeciesParam[groupIndex][Growthage_L2_id]));
    fprintf(fid, "0 #_SD_add_to_LAA (set to 0.1 for SS2 V1.x compatibility)\n");
    fprintf(fid, "0 #_CV_Growth_Pattern:  0 CV=f(LAA); 1 CV=F(A); 2 SD=F(LAA); 3 SD=F(A); 4 logSD=F(A)\n");
    fprintf(fid, "1 #_maturity_option:  1=length logistic; 2=age logistic; 3=read age-maturity matrix by growth_pattern; 4=read age-fecundity; 5=read fec and wt from wtatage.ss\n");
    fprintf(fid, "#_placeholder for empirical age-maturity by growth pattern\n");
    fprintf(fid, "%d #_First_Mature_Age\n",(int)(FunctGroupArray[groupIndex].speciesParams[age_mat_id] * FunctGroupArray[groupIndex].ageClassSize));
    fprintf(fid, "1 #_fecundity option:(1)eggs=Wt*(a+b*Wt);(2)eggs=a*L^b;(3)eggs=a*Wt^b; (4)eggs=a+b*L; (5)eggs=a+b*W\n");
    fprintf(fid, "0 #_hermaphroditism option:  0=none; 1=age-specific fxn\n");
    fprintf(fid, "%d #_parameter_offset_approach (1=none, 2= M, G, CV_G as offset from female-GP1, 3=like SS2 V1.x)\n", (int) (bm->RBCestimation.RBCspeciesParam[groupIndex][MG_offset_id]));
    fprintf(fid, "1 #_env/block/dev_adjust_method (1=standard; 2=logistic transform keeps in base parm bounds; 3=standard w/ no bound check)\n");

    fprintf(fid, "#\n");
    fprintf(fid, "#_growth_parms\n");
    fprintf(fid, "#_LO HI INIT PRIOR PR_type SD PHASE env-var use_dev dev_minyr dev_maxyr dev_stddev Block Block_Fxn\n");

    mortality = bm->RBCestimation.RBCspeciesArray[groupIndex].SSMort[0][FEMALE];

    printf("HistYrMin = %d\n", HistYrMin);

    fprintf(fid, " %f %f %f %f 0 0.8 -3 0 0 0 0 0.5 0 0 # NatM_p_1_Fem_GP_1\n", mortality / 2.0, mortality * 2.0, mortality, mortality);

    lminf = bm->RBCestimation.RBCspeciesArray[groupIndex].VBLinf[0][FEMALE] * (1.0 - exp( -1.0 * bm->RBCestimation.RBCspeciesArray[groupIndex].VBk[0][FEMALE] * (bm->RBCestimation.RBCspeciesParam[groupIndex][Growthage_L1_id] - bm->RBCestimation.RBCspeciesArray->VBt0[0][FEMALE])));

    fprintf(fid, " %f %f %f %f 0 10 -3 0 0 0 0 0.5 0 0 # L_at_Amin_Fem_GP_1\n", lminf / 2.0, lminf * 2.0, lminf, lminf);

    if (bm->RBCestimation.RBCspeciesParam[groupIndex][Growthage_L2_id] < 999)
        lmaxf = bm->RBCestimation.RBCspeciesArray[groupIndex].VBLinf[0][FEMALE] * (1.0 - exp( -1.0 * bm->RBCestimation.RBCspeciesArray[groupIndex].VBk[0][FEMALE] * (bm->RBCestimation.RBCspeciesParam[groupIndex][Growthage_L2_id] - bm->RBCestimation.RBCspeciesArray->VBt0[0][FEMALE])));
    else
        lmaxf = bm->RBCestimation.RBCspeciesArray[groupIndex].VBLinf[0][FEMALE];     // use Linf

    fprintf(fid, " %f %f %f %f 0 10 -3 0 0 0 0 0.5 0 0 # L_at_Amax_Fem_GP_1\n", lmaxf / 2.0, lmaxf * 2.0, lmaxf, lmaxf);

    vbk = bm->RBCestimation.RBCspeciesArray[groupIndex].VBk[0][FEMALE];
    fprintf(fid, " %f %f %f %f 0 0.8 -3 0 0 0 0 0.5 0 0 # VonBert_K_Fem_GP_1\n", vbk / 3.0, vbk * 3.0, vbk, vbk);

    cvest = bm->RBCestimation.RBCspeciesArray[groupIndex].CvLA0[0][0];
    fprintf(fid, " %f %f %f %f 0 0.8 -2 0 0 0 0 0.5 0 0 # CV_young_Fem_GP_1\n", cvest / 3.0, cvest * 3.0, cvest, cvest);

    if (bm->RBCestimation.RBCspeciesParam[groupIndex][MG_offset_id] == 3) {
        cvest = log(fabs(bm->RBCestimation.RBCspeciesArray[groupIndex].CvLAmax[0][0]) / fabs(bm->RBCestimation.RBCspeciesArray[groupIndex].CvLA0[0][0]));
        lo = -1.0;
        hi = 1.0;
    } else {
        cvest = fabs(bm->RBCestimation.RBCspeciesArray[groupIndex].CvLAmax[0][0]);
        lo = cvest / 3.0;
        hi = cvest * 3.0;
    }

    fprintf(fid, " %f %f %f %f 0 0.8 -3 0 0 0 0 0.5 0 0 #CV_old_Fem_GP_1\n", lo, hi, cvest, cvest);

    if (Nsexes > 1) {
        if (bm->RBCestimation.RBCspeciesParam[groupIndex][MG_offset_id] == 3) {
            mval = log(fabs(bm->RBCestimation.RBCspeciesArray[groupIndex].SSMort[0][FEMALE]) / fabs(bm->RBCestimation.RBCspeciesArray[groupIndex].SSMort[0][FEMALE]));
            lo = -1.0;
            hi = 1.0;
        } else {
            mval = fabs(bm->RBCestimation.RBCspeciesArray[groupIndex].SSMort[0][FEMALE]);
            lo = mval / 2.0;
            hi = mval * 2.0;
        }

        fprintf(fid, " %f %f %f %f 0 0.8 -3 0 0 0 0 0.5 0 0 # NatM_p_1_Male_GP_1\n", lo, hi, mval, mval);

        if (bm->RBCestimation.RBCspeciesParam[groupIndex][MG_offset_id] == 3) {
            lminm = bm->RBCestimation.RBCspeciesArray[groupIndex].VBLinf[0][FEMALE] * (1.0 - exp( -1.0 * bm->RBCestimation.RBCspeciesArray[groupIndex].VBk[0][FEMALE] * (bm->RBCestimation.RBCspeciesParam[groupIndex][Growthage_L1_id] - bm->RBCestimation.RBCspeciesArray[groupIndex].VBt0[0][FEMALE])));
            lminm = log(lminm / lminf);
            //lmin = log(MeanLenAge[0][FEMALE][Growthage_L1])/MeanLenAge[0][FEMALE][Growthage_L1]);
            lo = -1.0;
            hi = 1.0;
        } else {
            lminm = bm->RBCestimation.RBCspeciesArray[groupIndex].VBLinf[0][FEMALE] * (1.0 - exp( -1.0 * bm->RBCestimation.RBCspeciesArray[groupIndex].VBk[0][FEMALE] * (bm->RBCestimation.RBCspeciesParam[groupIndex][Growthage_L1_id] - bm->RBCestimation.RBCspeciesArray[groupIndex].VBt0[0][FEMALE])));
            //lmin = MeanLenAge[0][FEMALE][Growthage_L1];
            lo = lminm / 2.0;
            hi = lminm * 2.0;
        }
        fprintf(fid, " %f %f %f %f 0 10 -3 0 0 0 0 0.5 0 0 # L_at_Amin_Male_GP_1\n", lo, hi, lminm, lminm);

        if (bm->RBCestimation.RBCspeciesParam[groupIndex][Growthage_L2_id] < 999)
            lmaxm = bm->RBCestimation.RBCspeciesArray[groupIndex].VBLinf[0][FEMALE] * (1.0 - exp(-1.0 * bm->RBCestimation.RBCspeciesArray[groupIndex].VBk[0][FEMALE] * (bm->RBCestimation.RBCspeciesParam[groupIndex][Growthage_L2_id] - bm->RBCestimation.RBCspeciesArray[groupIndex].VBt0[0][FEMALE])));
        else
            lmaxm = bm->RBCestimation.RBCspeciesArray[groupIndex].VBLinf[0][FEMALE];

        if (bm->RBCestimation.RBCspeciesParam[groupIndex][MG_offset_id] == 3) {
            lmaxm = log(lmaxm / lmaxf);
            //lmaxm = log(MeanLenAge[0][FEMALE][MaxAge][HistYrMin]/MeanLenAge[0][FEMALE][MaxAge][HistYrMin]);
            lo = -1.0;
            hi = 1.0;
        } else {
            //lmax = MeanLenAge[0][FEMALE][MaxAge][HistYrMin];
            lo = lmaxm / 2.0;
            hi = lmaxm * 2.0;
        }
        fprintf(fid, " %f %f %f %f 0 10 -3 0 0 0 0 0.5 0 0 # L_at_Amax_Male_GP_1\n", lo, hi, lmaxm, lmaxm);

        if (bm->RBCestimation.RBCspeciesParam[groupIndex][MG_offset_id] == 3) {
            vbk = log(bm->RBCestimation.RBCspeciesArray[groupIndex].VBk[0][FEMALE] / bm->RBCestimation.RBCspeciesArray[groupIndex].VBk[0][FEMALE]);
            lo = -1.0;
            hi = 1.0;
        } else {
            vbk = bm->RBCestimation.RBCspeciesArray[groupIndex].VBk[0][FEMALE];
            lo = vbk / 2.0;
            hi = vbk * 2.0;
        }

        fprintf(fid, " %f %f %f %f 0 0.8 -3 0 0 0 0 0.5 0 0 # VonBert_K_Male_GP_1\n", lo, hi, vbk, vbk);

        if (bm->RBCestimation.RBCspeciesParam[groupIndex][MG_offset_id] == 3) {
            cvest = log(bm->RBCestimation.RBCspeciesArray[groupIndex].CvLA0[0][1] / bm->RBCestimation.RBCspeciesArray[groupIndex].CvLA0[0][0]);
            lo = -1.0;
            hi = 1.0;
        } else {
            cvest = bm->RBCestimation.RBCspeciesArray[groupIndex].CvLA0[0][1];
            lo = cvest / 3.0;
            hi = cvest * 3.0;
        }

        fprintf(fid, " %f %f %f %f 0 0.8 -3 0 0 0 0 0.5 0 0 # CV_young_Male_GP_1\n", lo, hi, cvest, cvest);

        if (bm->RBCestimation.RBCspeciesParam[groupIndex][MG_offset_id] == 3) {
            cvest = log(bm->RBCestimation.RBCspeciesArray[groupIndex].CvLAmax[0][0] / bm->RBCestimation.RBCspeciesArray[groupIndex].CvLA0[0][0]);
            lo = -1.0;
            hi = 1.0;
        } else {
            cvest = bm->RBCestimation.RBCspeciesArray[groupIndex].CvLAmax[0][0];
            lo = cvest / 3.0;
            hi = cvest * 3.0;
        }
        fprintf(fid, " %f %f %f %f 0 0.8 -3 0 0 0 0 0.5 0 0 # CV_old_Male_GP_1\n", lo, hi, cvest, cvest);

    }

    fprintf(fid, "# wt-len and mat-len parameters\n");

    fprintf(fid, "#   LO HI INIT PRIOR PR_TYPE SD PHASE env-var use-dev dev_minyr dev_maxyr dev_stddev block block_fxn\n");

    fprintf(fid, " -3 3 %f %f 0 10 -3 0 0 0 0 0.5 0 0 # Wtlen_1_Fem\n", bm->RBCestimation.RBCspeciesArray[groupIndex].Wtlen_a[0][FEMALE], bm->RBCestimation.RBCspeciesArray[groupIndex].Wtlen_a[0][FEMALE]);

    fprintf(fid, " 0 6 %f %f 0 10 -3 0 0 0 0 0.5 0 0 # Wtlen_2_Fem\n", bm->RBCestimation.RBCspeciesArray[groupIndex].Wtlen_b[0][FEMALE], bm->RBCestimation.RBCspeciesArray[groupIndex].Wtlen_b[0][FEMALE]);

    fprintf(fid, " 0 6 %f %f 0 10 -3 0 0 0 0 0.5 0 0 # Wtlen_2_Fem\n", bm->RBCestimation.RBCspeciesArray[groupIndex].Wtlen_b[0][FEMALE], bm->RBCestimation.RBCspeciesArray[groupIndex].Wtlen_b[0][FEMALE]);

    fprintf(fid, " %f %f %f %f 0 10 -3 0 0 0 0 0.5 0 0 # Mat50_Fem\n", bm->RBCestimation.RBCspeciesParam[groupIndex][Maturity_Inflect_id] / 3.0, bm->RBCestimation.RBCspeciesParam[groupIndex][Maturity_Inflect_id] * 3.0, bm->RBCestimation.RBCspeciesParam[groupIndex][Maturity_Inflect_id], bm->RBCestimation.RBCspeciesParam[groupIndex][Maturity_Inflect_id]);

    fprintf(fid, " -3 3 %f %f 0 10 -3 0 0 0 0 0.5 0 0 # Mat_slope_Fem\n", bm->RBCestimation.RBCspeciesParam[groupIndex][Maturity_Slope_id], bm->RBCestimation.RBCspeciesParam[groupIndex][Maturity_Slope_id]);

    fprintf(fid, " -3 3 1 1 0 10 -3 0 0 0 0 0.5 0 0 # Eggs/kg_inter_Fem\n");
    fprintf(fid, " -3 3 0 0 0 10 -3 0 0 0 0 0.5 0 0 # Eggs/kg_slope_wt_Fem\n");

    if (Nsexes > 1) {
        fprintf(fid, " -3 3 %f %f  0 10 -3 0 0 0 0 0.5 0 0 # Wtlen_1_Fem\n", bm->RBCestimation.RBCspeciesArray[groupIndex].Wtlen_a[0][FEMALE], bm->RBCestimation.RBCspeciesArray[groupIndex].Wtlen_a[0][FEMALE]);

        fprintf(fid, " 0 6 %f %f  0 10 -3 0 0 0 0 0.5 0 0 # Wtlen_2_Fem\n", bm->RBCestimation.RBCspeciesArray[groupIndex].Wtlen_b[0][FEMALE], bm->RBCestimation.RBCspeciesArray[groupIndex].Wtlen_b[0][FEMALE]);

    }

    fprintf(fid, " 0 0 0 0 -1 0 -3 0 0 0 0 0 0 0 # RecrDist_GP_1\n");
    fprintf(fid, " 0 0 0 0 -1 0 -3 0 0 0 0 0 0 0 # RecrDist_Area_1\n");
    fprintf(fid, " 0 0 0 0 -1 0 -3 0 0 0 0 0 0 0 # RecrDist_Seas_1\n");
    fprintf(fid, " 1 1 1 1 -1 0 -3 0 0 0 0 0 0 0 # CohortGrowDev\n");

    fprintf(fid, "#\n");
    fprintf(fid, "#_Cond 0  #custom_MG-env_setup (0/1)\n");
    fprintf(fid, "#_Cond -2 2 0 0 -1 99 -2 #_placeholder when no MG-environ parameters\n");

    fprintf(fid, "#\n");
    fprintf(fid, "#_Cond 0  #custom_MG-block_setup (0/1)\n");
    fprintf(fid, "#_Cond -2 2 0 0 -1 99 -2 #_placeholder when no MG-block parameters\n");
    fprintf(fid, "#_Cond No MG parm trends \n");
    fprintf(fid, "#\n");
    fprintf(fid, "#_seasonal_effects_on_biology_parms\n");
    fprintf(fid, " 0 0 0 0 0 0 0 0 0 0 #_femwtlen1,femwtlen2,mat1,mat2,fec1,fec2,Malewtlen1,malewtlen2,L1,K\n");
    fprintf(fid, "#_Cond -2 2 0 0 -1 99 -2 #_placeholder when no seasonal MG parameters\n");
    fprintf(fid, "#\n");
    fprintf(fid, "#_Cond -4 #_MGparm_Dev_Phase\n");
    fprintf(fid, "#\n");

    fprintf(fid, "#_Spawner-Recruitment\n");
    if (recruit_sp == BevHolt_recruit) {

        fprintf(fid, "3 #_SR_function: 2=Ricker; 3=std_B-H; 4=SCAA; 5=Hockey; 6=B-H_flattop; 7=survival_3Parm\n");
        fprintf(fid, "#_LO HI INIT PRIOR PR_type SD PHASE\n");
        fprintf(fid, " 3 31 9.5 9.3 0 10 1 # SR_LN(R0)\n");

        fprintf(fid, " 0.2 1 %f 0.7 0 0.2 %f  1 # SR_BH_steep\n", bm->RBCestimation.RBCspeciesParam[groupIndex][Hsteep_id], bm->RBCestimation.RBCspeciesParam[groupIndex][T1_steep_phase_id]);

        fprintf(fid, " 0 2 %f %f 0 0.8 -2 # SR_sigmaR\n", bm->RBCestimation.RBCspeciesParam[groupIndex][SigmaR1_id], bm->RBCestimation.RBCspeciesParam[groupIndex][SigmaR1_id]);

        fprintf(fid, " -5 5 0.1 0 -1 1 -3 # SR_envlink\n");
        fprintf(fid, " -5 5 0 0 -1 1 -4 # SR_R1_offset\n");
        fprintf(fid, " 0 0 0 0 -1 0 -99 # SR_autocorr\n");

    } else {
        quit("recruit option not supported \n");
    }

    if (bm->RBCestimation.RBCspeciesParam[groupIndex][Regime_shift_assess_id]) {  //  for morwong regime shift
        fprintf(fid, "1    # index of environmental variable");
        fprintf(fid, "2    # SR env target 0=1,1=devs,2=R0,3=steepness");
    } else {
        fprintf(fid, "0    # index of environmental variable");
        fprintf(fid, "0    # SR env target 0=1,1=devs,2=R0,3=steepness");
    }

    fprintf(fid, "0 #_SR_env_link\n");
    fprintf(fid, "0 #_SR_env_target_0=none;1=devs;_2=R0;_3=steepness\n");

    fprintf(fid, "1 #do_recdev:  0=none; 1=devvector; 2=simple deviations\n");

    fprintf(fid, "%d # first year of main recr_devs; early devs can preceed this era\n", (int) (bm->RBCestimation.RBCspeciesParam[groupIndex][RecDevMinYr_id]));
    fprintf(fid, "%d # last year of main recr_devs; forecast devs start in following year\n",  maxyr - (int) bm->RBCestimation.RBCspeciesParam[groupIndex][RecDevBack_id]);

    fprintf(fid, "3 #_recdev phase \n");
    fprintf(fid, "1 # (0/1) to read 13 advanced options\n");
    fprintf(fid, " 0 #_recdev_early_start (0=none; neg value makes relative to recdev_start)\n");
    fprintf(fid, " -4 #_recdev_early_phase\n");
    fprintf(fid, " 0 #_forecast_recruitment phase (incl. late recr) (0 value resets to maxphase+1)\n");
    fprintf(fid, " 1000 #_lambda for Fcast_recr_like occurring before endyr+1\n");
    fprintf(fid, " %d #_last_early_yr_nobias_adj_in_MPD\n", HistYrMin - bm->RBCestimation.SSnoBiasAdj);
    fprintf(fid, " %d #_first_yr_fullbias_adj_in_MPD\n", (int) bm->RBCestimation.RBCspeciesParam[groupIndex][RecDevMinYr_id]);

    fprintf(fid, " %d #last_yr_fullbias_adj_in_MPD\n", maxyr - (int) bm->RBCestimation.RBCspeciesParam[groupIndex][RecDevBack_id]);

    fprintf(fid, " %d #_first_recent_yr_nobias_adj_in_MPD\n", maxyr - (int) bm->RBCestimation.RBCspeciesParam[groupIndex][RecDevBack_id] + 1);
    fprintf(fid, " 1 #_max_bias_adj_in_MPD (-1 to override ramp and set biasadj=1.0 for all estimated recdevs)\n");
    fprintf(fid, " 0 #_period of cycles in recruitment (N parms read below)\n");

    fprintf(fid, " -5 #min rec_dev\n");
    fprintf(fid, " 5 #max rec_dev\n");
    fprintf(fid, " 0 #_read_recdevs\n");
    fprintf(fid, "#_end of advanced SR options\n");
    fprintf(fid, "#\n");

// Done/

    fprintf(fid, "#\n");
    fprintf(fid, "#Fishing Mortality info \n");
    fprintf(fid, "%e # F ballpark for tuning early phases\n", bm->RBCestimation.RBCspeciesParam[groupIndex][BallParkF_id]); //0.2
    fprintf(fid, "%d # F ballpark year (neg value to disable)\n", (int)(bm->RBCestimation.RBCspeciesParam[groupIndex][BallParkYr_id]));
    fprintf(fid, "3 # F_Method:  1=Pope; 2=instan. F; 3=hybrid (hybrid is recommended)\n");
    fprintf(fid, "4 # max F or harvest rate, depends on F_Method\n");
    fprintf(fid, "# no additional F input needed for Fmethod 1\n");

    fprintf(fid, "# if Fmethod=2; read overall start F value; overall phase; N detailed inputs to read\n");
    fprintf(fid, "# if Fmethod=3; read N iterations for tuning for Fmethod 3\n");
    fprintf(fid, "5  # N iterations for tuning F in hybrid method (recommend 3 to 7)\n");
    fprintf(fid, "#\n");
    fprintf(fid, "#_initial_F_parms\n");
    fprintf(fid, "#_LO HI INIT PRIOR PR_type SD PHASE\n");
    for (fisheryIndex = 0; fisheryIndex < bm->RBCestimation.RBCspeciesParam[groupIndex][NumFisheries_id]; fisheryIndex++) {
        fprintf(fid, " 0 1 0 0.01 0 99 -1 # InitF_1FISHERY%d\n", fisheryIndex + 1);
    }
    fprintf(fid, "#\n");

    fprintf(fid, "#_Q_setup\n");
    fprintf(fid, " # Q_type options:  <0=mirror, 0=float_nobiasadj, 1=float_biasadj, 2=parm_nobiasadj, 3=parm_w_random_dev, 4=parm_w_randwalk, 5=mean_unbiased_float_assign_to_parm\n");
    fprintf(fid, "#_for_env-var:_enter_index_of_the_env-var_to_be_linked\n");
    fprintf(fid, "#_Den-dep  env-var  extra_se  Q_type\n");

    for (fleetIndex = 0; fleetIndex < bm->RBCestimation.RBCspeciesParam[groupIndex][NumFisheries_id]; fleetIndex++) {
        fprintf(fid, " 0 0 0 0 # 1 FISHERY%d\n", fleetIndex + 1);
    }

    fprintf(fid, "\n#\n");

    //done.
//
//    fprintf(fid, "#_Cond 0 #_If q has random component, then 0=read one parm for each fleet with random q; 1=read a parm for each year of index\n");
//    fprintf(fid, "#_Q_parms(if_any)\n");
//    fprintf(fid, "# LO HI INIT PRIOR PR_type SD PHASE\n");
//    fprintf(fid, " 0 0.5 0 0.05 1 0 -4 # Q_extraSD_2_SURVEY1\n");
//    fprintf(fid, " -7 5 0.515263 0 -1 1 1 # Q_base_2_SURVEY1\n");
//    fprintf(fid, "\n\n");

    fprintf(fid, "#\n");
    fprintf(fid, "#_size_selex_types\n");

    fprintf(fid, "#discard_options:_0=none;_1=define_retention;_2=retention&mortality;_3=all_discarded_dead\n");
    fprintf(fid, "#_Pattern Discard Male Special\n");

    for (yearIndex = HistYrMin; yearIndex <= maxyr; yearIndex++)
        for (fleetIndex = 0; fleetIndex < bm->RBCestimation.RBCspeciesParam[groupIndex][NumFisheries_id]; fleetIndex++)
            if (bm->RBCestimation.RBCspeciesArray[groupIndex].DiscData[fleetIndex][allregion][yearIndex] > 0)
                dflt[fleetIndex] = 1;
            else
                dflt[fleetIndex] = 0;
    for (fleetIndex = 0; fleetIndex < bm->RBCestimation.RBCspeciesParam[groupIndex][NumFisheries_id]; fleetIndex++) {
        if (dflt[fleetIndex] > 0)
            disc = 1;
        else
            disc = 0;

        fprintf(fid, " 1 %d 0 0 # FISHERY%d\n", disc, fleetIndex + 1);

    }
//
//    for (fleetIndex = 0; fleetIndex < numfleets; fleetIndex++) {
//        fprintf(fid, " 1 0 0 0 # 1 FISHERY%d\n", fleetIndex + 1);
//    }
//    for (surveyIndex = 0; surveyIndex < numsurveys; surveyIndex++) {
//        fprintf(fid, " 1 0 0 0 # 2 SURVEY%d\n", surveyIndex + 1);
//    }

    fprintf(fid, "\n");
    fprintf(fid, "#\n");
    fprintf(fid, "#_age_selex_types\n");
    fprintf(fid, "#_Pattern ___ Male Special\n");

    for (fleetIndex = 0; fleetIndex < bm->RBCestimation.RBCspeciesParam[groupIndex][NumFisheries_id]; fleetIndex++) {
        fprintf(fid, " %d 0 0 0 # FISHERY %d\n", (int) (bm->RBCestimation.RBCspeciesParam[groupIndex][Agesel_Pattern_id]), fleetIndex + 1);
    }

    //Done
    fprintf(fid, "#Selectivity parameters\n");
    fprintf(fid, "#_LO HI INIT PRIOR PR_type SD PHASE env-var use_dev dev_minyr dev_maxyr dev_stddev Block Block_Fxn\n");

    for (fleetIndex = 0; fleetIndex < bm->RBCestimation.RBCspeciesParam[groupIndex][NumFisheries_id]; fleetIndex++) {

        fprintf(fid, " Fleet %d", fleetIndex + 1);

        sv = bm->RBCestimation.RBCspeciesArray[groupIndex].Start_SelInflect[fleetIndex];
        phase = 2;
        if (sv < 0) {
            sv = -sv;
            phase = -2;
        }
        fprintf(fid, " %f %f %f %f 0 99 %d 0 0 0 0 0 0 0 # inflection for logistic", sv / 2.0, sv * 2.0, sv, sv, phase);

        sv = bm->RBCestimation.RBCspeciesArray[groupIndex].Start_SelWidth[fleetIndex];
        phase = 3;
        if (sv < 0) {
            sv = -sv;
            phase = -3;
        }

        fprintf(fid, " %f %f %f %f 0 99 %d 0 0 0 0 0 0 0 # width for logistic", sv / 2.0, sv * 2.0, sv, sv, phase);

        if (dflt[fleetIndex] > 0) { // if discard data

            sv = bm->RBCestimation.RBCspeciesArray[groupIndex].Start_RetInflect[fleetIndex];
            phase = 3;
            if (sv < 0) {
                sv = -sv;
                phase = -3;
            }

            fprintf(fid, " %f %f %f %f 0 99 %d 0 0 0 0 0 0 0 #  inflection for logistic retention", sv / 2.0, sv * 2.0, sv, sv, phase);

            sv = bm->RBCestimation.RBCspeciesArray[groupIndex].Start_RetSlope[fleetIndex];
            phase = 4;
            if (sv < 0) {
                sv = -sv;
                phase = -4;
            }

            fprintf(fid, " 0.2 %f %f %f 0 99 %d 0 0 0 0 0 0 0 #  slope for logistic retention", sv * 2.0, sv, sv, phase);

            fprintf(fid, " 0.001 1 1.0 0.1 0 99 -3 0 0 0 0 0 0 0\n");

            fprintf(fid, " -10 10 0 1 0 99 -3 0 0 0 0 0 0 0\n");

        }
    }

    if (bm->RBCestimation.RBCspeciesParam[groupIndex][Agesel_Pattern_id] == 11) {

        fprintf(fid, "# Age parameters");

        for (fleetIndex = 0; fleetIndex < bm->RBCestimation.RBCspeciesParam[groupIndex][NumFisheries_id]; fleetIndex++) {

            fprintf(fid, "# Fleet %d\n", fleetIndex + 1);

            fprintf(fid, " 0 %d 0.1 0.1 0 99 -3 0 0 0 0 0 0 0 #min age", (int) bm->RBCestimation.RBCspeciesParam[groupIndex][AccumAge_id]);

            fprintf(fid, " 0 %d %d %d 0 99 -3 0 0 0 0 0 0 0 #max age", (int) bm->RBCestimation.RBCspeciesParam[groupIndex][AccumAge_id],
                    (int) bm->RBCestimation.RBCspeciesParam[groupIndex][AccumAge_id], (int) bm->RBCestimation.RBCspeciesParam[groupIndex][AccumAge_id]);

        }
        fprintf(fid, "\n");
    }

    fprintf(fid, "#_Cond 0 #_custom_sel-env_setup (0/1) \n");
    fprintf(fid, "#_Cond -2 2 0 0 -1 99 -2 #_placeholder when no enviro fxns\n");
    fprintf(fid, "#_Cond 0 #_custom_sel-blk_setup (0/1) \n");
    fprintf(fid, "#_Cond -2 2 0 0 -1 99 -2 #_placeholder when no block usage\n");
    fprintf(fid, "#_Cond No selex parm trends \n");
    fprintf(fid, "#_Cond -4 # placeholder for selparm_Dev_Phase\n");
    fprintf(fid, "#_Cond 0 #_env/block/dev_adjust_method (1=standard; 2=logistic trans to keep in base parm bounds; 3=standard w/ no bound check)\n");
    fprintf(fid, "#\n");
    fprintf(fid, "# Tag loss and Tag reporting parameters go next\n");
    fprintf(fid, "0  # TG_custom:  0=no read; 1=read if tags exist\n");
    fprintf(fid, "#_Cond -6 6 1 1 2 0.01 -4 0 0 0 0 0 0 0  #_placeholder if no parameters\n");
    fprintf(fid, "#\n");
    fprintf(fid, "1 #_Variance_adjustments_to_input_values\n");

    // done.

    for (fleetIndex = 0; fleetIndex < bm->RBCestimation.RBCspeciesParam[groupIndex][NumFisheries_id]; fleetIndex++) {
        fprintf(fid, " %f", bm->RBCestimation.RBCspeciesArray[groupIndex].Varadj_CPUE[fleetIndex]);
    }
    fprintf(fid, "      # add to CPUE CV\n");

    for (fleetIndex = 0; fleetIndex < bm->RBCestimation.RBCspeciesParam[groupIndex][NumFisheries_id]; fleetIndex++) {
        fprintf(fid, " %f", bm->RBCestimation.RBCspeciesArray[groupIndex].Varadj_discard[fleetIndex]);
    }
    fprintf(fid, "      # add to discard stdev\n");
    for (fleetIndex = 0; fleetIndex < bm->RBCestimation.RBCspeciesParam[groupIndex][NumFisheries_id]; fleetIndex++) {
        fprintf(fid, " 0");
    }
    fprintf(fid, "      # add to mean bodywt CV\n");
    for (fleetIndex = 0; fleetIndex < bm->RBCestimation.RBCspeciesParam[groupIndex][NumFisheries_id]; fleetIndex++) {
        fprintf(fid, " %f", bm->RBCestimation.RBCspeciesArray[groupIndex].Varadj_length[fleetIndex]);
    }
    fprintf(fid, "      # mult by length comp\n");
    for (fleetIndex = 0; fleetIndex < bm->RBCestimation.RBCspeciesParam[groupIndex][NumFisheries_id]; fleetIndex++) {
        fprintf(fid, " %f", bm->RBCestimation.RBCspeciesArray[groupIndex].Varadj_age[fleetIndex]);
    }
    fprintf(fid, "      # mult by age comp\n");
    for (fleetIndex = 0; fleetIndex < bm->RBCestimation.RBCspeciesParam[groupIndex][NumFisheries_id]; fleetIndex++) {
        fprintf(fid, " 1");
    }
    fprintf(fid, "     # mult by mean size at age\n");

    fprintf(fid, "#\n");
    fprintf(fid, "4 #_maxlambdaphase\n");
    fprintf(fid, "1 #_sd_offset\n");
    fprintf(fid, "#\n");

   if (bm->RBCestimation.RBCspeciesParam[groupIndex][NumChangeLambda_id] > 0) {

        fprintf(fid, "#Lambdas\n");
        fprintf(fid, "%d      #  number of changes to make to default lambdas\n", (int)(bm->RBCestimation.RBCspeciesParam[groupIndex][NumChangeLambda_id]));
        fprintf(fid,"# Like_comp codes:  1=surv; 2=disc; 3=mnwt; 4=length; 5=age; 6=SizeFreq; 7=sizeage; 8=catch;\n");
        fprintf(fid, "# 9=init_equ_catch; 10=recrdev; 11=parm_prior; 12=parm_dev; 13=CrashPen; 14=Morphcomp; 15=Tag-comp; 16=Tag-negbin\n");

        fprintf(fid, "#component  fleet phase lambda sizefreq_meth\n");
        for (fleetIndex = 0; fleetIndex < bm->RBCestimation.RBCspeciesParam[groupIndex][NumFisheries_id]; fleetIndex++) {
            fprintf(fid, " 4 %d 1 0.1 1\n", fleetIndex + 1);
        }
        for (fleetIndex = 0; fleetIndex < bm->RBCestimation.RBCspeciesParam[groupIndex][NumFisheries_id]; fleetIndex++) {
            fprintf(fid, " 5 %d 1 0.1 1\n", fleetIndex + 1);
        }
    } else {
        fprintf(fid, "0   #  number of changes to make to default lambdas\n");
    }

    fprintf(fid, "# lambdas (for info only; columns are phases)\n");
    fprintf(fid, "#  0 0 0 0 #_CPUE/survey:_1\n");
    fprintf(fid, "#  1 1 1 1 #_CPUE/survey:_2\n");
    fprintf(fid, "#  1 1 1 1 #_lencomp:_1\n");
    fprintf(fid, "#  1 1 1 1 #_lencomp:_2\n");
    fprintf(fid, "#  1 1 1 1 #_agecomp:_1\n");
    fprintf(fid, "#  1 1 1 1 #_agecomp:_2\n");
    fprintf(fid, "#  1 1 1 1 #_size-age:_1\n");
    fprintf(fid, "#  1 1 1 1 #_size-age:_2\n");
    fprintf(fid, "#  1 1 1 1 #_init_equ_catch\n");
    fprintf(fid, "#  1 1 1 1 #_recruitments\n");
    fprintf(fid, "#  1 1 1 1 #_parameter-priors\n");
    fprintf(fid, "#  1 1 1 1 #_parameter-dev-vectors\n");
    fprintf(fid, "#  1 1 1 1 #_crashPenLambda\n");
    fprintf(fid, "0 # (0/1) read specs for more stddev reporting \n");

    fprintf(fid, "\n");

    fprintf(fid, "999\n");
    free(dflt);

    fclose(fid);

}

