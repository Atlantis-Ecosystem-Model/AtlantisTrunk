/**
 * \defgroup ateconomic ateconomic
 *
 * The economic module of Atlantis
 *
 *
 */
/**
 *   ateconomic.h is a header file for management routines in south east ecological models
 *   Author: Beth Fulton
 *   CSIRO Marine Laboratories,  2004
 *
 *   Revisions: 27/10/2004 Created the file
 *
 *   28-10-2009 Bec Gorton
 *   The following arrays were moved into the atHarvestModule:
 *   	SELchange
 *   	Qchange
 *    	Pchange
 *		SWEPTchange
 *		DISCRDchange
 *
 */

#ifndef _WIN32
#define	UINT		unsigned int
#define BOOL  UINT
#endif

#define linPI_id    0
#define idd_id		1

#include <sjwlib.h>
#include <atlantisboxmodel.h>
#include <atUtilLib.h>
#include "atHarvestLib.h"
#include <atEconomicLib.h>

/***********************************************************************************
 Modelling variables for control of processes within the model
 */
extern double ****pi, ****friendship_netwk, ****RegCatch, ***linearPI, ***prop_tow_time, **tot_marg_rent, **totland, **tot_cumcatch, **quota_left,
		***net_return, *ax, *bx, *cx, *dx, *ex, *portweight, *maxTargetCatch, *all_pi, *max_demand, *any2spare, *old_effort, *new_effort, *orig_pi, *raw_pi,
		*tradeable_quota, *quota_check;

extern int **boats_free, **boats_new, *best_subfleet, *tempTarget, *redo_effort;

/* External parameters */
extern double H2Otemp, tot_dyn_sea_area, X_CN, k_avgcatch, k_varcatch;

extern int it_count, waterboundary, flagcatch, flaglbs;

//double k_initEffortThresh;

extern char ** EconIndicatorInputNames;
extern char ** entryInputNames;
extern char **econInputNames;

extern char *monthStrings[];

/*************************************************************************************
 Model  Parameters
 */
/**************************************************************************
 Function prototypes
 */

/* Initialisation routines */

int Read_Economic_Parameters(MSEBoxModel *bm, char *filename);

void Initialise_Subfleets(MSEBoxModel *bm, FILE *llogfp);
void Read_Econ_Time_Series(char *folderPath, char *name, char *tsname, char *t_units, FisheryTimeSeries **ts);

/* Economic model routines */
void Allocate_Catch(MSEBoxModel *bm, FILE *llogfp);
void Allocate_Final_Effort(MSEBoxModel *bm, FILE *llogfp);
void Annual_Effort_Schedule(MSEBoxModel *bm, FILE *llogfp);
void Annual_Quota_Allocation(MSEBoxModel *bm, FILE *llogfp);
void Consolidate_Allowed_Catches(MSEBoxModel *bm, FILE *llogfp);
void Economic_Response(MSEBoxModel *bm, FILE *llogfp);
void Get_Fish_Prices(MSEBoxModel *bm, FILE *llogfp);
void Marginal_Profit_Calc(MSEBoxModel *bm, FILE *llogfp);
void Marginal_Rent_Calc(MSEBoxModel *bm, FILE *llogfp);
void Monthly_Effort_Schedule(MSEBoxModel *bm, FILE *llogfp);
void Quota_trade(MSEBoxModel *bm, FILE *llogfp);
void Total_Catch_Value(MSEBoxModel *bm, FILE *llogfp);
void Total_Quota_Price(MSEBoxModel *bm, FILE *llogfp);
void Port_Growth(MSEBoxModel *bm, FILE *llogfp);
void Economic_Indicators(MSEBoxModel *bm, FILE *llogfp);

void Effort_Quota_trade(MSEBoxModel *bm, FILE *llogfp);
void Update_Vessel_Numbers(MSEBoxModel *bm, FILE *llogfp);

/* Economic indicator calculation routines */
double EconIndCalc(MSEBoxModel *bm, int nf, int ns, int econind_id, int ecolharvind_id, int actharvind_id);

void Dynamic_Econ_Indicators(MSEBoxModel *bm, int nf, int ns, FILE *llogfp);
void Expected_Indicators(MSEBoxModel *bm, int nf, int ns, FILE *llogfp);
void Profit_Dependent_Indicators(MSEBoxModel *bm, int nf, int ns, FILE *llogfp);
void Simple_Econ_Indicators(MSEBoxModel *bm, int nf, int ns, FILE *llogfp);
void Update_Econ_Indx(MSEBoxModel *bm, FILE *llogfp);
void Update_Port_Indx(MSEBoxModel *bm, FILE *llogfp);

/* Open and close all of the economic output files. */
void Open_Econ_Indx_Files(MSEBoxModel *bm);
void Close_Econ_Indx_Files(MSEBoxModel *bm);

/* Numerical routines */
void Quicksort_Dir(double *x, double *bbx, double *ccx, double *ddx, double *eex, int n, int ascendflag);

/* Util functions */
double Get_Deemed_Value_Scale(MSEBoxModel *bm, int sp, int nf);
double Get_Expected_Profit(MSEBoxModel *bm, int nf, int ns, int sp, int month, int ntarg, double ExpEffort, char *checkName, int do_debug, FILE *llogfp);
