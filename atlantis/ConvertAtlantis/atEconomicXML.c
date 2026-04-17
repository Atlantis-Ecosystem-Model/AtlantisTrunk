/*
 * atEconomicXML.c
 *
 *  Created on: 03/03/2010
 *      Author: bec
 */

/*  Open library routines */
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdarg.h>
#include <math.h>
#include <ctype.h>
#include <assert.h>
#include <netcdf.h>
#include <sjwlib.h>
#include <atEconomic.h>
#include "convertXML.h"


//static char *monthStrings[] =
//	{ "Jan", "Feb", "Mar", "Apr", "May", "Jun", "Jul", "Aug", "Sep", "Oct", "Nov", "Dec" };

/**
 * \brief Will cut the string where there is an '_'.
 */
void FisheryFirstXMLFunction(MSEBoxModel *bm, char *fileName, xmlNodePtr parent, char *str, char *valueStr) {

	//char *fisheryStr;
	int fisheryIndex;

	*strchr(str, '_') = '\0';

	fisheryIndex = Util_Get_Fishery_Index(bm, str);
	Util_XML_Set_Node_Value(ATLANTIS_FISHERY_ATTRIBUTE, parent, FisheryArray[fisheryIndex].fisheryCode, valueStr);
}

void createMarketXMLNodes(MSEBoxModel *bm, xmlNodePtr parent) {

	xmlNodePtr groupingNode;
	int index;
	char str[200];

	groupingNode = Util_XML_Create_Node(ATLANTIS_ATTRIBUTE, parent, "Market_Intercept", "", "", "");
	for (index = 0; index < bm->K_num_markets; index++) {
		snprintf(str, sizeof(str), "Market_%d", index);
		Util_XML_Create_Node(ATLANTIS_ATTRIBUTE, groupingNode, str, "", "", "");
	}
	groupingNode = Util_XML_Create_Node(ATLANTIS_ATTRIBUTE, parent, "Market_Trend", "", "", "");
	for (index = 0; index < bm->K_num_markets; index++) {
		snprintf(str, sizeof(str), "Market_%d", index);
		Util_XML_Create_Node(ATLANTIS_ATTRIBUTE, groupingNode, str, "", "", "");
	}
	groupingNode = Util_XML_Create_Node(ATLANTIS_ATTRIBUTE, parent, "Market_Autocoefft", "Overall market specific coefficient", "", "");
	for (index = 0; index < bm->K_num_markets; index++) {
		snprintf(str, sizeof(str), "Market_%d", index);
		Util_XML_Create_Node(ATLANTIS_ATTRIBUTE, groupingNode, str, "", "", "");
	}
}

void MarketXML(MSEBoxModel *bm, char *fileName, xmlNodePtr parent, char *str, char *valueStr) {

	int index;
	char paramStr[200];
	xmlNodePtr groupingNode = NULL;
	char *lastLetter;

	lastLetter = str + strlen(str) - 1;
	index = atoi(lastLetter) - 1;
	*lastLetter = '\0';

	/* find the parent node */
	if (strcmp(str, "MARKET_intercept") == 0) {
		groupingNode = Util_XML_Get_Or_Create_Node(ATLANTIS_ATTRIBUTE, parent, "Market_Intercept");
	} else if (strcmp(str, "MARKET_trend") == 0) {
		groupingNode = Util_XML_Get_Or_Create_Node(ATLANTIS_ATTRIBUTE, parent, "Market_Trend");
	} else if (strcmp(str, "MARKET_autocoefft") == 0) {
		groupingNode = Util_XML_Get_Or_Create_Node(ATLANTIS_ATTRIBUTE, parent, "Market_Autocoefft");
	} else {
		quit("MarketXML: Unrecognised option %s\n", str);
	}

	snprintf(paramStr, sizeof(paramStr), "Market_%d", index);
	Util_XML_Set_Node_Value(ATLANTIS_ATTRIBUTE, groupingNode, paramStr, valueStr);

}

void createMonthlyMarketXMLNodes(MSEBoxModel *bm, xmlNodePtr parent) {

	int index;
	char str[200];
	xmlNodePtr groupingNode;

	groupingNode = Util_XML_Create_Node(ATLANTIS_ATTRIBUTE, parent, "MarketCoefft1",
			"Monthly market specific coefficients -  per fished species for the first market", "", "");
	for (index = 0; index < 12; index++) {
		snprintf(str, sizeof(str), "%s", monthStrings[index]);
		Util_XML_Create_Node(ATLANTIS_ATTRIBUTE, groupingNode, str, "", "", "");
	}
	groupingNode = Util_XML_Create_Node(ATLANTIS_ATTRIBUTE, parent, "MarketCoefft2",
			"Monthly market specific coefficients -  per fished species for the second market", "", "");
	for (index = 0; index < 12; index++) {
		snprintf(str, sizeof(str), "%s", monthStrings[index]);
		Util_XML_Create_Node(ATLANTIS_ATTRIBUTE, groupingNode, str, "", "", "");
	}
}

void MonthlyMarketXML(MSEBoxModel *bm, char *fileName, xmlNodePtr parent, char *str, char *valueStr) {

	int index;
	char paramStr[200];
	xmlNodePtr groupingNode;
	char *lastLetter;

	/* Get the index */
	lastLetter = str + strlen(str) - 1;
	index = atoi(lastLetter);

	/* strip off the MarketCoefft1 bit from the string */
	*strstr(str, "MarketCoefft") = '\0';

	/* find the parent node */
	snprintf(paramStr, sizeof(paramStr), "MarketCoefft%d", index);
	groupingNode = Util_XML_Get_Or_Create_Node(ATLANTIS_ATTRIBUTE, parent, paramStr);

	Util_XML_Set_Node_Value(ATLANTIS_ATTRIBUTE, groupingNode, str, valueStr);
}

/**
 * \brief Create a node for each fishery.
 */
void createEconInputMLNodes(MSEBoxModel *bm, xmlNodePtr parent) {
	int index;

	/* Create a node for each functional group*/
	for (index = 0; index < K_econ_indicators; index++)
		Util_XML_Create_Node(ATLANTIS_ATTRIBUTE, parent, econInputNames[index], "", "", "");
}

/**
 * \brief Create a node for each fishery.
 */
void createOwnQuotaXMLNodes(MSEBoxModel *bm, xmlNodePtr parent) {
	int sp, i;
	char str[5000];
	xmlNodePtr groupingNode;

	/* Create a node for each functional group*/
	for (sp = 0; sp < bm->K_num_tot_sp; sp++) {
		if (FunctGroupArray[sp].isFished == TRUE) {
			groupingNode = Util_XML_Get_Or_Create_Node(ATLANTIS_ATTRIBUTE, parent, FunctGroupArray[sp].groupCode);
			for (i = 0; i < bm->K_max_num_subfleet; i++) {
				snprintf(str, sizeof(str), "sub%d", i + 1);
				Util_XML_Create_Node(ATLANTIS_ATTRIBUTE, groupingNode, str, "", "", "");

			}
		}
	}
}

void OwnQuotaXML(MSEBoxModel *bm, char *fileName, xmlNodePtr parent, char *str, char *valueStr) {

	char tempStr[2000];
	int speciesIndex, index;
	xmlNodePtr groupingNode;
	char *lastLetter;
	char *speciesStr;

	lastLetter = str + strlen(str) - 1;
	index = atoi(lastLetter) - 1;
	*lastLetter = '\0';
	speciesStr = str + strlen("OwnQuota");
	*strstr(speciesStr, "_") = '\0';

	speciesIndex = Util_Get_FG_Index_From_Token(bm, speciesStr, fileName, str, TRUE);

	/* find the FG node */
	groupingNode = Util_XML_Get_Or_Create_Node(ATLANTIS_ATTRIBUTE, parent, FunctGroupArray[speciesIndex].groupCode);

	snprintf(tempStr, sizeof(tempStr), "sub%d", index + 1);
	Util_XML_Set_Node_Value(ATLANTIS_ATTRIBUTE, groupingNode, tempStr, valueStr);
}

