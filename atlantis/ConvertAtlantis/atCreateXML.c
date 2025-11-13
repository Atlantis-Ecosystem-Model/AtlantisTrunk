/**

 \file
 \brief This file contains utility functions that are used in the conversion code.




 Changes:

 06-01-2010 Bec Gorton
 Added the homerangerad_id and rangeoverlap_id parameters to the Create_Species_ParamXML function.

 */
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

static regmatch_t pmatch[32];
extern char *cohortStrings[];
static void Print_Fishery_Species_Param_Error(MSEBoxModel *bm, char *fileName, int paramID, int guild);

static void Get_Next_Line(MSEBoxModel *bm, FILE *inputFP, char *buf){
	while(1){
		if(fgets(buf, STRLEN, inputFP) != NULL){
			trim(buf);
			if(buf[0] == '\n')
				continue;
			if(buf[0] == '#')
				continue;
			if(strlen(buf) > 0)
				return;
		}
	}
}


/**
 * Check that the given string only contains numbers.
 * If not return false.
 */
static int CheckIsDigits(char *s)
{
	// Trim spaces and tabs from beginning:
	int j;

	for(j=0;j<(int)strlen(s);j++) {
		if(isspace(s[j]))
			continue;
		if(s[j] == '+')
			continue;
		if(s[j] == '-')
			continue;
		if(s[j] != 'e' && s[j] != 'E' && s[j] != '.' && !isdigit(s[j])){
			return -1;
		}
	}
	return 1;
}

/**
 *
 * 	If the initFP is not null then this function will call the initFP function to create a new attribute node. If the initFP is null
 * 	then it is assumed the parent node for all the children nodes that will be created is the given cur node.
 *
 * 	This function will then read through the input file and search for all parameter lines (not starting with a comment or space) that
 * 	match the given regular expression. The next line is then read in, this is the line that contains the data values, and then the
 * 	newNode, the variable name and the data buffer are passed to the given funtion for processing.
 *
 * 	This is generally used to parse input parameters that look like:
 *
 * 	VERTday_CEP      5
 *	0.3 0.3 0.2 0.1 0.1
 *
 *	VERTnight_CEP    5
 *	0.1 0.1 0.2 0.4 0.2
 *
 *	VERTday_PWN      5
 *	0.5 0.5 0.0 0.0 0.0
 *
 * 	VERTnight_PWN    5
 *	0.5 0.5 0.0 0.0 0.0
 *
 * And turn them into XML that looks like:
 *
 * 	<Attribute AttributeName="VerticalDistribution" AttributeDescription="Vertical Distribution" AttributeType="FloatArray" ArrayLength="5">
 * 		<GroupValue GroupName="CEP">
 *			<TemporalValue AttributeName="day" AttributeValue="0.3 0.3 0.2 0.1 0.1 "/>
 *			<TemporalValue AttributeName="night" AttributeValue="0.1 0.1 0.2 0.4 0.2 "/>
 *		</GroupValue>
 *		<GroupValue GroupName="PWN">
 *			<TemporalValue AttributeName="day" AttributeValue="0.5 0.5 0.0 0.0 0.0"/>
 *			<TemporalValue AttributeName="night" AttributeValue="0.5 0.5 0.0 0.0 0.0"/>
 *		</GroupValue>
 * 	</Attribute>
 *
 * 	The call for this would be :
 * 	Parse_File(bm, fp, fileName, groupingNode, "VerticalDistribution", "^VERTday|^VERTnight", "Vertical Distribution", "", XML_TYPE_FLOATARRAY,
 * 		bm->wcnz, TRUE, InitVertDistributionXML, VERTXMLFunction);
 *
 */
xmlNodePtr Parse_File(MSEBoxModel *bm, FILE *inputFP, char *fileName, xmlNodePtr cur, char *tag, char *regEx, char *comment, char *units, int type, int length,
		int required, void(*initFP)(MSEBoxModel *bm, xmlNodePtr parent), void(*fp)(MSEBoxModel *bm, char *fileName, xmlNodePtr parent, char *str,
				char *valueStr)) {

	char ch, buf[STRLEN], seps[] = " ,\t\n";
	char *varStr;
	char *lengthString;
	char tempBuf[STRLEN];
	int found = FALSE;
	char lengthStr[100];
	int lengthValue;
	long filePosition;

	xmlNodePtr newnode;
	regex_t regBuffer;

	at_compileRegExpression(&regBuffer, regEx);

	/* Set up the length string */
	if (length > 0)
		sprintf(lengthStr, "%d", length);
	else
		strcpy(lengthStr, "");

	/* Set the file back to the start */
	fseek(inputFP, 0L, 0);

	/* if we have an initFP call that to set up the parent XMl node else use the 'cur' one given */
	if (initFP != NULL) {
		newnode = Util_XML_Create_Node(ATLANTIS_ATTRIBUTE, cur, tag, comment, units, "");
		initFP(bm, newnode);
	} else {
		newnode = cur;
	}

	if(type != XML_TYPE_FLOATARRAY && type != XML_TYPE_INTEGERARRAY && type != XML_TYPE_BOOLEANARRAY && type != XML_TYPE_LOOKUPARRAY && type != XML_TYPE_STRINGARRAY){
		quit("Parse_File parameter %s type should be an array type not %d\n", tag, type);
	}

	/* Set some properties of the XML node */
	Util_XML_Set_Node_Property(newnode, "AttributeType", AtlantisXMLAttributeTypeStrings[type]);
	Util_XML_Set_Node_Property(newnode, "ArrayLength", lengthStr);

	/* Now loop through the file and find all matches to the given regular expression */
	while (fgets(buf, STRLEN, inputFP) != NULL) {
		strcpy(tempBuf, buf);
		ch = tempBuf[0];

		if (ch == '#' || !isalpha(ch))
			continue;

		//printf("tempBuf = %s\n", tempBuf);

		if ((ch != '#') && (ch != ' ') && (ch != '\n') && (ch != '\t')) {
			/* it's a parameter line */

			/* Get the first token in the line. */
			varStr = strtok(tempBuf, seps);

			//printf("varStr = %s\n", varStr);

			/* Does it match the regular expression given */
			if (regexec(&(regBuffer), varStr, elementsof ( pmatch ), pmatch, 0) == 0) {

				/* Check the length */
				if (length > 0){
					lengthString = strtok(NULL, seps);
					if(lengthString == NULL){
						quit("Token %s in file %s must also include a length string\n", varStr, fileName);
					}
					lengthValue = atoi(lengthString);
					if(lengthValue != length){
                        warn("Parse_File - Creating XML - Length of attribute %s (%d) in file %s is not the required length %d in file %s\n", varStr, lengthValue, fileName, length, fileName);
					}
				}

				/* Get the next line - this contains the values array */
				//fgets(buf, STRLEN, inputFP);
				//printf("varStr = %s\n",varStr);
				Get_Next_Line(bm, inputFP, buf);
				trim(buf);
				if(strlen(buf) == 0){
					quit("ERROR: No value found in input file %s for parameter %s\n", fileName, tag);
				}
				/* Check the value string is only numbers */
				if(type != XML_TYPE_STRINGARRAY && CheckIsDigits(buf) < 0)
					quit("ERROR: Value for parameter %s (%s) in input file %s is not valid %s\n", tag, varStr, fileName, buf);


				/* Check the length */
				/* Check the length */
				if(fp == NULL){
					Util_XML_Set_Node_Value(ATLANTIS_ATTRIBUTE, newnode, varStr, buf);
				}else{
					fp(bm, fileName, newnode, varStr, buf);
				}
				found = TRUE;

				/* Preprocessor that will check for duplicate values in the input file. Makes loading files lots slower */
				if (bm->check_dups == TRUE){
					/* Store the current file pointer */
					filePosition = ftell(inputFP);
					/* Check we don't have any duplicate entries */
					set_keyprm_errfn(quiet);
					if(skipToKeyEndNoRewind(inputFP,varStr) > 0){
						quit("Parse_File: Duplicate key found %s\n", varStr);
						abort();
					}
					fseek(inputFP, filePosition, SEEK_SET);
				}
			}
		}
	}
	regfree(&(regBuffer));

	/* Check to see if we found what we were looking for, if not and it was required spit the dummy and quit */
	if (!found && required)
		quit("ParseFile %s not found in file %s. Regex = %s\n", tag, fileName, regEx);

	//printf("Got to the return\n");

	return newnode;
}

/**
 * The function is very similar to Parse_Line but it assumes the data values are on the same line as the variable name.
 *
 * Its useful for instances where we are turning prm parameters like the following:
 *
 * 	# Detritus handother deep demersal parameters
 *	k_trans_BFS 0.001
 * 	k_trans_BFF 1.0
 *	k_trans_BFD 1.0
 *
 *
 * 	Into XML like the following:
 *
 * 	<Attribute AttributeName="K_TRANS" AttributeDescription="Damping coefficient on rate of detritus transferral by benthic filter feeders" AttributeType="Float" AttributeRecommendedValue="">
 *		<GroupValue GroupName="BFS" AttributeValue="0.001"/>
 *		<GroupValue GroupName="BFF" AttributeValue="1.0"/>
 *		<GroupValue GroupName="BFD" AttributeValue="1.0"/>
 *	</Attribute>
 *
 *	This data can then be read in as a species param using Util_XML_Read_Species_Param().
 *
 */
xmlNodePtr Parse_File_Single_Line(MSEBoxModel *bm, FILE *inputFP, char *fileName, xmlNodePtr parent, char *tag, char *regEx, char *comment, char *units, int type,
		char *recommendedValue, int required, void(*initFP)(MSEBoxModel *bm, xmlNodePtr parent), void(*fp)(MSEBoxModel *bm, char *fileName, xmlNodePtr parent,
				char *str, char *valueStr)) {

	int buflen = STRLEN;
	char ch, buf[STRLEN], seps[] = " ,\t\n";
	char *varStr, *valueStr;
	char tempBuf[STRLEN];
	int found = FALSE;

	xmlNodePtr newnode;
	regex_t regBuffer;

	at_compileRegExpression(&regBuffer, regEx);

	if (initFP != NULL) {
		/* Create the new node */
		newnode = Util_XML_Create_Node(ATLANTIS_ATTRIBUTE, parent, tag, comment, units, "");
		initFP(bm, newnode);
	} else {
		newnode = parent;
	}

	/* Reset the file pointer back to 0 */
	fseek(inputFP, 0L, 0);

	while (fgets(buf, buflen, inputFP) != NULL) {
		strcpy(tempBuf, buf);
		ch = tempBuf[0];

		if (ch == '#' || !isalpha(ch))
			continue;

		if ((ch != '#') && (ch != ' ') && (ch != '\n') && (ch != '\t')) {
			/* it's a parameter line */

			/* Get the first token in the line - this is the parameter name */
			varStr = strtok(tempBuf, seps);
			/* Get the value token - must get this before we pass to the regular expression as it stuffs up the bufder */
			valueStr = strtok(NULL, seps);

			/* Check to see if our varStr matches our expression */
			if (regexec(&(regBuffer), varStr, elementsof ( pmatch ), pmatch, 0) == 0) {

				/* Call the given function pointer to parse the data and add it to the XML DOM */
				fp(bm, fileName, newnode, varStr, valueStr);
				found = TRUE;
			}
		}
	}

	/* Quit if we needed to find a match and we didn't */
	if (found == FALSE && required == TRUE)
		quit("Could not find any lines to match tag %s, regEx = %s in file %s\n", tag, regEx, fileName);

	/* Set the rest of the node properties */
	Util_XML_Set_Node_Property(newnode, "AttributeType", AtlantisXMLAttributeTypeStrings[type]);
	Util_XML_Set_Node_Property(newnode, "AttributeRecommendedValue", recommendedValue);

	/* Free up the buffer and return the parent node */
	regfree(&(regBuffer));
	return newnode;
}

/**
 *	\brief print out an error message about a missing species parameter and quit.
 */
