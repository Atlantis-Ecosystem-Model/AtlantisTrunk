/*
 * Util_Fishety_XML.c
 *
 *  Created on: 01/02/2010
 *      Author: bec
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
#include <atUtilLib.h>

SpeciesParamStruct FisheryParamsArray[numFisheryParams] =
	{

	/** Fishery Flags */
		{ "Set_Fishery_Active", fisheriesflagactive_id, "^flag.*day$", integer_check, 1.0, -1 },
		{ "FisheryDemersal", flagdempelfishery_id, "_flagdempelfishery$", binary_check, 1.0, -1 }, // TODO: Read this is from the fishery definition file.
				{ "FisheryEffortModel", flageffortmodel_id, "_effortmodel$", integer_check, 1.0, -1 },
				{ "ExploratoryFishingDone", flagexplore_id, "_explore$", integer_check, 1.0, -1 },
				{ "EffortDrop", flagdropEFF_id, "_effortdrop$", integer_check, 1.0, -1 },
				{ "SelectivityCurve", selcurve_id, "_selcurve$", integer_check, 1.0, -1 },

				{ "FisheryStart", tStart_id, "_tStart$", no_checking, 1.0, -1 },
				{ "FisheryEnd", tEnd_id, "_tEnd$", no_checking, 1.0, -1 },
				{ "FisheryManageStart", start_manage_id, "_start_manage$", no_checking, 1.0, -1 },
				{ "FisheryManageEnd", end_manage_id, "_end_manage$", no_checking, 1.0, -1 },

				{ "SelectivityChangeFlag", flagchangeSEL_id, "_changeSEL$", binary_check, 1.0, -1 },
				{ "CoverageChangeFlag", flagchangeP_id, "_changeP$", binary_check, 1.0, -1 },
				{ "SweptAreaChangeFlag", flagchangeSWEPT_id, "_changeSWEPT$", binary_check, 1.0, -1 },
				{ "MaxSeaFloorDepth", maxFCdepth_id, "_maxdepth$", no_checking, 1.0, -1 },
				{ "MinSeaFloorDepth", minFCdepth_id, "_mindepth$", no_checking, 1.0, -1 },

				{ "LogisticSelCurveB", sel_b_id, "sel_b$", no_checking, 1.0, -1 },
				{ "LogisticSelCurveInflectionPoint", sel_lsm_id, "sel_lsm$", no_checking, 1.0, -1 },

				{ "CurveSpread", sel_normsigma_id, "sel_normsigma$", no_checking, 1.0, -1 },
				{ "ModalLength", sel_normlsm_id, "sel_normlsm$", no_checking, 1.0, -1 },
				{ "LognormalCurveSpread", sel_lognormsigma_id, "sel_lognormsigma$", no_checking, 1.0, -1 },

				{ "LognormalCurveModalLength", sel_lognormlsm_id, "sel_lognormlsm$", no_checking, 1.0, -1 },
				{ "GammaCurveSpread", sel_gammasigma_id, "sel_gammasigma$", no_checking, 1.0, -1 },
				{ "GammaCurveModalLength", sel_gammalsm_id, "sel_gammalsm$", no_checking, 1.0, -1 },
				{ "biModalCurveSpread", sel_bisigma_id, "sel_bisigma$", no_checking, 1.0, -1 },
                { "biModalCurveSpread2", sel_bisigma2_id, "sel_bisigma2$", no_checking, 1.0, -1 },
                { "biModalCurveAmplitude", sel_ampli_id, "sel_ampli$", no_checking, 1.0, -1 },
                { "BimodalCurveLeftModalLength", sel_bilsm1_id, "sel_bilsm1$", no_checking, 1.0, -1 },
				{ "BimodalCurveRightModalLength", sel_bilsm2_id, "sel_bilsm2$", no_checking, 1.0, -1 },

				{ "MaxSalePrice", maxsaleprice_id, "_maxsaleprice$", no_checking, 1.0, -1 },
				{ "FishingGearSweptArea", swept_area_id, "_sweptarea$", no_checking, 1.0, -1 },

				{ "ProportionInfringement", infringe_id, "_infringe", no_checking, 1.0, -1 },
				{ "SelectivityChange", SEL_num_changes_id, "_sel_changes$", integer_check, 1.0, -1 },
				{ "NumCoverageChanges", P_num_changes_id, "_p_changes$", integer_check, 1.0, -1 },
				{ "NumSweptAreaChanges", SWEPT_num_changes_id, "_swept_changes$", integer_check, 1.0, -1 },
			/* Not sure that these checking types are correct */
				{ "FlagUseAll", flaguseall_id, "_flaguseall$", integer_check, 1.0, -1 },
				{ "UseMinLever", use_min_lever_id, "_use_min_lever$", integer_check, 1.0, -1 },

			/* Management values - these are read in from the management input file */
				{ "AdaptionManagementOption", flagmanage_id, "_flagmanage$", integer_check, 1.0, -1 },
				{ "TACParticipationFlag", flagTACpartipcate_id, "^flagTACparticipate_", binary_check, 1.0, -1 },
				{ "EffortCapFlag", flagcap_id, "_flagcap$", binary_check, 1.0, -1 },

				{ "EffortCapPeakFlag", flagcap_peak_id, "_flagcap_peak$", binary_check, 1.0, -1 },
				{ "SeasonalFlag", flagseasonal_id, "_flagseasonal$", binary_check, 1.0, -1 },
                { "FrameBasedFlag", flag_framebased_id, "_flag_framebased$", binary_check, 1.0, -1 },
				{ "FisherySpatialManagementOption", flagmpa_id, "_flagmpa$", integer_check, 1.0, -1 },

				{ "EffortChange", flagchangeEFF_id, "_changeEFF$", binary_check, 1.0, -1 },
				{ "SeasonLengthChange", flagchangeseason_id, "_changeseason$", binary_check, 1.0, -1 },
				{ "TACChange", TACchange_id, "_TACchange$", integer_check, 1.0, -1 },

				{ "MaximumEffortlevel", mEff_max_id, "_mEff_max$", no_checking, 1.0, -1 },
				{ "EffortCoefficient", mEff_a_id, "_mEff_a$", no_checking, 1.0, -1 },
				{ "EffortOffset", mEff_offset_id, "_mEff_offset$", no_checking, 1.0, -1 },
				{ "ExploratoryEffort", mEff_testfish_id, "_mEff_testfish$", no_checking, 1.0, -1 },

				{ "EffortThreshold", mEff_thresh_id, "_mEff_thresh$", no_checking, 1.0, -1 },
				{ "EffortTopThreshold", mEff_thresh_top_id, "_mEff_thresh_top$", no_checking, 1.0, -1 },
				{ "EffortShift", mEff_shift_id, "_mEff_shift$", no_checking, 1.0, -1 },
				{ "DistanceScaler", mFCscale_id, "_mFCscale$", no_checking, 1.0, -1 },
				{ "EffortLevel", EffortLevel_id, "_Effort$", no_checking, 1.0, -1 },

				{ "CPUEThresh", CPUE_effort_thresh_id, "CPUEthresh$", no_checking, 1.0, -1 },
				{ "CPUEscale", CPUE_effort_scale_id, "CPUEscale$", no_checking, 1.0, -1 },
                { "EffortCaps", cap_id, "_cap$", no_checking, 1.0, -1 },
        
				{ "LandCatch", landallTAC_sp_id, "_landallTAC_sp$", binary_check, 1.0, -1 },
				{ "MaxNumGroups", max_num_sp_id, "_max_num_sp$", integer_check, 1.0, -1 },

			/* Seasonal fishery params */
				{ "SeasonOpenDay", seasonopen_id, "_seasonopen$", integer_check, 1.0, -1 },
				{ "SeasonCloseDay", seasonclose_id, "_seasonclose$", integer_check, 1.0, -1 },

				{ "RestrictedEffortProportion", FC_restrict_id, "_FC_restrict$", no_checking, 1.0, -1 },
				{ "RestrictionPeriod", FC_period_id, "_FC_period$", integer_check, 1.0, -1 },
				{ "RestrictedCheckDelay", FC_period2_id, "_FC_period2$", integer_check, 1.0, -1 },

				{ "EndangeredRestrictedEffortProportion", FC_restrict_endangered_id, "_FC_restrict_endangered$", no_checking, 1.0, -1 },
				{ "EndangeredRestrictionPeriod", FC_endanger_period_id, "_FC_endanger_period$", integer_check, 1.0, -1 },
				{ "mFCPeak", mFCpeak_id, "_mFCpeak$", no_checking, 1.0, -1 },

				{ "NumEffortChanges", EFF_num_changes_id, "_num_changes$", integer_check, 1.0, -1 },
				{ "NumChangesEffortCap", CAP_num_changes_id, "_cap_changes$", integer_check, 1.0, -1 },

			/* Assessment parameters */
				{ "Population_Half_Length", q_lsm_id, "Population_Half_Length", no_checking, 1.0, -1 },
				{ "Selectivity_Spread", q_sigma_id, "Selectivity_Spread", no_checking, 1.0, -1 },
				{ "Selectivity_Option", assess_selcurve_id, "Selectivity_Option", integer_check, 1.0, -1 },

			/** Economic values **/
				{ "NumSubfleets", nsubfleets_id, "_nsubfleets", integer_check, 1.0, -1 },
				{ "MaxNumLicences", nlicence_id, "_nlicence", integer_check, 1.0, -1 },
				{ "MultiSpeciesEffort", flagMultiSpEffort_id, "_flagMultiSpEffort", binary_check, 1.0, -1 },
			//{ "DeemedValueFlag", flagDV_id, integer_check, 1.0, -1 },
				{ "DeemedValueFlag", flagDV_id, "_flagDV", integer_check, 1.0, -1 },
			//{	"DeemedValue", deemedvalue_id,  "deemed$",integer_check, 1.0, -1},
				{ "BuyBackDate", buybackdate_id, "_buybackdate", integer_check, 1.0, -1 }

	};

