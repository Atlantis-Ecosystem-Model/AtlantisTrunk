/**
 * \defgroup atassess atassess
 *
 * The assessment module of Atlantis
 *
 *
 */

/*********************************************************************

 File:           atsample.h

 Created:        8/8/2004

 Author:         Beth Fulton

 Purpose:        Include file for assessment code for Box Model

 Revisions:      8/8/2004 Created from southeast sesample.h

 10/9/2004 Reorganised list of function prototypes so easier to
 follow based on what they processes they are deaDemersal_DC with

 11/9/2004 Removed arrays used in Ecopath calculations
 (no longer required)

 12/9/2004 added prototype for Classical_Assessment()

 14/9/2004 added prototypes for VPA_Assessment(),
 and definitions for the arrays needed in model minimisation routines

 15/9/2004 added prototypes for Amoeba() and amotry(). Also added
 "#define" definitions for alpha, beta and gammaF

 16/9/2004 added prototypes for BackCalc(), SolvePlus(),
 SolveBack(), VPAFunkVal() and funk()

 20/9/2004 added prototypes for Schafer_Production_Assessment(),
 ProdCalc(), ProdFunkVal(), CalcPercentile(), LinearRegression()
 RegressionFunkVal(), and AmoebaL()

 21/9/2004 Renamed FSL, FSO, FSF, FST and FSG to generic
 names so apply to other models/generic case.
 FSL -> FDC  (Ling -> Demersal_DC_Fish)
 FSO -> FDO  (Orange_Roughy -> Demersal_O_Fish)
 FSF -> FDB  (Flathead -> Demersal_B_Fish)
 FST -> FVO  (Bluefin_Tuna -> Pisciv_O_Fish)
 FSG -> SHB  (Gummy_Shark -> Shark_B)

 3/10/2004 Added index names for the additional biological groups
 (prawns, other filter feeders, megazoobenthos, estuarine demersal fish,
 flat deep demersal fish, benthopelagics, other piscivores, penguins,
 skates and rays, reptiles and dugongs) and their diagnostic variables

 16/04/2008 Bec Gorton
 Removed the tracers from this file and moved them into atTracer.h

 22-04-2009 Bec Gorton

 Added the definitions of the functions dealing with the allocation and freeing
 of the atAssessModule. Also removed old commented out variable definitions.

 09-04-2010 Bec Gorton
 Resized the diet info matrices from K_num_tot_assess to 2, K_num_tot_sp.

 14-09-2020 Bec Gorton
 Removed the avginvsze. These value are now stored as speciesParams.

 30-09-2010 Bec Gorton
 Changed DiscardFate to be a 2D array.

 19-10-2010 Bec Gorton
 Removed the sampledSigma array.
 Moved the assessment prototypes into the atclassical file.

 08-11-2010 Bec Gorton
 Removed the aprm array. These values are now stored in the SP_FISHERYprms array.
 Removed the Rmax and qprm arrays. These values are now stored in the speciesParam array.

 12-11-2010 Bec Gotron
 Removed the functions Lubacksub and Ludecomp. These have been replaced with some
 open source functions in doolittle.c.

 *********************************************************************/
#include <atlantisboxmodel.h>
#include <atUtilLib.h>
#include <atEcologyLib.h>
#include <atAssessLib.h>


/* Sampling and attribute id */
#define sample_id 0
#define attrib_id 1

/* Biomass and abundance samplwes for size bin reporting id */
#define bioindex_id 0
#define abundindex_id 1

/* Assessment and fisheries data storage */
#define commerical_id 0
#define survey_id 1

#define alpha 1.0
#define beta 0.5
#define gamma 2.0

#define est1_id 0
#define est2_id 1

/* Sampling and assessment random numbers */
#define startN_id 0
#define calcF_id 1
#define calcM_id 2
#define nrand_id 3

#define tass_id 0
#define tasseat_id 1

/* Assessment estimates */
#define est_r_id 0
#define est_K_id 1
#define est_B0_id 2
#define est_p_id 3
#define est_B_id 4
#define est_I_id 5
#define est_q_id 6
#define est_msy_id 7

#define num_est_prm 8


/* Vertebrate data characteristics */
#define bstruct_id 0
#define bres_id 1
#define bstocknums_id 2
#define bcatchnums_id 3
#define bprod_id 4
#define beat_id 5
#define bdiscards_id 6

/* Sample types */
#define samplestock_id 0
#define samplecatch_id 1
#define samplediscard_id 2

