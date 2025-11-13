/**

 \file
 \brief This file contains functions to convert the biology prm file to an XML file.




 Changes:

 31-03-2009 Bec Gorton
 Got rid of the getNodeValue function as it did not allow for freeing the memory allocated in the Execute_Xpath_Expression
 function call. This has been replaced by doesNodeExist which can be used to check if a node exists.
 Have created a new function where you pass a string to it and the node string is copied into that string. This means
 we can free the node allocated in the Execute_Xpath_Expression function.


 14-04-2009 Bec Gorton
 Added code to split the normal prey avail and the detritus prey avail. These values are now stored in the DetritusSedimentFoodAvail
 xml element.

 15-05-2009 Bec Gorton
 Changed the structure of the XML document to use a more generic Attribute structure.
 constant use of the word flag - this will now only be used when
 the attribute is a boolean.

 10-06-2009 Bec Gorton
 Change the names of many of the attributes to try and have a similar structure throughout the document.


 10-06-2009 Bec Gorton
 Added the SEAGRASS invertebrate type.


 10-06-2009 Bec Gorton
 Added the SEAGRASS invertebrate type.

 15-06-2009 Bec Gorton
 Changed the k_trans value to be a species parameter. The k_trans value is now used for the
 BFS, BFF and BFD values. For the rest of the groups 1.0 is used. Parse_File_Single_Line is used in convertAtlantis
 to create the new XML.

 09-07-2009 Bec Gorton
 Change the biology XML to contain a number of additional values:
 - the type of the parameters are stored. The possible types are defined in AtlantisXMLAttributeTypeStrings.
 - the possible value are also added with the XML_TYPE_LOOKUP attribute type.
 - the recommended values

 04-11-2009 Bec Gorton
 Merged the k_SED functionality in the trunk into the bec-dev branch.

 16-08-2009 Bec Gorton
 Merged in the q10 species parameter.

 06-01-2010 Bec Gorton
 Removed the code that set the EDR and EDL values for invertebrates to 0 by default. The code will not quit if a value is
 not found for all invertebrates.
 Also added code to read in the homerange functional group attributes.

  10-01-2013 Beth Fulton
 Finalising addition of acidification and temperature dependent physiology parameters

 17-04-2013 Bec Gorton
 ATL-346 - Added support of tracking P:N:C ratios in groups.

 21-04-2013 Beth Fulton
 Added overwintering

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
#include <atEcologyLib.h>
#include "convertXML.h"

/* TODO: Get rid of this constrant */
#define K_max_num_mig 3


xmlNodePtr Create_Species_Cohort_ParamXML(MSEBoxModel *bm, char *fileName, FILE *inputFP, xmlDocPtr doc, xmlNodePtr parent, int paramID, char *comment,
		char *units, int type, char *recommendedValue, void(*fp)(MSEBoxModel *bm, char *fileName, xmlNodePtr parent, char *str, char *valueStr));
xmlNodePtr Create_Species_Spawn_ParamXML(MSEBoxModel *bm, char *fileName, FILE *inputFP, xmlDocPtr doc, xmlNodePtr parent, int paramID, char *comment,
		char *units, int type, char *recommendedValue, void(*fp)(MSEBoxModel *bm, char *fileName, xmlNodePtr parent, char *str, char *valueStr));

static char *CreateZeroString(int size){
	int i;
	char *zeroString = (char *) malloc(sizeof(char) * (unsigned int long)3 * (unsigned int long)size);


	strcpy(zeroString, "");

	for(i = 0; i < size; i++){
		strcat(zeroString, " 0");
	}

	trim(zeroString);
	return zeroString;

}
/**
 *	\brief Create the XML structure to hold the vertebrate seasonal distribution data.
 */
void Init_Age_Structured_SpeciesXML(MSEBoxModel *bm, xmlNodePtr parent) {

	int guild;

	/* Create a node for each functional group*/
	for (guild = 0; guild < bm->K_num_tot_sp; guild++) {
		if (FunctGroupArray[guild].groupAgeType == AGE_STRUCTURED || FunctGroupArray[guild].groupAgeType == AGE_STRUCTURED_BIOMASS) {
			Util_XML_Create_Node(ATLANTIS_GROUP_ATTRIBUTE, parent, FunctGroupArray[guild].groupCode, "", "", "");
		}
	}
}

/**
 *	\brief Create the XML structure to hold the vertebrate seasonal distribution data.
 */
void InitAGESTRUCTUREDBIOMASSSpeciesXML(MSEBoxModel *bm, xmlNodePtr parent) {

	int guild;

	/* Create a node for each functional group*/
	for (guild = 0; guild < bm->K_num_tot_sp; guild++) {
		if (FunctGroupArray[guild].groupAgeType == AGE_STRUCTURED_BIOMASS) {
			Util_XML_Create_Node(ATLANTIS_GROUP_ATTRIBUTE, parent, FunctGroupArray[guild].groupCode, "", "", "");
		}
	}
}

/**
 *    \brief Create the XML structure to hold the invertebrate seasonal distribution data.
 */
void InitInvertSpeciesZeroValuesXML(MSEBoxModel *bm, xmlNodePtr parent) {
    int guild;

    /* Create a node for each functional group*/
    for (guild = 0; guild < bm->K_num_tot_sp; guild++) {
        if (FunctGroupArray[guild].isVertebrate == FALSE) {
            Util_XML_Create_Node(ATLANTIS_GROUP_ATTRIBUTE, parent, FunctGroupArray[guild].groupCode, "", "", "0");
        }
    }
}

/**
 *    \brief Create the XML structure to hold the non-age structured parameters.
 */
void InitNonAgeSpeciesZeroValuesXML(MSEBoxModel *bm, xmlNodePtr parent) {
	int guild;

	/* Create a node for each functional group*/
	for (guild = 0; guild < bm->K_num_tot_sp; guild++) {
        if (FunctGroupArray[guild].numCohorts == 1) {
			Util_XML_Create_Node(ATLANTIS_GROUP_ATTRIBUTE, parent, FunctGroupArray[guild].groupCode, "", "", "0");
		}
	}
}


void InitSpeciesQ10ValuesXML(MSEBoxModel *bm, xmlNodePtr parent) {
	char valueStr[20];
	xmlNodePtr attributeGroupNode, attributeGroup;
	double Q10;
	int guild;

	/* Get the generic Q10 value */
	/* Temperature effects */
	attributeGroupNode = Util_XML_Get_Node(ATLANTIS_ATTRIBUTE_SUB_GROUP, parent->doc->children, "MetabolicQ10");
	if (attributeGroupNode == NULL)
		quit("MetabolicQ10 attribute group not found in biology input file.\n");
	attributeGroup = Util_XML_Get_Node(ATLANTIS_ATTRIBUTE_SUB_GROUP, attributeGroupNode, "Temperature_Effects");
	if (attributeGroup == NULL)
		quit("Temperature_Effects attribute group not found in biology input file.\n");

	/* Should really pass the fileName */

	/* Do Q10 first */
	Util_XML_Get_Value_Double("", ATLANTIS_ATTRIBUTE, bm->ecotest, 1, attributeGroup, no_checking, "Q10", &Q10);
	sprintf(valueStr, "%f", Q10);

	/* Create a node for each functional group*/
	for (guild = 0; guild < bm->K_num_tot_sp; guild++) {
		Util_XML_Create_Node(ATLANTIS_GROUP_ATTRIBUTE, parent, FunctGroupArray[guild].groupCode, "", "", valueStr);
	}
}

/**
 *	\brief Create the XML structure to hold the predator data.
 */
xmlNodePtr InitMammalXML(MSEBoxModel *bm, char *fileName, xmlNodePtr parent, char *tag, char *comment, char *units) {

	int guild, isMammal;
	xmlNodePtr newNode, attributeGroupNode, attributeGroup;

	newNode = Util_XML_Create_Node(ATLANTIS_ATTRIBUTE, parent, tag, comment, units, "");

	/* Temperature effects */
	attributeGroupNode = Util_XML_Get_Node(ATLANTIS_ATTRIBUTE_SUB_GROUP, parent->doc->children, "FunctionalGroupFlags");
	if (attributeGroupNode == NULL)
		quit("FunctionalGroupFlags attribute group not found in input file %s.\n", fileName);
	attributeGroup = Util_XML_Get_Node(ATLANTIS_ATTRIBUTE, attributeGroupNode, "IsMammal_Flag");
	if (attributeGroup == NULL)
		quit("IsMammal_Flag attribute group not found in input file %s.\n", fileName);

	/* Create a node for each functional group*/
	for (guild = 0; guild < bm->K_num_tot_sp; guild++) {

		if (FunctGroupArray[guild].isVertebrate == TRUE) {
			Util_XML_Get_Value_Integer(fileName, ATLANTIS_GROUP_ATTRIBUTE, bm->ecotest, 1, attributeGroup, no_checking, FunctGroupArray[guild].groupCode,
					&isMammal);

			if (isMammal == TRUE)
				Util_XML_Create_Node(ATLANTIS_GROUP_ATTRIBUTE, newNode, FunctGroupArray[guild].groupCode, "", "", "0");
		}
	}
	return newNode;
}

/**
 *	\brief Create the XML structure to hold the invertebrate predator data.
 */
xmlNodePtr InitInvertPredatorXML(MSEBoxModel *bm, xmlNodePtr parent, char *tag, char *comment, char *units) {

	int guild;
	xmlNodePtr newNode;

	newNode = Util_XML_Create_Node(ATLANTIS_ATTRIBUTE, parent, tag, comment, units, "");

	/* Create a node for each functional group*/
	for (guild = 0; guild < bm->K_num_tot_sp; guild++) {
		if (FunctGroupArray[guild].isVertebrate == FALSE && FunctGroupArray[guild].isPredator == TRUE) {
			Util_XML_Create_Node(ATLANTIS_GROUP_ATTRIBUTE, newNode, FunctGroupArray[guild].groupCode, "", "", "0.0");
		}
	}
	return newNode;
}

/**
 *	\brief Create the XML structure to hold the vertebrate seasonal distribution data.
 */
void InitBiomassSpeciesZeroValuesXML(MSEBoxModel *bm, xmlNodePtr parent) {

	int guild;

	/* Create a node for each functional group*/
	for (guild = 0; guild < bm->K_num_tot_sp; guild++) {
		if (FunctGroupArray[guild].groupAgeType == BIOMASS) {
			Util_XML_Create_Node(ATLANTIS_GROUP_ATTRIBUTE, parent, FunctGroupArray[guild].groupCode, "", "", "0.0");
		}
	}
}

/**
 *	\brief Create the XML structure to hold the vertebrate seasonal distribution data.
 */
void InitPrimaryProducerSpeciesZeroValuesXML(MSEBoxModel *bm, xmlNodePtr parent, char *tag, char *comment, char *units) {

	int guild;
	xmlNodePtr newNode;

	newNode = Util_XML_Create_Node(ATLANTIS_ATTRIBUTE, parent, tag, comment, units, "");

	/* Create a node for each functional group*/
	for (guild = 0; guild < bm->K_num_tot_sp; guild++) {
		if (FunctGroupArray[guild].groupAgeType == BIOMASS && FunctGroupArray[guild].isPrimaryProducer == TRUE){
			Util_XML_Create_Node(ATLANTIS_GROUP_ATTRIBUTE, newNode, FunctGroupArray[guild].groupCode, "", "", "0.0");
		}
	}
}

/**
 *	\brief Create the XML structure to hold the cultured species stocking distribution data.
 */
void Init_Cultured_Zero_ValuesXML(MSEBoxModel *bm, xmlNodePtr parent) {
    
	int guild;
    
	/* Create a node for each functional group*/
	for (guild = 0; guild < bm->K_num_tot_sp; guild++) {
		if (FunctGroupArray[guild].isCultured == TRUE) {
			Util_XML_Create_Node(ATLANTIS_GROUP_ATTRIBUTE, parent, FunctGroupArray[guild].groupCode, "", "", "0.0");
		}
	}
}

/**
 *	\brief Create the XML structure to hold the vertebrate seasonal distribution data.
 */
void Init_Vert_Species_Zero_ValuesXML(MSEBoxModel *bm, xmlNodePtr parent) {
	int guild;

	/* Create a node for each functional group*/
	for (guild = 0; guild < bm->K_num_tot_sp; guild++) {
		if (FunctGroupArray[guild].isVertebrate == TRUE) {
			Util_XML_Create_Node(ATLANTIS_GROUP_ATTRIBUTE, parent, FunctGroupArray[guild].groupCode, "", "", "0.0");
		}
	}
}


/**
 *    \brief Create the XML structure to hold basic age structured data.
 */
void Init_AgeStruct_Species_Zero_ValuesXML(MSEBoxModel *bm, xmlNodePtr parent) {
    int guild;
    
    /* Create a node for each functional group*/
    for (guild = 0; guild < bm->K_num_tot_sp; guild++) {
        if (FunctGroupArray[guild].numCohorts > 1) {
            Util_XML_Create_Node(ATLANTIS_GROUP_ATTRIBUTE, parent, FunctGroupArray[guild].groupCode, "", "", "0.0");
        }
    }
}

/**
 *	\brief Create the XML structure to hold the vertebrate seasonal distribution data.
 */
void InitSpeciesValuesXML(MSEBoxModel *bm, xmlNodePtr parent, char *valueStr) {

	int guild;

	/* Create a node for each functional group*/
	for (guild = 0; guild < bm->K_num_tot_sp; guild++) {
		Util_XML_Create_Node(ATLANTIS_GROUP_ATTRIBUTE, parent, FunctGroupArray[guild].groupCode, "", "", valueStr);
	}
}

/**
 *	\brief Create the XML structure to hold the vertebrate seasonal distribution data.
 */
void Init_Species_Zero_ValuesXML(MSEBoxModel *bm, xmlNodePtr parent) {

	InitSpeciesValuesXML(bm, parent, "0");
}

/**
 *	\brief Create a node for all groups with a value of 1.0
 */
void InitSpeciesOneValuesXML(MSEBoxModel *bm, xmlNodePtr parent) {

	InitSpeciesValuesXML(bm, parent, "1.0");
}

/**
 *	\brief Create the XML structure to hold the vertebrate seasonal distribution data.
 */
void InitAdJuvVertSpeciesXML(MSEBoxModel *bm, xmlNodePtr parent) {

	xmlNodePtr groupNode;
	int guild;

	/* Create a node for each functional group*/
	for (guild = 0; guild < bm->K_num_tot_sp; guild++) {
		if (FunctGroupArray[guild].isVertebrate == TRUE) {
			groupNode = Util_XML_Create_Node(ATLANTIS_GROUP_ATTRIBUTE, parent, FunctGroupArray[guild].groupCode, "", "", "");
			Util_XML_Create_Node(ATLANTIS_AGE_CLASS_ATTRIBUTE, groupNode, "adult", "", "", "");
			Util_XML_Create_Node(ATLANTIS_AGE_CLASS_ATTRIBUTE, groupNode, "juv", "", "", "");
		}
	}
}

/**
 Want the XML for these parameter to be:
 <StockStructure>
 <FPL>
 </FPL>
 </StockStructure>
 */
void SpeciesXMLFunction(MSEBoxModel *bm, char *fileName, xmlNodePtr parent, char *str, char *valueStr) {

	char speciesStr[50];
	char *strPtr;
	int speciesIndex;

	strcpy(speciesStr, str);
	strPtr = strchr(speciesStr, '_');
	*strPtr = '\0';
	speciesIndex = Util_Get_FG_Index_From_Token(bm, speciesStr, fileName, str, TRUE);

	Util_XML_Set_Node_Value(ATLANTIS_GROUP_ATTRIBUTE, parent, FunctGroupArray[speciesIndex].groupCode, valueStr);
}

/**
 */
void Species_Last_XMLFunction(MSEBoxModel *bm, char *fileName, xmlNodePtr parent, char *str, char *valueStr) {

	char *speciesStr;
	int speciesIndex;

	speciesStr = strchr(str, '_');
	speciesStr++;
	while (strchr(speciesStr, '_') != NULL) {
		speciesStr = strchr(speciesStr, '_');
		speciesStr++;
	}

	speciesIndex = Util_Get_FG_Index_From_Token(bm, speciesStr, fileName, str, TRUE);
	Util_XML_Set_Node_Value(ATLANTIS_GROUP_ATTRIBUTE, parent, FunctGroupArray[speciesIndex].groupCode, valueStr);
}

/**
 *	\brief Create the XML structure to hold the K_trans data.
 */
void InitKTransXML(MSEBoxModel *bm, xmlNodePtr parent) {
	int guild;

	/* Create a node for each functional group*/
	for (guild = 0; guild < bm->K_num_tot_sp; guild++) {
		if (FunctGroupArray[guild].groupType == SED_EP_FF) {
			Util_XML_Create_Node(ATLANTIS_GROUP_ATTRIBUTE, parent, FunctGroupArray[guild].groupCode, "", "", "");
		}
	}
}


/**
 *	\brief Create the XML structure to hold the invert food avail data.
 *
 *  THIS IS THE OLD VERSION BEFORE PUT AGE_STRUCTURED_BIOMASS INTO InitspVertEatXML()
 *
void InitspInvertEatXML(MSEBoxModel *bm, xmlNodePtr parent) {
    
	xmlNodePtr groupNode;
	int guild;
    
	// Create a node for each functional group
	for (guild = 0; guild < bm->K_num_tot_sp; guild++) {
		if (FunctGroupArray[guild].isVertebrate == FALSE && FunctGroupArray[guild].isPredator == TRUE) {
            
			groupNode = Util_XML_Create_Node(ATLANTIS_GROUP_ATTRIBUTE, parent, FunctGroupArray[guild].groupCode, "", "", "");
            
			if (FunctGroupArray[guild].groupAgeType == AGE_STRUCTURED_BIOMASS) {
				Util_XML_Create_Node(ATLANTIS_AGE_CLASS_ATTRIBUTE, groupNode, "adult", "", "", "");
				Util_XML_Create_Node(ATLANTIS_AGE_CLASS_ATTRIBUTE, groupNode, "juv", "", "", "");
			}
		}
	}
}
*/
/**
 *	\brief Create the XML structure to hold the invert food avail data.
 */
void InitspInvertEatXML(MSEBoxModel *bm, xmlNodePtr parent) {

	int guild;

	/* Create a node for each functional group*/
	for (guild = 0; guild < bm->K_num_tot_sp; guild++) {
		if ((FunctGroupArray[guild].groupAgeType == BIOMASS) && (FunctGroupArray[guild].isPredator == TRUE)) {
			Util_XML_Create_Node(ATLANTIS_GROUP_ATTRIBUTE, parent, FunctGroupArray[guild].groupCode, "", "", "");
		}
	}
}

/**
 *	\brief Create the XML structure to hold the invert food avail data.
 */
void InitspSeagrassXML(MSEBoxModel *bm, xmlNodePtr parent) {

    /* Old version of the code *
	xmlNodePtr groupNode;
	int guild, pred;

	// Create a node for each functional group*
	for (guild = 0; guild < bm->K_num_tot_sp; guild++) {
		if(FunctGroupArray[guild].groupType == SEAGRASS){
			groupNode = Util_XML_Create_Node(ATLANTIS_GROUP_ATTRIBUTE, parent, FunctGroupArray[guild].groupCode, "", "", "");
			for(pred = 0; pred < bm->K_num_tot_sp; pred++){
				Util_XML_Create_Node(ATLANTIS_AGE_CLASS_ATTRIBUTE, groupNode, FunctGroupArray[pred].groupCode, "", "", "");
			}
		}
	}
    */
    /* New version from Scratch */

    xmlNodePtr groupNode;
     int guild, stage;
    char stageString[10];

    /* Create a node for each functional group*/
    for (guild = 0; guild < bm->K_num_tot_sp; guild++) {
        if(FunctGroupArray[guild].groupType == SEAGRASS){
            groupNode = Util_XML_Create_Node(ATLANTIS_GROUP_ATTRIBUTE, parent, FunctGroupArray[guild].groupCode, "", "", "");
            for(stage = 0; stage < FunctGroupArray[guild].numStages; stage++){
                sprintf(stageString, "stage%d", stage);
                Util_XML_Create_Node(ATLANTIS_AGE_CLASS_ATTRIBUTE, groupNode, stageString, "", "", "");
            }
        }
    }

}



/**/
void SeagrassPreyXMLFunction(MSEBoxModel *bm, char *fileName, xmlNodePtr parent, char *str, char *valueStr) {

    /* Old version
    char *preyString = (char *)malloc(sizeof(char) * 500);
	char *predatorString;
	char *tempstr;
	int predatorIndex;
	int preyIndex;
	xmlNodePtr speciesNode;
    
	// Strip off the 'Seagrass_ bit from the front and store in the preyString variable.
	sscanf(str, "Seagrass_%s_pPREY", preyString);
    
	// Get the predator code from the end
	predatorString  = strstr(preyString, "pPREY") + strlen("pPREY");
    
	// terminate the prey string so we are just left with the group code
	tempstr = strstr(preyString, "_");
	*tempstr = '\0';
    
    
	// Get the indicies of each group
	predatorIndex = Util_Get_FG_Index_From_Token(bm, predatorString, fileName, str, TRUE);
	preyIndex = Util_Get_FG_Index_From_Token(bm, preyString, fileName, str, TRUE);
    
	speciesNode = Util_XML_Get_Or_Create_Node(ATLANTIS_GROUP_ATTRIBUTE, parent, FunctGroupArray[preyIndex].groupCode);
    
	Util_XML_Set_Node_Value(ATLANTIS_AGE_CLASS_ATTRIBUTE, speciesNode, FunctGroupArray[predatorIndex].groupCode, valueStr);
	free(preyString);
     
    */
    /* New version from Scratch */
    
    char seps[] = " ,\t";
    char *preyString = (char *)malloc(sizeof(char) * 500);
    char stageString[10];
    char finalValues[STRLEN];
    int stageIndex = 0;
    int preyIndex = 0;
    int sp;
    char *varStr;
    xmlNodePtr speciesNode;
    xmlNodePtr cohortNode;
    
    /* Strip off the 'Seagrass_ bit from the front and store in the preyString variable. */
    sscanf(str, "Seagrass_pPREY%s", preyString);
    
    /* See if there is a cohort value on the end */
    if (isdigit(preyString[strlen(preyString) - 1])) {
        stageIndex = atoi(preyString + strlen(preyString) - 1) - 1;
        preyString[strlen(preyString) - 1] = '\0';
    } else {
        quit("No stage string found for seagrass predation\n");
    }
    sprintf(stageString, "stage%d", stageIndex);
    
    strcpy(finalValues, "");

    /* Get values */
    for (sp = 0; sp < bm->K_num_tot_sp; sp++) {
        
        if (sp == 0)
            varStr = strtok(valueStr, seps);
        else
            varStr = strtok(NULL, seps);
        
        if(varStr == NULL){
            quit("spSeagrassPreyXMLFunction: Found %d values instead of the required %d for parameter %s\n", sp, (bm->K_num_tot_sp + bm->K_num_detritus), str);
        }
        strcat(finalValues, " ");
        strcat(finalValues, varStr);
    }
    trim(finalValues);
    
    
    /* Get the indicies of each group */
    preyIndex = Util_Get_FG_Index_From_Token(bm, preyString, fileName, str, TRUE);
    speciesNode = Util_XML_Get_Or_Create_Node(ATLANTIS_GROUP_ATTRIBUTE, parent, FunctGroupArray[preyIndex].groupCode);
    cohortNode = Util_XML_Get_Or_Create_Node(ATLANTIS_AGE_CLASS_ATTRIBUTE, speciesNode, stageString);
    
    Util_XML_Set_Node_Value(ATLANTIS_AGE_CLASS_ATTRIBUTE, cohortNode, stageString, finalValues);
    
    free(preyString);

    
}


/**
 *    \brief Create the XML structure to hold the supplemented diet data.
 */
void InitSuppDietXML(MSEBoxModel *bm, xmlNodePtr parent) {
    int guild;
    char *zeroString =  CreateZeroString(bm->K_num_cover_types);
    
    /* Create a node for each functional group*/
    for (guild = 0; guild < bm->K_num_tot_sp; guild++) {
        if (FunctGroupArray[guild].isSupplemented) {
            Util_XML_Create_Node(ATLANTIS_GROUP_ATTRIBUTE, parent, FunctGroupArray[guild].groupCode, "", "", "");
        }
    }
    free(zeroString);
}

/**
 * \brief Routine to read in the distribution of supplementary feed
 */
void SuppDietXMLFunction(MSEBoxModel *bm, char *fileName, xmlNodePtr parent, char *str, char *valueStr) {
    char *speciesStr;
    int speciesIndex;
    
    speciesStr = strchr(str, '_');
    speciesStr++;
    
    speciesIndex = Util_Get_FG_Index_From_Token(bm, speciesStr, fileName, str, TRUE);
    
    Util_XML_Set_Node_Value(ATLANTIS_GROUP_ATTRIBUTE, parent, FunctGroupArray[speciesIndex].groupCode, valueStr);
    
}

/**
 Want the XML for these parameter to be:
 <InvertebrateFoodAvail>
 <ZM>
 </ZM>
 </InvertebrateFoodAvail>
 */
void spInvertEatXMLFunction(MSEBoxModel *bm, char *fileName, xmlNodePtr parent, char *str, char *valueStr) {

	char seps[] = " ,\t";
	char speciesStr[20];
	int cohort = -1;
	char sedimentValues[1000];
	char finalValues[1000];
	int sp;
	char *varStr;
	int speciesIndex;
	xmlNodePtr speciesNode, attributeGroupNode, attributeNode;

    sscanf(str, "pPREY%s", speciesStr);
	strcpy(sedimentValues, "");
	strcpy(finalValues, "");

	/* See if there is a cohort value on the end */
	if (isdigit(speciesStr[strlen(speciesStr) - 1])) {
		cohort = atoi(speciesStr + strlen(speciesStr) - 1) - 1;
		speciesStr[strlen(speciesStr) - 1] = '\0';
	}

	speciesIndex = Util_Get_FG_Index_From_Token(bm, speciesStr, fileName, str, TRUE);

	if(cohort == juv_id && FunctGroupArray[speciesIndex].numCohorts < FunctGroupArray[speciesIndex].numStages){
		quit("spInvertEatXMLFunction - %s - Found a value for a juv cohort but %s has less cohorts than stages\n", str, FunctGroupArray[speciesIndex].groupCode);
	}

	if(cohort ==-1 && FunctGroupArray[speciesIndex].numCohorts > 1){
		quit("spInvertEatXMLFunction - %s - Age structured invertebrate groups must have cohort structured diet matrix values. You need a value per cohort.\n", FunctGroupArray[speciesIndex].groupCode);
	}

	/* Split the last 5 values off the end - ignore the last two values.*/
	for (sp = 0; sp < (bm->K_num_tot_sp + bm->K_num_detritus); sp++) {

		if (sp == 0)
			varStr = strtok(valueStr, seps);
		else
			varStr = strtok(NULL, seps);

		if(varStr == NULL){
			quit("spInvertEatXMLFunction: Found %d values instead of the required %d for parameter %s\n", sp, (bm->K_num_tot_sp + bm->K_num_detritus), str);
		}
		if (sp < bm->K_num_tot_sp) {
			strcat(finalValues, " ");
			strcat(finalValues, varStr);
		} else {
			strcat(sedimentValues, " ");
			strcat(sedimentValues, varStr);
		}
	}
	trim(finalValues);
	trim(sedimentValues);

	/* If this is a biomass group then the cohort value will be -1 so set the group attribute value.
	 * Else get or create the species node and set the age class attribute node to the finalValues string.
	 */
	if (FunctGroupArray[speciesIndex].numCohorts > 1){
		speciesNode = Util_XML_Get_Or_Create_Node(ATLANTIS_GROUP_ATTRIBUTE, parent, FunctGroupArray[speciesIndex].groupCode);

		Util_XML_Set_Node_Value(ATLANTIS_AGE_CLASS_ATTRIBUTE, speciesNode, cohortStrings[cohort], finalValues);
	} else {
		Util_XML_Set_Node_Value(ATLANTIS_GROUP_ATTRIBUTE, parent, FunctGroupArray[speciesIndex].groupCode, finalValues);
	}

	/* Now set the sediment detritus values.*/
	attributeGroupNode = parent->parent;
	attributeNode = Util_XML_Get_Or_Create_Node(ATLANTIS_ATTRIBUTE, attributeGroupNode, "DetritusSedimentFoodAvail");
	if (FunctGroupArray[speciesIndex].numCohorts > 1){

		speciesNode = Util_XML_Get_Or_Create_Node(ATLANTIS_GROUP_ATTRIBUTE, attributeNode, FunctGroupArray[speciesIndex].groupCode);
		Util_XML_Set_Node_Value(ATLANTIS_AGE_CLASS_ATTRIBUTE, speciesNode, cohortStrings[cohort], sedimentValues);
	} else {
		Util_XML_Set_Node_Value(ATLANTIS_GROUP_ATTRIBUTE, attributeNode, FunctGroupArray[speciesIndex].groupCode, sedimentValues);
	}
}

/**
 Want the XML for these parameter to be:
 <VertebrateFoodAvail>
 <FPL>
 <juv>
 <juvprey>
 </juvprey>
 <adultprey>
 </adultprey>
 </juv>
 </FPL>
 </VertebrateFoodAvail>
 */
void spVERTEatXMLFunction(MSEBoxModel *bm, char *fileName, xmlNodePtr parent, char *str, char *valueStr) {
	char tempStr[STRLEN];
	char speciesStr[STRLEN];
	int i, j, sp;
	char sedimentValues[STRLEN];
	char finalValues[STRLEN];
	char *tempValues = (char *) malloc(sizeof(char) * STRLEN);
	int speciesIndex;
	xmlNodePtr speciesNode;
	xmlNodePtr cohortNode;
	xmlNodePtr attributeGroupNode, attributeNode;
	char *zeroString =  CreateZeroString(bm->K_num_detritus);
	int size = bm->K_num_tot_sp + bm->K_num_detritus;
	double *p;
	double *tempArray;
	double *sedimentArray = (double *)malloc(sizeof(double) * (unsigned int long)bm->K_num_detritus);
	char lengthStr[50];
    
	sprintf(lengthStr, "%d", bm->K_num_tot_sp);

	strcpy(sedimentValues, "");
	strcpy(finalValues, "");

    sscanf(str, "pPREY%d%s", &j, speciesStr);

	i = atoi(speciesStr + strlen(speciesStr) - 1);
	*(speciesStr + strlen(speciesStr) - 1) = '\0';
	j--;
	i--;

	speciesIndex = Util_Get_FG_Index_From_Token(bm, speciesStr, fileName, str, TRUE);
	speciesNode = Util_XML_Get_Or_Create_Node(ATLANTIS_GROUP_ATTRIBUTE, parent, FunctGroupArray[speciesIndex].groupCode);
	cohortNode = Util_XML_Get_Or_Create_Node(ATLANTIS_AGE_CLASS_ATTRIBUTE, speciesNode, cohortStrings[i]);

	if(i ==-1 && FunctGroupArray[speciesIndex].numCohorts > 1){
		quit("spInvertEatXMLFunction - %s - Age structured invertebrate groups must have cohort structured diet matrix values\n", FunctGroupArray[speciesIndex].groupCode);
	}
	/* parse the data into an array */
	p = Parse_Into_Array(fileName, str, valueStr, size, proportion_check);

	/* Now grab the sediment values */
	for (sp = 0; sp < (bm->K_num_tot_sp + bm->K_num_detritus); sp++) {

		if (sp < bm->K_num_tot_sp) {
			sprintf(tempStr, "%s%.10e", sp>0?" ":"", p[sp]);
			strcat(finalValues, tempStr);

			//sprintf(finalValues, "%s%s%.10e", finalValues, sp>0?" ":"", p[sp]);
		} else {
			sedimentArray[sp - bm->K_num_tot_sp] = p[sp];
		}
	}
	trim(finalValues);

    /* Now find the node and set the value */
	Util_XML_Set_Node_Value(ATLANTIS_AGE_CLASS_ATTRIBUTE, cohortNode, cohortStrings[j], finalValues);

	/* Now set the sediment detritus values.*/
	attributeGroupNode = parent->parent;
	attributeNode = Util_XML_Get_Or_Create_Node(ATLANTIS_ATTRIBUTE, attributeGroupNode, "DetritusSedimentFoodAvail");
	speciesNode = Util_XML_Get_Or_Create_Node(ATLANTIS_GROUP_ATTRIBUTE, attributeNode, FunctGroupArray[speciesIndex].groupCode);

	/* See if there are already some values - if not use these one */
	Util_XML_Get_Value_String(fileName, ATLANTIS_AGE_CLASS_ATTRIBUTE, TRUE, speciesNode, cohortStrings[i], tempValues);

	/* Read the temp values into an array */
	tempArray = Parse_Into_Array(fileName, str, tempValues, bm->K_num_detritus, proportion_check);

	for(sp = 0; sp < bm->K_num_detritus; sp++){
		if(tempArray[sp] == 0){
			sprintf(tempStr, "%s%e", sp>0?" ":"", sedimentArray[sp]);
		}else{
			sprintf(tempStr, "%s%e", sp>0?" ":"", tempArray[sp]);
		}
		strcat(sedimentValues, tempStr);

	}
	trim(sedimentValues);

	Util_XML_Set_Node_Value(ATLANTIS_AGE_CLASS_ATTRIBUTE, speciesNode, cohortStrings[i], sedimentValues);


	/* Now set the length - this will override the value previously set */
	Util_XML_Set_Node_Property(parent, "ArrayLength", lengthStr);

	free(p);
	free(zeroString);
	free(tempValues);
	free(tempArray);
	free(sedimentArray);
}

/**
 *	\brief Create the XML structure to hold the detritus sediment diet availability,
 */
void InitSedimentFoodAvailXML(MSEBoxModel *bm, xmlNodePtr parent, char *tag, char *comment, char *units, int type, int length) {

	int guild;
	xmlNodePtr newNode;
	xmlNodePtr groupNode;
	char lengthStr[100];
	char *zeroString =  CreateZeroString(bm->K_num_detritus);

	sprintf(lengthStr, "%d", length);

	newNode = Util_XML_Create_Node(ATLANTIS_ATTRIBUTE, parent, tag, comment, units, "");
	Util_XML_Add_Node_Property(newNode, "AttributeType", AtlantisXMLAttributeTypeStrings[type]);
	Util_XML_Add_Node_Property(newNode, "ArrayLength", lengthStr);

	/* Create a node for each functional group*/
	for (guild = 0; guild < bm->K_num_tot_sp; guild++) {

		if (FunctGroupArray[guild].isVertebrate == TRUE || (FunctGroupArray[guild].isVertebrate == FALSE && FunctGroupArray[guild].isPredator == TRUE)) {

			groupNode = Util_XML_Create_Node(ATLANTIS_GROUP_ATTRIBUTE, newNode, FunctGroupArray[guild].groupCode, "", "", "");

			if (FunctGroupArray[guild].groupAgeType == AGE_STRUCTURED_BIOMASS || FunctGroupArray[guild].groupAgeType == AGE_STRUCTURED) {
				Util_XML_Create_Node(ATLANTIS_AGE_CLASS_ATTRIBUTE, groupNode, "adult", "", "", zeroString);
				Util_XML_Create_Node(ATLANTIS_AGE_CLASS_ATTRIBUTE, groupNode, "juv", "", "", zeroString);
			}
		}
	}
	free(zeroString);
}

/**
 *	\brief Create the XML structure to hold the vertical distribution data.
 */

