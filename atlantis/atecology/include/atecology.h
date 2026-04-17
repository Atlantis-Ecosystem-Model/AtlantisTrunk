/**
 *   \file atecology.h
 *   \brief atecology.h is a header file for the ecology module
 *	 \ingroup atEcology
 *	 \author Beth Fulton
 *
 *   CSIRO Marine Laboratories,  2004
 *
 *   <b>Revisions:</b>
 *
 *   8/8/2004 Created mseecom.h from the seecom.h file
 *
 *				21/8/2004 Cut back so only the biologically relevant arrays remain
 *
 *              21/9/2004 Created atecology.h from the mseecom.h file. And renamed
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
 *              30/9/2004 Added Prawns
 *
 *              1/10/2004 Moved all invertbrate structural equivalency parameters (XX_sn)
 *              to core boxmodel (e.g. CEP_sn is now bm->CEP_sn) so assessment library
 *              can access them too
 *
 *              3/10/2004 Added index names for the additional biological groups
 *				(prawns, other filter feeders, megazoobenthos, estuarine demersal fish,
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
 *				    Also removed juvCEP as now handled explicitly via cephalopod
 *					aging and migration
 *
 *				9/4/2005 Added parameters needed for new generalised form of invertebrate
 *				   equations (using Invert_Activities()). Removed the "global" doubles no
 *                 longer needed as using Global arrays now.
 *
 *				16/04/2008 Bec Gorton
 *					Removed the tracers from this file and moved them into atTracer.h
 *
 * 				18/02/2009
 * 					Changed the varname and DiagVarname arrays to string.
 *
 *				12-06-2009 Bec Gorton
 *				Added the calculateBBScale function.
 *
 *				02-02-2010 Bec Gorton
 *				Removed the TotVert array - no longer needed.
 *
 *				03-02-2010 Bec Gorton
 *				Changes to a number of function prototypes to reflect the passing around of local copies
 *				of tracer and flux values rather than using a global value stored in atEcologyModule.
 *
 *				26-02-2010 Bec Gorton
 *				Moved the function prototypes from the atbiolUtil.h file into here and deleted the atbiolUtil.h file
 *				Moved the prototypes that were in the atNutrient.h header into here as well. atNutrient.h has been deleted.
 *
 *				26-02-2010 Bec Gorton
 *				Got rid of the atEcologyModule structure. All variables that were references in that structure
 *				are now in the BoxLayerValues structure which is used to store information relevant to the current layer.
 *				The only excecption are the linkage arrays and these are now defined in this file.
 *
 *				 17-04-2013 Bec Gorton
 *				 ATL-346 - Added support of tracking P:N:C ratios in groups.
 */

#include <sjwlib.h>
#include <atlantisboxmodel.h>
#include <atUtilLib.h>
#include <atEcologyLib.h>
#include <atAdditionalTracer.h>
#include "atContaminants.h"



/**********************************************************************************
 Variables defined in main routine area
 */
extern int verbose; /** \var If true then detailed debugging information is written to the log file */

/******************************************************************************
 Modelling variables for control of processes within the model
 */

/* Migration and reproduction arrays */

extern int **recover_help, **starve_vert, **nSTOCK, **shiftVERTON, **prey_counted, *mig_returners, *active_den, *not_finished, *ngene_done, *stock_done;
//extern int *mig_status;

/* Population arrays */

extern double ****readinpopratio, ****newden, ****recVERTpopratio,
		***AGE_stock_struct_prop, ***init_stock_struct_prop,
		***BEDchange, ***Vchange, ***pSTOCK, ***initVERTinfo, ***cysts,
		***VERTinfo, **sumSTOCK, **Tchange, **roc, **spSTOCKprop,
		*sizeMinMax, **stock_prop, **recSTOCK, **totden, **recruit_vdistrib,
		**tempdistrib, **VERTabund_check, ***totrecruit, **totden_check, 
        **Schange, **tot_yoy, **KDENR, *lostden_zero, *adults_spawning,
		*recover_help_set, *BED_scale, **step1distrib,
		***PREYinfo, ***GRAZEinfo, ***EATINGinfo, ***FEEDinfo, **PHchange,
		**CATCHEATINGinfo, **CATCHGRAZEinfo, **SUPPdistrib, **boxden, ****currentden,
        **leftden, *newden_sum, ***preyamt, *totad, *totboxden, *totroc, *yoy, *totsum,
        *totksum, *tot_new_mat, *coming_SPden, *numbers_entering,
        *numbers_already_present, **totdenCheck;

