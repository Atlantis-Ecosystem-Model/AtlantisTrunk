/*******************************************************************//**
 \file
 \brief Functions to load parameters from the biology input XML file.
 \ingroup atEcology

 File:			atbiologyXMLParamIO.c

 Purpose:        Functions to load parameters from the biology input XML file.

 Revisions:

 20/05/2008 Bec Gorton

 Made sure each function prints its name if the species index is not found.
 Also deleted getMoveSpeciesFG() as its no longer needed.

 20-07-2008 Bec Gorton

 Split the MIGRATEIO flag into seperate tags so that these values are
 printed as seperate groups in the database output file that will be
 used to load the model data into the database.

 31-03-2009 Bec Gorton
 Changed the readXMLArray function from
 int readXMLArray(double **values, int size, char *valueStr, int entry_type)
 to
 int readXMLArray(xmlDocPtr doc, char *xpath, int doQuit, double **values, int size, int entry_type)

 So the readXMLArray now takes the xpath, finds the node in the tree, allocates memory for
 the values array, checks each values is the correct entry_type and sets each value in the
 newly allocated values array.

 10-06-2009 Bec Gorton
 Changed the structure of the XML document. The structure is not using the attributeNode concept.
 See Jira task around this for more information.

 10-06-2009 Bec Gorton
 Added the SEAGRASS invertebrate type.

 28-10-2009 Bec Gorton
 Change many of the attribute_groups to attribute_sub_groups.

 14-12-2009 Bec Gorton
 Resolved the duplicate min_deth_id and mindepth_id species parameters. The parameter is now called
 mindepth_id.

 06-01-2010 Bec Gorton
 Added code to read in the lim_sun_hours and min_channel_depth parameters. Also the homerangerad_id and
 rangeoverlap_id species parameters.

 16-02-2010 Bec Gorton
 Added the Read_Change_DataXML function to streamline reading in values that change over a period of time.
 This function is used to read in the T_change and S_change data. The Schange_max_num and Tchange_max_num
 values are now calculated as the input file is parsed allowing for allocation of arrays to store
 the change information. This removes the need to have a second pass of the input file.
 This function is very similar to the one used to read in fisheries data.

 22-02-2010 Bec Gorton
 Added the getSpeciesCohortNodeValue and Read_Cohort_Species_Param_Values functions to this file.
 These were previously in the atbiolSpeciesParamIO.c file.

 23-11-2010 Bec Gorton
 Removed the code that read in the catcheater_id species param from the biology input file
 - this is now read in from the functional group input file.

 26-03-2011 Beth Fulton
 Added species specific starvation coefficients - mStarve

 17-04-2013 Bec Gorton
 ATL-346 - Added support of tracking P:N:C ratios in groups.
 *********************************************************************/

/*  Open library routines */
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>
#include <ctype.h>
#include <sjwlib.h>
#include <netcdf.h>
#include <atecology.h>
#include "convertXML.h"

#define K_max_num_mig 3


#ifdef _WIN32
#ifdef _DEBUG
#include <crtdbg.h>
#endif
#endif

char *dayStrings[] =
	{ "night", "day" };
char *cohortString[] =
	{ "juv", "adult" };

void Util_XML_Read_Species_Param(MSEBoxModel *bm, char *fileName, xmlNodePtr parent, int paramID);
//
//static double *getSpeciesCohortNodeValue(xmlNodePtr parent, char *fileName, int paramID, int guild, int numCohorts, SpeciesParamStruct *paramStruct) {
//
//	double *values;
//	char *nodeName;
//
//	nodeName =  Util_Get_Node_Name(parent);
//
//	Util_XML_Read_Array_Double(ATLANTIS_GROUP_ATTRIBUTE, fileName, nodeName, parent, paramStruct[paramID].checkType, FunctGroupArray[guild].groupCode, &values,
//			numCohorts);
//
//	free(nodeName);
//	return values;
//}

/**
 * \brief Read in the cohort species parameters.
 *
 *
 *
 */
static void Read_Cohort_Species_Param_Values(MSEBoxModel *bm, char *fileName, xmlNodePtr parent, int paramID) {
	int guild;
	int index;
	int cohort;
	double *values;
	xmlNodePtr attributeGroup;
	char errorString[STRLEN];
	char *nodeName;

	nodeName =  Util_Get_Node_Name(parent);

	index = Util_XML_Get_Param_Index(cohortSpeciesParamStructArray, cohortDepParams, paramID);
	if (verbose > 0)
		printf("Read species cohort parameters %s\n", cohortSpeciesParamStructArray[index].tag);

	sprintf(errorString, "%s/%s", nodeName, cohortSpeciesParamStructArray[index].tag);
	free(nodeName);
	attributeGroup = Util_XML_Get_Node(ATLANTIS_ATTRIBUTE, parent, cohortSpeciesParamStructArray[index].tag);
	if (attributeGroup == NULL)
		quit("%s attribute group not found.\n", errorString);

	switch (paramID) {
	case mL_T15_id:
    case mLext_id:
    case mPext_id:
		for (guild = 0; guild < bm->K_num_tot_sp; guild++) {
			if ((FunctGroupArray[guild].speciesParams[flag_id] == TRUE) && (FunctGroupArray[guild].isDetritus == FALSE)) {
				if(FunctGroupArray[guild].numStages > 1){
					if (Util_XML_Read_Array_Double(ATLANTIS_GROUP_ATTRIBUTE, fileName, errorString, attributeGroup, cohortSpeciesParamStructArray[index].checkType,
							FunctGroupArray[guild].groupCode, &values, FunctGroupArray[guild].numStages) == FALSE) {
						quit("Error: Unable to read parameter %s/%s from input file %s\n", cohortSpeciesParamStructArray[index].tag,
								FunctGroupArray[guild].groupCode, fileName);
					}

					for (cohort = 0; cohort < FunctGroupArray[guild].numStages; cohort++) {
						FunctGroupArray[guild].cohortSpeciesParams[cohort][paramID] = values[cohort] / cohortSpeciesParamStructArray[index].divider;
                        // this used to read cohortSpeciesParamStructArray[paramID].divider which worked as paramID = index by coincidence here, changed it to index for consistency sake and incase the coincidence breaks in the future
                        
                        //fprintf(bm->logFile, "Reading in %s-%d paramID: %d val %e divider: %e\n", FunctGroupArray[guild].groupCode, cohort, paramID, values[cohort], cohortSpeciesParamStructArray[index].divider);
					}
					free(values);
				} else {
					FunctGroupArray[guild].cohortSpeciesParams[0][paramID] = Util_XML_Get_Species_Node_Value(attributeGroup, index, guild, cohortSpeciesParamStructArray);
				}
			}
		}
		break;
	case mQ_T15_id:
		for (guild = 0; guild < bm->K_num_tot_sp; guild++) {
			if ((FunctGroupArray[guild].speciesParams[flag_id] == TRUE) && (FunctGroupArray[guild].isDetritus == FALSE)
					&& (FunctGroupArray[guild].isPrimaryProducer == FALSE) && (FunctGroupArray[guild].isBacteria == FALSE)) {
				if(FunctGroupArray[guild].numStages > 1){
					if (Util_XML_Read_Array_Double(ATLANTIS_GROUP_ATTRIBUTE, fileName, errorString, attributeGroup, cohortSpeciesParamStructArray[index].checkType,
							FunctGroupArray[guild].groupCode, &values, FunctGroupArray[guild].numStages) == FALSE) {
						quit("Error: Unable to read parameter %s/%s from input file %s\n", cohortSpeciesParamStructArray[index].tag,
								FunctGroupArray[guild].groupCode, fileName);
					}

					for (cohort = 0; cohort < FunctGroupArray[guild].numStages; cohort++) {
						FunctGroupArray[guild].cohortSpeciesParams[cohort][paramID] = values[cohort] / cohortSpeciesParamStructArray[index].divider; // this used to read cohortSpeciesParamStructArray[paramID].divider which worked as paramID = index by coincidence here, changed it to index for consistency sake and incase the coincidence breaks in the future
                        
                        if(values[cohort] > 1.0)
                            quit("It does not make sense to have a mortality greater than 1.0 (%e) for %s stage %d\n", values[cohort], FunctGroupArray[guild].groupCode, cohort);
                        
					}
					free(values);
				} else {
					FunctGroupArray[guild].cohortSpeciesParams[0][paramID] = Util_XML_Get_Species_Node_Value(attributeGroup, index, guild,
							cohortSpeciesParamStructArray);
                    
                    if(FunctGroupArray[guild].cohortSpeciesParams[0][paramID] > 1.0)
                        quit("It does not make sense to have a mortality greater than 1.0 (%e) for the single stage of %s. If this is an age structured (e.g. vertebrate) group make sure you have only provided a single value of the form XX_mQ 0.1 not an age structured value as for groups with multiple age stages\n", FunctGroupArray[guild].cohortSpeciesParams[0][paramID], FunctGroupArray[guild].groupCode);
                    
				}
			}
		}
		break;
    case L_turbid_id:
    case a_turbid_id:
    case b_turbid_id:
        for (guild = 0; guild < bm->K_num_tot_sp; guild++) {
            if ((FunctGroupArray[guild].speciesParams[flag_id] == TRUE) && (FunctGroupArray[guild].isDetritus == FALSE)){
                if(FunctGroupArray[guild].numStages > 1){
                    if (Util_XML_Read_Array_Double(ATLANTIS_GROUP_ATTRIBUTE, fileName, errorString, attributeGroup, cohortSpeciesParamStructArray[index].checkType, FunctGroupArray[guild].groupCode, &values, FunctGroupArray[guild].numStages) == FALSE) {
                            quit("Error: Unable to read parameter %s/%s from input file %s\n", cohortSpeciesParamStructArray[index].tag, FunctGroupArray[guild].groupCode, fileName);
                    }
                        
                    for (cohort = 0; cohort < FunctGroupArray[guild].numStages; cohort++) {
                        FunctGroupArray[guild].cohortSpeciesParams[cohort][paramID] = values[cohort] / cohortSpeciesParamStructArray[index].divider;
                    }
                    free(values);
                } else {
                    FunctGroupArray[guild].cohortSpeciesParams[0][paramID] = Util_XML_Get_Species_Node_Value(attributeGroup, index, guild, cohortSpeciesParamStructArray);
                }
            }
        }
        break;
    /*
    case num_migrate_id:
		for (guild = 0; guild < bm->K_num_tot_sp; guild++) {
			if ((FunctGroupArray[guild].speciesParams[flag_id] == TRUE) && (FunctGroupArray[guild].isDetritus == FALSE)) {
				if (FunctGroupArray[guild].sp_geo_move == TRUE) {
					if (FunctGroupArray[guild].groupAgeType == AGE_STRUCTURED || FunctGroupArray[guild].groupAgeType == BIOMASS) {
						FunctGroupArray[guild].cohortSpeciesParams[0][paramID] = Util_XML_Get_Species_Node_Value(attributeGroup, index, guild,
								cohortSpeciesParamStructArray);
					} else if (FunctGroupArray[guild].groupAgeType == AGE_STRUCTURED_BIOMASS) {
						// We need to read in the array of values and then store then in the cohort slots
						if (Util_XML_Read_Array_Double(ATLANTIS_GROUP_ATTRIBUTE, fileName, errorString, attributeGroup, cohortSpeciesParamStructArray[index].checkType,
								FunctGroupArray[guild].groupCode, &values, FunctGroupArray[guild].numCohorts) == FALSE) {
							quit("Error: Unable to read parameter %s/%s from input file %s\n", cohortSpeciesParamStructArray[index].tag,
									FunctGroupArray[guild].groupCode, fileName);
						}
						for (cohort = 0; cohort < FunctGroupArray[guild].numStages; cohort++) {
							FunctGroupArray[guild].cohortSpeciesParams[cohort][paramID] = values[cohort];
						}
						free(values);
					}
				}
			}
		}
		break;
    */
	case rugosity_inc_id:
	case rugosity_dec_id:
    case colony_diam_id:

		for (guild = 0; guild < bm->K_num_tot_sp; guild++) {
			if ((FunctGroupArray[guild].speciesParams[flag_id] == TRUE) && ((FunctGroupArray[guild].groupType == CORAL) || (FunctGroupArray[guild].groupType == SPONGE))) {
				switch (FunctGroupArray[guild].groupAgeType) {
				case AGE_STRUCTURED:
				case AGE_STRUCTURED_BIOMASS:
				case BIOMASS:
					/* We need to read in the values for each life history slots*/
					if(FunctGroupArray[guild].numStages > 1) {
						if (Util_XML_Read_Array_Double(ATLANTIS_GROUP_ATTRIBUTE, fileName, errorString, attributeGroup, cohortSpeciesParamStructArray[index].checkType,
								FunctGroupArray[guild].groupCode, &values, FunctGroupArray[guild].numStages) == FALSE) {
							quit("Error: Unable to read parameter %s/%s from input file %s\n", cohortSpeciesParamStructArray[index].tag,
									FunctGroupArray[guild].groupCode, fileName);
						}

						for (cohort = 0; cohort < FunctGroupArray[guild].numStages; cohort++) {
							FunctGroupArray[guild].cohortSpeciesParams[cohort][paramID] = values[cohort] / cohortSpeciesParamStructArray[index].divider; // Note need to use index here for the divider call as paramID != index for these params whereas it does for mL and mQ by coincidence
						}
						free(values);
					} else {
						FunctGroupArray[guild].cohortSpeciesParams[0][paramID] = Util_XML_Get_Species_Node_Value(attributeGroup, index, guild,
								cohortSpeciesParamStructArray);

					}
					break;
				}
			}
		}

		break;
	default:
		quit("Read_Cohort_Species_Param_Values paramID not found = %d\n", paramID);
		break;
	}
}

static void Read_Spawn_Species_Param_Values(MSEBoxModel *bm, char *fileName, xmlNodePtr parent, int paramID) {
	int guild;
	int index;
	int nspawn;
	double *values;
	xmlNodePtr attributeGroup;
	char errorString[STRLEN];
	char *nodeName;

	if((paramID == Time_Age_id) && !bm->containsMCPYr){
		for (guild = 0; guild < bm->K_num_tot_sp; guild++) {
			for (nspawn = 0; nspawn < FunctGroupArray[guild].numSpawns; nspawn++) {
				FunctGroupArray[guild].spawnSpeciesParams[nspawn][paramID] = 0;
			}
		}
		return;
	}

	nodeName =  Util_Get_Node_Name(parent);

	index = Util_XML_Get_Param_Index(spawnSpeciesParamStructArray, spawnDepParams, paramID);
	if (verbose > 0)
		printf("Read species spawn parameters %s\n", spawnSpeciesParamStructArray[index].tag);

	sprintf(errorString, "%s/%s", nodeName, spawnSpeciesParamStructArray[index].tag);
	free(nodeName);
	attributeGroup = Util_XML_Get_Node(ATLANTIS_ATTRIBUTE, parent, spawnSpeciesParamStructArray[index].tag);
	if (attributeGroup == NULL)
		quit("%s attribute group not found.\n", errorString);

	switch (paramID) {
	case Time_Spawn_id:
		for (guild = 0; guild < bm->K_num_tot_sp; guild++) {
			if (FunctGroupArray[guild].speciesParams[flag_id] == TRUE) {
				if ((FunctGroupArray[guild].groupAgeType == AGE_STRUCTURED || (FunctGroupArray[guild].groupAgeType == AGE_STRUCTURED_BIOMASS)) || ((FunctGroupArray[guild].groupAgeType == BIOMASS) && FunctGroupArray[guild].isCultured )){
					if(FunctGroupArray[guild].numSpawns == 1) {
						FunctGroupArray[guild].spawnSpeciesParams[0][paramID] = Util_XML_Get_Species_Node_Value(attributeGroup, index, guild, 
							spawnSpeciesParamStructArray);
					} else {
						/* We need to read in the array of values and then store then in the slots*/
						if (Util_XML_Read_Array_Double(ATLANTIS_GROUP_ATTRIBUTE, fileName, errorString, attributeGroup, spawnSpeciesParamStructArray[index].checkType,
								FunctGroupArray[guild].groupCode, &values, FunctGroupArray[guild].numSpawns) == FALSE) {
							quit("Error: Unable to read parameter %s/%s from input file %s\n", spawnSpeciesParamStructArray[index].tag,
									FunctGroupArray[guild].groupCode, fileName);
						}

						/* We need to read in the array of values and then store then in the correct slots*/
						for (nspawn = 0; nspawn < FunctGroupArray[guild].numSpawns; nspawn++) {
							FunctGroupArray[guild].spawnSpeciesParams[nspawn][paramID] = values[nspawn];
						}
						free(values);
					}
				}
			}
		}
		break;
	case Time_Age_id:
		for (guild = 0; guild < bm->K_num_tot_sp; guild++) {
			if (FunctGroupArray[guild].speciesParams[flag_id] == TRUE) {
				if (FunctGroupArray[guild].groupAgeType == AGE_STRUCTURED || (FunctGroupArray[guild].groupAgeType == AGE_STRUCTURED_BIOMASS)){
					if(FunctGroupArray[guild].numSpawns == 1) {
						FunctGroupArray[guild].spawnSpeciesParams[0][paramID] = 0;  // Effectively do nothing as will just be the day before spawning
					} else {
						/* We need to read in the array of values and then store then in the slots*/
						if (Util_XML_Read_Array_Double(ATLANTIS_GROUP_ATTRIBUTE, fileName, errorString, attributeGroup, spawnSpeciesParamStructArray[index].checkType,
								FunctGroupArray[guild].groupCode, &values, FunctGroupArray[guild].numSpawns) == FALSE) {
							quit("Error: Unable to read parameter %s/%s from input file %s\n", spawnSpeciesParamStructArray[index].tag,
									FunctGroupArray[guild].groupCode, fileName);
						}

						/* We need to read in the array of values and then store then in the correct slots*/
						for (nspawn = 0; nspawn < FunctGroupArray[guild].numSpawns; nspawn++) {
							FunctGroupArray[guild].spawnSpeciesParams[nspawn][paramID] = values[nspawn];
						}
						free(values);
					}
				}
			}
		}
		break;
	default:
		quit("Read_Spawn_Species_Param_Values paramID not found = %d\n", paramID);
		break;
	}
}


void Read_Change_DataXML(MSEBoxModel *bm, char *fileName, xmlNodePtr parent, char *parameterName, double **array, int size, int checkType) {
	double *values = 0;
	int b, i;
	char *names[] =
		{ "_start", "_period", "_mult" };
	int indexValues[] =
		{ start_id, period_id, mult_id };
	char str[STRLEN];
	char errorString[STRLEN*2];
	char *nodeName;

	nodeName =  Util_Get_Node_Name(parent);

	for (i = 0; i < 3; i++) {

		sprintf(str, "%s%s", parameterName, names[i]);
		sprintf(errorString, "%s/%s", nodeName, str);
		if (verbose)
			printf("Reading %s values\n", str);

		/* Read in the value */
		if (Util_XML_Read_Array_Double(ATLANTIS_ATTRIBUTE, fileName, errorString, parent, checkType, str, &values, size) == FALSE) {
			quit("Error: Unable to read parameter %s from input file %s\n", str, fileName);
		}

		for (b = 0; b < size; b++) {
			array[b][indexValues[i]] = values[b];
 		}

		free(values);
	}
	free(nodeName);

}
/**
 *	\brief Create the XML structure to hold the vertebrate seasonal distribution data.
 */
void Read_Vert_Species_IntegerXML(MSEBoxModel *bm, char *fileName, xmlNodePtr parent, char *parameterName, int **array, int size, int checkType) {

	double *values = 0;
	int guild, b;
	xmlNodePtr attributeNode;
	char errorString[STRLEN];
	char *nodeName;

	nodeName =  Util_Get_Node_Name(parent);
	sprintf(errorString, "%s/%s", nodeName, parameterName);

	if (verbose)
		printf("Reading %s values\n", parameterName);

	attributeNode = Util_XML_Get_Node(ATLANTIS_ATTRIBUTE, parent, parameterName);
	if (attributeNode == NULL)
		quit("%s attribute group not found.\n", errorString);

	/* Create a node for each functional group*/
	for (guild = 0; guild < bm->K_num_tot_sp; guild++) {
		if (FunctGroupArray[guild].isVertebrate == TRUE) {
			if (Util_XML_Read_Array_Double(ATLANTIS_GROUP_ATTRIBUTE, fileName, errorString, attributeNode, checkType, FunctGroupArray[guild].groupCode, &values, size)
					== FALSE) {
				quit("Error: Unable to find parameter %s/%s in input file %s\n", parameterName, FunctGroupArray[guild].groupCode, fileName);
			}

			for (b = 0; b < size; b++)
				array[b][guild] = (int) values[b];

			free(values);
		}
	}
	free(nodeName);
}

/**
 *	\brief Create the XML structure to hold the cultured distribution data.
 */
static void Read_Cultured_SpeciesXML(MSEBoxModel *bm, char *fileName, xmlNodePtr parent, char *parameterName, double **array, int size, int checkType) {
    
	double *values = 0;
	int guild, b;
	xmlNodePtr attributeNode;
	char errorString[STRLEN];
	char *nodeName;
    
	nodeName =  Util_Get_Node_Name(parent);
    
	sprintf(errorString, "%s/%s", nodeName, parameterName);
    
	if (verbose)
		printf("Reading %s values\n", parameterName);
    
	attributeNode = Util_XML_Get_Node(ATLANTIS_ATTRIBUTE, parent, parameterName);
	if (attributeNode == NULL)
		quit("%s attribute group not found.\n", errorString);
    
	/* Create a node for each functional group*/
	for (guild = 0; guild < bm->K_num_tot_sp; guild++) {
        if(FunctGroupArray[guild].isCultured){
            
			if (Util_XML_Read_Array_Double(ATLANTIS_GROUP_ATTRIBUTE, fileName, errorString,  attributeNode, checkType, FunctGroupArray[guild].groupCode, &values, size) == FALSE) {
				quit("Error: Unable to find parameter %s/%s in input file %s\n", parameterName, FunctGroupArray[guild].groupCode, fileName);
			}
            
			for (b = 0; b < size; b++) {
				array[b][guild] = values[b];
            }
            
			free(values);
		}
	}
	free(nodeName);
}
/**
 *	\brief Create the XML structure to hold the vertebrate seasonal distribution data.
 */
static void Read_Vert_SpeciesXML(MSEBoxModel *bm, char *fileName, xmlNodePtr parent, char *parameterName, double **array, int size, int checkType) {

	double *values = 0;
	int guild, b;
	xmlNodePtr attributeNode;
	char errorString[STRLEN];
	char *nodeName;

	nodeName =  Util_Get_Node_Name(parent);

	sprintf(errorString, "%s/%s", nodeName, parameterName);

	if (verbose)
		printf("Reading %s values\n", parameterName);

	attributeNode = Util_XML_Get_Node(ATLANTIS_ATTRIBUTE, parent, parameterName);
	if (attributeNode == NULL)
		quit("%s attribute group not found.\n", errorString);

	/* Create a node for each functional group*/
	for (guild = 0; guild < bm->K_num_tot_sp; guild++) {
		//if (FunctGroupArray[guild].isVertebrate == TRUE) {
        if((FunctGroupArray[guild].groupAgeType == AGE_STRUCTURED) || (FunctGroupArray[guild].groupAgeType == AGE_STRUCTURED_BIOMASS)){

			if (Util_XML_Read_Array_Double(ATLANTIS_GROUP_ATTRIBUTE, fileName, errorString,  attributeNode, checkType, FunctGroupArray[guild].groupCode, &values, size)
					== FALSE) {
				quit("Error: Unable to find parameter %s/%s in input file %s\n", parameterName, FunctGroupArray[guild].groupCode, fileName);
			}

            for (b = 0; b < size; b++) {
				array[b][guild] = values[b];
            }
                
			free(values);
		}
	}
	free(nodeName);
}

/**
 *	\brief Read in data
 *
 *	The data is stored in the second dimension of the given array.
 *
 *
 */
static void Read_Vert_FSPBXML(MSEBoxModel *bm, char *fileName, xmlNodePtr parent){

	double *values = 0;
	double final_val, stdev_map_param, sp_std_dev;
	int guild, b, cohort, ngene;
	xmlNodePtr attributeNode;
	char errorString[STRLEN];
	char *nodeName;

	nodeName =  Util_Get_Node_Name(parent);
	sprintf(errorString, "%s/SpawningProportion", nodeName);

	if (verbose)
		printf("Reading SpawningProportion values\n");

	attributeNode = Util_XML_Get_Node(ATLANTIS_ATTRIBUTE, parent, "SpawningProportion");
	if (attributeNode == NULL)
		quit("SpawningProportion attribute group not found.\n");

	/* Read in the value for each vertebrate */
	for (guild = 0; guild < bm->K_num_tot_sp; guild++) {
		//if (FunctGroupArray[guild].isVertebrate == TRUE) 
		  if (FunctGroupArray[guild].groupAgeType == AGE_STRUCTURED_BIOMASS
				|| FunctGroupArray[guild].groupAgeType == AGE_STRUCTURED) {

			if (Util_XML_Read_Array_Double(ATLANTIS_GROUP_ATTRIBUTE, fileName, errorString, attributeNode, proportion_check, FunctGroupArray[guild].groupCode, &values, FunctGroupArray[guild].numCohorts)
					== FALSE) {
				quit("Error: Unable to find parameter SpawningProportion/%s in input file %s\n", FunctGroupArray[guild].groupCode, fileName);
			}

			for (cohort = 0; cohort < FunctGroupArray[guild].numCohortsXnumGenes; cohort++) {
				b = (int)floor(cohort / FunctGroupArray[guild].numGeneTypes);
				ngene = cohort - (b * FunctGroupArray[guild].numGeneTypes);
				final_val = values[b];
				if((bm->flag_do_evolution || bm->flag_mult_grow_curves) && (DNA[guild].trait_active[reproduce_trait_id] > 0)){
					stdev_map_param = DNA[guild].stdev_gene_map[ngene];
					sp_std_dev = FunctGroupArray[guild].speciesParams[trait_variance_id];
					final_val = values[b] + sp_std_dev * stdev_map_param * values[b];
				}
				FunctGroupArray[guild].FSPB[cohort] = final_val;
			}
			free(values);
		}
	}
	free(nodeName);
}

/**
 * \brief Read in the physical limitation input parameters.
 *
 */
static void Read_Setup_Values(MSEBoxModel *bm, char *fileName, xmlNodePtr rootnode) {

	xmlNodePtr attributeGroupNode;
	attributeGroupNode = Util_XML_Get_Node(ATLANTIS_ATTRIBUTE_SUB_GROUP, rootnode, "Setup");
	if (attributeGroupNode == NULL)
		quit("Setup attribute group not found in input file %s.\n", fileName);

    bm->ecotest = (int) Util_XML_Read_Value(fileName, ATLANTIS_ATTRIBUTE, bm->ecotest, 1, attributeGroupNode, no_checking, "ecotest");
	bm->do_availflag = (int) Util_XML_Read_Value(fileName, ATLANTIS_ATTRIBUTE, bm->ecotest, 1, attributeGroupNode, binary_check, "availflag");
	bm->flagrandom = (int) Util_XML_Read_Value(fileName, ATLANTIS_ATTRIBUTE, bm->ecotest, 1, attributeGroupNode, binary_check, "flagrandom");
    
    readin_popratio = (int) Util_XML_Read_Value(fileName, ATLANTIS_ATTRIBUTE, bm->ecotest, 1, attributeGroupNode, no_checking, "readin_popratio");
	flagresp = (int) Util_XML_Read_Value(fileName, ATLANTIS_ATTRIBUTE, bm->ecotest, 1, attributeGroupNode, integer_check, "flagresp");
	bm->flagavgmig = (int) Util_XML_Read_Value(fileName, ATLANTIS_ATTRIBUTE, bm->ecotest, 1, attributeGroupNode, binary_check, "flagavgmig");
	flagbactstim = (int) Util_XML_Read_Value(fileName, ATLANTIS_ATTRIBUTE, bm->ecotest, 1, attributeGroupNode, binary_check, "flagbactstim");
}

static void Read_Group_Flags(MSEBoxModel *bm, char *fileName, xmlNodePtr parent) {

	int sp;
	//int index, guild, value;
	//int shouldQuit = 0;
	xmlNodePtr attributeGroupNode;
	//xmlNodePtr attributeGroup;

	attributeGroupNode = Util_XML_Get_Node(ATLANTIS_ATTRIBUTE_SUB_GROUP, parent, "FunctionalGroupFlags");
	if (attributeGroupNode == NULL)
		quit("FunctionalGroupFlags attribute group not found in input file %s.\n", fileName);

//	for (guild = 0; guild < bm->K_num_tot_sp; guild++)
//		if (FunctGroupArray[guild].isVertebrate == FALSE)
//			FunctGroupArray[guild].flagLim = FALSE;

//	/* Read in the population limitation flag */
//	attributeNode = Util_XML_Get_Node(ATLANTIS_ATTRIBUTE, attributeGroupNode, "flaglim");
//	if (attributeNode == NULL)
//		quit("FunctionalGroupFlags/flaglim attribute group not found in input file %s.\n", fileName);
//
//	for (guild = 0; guild < bm->K_num_tot_sp; guild++) {
//		if (FunctGroupArray[guild].isVertebrate == FALSE) {
//			FunctGroupArray[guild].flagLim = (int) Util_XML_Read_Value(fileName, ATLANTIS_GROUP_ATTRIBUTE, bm->ecotest, 1, attributeNode, binary_check,
//					FunctGroupArray[guild].groupCode);
//		}
//	}

	/* Do a check that we have the correct groups active */

	/*
	index = Util_XML_Get_Param_Index(speciesParamStructArray, tot_prms, flag_id);

	if (verbose > 1)
		printf("Read species parameters %s\n", speciesParamStructArray[index].tag);

	attributeGroup = Util_XML_Get_Node(ATLANTIS_ATTRIBUTE, attributeGroupNode, speciesParamStructArray[index].tag);
	if (attributeGroup == NULL)
		warn("%s/%s attribute group not found in file %s.\n", parent->name, speciesParamStructArray[index].tag, fileName);
	else{

		for (guild = 0; guild < bm->K_num_tot_sp; guild++){
			value = Util_XML_Get_Species_Node_Value(attributeGroup, index, guild, speciesParamStructArray);
			if(value != FunctGroupArray[guild].speciesParams[flag_id]){
				warn("Group %s isTurnedOn should be %d\n", FunctGroupArray[guild].groupCode, value);
				shouldQuit = 1;
			}
		}

		if(shouldQuit)
			quit("Groups are on/off when they shouldn't be.");
	}
	*/

	//Util_XML_Read_Species_Param(bm, fileName, attributeGroupNode, flag_id);
	Util_XML_Read_Species_Param(bm, fileName, attributeGroupNode, flag_lim_id);
	/* Check that this is only read in for suitable groups */

	for(sp = 0; sp < bm->K_num_tot_sp; sp++){
		if(FunctGroupArray[sp].groupType == SED_EP_OTHER &&
			FunctGroupArray[sp].speciesParams[flag_lim_id] == TRUE && FunctGroupArray[sp].isCover == TRUE){
			quit("Group %s, You cannot have flag_lim set to true for SED_EP_OTHER cover groups. \n", FunctGroupArray[sp].groupCode);
		}
	}


	Util_XML_Read_Species_Param(bm, fileName, attributeGroupNode, flagdem_id);
	Util_XML_Read_Species_Param(bm, fileName, attributeGroupNode, flagplankfish_id);
	Util_XML_Read_Species_Param(bm, fileName, attributeGroupNode, flux_thresh_id);
	Util_XML_Read_Species_Param(bm, fileName, attributeGroupNode, flux_damp_id);

}

/**
 * \brief Read in the physical limitation input parameters.
 *
 */