static void Print_Species_Param_Error(MSEBoxModel *bm, char *fileName, int paramIndex, int guild) {
	fprintf(stderr, "\n\nERROR: Failed to find species parameter %s in file %s for group %s\n\n", paramStrings[paramIndex], fileName,
			FunctGroupArray[guild].groupCode);
	fflush(stderr);
	fflush(stdout);

	quit("ERROR\n");
}

/**
 * \brief Check the species param has been read in for all groups that it should be. If not Atlantis will
 * generate an error message and quit.
 *
 *
 *
 */
int Check_Species_Param(MSEBoxModel *bm, char *fileName, int index, int paramID, int *guildCheck) {
	int guild;

	/* First check that we have a paramStrings value for this paramID */
	if(strlen(paramStrings[paramID]) == 0){
		quit("Check_Species_Param: Please fill in the paramStrings slot for parameter %d:%s (%s).\n",
				paramID, speciesParamStructArray[index].tag, speciesParamStructArray[index].regEx);
	}
	switch (speciesParamStructArray[index].paramType) {
	case SP_TURNED_ON:
		for (guild = 0; guild < bm->K_num_tot_sp; guild++) {
			if (guildCheck[guild] == FALSE)
				Print_Species_Param_Error(bm, fileName, paramID, guild);
		}
		break;
	case SP_ALL:
		for (guild = 0; guild < bm->K_num_tot_sp; guild++) {
			if (FunctGroupArray[guild].speciesParams[flag_id] == TRUE) {
				if (guildCheck[guild] == FALSE)
					Print_Species_Param_Error(bm, fileName, paramID, guild);
			}
		}
		break;
	case SP_VERTS:
		for (guild = 0; guild < bm->K_num_tot_sp; guild++) {
			if (FunctGroupArray[guild].speciesParams[flag_id] == TRUE && FunctGroupArray[guild].isVertebrate == TRUE) {
				if (guildCheck[guild] == FALSE)
					Print_Species_Param_Error(bm, fileName, paramID, guild);
			}
		}
		break;
	case SP_REMIN_CONTRIB:
			for (guild = 0; guild < bm->K_num_tot_sp; guild++) {
				if (FunctGroupArray[guild].speciesParams[flag_id] == TRUE){
					if (FunctGroupArray[guild].groupType == SM_INF || FunctGroupArray[guild].groupType == LG_INF || FunctGroupArray[guild].groupType == SED_BACT){
						if (guildCheck[guild] == FALSE)
							Print_Species_Param_Error(bm, fileName, paramID, guild);
					}
				}
			}
			break;
	case SP_VERTS_HOME_RANGE:
		for (guild = 0; guild < bm->K_num_tot_sp; guild++) {
			if (FunctGroupArray[guild].speciesParams[flag_id] == TRUE && FunctGroupArray[guild].isVertebrate == TRUE
					&& (int) (FunctGroupArray[guild].speciesParams[ddepend_move_id]) == homerange_move) {
				if (guildCheck[guild] == FALSE)
					Print_Species_Param_Error(bm, fileName, paramID, guild);
			}
		}
		break;
	case SP_NOT_DET:
		for (guild = 0; guild < bm->K_num_tot_sp; guild++) {
			if (FunctGroupArray[guild].speciesParams[flag_id] == TRUE && FunctGroupArray[guild].groupType != REF_DET && FunctGroupArray[guild].groupType
					!= LAB_DET && FunctGroupArray[guild].groupType != CARRION) {
				if (guildCheck[guild] == FALSE)
					Print_Species_Param_Error(bm, fileName, paramID, guild);
			}
		}
		break;
	case SP_SED_BACT:
		for (guild = 0; guild < bm->K_num_tot_sp; guild++) {
			if (FunctGroupArray[guild].speciesParams[flag_id] == TRUE && FunctGroupArray[guild].groupType == SED_BACT) {
				if (guildCheck[guild] == FALSE)
					Print_Species_Param_Error(bm, fileName, paramID, guild);
			}
		}
		break;
	case SP_AGE_STRUCTURED:
		for (guild = 0; guild < bm->K_num_tot_sp; guild++) {
			if (FunctGroupArray[guild].speciesParams[flag_id] == TRUE) {
				if (FunctGroupArray[guild].groupAgeType == AGE_STRUCTURED || FunctGroupArray[guild].groupAgeType == AGE_STRUCTURED_BIOMASS)
					if (guildCheck[guild] == FALSE)
						Print_Species_Param_Error(bm, fileName, paramID, guild);
			}
		}

		break;
	case SP_AGE_STRUCTURED_BIOMASS:
		for (guild = 0; guild < bm->K_num_tot_sp; guild++) {
			if (FunctGroupArray[guild].speciesParams[flag_id] == TRUE) {
				if (FunctGroupArray[guild].groupAgeType == AGE_STRUCTURED_BIOMASS)
					if (guildCheck[guild] == FALSE)
						Print_Species_Param_Error(bm, fileName, paramID, guild);
			}
		}

		break;
	case SP_PREDATOR:
		for (guild = 0; guild < bm->K_num_tot_sp; guild++) {
			if (FunctGroupArray[guild].speciesParams[flag_id] == TRUE) {
				if (FunctGroupArray[guild].isPredator == TRUE)
					if (guildCheck[guild] == FALSE)
						Print_Species_Param_Error(bm, fileName, paramID, guild);
			}
		}
		break;
	case SP_INVERT_PREDATORS:
		for (guild = 0; guild < bm->K_num_tot_sp; guild++) {
			if (FunctGroupArray[guild].speciesParams[flag_id] == TRUE) {
				if (FunctGroupArray[guild].isVertebrate == FALSE && FunctGroupArray[guild].isPredator == TRUE)
					if (guildCheck[guild] == FALSE)
						Print_Species_Param_Error(bm, fileName, paramID, guild);
			}
		}
		break;
	case SP_Q10:
		for (guild = 0; guild < bm->K_num_tot_sp; guild++) {
			if (FunctGroupArray[guild].speciesParams[flag_id] == TRUE) {
				if (FunctGroupArray[guild].speciesParams[q10_method_id])
					if (guildCheck[guild] == FALSE)
						Print_Species_Param_Error(bm, fileName, paramID, guild);
			}
		}

		break;
    case SP_POLLUTE_IMPACTED:
        for (guild = 0; guild < bm->K_num_tot_sp; guild++) {
            if (FunctGroupArray[guild].speciesParams[flag_id] == TRUE) {
                if ((FunctGroupArray[guild].isLightEffected) || (FunctGroupArray[guild].isNoiseEffected))
                    if (guildCheck[guild] == FALSE)
                        Print_Species_Param_Error(bm, fileName, paramID, guild);
                }
            }
        break;
    case SP_BIOTURB:
		for (guild = 0; guild < bm->K_num_tot_sp; guild++)
			if (FunctGroupArray[guild].speciesParams[flag_id] == TRUE) {
				if ((FunctGroupArray[guild].groupType == LG_INF || FunctGroupArray[guild].groupType == MOB_EP_OTHER || FunctGroupArray[guild].groupType
                     == SED_EP_OTHER) || (FunctGroupArray[guild].isBioTurbator == TRUE))
					if (guildCheck[guild] == FALSE)
						Print_Species_Param_Error(bm, fileName, paramID, guild);
			}
		break;
	case SP_INF:
		for (guild = 0; guild < bm->K_num_tot_sp; guild++)
			if (FunctGroupArray[guild].speciesParams[flag_id] == TRUE) {
				if (FunctGroupArray[guild].isInfauna == TRUE)
					if (guildCheck[guild] == FALSE)
						Print_Species_Param_Error(bm, fileName, paramID, guild);
			}
		break;
	case SP_KN:
		for (guild = 0; guild < bm->K_num_tot_sp; guild++)
			if (FunctGroupArray[guild].speciesParams[flag_id] == TRUE) {
				if (FunctGroupArray[guild].isVertebrate == FALSE) {
					if (FunctGroupArray[guild].isPrimaryProducer == TRUE || FunctGroupArray[guild].groupType == SED_BACT) {
						if (guildCheck[guild] == FALSE)
							Print_Species_Param_Error(bm, fileName, paramID, guild);
					}
				}
			}
		break;
	case SP_PP:
		for (guild = 0; guild < bm->K_num_tot_sp; guild++)
			if (FunctGroupArray[guild].speciesParams[flag_id] == TRUE) {
				if (FunctGroupArray[guild].isVertebrate == FALSE) {
					if (FunctGroupArray[guild].isPrimaryProducer == TRUE) {
						if (guildCheck[guild] == FALSE)
							Print_Species_Param_Error(bm, fileName, paramID, guild);
					}
				}
			}
		break;

	case SP_SG:
		for (guild = 0; guild < bm->K_num_tot_sp; guild++)
			if (FunctGroupArray[guild].speciesParams[flag_id] == TRUE) {
				if (FunctGroupArray[guild].groupType == SEAGRASS){
					if (guildCheck[guild] == FALSE)
						Print_Species_Param_Error(bm, fileName, paramID, guild);

				}
			}
		break;

	case SP_MICROALGAE:
		for (guild = 0; guild < bm->K_num_tot_sp; guild++)
			if (FunctGroupArray[guild].speciesParams[flag_id] == TRUE) {
				if (FunctGroupArray[guild].isVertebrate == FALSE) {
					if (FunctGroupArray[guild].isPrimaryProducer == TRUE) {
						if (guildCheck[guild] == FALSE)
							Print_Species_Param_Error(bm, fileName, paramID, guild);
					}
				}
			}
		break;
	case SP_MACROPHYTE:
		for (guild = 0; guild < bm->K_num_tot_sp; guild++) {
			if (FunctGroupArray[guild].speciesParams[flag_id] == TRUE) {
				if ((FunctGroupArray[guild].groupType == PHYTOBEN) || (FunctGroupArray[guild].groupType == SEAGRASS)
						|| (FunctGroupArray[guild].groupType == TURF)) {
					if (guildCheck[guild] == FALSE)
						Print_Species_Param_Error(bm, fileName, paramID, guild);
				}
			}
		}
		break;

	case SP_OXYGEN:
		for (guild = 0; guild < bm->K_num_tot_sp; guild++)
			if (FunctGroupArray[guild].speciesParams[flag_id] == TRUE) {
				if (FunctGroupArray[guild].isVertebrate == FALSE) {
					if (FunctGroupArray[guild].isBacteria == TRUE ||
							FunctGroupArray[guild].groupType
							== MOB_EP_OTHER || FunctGroupArray[guild].groupType == LG_INF || FunctGroupArray[guild].groupType == SM_INF
							|| FunctGroupArray[guild].groupType == SED_EP_OTHER || FunctGroupArray[guild].groupType == SED_EP_FF
							|| FunctGroupArray[guild].groupType == ICE_BACT) {
						if (guildCheck[guild] == FALSE)
							Print_Species_Param_Error(bm, fileName, paramID, guild);
					}
				}
			}
		break;
	case SP_SED_EP_FF:
		for (guild = 0; guild < bm->K_num_tot_sp; guild++)
			if (FunctGroupArray[guild].speciesParams[flag_id] == TRUE) {
				if (FunctGroupArray[guild].isVertebrate == FALSE) {
					if (FunctGroupArray[guild].groupType == SED_EP_FF) {
						if (guildCheck[guild] == FALSE)
							Print_Species_Param_Error(bm, fileName, paramID, guild);
					}
				}
			}
		break;
	case SP_COVER_SED_EP_FF:
		for (guild = 0; guild < bm->K_num_tot_sp; guild++)
			if (FunctGroupArray[guild].speciesParams[flag_id] == TRUE) {
				if (FunctGroupArray[guild].isVertebrate == FALSE) {
					if (FunctGroupArray[guild].groupType == SED_EP_FF || (FunctGroupArray[guild].isCover && FunctGroupArray[guild].speciesParams[flag_lim_id] == TRUE)) {
						if (guildCheck[guild] == FALSE)
							Print_Species_Param_Error(bm, fileName, paramID, guild);
					}
				}
			}
		break;
	case SP_BASAL:
		for (guild = 0; guild < bm->K_num_tot_sp; guild++)
			if (FunctGroupArray[guild].speciesParams[flag_id] == TRUE) {
				if (FunctGroupArray[guild].isVertebrate == FALSE) {
					if (FunctGroupArray[guild].isInfauna || FunctGroupArray[guild].isEpiFauna == TRUE) {
						if (guildCheck[guild] == FALSE)
							Print_Species_Param_Error(bm, fileName, paramID, guild);
					}
				}
			}
		break;
	case SP_HORIZONATAL_MOVE:
		for (guild = 0; guild < bm->K_num_tot_sp; guild++)
			if (FunctGroupArray[guild].speciesParams[flag_id] == TRUE) {
				if (FunctGroupArray[guild].sp_geo_move == TRUE)
					if (guildCheck[guild] == FALSE)
						Print_Species_Param_Error(bm, fileName, paramID, guild);
			}
		break;
	case SP_MOVEMENT:
		for (guild = 0; guild < bm->K_num_tot_sp; guild++) {
			if (FunctGroupArray[guild].groupType == SED_EP_FF || FunctGroupArray[guild].groupType == MOB_EP_OTHER || FunctGroupArray[guild].sp_geo_move
					== TRUE) {
				if (guildCheck[guild] == FALSE)
					Print_Species_Param_Error(bm, fileName, paramID, guild);
			}
		}
		break;
	case SP_FISHED:
		for (guild = 0; guild < bm->K_num_tot_sp; guild++) {
			if (FunctGroupArray[guild].speciesParams[flag_id] == TRUE) {
				if (FunctGroupArray[guild].isFished == TRUE) {
					if (guildCheck[guild] == FALSE)
						Print_Species_Param_Error(bm, fileName, paramID, guild);
				}
			}
		}
		break;
	case SP_IMPACTED:
		for (guild = 0; guild < bm->K_num_tot_sp; guild++) {
			if (FunctGroupArray[guild].isImpacted == TRUE) {
				if (guildCheck[guild] == FALSE)
					Print_Species_Param_Error(bm, fileName, paramID, guild);
			}
		}
		break;
	case SP_NOT_PP:
		for (guild = 0; guild < bm->K_num_tot_sp; guild++) {
			if (FunctGroupArray[guild].isPrimaryProducer == FALSE && FunctGroupArray[guild].isDetritus == FALSE) {
				if (guildCheck[guild] == FALSE)
					Print_Species_Param_Error(bm, fileName, paramID, guild);
			}
		}
		break;
	case SP_POP_LIM:
		for (guild = 0; guild < bm->K_num_tot_sp; guild++) {
			if (FunctGroupArray[guild].isInfauna == TRUE || FunctGroupArray[guild].groupType == SED_EP_OTHER || FunctGroupArray[guild].groupType == SED_EP_FF) {
				if (guildCheck[guild] == FALSE)
					Print_Species_Param_Error(bm, fileName, paramID, guild);
			}
		}
		break;
	case SP_AGE_DIET:
		for (guild = 0; guild < bm->K_num_tot_sp; guild++) {
			if (FunctGroupArray[guild].groupType == LG_ZOO  || FunctGroupArray[guild].groupType == MED_ZOO || FunctGroupArray[guild].groupType == SM_ZOO || FunctGroupArray[guild].groupType == LG_INF
					|| FunctGroupArray[guild].groupType == SED_EP_FF || FunctGroupArray[guild].groupType == SED_EP_OTHER
					|| FunctGroupArray[guild].groupType == MOB_EP_OTHER)
				if (guildCheck[guild] == FALSE)
					Print_Species_Param_Error(bm, fileName, paramID, guild);
		}
		break;
	case SP_MORT:
        for (guild = 0; guild < bm->K_num_tot_sp; guild++) {
            if (FunctGroupArray[guild].speciesParams[flag_id] == TRUE && FunctGroupArray[guild].isDetritus == FALSE) {
                if (guildCheck[guild] == FALSE)
                    Print_Species_Param_Error(bm, fileName, paramID, guild);
            }
        }
        break;
	case SP_MORT_NOT_PP:
        for (guild = 0; guild < bm->K_num_tot_sp; guild++) {
            if (FunctGroupArray[guild].speciesParams[flag_id] == TRUE && FunctGroupArray[guild].isDetritus == FALSE && FunctGroupArray[guild].isPrimaryProducer == FALSE && FunctGroupArray[guild].isBacteria == FALSE) {
                if (guildCheck[guild] == FALSE)
                    Print_Species_Param_Error(bm, fileName, paramID, guild);
            }
        }
        break;
	case SP_TIME:
	case SP_TIME_AGE:
        break;
	case SP_OVERWINTER:
			for (guild = 0; guild < bm->K_num_tot_sp; guild++) {
				if (FunctGroupArray[guild].isOverWinter == TRUE)
					if (guildCheck[guild] == FALSE)
						Print_Species_Param_Error(bm, fileName, paramID, guild);
			}
			break;
	case SP_PREDATOR_OR_BACT:
			for (guild = 0; guild < bm->K_num_tot_sp; guild++) {
				if (FunctGroupArray[guild].speciesParams[flag_id] == TRUE) {
					if (FunctGroupArray[guild].isPredator == TRUE || FunctGroupArray[guild].isBacteria == TRUE )
						if (guildCheck[guild] == FALSE)
							Print_Species_Param_Error(bm, fileName, paramID, guild);
				}
			}
			break;
	case SP_CORAL:
		for (guild = 0; guild < bm->K_num_tot_sp; guild++) {
			if ((FunctGroupArray[guild].groupType == CORAL) || (FunctGroupArray[guild].groupType == SPONGE))
				if (guildCheck[guild] == FALSE)
					Print_Species_Param_Error(bm, fileName, paramID, guild);
		}
		break;
    case SP_SPONGE:
        for (guild = 0; guild < bm->K_num_tot_sp; guild++) {
            if (FunctGroupArray[guild].groupType == SPONGE)
                if (guildCheck[guild] == FALSE)
                    Print_Species_Param_Error(bm, fileName, paramID, guild);
        }
        break;
	case SP_CULTURED:
		for (guild = 0; guild < bm->K_num_tot_sp; guild++) {
			if (FunctGroupArray[guild].isCultured == TRUE)
				if (guildCheck[guild] == FALSE)
					Print_Species_Param_Error(bm, fileName, paramID, guild);
		}
		break;
    case SP_FED:
        for (guild = 0; guild < bm->K_num_tot_sp; guild++) {
            if ((FunctGroupArray[guild].isCultured == TRUE) || (FunctGroupArray[guild].isSupplemented == TRUE))
                if (guildCheck[guild] == FALSE)
                    Print_Species_Param_Error(bm, fileName, paramID, guild);
        }
        break;
    case SP_RUGOSITY:
        for (guild = 0; guild < bm->K_num_tot_sp; guild++) {
            if ((FunctGroupArray[guild].groupType == CORAL)
                || (FunctGroupArray[guild].groupType == SPONGE)
                || (FunctGroupArray[guild].groupType == SED_EP_FF)
                || (FunctGroupArray[guild].groupType == PHYTOBEN)
                || (FunctGroupArray[guild].groupType == SEAGRASS)
                || (FunctGroupArray[guild].groupType == TURF))
                    if (guildCheck[guild] == FALSE)
                        Print_Species_Param_Error(bm, fileName, paramID, guild);
            }
        break;
    case NOT_USED:	/* Intentional follow through */
	default:
		quit("Check_Species_Param. Does not handle the speciesParamStructArray[index].paramType value %d\n", speciesParamStructArray[index].paramType);
		break;
	}
	return TRUE;
}

