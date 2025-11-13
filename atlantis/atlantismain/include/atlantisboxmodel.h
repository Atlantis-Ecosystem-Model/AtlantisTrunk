/*********************************************************************

 File:           atlantisboxmodel.h

 Created:        Tue Sep 21 12:31:01 EST 2004

 Author:         Beth Fulton
 CSIRO Marine Research

 Purpose:        Include file for general purpose Box Model

 Revisions:      8/8/2004 mseboxm.h created from seboxm.h

 9/8/2004 All _ids moved here as defined constants

 12/8/2004 SP_prms array and other parameters arrays
 for biological, fisheries and management moved here
 from individual libraries.

 20/9/2004 Added assessment id numbers to definition section

 21/9/2004 Created msegenboxm.h from mseboxm.h. And renamed
 FSL, FSO, FSF, FST and FSG to generic names so apply to
 other models/generic case.
 FSL -> FDC
 FSO -> FDO
 FSF -> FDB
 FST -> FVO
 FSG -> SHB

 25/9/2004 Added extra id constants for new groups

 1/10/2004 Moved all invertbrate structural equivalency parameters (XX_sc)
 to core boxmodel (e.g. CEP_sc is now bm->CEP_sc) so assessment library
 can access them too

 23/10/2004 Added midwcPWN and additional fisheries links

 19/1/2005 Added Microphytobenthos (MB)

 28/3/2005 Added bm->do_availflag so that can do checks for active
 groups if only want active groups advected (so rounding errors
 don't add up to non-negligible flows in backwaters)

 29/3/2005 Added DIN_id etc so can track "stock size" of DIN

 8/4/2005 Added Invert_Activities() to handle mortality, growth,
 assimilation and waste equations in a general way

 9/4/2005 Added 	bm->INVERT_PREYeatenGlobal, bm->NutsProdGlobal,
 bm->NutsLostGlobal, bm->DetritusProdGlobal, bm->DetritusLostGlobal,
 bm->NutsProd, bm->NutsLost, bm->DetritusProd, bm->DetritusLost,
 created to simplify handling of handlgin of values spanning the
 it_count == 1 and subsequent iterations of the adaptive timestep.
 K_num_global_nonliving also added to facilitate loops through these
 vectors..

 10/4/2005 Added Primary_Production() to handle all primary production.
 Also added one_nut_lim to allow differentiation of those primary
 producers using one nutrient for limitation and those using
 multiple nutrients

 24/4/2005 Added constrain_wc

 13/7/2005 Added readkeyprm_darray_checked() and extended readValue()
 so that knows what data is expected so that can check on readin
 for obvious parameter errors

 19/7/2005 Added BDcat_id and MAcat_id and increased Not_fished_id from
 44 to 45 so BD now explicitly included in fished list.

 16/8/2005 Added stock_struct array, read-in type id numbers (no_checking,
 binary_check, proportion_check, probability_check, integer_check). Also
 added id numbers for some economics parameters

 24/9/2005 Added DebugFluxInfo and which_check and extra guild ids so can track
 specfic fluxes through time (helps in calibration with less fprintf
 statements

 25/9/2005 Added eflux and vflux to boxes data structure so can store flux values.
 Also added env_force time series (so can force recruitment with a TimeSeries
 file).

 22/12/2005 Added recruitRange params for re-written vertebrate aging


 17/04/2008 Bec Gorton
 Added a reference to atTracer.h.
 Also removed the move and migrate indices. These were used to
 refernence invertebrate arrays - they are no longer needed as the migration
 arrays have been resized to allow storage for all species regardless of if they
 migrate or not. A flag is now simply checked before the arrays are referenced.

 12/05/2008 Bec Gorton
 Added declarations of _isnan, _isfinite and isspace for linux.

 29/09/2008 Bec Gorton
 Removed the bm->sp_parms array. These are now stored in speciesParams and cohortSpeciesParams in the
 FunctGroup structure.
 Changed the parameters indices to be an enum that excluded the cohort dep parameters.
 These have been moved into a seperate cohort.

 19/10/2008 Bec Gorton
 Removed the bm->INVERT_PREYeatenGlobal and bm->INVERT_PREYeaten arrays. This data
 has been moved into the function group structured and resized to store habitat (WC, SED, EPI)
 data.

 03/11/2008 Bec Gorton
 Added the flux_thresh_id and flux_damp_id species params.

 2-02-2009 Beth Fulton
 Added new home range code.

 02-02-2009 Bec Gorton
 Simplified the code to add new species parameters. Also increased the number of species params
 to allow for the home range parameters.


 23-03-2009 Bec Gorton
 Moved the following arrays into the EcologyModule:

 double	***DebugInfo;	//< Array to store debug variable term values
 double **DebugFluxInfo; //< Array to store debug flux values

 07-04-2009 Bec Gorton
 Added the map projection parameter to the box model to handle the projection of the box model.
 This will allow us to calculate the number of day hours in a time step to accurately adjust the
 primary producer light limitation.

 The new lim_sun_hours has also be added. This is read in from the biology input file.


 22-04-2009 Bec Gorton
 Added each of the module header files to this file.
 Added the definitions:
 #define MAXSEDIMENTDEPTHSTR "100000"
 #define MAXSEDIMENTDEPTH 100000
 so that we can define the default depth boundaries of the epibenthic
 groups.

 Also added the following additional species params:
 age_structured_prey_id, KTUR_id, KIRR_id, vl_id, ht_id, KN_id, KS_id, KF_id, KI_T15_id,
 KI_id, KLYS_id, mD_id, mO_id, mS_id, mS_T15_id, KO2_id, KO2LIM_id,
 max_id, low_id, thresh_id, sat_id, min_depth_id, max_depth_id, flagchannel_id

 Added references to each of the module data structures in the BoxModel structure.
 Moved the definitions to the ecology functions that don't need to be globally defined such as Eat
 inte the atEcology module.


 02-06-2009 Bec Gorton
 Added support for log files to be rolled over when they get to a default size.
 By default the max size is MAX_LOG_FILE_SIZE 1.024e9. If the code is compiled
 with -D SMALL_LOG_FILE_SIZE then the max log file size is  MAX_LOG_FILE_SIZE 1.024e3.
 It was done this way to make for quicker testing - its easier to compare lots of small
 files instead of many really big files.

 10-06-2009 Bec Gorton
 Added the SEAGRASS invertebrate group.

 15-06-2009 Bec Gorton
 Changed the k_trans value to be a species parameter. The k_trans value is now used for the
 BFS, BFF and BFD values. For the rest of the groups 1.0 is used. Parse_File_Single_Line is used in convertAtlantis
 to create the new XML.

 02-07-2009 Bec Gorton
 Added the FC_case_id fishery parameters - the FC_case values will now be stored in bm->SP_FISHERYprms array.
 Added the harvestModuel structure to the box model structure.

 20-07-2009 Bec Gorton
 Added support for the user to provide more than one temperature and salinity input file.

 24-07-2009 Bec Gorton
 Changed the point source scaling to store a value per point source instead of a single mult, period and start
 value for all sources. Will require changes to input files - documented on the wiki.

 28-10-2009 Bec Gorton
 Added new species params that have been created. These were values that were stored in seperate arrays
 but could easily be moved into the speciesParam array.
 Also change the fishing params to be an enum instead of a set of #define values.

 30-10-2009 Bec Gorton
 Removed the K_num_invcohorts value.

 03-11-2009 Bec Gorton
 Removed the SPtoCATid array as these values are no longer required

 04-11-2009 Bec Gorton
 Merged in Beths fishing bycatch incentive code. - revision 961.

 05-11-2009 Bec Gorton
 Removed the #defined tot_prms value and added it to the end of the enum for
 the species params.
 Also changed the Subfleet (average vessel) characteristics and economic parameters value
 to be defined as a enum instead of #define values.

 10-11-2009 Bec Gorton
 Merged with trunk - revision 1423
 Added the calcMLinearMort, calcMQuadMort and calcMPredMort arrays to store different mortality values.

 15-12-2009 Bec Gorton
 Trunk merge 1437 - Added the atCLAMModule header file. Normally this would only be
 included in a win environment but for now it shall be included regardless of the environment.
 Also added an additional debugging option for the CLAM.

 29-01-2010 Bec Gorton
 Removed the fisheryNAME array. A number of variables and functions have been moved into the harvest
 and management libraries.

 03-02-2010 Bec Gorton
 Removed the atHarvestModule and the atManageModule as they are no longer required.

 26-02-2010 Bec Gorton
 Removed the following arrays:
 bm->NutsProd = Util_Alloc_Init_2D_Double(K_num_global_nonliving, EPIFAUNA, 0.0);
 bm->NutsProdGlobal = Util_Alloc_Init_3D_Double(K_num_global_nonliving, EPIFAUNA, nlevel_id, 0.0);
 bm->NutsLost = Util_Alloc_Init_2D_Double(K_num_global_nonliving, EPIFAUNA, 0.0);
 bm->NutsLostGlobal = Util_Alloc_Init_3D_Double(K_num_global_nonliving, EPIFAUNA, nlevel_id, 0.0);
 bm->DetritusProd = Util_Alloc_Init_2D_Double(K_num_global_nonliving, EPIFAUNA, 0.0);
 bm->DetritusProdGlobal = Util_Alloc_Init_3D_Double(K_num_global_nonliving, EPIFAUNA, nlevel_id, 0.0);
 bm->DetritusLost = Util_Alloc_Init_2D_Double(K_num_global_nonliving, EPIFAUNA, 0.0);
 bm->DetritusLostGlobal = Util_Alloc_Init_3D_Double(K_num_global_nonliving, EPIFAUNA, nlevel_id, 0.0);

 These are now stored in the BoxLayerValues structure that is allocated in Bio_Process - this
 stores values in the current box layer. Getting rid of the global arrays will assists us to
 move to a parallel platform in the future.
 Removed the atEcologyStructure from the model.

 01-03-2010 Bec Gorton
 Added variables to the MSEBoxModel structure to store information about larval dispersion.

 19-04-2010 Bec Gorton
 Increased the number of physical parameters that are sampled in the assessment module.

 17-05-2010 Bec Gorton
 Replaced the K_num_prod value with a check for isVertebrate = FALSE && IsDetritus = FALSE.
 Replaced the K_num_eat with with a check for isVertebrate = FALSE && isGrazer = TRUE.

 05-05-2010 Bec Gorton
 Added the mFC_scale_id definition so we can store the scaling value that should be aplied to the mFC value due to the broken-stick
 management strategy.
 Also added the definitions for the new temperature correction algorithm.

 07-05-2010 Bec Gorton
 Added additional species params to store the indices of each group (allow for adult and juv values for age structured groups)
 in the linear mortality scaling time series input file.
 Also added the new tslinearMort TimeSeries structure to the MSWBoxModel structure to store the new time series data and the function definitions
 readLinearMortalityTimeSeries to read in the time series data and Setup_Linear_Mortality_Indicies to link each group to the variables in the
 read in time series file.

 19-10-2010 Bec Gorton
 Removed the assessModule.h file. This is no longer required.
 Removed the K_num_invfished attribuute - this is now determined from the values in the functional group input file.

 08-11-2010 Bec Gorton
 Removed the atEconomic module.

 15-11-2010 Bec Gorton
 Removed references to K_num_charis.

 03-03-2011 Bec Gorton
 Added code to read in changes in mum and FSPB rates for vertebrates. Functionality documented on the Atlantis wiki at:
	https://wiki.csiro.au/confluence/display/Atlantis/2011/03/03/Changing+growth+rates+and+FSPB+over+time

 26-03-2011 Beth Fulton
 Added species specific starvation coefficients - mStarve

 16-08-2011 Beth Fulton
 Added DependDiscardsTot so that discards dependent on catch could be represented without the
 full harvest model being used

 03-10-2011 Beth Fulton
 Added bulkTAC, OldCatchSum, TotOldCumCatch, TotOldCumDiscards, tac_resetperiod_id, tac_resetcount_id to deal with multi-year TAC

 14-05-2012 Beth Fulton
 Added stuff for Savina seagrass model - including totagepop so can get age structured overall output

 21-06-2012 Bec Gorton
 Changed flageat_id to be a cohort dependant species flag.

 31-01-2013 Bec Gorton
 Added P as a nutrient. Removed the K_num_global_nonliving to be an enum and added P.

 05-07-2012 Bec Gorton
 Removed the TempSaltData structure as its not used anymore. We are using the PhyPropertyData structure.

 01-07-2013 Bec Gorton
 Added support to read in forcing netcdf files for any tracer in the model. The user needs to specify a missing_value value in the netcdf file. If the
 value for a given box/layer is 'missing_value' then it is ignored. Else the current tracer value for this box/layer is set to the value from the netcdf file
 and any existing value is overwritten.
 
 Also added support for an external mortality to be applied to a group. At present these values are never read in we just have the stub code to apply the mortality.


*********************************************************************/
//TODO: Change all defines to capitals or enums.
//TODO: dtype - chnage to enum.
//TODO: Split box model into more structs - one struct per data type.

#ifdef LINK_ENABLED
	#include <atLinkModule.h>
#endif
#ifdef BROKER_LINK_ENABLED
	#include <atBrokerLink.h>
#endif
#include <atPhysicsModule.h>

#define doubleINPUT  double
#define double double

#ifndef WIN32
//int _isnan(double v);
//int _finite(double v);

#define _isnan( v ) (isnan( v ) )
#define  _finite( v ) ( isfinite( v ) )
#define  _getcwd( buf, strlen ) ( getcwd( buf, strlen ) )
//#define _chdir(str) ( chdir(str) )


/* Unbelievable that we have to do this. _inline is valid in VS not inline. */
#define _inline inline
#endif

#define ROUNDGUARD (double)0.00000000000001
#define MAXSEDIMENTDEPTHSTR "100000"
#define MAXSEDIMENTDEPTH 100000
#define MAX_GROUPCODE_LENGTH 50 /* The max length of the group code to be read in from the functional group input file */
#define MAX_FISHERYCODE_LENGTH 50 /* The max length of the group code to be read in from the functional group input file */
#define MAX_XML_COMMENT_LENGTH 20000
#define STRLEN 2000

/** This allows for setting at compilation */
#ifndef SMALL_LOG_FILE_SIZE
#define MAX_LOG_FILE_SIZE 2.024e8
#elif BETH_LOG_FILE_SIZE
#define MAX_LOG_FILE_SIZE 1.024e8
#else
#define MAX_LOG_FILE_SIZE 1.024e7
#endif

#ifdef SMALL_NETCDF_FILES
#define MAX_NETCDF_FILE_SIZE  1.024e7
#else
#define MAX_NETCDF_FILE_SIZE  1.024e12
#endif

#define FEMALE 0
#define MALE 1

#define TRUE		1
#define FALSE		0
#define INT_NAN -123456
#define DOUBLE_NAN -123456.00


/* Note if you add an additional species parameter make sure you
 * add the line in the conversion code (in ecol_convertSetup()) to
 * tag the parameters.
 */
//#define numSpeciesParams 93
//#define numEcologyParams 25
//#define numEcologyStockParams 4


/* Type of floating point values for tracers, epi variables */

#ifndef NAN
#define NAN		-123456
#endif
#ifndef TRUE
	#define TRUE		1
#endif
#ifndef FALSE
	#define FALSE		0
#endif

//#define NO_RATIO
//#define TURN_OFF_P_IN_VERTEBRATES 1
//#define VERTS_TURN_OFF_GROWTH 1
//#define TURN_OFF_EATING_VERTS 1	// Turn off growth in vertebrates due to eating vertebrates.
/**
 * The possible model projections - used to calculate the proportaion of a timesteps day light.
 */
typedef enum {
	ALBERS
} PROJECTION;

typedef enum {
	WC = 0,
	SED,
	EPIFAUNA,
	LAND_BASED,
	ICE_BASED,
    MIXED
} HABITAT_TYPES;

#define nlevel_id (ICE_BASED + 1)
#define growth_id (ICE_BASED + 1)  /* growth_id and death_id are used with WC, SED and EPIFAUNA in MaxFluxCheck */
#define death_id (ICE_BASED + 2)


/* Useful macros */
/**/
//#if !defined(min)
#ifndef min
#define min(x,y)  ( (x) <= (y) ? (x) : (y) )
#endif

#if !defined(min)
#define max(x,y)  ( (x) >= (y) ? (x) : (y) )
#endif
/**/

/* Miscellaneous defines */
#define BMSLEN    (512)     /* Max string lengths */
#define WATER     "water"   /* Name of water variable */

#if !defined(MAXDOUBLE)
#define MAXDOUBLE (1.7976931348623158e+308)
#endif

#if !defined(MINDOUBLE)
#define MINDOUBLE (2.2250738585072014e-308)
#endif

#if !defined(MAXFLOAT)
#define MAXFLOAT (3.402823466e+38)
#endif

#if !defined(MINFLOAT)
#define MINFLOAT (1.175494351e-38)
#endif

#if !defined(MAXINT)
#define MAXINT (2147483647)
#endif

#define no_quota 1000000000000.0
#define buffer_tol 1.1
//#define small_num 0.0000000000000001
//#define small_num (double) 1e-08
#define small_num (double) 0.0000000000000001
#define buffer_rounding 0.000001
#define buffer_ratio 0.0000000000001
#define mg_2_tonne 0.00000002    /* mg C converted to wet weight in tonnes == 20 / 1000000000 */
#define mg_2_kg 0.00002 		 /* mg C converted to wet weight in kg == 20 / 1000000 */
#define mg_2_g 0.02 			 /* mg C converted to wet weight in kg == 20 / 1000 */
#define tonne_2_mg 50000000.0	 /* mg C converted to wet weight in tonnes == 1000000000 / 20 */
#define kg_2_mg 50000.0			 /* mg C converted to wet weight in kg == 1000000 / 20 */
#define g_2_mg 50.0				 /* mg C converted to wet weight in kg == 1000 / 20 */

#define s_2_yr 31536000			 /* seconds in a year */
#define per_m2_from_per_km2 0.000001
#define per_km2_from_per_m2 1000000

#define one_dt 0
#define two_dt 1
#define other_dt 2

typedef enum {
	p_id = 0,
	c_id,
	num_atomic_id
}ATOMIC_ELEMENTS;

/* read-in param type */
typedef enum  {
	no_checking, binary_check, proportion_check, probability_check, integer_check
} ATLANTIS_CHECK_TYPES;

/*********************************************************************
 File types - so know what will be read in
 */
typedef enum {
	ts_file = 0,
	cdf_file
} ICE_INPUT_TYPES;



/*********************************************************************

 DEFINITION OF ID NUMBERS

*/

/* Vertebrate physiological property id numbers */

#define SN_id 0
#define RN_id 1
#define DEN_id 2

/* The seafloor types have the following ids */
#define reef_id 0
#define flat_id 1
#define soft_id 2
#define canyon_id 3

#define K_num_sed_type 3

#define aerobic_sed 0
#define anaerobic_sed 1

#define tempscale_id 0
#define saltscale_id 1
#define pHscale_id 2

#define K_num_env_scales 3

#define light_orig 0
#define light_bott 1
#define light_top 2
#define light_mid 3

/* pH-temp-salinity relationship depth thresholds */
#define pH_surface_water 0
#define pH_mid_water 1
#define pH_deep_water 2

#define no_pH_effect_id 0
#define monod_pH_id 1
#define nonlin_pH_id 2
#define lin_pH_id 3
#define piecewise_pH_id 4
#define quadratic_pH_id 5

#define ph_no_mort 0
#define ph_sigmoid_mort 1
#define ph_McElhany_Bush_mort 2

#define no_arag 0
#define ts_arag 1
#define model_arag 2

#define ph_disabled 0
#define ph_data 1
#define ph_regression 2

/* min and max ids for gape check */
#define min_SN_id 0
#define max_SN_id 1
#define min_RN_id 2
#define max_RN_id 3

/* Invertebrate aging id numbers */
#define juv_id 0
#define adult_id 1
#define K_num_invageclass 2

/* Vertebrate cohort aging switches */
#define called_from_spawn 0
#define called_from_mature 1

/* Vertebrate aging check ids */
#define startday_id 0
#define endday_id 1
#define ageday_id 2
#define recdays_id 3
#define wrapon_id 4

#define K_num_agecheck_id 5

/* Stock structure type ids */
#define horiz_only 0
#define vert_only 1
#define mixed_stock 2

/* Define different respiration cases */
#define no_resp 0
#define base_resp 1
#define deb_resp 2

/* Migration (in/out model domain) id numbers - still needed for parameter read in */
#define Time_id 0
#define Return_id 1
#define Period_id 2
#define FSM_id 3
#define FSMG_id 4
#define returnstock_id 5
#define YearsAway_id 6

/* Refugia model ids */
#define no_refuge_model 0
#define orig_refuge_model 1
#define rugosity_refuge_model 2

/* Benthic space limitation model ids */
#define no_ben_lim 0
#define simple_ben_lim 1
#define ersem_ben_lim 2

/* Rugosity model type */
#define no_rugosity 0
#define mechanistic_rugosity 1
#define blackwood_rugosity 2
#define cover_sum_rugosity 3
#define bozec_rugosity 4
#define bioerosion_rugosity 5

/* Movement ids */
#define no_ddepend_id 0
#define step_ddepend_id 1
#define decay_ddepend_id 2

/* Nutrient limitation ids */
#define leibig_id 0
#define mult_nut_id 1
#define WQI_id 2
#define monod_nut_id 3

/* Light limitation ids */
#define irr_adapt_off 0
#define irr_adapt_on 1
#define irr_ice 2

/* Transport option ids */
#define true_mirror 1
#define zero_invalid 2

/* Oxygen limitation ids */
#define O2_ambient_id 0
#define O2_depth_id 1
#define O2_IGBEM_id 2
#define O2_quad_id 3

/* Temperature dependence ids */
#define base_q10_id 0
#define humped_griffith_q10_id 1
#define Heinichen_q10_id 2
#define CEATTLE_q10_id 3

/* Temperature effects on efficiency */
#define no_effect 0
#define poorer_when_cool 1
#define poorer_when_warm 2
#define versus_baseline 3

/* Functional response ids */
#define eat_parslow_holling2 0
#define eat_parslow_holling1 1
#define eat_parslow_holling3 2
#define eat_ecosim 3
#define eat_minmax 4
#define eat_holling3size 5
#define eat_ratio_dependent 6
#define eat_std_holling1 7
#define eat_std_holling2 8
#define eat_std_holling3 9
#define eat_std_holling4 10
#define eat_hassel_varley 11
#define eat_crowley_martin 12

/* Recruitment type ids */
#define no_recruit 0
#define const_recruit 1
#define chl_recruit 2
#define BevHolt_recruit 3
#define rand_recruit 4
#define plank_recruit 5
#define BevHolt_rand_recruit 6
#define recover_recruit 7
#define force_recover_recruit 8
#define Ricker_recruit 9
#define SSB_BevHolt_recruit 10
#define linear_recruit 11
#define fixed_linear_recruit 12
#define ts_recruit 13
#define larval_recruit 14
#define jackknife_recruit 15
#define coral_recruit 16
#define baltic_ricker 17
#define multiple_ts_recruit 18
#define BevHolt_num_recruit 19
#define SSB_ricker 20
#define BevHolt_direct_num_recruit 21
#define ice_wimd_based 22

#define independent_recruit_distrib 0
#define at_parent_location 1
#define external_recruit 2
#define larval_dispersal 3
#define external_spawn_local_recruit 4

#define intersp_linear 1
#define intersp_inverse 2

#define no_KWSR_forcing 0
#define scale_KWSR 1
#define supply_KWSR 2

/* Movement type ids */
#define perscribed_move 0
#define sedentary_move 1
#define weight_ddepend 2
#define sticky_ddepend 3
#define no_move 4
#define switch_ddepend 5
#define homerange_move 6
#define only_ddepend 7

/* Multi-year migration types */
#define all_cohorts_migrate_per_yr 0
#define one_cohort_per_yr 1

/* Invertebrate spawning and aging ids */
#define updatespawn_id 0
#define updateage_id 1
#define spawnbiom_id 2
#define newadult_id 3
#define oldj_id 4
#define olda_id 5
#define adbiom_id 6
#define juvbiom_id 7

#define invert_reprod_prm 8

/* Invading species model ids */
#define no_invader 0
#define simple_invader 1
#define conditional_invader 2
#define complex_invader 3

/* Overwintering ids */
#define no_overwintering 0
#define date_overwintering 1
#define enviro_overwintering 2

#define cystenter_id 0
#define cystexit_id 1

/* Evolution ids */
#define complex_inheritance 0
#define simple_inheritance 1
#define straight_inheritance 2

/* Migration related ids */
#define simple_migration 0
#define annual_spawn_multiyr_migration 1
#define multiple_spawn_multicycle_migation 2

/* Migration stagger related ids */
#define return_stagger_id 0
#define stagger_years_id 1

/* Environmental effects  options */
#define not_sensitive 0
#define sensitive_heaviside_window 1
#define sensitive_biologistic_window 2
#define sensitive_gradual_side 3

/******************************************************************************************
 Flux checking and global carry over of nutrient fluxes

 Chemical ids for global carry over of detritus, nutrients and consumption
 between the first and subsquent iterations of the adaptive time_step
 (i.e. ids for global arrays passing prey eaten, NH release etc from itcount == 1
 to later itcounts)
 */



#define DLdet_id 0
#define DRdet_id 1
#define DCdet_id 2

typedef enum{
	NH_id,
	NO_id,
	Si_id,
	Fe_id,
	P_id,
	C_id,
	K_num_nutrients
} NUTRIENTS;



#define DiagnostNH_id 0
#define DiagnostNHsed_id 1
#define DiagnostDL_id 2
#define DiagnostDR_id 3
#define DiagnostDLsed_id 4
#define DiagnostDRsed_id 5
#define Diagnostnlevel_id 6

#define one_nut_lim 3  /* Case when a single limiting nutrient for primary production */

#define qnancheck 0   /* These id numbers identify kind of error message reported to the logfile */
#define noqnanverbose 1
#define noqnan_n_fail 2

#define gain_id 0
#define loss_id 1
#define num_nut_flux_id 13
//#define not_tracking_flux 80

#define no_move_check 67

#define use_bm 0
#define use_tr 1

#define invert_propsum 0
#define invert_biosum 1

/* Debuging settings */
/*TODO: Change to enum and document **/

#define debug_fishing 1		// debug fishing
#define debug_discard 2		// debug discards
#define debug_histfishing 3	// debug histfishing
#define debug_assess 4		// debug assessments
#define debug_mpa 5			// debug mpas
#define debug_effort 6		// debug effort
#define debug_econeffort 7	// debug econ effort
#define debug_econ 8		// debug econ
#define debug_quota 9		// debug quota
#define debug_age 10		// debug aging
#define debug_recruit 11	// debug recruitment
#define debug_spawn 12		// debug_spawning
#define debug_migrate 13	// debug migration
#define debug_move 14		// debug movement
#define debug_stock 15		// debug stocks
#define debug_biom 16		// debug biomass calcs
#define debug_eat 17		// debug feeding
#define debug_general 18	// debug everything
#define debug_biology_process 20 //print the biology process function parameters
#define debug_prey_biology_process 21 //print the biology process function parameter
#define clam_debug 22		// debug clam - atlantis linkage
#define debug_deposit 23	// debug deposition
#define debug_mL 24			// debug mortality scalar
#define debug_mum 25		// debug growth scalar
#define debug_FSPB 26		// debug FSPB additions
#define debug_atomic 27     // debug C and P code
#define debug_ss3 28        // debug SS3 tiered code
#define debug_external_scaling 29
#define debug_external 30   // debug external population model

/***************************************************************************************************
 For Fisheries
 fishery							id
 midwater trawl on CEP			0
 jig on CEP						1
 midwater trawl FPS/FPL			2
 dredge on BFS					3
 gillnet on FDS/FDD/FDC/FDF		4
 gillnet on SHB/SHD				5
 pelagic line on FVO/FVT/SHP		6
 purse seine on FVO/FVT			7
 purse seine on FPS/FPL			8
 trap on BMS/BML					9
 trap on FDS/FDD/FDC/FDF			10
 demersal trawl on BMS			11
 demersal trawl on CEP			12
 demersal trawl on FDD/FVD/FDC/
 FDF                          13
 demersal trawl on FDS/FDB/FDE	14
 demersal trawl on FDO			15
 midwater trawl on FDD/FVD/FDC	16
 danish seine on FDS/FDB			17
 demersal line on FDD/FVD/FDC	18
 demersal line on FVS/FDF		19
 demersal line on SHB/SHD		20
 dive on BG						21
 purse seine on FVS/FDF			22
 cull of PIN						23
 recreational on BMS/BFS/BG/FVT/
 FVS/FDS/CEP/FDB/FPS/FPL/SHD/
 BFF/PWN/FDE/FVB/SHR/SSK/WDG/
 REP                          24
 prawn trawl on PWN/BMS          25
 demersal trawl on FBP           26
 midwater trawl on ZL            27
 trap on FDE 	                28
 line on FDE/FDB                 29
 net on FDE/FDS/FDB              30
 midwater trawl on PWN			31
 mow MA							32
 */