extern double *initialIceBiomass, *initialLandBiomass, *initialBiomass,
        *initialSedBiomass, *initialEpiBiomass, *initialWaterBiomass;

extern int maxMortChange;
extern int ***numMortChanges;
extern int *tsRecruitsid;

extern double *****LinearMortChange;

extern BoxLayerValues *boxLayerInfo;

extern int *Fluxflag; /* Flag vector for diagonostic tracers, 1=yes, 0=no */
extern int *Tolflag; /* Flag vector for tolerance checking variables, 1=yes, 0=no*/
extern int *Bioflag; /* Flag vector for flag_id checking variables, 1=yes, 0=no*/
extern int *Contamflag; /* Flag vector to signify a contaminant, 1=yes, 0=no*/
extern int *Activeflag; /* Flag vector for active status (flag_id = 1) of variables, 1=yes, 0=no*/
extern int *DiagFluxflag; /* Diagnostics - Flag vector for diagonostic tracers, 1=yes, 0=no */
extern int *DiagTolflag; /* Diagnostics - Flag vector for tolerance checking variables, 1=yes, 0=no*/
extern int *DiagBioflag; /* Diagnostics - Flag vector for flag_id checking variables, 1=yes, 0=no*/
extern int *DiagActiveflag; /* Diagnostics - Flag vector for active status (flag_id = 1) of variables, 1=yes, 0=no*/

extern char **Varname;
extern char **DiagVarname;

extern int   maxMortChange;
extern int ***numMortChanges;

extern double *****LinearMortChange;

extern double ***VERTinfo, ***shiftVERT, *TotVERT, *Box_degradedi;

extern double *regIDi;

extern char *pFCPIN, *pFCWHT, *pFCWHS, *Box_degraded, *regids;

extern double Susp_Sed, cell_depth, DRdepth, O2depth, newO2depth, current_layer_sed,
    Turbatn_contribs, Irrig_contribs, H2Otemp, current_SALT, current_PH, init_PH,
    Bact_stim, current_depth, area_reef, area_flat, area_canyon, area_box, area_soft,
    eddy_strength, current_SALT, LocalRugosity, current_ARAG, current_WIND;

extern double tot_dyn_sea_area, Enviro_turb, BioirrigEnh, BioturbEnh;

extern int numwcvar, numepivar, numlandvar, numdiagvar, numfstatvar, numicevar, first_year, idum;

/* Flags and switches */
extern int flagtrecruitdistrib, flagresp, flagkdrop, flagbactstim, flagchangepop, REEFchange_max_num, FLATchange_max_num, SOFTchange_max_num, readin_popratio;

/********************************************************************************
 Model  Parameters
 */

extern double *FDM;
extern int Tchange_max_num, Schange_max_num, PHchange_max_num, BEDchange_max_num, Vchange_max_num;

extern double Fben_den, FDL_wc, MB_wc, FDL_benth, FDL_fish, FDL_SG_roots, FDL_SG_leaves, FDL_top, p_linePIN, p_netPIN, p_trawlPIN, p_mtrawlPIN, p_lineWHT, p_BBfish, p_BBben, p_PBwc, p_PBben, p_IBice,
		recover_span, recover_trigger, recover_subseq, lognorm_mu, p_lineWHS, lognorm_sigma, roc_wgt, Kthresh1, Kthresh2, KHTD, KHTI,
		Kthreshm, r_DL_T15, r_DR_T15, r_DON_T15, r_DSi_T15, r_DC_T15, FFDDR, FDR_DL, FDON_D, FDR_DC, KST_fish, KST_shark, KST_bird, KST_mammal, Ktmp_fish,
		Ktmp_shark, Ktmp_bird, Ktmp_mammal, R_0_T15, R_D_T15, Dmax, k_w_depth, k_w_cdepth, k_w_deep, k_w_shallow, k_PN,
		k_DON, k_DL, k_IS, K_TUR_DEP, k_SED, K_addepth, K_conc, K_nit_T15, KI_avail, KIOP_min, KIOP_shift, K_IRR, K_MAX_IRR, K_MAX_TUR, K_MIN_IRR,
		K_TUR, Q10, RelTol, rec_sigma, rec_m, Flux_tol, k_migslow, XPB_DL, XPB_DR, XBB_DL, XBB_DR, k_PB, k_BB, FPB_DR, FPB_DON, FBB_DR, FBB_DON, p_NH_anad,
		k_roc_food, k_refDL, k_refDR, k_refsDL,
		albedo_ice, k_bs, k_bi, k_rs, k_ri, R_bi, k_ice, ka_star;