void Print_Species_Cohort_Param_Error(MSEBoxModel *bm, char *fileName, int paramIndex, int guild, int cohort) {
	fprintf(stderr, "\n\nERROR: Failed to find species parameter %s in file %s for group %s, cohort %s", cohortParamStrings[paramIndex], fileName,
			FunctGroupArray[guild].groupCode, cohortStrings[cohort]);
	fflush(stderr);
	fflush(stdout);

	quit("ERROR\n");
}

int Check_Species_Cohort_Param(MSEBoxModel *bm, char *fileName, int index, int paramID, int **guildCheck) {
	int guild, cohort;

	switch (cohortSpeciesParamStructArray[index].paramType) {

	case SP_MORT:
		for (guild = 0; guild < bm->K_num_tot_sp; guild++) {
			if (FunctGroupArray[guild].speciesParams[flag_id] == TRUE && FunctGroupArray[guild].isDetritus == FALSE) {                
                if (FunctGroupArray[guild].groupAgeType == AGE_STRUCTURED || FunctGroupArray[guild].groupAgeType == AGE_STRUCTURED_BIOMASS) {
					for (cohort = 0; cohort < FunctGroupArray[guild].numStages; cohort++) {
						if (guildCheck[guild][cohort] == FALSE)
							Print_Species_Cohort_Param_Error(bm, fileName, paramID, guild, cohort);
					}
				} else {
					cohort = juv_id;
					if (guildCheck[guild][cohort] == FALSE)
						Print_Species_Cohort_Param_Error(bm, fileName, paramID, guild, cohort);

				}
			}
		}
		break;
	case SP_MORT_NOT_PP:
		for (guild = 0; guild < bm->K_num_tot_sp; guild++) {
			if (FunctGroupArray[guild].speciesParams[flag_id] == TRUE && FunctGroupArray[guild].isDetritus == FALSE && FunctGroupArray[guild].isPrimaryProducer
					== FALSE && FunctGroupArray[guild].isBacteria == FALSE) {
				if (FunctGroupArray[guild].groupAgeType == AGE_STRUCTURED || FunctGroupArray[guild].groupAgeType == AGE_STRUCTURED_BIOMASS) {
					for (cohort = 0; cohort < FunctGroupArray[guild].numStages; cohort++) {
						if (guildCheck[guild][cohort] == FALSE)
							Print_Species_Cohort_Param_Error(bm, fileName, paramID, guild, cohort);
					}
				} else {
					cohort = juv_id;
					if (guildCheck[guild][cohort] == FALSE)
						Print_Species_Cohort_Param_Error(bm, fileName, paramID, guild, cohort);

				}
			}
		}
		break;
	case SP_CORAL:
		for (guild = 0; guild < bm->K_num_tot_sp; guild++) {
			if (FunctGroupArray[guild].speciesParams[flag_id] == TRUE && ((FunctGroupArray[guild].groupType == CORAL) || (FunctGroupArray[guild].groupType == SPONGE))) {
				if (FunctGroupArray[guild].groupAgeType == AGE_STRUCTURED || FunctGroupArray[guild].groupAgeType == AGE_STRUCTURED_BIOMASS) {
					for (cohort = 0; cohort < FunctGroupArray[guild].numStages; cohort++) {
						if (guildCheck[guild][cohort] == FALSE)
							Print_Species_Cohort_Param_Error(bm, fileName, paramID, guild, cohort);
					}
				} else {
					cohort = juv_id;
					if (guildCheck[guild][cohort] == FALSE)
						Print_Species_Cohort_Param_Error(bm, fileName, paramID, guild, cohort);

				}
			}
		}
		break;
    case SP_NOT_DET:
            for (guild = 0; guild < bm->K_num_tot_sp; guild++) {
                if (FunctGroupArray[guild].speciesParams[flag_id] == TRUE && FunctGroupArray[guild].groupType != REF_DET && FunctGroupArray[guild].groupType != LAB_DET && FunctGroupArray[guild].groupType != CARRION) {
                    if (FunctGroupArray[guild].groupAgeType == AGE_STRUCTURED || FunctGroupArray[guild].groupAgeType == AGE_STRUCTURED_BIOMASS) {
                        for (cohort = 0; cohort < FunctGroupArray[guild].numStages; cohort++) {
                            if (guildCheck[guild][cohort] == FALSE)
                                Print_Species_Cohort_Param_Error(bm, fileName, paramID, guild, cohort);
                        }
                    } else {
                        cohort = juv_id;
                        if (guildCheck[guild][cohort] == FALSE)
                            Print_Species_Cohort_Param_Error(bm, fileName, paramID, guild, cohort);
                        
                    }
                }
            }
            break;
	default:
		quit("Check_Species_Cohort_Param - paramType %d not found\n", cohortSpeciesParamStructArray[index].paramType);
		/* Do nothing */
		break;
	}

	return TRUE;
}