/**
 * \brief Will cut the string where there is an '_'.
 */
void EconInputXMLFunction(MSEBoxModel *bm, char *fielName, xmlNodePtr parent, char *str, char *valueStr) {

	//char *fisheryStr;
	//int fisheryIndex;

	*strchr(str, '_') = '\0';

	//fisheryIndex = Util_Get_Fishery_Index(bm, str);
	Util_XML_Set_Node_Value(ATLANTIS_ATTRIBUTE, parent, str, valueStr);
}

void economicFlagParamsXML(MSEBoxModel *bm, FILE *fp, char *fileName, xmlDocPtr doc, xmlNodePtr rootnode) {

	xmlNodePtr groupingNode, childGroupingNode;
	groupingNode = Util_XML_Create_Node(ATLANTIS_ATTRIBUTE_SUB_GROUP, rootnode, "Economic_Flags", "", "", "");

	/* General economic fleet dynamics flags */
	Util_XML_Parse_Create_Node(fp, fileName, groupingNode, "MultiPlanEffort", "Whether use base (Fulton) economic model (1) or Dan Holland model (0)", "", XML_TYPE_BOOLEAN,"1");
    Util_XML_Parse_Create_Node(fp, fileName, groupingNode, "MonthlyProfitUpdate", "Whether monthly profit updates are used in the economic model (1) or only annually (0)", "", XML_TYPE_BOOLEAN,"1");
	Util_XML_Parse_Create_Node(fp, fileName, groupingNode, "econweekly", "Whether use weekly step as finest step in economic model (1) or based on trip length (0)", "", XML_TYPE_BOOLEAN,"1");
	Util_XML_Parse_Create_Node(fp, fileName, groupingNode, "quota_trading", "Whether allow quota trading (1) or not (0)", "", XML_TYPE_BOOLEAN,"1");

	Util_XML_Parse_Create_Node(fp, fileName, groupingNode, "fish_withoutQ", "Whether fishing allowed without quota in hand (1) or not (0)", "", XML_TYPE_BOOLEAN,"0");
	Util_XML_Parse_Create_Node(fp, fileName, groupingNode, "stochastic_effort", "Whether stochastic (1) or deterministic (0) effort allocation routines used with economic drivers", "",
			XML_TYPE_BOOLEAN, "0");
	Util_XML_Parse_Create_Node(fp, fileName, groupingNode, "flagallmonths",
			"Whether readjust effort for all months (1) or only current month (0) when responding to size of actual catches", "", XML_TYPE_BOOLEAN,"0");
	Util_XML_Parse_Create_Node(fp, fileName, groupingNode, "flagswitch", "Whether allowed to switch fleets at will (1) or not (0)", "", XML_TYPE_BOOLEAN,"0");
	Util_XML_Parse_Create_Node(fp, fileName, groupingNode, "flagslowsell", "Whether a boat must leave everytime decomission trigger tripped (0) or not (1) if a marginal case", "",
			XML_TYPE_BOOLEAN, "1");
	Util_XML_Parse_Create_Node(fp, fileName, groupingNode, "flagsupp_allowed", "Allows flexible gear use (1) or not (0)", "", XML_TYPE_BOOLEAN,"0");
	Util_XML_Parse_Create_Node(fp, fileName, groupingNode, "flagnewboat", "Allows new boats to enter subfleets", "", XML_TYPE_BOOLEAN,"1");
	Util_XML_Parse_Create_Node(fp, fileName, groupingNode, "sp_by_sp", "Which quota trading model is used: 0 = packages, 1=species-by-species", "", XML_TYPE_BOOLEAN,"0");
	Util_XML_Parse_Create_Node(fp, fileName, groupingNode, "flagLease", "Which lease price model to use: 0=Rich Little, 1=Newell", "", XML_TYPE_BOOLEAN,"0");
	Util_XML_Parse_Create_Node(fp, fileName, groupingNode, "yr_horizon", "Time horizon of which long term depreciation etc considered over", "", XML_TYPE_INTEGER, "10");
	Util_XML_Parse_Create_Node(fp, fileName, groupingNode, "interest_rate", "Interest rate", "", XML_TYPE_FLOAT, "5.5");

	Util_XML_Parse_Create_Node(fp, fileName, groupingNode, "hist_only", "Whether use historic effort distribution (1) or dynamic based on economics (0)", "", XML_TYPE_BOOLEAN,"1");
	Util_XML_Parse_Create_Node(fp, fileName, groupingNode, "hist_only_timeout",
			"Day of run with hist_only is set back to 0 (so more dynamic effort allocation used) - currently assumed to be around end/start of calendar year",
			"", XML_TYPE_FLOAT, "1824");
	Util_XML_Parse_Create_Node(fp, fileName, groupingNode, "rescale_post_burnin", "Whether to rescale effort, catch, quota after the end of the burn-in period (1) or not (0) - so can do rapid depletion experiment", "", XML_TYPE_BOOLEAN,"0");
    Util_XML_Get_Value_Integer(fileName, ATLANTIS_ATTRIBUTE, bm->ecotest, 1, groupingNode, no_checking, "rescale_post_burnin", &bm->rescale_post_burnin);
    
	Util_XML_Parse_Create_Node(
			fp, fileName,
			groupingNode,
			"immed_cost_only",
			"Use only variable (immediate) costs in effort allocation calculations (1) or use all costs (including fixed and capital costs) when determining effort levels (0)",
			"", XML_TYPE_BOOLEAN,"0");
	Util_XML_Parse_Create_Node(fp, fileName, groupingNode, "EconLimDemand", "Whether to limit trading based on potential effort levels vs days left in month (1) or not (0)", "",
			XML_TYPE_BOOLEAN, "0");
	Util_XML_Parse_Create_Node(fp, fileName, groupingNode, "month_scalar", "Scalar to indicate number of days per month to apply when calculating max_month_effort", "", XML_TYPE_FLOAT, "40.0");
	Util_XML_Parse_Create_Node(fp, fileName, groupingNode, "minValue", "Minimum price per kg of fish", "", XML_TYPE_FLOAT, "0.01");

	Util_XML_Parse_Create_Node(fp, fileName, groupingNode, "DVstart",
			"Day of run when management using deemed values starts, Use a very large value if deemed values shouldn't be used.", "", XML_TYPE_INTEGER, "800000000");
	Util_XML_Parse_Create_Node(fp, fileName, groupingNode, "TaxStart", "Day of run when management using taxes starts", "", XML_TYPE_INTEGER, "366666666660");
	Util_XML_Parse_Create_Node(fp, fileName, groupingNode, "UseVPUE", "Flag to indicate whether using CPUE (0) or VPUE (1) to dictate effort distributions", "", XML_TYPE_BOOLEAN,"0");

	Util_XML_Parse_Create_Node(fp, fileName, groupingNode, "prop_leave", "Proportion of the fleet that can leave in any one month", "", XML_TYPE_FLOAT, "0.02");
	Util_XML_Parse_Create_Node(fp, fileName, groupingNode, "prop_switch", "Proportion of the fleet that can switch gears in any one month", "", XML_TYPE_FLOAT, "0.1");
	Util_XML_Parse_Create_Node(fp, fileName, groupingNode, "prop_supp", "Proportion of the fleet that can flexibility supplement gears in any one month", "", XML_TYPE_FLOAT, "0.0");
	Util_XML_Parse_Create_Node(fp, fileName, groupingNode, "prop_boatbound", "Lower bound on proportion of large fleets that remain through length of simulated period", "", XML_TYPE_FLOAT, "0.3");

	Util_XML_Parse_Create_Node(fp, fileName, groupingNode, "UseConstPrice", "Whether use base (Fulton) economic model (1) or Dan Holland model (0)", "", XML_TYPE_BOOLEAN,"0");
	Util_XML_Parse_Create_Node(fp, fileName, groupingNode, "UseMinValue", "Set price coefficients to zero in Newell model for species that have no landed price", "", XML_TYPE_FLOAT, "0");
	Util_XML_Parse_Create_Node(fp, fileName, groupingNode, "OrigEconCalc", "Whether to use the original economic driver calculation (1) or use one that is total price - total cost ",
			"", XML_TYPE_BOOLEAN,"0");
	Util_XML_Parse_Create_Node(fp, fileName, groupingNode, "UseEconQuotaDefine", "", "", XML_TYPE_BOOLEAN,"1");

	Util_XML_Parse_Create_Node(fp, fileName, groupingNode, "UseEconQuotaDefine", "Use the TAC as sety in harvest.prm (0) as in QuotaAlloc in this file (1)", "", XML_TYPE_BOOLEAN,"0");
	Util_XML_Parse_Create_Node(fp, fileName, groupingNode, "flaglbs", "catch and cpue is in lbs (1=yes, 0=no)", "", XML_TYPE_BOOLEAN,"0");
    
	
    if(bm->rescale_post_burnin) {
        printf("Attempting to write xml for rescaling\n");
        
        childGroupingNode = Util_XML_Create_Node(ATLANTIS_ATTRIBUTE_SUB_GROUP, rootnode, "Rescaling", "Rescaling for hsitrocial depletion experiments", "", "");
        
        Util_XML_Create_Node_Next_Line(fp, fileName, childGroupingNode, "ReScale_Eff", "Rescaling of effort for depletion experiement", "", XML_TYPE_FLOAT);
        Util_XML_Create_Node_Next_Line(fp, fileName, childGroupingNode, "ReScale_Cat", "Rescaling of catch for depletion experiement", "", XML_TYPE_FLOAT);
        
        printf("Done write\n");
    }


}