extern int *bleaching_has_occurred;


extern double r_DL, r_DR, r_DC, r_DON, r_DSi, K_nit, R_0, R_D;

extern double *vertTchange_multi, *vertSchange_multi, *vertPHchange_multi, *Box_degradedi;

/* Flags and switches */
extern int flagresp, flagkdrop;

/****************************************************************************
 Tracking values for one step per day animals that impact on multi-dt per day groups
 */
extern double RecycledNHglobal, wcFlux2global, wcFlux3global, wcFlux2aglobal, wcFlux3aglobal, wcFlux4global, smFlux2global, epiFlux2global, wcFishingGlobal,
		epiFishingGlobal;
/****/
extern double predayt;
extern double sporosity;
extern double surf_stress; /* stress on bottom */
extern double wcLayerThick, smLayerThick; /* depth of wc and sm layer */

/**
 * Indices of specific functional groups - these just make things faster - it means we don't need to go searching for
 * particular groups.
 */
//extern int pelagicBactIndex;
//extern int SedBactIndex;
//extern int LabDetIndex;
//extern int RefDetIndex;
//extern int CarrionIndex;

extern double *****DIET_check;

/*
 *
 */
extern double ****spPreyAvail; /** Information about prey availbility for predators. Calculated once at the start of the model and then used in Eat() */


/**************************************************************************
 Function prototypes
 */

/* For setup */
void Create_Update_XRS(MSEBoxModel *bm, FILE *llogfp);
void Create_Migration_Arrays(MSEBoxModel *bm, FILE *llogfp);
void Zero_Create_Migration_Arrays(MSEBoxModel *bm, FILE *llogfp);
void Convert_Biol_To_XML(MSEBoxModel *bm, char *fileName, char *outputFileName);
void Check_Migration(MSEBoxModel *bm);
void Check_Migration_Data(MSEBoxModel *bm, int speciesIndex, int stageIndex);
void Init_Migration(MSEBoxModel *bm, FILE *llogfp, int do_debug, int sp);
void Init_Migration_Age_Check(MSEBoxModel *bm, FILE *llogfp, int do_debug, int sp);

int Util_Read_Migration_XML(MSEBoxModel *bm, char *fileName, FILE *llogfp);
int Read_Biology_Parameters(MSEBoxModel *bm, char *filename, FILE *llogfp);
int Read_Biology_Stock_Parameters(MSEBoxModel *bm, char *filename);
int Read_Migration_Information(MSEBoxModel *bm, char *fileName, xmlNodePtr parent, FILE *llogfp);

/*  For main biology code and daily migration code */
void Check_Gape(MSEBoxModel *bm, FILE *llogfp);
double Get_Proportion_Aging(MSEBoxModel *bm, int species, int cohort, int do_debug, FILE *llogfp);

void Sediment_Box(MSEBoxModel *bm, double dtsz, BoxLayerValues *boxLayerInfo, FILE *llogfp);
void Water_Column_Box(MSEBoxModel *bm, double dtsz, BoxLayerValues *boxLayerInfo, FILE *llogfp);
void Epibenthic_Box(MSEBoxModel *bm, double dtsz, BoxLayerValues *boxLayerInfo, FILE *llogfp);
void Ice_Box(MSEBoxModel *bm, double dtsz, BoxLayerValues *boxLayerInfo, FILE *llogfp);

void Calculate_Catch(MSEBoxModel *bm, BoxLayerValues *boxLayerInfo, FILE *llogfp, int guild, int cohort, double SN, double RN, double NUMS, double propSediment, double propWater);
void Get_Vertical_Distribution(MSEBoxModel *bm, int ij, int species, double ****currentden, int enviro_depend, int day_part, int cohort, FILE *llogfp);

double Get_Species_Area_Hab(MSEBoxModel *bm, int guild, int cohort, BoxLayerValues *boxLayerInfo);

