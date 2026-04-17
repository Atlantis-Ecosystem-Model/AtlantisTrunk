/**
 \file
 \brief Routines dealing with point reading in time series data.
 \ingroup atEcology

 File:           atecologyts.c

 Created:        Sat Dec 30 12:02:20 EST 1995

 Author:         Stephen Walker
 CSIRO Division of Oceanography

 Purpose:        Routines dealing with reading in time series data.

 Arguments:      See routines below

 Returns:        See below

 Changes:

 10-05-2010 Bec Gorton
 Added code to read in a setup a new linear mortality time series.


 **/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <sjwlib.h>
#include "atecology.h"
#include <atExternalScalar.h>

static void Read_Ecology_Time_Series(MSEBoxModel *bm, char *key, TimeSeries **ts, char *t_units, void(*err)(char *format, ...)) {
	FILE *fp;
	char buf[BMSLEN];

	/* Open the file */
	if ((fp = Open_Input_File(bm->inputFolder,bm->forceIfname, "r")) == NULL)
		quit("Read_Ecology_Time_Series: Can't open %s%s\n", bm->inputFolder, bm->forceIfname);

	/** Get time series **/
	/* Find time series name in parameter file */
	set_keyprm_errfn(err);
	if (!readkeyprm_s(fp, key, buf)) {
		fclose(fp);
		return;
	}

	if (verbose)
		fprintf(stderr, "Read_Ecology_Time_Series: reading from %s\n", key);

	/* Allocate space for time series */
	if (*ts == NULL && (*ts = (TimeSeries *) malloc(sizeof(TimeSeries))) == NULL)
		quit("Read_Ecology_Time_Series: Can't allocate memory for %s time series\n", key);

	/* Read recruitment series data */
	tsRead(bm->inputFolder, buf, *ts);

	/* Check data time units */
	if (strcmp(t_units, (*ts)->t_units) != 0)
		tsNewTimeUnits(*ts, t_units);

	/* Close the file */
	fclose(fp);

	return;
}

/**
 *	\brief Reads a list of time series of recruitment forcing from an ascii file.
 *
 *	@param bm Pointer to box model
 *	@param key Pointer to name of series to read
 *	@param ts Pointer to array of TimeSeries structures to populate - not yet allocated.
 *	@param t_units Pointer to time units used by the hydrological model
 *	@param err Pointer to the error function to be called if there is an error.
 */
void Ecology_Read_Recruit_TS(MSEBoxModel *bm, char *key, TimeSeries **ts, char *t_units, void(*err)(char *format, ...)) {
    
	FILE *fp;
	char buf[BMSLEN];

	/* Open the file */
	if ((fp = Open_Input_File(bm->inputFolder,bm->forceIfname, "r")) == NULL)
		quit("Ecology_Read_Recruit_TS: Can't open %s%s\n", bm->inputFolder, bm->forceIfname);

	/** Get recruitment time series **/
	/* Find time series name in parameter file */
	set_keyprm_errfn(err);
	if (!readkeyprm_s(fp, key, buf)) {
		fclose(fp);
		return;
	}

	if (verbose)
		fprintf(stderr, "Ecology_Read_Recruit_TS: reading from %s\n", key);

	/* Allocate space for time series */
	if (*ts == NULL && (*ts = (TimeSeries *) malloc(sizeof(TimeSeries))) == NULL)
		quit("Ecology_Read_Recruit_TS: Can't allocate memory for %s time series\n", key);

	/* Read recruitment series data */
	tsRead(bm->inputFolder, buf, *ts);

	/* Check data time units */
	if (strcmp(t_units, (*ts)->t_units) != 0)
		tsNewTimeUnits(*ts, t_units);

	/* Close the file */
	fclose(fp);

	return;
}

/**
 *
 *	\brief Read recruitment environment forcing time series
 *
 */