void ByCatch_Incentives_XML(MSEBoxModel *bm, FILE *fp, char *fileName, xmlDocPtr doc, xmlNodePtr rootnode) {

	xmlNodePtr groupingNode;
	groupingNode = Util_XML_Create_Node(ATLANTIS_ATTRIBUTE_SUB_GROUP, rootnode, "Bycatch_Incentives", "Bycatch incentives flags", "", "");

	Util_XML_Parse_Create_Node(fp, fileName, groupingNode, "UsingEffortIncentive",
			"Whether using effort-based incentives (0=no, 1=yes) - i.e. using ITE and effort penalties if fish in certain places etc", "", XML_TYPE_BOOLEAN,"0");
	Util_XML_Parse_Create_Node(fp, fileName, groupingNode, "BycatchCountDiscards", " Whether bycatch counted in discards (0=no, 1-yes)", "", XML_TYPE_BOOLEAN,"0");
	Util_XML_Parse_Create_Node(fp, fileName, groupingNode, "TemporalBycatchAvoid",
			"Whether taking temporal exposure to bycatch species (and discards) into account when scheduling effort (0=no, 1=yes)", "", XML_TYPE_BOOLEAN,"0");
	Util_XML_Parse_Create_Node(fp, fileName, groupingNode, "SpatialBycatchAvoid",
			"Whether using non-penalty/non-cap incentives to steer fleets away from areas where they discard the most (0=no, 1=yes)", "", XML_TYPE_BOOLEAN,"0");
}

void Market_Characteristics_XML(MSEBoxModel *bm, FILE *fp, char *fileName, xmlDocPtr doc, xmlNodePtr rootnode) {

	xmlNodePtr groupingNode;
	groupingNode
			= Util_XML_Create_Node(
					ATLANTIS_ATTRIBUTE_SUB_GROUP,
					rootnode,
					"Market_Characteristics",
					"Market 0 = Sydney, Market 1 = Melbourne. The scalar needed vs base sale prices entered below to give individual market value intercepts of the autoregression sale price model for each fished species",
					"", "");

	Parse_File(
			bm,
			fp,fileName,
			groupingNode,
			"Market",
			"MARKET_",
			"The scalar needed vs base sale prices entered below to give individual market value intercepts of the autoregression sale price model for each fished species",
			"", XML_TYPE_FLOATARRAY, bm->K_num_tot_sp, TRUE, createMarketXMLNodes, MarketXML);

	Parse_File(bm, fp, fileName, groupingNode, "MonthlyMarket", "MarketCoefft", "Monthly market specific coefficients - per fished species for the first market", "",
			XML_TYPE_FLOATARRAY, bm->K_num_tot_sp, TRUE, createMonthlyMarketXMLNodes, MonthlyMarketXML);

	Create_Harvest_Fishery_Group_ParamXML(bm, fp, fileName, groupingNode, saleprice_id, "Fish sale prices ($/kg) - one entry per fishery", "",
			XML_TYPE_FLOATARRAY);

}

void Vessel_Characteristics_XML(MSEBoxModel *bm, FILE *fp, char *fileName, xmlDocPtr doc, xmlNodePtr rootnode) {

	xmlNodePtr groupingNode;
	groupingNode
			= Util_XML_Create_Node(ATLANTIS_ATTRIBUTE_SUB_GROUP, rootnode, "Vessel_Characteristics", "Information about the vessels in the model", "", "");

	Create_Fishery_ParamXML(bm, fp, fileName, groupingNode, flagDV_id,
			"Form of deemed values used (0=none, 1=simple proportion of price, 2=complex based on costs etc)", "", XML_TYPE_INTEGER, "0");

	Create_Harvest_Fishery_Group_ParamXML(bm, fp, fileName, groupingNode, deemedvalue_id,
			"Deemed value rates used per fleet - as disincentive on bycatch - one entry per fishery (needn't be identical, but is in AMS case).", "",
			XML_TYPE_FLOATARRAY);

	Create_Fishery_ParamXML(bm, fp, fileName, groupingNode, nlicence_id,  "Maximum number of licences the fishery can hold", "", XML_TYPE_INTEGER, "0");

	Create_Fishery_ParamXML(bm, fp, fileName, groupingNode, flagMultiSpEffort_id,
			"Degree of multi-species targetting by fishery (0=main target spp only, 1=secondary spp also considered)", "", XML_TYPE_BOOLEAN,"0");
}

/**
 * \brief Subfleet characteristic values.
 */