/* Vertebrate prototypes */
void Do_Aging_Update_Trait(MSEBoxModel *bm, int species, FILE *llogfp);
void Do_Evolution(MSEBoxModel *bm, int species, FILE *llogfp);
void Do_Phenotyic_Expression(MSEBoxModel *bm, int species, FILE *llogfp);
void Evolution_Curve(MSEBoxModel *bm, int species, int stock_id, int qid, int do_debug, FILE *llogfp);
void Find_Evolution_Stats(MSEBoxModel *bm, int sp, FILE *llogfp, int init_case);
void Find_Evolution_Shift(MSEBoxModel *bm, int sp, FILE *llogfp);
void Set_VERTinfo(MSEBoxModel *bm, double *localWCTracers, FILE *llogfp);
void Vertebrate_Reproduction(MSEBoxModel *bm, int wclayer, int maxdeep, int totaldeep, int nz, double *localWCTracers, FILE *llogfp);
void Vertebrate_Assess_Enviro(MSEBoxModel *bm, int guildcase, int boxin, double **cover_info, double cdepth, int rugosity_check_done, FILE *llogfp);

double Do_Vertebrate_Living(MSEBoxModel *bm, FILE *llogfp, int guildcase, HABITAT_TYPES level_id, BoxLayerValues *boxLayerInfo, double DLsed, double DRsed, double DCsed,
		double ***spPREYinfo, double ***spGRAZEinfo, double **spCATCHGRAZEinfo, double ***spSPinfo);

int Do_Inheritance(MSEBoxModel *bm, int species, int ngene, int basecohort, int cohort, FILE *llogfp);

/* Invertebrate proptypes */
void Init_Invert_Properties(MSEBoxModel *bm);
void Invertebrate_Reproduction(MSEBoxModel *bm, int wclayer, int maxdeep, int totaldeep, double *localWCTracers, FILE *llogfp);
void Invert_Activities(MSEBoxModel *bm, BoxLayerValues *boxLayerInfo, HABITAT_TYPES habitatType, FILE *llogfp,  int guild, int cohort, double SP, double area_hab, double E_SP, double EDL_SP, double EDR_SP, double bact_DL,
		double bact_DR, double sedbact_DL, double sedbact_DR, double PB_scale, double BB_scale, double mL_other, double FDL_SP, double DL, double DR,
		double DLsed, double DRsed, double ***spGRAZEinfo);

void Calculate_PelagicBact_Scale(MSEBoxModel *bm, BoxLayerValues *boxLayerInfo);
void Calculate_SedBact_Scale(MSEBoxModel *bm, HABITAT_TYPES habitatType, BoxLayerValues *boxLayerInfo);

/* Error checking */
void Call_Diagnostics(MSEBoxModel *bm, BoxLayerValues *boxLayerInfo, int level_id, FILE *llogfp, int qnancall);
void Max_Flux_Check(MSEBoxModel *bm, int callerid, int *fluxsp, int *fluxid, int *fluxpool, int *fluxn, double *localWCFlux, double *localSEDFlux,
		double *localEPIFlux, FILE *llogfp);

void Print_Flux(MSEBoxModel *bm, int level_id, double ***spSPinfo, int qnancheck_id, double wcFlux, double wcFlux1, double wcFlux2, double wcFlux3,
		double wcFlux4, double smFlux, double epiFlux, double TotFlux, double FishingFlux, double *localWCFlux, double *localSEDFlux, double *localEPIFlux,
		FILE *llogfp);
void Print_Eat_Diagnostics(MSEBoxModel *bm, FILE *llogfp, int guild, HABITAT_TYPES habitatType, int diagType);

/* Record output prototypes */
void Open_Ecology_Output_Files(MSEBoxModel *bm);
void Close_Ecology_Output_Files(MSEBoxModel *bm);
void Free_CoralReef(MSEBoxModel *bm);
void Free_Embryo(MSEBoxModel *bm);
void Free_Evolution(MSEBoxModel *bm);
void Free_Migration(MSEBoxModel *bm);

/* Biophysics */
double Oxygen(int O2_case, double oxy, double lethal, double lim, double mD_sp);
double pH_At_Depth(MSEBoxModel *bm, Box *pBox, double dayt, int numwclayer, double cdepth, int clayer,
		double midpoint, int flagmodel, FILE *llogfp);
void Apply_Q10_Corrections(MSEBoxModel *bm);
void Avail(int aerob_case, double sp, double Depth, double *avail_sp);
void Properties_At_Depth(MSEBoxModel *bm, Box *pBox, double dayt, int numwclayer, double cdepth, int clayer, double midpoint,
		int flagmodel, FILE *llogfp);