// Fisheries.
////TODO: Enum - then can loop over the parameters.
#define midwcCEP_id 0	// midwater trawl on cephalopods
#define jigCEP_id 1		// jig on cephalopods
#define midwcFP_id 2	// midwater trawl on planktivores
#define dredgeBFS_id 3	// dredge on filter feeders
#define netFD_id 4		// gillnet on demersals and ling
#define netSH_id 5		// gillnet on sharks
#define plineFVO_id 6	// pelagic line on tuna & sharks
#define pseineFVO_id 7	// purse seine on tuna & sharks
#define pseineFP_id 8	// purse seine on planktivores
#define trapBMS_id 9	// trap on macrozoobenthos
#define trapFD_id 10	// trap on demersals and ling
#define dtrawlBMS_id 11	// demersal trawl on macrozooben
#define dtrawlCEP_id 12	// demersal trawl on cephalopods
#define dtrawlFD_id 13	// demersal trawl on demersal/ling
#define dtrawlFDB_id 14	// demersal trawl on flathead
#define dtrawlFDO_id 15	// demersal trawl on roughy
#define midwcFD_id 16	// midwater trawl on demersal/ling
#define dseineFDB_id 17	// danish seine on flathead
#define dlineFD_id 18	// demersal line on demersal/ling
#define dlineFVS_id 19	// demersal line on piscivores
#define dlineSH_id 20	// demersal line on sharks
#define diveBG_id 21	// dive on benthic grazers
#define pseineFVS_id 22	// purse seine on piscivores
#define cullPIN_id 23	// cull of pinnipeds
#define recfish_id 24	// recreational fisheries
#define ptrawlPWN_id 25	// demersal trawl on prawns
#define dtrawlFBP_id 26 // demersal trawl on benthopela2g
#define midwcZL_id 27	// midwater trawl on krill
#define trapFDE_id 28	// trap on shallow demersals
#define dlineFDE_id 29	// demersal line  on shallow dem
#define netFDE_id 30	// gill net on shallow demersals
#define midwcPWN_id 31	// midwater trawl on prawns
#define mowMA_id 32		// mowing of kelp
//#define No_such_fishery_id 33

/************************************************************************************************
 Parameter IDS for parameter arrays
 **/

/******* Fisheries parameters *******/

/* id for BoxMapKey array for catch and effort time series */
#define catchkey_id 0
#define discardkey_id 1
#define rects_key_id 2

/* id number of fisheries targeting the group, any other fisheries are bycatch only */
#define tot_target_id 23
#define max_num_fc_id 24
#define failed_id 999

/* id numbers for FishingResults array from Harvest_Do_Fishing_And_ByCatch() routine */
#define Catch_result_id 0
#define CatchAtAge_result_id 1
#define Discards_result_id 2
#define DiscardsAtAge_result_id 3

/* id numbers for vertebrate group specific fisheries parameters */
typedef enum {
    sel_id,
    flagQchange_id,
    Q_num_changes_id,
    q_id,
    spawn_closure_id,
    flaghabitat_id,
    flagescapement_id,
    p_escape_id,
    Ka_escape_id,
    Kb_escape_id,
    flagdiscard_id,
    flagchangeDISCRD_id,
    DISCRD_num_changes_id,
    FFCDR_id,
    FCthreshli_id,
    k_retain_id,
    k_waste_id,
    FC_thresh_id,
    TAC_id,
    FC_high_thresh_id,
    flagimposecatch_id,
    imposecatchstart_id,
    imposecatchend_id,
    saleprice_id,                       /** Current sale price in $/kg */
    catch_allowed,                      /* Catch currently allowed to take (quota - cumulative catch) */
    flagquota_id,
    marketwgt_id,                       /* Market weighting for amount of fish feed to each market */
    flagF_id,                           /* flag indicating whether fishing mortalities being used instead of effort model */
    mFC_id,
    mFC_num_changes_id,
    flagFchange_id,
    desired_chrt_id,
    origprice_id,
    incidmort_id,
    TAC_num_changes_id,
    mFC_start_age_id,
    mFC_end_age_id,
    FC_reportscale_id,
    TACvsMPA_id,                        /* Whether to reduce q to reflect effects of seasonal spawning season closures */
    prop_spawn_close_id,
    trip_lim_id,
    phase_out_id,                       /* Day of run when TAC should be phased out completely */
    phase_start_id,                     /* Day of run when TAC phase out begins */
    tax_id,
    deemprice_id,
    deemedvalue_id,
    totquota_remain,                    /* Amount of owned quota remainging */
    FC_case_id,
    avail_id,
    origtax_id,
    taxpaid_id,
    FixedMinTax_id,                     /**< Fixed tax value */
    larger_extant_id,
    orig_mFC_scale_id,
    mFC_scale_id,
    assess_nf_id,                       /* Mapping of operating model fleets to assessment fleets */
    flagPerShotCPUE_id,                 /* Flag indicating whether per shot cpue results stored */
    flagRecordCPUE_id,                  /* Flag indicating want to store per time step general CPUE results */
    tot_sp_specif_fishing_prms
    /* The total number of values should always be last */
} SPFISHERY_PARAMS;

/* id numbers for fisheries parameters */

typedef enum {
	fisheriesactive_id,
	flagchangeSEL_id,
	SEL_num_changes_id,
	sel_b_id,
	sel_lsm_id,
	sel_normlsm_id,
	sel_normsigma_id,
	sel_lognormlsm_id,
	sel_lognormsigma_id,
	sel_gammalsm_id,
	sel_gammasigma_id,
	flagmanage_id,
	flagchangeP_id,
	P_num_changes_id,
	flageffortmodel_id,
	max_num_sp_id,
	flagchangeEFF_id,
	EFF_num_changes_id,
    flagseasonal_id,
    flag_framebased_id,
	mEff_max_id,
	mEff_a_id,
	flagmpa_id,
    mpascale_cap_id,
	max_mpa_sequence_id,
	infringe_id,
	FC_restrict_id,
	FC_period_id,
	FC_period2_id,
	FC_restrict_endangered_id,
	FC_endanger_period_id,
	TACchange_id,
	flagchangeseason_id,
	seasonopen_id,
	seasonclose_id,
	selcurve_id,
	mFCscale_id,
	flagchangeSWEPT_id,
	SWEPT_num_changes_id,
	swept_area_id,
	mEff_offset_id,
	mEff_thresh_id,
	mEff_thresh_top_id,
	mEff_shift_id,
	mEff_testfish_id,
	flagdropEFF_id,
	fisheriesflagactive_id,
	sel_bilsm1_id,
	sel_bilsm2_id,
	sel_bisigma_id,
	sel_bisigma2_id,
    sel_ampli_id,
	flagdempelfishery_id,
	nvessel_id,
	flagconflict_id,
	nsubfleets_id,
	nlicence_id,
	relconflict_id,
	conflict_contrib_id,
	maxsaleprice_id,		/** Current sale price in $/kg */
	EffortLevel_id,
	CPUE_effort_scale_id,
	CPUE_effort_thresh_id,
	flagcap_id,
	flagchangeCAP_id,
	CAP_num_changes_id,
	cap_id,
	ntargets_id,
	reset_id,
	closenotice_id,
	maxFCdepth_id,
	flagexplore_id,
	minFCdepth_id,
	flagrecfish_id,
	flagMultiSpEffort_id,
	landallTAC_sp_id,
	buybackdate_id,
	flagTACpartipcate_id,
	flaguse_preveffort_id,
	visitcouncil_id,
	mpadepth_id,
	depthoption_id,
	fisheries_need_port_id,
	tStart_id,
	tEnd_id,
	q_lsm_id, /* Assessment parameter - length half the population is selected */
	q_sigma_id, /* Assessment parameter - spread of selectivity curve */
	assess_selcurve_id, /* Assessment parameter - flag for selectivity curve Need to talk to beth about why this is different to the harvest values */
	flagcap_peak_id,
	mFCpeak_id,
	flaguseall_id,
	flagDV_id,
	start_manage_id,
	end_manage_id,
	flagADJUSTmanage_id,
	manage_on_id,
	use_min_lever_id,
	use_msy_effort_id,
	rugosity_effect_id,   /* Effect of fishing activity on rugosity - rate at which it crumbles rugosity per unit biomass of rugosity generating species */
    flagneed_shots_id,    /* Whether need to generate shot level CPUE for this fishery */
	tot_fisheries_prms
/* The total number of values should always be last */
} FISHERY_PARAMS;

/* IDs identifying whether weighting is simple or compound */
#define simple_id 0
#define compound_id 1

/* Quota storage ids */
#define old_id 0
#define now_id 1
#define RBCold_id 2
#define RBCnow_id 3

#define K_num_TAC_entries 4

#define source_id 0
#define sink_id 1

/* Tier reference limit options */
typedef enum {
    forage_secondary_tier,
    target_tier,
    byproduct_tier,
    bycatch_tier,
    TEP_tier,            // For species such as whales etc - needed for ERA and indicator based approaches
    ecol_significant_tier, 
    irrelevant_tier,     // For species such as plankton etc - needed for ERA and indicator based approaches
    K_num_indicator_tiers
} TIER_LEVELS;

/* Quota price model ids */
#define RichModel 0
#define NZmodel 1
#define NoPrice 2

/* Net returns ids */
#define immed_id 0
#define fut_id 1

/* Time series catch and discard ids */
#define tscocatch_id 0
#define tscodiscard_id 1
#define tsactcatch_id 2

/* MPA parameter id numbers */
#define start_DoY_id 0
#define end_DoY_id 1
#define start_date_id 2
#define period_used_id 3
#define rotate_period_id 4
#define sequence_id 5
#define prop_area_id 6

#define tot_mpa_prms 7

/* Port parameter id numbers */
#define portx_id 0
#define porty_id 1
#define port_start_id 2
#define port_end_id 3
#define population_id 4
#define popchange_id 5
#define POP_num_changes_id 6
#define popactive_id 7
#define old_act_id 8
#define new_act_id 9
#define prime_market_id 10

#define K_num_port_prm 11

/* Portcontrib id numbers */
#define pvalue_id 0
#define clipped_id 1

/* Market and fuel parameter id numbers */
#define fuel_intercept_id 0
#define market_intercept_id 0
#define trend_coefft_id 1
#define auto_coefft_id 2

#define K_num_market_prm 3
#define K_num_fuel_prm 3

/* mortality tracking - uses same start_id of 0 as below and then */
#define ongoingM1_id 1
#define ongoingM2_id 2
#define ongoingF_id 3
#define finalM1_id 4
#define finalM2_id 5
#define finalF_id 6
#define endNum_id 7
#define K_num_mort_counter 8

/* age specific predation mortality tracking */
#define ongoing_id 1
#define final_id 2

/* changing conditions parameters */
#define start_id 0
#define period_id 1
#define end_id 1
#define selcurvechange_id 1
#define thresh_mult_id 1
#define mult_id 2
#define addlsm_id 2
#define retain_mult_id 2
#define addsigma_id 3
#define discard_mult_id 3
#define pulse_id 3
#define waste_mult_id 4

/* Selectivity function type ids */
#define q_const_id 0
#define q_ageconst_id 1
#define q_logistic_id 2
#define q_norm_id 3
#define q_lognorm_id 4
#define q_gamma_id 5
#define q_knife_id 6
#define q_bimodal_id 7
#define q_binormal_id 8

/* Habitat - fishing pressure type ids */
#define homog_habitat_pressure 0
#define ellis_habitat_pressure 1    // old style Ellis and pantus patchy effort allocaiton model
#define ep_habitat_pressure 2       // new version of Ellis and pantus patchy effort allocaiton model based on Ellis et al 2014 CJFAS paper

/* Escapement type ids */
#define no_escape 0
#define fixed_escape 1
#define size_escape 2

/* Discarding type ids */
#define fixed_discard 0
#define fixedage_discard 1
#define size_discard 2
#define impose_discard 3
#define depend_discard 4

/* Imposed catch types */
#define no_impose 0
#define global_impose 1
#define spatial_impose 2
#define stock_impose 3
#define adjacent_impose 4

#define K_num_impose_classes 5

/* Fleet dynamics (effort) model type */
#define const_effort 0
#define const_qrt_effort 1
#define cpue_effort 2
#define array_effort 3
#define wgt_effort 4
#define dist_effort 5
#define rec_effort 6
#define alt_cpue_effort 7
#define alt_cpue_scale_effort 8
#define ideal_distrib_effort 9
#define cpue_dyneffort 10
#define readts_effort 11
#define rec_econ_model 12
#define econ_model_effort 13

/* Spatial management type ids */
#define no_mpa 0
#define fix_mpa 1
#define cycle_mpa 2
#define stock_mpa 3
#define pet_mpa 4
#define stock_pet_mpa 5
#define cycle_stock_mpa 6
#define cycle_pet_mpa 7
#define cycle_stock_pet_mpa 8
#define mix_fix_rolling_mpa 9
#define mix_f_r_spawn_mpa 10
#define depth_stock_mpa 11
#define council_stock_mpa 12
#define spawn_closure 13
#define catch_mpa 14

/* M estimate approach for use in psuedo code for assessments */
#define fixed_input_M 0
#define Z_and_F_based 1
#define full_dynamic_M_est 2
#define assess_M_est 3

/* Management tiers */
#define tier_orig 0
#define tier0 0
#define tier1 1
#define tier2 2
#define tier3 3
#define tier4 4
#define tier5 5
#define tier6 6
#define tier7 7
#define tier8 8
#define tier9 9
#define dyntier4 10
#define dyntier1B0 11
#define sp_rollover 12
#define tier13 13
#define tier14 14

#define orig_tier_rule 1
#define new_tier_rule 2
#define alt_tier_rule 3
#define safe_tier_rule 4

#define NoTrigger 0
#define TotTrigger 1
#define ZoneTrigger 2
#define LaxZoneTrigger 3
#define TightZoneTrigger 4

/* DO with filtering people to teh correct F based HCR */
#define per_sp_rescale 0
#define per_guild_rescale 1
#define per_system_cap 2

/* Companion TAC ids */
#define Weakest_Link 0
#define Strongest_link 1

/* Simple region IDs */
#define north_id 0
#define south_id 1
#define totalreg_id 2

/* Management adjustment IDs */
#define change_forced 1
#define change_dynamic 2
#define disable_tac 3

/* Management lever ID - to use with US-like MSY-based (tier6) management */
#define use_none 0
#define use_effort 1
#define use_catch 2
#define use_space 3
#define use_season 4
#define use_gear 5

/* Multispecies assessment types */
#define NoAssess -1
#define SingleSpOnly 0
#define PGMSY 1
#define IndicatorSpPGMSY 2  // PGMSY must be the first two due to way sets PGMSY_on in atassessParamIO.c
#define IndicatorSp 3
#define MultiSpProd 4

/* Contaminant fisheries management options */
#define no_closures 0
#define set_closures 1
#define conc_based 2

/******* Economic parameter id numbers ********/
/* Subfleet types */
#define dayTrip 0
#define threedayTrip 1
#define weekTrip 2
#define longTrip 3

/* Subfleet (average vessel) characteristics and economic parameters (after nboat) */
typedef enum {
	crew_size_id, capital_cost_id, max_trip_length_id, // used as if equivalent of Rich's EffortPerTrip
	var_cost_id,
	boat_size_id,
	home_port_id,
	nboat_id,
	flag_econind_id,
	cpue_ind_id,
	yield_ind_id, 				/* total catch (equivalent of Rich's RS_C) */
	size_ind_id,
	comp_ind_id,
	zone_ind_id,
	gear_ind_id,
	mgmt_cost_ind_id,
	res_cost_ind_id,
	discards_ind_id,
	gvp_ind_id,
	rev_land_ind_id,
	rev_effort_ind_id,
	cost_ind_id, 				// equivalent of Rich's trip Cost - cost in $ per unit effort. - calculated monthly.
	boat_cash_ind_id, 			// equivalent of Rich's MP
	invest_return_ind_id,
	cap_util_ind_id,
	leased_Q_val_ind_id,
	sale_Q_val_ind_id,
	Q_trade_ind_id, 			// equivalent of Rich's totalPurchasedQuota
	cost_ton_ind_id,
	flag_indtype_id,
	marg_rent_id, 				// equivalent of Rich's MR
	ExpectedCatch_id,			// equivalent of Rich's RS_C (??)
	Expectedeffort_id, 			// equivalent of Rich's RS_E (note Rich's q dealt with elsewhere in fisheries params)
	ExpectCPUE_id, 				/* Expected CPUE based on historical take for that month
								 (for use in calculating the quota) */
	totPlanCatch_id, 			// equivalent of Rich's totalPlannedCatch
	totPlanEffort_id, 			// equivalent of Rich's totalPlannedEffort
	AnnualCatch_id, 			// equiavlent of Rich's annualC
	AnnualEffort_id, 			// equivalent of Rich's annualEffort
	CurrentCatch_id, 			// equivalent of Rich's currentC = current catch in kg.
	CurrentEffort_id, 			// equivalent of Rich's currentEffort
	AnnualExpectPI_id,			/* Annual expected returns so can see expectations vs realised returns */
	AnnualUtility_id, 			// equivalent of Rich's annualUtility
	AnnualProfit_id,
								//TODO: Actually code this in
	effortDiscount_id, 			// equivalent of Rich's effortDiscount
	totcatch_saleprice_id, 		/* Sum of total price obtained for the groups landed by the subfleet */
	totQuota_id, 				/* Total catch of all groups allowed (allocation + leased quota) */
	FishableLength_id,
    ShotLength_id,
    MaxShotVol_id,
	OpUtility_id, 				/* Operators share of the utility (crew share taken from total) */
	crewshare_id, 				/* Percentage of the profits that belong to the crew */
	OldQuota_id, 				/* Last years total quota (owned and leased) */
	flexweight_id, 				/* Weighting of history when interpolating between accumulated history and most recent CPUE map */
	resale_id, 					/* Resale value of that boat type */
	switch_cost_id, 			/* Cost of refitting this type of boat to work in another fleet */
	fuel_cost_id,
	gear_cost_id,
	unload_cost_id,
	fixed_cost_id,
	buyout_id,
	newboat_id,
	switchboat_id,
	newboat_cost_id, 			/* Cost of buying new boat */
	lostboat_id,
	down_time_id,
	tot_cash_id,
	tied_up_id,
	mth_tiedup_id,
	revenue_ind_id,
	minrev_ind_id,
	maxleased_Q_val_ind_id,
	effortquotaprice_id,
	eleased_Q_val_ind_id,
	margprofit_id,
	tax_ind_id,
	dv_ind_id,
	tol_debt_id,
	last_pi_id, 				/* Previous years expected profitability */
	months_crippled_id, 		/* Number of months under crippling debt */
	cripples_lost_id, 			/* Number of extra cripples lost within the year */
	FCwgtscale_id,
	flexweight_orig_id, 		/* So can restore flexweights once historical effort forcing period is over */
	choicebuffer_id, 			/* So can buffer effort reductions if catch exceeds expectations */
	lasttot_cash_id,
	propbuyback_id,
	fleet_thresh_id,
	nboat_orig_id,
	betarev_id,
	minprob_id,
	supp_cost_id, 				/* Cost of flexibly using gear type of choice */
	mgmt_cost_id,
	hold_capacity_id, 			/* Capacity of hold of avaerge vessel in the subfleet */
	bycatchDiscount_id, 		/* Bycatch related equivalent of Rich's effortDiscount */
	pereffort_cost_ind_id,
	K_sub_fleet_params,
} SUBFLEET_PARAMS;

#define K_sub_fleet_reporting tol_debt_id /* Number of entries that will be reported on (so doesn't report tol_debt onwards) */
#define K_ecolharvest_indicators 9

/* For ecological/harvets vs economic index matchup
 which econ indicator being calculated is given in the
 which_econind entry and then the parameter values to
 use come from the cell entry, which uses the index list
 above for

 cpue_ind_id
 yield_ind_id
 size_ind_id
 comp_ind_id
 zone_ind_id
 gear_ind_id
 mgmt_cost_ind_id
 res_cost_ind_id
 discards_id
 */
#define ecoefft_ind 0
#define escale_ind 1
#define escaleB_ind 2
#define ethresh_ind 3
#define eindtype_ind 4

#define K_num_entry_indabc 5

#define igvp_id 0
#define irevland_id 1
#define irevenue_id 2
#define icost_id 3
#define icash_id 4
#define iinvest_id 5
#define icaputil_id 6
#define ileaseQ_id 7
#define isaleQ_id 8
#define iQtrade_id 9
#define icostton_id 10

#define K_econ_indicators 11

/* Econmodel types */
#define simple_func 0
#define dynamic_func 1
#define expect_func 2

#define add_funcs 0
#define mult_funcs 1

/* Economic functional response types */
#define simple_linear 0
#define piecewise_linear 1
#define asymptotic_curve 2
#define humped_curve 3

/* Economic quota types */
#define owned_id 0
#define leased_id 1
#define totalallowed_id 2
#define targetid_id 3
#define spmarg_profit_id 4			/* per kilo profit */
#define boatcatch_id 5
#define spmarg_rent_id 6			/* per kilo profit * kgs of catch */
#define finalutility_id 7			/* Kind of like wellbeing non-money type thing. */
#define spexpectcatch_id 8
#define cumboatcatch_id 9
#define quotaprice_id 10
#define over_id 11
#define permilease_id 12
#define permolease_id 13
#define templease_id 14
#define within_id 15				/* Flag indicating whether catch of a quota species is within 5% of fully taken */
#define spare2sell_id 16			/* Flag indicating whethre catch of quota species is <20% of quota so spare to sell */
#define oldcatch_id 17
#define oldquota_id 18
#define cummonthcatch_id 19
#define newleased_id 20				/* Quota leased in since last lease price calculation */
#define oldleased_id 21				/* So can track what is leased in since last lease price calculation */
#define cummonthbycatch_id 22
#define DVtriggered_id 23			/* Flag indicating whether the deemed values have kicked in for the species */
#define TAXtriggered_id 24			/* Flag indicating whether the taxes on overcatch have kicked in for the species */
#define deemed_value_id 25
#define spexpectvalue_id 26			/* Expected value per species so can do targetting based on value not catch */

#define K_num_sp_econ_prms 27

/* Black book ids */
#define expect_id 0
#define hist_id 1			// Starting values - for now only kept at original levels for SpatialBlackBook, may need to do this for BlackBook too
#define current_id 2
#define bycatch_id 3
#define updated_id 4		// As need to keep historical values for when recent records fail
#define K_num_BBook_prms 5

#define today_effort 12   // Entered at end of array after the monthly entries
#define calc_effort 13    // Entered at end of array after the monthly and current entries so can do calculations
#define new_today_effort 14   // Entered at end of array after the monthly entries so can update immediate effort allocation
#define K_num_effort_entries 15

/* Deemed values ID */
#define noDV_id 0
#define simpleDV_id 1
#define complexDV_id 2

/* Quota trade IDs */
#define amttrade_id 0
#define numtrade_id 1

/* id numbers for parameters, groups and properties */
typedef enum {
	active_id,
	age_mat_id,
	BHalpha_id,
	BHbeta_id,
	Bcov_juv_id,
	Bcov_ad_id,
	Acov_juv_id,
	Acov_ad_id,
	K_num_biogen_habad_id,
	K_num_biogen_habjuv_id,
	catcheater_id,
	ddepend_move_id,
	E1_id,
	E2_id,
	E3_id,
	flag_id,
	flagactive_id,
	flagdem_id,
	flagplankfish_id,
	fisheater_id,
	flagrecruit_id,
    flagstocking_id,
    flagkeep_plusgroup_id,
	FDMort_id,
	FDG_id,
	FDL_id,
	FSBDR_id,
	FSP_id,
	hta_id,
	htb_id,
	KA_id,
	KB_id,
	Kcov_juv_id,
	Kcov_ad_id,
	KDENR_id,
	KDEP_id,
	KL_id,
	KLP_id,
	Kmax_coefft_id,
	KUP_id,
	KSPA_id,
	KST_id,
	KWSR_id,
	KWRR_id,
	KU_id,
	Ktmp_id,
	log_mult_id,
    norm_sigma_id,
    flag_recruit_stochastic_id,
	PP_id,
	pR_id,
	recover_mult_id,
	recover_start_id,
	Speed_id,
    hvm_id,
	vla_id,
	vlb_id,
	predcase_id,
	vla_T15_id,
	Recruit_Period_id,
	Recruit_Time_id,
    cohort_recruit_entry_id,
    larval_queue_extension_id,
    direct_recruit_entry_id,
    recruit_outside_id,
    recruit_qid_id,
    li_a_id,
	li_b_id,
	min_li_mat_id,
	flagfish_id,
	access_thru_wc_id, /* Access identifier - whether a species can be accessed throughout	the water column
	 regardless of respective vertical distributions (1=yes, 0=no) */
	assess_flag_id,
	assess_bootstrap_id,
	assess_nat_mort_id,
    F_restart_id, // Scalar on mFC when re-open a fishery
	flag_prod_model_id,
	top_pcnt_id,
	bot_pcnt_id,
	flagtempsensitive_id,
	flagfecundsensitive_id,
	flagpHsensitive_id,
	flagSaltSensitive_id,
	flagnutvaleffect_id,
	flagpredavaileffect_id,
	temp_influence_id,
	Ralpha_id,
	Rbeta_id,
	FDGDL_id,
	FDGDR_id,
	min_move_temp_id,
	max_move_temp_id,
	min_spawn_temp_id,
	max_spawn_temp_id,
	min_move_salt_id,
	max_move_salt_id,
	min_spawn_salt_id,
	max_spawn_salt_id,
	min_O2_id,
    min_pH_id,
    max_pH_id,
    K_temp_const_id,
    K_salt_const_id,
    K_o2_const_id,
	need_update_id,
	E4_id,
	PBscale_id,
	BBscale_id,
    flagq10eff_id,
    flagq10receff_id,
	//flaglocalrecruit_id,
	flagbearlive_id,
	flagmother_id,
    flag_contam_distrib_id,
	E1orig_id,
	E2orig_id,
	E3orig_id,
	E4orig_id,
	stockstruct_type_id,
	hstockstruct_id,
	vstockstruct_id,
	basket_size_id, /* Basket quotas - FIX these may need to be SP_FISHERYprms not SP_prms */
	envforce_id,
    KWSRforce_id,
    feed_while_spawn_id,
	spawn_period_id,
	tsrecruit_id,
	need_recupdate_id,
	need_matupdate_id,
	firstrecday_id,
	flagrecpeak_id,
	flagimpose_id,
	basketSP_id, /* Flag to indicate whether a basket quota member */
	tier_id, /* Harvest management assessment tire */
    estError_id, /* FLag indicating broken stick perfect knowledge error typew */
    estCV_id, /* FLag indicating broken stick perfect knowledge bias typew */
    estBias_id, /* FLag indicating broken stick perfect knowledge error level typew */
	coType_id,
    max_co_sp_id, // Maximum number of companion species
    done_Co_sp_id,
	isbiogenhab_id,
	Age50pcntV_id,
	Age95pcntV_id,
	regionalSP_id, /* Flag to indicate whether a regional quota member */
	calcupdate_date_id,
	calcPerPredupdate_date_id,
	TACorig_id,
	BrefA_id,
	BrefB_id,
	BrefC_id,
	BrefD_id,
	BrefE_id,
	Blim_id,
    flag_systcap_sp_id,  /* Ecosystem cap related */
    sp_fishery_deduction_id,
    sp_fishery_expected_catch_id,
    sp_fishery_pref_id,
    sp_fishery_pref_norm_id,
	primary_fishery_id, /* Main fishery targeting a group - for use in assessment code */
	cpue_cdf_poor_r_id, /** number of events for negative binomial cdf for poorer skippers */
    cpue_cdf_poor_p_id, /** probabilty of an event for negative binomial cdf for poorer skippers */
    cpue_cdf_top_r_id,  /** number of events for negative binomial cdf for "gun" skippers */
    cpue_cdf_top_p_id,  /** probabilty of an event for negative binomial cdf for "gun" skippers */
	flux_thresh_id,
	flux_damp_id,
    //num_migrate_id,  // Now functional group direct attributes
    //multiyr_mig_id,
    //flag_multiyr_type_id,
    k_trans_id,
	sp_remin_contrib_id,
	age_structured_prey_id,
	p_split_id,
	KTUR_id,
	KIRR_id,
	vl_id,
	ht_id,
	KN_id,			// Half-sat const for phytoplankton growth on DIN mg N m-3
	KS_id,			// Half-sat const for phytoplankton growth on Si mg N m-3
	KF_id,			// Half-sat const for phytoplankton growth on Micro-nutrient mg Fe m-3
	KI_T15_id,
	KI_id,
	KLYS_id,
	mD_id,
	mO_id,
	mS_id,
	mS_T15_id,
	KO2_id,
	KO2LIM_id,
	/* Space restrictions for sedentary (epibenthic) groups and deposit feeders */
	max_id,
	low_id,
	thresh_id,
	sat_id,
	mindepth_id,
    maxdepth_id,
    maxtotdepth_id,
	flagchannel_id,
	homerangerad_id,
	rangeoverlap_id,
	sp_concern_id, /* Identification of species of concern */
	flagFonly_id,
	FrefA_id,
    FrefH_id,
    FrefLim_id,
    maxmFC_id,
    tac_resetcount_id,
	tac_resetperiod_id,
	tac_resetnow_id,
	flag_lim_id,
	mStarve_id,
	mT_id,
	prop_spawn_lost_id,
	jack_b_id,
	jack_a_id,
    intersp_depend_recruit_id,
    intersp_depend_sp_id,
    intersp_depend_scale_id,    
	/* Assessment params */
	samplesize_id,
	allometic_li_a_id, /* lwprm[0] */
	allometic_li_b_id, /* lwprm[1] */
	allometic_bin_size_id, /* lwprm[2] */
	allometic_bin_start_id, /* lwprm[3] */
	max_length_id,
	r_max_id,
	avg_inv_size_id,
	assessDataStream_id,
    whichRAssess_id,
	/* Physiology related params */
	q10_id,
	q10_method_id,
	q10_optimal_temp_id,
	q10_correction_id,
	use_lever_id,
	temp_coefftA_id,
	salt_correction_id,
	pHsensitive_model_id,
	pH_constA_id,
	pH_constB_id,
    pH_constC_id,
	KN_pH_id,
	optimal_pH_id,    
    pHmortstart_id,
    pHmortA_id,
    pHmortB_id,
    pHmortmid_id,
	//min_spawn_pH_id,
	//max_spawn_pH_id,
	pH_correction_id,
	flagcontract_tol_id,
	contract_tol_id,
	/* Macrophyte model params */
	KI_L_id,		// Extra params to do with epiphytes on seagrass - Savina model
	KI_L_T15_id,
	Kext_id,
	Ksub_id,
	KNepi_id,
	KsubEpi_id,
	Ktrans_id,
	/* C and P related params */
	P_max_uptake_id, 			//max phosphorus uptake rate per day
	P_min_internal_id,
	P_max_internal_id,
	P_uptake_scale_id, 		//affinity coefficient for phosphorus uptake mechanism
	P_concp_id, 				//internal concentration factor
	Psa_min_id,				// PSAMIN - min quota of phosphorus in phytoplankton cells in moles/mg dry weight.
	phyto_resp_rate_id,		// Phytoplankton respiration rate
	KP_id,					// Half-sat const for phytoplankton growth on P
	C_concp_id,
	C_max_uptake_id,
	C_uptake_scale_id,
	C_min_id,				// PSAMIN - min quota of Carbon in phytoplankton cells in moles/mg dry weight.
	Mdt_id,
    /* Pollutant effects parameters */
    light_coefft_id,
    noise_coefft_id,
    /* Overwintiering related parmeters */
	overwintering_id,  		// flag to set if the group is overwintering (or not)
	overwinterStartTofY_id,
	overwinterEndTofY_id,
	overwinterStartTemp_id,
	overwinterEndTemp_id,
	crit_mum_id,
	crit_nut_id,
	crit_temp_id,
	encyst_rate_id,
	hatch_rate_id,
	encyst_period_id,
	flagencyst_id,
	/* Evolution related params */
	max_prop_shift_id,
	inheritance_id,
	trait_variance_id,
	min_trait_variance_id,
	/* Bleaching parameters */
	bleach_periodA_id,
	bleach_periodB_id,
	mBleach_id,
	bleaching_rate_id,
	bleach_recovery_rate_id,
	bleach_tempshift_id,
	bleach_growshift_id,
	bleach_temp_id,
    min_bleach_temp_id,
    min_bleach_salt_id,
    max_bleach_salt_id,
    prop_zooxanth_id,
	DHW_thresh_id,
    threshdepth_id,
    depmum_scalar_id,
	/* Other coral related parameters */
	rugFeedScalar_id,
	HostRemin_id,
	calcifRefBaseline_id,
	calcifTconst_id,
	calcifTcoefft_id,
	calcifTopt_id,
	calcifLambda_id,
	FeedLightThresh_id,
	PropLightFeed_id,
	rug_erode_id,
	rug_bleacherode_id,
    rug_factor_id,
    colony_ha_id,
    max_colony_diam_id,
	coral_overgrow_id,
	coral_compete_id,
	coral_max_accel_trans_id,
	coral_max_accelA_id,
	coral_max_accelB_id,
	CrecruitA_id,
	CrecruitB_id,
	CrecruitC_id,
	rec_HabDepend_id,
	RugCover_scalar_id,
    coralID_id,
    /* Sponge related */
    sponge_overgrow_id, // Parameterised as daily rate of growth of MA over sponges, smothering. Refer to Ecology paper for the section with macroalgae.
    sponge_compete_id,  // Limitation to sponge growth from corals and macroalgae
    Ksmother_B_id,
    Ksmother_A_id,
    Vmax_deltaSi_id,    // Vmax to mimic shape of a Michaelis-Menten Si uptake, as per Maldonado et al. (2011)
    Km_deltaSi_id,      // Km (max concentration of Si)
    rug_erode_sponge_id, // Daily removal of rugosity from bioeroding sponges only
    extra_feed_id,        // Aquaculture related
    aquacult_fry_id,
	aquacult_age_harvest_id,
	/* Ice related parameters */
	Beta_D_id,
	PBmax_D_id,
	ICE_KDEP_id,
    /* External population related parameters */
    linf_id,
    Kbert_id,
    tzero_id,
    turbid_refuge_id, // Turbidity effects flag
    /* dynamic X_RS parameters */
    RSmax_id,
    RSmid_id,
    RSslope_id,
    RSprop_id,
    SNcost_id,
    RNcost_id,
    RSstarve_id,
    //flagusingRedusR_HCR_id,  // Using R harvest control rules for this species - just assume using Redus for all or none for now
    /* Lake fish recruitment parameters */
    prod_alpha_id,
    den_depend_beta1_id,
    den_depend_beta2_id,
    temp_coefft_id,
    rate_coefft_id,
    wind_coefft_id,
    recruit_var_id,
	tot_prms

} SPECIES_PARAMS;

