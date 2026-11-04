/**
 *  \file atManage.h
 *  \ingroup atManageLib
 *   atmanage.h is a header file for management routines in south east ecological models
 *   Author: Beth Fulton
 *   CSIRO Marine Laboratories,  2004
 *
 *   Revisions: 8/8/2004 Created from the semanage.h file
 *
 *    16/8/2004 Cut back to include only those integers and doubles actually
 *                 used in msemanage.c
 *
 *              21/9/2004 Renamed FSL, FSO, FSF, FST and FSG to generic names so apply to
 *       other models/generic case.
 *       FSL -> FDC  (Ling -> Demersal_DC_Fish)
 *       FSO -> FDO  (Orange_Roughy -> Demersal_O_Fish)
 *       FSF -> FDB  (Flathead -> Demersal_B_Fish)
 *       FST -> FVO  (Bluefin_Tuna -> Pisciv_O_Fish)
 *			   FSG -> SHB  (Gummy_Shark -> Shark_B)
 *
 *              5/10/2004 Added parameter initalisation for new fisheries (ptrawlPWN,
 *				   dtrawlFBP, midwcZL, trapFDE, dlineFDE, netFDE) and biological
 *				   groups (BML, BFF, ZL, PWN, FBP, FDE, FDF, FVB, SSK, SHR, SP, REP, WDG)
 *
 *				18/1/2005 Modfified age specific selectivity names so now just one
 *					addults and one juvenile entry for each group (rather than a fishery
 *					entry for each group). Also added FisheriesID.
 *
 *				8/6/2005 Added mowMA and the 3 groups FDM, FDP, WHS.
 *
 *				16/04/2008 Bec Gorton
 *					Removed the tracers from this file and moved them into atTracer.h
 *
 *				29-06-2009 Bec Gorton
 *				Changed references to fisheryPrmStrings with bm->fisheryParamNAME. This means these
 *				names can be refenced from the XML code.
 *
 *				17-09-2009 Bec Gorton
 *				Started moving the harvest parameters out of the management code that allocates and frees arrays.
 *
 *				28-10-2009 Bec Gorton
 *				Removed many parameter definitions as they are only used in a single file and thus can be local
 *				in that file.
 *
 *				03-11-2009 Bec Gorton
 *				Removed old variables that are no longer used in this library.
 *
 */

#include <sjwlib.h>
#include <atlantisboxmodel.h>
#include <atUtilLib.h>
#include "atManageLib.h"


/***********************************************************************************
Modelling variables for control of processes within the model
*/


extern int need_discard;
extern int /***catchind, */*flagdropeffort, *MPAKeyMap, *checkedbox;

/* External parameters */
extern double tot_dyn_sea_area;


/*************************************************************************************
Model  Parameters
*/

extern double k_proprecfish;

extern int POP_max_num_changes,
	nshot;  // Used in shot-by-shot CPUE generation (needs to be global as used in generation and output writing)


/** Model Performance Indicators
 *
 */
extern char **manageindxNAME;		   /**< name of management performance measures */
extern double **manageindx;	       /**< management performance measures */
extern double **mEff;
extern double **gear_conflict;
extern double **origEffort_vdistrib;
extern double *whichrefi;
extern double ***CAPchange;
extern double ***EFFORTchange;


extern double ***MPAendangeredlist;        /**< List of potential zoning (spatial management) based on
								           PET species needs */

extern double **MPAoverfishedlist;         /* List of potential zoning (spatial management) based on
								           overfished species (for US-like system) */

extern double ***LeverQueue;				/* MSY-based management queue */

extern double ***cpue_cdf;                  /* Cumulative distributions for the probabilty distribution associate
                                     with catch in CPUE generation - typically a negative binomial
                                     filled during set-up */
extern double *tot_alloced;

extern int flagcount;

extern double k_avgcount, k_varcount;

extern BOX_THREAD_LOCAL BoxLayerValues *boxLayerInfo;

/**************************************************************************
 Needed for close kin - leave in atCLoseKin.c for now, may move here later  */
/*
double *nsamps_y, ***nsamps_sya, *x, *sx, **inv_totfec_sy, ***n_sya, ****psurv_syay, *****Pr_PO_syaya,
    *****Pr_HS_syaya, ****Pr_GG_yaya, ****Pr_HS_yaya, *****ncomps_PO_syaya, ****ncomps_HS_yaya,
    *****Ekin_HS_syaya, *****Ekin_PO_syaya, *****sim_nkin_PO, *****sim_nkin_HS;
 */

/**************************************************************************
  Function prototypes
*/

/* Harvesting routines */
void Basic_Habitat_Overlap(MSEBoxModel *bm, int b, FILE *llogfp);

/* Management routines */



/* Initialisation routines */
int Read_Manage_Paramaters(MSEBoxModel *bm, char *filename);
void Open_Tiered_Output_File(MSEBoxModel *bm);
void Close_Tiered_Output_Files(MSEBoxModel *bm);
void Setup_MPA_Lists(MSEBoxModel *bm, FILE *llogfp);

void	Distance_to_Port(MSEBoxModel *bm);
/* Index routines */
void Update_Management_Indx(MSEBoxModel *bm, FILE *llogfp);
void Set_Manage_Index_Names(MSEBoxModel *bm);
void Open_Management_Index_File(MSEBoxModel *bm);
void Close_Management_Index_File(MSEBoxModel *bm);
void Open_Manage_Output_Files(MSEBoxModel *bm);
void Close_Manage_Output_Files(MSEBoxModel *bm);


/* Record output prototypes */
void Report_TACs(MSEBoxModel *bm, FILE *llogfp);
void WriteAnnBrokenStickFile(MSEBoxModel *bm,  int species, int nf, int tier, double FrefLim, double FrefA, double FrefH, double Blim, double BrefA, double BrefB, double FCurr, double FTARG, double Bcurr, double scalar);
void WriteAnnCapResultFile(MSEBoxModel *bm,  int species, int nf);
void Write_CPUEreport(MSEBoxModel *bm, int nf, FILE *llogfp, double *tot_alloced);
void Write_CPUE(MSEBoxModel *bm, FILE *llogfp);

/* Output files */
FILE * initAnnTACFile(MSEBoxModel *bm);
FILE * initCPUEGenFile(MSEBoxModel *bm);
FILE * initCPUEFile(MSEBoxModel *bm);

/* The tiered assessment related routines */
void CallTierAssessment(MSEBoxModel *bm, int species, int year, FILE *llogfp);
void Call_PGMSY(MSEBoxModel *bm, int year, FILE *llogfp);
void DoTierAssessment(MSEBoxModel *bm, int species, int year, FILE *llogfp);
void DoMultiStockAssessment(MSEBoxModel *bm, int year, FILE *llogfp);
void Evaluation(MSEBoxModel *bm, int species, int year, FILE *llogfp, FILE *fid);
void DynamicTier4Assessment(MSEBoxModel *bm, int species, int year, FILE *llogfp);
void SS330Assessment(MSEBoxModel *bm, int sp, int year, FILE *llogfp);

void writeDynTier4inputfile(MSEBoxModel *bm, int sp, int year, FILE *datfid, FILE *ctlfid);
void setRBC(MSEBoxModel *bm, int species, int year, FILE *llogfp);

void Do_Atlantis_PGMSY(MSEBoxModel *bm, int this_year);

#ifdef RASSESS_LINK_ENABLED
/* R related */
void Do_PGMSY_R(MSEBoxModel *bm, int year, FILE *llogfp);
#endif
