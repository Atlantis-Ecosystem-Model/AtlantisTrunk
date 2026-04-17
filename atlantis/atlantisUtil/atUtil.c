/*******************************************************************//**
 \ingroup atUtil

 File:           atlantisutil.c

 Created:        21/9/2004

 Author:         Beth Fulton
 CSIRO Division of Marine Resaech

 Purpose:        Utility routines for atlantis mse box model


 Revisions:      2/1/2008 Moved all code out of atlantismain except the main function.


 15-Feb_2008 Bec Gorton

 Fixed a bug in the initialisation of the bm->regionalData array.
 It was indexing the array as

 bm->RegionalData[i][b][sp] = 1.0;

 and it should be

 bm->RegionalData[sp][b][i] = 1.0;

 26-05-2008 Bec Gorton
 Changed all references from bm->VERTind to the tracer arrays associated with
 each functional group.

 14-06-2008 Bec Gorton
 Updated code to dynamically build the bm->dinfo array instead of
 reading in a blank file on start up.

 23-12-2008 Resized the following arrays:

 bm->mS = Util_Alloc_Init_3D_Double(2, 4, bm->K_num_vert_sp, 0.0); to

 bm->mS = Util_Alloc_Init_2D_Double(4, bm->K_num_vert_sp, 0.0); As the implicit
 mortality due to seabirds and fishing is now being combined into a single parameter.

 The age diet availability of invertebrates for consumption by vertebrates
 bm->pSPageeat = Util_Alloc_Init_3D_Double(Not_age_specific_id, bm->K_num_max_cohort, bm->K_num_vert_sp, 0.0); to:
 bm->pSPageeat = Util_Alloc_Init_3D_Double(bm->K_num_tot_sp, bm->K_num_max_cohort, bm->K_num_tot_sp, 0.0);
 This means this array can be referenced by the groupCode instead of the AGE_PREY_id values.

 02-02-2009 Bec Gorton
 Moved the freeing of the economic arrays in the bm structure into the Economic_Free
 function in this file instead of them being freed in the shutdownmodel function. This
 means that we can check to see if the economic module is on - if not these
 arrays should not be freed.
 Also added checks before calling freeassess and freeeconomic so these are
 only called if these modules are on in the model.

 02-04-2009 Bec Gorton
 Moved the function call that frees up the box geometry to after the call
 that frees up the eoclogy structure. The ecology structure now needs to
 free up the home range memory allocated in setupHomeRanges. A new function
 has been added to atbiolsetup.c that will free this up. This needs to be
 called before the code to free up the boxes.


 22-04-2009 Bec Gorton
 Removed the bm->fisheryParamNAME and SP_FISHERYprmsName arrays.
 Moved all the IO related functions to atIOUtil.c

 10-06-2009 Bec Gorton
 Added support for rolling log files. The log file will be closed when it gets larger
 than MAX_LOG_FILE_SIZE and a new one will be opened. The log file names are simply logD where D
 starts at 1 and is incremented when a new log file is opened.


 22-May-2009 Bec Gorton
 Changed the ncopen calls to use sjw_ncopen. This function checks that the
 netcdf file exits before calling ncopen which will crash if the file is not
 found.


 07-07-2009 Bec Gorton
 Moved the speciesParamStrings into this file - these are now setup in the atlantisUtil
 lib so they can be used in the fishery and management libs.
 Also started adding support for the fishery definition file to be read in. So there is now
 an additional command line argument - the fishery definition file. Useage now includes:
 -p fisheries.csv

 28-10-2009 Bec Gorton
 Changed array sizes and references to get rid of the K_num_fished_sp, K_num_impacted_sp
 and K_num_fishedTac references.

 30-10-2009 Bec Gorton
 Added the Util_Close_Output_File function.

 03-11-2009 Bec Gorton
 Removed the SPtoCATid array as these values are no longer required

 04-11-2009 Bec Gorton
 Merged in Beths bycatch incentive code - revision 961.

 05-11-2009 Bec Gorton
 Trunk merge 1064. Added support for the Q10 parameter to be specified for each functional group.

 14-12-2009 Bec Gorton
 Trunk merge 1424. Added the calcMLinearMort, calcMQuadMort and calcMPredMort arrays to store different mortality values.


 15-12-2009 Bec Gorton
 Trunk merge 1439 - Added the code to read in the clamlinkage input file.

 19-01-2010 Bec Gorton
 Removed the bm->PREYid array as its no longer used!
 Updated the headers that are included.
 Removed the code that read in the fstatistic_blank.nc netcdf file. This is replaced
 with a call to the harvest library to build the tracers as the biology tracers are now created.
 Moved the Distance_to_Port call into the Manage_init function so it can be private to the
 management library.
 Deleted the bm->SPnid2guild, bm->SPmove2guild and bm->PREYid arrays.
 The FFCDR array has been moved out of the bm and into the harvest library,
 ( as have the bm->tscatchid, bm->tsdiscardid, bm->harvestindx, bm->harvestindxNAME and bm->TotCumCatch variables).

 29-01-2010 Bec Gorton
 Removed the bm->fisheryName array. How using the FisheryArray[nf].fisheryCode.

 16-02-2010 Bec Gorton
 Added the Util_Setup_Species_Param_Strings function to this library - it was in the biology lib.

 25-02-2010 Bec Gorton
 Moved the model setup code back into the atlantis main module. We no longer need to use the test interface that
 meant they had to separate.

 26-02-2010 Bec Gorton
 Moved the functions that were defined in the atlog.h header file:
 Util_Logx_Result
 Util_Lognorm_Distrib
 Util_Mich_Ment
 out of the eoclogy lib into the util library and deleted the atlog.h file from the repository/.


 04-05-2010 Bec Gorton
 Added new functions to get calculate the scaling factor from the given change parameter arrays.

 05-05-2010 Bec Gorton
 Added code to Util_Setup_Species_Param_Strings() that will set up the paramString values
 for the new Q10 algorithm.

 There is now an additional method of implementing temperature correction based on work by Gary G.
 The new parameters to implement this are now read in from the input file. These are not compulsory unless the
 q10_method_id method is set to 1 in which case ecol_readSpeciesParam will display an error message and quit.

 *********************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <math.h>
#include <ctype.h>
#include <time.h>
#include <string.h>
#include <sjwlib.h>
#include <netcdf.h>
#include <sys/stat.h>
#include <atlantisboxmodel.h>
#include <atUtilLib.h>
#include <atEcologyLib.h>
#include <atHarvestLib.h>
#include <atManageLib.h>
#include <atImplementationLib.h>


char **paramStrings;
char **cohortParamStrings;
char **spawnParamStrings;
char **RBCParamStrings;

char *cohortStrings[] =
	{ "juv", "adult" };

char *sexStrings[] =
    { "F", "M" };

void Util_Free(MSEBoxModel *bm){
	Free_Fishery_Def_Memory(bm);
	Free_Functional_Group_Memory(bm);
}
/**
 *	\brief These functions are to get around the issue that the economic input files
 *	use 'recfish' for the recfish_id parameter but the manage input files use 'REC'.
 *
 */

char *Util_Get_Fishery_Name(MSEBoxModel *bm, int fisheryIndex) {
//	if (fisheryIndex == recfish_id)
//		return "REC";
	return FisheryArray[fisheryIndex].fisheryCode;
}

/**
 * \brief Close the given output file if the given pointer is not null.
 */
void Util_Close_Output_File(FILE *fp) {
	if (fp != NULL){
		fflush(fp);
		fclose(fp);
	}
}

/**
 * \brief Return the index of the given fishery code.
 *
 * This function will quit if the given string is not recognised.
 *
 *
 */
int Util_Get_Fishery_Index(MSEBoxModel *bm, char *strPtr) {
	int returnValue = -1;
	int nf;
//
//	if (strcmp(strPtr, "REC") == 0)
//		return recfish_id;
//	else {
		for (nf = 0; nf < bm->K_num_fisheries; nf++) {
			if (strcmp(strPtr, FisheryArray[nf].fisheryCode) == 0)
				return nf;
		}
	//}

	if (returnValue == -1)
		quit("Util_Get_Fishery_Index - fishery code '%s' not recognised", strPtr);

	return returnValue;
}

/**
 *	\brief Setup the array of species parameter strings.
 *
 *	These are used to read in values from the input file.
 *
 */
