/*******************************************************************//**
 \file
 \brief Functions to load parameters from the economic input file.
 \ingroup ateconomic

 File:			atbiolParamIO.c

 Purpose:        Functions to load parameters from the economic input file.


 Revisions:      20/05/2008 Bec Gorton

 Made sure each function prints its name if the species
 index or fishery index is not found.

 28-10-2010 Bec Gorton
 Changed code to read in the new XML format.

 *********************************************************************/

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>
#include <ctype.h>
#include <sjwlib.h>
#include <netcdf.h>
#include <atEconomic.h>


#define numEconIndicators 34
SpeciesParamStruct econIndicatorInputs[numEconIndicators] =
	{
		{ "NumBoats", nboat_id, "_nboat", integer_check, 1.0, -1 },
		{ "BoatSize", boat_size_id, "_boat_size", integer_check, 1.0, -1 },
		{ "HoldCapacity", hold_capacity_id, "_hold_capacity", integer_check, 1.0, -1 },
		{ "CrewSize", crew_size_id, "_crewsize", integer_check, 1.0, -1 },
		{ "HomePort", home_port_id, "_home_port", integer_check, 1.0, -1 },
		{ "FCWeightScale", FCwgtscale_id, "_FCwgtscale", integer_check, 1.0, -1 },
		{ "TripLength", max_trip_length_id, "_triplength", integer_check, 1.0, -1 },
		{ "FishableLength", FishableLength_id, "_FishableLength", integer_check, 1.0, -1 },
		{ "MaxShotVol", MaxShotVol_id, "_MaxShotVol", integer_check, 1.0, -1 },
		{ "ShotLength", ShotLength_id, "_ShotLength", integer_check, 1.0, -1 },
		{ "MinDownTime", down_time_id, "_minDownTime", no_checking, 1.0, -1 },
		{ "ChoiceBuffer", choicebuffer_id, "_choicebuffer", no_checking, 1.0, -1 },
		{ "EffortDiscount", effortDiscount_id, "_EffortDiscount", no_checking, 1.0, -1 },
		{ "BycatchDiscount", bycatchDiscount_id, "_BycatchDiscount", no_checking, 1.0, -1 },
		{ "VariableCosts", var_cost_id, "_varcost", no_checking, 1.0, -1 },
		{ "FuelPerDay", fuel_cost_id, "_fuelcost", no_checking, 1.0, -1 },
		{ "GearCosts", gear_cost_id, "_gearcost", no_checking, 1.0, -1 },
		{ "UnloadCosts", unload_cost_id, "_unloadcost", no_checking, 1.0, -1 },
		{ "FixedCosts", fixed_cost_id, "_fixedcost", no_checking, 1.0, -1 },
		{ "CapitalCosts", capital_cost_id, "_capitalcost", no_checking, 1.0, -1 },
		{ "ToleratedDebt", tol_debt_id, "_TolDebt", no_checking, 1.0, -1 },
		{ "CrewShare", crewshare_id, "_crewshare", no_checking, 1.0, -1 },
		{ "FlexWeight", flexweight_orig_id, "_flexweight", no_checking, 1.0, -1 },
		{ "Resale", resale_id, "_resale", no_checking, 1.0, -1 },
		{ "BuyOut", buyout_id, "_buyout", no_checking, 1.0, -1 },
		{ "SuppCost", supp_cost_id, "_suppcost", no_checking, 1.0, -1 },
		{ "SwitchCost", switch_cost_id, "_switchcost", no_checking, 1.0, -1 },
		{ "NewBoatCost", newboat_cost_id, "_newcost", no_checking, 1.0, -1 },
		{ "ManagementCosts", mgmt_cost_id, "_mgmtcost", no_checking, 1.0, -1 },
		{ "PropBuyBack", propbuyback_id, "_propbuyback", no_checking, 1.0, -1 },
		{ "EconIndicatorType", flag_econind_id, "_flageconind", binary_check, 1.0, -1 },
		{ "EconIndicatorCombination", flag_indtype_id, "_flagtypeind", binary_check, 1.0, -1 },
		{ "Betarev", betarev_id, "_betarev$", no_checking, 1.0, -1 },
		{ "MinProb", minprob_id, "_minprob$", no_checking, 1.0, -1 },

	};

void readEconIndicatorParamXML(MSEBoxModel *bm, char *fileName, xmlNodePtr parent, int paramID) {
	int fisheryIndex;
	xmlNodePtr attributeGroup;
	int index;
	double *values = 0;
	int ns;
	char errorString[STRLEN];
	char *nodeName;

	nodeName =  Util_Get_Node_Name(parent);

	index = Util_XML_Get_Fishery_Param_Index(econIndicatorInputs, numEconIndicators, paramID);

	snprintf(errorString, sizeof(errorString), "%s/%s", nodeName, econIndicatorInputs[index].tag);
	attributeGroup = Util_XML_Get_Node(ATLANTIS_ATTRIBUTE, parent, econIndicatorInputs[index].tag);
	if (attributeGroup == NULL)
		quit("readEconIndicatorParamXML - %s attribute group not found.\n", errorString);

	if(verbose > 1)
		printf("Read fisheries groups parameters %s\n", econIndicatorInputs[index].tag);

	for (fisheryIndex = 0; fisheryIndex < bm->K_num_fisheries; fisheryIndex++) {

		if(Util_XML_Read_Array_Double(ATLANTIS_FISHERY_ATTRIBUTE, fileName, errorString, attributeGroup, econIndicatorInputs[index].checkType, FisheryArray[fisheryIndex].fisheryCode,
				&values, (int)bm->FISHERYprms[fisheryIndex][nsubfleets_id]) == FALSE){
			quit("Error: Unable to find parameter '%s/%s' in input file %s\n", errorString,  FisheryArray[fisheryIndex].fisheryCode, fileName);
		}

		for (ns = 0; ns < bm->FISHERYprms[fisheryIndex][nsubfleets_id]; ns++) {
			bm->SUBFLEET_ECONprms[fisheryIndex][ns][paramID] = values[ns];
		}
		free(values);
	}
	free(nodeName);
}

