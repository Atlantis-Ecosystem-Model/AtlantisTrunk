/**
 *
 *
 *
 *
 *
 *  Changes:
 *
 *  31-03-2009 Bec Gorton
 *
 *  Added the doesNodeExist function to replace calls made to getNodeValue that were just used to check if a node
 *  with the given xpath exists in the dom. doesNodeExist will return true is the given xpath exits in the dom. getNodeValue
 *  was leaking memory so this was changed to no longer return a pointer to a string - instead a destination string is passed
 *  to the function as a parameter. This means the call to Execute_Xpath_Expression can be cleaned up by calling xmlXPathFreeObject to
 *  free up any memory allocated.
 *
 *  28-04-2009 Bec Gorton
 *  Moved the readXMLArray function into this file from the atecology code. This is a useful generic function that could be used
 *  by other modules to load xml data.
 *
 *  02-05-209 Bec Gorton
 *  Added the more generic XMl functions.
 *
 *  11-05-2009 Bec Gorton
 *  Added the Util_XML_Replace_Node_Value function and got rid of the append function.
 *
 *  07-07-2009 Bec Gorton
 *  Changed the AtlantisXMLObjectNAMES to include information about the fishery nodes.
 *  Also cleanup some of the generic XML functions - these were dying if the length of the comment got too long so i have allowed extra space.
 *  I have moved the code that concats the comment and the units into  addCommentNode only - this was happening all over the place.
 *
 *  09-07-2009 Bec Gorton
 *  Added the AtlantisXMLAttributeTypeStrings array to store information about the possible attribute types.
 *
 *  14-07-2009 Bec Gorton
 *  Fixed the addCommentNode code to not concat the comment and unit string into a single string.
 *
 *  28-10-2009 Bec Gorton
 *  Changed the AtlantisXMLObjectNAMES values and fixed memory issues highlighted by valgrind.
 *
 *	18-01-2010 Bec Gorton
 *	Changed the Parse_Into_Array function to allow for tab seperated arrays. This was causing a problem with the NSW input files
 *	which had many tab seperated fields.
 *
 *	29-01-2010 Bec Gorton
 *	Moved the definition of the speciesParamStructArray array from the biology library into this file.
 *	Also moved the functions Util_XML_Get_Param_Index() and Util_XML_Get_Species_Node_Value() into here.
 *
 *	11-01-2011 Bec Gorton
 *	Removed doesNodeExist and updated function names.
 *
 *	02-08-2012 Bec Gorton
 *  Added the Util_XML_Read_Array_String function so we can read in the list of additional tracers.
 *
 *  23-04-2013 Bec Gorton
 *	Changed the overwinter species params to only be read in for group that are over wintering.
 */
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <signal.h>
#include <math.h>
#include <ctype.h>
#include <time.h>
#include <string.h>
#include <sjwlib.h>
#include <netcdf.h>
#include <assert.h>
#include <atlantisboxmodel.h>
#include <atXMLUtil.h>
#include <atUtilLib.h>

static char *Replace_String_Entry(char *valueStr, char *replaceStr, int size, int replaceIndex);