SpeciesParamStruct FisheryGroupParamsArray[numGroupFisheryParams] =
	{

	/** Fishery Flags */
		{ "GroupHabitat", flaghabitat_id, "^flaghabitat_", binary_check, 1.0, SP_FISHED },
		{ "GroupDiscard", flagdiscard_id, "^flagdiscard_", integer_check, 1.0, SP_FISHED },
		{ "GroupEscapement", flagescapement_id, "^flagescapement_", integer_check, 1.0, SP_FISHED },
        { "GroupSpawnClosure", spawn_closure_id, "^spawn_closure_", integer_check, 1.0, SP_FISHED },

		{ "GroupImposeCatch", flagimposecatch_id, "^flagimposecatch_", integer_check, 1.0, SP_FISHED },
		{ "GroupCatchTimeSeries", flagF_id, "^flagF_", integer_check, 1.0, SP_FISHED },
		{ "GroupCatchabilityChangesFlag", flagQchange_id, "^flagQchange_", binary_check, 1.0, SP_FISHED },
		{ "GroupMortalityChangesFlag", flagFchange_id, "^flagFchange_", binary_check, 1.0, SP_FISHED },
		{ "GroupDiscardingChangesFlag", flagchangeDISCRD_id, "^flagchangeDISCRD_", binary_check, 1.0, SP_FISHED },
		{ "GroupCatchability", q_id, "^q_", no_checking, 1.0, SP_FISHED },
		{ "GroupFisheryStartDay", imposecatchstart_id, "^imposecatchstart_", integer_check, 1.0, SP_FISHED },
		{ "GroupFisheryEndDay", imposecatchend_id, "^imposecatchend_", integer_check, 1.0, SP_FISHED },
        { "FlagPerShotCPUE", flagPerShotCPUE_id, "^flagPerShotCPUE_", binary_check, 1.0, SP_FISHED },
        { "FlagRecordCPUE", flagRecordCPUE_id, "^flagRecordCPUE_", binary_check, 1.0, SP_FISHED },
        
		{ "CatchReportingScale", FC_reportscale_id, "^reportscale_", no_checking, 1.0, SP_FISHED },
		{ "ForcedFishingMortality", mFC_id, "^mFC_", no_checking, 1.0, SP_FISHED },
		{ "ForcedFishingAgeClass", mFC_start_age_id, "_mFC_startage$", integer_check, 1.0, SP_AGE_STRUCTURED },
        { "ForcedFishingAgeClassEnd", mFC_end_age_id, "_mFC_endage$", integer_check, 1.0, SP_AGE_STRUCTURED },
        
        //{ "GroupSelectivity", sel_id, "^sel_", no_checking, 1.0, SP_FISHED },
		{ "GroupSelectivity", sel_id, "^sel_[A-Z]{2,3}$", no_checking, 1.0, SP_FISHED },

		{ "EscapeProportion", p_escape_id, "^p_escape_", no_checking, 1.0, SP_FISHED },
		{ "EscapeCoefficient", Ka_escape_id, "^Ka_escape_", proportion_check, 1.0, SP_FISHED },
		{ "EscapeExponent", Kb_escape_id, "^Kb_escape_", proportion_check, 1.0, SP_FISHED },

		{ "FixedDiscardProportion", FFCDR_id, "^FFCDR_[A-Z]{2,3}$", proportion_check, 1.0, SP_FISHED },
		{ "AgeDependantDiscarding", FC_case_id, "^FC_case", binary_check, 1.0, SP_FISHED },
		{ "SizeBasedDiscarding", FCthreshli_id, "^FCthreshli_", no_checking, 1.0, SP_VERTS },
		{ "IncidentalMortality", incidmort_id, "^incidmort_", no_checking, 1.0, SP_VERTS },
		{ "ProportionRetained", k_retain_id, "^k_retain_", no_checking, 1.0, SP_VERTS },
		{ "ProportionDiscardedWaste", k_waste_id, "^k_waste_", no_checking, 1.0, SP_VERTS },

		{ "DiscardingChanges", DISCRD_num_changes_id, "_discard_changes$", integer_check, 1.0, SP_FISHED },
		{ "CatchabilityChanges", Q_num_changes_id, "_q_changes$", integer_check, 1.0, SP_FISHED },
		{ "FishingMortalityChanges", mFC_num_changes_id, "_mFC_changes$", integer_check, 1.0, SP_FISHED },

	/* Management Implementation parameters */
		{ "NumTACChanges", TAC_num_changes_id, "_TAC_changes$", integer_check, 1.0, SP_FISHED },
		{ "TripLimits", trip_lim_id, "^TripLimit_", no_checking, 1.0, SP_FISHED },
		{ "TotalAllowableCatch", TAC_id, "^TAC_[A-Z]", no_checking, 1.0, SP_FISHED },

	/* TAC_Parameters */
		{ "ProportionSpawningClosed", prop_spawn_close_id, "^prop_spawn_close_", no_checking, 1.0, SP_FISHED },

	/* Population threshold values */
		{ "ProportionEffortRestricted", FC_thresh_id, "^FC_thresh_", proportion_check, 1.0, SP_FISHED },
		{ "FCHighThreshold", FC_high_thresh_id, "^FC_high_thresh_", no_checking, 1.0, SP_FISHED },

	/* Assessment parameters */
		{ "StockAvailability", avail_id, "^avail_", proportion_check, 1.0, SP_VERTS },
        { "FleetMapping", assess_nf_id, "^assess_nf_", integer_check, 1.0, SP_FISHED },

	/* Economic parameters */
		{ "FishSalePrice", saleprice_id, "saleprice", no_checking, 1.0, SP_FISHED },
		{ "PenaltyTax", tax_id, "tax", no_checking, 1.0, SP_FISHED },
		{ "DeemedValue", deemedvalue_id, "deemed$", no_checking, 1.0, SP_FISHED },

	};

