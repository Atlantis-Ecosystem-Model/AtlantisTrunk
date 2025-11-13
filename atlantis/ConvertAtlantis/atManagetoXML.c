/**

 \file
 \brief This file contains functions to convert the manage prm file to the manage.XML file. Only the management paremeters will be pulled
 into the new XML file.



 There are a number of different types of input data:

 1. Flags for each fishery - these are stored in bm->FISHERYprms
 2. Flags for species - these are stored in the FunctGroupArray speciesParams arrays.
 3. Flags for species in each fishery - these are stored in bm->SP_FISHERYprms.

 Changes:


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
#include <atlantisboxmodel.h>
#include <atUtilLib.h>
#include <atHarvestLib.h>
#include "convertXML.h"


void Fix_Not_Fished_Species_Last_XMLFunction(MSEBoxModel *bm, char *fileName, xmlNodePtr parent, char *str, char *valueStr) {
	char seps[] = " ";
	char *varstr;
	int flag = TRUE;
	char *strPtr;
	char newValueStr[5000]; /* the value string once it has been padded with 0 values for the non fished groups */

	strcpy(newValueStr, "");

	strPtr = valueStr;

	while ((varstr = strtok(strPtr, seps)) != NULL) {
		strPtr = NULL;
		if (atoi(varstr) == 45) /* Very worring as the Not_fished_id value should be 51 */
			strcat(newValueStr, "-1");
		else
			strcat(newValueStr, varstr);

		if (flag)
			strcat(newValueStr, " ");
		flag = FALSE;
	}
	Species_Last_XMLFunction(bm, fileName, parent, str, newValueStr);
}

/**
 *	\brief Create the XML structure to hold the vertebrate seasonal distribution data.
 */
void InitFisherySeasonalDistXML(MSEBoxModel *bm, xmlNodePtr parent) {

	xmlNodePtr fisheryNode;
	int fisheryIndex;
	int season;
	char str[STRLEN];

	/* Create a node for each fishery*/
	for (fisheryIndex = 0; fisheryIndex < bm->K_num_fisheries; fisheryIndex++) {
		fisheryNode = Util_XML_Create_Node(ATLANTIS_FISHERY_ATTRIBUTE, parent, FisheryArray[fisheryIndex].fisheryCode, "", "", "");

		for (season = 1; season <= 4; season++) {
			sprintf(str, "season%d", season);
			Util_XML_Create_Node(ATLANTIS_TEMPORAL_ATTRIBUTE, fisheryNode, str, "", "", "");

		}
	}
}

/**
 *	\brief Create the XML structure to hold the bi-monthly TAC values.
 */
void InitBiMonthlyTACXML(MSEBoxModel *bm, xmlNodePtr parent) {

	xmlNodePtr speciesNode;
	int speciesIndex;
	int region;
	char str[STRLEN];

	/* Create a node for each impacted functional group */
	for (speciesIndex = 0; speciesIndex < bm->K_num_tot_sp; speciesIndex++) {
		if (FunctGroupArray[speciesIndex].isFished == TRUE) {
			speciesNode = Util_XML_Create_Node(ATLANTIS_GROUP_ATTRIBUTE, parent, FunctGroupArray[speciesIndex].groupCode, "", "", "");
			for (region = 1; region < totalreg_id + 1; region++) {
				sprintf(str, "region%d", region);
				Util_XML_Create_Node(ATLANTIS_ATTRIBUTE, speciesNode, str, "", "", "");
			}
		}
	}
}
void BiMonthlyXML(MSEBoxModel *bm, char *fileName, xmlNodePtr parent, char *str, char *valueStr) {

	char groupStr[MAX_GROUPCODE_LENGTH];
	char *indexStr;
	int index;
	int speciesIndex;
	xmlNodePtr speciesNode;
	char regionStr[STRLEN];

	indexStr = str + strlen("BiTAC_Reg");
	strcpy(groupStr, strstr(indexStr, "_") + 1);
	*(strstr(indexStr, "_")) = '\0';

	index = atoi(indexStr);

	/* get the fisheryIndex */
	speciesIndex = Util_Get_FG_Index_From_Token(bm, groupStr, fileName, str, TRUE);


	if (index < 0)
		quit("FBoMonthlyXML - Failed to get the index value %s\n", indexStr);

	speciesNode = Util_XML_Get_Or_Create_Node(ATLANTIS_GROUP_ATTRIBUTE, parent, FunctGroupArray[speciesIndex].groupCode);
	sprintf(regionStr, "region%d", index);
	Util_XML_Set_Node_Value(ATLANTIS_ATTRIBUTE, speciesNode, regionStr, valueStr);
}

void RegionalTACXML(MSEBoxModel *bm, char *fileName, xmlNodePtr parent, char *str, char *valueStr) {

	char *groupStr;
	int speciesIndex;

	groupStr = str + strlen("RegTAC_");
	/* get the fisheryIndex */
	speciesIndex = Util_Get_FG_Index_From_Token(bm, groupStr, fileName, str, TRUE);

	Util_XML_Set_Node_Value(ATLANTIS_GROUP_ATTRIBUTE, parent, FunctGroupArray[speciesIndex].groupCode, valueStr);
}

/**
 *    \brief Create the XML structure to hold the co_sp_catch data.
 *
 *
 *    <co_sp_catch>
 *        <FPL>
 *            <companion1>
 *            </companion1>
 *            <companion2>
 *            </companion2>
 *        </FPL>
 *
 *    </co_sp_catch>
 *
 */
void Init_Co_Sp_CatchXML(MSEBoxModel *bm, xmlNodePtr parent) {

    xmlNodePtr groupNode;
    int guild, co_sp;
    char str[50];

    /* Create a node for each functional group*/
    for (guild = 0; guild < bm->K_num_tot_sp; guild++) {
        if (FunctGroupArray[guild].isFished == TRUE) {
            groupNode = Util_XML_Create_Node(ATLANTIS_GROUP_ATTRIBUTE, parent, FunctGroupArray[guild].groupCode, "", "", "");

            for (co_sp = 0; co_sp < bm->K_max_co_sp; co_sp++) {
                sprintf(str, "companion%d", co_sp + 1);
                Util_XML_Create_Node(ATLANTIS_COMPANION_ATTRIBUTE, groupNode, str, "", "", "");
            }
        }
    }
}

void Co_Sp_CatchXMLFunction(MSEBoxModel *bm, char *fileName, xmlNodePtr parent, char *str, char *valueStr) {

    int cospIndex;
    char *lastLetter;
    char *secondlastLetter;
    char speciesStr[50];
    int i;
    char step1Str[50];
    char tempStr[50];
    char attributeName[100];
    int speciesIndex;
    xmlNodePtr speciesNode;

    // Find stock index
    strcpy(tempStr, str);
    strcpy(step1Str, str);
    lastLetter = step1Str + strlen(step1Str) - 1;
    secondlastLetter = step1Str + strlen(step1Str) - 2;

    if(isdigit(secondlastLetter[0])) {  // Is the first digit a number if yes then a two digit case
        cospIndex = atoi(secondlastLetter);
        tempStr[strlen(secondlastLetter) - strlen("_co_sp_catch") - 2] = '\0';        
    } else {  // No make it a single string case
        cospIndex = atoi(lastLetter);
        tempStr[strlen(tempStr) - strlen("_co_sp_catch") - 1] = '\0';
    }

    printf("Checking string %s\n", tempStr);
    
    // Get the species string section of the string.
    for (i = 0; isalpha(tempStr[i]); i++) {
        speciesStr[i] = tempStr[i];
    }
    speciesStr[i] = '\0';
    speciesIndex = Util_Get_FG_Index_From_Token(bm, speciesStr, fileName, str, TRUE);
    speciesNode = Util_XML_Get_Or_Create_Node(ATLANTIS_GROUP_ATTRIBUTE, parent, FunctGroupArray[speciesIndex].groupCode);

    sprintf(attributeName, "companion%d", cospIndex);
    Util_XML_Set_Node_Value(ATLANTIS_COMPANION_ATTRIBUTE, speciesNode, attributeName, valueStr);

}

void MakeBasketTACXML(MSEBoxModel *bm, char *fileName, xmlNodePtr parent, char *str, char *valueStr) {

    char *groupStr;
    int speciesIndex;

    groupStr = str + strlen("BasketTAC_");
    /* get the fisheryIndex */
    speciesIndex = Util_Get_FG_Index_From_Token(bm, groupStr, fileName, str, TRUE);

    Util_XML_Set_Node_Value(ATLANTIS_GROUP_ATTRIBUTE, parent, FunctGroupArray[speciesIndex].groupCode, valueStr);
}

void FisheriesVerticalEffortDistXML(MSEBoxModel *bm, char *fileName, xmlNodePtr parent, char *str, char *valueStr) {

	Fisheries_NodeXML(bm, fileName, parent, str, valueStr, "Effort_vdistrib");
}
void FisheriesEffortChangePulseXML(MSEBoxModel *bm, char *fileName, xmlNodePtr parent, char *str, char *valueStr) {

	Fisheries_NodeXML(bm, fileName, parent, str, valueStr, "flagpulse");
}

void FisheriesEffortDistributionXML(MSEBoxModel *bm, char *fileName, xmlNodePtr parent, char *str, char *valueStr) {

	char *fisheryStr;
	int index;
	int fisheryIndex;
	xmlNodePtr fisheryNode;
	char stockStr[STRLEN];

	fisheryStr = str + strlen("Effort_hdistrib");

	index = atoi(fisheryStr + strlen(fisheryStr) - 1);

	/* get the fisheryIndex */
	*(fisheryStr + strlen(fisheryStr) - 1) = '\0';
	fisheryIndex = Util_Get_Fishery_Index(bm, fisheryStr);

	if (fisheryIndex < 0)
		quit("FisheriesEffortDistributionXML - Failed to get the index value %s\n", fisheryStr);

	fisheryNode = Util_XML_Get_Or_Create_Node(ATLANTIS_FISHERY_ATTRIBUTE, parent, FisheryArray[fisheryIndex].fisheryCode);

	sprintf(stockStr, "season%d", index);
	Util_XML_Set_Node_Value(ATLANTIS_TEMPORAL_ATTRIBUTE, fisheryNode, stockStr, valueStr);
}

/**
 *	\brief Create the XML structure to hold the vertebrate seasonal distribution data.
 */
void InitPortChangeXML(MSEBoxModel *bm, xmlNodePtr parent) {

	int index;
	char str[STRLEN];

	/* create a node for each port */
	for (index = 1; index < bm->K_num_ports + 1; index++) {
		sprintf(str, "port%d", index);
		Util_XML_Create_Node(ATLANTIS_ATTRIBUTE, parent, str, "", "", "");
	}
}