void Irrig_and_Turb(MSEBoxModel *bm, double *Irrig, double *Turb);
void Calculate_Box_Light(MSEBoxModel *bm, Box *pBox, FILE *llogfp);
void Box_Light_Process(MSEBoxModel *bm, Box *pBox, FILE *llogfp);
void Box_O2_Depth_Process(MSEBoxModel *bm, Box *pBox);
void Box_Nutrient_Check(MSEBoxModel *bm, Box *pBox);

/* General ecological subroutine prototypes */
double Get_pHcorr(MSEBoxModel *bm, int sp, double current_pH, int cbox, int clayer);
double Get_Scorr(MSEBoxModel *bm, int sp, double current_salt);
double Get_Tcorr(MSEBoxModel *bm, int sp, double current_temp, double *current_corr);
double Get_Pollutant_Corrections(MSEBoxModel *bm, int sp, int b, int clayer);

double Projection_GetLatitude(MSEBoxModel *bm, double x_coord, double y_coord);
double Projection_GetLongitude(MSEBoxModel *bm, double x_coord, double y_coord);

double Acidif_Mort(MSEBoxModel *bm, int species);

double Ecology_Age_Structured_Spawn(int species, double KSPA_SP, double FSP_SP, double RSprop_SP, double SN, double RN, double FSPB_SP, double X_RS, int repcost, FILE *llogfp);

void Update_Migration_Index(MSEBoxModel *bm, FILE *llogfp);

/* General biophysical subroutine prototypes */
void Calculate_Box_Biomass(MSEBoxModel *bm, FILE *llogfp, int ij, int nreg, int isInitPops, int isDiagnostic);
void Parameter_Q10(MSEBoxModel *bm, Box *pBox, double dayt, int numwclayer, double cdepth, int clayer, double midpoint, int flagmodel, FILE *llogfp);

/* Growth subroutine prototypes */
void Primary_Production(MSEBoxModel *bm, FILE *llogfp, int sp_id, int micro_case, int lim_case, int macro_producer, double sp_biom,
		double DIN, double NH, double NO, double Si, double Fe, double P, double PRatio, double C, double CRatio,
		double IRR, double mum, double E_sp, double mL_other, double SPmax, double area_hab, double *spUptakeNO,
		double *spUptakeSi, double *spUptakeFe, double *spUptakeP, double *spUptakeC, double *sphN);

void Grow_Macrophytes(MSEBoxModel *bm, FILE *llogfp, int sp_id, int cohort, int micro_case, int lim_case, int macro_producer,
		double sp_biom, double sp_biom_sed, double sp_biom_epi,
		double DINs, double NHs, double NOs, double DIN, double NH, double NO, double Si, double Fe, double P, double C, double IRR,
		double E_sp, double mL_other, double SPmax, double area_hab,
		double *spUptakeNO, double *sphN, double PRatio, double CRatio, double *spUptakeP, double *spUptakeC);

double Light_Lim(MSEBoxModel *bm,  int light_case, double amt_irr, double KI_sp, double opt_irr, double prop_daylight, double PBmax_D);
double Nutrient_Lim(MSEBoxModel *bm, int nut_case, int micro_case, double amt_din, double amt_si, double amt_fe, double amt_p, double PRatio, double P0, double P1,
		double KN_sp, double KS_sp, double KF_sp, double KP_sp);


/* Feeding related subroutine prototypes */
double Avail_Fish(MSEBoxModel *bm, int guildcase, int chrt, int chrtstage, int prey, int bpreychrt, double SN, double ***SP, FILE *llogfp);
double Avail_Catch(MSEBoxModel *bm, int guildcase, int chrt, int chrtstage, int prey, int bpreychrt, double SN, double ***SP, FILE *llogfp);
double Get_Catch_Prey(MSEBoxModel *bm, FILE *llogfp, int predatorID, int cohort, int chrtstage, int preyID, int prey_chrt, int habitat);
double Get_Gape_Lim_Prey(MSEBoxModel *bm, FILE *llogfp, int predatorID, int cohort, int chrtstage, int preyID, int prey_chrt, int habitat, double ***spPREYinfo);

void Partition_Weight(MSEBoxModel *bm, int sp, double pR_SP, double SN, double RN, double X_RS, double *FRC_sp, FILE *llogfp);
void Partition_Weight_Dynamic(MSEBoxModel *bm, int sp, double SN, double RN, double X_RS, double *FRC_sp, double avail_intake, FILE *llogfp);
void Fish_Respiration(MSEBoxModel *bm, int guildcase, double SN, double RN, double X_RS, double Dens, double *respire);
void Fish_Respiration_DEB(MSEBoxModel *bm, int guildcase, double SN, double RN, double X_RS, double Dens, double *respire);