void Subfleet_Characteristics_XML(MSEBoxModel *bm, FILE *fp, char *fileName, xmlDocPtr doc, xmlNodePtr rootnode) {

	xmlNodePtr groupingNode, subGroupingNode, sgNode;
	groupingNode = Util_XML_Create_Node(ATLANTIS_ATTRIBUTE_SUB_GROUP, rootnode, "Subfleet_Characteristics", "Subfleet characteristics", "", "");

	Parse_File(bm, fp, fileName, groupingNode, "NumBoats", "_nboat$", "Number of boats per subfleet", "", XML_TYPE_INTEGERARRAY, bm->K_max_num_subfleet, TRUE, Create_Fishery_XMLNodes,
			FisheryFirstXMLFunction);
	Parse_File(bm, fp, fileName, groupingNode, "BoatSize", "_boat_size", "Boat size per subfleet", "", XML_TYPE_FLOATARRAY, bm->K_max_num_subfleet, TRUE, Create_Fishery_XMLNodes,
			FisheryFirstXMLFunction);

	Parse_File(bm, fp, fileName, groupingNode, "HoldCapacity", "_hold_capacity", "Hold capacity in tonnes - used in Dan Holland-like bioeconomic model variants", "",
			XML_TYPE_FLOATARRAY, bm->K_max_num_subfleet, TRUE, Create_Fishery_XMLNodes, FisheryFirstXMLFunction);

	Parse_File(bm, fp, fileName, groupingNode, "CrewSize", "_crewsize", "Crew Size per subfleet", "", XML_TYPE_FLOATARRAY, bm->K_max_num_subfleet, TRUE, Create_Fishery_XMLNodes,
			FisheryFirstXMLFunction);

	Parse_File(bm, fp, fileName, groupingNode, "HomePort", "_home_port", "The home port for each subfleet", "", XML_TYPE_FLOATARRAY, bm->K_max_num_subfleet, TRUE,
			Create_Fishery_XMLNodes, FisheryFirstXMLFunction);

	subGroupingNode = Util_XML_Create_Node(ATLANTIS_ATTRIBUTE_SUB_GROUP, groupingNode, "SubfleetEffort", "Subfleet effort characteristics", "", "");

	Parse_File(bm, fp, fileName, subGroupingNode, "FCWeightScale", "_FCwgtscale", "Scalar for influence of distance on effort allocation decisions per subfleet", "",
			XML_TYPE_INTEGERARRAY, bm->K_max_num_subfleet, TRUE, Create_Fishery_XMLNodes, FisheryFirstXMLFunction);

	Parse_File(bm, fp, fileName, subGroupingNode, "TripLength", "_triplength", "Trip length per subfleet", "d", XML_TYPE_INTEGERARRAY, bm->K_max_num_subfleet, TRUE,
			Create_Fishery_XMLNodes, FisheryFirstXMLFunction);

	Parse_File(bm, fp, fileName, subGroupingNode, "FishableLength", "_FishableLength",
               "Length of average time actually fished per day (in hours) (assuming typical shot length and typical number of shots per day)", "d", XML_TYPE_INTEGERARRAY,
               bm->K_max_num_subfleet, TRUE, Create_Fishery_XMLNodes, FisheryFirstXMLFunction);
    
	Parse_File(bm, fp, fileName, subGroupingNode, "ShotLength", "_ShotLength",
			"Length of average shot (in hours)", "hr", XML_TYPE_INTEGERARRAY,
			bm->K_max_num_subfleet, TRUE, Create_Fishery_XMLNodes, FisheryFirstXMLFunction);

	Parse_File(bm, fp, fileName, subGroupingNode, "MaxShotVol", "_MaxShotVol",
               "Maximum volume of the shot (in tonnes)", "hr", XML_TYPE_INTEGERARRAY,
               bm->K_max_num_subfleet, TRUE, Create_Fishery_XMLNodes, FisheryFirstXMLFunction);
    
	Parse_File(bm, fp, fileName, subGroupingNode, "MinDownTime", "_minDownTime",
			"Down time per month - minimum proportion of month where need to be in port for maintenance", "", XML_TYPE_FLOATARRAY, bm->K_max_num_subfleet, TRUE,
			Create_Fishery_XMLNodes, FisheryFirstXMLFunction);

	Parse_File(bm, fp, fileName, subGroupingNode, "ChoiceBuffer", "_choicebuffer",
			"Degree (per subfleet) that will reduce effort in any one month in response to catch exceeding expectations.", "", XML_TYPE_FLOATARRAY, bm->K_max_num_subfleet,
			TRUE, Create_Fishery_XMLNodes, FisheryFirstXMLFunction);

	Parse_File(bm, fp, fileName, subGroupingNode, "EffortDiscount", "_EffortDiscount", "Effort discounting per subfleet", "", XML_TYPE_FLOATARRAY, bm->K_max_num_subfleet, TRUE,
			Create_Fishery_XMLNodes, FisheryFirstXMLFunction);

	Parse_File(bm, fp, fileName, subGroupingNode, "BycatchDiscount", "_BycatchDiscount", "Discounting on update of bycatch blackbook per subfleet", "", XML_TYPE_FLOATARRAY, bm->K_max_num_subfleet, TRUE,
			Create_Fishery_XMLNodes, FisheryFirstXMLFunction);

	subGroupingNode = Util_XML_Create_Node(ATLANTIS_ATTRIBUTE_SUB_GROUP, groupingNode, "SubfleetCosts", "Subfleet costs", "", "");

	Util_XML_Parse_Create_Node(fp, fileName, subGroupingNode, "shore_costs", "Annual cost of running shore-based household", "", XML_TYPE_FLOAT, "100000");

	Parse_File(bm, fp, fileName, subGroupingNode, "VariableCosts", "_varcost", "Variable costs per subfleet in $ per day", "", XML_TYPE_FLOATARRAY, bm->K_max_num_subfleet, TRUE,
			Create_Fishery_XMLNodes, FisheryFirstXMLFunction);

	/* Fuel costs */
	sgNode = Util_XML_Create_Node(ATLANTIS_ATTRIBUTE_SUB_GROUP, subGroupingNode, "FuelCosts", "Fuel costs", "", "");

	Util_XML_Parse_Create_Node(fp, fileName, sgNode, "fuel_intercept", "", "", XML_TYPE_FLOAT, "73.32836594");
	Util_XML_Parse_Create_Node(fp, fileName, sgNode, "fuel_trend_coefft", "", "", XML_TYPE_FLOAT, "0.60297085");
	Util_XML_Parse_Create_Node(fp, fileName, sgNode, "fuel_auto_coefft", "", "", XML_TYPE_FLOAT, "1.10762799");

	Util_XML_Create_Node_Next_Line(fp, fileName, sgNode, "FuelMonthCoefft", "", "", XML_TYPE_FLOATARRAY);

	Parse_File(bm, fp, fileName, sgNode, "FuelPerDay", "_fuelcost", "L per day needed", "", XML_TYPE_FLOATARRAY, bm->K_max_num_subfleet, TRUE, Create_Fishery_XMLNodes,
			FisheryFirstXMLFunction);

	Parse_File(bm, fp, fileName, subGroupingNode, "GearCosts", "_gearcost", "Cost of gear maintenance per subfleet in $ per day", "", XML_TYPE_FLOATARRAY, bm->K_max_num_subfleet,
			TRUE, Create_Fishery_XMLNodes, FisheryFirstXMLFunction);

	Parse_File(bm, fp, fileName, subGroupingNode, "UnloadCosts", "_unloadcost", "Cost of unloading per subfleet in $ per tonne", "", XML_TYPE_FLOATARRAY, bm->K_max_num_subfleet, TRUE,
			Create_Fishery_XMLNodes, FisheryFirstXMLFunction);

	Parse_File(bm, fp, fileName, subGroupingNode, "FixedCosts", "_fixedcost", "Fixed costs per subfleet in $ per day", "", XML_TYPE_FLOATARRAY, bm->K_max_num_subfleet, TRUE,
			Create_Fishery_XMLNodes, FisheryFirstXMLFunction);

	Parse_File(bm, fp, fileName, subGroupingNode, "CapitalCosts", "_capitalcost", "Capital costs per subfleet in $ per day", "", XML_TYPE_FLOATARRAY, bm->K_max_num_subfleet, TRUE,
			Create_Fishery_XMLNodes, FisheryFirstXMLFunction);

	sgNode = Util_XML_Create_Node(ATLANTIS_ATTRIBUTE_SUB_GROUP, subGroupingNode, "GeneralDebt", "General debt parameters", "", "");

	Util_XML_Parse_Create_Node(fp, fileName, sgNode, "effort_reduction", "Proportion of effort in the month lost due to crippling debt (when debt is greater than TolDebt)", "", XML_TYPE_FLOAT,
			"0.15");
	Util_XML_Parse_Create_Node(fp, fileName, sgNode, "cripple_nboat_lost", "Proportion of boats lost from fishery once debt has been crippling for more than cripple_period", "", XML_TYPE_FLOAT,
			"0.05");
	Util_XML_Parse_Create_Node(fp, fileName, sgNode, "cripple_period", "Number of months of debt back to back before boats are forced from the fishery", "", XML_TYPE_FLOAT, "12");

	Parse_File(bm, fp, fileName, subGroupingNode, "ToleratedDebt", "_TolDebt", "Debt tolerated in $ per day", "", XML_TYPE_FLOATARRAY, bm->K_max_num_subfleet, TRUE,
			Create_Fishery_XMLNodes, FisheryFirstXMLFunction);

	Parse_File(bm, fp, fileName, subGroupingNode, "CrewShare", "_crewshare", "Proportion of the profits that are the crew share (per subfleet)", "", XML_TYPE_FLOATARRAY,
			bm->K_max_num_subfleet, TRUE, Create_Fishery_XMLNodes, FisheryFirstXMLFunction);

	Parse_File(bm, fp, fileName, subGroupingNode, "FlexWeight", "_flexweight", "Flexibility of behaviour (weighting) per subfleet", "", XML_TYPE_FLOATARRAY, bm->K_max_num_subfleet,
			TRUE, Create_Fishery_XMLNodes, FisheryFirstXMLFunction);
	Parse_File(bm, fp, fileName, subGroupingNode, "Resale", "_resale", "Resale value per subfleet", "", XML_TYPE_FLOATARRAY, bm->K_max_num_subfleet, TRUE, Create_Fishery_XMLNodes,
			FisheryFirstXMLFunction);

	Parse_File(bm, fp, fileName, subGroupingNode, "BuyOut", "_buyout", "Buy out value per subfleet", "", XML_TYPE_FLOATARRAY, bm->K_max_num_subfleet, TRUE, Create_Fishery_XMLNodes,
			FisheryFirstXMLFunction);

	Parse_File(bm, fp, fileName, subGroupingNode, "SuppCost", "_suppcost", "Cost of supplementing fleets per subfleet", "", XML_TYPE_FLOATARRAY, bm->K_max_num_subfleet, TRUE,
			Create_Fishery_XMLNodes, FisheryFirstXMLFunction);

	Parse_File(bm, fp, fileName, subGroupingNode, "SwitchCost", "_switchcost", "Cost of switching fleets per subfleet", "", XML_TYPE_FLOATARRAY, bm->K_max_num_subfleet, TRUE,
			Create_Fishery_XMLNodes, FisheryFirstXMLFunction);

	Parse_File(bm, fp, fileName, subGroupingNode, "NewBoatCost", "_newcost", "Cost of purchasing new boat per subfleet", "", XML_TYPE_FLOATARRAY, bm->K_max_num_subfleet, TRUE,
			Create_Fishery_XMLNodes, FisheryFirstXMLFunction);
}

