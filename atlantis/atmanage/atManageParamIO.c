/**
 * 	\ingroup atManageLib
 *	\file atmanageParamIO.c
 *	\brief Functions to load the management parameters from the manage.xml input document.
 *	Created: 12-08-06-2009 Bec Gorton CSIRO 2009
 *
 *	28-10-2009 Bec Gorton
 *	Finished rewriting the management input file parsing code to read in values from the new XML input file.
 *
 *
 *	02-11-2009 Bec Gorton
 *	Fixed a bug in the PrescribedEffortDistribution code.
 *
 */

/*  Open library routines */
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>
#include <ctype.h>
#include <atManage.h>


/**
 *	\brief Create the XML structure to hold the seasonal effort distribution for each fishery.
 */
void ReadPrescribedEffortDistributionXML(MSEBoxModel *bm, char *fileName, xmlNodePtr parent) {

	double *values = 0;
	int fisheryIndex, b, season;
	xmlNodePtr attributeNode, fisheryNode;
	char str[100];
	char errorString[STRLEN];

	attributeNode = Util_XML_Get_Node(ATLANTIS_ATTRIBUTE, parent, "PrescribedEffortDistribution");
	if (attributeNode == NULL)
		quit("Effort/PrescribedEffortDistribution attribute group not found in input file %s.\n", fileName);

	if (verbose)
		printf("Reading PrescribedEffortDistribution values\n");

	/* Read in the data for each functional group.*/
	for (fisheryIndex = 0; fisheryIndex < bm->K_num_fisheries; fisheryIndex++) {

		sprintf(errorString, "Effort/PrescribedEffortDistribution/%s", FisheryArray[fisheryIndex].fisheryCode);
		fisheryNode = Util_XML_Get_Node(ATLANTIS_FISHERY_ATTRIBUTE, attributeNode, FisheryArray[fisheryIndex].fisheryCode);
		if (fisheryNode == NULL)
			quit("%s fishery attribute group not found.\n", errorString);

		if (verbose > 1)
			printf(" FisheryArray[fisheryIndex].fisheryCode = %s\n", FisheryArray[fisheryIndex].fisheryCode);
		for (season = 1; season <= 4; season++) {
			sprintf(str, "season%d", season);
			if (Util_XML_Read_Array_Double(ATLANTIS_TEMPORAL_ATTRIBUTE, fileName, errorString, fisheryNode, proportion_check, str, &values, bm->nbox) == FALSE) {
				quit("Error: Unable to find parameter '%s/%s' in input file %s\n", errorString,
						str, fileName);
			}

			for (b = 0; b < bm->nbox; b++) {
				bm->Effort_hdistrib[b][fisheryIndex][season - 1] = values[b];
			}

			free(values);
		}
	}
}

/**
 * Read integer data into the given array.
 *
 */
void readEffortPenaltyXMLData(MSEBoxModel *bm, char *fileName, xmlNodePtr parent) {
	double *values = 0;
	int fisheryIndex, b;
	xmlNodePtr attributeNode;

	attributeNode = Util_XML_Get_Node(ATLANTIS_ATTRIBUTE, parent, "EffortPenalty");
	if (attributeNode == NULL)
		quit("Effort/EffortPenalty attribute group not found in input file %s.\n", fileName);

	if (verbose)
		printf("Reading EffortPenalty values\n");

	/* Read in the data for each functional group.*/
	for (fisheryIndex = 0; fisheryIndex < bm->K_num_fisheries; fisheryIndex++) {

		if (Util_XML_Read_Array_Double(ATLANTIS_FISHERY_ATTRIBUTE, fileName, "Effort/EffortPenalty", attributeNode, no_checking, FisheryArray[fisheryIndex].fisheryCode, &values,
				bm->nbox) == FALSE) {
			quit("Error: Unable to find parameter 'Effort/EffortPenalty/%s' in input file %s\n", FisheryArray[fisheryIndex].fisheryCode, fileName);
		}

		for (b = 0; b < bm->nbox; b++) {
			bm->EffortPenalty[b][fisheryIndex] = values[b];
		}

		free(values);
	}
}

/**
 * Read integer data into the given array.
 *
 */
void readPortXMLData(MSEBoxModel *bm, char *fileName, xmlNodePtr parent, char *paramName, int paramID) {
	double *values = 0;
	int b;
	char errorString[STRLEN];
	char *nodeName =  Util_Get_Node_Name(parent);

	sprintf(errorString, "%s/%s", nodeName, paramName);

	if (Util_XML_Read_Array_Double(ATLANTIS_ATTRIBUTE, fileName, errorString, parent, no_checking, paramName, &values, bm->K_num_ports) == FALSE) {
		quit("Error: Unable to find parameter '%s' in input file %s\n", errorString, fileName);
	}

	for (b = 0; b < bm->K_num_ports; b++) {
		bm->Port_info[b][paramID] = values[b];
	}
	free(values);
	free(nodeName);

}

void readBiMonthlyXML(MSEBoxModel *bm, char *fileName, xmlNodePtr parent) {
	int guild, b, region;
	xmlNodePtr attributeGroup, speciesNode;
	double *values = 0;
	char str[STRLEN];
	char errorString[STRLEN];
    
    attributeGroup = Util_XML_Get_Node(ATLANTIS_ATTRIBUTE, parent, "BiMonthlyTAC");
	if (attributeGroup == NULL)
		quit("readBiMonthlyXML - %s/BiMonthlyTAC attribute group not found.\n", parent->name);

	for (guild = 0; guild < bm->K_num_tot_sp; guild++) {
		if (FunctGroupArray[guild].isFished == TRUE) {

			sprintf(errorString, "TAC_Parameters/BiMonthlyTAC/%s", FunctGroupArray[guild].groupCode);

			speciesNode = Util_XML_Get_Node(ATLANTIS_GROUP_ATTRIBUTE, attributeGroup, FunctGroupArray[guild].groupCode);
			if (speciesNode == NULL)
				quit("readBiMonthlyXML - %s attribute group not found.\n", errorString);
			for (region = 0; region < bm->K_num_reg; region++) {

				sprintf(str, "region%d", region + 1);

				if (Util_XML_Read_Array_Double(ATLANTIS_ATTRIBUTE, fileName, errorString, speciesNode, proportion_check, str, &values, 6) == FALSE) {
					quit("Error: Unable to find parameter '%s/%s' in input file %s\n", errorString, str, fileName);
				}

				for (b = 0; b < 6; b++) {
					bm->BiTAC_sp[b][region][guild][old_id] = values[b];
				}
				free(values);
			}
		}
	}
}

void readRegionalTACXML(MSEBoxModel *bm, char *fileName, xmlNodePtr parent) {
	int guild, i;
	xmlNodePtr attributeGroup;
	double *values = 0;
	char errorString[STRLEN];
	char *nodeName =  Util_Get_Node_Name(parent);

	sprintf(errorString, "%s/RegionalTAC", nodeName);
	attributeGroup = Util_XML_Get_Node(ATLANTIS_ATTRIBUTE, parent, "RegionalTAC");
	if (attributeGroup == NULL)
		quit("readRegionalTACXML - %s attribute group not found.\n", errorString);

	for (guild = 0; guild < bm->K_num_tot_sp; guild++) {
		if (FunctGroupArray[guild].isFished == TRUE) {
			if (Util_XML_Read_Array_Double(ATLANTIS_GROUP_ATTRIBUTE, fileName, errorString, attributeGroup, proportion_check, FunctGroupArray[guild].groupCode, &values,
					bm->K_num_reg) == FALSE) {
				quit("Error: Unable to find parameter '%s/%s' in input file %s\n", errorString, FunctGroupArray[guild].groupCode, fileName);
			}

			for (i = 0; i < bm->K_num_reg; i++)
				bm->RegionalData[guild][i][reg_catch_id] = values[i];
			free(values);

		}
	}
	free(nodeName);
}

void readBasketTACXML(MSEBoxModel *bm, char *fileName, xmlNodePtr parent) {
	int guild, i;
	xmlNodePtr attributeGroup;
	double *values = 0;
	int counter = 0;
	char errorString[STRLEN];
	char *nodeName =  Util_Get_Node_Name(parent);


	sprintf(errorString, "%s/BasketTAC", nodeName);
	attributeGroup = Util_XML_Get_Node(ATLANTIS_ATTRIBUTE, parent, "BasketTAC");
	if (attributeGroup == NULL)
		quit("BasketTAC - %s attribute group not found.\n", errorString);

	for (guild = 0; guild < bm->K_num_tot_sp; guild++) {
		if (FunctGroupArray[guild].isFished == TRUE) {

			counter = (int) (FunctGroupArray[guild].speciesParams[basket_size_id]);
            if(counter > bm->K_num_basket) {
                quit("The basket size set for %s is set to %d which is greater than K_num_basket (set to %d in run.prm) - please adjust one or the other\n", FunctGroupArray[guild].groupCode, counter, bm->K_num_basket);
            }

			if(counter > 0){
				if (Util_XML_Read_Array_Double(ATLANTIS_GROUP_ATTRIBUTE, fileName, errorString, attributeGroup, integer_check, FunctGroupArray[guild].groupCode, &values,
						bm->K_num_basket) == FALSE) {
					quit("Error: Unable to find parameter '%s/%s' in input file %s\n", errorString, FunctGroupArray[guild].groupCode, fileName);
				}

                for (i = 0; i < counter; i++){
					bm->sp_basket[guild][i] = (int)values[i];
                    
                    printf("sp %d value-%d %d ", guild, i, bm->sp_basket[guild][i]);
                }
                printf("\n");
				free(values);
			}

		}
	}
	free(nodeName);
}

