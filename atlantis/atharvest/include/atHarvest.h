/**
 *   \file atHarvest.h
 *   \brief The private header file for the harvest module.
 *	 \ingroup atHarvestLib
 *
 *   Functions and variables that are defined in this header are only defined within the atharvest library.
 *   Depending on the size of this header we may need to split it into subsections at a later date.
 *
 *   \author: Beth Fulton/Bec Gorton
 *   CSIRO Marine Laboratories,  2004
 *
 *   Revisions:
 *
 *   8/8/2004 Created from the semanage.h file
 *
 *	16/8/2004 Cut back to include only those integers and doubles actually used in atharvest.c
 *
 *
 * 	22-04-2009 Bec Gorton
 *  Removed the sec_in_tsz variable.
 *
 *  28-10-2009 Bec Gorton
 *  Moved the following arrays into the atHarvestModule.
 *  	FFCDR
 *		checkedbox
 *		Qchange
 *		DISCRDchange
 *		mFCchange
 *		SELchange
 *		Pchange
 *		SWEPTchange
 *
 *	Moved the following arrays into the atImplementationModule
 *		CatchSum
 *
 *	Moved the following arrays into the atManageModule:
 *		EFFORTchange
 *		CAPchange
 *		origEffort_vdistrib
 *
 *
 *	28-01-2010 Bec Gorton
 *	Removed the atharvestModule in favor of making more variables and functions private to the atHarvest library.
 *	Moved the definitions of the imposed catch and discard variables out of the management lib.
 */

#include <sjwlib.h>
#include <atlantisboxmodel.h>
#include <atUtilLib.h>
#include <atHarvestLib.h>

/***********************************************************************************
 Modelling variables for control of processes within the model
 */

extern double ***p_fishi, ***Effort_vdistrib, ****p_fish_origi, ***MPAchange, **FC_hdistrib, **effort_scale, ***Effort_hdistrib, ***qSTOCK;

extern double ****RegCatch; // From economics library

extern double H2Otemp, tot_dyn_sea_area;
extern double wcLayerThick, smLayerThick;

/*************************************************************************************
 Model  Parameters
 */
/* From atmanage.h */
extern double KDEP_fishery;	/* Depth fishery gear penetrates into sediment */

/* Arrays */
extern double **k_cover;	/** BoxHabitatProportion*/
extern double k_pattern;
extern double k_patches;
extern double k_mismatch;	/** Reduction in effectiveness of gear due to mismatch in watercolumn of gear and vertebrates*/

extern double ***FFCDR;			/** Array of discard rates */
extern double ***SELchange;		/** Changes in selectivity through time*/
extern double ***Pchange;
extern double ****Qchange;
extern double ***SWEPTchange;
//extern double ****mFCchange;

extern double ****DISCRDchange;
extern double **OldCatchSum;
extern int *checkedBox;

extern double ***TotCumCatch; /**< Total cumulative catch over the entire region (or stock) */
extern int Q_max_num_changes, mFC_max_num_changes;

/**
 * Imposed catch and discard time series information variables.
 */
extern int tsCatchwarned;
extern int ntsCatch; /**< Number of catch time series (should match one per box at most,
 but keep this int as check) */
extern int tsCatchtype; /**< Whether to use interpolated or last valid entries from time-series */
extern int *tscatchid; /**< Array matching boxmodel species ids to catch time series entry ids */
extern FisheryTimeSeries *tsCatch; /**< List of catch time series (one per box) */

extern int tsDiscardwarned;
extern int ntsDiscard; /**< Number of discard time series (should match one per box at most,
 but keep this int as check) */
extern int tsDiscardtype; /**< Whether to use interpolated or last valid entries from time-series */
extern int *tsdiscardid; /**< Array matching boxmodel species ids to discard time series entry ids */
extern FisheryTimeSeries *tsDiscard; /**< List of discard time series (one per box) */

int first_data_done;


/* Harvest Performance measure variables */
extern char **harvestindxNAME; /**< name of fisheries performance measures */
extern double **harvestindx; /**< fisheries performance measures */


/* IO Functions */
int Read_Harvest_Parameters(MSEBoxModel *bm, char *filename);

/**
 * Catch and Discard calculation functions - these are mainly called from within Harvest_Do_Fishing_And_ByCatch
 */
double 	Get_Selectivity(MSEBoxModel *bm, int sp, int stage, int nf, double li, int sel_curve, double addlsm, double addsigma);
double 	Get_Discards(MSEBoxModel *bm, int guildcase, int chrt, int stage, int nf, int do_debug, int do_debug_econ, int do_debug_dis, int flagimposecatch, int boxkey_id, double Biom, double li, double vert_scale, double loadFC, double *SPtoFC, double *quota, double *FCwaste, double mpa_losses,int depend_dis, FILE *llogfp);
int Get_Catch(MSEBoxModel *bm, int sp, int chrt, int stage, int nf, int do_debug, int flagimposecatch, int depend_dis, int boxkey_id, double Biom, double li, double vert_scale, double *gear_change_scale, double *loadFC, double *SPtoFC, double *mpa_losses, FILE *llogfp);
double 	Get_Fishery_Change_Scale(MSEBoxModel *bm, int nf, int paramID, int numChangeParamID, double ***changeArray);

/**
 * Performance measure functions
 */
//void Update_Harvest_Index_Values(MSEBoxModel *bm, FILE *llogfp);
void Set_Harvest_Index_Names(MSEBoxModel *bm);

/**
 * Imposed catch and discard time series information functions
 */
double Get_Imposed_Catch(MSEBoxModel *bm, int guildcase, int chrt, int stage, int nf, int do_debug, double vert_scale, double Biom, int boxkey_id,
		double *mpa_losses, FILE *llogfp);

int Load_Imposed_Catch(MSEBoxModel *bm, FILE *llogfp);
int Load_Imposed_Discards(MSEBoxModel *bm);