void Util_Setup_Species_Param_Strings(MSEBoxModel *bm) {
    int paramlen = 25;
    int paramlenLong = 50;
    
	// Need to move this into a seperate function.
	paramStrings = (char **) c_alloc2d(paramlen, tot_prms);
	cohortParamStrings = (char **) c_alloc2d(paramlen, cohortDepParams);
	spawnParamStrings = (char **) c_alloc2d(paramlen, spawnDepParams);

	snprintf(paramStrings[flag_id], paramlen, "%s", "flag");
	snprintf(paramStrings[flagdem_id], paramlen, "%s", "flagdem");
	snprintf(paramStrings[flagplankfish_id], paramlen, "%s", "flagplankfish");
	snprintf(paramStrings[flagrecruit_id], paramlen, "%s", "flagrecruit");
	snprintf(paramStrings[flagrecpeak_id], paramlen, "%s", "flagrecpeak");
	//snprintf(paramStrings[flaglocalrecruit_id], paramlen, "%s", "flaglocalrecruit");
	snprintf(paramStrings[flagbearlive_id], paramlen, "%s", "flagbearlive");
	snprintf(paramStrings[flagmother_id], paramlen, "%s", "flagmother");
	snprintf(paramStrings[feed_while_spawn_id], paramlen, "%s", "feed_while_spawn");
	snprintf(paramStrings[flagstocking_id], paramlen, "%s", "flagstocking");
    snprintf(paramStrings[flagkeep_plusgroup_id], paramlen, "%s", "flagkeep_plusgroup");
	
    snprintf(paramStrings[flagq10receff_id], paramlen, "%s", "flagq10receff");
	snprintf(paramStrings[flagq10eff_id], paramlen, "%s", "flagq10eff");
	snprintf(paramStrings[q10_id], paramlen, "%s", "q10_");
	snprintf(paramStrings[q10_method_id], paramlen, "%s", "q10_method_");
    snprintf(paramStrings[q10_optimal_temp_id], paramlen, "%s", "q10_optimal_temp_");
	snprintf(paramStrings[q10_correction_id], paramlen, "%s", "q10_correction_");
	snprintf(paramStrings[optimal_pH_id], paramlen, "%s", "optimal_pH_");
	snprintf(paramStrings[salt_correction_id], paramlen, "%s", "salt_correction_");
	snprintf(paramStrings[pH_correction_id], paramlen, "%s", "pH_correction_");
	snprintf(paramStrings[flagtempsensitive_id], paramlen, "%s", "flagtempsensitive");
	snprintf(paramStrings[temp_coefftA_id], paramlen, "%s", "temp_coefftA_");
	snprintf(paramStrings[flagpHsensitive_id], paramlen, "%s", "flagpHsensitive_");
	snprintf(paramStrings[flagfecundsensitive_id], paramlen, "%s", "flagfecundsensitive_");
	snprintf(paramStrings[flagSaltSensitive_id], paramlen, "%s", "flagSaltSensitive_");
	snprintf(paramStrings[flagnutvaleffect_id], paramlen, "%s", "flagnutvaleffect_");
	snprintf(paramStrings[flagpredavaileffect_id], paramlen, "%s", "flagpredavaileffect_");
	snprintf(paramStrings[flagcontract_tol_id], paramlen, "%s", "flagcontract_tol_");
	snprintf(paramStrings[contract_tol_id], paramlen, "%s", "contract_tol_");
	snprintf(paramStrings[KN_pH_id], paramlen, "%s", "KN_pH_");
	snprintf(paramStrings[pH_constA_id], paramlen, "%s", "pH_constA_");
	snprintf(paramStrings[pH_constB_id], paramlen, "%s", "pH_constB_");
    snprintf(paramStrings[pH_constC_id], paramlen, "%s", "pH_constC_");
	snprintf(paramStrings[min_pH_id], paramlen, "%s", "min_pH_");
	snprintf(paramStrings[max_pH_id], paramlen, "%s", "max_pH_");
	snprintf(paramStrings[pHsensitive_model_id], paramlen, "%s", "pHsensitive_model_");
	snprintf(paramStrings[flagchannel_id], paramlen, "%s", "flagchannel");
	snprintf(paramStrings[pHmortstart_id], paramlen, "%s", "pHmortstart_");
	snprintf(paramStrings[pHmortA_id], paramlen, "%s", "pHmortA_");
	snprintf(paramStrings[pHmortB_id], paramlen, "%s", "pHmortB_");
	snprintf(paramStrings[pHmortmid_id], paramlen, "%s", "pHmortmid_");
    
    snprintf(paramStrings[light_coefft_id], paramlen, "%s", "light_coefft_");
    snprintf(paramStrings[noise_coefft_id], paramlen, "%s", "noise_coefft_");

	snprintf(paramStrings[catcheater_id], paramlen, "%s", "catcheater");
    snprintf(paramStrings[flagactive_id], paramlen, "%s", "day");
	snprintf(paramStrings[vla_T15_id], paramlen, "%s", "vla_");

	snprintf(paramStrings[Kcov_juv_id], paramlen, "%s", "Kcov_juv_");
	snprintf(paramStrings[Bcov_juv_id], paramlen, "%s", "Bcov_juv_");
	snprintf(paramStrings[Acov_juv_id], paramlen, "%s", "Acov_juv_");
	snprintf(paramStrings[Kcov_ad_id], paramlen, "%s", "Kcov_ad_");
	snprintf(paramStrings[Bcov_ad_id], paramlen, "%s", "Bcov_ad_");
	snprintf(paramStrings[Acov_ad_id], paramlen, "%s", "Acov_ad_");

	snprintf(paramStrings[KL_id], paramlen, "%s", "KL_");
	snprintf(paramStrings[KU_id], paramlen, "%s", "KU_");
	snprintf(paramStrings[KUP_id], paramlen, "%s", "KUP_");
	snprintf(paramStrings[KLP_id], paramlen, "%s", "KLP_");
	snprintf(paramStrings[Kmax_coefft_id], paramlen, "%s", "Kmax_coefft_");
	snprintf(paramStrings[extra_feed_id], paramlen, "%s", "_extra_feed");
	
	snprintf(paramStrings[KDEP_id], paramlen, "%s", "KDEP_");
	snprintf(paramStrings[KWSR_id], paramlen, "%s", "KWSR_");
	snprintf(paramStrings[KWRR_id], paramlen, "%s", "KWRR_");
	snprintf(paramStrings[recover_start_id], paramlen, "%s", "recover_start_");
	snprintf(paramStrings[recover_mult_id], paramlen, "%s", "recover_mult_");
	snprintf(paramStrings[BHbeta_id], paramlen, "%s", "BHbeta_");
	snprintf(paramStrings[BHalpha_id], paramlen, "%s", "BHalpha_");
	snprintf(paramStrings[Rbeta_id], paramlen, "%s", "Rbeta_");
	snprintf(paramStrings[Ralpha_id], paramlen, "%s", "Ralpha_");
	snprintf(paramStrings[PP_id], paramlen, "%s", "PP_");
	snprintf(paramStrings[hta_id], paramlen, "%s", "hta_");
	snprintf(paramStrings[htb_id], paramlen, "%s", "htb_");
	snprintf(paramStrings[pR_id], paramlen, "%s", "pR_");
	snprintf(paramStrings[prop_spawn_lost_id], paramlen, "%s", "prop_spawn_lost_");
	snprintf(paramStrings[jack_a_id], paramlen, "%s", "jack_a_");
	snprintf(paramStrings[jack_b_id], paramlen, "%s", "jack_b_");
    
    snprintf(paramStrings[prod_alpha_id], paramlen, "%s", "prod_alpha_");
    snprintf(paramStrings[den_depend_beta1_id], paramlen, "%s", "den_depend_beta1_");
    snprintf(paramStrings[den_depend_beta2_id], paramlen, "%s", "den_depend_beta2_");
    snprintf(paramStrings[temp_coefft_id], paramlen, "%s", "temp_coefft_");
    snprintf(paramStrings[rate_coefft_id], paramlen, "%s", "rate_coefft_");
    snprintf(paramStrings[wind_coefft_id], paramlen, "%s", "wind_coefft_");
    snprintf(paramStrings[recruit_var_id], paramlen, "%s", "rec_var_");
    
	snprintf(paramStrings[intersp_depend_recruit_id], paramlen, "%s", "intersp_depend_recruit_");
	snprintf(paramStrings[intersp_depend_sp_id], paramlen, "%s", "intersp_depend_sp_");
	snprintf(paramStrings[intersp_depend_scale_id], paramlen, "%s", "intersp_depend_scale_");
	snprintf(paramStrings[aquacult_fry_id], paramlen, "%s", "_aquacult_fry");

	snprintf(paramStrings[li_a_id], paramlen, "%s", "li_a_");
	snprintf(paramStrings[li_b_id], paramlen, "%s", "li_b_");
    snprintf(paramStrings[linf_id], paramlen, "%s", "linf_");
    snprintf(paramStrings[Kbert_id], paramlen, "%s", "Kbert_");
    snprintf(paramStrings[tzero_id], paramlen, "%s", "tzero_");
    
    snprintf(paramStrings[min_li_mat_id], paramlen, "%s", "min_li_mat_");
	snprintf(paramStrings[KA_id], paramlen, "%s", "KA_");
	snprintf(paramStrings[KB_id], paramlen, "%s", "KB_");

    snprintf(paramStrings[RSmax_id], paramlen, "%s", "RSmax_");
    snprintf(paramStrings[RSmid_id], paramlen, "%s", "RSmid_");
    snprintf(paramStrings[RSslope_id], paramlen, "%s", "RSslope_");
    snprintf(paramStrings[RSprop_id], paramlen, "%s", "RSprop_");
    snprintf(paramStrings[SNcost_id], paramlen, "%s", "SNcost_");
    snprintf(paramStrings[RNcost_id], paramlen, "%s", "RNcost_");
    snprintf(paramStrings[RSstarve_id], paramlen, "%s", "RNstarve_");
    
	snprintf(paramStrings[overwinterStartTofY_id], paramlen, "%s", "overwinterStartTofY_");
	snprintf(paramStrings[overwinterEndTofY_id], paramlen, "%s", "overwinterEndTofY_");
	snprintf(paramStrings[overwinterStartTemp_id], paramlen, "%s", "overwinterStartTemp_");
	snprintf(paramStrings[overwinterEndTemp_id], paramlen, "%s", "overwinterEndTemp_");
	snprintf(paramStrings[crit_mum_id], paramlen, "%s", "crit_mum_");
	snprintf(paramStrings[crit_nut_id], paramlen, "%s", "crit_nut_");
	snprintf(paramStrings[crit_temp_id], paramlen, "%s", "crit_temp_");
	snprintf(paramStrings[encyst_rate_id], paramlen, "%s", "encyst_rate_");
	snprintf(paramStrings[hatch_rate_id], paramlen, "%s", "hatch_rate_");
	snprintf(paramStrings[encyst_period_id], paramlen, "%s", "encyst_period_");
	snprintf(paramStrings[flagencyst_id], paramlen, "%s", "flagencyst_");

	snprintf(paramStrings[max_prop_shift_id], paramlen, "%s", "max_prop_shift_");
	snprintf(paramStrings[inheritance_id], paramlen, "%s", "inheritance_");
	snprintf(paramStrings[trait_variance_id], paramlen, "%s", "trait_variance_");
	snprintf(paramStrings[min_trait_variance_id], paramlen, "%s", "min_trait_variance_");

	snprintf(paramStrings[E1_id], paramlen, "%s", "E_");
	snprintf(paramStrings[E2_id], paramlen, "%s", "EPlant_");
	snprintf(paramStrings[E3_id], paramlen, "%s", "EDL_");
	snprintf(paramStrings[E4_id], paramlen, "%s", "EDR_");

	snprintf(paramStrings[KSPA_id], paramlen, "%s", "KSPA_");
	snprintf(paramStrings[FSP_id], paramlen, "%s", "FSP_");
	snprintf(paramStrings[Recruit_Time_id], paramlen, "%s", "_Recruit_Time");
    
	snprintf(paramStrings[Recruit_Period_id], paramlen, "%s", "Recruit_Period_");
    snprintf(paramStrings[cohort_recruit_entry_id], paramlen, "%s", "_cohort_recruit_entry");    
	snprintf(paramStrings[spawn_period_id], paramlen, "%s", "_spawn_period");
	snprintf(paramStrings[age_mat_id], paramlen, "%s", "_age_mat");
	snprintf(paramStrings[FDMort_id], paramlen, "%s", "FDM_");
	snprintf(paramStrings[FDG_id], paramlen, "%s", "FDG_");

	snprintf(paramStrings[log_mult_id], paramlen, "%s", "_log_mult");
    snprintf(paramStrings[norm_sigma_id], paramlen, "%s", "_norm_sigma");
    snprintf(paramStrings[flag_recruit_stochastic_id], paramlen, "%s", "_flag_recruit_stochastic");
	snprintf(paramStrings[min_spawn_temp_id], paramlen, "%s", "_min_spawn_temp");
	snprintf(paramStrings[max_spawn_temp_id], paramlen, "%s", "_max_spawn_temp");
	snprintf(paramStrings[min_spawn_salt_id], paramlen, "%s", "_min_spawn_salt");
	snprintf(paramStrings[max_spawn_salt_id], paramlen, "%s", "_max_spawn_salt");
	snprintf(paramStrings[min_move_temp_id], paramlen, "%s", "_min_move_temp");
	snprintf(paramStrings[max_move_temp_id], paramlen, "%s", "_max_move_temp");
	snprintf(paramStrings[min_move_salt_id], paramlen, "%s", "_min_move_salt");
	snprintf(paramStrings[max_move_salt_id], paramlen, "%s", "_max_move_salt");
	snprintf(paramStrings[min_O2_id], paramlen, "%s", "_min_O2");
    
    snprintf(paramStrings[K_temp_const_id], paramlen, "%s", "_K_temp_const");
    snprintf(paramStrings[K_salt_const_id], paramlen, "%s", "_K_salt_const");
    snprintf(paramStrings[K_o2_const_id], paramlen, "%s", "_K_o2_const");
    
	snprintf(paramStrings[predcase_id], paramlen, "%s", "predcase_");

	snprintf(paramStrings[ddepend_move_id], paramlen, "%s", "_ddepend_move");

	snprintf(paramStrings[vlb_id], paramlen, "%s", "vlb_");

    snprintf(paramStrings[turbid_refuge_id], paramlen, "%s", "turbid_refuge_");
    
    snprintf(paramStrings[FDGDL_id], paramlen, "%s", "FDGDL_");
	snprintf(paramStrings[FDGDR_id], paramlen, "%s", "FDGDR_");
	snprintf(paramStrings[age_structured_prey_id], paramlen, "%s", "age_structured_prey_");
	snprintf(paramStrings[p_split_id], paramlen, "%s", "p_split_");
	snprintf(paramStrings[KTUR_id], paramlen, "%s", "KTUR_");
	snprintf(paramStrings[KIRR_id], paramlen, "%s", "KIRR_");
	snprintf(paramStrings[vl_id], paramlen, "%s", "vl_");
	snprintf(paramStrings[ht_id], paramlen, "%s", "ht_");
    snprintf(paramStrings[hvm_id], paramlen, "%s", "hvm_");

	snprintf(paramStrings[KN_id], paramlen, "%s", "KN_");
	snprintf(paramStrings[KS_id], paramlen, "%s", "KS_");
	snprintf(paramStrings[KF_id], paramlen, "%s", "KF_");
	snprintf(paramStrings[KP_id], paramlen, "%s", "KP_");
	snprintf(paramStrings[KI_T15_id], paramlen, "%s", "KI_*_T15");
	snprintf(paramStrings[Beta_D_id], paramlen, "%s", "Beta_D_");
	snprintf(paramStrings[PBmax_D_id], paramlen, "%s", "PBmax_D_");
	snprintf(paramStrings[ICE_KDEP_id], paramlen, "%s", "ICE_KDEP_");

	snprintf(paramStrings[KI_L_T15_id], paramlen, "%s", "L_KI_*_T15");
	snprintf(paramStrings[Kext_id], paramlen, "%s", "Kext_");
	snprintf(paramStrings[Ksub_id], paramlen, "%s", "Ksub_");
	snprintf(paramStrings[KNepi_id], paramlen, "%s", "KN_epi_");
	snprintf(paramStrings[KsubEpi_id], paramlen, "%s", "KsubEpi_");
	snprintf(paramStrings[Ktrans_id], paramlen, "%s", "Ktrans_");

		/* Coral and rugosity related parameters */
	snprintf(paramStrings[bleach_periodA_id], paramlen, "%s", "_bleach_periodA");
	snprintf(paramStrings[bleach_periodB_id], paramlen, "%s", "_bleach_periodB");
	snprintf(paramStrings[mBleach_id], paramlen, "%s", "_mBleach");
	snprintf(paramStrings[bleaching_rate_id], paramlen, "%s", "_bleaching_rate");
	snprintf(paramStrings[bleach_recovery_rate_id], paramlen, "%s", "_bleach_recovery_rate");
	snprintf(paramStrings[bleach_tempshift_id], paramlen, "%s", "_bleach_tempshift");
	snprintf(paramStrings[bleach_growshift_id], paramlen, "%s", "_bleach_growshift");
	snprintf(paramStrings[bleach_temp_id], paramlen, "%s", "_bleach_temp");
    snprintf(paramStrings[min_bleach_temp_id], paramlen, "%s", "_min_bleach_temp");
    snprintf(paramStrings[prop_zooxanth_id], paramlen, "%s", "_prop_zooxanth");
	snprintf(paramStrings[DHW_thresh_id], paramlen, "%s", "_DHW_thresh");
    snprintf(paramStrings[threshdepth_id], paramlen, "%s", "_threshdepth_id");
    snprintf(paramStrings[depmum_scalar_id], paramlen, "%s", "_depmum_scalar_id");

    snprintf(paramStrings[min_bleach_salt_id], paramlen, "%s", "_min_bleach_salt");
    snprintf(paramStrings[max_bleach_salt_id], paramlen, "%s", "_max_bleach_salt");
    
    snprintf(paramStrings[rugFeedScalar_id], paramlen, "%s", "_rugFeedScalar");
	snprintf(paramStrings[HostRemin_id], paramlen, "%s", "_HostRemin");
	snprintf(paramStrings[calcifRefBaseline_id], paramlen, "%s", "_calcifRefBaseline");
	snprintf(paramStrings[calcifTconst_id], paramlen, "%s", "_calcifTconst");
	snprintf(paramStrings[calcifTcoefft_id], paramlen, "%s", "_calcifTcoefft");
	snprintf(paramStrings[calcifTopt_id], paramlen, "%s", "_calcifTopt");
	snprintf(paramStrings[calcifLambda_id], paramlen, "%s", "_calcifLambda");
	snprintf(paramStrings[FeedLightThresh_id], paramlen, "%s", "_FeedLightThresh");
	snprintf(paramStrings[PropLightFeed_id], paramlen, "%s", "_PropLightFeed");
	snprintf(paramStrings[rug_erode_id], paramlen, "%s", "_rug_erode");
	snprintf(paramStrings[rug_bleacherode_id], paramlen, "%s", "_rug_bleacherode");
	snprintf(paramStrings[rug_factor_id], paramlen, "%s", "_rug_factor");
	snprintf(paramStrings[colony_ha_id], paramlen, "%s", "_colony_ha");
	snprintf(paramStrings[coral_overgrow_id], paramlen, "%s", "_coral_overgrow");
	snprintf(paramStrings[coral_compete_id], paramlen, "%s", "_coral_compete");
	snprintf(paramStrings[coral_max_accel_trans_id], paramlen, "%s", "_coral_max_accel_trans");
	snprintf(paramStrings[coral_max_accelA_id], paramlen, "%s", "_coral_max_accelA");
	snprintf(paramStrings[coral_max_accelB_id], paramlen, "%s", "_coral_max_accelB");
	snprintf(paramStrings[CrecruitA_id], paramlen, "%s", "_CrecruitA");
	snprintf(paramStrings[CrecruitB_id], paramlen, "%s", "_CrecruitB");
	snprintf(paramStrings[CrecruitC_id], paramlen, "%s", "_CrecruitC");
	snprintf(paramStrings[rec_HabDepend_id], paramlen, "%s", "rec_HabDepend");
	snprintf(paramStrings[RugCover_scalar_id], paramlen, "%s", "_RugCover_scalar");

    snprintf(paramStrings[sponge_overgrow_id], paramlen, "%s", "_sponge_overgrow");
    snprintf(paramStrings[sponge_compete_id], paramlen, "%s", "_sponge_compete");
    snprintf(paramStrings[Ksmother_A_id], paramlen, "%s", "_Ksmother_A");
    snprintf(paramStrings[Ksmother_B_id], paramlen, "%s", "_Ksmother_B");
    snprintf(paramStrings[Vmax_deltaSi_id], paramlen, "%s", "_Vmax_deltaSi");
    snprintf(paramStrings[Km_deltaSi_id], paramlen, "%s", "_Km_deltaSi");
    snprintf(paramStrings[rug_erode_sponge_id], paramlen, "%s", "_rug_erode_sponge");
    
    snprintf(cohortParamStrings[rugosity_inc_id], paramlen, "%s", "_rugosity_inc");
	snprintf(cohortParamStrings[rugosity_dec_id], paramlen, "%s", "_rugosity_dec");
	snprintf(cohortParamStrings[colony_diam_id], paramlen, "%s", "_colony_diam");

	/* Primary producer P variables */
	snprintf(paramStrings[P_max_uptake_id], paramlen, "%s", "P_uptake_");
	snprintf(paramStrings[P_uptake_scale_id], paramlen, "%s", "P_scale_uptake_");
	snprintf(paramStrings[P_concp_id], paramlen, "%s", "P_concp_");
	snprintf(paramStrings[C_max_uptake_id], paramlen, "%s", "C_uptake_");
	snprintf(paramStrings[C_uptake_scale_id], paramlen, "%s", "C_scale_uptake_");
	snprintf(paramStrings[C_concp_id], paramlen, "%s", "C_concp_");

	snprintf(paramStrings[P_min_internal_id], paramlen, "%s", "P_min_internal_");
	snprintf(paramStrings[P_max_internal_id], paramlen, "%s", "P_max_internal_");

	snprintf(paramStrings[Psa_min_id], paramlen, "%s", "PSA_min_");
	snprintf(paramStrings[C_min_id], paramlen, "%s", "C_min_");
	snprintf(paramStrings[phyto_resp_rate_id], paramlen, "%s", "Phyto_Resp_Rate_");

	snprintf(paramStrings[KLYS_id], paramlen, "%s", "KLYS_");
	snprintf(paramStrings[mD_id], paramlen, "%s", "mD_");
	snprintf(paramStrings[mStarve_id], paramlen, "%s", "mStarve_");
	snprintf(paramStrings[mT_id], paramlen, "%s", "mT_");
	snprintf(paramStrings[mO_id], paramlen, "%s", "mO_");
	snprintf(paramStrings[mS_id], paramlen, "%s", "mS_");
	snprintf(paramStrings[mS_T15_id], paramlen, "%s", "mS_*_T15");
	snprintf(paramStrings[KO2_id], paramlen, "%s", "KO2_");
	snprintf(paramStrings[KO2LIM_id], paramlen, "%s", "KO2LIM_");
	snprintf(paramStrings[sp_remin_contrib_id], paramlen, "%s", "_remin_contrib");

	snprintf(paramStrings[max_id], paramlen, "%s", "max");
	snprintf(paramStrings[low_id], paramlen, "%s", "_low");
	snprintf(paramStrings[thresh_id], paramlen, "%s", "thresh");
	snprintf(paramStrings[sat_id], paramlen, "%s", "_sat");
	snprintf(paramStrings[FSBDR_id], paramlen, "%s", "FSBDR_");

	snprintf(paramStrings[flux_thresh_id], paramlen, "%s", "thresh");
	snprintf(paramStrings[flux_damp_id], paramlen, "%s", "damp");
	snprintf(paramStrings[flag_lim_id], paramlen, "%s", "flaglim");

	snprintf(cohortParamStrings[mL_T15_id], paramlen, "%s", "_mL");
	snprintf(cohortParamStrings[mQ_T15_id], paramlen, "%s", "_mQ");
	snprintf(spawnParamStrings[Time_Spawn_id], paramlen, "%s", "_Time_Spawn");
	snprintf(spawnParamStrings[Time_Age_id], paramlen, "%s", "_Time_Age");

    snprintf(cohortParamStrings[L_turbid_id], paramlen, "%s", "_L_turbid");
    snprintf(cohortParamStrings[a_turbid_id], paramlen, "%s", "_a_turbid");
    snprintf(cohortParamStrings[b_turbid_id], paramlen, "%s", "_b_turbid");
    
    snprintf(paramStrings[mindepth_id], paramlen, "%s", "_mindepth");
	snprintf(paramStrings[maxdepth_id], paramlen, "%s", "_maxdepth");
    snprintf(paramStrings[maxtotdepth_id], paramlen, "%s", "_maxtotdepth");
	snprintf(paramStrings[homerangerad_id], paramlen, "%s", "_homerangerad");
	snprintf(paramStrings[rangeoverlap_id], paramlen, "%s", "_overlap");
	snprintf(paramStrings[Speed_id], paramlen, "%s", "Speed_");

	/* The fisheries params */
	snprintf(paramStrings[flagfish_id], paramlen, "%s", "flagfish");
	snprintf(paramStrings[access_thru_wc_id], paramlen, "%s", "flag_access_thru_wc_");
	snprintf(paramStrings[tier_id], paramlen, "%s", "tier");
	snprintf(paramStrings[regionalSP_id], paramlen, "%s", "regionalSP");
	snprintf(paramStrings[basketSP_id], paramlen, "%s", "basketSP");
	snprintf(paramStrings[basket_size_id], paramlen, "%s", "basket_size");
    snprintf(paramStrings[max_co_sp_id], paramlen, "%s", "max_co_sp_");
	snprintf(paramStrings[coType_id], paramlen, "%s", "coType_");
	snprintf(paramStrings[tac_resetperiod_id], paramlen, "%s", "tac_resetperiod");
	snprintf(paramStrings[aquacult_age_harvest_id], paramlen, "%s", "_age_harvest");
    snprintf(paramStrings[cpue_cdf_poor_r_id], paramlen, "%s", "cpue_cdf_poor_r_");
    snprintf(paramStrings[cpue_cdf_poor_p_id], paramlen, "%s", "cpue_cdf_poor_p_");
    snprintf(paramStrings[cpue_cdf_top_r_id], paramlen, "%s", "cpue_cdf_top_r_");
    snprintf(paramStrings[cpue_cdf_top_p_id], paramlen, "%s", "cpue_cdf_top_p_");
    
    snprintf(paramStrings[whichRAssess_id], paramlen, "%s", "whichRAssess_");
    
	bm->SP_FISHERYprmsName = (char **) c_alloc2d(paramlenLong, tot_sp_specif_fishing_prms);
	snprintf(bm->SP_FISHERYprmsName[sel_id], paramlenLong, "%s", "sel_");
	snprintf(bm->SP_FISHERYprmsName[flagQchange_id], paramlenLong, "%s", "flagQchange_");
	snprintf(bm->SP_FISHERYprmsName[Q_num_changes_id], paramlenLong, "%s", "_q_changes");
	snprintf(bm->SP_FISHERYprmsName[q_id], paramlenLong, "%s", "q_");
	snprintf(bm->SP_FISHERYprmsName[mFC_start_age_id], paramlenLong, "%s", "_mFC_startage");
    snprintf(bm->SP_FISHERYprmsName[mFC_end_age_id], paramlenLong, "%s", "_mFC_endage");
	snprintf(bm->SP_FISHERYprmsName[mFC_num_changes_id], paramlenLong, "%s", "_mFC_changes");
	snprintf(bm->SP_FISHERYprmsName[mFC_id], paramlenLong, "%s", "mFC_");
    snprintf(bm->SP_FISHERYprmsName[flagFchange_id], paramlenLong, "%s", "flagFchange_");
	snprintf(bm->SP_FISHERYprmsName[flaghabitat_id], paramlenLong, "%s", "flaghabitat_");
	snprintf(bm->SP_FISHERYprmsName[flagescapement_id], paramlenLong, "%s", "flagescapement_");
    snprintf(bm->SP_FISHERYprmsName[spawn_closure_id], paramlenLong, "%s", "spawn_closure_");
	snprintf(bm->SP_FISHERYprmsName[flagF_id], paramlenLong, "%s", "flagF_");
	snprintf(bm->SP_FISHERYprmsName[Ka_escape_id], paramlenLong, "%s", "Ka_escape_");
	snprintf(bm->SP_FISHERYprmsName[Kb_escape_id], paramlenLong, "%s", "Kb_escape_");
	snprintf(bm->SP_FISHERYprmsName[flagdiscard_id], paramlenLong, "%s", "flagdiscard_");
	snprintf(bm->SP_FISHERYprmsName[flagchangeDISCRD_id], paramlenLong, "%s", "flagchangeDISCRD_");
	snprintf(bm->SP_FISHERYprmsName[DISCRD_num_changes_id], paramlenLong, "%s", "_discard_changes");
	snprintf(bm->SP_FISHERYprmsName[FFCDR_id], paramlenLong, "%s", "FFCDR_");
	snprintf(bm->SP_FISHERYprmsName[FC_thresh_id], paramlenLong, "%s", "FC_thresh_");
	snprintf(bm->SP_FISHERYprmsName[FC_high_thresh_id], paramlenLong, "%s", "FC_high_thresh_");
	snprintf(bm->SP_FISHERYprmsName[FCthreshli_id], paramlenLong, "%s", "FCthreshli_");
	snprintf(bm->SP_FISHERYprmsName[k_retain_id], paramlenLong, "%s", "k_retain_");
	snprintf(bm->SP_FISHERYprmsName[incidmort_id], paramlenLong, "%s", "incidmort_");
	snprintf(bm->SP_FISHERYprmsName[k_waste_id], paramlenLong, "%s", "k_waste_");
	snprintf(bm->SP_FISHERYprmsName[p_escape_id], paramlenLong, "%s", "p_escape_");
	snprintf(bm->SP_FISHERYprmsName[TAC_num_changes_id], paramlenLong, "%s", "_TAC_changes");
	snprintf(bm->SP_FISHERYprmsName[flagimposecatch_id], paramlenLong, "%s", "flagimposecatch_");
	snprintf(bm->SP_FISHERYprmsName[imposecatchstart_id], paramlenLong, "%s", "imposecatchstart_");
	snprintf(bm->SP_FISHERYprmsName[imposecatchend_id], paramlenLong, "%s", "imposecatchend_");
	snprintf(bm->SP_FISHERYprmsName[FC_reportscale_id], paramlenLong, "%s", "reportscale_");
	snprintf(bm->SP_FISHERYprmsName[trip_lim_id], paramlenLong, "%s", "TripLimit_");
	snprintf(bm->SP_FISHERYprmsName[prop_spawn_close_id], paramlenLong, "%s", "prop_spawn_close_");
	snprintf(bm->SP_FISHERYprmsName[TAC_id], paramlenLong, "%s", "TAC_");
	snprintf(bm->SP_FISHERYprmsName[FC_case_id], paramlenLong, "%s", "FC_case");
	snprintf(bm->SP_FISHERYprmsName[avail_id], paramlenLong, "%s", "avail_");
	snprintf(bm->SP_FISHERYprmsName[saleprice_id], paramlenLong, "%s", "saleprice");
	snprintf(bm->SP_FISHERYprmsName[tax_id], paramlenLong, "%s", "tax");
	snprintf(bm->SP_FISHERYprmsName[deemedvalue_id], paramlenLong, "%s", "deemed");
    snprintf(bm->SP_FISHERYprmsName[assess_nf_id], paramlenLong, "%s", "assess_nf_");
    snprintf(bm->SP_FISHERYprmsName[flagPerShotCPUE_id], paramlenLong, "%s", "flagPerShotCPUE_");
    snprintf(bm->SP_FISHERYprmsName[flagRecordCPUE_id], paramlenLong, "%s", "flagRecordCPUE_");
    
	//#define catch_allowed 23				/* Catch currently allowed to take (quota - cumulative catch) */
	//#define flagquota_id 24
	//#define marketwgt_id 25			    /* Market weighting for amount of fish feed to each market */
	//#define desired_chrt_id 31
	//#define origprice_id 32
	//#define TACvsMPA_id 38				/* Whether to reduce q to reflect effects of seasonal spawning season closures */
	//#define phase_out_id 41				/* Day of run when TAC should be phased out completely */
	//#define phase_start_id 42				/* Day of run when TAC phase out begins */


	RBCParamStrings = (char **) c_alloc2d(paramlen, num_rbc_species_params_id);
	snprintf(RBCParamStrings[DiscType_id], paramlen, "%s", "DiscType_");
	snprintf(RBCParamStrings[MaxH_id], paramlen, "%s", "MaxH_");
	snprintf(RBCParamStrings[Growthage_L1_id], paramlen, "%s", "Growthage_L1_");
	snprintf(RBCParamStrings[Growthage_L2_id], paramlen, "%s", "Growthage_L2_");
	snprintf(RBCParamStrings[MinCatch_id], paramlen, "%s", "MinCatch_");
	snprintf(RBCParamStrings[Tier4_Cmaxmult_id], paramlen, "%s", "Tier4_Cmaxmult_");
    snprintf(RBCParamStrings[AssessStart_id], paramlen, "%s", "AssessStart_");
    snprintf(RBCParamStrings[NumRegions_id], paramlen, "%s", "NumRegions_");
    snprintf(RBCParamStrings[Nsexes_id], paramlen, "%s", "Nsexes_");

	snprintf(RBCParamStrings[Maturity_Inflect_id], paramlen, "%s", "Maturity_Inflect_");
	snprintf(RBCParamStrings[Maturity_Slope_id], paramlen, "%s", "Maturity_Slope_");
	snprintf(RBCParamStrings[Tier3_Fcalc_id], paramlen, "%s", "Tier3_Fcalc_");
	snprintf(RBCParamStrings[Tier3_time_id], paramlen, "%s", "Tier3_time_");
	snprintf(RBCParamStrings[Tier3_maxage_id], paramlen, "%s", "Tier3_maxage_");
	snprintf(RBCParamStrings[T1_steep_phase_id], paramlen, "%s", "T1_steep_phase_");
	snprintf(RBCParamStrings[tiertype_id], paramlen, "%s", "tiertype_");
	snprintf(RBCParamStrings[Tier3_M_id], paramlen, "%s", "Tier3_M_");
	snprintf(RBCParamStrings[Tier3_S25_id], paramlen, "%s", "Tier3_S25_");
	snprintf(RBCParamStrings[Tier3_S50_id], paramlen, "%s", "Tier3_S50_");
	snprintf(RBCParamStrings[Tier3_F_id], paramlen, "%s", "Tier3_F_");
	snprintf(RBCParamStrings[Tier3_h_id], paramlen, "%s", "Tier3_h_");
	snprintf(RBCParamStrings[Tier3_matlen_id], paramlen, "%s", "Tier3_matlen_");
	snprintf(RBCParamStrings[Tier3_maxF_id], paramlen, "%s", "Tier3_maxF_");
	snprintf(RBCParamStrings[Tier4_avtime_id], paramlen, "%s", "Tier4_avtime_");
	snprintf(RBCParamStrings[Tier4_CPUEyrmin_id], paramlen, "%s", "Tier4_CPUEyrmin_");
	snprintf(RBCParamStrings[Tier4_CPUEyrmax_id], paramlen, "%s", "Tier4_CPUEyrmax_");
    snprintf(RBCParamStrings[Tier4_Bo_correct_id], paramlen, "%s", "Tier4_Bo_correct_");    
	snprintf(RBCParamStrings[Tier4_m_id], paramlen, "%s", "Tier4_m_");
	snprintf(RBCParamStrings[Tier4_alpha_id], paramlen, "%s", "Tier4_alpha_");
	snprintf(RBCParamStrings[Tier5_length_id], paramlen, "%s", "Tier5_length_");
	snprintf(RBCParamStrings[Tier5_S50_id], paramlen, "%s", "Tier5_S50_");
	snprintf(RBCParamStrings[Tier5_cv_id], paramlen, "%s", "Tier5_cv_");
	snprintf(RBCParamStrings[Tier5_flt_id], paramlen, "%s", "Tier5_flt_");
	snprintf(RBCParamStrings[Tier5_reg_id], paramlen, "%s", "Tier5_reg_");
	snprintf(RBCParamStrings[Tier5_p_id], paramlen, "%s", "Tier5_p_");
	snprintf(RBCParamStrings[Tier5sel_id], paramlen, "%s", "Tier5sel_");
	snprintf(RBCParamStrings[Tier5q_id], paramlen, "%s", "Tier5q_");
	snprintf(RBCParamStrings[PostRule_id], paramlen, "%s", "PostRule_");
    
	snprintf(RBCParamStrings[Tier1Sig_id], paramlen, "%s", "Tier1Sig_");
	snprintf(RBCParamStrings[Tier2Sig_id], paramlen, "%s", "Tier2Sig_");
	snprintf(RBCParamStrings[Tier3Sig_id], paramlen, "%s", "Tier3Sig_");
    
    snprintf(RBCParamStrings[UseRBCAveraging_id], paramlen, "%s", "UseRBCAveraging_");

    snprintf(RBCParamStrings[isTriggerSpecies_id], paramlen, "%s", "isTriggerSpecies_");
	snprintf(RBCParamStrings[TriggerResponseScen_id], paramlen, "%s", "TriggerResponseScen_");
    snprintf(RBCParamStrings[trigger_threshold_id], paramlen, "%s", "trigger_threshold_");

	snprintf(RBCParamStrings[CPUEmult_id], paramlen, "%s", "CPUEmult_");
	snprintf(RBCParamStrings[MaxChange_id], paramlen, "%s", "MaxChange_");

	snprintf(RBCParamStrings[Hsteep_id], paramlen, "%s", "Hsteep_");
	snprintf(RBCParamStrings[Agesel_Pattern_id], paramlen, "%s", "Agesel_Pattern_");
	snprintf(RBCParamStrings[AssessFreq_id], paramlen, "%s", "AssessFreq_");
	snprintf(RBCParamStrings[CCsel_years_id], paramlen, "%s", "CCsel_years_");

	snprintf(RBCParamStrings[MG_offset_id], paramlen, "%s", "MG_offset_");
	snprintf(RBCParamStrings[RecDevBack_id], paramlen, "%s", "RecDevBack_");
	snprintf(RBCParamStrings[Regime_shift_assess_id], paramlen, "%s", "Regime_shift_assess_");
    snprintf(RBCParamStrings[Regime_year_assess_id], paramlen, "%s", "Regime_year_assess_");
    snprintf(RBCParamStrings[BallParkF_id], paramlen, "%s", "BallParkF_");
	snprintf(RBCParamStrings[BallParkYr_id], paramlen, "%s", "BallParkYr_");
	snprintf(RBCParamStrings[NumChangeLambda_id], paramlen, "%s", "NumChangeLambda_");
    snprintf(RBCParamStrings[num_enviro_obs_id], paramlen, "%s", "num_enviro_obs_");
    snprintf(RBCParamStrings[num_growth_morphs_id], paramlen, "%s", "num_growth_morphs_");
    
	snprintf(RBCParamStrings[Nsex_samp_id], paramlen, "%s", "Nsex_samp_");
	snprintf(RBCParamStrings[MaxAge_id], paramlen, "%s", "MaxAge_");
	snprintf(RBCParamStrings[Nyfuture_id], paramlen, "%s", "Nyfuture_");
	snprintf(RBCParamStrings[Nlen_id], paramlen, "%s", "Nlen_");
	snprintf(RBCParamStrings[Lbin_id], paramlen, "%s", "Lbin_");
    snprintf(RBCParamStrings[NumFisheries_id], paramlen, "%s", "NumFisheries_");    
    snprintf(RBCParamStrings[thresh_mat_id], paramlen, "%s", "thresh_mat_");
    snprintf(RBCParamStrings[femsexratio_id], paramlen, "%s", "femsexratio_");

	snprintf(RBCParamStrings[flagLAdirect_id], paramlen, "%s", "flagLAdirect_");
	snprintf(RBCParamStrings[flagSLAdirect_id], paramlen, "%s", "flagSLAdirect_");
	snprintf(RBCParamStrings[flagWAdirect_id], paramlen, "%s", "flagWAdirect_");

	snprintf(RBCParamStrings[SigmaR1_id], paramlen, "%s", "SigmaR1_");
	snprintf(RBCParamStrings[SigmaR2_id], paramlen, "%s", "SigmaR2_");
	snprintf(RBCParamStrings[SigmaR_future_id], paramlen, "%s", "SigmaR_future_");
	snprintf(RBCParamStrings[PSigmaR1_id], paramlen, "%s", "PSigmaR1_");
	snprintf(RBCParamStrings[Regime_year_id], paramlen, "%s", "Regime_year_");
	snprintf(RBCParamStrings[RecDevMinYr_id], paramlen, "%s", "RecDevMinYr_");
	snprintf(RBCParamStrings[RecDevMaxYr_id], paramlen, "%s", "RecDevMaxYr_");
	snprintf(RBCParamStrings[RecDevFlag_id], paramlen, "%s", "RecDevFlag_");
	snprintf(RBCParamStrings[AutoCorRecDev_id], paramlen, "%s", "AutoCorRecDev_");
    
    snprintf(RBCParamStrings[LFSSlim_id], paramlen, "%s", "LFSSlim_");
    snprintf(RBCParamStrings[AFSSlim_id], paramlen, "%s", "AFSSlim_");
    snprintf(RBCParamStrings[NumSurvey_id], paramlen, "%s", "NumSurvey_");
    snprintf(RBCParamStrings[NblockPattern_id], paramlen, "%s", "NblockPattern_");
    snprintf(RBCParamStrings[SRBlock_id], paramlen, "%s", "SRBlock_");
    snprintf(RBCParamStrings[assRecDevMinYear_id], paramlen, "%s", "assRecDevMinYear_");
    
    snprintf(RBCParamStrings[MultispAssessType_id], paramlen, "%s", "MultispAssessType_");
    snprintf(RBCParamStrings[mgt_indicator_id], paramlen, "%s", "mgt_indicator_");
    snprintf(RBCParamStrings[init_mgt_category_id], paramlen, "%s", "init_mgt_category_");
    snprintf(RBCParamStrings[init_mgt_sp_id], paramlen, "%s", "init_mgt_sp_");
    
    snprintf(RBCParamStrings[PGMSYBHalpha_id], paramlen, "%s", "PGMSYBHalpha_");
    snprintf(RBCParamStrings[PGMSYBHbeta_id], paramlen, "%s", "PGMSYBHbeta_");

    return;
}