void Read_Economic_Flags(MSEBoxModel *bm, char *fileName, xmlNodePtr rootnode) {

	xmlNodePtr groupingNode, childGroupingNode;
	groupingNode = Util_XML_Get_Node(ATLANTIS_ATTRIBUTE_SUB_GROUP, rootnode, "Economic_Flags");
	if (groupingNode == NULL)
		quit("Read_Economic_Flags: Economic_Flags attribute group not found in input file %s.\n", fileName);

	/* General economic fleet dynamics flags */

	bm->immed_cost_only = (int) Util_XML_Read_Value(fileName, ATLANTIS_ATTRIBUTE,  bm->ecotest, 1, groupingNode, binary_check, "immed_cost_only");
	bm->UseConstPrice = (int) Util_XML_Read_Value(fileName, ATLANTIS_ATTRIBUTE,  bm->ecotest, 1, groupingNode, binary_check, "UseConstPrice");
	bm->UseEconQuotaDefine = (int) Util_XML_Read_Value(fileName, ATLANTIS_ATTRIBUTE,  bm->ecotest, 1, groupingNode, binary_check, "UseEconQuotaDefine");
	bm->UseMinValue = (int) Util_XML_Read_Value(fileName, ATLANTIS_ATTRIBUTE,  bm->ecotest, 1, groupingNode, binary_check, "UseMinValue");
	bm->MultiPlanEffort = (int) Util_XML_Read_Value(fileName, ATLANTIS_ATTRIBUTE,  bm->ecotest, 1, groupingNode, binary_check, "MultiPlanEffort");
	bm->EconLimDemand = (int) Util_XML_Read_Value(fileName, ATLANTIS_ATTRIBUTE,  bm->ecotest, 1, groupingNode, binary_check, "EconLimDemand");
	bm->OrigEconCalc = (int) Util_XML_Read_Value(fileName, ATLANTIS_ATTRIBUTE,  bm->ecotest, 1, groupingNode, binary_check, "OrigEconCalc");
    bm->MonthlyProfitUpdate = (int) Util_XML_Read_Value(fileName, ATLANTIS_ATTRIBUTE,  bm->ecotest, 1, groupingNode, binary_check, "MonthlyProfitUpdate");
    bm->econweekly = (int) Util_XML_Read_Value(fileName, ATLANTIS_ATTRIBUTE,  bm->ecotest, 1, groupingNode, binary_check, "econweekly");
	bm->quota_trading = (int) Util_XML_Read_Value(fileName, ATLANTIS_ATTRIBUTE,  bm->ecotest, 1, groupingNode, binary_check, "quota_trading");

	bm->fish_withoutQ = (int) Util_XML_Read_Value(fileName, ATLANTIS_ATTRIBUTE,  bm->ecotest, 1, groupingNode, binary_check, "fish_withoutQ");
	bm->stochastic_effort = (int) Util_XML_Read_Value(fileName, ATLANTIS_ATTRIBUTE,  bm->ecotest, 1, groupingNode, binary_check, "stochastic_effort");
	bm->flagallmonths = (int) Util_XML_Read_Value(fileName, ATLANTIS_ATTRIBUTE,  bm->ecotest, 1, groupingNode, binary_check, "flagallmonths");
	bm->flagslowsell = (int) Util_XML_Read_Value(fileName, ATLANTIS_ATTRIBUTE,  bm->ecotest, 1, groupingNode, binary_check, "flagslowsell");
	bm->flagnewboat = (int) Util_XML_Read_Value(fileName, ATLANTIS_ATTRIBUTE,  bm->ecotest, 1, groupingNode, binary_check, "flagnewboat");
	bm->flagsupp_allowed = (int) Util_XML_Read_Value(fileName, ATLANTIS_ATTRIBUTE,  bm->ecotest, 1, groupingNode, binary_check, "flagsupp_allowed");
	bm->sp_by_sp = (int) Util_XML_Read_Value(fileName, ATLANTIS_ATTRIBUTE,  bm->ecotest, 1, groupingNode, binary_check, "sp_by_sp");
	bm->flagLease = (int) Util_XML_Read_Value(fileName, ATLANTIS_ATTRIBUTE,  bm->ecotest, 1, groupingNode, integer_check, "flagLease");
	bm->yr_horizon = (int) Util_XML_Read_Value(fileName, ATLANTIS_ATTRIBUTE,  bm->ecotest, 1, groupingNode, integer_check, "yr_horizon");
	bm->interest_rate = Util_XML_Read_Value(fileName, ATLANTIS_ATTRIBUTE,  bm->ecotest, 1, groupingNode, no_checking, "interest_rate");

	bm->hist_only = (int) Util_XML_Read_Value(fileName, ATLANTIS_ATTRIBUTE,  bm->ecotest, 1, groupingNode, binary_check, "hist_only");
	bm->hist_only_timeout = Util_XML_Read_Value(fileName, ATLANTIS_ATTRIBUTE,  bm->ecotest, 1, groupingNode, integer_check, "hist_only_timeout");
    bm->rescale_post_burnin = (int)Util_XML_Read_Value(fileName, ATLANTIS_ATTRIBUTE,  bm->ecotest, 1, groupingNode, binary_check, "rescale_post_burnin");

	bm->DVstart = Util_XML_Read_Value(fileName, ATLANTIS_ATTRIBUTE,  bm->ecotest, 1, groupingNode, no_checking, "DVstart");
	bm->TaxStart = Util_XML_Read_Value(fileName, ATLANTIS_ATTRIBUTE,  bm->ecotest, 1, groupingNode, no_checking, "TaxStart");
	bm->UseVPUE = (int) Util_XML_Read_Value(fileName, ATLANTIS_ATTRIBUTE,  bm->ecotest, 1, groupingNode, integer_check, "UseVPUE");
	bm->month_scalar = Util_XML_Read_Value(fileName, ATLANTIS_ATTRIBUTE,  bm->ecotest, 1, groupingNode, integer_check, "month_scalar");

	bm->prop_leave = Util_XML_Read_Value(fileName, ATLANTIS_ATTRIBUTE,  bm->ecotest, 1, groupingNode, proportion_check, "prop_leave");
	bm->prop_switch = Util_XML_Read_Value(fileName, ATLANTIS_ATTRIBUTE,  bm->ecotest, 1, groupingNode, proportion_check, "prop_switch");
	bm->prop_supp = Util_XML_Read_Value(fileName, ATLANTIS_ATTRIBUTE,  bm->ecotest, 1, groupingNode, proportion_check, "prop_supp");
	bm->prop_boatbound = Util_XML_Read_Value(fileName, ATLANTIS_ATTRIBUTE,  bm->ecotest, 1, groupingNode, proportion_check, "prop_boatbound");

	printf("********");
	fflush(stdout);

	printf("********");
	fflush(stdout);

	bm->minValue = Util_XML_Read_Value(fileName, ATLANTIS_ATTRIBUTE,  bm->ecotest, 1, groupingNode, no_checking, "minValue");

	if (!bm->minValue && bm->UseMinValue && (bm->flagLease == NZmodel))
		quit("Can not have zero minimum value for Newell quota model as will need to take the log, please set to a balue (e.g. 0.01)\n");

	flaglbs = (int) Util_XML_Read_Value(fileName, ATLANTIS_ATTRIBUTE,  bm->ecotest, 1, groupingNode, binary_check, "flaglbs");
    
    if(bm->rescale_post_burnin) {
        printf("Attempting to read in rescaling\n");
        
        childGroupingNode = Util_XML_Get_Node(ATLANTIS_ATTRIBUTE_SUB_GROUP, rootnode, "Rescaling");
        if (childGroupingNode == NULL)
            quit("readEconXML: Rescaling attribute group not found in input file %s.\n", fileName);

        
        if(	Util_XML_Read_Array_Double(ATLANTIS_ATTRIBUTE, fileName, "Economic_Flags/Rescaling", childGroupingNode, no_checking, "ReScale_Eff", &bm->EffortReScale, bm->K_num_fisheries) == FALSE){
            quit("Error: Unable to find parameter 'Economic_Flags/Rescaling/ReScale_Effort' in input file %s\n",  fileName);
        }
        
        printf("Read in EffortReScale\n");

        if(	Util_XML_Read_Array_Double(ATLANTIS_ATTRIBUTE, fileName, "Economic_Flags/Rescaling", childGroupingNode, no_checking, "ReScale_Cat", &bm->CatchReScale, bm->K_num_tot_sp) == FALSE){
            quit("Error: Unable to find parameter 'Economic_Flags/Rescaling/ReScale_Catch' in input file %s\n",  fileName);
        }
    }
    
}