typedef enum {
	mQ_T15_id,
	mL_T15_id,
	mL_id,
	mQ_id,
    mLext_id,
    mPext_id,
    mL_scale_id,
	flageat_id,
	mE_id,			   /* External mortality */
	rugosity_inc_id,   /* Rugosity increases due to size-specific coral growth */
	rugosity_dec_id,   /* Predation induced decrease to size-specific rugosity */
    colony_diam_id,    /* Reef colony diameter */
    L_turbid_id,       /* Turbidity effects related */
    a_turbid_id,
    b_turbid_id,
    cohortDepParams

} COHORT_SPECIES_PARAMS;

typedef enum {
	Time_Spawn_id,
	Time_Age_id,
	spawnDepParams

} SPAWN_SPECIES_PARAMS;

/* Scaling indices */
typedef enum {
	size_scale_id = 0,
	mum_scale_id,
	FSPB_scale_id,
	//E_scale_id,	/* not yet implemented */
	num_scaling_indicies
} SCALING_INDICES;

/* Macrophyte indices */
typedef enum {
	main_biomass_id = 0,		/* Leaves */
	below_ground_biomass_id = 1,	/* Roots */
	epiphyte_biomass_id = 2,		/* Epiphytes */
	num_macrophyte_indices
} MACROPHYTE_INDICES;

/* Evolution relevant trait indices */
typedef enum {
	growth_trait_id = 0,
	consumption_trait_id,
	maturation_trait_id,
	reproduce_trait_id,
	temp_trait_id,
	pH_trait_id,
	salt_trait_id,
    rsmid_trait_id,
    rsprop_trait_id,
	K_num_traits
} TRAIT_INDICES;

typedef enum {
	evol_mean_id = 0,
	evol_var_id,
	evol_theoretmean_id,
    evol_relvaldiff_id,
	K_num_evol_prop
} EVOL_PROP_INDICES;

typedef enum {
	evol_birth_shift_id = 0,
	evol_birth_var_id,
	evol_parent_var_id,
	K_num_evol_shift
} EVOL_SHIFT_INDICES;

/******* Management model indices *******/

/* TAC handling indices */
#define target_day_id 0
#define endangered_day_id 1
#define triggered_scalar_id 2
#define endanger_is_trigger_id 3

/* TAC effort handling ids */
#define target_tac_id 0
#define endangered_tac_id 1
#define target_message_id 2
#define endangered_message_id 3

/* Regional TAC ids */
#define reg_tac_id 0
#define reg_catch_id 1

/* Management type ids */
#define no_adapt_mgmt 0
#define stock_adapt_mgmt 1
#define TAC_mgmt 2
#define basketTAC_mgmt 3
#define regionalTAC_mgmt 4
#define coTAC_mgmt 5
#define coBTAC_mgmt 6
#define RbasketTAC_mgmt 7
#define coBRTAC_mgmt 8
#define coBRTACtrans_mgmt 9
#define cumTACclosure_mgmt 10

/******* CPUE shot-by-shot generation indices *********/
#define tcatch_id 0
#define tdiscard_id 1
#define teffort_id 2
#define wdepth_id 3
#define tloc_id 4
#define guru_id 5
#define K_num_shot_data_entries 6

// negative binomial here for now but could also have normal, gamma, poisson etc

/******* Sampling model indices *******/
// Put here as also called from atManageTier (for the downhill_simplex component of that code)
/* Assessment types */
#define no_assess 0
#define schafer_model 1
#define VPA_model 2
#define MSVPA_model 3
#define CAB_model 4
#define CAB_mimic 5
#define qlinear_regress 6
#define CPUE_linear_regress 7
#define EquilF_Funk_model 8
#define SurplusProduction_model 9
#define Equil_Rec_Funk_model 10
#define SS3_model 11
#define PerfectKnow 12

///* Sampling and attribute id */
//#define sample_id 0
//#define attrib_id 1

///* Sampling and assessment random numbers */
//#define startN_id 0
//#define calcF_id 1
//#define calcM_id 2
//#define nrand_id 3

//#define tass_id 0
//#define tasseat_id 1

///* Assessment and fisheries data storage */
//#define commerical_id 0
//#define survey_id 1

///* Assessment output */
//#define est_med_recruit_id 0
//#define est_top_recruit_id 1
//#define est_bot_recruit_id 2
//#define est_med_prm2_id 3
//#define est_top_prm2_id 4
//#define est_bot_prm2_id 5
//#define est_med_stock_id 6
//#define est_top_stock_id 7
//#define est_bot_stock_id 8
//#define est_SS_id 9
//#define est_FrefA_id 10
//#define est_FrefC_id 11
//#define est_Fcurr_id 12
//#define est_M_id 13
//#define est_Ccurr_id 14
//#define est_Cslope_id 15
//#define est_numyr_id 16
//#define est_CPUEnow_id 17
//#define est_CPUEstart_id 18
//#define est_balpha_id 19
//#define est_bbeta_id 20
//#define est_trend_id 21
//
//#define K_num_assess_prms 22

///* Assessment estimates */
//#define est_r_id 0
//#define est_K_id 1
//#define est_B0_id 2
//#define est_p_id 3
//#define est_B_id 4
//#define est_I_id 5
//#define est_q_id 6
//#define est_msy_id 7
//
//#define num_est_prm 8

///* Sample types */
//#define samplestock_id 0
//#define samplecatch_id 1
//#define samplediscard_id 2

///* Assessment selectivity curve types */
//#define const_q 0
//#define logistic_q 1
//#define normal_q 2
//#define lognorm_q 3
//#define gamma_q 4
//
///* Vertebrate data characteristics */
//#define bstruct_id 0
//#define bres_id 1
//#define bstocknums_id 2
//#define bcatchnums_id 3
//#define bprod_id 4
//#define beat_id 5
//#define bdiscards_id 6

///* Stock characteristics */
//#define sbiomass_id 0
//#define sstocknums_id 1
//#define scatchnums_id 2
//#define sprod_id 3
//#define seat_id 4
//#define sdiscardnums_id 5
//#define stotcatch_id 6
//#define stoteffort_id 7
//#define sdiscards_id 8
//#define ssizemat_id 9
//#define scondn_id 10

///* Individ characteristics */
//#define iweight_id 0
//#define istocknums_id 1
//#define icatchnums_id 2
//#define ibiomass_id 3
//#define idiscards_id 4
//#define iq_id 5
//#define icondn_id 6
//
//#define K_num_individ_char 7
//
///* Fishery characteristics */
//#define fcatch_id 0
//#define feffort_id 1
//#define fdiscards_id 2
//
///* Size characteristics */
//#define sizeweight_id 0
//#define sizestocknums_id 1
//#define sizecatchnums_id 2
//#define sizediscardnums_id 3
//#define sizeselectnums_id 4
//
///* Total numbers characteristics */
//#define totstocknums_id 1
//#define totcatchnums_id 2
//#define totdiscardnums_id 3
//#define totselectnums_id 4
//
///* Network characteristics */
//#define netwk_cons_id 0
//#define netwk_prod_id 1
//#define netwk_biom_id 2
//#define netwk_ex_id 3
//#define netwk_resp_id 4
//#define netwk_flow_det_id 5
//#define netwk_pprod_id 6
//#define netwk_imm_req_id 7
//
//#define K_netwk_properties 8

///* For phys[id][box][record_in_time]
//property                 id
//water column depth        0
//sediment depth            1
//salinity                  2
//light in water column     3
//light at sediment surface 4
//oxygen in sediment        5
//don in water column       6
//din in water column       7
//volume                    8
//*/
//
//#define wc_depth_id 0
//#define sed_depth_id 1
//#define salinity_id 2
//#define light_wc_id 3
//#define light_sed_id 4
//#define o2_sed_id 5
//#define o2_wc_id 6
//#define TSS_id 7
//#define don_wc_id 8
//#define din_wc_id 9
//#define vol_id 10
//#define temperature_id 11
//#define num_sampled_phy_id 12

/******* Time series ids *******/
#define interpolate_id 0
#define exact_id 1

/******* Layer handling ids *******/
#define shrinking -1
#define expanding 1

/******* Output handling ids *******/
#define out_nums_id 0
#define out_size_id 1
#define out_catch_id 2
#define out_discards_id 3

/**********************************************************************
 Performance Measure Ids
 */
//#define PDratio_id 0
//#define FPFVratio_id 1
//#define DivCount_id 2
//#define InfEpi_id 3
//#define BSSslope_id 4
//#define coverindx_id 5
//
//#define K_num_ecol_indx 6

#define mgmtstability_id 0
#define mgmtaccess_id 1
#define mgmtcost_id 2
#define frstindx_id 3

#define K_num_manage_indx 4

#define avg_sze_id 0
#define tot_land_id 1			/* Total fish caught that is being taken to port */
#define tot_num_caught_id 2
#define tot_trash_id 3
#define	gearchange_id 4
#define	discardchange_id 5
#define tepkill_id 6
#define habkill_id 7
#define tot_dayfishing_id 8
#define overallcpue_id 9		/* Total landings / tot_day fishing */
#define sociallike_id 10
#define realconflict_id 11
#define sp_comp_id 12

#define checkdone_id 13
#define K_num_harvest_indx 14

#define FEMALE_ID 0
#define MALE_ID 1

/* Economic indicators */
enum{
	fleetnum_id,
	fleet_switch_id,
	avgboatsze_id,
	overallrent_id,
	GrossVal_id,
	SubfleetCash_id,
	LeaseVal_id,
	InvestReturn_id,
	RevPerT_id,
	TradeExtent_id,
	RevPerEffort_id,
	GrossTax_id,
	GrossDV_id,

	K_num_econ_indx
};

#define portwgt_id 0
#define portpop_id 1

#define K_num_port_indx 2


/****************************************************************************
 * Ice time series structure
 */
typedef struct {
	int iceclass; /**< Ice class the time series is dealing with */
	TimeSeries ts; /**< Time series of data for ice data in this box */
	TimeSeries depthts; /**< Time series of depth data for ice in this box */
} IceTimeSeries;


/**
 * Fisheries time series structure
 */
typedef struct {
	double x; /**< Location x coordinate */
	double y; /**< Location y coordinate */
	int b; /**< Box time series centred in */
	TimeSeries ts; /**< Time series of data for fisheries data in this box */
} FisheryTimeSeries;



/**
 The ratio information structure - kept separate from DiagInfo and
 EpiInfo in case of future development needs where attributes may diverge
 */
typedef struct {
	int n; /**< ratio variable number */

	/**@name
	 *  These values are read and written by readBMEpiInfo() and
	 *	writeBMEpiInfo()
	 */
	//@{
	char name[BMSLEN]; /**< name of epi netCDF variable */
	char units[BMSLEN]; /**< Units */
	char long_name[BMSLEN]; /**< Descriptive name */
	int dtype; /**< flag to indicate whether fisheries or general data type */
	int sumtype; /**< flag to indicate whether summary data type */
	int flagid; /**< flag indicating whether to process diagnostic or not */
//@}

} RatioInfo;

/**
 The diagnostic information structure - kept separate from DiagInfo and
 EpiInfo in case of future development needs where attributes may diverge
 */
typedef struct {
	int n; /**< epi variable number */

	/**@name
	 *  These values are read and written by readBMEpiInfo() and
	 *	writeBMEpiInfo()
	 */
	//@{
	char name[BMSLEN]; /**< name of epi netCDF variable */
	char units[BMSLEN]; /**< Units */
	char long_name[BMSLEN]; /**< Descriptive name */
	int dtype; /**< flag to indicate whether fisheries or general data type */
	int sumtype; /**< flag to indicate whether summary data type */
	int flagid; /**< flag indicating whether to process diagnostic or not */
//@}

} DiagInfo;

/********************************************************************//**
 RBC estimation data structure
 **********************************************************************/
typedef enum {

	// At present all the entries below that are commented out are ones from Sally's code that are not actually used here as yet

	// assessment specifications
	//NumCPUE_id,       // number of generated CPUEs
	//NumLenF_id,       // number of generated length compositions
	//NumAgeF_id,       // number of generated age compositions
	SB0_weighted_id,    // pre-fishery spawning biomass weighted by catch in regions
	DiscType_id,        // type of discards 1 = biomass, 2 = fraction

	MaxH_id,            // maximum exploitation rate
	Growthage_L1_id,    // growth age for L1 for SS control file
	Growthage_L2_id,    // growth age for L2 for SS control file
	MinCatch_id,        // catch if RBC = 0
	TACinitial_id,      // TAC in final historic year
	Tier4_Cmaxmult_id,  // proportion CPUEmax is above CPUEtarg

	// reference levels for new Tier 4 rule
	CPUElim_id,
	Cmax_id,
	CPUEtarg_id,
	CPUEav_id,

	CPUEfleet_id,    // fleet used for CPUE in Tier 4 rule
	tiertype_id,     // marking different kind of tier rule rather than have a separate parameter for tier 3, 4 and 5

	Avlen_id,           // avlen calculated for Tier 5 avlen method
	BcurrSP_id,         // Bcurr calculated by SP model
	PropAvlen_id,       // proportion of catch > reference length for avlen method
	Maturity_Inflect_id,   //  maturity inflection for SS control file
	Maturity_Slope_id,     //  maturity slope for SS control file
	Tier3_Fcalc_id,  // type of tier3 F calc 1 = catch-curve, 2=ASPM , 3 = true F
	Tier3_mult_id,   // catch multiplier in tier 3 rule
	//Tier3_rule_id,   // version of Tier 3 rule to use : 1=original , 2=Neil's - replaced by tiertype
	Tier3_time_id,   // time period for calc Ccurr : 1=4 years, 2=no ages selected
	Tier3_maxage_id, // maximum age for Tier 3
	T1_steep_phase_id,  // = n if want T1 to estimate steepness in phase n, else -n
	Tier3_M_id,         // M input for tier 3
	Tier3_S25_id,       // length at 25% selectivity by fleet - input for tier 3
	Tier3_S50_id,       // length at 50% selectivity by fleet - input for tier 3
	Tier3_S95_id,       // length at 95% selectivity by fleet - calculated from S25 and S50
	Tier3_F_id,         // starting estimate of F for tier 3
	Tier3_h_id,         // steepness input for Tier 3
	Tier3_a_id,         // a of length-wt relationship, input for Tier 3
	Tier3_b_id,         // b of length-wt relationship, input for Tier 3
	Tier3_matlen_id,    // knife-edge length at maturity, input for Tier 3
	Tier3_maxF_id,      // max F to use in Fref calcs (2 for flathead, 4 for whiting)
	//Tier4_type_id,      // 1= original Tier 4, 2= new Tier 4 - replaced by tiertype
	Tier4_avtime_id,    // time period for calc Ccurr : 1 = 4 years, 2 = same as years for CPUE target
	Tier4_CPUEyrmin_id, // year in which to start averaging CPUE target
	Tier4_CPUEyrmax_id, // no years over which to calc CPUE target
    Tier4_Bo_correct_id, // correct scalar based on whether at Bo in reference cpue period or not
	Tier4_m_id,         // average CPUE over last m years
	Tier4_alpha_id,     // alpha value for use in Tier 4 calculations
    Tier4_r_id,         // population growth rate from dynamic tier 4
    Tier4_Carry_id,     // from dynamic tier 4
    Tier4_z_id,         // population total mortality rate from dynamic tier 4
    Tier5_length_id,    // ref. length of full selection  for avlen method
	Tier5_S50_id,       // length of knife-edge sel (50% sel) for avlen method
	Tier5_cv_id,        // cv of length-at-age used in Avlen assessment
	//Tier5_type_id,      // 1= Surplus production, 2= av length - replaced by tiertype
	Tier5_flt_id,       // which fleet's CPUE to use in SP model
	Tier5_reg_id,       // which region's CPUE to use in SP model
	Tier5_p_id,         // asymmetry parameter for surplus production model
    Tier5sel_id,        // selectivity assumed in SAFE assessment method
    Tier5q_id,          // catchability assumed in SAFE assessment method
	Nfint_id,           // no of Fs to do YPR
	PostRule_id,        // use post-assessment rule 1 or 2 or not
	MinCyr_id, MaxCyr_id, // years over which to calculate catch average
	Ncage_id,           // actual number of years catch-at-age in CCsel (used to pass info to EquilF_Funk)

	//Init_S50_id,      // global used to pass information to minimisation routine in Tier 3 calculations
	CPUEmult_id,        // CPUE change multiplier in post-assess rule - TODO make sure used
	MaxChange_id,       // maximum change of TAC allowed from post-assess rule - TODO make sure used
	F20_id,             // F20 calculated for Tier 3 rule
	F40_id,             // F40 calculated for Tier 3 rule
	F48_id,             // F48 calculated for Tier 3 rule
	//F20_phd_id,
	//F40_phd_id,
	//F48_phd_id,
    
    // Tier buffers - was call US tier scalars (USsig)
    Tier1Sig_id,
    Tier2Sig_id,
    Tier3Sig_id,
    
	// estimated values at each assessment
	RBCest_id,
	RBC_old_id,
    EstB0_id,
    EstBinit_id,
    EstBcurr_id,
	EstDepletion_id,

	// TAC after various rules applied
	TACest_id,
	TAC_old_id,
	TACdisc_id,
	TACpost_id,
    
    UseRBCAveraging_id,
    
    //MYTAC related
    myTACend_id,

	//  for tier 4 log file
	est_discard_rate_id,
	Cmult_id,
	CCcur_id,
	Fcurr_id,
	True_F_id,
	True_F48_id,
	MaxConvergCrit_id,  // maximum convergence criterion
	SigmaTAC_id,        // over-dispersion in TAC allocation
	AssessFail_id,      // flag if assessment fails
    Assess_id,
	Hsteep_id,	     // recruitment steepness
	Agesel_Pattern_id,  // age selectivity pattern (for SS control file)
	AgeSel25_id,     // age at 25%  selectivity
	AgeSel50_id,     // age at 50%  selectivity
	AgeSel95_id,     // age at 95 % selectivity
	AssessFreq_id,   // perform an assessment every AssessFreq years
	CCsel_years_id,  // number of years of catch-at-age data to use in CCsel method
	MG_offset_id,    // mortality-growth parameter offset option
	RecDevBack_id,   // number of years back from current for estimating rec devs (age of recruit)
	Regime_shift_assess_id,   // =1 if want to use regime shift in tier 1 assess, else 0
    BallParkF_id,
    BallParkYr_id,
    NumChangeLambda_id,
    num_enviro_obs_id,
    num_growth_morphs_id,

	// size parameters
	Nsex_samp_id,   // =1 if want to combine sexes in age and length samples
    Nsexes_id,      // number of sexes used in assessment
	MaxAge_id,		// maximum age
    thresh_mat_id,  // THreshold of maturity for assessment
    femsexratio_id, // Sex ratio - female component
	AccumAge_id,    // accumulator age used in SS
	HistYrMin_id,	// first year of historic data
	HistYrMax_id,	// last year of historic data
	//Nypast_id,		// number of historic years
	Nyfuture_id,    // number of future years
	Nyears_id,      // total number of years
	MaxYr_id,       // last year (history + future projection)
	Nlen_id,        // number of length bins
	Lbin_id,        // size of length bin (cm)

	// SS3 Biological parameters
	flagLAdirect_id,    // flag showing whether length at age input directly (1) or not (0)
	flagSLAdirect_id,   // flag showing if std dev of length at age input directly (1) or not (0)
	flagWAdirect_id,     // flag showing if weight at age input directly (1) or not (0)

	// recruitment parameters
	RecDevStdev_id,    // std dev of historic rec devs
	SigmaR1_id,        // std dev recruitment 1 in past
	SigmaR2_id,        // std dev recruitment 2
	SigmaR_future_id,  // sigmaR for future rec
	PSigmaR1_id,       // probability of normal recruitment
	Regime_year_id,    // start year for regime shift
	RecDevMinYr_id,    // first year of rec devs
	RecDevMaxYr_id,    // last year of rec devs
	RecDevFlag_id,      // flag indicating whether historic rec devs read in (1) or not (0)
	AutoCorRecDev_id,   // flag indicating whether rec devs auto-correlated (1) or not (0)
	NumFisheries_id,
	NumRegions_id,

	HistDataInit_id,   // flag indicating whether or not historical data generated
	AssessStart_id,    // flag indicating which year to start assessments
	TotHistCatch_id,   // Historic catch over all years
	TotAltCatch_id,    // Alternative historic catch over all years (for tuning to low depletion)
    
    isTriggerSpecies_id, // Whether a trigger species or not
    TriggerResponseScen_id,  // Tier 6 and 7 parameters
    Closed_id,         // Whether fishery closed due to trigger
    TriggerReached_id, // Trigger level reached
    RBCadjscalar_id,   // Adjustment made due to tier6 assessment
    lastRBC_id,        // Last RBC before closure in tier6
    trigq_id,          // catchability marking a species as being targeted so close under tier6
    
    // For PGMSY
    F_5yrAvg_id,
    Catch_5yrAvg_id,
    AveF_id,
    PGMSYBHalpha_id,
    PGMSYBHbeta_id,
    
    // From ratpack
    LFSSlim_id,
    AFSSlim_id,
    NumSurvey_id,
    Regime_year_assess_id,
    NblockPattern_id,
    SRBlock_id,
    assRecDevMinYear_id,
    mgt_HCRtarget_id,
    mgt_HCRbreak_id,
    mgt_HCRlimit_id,
    mgt_category_id,
    mgt_sp_id,
    mgt_indicator_id,
    init_mgt_category_id,
    init_mgt_sp_id,
    prevAssessment_id,
    PropClosed_id,
    MultispAssessType_id,
    
    // To do with indicator harvest control rule
    avgInd_deltaRBC_id,
    trigger_threshold_id,
    CPUEtrend_id,
    CPUEdir_id,
    catchtrend_id,
    
	num_rbc_species_params_id

} RBC_SPECIES_PARAMS;

// Kind of product - for RBC tiered assessments
#define retained_id 0
#define whole_id 1
#define discarded_id 2