static double Get_Fisheries_Node_Value(char *fileName, xmlNodePtr parent, int paramID, int fisheryIndex, SpeciesParamStruct *paramStruct) {

	double value;
	xmlNodePtr attributeNode;

	attributeNode = Util_XML_Get_Node(ATLANTIS_FISHERY_ATTRIBUTE, parent, FisheryArray[fisheryIndex].fisheryCode);

	if (attributeNode == NULL)
		quit("Error: Cannot find fishery parameter %s/%s in input file %s.\n", paramStruct[paramID].tag, FisheryArray[fisheryIndex].fisheryCode, fileName);

	/* Sucessfully found parameter - convert to double, check and return.*/
	if(Util_XML_Get_Node_Value_Double(ATLANTIS_ATTRIBUTE, attributeNode, &value)){
		value = value* paramStruct[paramID].divider;
	}else{
		quit("Error: Cannot find fishery parameter %s/%s in input file %s.\n", paramStruct[paramID].tag, FisheryArray[fisheryIndex].fisheryCode, fileName);

	}

	if (verbose > 1)
		printf("Fishery parameter: %s = %e\n", FisheryArray[fisheryIndex].fisheryCode, value);

	Check_Value(FisheryArray[fisheryIndex].fisheryCode, value, paramStruct[paramID].checkType, paramStruct[paramID].tag);

	return value;
}