void PortChangeXML(MSEBoxModel *bm, char *fileName, xmlNodePtr parent, char *str, char *valueStr) {

	char *indexStr;
	int index;
	char portStr[STRLEN];

	/* Get the pointer to the last instance of '_' */
	while (strchr(str, '_') != NULL) {
		str = strchr(str, '_');
		str++;
	}
	indexStr = str + strlen("Port");
	index = atoi(indexStr);

	sprintf(portStr, "port%d", index);
	Util_XML_Set_Node_Value(ATLANTIS_ATTRIBUTE, parent, portStr, valueStr);
}

void createImpactedSpeciesParam(MSEBoxModel *bm, char *fileName, xmlNodePtr parent, char *str, char *valueStr) {

	int speciesIndex;
	char *varstr;
	char seps[] = " ";

	/* Now find the node for each group and use the appropriate value in the list */
	for (speciesIndex = 0; speciesIndex < bm->K_num_tot_sp; speciesIndex++) {

		if (speciesIndex == 0)
			varstr = strtok(valueStr, seps);
		else
			varstr = strtok(NULL, seps);

		if (FunctGroupArray[speciesIndex].isImpacted == TRUE) {

			if (varstr == NULL) {
				quit("ERROR: createImpactedSpeciesParam - Parameter %s is not the required length in file %s. A value is required for each impacted group.\n", str, fileName);
			}

			/* get the species node */
			Util_XML_Set_Node_Value(ATLANTIS_GROUP_ATTRIBUTE, parent, FunctGroupArray[speciesIndex].groupCode, varstr);
		}
	}
}

void createFishedSpeciesParam(MSEBoxModel *bm, char *fileName, xmlNodePtr parent, char *str, char *valueStr) {

	int speciesIndex;
	char *varstr;
	char seps[] = " ";

	/* Now find the node for each group and use the appropriate value in the list */
	for (speciesIndex = 0; speciesIndex < bm->K_num_tot_sp; speciesIndex++) {

		if (speciesIndex == 0)
			varstr = strtok(valueStr, seps);
		else
			varstr = strtok(NULL, seps);

		if (FunctGroupArray[speciesIndex].isFished == TRUE) {

			if (varstr == NULL) {
				quit("ERROR: createFishedSpeciesParam - Parameter %s is not the required length in file %s. A value is required for each fished group.\n", str, fileName);
			}

			/* get the species node */
			Util_XML_Set_Node_Value(ATLANTIS_GROUP_ATTRIBUTE, parent, FunctGroupArray[speciesIndex].groupCode, varstr);
		}
	}
}

void createSpeciesParam(MSEBoxModel *bm, char *fileName, xmlNodePtr parent, char *str, char *valueStr) {

	int speciesIndex;
	char *varstr;
	char seps[] = " ";

	/* Now find the node for each group and use the appropriate value in the list */
	for (speciesIndex = 0; speciesIndex < bm->K_num_tot_sp; speciesIndex++) {

		if (speciesIndex == 0)
			varstr = strtok(valueStr, seps);
		else
			varstr = strtok(NULL, seps);

		if (varstr == NULL) {
			quit("ERROR: createSpeciesParam - Parameter %s is not the required length in file %s. A value is required for each functional group.\n", str, fileName);
		}

		/* get the species node */
		Util_XML_Set_Node_Value(ATLANTIS_GROUP_ATTRIBUTE, parent, FunctGroupArray[speciesIndex].groupCode, varstr);
	}
}


void createFisheriesParam(MSEBoxModel *bm, char *fileName, xmlNodePtr parent, char *str, char *valueStr) {

	int fisheryIndex;
	char *varstr;
	char seps[] = " ";

	/* Now find the node for each group and use the appropriate value in the list */
	for (fisheryIndex = 0; fisheryIndex < bm->K_num_fisheries; fisheryIndex++) {
		if (fisheryIndex == 0)
			varstr = strtok(valueStr, seps);
		else
			varstr = strtok(NULL, seps);

		if (varstr == NULL) {
			quit("ERROR: Parameter %s is not the required %d length in file %s\n", str, bm->K_num_fisheries, fileName);
		}

		/* set the nod evalue */
		Util_XML_Set_Node_Value(ATLANTIS_FISHERY_ATTRIBUTE, parent, FisheryArray[fisheryIndex].fisheryCode, varstr);

	}
}

void CellMPAProportionXML(MSEBoxModel *bm, char *fileName, xmlNodePtr parent, char *str, char *valueStr) {

	if ((strstr(str, "endangered") == NULL) && (strstr(str, "overfished") == NULL))
		Fisheries_NodeXML(bm, fileName, parent, str, valueStr, "MPA");
}
void MPAEndangeredXML(MSEBoxModel *bm, char *fileName, xmlNodePtr parent, char *str, char *valueStr) {

	Fisheries_NodeXML(bm, fileName, parent, str, valueStr, "MPAendangered");
}
void MPAoverfishedXML(MSEBoxModel *bm, char *fileName, xmlNodePtr parent, char *str, char *valueStr) {

	Species_NodeXML(bm, fileName, parent, str, valueStr, "MPAoverfished");
}

void EffortPenaltyXML(MSEBoxModel *bm, char *fileName, xmlNodePtr parent, char *str, char *valueStr) {

	Fisheries_NodeXML(bm, fileName, parent, str, valueStr, "EffortPenalty");
}