void Calculate_Sediment_Prey_Avail(MSEBoxModel *bm, BoxLayerValues *boxLayerInfo, int guild, double ***spPREYinfo, double *avail_BB);
void Construct_Prey_Info(MSEBoxModel *bm, FILE *llogfp, BoxLayerValues *boxLayerInfo, int habitat_type);
void Do_Extra_Feed(MSEBoxModel *bm, FILE *llogfp, int sp_id, int flagcase, double CLEAR, double mum_sp, double E_sp, double scalar, double tprey, double ***spGRAZEinfo);
void Eat(MSEBoxModel *bm, FILE *llogfp, int flagcase, int sp_id, int cohort, double sp, double C_sp, double mum_sp, double KL_sp, double KU_sp, double vl_sp,
		double ht_sp, double E1_sp, double E2_sp, double EDL_sp, double EDR_sp, int sp_feed_while_spawn, int sp_spawn_now, double chrt_mat,
		double ***spPREYinfo, double ***spGRAZEinfo, double **spCATCHGRAZEinfo, double sp_Biomass);
void Get_Aquacult_Feed(MSEBoxModel *bm, FILE *llogfp, int sp_id, double sp_Biomass, int flagcase, double KL_sp, long double *living_prey,
		long double *living_prey_sq, double *denom_step);
void Update_GrazeInfo(MSEBoxModel *bm, FILE *llogfp, int flagcase, int sp_id, int cohort, int preyID, int kij, int catcheater,
		long double *graze_live, double ***spPREYinfo, double ***spGRAZEinfo, double **spCATCHGRAZEinfo);
void UpdateTrackedMort(MSEBoxModel *bm, FILE *llogfp, int guildcase, int cohort, HABITAT_TYPES level_id, HABITAT_TYPES habitat, int prey, int prey_chrt,
		BoxLayerValues *boxLayerInfo, double scalar, int global_contrib);

void Encystment(MSEBoxModel *bm, FILE *llogfp, int sp_id, double sp_biom, double DIN, double *sp_grow, double *cyst_transfer);
double Check_OverWintering(MSEBoxModel *bm, FILE *llogfp, int sp_id);

/* Ice related routines */
void Box_Ice_Flux(MSEBoxModel *bm, Box *pBox, FILE *llogfp);
void Box_Ice_Temperature_Related(MSEBoxModel *bm, Box *pBox, FILE *llogfp);
void Box_Ice_Light_Level(MSEBoxModel *bm, Box *pBox, FILE *llogfp);
void Box_Ice_Q10(MSEBoxModel *bm, Box *pBox, int ice_layer, FILE *llogfp);


/* Functions associated with building and freeing the tracer arrays */
void Tracer_Array_Free(MSEBoxModel *bm);
void Epi_Tracer_Array_Free(MSEBoxModel *bm);
void Land_Tracer_Array_Free(MSEBoxModel *bm);
void Build_TrName_NameList(MSEBoxModel *bm);
void Build_Diagnostic_NameList(MSEBoxModel *bm);
void Build_EPI_NameList(MSEBoxModel *bm);
void Build_LAND_NameList(MSEBoxModel *bm);

/* The biological process and change of equation function declarations */
int Invert_Consumers_Process(MSEBoxModel *bm, FILE *llogfp, HABITAT_TYPES habitatType, int guild, int cohort, BoxLayerValues *boxLayerInfo);
int Phytoplankton_Process(MSEBoxModel *bm, FILE *llogfp, HABITAT_TYPES habitatType, int guild, int cohort, BoxLayerValues *boxLayerInfo);
int Dinoflag_Process(MSEBoxModel *bm, FILE *llogfp, HABITAT_TYPES habitatType, int guild, int cohort, BoxLayerValues *boxLayerInfo);
int Coral_Process(MSEBoxModel *bm, FILE *llogfp, HABITAT_TYPES habitatType, int guild, int cohort, BoxLayerValues *boxLayerInfo);

int Pelagic_Bacteria_Process(MSEBoxModel *bm, FILE *llogfp, HABITAT_TYPES habitatType, int guild, int cohort, BoxLayerValues *boxLayerInfo);
int Ice_Bacteria_Process(MSEBoxModel *bm, FILE *llogfp, HABITAT_TYPES habitatType, int guild, int cohort, BoxLayerValues *boxLayerInfo);