int Util_XML_Get_Fishery_Param_Index(SpeciesParamStruct array[], int size, int paramID) {
	int index;

	for (index = 0; index < size; index++) {
		if (array[index].paramID == paramID)
			return index;
	}

	quit("Util_XML_Get_Fishery_Param_Index - paramID %d is not recognised\n", paramID);
	return -1;
}

void Util_XML_Read_Fishery_Param(MSEBoxModel *bm, char *fileName, xmlNodePtr parent, int paramID) {
	int fisheryIndex;
	xmlNodePtr attributeGroup;
	int index;
	char *nodeName;
	char errorString[STRLEN];



	index = Util_XML_Get_Fishery_Param_Index(FisheryParamsArray, numFisheryParams, paramID);
	if (verbose)
		printf("Read fisheries parameters '%s'\n", FisheryParamsArray[index].tag);

	nodeName =  Util_Get_Node_Name(parent);
	snprintf(errorString, sizeof(errorString), "%s/%s", nodeName, FisheryParamsArray[index].tag);

	attributeGroup = Util_XML_Get_Node(ATLANTIS_ATTRIBUTE, parent, FisheryParamsArray[index].tag);
	if (attributeGroup == NULL)
		quit("%s attribute group not found in file %s.\n", errorString, fileName);

	for (fisheryIndex = 0; fisheryIndex < bm->K_num_fisheries; fisheryIndex++) {
		bm->FISHERYprms[fisheryIndex][paramID] = Get_Fisheries_Node_Value(fileName, attributeGroup, index, fisheryIndex, FisheryParamsArray);
	}
	free(nodeName);

}