/*
 * \brief Get the index of the given tracer
 *
 * return -1 if not found.
 */
int Util_Get_Tracer_ID(MSEBoxModel *bm, char *tracerName) {
	int returnValue = -1;
	int i;

	for (i = 0; i < bm->ntracer; i++) {
		if (strcmp(bm->tinfo[i].name, tracerName) == 0)
			return i;
	}

	return returnValue;
}

/**
 * \brief Return the scaling factor that should be used at this timestep.
 *
 *
 */
double Util_Get_Change_Scale(MSEBoxModel *bm, int num_changes, double **changeArray) {

	int now_change = 0, past_change, i;
	double scale = 0.0, end_date, multB, multA, start, period;

	if ((bm->dayt >= changeArray[0][start_id]) && (changeArray[0][start_id] != 0)) {
		now_change = 0;
		for (i = 0; i < num_changes; i++) {
			if (bm->dayt >= changeArray[i][start_id])
				now_change = i;
		}

		end_date = (changeArray[now_change][start_id] + changeArray[now_change][period_id]);
		past_change = now_change - 1;
		multA = changeArray[now_change][mult_id];

		if (now_change > 0)
			multB = changeArray[past_change][mult_id];
		else
			multB = 1.0;
		start = changeArray[now_change][start_id];
		period = changeArray[now_change][period_id] + small_num;

		if (end_date < bm->dayt)
			scale = multA;
		else {
			if (multA > multB)
				scale = multB + (multA - multB) * (bm->dayt - start) / period;
			else
				scale = multB - (multB - multA) * (bm->dayt - start) / period;
		}
	} else {
		scale = 1.0;
	}
	return scale;
}