/**
 *
 *
 *
 */
void readCompanionSpeciesXML(MSEBoxModel *bm, char *fileName, xmlNodePtr parent) {
	int guild, i;
	xmlNodePtr attributeGroup;
	double *values = 0;
	char errorString[STRLEN];
	char *nodeName =  Util_Get_Node_Name(parent);

    printf("Doing readCompanionSpeciesXML\n");
    
	sprintf(errorString, "%s/CompanionSpecies", nodeName);

	attributeGroup = Util_XML_Get_Node(ATLANTIS_ATTRIBUTE, parent, "CompanionSpecies");
	if (attributeGroup == NULL)
		quit("readCompanionSpeciesXML - %s attribute group not found.\n", errorString);

	for (guild = 0; guild < bm->K_num_tot_sp; guild++) {
		if (FunctGroupArray[guild].isFished == TRUE) {
			if (Util_XML_Read_Array_Double(ATLANTIS_GROUP_ATTRIBUTE, fileName, errorString, attributeGroup, integer_check, FunctGroupArray[guild].groupCode, &values,
					bm->K_max_co_sp) == FALSE) {
				quit("Error: Unable to find parameter '%s/%s' in input file %s\n", errorString, FunctGroupArray[guild].groupCode, fileName);
			}

			for (i = 0; i < bm->K_max_co_sp; i++) {
				FunctGroupArray[guild].co_sp[i] = (int)values[i];
			}
			free(values);
		}
	}
	free(nodeName);
}

/**
 *    \brief Read the companion catch ratios
 */
static void Read_Fishery_Companion_Catch_XML(MSEBoxModel *bm, char *fileName, xmlNodePtr parent) {
    
    double *values = 0;
    int guild, nf, co_sp;
    xmlNodePtr attributeNode, speciesNode;
    char str[100];
    char errorString[STRLEN];

    if (verbose)
        printf("Reading co_sp_catch values\n");

    attributeNode = Util_XML_Get_Node(ATLANTIS_ATTRIBUTE, parent, "co_sp_catch");
    if (attributeNode == NULL)
        quit("CompanionSpeciesCatch attribute group not found in input file %s.\n", fileName);

    /* Create a node for each functional group*/
    for (guild = 0; guild < bm->K_num_tot_sp; guild++) {
        if (FunctGroupArray[guild].isFished == TRUE) {

            sprintf(errorString, "Companion_Parameters/co_sp_catch/%s", FunctGroupArray[guild].groupCode);
            speciesNode = Util_XML_Get_Node(ATLANTIS_GROUP_ATTRIBUTE, attributeNode, FunctGroupArray[guild].groupCode);
            if (speciesNode == NULL)
                quit("Companion_Parameters/co_sp_catch/%s species attribute group not found.\n", FunctGroupArray[guild].groupCode);

            for (co_sp = 0; co_sp < FunctGroupArray[guild].speciesParams[max_co_sp_id]; co_sp++) {

                sprintf(str, "companion%d", co_sp + 1);
                sprintf(errorString, "co_sp_catch/%s/%s", FunctGroupArray[guild].groupCode, str);

                if (Util_XML_Read_Array_Double(ATLANTIS_COMPANION_ATTRIBUTE, fileName, errorString, speciesNode, no_checking, str, &values, bm->K_num_fisheries) == FALSE) {
                    quit("Error: Unable to find parameter '%s/%s' in input file %s\n", errorString, str, fileName);
                }

                //printf("For %s co_sp %d ", FunctGroupArray[guild].groupCode, co_sp);

                for (nf = 0; nf < bm->K_num_fisheries; nf++) {
                    FunctGroupArray[guild].co_sp_catch[nf][co_sp] = values[nf];
                    
                    //printf(" %e ", values[nf]);
                }
                //printf("\n");
                free(values);
            }
        }
    }
}

/**
 *
 *
 *
 */
void readEffortChangeXML(MSEBoxModel *bm, char *fileName, xmlNodePtr parent, char *paramName, int paramID) {

	double *values = 0;
	int fisheryIndex, b;
	xmlNodePtr attributeNode;
	int numChanges = 0;
	char errorString[STRLEN];
	char *nodeName =  Util_Get_Node_Name(parent);

	sprintf(errorString, "%s/%s", nodeName, paramName);

	attributeNode = Util_XML_Get_Node(ATLANTIS_ATTRIBUTE, parent, paramName);
	if (attributeNode == NULL)
		quit("errorString%s attribute group not found.\n", errorString);

	if (verbose)
		printf("Reading %s values\n", paramName);

	/* Read in the data for each functional group.*/
	for (fisheryIndex = 0; fisheryIndex < bm->K_num_fisheries; fisheryIndex++) {

		numChanges = (int)bm->FISHERYprms[fisheryIndex][EFF_num_changes_id];
		if (numChanges > 0) {
			if (Util_XML_Read_Array_Double(ATLANTIS_FISHERY_ATTRIBUTE, fileName, errorString, attributeNode, no_checking, FisheryArray[fisheryIndex].fisheryCode, &values,
					numChanges) == FALSE) {
				quit("Error: Unable to find parameter '%s/%s' in input file %s\n", errorString, FisheryArray[fisheryIndex].fisheryCode, fileName);
			}

			for (b = 0; b < numChanges; b++)
				EFFORTchange[fisheryIndex][b][paramID] = values[b];

			free(values);
		}
	}
	free(nodeName);
}

/**
 *
 *	Read in the endangeredMPA values.
 *	These are automatically stored in the bm->MPAEndangeredlist array.
 *
 */
void readCellMPAStatusXML(MSEBoxModel *bm, char *fileName, xmlNodePtr parent) {

	double *values = 0;
	int fisheryIndex, b;
	xmlNodePtr attributeNode;
	int flagF, index;
	char errorString[STRLEN];
	char *nodeName =  Util_Get_Node_Name(parent);

	sprintf(errorString, "%s/CellMPAStatus", nodeName);

	attributeNode = Util_XML_Get_Node(ATLANTIS_ATTRIBUTE, parent, "CellMPAStatus");
	if (attributeNode == NULL)
		quit("%s attribute group not found in input file %s.\n", errorString, fileName);

	if (verbose)
		printf("Reading CellMPAStatus values\n");

	/* Read in the data for each functional group.*/
	for (fisheryIndex = 0; fisheryIndex < bm->K_num_fisheries; fisheryIndex++) {

		flagF = 0;
		for (index = 0; index < bm->K_num_tot_sp; index++) {
			if (FunctGroupArray[index].isImpacted == TRUE) {
				flagF += (int) (bm->SP_FISHERYprms[index][fisheryIndex][flagF_id]);
			}
		}
		if (flagF > 0) {
			if (Util_XML_Read_Array_Double(ATLANTIS_FISHERY_ATTRIBUTE, fileName, errorString, attributeNode, no_checking, FisheryArray[fisheryIndex].fisheryCode, &values,
					bm->nbox) == FALSE) {
				quit("Error: Unable to find parameter '%s/%s' in input file %s\n", errorString, FisheryArray[fisheryIndex].fisheryCode, fileName);
			}
		} else {
			if (Util_XML_Read_Array_Double(ATLANTIS_FISHERY_ATTRIBUTE, fileName, errorString, attributeNode, proportion_check, FisheryArray[fisheryIndex].fisheryCode,
					&values, bm->nbox) == FALSE) {
				quit("Error: Unable to find parameter '%s/%s' in input file %s\n", errorString, FisheryArray[fisheryIndex].fisheryCode, fileName);
			}
		}
		for (b = 0; b < bm->nbox; b++) {
			/* Zoning for each box per fishery */
			bm->MPA[b][fisheryIndex] = values[b];
			if (values[b] > 1.0)
				warn("MPA value for %s in box %d > 1.0 (this is ok if *fishing the line*, but otherwise may want to reset <= 1.0)\n", Util_Get_Fishery_Name(bm,
						fisheryIndex), b);
		}
		free(values);
	}
	free(nodeName);
}
/**
 *
 *	Read in the endangeredMPA values.
 *	These are automatically stored in the bm->MPAEndangeredlist array.
 *
 */
void readMPAEndangeredXML(MSEBoxModel *bm, char *fileName, xmlNodePtr parent) {

	double *values = 0;
	int fisheryIndex, b, thisday;
	xmlNodePtr attributeNode;
	char errorString[STRLEN];
	char *nodeName =  Util_Get_Node_Name(parent);

	sprintf(errorString, "%s/MPAEndangered", nodeName);


	attributeNode = Util_XML_Get_Node(ATLANTIS_ATTRIBUTE, parent, "MPAEndangered");
	if (attributeNode == NULL)
		quit("%s attribute group not found in input file %s.\n", errorString, fileName);

	if (verbose)
		printf("Reading MPAEndangered values\n");

	/* Read in the data for each functional group.*/
	for (fisheryIndex = 0; fisheryIndex < bm->K_num_fisheries; fisheryIndex++) {

		if(Util_XML_Read_Array_Double(ATLANTIS_FISHERY_ATTRIBUTE, fileName, errorString, attributeNode, no_checking, FisheryArray[fisheryIndex].fisheryCode, &values, bm->nbox) == FALSE){
			quit("Error: Unable to find parameter '%s/%s' in input file %s\n", errorString, FisheryArray[fisheryIndex].fisheryCode, fileName);
		}

		for (b = 0; b < bm->nbox; b++)
			for (thisday = 0; thisday < 365; thisday++)
				MPAendangeredlist[thisday][b][fisheryIndex] = values[b];

		free(values);

	}
	free(nodeName);
}