void Print_Species_Spawn_Param_Error(MSEBoxModel *bm, char *fileName, int paramIndex, int guild, int nspawn) {
	fprintf(stderr, "ERROR: Failed to find species parameter %s in file %s for group %s, entry %d", spawnParamStrings[paramIndex], fileName,
			FunctGroupArray[guild].groupCode, nspawn);
	fflush(stderr);
	fflush(stdout);

	quit("ERROR\n");
}

int Check_Species_Spawn_Param(MSEBoxModel *bm, char *fileName, int index, int paramID, int **guildCheck) {
	int guild, nspawn;

	switch (spawnSpeciesParamStructArray[index].paramType) {

	case SP_TIME:

		for (guild = 0; guild < bm->K_num_tot_sp; guild++) {
			if (FunctGroupArray[guild].speciesParams[flag_id] == TRUE){
				switch(FunctGroupArray[guild].groupAgeType){
					case AGE_STRUCTURED:
					case AGE_STRUCTURED_BIOMASS:
						for (nspawn = 0; nspawn < FunctGroupArray[guild].numSpawns; nspawn++) {
							if (guildCheck[guild][nspawn] == FALSE) {
								Print_Species_Spawn_Param_Error(bm, fileName, paramID, guild, nspawn);
							}
						}
						break;
					case BIOMASS:
						/* Do nothing */
						break;
				}
			}
		}

		break;
	case SP_TIME_AGE:

		for (guild = 0; guild < bm->K_num_tot_sp; guild++) {
			if (FunctGroupArray[guild].speciesParams[flag_id] == TRUE) {
				if (FunctGroupArray[guild].groupAgeType == AGE_STRUCTURED_BIOMASS ){
					for (nspawn = 0; nspawn < FunctGroupArray[guild].numSpawns; nspawn++) {
						if (guildCheck[guild][nspawn] == FALSE)
							Print_Species_Spawn_Param_Error(bm, fileName, paramID, guild, nspawn);
					}
				}
			}
		}

		break;
	default:
		quit("Check_Species_Spawn_Param - paramType %d not found\n", spawnSpeciesParamStructArray[index].paramType);
		/* Do nothing */
		break;
	}

	return TRUE;
}

/**
 *
 * \brief Create a species parameter in the XML document for the given paramID.
 *
 *
 * @param bm The pointer to the BoxModel
 * @param fileName The name of the current prm input file. This is used to create useful error messages.
 * @param inputFP The pointer to the input prm file.
 * @param paramID The species parameter defined in the SPECIES_PARAMS enum
 * @param comment The comment string to be written to the XML file.
 * @param units: The units value written to the XML file.
 * @param type: The type of XML node to create.
 * @recommendedValue: A recommended value if one is suitable.*
 *
 *
 */
xmlNodePtr Create_Species_ParamXML(MSEBoxModel *bm, char *fileName, FILE *inputFP, xmlNodePtr parent,
		int paramID, char *comment, char *units, ATL_XML_ATTRIBUTE_TYPES type,
		char *recommendedValue) {
	char *speciesStr = NULL;
	int buflen = 5000;
	char ch, buf[5000], seps[] = " ,\t\n";
	char *varStr, *valuestr;
	char strPtr[50];
	char tempBuf[5000];
	xmlNodePtr newNode;
	regex_t regBuffer;
	int guild, index;
	int *guildCheck = Util_Alloc_Init_1D_Int(bm->K_num_tot_sp, FALSE);
	long filePosition;

	/* Get the index of this paramID in the speciesParamStructArray structure - done this way so the
	 * values in speciesParamStructArray don't have to be in any particular order
	 */
	index = Util_XML_Get_Param_Index(speciesParamStructArray, tot_prms, paramID);

	at_compileRegExpression(&regBuffer, speciesParamStructArray[index].regEx);
	//if (verbose > 0)
	//printf("Reading in %s values, paramID = %d\n", speciesParamStructArray[index].tag, paramID);

	fseek(inputFP, 0L, 0);

	/* Check to see if the parent node already exists
	 * If not create the new node.
	 * This allows for cases when we want to pre fill have values in the tree.
	 * */

	newNode = Util_XML_Get_Node(ATLANTIS_ATTRIBUTE, parent, speciesParamStructArray[index].tag);
	if (newNode == NULL) {
		newNode = Util_XML_Create_Node(ATLANTIS_ATTRIBUTE, parent, speciesParamStructArray[index].tag, comment, units, "");
		Util_XML_Add_Node_Property(newNode, "AttributeType", AtlantisXMLAttributeTypeStrings[type]);
		Util_XML_Add_Node_Property(newNode, "AttributeRecommendedValue", recommendedValue);
	}

	while (fgets(buf, buflen, inputFP) != NULL) {

		strcpy(tempBuf, buf);
		ch = tempBuf[0];

		if (ch == '#') {
			continue;
		}

		if (!isalpha(ch)) {
			continue;
		}

		if ((ch != '#') && (ch != ' ') && (ch != '\n') && (ch != '\t')) {
			/* it's a parameter line */

			//printf("buf = %s\n", tempBuf);
			// Get the first token in the line.
			varStr = strtok(tempBuf, seps);

			if (bm->check_dups == TRUE){

				/* Store the current file pointer */
				filePosition = ftell(inputFP);
				/* Check we don't have any duplicate entries */
				set_keyprm_errfn(quiet);
				if(skipToKeyEndNoRewind(inputFP,varStr) > 0){
					quit("Create_Species_ParamXML: Duplicate key found %s\n", varStr);
					abort();
				}
				fseek(inputFP, filePosition, SEEK_SET);

			}

			//printf("varStr  = %s\n", varStr);
			if (regexec(&(regBuffer), varStr, elementsof ( pmatch ), pmatch, 0) == 0) {

                //printf("%s paramID %d", varStr, paramID);
                
				/* Get the speciesName. */
				switch (paramID) {
				case flag_id:
				case flagdem_id:
				case flagplankfish_id:
				case flagbearlive_id:
				case feed_while_spawn_id:
				case flagtempsensitive_id:
				case flagmother_id:
				case flagrecruit_id:
                case flagkeep_plusgroup_id:
                case flagstocking_id:
				case flagrecpeak_id:
				//case flaglocalrecruit_id:
                case flagq10eff_id:
                case flagq10receff_id:
                //case flag_multiyr_type_id:
				case q10_id:
				case q10_method_id:
				case q10_optimal_temp_id:
				case q10_correction_id:
				case predcase_id:
				case recover_start_id:
				case Recruit_Period_id:
				case Kcov_juv_id:
				case Bcov_juv_id:
				case Acov_juv_id:
				case Kcov_ad_id:
				case Bcov_ad_id:
				case Acov_ad_id:
				case KL_id:
				case KU_id:
				case KUP_id:
				case Kmax_coefft_id:
				case KLP_id:
				case KDEP_id:
				case ICE_KDEP_id:
				case KWSR_id:
				case KWRR_id:
				case recover_mult_id:
				case BHbeta_id:
				case BHalpha_id:
				case Rbeta_id:
				case Ralpha_id:
				case jack_a_id:
				case jack_b_id:
                case prod_alpha_id:
                case den_depend_beta1_id:
                case den_depend_beta2_id:
                case temp_coefft_id:
                case rate_coefft_id:
                case wind_coefft_id:
                case recruit_var_id:
				case PP_id:
				case hta_id:
				case htb_id:
				case pR_id:
				case prop_spawn_lost_id:
                case intersp_depend_recruit_id:
                case intersp_depend_sp_id:
                case intersp_depend_scale_id:
                case turbid_refuge_id:
				case li_a_id:
				case li_b_id:
                case linf_id:
                case Kbert_id:
                case tzero_id:
                case min_li_mat_id:
				case KA_id:
				case KB_id:
                case RSmax_id:
                case RSmid_id:
                case RSslope_id:
                case RSprop_id:
                case SNcost_id:
                case RNcost_id:
                case RSstarve_id:
				case KSPA_id:
				case FSP_id:
				case vlb_id:
				case E1_id:
				case E2_id:
				case E3_id:
				case E4_id:
				case FDMort_id:
				case FDG_id:
				case FDGDL_id:
				case FDGDR_id:
				case KTUR_id:
				case KIRR_id:
				case vl_id:
				case ht_id:
				case KN_id:
				case KS_id:
				case KF_id:
				case KLYS_id:
				case mD_id:
				case mO_id:
				case mS_id:
				case KO2_id:
				case KO2LIM_id:
				case FSBDR_id:
				case age_structured_prey_id:
				case flagfish_id:
				case access_thru_wc_id:
                case cpue_cdf_poor_r_id:
                case cpue_cdf_poor_p_id:
                case cpue_cdf_top_r_id:
                case cpue_cdf_top_p_id:
				case tier_id:
                case regionalSP_id:
				case basketSP_id:
				case basket_size_id:
				case coType_id:
                case max_co_sp_id:
				case p_split_id:
	            case mStarve_id:
				case mT_id:
	            case tac_resetperiod_id:
	            case Kext_id:
	            case Ksub_id:
	            case KNepi_id:
	            case KsubEpi_id:
	            case Ktrans_id:
	           	case temp_coefftA_id:
	            case salt_correction_id:
	            case flagfecundsensitive_id:
	            case flagpHsensitive_id:
	            case flagSaltSensitive_id:
	            case flagnutvaleffect_id:
	            case flagpredavaileffect_id:
	            case pHsensitive_model_id:
	            case pH_constA_id:
	            case pH_constB_id:
                case pH_constC_id:
                case min_pH_id:
                case max_pH_id:
	            case KN_pH_id:
	            case optimal_pH_id:
                case pHmortstart_id:
                case pHmortA_id:
                case pHmortB_id:
                case pHmortmid_id:
	            case pH_correction_id:
	            case flagcontract_tol_id:
                case light_coefft_id:
                case noise_coefft_id:
	            case contract_tol_id:
	            case P_max_uptake_id:
	            case P_uptake_scale_id:
	            case P_concp_id:
	            case P_min_internal_id:
	            case P_max_internal_id:
	            case C_max_uptake_id:
				case C_uptake_scale_id:
				case C_concp_id:
	            case Psa_min_id:
	            case C_min_id:
	            case phyto_resp_rate_id:
	            case KP_id:
	            case overwinterStartTofY_id:
	            case overwinterEndTofY_id:
	            case overwinterStartTemp_id:
	            case overwinterEndTemp_id:
	            case crit_mum_id:
	            case crit_nut_id:
	            case crit_temp_id:
	            case encyst_rate_id:
	            case hatch_rate_id:
	            case encyst_period_id:
	            case flagencyst_id:
	            case max_prop_shift_id:
	            case inheritance_id:
	            case trait_variance_id:
	            case min_trait_variance_id:
	            case Beta_D_id:
	            case PBmax_D_id:
                case whichRAssess_id:
					speciesStr = varStr + strlen(paramStrings[paramID]);
					break;

	            case thresh_id:
				case low_id:
				case max_id:
				case sat_id:
				case flux_thresh_id:
				case flux_damp_id:
					strcpy(strPtr, varStr);
					(*strstr(strPtr, paramStrings[paramID])) = '\0';
					speciesStr = strPtr;
					break;

				case KI_T15_id:
					strcpy(strPtr, varStr);
					speciesStr = strPtr + strlen("KI_");
					(*strstr(speciesStr, "_")) = '\0';
					break;

				case KI_L_T15_id:
					strcpy(strPtr, varStr);
					speciesStr = strPtr + strlen("L_KI_");
					(*strstr(speciesStr, "_")) = '\0';
					break;

					/* Note the different order of the sprintf - this is intentional. These
					 * parameters have the format "FPS_habdepend"
					 */
				case ddepend_move_id:
				case Recruit_Time_id:
                case cohort_recruit_entry_id:
				case spawn_period_id:
				case age_mat_id:
				case catcheater_id:
				case mindepth_id:
				case maxdepth_id:
                case maxtotdepth_id:
				case homerangerad_id:
				case rangeoverlap_id:
                case sp_remin_contrib_id:
				case min_O2_id:
					strcpy(strPtr, varStr);
					(*strchr(strPtr, '_')) = '\0';
					speciesStr = strPtr;
					break;

				case flagactive_id:
					strcpy(strPtr, varStr);
					speciesStr = strPtr + strlen("flag");
					(*strstr(speciesStr, "day")) = '\0';
					break;

				case flag_lim_id:
					strcpy(strPtr, varStr);
					speciesStr = strPtr + strlen("flag");
					(*strstr(speciesStr, "lim")) = '\0';
					break;

				case vla_T15_id:
					strcpy(strPtr, varStr);
					speciesStr = strPtr + strlen("vla_");
					(*strstr(speciesStr, "_T15")) = '\0';
					break;

				case min_move_temp_id:
				case max_move_temp_id:
				case min_move_salt_id:
				case max_move_salt_id:
				case log_mult_id:
                case norm_sigma_id:
                case flag_recruit_stochastic_id:
				case min_spawn_temp_id:
				case max_spawn_temp_id:
				case min_spawn_salt_id:
				case max_spawn_salt_id:
                case K_temp_const_id:
                case K_salt_const_id:
                case K_o2_const_id:
				case bleach_periodA_id:
				case bleach_periodB_id:
				case mBleach_id:
				case bleaching_rate_id:
				case bleach_recovery_rate_id:
				case bleach_tempshift_id:
				case bleach_growshift_id:
				case bleach_temp_id:
                case min_bleach_temp_id:
                case min_bleach_salt_id:
                case max_bleach_salt_id:
                case prop_zooxanth_id:
				case DHW_thresh_id:
                case threshdepth_id:
                case depmum_scalar_id:
				case rugFeedScalar_id:
				case HostRemin_id:
				case calcifRefBaseline_id:
				case calcifTconst_id:
				case calcifTcoefft_id:
				case calcifTopt_id:
				case calcifLambda_id:
				case FeedLightThresh_id:
				case PropLightFeed_id:
                case rug_erode_id:
                case rug_bleacherode_id:
                case rug_factor_id:
                case colony_ha_id:
				case coral_overgrow_id:
				case coral_compete_id:
				case coral_max_accel_trans_id:
				case coral_max_accelA_id:
				case coral_max_accelB_id:
				case CrecruitA_id:
				case CrecruitB_id:
				case CrecruitC_id:
				case rec_HabDepend_id:
				case RugCover_scalar_id:
                case sponge_overgrow_id:
                case sponge_compete_id:
                case Ksmother_B_id:
                case Ksmother_A_id:
                case Vmax_deltaSi_id:
                case Km_deltaSi_id:
                case rug_erode_sponge_id:
                case extra_feed_id:
                case aquacult_fry_id:
				case aquacult_age_harvest_id:
					strcpy(strPtr, varStr);
					(*strchr(strPtr, '_')) = '\0';
					speciesStr = strPtr;
					break;

				case Speed_id:
					speciesStr = varStr + strlen("Speed_");
					break;

				case mS_T15_id:
					strcpy(strPtr, varStr);
					speciesStr = strPtr + strlen("mS_");
					(*strstr(speciesStr, "_T15")) = '\0';
					break;
                /*
				case num_migrate_id:
					strcpy(strPtr, varStr);
					speciesStr = strPtr + strlen("flag");

					if (speciesStr[0] == 'j') {
						speciesStr++;
					}

					(*strstr(speciesStr, "Migrate")) = '\0';
					break;
                case multiyr_mig_id:
                    strcpy(strPtr, varStr);
                    speciesStr = strPtr + strlen("flag");
                        
                    if (speciesStr[0] == 'j') {
                        speciesStr++;
                    }
                        
                    (*strstr(speciesStr, "MultiyrMigrate")) = '\0';
                    break;
                 */
				default:
					quit("Create_Species_ParamXML: paramID not found = %d (%s) in switch statement. \n", paramID, speciesParamStructArray[index].tag, fileName);
					break;
				}

				//printf("speciesStr = %s\n", speciesStr);
				/* Get the groupCode */
				guild = Util_Get_FG_Index(bm, speciesStr);

				if(paramID == flux_thresh_id){
					if(guild == -1){
						quit("Create_Species_ParamXML: Param %s, the species string has not been recognised. Check and try again\n", buf);
					}
					if(FunctGroupArray[guild].groupType != SED_BACT)
						continue;
				}

				if(paramID == thresh_id){
					if(guild == -1){
						quit("Create_Species_ParamXML: Param %s, the species string has not been recognised. Check and try again\n", buf);
					}
					if(FunctGroupArray[guild].groupType == SED_BACT)
						continue;
				}

				if (guild != -1) {

					guildCheck[guild] = TRUE;
					/* Get the next token. */
					valuestr = strtok(NULL, seps);
					//printf("varStr = %s, valuestr = %s\n", varStr, valuestr);
					/* Set the group attribute - if the node does not already exist it will be created */
					Util_XML_Set_Node_Value(ATLANTIS_GROUP_ATTRIBUTE, newNode, FunctGroupArray[guild].groupCode, valuestr);
				}
			}
		}
	}

	/* Check that we read in all of the required values */
	Check_Species_Param(bm, fileName, index, paramID, guildCheck);
	regfree(&(regBuffer));
	free(guildCheck);
	return newNode;
}