/* Stock characteristics */
#define sbiomass_id 0
#define sstocknums_id 1
#define scatchnums_id 2
#define sprod_id 3
#define seat_id 4
#define sdiscardnums_id 5
#define stotcatch_id 6
#define stoteffort_id 7
#define sdiscards_id 8
#define ssizemat_id 9
#define scondn_id 10

/* Individ characteristics */
#define iweight_id 0
#define istocknums_id 1
#define icatchnums_id 2
#define ibiomass_id 3
#define idiscards_id 4
#define iq_id 5
#define icondn_id 6

#define K_num_individ_char 7

/* Fishery characteristics */
#define fcatch_id 0
#define feffort_id 1
#define fdiscards_id 2

/* Size characteristics */
#define sizeweight_id 0
#define sizestocknums_id 1
#define sizecatchnums_id 2
#define sizediscardnums_id 3
#define sizeselectnums_id 4

/* Total numbers characteristics */
#define totstocknums_id 1
#define totcatchnums_id 2
#define totdiscardnums_id 3
#define totselectnums_id 4

/* Network characteristics */
#define netwk_cons_id 0
#define netwk_prod_id 1
#define netwk_biom_id 2
#define netwk_ex_id 3
#define netwk_resp_id 4
#define netwk_flow_det_id 5
#define netwk_pprod_id 6
#define netwk_imm_req_id 7

#define K_netwk_properties 8

/*********************************************************************
 Global variables
 *********************************************************************/
extern int verbose;

extern double ***initVERTinfo;

/********************************************************************/

//extern char  **spInvName;

/* Parameter arrays */
extern int *InOffshorei;
extern int *PGMSYLinksInputs;

extern double **detrit_import;
extern double ****dietpref;
extern double **DiscardFate;
extern double *TrigPts;

/* Data storage arrays */
extern double *samplearea;
extern double *totalarea;
extern double *zonearea;
extern int *zoneboxes;

extern double ***biolbiom;
extern double **bioleat;
extern double **biolprod;
extern double ****biolVERTinfo;
extern double **phys;
extern double **physprocess;

/* Sample storage arrays */
extern double *****individVERTinfo;
extern double ****invstockinfo;
extern double ***pop_fraction;
extern double ***samplebiom;
extern double ****sampledetbiom;
extern double ***sampleeat;
extern double ***samplephys;
extern double ***sampleprocess;
extern double ***sampleprod;
extern double ****stockinfo;

/* Fisheries data arrays */
extern double *****agebins;
extern double *****agelengthkey;
extern double *****fishery;
extern double **samplenums;
extern double ******sizebins;

/* Diet info arrays */
extern double ****inshorediet;
extern double ****intruediet;
extern double ****offshorediet;
extern double ****offtruediet;
extern double ****totareadiet;
extern double ****tottruediet;
extern double ***TL;
extern double ***trueTL;

/* Data processing arrays */
extern double ****altavailfood;
extern double ****availfood;

extern double **new_altavailfood;
extern double **new_availfood;

extern double **contribvert;
extern double ***biom;
extern double ***globalnums;
extern double ***oldbaby;
extern double **nums;
extern double *totn;
extern double ***totnums;
extern double **totsamplebiom;

/* Network calculation arrays */
extern double ***biomnetwk;
extern double ***eatnetwk;
extern double ***exportnetwk;
extern double ***importnetwk;
extern double ***mortnetwk;
extern double ***prodnetwk;
extern double ***respnetwk;

/* Indice calculation arrays */
extern int **divsp;
extern int **divfn;
extern int ****nsq;
extern int *numsort;
extern int *permpivot;
extern int **stomachs;
extern double *upper;
extern double *lower;
extern int *MultiNom_binIndex;
extern int *MultiNom_cohortIndex;

extern double ***cvsample;
extern double ***cvphys; /* Array to store coefficient variance values for the physical parameters that are sampled */
extern double **food;
extern double **globalfledge;
extern double **globalpd;
extern double **mineat;
extern double ***oi;
extern double **PBRglobal;
extern double *zasum;

/* Arrays to store the standard deviation of sampled values */
extern double **physicalSigma;

/* Indices arrays */
extern double **avgtl;
extern double *cvt;
extern double ***endnums;
extern double ***endnumsbig;
extern double *disrate;
extern double **fledge;
extern double **habindx;
extern double **max_lngth;
extern double **monbet;
extern double **netwkindx;
extern double *nppb;
extern double **PBRcat;
extern double **pd;
extern double **pdcat;
extern double ***pelbin;
extern double **sedbin;
extern double *soi;
extern double ***trophspect;