void ManamentFlagTimeXML(MSEBoxModel *bm, FILE *fp, char *fileName, xmlDocPtr doc, xmlNodePtr rootnode) {

	xmlNodePtr groupingNode, node, lookupNode;
	groupingNode = Util_XML_Create_Node(ATLANTIS_ATTRIBUTE_SUB_GROUP, rootnode, "Management_Flags", "Management Flags", "", "");

	Util_XML_Parse_Create_Node(fp, fileName, groupingNode, "useRBCTiers", "Whether assessments is tiered (1) yes (0) no", "", XML_TYPE_BOOLEAN,"0");
    bm->useRBCTiers = (int) (Util_XML_Read_Value(fileName, ATLANTIS_ATTRIBUTE,  bm->ecotest, 1, groupingNode, binary_check, "useRBCTiers"));
    if (bm->useRBCTiers) {
        Util_XML_Parse_Create_Node(fp, fileName, groupingNode, "useMultispAssess", "Whether doing multispecues assessment or not", "", XML_TYPE_BOOLEAN,"0");
        bm->useMultispAssess = (int) (Util_XML_Read_Value(fileName, ATLANTIS_ATTRIBUTE,  bm->ecotest, 1, groupingNode, binary_check, "useMultispAssess"));
    }
    
    Util_XML_Parse_Create_Node(fp, fileName, groupingNode, "usingRedus_R_HCR", "Whether using Redus HCR", "", XML_TYPE_BOOLEAN,"0");
    
	Util_XML_Parse_Create_Node(fp, fileName, groupingNode, "flagStoreShotCPUE", "1=store shot level CPUE, 0=off", "", XML_TYPE_BOOLEAN,"0");
	Util_XML_Get_Value_Integer(fileName, ATLANTIS_ATTRIBUTE, bm->ecotest, 1, groupingNode, binary_check, "flagStoreShotCPUE", &bm->flagStoreShotCPUE);
	Util_XML_Parse_Create_Node(fp, fileName, groupingNode, "flagStoreCPUE", "1=store per time step level CPUE, 0=off", "", XML_TYPE_BOOLEAN,"0");
	Util_XML_Get_Value_Integer(fileName, ATLANTIS_ATTRIBUTE, bm->ecotest, 1, groupingNode, binary_check, "flagStoreCPUE", &bm->flagStoreCPUE);
    
    Util_XML_Parse_Create_Node(fp, fileName, groupingNode, "flagfinfish", "1=finfish fisheries active, 0=off", "", XML_TYPE_BOOLEAN,"0");
	Util_XML_Parse_Create_Node(fp, fileName, groupingNode, "flagincidmort", "1=fisheries cause incidental mortality (take bycatch), 0=off ", "", XML_TYPE_BOOLEAN,"0");

	Util_XML_Parse_Create_Node(fp, fileName, groupingNode, "flaghighgrading", "1=high grading, 0=no high grading", "", XML_TYPE_BOOLEAN,"0");
	Util_XML_Parse_Create_Node(fp, fileName, groupingNode, "flagmarketdiscard", "1=market-based discarding, 0=not on", "", XML_TYPE_BOOLEAN,"0");
	Util_XML_Parse_Create_Node(fp, fileName, groupingNode, "UpdateMaxSalePrice", "", "", XML_TYPE_BOOLEAN,"0");

    Util_XML_Parse_Create_Node(fp, fileName, groupingNode, "flagkeepZeroCatchTS", "", "", XML_TYPE_BOOLEAN,"0");
    
	Util_XML_Parse_Create_Node(fp, fileName, groupingNode, "flagSSBforHCR", "1=use SBB for HCR, 0=use total B for HCR", "", XML_TYPE_BOOLEAN,"0");
	Util_XML_Parse_Create_Node(fp, fileName, groupingNode, "flagendangered", "1=endangered group status effect effort allocation, 0=off", "", XML_TYPE_BOOLEAN,"0");
	Util_XML_Parse_Create_Node(fp, fileName, groupingNode, "flagmpa", " 0=no mpa for any group, 1=mpas for at least one group", "", XML_TYPE_BOOLEAN,"0");
	Util_XML_Parse_Create_Node(fp, fileName, groupingNode, "flaginfringe", "0=no infringement, 1=infringement of mpas", "", XML_TYPE_BOOLEAN,"0");
	Util_XML_Parse_Create_Node(fp, fileName, groupingNode, "flagdisplace", "1=effort displacement due to low CPUE or MPA present, 0=off", "", XML_TYPE_BOOLEAN,"0");
	Util_XML_Parse_Create_Node(fp, fileName, groupingNode, "flagbuffereffort", "1=effort buffering on for dynamic fleet model, 0 = off", "", XML_TYPE_BOOLEAN,"0");
	Util_XML_Parse_Create_Node(fp, fileName, groupingNode, "flagchangeeffort", "1=fishing pressure changes with time, 0=remains constant", "", XML_TYPE_BOOLEAN,"0");
	Util_XML_Parse_Create_Node(fp, fileName, groupingNode, "flagchangecap", "1=effort cap on and changes through time, 0=off or remain constant", "", XML_TYPE_BOOLEAN,"0");
	Util_XML_Parse_Create_Node(fp, fileName, groupingNode, "flagnewport", "1=new fishing ports may open, 0=only use inital ports", "", XML_TYPE_BOOLEAN,"0");
	Util_XML_Parse_Create_Node(fp, fileName, groupingNode, "flagchangepop", "1=human population size changes, 0=remains constant", "", XML_TYPE_BOOLEAN,"0");

	Util_XML_Parse_Create_Node(fp, fileName, groupingNode, "flag_sel_with_mFC", "Flag indicating whether use selectivity to indicate ages where fishing mortality applied", "", XML_TYPE_BOOLEAN,"0");
	Util_XML_Parse_Create_Node(fp, fileName, groupingNode, "flag_stop_F_tac", "Flag indicating whether use TAC to limit fishing where fishing mortality applied", "", XML_TYPE_BOOLEAN,"0");

    Util_XML_Parse_Create_Node(fp, fileName, groupingNode, "maxF_aggregate", "Flag to do with system cap calculations - indicagging whether want maxF as sum over mFC or max of mFC", "", XML_TYPE_BOOLEAN,"0");
    Util_XML_Parse_Create_Node(fp, fileName, groupingNode, "use_time_avg_biom", "Flag to do with system cap calculations -  if using B not N in calculating expected catch, is whether use avg min,max B stored over the year or rolling_B", "", XML_TYPE_BOOLEAN,"0");
    Util_XML_Parse_Create_Node(fp, fileName, groupingNode, "use_time_avg_wgt", "Flag to do with system cap calculations -  if using N not B in calculating expected catch, is whether use avg min,max wgt stored over the year or rolling_wgt ", "", XML_TYPE_BOOLEAN,"0");
    Util_XML_Parse_Create_Node(fp, fileName, groupingNode, "syst_cap_calc_method", "Flag to do with system cap calculations  - whether use numbers based application of the catch equation or just average B", "", XML_TYPE_INTEGER,"0");
    Util_XML_Parse_Create_Node(fp, fileName, groupingNode, "M_est_method", "Flag to do with system cap calculations - which option for calculating M", "", XML_TYPE_INTEGER,"0");
    Util_XML_Parse_Create_Node(fp, fileName, groupingNode, "Ecosystm_Cap_tonnes", "Value of ecosystem catch cap in tonnes", "", XML_TYPE_FLOAT,"0");
    Util_XML_Parse_Create_Node(fp, fileName, groupingNode, "K_cap_rolling_period", "Period in years of the period used in calculating rollign average weight and biomass", "", XML_TYPE_FLOAT,"0");

    // Needed in defining arrays so read in immediately - use K_rolling_cap_num to define size of the array assumign monthly updates
    bm->K_cap_rolling_period = (Util_XML_Read_Value(fileName, ATLANTIS_ATTRIBUTE,  bm->ecotest, 1, groupingNode, no_checking, "K_cap_rolling_period"));
    bm->K_rolling_cap_num = (int)(ceil(bm->K_cap_rolling_period * 12.0));
    if(!bm->K_rolling_cap_num) {
        bm->K_rolling_cap_num = 1;
    }

	Util_XML_Parse_Create_Node(fp, fileName, groupingNode, "dynDAS", "Dynamic Days at Sea flag.", "", XML_TYPE_BOOLEAN,"0");

	Util_XML_Parse_Create_Node(fp, fileName, groupingNode, "flagreinitpop", "0=virgin biomass calc day 0, 1=calc on user specified day", "", XML_TYPE_BOOLEAN,"0");
	Util_XML_Parse_Create_Node(fp, fileName, groupingNode, "pseudo_assess", "1=use pseudo assessment not assessment library code", "", XML_TYPE_BOOLEAN,"0");
    Util_XML_Parse_Create_Node(fp, fileName, groupingNode, "do_sumB_HCR", "0 = at species level individual, 1=sum species in co_sp and apply any F responses to entire guild they define, 2=Alaska style system cap", "", XML_TYPE_INTEGER,"0");

	Util_XML_Parse_Create_Node(fp, fileName, groupingNode, "flagTACincludeDiscard", "Flag indicating whether or not TAC includes discards or just landed catch (1=yes, 0=no)",
			"", XML_TYPE_BOOLEAN,"1");
	Util_XML_Parse_Create_Node(fp, fileName, groupingNode, "flagTradeTACvsMPA",
			"Flag indicating whether or not trade off spatial closures for TAC reductions (i.e. no/less reduction as close area instead)", "", XML_TYPE_BOOLEAN,"1");
	Util_XML_Parse_Create_Node(fp, fileName, groupingNode, "flagQuotaBound", "Flag indicating whether want to bound dynamic TAC to be a band around original TAC", "", XML_TYPE_BOOLEAN,
			"1");

	Util_XML_Parse_Create_Node(
			fp,
			fileName,
			groupingNode,
			"flagdyn_coupdate",
			"Flag indicating whether fixed ratio of catches for companion species given in co_sp_catch is used throughout the run (0) or whether it is updated dynamically (1)",
			"", XML_TYPE_BOOLEAN,"1");

	node = Create_Fishery_ParamXML(bm, fp, fileName, groupingNode, flagmanage_id, "Flags indicating form of adaptive management used.", "", XML_TYPE_LOOKUP, "0");
	lookupNode = Util_XML_Create_Lookup_Node(node);
	Util_XML_Create_Lookup_Value_Node(lookupNode, "0", "None");
	Util_XML_Create_Lookup_Value_Node(
			lookupNode,
			"1",
			"Simple adaptive management which responds to the size of the target groups for this fishery. If a population has fallen to critical % of virgin stock size reduce overall effort (done gradually over a specified time period rather than as a sudden drop). If stock size is still too low after a set period its reduced again.");
	Util_XML_Create_Lookup_Value_Node(lookupNode, "2", "Standard TACs in place.");
	Util_XML_Create_Lookup_Value_Node(lookupNode, "3", "Basket TACone");
	Util_XML_Create_Lookup_Value_Node(lookupNode, "4", "Regional TAC (only for multiple stock groups so far)");
	Util_XML_Create_Lookup_Value_Node(lookupNode, "5", "Companion TAC");
	Util_XML_Create_Lookup_Value_Node(lookupNode, "6", "Companion TACs and basket quotas");
	Util_XML_Create_Lookup_Value_Node(lookupNode, "7", "Basket and regional quotas");
	Util_XML_Create_Lookup_Value_Node(lookupNode, "8", "Companion, regional and basket TACs");
	Util_XML_Create_Lookup_Value_Node(lookupNode, "9", "Companion, regional and basket TACs and basket groups can transition to individual TAC");

	Create_Fishery_ParamXML(bm, fp, fileName, groupingNode, flagTACpartipcate_id, "Flags indicating whether fishery participates in common pool TAC", "",
			XML_TYPE_BOOLEAN, "0");
	Create_Fishery_ParamXML(bm, fp, fileName, groupingNode, flagcap_id, "Flags indicating whether effort caps are used. 0=none, 1=yes.", "", XML_TYPE_BOOLEAN,"0");
	Create_Fishery_ParamXML(bm, fp, fileName, groupingNode, flagcap_peak_id, "Flags indicating whether effort caps are used. 0=none, 1=yes..", "", XML_TYPE_BOOLEAN,"0");
	Create_Fishery_ParamXML(bm, fp, fileName, groupingNode, flaguseall_id,
			"Flags indicating whether use all species caught to define CPUE for CPUE based effort models (1=yes) or not (0=use target species only).", "",
			XML_TYPE_BOOLEAN, "0");

	Create_Fishery_ParamXML(bm, fp, fileName, groupingNode, flagseasonal_id,
			"Flag indicating whether the fishery is seasonal. 0=no seasonal closures, 1=fishery has seasonal closures.", "", XML_TYPE_BOOLEAN,"0");

    Create_Fishery_ParamXML(bm, fp, fileName, groupingNode, flag_framebased_id,
            "Flag indicating whether the fishery is frame based. 0=no, 1=yes.", "", XML_TYPE_BOOLEAN,"0");
    
	node = Create_Fishery_ParamXML(
					bm,
					fp,
					fileName,
					groupingNode,
					flagmpa_id,
					"Flag indicating whether the fishery has spatial management.","", XML_TYPE_LOOKUP, "1");
    //0=no spatial management, \n1=fixed mpas,\n2=rotating mpas, \n3=closures when stock low, \n4=closures when endangered groups depleted, \n5=area closures when stock or endangered groups depleted, \n6=dynamic area closures when stock low, \n7=dynamic area closures when endangered groups depleted, \n8=dynamic area closures when stock or endangered groups depleted, \n9=mix of fixed and rolling mpas, \n10= mix of fixed, rolling and spawning mpas, \n11=depth and stock based mpas, \n12=council dictated mpas, \n13=spawning closures, n\14=catch based mpas

	lookupNode = Util_XML_Create_Lookup_Node(node);
	Util_XML_Create_Lookup_Value_Node(lookupNode, "0", "No spatial management");
	Util_XML_Create_Lookup_Value_Node(lookupNode, "1", "Fixed mpas");
	Util_XML_Create_Lookup_Value_Node(lookupNode, "2", "Rotating (or changing) mpas");
	Util_XML_Create_Lookup_Value_Node(lookupNode, "3", "Area closures when stock low (using fixed mpas)");
	Util_XML_Create_Lookup_Value_Node(lookupNode, "4", "Area closures when endangered groups depleted (using fixed mpas)");
	Util_XML_Create_Lookup_Value_Node(lookupNode, "5", "Area closures when stock low and/or when endangered groups depleted (using fixed mpas)");
	Util_XML_Create_Lookup_Value_Node(lookupNode, "6", "Area closures when stock low (using changing mpa locations)");
	Util_XML_Create_Lookup_Value_Node(lookupNode, "7", "Area closures when endangered groups depleted (using changing mpa locations)");
	Util_XML_Create_Lookup_Value_Node(lookupNode, "8", "Area closures when stock low and/or when endangered groups depleted (using changing mpa locations)");
    
    Util_XML_Parse_Create_Node(fp, fileName, groupingNode, "flagSimpleStartStopMPAs", " 0=use more complicated MPAs, 1=open/close all MPAs on single set day", "", XML_TYPE_BOOLEAN,"0");
    Util_XML_Get_Value_Integer(fileName, ATLANTIS_ATTRIBUTE, bm->ecotest, 1, groupingNode, binary_check, "flagSimpleStartStopMPAs", &bm->flagSimpleStartStopMPAs);
	if (bm->flagSimpleStartStopMPAs) {
        Util_XML_Parse_Create_Node(fp, fileName, groupingNode, "All_MPAstartyr", "day of run to open MPAs", "", XML_TYPE_INTEGER,"0");
        Util_XML_Parse_Create_Node(fp, fileName, groupingNode, "All_MPAendyr", "day of run to close MPAs", "", XML_TYPE_INTEGER,"0");
    }
    
    if (bm->flagStoreShotCPUE) {
        Util_XML_Parse_Create_Node(fp, fileName, groupingNode, "flagfullCPUEreport", "1=store all CPUE data including 'left overs', 0=just straight shots", "", XML_TYPE_BOOLEAN,"0");

        Util_XML_Parse_Create_Node(fp, fileName, groupingNode, "PerShotCDFtype", "Shape of cdf distribution for generating CPUE", "", XML_TYPE_INTEGER, "");
        Util_XML_Parse_Create_Node(fp, fileName, groupingNode, "CPUEshotfdis", "Flag indicating what error type for generating CPUE", "", XML_TYPE_INTEGER, "");
        Util_XML_Parse_Create_Node(fp, fileName, groupingNode, "CPUEshotavgdis", "Bias in error for generating CPUE", "", XML_TYPE_FLOAT, "0.8");
        Util_XML_Parse_Create_Node(fp, fileName, groupingNode, "CPUEshotvartdis", "Variance in error for generating CPUE", "", XML_TYPE_FLOAT, "0.1");
        Util_XML_Parse_Create_Node(fp, fileName, groupingNode, "CPUEshotfeff", "Flag indicating what error type for effort when generating CPUE", "", XML_TYPE_INTEGER, "");
        Util_XML_Parse_Create_Node(fp, fileName, groupingNode, "CPUEshotavgeff", "Bias in effort error for generating CPUE", "", XML_TYPE_FLOAT, "1.0");
        Util_XML_Parse_Create_Node(fp, fileName, groupingNode, "CPUEshotvareff", "Variance in effort error for generating CPUE", "", XML_TYPE_FLOAT, "0.1");
        Util_XML_Parse_Create_Node(fp, fileName, groupingNode, "gun_flexthresh", "Flex weight valiue that marks threshold from mediocre to effective skipper", "", XML_TYPE_FLOAT, "0.22");
        Util_XML_Parse_Create_Node(fp, fileName, groupingNode, "K_min_shotlength", "Minimum length of a shot (in hours)", "", XML_TYPE_FLOAT, "0.25");
        Util_XML_Parse_Create_Node(fp, fileName, groupingNode, "K_num_catchbin", "Number of bins in catch size dstribution", "", XML_TYPE_INTEGER, "10");
        
        Util_XML_Parse_Create_Node(fp, fileName, groupingNode, "size_catchbin", "Size of catch bins used to create catch distribution", "", XML_TYPE_FLOAT, "50.0");
        Util_XML_Parse_Create_Node(fp, fileName, groupingNode, "min_effort_coefft", "Coefficient in linear relationship between atch taken and effort spent to obtain it", "", XML_TYPE_FLOAT, "");
        Util_XML_Parse_Create_Node(fp, fileName, groupingNode, "min_effort_const", "Constant in linear relationship between atch taken and effort spent to obtain it", "", XML_TYPE_FLOAT, "");
        
        Create_Harvest_Fishery_Group_ParamXML(bm, fp, fileName, groupingNode, flagPerShotCPUE_id, "Flag indicating whether to generate CPUE per shot (on=1, off=0)", "", XML_TYPE_FLOATARRAY);
        
        Create_Species_ParamXML(bm, fileName, fp, groupingNode, cpue_cdf_poor_r_id, "Number of events for negative binomial cdf for poorer skippers", "", XML_TYPE_INTEGER,"0");
        Create_Species_ParamXML(bm, fileName, fp, groupingNode, cpue_cdf_poor_p_id, "Probabilty of an event for negative binomial cdf for poorer skipper", "", XML_TYPE_FLOAT,"0");
        Create_Species_ParamXML(bm, fileName, fp, groupingNode, cpue_cdf_top_r_id, "Number of events for negative binomial cdf for top skippers", "", XML_TYPE_INTEGER,"0");
        Create_Species_ParamXML(bm, fileName, fp, groupingNode, cpue_cdf_top_p_id, "Probabilty of an event for negative binomial cdf for top skippers", "", XML_TYPE_FLOAT,"0");

    }
    
    if(bm->flagStoreCPUE) {
        Create_Harvest_Fishery_Group_ParamXML(bm, fp, fileName, groupingNode, flagRecordCPUE_id, "Flag indicating want to store per time step general CPUE results (on=1, off=0)", "", XML_TYPE_FLOATARRAY);
    }

}