typedef struct {

	// At present all the entries below that are commented out are ones from Sally's code that are not actually used here as yet

	// SS3 Assessment model structure
	int **Ia;         		    /* Allocation of fleets to regions */

    double *Future_catchprop;   /* Proportion of catch in each region in future */
	//double **Lambda0;           /* Fraction of recruitment for stock j in region r in initial year */
	//double *LenBin;     	    /* Length bin boundaries (lower bounds, plus upper bound of top bin) */
	//double *Lengths;    		/* Mid-point of length bins */

	// SS3 Biological parameters
	double **CvLA0;          /* CV of length-at-age by stock and sex for age 0 */
	double **CvLAmax;        /* CV of length-at-age by stock and sex for max age */
	double **Fecund;         /* Fecundity at age by stock */
	double ***MeanLenAge;    /* Mean length-at-age by stock, sex and age  */
	double ****MeanWtAge;    /* Mean weight-at-age */
	//double ****MidWtAge;     /* Mean weight-at-age  (mid-year) */
	double ***Mzero;         /* Initial natural mortality by stock,sex and age */
	double *****Mnat; 		 /* Natural mortality by stock, region, sex, age, years */
	double ****MoveProb;     /* Max average probability of moving from region to region by stock and sex - T in Sally's code */
	double ***Movrate;       /* Relative age-specific movement rate by stock and sex */
	//double ****SigmaLenAge;  /* Stdev of length-at-age by stock sex age years */
	double **VBLinf;         /* Von Bert Linfinity by stock and sex and years */
	double **VBk;            /* Von B k by stock and sex and years */
	double **VBt0;           /* Von B t0 by stock and sex and years */
	double **Wtlen_a;        /* Weight-length relationship coefficient by stock and sex */
	double **Wtlen_b;        /* Weight-length relationship exponent by stock and sex */
	double ***WtLen;         /* Weight at length */
	//double ****Xsa;        /* Movement probabilities from region to region by sex and age */
	double *****Xmove;       /* Movement probabilities from region to region by stock, sex and age */

    double *SlopeMat;         /* Maturity relationship - slope of the curve */
    double *Mat50;            /* Maturity relationship - 50% maturity */
    
	double VarDevMovProb;    /* Variance of deviations in movement probability */

	// Exploitation-related quantities
	//double ***Calc_catch;   /* Model-derived retained catch using age exploitation rates */
    double **Fhist;        /* Historical fishing pressure */
    double **Fupdate;        /* Updated fishing pressure in PGMSY */
    double **EnviroData;   /* Environmental data */
	double ***EffortData;  /* Effort data for use in calculating CPUEs */
	double ***CatchData;   /* Catches data feed to/from SS3 - retained catches by fleet, region, and
	                        year including sum over regions - called Catches in Sally's code */
	double ***DiscData;   /* Discard data feed to/from SS3 - discarded catches by region, fleet and year
							(in Sally's code: there was an array Discard for the oeprating model, as we have our own array
							we just call that for true discards and put all generated discards, historical and projection here) */
	//double ***RetCatch; /* Model-derived retained catches by region, fleet and year */
	double ***TotCatch;   /* Total catches by region, fleet and year */
	double *****Uage;       /* Exploitation rate on fish of stock j, sex s and age a in region r in year y */
	double ***Ufleet;       /* Exploitation rate on fish by fleet, region and time */

	// Selectivity
	//double *CorSel;         /* Length bin correlation for selectivity */
	//double ***CorDevSel;    /* Correlation matrix among length bins of annual deviations */
	//double ***DevSel;       /* Deviations in logit of selectivity by fleet and time */
	//double *****FracLen;    /* Fraction of fish in length bins by stock, sex, age and time */
	double *HiLenBin;         /* Upper lengths of length bins */
	double *LoLenBin;         /* Lower lengths of length bins */
	//double *****SelAge;     /* Sex- and age-specific selectivity pattern for fleet f on stock j */
	//double ***SelLen;       /* Selectivity at length by fleet and time */
	//double *VarDevSel;      /* Std dev of random walk for selectivity */
    
	// Retention
	//double *****AgeRetain;  /* Fraction of fish of each sex retained by age for each fleet,stock and year */
	int *DiscardType;    /* For each fleet, type of discards (and no of gammas) : 1 = market-based, 2 = size-based, 3 = both */
	double ***LenRetain;    /* Fraction of fish retained by length for each fleet and year */
	//double *RetGamma;       /* Retention parameters (up to 3) by fleet and year */

	// Recruitment
	//int **HaveRecDev;    	/* Flag if have recruitment dev this year (1=y,0=n) */
	//int *TransCol;        /* Transition matrix column sums */
	//int **Transition;  		/* Rec dev transition matrix for blue grenadier */

	//double **CorRecDevs;    /* Correlation of rec resids among regions */
	//double ***FracRec;      /* Expected fraction of age0's by stock assigned to regions through time */
	//double *Hsteep;         /* Steepness */
	//double ***Recruits;     /* No of recuits by stock, area and time */
	//double *R0;             /* Unfished recruitment by stock */
	//double *Rshift;         /* Equilibrium recruitment at regime shift */
	//double **RecDevs;       /* Recruitment deviations by region and time */
	//double *SigmaR;         /* sigmaR at time t */
	//double *SB0;          /* Pre-exploitation spawning biomass - commented out by Sally */
	//double ***VarRecDevs;   /* Variance-covariance matrix of the recruitment devs */

	// Temporary globals for passing information
	int GrangerI;
	int Sim;

	//double **GrangerM;

	// Derived quantities
	double *****Num;         /* Number of animals stock j, sex s, age a, region r */
	double ***RetVulBio;     /* Retainable vulnerable biomass by fleet,region,time */
	//double ******RetVulBio_byall;  /* Retainable vulnerable biomass by stock, region, fleet, sex, age, time */
	double **SBioZero_shift; /* SB0 for regime shift by stock and region */
	double **SBioZero;       /* Pre-fishery spawning biomass by stock and region */
	double ***SpawnBio;      /* Spawning biomass by stock, region, time */
	//double ***VulBio;        /* Vulnerable biomass by fleet, region, time */

	// Data generation
	int *AgeFuture;          /* Flag for  future age  data (retained, whole, discard) */
	int *AgeN;               /* Sample size of age comps  (retained, whole, discard) */
	int **AgeYears;          /* Flags which years for which to generate age comps (retained, whole, discard) */
	int ***AgeFltYr;         /* Flags AF years, fleets in past (r,w,d) */
	int *CPUEfuture;         /* Flag for future CPUE by fleet */
	int ***CPUEyears;        /* Flags which years (by fleet, region) for which to generate cpue */
	int *DiscFuture;	     /* Flag for future discard by fleet */
	int **DiscYears;         /* Flags which years (by fleet) for which to generate discards to feed to/from SS3 */
	int *LengthFuture;       /* Flag for future length data (retained, whole, discard) */
	int *LengthN;            /* Sample size of length comps  (retained, whole, discard) */
	int **LengthYears;       /* flags which years for which to generate length comps (retained, whole, discard) */
	int ***LengthFltYr;      /* Flags LF years, fleets in past (r,w,d) */

	int *****LenComp;        /* Generated length composition data */
	int *****AgeComp;        /* Generated age composition data */
	int ****LFss;            /* Sample sizes for each length frequency (LF) */
	int ****AFss;            /* Sample sizes for each age frequency */
    
    int *Sel_Pattern;       /* Selectivity pattern */
    int *mirrored_fleet;
    
    double **Start_Sel;
    double **SelBlock;
    double **Sel_Phase;

	double *CPUEqmu;         /* CPUE average catchability by fleet */
	double *CPUEpow;         /* CPUE power parameter by fleet */
	double ***CPUEq;         /* Generated catchability by fleet, region and year */
	double *CPUEcv;          /* CPUE CV by fleet */
	double *CPUEvar;         /* CPUE variance in random walk */
	double *CPUEcorr;        /* CPUE temporal correlation in cpue random walk */
	double ***CPUEdevs;      /* Rate of change in q by fleet, region and year */
	double ***CPUEgen;       /* Generated CPUE series by fleet and year */
	double *CatchCV;         /* Catch cv by fleet */
	double *DiscCV;          /* Discard cv by fleet */
	double *EffortCV;        /* Effort cv by fleet */

	// Assessment specifications
	int *regID;				 /* Identification of assessment regions per group */
	int CurrentYear;         /* Current year needed for SP model minimisation */
	int Warnings;            /* Number of warnings */

	char *SSDirName;         /* Directory name for where to write SS files */

	double *Ageing_error;     /* Aging error matrix. dimensions 0..AccumAge (SS data file) */
	double **SSMort;          /* M for SS control file (per stock and sex) */
	double **Obsageprop;      /* Global used to pass information to minimisation routine in Tier 3 calculations */
	double *Start_SelInflect; /* Starting values of selectivity parameters for SS control file - per fleet */
	double *Start_SelWidth;   /* Starting values of selectivity parameters for SS control file - per fleet */
	double *Start_RetInflect; /* Starting values of retention parameters for SS control file - per fleet */
	double *Start_RetSlope;   /* Starting values of retention parameters for SS control file */
	double *Tier3_Linf;       /* Linf input for tier 3 (from table 1 in Neil's tier 3 document) */
	double *Tier3_k;          /* k input for tier 3 */
	double *Tier3_t0;         /* t0 input for tier 3 */
	double *Tier5_Bstart;     /* Initial B for surplus production model */
	double *Tier5_r;          /* Initial r (and bounds) for surplus production model */
	double *Tier5_K;          /* Initial K (B0) for surplus production model */
	double *T5_removal;       /* Total removals by year */
	double *T5_Fval;          /* Fvals for Tier 5 avlen method */
	double *T5_Avlen_atF;     /* Average length at F for Tier 5 avlen method */
	double *TAC_mult; 		  /* Multiplier on the TAC - vector over future years */
	double *Varadj_CPUE;      /* Variance adjustment for CPUE (SS control file) */
	double *Varadj_discard;   /* Variance adjustment for discards (SS control file) */
	double *Varadj_length;    /* Variance adjustment for length (SS control file) */
	double *Varadj_age;       /* Variance adjustment for age (SS control file) */

	// HSF implementation
	double **Xi;              /* TAC allocation parameter xi by fleet and region */
	double **Psi;             /* TAC allocation parameter psi by fleet and region */
	double **Zeta;            /* TAC allocation parameter zeta by fleet and region */
    
     /** Arrays used in the assessment itself - were created and freed within functions for Wayte code **/
    double *propR;
    double **propSR;
    double *propCount;
    
    double **actualcatch;
    double **actualeffort;
    double **actualdiscard;
    double ***actualbiom;
    double ***actualcatchsize;
    
    double ****rawdata;
    double *****rawsizedata;
    double *****rawnumdata;
    double ****ndata;
    
    double **lenprops;
    double *lencomp;
    double **ageprops;
    double *agecomp;
    
    double **propvec;
    double ***newvec;
    
    double *props;
    
    double ****CKSurveyNum;
    
    double **propcatch;
    double ***propcatch_reg;
    double *avprop;
    double **avprop_reg;
    
    double *discrate;
    
    double **agesel_bysex;
    double **fval;
    
    double *agesel;
    double *mat;
    double *wt;
    double *fvalA;
    double *ageref;
    double *len;
    double **p_al;
    double *sumcatch;
    
    double *numatage;
    double *ages;
    int *propgt05;
    double *z;
    
    double *totnumatage;
    double **sel;
    double *chi;
    double *fvalF;
    
    double **xunit;
    double *xpar;
    double **simplex;
    double *scale;
    double *func;
    
    double *yield;
    double *yield_mid;
    double *exp_prop;
    double *num;
    double *Funkz;
    
    double *xyear;
    double *ycpue;
    
    double *expectB;
    double *expectq;
    
    double *TSbiomass;
    
    double **TriggerPoints;
    int *TriggerReached;
    double *AnnCPUE;
    
    double **CatchComp;
    double **relareafished;
    double *cpue;
    
    int mgt_Retro_catch;
    int mgt_dataYear;
    int mgt_rbcYear;
    int mgt_assessYear;
    int MaxYr;
    int NassessFail;
    int CPUEfleet;
    
    double mgt_tier4_MSYL;
    double mgt_tier4_CVMSYL;
    double mgt_tier4_estR;
    double mgt_tier4_estZ;
    double mgt_tier4_MSYRY1;
    double mgt_tier4_MSYRY2;
    double mgt_tier4_BtargtTier4;
    double mgt_tier4_MSYPriorY1;
    double mgt_tier4_MSYPriorY2;
    double mgt_tier4_PriorMeanR;
    double mgt_tier4_PriorSDr;
    double PropClosed;
    double MaxConvergCrit;
    double EstB0;
    double EstBinit;
    double EstBcurr;
    double EstDepletion;
    double MSY;
    
    double mgtTAC_old;

    int AssessFail;
    
    int *MetierUsed;
    int *NblockperPatt;  // NblockPattern_id in size
    int *discfleet;

    double *initialEquilCatch;
    double *StatusQuoCatch;
    double *R02;
    double *SSHsteep;
    double **RBC_post_PGMSY_by_year;
    double *RBC_by_year;
    double *FFs;
    double *PGMSY_q;
    double *PGMSY_selcurve;
    double *PGMSY_sel_lsm;
    double *PGMSY_sel_sigma;
    double *CscalarMetier;
    
    double **AvgCatFleet;
    double **Fupdated;
    double **RBCupdated;
    double **CatchStore;

    double***** FracLenS;   // fraction of fish in length bins by stock, sex, age and time (start-year)
    //double***** FracLenM;   // fraction of fish in length bins by stock, sex, age and time (mid-year)
    
    double ****MeanLenAgeM;
    double ****SigmaLenAgeM;
    double ****MeanWtAgeS;
    double ****MeanWtAgeM;

    double **Blocks;
    double *Mortality;
    double ****MeanLenAgeS;
    
} RBCarrays;

typedef struct {
    
    int PGMSYlinks;
    char metierCode[20];  /**< The code used to identify the metier */
    
} METarrays;

typedef struct {

	double **RBCspeciesParam;
	RBCarrays *RBCspeciesArray;
    METarrays *metierArray;
    
    // SS relevant parameters
    int UseAtlantisPGMSY; /* Flag to indicate whether to use Atlantis version of PGMSY */
    int UseSS;	         /* Flag indicating whether using SS for Tier 1 (1) or if using perfect info + error (0) */
    int UseTierBuffers;  /* Flag indicating whether using tier buffers (used to say US tiers) */
    double myTACbuffer;  /* Buffer used for myTAC multi-year TACs in subsequent years */
    int myTACperiod;     /* Length of myTAC */
    int AssessDelay;     /* Length of delay in the assessment process (years from data collection to RBC setting) */
	int SSForecastType;  /* SS Forecast type */
    double SSFscalar;    /* SS F scalar */
    int SSControlRule;   /* SS Control Rule method */
    double SSControlRuleB; /* SS Control Rule biomass */
    double SSControlRuleTargF; /*SS Control Rule target F */
    int SSLoop;          /* SS forecast loops */
    int SSLoopWithRandRec; /* SS forecast loop where stochastic ercruitment introduced */
    double SSTol;        /* SS convergence tolerance */
    int SSDepletionBasis;  /* SS Depletion basis */
    double SSFractX;     /* SS Depletion denominator */
    int SS_SPRreport;    /* SS SPR reporting flag */
    int SS_Freport;      /* SS F reporting flag */
    int SSFreportBasis;  /* SS F reporting basis */
    int SSnoBiasAdj;       /* SS last year of no bias adjustment - relative to model start */
    int SSnumGrowthPatterns;  /* SS Number of growth patterns */

    double sexratio_cv; /* Sexratio - female component */
    
    int earliestYr; /* Earliest yar of a multispecies assessment */
    int ReviewPeriod; /* Review period for indicator species */
    int ReviewYr;  /* Year review happening - updated within the run */
    int TriggerCheckPeriod;  /* Period of time over which Trigger criteria checked (i.e. length of "longterm" reference period) */
    int LagPeriod; /* Lag period in assessment process */
    int GradientWindow; /* Width of window that indicator species method uses to check for matching CPUE gradient */
    int GradientPeriod; /* Lag period on data used to judge indicator gradient */
    double GradientBuffer; /* Buffer around scores used in judging whether closest or not for the purposes of pairing up species */
    int UseCategory; /* Whether indicator species approach using a category to track against (1) or only a single species (0) */
    int UseClosest; /* Whether using closest regardless of category (1) or only the same category as you (0) */
    int UseTriggerMgmt; /* Whether using trigger based management - so basically rolling TAC until trigger conditions met */
    int ProjYr; /* Length of the projeciton period of assessments */
    double  ThresholdDepletion; /* Threshold for depletion levels for PGMSY */
    double ThresholdBound; /* Bound on convergence of depletion levels for PGMSY */
    int MaxIteration; /* Maximum iterations allowed in PGMSY */
    
	int initHistFileDone;  /*Flag indicating whether the history file has been initialised or not */
	//int Tier3_Fcalc;  /* Type of tier3 F calculation to use: 1 = catch-curve, 2 = ASPM, 3 = true F */
	double min_TACchange;
	int Use_TAC_mult; /* If want TAC multiplier (1) or not (0) */
	int Nsims;	      /* Number of simulations */
	int Tier_Max_Int;  /* Maximum number of iterations of downhill_simplex */
	int nFuture;			 /* Number of future years for prejections in the assessments */
	int HistYrMin;		/* Minimum historical year */
	int HistYrMax;		/* Max historical year */
	int OverallMaxAge;  /* Overall max age across all species */
	int lastTierSp;		/* ID of the functional group that is the last species under tiered assessment */
	double CPUElim;		/* Scalar on CPUEtarg to get CPUElim */

	// Tier 3 rule parameters
	double tier3RuleA;
	double tier3RuleB;
	double tier3RuleC;
	double tier3RuleD;
	double tier3RuleE;
	double tier3RuleF;
    
    double tier5JensenConst;
    double tier5limscalar;
    double tier5omega;
    double tier5DCACscalar;
    double tier5DCACconst;

    double tier7areachange;
    double tier7relarea;
    double tier7cpuechange;
    double tier7maxcompchange;
    double tier7maxannchange;
    double tier7period;
    
    int NumTriggers;

	// SS3 assessment random number seeds
	int Iseedz;   /* Seed for operating model selectivity */
	int Iseedx;   /* Seed for sigmaRs */
	int Iseedy;   /* Seed for fake Tier 1 */
	int SaveIseedx;   /* Save seeds at start of each sim in case want to reproduce for debugging */
	int SaveIseedz;

	double num_avg_year;
	double Tier1_cv;  /* CV to use on true depletion if not using SS */

	int RetDirect;    /* =1 if retention at length read directly, else 0 - TODO: does this need to be a species level parameter? */

	int DiscType;
    
    int tier6_to_tier4;   /* Which tier tier6 and tier7 steps to once trigger tripped */
    double delTAE;        /* Delta E for empirical harvest decision rule for tier 6 based on CPUE */
    
    // Mutispecies harvest related parameters
    int MULTISPestNspecies;
    int Lastsp;
    int Nmetiers;
    int LastMetier;
    int nRBCaverage;
    int sim;
    
    double MaxCritConvergeValue;        //0.1 and 10.0 in Ratpack
    double ConvergeThresh;              //0.01
    
    char Rlibpath[200];
    
    int **speciesMetierToRPFleet;
    int **speciesRPFleetToMetier;
    int *MULTISPestIDs;
    int *count_sp_in_buffer;
    
    double ****Catch_by_Metier;
    
    double *catchwghtCPUE;
    double *catchwght;
    
    double **F_actFleet;
    double **FratioFleet;
    double **Fhist;

} RBCstructure;

typedef struct {
    int UseCloseKin;      /* Whetehr use close kin */
    int nTotSamps;          //4000
    //int Nstocks;            // Assued number of stocks for close kin calculations
    int first_sy;           //2016
    int last_sy;            //2018
    int first_dy;           //2005
    int Nscale;             //1000
    int ThreshPercLost;     //5
    int samp_power;         //1
    int a_ck_thresh;        //5
    int a_ck_thresh2;       //10
    double a_ck_scalar;     //10.0
    double a_ck_scalar2;    //2.0
    
    int nCKsamples_proj;
    int nCKsamples_hist;
    int nCKsampled;
    
    double thresh_mat;       //0.1
    double HSP_false_neg_loss_rate;    //0.75
    

    double *fec_expo_s;      // size Nsexes
    double **fec_sa;         // size MaxAge, Nsexes   - declare in this order at intialisation
    
    double *lengths;         // size Nlen
    double *mature;          // size Nlen
    

    double ***samp_prop_to;  // size MaxAge, years, Nsexes

} CloseKinstructure;
/*******************************************************************//**
 The fisheries information structure - kept separate from FstatInfo and
 EpiInfo in case of future development needs where attributes may diverge
 *********************************************************************/
typedef struct {
	int n; /**< fstat variable number */

	/**@name
	 * These values are read and written by readBMEpiInfo() and
	 * writeBMEpiInfo()
	 */
	/*@{*/
	char name[BMSLEN]; /**< name of epi netCDF variable */
	char units[BMSLEN]; /**< Units */
	char long_name[BMSLEN]; /**< Descriptive name */
	int dtype; /**< flag to indicate whether fisheries or general data type */
	int sumtype; /**< flag to indicate whether summary data type */
	int flagid; /**< flag indicating whether to process diagnostic or not */
/*@}*/

} FstatInfo;

/*******************************************************************//**
 The epibenthos information structure
 *********************************************************************/
typedef struct {
	int n; /**< epi variable number */

	/**@name
	 *
	 *		These values are read and written by readBMEpiInfo() and
	 *		writeBMEpiInfo()
	 *
	 */
	/*@{*/
	char name[BMSLEN]; /**< name of epi netCDF variable */
	char units[BMSLEN]; /**< Units */
	char long_name[BMSLEN]; /**< Descriptive name */
	int dtype; /**< flag to indicate whether fisheries or general data type */
	int sumtype; /**< flag to indicate whether summary data type */
	int flagid; /**< flag indicating whether sensitive group is active */
	int isUsed;
/*@}*/

} EpiInfo;

/*******************************************************************//**
 \struct The tracer information structure
 *********************************************************************/
typedef struct {
	int n; /**< tracer number */
	/**@name
	 * These values are read and written by readBMEpiInfo() and
	 * writeBMEpiInfo()
	 */
	/*@{*/
	char name[BMSLEN]; /**< Name of tracer netCDF variable */
	char units[BMSLEN]; /**< Units for tracer */
	char long_name[BMSLEN]; /**< Descriptive name for tracer */
	int dtype; /**< flag to indicate whether fisheries or general data type */
	int sumtype; /**< flag to indicate whether summary data type */
	int inwc; /**< flag => exists in water, in water column */
	int insed; /**< flag => exists in sediments, in sediment layer */
	int inice; /**< flag => exists in ice, in ice layers */
	double decay; /**< Decay rate */
	int dissol; /**< flag => dissolved */
	int partic; /**< flag => particulate */
	int flagid; /**< flag indicating whether sensitive group is active */
	int isUsed;
	/*@}*/

	/**@name
	 * The following values are only applicable to particulate
	 * tracers (those which have the partic flag set)
	 */
	/*@{*/
	int passive; /**< flag indicating whether group is active or passive with
	 regard to advection and movement with currents
	 (1=passively advected, 2=actively move without advection) */
	double svel; /**< Settling velocity */
	double xvel; /**< Extra "settling" velocity due to migration or
	 nutrient limitation (from ERSEM formulations) */
	double psize; /**< Particle size */
	double b_dens; /**< Particle bulk density */
	double i_conc; /**< Initial deposit concentration */
    
    int can_be_moved;  /**< Whether it should be moved or not - so Aragoniste saturation and rugosity aren't */
/*@}*/
} TracerInfo;

/*******************************************************************//**
 Export set-up for linking to other models - EwE for start
 ********************************************************************/
typedef struct {
	int numrows; /**< Number of rows of external model the box overlaps */
	int numcols; /**< Number of cols of external model the box overlaps */

	int *rows; /**< Array of external model row ids that the box overlaps */
	int *cols; /**< Array of external model col ids that the box overlaps */

	double **bcellarea; /**< Area of overlap between the external model cell and Atlantis box */

} ExportInfo;

/*****************************************************************
 * Home range data structure - to be hung off boxes data structure
 *****************************************************************/
typedef struct {
	int n; /**< Number of homeranges */
	int *ids; /**< ID of homeranges in the box */
	double **rangenum; /**< Numbers of cohort j in home range i */
	double *rangewgt; /**< Weighting contribution of this box in home range i */

} HomeRange;

/*******************************************************************//**
 The sediment model structure

 Note that for any arrays here which have vertical
 extent, vertical indices (k values) start with 0 at the
 layer closest to the water (uppermost layer).
 *********************************************************************/
typedef struct {
	int nz; /**< Max number of sediment layers */
	double maxdz; /**< Maximum uncompacted layer thickness */
	double mindz; /**< Minimum layer thickness */
	int ntr; /**< number of tracers */

	/**@name
	 * These values may change during a run, and are read
	 * and written by readBMphysData() and writeBMphysData()
	 * and readBMTracerData() and writeBMTracerData().
	 */
	/*@{*/
	int topk; /**< Index of first non-empty layer */
	double biodepth; /**< Max depth for biological activity */
	double detdepth; /**< Max depth for detritus (changes with time) */
	double oxdepth; /**< Oxygen horizon (changes with time) */
	double biodens; /**< Measure of biological activity (animals
	 * per square metre?) */
	double irrigenh; /**< Scalar of biological activity enhancement
	 due to the actions of infaunal species. */
	double turbenh; /**< Scalar of biological activity enhancement
	 due to the actions of infaunal species. */
	double *volume; /**< layer volumes */
	double *eflux; /**< net horizontal exchange for the layer */
	double *vflux; /**< net vertical exchange for the layer */
	double *hdsource; /**< number of boxes feeding each layer */
	double *hdsink; /**< number of boxes fed by each layer */
	double *dz; /**< Layer thicknesses */
	double *gridz; /**< Layer interface z-coords, referenced
	 * to the bottom of the water column and
	 * progressing downwards.
	 */
	double *cellz; /**< Layer centre z-coords, referenced
	 * as for gridz above
	 */
	double *filltime; /**< Time layer became full */
	double *css; /**< Critical shear stress */
	double *er; /**< Erosion rate constant (ms-1) */
	double *porosity; /**< Porosity (fraction by volume which is water) */
	double *dissol_kz; /**< diffusion cfft for dissolved things */
	double *partic_kz; /**< diffusion cfft for particulate things */
	double **tr; /**< tracer values */
	double *fisheries; /**< Fisheries statistics */
/*@}*/
} SedModel;

/*******************************************************************//**
 The Ice model structure

 Note that for any arrays here which have vertical
 extent, vertical indices (k values) start with 0 at the
 layer closest to the water (lowermost layer).
 *********************************************************************/
typedef struct {
	int nz; /**< Max number of ice layers */
	int currentnz; /**< Current number of ice layers */
	double maxdz; /**< Maximum uncompacted layer thickness */
	double mindz; /**< Minimum layer thickness */
	int ntr; /**< number of tracers */
	int n_ice_types; /**< number of ice class types */
	int is_freezing; /**< whether ice sheet is mainly freezing
						 or melting - for flux purposes
						 0 = no change
						 1 = freezing
						 -1 = melting */
	double last_depth;  /**< Last depth the ice reached - so know if freezing or melting */
	double ice_growth_rate; /**< Amount (m) that ice depth has changed in the last time step */

	double max_ice_depth;
	int fast_ice;
	double *volume; /**< layer volumes */
	double *dz; /**< Layer thicknesses */
	double *gridz; /**< Layer interface z-coords, referenced
	 * to the bottom of the water column and
	 * progressing downwards.
	 */
	double *cellz; /**< Layer centre z-coords, referenced
	 * as for gridz above
	 */
	double **ice_classes; /**< Proportional presence of different
	 * classes of ice
	 */
	int *wc_layer_match; /**< Match of ice layer to water column layers */
	double *porosity; /**< Porosity (fraction by volume which is water) */
	double *er; /**< Erosion rate constant (ms-1) */
	double **tr; /**< tracer values */

	double bottom_light; /* The light at the bottom of all of the ice in the layer */
/*@}*/
} IceModel;

/*******************************************************************//**
 The land model structure

 No vertical structure, only 2D surface assumed (with botz in main
 box to define "height"). Will grow as terrestrial components fleshed out

 *********************************************************************/
typedef struct {
	int ntr; /**< number of tracers */
	double *tr; /**< tracer values - only a single vector as effectively 2D surface */
	double *exploitation; /**< Like fisheries statistics for other parts of the system
	 * this may not be needed, but place holder for other industries to be developed in
	 * future applications */
/*@}*/

	double surface_light; /* The light land surface. */

} LandModel;


/*******************************************************************//**
	Parameters associated with tracking C:P:N ratios in functional groups.
*********************************************************************/
typedef struct {
	double RTOP; /** Rates of transformation from available nutrient forms to available forms in days-1 */

	double PCM_respiration;
	double TOP_respiration;

	double PCM_excretion;
	double TOP_excretion;

	double PCM_decomp;
	double TOP_decomp;

	double C_respiration;
	double C_decomp;

	double P_trans;			/** Transformation from unavailable back to available P. */
	double C_trans;			/** Transformation from unavailable back to available P. */

	double TOP_death;

	char **atomicName; /* Names of additional atomic values */

	double K_Lc;	/**< Inorganic carbon exchange rate */


} AtomicRatioStructure;

/* Box types */
typedef enum {
	INTERNALBOX = 0,
	BOUNDARY,
	LAND
}BOX_TYPE;
/*******************************************************************//**
 The box structure

 The model has a number of boxes - they are defined by the model input files.
 Each box has a number of layers including water column layers and sediment layers.
 The number of water column layers is constant across all boxes ( ? i think )
 but the number of sediment layers varies from box to box depending on the model
 input parameters.


 Not sure what the faces are ? Are these the faces of each box?


 Note that for any arrays in the box which have vertical
 extent, vertical indices (k values) start with 0 at the
 deepest layer.
 *********************************************************************/