/**
 * \brief Create cohort species param XML.
 *
 * This function is very similar to the Create_Species_ParamXML function expect that it expects an adult and juv value for each species parameter.
 *
 */
xmlNodePtr Create_Species_Cohort_ParamXML(MSEBoxModel *bm, char *fileName, FILE *inputFP, xmlDocPtr doc, xmlNodePtr parent, int paramID, char *comment,
		char *units, int type, char *recommendedValue, void(*fp)(MSEBoxModel *bm, char *fileName, xmlNodePtr parent, char *str, char *valueStr)) {
	char *speciesStr = NULL;
	int buflen = STRLEN;
	char ch, buf[STRLEN], seps[] = " ,\t\n";
	char *varStr, *valuestr;
	char strPtr[50];
	char tempBuf[STRLEN];
	xmlNodePtr newNode;
	regex_t regBuffer;
	int speciesIndex, nstage;
	char commentString[200];
	int size = 0;
	int **guildCheck = Util_Alloc_Init_2D_Int(bm->K_num_max_stages, bm->K_num_tot_sp, FALSE);
	int index;
	int lengthValue;
	char lengthStr[100];
	char *lengthString;

	if( verbose > 1)
		printf("Creating cohort XML\n");

	sprintf(lengthStr, "%d", bm->K_num_max_stages);


	/* Get the index of this paramID in the speciesParamStructArray structure - done this way so the
	 * values in speciesParamStructArray don't have to be in any particular order
	 */
	index = Util_XML_Get_Param_Index(cohortSpeciesParamStructArray, cohortDepParams, paramID);

	strcpy(commentString, comment);
	strcat(commentString, ". Units = ");
	strcat(commentString, units);

	at_compileRegExpression(&regBuffer, cohortSpeciesParamStructArray[index].regEx);

	fseek(inputFP, 0L, 0);

	/* Does the attribute node already exist? If not create it */
	newNode = Util_XML_Get_Node(ATLANTIS_ATTRIBUTE, parent, cohortSpeciesParamStructArray[index].tag);
	if (newNode == NULL)
		newNode = Util_XML_Create_Node(ATLANTIS_ATTRIBUTE, parent, cohortSpeciesParamStructArray[index].tag, comment, units, "");

	while (fgets(buf, buflen, inputFP) != NULL) {

		strcpy(tempBuf, buf);
		ch = tempBuf[0];

		if (ch == '#') {
			continue;
		}

		if (!isalpha(ch)) {
			continue;
		}

		if ((ch != '#') && (ch != ' ') && (ch != '\n') && (ch != '\t')) {
			/* it's a parameter line */

			// Get the first token in the line.
			varStr = strtok(tempBuf, seps);

			/* Does this match the regular expresion given? */
			if (regexec(&(regBuffer), varStr, elementsof ( pmatch ), pmatch, 0) == 0) {

				/* Get the speciesName. */
				switch (paramID) {
				case mL_T15_id:
				case mQ_T15_id:
                case mLext_id:
                case mPext_id:
                case L_turbid_id:
                case a_turbid_id:
                case b_turbid_id:
                case rugosity_inc_id:
				case rugosity_dec_id:
                case colony_diam_id:
					strcpy(strPtr, varStr);
					(*strchr(strPtr, '_')) = '\0';
					speciesStr = strPtr;
					break;
				default:
					quit("Create_Species_Cohort_ParamXML paramID not found = %d\n", paramID);
					break;

				}

				/* Get the speciesIndex; */
				speciesIndex = Util_Get_FG_Index(bm, speciesStr);
				if (speciesIndex == -1){
					quit("ecol_readSpeciesParam - species not found '%s', original parameter = %s, tag = %s in file %s\n", speciesStr, varStr,
							cohortSpeciesParamStructArray[index].tag, fileName);
				}

				/* Now we know what type of group it is. Check to see if a multiple entries should have been found */
				if (FunctGroupArray[speciesIndex].groupAgeType == AGE_STRUCTURED_BIOMASS
						|| FunctGroupArray[speciesIndex].groupAgeType == AGE_STRUCTURED){
					size = FunctGroupArray[speciesIndex].numStages;
				} else if ((FunctGroupArray[speciesIndex].groupType == SEAGRASS) && (FunctGroupArray[speciesIndex].numStages > 1)) {
                    size = FunctGroupArray[speciesIndex].numStages;
                } else {
					size = 1;
				}

				/* Put the values in */
				if(size > 1){
					/* Check the length */
					lengthString = strtok(NULL, seps);
					lengthValue = atoi(lengthString);
					if(lengthValue != size){
						warn("Create_Species_Cohort_ParamXML - Length of attribute %s in file %s is not the required length %d in file %s\n", varStr, fileName, size, fileName);
					}

					for(nstage = 0; nstage < bm->K_num_max_stages; nstage++){
						guildCheck[speciesIndex][nstage] = TRUE;  // As should all be there as the length matches
					}

					/* Get the next line - this contains the values array */
					Get_Next_Line(bm, inputFP, buf);
					trim(buf);
					if(strlen(buf) == 0){
						quit("ERROR: No value found in input file %s for parameter %s\n", fileName, cohortSpeciesParamStructArray[index].tag);
					}

					/* Check the value string is only numbers */
					if(CheckIsDigits(buf) < 0)
						quit("ERROR: Value for parameter %s in input file %s is not valid %s\n", cohortSpeciesParamStructArray[index].tag, fileName, buf);
					fp(bm, fileName, newNode, varStr, buf);
				} else {

					/* Get the next token. */
					nstage = 0;
					valuestr = strtok(NULL, seps);
					Util_XML_Set_Node_Value(ATLANTIS_GROUP_ATTRIBUTE, newNode, FunctGroupArray[speciesIndex].groupCode, valuestr);
					guildCheck[speciesIndex][nstage] = TRUE;
				}
			}
		}
	}

	Check_Species_Cohort_Param(bm, fileName, index, paramID, guildCheck);

	i_free2d(guildCheck);
	regfree(&(regBuffer));
	Util_XML_Add_Node_Property(newNode, "AttributeType", AtlantisXMLAttributeTypeStrings[type]);
	Util_XML_Set_Node_Property(newNode, "ArrayLength", lengthStr);
	return newNode;

}