void ManagementSenarioFlagsXML(MSEBoxModel *bm, FILE *fp, char *fileName, xmlDocPtr doc, xmlNodePtr rootnode) {

	xmlNodePtr groupingNode;
	groupingNode = Util_XML_Create_Node(ATLANTIS_ATTRIBUTE_SUB_GROUP, rootnode, "ScenarioFlags", "Scenario specification flags", "", "");

	//Create_Fishery_ParamXML(bm, fp, fileName, groupingNode, flagchangeEFF_id, "CoverageChange", "_changeSEL$", "", "");
	Create_Fishery_ParamXML(
			bm,
			fp,
			fileName,
			groupingNode,
			flagchangeEFF_id,
			"Flag indicating whether the effort of a specific fishery changes through time. 0=no, 0=yes If any of these are set flagchangeeffort to 0 then set to 0 too",
			"", XML_TYPE_BOOLEAN,"0");
	Create_Fishery_ParamXML(bm, fp, fileName, groupingNode, flagchangeseason_id,
			"Flag indicating whether the length of the fishing season changes for a specific fishery changes through time. 0=no, 1=yes", "", XML_TYPE_BOOLEAN,"0");
	Create_Fishery_ParamXML(bm, fp, fileName, groupingNode, TACchange_id, "Flag indicating whether the TACs for a fishery change through time. 0=no, 1=yes", "", XML_TYPE_BOOLEAN, "0");

}

void ManagementMSYXML(MSEBoxModel *bm, FILE *fp, char *fileName, xmlDocPtr doc, xmlNodePtr rootnode) {

	xmlNodePtr groupingNode, node, lookupNode;
	groupingNode = Util_XML_Create_Node(ATLANTIS_ATTRIBUTE_SUB_GROUP, rootnode, "MSYParameters", "MSY related management parameters", "", "");

	node = Util_XML_Create_Node_Next_Line(fp, fileName, groupingNode, "UseLever",
			"When using MSY based management (tier6 above), then this identifies the lever to use for each group", "", XML_TYPE_INTEGER);
	lookupNode = Util_XML_Create_Lookup_Node(node);
	Util_XML_Create_Lookup_Value_Node(lookupNode, "0", "None");
	Util_XML_Create_Lookup_Value_Node(lookupNode, "1", "Effort");
	Util_XML_Create_Lookup_Value_Node(lookupNode, "2", "Catch (quota) 2      (Will require meaningful values in the TAC_XX parameter arrays below)");
	Util_XML_Create_Lookup_Value_Node(lookupNode, "3", "Spatial management 3  (Will require meaningful values in the MPA_YY parameter arrays below)");
	Util_XML_Create_Lookup_Value_Node(lookupNode, "4", "Season (Will require meaningful seasonopen and seasonclose values below)");
	Util_XML_Create_Lookup_Value_Node(lookupNode, "5", "Gear");

	Create_Fishery_ParamXML(bm, fp, fileName, groupingNode, use_min_lever_id,
			"Whether to rescale fisheries management levers based on minimum across species (1) or  the average (0)", "", XML_TYPE_BOOLEAN,"0");

}

void ManagementEffortXML(MSEBoxModel *bm, FILE *fp, char *fileName, xmlDocPtr doc, xmlNodePtr rootnode) {

	xmlNodePtr groupingNode, childGroupingNode;
	groupingNode = Util_XML_Create_Node(ATLANTIS_ATTRIBUTE_SUB_GROUP, rootnode, "Effort", "Effort model parameters", "", "");

	Parse_File(bm, fp, fileName, groupingNode, "PerscribedEffort", "^mEff_", "Perscribed effort, per quarter", "d-1", XML_TYPE_FLOATARRAY, 4, TRUE,
			Create_Fishery_XMLNodes, Fishery_Last_XMLFunction);

	Create_Fishery_ParamXML(bm, fp, fileName, groupingNode, mEff_max_id, "Maximum level of effort for dynamic effort model", "d-1", XML_TYPE_FLOAT, "0.001");
	Create_Fishery_ParamXML(bm, fp, fileName, groupingNode, mEff_a_id, "Coefficient in dynamic effort model", "", XML_TYPE_FLOAT, "100.0");
	Create_Fishery_ParamXML(bm, fp, fileName, groupingNode, mEff_offset_id,
			"Offset in dynamic effort model (this value is taken away from the value calculated value so that can have zero effort in poor production boxes)",
			"", XML_TYPE_FLOAT, "100.0");
	Create_Fishery_ParamXML(bm, fp, fileName, groupingNode, mEff_testfish_id, "Exploratory fishing effort in dynamic effort model", "", XML_TYPE_FLOAT, "1.0");

	Create_Fishery_ParamXML(bm, fp, fileName, groupingNode, mEff_thresh_id,
			"Threshold CPUE for effort displacement (below this and fishery will try and displace effort to best stocked neighbouring cells)", "mg N m-3 d-1",
			XML_TYPE_FLOAT, "0.0");
	Create_Fishery_ParamXML(bm, fp, fileName, groupingNode, mEff_thresh_top_id, "CPUE threshold", "", XML_TYPE_FLOAT, "1.0");
	Create_Fishery_ParamXML(bm, fp, fileName, groupingNode, mEff_shift_id, "Effort decrease will be (1-mEff_prop) and increase will be (1+mEff_prop)", "", XML_TYPE_FLOAT,
			"0.0");

	Create_Fishery_ParamXML(bm, fp, fileName, groupingNode, mFCscale_id, "Distance scalar for each fishery (this scales impact of travel on effort allocation)",
			"", XML_TYPE_FLOAT, "1.0");

	Create_Fishery_ParamXML(bm, fp, fileName, groupingNode, mFCpeak_id,
			"Max proportion of the effort allowed in any one cell (for use with effort model 8 and flagcap_peak)", "", XML_TYPE_FLOAT, "1.0");

	Create_Fishery_ParamXML(bm, fp, fileName, groupingNode, EffortLevel_id,
			"Effort per day for each fishery (so this is effort in total days per day fished by the fishery)", "", XML_TYPE_FLOAT, "0 - 20800");

	Parse_File(bm, fp, fileName, groupingNode, "PrescribedEffortDistribution", "^Effort_hdistrib",
			"Prescribed effort distributions (total effort per box per in each quarter)", "", XML_TYPE_FLOATARRAY, bm->nbox, TRUE, InitFisherySeasonalDistXML,
			FisheriesEffortDistributionXML);

	Parse_File(bm, fp, fileName, groupingNode, "VerticalEffortDistribution", "^Effort_vdistrib",
			" Vertical effort distributions (proportion of effort for the box applied in each water column layer)", "", XML_TYPE_FLOATARRAY, bm->wcnz, TRUE,
			Create_Fishery_XMLNodes, FisheriesVerticalEffortDistXML);

	Parse_File(bm,
			fp,
			fileName,
			groupingNode,
			"EffortPenalty",
			"^EffortPenalty",
			"Effort panalties applied for fishing in different locations - Chris Wilcox idea to steer people to desired locaiton when using effort caps and ITE",
			"", XML_TYPE_FLOATARRAY, bm->nbox, TRUE, Create_Fishery_XMLNodes, EffortPenaltyXML);

	childGroupingNode = Util_XML_Create_Node(ATLANTIS_ATTRIBUTE_SUB_GROUP, rootnode, "CPUE_Thresholds",
			"CPUE thresholds - CPUEs above or below this tonnes/days-at-sea for a target species will cause an effort shift", "", "");

	Create_Fishery_ParamXML(bm, fp, fileName, childGroupingNode, CPUE_effort_thresh_id, "Proportional size of that shift is given by the scalar below", "",
			XML_TYPE_FLOAT, "0");

	Util_XML_Parse_Create_Node(fp, fileName, childGroupingNode, "explore_thresh_effort", "Proportion of effort above which exploratory fishing will occur in nearby cells", "",
			XML_TYPE_FLOAT, "0.10");

	Create_Fishery_ParamXML(bm, fp, fileName, childGroupingNode, CPUE_effort_scale_id,
			"Incremental proportional change in CPUE if pass CPUE threshold (i.e. 1+prop on way up and 1-prop on way down)", "", XML_TYPE_FLOAT, "0");
	Create_Fishery_ParamXML(bm, fp, fileName, childGroupingNode, cap_id,
			"Effort caps - size in total days at sea allowed for the entire fleet for the year. Only need be > 0  if flagcap set = 1", "d", XML_TYPE_INTEGER,
			"0 - 155000");
}

