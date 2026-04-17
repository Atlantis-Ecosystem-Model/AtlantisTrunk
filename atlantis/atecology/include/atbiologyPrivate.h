#ifndef ATBIOLOGYPRIVATE_H_
#define ATBIOLOGYPRIVATE_H_


/**************************************************************************************
Local Modelling variables
*/

char **Varname;
char **DiagVarname;

double   sporosity;
double   surf_stress;   /* stress on bottom */
double   wcLayerThick, smLayerThick, iceLayerThick;  /* depth of wc and sm layer */



/************************************************************************
 Modelling variables for control of processes within the model
*/

double Susp_Sed, tot_dyn_sea_area,
	DRdepth, O2depth, newO2depth, Enviro_turb, current_layer_sed, eddy_strength,
	BioirrigEnh, BioturbEnh, Turbatn_contribs, Irrig_contribs, cell_depth,
	H2Otemp, current_SALT, current_PH, init_PH, Bact_stim, current_depth, area_reef, area_flat, LocalRugosity,
	area_canyon, area_box, area_soft, current_SALT, current_ARAG, current_WIND;

int numwcvar, numepivar, numlandvar, numdiagvar, numfstatvar, numicevar, first_year, idum;

/* defining sizes for arrays for preference and fish distribution parameters*/
int BEDchange_max_num, Tchange_max_num, Schange_max_num, PHchange_max_num, Vchange_max_num;
/**************************************************************************
Model  Parameters
*/
double
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
double  r_DL, r_DR, r_DC, r_DON, r_DSi, K_nit, R_0, R_D;


/* Flags and switches */
int	flagtrecruitdistrib, flagresp, flagkdrop, flagbactstim,
	flagchangepop, REEFchange_max_num, FLATchange_max_num,
	SOFTchange_max_num, readin_popratio;


/****************************************************************************
Tracking values for one step per day animals that impact on multi-dt per day groups
*/
double RecycledNHglobal, wcFlux2global, wcFlux3global, wcFlux2aglobal, wcFlux3aglobal,
	wcFlux4global, smFlux2global, epiFlux2global, wcFishingGlobal, epiFishingGlobal;


/**************************************************************************************
Local Modelling variables
*/


int **recover_help = 0, **starve_vert = 0, **nSTOCK = 0, **shiftVERTON = 0, **prey_counted = 0, *mig_returners = 0, *active_den = 0, *not_finished = 0, *ngene_done = 0, *stock_done = 0;
//int *mig_status = 0;

double ***AGE_stock_struct_prop = 0, // Also updated in Prepare_Age_Distrib - used to store the normalised distribution of the cohort species across each stock
	****newden, ***init_stock_struct_prop = 0, ****recVERTpopratio = 0,
	***shiftVERT = 0, ***cysts = 0, ***initVERTinfo = 0,
	***BEDchange = 0, ***Vchange = 0, ***pSTOCK = 0, ***totrecruit = 0,
	**VERTabund_check = 0, **sumSTOCK = 0, **Tchange = 0, **roc = 0,
	**spSTOCKprop = 0, **recSTOCK = 0, **tot_yoy = 0, ***VERTinfo = 0,
	**stock_prop = 0, **totden = 0, **recruit_vdistrib = 0, **totden_check = 0,
	**tempdistrib = 0, **PHchange = 0, **Schange = 0,
	*sizeMinMax = 0, **SUPPdistrib = 0, *adults_spawning = 0,
	*recover_help_set = 0, *BED_scale = 0, **KDENR = 0, *yoy = 0,
	***PREYinfo = 0, ***GRAZEinfo = 0, ***EATINGinfo = 0, *lostden_zero = 0,
	***FEEDinfo = 0, **step1distrib = 0, **CATCHEATINGinfo = 0,
	**CATCHGRAZEinfo = 0, **boxden = 0, ****currentden = 0, **leftden = 0,
    *newden_sum = 0, ***preyamt = 0, *totad = 0, *totboxden = 0, *totroc = 0,
    *totsum = 0, *totksum = 0, *tot_new_mat = 0, *coming_SPden = 0,
    *numbers_entering = 0, *numbers_already_present = 0,
    **totdenCheck = 0;

double *initialIceBiomass = 0, *initialLandBiomass = 0, *initialBiomass = 0,
    *initialSedBiomass = 0, *initialEpiBiomass = 0, *initialWaterBiomass = 0;

double ****readinpopratio = 0;

int   maxMortChange;
int ***numMortChanges;
int *bleaching_has_occurred;
double *****LinearMortChange = 0;

BoxLayerValues *boxLayerInfo;

/**************************************************************************************
Defining pointers to arrays for preference and fish distribution
- the names given are the same as the parameter
names in the parameter file but with an i on the end.
*/

double
	*vertTchange_multi,
	*vertSchange_multi,
	*vertPHchange_multi,
	*Box_degradedi, *regIDi;

int     *Fluxflag;  /* Flag vector for diagonostic tracers, 1=yes, 0=no */
int		*Tolflag;	/* Flag vector for tolerance checking variables, 1=yes, 0=no*/
int		*Bioflag;   /* Flag vector for flag_id checking variables, 1=yes, 0=no*/
int     *Contamflag;  /* Flag vector to signify a contaminant, 1=yes, 0=no*/
int		*Activeflag; /* Flag vector for active status (flag_id = 1) of variables, 1=yes, 0=no*/
int		*DiagFluxflag;  /* Diagnostics - Flag vector for diagonostic tracers, 1=yes, 0=no */
int		*DiagTolflag;	/* Diagnostics - Flag vector for tolerance checking variables, 1=yes, 0=no*/
int		*DiagBioflag;   /* Diagnostics - Flag vector for flag_id checking variables, 1=yes, 0=no*/
int		*DiagActiveflag; /* Diagnostics - Flag vector for active status (flag_id = 1) of variables, 1=yes, 0=no*/


double *****DIET_check;
/*******************************************************************
Defining keys to arrays for preference and fish distribution parameters.
These are exactly the same as the names used in the parameter file.
*/

char
	*pFCPIN = "pFCPIN",
	*pFCWHT = "pFCWHT",
	*pFCWHS = "pFCWHS",
	*Box_degraded = "Box_degraded",
	*regids = "regID";


/**
 * Indices of specific functional groups - these just make things faster - it means we don't need to go searching for
 * particular groups.
 */
int pelagicBactIndex;
int SedBactIndex;
int LabDetIndex;
int RefDetIndex;
int CarrionIndex;
int AquacultFeedIndex;
int IceBactIndex;

double ****spPreyAvail;	/** Information about prey availbility for predators. Calculated once at the start of the model and then used in Eat() */
//char **cohortParamStrings;
//char **spawnParamStrings;
#endif /* ATBIOLOGYPRIVATE_H_ */