//TODO: This XML structure might need to change.
void InitspVertEatXML(MSEBoxModel *bm, xmlNodePtr parent) {

	xmlNodePtr groupNode;
	//xmlNodePtr newnode;
	xmlNodePtr cohortNode;
	int guild;

	/* Create a node for each functional group*/
	for (guild = 0; guild < bm->K_num_tot_sp; guild++) {
		//if (FunctGroupArray[guild].isVertebrate == TRUE) {
        if ((FunctGroupArray[guild].groupAgeType == AGE_STRUCTURED) || (FunctGroupArray[guild].groupAgeType == AGE_STRUCTURED_BIOMASS)){

			groupNode = Util_XML_Create_Node(ATLANTIS_GROUP_ATTRIBUTE, parent, FunctGroupArray[guild].groupCode, "", "", "");

			cohortNode = Util_XML_Create_Node(ATLANTIS_AGE_CLASS_ATTRIBUTE, groupNode, "adult", "", "", "");
			Util_XML_Create_Node(ATLANTIS_AGE_CLASS_ATTRIBUTE, cohortNode, "adult", "", "", "");
			Util_XML_Create_Node(ATLANTIS_AGE_CLASS_ATTRIBUTE, cohortNode, "juv", "", "", "");

			cohortNode = Util_XML_Create_Node(ATLANTIS_AGE_CLASS_ATTRIBUTE, groupNode, "juv", "", "", "");
			Util_XML_Create_Node(ATLANTIS_AGE_CLASS_ATTRIBUTE, cohortNode, "adult", "", "", "");
			Util_XML_Create_Node(ATLANTIS_AGE_CLASS_ATTRIBUTE, cohortNode, "juv", "", "", "");
		}
	}
}

/**
 Want the XML for these parameter to be:
 <VerticalDistribution>
 <FPL>
 <day>
 <adult>0.0 0.0 0.0 0.2 0.8</adult>
 <juv>0.0 0.0 0.0 0.2 0.8</juv>
 </day>
 <night>
 <adult>0.0 0.0 0.0 0.2 0.8</adult>
 <juv>0.0 0.0 0.0 0.2 0.8</juv>
 </night>
 </FPL>
 </VerticalDistribution>


 */
void VERTXMLFunction(MSEBoxModel *bm, char *fileName, xmlNodePtr parent, char *str, char *valueStr) {
	char tempStr[50];
	char *speciesStr;
	int juvIndex = -1;
	char *lastLetter;
	char dayStr[12];
	xmlNodePtr speciesNode, tempNode;
	int speciesIndex;

	strcpy(tempStr, str);
	// Find out if this is day or night.
	if (strstr(tempStr, "day")) {
		sprintf(dayStr, "day");
	}else if(strstr(tempStr, "overwinter")) {
		sprintf(dayStr, "overwinter");
	} else {
		sprintf(dayStr, "night");
	}

	speciesStr = strchr(tempStr, '_') + 1;

	lastLetter = speciesStr + strlen(speciesStr) - 1;
	if (*lastLetter == '1') {
		juvIndex = 1;
		*lastLetter = '\0';
	} else if (*lastLetter == '2') {
		juvIndex = 2;
		*lastLetter = '\0';
	}

	// Get the speciesIndex;
	speciesIndex = Util_Get_FG_Index_From_Token(bm, speciesStr, fileName, str, TRUE);
	speciesNode = Util_XML_Get_Or_Create_Node(ATLANTIS_GROUP_ATTRIBUTE, parent, FunctGroupArray[speciesIndex].groupCode);

		/*  */
		switch (juvIndex) {
		case -1:
			Util_XML_Set_Node_Value(ATLANTIS_TEMPORAL_ATTRIBUTE, speciesNode, dayStr, valueStr);
			break;
		case 1:
			tempNode = Util_XML_Get_Or_Create_Node(ATLANTIS_TEMPORAL_ATTRIBUTE, speciesNode, dayStr);
			Util_XML_Set_Node_Value(ATLANTIS_AGE_CLASS_ATTRIBUTE, tempNode, "juv", valueStr);
			break;
		case 2:
			tempNode = Util_XML_Get_Or_Create_Node(ATLANTIS_TEMPORAL_ATTRIBUTE, speciesNode, dayStr);
			Util_XML_Set_Node_Value(ATLANTIS_AGE_CLASS_ATTRIBUTE, tempNode, "adult", valueStr);
			break;
		}
}

/**
 *	\brief Create the XML structure to hold the vertical distribution data.
 */
void InitVertDistributionXML(MSEBoxModel *bm, xmlNodePtr parent) {

	xmlNodePtr groupNode = NULL;
	xmlNodePtr tempNode;
	int guild;

	/* Create a node for each functional group*/
	for (guild = 0; guild < bm->K_num_tot_sp; guild++) {

		if (FunctGroupArray[guild].isMobile == TRUE && FunctGroupArray[guild].habitatType != EPIFAUNA){

			groupNode = Util_XML_Create_Node(ATLANTIS_GROUP_ATTRIBUTE, parent, FunctGroupArray[guild].groupCode, "", "", "");

			tempNode = Util_XML_Create_Node(ATLANTIS_TEMPORAL_ATTRIBUTE, groupNode, "day", "", "", "");
			/* If this is a vertebrate group add the adult and juv nodes*/
			if (FunctGroupArray[guild].isVertebrate) {
				Util_XML_Create_Node(ATLANTIS_AGE_CLASS_ATTRIBUTE, tempNode, "juv", "", "", "");
				Util_XML_Create_Node(ATLANTIS_AGE_CLASS_ATTRIBUTE, tempNode, "adult", "", "", "");
			}

			tempNode = Util_XML_Create_Node(ATLANTIS_TEMPORAL_ATTRIBUTE, groupNode, "night", "", "", "");
			/* If this is a vertebrate group add the adult and juv nodes*/
			if (FunctGroupArray[guild].isVertebrate) {
				Util_XML_Create_Node(ATLANTIS_AGE_CLASS_ATTRIBUTE, tempNode, "juv", "", "", "");
				Util_XML_Create_Node(ATLANTIS_AGE_CLASS_ATTRIBUTE, tempNode, "adult", "", "", "");
			}
		}
		/* If overwinter add that in too */
		if (FunctGroupArray[guild].isOverWinter == TRUE ){

			if(groupNode == NULL){
				groupNode = Util_XML_Create_Node(ATLANTIS_GROUP_ATTRIBUTE, parent, FunctGroupArray[guild].groupCode, "", "", "");
			}
			tempNode = Util_XML_Create_Node(ATLANTIS_TEMPORAL_ATTRIBUTE, groupNode, "overwinter", "", "", "");
			/* If this is a vertebrate group add the adult and juv nodes*/
			if (FunctGroupArray[guild].isVertebrate) {
				Util_XML_Create_Node(ATLANTIS_AGE_CLASS_ATTRIBUTE, tempNode, "juv", "", "", "");
				Util_XML_Create_Node(ATLANTIS_AGE_CLASS_ATTRIBUTE, tempNode, "adult", "", "", "");
			}
		}
	}
}

/**
 *	\brief Create the XML structure to hold the vertebrate seasonal distribution data.
 */
void InitVertSeasonalDistXML(MSEBoxModel *bm, xmlNodePtr parent) {

	xmlNodePtr groupNode;
	xmlNodePtr cohortNode;
	int guild;
	int cohort;
	int season;
	char str[100];

	/* Create a node for each functional group*/
	for (guild = 0; guild < bm->K_num_tot_sp; guild++) {
        if (FunctGroupArray[guild].groupAgeType == AGE_STRUCTURED || FunctGroupArray[guild].groupAgeType == AGE_STRUCTURED_BIOMASS) {
            groupNode = Util_XML_Create_Node(ATLANTIS_GROUP_ATTRIBUTE, parent, FunctGroupArray[guild].groupCode, "", "", "");

            for (cohort = 0; cohort <= 1; cohort++) {
                cohortNode = Util_XML_Create_Node(ATLANTIS_AGE_CLASS_ATTRIBUTE, groupNode, cohortStrings[cohort], "", "", "");
                for (season = 1; season <= FunctGroupArray[guild].numMoveEntries; season++) {
                    sprintf(str, "season%d", season);
                    Util_XML_Create_Node(ATLANTIS_TEMPORAL_ATTRIBUTE, cohortNode, str, "", "", "");
                }
            }
        } else if (FunctGroupArray[guild].sp_geo_move > 0) {
            groupNode = Util_XML_Create_Node(ATLANTIS_GROUP_ATTRIBUTE, parent, FunctGroupArray[guild].groupCode, "", "", "");
            cohortNode = Util_XML_Create_Node(ATLANTIS_AGE_CLASS_ATTRIBUTE, groupNode, "adult", "", "", "");

            for (season = 1; season <= FunctGroupArray[guild].numMoveEntries; season++) {
                sprintf(str, "season%d", season);
                Util_XML_Create_Node(ATLANTIS_TEMPORAL_ATTRIBUTE, cohortNode, str, "", "", "");
			}
		}
	}
}

/**
 Want the XML for these parameter to be:
 <SeasonalDistribution>
 <ZM>
 <adult>
 <season1>
 ..
 <season4>
 </adult>
 </ZM>
 </SeasonalDistribution>
 */
void VertSeasonalDistXMLFunction(MSEBoxModel *bm, char *fileName, xmlNodePtr parent, char *str, char *valueStr) {

	char speciesStr[20];
	int cohort = adult_id;
	int index;
	int speciesIndex;
	xmlNodePtr speciesNode, ageClassNode;
	char stockStr[100];

	sscanf(str, "F%s", speciesStr);

	if (strstr(speciesStr, "juv") != NULL) {
		cohort = juv_id;
		// Get rid of the 'juv' text on the end.
		*(speciesStr + strlen(speciesStr) - 3) = '\0';

		/* Add in some error checking */
		if (strstr(speciesStr, "juv") != NULL) {
			quit("ERROR: The parameter %s in your biology.prm file is not the correct format. Please edit and try again\n", str);

		}
	}
	index = atoi(strchr(speciesStr, '_') + 2);

	if (index < 0)
		quit("VertSeasonalDistXMLFunction - Failed to get the index value %s\n", speciesStr);

	/* End the string where we have the '_' */
	*(strchr(speciesStr, '_')) = '\0';

	speciesIndex = Util_Get_FG_Index_From_Token(bm, speciesStr, fileName, str, TRUE);

	if(cohort == juv_id && (FunctGroupArray[speciesIndex].numCohorts < FunctGroupArray[speciesIndex].numStages)){
		quit("VertSeasonalDistXMLFunction - %s - Found a value for a juv cohort but %s less cohorts than stages\n", str, FunctGroupArray[speciesIndex].groupCode);
	}

	speciesNode = Util_XML_Get_Or_Create_Node(ATLANTIS_GROUP_ATTRIBUTE, parent, FunctGroupArray[speciesIndex].groupCode);
	ageClassNode = Util_XML_Get_Or_Create_Node(ATLANTIS_AGE_CLASS_ATTRIBUTE, speciesNode, cohortStrings[cohort]);
	sprintf(stockStr, "season%d", index);
	Util_XML_Set_Node_Value(ATLANTIS_TEMPORAL_ATTRIBUTE, ageClassNode, stockStr, valueStr);
}

/**
 *	\brief Create the XML structure to hold the vertical distribution data.
 */
void InitDietAvailXML(MSEBoxModel *bm, xmlNodePtr parent) {

	xmlNodePtr groupNode;
	int guild, vertGuild;
	char *zeroString =  CreateZeroString(bm->K_num_max_cohort);

	/* Create a node for each functional group */
	for (vertGuild = 0; vertGuild < bm->K_num_tot_sp; vertGuild++) {
		if (FunctGroupArray[vertGuild].isVertebrate) {
			groupNode = Util_XML_Create_Node(ATLANTIS_GROUP_ATTRIBUTE, parent, FunctGroupArray[vertGuild].groupCode, "", "", "");

			for (guild = 0; guild < bm->K_num_tot_sp; guild++) {

				if (FunctGroupArray[guild].groupType == LG_ZOO || FunctGroupArray[guild].groupType == MED_ZOO || FunctGroupArray[guild].groupType == SM_ZOO || FunctGroupArray[guild].groupType
						== LG_INF || FunctGroupArray[guild].groupType == SED_EP_FF || FunctGroupArray[guild].groupType == SED_EP_OTHER
						|| FunctGroupArray[guild].groupType == MOB_EP_OTHER) {

					Util_XML_Create_Node(ATLANTIS_GROUP_ATTRIBUTE, groupNode, FunctGroupArray[guild].groupCode, "", "", zeroString);
				}
			}
		}
	}
	free(zeroString);
}

/**
 Want the XML for these parameter to be:
 <AgeDietAvail>
 <FPL>
 <ZM></ZM>
 </FPL>
 </AgeDietAvail>
 */
void AgeDietXMLFunction(MSEBoxModel *bm, char *fileName, xmlNodePtr parent, char *str, char *valueStr) {

	char tempStr[50];
	char *speciesStr;
	char *strPtr;
	xmlNodePtr speciesNode;

	// copy the key to the key string.
	strcpy(tempStr, str);
	speciesStr = str + strlen(str) - 3;
	strcpy(tempStr, speciesStr);

	*speciesStr = '\0';
	strPtr = strchr(str, '_');
	strPtr++;

	/* Find the parent node in the tree*/
	speciesNode = Util_XML_Get_Or_Create_Node(ATLANTIS_GROUP_ATTRIBUTE, parent, tempStr);

	Util_XML_Set_Node_Value(ATLANTIS_GROUP_ATTRIBUTE, speciesNode, strPtr, valueStr);
}
/**
 Want the XML for these parameter to be:
 <ClearanceRate>
 <FPL>
 </FPL>
 </ClearanceRate>

 NOTE: This will only work if the adult values for CEP and PWN are before the j values.
 */
void InvertebrateClearanceRateXMLFunction(MSEBoxModel *bm, char *fileName, xmlNodePtr parent, char *str, char *valueStr) {

	char speciesStr[50];
	char *strPtr;
	int cohort = adult_id;
	int speciesIndex;

	strcpy(speciesStr, str);
	strPtr = strchr(str, '_');
	strPtr++;

	if (strstr(strPtr, "_T15") != NULL)
		strPtr[strlen(strPtr) - 4] = '\0';

	if (strstr(speciesStr, "j") != NULL) {
		strPtr++;
		cohort = juv_id;
	}

	if(bm->flag_macro_model){
		/* Allow for seagrass cohorts as well */
		if (strstr(speciesStr, "Epi_") != NULL) {
			strPtr+= 4;
			cohort = epiphyte_biomass_id;
		}
		else if (strstr(speciesStr, "Root_") != NULL) {
			strPtr+= 5;
			cohort = below_ground_biomass_id;
		}
		else{
			cohort = main_biomass_id;
		}
	}

	speciesIndex = Util_Get_FG_Index_From_Token(bm, strPtr, fileName, str, TRUE);

	if(bm->flag_macro_model && FunctGroupArray[speciesIndex].groupType == SEAGRASS){
		Util_XML_Replace_Node_Value(ATLANTIS_GROUP_ATTRIBUTE, parent, FunctGroupArray[speciesIndex].groupCode, valueStr, num_macrophyte_indices, cohort);
	}else{

		/** if this group is an age structured biomass group then we want to either replace the juv or adult value with the new value */
		if (FunctGroupArray[speciesIndex].groupAgeType == AGE_STRUCTURED_BIOMASS)
			Util_XML_Replace_Node_Value(ATLANTIS_GROUP_ATTRIBUTE, parent, FunctGroupArray[speciesIndex].groupCode, valueStr, 2, cohort);
		else{
			//if(cohort != juv_id)
				Util_XML_Set_Node_Value(ATLANTIS_GROUP_ATTRIBUTE, parent, FunctGroupArray[speciesIndex].groupCode, valueStr);
		}
	}
}

/**
 * \brief Create the XML structure to hold the rugosity data
 */
void InitRugosityRateXML(MSEBoxModel *bm, xmlNodePtr parent) {

	int guild;
	char *zeroString;

	/* Create a node for each functional group*/
	for (guild = 0; guild < bm->K_num_tot_sp; guild++) {
		if ((FunctGroupArray[guild].groupType == CORAL) || (FunctGroupArray[guild].groupType == SPONGE)) {
			if(FunctGroupArray[guild].numStages > 1){
				zeroString = CreateZeroString(FunctGroupArray[guild].numStages);
				Util_XML_Create_Node(ATLANTIS_GROUP_ATTRIBUTE, parent, FunctGroupArray[guild].groupCode, "", "", zeroString);
				free(zeroString);
			}else
			{
				 Util_XML_Create_Node(ATLANTIS_GROUP_ATTRIBUTE, parent, FunctGroupArray[guild].groupCode, "", "", "0");

			}
		}
	}

}

/**
 * \brief Create the XML structure to hold the linear mortality data
 */
void InitMortalityRateXML(MSEBoxModel *bm, xmlNodePtr parent) {

	int guild;
	char *zeroString;

	/* Create a node for each functional group*/
	for (guild = 0; guild < bm->K_num_tot_sp; guild++) {
		if (FunctGroupArray[guild].isDetritus == FALSE) {
			if(FunctGroupArray[guild].numStages > 1){
				zeroString = CreateZeroString(FunctGroupArray[guild].numStages);
				Util_XML_Create_Node(ATLANTIS_GROUP_ATTRIBUTE, parent, FunctGroupArray[guild].groupCode, "", "", zeroString);
				free(zeroString);
			}else
			{
				 Util_XML_Create_Node(ATLANTIS_GROUP_ATTRIBUTE, parent, FunctGroupArray[guild].groupCode, "", "", "0");

			}
		}
	}

}

/**
 *	\brief Static seabird and mammal induced mortality of each fish group, per quarter d-1
 */
void InitImplicitMortalityRateXML(MSEBoxModel *bm, xmlNodePtr parent) {

	int guild;
	char *zeroString = CreateZeroString(4);  // As quarterly

	/* Create a node for each functional group*/
	for (guild = 0; guild < bm->K_num_tot_sp; guild++) {
		if (FunctGroupArray[guild].isVertebrate == TRUE) {
			Util_XML_Create_Node(ATLANTIS_GROUP_ATTRIBUTE, parent, FunctGroupArray[guild].groupCode, "", "", zeroString);
		}
	}
	free(zeroString);
}

void ImplicitMortalityRateXMLFunction(MSEBoxModel *bm, char *fileName, xmlNodePtr parent, char *str, char *valueStr) {

	char *speciesStr;
	int speciesIndex;

	if(strlen(str) == 9){
		speciesStr = str + 6;
	}else{
		speciesStr = str + 5;
	}

	speciesIndex = Util_Get_FG_Index_From_Token(bm, speciesStr, fileName, str, TRUE);
	Util_XML_Set_Node_Value(ATLANTIS_GROUP_ATTRIBUTE, parent, FunctGroupArray[speciesIndex].groupCode, valueStr);
}

/**
 *	\brief Create the XML structure to hold the vertical distribution data.
 */
void InitMigrateIOBoxXML(MSEBoxModel *bm, xmlNodePtr parent) {

	xmlNodePtr groupNode, cohortNode;
	int guild, cohort, migrate;
	char migrateString[20];
	char *zeroString = CreateZeroString(bm->nbox);

	/* Create a node for each functional group*/
	for (guild = 0; guild < bm->K_num_tot_sp; guild++) {
		if (FunctGroupArray[guild].sp_geo_move == TRUE) {
			groupNode = Util_XML_Create_Node(ATLANTIS_GROUP_ATTRIBUTE, parent, FunctGroupArray[guild].groupCode, "", "", "");

			switch (FunctGroupArray[guild].groupAgeType) {
			case AGE_STRUCTURED_BIOMASS:
			case AGE_STRUCTURED:
				for (cohort = 0; cohort < FunctGroupArray[guild].numStages; cohort++) {
					cohortNode = Util_XML_Create_Node(ATLANTIS_AGE_CLASS_ATTRIBUTE, groupNode, cohortStrings[cohort], "", "", "");

					for (migrate = 0; migrate < K_max_num_mig; migrate++) {
						sprintf(migrateString, "Migrate%d", migrate + 1);
						Util_XML_Create_Node(
								ATLANTIS_ATTRIBUTE,
								cohortNode,
								migrateString,
								"",
								"",
								zeroString);
					}
				}
				break;
			case BIOMASS:
				for (migrate = 0; migrate < K_max_num_mig; migrate++) {
					sprintf(migrateString, "Migrate%d", migrate + 1);
					Util_XML_Create_Node(
							ATLANTIS_ATTRIBUTE,
							groupNode,
							migrateString,
							"",
							"",
							zeroString);
				}

				break;
			}
		}
	}
	free(zeroString);
}

/**
 Want the XML for these parameter to be:
 <MigrateIOBox>
 <ZL></ZL>
 <FPL>
 <adult></adult>
 <juv></juv>
 </FPL>
 </ImplicitMortalityRate>
 */
void MigrateReturnXMLFunction(MSEBoxModel *bm, char *fileName, xmlNodePtr parent, char *str, char *valueStr) {

	char *strPtr;
	char speciesStr[50];
	int adultIndex = -1; // default value.
	int speciesIndex;
	char param[30];
	xmlNodePtr paramNode, speciesNode;

	strcpy(speciesStr, str);
	strPtr = strchr(speciesStr, '_');
	*strPtr = '\0';
	// Point it back to the front.
	strPtr = speciesStr;

	// Check to see if there is a j present.
	if (*strPtr == 'j') {
		adultIndex = juv_id;
		strPtr++;
	} else {
		adultIndex = adult_id;
	}
	speciesIndex = Util_Get_FG_Index_From_Token(bm, strPtr, fileName, str, TRUE);

	if (strstr(str, "Migrate_Time") != NULL) {
		sprintf(param, "MigrateTime");
	} else if (strstr(str, "Migrate_Return") != NULL) {
		sprintf(param, "MigrateReturn");
	} else if (strstr(str, "Period") != NULL) {
		sprintf(param, "MigratePeriod");
    } else if (strstr(str, "YearsAway") != NULL) {
        sprintf(param, "MigrateYearsAway");
    } else if (strstr(str, "ReturnStock") != NULL) {
		sprintf(param, "MigrateStockReturn");
	} else if (strstr(str, "FSMG") != NULL) {
		sprintf(param, "MigrateProportionIncrease");
	} else if (strstr(str, "FSM") != NULL) {
		sprintf(param, "MigrateSurvivorshipProportion");
	}

	paramNode = Util_XML_Get_Or_Create_Node(ATLANTIS_ATTRIBUTE, parent->parent, param);

	/* If this is a biomass group just create a species attribute node - else create a species node and then add a child age class
	 * attribute node with the value.
	 */
	if (FunctGroupArray[speciesIndex].groupAgeType == BIOMASS) {
		Util_XML_Set_Node_Value(ATLANTIS_GROUP_ATTRIBUTE, paramNode, FunctGroupArray[speciesIndex].groupCode, valueStr);
	} else {
		speciesNode = Util_XML_Get_Or_Create_Node(ATLANTIS_GROUP_ATTRIBUTE, paramNode, FunctGroupArray[speciesIndex].groupCode);
		Util_XML_Set_Node_Value(ATLANTIS_AGE_CLASS_ATTRIBUTE, speciesNode, cohortStrings[adultIndex], valueStr);
	}
}

/**
 * FLAG_MIGRATE_RETURN
 */

void InitMigrateXML(MSEBoxModel *bm, xmlNodePtr parent) {

	xmlNodePtr groupNode;
	int guild;

	/* Create a node for each functional group*/
	for (guild = 0; guild < bm->K_num_tot_sp; guild++) {
		if (FunctGroupArray[guild].sp_geo_move == TRUE) {

			groupNode = Util_XML_Create_Node(ATLANTIS_GROUP_ATTRIBUTE, parent, FunctGroupArray[guild].groupCode, "", "", "");

			switch (FunctGroupArray[guild].groupAgeType) {
			case AGE_STRUCTURED_BIOMASS:
			case AGE_STRUCTURED:
				Util_XML_Create_Node(ATLANTIS_AGE_CLASS_ATTRIBUTE, groupNode, "adult", "", "", "");
				Util_XML_Create_Node(ATLANTIS_AGE_CLASS_ATTRIBUTE, groupNode, "juv", "", "", "");
				break;
			case BIOMASS:
				/* Do nothing */
				break;
			}
		}
	}
}

/**
 * KMIG_INVERT
 */

void InitKMIGInvertXML(MSEBoxModel *bm, xmlNodePtr parent) {

	int guild;
	char *zeroString;

	/* Create a node for each functional group*/
	for (guild = 0; guild < bm->K_num_tot_sp; guild++) {
		if (FunctGroupArray[guild].isVertebrate == FALSE && FunctGroupArray[guild].sp_geo_move == TRUE) {

			if(FunctGroupArray[guild].numCohorts > 1){
				zeroString = CreateZeroString(FunctGroupArray[guild].numCohorts);
				Util_XML_Create_Node(ATLANTIS_GROUP_ATTRIBUTE, parent, FunctGroupArray[guild].groupCode, "", "", zeroString);
				free(zeroString);
			} else {
				 Util_XML_Create_Node(ATLANTIS_GROUP_ATTRIBUTE, parent, FunctGroupArray[guild].groupCode, "", "", "0");

			}
		}
	}
}

/**
 * FLAG_MIGRATE_RETURN
 */

void InitStockMigrateXML(MSEBoxModel *bm, xmlNodePtr parent) {

	xmlNodePtr groupNode;
	int guild;

	/* Create a node for each functional group*/
	for (guild = 0; guild < bm->K_num_tot_sp; guild++) {
		if (FunctGroupArray[guild].sp_geo_move == TRUE) {
			groupNode = Util_XML_Create_Node(ATLANTIS_GROUP_ATTRIBUTE, parent, FunctGroupArray[guild].groupCode, "", "", "");
			switch (FunctGroupArray[guild].groupAgeType) {
			case AGE_STRUCTURED:
				Util_XML_Create_Node(ATLANTIS_AGE_CLASS_ATTRIBUTE, groupNode, "adult", "", "", "");
				Util_XML_Create_Node(ATLANTIS_AGE_CLASS_ATTRIBUTE, groupNode, "juv", "", "", "");
				break;
			case BIOMASS:
			case AGE_STRUCTURED_BIOMASS:
				/* Do nothing */
				break;
			}
		}
	}
}

/**
 Want the XML for these parameter to be:
 <MigrateIOBox>
 <ZL></ZL>
 <FPL>
 <adult></adult>
 <juv></juv>
 </FPL>
 </ImplicitMortalityRate>
 */
void MigrateIOBoxXMLFunction(MSEBoxModel *bm, char *fileName, xmlNodePtr parent, char *str, char *valueStr) {

	char tempStr[50];
	char *strPtr, *speciesStr, *lastLetter;
	int index = 0;
	int adultIndex = -1; // default value.
	xmlNodePtr speciesNode, ageNode;
	int speciesIndex;
	char migrateString[20];

	// Copy the str into the key string.
	strcpy(tempStr, str);

	speciesStr = strchr(tempStr, '_');
	speciesStr++;

	strPtr = strstr(speciesStr, "ad");
	if (strPtr != NULL) {
		adultIndex = adult_id;
		*strPtr = '\0';
	}

	strPtr = strstr(speciesStr, "juv");
	if (strPtr != NULL) {
		adultIndex = juv_id;
		*strPtr = '\0';
	}

	// Check the last letter is not a number.
	lastLetter = speciesStr + strlen(speciesStr) - 1;
	if (*lastLetter == '2') {
		index = 1;
		*lastLetter = '\0';
	} else if (*lastLetter == '3') {
		index = 2;
		*lastLetter = '\0';
	}

	speciesIndex = Util_Get_FG_Index_From_Token(bm, speciesStr, fileName, str, TRUE);
	speciesNode = Util_XML_Get_Or_Create_Node(ATLANTIS_GROUP_ATTRIBUTE, parent, FunctGroupArray[speciesIndex].groupCode);

	if(FunctGroupArray[speciesIndex].numStages > 1 && adultIndex == -1){
		quit("Parameter %s is not a valid parameter. If a group has more than 1 stage then juv and ad values should be provided\n", str);
	}

	sprintf(migrateString, "Migrate%d", index + 1);
	if (adultIndex != -1) {
		ageNode = Util_XML_Get_Or_Create_Node(ATLANTIS_AGE_CLASS_ATTRIBUTE, speciesNode, cohortStrings[adultIndex]);
		Util_XML_Set_Node_Value(ATLANTIS_ATTRIBUTE, ageNode, migrateString, valueStr);

	} else {
		Util_XML_Set_Node_Value(ATLANTIS_ATTRIBUTE, speciesNode, migrateString, valueStr);
	}
}

void KMIGXMLFunction(MSEBoxModel *bm, char *fileName, xmlNodePtr parent, char *str, char *valueStr) {

	char tempStr[20];

	strcpy(tempStr, str);
	if(strstr(tempStr, "rn") != NULL || strstr(tempStr, "sn") != NULL  )
		tempStr[strlen(tempStr) - 2] = '\0';

	Species_Last_XMLFunction(bm, fileName, parent, tempStr, valueStr);
}


/**
 *	\brief Create the XML structure to hold the habitat preference data.
 */
void InitHabitatXML(MSEBoxModel *bm, xmlNodePtr parent) {

	xmlNodePtr groupNode;
	int guild;
	char *zeroString =  CreateZeroString(bm->K_num_cover_types);

	/* Create a node for each functional group*/
	for (guild = 0; guild < bm->K_num_tot_sp; guild++) {
		switch (FunctGroupArray[guild].groupAgeType) {
		case BIOMASS: /* Do nothing */
			//if(FunctGroupArray[guild].isImpacted == TRUE){
				//Util_XML_Create_Node(ATLANTIS_GROUP_ATTRIBUTE, parent, FunctGroupArray[guild].groupCode, "", "", zeroString);
				Util_XML_Create_Node(ATLANTIS_GROUP_ATTRIBUTE, parent, FunctGroupArray[guild].groupCode, "", "", "");

			//}
			break;
		case AGE_STRUCTURED_BIOMASS: /* Intentional follow through */
		case AGE_STRUCTURED:
			groupNode = Util_XML_Create_Node(ATLANTIS_GROUP_ATTRIBUTE, parent, FunctGroupArray[guild].groupCode, "", "", "");
			Util_XML_Create_Node(ATLANTIS_AGE_CLASS_ATTRIBUTE, groupNode, "adult", "", "", zeroString);
			Util_XML_Create_Node(ATLANTIS_AGE_CLASS_ATTRIBUTE, groupNode, "juv", "", "", zeroString);
			break;
		}
	}
	free(zeroString);
}

/**
 Want the XML for these parameter to be:
 <habitat>
 <FPL>
 <adult\>
 <juv\>
 </FPL>
 </habitat>

 The values in the biology input file are in a predefined order. The values written to the output file
 are in the order of the cover groups as they are defined in the functional group input file.

 TODO: Get rid of the specific code.
 */
void HabitatXMLFunction(MSEBoxModel *bm, char *fileName, xmlNodePtr parent, char *str, char *valueStr) {

	char *speciesStr, *temp;
	int adultIndex = -1;
	int speciesIndex;
	xmlNodePtr speciesNode;
    
	if (strstr(str, "juv") != NULL) {
		adultIndex = juv_id;
	} else if (strstr(str, "ad") != NULL) {
		adultIndex = adult_id;
	}
	speciesStr = strchr(str, '_');
	speciesStr++;

	// Check to see if there is another '_' - if so get the string after that.
	temp = strchr(speciesStr, '_');
	if (temp != NULL) {
		speciesStr = temp;
		speciesStr++;
	}
	speciesIndex = Util_Get_FG_Index_From_Token(bm, speciesStr, fileName, str, TRUE);

	if (adultIndex == -1) {
		//Util_XML_Create_Node(ATLANTIS_GROUP_ATTRIBUTE, parent, FunctGroupArray[speciesIndex].groupCode, "", "", valueStr);

		Util_XML_Set_Node_Value(ATLANTIS_GROUP_ATTRIBUTE, parent, FunctGroupArray[speciesIndex].groupCode, valueStr);
	} else {
		/* Check that there should be cohort level info */
		if(FunctGroupArray[speciesIndex].numCohorts == 1){
			quit("Error: Parameter %s - there is only a single cohort for this group\n", str);
		}
		speciesNode = Util_XML_Get_Or_Create_Node(ATLANTIS_GROUP_ATTRIBUTE, parent, FunctGroupArray[speciesIndex].groupCode);
		Util_XML_Set_Node_Value(ATLANTIS_AGE_CLASS_ATTRIBUTE, speciesNode, cohortStrings[adultIndex], valueStr);
		//Util_XML_Create_Node(ATLANTIS_AGE_CLASS_ATTRIBUTE, speciesNode, cohortStrings[adultIndex], "", "", valueStr);
	}

}


/**
 *	\brief Create the XML structure to hold the vertebrate seasonal distribution data.
 */
void InitIceHabitatXML(MSEBoxModel *bm, xmlNodePtr parent) {

	xmlNodePtr groupNode;
	int guild;
	char *zeroString =  CreateZeroString(bm->K_num_ice_classes);

	/* Create a node for each functional group*/
	for (guild = 0; guild < bm->K_num_tot_sp; guild++) {
		switch (FunctGroupArray[guild].groupAgeType) {
		case BIOMASS: /* Do nothing */
			//if(FunctGroupArray[guild].isImpacted == TRUE){
				Util_XML_Create_Node(ATLANTIS_GROUP_ATTRIBUTE, parent, FunctGroupArray[guild].groupCode, "", "", zeroString);
			//}
			break;
		case AGE_STRUCTURED_BIOMASS: /* Intentional follow through */
		case AGE_STRUCTURED:
			groupNode = Util_XML_Create_Node(ATLANTIS_GROUP_ATTRIBUTE, parent, FunctGroupArray[guild].groupCode, "", "", "");
			Util_XML_Create_Node(ATLANTIS_AGE_CLASS_ATTRIBUTE, groupNode, "adult", "", "", zeroString);
			Util_XML_Create_Node(ATLANTIS_AGE_CLASS_ATTRIBUTE, groupNode, "juv", "", "", zeroString);
			break;
		}
	}
	free(zeroString);
}

/**
 Want the XML for these parameter to be:
 <habitat>
 <FPL>
 <adult\>
 <juv\>
 </FPL>
 </habitat>

 The values in the biology input file are in a predefined order. The values written to the output file
 are in the order of the cover groups as they are defined in the functional group input file.

 TODO: Get rid of the specific code.
 */
void IceHabitatXMLFunction(MSEBoxModel *bm, char *fileName, xmlNodePtr parent, char *str, char *valueStr) {

	char *speciesStr, *temp;
	int adultIndex = -1;
	int speciesIndex;
	xmlNodePtr speciesNode;

	if (strstr(str, "juv") != NULL) {
		adultIndex = juv_id;
		str = str + strlen("juv_");
	} else if (strstr(str, "ad") != NULL) {
		adultIndex = adult_id;
		str = str + strlen("ad_");
	}
	speciesStr = strchr(str, '_');
	speciesStr++;

	speciesStr = strchr(speciesStr, '_');
	speciesStr++;

	// Check to see if there is another '_' - if so get the string after that.
	temp = strchr(speciesStr, '_');
	if (temp != NULL) {
		speciesStr = temp;
		speciesStr++;
	}
	speciesIndex = Util_Get_FG_Index_From_Token(bm, speciesStr, fileName, str, TRUE);

	if (adultIndex == -1) {
		//Util_XML_Create_Node(ATLANTIS_GROUP_ATTRIBUTE, parent, FunctGroupArray[speciesIndex].groupCode, "", "", valueStr);

		Util_XML_Set_Node_Value(ATLANTIS_GROUP_ATTRIBUTE, parent, FunctGroupArray[speciesIndex].groupCode, valueStr);
	} else {
		/* Check that there should be cohort level info */
		if(FunctGroupArray[speciesIndex].numCohorts == 1){
			quit("Error: Parameter %s - there is only a single cohort for this group\n", str);
		}
		speciesNode = Util_XML_Get_Or_Create_Node(ATLANTIS_GROUP_ATTRIBUTE, parent, FunctGroupArray[speciesIndex].groupCode);
		Util_XML_Set_Node_Value(ATLANTIS_AGE_CLASS_ATTRIBUTE, speciesNode, cohortStrings[adultIndex], valueStr);
		//Util_XML_Create_Node(ATLANTIS_AGE_CLASS_ATTRIBUTE, speciesNode, cohortStrings[adultIndex], "", "", valueStr);
	}

}