static void Read_Physical_Limitation(MSEBoxModel *bm, char *fileName, xmlNodePtr rootnode) {

	int sp;
	xmlNodePtr attributeGroupNode;

	printf("Read_Physical_Limitation\n");

	attributeGroupNode = Util_XML_Get_Node(ATLANTIS_ATTRIBUTE_SUB_GROUP, rootnode, "PhysicalLimitation");
	if (attributeGroupNode == NULL)
		quit("PhysicalLimitation attribute group not found in input file %s.\n", fileName);

	/* Those determining which temperature, light, oxygen and nutrient limitation
	 formulations are used */

	bm->flagtrackpops = (int) Util_XML_Read_Value(fileName, ATLANTIS_ATTRIBUTE, bm->ecotest, 1, attributeGroupNode, binary_check, "flagtrackpops");
	bm->flagseason = (int) Util_XML_Read_Value(fileName, ATLANTIS_ATTRIBUTE, bm->ecotest, 1, attributeGroupNode, binary_check, "flagseason");
	bm->flaglight = (int) Util_XML_Read_Value(fileName, ATLANTIS_ATTRIBUTE, bm->ecotest, 1, attributeGroupNode, binary_check, "flaglight");
	bm->flaglightopt = (int) Util_XML_Read_Value(fileName, ATLANTIS_ATTRIBUTE, bm->ecotest, 1, attributeGroupNode, integer_check, "flaglightopt");
	bm->lim_sun_hours = (int) Util_XML_Read_Value(fileName, ATLANTIS_ATTRIBUTE, bm->ecotest, 1, attributeGroupNode, integer_check, "lim_sun_hours");

	/* Add a check to make sure we have loaded solar radiation files if lim_sun_hours is false */
	if( bm->lim_sun_hours == 0 && bm->swr == NULL && bm->swrinput.nFiles == 0) {
		quit("ERROR - You have set lim_sun_hours in your biology file (%s) to 0 but you haven't provided a solar radiation file in your forcing file.\n", fileName);
	}

	bm->flagmodeltemp = (int) Util_XML_Read_Value(fileName, ATLANTIS_ATTRIBUTE, bm->ecotest, 1, attributeGroupNode, integer_check, "flagmodeltemp");
	bm->flagq10 = (int) Util_XML_Read_Value(fileName, ATLANTIS_ATTRIBUTE, bm->ecotest, 1, attributeGroupNode, binary_check, "flagq10");
	bm->O2case = (int) Util_XML_Read_Value(fileName, ATLANTIS_ATTRIBUTE, bm->ecotest, 1, attributeGroupNode, integer_check, "O2case");
	bm->flagnut = (int) Util_XML_Read_Value(fileName, ATLANTIS_ATTRIBUTE, bm->ecotest, 1, attributeGroupNode, integer_check, "flagnut");
	bm->flagmicro = (int) Util_XML_Read_Value(fileName, ATLANTIS_ATTRIBUTE, bm->ecotest, 1, attributeGroupNode, integer_check, "flagmicro");
	bm->flagtempchange = (int) Util_XML_Read_Value(fileName, ATLANTIS_ATTRIBUTE, bm->ecotest, 1, attributeGroupNode, binary_check, "flagtempchange");
	bm->flagsaltchange = (int) Util_XML_Read_Value(fileName, ATLANTIS_ATTRIBUTE, bm->ecotest, 1, attributeGroupNode, binary_check, "flagsaltchange");
	bm->flagpHchange = (int) Util_XML_Read_Value(fileName, ATLANTIS_ATTRIBUTE, bm->ecotest, 1, attributeGroupNode, binary_check, "flagpHchange");
	bm->flagstarve = (int) Util_XML_Read_Value(fileName, ATLANTIS_ATTRIBUTE, bm->ecotest, 1, attributeGroupNode, binary_check, "flagstarve");
	bm->flagdegrade = (int) Util_XML_Read_Value(fileName, ATLANTIS_ATTRIBUTE, bm->ecotest, 1, attributeGroupNode, binary_check, "flagdegrade");
	bm->flagroc = (int) Util_XML_Read_Value(fileName, ATLANTIS_ATTRIBUTE, bm->ecotest, 1, attributeGroupNode, integer_check, "flagroc");
	bm->flaghomog_sp = (int) Util_XML_Read_Value(fileName, ATLANTIS_ATTRIBUTE, bm->ecotest, 1, attributeGroupNode, binary_check, "flaghomog_sp");
	bm->flagagestruct = (int) Util_XML_Read_Value(fileName, ATLANTIS_ATTRIBUTE, bm->ecotest, 1, attributeGroupNode, binary_check, "flagagestruct");
	bm->flagsenesce = (int) Util_XML_Read_Value(fileName, ATLANTIS_ATTRIBUTE, bm->ecotest, 1, attributeGroupNode, binary_check, "flagsenesce");
	bm->flagtsforcerecruit = (int) Util_XML_Read_Value(fileName, ATLANTIS_ATTRIBUTE, bm->ecotest, 1, attributeGroupNode, binary_check, "flagtsforcerecruit");
    bm->flag_modify_KWSR = (int) Util_XML_Read_Value(fileName, ATLANTIS_ATTRIBUTE, bm->ecotest, 1, attributeGroupNode, binary_check, "flag_modify_KWSR");
    
    bm->flag_extpop_growth_option = 0;
    if(bm->external_populations)
        bm->flag_extpop_growth_option = (int) Util_XML_Read_Value(fileName, ATLANTIS_ATTRIBUTE, bm->ecotest, 1, attributeGroupNode, binary_check, "flag_extpop_growth_option");
    
    bm->juv_transition_thresh = Util_XML_Read_Value(fileName, ATLANTIS_ATTRIBUTE, bm->ecotest, 1, attributeGroupNode, no_checking, "juv_transition_thresh");
    bm->mat_transition_thresh = Util_XML_Read_Value(fileName, ATLANTIS_ATTRIBUTE, bm->ecotest, 1, attributeGroupNode, no_checking, "mat_transition_thresh");

    bm->norm_larval_distrib = (int) Util_XML_Read_Value(fileName, ATLANTIS_ATTRIBUTE, bm->ecotest, 1, attributeGroupNode, binary_check, "norm_larval_distrib");
    bm->larvae_connect_only = (int) Util_XML_Read_Value(fileName, ATLANTIS_ATTRIBUTE, bm->ecotest, 1, attributeGroupNode, binary_check, "larvae_connect_only");
    bm->enviro_independ_larvae = (int) Util_XML_Read_Value(fileName, ATLANTIS_ATTRIBUTE, bm->ecotest, 1, attributeGroupNode, binary_check, "enviro_independ_larvae");
    bm->flag_recruit_effect = (int) Util_XML_Read_Value(fileName, ATLANTIS_ATTRIBUTE, bm->ecotest, 1, attributeGroupNode, binary_check, "flag_recruit_effect");
    
    bm->flag_macro_model = (int) Util_XML_Read_Value(fileName, ATLANTIS_ATTRIBUTE, bm->ecotest, 1, attributeGroupNode, binary_check, "flag_macro_model");
	bm->flag_benthos_sediment_link = (int) Util_XML_Read_Value(fileName, ATLANTIS_ATTRIBUTE, bm->ecotest, 1, attributeGroupNode, binary_check, "flag_benthos_sediment_link");
    bm->flag_competing_epiff = (int) Util_XML_Read_Value(fileName, ATLANTIS_ATTRIBUTE, bm->ecotest, 1, attributeGroupNode, binary_check, "flag_competing_epiff");
    if(bm->flag_competing_epiff)
    	bm->max_available_habitat = Util_XML_Read_Value(fileName, ATLANTIS_ATTRIBUTE, bm->ecotest, 1, attributeGroupNode, no_checking, "max_available_habitat");

    bm->flag_invert_biohab = (int) Util_XML_Read_Value(fileName, ATLANTIS_ATTRIBUTE, bm->ecotest, 1, attributeGroupNode, binary_check, "flag_invert_biohab");
	bm->flag_olddiet = (int) Util_XML_Read_Value(fileName, ATLANTIS_ATTRIBUTE, bm->ecotest, 1, attributeGroupNode, binary_check, "flag_olddiet");
	bm->flag_fine_ontogenetic_diets = (int) Util_XML_Read_Value(fileName, ATLANTIS_ATTRIBUTE, bm->ecotest, 1, attributeGroupNode, binary_check, "flag_fine_ontogenetic_diets");
	bm->UseHardFeedingWindow = (int) Util_XML_Read_Value(fileName, ATLANTIS_ATTRIBUTE, bm->ecotest, 1, attributeGroupNode, binary_check, "UseHardFeedingWindow");
    bm->UseBiLogisticFeedingWindow = (int) Util_XML_Read_Value(fileName, ATLANTIS_ATTRIBUTE, bm->ecotest, 1, attributeGroupNode, binary_check, "UseBiLogisticFeedingWindow");
    bm->flag_satiation = (int) Util_XML_Read_Value(fileName, ATLANTIS_ATTRIBUTE, bm->ecotest, 1, attributeGroupNode, binary_check, "flag_satiation");
    bm->flag_shrinkfat = (int) Util_XML_Read_Value(fileName, ATLANTIS_ATTRIBUTE, bm->ecotest, 1, attributeGroupNode, binary_check, "flag_shrinkfat");
    bm->flag_predratiodepend = (int) Util_XML_Read_Value(fileName, ATLANTIS_ATTRIBUTE, bm->ecotest, 1, attributeGroupNode, binary_check, "flag_predratiodepend");
    bm->flag_dynamicXRS = (int) Util_XML_Read_Value(fileName, ATLANTIS_ATTRIBUTE, bm->ecotest, 1, attributeGroupNode, binary_check, "flag_dynamicXRS");
    if(bm->flag_dynamicXRS) {
        bm->flag_repcostSpawn = (int) Util_XML_Read_Value(fileName, ATLANTIS_ATTRIBUTE, bm->ecotest, 1, attributeGroupNode, binary_check, "flag_repcostSpawn");
        bm->flag_lengthSN = (int) Util_XML_Read_Value(fileName, ATLANTIS_ATTRIBUTE, bm->ecotest, 1, attributeGroupNode, binary_check, "flag_lengthSN");
        bm->XRS_cap = Util_XML_Read_Value(fileName, ATLANTIS_ATTRIBUTE, bm->ecotest, 1, attributeGroupNode, no_checking, "XRS_cap");
    }

	bm->flag_rel_cover = (int) Util_XML_Read_Value(fileName, ATLANTIS_ATTRIBUTE, bm->ecotest, 1, attributeGroupNode, binary_check, "flag_rel_cover");
	bm->flag_report_water_detritus = (int) Util_XML_Read_Value(fileName, ATLANTIS_ATTRIBUTE, bm->ecotest, 1, attributeGroupNode, binary_check, "flag_report_water_detritus");
	bm->flag_refuge_model = (int) Util_XML_Read_Value(fileName, ATLANTIS_ATTRIBUTE, bm->ecotest, 1, attributeGroupNode, no_checking, "flag_refuge_model");
	bm->flag_rugosity_model = (int) Util_XML_Read_Value(fileName, ATLANTIS_ATTRIBUTE, bm->ecotest, 1, attributeGroupNode, integer_check, "flag_rugosity_model");
    bm->flag_georugosity = (int) Util_XML_Read_Value(fileName, ATLANTIS_ATTRIBUTE, bm->ecotest, 1, attributeGroupNode, integer_check, "flag_georugosity");

	bm->RugCover_Coefft = Util_XML_Read_Value(fileName, ATLANTIS_ATTRIBUTE, bm->ecotest, 1, attributeGroupNode, no_checking, "RugCover_Coefft");
	bm->RugCover_Const = Util_XML_Read_Value(fileName, ATLANTIS_ATTRIBUTE, bm->ecotest, 1, attributeGroupNode, no_checking, "RugCover_Const");
	bm->RugCover_Cap = Util_XML_Read_Value(fileName, ATLANTIS_ATTRIBUTE, bm->ecotest, 1, attributeGroupNode, no_checking, "RugCover_Cap");

	bm->min_rugosity = Util_XML_Read_Value(fileName, ATLANTIS_ATTRIBUTE, bm->ecotest, 1, attributeGroupNode, no_checking, "min_rugosity");
	bm->max_rugosity = Util_XML_Read_Value(fileName, ATLANTIS_ATTRIBUTE, bm->ecotest, 1, attributeGroupNode, no_checking, "max_rugosity");
    bm->rugosity_const = Util_XML_Read_Value(fileName, ATLANTIS_ATTRIBUTE, bm->ecotest, 1, attributeGroupNode, no_checking, "rugosity_const");

    bm->rugosity_bozec_a = Util_XML_Read_Value(fileName, ATLANTIS_ATTRIBUTE, bm->ecotest, 1, attributeGroupNode, no_checking, "rugosity_bozec_a");
    bm->rugosity_bozec_b = Util_XML_Read_Value(fileName, ATLANTIS_ATTRIBUTE, bm->ecotest, 1, attributeGroupNode, no_checking, "rugosity_bozec_b");
    bm->rugosity_bozec_c = Util_XML_Read_Value(fileName, ATLANTIS_ATTRIBUTE, bm->ecotest, 1, attributeGroupNode, no_checking, "rugosity_bozec_c");
    bm->rugosity_bozec_d = Util_XML_Read_Value(fileName, ATLANTIS_ATTRIBUTE, bm->ecotest, 1, attributeGroupNode, no_checking, "rugosity_bozec_d");
    
	bm->flagmodelpH = (int) Util_XML_Read_Value(fileName, ATLANTIS_ATTRIBUTE, bm->ecotest, 1, attributeGroupNode, integer_check, "flagmodelpH");
	if(bm->flagmodelpH){
        //bm->pH_sensitivity_model = (int) Util_XML_Read_Value(fileName, ATLANTIS_ATTRIBUTE, bm->ecotest, 1, attributeGroupNode, integer_check, "pH_sensitivity_model"); Deprecated as now a species level parameter
        
        bm->flagPHmortcase = (int) Util_XML_Read_Value(fileName, ATLANTIS_ATTRIBUTE, bm->ecotest, 1, attributeGroupNode, integer_check, "flagPHmortcase");

        if (bm->flagmodelpH == ph_regression) {
            bm->pH_surface_depth = Util_XML_Read_Value(fileName, ATLANTIS_ATTRIBUTE, bm->ecotest, 1, attributeGroupNode, no_checking, "pH_surface_depth");
            bm->pH_mid_depth = Util_XML_Read_Value(fileName, ATLANTIS_ATTRIBUTE, bm->ecotest, 1, attributeGroupNode, no_checking, "pH_mid_depth");
            bm->pH_surface_coefft_T = Util_XML_Read_Value(fileName, ATLANTIS_ATTRIBUTE, bm->ecotest, 1, attributeGroupNode, no_checking, "pH_surface_coefft_T");
            bm->pH_surface_coefft_S = Util_XML_Read_Value(fileName, ATLANTIS_ATTRIBUTE, bm->ecotest, 1, attributeGroupNode, no_checking, "pH_surface_coefft_S");
            bm->pH_surface_coefft_O = Util_XML_Read_Value(fileName, ATLANTIS_ATTRIBUTE, bm->ecotest, 1, attributeGroupNode, no_checking, "pH_surface_coefft_O");
            bm->pH_surface_const = Util_XML_Read_Value(fileName, ATLANTIS_ATTRIBUTE, bm->ecotest, 1, attributeGroupNode, no_checking, "pH_surface_const");
            bm->pH_mid_coefft_T = Util_XML_Read_Value(fileName, ATLANTIS_ATTRIBUTE, bm->ecotest, 1, attributeGroupNode, no_checking, "pH_mid_coefft_T");
            bm->pH_mid_coefft_S = Util_XML_Read_Value(fileName, ATLANTIS_ATTRIBUTE, bm->ecotest, 1, attributeGroupNode, no_checking, "pH_mid_coefft_S");
            bm->pH_mid_coefft_O = Util_XML_Read_Value(fileName, ATLANTIS_ATTRIBUTE, bm->ecotest, 1, attributeGroupNode, no_checking, "pH_mid_coefft_O");
            bm->pH_mid_const = Util_XML_Read_Value(fileName, ATLANTIS_ATTRIBUTE, bm->ecotest, 1, attributeGroupNode, no_checking, "pH_mid_const");
            bm->pH_deep_coefft_T = Util_XML_Read_Value(fileName, ATLANTIS_ATTRIBUTE, bm->ecotest, 1, attributeGroupNode, no_checking, "pH_deep_coefft_T");
            bm->pH_deep_coefft_S = Util_XML_Read_Value(fileName, ATLANTIS_ATTRIBUTE, bm->ecotest, 1, attributeGroupNode, no_checking, "pH_deep_coefft_S");
            bm->pH_deep_coefft_O = Util_XML_Read_Value(fileName, ATLANTIS_ATTRIBUTE, bm->ecotest, 1, attributeGroupNode, no_checking, "pH_deep_coefft_O");
            bm->pH_deep_const = Util_XML_Read_Value(fileName, ATLANTIS_ATTRIBUTE, bm->ecotest, 1, attributeGroupNode, no_checking, "pH_deep_const");
        }
	}

	bm->flagmodelArag = (int) Util_XML_Read_Value(fileName, ATLANTIS_ATTRIBUTE, bm->ecotest, 1, attributeGroupNode, integer_check, "flagmodelArag");
	if(bm->containsCoral){
		bm->K_max_num_DHW = (int) Util_XML_Read_Value(fileName, ATLANTIS_ATTRIBUTE, bm->ecotest, 1, attributeGroupNode, integer_check, "K_max_num_DHW");
		bm->Karag_A = Util_XML_Read_Value(fileName, ATLANTIS_ATTRIBUTE, bm->ecotest, 1, attributeGroupNode, no_checking, "Karag_A");
		bm->Karag_B = Util_XML_Read_Value(fileName, ATLANTIS_ATTRIBUTE, bm->ecotest, 1, attributeGroupNode, no_checking, "Karag_B");
		bm->Karag_C = Util_XML_Read_Value(fileName, ATLANTIS_ATTRIBUTE, bm->ecotest, 1, attributeGroupNode, no_checking, "Karag_C");
		bm->Karag_pH = Util_XML_Read_Value(fileName, ATLANTIS_ATTRIBUTE, bm->ecotest, 1, attributeGroupNode, no_checking, "Karag_pH");
		bm->Kca_const = Util_XML_Read_Value(fileName, ATLANTIS_ATTRIBUTE, bm->ecotest, 1, attributeGroupNode, no_checking, "Kca_const");
		bm->K_Ks = Util_XML_Read_Value(fileName, ATLANTIS_ATTRIBUTE, bm->ecotest, 1, attributeGroupNode, no_checking, "K_Ks");
		bm->Ksmother_coefft = Util_XML_Read_Value(fileName, ATLANTIS_ATTRIBUTE, bm->ecotest, 1, attributeGroupNode, no_checking, "Ksmother_coefft");
		bm->Ksmother_const = Util_XML_Read_Value(fileName, ATLANTIS_ATTRIBUTE, bm->ecotest, 1, attributeGroupNode, no_checking, "Ksmother_const");

        if(!bm->track_rugosity_arag|| !bm->flagmodelArag)
            quit("You can not have CORAL functional groups but no rugosity and aragonite tracking - either reset the functional group to something else, like SED_EP_FF, OR set track_rugosity_arag to 1 in run.prm\n");
        
    }
    
    if(bm->track_atomic_ratio == TRUE){
        bm->Pads_r_t0 = Util_XML_Read_Value(fileName, ATLANTIS_ATTRIBUTE, bm->ecotest, 1, attributeGroupNode, no_checking, "Pads_r_t0");
        bm->Pads_K = Util_XML_Read_Value(fileName, ATLANTIS_ATTRIBUTE, bm->ecotest, 1, attributeGroupNode, no_checking, "Pads_K");
        bm->Pads_KO = Util_XML_Read_Value(fileName, ATLANTIS_ATTRIBUTE, bm->ecotest, 1, attributeGroupNode, no_checking, "Pads_KO");
        bm->r_immob_PIP_t0 = Util_XML_Read_Value(fileName, ATLANTIS_ATTRIBUTE, bm->ecotest, 1, attributeGroupNode, no_checking, "r_immob_PIP_t0");
    }

	/*** Do checking ***/
	/* Check that the flag_macro-model value is ok*/
	for(sp = 0; sp < bm->K_num_tot_sp; sp++){
		if(FunctGroupArray[sp].groupType == SEAGRASS && FunctGroupArray[sp].numCohorts > 1){
			if(bm->flag_macro_model == FALSE){
				quit("ERROR: You cannot have age structured seagrass with the flag_macro_model turned off in your run.prm file.\n\n");
			}
		}
		if(bm->flag_macro_model == TRUE){
			FunctGroupArray[sp].pSP_SG_eat = Util_Alloc_Init_1D_Double(3, 0.0);
		}
	}

	if ((bm->flagmodeltemp == 1) && (bm->flaghemisphere == 1))
		warn("Regression temperature model only works in southern hemisphere.  You have set flagmodeltemp = 1.\n");


	if(bm->tempinput.nFiles > 0 && bm->flagmodeltemp){
		quit("You cannot provide temperature input files and use temperature regression (flagmodeltemp = 1 in your biology file)\n");
	}
}

static void Read_Physical_Chem_Values(MSEBoxModel *bm, char *fileName, xmlNodePtr rootnode) {
	double numsec = 86400.0; /* number of seconds in a day */
	int counter;
	double nset, base, diff;

	/** Biophysical constants ************/
	xmlNodePtr attributeGroupNode, attributeGroup;
	attributeGroupNode = Util_XML_Get_Node(ATLANTIS_ATTRIBUTE_SUB_GROUP, rootnode, "PhysicalChemical");
	if (attributeGroupNode == NULL)
		quit("Chemical_Limitation attribute group not found in input file %s.\n", fileName);

	/* Bioturbation */
	attributeGroup = Util_XML_Get_Node(ATLANTIS_ATTRIBUTE_SUB_GROUP, attributeGroupNode, "Bioturbation");
	if (attributeGroup == NULL)
		quit("Bioturbation attribute group not found in input file %s.\n", fileName);

	Enviro_turb = Util_XML_Read_Value(fileName, ATLANTIS_ATTRIBUTE, bm->ecotest, 1, attributeGroup, no_checking, "Enviro_turb") / numsec;
	K_TUR = Util_XML_Read_Value(fileName, ATLANTIS_ATTRIBUTE, bm->ecotest, 1, attributeGroup, no_checking, "K_TUR") / numsec;
	K_TUR_DEP = Util_XML_Read_Value(fileName, ATLANTIS_ATTRIBUTE, bm->ecotest, 1, attributeGroup, no_checking, "K_TUR_DEP");
	K_MAX_TUR = Util_XML_Read_Value(fileName, ATLANTIS_ATTRIBUTE, bm->ecotest, 1, attributeGroup, no_checking, "K_MAX_TUR");

	/* Bioirrigation */
	attributeGroup = Util_XML_Get_Node(ATLANTIS_ATTRIBUTE_SUB_GROUP, attributeGroupNode, "Bioirrigation");
	if (attributeGroup == NULL)
		quit("Bioirrigation attribute group not found in input file %s.\n", fileName);

	K_IRR = Util_XML_Read_Value(fileName, ATLANTIS_ATTRIBUTE, bm->ecotest, 1, attributeGroup, no_checking, "K_IRR") / numsec;
	K_MAX_IRR = Util_XML_Read_Value(fileName, ATLANTIS_ATTRIBUTE, bm->ecotest, 1, attributeGroup, no_checking, "K_MAX_IRR");
	K_MIN_IRR = Util_XML_Read_Value(fileName, ATLANTIS_ATTRIBUTE, bm->ecotest, 1, attributeGroup, no_checking, "K_MIN_IRR");

	/* Detrital breakdown */
	attributeGroup = Util_XML_Get_Node(ATLANTIS_ATTRIBUTE_SUB_GROUP, attributeGroupNode, "DetritusBreakdown");
	if (attributeGroup == NULL)
		quit("DetritusBreakdown attribute group not found in input file %s.\n", fileName);

	r_DC_T15 = Util_XML_Read_Value(fileName, ATLANTIS_ATTRIBUTE, bm->ecotest, 1, attributeGroup, no_checking, "r_DC_T15") / numsec;
	r_DL_T15 = Util_XML_Read_Value(fileName, ATLANTIS_ATTRIBUTE, bm->ecotest, 1, attributeGroup, no_checking, "r_DL_T15") / numsec;
	r_DR_T15 = Util_XML_Read_Value(fileName, ATLANTIS_ATTRIBUTE, bm->ecotest, 1, attributeGroup, no_checking, "r_DR_T15") / numsec;
	r_DON_T15 = Util_XML_Read_Value(fileName, ATLANTIS_ATTRIBUTE, bm->ecotest, 1, attributeGroup, no_checking, "r_DON_T15") / numsec;
	r_DSi_T15 = Util_XML_Read_Value(fileName, ATLANTIS_ATTRIBUTE, bm->ecotest, 1, attributeGroup, no_checking, "r_DSi_T15") / numsec;

	FDR_DC = Util_XML_Read_Value(fileName, ATLANTIS_ATTRIBUTE, bm->ecotest, 1, attributeGroup, no_checking, "FDR_DC");
	FDR_DL = Util_XML_Read_Value(fileName, ATLANTIS_ATTRIBUTE, bm->ecotest, 1, attributeGroup, no_checking, "FDR_DL");
	FDON_D = Util_XML_Read_Value(fileName, ATLANTIS_ATTRIBUTE, bm->ecotest, 1, attributeGroup, no_checking, "FDON_D");

	/* Nitrification and denitrification */
	attributeGroup = Util_XML_Get_Node(ATLANTIS_ATTRIBUTE_SUB_GROUP, attributeGroupNode, "Nitrification_And_Denitrification");
	if (attributeGroup == NULL)
		quit("Nitrification_And_Denitrification attribute group not found in input file %s.\n", fileName);

	R_0_T15 = Util_XML_Read_Value(fileName, ATLANTIS_ATTRIBUTE, bm->ecotest, 1, attributeGroup, no_checking, "R_0_T15") / numsec;
	R_D_T15 = Util_XML_Read_Value(fileName, ATLANTIS_ATTRIBUTE, bm->ecotest, 1, attributeGroup, no_checking, "R_D_T15") / numsec;

	Dmax = Util_XML_Read_Value(fileName, ATLANTIS_ATTRIBUTE, bm->ecotest, 1, attributeGroup, no_checking, "Dmax");

	K_nit_T15 = Util_XML_Read_Value(fileName, ATLANTIS_ATTRIBUTE, bm->ecotest, 1, attributeGroup, no_checking, "K_nit_T15") / numsec;
	K_conc = Util_XML_Read_Value(fileName, ATLANTIS_ATTRIBUTE, bm->ecotest, 1, attributeGroup, no_checking, "K_conc");

	/* Adsorption */
	p_NH_anad = Util_XML_Read_Value(fileName, ATLANTIS_ATTRIBUTE, bm->ecotest, 1, attributeGroup, no_checking, "p_NH_anad");

	/* Redfield */
	attributeGroup = Util_XML_Get_Node(ATLANTIS_ATTRIBUTE_SUB_GROUP, attributeGroupNode, "Redfield");
	if (attributeGroup == NULL)
		quit("Redfield attribute group not found in input file %s.\n", fileName);

	X_ON = Util_XML_Read_Value(fileName, ATLANTIS_ATTRIBUTE, bm->ecotest, 1, attributeGroup, no_checking, "X_ON");
	bm->X_CN = Util_XML_Read_Value(fileName, ATLANTIS_ATTRIBUTE, bm->ecotest, 1, attributeGroup, no_checking, "X_CN");
	X_CHLN = Util_XML_Read_Value(fileName, ATLANTIS_ATTRIBUTE, bm->ecotest, 1, attributeGroup, no_checking, "X_CHLN");
	X_SiN = Util_XML_Read_Value(fileName, ATLANTIS_ATTRIBUTE, bm->ecotest, 1, attributeGroup, no_checking, "X_SiN");
	X_FeN = Util_XML_Read_Value(fileName, ATLANTIS_ATTRIBUTE, bm->ecotest, 1, attributeGroup, no_checking, "X_FeN");

    bm->k_wetdry = Util_XML_Read_Value(fileName, ATLANTIS_ATTRIBUTE, bm->ecotest, 1, attributeGroupNode, no_checking, "k_wetdry");

	/* Light penetration through the water column. Note that the coastal-ocean split
	 depth is multipled by -1 as depths must be negative and program reads in positives */
	attributeGroup = Util_XML_Get_Node(ATLANTIS_ATTRIBUTE_SUB_GROUP, attributeGroupNode, "Light_Penetration");
	if (attributeGroup == NULL)
		quit("Light_Penetration attribute group not found in input file %s.\n", fileName);

	k_w_cdepth = -1.0 * Util_XML_Read_Value(fileName, ATLANTIS_ATTRIBUTE, bm->ecotest, 1, attributeGroup, no_checking, "k_w_cdepth");
	k_w_depth = -1.0 * Util_XML_Read_Value(fileName, ATLANTIS_ATTRIBUTE, bm->ecotest, 1, attributeGroup, no_checking, "k_w_depth");
	k_w_deep = Util_XML_Read_Value(fileName, ATLANTIS_ATTRIBUTE, bm->ecotest, 1, attributeGroup, no_checking, "k_w_deep");
	k_w_shallow = Util_XML_Read_Value(fileName, ATLANTIS_ATTRIBUTE, bm->ecotest, 1, attributeGroup, no_checking, "k_w_shallow");
	k_PN = Util_XML_Read_Value(fileName, ATLANTIS_ATTRIBUTE, bm->ecotest, 1, attributeGroup, no_checking, "k_PN");
	k_DON = Util_XML_Read_Value(fileName, ATLANTIS_ATTRIBUTE, bm->ecotest, 1, attributeGroup, no_checking, "k_DON");
	k_DL = Util_XML_Read_Value(fileName, ATLANTIS_ATTRIBUTE, bm->ecotest, 1, attributeGroup, no_checking, "k_DL");
	k_IS = Util_XML_Read_Value(fileName, ATLANTIS_ATTRIBUTE, bm->ecotest, 1, attributeGroup, no_checking, "k_IS");
	k_SED = Util_XML_Read_Value(fileName, ATLANTIS_ATTRIBUTE, bm->ecotest, 1, attributeGroup, no_checking, "k_SED");
	KIOP_min = Util_XML_Read_Value(fileName, ATLANTIS_ATTRIBUTE, bm->ecotest, 1, attributeGroup, no_checking, "KIOP_min");
	KIOP_shift = Util_XML_Read_Value(fileName, ATLANTIS_ATTRIBUTE, bm->ecotest, 1, attributeGroup, no_checking, "KIOP_shift") / numsec;
	KI_avail = Util_XML_Read_Value(fileName, ATLANTIS_ATTRIBUTE, bm->ecotest, 1, attributeGroup, no_checking, "KI_avail");
	K_addepth = Util_XML_Read_Value(fileName, ATLANTIS_ATTRIBUTE, bm->ecotest, 1, attributeGroup, no_checking, "K_addepth");
	bm->swr_scalar = Util_XML_Read_Value(fileName, ATLANTIS_ATTRIBUTE, bm->ecotest, 1, attributeGroup, no_checking, "swr_scalar");
    bm->swr_const = Util_XML_Read_Value(fileName, ATLANTIS_ATTRIBUTE, bm->ecotest, 1, attributeGroup, no_checking, "swr_const");
    bm->swr_cos_coefft = Util_XML_Read_Value(fileName, ATLANTIS_ATTRIBUTE, bm->ecotest, 1, attributeGroup, no_checking, "swr_cos_coefft");
    bm->swr_cos_offset = Util_XML_Read_Value(fileName, ATLANTIS_ATTRIBUTE, bm->ecotest, 1, attributeGroup, no_checking, "swr_cos_offset");

	/* Set up vectors of change */
	/* Ice related light parameters */
	if (bm->ice_on) {
		attributeGroup = Util_XML_Get_Node(ATLANTIS_ATTRIBUTE_SUB_GROUP, attributeGroupNode, "Ice_Light_Penetration");
		if (attributeGroup == NULL)
			quit("Ice_Light_Penetration attribute group not found in input file %s.\n", fileName);
		albedo_ice = Util_XML_Read_Value(fileName, ATLANTIS_ATTRIBUTE, bm->ecotest, 1, attributeGroup, no_checking, "albedo_ice");
		k_bs = Util_XML_Read_Value(fileName, ATLANTIS_ATTRIBUTE, bm->ecotest, 1, attributeGroup, no_checking, "k_bs");
		k_bi = Util_XML_Read_Value(fileName, ATLANTIS_ATTRIBUTE, bm->ecotest, 1, attributeGroup, no_checking, "k_bi");
		k_rs = Util_XML_Read_Value(fileName, ATLANTIS_ATTRIBUTE, bm->ecotest, 1, attributeGroup, no_checking, "k_rs");
		k_ri = Util_XML_Read_Value(fileName, ATLANTIS_ATTRIBUTE, bm->ecotest, 1, attributeGroup, no_checking, "k_ri");
		R_bi = Util_XML_Read_Value(fileName, ATLANTIS_ATTRIBUTE, bm->ecotest, 1, attributeGroup, no_checking, "R_bi");

		k_ice = Util_XML_Read_Value(fileName, ATLANTIS_ATTRIBUTE, bm->ecotest, 1, attributeGroup, no_checking, "k_ice");
		ka_star = Util_XML_Read_Value(fileName, ATLANTIS_ATTRIBUTE, bm->ecotest, 1, attributeGroup, no_checking, "ka_star");
	}

	/* Temperature effects */
	attributeGroup = Util_XML_Get_Node(ATLANTIS_ATTRIBUTE_SUB_GROUP, attributeGroupNode, "Temperature_Effects");
	if (attributeGroup == NULL)
		quit("Physical_Change attribute group not found in input file %s.\n", fileName);

	if(bm->flagtempchange){		
		Tchange_max_num = (int) Util_XML_Read_Value(fileName, ATLANTIS_ATTRIBUTE, bm->ecotest, 1, attributeGroup, no_checking, "Tchange_max_num");
		Vchange_max_num = Tchange_max_num;

		/* We can now allocate the Tchange array */
		Tchange = (double **) alloc2d(3, Tchange_max_num);
		Read_Change_DataXML(bm, fileName, attributeGroup, "Tchange", Tchange, Tchange_max_num, no_checking);
        
        printf("Created Tchange\n");

		//TODO: Check this - we might need to quit if there are not the right number of entries. It might be safer.
		counter = Tchange_max_num * bm->wcnz;
		if (Util_XML_Read_Array_Double(ATLANTIS_ATTRIBUTE, fileName, "PhysicalChemical/Temperature_Effects", attributeGroup, no_checking, "vertTchange_mult", &vertTchange_multi, Tchange_max_num
				* bm->wcnz) == FALSE) {

			quit("Error: Unable to find parameter 'PhysicalChemical/Salinity_Effects/vertTchange_mult' in input file %s\n", fileName);
		}
		base = Tchange_max_num * bm->wcnz;
		nset = counter / 1.0;
		diff = fabs(base - nset);
		if (diff > 0) {
			quit("Incomplete set of entries for vertTchange_mult (says there are %e entries instead of %e = %d * %d (Vchange_num * wcnz)\n", nset, base,
					Tchange_max_num, bm->wcnz);
		}
	}else{
		Tchange_max_num = 0;
	}
	Vchange_max_num = Tchange_max_num;

	/* Salinity effects */
	if(bm->flagsaltchange){
		attributeGroup = Util_XML_Get_Node(ATLANTIS_ATTRIBUTE_SUB_GROUP, attributeGroupNode, "Salinity_Effects");
		if (attributeGroup == NULL)
			quit("Salinity_Effects attribute group not found in input file %s.\n", fileName);

		Schange_max_num = (int) Util_XML_Read_Value(fileName, ATLANTIS_ATTRIBUTE, bm->ecotest, 1, attributeGroup, no_checking, "Schange_max_num");

		if(Schange_max_num > 0)
			Schange = (double **) alloc2d(3, Schange_max_num);
		Read_Change_DataXML(bm, fileName, attributeGroup, "Schange", Schange, Schange_max_num, no_checking);

		counter = Schange_max_num * bm->wcnz;

		if (Util_XML_Read_Array_Double(ATLANTIS_ATTRIBUTE, fileName, "Salinity_Effects", attributeGroup, no_checking, "vertSchange_mult",
				&vertSchange_multi, Schange_max_num * bm->wcnz) == FALSE) {
			quit("Error: Unable to find parameter 'PhysicalChemical/Salinity_Effects/vertSchange_mult' in input file %s\n", fileName);
		}
		nset = counter / bm->wcnz;
		base = floor(nset);
		diff = fabs(base - nset);
		if (diff > 0) {
			quit("Incomplete set of entries for vertSchange_mult (says there are %e entries instead of %d = %d * %d (Schange_num * wcnz)\n", nset, Schange_max_num
					* bm->wcnz, Schange_max_num, bm->wcnz);
		}
	}else{
		Schange_max_num = 0;
	}
	Vchange_max_num = max(Tchange_max_num, Schange_max_num);

	/* pH effects */
	if(bm->flagpHchange){
		attributeGroup = Util_XML_Get_Node(ATLANTIS_ATTRIBUTE_SUB_GROUP, attributeGroupNode, "pH_Effects");
		if (attributeGroup == NULL)
			quit("pH_Effects attribute group not found in input file %s.\n", fileName);

		PHchange_max_num = (int) Util_XML_Read_Value(fileName, ATLANTIS_ATTRIBUTE, bm->ecotest, 1, attributeGroup, no_checking, "pHchange_max_num");

		if(PHchange_max_num > 0)
			PHchange = (double **) alloc2d(3, PHchange_max_num);
		Read_Change_DataXML(bm, fileName, attributeGroup, "PHchange", PHchange, PHchange_max_num, no_checking);

		counter = PHchange_max_num * bm->wcnz;

		if (Util_XML_Read_Array_Double(ATLANTIS_ATTRIBUTE, fileName, "pH_Effects", attributeGroup, no_checking, "vertPHchange_mult", &vertPHchange_multi, PHchange_max_num
					* bm->wcnz) == FALSE) {
			quit("Error: Unable to find parameter 'PhysicalChemical/Salinity_Effects/vertPHchange_mult' in input file %s\n", fileName);
		}
		nset = counter / bm->wcnz;
		base = floor(nset);
		diff = fabs(base - nset);
		if (diff > 0) {
			quit("Incomplete set of entries for vertPHchange_mult (says there are %e entries instead of %d = %d * %d (PHchange_num * wcnz)\n", nset, PHchange_max_num
					* bm->wcnz, PHchange_max_num, bm->wcnz);
		}
	}else{
		PHchange_max_num = 0;
	}
	Vchange_max_num = max(Vchange_max_num, PHchange_max_num);

}