void PortXMLFunction(MSEBoxModel *bm, char *fileName, xmlNodePtr parent, char *str, char *valueStr) {

	char *fisheryStr;
	int fisheryIndex;
	if(strcmp(str, "ports_x") == 0 || strcmp(str, "ports_y") == 0 ||
			strcmp(str, "ports_start") == 0 || strcmp(str, "ports_end") == 0 ||
			strcmp(str, "ports_pop") == 0	|| strcmp(str, "ports_markets") == 0
			|| strcmp(str, "ports_popchange") == 0 || strcmp(str, "ports_popnumchange") == 0 )
		return;

	fisheryStr = strchr(str, '_');
	fisheryStr++;
	while (strchr(fisheryStr, '_') != NULL) {
		fisheryStr = strchr(fisheryStr, '_');
		fisheryStr++;
	}
	fisheryIndex = Util_Get_Fishery_Index(bm, fisheryStr);
	Util_XML_Set_Node_Value(ATLANTIS_FISHERY_ATTRIBUTE, parent, FisheryArray[fisheryIndex].fisheryCode, valueStr);
}

void PortIndexXML(MSEBoxModel *bm, FILE *fp, char *fileName, xmlDocPtr doc, xmlNodePtr rootnode) {

	xmlNodePtr groupingNode, childGroupingNode;
	groupingNode = Util_XML_Create_Node(ATLANTIS_ATTRIBUTE_SUB_GROUP, rootnode, "Port_Information", "Port Information", "", "");

	Parse_File(
			bm,
			fp,
			fileName,
			groupingNode,
			"FisheryPort",
			"^ports_.*",
			"Vectors indicating which ports each fishery can operate from initially (other ports may be activated using the flagnewport option in the switch and flag section at the top of the file). \n1 = from the port, 0 = not active from the port. \nThe list of ports are listed at the top of the parameter file - there must be as many entries as there are ports (K_num_ports)",
			"", XML_TYPE_BOOLEANARRAY, bm->K_num_ports, TRUE, Create_Fishery_XMLNodes, PortXMLFunction);

	childGroupingNode = Util_XML_Create_Node(ATLANTIS_ATTRIBUTE_SUB_GROUP, rootnode, "Location_And_Timing",
			"Port location and timing parameters - there must be as many entries as there are ports (K_num_ports).", "", "");
	Util_XML_Create_Node_Next_Line(fp, fileName, childGroupingNode, "ports_x", "X coordinate (in metres as in model space)", "", XML_TYPE_FLOAT);
	Util_XML_Create_Node_Next_Line(fp, fileName, childGroupingNode, "ports_y", "Y coordinate (in metres as in model space)", "", XML_TYPE_FLOAT);
	Util_XML_Create_Node_Next_Line(fp, fileName, childGroupingNode, "ports_start", "Time port first used (days from model start time)", "d", XML_TYPE_INTEGER);
	Util_XML_Create_Node_Next_Line(fp, fileName, childGroupingNode, "ports_end", "Time port last used (days from model start time)", "d", XML_TYPE_INTEGER);
	Util_XML_Create_Node_Next_Line(fp, fileName, childGroupingNode, "ports_pop", "Population of each port (numbers as of 2001)", "People", XML_TYPE_INTEGER);
	Util_XML_Create_Node_Next_Line(fp, fileName, childGroupingNode, "ports_markets", "Which market is feed by each port (for economic model)", "", XML_TYPE_BOOLEAN);
	Util_XML_Parse_Create_Node(fp, fileName, childGroupingNode, "k_proprecfish", "The proportion of the population who fish recreationally", "", XML_TYPE_FLOAT, "0.20");

}

