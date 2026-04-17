/*
 * atCLAMLinkIO.c
 *
 *  Created on: 23/11/2009
 *      Author: bec
 *
 *
 *
 *    This file contains functions to read in the XML file for the CLAM linkage module.
 *
 *
 *    Changes:
 *
 *    11-03-2010 Bec Gorton
 *    Added the idealValue for each indicator.
 *
 *    24-03-2010 Bec Gorton
 *    Added code to read in indicator weighting.
 *
 *    06-08-2010 Bec Gorton
 *    Added support for read in a strategy lockout period.
 */

/*
 <CLAMLink>
 <AttributeGroup name = "IndicatorList">
 <Attribute name = "Indicator">
 <attribute name = "tracerName" value = "temperature">
 <attribute name = "minvalue" value = "10">
 <attribute name = "maxvalue" value = "15">
 </Attribute>
 </AttributeGroup>
 </CLAMLink>


 */
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <math.h>
#include <ctype.h>
#include <time.h>
#include <string.h>
#include <sjwlib.h>
#include <atlantisboxmodel.h>
#include <atUtilLib.h>
#include <atCLAMLinkPrivate.h>
#include <atCLAMLink.h>

/**
 * \brief Get the scenario ID from the scenario name.
 */
int Get_CLAM_ScenarioID(char *name) {
	int i;

	printf("numScenarioOptions = %d\n", numScenarioOptions);
	for (i = 0; i < numScenarioOptions; i++) {
		if (strcmp(name, CLAMScenarioOptions[i].scenarioOptionName) == 0)
			return i;
	}
	return -1;
}
/**
 * \brief Get the index of the CLAM indicator with the given name.
 */
int Get_CLAM_IndicatorID(MSEBoxModel *bm, char *name) {
	int i;

	printf("name = %s\n", name);
	for (i = 0; i < numIndicators; i++) {
		printf("indicators[i].CLAMOutputName= %s\n", indicators[i].CLAMOutputName);
		if (strcmp(name, indicators[i].CLAMOutputName) == 0)
			return i;
	}
	return -1;
}

/**
 * \brief Get the index of the CLAM output indicator with the given name.
 */
int Get_CLAM_OutputID(char *name) {
	int i;

	for (i = 0; i < numClamIndicators; i++) {
		if (strcmp(name, CLAMOutputOptions[i].outputOptionName) == 0)
			return i;
	}
	return -1;
}

int Get_Lever_Type(char *name) {
	if (strcmp(name, "PSSChange") == 0)
		return PSS_CHANGE;
	if (strcmp(name, "QChange") == 0)
		return Q_CHANGE;
	if (strcmp(name, "HabitatChange") == 0)
		return HABITAT_CHANGE;
	if (strcmp(name, "LinearMortChange") == 0)
		return LINEAR_MORT_CHANGE;

	quit("Lever name '%s' not recognised\n");
	return -1;
}

/**
 * \bried Get the index of the given scenario name.
 *
 * Used to check that the user is specifying a valid name in the input file.
 *
 */
int Get_Scenario_Index(char *name) {
	int i;

	for (i = 0; i < MAXSCENARIOS; i++) {
		if (strcmp(name, CLAMScenarios[i].scenarioName) == 0)
			return i;
	}
	return -1;
}

int Get_Strategy_Index(int scenarioIndex, char *name) {
	int i;

	for (i = 0; i < maxStrategyOptions; i++) {
		if (strcmp(name, CLAMScenarios[scenarioIndex].strategyNames[i]) == 0)
			return i;
	}
	return -1;
}