/**
 * \brief Create species param XML for time spawning.
 *
 * This function is very similar to the Create_Species_ParamXML function expect that it expects an adult and juv value for each species parameter.
 *
 */
xmlNodePtr Create_Species_Spawn_ParamXML(MSEBoxModel *bm, char *fileName, FILE *inputFP, xmlDocPtr doc, xmlNodePtr parent, int paramID, char *comment,
		char *units, int type, char *recommendedValue, void(*fp)(MSEBoxModel *bm, char *fileName, xmlNodePtr parent, char *str, char *valueStr)) {
	char *speciesStr = NULL;
	int buflen = STRLEN;
	char ch, buf[STRLEN], seps[] = " ,\t\n";
	char *varStr, *valuestr;
	char strPtr[50];
	char tempBuf[STRLEN];
	xmlNodePtr newNode;
	regex_t regBuffer;
	int nspawn, speciesIndex;
	char commentString[200];
	int size = 0;
	int **guildCheck = Util_Alloc_Init_2D_Int(bm->K_num_max_spawns, bm->K_num_tot_sp, FALSE);
	int index;
	int lengthValue;
	char lengthStr[100];
	char *lengthString;

	if( verbose > 1)
		printf("Creating spawn XML\n"); 
	
	sprintf(lengthStr, "%d", bm->K_num_max_spawns);

	/* Get the index of this paramID in the speciesParamStructArray structure - done this way so the
	 * values in speciesParamStructArray don't have to be in any particular order
	 */
	index = Util_XML_Get_Param_Index(spawnSpeciesParamStructArray, spawnDepParams, paramID);

	strcpy(commentString, comment);
	strcat(commentString, ". Units = ");
	strcat(commentString, units);

	at_compileRegExpression(&regBuffer, spawnSpeciesParamStructArray[index].regEx);

	fseek(inputFP, 0L, 0);

	/* Does the attribute node already exist? If not create it */
	newNode = Util_XML_Get_Node(ATLANTIS_ATTRIBUTE, parent, spawnSpeciesParamStructArray[index].tag);
	if (newNode == NULL)
		newNode = Util_XML_Create_Node(ATLANTIS_ATTRIBUTE, parent, spawnSpeciesParamStructArray[index].tag, comment, units, "");

	while (fgets(buf, buflen, inputFP) != NULL) {

		strcpy(tempBuf, buf);
		ch = tempBuf[0];

		if (ch == '#') {
			continue;
		}

		if (!isalpha(ch)) {
			continue;
		}

		if ((ch != '#') && (ch != ' ') && (ch != '\n') && (ch != '\t')) {
			/* it's a parameter line */

			// Get the first token in the line.
			varStr = strtok(tempBuf, seps);

			/* Does this match the regular expresion given? */
			if (regexec(&(regBuffer), varStr, elementsof ( pmatch ), pmatch, 0) == 0) {

				/* Get the speciesName. */
				switch (paramID) {
				case Time_Spawn_id:
					strcpy(strPtr, varStr);
					(*strchr(strPtr, '_')) = '\0';
					speciesStr = strPtr;
					break;

				default:
					quit("Create_Species_Spawn_ParamXML paramID not found = %d\n", paramID);
					break;

				}

				/* Get the speciesIndex; */
				speciesIndex = Util_Get_FG_Index(bm, speciesStr);
				if (speciesIndex == -1){
					quit("ecol_readSpeciesParam - species not found '%s', original parameter = %s, tag = %s in file %s\n", speciesStr, varStr,
							spawnSpeciesParamStructArray[index].tag, fileName);
				}

				/* if this is not an age structured group ignore the value */
				if(FunctGroupArray[speciesIndex].groupAgeType == BIOMASS && !FunctGroupArray[speciesIndex].isCultured ){
					continue;
				}

				/* Now we know what type of group it is. Check to see if a multiple entries should have been found */
				if (FunctGroupArray[speciesIndex].groupAgeType == AGE_STRUCTURED_BIOMASS
						|| FunctGroupArray[speciesIndex].groupAgeType == AGE_STRUCTURED){
					size = FunctGroupArray[speciesIndex].numSpawns;
				} else {
					size = 1;
				}

				/* Put the values in */
				if(size > 1){
					/* Check the length */
					lengthString = strtok(NULL, seps);
					lengthValue = atoi(lengthString);
					if(lengthValue != size){
						warn("Create_Species_Spawn_ParamXML - Length of attribute %s in file %s is not the required length %d in file %s\n", varStr, fileName, size, fileName);
					}

					for(nspawn = 0; nspawn < bm->K_num_max_spawns; nspawn++){
						guildCheck[speciesIndex][nspawn] = TRUE;  // As should all be there as the length matches
					}

					/* Get the next line - this contains the values array */
					Get_Next_Line(bm, inputFP, buf);
					trim(buf);
					if(strlen(buf) == 0){
						quit("ERROR: No value found in input file %s for parameter %s\n", fileName, spawnSpeciesParamStructArray[index].tag);
					}

					/* Check the value string is only numbers */
					if(CheckIsDigits(buf) < 0)
						quit("ERROR: Value for parameter %s in input file %s is not valid %s\n", spawnSpeciesParamStructArray[index].tag, fileName, buf);
					fp(bm, fileName, newNode, varStr, buf);
				} else {

					/* Get the next token. */
					nspawn = 0;
					valuestr = strtok(NULL, seps);
					Util_XML_Set_Node_Value(ATLANTIS_GROUP_ATTRIBUTE, newNode, FunctGroupArray[speciesIndex].groupCode, valuestr);
					guildCheck[speciesIndex][nspawn] = TRUE;
				}
			}
		}
	}

	Check_Species_Spawn_Param(bm, fileName, index, paramID, guildCheck);

	i_free2d(guildCheck);
	regfree(&(regBuffer));
	Util_XML_Add_Node_Property(newNode, "AttributeType", AtlantisXMLAttributeTypeStrings[type]);
	Util_XML_Set_Node_Property(newNode, "ArrayLength", lengthStr);
	return newNode;

}

int Check_Fishery_Species_Param(MSEBoxModel *bm, char *fileName, int index, int paramID, int *guildCheck) {
	int guild;

	switch (FisheryGroupParamsArray[index].paramType) {
	case SP_AGE_STRUCTURED:
		for (guild = 0; guild < bm->K_num_tot_sp; guild++) {
			if (FunctGroupArray[guild].isFished == TRUE && (FunctGroupArray[guild].groupAgeType == AGE_STRUCTURED || FunctGroupArray[guild].groupAgeType
					== AGE_STRUCTURED_BIOMASS))
				if (guildCheck[guild] == FALSE)
					Print_Fishery_Species_Param_Error(bm, fileName, paramID, guild);
		}
		break;
	case SP_VERTS:
		for (guild = 0; guild < bm->K_num_tot_sp; guild++) {
			if (FunctGroupArray[guild].isVertebrate == TRUE)
				if (guildCheck[guild] == FALSE)
					Print_Fishery_Species_Param_Error(bm, fileName, paramID, guild);
		}
		break;
	case SP_FISHED:
		for (guild = 0; guild < bm->K_num_tot_sp; guild++) {
			if (FunctGroupArray[guild].isFished == TRUE)
				if (guildCheck[guild] == FALSE)
					Print_Fishery_Species_Param_Error(bm, fileName, paramID, guild);
		}
		break;

	default:

		quit("Check_Fishery_Species_Param - no type specified for param %s\n", bm->SP_FISHERYprmsName[paramID]);
		for (guild = 0; guild < bm->K_num_tot_sp; guild++) {
			if (FunctGroupArray[guild].isFished == TRUE)
				if (guildCheck[guild] == FALSE)
					Print_Fishery_Species_Param_Error(bm, fileName, paramID, guild);
		}
		break;
	}
	return 0;
}

/**
 *
 * 	\brief Parse the input values for a fishery species parameter and write to the given XML document.
 *
 *
 * 	This is used to parse parameters where there is an array of size K_num_fisheries for each fished species
 * 	in the model.
 *
 *
 *
 */
xmlNodePtr Create_Harvest_Fishery_Group_ParamXML(MSEBoxModel *bm, FILE *inputFP, char *fileName, xmlNodePtr parent, int paramID, char *comment, char *units,
		int type) {
	char *speciesStr = NULL;
	int buflen = STRLEN, guild, index;
	char ch, buf[STRLEN], seps[] = " ,\t\n";
	char *varStr;
	char strPtr[50];
	char tempBuf[STRLEN];
	xmlNodePtr newNode;
	regex_t regBuffer;
	char lengthStr[200];
	int *guildCheck = Util_Alloc_Init_1D_Int(bm->K_num_tot_sp, FALSE);

	index = Util_XML_Get_Param_Index(FisheryGroupParamsArray, numGroupFisheryParams, paramID);

	if(index == -1){
		quit("Create_Harvest_Fishery_Group_ParamXML param (%d) not found in the FisheryGroupParamsArray array.\n", paramID);
	}
	sprintf(lengthStr, "%d", bm->K_num_fisheries);

	at_compileRegExpression(&regBuffer, FisheryGroupParamsArray[index].regEx);
	if (verbose > 0)
		printf("Reading in %s values\n", FisheryGroupParamsArray[index].tag);

	fseek(inputFP, 0L, 0);

	/* Check to see if the parent node already exists
	 * If not create the new node.
	 * This allows for cases when we want to pre fill have values in the tree.
	 * */

	newNode = Util_XML_Get_Node(ATLANTIS_ATTRIBUTE, parent, FisheryGroupParamsArray[index].tag);
	if (newNode == NULL)
		newNode = Util_XML_Create_Node(ATLANTIS_ATTRIBUTE, parent, FisheryGroupParamsArray[index].tag, comment, units, "");

	while (fgets(buf, buflen, inputFP) != NULL) {

		strcpy(tempBuf, buf);
		ch = tempBuf[0];

		if (ch == '#' || !isalpha(ch)) {
			continue;
		}

		if ((ch != '#') && (ch != ' ') && (ch != '\n') && (ch != '\t')) {
			/* it's a parameter line */

			// Get the first token in the line.
			varStr = strtok(tempBuf, seps);

			if (regexec(&(regBuffer), varStr, elementsof ( pmatch ), pmatch, 0) == 0) {
				switch (paramID) {

				case sel_id:
				case flagQchange_id:
				case q_id:
				case mFC_id:
                case flagFchange_id:
				case flaghabitat_id:
				case flagescapement_id:
                case spawn_closure_id:
				case flagF_id:
				case Ka_escape_id:
				case Kb_escape_id:
				case flagdiscard_id:
				case flagchangeDISCRD_id:
				case FFCDR_id:
				case FC_thresh_id:
				case FC_high_thresh_id:
				case FCthreshli_id:
				case k_retain_id:
				case incidmort_id:
				case k_waste_id:
				case p_escape_id:
				case flagimposecatch_id:
				case imposecatchstart_id:
				case imposecatchend_id:
				case FC_reportscale_id:
				case trip_lim_id:
				case prop_spawn_close_id:
				case TAC_id:
				case FC_case_id:
                case avail_id:
                case assess_nf_id:
                case flagPerShotCPUE_id:
                case flagRecordCPUE_id:
					strcpy(strPtr, varStr);
					speciesStr = strPtr + strlen(bm->SP_FISHERYprmsName[paramID]);
					break;

				case Q_num_changes_id:
				case mFC_start_age_id:
                case mFC_end_age_id:
				case mFC_num_changes_id:
				case DISCRD_num_changes_id:
				case TAC_num_changes_id:
				case saleprice_id:
				case tax_id:
				case deemedvalue_id:
					strcpy(strPtr, varStr);
					(*strstr(strPtr, bm->SP_FISHERYprmsName[paramID])) = '\0';
					speciesStr = strPtr;
					break;
				default:
					quit("Create_Harvest_Fishery_Group_ParamXML: paramID not found = %d\n", paramID);
					break;
				}

				/* Get the groupCode */
				guild = Util_Get_FG_Index(bm, speciesStr);
				if (guild != -1) {

					guildCheck[guild] = TRUE;
					// Get the next line.
					fgets(buf, buflen, inputFP);
					/* Set the group attribute - if the node does not already exist it will be created */
					Util_XML_Set_Node_Value(ATLANTIS_GROUP_ATTRIBUTE, newNode, FunctGroupArray[guild].groupCode, buf);
				} else {
					quit("Create_Harvest_Fishery_Group_ParamXML: SpeciesString not recognised '%s'.\n varStr = %s, bm->SP_FISHERYprmsName[%d] = %s\n", speciesStr,
							varStr, paramID, bm->SP_FISHERYprmsName[paramID]);
				}
			}
		}
	}

	/* Check that we found values for all the groups that we should have */
	Check_Fishery_Species_Param(bm, fileName, index, paramID, guildCheck);

	Util_XML_Set_Node_Property(newNode, "AttributeType", AtlantisXMLAttributeTypeStrings[type]);
	Util_XML_Set_Node_Property(newNode, "ArrayLength", lengthStr);

	i_free1d(guildCheck);
	regfree(&(regBuffer));

	return newNode;
}