/**
 * \brief Return the scaling factor that should be used at this timestep.
 *
 *
 */
double Util_Get_Accumulative_Change_Scale(MSEBoxModel *bm, int num_changes, double **changeArray) {

	int changeIndex;
	double scale = 1.0, end_date, step1  = 0.0;

	for (changeIndex = 0; changeIndex < num_changes; changeIndex++) {

		if ((bm->dayt >= changeArray[changeIndex][start_id]) && (changeArray[0][start_id] != 0)) {

			end_date = changeArray[changeIndex][start_id] + changeArray[changeIndex][period_id];

			if (bm->dayt >= end_date) {
                /* If the increase has finished then simply apply */
				scale = scale * changeArray[changeIndex][mult_id];
			} else {
                /* Else apply partial outcome */
                if (changeArray[changeIndex][mult_id] < 1.0) {
                    step1 = (1.0 - changeArray[changeIndex][mult_id]) * ((bm->dayt - changeArray[changeIndex][start_id]) / changeArray[changeIndex][period_id]);
                    scale = scale * (1.0 - step1);
                } else {
                    step1 = (changeArray[changeIndex][mult_id] - 1.0) * ((bm->dayt - changeArray[changeIndex][start_id]) / changeArray[changeIndex][period_id]);
                    scale = scale * (step1 + 1.0);
                }
			}
		}
	}
	return scale;
}