typedef struct {

	int n; /**< box number (starts at 0) */
	BOX_TYPE type; /**< box type (boundary or internal */
	int edge_type; /**< switch to show whether boundary box is standard,
	 absorptive or relfective */
	int nz; /**< number of water column layers */
	int numlayers; /**< number of water column layers for print out */
	int ntr; /**< number of tracers */

	/**@name
	 * These values don't change during a run, and are read
	 * and written by readBoxGeom() and writeBoxGeom()
	 */
	/*@{*/
	char label[BMSLEN]; /**< box label */
	dpoint inside; /**< a point inside the box */
	int nconn; /**< number of connections (number of
	 entries in iface and ibox below) */
	int *iface; /**< array of open face indices */
	int *ibox; /**< array of neighbour box indices
	 (same order as iface) */
	double botz; /**< z-coord of bottom (-depth) */
	double current_botz; /**< Current bottom depth (only differs
	 to botz if tidal */
	double *gridz; /**< nominal layer interface z-coords (nz+1
	 values - includes eta) */
	double *cellz; /**< nominal layer centre z-coords */
	polyline *bnd; /**< boundary of box */
	double area; /**< area of box */
	double horizmix; /**< horiz mixing rate for that box */
	double vertmix; /**< vertical mixing rate for that box */
	
    int relax_tol; /**< whether can/should relax the tolerance of the adaptive difference method for this box */
    
    /*@}*/
	/**@name
	 * These values may change during a run, and are read
	 * and written by readBMTracerData() and writeBMTracerData(),
	 * readBMphysData() and writeBMphysData().
	 */
	/*@{*/
	double *volume; /**< volume of box layers */
	double *eflux; /**< net horizontal exchange for the layer */
	double *vflux; /**< net vertical exchange for the layer */
	double *hdsource; /**< number of boxes feeding each layer */
	double *hdsink; /**< number of boxes fed by each layer */
    double *flush_in; /**< count of days since inflow */
    double *flush_out; /**< count of days since outflow */
	double *dz; /**< thickness of box layers */
	double *kz; /**< diffusion cfft at layer interfaces */
	double **tr; /**< tracer values for each layer - note
	 that this array is allocated so
	 that value for tracer n for layer k
	 is tr[k][n] */
	SedModel sm; /**< Sediment model for this box - see
	 SedModel structure above for details */


	IceModel ice; /**< Ice model for this box - see
		 IceModel structure above for details */


	LandModel land; /**< terrestrial model for this box - see
			 LandModel structure above for details */

	double *epi; /**< Epibenthos variables - these
	 are a special compartment in each
	 box where the sediment surface biology
	 happens! */
	double *diagnost; /**< Diagnostic outputs - this where growth
	 and mortality values are stored */
	double *fishstat; /**< Fisheries statistics - this is where
	 the fisheries catch, effort and observer
	 data is stored */

	double *ratio; /**< Ratio statistics - this is where
	 the info about P:N:C values are stored */

	double ***vert_vdistrib; /**< distribution of vertebrates per box
	 - so can condition time series of catch */
	double erosion_rate; /**< Erosion rate (m s-1) */
	double swr; /**< Short-wave radiation input */
	double stress; /**< Bottom stress input*/
	double reef; /**< Percent rocky bottom */
	double soft; /**< Percent soft sediments */
	double flat; /**< Percent flat sediments */
	double canyon; /**< Percentage area made up by canyons */
	double eddy; /**< Strength of eddy in the box */
    double base_rugosity; /**< Base rugosity - value read in which covers geology not reefs, which is then calculated dynamically */
    double vmix_scale; /**< Scalar for vertical mixing - if its being set dynamically through time */
    
	double bottNH; /**< Original bottom NH for use in resetting flux through open bottom boundary
	 layer in open ocean boxes */
	double bottNO; /**< Original bottom NO for use in resetting flux through open bottom boundary
	 layer in open ocean boxes */
    double bottO2; /**< Original bottom Oxygen for use in resetting flux through open bottom boundary
     layer in open ocean boxes */
    double bottSi; /**< Original bottom Si for use in resetting flux through open bottom boundary
	 layer in open ocean boxes */
	double bottFe; /**< Original bottom Fe for use in resetting flux through open bottom boundary
	 layer in open ocean boxes */

	double bottP; /**< Original bottom P for use in resetting flux through open bottom boundary
		 layer in open ocean boxes */

	double bottC; /**< Original bottom C for use in resetting flux through open bottom boundary
		 layer in open ocean boxes */


	int zone; /**< Fishing zone the box is in */

	double prop_light_time; /**< proportion of the time step that is daylight in this box */

	HomeRange *HomeRangeInfo; /**< Information on home ranges for groups in this box */
	/*@}*/

	ExportInfo *export; /**< Export structure for linking to other models like EwE */

	/*@}*/
	/**@name
	 * Terrestrial related
	 */
	/*@{*/
	int has_land;				/**< whether land in this box */
	double land_habitat_qual;	/**< quality of land as habitat for terrestrial species
									TODO: this may need to be a vector across terrestrial habitat types in the future */
	/*@}*/

	/*@}*/
	/**@name
	 * Ice related
	 */
	/*@{*/
	int has_ice;
	double tot_ice_prop;
	double **ice_prop;
	// Note quality of different ice types stored in ice_classes array

	/* Unique adjacent boxes */
	int nUniqueAdj;
	int *uniqueAdjBoxes;

    /* Ordered list of boxes (IDs) - closest to furtherest */
    int *distID;
    
} Box;

/*******************************************************************//**
 The face structure. The face geometry is defined by the two points
 p1 and p2. The positive direction for fluxes etc is defined
 to be to the right when looking from p1 to p2. I'm not sure whether
 this is compatible with John Hunters definition or not.
 *********************************************************************/
typedef struct {
	/**@name
	 * These values don't change during a run, and are read
	 * and written by readFaceGeom() and writeFaceGeom()
	 */
	/*{@*/
	int n; /**< face number (starts at 0) */
	dpoint p1; /**< first point coordinates */
	dpoint p2; /**< second point coordinates */
	double len; /**< length of face */
	double cos; /**< cos angle from face normal to x-axis */
	double sin; /**< sin angle from face normal to x-axis */
	int ibl; /**< index of box to left */
	int ibr; /**< index of box to right */
/*@}*/
} Face;

/*******************************************************************//**
 The hydrodynamic model data input structure
 *********************************************************************/
typedef struct {
	/**@name
	 * Input file management
	 */
	/*@{*/
	char **fname; /**< List of file names */
	int nfiles; /**< Number of input files */
	int curfile; /**< Index of currently used file */
	/*@}*/

	/**@name
	 * Time variables
	 */
	/*@{*/
	double t; /* Hydrodynamic time (as obtained from
	 * hydrodynamic model - this may get
	 * used to access other forcing files
	 */
	char t_units[BMSLEN]; /**< Hydrodynamic time units */
	double dt; /**< Hydrodynamic time step */
	double tleft; /**< Time left in this hydrodynamic time
	 * step.
	 */
	/*@}*/

	/**@name
	 * Data buffers - these hold data for possibly several time steps
	 */
	/*@{*/
	double *tbuf; /**< Buffer for time values */
	double ****ebuf; /**< Buffer for cell exchange data */
	int ****bbuf; /**< Buffer for cell exchange b indices */
	int ****kbuf; /**< Buffer for cell exchange k indices */
	/*@}*/

	/**@name
	 * Info used for accessing buffers
	 */
	/*@{*/
	long nbuf; /**< Number of time steps in buffers */
	long bufstart; /**< First time step in buffers */
	long bufend; /**< Last time step in buffers */
	long nextrec; /**< Next record to be used */
	/*@}*/

	/**@name
	 * Info about input file
	 */
	/*@{*/
	int fid; /**< netCDF id for input file */
	int t_did; /**< t dimension id */
	int b_did; /**< b dimension id */
	int z_did; /**< z dimension id */
	int d_did; /**< dest dimension id */
	int t_vid; /**< time variable id */
	int e_vid; /**< exchange variable id */
	int b_vid; /**< dest_b variable id */
	int k_vid; /**< dest_k variable id */
	long nstep; /**< Number of time steps in file */
	long dsize; /**< size of dest dimension */
	/*@}*/

	/**@name
	 * Pointers to input data for 1 time step - these
	 * will be set to point to data for one time step
	 * contained in the input buffers above
	 */
	/*@{*/
	double ***exch; /**< Buffer for cell exchange data */
	int ***b; /**< Buffer for cell exchange b indices */
	int ***k; /**< Buffer for cell exchange k indices */
/*@}*/

} HydroData;



/* Definitions of forcing tracer that correspond to box variables instead of tracers 
   Using this method so we can add other later as required 
*/
typedef enum{
	EDDY_ID,
    VERTMIX_ID,
} BOX_VALUE_INDEX;

/*******************************************************************//**
 The temperature and salinity data input structure
 *********************************************************************/
typedef struct {
	/**@name
	 * Input file management
	 */
	/*@{*/
	int nFiles;
	char **fname; /**< File names */
	int curFile; /**< Index of currently used file */
	char *variableName;
	int is_valid_z;	/* true if there is layer data - otherwise this is box data */
	/*@}*/

	/**@name
	 * Time variables
	 */
	/*@{*/
	double t; /**< Time */
	char t_units[BMSLEN]; /**< Time units */
	double dt; /**< Time step */
	double tleft; /**< Time left in this time step.*/
	/*@}*/

	/**@name
	 * Data buffers - these hold data for possibly several time steps
	 */
	/*@{*/
	double *tbuf; /**< Buffer for time values */
	double ***valuebuf; /**< Buffer for cell exchange data */

	/*@}*/

	/**@name
	 * Info used for accessing buffers
	 */
	/*@{*/
	long nbuf; /**< Number of time steps in buffers */
	long bufstart; /**< First time step in buffers */
	long bufend; /**< Last time step in buffers */
	long nextrec; /**< Next record to be used */
	/*@}*/

	/**@name
	 * Info about input file
	 */
	/*@{*/
	int fid; /**< netCDF id for input file */
	int t_did; /**< t dimension id for input file */
	int b_did; /**< b dimension id for input file */
	int z_did; /**< z dimension id for input file */

	int prop_vid; /**< property variable id */
	int t_vid; /**< time variable id */
	long nstep; /**< Number of time steps in file */
	/*@}*/

	/**@name
	 * Pointers to input data for 1 time step - these
	 * will be set to point to data for one time step
	 * contained in the input buffers above
	 */
	/*@{*/
	double **dataBuffer; /**< Buffer for data */

	/* Min and max values allowed */
	double min_value;
	double max_value;

	int tracerID; /* link to the tracer in the model - for movement entries it is the group ID */
    int stageID; /* For movement cases need to know what stage it is */
	double missing_value;
	int missing_value_set;
	char tracer_units[BMSLEN]; /**< tracer units */

	/* Rewind option */
	int rewind;
	int atEnd;

    /* Data assimilation weighting */
    double wgt_coefft;
    int *use_resets; /* Whether Atantis should reset it's values based on this, rather than adding in new values */
    double ResetTol; /* The buffer around the Atlantis value before the reset value is used instead */
    double total_input;  /* Track of total material added */

    /* Timing of start of activity */
    double movetstart;
    int moveInitDone;
    
	/* true if the tracer is an epi tracer */
	int isEpi;

    /* true of the input is a scalar */
    int isScalar;
    
	int isBoxValue;					/* True if the force tracer corresponds to a box value such as eddy value instead of a tracer value */
	BOX_VALUE_INDEX boxValueIndex;	/* The index of the variable name  - allows for easy expansion later */

/*@}*/

} PhyPropertyData;


typedef enum{
	MORT_INDEX_ID,
	GROWTH_INDEX_ID,
	RECRUITMENT_INDEX_ID,
	NUM_BIOLOGY_SCALAR
} BIOLOGY_SCALAR;

typedef struct {
	/**@name
	 * Input file management
	 */
	/*@{*/
	int nFiles;
	char fname[BMSLEN]; /**< File names */
	char **variableName;
	/*@}*/

	/**@name
	 * Time variables
	 */
	/*@{*/
	double t; /**< Time */
	char t_units[BMSLEN]; /**< Time units */
	double dt; /**< Time step */
	double tleft; /**< Time left in this time step.*/
	/*@}*/

	/**@name
	 * Data buffers - these hold data for possibly several time steps
	 */
	/*@{*/
	double *tbuf; /**< Buffer for time values */
	double ****valuebuf; /**< Buffer for cell exchange data */

	/*@}*/

	/**@name
	 * Info used for accessing buffers
	 */
	/*@{*/
	long nbuf; /**< Number of time steps in buffers */
	long bufstart; /**< First time step in buffers */
	long bufend; /**< Last time step in buffers */
	long nextrec; /**< Next record to be used */
	long current_nbuf;

	/*@}*/

	/**@name
	 * Info about input file
	 */
	/*@{*/
	int fid; /**< netCDF id for input file */
	int t_did; /**< t dimension id for input file */
	int b_did; /**< b dimension id for input file */
	int z_did; /**< z dimension id for input file */

	int nvariables; /** The number of variables in the netcdf file */
	int *prop_vid; /**< property variable id */
	int t_vid; /**< time variable id */
	long nstep; /**< Number of time steps in file */
	/*@}*/

	/**@name
	 * Pointers to input data for 1 time step - these
	 * will be set to point to data for one time step
	 * contained in the input buffers above
	 */
	/*@{*/
	double ***dataBuffer; /**< Buffer for data */
	double *timeDataBuffer;

	int currentTimeIndex;

	int ***Species_Cohort_Lookup;
	int rewind;
	int rewindCount;

/*@}*/

} EcologyScalarData;


/*******************************************************************//**
 The ice data input structure
 *********************************************************************/
typedef struct {
	/**@name
	 * Input file management
	 */
	/*@{*/
	int nFiles;
	char **fname; /**< File names */
	int curFile; /**< Index of currently used file */
	char **variableName;
	char *depthName;
	/*@}*/

	/**@name
	 * Time variables
	 */
	/*@{*/
	double t; /**< Time */
	char t_units[BMSLEN]; /**< Time units */
	double dt; /**< Time step */
	double tleft; /**< Time left in this time step.*/
	/*@}*/

	/**@name
	 * Data buffers - these hold data for possibly several time steps
	 */
	/*@{*/
	double *tbuf; /**< Buffer for time values */
	double ***valuebuf; /**< Buffer for ice data */
	double **dzvaluebuf; /**< Buffer for ice depth data */

	/*@}*/

	/**@name
	 * Info used for accessing buffers
	 */
	/*@{*/
	long nbuf; /**< Number of time steps in buffers */
	long bufstart; /**< First time step in buffers */
	long bufend; /**< Last time step in buffers */
	long nextrec; /**< Next record to be used */
	/*@}*/

	/**@name
	 * Info about input file
	 */
	/*@{*/
	int fid; /**< netCDF id for temperature input file */
	int t_did; /**< t dimension id for ice input file */
	int b_did; /**< b dimension id for ice input file */

	int *prop_vid; /**< property variable id */
	int dz_vid; /**< depth variable id */
	int t_vid; /**< time variable id */
	long nstep; /**< Number of time steps in file */
	/*@}*/

	/**@name
	 * Pointers to input data for 1 time step - these
	 * will be set to point to data for one time step
	 * contained in the input buffers above
	 */
	/*@{*/
	double **dataBuffer; /**< Buffer for data */
	double *dzBuffer; /**< Buffer for data */
/*@}*/

} IcePropertyData;

/*******************************************************************
 * Structure that contains information about the contaminants.
 *
 *
 *********************************************************************/

typedef struct {
    char contaminant_name[STRLEN];    /** The name of the contaminant */
    char contaminant_unit[STRLEN];    /** The units string of the contaminant */
    
    int contam_temp_depend;         /** Whether LC50s are temperature dependent */
    int contaminant_tracer;            /** The index of the contaminant tracer in the tracer array */

    double half_life;                /** The half life of the contaminant in seconds */
    double amount_decayed;
    double dissolv_coefft;
    
    double fishery_thresh_level;   /** Threshold level of contaminant for fishery closure */
    
    double *sp_decay_half_life;
    double **sp_amount_decayed;
    
    //double *lost;
    double *gained;
    
    //double **lostGlobal;
    double **gainedGlobal;
    
    /* Information about how this contaminant affects species */
    int *sp_uptake_option;        /* For each species for each contaminant there is an uptake option. */
    double *sp_uptake_rate;     /* For each species for each contaminant there is an uptake rate. */
    double ***sp_uptake;            /* The uptake calculated for the cohort/group */
    
    /* Transfer of the contaiminant from one group to another */
    double ***sp_transfer;        /* transfer of contaminant due to predation */
    double ****sp_transfer_global;
    
    /* Affect of the contaiminant on species */
    double *sp_LD50;
    double *sp_LD100;
    double *sp_LDChronic;
    double *sp_LDslope;
    double *sp_EC50;
    double *sp_ECslope;
    double *sp_EC50_r;
    double *sp_ECslope_r;
    double *sp_maxConcentration;
    
    double *sp_L;  // Parameters for logistic growth effects model
    double *sp_A;
    double *sp_B;
    
    double *sp_L_r;  // Parameters for logistic reprod effects model
    double *sp_A_r;
    double *sp_B_r;
    
    double *sp_GrowthThresh;  /* Threshold tissue level where get growth effects */
    double *sp_GrowthEffect;  /* Effect size */
    double *sp_MoveEffect;  /* Effect size on movement */
    double *sp_ReprodThresh;  /* Threshold tissue level where get reprod effects */
    double *sp_ReprodEffect;  /* Effect size on reproduction (on number of settlers) */
    double *sp_ContamScalar;  /* Generic scalar to represent contaminant effects */
    double *sp_maternal_transfer; /* Maternal transfer rate during conception and recruitment */
    double *sp_suckling_transfer; /* Maternal transfer rate during suckling */
    
    double ****sp_maxDoseToDate;    /* Max dose to date of each contaminant - per cohort per box and layer */
    double *sp_maxLethalConc;    /* Max concentration for lethal dose of each contaminant */
    double *sp_maxChronicConc;    /* Max concentration for chronic dose response of each contaminant */
    double *sp_instantDoseMortality;
    
    double *sp_TimeToLD50;
    double *sp_Cx;
    double *sp_Cy;
    
    double *interact_coefft;
    
    /* Contaminant movement related parameters */
    double *sp_avoid;
    double *sp_K_avoid;

    /* Tracking */
    double ****expose_time; /* Exposure time for that contaminant - per species at that location */
    
    double ****sp_point; // point in the population hypercube - per cohort per box per layer
    
    //double **speciesMort;

}ContaminantStructure;

/* List of contaminant model options */
#define no_contam_interact 0
#define additive_contam_interact 1
#define mult_contam_interact 2
#define most_lim_contam_interact 3

/* List of contaminant growth options */
#define NoGrowthEffects 0
#define InVitro_model 1
#define Salmon_logistic_model 2

/* List of contaminant reprod options */
#define NoReprodEffects 0
#define Lovindeeretal 1
#define InVitro_model_r 2
#define Salmon_logistic_model_r 3

/* List of industry model options */
#define no_industry_model 0
#define simple_industry_model 1
#define dyn_industry_model 2

/*******************************************************************//**
 The Box Model structure
 *********************************************************************/