/**
 * Read integer data into the given array.
 *
 */
void Util_XML_Read_Fishery_Param_Int(MSEBoxModel *bm, char *fileName, xmlNodePtr parent, char *paramName, int checkType, int **targetArray, int size) {
	int fisheryIndex;
	xmlNodePtr attributeGroup;
	double *values = 0;
	int b;
	char errorString[STRLEN];
	char *nodeName =  Util_Get_Node_Name(parent);

	snprintf(errorString, sizeof(errorString), "%s/%s", nodeName, paramName);
	attributeGroup = Util_XML_Get_Node(ATLANTIS_ATTRIBUTE, parent, paramName);
	if (attributeGroup == NULL)
		quit("readFisheryGroupParamXML - %s attribute group not found.\n", errorString);

	if (verbose)
		printf("Read fisheries groups parameters %s\n", paramName);

	for (fisheryIndex = 0; fisheryIndex < bm->K_num_fisheries; fisheryIndex++) {
		if (Util_XML_Read_Array_Double(ATLANTIS_FISHERY_ATTRIBUTE, fileName, errorString, attributeGroup, checkType, FisheryArray[fisheryIndex].fisheryCode, &values, size)
				== FALSE) {
			quit("Error: Unable to find parameter '%s/%s' in input file %s\n", errorString, FisheryArray[fisheryIndex].fisheryCode, fileName);
		}

		for (b = 0; b < size; b++) {
			targetArray[fisheryIndex][b] = (int) values[b];
		}
		free(values);
	}
	free(nodeName);
}