/* Create the TAC xml ndoes */
void TACXML(MSEBoxModel *bm, FILE *fp, char *fileName, xmlDocPtr doc, xmlNodePtr rootnode) {

	xmlNodePtr groupingNode, childGroupingNode, node, lookupNode;
	groupingNode = Util_XML_Create_Node(ATLANTIS_ATTRIBUTE_SUB_GROUP, rootnode, "TAC_Parameters", "Total Allowable Catch Parameters", "", "");

	childGroupingNode = Util_XML_Create_Node(ATLANTIS_ATTRIBUTE_SUB_GROUP, rootnode, "Reference_Points",
			"Reference points and trend lengths for use in dynamic TAC setting", "", "");

	Util_XML_Parse_Create_Node(fp, fileName, childGroupingNode, "targ_refA", "Target reference point - tier1", "", XML_TYPE_FLOAT, "");
	Util_XML_Parse_Create_Node(fp, fileName, childGroupingNode, "targ_refB", "Target reference point - tier2", "", XML_TYPE_FLOAT, "");
	Util_XML_Parse_Create_Node(fp, fileName, childGroupingNode, "targ_refC", "Target reference point - tier3", "", XML_TYPE_FLOAT, "");
	Util_XML_Parse_Create_Node(fp, fileName, childGroupingNode, "targ_refD", "Target reference point - tier4", "", XML_TYPE_FLOAT, "");
    Util_XML_Parse_Create_Node(fp, fileName, childGroupingNode, "targ_refE", "Target reference point - tier4", "", XML_TYPE_FLOAT, "");
	Util_XML_Parse_Create_Node(fp, fileName, childGroupingNode, "lim_ref", "Limit reference point", "", XML_TYPE_FLOAT, "");

	Util_XML_Parse_Create_Node(fp, fileName, childGroupingNode, "forage_refA", "Forage target reference point - tier1", "", XML_TYPE_FLOAT, "");
	Util_XML_Parse_Create_Node(fp, fileName, childGroupingNode, "forage_refB", "Forage target reference point - tier2", "", XML_TYPE_FLOAT, "");
	Util_XML_Parse_Create_Node(fp, fileName, childGroupingNode, "forage_refC", "Forage target reference point - tier3", "", XML_TYPE_FLOAT, "");
	Util_XML_Parse_Create_Node(fp, fileName, childGroupingNode, "forage_refD", "Forage target reference point - tier4", "", XML_TYPE_FLOAT, "");
    Util_XML_Parse_Create_Node(fp, fileName, childGroupingNode, "forage_refE", "Forage target reference point - tier4", "", XML_TYPE_FLOAT, "");
	Util_XML_Parse_Create_Node(fp, fileName, childGroupingNode, "forage_lim_ref", "Forage limit reference point", "", XML_TYPE_FLOAT, "");

    Util_XML_Parse_Create_Node(fp, fileName, childGroupingNode, "byproduct_refA", "Byproduct target reference point - tier1", "", XML_TYPE_FLOAT, "");
    Util_XML_Parse_Create_Node(fp, fileName, childGroupingNode, "byproduct_refB", "Byproduct target reference point - tier2", "", XML_TYPE_FLOAT, "");
    Util_XML_Parse_Create_Node(fp, fileName, childGroupingNode, "byproduct_refC", "Byproduct target reference point - tier3", "", XML_TYPE_FLOAT, "");
    Util_XML_Parse_Create_Node(fp, fileName, childGroupingNode, "byproduct_refD", "Byproduct target reference point - tier4", "", XML_TYPE_FLOAT, "");
    Util_XML_Parse_Create_Node(fp, fileName, childGroupingNode, "byproduct_lim_ref", "Byproduct limit reference point", "", XML_TYPE_FLOAT, "");

    Util_XML_Parse_Create_Node(fp, fileName, childGroupingNode, "bycatch_refA", "Bycatch target reference point - tier1", "", XML_TYPE_FLOAT, "");
    Util_XML_Parse_Create_Node(fp, fileName, childGroupingNode, "bycatch_refB", "Bycatch target reference point - tier2", "", XML_TYPE_FLOAT, "");
    Util_XML_Parse_Create_Node(fp, fileName, childGroupingNode, "bycatch_refC", "Bycatch target reference point - tier3", "", XML_TYPE_FLOAT, "");
    Util_XML_Parse_Create_Node(fp, fileName, childGroupingNode, "bycatch_refD", "Bycatch target reference point - tier4", "", XML_TYPE_FLOAT, "");
    Util_XML_Parse_Create_Node(fp, fileName, childGroupingNode, "bycatch_lim_ref", "Bycatch limit reference point", "", XML_TYPE_FLOAT, "");
    
    Util_XML_Parse_Create_Node(fp, fileName, childGroupingNode, "discardTAC", "Discard TAC", "", XML_TYPE_FLOAT, "");
    
	Util_XML_Create_Node_Next_Line(fp, fileName, childGroupingNode, "whichref",
			"Flag to indicate which set of reference points to use. Enter a 1 to use targ_ref reference points or 0 to use forage reference points", "",
			XML_TYPE_INTEGER);

	Util_XML_Parse_Create_Node(fp, fileName, childGroupingNode, "TAC_multiyr", "Years of trend before change TAC", "", XML_TYPE_INTEGER, "");
	Util_XML_Parse_Create_Node(fp, fileName, childGroupingNode, "critCPUE", "Threshold CPUE before TAC changes", "", XML_TYPE_FLOAT, "");
	Util_XML_Parse_Create_Node(fp, fileName, childGroupingNode, "TACbound", "Maximum possible TAC increase through run (relative to starting TACs)", "", XML_TYPE_FLOAT, "");
	Util_XML_Parse_Create_Node(fp, fileName, childGroupingNode, "prop_incTAC", "Allowable proportional jump in TAC (scalar so if 20% jump put 1.2)", "", XML_TYPE_FLOAT, "");
	Util_XML_Parse_Create_Node(fp, fileName, childGroupingNode, "prop_decTAC", "Allowable proportional drop in TAC (scalar so if 10% drop put 0.9)", "", XML_TYPE_FLOAT, "");
	Util_XML_Parse_Create_Node(fp, fileName, childGroupingNode, "RBCalpha", "Coefficient for RBC calculations", "", XML_TYPE_FLOAT, "");
	Util_XML_Parse_Create_Node(fp, fileName, childGroupingNode, "TAC_crit_change",
			"Magnitude of TAC shift before actually implemented (t) - so don;t get caught up shifting it 10kg", "", XML_TYPE_FLOAT, "");

	Util_XML_Create_Node_Next_Line(fp, fileName, childGroupingNode, "estBo",
			"Virgin biomasses for use with pseudo assessment (typically dynamically calculated in model instead)", "", XML_TYPE_FLOAT);

    Util_XML_Create_Node_Next_Line(fp, fileName, childGroupingNode, "estError", "error type applied (per species) to 'perfect knowledge biomass estimate' for broken stick management (if not using formal assessment code)", "", XML_TYPE_INTEGER);
    Util_XML_Create_Node_Next_Line(fp, fileName, childGroupingNode, "estCV", "error variance applied (per species) to 'perfect knowledge biomass estimate' for broken stick management (if not using formal assessment code)", "", XML_TYPE_FLOAT);
    Util_XML_Create_Node_Next_Line(fp, fileName, childGroupingNode, "estBias", "bias applied (per species) to 'perfect knowledge biomass estimate' for broken stick management (if not using formal assessment code)", "", XML_TYPE_FLOAT);

    Util_XML_Create_Node_Next_Line(fp, fileName, childGroupingNode, "Fref", "Fishing rate (F) that equates with BrefA (i.e.F to apply if B > Bref)", "", XML_TYPE_FLOAT);

    Util_XML_Create_Node_Next_Line(fp, fileName, childGroupingNode, "Fref_High", "Fishing rate (F) that equates with BrefA (i.e.F to apply if B > Bref)", "", XML_TYPE_FLOAT);
    
    Util_XML_Create_Node_Next_Line(fp, fileName, childGroupingNode, "FrefLim", "Fishing rate (F) that is the F once the stocks statis is below Blim", "", XML_TYPE_FLOAT);
    
    Util_XML_Create_Node_Next_Line(fp, fileName, childGroupingNode, "Frestart_scalar", "Scalar on original mFC that becomes the new F after fishery reopened", "", XML_TYPE_FLOAT);
    
    childGroupingNode = Util_XML_Create_Node(ATLANTIS_ATTRIBUTE_SUB_GROUP, rootnode, "SystemCap_Parameters",
            "Settingd for system cap related management", "", "");

    Util_XML_Create_Node_Next_Line(fp, fileName, childGroupingNode, "FlagSystCapSP", "Flag for whether included in system cap or not", "", XML_TYPE_FLOAT);
    
    Util_XML_Create_Node_Next_Line(fp, fileName, childGroupingNode, "SystCapSPpref", "Weighting on species to consider in reallocaiton and ductions in response to exceeding the cap - 1 is low prioroity, number greater than 1 is a higher priority", "", XML_TYPE_FLOAT);
    
    if(!do_assess) {
        Util_XML_Create_Node_Next_Line(fp, fileName, childGroupingNode, "FixedAssessMort", "Fixed assessment M assumed for use in calculating the System Cap associated expected catches - overwritten with the assess_nat_mort vector values if assess.prm included in the run command", "", XML_TYPE_FLOAT);
    }
 
	node = Create_Species_ParamXML(bm, fileName, fp, groupingNode, tier_id,
				"Harvest strategy tier for RBC setting (only >0 if dynamically setting TACs). \n0 = whinging harvest strategy, 1-4 = tier 1-4, tier 5 = ecological indicator based",
				"", XML_TYPE_LOOKUP, "0");

	lookupNode = Util_XML_Create_Lookup_Node(node);
	Util_XML_Create_Lookup_Value_Node(lookupNode, "0", "Whinging harvest strategy");
	Util_XML_Create_Lookup_Value_Node(lookupNode, "1", "Tier 1");
	Util_XML_Create_Lookup_Value_Node(lookupNode, "2", "Tier 2");
	Util_XML_Create_Lookup_Value_Node(lookupNode, "3", "Tier 3");
	Util_XML_Create_Lookup_Value_Node(lookupNode, "4", "Tier 4");
	Util_XML_Create_Lookup_Value_Node(lookupNode, "5", "Tier 5 -ecological indicator based");

	Create_Harvest_Fishery_Group_ParamXML(
			bm,
			fp,
			fileName,
			groupingNode,
			trip_lim_id,
			"Trip Limit parameters - not necessarily associated with a TAC, this is a limit on total catch (in kg) of the group for that fishery per seven day period (if there are shorter trips in reality, \nsum over these trips to get 7 days duration and use the sum as the limit here).	\nIndividual trip limits (in tonnes) per fishery per group",
			"", XML_TYPE_FLOATARRAY);
	Create_Harvest_Fishery_Group_ParamXML(bm, fp, fileName, groupingNode, TAC_id, "Total allowable catch (TAC in tonnes) per fishery per group", "tonnes",
			XML_TYPE_FLOATARRAY);

	Parse_File(
			bm,
			fp,
			fileName,
			groupingNode,
			"BiMonthlyTAC",
			"^BiTAC_Reg",
			"Bimonthly trip limts (like in the US) - if you want to use these (currently only with Dan Holland economic model) then to enter them, fill in TAC above for entire year and then enter the proportion in each bimonthly period in the vectors below.\nYou must do one set for each management region (currently allows for 3 management regions)",
			"", XML_TYPE_FLOATARRAY, 6, TRUE, InitBiMonthlyTACXML, BiMonthlyXML);

	Create_Fishery_ParamXML(bm, fp, fileName, groupingNode, landallTAC_sp_id,
			"Whether the fishery must land all catch of quota groups regardless of quota remaining (i.e. no discards allowed if over quota).", "", XML_TYPE_BOOLEAN,"0");
	Create_Fishery_ParamXML(bm, fp, fileName, groupingNode, max_num_sp_id,
			"Maximum number of groups for which the fishery can exceed its TAC before being closed", "", XML_TYPE_INTEGER, "4");

	Create_Species_ParamXML(bm, fileName, fp, groupingNode, regionalSP_id, "True if the group is part of the regional quoa member", "", XML_TYPE_BOOLEAN,"0");

	Util_XML_Parse_Create_Node(fp, fileName, groupingNode, "manage_reg", "Whether TACs use stock based regional TAC (0) or management area regional TAC (1)", "", XML_TYPE_BOOLEAN,"0");

	Parse_File(
			bm,
			fp,
			fileName,
			groupingNode,
			"RegionalTAC",
			"^RegTAC_",
			"Proportional distribution of overall TACs across management regions - this read in is currently only used by the Dan Holland model (the standard regional management model has the management zones mapped to the reproductive stocks of the individual groups and so calculates the distribution directly rather than using this read-in value",
			"", XML_TYPE_FLOATARRAY, bm->K_num_reg, TRUE, Create_Fished_Species_XMLNodes, RegionalTACXML);

	Create_Species_ParamXML(bm, fileName, fp, groupingNode, basketSP_id, "Basket quotas flag", "", XML_TYPE_BOOLEAN,"0");
	Create_Species_ParamXML(bm, fileName, fp, groupingNode, basket_size_id, "Number of each groups in the quota", "", XML_TYPE_INTEGER, "1");
    
	Util_XML_Parse_Create_Node(fp, fileName, groupingNode, "bulkTAC", "Whether multi-year TACs are one total for the entire period or if it is annual quota to track just not doing an assessment for x years", "", XML_TYPE_BOOLEAN, "");

	Create_Species_ParamXML(bm, fileName, fp, groupingNode, tac_resetperiod_id, "Multi-year TAC - time (in years between resets)", "", XML_TYPE_FLOAT,"1");

	Create_Harvest_Fishery_Group_ParamXML(
			bm,
			fp,
			fileName,
			groupingNode,
			prop_spawn_close_id,
			"Proportion of spawning grounds closed when trading off vs TAC reduction of the group for that fishery per seven day period (if there are shorter trips in reality, sum over these trips to get 7 days duration and use the sum as the limit here). Individual trip limits (in tonnes) per fishery per group.",
			"", XML_TYPE_FLOATARRAY);

	/* change the species of concern into a species parameter instead of an array of values */

	Parse_File(
			bm,
			fp,
			fileName,
			groupingNode,
			"SP_Concern",
			"SP_Concern",
			"Identification of groups of concern - those groups regarded as threatened. If the stock of these groups falls too low then fisheries may be closed. A 1=threatened, 0=ignore",
			"", XML_TYPE_BOOLEANARRAY, bm->K_num_tot_sp, TRUE, Create_Impacted_Species_XMLNodes, createImpactedSpeciesParam);
    
    
    Parse_File(
            bm,
            fp,
            fileName,
            groupingNode,
            "BasketTAC",
            "^basketTAC_",
            "Basket quota membership (entries should be the guild id numbers (see top of file for values) for each species in the basket other than the basket_TACFxx group (i.e. in basketTAC_FPS do not put the code for FPS, only the species other than FPS in the basket with FPS). For an entry of 'no other species' (i.e. single species quota) put in a single entry with value -1. Array length should be BasketSize for each species.",
            "", XML_TYPE_FLOATARRAY, bm->K_num_basket, TRUE, Create_Fished_Species_XMLNodes, MakeBasketTACXML);

    groupingNode = Util_XML_Create_Node(ATLANTIS_ATTRIBUTE_SUB_GROUP, rootnode, "Companion_Parameters", "Companion Catch Parameters", "", "");

    Util_XML_Parse_Create_Node(fp, fileName, groupingNode, "K_max_co_sp", "Maximum number of companions in a companion TAC", "", XML_TYPE_INTEGER, "2");
    bm->K_max_co_sp = (int)Util_XML_Read_Value(fileName, ATLANTIS_ATTRIBUTE,  bm->ecotest, 1, groupingNode, integer_check, "K_max_co_sp");
    
    if (bm->K_max_co_sp > 0) {
        Create_Species_ParamXML(bm, fileName, fp, groupingNode, max_co_sp_id, "Number of companion species for this fished species", "", XML_TYPE_INTEGER,"1");        

        // Old verion when had fixed limit to the number of fishable spp
        // Parse_File( bm, fp, fileName, groupingNode, "CompanionSpecies", "^co_sp_[A-Z]{2,3}", "Identity of companions in companion TAC - for all entries must have as many entries as for max_co_sp. \nIf there are not enough (or any) companions then for all extra entries enter companion as -1. For those WITH companions entries to the ID numbers of the companion then enter -1 to fill up the rest of the array. \nFor example if max_co_sp = 2 and the companions for FXX were FPS and FVV \nco_sp_FXX    2\n2 4", "", XML_TYPE_INTEGERARRAY, -1, TRUE, Create_Fished_Species_XMLNodes, Fix_Not_Fished_Species_Last_XMLFunction);
        Parse_File( bm, fp, fileName, groupingNode, "CompanionSpecies", "^co_sp_[A-Z]{2,3}", "Identity of companions in companion TAC - for all entries must have as many entries as for max_co_sp. \nIf there are not enough (or any) companions then for all extra entries enter companion as -1. For those WITH companions entries to the ID numbers of the companion then enter -1 to fill up the rest of the array. \nFor example if max_co_sp = 2 and the companions for FXX were FPS and FVV \nco_sp_FXX    2\n2 4", "", XML_TYPE_INTEGERARRAY, -1, TRUE, Create_Fished_Species_XMLNodes, Species_Last_XMLFunction);


        Create_Species_ParamXML(bm, fileName, fp, groupingNode, coType_id, "Type of companion TAC (0 = weakest link dictates quota, 1 = strongest link dictates quota)", "", XML_TYPE_BOOLEAN,"1");
        
        Parse_File(bm, fp, fileName, groupingNode, "co_sp_catch", "^[A-Z]{2,3}_co_sp_catch[1-9]{1,2}","Ratio of catch of companions in companion TAC - for all entries must have as many entries as there are fisheries. \nOne vctor is required per companion species.", "", XML_TYPE_FLOATARRAY, bm->K_num_fisheries, TRUE, Init_Co_Sp_CatchXML, Co_Sp_CatchXMLFunction);

    }


}