void Ecology_Read_Enviro_Forcing(MSEBoxModel *bm, char *key, TimeSeries **ts, char *t_units, void(*err)(char *format, ...)) {
	FILE *fp;
	char buf[BMSLEN];

	/* Open the file */
	if ((fp = Open_Input_File(bm->inputFolder,bm->forceIfname, "r")) == NULL)
		quit("Ecology_Read_Enviro_Forcing: Can't open %s%s\n", bm->inputFolder, bm->forceIfname);

	/* Find time series name in parameter file */
	set_keyprm_errfn(err);
	if (!readkeyprm_s(fp, key, buf)) {
		/* Close the file */
		fclose(fp);
		return;
	}

	if (verbose)
		fprintf(stderr, "Ecology_Read_Enviro_Forcing: reading from %s\n", key);

	/* Allocate space for time series */
	if (*ts == NULL && (*ts = (TimeSeries *) malloc(sizeof(TimeSeries))) == NULL)
		quit("Ecology_Read_Enviro_Forcing: Can't allocate memory for %s time series\n", key);

	/* Read forcing data */
	tsRead(bm->inputFolder, buf, *ts);

	/* Check data time units */
	if (strcmp(t_units, (*ts)->t_units) != 0)
		tsNewTimeUnits(*ts, t_units);

	/* Close the file */
	fclose(fp);
}

/**************************************************************************//**
 *	\brief Reads a list of time series of linear mortality scaling forcing from an ascii file.
 *
 *	@param bm Pointer to box model
 *	@param key Pointer to name of series to read
 *	@param ts Pointer to array of TimeSeries structures to populate - not yet allocated.
 *	@param t_units Pointer to time units used by the hydrological model
 *	@param err Pointer to the error function to be called if there is an error.
 *****************************************************************************/
void Ecology_Read_LinearMortality_TS(MSEBoxModel *bm, char *key, TimeSeries **ts, char *t_units, void(*err)(char *format, ...)) {

	if (verbose > 1)
		printf("Reading in linear mortality time series forcing file\n");
	Read_Ecology_Time_Series(bm, key, ts, t_units, err);
}

/**
 * \brief Setup the links between the variables defined in the linear mortality time series file
 * and each of the functional groups. The resulting indices are stored in the bm->Sp_prms array.
 */
void Setup_Linear_Mortality_Indices(MSEBoxModel *bm) {
	int b, sp;
	char str[50];
    
    if (verbose > 1)
        printf("Doing Setup_Linear_Mortality_Indices\n");

    for (sp = 0; sp < bm->K_num_tot_sp; sp++) {
		if (FunctGroupArray[sp].groupAgeType == AGE_STRUCTURED || FunctGroupArray[sp].groupAgeType == AGE_STRUCTURED_BIOMASS) {
			for (b = 0; b < FunctGroupArray[sp].numStages; b++)
				FunctGroupArray[sp].cohortSpeciesParams[b][mL_scale_id] = -1;
		} else {
			FunctGroupArray[sp].cohortSpeciesParams[0][mL_scale_id] = -1;
		}
	}
	if (bm->tslinearMort != NULL) {
		/* Set up the indexing on the names in the forcing file */
		for (sp = 0; sp < bm->K_num_tot_sp; sp++) {
			if (FunctGroupArray[sp].groupAgeType == AGE_STRUCTURED || FunctGroupArray[sp].groupAgeType == AGE_STRUCTURED_BIOMASS) {

                if (verbose > 1)
                    printf("Doing Setup Linear_Mortality step juv for %s with tslinearMort->nv: %d\n", FunctGroupArray[sp].groupCode, bm->tslinearMort->nv);

                
				snprintf(str, sizeof(str), "juv_%s", FunctGroupArray[sp].groupCode);
				for (b = 0; b < bm->tslinearMort->nv; b++) {
					if (strcmp(str, bm->tslinearMort->varname[b]) == 0) {
						FunctGroupArray[sp].cohortSpeciesParams[juv_id][mL_scale_id] = b;
                        break;
					}
				}

                if (verbose > 1)
                    printf("Doing Setup Linear_Mortality step adult for %s with tslinearMort->nv: %d\n", FunctGroupArray[sp].groupCode, bm->tslinearMort->nv);

                snprintf(str, sizeof(str), "adult_%s", FunctGroupArray[sp].groupCode);
				for (b = 0; b < bm->tslinearMort->nv; b++) {
					if (strcmp(str, bm->tslinearMort->varname[b]) == 0) {
						FunctGroupArray[sp].cohortSpeciesParams[adult_id][mL_scale_id] = b;
                        break;
					}
				}

			} else {
				/* Biomass pool so don't worry about adult and juv values */

                if (verbose > 1)
                    printf("Doing Setup Linear_Mortality for %s with tslinearMort->nv: %d\n", FunctGroupArray[sp].groupCode, bm->tslinearMort->nv);

				for (b = 0; b < bm->tslinearMort->nv; b++) {
					if (strcmp(FunctGroupArray[sp].groupCode, bm->tslinearMort->varname[b]) == 0) {
						FunctGroupArray[sp].cohortSpeciesParams[0][mL_scale_id] = b;
                        break;
					}
				}
			}
		}
	}
    
    if (verbose > 1)
        printf("Finishing Setup_Linear_Mortality_Indices\n");

    return;
}