/**
 *	\brief Create the XML structure to hold the popratio stock data.
 *
 *
 *	<popratioStock>
 *		<FPL>
 *			<cohort1>
 *				<stock1>
 *				</stock1>
 *			</cohor1>
 *		</FPL>
 *
 *	</popratioStock>
 *
 */
void InitPopRatioStockXML(MSEBoxModel *bm, xmlNodePtr parent) {

	xmlNodePtr groupNode;
	xmlNodePtr cohortNode;
	int guild, cohort, stock;
	char str[50];

	/* Create a node for each functional group*/
	for (guild = 0; guild < bm->K_num_tot_sp; guild++) {
		if (FunctGroupArray[guild].isVertebrate == TRUE) {
			groupNode = Util_XML_Create_Node(ATLANTIS_GROUP_ATTRIBUTE, parent, FunctGroupArray[guild].groupCode, "", "", "");

			for (cohort = 0; cohort < FunctGroupArray[guild].numCohorts; cohort++) {

				sprintf(str, "cohort%d", cohort + 1);
				cohortNode = Util_XML_Create_Node(ATLANTIS_COHORT_ATTRIBUTE, groupNode, str, "", "", "");

				for (stock = 0; stock < bm->K_num_stocks_per_sp; stock++) {
					sprintf(str, "stock%d", stock + 1);
					Util_XML_Create_Node(ATLANTIS_ATTRIBUTE, cohortNode, str, "", "", "");
				}
			}
		}
	}
}

void PopRatioStockXMLFunction(MSEBoxModel *bm, char *fileName, xmlNodePtr parent, char *str, char *valueStr) {

	int cohortIndex, stockIndex;
	char *lastLetter;
	char *secondlastLetter;
	char speciesStr[50];
	int i;
	char step1Str[50];
	char tempStr[50];
	char cohortString[100], attributeName[100];
	int speciesIndex;
	xmlNodePtr speciesNode, cohortNode;

	// Find stock index
	strcpy(tempStr, str);
	strcpy(step1Str, str);
	lastLetter = step1Str + strlen(step1Str) - 1;
	secondlastLetter = step1Str + strlen(step1Str) - 2;

	if(isdigit(secondlastLetter[0])) {  // Is the first digit a number if yes then a two digit case
		stockIndex = atoi(secondlastLetter);
		tempStr[strlen(secondlastLetter) - strlen("popratioStock") - 2] = '\0';
	} else {  // No make it a single string case
		stockIndex = atoi(lastLetter);
		tempStr[strlen(tempStr) - strlen("popratioStock") - 1] = '\0';
	}

	// Get the species string section of the string.
	for (i = 0; isalpha(tempStr[i]); i++) {
		speciesStr[i] = tempStr[i];
	}
	speciesStr[i] = '\0';
	cohortIndex = atoi(tempStr + i);

	speciesIndex = Util_Get_FG_Index_From_Token(bm, speciesStr, fileName, str, TRUE);
	speciesNode = Util_XML_Get_Or_Create_Node(ATLANTIS_GROUP_ATTRIBUTE, parent, FunctGroupArray[speciesIndex].groupCode);

	sprintf(cohortString, "cohort%d", cohortIndex);
	cohortNode = Util_XML_Get_Or_Create_Node(ATLANTIS_COHORT_ATTRIBUTE, speciesNode, cohortString);

	sprintf(attributeName, "stock%d", stockIndex);
	Util_XML_Set_Node_Value(ATLANTIS_ATTRIBUTE, cohortNode, attributeName, valueStr);

}

void pStockXMLFunction(MSEBoxModel *bm, char *fileName, xmlNodePtr parent, char *str, char *valueStr) {

	char *speciesStr;
	int juvIndex = adult_id;
	xmlNodePtr speciesNode;
	int speciesIndex;

	speciesStr = strchr(str, '_');
	speciesStr++;

	// Check to see if this is a juv value.
	if (*speciesStr == 'j') {
		juvIndex = juv_id;
		speciesStr++;
	}
	speciesIndex = Util_Get_FG_Index_From_Token(bm, speciesStr, fileName, str, TRUE);
	speciesNode = Util_XML_Get_Or_Create_Node(ATLANTIS_GROUP_ATTRIBUTE, parent, FunctGroupArray[speciesIndex].groupCode);
	Util_XML_Set_Node_Value(ATLANTIS_AGE_CLASS_ATTRIBUTE, speciesNode, cohortStrings[juvIndex], valueStr);
}


void TimeSpawnFunction(MSEBoxModel *bm, char *fileName, xmlNodePtr parent, char *str, char *valueStr) {

	char *speciesStr;
	char tempStr[50];
	int speciesIndex;

	strcpy(tempStr, str);
	speciesStr = tempStr + strlen("Time_Spawn_");

	speciesIndex = Util_Get_FG_Index_From_Token(bm, speciesStr, fileName, str, TRUE);
	Util_XML_Set_Node_Value(ATLANTIS_GROUP_ATTRIBUTE, parent, FunctGroupArray[speciesIndex].groupCode, valueStr);
}

/**
 */
void InvertebrateSNXMLFunction(MSEBoxModel *bm, char *fileName, xmlNodePtr parent, char *str, char *valueStr) {

	char speciesStr[50];
	char *strPtr;
	int cohort = adult_id;
	int speciesIndex;

	strcpy(speciesStr, str);
	strPtr = speciesStr;
	*(strstr(speciesStr, "_")) = '\0';

	if (strstr(strPtr, "j") != NULL) {
		strPtr++;
		cohort = juv_id;
	}

	speciesIndex = Util_Get_FG_Index_From_Token(bm, strPtr, fileName, str, TRUE);

	if (FunctGroupArray[speciesIndex].groupAgeType == AGE_STRUCTURED_BIOMASS)
		Util_XML_Replace_Node_Value(ATLANTIS_GROUP_ATTRIBUTE, parent, FunctGroupArray[speciesIndex].groupCode, valueStr, 2, cohort);
	else{
		if(cohort != juv_id)
			Util_XML_Set_Node_Value(ATLANTIS_GROUP_ATTRIBUTE, parent, FunctGroupArray[speciesIndex].groupCode, valueStr);
	}
}

void InitCatchGrazersXML(MSEBoxModel *bm, xmlNodePtr parent) {

	int guild;
	char *str = CreateZeroString(bm->K_num_tot_sp);

	/* Create a node for each functional group*/
	for (guild = 0; guild < bm->K_num_tot_sp; guild++) {
		if (FunctGroupArray[guild].speciesParams[catcheater_id] > 0) {
			Util_XML_Create_Node(ATLANTIS_GROUP_ATTRIBUTE, parent, FunctGroupArray[guild].groupCode, "", "", str);
		}
	}
	free(str);
}

void catchGrazersXML(MSEBoxModel *bm, char *fileName, xmlNodePtr parent, char *str, char *valueStr) {

	char *speciesStr;
	int speciesIndex;

	speciesStr = str + strlen("pFC");

	speciesIndex = Util_Get_FG_Index_From_Token(bm, speciesStr, fileName, str, TRUE);
	Util_XML_Set_Node_Value(ATLANTIS_GROUP_ATTRIBUTE, parent, FunctGroupArray[speciesIndex].groupCode, valueStr);
}

void InitCatchPropGrazersXML(MSEBoxModel *bm, xmlNodePtr parent) {

	int guild;
	char *str = CreateZeroString(bm->K_num_fisheries);

	/* Create a node for each functional group*/
	for (guild = 0; guild < bm->K_num_tot_sp; guild++) {
		if (FunctGroupArray[guild].speciesParams[catcheater_id] > 0) {
			Util_XML_Create_Node(ATLANTIS_GROUP_ATTRIBUTE, parent, FunctGroupArray[guild].groupCode, "", "", str);
		}
	}
	free(str);
}

void catchPropGrazersXML(MSEBoxModel *bm, char *fileName, xmlNodePtr parent, char *str, char *valueStr) {

	char *speciesStr;
	int speciesIndex;

	speciesStr = str + strlen("PropCatch_");

	speciesIndex = Util_Get_FG_Index_From_Token(bm, speciesStr, fileName, str, TRUE);
	Util_XML_Set_Node_Value(ATLANTIS_GROUP_ATTRIBUTE, parent, FunctGroupArray[speciesIndex].groupCode, valueStr);
}

void createSetupXML(MSEBoxModel *bm, FILE *fp, char *fileName, xmlDocPtr doc, xmlNodePtr rootnode) {
	xmlNodePtr groupingNode;

	xmlNodePtr node, lookupNode;

	groupingNode = Util_XML_Create_Node(ATLANTIS_ATTRIBUTE_SUB_GROUP, rootnode, "Setup", "Those determining set-up and recording", "", "");

	node = Util_XML_Parse_Create_Node(fp, fileName, groupingNode, "ecotest", "Debugging option. This flag indicates the messages printed to the log file.", "",
			XML_TYPE_LOOKUP, "2");
	lookupNode = Util_XML_Create_Lookup_Node(node);
	Util_XML_Create_Lookup_Value_Node(lookupNode, "0", "No additional debugging");
	Util_XML_Create_Lookup_Value_Node(lookupNode, "1", "Part additional debugging");
	Util_XML_Create_Lookup_Value_Node(lookupNode, "2", "More additional debugging");
	Util_XML_Create_Lookup_Value_Node(lookupNode, "3", "All additional debugging");
	Util_XML_Create_Lookup_Value_Node(lookupNode, "4", "Flux debugging");
	Util_XML_Create_Lookup_Value_Node(lookupNode, "5", "Do one iteration only");

	Util_XML_Parse_Create_Node(fp, fileName, groupingNode, "availflag", "Switch to filter flows based on active groups", "", XML_TYPE_BOOLEAN,"1");
	Util_XML_Parse_Create_Node(fp, fileName, groupingNode, "flagrandom", "Flag indicating whether add random wobble in aging/spawning dates", "", XML_TYPE_BOOLEAN,"1");
	Util_XML_Parse_Create_Node(fp, fileName, groupingNode, "readin_popratio", "Switch to indicate whether loading pre-existing age distributions or not.", "",
			XML_TYPE_BOOLEAN, "0");
	Util_XML_Parse_Create_Node(fp, fileName, groupingNode, "flagresp",
			"Switch to turn fish resperation on. If fish respiration is off then the fish model is essentially/very similar to a Deriso-Schnute", "", XML_TYPE_BOOLEAN,
			"1");
	Util_XML_Parse_Create_Node(fp, fileName, groupingNode, "flagavgmig", "Switch to turn averaging of migrate sizes on/off.", "", XML_TYPE_BOOLEAN,"1");
	Util_XML_Parse_Create_Node(fp, fileName, groupingNode, "flagkdrop", "Switch to turn cubic bacterial growth on/off.", "", XML_TYPE_BOOLEAN,"0");
	Util_XML_Parse_Create_Node(fp, fileName, groupingNode, "flagbactstim", "Switch to turn stimulation of bacterial growth on/off", "", XML_TYPE_BOOLEAN,"0");

}

void createPhysicalLim_n_FLagsXML(MSEBoxModel *bm, FILE *fp, char *fileName, xmlDocPtr doc, xmlNodePtr rootnode) {

	xmlNodePtr groupingNode, node, lookupNode;
	xmlNodePtr attributeNode;
	//xmlNodePtr groupAttribute;
	int value;

	printf("createPhysicalLimXML\n");

	groupingNode = Util_XML_Create_Node(ATLANTIS_ATTRIBUTE_SUB_GROUP, rootnode, "PhysicalLimitation",
			"Those determining which temperature, light, oxygen and nutrient limitation formulations are used ", "", "");

	Util_XML_Parse_Create_Node(fp, fileName, groupingNode, "flagtrackpops", "Flag to indicate whether yearly stock value required", "", XML_TYPE_BOOLEAN,"1");
	Util_XML_Parse_Create_Node(fp, fileName, groupingNode, "flagseason", "Flag to indicate seasonal change.", "", XML_TYPE_BOOLEAN,"1");
	Util_XML_Parse_Create_Node(
			fp,
			fileName,
			groupingNode,
			"flaglight",
			"Light adaptation so that production is not so tightly bound to irradiance and captures algae's ability to adapt to a wide range of light conditions. Light adaptation: 1 = on, 0 = off",
			"", XML_TYPE_BOOLEAN,"1");

	node = Util_XML_Parse_Create_Node(fp, fileName, groupingNode, "flaglightopt",
			"The option to use when calculating the light intensity on the surface of each cell", "", XML_TYPE_LOOKUP, "3");
	lookupNode = Util_XML_Create_Lookup_Node(node);
	Util_XML_Create_Lookup_Value_Node(lookupNode, "0", "Original");
	Util_XML_Create_Lookup_Value_Node(lookupNode, "1", "Bottom");
	Util_XML_Create_Lookup_Value_Node(lookupNode, "2", "Top");
	Util_XML_Create_Lookup_Value_Node(lookupNode, "3", "Mod-depth of cell");

	node = Util_XML_Parse_Create_Node(fp, fileName, groupingNode, "lim_sun_hours",
			"The option to use when calculating the number of sun light hours in each box.", "", XML_TYPE_LOOKUP, "0");
	lookupNode = Util_XML_Create_Lookup_Node(node);
	Util_XML_Create_Lookup_Value_Node(lookupNode, "0", "Assume 12 hours of sunlight");
	Util_XML_Create_Lookup_Value_Node(lookupNode, "1", "Calculate light based on latitude");
    Util_XML_Get_Value_Integer(fileName, ATLANTIS_ATTRIBUTE, bm->ecotest, 1, groupingNode, no_checking, "lim_sun_hours", &bm->lim_sun_hours);
    if(bm->lim_sun_hours)
        warn("lim_sun_hours set >0 so will calculate light rather than rading it in. If you wish to read in solar time series set lim_sun_hours to 0\n");
    
    node = Util_XML_Parse_Create_Node(fp, fileName, groupingNode, "flagmodeltemp", "The temperature model used.", "", XML_TYPE_LOOKUP, "1");
	lookupNode = Util_XML_Create_Lookup_Node(node);
	Util_XML_Create_Lookup_Value_Node(lookupNode, "0", "From Data");
	Util_XML_Create_Lookup_Value_Node(lookupNode, "1", "Regression");
	Util_XML_Create_Lookup_Value_Node(lookupNode, "2", "Fixed Baseline");

	node = Util_XML_Parse_Create_Node(fp, fileName, groupingNode, "flagmodelpH", "The pH model used.", "", XML_TYPE_LOOKUP, "1");
	lookupNode = Util_XML_Create_Lookup_Node(node);
	Util_XML_Create_Lookup_Value_Node(lookupNode, "0", "Disabled");
	Util_XML_Create_Lookup_Value_Node(lookupNode, "1", "From data");
	Util_XML_Create_Lookup_Value_Node(lookupNode, "2", "Regression");
	Util_XML_Get_Value_Integer(fileName, ATLANTIS_ATTRIBUTE, bm->ecotest, 1, groupingNode, no_checking, "flagmodelpH", &bm->flagmodelpH);

	node = Util_XML_Parse_Create_Node(fp, fileName, groupingNode, "flagmodelArag", "The agaonite saturation model used.", "", XML_TYPE_LOOKUP, "1");
	lookupNode = Util_XML_Create_Lookup_Node(node);
	Util_XML_Create_Lookup_Value_Node(lookupNode, "0", "Disabled");
	Util_XML_Create_Lookup_Value_Node(lookupNode, "1", "From data");
	Util_XML_Create_Lookup_Value_Node(lookupNode, "2", "Calculated dynamically");

	Util_XML_Parse_Create_Node(fp, fileName, groupingNode, "flagq10", "Switch to turn on Q10 corrections in the model", "", XML_TYPE_BOOLEAN,"1");
	node = Util_XML_Parse_Create_Node(fp, fileName, groupingNode, "O2case", "Oxygen limitation equation used", "", XML_TYPE_LOOKUP, "3");
	lookupNode = Util_XML_Create_Lookup_Node(node);
	Util_XML_Create_Lookup_Value_Node(lookupNode, "0", "Ambient");
	Util_XML_Create_Lookup_Value_Node(lookupNode, "1", "Based on depth");
	Util_XML_Create_Lookup_Value_Node(lookupNode, "2", "IGBEM");
	Util_XML_Create_Lookup_Value_Node(lookupNode, "3", "Quadratic Limitation");

	node = Util_XML_Parse_Create_Node(fp, fileName, groupingNode, "flagnut", "Option to indicate there is nutrient lim by micronutrient", "", XML_TYPE_LOOKUP, "0");
	lookupNode = Util_XML_Create_Lookup_Node(node);
	Util_XML_Create_Lookup_Value_Node(lookupNode, "0", "Leibig (min)");
	Util_XML_Create_Lookup_Value_Node(lookupNode, "1", "Multiplicative");
	Util_XML_Create_Lookup_Value_Node(lookupNode, "2", "WQI");

	Util_XML_Parse_Create_Node(fp, fileName, groupingNode, "flagmicro", "Nutrient lim by micronutrient.", "", XML_TYPE_BOOLEAN,"0");

	Util_XML_Parse_Create_Node(fp, fileName, groupingNode, "flagtempchange", "Flag to indicate that the mean temperature can change", "", XML_TYPE_BOOLEAN,"0");
	Util_XML_Parse_Create_Node(fp, fileName, groupingNode, "flagsaltchange", "Flag to indicate that the mean salinity can change", "", XML_TYPE_BOOLEAN,"0");
	Util_XML_Parse_Create_Node(fp, fileName, groupingNode, "flagpHchange", "Flag to indicate that the mean pH can change", "", XML_TYPE_BOOLEAN,"0");

	Util_XML_Parse_Create_Node(fp, fileName, groupingNode, "flagstarve", "Flag to turn on starvation notices being printed to the log file", "", XML_TYPE_BOOLEAN,"0");
	Util_XML_Parse_Create_Node(fp, fileName, groupingNode, "flagdegrade",
			"Flag to turn on degradation of coastal habitats, this will effect the growth of coastal groups", "", XML_TYPE_BOOLEAN,"0");

	node = Util_XML_Parse_Create_Node(fp, fileName, groupingNode, "flagroc", "Density dependant drop off option.", "", XML_TYPE_LOOKUP, "0");
	lookupNode = Util_XML_Create_Lookup_Node(node);
	Util_XML_Create_Lookup_Value_Node(lookupNode, "0", "None");
	Util_XML_Create_Lookup_Value_Node(lookupNode, "1", "Ratio drop off");
	Util_XML_Create_Lookup_Value_Node(lookupNode, "2", "exponential decline");

	Util_XML_Parse_Create_Node(fp, fileName, groupingNode, "flaghomog_sp", "Flag to indicate if body condition is homogenised across the entire area", "",
			XML_TYPE_BOOLEAN, "0");
	Util_XML_Parse_Create_Node(fp, fileName, groupingNode, "flagagestruct", "Age structure option: FALSE = homogeneous cohorts, TRUE = track cohorts", "",
			XML_TYPE_BOOLEAN, "0");
	Util_XML_Parse_Create_Node(fp, fileName, groupingNode, "flagsenesce", "Flag to indicate if oldest sub-part of final age group senesces", "", XML_TYPE_BOOLEAN,"0");
	Util_XML_Parse_Create_Node(fp, fileName, groupingNode, "flagtsforcerecruit",
			"Flag to indicate forcing of recruitment of 1+ groups with environmental forcing", "", XML_TYPE_BOOLEAN,"0");

    Util_XML_Parse_Create_Node(fp, fileName, groupingNode, "norm_larval_distrib",
                               "Flag to indicate whether to normalise larval distributions or not", "", XML_TYPE_BOOLEAN,"0");
    Util_XML_Parse_Create_Node(fp, fileName, groupingNode, "larvae_connect_only",
                               "Flag to indicate whether to use recruit_hdistrib and larval_connectivity for dispersal or just larval_connect only", "", XML_TYPE_BOOLEAN,"0");
    Util_XML_Parse_Create_Node(fp, fileName, groupingNode, "enviro_independ_larvae",
                               "Flag to indicate whether larvae are sensitive to environmental conditions or whether only apply to settlers and older", "", XML_TYPE_BOOLEAN,"0");
    Util_XML_Parse_Create_Node(fp, fileName, groupingNode, "flag_recruit_effect",
                               "Flag indicating whether to have environmental effects on start or end of larval period effect recruit_hdistrib", "", XML_TYPE_BOOLEAN,"0");
    
    Util_XML_Parse_Create_Node(fp, fileName, groupingNode, "flag_modify_KWSR",
                               "Flag to indicate whether forcing the size of recruits", "", XML_TYPE_INTEGER,"0");
    
    if(bm->external_populations)
        Util_XML_Parse_Create_Node(fp, fileName, groupingNode, "flag_extpop_growth_option",
                                   "Flag to indicate what external growth option is beign used", "", XML_TYPE_INTEGER,"0");
    
    Util_XML_Parse_Create_Node(fp, fileName, groupingNode, "juv_transition_thresh",
        "Threshold for saying juveniles are present enough in model to worry about", "", XML_TYPE_FLOAT,"0");
    Util_XML_Parse_Create_Node(fp, fileName, groupingNode, "mat_transition_thresh",
        "Threshold for saying mature adults are present enough in model to worry about", "", XML_TYPE_FLOAT,"0");
    
    Util_XML_Parse_Create_Node(fp, fileName, groupingNode, "flag_olddiet", "Flag to indicate whether need to reproduce old diet calculation", "", XML_TYPE_BOOLEAN,"0");
	Util_XML_Parse_Create_Node(fp, fileName, groupingNode, "UseHardFeedingWindow", "Flag to indicate whether using heaviside feeding window or smoother curve", "", XML_TYPE_BOOLEAN,"0");
    Util_XML_Parse_Create_Node(fp, fileName, groupingNode, "UseBiLogisticFeedingWindow", "Flag to indicate whether using the bilogistic (1) or humped (0) smooth window feeding curve", "", XML_TYPE_BOOLEAN,"0");
    Util_XML_Parse_Create_Node(fp, fileName, groupingNode, "flag_dynamicXRS", "Flag to indicate whether to use dynamic (1) or static (0) X_RS value", "", XML_TYPE_BOOLEAN,"0");
    Util_XML_Get_Value_Integer(fileName, ATLANTIS_ATTRIBUTE, bm->ecotest, 1, groupingNode, no_checking, "flag_dynamicXRS", &bm->flag_dynamicXRS);
    if(bm->flag_dynamicXRS) {
        Util_XML_Parse_Create_Node(fp, fileName, groupingNode, "XRS_cap", "Cap to use in dynamic XRS calcualtions", "", XML_TYPE_FLOAT,"20.0");
        Util_XML_Parse_Create_Node(fp, fileName, groupingNode, "flag_repcostSpawn", "Flag to indicate whether to use standard spawn calculation (0) or the one accounting for reproduction cost (1)", "", XML_TYPE_BOOLEAN,"0");
        Util_XML_Parse_Create_Node(fp, fileName, groupingNode, "flag_lengthSN", "Flag to indicate whether length is calculated from total weight (0) or only from the SN pool (1)", "", XML_TYPE_BOOLEAN,"0");
    }
    
	Util_XML_Get_Value_Integer(fileName, ATLANTIS_ATTRIBUTE, bm->ecotest, 1, groupingNode, binary_check, "UseHardFeedingWindow", &bm->UseHardFeedingWindow);

    Util_XML_Parse_Create_Node(fp, fileName, groupingNode, "flag_satiation", "Flag to indicate whether using satiation of feeding functions - especially for standard form of functional responses", "", XML_TYPE_BOOLEAN,"0");
    Util_XML_Parse_Create_Node(fp, fileName, groupingNode, "flag_shrinkfat", "Flag to indicate whether growth of fish allows fish to lose fat (1) or not (0)", "", XML_TYPE_BOOLEAN,"0");
    Util_XML_Parse_Create_Node(fp, fileName, groupingNode, "flag_predratiodepend", "Flag indicating whether using any ratio dependent feeding functions", "", XML_TYPE_BOOLEAN,"0");
    
	Util_XML_Parse_Create_Node(fp, fileName, groupingNode, "flag_macro_model", "Flag indicates which macrophyte model to use", "", XML_TYPE_BOOLEAN,"0");
	Util_XML_Get_Value_Integer(fileName, ATLANTIS_ATTRIBUTE, bm->ecotest, 1, groupingNode, binary_check, "flag_macro_model", &bm->flag_macro_model);

	Util_XML_Parse_Create_Node(fp, fileName, groupingNode, "flag_benthos_sediment_link", "Flag indicates whether growth of benthos is linked to the proportional cover of specific kinds of sediment", "", XML_TYPE_BOOLEAN,"0");
	Util_XML_Parse_Create_Node(fp, fileName, groupingNode, "flag_competing_epiff", "Flag indicating whether corals, filter feeders and macrophytes compete for space", "", XML_TYPE_BOOLEAN,"0");

	attributeNode = Util_XML_Get_Node(ATLANTIS_ATTRIBUTE, groupingNode, "flag_competing_epiff");
	value = Util_XML_Get_Node_Value_Int(fileName, ATLANTIS_ATTRIBUTE, attributeNode);

	if(value){
		Util_XML_Parse_Create_Node(fp, fileName, groupingNode, "max_available_habitat", "The max available habitat - should normally be 1.0 but changed to allow people to allow overlaping habitats", "", XML_TYPE_FLOAT,"1.0");
	}

	Util_XML_Parse_Create_Node(fp, fileName, groupingNode, "flag_invert_biohab", "Flag indicating whether inverebrates use dynamic biohabitat to condition growth", "", XML_TYPE_BOOLEAN,"0");
    
	Util_XML_Parse_Create_Node(fp, fileName, groupingNode, "flag_fine_ontogenetic_diets", "Flag indicates whether fine scale ontogenetic diets are required (1) or not (0)", "", XML_TYPE_BOOLEAN,"0");
	Util_XML_Get_Value_Integer(fileName, ATLANTIS_ATTRIBUTE, bm->ecotest, 1, groupingNode, binary_check, "flag_fine_ontogenetic_diets", &bm->flag_fine_ontogenetic_diets);

	Util_XML_Parse_Create_Node(fp, fileName, groupingNode, "flag_rel_cover", "Flag indicates whether using relative habitats (sum to 1) or cumulative (sum > 1, not as sensitive if lots of habitats).", "", XML_TYPE_BOOLEAN,"0");
	Util_XML_Parse_Create_Node(fp, fileName, groupingNode, "flag_report_water_detritus", "Flag indicates whether to report on detritus in water column as well as sediment.", "", XML_TYPE_BOOLEAN,"0");
	node = Util_XML_Parse_Create_Node(fp, fileName, groupingNode, "flag_refuge_model", "Flag indicating which refugia model to use", "", XML_TYPE_BOOLEAN,"0");
	lookupNode = Util_XML_Create_Lookup_Node(node);
	Util_XML_Create_Lookup_Value_Node(lookupNode, "0", "None");
	Util_XML_Create_Lookup_Value_Node(lookupNode, "1", "Original");
	Util_XML_Create_Lookup_Value_Node(lookupNode, "2", "Rugosity related");
	Util_XML_Get_Value_Integer(fileName, ATLANTIS_ATTRIBUTE, bm->ecotest, 1, groupingNode, no_checking, "flag_refuge_model", &bm->flag_refuge_model);
	if(bm->flag_refuge_model == rugosity_refuge_model)
		bm->flag_adv_habitat = 1;
	else
		bm->flag_adv_habitat = 0;

	node = Util_XML_Parse_Create_Node(fp, fileName, groupingNode, "flag_rugosity_model", "Flag indicating which rugosity model to use", "", XML_TYPE_INTEGER,"0");
	lookupNode = Util_XML_Create_Lookup_Node(node);
	Util_XML_Create_Lookup_Value_Node(lookupNode, "0", "None");
	Util_XML_Create_Lookup_Value_Node(lookupNode, "1", "Mechanistic");
	Util_XML_Create_Lookup_Value_Node(lookupNode, "2", "Blackwood empirical relationship");
    
	Util_XML_Parse_Create_Node(fp, fileName, groupingNode, "flag_georugosity", "Whether there is a geological component to rugosity - assumed to be what is read in on initalisation", "", XML_TYPE_BOOLEAN,"0");
    
	Util_XML_Parse_Create_Node(fp, fileName, groupingNode, "RugCover_Coefft", "Rugosity cover coefficient", "", XML_TYPE_FLOAT,"0");
	Util_XML_Parse_Create_Node(fp, fileName, groupingNode, "RugCover_Const", "Rugosity cover constant", "", XML_TYPE_FLOAT,"0");
	Util_XML_Parse_Create_Node(fp, fileName, groupingNode, "RugCover_Cap", "Rugosity cover cap", "", XML_TYPE_FLOAT,"0");

	Util_XML_Parse_Create_Node(fp, fileName, groupingNode, "min_rugosity", "Minimum rugosity", "", XML_TYPE_FLOAT,"0");
	Util_XML_Parse_Create_Node(fp, fileName, groupingNode, "max_rugosity", "Maximum rugosity", "", XML_TYPE_FLOAT,"0");
    Util_XML_Parse_Create_Node(fp, fileName, groupingNode, "rugosity_const", "Bozec empirical rugosity relationship constant", "", XML_TYPE_FLOAT,"0");
    Util_XML_Parse_Create_Node(fp, fileName, groupingNode, "rugosity_bozec_a", "Bozec empirical rugosity constant in step1 and step22", "", XML_TYPE_FLOAT,"0");
    Util_XML_Parse_Create_Node(fp, fileName, groupingNode, "rugosity_bozec_b", "Bozec empirical rugosity power cpefficient", "", XML_TYPE_FLOAT,"0");
    Util_XML_Parse_Create_Node(fp, fileName, groupingNode, "rugosity_bozec_c", "Bozec empirical rugosity scalar", "", XML_TYPE_FLOAT,"0");
    Util_XML_Parse_Create_Node(fp, fileName, groupingNode, "rugosity_bozec_d", "Bozec empirical rugosity divisor", "", XML_TYPE_FLOAT,"0");
    
	if(bm->flagmodelpH){
        node = Util_XML_Parse_Create_Node(fp, fileName, groupingNode, "flagPHmortcase", "Which pH mortality method to use", "", XML_TYPE_INTEGER,"0");
        lookupNode = Util_XML_Create_Lookup_Node(node);
        Util_XML_Create_Lookup_Value_Node(lookupNode, "0", "None");
        Util_XML_Create_Lookup_Value_Node(lookupNode, "1", "Sigmoid");
        Util_XML_Create_Lookup_Value_Node(lookupNode, "2", "McElhany and Bush style");
        
        //Deprecated as now a species level parameter
        //node = Util_XML_Parse_Create_Node(fp, fileName, groupingNode, "pH_sensitivity_model", "Form of pH sensitivity model", "", XML_TYPE_INTEGER,"0");
        //lookupNode = Util_XML_Create_Lookup_Node(node);
        //Util_XML_Create_Lookup_Value_Node(lookupNode, "0", "None");
        //Util_XML_Create_Lookup_Value_Node(lookupNode, "1", "Monod");
        //Util_XML_Create_Lookup_Value_Node(lookupNode, "2", "Non-linear");
        //Util_XML_Create_Lookup_Value_Node(lookupNode, "3", "Linear");
        //Util_XML_Create_Lookup_Value_Node(lookupNode, "4", "Piecewise");
        
        if (bm->flagmodelpH == ph_regression) {
            Util_XML_Parse_Create_Node(fp, fileName, groupingNode, "pH_surface_depth", "Depth of surface layer", "m", XML_TYPE_FLOAT,"200");
            Util_XML_Parse_Create_Node(fp, fileName, groupingNode, "pH_mid_depth", "Depth of midwater layer", "m", XML_TYPE_FLOAT,"1600");
            Util_XML_Parse_Create_Node(fp, fileName, groupingNode, "pH_surface_coefft_T", "Temperature term coefficient for surface waters", "", XML_TYPE_FLOAT,"0");
            Util_XML_Parse_Create_Node(fp, fileName, groupingNode, "pH_surface_coefft_S", "Salinity term coefficient for surface waters", "", XML_TYPE_FLOAT,"0");
            Util_XML_Parse_Create_Node(fp, fileName, groupingNode, "pH_surface_coefft_O", "Oxygen term coefficient for surface waters", "", XML_TYPE_FLOAT,"0");
            Util_XML_Parse_Create_Node(fp, fileName, groupingNode, "pH_surface_const", "Constant term coefficient for surface waters", "", XML_TYPE_FLOAT,"0");
            Util_XML_Parse_Create_Node(fp, fileName, groupingNode, "pH_mid_coefft_T", "Temperature term coefficient for mid-level waters", "", XML_TYPE_FLOAT,"0");
            Util_XML_Parse_Create_Node(fp, fileName, groupingNode, "pH_mid_coefft_S", "Salinity term coefficient for mid-level waters", "", XML_TYPE_FLOAT,"0");
            Util_XML_Parse_Create_Node(fp, fileName, groupingNode, "pH_mid_coefft_O", "Oxygen term coefficient for mid-level waters", "", XML_TYPE_FLOAT,"0");
            Util_XML_Parse_Create_Node(fp, fileName, groupingNode, "pH_mid_const", "Constant term coefficient for mid-level waters", "", XML_TYPE_FLOAT,"0");
            Util_XML_Parse_Create_Node(fp, fileName, groupingNode, "pH_deep_coefft_T", "Temperature term coefficient for deep waters", "", XML_TYPE_FLOAT,"0");
            Util_XML_Parse_Create_Node(fp, fileName, groupingNode, "pH_deep_coefft_S", "Salinity term coefficient for deep waters", "", XML_TYPE_FLOAT,"0");
            Util_XML_Parse_Create_Node(fp, fileName, groupingNode, "pH_deep_coefft_O", "Oxygen term coefficient for deep waters", "", XML_TYPE_FLOAT,"0");
            Util_XML_Parse_Create_Node(fp, fileName, groupingNode, "pH_deep_const", "Constant term coefficient for deep waters", "", XML_TYPE_FLOAT,"0");
        }
	}

	if(bm->containsCoral){
		Util_XML_Parse_Create_Node(fp, fileName, groupingNode, "K_max_num_DHW", "Size of degree heat weeks record queue for calculating bleaching triggers for corals", "", XML_TYPE_BOOLEAN,"14");
		Util_XML_Get_Value_Integer(fileName, ATLANTIS_ATTRIBUTE, bm->ecotest, 1, groupingNode, integer_check, "K_max_num_DHW", &bm->K_max_num_DHW);

		Util_XML_Parse_Create_Node(fp, fileName, groupingNode, "Karag_A", "Constant term for Bjerrum plot", "", XML_TYPE_FLOAT,"0");
		Util_XML_Parse_Create_Node(fp, fileName, groupingNode, "Karag_B", "Coefficient for Bjerrum plot", "", XML_TYPE_FLOAT,"0");
		Util_XML_Parse_Create_Node(fp, fileName, groupingNode, "Karag_C", "Exponent term for Bjerrum plot", "", XML_TYPE_FLOAT,"0");
		Util_XML_Parse_Create_Node(fp, fileName, groupingNode, "Karag_pH", "pH constant for Bjerrum plot", "", XML_TYPE_FLOAT,"0");
		Util_XML_Parse_Create_Node(fp, fileName, groupingNode, "Kca_const", "Constant scalar for for Bjerrum plot", "", XML_TYPE_FLOAT,"0");
		Util_XML_Parse_Create_Node(fp, fileName, groupingNode, "K_Ks", "Aragonite constant", "", XML_TYPE_FLOAT,"0");
		Util_XML_Parse_Create_Node(fp, fileName, groupingNode, "Ksmother_coefft", "Sediment smothering coefficient A", "", XML_TYPE_FLOAT,"0");
		Util_XML_Parse_Create_Node(fp, fileName, groupingNode, "Ksmother_const", "Sediment smothering constant", "", XML_TYPE_FLOAT,"0");
	}

    if(bm->track_atomic_ratio == TRUE){
        Util_XML_Parse_Create_Node(fp, fileName, groupingNode, "Pads_r_t0", "P adsorprtion starting r", "", XML_TYPE_FLOAT,"0");
        Util_XML_Parse_Create_Node(fp, fileName, groupingNode, "Pads_K", "P adsorprtion coefficient", "", XML_TYPE_FLOAT,"0");
        Util_XML_Parse_Create_Node(fp, fileName, groupingNode, "Pads_KO", "P adsorprtion base K", "", XML_TYPE_FLOAT,"0");
        Util_XML_Parse_Create_Node(fp, fileName, groupingNode, "r_immob_PIP_t0", "P immobilisation dtarting point", "", XML_TYPE_FLOAT,"0");
        
    }

}