void Read_Vessel_Characteristics_XML(MSEBoxModel *bm, char *fileName, xmlNodePtr rootnode) {

	xmlNodePtr groupingNode;
	groupingNode = Util_XML_Get_Node(ATLANTIS_ATTRIBUTE_SUB_GROUP, rootnode, "Vessel_Characteristics");
	if (groupingNode == NULL)
		quit("Read_Vessel_Characteristics_XML: Vessel_characteristics attribute group not found in input file %s.\n", fileName);

	/* Vessel characteristics */
	//Util_XML_Read_Fishery_Param(bm, fileName, groupingNode, nsubfleets_id);

	/* Maximum number of licences the fishery can hold */
	Util_XML_Read_Fishery_Param(bm, fileName, groupingNode, nlicence_id);

	/* Vessel targeting considerations */
	Util_XML_Read_Fishery_Param(bm, fileName, groupingNode, flagMultiSpEffort_id);

	/* Use of deemed value */
	Util_XML_Read_Fishery_Param(bm, fileName, groupingNode, flagDV_id);

	/* Deemed value */
	Util_XML_Read_Impacted_Group_Param(bm, fileName, groupingNode, deemedvalue_id);

}

void Read_ByCatch_Incentives_XML(MSEBoxModel *bm, char *fileName, xmlNodePtr rootnode) {

	xmlNodePtr groupingNode;

	groupingNode = Util_XML_Get_Node(ATLANTIS_ATTRIBUTE_SUB_GROUP, rootnode, "Bycatch_Incentives");
	if (groupingNode == NULL)
		quit("Read_ByCatch_Incentives_XML: Bycatch_Incentives attribute group not found in input file %s.\n", fileName);

	/* Bycatch incentive flags */
	bm->UsingEffortIncentive = (int) Util_XML_Read_Value(fileName, ATLANTIS_ATTRIBUTE,  bm->ecotest, 1, groupingNode, binary_check, "UsingEffortIncentive");
	bm->BycatchCountDiscards = (int) Util_XML_Read_Value(fileName, ATLANTIS_ATTRIBUTE,  bm->ecotest, 1, groupingNode, binary_check, "BycatchCountDiscards");
	bm->TemporalBycatchAvoid = (int) Util_XML_Read_Value(fileName, ATLANTIS_ATTRIBUTE,  bm->ecotest, 1, groupingNode, binary_check, "TemporalBycatchAvoid");
	bm->SpatialBycatchAvoid = (int) Util_XML_Read_Value(fileName, ATLANTIS_ATTRIBUTE,  bm->ecotest, 1, groupingNode, binary_check, "SpatialBycatchAvoid");

}

static void Read_Market_Value(MSEBoxModel *bm, char *fileName, xmlNodePtr parent, char *paramName, int paramIndex) {
	double *values = 0;
	int index, sp;
	xmlNodePtr attributeNode;
	char str[100];

	attributeNode = Util_XML_Get_Node(ATLANTIS_ATTRIBUTE, parent, paramName);
	if (attributeNode == NULL)
		quit("Market_Characteristics/Market_Intercept attribute group not found in input file %s.\n", fileName);

	for (index = 0; index < bm->K_num_markets; index++) {
		snprintf(str, sizeof(str), "Market_%d", index);
		if(Util_XML_Read_Array_Double(ATLANTIS_ATTRIBUTE, fileName, "Market_Characteristics/Market_Intercept", attributeNode, no_checking, str, &values, bm->K_num_tot_sp) == FALSE){
			quit("Error: Unable to find parameter 'Market_Characteristics/Market_Intercept/%s' in input file %s\n", str, fileName);
		}
		for (sp = 0; sp < bm->K_num_tot_sp; sp++) {
			bm->MARKETprms[index][sp][paramIndex] = values[sp];
		}
		free(values);
	}
}

static void Read_Monthly_Market_Value(MSEBoxModel *bm, char *fileName, xmlNodePtr parent) {
	char str[100];
	int sp, index, month;
	double *values = 0;
	char errorString[STRLEN];
	xmlNodePtr groupingNode, attributeNode;

	groupingNode = Util_XML_Get_Node(ATLANTIS_ATTRIBUTE, parent, "MonthlyMarket");
	if (groupingNode == NULL)
		quit("Read_Monthly_Market_Value: MonthlyMarket attribute group not found in input file %s.\n", fileName);

	for (index = 0; index < bm->K_num_markets; index++) {
		snprintf(str, sizeof(str), "MarketCoefft%d", index + 1);

		snprintf(errorString, sizeof(errorString), "Market_Characteristics/Market_Intercept/%s", str);
		attributeNode = Util_XML_Get_Node(ATLANTIS_ATTRIBUTE, groupingNode, str);
		if (attributeNode == NULL)
			quit("Market_Characteristics/Market_Intercept/%s attribute group not found.\n", str);

		for (month = 0; month < 12; month++) {
			if(Util_XML_Read_Array_Double(ATLANTIS_ATTRIBUTE, fileName, errorString, attributeNode, no_checking, monthStrings[month], &values, bm->K_num_tot_sp) == FALSE){
				quit("Error: Unable to find parameter '%s/%s' in input file %s\n", errorString, monthStrings[month], fileName);
			}
			for (sp = 0; sp < bm->K_num_tot_sp; sp++) {
				bm->MARKETmonth[index][sp][month] = values[sp];
			}
			free(values);
		}
	}
}

/**
 *	\brief Create the XML structure to hold the seasonal effort distribution for each fishery.
 */