/**
 * Read in information about additional nutrient tracers we are tracking.
 *
 *
 */
static void Read_Additional_Tracer_Values(MSEBoxModel *bm, char *fileName, xmlNodePtr parent) {
	xmlNodePtr attributeGroupNode;

	attributeGroupNode = Util_XML_Get_Node(ATLANTIS_ATTRIBUTE_SUB_GROUP, parent, "AdditionalNutrients");
	if (attributeGroupNode == NULL)
		quit("AdditionalNutrients attribute group not found in input file %s.\n", fileName);

	bm->atomicRatioInfo->RTOP = Util_XML_Read_Value(fileName, ATLANTIS_ATTRIBUTE, bm->ecotest, 1, attributeGroupNode, no_checking, "RTOP");

	/* If we are tracking P then read in P primary producer values */
	Util_XML_Read_Species_Param(bm, fileName, attributeGroupNode, P_max_uptake_id);
	Util_XML_Read_Species_Param(bm, fileName, attributeGroupNode, P_uptake_scale_id);
	Util_XML_Read_Species_Param(bm, fileName, attributeGroupNode, P_concp_id);
	Util_XML_Read_Species_Param(bm, fileName, attributeGroupNode, Psa_min_id);
	Util_XML_Read_Species_Param(bm, fileName, attributeGroupNode, phyto_resp_rate_id);

	Util_XML_Read_Species_Param(bm, fileName, attributeGroupNode, P_min_internal_id);
	Util_XML_Read_Species_Param(bm, fileName, attributeGroupNode, P_max_internal_id);

	Util_XML_Read_Species_Param(bm, fileName, attributeGroupNode, C_max_uptake_id);
	Util_XML_Read_Species_Param(bm, fileName, attributeGroupNode, C_uptake_scale_id);
	Util_XML_Read_Species_Param(bm, fileName, attributeGroupNode, C_concp_id);
	Util_XML_Read_Species_Param(bm, fileName, attributeGroupNode, C_min_id);

	Util_XML_Read_Species_Param(bm, fileName, attributeGroupNode, KP_id);

	bm->atomicRatioInfo->K_Lc = Util_XML_Read_Value(fileName, ATLANTIS_ATTRIBUTE, bm->ecotest, 1, attributeGroupNode, no_checking, "K_Lc");
}

static void Read_Tolerance_Values(MSEBoxModel *bm, char *fileName, xmlNodePtr parent) {
	xmlNodePtr attributeGroupNode;

	attributeGroupNode = Util_XML_Get_Node(ATLANTIS_ATTRIBUTE_SUB_GROUP, parent, "Tolerance");
	if (attributeGroupNode == NULL)
		quit("Tolerance attribute group not found in input file %s.\n", fileName);

	/* Tolerances and boundaries */
	RelTol = Util_XML_Read_Value(fileName, ATLANTIS_ATTRIBUTE, bm->ecotest, 1, attributeGroupNode, no_checking, "RelTol");
	Flux_tol = Util_XML_Read_Value(fileName, ATLANTIS_ATTRIBUTE, bm->ecotest, 1, attributeGroupNode, no_checking, "Flux_tol");
	bm->min_pool = Util_XML_Read_Value(fileName, ATLANTIS_ATTRIBUTE, bm->ecotest, 1, attributeGroupNode, no_checking, "min_pool");
	bm->min_dens = Util_XML_Read_Value(fileName, ATLANTIS_ATTRIBUTE, bm->ecotest, 1, attributeGroupNode, no_checking, "min_dens");
	bm->min_channel_depth = Util_XML_Read_Value(fileName, ATLANTIS_ATTRIBUTE, bm->ecotest, 1, attributeGroupNode, no_checking, "min_channel_depth");
}

/**
 * Read in information relevant to evolution
 *
 *
 */
static void Read_Active_Trait_XML(MSEBoxModel *bm, char *fileName, xmlNodePtr parent) {

	double *values = 0;
	int guild, ntrait;
	xmlNodePtr attributeNode;

	if (verbose)
		printf("Reading Active Trait values\n");

	attributeNode = Util_XML_Get_Node(ATLANTIS_ATTRIBUTE, parent, "ActiveTrait");
	if (attributeNode == NULL)
		quit("Evolution/ActiveTrait attribute group not found in input file %s.\n", fileName);

	/* Create a node for each functional group*/
	for (guild = 0; guild < bm->K_num_tot_sp; guild++) {
		if (Util_XML_Read_Array_Double(ATLANTIS_GROUP_ATTRIBUTE, fileName, "Evolution/ActiveTrait", attributeNode, no_checking, FunctGroupArray[guild].groupCode, &values,
				K_num_traits) == FALSE) {
			quit("Error: Unable to find parameter 'Evolution/ActiveTrait/%s' in input file %s\n", FunctGroupArray[guild].groupCode, fileName);
		}

		for (ntrait = 0; ntrait < K_num_traits; ntrait++) {
			DNA[guild].trait_active[ntrait] = (int)(values[ntrait]);
		}
		free(values);
	}
}

static void Calculate_stdev_gene_map(MSEBoxModel *bm, FILE *llogfp) {
	int sp, ngene, midgene;
	double diff_gene, mgene, range_gene, gene_per_std_dev;

	for (sp = 0; sp < bm->K_num_tot_sp; sp++) {
		midgene = FunctGroupArray[sp].numGeneTypes / 2;
		mgene = (double)(midgene);
		range_gene = FunctGroupArray[sp].numGeneTypes - mgene;
		gene_per_std_dev = bm->evol_stdev_range / range_gene;

		//printf("Doing stdev_gene_map for %s\n", FunctGroupArray[sp].groupCode);

		for(ngene=0; ngene < FunctGroupArray[sp].numGeneTypes; ngene++){
			diff_gene = (double)(ngene - midgene);
			DNA[sp].stdev_gene_map[ngene] = diff_gene * gene_per_std_dev;
		}
	}

}

static void Read_Evolution_Values(MSEBoxModel *bm, char *fileName, xmlNodePtr parent, FILE *llogfp) {
	xmlNodePtr attributeGroupNode;

	attributeGroupNode = Util_XML_Get_Node(ATLANTIS_ATTRIBUTE_SUB_GROUP, parent, "Evolution");
	if (attributeGroupNode == NULL)
		quit("Evolution attribute group not found in input file %s.\n", fileName);

    bm->flag_do_var_express = (int)Util_XML_Read_Value(fileName, ATLANTIS_ATTRIBUTE, bm->ecotest, 1, attributeGroupNode, binary_check, "flag_do_var_express");
	bm->flag_do_evolution = (int)Util_XML_Read_Value(fileName, ATLANTIS_ATTRIBUTE, bm->ecotest, 1, attributeGroupNode, binary_check, "flag_do_evolution");
	bm->flag_bound_change = (int)Util_XML_Read_Value(fileName, ATLANTIS_ATTRIBUTE, bm->ecotest, 1, attributeGroupNode, binary_check, "flag_bound_change");
	bm->flag_inheritance = (int)Util_XML_Read_Value(fileName, ATLANTIS_ATTRIBUTE, bm->ecotest, 1, attributeGroupNode, binary_check, "flag_inheritance");
	bm->flag_evolvar_capped = (int)Util_XML_Read_Value(fileName, ATLANTIS_ATTRIBUTE, bm->ecotest, 1, attributeGroupNode, binary_check, "flag_evolvar_capped");
    bm->evol_stdev_range = Util_XML_Read_Value(fileName, ATLANTIS_ATTRIBUTE, bm->ecotest, 1, attributeGroupNode, no_checking, "evol_stdev_range");
	bm->max_rate_evol = Util_XML_Read_Value(fileName, ATLANTIS_ATTRIBUTE, bm->ecotest, 1, attributeGroupNode, no_checking, "max_rate_evol");

	bm->flag_mult_grow_curves = (int)Util_XML_Read_Value(fileName, ATLANTIS_ATTRIBUTE, bm->ecotest, 1, attributeGroupNode, binary_check, "flag_mult_grow_curves");

	if((bm->K_num_max_genetypes > 1) && !bm->flag_do_evolution && !bm->flag_mult_grow_curves)
		quit("Have at least one group with multiple genotypes (max num = %d) but do not have evolution or multiple growth curves activated\n", bm->K_num_max_genetypes);
	if((bm->flag_do_evolution || bm->flag_mult_grow_curves) && (bm->K_num_max_genetypes == 1))
		quit("Have evolution or multiple growth curves activated, but all groups only have a single genotype (max num genotypes = %d)\n", bm->K_num_max_genetypes);

	if(bm->flag_do_evolution) {

		// Calculate DNA[species].stdev_gene_map[ngene]
		Calculate_stdev_gene_map(bm, llogfp);

		// Read in whether traits active
		Read_Active_Trait_XML(bm, fileName, attributeGroupNode);

		// Read in other parameters
		Util_XML_Read_Species_Param(bm, fileName, attributeGroupNode, max_prop_shift_id);
		Util_XML_Read_Species_Param(bm, fileName, attributeGroupNode, inheritance_id);
		Util_XML_Read_Species_Param(bm, fileName, attributeGroupNode, trait_variance_id);
		Util_XML_Read_Species_Param(bm, fileName, attributeGroupNode, min_trait_variance_id);
	}

}

/**
 * Read in information relevant to primary producers
 *
 *
 */
static void Read_Primary_Producer_Values(MSEBoxModel *bm, char *fileName, xmlNodePtr parent) {
	xmlNodePtr attributeGroupNode;

	attributeGroupNode = Util_XML_Get_Node(ATLANTIS_ATTRIBUTE_SUB_GROUP, parent, "PrimaryProduction");
	if (attributeGroupNode == NULL)
		quit("PrimaryProduction attribute group not found in input file %s.\n", fileName);

	MB_wc = Util_XML_Read_Value(fileName, ATLANTIS_ATTRIBUTE, bm->ecotest, 1, attributeGroupNode, no_checking, "MB_wc");

	bm->eddy_scale = Util_XML_Read_Value(fileName, ATLANTIS_ATTRIBUTE, bm->ecotest, 1, attributeGroupNode, no_checking, "eddy_scale");

	Util_XML_Read_Species_Param(bm, fileName, attributeGroupNode, KTUR_id);
	Util_XML_Read_Species_Param(bm, fileName, attributeGroupNode, KIRR_id);
	Util_XML_Read_Species_Param(bm, fileName, attributeGroupNode, KN_id);
	Util_XML_Read_Species_Param(bm, fileName, attributeGroupNode, KS_id);
	Util_XML_Read_Species_Param(bm, fileName, attributeGroupNode, KF_id);
	Util_XML_Read_Species_Param(bm, fileName, attributeGroupNode, KI_T15_id);

	if(bm->ice_on) {
		attributeGroupNode = Util_XML_Get_Node(ATLANTIS_ATTRIBUTE_SUB_GROUP, parent, "IcePrimaryProduction");
		if (attributeGroupNode == NULL)
			quit("IcePrimaryProduction attribute group not found in input file %s.\n", fileName);
		Util_XML_Read_Species_Param(bm, fileName, attributeGroupNode, Beta_D_id);
		Util_XML_Read_Species_Param(bm, fileName, attributeGroupNode, PBmax_D_id);
	}

	if(bm->flag_macro_model) {
		Util_XML_Read_Species_Param(bm, fileName, attributeGroupNode, KI_L_T15_id);
		Util_XML_Read_Species_Param(bm, fileName, attributeGroupNode, Kext_id);
		Util_XML_Read_Species_Param(bm, fileName, attributeGroupNode, Ksub_id);
		Util_XML_Read_Species_Param(bm, fileName, attributeGroupNode, KNepi_id);
		Util_XML_Read_Species_Param(bm, fileName, attributeGroupNode, KsubEpi_id);
		Util_XML_Read_Species_Param(bm, fileName, attributeGroupNode, Ktrans_id);
	}

	if(bm->containsCoral){
		Util_XML_Read_Species_Param(bm, fileName, attributeGroupNode, coral_overgrow_id);
		Util_XML_Read_Species_Param(bm, fileName, attributeGroupNode, coral_compete_id);
	}
    
    if(bm->containsSponge){
        Util_XML_Read_Species_Param(bm, fileName, attributeGroupNode, sponge_overgrow_id);
        Util_XML_Read_Species_Param(bm, fileName, attributeGroupNode, sponge_compete_id);
    }
}

static void Read_Bacteria_Values(MSEBoxModel *bm, char *fileName, xmlNodePtr parent) {
	xmlNodePtr attributeGroupNode;

	attributeGroupNode = Util_XML_Get_Node(ATLANTIS_ATTRIBUTE_SUB_GROUP, parent, "Bacteria");
	if (attributeGroupNode == NULL)
		quit("Bacteria attribute group not found in input file %s.\n", fileName);

	/* Bacterial population parameters (how much bacterial biomass detritus can support) */
	XPB_DL = Util_XML_Read_Value(fileName, ATLANTIS_ATTRIBUTE, bm->ecotest, 1, attributeGroupNode, no_checking, "XPB_DL");
	XBB_DL = Util_XML_Read_Value(fileName, ATLANTIS_ATTRIBUTE, bm->ecotest, 1, attributeGroupNode, no_checking, "XBB_DL");

	XPB_DR = Util_XML_Read_Value(fileName, ATLANTIS_ATTRIBUTE, bm->ecotest, 1, attributeGroupNode, no_checking, "XPB_DR");
	XBB_DR = Util_XML_Read_Value(fileName, ATLANTIS_ATTRIBUTE, bm->ecotest, 1, attributeGroupNode, no_checking, "XBB_DR");

	k_PB = Util_XML_Read_Value(fileName, ATLANTIS_ATTRIBUTE, bm->ecotest, 1, attributeGroupNode, no_checking, "k_PB");
	k_BB = Util_XML_Read_Value(fileName, ATLANTIS_ATTRIBUTE, bm->ecotest, 1, attributeGroupNode, no_checking, "k_BB");
}

/**
 *  \brief Read the bleaching data into the coral species parameters
 */

static void Read_Coral_Bleaching_Values(MSEBoxModel *bm, char *fileName, xmlNodePtr parent) {
	xmlNodePtr attributeGroupNode;

	attributeGroupNode = Util_XML_Get_Node(ATLANTIS_ATTRIBUTE_SUB_GROUP, parent, "Bleaching");
	if (attributeGroupNode == NULL)
		quit("Bleaching attribute group not found in input file %s.\n", fileName);

	Util_XML_Read_Species_Param(bm, fileName, attributeGroupNode, bleach_periodA_id);
	Util_XML_Read_Species_Param(bm, fileName, attributeGroupNode, bleach_periodB_id);
	Util_XML_Read_Species_Param(bm, fileName, attributeGroupNode, mBleach_id);
	Util_XML_Read_Species_Param(bm, fileName, attributeGroupNode, bleaching_rate_id);
	Util_XML_Read_Species_Param(bm, fileName, attributeGroupNode, bleach_recovery_rate_id);
	Util_XML_Read_Species_Param(bm, fileName, attributeGroupNode, bleach_tempshift_id);
	Util_XML_Read_Species_Param(bm, fileName, attributeGroupNode, bleach_growshift_id);
	Util_XML_Read_Species_Param(bm, fileName, attributeGroupNode, bleach_temp_id);
    Util_XML_Read_Species_Param(bm, fileName, attributeGroupNode, min_bleach_temp_id);
	Util_XML_Read_Species_Param(bm, fileName, attributeGroupNode, DHW_thresh_id);
    Util_XML_Read_Species_Param(bm, fileName, attributeGroupNode, threshdepth_id);
    Util_XML_Read_Species_Param(bm, fileName, attributeGroupNode, depmum_scalar_id);

    Util_XML_Read_Species_Param(bm, fileName, attributeGroupNode, min_bleach_salt_id);
    Util_XML_Read_Species_Param(bm, fileName, attributeGroupNode, max_bleach_salt_id);

}

/**
 *  \brief Read the bleaching data into the coral species parameters
 */

static void Read_Coral_Calcification_Values(MSEBoxModel *bm, char *fileName, xmlNodePtr parent) {
	xmlNodePtr attributeGroupNode;

	attributeGroupNode = Util_XML_Get_Node(ATLANTIS_ATTRIBUTE_SUB_GROUP, parent, "Calcification");
	if (attributeGroupNode == NULL)
		quit("Bleaching attribute group not found in input file %s.\n", fileName);

	Util_XML_Read_Species_Param(bm, fileName, attributeGroupNode, calcifRefBaseline_id);
	Util_XML_Read_Species_Param(bm, fileName, attributeGroupNode, calcifTconst_id);
	Util_XML_Read_Species_Param(bm, fileName, attributeGroupNode, calcifTcoefft_id);
	Util_XML_Read_Species_Param(bm, fileName, attributeGroupNode, calcifTopt_id);
	Util_XML_Read_Species_Param(bm, fileName, attributeGroupNode, calcifLambda_id);
}

/**
 *  \brief Read the bleaching data into the coral species parameters
 */

static void Read_Coral_ReefGrowth_Values(MSEBoxModel *bm, char *fileName, xmlNodePtr parent) {
	xmlNodePtr attributeGroupNode;

	attributeGroupNode = Util_XML_Get_Node(ATLANTIS_ATTRIBUTE_SUB_GROUP, parent, "ReefGrowth");
	if (attributeGroupNode == NULL)
		quit("Bleaching attribute group not found in input file %s.\n", fileName);

	Util_XML_Read_Species_Param(bm, fileName, attributeGroupNode, coral_max_accel_trans_id);
	Util_XML_Read_Species_Param(bm, fileName, attributeGroupNode, coral_max_accelA_id);
	Util_XML_Read_Species_Param(bm, fileName, attributeGroupNode, coral_max_accelB_id);

}

/**
 *  \brief Read the bleaching data into the coral species parameters
 */

static void Read_Coral_ReefRecruit_Values(MSEBoxModel *bm, char *fileName, xmlNodePtr parent) {
	xmlNodePtr attributeGroupNode;

	attributeGroupNode = Util_XML_Get_Node(ATLANTIS_ATTRIBUTE_SUB_GROUP, parent, "ReefRecruit");
	if (attributeGroupNode == NULL)
		quit("Bleaching attribute group not found in input file %s.\n", fileName);

	Util_XML_Read_Species_Param(bm, fileName, attributeGroupNode, CrecruitA_id);
	Util_XML_Read_Species_Param(bm, fileName, attributeGroupNode, CrecruitB_id);
	Util_XML_Read_Species_Param(bm, fileName, attributeGroupNode, CrecruitC_id);

}

/**
 *  \brief Read the bleaching data into the coral species parameters
 */

static void Read_Coral_Rugosity_Values(MSEBoxModel *bm, char *fileName, xmlNodePtr parent) {
	xmlNodePtr attributeGroupNode;

	attributeGroupNode = Util_XML_Get_Node(ATLANTIS_ATTRIBUTE_SUB_GROUP, parent, "RugosityModel");
	if (attributeGroupNode == NULL)
		quit("Bleaching attribute group not found in input file %s.\n", fileName);

    Util_XML_Read_Species_Param(bm, fileName, attributeGroupNode, rug_factor_id);
	Util_XML_Read_Species_Param(bm, fileName, attributeGroupNode, rug_erode_id);
	Util_XML_Read_Species_Param(bm, fileName, attributeGroupNode, rug_bleacherode_id);
    Util_XML_Read_Species_Param(bm, fileName, attributeGroupNode, colony_ha_id);
    Util_XML_Read_Species_Param(bm, fileName, attributeGroupNode, rug_erode_sponge_id);

	Read_Cohort_Species_Param_Values(bm, fileName, attributeGroupNode, rugosity_inc_id);
	Read_Cohort_Species_Param_Values(bm, fileName, attributeGroupNode, rugosity_dec_id);
	Read_Cohort_Species_Param_Values(bm, fileName, attributeGroupNode, colony_diam_id);

}

/**
 *  \brief Read the sponge and smothering related parameters
 */

static void Read_Sponge_Smother_Values(MSEBoxModel *bm, char *fileName, xmlNodePtr parent) {
    xmlNodePtr attributeGroupNode;
    
    attributeGroupNode = Util_XML_Get_Node(ATLANTIS_ATTRIBUTE_SUB_GROUP, parent, "SpongeSmother");
    if (attributeGroupNode == NULL)
        quit("SpongeSmother attribute group not found in input file %s.\n", fileName);
    
    Util_XML_Read_Species_Param(bm, fileName, attributeGroupNode, Ksmother_B_id);
    Util_XML_Read_Species_Param(bm, fileName, attributeGroupNode, Ksmother_A_id);
    
    if(bm->containsSponge){
        Util_XML_Read_Species_Param(bm, fileName, attributeGroupNode, Vmax_deltaSi_id);
        Util_XML_Read_Species_Param(bm, fileName, attributeGroupNode, Km_deltaSi_id);
    }
}

/**
 *  \brief Read the data into the coral species parameters
 */

static void Read_Coral_Species_Values(MSEBoxModel *bm, char *fileName, xmlNodePtr parent, FILE *llogfp) {
	xmlNodePtr attributeGroupNode;

	attributeGroupNode = Util_XML_Get_Node(ATLANTIS_ATTRIBUTE_SUB_GROUP, parent, "CoralReef");
	if (attributeGroupNode == NULL)
		quit("CoralReef attribute group not found in input file %s.\n", fileName);

	Read_Coral_Bleaching_Values(bm, fileName, attributeGroupNode);

	Util_XML_Read_Species_Param(bm, fileName, attributeGroupNode, prop_zooxanth_id);
	Util_XML_Read_Species_Param(bm, fileName, attributeGroupNode, HostRemin_id);
	Util_XML_Read_Species_Param(bm, fileName, attributeGroupNode, FeedLightThresh_id);
	Util_XML_Read_Species_Param(bm, fileName, attributeGroupNode, PropLightFeed_id);

	Read_Coral_Calcification_Values(bm, fileName, attributeGroupNode);
	Read_Coral_ReefGrowth_Values(bm, fileName, attributeGroupNode);
	Read_Coral_ReefRecruit_Values(bm, fileName, attributeGroupNode);
	Read_Coral_Rugosity_Values(bm, fileName, attributeGroupNode);
    Read_Sponge_Smother_Values(bm, fileName, attributeGroupNode);
}

/**
 *	\brief Read the data into the recruit_vdistrib array from the input XML file.
 */
static void Read_Habitat_XML(MSEBoxModel *bm, char *fileName, xmlNodePtr parent) {

	double *values = 0;
	double sum_hab;
	int guild, b, cohort;
	xmlNodePtr attributeGroup, speciesNode;
	char errorString[STRLEN];
	char *nodeName;

	nodeName =  Util_Get_Node_Name(parent);
	if (verbose)
		printf("Reading habitat values\n");

	attributeGroup = Util_XML_Get_Node(ATLANTIS_ATTRIBUTE, parent, "Group_Habitat_Preference");
	if (attributeGroup == NULL)
		quit("Group_Habitat_Preference attribute group not found in input file %s.\n", fileName);

	/* Create a node for each functional group*/
	for (guild = 0; guild < bm->K_num_tot_sp; guild++) {
		if (FunctGroupArray[guild].speciesParams[flag_id] == TRUE) {

			switch (FunctGroupArray[guild].groupAgeType) {
			case BIOMASS:

					if(FunctGroupArray[guild].habitatType == EPIFAUNA || (FunctGroupArray[guild].habitatCoeffs[SED] > 0 && FunctGroupArray[guild].isGrazer == TRUE && FunctGroupArray[guild].isBacteria == FALSE) ){

						// Read the data into a temp array.
						if (Util_XML_Read_Array_Double(ATLANTIS_GROUP_ATTRIBUTE, fileName, "Group_Habitat_Preference", attributeGroup, no_checking, FunctGroupArray[guild].groupCode, &values,
								bm->K_num_cover_types) == FALSE) {
							quit("Error: Unable to find parameter Group_Habitat_Preference/%s in input file %s\n", FunctGroupArray[guild].groupCode, fileName);
						}

						sum_hab = 0.0;
						for (b = 0; b < bm->K_num_cover_types; b++) {
							bm->HABITATlike[guild][0][b] = values[b];
							bm->HABITATlike[guild][1][b] = values[b];
							sum_hab += values[b];
						}
						free(values);

						sum_hab -= bm->HABITATlike[guild][0][bm->REEFcover_id];
						sum_hab -= bm->HABITATlike[guild][0][bm->FLATcover_id];
						sum_hab -= bm->HABITATlike[guild][0][bm->SOFTcover_id];
						sum_hab -= bm->HABITATlike[guild][0][bm->CANYONcover_id];
						FunctGroupArray[guild].speciesParams[K_num_biogen_habad_id] = sum_hab;
						FunctGroupArray[guild].speciesParams[K_num_biogen_habjuv_id] = sum_hab;
				}

				break;
			case AGE_STRUCTURED_BIOMASS: /* Intentional follow through */
			case AGE_STRUCTURED:
				/* get the species node */
				sprintf(errorString, "%s/Group_Habitat_Preference/%s", nodeName, FunctGroupArray[guild].groupCode);

				speciesNode = Util_XML_Get_Node(ATLANTIS_GROUP_ATTRIBUTE, attributeGroup, FunctGroupArray[guild].groupCode);
				if (speciesNode == NULL)
					quit("%s attribute group not found.\n", errorString);

				for (cohort = 0; cohort < FunctGroupArray[guild].numStages; cohort++) {
					if (Util_XML_Read_Array_Double(ATLANTIS_AGE_CLASS_ATTRIBUTE, fileName, errorString, speciesNode, no_checking, cohortString[cohort], &values,
							bm->K_num_cover_types) == FALSE) {
						quit("Error: Unable to find parameter Group_Habitat_Preference/%s/%s in input file %s\n", FunctGroupArray[guild].groupCode,
								cohortString[cohort], fileName);
					}
					sum_hab = 0.0;
					for (b = 0; b < bm->K_num_cover_types; b++) {
						bm->HABITATlike[guild][cohort][b] = values[b];
						sum_hab += values[b];
					}
					free(values);

					sum_hab -= bm->HABITATlike[guild][cohort][bm->REEFcover_id];
					sum_hab -= bm->HABITATlike[guild][cohort][bm->FLATcover_id];
					sum_hab -= bm->HABITATlike[guild][cohort][bm->SOFTcover_id];
					sum_hab -= bm->HABITATlike[guild][cohort][bm->CANYONcover_id];
					if(cohort)
						FunctGroupArray[guild].speciesParams[K_num_biogen_habad_id] = sum_hab;
					else
						FunctGroupArray[guild].speciesParams[K_num_biogen_habjuv_id] = sum_hab;
				}
				break;
			}
		}
	}
	free(nodeName);
}



/**
 *	\brief Read the data into the recruit_vdistrib array from the input XML file.
 */
static void Read_Ice_Habitat_XML(MSEBoxModel *bm, char *fileName, xmlNodePtr parent) {

	double *values = 0;
	int guild, b, cohort;
	xmlNodePtr attributeGroup, speciesNode;
	char errorString[STRLEN];
	char *nodeName;

	// Initialise - even if ice not on
	if(!bm->ice_on){
		for (guild = 0; guild < bm->K_num_tot_sp; guild++) {
			for (cohort = 0; cohort < FunctGroupArray[guild].numStages; cohort++) {
				for (b = 0; b < bm->K_num_ice_classes; b++) {
					bm->ICE_HABITATlike[guild][cohort][b] = 0;
					bm->ICE_HABITATlikeReprod[guild][cohort][b] = 0;
				}
			}
		}
		return;
	}

	// Read in actual parameter values
	nodeName =  Util_Get_Node_Name(parent);
	if (verbose)
		printf("Reading habitat values\n");

	attributeGroup = Util_XML_Get_Node(ATLANTIS_ATTRIBUTE, parent, "Group_Ice_Preference");
	if (attributeGroup == NULL)
		quit("Group_Ice_Preference attribute group not found in input file %s.\n", fileName);

	/* Create a node for each functional group*/
	for (guild = 0; guild < bm->K_num_tot_sp; guild++) {
		if (FunctGroupArray[guild].speciesParams[flag_id] == TRUE) {
			switch (FunctGroupArray[guild].groupAgeType) {
			case BIOMASS:
				// Read the data into a temp array.
				if (Util_XML_Read_Array_Double(ATLANTIS_GROUP_ATTRIBUTE, fileName, "Group_Ice_Preference", attributeGroup, no_checking, FunctGroupArray[guild].groupCode, &values,
						bm->K_num_ice_classes) == FALSE) {
					quit("Error: Unable to find parameter Group_Ice_Preference/%s in input file %s\n", FunctGroupArray[guild].groupCode, fileName);
				}

				for (b = 0; b < bm->K_num_ice_classes; b++) {
					bm->ICE_HABITATlike[guild][juv_id][b] = (int)values[b];
					bm->ICE_HABITATlike[guild][adult_id][b] = (int)values[b];
				}
				free(values);
				break;
			case AGE_STRUCTURED_BIOMASS: /* Intentional follow through */
			case AGE_STRUCTURED:
				/* get the species node */
				sprintf(errorString, "%s/Group_Ice_Preference/%s", nodeName, FunctGroupArray[guild].groupCode);

				speciesNode = Util_XML_Get_Node(ATLANTIS_GROUP_ATTRIBUTE, attributeGroup, FunctGroupArray[guild].groupCode);
				if (speciesNode == NULL)
					quit("%s attribute group not found.\n", errorString);

				for (cohort = 0; cohort < FunctGroupArray[guild].numStages; cohort++) {
					if (Util_XML_Read_Array_Double(ATLANTIS_AGE_CLASS_ATTRIBUTE, fileName, errorString, speciesNode, no_checking, cohortString[cohort], &values,
							bm->K_num_ice_classes) == FALSE) {
						quit("Error: Unable to find parameter Group_Ice_Preference/%s/%s in input file %s\n", FunctGroupArray[guild].groupCode,
								cohortString[cohort], fileName);
					}
					for (b = 0; b < bm->K_num_ice_classes; b++) {
						bm->ICE_HABITATlike[guild][cohort][b] = (int)values[b];
					}
					free(values);
				}
				break;
			}
		}
	}

	attributeGroup = Util_XML_Get_Node(ATLANTIS_ATTRIBUTE, parent, "Group_IceReprod_Preference");
	if (attributeGroup == NULL)
		quit("Group_IceReprod_Preference attribute group not found in input file %s.\n", fileName);

	/* Create a node for each functional group*/
	for (guild = 0; guild < bm->K_num_tot_sp; guild++) {
		if (FunctGroupArray[guild].speciesParams[flag_id] == TRUE) {
			switch (FunctGroupArray[guild].groupAgeType) {
			case BIOMASS:
				// Read the data into a temp array.
				if (Util_XML_Read_Array_Double(ATLANTIS_GROUP_ATTRIBUTE, fileName, "Group_IceReprod_Preference", attributeGroup, no_checking, FunctGroupArray[guild].groupCode, &values,
						bm->K_num_ice_classes) == FALSE) {
					quit("Error: Unable to find parameter Group_Ice_Preference/%s in input file %s\n", FunctGroupArray[guild].groupCode, fileName);
				}

				for (b = 0; b < bm->K_num_ice_classes; b++) {
					bm->ICE_HABITATlikeReprod[guild][juv_id][b] = (int)values[b];
					bm->ICE_HABITATlikeReprod[guild][adult_id][b] = (int)values[b];
				}
				free(values);
				break;
			case AGE_STRUCTURED_BIOMASS: /* Intentional follow through */
			case AGE_STRUCTURED:
				/* get the species node */
				sprintf(errorString, "%s/Group_IceReprod_Preference/%s", nodeName, FunctGroupArray[guild].groupCode);

				speciesNode = Util_XML_Get_Node(ATLANTIS_GROUP_ATTRIBUTE, attributeGroup, FunctGroupArray[guild].groupCode);
				if (speciesNode == NULL)
					quit("%s attribute group not found.\n", errorString);

				for (cohort = 0; cohort < FunctGroupArray[guild].numStages; cohort++) {
					if (Util_XML_Read_Array_Double(ATLANTIS_AGE_CLASS_ATTRIBUTE, fileName, errorString, speciesNode, no_checking, cohortString[cohort], &values,
							bm->K_num_ice_classes) == FALSE) {
						quit("Error: Unable to find parameter Group_IceReprod_Preference/%s/%s in input file %s\n", FunctGroupArray[guild].groupCode,
								cohortString[cohort], fileName);
					}
					for (b = 0; b < bm->K_num_ice_classes; b++) {
						bm->ICE_HABITATlikeReprod[guild][cohort][b] = (int)values[b];
					}
					free(values);
				}
				break;
			}
		}
	}
	free(nodeName);
}

