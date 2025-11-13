/**
 * \defgroup atHarvestLib atHarvestLib
 *  \brief The atHarvest library in the Atlantis code.
 *  This library handles the calculations of catch and discard values based on either values in input files or, in the case of a full
 *  fleet dynamic model, based on effort calculated by the management module.
 *
 *
 *
 *	 All functions that a public outside this library must start with Harvest_.
 *
 *
 *   Author: Bec Gorton
 *   CSIRO Marine Laboratories,  2009
 *
 *	\file atHarvestLib.h
 *   Revisions:
 *
 *		28-10-2009 Bec Gorton
 *		Added the Harvest_Free function.
 *
 *		27-01-2010 Bec Gorton
 *		Renamed the public functions to Harvest_ to make it very clear which are public and which are not.
 *
 */

/**
 *	Annual harvest functions. These are generally called at the start of the model run and then at the start of each year.
 */
void Harvest_Annual_Calculations(MSEBoxModel *bm, FILE *llogfp);
void Harvest_Annual_Reset(MSEBoxModel *bm, FILE *llogfp);


/* Setup and IO functions */
void Harvest_Init(MSEBoxModel *bm, FILE *llogfp);
void Harvest_Free(MSEBoxModel *bm);
void Harvest_Update_Temp_Catch_Array(MSEBoxModel *bm, FILE *llogfp);

/* Harvest Index functions - writing and set/get functions */
void Harvest_Init_Index_Output(MSEBoxModel *bm, FILE *outputFile);
void Harvest_Write_Harvest_Index(MSEBoxModel *bm, int nf, FILE *outputFile);
void Harvest_Set_Harvest_Index(MSEBoxModel *bm, int nf, int index, double value);
double Harvest_Get_Harvest_Index(MSEBoxModel *bm, int nf, int index);

/* Public Functions*/
void Harvest_Init_Box_Arrays(MSEBoxModel *bm, int ij, FILE *llogfp);
void Harvest_Init_Layer_Arrays(MSEBoxModel *bm, int ij, int nl, FILE *llogfp);
void Harvest_Update_Temp_Catch_Array(MSEBoxModel *bm, FILE *llogfp);

void Harvest_Set_Fishery_Active(MSEBoxModel *bm, FILE *llogfp);
int Harvest_Do_Fishing_And_ByCatch(MSEBoxModel *bm, FILE *llogfp, int guildcase, int chrt, double SC, double RC, double NUMS, double **FishingRes,
		double *numsdead, double *waste);
int Harvest_Get_Num_Gear_Changes(MSEBoxModel *bm, int nf, FILE *llogfp);
void Harvest_Update_Total_Catch(MSEBoxModel *bm, double *FishTracers, int habitat, FILE *llogfp);
void Harvest_Allocate_FStat_Arrays(MSEBoxModel *bm);
void Harvest_Skip_biology(MSEBoxModel *bm, FILE *llogfp); /* Dummy routine to use when biology turned off so can quickly test fisheries */
void Harvest_Update_Habitat_Overlap(MSEBoxModel *bm, int b);
void Harvest_Scale_Q(MSEBoxModel *bm, int species, int fishery, double start, double period, double mult ,FILE *llogfp);


/* Discards information */
double Harvest_Get_Fixed_Cohort_Discard(int sp, int nf, int chrt, FILE *llogfp);

/* Catch related information */
double Harvest_Get_TotCumCatch(int sp, int nf, int iy);
double Harvest_Get_Habitat_Availability(MSEBoxModel *bm, int sp, int stage, int nf, double q, int do_debug, FILE *llogfp);

/* Reading in and recalculating time series data */
void Harvest_Recalc_Time_Series_Distrib(MSEBoxModel *bm, double **sptotden, int sp, int decdis, FILE *llogfp);

void Update_Harvest_Index_Values(MSEBoxModel *bm, FILE *llogfp);
/*
 * Harvest statistics functions
 *
 */
void Harvest_Report_Fisheries_Stats(MSEBoxModel *bm, FILE *llogfp);
void Harvest_Refresh_Fishing_Stats(MSEBoxModel *bm);
void Harvest_Report_Annual_Stats(MSEBoxModel *bm, FILE *llogfp);
void Harvest_Report_Monthly_Stats(MSEBoxModel *bm, FILE *llogfp);

/* Needed in Annual Management for Ecosytem Cap calculations too */
double Get_Fishery_Group_Change_Scale(MSEBoxModel *bm, int nf, int sp, int paramID, int numChangeParamID, double ***changeArray);
double Get_Catch_Selectivity(MSEBoxModel *bm, int sp, int stage, int nf, double li, double *gear_change_scale, int *sel_curve);

/* Array needed for tracking fisheries interactions */
extern int **FisherySpeciesCatchFlags; /* Array to hold a flag for each fishery/species combinations to indicate which species are fished by which fisheries */