void Read_Market_XML(MSEBoxModel *bm, char *fileName, xmlNodePtr parent) {

	xmlNodePtr groupingNode;

	groupingNode = Util_XML_Get_Node(ATLANTIS_ATTRIBUTE, parent, "Market");
	if (groupingNode == NULL)
		quit("Read_Market_XML: Market attribute group not found in input file %s.\n", fileName);

	Read_Market_Value(bm, fileName, groupingNode, "Market_Intercept", market_intercept_id);
	Read_Market_Value(bm, fileName, groupingNode, "Market_Trend", trend_coefft_id);
	Read_Market_Value(bm, fileName, groupingNode, "Market_Autocoefft", auto_coefft_id);
}

void Read_Market_Characteristics_XML(MSEBoxModel *bm, char *fileName, xmlNodePtr rootnode) {

	int sp, i;
	xmlNodePtr groupingNode;

	groupingNode = Util_XML_Get_Node(ATLANTIS_ATTRIBUTE_SUB_GROUP, rootnode, "Market_Characteristics");
	if (groupingNode == NULL)
		quit("Read_Market_Characteristics_XML: Market_Characteristics attribute group not found in input file %s.\n", fileName);

	Read_Market_XML(bm, fileName, groupingNode);
	Read_Monthly_Market_Value(bm, fileName, groupingNode);

	Util_XML_Read_Impacted_Group_Param(bm, fileName, groupingNode, saleprice_id);

	/* Copy the value across to the original sale price field */
	for (sp = 0; sp < bm->K_num_tot_sp; sp++) {
		for (i = 0; i < bm->K_num_fisheries; i++) {
			bm->SP_FISHERYprms[sp][i][origprice_id] = bm->SP_FISHERYprms[sp][i][saleprice_id];
		}
	}

}

void Read_Quota_XML(MSEBoxModel *bm, char *fileName, xmlNodePtr rootnode) {

	xmlNodePtr groupingNode, subGroupingNode;

	groupingNode = Util_XML_Get_Node(ATLANTIS_ATTRIBUTE_SUB_GROUP, rootnode, "Quota_Reconciliation");
	if (groupingNode == NULL)
		quit("Read_Quota_XML: Quota_Reconciliation attribute group not found in input file %s.\n", fileName);

	/* Quota reconciliation */
	bm->recon_buffer = Util_XML_Read_Value(fileName, ATLANTIS_ATTRIBUTE,  bm->ecotest, 1, groupingNode, no_checking, "recon_buffer");

	subGroupingNode = Util_XML_Get_Node(ATLANTIS_ATTRIBUTE_SUB_GROUP, groupingNode, "LeaseValues");
	if (subGroupingNode == NULL)
		quit("Read_Quota_XML: LeaseValues attribute group not found in input file %s.\n", fileName);

	/* Newell et al lease price model coefficients */
	bm->price_Coefft = Util_XML_Read_Value(fileName, ATLANTIS_ATTRIBUTE,  bm->ecotest, 1, subGroupingNode, no_checking, "price_Coefft");
	bm->price_CoefftSq = Util_XML_Read_Value(fileName, ATLANTIS_ATTRIBUTE,  bm->ecotest, 1, subGroupingNode, no_checking, "price_CoefftSq");
	bm->cost_Coefft = Util_XML_Read_Value(fileName, ATLANTIS_ATTRIBUTE,  bm->ecotest, 1, subGroupingNode, no_checking, "cost_Coefft");
	bm->prev_fill_Coefft = Util_XML_Read_Value(fileName, ATLANTIS_ATTRIBUTE,  bm->ecotest, 1, subGroupingNode, no_checking, "prev_fill_Coefft");
	bm->prev_fill_CoefftSq = Util_XML_Read_Value(fileName, ATLANTIS_ATTRIBUTE,  bm->ecotest, 1, subGroupingNode, no_checking, "prev_fill_CoefftSq");
	bm->part_fill_Coefft = Util_XML_Read_Value(fileName, ATLANTIS_ATTRIBUTE,  bm->ecotest, 1, subGroupingNode, no_checking, "part_fill_Coefft");
	bm->part_fill_CoefftSq = Util_XML_Read_Value(fileName, ATLANTIS_ATTRIBUTE,  bm->ecotest, 1, subGroupingNode, no_checking, "part_fill_CoefftSq");
	bm->bind_Coefft = Util_XML_Read_Value(fileName, ATLANTIS_ATTRIBUTE,  bm->ecotest, 1, subGroupingNode, no_checking, "bind_Coefft");
	bm->tac_trend_Coefft = Util_XML_Read_Value(fileName, ATLANTIS_ATTRIBUTE,  bm->ecotest, 1, subGroupingNode, no_checking, "tac_trend_Coefft");
	bm->const_Coefft = Util_XML_Read_Value(fileName, ATLANTIS_ATTRIBUTE,  bm->ecotest, 1, subGroupingNode, no_checking, "const_Coefft");
	bm->GDP_Coefft = Util_XML_Read_Value(fileName, ATLANTIS_ATTRIBUTE,  bm->ecotest, 1, subGroupingNode, no_checking, "GDP_Coefft");
	bm->minValue = Util_XML_Read_Value(fileName, ATLANTIS_ATTRIBUTE,  bm->ecotest, 1, subGroupingNode, no_checking, "minValue");

	subGroupingNode = Util_XML_Get_Node(ATLANTIS_ATTRIBUTE_SUB_GROUP, groupingNode, "FleetSwitching");
	if (subGroupingNode == NULL)
		quit("Read_Quota_XML: FleetSwitching attribute group not found in input file %s.\n", fileName);

	/* Fleet switching and decomission parameters - from Thebaud et al 2006 and Guyader 2002 */
	bm->decom_return_coefft = Util_XML_Read_Value(fileName, ATLANTIS_ATTRIBUTE,  bm->ecotest, 1, subGroupingNode, no_checking, "decom_return_coefft");
	bm->resale_coeff = Util_XML_Read_Value(fileName, ATLANTIS_ATTRIBUTE,  bm->ecotest, 1, subGroupingNode, no_checking, "resale_coeff");
	bm->decom_crew_coefft = Util_XML_Read_Value(fileName, ATLANTIS_ATTRIBUTE,  bm->ecotest, 1, subGroupingNode, no_checking, "decom_crew_coefft");
	bm->switch_return_coefft = Util_XML_Read_Value(fileName, ATLANTIS_ATTRIBUTE,  bm->ecotest, 1, subGroupingNode, no_checking, "switch_return_coefft");
	bm->switch_coefft = Util_XML_Read_Value(fileName, ATLANTIS_ATTRIBUTE,  bm->ecotest, 1, subGroupingNode, no_checking, "switch_coefft");
	bm->max_size_buffer = Util_XML_Read_Value(fileName, ATLANTIS_ATTRIBUTE,  bm->ecotest, 1, subGroupingNode, no_checking, "max_size_buffer");
	bm->min_size_buffer = Util_XML_Read_Value(fileName, ATLANTIS_ATTRIBUTE,  bm->ecotest, 1, subGroupingNode, no_checking, "min_size_buffer");
	bm->new_return_coefft = Util_XML_Read_Value(fileName, ATLANTIS_ATTRIBUTE,  bm->ecotest, 1, subGroupingNode, no_checking, "new_return_coefft");
	bm->new_coefft = Util_XML_Read_Value(fileName, ATLANTIS_ATTRIBUTE,  bm->ecotest, 1, subGroupingNode, no_checking, "new_coefft");

	subGroupingNode = Util_XML_Get_Node(ATLANTIS_ATTRIBUTE_SUB_GROUP, groupingNode, "BuyBack");
	if (subGroupingNode == NULL)
		quit("Read_Quota_XML: BuyBack attribute group not found in input file %s.\n", fileName);

	readEconIndicatorParamXML(bm, fileName, subGroupingNode, propbuyback_id);

	Util_XML_Read_Fishery_Param(bm, fileName, subGroupingNode, buybackdate_id);

	subGroupingNode = Util_XML_Get_Node(ATLANTIS_ATTRIBUTE_SUB_GROUP, groupingNode, "QuotaPurchase");
	if (subGroupingNode == NULL)
		quit("Read_Quota_XML: QuotaPurchase attribute group not found in input file %s.\n", fileName);

	/* Quota purchase coefficents */
	bm->perm_coefft = Util_XML_Read_Value(fileName, ATLANTIS_ATTRIBUTE,  bm->ecotest, 1, subGroupingNode, no_checking, "perm_coefft");
	bm->buy_coefft = Util_XML_Read_Value(fileName, ATLANTIS_ATTRIBUTE,  bm->ecotest, 1, subGroupingNode, no_checking, "buy_coefft");

	/* Relative weighting of target species above other species in preferences */
	bm->targetscale = Util_XML_Read_Value(fileName, ATLANTIS_ATTRIBUTE,  bm->ecotest, 1, subGroupingNode, no_checking, "targetscale");

	/* Proportions where buyng and selling kick-on/off */
	bm->prop_within = Util_XML_Read_Value(fileName, ATLANTIS_ATTRIBUTE,  bm->ecotest, 1, subGroupingNode, no_checking, "prop_within");
	bm->prop_spareend = Util_XML_Read_Value(fileName, ATLANTIS_ATTRIBUTE,  bm->ecotest, 1, subGroupingNode, no_checking, "prop_spareend");


	subGroupingNode = Util_XML_Get_Node(ATLANTIS_ATTRIBUTE_SUB_GROUP, groupingNode, "FriendShip");
	if (subGroupingNode == NULL)
		quit("Read_Quota_XML: FriendShip attribute group not found in input file %s.\n", fileName);

	Util_XML_Get_Value_String(fileName, ATLANTIS_ATTRIBUTE, FALSE, subGroupingNode, "friendship_network_file", bm->friendship_file);

}