static int Read_Lever_List(MSEBoxModel *bm, char *fileName, int scenarioID, xmlNodePtr strategyNode, int strategyID) {

	xmlNodePtr LeverListNode;
	xmlNodePtr leverNode;
	xmlXPathObjectPtr leverList;
	int numLevers, leverID;
	char cohortStr[50];

	/* Strategy Levers */
	LeverListNode = Util_XML_Get_Node(ATLANTIS_ATTRIBUTE_SUB_GROUP, strategyNode, "LeverList");
	if (LeverListNode == NULL)
		quit("Read_CLAM_Link_Params: LeverList attribute group not found.\n");

	leverList = Util_XML_Get_Node_List(ATLANTIS_ATTRIBUTE_SUB_GROUP, LeverListNode, "Lever");

	if (leverList && leverList->nodesetval->nodeNr > 0) {
		numLevers = leverList->nodesetval->nodeNr;

		printf("numLevers = %d\n", numLevers);
		CLAMScenarioOptions[scenarioID].strategies[strategyID].numLevers = numLevers;

		/* read in the information about each scenario */
		for (leverID = 0; leverID < numLevers; leverID++) {

			leverNode = leverList->nodesetval->nodeTab[leverID];

			Util_XML_Get_Value_String(fileName, ATLANTIS_ATTRIBUTE, TRUE, leverNode, "LeverName",
					CLAMScenarioOptions[scenarioID].strategies[strategyID].strategyLevers[leverID].leverName);
			CLAMScenarioOptions[scenarioID].strategies[strategyID].strategyLevers[leverID].leverType = (CLAMLevers) Get_Lever_Type(
					CLAMScenarioOptions[scenarioID].strategies[strategyID].strategyLevers[leverID].leverName);

			printf("lever.leverType  = %d\n", CLAMScenarioOptions[scenarioID].strategies[strategyID].strategyLevers[leverID].leverType);

			/* read in the attributes that are common to all lever types*/
			Util_XML_Get_Value_Double(fileName, ATLANTIS_ATTRIBUTE, 0, TRUE, leverNode, no_checking, "Start",
					&CLAMScenarioOptions[scenarioID].strategies[strategyID].strategyLevers[leverID].start);
			Util_XML_Get_Value_Double(fileName, ATLANTIS_ATTRIBUTE, 0, TRUE, leverNode, no_checking, "Period",
					&CLAMScenarioOptions[scenarioID].strategies[strategyID].strategyLevers[leverID].period);
			Util_XML_Get_Value_Double(fileName, ATLANTIS_ATTRIBUTE, 0, TRUE, leverNode, no_checking, "Mult",
					&CLAMScenarioOptions[scenarioID].strategies[strategyID].strategyLevers[leverID].mult);

			/* Now read in the attributes that are unique to the lever type */
			switch (CLAMScenarioOptions[scenarioID].strategies[strategyID].strategyLevers[leverID].leverType) {
			case PSS_CHANGE:
				Util_XML_Get_Value_String(fileName, ATLANTIS_ATTRIBUTE, TRUE, leverNode, "TargetName",
						CLAMScenarioOptions[scenarioID].strategies[strategyID].strategyLevers[leverID].targetName);

				Util_XML_Get_Value_Integer(fileName, ATLANTIS_ATTRIBUTE, 0, TRUE, leverNode, integer_check, "Location",
						&CLAMScenarioOptions[scenarioID].strategies[strategyID].strategyLevers[leverID].location);
				break;
			case HABITAT_CHANGE:
				Util_XML_Get_Value_String(fileName, ATLANTIS_ATTRIBUTE, TRUE, leverNode, "TargetName",
						CLAMScenarioOptions[scenarioID].strategies[strategyID].strategyLevers[leverID].targetName);

				/* Read in the box information */
				Util_XML_Get_Value_Integer(fileName, ATLANTIS_ATTRIBUTE, 0, TRUE, leverNode, integer_check, "Location",
						&CLAMScenarioOptions[scenarioID].strategies[strategyID].strategyLevers[leverID].location);
				break;
			case LINEAR_MORT_CHANGE:

				/* Read in the box information */
				Util_XML_Get_Value_Integer(fileName, ATLANTIS_ATTRIBUTE, 0, TRUE, leverNode, integer_check, "Location",
						&CLAMScenarioOptions[scenarioID].strategies[strategyID].strategyLevers[leverID].location);

				Util_XML_Get_Value_String(fileName, ATLANTIS_ATTRIBUTE, TRUE, leverNode, "TargetName",
						CLAMScenarioOptions[scenarioID].strategies[strategyID].strategyLevers[leverID].targetName);

				/* Now set the groupCode value */
				CLAMScenarioOptions[scenarioID].strategies[strategyID].strategyLevers[leverID].groupCode = Util_Get_FG_Index(bm,
						CLAMScenarioOptions[scenarioID].strategies[strategyID].strategyLevers[leverID].targetName);

				if (CLAMScenarioOptions[scenarioID].strategies[strategyID].strategyLevers[leverID].groupCode == -1)
					quit("Read_Lever_List - Reading in the CLAM-Atlantis input file. SpeciesName '%s' is not recognised\n",
							CLAMScenarioOptions[scenarioID].strategies[strategyID].strategyLevers[leverID].targetName);
				/* Read in the cohort information */
				Util_XML_Get_Value_String(fileName, ATLANTIS_ATTRIBUTE, TRUE, leverNode, "Cohort", cohortStr);
				if (strcmp(cohortStr, "Adult") == 0)
					CLAMScenarioOptions[scenarioID].strategies[strategyID].strategyLevers[leverID].cohort = adult_id;
				else if (strcmp(cohortStr, "Juv") == 0)
					CLAMScenarioOptions[scenarioID].strategies[strategyID].strategyLevers[leverID].cohort = juv_id;
				else
					quit("Read_Lever_List - Cohort string %s is not recognised. Should be either 'Adult' or 'Juv'\n", cohortStr);

				break;
			case Q_CHANGE:
				Util_XML_Get_Value_String(fileName, ATLANTIS_ATTRIBUTE, TRUE, leverNode, "TargetName",
						CLAMScenarioOptions[scenarioID].strategies[strategyID].strategyLevers[leverID].targetName);

				/* Now set the groupCode value */
				CLAMScenarioOptions[scenarioID].strategies[strategyID].strategyLevers[leverID].groupCode = Util_Get_FG_Index(bm,
						CLAMScenarioOptions[scenarioID].strategies[strategyID].strategyLevers[leverID].targetName);

				break;
			}
		}
	}
	xmlXPathFreeObject(leverList);
	return TRUE;
}
//
//static int Read_Lever_List(MSEBoxModel *bm, char *fileName, int scenarioID, xmlNodePtr strategyNode, int strategyID) {
//
//	xmlNodePtr LeverListNode;
//	xmlNodePtr leverNode;
//	xmlXPathObjectPtr leverList;
//	int numLevers, leverID;
//	char cohortStr[50];
//
//	/* Strategy Levers */
//	LeverListNode = Util_XML_Get_Node(ATLANTIS_ATTRIBUTE_SUB_GROUP, strategyNode, "LeverList");
//	if (LeverListNode == NULL)
//		quit("Read_CLAM_Link_Params: LeverList attribute group not found in input file %s.\n", fileName);
//
//	leverList = Util_XML_Get_Node_List(ATLANTIS_ATTRIBUTE_SUB_GROUP, LeverListNode, "Lever");
//
//	if (leverList && leverList->nodesetval->nodeNr > 0) {
//		numLevers = leverList->nodesetval->nodeNr;
//
//		printf("numLevers = %d\n", numLevers);
//		CLAMScenarioOptions[scenarioID].strategies[strategyID].numLevers = numLevers;
//
//		/* read in the information about each scenario */
//		for (leverID = 0; leverID < numLevers; leverID++) {
//
//			leverNode = leverList->nodesetval->nodeTab[leverID];
//
//			Util_XML_Get_Value_String(fileName, ATLANTIS_ATTRIBUTE, TRUE, leverNode, "LeverName", CLAMScenarioOptions[scenarioID].strategyLevers[strategyID][leverID].leverName);
//			CLAMScenarioOptions[scenarioID].strategyLevers[strategyID][leverID].leverType = (CLAMLevers)Get_Lever_Type(
//					CLAMScenarioOptions[scenarioID].strategyLevers[strategyID][leverID].leverName);
//
//			printf("lever.leverType  = %d\n", CLAMScenarioOptions[scenarioID].strategyLevers[strategyID][leverID].leverType);
//
//			/* read in the attributes that are common to all lever types*/
//			Util_XML_Get_Value_Double(fileName,ATLANTIS_ATTRIBUTE, 0, TRUE, leverNode, no_checking, "Start",
//					&CLAMScenarioOptions[scenarioID].strategyLevers[strategyID][leverID].start);
//			Util_XML_Get_Value_Double(fileName,ATLANTIS_ATTRIBUTE, 0, TRUE, leverNode, no_checking, "Period",
//					&CLAMScenarioOptions[scenarioID].strategyLevers[strategyID][leverID].period);
//			Util_XML_Get_Value_Double(fileName,ATLANTIS_ATTRIBUTE, 0, TRUE, leverNode, no_checking, "Mult",
//					&CLAMScenarioOptions[scenarioID].strategyLevers[strategyID][leverID].mult);
//
//			/* Now read in the attributes that are unique to the lever type */
//			switch (CLAMScenarioOptions[scenarioID].strategyLevers[strategyID][leverID].leverType) {
//			case PSS_CHANGE:
//				Util_XML_Get_Value_String(fileName, ATLANTIS_ATTRIBUTE, TRUE, leverNode, "TargetName",
//						CLAMScenarioOptions[scenarioID].strategyLevers[strategyID][leverID].targetName);
//
//				Util_XML_Get_Value_Integer(fileName, ATLANTIS_ATTRIBUTE, 0, TRUE, leverNode, integer_check, "Location",
//						&CLAMScenarioOptions[scenarioID].strategyLevers[strategyID][leverID].location);
//				break;
//			case HABITAT_CHANGE:
//				Util_XML_Get_Value_String(fileName, ATLANTIS_ATTRIBUTE, TRUE, leverNode, "TargetName",
//						CLAMScenarioOptions[scenarioID].strategyLevers[strategyID][leverID].targetName);
//
//				/* Read in the box information */
//				Util_XML_Get_Value_Integer(fileName, ATLANTIS_ATTRIBUTE, 0, TRUE, leverNode, integer_check, "Location",
//						&CLAMScenarioOptions[scenarioID].strategyLevers[strategyID][leverID].location);
//				break;
//			case LINEAR_MORT_CHANGE:
//
//				/* Read in the box information */
//				Util_XML_Get_Value_Integer(fileName, ATLANTIS_ATTRIBUTE, 0, TRUE, leverNode, integer_check, "Location",
//						&CLAMScenarioOptions[scenarioID].strategyLevers[strategyID][leverID].location);
//
//				Util_XML_Get_Value_String(fileName, ATLANTIS_ATTRIBUTE, TRUE, leverNode, "TargetName",
//						CLAMScenarioOptions[scenarioID].strategyLevers[strategyID][leverID].targetName);
//
//				/* Now set the groupCode value */
//				CLAMScenarioOptions[scenarioID].strategyLevers[strategyID][leverID].groupCode = Util_Get_FG_Index(bm,
//						CLAMScenarioOptions[scenarioID].strategyLevers[strategyID][leverID].targetName);
//
//				if(CLAMScenarioOptions[scenarioID].strategyLevers[strategyID][leverID].groupCode == -1)
//					quit("Read_Lever_List - Reading in the CLAM-Atlantis input file. SpeciesName '%s' is not recognised\n",
//							CLAMScenarioOptions[scenarioID].strategyLevers[strategyID][leverID].targetName);
//				/* Read in the cohort information */
//				Util_XML_Get_Value_String(fileName, ATLANTIS_ATTRIBUTE, TRUE, leverNode, "Cohort", cohortStr);
//				if (strcmp(cohortStr, "Adult") == 0)
//					CLAMScenarioOptions[scenarioID].strategyLevers[strategyID][leverID].cohort = adult_id;
//				else if (strcmp(cohortStr, "Juv") == 0)
//					CLAMScenarioOptions[scenarioID].strategyLevers[strategyID][leverID].cohort = juv_id;
//				else
//					quit("Read_Lever_List - Cohort string %s is not recognised. Should be either 'Adult' or 'Juv'\n", cohortStr);
//
//				break;
//			case Q_CHANGE:
//				Util_XML_Get_Value_String(fileName, ATLANTIS_ATTRIBUTE, TRUE, leverNode, "TargetName",
//						CLAMScenarioOptions[scenarioID].strategyLevers[strategyID][leverID].targetName);
//
//				/* Now set the groupCode value */
//				CLAMScenarioOptions[scenarioID].strategyLevers[strategyID][leverID].groupCode = Util_Get_FG_Index(bm,
//						CLAMScenarioOptions[scenarioID].strategyLevers[strategyID][leverID].targetName);
//
//				break;
//			}
//		}
//	}
//	xmlXPathFreeObject(leverList);
//	return TRUE;
//}
/**
 *  /brief Initialise the CLAM and Atlantis linking module.
 *
 *  This function sets the paths and allocates any memory required.
 *  All of these values are hard coded for now but eventually the would be read in from an input file.
 *
 *  This needs additional error checking.
 */