/* Zoning for each box per fishery when	trip limit trigger to protect
 overfished groups is set off */
void readMPAOverFishedXML(MSEBoxModel *bm, char *fileName, xmlNodePtr parent) {

	double *values = 0;
	int speciesIndex, b;
	xmlNodePtr attributeNode;
	char errorString[STRLEN];
	char *nodeName =  Util_Get_Node_Name(parent);

	sprintf(errorString, "%s/MPAOverFished", nodeName);

	attributeNode = Util_XML_Get_Node(ATLANTIS_ATTRIBUTE, parent, "MPAOverFished");
	if (attributeNode == NULL)
		quit("%s attribute group not found in input file %s.\n", errorString, fileName);

	if (verbose)
		printf("Reading MPAOverFished values\n");

	/* Read in the data for each functional group.*/
	for (speciesIndex = 0; speciesIndex < bm->K_num_tot_sp; speciesIndex++) {

		if (FunctGroupArray[speciesIndex].speciesParams[flag_id] && FunctGroupArray[speciesIndex].isImpacted == TRUE) {
			if(Util_XML_Read_Array_Double(ATLANTIS_GROUP_ATTRIBUTE, fileName, errorString, attributeNode, proportion_check, FunctGroupArray[speciesIndex].groupCode, &values,
					bm->nbox) == FALSE){
				quit("Error: Unable to find parameter '%s/%s' in input file %s\n", errorString, FunctGroupArray[speciesIndex].groupCode, fileName);
			}

			for (b = 0; b < bm->nbox; b++)
				MPAoverfishedlist[b][speciesIndex] = values[b];

			free(values);
		}

	}
	free(nodeName);
}

/* Zoning for each box per fishery when	trip limit trigger to protect
 qoverfished groups is set off */
void readPopulationChangeXML(MSEBoxModel *bm, char *fileName, xmlNodePtr parent, char *paramName, int paramID) {

	double *values = 0;
	int counter, b;
	xmlNodePtr attributeNode;
	int index;
	char str[STRLEN];
	char errorString[STRLEN];
	char *nodeName =  Util_Get_Node_Name(parent);

	sprintf(errorString, "%s/%s", nodeName, paramName);

	attributeNode = Util_XML_Get_Node(ATLANTIS_ATTRIBUTE, parent, paramName);
	if (attributeNode == NULL)
		quit("%s attribute group not found.\n", errorString);

	if (verbose)
		printf("Reading %s values\n", paramName);

	/* Read in the data for each functional group.*/
	for (index = 0; index < bm->K_num_ports; index++) {

		counter = (int) bm->Port_info[index][POP_num_changes_id];

		sprintf(str, "port%d", index + 1);
		if(Util_XML_Read_Array_Double(ATLANTIS_ATTRIBUTE, fileName, errorString, attributeNode, no_checking, str, &values, counter) == FALSE){
			quit("Error: Unable to find parameter '%s/%s' in input file %s\n", errorString, str, fileName);
		}
		for (b = 0; b < counter; b++)
			bm->POPchange[index][b][paramID] = values[b];

		free(values);
	}
	free(nodeName);
}