void Quota_XML(MSEBoxModel *bm, FILE *fp, char *fileName, xmlDocPtr doc, xmlNodePtr rootnode) {

	xmlNodePtr groupingNode, subGroupingNode;
	groupingNode = Util_XML_Create_Node(ATLANTIS_ATTRIBUTE_SUB_GROUP, rootnode, "Quota_Reconciliation", "Quota reconciliation", "", "");

	Util_XML_Parse_Create_Node(fp, fileName, groupingNode, "recon_buffer", " Minimum resolution of quota reconcilation for ITQ", "", XML_TYPE_FLOAT, "10.0");

	subGroupingNode = Util_XML_Create_Node(ATLANTIS_ATTRIBUTE_SUB_GROUP, groupingNode, "LeaseValues",
			"Newell et al lease price model coefficients - sets quota value", "", "");

	Util_XML_Parse_Create_Node(fp, fileName, subGroupingNode, "price_Coefft", "Coefficient of product prices in quota sale value regression model", "", XML_TYPE_FLOAT, "0.218");
	Util_XML_Parse_Create_Node(fp, fileName, subGroupingNode, "price_CoefftSq", "Coefficient of squared price in quota sale value regression model", "", XML_TYPE_FLOAT, "0.047");
	Util_XML_Parse_Create_Node(fp, fileName, subGroupingNode, "cost_Coefft", "Coefficient of total costs in quota sale value regression model", "", XML_TYPE_FLOAT, "-0.741");
	Util_XML_Parse_Create_Node(fp, fileName, subGroupingNode, "prev_fill_Coefft", "Coefficient of previously filled quotas in quota sale value regression model", "", XML_TYPE_FLOAT, "0.382");
	Util_XML_Parse_Create_Node(fp, fileName, subGroupingNode, "prev_fill_CoefftSq", "Coefficient of squared previously filled quotas in quota sale value regression model", "", XML_TYPE_FLOAT,
			"-0.228");
	Util_XML_Parse_Create_Node(fp, fileName, subGroupingNode, "part_fill_Coefft", "Coefficient of currently filled quotas in quota sale value regression model", "", XML_TYPE_FLOAT, "0.076");
	Util_XML_Parse_Create_Node(fp, fileName, subGroupingNode, "part_fill_CoefftSq", "Coefficient of squared currently filled quotas in quota sale value regression model", "", XML_TYPE_FLOAT,
			"-0.03");
	Util_XML_Parse_Create_Node(fp, fileName, subGroupingNode, "bind_Coefft", "Coefficient of binding in quota sale value regression model", "", XML_TYPE_FLOAT, "0.162");

	Util_XML_Parse_Create_Node(fp, fileName, subGroupingNode, "tac_trend_Coefft", "Coefficient of TAC trend in quota sale value regression model", "", XML_TYPE_FLOAT, "0.042");
	Util_XML_Parse_Create_Node(fp, fileName, subGroupingNode, "const_Coefft", "Constant in quota sale value regression model", "", XML_TYPE_FLOAT, "5.643");
	Util_XML_Parse_Create_Node(fp, fileName, subGroupingNode, "GDP_Coefft", "Coefficient of GDP in quota sale value regression model", "", XML_TYPE_FLOAT, "1.701");

	Util_XML_Parse_Create_Node(fp, fileName, subGroupingNode, "minValue", "minimum price per kg of fish", "", XML_TYPE_FLOAT, "1");

	subGroupingNode = Util_XML_Create_Node(ATLANTIS_ATTRIBUTE_SUB_GROUP, groupingNode, "FleetSwitching",
			"Fleet switching and decomission parameters - from Thebaud et al 2006 and Guyader 2002", "", "");

	Util_XML_Parse_Create_Node(fp, fileName, subGroupingNode, "decom_return_coefft", "Coefficient of returns on decomissioning", "", XML_TYPE_FLOAT, "1");
	Util_XML_Parse_Create_Node(fp, fileName, subGroupingNode, "resale_coeff", "Coefficient of resale value", "", XML_TYPE_FLOAT, "1");
	Util_XML_Parse_Create_Node(fp, fileName, subGroupingNode, "decom_crew_coefft", "Coefficient of crew share in decomissioning model", "", XML_TYPE_FLOAT, "1");
	Util_XML_Parse_Create_Node(fp, fileName, subGroupingNode, "switch_return_coefft", "Coefficient of returns after fleet switching", "", XML_TYPE_FLOAT, "1");
	Util_XML_Parse_Create_Node(fp, fileName, subGroupingNode, "switch_coefft", "switch_coefft", "", XML_TYPE_FLOAT, "1");
	Util_XML_Parse_Create_Node(fp, fileName, subGroupingNode, "max_size_buffer", "Max size buffer on boat size (constrains which subfleet the boat can switch to)", "", XML_TYPE_FLOAT, "1");
	Util_XML_Parse_Create_Node(fp, fileName, subGroupingNode, "min_size_buffer", "Min size buffer on boat size (constrains which subfleet the boat can switch to)", "", XML_TYPE_FLOAT, "1");
	Util_XML_Parse_Create_Node(fp, fileName, subGroupingNode, "new_return_coefft", "Coefficient of returns after buying a new boat", "", XML_TYPE_FLOAT, "1");
	Util_XML_Parse_Create_Node(fp, fileName, subGroupingNode, "new_coefft", "Coefficient of purchasing a new boat", "", XML_TYPE_FLOAT, "1");

	subGroupingNode = Util_XML_Create_Node(ATLANTIS_ATTRIBUTE_SUB_GROUP, groupingNode, "BuyBack", "Buyback parameters ", "", "");

	Create_Fishery_ParamXML(bm, fp, fileName, subGroupingNode, buybackdate_id, "Buy back date (day after start of run)", "", XML_TYPE_INTEGER, "0");

	Parse_File(bm, fp, fileName, subGroupingNode, "PropBuyBack", "_propbuyback", "dayTrip 3dayTrip weekTrip longTrip", "", XML_TYPE_FLOATARRAY, bm->K_max_num_subfleet, TRUE,
			Create_Fishery_XMLNodes, FisheryFirstXMLFunction);

	subGroupingNode = Util_XML_Create_Node(ATLANTIS_ATTRIBUTE_SUB_GROUP, groupingNode, "QuotaPurchase", "Quota purchase coefficents", "", "");

	Util_XML_Parse_Create_Node(fp, fileName, subGroupingNode, "perm_coefft", "Coefficient on permanent trades (scale lease price by this to get permanent trade cost)", "", XML_TYPE_FLOAT, "1");
	Util_XML_Parse_Create_Node(fp, fileName, subGroupingNode, "buy_coefft", "Coefficient on sales (scale lease price by this to get sale price)", "", XML_TYPE_FLOAT, "1");

	Util_XML_Parse_Create_Node(fp, fileName, subGroupingNode, "targetscale", "Relative weighting of target species above other species in preferences", "", XML_TYPE_FLOAT, "1");

	Util_XML_Parse_Create_Node(fp, fileName, subGroupingNode, "prop_within",
			"Proportions where buyng and selling kick-on/off - Proportion of quota filled before consider obtaining more", "", XML_TYPE_FLOAT, "1");
	Util_XML_Parse_Create_Node(fp, fileName, subGroupingNode, "prop_spareend",
			"Proportions where buyng and selling kick-on/off - Proportion of quota below which willing to trade extra quota", "", XML_TYPE_FLOAT, "1");

	subGroupingNode = Util_XML_Create_Node(ATLANTIS_ATTRIBUTE_SUB_GROUP, groupingNode, "FriendShip", "Friendship parameters ", "", "");

	set_keyprm_errfn(warn);
	Util_XML_Parse_Create_Node(fp, fileName, subGroupingNode, "friendship_network_file", "csv file containing friendship network information", "", XML_TYPE_STRING, "1");
	set_keyprm_errfn(quit);


}