void speciesXML(MSEBoxModel *bm, FILE *fp, char *fileName, xmlDocPtr doc, xmlNodePtr rootnode) {
	xmlNodePtr groupingNode;

	groupingNode = Util_XML_Create_Node(ATLANTIS_ATTRIBUTE_SUB_GROUP, rootnode, "FunctionalGroupFlags", "", "", "");

	/* Create the species param nodes */
	//Create_Species_ParamXML(bm, fileName, fp, groupingNode, flag_id, "Groups functioning", "", XML_TYPE_BOOLEAN,"");
	Create_Species_ParamXML(
			bm,
			fileName,
			fp,
			groupingNode,
			flagdem_id,
			"Preferred location trend (whether to weight vertical distributions towards surface or bottom layers when in depths where there were less than complete set of depth layers)",
			"", XML_TYPE_BOOLEAN,"1");

	Create_Species_ParamXML(bm, fileName, fp, groupingNode, flagplankfish_id, "Identification of planktivores.", "", XML_TYPE_BOOLEAN,"1");
	Create_Species_ParamXML(bm, fileName, fp, groupingNode, flag_lim_id, "Flag to turn on population limitation for biomass groups.", "", XML_TYPE_BOOLEAN,"1");
	Create_Species_ParamXML(bm, fileName, fp, groupingNode, flux_thresh_id, "Threshold sed bacteria flux level where damping starts", "mg N m-2 s-1", XML_TYPE_FLOAT, "0.1");
	Create_Species_ParamXML(bm, fileName, fp, groupingNode, flux_damp_id, "Damping coefficient for group fluxes", "", XML_TYPE_FLOAT, "0.75");

}

void createPhysicalChemicalXML(MSEBoxModel *bm, FILE *fp, char *fileName, xmlDocPtr doc, xmlNodePtr rootnode) {

	int changeActive;
	xmlNodePtr childGroupingNode, groupingNode;
	xmlNodePtr attributeNode, groupAttribute;

	groupingNode = Util_XML_Create_Node(ATLANTIS_ATTRIBUTE_SUB_GROUP, rootnode, "PhysicalChemical", "Physical and chemical parameters", "", "");
	childGroupingNode = Util_XML_Create_Node(ATLANTIS_ATTRIBUTE_SUB_GROUP, groupingNode, "Bioturbation", "Bioturbation", "", "");

	Util_XML_Parse_Create_Node(fp, fileName, childGroupingNode, "Enviro_turb", "Background level of bioturbation (physical mixing)", "", XML_TYPE_FLOAT, "0.00002");
	Util_XML_Parse_Create_Node(fp, fileName, childGroupingNode, "K_TUR", "Half saturation of bioturbation", "", XML_TYPE_FLOAT, "116.0");
	Util_XML_Parse_Create_Node(fp, fileName, childGroupingNode, "K_TUR_DEP", "Bioturbation depth", "m", XML_TYPE_FLOAT, "0.02");
	Util_XML_Parse_Create_Node(fp, fileName, childGroupingNode, "K_MAX_TUR", "Maximum contributions to bioturbation", "", XML_TYPE_FLOAT, "10.0");

	childGroupingNode = Util_XML_Create_Node(ATLANTIS_ATTRIBUTE_SUB_GROUP, groupingNode, "Bioirrigation", "Bioirrigation", "", "");
	Util_XML_Parse_Create_Node(fp, fileName, childGroupingNode, "K_IRR", "Half saturation of bioirrigation", "", XML_TYPE_FLOAT, "101.0");
	Util_XML_Parse_Create_Node(fp, fileName, childGroupingNode, "K_MAX_IRR", "Maximum contributions to bioirrigation", "", XML_TYPE_FLOAT, "10.0");
	Util_XML_Parse_Create_Node(fp, fileName, childGroupingNode, "K_MIN_IRR", "Minimum contributions to bioirrigation ", "", XML_TYPE_FLOAT, "2.0");

	//TODO: Work out what to do about DR , DL and DC.
	childGroupingNode = Util_XML_Create_Node(ATLANTIS_ATTRIBUTE_SUB_GROUP, groupingNode, "DetritusBreakdown",
			"Detrital breakdown (DL and DR breakdown not used if bacteria groups are on)", "", "");

	Util_XML_Parse_Create_Node(fp, fileName, childGroupingNode, "r_DC_T15", "Rate of carrion breakdown to detritus at 15 deg C ", "d-1", XML_TYPE_FLOAT, "0.5");
	Util_XML_Parse_Create_Node(fp, fileName, childGroupingNode, "r_DL_T15", "Rate of labile detrital breakdown at 15 deg C ", "d-1", XML_TYPE_FLOAT, "0.1");
	Util_XML_Parse_Create_Node(fp, fileName, childGroupingNode, "r_DR_T15", "Rate of ref detrital breakdown at 15 deg C", "d-1", XML_TYPE_FLOAT, "0.008");
	Util_XML_Parse_Create_Node(fp, fileName, childGroupingNode, "r_DON_T15", "Rate of DON breakdown at 15 deg C", "d-1", XML_TYPE_FLOAT, "0.00176");
	Util_XML_Parse_Create_Node(fp, fileName, childGroupingNode, "r_DSi_T15", "Rate of detrital Si breakdown at 15 deg C", "d-1", XML_TYPE_FLOAT, "0.5");

	Util_XML_Parse_Create_Node(fp, fileName, childGroupingNode, "FDR_DC", "Fraction of carrion that breaks down to refractory detritus", "", XML_TYPE_FLOAT, "0.1");
	Util_XML_Parse_Create_Node(fp, fileName, childGroupingNode, "FDR_DL", "Fraction of lab detrit that breaks down to refractory detritus", "", XML_TYPE_FLOAT, "0.2");
	Util_XML_Parse_Create_Node(fp, fileName, childGroupingNode, "FDON_D", "Fraction of DON breakdown that becomes detritus", "", XML_TYPE_FLOAT, "0.05");

	childGroupingNode = Util_XML_Create_Node(ATLANTIS_ATTRIBUTE_SUB_GROUP, groupingNode, "Nitrification_And_Denitrification",
			"Nitrification and denitrification", "", "");

	Util_XML_Parse_Create_Node(fp, fileName, childGroupingNode, "R_0_T15", "Minimum rate of sediment respiration that supports nitrification", "", XML_TYPE_FLOAT,
			"200 - 250");
	Util_XML_Parse_Create_Node(fp, fileName, childGroupingNode, "R_D_T15", "Peak of nitrification-denitrification curve ", "", XML_TYPE_FLOAT, "10 - 20");
	Util_XML_Parse_Create_Node(fp, fileName, childGroupingNode, "Dmax", "Maximum rate of denitrification", "", XML_TYPE_FLOAT, "0.25");
	Util_XML_Parse_Create_Node(fp, fileName, childGroupingNode, "K_nit_T15", "Rate of nitrification by free bacteria", "", XML_TYPE_FLOAT, "0.01");
	Util_XML_Parse_Create_Node(fp, fileName, childGroupingNode, "K_conc", "Half concentration level for nitrification by PFB", "mg N m-3", XML_TYPE_FLOAT, "2000.0");

	Util_XML_Parse_Create_Node(fp, fileName, childGroupingNode, "p_NH_anad", "Proportion of NH4 adsorped to NH4 dissolved", "", XML_TYPE_FLOAT, "3.0");

	childGroupingNode = Util_XML_Create_Node(ATLANTIS_ATTRIBUTE_SUB_GROUP, groupingNode, "Redfield", "Redfield parameters", "", "");

	Util_XML_Parse_Create_Node(fp, fileName, childGroupingNode, "X_ON", "Redfield ratio of O:N", "", XML_TYPE_FLOAT, "16.0");
	Util_XML_Parse_Create_Node(fp, fileName, childGroupingNode, "X_CN", "Redfield ratio of C:N", "", XML_TYPE_FLOAT, "5.7");
	Util_XML_Parse_Create_Node(fp, fileName, childGroupingNode, "X_CHLN", "Ratio of chla to N", "", XML_TYPE_FLOAT, "7.0");
	Util_XML_Parse_Create_Node(fp, fileName, childGroupingNode, "X_SiN", "Redfield ratio of Si:N", "", XML_TYPE_FLOAT, "3.0");
	Util_XML_Parse_Create_Node(fp, fileName, childGroupingNode, "X_FeN", "Redfield ratio of Fe:N", "", XML_TYPE_FLOAT, "30000.0");

	Util_XML_Parse_Create_Node(fp, fileName, groupingNode, "k_wetdry", "Ratio of Wet weight to AFDW", "", XML_TYPE_FLOAT, "20.0");

	childGroupingNode = Util_XML_Create_Node(ATLANTIS_ATTRIBUTE_SUB_GROUP, groupingNode, "Light_Penetration", "Light penetration through the water column", "",
			"");

	Util_XML_Parse_Create_Node(fp, fileName, childGroupingNode, "k_w_depth", "Bottom depth marking estuarine vs oceanic water (for light absorp)", "m", XML_TYPE_FLOAT,
			"55.0");
	Util_XML_Parse_Create_Node(fp, fileName, childGroupingNode, "k_w_cdepth", "Bottom depth marking coastal from oceanic water (for light absorp)", "m", XML_TYPE_FLOAT,
			"205.0");
	Util_XML_Parse_Create_Node(fp, fileName, childGroupingNode, "k_w_deep", "Coefficient of background light absorption due to oceanic water ", "", XML_TYPE_FLOAT,
			"0.005 - 0.016");
	Util_XML_Parse_Create_Node(fp, fileName, childGroupingNode, "k_w_shallow", "Coefficient of background light absorption due to coastal water ", "", XML_TYPE_FLOAT,
			"0.016 - 0.078");
	Util_XML_Parse_Create_Node(fp, fileName, childGroupingNode, "k_PN", "Coefficient of light absorption due to particulate matter", "", XML_TYPE_FLOAT, "0.0035");
	Util_XML_Parse_Create_Node(fp, fileName, childGroupingNode, "k_DON", " Coefficient of light absorption due to DON", "", XML_TYPE_FLOAT, "0.0009");
	Util_XML_Parse_Create_Node(fp, fileName, childGroupingNode, "k_DL", "Coefficient of light absorption due to labile detritus", "", XML_TYPE_FLOAT, "0.0038");
	Util_XML_Parse_Create_Node(fp, fileName, childGroupingNode, "k_IS", " Basic coefficient of light absorption", "", XML_TYPE_FLOAT, "0.05");
	Util_XML_Parse_Create_Node(fp, fileName, childGroupingNode, "k_SED", " Basic coefficient of light absorption due to sediment", "", XML_TYPE_FLOAT, "0.05");

	Util_XML_Parse_Create_Node(fp, fileName, childGroupingNode, "KIOP_min", "Minimum level of light adaptation", "", XML_TYPE_FLOAT, "4.0");
	Util_XML_Parse_Create_Node(fp, fileName, childGroupingNode, "KIOP_shift", "Minimum rate of shift under light adaptation", "", XML_TYPE_FLOAT, "0.25");
	Util_XML_Parse_Create_Node(fp, fileName, childGroupingNode, "KI_avail", "Available light", "", XML_TYPE_FLOAT, "0.5");
	Util_XML_Parse_Create_Node(fp, fileName, childGroupingNode, "K_addepth", "Depth down to which adaptation occurs", "", XML_TYPE_FLOAT, "10.0");

	Util_XML_Parse_Create_Node(fp, fileName, childGroupingNode, "swr_scalar", " Proportion of shortwave radiation available to photosynthesis", "", XML_TYPE_FLOAT, "0.45");
    Util_XML_Parse_Create_Node(fp, fileName, childGroupingNode, "swr_const", " Constant in the equation for the shortwave radiation available to photosynthesis", "", XML_TYPE_FLOAT, "1367.0");
    Util_XML_Parse_Create_Node(fp, fileName, childGroupingNode, "swr_cos_coefft", " Coefficient of the cos term in the equation for the shortwave radiation available to photosynthesis", "", XML_TYPE_FLOAT, "0.034");
    Util_XML_Parse_Create_Node(fp, fileName, childGroupingNode, "swr_cos_offset", " Offset term in the cos term in the equation for the shortwave radiation available to photosynthesis", "", XML_TYPE_FLOAT, "0");

	if (bm->ice_on) {
		childGroupingNode = Util_XML_Create_Node(ATLANTIS_ATTRIBUTE_SUB_GROUP, groupingNode, "Ice_Light_Penetration", "Ice Light Penetration", "", "");
		Util_XML_Parse_Create_Node(fp, fileName, childGroupingNode, "albedo_ice", " ", "", XML_TYPE_FLOAT, "0.45");
		Util_XML_Parse_Create_Node(fp, fileName, childGroupingNode, "k_bs", " ", "", XML_TYPE_FLOAT, "0.45");
		Util_XML_Parse_Create_Node(fp, fileName, childGroupingNode, "k_bi", " ", "", XML_TYPE_FLOAT, "0.45");
		Util_XML_Parse_Create_Node(fp, fileName, childGroupingNode, "k_rs", " ", "", XML_TYPE_FLOAT, "0.45");
		Util_XML_Parse_Create_Node(fp, fileName, childGroupingNode, "k_ri", " ", "", XML_TYPE_FLOAT, "0.45");
		Util_XML_Parse_Create_Node(fp, fileName, childGroupingNode, "R_bi", " ", "", XML_TYPE_FLOAT, "0.45");
		Util_XML_Parse_Create_Node(fp, fileName, childGroupingNode, "k_ice", " ", "", XML_TYPE_FLOAT, "0.45");
		Util_XML_Parse_Create_Node(fp, fileName, childGroupingNode, "ka_star", " ", "", XML_TYPE_FLOAT, "0.45");
	}

	/* These are only read in if the readin_popratio flag is true */

	childGroupingNode = Util_XML_Create_Node(ATLANTIS_ATTRIBUTE_SUB_GROUP, groupingNode, "Temperature_Effects", "Temperature effects", "", "");

	groupAttribute = Util_XML_Get_Node(ATLANTIS_ATTRIBUTE_SUB_GROUP, rootnode, "PhysicalLimitation");
	if (groupAttribute == NULL)
		quit("Node PhysicalLimitation not found in file %s\n", fileName);
	attributeNode = Util_XML_Get_Node(ATLANTIS_ATTRIBUTE, groupAttribute, "flagtempchange");
	changeActive = Util_XML_Get_Node_Value_Int(fileName, ATLANTIS_ATTRIBUTE, attributeNode);


	if (changeActive == TRUE){
		Util_XML_Parse_Create_Node(fp, fileName, childGroupingNode, "Tchange_max_num", "Number of changes in average base temperature", "", XML_TYPE_INTEGER, "1");

		Util_XML_Create_Node_Next_Line(fp, fileName, childGroupingNode, "Tchange_start",
				"Day changes in base temperature begins (must have same number of entries as given by Tchange_max_num)", "", XML_TYPE_FLOATARRAY);
		Util_XML_Create_Node_Next_Line(fp, fileName, childGroupingNode, "Tchange_period",
				"Period of time changes in base temperature take (must have same number of entries as given by Tchange_max_num)", "", XML_TYPE_FLOATARRAY);
		Util_XML_Create_Node_Next_Line(fp, fileName, childGroupingNode, "Tchange_mult",
				"Scale of change in base temperature (must have same number of entries as given by Tchange_max_num)", "", XML_TYPE_FLOATARRAY);
		Util_XML_Create_Node_Next_Line(fp, fileName, childGroupingNode, "vertTchange_mult",
				"Differential scaling with depth (must be as many entries as number of water column layers * Tchange_max_num)", "", XML_TYPE_FLOATARRAY);
	}
	attributeNode = Util_XML_Get_Node(ATLANTIS_ATTRIBUTE, groupAttribute, "flagsaltchange");
	changeActive = Util_XML_Get_Node_Value_Int(fileName, ATLANTIS_ATTRIBUTE, attributeNode);

	if (changeActive == TRUE){
		childGroupingNode = Util_XML_Create_Node(ATLANTIS_ATTRIBUTE_SUB_GROUP, groupingNode, "Salinity_Effects", "Salinity effects", "", "");
		Util_XML_Parse_Create_Node(fp, fileName, childGroupingNode, "Schange_max_num", "Number of changes in average base salinity", "", XML_TYPE_INTEGER, "1");
		Util_XML_Create_Node_Next_Line(fp, fileName, childGroupingNode, "Schange_start",
				"Day changes in base salinity begins (must have same number of entries as given by Schange_max_num)", "", XML_TYPE_FLOATARRAY);
		Util_XML_Create_Node_Next_Line(fp, fileName, childGroupingNode, "Schange_period",
				"Period of time changes in base salinity take (must have same number of entries as given by Schange_max_num)", "", XML_TYPE_FLOATARRAY);
		Util_XML_Create_Node_Next_Line(fp, fileName, childGroupingNode, "Schange_mult",
				"Scale of change in base salinity (must have same number of entries as given by Schange_max_num)", "", XML_TYPE_FLOATARRAY);
		Util_XML_Create_Node_Next_Line(fp, fileName, childGroupingNode, "vertSchange_mult",
				"Differential scaling with depth (must be as many entries as number of water column layers * number of changes)", "", XML_TYPE_FLOATARRAY);
	}

	if(bm->track_pH == TRUE){
		childGroupingNode = Util_XML_Create_Node(ATLANTIS_ATTRIBUTE_SUB_GROUP, groupingNode, "pH_Effects", "pH effects", "", "");
		Util_XML_Parse_Create_Node(fp, fileName, childGroupingNode, "pHchange_max_num", "Number of changes in average base pH", "", XML_TYPE_INTEGER, "1");
		Util_XML_Create_Node_Next_Line(fp, fileName, childGroupingNode, "pHchange_start",
				"Day changes in base pH begins (must have same number of entries as given by pHchange_max_num)", "", XML_TYPE_FLOATARRAY);
		Util_XML_Create_Node_Next_Line(fp, fileName, childGroupingNode, "pHchange_period",
				"Period of time changes in base pH take (must have same number of entries as given by pHchange_max_num)", "", XML_TYPE_FLOATARRAY);
		Util_XML_Create_Node_Next_Line(fp, fileName, childGroupingNode, "pHchange_mult",
				"Scale of change in base pH (must have same number of entries as given by pHchange_max_num)", "", XML_TYPE_FLOATARRAY);
		Util_XML_Create_Node_Next_Line(fp, fileName, childGroupingNode, "vertpHchange_mult",
				"Differential scaling with depth (must be as many entries as number of water column layers * number of changes)", "", XML_TYPE_FLOATARRAY);
	}
}


/**
 * Create the XML for additional tracers if thats active in this model run.
 *
 *
 *
 */
void createAdditionalTracersXML(MSEBoxModel *bm, FILE *fp, char *fileName, xmlDocPtr doc, xmlNodePtr rootnode) {
	xmlNodePtr groupingNode;

	groupingNode = Util_XML_Create_Node(ATLANTIS_ATTRIBUTE_SUB_GROUP, rootnode, "AdditionalNutrients", "", "", "");

	Util_XML_Parse_Create_Node(fp, fileName, groupingNode, "RTOP", "rates of transformation from available nutrient forms to available forms in days-1", "", XML_TYPE_FLOAT, "0.005");

	/* If we are tracking P then read in P primary producer values */
	Create_Species_ParamXML(bm, fileName, fp, groupingNode, P_max_uptake_id, "Maximum phosphorus uptake rate per day.", "day-1", XML_TYPE_FLOAT, "0.5");
	Create_Species_ParamXML(bm, fileName, fp, groupingNode, P_uptake_scale_id, "Affinity coefficient for phosphorus uptake mechanism", "", XML_TYPE_FLOAT, "0.518?");
	Create_Species_ParamXML(bm, fileName, fp, groupingNode, P_concp_id, "Internal concentration factor", "", XML_TYPE_FLOAT, "0.250e6");

	Create_Species_ParamXML(bm, fileName, fp, groupingNode, P_min_internal_id, "Min internal phosphorus concentrations", "", XML_TYPE_FLOAT, "0.5");
	Create_Species_ParamXML(bm, fileName, fp, groupingNode, P_max_internal_id, "Max internal phosphorus concentrations", "", XML_TYPE_FLOAT, "0.5");

	Create_Species_ParamXML(bm, fileName, fp, groupingNode, C_max_uptake_id, "Maximum carbon uptake rate per day.", "day-1", XML_TYPE_FLOAT, "0.5");
	Create_Species_ParamXML(bm, fileName, fp, groupingNode, C_uptake_scale_id, "Affinity coefficient for carbon uptake mechanism", "", XML_TYPE_FLOAT, "0.518?");
	Create_Species_ParamXML(bm, fileName, fp, groupingNode, C_concp_id, "Internal concentration factor", "", XML_TYPE_FLOAT, "0.250e6");

	Create_Species_ParamXML(bm, fileName, fp, groupingNode, Psa_min_id, "Min quota of phosphorus in phytoplankton cells in moles/mg dry weight", "", XML_TYPE_FLOAT, "1.2287");
	Create_Species_ParamXML(bm, fileName, fp, groupingNode, C_min_id, "Min quota of Carbon in phytoplankton cells in moles/mg dry weight", "", XML_TYPE_FLOAT, "1.2287");
	Create_Species_ParamXML(bm, fileName, fp, groupingNode, phyto_resp_rate_id, "Phytoplankton respiration rate", "", XML_TYPE_FLOAT, "0.05");
	Create_Species_ParamXML(bm, fileName, fp, groupingNode, KP_id, "Half-sat const for group growth on Phosphorus", "", XML_TYPE_FLOAT, "");

	Util_XML_Parse_Create_Node(fp, fileName, groupingNode, "K_Lc", "Inorganic carbon exchange rate", "", XML_TYPE_FLOAT, "1.0");
}

void createMovementXML(MSEBoxModel *bm, FILE *fp, char *fileName, xmlDocPtr doc, xmlNodePtr rootnode) {
	xmlNodePtr groupingNode, node, lookupNode;
	int flagdepend;

	groupingNode = Util_XML_Create_Node(ATLANTIS_ATTRIBUTE_SUB_GROUP, rootnode, "Movement", "Group movement options", "", "");

	Util_XML_Parse_Create_Node(fp, fileName, groupingNode, "roc_wgt", "Wgt factor for density dependent fish move", "", XML_TYPE_FLOAT, "10000.0");
	Util_XML_Parse_Create_Node(fp, fileName, groupingNode, "k_roc_food", "Percent of body mass needed as food before disperse (ddepend move on)", "", XML_TYPE_FLOAT,
			"0.5");
	Parse_File(bm, fp, fileName, groupingNode, "VerticalDistribution", "^VERTday|^VERTnight|^VERToverwinter", "Vertical Distribution", "", XML_TYPE_FLOATARRAY, bm->wcnz, TRUE,
			InitVertDistributionXML, VERTXMLFunction);

	Parse_File(bm, fp, fileName, groupingNode, "SeasonalDistribution", "^F.*_S[0-9]+$|^F.*_S[0-9]+juv$", "Seasonal distribution", "", XML_TYPE_FLOATARRAY, bm->nbox,
			TRUE, InitVertSeasonalDistXML, VertSeasonalDistXMLFunction);

	Create_Species_ParamXML(bm, fileName, fp, groupingNode, Speed_id, "Swimming speed", "m.hr-1", XML_TYPE_FLOAT, "12500");

	Util_XML_Parse_Create_Node(fp, fileName, groupingNode, "flagtempdepend_move", "Switch to turn on group temperature preferences for movement", "", XML_TYPE_BOOLEAN,"0");
    Util_XML_Parse_Create_Node(fp, fileName, groupingNode, "flagtempdepend_reprod", "Switch to turn on group temperature preferences for reproduction", "", XML_TYPE_BOOLEAN,"0");
	Util_XML_Parse_Create_Node(fp, fileName, groupingNode, "flagsaltdepend", "Switch to turn on group salinity preferences", "", XML_TYPE_BOOLEAN,"0");
	Util_XML_Parse_Create_Node(fp, fileName, groupingNode, "flagO2depend", "Switch to turn on group O2 preferences", "", XML_TYPE_BOOLEAN,"0");
    Util_XML_Parse_Create_Node(fp, fileName, groupingNode, "flagconstrain_epiwander", "Switch to indicate that want to prevent epibenthic wandering", "", XML_TYPE_BOOLEAN,"0");

    Util_XML_Get_Value_Integer(fileName, ATLANTIS_ATTRIBUTE, bm->ecotest, 1, groupingNode, no_checking, "flagtempdepend_move", &bm->flagtempdepend_move);
    Util_XML_Get_Value_Integer(fileName, ATLANTIS_ATTRIBUTE, bm->ecotest, 1, groupingNode, no_checking, "flagtempdepend_reprod", &bm->flagtempdepend_reprod);

    if (bm->flagtempdepend_move) {
        Create_Species_ParamXML(bm, fileName, fp, groupingNode, max_move_temp_id, "Min tolerated temperatures", "deg C", XML_TYPE_FLOAT, "");
        Create_Species_ParamXML(bm, fileName, fp, groupingNode, min_move_temp_id, "Maximum tolerated temperatures", "deg C", XML_TYPE_FLOAT, "");
        Create_Species_ParamXML(bm, fileName, fp, groupingNode, K_temp_const_id, "Constant in bilogistic tolerated temperature relationship", "deg C", XML_TYPE_FLOAT, "");
    }

	Util_XML_Get_Value_Integer(fileName, ATLANTIS_ATTRIBUTE, bm->ecotest, 1, groupingNode, no_checking, "flagsaltdepend", &bm->flagsaltdepend);
	if (bm->flagsaltdepend) {
		Create_Species_ParamXML(bm, fileName, fp, groupingNode, max_move_salt_id, "Min tolerated salinity", "", XML_TYPE_FLOAT, "");
		Create_Species_ParamXML(bm, fileName, fp, groupingNode, min_move_salt_id, "Maximum tolerated salinity", "", XML_TYPE_FLOAT, "");
        Create_Species_ParamXML(bm, fileName, fp, groupingNode, K_salt_const_id, "Constant in bilogistic tolerated salinity relationship", "deg C", XML_TYPE_FLOAT, "");
	}
	Util_XML_Get_Value_Integer(fileName, ATLANTIS_ATTRIBUTE, bm->ecotest, 1, groupingNode, no_checking, "flagO2depend", &flagdepend);
	if (flagdepend) {
		Create_Species_ParamXML(bm, fileName, fp, groupingNode, min_O2_id, "Maximum tolerated O2", "", XML_TYPE_FLOAT, "");
        Create_Species_ParamXML(bm, fileName, fp, groupingNode, K_o2_const_id, "Constant in bilogistic tolerated O2 relationship", "deg C", XML_TYPE_FLOAT, "");
	}
	node = Create_Species_ParamXML(bm, fileName, fp, groupingNode, ddepend_move_id, "Vertebrate density dependent movement related flags.", "", XML_TYPE_LOOKUP, "0");
	lookupNode = Util_XML_Create_Lookup_Node(node);
	Util_XML_Create_Lookup_Value_Node(lookupNode, "0", "Off");
	Util_XML_Create_Lookup_Value_Node(lookupNode, "1", "Sedentary");
	Util_XML_Create_Lookup_Value_Node(lookupNode, "2", "On");
	Util_XML_Create_Lookup_Value_Node(lookupNode, "3", "Sticky");
	Util_XML_Create_Lookup_Value_Node(lookupNode, "4", "No explicit movement");

	Create_Species_ParamXML(bm, fileName, fp, groupingNode, mindepth_id, "", "", XML_TYPE_FLOAT, "");
	Create_Species_ParamXML(bm, fileName, fp, groupingNode, maxdepth_id, "", "", XML_TYPE_FLOAT, "");
    Create_Species_ParamXML(bm, fileName, fp, groupingNode, maxtotdepth_id, "", "", XML_TYPE_FLOAT, "");

	Create_Species_ParamXML(bm, fileName, fp, groupingNode, homerangerad_id, "Home range radius (m) for each group", "m", XML_TYPE_FLOAT, "");
	Create_Species_ParamXML(bm, fileName, fp, groupingNode, rangeoverlap_id, "Whether home ranges overlap for each group", "", XML_TYPE_BOOLEAN,"");

}

void createMortalityXML(MSEBoxModel *bm, FILE *fp, char *fileName, xmlDocPtr doc, xmlNodePtr groupingNode) {
	xmlNodePtr childGroupingNode, newchildGroupingNode;
	xmlNodePtr newnode;

	childGroupingNode = Util_XML_Create_Node(ATLANTIS_ATTRIBUTE_SUB_GROUP, groupingNode, "Mortality", "Mortality Parameters", "", "");

	InitPrimaryProducerSpeciesZeroValuesXML(bm, childGroupingNode, "FLAG_LYSIS_RATE", "Mircoalgae groups lysis rate", "mg O2 m-3");
	Create_Species_ParamXML(bm, fileName, fp, childGroupingNode, KLYS_id, "Mircoalgae groups lysis rate", "d-1", XML_TYPE_FLOAT, "0.01 - 0.0001");

	// Extra macrophyte mortality (based on physical environment)
		Create_Species_ParamXML(bm, fileName, fp, childGroupingNode, mS_T15_id, "Extra macrophyte mortality (based on physical environment)", "d-1", XML_TYPE_FLOAT, "0.0 - 180");

	Create_Species_Cohort_ParamXML(bm, fileName, fp, doc, childGroupingNode, mL_T15_id, "Linear mortality rate", "d-1", XML_TYPE_FLOATARRAY, "0 - 1.0", SpeciesXMLFunction);
	Create_Species_Cohort_ParamXML(bm, fileName, fp, doc, childGroupingNode, mQ_T15_id, "Quadratic mortality rate", "d-1", XML_TYPE_FLOATARRAY, "0 - 1.0", SpeciesXMLFunction);

    if(bm->external_populations){
        Create_Species_Cohort_ParamXML(bm, fileName, fp, doc, childGroupingNode, mLext_id, "External linear mortality rate", "d-1", XML_TYPE_FLOATARRAY, "0 - 1.0", SpeciesXMLFunction);
        Create_Species_Cohort_ParamXML(bm, fileName, fp, doc, childGroupingNode, mPext_id, "P external mortality rate", "d-1", XML_TYPE_FLOATARRAY, "0 - 1.0", SpeciesXMLFunction);
    }
    
    if(bm->containsBirds){
		Parse_File(bm, fp, fileName, childGroupingNode, "ImplicitSeabirdMortalityRate", "^mS_SB",
				"Static seabird induced mortality of each fish group, per quarter, d-1", "", XML_TYPE_FLOATARRAY, 4, TRUE, InitImplicitMortalityRateXML,
				ImplicitMortalityRateXMLFunction);
	}

	Parse_File(bm, fp, fileName, childGroupingNode, "ImplicitFishMortalityRate", "^mS_FD",
			"Static mammal induced mortality of each fish group, per quarter, d-1", "", XML_TYPE_FLOATARRAY, 4, TRUE, InitImplicitMortalityRateXML,
			ImplicitMortalityRateXMLFunction);

	Util_XML_Parse_Create_Node(fp, fileName, childGroupingNode, "Kthreshm", "Threshold starvation level when mort starts ", "", XML_TYPE_FLOAT, "0.7");
	Create_Species_ParamXML(bm, fileName, fp, childGroupingNode, mStarve_id, "Starvation mortality of fish", "d-1", XML_TYPE_FLOAT, "0.005");
	Create_Species_ParamXML(bm, fileName, fp, childGroupingNode, mT_id, "Terminal (senescence) mortality rate for age structured biomass pools (proportion of terminal age group dying)", "", XML_TYPE_FLOAT, "0.0");

	/* Oxygen species parameters */
	newchildGroupingNode = Util_XML_Create_Node(ATLANTIS_ATTRIBUTE_SUB_GROUP, childGroupingNode, "Oxygen_Mortality", "Oxygen Mortality Parameters", "", "");

	Create_Species_ParamXML(bm, fileName, fp, newchildGroupingNode, mD_id, "Half oxygen mortality depth for groups - based on depth and oxygen horizon", "m",
			XML_TYPE_FLOAT, "0 - 0.001");
	Create_Species_ParamXML(bm, fileName, fp, newchildGroupingNode, mO_id, "Oxygen dependent mortality for groups - based on ambient oxygen", "d-1", XML_TYPE_FLOAT,
			"0.1 - 0.3");
	Create_Species_ParamXML(bm, fileName, fp, newchildGroupingNode, KO2_id, "Lethal oxygen level", "mg O2 m-3", XML_TYPE_FLOAT, "0.0");
	Create_Species_ParamXML(bm, fileName, fp, newchildGroupingNode, KO2LIM_id, "Limiting oxygen level", "mg O2 m-3", XML_TYPE_FLOAT, "0.0");

	/* Init the FSBDR values to 0 for all verts */

	if(bm->containsBirds == TRUE){
	/* Create the new node */
		newnode = Util_XML_Create_Node(ATLANTIS_ATTRIBUTE, childGroupingNode, "FLAG_FSBDR",
				"Seabird sloppy feeding - Proportion of seabird catch of each group lost", "", "");

		Init_Vert_Species_Zero_ValuesXML(bm, newnode);
		Create_Species_ParamXML(bm, fileName, fp, childGroupingNode, FSBDR_id, "Seabird sloppy feedin", "", XML_TYPE_FLOAT, "0.05 - 0.2");
	}

	//# Demersal fish sloppy feeding
	Util_XML_Parse_Create_Node(fp, fileName, childGroupingNode, "FFDDR", "Proportion of static demersal fish's planktivore prey that's lost", "", XML_TYPE_FLOAT, "");
}

void createToleranceXML(MSEBoxModel *bm, FILE *fp, char *fileName, xmlDocPtr doc, xmlNodePtr rootnode) {
	xmlNodePtr groupingNode;

	groupingNode = Util_XML_Create_Node(ATLANTIS_ATTRIBUTE_SUB_GROUP, rootnode, "Tolerance", "Tolerances and boundaries", "", "");

	Util_XML_Parse_Create_Node(fp, fileName, groupingNode, "RelTol", "Relative tolerance of the adaptive differentiation scheme", "mg N m-3", XML_TYPE_FLOAT, "0.1");
	Util_XML_Parse_Create_Node(fp, fileName, groupingNode, "Flux_tol", "Tolerance of mass balance assumption per cell", "", XML_TYPE_FLOAT, "0.1");
	Util_XML_Parse_Create_Node(fp, fileName, groupingNode, "min_pool", "Minimum pool size", "", XML_TYPE_FLOAT, "0.00000001");
	Util_XML_Parse_Create_Node(fp, fileName, groupingNode, "min_dens", "Minimum density of top predators and fish", "", XML_TYPE_FLOAT, "0.0001");
	Util_XML_Parse_Create_Node(fp, fileName, groupingNode, "min_channel_depth", "Minimum depth of estuarine channels", "", XML_TYPE_FLOAT, "0.0001");
}