static void Read_Habitat_Values(MSEBoxModel *bm, char *fileName, xmlNodePtr parent) {
	xmlNodePtr attributeGroupNode, attributeGroup;

	attributeGroupNode = Util_XML_Get_Node(ATLANTIS_ATTRIBUTE_SUB_GROUP, parent, "Habitat");
	if (attributeGroupNode == NULL)
		quit("Habitat attribute group not found in input file %s.\n", fileName);

	bm->flaghabdepend = (int) Util_XML_Read_Value(fileName, ATLANTIS_ATTRIBUTE, bm->ecotest, 1, attributeGroupNode, binary_check, "flaghabdepend");
    bm->flag_move_habdepend = (int) Util_XML_Read_Value(fileName, ATLANTIS_ATTRIBUTE, bm->ecotest, 1, attributeGroupNode, binary_check, "flag_move_habdepend");
    bm->flagenviro_displace = (int) Util_XML_Read_Value(fileName, ATLANTIS_ATTRIBUTE, bm->ecotest, 1, attributeGroupNode, binary_check, "flagenviro_displace");
    bm->flagenviro_kill = (int) Util_XML_Read_Value(fileName, ATLANTIS_ATTRIBUTE, bm->ecotest, 1, attributeGroupNode, binary_check, "flagenviro_kill");


	/* Habitat degradation effects */
	attributeGroup = Util_XML_Get_Node(ATLANTIS_ATTRIBUTE_SUB_GROUP, attributeGroupNode, "Habitat_Degradation");
	if (attributeGroup == NULL)
		quit("Habitat_Degradation attribute group not found in input file %s.\n", fileName);

	REEFchange_max_num = (int) Util_XML_Read_Value(fileName, ATLANTIS_ATTRIBUTE, bm->ecotest, 1, attributeGroup, no_checking, "REEFchange_max_num");
	FLATchange_max_num = (int) Util_XML_Read_Value(fileName, ATLANTIS_ATTRIBUTE, bm->ecotest, 1, attributeGroup, no_checking, "FLATchange_max_num");
	SOFTchange_max_num = (int) Util_XML_Read_Value(fileName, ATLANTIS_ATTRIBUTE, bm->ecotest, 1, attributeGroup, no_checking, "SOFTchange_max_num");

	/* Find the max of these values */
	/* Check which is largest number of habitat degradations */
	BEDchange_max_num = 0;
	if (REEFchange_max_num > BEDchange_max_num)
		BEDchange_max_num = REEFchange_max_num;
	if (FLATchange_max_num > BEDchange_max_num)
		BEDchange_max_num = FLATchange_max_num;
	if (SOFTchange_max_num > BEDchange_max_num)
		BEDchange_max_num = SOFTchange_max_num;

	/* Now we can allocate the BEDchange array */
	if(BEDchange_max_num > 0){
		BEDchange = (double ***) alloc3d(3, BEDchange_max_num, bm->K_num_bed_types);

		attributeGroup = Util_XML_Get_Node(ATLANTIS_ATTRIBUTE_SUB_GROUP, attributeGroupNode, "Habitat_Change");
		if (attributeGroup == NULL)
			quit("Habitat_Change attribute group not found in input file %s.\n", fileName);
		Read_Change_DataXML(bm, fileName, attributeGroup, "REEFchange", BEDchange[reef_id], REEFchange_max_num, no_checking);
		Read_Change_DataXML(bm, fileName, attributeGroup, "FLATchange", BEDchange[flat_id], FLATchange_max_num, no_checking);
		Read_Change_DataXML(bm, fileName, attributeGroup, "SOFTchange", BEDchange[soft_id], SOFTchange_max_num, no_checking);
	}
	if (Util_XML_Read_Array_Double(ATLANTIS_ATTRIBUTE, fileName, "Habitat/Habitat_Change", attributeGroupNode, binary_check, "Box_degraded", &Box_degradedi, bm->nbox) == FALSE) {
		quit("Error: Unable to find parameters 'Habitat/Habitat_Change/Box_degraded' in input file %s\n", fileName);
	}

	Util_XML_Read_Species_Param(bm, fileName, attributeGroupNode, Kcov_juv_id);
	Util_XML_Read_Species_Param(bm, fileName, attributeGroupNode, Bcov_juv_id);
	Util_XML_Read_Species_Param(bm, fileName, attributeGroupNode, Acov_juv_id);
	Util_XML_Read_Species_Param(bm, fileName, attributeGroupNode, Kcov_ad_id);
	Util_XML_Read_Species_Param(bm, fileName, attributeGroupNode, Bcov_ad_id);
	Util_XML_Read_Species_Param(bm, fileName, attributeGroupNode, Acov_ad_id);
	if(bm->flag_adv_habitat) {
		Util_XML_Read_Species_Param(bm, fileName, attributeGroupNode, RugCover_scalar_id);
	}

	Read_Habitat_XML(bm, fileName, attributeGroupNode);

	if(bm->ice_on){
		Read_Ice_Habitat_XML(bm, fileName, attributeGroupNode);
	}
}

static void Read_Spatial_Threshold_Values(MSEBoxModel *bm, char *fileName, xmlNodePtr parent) {
	xmlNodePtr attributeGroupNode;

	attributeGroupNode = Util_XML_Get_Node(ATLANTIS_ATTRIBUTE_SUB_GROUP, parent, "Spatial_Threshold");
	if (attributeGroupNode == NULL)
		quit("Spatial_Threshold attribute group not found in input file %s.\n", fileName);

	Util_XML_Read_Species_Param(bm, fileName, attributeGroupNode, low_id);
	Util_XML_Read_Species_Param(bm, fileName, attributeGroupNode, max_id);
	Util_XML_Read_Species_Param(bm, fileName, attributeGroupNode, sat_id);
	Util_XML_Read_Species_Param(bm, fileName, attributeGroupNode, thresh_id);

}

/**
 *	/brief Load the vertical distribution data from the biology input file.
 *
 *	Data will be read into the distribVERT_VERTICAL array.
 *
 **/
static void Read_VERT_Function(MSEBoxModel *bm, char *fileName, xmlNodePtr parent) {
	double *values = 0;
	int b;
	int cohort, dayIndex;
	int guild;
	xmlNodePtr attributeNode, specieNode, tempNode;
	char errorString[STRLEN];

	attributeNode = Util_XML_Get_Node(ATLANTIS_ATTRIBUTE, parent, "VerticalDistribution");
	if (attributeNode == NULL)
		quit("Movement/VerticalDistribution attribute group not found in input file %s.\n", fileName);

	if (verbose)
		printf("Reading VerticalDistribution values\n");

	/* Create a node for each functional group*/
	for (guild = 0; guild < bm->K_num_tot_sp; guild++) {

		if (FunctGroupArray[guild].groupAgeType == AGE_STRUCTURED) {
			specieNode = Util_XML_Get_Node(ATLANTIS_GROUP_ATTRIBUTE, attributeNode, FunctGroupArray[guild].groupCode);
			if (specieNode == NULL)
				quit("Movement/VerticalDistribution/%s species attribute group not found.\n", FunctGroupArray[guild].groupCode);

			if (FunctGroupArray[guild].isMobile > 0) {
				for (dayIndex = 0; dayIndex < 2; dayIndex++) {
					sprintf(errorString , "Movement/VerticalDistribution/%s/%s", FunctGroupArray[guild].groupCode, dayStrings[dayIndex]);
					tempNode = Util_XML_Get_Node(ATLANTIS_TEMPORAL_ATTRIBUTE, specieNode, dayStrings[dayIndex]);
					if (tempNode == NULL)
						quit("%s species attribute group not found.\n", errorString);

					for (cohort = 0; cohort < 2; cohort++) {
						if (Util_XML_Read_Array_Double(ATLANTIS_AGE_CLASS_ATTRIBUTE, fileName, errorString, tempNode, probability_check, cohortString[cohort], &values,
								bm->wcnz) == FALSE) {
							quit("Error: Unable to find parameter '%s/%s' in input file %s\n",errorString, cohortString[cohort], fileName);
						}
						for (b = 0; b < bm->wcnz; b++)
							FunctGroupArray[guild].distrib_VERTICAL[dayIndex][b][cohort] = values[b];

						free(values);
					}
				}
			}

			if(FunctGroupArray[guild].isOverWinter > 0){
				sprintf(errorString , "Movement/VerticalDistribution/%s/overwinter", FunctGroupArray[guild].groupCode);
				tempNode = Util_XML_Get_Node(ATLANTIS_TEMPORAL_ATTRIBUTE, specieNode, "overwinter");
				if (tempNode == NULL)
					quit("%s species attribute group not found.\n", errorString);

				for (cohort = 0; cohort < FunctGroupArray[guild].numStages; cohort++) {
					if (Util_XML_Read_Array_Double(ATLANTIS_AGE_CLASS_ATTRIBUTE, fileName, errorString, tempNode, probability_check, cohortString[cohort], &values,
							bm->wcnz) == FALSE) {
						quit("Error: Unable to find parameter '%s/%s' in input file %s\n",errorString, cohortString[cohort], fileName);
					}
					for (b = 0; b < bm->wcnz; b++){
						FunctGroupArray[guild].distrib_OVERWINTER[b][cohort]  = values[b];
					}

					free(values);
				}
			}

		} else if(FunctGroupArray[guild].isMobile == TRUE && FunctGroupArray[guild].habitatType != EPIFAUNA){

			sprintf(errorString , "Movement/VerticalDistribution/%s", FunctGroupArray[guild].groupCode);

			specieNode = Util_XML_Get_Node(ATLANTIS_GROUP_ATTRIBUTE, attributeNode, FunctGroupArray[guild].groupCode);
			if (specieNode == NULL)
				quit("%s species attribute group not found.\n", errorString);

			if (FunctGroupArray[guild].isMobile > 0) {
				for (dayIndex = 0; dayIndex < 2; dayIndex++) {

					if (Util_XML_Read_Array_Double(ATLANTIS_TEMPORAL_ATTRIBUTE, fileName, errorString, specieNode, probability_check, dayStrings[dayIndex], &values, bm->wcnz)
							== FALSE) {
						quit("Error: Unable to find parameter '%s/%s' in input file %s\n",errorString,
								dayStrings[dayIndex], fileName);
					}

					for (b = 0; b < bm->wcnz; b++) {
						FunctGroupArray[guild].distrib_VERTICAL[dayIndex][b][adult_id] = values[b];
						FunctGroupArray[guild].distrib_VERTICAL[dayIndex][b][juv_id] = values[b];
					}

					free(values);
				}
			}

			if(FunctGroupArray[guild].isOverWinter > 0){


				if (Util_XML_Read_Array_Double(ATLANTIS_TEMPORAL_ATTRIBUTE, fileName, errorString, specieNode, probability_check, "overwinter", &values, bm->wcnz)
						== FALSE) {
					quit("Error: Unable to find parameter '%s/%s' in input file %s\n",errorString,
							"overwinter", fileName);
				}

				for (b = 0; b < bm->wcnz; b++) {
					FunctGroupArray[guild].distrib_OVERWINTER[b][adult_id]  = values[b];
					FunctGroupArray[guild].distrib_OVERWINTER[b][juv_id]  = values[b];
				}

				free(values);

			}
		}
	}
}

/**
 *	\brief Create the XML structure to hold the seasonal distribution data for all functional groups.
 */
static void Read_Sesonal_Dist_XML(MSEBoxModel *bm, char *fileName, xmlNodePtr parent) {

	double *values = 0;
	int cohort, guild, b, season, readcase = 0;
	xmlNodePtr attributeNode, specieNode, cohortNode;
	char str[100];
	char errorString[STRLEN];

	attributeNode = Util_XML_Get_Node(ATLANTIS_ATTRIBUTE, parent, "SeasonalDistribution");
	if (attributeNode == NULL)
		quit("Movement/SeasonalDistribution attribute group not found in input file %s.\n", fileName);

	if (verbose)
		printf("Reading SeasonalDistribution values\n");

	/* Read in the data for each functional group.*/
	for (guild = 0; guild < bm->K_num_tot_sp; guild++) {
		if ((int)FunctGroupArray[guild].speciesParams[flag_id] == TRUE){

			if (FunctGroupArray[guild].sp_geo_move) {
				specieNode = Util_XML_Get_Node(ATLANTIS_GROUP_ATTRIBUTE, attributeNode, FunctGroupArray[guild].groupCode);
				if (specieNode == NULL)
					quit("Movement/SeasonalDistribution/%s species attribute group not found.\n", FunctGroupArray[guild].groupCode);

				switch (FunctGroupArray[guild].groupAgeType) {
				case AGE_STRUCTURED: /* Follow through */
				case AGE_STRUCTURED_BIOMASS:
					if (FunctGroupArray[guild].numStages > 1)
						readcase = 1;
					else
						readcase = 0;
					break;
				case BIOMASS:
					readcase = 0;
					break;
				}
                
                
                switch (readcase) {
				case 1:
					for (cohort = 0; cohort < FunctGroupArray[guild].numStages; cohort++) {  
						sprintf(errorString, "Movement/SeasonalDistribution/%s/%s", FunctGroupArray[guild].groupCode, cohortString[cohort]);
						cohortNode = Util_XML_Get_Node(ATLANTIS_AGE_CLASS_ATTRIBUTE, specieNode, cohortString[cohort]);
						if (cohortNode == NULL)
							quit("%s cohort attribute group not found.\n", errorString);


						for (season = 1; season <= FunctGroupArray[guild].numMoveEntries; season++) {
							sprintf(str, "season%d", season);
							if (Util_XML_Read_Array_Double(ATLANTIS_TEMPORAL_ATTRIBUTE, fileName, errorString, cohortNode, probability_check, str, &values, bm->nbox) == FALSE) {
								quit("Error: Unable to find parameter '/%s/%s' in input file %s\n",
										errorString, str, fileName);
							}

							for (b = 0; b < bm->nbox; b++){
								FunctGroupArray[guild].distrib[b][season - 1][cohort] = values[b];
							}

							free(values);
						}
					}
					break;
				case 0:
					sprintf(errorString, "Movement/SeasonalDistribution/%s/adult", FunctGroupArray[guild].groupCode);
					cohort = adult_id;
					cohortNode = Util_XML_Get_Node(ATLANTIS_AGE_CLASS_ATTRIBUTE, specieNode, "adult");
					if (cohortNode == NULL)
						quit("%s cohort attribute group not found.\n", errorString);

					for (season = 1; season <= FunctGroupArray[guild].numMoveEntries; season++) {
						sprintf(str, "season%d", season);

						//printf("Reading %s %s\n", errorString, str);
						if (Util_XML_Read_Array_Double(ATLANTIS_TEMPORAL_ATTRIBUTE, fileName, errorString, cohortNode, probability_check, str, &values, bm->nbox) == FALSE) {
							quit("Error: Unable to find parameter '%s/%s' in input file %s\n", errorString, str, fileName);

						}

						for (b = 0; b < bm->nbox; b++) {
							FunctGroupArray[guild].distrib[b][season - 1][adult_id] = values[b];
							FunctGroupArray[guild].distrib[b][season - 1][juv_id] = values[b];  // Just to be sure
						}
						free(values);
					}
					break;
				}
			}
		}
	}
}

static void Read_Movement_Values(MSEBoxModel *bm, char *fileName, xmlNodePtr parent) {
	xmlNodePtr attributeGroupNode;

	attributeGroupNode = Util_XML_Get_Node(ATLANTIS_ATTRIBUTE_SUB_GROUP, parent, "Movement");
	if (attributeGroupNode == NULL)
		quit("Movement attribute group not found in input file %s.\n", fileName);

	/* Vertebrate movement - these can represent the total prescribed movement
	 or the contribution due to spawning migration pressure */
	roc_wgt = Util_XML_Read_Value(fileName, ATLANTIS_ATTRIBUTE, bm->ecotest, 1, attributeGroupNode, no_checking, "roc_wgt");
	k_roc_food = Util_XML_Read_Value(fileName, ATLANTIS_ATTRIBUTE, bm->ecotest, 1, attributeGroupNode, no_checking, "k_roc_food");
	Read_VERT_Function(bm, fileName, attributeGroupNode);
	Read_Sesonal_Dist_XML(bm, fileName, attributeGroupNode);

	bm->flagtempdepend_move = (int) Util_XML_Read_Value(fileName, ATLANTIS_ATTRIBUTE, bm->ecotest, 1, attributeGroupNode, integer_check, "flagtempdepend_move");
    bm->flagtempdepend_reprod = (int) Util_XML_Read_Value(fileName, ATLANTIS_ATTRIBUTE, bm->ecotest, 1, attributeGroupNode, integer_check, "flagtempdepend_reprod");
	bm->flagsaltdepend = (int) Util_XML_Read_Value(fileName, ATLANTIS_ATTRIBUTE, bm->ecotest, 1, attributeGroupNode, integer_check, "flagsaltdepend");
	bm->flagO2depend = (int) Util_XML_Read_Value(fileName, ATLANTIS_ATTRIBUTE, bm->ecotest, 1, attributeGroupNode, integer_check, "flagO2depend");
    bm->flagconstrain_epiwander = (int) Util_XML_Read_Value(fileName, ATLANTIS_ATTRIBUTE, bm->ecotest, 1, attributeGroupNode, integer_check, "flagconstrain_epiwander");

	Util_XML_Read_Species_Param(bm, fileName, attributeGroupNode, ddepend_move_id);
	if (bm->flagtempdepend_move) {
		Util_XML_Read_Species_Param(bm, fileName, attributeGroupNode, max_move_temp_id);
		Util_XML_Read_Species_Param(bm, fileName, attributeGroupNode, min_move_temp_id);
        Util_XML_Read_Species_Param(bm, fileName, attributeGroupNode, K_temp_const_id);
	}
	if (bm->flagsaltdepend) {
		Util_XML_Read_Species_Param(bm, fileName, attributeGroupNode, max_move_salt_id);
		Util_XML_Read_Species_Param(bm, fileName, attributeGroupNode, min_move_salt_id);
        Util_XML_Read_Species_Param(bm, fileName, attributeGroupNode, K_salt_const_id);
	}
	Util_XML_Read_Species_Param(bm, fileName, attributeGroupNode, Speed_id);

	Util_XML_Read_Species_Param(bm, fileName, attributeGroupNode, mindepth_id);
    Util_XML_Read_Species_Param(bm, fileName, attributeGroupNode, maxdepth_id);
    Util_XML_Read_Species_Param(bm, fileName, attributeGroupNode, maxtotdepth_id);

    if (bm->flagO2depend) {
		Util_XML_Read_Species_Param(bm, fileName, attributeGroupNode, min_O2_id);
        Util_XML_Read_Species_Param(bm, fileName, attributeGroupNode, K_o2_const_id);
    }

	Util_XML_Read_Species_Param(bm, fileName, attributeGroupNode, homerangerad_id);
	Util_XML_Read_Species_Param(bm, fileName, attributeGroupNode, rangeoverlap_id);

}

static void Read_Vert_Body_Values(MSEBoxModel *bm, char *fileName, xmlNodePtr parent) {
	xmlNodePtr attributeGroupNode;

	attributeGroupNode = Util_XML_Get_Node(ATLANTIS_ATTRIBUTE_SUB_GROUP, parent, "VertebrateBody");
	if (attributeGroupNode == NULL)
		quit("VertebrateBody attribute group not found in input file %s.\n", fileName);

	/* Fish body form and aging details */
	bm->XRS_static = Util_XML_Read_Value(fileName, ATLANTIS_ATTRIBUTE, bm->ecotest, 1, attributeGroupNode, no_checking, "X_RS"); //Read in case dynamic X_RS not being used
    
	Kthresh1 = Util_XML_Read_Value(fileName, ATLANTIS_ATTRIBUTE, bm->ecotest, 1, attributeGroupNode, no_checking, "Kthresh1");
	Kthresh2 = Util_XML_Read_Value(fileName, ATLANTIS_ATTRIBUTE, bm->ecotest, 1, attributeGroupNode, no_checking, "Kthresh1");
	KHTD = Util_XML_Read_Value(fileName, ATLANTIS_ATTRIBUTE, bm->ecotest, 1, attributeGroupNode, no_checking, "KHTD");
	KHTI = Util_XML_Read_Value(fileName, ATLANTIS_ATTRIBUTE, bm->ecotest, 1, attributeGroupNode, no_checking, "KHTI");
}

/**
 *	\brief Read in the detritus sediment food availability data from the input file.
 *	Data is read into the bm->pSPVERTeat array.
 */
static void Read_Detritus_Sediment_Food_Avail(MSEBoxModel *bm, char *fileName, xmlNodePtr parent) {

	double *values = 0;
	int cohort, guild, b;
	int prey;
	xmlNodePtr attributeNode, specieNode;
	char errorString[STRLEN];

	if (verbose)
		printf("Reading Detritus Sediment values\n");

	attributeNode = Util_XML_Get_Node(ATLANTIS_ATTRIBUTE, parent, "DetritusSedimentFoodAvail");
	if (attributeNode == NULL)
		quit("Diet/DetritusSedimentFoodAvail attribute group not found in input file %s.\n", fileName);

	/* Create a node for each functional group*/
	for (guild = 0; guild < bm->K_num_tot_sp; guild++) {
		if ((int)FunctGroupArray[guild].speciesParams[flag_id] == TRUE) {

			if (FunctGroupArray[guild].isPredator == TRUE) {

				switch (FunctGroupArray[guild].groupAgeType) {
				case AGE_STRUCTURED:
					sprintf(errorString, "Diet/DetritusSedimentFoodAvail/%s", FunctGroupArray[guild].groupCode);
					specieNode = Util_XML_Get_Node(ATLANTIS_GROUP_ATTRIBUTE, attributeNode, FunctGroupArray[guild].groupCode);
					if (specieNode == NULL)
						quit("%s species attribute group not found.\n", errorString);

					for (cohort = 0; cohort < FunctGroupArray[guild].numStages; cohort++) {
						if (Util_XML_Read_Array_Double(ATLANTIS_AGE_CLASS_ATTRIBUTE, fileName, errorString, specieNode, no_checking, cohortString[cohort], &values,
								bm->K_num_detritus) == FALSE) {
							quit("Error: Unable to find parameter '%s/%s' in input file %s\n", errorString,
									cohortString[cohort], fileName);
						}

						for (b = 0; b < bm->K_num_detritus; b++) {
							/* Work out which detritus group we are looking at*/
							prey = Util_Get_Detritus_Index(bm, b);

							FunctGroupArray[guild].pSPEat[cohort][prey][SED] = values[b];
							if (FunctGroupArray[guild].pSPEat[cohort][prey][SED] == 0 && values[b] > 0)
								FunctGroupArray[guild].pSPEat[0][prey][SED] = values[b];
						}
						free(values);
					}
					break;
				case AGE_STRUCTURED_BIOMASS:
					sprintf(errorString, "Diet/DetritusSedimentFoodAvail/%s", FunctGroupArray[guild].groupCode);

					specieNode = Util_XML_Get_Node(ATLANTIS_GROUP_ATTRIBUTE, attributeNode, FunctGroupArray[guild].groupCode);
					if (specieNode == NULL)
						quit("%s species attribute group not found.\n", errorString);

					for (cohort = 0; cohort < FunctGroupArray[guild].numStages; cohort++) {

						if (Util_XML_Read_Array_Double(ATLANTIS_AGE_CLASS_ATTRIBUTE, fileName, errorString, specieNode, no_checking, cohortString[cohort], &values,
								bm->K_num_detritus) == FALSE) {
							quit("Error: Unable to find parameter '%s/%s' in input file %s\n", errorString,
									cohortString[cohort], fileName);
						}

						for (b = 0; b < bm->K_num_detritus; b++) {
							/* Work out which detritus group we are looking at*/
							prey = Util_Get_Detritus_Index(bm, b);
							FunctGroupArray[guild].pSPEat[cohort][prey][SED] = values[b];
						}
						free(values);
					}
					break;
				case BIOMASS:
					cohort = 0;
					if (Util_XML_Read_Array_Double(ATLANTIS_GROUP_ATTRIBUTE, fileName, "Diet/DetritusSedimentFoodAvail", attributeNode, no_checking, FunctGroupArray[guild].groupCode, &values,
							bm->K_num_detritus) == FALSE) {
						quit("Error: Unable to find parameter 'Diet/DetritusSedimentFoodAvail/%s' in input file %s\n", FunctGroupArray[guild].groupCode, fileName);
					}

					for (b = 0; b < bm->K_num_detritus; b++) {
						/* Work out which detritus group we are looking at*/
						prey = Util_Get_Detritus_Index(bm, b);
						FunctGroupArray[guild].pSPEat[cohort][prey][SED] = values[b];
					}
					free(values);
					break;
				}
			}
		}
	}
}

/**
 *	\brief Read in the vertebrate food availability data from the input file.
 *	Data is read into the bm->pSPVERTeat array.
 */
static void Read_Vertebrate_Food_Avail(MSEBoxModel *bm, char *fileName, xmlNodePtr parent) {

	double *values = 0;
	int cohort, guild, preyCohort, b;
	char errorString[STRLEN];
	xmlNodePtr attributeNode, specieNode, cohortNode;

	if (verbose)
		printf("Reading VertebrateFoodAvail values\n");

	attributeNode = Util_XML_Get_Node(ATLANTIS_ATTRIBUTE, parent, "VertebrateFoodAvail");
	if (attributeNode == NULL)
		quit("Diet/VertebrateFoodAvail attribute group not found in input file %s.\n", fileName);

	/* Create a node for each functional group*/
	for (guild = 0; guild < bm->K_num_tot_sp; guild++) {
		//if (FunctGroupArray[guild].isVertebrate == TRUE) {
        if ((FunctGroupArray[guild].groupAgeType == AGE_STRUCTURED) || (FunctGroupArray[guild].groupAgeType == AGE_STRUCTURED_BIOMASS)){

			specieNode = Util_XML_Get_Node(ATLANTIS_GROUP_ATTRIBUTE, attributeNode, FunctGroupArray[guild].groupCode);
			if (specieNode == NULL)
				quit("Diet/VertebrateFoodAvail/%s species attribute group not found.\n", FunctGroupArray[guild].groupCode);

			for (cohort = 0; cohort < FunctGroupArray[guild].numStages; cohort++) {
				sprintf(errorString, "Diet/VertebrateFoodAvail/%s/%s", FunctGroupArray[guild].groupCode, cohortString[cohort]);
				cohortNode = Util_XML_Get_Node(ATLANTIS_AGE_CLASS_ATTRIBUTE, specieNode, cohortString[cohort]);
				if (cohortNode == NULL)
					quit("%s cohort attribute group not found.\n", errorString);

				for (preyCohort = 0; preyCohort < FunctGroupArray[guild].numStages; preyCohort++) {

					if (Util_XML_Read_Array_Double(ATLANTIS_AGE_CLASS_ATTRIBUTE, fileName, errorString, cohortNode, proportion_check, cohortString[preyCohort], &values,
							bm->K_num_tot_sp) == FALSE) {
						quit("Error: Unable to find parameter '%s/%s/%s' in input file %s\n", errorString, cohortString[preyCohort], fileName);
					}

					for (b = 0; b < bm->K_num_tot_sp; b++){
						bm->pSPVERTeat[guild][b][cohort][preyCohort] = values[b];
						if(bm->flag_olddiet){
							if (!FunctGroupArray[guild].pSPEat[0][b][WC] && bm->pSPVERTeat[guild][b][cohort][preyCohort] > 0) {
								FunctGroupArray[guild].pSPEat[cohort][b][WC] = values[b];  // This may get overwritten in following setup checks
								if(FunctGroupArray[b].isDetritus == FALSE)
									FunctGroupArray[guild].pSPEat[cohort][b][SED] = values[b];
								FunctGroupArray[guild].pSPEat[cohort][b][EPIFAUNA] = values[b];
							}
						} else {
							FunctGroupArray[guild].pSPEat[cohort][b][WC] = values[b];  // This may get overwritten in following setup checks
							if(FunctGroupArray[b].isDetritus == FALSE)
								FunctGroupArray[guild].pSPEat[cohort][b][SED] = values[b];
							FunctGroupArray[guild].pSPEat[cohort][b][EPIFAUNA] = values[b];
						}

                        /*
						if(guild == bm->which_check){
							fprintf(bm->logFile, "Read_Vertebrate_Food_Avail: %s on %s cohort: %d, preycohort: %d, bm->pSPVERTeat = %.20e, pSPEat: %.20e\n",
								FunctGroupArray[guild].groupCode, FunctGroupArray[b].groupCode, cohort, preyCohort, bm->pSPVERTeat[guild][b][cohort][preyCohort], FunctGroupArray[guild].pSPEat[cohort][b][WC]);
						}
                        */
					}
					free(values);
				}
			}
		}
	}
}

/**
 *	\brief Read in the invertebrate food availability data.
 *
 * OLD VERSION BEFORE MOVED AGE_STRUCTURED_BIOMASS TO Read_Verebrate_Food_Avail()
 *
static void Read_Invertebrate_Food_Avail(MSEBoxModel *bm, char *fileName, xmlNodePtr parent) {

	double *values = 0;
	char errorString[STRLEN];
	int cohort, guild, b;
	// int preyCohort;
	xmlNodePtr attributeNode, specieNode;

	if (verbose)
		printf("Reading InvertebrateFoodAvail values\n");

	attributeNode = Util_XML_Get_Node(ATLANTIS_ATTRIBUTE, parent, "InvertebrateFoodAvail");
	if (attributeNode == NULL)
		quit("Diet/InvertebrateFoodAvail attribute group not found in input file %s.\n", fileName);

	// Create a node for each functional group
	for (guild = 0; guild < bm->K_num_tot_sp; guild++) {
		if ((int)FunctGroupArray[guild].speciesParams[flag_id] == TRUE) {
			if (FunctGroupArray[guild].isVertebrate == FALSE && FunctGroupArray[guild].isPredator == TRUE) {

				switch (FunctGroupArray[guild].groupAgeType) {
				case AGE_STRUCTURED_BIOMASS:

					sprintf(errorString, "Diet/InvertebrateFoodAvail/%s", FunctGroupArray[guild].groupCode);
					specieNode = Util_XML_Get_Node(ATLANTIS_GROUP_ATTRIBUTE, attributeNode, FunctGroupArray[guild].groupCode);
					if (specieNode == NULL)
						quit("%s species attribute group not found.\n", errorString);

					// Predator cohort
					for (cohort = 0; cohort < FunctGroupArray[guild].numStages; cohort++) {

						if (Util_XML_Read_Array_Double(ATLANTIS_AGE_CLASS_ATTRIBUTE, fileName, errorString, specieNode, proportion_check, cohortString[cohort], &values,
								bm->K_num_tot_sp) == FALSE) {
							quit("Error: Unable to find parameter '%s/%s' in input file %s\n", errorString,
									cohortString[cohort], fileName);

						}
						for (b = 0; b < bm->K_num_tot_sp; b++) {
							bm->pSPVERTeat[guild][b][cohort][juv_id] = values[b];
							bm->pSPVERTeat[guild][b][cohort][adult_id] = values[b];
							FunctGroupArray[guild].pSPEat[cohort][b][WC] = values[b];

//							if(guild == bm->which_check){
//								for(preyCohort=0; preyCohort<adult_id+1; preyCohort++){
//									fprintf(bm->logFile, "Read_Invertebrate_Food_Avail: %s on %s b: %d, cohort: %d, preyCohort: %d, pSPEat: %.20e, pSPVERTeat: %.20e\n",
//										FunctGroupArray[guild].groupCode, FunctGroupArray[b].groupCode, b, cohort, preyCohort, FunctGroupArray[guild].pSPEat[cohort][b][WC], bm->pSPVERTeat[guild][b][cohort][preyCohort]);
//								}
//							}

						}
						free(values);
					}
					break;
				case BIOMASS:
					cohort = 0;
					if (Util_XML_Read_Array_Double(ATLANTIS_GROUP_ATTRIBUTE, fileName, "Diet/InvertebrateFoodAvail", attributeNode, proportion_check, FunctGroupArray[guild].groupCode, &values,
							bm->K_num_tot_sp) == FALSE) {
						quit("Error: Unable to find parameter 'Diet/InvertebrateFoodAvail/%s' in input file %s\n", FunctGroupArray[guild].groupCode, fileName);
					}

					for (b = 0; b < bm->K_num_tot_sp; b++) {
						bm->pSPVERTeat[guild][b][juv_id][juv_id] = values[b];
						bm->pSPVERTeat[guild][b][juv_id][adult_id] = values[b];

						bm->pSPVERTeat[guild][b][adult_id][juv_id] = values[b];
						bm->pSPVERTeat[guild][b][adult_id][adult_id] = values[b];

						FunctGroupArray[guild].pSPEat[cohort][b][WC] = values[b];
						//if(b == PBguild_id || b == BBguild_id)
						//	fprintf(bm->logFile, "Read_Invertebrate_Food_Avail: FunctGroupArray[%s].pSPEat[0][%d - %s][WC] = %e, pSPVERTeat: %e\n",
						//			FunctGroupArray[guild].groupCode, b, FunctGroupArray[b].groupCode, FunctGroupArray[guild].pSPEat[0][b][WC], bm->pSPVERTeat[guild][b][cohort][0]);

					}
					free(values);
					break;
				case AGE_STRUCTURED:
					// Do nothing
					break;
				}
			}
		}
	}
}
 */

 /**
 *	\brief Read in the invertebrate food availability data.
 */