SpeciesParamStruct speciesParamStructArray[tot_prms] =
	{

        /** Functional Group Flags */
		{ "Group_Functioning_Flag", flag_id, "^flag(j[A-Z]{3}|[A-Z]{2,3})$", binary_check, 1.0, SP_TURNED_ON },
		{ "Preferred_Location_Trend_Flag", flagdem_id, "^flagdem", binary_check, 1.0, SP_NOT_DET },
		{"Flag_Planktivore", flagplankfish_id, "^flagplankfish", binary_check, 1.0, SP_VERTS },
		{ "Flux_Threshhold", flux_thresh_id, "[A-Z]{2,3}thresh$", no_checking, 1.0, SP_SED_BACT },
		{ "Flux_Damp", flux_damp_id, "damp$", no_checking, 1.0, SP_SED_BACT },

		/** Reproduction parameters */
		{ "Bearlive_Flag", flagbearlive_id, "^flagbearlive", binary_check, 1.0, SP_VERTS },
		{ "Feed_While_Spawn_Flag", feed_while_spawn_id, "^feed_while_spawn", binary_check, 1.0, SP_AGE_STRUCTURED },
		{ "Provided_Parental_Care", flagmother_id, "^flagmother", integer_check, 1.0, SP_VERTS },
		{ "Recruitment_Function", flagrecruit_id, "^flagrecruit", integer_check, 1.0, SP_AGE_STRUCTURED },
		{ "FLAG_RECPEAK", flagrecpeak_id, "^flagrecpeak", integer_check, 1.0, SP_VERTS },
		//{ "FLAG_LOCALRECRUIT", flaglocalrecruit_id, "^flaglocalrecruit", integer_check, 1.0, SP_AGE_STRUCTURED },
		{ "FLAG_Stocking", flagstocking_id, "^flagstocking", binary_check, 1.0, SP_AGE_STRUCTURED },
        { "FLAG_PlusGroup", flagkeep_plusgroup_id, "^flagkeep_plusgroup", binary_check, 1.0, SP_AGE_STRUCTURED },
		{ "Recruitment_Function_PERIOD", Recruit_Period_id, "^Recruit_Period", integer_check, 1.0, SP_AGE_STRUCTURED },
		{ "Recruitment_Function_TIME", Recruit_Time_id, "_Recruit_Time$", integer_check, 1.0, SP_AGE_STRUCTURED },
        { "Recruitment_Entry", cohort_recruit_entry_id, "_cohort_recruit_entry$", integer_check, 1.0, SP_AGE_STRUCTURED },
		{ "FLAG_SPAWN_PERIOD", spawn_period_id, "_spawn_period$", integer_check, 1.0, SP_AGE_STRUCTURED },
		{ "FLAG_KSPA", KSPA_id, "^KSPA_", no_checking, 1.0, SP_VERTS },
		{ "FLAG_FSP", FSP_id, "^FSP_[A-Z]{2,3}$", no_checking, 1.0, SP_AGE_STRUCTURED },

		/** Habitat parameters */
		{ "FLAG_KCOV_JUV", Kcov_juv_id, "^Kcov_juv_", no_checking, 1.0, SP_VERTS },
		{ "FLAG_BCOV_JUV", Bcov_juv_id, "^Bcov_juv_", no_checking, 1.0, SP_VERTS },
		{ "FLAG_ACOV_JUV", Acov_juv_id, "^Acov_juv_", no_checking, 1.0, SP_VERTS },
		{ "FLAG_KCOV_AD", Kcov_ad_id, "^Kcov_ad_", no_checking, 1.0, SP_VERTS },
		{ "FLAG_BCOV_AD", Bcov_ad_id, "^Bcov_ad_", no_checking, 1.0, SP_VERTS },
		{ "FLAG_ACOV_AD", Acov_ad_id, "^Acov_ad_", no_checking, 1.0, SP_VERTS },
		{ "FLAG_RUG_SCALAR", RugCover_scalar_id, "RugCover_scalar$", no_checking, 1.0, SP_VERTS },

		/** Recruitment paramters */
		{ "FLAG_AGE_MAT", age_mat_id, "_age_mat$", integer_check, 1.0, SP_VERTS },
		{ "FLAG_RECOVERSTART", recover_start_id, "^recover_start", integer_check, 1.0, SP_VERTS },
		{ "FLAG_KWSR", KWSR_id, "^KWSR_", no_checking, 1.0, SP_VERTS },
		{ "FLAG_KWRR", KWRR_id, "^KWRR_", no_checking, 1.0, SP_VERTS },
		{ "FLAG_RECOVER_MULT", recover_mult_id, "^recover_mult_", no_checking, 1.0, SP_VERTS },
		{ "FLAG_BHBETA", BHbeta_id, "^BHbeta_", no_checking, 1.0, SP_AGE_STRUCTURED },
		{ "FLAG_BHALPHA", BHalpha_id, "^BHalpha_", no_checking, 1.0, SP_AGE_STRUCTURED },
		{ "FLAG_RBETA", Rbeta_id, "^Rbeta_", no_checking, 1.0, SP_AGE_STRUCTURED },
		{ "FLAG_RALPHA", Ralpha_id, "^Ralpha_", no_checking, 1.0, SP_AGE_STRUCTURED },
		{ "FLAG_PP", PP_id, "^PP_", no_checking, 1.0, SP_AGE_STRUCTURED },
		{ "FLAG_LOG_MULT", log_mult_id, "_log_mult$", no_checking, 1.0, SP_AGE_STRUCTURED },
        { "FLAG_NORM_SIGMA", norm_sigma_id, "_norm_sigma$", no_checking, 1.0, SP_AGE_STRUCTURED },
        { "FLAG_STOCASTIC_REC", flag_recruit_stochastic_id, "_flag_recruit_stochastic$", binary_check, 1.0, SP_AGE_STRUCTURED },
        
        { "FLAG_PROD_ALPHA", prod_alpha_id, "^prod_alpha_", no_checking, 1.0, SP_AGE_STRUCTURED },
        { "FLAG_DDEPEND_BETA1", den_depend_beta1_id, "^den_depend_beta1_", no_checking, 1.0, SP_AGE_STRUCTURED },
        { "FLAG_DDEPEND_BETA2", den_depend_beta2_id, "^den_depend_beta2_", no_checking, 1.0, SP_AGE_STRUCTURED },
        { "FLAG_TEMP_COEFFT", temp_coefft_id, "^temp_coefft_", no_checking, 1.0, SP_AGE_STRUCTURED },
        { "FLAG_RATE_COEFFT", rate_coefft_id, "^rate_coefft_", no_checking, 1.0, SP_AGE_STRUCTURED },
        { "FLAG_WIND_COEFFT", wind_coefft_id, "^wind_coefft_", no_checking, 1.0, SP_AGE_STRUCTURED },
        { "FLAG_REC_VAR", recruit_var_id, "^rec_var_", no_checking, 1.0, SP_AGE_STRUCTURED },
        
		{ "FLAG_MIN_SPAWN_TEMP", min_spawn_temp_id, "_min_spawn_temp$", no_checking, 1.0, SP_AGE_STRUCTURED },
		{ "FLAG_MAX_SPAWN_TEMP", max_spawn_temp_id, "_max_spawn_temp$", no_checking, 1.0, SP_AGE_STRUCTURED },
		{ "FLAG_MIN_SPAWN_SALT", min_spawn_salt_id, "_min_spawn_salt$", no_checking, 1.0, SP_AGE_STRUCTURED },
		{ "FLAG_MAX_SPAWN_SALT", max_spawn_salt_id, "_max_spawn_salt$", no_checking, 1.0, SP_AGE_STRUCTURED },
        
        { "FLAG_PROP_SPAWNLOST", prop_spawn_lost_id, "^prop_spawn_lost_", no_checking, 1.0, SP_AGE_STRUCTURED_BIOMASS },
		{ "FLAG_JACKA", jack_a_id, "^jack_a_", no_checking, 1.0, SP_AGE_STRUCTURED },
		{ "FLAG_JACKB", jack_b_id, "^jack_b_", no_checking, 1.0, SP_AGE_STRUCTURED },
		{ "FLAG_SPAWN_RECHABDEP", rec_HabDepend_id, "rec_HabDepend$", no_checking, 1.0, SP_AGE_STRUCTURED },
        { "FLAG_INTERSP", intersp_depend_recruit_id, "^intersp_depend_recruit_", integer_check, 1.0, SP_AGE_STRUCTURED },
		{ "FLAG_INTERSPID", intersp_depend_sp_id, "^intersp_depend_sp_", integer_check, 1.0, SP_AGE_STRUCTURED },
		{ "FLAG_INTERSPK", intersp_depend_scale_id, "^intersp_depend_scale_", no_checking, 1.0, SP_AGE_STRUCTURED },
        
        { "FLAG_AQUACULTFRY", aquacult_fry_id, "_aquacult_fry$", no_checking, 1.0, SP_CULTURED },
		{ "FLAG_KA", KA_id, "^KA_", no_checking, 1.0, SP_VERTS },
		{ "FLAG_KB", KB_id, "^KB_", no_checking, 1.0, SP_VERTS },

		/** Overwintering and Encystment */
		{ "FLAG_OWSTofY", overwinterStartTofY_id, "^overwinterStartTofY_", no_checking, 1.0, SP_OVERWINTER },
		{ "FLAG_OWETofY", overwinterEndTofY_id, "^overwinterEndTofY_", no_checking, 1.0, SP_OVERWINTER },
		{ "FLAG_OWSTemp", overwinterStartTemp_id, "^overwinterStartTemp_", no_checking, 1.0, SP_OVERWINTER },
		{ "FLAG_OWETemp", overwinterEndTemp_id, "^overwinterEndTemp_", no_checking, 1.0, SP_OVERWINTER },
		{ "FLAG_OWcritmum", crit_mum_id, "^crit_mum_", no_checking, 1.0, SP_OVERWINTER },
		{ "FLAG_OWcritnut", crit_nut_id, "^crit_nut_", no_checking, 1.0, SP_OVERWINTER },
		{ "FLAG_OWcrittemp", crit_temp_id, "^crit_temp_", no_checking, 1.0, SP_OVERWINTER },
		{ "FLAG_OWcystrate", encyst_rate_id, "^encyst_rate_", no_checking, 1.0, SP_OVERWINTER },
		{ "FLAG_OWhatchrate", hatch_rate_id, "^hatch_rate_", no_checking, 1.0, SP_OVERWINTER },
		{ "FLAG_OWcystperiod", encyst_period_id, "^encyst_period_", no_checking, 1.0, SP_OVERWINTER },
		{ "FLAG_OWflagencyst", flagencyst_id, "^flagencyst_", no_checking, 1.0, SP_OVERWINTER},

		/** Evolution parameters - TODO: Generalise so all groups can evolve */
		{ "FLAG_EVOLmaxpropshift", max_prop_shift_id, "^max_prop_shift_", no_checking, 1.0, SP_AGE_STRUCTURED},
		{ "FLAG_EVOLinheritance", inheritance_id, "^inheritance_", no_checking, 1.0, SP_AGE_STRUCTURED},
		{ "FLAG_EVOLtraitvar", trait_variance_id, "^trait_variance_", no_checking, 1.0, SP_AGE_STRUCTURED},
		{ "FLAG_EVOLmintraitvar", min_trait_variance_id, "^min_trait_variance_", no_checking, 1.0, SP_AGE_STRUCTURED},

		/** Detritus_Conversion parameters */
		{ "FLAG_FDM", FDMort_id, "^FDM_([A-Z]{2,3}$)", no_checking, 1.0, SP_NOT_PP },
		{ "FLAG_FDG", FDG_id, "^FDG_([A-Z]{2,3})$", no_checking, 1.0, SP_PREDATOR },
		{ "FLAG_FDGDL", FDGDL_id, "^FDGDL_", no_checking, 1.0, SP_PREDATOR },
		{ "FLAG_FDGDR", FDGDR_id, "^FDGDR_", no_checking, 1.0, SP_PREDATOR },

		/**Diet parameters */
		{ "FLAG_CATCHEATE", catcheater_id, "_catcheater$", integer_check, 1.0, SP_VERTS },
		{ "FLAG_ACTIVE", flagactive_id, "day$", integer_check, 1.0, SP_PREDATOR },
		{ "FLAG_VLA_T15", vla_T15_id, "vla_[A-Z]{2,3}_T15$", no_checking, 1.0, SP_VERTS },
		{ "FLAG_KL", KL_id, "^KL_([A-Z]{2,3})$", no_checking, 1.0, SP_PREDATOR },
		{ "FLAG_KU", KU_id, "^KU_([A-Z]{2,3})$", no_checking, 1.0, SP_PREDATOR },
		{ "FLAG_KUP", KUP_id, "^KUP_([A-Z]{2,3})$", no_checking, 1.0, SP_PREDATOR },
		{ "FLAG_KLP", KLP_id, "^KLP_([A-Z]{2,3})$", no_checking, 1.0, SP_PREDATOR },
		{ "FLAG_KMAX_COEFFT", Kmax_coefft_id, "^Kmax_coefft_([A-Z]{2,3})$", no_checking, 1.0, SP_PREDATOR },
		{ "FLAG_KDEP", KDEP_id, "^KDEP_([A-Z]{2,3})$", no_checking, 1.0, SP_PREDATOR }, //Sediment penetration depth
		{ "FLAG_VLB", vlb_id, "^vlb_", no_checking, 1.0, SP_VERTS },
		{ "FLAG_HTA", hta_id, "^hta_", no_checking, 1.0, SP_VERTS },
		{ "FLAG_HTB", htb_id, "^htb_", no_checking, 1.0, SP_VERTS },
        { "FLAG_PR", pR_id, "^pR_", no_checking, 1.0, SP_VERTS },
		{ "FLAG_LI_A", li_a_id, "^li_a_", no_checking, 1.0, SP_VERTS },
		{ "FLAG_LI_B", li_b_id, "^li_b_", no_checking, 1.0, SP_VERTS },
        { "FLAG_LINF", linf_id, "^linf_", no_checking, 1.0, SP_VERTS },
        { "FLAG_KBERT", Kbert_id, "^Kbert_", no_checking, 1.0, SP_VERTS },
        { "FLAG_TZERO", tzero_id, "^tzero_", no_checking, 1.0, SP_VERTS },
        
        { "FLAG_MIN_LI_MAT", min_li_mat_id, "^min_li_mat_", no_checking, 1.0, SP_VERTS },
		{ "FLAG_PREDCASE", predcase_id, "^predcase", integer_check, 1.0, SP_PREDATOR },
		{ "FLAG_COHORT_PREY_AVAIL", age_structured_prey_id, "^age_structured_prey_", binary_check, 1.0, SP_VERTS },
		{ "FLAG_P_SPLIT", p_split_id, "^p_split_", binary_check, 1.0, SP_AGE_DIET },

        { "FLAG_EXTRAFEED", extra_feed_id, "_extra_feed$", proportion_check, 1.0, SP_FED },
        
        { "FLAG_SEARCH_VOLUME", vl_id, "^vl_", no_checking, 86400.0, SP_INVERT_PREDATORS },
		{ "FLAG_HANDOVER_TIME", ht_id, "^ht_", no_checking, 1.0, SP_INVERT_PREDATORS },
        { "FLAG_HVM", hvm_id, "^hvm_", no_checking, 1.0, SP_PREDATOR },

        { "FLAG_TURBID_REFUGE", turbid_refuge_id, "^turbid_refuge_", no_checking, 1.0, SP_NOT_DET },
        
        /* Dynamic RN/SN ratio related parameters */
        { "FLAG_RSMAX", RSmax_id, "^RSmax_", no_checking, 1.0, SP_VERTS },
        { "FLAG_RSMID", RSmid_id, "^RSmid_", no_checking, 1.0, SP_VERTS },
        { "FLAG_RSSLOPE", RSslope_id, "^RSslope_", no_checking, 1.0, SP_VERTS },
        { "FLAG_RSPROP", RSprop_id, "^RSprop_", no_checking, 1.0, SP_VERTS },
        { "FLAG_SNCOST", SNcost_id, "^SNcost_", no_checking, 1.0, SP_VERTS },
        { "FLAG_RNCOST", RNcost_id, "^RNcost_", no_checking, 1.0, SP_VERTS },
        { "FLAG_RSSTARVE", RSstarve_id, "^RSstarve_", no_checking, 1.0, SP_VERTS },
        
		/* Assimilation_Efficiency Diet parameters */
		{ "FLAG_E", E1_id, "^E_([A-Z]{2,3})$", no_checking, 1.0, SP_PREDATOR },
		{ "FLAG_EPLANT", E2_id, "^EPlant_", no_checking, 1.0, SP_PREDATOR },
		{ "FLAG_EDL", E3_id, "^EDL_([A-Z]{2,3})$", no_checking, 1.0, SP_PREDATOR_OR_BACT },
		{ "FLAG_EDR", E4_id, "^EDR_([A-Z]{2,3})$", no_checking, 1.0, SP_PREDATOR_OR_BACT },
		{ "FLAG_Q10EFF", flagq10eff_id, "^flagq10eff", integer_check, 1.0, SP_VERTS },
        { "FLAG_Q10RECEFF", flagq10receff_id, "^flagq10receff", integer_check, 1.0, SP_VERTS },

		/* Temperature related parameters */
		{ "FLAG_Q10", q10_id, "^q10_([A-Z]{2,3})$", no_checking, 1.0, SP_ALL },
		{ "FLAG_Q10_METHOD", q10_method_id, "^q10_method_", integer_check, 1.0, SP_NOT_DET },
		{ "FLAG_Q10_OPTIMAL_TEMP", q10_optimal_temp_id, "^q10_optimal_temp_", no_checking, 1.0, SP_Q10 },
		{ "FLAG_Q10_CORRECTION", q10_correction_id, "^q10_correction_", no_checking, 1.0, SP_Q10 },

		{ "FLAG_TEMP_COEFFTA", temp_coefftA_id, "^temp_coefftA_", no_checking, 1.0, SP_NOT_DET },

		{ "FLAG_TEMP_SENSITIVE", flagtempsensitive_id, "^flagtempsensitive", integer_check, 1.0, SP_NOT_DET },
		{ "FLAG_FECUND_SENSITIVE", flagfecundsensitive_id, "^flagfecundsensitive", integer_check, 1.0, SP_AGE_STRUCTURED },

		/* Salinity related parameters */
		{ "FLAG_SALT_SENSITIVE", flagSaltSensitive_id, "^flagSaltSensitive", integer_check, 1.0, SP_NOT_DET },
		{ "FLAG_SALT_CORRECTION", salt_correction_id, "^salt_correction_", no_checking, 1.0, SP_NOT_DET },

		/* pH related parameters */
		{ "FLAG_PH_SENSITIVE", flagpHsensitive_id, "^flagpHsensitive", binary_check, 1.0, SP_NOT_DET },
		{ "FLAG_PH_SENSITIVEMODEL", pHsensitive_model_id, "^pHsensitive_model_", integer_check, 1.0, SP_NOT_DET },
		{ "FLAG_PH_CONSTA", pH_constA_id, "^pH_constA_", no_checking, 1.0, SP_NOT_DET },
		{ "FLAG_PH_CONSTB", pH_constB_id, "^pH_constB_", no_checking, 1.0, SP_NOT_DET },
        { "FLAG_PH_CONSTC", pH_constC_id, "^pH_constC_", no_checking, 1.0, SP_NOT_DET },
        { "FLAG_PH_MIN", min_pH_id, "^min_pH_", no_checking, 1.0, SP_NOT_DET },
		{ "FLAG_PH_MAX", max_pH_id, "^max_pH_", no_checking, 1.0, SP_NOT_DET },
		{ "FLAG_PH_KN", KN_pH_id, "KN_pH_", no_checking, 1.0, SP_NOT_DET },
		{ "FLAG_PH_OPTIMAL", optimal_pH_id, "^optimal_pH_", no_checking, 1.0, SP_NOT_DET },
		{ "FLAG_PH_CORRECTION", pH_correction_id, "^pH_correction_", no_checking, 1.0, SP_NOT_DET },
		{ "FLAG_PH_CONTRACT_TOL", contract_tol_id, "^contract_tol_", no_checking, 1.0, SP_NOT_DET },
		{ "FLAG_FLAGCONTRACT_TOL", flagcontract_tol_id, "^flagcontract_tol_", no_checking, 1.0, SP_NOT_DET },
		{ "FLAG_PH_TROPHSENSITIVE", flagpredavaileffect_id, "^flagpredavaileffect", binary_check, 1.0, SP_NOT_DET },
		{ "FLAG_PH_NUTVAL", flagnutvaleffect_id, "^flagnutvaleffect", binary_check, 1.0, SP_NOT_DET },
        { "FLAG_PH_MORTSTART", pHmortstart_id, "^pHmortstart_", no_checking, 1.0, SP_NOT_DET },
        { "FLAG_PH_MORTA", pHmortA_id, "^pHmortA_", no_checking, 1.0, SP_NOT_DET },
        { "FLAG_PH_MORTB", pHmortB_id, "^pHmortB_", no_checking, 1.0, SP_NOT_DET },
        { "FLAG_PH_MORTMID", pHmortmid_id, "^pHmortmid_", no_checking, 1.0, SP_NOT_DET },

        /* light and noise pollution related */
        { "FLAG_LIGHT_COEFFT", light_coefft_id, "^light_coefft_", no_checking, 1.0, SP_POLLUTE_IMPACTED },
        { "FLAG_NOISE_COEFFT", noise_coefft_id, "^noise_coefft_", no_checking, 1.0, SP_POLLUTE_IMPACTED },

		/* Primary producer values */
		{ "FLAG_KTUR", KTUR_id, "^KTUR_", no_checking, 1.0, SP_BIOTURB },
		{ "FLAG_KIRR", KIRR_id, "^KIRR_", no_checking, 1.0, SP_INF },
		{ "FLAG_DIN_HALF_SAT", KN_id, "^KN_", no_checking, 1.0, SP_KN },
		{ "FLAG_SI_HALF_SAT", KS_id, "^KS_", no_checking, 1.0, SP_PP },
		{ "FLAG_MICRONUTRIENT_HALF_SAT", KF_id, "^KF_", no_checking, 1.0, SP_PP },
		{ "FLAG_LIM", flag_lim_id, "^flag.*lim$", integer_check, 1.0, SP_POP_LIM },
		{ "FLAG_KI_T15", KI_T15_id, "^KI_.*_T15$", no_checking, 1.0, SP_PP },
		{ "FLAG_KI_L_T15", KI_L_T15_id, "^L_KI_.*_T15", no_checking, 1.0, SP_SG },
		{ "FLAG_Kext_SG", Kext_id, "^Kext_", no_checking, 1.0, SP_SG },
		{ "FLAG_Ksub_SG", Ksub_id, "^Ksub_", no_checking, 1.0, SP_SG },
		{ "FLAG_KNEPI_SG", KNepi_id, "^KN_epi_", no_checking, 1.0, SP_SG },
		{ "FLAG_BETAD", Beta_D_id, "^Beta_D_", no_checking, 1.0, SP_PP },
		{ "FLAG_PBMAX", PBmax_D_id, "^PBmax_D_", no_checking, 1.0, SP_PP },

		{ "FLAG_KsubEPI_SG", KsubEpi_id, "^KsubEpi_", no_checking, 1.0, SP_SG },
		{ "FLAG_Ktrans_SG", Ktrans_id, "^Ktrans_", no_checking, 1.0, SP_SG },

		/* primary producer phosphorus values */
		{ "FLAG_P_UPTAKE", P_max_uptake_id, "^P_uptake_.*", no_checking, 1.0, SP_PP },	//max phosphorus uptake rate per day
		{ "FLAG_P_SCALE_UPTAKE", P_uptake_scale_id, "^P_scale_uptake_.*", no_checking, 1.0, SP_PP },	//affinity coefficient for phosphorus uptake mechanism
		{ "FLAG_P_CONCP", P_concp_id, "^P_concp_.*", no_checking, 1.0, SP_PP },	//internal concentration factor
		{ "FLAG_P_MIN", P_min_internal_id, "^P_min_internal_.*", no_checking, 1.0, SP_PP },	// min internal phosphorus concentrations
		{ "FLAG_P_MAX", P_max_internal_id, "^P_max_internal_.*", no_checking, 1.0, SP_PP },	// max internal phosphorus concentrations


		{ "FLAG_C_UPTAKE", C_max_uptake_id, "^C_uptake_.*", no_checking, 86400.0, SP_PP },	//max carbon uptake rate per day

		{ "FLAG_C_SCALE_UPTAKE", C_uptake_scale_id, "^C_scale_uptake_.*", no_checking, 1.0, SP_PP },	//affinity coefficient for carbon uptake mechanism
		{ "FLAG_C_CONCP", C_concp_id, "^C_concp_.*", no_checking, 1.0, SP_PP },	//internal concentration factor


		{ "FLAG_PSA_MIN", Psa_min_id, "^PSA_min_.*", no_checking, 1.0, SP_PP },	//Min quota of phosphorus in phytoplankton cells in moles/mg dry weight
		{ "FLAG_C_MIN", C_min_id, "^C_min_.*", no_checking, 1.0, SP_PP },	//Min quota of phosphorus in phytoplankton cells in moles/mg dry weight
		{ "FLAG_PHYTO_RESP_RATE", phyto_resp_rate_id, "^Phyto_Resp_Rate_.*", no_checking, 86400.0, SP_PP },	//Phytoplankton respiration rate
		{ "FLAG_P_HALF_SAT", KP_id, "^KP_", no_checking, 1.0, SP_PP },

		/** Mortality parameters */
		{ "FLAG_LYSIS_RATE", KLYS_id, "^KLYS_", proportion_check, 86400.0, SP_MICROALGAE },
		{ "FLAG_FSBDR", FSBDR_id, "^FSBDR_", no_checking, 1.0, SP_VERTS },
		{ "FLAG_mS_T15", mS_T15_id, "^mS_.*_T15$", proportion_check, 86400.0, SP_MACROPHYTE },
		{ "FLAG_MSTARVE", mStarve_id, "^mStarve_", no_checking, 86400.0, SP_VERTS },
		{ "FLAG_MT", mT_id, "^mT_", no_checking, 1.0, SP_AGE_STRUCTURED_BIOMASS },

		/** Oxygen_Mortality parameters */
		{ "FLAG_DEPTH_OXYGEN_MORTALITY", mD_id, "^mD_", proportion_check, 86400.0, SP_OXYGEN },
		{ "FLAG_OXYGEN_DEP_MORTALITY", mO_id, "^mO_", proportion_check, 86400.0, SP_OXYGEN },
		{ "FLAG_LETHAL_OXYGEN", KO2_id, "^KO2_", no_checking, 1.0, SP_OXYGEN },
		{ "FLAG_LIMITING_OXYGEN", KO2LIM_id, "^KO2LIM_", no_checking, 1.0, SP_OXYGEN },

		/** Spatial_Threshold values */
		{ "FLAG_LOW", low_id, "_low$", no_checking, 1.0, SP_SED_EP_FF },
		{ "FLAG_MAX", max_id, "[A-Z]{2,3}max$", no_checking, 1.0, SP_BASAL },
		{ "FLAG_SAT", sat_id, "_sat$", no_checking, 1.0, SP_SED_EP_FF },
		{ "FLAG_THRESH", thresh_id, "[A-Z]{2,3}thresh$", no_checking, 1.0, SP_SED_EP_FF },

		/** Movement values */
		{ "FLAG_DEPEND_MOVE", ddepend_move_id, "_ddepend_move$", integer_check, 1.0, SP_HORIZONATAL_MOVE },
		{ "FLAG_MAX_MOVE_TEMP", max_move_temp_id, "_max_move_temp$", no_checking, 1.0, SP_NOT_DET },
		{ "FLAG_MIN_MOVE_TEMP", min_move_temp_id, "_min_move_temp$", no_checking, 1.0, SP_NOT_DET },
		{ "FLAG_MAX_MOVE_SALT", max_move_salt_id, "_max_move_salt$", no_checking, 1.0, SP_NOT_DET },
		{ "FLAG_MIN_MOVE_SALT", min_move_salt_id, "_min_move_salt$", no_checking, 1.0, SP_NOT_DET },
        
        { "FLAG_K_MOVE_TEMP_CONST", K_temp_const_id, "_K_temp_const$", no_checking, 1.0, SP_NOT_DET },
        { "FLAG_K_MOVE_SALT_CONST", K_salt_const_id, "_K_salt_const$", no_checking, 1.0, SP_NOT_DET },
		{ "FLAG_SPEED", Speed_id, "^Speed_", no_checking, 3600.0, SP_VERTS },
		{ "MIN_DEPTH", mindepth_id, "_mindepth$", no_checking, 1.0, SP_NOT_DET },
		{ "MAX_DEPTH", maxdepth_id, "_maxdepth$", no_checking, 1.0, SP_NOT_DET },
        { "MAX_TOTDEPTH", maxtotdepth_id, "_maxtotdepth$", no_checking, 1.0, SP_NOT_DET },
		{ "MIN_O2", min_O2_id, "_min_O2$", no_checking, 1.0, SP_NOT_DET },
        { "FLAG_K_MOVE_O2_CONST", K_o2_const_id, "_K_o2_const$", no_checking, 1.0, SP_NOT_DET },

		/** Home range values */
		{ "FLAG_CHANNEL", flagchannel_id, "", binary_check, 1.0, SP_VERTS }, //TODO This is never used in this code. fix.
		{ "HOME_RANGE_RADIUS", homerangerad_id, "_homerangerad$", no_checking, 1.0, SP_VERTS_HOME_RANGE },
		{ "HOME_RANGE_OVERLAP", rangeoverlap_id, "_overlap$", binary_check, 1.0, SP_VERTS_HOME_RANGE },

		/** Detritus_Handover parameters */
		{ "K_TRANS", k_trans_id, "k_trans", no_checking, 1.0, SP_SED_EP_FF },

		/** sp_remin_contrib value */
		{"REMIN_CONTRIB", sp_remin_contrib_id, "_remin_contrib$", binary_check, 1.0, SP_INF},

		/** Coral related bleaching parameters */
		{ "FLAG_BLEACH_A", bleach_periodA_id, "_bleach_periodA$", no_checking, 1.0, SP_CORAL },
		{ "FLAG_BLEACH_B", bleach_periodB_id, "_bleach_periodB$", no_checking, 1.0, SP_CORAL },
		{ "FLAG_BLEACH_MORT", mBleach_id, "_mBleach$", no_checking, 86400.0, SP_CORAL },
		{ "FLAG_BLEACH_RATE", bleaching_rate_id, "_bleaching_rate$", no_checking, 86400.0, SP_CORAL },
		{ "FLAG_BLEACH_RECOVER", bleach_recovery_rate_id, "_bleach_recovery_rate$", no_checking, 1.0, SP_CORAL },
		{ "FLAG_BLEACH_TEMPSH", bleach_tempshift_id, "_bleach_tempshift$", no_checking, 1.0, SP_CORAL },
		{ "FLAG_BLEACH_GROW", bleach_growshift_id, "_bleach_growshift$", no_checking, 1.0, SP_CORAL },
		{ "FLAG_BLEACH_TEMP", bleach_temp_id, "_bleach_temp$", no_checking, 1.0, SP_CORAL },
        { "FLAG_MINBLEACH_TEMP", min_bleach_temp_id, "_min_bleach_temp$", no_checking, 1.0, SP_CORAL },
        { "FLAG_PROP_ZOOXANTH", prop_zooxanth_id, "_prop_zooxanth$", no_checking, 1.0, SP_CORAL },
		{ "FLAG_DHW_THRESH", DHW_thresh_id, "_DHW_thresh$", no_checking, 1.0, SP_CORAL },
		{ "FLAG_CDEPTH_THRESH", threshdepth_id, "_threshdepth$", no_checking, 1.0, SP_CORAL },
        { "FLAG_DEPMUM", depmum_scalar_id, "_depmum_scalar$", no_checking, 1.0, SP_CORAL },
        
        { "FLAG_MINBLEACH_SALT", min_bleach_salt_id, "_min_bleach_salt$", no_checking, 1.0, SP_CORAL },
        { "FLAG_MAXBLEACH_SALT", max_bleach_salt_id, "_max_bleach_salt$", no_checking, 1.0, SP_CORAL },
        
        /** Coral related parameters */
		{ "FLAG_HOST_REMIN", HostRemin_id, "_HostRemin$", no_checking, 1.0, SP_CORAL },
		{ "FLAG_CALCIF_REF", calcifRefBaseline_id, "_calcifRefBaseline$", no_checking, 1.0, SP_CORAL },
		{ "FLAG_CALCIF_CONST", calcifTconst_id, "_calcifTconst$", no_checking, 1.0, SP_CORAL },
		{ "FLAG_CALCIF_COEFFT", calcifTcoefft_id, "_calcifTcoefft$", no_checking, 1.0, SP_CORAL },
		{ "FLAG_CALCIF_OPT", calcifTopt_id, "_calcifTopt$", no_checking, 1.0, SP_CORAL },
		{ "FLAG_CALCIF_LAMBDA", calcifLambda_id, "_calcifLambda$", no_checking, 1.0, SP_CORAL },
		{ "FLAG_FEED_LIGHT", FeedLightThresh_id, "_FeedLightThresh$", no_checking, 1.0, SP_CORAL },
		{ "FLAG_FEED_PROP", PropLightFeed_id, "_PropLightFeed$", no_checking, 1.0, SP_CORAL },
		{ "FLAG_CORAL_MAXTRANS", coral_max_accel_trans_id, "_coral_max_accel_trans$", no_checking, 1.0, SP_CORAL },
		{ "FLAG_CORAL_MAXACCELA", coral_max_accelA_id, "_coral_max_accelA$", no_checking, 1.0, SP_CORAL },
		{ "FLAG_CORAL_MAXACCELB", coral_max_accelB_id, "_coral_max_accelB$", no_checking, 1.0, SP_CORAL },
		{ "FLAG_CORAL_RECA", CrecruitA_id, "_CrecruitA$", no_checking, 1.0, SP_CORAL },
		{ "FLAG_CORAL_RECB", CrecruitB_id, "_CrecruitB$", no_checking, 1.0, SP_CORAL },
		{ "FLAG_CORAL_RECC", CrecruitC_id, "_CrecruitC$", no_checking, 1.0, SP_CORAL },

		{ "FLAG_CORAL_OVERGROW", coral_overgrow_id, "_coral_overgrow$", no_checking, 86400.0, SP_MACROPHYTE },
		{ "FLAG_CORAL_COMPETE", coral_compete_id, "_coral_compete$", no_checking, 1.0, SP_MACROPHYTE },
        { "FLAG_SPONGE_OVERGROW", sponge_overgrow_id, "_sponge_overgrow$", no_checking, 86400.0, SP_MACROPHYTE },
        { "FLAG_SPONGE_COMPETE", sponge_compete_id, "_sponge_compete$", no_checking, 1.0, SP_MACROPHYTE },

        /** Sponge related parameters */
        { "FLAG_KSMOTHER_B", Ksmother_B_id, "_Ksmother_B$", no_checking, 86400.0, SP_CORAL },
        { "FLAG_KSMOTHER_A", Ksmother_A_id, "_Ksmother_A$", no_checking, 86400.0, SP_CORAL },
        { "FLAG_VMAX_DELTASI", Vmax_deltaSi_id, "_Vmax_deltaSi$", no_checking, 86400.0, SP_SPONGE },
        { "FLAG_FM_DELTASI", Km_deltaSi_id, "_Km_deltaSi$", no_checking, 86400.0, SP_SPONGE },
        
		/** Rugosity related - from corals and their predators */
		{ "FLAG_RUG_ERODE", rug_erode_id, "_rug_erode$", no_checking, 86400.0, SP_CORAL },
		{ "FLAG_RUG_BLEACHERODE", rug_bleacherode_id, "_rug_bleacherode$", no_checking, 86400.0, SP_CORAL },
		{ "FLAG_RUG_FEEDSCALAR", rugFeedScalar_id, "_rugFeedScalar$", no_checking, 1.0, SP_PREDATOR },
		{ "FLAG_RUG_FACTOR", rug_factor_id, "_rug_factor$", no_checking, 1.0, SP_RUGOSITY },
		{ "FLAG_COLH", colony_ha_id, "_colony_ha$", no_checking, 1.0, SP_CORAL },
        { "FLAG_RUG_ERODE_SPONGE", rug_erode_sponge_id, "_rug_erode_sponge$", no_checking, 86400.0, SP_CORAL },

		/** Fishery parameters - these are read in from harvest.xml file */
		{ "GroupTargettingOptions", flagfish_id, "^flagfish", binary_check, 1.0, SP_FISHED },
		{ "GroupWCAccess", access_thru_wc_id, "^flag_access_thru_wc_", binary_check, 1.0, SP_FISHED },

		{ "GroupAquacultAge", aquacult_age_harvest_id, "_age_harvest$", integer_check, 1.0, SP_CULTURED },

		/** Mangement parameters - these are read in from the management input file */
		{ "GroupHarvestStrategyTier", tier_id, "^tier", integer_check, 1.0, SP_FISHED },
		{ "RegionalQuotaFlag", regionalSP_id, "^regionalSP", binary_check, 1.0, SP_FISHED },
		{ "SP_Concern", sp_concern_id, "", binary_check, 1.0, SP_FISHED },
		{ "BasketQuotaFlag", basketSP_id, "^basketSP", binary_check, 1.0, SP_FISHED },
		{ "BasketSize", basket_size_id, "^basket_size", integer_check, 1.0, SP_FISHED },
        { "CompanionSpMax", max_co_sp_id, "^max_co_sp_", integer_check, 1.0, SP_FISHED },
		{ "CompanionTACType", coType_id, "^coType_", integer_check, 1.0, SP_FISHED },
		{ "TACResetPeriod", tac_resetperiod_id, "^tac_resetperiod", no_checking, 1.0, SP_FISHED },
        
        /** CPUE generation parameters **/
		{ "CPUEcdfPR", cpue_cdf_poor_r_id, "^cpue_cdf_poor_r_", no_checking, 1.0, SP_FISHED },
		{ "CPUEcdfPP", cpue_cdf_poor_p_id, "^cpue_cdf_poor_p_", no_checking, 1.0, SP_FISHED },
		{ "CPUEcdfTR", cpue_cdf_top_r_id, "^cpue_cdf_top_r_", no_checking, 1.0, SP_FISHED },
		{ "CPUEcdfTP", cpue_cdf_top_p_id, "^cpue_cdf_top_p_", no_checking, 1.0, SP_FISHED },
        
		/* Assessment parameters */
		{ "SampleSize", samplesize_id, "samplesize", integer_check, 1.0, SP_VERTS },
		{ "Allometic_Coefficient", allometic_li_a_id, "li_a", no_checking, 1.0, SP_VERTS },
		{ "Allometic_Exponent", allometic_li_b_id, "li_b$", no_checking, 1.0, SP_VERTS },
		{ "Allometic_Bin_Size", allometic_bin_size_id, "li_bin", no_checking, 1.0, SP_VERTS },
		{ "Allometic_Bin_Start", allometic_bin_start_id, "li_start", no_checking, 1.0, SP_VERTS },
		{ "Max_Length", max_length_id, "li_max", no_checking, 1.0, SP_IMPACTED },

		{ "R_max", r_max_id, "R_max", no_checking, 1.0, SP_ALL },
		{ "avg_inv_size", avg_inv_size_id, "avg_inv_size", no_checking, 1.0, SP_ALL },

		{ "flag_assess", assess_flag_id, "flag_assess", integer_check, 1.0, SP_FISHED },
		{ "assess_bootstrap", assess_bootstrap_id, "assess_bootstrap", no_checking, 1.0, SP_FISHED },
		{ "assess_nat_mort", assess_nat_mort_id, "assess_nat_mort", no_checking, 1.0, SP_FISHED },
		{ "flag_prod_model", flag_prod_model_id, "flag_prod_model", no_checking, 1.0, SP_FISHED },
		{ "top_pcnt", top_pcnt_id, "top_pcnt", no_checking, 1.0, SP_FISHED },
		{ "bot_pcnt", bot_pcnt_id, "bot_pcnt", no_checking, 1.0, SP_FISHED },
		{ "assess_datastream", assessDataStream_id, "assess_datastream", no_checking, 1.0, SP_FISHED },
        
        { "FLAG_WHICHR", whichRAssess_id, "whichRAssess_", no_checking, 1, SP_IMPACTED },

		/* Ice related parameters */
		{ "ICE_KDEP_id", ICE_KDEP_id, "^ICE_KDEP_", no_checking, 1.0, SP_PREDATOR }

	};