void createEvolutionXML (MSEBoxModel *bm, FILE *fp, char *fileName, xmlDocPtr doc, xmlNodePtr rootnode) {
	xmlNodePtr groupingNode;

	groupingNode = Util_XML_Create_Node(ATLANTIS_ATTRIBUTE_SUB_GROUP, rootnode, "Evolution", "Evolution", "", "");
    
	Util_XML_Parse_Create_Node(fp, fileName, groupingNode, "flag_do_var_express", "Whether do variable phenotypic expression (1) or not (0)", "", XML_TYPE_FLOAT, "0");
	Util_XML_Parse_Create_Node(fp, fileName, groupingNode, "flag_do_evolution", "Whether do evolution (1) or not (0)", "", XML_TYPE_FLOAT, "0");
	Util_XML_Get_Value_Integer(fileName, ATLANTIS_ATTRIBUTE, bm->ecotest, 1, groupingNode, binary_check, "flag_do_evolution", &bm->flag_do_evolution);
	Util_XML_Parse_Create_Node(fp, fileName, groupingNode, "flag_bound_change", "Whether evolution is effecting bound (1) or impact (0)", "", XML_TYPE_FLOAT, "0");
	Util_XML_Parse_Create_Node(fp, fileName, groupingNode, "flag_inheritance", "Flag indicating whether using complex (0) simple (1) or straight (2) inheritance model", "", XML_TYPE_FLOAT, "0");
	Util_XML_Parse_Create_Node(fp, fileName, groupingNode, "flag_evolvar_capped", "Flag indicating whether evolution variability capped (1) or not (0)", "", XML_TYPE_FLOAT, "0");
    
	Util_XML_Parse_Create_Node(fp, fileName, groupingNode, "evol_stdev_range", "Range of genotypes in terms of stdev", "", XML_TYPE_FLOAT, "0");
	Util_XML_Parse_Create_Node(fp, fileName, groupingNode, "max_rate_evol", "Maxium proportional rate of evolution", "", XML_TYPE_FLOAT, "0");

	Util_XML_Parse_Create_Node(fp, fileName, groupingNode, "flag_mult_grow_curves", "Whether using mutiple growth curves not evolution per se (1) or not (0)", "", XML_TYPE_FLOAT, "0");
	Util_XML_Get_Value_Integer(fileName, ATLANTIS_ATTRIBUTE, bm->ecotest, 1, groupingNode, binary_check, "flag_mult_grow_curves", &bm->flag_mult_grow_curves);

	if(bm->flag_do_evolution){
		Create_Species_ParamXML(bm, fileName, fp, groupingNode, max_prop_shift_id, "Maximum proportional change in a parameter", "", XML_TYPE_FLOAT, "0-1");
		Create_Species_ParamXML(bm, fileName, fp, groupingNode, inheritance_id, "Proportional inheritance index", "", XML_TYPE_FLOAT, "0-1");
		Create_Species_ParamXML(bm, fileName, fp, groupingNode, trait_variance_id, "Variance in a trait across genotypes", "", XML_TYPE_FLOAT, "0-1");
		Create_Species_ParamXML(bm, fileName, fp, groupingNode, min_trait_variance_id, "Minimum variance in a trait across genotypes", "", XML_TYPE_FLOAT, "0-1");

		Parse_File(bm, fp, fileName, groupingNode, "ActiveTrait", "^trait_active_[A-Z]{2,3}$", "Trait status", "", XML_TYPE_FLOATARRAY, K_num_traits, TRUE,
				Init_Species_Zero_ValuesXML, Species_Last_XMLFunction);

	}

}

void createFishRespirationXML(MSEBoxModel *bm, FILE *fp, char *fileName, xmlDocPtr doc, xmlNodePtr rootnode) {
	xmlNodePtr groupingNode;

	groupingNode = Util_XML_Create_Node(ATLANTIS_ATTRIBUTE_SUB_GROUP, rootnode, "Fish_Respiration", "Fish Respiration", "", "");

	Util_XML_Parse_Create_Node(fp, fileName, groupingNode, "KST_fish", "Factor respiration reduced by when fish are starved", "", XML_TYPE_FLOAT, "0.3");
	Util_XML_Parse_Create_Node(fp, fileName, groupingNode, "KST_shark", "Factor respiration reduced by when sharks are starved", "", XML_TYPE_FLOAT, "0.3");
	Util_XML_Parse_Create_Node(fp, fileName, groupingNode, "KST_bird", "Factor respiration reduced by when seabirds are starved", "", XML_TYPE_FLOAT, "0.3");
	Util_XML_Parse_Create_Node(fp, fileName, groupingNode, "KST_mammal", "Factor respiration reduced by when mammals are starved", "", XML_TYPE_FLOAT, "0.3");

	Util_XML_Parse_Create_Node(fp, fileName, groupingNode, "Ktmp_fish", "Factor relating temperature to respiration for fish", "", XML_TYPE_FLOAT, "0.08");
	Util_XML_Parse_Create_Node(fp, fileName, groupingNode, "Ktmp_shark", "Factor relating temperature to respiration for sharks", "", XML_TYPE_FLOAT, "0.075");
	Util_XML_Parse_Create_Node(fp, fileName, groupingNode, "Ktmp_bird", "Factor relating temperature to respiration for seabirds", "", XML_TYPE_FLOAT, "0.08");
	Util_XML_Parse_Create_Node(fp, fileName, groupingNode, "Ktmp_mammal", "Factor relating temperature to respiration for mammals", "", XML_TYPE_FLOAT, "0.08");
	Create_Species_ParamXML(bm, fileName, fp, groupingNode, KA_id, "Fish respiration - Scaling coefft in allometry of resp vs weight for groups", "", XML_TYPE_FLOAT,
			"0.01 - 0.025");
	Create_Species_ParamXML(bm, fileName, fp, groupingNode, KB_id, "Fish respiration - Exponent in respiration vs weight for groups", "", XML_TYPE_FLOAT, "0.8 - 0.85");
}

void createOverwinteringXML(MSEBoxModel *bm, FILE *fp, char *fileName, xmlDocPtr doc, xmlNodePtr rootnode) {
	xmlNodePtr groupingNode;

	groupingNode = Util_XML_Create_Node(ATLANTIS_ATTRIBUTE_SUB_GROUP, rootnode, "Encystment", "Encystment", "", "");

	Create_Species_ParamXML(bm, fileName, fp, groupingNode, overwinterStartTofY_id, "TofY start encystment", "", XML_TYPE_INTEGER, "0 - 364");
	Create_Species_ParamXML(bm, fileName, fp, groupingNode, overwinterEndTofY_id, "TofY end encystment", "", XML_TYPE_INTEGER, "0 - 364");
	Create_Species_ParamXML(bm, fileName, fp, groupingNode, overwinterStartTemp_id, "Temperature to trigger start of overwintering encystment", "", XML_TYPE_FLOAT, "0.0 - 10.0");
	Create_Species_ParamXML(bm, fileName, fp, groupingNode, overwinterEndTemp_id, "Temperature to trigger end of overwintering encystment", "", XML_TYPE_FLOAT, "0.0 - 10.0");
	Create_Species_ParamXML(bm, fileName, fp, groupingNode, crit_mum_id, "Critical growth rate, less than this and begin encystment", "", XML_TYPE_FLOAT, "0.0 - 0.5");
	Create_Species_ParamXML(bm, fileName, fp, groupingNode, crit_nut_id, "Critical nutrient level to trigger end of overwintering encystment", "", XML_TYPE_FLOAT, "0.0 - 20.0");
	Create_Species_ParamXML(bm, fileName, fp, groupingNode, crit_temp_id, "Critical temperature to trigger end of overwintering encystment", "", XML_TYPE_FLOAT, "0.0 - 10.0");
	Create_Species_ParamXML(bm, fileName, fp, groupingNode, encyst_rate_id, "Rate of encystment (as begin overwintering)", "", XML_TYPE_FLOAT, "0.0 - 1.0");
	Create_Species_ParamXML(bm, fileName, fp, groupingNode, hatch_rate_id, "Rate of hatching (as end overwintering)", "", XML_TYPE_FLOAT, "0.0 - 1.0");
	Create_Species_ParamXML(bm, fileName, fp, groupingNode, encyst_period_id, "Period of time remain encysted (if date driven)", "", XML_TYPE_INTEGER, "10 - 120");
	Create_Species_ParamXML(bm, fileName, fp, groupingNode, flagencyst_id, "Flag for overwintering - 0 = off, 1 = date trigger, 2 = temperature trigger", "", XML_TYPE_INTEGER, "0-2");
}

void createPrimaryProduceXML(MSEBoxModel *bm, FILE *fp, char *fileName, xmlDocPtr doc, xmlNodePtr groupingNode) {
	xmlNodePtr childGroupingNode;

	childGroupingNode = Util_XML_Create_Node(ATLANTIS_ATTRIBUTE_SUB_GROUP, groupingNode, "PrimaryProduction", "Primary Producer Parameters", "", "");

	Util_XML_Parse_Create_Node(fp, fileName, childGroupingNode, "MB_wc", "Scalar for microphytoben. growth in wc (reduced as not on substrate)", "", XML_TYPE_FLOAT,
			"0.1");
	Util_XML_Parse_Create_Node(fp, fileName, childGroupingNode, "eddy_scale", "Coefficient scaling eddy strength impact on phytoplankton growth", "", XML_TYPE_FLOAT,
			"1.0");

	Create_Species_ParamXML(bm, fileName, fp, childGroupingNode, KTUR_id, "Bioturbation contribution by the group.", "", XML_TYPE_FLOAT, "0.5 - 1.0");
	Create_Species_ParamXML(bm, fileName, fp, childGroupingNode, KIRR_id, "Bioirrigation contribution by the group", "", XML_TYPE_FLOAT, "0.2 - 1.0");
	Create_Species_ParamXML(bm, fileName, fp, childGroupingNode, KN_id, "Half-sat const for group growth on DIN", "", XML_TYPE_FLOAT, "4.0 - 200.0");
	Create_Species_ParamXML(bm, fileName, fp, childGroupingNode, KS_id, "Half-sat const for group growth on Si", "", XML_TYPE_FLOAT, "0 - 100");
	Create_Species_ParamXML(bm, fileName, fp, childGroupingNode, KF_id, "Half-sat const for group growth on Micro-nutrient ", "", XML_TYPE_FLOAT, "0 - 100");
	Create_Species_ParamXML(bm, fileName, fp, childGroupingNode, KI_T15_id, "Primary producer light requirements ", "W m-2", XML_TYPE_FLOAT, "3 - 65");

	if(bm->ice_on){
		childGroupingNode = Util_XML_Create_Node(ATLANTIS_ATTRIBUTE_SUB_GROUP, groupingNode, "IcePrimaryProduction", "Ice Primary Producer Parameters", "", "");
		Create_Species_ParamXML(bm, fileName, fp, childGroupingNode, Beta_D_id, "Photoinhibition", "W m-2", XML_TYPE_FLOAT, "0.002");
		Create_Species_ParamXML(bm, fileName, fp, childGroupingNode, PBmax_D_id, "Photosynthetic efficiency ", " ", XML_TYPE_FLOAT, "0.44");
	}

	if(bm->flag_macro_model) {
		Create_Species_ParamXML(bm, fileName, fp, childGroupingNode, KI_L_T15_id, "Light saturation for above-ground biomass", "", XML_TYPE_FLOAT, "");
		Create_Species_ParamXML(bm, fileName, fp, childGroupingNode, Kext_id, "Primary producer epiphyte light requirements ", "", XML_TYPE_FLOAT, "");
		Create_Species_ParamXML(bm, fileName, fp, childGroupingNode, Ksub_id, "Primary producer seagrass space limitations ", "", XML_TYPE_FLOAT, "");
		Create_Species_ParamXML(bm, fileName, fp, childGroupingNode, KNepi_id, "Primary producer seagrass epiphyte space limitations ", "", XML_TYPE_FLOAT, "");
		Create_Species_ParamXML(bm, fileName, fp, childGroupingNode, KsubEpi_id, "Primary producer seagrass epiphyte space limitations ", "", XML_TYPE_FLOAT, "");
		Create_Species_ParamXML(bm, fileName, fp, childGroupingNode, Ktrans_id, "Primary producer seagrass allocation across biomass pools ", "", XML_TYPE_FLOAT, "0.0 - 1.0");
	}

	if(bm->containsCoral){
		Create_Species_ParamXML(bm, fileName, fp, childGroupingNode, coral_overgrow_id, "Rate macroalgae overgrow coral", "d-1", XML_TYPE_FLOAT, "");
		Create_Species_ParamXML(bm, fileName, fp, childGroupingNode, coral_compete_id, "Rate macroalgae overgrow coral", "d-1", XML_TYPE_FLOAT, "");
	}
    if(bm->containsSponge){
        Create_Species_ParamXML(bm, fileName, fp, childGroupingNode, sponge_overgrow_id, "Rate overgrowth of sponge", "d-1", XML_TYPE_FLOAT, "");
        Create_Species_ParamXML(bm, fileName, fp, childGroupingNode, sponge_compete_id, "Limitation to sponge growth from corals and macroalgae", " ", XML_TYPE_FLOAT, "");
    }
}

void createBacteriaXML(MSEBoxModel *bm, FILE *fp, char *fileName, xmlDocPtr doc, xmlNodePtr groupingNode) {
	xmlNodePtr childGroupingNode, node, lookupNode;

	childGroupingNode = Util_XML_Create_Node(ATLANTIS_ATTRIBUTE_SUB_GROUP, groupingNode, "Bacteria", "Bacteria Parameters", "", "");
	Util_XML_Parse_Create_Node(fp, fileName, childGroupingNode, "XPB_DL", "Maximum ratio of PB to DL biomass", "", XML_TYPE_FLOAT, "0.3 - 0.4");
	Util_XML_Parse_Create_Node(fp, fileName, childGroupingNode, "XBB_DL", "Maximum ratio of BB to DL biomass", "", XML_TYPE_FLOAT, "0.15 - 0.2");
	Util_XML_Parse_Create_Node(fp, fileName, childGroupingNode, "XPB_DR", "Maximum ratio of PB to DR biomass", "", XML_TYPE_FLOAT, "0.01 - 0.015");
	Util_XML_Parse_Create_Node(fp, fileName, childGroupingNode, "XBB_DR", "Maximum ratio of BB to DR biomass", "", XML_TYPE_FLOAT, "0.005 - 0.01");
	node = Util_XML_Parse_Create_Node(fp, fileName, childGroupingNode, "k_PB", "Shape of PB growth eq'n, 1 = linear, 3 = drop before max, 5 = drop at max", "",
			XML_TYPE_LOOKUP, "3.0");
	lookupNode = Util_XML_Create_Lookup_Node(node);
	Util_XML_Create_Lookup_Value_Node(lookupNode, "1", "Linear");
	Util_XML_Create_Lookup_Value_Node(lookupNode, "3", "drop before max");
	Util_XML_Create_Lookup_Value_Node(lookupNode, "5", "drop at max");

	node = Util_XML_Parse_Create_Node(fp, fileName, childGroupingNode, "k_BB", "Shape of BB growth eq'n, 1 = linear, 3 = drop before max, 5 = drop at max", "",
			XML_TYPE_LOOKUP, "3.0");
	lookupNode = Util_XML_Create_Lookup_Node(node);
	Util_XML_Create_Lookup_Value_Node(lookupNode, "1", "Linear");
	Util_XML_Create_Lookup_Value_Node(lookupNode, "3", "drop before max");
	Util_XML_Create_Lookup_Value_Node(lookupNode, "5", "drop at max");
}

void createHabitatXML(MSEBoxModel *bm, FILE *fp, char *fileName, xmlDocPtr doc, xmlNodePtr rootnode) {
	xmlNodePtr groupingNode;
	xmlNodePtr childGroupingNode;

	groupingNode = Util_XML_Create_Node(ATLANTIS_ATTRIBUTE_SUB_GROUP, rootnode, "Habitat", "Habitat parameters", "", "");
	Util_XML_Parse_Create_Node(fp, fileName, groupingNode, "flaghabdepend", "General vertebrate demersal habitat dependency", "", XML_TYPE_BOOLEAN,"0");
	Util_XML_Parse_Create_Node(fp, fileName, groupingNode, "flag_move_habdepend", "General vertebrate demersal habitat dependency that influences movement", "", XML_TYPE_BOOLEAN,"0");
    Util_XML_Parse_Create_Node(fp, fileName, groupingNode, "flagenviro_displace", "Whether the environment displaces the species to adjacent box or disappear from the model", "", XML_TYPE_BOOLEAN,"0");
    Util_XML_Parse_Create_Node(fp, fileName, groupingNode, "flagenviro_kill", "Whether kill off species when environment is not suitable", "", XML_TYPE_BOOLEAN,"0");
 
	childGroupingNode = Util_XML_Create_Node( ATLANTIS_ATTRIBUTE_SUB_GROUP, groupingNode, "Habitat_Degradation",
					"This represents effects on suitable bottom types used by macroalgae, seagrass and benthic filter feeders due to human activities (e.g. reef removal, dredging)",
					"", "");

	Util_XML_Parse_Create_Node(fp, fileName, childGroupingNode, "REEFchange_max_num", "Number of changes in area of available reef habitat per coastal cell",
			"", XML_TYPE_INTEGER, "");
	Util_XML_Parse_Create_Node(fp, fileName, childGroupingNode, "FLATchange_max_num", "Number of changes in area of available flat bottom per coastal cell", "",
			XML_TYPE_INTEGER, "");
	Util_XML_Parse_Create_Node(fp, fileName, childGroupingNode, "SOFTchange_max_num", "Number of changes in area of available soft bottom per coastal cell", "",
			XML_TYPE_INTEGER, "");

	childGroupingNode
			= Util_XML_Create_Node(ATLANTIS_ATTRIBUTE_SUB_GROUP, groupingNode, "Habitat_Change",
					"Arrays to indicate start, period and proportion of change in available habitat for each bed type. There must be as many entries in this array as there are number of changes in area for that bed type.",
					"", "");
	Util_XML_Create_Node_Next_Line(fp, fileName, childGroupingNode, "REEFchange_start", "", "d", XML_TYPE_INTEGERARRAY);
	Util_XML_Create_Node_Next_Line(fp, fileName, childGroupingNode, "FLATchange_start", "", "d", XML_TYPE_INTEGERARRAY);
	Util_XML_Create_Node_Next_Line(fp, fileName, childGroupingNode, "SOFTchange_start", "", "d", XML_TYPE_INTEGERARRAY);

	//	childGroupingNode
	//			= Util_XML_Create_Node(ATLANTIS_ATTRIBUTE_SUB_GROUP,
	//					groupingNode,
	//					"Habitat_Change_Period",
	//					"Arrays to indicate period of change in available habitat for each bed type. Number of entries is equal to the number of changes in area for this bed type.", "", "");

	Util_XML_Create_Node_Next_Line(fp, fileName, childGroupingNode, "REEFchange_period", "", "d", XML_TYPE_INTEGERARRAY);
	Util_XML_Create_Node_Next_Line(fp, fileName, childGroupingNode, "FLATchange_period", "", "d", XML_TYPE_INTEGERARRAY);
	Util_XML_Create_Node_Next_Line(fp, fileName, childGroupingNode, "SOFTchange_period", "", "d", XML_TYPE_INTEGERARRAY);

	//	childGroupingNode
	//			= Util_XML_Create_Node(ATLANTIS_ATTRIBUTE_SUB_GROUP,
	//					groupingNode,
	//					"Habitat_Change_Mult",
	//					"Arrays to indicate proportional of change in available habitat for each bed type. Number of entries is equal to the number of changes in area for this bed type.", "", "");
	Util_XML_Create_Node_Next_Line(fp, fileName, childGroupingNode, "REEFchange_mult", "", "", XML_TYPE_FLOATARRAY);
	Util_XML_Create_Node_Next_Line(fp, fileName, childGroupingNode, "FLATchange_mult", "", "", XML_TYPE_FLOATARRAY);
	Util_XML_Create_Node_Next_Line(fp, fileName, childGroupingNode, "SOFTchange_mult", "", "", XML_TYPE_FLOATARRAY);

	Util_XML_Create_Node_Next_Line(fp, fileName, groupingNode, "Box_degraded", "Array indicating cells effected by coastal degradation", "", XML_TYPE_INTEGERARRAY);

	Create_Species_ParamXML(bm, fileName, fp, groupingNode, Kcov_juv_id, "Exponent of refuge relationship with biogenic habitat - juv value", "", XML_TYPE_FLOAT, "3.0");
	Create_Species_ParamXML(bm, fileName, fp, groupingNode, Bcov_juv_id, "Coefft of refuge relationship with biogenic habitat - juv value", "", XML_TYPE_FLOAT, "0.5 - 0.6");
	Create_Species_ParamXML(bm, fileName, fp, groupingNode, Acov_juv_id, "Scaling coefft of refuge relationship with biogenic habitat - juv value", "", XML_TYPE_FLOAT, "1.0");
	Create_Species_ParamXML(bm, fileName, fp, groupingNode, Kcov_ad_id, "Exponent of refuge relationship with biogenic habitat - adult value", "", XML_TYPE_FLOAT, "2.1 - 3.0");
	Create_Species_ParamXML(bm, fileName, fp, groupingNode, Bcov_ad_id, "Coefft of refuge relationship with biogenic habitat - adult value", "", XML_TYPE_FLOAT, "0.01 - 0.6");
	Create_Species_ParamXML(bm, fileName, fp, groupingNode, Acov_ad_id, "Scaling coefft of refuge relationship with biogenic habitat - adult value", "", XML_TYPE_FLOAT, "1.0");

	Parse_File(bm, fp,fileName, groupingNode, "Group_Habitat_Preference", "habitat_",
			"Vectors indicating which habitats each group prefers. 1=group lives on the habitat, 0=not present. The list of habitats are BFS, BFF, BFD, MA, SG, MB, reef, flat, soft, canyon",
			"", XML_TYPE_BOOLEANARRAY, bm->K_num_cover_types, TRUE, InitHabitatXML, HabitatXMLFunction);

	if(bm->ice_on){
		Parse_File(bm,
					fp,
					fileName,
					groupingNode,
					"Group_Ice_Preference",
					"ice_hab",
					"Vectors indicating which ice class each group prefers. 1=group lives on the habitat, 0=not present. Order is based on your ice classes.",
					"", XML_TYPE_BOOLEANARRAY, bm->K_num_ice_classes, TRUE, InitIceHabitatXML, IceHabitatXMLFunction);

		Parse_File(bm,
					fp,
					fileName,
					groupingNode,
					"Group_IceReprod_Preference",
					"ice_reprod_hab",
					"Vectors indicating which ice class each group prefers. 1=group lives on the habitat, 0=not present. Order is based on your ice classes.",
					"", XML_TYPE_BOOLEANARRAY, bm->K_num_ice_classes, TRUE, InitIceHabitatXML, IceHabitatXMLFunction);

	}

	if(bm->flag_adv_habitat) {
		Create_Species_ParamXML(bm, fileName, fp, groupingNode, RugCover_scalar_id, "Rugosity based habitat dependency scalar", "", XML_TYPE_FLOAT, "");
	}

	if(verbose > 1)
		printf("Done creating habitats\n");

}

void createSpatialThresholdXML(MSEBoxModel *bm, FILE *fp, char *fileName, xmlDocPtr doc, xmlNodePtr rootnode) {
	xmlNodePtr groupingNode;

	groupingNode = Util_XML_Create_Node(ATLANTIS_ATTRIBUTE_SUB_GROUP, rootnode, "Spatial_Threshold",
			"Threshold spatial factors for filter feeders if using ERSEM formulation", "", "");

	Create_Species_ParamXML(bm, fileName, fp, groupingNode, max_id, "Maximum Biomass", "mg.N.m-2", XML_TYPE_FLOAT, "1000 - 50000");
	Create_Species_ParamXML(bm, fileName, fp, groupingNode, sat_id, "Interference to uptake due to shading", "mg.N.m-2", XML_TYPE_FLOAT, "2500 - 3000");
	Create_Species_ParamXML(bm, fileName, fp, groupingNode, thresh_id, "", "mg.N.m-2", XML_TYPE_FLOAT, "1000");
	Create_Species_ParamXML(bm, fileName, fp, groupingNode, low_id, "Little space limitation (pop too small_num)", "mg.N.m-2", XML_TYPE_FLOAT, "2000 - 3000");

}

/** Read from csv file for migration */
 void createMigrationXML(MSEBoxModel *bm, FILE *fp, char *fileName, xmlDocPtr doc, xmlNodePtr rootnode) {
 xmlNodePtr groupingNode;
 
 // migration information in prm file
 groupingNode = Util_XML_Create_Node(ATLANTIS_ATTRIBUTE_SUB_GROUP, rootnode, "Migration", "The migration information", "", "");
 
 Util_XML_Parse_Create_Node(fp, fileName, groupingNode, "k_migslow", "0 = all migrators leave at once, 1 = migrators leave gradually", "", XML_TYPE_BOOLEAN,"0");

 Parse_File(bm, fp, fileName, groupingNode, "MigrateIOBox", "^MigIOBox_",
 "Proportion from each box in the model that exit when group migrates out of the model domain (default value will be 0.0 except for the few cells along the boundary where the group can flow out of the model. Juv stands for juveniles and ad for adults (treated seperately in the model so separate entries for them are given here)",
 "", XML_TYPE_FLOATARRAY, bm->nbox, FALSE, InitMigrateIOBoxXML, MigrateIOBoxXMLFunction);
  
 Parse_File(bm, fp, fileName, groupingNode, "KMIG_DEN", "^KMIGa_[A-Z]{2,3}$",
 "Numbers of each group that begin outside the model domain and must migrate in during the first year", "",
 XML_TYPE_FLOATARRAY, bm->K_num_max_cohort, FALSE,
 Init_Vert_Species_Zero_ValuesXML, KMIGXMLFunction);
 
 Parse_File(bm, fp, fileName, groupingNode, "KMIG_SN", "^KMIGa_.*sn$",
 "Reserve weights for each age group that begin outside the model domain and must migrate in during the first year", "", XML_TYPE_FLOATARRAY,
 bm->K_num_max_cohort, FALSE, Init_Vert_Species_Zero_ValuesXML, KMIGXMLFunction);
 Parse_File(bm, fp, fileName, groupingNode, "KMIG_RN", "^KMIGa_.*rn$",
 "Structural weights for each age group that begin outside the model domain and must migrate in during the first year", "", XML_TYPE_FLOATARRAY,
 bm->K_num_max_cohort, FALSE, Init_Vert_Species_Zero_ValuesXML, KMIGXMLFunction);
 
 Parse_File(bm, fp, fileName, groupingNode, "KMIG_INVERT", "^KMIGa_INVERT_",
 "Invertebrate migration parameters - biomass beginning outside model domain", "", XML_TYPE_FLOATARRAY,
 bm->K_num_max_cohort, FALSE, InitKMIGInvertXML, Species_Last_XMLFunction);
 
}

void createVertBodyXML(MSEBoxModel *bm, FILE *fp, char *fileName, xmlDocPtr doc, xmlNodePtr rootnode) {
	xmlNodePtr groupingNode;

	/* migration information */
	groupingNode = Util_XML_Create_Node(ATLANTIS_ATTRIBUTE_SUB_GROUP, rootnode, "VertebrateBody", "Vertebrate body form and aging detail", "", "");

	Util_XML_Parse_Create_Node(fp, fileName, groupingNode, "X_RS", "Ratio reserve to structure tissue in well fed vertebrates", "", XML_TYPE_FLOAT, "2.2 - 2.7");  // Still this for the static value
	Util_XML_Parse_Create_Node(fp, fileName, groupingNode, "Kthresh1", "Threshhold rel. reserve when handling time is reduced", "", XML_TYPE_FLOAT, "0.99");
	Util_XML_Parse_Create_Node(fp, fileName, groupingNode, "Kthresh2", "Threshhold rel. reserve when costs reduce + handling time inc", "", XML_TYPE_FLOAT, "0.92");
	Util_XML_Parse_Create_Node(fp, fileName, groupingNode, "KHTD", "Factor handling time reduced by for thresh2<relres<thesh1", "", XML_TYPE_FLOAT, "0.5");
	Util_XML_Parse_Create_Node(fp, fileName, groupingNode, "KHTI", "Factor handling time increased by for relres<thresh2", "", XML_TYPE_FLOAT, "1.5");
}