void Util_Copy_Change_Values(MSEBoxModel *bm, double **originalArray, double **newArray, int size, double start, double period, double mult) {

	int arrayIndex, i;
	int beenInserted = FALSE;

	/* Now copy the values across inserting the new value in the correct place.*/
	arrayIndex = 0;
	if (size > 1) {
		for (i = 0; i < size - 1; i++) {
			if (beenInserted == FALSE) {
				if (originalArray[i][start_id] > start) {
					/* insert the new values into the array here */
					newArray[arrayIndex][start_id] = start + bm->dayt;
					newArray[arrayIndex][period_id] = period;
					newArray[arrayIndex][mult_id] = mult;
					arrayIndex++;
					beenInserted = TRUE;
				}
			}
			newArray[arrayIndex][start_id] = originalArray[i][start_id];
			newArray[arrayIndex][period_id] = originalArray[i][period_id];
			newArray[arrayIndex][mult_id] = originalArray[i][mult_id];

			arrayIndex++;
		}
	}
	/* Make sure the value has been inserted else insert it at the end */
	if (beenInserted == FALSE) {
		newArray[arrayIndex][start_id] = start + bm->dayt;
		newArray[arrayIndex][period_id] = period;
		newArray[arrayIndex][mult_id] = mult;
	}
}