static void Read_Functional_Reponse(MSEBoxModel *bm, char *fileName, xmlNodePtr parent, char *paramName, int paramIndex) {
	double *values = 0;
	int index, i;
	xmlNodePtr attributeNode;

	attributeNode = Util_XML_Get_Node(ATLANTIS_ATTRIBUTE, parent, paramName);
	if (attributeNode == NULL)
		quit("Basic_Economic_Indicators/FunctionalResponseParameters attribute group not found in input file %s.\n", fileName);

	for (index = 0; index < K_econ_indicators; index++) {
		if(Util_XML_Read_Array_Double(ATLANTIS_ATTRIBUTE, fileName, "Basic_Economic_Indicators/FunctionalResponseParameters", attributeNode, no_checking, econInputNames[index], &values, K_ecolharvest_indicators) == FALSE){
			quit("Error: Unable to find parameter 'Basic_Economic_Indicators/FunctionalResponseParameters/%s' in input file %s\n", econInputNames[index], fileName);
		}
		for (i = 0; i < K_ecolharvest_indicators; i++) {
			bm->ECON_INDprms[index][i][paramIndex] = values[i];
		}
		free(values);
	}
}

void Read_Econ_Indicators_XML(MSEBoxModel *bm, char *fileName, xmlNodePtr rootnode) {

	xmlNodePtr groupingNode, subGroupingNode;

	groupingNode = Util_XML_Get_Node(ATLANTIS_ATTRIBUTE_SUB_GROUP, rootnode, "Basic_Economic_Indicators");
	if (groupingNode == NULL)
		quit("Basic_Economic_Indicators: Basic_Economic_Indicators attribute group not found in input file %s.\n", fileName);

	readEconIndicatorParamXML(bm, fileName, groupingNode, flag_econind_id);
	readEconIndicatorParamXML(bm, fileName, groupingNode, flag_indtype_id);

	subGroupingNode = Util_XML_Get_Node(ATLANTIS_ATTRIBUTE_SUB_GROUP, groupingNode, "FunctionalResponseParameters");
	if (subGroupingNode == NULL)
		quit("Basic_Economic_Indicators: FunctionalResponseParameters attribute group not found in input file %s.\n", fileName);

	Read_Functional_Reponse(bm, fileName, subGroupingNode, "FunctionalResponseType", eindtype_ind);
	Read_Functional_Reponse(bm, fileName, subGroupingNode, "FunctionalResponseConstant", ecoefft_ind);
	Read_Functional_Reponse(bm, fileName, subGroupingNode, "FunctionalResponseSlope", escale_ind);
	Read_Functional_Reponse(bm, fileName, subGroupingNode, "FunctionalResponseSlopeB", escaleB_ind);
	Read_Functional_Reponse(bm, fileName, subGroupingNode, "FunctionalResponseThreshold", ethresh_ind);
}

/**
 *
 */