#define num_cmd_line_flags 15
typedef struct {

	/**Description of this model */
	char title[BMSLEN];

	/** Command line arguments/parameter file names */
	char params[num_cmd_line_flags * BMSLEN];

	/**@Logging */
	char logFileName[100];
	int logFileIndex;
	FILE *logFile;

	/** True if we are checking duplicates in the input prm files - turning this on will make readin very slow */
	int check_dups;

	/**@name
	 * Flags indicating which libraries are in active use - biology and physics are
	 * always assumed to be in use
	 */
	/*@{*/
	int flagecon_on; /**< flag indicating economics in use */
	int flag_fisheries_on; /**< flag indicating fisheries active */
    int flag_migration_on; /**< flag indicating migration active */
	/*@}*/

	/**@name
	 * I/O file info for this model
	 */
	/*@{*/
	char ncIfname[BMSLEN]; /**< tracer input file name */
	char	destFolder[BMSLEN];			/**< Destination folder */
	char	inputFolder[BMSLEN];			/**< Input folder */
	int ncIfdump; /**< tracer input file dump - dump indicates where in input file to start*/
	char geomIfname[BMSLEN]; /**< geometry input file name */
	char ncOfname[BMSLEN]; /**< tracer output file name */
	char ncOFishfname[BMSLEN]; /**< fisheries output file name */
	char ncOSumfname[BMSLEN]; /**< fisheries output file name */
	char ncOPCfname[BMSLEN]; /**< growth and consumption output file name */
	char ncODetFishfname[BMSLEN]; /**< detailed fisheries data output file name */
    char ncOAAfname[BMSLEN]; /**< annual age structured output file name */
    char ncOAACfname[BMSLEN]; /**< annual age structured catch and discards output file name */
    char ncODIETfname[BMSLEN]; /**< spatial diet output file name */
    char ncIDfname[BMSLEN]; /**< diagnostics input file name */
	char ncIDfsname[BMSLEN]; /**< fisheries statistics input file name */
	int ncOfdump; /**< number of next output dump for biological and physics data */
	int ncOfishdump; /**< number of next output dump for fisheries data */
	int ncOsumdump; /**< number of next output dump for summary data */
	int ncOpcdump; /**< number of next output dump for growth/consumption data */
    int ncOaadump; /**< number of next output dump for age structured data */
    int ncOaacdump; /**< number of next output dump for age structured catch and discard data */
    int ncOdetfishdump; /**< number of next output dump for detailed fisheries data */
    int ncOdietdump; /**< number of next output dump for spatial diet data */
	int ncOfid; /**< netCDF id for general output file */
	int ncOfishfid; /**< netCDF id for fisheries output file */
	int ncOsumfid; /**< netCDF id for summary output file */
	int ncOpcfid; /**< netCDF id for growth/consumption output file */
    int ncOaafid; /**< netCDF id for age structured output file */
    int ncOaacfid; /**< netCDF id for age structured catch and discards output file */
	int ncOdetfishfid; /**< netCDF id for detailed fisheries output file */
    int ncOdietfid; /**< netCDF id for spatial dietoutput file */

	int ncOfIndex; /**< File index for the general output file */
	int ncOfishIndex; /**< File index for the fisheries output file */
	int ncOsumIndex; /**< File index for the summary output file */
    int ncOpcIndex; /**< File index for the growth/consumption output file */
    int ncOaaIndex; /**< File index for the age structured output file */
    int ncOaacIndex; /**< File index for the age structured catch and discards output file */
	int ncOdetfishIndex; /**< File index for the detailed fisheries output file */
    int ncOdietIndex; /**< File index for the spatial structured diet output file */

	char runprmIfname[BMSLEN]; /**< run parameter file */
	char physprmIfname[BMSLEN]; /**< physics parameter file */
	char bioprmIfname[BMSLEN]; /**< biology parameter file */
	char fishprmIfname[BMSLEN]; /**< fisheries parameter file */
	char assessprmIfname[BMSLEN]; /**< assessment model parameter file */
	char forceIfname[BMSLEN]; /**< forcing file */
	char econprmIfname[BMSLEN]; /**< economics model parameter file */
	char functionGroupIfname[BMSLEN]; /**< functional group parameter file */
	char fisheryIfname[BMSLEN]; /**< Fishery parameter file */
    char migrationIfname[BMSLEN]; /**< Migration parameter file */
	char CLAMLinkageprmIfname[BMSLEN]; /**< The parameter file for the CLAM linkage code */
	char brokerLinkageprmIfname[BMSLEN];
	double clamDay;

	char startfname[BMSLEN]; /**< output base name - readin from command line and
	 stored here so can create other files with this
	 as start/basis */
	/*@}*/

	/**@name
	 * Geometry data for this model
	 */
	/*@{*/

	int nbox; /**< number of boxes */
	int nboundary; /**< number of boundary boxes */
	int nface; /**< number of open faces */
	int wcnz; /**< number of layers in water column */
	int sednz; /**< max number of layers in sediment */
	int icenz; /**< max number of layers in ice */
	int first_box; /**< index of first box (starting from zero and ascending) that is a
	 non-boundary box */

	int top_layer; /* top layer of first box */
	int last_box; /* index of last box that is a non-boundary box */
	double maxwcbotz; /* depth at which water column becomes open lower boundary  (if not hit bottom yet) */
	double maxseddz; /* Max thickness for any sediment layer */
	double minseddz; /* Min thickness for any sediment layer */
	double max_erosion;/* Max erosion thickness per timestep */
    double wgt_georugosity; /* Weighting for geological component of rugosity */
	polyline *bnd; /* boundary of the model */
	Box *boxes; /* array of boxes */
	Face *faces; /* array of open faces */
	MapProjection *projection; /* Information about the model projection */

	/* Run parameters */
	double tstop; /* stop time */
	double toutstart; /* output start time */
	double toutinc; /* output time increment for biological and physical data */
	double toutfinc; /* output time increment for fisheries data */
	double dt; /* time step */
    double dtsz_stored; /* stored value for sub time step */
	double tburnday; /* day of run where burn-in conditions relaxed */
    int ts_on_hydro_time;  /* whether forcing time series in physics file in terms of t or hd.t */

    double inputs_tout; /* Frequency of the inputs.ts and export.ts file */
    double inputs_toutNext; /* Next output time. */

	/* Physical  parameters */
	int tidal; /* Whether model tidal or not - based on dt used (if dt < 43200) */
	int max_nconn; /* Maximum number of connections for any one box in the model domain */
	double *hc; /* Concentrations to diffuse */
	double *hk; /* Diffusion coefficients */
	double *hx; /* X-coordinates of boxes diffusing */
    int *hdone; /* Whether a box already handled in diffusion calculations */

    int include_atmosphere;	/* Flag to turn the atmospheric changes on/off - ensures modellers can 
    							have backwards compatible model results */

    double atmospheric_NH;  /* Ambient atmospheric concentration of NH3 */
    double atmospheric_NO;  /* Ambient atmospheric concentration of NO3 */
	double atmospheric_F;   /* Ambient atmospheric concentration of micronutrient - dust as source */
	double atmospheric_O2;  /* Ambient atmospheric concentration of O2 */
	double atmospheric_CO2;  /* Ambient atmospheric concentration of CO2 */
	double atmospheric_P;   /* Ambient atmospheric concentration of P - dust as at least one source */
	double atmospheric_Si;   /* Ambient atmospheric concentration of silica - dust as source */
    
	double a_wc; /* Water column diffusion scheme */
	double a_sed; /* Sediment diffusion scheme */
	double wc_kz; /* Water column mixing cfft PILOT MODEL */
	double resusp; /* Resuspension rate PILOT MODEL */
	double wc_dz_tol; /* Fractional dz change allowed in water column */

	/* Vertical mixing rate due to upwelling */
	double mix_injection; /* Upwelling injection (mixing) rate; */
	double mix_season_kz; /* Winter increase in mixing rate (due to mixed-layer breakdown) */
	double mix_deep_depth; /* Depth counted as deep ocean, boxes deeper than this will get deep ocean mixing */
	int mix_deep; /* Flag indicating whether deep ocean mixing turned on */
    int mix_deep_O2; /* Flag indicating whether to allow for O2 upwelling in deepwater open boundary boxes where get deepwater mixing of nutrients */
                      
	/* Bio-irrigation rates */
	double bi_dissol_kz; /* Dissolved diffusion cfft */
	double bi_exchange; /* Exchange rate */
	double bi_injection; /* Injection rate */
	/* Bio-turbation rates */
	double bt_partic_kz; /* Particulate diffusion cfft */
	double bt_exchange; /* Exchange rate */
	double bt_expulsion; /* Expulsion rate */
	/* Profile of activity in sediments */

	char biosedprofile;
	char biooxprofile; /** Oxygen profile */
	/*@}*/

	int constrain_wc; /**< Switch to show whether water column depth is constrained */
	int injection; /**< Point source switch */
	int atmospherics; /**< Atmospheric exchange switch */
	int settling; /**< Settling switch */
	int bioirrigation; /**< Bioirrigation switch */
	int bioturbation; /**< Bioturbation switch */
	int resuspension; /**< Resuspension switch */
	int horiz_diffusion; /**< Horizontal diffusion switch */
	int vert_diffusion; /**< Vertical diffusion switch */
	int vert_mix; /**< Forced vertical mixing switch */
	int advect_diffusion;/**< Transport model (advection diffusion) switch */
    int fill_zero_exchange; /**< Whether patching holes in advection model with horiz_diffusion */
    double flush_threshold; /**< Theshold number of days without fluxes before force horizontal diffusion */
    int use_fill_horizmix; /**< Flag indicating whether use horizmix scalar in filler fluxes */
	int decay_wc; /**< Decay in watercolumn switch */
	int decay_sed; /**< Decay in sediments switch */
	double decay_sed_scale; /**< Decay in sediments scalar */
	int scale_transport; /**< Scale transports switch */
	double prcnt_exchange; /**< Constant scalar for exchanges */
	double ka_exchange; /**< Conefficient of area dependent scaling for exchanges */
	int cascade_flows; /**< Cascading flows switch */
	double width; /**< Distance across the model domain in metres */
	double baseline_temp; /**< Base temperature of the model */
	double temp_amplitude; /**< Amplitude of temperature variation in the model */
	double temp_const_B; /**< Constant coefficient in nonlinear temperature step1 */
	double temp_const_C; /**< Constant in second step of coefficient in nonlinear temperature */
	double temp_const_D; /**< Exponent in second step of nonlinear temperature */
	double **eddy_seasonal; /**< Eddy arrays */

	int supplied_stress; /**< Flag showing whether stress values must be read from
	 time series file or supplied in initial conditions netcdf file */
	double min_channel_depth; /**< Minimum channel depth when still counted as flowing vs dry (as tidally emptied) */

	/**@name
	 * Run Time variables
	 */
	/*@{*/
	double tstart; /**< model start time */
	double t; /**< current model time */
	double tout; /**< next output time for biological and physical data */
	double tfishout; /**< next output time for fisheries data */
	double tsumout; /**< period between fish stock state output */
	double tsumoutnext; /**< next stock summary output time */
	int fishout; /**< switch indicating if fisheries data output necessary
	 on = 1, off = 0
	 */
	char t_units[BMSLEN]; /**< time units */
	long nt; /**< Number of time steps taken so far */
	int flagreusefile; /**< flag indicating whether want to reuse existing output
	 file (i.e. whether want to append on the end of it
	 no = 0, yes = 1, replace = 2
	 */
	int coming_to_end; /**< Counter to keep track of whether ending the program so need
	 print out in case of chaining */
	/*@}*/


	int flag_replicated_old; /* Flag to allow modellers to replicate the results of the old bec_dev version of the code. We will remove this asap. */
    int flag_replicated_old_PPmort; /* So can replicate old primary production mortality assumption */
    int flag_old_embryo_init; /* Flag to use the old means of having embryoes carried over from spawning pre-model start */
    int flag_replicate_old_calendar; /* Flag to allow modellers to replicate the results of the old way of doing the aging and spawning calendar dates */
    int flag_sanity_check; /* Flag to trigger sanity checks in the demographics code */
    int store_aggregate_yoy; /* Flag as to what YOY stored */
    int store_mig_array; /* Flag to indicate want to store migration arrya */
    
    int *RAssess_initiated; /* If gatdget file initiated */
    
	/**@name
	 * Model Time variables
	 */
	/*@{*/
	double DayInYear;	/** The day of the year - stored as a double */
	double dayt; /**< current model time (day since start of run) */
	double predayt; /**< last day in model time */
	double HowFar; /**< relative spot in current quarter */
	double timeleft; /**< proportion of day remaining */
	int TofY; /**< day in the year (assumes 365 days in a year) */
	int MofY; /**< month of the year (assumes 13 lunar months) */
	int DofM; /**< day of the month (assumes 30 days per month and any days lost
	 = fairly universal rest days like Christmas) */
	int lastMofY; /**< last month of the year (i.e. previous to this month) */
	int newmonth; /**< flag indicating new month has ticked over */
	int DofW; /**< day of the week */
	int newweek; /**< flag indicating new week has ticked over */
	int QofY; /**< quarter in the year (0-3) */
	int NextQofY; /**< next quarter in the year (0-3) */
	int DofQ; /**< day of the quarter */
	int BiM; /**< bimonthly period of the year */
	int LastBiM; /**< previous bimonthly period of the year */
	int flagday; /**< flag identifying whether day or night: 0 = night, 1 = day */
	int thisyear; /**< year in model time */

	/*@}*/

	/**@name
	 * Model debugging aids and diagnostic flags
	 */
	/*@{*/
	int flag_skip_biol; /**< Flag indicating biology to be skipped while decoding physics or fisheries fast */
	int flag_skip_phys; /**< Flag indicating physics to be skipped while decoding ecology or fisheries fast */
	int debug; /**< Flag enabling lots of debugging output code
	 - different values of this flag indicate different debuging options.*/
	int debug_it; /**< Flag checking for adaptive timestep bottlenecks */
	int flagannual_Mest; /**< Flag checking whether mortality per predator outputs only annually
	 - its useful to do it less than annually if calibrating */
	int fishmove; /**< Flag turning fish movement on/off */
	int which_fleet; /**< Flag indicating which fleet to track fluxes for */
	int which_check; /**< Flag indicating which group to track fluxes for */
	HABITAT_TYPES habitat_check; /**< Flag indicating which habitat to track fluxes in */
	int move_check; /**< Flag indicating whether checking movement calculations */
	int checkNH; /**< Flag indicating want diagnostic output for NH */
	int checkDL; /**< Flag indicating want diagnostic output for DL */
	int checkDR; /**< Flag indicating want diagnostic output for DR */
    int masscheck; /**< Flag indicating want mass check for the species identified by which_check */
	int checkbox; /**< Current box of interest for printout */
	int checkbiom; /**< Flag indicating whether want diagnostic biomass output */
	double checkstart; /**< Start writing output as of this day */
	double checkstop; /**< Stop writing output as of this day */
    
    int flagstarve; /**< Flag indicating whether reporting starvation */
    int flaggape; /**< Flag indicating whether reporting gape limitation */
    int flagchecksize; /**< Flag indicating whether reporting current size vs
     initial conditions */
    int flagagecheck; /**< Flag check age structure when reporting on size of vertebrates (flagchecksize) */
    int flagdietcheck; /**< Flag indicating whether want to output detailed diets */
    int flagenviro_warn; /**< Flag indicating that whether want earnings anout environmental constraints on movement recorded to the logfile */
    int flagenviro_displace; /**< Flag indicating whether environment displaces the species or allows it to disappear */
    int flagenviro_kill; /**< Flag indicating whether environment displaces the species or allows it to disappear */
    int flag_mig_in_bioindx; /**< Whether to include biomass from migration arrays */

	/*@{*/

	/**@name
	 * Flags indicating whether have ice or terrestrial components
	 */
	/*@{*/
	int ice_on; /**< If ice components activated */
	int terrestrial_on; /**< If terrestrial components allowed */
    
    int flag_pollutant_impacts; /**< Whether looking at noise and light pollution */

	int num_active_habitats;	/**<If we have ice and/or land then this will be increased past the normal EPIFAUNA value */


	/**@name
	 * Tracer info and data storage
	 */
	/*@{*/
	int ntracer; /**< number of tracers */
	TracerInfo *tinfo; /**< array of tracer info */
	double ***wctr; /**< water column values */
	double ***sedtr; /**< sediment values */
	int waterid; /**< index of water tracer variable */
	int flagscaleinit; /**< flag indicating whether or not need
	 to scale initial conditions using init_scalar */
	double *init_scalar; /**< scalar for vertebrate initial densities.
	 Only for use in emergencies (should really
	 redo cdf file with correct values) */
	char **spSTRNAME; /**< Array of species long names (for dtrong search
	 when rescaling inital conditions using init_scalar) */

	/**@name
	 * Epibenthos info and data storage
	 */
	/*@{*/
	int nepi; /**< number of epibenthos variables */
	EpiInfo *einfo; /**< array of epibenthos variable info */
	double **epi; /**< epibenthos values */
	/*@}*/


	/**@name
	 * Terrestrial info and data storage
	 */
	/*@{*/
	int nland; /**< number of land variables */
	EpiInfo *landinfo; /**< array of land variable info */
	double **landtr; /**< land values */
	/*@}*/


	/**@name
	 * Ice info and data storage and relevant files
	 */
	/*@{*/
	double ***icetr; /**< ice values */
	int nicetracer; /**< number of ice specific tracers */
	ICE_INPUT_TYPES kind_ice_model; /**< kind of ice data file to be loaded */
	int K_num_ice_classes; /**< number of classes of ice */
	IcePropertyData iceinput;
	int maxicedz; /**< maximum width of ice layer */
	int minicedz; /**< minimum width of ice layer */
	int slush_id; /**< which is the ice class that is slush */
	int snow_id; /**< which is the ice class that is actually snow cover - for the ice light model */
	double **ts_ice_data_array; /**< buffer array to load in time series of ice class values */
	double *ts_ice_dz_data; /**< buffer array to load in time series of ice class values */
	int ***ICE_HABITATlike; /* Habitat preferences */
	int ***ICE_HABITATlikeReprod;

	/**@name
	 * Diagnostic info and data storage
	 */
	/*@{*/
	int ndiag; /**< number of diagnostic variables */
	DiagInfo *dinfo; /**< array of diagnostic variable info */
	double **diagnost; /**< diagnostic values */
    double *****totDiet; /**< water column diet values */

	/*@}*/

	/**@name
	 * Fisheries statistics info and data storage
	 */
	/*@{*/
	int nfstat; /**< number of fisheries statistics variables */
	FstatInfo *finfo; /**< array of fisheries statistics variable info */
	double **fishstat; /**< fisheries statistics and data values */
	/*@}*/

	/**@name
	 * Physical data storage
	 */
	/*@{*/
    int *is_boundary; /**< Flag indicating whether a boundary box or not **/
	double **nom_dz; /**< Nominal cell thicknesses */
	double **dz; /**< Time varying cell thicknesses */
	double **icedz; /**< Time varying ice cell thicknesses */
	double **vol; /**< Cell volumes */
	double **por; /**< Porosity */
	double **hddrain; /**< Hydrodynamicc exchanges (transports) sources */
	double **hdpool; /**< Hydrodynamicc exchanges (transports) sinks */
	double **efluxes; /**< Hydrodynamic exchanges (transports) net horizontal exchange */
	double **vfluxes; /**< Hydrodynamic exchanges (transports) net vertical exchange */
    
    double **init_pH; /**< INitial pH values per box - for calculating acidification */
	atPhysicsStructure *atPhysicsModule;
	/*@}*/


	/**@name
	 * Point source/sink list and constants
	 */
	/*@{*/
	int npss; /**< Number of point source/sinks */
	PointSourceSink *pss; /**< List of point source/sinks */

	int pulsechange; /**< Switch to turn pulse of nutrient change on */
	int nutrientChange; /**< Switch to turn pulse of nutrient change on */

	/**@name
	 * Precipitation and evaporation
	 */
	/*@{*/
	TimeSeries *precip;
	int precip_id;
	TimeSeries *evap;
	int evap_id;
	/*@}*/

	/**@name
	 * Bottom stress
	 */
	/*@{*/
	TimeSeries *stress;
	int stress_id;
	/*@}*/

	/**@name
	 * Solar radiation
	 */
	/*@{*/
	TimeSeries *swr;
	int swr_id;
	int swr_rewindid;
	double swr_scalar;
    double swr_const;
    double swr_cos_coefft;
    double swr_cos_offset;
	/*@}*/

    /**@name
     * thermal and rate index
     */
    /*@{*/
    TimeSeries *thermal_index;
    int tindex_id;
    int tindex_rewindid;
    double airtemp_index;
    
    TimeSeries *rate_index;
    int rindex_id;
    int rindex_rewindid;
    double airT_rate_index;
    /*@}*/

	/**@name
	 * Temperature and salinity model data input structure
	 */
	/*@{*/
	int use_tempfiles; /**< Flag to indicate whether using temperature forcing files */
	int use_saltfiles; /**< Flag to indicate whether using salinity forcing files */
	int use_pHfiles; /**< Flag to indicate whether using salinity forcing files */
    int use_VertMixfiles; /**< Flag to indicate whether using vertical mixing forcing files */
    int use_pollutantfiles; /**< Flag to indicate whether using nosie and artifical light (pollutant) forcing files */
	int use_forceTracers; /** Flag to indicate that the user wants us to read in forcing files for other tracers */
	int numForceTracers;
    int use_weighted_assim; /** Flag to indicate whether using weighted assimilation or not */
    
    int use_move_entries; /**< Flag to indicate whether feeding in dynamic perscribed spatial distributions */
    int numForceMoveEntries; /**< Flag indicating how many files will read in for movement spatial distributions */

	PhyPropertyData *forceTracerInput;  /**< For "data assimilating" variables from forcing file */
    PhyPropertyData *forceMoveEntryInput; /**< For reasing in the species move entries */

	PhyPropertyData tempinput;
	PhyPropertyData saltinput;
	PhyPropertyData pHinput;
    PhyPropertyData windinput;
    PhyPropertyData swrinput;
    PhyPropertyData VertMixinput;
    PhyPropertyData noiseinput;
    PhyPropertyData lightpinput;

	double *TempLayerScalar;
	double *SaltLayerScalar;
	double *pHLayerScalar;


	int tempid; /**< Tracer id for temperature */
	int saltid; /**< Tracer id for salinity */
	int pHid; /**< Tracer id for salinity */
    int windid; /**< Tracer id for wind */
    int noiseid; /**< Tracer id for noise pollution */
    int lightpid; /**< Tracer id for light pollution */
	int **checkedalready; /**< Array of flags to indicate whether local temperature
	 and salinity modifications have been made */

	TimeSeries *tr_areainp; /**< Area inputs of tracers (mass flux)  */
	TimeSeries *env_force; /**< Environmental recruitment forcing */
    TimeSeries *KWSR_force; /**< Environmental recruitment forcing */
    TimeSeries *tsRecruit; /**< Recruitment forcing */
	TimeSeries *tslinearMort;
	TimeSeries *tsSizeChange; /* Size change forcing */
	TimeSeries *tsGrowthRateChange; /* Growth rate change forcing */
	TimeSeries *tsFSPBChange; /* FSPB change forcing */

	HydroData hd; /**< Hydrodynamic model data input structure */
	int mirror_invalid; /* Flag indicating that if get invalid destination layer then match source layer */
	/*@}*/


	 /**@name
	 * Time series and parameters to do with atmospheric pCO2
	 */
	/*@{*/

	TimeSeries *tspCO2; /* pCO2 forcing */
	int pCO2_id;
	double pH_surface_depth;   /**< Water depth marking surface waters */
	double pH_mid_depth;   /**< Water depth marking mid waters */
	// Coefficients for Bostock et al 2012 alkalinity model
	double pH_surface_coefft_T;
	double pH_surface_coefft_S;
	double pH_surface_coefft_O;
	double pH_surface_const;
	double pH_mid_coefft_T;
	double pH_mid_coefft_S;
	double pH_mid_coefft_O;
	double pH_mid_const;
	double pH_deep_coefft_T;
	double pH_deep_coefft_S;
	double pH_deep_coefft_O;
	double pH_deep_const;

    int flagPHmortcase; /**< What kind of pH related mortality to use */
    int pH_sensitivity_model; /**< What kind of pH sensitivity model to use */

	/* And Aragonite parameterisation - needed for coral reefs in particular
	 * Values for coefficients from proxy fitting exercise - from data
	 * given in Bjerrum plot from Zeebe & Wolf Ocean Chem encyclopedia */
	double Karag_A;
	double Karag_B;
	double Karag_C;
	double Karag_pH;
	double Kca_const;
	double K_Ks;

	/* Smothering effects */
	double Ksmother_coefft;
	double Ksmother_const;
    
    int sp_boring_sponges;
    
    /* Phosphorous and Carbon related */
    double Pads_r_t0;
    double r_immob_PIP_t0;
    double Pads_K;
    double Pads_KO;
    
	/*@}*/

	/**@name
	 *	Model restrictions
	 */
	/*@{*/
	double min_pool; /**< Minimum size of a pool before it was ignored as negligible */
	double min_dens;
	double max_depth; /**< The seward depth boundary of the model layers */
	/*@}*/

	/* Model Space variables - so can reference arrays correctly and easily */
	int current_box; /* Location tracking - current box */
	int current_layer; /* Location tracking - current layer in current box */
	int current_icelayer; /* Location reacking - current ice layer in current box */

	double cell_vol; /* Volume of current cell */
    double avg_cell_vol; /* AVergae cell volume at model start - only needed when initialising some reproduction */

	/* Flags and parameters potentially required by biological and harvesting models */
	int ecotest;
    int assess_share_params;  /**< Flag indicating whether all stocks assessed for an individual species share same parameters (1) or not (0) */
	int do_assessing; /**< Flags indicating whether assessments and scientific sampling carried out */
	int do_TACassessing; /**< Flag indicating whether doing single species management adjustments (1) or just ecological sampling (0) */
    int usingR; /**< FLag indicating whether using R in the assessment process */
    int usingRedus_R_HCR; /**< Flag indicating whether R for harvest control rule */
    int K_max_BlockPattern; /**< Counter for blocks in fishery monitoring*/
    
	int do_availflag; /**< Flag indicating whether want to use Activeflag code */
	int flagrandom; /**< Flag indicating whether or not have small non-zero random fluctuation in
	 dates of recruitment and migration */
    int flag_multiyr_migs; /**< Flag indicating whether multi year migrations are active or not */
    int external_populations; /**< Flag indicating have groups using the external population model */
    
    int track_rugosity_arag; /**< Flag indicating wheher need rugosity or aragonite saturation related information */
	int track_pH; /**< A flag to indicate that need a pH variable even if not reading it in from data file */
    int flag_use_deltaH; /** Flag indicating that using delta H for pHcorr calculations rather than pH */
    
	int K_num_bed_types; /**< Total number of seabed types in the model */
	int K_num_cover_types; /**< Total number of benthic habitat (cover) types in the model */
	int K_num_max_cohort; /**< The maximum number of cohorts in the model */
	int K_num_migout; /**< Total number of migrant groups in the model */
	int K_num_tot_sp; /**< Total number of groups in the model with no juvenile invertebrates */
	//int K_num_vert_sp; /**< Total number of vertebrate groups in the model */
	int K_num_stocks_per_sp; /**< Maximum number of stocks per vertebrate group in the model */

	int K_num_sexes;  /**< Number of sexes covered - TODO: Need to generalise code because at present effectively assumes 1 sex */
    int K_max_invert_cohorts;  /**< Max number of cohorts for invertebrates - needed for invertebrate movement code */

	// invertebrates like cephalopods and prawns */
	int K_num_physiochem; /**<Total number of physiochemical properties */
	int K_num_homerange; /**< Total number of home ranges */
	int maxspage;
	int K_max_num_migration;

    int K_max_num_DHW;	/**< Size of degree heat weeks record queue for calculating bleaching triggers for corals */

	int K_num_max_genetypes; /**< Maximum number of gene types in the model */
	int K_num_max_stages; /**< Maximum number of life stages in the model - typically juvenile and adults */
	int K_num_max_spawns; /**< Maximum number of spawnings in a year */

	int K_num_reg; /* Number of biomass reporting regions */
	int K_num_active_reg; /* Number of active managment or biomass reporting regions (rather than max number needed for read-in */
	int DIN_id; /* ID of entry in totbiom and totfishpop arrays for DIN */

	int flag_age_output;  /* If want age structured output */
	int containsMammals;	/* TRUE if the model contains at least one mammal */
	int containsBirds;		/* TRUE if the model contains at least one bird */
	int containsTurf;		/* TRUE if the model contains at least one turf group */
	int containsMCPYr;      /* TRUE if the model contains at least one group with multiple cohorts per year */
	int flagWCVert;			/* TRUE if the model only contains AGE_STRUCTURED groups in habitat WC */

	/* - for overall total "stock size" output record keeping purposes */
	/*@}*/

	/**@name
	 * Ecosystem model parameters and flags
	 */
	/*@{*/
    int flagpassivetracer; /**<Flag allowing for passive transport tracking */
    int containsCoral; /**< Flag indicating whether coral groups in play or not */
    int containsSponge; /**< Flag indicating whether sponge groups in play or not */
    
	int O2case; /**< Flag indicating oxygen limitation case */
	int flagtrackpops; /**< Flag indicating whether to store information on overall
	 stock size */
	int flagseason; /**< Flag indicating whether use seasonal variation */
	int flaglight; /**< Flag indicating whether using light adaption formulations */
	int flagIsEstuary; /**< Flag indicating model is estuary. If this is true then the sediment tracer will be read in from the
	 initial conditions file and used in the light model */
    int flagIsPolar; /**< Flag indicating model is polar - so it turns off the light level warnings */

	int flagIncludepH; /**< Flag indicating model is including pH. If this is true then the sediment tracer will be read in from the
		 initial conditions file and used in the light model */
	int flaglightopt; /**< Flag indicating which light model to use */
	int lim_sun_hours; /**< whether to worry about finely resolved daylight hours */
	int flagmodeltemp; /**< Flag indicating which seasonal temperature variation
	 formulation to use */
	int flagq10; /**< Flag indicating whether q10 considerations on */
	double Tcorr; /**< Temperature correction */
	double current_corr;

	int flaghemisphere; /**< Flag indicating hemisphere model is in (so get
	 seasonal temperature sequence correct */
	int flagmodelArag; /**<Flag indicating whether aragonite note needed (0) provided from time series (1) or calculated using an empirical proxy (2) */
	int flagmodelpH; /**< Flag indicating whether using pH data or algorithm related to salinity */
    
	int flagnut; /**< Flag indicating what form of nutrient limitation to use */
	int flagmicro; /**< Flag indicating whether to use micro-nutrient limitation or not */
	int flagtempchange; /**< Flag indicating whether baseline temperature is changing */
	int flagsaltchange; /**< Flag indicating whether baseline salinity is changing */
	int flagpHchange; /**< Flag indicating whether baseline salinity is changing */

	int flagdegrade; /**< Flag indicating whether have habitat degradation */
	int flaghabdepend; /**< Flag indicating whether habitat dependency (especially
	 refugia from predation) */
    int flag_move_habdepend; /**< Flag indicating whether habitat dependency (especially refugia from predation) influences movement */
	int flagroc; /**< Flag indicating which forage dependency to use in movement routines */
	int flaghomog_sp; /**< Flag indicating whether size is localised or smeared
	 across entire domain */
	int flagagestruct; /**< Flag indicating whether to track age-class distribution
	 within age phase */
	int flagtempdepend_move; /**< Flag indicating whether movement activities is temperature dependent */
    int flagtempdepend_reprod; /**< Flag indicating whether spawning and reproduction is temperature dependent */
	int flagsaltdepend; /**< Flag indicating whether ecological activities
	 (e.g. movement and spawning) salinity dependent */
	int flagO2depend; /**< Flag indicating whether ecological activities
	 (e.g. movement and spawning) oxygen dependent */
	int flagsenesce; /**< Flag indicating whether oldest individuals senesce */
	int flagtsforcerecruit; /**< Flag indicating whether there is time series forcing
	 of recruitment */
    int flag_modify_KWSR; /**< Flag indicating whether scaling or forcing KWSR in recrutiment */
    int flag_extpop_growth_option; /**< Flag indicating which growth option to use for the external populations */
    
    int norm_larval_distrib; /** Flag indicating whether to normalise recruit distributions or have the potential for recruits to be lost from the system */
    int larvae_connect_only; /** Flag indicating whether want to condition recruit_hdistrib with a connectivity matrix or just use a connectivity matrix */
    int enviro_independ_larvae; /** Flag indicating whether to change recruit_hdistrib based on environmental conditions (like temperature etc) or just allow for deaths/movement once settled */
    int flag_recruit_effect; /** Flag indicating whether to have environmental effects on start or end of larval period effect recruit_hdistrib */
	int flagfishrates; /**< Flag indicating whether absolute or relative fish
	 clearance and growth rates entered */
	int flagavgmig; /**< Flag indicating whether to average size for migrants
	 or simply pass individuals */
    
    int flagconstrain_epiwander; /**< Flag indicating that want to constrain epibenthic wandering */

    int flag_refuge_model; /**< Flag indicating refugia model being used - 0 = none, 1 = original, 2 = rugosity related */
	int flag_adv_habitat; /**< Flag indicating whether using advanced habitat model with rugosity */
	int flag_rugosity_model; /**< Which rugosity model to track */
    int flag_georugosity; /**< Whether there is a geological component to rugosity - assumed to be what is read in on initalisation */
	int flag_macro_model; /**< FLag indicates which macrophyte model to use */
	int flag_benthos_sediment_link;  /**<Flag indicates whether growth of benthos is linked to the proportiona cover of specific kinds of sediment */
    int flag_competing_epiff;  /**< Flag indicating whether corals, filter feeders and macrophytes compete for space */
    double max_available_habitat; /** The max available habitat - should normally be 1.0 but changed to allow people to allow overlaping habitats */
    int flag_invert_biohab;   /**< Flag indicating whether inverebrates use dynamic biohabitat to condition growth */
	int flagOverwinter; /**< Flag whether have overwintering zooplankton or not */
	int flag_fine_ontogenetic_diets; /**< Flag whether want finely detailed onotgenetic diets or not */
	int flag_report_water_detritus; /* Flag whether to include detritus in the water column in reporting numbers. Traditionallty we have only counted DR and DL in the sediment */
	int flag_olddiet; /**< Whether want to reproduce the old way of mapping diets or not - best not to, but just in case */
	int UseHardFeedingWindow; /**< Whether using heaviside step function for feeding or smoother curves */
    int UseBiLogisticFeedingWindow; /**< Whether using humped or bi-logistic shape smoothed feeding curve */
    int flag_satiation; /**< Flag to indicate whether using satiation of feeding functions - especially for standard form of functional responses */
    int flag_shrinkfat; /**< Flag to indicate whetehr the growth of fish allows fish to lose fat (1) or not (0) */
    int flag_predratiodepend; /**< Flag indicating whether using any ratio dependent feeding functions */
    int need_predcomp_params; /**< Flag indicating whether need to allow for predator interferernce */
    int need_tot_pred; /**< Flag indicating whether need to allow for calculation of total predator biomass */    
    int flag_import_feed; /**< Flag indicating bringing in feed layer form outside the model domain */
    int flag_aquacult_on;  /**<Flag indicating whether aquaculture is on in model */
    int flag_dynamicXRS; /**<Flag indicating that X_RS ratio is calculated dynamically for each species and age group - ASTA */
    int flag_repcostSpawn; /**<Flag to calculate spawn based on proportion of RN available for reproduction and reproduction cost - ASTA */
    int flag_lengthSN; /**<Flag to indicate that length is calculated using SN pool only (not SN+RN as used by default) - ASTA_May */
    double XRS_static;  /**<X_RS ratio when not using dynamic X_RS calculation - traditional */
    double XRS_cap; /**< X_RS calculation cap for use with dnynamic X_RS - set to 20.0 by Asta */
    
	int external_mortality;	/* Do we have some external mortality pressure - if so print out to mortality files */
    
    /** Migration Aging related */
    double juv_transition_thresh;
    double mat_transition_thresh;

	/** Rugosity related parameters */
	double RugCover_Coefft;
	double RugCover_Const;
	double RugCover_Cap;
	double min_rugosity;
	double max_rugosity;
    double rugosity_const;  // Bozec empirical rugosity relationship constant
    double rugosity_bozec_a; // Used in Bozec empirical rugosity relationship
    double rugosity_bozec_b; // Used in Bozec empirical rugosity relationship
    double rugosity_bozec_c; // Used in Bozec empirical rugosity relationship
    double rugosity_bozec_d; // Used in Bozec empirical rugosity relationship


	/** Evolution related information */
    int flag_do_var_express; /**< Whether having variable phenotypic expression (1) or not (0) */
	int flag_do_evolution;  /**< Whether doing full evolution with shifting genotype etc (1) or just allowing for multiple growth morphs (0) */
	int flag_bound_change;  /**< Whether its the bound that are changing or another feature trait */
    int flag_inheritance;  /**< Flag indicating inheritance model in use */
    int flag_evolvar_capped;  /**< Flag indicating whether variabilty of evolution is capped or not */
	double evol_stdev_range; /**< Range of distribution represented by genotypes (in terms of number of stdev) */
	double max_rate_evol;  /**< Maximum rate of evolution - proportion */

	int flag_mult_grow_curves; /**< Whether there are multiple age curves but no evolution per se - for use when generating data for SS3 assessments */
    double ***evolTraitValues;	/** Trait values per species , cohort * ngenes, trait index */
    double ***evolTraitOriginalValues; /** Original trait values per species , cohort * ngenes, trait index */

	/** Additional tracer information */
	int track_atomic_ratio;
    int flagratio_warn;
    double N_to_C;
    double N_to_P;
	AtomicRatioStructure *atomicRatioInfo;

    int track_wind; /* Tracking wind for forcing recruitment */
    
	int track_contaminants;	/* Are we tracking contaminants in the model */
    int biopools_dodge_contam; /* So can stop biomass pools inappriopriately dodging contaminants - important in some models */
	int num_contaminants;
    int flag_contam_sanity_check;
    int flag_dissolved_pollutants;
    int flag_contamMortModel;
    int flag_contamInteractModel;
    int flag_contamOnlyAmplify;
    int flag_contamGrowthModel;
    int flag_contamReprodModel;
    int flag_contamMove;
    int flag_contamMinTemp;
    int flag_detritus_contam;
    int flag_contam_fisheries_mgmt;
    int flag_contam_halflife_spbased;
    int flag_contamMaternalTransfer;
    int flag_contam_distrib;
    int contam_fishery_closure_day;
    int contam_fishery_closure_period;
    int contam_fishery_closure_option;
    
    double contam_sig_uptake_const;
    double contam_tau;
    
    double *ContamClosed;
    ContaminantStructure **contaminantStructure;

	double X_CN; /**< C:N ratio for use in Redfield or conversion AFDW to wet weight (typically 5.7) */
    double k_wetdry;
	double li_a_invert;
	double li_b_invert;

	int p_BFDsplit; /**< Flag indicating whether to use lumped ontogentic diet connections or large pSPeat
	 array when BFD is the prey */
	int p_BMDsplit; /**< Flag indicating whether to use lumped ontogentic diet connections or large pSPeat
	 array when BMD is the prey */

	//double **SP_prms;		/* Array of species specific parameters */
	int num_active_funcGroups;

	double ***stock_struct_prop; /**< Array of proportional stock structure (here so fisheries code can
	 access for use in setting regional TACs */
	double **pFLEET; /**< Array of catch (as diet item) availability */

	/**@name
	 *	Prey availability arrays:
	 *	There are quite a few of these but each have a different level of information.
	 *
	 *
	 *
	 */
	/*@{*/

	double ***pSPageeat; /**< Array of age specific diet availability
	 (for finer scale ontogentic shifts in prey availability) */
	double ****pSPFCeat; /**< Array of availability of catch as prey */
	double ****pSPVERTeat; /**< Array of diet availability to vertebrates and invertebrates that eat fish. */
	double ****tempPopRatio; /**< Ratio of age classes within age phases but not just for vertebrates -
							 used in demographic code store it here so can use it in assessment library)*/

	//double ****agingVERT; /* Storage for newly aged vertebrates */
	//double **agingMIG; /* Storage for newly aged vertebrates in migration array */
	//double **C_spVERT;			/* Clearance rates for vertebrates */
	//double **FSPB; /* Spawning ogives for vertebrates */
	//double **mum_spVERT;		/* Growth rates for vertebrates */
	double ***recruit_hdistrib; /* Spatial distribution for vertebrate recruits */
    double ***recruit_hdistrib_orig; /* Spatial distribution for vertebrate recruits */
	double ***mS; /* Additional seasonal mortality rates for vertebrates */
	double ***refuge_status; /* Refuge status of habitat for vertebrates */
    double ****turbid_effect; /* Refuge from predation provided by turbidity conditions - only in effect if flagIsEstuary is active (on) */

    double *tot_SSB; /* Spawning stock biomass */
    double **tot_cohort; /* Total cohort numbers */

	double **coveramt; /* Proportional cover by each substrate type per box */
	int REEFcover_id;
	int FLATcover_id;
	int SOFTcover_id;
	int CANYONcover_id;
	int flag_rel_cover;

	double *totbiom; /* Total biomass for each group */
	double **groupTotCatch; /* Biomass array for use in eat */

	double *diagnosticBiom; /* Diagnostic biomass value for each group */

	double **reg_prop; /* Biomass split between reporting regions for reporting of biomass output */
	double **initreg_prop; /* Inital split of biomasses between reporting regions for biomass output */
	double **lastreg_prop; /* Inital split of biomasses between reporting regions for biomass output */
	int *regID; /* ID for each box indicating which biomass reporting region it is in */
	double ***HABITATlike; /* Habitat preferences */
	int **stock_struct; /* Array mapping stocks to boxes */
	int **v_stock_struct; /* Array mapping stocks to layers */
	int ***group_stock; /* Group stock value */

	double ***HomeRangeTotal; /**< Array of total numbers of each species in each home range */
	int ***scaling_indices; 	/** Array to store scaling indices - used for size, mum and efficency rates */

	/** Larval Dispsersion matrices */
	int use_larvalfiles;
	int larval_rewind;
	int larvalYears; /* Number of years there is larval data in the input netcdf file */
	double **larvaldistrib;
	double ****connectivitymatrix;

	/*@}*/

	/**@name
	 * Invading species parameters and flags
	 */
	/*@{*/

	int invading_sp_model; /**< Invading species model */
	int InvaderIndex; /**< Invading species guild index */
	int minInvaderAge; /**< Minimum cohort entering as invading species */
	int maxInvaderAge; /**< Maximum cohort entering as invading species */
	int InvaderEntryBox; /**< Entry box for invading species */
    int InvaderEntryLayer; /**< Entry layer for invading species */
    int InvadersEntering; /**< Flag indicating invading species has explicit numbers enter, not just amplification of existing numbers */
	double InvaderStartDay; /**< Day invading species enters the system */
	double InvaderEndDay; /**< Day invading species stops entering the system */
	double InvaderSatDay; /**< Day when entire area is covered with invading species */
	double InvaderScalar; /**< Increase per year of invading species */
	double InvaderSpeed; /**< Speed of spread of invading species */
	double InvaderMinDepth; /**< Minimum depth invading species tolerates */
	double InvaderMaxDepth; /**< Maximum depth invading species tolerates */

	int Invadendt; /**< Number of entries in InvadeArray */
	int InvaderPID; /**< Invading species tracer index */
	double Invadedt; /**< Number of days marking time between invading species spreading distribution snapshots */
	double **InvadeArray; /**< Array mapping invading species spread */
    double **invader_spread_scalars;
    double *InvaderStartNums; /**< Numbers of each age group invading */

	/*@}*/

	double temp_influence; /** Temperature correction for recruitment  */

	/**
	 * Data for external biology forcing.
	 */
	int use_external_scaling;
	int scale_all_mortality;	/* If true then scale all of the mortality of the groups */
	int mortality_addition;	/* If true then just add the mortality term */
	EcologyScalarData *externalBiologyInput;

	/**@name
	 * Eddy influence
	 */
	/*@{*/
	double eddy_scale; /**< Scalar for eddys on phytoplankton growth */
	double eddy_mixscale; /**< Scalar for eddys on vertical mixing */
	int eddy_vertmix; /**< Flag indicating whether to scal vertical mixing by eddy strength */
	int use_eddy_NC;	/** Flag to indicate the eddy value is forced using a netcdf file. */



	/*@}*/

	/**@name
	 * Port information
	 */
	/*@{*/
	int K_num_ports;

	double **Port_info; /* Port information */
	double **Port_distance; /* Distance from each point to each box */
	double ***PortContrib; /* Contribution of each port to effort of each fleet - taking into account distance as measure of cost */
	double **totPortContrib; /* Sum of port contributions (totaled per fishery) so proportional allocation can be made) */

	/** Fisheries and management model information **/
	double invert_incidmort; /* Proportion of discarded iverts that survive */
	double KDEP_fishery; /* Depth fishery gear penetrates into sediment */
	double Speed_boat; /* Speed of fishing boats */
	double Speed_recboat; /* Speed of recreational fishing boats */

    int K_max_co_sp;
    int K_max_impacted_sp; /* id of the last impacted species listed in the groups.csv file */
	int K_num_fisheries; /* Maximum number of fisheries in the model */
	int K_max_num_subfleet;
	int K_max_num_zoning; /* Maximum number of fisheries zonings in the model */
	int K_num_basket; /* Maximum number of groups in a basket or companion quota in the model */
	int K_num_catchqueue; /* Length of the catch queue in days for use in running average of
	 recent catch (typically a week or month) */

	int need_pops; /* Flag indicating populations need tracking for fisheries purposes */
	int Council_needed; /* Flag indicating whether need to visit management councils in US-like framework */
	int min_fstatid; /* Minimum fisheries statistics fstat ID - for use in refreshing pools
	 for cumulative records */
	int max_fstatid; /* Maximum fisheries statistics fstat ID - for use in refreshing pools
	 for cumulative records */

	int conflict_id; /* ID number of gear conflict indicator in data structure */

	int flagfinfish; /* Flag inficating fish fin-fish */
	int flagincidmort; /* Flag indicating incidental mortality on */
	int flagendangered; /* Flag indicating whether using management based on PET groups */
	int flagSSBforHCR; /* Flag indicating whether using SSB for HCR (1) or total B (0) */
	int flagmpa; /* Flag indicating form of spatial management used */
	int flagrollingmpa; /* Flag indicating whether have rolling spatial management or not */
	int flaginfringe; /* Flag indicating whether there are management infringements */
	int flag_sel_with_mFC;	/* Flag indicating whether use selectivity to indicate ages where fishing mortality applied */
    int flag_stop_F_tac;  /* Flag indicating whether use TAC to limit fishing where fishing mortality applied */
	int flagdisplace; /* Flag indicating whether effort displaced */
	int flagbuffereffort; /* Flag indicating whether to buffer effort changes */
	int flagchangeeffort; /* Flag indicating whether using "effort change" scenarios */
	int flagchangediscard; /* Flag indicating whether using "discard change" scenarios */
	int flagchangesel; /* Flag indicating whether using "selectivity change" scenarios */
	int flagchangeF; /* Flag indicating whether using "forced fishing mortality change" scenarios */
	int flagchangeq; /* Flag indicating whether using "catchability change" scenarios */
	int flagchangep; /* Flag indicating whether using "availability change" scenarios */
	int flagchangecap; /* Flag indicating whether using "effort cap change" scenarios */
	int flagchangeswept; /* Flag indicating whether using "swept area change" scenarios */
	int flagnewport; /* Flag indicating whether new ports come on line */
	int flagchangepop; /* Flag indicating whether using "human poopulation change" scenarios */
	int forcedTACchange; /* FLag indicating whether using "forced TAC changes" scenarios */
	int flagreinitpop; /* Flag indicating whether use alternative (non zero) date for specifiying virgin biomasses */
	int flagdepend_dis;		/**< Flag at least one fishery uses density dependent discards */
	int flagimposeglobal; /**< Flag indicating whether single global or individually spatially explicit
	 historical catch time series are in use */
	int flaghighgrading; /**< Flag indicating whether high grading */
	int flagmarketdiscard; /**< Flag indicating whether have market-based discarding */
    int flagkeepZeroCatchTS; /**< Flag indicating cannot change TS distribution if zero */
	int UpdateMaxSalePrice; /**< Flag indicating whether want max_sale_price updated (1) or not (0) */
	int flagdyn_coupdate; /**< Flag indicating whether to use static or dynamic catch rations in companion
	 TAC calculations */
	int dynanyway; /**< Flag indicating whether want dynamic effort allocation and pseudo assessment or
	 pseudo movement to go with the pseudo assessment */
    
    /* Ecosystem cap related */
    int maxF_aggregate; /**< Flag to do with system cap calculations - indicagging whether want maxF as sum over mFC or max of mFC */
    int use_time_avg_biom;  /**< Flag to do with system cap calculations -  if using B not N in calculating expected catch, is whether use avg min,max B stored over the year or rolling_B  */
    int use_time_avg_wgt; /**< Flag to do with system cap calculations -  if using N not B in calculating expected catch, is whether use avg min,max wgt stored over the year or rolling_wgt */
    int syst_cap_calc_method; /**< Flag to do with system cap calculations  - whether use numbers based application of the catch equation or just average B */
    int sp_pref_inv_norm_done; /**< Flag to do with system cap calculations - whether the normalised weighting has been done for later use in ecosystem cap HCR */
    int M_est_method; /**< Flag to do with system cap calculations - which option for calculating M */
    double Ecosystm_Cap_tonnes; /**< Value of ecosystem catch cap in tonnes */
    int K_rolling_cap_num; /**< Value used to create array and refer to values - will be ceil(12 * bm->K_cap_rolling_period) */
    double K_cap_rolling_period; /**< Length of the period to using for the roling average in years */

    int **rolling_cap_initialised; /**< Flag so know whether initialised or not as yet */
    
    int flagSimpleStartStopMPAs; /**< Flag indicating simple start and stop dates for MPAs un use */
    int MPAstartyr; /**< Run day that MPAs start */
    int MPAendyr; /**< Run day that MPAs end */

	int DynDAS; /**< Flag indicating whether using dynamic days-at-sea */

	int TradeEffort; /**< Flag indicating whether using ITE tradable effort model */

	int flagTACincludeDiscard; /* Flag indicating whether TACs include discards in the account keeping */
	int flagQuotaBound; /* Flag indicating whether want to bound dynamic TAC to be a band around original TAC */
	int flagForceRec; /* Flag indicating whether using forced recreational fishing or economically driven version */
	int flagTradeTACvsMPA; /* Flag indicating whether trade spatial closures for reduction in TAC */
    
	int manage_reg; /* Whether using management region or stocks as defining regional TACs */

	int *reg_season; /* Flag of whether seasonal closures are only applied in specific regions */

	double reinit_pop_day; /**< day to use for date of virgin biomass re-initialisation */

	double recruitRange; /**< max relative magnitude (strength) of large age classes for vertebrates
	 with occasional strong recruitment pulses */
	double recruitRangeFlat; /**< max relative magnitude (strength) of large age classes for vertebrates
	 without occasional strong recruitment pulses */
    
    double ref_chl;  /**< reference level of chlorophyll or plankton for plankton dependent recruitment relationships - moved from ecology.h */

	double ****vvdistrib; /**< Vertical distributions of vertebrates (so can condition
	 time series of catch) */

	//    int    ntsCatch;        /**< Number of catch time series (should match one per box at most,
	//							   but keep this int as check) */
	//	int    tsCatchtype;		/**< Whether to use interpolated or last valid entries from time-series */
	//	int	   *tscatchid;		/**< Array matching boxmodel species ids to catch time series entry ids */
	//    FisheryTimeSeries *tsCatch;    /**< List of catch time series (one per box) */
	//
	//    int    ntsDiscard;        /**< Number of discard time series (should match one per box at most,
	//							   but keep this int as check) */
	//	int    tsDiscardtype;	  /**< Whether to use interpolated or last valid entries from time-series */
	//	int	   *tsdiscardid;	  /**< Array matching boxmodel species ids to discard time series entry ids */
	//    FisheryTimeSeries *tsDiscard; /**< List of discard time series (one per box) */

    int	**BoxKeyMap;  /**< Indices for use with catch, discards and recruitment time series */
    
	int ntsEffort; /**< Number of effort time series (must match one per box,
	 but keep this int as check) */
	int tsEfforttype; /**< Whether to use interpolated or last valid entries from time-series */
	int *tseffortid; /**< Array matching boxmodel species ids to effort time series entry ids */
	FisheryTimeSeries *tsEffort; /**< List of effort time series (one per box) */

	int ntsMPA; /**< Number of MPA time series (should match one per box at most,
	 but keep this int as check) */
	int tsMPAtype; /**< Whether to use interpolated or last valid entries from time-series */
	int *tsMPAid; /**< Array matching boxmodel fisheries ids to catch time series entry ids */
	FisheryTimeSeries *tsMPA; /**< List of MPA time series (one per box) */

	double **FISHERYprms; /**< Array of fisheries specific parameters */
	double ***SP_FISHERYprms; /**< Array of species specific fisheries parameters */
	char **SP_FISHERYprmsName; /**< Array of species specific fisheries parameters names*/
	//int	*SPid;				  /**< ID mapping of species ids to fisheries species ids */

	int **sp_basket; /**< ID of species in basket */
	int **FISHERYtarget; /**< Fishery targeting (flag indicating whether group is targeted by fishery) */
	int ***SUBFISHERYtarget; /**< Targeting per subfishery - used in the effort model to weight interest in fishing */

	int **pSPcheck; /**< Array of flags to indicate whether cohort is depleted and
	 CatchTS_agedistrib squeeze required */

	double **FCcocatch; /**< Ratio of discards of species x to catch of species y - used to
	 determine discarding if use depend_discard */
	double **FishingResults; /**< Immediate (this time-step, this box) catch results for
	 assigning in current time-step */

	double ***FCcaughttemp; /**< Catch available as prey pool in current time-step */
	double **FCcaught; /**< Cumulative fisheries catch statistics for storage and model output */
	double ***FCtsCarryOver; /**< Carry over uncaught hisorical timeseries catch per box */
	double **FCdiscard; /**< Cumulative fisheries discard statistics for storage and model output */

	double ****Catch; /**< Current time-step fine scale (per box and age group)
	 catch statistics for storage and model output - seems to be in tonnes - already multiplied by box volume and dt.*/

	double ****CumCatch; /**< Cumulative fine scale catch statistics */
	double ***LastCatch; /**< Running average of the recent catch - for calculating recent
	 CPUE for some effort models */
	double ****CatchQueue; /**< List for recent catches - for use in LastCatch */
	double ****MPAlist; /**< List of potential zoning (spatial management) */
	double **MPA; /**< Active zoning (spatial management) */
	double ***FFCDR; /**< Array of discard rates */
	double ***Discards; /**< Current time-step fine scale discard statistics */
	double ***CumDiscards; /**< Cumulative fine scale discard statistics */
	double ***DependDiscardsTot;	/**< Discards dependent on catch of other species - outside own fishery interaction */
	double ***CatchTS_agedistrib; /**< Proportion of time series catch due to each age group */
	double ***CatchTS_agedistribOrig; /**< Proportion of time series catch due to each age group */
	double ***DiscardTS_agedistrib; /**< Proportion of time series discard due to each age group */
	double ***DiscardTS_agedistribOrig; /**< Proportion of time series discard due to each age group */
	double ***Effort_hdistrib; /**< Relative spatial distribution of effort - if read in it is
	 by quarter, but if calculated by the economics model it is
	 per month */
	//	double ***MPAendangeredlist;        /**< List of potential zoning (spatial management) based on
	//								           PET species needs */

	double **MPAoverfishedlist; /* List of potential zoning (spatial management) based on
	 overfished species (for US-like system) */
	double ***RegionalData; /* Regional management information */

	double explore_thresh_effort; /* Threshold proportion of effort (e.g 0.1) beyond which will

	 expend exploratory fishing in adjacent cells, if fleet
	 dynamics model uses exploratory fishing */
	double **CumEffort; /**< Cumulative effort statistics */
	double **GhostEffort; /**< Cumulative effort statistics - including effort penalties if using effort cap and spatial incentives */
	double **EffortPenalty; /**< Spatial incentives/penalties */

	double **OldCumEffort; /**< Previous year's cumulative effort statistics */
	double *TotOldCumEffort; /**< Total of pervious year's cumulative effort statistics for each fishery */
	double **Effort; /**< Current time-step effort statistics */
	double **OldEffort; /**< Previous effort statistics */
	double ***RecCatch; /**< Recreational catch statistics */
	double **targetspbiom; /**< Biomass map to condition fisheries effort allocation
	 and targeting of trips */
    
    double **CumDisplaceEffort; /** Cumualtive displaced effort - for reporting purposes */
    
    double ***selectivity;    /** Constant selectivity per cohort or stage for each group (for the invertebrates it is identical to the entries above for constant selectivity regardless of size.*/

	double ***TACamt; /* Array of total allowable catch levels */
	double ****BiTACamt; /* Array of bimonthly total allowable catch levels */
	double ****BiTAC_sp; /* Regional bimonthly total allowable catch levels for each species - US-like system */
	double **TAC_trigger; /* Array of switches showing if TAC-based closures have been triggered */
	double **TempCPUE; /* CPUE for use in conditioning fisheries effort allocation and targeting of trips */

	double **TotCumRecCatch; /**< Total cumulative recreational catch over the entire region (or stock) */
	double **TotOldCumCatch;  /**< Total cumulative catch over the entire region (or stock) for previous year(s) */

	double **TotCumBiCatch; /**< Total cumulative bimonthly catch over the entire region (or stock) - per fishery */
	double **TripCatch; /**< Total catch taken in this trip - mainly for economically driven effort model */
	double *totcatch; /**< Total catch per species in local box - for output storage */
	double *totCPUE; /**< Total previous CPUE (to normalise distribution used for
	 conditioning fisheries effort allocation and targeting of trips */
	double *totdiscards; /**< Total discards per species in local box - for output storage */
	double **TotCumDiscards; /**< Total cumulative discards over the entire region (or stock) - per fishery */
	double **TotOldCumDiscards;	  /**< Total cumulative discards over the entire region (or stock) - per fishery for the previous year(s)*/

	double *totinitpop; /**< Array of overall (per species) virgin biomasses */
	double *totfishpop; /**< Current per species biomasses */
	double **totagepop; /**< Current per age group per species biomasses */

	double *totOldEffort; /**< Array of previous effort levels (for use when buffering effort changes) */
	double *totNewEffort; /**< Array of near future effort levels (for use when buffering effort changes) */

	double **boxBiomass; /* Biomass per box */

	double *DAScalc; /**<Array for calculation new days-at-sea */

	double ***POPchange; /**< Array storing human population change information */

	int ***TAC_over; /* Record of groups, stocks and fisheries that have had TAC exceeded */
	int **inQuota; /* Whether fishery involved in fisheries quota or not */
	double **FC_HABITATlike; /* Fisheries habitat preferences (accessibility) */
	int **Port_Fishery; /* Array of ports used by each fishery */

	char **fisheryParamNAME;/**< Array of fisheries parameter names */

	double **phased_out; /* Array indicating whether TAC for the group being phased out */
	double targ_refA; /* Target reference point A (e.g. B48) for tiered harvest rules */
	double targ_refB; /* Target reference point B (e.g. B40) for tiered harvest rules */
	double targ_refC; /* Target reference point C (e.g. B60) for tiered harvest rules */
	double targ_refD; /* Target reference point D (e.g. B50) for tiered harvest rules */
  double targ_refE; /* Target reference point E (e.g. B20) for tiered harvest rules */
	double lim_ref; /* Limit reference point for assessments and TAC setting */
	double forage_refA; /* Target reference point A (e.g. B48) for forage species in tiered harvest rules */
	double forage_refB; /* Target reference point B (e.g. B40) for forage species in tiered harvest rules */
	double forage_refC; /* Target reference point C (e.g. B60) for forage species in tiered harvest rules */
	double forage_refD; /* Target reference point D (e.g. B50) for forage species in tiered harvest rules */
  double forage_refE; /* Target reference point E (e.g. B20) for forage species in tiered harvest rules */
	double forage_lim_ref; /* Limit reference point for forage species in assessments and TAC setting */
    double byproduct_refA; /* Target reference point A (e.g. B48) for byproduct species in tiered harvest rules */
    double byproduct_refB; /* Target reference point B (e.g. B40) for byproduct species in tiered harvest rules */
    double byproduct_refC; /* Target reference point C (e.g. B60) for byproduct species in tiered harvest rules */
    double byproduct_refD; /* Target reference point D (e.g. B50) for byproduct species in tiered harvest rules */
    double byproduct_lim_ref; /* Limit reference point for byproduct species in assessments and TAC setting */
    double bycatch_refA; /* Target reference point A (e.g. B48) for bycatch species in tiered harvest rules */
    double bycatch_refB; /* Target reference point B (e.g. B40) for bycatch species in tiered harvest rules */
    double bycatch_refC; /* Target reference point C (e.g. B60) for bycatch species in tiered harvest rules */
    double bycatch_refD; /* Target reference point D (e.g. B50) for bycatch species in tiered harvest rules */
    double bycatch_lim_ref; /* Limit reference point for bycatch species in assessments and TAC setting */
    
    double discardTAC;  /* Discard TAC */
    int flagdisableTAC; /* Whether TACs intentinally ended at any point in the run */
    
    double TAC_multiyr; /* Length of CPUE trend before act on TAC resetting */
	double TACbound; /* Maxmimum proportional increase in TAC above initial level at start of run */
	double prop_incTAC; /* Proportional increase in TAC when its rising */
	double prop_decTAC; /* Proportional decrease in TAC when its dropping */
	double critCPUE; /* Critical drop in CPUE before drop TAC */
	double RBCalpha; /* Controlling coefficient in tier 4 harvest rule */
	double TAC_crit_change; /* Proportional change before worth making TAC shift */
	int do_MSY; /* Whether doing MSY related management for any species or not */
	int bulkTAC;			  /* Whether multi-year TACs are one total for the entire period or if
								 it is annual quota to track just not doing an assessment for x years */
	int OldCatchReset;		  /* Whether need to reset the OldCatchSum and TotOldCumCatch arrays */
    
    int EffortModelsActive; /* Flag to indicate an effort model is in use */

	/* Parameters for tiered assessments - for catch-cost-risk analyses */
	RBCstructure RBCestimation;
	TimeSeries *tsTACmult; /* TAC_mult forcing for each tier assessed species*/
    CloseKinstructure *CloseKinEst;
	/**/

	double highgrade_thresh; /**< High grading catch composition threshold (if age class typically
	 makes up less than this proportion of the catch then discard it
	 if high grading) */
	double salethresh; /**< If a group has a saleprice less than this proportion of the
	 maximum sale price then it will be discarded under market-driven
	 discarding */
    
	/*@}*/
    
    /********** Shot per shot related parameters ***********/
    
    int flagStoreShotCPUE; /**< Flag indicating whether storing per shot CPUE */
    int flagStoreCPUE; /**< Flag indicating whether storing per time step CPUE */
    int flagfullCPUEreport; /**< Flag indicating whether to store "remaining to be allocated catch/effort data" or just the calculated shots only */
    int PerShotCDFtype;  /**< Shape of cdf distribution for generating CPUE */
    int CPUEshotfdis;   /**< Flag indicating what error type (normal etc) to use for discards when generating shot-by-shot CPUE data */
    double CPUEshotavgdis;   /**< Bias (as proportion pf true value) parameter for discards when generating shot-by-shot CPUE data */
    double CPUEshotvartdis;   /**< Variance (as proportion of the mean) parameter for discards when generating shot-by-shot CPUE data */
    int CPUEshotfeff;   /**< Flag indicating what error type (normal etc) to use for effort when generating shot-by-shot CPUE data */
    double CPUEshotavgeff;  /**< Bias (as proportion pf true value) parameter for effort when generating shot-by-shot CPUE data */
    double CPUEshotvareff;   /**< Variance (as proportion of the mean) parameter for effort when generating shot-by-shot CPUE data */
    double gun_flexthresh;  /**< Flex weight valiue that marks threshold from mediocre to "gun" (very effective) skippers */
    double K_min_shotlength;  /**< Minimum length of a shot (in hours) */
    int K_num_catchbin;     /**< Number of bins in catch size dstribution */
    double size_catchbin;   /**< Size of catch bins used to create catch distribution (e.g. 50kg) */
    double min_effort_coefft;  /**< Coefficient in linear relationship between atch taken and effort spent to obtain it */
    double min_effort_const;  /**< Constant in linear relationship between atch taken and effort spent to obtain it */
    
    int K_max_num_boats;    /**< Absolute maximum number of boats in any one fishery based on current parameterisation */
    int *box_done;          /**< Tracker to see if box already covered in identifying locations for shots */
    int *shot_loc;          /**< Location of each shot */
    double *alloced_effort; /**< Effort in each box already allocated in ShotData */
    double ***alloced_catch;  /**< Catch in each layer of each box for each species already allocated in ShotData */
    double **alloced_discard;   /**< Discard in each box for each species already allocated in ShotData */
    double *prop_depth;     /**< Proportional remaining catch at each depth - for allocating shot */
    double *prop_effort;     /**< Proportional remaining effort in each box - for allocating shot */
    double ***ShotData;     /**< Final generated shot by shot data - written over for each fishery 
                             at each time step, so needs to be written out as soon as generated */
    
    /********************/

	int sample_now; /* flag indicating whether an sampling in this survey instance */
	int annual_assess; /* flag indicating whether time for annual calculation and output of indicators */
	int annual_sample; /* flag indicating whether an annual sample is being taken */
	int flagecosysassess; /* flag indicating whether ecosystem indicators calculated */
	int flagobsdata; /* flag indicating whether observer data calculated */
	int flagareacorrect; /* flag indicating whether to area correct biomass indices for sampling output */

	double totarea; /* Total area sampled */
	double inshorearea; /* Relative area modelled that is inshore */
	double offshorearea; /* Relative area modelled that is offshore */

	/* Assessment run parameters */
	int samplediet; /* flag indicating whether sampled diet matrices are output */
	int ageout; /* flag indicating whether sample age info is output */

	double last_snap; /* time of last intensive sampling */
	double tassessstart; /* time assessments should start */
	double tClassicalAssessStart; /* time formal assessment model kicks in */
	double tassessinc; /* increment between assessments */
	double teatassessinc; /* increment between "years of the stomach" */
	double tassess; /* next time the sample colection and assessment code should run */
	double teatassess; /* next time diet sampling should occur */
	double minfreq; /* Minimum frequency of patchy sampling */
	double maxfreq; /* Maximum frequency of patchy sampling */
	int flaggen; /* Whether generating or loading random number sequences */
	int flagpatchy; /* Whether using patchy sampling */
	int flagcont; /* Whether using continuous within year sampling for vertebrates */
	int nsbox; /* Number of sampling locations */
	int nsbox_i; /* Number of sampling locations - intensive sampling */
	int nsbox_o; /* Number of sampling locations - other (less intensive) sampling */
	int nfzones; /* Number of fishing zones */
	int *nsboxes; /* Identity of the sampling locations */
	int *nsboxes_intense; /* Identity of the sampling locations */
	int *nsboxes_other; /* Identity of the sampling locations */
	int K_num_tot_assess; /* Total number of groups in the assessment model */
	int K_num_live; /* Total number of living groups in the model */
	int K_num_detritus; /* Total number of detritus groups in the model */
	int K_num_eweinvert; /* Total number of invertebrate groups in the ecopath submodel */
	//int K_num_charis; /* Total number of charismatic groups in the model */
	int K_num_size; /* Number of vertebrate size bins */
	int K_num_sampleage; /* Number of age classes sampled */
	int K_num_pelbin; /* Number of pelagic bins in biomass size spectra */
	int K_num_sedbin; /* Number of sediment bins in biomass size spectra */
	int K_num_abcbin; /* Number of bins in the ABC curve calculation */
	int K_num_trophbin; /* Number of bins in trophic spectrum */
	int K_max_agekey; /* Maximum recorded age of any vertebrate in the model - for age length key */
	int maxTL; /* Maximum number of trophic levels for Lindeman network analysis */
	double cdz; /* Critical depth defining inshore-offshore break */

	double **calcFnum; /* Array for storing pseudo fishing mortality estimates */
	double **calcMnum; /* Array for storing pseudo natural mortality estimates */
	double ***calcMnumPerPred; /* Array for storing pseudo natural mortality estimates per predator */
	double **calcNstart; /* Array for storing pseudo recruit estimates */
	double **calcNstartPerPred; /* Array for storing pseudo number estimates - for population level M estimates */
	double ****calcTrackedMort;  /* Array tracking specific natural mortality per species */
	double *****calcTrackedPredMort;  /* Array tracking specific natural mortality per predator species */

	double **calcMLinearMort; /* Array for storing vertebrate linear mortality values */
	double **calcMQuadMort;
	double **calcMPredMort;
	double **calcELinearMort; /* Array for storing external mortality values */

	int ***rep_box_of_zone; /* Representative box for the zone for the species */

	double ***rand; /* Array of random numebrs used in pseudo assessment,
	 either generate internally or load from previous file,
	 so can be sure of consistency between runs */
	double *estBo; /* Array for virgn biomass estimates for pseudo assessment */
	double **tassPatchy; /* String of gaps between samples (for patchy sampling) */
	int pseudo_assess; /* Flag indicating whether using pseudo assessments */
    int do_sumB_HCR; /* Whether the harvest control rule applies to the sum of the guild rather than per species */
    int do_syst_cap; /* Whether using an ecosystem cap in management or not */

	/**@name
	 * Assessment model parameters
	 */
	/*@{*/

	/**@name
	 * Globals to do with calculating system wide indicator values
	 */
	/*@{*/
	double globalHabstep1; /**< For habitat index */
	double globalHabstep2;
	double globalstep1; /**< For nppb index */
	double globalstep2;
	double globalAscen; /**< For ascendency index */
	double globalOverhead;
	double globalCapacity;
	double globalTruPut;
	double globalsysom; /**< For soi index */
	double globalsysdenom;
	double globaldiscards; /**< For discard rate index */
	double globalcatches;
	double globalNcep; /**< For PBR index */
	double globalPBRcatcep;
	/*@}*/

	/**@name
	 * Fisheries assesment parameters and arrays
	 */
	/*@{*/
	double ***EffortRecord; /**< Effort statistics on annual basis */
	double ****CatchRecord; /**< Catch statistics on annual basis */
	double **NAssess; /**< Assessment results */
	double *estinitpop; /**< Estimated virgin biomass */

	char **NAssessNAME; /**< name of estimated assessment parameters and variables */

	double Assess_Tol; /**< Fractional convergence tolerance of minimisation
	 routine (Amoeba) for assessments */
	double Assess_Btol; /**< Fractional convergence of BrefA calculations */
	int Assess_Max_Int; /**< Maxiumum number of iterations of Amoeba */
	int assessnyr; /**< Number of years to run out assessment "long term" stability runs */
    
    int PGMSY_on; /**< Whether PGMSY being used */

	char cabspot[BMSLEN]; /**< pathname for cab.exe for use in integrated assessment */
	char cabIfname[BMSLEN]; /**< name of cab input file for use in integrated assessment */
	char cabOfname[BMSLEN]; /**< name of cab output file (contains integrated assessment
	 results to read back into Atlantis) */
	/*@}*/

	/**@name
	 * Economic model parameters
	 */
	/*@{*/
	int immed_cost_only; /**< Whether to use immediate or all costs */
	int UseConstPrice; /**< Whether use constant fish prices through time (1) or dynamically update using market prices */
	int UseEconQuotaDefine; /**< Whether using econ file TAC in place of harvest file TAC (1) or adding them (0) */
	int UseMinValue; /**< Whether to use minimum value for a fish in setting quota price (i) or ignore price terms if price is zero (0) */
	int MultiPlanEffort; /**< Economic model in use - Fulton = 1, Holland = 0 */
	int OrigEconCalc; /**< Whether to use the original economic driver calculation (1) or use one that is total price - total cost */
	int EconLimDemand; /**< Whether to limit trading based on potential effort levels vs days left in month (yes = 1, no = 0) */
	int reset_month_val; /**< Flag to indicate whether or not to reset monthly values - need to do this after indicators calculated */
	int econweekly; /**< Whether finest economically driven effort allocation is weekly or otherwise */
	int quota_trading; /**< Whether quota trading in effect in the economic model */
	int K_num_markets; /**< Number of markets in the model */
	int hist_only; /**< Flag indicating whether historic spatial effort used or whether dynamically updated */
    int rescale_post_burnin;  /**< Flag indicating whether to rescale effort, catch, quota after the end of the burn-in period so can do rapid depletion experiment */
	double hist_only_timeout; /**< Day of run when the hist_only flag times out (so can have forced and dynamic periods) */
	double DVstart; /**< Day of run when management using deemed values starts */
	double TaxStart; /**< Day of run when management using taxes starts */
	int TaxDone; /**< Flag indicating whether the taxes have been initialised */

	int UseVPUE; /**< Flag to indicate whether using CPUE or VPUE to dictate effort distributions */

	int stochastic_effort; /**< Flag indicating whether stochastic or deterministic effort models used */
	int fish_withoutQ; /**< Flag indicating whether can fish without quota */
	int flagsupp_allowed; /**< Whether flexible shortterm gear switching/supplementing is allowed */
	int flagnewboat; /**< Flag indicating whether new boats are allowed to enter a fishery
	 (not always the case in reality as may only have vessel exit strategy
	 so can contain effort) */
	int flagslowsell; /**< Whether want boats to slowly leave fishery if close-run thing or whether want
	 to sell up as soon as trigger tripped
	 */
	int flagallmonths; /**< Whether adjust effort mon-by-month or through the year simultaneously,
	 as new knowledge comes in */
	int flagswitch; /**< Flag indicating whether can switch gear at will or not */
	int sp_by_sp; /**< Flag indicating whether trading model treats
	 species independently or as a single unit */

	int flagLoadDiscardHist; /* Flag indicating on whether loading discards black book style or creating them from quasi-perfect knowledge */

	int flagLease; /* Flag indicating which lease price model to use */
	double month_scalar; /* Scalar to indicate number of days per month to apply when calcualting max_month_effort */
	int max_month_effort; /* Maximum effort possible per month */
	int min_month_effort; /* Minimum effort possible per month */
	double interest_rate; /* Interest rate to use in economic calculations */
	double high_price; /* Arbitary high price for species where quota exceeded - used in Holland ITQ model */
	int ITQ_think_ahead; /* Flag indicating whether Dan Holland ITQ showing forethought */
	int yr_horizon; /* Number years into the future fishers think during economic calculations */

	double shorecost; /**< Costs of being ashore - if accumulated cash < than this then will not go to see
	 unless tot_marg_rent > 0 */
	double prop_leave; /**< Maximum proportion of the fleet that can leave in any non-buyout month */
	double prop_switch; /**< Maximum proportion of the fleet that can switch gears in any month */
	double prop_supp; /**< Maximum proportion of the fleet that can flexibility supplement gears in any month */
	double prop_boatbound; /**< Lower bound on proportion of the starting fishery which remains over simulated period */
	double ***SUBFLEET_ECONprms; /**< Paramters per subfleet / vessel type */
	double ***ECON_INDprms; /**< Paramters for calculation of economic indicators
	 based on ecological and harvest indicators using
	 functional responses defined by Gerry Geen and
	 Tom Kompas */
	double ****QuotaAlloc; /**< Quota allocation for each subfleet of each fished group
	 (equivalent of Rich's Qallocation and purchasedQuota)
	 with entry 0 = owned, 1 = leased amounts,
	 2 = target classification (whether target/byproduct/unwanted)
	 3 = unit cost to catch (per kg), 4 = marginal profit value
	 5 = current catch, 6 = marginal rent, 7 = utility, */
	double ****QuotaTrade; /**< Trades between different fisheries per species */
	double ****EffortSchedule; /**< Monthly effort schedule with entry 0 = historical, 1 = expected
	 (equivalent of Rich's Eschedule) */
	double *****BlackBook; /**< Monthly historical catch record */
	double *****SpatialBlackBook; /**< Spatial historical effort record */
	double ****MargProfit; /**< Monthly marginal profit record */
	double ****MargRent; /**< Monthly marginal rent record */
	double ****MonthAlloc; /**< Monthly allocations of fishing effort and expected catch */
	double ****BoxAlloc; /**< Array used in effort allocation calculations */
	double ****SpatialCPUE; /**< Spatial CPUE map */
	double ****SpatialVPUE; /**< Spatial VPUE map */
	double ****SpatialDisPUE; /**< Spatial discards map */
	double ***MARKETprms; /**< Market specific parameters - usually to do with fish prices */
	double ***MARKETmonth; /**< Month specific market parameters - to capture seasonality
	 in prices due to demand cycles */
	double *FUELprms; /**< Fuel specific parameters - usually to do with fish prices */
	double *FUELmonth; /**< Month specific fuel parameters - to capture seasonality in fuel prices */
	int **Trades; /**< Record of number of trades per month */
	int ***EffortTrades; /**< Record of number of effort trades per month at a fishery level */
    
	double *****ECONexprice; /**< Region specific prices */
    double *prop_hold;      /**< Porportion of total hold capacity in each vessel size class */
    double *EffortReScale;  /**< Arrays for rescaling effort if doing depletion experiment after light depletion burn-in */
    double *CatchReScale;  /**< Arrays for rescaling catch if doing depletion experiment after light depletion burn-in */


	int GDP_id; /**< GDP entry id */
	int FuelCost_id; /**< Fuel cost entry id */
	FisheryTimeSeries *tsGDP; /**< GDP time series (one for entire model) */
	int tsGDPtype; /**< Whether to use interpolated or last valid entries from time-series */
	int tsGDPid; /**< Array matching boxmodel fisheries ids to catch time series entry ids */

	double recon_buffer; /**< Size of minimum reconciliation of quota for ITQ */
	double effort_reduction; /**< Proportion of boats that don't fish each month once debt is crippling */
	double cripple_nboat; /**< Proportion of boats that leave the fishery once debt is crippling */
	int cripple_period; /**< Number of months before debt considered crippling */
	/**< Coefficients for Newell et al lease price model */
	double price_Coefft;
	double price_CoefftSq;
	double cost_Coefft;
	double prev_fill_Coefft;
	double prev_fill_CoefftSq;
	double part_fill_Coefft;
	double part_fill_CoefftSq;
	double bind_Coefft;
	double tac_trend_Coefft;
	double const_Coefft;
	double GDP_Coefft;

	double minValue; /**< Minimum value (cents) of a fish per kg */

	double decom_return_coefft; /**< Coefficients for vessel exiting model - from Thebaud et al 2006 */
	double resale_coeff;
	double decom_crew_coefft;

	double switch_return_coefft; /**< Coefficients for vessel fleet switching - from Guyader 2002,
	 with modifications inspired by Thebaud et al 2006 formulations */
	double switch_coefft;

	double min_size_buffer; /**< Minimum bound (on boat size - as a proportion of boat_size_id value)
	 of boats allowed in a subfleet */
	double max_size_buffer; /**< Maximum bound (on boat size - as a proportion of boat_size_id value)
	 of boats allowed in a subfleet */

	double new_return_coefft; /**< Coefficients for buying a new vessel - inspired by Guyader 2002,
	 and Thebaud et al 2006 formulations */
	double new_coefft;

	double perm_coefft; /**< Coefficients for cost of permanent leasing quota vs temporary lease */
	double buy_coefft; /**< Coefficients for cost of buying quota vs temporary lease */

	double targetscale; /**< Scalar on CPUE distribution so weight for targeting */
	double prop_within; /**< Proportion of quota filled when start seeking more */
	double prop_spareend; /**< Proportion of quota filled when no longer willing to trade quota */
	char friendship_file[STRLEN];	/**> The name of the friendship network input file */

	int ntsResid; /**< Number of time series, but as only one its really effectively a flag to say
	 whether this time series is in use or not */
	int tsResidtype; /**< Whether to use interpolated or last valid entries from time-series */
	int *tseconid; /**< Array matching species ids to time series of residuals entry ids */
	FisheryTimeSeries *tsResid; /**< Time series of residuals for the fish price model */
	/*@}*/

	/**@name
	 * Performance Measure arrays and variables
	 */
	/*@{*/
	int smallID; /**< ID of smallest vertebrate (for size spectra calcs) */
	int largeID; /**< ID of largest vertebrate (for size spectra calcs) */
	double smallsize; /**< individual size of smallest vertebrate (for size spectra calcs) */
	double largesize; /**< individual size of largest vertebrate (for size spectra calcs) */

	double *ecolindx; /**< ecological performance measures (other biomass) */

	double **econindx; /**< socioeconomic performance measures */
	double **portindx; /**< port-based socioeconomic performance measures */

	char **ecolindxNAME; /**< name of ecological performance measures (other biomass) */

	char **econindxNAME; /**< name of socioeconomic performance measures */
	char **SUBFLEETeconindxNAME; /**< subfleet specific socioeconomic parameters and performance measures */
	char **portindxNAME; /**< name of port-based socioeconomic performance measures */

	double tepcoefft; /**< Coefficient for TEP impacts on social acceptance index */
	double sharkcoefft; /**< Coefficient for shark impacts on social acceptance index */
	double habcoefft; /**< Coefficient for habitat impacts on social acceptance index */
	double enforce_coefft; /**< Coefficient for enforcement costs of spatial vs TAC management */

	/**< Bycatch incentive parameters */
	int UsingEffortIncentive; /**< Flag indicating whether using effort-based incentives */
	int BycatchCountDiscards; /**< Flag indicating whether bycatch counts discards */
	int TemporalBycatchAvoid; /**< Flag indicating whether using economic penalties to encourage avoidance of bycatch, by
	 directing effort away from months where expecte high bycatch */
	int SpatialBycatchAvoid; /**< Flag indicating whether using economic penalties to encourage avoidance of bycatch, by
	 directing effort away from locations where expecte high bycatch */

	int renewTrade; /**< Flag indicating whether to refresh the trade matrix */

#ifdef LINK_ENABLED
	/**< Linking model parameters */
	//double *ConvertPRM;			   /**< parameters for linking to models like EwE - loaded from server app */
	EwELinkStruct *eweLinkModule;
#endif

#ifdef BROKER_LINK_ENABLED
	/* Broker Linkage code */
	LinkageInterface *linkageInterface;
	int delayStart;
#endif

	int useRBCTiers;    /**< Whether doing tiered assessments or not */
    int useMultispAssess; /**< Whether doing multispecues assessment or not */
    int useGenMnomial;  /**< How generate the length and age data - whether with GenMnomial or standard in atassess */
    int UsingRAssess;    /**< Whether RAssess file creation required */
    
    char ss3Name[100]; /**< Name of ss3 executible */
    
    int RAssessFisheryID;
    int RAssessSpringSurveyDay;
    int RAssessAutumnSurveyDay;
    int RAssessFixedM;
    int RAssessRefYear;
    
    int RAssessRuseScript;  /* Whether using system call to Rscript (1) or embedded R calls (0) */
    int NumRAssessScriptNames;  /* Number of Rscripts that will be used across functional groups by RAssess */
    
    char RAssessRinName[100]; /* Base of name for RAssess input files - will have the species name appended to the start */
    char RAssessRoutName[100]; /* Base of name for RAssess output files - will have the species name appended to the start */
    char RassessColDelimiter[5]; /* Column delimiter for assessment input csv file */
    char **RAssessRscriptName; /* List of file names for RAssess R scripts - will have the species name appended to the start */
    
    FILE **RAssessFnames; /**< File names */
        
    /**@Multispecies assessment files */
    char metierFileName[100];
    int metierFileIndex;
    FILE *metierFile;
    FILE *Dyn4File;
    FILE *Dyn4CTLFile;
    FILE *tierRBCfp;
    
    FILE *closekinCFile;
    FILE *closekinSampFile;
    FILE *CloseKinPOFile;
    FILE *CloseKinHSFile;

/*@}*/

} MSEBoxModel;