/**
 * \brief Get the current stock id for this species in the given box and depth.
 *
 * Only age structured groups have stock structure typically so the stocj array
 * for other groups will be zeroed on initialisation. So to check against that
 * use max_id to make sure there is a stock map, if not default to stock_id 0
 */
int Util_Get_Current_Stock_Index(MSEBoxModel *bm, int sp, int boxNumber, int nzk) {
	int sp_stock_type;
	int stock_id = 0;
	int relk, diffdz;


	diffdz = bm->wcnz - bm->boxes[boxNumber].nz;
	relk = nzk + diffdz;
	sp_stock_type = (int) (FunctGroupArray[sp].speciesParams[stockstruct_type_id]);
	switch (sp_stock_type) {
	case horiz_only:
		stock_id = bm->stock_struct[boxNumber][sp] - 1;
		break;
	case vert_only:
		stock_id = bm->v_stock_struct[relk][sp] - 1;
		break;
	case mixed_stock:
		stock_id = (bm->stock_struct[boxNumber][sp] - 1) + (bm->v_stock_struct[relk][sp] - 1) * ((int) (FunctGroupArray[sp].speciesParams[hstockstruct_id]));
		break;
	default:
		quit("No such stock structure case (%d) defined. Recode\n", sp_stock_type);
		break;
	}



	if(stock_id < 0) {
		// Default to a value of zero - will be for biomass pools (age structured or otherwise)
		stock_id = 0;
		//quit("Stock_id %d is not a valid value for %s (stock type: %d)", stock_id, FunctGroupArray[sp].groupCode, sp_stock_type);
	}

	if(stock_id != bm->group_stock[sp][boxNumber][nzk]){
			printf("bm->group_stock[%d][%d][%d] = %d,  stock_id = %d\n", sp, boxNumber, nzk, bm->group_stock[sp][boxNumber][nzk], stock_id);
			abort();
		}
	return stock_id;
}

