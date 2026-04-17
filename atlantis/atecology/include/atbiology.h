/**
 *   atbiology.h is a header file of ecological module in south east Study
 *
 *   Author: Beth Fulton
 *   CSIRO Marine Laboratories
 *   2004
 *
 *   Revisions: 8/8/2004 Created msebiol.h from the sebiol.h file
 *
 *				21/8/2004 Cut back so only the biologically relevant arrays remain
 *
 *				21/9/2004 Created atbiology.h from msebiol.h. And renamed
 *				FSL, FSO, FSF, FST and FSG to generic names so apply to
 *				other models/generic case.
 *				FSL -> FDC  (Ling -> Demersal_DC_Fish)
 *				FSO -> FDO  (Orange_Roughy -> Demersal_O_Fish)
 *				FSF -> FDB  (Flathead -> Demersal_B_Fish)
 *				FST -> FVO  (Bluefin_Tuna -> Pisciv_O_Fish)
 *				FSG -> SHB  (Gummy_Shark -> Shark_B)
 *
 *              26/9/2004 Added Megazoobenthos
 *
 *              30/9/2004 Added Prawns and extra trophic link parameters for exiting
 *				groups so they match the generic diet matrix (see comments
 *				in atprocess.c)
 *
 *              3/10/2004 Added index names for the additional biological groups
 *				(other filter feeders, megazoobenthos, estuarine demersal fish,
 *				flat deep demersal fish, benthopelagics, other piscivores, penguins,
 *				skates and rays, reptiles and dugongs) and their diagnostic variables
 *
 *              15/10/2004 Added BML and BFF.parameters
 *
 *			    19/10/2004 Added Prawn parameters
 *
 *			    24/10/2004 Added soft sediments
 *
 *			    26/10/2004 Added FBP, FVB, SP, FDE, FDF, SHR, SSK, REP, WDG parameters.
 *				    Also removed juvenileCEP as now handled explicitly via cephalopod
 *					aging and migration
 *
 *				9/4/2005 Added parameters needed for new generalised form of invertebrate
 *				   equations (using Invert_Activities())
 *
 *				13/7/2005 Added readkeyprm_darray_checked() and extended readValue()
 *					so that knows what data is expected so that can check on readin
 *					for obvious parameter errors
 *
 *				16/04/2008 Bec Gorton
 * 				Removed the tracers from this file and moved them into atTracer.h
 *
 *				18/05/2008 Bec Gorton
 *				Changed the Name char array from 30 to 50 to fix issues with tracer names over writing
 *				each other.
 *
 *				02/03/2009 Bec Gorton
 *				Changed the structure of the header files in the ecology library. The atecologylib.h header
 *				file now contains the functions that are used outside the ecology library. This header contains the
 *				variables that are global within the ecology library. These will be moved into the atEcologyModule.
 *
 *				14-04-2009 Bec Gorton
 *				Removed the predayt variable.
 *
 *				22-04-2009 Bec Gorton
 *				Got rid of the Name structure as it was just contained a string.
 *
 *				06-07-2009 Bec Gorton
 *				Moved the paramstrings and cohortParamStrings into the atlantisUtil lib.
 *
 *				17-07-2009 Bec Gorton
 *				Moved the k_pattern and k_patches variables into the harvest module.
 *
 *				01-03-2010 Bec Gorton
 *				Added function prototypes for the larval dispersion code.
 */

#define TRUE		1
#define FALSE		0

#ifndef _WIN32
	#define	UINT		unsigned int
	#define BOOL  UINT
#endif

/**************************************************************************************
Local Modelling variables
*/

extern char **Varname;
extern char **DiagVarname;

extern double   sporosity;
extern double   surf_stress;   /* stress on bottom */
extern double   wcLayerThick, smLayerThick, iceLayerThick;  /* depth of wc and sm layer */



/************************************************************************
 Modelling variables for control of processes within the model
*/

extern double Susp_Sed, tot_dyn_sea_area,
	DRdepth, O2depth, newO2depth, Enviro_turb, current_layer_sed, eddy_strength,
	BioirrigEnh, BioturbEnh, Turbatn_contribs, Irrig_contribs, cell_depth,
	H2Otemp, current_SALT, current_PH, init_PH, Bact_stim, current_depth, area_reef, area_flat, LocalRugosity,
	area_canyon, area_box, area_soft, current_SALT, current_ARAG, current_WIND;

extern int numwcvar, numepivar, numlandvar, numdiagvar, numfstatvar, numicevar, first_year, idum;

/* defining sizes for arrays for preference and fish distribution parameters*/
extern int BEDchange_max_num, Tchange_max_num, Schange_max_num, PHchange_max_num, Vchange_max_num;
/**************************************************************************
Model  Parameters
*/
extern double
	Fben_den, FDL_wc,
	MB_wc,	FDL_benth, FDL_fish, FDL_SG_roots, FDL_SG_leaves, FDL_top,
	p_linePIN, p_netPIN, p_trawlPIN, p_mtrawlPIN, p_lineWHT, p_BBfish, p_BBben, p_PBwc, p_PBben, p_IBice,
	recover_span, recover_trigger, recover_subseq, lognorm_mu, p_lineWHS,
	lognorm_sigma, roc_wgt, Kthresh1, Kthresh2, KHTD, KHTI,
	Kthreshm, r_DL_T15, r_DR_T15, r_DON_T15, r_DSi_T15, r_DC_T15,
	FFDDR, FDR_DL, FDON_D, FDR_DC,
	KST_fish, KST_shark, KST_bird, KST_mammal,
	Ktmp_fish, Ktmp_shark, Ktmp_bird, Ktmp_mammal,
	R_0_T15, R_D_T15, Dmax, 
	k_w_depth, k_w_cdepth, k_w_deep, k_w_shallow, k_PN, k_DON, k_DL, k_IS, K_TUR_DEP, k_SED,
	K_addepth, K_conc, K_nit_T15, KI_avail, KIOP_min, KIOP_shift,
	K_IRR, K_MAX_IRR, K_MAX_TUR, K_MIN_IRR, K_TUR,
	Q10, RelTol, rec_sigma, rec_m, Flux_tol, k_migslow,
	XPB_DL, XPB_DR, XBB_DL, XBB_DR, k_PB, k_BB,
	FPB_DR, FPB_DON, FBB_DR, FBB_DON, p_NH_anad, k_roc_food,
	k_refDL, k_refDR, k_refsDL,
	albedo_ice, k_bs, k_bi, k_rs, k_ri, R_bi, k_ice, ka_star;