static void Read_Invertebrate_Food_Avail(MSEBoxModel *bm, char *fileName, xmlNodePtr parent) {
 
    double *values = 0;
    //char errorString[STRLEN];
    int cohort, guild, b;
    // int preyCohort;
    xmlNodePtr attributeNode;
 
    if (verbose)
        printf("Reading InvertebrateFoodAvail values\n");
 
    attributeNode = Util_XML_Get_Node(ATLANTIS_ATTRIBUTE, parent, "InvertebrateFoodAvail");
    if (attributeNode == NULL)
        quit("Diet/InvertebrateFoodAvail attribute group not found in input file %s.\n", fileName);
 
    // Create a node for each functional group
    for (guild = 0; guild < bm->K_num_tot_sp; guild++) {
        if ((int)FunctGroupArray[guild].speciesParams[flag_id] == TRUE) {
            if ((FunctGroupArray[guild].groupAgeType == BIOMASS) && (FunctGroupArray[guild].isPredator == TRUE)) {
 
                cohort = 0;
                if (Util_XML_Read_Array_Double(ATLANTIS_GROUP_ATTRIBUTE, fileName, "Diet/InvertebrateFoodAvail", attributeNode, proportion_check, FunctGroupArray[guild].groupCode, &values, bm->K_num_tot_sp) == FALSE) {
                    quit("Error: Unable to find parameter 'Diet/InvertebrateFoodAvail/%s' in input file %s\n", FunctGroupArray[guild].groupCode, fileName);
                }
 
                for (b = 0; b < bm->K_num_tot_sp; b++) {
                    bm->pSPVERTeat[guild][b][juv_id][juv_id] = values[b];
                    bm->pSPVERTeat[guild][b][juv_id][adult_id] = values[b];
 
                    bm->pSPVERTeat[guild][b][adult_id][juv_id] = values[b];
                    bm->pSPVERTeat[guild][b][adult_id][adult_id] = values[b];
 
                    FunctGroupArray[guild].pSPEat[cohort][b][WC] = values[b];
                    //if(b == PBguild_id || b == BBguild_id)
                    //	fprintf(bm->logFile, "Read_Invertebrate_Food_Avail: FunctGroupArray[%s].pSPEat[0][%d - %s][WC] = %e, pSPVERTeat: %e\n",
                    //			FunctGroupArray[guild].groupCode, b, FunctGroupArray[b].groupCode, FunctGroupArray[guild].pSPEat[0][b][WC], bm->pSPVERTeat[guild][b][cohort][0]);
                }
                free(values);
            }
        }
    }
 }
 
 
/**
 *	\brief Read in the age diet availability data. There won't be data for all groups
 *	here. In the future we might add functionality to allow users to provide this for
 *	all functional groups.
 */
static void Read_Age_Diet_Avail_XML(MSEBoxModel *bm, char *fileName, xmlNodePtr parent) {

	double *values = 0;
	char errorString[STRLEN];
	int guild, vertGuild, b;
	xmlNodePtr attributeNode, specieNode;

	if (verbose)
		printf("Reading AgeDietAvail values\n");

	attributeNode = Util_XML_Get_Node(ATLANTIS_ATTRIBUTE, parent, "AgeDietAvail");
	if (attributeNode == NULL)
		quit("Diet/AgeDietAvail attribute group not found in input file %s.\n", fileName);

	for (vertGuild = 0; vertGuild < bm->K_num_tot_sp; vertGuild++) {
		if ((FunctGroupArray[vertGuild].isVertebrate) && (FunctGroupArray[vertGuild].speciesParams[age_structured_prey_id] > 0)) {

			sprintf(errorString, "Diet/AgeDietAvail/%s", FunctGroupArray[vertGuild].groupCode);
			specieNode = Util_XML_Get_Node(ATLANTIS_GROUP_ATTRIBUTE, attributeNode, FunctGroupArray[vertGuild].groupCode);
			if (specieNode == NULL)
				quit("%s species attribute group not found.\n", errorString);

			for (guild = 0; guild < bm->K_num_tot_sp; guild++) {

				//TODO: Can this be simplified?
				if (FunctGroupArray[guild].groupType == LG_ZOO || FunctGroupArray[guild].groupType == MED_ZOO || FunctGroupArray[guild].groupType == SM_ZOO || FunctGroupArray[guild].groupType
						== LG_INF || FunctGroupArray[guild].groupType == SED_EP_FF || FunctGroupArray[guild].groupType == SED_EP_OTHER
						|| FunctGroupArray[guild].groupType == MOB_EP_OTHER) {

					if(bm->flag_fine_ontogenetic_diets){
						if (Util_XML_Read_Array_Double(ATLANTIS_GROUP_ATTRIBUTE, fileName, errorString, specieNode, proportion_check, FunctGroupArray[guild].groupCode,
								&values, FunctGroupArray[vertGuild].numCohorts) == FALSE) {
							quit("Error: Unable to find parameter '%s/%s' in input file %s\n", errorString,
									FunctGroupArray[guild].groupCode, fileName);
						}

						for (b = 0; b < FunctGroupArray[vertGuild].numCohorts; b++)
							bm->pSPageeat[vertGuild][b][guild] = values[b];

						free(values);
					} else {
						for (b = 0; b < FunctGroupArray[vertGuild].numCohorts; b++)
							bm->pSPageeat[vertGuild][b][guild] = 0.0;
					}
				}
			}
		}
	}
}

/**
 *	\brief Read in the age diet availability data. There won't be data for all groups
 *	here. In the future we might add functionality to allow users to provide this for
 *	all functional groups.
 */
static void Read_Seagrass_Diet_Avail_XML(MSEBoxModel *bm, char *fileName, xmlNodePtr parent) {
    double *values = 0;
    int sp, b, cohort;
    xmlNodePtr attributeNode;
    xmlNodePtr specieNode, cohortNode;
    char errorString[STRLEN];
    char stageString[10];


    if (verbose)
        printf("Reading AgeDietAvail values\n");

    attributeNode = Util_XML_Get_Node(ATLANTIS_ATTRIBUTE, parent, "SeagrassFoodAvail");
    if (attributeNode == NULL)
        quit("Diet/SeagrassFoodAvail attribute group not found in input file %s.\n", fileName);

    for (sp = 0; sp < bm->K_num_tot_sp; sp++) {
        if(FunctGroupArray[sp].groupType == SEAGRASS){

            sprintf(errorString, "Diet/SeagrassFoodAvail/%s", FunctGroupArray[sp].groupCode);
            specieNode = Util_XML_Get_Node(ATLANTIS_GROUP_ATTRIBUTE, attributeNode, FunctGroupArray[sp].groupCode);
            if (specieNode == NULL)
                quit("%s species attribute group not found.\n", errorString);
            
            for (cohort = 0; cohort < FunctGroupArray[sp].numStages; cohort++) {
                sprintf(stageString, "stage%d", cohort);
                sprintf(errorString, "Diet/SeagrassFoodAvail/%s/stage%d", FunctGroupArray[sp].groupCode, cohort);
                cohortNode = Util_XML_Get_Node(ATLANTIS_AGE_CLASS_ATTRIBUTE, specieNode, stageString);
                if (cohortNode == NULL)
                    quit("%s cohort attribute group not found.\n", errorString);
                
                if (Util_XML_Read_Array_Double(ATLANTIS_AGE_CLASS_ATTRIBUTE, fileName, errorString, cohortNode, proportion_check, stageString, &values, bm->K_num_tot_sp) == FALSE) {
                    quit("Error: Unable to find parameter 'Diet/SeagrassFoodAvail/%s' in input file %s\n", errorString, stageString, fileName);
                }
                
                // Store the values
                for (b = 0; b < bm->K_num_tot_sp; b++){
                    FunctGroupArray[b].pSP_SG_eat[cohort] = values[b];
                }
                free(values);
            }
        }
    }
}

/**
 *	\brief Read the data into the recruit_vdistrib array from the input XML file.
 */
static void Read_PStock_XML(MSEBoxModel *bm, char *fileName, xmlNodePtr parent) {

	double *values = 0;
	int guild, b, cohort;
	int counter;
	char errorString[STRLEN];
	xmlNodePtr attributeNode, specieNode;

	if (verbose)
		printf("Reading pStock values\n");

	attributeNode = Util_XML_Get_Node(ATLANTIS_ATTRIBUTE, parent, "pStock");
	if (attributeNode == NULL)
		quit("Diet/pStock attribute group not found in input file %s.\n", fileName);

	/* Create a node for each functional group*/
	for (guild = 0; guild < bm->K_num_tot_sp; guild++) {
		if (FunctGroupArray[guild].isVertebrate == TRUE) {

			sprintf(errorString, "Diet/pStock/%s", FunctGroupArray[guild].groupCode);
			specieNode = Util_XML_Get_Node(ATLANTIS_GROUP_ATTRIBUTE, attributeNode, FunctGroupArray[guild].groupCode);
			if (specieNode == NULL)
				quit("%s species attribute group not found.\n", errorString);

			for (cohort = 0; cohort < FunctGroupArray[guild].numStages; cohort++) {

				counter = FunctGroupArray[guild].numStocks;
				if (Util_XML_Read_Array_Double(ATLANTIS_AGE_CLASS_ATTRIBUTE, fileName, errorString, specieNode, no_checking, cohortString[cohort], &values, counter) == FALSE) {
					quit("Error: Unable to find parameter '%s/%s' in input file %s\n", errorString, cohortString[cohort],
							fileName);
				}

				for (b = 0; b < counter; b++) {
					pSTOCK[guild][b][cohort] = values[b];
				}
				free(values);
			}
		}
	}
}

/**
 *	\brief Read data into the 2 d Array. Assumes the guild is the first index in the array.
 */
static void Read_Invertebrate_SN_XML(MSEBoxModel *bm, char *fileName, xmlNodePtr parent) {

	double *values = 0;
	int guild, b;
	xmlNodePtr attributeNode;

	if (verbose)
		printf("Reading InvertebrateSN values\n");

	attributeNode = Util_XML_Get_Node(ATLANTIS_ATTRIBUTE, parent, "InvertebrateSN");
	if (attributeNode == NULL)
		quit("Diet/InvertebrateSN attribute group not found in input file %s.\n", fileName);

	/* Clearance rates for invertebrate consumers */
	for (guild = 0; guild < bm->K_num_tot_sp; guild++) {
		if (FunctGroupArray[guild].groupAgeType == AGE_STRUCTURED_BIOMASS || FunctGroupArray[guild].groupAgeType == BIOMASS) {
			if (FunctGroupArray[guild].isPredator == TRUE){

				if (Util_XML_Read_Array_Double(ATLANTIS_GROUP_ATTRIBUTE, fileName, "Diet/InvertebrateSN", attributeNode, no_checking, FunctGroupArray[guild].groupCode, &values,
						FunctGroupArray[guild].numCohorts) == FALSE) {
					quit("Error: Unable to find parameter 'Diet/InvertebrateSN/%s' in input file %s\n", FunctGroupArray[guild].groupCode, fileName);
				}
				for (b = 0; b < FunctGroupArray[guild].numCohorts; b++) {
					FunctGroupArray[guild].sn[b] = values[b];
				}
				free(values);
			}else{
				FunctGroupArray[guild].sn[juv_id] = 0;
			}
		}
	}
}

/**
 *	\brief Read in the vertebrate clearance rate data. This data is stored in the SP_C parameter.
 */
static void Read_Clearance_Rates_XML(MSEBoxModel *bm, char *fileName, xmlNodePtr parent) {

	double numsec = 86400.0; /* number of seconds in a day */
	double *values = 0;
	double final_val, stdev_map_param, sp_std_dev;
	int guild, b, cohort, ngene;
	xmlNodePtr attributeNode;

	if (verbose)
		printf("Reading ClearanceRate values\n");

	attributeNode = Util_XML_Get_Node(ATLANTIS_ATTRIBUTE, parent, "ClearanceRate");
	if (attributeNode == NULL)
		quit("Diet/ClearanceRate attribute group not found in input file %s.\n", fileName);

	/* Create a node for each functional group*/
	for (guild = 0; guild < bm->K_num_tot_sp; guild++) {
		if (FunctGroupArray[guild].isVertebrate == TRUE) {

			if (Util_XML_Read_Array_Double(ATLANTIS_GROUP_ATTRIBUTE, fileName, "Diet/ClearanceRate", attributeNode, no_checking, FunctGroupArray[guild].groupCode, &values,
					FunctGroupArray[guild].numCohorts) == FALSE) {
				quit("Error: Unable to find parameter 'Diet/ClearanceRate/%s' in input file %s\n", FunctGroupArray[guild].groupCode, fileName);
			}

			for (cohort = 0; cohort < FunctGroupArray[guild].numCohortsXnumGenes; cohort++) {
				b = (int)floor(cohort / FunctGroupArray[guild].numGeneTypes);
				ngene = cohort - (b * FunctGroupArray[guild].numGeneTypes);
				final_val = values[b];
				if((bm->flag_do_evolution || bm->flag_mult_grow_curves) && (DNA[guild].trait_active[growth_trait_id] > 0)){
					stdev_map_param = DNA[guild].stdev_gene_map[ngene];
					sp_std_dev = FunctGroupArray[guild].speciesParams[trait_variance_id];
					final_val = values[b] + sp_std_dev * stdev_map_param * values[b];
				}
				FunctGroupArray[guild].C_T15[cohort] = final_val / numsec; // Add so can store original values for use in evolution
				FunctGroupArray[guild].SP_C[cohort] = final_val / numsec;
				FunctGroupArray[guild].SP_C_per_day[cohort] = final_val;
			}
			free(values);
		}
	}
}

/**
 *	\brief Read in the invertebrate clearance rates. These are only read in for the grazing functional groups.
 */
static void Read_Invert_Clearance_Rates_XML(MSEBoxModel *bm, char *fileName, xmlNodePtr parent) {

	double numsec = 86400.0; /* number of seconds in a day */
	double *values = 0;
	int guild, b, cohort;
	xmlNodePtr attributeNode;

	if (verbose)
		printf("Reading InvertebrateClearanceRate values\n");

	attributeNode = Util_XML_Get_Node(ATLANTIS_ATTRIBUTE, parent, "InvertebrateClearanceRate");
	if (attributeNode == NULL)
		quit("Diet/InvertebrateClearanceRate attribute group not found in input file %s.\n", fileName);

	/* Clearance rates for invertebrate consumers */
	for (guild = 0; guild < bm->K_num_tot_sp; guild++) {
		if (FunctGroupArray[guild].isVertebrate == FALSE){
			if (FunctGroupArray[guild].isPredator == TRUE) {

				if (Util_XML_Read_Array_Double(ATLANTIS_GROUP_ATTRIBUTE, fileName, "Diet/InvertebrateClearanceRate", attributeNode, no_checking, FunctGroupArray[guild].groupCode, &values,
						FunctGroupArray[guild].numCohorts) == FALSE) {
					quit("Error: Unable to find parameter 'Diet/InvertebrateClearanceRate/%s' in input file %s\n", FunctGroupArray[guild].groupCode, fileName);
				}

				for (cohort = 0; cohort < FunctGroupArray[guild].numCohortsXnumGenes; cohort++) {
					b = (int)floor(cohort / FunctGroupArray[guild].numGeneTypes);
					FunctGroupArray[guild].C_T15[cohort] = values[b] / numsec;  // Retained for use in evolution
					FunctGroupArray[guild].C_T15_per_day[cohort] = values[b];
                    FunctGroupArray[guild].SP_C[cohort] = values[b] / numsec;   // Invertebrates now use this as well

                    //printf("Reading clearance rate for %s cohort %d - value %e\n", FunctGroupArray[guild].name, b, values[b]);

					// Sanity check otherwise get a divide by zero pretty quickly
					if(values[b] == 0.0){
						if (FunctGroupArray[guild].numCohorts > 1)
							quit("You have a zero clearance rate for %s cohort %d\n", FunctGroupArray[guild].groupCode, b);
						else
							quit("You have a zero clearance rate for %s\n", FunctGroupArray[guild].groupCode);
					}

				}
				free(values);
			}
		}
	}
}

/**
 *	\brief Read in the invertebrate maximum growth rates.
 */
static void Read_Invert_Growth_Rates_XML(MSEBoxModel *bm, char *fileName, xmlNodePtr parent) {

	double numsec = 86400.0; /* number of seconds in a day */
	double *values = 0;
	double final_val, stdev_map_param, sp_std_dev;
	int guild, b, cohort, ngene;
	xmlNodePtr attributeNode;

	if (verbose)
		printf("Reading InvertebrateGrowthRate values\n");

	attributeNode = Util_XML_Get_Node(ATLANTIS_ATTRIBUTE, parent, "InvertebrateGrowthRate");
	if (attributeNode == NULL)
		quit("Diet/InvertebrateGrowthRate attribute group not found in input file %s.\n", fileName);

	/* Create a node for each functional group*/
	/* Clearance rates for invertebrate consumers */
	for (guild = 0; guild < bm->K_num_tot_sp; guild++) {
		//if (FunctGroupArray[guild].isVertebrate == FALSE){
        if (FunctGroupArray[guild].numCohorts == 1){
			if(FunctGroupArray[guild].isPredator == TRUE|| FunctGroupArray[guild].isBacteria == TRUE || FunctGroupArray[guild].isPrimaryProducer == TRUE) {
				if (Util_XML_Read_Array_Double(ATLANTIS_GROUP_ATTRIBUTE, fileName, "Diet/InvertebrateGrowthRate", attributeNode, no_checking, FunctGroupArray[guild].groupCode, &values,
						FunctGroupArray[guild].numCohorts) == FALSE) {
					quit("Error: Unable to find parameter 'Diet/InvertebrateGrowthRate/%s' in input file %s\n", FunctGroupArray[guild].groupCode, fileName);
				}

				for (cohort = 0; cohort < FunctGroupArray[guild].numCohortsXnumGenes; cohort++) {
					b = (int)floor(cohort / FunctGroupArray[guild].numGeneTypes);
					ngene = cohort - (b * FunctGroupArray[guild].numGeneTypes);
					final_val = values[b];
					if((bm->flag_do_evolution || bm->flag_mult_grow_curves) && (DNA[guild].trait_active[consumption_trait_id] > 0)){
						stdev_map_param = DNA[guild].stdev_gene_map[ngene];
						sp_std_dev = FunctGroupArray[guild].speciesParams[trait_variance_id];
						final_val = values[b] + sp_std_dev * stdev_map_param * values[b];
					}
                    
                    FunctGroupArray[guild].mum[cohort] = final_val / numsec;  // Added - was not originall tracked, now tracked so that can store original values in mum_T15 for evolution
					FunctGroupArray[guild].mum_T15[cohort] = final_val / numsec;
					FunctGroupArray[guild].mum_T15_per_day[cohort] = final_val;

					// Sanity check otherwise get a divide by zero pretty quickly
					if(FunctGroupArray[guild].speciesParams[flag_id]){
						if(values[b] == 0.0){
							if (FunctGroupArray[guild].numCohorts > 1)
								quit("You have a zero growth rate for %s cohort %d\n", FunctGroupArray[guild].groupCode, b);
							else
								quit("You have a zero growth rate for %s\n", FunctGroupArray[guild].groupCode);
						}
					}
				}
				free(values);
			}
		}
	}
}

/**
 *	\brief Read data into the 2 d Array. Assumes the guild is the first index in the array.
 */
static void Read_Vert_Growth_Rate_XML(MSEBoxModel *bm, char *fileName, xmlNodePtr parent) {

	double numsec = 86400.0; /* number of seconds in a day */
	double final_val, stdev_map_param, sp_std_dev;
	double *values = 0;
	int guild, b, cohort, ngene;
	xmlNodePtr attributeNode;

	if (verbose)
		printf("Reading VertebrateGrowthRate values\n");

	attributeNode = Util_XML_Get_Node(ATLANTIS_ATTRIBUTE, parent, "VertebrateGrowthRate");
	if (attributeNode == NULL)
		quit("Diet/VertebrateGrowthRate attribute group not found in input file %s.\n", fileName);

	/* Create a node for each functional group*/
	for (guild = 0; guild < bm->K_num_tot_sp; guild++) {
		//if (FunctGroupArray[guild].isVertebrate == TRUE) {
        if (FunctGroupArray[guild].numCohorts > 1) {
			if (Util_XML_Read_Array_Double(ATLANTIS_GROUP_ATTRIBUTE, fileName, "Diet/VertebrateGrowthRate", attributeNode, no_checking, FunctGroupArray[guild].groupCode, &values,
					FunctGroupArray[guild].numCohorts) == FALSE) {
				quit("Error: Unable to find parameter 'Diet/VertebrateGrowthRate/%s' in input file %s\n", FunctGroupArray[guild].groupCode, fileName);
			}

			for (cohort = 0; cohort < FunctGroupArray[guild].numCohortsXnumGenes; cohort++) {
				b = (int)floor(cohort / FunctGroupArray[guild].numGeneTypes);
				ngene = cohort - (b * FunctGroupArray[guild].numGeneTypes);
				final_val = values[b];
				if((bm->flag_do_evolution || bm->flag_mult_grow_curves) && (DNA[guild].trait_active[growth_trait_id] > 0)){
					stdev_map_param = DNA[guild].stdev_gene_map[ngene];
					sp_std_dev = FunctGroupArray[guild].speciesParams[trait_variance_id];
					final_val = values[b] + sp_std_dev * stdev_map_param * values[b];
				}
				FunctGroupArray[guild].mum[cohort] = final_val / numsec;
                FunctGroupArray[guild].mum_T15[cohort] = final_val / numsec;  // To store original values in case of evolution
				FunctGroupArray[guild].mum_per_day[cohort] = final_val;

			}

			free(values);
		}
	}
}

/**
 *    \brief Read the data into the suuplemented diet array from the input XML file.
 */
static void Read_SuppDiet_Dist_XML(MSEBoxModel *bm, char *fileName, xmlNodePtr parent) {
    
    double *values = 0;
    int guild, b;
    xmlNodePtr attributeGroup;
    char *nodeName;
    
    nodeName =  Util_Get_Node_Name(parent);
    if (verbose)
        printf("Reading supplemented diet values\n");
    
    attributeGroup = Util_XML_Get_Node(ATLANTIS_ATTRIBUTE, parent, "Supplemental_Diets");
    if (attributeGroup == NULL)
        quit("Supplemental_Diets attribute group not found in input file %s.\n", fileName);
    
    /* Create a node for each functional group*/
    for (guild = 0; guild < bm->K_num_tot_sp; guild++) {
        if (FunctGroupArray[guild].speciesParams[flag_id] == TRUE) {
            if(FunctGroupArray[guild].isSupplemented){
                // Read the data into a temp array.
                if (Util_XML_Read_Array_Double(ATLANTIS_GROUP_ATTRIBUTE, fileName, "Supplemental_Diets", attributeGroup, no_checking, FunctGroupArray[guild].groupCode, &values, bm->nbox) == FALSE) {
                    quit("Error: Unable to find parameter Supplemental_Diets/%s in input file %s\n", FunctGroupArray[guild].groupCode, fileName);
                }
                
                for (b = 0; b < bm->nbox; b++) {
                    SUPPdistrib[guild][b] = values[b];
                }
                free(values);
            }
        }
    }
    free(nodeName);
}

/**
 *	\brief Read data into the 2 d Array. Assumes the guild is the first index in the array.
 */