/**************************************************************************//**
 *	\brief Reads a list of time series of size (sn and rn) scaling forcing from an ascii file.
 *
 *	@param bm Pointer to box model
 *	@param key Pointer to name of series to read
 *	@param ts Pointer to array of TimeSeries structures to populate - not yet allocated.
 *	@param t_units Pointer to time units used by the hydrological model
 *	@param err Pointer to the error function to be called if there is an error.
 *****************************************************************************/
void Ecology_Read_Size_Change_TS(MSEBoxModel *bm, char *key, TimeSeries **ts, char *t_units, void(*err)(char *format, ...)) {

	printf("Reading in size change time series forcing file with units %s\n", t_units);
	if (verbose > 1)
		fprintf(bm->logFile, "Reading in size change time series forcing file\n");
	Read_Ecology_Time_Series(bm, key, ts, t_units, err);
}


/**************************************************************************//**
 *	\brief Reads a list of time series of growth rate changes from a ts file.
 *
 *	@param bm Pointer to box model
 *	@param key Pointer to name of series to read
 *	@param ts Pointer to array of TimeSeries structures to populate - not yet allocated.
 *	@param t_units Pointer to time units used by the hydrological model
 *	@param err Pointer to the error function to be called if there is an error.
 *****************************************************************************/
void Ecology_Read_Growth_Rate_TS(MSEBoxModel *bm, char *key, TimeSeries **ts,
		char *t_units, void(*err)(char *format, ...)) {

	printf("Reading in growth rate time series forcing file with units %s\n", t_units);
	if (verbose > 1)
		fprintf(bm->logFile,
				"Reading in growth rate time series forcing file\n");
	Read_Ecology_Time_Series(bm, key, ts, t_units, err);
}

/**************************************************************************//**
 *	\brief Reads a list of time series of FSPB changes from a ts file.
 *
 *	@param bm Pointer to box model
 *	@param key Pointer to name of series to read
 *	@param ts Pointer to array of TimeSeries structures to populate - not yet allocated.
 *	@param t_units Pointer to time units used by the hydrological model
 *	@param err Pointer to the error function to be called if there is an error.
 *****************************************************************************/
void Ecology_Read_FSPB_TS(MSEBoxModel *bm, char *key, TimeSeries **ts,
		char *t_units, void(*err)(char *format, ...)) {

	printf("Reading in FSPB time series forcing file with units %s\n", t_units);
	if (verbose > 1)
		fprintf(bm->logFile,
				"Reading in FSPB time series forcing file\n");
	Read_Ecology_Time_Series(bm, key, ts, t_units, err);
}