/**
 * Read integer data into the given array.
 *
 */
void Util_XML_Read_Fishery_Param_Double(MSEBoxModel *bm, char *fileName, xmlNodePtr parent, char *paramName, int checkType, double **targetArray, int size) {
	int fisheryIndex, b;
	xmlNodePtr attributeGroup;
	double *values = 0;
	char errorString[STRLEN];
	char *nodeName =  Util_Get_Node_Name(parent);

	snprintf(errorString, sizeof(errorString), "%s/%s", nodeName, paramName);

	attributeGroup = Util_XML_Get_Node(ATLANTIS_ATTRIBUTE, parent, paramName);
	if (attributeGroup == NULL)
		quit("readFisheryGroupParamXML - %s attribute group not found.\n", errorString);

	if (verbose)
		printf("Read fisheries groups parameters %s\n", paramName);

	for (fisheryIndex = 0; fisheryIndex < bm->K_num_fisheries; fisheryIndex++) {
		if (Util_XML_Read_Array_Double(ATLANTIS_FISHERY_ATTRIBUTE, fileName, errorString, attributeGroup, checkType, FisheryArray[fisheryIndex].fisheryCode, &values, size)
				== FALSE) {
			quit("Error: Unable to find parameter '%s/%s' in input file %s\n", errorString, FisheryArray[fisheryIndex].fisheryCode, fileName);
		}

		for (b = 0; b < size; b++) {
			targetArray[fisheryIndex][b] = values[b];
		}
		free(values);
	}
	free(nodeName);
}

void Util_XML_Read_Fished_Group_Param(MSEBoxModel *bm, char *fileName, xmlNodePtr parent, char *paramName, int check_type, double **targetArray, int size) {
	int guild, b;
	xmlNodePtr attributeGroup;
	double *values = 0;
	char errorString[STRLEN];
	char *nodeName =  Util_Get_Node_Name(parent);

	snprintf(errorString, sizeof(errorString), "%s/%s", nodeName, paramName);

	attributeGroup = Util_XML_Get_Node(ATLANTIS_ATTRIBUTE, parent, paramName);
	if (attributeGroup == NULL)
		quit("Util_XML_Read_Fished_Group_Param - %s attribute group not found.\n", errorString);

	if (verbose > 1)
		printf("Read fisheries groups parameters %s\n", paramName);

	for (guild = 0; guild < bm->K_num_tot_sp; guild++) {
		if (FunctGroupArray[guild].isFished == TRUE) {

			if (Util_XML_Read_Array_Double(ATLANTIS_GROUP_ATTRIBUTE, fileName, errorString, attributeGroup, check_type, FunctGroupArray[guild].groupCode, &values, size)
					== FALSE) {
				quit("Error: Unable to find parameter '%s/%s' in input file %s\n", errorString, FunctGroupArray[guild].groupCode, fileName);
			}

			for (b = 0; b < size; b++) {
				targetArray[guild][b] = values[b];
			}
			free(values);
		}
	}
	free(nodeName);
}

void Util_XML_Read_Impacted_Group_Param(MSEBoxModel *bm, char *fileName, xmlNodePtr parent, int paramID) {
	int guild, b;
	xmlNodePtr attributeGroup;
	int index;
	double *values = 0;
	char errorString[STRLEN];
	char *nodeName =  Util_Get_Node_Name(parent);


	index = Util_XML_Get_Fishery_Param_Index(FisheryGroupParamsArray, numGroupFisheryParams, paramID);
	snprintf(errorString, sizeof(errorString), "%s/%s", nodeName, FisheryGroupParamsArray[index].tag);

	attributeGroup = Util_XML_Get_Node(ATLANTIS_ATTRIBUTE, parent, FisheryGroupParamsArray[index].tag);
	if (attributeGroup == NULL)
		quit("Util_XML_Read_Impacted_Group_Param - %s attribute group not found.\n", errorString);

    if(verbose)
        printf("Read fisheries groups parameters %s\n", FisheryGroupParamsArray[index].tag);

    for (guild = 0; guild < bm->K_num_tot_sp; guild++) {
		if (FunctGroupArray[guild].isImpacted == TRUE) {

			if (Util_XML_Read_Array_Double(ATLANTIS_GROUP_ATTRIBUTE, fileName, errorString, attributeGroup, FisheryGroupParamsArray[index].checkType,
					FunctGroupArray[guild].groupCode, &values, bm->K_num_fisheries) == FALSE) {
				quit("Error: Unable to find parameter '%s/%s' in input file %s\n", errorString, FunctGroupArray[guild].groupCode,
						fileName);
			}

			for (b = 0; b < bm->K_num_fisheries; b++) {
				bm->SP_FISHERYprms[guild][b][paramID] = values[b];
			}
			free(values);
		}
	}
	free(nodeName);
}