static void Read_Diet_Information_Values(MSEBoxModel *bm, char *fileName, xmlNodePtr parent) {
	xmlNodePtr attributeGroupNode, attributeGroup, catchNode;
	int sp, prey, pred, chrtstage, preyage, nf;
	double *values;

	if (verbose)
		printf("Reading the Diet values\n");

	attributeGroupNode = Util_XML_Get_Node(ATLANTIS_ATTRIBUTE_SUB_GROUP, parent, "Diet");
	if (attributeGroupNode == NULL)
		quit("Diet attribute group not found in input file %s.\n", fileName);

	Util_XML_Read_Species_Param(bm, fileName, attributeGroupNode, predcase_id);

	/* Check the predcase of the invertebrates */
    bm->need_predcomp_params = 0;
    bm->need_tot_pred = 0;
    for(sp = 0; sp < bm->K_num_tot_sp; sp++){
		if(FunctGroupArray[sp].isVertebrate == FALSE){
			if(FunctGroupArray[sp].isPredator == TRUE){
                switch ((int) (FunctGroupArray[sp].speciesParams[predcase_id])) {
                    case eat_ecosim:
                        quit("Invalid predcase for %s\n", FunctGroupArray[sp].groupCode);
                        break;
                    case eat_ratio_dependent:
                    case eat_crowley_martin:
                        bm->need_predcomp_params = 1;
                        bm->need_tot_pred = 1;
                        if (!bm->flag_predratiodepend)
                            quit("You have a predcase mismatch - flag_predratiodepend is set to 0 but at least one predcase is set to %d. If you want to keep that predcase set flag_predratiodepend to 1. If you do not actually want to be using raito dependent functional feeding responses then set the predcase of %s to another setting (e.g. one of the Holling classes)\n", (int) (FunctGroupArray[sp].speciesParams[predcase_id]), FunctGroupArray[sp].groupCode);
                        break;
                    case eat_hassel_varley:
                        bm->need_tot_pred = 1;
                        if (!bm->flag_predratiodepend)
                            quit("You have a predcase mismatch - flag_predratiodepend is set to 0 but at least one predcase is set to %d. If you want to keep that predcase set flag_predratiodepend to 1. If you do not actually want to be using raito dependent functional feeding responses then set the predcase of %s to another setting (e.g. one of the Holling classes)\n", (int) (FunctGroupArray[sp].speciesParams[predcase_id]), FunctGroupArray[sp].groupCode);
                        break;
                    default:
                        if (bm->flag_predratiodepend)
                            quit("You have a predcase mismatch - flag_predratiodepend is set to 1 but the predcase for %s is not set to a ratio dependent functional feeding responses setting (%d). You need to reset the predcase or flag_predratiodepend\n", FunctGroupArray[sp].groupCode, (int) (FunctGroupArray[sp].speciesParams[predcase_id]));
                        break;
                }
            }
		}
	}
    
	//Util_XML_Read_Species_Param(bm, fileName, attributeGroupNode, catcheater_id);
	Util_XML_Read_Species_Param(bm, fileName, attributeGroupNode, flagactive_id);

    Read_Detritus_Sediment_Food_Avail(bm, fileName, attributeGroupNode);
    Read_Vertebrate_Food_Avail(bm, fileName, attributeGroupNode);
    Read_Invertebrate_Food_Avail(bm, fileName, attributeGroupNode);
    
	if(bm->flag_fine_ontogenetic_diets){
		Util_XML_Read_Species_Param(bm, fileName, attributeGroupNode, age_structured_prey_id);
		Util_XML_Read_Species_Param(bm, fileName, attributeGroupNode, p_split_id);
		Read_Age_Diet_Avail_XML(bm, fileName, attributeGroupNode);
	}
    
    if(bm->flag_macro_model){
		Read_Seagrass_Diet_Avail_XML(bm, fileName, attributeGroupNode);
	}
    
    if(bm->flag_import_feed) {
        Read_SuppDiet_Dist_XML(bm, fileName, attributeGroupNode);
    }
    
	/* Enter diet parameters for fish catch */
	for (pred = 0; pred < bm->K_num_tot_sp; pred++) {
		if (FunctGroupArray[pred].isVertebrate == TRUE) {
			for (prey = 0; prey < bm->K_num_tot_sp; prey++) {
				if (FunctGroupArray[prey].isVertebrate == TRUE) {

					for (chrtstage = 0; chrtstage < FunctGroupArray[pred].numStages; chrtstage++) {
						for (preyage = 0; preyage < FunctGroupArray[prey].numStages; preyage++) {
							bm->pSPFCeat[pred][prey][chrtstage][preyage] = bm->pSPVERTeat[pred][prey][chrtstage][preyage];
						}
					}
				}
			}
		}
	}


    if(bm->flagIsEstuary) {
       
        Util_XML_Read_Species_Param(bm, fileName, attributeGroupNode, turbid_refuge_id);
        
        attributeGroup = Util_XML_Get_Node(ATLANTIS_ATTRIBUTE_SUB_GROUP, attributeGroupNode, "Turbidity");        
        Read_Cohort_Species_Param_Values(bm, fileName, attributeGroup, L_turbid_id);
        Read_Cohort_Species_Param_Values(bm, fileName, attributeGroup, a_turbid_id);
        Read_Cohort_Species_Param_Values(bm, fileName, attributeGroup, b_turbid_id);
                
    }
    
    if(bm->containsMammals == TRUE  && bm->flag_fisheries_on){
		attributeGroup = Util_XML_Get_Node(ATLANTIS_ATTRIBUTE_SUB_GROUP, attributeGroupNode, "Catch_Opportunity");
		if (attributeGroup == NULL)
			quit("Diet/Catch_Opportunity attribute group not found in input file %s.\n", fileName);

		/* Availabilty of catch to opportunistic catch grazers (thieves) */
		catchNode = Util_XML_Get_Node(ATLANTIS_ATTRIBUTE, attributeGroup, "Catch_Availability");
		if (catchNode == NULL)
			quit("Diet/Catch_Opportunity/Catch_Availability attribute group not found in input file %s.\n", fileName);

		for (sp = 0; sp < bm->K_num_tot_sp; sp++) {
			if (FunctGroupArray[sp].speciesParams[catcheater_id] > 0) {
				if (Util_XML_Read_Array_Double(ATLANTIS_GROUP_ATTRIBUTE, fileName, "Diet/Catch_Opportunity/Catch_Availability", catchNode, no_checking, FunctGroupArray[sp].groupCode, &values,
						bm->K_num_tot_sp) == FALSE) {
					quit("Error: Unable to find parameter 'Diet/Catch_Opportunity/Catch_Availability/%s' in input file %s\n", FunctGroupArray[sp].groupCode,
							fileName);
				}

				for (prey = 0; prey < bm->K_num_tot_sp; prey++) {
					if (FunctGroupArray[prey].isVertebrate == TRUE) {
						for (chrtstage = 0; chrtstage < FunctGroupArray[sp].numStages; chrtstage++) {
							for (preyage = 0; preyage < FunctGroupArray[prey].numStages; preyage++) {
								bm->pSPFCeat[sp][prey][chrtstage][preyage] = values[prey];
							}
						}
					}
				}
				free(values);
			}
		}
		/* Availabilty of catch to opportunistic catch grazers (thieves) */
		catchNode = Util_XML_Get_Node(ATLANTIS_ATTRIBUTE, attributeGroup, "Proportion_Exploitable");
		if (catchNode == NULL)
			quit("Diet/Catch_Opportunity/Proportion_Exploitable attribute group not found in input file %s.\n", fileName);

		for (sp = 0; sp < bm->K_num_tot_sp; sp++) {
			if (FunctGroupArray[sp].speciesParams[catcheater_id] > 0) {
				if (Util_XML_Read_Array_Double(ATLANTIS_GROUP_ATTRIBUTE, fileName, "Diet/Catch_Opportunity/Proportion_Exploitable", catchNode, no_checking, FunctGroupArray[sp].groupCode, &values,
						bm->K_num_fisheries) == FALSE) {
					quit("Error: Unable to find parameter 'Diet/Catch_Opportunity/Proportion_Exploitable/%s' in input file %s\n", FunctGroupArray[sp].groupCode,
							fileName);
				}

				for (nf = 0; nf < bm->K_num_fisheries; nf++) {
					bm->pFLEET[sp][nf] = values[nf];
				}
				free(values);

			}
		}

	}


	/* Availability of bacteria being cleaned from ingested detritus is done on
	 a general medium rather than group based level */

	attributeGroup = Util_XML_Get_Node(ATLANTIS_ATTRIBUTE_SUB_GROUP, attributeGroupNode, "Bacteria_availability");
	if (attributeGroup == NULL)
		quit("Diet/Bacteria_availability attribute group not found in input file %s.\n", fileName);

	p_BBfish = Util_XML_Read_Value(fileName, ATLANTIS_ATTRIBUTE, bm->ecotest, 1, attributeGroup, proportion_check, "p_BBfish");
	p_BBben = Util_XML_Read_Value(fileName, ATLANTIS_ATTRIBUTE, bm->ecotest, 1, attributeGroup, proportion_check, "p_BBben");
	p_PBwc = Util_XML_Read_Value(fileName, ATLANTIS_ATTRIBUTE, bm->ecotest, 1, attributeGroup, proportion_check, "p_PBwc");
	p_PBben = Util_XML_Read_Value(fileName, ATLANTIS_ATTRIBUTE, bm->ecotest, 1, attributeGroup, proportion_check, "p_PBben");

	for (sp = 0; sp < bm->K_num_tot_sp; sp++) {
		if (FunctGroupArray[sp].isVertebrate == TRUE) {
			if (FunctGroupArray[sp].speciesParams[flagdem_id])
				FunctGroupArray[sp].speciesParams[PBscale_id] = p_PBben;
			else
				FunctGroupArray[sp].speciesParams[PBscale_id] = p_PBwc;
			FunctGroupArray[sp].speciesParams[BBscale_id] = p_BBfish;
		}
	}

	Util_XML_Read_Species_Param(bm, fileName, attributeGroupNode, KDEP_id);
	Util_XML_Read_Species_Param(bm, fileName, attributeGroupNode, KLP_id);
	Util_XML_Read_Species_Param(bm, fileName, attributeGroupNode, KUP_id);
	if(!bm->UseHardFeedingWindow){
		Util_XML_Read_Species_Param(bm, fileName, attributeGroupNode, Kmax_coefft_id);
	}
	Read_Invertebrate_SN_XML(bm, fileName, attributeGroupNode);

    //if(bm->flag_hassel_varley_on)
    //    Util_XML_Read_Species_Param(bm, fileName, attributeGroupNode, hvm_id);

	/* Detritus handling parameters (transfer rates to sediment by filter feeders,
	 and reference levels for omnivores supplementing on detritus) */

	attributeGroup = Util_XML_Get_Node(ATLANTIS_ATTRIBUTE_SUB_GROUP, attributeGroupNode, "Detritus_Handover");
	if (attributeGroup == NULL)
		quit("Diet/Detritus_Handover attribute group not found in input file %s.\n", fileName);

	Util_XML_Read_Species_Param(bm, fileName, attributeGroup, k_trans_id);
	k_refDL = Util_XML_Read_Value(fileName, ATLANTIS_ATTRIBUTE, bm->ecotest, 1, attributeGroup, no_checking, "k_refDL");
	k_refDR = Util_XML_Read_Value(fileName, ATLANTIS_ATTRIBUTE, bm->ecotest, 1, attributeGroup, no_checking, "k_refDR");
	k_refsDL = Util_XML_Read_Value(fileName, ATLANTIS_ATTRIBUTE, bm->ecotest, 1, attributeGroup, no_checking, "k_refsDL");

	bm->flagfishrates = (int) Util_XML_Read_Value(fileName, ATLANTIS_ATTRIBUTE, bm->ecotest, 1, attributeGroupNode, no_checking, "flagfishrates");

	Read_Clearance_Rates_XML(bm, fileName, attributeGroupNode);
	Read_Invert_Clearance_Rates_XML(bm, fileName, attributeGroupNode);

	attributeGroup = Util_XML_Get_Node(ATLANTIS_ATTRIBUTE_SUB_GROUP, attributeGroupNode, "Growth_Rates");
	if (attributeGroup == NULL)
		quit("Diet/Growth_Rates attribute group not found in input file %s.\n", fileName);

	Read_Vert_Growth_Rate_XML(bm, fileName, attributeGroup);
	Read_Invert_Growth_Rates_XML(bm, fileName, attributeGroup);

	Util_XML_Read_Species_Param(bm, fileName, attributeGroupNode, KL_id);
	Util_XML_Read_Species_Param(bm, fileName, attributeGroupNode, KU_id);
	Util_XML_Read_Species_Param(bm, fileName, attributeGroupNode, vl_id);
	Util_XML_Read_Species_Param(bm, fileName, attributeGroupNode, vla_T15_id);
	Util_XML_Read_Species_Param(bm, fileName, attributeGroupNode, vlb_id);
	Util_XML_Read_Species_Param(bm, fileName, attributeGroupNode, ht_id);
	Util_XML_Read_Species_Param(bm, fileName, attributeGroupNode, hta_id);
	Util_XML_Read_Species_Param(bm, fileName, attributeGroupNode, htb_id);
	Util_XML_Read_Species_Param(bm, fileName, attributeGroupNode, pR_id);
	Util_XML_Read_Species_Param(bm, fileName, attributeGroupNode, li_a_id);
	Util_XML_Read_Species_Param(bm, fileName, attributeGroupNode, li_b_id);
    
    if (bm->external_populations) {
        Util_XML_Read_Species_Param(bm, fileName, attributeGroupNode, linf_id);
        Util_XML_Read_Species_Param(bm, fileName, attributeGroupNode, Kbert_id);
        Util_XML_Read_Species_Param(bm, fileName, attributeGroupNode, tzero_id);
    }

	/* Read in li_a and li_b for inverts */
	bm->li_a_invert = Util_XML_Read_Value(fileName, ATLANTIS_ATTRIBUTE, bm->ecotest, 1, attributeGroupNode, no_checking, "li_a_invert");
	bm->li_b_invert = Util_XML_Read_Value(fileName, ATLANTIS_ATTRIBUTE, bm->ecotest, 1, attributeGroupNode, no_checking, "li_b_invert");

    /* Read in dynamic X_RS values */
    if (bm->flag_dynamicXRS) {
        Util_XML_Read_Species_Param(bm, fileName, attributeGroupNode, RSmax_id);
        Util_XML_Read_Species_Param(bm, fileName, attributeGroupNode, RSmid_id);
        Util_XML_Read_Species_Param(bm, fileName, attributeGroupNode, RSslope_id);
        Util_XML_Read_Species_Param(bm, fileName, attributeGroupNode, RSprop_id);
        Util_XML_Read_Species_Param(bm, fileName, attributeGroupNode, SNcost_id);
        Util_XML_Read_Species_Param(bm, fileName, attributeGroupNode, RNcost_id);
        Util_XML_Read_Species_Param(bm, fileName, attributeGroupNode, RSstarve_id);
    }
    
	attributeGroup = Util_XML_Get_Node(ATLANTIS_ATTRIBUTE_SUB_GROUP, attributeGroupNode, "Assimilation_Efficiency");
	if (attributeGroup == NULL)
		quit("Diet/Assimilation_Efficiency attribute group not found in input file %s.\n", fileName);

	Util_XML_Read_Species_Param(bm, fileName, attributeGroup, E1_id);
	Util_XML_Read_Species_Param(bm, fileName, attributeGroup, E2_id);
	Util_XML_Read_Species_Param(bm, fileName, attributeGroup, E3_id);
	Util_XML_Read_Species_Param(bm, fileName, attributeGroup, E4_id);
	Util_XML_Read_Species_Param(bm, fileName, attributeGroup, flagq10eff_id);
    Util_XML_Read_Species_Param(bm, fileName, attributeGroup, flagq10receff_id);

	/* In case need temperature dependent efficiencies store original values */
	for (sp = 0; sp < bm->K_num_tot_sp; sp++) {
		if (FunctGroupArray[sp].isVertebrate == TRUE) {
			FunctGroupArray[sp].speciesParams[E1orig_id] = FunctGroupArray[sp].speciesParams[E1_id];
			FunctGroupArray[sp].speciesParams[E2orig_id] = FunctGroupArray[sp].speciesParams[E2_id];
			FunctGroupArray[sp].speciesParams[E3orig_id] = FunctGroupArray[sp].speciesParams[E3_id];
			FunctGroupArray[sp].speciesParams[E4orig_id] = FunctGroupArray[sp].speciesParams[E4_id];
		}
	}

	if(bm->containsCoral){
		attributeGroup = Util_XML_Get_Node(ATLANTIS_ATTRIBUTE_SUB_GROUP, attributeGroupNode, "Rugosity_Feeding");
			if (attributeGroup == NULL)
				quit("Diet/Rugosity_Feeding attribute group not found in input file %s.\n", fileName);

		Util_XML_Read_Species_Param(bm, fileName, attributeGroup, rugFeedScalar_id);
	}

    /* Aquaculture feed */
    
    if (bm->flag_aquacult_on|| bm->flag_import_feed) {
        attributeGroup = Util_XML_Get_Node(ATLANTIS_ATTRIBUTE_SUB_GROUP, attributeGroupNode, "Extra_Feeding");
        if (attributeGroup == NULL)
            quit("Diet/Extra_Feeding attribute group not found in input file %s.\n", fileName);
        
        Util_XML_Read_Species_Param(bm, fileName, attributeGroup, extra_feed_id);
    }

}

static void Read_Q10_Information_Values(MSEBoxModel *bm, char *fileName, xmlNodePtr rootnode) {
	xmlNodePtr attributeGroupNode, attributeGroup;

	if (verbose)
		printf("Reading the Physiological Q10 and environmental influences\n");

	/** Biophysical constants ************/
	attributeGroupNode = Util_XML_Get_Node(ATLANTIS_ATTRIBUTE_SUB_GROUP, rootnode, "MetabolicQ10");
	if (attributeGroupNode == NULL)
		quit("MetabolicQ10 attribute group not found in input file %s.\n", fileName);

	/* Temperature effects */
	attributeGroup = Util_XML_Get_Node(ATLANTIS_ATTRIBUTE_SUB_GROUP, attributeGroupNode, "Temperature_Effects");
	if (attributeGroup == NULL)
		quit("Temperature_Effects attribute group not found in input file %s.\n", fileName);

	Q10 = Util_XML_Read_Value(fileName, ATLANTIS_ATTRIBUTE, bm->ecotest, 1, attributeGroup, no_checking, "Q10");
	bm->temp_const_B = Util_XML_Read_Value(fileName, ATLANTIS_ATTRIBUTE, bm->ecotest, 1, attributeGroup, no_checking, "temp_coefftB");
	bm->temp_const_C = Util_XML_Read_Value(fileName, ATLANTIS_ATTRIBUTE, bm->ecotest, 1, attributeGroup, no_checking, "temp_coefftC");
	bm->temp_const_D = Util_XML_Read_Value(fileName, ATLANTIS_ATTRIBUTE, bm->ecotest, 1, attributeGroup, no_checking, "temp_exp");
	
	Util_XML_Read_Species_Param(bm, fileName, attributeGroup, temp_coefftA_id);

	Util_XML_Read_Species_Param(bm, fileName, attributeGroup, q10_id);
	Util_XML_Read_Species_Param(bm, fileName, attributeGroup, q10_method_id);
	Util_XML_Read_Species_Param(bm, fileName, attributeGroup, q10_optimal_temp_id);
	Util_XML_Read_Species_Param(bm, fileName, attributeGroup, q10_correction_id);

	Util_XML_Read_Species_Param(bm, fileName, attributeGroup, flagtempsensitive_id);
	Util_XML_Read_Species_Param(bm, fileName, attributeGroup, flagfecundsensitive_id);

	attributeGroup = Util_XML_Get_Node(ATLANTIS_ATTRIBUTE_SUB_GROUP, attributeGroupNode, "Salinity_Effects");
	if (attributeGroup == NULL)
		quit("Salinity_Effects attribute group not found in input file %s.\n", fileName);

	Util_XML_Read_Species_Param(bm, fileName, attributeGroup, flagSaltSensitive_id);
	Util_XML_Read_Species_Param(bm, fileName, attributeGroup, salt_correction_id);

    if(bm->flagmodelpH){
        attributeGroup = Util_XML_Get_Node(ATLANTIS_ATTRIBUTE_SUB_GROUP, attributeGroupNode, "pH_Effects");
        if (attributeGroup == NULL)
            quit("pH_Effects attribute group not found in input file %s.\n", fileName);

        Util_XML_Read_Species_Param(bm, fileName, attributeGroup, flagpHsensitive_id);
        Util_XML_Read_Species_Param(bm, fileName, attributeGroup, pHsensitive_model_id);
        Util_XML_Read_Species_Param(bm, fileName, attributeGroup, flagnutvaleffect_id);
        Util_XML_Read_Species_Param(bm, fileName, attributeGroup, flagpredavaileffect_id);
        Util_XML_Read_Species_Param(bm, fileName, attributeGroup, flagcontract_tol_id);
	
        Util_XML_Read_Species_Param(bm, fileName, attributeGroup, KN_pH_id);
        Util_XML_Read_Species_Param(bm, fileName, attributeGroup, pH_constA_id);
        Util_XML_Read_Species_Param(bm, fileName, attributeGroup, pH_constB_id);
        Util_XML_Read_Species_Param(bm, fileName, attributeGroup, pH_constC_id);
        Util_XML_Read_Species_Param(bm, fileName, attributeGroup, optimal_pH_id);
        Util_XML_Read_Species_Param(bm, fileName, attributeGroup, pH_correction_id);
        Util_XML_Read_Species_Param(bm, fileName, attributeGroup, contract_tol_id);
        Util_XML_Read_Species_Param(bm, fileName, attributeGroup, min_pH_id);
        Util_XML_Read_Species_Param(bm, fileName, attributeGroup, max_pH_id);

        Util_XML_Read_Species_Param(bm, fileName, attributeGroup, pHmortstart_id);
        Util_XML_Read_Species_Param(bm, fileName, attributeGroup, pHmortA_id);
        Util_XML_Read_Species_Param(bm, fileName, attributeGroup, pHmortB_id);
        Util_XML_Read_Species_Param(bm, fileName, attributeGroup, pHmortmid_id);
    }
    
    if (bm->flag_pollutant_impacts) {
        Util_XML_Read_Species_Param(bm, fileName, attributeGroup, light_coefft_id);
        Util_XML_Read_Species_Param(bm, fileName, attributeGroup, noise_coefft_id);
    }
    
}

static void Read_Encystment(MSEBoxModel *bm, char *fileName, xmlNodePtr parent) {
	xmlNodePtr attributeGroupNode;

	if (verbose)
		printf("Reading the Encystment and overwintering values\n");

	attributeGroupNode = Util_XML_Get_Node(ATLANTIS_ATTRIBUTE_SUB_GROUP, parent, "Encystment");
	if (attributeGroupNode == NULL)
		quit("Encystment attribute group not found in input file %s.\n", fileName);

	Util_XML_Read_Species_Param(bm, fileName, attributeGroupNode, overwinterStartTofY_id);
	Util_XML_Read_Species_Param(bm, fileName, attributeGroupNode, overwinterEndTofY_id);
	Util_XML_Read_Species_Param(bm, fileName, attributeGroupNode, overwinterStartTemp_id);
	Util_XML_Read_Species_Param(bm, fileName, attributeGroupNode, overwinterEndTemp_id);
	Util_XML_Read_Species_Param(bm, fileName, attributeGroupNode, crit_mum_id);
	Util_XML_Read_Species_Param(bm, fileName, attributeGroupNode, crit_nut_id);
	Util_XML_Read_Species_Param(bm, fileName, attributeGroupNode, crit_temp_id);
	Util_XML_Read_Species_Param(bm, fileName, attributeGroupNode, encyst_rate_id);
	Util_XML_Read_Species_Param(bm, fileName, attributeGroupNode, hatch_rate_id);
	Util_XML_Read_Species_Param(bm, fileName, attributeGroupNode, encyst_period_id);
	Util_XML_Read_Species_Param(bm, fileName, attributeGroupNode, flagencyst_id);
}

static void Read_Fish_Respiration_Values(MSEBoxModel *bm, char *fileName, xmlNodePtr parent) {
	xmlNodePtr attributeGroupNode;
	int sp;

	if (verbose)
		printf("Reading the Fish_Respiration values\n");

	attributeGroupNode = Util_XML_Get_Node(ATLANTIS_ATTRIBUTE_SUB_GROUP, parent, "Fish_Respiration");
	if (attributeGroupNode == NULL)
		quit("Fish_Respiration attribute group not found in input file %s.\n", fileName);

	KST_fish = Util_XML_Read_Value(fileName, ATLANTIS_ATTRIBUTE, bm->ecotest, 1, attributeGroupNode, no_checking, "KST_fish");
	KST_shark = Util_XML_Read_Value(fileName, ATLANTIS_ATTRIBUTE, bm->ecotest, 1, attributeGroupNode, no_checking, "KST_shark");
	KST_bird = Util_XML_Read_Value(fileName, ATLANTIS_ATTRIBUTE, bm->ecotest, 1, attributeGroupNode, no_checking, "KST_bird");
	KST_mammal = Util_XML_Read_Value(fileName, ATLANTIS_ATTRIBUTE, bm->ecotest, 1, attributeGroupNode, no_checking, "KST_mammal");

	for (sp = 0; sp < bm->K_num_tot_sp; sp++) {
		if (FunctGroupArray[sp].isVertebrate == TRUE) {
			if ((FunctGroupArray[sp].groupType == FISH) || (FunctGroupArray[sp].groupType == FISH_INVERT))
				FunctGroupArray[sp].speciesParams[KST_id] = KST_fish;
			else if (FunctGroupArray[sp].groupType == SHARK)
				FunctGroupArray[sp].speciesParams[KST_id] = KST_shark;
			else if (FunctGroupArray[sp].groupType == BIRD)
				FunctGroupArray[sp].speciesParams[KST_id] = KST_bird;
			else
				FunctGroupArray[sp].speciesParams[KST_id] = KST_mammal;
		}
	}

	Ktmp_fish = Util_XML_Read_Value(fileName, ATLANTIS_ATTRIBUTE, bm->ecotest, 1, attributeGroupNode, no_checking, "Ktmp_fish");
	Ktmp_shark = Util_XML_Read_Value(fileName, ATLANTIS_ATTRIBUTE, bm->ecotest, 1, attributeGroupNode, no_checking, "Ktmp_shark");
	Ktmp_bird = Util_XML_Read_Value(fileName, ATLANTIS_ATTRIBUTE, bm->ecotest, 1, attributeGroupNode, no_checking, "Ktmp_bird");
	Ktmp_mammal = Util_XML_Read_Value(fileName, ATLANTIS_ATTRIBUTE, bm->ecotest, 1, attributeGroupNode, no_checking, "Ktmp_mammal");

	Util_XML_Read_Species_Param(bm, fileName, attributeGroupNode, KA_id);
	Util_XML_Read_Species_Param(bm, fileName, attributeGroupNode, KB_id);
}

/**
 *	\brief Read data into the 2 d Array. Assumes the guild is the first index in the array.
 */
static void Read_Implicit_Mortality_Rate_XML(MSEBoxModel *bm, char *fileName, xmlNodePtr parent) {

	double *values = 0;
	int guild, b;
	xmlNodePtr attributeNode;

	if(bm->containsBirds){
		if (verbose)
			printf("Reading ImplicitSeabirdMortalityRate values\n");

		attributeNode = Util_XML_Get_Node(ATLANTIS_ATTRIBUTE, parent, "ImplicitSeabirdMortalityRate");
		if (attributeNode == NULL)
			quit("Mortality/ImplicitSeabirdMortalityRate attribute group not found in input file %s.\n", fileName);

		/* Read in the seabird data first */
		for (guild = 0; guild < bm->K_num_tot_sp; guild++) {
			if (FunctGroupArray[guild].isVertebrate == TRUE) {

				if (Util_XML_Read_Array_Double(ATLANTIS_GROUP_ATTRIBUTE, fileName, "Mortality/ImplicitSeabirdMortalityRate", attributeNode, no_checking, FunctGroupArray[guild].groupCode, &values, 4)
						== FALSE) {
					quit("Error: Unable to find parameter 'Mortality/ImplicitSeabirdMortalityRate/%s' in input file %s\n", FunctGroupArray[guild].groupCode,
							fileName);
				}
				for (b = 0; b < 4; b++)
					bm->mS[guild][b][0] = values[b];

				free(values);
			}
		}
	}

	attributeNode = Util_XML_Get_Node(ATLANTIS_ATTRIBUTE, parent, "ImplicitFishMortalityRate");
	if (attributeNode == NULL)
		quit("Mortality/ImplicitFishMortalityRate attribute group not found in input file %s.\n", fileName);

	/* Read in the seabird data first */
	for (guild = 0; guild < bm->K_num_tot_sp; guild++) {
		if (FunctGroupArray[guild].isVertebrate == TRUE) {

			if (Util_XML_Read_Array_Double(ATLANTIS_GROUP_ATTRIBUTE, fileName, "Mortality/ImplicitFishMortalityRate",attributeNode, no_checking, FunctGroupArray[guild].groupCode, &values, 4)
					== FALSE) {
				quit("Error: Unable to find parameter 'Mortality/ImplicitFishMortalityRate/%s' in input file %s\n", FunctGroupArray[guild].groupCode, fileName);
			}

			for (b = 0; b < 4; b++)
				bm->mS[guild][b][1] = values[b];
			free(values);

		}
	}
}

static void Read_Mortality_Values(MSEBoxModel *bm, char *fileName, xmlNodePtr parent) {
	xmlNodePtr attributeGroupNode, attributeGroup;

	if (verbose)
		printf("Reading the Mortality values\n");

	attributeGroupNode = Util_XML_Get_Node(ATLANTIS_ATTRIBUTE_SUB_GROUP, parent, "Mortality");
	if (attributeGroupNode == NULL)
		quit("Mortality attribute group not found in input file %s.\n", fileName);

	Util_XML_Read_Species_Param(bm, fileName, attributeGroupNode, KLYS_id);
	Util_XML_Read_Species_Param(bm, fileName, attributeGroupNode, mS_T15_id);

    Read_Cohort_Species_Param_Values(bm, fileName, attributeGroupNode, mL_T15_id);
    Read_Cohort_Species_Param_Values(bm, fileName, attributeGroupNode, mQ_T15_id);

    if(bm->external_populations){
        Read_Cohort_Species_Param_Values(bm, fileName, attributeGroupNode, mLext_id);
        Read_Cohort_Species_Param_Values(bm, fileName, attributeGroupNode, mPext_id);
    }
    
    Read_Implicit_Mortality_Rate_XML(bm, fileName, attributeGroupNode);

	/* Starvation mortality for fish (based on condition) */
	Kthreshm = Util_XML_Read_Value(fileName, ATLANTIS_ATTRIBUTE, bm->ecotest, 1, attributeGroupNode, proportion_check, "Kthreshm");
	Util_XML_Read_Species_Param(bm, fileName, attributeGroupNode, mStarve_id);
	Util_XML_Read_Species_Param(bm, fileName, attributeGroupNode, mT_id);

	attributeGroup = Util_XML_Get_Node(ATLANTIS_ATTRIBUTE_SUB_GROUP, attributeGroupNode, "Oxygen_Mortality");
	if (attributeGroup == NULL)
		quit("Mortality/Oxygen_Mortality attribute group not found in input file %s.\n", fileName);

	Util_XML_Read_Species_Param(bm, fileName, attributeGroup, mD_id);
	Util_XML_Read_Species_Param(bm, fileName, attributeGroup, mO_id);
	Util_XML_Read_Species_Param(bm, fileName, attributeGroup, KO2_id);
	Util_XML_Read_Species_Param(bm, fileName, attributeGroup, KO2LIM_id);

	if(bm->containsBirds){
		Util_XML_Read_Species_Param(bm, fileName, attributeGroupNode, FSBDR_id);
	}
	/* Demersal fish sloppy feeding */
	FFDDR = Util_XML_Read_Value(fileName, ATLANTIS_ATTRIBUTE, bm->ecotest, 1, attributeGroupNode, no_checking, "FFDDR");

}
static void Read_Detritus_Conversion_Values(MSEBoxModel *bm, char *fileName, xmlNodePtr parent) {
	xmlNodePtr attributeGroupNode;
	int sp;

	if (verbose)
		printf("Reading the Detritus_Conversion values\n");

	attributeGroupNode = Util_XML_Get_Node(ATLANTIS_ATTRIBUTE_SUB_GROUP, parent, "Detritus_Conversion");
	if (attributeGroupNode == NULL)
		quit("Detritus_Conversion attribute group not found in input file %s.\n", fileName);

	Util_XML_Read_Species_Param(bm, fileName, attributeGroupNode, FDG_id);
	Util_XML_Read_Species_Param(bm, fileName, attributeGroupNode, FDGDL_id);
	Util_XML_Read_Species_Param(bm, fileName, attributeGroupNode, FDGDR_id);
	Util_XML_Read_Species_Param(bm, fileName, attributeGroupNode, FDMort_id);

	/* Split up of losses to detritus between labile and refractory detritus */
	FDL_fish = Util_XML_Read_Value(fileName, ATLANTIS_ATTRIBUTE, bm->ecotest, 1, attributeGroupNode, proportion_check, "FDL_fish");
	FDL_benth = Util_XML_Read_Value(fileName, ATLANTIS_ATTRIBUTE, bm->ecotest, 1, attributeGroupNode, proportion_check, "FDL_benth");
	FDL_top = Util_XML_Read_Value(fileName, ATLANTIS_ATTRIBUTE, bm->ecotest, 1, attributeGroupNode, proportion_check, "FDL_top");
	FDL_wc = Util_XML_Read_Value(fileName, ATLANTIS_ATTRIBUTE, bm->ecotest, 1, attributeGroupNode, proportion_check, "FDL_wc");

	if(bm->flag_macro_model){
		FDL_SG_roots = Util_XML_Read_Value(fileName, ATLANTIS_ATTRIBUTE, bm->ecotest, 1, attributeGroupNode, proportion_check, "FDL_SG_roots");
		FDL_SG_leaves =  Util_XML_Read_Value(fileName, ATLANTIS_ATTRIBUTE, bm->ecotest, 1, attributeGroupNode, proportion_check, "FDL_SG_leaves");
	}else{
		FDL_SG_roots = 1.0;
		FDL_SG_leaves = 1.0;
	}

	for (sp = 0; sp < bm->K_num_tot_sp; sp++) {
		if (FunctGroupArray[sp].isVertebrate == TRUE) {
			if (FunctGroupArray[sp].groupType == FISH_INVERT)
                FunctGroupArray[sp].speciesParams[FDL_id] = FDL_benth;
            else if ((FunctGroupArray[sp].groupType == FISH) || (FunctGroupArray[sp].groupType == SHARK))
				FunctGroupArray[sp].speciesParams[FDL_id] = FDL_fish;
			else
				FunctGroupArray[sp].speciesParams[FDL_id] = FDL_top;
		}else{
			/* Now set up the invert FDL_id values */
			if(FunctGroupArray[sp].groupAgeType == AGE_STRUCTURED_BIOMASS){
				FunctGroupArray[sp].speciesParams[FDL_id] = FDL_benth;
			}else{
				if(FunctGroupArray[sp].groupType == CEP || FunctGroupArray[sp].groupType == PWN
						|| FunctGroupArray[sp].habitatType == SED || FunctGroupArray[sp].habitatType == EPIFAUNA){
					FunctGroupArray[sp].speciesParams[FDL_id] = FDL_benth;
				}else{
					FunctGroupArray[sp].speciesParams[FDL_id] = FDL_wc;
				}
			}
		}
	}




	/* Fraction of bacterial losses sent to detritus */
	FPB_DR = Util_XML_Read_Value(fileName, ATLANTIS_ATTRIBUTE, bm->ecotest, 1, attributeGroupNode, proportion_check, "FPB_DR");
	FBB_DR = Util_XML_Read_Value(fileName, ATLANTIS_ATTRIBUTE, bm->ecotest, 1, attributeGroupNode, proportion_check, "FBB_DR");

	/* Fraction of bacterial losses sent to DON */
	FPB_DON = Util_XML_Read_Value(fileName, ATLANTIS_ATTRIBUTE, bm->ecotest, 1, attributeGroupNode, proportion_check, "FPB_DON");
	FBB_DON = Util_XML_Read_Value(fileName, ATLANTIS_ATTRIBUTE, bm->ecotest, 1, attributeGroupNode, proportion_check, "FBB_DON");

	/* Fraction of infaunal NH involved in denitrification */
	Fben_den = Util_XML_Read_Value(fileName, ATLANTIS_ATTRIBUTE, bm->ecotest, 1, attributeGroupNode, proportion_check, "Fben_den");

	Util_XML_Read_Species_Param(bm, fileName, attributeGroupNode, sp_remin_contrib_id);
}

/**
 *	\brief Read the data into the recruit_vdistrib array from the input XML file.
 */
static void Read_KDENR_XML(MSEBoxModel *bm, char *fileName, xmlNodePtr parent) {

	double *values = 0;
	int guild, b;
	int counter;
	double sum;

	xmlNodePtr attributeNode;

	if (verbose)
		printf("Reading KDENR values\n");

	attributeNode = Util_XML_Get_Node(ATLANTIS_ATTRIBUTE, parent, "KDENR");
	if (attributeNode == NULL)
		quit("Reproduction/KDENR attribute group not found in input file %s.\n", fileName);

	/* Create a node for each functional group*/
	for (guild = 0; guild < bm->K_num_tot_sp; guild++) {
		switch (FunctGroupArray[guild].groupAgeType) {
		case AGE_STRUCTURED:
			counter = FunctGroupArray[guild].numStocks;

			if (Util_XML_Read_Array_Double(ATLANTIS_GROUP_ATTRIBUTE, fileName, "Reproduction/KDENR", attributeNode, no_checking, FunctGroupArray[guild].groupCode, &values, counter) == FALSE) {
				quit("Error: Unable to find parameter 'Reproduction/KDENR/%s' in input file %s\n", FunctGroupArray[guild].groupCode, fileName);
			}

			sum = 0.0;

			for (b = 0; b < counter; b++) {
				KDENR[guild][b] = values[b];
				sum = sum + values[b];
			}
			FunctGroupArray[guild].speciesParams[KDENR_id] = sum;

			free(values);
			break;
		case AGE_STRUCTURED_BIOMASS:
			FunctGroupArray[guild].speciesParams[KDENR_id] = Util_XML_Read_Value(fileName, ATLANTIS_GROUP_ATTRIBUTE, bm->ecotest, 1, attributeNode, no_checking,
					FunctGroupArray[guild].groupCode);
			for (b = 0; b < FunctGroupArray[guild].numStocks; b++) {
				KDENR[guild][b] = FunctGroupArray[guild].speciesParams[KDENR_id];
			}
			break;
		case BIOMASS:
            /* Do nothing */
			break;

		}
	}
}

/**
 *	\brief Read the data into the recruit_vdistrib array from the input XML file.
 */
static void Read_RecSTOCK_XML(MSEBoxModel *bm, char *fileName, xmlNodePtr parent) {

	double *values = 0;
	int guild, b, counter;
	xmlNodePtr attributeNode;

	if (verbose)
		printf("Reading recSTOCK values\n");

	attributeNode = Util_XML_Get_Node(ATLANTIS_ATTRIBUTE, parent, "recSTOCK");
	if (attributeNode == NULL)
		quit("recSTOCK attribute group not found in input file %s.\n", fileName);

	/* Create a node for each functional group*/
	for (guild = 0; guild < bm->K_num_tot_sp; guild++) {
		if (FunctGroupArray[guild].isVertebrate == TRUE) {
			counter = FunctGroupArray[guild].numStocks;

			if (Util_XML_Read_Array_Double(ATLANTIS_GROUP_ATTRIBUTE, fileName, "Reproduction/recSTOCK", attributeNode, no_checking, FunctGroupArray[guild].groupCode, &values, counter)
					== FALSE) {
				quit("Error: Unable to find parameter 'Reproduction/recSTOCK/%s' in input file %s\n", FunctGroupArray[guild].groupCode, fileName);
			}

			for (b = 0; b < counter; b++) {
				recSTOCK[guild][b] = values[b];
			}
			free(values);
		}
	}
}

/**
 *	\brief Read the data into the recruit_vdistrib array from the input XML file.
 */
static void Read_Popratio_Stock_XML(MSEBoxModel *bm, char *fileName, xmlNodePtr parent) {

	double *values = 0;
	int guild, b, cohort, i, counter, ngene, nc;
	xmlNodePtr attributeNode, speciesNode, cohortNode;
	char str[100];
	char errorString[STRLEN];

	if (verbose)
		printf("Reading popratioStock values\n");

	attributeNode = Util_XML_Get_Node(ATLANTIS_ATTRIBUTE, parent, "popratioStock");
	if (attributeNode == NULL)
		quit("popratioStock attribute group not found in input file %s.\n", fileName);

	/* Create a node for each functional group*/
	for (guild = 0; guild < bm->K_num_tot_sp; guild++) {
		if ((FunctGroupArray[guild].groupAgeType == AGE_STRUCTURED_BIOMASS) || (FunctGroupArray[guild].groupAgeType == AGE_STRUCTURED)) {

			speciesNode = Util_XML_Get_Node(ATLANTIS_GROUP_ATTRIBUTE, attributeNode, FunctGroupArray[guild].groupCode);
			if (speciesNode == NULL)
				quit("Reproduction/popratioStock/%s species attribute group not found.\n", FunctGroupArray[guild].groupCode);

			for (cohort = 0; cohort < FunctGroupArray[guild].numCohorts; cohort++) {

				sprintf(str, "cohort%d", cohort + 1);
				sprintf(errorString, "Reproduction/popratioStock/%s/%s", FunctGroupArray[guild].groupCode, str);

				cohortNode = Util_XML_Get_Node(ATLANTIS_COHORT_ATTRIBUTE, speciesNode, str);
				if (cohortNode == NULL)
					quit("%s cohort attribute group not found.\n", errorString);

				for (i = 0; i < FunctGroupArray[guild].numStocks; i++) {
					counter = FunctGroupArray[guild].ageClassSize;
                    if (FunctGroupArray[guild].ageClassSize < 1) {
                        quit("Currently the popratioStock code will not work for ageclasses < 1 - you have ageClassSize for %s if %e\n", FunctGroupArray[guild].groupCode, FunctGroupArray[guild].ageClassSize);
                    }

					sprintf(str, "stock%d", i + 1);

					if (Util_XML_Read_Array_Double(ATLANTIS_ATTRIBUTE, fileName, errorString, cohortNode, proportion_check, str, &values, counter) == FALSE) {
						quit("Error: Unable to find parameter '%s/%s' in input file %s\n", errorString, str, fileName);
					}

					for(ngene = 0; ngene < FunctGroupArray[guild].numGeneTypes; ngene++){
						nc = (FunctGroupArray[guild].numGeneTypes * cohort) + ngene;
						for (b = 0; b < counter; b++) {
							readinpopratio[i][guild][nc][b] = values[b];
						}
					}
					free(values);
				}
			}
		}
	}
}