typedef struct {
	//char *name; /* Name of tracer/epibenthos */
	char name[100]; /* Name of tracer/epibenthos */
	int *index; /* pointer to tracer/epibenthos index */
	int Flux; /* flag of diagonostic variable, 1=yes, 0=no */
	int Tol; /* flag of tolerance checking variable, 1=yes, 0=no,
	 2=do only once a timestep, 3=do once a timestep
	 and don't reset to minpool */
	int Bio; /* flag indicating whether flag_id sensitive, 1=yes, 0=no */
    int Contam; /* flag indication whether a contaminant tracer or not */
} Namelist;


/*********************************************************************
 Global variables
 *********************************************************************/
extern double **dvol, ***dtr;
extern double **CatchSum;
extern double ****mFCchange;

extern int it_count, waterboundary;

extern int verbose;
extern int fishtest;
extern int noudunits;

/*********************************************************************
 Prototypes
 *********************************************************************/

/* MSEBoxModel handling prototypes */
void freeMSEBoxModel(MSEBoxModel *bm);
void freeMSEBoxModelGeom(MSEBoxModel *bm);
void readBoxGeom(FILE *fp, char *name, Box *b);
void readMSEBoxModelGeom(char *name, MSEBoxModel *bm);
void readFaceGeom(FILE *fp, Face *f);
void writeBoxGeom(FILE *fp, Box *b);
void writeMSEBoxModelGeom(FILE *fp, MSEBoxModel *bm);
void writeFaceGeom(FILE *fp, Face *f);