extern int **divindx;

/* Asessment arrays */
extern double *whichrefi;

extern double *BootResu;
extern double **CData;
extern double *CTData;
extern double **CPUEtrend;
extern double **F;
extern double **IData;
extern double *ITData;
extern double **IDatahat;
extern double **NEst;
extern double ***NResult;
extern double **num_nyr;
extern double **P;
extern double **PP;
extern double **Resu;
extern double *ResultToSort;
extern double *ResultSorted;
extern double *X;
extern double *XX;
extern double *Y;
extern double *YY;
extern double ****zoneVERTpopratio;

extern int *nbsSort;

/* Extra calculation arrays */
extern int *checkedz;

/* Global parameters */
extern double SSmin;

extern int flagphys, phys_samplingsize, flaginvpbiom, flagepibiom, flaginfbiom, flagdetbiom, flagverts, flagprod, flageat, flagcatch, flageffort, flagdiscrd,
		flagcount, flagcurve, flagprms, flagage, flagfishbiom, K_num_stomaches;

extern double k_avgsalt, k_avgphys, k_avgnuts, k_avgprocs, k_avgphytl, k_avgphyts, k_avgzoops, k_avgzoopl, k_avgcep, k_avgpbact, k_avgsbact, k_avginfs, k_avginfl,
		k_avgepif1, k_avgepif2, k_avgppben, k_avgrdet, k_avgldet, k_avgpelpp, k_avgzoopp, k_avgcepp, k_avgpbactp, k_avgsbactp, k_avginfsp, k_avginflp,
		k_avgepi1p, k_avgepi2p, k_avgbenpp, k_avgzoopeat, k_avgcepeat, k_avgpbacteat, k_avgsbacteat, k_avginfseat, k_avginfleat, k_avgepi1eat, k_avgepi2eat,
		k_avgweight, k_avgprodn, k_avgeat, k_avgdiscrd, k_avgcount, k_avgcatch, k_avgeffort, k_avgtdiscrd, k_avgcobs, k_avgcurve, k_avgprms, k_avgage,
		k_avgfish;

extern double k_varsalt, k_varphys, k_varnuts, k_varprocs, k_varphytl, k_varphyts, k_varzoops, k_varzoopl, k_varcep, k_varpbact, k_varsbact, k_varinfs, k_varinfl,
		k_varepif1, k_varepif2, k_varppben, k_varrdet, k_varldet, k_varpelpp, k_varzoopp, k_varcepp, k_varpbactp, k_varsbactp, k_varinfsp, k_varinflp,
		k_varepi1p, k_varepi2p, k_varbenpp, k_varzoopeat, k_varcepeat, k_varpbacteat, k_varsbacteat, k_varinfseat, k_varinfleat, k_varepi1eat, k_varepi2eat,
		k_varweight, k_varprodn, k_vareat, k_vardiscrd, k_varcount, k_varcatch, k_vareffort, k_vartdiscrd, k_varcobs, k_varcurve, k_varprms, k_varage,
		k_varfish;

extern double TINY, minpool;

extern int firstdata, numsteps, numsteps_orig, datain, nextij, nexteatij;

typedef struct {
	double k_avg_Biomass[2];
	double k_var_Biomass[2];
	double k_avg_pp;
	double k_var_pp;
	double k_avg_eat;
	double k_var_eat;
	int spErrorStructureFlag;
} ErrorStructure;

extern ErrorStructure *spErrorStructure;

/*********************************************************************
 Prototypes
 *********************************************************************/

/* Sampling routines */
void Init_Data_Array(MSEBoxModel *bm);
void Populate_Arrays(MSEBoxModel *bm, FILE *ofp);
void Sample_Stuff(MSEBoxModel *bm, FILE *ofp);

/* Sample handDemersal_DC routines */
double Get_Weight(double l, int i);
void Length_Age_Key(MSEBoxModel *bm, int z, int i, int ni, double l, int c, int cmax, double n, int id);
void
Sort_Length_Weight(MSEBoxModel *bm, int casetype, int flag, int ni, int z, int i, int fishery_id, double l, double num, double w, int ci, int id,
		int *lengthclass, FILE *ofp);