/**
 *	\brief Read the data into the recruit_vdistrib array from the input XML file.
 */
static void Read_Vertical_Recruit_Location_XML(MSEBoxModel *bm, char *fileName, xmlNodePtr parent) {

	double *values = 0;
	int guild, b;
	xmlNodePtr attributeNode;

	if (verbose)
		printf("Reading VerticalRecruitLocation values\n");

	attributeNode = Util_XML_Get_Node(ATLANTIS_ATTRIBUTE, parent, "VerticalRecruitLocation");
	if (attributeNode == NULL)
		quit("Reproduction/VerticalRecruitLocation attribute group not found in input file %s.\n", fileName);

	/* Create a node for each functional group*/
	for (guild = 0; guild < bm->K_num_tot_sp; guild++) {
		if (FunctGroupArray[guild].isVertebrate == TRUE) {

			if (Util_XML_Read_Array_Double(ATLANTIS_GROUP_ATTRIBUTE, fileName, "Reproduction/VerticalRecruitLocation", attributeNode, no_checking, FunctGroupArray[guild].groupCode, &values, bm->wcnz)
					== FALSE) {
				quit("Error: Unable to find parameter 'Reproduction/VerticalRecruitLocation/%s' in input file %s\n", FunctGroupArray[guild].groupCode, fileName);
			}

			for (b = 0; b < bm->wcnz; b++)
				recruit_vdistrib[b][guild] = values[b];
			free(values);
		}
	}
}

static void Read_Reproduction_Values(MSEBoxModel *bm, char *fileName, xmlNodePtr parent) {
	xmlNodePtr attributeGroupNode;

	if (verbose)
		printf("Reading the Reproduction values\n");

	attributeGroupNode = Util_XML_Get_Node(ATLANTIS_ATTRIBUTE_SUB_GROUP, parent, "Reproduction");
	if (attributeGroupNode == NULL)
		quit("Reproduction attribute group not found in input file %s.\n", fileName);

	/* Spawning related */
	flagtrecruitdistrib = (int) Util_XML_Read_Value(fileName, ATLANTIS_ATTRIBUTE, bm->ecotest, 1, attributeGroupNode, binary_check, "flagtrecruitdistrib");

    
	Util_XML_Read_Species_Param(bm, fileName, attributeGroupNode, PP_id);
	Util_XML_Read_Species_Param(bm, fileName, attributeGroupNode, Ralpha_id);
	Util_XML_Read_Species_Param(bm, fileName, attributeGroupNode, Rbeta_id);
	Util_XML_Read_Species_Param(bm, fileName, attributeGroupNode, BHalpha_id);
	Util_XML_Read_Species_Param(bm, fileName, attributeGroupNode, BHbeta_id);

	Util_XML_Read_Species_Param(bm, fileName, attributeGroupNode, jack_a_id);
	Util_XML_Read_Species_Param(bm, fileName, attributeGroupNode, jack_b_id);

    Util_XML_Read_Species_Param(bm, fileName, attributeGroupNode, intersp_depend_recruit_id);
 	Util_XML_Read_Species_Param(bm, fileName, attributeGroupNode, intersp_depend_sp_id);
	Util_XML_Read_Species_Param(bm, fileName, attributeGroupNode, intersp_depend_scale_id);
    
    /* Aquaculture fry */
	if (bm->flag_aquacult_on) {
		Util_XML_Read_Species_Param(bm, fileName, attributeGroupNode, aquacult_fry_id);
	}

	/* Pulse dynamics for recruitment - e.g. recovery pulses */
	recover_trigger = Util_XML_Read_Value(fileName, ATLANTIS_ATTRIBUTE, bm->ecotest, 1, attributeGroupNode, no_checking, "recover_trigger");
	recover_span = Util_XML_Read_Value(fileName, ATLANTIS_ATTRIBUTE, bm->ecotest, 1, attributeGroupNode, no_checking, "recover_span");
	recover_subseq = Util_XML_Read_Value(fileName, ATLANTIS_ATTRIBUTE, bm->ecotest, 1, attributeGroupNode, no_checking, "recover_subseq");

	Util_XML_Read_Species_Param(bm, fileName, attributeGroupNode, recover_start_id);

	/* Log-normal random recruitment */
	lognorm_mu = Util_XML_Read_Value(fileName, ATLANTIS_ATTRIBUTE, bm->ecotest, 1, attributeGroupNode, no_checking, "lognorm_mu");
	lognorm_sigma = Util_XML_Read_Value(fileName, ATLANTIS_ATTRIBUTE, bm->ecotest, 1, attributeGroupNode, no_checking, "lognorm_sigma");

	Util_XML_Read_Species_Param(bm, fileName, attributeGroupNode, log_mult_id);
    Util_XML_Read_Species_Param(bm, fileName, attributeGroupNode, norm_sigma_id);
    Util_XML_Read_Species_Param(bm, fileName, attributeGroupNode, flag_recruit_stochastic_id);
    
    if(bm->ice_on) {
        Util_XML_Read_Species_Param(bm, fileName, attributeGroupNode, prod_alpha_id);
        Util_XML_Read_Species_Param(bm, fileName, attributeGroupNode, den_depend_beta1_id);
        Util_XML_Read_Species_Param(bm, fileName, attributeGroupNode, den_depend_beta2_id);
        Util_XML_Read_Species_Param(bm, fileName, attributeGroupNode, temp_coefft_id);
        Util_XML_Read_Species_Param(bm, fileName, attributeGroupNode, rate_coefft_id);
        Util_XML_Read_Species_Param(bm, fileName, attributeGroupNode, wind_coefft_id);
        Util_XML_Read_Species_Param(bm, fileName, attributeGroupNode, recruit_var_id);
    }
        
	Util_XML_Read_Species_Param(bm, fileName, attributeGroupNode, KWSR_id);
	Util_XML_Read_Species_Param(bm, fileName, attributeGroupNode, KWRR_id);

    if (bm->flagtempdepend_reprod) {
        Util_XML_Read_Species_Param(bm, fileName, attributeGroupNode, min_spawn_temp_id);
        Util_XML_Read_Species_Param(bm, fileName, attributeGroupNode, max_spawn_temp_id);
    }

	if (bm->flagsaltdepend) {
		Util_XML_Read_Species_Param(bm, fileName, attributeGroupNode, min_spawn_salt_id);
		Util_XML_Read_Species_Param(bm, fileName, attributeGroupNode, max_spawn_salt_id);
	}

	if(bm->containsMCPYr){
		//Read_Cohort_Species_Param_Values(bm, fileName, attributeGroupNode, Time_Age_id); //No longer read in separately per age cohort
		Read_Spawn_Species_Param_Values(bm, fileName, attributeGroupNode, Time_Age_id); // Read in as a vector instead
	}

	Util_XML_Read_Species_Param(bm, fileName, attributeGroupNode, age_mat_id);

	Read_Vertical_Recruit_Location_XML(bm, fileName, attributeGroupNode);

//}
//
//
//
//void Read_Reproduction_Values(MSEBoxModel *bm, char *fileName, xmlNodePtr parent) {
//	xmlNodePtr attributeGroupNode;
//
//	if (verbose)
//		printf("Reading the Reproduction values\n");
//
//	attributeGroupNode = Util_XML_Get_Node(ATLANTIS_ATTRIBUTE_SUB_GROUP, parent, "Reproduction");
//	if (attributeGroupNode == NULL)
//		quit("Reproduction attribute group not found in input file %s.\n", fileName);

	Util_XML_Read_Species_Param(bm, fileName, attributeGroupNode, flagmother_id);
	Util_XML_Read_Species_Param(bm, fileName, attributeGroupNode, flagrecruit_id);
	Util_XML_Read_Species_Param(bm, fileName, attributeGroupNode, flagrecpeak_id);
	//Util_XML_Read_Species_Param(bm, fileName, attributeGroupNode, flaglocalrecruit_id);
	Util_XML_Read_Species_Param(bm, fileName, attributeGroupNode, flagstocking_id);
    
    Util_XML_Read_Species_Param(bm, fileName, attributeGroupNode, flagkeep_plusgroup_id);

	Util_XML_Read_Species_Param(bm, fileName, attributeGroupNode, flagbearlive_id);
	Util_XML_Read_Species_Param(bm, fileName, attributeGroupNode, feed_while_spawn_id);

	Read_Spawn_Species_Param_Values(bm, fileName, attributeGroupNode, Time_Spawn_id);
	Util_XML_Read_Species_Param(bm, fileName, attributeGroupNode, spawn_period_id);
	Util_XML_Read_Species_Param(bm, fileName, attributeGroupNode, Recruit_Time_id);
	Util_XML_Read_Species_Param(bm, fileName, attributeGroupNode, Recruit_Period_id);
    Util_XML_Read_Species_Param(bm, fileName, attributeGroupNode, cohort_recruit_entry_id);
	Util_XML_Read_Species_Param(bm, fileName, attributeGroupNode, KSPA_id);
	Util_XML_Read_Species_Param(bm, fileName, attributeGroupNode, FSP_id);

	Read_Vert_FSPBXML(bm, fileName, attributeGroupNode);
	Read_Vert_SpeciesXML(bm, fileName, attributeGroupNode, "RecruitDistribution", bm->recruit_hdistrib[0], bm->nbox, proportion_check);
    
    if(bm->flag_aquacult_on){
        Read_Cultured_SpeciesXML(bm, fileName, attributeGroupNode, "AquacultDistribution", bm->recruit_hdistrib[0], bm->nbox, proportion_check);
    }

	/* Distribution of recruits within recruitment period */
	rec_m = Util_XML_Read_Value(fileName, ATLANTIS_ATTRIBUTE, bm->ecotest, 1, attributeGroupNode, no_checking, "rec_m");
	rec_sigma = Util_XML_Read_Value(fileName, ATLANTIS_ATTRIBUTE, bm->ecotest, 1, attributeGroupNode, no_checking, "rec_sigma");

	bm->recruitRange = Util_XML_Read_Value(fileName, ATLANTIS_ATTRIBUTE, bm->ecotest, 1, attributeGroupNode, no_checking, "recruitRange");
	bm->recruitRangeFlat = Util_XML_Read_Value(fileName, ATLANTIS_ATTRIBUTE, bm->ecotest, 1, attributeGroupNode, no_checking, "recruitRangeFlat");

	bm->ref_chl = Util_XML_Read_Value(fileName, ATLANTIS_ATTRIBUTE, bm->ecotest, 1, attributeGroupNode, no_checking, "ref_chl");

}

void Read_Stock_Structure_Values(MSEBoxModel *bm, char *fileName, xmlNodePtr parent) {
	xmlNodePtr attributeGroupNode;

	if (verbose)
		printf("Reading the Stock_Structure values\n");

	attributeGroupNode = Util_XML_Get_Node(ATLANTIS_ATTRIBUTE_SUB_GROUP, parent, "Stock_Structure");
	if (attributeGroupNode == NULL)
		quit("Stock_Structure attribute group not found in input file %s.\n", fileName);

	Read_Vert_Species_IntegerXML(bm, fileName, attributeGroupNode, "StockStructure", bm->stock_struct, bm->nbox, integer_check);
	Read_Vert_Species_IntegerXML(bm, fileName, attributeGroupNode, "VerticalStockStructure", bm->v_stock_struct, bm->wcnz, integer_check);

	if (Util_XML_Read_Array_Double(ATLANTIS_ATTRIBUTE, fileName, "Stock_Structure", attributeGroupNode, integer_check, regids, &regIDi, bm->nbox) == FALSE) {
		quit("Error: Unable to find parameter 'Stock_Structure/%s' in input file %s\n", regids, fileName);
	}
}

void Read_Invading_Species_Values(MSEBoxModel *bm, char *fileName, xmlNodePtr parent) {
	xmlNodePtr attributeGroupNode;
	double numsechr = 3600.0; /* number of seconds in a hour */
    double *values;
    int n, cohort;

	if (verbose)
		printf("Reading the Invading_Species values\n");

	attributeGroupNode = Util_XML_Get_Node(ATLANTIS_ATTRIBUTE_SUB_GROUP, parent, "Invading_Species");
	if (attributeGroupNode == NULL)
		quit("Invading_Species attribute group not found in input file %s.\n", fileName);

	/** Invading species related ************/
	bm->invading_sp_model = (int) Util_XML_Read_Value(fileName, ATLANTIS_ATTRIBUTE, bm->ecotest, 1, attributeGroupNode, integer_check, "invading_sp_model");
	bm->InvaderIndex = (int) Util_XML_Read_Value(fileName, ATLANTIS_ATTRIBUTE, bm->ecotest, 1, attributeGroupNode, integer_check, "InvaderIndex");
	bm->minInvaderAge = (int) Util_XML_Read_Value(fileName, ATLANTIS_ATTRIBUTE, bm->ecotest, 1, attributeGroupNode, integer_check, "minInvaderAge");
	bm->maxInvaderAge = (int) Util_XML_Read_Value(fileName, ATLANTIS_ATTRIBUTE, bm->ecotest, 1, attributeGroupNode, integer_check, "maxInvaderAge");
	bm->InvaderEntryBox = (int) Util_XML_Read_Value(fileName, ATLANTIS_ATTRIBUTE, bm->ecotest, 1, attributeGroupNode, integer_check, "InvaderEntryBox");
    bm->InvadersEntering = (int) Util_XML_Read_Value(fileName, ATLANTIS_ATTRIBUTE, bm->ecotest, 1, attributeGroupNode, integer_check, "InvadersEntering");
    
	bm->InvaderMinDepth = Util_XML_Read_Value(fileName, ATLANTIS_ATTRIBUTE, bm->ecotest, 1, attributeGroupNode, no_checking, "InvaderMinDepth");
	bm->InvaderMaxDepth = Util_XML_Read_Value(fileName, ATLANTIS_ATTRIBUTE, bm->ecotest, 1, attributeGroupNode, no_checking, "InvaderMaxDepth");
	bm->InvaderStartDay = Util_XML_Read_Value(fileName, ATLANTIS_ATTRIBUTE, bm->ecotest, 1, attributeGroupNode, no_checking, "InvaderStartDay");
	bm->InvaderEndDay = Util_XML_Read_Value(fileName, ATLANTIS_ATTRIBUTE, bm->ecotest, 1, attributeGroupNode, no_checking, "InvaderEndDay");
	bm->InvaderScalar = Util_XML_Read_Value(fileName, ATLANTIS_ATTRIBUTE, bm->ecotest, 1, attributeGroupNode, no_checking, "InvaderScalar");
	bm->InvaderSpeed = Util_XML_Read_Value(fileName, ATLANTIS_ATTRIBUTE, bm->ecotest, 1, attributeGroupNode, no_checking, "InvaderSpeed") / numsechr;
    
    if (bm->InvadersEntering ) {
        bm->InvaderEntryLayer = (int) Util_XML_Read_Value(fileName, ATLANTIS_ATTRIBUTE, bm->ecotest, 1, attributeGroupNode, integer_check, "InvaderEntryLayer");
        
        n = bm->maxInvaderAge - bm->minInvaderAge;
        if (n <= 0)
            n = 1;
        
        /* Load number of invaders to add to the system - in first invasion pulse */
        if (Util_XML_Read_Array_Double(ATLANTIS_ATTRIBUTE, fileName, "InvaderStartNums", attributeGroupNode, integer_check, "InvaderStartNums", &values, n) == FALSE) {
            quit("Error: Unable to find parameter 'Invading_Species/InvaderStartNums' in input file %s\n", "InvaderStartNums", fileName);
        }
    
        for (cohort = 0; cohort < n; cohort++) {
            bm->InvaderStartNums[cohort] = values[cohort];
        }
        free(values);
    }
}


static void Read_Ice_Values(MSEBoxModel *bm, char *fileName, xmlNodePtr parent) {
	xmlNodePtr attributeGroupNode;

	if (verbose)
		printf("Reading the Reproduction values\n");

	attributeGroupNode = Util_XML_Get_Node(ATLANTIS_ATTRIBUTE_SUB_GROUP, parent, "Ice_Related");
	if (attributeGroupNode == NULL)
		quit("Ice_Related attribute group not found in input file %s.\n", fileName);

	Util_XML_Read_Species_Param(bm, fileName, attributeGroupNode, ICE_KDEP_id);

	p_IBice = Util_XML_Read_Value(fileName, ATLANTIS_ATTRIBUTE, bm->ecotest, 1, attributeGroupNode, proportion_check, "p_IBice");



}

/**
 * Read in contaiminant values.
 * 
 * 
 * 
 */

static void Read_Contaminant_Values(MSEBoxModel *bm, char *fileName, xmlNodePtr parent) {
	int cIndex, sp, contamIndex;
	char varStr[STRLEN*2];
	xmlNodePtr attributeGroupNode;

	if (verbose)
		printf("Reading the Contaminant values\n");

	attributeGroupNode = Util_XML_Get_Node(ATLANTIS_ATTRIBUTE_SUB_GROUP, parent, "Contaminant_Related");
	if (attributeGroupNode == NULL)
		quit("Contaminant_Related attribute group not found in input file %s.\n", fileName);

    bm->flag_dissolved_pollutants = (int) Util_XML_Read_Value(fileName, ATLANTIS_ATTRIBUTE, bm->ecotest, 1, attributeGroupNode, integer_check, "flag_dissolved_pollutants");
    bm->flag_contamMortModel = (int) Util_XML_Read_Value(fileName, ATLANTIS_ATTRIBUTE, bm->ecotest, 1, attributeGroupNode, integer_check, "flag_contamMortModel");
    bm->flag_contamInteractModel = (int) Util_XML_Read_Value(fileName, ATLANTIS_ATTRIBUTE, bm->ecotest, 1, attributeGroupNode, integer_check, "flag_contamInteractModel");
    bm->flag_contamGrowthModel = (int) Util_XML_Read_Value(fileName, ATLANTIS_ATTRIBUTE, bm->ecotest, 1, attributeGroupNode, integer_check, "flag_contamGrowthModel");
    bm->flag_contamReprodModel = (int) Util_XML_Read_Value(fileName, ATLANTIS_ATTRIBUTE, bm->ecotest, 1, attributeGroupNode, integer_check, "flag_contamReprodModel");
    bm->flag_contamOnlyAmplify = (int) Util_XML_Read_Value(fileName, ATLANTIS_ATTRIBUTE, bm->ecotest, 1, attributeGroupNode, integer_check, "flag_contamOnlyAmplify");
    bm->flag_contamMove = (int) Util_XML_Read_Value(fileName, ATLANTIS_ATTRIBUTE, bm->ecotest, 1, attributeGroupNode, integer_check, "flag_contamMove");
    bm->flag_contamMinTemp = (int) Util_XML_Read_Value(fileName, ATLANTIS_ATTRIBUTE, bm->ecotest, 1, attributeGroupNode, no_checking, "flag_contamMinTemp");
    bm->flag_contam_halflife_spbased = (int) Util_XML_Read_Value(fileName, ATLANTIS_ATTRIBUTE, bm->ecotest, 1, attributeGroupNode, integer_check, "flag_contam_halflife_spbased");
    bm->flag_contamMaternalTransfer = (int) Util_XML_Read_Value(fileName, ATLANTIS_ATTRIBUTE, bm->ecotest, 1, attributeGroupNode, integer_check, "flag_contamMaternalTransfer");
    
    bm->biopools_dodge_contam = (int) Util_XML_Read_Value(fileName, ATLANTIS_ATTRIBUTE, bm->ecotest, 1, attributeGroupNode, integer_check, "biopools_dodge_contam");
    
    bm->contam_tau = Util_XML_Read_Value(fileName, ATLANTIS_ATTRIBUTE, bm->ecotest, 1, attributeGroupNode, no_checking, "contam_tau");
    
    bm->contam_sig_uptake_const = Util_XML_Read_Value(fileName, ATLANTIS_ATTRIBUTE, bm->ecotest, 1, attributeGroupNode, no_checking, "contam_sig_uptake_const");
    bm->flag_detritus_contam = Util_XML_Read_Value(fileName, ATLANTIS_ATTRIBUTE, bm->ecotest, 1, attributeGroupNode, integer_check, "flag_detritus_contam");
    
    //fprintf(bm->logFile, "contam_sig_uptake_const starts: %e\n", bm->contam_sig_uptake_const);
    
	for(cIndex = 0; cIndex < bm->num_contaminants; cIndex++){
        double *values = 0;
        
		for(sp = 0; sp < bm->K_num_tot_sp; sp++){
			//if(FunctGroupArray[sp].speciesParams[flag_id] == TRUE && FunctGroupArray[sp].isDetritus == FALSE){
			if((FunctGroupArray[sp].speciesParams[flag_id] == TRUE) && (FunctGroupArray[sp].isDetritus == FALSE)){

                if (bm->flag_contam_halflife_spbased) {
                    sprintf(varStr, "%s_%s_decay_half_life", FunctGroupArray[sp].groupCode, bm->contaminantStructure[cIndex]->contaminant_name);
                    bm->contaminantStructure[cIndex]->sp_decay_half_life[sp] = Util_XML_Read_Value(fileName, ATLANTIS_ATTRIBUTE, bm->ecotest, 1, attributeGroupNode, no_checking, varStr);
                }
                
				sprintf(varStr, "%s_%s_uptake_rate", FunctGroupArray[sp].groupCode, bm->contaminantStructure[cIndex]->contaminant_name);
				bm->contaminantStructure[cIndex]->sp_uptake_rate[sp] = Util_XML_Read_Value(fileName, ATLANTIS_ATTRIBUTE, bm->ecotest, 1, attributeGroupNode, no_checking, varStr);
                bm->contaminantStructure[cIndex]->sp_uptake_rate[sp] /= 86400.0; // So now per second rate

				sprintf(varStr, "%s_%s_uptake_option", FunctGroupArray[sp].groupCode, bm->contaminantStructure[cIndex]->contaminant_name);
				bm->contaminantStructure[cIndex]->sp_uptake_option[sp] = (int)Util_XML_Read_Value(fileName, ATLANTIS_ATTRIBUTE, bm->ecotest, 1, attributeGroupNode, integer_check, varStr);

				sprintf(varStr, "%s_%s_LD50", FunctGroupArray[sp].groupCode, bm->contaminantStructure[cIndex]->contaminant_name);
				bm->contaminantStructure[cIndex]->sp_LD50[sp] = Util_XML_Read_Value(fileName, ATLANTIS_ATTRIBUTE, bm->ecotest, 1, attributeGroupNode, no_checking, varStr);

				sprintf(varStr, "%s_%s_LD100", FunctGroupArray[sp].groupCode, bm->contaminantStructure[cIndex]->contaminant_name);
				bm->contaminantStructure[cIndex]->sp_LD100[sp] = Util_XML_Read_Value(fileName, ATLANTIS_ATTRIBUTE, bm->ecotest, 1, attributeGroupNode, no_checking, varStr);

				bm->contaminantStructure[cIndex]->sp_maxConcentration[sp] = bm->contaminantStructure[cIndex]->sp_LD100[sp];
                bm->contaminantStructure[cIndex]->sp_maxLethalConc[sp] = bm->contaminantStructure[cIndex]->sp_LD100[sp];

                sprintf(varStr, "%s_%s_LDChronic", FunctGroupArray[sp].groupCode, bm->contaminantStructure[cIndex]->contaminant_name);
                
                bm->contaminantStructure[cIndex]->sp_LDChronic[sp] = Util_XML_Read_Value(fileName, ATLANTIS_ATTRIBUTE, bm->ecotest, 1, attributeGroupNode, no_checking, varStr);

                bm->contaminantStructure[cIndex]->sp_maxChronicConc[sp] = bm->contaminantStructure[cIndex]->sp_LDChronic[sp];
                
                sprintf(varStr, "%s_%s_LDslope", FunctGroupArray[sp].groupCode, bm->contaminantStructure[cIndex]->contaminant_name);
                bm->contaminantStructure[cIndex]->sp_LDslope[sp] = Util_XML_Read_Value(fileName, ATLANTIS_ATTRIBUTE, bm->ecotest, 1, attributeGroupNode, no_checking, varStr);
                
                sprintf(varStr, "%s_%s_EC50", FunctGroupArray[sp].groupCode, bm->contaminantStructure[cIndex]->contaminant_name);
                bm->contaminantStructure[cIndex]->sp_EC50[sp] = Util_XML_Read_Value(fileName, ATLANTIS_ATTRIBUTE, bm->ecotest, 1, attributeGroupNode, no_checking, varStr);
                
                sprintf(varStr, "%s_%s_ECslope", FunctGroupArray[sp].groupCode, bm->contaminantStructure[cIndex]->contaminant_name);
                bm->contaminantStructure[cIndex]->sp_ECslope[sp] = Util_XML_Read_Value(fileName, ATLANTIS_ATTRIBUTE, bm->ecotest, 1, attributeGroupNode, no_checking, varStr);
                
                sprintf(varStr, "%s_%s_EC50_r", FunctGroupArray[sp].groupCode, bm->contaminantStructure[cIndex]->contaminant_name);
                bm->contaminantStructure[cIndex]->sp_EC50_r[sp] = Util_XML_Read_Value(fileName, ATLANTIS_ATTRIBUTE, bm->ecotest, 1, attributeGroupNode, no_checking, varStr);
                
                sprintf(varStr, "%s_%s_ECslope_r", FunctGroupArray[sp].groupCode, bm->contaminantStructure[cIndex]->contaminant_name);
                bm->contaminantStructure[cIndex]->sp_ECslope_r[sp] = Util_XML_Read_Value(fileName, ATLANTIS_ATTRIBUTE, bm->ecotest, 1, attributeGroupNode, no_checking, varStr);
                
                sprintf(varStr, "%s_%s_TimeToLD50", FunctGroupArray[sp].groupCode, bm->contaminantStructure[cIndex]->contaminant_name);
                bm->contaminantStructure[cIndex]->sp_TimeToLD50[sp] = Util_XML_Read_Value(fileName, ATLANTIS_ATTRIBUTE, bm->ecotest, 1, attributeGroupNode, no_checking, varStr);

				sprintf(varStr, "%s_%s_Cx", FunctGroupArray[sp].groupCode, bm->contaminantStructure[cIndex]->contaminant_name);
				bm->contaminantStructure[cIndex]->sp_Cx[sp] = Util_XML_Read_Value(fileName, ATLANTIS_ATTRIBUTE, bm->ecotest, 1, attributeGroupNode, no_checking, varStr);

				sprintf(varStr, "%s_%s_Cy", FunctGroupArray[sp].groupCode, bm->contaminantStructure[cIndex]->contaminant_name);
				bm->contaminantStructure[cIndex]->sp_Cy[sp] = Util_XML_Read_Value(fileName, ATLANTIS_ATTRIBUTE, bm->ecotest, 1, attributeGroupNode, no_checking, varStr);

                sprintf(varStr, "%s_%s_spL", FunctGroupArray[sp].groupCode, bm->contaminantStructure[cIndex]->contaminant_name);
                bm->contaminantStructure[cIndex]->sp_L[sp] = Util_XML_Read_Value(fileName, ATLANTIS_ATTRIBUTE, bm->ecotest, 1, attributeGroupNode, no_checking, varStr);
                
                sprintf(varStr, "%s_%s_spA", FunctGroupArray[sp].groupCode, bm->contaminantStructure[cIndex]->contaminant_name);
                bm->contaminantStructure[cIndex]->sp_A[sp] = Util_XML_Read_Value(fileName, ATLANTIS_ATTRIBUTE, bm->ecotest, 1, attributeGroupNode, no_checking, varStr);

                sprintf(varStr, "%s_%s_spB", FunctGroupArray[sp].groupCode, bm->contaminantStructure[cIndex]->contaminant_name);
                bm->contaminantStructure[cIndex]->sp_B[sp] = Util_XML_Read_Value(fileName, ATLANTIS_ATTRIBUTE, bm->ecotest, 1, attributeGroupNode, no_checking, varStr);

                sprintf(varStr, "%s_%s_spL_r", FunctGroupArray[sp].groupCode, bm->contaminantStructure[cIndex]->contaminant_name);
                bm->contaminantStructure[cIndex]->sp_L_r[sp] = Util_XML_Read_Value(fileName, ATLANTIS_ATTRIBUTE, bm->ecotest, 1, attributeGroupNode, no_checking, varStr);
                
                sprintf(varStr, "%s_%s_spA_r", FunctGroupArray[sp].groupCode, bm->contaminantStructure[cIndex]->contaminant_name);
                bm->contaminantStructure[cIndex]->sp_A_r[sp] = Util_XML_Read_Value(fileName, ATLANTIS_ATTRIBUTE, bm->ecotest, 1, attributeGroupNode, no_checking, varStr);
                
                sprintf(varStr, "%s_%s_spB_r", FunctGroupArray[sp].groupCode, bm->contaminantStructure[cIndex]->contaminant_name);
                bm->contaminantStructure[cIndex]->sp_B_r[sp] = Util_XML_Read_Value(fileName, ATLANTIS_ATTRIBUTE, bm->ecotest, 1, attributeGroupNode, no_checking, varStr);
                
                sprintf(varStr, "%s_%s_avoid", FunctGroupArray[sp].groupCode, bm->contaminantStructure[cIndex]->contaminant_name);
                bm->contaminantStructure[cIndex]->sp_avoid[sp] = Util_XML_Read_Value(fileName, ATLANTIS_ATTRIBUTE, bm->ecotest, 1, attributeGroupNode, no_checking, varStr);

                sprintf(varStr, "%s_%s_K_avoid", FunctGroupArray[sp].groupCode, bm->contaminantStructure[cIndex]->contaminant_name);
                bm->contaminantStructure[cIndex]->sp_K_avoid[sp] = Util_XML_Read_Value(fileName, ATLANTIS_ATTRIBUTE, bm->ecotest, 1, attributeGroupNode, no_checking, varStr);

                /* Don't think we will use this - invitro used instant dose mort for eating */
                sprintf(varStr, "%s_%s_InstantDoseMortality", FunctGroupArray[sp].groupCode, bm->contaminantStructure[cIndex]->contaminant_name);
                bm->contaminantStructure[cIndex]->sp_instantDoseMortality[sp] = Util_XML_Read_Value(fileName, ATLANTIS_ATTRIBUTE, bm->ecotest, 1, attributeGroupNode, no_checking, varStr);
                
                sprintf(varStr, "%s_%s_GrowthThresh", FunctGroupArray[sp].groupCode, bm->contaminantStructure[cIndex]->contaminant_name);
                bm->contaminantStructure[cIndex]->sp_GrowthThresh[sp] = Util_XML_Read_Value(fileName, ATLANTIS_ATTRIBUTE, bm->ecotest, 1, attributeGroupNode, no_checking, varStr);
                
                sprintf(varStr, "%s_%s_ReprodThresh", FunctGroupArray[sp].groupCode, bm->contaminantStructure[cIndex]->contaminant_name);
                bm->contaminantStructure[cIndex]->sp_ReprodThresh[sp] = Util_XML_Read_Value(fileName, ATLANTIS_ATTRIBUTE, bm->ecotest, 1, attributeGroupNode, no_checking, varStr);
                
                sprintf(varStr, "%s_%s_GrowthEffect", FunctGroupArray[sp].groupCode, bm->contaminantStructure[cIndex]->contaminant_name);
			          bm->contaminantStructure[cIndex]->sp_GrowthEffect[sp] = Util_XML_Read_Value(fileName, ATLANTIS_ATTRIBUTE, bm->ecotest, 1, attributeGroupNode, no_checking, varStr);
                
                sprintf(varStr, "%s_%s_MoveEffect", FunctGroupArray[sp].groupCode, bm->contaminantStructure[cIndex]->contaminant_name);
                bm->contaminantStructure[cIndex]->sp_MoveEffect[sp] = Util_XML_Read_Value(fileName, ATLANTIS_ATTRIBUTE, bm->ecotest, 1, attributeGroupNode, no_checking, varStr);
                
                sprintf(varStr, "%s_%s_ReprodEffect", FunctGroupArray[sp].groupCode, bm->contaminantStructure[cIndex]->contaminant_name);
                bm->contaminantStructure[cIndex]->sp_ReprodEffect[sp] = Util_XML_Read_Value(fileName, ATLANTIS_ATTRIBUTE, bm->ecotest, 1, attributeGroupNode, no_checking, varStr);
                
                sprintf(varStr, "%s_%s_maternal_transfer", FunctGroupArray[sp].groupCode, bm->contaminantStructure[cIndex]->contaminant_name);
                bm->contaminantStructure[cIndex]->sp_maternal_transfer[sp] = Util_XML_Read_Value(fileName, ATLANTIS_ATTRIBUTE, bm->ecotest, 1, attributeGroupNode, no_checking, varStr);

                sprintf(varStr, "%s_%s_suckling_transfer", FunctGroupArray[sp].groupCode, bm->contaminantStructure[cIndex]->contaminant_name);
                bm->contaminantStructure[cIndex]->sp_suckling_transfer[sp] = Util_XML_Read_Value(fileName, ATLANTIS_ATTRIBUTE, bm->ecotest, 1, attributeGroupNode, no_checking, varStr);

                sprintf(varStr, "%s_%s_ContamScalar", FunctGroupArray[sp].groupCode, bm->contaminantStructure[cIndex]->contaminant_name);
                bm->contaminantStructure[cIndex]->sp_ContamScalar[sp] = Util_XML_Read_Value(fileName, ATLANTIS_ATTRIBUTE, bm->ecotest, 1, attributeGroupNode, no_checking, varStr);

			}
		}

		sprintf(varStr, "%s_half_life", bm->contaminantStructure[cIndex]->contaminant_name);
		bm->contaminantStructure[cIndex]->half_life = Util_XML_Read_Value(fileName, ATLANTIS_ATTRIBUTE, bm->ecotest, 1, attributeGroupNode, no_checking, varStr);

        sprintf(varStr, "%s_temp_depend", bm->contaminantStructure[cIndex]->contaminant_name);
        bm->contaminantStructure[cIndex]->contam_temp_depend = Util_XML_Read_Value(fileName, ATLANTIS_ATTRIBUTE, bm->ecotest, 1, attributeGroupNode, integer_check, varStr);

        sprintf(varStr, "%s_dissolv_coefft", bm->contaminantStructure[cIndex]->contaminant_name);
        bm->contaminantStructure[cIndex]->dissolv_coefft = Util_XML_Read_Value(fileName, ATLANTIS_ATTRIBUTE, bm->ecotest, 1, attributeGroupNode, integer_check, varStr);
        
        if (bm->num_contaminants > 1) {
            sprintf(varStr, "%s_contam_interaction", bm->contaminantStructure[cIndex]->contaminant_name);
            if (Util_XML_Read_Array_Double(ATLANTIS_ATTRIBUTE, fileName, "Contaminants", attributeGroupNode, no_checking, varStr, &values, bm->num_contaminants) == FALSE){
                quit("Error: Unable to read parameter %s from input file %s\n", varStr, fileName);
            }
            
            for(contamIndex = 0; contamIndex < bm->num_contaminants; contamIndex++){
               bm->contaminantStructure[cIndex]->interact_coefft[contamIndex] = values[contamIndex];
            }
        }
	}
}