int Sediment_Bacterica_Process(MSEBoxModel *bm, FILE *llogfp, HABITAT_TYPES habitatType, int guild, int cohort, BoxLayerValues *boxLayerInfo);
int Epibenthic_Invert_Process(MSEBoxModel *bm, FILE *llogfp, HABITAT_TYPES habitatType, int guild, int cohort, BoxLayerValues *boxLayerInfo);
int Sediment_Epi_Other_Process(MSEBoxModel *bm, FILE *llogfp, HABITAT_TYPES habitatType, int guild, int cohort, BoxLayerValues *boxLayerInfo);

int Labile_Detritus_Process(MSEBoxModel *bm, FILE *llogfp, HABITAT_TYPES habitatType, int guild, int cohort, BoxLayerValues *boxLayerInfo);
int Refractory_Detritus_Process(MSEBoxModel *bm, FILE *llogfp, HABITAT_TYPES habitatType, int guild, int cohort, BoxLayerValues *boxLayerInfo);
int Carrion_Process(MSEBoxModel *bm, FILE *llogfp, HABITAT_TYPES habitatType, int guild, int cohort, BoxLayerValues *boxLayerInfo);

/*
 * The nutrient process functions.
 */
void Detrital_Silica_ROC(MSEBoxModel *bm, FILE *llogfp, HABITAT_TYPES habitatType, int index, BoxLayerValues *boxLayerInfo);
void Refractory_Detritus_ROC(MSEBoxModel *bm, FILE *llogfp, HABITAT_TYPES habitatType, BoxLayerValues *boxLayerInfo);
void Labile_Detritus_ROC(MSEBoxModel *bm, FILE *llogfp, HABITAT_TYPES habitatType, BoxLayerValues *boxLayerInfo);
void Carrion_ROC(MSEBoxModel *bm, FILE *llogfp, HABITAT_TYPES habitatType, BoxLayerValues *boxLayerInfo);

void Ammonium_ROC(MSEBoxModel *bm, FILE *llogfp, HABITAT_TYPES habitatType, int index, BoxLayerValues *boxLayerInfo);
void Nitrate_ROC(MSEBoxModel *bm, FILE *llogfp, HABITAT_TYPES habitatType, int index, BoxLayerValues *boxLayerInfo);
void Micronutrient_ROC(MSEBoxModel *bm, FILE *llogfp, HABITAT_TYPES habitatType, int index,  BoxLayerValues *boxLayerInfo);
void DON_ROC(MSEBoxModel *bm, FILE *llogfp, HABITAT_TYPES habitatType, int index,  BoxLayerValues *boxLayerInfo);
void Oxygen_ROC(MSEBoxModel *bm, FILE *llogfp, HABITAT_TYPES habitatType, int index,  BoxLayerValues *boxLayerInfo);
void Dissolved_Silica_ROC(MSEBoxModel *bm, FILE *llogfp, HABITAT_TYPES habitatType, int index,  BoxLayerValues *boxLayerInfo);

void Carbon_ROC(MSEBoxModel *bm, FILE *llogfp, HABITAT_TYPES habitatType, int index, BoxLayerValues *boxLayerInfo);
void Phosphorus_ROC(MSEBoxModel *bm, FILE *llogfp, HABITAT_TYPES habitatType, int index, BoxLayerValues *boxLayerInfo) ;
void TOP_ROC(MSEBoxModel *bm, FILE *llogfp, HABITAT_TYPES habitatType, int index, BoxLayerValues *boxLayerInfo) ;

double *getTracerArray(BoxLayerValues *boxLayerInfo, HABITAT_TYPES habitatType);

/* Functions related to larval dispersion */
int Larval_Dispersal(MSEBoxModel *bm, int sp, FILE *llogfp);
int Load_Imposed_Recruits(MSEBoxModel *bm, FILE *llogfp);
void Free_Imposed_Recruits(MSEBoxModel *bm);
double Get_Recruit_Size_Forcing(MSEBoxModel *bm, int species, int do_debug, FILE *llogfp);
double Get_Imposed_MultRecruits(MSEBoxModel *bm, int guildcase, int do_debug, int boxkey_id, FILE *llogfp);

void 	Setup_Linear_Mortality_Indices(MSEBoxModel *bm) ;
void 	Setup_Size_Change_Indices(MSEBoxModel *bm) ;
void 	Setup_Change_Indices(MSEBoxModel *bm, TimeSeries *ts, int index);
double  Get_Biomass_Correction(MSEBoxModel *bm, int sp, HABITAT_TYPES habitatType);
void 	Check_Layer_Initial_Biomass(MSEBoxModel *bm);