void Econ_Indicators_XML(MSEBoxModel *bm, FILE *fp, char *fileName, xmlDocPtr doc, xmlNodePtr rootnode) {

	xmlNodePtr groupingNode, subGroupingNode;
	groupingNode = Util_XML_Create_Node(ATLANTIS_ATTRIBUTE_SUB_GROUP, rootnode, "Basic_Economic_Indicators", "Basic economic indicators", "", "");

	Parse_File(bm,
			fp,fileName,
			groupingNode,
			"EconIndicatorType",
			"_flageconind",
			"Economic indicator calculation type (0=functional responses, 1=dynamics models, 2=expected functions). Only option 0 is in place as yet. This assumes one entry for each subfleet type, assuming the following order - dayTrip 3dayTrip weekTrip longTrip",
			"", XML_TYPE_FLOATARRAY, bm->K_max_num_subfleet, TRUE, Create_Fishery_XMLNodes, FisheryFirstXMLFunction);

	Parse_File(bm, fp, fileName, groupingNode, "EconIndicatorCombination", "_flagtypeind", "Type of indicator combination, addition = 0, multiplication = 1", "",
			XML_TYPE_FLOATARRAY, bm->K_max_num_subfleet, TRUE, Create_Fishery_XMLNodes, FisheryFirstXMLFunction);

	subGroupingNode = Util_XML_Create_Node(ATLANTIS_ATTRIBUTE_SUB_GROUP, groupingNode, "FunctionalResponseParameters",
			"Economic indication - functional response parameters", "", "");

	Parse_File(bm, fp, fileName, subGroupingNode, "FunctionalResponseType", "_eindtype$", "Type of economic indicator functional response", "", XML_TYPE_FLOATARRAY,
			K_ecolharvest_indicators, TRUE, createEconInputMLNodes, EconInputXMLFunction);

	Parse_File(bm, fp, fileName, subGroupingNode, "FunctionalResponseConstant", "_ecoefft$", "Constant in the economic indicator functional response", "", XML_TYPE_FLOATARRAY,
			K_ecolharvest_indicators, TRUE, createEconInputMLNodes, EconInputXMLFunction);

	Parse_File(bm, fp, fileName, subGroupingNode, "FunctionalResponseSlope", "_escale$", "Slope (coefficient) in the economic indicator functional response", "",
			XML_TYPE_FLOATARRAY, K_ecolharvest_indicators, TRUE, createEconInputMLNodes, EconInputXMLFunction);

	Parse_File(bm, fp, fileName, subGroupingNode, "FunctionalResponseSlopeB", "_escaleB$",
			"Slope (coefficient) in second stage (piecewise) economic indicator functional response", "", XML_TYPE_FLOATARRAY, K_ecolharvest_indicators, TRUE,
			createEconInputMLNodes, EconInputXMLFunction);

	Parse_File(bm, fp, fileName, subGroupingNode, "FunctionalResponseThreshold", "_ethresh$", "Threshold in piecewise economic indicator functional response", "",
			XML_TYPE_FLOATARRAY, K_ecolharvest_indicators, TRUE, createEconInputMLNodes, EconInputXMLFunction);
}