/* Create the Seasonal Fishery xml ndoes */
void SeaonalFisheryXML(MSEBoxModel *bm, FILE *fp, char *fileName, xmlDocPtr doc, xmlNodePtr rootnode) {

	xmlNodePtr groupingNode;
	groupingNode = Util_XML_Create_Node(ATLANTIS_ATTRIBUTE_SUB_GROUP, rootnode, "Seasonal_Fishery_Parameters", "Seasonal fishery parameters ", "", "");

	/* not sure this is going to give the correct array length */
	Util_XML_Create_Node_Next_Line(
			fp,
			fileName,
			groupingNode,
			"RegSeason",
			"Whether seasonal closures in effect for that management region (must be as many entries as there are management regions (minimum = 1). \nIf want seasonal closure used in all regions enter 1 for all regions e.g.",
			"", XML_TYPE_BOOLEANARRAY);

	Create_Fishery_ParamXML(bm, fp, fileName, groupingNode, seasonopen_id, "Day of the year season opens (note: start of the year is day 0 NOT day 1)", "d",
			XML_TYPE_INTEGER, "0");
	Create_Fishery_ParamXML(bm, fp, fileName, groupingNode, seasonclose_id, "Day of the year season closes (note: end of the year is day 364 NOT day 365)", "d",
			XML_TYPE_INTEGER, "364");
}

/* Create the Seasonal Fishery xml ndoes */
void RuleBasedFisheryManagementXML(MSEBoxModel *bm, FILE *fp, char *fileName, xmlDocPtr doc, xmlNodePtr rootnode) {

	xmlNodePtr groupingNode;
	groupingNode = Util_XML_Create_Node(ATLANTIS_ATTRIBUTE_SUB_GROUP, rootnode, "Rule_Based_Fishery_Management",
			"Rule based fishery management. Population thresholds when populations fall below threshold values ", "", "");

	Create_Harvest_Fishery_Group_ParamXML(bm, fp, fileName, groupingNode, FC_thresh_id, "Proportion of the population when effort restricted in each fishery.", "",
			XML_TYPE_FLOATARRAY);
	Create_Harvest_Fishery_Group_ParamXML(
			bm,
			fp,
			fileName,
			groupingNode,
			FC_high_thresh_id,
			"Population reference points (proportion of the population) when TAC restriction due to population size completely lifted and original TAC levels are reinstituted",
			"", XML_TYPE_FLOATARRAY);

	Util_XML_Parse_Create_Node(
			fp,
			fileName,
			groupingNode,
			"reinit_pop_day",
			"Day virgin stocks calculated (reference point for stock declines). Normally first timestep of run, but can be set later if populations equilibrate at levels substantially different to initial conditions",
			"d", XML_TYPE_INTEGER, "0");

}

/* This should perhaps be in the HarvestManagement input file */
void RestricturedXML(MSEBoxModel *bm, FILE *fp, char *fileName, xmlDocPtr doc, xmlNodePtr rootnode) {

	xmlNodePtr groupingNode, childGroupingNode;
	groupingNode = Util_XML_Create_Node(ATLANTIS_ATTRIBUTE_SUB_GROUP, rootnode, "Effort_Reduction", "Effort reduction parameters", "", "");

	Create_Fishery_ParamXML(bm, fp, fileName, groupingNode, FC_restrict_id,
			"The following restrictions are applied once target or non-threatened stocks full to threshold levels defined by FC_thresh", "", XML_TYPE_FLOAT, "0.1");
	Create_Fishery_ParamXML(bm, fp, fileName, groupingNode, FC_period_id, "Period of time over which effort reduction occurs", "d", XML_TYPE_INTEGER, "730 - 1825");
	Create_Fishery_ParamXML(bm, fp, fileName, groupingNode, FC_period2_id, "Period of time before check if need further effort reduction", "d", XML_TYPE_INTEGER, "1825");

	childGroupingNode = Util_XML_Create_Node(ATLANTIS_ATTRIBUTE_SUB_GROUP, groupingNode, "Endangered_Restrictions",
			"The following restrictions are applied when threatened stocks full to threshold levels defined by FC_thresh", "", "");
	Create_Fishery_ParamXML(bm, fp, fileName, childGroupingNode, FC_restrict_endangered_id, "Proportion of effort allowed once restrictions begin", "", XML_TYPE_FLOAT,
			"0.1");
	Create_Fishery_ParamXML(bm, fp, fileName, childGroupingNode, FC_endanger_period_id, "Period of time over which effort reduction occurs", "d", XML_TYPE_INTEGER, "730");
}

/* Changes in effort */
void EffortChangedXML(MSEBoxModel *bm, FILE *fp, char *fileName, xmlDocPtr doc, xmlNodePtr rootnode) {

	xmlNodePtr groupingNode, childGroupingNode;
	groupingNode = Util_XML_Create_Node(ATLANTIS_ATTRIBUTE_SUB_GROUP, rootnode, "Effort_Changes",
			"Scenario specification - changing fisheries and management conditions", "", "");

	Create_Fishery_ParamXML(bm, fp, fileName, groupingNode, EFF_num_changes_id, "number of changes in effort", "", XML_TYPE_INTEGER, "1");

	Parse_File(
			bm,
			fp,
			fileName,
			groupingNode,
			"StartChangeDay",
			"^EffortChange_start_",
			"Arrays to indicate when each change in fishing begins (days from run start). There must be as many entries in this array as there are number of changes in effort for that fishery.",
			"d", XML_TYPE_INTEGERARRAY, -1, TRUE, Create_Fishery_XMLNodes, Fishery_Last_XMLFunction);

	Parse_File(
			bm,
			fp,
			fileName,
			groupingNode,
			"StartChangePeriod",
			"^EffortChange_period_",
			"Arrays to indicate time period over which change in fishing occurs (days). There must be as many entries in this array as there are number of changes in effort for that fishery.",
			"d", XML_TYPE_INTEGERARRAY, -1, TRUE, Create_Fishery_XMLNodes, Fishery_Last_XMLFunction);

	Parse_File(
			bm,
			fp,
			fileName,
			groupingNode,
			"EffortChangeProportion",
			"^Effort_mult_",
			"Arrays to indicate proportional change in effort (relative to base mEff entered above) for each change in effort. There must be as many entries in this array as there are number of changes in effort for that fishery.",
			"", XML_TYPE_FLOATARRAY, -1, TRUE, Create_Fishery_XMLNodes, Fishery_Last_XMLFunction);

	Parse_File(bm, fp, fileName, groupingNode, "EffortChangePulseFlag", "^flagpulse",
			"Arrays to indicate whether each change in fishing is a simple pulse or not.1=return to original levels of fishing on FP, 0=remain changed", "",
			XML_TYPE_FLOATARRAY, -1, TRUE, Create_Fishery_XMLNodes, FisheriesEffortChangePulseXML);

	childGroupingNode = Util_XML_Create_Node(ATLANTIS_ATTRIBUTE_SUB_GROUP, groupingNode, "Effort_CAP", "Effort CAP paremeters", "", "");
	Create_Fishery_ParamXML(bm, fp, fileName, childGroupingNode, CAP_num_changes_id, "Change in effort cap through time", "", XML_TYPE_INTEGER, "1");
	Create_Fishery_ChangeXML(bm, fp, fileName, doc, childGroupingNode, "CAPchange", "effort cap", "Effort_CAP");

}

/**
 * changes in management over time
 *
 */