/**
 *	This routine reads all parameters of the ecological models
 *
 *  Input:  char *filename: pointer to biology input XML file.
 *
 */
int Read_Biology_Parameters(MSEBoxModel *bm, char *filename, FILE *llogfp) {
	xmlDocPtr inputDoc;
	int guild, sp;

	inputDoc = xmlReadFileDestFolder(bm->destFolder, filename, NULL, 0);
	/** Flags and switches ******************/

	/* Flags and switches determining set-up and recording */
	printf("Biological parameter file loading\n");
	printf("0                                 100\n");
	printf("**");
	fflush(stdout);
    
//
//	for (guild = 0; guild < bm->K_num_tot_sp; guild++)
//		if (FunctGroupArray[guild].isVertebrate == FALSE)
//			FunctGroupArray[guild].flagLim = FALSE;

	/* Initalise the min and max depth values for each group - this is only used for the shallow and deep benthic groups.*/
	for (guild = 0; guild < bm->K_num_tot_sp; guild++) {
		FunctGroupArray[guild].speciesParams[mindepth_id] = 0;
		FunctGroupArray[guild].speciesParams[maxdepth_id] = 1000000; /* Just a large number */
        FunctGroupArray[guild].speciesParams[maxtotdepth_id] = 1000000; /* Just a large number */
	}

	Read_Setup_Values(bm, filename, inputDoc->children);
	Read_Group_Flags(bm, filename, inputDoc->children);
	Read_Physical_Limitation(bm, filename, inputDoc->children);

	if(bm->track_atomic_ratio == TRUE){
		Read_Additional_Tracer_Values(bm, filename, inputDoc->children);
	}
	Read_Physical_Chem_Values(bm, filename, inputDoc->children);
	Read_Tolerance_Values(bm, filename, inputDoc->children);
	Read_Evolution_Values(bm, filename, inputDoc->children, llogfp);
	Read_Primary_Producer_Values(bm, filename, inputDoc->children);
	Read_Bacteria_Values(bm, filename, inputDoc->children);
	Read_Habitat_Values(bm, filename, inputDoc->children);

	Read_Spatial_Threshold_Values(bm, filename, inputDoc->children);
	Read_Movement_Values(bm, filename, inputDoc->children);
	Read_Vert_Body_Values(bm, filename, inputDoc->children);
	Read_Diet_Information_Values(bm, filename, inputDoc->children);
	Read_Q10_Information_Values(bm, filename, inputDoc->children);
	Read_Encystment(bm, filename, inputDoc->children);
	Read_Fish_Respiration_Values(bm, filename, inputDoc->children);
    
	Read_Mortality_Values(bm, filename, inputDoc->children);
	Read_Detritus_Conversion_Values(bm, filename, inputDoc->children);
	Read_Reproduction_Values(bm, filename, inputDoc->children);
	Read_Stock_Structure_Values(bm, filename, inputDoc->children);
	Read_Invading_Species_Values(bm, filename, inputDoc->children);
	//Read_Migration_Information(bm, filename, inputDoc->children, llogfp); - Now handled separately
	if(bm->containsCoral)
		Read_Coral_Species_Values(bm, filename, inputDoc->children, llogfp);
	
	// Check number of spawnings per year - if age structured groups
	for (sp = 0; sp < bm->K_num_tot_sp; sp++) {
		if(FunctGroupArray[sp].groupAgeType == AGE_STRUCTURED) {
			if(((int)(FunctGroupArray[sp].numSpawns)) < 1) {
				quit("You can not have an age structured group without at least one spawning per year - fix for %s\n", FunctGroupArray[sp].groupCode);
			}
		}
	}

	if(bm->ice_on){
		Read_Ice_Values(bm, filename, inputDoc->children);
	}

	if(bm->track_contaminants){
		Read_Contaminant_Values(bm, filename, inputDoc->children);
	}

	printf("Checking the loaded biology data\n");

	// Calculate the number of active functional groups.
	bm->num_active_funcGroups = 0;
	for (sp = 0; sp < bm->K_num_tot_sp; sp++) {
		if (FunctGroupArray[sp].isVertebrate == TRUE) {
			if (FunctGroupArray[sp].speciesParams[flag_id] == TRUE)
				bm->num_active_funcGroups++;
		}
	}
    
    /* Record habitat defining groups */
	for (sp = 0; sp < bm->K_num_tot_sp; sp++) {
		if(FunctGroupArray[sp].isCover == TRUE)
			FunctGroupArray[sp].speciesParams[isbiogenhab_id] = TRUE;
	}
    
    bm->K_num_migout = K_max_num_mig;
	/* Get the max number of cohorts in/out of the model. This value is used to allocate the migration arrays */
	for (sp = 0; sp < bm->K_num_tot_sp; sp++) {
		if (FunctGroupArray[sp].num_migrate > bm->K_num_migout)
			bm->K_num_migout = (int) (FunctGroupArray[sp].num_migrate);
	}

	printf("**");
	fflush(stdout);

	for (sp = 0; sp < bm->K_num_tot_sp; sp++) {
		if (FunctGroupArray[sp].isVertebrate == TRUE) {
			if (FunctGroupArray[sp].speciesParams[flagdem_id])
				FunctGroupArray[sp].speciesParams[PBscale_id] = p_PBben;
			else
				FunctGroupArray[sp].speciesParams[PBscale_id] = p_PBwc;
			FunctGroupArray[sp].speciesParams[BBscale_id] = p_BBfish;
		}
	}
    
    /* In case need temperature dependent efficiencies store original values */
	for (sp = 0; sp < bm->K_num_tot_sp; sp++) {
		if (FunctGroupArray[sp].isVertebrate == TRUE) {
			FunctGroupArray[sp].speciesParams[E1orig_id] = FunctGroupArray[sp].speciesParams[E1_id];
			FunctGroupArray[sp].speciesParams[E2orig_id] = FunctGroupArray[sp].speciesParams[E2_id];
			FunctGroupArray[sp].speciesParams[E3orig_id] = FunctGroupArray[sp].speciesParams[E3_id];
			FunctGroupArray[sp].speciesParams[E4orig_id] = FunctGroupArray[sp].speciesParams[E4_id];
		}
	}
    
    /* TODO: Change these to species values - read in a value per group. */
	for (sp = 0; sp < bm->K_num_tot_sp; sp++) {
		if (FunctGroupArray[sp].isVertebrate == TRUE) {
			if ((FunctGroupArray[sp].groupType == FISH) || (FunctGroupArray[sp].groupType == FISH_INVERT))
				FunctGroupArray[sp].speciesParams[KST_id] = KST_fish;
			else if (FunctGroupArray[sp].groupType == SHARK)
				FunctGroupArray[sp].speciesParams[KST_id] = KST_shark;
			else if (FunctGroupArray[sp].groupType == BIRD)
				FunctGroupArray[sp].speciesParams[KST_id] = KST_bird;
			else
				FunctGroupArray[sp].speciesParams[KST_id] = KST_mammal;

			if ((FunctGroupArray[sp].groupType == FISH) || (FunctGroupArray[sp].groupType == FISH_INVERT))
				FunctGroupArray[sp].speciesParams[Ktmp_id] = Ktmp_fish;
			else if (FunctGroupArray[sp].groupType == SHARK)
				FunctGroupArray[sp].speciesParams[Ktmp_id] = Ktmp_shark;
			else if (FunctGroupArray[sp].groupType == BIRD)
				FunctGroupArray[sp].speciesParams[Ktmp_id] = Ktmp_bird;
			else
				FunctGroupArray[sp].speciesParams[Ktmp_id] = Ktmp_mammal;

            if (FunctGroupArray[sp].groupType == FISH_INVERT)
                FunctGroupArray[sp].speciesParams[FDL_id] = FDL_benth;
			else if ((FunctGroupArray[sp].groupType == FISH) || (FunctGroupArray[sp].groupType == SHARK))
				FunctGroupArray[sp].speciesParams[FDL_id] = FDL_fish;
			else
				FunctGroupArray[sp].speciesParams[FDL_id] = FDL_top;

		} else {
			/* Age class of maturity */
			FunctGroupArray[sp].speciesParams[age_mat_id] = 1;
		}
	}
    
    xmlFreeDoc(inputDoc);
	/* Shutdown libxml */
	xmlCleanupParser();
    
	return TRUE;
}

/**
 * \brief Read in the stock parameters from the input file.
 *
 *
 */
int Read_Biology_Stock_Parameters(MSEBoxModel *bm, char *fileName) {
	xmlDocPtr inputDoc;
	xmlNodePtr attributeGroupNode, parent;
	inputDoc = xmlReadFileDestFolder(bm->destFolder, fileName, NULL, XML_PARSE_PEDANTIC);

	parent = inputDoc->children;

	if (verbose)
		printf("Reading the Reproduction values\n");

	attributeGroupNode = Util_XML_Get_Node(ATLANTIS_ATTRIBUTE_SUB_GROUP, parent, "Reproduction");
	if (attributeGroupNode == NULL)
		quit("Reproduction attribute group not found in input file %s.\n", fileName);

	if (readin_popratio) {
		Read_Popratio_Stock_XML(bm, fileName, attributeGroupNode);
	}

	Read_KDENR_XML(bm, fileName, attributeGroupNode);
	Read_RecSTOCK_XML(bm, fileName, attributeGroupNode);

	if (verbose)
		printf("Reading the Diet values\n");

	attributeGroupNode = Util_XML_Get_Node(ATLANTIS_ATTRIBUTE_SUB_GROUP, parent, "Diet");
	if (attributeGroupNode == NULL)
		quit("Diet attribute group not found in input file %s.\n", fileName);

	Read_PStock_XML(bm, fileName, attributeGroupNode);
	xmlFreeDoc(inputDoc);

	/* Shutdown libxml */
	xmlCleanupParser();
	return 1;
}

/***************** Migration Parameters ****************************/
/**
 *    \brief Read the data into the MigBoxIO array from the input XML file.
 */
static void Read_MigrateIO_Box_XML(MSEBoxModel *bm, char *fileName, xmlNodePtr parent) {
    
    char str[100];
    double *values = 0;
    int guild, cohort, migrate, b, all_one;
    char errorString[STRLEN];
    xmlNodePtr attributeNode, specieNode, cohortNode;
    
    if (verbose)
        printf("Reading MigrateIOBox values\n");
    
    attributeNode = Util_XML_Get_Node(ATLANTIS_ATTRIBUTE, parent, "MigrateIOBox");
    if (attributeNode == NULL)
        quit("Migration/MigrateIOBox attribute group not found in input file %s.\n", fileName);
    
    /* Create a node for each functional group*/
    for (guild = 0; guild < bm->K_num_tot_sp; guild++) {
        if (FunctGroupArray[guild].sp_geo_move == TRUE) {
            if(FunctGroupArray[guild].num_migrate){
                specieNode = Util_XML_Get_Node(ATLANTIS_GROUP_ATTRIBUTE, attributeNode, FunctGroupArray[guild].groupCode);
                if (specieNode == NULL)
                    quit("Migration/MigrateIOBox/%s species attribute group not found.\n", FunctGroupArray[guild].groupCode);
                
                switch (FunctGroupArray[guild].groupAgeType) {
                    case AGE_STRUCTURED_BIOMASS:
                    case AGE_STRUCTURED:
                        for (cohort = 0; cohort < FunctGroupArray[guild].numStages; cohort++) {
                            sprintf(errorString, "Migration/MigrateIOBox/%s/%s", FunctGroupArray[guild].groupCode, cohortString[cohort]);
                            cohortNode = Util_XML_Get_Node(ATLANTIS_AGE_CLASS_ATTRIBUTE, specieNode, cohortString[cohort]);
                            if (cohortNode == NULL)
                                quit("%s cohort attribute group not found.\n", errorString);
                            
                            for (migrate = 0; migrate < FunctGroupArray[guild].num_migrate; migrate++) {
                                sprintf(str, "Migrate%d", migrate + 1);
                                if (Util_XML_Read_Array_Double(ATLANTIS_ATTRIBUTE, fileName, errorString, cohortNode, proportion_check, str, &values, bm->nbox) == FALSE) {
                                    quit("Error: Unable to find parameter '%s/%s' in input file %s\n", errorString, str, fileName);
                                }
                                all_one = 1;
                                for (b = 0; b < bm->nbox; b++) {
                                    MIGRATION[guild].Box_Prm[b][cohort][migrate] = values[b];
                                    if(values[b] < 1.0) {
                                        all_one = 0;
                                    }
                                }
                                MIGRATION[guild].all_go_Prm[cohort][migrate] = all_one;
                                
                                free(values);
                            }
                        }
                        break;
                    case BIOMASS:
                        cohort = juv_id;
                        sprintf(errorString, "Migration/MigrateIOBox/%s", FunctGroupArray[guild].groupCode);
                        for (migrate = 0; migrate < FunctGroupArray[guild].num_migrate; migrate++) {
                            sprintf(str, "Migrate%d", migrate + 1);
                            if (Util_XML_Read_Array_Double(ATLANTIS_ATTRIBUTE, fileName, errorString, specieNode, proportion_check, str, &values, bm->nbox) == FALSE) {
                                quit("Error: Unable to find parameter '%s/%s' in input file %s\n", errorString, str,
                                     fileName);
                            }
                            all_one = 1;
                            for (b = 0; b < bm->nbox; b++) {
                                MIGRATION[guild].Box_Prm[b][cohort][migrate] = values[b];
                                if(values[b] < 1.0) {
                                    all_one = 0;
                                }
                            }
                            MIGRATION[guild].all_go_Prm[cohort][migrate] = all_one;
                            
                            free(values);
                        }
                        break;
                }
            }
        }
    }
}

/**
 *    \brief Read the data into the KMIG array from the input XML file.
 */
static void Read_KMIG_XML(MSEBoxModel *bm, char *fileName, xmlNodePtr parent, char *parameterName, int parameterIndex, FILE *llogfp) {

    double *values = 0;
    int guild, b, qid, basechrt;
    xmlNodePtr attributeNode;
    char errorString[STRLEN];
    
    qid = 0; // As assume only one lot outside the model at start time.
    // TODO: May need to change this - to allow for multiple queue entries at the start
    
    if (verbose)
        printf("Reading %s values\n", parameterName);
    
    sprintf(errorString, "Migration/%s", parameterName);
    attributeNode = Util_XML_Get_Node(ATLANTIS_ATTRIBUTE, parent, parameterName);
    if (attributeNode == NULL)
        quit("errorString%s attribute group not found.\n", errorString);
    
    for (guild = 0; guild < bm->K_num_tot_sp; guild++) {
        // If doesn't migrate this often leave
        if(FunctGroupArray[guild].num_migrate < 1)
            continue;
        
        if (FunctGroupArray[guild].isVertebrate == TRUE) {
            if (Util_XML_Read_Array_Double(ATLANTIS_GROUP_ATTRIBUTE, fileName, errorString, attributeNode, no_checking, FunctGroupArray[guild].groupCode, &values, bm->K_num_max_cohort) == FALSE) {
                quit("Error: Unable to find parameter '%s/%s' in input file %s\n", errorString, FunctGroupArray[guild].groupCode, fileName);
            }
            for (b = 0; b < FunctGroupArray[guild].numCohortsXnumGenes; b++){
                basechrt = (int)floor(b / FunctGroupArray[guild].numGeneTypes);
                switch(parameterIndex){
                    case DEN_id:
                        MIGRATION[guild].InitDEN[b][qid] = values[basechrt];
                        break;
                    case SN_id:
                        MIGRATION[guild].InitSN[b][qid] = values[basechrt];
                        break;
                    case RN_id:
                        MIGRATION[guild].InitRN[b][qid] = values[basechrt];
                        break;
                    default:
                        quit("No such parameterIndex %d for DEN, SN, RN parameters\n", parameterIndex);
                        break;
                }
            }
            free(values);
        }
    }
}

/**
 *    \brief Read the data into the recruit_vdistrib array from the input XML file.
 */
static void Read_KMIG_INVERT_XML(MSEBoxModel *bm, char *fileName, xmlNodePtr parent, char *parameterName){
    
    double *values = 0;
    int guild, b, qid, basechrt;
    xmlNodePtr attributeNode;
    char parameterNameMigNum[STRLEN];
    char errorString[STRLEN];
    
    qid = 0; // As assume only one lot outside the model at start time.
    // TODO: May need to change this - to allow for multiple queue entries at the start

    if (verbose)
        printf("Reading %s values\n", parameterName);
    
    sprintf(errorString, "Migration/%s", parameterName);
    attributeNode = Util_XML_Get_Node(ATLANTIS_ATTRIBUTE, parent, parameterName);
    if (attributeNode == NULL)
        quit("%s attribute group not found.\n", errorString);
    
    /* Create a node for each functional group*/
    for (guild = 0; guild < bm->K_num_tot_sp; guild++) {
        for (qid = 0; qid < FunctGroupArray[guild].num_migrate; qid++) {
            sprintf(errorString, "Migration/%s%d", parameterName,qid);
            sprintf(parameterNameMigNum, "Migration/%s%d", parameterName, qid); // This wasn't originally set - was that intentional?
            attributeNode = Util_XML_Get_Node(ATLANTIS_ATTRIBUTE, parent, parameterNameMigNum);
            
            // Load those migrations that do happen
            if (FunctGroupArray[guild].isVertebrate == FALSE && FunctGroupArray[guild].sp_geo_move == TRUE) {
                
                printf("Reading %s values for %s with %d genotypes and %d cohorts - %s\n", parameterName, FunctGroupArray[guild].groupCode, FunctGroupArray[guild].numGeneTypes, FunctGroupArray[guild].numCohortsXnumGenes, errorString);
                
                if (Util_XML_Read_Array_Double(ATLANTIS_GROUP_ATTRIBUTE, fileName, errorString, attributeNode, no_checking, FunctGroupArray[guild].groupCode, &values, FunctGroupArray[guild].numCohorts) == FALSE) {
                    quit("Error: Unable to find parameter '%s/%s' in input file %s\n", errorString, FunctGroupArray[guild].groupCode, fileName);
                }
                
                printf("Read %s values for %s now to assign\n", parameterName, FunctGroupArray[guild].groupCode);

                
                for (b = 0; b < FunctGroupArray[guild].numCohortsXnumGenes; b++){
                    basechrt = (int)floor(b / FunctGroupArray[guild].numGeneTypes);
                    MIGRATION[guild].InitDEN[b][qid] = values[basechrt];
                }
                free(values);
            }
        }
    }
}

/* Migration parameter read - those now handled through csv commented out for now - TODO: eventually remove */
int Read_Migration_Information(MSEBoxModel *bm, char *fileName, xmlNodePtr parent, FILE *llogfp) {
    xmlNodePtr attributeGroupNode;
    
    attributeGroupNode = Util_XML_Get_Node(ATLANTIS_ATTRIBUTE_SUB_GROUP, parent, "Migration");
    if (attributeGroupNode == NULL)
        quit("Migration attribute group not found in input file %s.\n", fileName);

    k_migslow = Util_XML_Read_Value(fileName, ATLANTIS_ATTRIBUTE, bm->ecotest, 1, attributeGroupNode, no_checking, "k_migslow");
        
    Read_MigrateIO_Box_XML(bm, fileName, attributeGroupNode);
    
    Read_KMIG_XML(bm, fileName, attributeGroupNode, "KMIG_DEN", DEN_id, llogfp);
    Read_KMIG_XML(bm, fileName, attributeGroupNode, "KMIG_RN", RN_id, llogfp);
    Read_KMIG_XML(bm, fileName, attributeGroupNode, "KMIG_SN", SN_id, llogfp);
    
    Read_KMIG_INVERT_XML(bm, fileName, attributeGroupNode, "KMIG_INVERT");
    
    return TRUE;
}

/* Create a zeroed MIGRATION array */
void Zero_Create_Migration_Arrays(MSEBoxModel *bm, FILE *llogfp) {
    int n = 0, sp;
    
    bm->K_max_num_migration = 0;
    MIGRATION = (MigrationStruct *) malloc(sizeof(MigrationStruct) * (size_t)(bm->K_num_tot_sp + 1));
    
    for (sp = 0; sp < bm->K_num_tot_sp; sp++) {
        MIGRATION[sp].num_in_queue = 0;
        MIGRATION[sp].num_in_queue_done = 0;
        MIGRATION[sp].ActualMigrator = Util_Alloc_Init_1D_Int(FunctGroupArray[sp].numStages, 0);
        MIGRATION[sp].totprop_mig = Util_Alloc_Init_1D_Double(FunctGroupArray[sp].numCohortsXnumGenes, 0);
        
        for (n = 0; n < FunctGroupArray[sp].numStages; n++) {
            MIGRATION[sp].ActualMigrator[n] = 0;
        }
        
        for (n = 0; n < FunctGroupArray[sp].numCohortsXnumGenes; n++) {
            MIGRATION[sp].totprop_mig[n] = 0.0;
        }
    }
    return;
}

/* Create the migration arrays */
void Create_Migration_Arrays(MSEBoxModel *bm, FILE *llogfp) {
    int sp, cohort;
    int maxnum, num_migs;
    //int use_nyears;  // Was used previously but not anymore excpet for reporting
    int nyears = (int) (ceil((bm->tstop - bm->tstart) / (365.0 * 86400.0))) + 1;
    
    bm->K_max_num_migration = 0;
    for(sp = 0; sp < bm->K_num_tot_sp; sp++){
        if(FunctGroupArray[sp].sp_geo_move){
            if(FunctGroupArray[sp].num_migrate > bm->K_max_num_migration)
                bm->K_max_num_migration = (int)FunctGroupArray[sp].num_migrate;
        }
    }
    // Create MIGRATION data structure
    printf("Creating MIGRATION arrays\n");
    
    MIGRATION = (MigrationStruct *) malloc(sizeof(MigrationStruct) * (size_t)(bm->K_num_tot_sp + 1));
    for (sp = 0; sp < bm->K_num_tot_sp; sp++) {
        cohort = FunctGroupArray[sp].numCohortsXnumGenes;
        maxnum = (FunctGroupArray[sp].numStages * nyears * FunctGroupArray[sp].num_migrate) + 1;  // Extra one to allow for species where returns in first year are due to migrations before the model began
        num_migs = FunctGroupArray[sp].num_migrate + 1;
        //use_nyears = 1;
        if(FunctGroupArray[sp].multiyr_mig) {
            maxnum = (nyears * FunctGroupArray[sp].multiyr_mig * FunctGroupArray[sp].numStages * (FunctGroupArray[sp].num_migrate + 1));  // Extra one to allow for species where returns in first year are due to migrations before the model began
            //use_nyears = nyears;
            FunctGroupArray[sp].MaxNumMig = maxnum;
            
            //warn("maxnum set to %d for %s - as nyears: %d, multiyr_mig: %d, num_migrate: %d\n", maxnum, FunctGroupArray[sp].groupCode, nyears, FunctGroupArray[sp].multiyr_mig, FunctGroupArray[sp].num_migrate);
        }
        
        //fprintf(bm->logFile, "maxnum set to %d for %s - as nyears: %d, multiyr_mig: %d, num_migrate: %d\n", maxnum, FunctGroupArray[sp].groupCode, nyears, FunctGroupArray[sp].multiyr_mig, FunctGroupArray[sp].num_migrate);
        
        //fprintf(llogfp, "%s has maxnum: %d and cohort %d\n", FunctGroupArray[sp].groupCode, maxnum, cohort);
        
        // Migration queue
        MIGRATION[sp].ActualMigrator = Util_Alloc_Init_1D_Int(FunctGroupArray[sp].numStages, 0);
        
        MIGRATION[sp].aging = Util_Alloc_Init_2D_Double(maxnum, cohort, 0.0);
        MIGRATION[sp].Leave_Now = Util_Alloc_Init_1D_Int(maxnum, MAXINT);
        MIGRATION[sp].Return_Now = Util_Alloc_Init_1D_Int(maxnum, MAXINT);
        
        MIGRATION[sp].all_go = Util_Alloc_Init_2D_Int(maxnum, cohort, 0);
        MIGRATION[sp].survival = Util_Alloc_Init_1D_Double(maxnum, 0);
        MIGRATION[sp].growth = Util_Alloc_Init_1D_Double(maxnum, 0);
        MIGRATION[sp].Return_Period = Util_Alloc_Init_1D_Int(maxnum, 0);
        MIGRATION[sp].Leave_Period = Util_Alloc_Init_1D_Int(maxnum, 0);
        MIGRATION[sp].MinYearsAway = Util_Alloc_Init_1D_Int(maxnum, 0);
        MIGRATION[sp].MaxYearsAway = Util_Alloc_Init_1D_Int(maxnum, 0);
        MIGRATION[sp].IsAnnualMigration = Util_Alloc_Init_1D_Int(maxnum, 0);
        MIGRATION[sp].returnstock = Util_Alloc_Init_1D_Int(maxnum, 0);
        MIGRATION[sp].start_return_chrt = Util_Alloc_Init_1D_Int(maxnum, 0);
        MIGRATION[sp].start_cohort = Util_Alloc_Init_1D_Int(maxnum, 0);
        MIGRATION[sp].cohort_migrating = Util_Alloc_Init_2D_Int(maxnum, cohort, 0);
        
        MIGRATION[sp].IsPartialMigration = Util_Alloc_Init_1D_Int(maxnum, 0);
        MIGRATION[sp].PartialMinAge = Util_Alloc_Init_1D_Int(maxnum, 0);
        MIGRATION[sp].PartialMaxAge = Util_Alloc_Init_1D_Int(maxnum, 0);
        
        MIGRATION[sp].num_stagger = Util_Alloc_Init_1D_Double(maxnum, 0);
        MIGRATION[sp].Stagger = Util_Alloc_Init_2D_Int(2, maxnum, 0);
        
        MIGRATION[sp].num_aging_event = Util_Alloc_Init_1D_Double(maxnum, 0);
        MIGRATION[sp].current_pop_ratio = Util_Alloc_Init_2D_Double(FunctGroupArray[sp].ageClassSize, cohort, 0);
        MIGRATION[sp].pop_ratio = Util_Alloc_Init_3D_Double(FunctGroupArray[sp].ageClassSize, cohort, maxnum, 0);
        MIGRATION[sp].prop_mig = Util_Alloc_Init_2D_Double(maxnum, cohort, 0);
        MIGRATION[sp].totprop_mig = Util_Alloc_Init_1D_Double(cohort, 0);
        MIGRATION[sp].recruit = Util_Alloc_Init_2D_Double(maxnum, FunctGroupArray[sp].numGeneTypes, 0);
        
        MIGRATION[sp].DEN = Util_Alloc_Init_2D_Double(maxnum, cohort, 0);
        MIGRATION[sp].SN = Util_Alloc_Init_2D_Double(maxnum, cohort, 0);
        MIGRATION[sp].RN = Util_Alloc_Init_2D_Double(maxnum, cohort, 0);
        
        MIGRATION[sp].MigYOY = Util_Alloc_Init_2D_Double(maxnum, FunctGroupArray[sp].numGeneTypes, 0);
        MIGRATION[sp].MigYOY_SN = Util_Alloc_Init_2D_Double(maxnum, FunctGroupArray[sp].numGeneTypes, 0);
        MIGRATION[sp].MigYOY_RN = Util_Alloc_Init_2D_Double(maxnum, FunctGroupArray[sp].numGeneTypes, 0);
        
        MIGRATION[sp].Box = Util_Alloc_Init_2D_Double(maxnum, bm->nbox, 0);
        
        MIGRATION[sp].InitDEN = Util_Alloc_Init_2D_Double(maxnum, cohort, 0.0);
        MIGRATION[sp].InitSN = Util_Alloc_Init_2D_Double(maxnum, cohort, 0.0);
        MIGRATION[sp].InitRN = Util_Alloc_Init_2D_Double(maxnum, cohort, 0.0);
        
        MIGRATION[sp].ReprodAllowed = Util_Alloc_Init_2D_Int(maxnum, cohort, 0.0);
        
        MIGRATION[sp].yrs_to_age_pre_model = Util_Alloc_Init_1D_Int(maxnum, 0);
        MIGRATION[sp].end_pt = Util_Alloc_Init_1D_Int(maxnum, 0);
        
        MIGRATION[sp].RecruitQueueMatch = Util_Alloc_Init_1D_Int(maxnum, 0);
        
        // Parameter files
        MIGRATION[sp].Box_Prm = Util_Alloc_Init_3D_Double(num_migs, FunctGroupArray[sp].numStages, bm->nbox, 0);
        MIGRATION[sp].StartDay_Prm = Util_Alloc_Init_2D_Int(num_migs, FunctGroupArray[sp].numStages, 0);
        MIGRATION[sp].EndDay_Prm = Util_Alloc_Init_2D_Int(num_migs, FunctGroupArray[sp].numStages, 0);
        MIGRATION[sp].Leave_Period_Prm = Util_Alloc_Init_2D_Int(num_migs, FunctGroupArray[sp].numStages, 0);
        MIGRATION[sp].Return_Period_Prm = Util_Alloc_Init_2D_Int(num_migs, FunctGroupArray[sp].numStages, 0);
        MIGRATION[sp].MinYearsAway_Prm = Util_Alloc_Init_2D_Int(num_migs, FunctGroupArray[sp].numStages, 0);
        MIGRATION[sp].MaxYearsAway_Prm = Util_Alloc_Init_2D_Int(num_migs, FunctGroupArray[sp].numStages, 0);
        MIGRATION[sp].IsAnnualMigration_Prm = Util_Alloc_Init_2D_Int(num_migs, FunctGroupArray[sp].numStages, 0);
        MIGRATION[sp].survival_Prm = Util_Alloc_Init_2D_Double(num_migs, FunctGroupArray[sp].numStages, 0);
        MIGRATION[sp].growth_Prm = Util_Alloc_Init_2D_Double(num_migs, FunctGroupArray[sp].numStages, 0);
        MIGRATION[sp].returnstock_Prm = Util_Alloc_Init_2D_Int(num_migs, FunctGroupArray[sp].numStages, 0);
        MIGRATION[sp].Stagger_Prm = Util_Alloc_Init_3D_Int(2, num_migs, FunctGroupArray[sp].numStages, 0);
        MIGRATION[sp].all_go_Prm = Util_Alloc_Init_2D_Int(num_migs, cohort, 0);
        
        MIGRATION[sp].IsPartialMigration_Prm = Util_Alloc_Init_2D_Int(num_migs, FunctGroupArray[sp].numStages, 0);
        MIGRATION[sp].PartialMigration_MinPrm = Util_Alloc_Init_2D_Int(num_migs, FunctGroupArray[sp].numStages, 0);
        MIGRATION[sp].PartialMigration_MaxPrm = Util_Alloc_Init_2D_Int(num_migs, FunctGroupArray[sp].numStages, 0);
        MIGRATION[sp].ReprodAllowedPrm = Util_Alloc_Init_2D_Int(num_migs, FunctGroupArray[sp].numStages, 0);
        
        if(bm->track_contaminants) {
            MIGRATION[sp].RecruitContam = Util_Alloc_Init_3D_Double(bm->num_contaminants, maxnum, FunctGroupArray[sp].numGeneTypes,0);
            MIGRATION[sp].contam = Util_Alloc_Init_3D_Double(bm->num_contaminants, maxnum, cohort, 0);
        }
    }

    return;
}