//char AtlantisXMLObjectNAMES[8][3][100] = {
//      {"AttributeGroup", "AttributeGroupName", "" },
//      {"Attribute", "AttributeName", "AttributeValue" },
//      {"GroupAttribute", "GroupName", "AttributeValue" },
//      {"AgeClassAttribute", "AgeClassName", "AttributeValue" },
//      {"TemporalAttribute", "AttributeName", "AttributeValue" },
//      {"CohortAttribute", "CohortName", "AttributeValue" },
//      {"FisheryArribute", "FisheyName", "AttributeValue" },
//      {"FisheyGroupAttribute", "GroupName", "AttributeValue"}
//};


/*
 * 	mQ_T15_id,
 mL_T15_id,
 jmL_T15_id,
 jmQ_T15_id,
 mL_id ,
 mQ_id ,
 jmL_id,
 jmQ_id,
 num_migrate_id,
 Time_Spawn_id ,
 Time_Age_id
 */

SpeciesParamStruct cohortSpeciesParamStructArray[cohortDepParams] =
	{

	/** Mortality values */
		{ "FLAG_MQ_T15", mQ_T15_id, "_mQ$", proportion_check, 86400.0, SP_MORT_NOT_PP }, /* Divide by the number of seconds in a day*/
		{ "FLAG_ML_T15", mL_T15_id, "_mL$", proportion_check, 86400.0, SP_MORT },
        { "FLAG_MLext_T15", mLext_id, "_mLext$", proportion_check, 86400.0, SP_MORT },
        { "FLAG_MPext_T15", mPext_id, "_mPext$", proportion_check, 86400.0, SP_MORT },

//		/** Mortality values */
//		{ "FLAG_MQ_T15", mQ_T15_id, "^mQ_|^jmQ_", proportion_check, 86400.0, SP_MORT_NOT_PP }, /* Divide by the number of seconds in a day*/
//		{ "FLAG_ML_T15", mL_T15_id, "^mL_|^jmL_|^epimL_", proportion_check, 86400.0, SP_MORT },


	/** Rugosity related */
		{ "FLAG_RUG_INC", rugosity_inc_id, "_rugosity_inc$", no_checking, 86400.0, SP_CORAL },
		{ "FLAG_RUG_DEC", rugosity_dec_id, "_rugosity_dec$", no_checking, 86400.0, SP_CORAL },
		{ "FLAG_DIAM", colony_diam_id, "_colony_diam", no_checking, 1.0, SP_CORAL },
        
    /** Turbidity effects related */
        { "FLAG_TURBID_L", L_turbid_id, "_turbid_L$", no_checking, 1.0, SP_MORT },
        { "FLAG_TURBID_A", a_turbid_id, "_turbid_a$", no_checking, 1.0, SP_MORT },
        { "FLAG_TURBID_B", b_turbid_id, "_turbid_b$", no_checking, 1.0, SP_MORT }
        
    };