/* Evolution reporting functions */
FILE * Init_Evol_File(MSEBoxModel *bm);
void Write_Evolution(FILE *fid, MSEBoxModel *bm, FILE *llogfp);

/* Coral related routines */
void Box_Rugosity(MSEBoxModel *bm, Box *pBox, int rugosity_check_done, FILE *llogfp);
void BoundRugosity(MSEBoxModel *bm, double *tracerArray);
void Coral_Consumer_Activities(MSEBoxModel *bm, HABITAT_TYPES habitatType, FILE *llogfp, int guild, int cohort, double SP, double SPmax, double IRR,
		double area_hab, double E_SP, double EDL_SP, double EDR_SP, double bact_DL, double bact_DR, double sedbact_DL, double sedbact_DR,
		double PB_scale, double BB_scale, double mL_other, double FDL_SP, double DL, double DR, double DLsed, double DRsed, double Si, double ***spGRAZEinfo);
void Coral_Limiting_Growth_Factors(MSEBoxModel *bm, FILE *llogfp, int guild, int cohort, double sed_level);
double Calculate_Rugosity(MSEBoxModel *bm, int guild, int cohort, FILE *llogfp, int sp_level_calc);
double Coral_Space_Competition(MSEBoxModel *bm, BoxLayerValues *boxLayerInfo, FILE *llogfp, int guild, int cohort, double mum);
double Coral_Variable_Transitions(MSEBoxModel *bm, int species, int cohort, int do_debug, FILE *llogfp);
void Destroy_Rugosity(MSEBoxModel *bm, BoxLayerValues *boxLayerInfo, FILE *llogfp, int guild, int cohort, int fishery, double dead_biom);
void Do_Bleaching(MSEBoxModel *bm, FILE *llogfp, int guild, int cohort, double sp_biom);
double Get_Habitat_Mediated_Recruit_Scalar(MSEBoxModel *bm, int species, FILE *llogfp);
void Grow_Coral_Symbionts(MSEBoxModel *bm, BoxLayerValues *boxLayerInfo, FILE *llogfp, int guild, int cohort, int lim_case, int micro_case, double sp_biom,
		double DIN, double NH, double NO, double Si, double Fe, double P, double PRatio, double C, double CRatio,
		double IRR, double mum, double E_sp, double mL_other, double SPmax, double FDL_SP, double area_hab, double *spUptakeNO,
		double *spUptakeSi, double *spUptakeFe, double *spUptakeP,  double *spUptakeC, double *sphN);

double Ecology_Get_Growth_Scalar(MSEBoxModel *bm, int sp, int cohort);
double Ecology_Get_Recruitment_Scalar(MSEBoxModel *bm, int sp);

/* Ice related prototypes */
void Ice_HabitatState(MSEBoxModel *bm, FILE *llogfp);

void Ice_PrimaryProduction(MSEBoxModel *bm, FILE *llogfp, int sp_id, int micro_case, int lim_case, int macro_producer, double sp, double DIN, double NH,
		double NO, double Si, double Fe, double P, double PRatio, double IRR, double pH, double E_sp, double mL_other, double SPmax, double area_hab, double *spUptakeNO, double *spUptakeSi,
		double *spUptakeFe, double *sphN);

double 	Get_Ice_Presence(MSEBoxModel *bm, int sp, int stage, int ij, int k, int ***HABlike);
double  Get_Ice_Rating(MSEBoxModel *bm, int sp);
double 	Get_Ice_Vertebrate_Habitat_Rating(MSEBoxModel *bm, int guildcase, int stage, int boxin);
void 	Calculate_IceBact_Scale(MSEBoxModel *bm, HABITAT_TYPES habitatType, BoxLayerValues *boxLayerInfo);
void 	Calculate_Ice_Prey_Avail(MSEBoxModel *bm, BoxLayerValues *boxLayerInfo, int guild, double ***spPREYinfo, double *avail_Ice_Bact);

/* Land based processes */
void Land_HabitatState(MSEBoxModel *bm);
void Land_PrimaryProduction(MSEBoxModel *bm);
void Land_Biology_Process(MSEBoxModel *bm);
void Copy_WC_Tracers(MSEBoxModel *bm, double *localWCTracers, double *localWCFlux, FILE *llogfp);