/* Diet sampDemersal_DC routines */
void Calculate_Trophic_Levels(MSEBoxModel *bm, double ***tl, double ****indiet, double ****offdiet, double ****totdiet, FILE *ofp);
void Set_Preference_Matrix(MSEBoxModel *bm, FILE *ofp);
void Setup_Avail_Food(MSEBoxModel *bm, int b, FILE *ofp);
void Year_Of_Stomach(MSEBoxModel *bm, FILE *ofp);

/* Ecological indicator routines */
void Calc_ABC(MSEBoxModel *bm, int z, int id);
void Calc_Avgtl(MSEBoxModel *bm, double ***tl, int id, FILE *ofp);
void Calc_CV(MSEBoxModel *bm, int z);
void Calculate_Phys_SD(MSEBoxModel *bm, int z);
void Calc_Discards(MSEBoxModel *bm, int z, int id);
void Calc_Div(MSEBoxModel *bm, FILE *ofp);
void Calc_Hab_Complx(MSEBoxModel *bm, int z, int id);
void Calc_Max_Size(MSEBoxModel *bm, int z, int id);
void Calc_Monbet(MSEBoxModel *bm, int z, int id);
void Calc_Network_Info(MSEBoxModel *bm, int z, int id);
void Calc_Nppb(MSEBoxModel *bm, int z, int id);
void Calc_PBR(MSEBoxModel *bm, int z, int id, FILE *ofp);
void Calc_PD_Ratio(MSEBoxModel *bm, int z, int id);
void Calc_OI(MSEBoxModel *bm, double ***tl, double ****indiet, double ****offdiet, double ****totdiet);
void Calc_Reprod_Success(MSEBoxModel *bm, int z, int id, FILE *ofp);
void Calc_Size_Mature_And_Condn(MSEBoxModel *bm, int z, int id, FILE *ofp);
void Calc_Size_Spectra(MSEBoxModel *bm, int z, int id);
void Calc_SOI(MSEBoxModel *bm, int z, int id, FILE *ofp);
void Calc_Total_PP(MSEBoxModel *bm, int z);
void Calc_Trophic_Spectra(MSEBoxModel *bm, double ***tl, int id);
void Diet_Indices(MSEBoxModel *bm);
void Indices(MSEBoxModel *bm, FILE *ofp);
void Init_Globals_And_Arrays(MSEBoxModel *bm);

/* Attribute calculations and write out routines */
void Calculate_Attribute_Values(MSEBoxModel *bm, FILE *ofp);
void Init_Attribute_Array(MSEBoxModel *bm, FILE *ofp, int id);

void Calculate_Real_Stomachs(MSEBoxModel *bm, FILE *ofp);
void Calculate_True_Diet_Indices(MSEBoxModel *bm);

/* File IO routines */
void Write_Attributes(MSEBoxModel *bm, int ij);
void Write_True_Diet_Out(MSEBoxModel *bm, int ij);

void Open_Attribute_Files(MSEBoxModel *bm);
void Open_Index_Files(MSEBoxModel *bm);

void Close_Attribute_Files(MSEBoxModel *bm);
void Close_Index_Files(MSEBoxModel *bm);
void Write_Index_Out(MSEBoxModel *bm, int ij);

/* Tiered assessment initialisation routines */
int Tier_Assessment_Setup(MSEBoxModel *bm) ;
int Tier_Assessment_Allocate(MSEBoxModel *bm);
int Tier_Assessment_PostLoad_Allocate(MSEBoxModel *bm);
void PreAllocate_Index_Setting(MSEBoxModel *bm);

/* Numerical matrix solution and minimisation routines */
void AmoebaL(MSEBoxModel *bm, double **p, double *y, int ndim, double ftol, int itmax, int funkflag, int nchrt, int YrMax, double prm_sp, int sp, int *nfunk, int *ilow, FILE *ofp, double *xpar);

/* Assessment routines */
void Classical_Assessment(MSEBoxModel *bm, FILE *ofp);
void Estimate_Other_AssessPrm(MSEBoxModel *bm, int sp, FILE *ofp);

/* Writing out assessment result routines and files */
void Write_Assess_Out(MSEBoxModel *bm, int ij);

/* Writing out random numbers */
void Write_Random_Numbers(MSEBoxModel *bm, int numyr, int numsamples);

/* Diet functions  - used in the TL caclulations.*/
void Normalise_AvailFood(MSEBoxModel *bm, double ****availfoodArray);
void new_Normalise_AvailFood(MSEBoxModel *bm, double **availfoodArray);

void Calculate_Overall_Diet_Composition(MSEBoxModel *bm, double ****totDiet, double ****indiet, double ****offdiet);