void OwnQuota_XML(MSEBoxModel *bm, FILE *fp, char *fileName, xmlDocPtr doc, xmlNodePtr rootnode) {

	//	xmlNodePtr groupingNode;
	//	groupingNode = Util_XML_Create_Node(ATLANTIS_ATTRIBUTE_SUB_GROUP, rootnode, "OwnQuota",
	//			"Initial Quota Allocation per subfleet for each fishery (in tonnes) - if unlimited enter 1000000000000.0", "", "");

	Parse_File(bm, fp, fileName, rootnode, "OwnQuota", "^OwnQuota",
			"Initial Quota Allocation per subfleet for each fishery (in tonnes) - if unlimited enter 1000000000000.0", "", XML_TYPE_FLOATARRAY, bm->K_num_fisheries,
			TRUE, createOwnQuotaXMLNodes, OwnQuotaXML);
}

void createTempBlackBookXMLNodes(MSEBoxModel *bm, xmlNodePtr parent) {
	int sp, i, month;
	char str[5000];
	xmlNodePtr groupingNode, monthNode;

	/* Create a node for each functional group*/
	for (sp = 0; sp < bm->K_num_tot_sp; sp++) {
		if (FunctGroupArray[sp].isFished == TRUE) {
			groupingNode = Util_XML_Get_Or_Create_Node(ATLANTIS_ATTRIBUTE, parent, FunctGroupArray[sp].groupCode);
			for (month = 0; month < 12; month++) {
				monthNode = Util_XML_Get_Or_Create_Node(ATLANTIS_ATTRIBUTE, groupingNode, monthStrings[month]);
				for (i = 0; i < bm->K_max_num_subfleet; i++) {
					snprintf(str, sizeof(str), "sub%d", i + 1);
					Util_XML_Create_Node(ATLANTIS_ATTRIBUTE, monthNode, str, "", "", "");

				}
			}
		}
	}
}
/**
 * JanCatchFPL_sub1
 */
void TemporalBlackBookXML(MSEBoxModel *bm, char *fileName, xmlNodePtr parent, char *str, char *valueStr) {

	char tempStr[2000];
	int speciesIndex, index;
	xmlNodePtr groupingNode, monthNode;
	char *lastLetter;
	char *speciesStr;

	lastLetter = str + strlen(str) - 1;
	index = atoi(lastLetter) - 1;
	*lastLetter = '\0';

	speciesStr = str + strlen("JanCatch");
	*strstr(speciesStr, "_") = '\0';

	speciesIndex = Util_Get_FG_Index_From_Token(bm, speciesStr, fileName, str, TRUE);

	*strstr(str, "Catch") = '\0';

	/* find the FG node */
	groupingNode = Util_XML_Get_Or_Create_Node(ATLANTIS_ATTRIBUTE, parent, FunctGroupArray[speciesIndex].groupCode);
	monthNode = Util_XML_Get_Or_Create_Node(ATLANTIS_ATTRIBUTE, groupingNode, str);
	snprintf(tempStr, sizeof(tempStr), "sub%d", index + 1);
	Util_XML_Set_Node_Value(ATLANTIS_ATTRIBUTE, monthNode, tempStr, valueStr);
}

void createSpatialBlackBookXMLNodes(MSEBoxModel *bm, xmlNodePtr parent) {
	int fisheryIndex, i, month;
	char str[5000];
	xmlNodePtr groupingNode, monthNode;

	/* Create a node for each functional group*/
	for (fisheryIndex = 0; fisheryIndex < bm->K_num_fisheries; fisheryIndex++) {
		groupingNode = Util_XML_Get_Or_Create_Node(ATLANTIS_ATTRIBUTE, parent, FisheryArray[fisheryIndex].fisheryCode);
		for (month = 0; month < 12; month++) {
			monthNode = Util_XML_Get_Or_Create_Node(ATLANTIS_ATTRIBUTE, groupingNode, monthStrings[month]);
			for (i = 0; i < bm->K_max_num_subfleet; i++) {
				snprintf(str, sizeof(str), "sub%d", i + 1);
				Util_XML_Create_Node(ATLANTIS_ATTRIBUTE, monthNode, str, "", "", "");

			}
		}
	}
}

/**
 * JanmidwcCEPCPUE_sub1
 */
void SpatialBlackBookXML(MSEBoxModel *bm, char *fileName, xmlNodePtr parent, char *str, char *valueStr, char *paramName) {

	char tempStr[2000];
	int fisheryIndex, index;
	xmlNodePtr groupingNode, monthNode;
	char *lastLetter;
	char *fisheryStr;

	lastLetter = str + strlen(str) - 1;
	index = atoi(lastLetter) - 1;
	*lastLetter = '\0';

	fisheryStr = str + strlen("Jan");
	*strstr(fisheryStr, paramName) = '\0';
	fisheryIndex = Util_Get_Fishery_Index(bm, fisheryStr);

	*(str + strlen("Jan")) = '\0';

	/* find the FG node */
	groupingNode = Util_XML_Get_Or_Create_Node(ATLANTIS_ATTRIBUTE, parent, FisheryArray[fisheryIndex].fisheryCode);
	monthNode = Util_XML_Get_Or_Create_Node(ATLANTIS_ATTRIBUTE, groupingNode, str);
	snprintf(tempStr, sizeof(tempStr), "sub%d", index + 1);
	Util_XML_Set_Node_Value(ATLANTIS_ATTRIBUTE, monthNode, tempStr, valueStr);
}

/**
 * JanCatchFPL_sub1
 */
void EffortSpatialBlackBookXML(MSEBoxModel *bm, char *fileName, xmlNodePtr parent, char *str, char *valueStr) {
	SpatialBlackBookXML(bm, fileName, parent, str, valueStr, "Effort");
}

/**
 * JanmidwcCEPCPUE_sub1
 */
void CPUESpatialBlackBookXML(MSEBoxModel *bm, char *fileName, xmlNodePtr parent, char *str, char *valueStr) {

	SpatialBlackBookXML(bm, fileName, parent, str, valueStr, "CPUE");
}

void BlackBook_XML(MSEBoxModel *bm, FILE *fp, char *fileName, xmlDocPtr doc, xmlNodePtr rootnode) {

	xmlNodePtr groupingNode;
	groupingNode = Util_XML_Create_Node(ATLANTIS_ATTRIBUTE_SUB_GROUP, rootnode, "BlackBook", "Black books - catch per month (in kg)", "", "");

	Parse_File(bm, fp, fileName, groupingNode, "TemporalBlackBook", "Catch", "Temporal black books - catch per month (in kg) per species for each fleet and subfleet", "",
			XML_TYPE_FLOATARRAY, bm->K_num_fisheries, TRUE, createTempBlackBookXMLNodes, TemporalBlackBookXML);
	Parse_File(bm, fp, fileName, groupingNode, "SpatialEffortBlackBook", "Effort_", "Spatial black books - effort (shots) per box per month per subfleet per fishery", "",
			XML_TYPE_FLOATARRAY, bm->nbox, TRUE, createSpatialBlackBookXMLNodes, EffortSpatialBlackBookXML);
	Parse_File(bm, fp, fileName, groupingNode, "SpatialCPUEBlackBook", "CPUE_", "Spatial CPUE per box per month per subfleet per fishery", "",
				XML_TYPE_FLOATARRAY, bm->nbox, TRUE, createSpatialBlackBookXMLNodes, CPUESpatialBlackBookXML);
}

void ManagementCosts_XML(MSEBoxModel *bm, FILE *fp, char *fileName, xmlDocPtr doc, xmlNodePtr rootnode) {

	xmlNodePtr groupingNode;
	groupingNode
			= Util_XML_Create_Node(
					ATLANTIS_ATTRIBUTE_SUB_GROUP,
					rootnode,
					"ManagementCosts",
					"Management Cost - do this the same as the other costs (vector per fleet with entry for each subfleet) and it's the proportion of revenue that can be taken as recovered management costs. For the observer costs just put that addition cost into the fixed cost value (i.e. normal fixed costs + observer cost = new value to put in for that param).",
					"", "");

	Parse_File(bm, fp, fileName, groupingNode, "ManagementCosts", "_mgmtcost", "Subfleet management costs", "", XML_TYPE_FLOATARRAY, bm->K_max_num_subfleet, TRUE,
			Create_Fishery_XMLNodes, FisheryFirstXMLFunction);
}