SpeciesParamStruct spawnSpeciesParamStructArray[spawnDepParams] =
	{
	/** Reproduction paremeters  */
		{ "FLAG_TIME_SPAWN", Time_Spawn_id, "_Time_Spawn$", integer_check, 1, SP_TIME },

	/** Recruitment */
		{ "FLAG_TIME_AGE", Time_Age_id, "^Time_Age_", integer_check, 1, SP_TIME_AGE }
    };



SpeciesParamStruct RBCSpeciesParamStructArray[num_rbc_species_params_id] =
	{

	/** assessment specifications */
		{ "FLAG_DISCTYPE", DiscType_id, "^DiscType_", integer_check, 1, SP_FISHED },
		{ "FLAG_MAXH", MaxH_id, "^MaxH_", no_checking, 1, SP_IMPACTED },
		{ "FLAG_GROWAGEL1", Growthage_L1_id, "^Growthage_L1_", no_checking, 1, SP_FISHED },
		{ "FLAG_GROWAGEL2", Growthage_L2_id, "^Growthage_L2_", no_checking, 1, SP_FISHED },
		{ "FLAG_MINCATCH", MinCatch_id, "^MinCatch_", no_checking, 1, SP_FISHED },
		{ "FLAG_SSSTART", AssessStart_id, "^AssessStart_", no_checking, 1, SP_FISHED },
		{ "FLAG_NREG", NumRegions_id, "^NumRegions_", integer_check, 1, SP_FISHED },
        { "FLAG_NSEX", Nsexes_id, "^Nsexes_", integer_check, 1, SP_FISHED },
        
        { "FLAG_TIER1SIG", Tier1Sig_id, "^Tier1Sig_", no_checking, 1, SP_IMPACTED },
        { "FLAG_TIER2SIG", Tier2Sig_id, "^Tier2Sig_", no_checking, 1, SP_IMPACTED },
        { "FLAG_TIER3SIG", Tier3Sig_id, "^Tier3Sig_", no_checking, 1, SP_IMPACTED },
        
        { "FLAG_TRIGGERSP", isTriggerSpecies_id, "^isTriggerSpecies_", integer_check, 1, SP_IMPACTED },
        { "FLAG_TRIGTHRESH", trigger_threshold_id, "^trigger_threshold_", no_checking, 1, SP_IMPACTED },
        { "FLAG_USERBCAVG", UseRBCAveraging_id, "^UseRBCAveraging_", integer_check, 1, SP_IMPACTED },
        
	/** reference levels */
		{ "FLAG_MATINFLECT", Maturity_Inflect_id, "^Maturity_Inflect_", no_checking, 1, SP_FISHED },
		{ "FLAG_MATSLOPE", Maturity_Slope_id, "^Maturity_Slope_", no_checking, 1, SP_FISHED },

	/** tier specific rules */
		{ "FLAG_T1STEEP", T1_steep_phase_id, "^T1_steep_phase_", integer_check, 1, SP_FISHED },
		{ "FLAG_TIERTYPE", tiertype_id, "^tiertype_", integer_check, 1, SP_IMPACTED },

	// tier 3
		{ "FLAG_TIER3FCALC", Tier3_Fcalc_id, "^Tier3_Fcalc_", no_checking, 1, SP_FISHED },
		{ "FLAG_TIER3TIME", Tier3_time_id, "^Tier3_time_", no_checking, 1, SP_FISHED },
		{ "FLAG_TIER3MAXAGE", Tier3_maxage_id, "^Tier3_maxage_", no_checking, 1, SP_FISHED },
		{ "FLAG_TIER3M", Tier3_M_id, "^Tier3_M_", no_checking, 1, SP_FISHED },
		{ "FLAG_TIER3S25", Tier3_S25_id, "^Tier3_S25_", no_checking, 1, SP_FISHED },
		{ "FLAG_TIER3S50", Tier3_S50_id, "^Tier3_S50_", no_checking, 1, SP_FISHED },
		{ "FLAG_TIER3F", Tier3_F_id, "^Tier3_F_", no_checking, 1, SP_FISHED },
		{ "FLAG_TIER3H", Tier3_h_id, "^Tier3_h_", no_checking, 1, SP_FISHED },
		{ "FLAG_TIER3MATLEN", Tier3_matlen_id, "^Tier3_matlen_", no_checking, 1, SP_FISHED },
		{ "FLAG_TIER3MAXF", Tier3_maxF_id, "^Tier3_maxF_", no_checking, 1, SP_FISHED },
		{ "FLAG_CCSELYEAR", CCsel_years_id, "^CCsel_years_", no_checking, 1, SP_FISHED },

	// tier 4
		{ "FLAG_TIER4AVTIME", Tier4_avtime_id, "^Tier4_avtime_", no_checking, 1, SP_FISHED },
		{ "FLAG_TIER4MIN", Tier4_CPUEyrmin_id, "^Tier4_CPUEyrmin_", no_checking, 1, SP_FISHED },
		{ "FLAG_TIER4MAX", Tier4_CPUEyrmax_id, "^Tier4_CPUEyrmax_", no_checking, 1, SP_FISHED },
		{ "FLAG_TIER4M", Tier4_m_id, "^Tier4_m_", no_checking, 1, SP_FISHED },
		{ "FLAG_TIER4ALPHA", Tier4_alpha_id, "^Tier4_alpha_", no_checking, 1, SP_FISHED },
		{ "FLAG_TIER4CMAX", Tier4_Cmaxmult_id, "^Tier4_Cmaxmult_", no_checking, 1, SP_FISHED },
        { "FLAG_TIER4BOCORR", Tier4_Bo_correct_id, "^Tier4_Bo_correct_", no_checking, 1, SP_FISHED },

	//tier 5
		{ "FLAG_TIER5LEN", Tier5_length_id, "^Tier5_length_", no_checking, 1, SP_IMPACTED },
		{ "FLAG_TIER5S50", Tier5_S50_id, "^Tier5_S50_", no_checking, 1, SP_IMPACTED },
		{ "FLAG_TIER5CV", Tier5_cv_id, "^Tier5_cv_", no_checking, 1, SP_IMPACTED },
		{ "FLAG_TIER5FLT", Tier5_flt_id, "^Tier5_flt_", no_checking, 1, SP_IMPACTED },
		{ "FLAG_TIER5REG", Tier5_reg_id, "^Tier5_reg_", no_checking, 1, SP_IMPACTED },
		{ "FLAG_TIER5P", Tier5_p_id, "^Tier5_p_", no_checking, 1, SP_IMPACTED },
		{ "FLAG_TIER5SEL", Tier5sel_id, "^Tier5sel_", no_checking, 1, SP_IMPACTED },
		{ "FLAG_TIER5Q", Tier5q_id, "^Tier5q_", no_checking, 1, SP_IMPACTED },
		{ "FLAG_POSTRULE", PostRule_id, "^PostRule_", no_checking, 1, SP_IMPACTED },

		{ "FLAG_CPUEMULT", CPUEmult_id, "^CPUEmult_", no_checking, 1, SP_IMPACTED },
		{ "FLAG_MAXCHANGE", MaxChange_id, "^MaxChange_", no_checking, 1, SP_IMPACTED },

    // tier 6
		{ "FLAG_TRIGRESP", TriggerResponseScen_id, "TriggerResponseScen_", integer_check, 1, SP_IMPACTED },
        
	/** Other paremeters  */
		{ "FLAG_MG_OFFSET", MG_offset_id, "^MG_offset_", integer_check, 1, SP_IMPACTED },
		{ "FLAG_SHIFTASSESS", Regime_shift_assess_id, "^Regime_shift_assess_", integer_check, 1, SP_IMPACTED },
		{ "FLAG_RECDEVBACK", RecDevBack_id, "^RecDevBack_", no_checking, 1, SP_IMPACTED },
		{ "FLAG_HSTEEP", Hsteep_id, "^Hsteep_", no_checking, 1, SP_IMPACTED },
		{ "FLAG_AGESELPAT", Agesel_Pattern_id, "^Agesel_Pattern_", no_checking, 1, SP_IMPACTED },
		{ "FLAG_ASSESSFREQ", AssessFreq_id, "^AssessFreq_", no_checking, 1, SP_IMPACTED },
		{ "FLAG_BALLPARKF", BallParkF_id, "^BallParkF_", no_checking, 1, SP_FISHED },
		{ "FLAG_BALLPARKYR", BallParkYr_id, "^BallParkYr_", no_checking, 1, SP_FISHED },
		{ "FLAG_NUMCHANGELAM", NumChangeLambda_id, "^NumChangeLambda_", integer_check, 1, SP_FISHED },
		{ "FLAG_NUMCHANGELAM", num_enviro_obs_id, "^num_enviro_obs_", integer_check, 1, SP_FISHED },
		{ "FLAG_NUMCHANGELAM", num_growth_morphs_id, "^num_growth_morphs_", integer_check, 1, SP_FISHED },
        
	// size parameters
		{ "FLAG_NSEXSAMP", Nsex_samp_id, "Nsex_samp_", integer_check, 1, SP_IMPACTED },
		{ "FLAG_MAXAGE", MaxAge_id, "MaxAge_", integer_check, 1, SP_IMPACTED },
		{ "FLAG_NYFUT", Nyfuture_id, "Nyfuture_", integer_check, 1, SP_IMPACTED },
        { "FLAG_NF", NumFisheries_id, "NumFisheries_", integer_check, 1, SP_IMPACTED },
		{ "FLAG_NLEN", Nlen_id, "Nlen_", integer_check, 1, SP_IMPACTED },
		{ "FLAG_LBIN", Lbin_id, "Lbin_", integer_check, 1, SP_IMPACTED },
        { "FLAG_THRESHMAT", thresh_mat_id, "thresh_mat_", no_checking, 1, SP_IMPACTED },
        { "FLAG_SEXRATIO", femsexratio_id, "femsexratio_", no_checking, 1, SP_IMPACTED },
        
	// SS3 Biological parameters
		{ "FLAG_LADIRECT", flagLAdirect_id, "flagLAdirect_", integer_check, 1, SP_IMPACTED },
		{ "FLAG_SLADIRECT", flagSLAdirect_id, "flagSLAdirect_", integer_check, 1, SP_IMPACTED },
		{ "FLAG_WADIRECT", flagWAdirect_id, "flagWAdirect_", integer_check, 1, SP_IMPACTED },

	// recruitment parameters
		{ "FLAG_SIGMAR1", SigmaR1_id, "SigmaR1_", no_checking, 1, SP_IMPACTED },
		{ "FLAG_SIGMAR2", SigmaR2_id, "SigmaR2_", no_checking, 1, SP_IMPACTED },
		{ "FLAG_SIGMARFUT", SigmaR_future_id, "SigmaR_future_", no_checking, 1, SP_IMPACTED },
		{ "FLAG_PSIGMAR1", PSigmaR1_id, "PSigmaR1_", no_checking, 1, SP_IMPACTED },
		{ "FLAG_REGIMEYR", Regime_year_id, "Regime_year_", integer_check, 1, SP_IMPACTED },
		{ "FLAG_RECDEVMINYR", RecDevMinYr_id, "RecDevMinYr_", integer_check, 1, SP_IMPACTED },
		{ "FLAG_RECDEVMAXYR", RecDevMaxYr_id, "RecDevMaxYr_", integer_check, 1, SP_IMPACTED },
		{ "FLAG_RECDEV", RecDevFlag_id, "RecDevFlag_", integer_check, 1, SP_IMPACTED },
		{ "FLAG_AUTOCORRECDEV", AutoCorRecDev_id, "AutoCorRecDev_", integer_check, 1, SP_IMPACTED },
        
    // Ratpack parameters
        { "FLAG_LFSSLIM", LFSSlim_id, "LFSSlim_", integer_check, 1, SP_IMPACTED },
        { "FLAG_AFSSLIM", AFSSlim_id, "AFSSlim_", integer_check, 1, SP_IMPACTED },
        { "FLAG_NSURVEY", NumSurvey_id, "NumSurvey_", integer_check, 1, SP_IMPACTED },
        { "FLAG_REGIMEYR", Regime_year_assess_id, "Regime_year_assess_", integer_check, 1, SP_IMPACTED },
        { "FLAG_NBLOCK", NblockPattern_id, "NblockPattern_", integer_check, 1, SP_IMPACTED },
        { "FLAG_SRBLOCK", SRBlock_id, "SRBlock_", no_checking, 1, SP_IMPACTED },
        { "FLAG_REDDEVMIN", assRecDevMinYear_id, "assRecDevMinYear_", integer_check, 1, SP_IMPACTED },
        
        { "FLAG_MSASSESS", MultispAssessType_id, "MultispAssessType_", integer_check, 1, SP_IMPACTED },
        { "FLAG_INDIC_SP", mgt_indicator_id, "mgt_indicator_", integer_check, 1, SP_IMPACTED },
        { "FLAG_INITMGMT_CAT", init_mgt_category_id, "init_mgt_category_", integer_check, 1, SP_IMPACTED },
        { "FLAG_INITMGMT_SP", init_mgt_sp_id, "init_mgt_sp_", integer_check, 1, SP_IMPACTED },
        
    // PGMSY parameters
        { "FLAG_PGMSYALPHA", PGMSYBHalpha_id, "^PGMSYBHalpha_", no_checking, 1, SP_FISHED },
        { "FLAG_PGMSYBETA", PGMSYBHbeta_id, "^PGMSYBHbeta_", no_checking, 1, SP_FISHED }
	};


char AtlantisXMLObjectNAMES[NUM_ATLANTIS_NODE_TYPES][3][100] =
	{
		{ "AttributeGroup", "AttributeGroupName", "" },
		{ "AttributeSubGroup", "AttributeGroupName", "" },
		{ "Attribute", "AttributeName", "AttributeValue" },
		{ "GroupValue", "GroupName", "AttributeValue" },
		{ "AgeClassValue", "AgeClassName", "AttributeValue" },
		{ "TemporalValue", "AttributeName", "AttributeValue" },
		{ "CohortValue", "CohortName", "AttributeValue" },
		{ "SexValue", "SexName", "AttributeValue" },
		{ "FisheryValue", "FisheryName", "AttributeValue" },
		{ "FisheyGroupValue", "GroupName", "AttributeValue" },
        { "CompanionValue", "CompanionName", "AttributeValue" }
    };


char AtlantisXMLAttributeTypeStrings[10][100] =
	{
		{ "Float" },
		{ "Integer" },
		{ "Boolean" },
		{ "FloatArray" },
		{ "IntegerArray" },
		{ "Lookup" },
		{ "BooleanArray" },
		{ "LookupArray" },
        {"String" },
        {"StringArray" }};