void readManamentFlagTimeXML(MSEBoxModel *bm, char *fileName, xmlNodePtr rootnode) {

	xmlNodePtr attributeGroupNode;
	int i, do_fish_prm;

	attributeGroupNode = Util_XML_Get_Node(ATLANTIS_ATTRIBUTE_SUB_GROUP, rootnode, "Management_Flags");
	if (attributeGroupNode == NULL)
		quit("readManamentFlagTimeXML: Management_Flags attribute group not found in input file %s.\n", fileName);

	/* Flags and switches */
	/* Those determining set-up and recording */
	printf("Fisheries parameter file loading\n");
	printf("0                                 100\n");
	printf("***");
	fflush(stdout);

	//	/bm->dynanyway = (int) Util_XML_Read_Value(fileName, ATLANTIS_ATTRIBUTE,  bm->ecotest, 1, attributeGroupNode, integer_check, "dynanyway");


	/* General harvest related flags */
	bm->flagincidmort = (int) (Util_XML_Read_Value(fileName, ATLANTIS_ATTRIBUTE,  bm->ecotest, 1, attributeGroupNode, binary_check, "flagincidmort"));
	bm->flaghighgrading = (int) (Util_XML_Read_Value(fileName, ATLANTIS_ATTRIBUTE,  bm->ecotest, 1, attributeGroupNode, binary_check, "flaghighgrading"));
	bm->flagmarketdiscard = (int) (Util_XML_Read_Value(fileName, ATLANTIS_ATTRIBUTE,  bm->ecotest, 1, attributeGroupNode, binary_check, "flagmarketdiscard"));
	bm->UpdateMaxSalePrice = (int) (Util_XML_Read_Value(fileName, ATLANTIS_ATTRIBUTE,  bm->ecotest, 1, attributeGroupNode, binary_check, "UpdateMaxSalePrice"));

    bm->flagkeepZeroCatchTS = (int) (Util_XML_Read_Value(fileName, ATLANTIS_ATTRIBUTE,  bm->ecotest, 1, attributeGroupNode, binary_check, "flagkeepZeroCatchTS"));
    
	/* General management related flags */
	bm->flagendangered = (int) (Util_XML_Read_Value(fileName, ATLANTIS_ATTRIBUTE,  bm->ecotest, 1, attributeGroupNode, binary_check, "flagendangered"));
	bm->flagmpa = (int) (Util_XML_Read_Value(fileName, ATLANTIS_ATTRIBUTE,  bm->ecotest, 1, attributeGroupNode, binary_check, "flagmpa"));
	bm->flaginfringe = (int) (Util_XML_Read_Value(fileName, ATLANTIS_ATTRIBUTE,  bm->ecotest, 1, attributeGroupNode, binary_check, "flaginfringe"));
	bm->flagdisplace = (int) (Util_XML_Read_Value(fileName, ATLANTIS_ATTRIBUTE,  bm->ecotest, 1, attributeGroupNode, binary_check, "flagdisplace"));

	bm->flagreinitpop = (int) (Util_XML_Read_Value(fileName, ATLANTIS_ATTRIBUTE,  bm->ecotest, 1, attributeGroupNode, binary_check, "flagreinitpop"));
	bm->pseudo_assess = (int) (Util_XML_Read_Value(fileName, ATLANTIS_ATTRIBUTE,  bm->ecotest, 1, attributeGroupNode, binary_check, "pseudo_assess"));
    bm->do_sumB_HCR = (int) (Util_XML_Read_Value(fileName, ATLANTIS_ATTRIBUTE,  bm->ecotest, 1, attributeGroupNode, integer_check, "do_sumB_HCR"));
    bm->flagSSBforHCR = (int) (Util_XML_Read_Value(fileName, ATLANTIS_ATTRIBUTE,  bm->ecotest, 1, attributeGroupNode, binary_check, "flagSSBforHCR"));

	/* Scenario painting switches */
	bm->flagbuffereffort = (int) (Util_XML_Read_Value(fileName, ATLANTIS_ATTRIBUTE,  bm->ecotest, 1, attributeGroupNode, binary_check, "flagbuffereffort"));
	bm->flagchangeeffort = (int) (Util_XML_Read_Value(fileName, ATLANTIS_ATTRIBUTE,  bm->ecotest, 1, attributeGroupNode, binary_check, "flagchangeeffort"));
	bm->flagchangecap = (int) (Util_XML_Read_Value(fileName, ATLANTIS_ATTRIBUTE,  bm->ecotest, 1, attributeGroupNode, binary_check, "flagchangecap"));
	bm->flagnewport = (int) (Util_XML_Read_Value(fileName, ATLANTIS_ATTRIBUTE,  bm->ecotest, 1, attributeGroupNode, binary_check, "flagnewport"));
	bm->flagchangepop = (int) (Util_XML_Read_Value(fileName, ATLANTIS_ATTRIBUTE,  bm->ecotest, 1, attributeGroupNode, binary_check, "flagchangepop"));
	bm->flag_sel_with_mFC = (int) (Util_XML_Read_Value(fileName, ATLANTIS_ATTRIBUTE,  bm->ecotest, 1, attributeGroupNode, binary_check, "flag_sel_with_mFC"));
    bm->flag_stop_F_tac = (int) (Util_XML_Read_Value(fileName, ATLANTIS_ATTRIBUTE,  bm->ecotest, 1, attributeGroupNode, binary_check, "flag_stop_F_tac"));

    /** Ecosystem cacth cap parameters */
    bm->maxF_aggregate = (int) (Util_XML_Read_Value(fileName, ATLANTIS_ATTRIBUTE,  bm->ecotest, 1, attributeGroupNode, binary_check, "maxF_aggregate"));
    bm->use_time_avg_biom = (int) (Util_XML_Read_Value(fileName, ATLANTIS_ATTRIBUTE,  bm->ecotest, 1, attributeGroupNode, binary_check, "use_time_avg_biom"));
    bm->use_time_avg_wgt = (int) (Util_XML_Read_Value(fileName, ATLANTIS_ATTRIBUTE,  bm->ecotest, 1, attributeGroupNode, binary_check, "use_time_avg_wgt"));
    bm->syst_cap_calc_method = (int) (Util_XML_Read_Value(fileName, ATLANTIS_ATTRIBUTE,  bm->ecotest, 1, attributeGroupNode, integer_check, "syst_cap_calc_method"));
    bm->M_est_method = (int) (Util_XML_Read_Value(fileName, ATLANTIS_ATTRIBUTE,  bm->ecotest, 1, attributeGroupNode, integer_check, "M_est_method"));
    bm->Ecosystm_Cap_tonnes = (Util_XML_Read_Value(fileName, ATLANTIS_ATTRIBUTE,  bm->ecotest, 1, attributeGroupNode, no_checking, "Ecosystm_Cap_tonnes"));
    bm->K_cap_rolling_period = (Util_XML_Read_Value(fileName, ATLANTIS_ATTRIBUTE,  bm->ecotest, 1, attributeGroupNode, no_checking, "K_cap_rolling_period"));
    bm->sp_pref_inv_norm_done = 0;

	/** Fisheries flags **/
	bm->DynDAS = (int) (Util_XML_Read_Value(fileName, ATLANTIS_ATTRIBUTE,  bm->ecotest, 1, attributeGroupNode, binary_check, "dynDAS"));

	/* Set flagfinfish and check it matches the flagfish settings read in from the harvest input file. */
	bm->flagfinfish = (int) (Util_XML_Read_Value(fileName, ATLANTIS_ATTRIBUTE,  bm->ecotest, 1, attributeGroupNode, binary_check, "flagfinfish"));
    bm->flagStoreShotCPUE = (int)(Util_XML_Read_Value(fileName, ATLANTIS_ATTRIBUTE,  bm->ecotest, 1, attributeGroupNode, binary_check, "flagStoreShotCPUE"));
    bm->flagStoreCPUE = (int)(Util_XML_Read_Value(fileName, ATLANTIS_ATTRIBUTE,  bm->ecotest, 1, attributeGroupNode, binary_check, "flagStoreCPUE"));
    
	do_fish_prm = 0;
	bm->flag_fisheries_on = 0;

	/* Check if the vertebrates are being fished */
	for (i = 0; i < bm->K_num_tot_sp; i++) {
		if (FunctGroupArray[i].isVertebrate == TRUE) {
			if (FunctGroupArray[i].speciesParams[flagfish_id]) {
				do_fish_prm = 1;
				bm->flag_fisheries_on = 1;
			}
		}
	}

	if (do_fish_prm != bm->flagfinfish){
		printf("do_fish_prm = %d, bm->flagfinfish = %d\n", do_fish_prm, bm->flagfinfish);
		quit("flagfinfish and flagfish mismatch. If fishing any vertebrates (flagfish set to 1 above) then set flagfinfish = 1 otherwise set = 0\n");

	}
	/* Check if the invertebrates are being fished */
	for (i = 0; i < bm->K_num_tot_sp; i++) {
		if (FunctGroupArray[i].isVertebrate == FALSE) {
			if (FunctGroupArray[i].speciesParams[flagfish_id]) {
				bm->flag_fisheries_on = 1;
			}
		}
	}

	/** Management flags **/
	bm->flagTACincludeDiscard = (int) (Util_XML_Read_Value(fileName, ATLANTIS_ATTRIBUTE,  bm->ecotest, 1, attributeGroupNode, binary_check, "flagTACincludeDiscard"));
	bm->flagdyn_coupdate = (int) (Util_XML_Read_Value(fileName, ATLANTIS_ATTRIBUTE,  bm->ecotest, 1, attributeGroupNode, binary_check, "flagdyn_coupdate"));
	bm->flagTradeTACvsMPA = (int) (Util_XML_Read_Value(fileName, ATLANTIS_ATTRIBUTE,  bm->ecotest, 1, attributeGroupNode, binary_check, "flagTradeTACvsMPA"));
	bm->flagQuotaBound = (int) (Util_XML_Read_Value(fileName, ATLANTIS_ATTRIBUTE,  bm->ecotest, 1, attributeGroupNode, integer_check, "flagQuotaBound"));

	Util_XML_Read_Fishery_Param(bm, fileName, attributeGroupNode, flagmanage_id);
	Util_XML_Read_Fishery_Param(bm, fileName, attributeGroupNode, flagTACpartipcate_id);
	Util_XML_Read_Fishery_Param(bm, fileName, attributeGroupNode, flagcap_id);
	Util_XML_Read_Fishery_Param(bm, fileName, attributeGroupNode, flagcap_peak_id);
	Util_XML_Read_Fishery_Param(bm, fileName, attributeGroupNode, flaguseall_id);
	Util_XML_Read_Fishery_Param(bm, fileName, attributeGroupNode, flagseasonal_id);
    Util_XML_Read_Fishery_Param(bm, fileName, attributeGroupNode, flag_framebased_id);
	Util_XML_Read_Fishery_Param(bm, fileName, attributeGroupNode, flagmpa_id);

	bm->useRBCTiers = (int)(Util_XML_Read_Value(fileName, ATLANTIS_ATTRIBUTE,  bm->ecotest, 1, attributeGroupNode, binary_check, "useRBCTiers"));
    bm->do_assessing = 1;
    
    if(bm->useRBCTiers) {
        bm->useMultispAssess = (int)(Util_XML_Read_Value(fileName, ATLANTIS_ATTRIBUTE,  bm->ecotest, 1, attributeGroupNode, binary_check, "useMultispAssess"));
    }
    
    bm->usingRedus_R_HCR = (int)(Util_XML_Read_Value(fileName, ATLANTIS_ATTRIBUTE,  bm->ecotest, 1, attributeGroupNode, binary_check, "usingRedus_R_HCR"));
    
    /* How handling MPAs */
    bm->flagSimpleStartStopMPAs = (int)(Util_XML_Read_Value(fileName, ATLANTIS_ATTRIBUTE,  bm->ecotest, 1, attributeGroupNode, binary_check, "flagSimpleStartStopMPAs"));
    
    if (bm->flagSimpleStartStopMPAs) {
        bm->MPAstartyr = (int)(Util_XML_Read_Value(fileName, ATLANTIS_ATTRIBUTE,  bm->ecotest, 1, attributeGroupNode, integer_check, "All_MPAstartyr"));
        bm->MPAendyr = (int)(Util_XML_Read_Value(fileName, ATLANTIS_ATTRIBUTE,  bm->ecotest, 1, attributeGroupNode, integer_check, "All_MPAendyr"));
    }
    
    
    /* Load shot level CPUE params if necessary */
    if(bm->flagStoreShotCPUE) {
        bm->flagfullCPUEreport = (int)(Util_XML_Read_Value(fileName, ATLANTIS_ATTRIBUTE,  bm->ecotest, 1, attributeGroupNode, binary_check, "flagfullCPUEreport"));

        bm->PerShotCDFtype = (int)(Util_XML_Read_Value(fileName, ATLANTIS_ATTRIBUTE,  bm->ecotest, 1, attributeGroupNode, integer_check, "PerShotCDFtype"));
        bm->CPUEshotfdis = (int)(Util_XML_Read_Value(fileName, ATLANTIS_ATTRIBUTE,  bm->ecotest, 1, attributeGroupNode, integer_check, "CPUEshotfdis"));
        bm->CPUEshotavgdis = Util_XML_Read_Value(fileName, ATLANTIS_ATTRIBUTE,  bm->ecotest, 1, attributeGroupNode, no_checking, "CPUEshotfdis");
        bm->CPUEshotvartdis = Util_XML_Read_Value(fileName, ATLANTIS_ATTRIBUTE,  bm->ecotest, 1, attributeGroupNode, no_checking, "CPUEshotvartdis");
    
        bm->CPUEshotfeff = (int)(Util_XML_Read_Value(fileName, ATLANTIS_ATTRIBUTE,  bm->ecotest, 1, attributeGroupNode, integer_check, "CPUEshotfeff"));
        bm->CPUEshotavgeff = Util_XML_Read_Value(fileName, ATLANTIS_ATTRIBUTE,  bm->ecotest, 1, attributeGroupNode, no_checking, "CPUEshotavgeff");
        bm->CPUEshotvareff = Util_XML_Read_Value(fileName, ATLANTIS_ATTRIBUTE,  bm->ecotest, 1, attributeGroupNode, no_checking, "CPUEshotvareff");
    
        bm->gun_flexthresh = Util_XML_Read_Value(fileName, ATLANTIS_ATTRIBUTE,  bm->ecotest, 1, attributeGroupNode, no_checking, "gun_flexthresh");
        bm->K_min_shotlength = Util_XML_Read_Value(fileName, ATLANTIS_ATTRIBUTE,  bm->ecotest, 1, attributeGroupNode, no_checking, "K_min_shotlength");
    
        bm->K_num_catchbin = (int)(Util_XML_Read_Value(fileName, ATLANTIS_ATTRIBUTE,  bm->ecotest, 1, attributeGroupNode, integer_check, "K_num_catchbin"));
        bm->size_catchbin = Util_XML_Read_Value(fileName, ATLANTIS_ATTRIBUTE,  bm->ecotest, 1, attributeGroupNode, no_checking, "size_catchbin");
        bm->min_effort_coefft = Util_XML_Read_Value(fileName, ATLANTIS_ATTRIBUTE,  bm->ecotest, 1, attributeGroupNode, no_checking, "min_effort_coefft");
        bm->min_effort_const = Util_XML_Read_Value(fileName, ATLANTIS_ATTRIBUTE,  bm->ecotest, 1, attributeGroupNode, no_checking, "min_effort_const");
    
        Util_XML_Read_Impacted_Group_Param(bm, fileName, attributeGroupNode, flagPerShotCPUE_id);
        
        Util_XML_Read_Species_Param(bm, fileName, attributeGroupNode, cpue_cdf_poor_r_id);
        Util_XML_Read_Species_Param(bm, fileName, attributeGroupNode, cpue_cdf_poor_p_id);
        Util_XML_Read_Species_Param(bm, fileName, attributeGroupNode, cpue_cdf_top_r_id);
        Util_XML_Read_Species_Param(bm, fileName, attributeGroupNode, cpue_cdf_top_p_id);

        

    }
    
    if(bm->flagStoreCPUE) {
        Util_XML_Read_Impacted_Group_Param(bm, fileName, attributeGroupNode, flagRecordCPUE_id);
    }

}