void Util_XML_Read_Fishery_Group_Param(MSEBoxModel *bm, char *fileName, xmlNodePtr parent, int paramID) {
	int guild, b;
	xmlNodePtr attributeGroup;
	int index;
	double *values = 0;
	char errorString[STRLEN];
	char *nodeName =  Util_Get_Node_Name(parent);


	index = Util_XML_Get_Fishery_Param_Index(FisheryGroupParamsArray, numGroupFisheryParams, paramID);

	snprintf(errorString, sizeof(errorString), "%s/%s", nodeName, FisheryGroupParamsArray[index].tag);

	attributeGroup = Util_XML_Get_Node(ATLANTIS_ATTRIBUTE, parent, FisheryGroupParamsArray[index].tag);
	if (attributeGroup == NULL)
		quit("Util_XML_Read_Fishery_Group_Param - %s attribute group not found.\n", errorString);

	if (verbose)
		printf("Read fisheries groups parameters %s\n", FisheryGroupParamsArray[index].tag);

	for (guild = 0; guild < bm->K_num_tot_sp; guild++) {
		if (FunctGroupArray[guild].isFished == TRUE) {

			if (Util_XML_Read_Array_Double(ATLANTIS_GROUP_ATTRIBUTE, fileName, errorString, attributeGroup, FisheryGroupParamsArray[index].checkType,
					FunctGroupArray[guild].groupCode, &values, bm->K_num_fisheries) == FALSE) {
				quit("Error: Unable to find parameter '%s/%s' in input file %s\n", errorString, FunctGroupArray[guild].groupCode,
						fileName);
			}

			for (b = 0; b < bm->K_num_fisheries; b++) {
				bm->SP_FISHERYprms[guild][b][paramID] = values[b];
			}
			free(values);
		}
	}
	free(nodeName);
}

void Util_XML_Read_Change_Values(MSEBoxModel *bm, char *fileName, xmlNodePtr attributeGroup, char *paramName, int paramID, double ***array) {
	xmlNodePtr groupNode;
	double *values = 0;
	int fisheryIndex = -1;
	int counter, b, index;
	char *paramStr[] =
		{ "ChangeStart", "ChangePeriod", "ChangeMult" };

	char errorString[STRLEN];
	char *nodeName =  Util_Get_Node_Name(attributeGroup);



	for (index = 0; index < 3; index++) {

		snprintf(errorString, sizeof(errorString), "%s/%s", nodeName, paramStr[index]);
		groupNode = Util_XML_Get_Node(ATLANTIS_ATTRIBUTE, attributeGroup, paramStr[index]);
		if (groupNode == NULL) {
			quit("Util_XML_Read_Change_Values attributeNode not found %s\n", errorString);
		}
		for (fisheryIndex = 0; fisheryIndex < bm->K_num_fisheries; fisheryIndex++) {

			counter = (int) bm->FISHERYprms[fisheryIndex][paramID];

			if (Util_XML_Read_Array_Double(ATLANTIS_FISHERY_ATTRIBUTE, fileName, errorString, groupNode, no_checking, FisheryArray[fisheryIndex].fisheryCode, &values,
					counter) == FALSE) {
				quit("Error: Unable to find parameter '%s/%s' in input file %s\n", errorString, FisheryArray[fisheryIndex].fisheryCode, fileName);
			}

			for (b = 0; b < counter; b++) {
				array[fisheryIndex][b][index] = values[b];
			}
			free(values);
		}
	}
	free(nodeName);
}