void Read_Subfleet_Characteristics_XML(MSEBoxModel *bm, char *fileName, xmlNodePtr rootnode) {

	int index;
	double *values = 0;
	xmlNodePtr groupingNode, subGroupingNode, fuelNode, debtNode;

	groupingNode = Util_XML_Get_Node(ATLANTIS_ATTRIBUTE_SUB_GROUP, rootnode, "Subfleet_Characteristics");
	if (groupingNode == NULL)
		quit("Read_Subfleet_Characteristics_XML: Subfleet_Characteristics attribute group not found in input file %s.\n", fileName);

	readEconIndicatorParamXML(bm, fileName, groupingNode, nboat_id);
	readEconIndicatorParamXML(bm, fileName, groupingNode, boat_size_id);
	readEconIndicatorParamXML(bm, fileName, groupingNode, hold_capacity_id);
	readEconIndicatorParamXML(bm, fileName, groupingNode, crew_size_id);
	readEconIndicatorParamXML(bm, fileName, groupingNode, home_port_id);

	subGroupingNode = Util_XML_Get_Node(ATLANTIS_ATTRIBUTE_SUB_GROUP, groupingNode, "SubfleetEffort");
	if (subGroupingNode == NULL)
		quit("Subfleet_Characteristics/SubfleetEffort attribute group not found in input file %s\n", fileName);

	readEconIndicatorParamXML(bm, fileName, subGroupingNode, FCwgtscale_id);
	readEconIndicatorParamXML(bm, fileName, subGroupingNode, max_trip_length_id);
	readEconIndicatorParamXML(bm, fileName, subGroupingNode, FishableLength_id);
	readEconIndicatorParamXML(bm, fileName, subGroupingNode, MaxShotVol_id);
	readEconIndicatorParamXML(bm, fileName, subGroupingNode, ShotLength_id);
	readEconIndicatorParamXML(bm, fileName, subGroupingNode, down_time_id);
	readEconIndicatorParamXML(bm, fileName, subGroupingNode, choicebuffer_id);
	readEconIndicatorParamXML(bm, fileName, subGroupingNode, effortDiscount_id);
	readEconIndicatorParamXML(bm, fileName, subGroupingNode, bycatchDiscount_id);

	subGroupingNode = Util_XML_Get_Node(ATLANTIS_ATTRIBUTE_SUB_GROUP, groupingNode, "SubfleetCosts");
	if (subGroupingNode == NULL)
		quit("Subfleet_Characteristics/SubfleetCosts attribute group not found in input file %s\n", fileName);

	bm->shorecost = (int) Util_XML_Read_Value(fileName, ATLANTIS_ATTRIBUTE,  bm->ecotest, 1, subGroupingNode, integer_check, "shore_costs");
	readEconIndicatorParamXML(bm, fileName, subGroupingNode, var_cost_id);

	/* Fuel costs */
	fuelNode = Util_XML_Get_Node(ATLANTIS_ATTRIBUTE_SUB_GROUP, subGroupingNode, "FuelCosts");
	if (fuelNode == NULL)
		quit("Subfleet_Characteristics/FuelCosts attribute group not found in input file %s\n", fileName);
	bm->FUELprms[fuel_intercept_id] = Util_XML_Read_Value(fileName, ATLANTIS_ATTRIBUTE,  bm->ecotest, 1, fuelNode, no_checking, "fuel_intercept");
	bm->FUELprms[trend_coefft_id] = Util_XML_Read_Value(fileName, ATLANTIS_ATTRIBUTE,  bm->ecotest, 1, fuelNode, no_checking, "fuel_trend_coefft");
	bm->FUELprms[auto_coefft_id] = Util_XML_Read_Value(fileName, ATLANTIS_ATTRIBUTE,  bm->ecotest, 1, fuelNode, no_checking, "fuel_auto_coefft");

	if(Util_XML_Read_Array_Double(ATLANTIS_ATTRIBUTE, fileName, "SubfleetEffort/FuelCosts", fuelNode, no_checking, "FuelMonthCoefft", &values, 12) == FALSE){
		quit("Error: Unable to find parameter 'Subfleet_Characteristics/FuelCosts/FuelMonthCoefft' in input file %s\n", fileName);
	}
	for (index = 0; index < 12; index++) {
		bm->FUELmonth[index] = values[index];
	}
	free(values);

	readEconIndicatorParamXML(bm, fileName, fuelNode, fuel_cost_id);

	/* End of the fuel costs */
	readEconIndicatorParamXML(bm, fileName, subGroupingNode, gear_cost_id);
	readEconIndicatorParamXML(bm, fileName, subGroupingNode, unload_cost_id);
	readEconIndicatorParamXML(bm, fileName, subGroupingNode, fixed_cost_id);

	readEconIndicatorParamXML(bm, fileName, subGroupingNode, capital_cost_id);

	debtNode = Util_XML_Get_Node(ATLANTIS_ATTRIBUTE_SUB_GROUP, subGroupingNode, "GeneralDebt");
	if (debtNode == NULL)
		quit("Read_Subfleet_Characteristics_XML: GeneralDebt attribute group not found in input file %s.\n", fileName);

	/* Debt related parameters */
	bm->effort_reduction = Util_XML_Read_Value(fileName, ATLANTIS_ATTRIBUTE,  bm->ecotest, 1, debtNode, proportion_check, "effort_reduction");
	bm->cripple_nboat = Util_XML_Read_Value(fileName, ATLANTIS_ATTRIBUTE,  bm->ecotest, 1, debtNode, proportion_check, "cripple_nboat_lost");
	bm->cripple_period = (int) Util_XML_Read_Value(fileName, ATLANTIS_ATTRIBUTE,  bm->ecotest, 1, debtNode, integer_check, "cripple_period");

	readEconIndicatorParamXML(bm, fileName, subGroupingNode, tol_debt_id);
	readEconIndicatorParamXML(bm, fileName, subGroupingNode, crewshare_id);
	readEconIndicatorParamXML(bm, fileName, subGroupingNode, flexweight_orig_id);
	readEconIndicatorParamXML(bm, fileName, subGroupingNode, resale_id);
	readEconIndicatorParamXML(bm, fileName, subGroupingNode, buyout_id);
	readEconIndicatorParamXML(bm, fileName, subGroupingNode, supp_cost_id);
	readEconIndicatorParamXML(bm, fileName, subGroupingNode, switch_cost_id);
	readEconIndicatorParamXML(bm, fileName, subGroupingNode, newboat_cost_id);

}

void Read_Own_Quota_XML(MSEBoxModel *bm, char *fileName, xmlNodePtr rootnode) {

	double *values = 0;
	int sp, index, i;
	xmlNodePtr groupingNode, attributeNode;
	char str[500];
	char errorString[STRLEN];

	groupingNode = Util_XML_Get_Node(ATLANTIS_ATTRIBUTE, rootnode, "OwnQuota");
	if (groupingNode == NULL)
		quit("Basic_Economic_Indicators: OwnQuota attribute group not found in input file %s.\n", fileName);

	for (sp = 0; sp < bm->K_num_tot_sp; sp++) {

		if (FunctGroupArray[sp].isFished == TRUE){
			snprintf(errorString, sizeof(errorString), "OwnQuota/%s", FunctGroupArray[sp].groupCode);
			attributeNode = Util_XML_Get_Node(ATLANTIS_ATTRIBUTE, groupingNode, FunctGroupArray[sp].groupCode);
			if (attributeNode == NULL)
				quit("%s attribute group not found.\n", errorString);

			for (index = 0; index < bm->K_max_num_subfleet; index++) {
				snprintf(str, sizeof(str), "sub%d", index + 1);
				if(Util_XML_Read_Array_Double(ATLANTIS_ATTRIBUTE, fileName, errorString, attributeNode, no_checking, str, &values, bm->K_num_fisheries) == FALSE){
					quit("Error: Unable to find parameter '%s/%s' in input file %s\n", errorString, str, fileName);
				}
				for (i = 0; i < bm->K_num_fisheries; i++) {
					bm->QuotaAlloc[i][index][sp][owned_id] = values[i];
				}
				free(values);
			}
		}
	}
}