void readManagementSenarioFlagsXML(MSEBoxModel *bm, char *fileName, xmlNodePtr rootnode) {

	xmlNodePtr groupingNode;
	groupingNode = Util_XML_Get_Node(ATLANTIS_ATTRIBUTE_SUB_GROUP, rootnode, "ScenarioFlags");
	if (groupingNode == NULL)
		quit("readManagementSenarioFlagsXML: ScenarioFlags attribute group not found in input file %s.\n", fileName);

	Util_XML_Read_Fishery_Param(bm, fileName, groupingNode, flagchangeEFF_id);
	Util_XML_Read_Fishery_Param(bm, fileName, groupingNode, flagchangeseason_id);
	Util_XML_Read_Fishery_Param(bm, fileName, groupingNode, TACchange_id);
}

void readManagementMSYXML(MSEBoxModel *bm, char *fileName, xmlNodePtr rootnode) {

	xmlNodePtr groupingNode;
	groupingNode = Util_XML_Get_Node(ATLANTIS_ATTRIBUTE_SUB_GROUP, rootnode, "MSYParameters");
	if (groupingNode == NULL)
		quit("readManagementMSYXML: MSYParameters attribute group not found in input file %s.\n", fileName);

	if(Util_XML_Read_Array_Double(ATLANTIS_ATTRIBUTE, fileName, "MSYParameters", groupingNode, integer_check, "UseLever", &LeverUsei, bm->K_num_tot_sp) == FALSE){
		quit("Error: Unable to find parameter 'MSYParameters/UseLever' in input file %s\n",  fileName);
	}

	Util_XML_Read_Fishery_Param(bm, fileName, groupingNode, use_min_lever_id);
}

void readManagementEffortXML(MSEBoxModel *bm, char *fileName, xmlNodePtr rootnode) {

	xmlNodePtr groupingNode, childGroupingNode;
	groupingNode = Util_XML_Get_Node(ATLANTIS_ATTRIBUTE_SUB_GROUP, rootnode, "Effort");
	if (groupingNode == NULL)
		quit("readManagementEffortXML: Effort attribute group not found in input file %s.\n", fileName);

	Util_XML_Read_Fishery_Param_Double(bm, fileName, groupingNode, "PerscribedEffort", no_checking, mEff, 4);

	Util_XML_Read_Fishery_Param(bm, fileName, groupingNode, mEff_max_id);
	Util_XML_Read_Fishery_Param(bm, fileName, groupingNode, mEff_a_id);
	Util_XML_Read_Fishery_Param(bm, fileName, groupingNode, mEff_offset_id);
	Util_XML_Read_Fishery_Param(bm, fileName, groupingNode, mEff_testfish_id);

	Util_XML_Read_Fishery_Param(bm, fileName, groupingNode, mEff_thresh_id);
	Util_XML_Read_Fishery_Param(bm, fileName, groupingNode, mEff_thresh_top_id);
	Util_XML_Read_Fishery_Param(bm, fileName, groupingNode, mEff_shift_id);
	Util_XML_Read_Fishery_Param(bm, fileName, groupingNode, mFCscale_id);
	Util_XML_Read_Fishery_Param(bm, fileName, groupingNode, EffortLevel_id);

	ReadPrescribedEffortDistributionXML(bm, fileName, groupingNode);
	Util_XML_Read_Fishery_Param_Double(bm, fileName, groupingNode, "VerticalEffortDistribution", no_checking, origEffort_vdistrib, bm->wcnz);

	/* Read in the new EffortPenalty values */
	readEffortPenaltyXMLData(bm, fileName, groupingNode);

	childGroupingNode = Util_XML_Get_Node(ATLANTIS_ATTRIBUTE_SUB_GROUP, rootnode, "CPUE_Thresholds");
	if (childGroupingNode == NULL)
		quit("readManagementEffortXML: EPUE_Thresholds attribute group not found in input file %s.\n", fileName);

	Util_XML_Read_Fishery_Param(bm, fileName, childGroupingNode, CPUE_effort_thresh_id);
	Util_XML_Read_Fishery_Param(bm, fileName, childGroupingNode, CPUE_effort_scale_id);
	Util_XML_Read_Fishery_Param(bm, fileName, childGroupingNode, cap_id);

	//TODO: Check why this is not loaded in the original code.
	bm->explore_thresh_effort = Util_XML_Read_Value(fileName, ATLANTIS_ATTRIBUTE,  bm->ecotest, 1, childGroupingNode, no_checking, "explore_thresh_effort");

}

void readPortIndexXML(MSEBoxModel *bm, char *fileName, xmlNodePtr rootnode) {

	xmlNodePtr groupingNode, childGroupingNode;
	groupingNode = Util_XML_Get_Node(ATLANTIS_ATTRIBUTE_SUB_GROUP, rootnode, "Port_Information");
	if (groupingNode == NULL)
		quit("readPortIndexXML: Port_Information attribute group not found in input file %s.\n", fileName);

	Util_XML_Read_Fishery_Param_Int(bm, fileName, groupingNode, "FisheryPort", no_checking, bm->Port_Fishery, bm->K_num_ports);

	childGroupingNode = Util_XML_Get_Node(ATLANTIS_ATTRIBUTE_SUB_GROUP, rootnode, "Location_And_Timing");

	readPortXMLData(bm, fileName, childGroupingNode, "ports_x", portx_id);
	readPortXMLData(bm, fileName, childGroupingNode, "ports_y", porty_id);
	readPortXMLData(bm, fileName, childGroupingNode, "ports_start", port_start_id);
	readPortXMLData(bm, fileName, childGroupingNode, "ports_end", port_end_id);
	readPortXMLData(bm, fileName, childGroupingNode, "ports_pop", population_id);
	readPortXMLData(bm, fileName, childGroupingNode, "ports_markets", prime_market_id);

	k_proprecfish = Util_XML_Read_Value(fileName, ATLANTIS_ATTRIBUTE,  bm->ecotest, 1, childGroupingNode, no_checking, "k_proprecfish");
}