static int Check_Input_Array(double **p, int sized, int entry_type, char *key) {
	int returnValue = TRUE;
	int i;
	double sum, diff, base;

	switch (entry_type) {
	case no_checking:
		/* No checks needed */
		break;
	case binary_check:
		/* Flag values only (i.e. either 1 or 0 only) */
		for (i = 0; i < sized; i++) {
			if ((*p)[i] != 0.0 && (*p)[i] != 1.0)
				quit("Entries in %s are supposed to be flag values (i.e. either 1 or 0), but %d entry is %e\n", key, i, (*p)[i]);
		}
		break;
	case proportion_check:
		/* Values are between 0 and 1 */
		for (i = 0; i < sized; i++) {
			if ((*p)[i] < 0.0 || (*p)[i] > 1.0)
				quit("Entries in %s are supposed to be proportions (i.e. between 0 and 1), but %d entry is %e\n", key, i, (*p)[i]);
		}
		break;
	case probability_check:
		/* Values are between 0 and 1, and sum is 1.0 */
		if (verbose > 2)
			printf("Checking %s\n", key);
		sum = 0.0;
		for (i = 0; i < sized; i++) {
			if ((*p)[i] < 0.0 && (*p)[i] > 1.0)
				quit("Entries in %s are supposed to be proportions (i.e. between 0 and 1), but %d entry is %e\n", key, i, (*p)[i]);
			else {
				sum += (*p)[i];
				if (verbose > 2)
					printf("%e (sum %e)\n", (*p)[i], sum);
			}
		}
		diff = fabs(sum - 1.0);
		if (diff > buffer_rounding)
			quit("Entries in %s are supposed to sum to 1.0, but current sum is %e (diff %e)\n", key, sum, diff);
		break;
	case integer_check:
		/* Check that integer values are integer */
		for (i = 0; i < sized; i++) {
			base = floor((*p)[i]);
			diff = fabs((*p)[i] - base);
			if (diff > 0)
				quit("Entries in %s are supposed to be integer values, but %d entry is %e\n", key, i, (*p)[i]);
		}
		break;
	default:
		quit("Check id for array check must be 0, 1, 2 or 3. You have set %d, so recode\n", entry_type);
		break;
	}

	return returnValue;
}

/* Comment out if attributes should be added as attributes not elements */
//#define ELEMENT_ATTRIBUTE

/**
 * register_namespaces:
 * @xpathCtx:       the pointer to an XPath context.
 * @nsList:     the list of known namespaces in
 *          "<prefix1>=<href1> <prefix2>=href2> ..." format.
 *
 * Registers namespaces from @nsList in @xpathCtx.
 *
 * Returns 0 on success and a negative value otherwise.
 */
static int Register_Namespaces(xmlXPathContextPtr xpathCtx, const xmlChar* nsList) {
	xmlChar* nsListDup;
	xmlChar* prefix;
	xmlChar* href;
	xmlChar* next;

	assert(xpathCtx);
	assert(nsList);

	nsListDup = xmlStrdup(nsList);
	if (nsListDup == NULL) {
		fprintf(stderr, "Error: unable to strdup namespaces list\n");
		return (-1);
	}

	next = nsListDup;
	while (next != NULL) {
		/* skip spaces */
		while ((*next) == ' ')
			next++;
		if ((*next) == '\0')
			break;

		/* find prefix */
		prefix = next;
		next = (xmlChar*) xmlStrchr(next, '=');
		if (next == NULL) {
			fprintf(stderr, "Error: invalid namespaces list format\n");
			xmlFree(nsListDup);
			return (-1);
		}
		*(next++) = '\0';

		/* find href */
		href = next;
		next = (xmlChar*) xmlStrchr(next, ' ');
		if (next != NULL) {
			*(next++) = '\0';
		}

		/* do register namespace */
		if (xmlXPathRegisterNs(xpathCtx, (unsigned char *) prefix, (unsigned char *) href) != 0) {
			fprintf(stderr, "Error: unable to register NS with prefix=\"%s\" and href=\"%s\"\n", prefix, href);
			xmlFree(nsListDup);
			return (-1);
		}
	}

	xmlFree(nsListDup);
	return (0);
}

/**
 * Execute_Xpath_Expression:
 * @filename:       the input XML filename.
 * @xpathExpr:      the xpath expression for evaluation.
 * @nsList:     the optional list of known namespaces in
 *          "<prefix1>=<href1> <prefix2>=href2> ..." format.
 *
 * Parses input XML file, evaluates XPath expression and prints results.
 *
 * Returns 0 on success and a negative value otherwise.
 */
static xmlXPathObjectPtr Execute_Xpath_Expression(xmlDocPtr doc, const xmlChar* xpathExpr, const xmlChar* nsList) {
	xmlXPathContextPtr xpathCtx;
	xmlXPathObjectPtr xpathObj;

	assert(xpathExpr);

	/* Create xpath evaluation context */
	xpathCtx = xmlXPathNewContext(doc);
	if (xpathCtx == NULL) {
		fprintf(stderr, "Error: unable to create new XPath context\n");
		xmlFreeDoc(doc);
		return (NULL);
	}

	/* Register namespaces from list (if any) */
	if ((nsList != NULL) && (Register_Namespaces(xpathCtx, nsList) < 0)) {
		fprintf(stderr, "Error: failed to register namespaces list \"%s\"\n", nsList);
		xmlXPathFreeContext(xpathCtx);
		return (NULL);
	}

	/* Evaluate xpath expression */
	xpathObj = xmlXPathEvalExpression(xpathExpr, xpathCtx);
	if (xpathObj == NULL) {
		fprintf(stderr, "Error: unable to evaluate xpath expression \"%s\"\n", xpathExpr);
		xmlXPathFreeContext(xpathCtx);
		return (NULL);
	}

	/* Cleanup */
	xmlXPathFreeContext(xpathCtx);

	return xpathObj;
}

void printNode(xmlNodePtr node) {

	xmlBufferPtr buff = xmlBufferCreate();
	xmlNodeDump(buff, node->doc, node->doc->children, 1, 1);

	printf("buff = %s\n", (char *) buff->content);
	xmlBufferFree(buff);

}

/**
 * \Add a property to the given node.
 *
 * This will either be done using an attribute or creating a new text child node.
 */

void Util_XML_Add_Node_Property(xmlNodePtr attributeNode, char *attributeName, char *attributeValue) {

#ifdef ELEMENT_ATTRIBUTE
	xmlNewTextChild(attributeNode, NULL, (unsigned char *)attributeName, (unsigned char *)attributeValue);
#else
	xmlNewProp(attributeNode, (unsigned char *) attributeName, (unsigned char *) attributeValue);
#endif

}

static void Add_Comment_Node(xmlNodePtr rootNode, char *comment, char *units) {

	if (comment != NULL) {
		if (strlen(comment) > 2000) {
			quit("Comment is too long length = %d\n", strlen(comment));
		}
		if (strlen(comment) > 0) {
			Util_XML_Add_Node_Property(rootNode, "AttributeDescription", comment);
		}
	}
	if (strlen(units) > 0) {
		Util_XML_Add_Node_Property(rootNode, "AttributeUnits", units);
	}
}

/**
 * \Add a property to the given node.
 *
 * This will either be done using an attribute or creating a new text child node.
 */

void Util_XML_Set_Node_Property(xmlNodePtr attributeNode, char *attributeName, char *attributeValue) {

#ifdef ELEMENT_ATTRIBUTE
	xmlXPathObjectPtr attributeValueNode;
	char xpath[1000];
	char *currentPath = (char *)xmlGetNodePath(attributeNode);
	sprintf(xpath, "%s/%s", currentPath, (unsigned char *) attributeName);
	free(currentPath);

	/* Need to see if the given attributeNode has a child called attributeName - if so then set its value */
	attributeValueNode = Execute_Xpath_Expression(attributeNode->doc, (unsigned char *)xpath, NULL);
	if(attributeValueNode->nodesetval->nodeNr > 0)
	xmlSetProp(attributeValueNode->nodesetval->nodeTab[0], (unsigned char *)attributeName, (unsigned char *)attributeValue);
	else
	xmlNewTextChild(attributeNode, NULL, (unsigned char *)attributeName, (unsigned char *)attributeValue);

	xmlXPathFreeObject(attributeNode);
#else
	xmlSetProp(attributeNode, (unsigned char *) attributeName, (unsigned char *) attributeValue);
#endif

}

/**
 * \Add a property to the given node.
 *
 * This will either be done using an attribute or creating a new text child node.
 */

static char *Get_Node_Property(xmlNodePtr attributeNode, char *attributeName) {

#ifdef ELEMENT_ATTRIBUTE
	xmlXPathObjectPtr attributeValueNode;
	char xpath[1000];
	char *currentPath = (char *)xmlGetNodePath(attributeNode);
	char *returnValue;
	sprintf(xpath, "%s/%s", currentPath, (unsigned char *) attributeName);
	free(currentPath);

	/* Need to see if the given attributeNode has a child called attributeName - if so then set its value */
	attributeValueNode = Execute_Xpath_Expression(attributeNode->doc, (unsigned char *)xpath, NULL);
	if(attributeValueNode->nodesetval->nodeNr > 0)
	returnValue = (char *)xmlGetProp(attributeValueNode->nodesetval->nodeTab[0], (unsigned char *)attributeName);
	else
	returnValue = "";

	xmlXPathFreeObject(attributeNode);
	return returnValue;
#else
	return (char *) xmlGetProp(attributeNode, (unsigned char *) attributeName);
#endif

}

/**
 * \brief Build the path string for the given node.
 *
 * The value will be stored in the given xpath string. It is assumed that the xpath string is already allocated and does
 * not do any checking to see that this is the case.
 *
 *
 */
static void Build_Node_XPath(xmlNodePtr parent, char *xpath, char *nodeName, char *attributeName, char *attributeValue) {

	char *currentPath = (char *) xmlGetNodePath(parent);

#ifdef ELEMENT_ATTRIBUTE
	sprintf(xpath, "%s/%s[%s='%s']", currentPath, (unsigned char *)nodeName, (unsigned char *)attributeName, (unsigned char *) attributeValue);
#else
	sprintf(xpath, "%s/%s[@%s='%s']", currentPath, (unsigned char *) nodeName, (unsigned char *) attributeName, (unsigned char *) attributeValue);
#endif
	free(currentPath);
}

/**
 * \brief Create and return a new atlantis Attribute node of the given type.
 *
 *
 *
 */
xmlNodePtr Util_XML_Create_Lookup_Node(xmlNodePtr parent) {
	xmlNodePtr attributeNode;

	attributeNode = xmlNewChild(parent, NULL, (unsigned char *) "Lookup", (unsigned char *) "");

	return attributeNode;
}

/**
 * \brief Create and return a new atlantis Attribute node of the given type.
 *
 *
 *
 */
xmlNodePtr Util_XML_Create_Lookup_Value_Node(xmlNodePtr parent, char *value, char *description) {
	xmlNodePtr lookupNode;

	lookupNode = xmlNewChild(parent, NULL, (unsigned char *) "LookupValue", (unsigned char *) "");
	Util_XML_Add_Node_Property(lookupNode, "LookupValue", value);
	Util_XML_Add_Node_Property(lookupNode, "LookupDescription", description);

	return lookupNode;
}

/**
 * \brief Create and return a new atlantis Attribute node of the given type.
 *
 *
 *
 */
xmlNodePtr Util_XML_Create_Node(int type, xmlNodePtr parent, char *attributeName, char *commentString, char *units, char *value) {
	xmlNodePtr attributeNode;

	trim(value);
	attributeNode = xmlNewChild(parent, NULL, (unsigned char *) AtlantisXMLObjectNAMES[type][ATTRIBUTE_TYPE], (unsigned char *) "");
	Util_XML_Add_Node_Property(attributeNode, AtlantisXMLObjectNAMES[type][ATTRIBUTE_NAME], attributeName);
	Add_Comment_Node(attributeNode, commentString, units);

	if (strlen(value) > 0) {
		//printf("set prop %s = %s\n", attributeName, value);
		/* Make sure there is not \n on the end of the value */
		if (strchr(value, '\n') != NULL)
			*strchr(value, '\n') = '\0';
		Util_XML_Add_Node_Property(attributeNode, AtlantisXMLObjectNAMES[type][ATTRIBUTE_VALUE], value);
	}

	return attributeNode;
}

/*
 * brief Find the xml node in the given document with the given xpath.
 * The xpath is assumed to be relative to the given parent.
 *
 * Will currently return NULL if the node is not found.
 * This function should be changed to return a result TRUE or FALSE
 * and return the pointer to the node as an output parameter.
 *
 *
 *  This function results in a memory issue that will have to be cleaned up.
 */
xmlXPathObjectPtr Util_XML_Get_Node_List(int type, xmlNodePtr parent, char *attributeName) {
	char xpath[1000];
	xmlXPathObjectPtr attributeNode;
	/**
	 * See if the node already exists.
	 */
	Build_Node_XPath(parent, &xpath[0], AtlantisXMLObjectNAMES[type][ATTRIBUTE_TYPE], AtlantisXMLObjectNAMES[type][ATTRIBUTE_NAME], attributeName);

	if (verbose > 2)
		printf("Util_XML_Get_Node_List: xpath = %s\n", xpath);

	attributeNode = Execute_Xpath_Expression(parent->doc, (unsigned char *) xpath, NULL);

	return attributeNode;
}

/*
 * brief Find the xml node in the given document with the given xpath.
 * The xpath is assumed to be relative to the given parent.
 *
 * Will currently return NULL if the node is not found.
 * This function should be changed to return a result TRUE or FALSE
 * and return the pointer to the node as an output parameter.
 *
 */
xmlNodePtr Util_XML_Get_Node(int type, xmlNodePtr parent, char *attributeName) {

	xmlNodePtr returnValue = NULL;
	xmlXPathObjectPtr attributeNode = Util_XML_Get_Node_List(type, parent, attributeName);

	if (attributeNode == NULL)
		returnValue = NULL;
	if (attributeNode->nodesetval->nodeNr > 0)
		returnValue = attributeNode->nodesetval->nodeTab[0];

	xmlXPathFreeObject(attributeNode);

	return returnValue;
}

xmlNodePtr Util_XML_Get_Or_Create_Node(int type, xmlNodePtr parent, char *attributeName) {

	xmlNodePtr returnValue;

	returnValue = Util_XML_Get_Node(type, parent, attributeName);
	if (returnValue == NULL)
		returnValue = Util_XML_Create_Node(type, parent, attributeName, "", "", "");
	return returnValue;

}

/**
 *
 *
 *
 *
 */
void Util_XML_Set_Node_Value(int type, xmlNodePtr parent, char *attributeName, char *value) {

	xmlXPathObjectPtr attributeNode;
	char xpath[1000];

	if(value == NULL){
		quit("ERROR: Util_XML_Set_Node_Value value is NULL for attributeName %s", attributeName);
	}
	/* Make sure there is not \n on the end of the value */
	if (strchr(value, '\n') != NULL)
		*strchr(value, '\n') = '\0';

	Build_Node_XPath(parent, &xpath[0], AtlantisXMLObjectNAMES[type][ATTRIBUTE_TYPE], AtlantisXMLObjectNAMES[type][ATTRIBUTE_NAME], attributeName);
	
	if (verbose > 2)
		printf("Util_XML_Set_Node_Value: xpath = %s with value %s\n", xpath, value);

	
	attributeNode = Execute_Xpath_Expression(parent->doc, (unsigned char *) xpath, NULL);
	if (attributeNode->nodesetval->nodeNr > 0) {
		Util_XML_Set_Node_Property(attributeNode->nodesetval->nodeTab[0], AtlantisXMLObjectNAMES[type][ATTRIBUTE_VALUE], value);
	} else {
		/* Create the node */
		Util_XML_Create_Node(type, parent, attributeName, "", "", value);
	}
	xmlXPathFreeObject(attributeNode);
}