void Print_Fishery_Param_Error(MSEBoxModel *bm, char *fileName, int paramID, int fisheryIndex) {
	fprintf(stderr, "\n\nERROR: Failed to find fishery parameter %s in file %s for fishery %s\n\n", bm->fisheryParamNAME[paramID], fileName,
			FisheryArray[fisheryIndex].fisheryCode);
	fflush(stderr);

	quit("ERROR\n");
}

int Check_Fishery_Param(MSEBoxModel *bm, char *fileName, int index, int paramID, int *fisheryCheck) {
	int fishery;

	for (fishery = 0; fishery < bm->K_num_fisheries; fishery++) {
		if (fisheryCheck[fishery] == FALSE)
			Print_Fishery_Param_Error(bm, fileName, paramID, fishery);
	}
	return 0;
}

void Print_Fishery_Species_Param_Error(MSEBoxModel *bm, char *fileName, int paramID, int guild) {
	fprintf(stderr, "\n\nERROR: Failed to find fishery species parameter %s in file %s for guild %s\n\n", bm->SP_FISHERYprmsName[paramID], fileName,
			FunctGroupArray[guild].groupCode);
	fflush(stderr);
	fflush(stdout);

	quit("ERROR\n");
}


/**
 */
xmlNodePtr Create_Fishery_ParamXML(MSEBoxModel *bm, FILE *inputFP, char *fileName, xmlNodePtr parent, int paramID, char *comment, char *units, int type,
		char *recommendedValue) {

	char *fisheryStr = NULL;
	int buflen = STRLEN;
	char ch, buf[STRLEN], seps[] = " ,\t\n";
	char *varStr, *valuestr;
	char strPtr[50];
	char tempBuf[STRLEN];
	xmlNodePtr newNode;
	regex_t regBuffer;
	int index;
	int paramIndex;
	int *fisheryCheck = Util_Alloc_Init_1D_Int(bm->K_num_fisheries, FALSE);

	paramIndex = Util_XML_Get_Param_Index(FisheryParamsArray, numFisheryParams, paramID);

	at_compileRegExpression(&regBuffer, FisheryParamsArray[paramIndex].regEx);

	if (verbose > 0)
		printf("Reading in %s values\n", FisheryParamsArray[paramIndex].tag);

	fseek(inputFP, 0L, 0);

	/* Check to see if the parent node already exists
	 * If not create the new node.
	 * This allows for cases when we want to pre fill have values in the tree.
	 * */

	newNode = Util_XML_Get_Node(ATLANTIS_ATTRIBUTE, parent, FisheryParamsArray[paramIndex].tag);
	if (newNode == NULL)
		newNode = Util_XML_Create_Node(ATLANTIS_ATTRIBUTE, parent, FisheryParamsArray[paramIndex].tag, comment, units, "");

	while (fgets(buf, buflen, inputFP) != NULL) {

		strcpy(tempBuf, buf);
		ch = tempBuf[0];

		if (ch == '#') {
			continue;
		}

		if (!isalpha(ch)) {
			continue;
		}

		if ((ch != '#') && (ch != ' ') && (ch != '\n') && (ch != '\t')) {
			/* it's a parameter line */

			// Get the first token in the line.
			varStr = strtok(tempBuf, seps);

			if (regexec(&(regBuffer), varStr, elementsof ( pmatch ), pmatch, 0) == 0) {

				/* Get the speciesName. */
				switch (paramID) {

				case fisheriesflagactive_id: //Period of activity for fisheries
					strcpy(strPtr, varStr);
					fisheryStr = strPtr + strlen(bm->fisheryParamNAME[paramID]);
					(*strstr(fisheryStr, "day")) = '\0';
					break;
				case flagTACpartipcate_id:

					strcpy(strPtr, varStr);
					fisheryStr = strPtr + strlen(bm->fisheryParamNAME[paramID]);
					break;
				case tStart_id: //Start of fishery (e.g. if reproducing history and sequential start of some fisheries)
				case tEnd_id: //End of fishery (e.g. if closing an international fishery)
				case start_manage_id: //Start of fishery management (so things won't necessarily change immediately)
				case end_manage_id: //End of fishery management (so have complete control)
				case FC_restrict_id:
				case FC_restrict_endangered_id:
				case seasonopen_id: //Seasonal fishery parameter
				case seasonclose_id:
				case infringe_id: //Spatial management
				case cap_id: //Effort caps
				case CPUE_effort_thresh_id: //CPUE threshold for effort shifts
				case CPUE_effort_scale_id: //CPUE scalar for effort shifts
				case flagdempelfishery_id:
				case flageffortmodel_id:
				case flagexplore_id:
				case flagdropEFF_id:
				case selcurve_id:
				case flagmanage_id:
				case flagcap_id:
				case flagcap_peak_id:
				case flaguseall_id:
				case flagseasonal_id:
                case flag_framebased_id:
				case flagmpa_id:
				case flagchangeSEL_id: //Scenario specification flags
				case flagchangeP_id:
				case flagchangeSWEPT_id:
				case flagchangeEFF_id:
				case flagchangeseason_id:
				case TACchange_id:
				case maxFCdepth_id: //Depth restrictions
				case minFCdepth_id: //Depth restrictions
				case landallTAC_sp_id:
				case max_num_sp_id:
				case FC_period_id:
				case FC_period2_id:
				case FC_endanger_period_id:
				case EFF_num_changes_id: //Changes in fishing pressure
				case CAP_num_changes_id: //Fishing effort cap changes
				case SEL_num_changes_id: //Selectivity curve changes
				case P_num_changes_id: //Fishing coverage changes
				case swept_area_id: //Swept area parameters
				case SWEPT_num_changes_id:
				case sel_b_id: //Selectivity curve parameters
				case sel_lsm_id:
				case sel_normsigma_id:
				case sel_normlsm_id:
				case sel_lognormsigma_id:
				case sel_lognormlsm_id:
				case sel_gammasigma_id:
				case sel_gammalsm_id:
                case sel_bisigma_id:
                case sel_bisigma2_id:
                case sel_ampli_id:
				case sel_bilsm1_id:
				case sel_bilsm2_id:
				case maxsaleprice_id: //Initialising max prices per fishery
				case mEff_max_id: //Simple effort model
				case mEff_a_id:
				case mEff_offset_id:
				case mEff_testfish_id: //Exploratory fishing effort
				case mEff_thresh_id: //Effort threshold for reducing effort
				case mEff_thresh_top_id: //Effort threshold for increasing effort
				case mEff_shift_id: //Effort shift once outside CPUE band
				case mFCscale_id: //Fuel cost penalty
				case EffortLevel_id: //Starting effort levels to go with effort spatial vectors
				case use_min_lever_id:
				case flagDV_id:
				case nsubfleets_id:
				case nlicence_id:
				case flagMultiSpEffort_id:
				case buybackdate_id:
				case mFCpeak_id:
					strcpy(strPtr, varStr);
					//printf("strPtr = %s\n", strPtr);
					(*strstr(strPtr, bm->fisheryParamNAME[paramID])) = '\0';
					fisheryStr = strPtr;
					break;
				default:
					quit("Create_Fishery_ParamXML fishery param %d not recognised. Tag = %s in file %s.\n", paramID, FisheryParamsArray[paramIndex].tag, fileName);
					break;

				}

				if (fisheryStr == NULL) {
					quit("Create_Fishery_ParamXML - fisheryStr = NULL");
				}
				/* Get the fisheryCode */
				index = Util_Get_Fishery_Index(bm, fisheryStr);
				if (index != -1) {

					fisheryCheck[index] = TRUE;
					/* Get the next token. */
					valuestr = strtok(NULL, seps);
					/* Set the group attribute - if the node does not already exist it will be created */
					Util_XML_Set_Node_Value(ATLANTIS_FISHERY_ATTRIBUTE, newNode, FisheryArray[index].fisheryCode, valuestr);
				} else {
					//					printf("varStr = %s\n", varStr);
					//					printf("bm->fisheryParamNAME[paramID] = %s\n", bm->fisheryParamNAME[paramID]);
					quit("Fishery string not recognised %s\n", fisheryStr);
				}
			}
		}
	}

	/* Check that we read in all of the required values */
	Check_Fishery_Param(bm, fileName, paramIndex, paramID, fisheryCheck);
	regfree(&(regBuffer));
	Util_XML_Add_Node_Property(newNode, "AttributeType", AtlantisXMLAttributeTypeStrings[type]);
	Util_XML_Add_Node_Property(newNode, "AttributeRecommendedValue", recommendedValue);
	free(fisheryCheck);
	return newNode;
}


/**
 * \brief Strip off the param string from the start of the str given and find the fisheryIndex. Then set the
 * node under the given parent where the name is the fishery with the given value.
 *
 */
void Fisheries_NodeXML(MSEBoxModel *bm, char *fileName, xmlNodePtr parent, char *str, char *valueStr, char *paramStr) {

	int fisheryIndex;
	char *fisheryStr;

	/* Get the fishery index based on the str value */
	fisheryStr = str + strlen(paramStr);

	if (fisheryStr == NULL) {
		quit("Fisheries_NodeXML - str not of the correct format %s in file %s", str, fileName);
	}
	fisheryIndex = Util_Get_Fishery_Index(bm, fisheryStr);

	Util_XML_Set_Node_Value(ATLANTIS_FISHERY_ATTRIBUTE, parent, FisheryArray[fisheryIndex].fisheryCode, valueStr);
}
/**
 * \brief Strip off the param string from the start of the str given and find the speciesIndex. Then set the
 * node under the given parent where the name is the group with the given value.
 *
 */

void Species_NodeXML(MSEBoxModel *bm, char *fileName, xmlNodePtr parent, char *str, char *valueStr, char *paramStr) {

	int speciesIndex;
	char *speciesStr;

	/* Get the species index based on the str value */
	speciesStr = str + strlen(paramStr);

	if (speciesStr == NULL) {
		quit("Species_NodeXML - str not of the correct format %s", str);
	}
	speciesIndex = Util_Get_FG_Index_From_Token(bm, speciesStr, fileName, str, TRUE);

	Util_XML_Set_Node_Value(ATLANTIS_GROUP_ATTRIBUTE, parent, FunctGroupArray[speciesIndex].groupCode, valueStr);
}