void Read_Black_Book_XML(MSEBoxModel *bm, char *fileName, xmlNodePtr rootnode) {

	double *values = 0;
	int sp, index, i, monthIndex, fisheryIndex;
	xmlNodePtr groupingNode, subGroupingNode, attributeNode, monthNode;
	char str[500];
	char errorString[STRLEN];

	groupingNode = Util_XML_Get_Node(ATLANTIS_ATTRIBUTE_SUB_GROUP, rootnode, "BlackBook");
	if (groupingNode == NULL)
		quit("Read_Black_Book_XML: BlackBook attribute group not found in input file %s.\n", fileName);

	subGroupingNode = Util_XML_Get_Node(ATLANTIS_ATTRIBUTE, groupingNode, "TemporalBlackBook");
	if (subGroupingNode == NULL)
		quit("Read_Black_Book_XML: TemporalBlackBook attribute group not found in input file %s.\n", fileName);

	for (sp = 0; sp < bm->K_num_tot_sp; sp++) {
		if (FunctGroupArray[sp].isFished == TRUE) {
			attributeNode = Util_XML_Get_Node(ATLANTIS_ATTRIBUTE, subGroupingNode, FunctGroupArray[sp].groupCode);
			if (attributeNode == NULL)
				quit("TemporalBlackBook/%s attribute group not found.\n", FunctGroupArray[sp].groupCode);

			for (monthIndex = 0; monthIndex < 12; monthIndex++) {
				snprintf(errorString, sizeof(errorString), "TemporalBlackBook/%s/%s", FunctGroupArray[sp].groupCode, monthStrings[monthIndex]);
				monthNode = Util_XML_Get_Node(ATLANTIS_ATTRIBUTE, attributeNode, monthStrings[monthIndex]);
				if (monthNode == NULL)
					quit("%s attribute group not found.\n", errorString);

				for (index = 0; index < bm->K_max_num_subfleet; index++) {
					snprintf(str, sizeof(str), "sub%d", index + 1);
					if(Util_XML_Read_Array_Double(ATLANTIS_ATTRIBUTE, fileName, errorString, monthNode, no_checking, str, &values, bm->K_num_fisheries) == FALSE){
						quit("Error: Unable to find parameter '%s/%s' in input file %s\n", errorString, str, fileName);
					}
					for (i = 0; i < bm->K_num_fisheries; i++) {
						bm->BlackBook[i][index][sp][monthIndex][hist_id] = values[i];
					}
					free(values);
				}
			}
		}
	}

	subGroupingNode = Util_XML_Get_Node(ATLANTIS_ATTRIBUTE, groupingNode, "SpatialEffortBlackBook");
	if (subGroupingNode == NULL)
		quit("Read_Black_Book_XML: TemporalBlackBook attribute group not found in input file %s.\n", fileName);

	for (fisheryIndex = 0; fisheryIndex < bm->K_num_fisheries; fisheryIndex++) {
		attributeNode = Util_XML_Get_Node(ATLANTIS_ATTRIBUTE, subGroupingNode, FisheryArray[fisheryIndex].fisheryCode);
		if (attributeNode == NULL)
			quit("SpatialEffortBlackBook/%s attribute group not found.\n", FisheryArray[fisheryIndex].fisheryCode);

		for (monthIndex = 0; monthIndex < 12; monthIndex++) {
			snprintf(errorString, sizeof(errorString), "SpatialEffortBlackBook/%s/%s", FisheryArray[fisheryIndex].fisheryCode, monthStrings[monthIndex]);
			monthNode = Util_XML_Get_Node(ATLANTIS_ATTRIBUTE, attributeNode, monthStrings[monthIndex]);
			if (monthNode == NULL)
				quit("%s attribute group not found.\n", errorString);

			for (index = 0; index < bm->K_max_num_subfleet; index++) {
				snprintf(str, sizeof(str), "sub%d", index + 1);
				if(Util_XML_Read_Array_Double(ATLANTIS_ATTRIBUTE, fileName, errorString, monthNode, no_checking, str, &values, bm->nbox) == FALSE){
					quit("Error: Unable to find parameter '%s/%s' in input file %s\n", errorString, str, fileName);
				}
				for (i = 0; i < bm->nbox; i++) {
					bm->SpatialBlackBook[fisheryIndex][index][monthIndex][i][hist_id] = values[i];
				}
				free(values);
			}
		}
	}

	subGroupingNode = Util_XML_Get_Node(ATLANTIS_ATTRIBUTE, groupingNode, "SpatialCPUEBlackBook");
	if (subGroupingNode == NULL)
		quit("Read_Black_Book_XML: TemporalBlackBook attribute group not found in input file %s.\n", fileName);

	for (fisheryIndex = 0; fisheryIndex < bm->K_num_fisheries; fisheryIndex++) {
		attributeNode = Util_XML_Get_Node(ATLANTIS_ATTRIBUTE, subGroupingNode, FisheryArray[fisheryIndex].fisheryCode);
		if (attributeNode == NULL)
			quit("SpatialCPUEBlackBook/%s attribute group not found.\n", FisheryArray[fisheryIndex].fisheryCode);

		for (monthIndex = 0; monthIndex < 12; monthIndex++) {
			snprintf(errorString, sizeof(errorString), "SpatialCPUEBlackBook/%s/%s", FisheryArray[fisheryIndex].fisheryCode, monthStrings[monthIndex]);
			monthNode = Util_XML_Get_Node(ATLANTIS_ATTRIBUTE, attributeNode, monthStrings[monthIndex]);
			if (monthNode == NULL)
				quit("%s attribute group not found.\n", errorString);

			for (index = 0; index < bm->K_max_num_subfleet; index++) {
				snprintf(str, sizeof(str), "sub%d", index + 1);
				if(Util_XML_Read_Array_Double(ATLANTIS_ATTRIBUTE, fileName, errorString, monthNode, no_checking, str, &values, bm->nbox) == FALSE){
					quit("Error: Unable to find parameter '%s/%s' in input file %s\n", errorString, str, fileName);
				}
				for (i = 0; i < bm->nbox; i++) {
					bm->SpatialCPUE[fisheryIndex][index][monthIndex][i] = values[i];

					/* Start with VPUE matching CPUE */
					bm->SpatialVPUE[fisheryIndex][index][monthIndex][i] = bm->SpatialCPUE[fisheryIndex][index][monthIndex][i];
				}
				free(values);
			}
		}
	}
}