void Util_XML_Replace_Node_Value(int type, xmlNodePtr parent, char *attributeName, char *value, int size, int replaceIndex) {
	xmlXPathObjectPtr attributeNode;
	char xpath[1000];
	char *currentValue;
	char *newNodeValue;
	int i;

	Build_Node_XPath(parent, &xpath[0], AtlantisXMLObjectNAMES[type][ATTRIBUTE_TYPE], AtlantisXMLObjectNAMES[type][ATTRIBUTE_NAME], attributeName);

	if (verbose > 2)
		printf("Util_XML_Replace_Node_Value: xpath = %s with value %s\n", xpath, value);

	attributeNode = Execute_Xpath_Expression(parent->doc, (unsigned char *) xpath, NULL);
	if (attributeNode->nodesetval->nodeNr > 0) {
		currentValue = Get_Node_Property(attributeNode->nodesetval->nodeTab[0], AtlantisXMLObjectNAMES[type][ATTRIBUTE_VALUE]);
		newNodeValue = Replace_String_Entry(currentValue, value, size, replaceIndex);
		Util_XML_Set_Node_Property(attributeNode->nodesetval->nodeTab[0], AtlantisXMLObjectNAMES[type][ATTRIBUTE_VALUE], newNodeValue);

		free(currentValue);
		free(newNodeValue);

	} else {
		currentValue = (char *) malloc(sizeof(char) * (size_t)(size * 10));
		strcpy(currentValue, "0.0");
		for (i = 1; i < size; i++)
			strcat(currentValue, " 0.0");

		newNodeValue = Replace_String_Entry(currentValue, value, size, replaceIndex);
		Util_XML_Create_Node(type, parent, attributeName, "", "", newNodeValue);

		free(currentValue);
		free(newNodeValue);

	}

	xmlXPathFreeObject(attributeNode);
}

int Util_XML_Get_Node_Value_Double(int type, xmlNodePtr node, double *returnValue) {
	char *value = Get_Node_Property(node, AtlantisXMLObjectNAMES[type][ATTRIBUTE_VALUE]);

	if(value == NULL){
		return -1;
	}
	*returnValue = atof(value);

	//	sscanf(value, "%f",&returnValue);
	//
	free(value);
	return 1;

}
int Util_XML_Get_Node_Value_Int(char *fileName, int type, xmlNodePtr node) {

	char *value;
	int returnValue;

	if (node == NULL) {
		quit("Util_XML_Get_Node_Value_Int given node is null\n");
	}
	value = Get_Node_Property(node, AtlantisXMLObjectNAMES[type][ATTRIBUTE_VALUE]);

	if (value == NULL) {
        printf("Missing integer - Util_XML_Get_Node_Value_Int %s not found in file %s\n", Get_Node_Property(node, AtlantisXMLObjectNAMES[type][ATTRIBUTE_NAME]), fileName);
		printNode(node);
		printf("AtlantisXMLObjectNAMES[type][ATTRIBUTE_VALUE] = %s\n", AtlantisXMLObjectNAMES[type][ATTRIBUTE_VALUE]);
		quit("Util_XML_Get_Node_Value_Int %s not found in file %s\n", Get_Node_Property(node, AtlantisXMLObjectNAMES[type][ATTRIBUTE_NAME]), fileName);
	}
	if (strlen(value) == 0)
		quit("Util_XML_Get_Node_Value_Int %s not found in file %s\n", node->name, fileName);
	returnValue = atoi(value);

	free(value);
	return returnValue;

}

/**
 *  Parameter reading
 *  This routine reads a parameter value of ecological models and
 *  modelling process according to its name, by searching from
 *  the begining of the file.
 *
 *  Inputs: FILE  *infile:  handle of input file,
 *      char  *valueName: parameter name.
 *  Output: returned value of the parameter, or 0.0 if it's not found.
 *
 */
int Util_XML_Get_Value_Double(char *fileName, int type, int localecotest, int isRequired, xmlNodePtr parent, int entry_type, char *valueName,
		double *returnValue) {
	double value = 0;
	xmlNodePtr attributeNode;

	attributeNode = Util_XML_Get_Node(type, parent, valueName);

	if (attributeNode == NULL) {
		if (isRequired == TRUE)
			quit("Error: Cannot find atlantis parameter '%s' in file %s.\n", valueName, fileName);
		*returnValue = 0.0;
		return FALSE;
	}

	/* Successfully found parameter - convert to double, check and return.*/
	if(!Util_XML_Get_Node_Value_Double(ATLANTIS_ATTRIBUTE, attributeNode, &value)){
		if (isRequired == TRUE)
			quit("Error: Cannot find atlantis parameter '%s' in file %s.\n", valueName, fileName);
		*returnValue = 0.0;
		return FALSE;
	}
	if (verbose > 2)
		printf("value = %e\n", value);
	Check_Value(valueName, value, entry_type, valueName);
	
	printf("In atUtilXML printing ecosystem parameters");
	
	if (localecotest > 2)
		printf("Ecosystem parameter: %s = %f\n", valueName, value);
	printf("In atUtilXML printing ecosystem parameters");
	
	*returnValue = value;
	return TRUE;
}

/**
 *  Parameter reading
 *  This routine reads a parameter value of ecological models and
 *  modelling process according to its name, by searching from
 *  the begining of the file.
 *
 *  Inputs: FILE  *infile:  handle of input file,
 *      char  *valueName: parameter name.
 *  Output: returned value of the parameter, or 0.0 if it's not found.
 *
 */
int Util_XML_Get_Value_Integer(char *fileName, int type, int localecotest, int isRequired, xmlNodePtr parent, int entry_type, char *valueName, int *returnValue) {
	int value;
	xmlNodePtr attributeNode;
    
	attributeNode = Util_XML_Get_Node(type, parent, valueName);
    
    //if (verbose > 2)
    //    printf("Util_XML_Get_Value_Integer reading %s\n", valueName);

	if (attributeNode == NULL) {
		if (isRequired == TRUE)
			quit("Error: Cannot find atlantis parameter '%s' in input file %s.\n", valueName, fileName);
		*returnValue = 0;
		return FALSE;
	}

	/* Sucessfully found parameter - convert to double, check and return.*/
	value = Util_XML_Get_Node_Value_Int(fileName, ATLANTIS_ATTRIBUTE, attributeNode);

	if (verbose > 2)
		printf("Util_XML_Get_Value_Integer %s value = %d\n", valueName, value);

	/* Check the values */
	Check_Value(valueName, value, entry_type, valueName);
	if (localecotest > 2)
		printf("Ecosystem parameter: %s = %d\n", valueName, value);

	*returnValue = value;
	return TRUE;
}

/**
 *  Parameter reading
 *  This routine reads a parameter value of ecological models and
 *  modelling process according to its name, by searching from
 *  the begining of the file.
 *
 *  Inputs: FILE  *infile:  handle of input file,
 *      char  *valueName: parameter name.
 *  Output: returned value of the parameter, or 0.0 if it's not found.
 *
 */
int Util_XML_Get_Value_String(char *fileName, int type, int isRequired, xmlNodePtr parent, char *valueName, char *returnValue) {
	char *value;
	xmlNodePtr attributeNode;

	attributeNode = Util_XML_Get_Node(type, parent, valueName);

	if (attributeNode == NULL) {
		if (isRequired == TRUE)
			quit("Error: Cannot find atlantis parameter '%s' in input file %s.\n", valueName, fileName);
		return FALSE;
	}

	/* Sucessfully found parameter - convert to double, check and return.*/
	value = Util_XML_Get_Node_Value_String(ATLANTIS_ATTRIBUTE, attributeNode);

	if (verbose > 2)
		printf("Util_XML_Get_Value_String: Read in value %s for variable %s\n", value, valueName);

	if (value == NULL && isRequired == TRUE) {
		quit("Util_XML_Get_Value_String: NULL value found for node %s in input file %s.\n", valueName, fileName);
	}

	/* Copy the found string into the destination */
	strcpy(returnValue, value);

	free(value);
	return TRUE;
}

/**
 * \brief Get the value string of the given node.
 * NOTE: the returned string must be freed by the caller.
 *
 *
 */
char *Util_XML_Get_Node_Value_String(int type, xmlNodePtr node) {

	char *value = Get_Node_Property(node, AtlantisXMLObjectNAMES[type][ATTRIBUTE_VALUE]);
	return value;
}

/**
 * \brief Create a new node appended to the cur node.
 *
 * This function will search through the original input file
 * and create a replacement node in the XML document.
 *
 */
xmlNodePtr Util_XML_Parse_Create_Node(FILE *infile, char *fileName, xmlNodePtr cur, char *valueName, char *comment, char *units, int type,
		char *recommendedValue) {
	char ch, buf[STRLEN], seps[] = " ,\t\n";
	char *varstr, *valuestr, *unitstr;
	int buflen = STRLEN;
	xmlNodePtr returnNode;
	char tempBuf[STRLEN];
	char *unitTypes[] =  { "us", "usec",  "ms",  "msec",  "s",  "sec",  "second",  "min", "minute",  "h",  "hr", "hour",  "d",  "day",  "week"};
	int numUnits = 15;
	int i;
	void (*fn)(char *format, ...) = keyprm_errfn;

	/* Loop for all lines */
	fseek(infile, 0L, 0);
	while (fgets(buf, buflen, infile) != NULL) {
		ch = buf[0];
		if ((ch != '#') && (ch != ' ') && (ch != '\n') && (ch != '\t')) {
			/* it's a parameter line */
			varstr = strtok(buf, seps);

			if (strcmp(varstr, valueName) == 0) {


				/* Check there isn't a duplicate value */
				set_keyprm_errfn(quiet);
				if(skipToKeyEndNoRewind(infile,varstr) < 0){
					quit("Util_XML_Parse_Create_Node: Duplicate key found %s\n", varstr);
				}

				valuestr = strtok(NULL, seps);

				/* grab the next token in case its a unit */
				unitstr = strtok(NULL, seps);

				if(unitstr != NULL){
					for(i = 0; i < numUnits; i++){
						if(strcmp(unitstr, unitTypes[i]) == 0){
							sprintf(tempBuf, "%s %s", valuestr, unitstr);
							strcpy(valuestr, tempBuf);
							break;
						}
					}
				}

				if (valuestr == NULL)
					keyprm_errfn("\nUtil_XML_Create_Node: %s parameter not found. Please ensure the format for this parameter is: \n%s value \n", valueName, valueName);
				if (strlen(valuestr) == 0) {
					keyprm_errfn("\nUtil_XML_Create_Node: %s parameter not found. Please ensure the format for this parameter is: \n%s value \n", valueName, valueName);
				}

				/* Create the new node */
				returnNode = Util_XML_Create_Node(ATLANTIS_ATTRIBUTE, cur, valueName, comment, units, valuestr);

				Util_XML_Add_Node_Property(returnNode, "AttributeType", AtlantisXMLAttributeTypeStrings[type]);
				Util_XML_Add_Node_Property(returnNode, "AttributeRecommendedValue", recommendedValue);

				set_keyprm_errfn(fn);

				return returnNode;
			}
		}
	}
	keyprm_errfn("Could not find parameter '%s' in input file %s\n\n", valueName, fileName);
	return NULL;
}


/**
 * \brief Add a node to the XML document with the valueName given where the length is on the same line as the valueName but the values
 * are on the next line.
 *
 * This is useful for entries in the prm file like:
 *
 * 	Tchange_start    1
 *	0
 *
 *
 *
 */
xmlNodePtr Util_XML_Create_Node_Next_Line(FILE *infile, char *fileName, xmlNodePtr cur, char *valueName, char *comment, char *units, int type) {
	char ch, buf[2000], seps[] = " ,\t\n";
	char *varstr, *valuestr;
	int buflen = 2000;
	xmlNodePtr returnNode;
	char lengthStr[100];

	/* Loop for all lines */
	fseek(infile, 0L, 0);
	while (fgets(buf, buflen, infile) != NULL) {
		ch = buf[0];
		if ((ch != '#') && (ch != ' ') && (ch != '\n') && (ch != '\t')) {
			/* it's a parameter line */
			varstr = strtok(buf, seps);

			if (strcmp(varstr, valueName) == 0) {

				valuestr = strtok(NULL, seps);
				/* Copy the length string so we can use it later */
				strcpy(lengthStr, valuestr);

				if (valuestr == NULL)
					quit("%s parameter not found. Please ensure the format for this parameter is: \n%s value \n", valueName, valueName);
				if (strlen(valuestr) == 0) {
					quit("%s parameter not found. Please ensure the format for this parameter is: \n%s value \n", valueName, valueName);
				}

				// Get the next line.
				fgets(buf, buflen, infile);

				/* Create the new node */
				returnNode = Util_XML_Create_Node(ATLANTIS_ATTRIBUTE, cur, valueName, comment, units, buf);

				Util_XML_Add_Node_Property(returnNode, "AttributeType", AtlantisXMLAttributeTypeStrings[type]);
				Util_XML_Add_Node_Property(returnNode, "ArrayLength", lengthStr);

				return returnNode;

			}
		}
	}
	quit("ERROR: Could not find parameter '%s' in input file %s\n\n", valueName, fileName);
	return NULL;
}


/**
 * \brief Replace the value in the replaceIndex'th position of the string valueStr with the given replaceStr.
 *
 *
 *
 */
static char *Replace_String_Entry(char *valueStr, char *replaceStr, int size, int replaceIndex) {
	char *varStr;
	char seps[] = " ";
	int i;
	char tempStr[256];
	char dummyString[100] = "0.0";
	char *returnString = (char *) malloc(sizeof(char) * (size_t)(size * 100));

	/* Read in value until size is reached. */
	for (i = 0; i < size; ++i) {

		if (i == 0)
			varStr = strtok(valueStr, seps);
		else
			varStr = strtok(NULL, seps);

		if (varStr == NULL)
			varStr = dummyString;

		if (i == replaceIndex) {
			if (i == 0)
				strcpy(tempStr, replaceStr);
			else
				sprintf(tempStr, "%s %s", returnString, replaceStr);
			strcpy(returnString, tempStr);
		} else {
			if (i == 0)
				strcpy(tempStr, varStr);
			else
				sprintf(tempStr, "%s %s", returnString, varStr);
			strcpy(returnString, tempStr);
		}
	}
	return returnString;
}

/**
 * 	\brief Parse the values in the valueStr and return a newly allocated integer array.
 *
 * Note this function allocates memory and the caller is responsible for freeing this memory.
 *
 *
 *
 */
static int *Parse_Into_Int_Array(char *fileName, char *paramName, char *valueStr, int size, int entry_type) {
	int i;
	int *values = (int *) malloc((size_t)size * sizeof(int));
	char *varStr;
	char seps[] = " ,\t";
	int val;
	int lastval = 0;
	double diff, base;

	/* Read in value until size is reached. */
	for (i = 0; i < size; ++i) {

		if (i == 0)
			varStr = strtok(valueStr, seps);
		else
			varStr = strtok(NULL, seps);

		if (varStr == NULL) {
			if (i == 0) {
				quit("Parse_Into_Int_Array: Can't read any array values for %s in input file %s\n", paramName, fileName);
			} else {
				/* not enough values - fill remainder with last value */
				printf("Parse_Into_Int_Array: Filling to end of array %s with last value, %d\n", paramName, lastval);
				for (; i < size; ++i)
					values[i] = lastval;
				break;
			}
		} else {
			val = atoi(varStr);
			/* A single value was located, so populate the array */
			values[i] = val;
			lastval = val;
		}
		switch (entry_type) {
		case binary_check:
			/* Flag values only (i.e. either 1 or 0 only) */
			if (values[i] != 0.0 && values[i] != 1.0)
				quit("Error: In input file %s, entries in %s are supposed to be flag values (i.e. either 1 or 0), but %d entry is %e\n", fileName, paramName,
						i, values[i]);

			break;
		case integer_check:
			/* Check that integer values are integer */
			base = floor(values[i]);
			diff = fabs(values[i] - base);
			if (diff > 0)
				quit("Error: In input file %s, entries in %s are supposed to be integer values, but %d entry is %e\n", fileName, paramName, i, values[i]);
			break;
		default:
			quit("Parse_Into_Int_Array - Check id for array check must be integer or binary check when reading in integer arrays\n");
			break;
		}
	}

	free(valueStr);
	return values;

}