/* Create the TAC xml ndoes */
void readTACXML(MSEBoxModel *bm, char *fileName, xmlNodePtr rootnode) {

	int i, sp, bulk_possible = 0;
	xmlNodePtr groupingNode, childGroupingNode;

	groupingNode = Util_XML_Get_Node(ATLANTIS_ATTRIBUTE_SUB_GROUP, rootnode, "TAC_Parameters");
	if (groupingNode == NULL)
		quit("readTACXML: TAC_Parameters attribute group not found in input file %s.\n", fileName);

	childGroupingNode = Util_XML_Get_Node(ATLANTIS_ATTRIBUTE_SUB_GROUP, rootnode, "Reference_Points");
	if (childGroupingNode == NULL)
		quit("readTACXML: Reference_Points attribute group not found in input file %s.\n", fileName);

	/** Management parameters **/
	/* General management reference points */
	bm->targ_refA = Util_XML_Read_Value(fileName, ATLANTIS_ATTRIBUTE,  bm->ecotest, 1, childGroupingNode, proportion_check, "targ_refA");
	bm->targ_refB = Util_XML_Read_Value(fileName, ATLANTIS_ATTRIBUTE,  bm->ecotest, 1, childGroupingNode, proportion_check, "targ_refB");
	bm->targ_refC = Util_XML_Read_Value(fileName, ATLANTIS_ATTRIBUTE,  bm->ecotest, 1, childGroupingNode, proportion_check, "targ_refC");
	bm->targ_refD = Util_XML_Read_Value(fileName, ATLANTIS_ATTRIBUTE,  bm->ecotest, 1, childGroupingNode, proportion_check, "targ_refD");
	bm->targ_refE = Util_XML_Read_Value(fileName, ATLANTIS_ATTRIBUTE,  bm->ecotest, 1, childGroupingNode, proportion_check, "targ_refE");
	bm->lim_ref = Util_XML_Read_Value(fileName, ATLANTIS_ATTRIBUTE,  bm->ecotest, 1, childGroupingNode, proportion_check, "lim_ref");
	bm->forage_refA = Util_XML_Read_Value(fileName, ATLANTIS_ATTRIBUTE,  bm->ecotest, 1, childGroupingNode, proportion_check, "forage_refA");
	bm->forage_refB = Util_XML_Read_Value(fileName, ATLANTIS_ATTRIBUTE,  bm->ecotest, 1, childGroupingNode, proportion_check, "forage_refB");
	bm->forage_refC = Util_XML_Read_Value(fileName, ATLANTIS_ATTRIBUTE,  bm->ecotest, 1, childGroupingNode, proportion_check, "forage_refC");
	bm->forage_refD = Util_XML_Read_Value(fileName, ATLANTIS_ATTRIBUTE,  bm->ecotest, 1, childGroupingNode, proportion_check, "forage_refD");
	bm->forage_refE = Util_XML_Read_Value(fileName, ATLANTIS_ATTRIBUTE,  bm->ecotest, 1, childGroupingNode, proportion_check, "forage_refE");
	bm->forage_lim_ref = Util_XML_Read_Value(fileName, ATLANTIS_ATTRIBUTE,  bm->ecotest, 1, childGroupingNode, proportion_check, "forage_lim_ref");
    bm->byproduct_refA = Util_XML_Read_Value(fileName, ATLANTIS_ATTRIBUTE,  bm->ecotest, 1, childGroupingNode, proportion_check, "byproduct_refA");
    bm->byproduct_refB = Util_XML_Read_Value(fileName, ATLANTIS_ATTRIBUTE,  bm->ecotest, 1, childGroupingNode, proportion_check, "byproduct_refB");
    bm->byproduct_refC = Util_XML_Read_Value(fileName, ATLANTIS_ATTRIBUTE,  bm->ecotest, 1, childGroupingNode, proportion_check, "byproduct_refC");
    bm->byproduct_refD = Util_XML_Read_Value(fileName, ATLANTIS_ATTRIBUTE,  bm->ecotest, 1, childGroupingNode, proportion_check, "byproduct_refD");
    bm->byproduct_lim_ref = Util_XML_Read_Value(fileName, ATLANTIS_ATTRIBUTE,  bm->ecotest, 1, childGroupingNode, proportion_check, "byproduct_lim_ref");
    bm->bycatch_refA = Util_XML_Read_Value(fileName, ATLANTIS_ATTRIBUTE,  bm->ecotest, 1, childGroupingNode, proportion_check, "bycatch_refA");
    bm->bycatch_refB = Util_XML_Read_Value(fileName, ATLANTIS_ATTRIBUTE,  bm->ecotest, 1, childGroupingNode, proportion_check, "bycatch_refB");
    bm->bycatch_refC = Util_XML_Read_Value(fileName, ATLANTIS_ATTRIBUTE,  bm->ecotest, 1, childGroupingNode, proportion_check, "bycatch_refC");
    bm->bycatch_refD = Util_XML_Read_Value(fileName, ATLANTIS_ATTRIBUTE,  bm->ecotest, 1, childGroupingNode, proportion_check, "bycatch_refD");
    bm->bycatch_lim_ref = Util_XML_Read_Value(fileName, ATLANTIS_ATTRIBUTE,  bm->ecotest, 1, childGroupingNode, proportion_check, "bycatch_lim_ref");
    
    bm->discardTAC = Util_XML_Read_Value(fileName, ATLANTIS_ATTRIBUTE,  bm->ecotest, 1, childGroupingNode, proportion_check, "discardTAC");

	if(Util_XML_Read_Array_Double(ATLANTIS_ATTRIBUTE, fileName, "TAC_Parameters/Reference_Points", childGroupingNode, integer_check, "whichref", &whichrefi, bm->K_num_tot_sp) == FALSE){
		quit("Error: Unable to find parameter 'TAC_Parameters/Reference_Points/whichref' in input file %s\n",  fileName);
	}

	bm->TAC_multiyr = Util_XML_Read_Value(fileName, ATLANTIS_ATTRIBUTE,  bm->ecotest, 1, childGroupingNode, no_checking, "TAC_multiyr");
	bm->critCPUE = Util_XML_Read_Value(fileName, ATLANTIS_ATTRIBUTE,  bm->ecotest, 1, childGroupingNode, no_checking, "critCPUE");
	bm->TACbound = Util_XML_Read_Value(fileName, ATLANTIS_ATTRIBUTE,  bm->ecotest, 1, childGroupingNode, no_checking, "TACbound");
	bm->prop_incTAC = Util_XML_Read_Value(fileName, ATLANTIS_ATTRIBUTE,  bm->ecotest, 1, childGroupingNode, no_checking, "prop_incTAC");
	bm->prop_decTAC = Util_XML_Read_Value(fileName, ATLANTIS_ATTRIBUTE,  bm->ecotest, 1, childGroupingNode, no_checking, "prop_decTAC");
	bm->RBCalpha = Util_XML_Read_Value(fileName, ATLANTIS_ATTRIBUTE,  bm->ecotest, 1, childGroupingNode, no_checking, "RBCalpha");
	bm->TAC_crit_change = Util_XML_Read_Value(fileName, ATLANTIS_ATTRIBUTE,  bm->ecotest, 1, childGroupingNode, no_checking, "TAC_crit_change");

	if(Util_XML_Read_Array_Double(ATLANTIS_ATTRIBUTE, fileName, "TAC_Parameters/Reference_Points",childGroupingNode, no_checking, "estBo", &bm->estBo, bm->K_num_tot_sp) == FALSE){
		quit("Error: Unable to find parameter 'TAC_Parameters/Reference_Points/estBo' in input file %s\n",  fileName);
	}
    if(Util_XML_Read_Array_Double(ATLANTIS_ATTRIBUTE, fileName, "TAC_Parameters/Reference_Points",childGroupingNode, no_checking, "estError", &estErrori, bm->K_num_tot_sp) == FALSE){
        quit("Error: Unable to find parameter 'TAC_Parameters/Reference_Points/estError' in input file %s\n",  fileName);
    }
    if(Util_XML_Read_Array_Double(ATLANTIS_ATTRIBUTE, fileName, "TAC_Parameters/Reference_Points",childGroupingNode, no_checking, "estCV", &estCVi, bm->K_num_tot_sp) == FALSE){
        quit("Error: Unable to find parameter 'TAC_Parameters/Reference_Points/estCV' in input file %s\n",  fileName);
    }
    if(Util_XML_Read_Array_Double(ATLANTIS_ATTRIBUTE, fileName, "TAC_Parameters/Reference_Points",childGroupingNode, no_checking, "estBias", &estBiasi, bm->K_num_tot_sp) == FALSE){
        quit("Error: Unable to find parameter 'TAC_Parameters/Reference_Points/estBias' in input file %s\n",  fileName);
    }
    
    
    
    if(	Util_XML_Read_Array_Double(ATLANTIS_ATTRIBUTE, fileName, "TAC_Parameters/Reference_Points",childGroupingNode, no_checking, "Fref", &FrefAi, bm->K_num_tot_sp) == FALSE){
		quit("Error: Unable to find parameter 'TAC_Parameters/Reference_Points/Fref' in input file %s\n",  fileName);
	}
    if(	Util_XML_Read_Array_Double(ATLANTIS_ATTRIBUTE, fileName, "TAC_Parameters/Reference_Points",childGroupingNode, no_checking, "Fref_High", &FrefHi, bm->K_num_tot_sp) == FALSE){
        quit("Error: Unable to find parameter 'TAC_Parameters/Reference_Points/Fref_High' in input file %s\n",  fileName);
    }
    if(    Util_XML_Read_Array_Double(ATLANTIS_ATTRIBUTE, fileName, "TAC_Parameters/Reference_Points",childGroupingNode, no_checking, "FrefLim", &FrefLimi, bm->K_num_tot_sp) == FALSE){
        quit("Error: Unable to find parameter 'TAC_Parameters/Reference_Points/FrefLim' in input file %s\n",  fileName);
    }
    if(	Util_XML_Read_Array_Double(ATLANTIS_ATTRIBUTE, fileName, "TAC_Parameters/Reference_Points",childGroupingNode, no_checking, "Frestart_scalar", &FreStarti, bm->K_num_tot_sp) == FALSE){
        quit("Error: Unable to find parameter 'TAC_Parameters/Reference_Points/Frestart_scalar' in input file %s\n",  fileName);
    }
    
	childGroupingNode = Util_XML_Get_Node(ATLANTIS_ATTRIBUTE_SUB_GROUP, rootnode, "SystemCap_Parameters");
	if (childGroupingNode == NULL)
		quit("readTACXML: SystemCap_Parameters attribute group not found in input file %s.\n", fileName);

	if( Util_XML_Read_Array_Double(ATLANTIS_ATTRIBUTE, fileName, "SystemCap_Parameters/",childGroupingNode, no_checking, "FlagSystCapSP", &FlagSystCapSPi, bm->K_num_tot_sp) == FALSE){
        quit("Error: Unable to find parameter 'SystemCap_Parameters/FlagSystCapSP' in input file %s\n",  fileName);
    }
    if( Util_XML_Read_Array_Double(ATLANTIS_ATTRIBUTE, fileName, "SystemCap_Parameters/",childGroupingNode, no_checking, "SystCapSPpref", &SystCapSPprefi, bm->K_num_tot_sp) == FALSE){
        quit("Error: Unable to find parameter 'SystemCap_Parameters/SystCapSPpref' in input file %s\n",  fileName);
    }
    
    if(!do_assess) {
        if( Util_XML_Read_Array_Double(ATLANTIS_ATTRIBUTE, fileName, "SystemCap_Parameters/",childGroupingNode, no_checking, "FixedAssessMort", &AssessMorti, bm->K_num_tot_sp) == FALSE){
            quit("Error: Unable to find parameter 'SystemCap_Parameters/FixedAssessMort' in input file %s\n",  fileName);
        }
    }
    

    Util_XML_Read_Species_Param(bm, fileName, groupingNode, tier_id);
    
	Util_XML_Read_Fishery_Group_Param(bm, fileName, groupingNode, trip_lim_id);
	Util_XML_Read_Fishery_Group_Param(bm, fileName, groupingNode, TAC_id);

	readBiMonthlyXML(bm, fileName, groupingNode);

	Util_XML_Read_Fishery_Param(bm, fileName, groupingNode, landallTAC_sp_id);
	Util_XML_Read_Fishery_Param(bm, fileName, groupingNode, max_num_sp_id);

	Util_XML_Read_Species_Param(bm, fileName, groupingNode, regionalSP_id);
	bm->manage_reg = (int)Util_XML_Read_Value(fileName, ATLANTIS_ATTRIBUTE,  bm->ecotest, 1, groupingNode, binary_check, "manage_reg");

	readRegionalTACXML(bm, fileName, groupingNode);

	Util_XML_Read_Species_Param(bm, fileName, groupingNode, basketSP_id);
	Util_XML_Read_Species_Param(bm, fileName, groupingNode, basket_size_id);
	bm->sp_basket = Util_Alloc_Init_2D_Int(bm->K_num_basket, bm->K_num_tot_sp, bm->K_num_tot_sp);

	readBasketTACXML(bm, fileName, groupingNode);
        
	Util_XML_Read_Species_Param(bm, fileName, groupingNode, tac_resetperiod_id);
	bm->bulkTAC = (int)Util_XML_Read_Value(fileName, ATLANTIS_ATTRIBUTE,  bm->ecotest, 1, groupingNode, binary_check, "bulkTAC");

    for(sp = 0; sp<bm->K_num_tot_sp; sp++){
    	if(FunctGroupArray[sp].isImpacted == TRUE){
			// So have a minimum of 1.0 year so when do rescaling for
			// quota period in Make_Mgmt_Decisions() don't accidently reset to zero;
			if(FunctGroupArray[sp].speciesParams[tac_resetperiod_id] < 1.0)
				FunctGroupArray[sp].speciesParams[tac_resetperiod_id] = 1.0;
			FunctGroupArray[sp].speciesParams[tac_resetcount_id] = 1.0; // So will go off in the first year if annual quotas
			if(FunctGroupArray[sp].speciesParams[tac_resetperiod_id] > 1.0){
				bulk_possible = 1;
			}
    	}
    }
    if(!bulk_possible)
    	bm->bulkTAC = 0;


	Util_XML_Read_Fishery_Group_Param(bm, fileName, groupingNode, prop_spawn_close_id);

	Util_XML_Read_Species_Param(bm, fileName, groupingNode, sp_concern_id);
    
    groupingNode = Util_XML_Get_Node(ATLANTIS_ATTRIBUTE_SUB_GROUP, rootnode, "Companion_Parameters");
    if (groupingNode == NULL)
        quit("readTACXML: Companion_Parameters attribute group not found in input file %s.\n", fileName);

    bm->K_max_co_sp = (int)Util_XML_Read_Value(fileName, ATLANTIS_ATTRIBUTE,  bm->ecotest, 1, groupingNode, integer_check, "K_max_co_sp");
    //Util_XML_Parse_Create_Node(fp, fileName, groupingNode, "max_co_sp", "Maximum number of companions in a companion TAC", "", XML_TYPE_INTEGER, "2");

    if( bm->K_max_co_sp > 0) {
        for (i = 0; i < bm->K_num_tot_sp; i++) {
            FunctGroupArray[i].co_sp = Util_Alloc_Init_1D_Int(bm->K_max_co_sp, 0);
            FunctGroupArray[i].co_sp_catch = Util_Alloc_Init_2D_Double(bm->K_max_co_sp, bm->K_num_fisheries, 0.0);
        }

        Util_XML_Read_Species_Param(bm, fileName, groupingNode, max_co_sp_id);
        readCompanionSpeciesXML(bm, fileName, groupingNode);
        Util_XML_Read_Species_Param(bm, fileName, groupingNode, coType_id);
        
        Read_Fishery_Companion_Catch_XML(bm, fileName, groupingNode);
    }


}