void Read_Management_Costs_XML(MSEBoxModel *bm, char *fileName, xmlNodePtr rootnode) {

	xmlNodePtr groupingNode;

	groupingNode = Util_XML_Get_Node(ATLANTIS_ATTRIBUTE_SUB_GROUP, rootnode, "ManagementCosts");
	if (groupingNode == NULL)
		quit("Read_Management_Costs_XML: ManagementCosts attribute group not found in input file %s.\n", fileName);

	readEconIndicatorParamXML(bm, fileName, groupingNode, mgmt_cost_id);
}
void Read_Dan_Holland_XML(MSEBoxModel *bm, char *fileName, xmlNodePtr rootnode) {

	int sp, i, fisheryIndex, ns;
	double *values = 0;
	xmlNodePtr groupingNode, subGroupingNode;

	groupingNode = Util_XML_Get_Node(ATLANTIS_ATTRIBUTE_SUB_GROUP, rootnode, "DanHolland");
	if (groupingNode == NULL)
		quit("Read_Management_Costs_XML: DanHolland attribute group not found in input file %s.\n", fileName);

	bm->ITQ_think_ahead = (int) Util_XML_Read_Value(fileName, ATLANTIS_ATTRIBUTE,  bm->ecotest, 1, groupingNode, integer_check, "ITQ_think_ahead");
	bm->high_price = (int) Util_XML_Read_Value(fileName, ATLANTIS_ATTRIBUTE,  bm->ecotest, 1, groupingNode, no_checking, "high_price");

	/* Penatly tax */
	Util_XML_Read_Impacted_Group_Param(bm, fileName, groupingNode, tax_id);

	if(Util_XML_Read_Array_Double(ATLANTIS_ATTRIBUTE, fileName, "DanHolland", groupingNode, no_checking, "FixedMinTax", &values, bm->K_num_tot_sp) == FALSE){
		quit("Error: Unable to find parameter 'DanHolland/FixedMinTax' in input file %s\n", fileName);
	}

	for (i = 0; i < bm->K_num_fisheries; i++) {
		for (sp = 0; sp < bm->K_num_tot_sp; sp++) {
			bm->SP_FISHERYprms[sp][i][FixedMinTax_id] = values[sp];
		}
	}

	free(values);

    
    // If bm->MultiPlanEffort then these params shouldn't be loaded or present.
    if (!bm->MultiPlanEffort) {
        readEconIndicatorParamXML(bm, fileName, groupingNode, betarev_id);
        readEconIndicatorParamXML(bm, fileName, groupingNode, minprob_id);
    }
    
	/* Read in the tow time values */
	subGroupingNode = Util_XML_Get_Node(ATLANTIS_ATTRIBUTE, groupingNode, "TowTime");
	if (subGroupingNode == NULL)
		quit("Read_Dan_Holland_XML: TowTime attribute group not found in input file %s.\n", fileName);

	for (fisheryIndex = 0; fisheryIndex < bm->K_num_fisheries; fisheryIndex++) {

		if(Util_XML_Read_Array_Double(ATLANTIS_FISHERY_ATTRIBUTE, fileName, "DanHolland/TowTime", subGroupingNode, no_checking, FisheryArray[fisheryIndex].fisheryCode, &values, bm->nbox) == FALSE){
			quit("Error: Unable to find parameter 'DanHolland/TowTime/%s' in input file %s\n",  FisheryArray[fisheryIndex].fisheryCode, fileName);
		}

		for (i = 0; i < bm->nbox; i++) {
			for (ns = 0; ns < bm->FISHERYprms[fisheryIndex][nsubfleets_id]; ns++) {
				prop_tow_time[fisheryIndex][ns][i] = values[i];
			}
		}
		free(values);
	}

	/* Read in the ECONexprice values */
	subGroupingNode = Util_XML_Get_Node(ATLANTIS_ATTRIBUTE, groupingNode, "ECONexprice");
	if (subGroupingNode == NULL)
		quit("Read_Dan_Holland_XML: ECONexprice attribute group not found in input file %s.\n", fileName);

	for (sp = 0; sp < bm->K_num_tot_sp; sp++) {
		if (FunctGroupArray[sp].isFished == TRUE) {

			if(Util_XML_Read_Array_Double(ATLANTIS_GROUP_ATTRIBUTE, fileName, "DanHolland/ECONexprice", subGroupingNode, no_checking, FunctGroupArray[sp].groupCode, &values, bm->nbox) == FALSE){
				quit("Error: Unable to find parameter 'DanHolland/ECONexprice/%s' in input file %s\n",  FunctGroupArray[sp].groupCode, fileName);
			}

			for (i = 0; i < bm->nbox; i++) {
				for (fisheryIndex = 0; fisheryIndex < bm->K_num_fisheries; fisheryIndex++) {
					bm->ECONexprice[fisheryIndex][0][sp][i][hist_id] = values[i];
				}
			}
			free(values);
		}
	}

}
/**
 *	This routine reads all parameters of the economic models
 *
 *  Input:  char *filename: The name of the management xml input file.
 *
 */
int Read_Economic_Parameters(MSEBoxModel *bm, char *filename) {
	xmlDocPtr inputDoc;
	inputDoc = xmlReadFileDestFolder(bm->destFolder, filename, NULL, 0);

	printf("Economic parameter file loading\n");
	printf("0                                 100\n");
	printf("***");

	Read_Economic_Flags(bm, filename, inputDoc->children);
	Read_ByCatch_Incentives_XML(bm, filename, inputDoc->children);
	Read_Market_Characteristics_XML(bm, filename, inputDoc->children);
	Read_Vessel_Characteristics_XML(bm, filename, inputDoc->children);
	Read_Subfleet_Characteristics_XML(bm, filename, inputDoc->children);
	Read_Quota_XML(bm, filename, inputDoc->children);
	Read_Econ_Indicators_XML(bm, filename, inputDoc->children);
	Read_Own_Quota_XML(bm, filename, inputDoc->children);
	Read_Black_Book_XML(bm, filename, inputDoc->children);
	Read_Management_Costs_XML(bm, filename, inputDoc->children);
	if (!bm->MultiPlanEffort) {
		Read_Dan_Holland_XML(bm, filename, inputDoc->children);
	}

	xmlFreeDoc(inputDoc);
	/* Shutdown libxml */
	xmlCleanupParser();
	return TRUE;
}