/* Parameters after Q10 adjustments */
extern double  r_DL, r_DR, r_DC, r_DON, r_DSi, K_nit, R_0, R_D;


/* Flags and switches */
extern int	flagtrecruitdistrib, flagresp, flagkdrop, flagbactstim,
	flagchangepop, REEFchange_max_num, FLATchange_max_num,
	SOFTchange_max_num, readin_popratio;


/****************************************************************************
Tracking values for one step per day animals that impact on multi-dt per day groups
*/
extern double RecycledNHglobal, wcFlux2global, wcFlux3global, wcFlux2aglobal, wcFlux3aglobal,
	wcFlux4global, smFlux2global, epiFlux2global, wcFishingGlobal, epiFishingGlobal;


/**************************************************************************************
Local Modelling variables
*/


extern int **recover_help, **starve_vert, **nSTOCK, **shiftVERTON, **prey_counted, *mig_returners, *active_den, *not_finished, *ngene_done, *stock_done;
//extern *mig_status;


extern double ***AGE_stock_struct_prop, // Also updated in Prepare_Age_Distrib - used to store the normalised distribution of the cohort species across each stock
	****newden, ***init_stock_struct_prop, ****recVERTpopratio,
	***shiftVERT, ***cysts, ***initVERTinfo,
	***BEDchange, ***Vchange, ***pSTOCK, ***totrecruit,
	**VERTabund_check, **sumSTOCK, **Tchange, **roc,
	**spSTOCKprop, **recSTOCK, **tot_yoy, ***VERTinfo,
	**stock_prop, **totden, **recruit_vdistrib, **totden_check,
	**tempdistrib, **PHchange, **Schange, *adults_spawning,
	*sizeMinMax, **SUPPdistrib, *recover_help_set, *BED_scale,
	***PREYinfo, ***GRAZEinfo, ***EATINGinfo, **KDENR, *yoy,
	***FEEDinfo, **step1distrib, **CATCHEATINGinfo,
	**CATCHGRAZEinfo, **boxden, ****currentden, **leftden, *newden_sum,
    ***preyamt, *totad, *totboxden, *totroc, *lostden_zero, *totsum,
    *totksum, *tot_new_mat, *coming_SPden, *numbers_entering,
    *numbers_already_present, **totdenCheck;

extern double *initialIceBiomass, *initialLandBiomass, *initialBiomass,
    *initialSedBiomass, *initialEpiBiomass, *initialWaterBiomass;

extern double ****readinpopratio;

extern int   maxMortChange;
extern int ***numMortChanges;
extern int *bleaching_has_occurred;
extern double *****LinearMortChange;

/**************************************************************************************
Defining pointers to arrays for preference and fish distribution
- the names given are the same as the parameter
names in the parameter file but with an i on the end.
*/

extern double
	*vertTchange_multi,
	*vertSchange_multi,
	*vertPHchange_multi,
	*Box_degradedi, *regIDi;

extern int     *Fluxflag;  /* Flag vector for diagonostic tracers, 1=yes, 0=no */
extern int		*Tolflag;	/* Flag vector for tolerance checking variables, 1=yes, 0=no*/
extern int		*Bioflag;   /* Flag vector for flag_id checking variables, 1=yes, 0=no*/
extern int      *Contamflag; /* Flag vector to signify a contaminant, 1=yes, 0=no*/
extern int		*Activeflag; /* Flag vector for active status (flag_id = 1) of variables, 1=yes, 0=no*/
extern int		*DiagFluxflag;  /* Diagnostics - Flag vector for diagonostic tracers, 1=yes, 0=no */
extern int		*DiagTolflag;	/* Diagnostics - Flag vector for tolerance checking variables, 1=yes, 0=no*/
extern int		*DiagBioflag;   /* Diagnostics - Flag vector for flag_id checking variables, 1=yes, 0=no*/
extern int		*DiagActiveflag; /* Diagnostics - Flag vector for active status (flag_id = 1) of variables, 1=yes, 0=no*/


extern double *****DIET_check;
/*******************************************************************
Defining keys to arrays for preference and fish distribution parameters.
These are exactly the same as the names used in the parameter file.
*/

extern char
	*pFCPIN, 	*pFCWHT , 	*pFCWHS, 
	*Box_degraded, 
	*regids;


/**
 * Indices of specific functional groups - these just make things faster - it means we don't need to go searching for
 * particular groups.
 */
extern int pelagicBactIndex;
extern int SedBactIndex;
extern int LabDetIndex;
extern int RefDetIndex;
extern int CarrionIndex;
extern int AquacultFeedIndex;
extern int IceBactIndex;

extern double ****spPreyAvail;	/** Information about prey availbility for predators. Calculated once at the start of the model and then used in Eat() */
extern char **cohortParamStrings;
extern char **spawnParamStrings;