/**
 * 
 * Calculate the stock id for each group in each box/layer combination.
 */
void Util_Calculate_StockID(MSEBoxModel *bm){
	int sp, box, layer;
	int stock_type, diff, relk, stock = 0;

	for(sp = 0; sp < bm->K_num_tot_sp; sp++){
		stock_type = (int) (FunctGroupArray[sp].speciesParams[stockstruct_type_id]);
		for(box = 0; box < bm->nbox; box++){
			diff = bm->wcnz - bm->boxes[box].nz;

            for(layer = 0; layer < bm->boxes[box].nz; layer++){
				relk = layer + diff;
				switch (stock_type) {
				case horiz_only:
					stock = bm->stock_struct[box][sp] - 1;
					break;
				case vert_only:
					stock = bm->v_stock_struct[relk][sp] - 1;
					break;
				case mixed_stock:
					stock = (bm->stock_struct[box][sp] - 1) + (bm->v_stock_struct[relk][sp] - 1)
							* ((int) (FunctGroupArray[sp].speciesParams[hstockstruct_id]));
					break;
				default:
					quit("No such stock structure case (%d) defined. Recode\n", stock_type);
					break;
				}
				if(stock < 0) {
					// Default to a value of zero - will be for biomass pools (age structured or otherwise)
					stock = 0;
					//quit("Stock_id %d is not a valid value for %s (stock type: %d)", stock_id, FunctGroupArray[sp].groupCode, sp_stock_type);
				}

				bm->group_stock[sp][box][layer] = stock;
			}
		}
	}

}