void createMinFixedTaxXML(MSEBoxModel *bm, xmlNodePtr parent) {
	/* Do nothing */
}
void MinFixedTaxXML(MSEBoxModel *bm, char *fileName, xmlNodePtr parent, char *str, char *valueStr) {

	Util_XML_Set_Node_Value(ATLANTIS_ATTRIBUTE, parent->parent, str, valueStr);

}

/**
 * \brief Will cut the string where there is an '_'.
 */
void TowTimeXMLFunction(MSEBoxModel *bm, char *fileName, xmlNodePtr parent, char *str, char *valueStr) {

	//char *fisheryStr;
	int fisheryIndex;

	str = str + strlen("TowTime");

	fisheryIndex = Util_Get_Fishery_Index(bm, str);
	Util_XML_Set_Node_Value(ATLANTIS_FISHERY_ATTRIBUTE, parent, FisheryArray[fisheryIndex].fisheryCode, valueStr);

}

/**
 */
void ECONexpriceXMLFunction(MSEBoxModel *bm, char *fileName, xmlNodePtr parent, char *str, char *valueStr) {
	int speciesIndex;

	str = str + strlen("ECONexprice");

	speciesIndex = Util_Get_FG_Index_From_Token(bm, str, fileName, str, TRUE);

	Util_XML_Set_Node_Value(ATLANTIS_GROUP_ATTRIBUTE, parent, FunctGroupArray[speciesIndex].groupCode, valueStr);
}

void Dan_XML(MSEBoxModel *bm, FILE *fp, char *fileName, xmlDocPtr doc, xmlNodePtr rootnode) {

	xmlNodePtr groupingNode;
	groupingNode = Util_XML_Create_Node(ATLANTIS_ATTRIBUTE_SUB_GROUP, rootnode, "DanHolland", "Parameters to do with Dan Holland style bioeconomics model",
			"", "");

	Util_XML_Parse_Create_Node(
			fp, fileName,
			groupingNode,
			"ITQ_think_ahead",
			"ITQ_think_ahead set it to 1 for the fishers to think ahead when using Dan's code and 0 if you want it exactly as Dan wrote it. To think ahead the fishers use maxland = min(totland, quota_left* month_expect_cath /catch_now_to_years_end) Instead of Dan's maxland = min(totland,quota_left)",
			"", XML_TYPE_FLOAT, "0.0");
	Util_XML_Parse_Create_Node(fp, fileName, groupingNode, "high_price", "This is the arbitrary high price to set quota price to if quota exceeded (from Dan's code)", "", XML_TYPE_FLOAT, "0.0");

	Create_Harvest_Fishery_Group_ParamXML(bm, fp, fileName, groupingNode, tax_id,
			"Penalty tax ($/kg) - as disincentive on bycatch - one entry per fishery", "", XML_TYPE_FLOATARRAY);

	Parse_File(bm, fp, fileName, groupingNode, "FixedMinTax", "^FixedMinTax",
			"Penalty tax ($/kg) - as disincentive on bycatch - one entry per fishery (needn't be identical, but is in AMS case).", "", XML_TYPE_FLOATARRAY,
			bm->K_num_tot_sp, TRUE, createMinFixedTaxXML, MinFixedTaxXML);

	Parse_File(bm, fp, fileName, groupingNode, "Betarev", "_betarev$", "betarev is a parameter to concentrate effort set equal to 1 initially", "", XML_TYPE_FLOATARRAY,
			bm->K_max_num_subfleet, TRUE, Create_Fishery_XMLNodes, FisheryFirstXMLFunction);

	Parse_File(bm,
			fp,fileName,
			groupingNode,
			"MinProb",
			"_minprob$",
			"minprob is minimum probability score that is minimum probability score for a location choice below which that choice is eliminated from the subfleet's choice set. Choice probabilties are then renormalized based on the scores of the remaining areas",
			"", XML_TYPE_FLOATARRAY, bm->K_max_num_subfleet, TRUE, Create_Fishery_XMLNodes, FisheryFirstXMLFunction);

	Parse_File(bm, fp, fileName, groupingNode, "TowTime", "^TowTime",
			"Area specific tow times (actually representing differential steam times as different distances from port", "", XML_TYPE_FLOATARRAY, bm->nbox, TRUE,
			Create_Fishery_XMLNodes, TowTimeXMLFunction);

	Parse_File(bm, fp, fileName, groupingNode, "ECONexprice", "^ECONexprice", "Load area specific prices", "", XML_TYPE_FLOATARRAY, bm->nbox, TRUE, Create_Fished_Species_XMLNodes,
			ECONexpriceXMLFunction);
}

void Convert_Economic_To_XML(MSEBoxModel *bm, char *fileName, char *outputFileName) {
	xmlDocPtr doc;
	xmlNodePtr rootnode, attributeGroupNode;
	FILE *fp;
	int MultiPlanEffort;

	/* Init libxml */
	xmlInitParser();
	LIBXML_TEST_VERSION

	if ((fp = Open_Input_File(bm->inputFolder,fileName, "rt")) == NULL) {
		quit("Cannot open economic input file %s%s\n", bm->inputFolder, fileName);
	}

	doc = xmlNewDoc((unsigned char *) "1.0");
	if (doc == NULL) {
		fprintf(stderr, "Document not created successfully. \n");
		return;
	}

	rootnode = xmlNewDocNode(doc, NULL, (const xmlChar*) "AtlantisEconomic", NULL);
	xmlDocSetRootElement(doc, rootnode);

	economicFlagParamsXML(bm, fp, fileName, doc, rootnode);
	ByCatch_Incentives_XML(bm, fp, fileName, doc, rootnode);
	Market_Characteristics_XML(bm, fp, fileName, doc, rootnode);
	Vessel_Characteristics_XML(bm, fp, fileName, doc, rootnode);
	Subfleet_Characteristics_XML(bm, fp, fileName, doc, rootnode);
	Quota_XML(bm, fp, fileName, doc, rootnode);
	Econ_Indicators_XML(bm, fp, fileName, doc, rootnode);
	OwnQuota_XML(bm, fp, fileName, doc, rootnode);
	BlackBook_XML(bm, fp, fileName, doc, rootnode);
	ManagementCosts_XML(bm, fp, fileName, doc, rootnode);

	attributeGroupNode = Util_XML_Get_Node(ATLANTIS_ATTRIBUTE_SUB_GROUP, rootnode->doc->children, "Economic_Flags");
	if (attributeGroupNode == NULL)
		quit("Chemical_Limitation attribute group not found in input file %s.\n", fileName);

	Util_XML_Get_Value_Integer(fileName, ATLANTIS_ATTRIBUTE, bm->ecotest, 1, attributeGroupNode, binary_check, "MultiPlanEffort", &MultiPlanEffort);

	if(!MultiPlanEffort){
		Dan_XML(bm, fp, fileName, doc, rootnode);
	}

	/* Save to the output file */
	xmlSaveFormatFileDestFolder(bm->destFolder, outputFileName, doc, 1);
	xmlFreeDoc(doc);
	fclose(fp);

	/* Shutdown libxml */
	xmlCleanupParser();
	return;
}