void createDietXML(MSEBoxModel *bm, FILE *fp, char *fileName, xmlDocPtr doc, xmlNodePtr rootnode) {
	xmlNodePtr groupingNode, childGroupingNode;
	xmlNodePtr node, lookupNode;

	groupingNode = Util_XML_Create_Node(ATLANTIS_ATTRIBUTE_SUB_GROUP, rootnode, "Diet", "The group diet information", "", "");

	// Predator species parameters
	node = Create_Species_ParamXML(bm, fileName, fp, groupingNode, predcase_id, "Predation formulation switches", "", XML_TYPE_LOOKUP, "0");
	lookupNode = Util_XML_Create_Lookup_Node(node);
	Util_XML_Create_Lookup_Value_Node(lookupNode, "0", "Holling type II");
	Util_XML_Create_Lookup_Value_Node(lookupNode, "1", "Holling type I");
	Util_XML_Create_Lookup_Value_Node(lookupNode, "2", "Holling type III");
	Util_XML_Create_Lookup_Value_Node(lookupNode, "3", "ECOSIM (currently disabled)");
	Util_XML_Create_Lookup_Value_Node(lookupNode, "4", "min-max");
	Util_XML_Create_Lookup_Value_Node(lookupNode, "5", "Size specific Holling type III");

	//	newnode = InitMammalXML(bm, groupingNode, "FLAG_CATCHEATE", "Flags indicating whether large mammals steal from fisheries catches", "");
	//	Create_Species_ParamXML(bm, fileName, fp, groupingNode, catcheater_id, "FLAG_CATCHEATE", "_catcheater$",
	//			"Flags indicating whether large mammals steal from fisheries catches", "", XML_TYPE_BOOLEAN,"1");

	node = Create_Species_ParamXML(bm, fileName, fp, groupingNode, flagactive_id,
			"Period of activity for consumer groups. Active: 2 = no preference, 1 = day, 0 = night", "", XML_TYPE_LOOKUP, "2");
	lookupNode = Util_XML_Create_Lookup_Node(node);
	Util_XML_Create_Lookup_Value_Node(lookupNode, "0", "night");
	Util_XML_Create_Lookup_Value_Node(lookupNode, "1", "day");
	Util_XML_Create_Lookup_Value_Node(lookupNode, "2", "no preference");

    InitSedimentFoodAvailXML(bm, groupingNode, "DetritusSedimentFoodAvail", "", "", XML_TYPE_FLOATARRAY, bm->K_num_detritus);
	Parse_File(bm, fp, fileName, groupingNode, "VertebrateFoodAvail", "^pPREY[12]", "Vertebrate food availability", "", XML_TYPE_FLOATARRAY, bm->K_num_tot_sp + bm->K_num_detritus, TRUE, InitspVertEatXML, spVERTEatXMLFunction);
	Parse_File(bm, fp, fileName, groupingNode, "InvertebrateFoodAvail", "^pPREY[A-Z]", "Invertebrate food availability", "", XML_TYPE_FLOATARRAY, bm->K_num_tot_sp + bm->K_num_detritus, TRUE, InitspInvertEatXML, spInvertEatXMLFunction);

	if(bm->flag_macro_model){
        /* Read in predation on seagrass */
        Parse_File(bm, fp, fileName, groupingNode, "SeagrassFoodAvail", "^Seagrass_pPREY[A-Z]", "Seagrass food availability", "", XML_TYPE_FLOATARRAY, bm->K_num_tot_sp, TRUE, InitspSeagrassXML, SeagrassPreyXMLFunction);
	}
    
    if(bm->flag_import_feed) {
        Parse_File(bm, fp,fileName, groupingNode, "Supplemental_Diets", "Supp_", "Vectors indicating distribution of supplementary feeds",
                   "", XML_TYPE_FLOATARRAY, bm->nbox, TRUE, InitSuppDietXML, SuppDietXMLFunction);
    }
    
    //newnode = Util_XML_Create_Node(ATLANTIS_ATTRIBUTE, groupingNode, "Cohort_Prey_Avail_Flag",  "Flag to indicate that the group uses age structured prey.", "", "");
	//Init_Vert_Species_Zero_ValuesXML(bm, newnode);

	if(bm->flag_fine_ontogenetic_diets){
		Create_Species_ParamXML(bm, fileName, fp, groupingNode, age_structured_prey_id, "Flag to indicate that the group uses age structured prey.", "", XML_TYPE_BOOLEAN,"1");
		Create_Species_ParamXML(bm, fileName, fp, groupingNode, p_split_id, "1=use explicit interactions above, 0=use ontogenetic values below", "", XML_TYPE_BOOLEAN,"1");
		Parse_File(bm, fp, fileName, groupingNode, "AgeDietAvail", "^p_[A-Z].*[^a-z]$", "Age Diet Avail", "", XML_TYPE_FLOATARRAY, bm->K_num_max_cohort,
				TRUE, InitDietAvailXML, AgeDietXMLFunction);
	}

	Parse_File(bm, fp, fileName, groupingNode, "pStock", "^pSTOCK_", "Variable availability per stock. Array length of each group is the number of stocks.", "",
			XML_TYPE_FLOATARRAY, -1, TRUE, InitAdJuvVertSpeciesXML, pStockXMLFunction);

    if(bm->flagIsEstuary) {
        Create_Species_ParamXML(bm, fileName, fp, groupingNode, turbid_refuge_id, "Flag indictaing whether the species uses turbidity as a refuge in predation", "", XML_TYPE_INTEGER, "");
        
        childGroupingNode = Util_XML_Create_Node(ATLANTIS_ATTRIBUTE_SUB_GROUP, groupingNode, "Turbidity", "Effect of turbidity on predation", "", "");
        
        Create_Species_Cohort_ParamXML(bm, fileName, fp, doc, childGroupingNode, L_turbid_id, "turbidity effect coefficient", "", XML_TYPE_FLOATARRAY, "", SpeciesXMLFunction);
        Create_Species_Cohort_ParamXML(bm, fileName, fp, doc, childGroupingNode, a_turbid_id, "turbidity effect exponent", "", XML_TYPE_FLOATARRAY, "", SpeciesXMLFunction);
        Create_Species_Cohort_ParamXML(bm, fileName, fp, doc, childGroupingNode, b_turbid_id, "turbidity effect constant", "", XML_TYPE_FLOATARRAY, "", SpeciesXMLFunction);
    }
    
    /* Do we have mammals in the model? */
	if(bm->containsMammals  == TRUE && bm->flag_fisheries_on){
		childGroupingNode = Util_XML_Create_Node(ATLANTIS_ATTRIBUTE_SUB_GROUP, groupingNode, "Catch_Opportunity",
					"Availability of catch to opportunistic catch grazers (thieves)", "", "");

		Parse_File(bm, fp, fileName, childGroupingNode, "Catch_Availability", "^pFC[A-Z]", "Availabilty of catch to opportunistic catch grazers (thieves)", "",
				XML_TYPE_FLOATARRAY, bm->K_num_tot_sp, TRUE, InitCatchGrazersXML, catchGrazersXML);

		Parse_File(bm, fp, fileName, childGroupingNode, "Proportion_Exploitable", "^PropCatch_",
				"Proportion of the catch in each fishery that can be exploited by the isCatchGrazer groups. Will be read in for groups that have the isCatchGrazer set to 1 in the functional group input file.",
				"", XML_TYPE_FLOATARRAY, bm->K_num_fisheries, TRUE, InitCatchPropGrazersXML, catchPropGrazersXML);
	}

	/* What about corals? */
	if(bm->containsCoral){
		childGroupingNode = Util_XML_Create_Node(ATLANTIS_ATTRIBUTE_SUB_GROUP, groupingNode, "Rugosity_Feeding",
				"Feeding effects on rugosity by coral predators", "", "");
		Create_Species_ParamXML(bm, fileName, fp, childGroupingNode, rugFeedScalar_id, "Rugosity feeding scalar", "", XML_TYPE_FLOAT, "0.0 - 1.0");
	}

    /* Aquaculture related */
    if(bm->flag_aquacult_on || bm->flag_import_feed){
        childGroupingNode = Util_XML_Create_Node(ATLANTIS_ATTRIBUTE_SUB_GROUP, groupingNode, "Extra_Feeding",
                                                 "Feed rates for aquaculture species or species feeding outside the model domain", "", "");
        Create_Species_ParamXML(bm, fileName, fp, childGroupingNode, extra_feed_id, "Supplemental feed rates for aquaculture or feeding outside the model domain", "proportion of body weight", XML_TYPE_FLOAT, "0.0 - 1.0");
    }
    
    //Availability of bacteria being cleaned from ingested detritus
	childGroupingNode = Util_XML_Create_Node(ATLANTIS_ATTRIBUTE_SUB_GROUP, groupingNode, "Bacteria_availability",
			"Availability of bacteria being cleaned from ingested detritus", "", "");

	Util_XML_Parse_Create_Node(fp, fileName, childGroupingNode, "p_BBfish", "Prop. of sed. bacteria taken when detrit eaten by verts.", "", XML_TYPE_FLOAT, "0.3 - 1.0");
	Util_XML_Parse_Create_Node(fp, fileName, childGroupingNode, "p_BBben", "Prop. of sed. bacteria taken when detrit eaten by benthos", "", XML_TYPE_FLOAT, "0.6 - 1.0");
	Util_XML_Parse_Create_Node(fp, fileName, childGroupingNode, "p_PBwc", "Prop. of pelag. bacteria taken when detrit eaten in wc", "", XML_TYPE_FLOAT, "0.6 - 1.0");
	Util_XML_Parse_Create_Node(fp, fileName, childGroupingNode, "p_PBben", "Prop. of pelag. bact. taken when detrit eaten by benthos", "", XML_TYPE_FLOAT, "0.6 - 1.0");

	Create_Species_ParamXML(bm, fileName, fp, groupingNode, KDEP_id, "Sediment penetration depth", "m", XML_TYPE_FLOAT, "0.0 - 1");

	Create_Species_ParamXML(bm, fileName, fp, groupingNode, KUP_id,
			"Gape size for vertebrates, to determine available prey fish groups - Upper limit. The size is proportional to the group length.", "", XML_TYPE_FLOAT,
			"0 - 15");
	Create_Species_ParamXML(bm, fileName, fp, groupingNode, KLP_id,
			"Gape size for vertebrates, to determine available prey fish groups - lower limit. The size is proportional to the group length.", "", XML_TYPE_FLOAT,
			"0 - 0.0001");

	if(!bm->UseHardFeedingWindow){
		Create_Species_ParamXML(bm, fileName, fp, groupingNode, Kmax_coefft_id,
				"Scalar coefficient on max pred-prey ratio for vertebrates if using a smooth curve, to determine available prey fish groups.", "", XML_TYPE_FLOAT,
				"0 - 10");
	}

	Parse_File_Single_Line(bm, fp, fileName, groupingNode, "InvertebrateSN", "_sn$",
			"Size of fish eating invertebrates (needed here as required in ecology and assessment libraries). There will be a value in the array for each age group.",
			"", XML_TYPE_FLOATARRAY, "", TRUE, InitInvertSpeciesZeroValuesXML, InvertebrateSNXMLFunction);

	childGroupingNode = Util_XML_Create_Node(ATLANTIS_ATTRIBUTE_SUB_GROUP, groupingNode, "Detritus_Handover", "Detritus handother deep demersal parameters",
			"", "");

	Parse_File_Single_Line(bm, fp, fileName, childGroupingNode, "K_TRANS", "k_trans",
			"Damping coefficient on rate of detritus transferral by benthic filter feeders", "", XML_TYPE_FLOAT, "", TRUE, InitKTransXML, Species_Last_XMLFunction);

	//Util_XML_Parse_Create_Node(fp, fileName, childGroupingNode, "k_trans", "Damping coefficient on rate of detritus transferral by benthic filter feeders", "");
	Util_XML_Parse_Create_Node(fp, fileName, childGroupingNode, "k_refDL",
			"Reference labile detrit. level in water column (if higher availability to omnivores reduced)", "", XML_TYPE_FLOAT, "10.0");
	Util_XML_Parse_Create_Node(fp, fileName, childGroupingNode, "k_refDR",
			"Reference refract. detrit. level in water column (if higher availability to omnivores reduced)", "", XML_TYPE_FLOAT, "20.0");
	Util_XML_Parse_Create_Node(fp, fileName, childGroupingNode, "k_refsDL",
			"Reference labile detrit. level in sediments (if higher availability to omnivores reduced)", "", XML_TYPE_FLOAT, "10000.0");

	/*# Clearance rate of each vertebrate group, per age class                mg3(mg N)-1 d-1*/
	Util_XML_Parse_Create_Node(fp, fileName, groupingNode, "flagfishrates",
			"Switch to show whether vertebrate clearance and growth rates are absolute=TRUE, or as proportion of body mass=FALSE", "", XML_TYPE_BOOLEAN,"0");

    Parse_File(bm, fp, fileName, groupingNode, "ClearanceRate", "^C_[A-Z]{2,3}$", "Clearance Rate", "mg3(mg N)-1 d-1", XML_TYPE_FLOATARRAY, bm->K_num_max_cohort, TRUE, Init_AgeStruct_Species_Zero_ValuesXML, Species_Last_XMLFunction);

    Parse_File_Single_Line(bm, fp, fileName, groupingNode, "InvertebrateClearanceRate", "^C_.*_T15$", "Invertebrate Clearance Rate", "mg3(mg N)-1 d-1",
            XML_TYPE_FLOATARRAY, "0.0002 - 0.4", TRUE, InitNonAgeSpeciesZeroValuesXML, InvertebrateClearanceRateXMLFunction);
    
    childGroupingNode = Util_XML_Create_Node(ATLANTIS_ATTRIBUTE_SUB_GROUP, groupingNode, "Growth_Rates",
            "Consumer growth - this is used by Holling type feeding (predcase 0, 1, 2)", "", "");

    Parse_File(bm, fp, fileName, childGroupingNode, "VertebrateGrowthRate", "^mum_[A-Z]{2,3}$",
                "Vertebrate Growth Rate. These are not corrected for temperature before use. Array length is the number of age groups.", "mg N d-1", XML_TYPE_FLOATARRAY,
                bm->K_num_max_cohort, TRUE, Init_AgeStruct_Species_Zero_ValuesXML, Species_Last_XMLFunction);
    
    Parse_File_Single_Line(bm, fp, fileName, childGroupingNode, "InvertebrateGrowthRate", "^mum_.*_T15$",
            "Invertebrate Growth Rate. These are corrected for temperature before use.", "mg N d-1", XML_TYPE_FLOATARRAY, "0.001 - 2.5", TRUE,
            InitNonAgeSpeciesZeroValuesXML, InvertebrateClearanceRateXMLFunction);

	Create_Species_ParamXML(bm, fileName, fp, groupingNode, KL_id, "Seed bank/population refuge based on consumer intake if available food below this feeding slows/stops so can't eat out all food. This is used by min-max feeding (predcase 4)",
			"mg N m-3", XML_TYPE_FLOAT, "0.05 - 4000");
	Create_Species_ParamXML(bm, fileName, fp, groupingNode, KU_id, "Saturation levels for consumer food intake. This is used by min-max feeding (predcase 4)", "mg N m-3", XML_TYPE_FLOAT, "");

	/* These two are invert only paramaters */
	InitInvertPredatorXML(bm, groupingNode, "FLAG_SEARCH_VOLUME", "Search volume - used by size specific Holling type III (predcase 5)", "m3 d-1");
	Create_Species_ParamXML(bm, fileName, fp, groupingNode, vl_id, "", "", XML_TYPE_FLOAT, "0.00005 - 0.4");

	Create_Species_ParamXML(bm, fileName, fp, groupingNode, vla_T15_id, "Coefficient of search volume", "", XML_TYPE_FLOAT, "1.0 - 5.0");
	Create_Species_ParamXML(bm, fileName, fp, groupingNode, vlb_id, "Exponent for search volume relation", "", XML_TYPE_FLOAT, "0.25 - 0.55");

	InitInvertPredatorXML(bm, groupingNode, "FLAG_HANDOVER_TIME",
			"Handother deep demersal time - used by size specific Holling type III (predcase 5)", "d");
	Create_Species_ParamXML(bm, fileName, fp, groupingNode, ht_id, "", "d", XML_TYPE_FLOAT, "0.14 - 100.0");

	Create_Species_ParamXML(bm, fileName, fp, groupingNode, hta_id, "Coefficient of handling deep demersal time", "", XML_TYPE_FLOAT, "10.0 - 250.0");
	Create_Species_ParamXML(bm, fileName, fp, groupingNode, htb_id, "Exponent for handling time relation", "", XML_TYPE_FLOAT, "0.1 - 0.63");

	Create_Species_ParamXML(bm, fileName, fp, groupingNode, pR_id, "Vertebrate preference for rebuilding reserves over structure", "", XML_TYPE_FLOAT, "2.0 - 5.0");
	Create_Species_ParamXML(bm, fileName, fp, groupingNode, li_a_id, "Coefficient of allometic length-weight relation", "", XML_TYPE_FLOAT, "0.0048 - 0.0178");
	Create_Species_ParamXML(bm, fileName, fp, groupingNode, li_b_id, "Exponent in allometic length-weight relation", "", XML_TYPE_FLOAT, "3.0");
	Create_Species_ParamXML(bm, fileName, fp, groupingNode, min_li_mat_id,
			"This is the minimum size (in cm) that FXX can spawn - if don't want this option on set it to 0.", "cm", XML_TYPE_FLOAT, "0");

    /* Read in dynamic RN/SN parameters */
    if (bm->flag_dynamicXRS) {
        Create_Species_ParamXML(bm, fileName, fp, groupingNode, RSmax_id, "Maximum RN/SN ratio possible", "", XML_TYPE_FLOAT, "1 - 2");
        Create_Species_ParamXML(bm, fileName, fp, groupingNode, RSmid_id, "Age in days at which RN is half the maximum", "", XML_TYPE_FLOAT, "");
        Create_Species_ParamXML(bm, fileName, fp, groupingNode, RSslope_id, "Steepness of the logistic function determining RN/SN", "", XML_TYPE_FLOAT, "");
        Create_Species_ParamXML(bm, fileName, fp, groupingNode, RSprop_id, "Proportion of RN used for reproduction", "", XML_TYPE_FLOAT, "0.5 - 0.9");
        Create_Species_ParamXML(bm, fileName, fp, groupingNode, SNcost_id, "Efficiency of energy transfer when allocating to SN (inverse of cost of growth)", "", XML_TYPE_FLOAT, "0.01 - 1");
        Create_Species_ParamXML(bm, fileName, fp, groupingNode, RNcost_id, "Efficiency of energy transfer when allocating to RN (inverse of cost of growth)", "", XML_TYPE_FLOAT, "0.01 - 1");
        Create_Species_ParamXML(bm, fileName, fp, groupingNode, RSstarve_id, "Exponent in starvation mortality function", "", XML_TYPE_FLOAT, "0.5-20");
    }
    
    if (bm->external_populations) {
        Create_Species_ParamXML(bm, fileName, fp, groupingNode, linf_id, "L-infinity for von Bertalanffy growth form for external population model", "", XML_TYPE_FLOAT, "");
        Create_Species_ParamXML(bm, fileName, fp, groupingNode, Kbert_id, "K for von Bertalanffy growth form for external population model", "", XML_TYPE_FLOAT, "");
        Create_Species_ParamXML(bm, fileName, fp, groupingNode, tzero_id, "t0 for von Bertalanffy growth form for external population model", "", XML_TYPE_FLOAT, "");
    }
    
    /* Read in the li_a and li_b values for inverts */
	Util_XML_Parse_Create_Node(fp, fileName, groupingNode, "li_a_invert", "Coefficient of allometic length-weight relation for inverts", "", XML_TYPE_FLOAT, "0.01");
	Util_XML_Parse_Create_Node(fp, fileName, groupingNode, "li_b_invert", "Exponent in allometic length-weight relation for inverts", "", XML_TYPE_FLOAT, "3.0");

	/* Assimilation_Efficiency parameters */
	childGroupingNode = Util_XML_Create_Node(ATLANTIS_ATTRIBUTE_SUB_GROUP, groupingNode, "Assimilation_Efficiency", "Assimilation efficiency values", "", "");

	//reateSpeciesParamXML(bm, fp, childGroupingNode, E2_id, "FLAG_EPLANT", "^EPlant_", "Assimilation efficiencies of consumers on plants", "");

	Create_Species_ParamXML(bm, fileName, fp, childGroupingNode, E1_id, "Assimilation efficiency of consumers on live food and carrion", "", XML_TYPE_FLOAT, "");

	Util_XML_Create_Node(ATLANTIS_ATTRIBUTE, childGroupingNode, "FLAG_EDL", "Assimilation efficiency of consumers on labile detritus", "", "");
	Create_Species_ParamXML(bm, fileName, fp, childGroupingNode, E3_id, "", "", XML_TYPE_FLOAT, "0.002 - 0.5");

	Util_XML_Create_Node(ATLANTIS_ATTRIBUTE, childGroupingNode, "FLAG_EDR", "Assimilation efficiency of consumers on refractory detritus", "", "");
	Create_Species_ParamXML(bm, fileName, fp, childGroupingNode, E4_id, "", "", XML_TYPE_FLOAT, "0.001 - 0.25");

	Create_Species_ParamXML(bm, fileName, fp, childGroupingNode, E2_id, "Assimilation efficiencies of consumers on plants", "", XML_TYPE_FLOAT, "0.01 - 0.9");

	node = Create_Species_ParamXML(bm, fileName, fp, childGroupingNode, flagq10eff_id,
			"Switch indicating whether or not efficiency of assimilation is temperature dependent", "", XML_TYPE_LOOKUP, "1");

	lookupNode = Util_XML_Create_Lookup_Node(node);
	Util_XML_Create_Lookup_Value_Node(lookupNode, "0", "no (same efficiency regardless)");
	Util_XML_Create_Lookup_Value_Node(lookupNode, "1", "poorer when cooler");
	Util_XML_Create_Lookup_Value_Node(lookupNode, "2", "poorer when warmer");

    node = Create_Species_ParamXML(bm, fileName, fp, childGroupingNode, flagq10receff_id, "Switch indicating in whcih way recruitment is temperature dependent", "", XML_TYPE_LOOKUP, "1");
    
    lookupNode = Util_XML_Create_Lookup_Node(node);
    Util_XML_Create_Lookup_Value_Node(lookupNode, "0", "no (same effect regardless)");
    Util_XML_Create_Lookup_Value_Node(lookupNode, "1", "poorer when cooler");
    Util_XML_Create_Lookup_Value_Node(lookupNode, "2", "poorer when warmer");
    
}

void createQ10XML(MSEBoxModel *bm, FILE *fp, char *fileName, xmlDocPtr doc, xmlNodePtr rootnode) {
	xmlNodePtr groupingNode, childGroupingNode;

	groupingNode = Util_XML_Create_Node(ATLANTIS_ATTRIBUTE_SUB_GROUP, rootnode, "MetabolicQ10", "The metabolic influences of environmental drivers", "", "");

	/* These values are not required as the Q10 values will default to the generic Q10 value */
	childGroupingNode = Util_XML_Create_Node(ATLANTIS_ATTRIBUTE_SUB_GROUP, groupingNode, "Temperature_Effects", "Environmental influence values for temperature", "", "");


	Util_XML_Parse_Create_Node(fp, fileName, childGroupingNode, "Q10", "Exponent in temperature effect on rate parameters", "", XML_TYPE_FLOAT, "2.0");
	Util_XML_Parse_Create_Node(fp, fileName, childGroupingNode, "temp_coefftB", "Coefficient B in Gary Griffith temperature function", "", XML_TYPE_FLOAT, "1.066");
	Util_XML_Parse_Create_Node(fp, fileName, childGroupingNode, "temp_coefftC", "Coefficient C in Gary Griffith temperature function", "", XML_TYPE_FLOAT, "0.5");
	Util_XML_Parse_Create_Node(fp, fileName, childGroupingNode, "temp_exp", "Exponent in Gary Griffith temperature function", "", XML_TYPE_FLOAT, "1.0");

	Util_XML_Create_Node(ATLANTIS_ATTRIBUTE, childGroupingNode, "FLAG_TEMP_COEFFTA", "Coefficient A in nonlinear temperature function", "", "");
	Create_Species_ParamXML(bm, fileName, fp, childGroupingNode, temp_coefftA_id, "", "", XML_TYPE_FLOAT, "");

	Parse_File_Single_Line(bm, fp, fileName, childGroupingNode, "FLAG_Q10", "^q10_([A-Z]{2,3})$", "Exponent in temperature effect on rate parameters", "", XML_TYPE_FLOATARRAY, "2",
			FALSE, InitSpeciesQ10ValuesXML, Species_Last_XMLFunction);

	Parse_File_Single_Line(bm, fp, fileName, childGroupingNode, "FLAG_Q10_METHOD", "^q10_method_", "The method of calculating Q10 0 is the 'normal' way of calculating it.1 is the 'new' climate change method from Gary G.", "", XML_TYPE_FLOATARRAY, "0", FALSE, Init_Species_Zero_ValuesXML, Species_Last_XMLFunction);
    Create_Species_ParamXML(bm, fileName, fp, childGroupingNode, q10_method_id, "", "", XML_TYPE_INTEGER, "0");

	Util_XML_Create_Node(ATLANTIS_ATTRIBUTE, childGroupingNode, "FLAG_Q10_OPTIMAL_TEMP", "Optimal temperature if use method 1.", "", "");
	Create_Species_ParamXML(bm, fileName, fp, childGroupingNode, q10_optimal_temp_id, "", "", XML_TYPE_FLOAT, "2.0 - 24.0");

	Util_XML_Create_Node(ATLANTIS_ATTRIBUTE, childGroupingNode, "FLAG_Q10_CORRECTION", "Correction factor if using q10 method 1.", "", "");
	Create_Species_ParamXML(bm, fileName, fp, childGroupingNode, q10_correction_id, "", "", XML_TYPE_FLOAT, "");

	Util_XML_Create_Node(ATLANTIS_ATTRIBUTE, childGroupingNode, "FLAG_TEMP_SENSITIVE", "Temperature sensitivty", "", "");
	Create_Species_ParamXML(bm, fileName, fp, childGroupingNode, flagtempsensitive_id, "", "", XML_TYPE_FLOAT, "");

	Util_XML_Create_Node(ATLANTIS_ATTRIBUTE, childGroupingNode, "FLAG_FECUND_SENSITIVE", "Whether the species fecundity is sensitive to pH", "", "");
	Create_Species_ParamXML(bm, fileName, fp, childGroupingNode, flagfecundsensitive_id, "", "", XML_TYPE_FLOAT, "");

	/* Salinity Effects */
	childGroupingNode = Util_XML_Create_Node(ATLANTIS_ATTRIBUTE_SUB_GROUP, groupingNode, "Salinity_Effects", "Environmental influence values for salinity", "", "");

	Util_XML_Create_Node(ATLANTIS_ATTRIBUTE, childGroupingNode, "FLAG_SALT_SENSITIVE", "Temperature sensitivty ", "", "");
	Create_Species_ParamXML(bm, fileName, fp, childGroupingNode, flagSaltSensitive_id, "", "", XML_TYPE_FLOAT, "");

	Util_XML_Create_Node(ATLANTIS_ATTRIBUTE, childGroupingNode, "FLAG_SALT_CORRECTION", "Correction factor for salinity corrections", "", "");
	Create_Species_ParamXML(bm, fileName, fp, childGroupingNode, salt_correction_id, "", "", XML_TYPE_FLOAT, "");

	/* pH Effects */
    if(bm->flagmodelpH){
        childGroupingNode = Util_XML_Create_Node(ATLANTIS_ATTRIBUTE_SUB_GROUP, groupingNode, "pH_Effects", "Environmental influence values for pH", "", "");

        Util_XML_Create_Node(ATLANTIS_ATTRIBUTE, childGroupingNode, "FLAG_PH_SENSITIVE", "Whether senstive to pH", "", "");
        Create_Species_ParamXML(bm, fileName, fp, childGroupingNode, flagpHsensitive_id, "", "", XML_TYPE_FLOAT, "");

        Util_XML_Create_Node(ATLANTIS_ATTRIBUTE, childGroupingNode, "FLAG_PH_SENSITIVEMODEL", "Which pH effects model to use", "", "");
        Create_Species_ParamXML(bm, fileName, fp, childGroupingNode, pHsensitive_model_id, "", "", XML_TYPE_FLOAT, "");

        Util_XML_Create_Node(ATLANTIS_ATTRIBUTE, childGroupingNode, "FLAG_PH_NUTVAL", "Whether the species nutritional value is sensitive to salinity or pH (mainly an issue for phytoplankton)", "", "");
        Create_Species_ParamXML(bm, fileName, fp, childGroupingNode, flagnutvaleffect_id, "", "", XML_TYPE_FLOAT, "");

        Util_XML_Create_Node(ATLANTIS_ATTRIBUTE, childGroupingNode, "FLAG_PH_CONSTA", "Coefficient A of the pH effects function", "", "");
        Create_Species_ParamXML(bm, fileName, fp, childGroupingNode, pH_constA_id, "", "", XML_TYPE_FLOAT, "");

        Util_XML_Create_Node(ATLANTIS_ATTRIBUTE, childGroupingNode, "FLAG_PH_CONSTB", "Coefficient B of the pH effects function", "", "");
        Create_Species_ParamXML(bm, fileName, fp, childGroupingNode, pH_constB_id, "", "", XML_TYPE_FLOAT, "");

        Util_XML_Create_Node(ATLANTIS_ATTRIBUTE, childGroupingNode, "FLAG_PH_CONSTC", "Coefficient B of the pH effects function", "", "");
        Create_Species_ParamXML(bm, fileName, fp, childGroupingNode, pH_constC_id, "", "", XML_TYPE_FLOAT, "");
        
        Util_XML_Create_Node(ATLANTIS_ATTRIBUTE, childGroupingNode, "FLAG_PH_KN", "Inflection point in moond form of pH effects function", "", "");
        Create_Species_ParamXML(bm, fileName, fp, childGroupingNode, KN_pH_id, "", "", XML_TYPE_FLOAT, "");

        Util_XML_Create_Node(ATLANTIS_ATTRIBUTE, childGroupingNode, "FLAG_PH_OPTIMAL", "Optimal pH level for the group", "", "");
        Create_Species_ParamXML(bm, fileName, fp, childGroupingNode, optimal_pH_id, "", "", XML_TYPE_FLOAT, "");

        Util_XML_Create_Node(ATLANTIS_ATTRIBUTE, childGroupingNode, "FLAG_PH_CORRECTION", "pH correction factor in nonlinear effects function", "", "");
        Create_Species_ParamXML(bm, fileName, fp, childGroupingNode, pH_correction_id, "", "", XML_TYPE_FLOAT, "");

        Util_XML_Create_Node(ATLANTIS_ATTRIBUTE, childGroupingNode, "FLAG_PH_CONTRACT_TOL", "Amount that pH change contracts salinity or temperature tolerance", "", "");
        Create_Species_ParamXML(bm, fileName, fp, childGroupingNode, contract_tol_id, "", "", XML_TYPE_FLOAT, "");
    
        Util_XML_Create_Node(ATLANTIS_ATTRIBUTE, childGroupingNode, "FLAG_MINPH", "Minimum pH for piece wise shape", "", "");
        Create_Species_ParamXML(bm, fileName, fp, childGroupingNode, min_pH_id, "", "", XML_TYPE_FLOAT, "");

        Util_XML_Create_Node(ATLANTIS_ATTRIBUTE, childGroupingNode, "FLAG_MAXPH", "Maximum pH for piece wise shape", "", "");
        Create_Species_ParamXML(bm, fileName, fp, childGroupingNode, max_pH_id, "", "", XML_TYPE_FLOAT, "");

        Util_XML_Create_Node(ATLANTIS_ATTRIBUTE, childGroupingNode, "FLAG_FLAGCONTRACT_TOL", "Whether the species growth or non predation mortality is sensitive to pH", "", "");
        Create_Species_ParamXML(bm, fileName, fp, childGroupingNode, flagcontract_tol_id, "", "", XML_TYPE_FLOAT, "");

        Util_XML_Create_Node(ATLANTIS_ATTRIBUTE, childGroupingNode, "FLAG_PH_TROPHSENSITIVE", "Whether the species availability to predators is sensitive to pH", "", "");
        Create_Species_ParamXML(bm, fileName, fp, childGroupingNode, flagpredavaileffect_id, "", "", XML_TYPE_FLOAT, "");

        Util_XML_Create_Node(ATLANTIS_ATTRIBUTE, childGroupingNode, "FLAG_PH_MORTSTART", "pH level where acidification induced mortality begins", "", "");
        Create_Species_ParamXML(bm, fileName, fp, childGroupingNode, pHmortstart_id, "", "", XML_TYPE_FLOAT, "");

        Util_XML_Create_Node(ATLANTIS_ATTRIBUTE, childGroupingNode, "FLAG_PH_MORTA", "maximum acidification mortality rate", "", "");
        Create_Species_ParamXML(bm, fileName, fp, childGroupingNode, pHmortA_id, "", "", XML_TYPE_FLOAT, "");

        Util_XML_Create_Node(ATLANTIS_ATTRIBUTE, childGroupingNode, "FLAG_PH_MORTB", "steepness of the acidification mortality curve", "", "");
        Create_Species_ParamXML(bm, fileName, fp, childGroupingNode, pHmortB_id, "", "", XML_TYPE_FLOAT, "");

        Util_XML_Create_Node(ATLANTIS_ATTRIBUTE, childGroupingNode, "FLAG_PH_MORTMID", "pH of midpoint of acidification mortality curve", "", "");
        Create_Species_ParamXML(bm, fileName, fp, childGroupingNode, pHmortmid_id, "", "", XML_TYPE_FLOAT, "");
    }
    
    // Pollutant effects
    if (bm->flag_pollutant_impacts) {
        Util_XML_Create_Node(ATLANTIS_ATTRIBUTE, childGroupingNode, "FLAG_LIGHT_COEFFT", "scalar for linear light impacts curve", "", "");
        Create_Species_ParamXML(bm, fileName, fp, childGroupingNode, light_coefft_id, "", "", XML_TYPE_FLOAT, "");
        
        Util_XML_Create_Node(ATLANTIS_ATTRIBUTE, childGroupingNode, "FLAG_NOISE_COEFFT", "scalar for linear noise impacts curve", "", "");
        Create_Species_ParamXML(bm, fileName, fp, childGroupingNode, noise_coefft_id, "", "", XML_TYPE_FLOAT, "");
    }

}