/******** HELPER ROUTINES ************************************************************

 These general routines are used by all the modules, mainly to do with feeding
 or are other helper routines.
 **/

/**
 *	\brief Michaelis-Menten Kinetics
 *
 */
double Util_Mich_Ment(double x, double m) {
	double result_MM = 0;

	if (x > 0.0)
		if (m > 0.0)
			result_MM = x / (m + x);
		else
			result_MM = 1.0;
	else
		result_MM = 0.0;

	return (result_MM);
}

/**
 *    \brief random number from normal distribution
 *
 */
double Util_Normx_Result(double mu, double sigma) {
    double result, step_a, step_b, step_c;

    step_a = drandom(0.0, 1.0);
    step_b = drandom(0.0, 1.0);
    step_c = sqrt(-2.0 * log(step_a)) * cos(2.0 * 3.1415926 * step_b);
    result = step_c * sigma + mu;

    return (result);

}

/**
 *	\brief random number from lognormal distribution
 *
 */
double Util_Logx_Result(double mu, double sigma) {
	double x_b, result, step_a, step_b, step_c;

	x_b = drandom(0.0, 1.0);
	step_a = 1.0 / (x_b * sigma * sqrt(2.0 * 3.141592654));
	step_b = (log(x_b) - mu) * (log(x_b) - mu);
	step_c = 2.0 * sigma * sigma;
	result = step_a * exp(-step_b / step_c);

	return (result);

}

/**
 *	\brief lognormal distribution
 *
 */
double Util_Lognorm_Distrib(double mu, double sigma, double x_b) {
	double result, step_a, step_b, step_c;

	step_a = 1.0 / (sigma * sqrt(2.0 * 3.141592654));
	step_b = (log(x_b) - mu) * (log(x_b) - mu);
	step_c = 2.0 * sigma * sigma;
	result = step_a * exp(-step_b / step_c);

	return (result);

}

int at_compileRegExpression(regex_t *regBuffer, char *str) {
	int returnValue;
	regBuffer->re_nsub = 1;
	returnValue = regcomp(regBuffer, str, REG_EXTENDED);

	if (returnValue != 0) {
		printf("error compiling regular expression");
		return -1;
	}
	return 0;
}


/**
 *	Open an Atlantis output file in the given destination folder.
 *
 */
FILE	*Util_fopen(MSEBoxModel *bm, const char *name, const char *mode){
	char fileName[BMSLEN];


	snprintf(fileName, sizeof(fileName), "%s%s", bm->destFolder, name);
	return fopen(fileName,mode);
}


/**
 *	\brief - Wrapper around the ncopen function to check that a file with the given filename
 *	exists before ncopen is call.
 *
 *	@returns The FILE * pointer to the opened file.
 *
 *	@param name The name of the file to open
 *	@param mode The netcdf mode to use when opening the file. See the ncopen documentation
 *	for more information.
 *
 *
 */
int Util_ncopen(const char *destFolder, const char* name ,int mode){
	int fp;
	FILE *fileFp;
	char fileName[BMSLEN];

	if(strlen(destFolder) > 0){
        snprintf(fileName, sizeof(fileName), "%s%s", destFolder, name);
	}else{
		strcpy(fileName, name);
	}
	trim(fileName);
    
    //printf("Opening %s\n", fileName);

	/**
	 *	Open the file to check that it exists if the mode is NC_NOWRITE.
	 *	The netcdf library should return an error if the file
	 *	does not exits but its spitting the dummy.
	 *
	 */
	if(mode == NC_NOWRITE){
		if((fileFp = fopen(fileName, "r")) == NULL)
			quit("fopen: Can't open netcdf model input data fileno readable - reason 1 \n '%s' \n",fileName);
		fclose(fileFp);
		/* Open the file */
		if( (fp=ncopen(fileName,mode)) < 0 )
		   quit("Util_ncopen: Can't open netcdf model input data file - ncopen fail '%s'\n",fileName);
	}
	else{
		if( (fp=ncopen(fileName,mode)) < 0 ){
			return -1;
		}
			//quit("Util_ncopen: Can't create netcdf model input data file %s\n",fileName);
	}
    
    if(verbose) {
        printf("Finished Util_ncopen\n");
    }
    
	return fp;
}

//  mean is mean of normal dsn and sigg is std devn
//
//  Calls :   ran1  (which is in cplib library)
//  Called by:
//  Created: translated from Gavin's Fortran by Sally 10/10/2007
//
// ******************************************************************************

double Util_xnorm(double mean, double sigg, int *iiseed)
{
	double z1,z2,xn;

// ignore zero calls
	if (sigg==0.0) return 0.0;

	z1 = ran3(iiseed);
	z2 = ran3(iiseed);

	xn = sin(6.283185*z1) * sqrt(-2.0*log(z2)) * sigg + mean;

	return xn;

}

/******************************************************************************
 *
 *  Name: GenMnorm
 *  Description: generate from a multivariate normal
 *  Parameters :
 *      vec is the answer
 *      means is the vector of means
 *      np is number of parameters (array dimensions)
 *      tt is np x np correlation matrix
 *      sg is 1 x np vector - standard deviation of random walk
 *
 *  Calls :    xnorm
 *  Created:  translated from Gavin's Fortran by Sally 10/10/07
 *
 ******************************************************************************/
void Util_GenMnorm(double *vec, double *means, int *iseed, int np, double **tt, double *sg){
	double *aa;
	double x,sigs;
	int i,j;

	// initial errors to zero
	aa = Util_Alloc_Init_1D_Double(np, 0.0);

	// generate the multivariate normal - sg is the same for all lengths
	for (i=0; i < np; i++){
		sigs = sg[i];
		x = Util_xnorm(0.0, sigs, iseed);
		for (j=0; j < np; j++)
			aa[j] += x * tt[j][i];    // shouldn't this be sqrt(tt(j,i))?? NO correlation, not var-covar
	}

	// complete the generation
	for (i=0; i < np; i++)
		vec[i] = means[i] + aa[i];

	// clean-up
	free1d(aa);

	return;
}

int Util_Check_NetCDF_Size(MSEBoxModel *bm, int fid, int *dump, char *fileName, int *index, int type){
	struct stat buffer;
	char fname[STRLEN*2];
	char endname[] = ".nc";
	char tempStr[STRLEN];
	char *pdest;
	int newFid;

	if(fid < 0)
		return -1;

	if((*index) > 0){
		strcpy(tempStr, fileName);
		pdest = strstr(tempStr, endname);
		*pdest = '\0';

		snprintf(fname, sizeof(fname), "%s%s%d.nc", bm->destFolder, tempStr, *index);
	}else{
		snprintf(fname, sizeof(fname), "%s%s", bm->destFolder, fileName);
	}


	/* Might need to close the file and then open it after we have checked its size */
	/* Check the size of the diff file. */
	if(stat(fname, &buffer) < 0){
		quit("Unable to get information about file %s\n", fileName);
	}

	if (buffer.st_size > MAX_NETCDF_FILE_SIZE) {
		strcpy(tempStr, fileName);
		pdest = strstr(tempStr, endname);
		*pdest = '\0';

		(*index)++;
		snprintf(fname, sizeof(fname), "%s%d.nc", tempStr, *index);
		ncclose(fid);

		/* Create file anew */
		if(type == -1){
			newFid = createBMSummaryDataFile(bm->destFolder, fname, bm);
		} else if (type == 4) {
            newFid = createBMAnnAgeBioDataFile(bm->destFolder, fname, bm);
        } else if (type == 5) {
            newFid = createBMAnnAgeCatDataFile(bm->destFolder, fname, bm);
        } else if (type == 6) {
            newFid = createBMDietDataFile(bm->destFolder, fname, bm);
        } else {
			newFid = createBMDataFile(bm->destFolder, fname, bm, type);
		}
		*dump = 0;


		return newFid;

	}
	return fid;
}