/**
 * \brief Setup the links between the variables defined in the time series file
 * and each of the functional groups. The resulting indices are stored in the bm->Sp_prms array.
 */
void Setup_Change_Indices(MSEBoxModel *bm, TimeSeries *ts, int index) {
	int b, sp, n;
	char str[50];
    
    //if(verbose)
    //    printf("Doing Setup_Change_Indices\n");

	for (sp = 0; sp < bm->K_num_tot_sp; sp++) {
		for(n = 0; n < FunctGroupArray[sp].numCohortsXnumGenes; n++){
			/* By default values are set to -1 so we can easily determine it values are not in the ts file */
			bm->scaling_indices[index][sp][n] = -1;
		}
	}
	if(ts != NULL){
		/* Set up the indexing on the names in the forcing file */
		for (sp = 0; sp < bm->K_num_tot_sp; sp++) {
			if (FunctGroupArray[sp].isVertebrate == TRUE) {
				for(n = 0; n < FunctGroupArray[sp].numCohortsXnumGenes; n++){
					snprintf(str, sizeof(str), "%s_%d", FunctGroupArray[sp].groupCode, n);
					for (b = 0; b < ts->nv; b++) {
						if (strcmp(str, ts->varname[b]) == 0){
							bm->scaling_indices[index][sp][n] = b;
                            break;
						}
					}
				}
			}
		}
	}
}

/**
 * \brief Scale the group size based on scaler values read in from the GrowthRateChange ts forcing input file.
 *
 *
 *
 */
static void Scale_Growth_Rate(MSEBoxModel *bm){
	int sp, scale_index, cohort;
	double scalar;

	if(verbose > 2)
		printf("Scale_Growth_Rate\n");
	if(bm->tsGrowthRateChange != NULL){
		for(sp = 0; sp < bm->K_num_tot_sp; sp++){
			if(FunctGroupArray[sp].isVertebrate == TRUE){  // So far only doing scaling for verebrate groups, but in theory could be for any group
				for(cohort = 0; cohort < FunctGroupArray[sp].numCohortsXnumGenes; cohort++){
					scale_index = bm->scaling_indices[mum_scale_id][sp][cohort];
					if(scale_index != -1){
						scalar = tsEval(bm->tsGrowthRateChange, scale_index, bm->t);
						if(fabs(scalar - 1.0) > 1e-100){
							FunctGroupArray[sp].scaled_mum[cohort] = FunctGroupArray[sp].mum[cohort] * scalar;
                            
                            /**/
							//if((bm->dayt > bm->checkstart) && (bm->debug == debug_mum)){
								fprintf(bm->logFile, "Time: %e, Growth Rate %s:%d - %e scaled by %e. Final growth rate = %e\n",
									bm->dayt, FunctGroupArray[sp].groupCode, cohort, FunctGroupArray[sp].mum[cohort], scalar,
									FunctGroupArray[sp].scaled_mum[cohort]);
							//}
                            /**/
						}
					}
				}
			}
		}
	}
}

/**
 * \brief Calculate the updated FSPB rate.
 *
 * 	This function will read in the value that should be added to the FSPB value for each
 * 	vertebrate cohort from the supplied TS file.
 *
 * 	The index of each cohort for each vertebrate is set in Setup_Change_Indices which is called from initBiology().
 *
 * 	This will set the value in the bm->scaled_FSPB array with the value in bm->FSPB + the value read in.
 *
 * 	Important to remember that the values added are not accumulative but the value read in from the TS input file
 * 	will be added to the initial value from the biology input file.
 *
 *
 */