/* Read the Seasonal Fishery xml nodes */
void readSeaonalFisheryXML(MSEBoxModel *bm, char *fileName, xmlNodePtr rootnode) {

	int i;
	xmlNodePtr groupingNode;
	double *values = 0;
	char errorString[STRLEN];
	sprintf(errorString, "%s/Seasonal_Fishery_Parameters", (char *)rootnode->name);

	groupingNode = Util_XML_Get_Node(ATLANTIS_ATTRIBUTE_SUB_GROUP, rootnode, "Seasonal_Fishery_Parameters");
	if (groupingNode == NULL)
		quit("readSeaonalFisheryXML: %s attribute group not found in input file %s.\n", errorString, fileName);

	if(Util_XML_Read_Array_Double(ATLANTIS_ATTRIBUTE, fileName, errorString, groupingNode, integer_check, "RegSeason", (double **) &values, bm->K_num_reg) == FALSE){
		quit("Error: Unable to find parameter '%s/RegSeason' in input file %s\n",  errorString, fileName);
	}
	for (i = 0; i < bm->K_num_reg; i++) {
		bm->reg_season[i] = (int) values[i];
	}
	free(values);
	Util_XML_Read_Fishery_Param(bm, fileName, groupingNode, seasonopen_id);
	Util_XML_Read_Fishery_Param(bm, fileName, groupingNode, seasonclose_id);
}

/* Read the ryke based Fishery xml nodes */
void readRuleBasedFisheryManagementXML(MSEBoxModel *bm, char *fileName, xmlNodePtr rootnode) {

	xmlNodePtr groupingNode;

	groupingNode = Util_XML_Get_Node(ATLANTIS_ATTRIBUTE_SUB_GROUP, rootnode, "Rule_Based_Fishery_Management");
	if (groupingNode == NULL)
		quit("readRuleBasedFisheryManagementXML: Rule_Based_Fishery_Management attribute group not found in input file %s.\n", fileName);

	Util_XML_Read_Impacted_Group_Param(bm, fileName, groupingNode, FC_thresh_id);
	Util_XML_Read_Impacted_Group_Param(bm, fileName, groupingNode, FC_high_thresh_id);

	bm->reinit_pop_day = Util_XML_Read_Value(fileName, ATLANTIS_ATTRIBUTE,  bm->ecotest, 1, groupingNode, no_checking, "reinit_pop_day");
}

/* This should perhaps be in the HarvestManagement input file */
void readRestricturedXML(MSEBoxModel *bm, char *fileName, xmlNodePtr rootnode) {

	xmlNodePtr groupingNode, childGroupingNode;

	groupingNode = Util_XML_Get_Node(ATLANTIS_ATTRIBUTE_SUB_GROUP, rootnode, "Effort_Reduction");
	if (groupingNode == NULL)
		quit("readRestricturedXML: Effort_Reduction attribute group not found in input file %s.\n", fileName);

	Util_XML_Read_Fishery_Param(bm, fileName, groupingNode, FC_restrict_id);
	Util_XML_Read_Fishery_Param(bm, fileName, groupingNode, FC_period_id);
	Util_XML_Read_Fishery_Param(bm, fileName, groupingNode, FC_period2_id);

	childGroupingNode = Util_XML_Get_Node(ATLANTIS_ATTRIBUTE_SUB_GROUP, groupingNode, "Endangered_Restrictions");
	if (childGroupingNode == NULL)
		quit("readRestricturedXML: Endangered_Restrictions attribute group not found in input file %s.\n", fileName);

	Util_XML_Read_Fishery_Param(bm, fileName, childGroupingNode, FC_restrict_endangered_id);
	Util_XML_Read_Fishery_Param(bm, fileName, childGroupingNode, FC_endanger_period_id);

}

/* Changes in effort */
void readEffortChangedXML(MSEBoxModel *bm, char *fileName, xmlNodePtr rootnode) {

	xmlNodePtr groupingNode, childGroupingNode;
	int Effort_max_num_changes, CAP_max_num_changes;

	groupingNode = Util_XML_Get_Node(ATLANTIS_ATTRIBUTE_SUB_GROUP, rootnode, "Effort_Changes");
	if (groupingNode == NULL)
		quit("readEffortChangedXML: Effort_Changes attribute group not found in input file %s.\n", fileName);

	Util_XML_Read_Fishery_Param(bm, fileName, groupingNode, EFF_num_changes_id);

	Effort_max_num_changes = 1;
	if (bm->flagchangeeffort)
		Effort_max_num_changes = Manage_Get_Max_Fishery_Param(bm, EFF_num_changes_id);

	EFFORTchange = Util_Alloc_Init_3D_Double(4, Effort_max_num_changes, bm->K_num_fisheries, 0.0);

	readEffortChangeXML(bm, fileName, groupingNode, "StartChangeDay", start_id);
	readEffortChangeXML(bm, fileName, groupingNode, "StartChangePeriod", period_id);
	readEffortChangeXML(bm, fileName, groupingNode, "EffortChangeProportion", mult_id);
	readEffortChangeXML(bm, fileName, groupingNode, "EffortChangePulseFlag", pulse_id);

	childGroupingNode = Util_XML_Get_Node(ATLANTIS_ATTRIBUTE_SUB_GROUP, groupingNode, "Effort_CAP");
	if (childGroupingNode == NULL)
		quit("Effort_CAP: Effort_CAP attribute group not found in input file %s.\n", fileName);

	Util_XML_Read_Fishery_Param(bm, fileName, childGroupingNode, CAP_num_changes_id);

	/* Get the maximum number of CAP changes so we can allocate the array and load the values */
	CAP_max_num_changes = 1;
	if (bm->flagchangecap)
		CAP_max_num_changes = Manage_Get_Max_Fishery_Param(bm, CAP_num_changes_id) + 1;
	CAPchange = Util_Alloc_Init_3D_Double(4, CAP_max_num_changes, bm->K_num_fisheries, 0.0);

	Util_XML_Read_Change_Values(bm, fileName, childGroupingNode, "CAPchange", CAP_num_changes_id, CAPchange);

}

