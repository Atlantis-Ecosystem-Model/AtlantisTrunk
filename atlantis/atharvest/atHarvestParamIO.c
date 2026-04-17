/**
 * \ingroup atHarvestLib
 *	\file
 *	\brief Functions to load the harvest parameters from the harvest.xml input document.
 *	Created: 30-06-2009 Bec Gorton CSIRO 2009
 *
 *
 *	<b>Revisions</b>
 *	14-07-2009 Bec Gorton
 *	Added the readFishingFlagValues, readFisheryCatchTimeXML and readFisheryScenarioFlags functions.
 *
 *	28-10-2009 Bec Gorton
 *	Finished the code to read in the harvest parameters from the new XML file. Will need to streamline code
 *	once i have finsihed updating the other modules. There might be a number of functions that are only used a single time.
 *
 */

/*  Open library routines */
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>
#include <atHarvest.h>
#include <atManageLib.h>

/**
 * Variables that are global to the atHarvest Lib. See atHarvest.h for the full list.
 */
double KDEP_fishery; /* Depth fishery gear penetrates into sediment */

/**
 * Private variables.
 *
 */
static int knife_curve;

/**
 * will only read in values for the impacted and age structured groups.
 *
 *
 */
void readAgeStructuredBiomassFisheryParamXML(MSEBoxModel *bm, char *fileName, xmlNodePtr parent, int paramID) {
	int guild, b;
	xmlNodePtr attributeGroup;
	int index;
	double *values = 0;
	char errorString[STRLEN];
	char *nodeName;

	nodeName =  Util_Get_Node_Name(parent);

	index = Util_XML_Get_Fishery_Param_Index(FisheryGroupParamsArray, numGroupFisheryParams, paramID);

	snprintf(errorString, sizeof(errorString), "%s/%s", nodeName, FisheryGroupParamsArray[index].tag);
	attributeGroup = Util_XML_Get_Node(ATLANTIS_ATTRIBUTE, parent, FisheryGroupParamsArray[index].tag);
	if (attributeGroup == NULL)
		quit("readAgeStructuredBiomassFisheryParamXML - %s attribute group not found.\n", errorString);

	if (verbose)
		printf("Read fisheries groups parameters %s\n", FisheryGroupParamsArray[index].tag);

	for (guild = 0; guild < bm->K_num_tot_sp; guild++) {
			if (FunctGroupArray[guild].isFished == TRUE && (FunctGroupArray[guild].groupAgeType == AGE_STRUCTURED || FunctGroupArray[guild].groupAgeType
							== AGE_STRUCTURED_BIOMASS)) {

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

/**
 * will only read in values for the impacted and age structured groups.
 *
 *
 */
void readAgeStructuredGroupFisheryParamXML(MSEBoxModel *bm, char *fileName, xmlNodePtr parent, int paramID) {
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
		quit("readAgeStructuredGroupFisheryParamXML - %s attribute group not found.\n", errorString);

	if (verbose)
		printf("Read fisheries groups parameters %s\n", FisheryGroupParamsArray[index].tag);

	for (guild = 0; guild < bm->K_num_tot_sp; guild++) {
		if (FunctGroupArray[guild].isVertebrate == TRUE){

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

void readFisheryAgeDistributionXML(MSEBoxModel *bm, char *fileName, xmlNodePtr parent, char *paramName, int checkType, double ***arrayPtr) {
	int guild, fisheryIndex, cohort;
	xmlNodePtr attributeGroup;
	double *values = 0;
	char errorString[STRLEN];
	char *nodeName;

	nodeName =  Util_Get_Node_Name(parent);

	snprintf(errorString, sizeof(errorString), "%s/%s", nodeName, paramName);
	attributeGroup = Util_XML_Get_Node(ATLANTIS_ATTRIBUTE, parent, paramName);
	if (attributeGroup == NULL)
		quit("readFisheryAgeDistributionXML - %s attribute group not found.\n", errorString);

	if (verbose)
		printf("Read fisheries groups parameters %s\n", paramName);

	for (guild = 0; guild < bm->K_num_tot_sp; guild++) {
		//if((int)FunctGroupArray[guild].speciesParams[flag_id] == TRUE && FunctGroupArray[guild].isFished == TRUE){
			if((int)FunctGroupArray[guild].speciesParams[flag_id] == TRUE && FunctGroupArray[guild].isImpacted == TRUE){

			if (FunctGroupArray[guild].groupAgeType == AGE_STRUCTURED_BIOMASS || FunctGroupArray[guild].groupAgeType == AGE_STRUCTURED) {

				if (Util_XML_Read_Array_Double(ATLANTIS_GROUP_ATTRIBUTE, fileName, errorString, attributeGroup, checkType, FunctGroupArray[guild].groupCode, &values,
						FunctGroupArray[guild].numCohorts) == FALSE) {
					quit("Error: Unable to find parameter '%s/%s' in input file %s\n", errorString, FunctGroupArray[guild].groupCode,
							fileName);
				}

				for (fisheryIndex = 0; fisheryIndex < bm->K_num_fisheries; fisheryIndex++){
					// Not using "* FunctGroupArray[guild].numGeneTypes" here as harvest code refers back to age classes only for this parameter 
					for (cohort = 0; cohort < FunctGroupArray[guild].numCohorts; cohort++) {
						arrayPtr[fisheryIndex][guild][cohort] = values[cohort];
					}
				}

				free(values);
			}
		}
	}
	free(nodeName);
}

void readGroupAgeGroupFisheryParamXML(MSEBoxModel *bm, char *fileName, xmlNodePtr parent, int checkType, char *paramName, double ***targetArray) {
	int guild;
	xmlNodePtr attributeGroup, speciesNode;
	double *values = 0;
	int b, cohort;
	char errorString[STRLEN];
	char *nodeName;

	nodeName =  Util_Get_Node_Name(parent);

	attributeGroup = Util_XML_Get_Node(ATLANTIS_ATTRIBUTE, parent, paramName);
	if (attributeGroup == NULL)
		quit("readGroupAgeGroupFisheryParamXML - %s/%s attribute group not found.\n", nodeName, paramName);

	if (verbose > 1)
		printf("Read fisheries groups parameters %s\n", paramName);

	for (guild = 0; guild < bm->K_num_tot_sp; guild++) {
		if(FunctGroupArray[guild].isFished == TRUE){
			if (FunctGroupArray[guild].groupAgeType == AGE_STRUCTURED || FunctGroupArray[guild].groupAgeType == AGE_STRUCTURED_BIOMASS) {

				snprintf(errorString, sizeof(errorString), "%s/%s", paramName, FunctGroupArray[guild].groupCode);

				speciesNode = Util_XML_Get_Node(ATLANTIS_GROUP_ATTRIBUTE, attributeGroup, FunctGroupArray[guild].groupCode);
				if (speciesNode == NULL)
					quit("readGroupAgeGroupFisheryParamXML - %s attribute group not found.\n", errorString);


				for (cohort = 0; cohort < FunctGroupArray[guild].numStages; cohort++) {
					if (Util_XML_Read_Array_Double(ATLANTIS_AGE_CLASS_ATTRIBUTE, fileName, errorString, speciesNode, checkType, cohortStrings[cohort], &values,
							bm->K_num_fisheries) == FALSE) {
						quit("Error: Unable to find parameter '%s/%s' in input file %s\n", errorString, FunctGroupArray[guild].groupCode,
								fileName);
					}
					for (b = 0; b < bm->K_num_fisheries; b++) {
						targetArray[guild][b][cohort] = values[b];
					}
					free(values);
				}
			}
		}
	}
	free(nodeName);
}

void readAgeGroupConstantDiscardingXML(MSEBoxModel *bm, char *fileName, xmlNodePtr parent, char *paramName) {
	int guild, basecohort;
	xmlNodePtr attributeGroup;
	double *values = 0;
	int b, cohort;
	int isA = strcmp(paramName, "AgeGroupConstantDiscardingA");
	char errorString[STRLEN];
	char *nodeName;

	nodeName =  Util_Get_Node_Name(parent);

	snprintf(errorString, sizeof(errorString), "%s/%s", nodeName, paramName);

	attributeGroup = Util_XML_Get_Node(ATLANTIS_ATTRIBUTE, parent, paramName);
	if (attributeGroup == NULL)
		quit("readAgeGroupConstantDiscardingXML - %s attribute group not found.\n", errorString);

	if (verbose > 1)
		printf("Read fisheries groups parameters %s\n", paramName);

	for (guild = 0; guild < bm->K_num_tot_sp; guild++) {
		if (FunctGroupArray[guild].groupAgeType == AGE_STRUCTURED) {

			if (Util_XML_Read_Array_Double(ATLANTIS_GROUP_ATTRIBUTE, fileName, errorString, attributeGroup, no_checking, FunctGroupArray[guild].groupCode, &values,
					FunctGroupArray[guild].numCohorts) == FALSE) {
				quit("Error: Unable to find parameter '%s/%s' in input file %s\n", errorString, FunctGroupArray[guild].groupCode, fileName);
			}

			// Not using "* FunctGroupArray[guild].numGeneTypes" here as harvest code refers back to age classes only for this parameter 
			for (cohort = 0; cohort < FunctGroupArray[guild].numCohortsXnumGenes; cohort++) {
				basecohort = (int)floor(cohort / FunctGroupArray[guild].numGeneTypes);
				for (b = 0; b < bm->K_num_fisheries; b++) {
					if (bm->SP_FISHERYprms[guild][b][FC_case_id] == 0 && isA == 0) {
						FFCDR[guild][b][cohort] = values[basecohort];
					}
					if (bm->SP_FISHERYprms[guild][b][FC_case_id] == 1 && isA > 0) {
						FFCDR[guild][b][cohort] = values[basecohort];
					}
				}
			}

			free(values);
		} else if (FunctGroupArray[guild].groupAgeType == AGE_STRUCTURED_BIOMASS) {
			/* Only bother trying to read in data if this is group A. */
			if (isA == 0) {
				if(Util_XML_Read_Array_Double(ATLANTIS_GROUP_ATTRIBUTE, fileName, errorString, attributeGroup, no_checking, FunctGroupArray[guild].groupCode, &values,
						FunctGroupArray[guild].numCohorts) == FALSE){
					quit("Error: Unable to find parameter '%s/%s' in input file %s\n", errorString, FunctGroupArray[guild].groupCode, fileName);
				}

			// Not using "* FunctGroupArray[guild].numGeneTypes" here as harvest code refers back to age classes only for this parameter 
				for (cohort = 0; cohort < FunctGroupArray[guild].numCohortsXnumGenes; cohort++) {
					basecohort = (int)floor(cohort / FunctGroupArray[guild].numGeneTypes);
					for (b = 0; b < bm->K_num_fisheries; b++) {
						FFCDR[guild][b][cohort] = values[basecohort];
					}
				}
				free(values);
			}
		}
	}
	free(nodeName);
}

void readImpactedGroupParamXML(MSEBoxModel *bm, char *fileName, xmlNodePtr parent, char *paramName, int check_type, double **targetArray) {
	int guild, b;
	xmlNodePtr attributeGroup;
	double *values = 0;
	char errorString[STRLEN];
	char *nodeName;

	nodeName =  Util_Get_Node_Name(parent);

	snprintf(errorString, sizeof(errorString), "%s/%s",nodeName, paramName);

	attributeGroup = Util_XML_Get_Node(ATLANTIS_ATTRIBUTE, parent, paramName);
	if (attributeGroup == NULL)
		quit("readImpactedGroupParamXML - %s attribute group not found.\n", errorString);

	if (verbose > 1)
		printf("Read fisheries groups parameters %s\n", paramName);

	for (guild = 0; guild < bm->K_num_tot_sp; guild++) {
		if (FunctGroupArray[guild].isImpacted == TRUE) {

			if(Util_XML_Read_Array_Double(ATLANTIS_GROUP_ATTRIBUTE, fileName, errorString, attributeGroup, check_type, FunctGroupArray[guild].groupCode, &values,
					bm->K_num_tot_sp) == FALSE){
				quit("Error: Unable to find parameter '%s/%s' in input file %s\n", errorString, FunctGroupArray[guild].groupCode, fileName);
			}

			for (b = 0; b < bm->K_num_tot_sp; b++) {
				targetArray[guild][b] = values[b];
			}
			free(values);
		}
	}
	free(nodeName);
}

void readMPAXML(MSEBoxModel *bm, char *fileName, xmlNodePtr parent, char *paramName, double **targetArray) {
	xmlNodePtr attributeGroup;
	int fisheryIndex, b, flagF, index;
	double *values = 0;
	int check_type = proportion_check;
	int isMPA = FALSE;
	char errorString[STRLEN];
	char *nodeName;

	nodeName =  Util_Get_Node_Name(parent);
	snprintf(errorString, sizeof(errorString), "%s/%s", nodeName, paramName);

	if (strcmp(paramName, "CellMPAStatus") == 0)
		isMPA = TRUE;

	attributeGroup = Util_XML_Get_Node(ATLANTIS_ATTRIBUTE, parent, paramName);
	;
	if (attributeGroup == NULL)
		quit("readMPAXML - %s attribute group not found.\n", errorString);

	if (verbose > 1)
		printf("Read fisheries groups parameters %s\n", paramName);

	/* Load simple fisheries specific MPA info */
	for (fisheryIndex = 0; fisheryIndex < bm->K_num_fisheries; fisheryIndex++) {

		if (isMPA == TRUE) {

			/* Work out what sort of checking we should be using */
			flagF = 0;
			for (index = 0; index < bm->K_num_tot_sp; index++) {
				if (FunctGroupArray[index].isImpacted == TRUE) {
					flagF += (int) (bm->SP_FISHERYprms[index][fisheryIndex][flagF_id]);
				}
			}
			if (flagF > 0)
				check_type = no_checking;
			else
				check_type = proportion_check;
		}

		if(Util_XML_Read_Array_Double(ATLANTIS_FISHERY_ATTRIBUTE, fileName, errorString, attributeGroup, check_type, FisheryArray[fisheryIndex].fisheryCode, &values, bm->nbox) == FALSE){
			quit("Error: Unable to find parameter '%s/%s' in input file %s\n", errorString, FisheryArray[fisheryIndex].fisheryCode, fileName);
		}

		for (b = 0; b < bm->nbox; b++) {
			/* Zoning for each box per fishery */
			targetArray[b][fisheryIndex] = values[b];
			if (isMPA == TRUE) {
				if (values[b] > 1.0)
					warn("MPA value for %s in box %d > 1.0 (this is ok if *fishing the line*, but otherwise may want to reset <= 1.0)\n",
							FisheryArray[fisheryIndex].fisheryCode, b);
			}
		}
		free(values);
	}
	free(nodeName);
}

void readSelChangeXML(MSEBoxModel *bm, char *fileName, xmlNodePtr parent, char *paramName, int paramID) {

	xmlNodePtr attributeGroup;
	double *values = 0;
	int fisheryIndex = -1;
	int counter, b;
	char errorString[STRLEN];
	char *nodeName;

	nodeName =  Util_Get_Node_Name(parent);
	snprintf(errorString, sizeof(errorString), "%s/%s", nodeName, paramName);

	attributeGroup = Util_XML_Get_Node(ATLANTIS_ATTRIBUTE, parent, paramName);
	if (attributeGroup == NULL)
		quit("readSelChangeXML - %s attribute group not found.\n", errorString);

	for (fisheryIndex = 0; fisheryIndex < bm->K_num_fisheries; fisheryIndex++) {

		counter = (int) bm->FISHERYprms[fisheryIndex][SEL_num_changes_id];

		if(Util_XML_Read_Array_Double(ATLANTIS_FISHERY_ATTRIBUTE, fileName, errorString, attributeGroup, no_checking, FisheryArray[fisheryIndex].fisheryCode, &values, counter) == FALSE){
			quit("Error: Unable to find parameter '%s/%s' in input file %s\n", errorString, FisheryArray[fisheryIndex].fisheryCode, fileName);
		}

		for (b = 0; b < counter; b++) {
			SELchange[fisheryIndex][b][paramID] = values[b];
		}
		free(values);
	}
	free(nodeName);

}

void readDiscardChangeXML(MSEBoxModel *bm, char *fileName, xmlNodePtr parent, char *paramName, int paramID) {

	xmlNodePtr attributeGroup;
	double *values = 0;
	int counter, b;
	int check = no_checking;
	int speciesIndex, i;
	char errorString[STRLEN];
	char *nodeName;

	nodeName =  Util_Get_Node_Name(parent);
	snprintf(errorString, sizeof(errorString), "%s/%s", nodeName, paramName);

	if (paramID == start_id)
		check = integer_check;

	attributeGroup = Util_XML_Get_Node(ATLANTIS_ATTRIBUTE, parent, paramName);
	if (attributeGroup == NULL)
		quit("readDiscardChangeXML - %s attribute group not found.\n", errorString);

	for (speciesIndex = 0; speciesIndex < bm->K_num_tot_sp; speciesIndex++) {

		if (FunctGroupArray[speciesIndex].isFished == TRUE) {
			// Get the speciesIndex
			counter = 0;	/* Changed from 1 */
			for (i = 0; i < bm->K_num_fisheries; i++) {
				counter = max ( counter, (int)bm->SP_FISHERYprms[speciesIndex][i][DISCRD_num_changes_id] );
			}

			if(counter > 0){
				if(Util_XML_Read_Array_Double(ATLANTIS_GROUP_ATTRIBUTE, fileName, errorString, attributeGroup, check, FunctGroupArray[speciesIndex].groupCode, &values, counter) == FALSE){
					quit("Error: Unable to find parameter '%s/%s' in input file %s\n", errorString, FunctGroupArray[speciesIndex].groupCode, fileName);
				}

				for (i = 0; i < bm->K_num_fisheries; i++)
					for (b = 0; b < bm->SP_FISHERYprms[speciesIndex][i][DISCRD_num_changes_id]; b++)
						DISCRDchange[speciesIndex][i][b][paramID] = values[b];
				free(values);
			}
		}
	}
	free(nodeName);
}

/**
 *	\brief Read the data into the recruit_vdistrib array from the input XML file.
 */
static void Read_QStock_XML(MSEBoxModel *bm, char *fileName, xmlNodePtr parent) {
    
	double *values = 0;
	int guild, b, cohort;
	int counter;
	char errorString[STRLEN];
	xmlNodePtr attributeNode, specieNode;
    
    if (bm->K_num_stocks_per_sp > 1) {
    
        if (verbose)
            printf("Reading qStock values\n");
    
        attributeNode = Util_XML_Get_Node(ATLANTIS_ATTRIBUTE, parent, "qStock");
        if (attributeNode == NULL)
            quit("Fishing_Scenario_Flags/qStock attribute group not found in input file %s.\n", fileName);
    
        /* Create a node for each functional group*/
        for (guild = 0; guild < bm->K_num_tot_sp; guild++) {
            if (FunctGroupArray[guild].isVertebrate == TRUE) {
            
                snprintf(errorString, sizeof(errorString), "Fishing_Scenario_Flags/qStock/%s", FunctGroupArray[guild].groupCode);
                specieNode = Util_XML_Get_Node(ATLANTIS_GROUP_ATTRIBUTE, attributeNode, FunctGroupArray[guild].groupCode);
                if (specieNode == NULL)
                    quit("%s species attribute group not found.\n", errorString);
            
                for (cohort = 0; cohort < FunctGroupArray[guild].numStages; cohort++) {
                
                    counter = FunctGroupArray[guild].numStocks;
                    if (Util_XML_Read_Array_Double(ATLANTIS_AGE_CLASS_ATTRIBUTE, fileName, errorString, specieNode, no_checking, cohortStrings[cohort], &values, counter) == FALSE) {
                        quit("Error: Unable to find parameter '%s/%s' in input file %s\n", errorString, cohortStrings[cohort],
                         fileName);
                    }
                
                    for (b = 0; b < counter; b++) {
                        qSTOCK[guild][b][cohort] = values[b];
                    }
                    free(values);
                }
			}
		}
	} else {
        for (b = 0; b < bm->K_num_tot_sp; b++) {
             for (cohort = 0; cohort < FunctGroupArray[b].numStages; cohort++) {
                 qSTOCK[b][0][cohort] = 1.0;
             }
        }
    }
}


/**
 * \brief Read in the fishery flag parameters from the input file.
 *
 */
void readFishingFlagValues(MSEBoxModel *bm, char *fileName, xmlNodePtr rootnode) {

	int i;
	xmlNodePtr attributeGroupNode;
	attributeGroupNode = Util_XML_Get_Node(ATLANTIS_ATTRIBUTE_SUB_GROUP, rootnode, "Fishing_Flags");
	if (attributeGroupNode == NULL)
		quit("Fishing_Flags attribute group not found in input file %s.\n", fileName);

	Util_XML_Read_Fishery_Param(bm, fileName, attributeGroupNode, fisheriesflagactive_id);
	Util_XML_Read_Fishery_Param(bm, fileName, attributeGroupNode, tStart_id);
	Util_XML_Read_Fishery_Param(bm, fileName, attributeGroupNode, tEnd_id);
	Util_XML_Read_Fishery_Param(bm, fileName, attributeGroupNode, start_manage_id);
	Util_XML_Read_Fishery_Param(bm, fileName, attributeGroupNode, end_manage_id);
	Util_XML_Read_Species_Param(bm, fileName, attributeGroupNode, flagfish_id);
	Util_XML_Read_Impacted_Group_Param(bm, fileName, attributeGroupNode, flaghabitat_id);

	Util_XML_Read_Fishery_Param(bm, fileName, attributeGroupNode, flagdempelfishery_id);
	Util_XML_Read_Fishery_Param(bm, fileName, attributeGroupNode, flageffortmodel_id);

	Util_XML_Get_Value_Integer(fileName, ATLANTIS_ATTRIBUTE, bm->ecotest, 1, attributeGroupNode, integer_check, "dynanyway", &bm->dynanyway);

	Util_XML_Read_Fishery_Param(bm, fileName, attributeGroupNode, flagexplore_id);
	Util_XML_Read_Fishery_Param(bm, fileName, attributeGroupNode, flagdropEFF_id);
	Util_XML_Read_Fishery_Param(bm, fileName, attributeGroupNode, selcurve_id);

	/* Get the knife curve value = this changes the type of checking done
	 * on the sel values
	 */
	knife_curve = 0;
	for (i = 0; i < bm->K_num_fisheries; i++) {
		if (bm->FISHERYprms[i][selcurve_id] == q_knife_id) {
			knife_curve++;
			break;
		}
	}

	Util_XML_Read_Impacted_Group_Param(bm, fileName, attributeGroupNode, flagdiscard_id);
	Util_XML_Read_Impacted_Group_Param(bm, fileName, attributeGroupNode, flagescapement_id);
    Util_XML_Read_Impacted_Group_Param(bm, fileName, attributeGroupNode, spawn_closure_id);
    
	Util_XML_Read_Species_Param(bm, fileName, attributeGroupNode, access_thru_wc_id);
}

/**
 * \brief Read in the fishery flag parameters from the input file.
 *
 */
void readAquacultValues(MSEBoxModel *bm, char *fileName, xmlNodePtr rootnode) {

	xmlNodePtr attributeGroupNode;
	attributeGroupNode = Util_XML_Get_Node(ATLANTIS_ATTRIBUTE_SUB_GROUP, rootnode, "Aquaculture_Values");
	if (attributeGroupNode == NULL)
		quit("Aquaculture_Values attribute group not found in input file %s.\n", fileName);

	Util_XML_Read_Species_Param(bm, fileName, attributeGroupNode, aquacult_age_harvest_id);
}


/**
 * \brief Read in the physical limitation input parameters.
 *
 */
void readFisheryCatchTimeXML(MSEBoxModel *bm, char *fileName, xmlNodePtr rootnode) {

	xmlNodePtr attributeGroupNode;
	attributeGroupNode = Util_XML_Get_Node(ATLANTIS_ATTRIBUTE_SUB_GROUP, rootnode, "Catch_Time");
	if (attributeGroupNode == NULL)
		quit("Catch_Time attribute group not found in input file %s.\n", fileName);

	//flagimposeglobal =  (int) Util_XML_Read_Value(fileName, ATLANTIS_ATTRIBUTE,  bm->ecotest, 1, attributeGroupNode, integer_check, "flagimposeglobal");
	Util_XML_Read_Impacted_Group_Param(bm, fileName, attributeGroupNode, flagimposecatch_id);
	Util_XML_Read_Impacted_Group_Param(bm, fileName, attributeGroupNode, flagF_id);

}

void readFisheryScenarioFlags(MSEBoxModel *bm, char *fileName, xmlNodePtr rootnode) {
	xmlNodePtr attributeGroupNode;

	attributeGroupNode = Util_XML_Get_Node(ATLANTIS_ATTRIBUTE_SUB_GROUP, rootnode, "Fishing_Scenario_Flags");

    bm->flagchangediscard = (int) (Util_XML_Read_Value(fileName, ATLANTIS_ATTRIBUTE,  bm->ecotest, 1, attributeGroupNode, binary_check, "flagchangediscrd"));
    bm->flagchangesel = (int) (Util_XML_Read_Value(fileName, ATLANTIS_ATTRIBUTE,  bm->ecotest, 1, attributeGroupNode, binary_check, "flagchangesel"));
    bm->flagchangeF = (int) (Util_XML_Read_Value(fileName, ATLANTIS_ATTRIBUTE,  bm->ecotest, 1, attributeGroupNode, binary_check, "flagchangeF"));
    bm->flagchangeq = (int) (Util_XML_Read_Value(fileName, ATLANTIS_ATTRIBUTE,  bm->ecotest, 1, attributeGroupNode, binary_check, "flagchangeq"));
    bm->flagchangep = (int) (Util_XML_Read_Value(fileName, ATLANTIS_ATTRIBUTE,  bm->ecotest, 1, attributeGroupNode, binary_check, "flagchangep"));
    bm->flagchangeswept = (int) (Util_XML_Read_Value(fileName, ATLANTIS_ATTRIBUTE,  bm->ecotest, 1, attributeGroupNode, binary_check, "flagchangeswept"));
    
	Util_XML_Read_Fishery_Param(bm, fileName, attributeGroupNode, flagchangeSEL_id);
	Util_XML_Read_Fishery_Param(bm, fileName, attributeGroupNode, flagchangeP_id);
	Util_XML_Read_Fishery_Param(bm, fileName, attributeGroupNode, flagchangeSWEPT_id);

	Util_XML_Read_Impacted_Group_Param(bm, fileName, attributeGroupNode, flagQchange_id);
	Util_XML_Read_Impacted_Group_Param(bm, fileName, attributeGroupNode, flagFchange_id);
	Util_XML_Read_Impacted_Group_Param(bm, fileName, attributeGroupNode, flagchangeDISCRD_id);

	/* Reduction in effectiveness of gear due to mismatch in watercolumn of
	 gear and vertebrates */
	Util_XML_Get_Value_Double(fileName, ATLANTIS_ATTRIBUTE, bm->ecotest, 1, attributeGroupNode, no_checking, "k_mismatch", &k_mismatch);

	Util_XML_Read_Fishery_Param_Int(bm, fileName, attributeGroupNode, "FisheryTargetSpecies", binary_check, bm->FISHERYtarget, bm->K_num_tot_sp);

	readFisheryAgeDistributionXML(bm, fileName, attributeGroupNode, "CatchAgeDistributions", proportion_check, bm->CatchTS_agedistrib);

	Util_XML_Get_Value_Double(fileName, ATLANTIS_ATTRIBUTE, bm->ecotest, 1, attributeGroupNode, no_checking, "highgrade_thresh", &bm->highgrade_thresh);
	Util_XML_Read_Impacted_Group_Param(bm, fileName, attributeGroupNode, q_id);
    Read_QStock_XML(bm, fileName, attributeGroupNode);
        
}

/**
 * General Vessel Characteristics
 *
 */
void readGeneralVesselCharateristics(MSEBoxModel *bm, char *fileName, xmlNodePtr rootnode) {
	xmlNodePtr attributeGroupNode;
	 double  numsechr = 3600.0;		/* number of seconds in a hour */

	attributeGroupNode = Util_XML_Get_Node(ATLANTIS_ATTRIBUTE_SUB_GROUP, rootnode, "General_Vessel_Characteristics");
	if (attributeGroupNode == NULL)
		quit("General_Vessel_Characteristics attribute group not found in input file %s.\n", fileName);

	Util_XML_Get_Value_Double(fileName, ATLANTIS_ATTRIBUTE, bm->ecotest, 1, attributeGroupNode, no_checking, "Speed_boat", &bm->Speed_boat);
	Util_XML_Get_Value_Double(fileName, ATLANTIS_ATTRIBUTE, bm->ecotest, 1, attributeGroupNode, no_checking, "Speed_recboat", &bm->Speed_recboat);
	Util_XML_Get_Value_Double(fileName, ATLANTIS_ATTRIBUTE, bm->ecotest, 1, attributeGroupNode, no_checking, "KDEP_fishery", &KDEP_fishery);

	bm->Speed_boat = bm->Speed_boat / numsechr;
	bm->Speed_recboat = bm->Speed_recboat / numsechr;
}

/*
 * Fisheries habitat association.
 */
void readFisheryHabitat(MSEBoxModel *bm, char *fileName, xmlNodePtr rootnode) {
	xmlNodePtr childGroupingNode;
	xmlNodePtr attributeGroupNode;

	attributeGroupNode = Util_XML_Get_Node(ATLANTIS_ATTRIBUTE_SUB_GROUP, rootnode, "Fishery_Habitat");
	if (attributeGroupNode == NULL)
		quit("Fishery_Habitat attribute group not found in input file %s.\n", fileName);

	Util_XML_Read_Fishery_Param_Double(bm, fileName, attributeGroupNode, "FisheryHabitats", no_checking, bm->FC_HABITATlike, bm->K_num_cover_types);

	//TODO possibly should be in management */
	Util_XML_Read_Fishery_Param(bm, fileName, attributeGroupNode, maxFCdepth_id);
	Util_XML_Read_Fishery_Param(bm, fileName, attributeGroupNode, minFCdepth_id);

	childGroupingNode = Util_XML_Get_Node(ATLANTIS_ATTRIBUTE_SUB_GROUP, attributeGroupNode, "Habitat_Patchiness");
	if (childGroupingNode == NULL)
		quit("Habitat_Patchiness attribute group not found in input file %s.\n", fileName);

	/* Habitat patchiness parameters for trawl mortality */
	k_pattern = Util_XML_Read_Value(fileName, ATLANTIS_ATTRIBUTE,  bm->ecotest, 1, childGroupingNode, no_checking, "k_pattern");
	k_patches = Util_XML_Read_Value(fileName, ATLANTIS_ATTRIBUTE,  bm->ecotest, 1, childGroupingNode, no_checking, "k_patches");
	Util_XML_Read_Fishery_Param_Double(bm, fileName, childGroupingNode, "BoxHabitatProportion", no_checking, k_cover, bm->nbox);
    
    if(k_pattern <= -1.0)
        quit("k_pattern must be > -1.0\n");
}

/*
 * Fisheries time values.
 */
void readFisheryTimeValues(MSEBoxModel *bm, char *fileName, xmlNodePtr rootnode) {
	xmlNodePtr groupingNode = Util_XML_Get_Node(ATLANTIS_ATTRIBUTE_SUB_GROUP, rootnode, "Fishery_Times");
	if (groupingNode == NULL)
		quit("Fishery_Times attribute group not found in input file %s.\n", fileName);

	Util_XML_Read_Impacted_Group_Param(bm, fileName, groupingNode, imposecatchstart_id);
	Util_XML_Read_Impacted_Group_Param(bm, fileName, groupingNode, imposecatchend_id);
}

/*
 * Fisheries reporting
 */
void readCatchReportingValues(MSEBoxModel *bm, char *fileName, xmlNodePtr rootnode) {

	xmlNodePtr groupingNode = Util_XML_Get_Node(ATLANTIS_ATTRIBUTE_SUB_GROUP, rootnode, "Catch_Reporting");
	if (groupingNode == NULL)
		quit("Catch_Reporting attribute group not found in input file %s.\n", fileName);

	Util_XML_Read_Impacted_Group_Param(bm, fileName, groupingNode, FC_reportscale_id);
}

/*
 * Fishing mortality
 */
void readFishingMortalityXML(MSEBoxModel *bm, char *fileName, xmlNodePtr rootnode, int id) {
	xmlNodePtr groupingNode = Util_XML_Get_Node(ATLANTIS_ATTRIBUTE_SUB_GROUP, rootnode, "Fishing_Mortality");
	if (groupingNode == NULL)
		quit("Fishing_Mortality attribute group not found in input file %s.\n", fileName);

	Util_XML_Read_Impacted_Group_Param(bm, fileName, groupingNode, id);
	readAgeStructuredBiomassFisheryParamXML(bm, fileName, groupingNode, mFC_start_age_id);
    Util_XML_Read_Impacted_Group_Param(bm, fileName, groupingNode, id);
    readAgeStructuredBiomassFisheryParamXML(bm, fileName, groupingNode, mFC_end_age_id);

}

/*
 * Fishing selectivity
 */
void readFishingSelectivityXML(MSEBoxModel *bm, char *fileName, xmlNodePtr rootnode) {
	xmlNodePtr selGroupNode;
	xmlNodePtr groupingNode = Util_XML_Get_Node(ATLANTIS_ATTRIBUTE_SUB_GROUP, rootnode, "Fishing_Selectivity");
	if (groupingNode == NULL)
		quit("Fishing_Selectivity attribute group not found in input file %s.\n", fileName);

	Util_XML_Read_Impacted_Group_Param(bm, fileName, groupingNode, sel_id);

	/* The type of checking depends on the curve_knife value */
	if (knife_curve)
		readGroupAgeGroupFisheryParamXML(bm, fileName, groupingNode, no_checking, "ConstantAgeGroupSelectivity", bm->selectivity);
	else
		readGroupAgeGroupFisheryParamXML(bm, fileName, groupingNode, proportion_check, "ConstantAgeGroupSelectivity", bm->selectivity);

	selGroupNode = Util_XML_Get_Node(ATLANTIS_ATTRIBUTE_SUB_GROUP, groupingNode, "Selectivity_Curve");
	if (selGroupNode == NULL)
		quit("Selectivity_Curve attribute group not found in input file %s.\n", fileName);

	Util_XML_Read_Fishery_Param(bm, fileName, selGroupNode, sel_b_id);
	Util_XML_Read_Fishery_Param(bm, fileName, selGroupNode, sel_lsm_id);
	Util_XML_Read_Fishery_Param(bm, fileName, selGroupNode, sel_normsigma_id);
	Util_XML_Read_Fishery_Param(bm, fileName, selGroupNode, sel_normlsm_id);
	Util_XML_Read_Fishery_Param(bm, fileName, selGroupNode, sel_lognormsigma_id);
	Util_XML_Read_Fishery_Param(bm, fileName, selGroupNode, sel_lognormlsm_id);
	Util_XML_Read_Fishery_Param(bm, fileName, selGroupNode, sel_gammasigma_id);
	Util_XML_Read_Fishery_Param(bm, fileName, selGroupNode, sel_gammalsm_id);
	Util_XML_Read_Fishery_Param(bm, fileName, selGroupNode, sel_bisigma_id);
	Util_XML_Read_Fishery_Param(bm, fileName, selGroupNode, sel_bisigma2_id);
	Util_XML_Read_Fishery_Param(bm, fileName, selGroupNode, sel_ampli_id);
	Util_XML_Read_Fishery_Param(bm, fileName, selGroupNode, sel_bilsm1_id);
	Util_XML_Read_Fishery_Param(bm, fileName, selGroupNode, sel_bilsm2_id);
}

/*
 * Fishing Escapement
 */
void readFishingEscapmentXML(MSEBoxModel *bm, char *fileName, xmlNodePtr rootnode) {
	xmlNodePtr groupingNode;
	groupingNode = Util_XML_Get_Node(ATLANTIS_ATTRIBUTE_SUB_GROUP, rootnode, "Fishing_Escapment");
	if (groupingNode == NULL)
		quit("Fishing_Escapment attribute group not found in input file %s.\n", fileName);

	Util_XML_Read_Impacted_Group_Param(bm, fileName, groupingNode, p_escape_id);
	Util_XML_Read_Impacted_Group_Param(bm, fileName, groupingNode, Ka_escape_id);
	Util_XML_Read_Impacted_Group_Param(bm, fileName, groupingNode, Kb_escape_id);
}

/*
 * Fishing Discarding
 */
void readFishingDiscardingXML(MSEBoxModel *bm, char *fileName, xmlNodePtr rootnode) {
	xmlNodePtr groupingNode, selGroupNode;
	groupingNode = Util_XML_Get_Node(ATLANTIS_ATTRIBUTE_SUB_GROUP, rootnode, "Fishing_Discarding");
	if (groupingNode == NULL)
		quit("Fishing_Discarding attribute group not found in input file %s.\n", fileName);

	/* Discarding parameters */
	bm->prop_within = Util_XML_Read_Value(fileName, ATLANTIS_ATTRIBUTE,  bm->ecotest, 1, groupingNode, no_checking, "prop_within");
	bm->salethresh = Util_XML_Read_Value(fileName, ATLANTIS_ATTRIBUTE,  bm->ecotest, 1, groupingNode, no_checking, "salethresh");

	Util_XML_Read_Fishery_Param(bm, fileName, groupingNode, maxsaleprice_id);

	readFisheryAgeDistributionXML(bm, fileName, groupingNode, "DiscardAgeDistributions", proportion_check, bm->DiscardTS_agedistrib);
	readImpactedGroupParamXML(bm, fileName, groupingNode, "ProportionDiscarding", no_checking, bm->FCcocatch);


	Util_XML_Read_Impacted_Group_Param(bm, fileName, groupingNode, FFCDR_id);
	Util_XML_Read_Impacted_Group_Param(bm, fileName, groupingNode, FC_case_id);

	readAgeGroupConstantDiscardingXML(bm, fileName, groupingNode, "AgeGroupConstantDiscardingA");
	readAgeGroupConstantDiscardingXML(bm, fileName, groupingNode, "AgeGroupConstantDiscardingB");

	selGroupNode = Util_XML_Get_Node(ATLANTIS_ATTRIBUTE_SUB_GROUP, groupingNode, "Size_Based_Discarding");
	if (selGroupNode == NULL)
		quit("Fishing_Discarding/Size_Based_Discarding attribute group not found in input file %s.\n", fileName);

	readAgeStructuredGroupFisheryParamXML(bm, fileName, selGroupNode, FCthreshli_id);
	Util_XML_Read_Impacted_Group_Param(bm, fileName, selGroupNode, incidmort_id);
	readAgeStructuredGroupFisheryParamXML(bm, fileName, selGroupNode, k_retain_id);
	readAgeStructuredGroupFisheryParamXML(bm, fileName, selGroupNode, k_waste_id);

}

/*
 * Fishing gear
 */
void readFishingGearXML(MSEBoxModel *bm, char *fileName, xmlNodePtr rootnode) {
	xmlNodePtr groupingNode;
	groupingNode = Util_XML_Get_Node(ATLANTIS_ATTRIBUTE_SUB_GROUP, rootnode, "Fishing_Gear");
	if (groupingNode == NULL)
		quit("Fishing_Gear attribute group not found in input file %s.\n", fileName);

	Util_XML_Read_Fishery_Param(bm, fileName, groupingNode, swept_area_id);

}

//
///*
// * Fishing Spatial Management
// */
//void readSpatialManagementXML(MSEBoxModel *bm, char *fileName, xmlNodePtr rootnode) {
//	xmlNodePtr groupingNode;
//	groupingNode = Util_XML_Get_Node(ATLANTIS_ATTRIBUTE_SUB_GROUP, rootnode, "Spatial_Management");
//
//
//	Util_XML_Read_Fishery_Param(bm, fileName, groupingNode, infringe_id);
//
//	readMPAXML(bm, groupingNode, "CellMPAStatus", bm->MPA);
//
//	readMPAXML(bm, groupingNode, "MPAEndangered", MPAendangered);
//	readMPAXML(bm, groupingNode, "MPAOverFished", bm->MPAoverfishedlist);
//
//
//}


/*
 * Changes in fisheries over time.
 */
void readFisheryChangeXML(MSEBoxModel *bm, char *fileName, xmlNodePtr rootnode) {

	//printf("Creating fishery change arrays\n");

	int SEL_max_num_changes, SWEPT_max_num_changes, DISCRD_max_num_changes, P_max_num_changes;
	xmlNodePtr groupingNode, selGroupNode;
	groupingNode = Util_XML_Get_Node(ATLANTIS_ATTRIBUTE_SUB_GROUP, rootnode, "Fishery_Change");

	mFC_max_num_changes = 0;
	/* not sure about the grouping of these nodes */
	selGroupNode = Util_XML_Get_Node(ATLANTIS_ATTRIBUTE_SUB_GROUP, groupingNode, "Selectivity_Change");

	Util_XML_Read_Fishery_Param(bm, fileName, selGroupNode, SEL_num_changes_id);

    /* Get the max number of changes, allocate a big enough array and load the values */

	SEL_max_num_changes = 1;
	if (bm->flagchangesel)
		SEL_max_num_changes = Manage_Get_Max_Fishery_Param(bm, SEL_num_changes_id) + 1;

	SELchange = Util_Alloc_Init_3D_Double(4, SEL_max_num_changes, bm->K_num_fisheries, 0.0);
    
    /* not sure about the grouping of these nodes */
	//selGroupNode = Util_XML_Get_Node(ATLANTIS_ATTRIBUTE_SUB_GROUP, groupingNode, "Selectivity_Change");

	readSelChangeXML(bm, fileName, selGroupNode, "Start", start_id);
	readSelChangeXML(bm, fileName, selGroupNode, "NewCurve", selcurvechange_id);
	readSelChangeXML(bm, fileName, selGroupNode, "ConstantValueAddition", addlsm_id);
	readSelChangeXML(bm, fileName, selGroupNode, "SigmaValueAddition", addsigma_id);
    
	/* */
	selGroupNode = Util_XML_Get_Node(ATLANTIS_ATTRIBUTE_SUB_GROUP, groupingNode, "Coverage_Change");

	Util_XML_Read_Fishery_Param(bm, fileName, selGroupNode, P_num_changes_id);

	P_max_num_changes = 1;
	if (bm->flagchangep)
		P_max_num_changes = Manage_Get_Max_Fishery_Param(bm, P_num_changes_id) + 1;

	Pchange = Util_Alloc_Init_3D_Double(4, P_max_num_changes, bm->K_num_fisheries, 0.0);

	Util_XML_Read_Change_Values(bm, fileName, selGroupNode, "Pchange", P_num_changes_id, Pchange);

	//Util_XML_Read_Change_Values(bm, selGroupNode, "Pchange", P_num_changes_id, Pchange);

	selGroupNode = Util_XML_Get_Node(ATLANTIS_ATTRIBUTE_SUB_GROUP, groupingNode, "Swept_change");

	Util_XML_Read_Fishery_Param(bm, fileName, selGroupNode, SWEPT_num_changes_id);

	SWEPT_max_num_changes = 1;
	if (bm->flagchangeswept)
		SWEPT_max_num_changes = Manage_Get_Max_Fishery_Param(bm, SWEPT_num_changes_id) + 1;

	SWEPTchange = Util_Alloc_Init_3D_Double(4, SWEPT_max_num_changes, bm->K_num_fisheries, 0.0);

	Util_XML_Read_Change_Values(bm, fileName, selGroupNode, "SWEPTchange", SWEPT_num_changes_id, SWEPTchange);

	selGroupNode = Util_XML_Get_Node(ATLANTIS_ATTRIBUTE_SUB_GROUP, groupingNode, "Discard_Changes");

	Util_XML_Read_Impacted_Group_Param(bm, fileName, selGroupNode, DISCRD_num_changes_id);

	/* Get the max number of discard changes so we can allocate the array */
	DISCRD_max_num_changes = 1;
	if (bm->flagchangediscard)
		DISCRD_max_num_changes = Manage_Get_Max_Species_Fishery_Param(bm, DISCRD_num_changes_id) + 1;

	DISCRDchange = Util_Alloc_Init_4D_Double(5, DISCRD_max_num_changes, bm->K_num_fisheries, bm->K_num_tot_sp, 0.0);

	readDiscardChangeXML(bm, fileName, selGroupNode, "Start", start_id);
	readDiscardChangeXML(bm, fileName, selGroupNode, "ChangeThreshMult", thresh_mult_id);
	readDiscardChangeXML(bm, fileName, selGroupNode, "RetainMult", retain_mult_id);
    readDiscardChangeXML(bm, fileName, selGroupNode, "DiscardMult", discard_mult_id);
    readDiscardChangeXML(bm, fileName, selGroupNode, "WasteMult", waste_mult_id);

	/* not sure about the grouping of these nodes */
	selGroupNode = Util_XML_Get_Node(ATLANTIS_ATTRIBUTE_SUB_GROUP, groupingNode, "Catchability_Change");

	Util_XML_Read_Impacted_Group_Param(bm, fileName, selGroupNode, Q_num_changes_id);
	//Util_XML_Read_Change_Fished_Groups(bm, selGroupNode, "Qchange", Q_num_changes_id, Qchange);

	Q_max_num_changes = 1;
	if (bm->flagchangeq)
		Q_max_num_changes = Manage_Get_Max_Species_Fishery_Param(bm, Q_num_changes_id) + 1;
	Qchange = Util_Alloc_Init_4D_Double(3, Q_max_num_changes, bm->K_num_fisheries, bm->K_num_tot_sp, 0.0);

	Util_XML_Read_Change_Fished_Groups(bm, fileName, selGroupNode, "Qchange", Q_num_changes_id, Qchange);

	/* not sure about the grouping of these nodes */
	selGroupNode = Util_XML_Get_Node(ATLANTIS_ATTRIBUTE_SUB_GROUP, groupingNode, "Fishing_Mortality_Change");
	Util_XML_Read_Impacted_Group_Param(bm, fileName, selGroupNode, mFC_num_changes_id);

	/* Get the max number of mFC changes so we can allocate the array and load the values */
	mFC_max_num_changes = Manage_Get_Max_Species_Fishery_Param(bm, mFC_num_changes_id) + 1;
	mFCchange = Util_Alloc_Init_4D_Double(3, mFC_max_num_changes, bm->K_num_fisheries, bm->K_num_tot_sp, 0.0);

	//selGroupNode = Util_XML_Get_Node(ATLANTIS_ATTRIBUTE_SUB_GROUP, groupingNode, "Fishing_Mortality_Change");
	Util_XML_Read_Change_Fished_Groups(bm, fileName, selGroupNode, "mFCchange", mFC_num_changes_id, mFCchange);

}

/**
 *	This routine reads all parameters of the ecological models
 *
 *  Input:  char *filename: pointer to biology input XML file.
 *
 */
int Read_Harvest_Parameters(MSEBoxModel *bm, char *filename) {
	xmlDocPtr inputDoc;
	inputDoc = xmlReadFileDestFolder(bm->destFolder, filename, NULL, 0);

	readFishingFlagValues(bm, filename, inputDoc->children);
	readFisheryCatchTimeXML(bm, filename, inputDoc->children);
	readFisheryScenarioFlags(bm, filename, inputDoc->children);
	readGeneralVesselCharateristics(bm, filename, inputDoc->children);
	readFisheryHabitat(bm, filename, inputDoc->children);
	readFisheryTimeValues(bm, filename, inputDoc->children);
	readCatchReportingValues(bm, filename, inputDoc->children);
	readFishingMortalityXML(bm, filename, inputDoc->children, mFC_id);
    readFishingSelectivityXML(bm, filename, inputDoc->children);
	readFishingEscapmentXML(bm, filename, inputDoc->children);
	readFishingDiscardingXML(bm, filename, inputDoc->children);
	readFishingGearXML(bm, filename, inputDoc->children);

	readFisheryChangeXML(bm, filename, inputDoc->children);
    
    

    
	if(bm->flag_aquacult_on)
		readAquacultValues(bm, filename, inputDoc->children);

	xmlFreeDoc(inputDoc);
	/* Shutdown libxml */
	xmlCleanupParser();
	return TRUE;
}