void createReproductionXML(MSEBoxModel *bm, FILE *fp, char *fileName, xmlDocPtr doc, xmlNodePtr rootnode) {
	xmlNodePtr groupingNode;
	int readinPopratioFlag;
	xmlNodePtr groupAttribute, attributeNode, node, lookupNode;

	groupingNode = Util_XML_Create_Node(ATLANTIS_ATTRIBUTE_SUB_GROUP, rootnode, "Reproduction", "Reproduction options", "", "");

	Util_XML_Parse_Create_Node(fp, fileName, groupingNode, "flagtrecruitdistrib", "0=humped recruit arrival distrib, 1=flat distrib of recruits", "", XML_TYPE_BOOLEAN,
			"1");

	groupAttribute = Parse_File(bm, fp, fileName, groupingNode, "KDENR", "^KDENR_",
					"Total number of new recruits for the year (used if recruit flag is set to 1)  or number produced per adult (used if recruit flag is set to 12). Array size is number of stocks for that species.",
					"", XML_TYPE_FLOATARRAY, -1, TRUE, Init_Age_Structured_SpeciesXML, Species_Last_XMLFunction);

	Create_Species_ParamXML(bm, fileName, fp, groupingNode, PP_id, "Primary production dependent recruitment", "", XML_TYPE_FLOAT, "");

	Create_Species_ParamXML(bm, fileName, fp, groupingNode, Ralpha_id, "Alpha in Ricker recruitment", "", XML_TYPE_FLOAT, "");
	Create_Species_ParamXML(bm, fileName, fp, groupingNode, Rbeta_id, "Beta in Ricker recruitment", "", XML_TYPE_FLOAT, "");

	Create_Species_ParamXML(bm, fileName, fp, groupingNode, BHalpha_id, "Alpha in Beverton-Holt recruitment", "", XML_TYPE_FLOAT, "");
	Create_Species_ParamXML(bm, fileName, fp, groupingNode, BHbeta_id, "Beta in Beverton-Holt recruitment", "", XML_TYPE_FLOAT, "");

	Create_Species_ParamXML(bm, fileName, fp, groupingNode, jack_a_id, "Slope in Jack-knife recruitment", "", XML_TYPE_FLOAT, "");
	Create_Species_ParamXML(bm, fileName, fp, groupingNode, jack_b_id, "Threshold biomass in Jack-knife recruitment", "", XML_TYPE_FLOAT, "");

    Create_Species_ParamXML(bm, fileName, fp, groupingNode, intersp_depend_recruit_id, "Flag indicating interspecies dependence of recruitment: 0 = none, 1 = linear, 2 = inverse", "", XML_TYPE_INTEGER, "");
	Create_Species_ParamXML(bm, fileName, fp, groupingNode, intersp_depend_sp_id, "ID of species recruitment dependent on", "", XML_TYPE_INTEGER, "");
	Create_Species_ParamXML(bm, fileName, fp, groupingNode, intersp_depend_scale_id, "Scalar for dependency of recruitment", "", XML_TYPE_FLOAT, "");

	Create_Species_ParamXML(bm, fileName, fp, groupingNode, rec_HabDepend_id, "If recruitment is habitat dependent", "", XML_TYPE_FLOAT, "");

	Create_Species_ParamXML(bm, fileName, fp, groupingNode, prop_spawn_lost_id, "Proportion of age structured biomass pool lost in spawning process", "", XML_TYPE_FLOAT, "0-1");
    
    /* Aquaculture related */
	if(bm->flag_aquacult_on){
		Create_Species_ParamXML(bm, fileName, fp, groupingNode, aquacult_fry_id, "Stocking rates for aquaculture", "", XML_TYPE_FLOAT, "");
	}

	Parse_File(bm, fp, fileName, groupingNode, "recSTOCK", "^recSTOCK_",
			"Stock specific recruitment scalars - to represent differential recruitment effectiveness for different stocks. Maybe contributing factor for regional differences in stock size, but required in calibration occassionally. If don't believe there is stock specific differences set array cell contents to 1.0",
			"", XML_TYPE_FLOATARRAY, -1, TRUE, Init_Vert_Species_Zero_ValuesXML, Species_Last_XMLFunction);
	Create_Species_ParamXML(bm, fileName, fp, groupingNode, recover_mult_id, "Recruitment modifiers to 'encourage recovery' of stocks", "", XML_TYPE_FLOAT, "1.0");

	Util_XML_Parse_Create_Node(fp, fileName, groupingNode, "recover_trigger", "Prop. of init. stock remaining that triggers 'encourage recovery'", "", XML_TYPE_FLOAT,
			"");
	Util_XML_Parse_Create_Node(fp, fileName, groupingNode, "recover_span", "Number of days from initial trigger until recover_mult used", "days", XML_TYPE_FLOAT, "");
	Util_XML_Parse_Create_Node(fp, fileName, groupingNode, "recover_subseq", "Number of years in a row where get increased recruitment", "years", XML_TYPE_FLOAT, "");

	Create_Species_ParamXML(bm, fileName, fp, groupingNode, recover_start_id, "Starting time of perscribed recruitment 'recovery' of stocks", "day", XML_TYPE_INTEGER, "");

	//# Log-normal random recruitment
	Util_XML_Parse_Create_Node(fp, fileName, groupingNode, "lognorm_mu", "Centre for lognormal distribution of the number of recruits", "", XML_TYPE_FLOAT, "");
	Util_XML_Parse_Create_Node(fp, fileName, groupingNode, "lognorm_sigma", "Variation of lognormal distribution of the number of recruits", "", XML_TYPE_FLOAT, "");

	Create_Species_ParamXML(bm, fileName, fp, groupingNode, log_mult_id, "Log-normal random recruitment", "", XML_TYPE_FLOAT, "");
    Create_Species_ParamXML(bm, fileName, fp, groupingNode, norm_sigma_id, "Sigma for normal random recruitment", "", XML_TYPE_FLOAT, "");
    Create_Species_ParamXML(bm, fileName, fp, groupingNode, flag_recruit_stochastic_id, "Flag indicating whether to add normal random noise to recruitment", "", XML_TYPE_INTEGER, "");

    if(bm->ice_on) {
        Create_Species_ParamXML(bm, fileName, fp, groupingNode, prod_alpha_id, "Productivity scalar in lake recruitment function", "", XML_TYPE_FLOAT, "");
        Create_Species_ParamXML(bm, fileName, fp, groupingNode, den_depend_beta1_id, "Density dependent coefficient in lake recruitment function", "", XML_TYPE_FLOAT, "");
        Create_Species_ParamXML(bm, fileName, fp, groupingNode, den_depend_beta2_id, "Second density dependent coefficient in lake recruitment function", "", XML_TYPE_FLOAT, "");
        Create_Species_ParamXML(bm, fileName, fp, groupingNode, temp_coefft_id, "Thermal index coefficient in lake recruitment function", "", XML_TYPE_FLOAT, "");
        Create_Species_ParamXML(bm, fileName, fp, groupingNode, rate_coefft_id, "Air temperature rate of change coefficient in lake recruitment function", "", XML_TYPE_FLOAT, "");
        Create_Species_ParamXML(bm, fileName, fp, groupingNode, wind_coefft_id, "Wind coefficient in lake recruitment function", "", XML_TYPE_FLOAT, "");
        Create_Species_ParamXML(bm, fileName, fp, groupingNode, recruit_var_id, "Recruitment variability for use in lake recruitment function", "", XML_TYPE_FLOAT, "");
    }
    
	Create_Species_ParamXML(bm, fileName, fp, groupingNode, KWSR_id, "Structural weight of group recruits", "mg N m-3", XML_TYPE_FLOAT, "0.01 - 117362680.0");
	Create_Species_ParamXML(bm, fileName, fp, groupingNode, KWRR_id, "Reserve weight of group recruits", "mg N m-3", XML_TYPE_FLOAT, "0.038 -  311011104.0");

    
    if (bm->flagtempdepend_reprod) {
        Create_Species_ParamXML(bm, fileName, fp, groupingNode, min_spawn_temp_id, "Minimum spawning temperature", "deg C", XML_TYPE_FLOAT, "4.0 - 22.0");
        Create_Species_ParamXML(bm, fileName, fp, groupingNode, max_spawn_temp_id, "Maximum spawning temperature", "deg C", XML_TYPE_FLOAT, "21.0 - 25.0");
    }

	if (bm->flagsaltdepend) {
		Create_Species_ParamXML(bm, fileName, fp, groupingNode, min_spawn_salt_id, "Minimum spawning salinity", "", XML_TYPE_FLOAT, "");
		Create_Species_ParamXML(bm, fileName, fp, groupingNode, max_spawn_salt_id, "Maximum spawning salinity", "", XML_TYPE_FLOAT, "");
	}

	if(bm->containsMCPYr){
		groupAttribute = Parse_File(bm, fp, fileName, groupingNode, "FLAG_TIME_AGE", "^Time_Age_", "Day of year that the group ages.", "day", XML_TYPE_FLOATARRAY,
				bm->K_num_max_cohort, TRUE, InitAGESTRUCTUREDBIOMASSSpeciesXML, Species_Last_XMLFunction);
	}

	Create_Species_ParamXML(bm, fileName, fp, groupingNode, age_mat_id, "First mature age class", "", XML_TYPE_INTEGER, "1 - 4");

	/* These are only read in if the readin_popratio flag is true */
	groupAttribute = Util_XML_Get_Node(ATLANTIS_ATTRIBUTE_SUB_GROUP, rootnode, "Setup");
	if (groupAttribute == NULL)
		quit("Node steup not found\n");
	attributeNode = Util_XML_Get_Node(ATLANTIS_ATTRIBUTE, groupAttribute, "readin_popratio");
	readinPopratioFlag = Util_XML_Get_Node_Value_Int(fileName, ATLANTIS_ATTRIBUTE, attributeNode);

	if (readinPopratioFlag == TRUE)
		Parse_File(bm, fp, fileName, groupingNode, "popratioStock", "popratioStock[1-9][0-9]{0,2}$",
				"Vectors of pre-existing age distirbutions (how numbers within age class are split up by years in that class)", "", XML_TYPE_FLOATARRAY,
				bm->K_num_stocks_per_sp, TRUE, InitPopRatioStockXML, PopRatioStockXMLFunction);

	Parse_File(bm, fp, fileName, groupingNode, "VerticalRecruitLocation", "_recruit_vdistrib",
			"Recruit location vertical distibutions - uses appropriate VERTday or VERTnight entry from movement parameters above", "", XML_TYPE_FLOATARRAY, bm->wcnz,
			TRUE, Init_Vert_Species_Zero_ValuesXML, SpeciesXMLFunction);
//}
//
//void createReproductionXML(MSEBoxModel *bm, FILE *fp, char *fileName, xmlDocPtr doc, xmlNodePtr rootnode) {
//	xmlNodePtr groupingNode, parentNode, node, lookupNode;


//	groupingNode = Util_XML_Create_Node(ATLANTIS_ATTRIBUTE_SUB_GROUP, rootnode, "Reproduction", "Reproduction variables", "", "");

	node = Create_Species_ParamXML(bm, fileName, fp, groupingNode, flagmother_id, "Flag indicating whether the vertebrate group provides parental care for young until maturity (0 = no, 1 = yes, -1 = semelparous so die after reproduction)", "", XML_TYPE_LOOKUP, "");

	lookupNode = Util_XML_Create_Lookup_Node(node);
	Util_XML_Create_Lookup_Value_Node(lookupNode, "0", "no");
	Util_XML_Create_Lookup_Value_Node(lookupNode, "1", "yes");
	Util_XML_Create_Lookup_Value_Node(lookupNode, "-1", "semelparous so die after reproduction");

	node = Create_Species_ParamXML(bm, fileName, fp, groupingNode, flagrecruit_id,
			"Vertebrate reproduction related flags. The flagrecruit entries refer to the recruitment function used.", "", XML_TYPE_LOOKUP, "3");

	lookupNode = Util_XML_Create_Lookup_Node(node);
	Util_XML_Create_Lookup_Value_Node(lookupNode, "1", "Constant");
	Util_XML_Create_Lookup_Value_Node(lookupNode, "2", "dependent on prim producers (Chla)");
	Util_XML_Create_Lookup_Value_Node(lookupNode, "3", "Beverton-Holt");
	Util_XML_Create_Lookup_Value_Node(lookupNode, "4", "lognormal");
	Util_XML_Create_Lookup_Value_Node(lookupNode, "5", "dependent on all plankton groups not just Chla");
	Util_XML_Create_Lookup_Value_Node(lookupNode, "6", "Bev-Holt with lognormal variation added");
	Util_XML_Create_Lookup_Value_Node(lookupNode, "7", "Bev-Holt with 'encourage recovery'");
	Util_XML_Create_Lookup_Value_Node(lookupNode, "8", "Bev-Holt with perscribed 'recovery'");
	Util_XML_Create_Lookup_Value_Node(lookupNode, "9", "Ricker");
	Util_XML_Create_Lookup_Value_Node(lookupNode, "10", "Standard Bev-Holt (no explict use of spawn included)");
	Util_XML_Create_Lookup_Value_Node(lookupNode, "11", "pupping/calving linearly dependent on maternal condition");
	Util_XML_Create_Lookup_Value_Node(lookupNode, "12", "pupping/calving a fixed number per adult spawning");
	Util_XML_Create_Lookup_Value_Node(lookupNode, "13", "forced timeseries of recruitment");

	node = Create_Species_ParamXML(bm, fileName, fp, groupingNode, flagrecpeak_id, "Vertebrate reproduction strength flags.", "", XML_TYPE_BOOLEAN,"1");

	lookupNode = Util_XML_Create_Lookup_Node(node);
	Util_XML_Create_Lookup_Value_Node(
			lookupNode,
			"0",
			"only moderate variation in year class strength possible, mainly for top predators with few young per reproductive event, relative strength set using recruitRangeFlat");
	Util_XML_Create_Lookup_Value_Node(lookupNode, "1", "very strong year classes possible, relative strength set using recruitRange");

	//node = Create_Species_ParamXML(bm, fileName, fp, groupingNode, flaglocalrecruit_id, "Local recruitment", "", XML_TYPE_INTEGER, "0");
	//lookupNode = Util_XML_Create_Lookup_Node(node);
	//Util_XML_Create_Lookup_Value_Node(lookupNode, "0", "independent distribution");
	//Util_XML_Create_Lookup_Value_Node(lookupNode, "1", "demersal and piscivorous fish recruit at parental locations");
	//Util_XML_Create_Lookup_Value_Node(lookupNode, "2", "external reproduction");

    Create_Species_ParamXML(bm, fileName, fp, groupingNode, flagstocking_id, "Flag indicating whether stocking recruits or not", "", XML_TYPE_BOOLEAN, "0");
    
    Create_Species_ParamXML(bm, fileName, fp, groupingNode, flagkeep_plusgroup_id, "Flag indicating whether to keep the plus gorup if it hasn't returned to the model yet", "", XML_TYPE_BOOLEAN, "0");

	Create_Species_ParamXML(bm, fileName, fp, groupingNode, flagbearlive_id, "Flag indicating whether the vertebrate group bears live young", "", XML_TYPE_BOOLEAN,"");
	Create_Species_ParamXML(bm, fileName, fp, groupingNode, feed_while_spawn_id, "Flag indicating whether feed during the spawning period", "", XML_TYPE_BOOLEAN,"0");

	Create_Species_Spawn_ParamXML(bm, fileName, fp, doc, groupingNode, Time_Spawn_id, "Day of year the group spawns", "day", XML_TYPE_INTEGER, "0 - 360", SpeciesXMLFunction);

	Create_Species_ParamXML(bm, fileName, fp, groupingNode, spawn_period_id, "Period of spawning in days", "days", XML_TYPE_INTEGER, "1 - 365");
	Create_Species_ParamXML(bm, fileName, fp, groupingNode, Recruit_Time_id, "Length of larval period in days", "days", XML_TYPE_INTEGER, "20 - 720");
	Create_Species_ParamXML(bm, fileName, fp, groupingNode, Recruit_Period_id, "Length of time recruits arrive over", "days", XML_TYPE_INTEGER, "15 - 90");
    
    Create_Species_ParamXML(bm, fileName, fp, groupingNode, cohort_recruit_entry_id, "Cohort recruits enter the model", "", XML_TYPE_INTEGER, "0");

	Create_Species_ParamXML(bm, fileName, fp, groupingNode, KSPA_id, "Constant in spawning formulation", "", XML_TYPE_FLOAT, "31.93 - 38376.9");
	Create_Species_ParamXML(bm, fileName, fp, groupingNode, FSP_id, "Fraction used in spawning formulation", "", XML_TYPE_FLOAT, "0.16 - 0.425");

	Parse_File(bm, fp, fileName, groupingNode, "SpawningProportion", "^FSPB_", "Proportion of the population (per age) spawning", "", XML_TYPE_FLOATARRAY,
			bm->K_num_max_cohort, TRUE, Init_Vert_Species_Zero_ValuesXML, Species_Last_XMLFunction);

	Parse_File(bm, fp, fileName, groupingNode, "RecruitDistribution", "_recruit_hdistrib$",
			"Recruit location spatial distributions. One value per functional group per box in the model.", "", XML_TYPE_FLOATARRAY, bm->nbox, TRUE,
			Init_Vert_Species_Zero_ValuesXML, SpeciesXMLFunction);

    if(bm->flag_aquacult_on){
        Parse_File(bm, fp, fileName, groupingNode, "AquacultDistribution", "_aquacult_hdistrib$",
                   "Recruit location spatial distributions. One value per aquaculture group per box in the model.", "", XML_TYPE_FLOATARRAY, bm->nbox, TRUE,
                   Init_Cultured_Zero_ValuesXML, SpeciesXMLFunction);
    }
    
	//# Distribution of recruits within recruitment period
	Util_XML_Parse_Create_Node(fp, fileName, groupingNode, "rec_m", "", "", XML_TYPE_FLOAT, "");
	Util_XML_Parse_Create_Node(fp, fileName, groupingNode, "rec_sigma", "", "", XML_TYPE_FLOAT, "");
	Util_XML_Parse_Create_Node(fp, fileName, groupingNode, "recruitRange", "max relative cohort strength (multiplicative) for groups with strong year classes",
			"", XML_TYPE_FLOAT, "");
	Util_XML_Parse_Create_Node(fp, fileName, groupingNode, "recruitRangeFlat",
			"max relative cohort strength (multiplicative) for groups without strong year classes", "", XML_TYPE_FLOAT, "");

	Util_XML_Parse_Create_Node(fp, fileName, groupingNode, "ref_chl", "Reference level of chla", "", XML_TYPE_FLOAT, "1.5");
}

void createStockStructureXML(MSEBoxModel *bm, FILE *fp, char *fileName, xmlDocPtr doc, xmlNodePtr rootnode) {
		xmlNodePtr groupingNode;
	groupingNode = Util_XML_Create_Node(ATLANTIS_ATTRIBUTE_SUB_GROUP, rootnode, "Stock_Structure", "Stock Structure", "", "");
	Parse_File(bm, fp, fileName, groupingNode, "StockStructure", "[A-Z]{2,3}_stock_struct$", "Stock Structure. One value per functional group per box.", "",
			XML_TYPE_INTEGERARRAY, bm->nbox, TRUE, Init_Vert_Species_Zero_ValuesXML, SpeciesXMLFunction);

	Parse_File(bm,
				fp,
				fileName,
				groupingNode,
				"VerticalStockStructure",
				".*_vert_stock_struct$",
				"Vertical Stock Structure First entry for each vector refers to water column layer closest to the sediment and then up through the watercolumn to surface most layer on the extreme right",
				"", XML_TYPE_INTEGERARRAY, bm->wcnz, TRUE, Init_Vert_Species_Zero_ValuesXML, SpeciesXMLFunction);

    Util_XML_Create_Node_Next_Line(
			fp,
			fileName,
			groupingNode,
			"regID",
			"Regional output information - when want overall Biom.txt file split by region too Biomass reporting regions - give the region each box will be counted in (so if two regions each box must be either in region 0 or 1 etc)",
			"", XML_TYPE_INTEGERARRAY);
    
}
void createDetritusConversionXML(MSEBoxModel *bm, FILE *fp, char *fileName, xmlDocPtr doc, xmlNodePtr rootnode) {
	xmlNodePtr groupingNode;
	xmlNodePtr newnode;

	groupingNode = Util_XML_Create_Node(ATLANTIS_ATTRIBUTE_SUB_GROUP, rootnode, "Detritus_Conversion", "Conversion to detritus", "", "");

	newnode = Util_XML_Create_Node(ATLANTIS_ATTRIBUTE, groupingNode, "FLAG_FDG",
			"Fraction of non-assimilate lost to detritus  - Detrit contrib from unassim material ingested by group", "", "");
	InitInvertSpeciesZeroValuesXML(bm, newnode);
	Create_Species_ParamXML(bm, fileName, fp, groupingNode, FDG_id, "", "", XML_TYPE_FLOAT, "0.1 - 0.5");

	Create_Species_ParamXML(bm, fileName, fp, groupingNode, FDGDL_id,
			"Fraction of non-assimilate from labile detrital food lost to detritus  - Detrit contrib from unassim lab detrit ingested by group", "", XML_TYPE_FLOAT,
			"0.0 - 0.9");
	Create_Species_ParamXML(bm, fileName, fp, groupingNode, FDGDR_id, "Fraction of non-assimilate from refractory detrital food lost to detritus ", "", XML_TYPE_FLOAT,
			"0.5 - 0.95");

	newnode = Util_XML_Create_Node(ATLANTIS_ATTRIBUTE, groupingNode, "FLAG_FDM",
			"Fraction of mortality products lost to detritus - Detrit contrib from mortality of group", "", "");
	InitInvertSpeciesZeroValuesXML(bm, newnode);
	Create_Species_ParamXML(bm, fileName, fp, groupingNode, FDMort_id, "", "", XML_TYPE_FLOAT, "0.2 - 0.6");

	//# Split up of losses to detritus between labile and refractory detritus
	Util_XML_Parse_Create_Node(fp, fileName, groupingNode, "FDL_fish", "Split between DL and DR for fish", "", XML_TYPE_FLOAT, "0.0 - 1.0");
	Util_XML_Parse_Create_Node(fp, fileName, groupingNode, "FDL_benth", "Split between DL and DR for benthos", "", XML_TYPE_FLOAT, "0.0 - 1.0");
	Util_XML_Parse_Create_Node(fp, fileName, groupingNode, "FDL_top", "Split between DL and DR for top predators", "", XML_TYPE_FLOAT, "0.0 - 1.0");
	Util_XML_Parse_Create_Node(fp, fileName, groupingNode, "FDL_wc", "Split between DL and DR for pelagic invertebrates", "", XML_TYPE_FLOAT, "0.0 - 1.0");

	if(bm->flag_macro_model){
		Util_XML_Parse_Create_Node(fp, fileName, groupingNode, "FDL_SG_roots", "Split between DL and DR for Seagrass roots.", "", XML_TYPE_FLOAT, "0.3");
		Util_XML_Parse_Create_Node(fp, fileName, groupingNode, "FDL_SG_leaves", "Split between DL and DR for Seagrass leaves", "", XML_TYPE_FLOAT, "0.5");
	}

	//# Fraction of bacterial losses sent to detritus
	Util_XML_Parse_Create_Node(fp, fileName, groupingNode, "FPB_DR", "Detrit contrib from pelagic bacteria", "", XML_TYPE_FLOAT, "0.2");
	Util_XML_Parse_Create_Node(fp, fileName, groupingNode, "FBB_DR", "Detrit contrib from sediment bacteria", "", XML_TYPE_FLOAT, "0.2");

	//# Fraction of bacterial losses sent to DON
	Util_XML_Parse_Create_Node(fp, fileName, groupingNode, "FPB_DON", "DON contrib from pelagic bacteria", "", XML_TYPE_FLOAT, "0.05");
	Util_XML_Parse_Create_Node(fp, fileName, groupingNode, "FBB_DON", "DON contrib from sediment bacteria", "", XML_TYPE_FLOAT, "0.05");

	//# Fraction of infaunal NH involved in denitrification
	Util_XML_Parse_Create_Node(fp, fileName, groupingNode, "Fben_den", "Fraction of NH release from infauna used in nitrif etc calcs", "", XML_TYPE_FLOAT, "0.5 - 0.7");

	Create_Species_ParamXML(bm, fileName, fp, groupingNode, sp_remin_contrib_id, "", "", XML_TYPE_FLOAT, "0.2 - 0.6");

}

void createCoralXML(MSEBoxModel *bm, FILE *fp, char *fileName, xmlDocPtr doc, xmlNodePtr rootnode) {
	xmlNodePtr groupingNode, childGroupingNode;

	groupingNode = Util_XML_Create_Node(ATLANTIS_ATTRIBUTE_SUB_GROUP, rootnode, "CoralReef", "Reef related information", "", "");

	/* Bleaching related */
	childGroupingNode = Util_XML_Create_Node(ATLANTIS_ATTRIBUTE_SUB_GROUP, groupingNode, "Bleaching", "Bleaching parameters", "", "");

	Create_Species_ParamXML(bm, fileName, fp, childGroupingNode, bleach_periodA_id, "Quick recovery period (regain original zooxanthellae)", "days", XML_TYPE_FLOAT, "1 - 2");
	Create_Species_ParamXML(bm, fileName, fp, childGroupingNode, bleach_periodB_id, "Slow recovery period (regain new zooxanthellae)", "days", XML_TYPE_FLOAT, "10 - 200");

	Create_Species_ParamXML(bm, fileName, fp, childGroupingNode, mBleach_id, "Slow recovery period (regain new zooxanthellae)", "days", XML_TYPE_FLOAT, "10 - 200");
	Create_Species_ParamXML(bm, fileName, fp, childGroupingNode, bleaching_rate_id, "Bleaching rate", "d-1", XML_TYPE_FLOAT, "");
	Create_Species_ParamXML(bm, fileName, fp, childGroupingNode, bleach_recovery_rate_id, "Bleaching recovery rate", "d-1", XML_TYPE_FLOAT, "10 - 200");
	Create_Species_ParamXML(bm, fileName, fp, childGroupingNode, bleach_tempshift_id, "Temperature shift with new zooxanthellae", "", XML_TYPE_FLOAT, "0.0 - 1.0");
	Create_Species_ParamXML(bm, fileName, fp, childGroupingNode, bleach_growshift_id, "Growth shift with new zooxanthellae", "", XML_TYPE_FLOAT, "0.0 - 1.0");
	Create_Species_ParamXML(bm, fileName, fp, childGroupingNode, bleach_temp_id, "Bleaching temperature threshold", "degrees C", XML_TYPE_FLOAT, "25 - 40");
    Create_Species_ParamXML(bm, fileName, fp, childGroupingNode, min_bleach_temp_id, "Minimum bleaching temperature threshold", "degrees C", XML_TYPE_FLOAT, "25");
    Create_Species_ParamXML(bm, fileName, fp, childGroupingNode, DHW_thresh_id, "Degree heating weeks threshold", "", XML_TYPE_FLOAT, "4.0 - 12.0");

    Create_Species_ParamXML(bm, fileName, fp, childGroupingNode, threshdepth_id, "Depth threshold for shifting growth state", "", XML_TYPE_FLOAT, "0.0 - 200.0");
    Create_Species_ParamXML(bm, fileName, fp, childGroupingNode, depmum_scalar_id, "Scalar for shifting growth state", "", XML_TYPE_FLOAT, "0.0 - 2.0");
    
    Create_Species_ParamXML(bm, fileName, fp, childGroupingNode, min_bleach_salt_id, "Minimum bleaching salinity threshold", "ppt", XML_TYPE_FLOAT, "40");
    Create_Species_ParamXML(bm, fileName, fp, childGroupingNode, max_bleach_salt_id, "Maximum bleaching salinity threshold", "ppt", XML_TYPE_FLOAT, "45");
    
    /* Other coral related */
	Create_Species_ParamXML(bm, fileName, fp, groupingNode, prop_zooxanth_id, "Proportion of coral biomass that is zooxanthellae", "", XML_TYPE_FLOAT, "0.0 - 1.0");
	Create_Species_ParamXML(bm, fileName, fp, groupingNode, HostRemin_id, "Remineralisation rate of host respiration", "d-1", XML_TYPE_FLOAT, "0.0 - 1.0");

	Create_Species_ParamXML(bm, fileName, fp, groupingNode, FeedLightThresh_id, "Light threshold", "W", XML_TYPE_FLOAT, "0.0 - 400.0");
	Create_Species_ParamXML(bm, fileName, fp, groupingNode, PropLightFeed_id, "Proportion of night feeding done when in daylight", "", XML_TYPE_FLOAT, "0.0 - 1.0");

	/* Calcification related */
	childGroupingNode = Util_XML_Create_Node(ATLANTIS_ATTRIBUTE_SUB_GROUP, groupingNode, "Calcification", "Calcification parameters", "", "");

	Create_Species_ParamXML(bm, fileName, fp, childGroupingNode, calcifRefBaseline_id, "Calcification baseline rate", "d-1", XML_TYPE_FLOAT, "");
	Create_Species_ParamXML(bm, fileName, fp, childGroupingNode, calcifTconst_id, "Calcification temperature constant", "", XML_TYPE_FLOAT, "");
	Create_Species_ParamXML(bm, fileName, fp, childGroupingNode, calcifTcoefft_id, "Calcification temperature coefficient", "", XML_TYPE_FLOAT, "");
	Create_Species_ParamXML(bm, fileName, fp, childGroupingNode, calcifTopt_id, "Calcification optimal temperature", "deg C", XML_TYPE_FLOAT, "");
	Create_Species_ParamXML(bm, fileName, fp, childGroupingNode, calcifLambda_id, "Calcification lambda", "", XML_TYPE_FLOAT, "");

	/* Coral spatial competition related */
	childGroupingNode = Util_XML_Create_Node(ATLANTIS_ATTRIBUTE_SUB_GROUP, groupingNode, "ReefGrowth", "Coral accelerated growth potential", "", "");

	Create_Species_ParamXML(bm, fileName, fp, childGroupingNode, coral_max_accel_trans_id, "Max proportion of accelerated growth", "", XML_TYPE_FLOAT, "0.0 - 1.0");
	Create_Species_ParamXML(bm, fileName, fp, childGroupingNode, coral_max_accelA_id, "Steepness of accelerated growth", "", XML_TYPE_FLOAT, "");
	Create_Species_ParamXML(bm, fileName, fp, childGroupingNode, coral_max_accelB_id, "Midpoint of accelerated growth", "", XML_TYPE_FLOAT, "");

	/* Recruitment related */
	childGroupingNode = Util_XML_Create_Node(ATLANTIS_ATTRIBUTE_SUB_GROUP, groupingNode, "ReefRecruit", "Coral recruitment parameters", "", "");

	Create_Species_ParamXML(bm, fileName, fp, childGroupingNode, CrecruitA_id, "Coral recruitment scalar", "", XML_TYPE_FLOAT, "");
	Create_Species_ParamXML(bm, fileName, fp, childGroupingNode, CrecruitB_id, "Coral recruitment constant", "", XML_TYPE_FLOAT, "");
	Create_Species_ParamXML(bm, fileName, fp, childGroupingNode, CrecruitC_id, "Coral recruitment exponent", "", XML_TYPE_FLOAT, "");

	/* Rugosity related */
	childGroupingNode = Util_XML_Create_Node(ATLANTIS_ATTRIBUTE_SUB_GROUP, groupingNode, "RugosityModel", "Rugosity parameters", "", "");

	Create_Species_ParamXML(bm, fileName, fp, childGroupingNode, rug_factor_id, "Rugosity surface area scalar", "", XML_TYPE_FLOAT, "");
	Create_Species_ParamXML(bm, fileName, fp, childGroupingNode, rug_erode_id, "Cryptic coral erosion rate", "d-1", XML_TYPE_FLOAT, "");
	Create_Species_ParamXML(bm, fileName, fp, childGroupingNode, rug_bleacherode_id, "Cryptic bleached coral erosion rate", "d-1", XML_TYPE_FLOAT, "");
	Create_Species_ParamXML(bm, fileName, fp, childGroupingNode, colony_ha_id, "Constant scaling colony height to diameter", "", XML_TYPE_FLOAT, "");
    Create_Species_ParamXML(bm, fileName, fp, childGroupingNode, rug_erode_sponge_id, "Rugosity erosion rate due to bio-eroding sponges", "", XML_TYPE_FLOAT, "");

	/*
	Parse_File(bm, fp, fileName, childGroupingNode, "RugosityIncRate", "^rugosity_inc_",
			"Rate of increase in rugosity", "d-1", XML_TYPE_FLOATARRAY, bm->K_num_max_stages, FALSE, InitRugosityRateXML,
			Species_Last_XMLFunction);
	Parse_File(bm, fp, fileName, childGroupingNode, "RugosityDecRate", "^rugosity_dec_",
			"Rate at which rugosity drops as coral is degraded", "d-1", XML_TYPE_FLOATARRAY, bm->K_num_max_stages, FALSE, InitRugosityRateXML,
			Species_Last_XMLFunction);
	*/
	Create_Species_Cohort_ParamXML(bm, fileName, fp, doc, childGroupingNode, rugosity_inc_id, "Rate of increase in rugosity", "d-1", XML_TYPE_FLOATARRAY, "0 - 1.0", Species_Last_XMLFunction);
	Create_Species_Cohort_ParamXML(bm, fileName, fp, doc, childGroupingNode, rugosity_dec_id, "Rate at which rugosity drops as coral is degraded", "d-1", XML_TYPE_FLOATARRAY, "0 - 1.0", Species_Last_XMLFunction);
	Create_Species_Cohort_ParamXML(bm, fileName, fp, doc, childGroupingNode, colony_diam_id, "Colony diameter", "cm", XML_TYPE_FLOATARRAY, "1.0 - 150.0", Species_Last_XMLFunction);
    
    childGroupingNode = Util_XML_Create_Node(ATLANTIS_ATTRIBUTE_SUB_GROUP, groupingNode, "SpongeSmother", "Sponge and smothering parameters", "", "");
    
    Create_Species_ParamXML(bm, fileName, fp, childGroupingNode, Ksmother_B_id, "Coefficient of smothering", "d-1", XML_TYPE_FLOAT, "");
    Create_Species_ParamXML(bm, fileName, fp, childGroupingNode, Ksmother_A_id, "Constant in smothering relationship", "d-1", XML_TYPE_FLOAT, "");
    
    if(bm->containsSponge){
        Create_Species_ParamXML(bm, fileName, fp, childGroupingNode, Vmax_deltaSi_id, "Vmax to mimic shape of a Michaelis-Menten Si uptake, as per Maldonado et al. (2011)", " ", XML_TYPE_FLOAT, "");
        Create_Species_ParamXML(bm, fileName, fp, childGroupingNode, Km_deltaSi_id, "Km (max concentration of Si)", " ", XML_TYPE_FLOAT, "");
    }

    
}

void createIceXML(MSEBoxModel *bm, FILE *fp, char *fileName, xmlDocPtr doc, xmlNodePtr rootnode) {
	xmlNodePtr groupingNode;

	groupingNode = Util_XML_Create_Node(ATLANTIS_ATTRIBUTE_SUB_GROUP, rootnode, "Ice_Related", "Ice related values", "", "");

	Create_Species_ParamXML(bm, fileName, fp, groupingNode, ICE_KDEP_id, "", "", XML_TYPE_FLOAT, "0.2 - 0.6");
	Util_XML_Parse_Create_Node(fp, fileName, groupingNode, "p_IBice", "Prop. of ice. bact. taken when detrit eaten by predators", "", XML_TYPE_FLOAT, "0.6 - 1.0");

}

/**
 * Create the contaminant XML.
 *
 */