/**
 * changes in management over time
 *
 */

void readManagementChangeXML(MSEBoxModel *bm, char *fileName, xmlNodePtr rootnode) {

	xmlNodePtr groupingNode, childGroupingNode;
	int POP_changes, i, b;

	groupingNode = Util_XML_Get_Node(ATLANTIS_ATTRIBUTE_SUB_GROUP, rootnode, "Management_Changes");
	if (groupingNode == NULL)
		quit("readManagementChangeXML: Management_Changes attribute group not found in input file %s.\n", fileName);

	childGroupingNode = Util_XML_Get_Node(ATLANTIS_ATTRIBUTE_SUB_GROUP, groupingNode, "Population_Changes");
	if (childGroupingNode == NULL)
		quit("readManagementChangeXML: Population_Changes attribute group not found in input file %s.\n", fileName);

	readPortXMLData(bm, fileName, childGroupingNode, "ports_popchange", popchange_id);
	readPortXMLData(bm, fileName, childGroupingNode, "ports_popnumchange", POP_num_changes_id);

	/* Get the max number of POP changes and then allocate the POP change array.*/
	//TODO: This might be a bug - should POP_changes be set to 0 intially?
	POP_changes = 1;
	for (i = 0; i < bm->K_num_ports; i++) {
		POP_changes = (int) max ( (double) POP_changes, bm->Port_info[i][POP_num_changes_id] );
	}

	if (POP_changes > 0)
		bm->flagchangepop = 0;

	POP_max_num_changes = 1;
	if (bm->flagchangepop)
		POP_max_num_changes = POP_changes + 1;

	bm->POPchange = (double ***) alloc3d(3, POP_max_num_changes, bm->K_num_ports);

	for (i = 0; i < POP_max_num_changes; i++) {
		for (b = 0; b < bm->K_num_ports; b++) {
			bm->POPchange[b][i][start_id] = 0;
			bm->POPchange[b][i][period_id] = 1;
			bm->POPchange[b][i][mult_id] = 1;
		}
	}

	readPopulationChangeXML(bm, fileName, childGroupingNode, "PortPopulationChangeStart", start_id);
	readPopulationChangeXML(bm, fileName, childGroupingNode, "PortPopulationChangePeriod", period_id);
	readPopulationChangeXML(bm, fileName, childGroupingNode, "PortPopulationChangeMult", mult_id);
}

/**
 * Management performance indicators.
 / *
 */

void readManagementPerformanceIndicatorsXML(MSEBoxModel *bm, char *fileName, xmlNodePtr rootnode) {

	xmlNodePtr groupingNode;

	groupingNode = Util_XML_Get_Node(ATLANTIS_ATTRIBUTE_SUB_GROUP, rootnode, "Management_Performance_Indicators");
	if (groupingNode == NULL)
		quit("readManagementPerformanceIndicatorsXML: Management_Performance_Indicators attribute group not found in input file %s.\n", fileName);

	/* Performance Measure coefficients */
	bm->enforce_coefft = Util_XML_Read_Value(fileName, ATLANTIS_ATTRIBUTE,  bm->ecotest, 1, groupingNode, no_checking, "enforce_coefft");
	bm->habcoefft = Util_XML_Read_Value(fileName, ATLANTIS_ATTRIBUTE,  bm->ecotest, 1, groupingNode, no_checking, "habcoefft");
	bm->tepcoefft = Util_XML_Read_Value(fileName, ATLANTIS_ATTRIBUTE,  bm->ecotest, 1, groupingNode, no_checking, "tepcoefft");
	bm->sharkcoefft = Util_XML_Read_Value(fileName, ATLANTIS_ATTRIBUTE,  bm->ecotest, 1, groupingNode, no_checking, "sharkcoefft");

}
/*
 * Fishing gear
 */
void readGearConflictXML(MSEBoxModel *bm, char *fileName, xmlNodePtr rootnode) {
	xmlNodePtr groupingNode;

	groupingNode = Util_XML_Get_Node(ATLANTIS_ATTRIBUTE_SUB_GROUP, rootnode, "Gear_Conflict");
	if (groupingNode == NULL)
		quit("readGearConflictXML: Gear_Conflict attribute group not found in input file %s.\n", fileName);

	Util_XML_Read_Fishery_Param_Double(bm, fileName, groupingNode, "GearConflictBetweenFisheries", binary_check, gear_conflict, bm->K_num_fisheries);
}

/*
 * Fishing Spatial Management
 */
void readSpatialManagementXML(MSEBoxModel *bm, char *fileName, xmlNodePtr rootnode) {
	xmlNodePtr groupingNode;

	groupingNode = Util_XML_Get_Node(ATLANTIS_ATTRIBUTE_SUB_GROUP, rootnode, "Spatial_Management");
	if (groupingNode == NULL)
		quit("readSpatialManagementXML: Spatial_Management attribute group not found in input file %s.\n", fileName);

	Util_XML_Read_Fishery_Param(bm, fileName, groupingNode, infringe_id);

	readCellMPAStatusXML(bm, fileName, groupingNode);

	/* Zoning for each box per fishery when	trigger to protect
	 endangered groups is set off */

	readMPAEndangeredXML(bm, fileName, groupingNode);
	/* Zoning for each box per fishery when	trip limit trigger to protect
	 qoverfished groups is set off */

	readMPAOverFishedXML(bm, fileName, groupingNode);

}

/* Read the contaminant Fishery xml ndoes */
void readContaminantFisheryXML(MSEBoxModel *bm, char *fileName, xmlNodePtr rootnode) {
    int i, cIndex;
    xmlNodePtr groupingNode;
    char varStr[STRLEN*2];
    double *values = 0;
    char errorString[STRLEN];
    sprintf(errorString, "%s/Fishery_Contaminant_Parameters", (char *)rootnode->name);
 
    groupingNode = Util_XML_Get_Node(ATLANTIS_ATTRIBUTE_SUB_GROUP, rootnode, "Fishery_Contaminant_Parameters");
    if (groupingNode == NULL)
        quit("readContaminantFisheryXML: %s attribute group not found in input file %s.\n", errorString, fileName);
    
    if(Util_XML_Read_Array_Double(ATLANTIS_ATTRIBUTE, fileName, errorString, groupingNode, integer_check, "ContamClosed", &values, bm->nbox) == FALSE){
        quit("Error: Unable to find parameter '%s/ContamClosed' in input file %s\n",  errorString, fileName);
    }
    
    for (i = 0; i < bm->nbox; i++) {
        bm->ContamClosed[i] = values[i];
    }
    free(values);
    
    bm->flag_contam_fisheries_mgmt = Util_XML_Read_Value(fileName, ATLANTIS_ATTRIBUTE, bm->ecotest, 1, groupingNode, integer_check, "flag_contam_fisheries_mgmt");
    bm->contam_fishery_closure_day = Util_XML_Read_Value(fileName, ATLANTIS_ATTRIBUTE, bm->ecotest, 1, groupingNode, integer_check, "contam_fishery_closure_day");
    bm->contam_fishery_closure_period = Util_XML_Read_Value(fileName, ATLANTIS_ATTRIBUTE, bm->ecotest, 1, groupingNode, integer_check, "contam_fishery_closure_period");
    bm->contam_fishery_closure_option = Util_XML_Read_Value(fileName, ATLANTIS_ATTRIBUTE, bm->ecotest, 1, groupingNode, integer_check, "contam_fishery_closure_option");

    for(cIndex = 0; cIndex < bm->num_contaminants; cIndex++){
        sprintf(varStr, "%s_fishery_thresh_level", bm->contaminantStructure[cIndex]->contaminant_name);
        bm->contaminantStructure[cIndex]->fishery_thresh_level = Util_XML_Read_Value(fileName, ATLANTIS_ATTRIBUTE, bm->ecotest, 1, groupingNode, no_checking, varStr);
    }
}

/**
 *	This routine reads all parameters of the management models
 *
 *  Input:  char *filename: The name of the management xml input file.
 *
 */
int Read_Manage_Paramaters(MSEBoxModel *bm, char *filename) {
	xmlDocPtr inputDoc;
	inputDoc = xmlReadFileDestFolder(bm->destFolder, filename, NULL, 0);

	printf("Management parameter file loading\n");
	printf("0                                 100\n");
	printf("***");

	readManamentFlagTimeXML(bm, filename, inputDoc->children);
	readManagementSenarioFlagsXML(bm, filename, inputDoc->children);
	readManagementMSYXML(bm, filename, inputDoc->children);
	readManagementEffortXML(bm, filename, inputDoc->children);
	readPortIndexXML(bm, filename, inputDoc->children);
	readTACXML(bm, filename, inputDoc->children);

	readSeaonalFisheryXML(bm, filename, inputDoc->children);
	readRuleBasedFisheryManagementXML(bm, filename, inputDoc->children);
	readRestricturedXML(bm, filename, inputDoc->children);
	readEffortChangedXML(bm, filename, inputDoc->children);
	readManagementChangeXML(bm, filename, inputDoc->children);
	readManagementPerformanceIndicatorsXML(bm, filename, inputDoc->children);
	readSpatialManagementXML(bm, filename, inputDoc->children);
	readGearConflictXML(bm, filename, inputDoc->children);
    if(bm->track_contaminants){
        readContaminantFisheryXML(bm, filename, inputDoc->children);
    }    
	// Used to do tiered read-in here but moved to the assessment library

	xmlFreeDoc(inputDoc);
	/* Shutdown libxml */
	xmlCleanupParser();
	return TRUE;
}