static void Scale_FSPB_Rate(MSEBoxModel *bm){
	int sp, scale_index, cohort;
	double scalar;

	if(bm->tsFSPBChange != NULL){
		for(sp = 0; sp < bm->K_num_tot_sp; sp++){
			if(FunctGroupArray[sp].isVertebrate == TRUE){
				for(cohort = 0; cohort < FunctGroupArray[sp].numCohortsXnumGenes; cohort++){
					scale_index = bm->scaling_indices[FSPB_scale_id][sp][cohort];
					if(scale_index != -1){
						scalar = tsEval(bm->tsFSPBChange, scale_index, bm->t);

						/* Need to check to see if this should be an addition or a scalar */
						if(fabs(scalar) > 1e-100){
							FunctGroupArray[sp].scaled_FSPB[cohort] = FunctGroupArray[sp].FSPB[cohort] + scalar;
							if(FunctGroupArray[sp].scaled_FSPB[cohort] > 1.0)
								FunctGroupArray[sp].scaled_FSPB[cohort] = 1.0;

                            /**/
							//if((bm->dayt > bm->checkstart) && (bm->debug == debug_FSPB)){
								fprintf(bm->logFile, "Time: %e, FSPB Rate %s:%d - %e with %e added. Final FSPB rate = %e\n",
									bm->dayt, FunctGroupArray[sp].groupCode, cohort, FunctGroupArray[sp].FSPB[cohort], scalar, FunctGroupArray[sp].scaled_FSPB[cohort]);
							//}
                            /**/
						}
					}
				}
			}
		}
	}
}

/**
 * \brief Update the scaled values for each group.
 *
 * This is called once per timestep. Values are calculated for all boxes and layers.
 *
 */
void Ecology_Update_Scaled_Values(MSEBoxModel *bm){
	Scale_Growth_Rate(bm);
	Scale_FSPB_Rate(bm);
	if (bm->use_external_scaling) {
		Calc_Scalar_Values(bm, bm->externalBiologyInput);
	}
    if (bm->flag_dynamicXRS) {
        Create_Update_XRS(bm, bm->logFile);
    }
}

/**************************************************************************//**
 *	\brief Reads a list of time series of pCO2 states from a ts file.
 *
 *	@param bm Pointer to box model
 *	@param key Pointer to name of series to read
 *	@param ts Pointer to array of TimeSeries structures to populate - not yet allocated.
 *	@param t_units Pointer to time units used by the hydrological model
 *	@param err Pointer to the error function to be called if there is an error.
 *****************************************************************************/
void Ecology_Read_pCO2_TS(MSEBoxModel *bm, char *key, TimeSeries **ts,
		char *t_units, void(*err)(char *format, ...)) {

	FILE *fp;
	char buf[BMSLEN];
	int b = 0;

	printf("Reading in pCO2 time series forcing file with units %s\n", t_units);

	if (verbose > 1)
		fprintf(bm->logFile, "Reading in pCO2 time series forcing file\n");

	/* Open the file */
	if ((fp = Open_Input_File(bm->inputFolder, bm->forceIfname, "r")) == NULL)
		quit("readpCO2TimeSeries: Can't open %s\n", bm->forceIfname);

	/** Get time series **/
	/* Find time series name in parameter file */
	set_keyprm_errfn(err);
	if (!readkeyprm_s(fp, key, buf)) {
		fclose(fp);
		return;
	}

	/* Allocate space for time series */
	if (*ts == NULL && (*ts = (TimeSeries *) malloc(sizeof(TimeSeries)))
			== NULL)
		quit( "readEcologyTimeSeries: Can't allocate memory for %s time series\n", key);

	/* Read time series data */
	tsRead(bm->inputFolder, buf, *ts);

	/* Check data time units */
	printf("comparing units %s with %s\n", t_units, (*ts)->t_units);

	if (strcmp(t_units, (*ts)->t_units) != 0)
		tsNewTimeUnits(*ts, t_units);

	/* Close the file */
	fclose(fp);

	bm->pCO2_id = -1;
	for(b=0; b<(*ts)->nv; b++){
		if(strcmp("pCO2",(*ts)->varname[b]) == 0){
			bm->pCO2_id = b;
            break;
		}
	}
	if(bm->pCO2_id < 0)
		quit("Did not find time series of pCO2 in %s - check the ts files and retry\n",(*ts)->df->name);

	return;
}