int Read_CLAM_Link_Params(MSEBoxModel *bm, char *fileName) {

	xmlNodePtr IndicatorListNode, SetupGroupNode, indicatorNode, node, ScenarioListNode, scenarioNode;
	xmlNodePtr StrategyListNode, strategyNode;
	xmlDocPtr inputDoc;
	int indicatorID = 0, linkID = 0, scenarioID;
	char *scenarioName;
	xmlXPathObjectPtr indicatorList, scenarioList, strategyList;
	xmlNodePtr indicatorScenarioList;
	int numDays, index;
	char strategyName[STRLEN];
	int numStrategyOptions;

	/* Force this for now - will want it to be an input param eventually */
	considerAllIndicators = TRUE;

	if (verbose)
		printf("Read_CLAM_Link_Params\n");

	inputDoc = xmlReadFileDestFolder(bm->destFolder, fileName, NULL, 0);
	if (inputDoc == NULL) {
		quit("failed to read the CLAM link input files '%s'\n", fileName);
	}

	/* Read in the setup params */
	SetupGroupNode = Util_XML_Get_Node(ATLANTIS_ATTRIBUTE_GROUP, inputDoc->children, "Setup");
	if (SetupGroupNode == NULL)
		quit("Read_CLAM_Link_Params: SetupGroupNode attribute group not found in input file %s.\n", fileName);

	Util_XML_Get_Value_String(fileName, ATLANTIS_ATTRIBUTE, TRUE, SetupGroupNode, "outputFolderName", outputFolderPath);
	Util_XML_Get_Value_String(fileName, ATLANTIS_ATTRIBUTE, TRUE, SetupGroupNode, "outputFileName", outputFileName);
	Util_XML_Get_Value_String(fileName, ATLANTIS_ATTRIBUTE, TRUE, SetupGroupNode, "inputFileName", inputFileName);
	Util_XML_Get_Value_String(fileName, ATLANTIS_ATTRIBUTE, TRUE, SetupGroupNode, "iniFileName", iniFileName);
	Util_XML_Get_Value_String(fileName, ATLANTIS_ATTRIBUTE, TRUE, SetupGroupNode, "startedFileName", startedFileName);
	Util_XML_Get_Value_String(fileName, ATLANTIS_ATTRIBUTE, TRUE, SetupGroupNode, "errorFileName", errorFileName);
	Util_XML_Get_Value_String(fileName, ATLANTIS_ATTRIBUTE, TRUE, SetupGroupNode, "finishedFileName", finishedFileName);

	Util_XML_Get_Value_String(fileName, ATLANTIS_ATTRIBUTE, TRUE, SetupGroupNode, "pathToICMSBuilder", pathToICMSBuilder);
	Util_XML_Get_Value_String(fileName, ATLANTIS_ATTRIBUTE, TRUE, SetupGroupNode, "ICMSProject", ICMSProject);
	Util_XML_Get_Value_String(fileName, ATLANTIS_ATTRIBUTE, TRUE, SetupGroupNode, "ICMSDLL", ICMSDLL);

	Util_XML_Get_Value_Double(fileName, ATLANTIS_ATTRIBUTE, 0, TRUE, SetupGroupNode, no_checking, "startDay", &CLAMStartDay);
	Util_XML_Get_Value_Double(fileName, ATLANTIS_ATTRIBUTE, 0, TRUE, SetupGroupNode, no_checking, "freq", &CLAMFreq);
	Util_XML_Get_Value_Integer(fileName, ATLANTIS_ATTRIBUTE, 0, TRUE, SetupGroupNode, no_checking, "ManagerType", (int *) &CLAMManagerType);

	/* Read in the strategy information */

	/* Read in each of the scenarios */
	ScenarioListNode = Util_XML_Get_Node(ATLANTIS_ATTRIBUTE_GROUP, inputDoc->children, "ScenarioList");
	if (ScenarioListNode == NULL)
		quit("Read_CLAM_Link_Params: ScenarioListNode attribute group not found in input file %s.\n", fileName);

	scenarioList = Util_XML_Get_Node_List(ATLANTIS_ATTRIBUTE_SUB_GROUP, ScenarioListNode, "Scenario");

	if (scenarioList && scenarioList->nodesetval->nodeNr > 0) {
		if (bm->debug == clam_debug)
			printf("DEBUG: Reading in %d CLAM scenarios.\n", scenarioList->nodesetval->nodeNr);

		numScenarioOptions = scenarioList->nodesetval->nodeNr;
		CLAMScenarioOptions = malloc(sizeof(CLAMScenarioOptionStructure) * numScenarioOptions);

		printf("numScenarios  %d\n", numScenarioOptions);

		/* Scenarios */
		for (scenarioID = 0; scenarioID < numScenarioOptions; scenarioID++) {

			scenarioNode = scenarioList->nodesetval->nodeTab[scenarioID];

			Util_XML_Get_Value_String(fileName, ATLANTIS_ATTRIBUTE, TRUE, scenarioNode, "ScenarioName", CLAMScenarioOptions[scenarioID].scenarioOptionName);

			/* Check that this scenario is recognised */
			CLAMScenarioOptions[scenarioID].clamScenarioIndex = Get_Scenario_Index(CLAMScenarioOptions[scenarioID].scenarioOptionName);

			if (CLAMScenarioOptions[scenarioID].clamScenarioIndex < 0) {
				quit("Read_CLAM_Link_Params: Scenario %s is not recognised\n", CLAMScenarioOptions[scenarioID].scenarioOptionName);
			}
			Util_XML_Get_Value_Integer(fileName, ATLANTIS_ATTRIBUTE, 0, TRUE, scenarioNode, integer_check, "maxOptionValue",
					&CLAMScenarioOptions[scenarioID].maxOptionValue);

			/* Read in each of the strategies associated with this scenario */
			StrategyListNode = Util_XML_Get_Node(ATLANTIS_ATTRIBUTE_SUB_GROUP, scenarioNode, "StrategyList");
			if (StrategyListNode == NULL)
				quit("Read_CLAM_Link_Params: StrategyListNode attribute group not found in input file %s.\n", fileName);

			strategyList = Util_XML_Get_Node_List(ATLANTIS_ATTRIBUTE_SUB_GROUP, StrategyListNode, "Strategy");

			if (strategyList && strategyList->nodesetval->nodeNr > 0) {
				numStrategyOptions = strategyList->nodesetval->nodeNr;

				/* Strategies */
				for (index = 0; index < numStrategyOptions; index++) {

					strategyNode = strategyList->nodesetval->nodeTab[index];

					Util_XML_Get_Value_String(fileName, ATLANTIS_ATTRIBUTE, TRUE, strategyNode, "StrategyName", strategyName);

					CLAMScenarioOptions[scenarioID].strategies[index].CLAMStrategyID = Get_Strategy_Index(CLAMScenarioOptions[scenarioID].clamScenarioIndex,
							strategyName);

					/* Get the strategy index from the list */
					if (CLAMScenarioOptions[scenarioID].strategies[index].CLAMStrategyID < 0) {
						quit("Read_CLAM_Link_Params: Strategy %s is not recognised\n", strategyName);
					}

					strcpy(CLAMScenarioOptions[scenarioID].strategies[index].strategyName, strategyName);

					/* Read in the enable value */
					Util_XML_Get_Value_Integer(fileName, ATLANTIS_ATTRIBUTE, 0, TRUE, strategyNode, integer_check, "IsEnabled",
							&CLAMScenarioOptions[scenarioID].strategies[index].isEnabled);
					printf("CLAMScenarioOptions[scenarioID].strategies[index].isEnabled = %d\n", CLAMScenarioOptions[scenarioID].strategies[index].isEnabled);

					/* readin the lock out period - user will have specified in days but we want seconds */
					Util_XML_Get_Value_Integer(fileName, ATLANTIS_ATTRIBUTE, 0, TRUE, strategyNode, integer_check, "StrategyLockoutPeriod", &numDays);
					CLAMScenarioOptions[scenarioID].strategies[index].lockoutPeriod = numDays * 24 * 60 * 60;

					Util_XML_Get_Value_Double(fileName, ATLANTIS_ATTRIBUTE, 0, TRUE, strategyNode, proportion_check, "StrategyCost",
							&CLAMScenarioOptions[scenarioID].strategies[index].strategyCost);

					printf("StrategyName = %s\n", CLAMScenarioOptions[scenarioID].strategies[index].strategyName);
					/* Strategy Levers */
					Read_Lever_List(bm, fileName, scenarioID, strategyNode, index);
				}
			}
		}
	}

	/* Read in each of the indicators */
	IndicatorListNode = Util_XML_Get_Node(ATLANTIS_ATTRIBUTE_GROUP, inputDoc->children, "IndicatorList");
	if (IndicatorListNode == NULL)
		quit("IndicatorList: IndicatorList attribute group not found in input file %s.\n", fileName);

	indicatorList = Util_XML_Get_Node_List(ATLANTIS_ATTRIBUTE_SUB_GROUP, IndicatorListNode, "Indicator");

	printf("indicatorList->nodesetval->nodeNr = %d\n", indicatorList->nodesetval->nodeNr);
	if (indicatorList && indicatorList->nodesetval->nodeNr > 0) {
		if (bm->debug == clam_debug)
			printf("DEBUG: Reading in %d CLAM indicators.\n", indicatorList->nodesetval->nodeNr);

		numIndicators = indicatorList->nodesetval->nodeNr;
		indicators = (atCLAMIndicator *) malloc(sizeof(atCLAMIndicator) * numIndicators);

		printf("numIndicators  %d\n", numIndicators);

		/* Process each node in turn.  Note that because order doesn't
		 * matter in the mapping array, we can just re-use
		 * groupMapping index to index into the mapping array as well
		 * as the xpath results array.  We also initialise ingroup,
		 * which is the number of groups that are imported, based on
		 * the configured mapping /destinations/, which is more robust
		 * than the user configuring it. */
		for (indicatorID = 0; indicatorID < numIndicators; indicatorID++) {

			indicatorNode = indicatorList->nodesetval->nodeTab[indicatorID];

			/* get the min and max values */
			Util_XML_Get_Value_String(fileName, ATLANTIS_ATTRIBUTE, TRUE, indicatorNode, "CLAMOutputName", indicators[indicatorID].CLAMOutputName);

			Util_XML_Get_Value_Double(fileName, ATLANTIS_ATTRIBUTE, 0, TRUE, indicatorNode, no_checking, "MinValue", &indicators[indicatorID].minValue);
			Util_XML_Get_Value_Double(fileName, ATLANTIS_ATTRIBUTE, 0, TRUE, indicatorNode, no_checking, "MaxValue", &indicators[indicatorID].maxValue);
			Util_XML_Get_Value_Double(fileName, ATLANTIS_ATTRIBUTE, 0, TRUE, indicatorNode, no_checking, "IdealValue", &indicators[indicatorID].idealValue);
			Util_XML_Get_Value_Double(fileName, ATLANTIS_ATTRIBUTE, 0, TRUE, indicatorNode, no_checking, "IndicatorWeighting",
					&indicators[indicatorID].indicatorWeighting);

			/* Link this CLAMoutputName to the values in the CLAMOutputOptions values */
			indicators[indicatorID].CLAMOutputIndex = Get_CLAM_OutputID(indicators[indicatorID].CLAMOutputName);

			if(indicators[indicatorID].CLAMOutputIndex < 0){
				quit("Read_CLAM_Link_Params: The CLAMOutputName %s is not yet supported. We may need to add support to the assessment module to support this.\n", indicators[indicatorID].CLAMOutputName);
			}

			fprintf(bm->logFile, "name = %s, index - %d\n", indicators[indicatorID].CLAMOutputName, indicators[indicatorID].CLAMOutputIndex);

			indicatorScenarioList = Util_XML_Get_Node(ATLANTIS_ATTRIBUTE_SUB_GROUP, indicatorNode, "ScenarioLinks");

			if (indicatorScenarioList == NULL) {
				quit("Read_CLAM_Link_Params: Cannot find the ScenarioLinks nodes.\n");
			}
			printf("indicatorScenarioList = %s", indicatorScenarioList->name);

			node = indicatorScenarioList->children;
			indicators[indicatorID].numScenarioLinks = 0;

			/* For each child */
			while (node != NULL) {

				/* Skip any text nodes */
				if (node->type == XML_TEXT_NODE || node->type == XML_COMMENT_NODE) {
					node = node->next;
					continue;
				}

				if (node == NULL)
					quit("Read_CLAM_Link_Params: Error parsing the ScenarioLinks nodes. One of the nodes does not have a ScenarioName node\n");
				scenarioName = Util_XML_Get_Node_Value_String(ATLANTIS_ATTRIBUTE, node);

				printf("scenarioName = %s\n", scenarioName);
				/* find the scenario index */
				scenarioID = Get_CLAM_ScenarioID(scenarioName);
				if (scenarioID == -1)
					quit("Read_CLAM_Link_Params: Could not match the CLAM scenario %s with any known scenarios in this model\n", scenarioName);

				/* Check that the scenario is in our list of ones we know how to implement in thsi run */
				fprintf(bm->logFile, "scenarioName  = %s, scenarioID = %d\n", scenarioName, scenarioID);

				/* Store the index of the scenario for later use */
				indicators[indicatorID].scenarioLinks[linkID] = scenarioID;
				node = node->next;
				indicators[indicatorID].numScenarioLinks++;
				free(scenarioName);
			}
		}
	}

	/* Build the command line string */
	snprintf(commandLinesStr, sizeof(commandLinesStr), "%s \"%s\" %s", pathToICMSBuilder, ICMSProject, ICMSDLL);

	printf("commandLinesStr = %s\n", commandLinesStr);
	/* Set the wait times */
	processWaitTime = 1000 * 60 * 2;
	startedWaitTime = 1000 * 60 * 2;

	/* Clear the output folder before the CLAM model is called */
	//    snprintf(str, sizeof(str), "del /Q %s\\*.*", outputFolderPath);
	//    printf("str = %s\n", str);
	//    system(str);

	xmlXPathFreeObject(indicatorList);
	xmlXPathFreeObject(scenarioList);
	xmlXPathFreeObject(strategyList);
	xmlFreeDoc(inputDoc);
	xmlCleanupParser();

	/* Work out the start day */
	bm->clamDay = CLAMStartDay;

	return TRUE;
}