/**
 * \brief Routine to error check size of array as its read in
 *
 *  entry_type indicates what parameter assertion checks have to be made
 *  0 = none
 *  1 = values are 1 or 0 only (as is a flag)
 *  2 = values are [0,1]
 *  3 = values are [0,1] and sum of values is 1.0
 */
int Util_XML_Read_Array_Integer(int type, char *fileName, xmlNodePtr parent, int entry_type, char *key, int **p, int size) {
	xmlNodePtr attributeNode;
	char *values;

	attributeNode = Util_XML_Get_Node(type, parent, key);
	if (attributeNode == NULL)
		return FALSE;
	values = Util_XML_Get_Node_Value_String(type, attributeNode);
	if (values == NULL)
		return FALSE;

	// Read the data into a temp array.
	*p = Parse_Into_Int_Array(key, fileName, values, size, entry_type);
	return TRUE;
}

//static
double *Parse_Into_Array(char *fileName, char *paramName, char *valueStr, int size, int entry_type) {
	int i;
	double *values = (double *) malloc((size_t)size * sizeof(double));
	char *varStr;
	char seps[] = " ,\t";
	double val, lastval = 0.0;
	char *valueCopy = (char *)malloc(sizeof(char) *STRLEN);

	valueCopy  = strcpy(valueCopy, valueStr);


	varStr = NULL;
	// Read in value until size is reached.
	for (i = 0; i < size; ++i) {

		if (i == 0)
			// Get the first token in the line.
			varStr = strtok(valueStr, seps);
		else
			// Get the first token in the line.
			varStr = strtok(NULL, seps);

		if (varStr == NULL) {
			if (i == 0) {
				quit("Parse_Into_Array: Can't read any array values for %s in input file %s.\n", paramName, fileName);
			} else {
				// not enough values - fill remainder with last value */
				warn("Parse_Into_Array: Filling %d values to end of array %s with last value, %e.\n", (size - i), paramName, lastval);
				for (; i < size; ++i)
					values[i] = (double) lastval;
				break;
			}
		} else {

			if (sscanf(varStr, "%lf", &val) != 1) {

				quit("Error: Reading parameter %s - \nNo numeric values in string %s, value index %d\n", paramName, valueCopy, i);
			}
			// A single value was located, so populate the array */
			values[i] = val;
			lastval = val;
		}
	}
	/* Check we don't have more values than we should */
	if(varStr != NULL){
		varStr = strtok(NULL, seps);
		if(varStr != NULL){
			warn("Parse_Into_Array: Additional values in parameter %s. Array length should be %d.\n", paramName, size);
		}
	}

	//free(valueStr);
	free(valueCopy);
	/* Check the data is of the correct type */
	if (Check_Input_Array(&values, size, entry_type, paramName))
		return values;

	return NULL;
}

/**
 * \brief Routine to error check size of array as its read in
 *
 *  entry_type indicates what parameter assertion checks have to be made
 *  0 = none
 *  1 = values are 1 or 0 only (as is a flag)
 *  2 = values are [0,1]
 *  3 = values are [0,1] and sum of values is 1.0
 */
int Util_XML_Read_Array_Double(int type, char *fileName, char *errorPath, xmlNodePtr parent, int entry_type, char *key, double **p, int size) {
	xmlNodePtr attributeNode;
	char *values;
	char xpath[1000];
	sprintf(xpath, "%s/%s", errorPath, key);

	attributeNode = Util_XML_Get_Node(type, parent, key);
	if (attributeNode == NULL) {
		return FALSE;
	}

	values = Util_XML_Get_Node_Value_String(type, attributeNode);
	if (values == NULL) {
		return FALSE;
	}
	// Read the data into a temp array.
	*p = Parse_Into_Array(fileName, xpath, values, size, entry_type);
	free(values);
	return TRUE;
}


/**
 * \brief Read in string array.
 *
 */
int Util_XML_Read_Array_String(int type, char *fileName, char *errorPath, xmlNodePtr parent, char *key, char ***p, int size) {
	xmlNodePtr attributeNode;
	char *values;
	char xpath[1000];
	char seps[] = " ";
	int i;
	char *varStr;

	*p = (char **) c_alloc2d(STRLEN, size);

	sprintf(xpath, "%s/%s", errorPath, key);

	attributeNode = Util_XML_Get_Node(type, parent, key);
	if (attributeNode == NULL) {
		return FALSE;
	}

	values = Util_XML_Get_Node_Value_String(type, attributeNode);
	if (values == NULL) {
		return FALSE;
	}

	// Read the data into a temp array.
	for (i = 0; i < size; ++i) {
		if (i == 0)
			// Get the first token in the line.
			varStr = strtok(values, seps);
		else
			// Get the first token in the line.
			varStr = strtok(NULL, seps);

		strcpy((*p)[i], varStr);
	}
	free(values);
	return TRUE;
}


int Util_XML_Get_Param_Index(SpeciesParamStruct array[], int size, int paramID) {
	int index;

	for (index = 0; index < size; index++) {
		if (array[index].paramID == paramID)
			return index;
	}
	quit("Util_XML_Get_Param_Index - paramID %d not found.", paramID);
	return FALSE;
}

double Util_XML_Get_Species_Node_Value(xmlNodePtr parent, int paramID, int guild, SpeciesParamStruct *paramStruct) {

	int result;
	double value = -1;
	xmlNodePtr attributeNode;

	attributeNode = Util_XML_Get_Node(ATLANTIS_GROUP_ATTRIBUTE, parent, FunctGroupArray[guild].groupCode);

	if (attributeNode == NULL)
		quit("Util_XML_Get_Species_Node_Value. Error: Cannot find parameter %s/%s in file %s.\n", paramStruct[paramID].tag, FunctGroupArray[guild].groupCode,
				parent->doc->URL);

	/* Sucessfully found parameter - convert to double, check and return.*/
	if(( result = Util_XML_Get_Node_Value_Double(ATLANTIS_ATTRIBUTE, attributeNode, &value)) < 0){
		quit("Util_XML_Get_Species_Node_Value. Error: Cannot find parameter %s/%s in file %s.\n", paramStruct[paramID].tag, FunctGroupArray[guild].groupCode,
						parent->doc->URL);
	}
	else{
		if(result == -1)
			quit("Cannot find value %s for group %s\n", paramStruct[paramID].tag, FunctGroupArray[guild].groupCode);

		value = value / paramStruct[paramID].divider;
	}

	if (!(_finite(value)) || !(_finite(value))) {
		quit("Util_XML_Get_Species_Node_Value value is undefined for group %s value = %e", FunctGroupArray[guild].groupCode, value);
	}
	if (verbose > 2)
		printf("Ecosystem parameter: %s = %e\n", FunctGroupArray[guild].groupCode, value);
	Check_Value(FunctGroupArray[guild].groupCode, value, paramStruct[paramID].checkType, paramStruct[paramID].tag);

	return value;
}



/**
 * \brief Read in the species parameters from the given file.
 *
 */