void Create_Fished_Groups_ChangeXML(MSEBoxModel *bm, FILE *fp, char *fileName, xmlDocPtr doc, xmlNodePtr parent, char *paramName, char *comment) {

	char commentStr[MAX_XML_COMMENT_LENGTH];
	char parameterName[200];

	sprintf(parameterName, "^%s_start", paramName);
	sprintf(
			commentStr,
			"Arrays to indicate when each change in %s begins (days from run start).\nThere must be as many entries in this array as the MAXIMUM number of changes in %s across all groups (e.g. if the highest number of changes for a single fishery by a single group is 6 then there must be 6 entries etc).",
			comment, comment);

	Parse_File(bm, fp, fileName, parent, "ChangeStart", parameterName, commentStr, "d", XML_TYPE_INTEGERARRAY, -1, TRUE, Create_Impacted_Species_XMLNodes, Species_Last_XMLFunction);

	sprintf(parameterName, "^%s_period", paramName);
	sprintf(
			commentStr,
			"Arrays to indicate period over which change in %s occurs (days). \nThere must be as many entries in this array as the MAXIMUM number of changes in %s across all groups.",
			comment, comment);

	Parse_File(bm, fp, fileName, parent, "ChangePeriod", parameterName, commentStr, "d", XML_TYPE_INTEGERARRAY, -1, TRUE, Create_Impacted_Species_XMLNodes, Species_Last_XMLFunction);

	sprintf(parameterName, "^%s_mult", paramName);
	sprintf(
			commentStr,
			"Arrays to indicate new level of %s for that group. \nIt a a multiplication factor relative to the original level of %s. There must be as many entries in this array as the MAXIMUM number of changes in %s across all groups.",
			comment, comment, comment);
	Parse_File(bm, fp, fileName, parent, "ChangeMult", parameterName, commentStr, "", XML_TYPE_FLOATARRAY, -1, TRUE, Create_Impacted_Species_XMLNodes, Species_Last_XMLFunction);

}

void Create_Fishery_ChangeXML(MSEBoxModel *bm, FILE *fp, char *fileName, xmlDocPtr doc, xmlNodePtr parent, char *paramName, char *comment, char *multParam) {

	char commentStr[MAX_XML_COMMENT_LENGTH];
	char parameterName[200];

	sprintf(parameterName, "^%s_start", paramName);
	sprintf(
			commentStr,
			"Arrays to indicate when each change in %s begins (days from run start).\n There must be as many entries in this array as there are number of changes in %s for that fishery.",
			comment, comment);

	Parse_File(bm, fp, fileName, parent, "ChangeStart", parameterName, commentStr, "d", XML_TYPE_INTEGERARRAY, -1, TRUE, Create_Fishery_XMLNodes, Fishery_Last_XMLFunction);

	sprintf(parameterName, "^%s_period", paramName);
	sprintf(
			commentStr,
			"Arrays to indicate time period over which change in %s occurs (days). \nThere must be as many entries in this array as there are number of changes in %s for that fishery.",
			comment, comment);
	Parse_File(bm, fp, fileName, parent, "ChangePeriod", parameterName, commentStr, "d", XML_TYPE_INTEGERARRAY, -1, TRUE, Create_Fishery_XMLNodes, Fishery_Last_XMLFunction);

	sprintf(parameterName, "^%s_mult", paramName);
	sprintf(
			commentStr,
			"Arrays to indicate proportional change in %s (relative to %s entered above). \nThere must be as many entries in this array as there are number of changes in %s for that fishery.",
			comment, multParam, comment);
	Parse_File(bm, fp, fileName, parent, "ChangeMult", parameterName, commentStr, "", XML_TYPE_FLOATARRAY, -1, TRUE, Create_Fishery_XMLNodes, Fishery_Last_XMLFunction);

}


void Print_RBC_Species_Param_Error(MSEBoxModel *bm, char *fileName, int paramIndex, int guild) {
	fprintf(stderr, "ERROR: Failed to find RBC species parameter %s (%d) in file %s for group %s.\n", RBCParamStrings[paramIndex], paramIndex, fileName,
			FunctGroupArray[guild].groupCode);
	fflush(stderr);
	fflush(stdout);

	quit("ERROR\n");
}

int Check_RBC_Species_Param(MSEBoxModel *bm, char *fileName, int index, int paramID, int *guildCheck) {
	int guild;

	switch (RBCSpeciesParamStructArray[index].paramType) {
	case SP_FISHED:
		for (guild = 0; guild < bm->K_num_tot_sp; guild++) {
			if (FunctGroupArray[guild].isFished == TRUE) {
				if (guildCheck[guild] == FALSE)
					Print_RBC_Species_Param_Error(bm, fileName, paramID, guild);
			}
		}
		break;
        case SP_IMPACTED:
            for (guild = 0; guild < bm->K_num_tot_sp; guild++) {
                if (FunctGroupArray[guild].isImpacted == TRUE) {
                    if (guildCheck[guild] == FALSE)
                        Print_RBC_Species_Param_Error(bm, fileName, paramID, guild);
                }
            }
            break;
	default:
		quit("Check_RBC_Species_Param - paramType %d not found\n", RBCSpeciesParamStructArray[index].paramType);
		break;
	}
	return TRUE;
}


/**
 *
 * \brief Create a species parameter in the XML document for the given paramID.
 *
 *
 * @param bm The pointer to the BoxModel
 * @param fileName The name of the current prm input file. This is used to create useful error messages.
 * @param inputFP The pointer to the input prm file.
 * @param paramID The species parameter defined in the SPECIES_PARAMS enum
 * @param comment The comment string to be written to the XML file.
 * @param units: The units value written to the XML file.
 * @param type: The type of XML node to create.
 * @recommendedValue: A recommended value if one is suitable.*
 *
 *
 */
xmlNodePtr Create_RBC_Species_ParamXML(MSEBoxModel *bm, char *fileName, FILE *inputFP, xmlNodePtr parent,
		int paramID, char *comment, char *units, ATL_XML_ATTRIBUTE_TYPES type,
		char *recommendedValue) {
	int buflen = 5000, guild, index;
	char *varStr, *valuestr, *speciesStr = NULL;
	char tempBuf[5000], ch, buf[5000], seps[] = " ,\t\n";
	xmlNodePtr newNode;
	regex_t regBuffer;
	int *guildCheck = Util_Alloc_Init_1D_Int(bm->K_num_tot_sp, FALSE);

    
    //printf("Doing %s\n", comment);
    
	/* Get the index of this paramID in the speciesParamStructArray structure - done this way so the
	 * values in speciesParamStructArray don't have to be in any particular order
	 */
    
    
    
	index = Util_XML_Get_Param_Index(RBCSpeciesParamStructArray, num_rbc_species_params_id, paramID);

	at_compileRegExpression(&regBuffer, RBCSpeciesParamStructArray[index].regEx);
	//if (verbose > 0)
	//printf("Reading in %s values, paramID = %d\n", speciesParamStructArray[index].tag, paramID);

	fseek(inputFP, 0L, 0);

	/* Check to see if the parent node already exists
	 * If not create the new node.
	 * This allows for cases when we want to pre fill have values in the tree.
	 * */

	newNode = Util_XML_Get_Node(ATLANTIS_ATTRIBUTE, parent, RBCSpeciesParamStructArray[index].tag);
	if (newNode == NULL) {
		newNode = Util_XML_Create_Node(ATLANTIS_ATTRIBUTE, parent, RBCSpeciesParamStructArray[index].tag, comment, units, "");
		Util_XML_Add_Node_Property(newNode, "AttributeType", AtlantisXMLAttributeTypeStrings[type]);
		Util_XML_Add_Node_Property(newNode, "AttributeRecommendedValue", recommendedValue);
	}

	while (fgets(buf, buflen, inputFP) != NULL) {

		strcpy(tempBuf, buf);
		ch = tempBuf[0];

		if (ch == '#' || !isalpha(ch))
			continue;

		if ((ch != '#') && (ch != ' ') && (ch != '\n') && (ch != '\t')) { /* it's a parameter line */

			varStr = strtok(tempBuf, seps); // Get the first token in the line.

			if (regexec(&(regBuffer), varStr, elementsof ( pmatch ), pmatch, 0) == 0) {

				//printf("varStr = %s\n",varStr);
				/* Get the speciesName. */
				switch (paramID) {
				case DiscType_id:
				case MaxH_id:
				case Growthage_L1_id:
				case Growthage_L2_id:
				case MinCatch_id:
				case Maturity_Inflect_id:
				case Maturity_Slope_id:
				case tiertype_id:
				case T1_steep_phase_id:
				case Tier3_Fcalc_id:
				case Tier3_time_id:
				case Tier3_maxage_id:
				case Tier3_M_id:
				case Tier3_S25_id:
				case Tier3_S50_id:
				case Tier3_F_id:
				case Tier3_h_id:
				case Tier3_matlen_id:
				case Tier3_maxF_id:
				case Tier4_avtime_id:
				case Tier4_CPUEyrmin_id:
                case Tier4_CPUEyrmax_id:
                case Tier4_Bo_correct_id:
				case Tier4_m_id:
				case Tier4_alpha_id:
				case Tier4_Cmaxmult_id:
				case Tier5_length_id:
				case Tier5_S50_id:
				case Tier5_cv_id:
				case Tier5_flt_id:
				case Tier5_reg_id:
				case Tier5_p_id:
                case Tier5sel_id:
                case Tier5q_id:
                case Tier1Sig_id:
                case Tier2Sig_id:
                case Tier3Sig_id:
                case isTriggerSpecies_id:
                case trigger_threshold_id:
                case UseRBCAveraging_id:
				case PostRule_id:
				case CPUEmult_id:
				case MaxChange_id:
                case MG_offset_id:
                case BallParkF_id:
                case BallParkYr_id:
                case NumChangeLambda_id:
                case num_enviro_obs_id:
                case num_growth_morphs_id:
				case Regime_shift_assess_id:
				case RecDevBack_id:
				case Hsteep_id:
				case Agesel_Pattern_id:
				case AssessFreq_id:
				case CCsel_years_id:
				case Nsex_samp_id:
				case MaxAge_id:
                case femsexratio_id:
                case thresh_mat_id:
				case Nyfuture_id:
				case Nlen_id:
				case Lbin_id:
 				case flagLAdirect_id:
				case flagSLAdirect_id:
				case flagWAdirect_id:
				case SigmaR1_id:
				case SigmaR2_id:
				case SigmaR_future_id:
				case PSigmaR1_id:
				case Regime_year_id:
				case RecDevMinYr_id:
				case RecDevMaxYr_id:
				case RecDevFlag_id:
				case AutoCorRecDev_id:
                case AssessStart_id:
                case NumFisheries_id:
                case NumRegions_id:
                case Nsexes_id:
                case TriggerResponseScen_id:
                case LFSSlim_id:
                case AFSSlim_id:
                case NumSurvey_id:
                case Regime_year_assess_id:
                case NblockPattern_id:
                case SRBlock_id:
                case assRecDevMinYear_id:
                case MultispAssessType_id:
                case mgt_indicator_id:
                case init_mgt_category_id:
                case init_mgt_sp_id:
                case PGMSYBHalpha_id:
                case PGMSYBHbeta_id:
					speciesStr = varStr + strlen(RBCParamStrings[paramID]);
					break;
				default:
					quit("Create_RBC_Species_ParamXML: paramID not found = %d (%s) in switch statement. \n", paramID, RBCSpeciesParamStructArray[index].tag, fileName);
					break;
				}

				//printf("speciesStr = %s\n", speciesStr);
				/* Get the groupCode */
				guild = Util_Get_FG_Index(bm, speciesStr);
				if (guild != -1) {

					guildCheck[guild] = TRUE;

					valuestr = strtok(NULL, seps); /* Get the next token. */
					/* Set the group attribute - if the node does not already exist it will be created */
					Util_XML_Set_Node_Value(ATLANTIS_GROUP_ATTRIBUTE, newNode, FunctGroupArray[guild].groupCode, valuestr);
				}
			}
		}
	}

	/* Check that we read in all of the required values */
	Check_RBC_Species_Param(bm, fileName, index, paramID, guildCheck);
	regfree(&(regBuffer));
	free(guildCheck);
	return newNode;
}

void xmlSaveFormatFileDestFolder(char *destFolder, char *fileName, xmlDocPtr doc, int number){
	char finalFile[BMSLEN];

	sprintf(finalFile, "%s%s", destFolder, fileName);
	xmlSaveFormatFile(finalFile, doc, number);
}