void Util_XML_Read_Change_Fished_Groups(MSEBoxModel *bm, char *fileName, xmlNodePtr attributeGroup, char *paramName, int paramID, double ****array) {
	xmlNodePtr groupNode;
	double *values = 0;
	int counter, b, index, speciesIndex, i;
	int check = no_checking;
	char *paramStr[] =
		{ "ChangeStart", "ChangePeriod", "ChangeMult" };
	char errorString[STRLEN];
	char *nodeName =  Util_Get_Node_Name(attributeGroup);


	for (index = 0; index < 3; index++) {

		if (index == 0)
			check = integer_check;
		else{
			check = no_checking;
		}

		snprintf(errorString, sizeof(errorString), "%s/%s", nodeName, paramStr[index]);
		groupNode = Util_XML_Get_Node(ATLANTIS_ATTRIBUTE, attributeGroup, paramStr[index]);
		if (groupNode == NULL) {
			quit("Util_XML_Read_Change_Fished_Groups attributeNode not found %s\n", errorString);
		}

		for (speciesIndex = 0; speciesIndex < bm->K_num_tot_sp; speciesIndex++) {

			if (FunctGroupArray[speciesIndex].isImpacted == TRUE) {
				counter = 0;
                for (i = 0; i < bm->K_num_fisheries; i++) {
                    counter = max ( counter,(int) bm->SP_FISHERYprms[speciesIndex][i][paramID] );
                }

				if(counter > 0){
					if (Util_XML_Read_Array_Double(ATLANTIS_GROUP_ATTRIBUTE, fileName, errorString, groupNode, check, FunctGroupArray[speciesIndex].groupCode, &values,
							counter) == FALSE) {
						quit("Error: Unable to find parameter '%s/%s' in input file %s\n", errorString, FunctGroupArray[speciesIndex].groupCode, fileName);
					}

					for (i = 0; i < bm->K_num_fisheries; i++)
						for (b = 0; b < bm->SP_FISHERYprms[speciesIndex][i][paramID]; b++)
							array[speciesIndex][i][b][index] = values[b];

					free(values);
				}
			}
		}
	}
	free(nodeName);
}


void Util_XML_Read_Vert_Fishery_Param(MSEBoxModel *bm, char *fileName, xmlNodePtr parent, int paramID) {
	int guild, b;
	xmlNodePtr attributeGroup;
	int index;
	double *values = 0;
	char errorString[STRLEN];
	char *nodeName =  Util_Get_Node_Name(parent);


	index = Util_XML_Get_Fishery_Param_Index(FisheryGroupParamsArray, numGroupFisheryParams, paramID);

	snprintf(errorString, sizeof(errorString), "%s/%s", nodeName, FisheryGroupParamsArray[index].tag);
	attributeGroup = Util_XML_Get_Node(ATLANTIS_ATTRIBUTE, parent, FisheryGroupParamsArray[index].tag);
	if (attributeGroup == NULL)
		quit("Util_XML_Read_Vert_Fishery_Param - %s attribute group not found.\n", errorString);
	if (verbose)
		printf("Read vertebrate groups parameters %s\n", FisheryGroupParamsArray[index].tag);

	for (guild = 0; guild < bm->K_num_tot_sp; guild++) {
		if (FunctGroupArray[guild].isVertebrate == TRUE) {

			if (Util_XML_Read_Array_Double(ATLANTIS_GROUP_ATTRIBUTE, fileName, errorString, attributeGroup, FisheryGroupParamsArray[index].checkType,
					FunctGroupArray[guild].groupCode, &values, bm->K_num_fisheries) == FALSE) {
				quit("Error: Unable to find parameter '%s/%s' in input file %s\n", errorString, FunctGroupArray[guild].groupCode,
						fileName);
			}

			for (b = 0; b < bm->K_num_fisheries; b++) {
				bm->SP_FISHERYprms[guild][b][paramID] = values[b];
			}
			free(values);
		}
	}
	free(nodeName);
}