void Util_XML_Read_Species_Param(MSEBoxModel *bm, char *fileName, xmlNodePtr parent, int paramID) {
	int guild;
	xmlNodePtr attributeGroup;
	int index;

	/* Get the index of this paramID in the speciesParamStructArray structure - done this way so the
	 * values in speciesParamStructArray don't have to be in any particular order
	 */
	index = Util_XML_Get_Param_Index(speciesParamStructArray, tot_prms, paramID);

	if (verbose > 2)
		printf("Read species parameters %s\n", speciesParamStructArray[index].tag);

	attributeGroup = Util_XML_Get_Node(ATLANTIS_ATTRIBUTE, parent, speciesParamStructArray[index].tag);
	if (attributeGroup == NULL)
		quit("%s/%s attribute group not found in file %s.\n", parent->name, speciesParamStructArray[index].tag, fileName);

	switch (speciesParamStructArray[index].paramType) {

	/* All functional groups */
		case SP_TURNED_ON:
			for (guild = 0; guild < bm->K_num_tot_sp; guild++)
				FunctGroupArray[guild].speciesParams[paramID] = Util_XML_Get_Species_Node_Value(attributeGroup, index, guild, speciesParamStructArray);

			break;

		/* All functional groups */
		case SP_ALL:
			for (guild = 0; guild < bm->K_num_tot_sp; guild++)
				if (FunctGroupArray[guild].speciesParams[flag_id] == TRUE) {
					FunctGroupArray[guild].speciesParams[paramID] = Util_XML_Get_Species_Node_Value(attributeGroup, index, guild, speciesParamStructArray);
				}

			break;
		case SP_NOT_PP:
			for (guild = 0; guild < bm->K_num_tot_sp; guild++) {
				if ((FunctGroupArray[guild].isPrimaryProducer == FALSE) && (FunctGroupArray[guild].isDetritus == FALSE)) {
					FunctGroupArray[guild].speciesParams[paramID] = Util_XML_Get_Species_Node_Value(attributeGroup, index, guild, speciesParamStructArray);

				}
			}
			break;

		case SP_Q10:
			for (guild = 0; guild < bm->K_num_tot_sp; guild++)
				if (FunctGroupArray[guild].speciesParams[q10_method_id])
					FunctGroupArray[guild].speciesParams[paramID] = Util_XML_Get_Species_Node_Value(attributeGroup, index, guild, speciesParamStructArray);

			break;

		/* Deal with all the vertebrate only params first */
		case SP_VERTS:
			for (guild = 0; guild < bm->K_num_tot_sp; guild++)
				if ((FunctGroupArray[guild].speciesParams[flag_id] == TRUE) && (FunctGroupArray[guild].isVertebrate == TRUE))
					FunctGroupArray[guild].speciesParams[paramID] = Util_XML_Get_Species_Node_Value(attributeGroup, index, guild, speciesParamStructArray);
			break;

		/* Home range movers */
		case SP_VERTS_HOME_RANGE:
			for (guild = 0; guild < bm->K_num_tot_sp; guild++)
				if((FunctGroupArray[guild].speciesParams[flag_id] == TRUE) && (FunctGroupArray[guild].isVertebrate == TRUE)
								&& ((int) (FunctGroupArray[guild].speciesParams[ddepend_move_id]) == homerange_move)) {
					FunctGroupArray[guild].speciesParams[paramID] = Util_XML_Get_Species_Node_Value(attributeGroup, index, guild, speciesParamStructArray);
				}

			break;

		/* Vertebrates and age structured biomass groups */
		case SP_AGE_STRUCTURED:
			for (guild = 0; guild < bm->K_num_tot_sp; guild++)
				if (FunctGroupArray[guild].speciesParams[flag_id] == TRUE) {
					if ((FunctGroupArray[guild].groupAgeType == AGE_STRUCTURED) || (FunctGroupArray[guild].groupAgeType == AGE_STRUCTURED_BIOMASS))
						FunctGroupArray[guild].speciesParams[paramID] = Util_XML_Get_Species_Node_Value(attributeGroup, index, guild, speciesParamStructArray);
				}
			break;

			/* Age structured biomass groups */
		case SP_AGE_STRUCTURED_BIOMASS:
			for (guild = 0; guild < bm->K_num_tot_sp; guild++)
				if (FunctGroupArray[guild].speciesParams[flag_id] == TRUE) {
					if (FunctGroupArray[guild].groupAgeType == AGE_STRUCTURED_BIOMASS)
						FunctGroupArray[guild].speciesParams[paramID] = Util_XML_Get_Species_Node_Value(attributeGroup, index, guild, speciesParamStructArray);
				}
			break;


			/* Predators paramaters*/
		case SP_PREDATOR:
			for (guild = 0; guild < bm->K_num_tot_sp; guild++)
				if (FunctGroupArray[guild].speciesParams[flag_id] == TRUE) {
					if (FunctGroupArray[guild].isPredator == TRUE) {
						FunctGroupArray[guild].speciesParams[paramID] = Util_XML_Get_Species_Node_Value(attributeGroup, index, guild, speciesParamStructArray);
					}
				}
			break;

		/* Everything but detritus - which is everywhere */
		case SP_NOT_DET:
			for (guild = 0; guild < bm->K_num_tot_sp; guild++)
				if (FunctGroupArray[guild].speciesParams[flag_id] == TRUE) {
					if ((FunctGroupArray[guild].groupType != REF_DET) && (FunctGroupArray[guild].groupType != LAB_DET)
							&& (FunctGroupArray[guild].groupType != CARRION)) {
						FunctGroupArray[guild].speciesParams[paramID] = Util_XML_Get_Species_Node_Value(attributeGroup, index, guild, speciesParamStructArray);
					}
				}
			break;


		/* Invertebrate predators paramaters*/
		case SP_INVERT_PREDATORS:
			for (guild = 0; guild < bm->K_num_tot_sp; guild++)
				if (FunctGroupArray[guild].speciesParams[flag_id] == TRUE) {
					if ((FunctGroupArray[guild].isVertebrate == FALSE) && (FunctGroupArray[guild].isPredator == TRUE)) {
						if(paramID == ht_id){
							FunctGroupArray[guild].speciesParams[paramID] = Util_XML_Get_Species_Node_Value(attributeGroup, index, guild, speciesParamStructArray) * 86400.0;
						}else{
							FunctGroupArray[guild].speciesParams[paramID] = Util_XML_Get_Species_Node_Value(attributeGroup, index, guild, speciesParamStructArray);
						}
					}
				}
			break;

		case SP_AGE_DIET:
			for (guild = 0; guild < bm->K_num_tot_sp; guild++) {
				if (FunctGroupArray[guild].speciesParams[flag_id] == TRUE) {
					if ((FunctGroupArray[guild].groupType == LG_ZOO) || (FunctGroupArray[guild].groupType == MED_ZOO)
							|| (FunctGroupArray[guild].groupType == SM_ZOO) || (FunctGroupArray[guild].groupType == LG_INF)
							|| (FunctGroupArray[guild].groupType == SED_EP_FF) || (FunctGroupArray[guild].groupType == SED_EP_OTHER)
							|| (FunctGroupArray[guild].groupType == MOB_EP_OTHER))
						FunctGroupArray[guild].speciesParams[paramID] = Util_XML_Get_Species_Node_Value(attributeGroup, index, guild, speciesParamStructArray);
				}
			}
			break;

		case SP_HORIZONATAL_MOVE:
			for (guild = 0; guild < bm->K_num_tot_sp; guild++)
				if (FunctGroupArray[guild].speciesParams[flag_id] == TRUE) {
					if (FunctGroupArray[guild].sp_geo_move == TRUE)
						FunctGroupArray[guild].speciesParams[paramID] = Util_XML_Get_Species_Node_Value(attributeGroup, index, guild, speciesParamStructArray);
				}
			break;

		case SP_BIOTURB:
			for (guild = 0; guild < bm->K_num_tot_sp; guild++)
				if (FunctGroupArray[guild].speciesParams[flag_id] == TRUE) {
					if ((FunctGroupArray[guild].groupType == LG_INF) || (FunctGroupArray[guild].groupType == MOB_EP_OTHER)
							|| (FunctGroupArray[guild].groupType == SED_EP_OTHER))
						FunctGroupArray[guild].speciesParams[paramID] = Util_XML_Get_Species_Node_Value(attributeGroup, index, guild, speciesParamStructArray);
				}
			break;
		case SP_INF:
			for (guild = 0; guild < bm->K_num_tot_sp; guild++)
				if (FunctGroupArray[guild].speciesParams[flag_id] == TRUE) {
					if (FunctGroupArray[guild].isInfauna == TRUE)
						FunctGroupArray[guild].speciesParams[paramID] = Util_XML_Get_Species_Node_Value(attributeGroup, index, guild, speciesParamStructArray);
				}
			break;

		case SP_PP:
			for (guild = 0; guild < bm->K_num_tot_sp; guild++)
				if (FunctGroupArray[guild].speciesParams[flag_id] == TRUE) {
					if (FunctGroupArray[guild].isVertebrate == FALSE) {
						if (FunctGroupArray[guild].isPrimaryProducer == TRUE) {
							FunctGroupArray[guild].speciesParams[paramID] = Util_XML_Get_Species_Node_Value(attributeGroup, index, guild, speciesParamStructArray);
						}
					}
				}
			break;

		case SP_SG:
			for (guild = 0; guild < bm->K_num_tot_sp; guild++)
				if (FunctGroupArray[guild].speciesParams[flag_id] == TRUE) {
					if (FunctGroupArray[guild].groupType == SEAGRASS){
						FunctGroupArray[guild].speciesParams[paramID] = Util_XML_Get_Species_Node_Value(attributeGroup, index, guild, speciesParamStructArray);
					}
				}
			break;

		case SP_KN:
			for (guild = 0; guild < bm->K_num_tot_sp; guild++)
				if (FunctGroupArray[guild].speciesParams[flag_id] == TRUE) {
					if (FunctGroupArray[guild].isVertebrate == FALSE) {
						if ((FunctGroupArray[guild].isPrimaryProducer == TRUE) || (FunctGroupArray[guild].groupType == SED_BACT)) {
							FunctGroupArray[guild].speciesParams[paramID] = Util_XML_Get_Species_Node_Value(attributeGroup, index, guild, speciesParamStructArray);

						}
					}
				}
			break;

		case SP_MICROALGAE:
			for (guild = 0; guild < bm->K_num_tot_sp; guild++)
				if (FunctGroupArray[guild].speciesParams[flag_id] == TRUE) {
					if (FunctGroupArray[guild].isVertebrate == FALSE) {
						if (FunctGroupArray[guild].isPrimaryProducer == TRUE) {
							FunctGroupArray[guild].speciesParams[paramID] = Util_XML_Get_Species_Node_Value(attributeGroup, index, guild, speciesParamStructArray);
						}
					}
				}

			break;

		/* Read in the oxygen values */
		case SP_OXYGEN:
			for (guild = 0; guild < bm->K_num_tot_sp; guild++)
				if (FunctGroupArray[guild].speciesParams[flag_id] == TRUE) {
					if (FunctGroupArray[guild].isVertebrate == FALSE) {
						if ((FunctGroupArray[guild].groupType == SED_BACT) || (FunctGroupArray[guild].groupType == PL_BACT)
								|| (FunctGroupArray[guild].groupType == MOB_EP_OTHER) || (FunctGroupArray[guild].groupType == LG_INF)
								|| (FunctGroupArray[guild].groupType == SM_INF) || (FunctGroupArray[guild].groupType == SED_EP_OTHER)
								|| (FunctGroupArray[guild].groupType == SED_EP_FF) || (FunctGroupArray[guild].groupType == CORAL)
                                || (FunctGroupArray[guild].groupType == SPONGE)) {
							FunctGroupArray[guild].speciesParams[paramID] = Util_XML_Get_Species_Node_Value(attributeGroup, index, guild, speciesParamStructArray);
						}
					}
				}

			break;

		case SP_SED_EP_FF:
			for (guild = 0; guild < bm->K_num_tot_sp; guild++)
				//if (FunctGroupArray[guild].speciesParams[flag_id] == TRUE) {
					if (FunctGroupArray[guild].isVertebrate == FALSE) {
						if ((FunctGroupArray[guild].groupType == SED_EP_FF) || (FunctGroupArray[guild].groupType == CORAL)
                            || (FunctGroupArray[guild].groupType == SPONGE)) {
							FunctGroupArray[guild].speciesParams[paramID] = Util_XML_Get_Species_Node_Value(attributeGroup, index, guild, speciesParamStructArray);

						}
					}
				//}

			break;
		case SP_BASAL:
			for (guild = 0; guild < bm->K_num_tot_sp; guild++)
				//if (FunctGroupArray[guild].speciesParams[flag_id] == TRUE) {
					if (FunctGroupArray[guild].isVertebrate == FALSE) {
                        if (FunctGroupArray[guild].isInfauna == TRUE || FunctGroupArray[guild].isEpiFauna == TRUE ) {
                            
						/*if ((FunctGroupArray[guild].groupType == PHYTOBEN) || (FunctGroupArray[guild].groupType == SEAGRASS)
								|| (FunctGroupArray[guild].groupType == MICROPHTYBENTHOS) || (FunctGroupArray[guild].groupType == SED_EP_FF)
								|| (FunctGroupArray[guild].groupType == SED_EP_OTHER) || (FunctGroupArray[guild].groupType == LG_INF)
								|| (FunctGroupArray[guild].groupType == SM_INF) || (FunctGroupArray[guild].groupType == TURF)
								|| (FunctGroupArray[guild].groupType == CORAL) || (FunctGroupArray[guild].groupType == SPONGE)
                                || (FunctGroupArray[guild].groupType == MOB_EP_OTHER)) {
                         */
							FunctGroupArray[guild].speciesParams[paramID] = Util_XML_Get_Species_Node_Value(attributeGroup, index, guild, speciesParamStructArray);
						}
					}
				//}
			break;

		case SP_MACROPHYTE:
			for (guild = 0; guild < bm->K_num_tot_sp; guild++) {
				if (FunctGroupArray[guild].speciesParams[flag_id] == TRUE) {
					if ((FunctGroupArray[guild].groupType == PHYTOBEN) || (FunctGroupArray[guild].groupType == SEAGRASS)
							|| (FunctGroupArray[guild].groupType == TURF)) {
						FunctGroupArray[guild].speciesParams[paramID] = Util_XML_Get_Species_Node_Value(attributeGroup, index, guild, speciesParamStructArray);
					}
				}
			}
			break;
		case SP_MOVEMENT:
			for (guild = 0; guild < bm->K_num_tot_sp; guild++) {
				if (FunctGroupArray[guild].speciesParams[flag_id] == TRUE) {
					if ((FunctGroupArray[guild].groupType == SED_EP_FF) || (FunctGroupArray[guild].groupType == MOB_EP_OTHER)
							|| (FunctGroupArray[guild].sp_geo_move == TRUE)) {
						FunctGroupArray[guild].speciesParams[paramID] = Util_XML_Get_Species_Node_Value(attributeGroup, index, guild, speciesParamStructArray);
					}
				}
			}
			break;
			/* Biomass groups threshhold and damping values - for now just present for BB but could be added for other groups */
		case SP_SED_BACT:
			for (guild = 0; guild < bm->K_num_tot_sp; guild++) {
				if ((FunctGroupArray[guild].speciesParams[flag_id] == TRUE) && (FunctGroupArray[guild].groupType == SED_BACT)) {
					FunctGroupArray[guild].speciesParams[paramID] = Util_XML_Get_Species_Node_Value(attributeGroup, index, guild, speciesParamStructArray);
				}
			}
			break;

		case SP_POP_LIM:
			for (guild = 0; guild < bm->K_num_tot_sp; guild++) {
				//if (FunctGroupArray[guild].speciesParams[flag_id] == TRUE) {
					if((FunctGroupArray[guild].isInfauna == TRUE) || (FunctGroupArray[guild].groupType == SED_EP_OTHER)
							|| (FunctGroupArray[guild].groupType == SED_EP_FF) || (FunctGroupArray[guild].groupType == CORAL)
                            || (FunctGroupArray[guild].groupType == PHYTOBEN) || (FunctGroupArray[guild].groupType == SEAGRASS)
                            || (FunctGroupArray[guild].groupType == TURF)) {
						FunctGroupArray[guild].speciesParams[paramID] = Util_XML_Get_Species_Node_Value(attributeGroup, index, guild, speciesParamStructArray);
					}
				//}
			}
			break;
			/* The fishing input variables */
		case SP_FISHED:
			for (guild = 0; guild < bm->K_num_tot_sp; guild++) {
				if (FunctGroupArray[guild].speciesParams[flag_id] == TRUE) {
					if (FunctGroupArray[guild].isFished == TRUE) {
						FunctGroupArray[guild].speciesParams[paramID] = Util_XML_Get_Species_Node_Value(attributeGroup, index, guild, speciesParamStructArray);
					}
				}
			}
			break;

			/* Impacted species variables */
		case SP_IMPACTED:
			for (guild = 0; guild < bm->K_num_tot_sp; guild++) {
				if (FunctGroupArray[guild].isImpacted == TRUE) {
					FunctGroupArray[guild].speciesParams[paramID] = Util_XML_Get_Species_Node_Value(attributeGroup, index, guild, speciesParamStructArray);
				}
			}
			break;

		case SP_OVERWINTER:
			for (guild = 0; guild < bm->K_num_tot_sp; guild++) {
				if (FunctGroupArray[guild].isOverWinter == TRUE) {
					FunctGroupArray[guild].speciesParams[paramID] = Util_XML_Get_Species_Node_Value(attributeGroup, index, guild, speciesParamStructArray);
				}
			}
			break;
		case SP_PREDATOR_OR_BACT:
			for (guild = 0; guild < bm->K_num_tot_sp; guild++) {
				if (FunctGroupArray[guild].isPredator == TRUE || FunctGroupArray[guild].isBacteria == TRUE ){
					FunctGroupArray[guild].speciesParams[paramID] = Util_XML_Get_Species_Node_Value(attributeGroup, index, guild, speciesParamStructArray);
				}
			}
			break;
		case SP_CORAL:
			for (guild = 0; guild < bm->K_num_tot_sp; guild++) {
				if ((FunctGroupArray[guild].groupType == CORAL) || (FunctGroupArray[guild].groupType == SPONGE)) {
					FunctGroupArray[guild].speciesParams[paramID] = Util_XML_Get_Species_Node_Value(attributeGroup, index, guild, speciesParamStructArray);
				}
			}
			break;
        case SP_SPONGE:
            for (guild = 0; guild < bm->K_num_tot_sp; guild++) {
                if (FunctGroupArray[guild].groupType == SPONGE) {
                    FunctGroupArray[guild].speciesParams[paramID] = Util_XML_Get_Species_Node_Value(attributeGroup, index, guild, speciesParamStructArray);
                }
            }
            break;
        case SP_CULTURED:
			for (guild = 0; guild < bm->K_num_tot_sp; guild++) {
				if (FunctGroupArray[guild].isCultured == TRUE) {
					FunctGroupArray[guild].speciesParams[paramID] = Util_XML_Get_Species_Node_Value(attributeGroup, index, guild, speciesParamStructArray);
				}
			}
			break;
        case SP_FED:
            for (guild = 0; guild < bm->K_num_tot_sp; guild++) {
                if ((FunctGroupArray[guild].isCultured == TRUE) || (FunctGroupArray[guild].isSupplemented == TRUE)){
                    FunctGroupArray[guild].speciesParams[paramID] = Util_XML_Get_Species_Node_Value(attributeGroup, index, guild, speciesParamStructArray);
                }
            }
            break;
        case SP_RUGOSITY:
 			for (guild = 0; guild < bm->K_num_tot_sp; guild++)
				//if (FunctGroupArray[guild].speciesParams[flag_id] == TRUE) {
                if (FunctGroupArray[guild].isVertebrate == FALSE) {
                    if ((FunctGroupArray[guild].groupType == PHYTOBEN) || (FunctGroupArray[guild].groupType == SEAGRASS)
                        || (FunctGroupArray[guild].groupType == SED_EP_FF) || (FunctGroupArray[guild].groupType == TURF)
                        || (FunctGroupArray[guild].groupType == CORAL) || (FunctGroupArray[guild].groupType == SPONGE)) {
                            FunctGroupArray[guild].speciesParams[paramID] = Util_XML_Get_Species_Node_Value(attributeGroup, index, guild, speciesParamStructArray);
                    }
                }
            //}
			break;
        case SP_POLLUTE_IMPACTED:
            for (guild = 0; guild < bm->K_num_tot_sp; guild++) {
                if ((FunctGroupArray[guild].isLightEffected == TRUE) || (FunctGroupArray[guild].isNoiseEffected == TRUE))
                    FunctGroupArray[guild].speciesParams[paramID] = Util_XML_Get_Species_Node_Value(attributeGroup, index, guild, speciesParamStructArray);
                }
            break;

        case NOT_USED:	/* Intentional follow through */
		default:
			quit("Util_XML_Read_Species_Param: paramID %s is not found. Add to the switch statement in Util_XML_Read_Species_Param.\n",
					speciesParamStructArray[index].tag);
			break;
	}

}

/*
 * \brief Grab the name of the given node. Used to print out useful error messages to users if nodes are missing.
 *
 */
char *Util_Get_Node_Name(xmlNodePtr node){
	int i;

	for(i = 0; i < NUM_ATLANTIS_NODE_TYPES; i++){
		if(strcmp((char *)node->name, AtlantisXMLObjectNAMES[i][0]))
			return Get_Node_Property(node, AtlantisXMLObjectNAMES[i][ATTRIBUTE_NAME]);
	}
	return (char *)node->name;
}


/** Skip forward from the current file position to
  * the next line beginning with key, positioned at the character
  * immediately after the key.
  *
  * @param fp pointer to stdio FILE structure.
  * @param key keyname to locate in file.
  * @return non-zero if successful.
  */
int skipToKeyEndNoRewind(FILE *fp, char *key)
{
    int i;
    int len = (int)strlen(key);
    long fpos;
    char *s;
    char buf[MAXLINELEN];

    do {
		fpos = ftell(fp);
		s = fgets(buf,MAXLINELEN,fp);

		// If we have got to the end of the file return no find.
		if( s==NULL)
			return 0;

		/* Truncate the string at the first space after the key length.
		*/
		for (i=len; s && i<((int)strlen(buf)); ++i)
			if (isspace(buf[i])) {
				buf[i]='\000';
				break;
		}
	}
    while( s && strcmp(key,buf) != 0 );

//
//    if( s == NULL ) {
//		quit("skipToKeyEnd: key %s not found\n",key);
//		return(1);
//    }
//
    /* seek to character after key */
    if( fseek(fp,fpos+len,0) < 0 )
		return(0);


    //quit("skipToKeyEndNoRewind: Duplicate key found %s\n", key);


    return(1);
}

/**
 * Read the given fileName in the destFolder.
 *
 */
xmlDocPtr xmlReadFileDestFolder(char *destFolder, char *fileName,  const char *encoding, int options){
	char finalFile[BMSLEN];

	sprintf(finalFile, "%s%s", destFolder, fileName);
	return xmlReadFile(finalFile, encoding, options);

}