void ManagementChangeXML(MSEBoxModel *bm, FILE *fp, char *fileName, xmlDocPtr doc, xmlNodePtr rootnode) {

	xmlNodePtr groupingNode, childGroupingNode;
	groupingNode = Util_XML_Create_Node(ATLANTIS_ATTRIBUTE_SUB_GROUP, rootnode, "Management_Changes",
			"Scenario specification - changes in management conditions over time.", "", "");

	childGroupingNode = Util_XML_Create_Node(ATLANTIS_ATTRIBUTE_SUB_GROUP, groupingNode, "Population_Changes",
			"Human population size changes (results in changes weight of recreation fishing pressure)", "", "");

	Util_XML_Create_Node_Next_Line(fp, fileName, childGroupingNode, "ports_popchange", "Does population of port change (1=yes, 0=no)", "", XML_TYPE_BOOLEANARRAY);
	Util_XML_Create_Node_Next_Line(fp, fileName, childGroupingNode, "ports_popnumchange",
			"number of changes in population for each port (1 is default as = minimum array size)", "", XML_TYPE_INTEGERARRAY);

	Parse_File(bm, fp, fileName, childGroupingNode, "PortPopulationChangeStart", "^PortPopChange_start_",
			"Start time of any changes in population size for each port. This must have the same number of entries as given by ports_pop", "d", XML_TYPE_INTEGERARRAY,
			-1, TRUE, InitPortChangeXML, PortChangeXML);

	Parse_File(
			bm,
			fp,
			fileName,
			childGroupingNode,
			"PortPopulationChangePeriod",
			"^PortPopChange_period_",
			"Arrays to indicate time period over which change in port population occurs (days). This must have the same number of entries as given by ports_pop",
			"d", XML_TYPE_INTEGERARRAY, -1, TRUE, InitPortChangeXML, PortChangeXML);

	Parse_File(bm, fp, fileName, childGroupingNode, "PortPopulationChangeMult", "^PortPopChange_mult_",
			"Start time of any changes in population size for each port. This must have the same number of entries as given by ports_pop", "", XML_TYPE_FLOATARRAY, -1,
			TRUE, InitPortChangeXML, PortChangeXML);

}

/**
 * Management performance indicators.
 *
 */

void ManagementPerformanceIndicatorsXML(MSEBoxModel *bm, FILE *fp, char *fileName, xmlDocPtr doc, xmlNodePtr rootnode) {

	xmlNodePtr groupingNode;
	groupingNode = Util_XML_Create_Node(ATLANTIS_ATTRIBUTE_SUB_GROUP, rootnode, "Management_Performance_Indicators", "Management performance indicators.", "",
			"");

	Util_XML_Parse_Create_Node(fp, fileName, groupingNode, "enforce_coefft", "Coefficient for  management enforcement performance measure", "", XML_TYPE_FLOAT, "1.0");
	Util_XML_Parse_Create_Node(fp, fileName, groupingNode, "habcoefft", "Coefficient for habitat impacts performance measure", "", XML_TYPE_FLOAT, "1.0");
	Util_XML_Parse_Create_Node(fp, fileName, groupingNode, "tepcoefft", "Coefficient for interactions with TEP groups performance measure", "", XML_TYPE_FLOAT, "1.0");
	Util_XML_Parse_Create_Node(fp, fileName, groupingNode, "sharkcoefft", "Coefficient for interactions with shark groups performance measure", "", XML_TYPE_FLOAT, "1.0");

}

void FisheriesGearConflictXML(MSEBoxModel *bm, char *fileName, xmlNodePtr parent, char *str, char *valueStr) {

	Fisheries_NodeXML(bm, fileName, parent, str, valueStr, "gear_conflict_");
}

/*
 * Fishing gear
 */
void GearConflictXML(MSEBoxModel *bm, FILE *fp, char *fileName, xmlDocPtr doc, xmlNodePtr rootnode) {
	xmlNodePtr groupingNode;
	groupingNode = Util_XML_Create_Node(ATLANTIS_ATTRIBUTE_SUB_GROUP, rootnode, "Gear_Conflict", "Fishing gear conflict parameters.", "", "");

	Parse_File(bm, fp, fileName, groupingNode, "GearConflictBetweenFisheries", "^gear_conflict_", "Vectors indicating which gears conflict", "", XML_TYPE_BOOLEANARRAY,
			bm->K_num_fisheries, TRUE, Create_Fishery_XMLNodes, FisheriesGearConflictXML);

}

/*
 * Fishing Spatial Management
 */
void SpatialManagementXML(MSEBoxModel *bm, FILE *fp, char *fileName, xmlDocPtr doc, xmlNodePtr rootnode) {
	xmlNodePtr groupingNode;
	groupingNode = Util_XML_Create_Node(ATLANTIS_ATTRIBUTE_SUB_GROUP, rootnode, "Spatial_Management", "Fisheries Spatial Management", "", "");

	Create_Fishery_ParamXML(bm, fp, fileName, groupingNode, infringe_id, "Infringement (as proportion of uncontrolled effort)", "", XML_TYPE_FLOAT, "0.0");

	Parse_File(bm,
			fp,
			fileName,
			groupingNode,
			"CellMPAStatus",
			"^MPA",
			"General MPA status of the cell with regard to the specific fishery (proportion of cell open to fishing). 1.0=fishing allowed, 0.0=no fishing allowed, fractions represent some fishing allowed.",
			"", XML_TYPE_FLOATARRAY, bm->nbox, TRUE, Create_Fishery_XMLNodes, CellMPAProportionXML);
	Parse_File(bm,
			fp,
			fileName,
			groupingNode,
			"MPAEndangered",
			"^MPAendangered",
			"MPA's imposed when threatened groups trigger spatial management for the fishery. 1.0=fishing allowed, 0.0=no fishing allowed, fractions represent some fishing allowed.",
			"", XML_TYPE_FLOATARRAY, bm->nbox, TRUE, Create_Fishery_XMLNodes, MPAEndangeredXML);

	Parse_File(bm, fp, fileName, groupingNode, "MPAOverFished", "^MPAoverfished",
			"MPA imposed if a fished judged as overfished has TAC exceeded (like in US style management)", "", XML_TYPE_FLOATARRAY, bm->nbox, TRUE,
			Create_Fished_Species_XMLNodes, MPAoverfishedXML);

}

/* Fisheries contaminant management */
void ContaminantFisheryXML(MSEBoxModel *bm, FILE *fp, char *fileName, xmlDocPtr doc, xmlNodePtr rootnode) {
    int cIndex;
    char varStr[STRLEN*2];
    char longStr[2*STRLEN];

    xmlNodePtr groupingNode;
    groupingNode = Util_XML_Create_Node(ATLANTIS_ATTRIBUTE_SUB_GROUP, rootnode, "Fishery_Contaminant_Parameters", "Parameters for contaminant closures", "", "");

    /* not sure this is going to give the correct array length */
    Util_XML_Create_Node_Next_Line(
            fp,
            fileName,
            groupingNode,
            "ContamClosed",
            "Sites closed due to a contaminant spill",
            "", XML_TYPE_BOOLEANARRAY);

    Util_XML_Parse_Create_Node(fp, fileName, groupingNode, "flag_contam_fisheries_mgmt", "FLag indicating which kind of closure management to use with contaminant spills", "", XML_TYPE_INTEGER, "1");
    Util_XML_Parse_Create_Node(fp, fileName, groupingNode, "contam_fishery_closure_day", "Day closure due to spill begins", "", XML_TYPE_INTEGER, "1");
    Util_XML_Parse_Create_Node(fp, fileName, groupingNode, "contam_fishery_closure_period", "Length of fishery closure due to spill", "", XML_TYPE_INTEGER, "1");
    Util_XML_Parse_Create_Node(fp, fileName, groupingNode, "contam_fishery_closure_option", "Option that defines how the fishery is closed due to a spill", "", XML_TYPE_INTEGER, "1");

    for(cIndex = 0; cIndex < bm->num_contaminants; cIndex++){
        sprintf(varStr, "%s_fishery_thresh_level", bm->contaminantStructure[cIndex]->contaminant_name);
        sprintf(longStr, "Concentration of contaminant %s where fishery closed", bm->contaminantStructure[cIndex]->contaminant_name);
        Util_XML_Parse_Create_Node(fp, fileName, groupingNode, varStr, longStr, "", XML_TYPE_FLOAT, "");
    }

}



void Convert_Management_To_XML(MSEBoxModel *bm, char *fileName, char *outputFileName) {
	xmlDocPtr doc;
	xmlNodePtr rootnode;
	FILE *fp;

	/* Init libxml */
	xmlInitParser();
	LIBXML_TEST_VERSION

	if ((fp = Open_Input_File(bm->inputFolder,fileName, "rt")) == NULL) {
		quit("Cannot open harvest input file %s%s\n", bm->inputFolder, fileName);
	}

	doc = xmlNewDoc((unsigned char *) "1.0");

	if (doc == NULL) {
		quit("Convert_Management_To_XML - Document not created successfully. \n");
	}

	rootnode = xmlNewDocNode(doc, NULL, (const xmlChar*) "AtlantisManagement", NULL);
	xmlDocSetRootElement(doc, rootnode);

	ManamentFlagTimeXML(bm, fp, fileName, doc, rootnode);
	ManagementSenarioFlagsXML(bm, fp, fileName, doc, rootnode);
	ManagementMSYXML(bm, fp, fileName, doc, rootnode);
	ManagementEffortXML(bm, fp, fileName, doc, rootnode);
	PortIndexXML(bm, fp, fileName, doc, rootnode);
	TACXML(bm, fp, fileName, doc, rootnode);

	SeaonalFisheryXML(bm, fp, fileName, doc, rootnode);
	RuleBasedFisheryManagementXML(bm, fp, fileName, doc, rootnode);
	RestricturedXML(bm, fp, fileName, doc, rootnode);
	EffortChangedXML(bm, fp, fileName, doc, rootnode);
	ManagementChangeXML(bm, fp, fileName, doc, rootnode);
	ManagementPerformanceIndicatorsXML(bm, fp, fileName, doc, rootnode);
	SpatialManagementXML(bm, fp, fileName, doc, rootnode);
	GearConflictXML(bm, fp, fileName, doc, rootnode);
    if(bm->track_contaminants) {
        ContaminantFisheryXML(bm, fp, fileName, doc, rootnode);
    }

	// Moved TierAssessmentXML creation from here to assessment file part

	/* Save to the output file */
	xmlSaveFormatFileDestFolder(bm->destFolder, outputFileName, doc, 1);
	xmlFreeDoc(doc);
	fclose(fp);

	/* Shutdown libxml */
	xmlCleanupParser();

	return;
}