/* Parameter and tracer IO prototypes */
int createBMAnnAgeBioDataFile(char *destFolder, char *name, MSEBoxModel *bm);
int createBMAnnAgeCatDataFile(char *destFolder, char *name, MSEBoxModel *bm);
int createBMDataFile(char *destFolder, char *name, MSEBoxModel *bm, int dtype);
int createBMDietDataFile(char *destFolder, char *name, MSEBoxModel *bm);
int createBMSummaryDataFile(char *destFolder, char *name, MSEBoxModel *bm);
int readkeyprm_darray_checked(FILE *fp, int entry_type, char *key, double **p, int *size, int verifsize);
int readkeyprm_iarray_checked(FILE *fp, int entry_type, char *key, int **p, int *size, int verifsize);
void alloc_names(MSEBoxModel *bm);
void readBMEpiData(int fid, int dump, MSEBoxModel *bm);
void readBMEpiInfo(int fid, MSEBoxModel *bm);
void readBMphysData(int fid, int dump, MSEBoxModel *bm, FILE *llogfp);
void readBMphysInfo(int fid, MSEBoxModel *bm);
void readBMTracerData(int fid, int dump, MSEBoxModel *bm);
void readBMTracerInfo(int fid, char *fileName, MSEBoxModel *bm);
//void	readBMDiagInfo(int fid, MSEBoxModel *bm);
void readBMIceData(int fid, int dump, MSEBoxModel *bm);
void readBMIceInfo(int fid, char *fielName, MSEBoxModel *bm);
void readBMLandInfo(int fid, MSEBoxModel *bm);
void readBMLandData(int fid, int dump, MSEBoxModel *bm);


void readBMFisheriesInfo(int fid, MSEBoxModel *bm);
void read_bm_ts(MSEBoxModel *bm, char *key, TimeSeries **ts, char *t_units, char *varname, char *varunit, int *id, void(*err)(char *format, ...));
void read_bm_ts_rewind(MSEBoxModel *bm, char *key, TimeSeries **ts, char *t_units, char *varname, char *varunit, int *id, int *rid, void(*err)(char *format,
		...));

void readRunParams(char *name, MSEBoxModel *bm);
int Util_Read_Run_XML( MSEBoxModel *bm, char *name);
void readSourceSink(char *name, MSEBoxModel *bm);
void writeBMEpiData(int fid, int dump, MSEBoxModel *bm, int dtype);
void writeBMEpiInfo(int fid, MSEBoxModel *bm, int dtype);
void writeBMphysData(int fid, int dump, MSEBoxModel *bm, int dtype);
void writeBMphysInfo(int fid, MSEBoxModel *bm, int dtype);
void writeBMTracerInfo(int fid, MSEBoxModel *bm, int dtype);
void writeBMTracerData(int fid, int dump, MSEBoxModel *bm, int dtype);
void writeBMIceInfo(int fid, MSEBoxModel *bm, int dtype);
void writeBMIceData(int fid, int dump, MSEBoxModel *bm, int dtype);

void writeBMLandData(int fid, int dump, MSEBoxModel *bm, int dtype);
void writeBMLandInfo(int fid, MSEBoxModel *bm, int dtype);

void writeBMAnnAgeBioInfo(int fid, MSEBoxModel *bm);
void writeBMAnnAgeBioData(int fid, int dump, MSEBoxModel *bm);
void writeBMAnnAgeCatInfo(int fid, MSEBoxModel *bm);
void writeBMAnnAgeCatData(int fid, int dump, MSEBoxModel *bm);
void writeBMDiagInfo(int fid, MSEBoxModel *bm, int dtype);
void writeBMDiagData(int fid, int dump, MSEBoxModel *bm, int dtype);
void writeBMDietData(int fid, int dump, MSEBoxModel *bm);
void writeBMDietInfo(int fid, MSEBoxModel *bm);
void writeBMFisheriesInfo(int fid, MSEBoxModel *bm, int dtype);
void writeBMFisheriesData(int fid, int dump, MSEBoxModel *bm, int dtype);
void writeBMSummaryEpiData(int fid, int dump, MSEBoxModel *bm);
void writeBMSummaryEpiInfo(int fid, MSEBoxModel *bm);
void writeBMSummaryInfo(int fid, MSEBoxModel *bm);
void writeBMSummaryData(int fid, int dump, MSEBoxModel *bm);

/* Initialisation prototypes */
void AllocateArrayMemory(MSEBoxModel *bm, FILE *llogfp);
//void	Distance_to_Port(MSEBoxModel *bm);
void hydro_init(MSEBoxModel *bm);
void tempsalt_init(MSEBoxModel *bm);


void initPhysics(MSEBoxModel *bm);
void freePhysics(MSEBoxModel *bm);
void closeHydroFinal(MSEBoxModel *bm);
//void close_tempsalt(MSEBoxModel *bm, int index);
//void closeTempSaltFinal(MSEBoxModel *bm, int index);


void setfisheryNameID(MSEBoxModel *bm);
void setfisheryParamNameID(MSEBoxModel *bm);
void setspNameID(MSEBoxModel *bm);
void sourcesink_init(MSEBoxModel *bm);
void freeSourceSink(MSEBoxModel *bm);

/* Run time library call prototypes */
void physics(MSEBoxModel *bm, double ***newwctr, double ***newsedtr, FILE *llogfp);  // Removed "double ***newicetr, double **newlandtr," for now as not used here and tripping compiler. Put it back in once actually need it
void ReInitDietData(MSEBoxModel *bm, int b);

double Harvest_How_Much_Fishery_Access(MSEBoxModel *bm, int species, int cohort, int nf, double Wgt);

void Check_Contam_Totals(MSEBoxModel *bm);

/* Box and layer accounting prototypes */
int diagIndex(DiagInfo *dinfo, int n, char *name);
int epiIndex(EpiInfo *einfo, int n, char *name);
int landIndex(EpiInfo *landinfo, int n, char *name);

int fstatIndex(FstatInfo *finfo, int n, char *name);
int ext_trIndex(void *dummy, char *name);
int ext_xyztoijk(void *dummy, double x, double y, double z, int *i, int *j, int *k);
int sedzbtok(double z, Box *bp);
int trIndex(TracerInfo *tinfo, int n, char *name);
int wczbtok(double z, Box *bp);
int xytobox(double x, double y, MSEBoxModel *bm);
void boundaries(MSEBoxModel *bm, double ***newwctr, double ***newsedtr, double ***newicetr, double **newlandtr, FILE *llogfp);
void dz_from_volume(MSEBoxModel *bm);
void nz_from_nomdz(MSEBoxModel *bm);
void layer_coords(MSEBoxModel *bm, FILE *llogfp);
void sed_layer_coords(SedModel *sm, FILE *llogfp);

void ice_layer_coords(Box *bp, IceModel *ice);

void setIndexBM(MSEBoxModel *bm);
void volume_from_dz(MSEBoxModel *bm);

/* Physical routine prototypes */
double bioprofile(MSEBoxModel *bm, double z, double scale);
void deposit(MSEBoxModel *bm, Box *bp, double *m, double **newwc, double **newsed, FILE *llogfp);
void sedprops(MSEBoxModel *bm);
void transfer_sed(SedModel *sm, int src_k, int dst_k, double a);
void transfer_ice(IceModel *ice, int src_k, int dst_k, double a, int change_type);

void vertgeom(MSEBoxModel *bm, FILE *llogfp);

//void ConservationMatterCheck(MSEBoxModel *bm, int spotid);
//void NewMatterCheck(MSEBoxModel *bm, double ***newwctr, double ***newsedtr, int spotid);

/* Warning and testing prototypes */
void check_vals(MSEBoxModel *bm, double ***val, int nz, char *msg);

/* Biology related prototypes */
double Ecology_Get_Size(MSEBoxModel *bm, int species, double Wgt, int cohort);
void Determine_Stock_Prop(MSEBoxModel *bm, int sp, int maxstock_id, int who_call, double *remnant, FILE *llogfp);
void External_Box_Ecology(MSEBoxModel *bm, int b, double dt, FILE *llogfp);

/* Migration related prototypes */
void Init_full_move_array(MSEBoxModel *bm, PhyPropertyData *propInput, int tracerIndex);
void init_forceMoveEntries(MSEBoxModel *bm, FILE *fp);
void open_move_prop(MSEBoxModel *bm, PhyPropertyData *propInput);
void Ecology_Update_Move_Entry(MSEBoxModel *bm, FILE *llogfp);

/* Fisheries related prototypes */
void setSPid(MSEBoxModel *bm);
void Harvest_Read_Time_Series(char *folderPath, char *name, char *tsname, char *t_units, FisheryTimeSeries **ts, int *nts, int *typets);
void Harvest_Free_Time_Series(FisheryTimeSeries *fts, int nnts);

/* Assessment related prototypes */
// Used to be in atsample.h but put here as also called from atManageTier
// (for the downhill_simplex component of that code)
double Assess_Add_Error(MSEBoxModel *bm, int er_case, double true_val, double a, double v);
double Funk(MSEBoxModel *bm, int funkflag, int sp, int nchrt, int ndim, int YrMax, double prm_sp, double *X, FILE *ofp, double *xpar);
double EquilF_Funk(MSEBoxModel *bm, int species, double *xpar, FILE *llogfp);
double SurplusProduction(MSEBoxModel *bm, int species, double *xpar, FILE *llogfp);

void Amoeba(MSEBoxModel *bm, int assessing, int sp, double dayt, char* speciesname, double **p, double *y, int ndim, double ftol, int itmax, int funkflag, int nchrt, int YrMax,
		double prm_sp, int *nfunk, int *ilow, FILE *ofp, double *xpar);
void powell(MSEBoxModel *bm, int sp, double *xf, double **xunit, int npar, double ftol, int *iter, double *ss, int funkflag);


/* Economics related prototypes */
void Quicksort_Dir(double *x, double *bbx, double *ccx, double *ddx, double *eex, int n, int ascendflag);

/* Log file definition */
FILE * initLogFile(MSEBoxModel *bm);
FILE *checkLogFileSize(FILE *llogfp, MSEBoxModel *bm);

/* Halt file creation */
void initHaltFile(MSEBoxModel *bm);

/* Output text file call routines */
void Textfile_Dump(MSEBoxModel *bm, FILE *llogfp);

/* R or sampling related - for management */
FILE * initMetierFile(MSEBoxModel *bm);
FILE * initDynTier4File(MSEBoxModel *bm);
FILE * initDynTier4CTLFile(MSEBoxModel *bm);

#ifdef RASSESS_LINK_ENABLED
/* RAssess related */
void RRAssess_Linkage_Start(MSEBoxModel *bm);
void Do_RAssess(MSEBoxModel *bm, int species, int year, FILE *llogfp);
int freeRRAssess();

/* Redus management related prototypes */
void Redus_Linkage_Start(MSEBoxModel *bm);
int freeRedus();
void REDUS_management(MSEBoxModel *bm, FILE *llogfp);
void RAssessSurvey(MSEBoxModel *bm, FILE *llogfp);
#endif

void CKsimulator(MSEBoxModel *bm, int sp, int year);
void GenData(MSEBoxModel *bm, int groupIndex, int yearIndex);
void WriteSS330Files(MSEBoxModel *bm, int sp, int maxyr, char *baseFolder, char *fileName);
void WriteSSCtl(MSEBoxModel *bm, FILE *fid, int sp, int maxyr);
void WriteSSFor(MSEBoxModel *bm, FILE *fid, int sp, int maxyr);

double Get_Length(MSEBoxModel *bm, double w, int i);
double Selectivity(MSEBoxModel *bm, double li, int i, double av, int err, FILE *ofp);

double Read_SS3_Par_File(MSEBoxModel *bm, int sp, int year, char *folderName);
double Read_SS3_Report_File(MSEBoxModel *bm, int species, int year, char *folderName);

/* Memory freeing routines (for shutdown) */
void freeBMDiagInfo(MSEBoxModel *bm);
void freeBMEpiInfo(MSEBoxModel *bm);
void freeBMFisheriesInfo(MSEBoxModel *bm);
void freeBMphysInfo(MSEBoxModel *bm);
void freeBMTracerInfo(MSEBoxModel *bm);
void freeBMIceInfo(MSEBoxModel *bm);
void freeBMLandInfo(MSEBoxModel *bm);

void modelshutdown(MSEBoxModel *bm);
int runNextTimeStep(MSEBoxModel *bm);