void createContaminantsXML(MSEBoxModel *bm, FILE *fp, char *fileName, xmlDocPtr doc, xmlNodePtr rootnode) {
	xmlNodePtr groupingNode;
	int cIndex, sp;
	char varStr[2*STRLEN];
	char longStr[2*STRLEN];

	groupingNode = Util_XML_Create_Node(ATLANTIS_ATTRIBUTE_SUB_GROUP, rootnode, "Contaminant_Related", "Contaminant related values", "", "");

    Util_XML_Parse_Create_Node(fp, fileName, groupingNode, "flag_dissolved_pollutants", "Flag indicating whether the contaminants are dissolved and transmitted in excretion (1) or not (0)", "", XML_TYPE_INTEGER,"0");
    Util_XML_Parse_Create_Node(fp, fileName, groupingNode, "flag_contamMortModel", "Flag indicating which contaminant mortality is being used - (1) simple, (0) InVitro", "", XML_TYPE_INTEGER,"0");
    Util_XML_Parse_Create_Node(fp, fileName, groupingNode, "flag_contamInteractModel", "Flag indicating which contaminant interaction model is being used - (0) none, (1) additive, (2) multiplicative, (3) most limiting", "", XML_TYPE_INTEGER,"0");
    Util_XML_Parse_Create_Node(fp, fileName, groupingNode, "flag_contamGrowthModel", "Flag indicating which contaminant growth effects model is being used - (0) InVitro, (1) logistic", "", XML_TYPE_INTEGER, "1");
    Util_XML_Parse_Create_Node(fp, fileName, groupingNode, "flag_contamReprodModel", "Flag indicating which contaminant reprod effects model is being used - (0) off (1) Lovindeer paper equation (2) InVitro, (3) logistic", "", XML_TYPE_INTEGER, "1");
    Util_XML_Parse_Create_Node(fp, fileName, groupingNode, "flag_contamOnlyAmplify", "Flag indicating whether interacting contaminants can only amplify (1) or also buffer (0) each other", "", XML_TYPE_INTEGER,"0");
    Util_XML_Parse_Create_Node(fp, fileName, groupingNode, "flag_contamMove", "Flag indicating avoidance scalar used - none (0), knife-edge (1), sigmoidal (2), left shoulder flat top (3)", "", XML_TYPE_INTEGER,"0");
    Util_XML_Parse_Create_Node(fp, fileName, groupingNode, "flag_contamMinTemp", "Minimum temperature to consider in contaminants relationships that involve temperature corrections", "", XML_TYPE_FLOAT,"0");
    Util_XML_Parse_Create_Node(fp, fileName, groupingNode, "flag_contam_halflife_spbased", "Whether half life of contaminants is species specific", "", XML_TYPE_FLOAT,"0");
    bm->flag_contam_halflife_spbased = (int) Util_XML_Read_Value(fileName, ATLANTIS_ATTRIBUTE, bm->ecotest, 1, groupingNode, integer_check, "flag_contam_halflife_spbased");
    
    Util_XML_Parse_Create_Node(fp, fileName, groupingNode, "flag_contamMaternalTransfer", "Whether maternal transfer included", "", XML_TYPE_FLOAT,"0");
    Util_XML_Parse_Create_Node(fp, fileName, groupingNode, "flag_contam_distrib", "Whether contaminants track an average or a distreibution", "", XML_TYPE_FLOAT,"0");

    Util_XML_Parse_Create_Node(fp, fileName, groupingNode, "biopools_dodge_contam", "Flag to turn on whetehr biomass pool groups can dodge contaminants..", "", XML_TYPE_BOOLEAN,"0");
    
    Util_XML_Parse_Create_Node(fp, fileName, groupingNode, "contam_tau", "Tau coefficient", "", XML_TYPE_FLOAT, "1");

    Util_XML_Parse_Create_Node(fp, fileName, groupingNode, "contam_sig_uptake_const", "Sigmoid uptake function coefficient", "", XML_TYPE_FLOAT, "1");

    Util_XML_Parse_Create_Node(fp, fileName, groupingNode, "flag_detritus_contam", "Flag indicating whether allow contaminant uptake from detritus or not", "", XML_TYPE_INTEGER, "1");
        
    for(cIndex = 0; cIndex < bm->num_contaminants; cIndex++){
		for(sp = 0; sp < bm->K_num_tot_sp; sp++){
			if(FunctGroupArray[sp].speciesParams[flag_id] == TRUE && FunctGroupArray[sp].isDetritus == FALSE){

                if(bm->flag_contam_halflife_spbased) {
                    sprintf(varStr, "%s_%s_decay_half_life", FunctGroupArray[sp].groupCode, bm->contaminantStructure[cIndex]->contaminant_name);
                    sprintf(longStr, "Species specific half life value of group %s for contaminant %s.", FunctGroupArray[sp].groupCode, bm->contaminantStructure[cIndex]->contaminant_name);
                    Util_XML_Parse_Create_Node(fp, fileName, groupingNode, varStr, longStr, "", XML_TYPE_FLOAT, "");
                }
				sprintf(varStr, "%s_%s_uptake_rate", FunctGroupArray[sp].groupCode, bm->contaminantStructure[cIndex]->contaminant_name);
				sprintf(longStr, "Uptake rate of group %s of contaminant %s.", FunctGroupArray[sp].groupCode, bm->contaminantStructure[cIndex]->contaminant_name);
				Util_XML_Parse_Create_Node(fp, fileName, groupingNode, varStr, longStr, "", XML_TYPE_FLOAT, "");

				sprintf(varStr, "%s_%s_uptake_option", FunctGroupArray[sp].groupCode, bm->contaminantStructure[cIndex]->contaminant_name);
				sprintf(longStr, "Uptake option of group %s of contaminant %s.", FunctGroupArray[sp].groupCode, bm->contaminantStructure[cIndex]->contaminant_name);
				Util_XML_Parse_Create_Node(fp, fileName, groupingNode, varStr, longStr, "", XML_TYPE_INTEGER, "");

				sprintf(varStr, "%s_%s_LD50", FunctGroupArray[sp].groupCode, bm->contaminantStructure[cIndex]->contaminant_name);
				sprintf(longStr, "LD50 value of group %s of contaminant %s.", FunctGroupArray[sp].groupCode, bm->contaminantStructure[cIndex]->contaminant_name);
				Util_XML_Parse_Create_Node(fp, fileName, groupingNode, varStr, longStr, "", XML_TYPE_FLOAT, "");

				sprintf(varStr, "%s_%s_LD100", FunctGroupArray[sp].groupCode, bm->contaminantStructure[cIndex]->contaminant_name);
				sprintf(longStr, "LD100 value of group %s of contaminant %s.", FunctGroupArray[sp].groupCode, bm->contaminantStructure[cIndex]->contaminant_name);
				Util_XML_Parse_Create_Node(fp, fileName, groupingNode, varStr, longStr, "", XML_TYPE_FLOAT, "");

                sprintf(varStr, "%s_%s_LDChronic", FunctGroupArray[sp].groupCode, bm->contaminantStructure[cIndex]->contaminant_name);
                sprintf(longStr, "LDchronic value of group %s of contaminant %s.", FunctGroupArray[sp].groupCode, bm->contaminantStructure[cIndex]->contaminant_name);
                Util_XML_Parse_Create_Node(fp, fileName, groupingNode, varStr, longStr, "", XML_TYPE_FLOAT, "");

                sprintf(varStr, "%s_%s_TimeToLD50", FunctGroupArray[sp].groupCode, bm->contaminantStructure[cIndex]->contaminant_name);
				sprintf(longStr, "Time to LD50 value of group %s of contaminant %s.", FunctGroupArray[sp].groupCode, bm->contaminantStructure[cIndex]->contaminant_name);
				Util_XML_Parse_Create_Node(fp, fileName, groupingNode, varStr, longStr, "", XML_TYPE_FLOAT, "");

                sprintf(varStr, "%s_%s_LDslope", FunctGroupArray[sp].groupCode, bm->contaminantStructure[cIndex]->contaminant_name);
                sprintf(longStr, "slope of LD50 function of value of group %s of contaminant %s.", FunctGroupArray[sp].groupCode, bm->contaminantStructure[cIndex]->contaminant_name);
                Util_XML_Parse_Create_Node(fp, fileName, groupingNode, varStr, longStr, "", XML_TYPE_FLOAT, "");
                
                sprintf(varStr, "%s_%s_EC50", FunctGroupArray[sp].groupCode, bm->contaminantStructure[cIndex]->contaminant_name);
                sprintf(longStr, "EC50 (50 percent growth reduction) value of group %s of contaminant %s.", FunctGroupArray[sp].groupCode, bm->contaminantStructure[cIndex]->contaminant_name);
                Util_XML_Parse_Create_Node(fp, fileName, groupingNode, varStr, longStr, "", XML_TYPE_FLOAT, "");
                
                sprintf(varStr, "%s_%s_ECslope", FunctGroupArray[sp].groupCode, bm->contaminantStructure[cIndex]->contaminant_name);
                sprintf(longStr, "slope of EC50 function of value of group %s of contaminant %s.", FunctGroupArray[sp].groupCode, bm->contaminantStructure[cIndex]->contaminant_name);
                Util_XML_Parse_Create_Node(fp, fileName, groupingNode, varStr, longStr, "", XML_TYPE_FLOAT, "");
                
                sprintf(varStr, "%s_%s_EC50_r", FunctGroupArray[sp].groupCode, bm->contaminantStructure[cIndex]->contaminant_name);
                sprintf(longStr, "EC50 (50 percent reprod reduction) value of group %s of contaminant %s.", FunctGroupArray[sp].groupCode, bm->contaminantStructure[cIndex]->contaminant_name);
                Util_XML_Parse_Create_Node(fp, fileName, groupingNode, varStr, longStr, "", XML_TYPE_FLOAT, "");
                
                sprintf(varStr, "%s_%s_ECslope_r", FunctGroupArray[sp].groupCode, bm->contaminantStructure[cIndex]->contaminant_name);
                sprintf(longStr, "slope of EC50_r function of value of group %s of contaminant %s.", FunctGroupArray[sp].groupCode, bm->contaminantStructure[cIndex]->contaminant_name);
                Util_XML_Parse_Create_Node(fp, fileName, groupingNode, varStr, longStr, "", XML_TYPE_FLOAT, "");
                
                sprintf(varStr, "%s_%s_spL", FunctGroupArray[sp].groupCode, bm->contaminantStructure[cIndex]->contaminant_name);
                sprintf(longStr, "growth effects model minimum (L) value for group %s of contaminant %s.", FunctGroupArray[sp].groupCode, bm->contaminantStructure[cIndex]->contaminant_name);
                Util_XML_Parse_Create_Node(fp, fileName, groupingNode, varStr, longStr, "", XML_TYPE_FLOAT, "");
                
                sprintf(varStr, "%s_%s_spA", FunctGroupArray[sp].groupCode, bm->contaminantStructure[cIndex]->contaminant_name);
                sprintf(longStr, "growth effects model slope (A) value for group %s of contaminant %s.", FunctGroupArray[sp].groupCode, bm->contaminantStructure[cIndex]->contaminant_name);
                Util_XML_Parse_Create_Node(fp, fileName, groupingNode, varStr, longStr, "", XML_TYPE_FLOAT, "");
                
                sprintf(varStr, "%s_%s_spB", FunctGroupArray[sp].groupCode, bm->contaminantStructure[cIndex]->contaminant_name);
                sprintf(longStr, "growth effects model midpoint (B) value for group %s of contaminant %s.", FunctGroupArray[sp].groupCode, bm->contaminantStructure[cIndex]->contaminant_name);
                Util_XML_Parse_Create_Node(fp, fileName, groupingNode, varStr, longStr, "", XML_TYPE_FLOAT, "");

                sprintf(varStr, "%s_%s_spL_r", FunctGroupArray[sp].groupCode, bm->contaminantStructure[cIndex]->contaminant_name);
                sprintf(longStr, "Reprod effects model minimum (L) value for group %s of contaminant %s.", FunctGroupArray[sp].groupCode, bm->contaminantStructure[cIndex]->contaminant_name);
                Util_XML_Parse_Create_Node(fp, fileName, groupingNode, varStr, longStr, "", XML_TYPE_FLOAT, "");
                
                sprintf(varStr, "%s_%s_spA_r", FunctGroupArray[sp].groupCode, bm->contaminantStructure[cIndex]->contaminant_name);
                sprintf(longStr, "Reprod effects model slope (A) value for group %s of contaminant %s.", FunctGroupArray[sp].groupCode, bm->contaminantStructure[cIndex]->contaminant_name);
                Util_XML_Parse_Create_Node(fp, fileName, groupingNode, varStr, longStr, "", XML_TYPE_FLOAT, "");
                
                sprintf(varStr, "%s_%s_spB_r", FunctGroupArray[sp].groupCode, bm->contaminantStructure[cIndex]->contaminant_name);
                sprintf(longStr, "Reprod effects model midpoint (B) value for group %s of contaminant %s.", FunctGroupArray[sp].groupCode, bm->contaminantStructure[cIndex]->contaminant_name);
                Util_XML_Parse_Create_Node(fp, fileName, groupingNode, varStr, longStr, "", XML_TYPE_FLOAT, "");
                
                sprintf(varStr, "%s_%s_avoid", FunctGroupArray[sp].groupCode, bm->contaminantStructure[cIndex]->contaminant_name);
                sprintf(longStr, "level triggering avoidance behaviour in group %s of contaminant %s", FunctGroupArray[sp].groupCode, bm->contaminantStructure[cIndex]->contaminant_name);
                Util_XML_Parse_Create_Node(fp, fileName, groupingNode, varStr, longStr, "", XML_TYPE_FLOAT, "");

                sprintf(varStr, "%s_%s_K_avoid", FunctGroupArray[sp].groupCode, bm->contaminantStructure[cIndex]->contaminant_name);
                sprintf(longStr, "Constant in avoidance behaviour relationship for group %s of contaminant %s ", FunctGroupArray[sp].groupCode, bm->contaminantStructure[cIndex]->contaminant_name);
                Util_XML_Parse_Create_Node(fp, fileName, groupingNode, varStr, longStr, "", XML_TYPE_FLOAT, "");

                set_keyprm_errfn(warn);
				sprintf(varStr, "%s_%s_Cx", FunctGroupArray[sp].groupCode, bm->contaminantStructure[cIndex]->contaminant_name);
				sprintf(longStr, "Cx value of group %s of contaminant %s.", FunctGroupArray[sp].groupCode, bm->contaminantStructure[cIndex]->contaminant_name);
				Util_XML_Parse_Create_Node(fp, fileName, groupingNode, varStr, longStr, "", XML_TYPE_FLOAT, "");

				sprintf(varStr, "%s_%s_Cy", FunctGroupArray[sp].groupCode, bm->contaminantStructure[cIndex]->contaminant_name);
				sprintf(longStr, "Cy value of group %s of contaminant %s.", FunctGroupArray[sp].groupCode, bm->contaminantStructure[cIndex]->contaminant_name);
				Util_XML_Parse_Create_Node(fp, fileName, groupingNode, varStr, longStr, "", XML_TYPE_FLOAT, "");

				set_keyprm_errfn(quit);
				sprintf(varStr, "%s_%s_InstantDoseMortality", FunctGroupArray[sp].groupCode, bm->contaminantStructure[cIndex]->contaminant_name);
				sprintf(longStr, "Instant Dose Mortality value of group %s of contaminant %s.", FunctGroupArray[sp].groupCode, bm->contaminantStructure[cIndex]->contaminant_name);
				Util_XML_Parse_Create_Node(fp, fileName, groupingNode, varStr, longStr, "", XML_TYPE_INTEGER, "");
                
				set_keyprm_errfn(quit);
				sprintf(varStr, "%s_%s_GrowthThresh", FunctGroupArray[sp].groupCode, bm->contaminantStructure[cIndex]->contaminant_name);
				sprintf(longStr, "Threshold value of contaminant %s when %s sees growth effects ", bm->contaminantStructure[cIndex]->contaminant_name, FunctGroupArray[sp].groupCode);
				Util_XML_Parse_Create_Node(fp, fileName, groupingNode, varStr, longStr, "", XML_TYPE_FLOAT, "");

				set_keyprm_errfn(quit);
				sprintf(varStr, "%s_%s_ReprodThresh", FunctGroupArray[sp].groupCode, bm->contaminantStructure[cIndex]->contaminant_name);
				sprintf(longStr, "Threshold value of contaminant %s when %s sees reprod effects ", bm->contaminantStructure[cIndex]->contaminant_name, FunctGroupArray[sp].groupCode);
				Util_XML_Parse_Create_Node(fp, fileName, groupingNode, varStr, longStr, "", XML_TYPE_FLOAT, "");

                set_keyprm_errfn(quit);
				sprintf(varStr, "%s_%s_GrowthEffect", FunctGroupArray[sp].groupCode, bm->contaminantStructure[cIndex]->contaminant_name);
				sprintf(longStr, "Growth effect for group %s once contaminant %s above the threshold", FunctGroupArray[sp].groupCode, bm->contaminantStructure[cIndex]->contaminant_name);
				Util_XML_Parse_Create_Node(fp, fileName, groupingNode, varStr, longStr, "", XML_TYPE_FLOAT, "");
                
                set_keyprm_errfn(quit);
                sprintf(varStr, "%s_%s_ReprodEffect", FunctGroupArray[sp].groupCode, bm->contaminantStructure[cIndex]->contaminant_name);
                sprintf(longStr, "Reproduction effect for group %s once contaminant %s above the threshold", FunctGroupArray[sp].groupCode, bm->contaminantStructure[cIndex]->contaminant_name);
                Util_XML_Parse_Create_Node(fp, fileName, groupingNode, varStr, longStr, "", XML_TYPE_FLOAT, "");
                
                set_keyprm_errfn(quit);
                sprintf(varStr, "%s_%s_MoveEffect", FunctGroupArray[sp].groupCode, bm->contaminantStructure[cIndex]->contaminant_name);
                sprintf(longStr, "Movement effect for group %s once contaminant %s above the threshold", FunctGroupArray[sp].groupCode, bm->contaminantStructure[cIndex]->contaminant_name);
                Util_XML_Parse_Create_Node(fp, fileName, groupingNode, varStr, longStr, "", XML_TYPE_FLOAT, "");
                
                set_keyprm_errfn(quit);
                sprintf(varStr, "%s_%s_ContamScalar", FunctGroupArray[sp].groupCode, bm->contaminantStructure[cIndex]->contaminant_name);
                sprintf(longStr, "Generic scalar for group %s once contaminant %s above the threshold", FunctGroupArray[sp].groupCode, bm->contaminantStructure[cIndex]->contaminant_name);
                Util_XML_Parse_Create_Node(fp, fileName, groupingNode, varStr, longStr, "", XML_TYPE_FLOAT, "");
                
                set_keyprm_errfn(quit);
                sprintf(varStr, "%s_%s_maternal_transfer", FunctGroupArray[sp].groupCode, bm->contaminantStructure[cIndex]->contaminant_name);
                sprintf(longStr, "Maternal transfer for group %s and contaminant %s during recruitment and breeding", FunctGroupArray[sp].groupCode, bm->contaminantStructure[cIndex]->contaminant_name);
                Util_XML_Parse_Create_Node(fp, fileName, groupingNode, varStr, longStr, "", XML_TYPE_FLOAT, "");

                set_keyprm_errfn(quit);
                sprintf(varStr, "%s_%s_suckling_transfer", FunctGroupArray[sp].groupCode, bm->contaminantStructure[cIndex]->contaminant_name);
                sprintf(longStr, "Maternal transfer for group %s and contaminant %s during sucking", FunctGroupArray[sp].groupCode, bm->contaminantStructure[cIndex]->contaminant_name);
                Util_XML_Parse_Create_Node(fp, fileName, groupingNode, varStr, longStr, "", XML_TYPE_FLOAT, "");
			}
		}

		sprintf(varStr, "%s_half_life", bm->contaminantStructure[cIndex]->contaminant_name);
		sprintf(longStr, "Half life of contaminant %s.", bm->contaminantStructure[cIndex]->contaminant_name);
		Util_XML_Parse_Create_Node(fp, fileName, groupingNode, varStr, longStr, "", XML_TYPE_FLOAT, "");
        
        sprintf(varStr, "%s_temp_depend", bm->contaminantStructure[cIndex]->contaminant_name);
        sprintf(longStr, "Flag indicating using temperature dependent LD50 for contaminant %s (1) or not (0)", bm->contaminantStructure[cIndex]->contaminant_name);
        Util_XML_Parse_Create_Node(fp, fileName, groupingNode, varStr, longStr, "", XML_TYPE_FLOAT, "");
        
        sprintf(varStr, "%s_dissolv_coefft", bm->contaminantStructure[cIndex]->contaminant_name);
        sprintf(longStr, "coefficient showing capacity to be dissolved and moved in excretion %s.", bm->contaminantStructure[cIndex]->contaminant_name);
        Util_XML_Parse_Create_Node(fp, fileName, groupingNode, varStr, longStr, "", XML_TYPE_FLOAT, "");

        if (bm->num_contaminants > 1) {
            sprintf(varStr, "%s_contam_interaction", bm->contaminantStructure[cIndex]->contaminant_name);
            sprintf(longStr, "Interaction of %s with other contaminants", bm->contaminantStructure[cIndex]->contaminant_name);
            Util_XML_Create_Node_Next_Line(fp, fileName, groupingNode, varStr, longStr, "", XML_TYPE_FLOATARRAY);
        }

	}
}

/**
 * Create the invading species XML.
 *
 */
void createInvaderXML(MSEBoxModel *bm, FILE *fp, char *fileName, xmlDocPtr doc, xmlNodePtr rootnode) {
    xmlNodePtr groupingNode;
    
    groupingNode = Util_XML_Create_Node(ATLANTIS_ATTRIBUTE_SUB_GROUP, rootnode, "Invading_Species", "Parameters associated with invading species", "", "");
    Util_XML_Parse_Create_Node(fp, fileName, groupingNode, "invading_sp_model", "", "", XML_TYPE_INTEGER, "");
    Util_XML_Parse_Create_Node(fp, fileName, groupingNode, "InvaderIndex", "", "", XML_TYPE_INTEGER, "");
    Util_XML_Parse_Create_Node(fp, fileName, groupingNode, "minInvaderAge", "", "", XML_TYPE_INTEGER, "");
    Util_XML_Parse_Create_Node(fp, fileName, groupingNode, "maxInvaderAge", "", "", XML_TYPE_INTEGER, "");
    Util_XML_Parse_Create_Node(fp, fileName, groupingNode, "InvaderEntryBox", "", "", XML_TYPE_INTEGER, "");
    Util_XML_Parse_Create_Node(fp, fileName, groupingNode, "InvaderMinDepth", "", "", XML_TYPE_FLOAT, "");
    Util_XML_Parse_Create_Node(fp, fileName, groupingNode, "InvaderMaxDepth", "", "", XML_TYPE_FLOAT, "");
    Util_XML_Parse_Create_Node(fp, fileName, groupingNode, "InvaderStartDay", "", "", XML_TYPE_FLOAT, "");
    Util_XML_Parse_Create_Node(fp, fileName, groupingNode, "InvaderEndDay", "", "", XML_TYPE_FLOAT, "");
    Util_XML_Parse_Create_Node(fp, fileName, groupingNode, "InvadersEntering", "", "", XML_TYPE_INTEGER, "");
    Util_XML_Parse_Create_Node(fp, fileName, groupingNode, "InvaderScalar", "", "", XML_TYPE_FLOAT, "");
    Util_XML_Parse_Create_Node(fp, fileName, groupingNode, "InvaderSpeed", "", "", XML_TYPE_FLOAT, "");

    Util_XML_Parse_Create_Node(fp, fileName, groupingNode, "InvadersEntering", "", "", XML_TYPE_INTEGER, "");
    Util_XML_Get_Value_Integer(fileName, ATLANTIS_ATTRIBUTE, bm->ecotest, 1, groupingNode, no_checking, "InvadersEntering", &bm->InvadersEntering);
    
    if ( bm->InvadersEntering == TRUE ) {
        Util_XML_Parse_Create_Node(fp, fileName, groupingNode, "InvaderEntryLayer", "", "", XML_TYPE_INTEGER, "");
        Util_XML_Create_Node_Next_Line(fp,
                                       fileName,
                                       groupingNode,
                                       "InvaderStartNums",
                                       "Number of invaders entering (per invading age class) on first day of the invasion",
                                       "", XML_TYPE_INTEGERARRAY);
    }

}


void Convert_Biol_To_XML(MSEBoxModel *bm, char *fileName, char *outputFileName) {
	xmlDocPtr doc;
	xmlNodePtr rootnode;
    FILE *fp;

	/* Init libxml */
	xmlInitParser();
	LIBXML_TEST_VERSION

	if ((fp = Open_Input_File(bm->inputFolder,fileName, "rt")) == NULL) {
		quit("Cannot open biological input file %s%s\n", bm->inputFolder, fileName);
	}

	doc = xmlNewDoc((unsigned char *) "1.0");

	if (doc == NULL) {
		fprintf(stderr, "Document not created successfully. \n");
		return;
	}

	set_keyprm_errfn(quit);

	rootnode = xmlNewDocNode(doc, NULL, (const xmlChar*) AtlantisXMLObjectNAMES[ATLANTIS_ATTRIBUTE_GROUP], NULL);
	Util_XML_Add_Node_Property(rootnode, AtlantisXMLObjectNAMES[ATLANTIS_ATTRIBUTE_GROUP][ATTRIBUTE_NAME], "AtlantisBiology");

	//rootnode = Util_XML_Create_Node(ATLANTIS_ATTRIBUTE_SUB_GROUP, rootnode, "Setup", "Those determining set-up and recording", "", "");

	xmlDocSetRootElement(doc, rootnode);

	createSetupXML(bm, fp, fileName, doc, rootnode);
	speciesXML(bm, fp, fileName, doc, rootnode);

	createPhysicalLim_n_FLagsXML(bm, fp, fileName, doc, rootnode);

	if(bm->track_atomic_ratio == TRUE){
		createAdditionalTracersXML(bm, fp, fileName, doc, rootnode);
	}
    
	createPhysicalChemicalXML(bm, fp, fileName, doc, rootnode);
	createToleranceXML(bm, fp, fileName, doc, rootnode);
	createEvolutionXML(bm, fp, fileName, doc, rootnode);

	createPrimaryProduceXML(bm, fp, fileName, doc, rootnode);
	createBacteriaXML(bm, fp, fileName, doc, rootnode);

	createHabitatXML(bm, fp, fileName, doc, rootnode);
	createSpatialThresholdXML(bm, fp, fileName, doc, rootnode);
	createMovementXML(bm, fp, fileName, doc, rootnode);

	/* migration information */
    if(bm->flag_migration_on == TRUE){
        // prm file parameters
        createMigrationXML(bm, fp, fileName, doc, rootnode);
    }
	createVertBodyXML(bm, fp, fileName, doc, rootnode);

	/* Diet and metablic information */
	createDietXML(bm, fp, fileName, doc, rootnode);
	createQ10XML(bm, fp, fileName, doc, rootnode);

	/* Fish Respiration */
	createFishRespirationXML(bm, fp, fileName, doc, rootnode);

	/* Overwintering and mortality */
	createOverwinteringXML(bm, fp, fileName, doc, rootnode);
	createMortalityXML(bm, fp, fileName, doc, rootnode);

	/* Detritus related */
	createDetritusConversionXML(bm, fp, fileName, doc, rootnode);

	/* Reproduction related */
	createReproductionXML(bm, fp, fileName, doc, rootnode);
    
	//createRecruitmentXML(bm, fp, fileName, doc, rootnode);
	createStockStructureXML(bm, fp, fileName, doc, rootnode);
    
	if(bm->ice_on == TRUE){
		createIceXML(bm, fp, fileName, doc, rootnode);
	}

	/* Coral Related */
	if(bm->containsCoral)
		createCoralXML(bm, fp, fileName, doc, rootnode);

	/* Contaminants values */
	if(bm->track_contaminants)
		createContaminantsXML(bm, fp, fileName, doc, rootnode);

    /* Invader values */
    createInvaderXML(bm, fp, fileName, doc, rootnode);
    
    printf("Done invader and moving on\n");
    
	/* Save to the output file */
	xmlSaveFormatFileDestFolder(bm->destFolder, outputFileName, doc, 1);
	xmlFreeDoc(doc);
	fclose(fp);

	/* Shutdown libxml */
	xmlCleanupParser();

	//set_keyprm_errfn(fn);

	return;
}


void Convert_Migration_To_XML(MSEBoxModel *bm, char *fileName, char *outputFileName) {
    xmlDocPtr doc;
    xmlNodePtr rootnode;
    xmlNodePtr groupNode;
    FILE *inputFP;
    //xmlNodePtr node;
    //xmlNodePtr lookupNode;
    int numExpectedTokens = 17;
    int buflen = 2000;
    char ch, buf[2000], seps[] = ",";
    char *varStr;
    int count = 0;
    
    /* Init libxml */
    xmlInitParser();
    LIBXML_TEST_VERSION
    
    if ((inputFP = Open_Input_File(bm->inputFolder,fileName, "rt")) == NULL) {
        quit("Cannot open atlantis groups input file %s%s\n", bm->inputFolder, fileName);
    }
    
    doc = xmlNewDoc((unsigned char *) "1.0");
    
    if (doc == NULL) {
        fprintf(stderr, "Document not created successfully. \n");
        return;
    }
    
    rootnode = xmlNewDocNode(doc, NULL, (const xmlChar*) AtlantisXMLObjectNAMES[ATLANTIS_ATTRIBUTE_GROUP], NULL);
    Util_XML_Add_Node_Property(rootnode, AtlantisXMLObjectNAMES[ATLANTIS_ATTRIBUTE_GROUP][ATTRIBUTE_NAME], "AtlantisMigrations");
    
    xmlDocSetRootElement(doc, rootnode);
    
    /* Now parse each group and create the XML */
    
    fseek(inputFP, 0L, 0);
    
    /* Get the first line */
    fgets(buf, buflen, inputFP);
    
    /* First we need to check that the number of tokens is correct */
    varStr = strtok(buf, seps);
    while (varStr != NULL) {
        varStr = strtok(NULL, seps);
        count++;
    }
    
    if (count != numExpectedTokens) {
        quit("\n\nERROR: The migration parameter csv file %s does not have the correct number of columns %d it has %d columns\n\n", fileName, numExpectedTokens, count);
        
    }
    
    while (fgets(buf, buflen, inputFP) != NULL) {
        
        /* Ignore the first line */
        if (strstr(buf, "Code") != NULL)
            continue;
        
        ch = buf[0];
        
        if (ch != '\t' && ch != '\n' && ch != '#' && ch != ',') {
            
            /* Create the node for the group */
            groupNode = Util_XML_Create_Node(ATLANTIS_ATTRIBUTE_SUB_GROUP, rootnode, "MigrationGroup", "The defintion of a migration", "", "");
            
            /* Group Code value. */
            varStr = strtok(buf, seps);
            Util_XML_Create_Node(ATLANTIS_ATTRIBUTE, groupNode, "GroupCode", "", "", varStr);
            
            /* Starting stage */
            varStr = strtok(NULL, seps);
            Util_XML_Create_Node(ATLANTIS_ATTRIBUTE, groupNode, "StartStage", "", "", varStr);
            
            /* MigID - in case multiple migrations per calendar year */
            varStr = strtok(NULL, seps);
            Util_XML_Create_Node(ATLANTIS_ATTRIBUTE, groupNode, "MigID", "", "", varStr);
            
            /* Start TofY */
            varStr = strtok(NULL, seps);
            Util_XML_Create_Node(ATLANTIS_ATTRIBUTE, groupNode, "StartTofY", "", "", varStr);
            
            /* End TofY */
            varStr = strtok(NULL, seps);
            Util_XML_Create_Node(ATLANTIS_ATTRIBUTE, groupNode, "EndTofY", "", "", varStr);
            
            /* Leave_Period */
            varStr = strtok(NULL, seps);
            Util_XML_Create_Node(ATLANTIS_ATTRIBUTE, groupNode, "Leave_Period", "", "", varStr);
            
            /* Return_Period */
            varStr = strtok(NULL, seps);
            Util_XML_Create_Node(ATLANTIS_ATTRIBUTE, groupNode, "Return_Period", "", "", varStr);
            
            /* IsAnnualMigration */
            varStr = strtok(NULL, seps);
            Util_XML_Create_Node(ATLANTIS_ATTRIBUTE, groupNode, "IsAnnualMigration", "", "", varStr);
            
            /* Min time away */
            varStr = strtok(NULL, seps);
            Util_XML_Create_Node(ATLANTIS_ATTRIBUTE, groupNode, "MinTimeAway", "", "", varStr);
            
            /* Max time away */
            varStr = strtok(NULL, seps);
            Util_XML_Create_Node(ATLANTIS_ATTRIBUTE, groupNode, "MaxTimeAway", "", "", varStr);
            
            /* Stock returns to */
            varStr = strtok(NULL, seps);
            Util_XML_Create_Node(ATLANTIS_ATTRIBUTE, groupNode, "MigrateStockReturn", "", "", varStr);
            
            /* Size increase */
            varStr = strtok(NULL, seps);
            Util_XML_Create_Node(ATLANTIS_ATTRIBUTE, groupNode, "MigPropSizeInc", "", "", varStr);

            /* Survivorship */
            varStr = strtok(NULL, seps);
            Util_XML_Create_Node(ATLANTIS_ATTRIBUTE, groupNode, "MigPropSurvive", "", "", varStr);
            
            /* Whether partial migration (some left behind or not) */
            varStr = strtok(NULL, seps);
            Util_XML_Create_Node(ATLANTIS_ATTRIBUTE, groupNode, "PartialExit", "", "", varStr);
            
            /* Lower End of age when partial migration occurs */
            varStr = strtok(NULL, seps);
            Util_XML_Create_Node(ATLANTIS_ATTRIBUTE, groupNode, "MinAgeLeave", "", "", varStr);

            /* Upper End of age when partial migration occurs */
            varStr = strtok(NULL, seps);
            Util_XML_Create_Node(ATLANTIS_ATTRIBUTE, groupNode, "MaxAgeLeave", "", "", varStr);
            
            /* Whether can reproduce while outside the model */
            varStr = strtok(NULL, seps);
            Util_XML_Create_Node(ATLANTIS_ATTRIBUTE, groupNode, "ReprodAllowed", "", "", varStr);

        }
    }
    /* Save to the output file */
    xmlSaveFormatFileDestFolder(bm->destFolder, outputFileName, doc, 1);
    xmlFreeDoc(doc);
    fclose(inputFP);
    
    /* Shutdown libxml */
    xmlCleanupParser();
    return;
}


int Util_Read_Migration_XML(MSEBoxModel *bm, char *fileName, FILE *llogfp) {
    xmlDocPtr inputDoc;
    xmlXPathObjectPtr groupList;
    int groupIndex, numGroups;
    xmlNodePtr groupNode;
    char groupStr[STRLEN];
    int speciesIndex;
    int start_stage, migrationID, start_tofy, end_tofy, mig_period, mig_period2, minaway, maxaway, ret_stock, isAnnual, flagPartialExit, MinAgePartial, MaxAgePartial, ReprodAllowedPartial;
    double grow_rate, survive_rate;
    char convertedXMLFileName[STRLEN];
    
    if (verbose){
        printf("Doing Util_Read_Migration_XML - read in final migration parameters\n");
    }
        
    
    /* Check to see if we are reading in an XML file or a csv file */
    if (strstr(fileName, ".csv") != NULL) {
        /* Convert the file to XML. */
        /* Build the converted filename */
        sprintf(convertedXMLFileName, "%s", fileName);
        *(strstr(convertedXMLFileName, ".csv")) = '\0';
        strcat(convertedXMLFileName, ".xml");
        
        /* Convert the input file to XML - the XML file will be stored in the destination folder if present.*/
        Convert_Migration_To_XML(bm, fileName, convertedXMLFileName);
        inputDoc = xmlReadFileDestFolder(bm->destFolder, convertedXMLFileName, NULL, 0);
    } else {
        sprintf(convertedXMLFileName, "%s", fileName);
        inputDoc = xmlReadFileDestFolder("", convertedXMLFileName, NULL, 0);
    }
    
    if (!inputDoc) {
        fprintf(stderr, "\n\nERROR. Util_Read_Migration_XML: Failed to open migration input file %s\nThis may be because the file does not exist or it may not be valid XML (which is typically created from a csv file).\n", convertedXMLFileName);
        exit(-1);
        
    }
    
    /* Read in each of the functional groups */
    groupList = Util_XML_Get_Node_List(ATLANTIS_ATTRIBUTE_SUB_GROUP, inputDoc->children, "MigrationGroup");
    
    if (groupList && groupList->nodesetval->nodeNr > 0) {
        
        /* Check the number of groups match the values in the run input file*/
        numGroups = groupList->nodesetval->nodeNr;
        if (numGroups > bm->K_num_tot_sp) {
            quit("Util_Read_Migration_XML: Number of groups specified (%d) in your migration definition file is greater than the number of species - K_num_tot_sp (value of %d) in your run file\n", numGroups, convertedXMLFileName, bm->K_num_tot_sp);
        }
        
        /* Now read in the groups. */
        for (groupIndex = 0; groupIndex < numGroups; groupIndex++) {

            groupNode = groupList->nodesetval->nodeTab[groupIndex];
            Util_XML_Get_Value_String(convertedXMLFileName, ATLANTIS_ATTRIBUTE, TRUE, groupNode, "GroupCode", groupStr);
            speciesIndex = Util_Get_FG_Index_From_Token(bm, groupStr, fileName, groupStr, TRUE);

            if (verbose > 0)
                printf("Reading in migration definition of %s\n", FunctGroupArray[speciesIndex].groupCode);

            Util_XML_Get_Value_Integer(convertedXMLFileName, ATLANTIS_ATTRIBUTE, 0, TRUE, groupNode, integer_check, "StartStage", &start_stage);
            Util_XML_Get_Value_Integer(convertedXMLFileName, ATLANTIS_ATTRIBUTE, 0, TRUE, groupNode, integer_check, "MigID", &migrationID);
            Util_XML_Get_Value_Integer(convertedXMLFileName, ATLANTIS_ATTRIBUTE, 0, TRUE, groupNode, integer_check, "StartTofY", &start_tofy);
            Util_XML_Get_Value_Integer(convertedXMLFileName, ATLANTIS_ATTRIBUTE, 0, TRUE, groupNode, integer_check, "EndTofY", &end_tofy);
            Util_XML_Get_Value_Integer(convertedXMLFileName, ATLANTIS_ATTRIBUTE, 0, TRUE, groupNode, integer_check, "Leave_Period", &mig_period);
            Util_XML_Get_Value_Integer(convertedXMLFileName, ATLANTIS_ATTRIBUTE, 0, TRUE, groupNode, integer_check, "Return_Period", &mig_period2);
            Util_XML_Get_Value_Integer(convertedXMLFileName, ATLANTIS_ATTRIBUTE, 0, TRUE, groupNode, integer_check, "IsAnnualMigration", &isAnnual);
            Util_XML_Get_Value_Integer(convertedXMLFileName, ATLANTIS_ATTRIBUTE, 0, TRUE, groupNode, integer_check, "MinTimeAway", &minaway);
            Util_XML_Get_Value_Integer(convertedXMLFileName, ATLANTIS_ATTRIBUTE, 0, TRUE, groupNode, integer_check, "MaxTimeAway", &maxaway);
            Util_XML_Get_Value_Integer(convertedXMLFileName, ATLANTIS_ATTRIBUTE, 0, TRUE, groupNode, integer_check, "MigrateStockReturn", &ret_stock);
            Util_XML_Get_Value_Double(convertedXMLFileName, ATLANTIS_ATTRIBUTE, 0, TRUE, groupNode, no_checking, "MigPropSizeInc", &grow_rate);
            Util_XML_Get_Value_Double(convertedXMLFileName, ATLANTIS_ATTRIBUTE, 0, TRUE, groupNode, no_checking, "MigPropSurvive", &survive_rate);

            Util_XML_Get_Value_Integer(convertedXMLFileName, ATLANTIS_ATTRIBUTE, 0, TRUE, groupNode, no_checking, "PartialExit", &flagPartialExit);
            Util_XML_Get_Value_Integer(convertedXMLFileName, ATLANTIS_ATTRIBUTE, 0, TRUE, groupNode, no_checking, "MinAgeLeave", &MinAgePartial);
            Util_XML_Get_Value_Integer(convertedXMLFileName, ATLANTIS_ATTRIBUTE, 0, TRUE, groupNode, no_checking, "MaxAgeLeave", &MaxAgePartial);
            Util_XML_Get_Value_Integer(convertedXMLFileName, ATLANTIS_ATTRIBUTE, 0, TRUE, groupNode, no_checking, "ReprodAllowed", &ReprodAllowedPartial);
            
            // Set parameters
            MIGRATION[speciesIndex].StartDay_Prm[start_stage][migrationID] = start_tofy;
            MIGRATION[speciesIndex].EndDay_Prm[start_stage][migrationID] = end_tofy;
            MIGRATION[speciesIndex].Leave_Period_Prm[start_stage][migrationID] = mig_period;
            MIGRATION[speciesIndex].Return_Period_Prm[start_stage][migrationID] = mig_period2;
            MIGRATION[speciesIndex].MinYearsAway_Prm[start_stage][migrationID] = minaway;
            MIGRATION[speciesIndex].MaxYearsAway_Prm[start_stage][migrationID] = maxaway;
            MIGRATION[speciesIndex].IsAnnualMigration_Prm[start_stage][migrationID] = isAnnual;
            MIGRATION[speciesIndex].survival_Prm[start_stage][migrationID] = survive_rate;
            MIGRATION[speciesIndex].growth_Prm[start_stage][migrationID] = grow_rate;
            MIGRATION[speciesIndex].returnstock_Prm[start_stage][migrationID] = ret_stock;
            MIGRATION[speciesIndex].IsPartialMigration_Prm[start_stage][migrationID] = flagPartialExit;
            MIGRATION[speciesIndex].PartialMigration_MinPrm[start_stage][migrationID] = MinAgePartial;
            MIGRATION[speciesIndex].PartialMigration_MaxPrm[start_stage][migrationID] = MaxAgePartial;
            MIGRATION[speciesIndex].ReprodAllowedPrm[start_stage][migrationID] = ReprodAllowedPartial;
            
            MIGRATION[speciesIndex].Stagger_Prm[start_stage][migrationID][stagger_years_id] = MIGRATION[speciesIndex].MaxYearsAway_Prm[start_stage][migrationID] - MIGRATION[speciesIndex].MinYearsAway_Prm[start_stage][migrationID];
            if( MIGRATION[speciesIndex].Stagger_Prm[start_stage][migrationID][stagger_years_id] > 0) {
                MIGRATION[speciesIndex].Stagger_Prm[start_stage][migrationID][return_stagger_id] = 1;
            }
            
            
            /*
            if( speciesIndex == 35 ){
                fprintf(bm->logFile, "%s stage %d mig %d startday %d endday %d leave_period %d return_period %d annual_Mig %d MinTime: %d MaxTime: %d Stock: %d PropSizeInc %e MigPropSurvive: %e stagger_years: %d return_stagger: %d\n", FunctGroupArray[speciesIndex].groupCode, start_stage, migrationID, MIGRATION[speciesIndex].StartDay_Prm[start_stage][migrationID], MIGRATION[speciesIndex].EndDay_Prm[start_stage][migrationID], MIGRATION[speciesIndex].Leave_Period_Prm[start_stage][migrationID], MIGRATION[speciesIndex].Return_Period_Prm[start_stage][migrationID], MIGRATION[speciesIndex].IsAnnualMigration_Prm[start_stage][migrationID], MIGRATION[speciesIndex].MinYearsAway_Prm[start_stage][migrationID], MIGRATION[speciesIndex].MaxYearsAway_Prm[start_stage][migrationID], MIGRATION[speciesIndex].returnstock_Prm[start_stage][migrationID], MIGRATION[speciesIndex].growth_Prm[start_stage][migrationID], MIGRATION[speciesIndex].survival_Prm[start_stage][migrationID], MIGRATION[speciesIndex].Stagger_Prm[start_stage][migrationID][stagger_years_id], MIGRATION[speciesIndex].Stagger_Prm[start_stage][migrationID][return_stagger_id]);
            }
            */
        }
        xmlXPathFreeObject(groupList);
    }
    
    xmlFreeDoc(inputDoc);
    /* Shutdown libxml */
    xmlCleanupParser();

    return TRUE;
}
