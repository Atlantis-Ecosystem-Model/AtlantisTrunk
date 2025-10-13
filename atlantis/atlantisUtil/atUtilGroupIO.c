/************************************************************************//**
 *	\file
 *	\brief This file contains function associated with loading the functional group information.
 *	\brief \ingroup atUtil
 *
 * Revisions:
 *
 * 			28-05-2008 Bec Gorton
 * 			Fixed a bug in the code that sets the Growth and Eat tracer arrays. The
 * 			Demersal_M_fish was being allocated to FDE and Demeral_P_Fish was being
 * 			allocated to FDF. Changed these to be set correctly.
 *
 *          06-06-2008 Bec Gorton
 *          Added code to read in the full functional group name, the diagnositic
 *          name and the diagnostic tolerance. The full functional group name is used
 *          to build the functional group tracer names. The diagnostic parameters are
 *          used to build the diagnostic tracer names.
 *          Changed the readFunctionalGroupInput function to expect a pointer to the log
 *          file. If debug is set to true the functional group values will be printed out
 *          to the log file as they are read in.
 *          Added code to read in the isPrey and isCover from the input file. Also added code
 *          to determine if the functional group is a producer and if its a grazer.
 *
 *          14-06-2008 Bec Gorton
 *          Added code to read in isGrazer from the functional group input file.
 *          Also removed the assignTracerArrays function as its no longer used.
 *
 *           19/08/2008 Bec Gorton
 *            Added preyID and spawnID to the FunctGroup array. These will eventually be removed.
 *            The values for each functional group are set in assignTracerArrays().
 *
 *            29-10-2008 Bec Gorton
 *            Added the pSPeat array to the Funct Group structure. This has been resized to
 *            include habitat data - thus allowing us to no longer need to DRsed_is index variables
 *            in the atecology lib.
 *
 *			2-12-2008 Beth Fulton
 *			Replace isVerticalMigrator with isMobile (so clearly covers horizontal and vertical movements)
 *
 * 			16-01-2009 Bec Gorton
 * 			Replaced isHarvested with isFished and isImpacted.
 *
 * 			23-01-2009 Bec Gorton
 * 			Changed the size of the cohortSpeciesParams for AGE_STRUCTURED groups from 1 to 2 to allow
 * 			for storage of the adult and juv linear and quatratic mortality values.
 *
 *			02-20-2009 Bec Gorton
 *			Fixed a bug in the new code that sets the functional group isOncePerDT value. This was being
 *			set the wrong way around. Also removed a heap of commented out code.
 *
 *
 *			22-04-2009 Bec Gorton
 *			Added the isSiliconDependant functional group variable. This is currently used by PL and MB
 *			to track the groups silicon tracer.
 *
 *			10-06-2009 Bec Gorton
 *			Added the SEAGRASS invertebrate type.
 *
 *			24-06-2009 Bec Gorton
 *			Changed the size of the FunctGroupArray to bm->K_tot_num_sp + 1
 *			to allow for DIN.
 *
 *			21-10-2009 Bec Gorton
 *			Added the isTAC parameter so i can get rid of the K_num_fishedTac value.
 *
 * 	29-01-2010 Bec Gorton
 * 	Added the CaughtByFisheryTracers and DiscardedByFisheryTracers arrays to
 * 	store the harvest tracers. This means that old tracer variables are no longer required.
 *
 *	07-04-2010 Bec Gorton
 *		Add a couple of new flags to the FunctGroup structure:
 *		isInfauna - true if invert_type is SM_INF or LG_INF
 *		isEpifauna - true if invert_type is one of the filter feeder groups, phytoben, seagrass or MB.
 *		 The values for these flags are set here.
 *
 *
 *	05-07-2012 Bec Gorton
 *	Added support to calculate the detIndex of the detritus groups.
 *
 *	02-06-2012 Beth Fulton
 *	Added overwinter as functional group property
 *
 */
/*  Open library routines */
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>
#include <ctype.h>
#include <sjwlib.h>
#include <atlantisboxmodel.h>
#include <atUtilLib.h>
#include <atEcologyLib.h>
#include <atGroupTypes.h>
#include <convertXML.h>

static regmatch_t pmatch[32];

#ifdef ORIGINAL_GROUP_ORDER
int *groupEatOrder;
int *groupMovementOrder;
int *TLorder2;
int *TLorder3;
int *wcOrder;
int *sedOrder;
int *epiOrder;
#endif

static void Setup_Cover_Groups(MSEBoxModel *bm, FILE *llogfp);
static void Set_Group_Habitats(MSEBoxModel *bm);

#ifdef ORIGINAL_GROUP_ORDER
void setInvertTypeIndex(MSEBoxModel *bm, GROUP_TYPES group_type, int *array, int *index) {
	int sp;

	for (sp = 0; sp < bm->K_num_tot_sp; sp++) {
		if (FunctGroupArray[sp].groupType == group_type) {
			array[*index] = sp;
			(*index)++;
		}
	}
}
#endif


static int Is_Valid_Group_Code(MSEBoxModel *bm, char *groupStr){
	regex_t regBuffer;
	char GroupCode[] = "^[A-Z]{2,3}$";
	// Cannot contain ad or juv

	/* Check that the group code isn't FSM */
	if(strcmp(groupStr, "FSM") == 0){
		warn("FSM is a parameter name in the biology file so isn't a valid group code");
		return FALSE;

	}

	if(strcmp(groupStr, "FSP") == 0){
		warn("FSP is a parameter name in the biology file so isn't a valid group code");
		return FALSE;
	}
	at_compileRegExpression(&regBuffer, GroupCode);
	if (regexec(&(regBuffer), groupStr, elementsof ( pmatch ), pmatch, 0) == 0) {
		regfree(&(regBuffer));
		return TRUE;
	}
	regfree(&(regBuffer));
	return FALSE;

}
/*
 * <AttributeGroup AttributeGroupName="AtlantisFunctionalGroups">
 <AttributeSubGroup AttributeGroupName="FunctionalGroup" AttributeDescription="The defintion of a functional group">
 <Attribute AttributeName = "GroupCode" AttributeValue = "FPL"/>
 <Attribute AttributeName = "Name" AttributeValue = "Planktiv_L_Fish"/>
 <Attribute AttributeName = "Long Name" AttributeValue = "Large planktivorous fish"/>
 <Attribute AttributeName = "DiagTol" AttributeValue = "2"/>
 <Attribute AttributeName = "isVertebrate" AttributeValue = "1"/>
 <Attribute AttributeName = "NumCohorts" AttributeValue = "10"/>
 <Attribute AttributeName = "NumGeneTypes" AttributeValue = "1"/>
 <Attribute AttributeName = "NumStages" AttributeValue = "1"/>
 <Attribute AttributeName = "NumSpawns" AttributeValue = "1"/>
 <Attribute AttributeName = "NumAgeClassSize" AttributeValue = "1"/>
 <Attribute AttributeName = "VerticallyMigrates" AttributeValue = "1"/>
 <Attribute AttributeName = "MigratesOutOfModel" AttributeValue = "1"/>
 <Attribute AttributeName = "isFished" AttributeValue = "1"/>
 <Attribute AttributeName = "isImpacted" AttributeValue = "1"/>
 <Attribute AttributeName = "isTAC" AttributeValue = "1"/>
 <Attribute AttributeName = "groupType" AttributeValue = "-1"/>
 <Attribute AttributeName = "isPredator" AttributeValue = "1"/>
 <Attribute AttributeName = "isCover" AttributeValue = "0"/>
 <Attribute AttributeName = "isGrazer" AttributeValue = "1"/>
 <Attribute AttributeName = "isSiliconDep" AttributeValue = "0"/>
 <Attribute AttributeName = "isAssessed" AttributeValue = "1"/>
 </AttributeSubGroup>
 <AttributeGroup>

 */
int Util_Read_Functional_Group_XML(MSEBoxModel *bm, char *fileName, FILE *llogfp) {

	xmlDocPtr inputDoc;
	xmlXPathObjectPtr groupList;
	int groupIndex, numGroups;
	xmlNodePtr groupNode;
	char varStr[STRLEN];
	int j, i;
	int guild, found;
	int nlayer = bm->wcnz;
	int ncells = bm->nbox;
	int nstock = bm->K_num_stocks_per_sp;
	int detIndex;
	int count;
	char convertedXMLFileName[STRLEN];
	int shouldQuit = FALSE;
#ifdef ORIGINAL_GROUP_ORDER
	int eatIndex = 0;
	int vertCount = 0;
#endif

    if (verbose > 0)
        printf("Read_Functional_Group_XML\n");

    bm->K_max_invert_cohorts = 0;  // Initialise as set below
    bm->flag_multiyr_migs = 0; // Initialise as set below

	RefDetIndex = -1;
	LabDetIndex = -1;
	SedBactIndex = -1;
	pelagicBactIndex = -1;
	CarrionIndex = -1;
	IceBactIndex = -1;

	bm->containsCoral = 0;
    bm->containsSponge = 0;
    
    if (strstr(fileName, ".csv") != NULL) {

		/* Convert the file to XML. */
		/* Build the converted filename */
		sprintf(convertedXMLFileName, "%s", fileName);
		*(strstr(convertedXMLFileName, ".csv")) = '\0';
		strcat(convertedXMLFileName, ".xml");

		/* Convert the input file to XML - the XML file will be stored in the destination folder if present.*/
		Convert_Groups_To_XML(bm, fileName, convertedXMLFileName);
        
		inputDoc = xmlReadFileDestFolder(bm->destFolder, convertedXMLFileName, NULL, 0);

	}else{
		sprintf(convertedXMLFileName, "%s", fileName);
		inputDoc = xmlReadFileDestFolder("", convertedXMLFileName, NULL, 0);


	}

	if (!inputDoc) {
		fprintf(stderr, "\n\nERROR. Util_Read_Functional_Group_XML: Failed to open functional group input file %s\nThis may be because the file does not exist or it may not be valid XML.\n", convertedXMLFileName);
		exit(-1);

	}

	/* Read in each of the functional groups */
	groupList = Util_XML_Get_Node_List(ATLANTIS_ATTRIBUTE_SUB_GROUP, inputDoc->children, "FunctionalGroup");

	if (groupList && groupList->nodesetval->nodeNr > 0) {

		/* Check the number of groups match the values in the run input file*/
		numGroups = groupList->nodesetval->nodeNr;
		if (numGroups != bm->K_num_tot_sp) {
			quit("Util_Read_Functional_Group_XML: Number of groups specified (%d) in your group definition file %s does not match the K_num_tot_sp value (%d) in your run file\n", numGroups, convertedXMLFileName, bm->K_num_tot_sp);
		}

		FunctGroupArray = (FunctionalGroupStruct *) malloc(sizeof(FunctionalGroupStruct) * (size_t)(bm->K_num_tot_sp + 1));

#ifdef ORIGINAL_GROUP_ORDER
		if(numGroups > 62)
			quit("Cannot use original order if you have extra groups\n");
#endif
		/* Now read in the groups. */
		for (groupIndex = 0; groupIndex < numGroups; groupIndex++) {
            FunctGroupArray[groupIndex].isBioTurbator = FALSE;

			FunctGroupArray[groupIndex].speciesParams = Util_Alloc_Init_1D_Double(tot_prms, 0.0);

			groupNode = groupList->nodesetval->nodeTab[groupIndex];

			Util_XML_Get_Value_String(convertedXMLFileName, ATLANTIS_ATTRIBUTE, TRUE, groupNode, "GroupCode", FunctGroupArray[groupIndex].groupCode);
			if (verbose > 0)
				printf("Reading in definition of %s\n", FunctGroupArray[groupIndex].groupCode);

			/* Check the group code is a valid code */
			if(!Is_Valid_Group_Code(bm,  FunctGroupArray[groupIndex].groupCode))
				quit("The group code must be a 2 or 3 letter code of upper case letters used to identify each functional group in the input files read into Atlantis.\nThe value %s is not valid\n\n",
						FunctGroupArray[groupIndex].groupCode);

			Util_XML_Get_Value_Double(convertedXMLFileName, ATLANTIS_ATTRIBUTE, 0, TRUE, groupNode, integer_check, "IsTurnedOn",
					&FunctGroupArray[groupIndex].speciesParams[flag_id]);
			Util_XML_Get_Value_String(convertedXMLFileName, ATLANTIS_ATTRIBUTE, TRUE, groupNode, "Name", FunctGroupArray[groupIndex].name);
			Util_XML_Get_Value_String(convertedXMLFileName, ATLANTIS_ATTRIBUTE, TRUE, groupNode, "LongName", FunctGroupArray[groupIndex].fullName);
			Util_XML_Get_Value_Integer(convertedXMLFileName, ATLANTIS_ATTRIBUTE, 0, TRUE, groupNode, integer_check, "NumCohorts", &FunctGroupArray[groupIndex].numCohorts);
			Util_XML_Get_Value_Integer(convertedXMLFileName, ATLANTIS_ATTRIBUTE, 0, TRUE, groupNode, integer_check, "NumGeneTypes", &FunctGroupArray[groupIndex].numGeneTypes);
			Util_XML_Get_Value_Integer(convertedXMLFileName, ATLANTIS_ATTRIBUTE, 0, TRUE, groupNode, integer_check, "NumStages", &FunctGroupArray[groupIndex].numStages);
			Util_XML_Get_Value_Integer(convertedXMLFileName, ATLANTIS_ATTRIBUTE, 0, TRUE, groupNode, integer_check, "NumSpawns", &FunctGroupArray[groupIndex].numSpawns);
			Util_XML_Get_Value_Integer(convertedXMLFileName, ATLANTIS_ATTRIBUTE, 0, TRUE, groupNode, integer_check, "NumAgeClassSize", &FunctGroupArray[groupIndex].ageClassSize);
			Util_XML_Get_Value_Integer(convertedXMLFileName, ATLANTIS_ATTRIBUTE, 0, TRUE, groupNode, integer_check, "NumStocks", &FunctGroupArray[groupIndex].numStocks);

			FunctGroupArray[groupIndex].numCohortsXnumGenes = FunctGroupArray[groupIndex].numCohorts * FunctGroupArray[groupIndex].numGeneTypes;

            if(FunctGroupArray[groupIndex].ageClassSize < 1)
                quit("%s has age class size of 0 - which is not possible, this is likely because you have used a decimal of < 1 in the the groups.csv file. Please set to 1 or more (in the groups.csv file). The model dynamics may still see individuals in a cohort for < 1 year, but it needs to be at least 1 here as it is used in array creation\n", FunctGroupArray[groupIndex].groupCode);
			if(FunctGroupArray[groupIndex].numCohorts < 1)
				quit("%s has less than one cohort - which is not possible, please set to 1 or more (in the groups.csv file)\n", FunctGroupArray[groupIndex].groupCode);
			if(FunctGroupArray[groupIndex].numGeneTypes < 1)
				quit("%s has less than one genetype - which is not possible, please set to 1 or more (in the groups.csv file)\n", FunctGroupArray[groupIndex].groupCode);
			if(FunctGroupArray[groupIndex].numStages < 1)
				quit("%s has less than one stage - which is not possible, please set to 1 or more (in the groups.csv file)\n", FunctGroupArray[groupIndex].groupCode);

			printf("Loaded %s with numCohorts: %d, numGeneTypes: %d, numStages: %d, numSpawns: %d\n", FunctGroupArray[groupIndex].groupCode, FunctGroupArray[groupIndex].numCohorts, FunctGroupArray[groupIndex].numGeneTypes, FunctGroupArray[groupIndex].numStages, FunctGroupArray[groupIndex].numSpawns);

            fprintf(bm->logFile, "Loaded %s with numCohorts: %d, numGeneTypes: %d, numStages: %d, numSpawns: %d\n", FunctGroupArray[groupIndex].groupCode, FunctGroupArray[groupIndex].numCohorts, FunctGroupArray[groupIndex].numGeneTypes, FunctGroupArray[groupIndex].numStages, FunctGroupArray[groupIndex].numSpawns);
            
			/* See if this is the max number of cohorts so far */
			bm->K_num_max_cohort = max(bm->K_num_max_cohort, FunctGroupArray[groupIndex].numCohorts );
			bm->K_num_max_genetypes = max(bm->K_num_max_genetypes, FunctGroupArray[groupIndex].numGeneTypes );
			bm->K_num_max_stages = max(bm->K_num_max_stages, FunctGroupArray[groupIndex].numStages );
			bm->K_num_max_spawns = max(bm->K_num_max_spawns, FunctGroupArray[groupIndex].numSpawns );

			Util_XML_Get_Value_Integer(convertedXMLFileName, ATLANTIS_ATTRIBUTE, 0, TRUE, groupNode, binary_check, "VerticallyMobile",
					&FunctGroupArray[groupIndex].isMobile);
			Util_XML_Get_Value_Integer(convertedXMLFileName, ATLANTIS_ATTRIBUTE, 0, TRUE, groupNode, binary_check, "Mobile",
					&FunctGroupArray[groupIndex].sp_geo_move);
            Util_XML_Get_Value_Integer(convertedXMLFileName, ATLANTIS_ATTRIBUTE, 0, TRUE, groupNode, integer_check, "NumMigrations",
                    &FunctGroupArray[groupIndex].num_migrate);
            Util_XML_Get_Value_Integer(convertedXMLFileName, ATLANTIS_ATTRIBUTE, 0, TRUE, groupNode, binary_check, "MultiYrMigrations",
                    &FunctGroupArray[groupIndex].multiyr_mig);

            /* Check if any multi year migrations are occuring */
            if(FunctGroupArray[groupIndex].multiyr_mig > 0) {
                bm->flag_multiyr_migs = 1;
            }
            
            if((FunctGroupArray[groupIndex].num_migrate > 0) && !bm->flag_migration_on) {
                quit("Util_Read_Functional_Group_XML: NumMigrations > 0 for %s but you have flag_migration_on set to 0 in run.prm - please change something\n", FunctGroupArray[groupIndex].groupCode);
            }

            
            Util_XML_Get_Value_Integer(convertedXMLFileName, ATLANTIS_ATTRIBUTE, 0, TRUE, groupNode, binary_check, "ExternalReproduction", &FunctGroupArray[groupIndex].externalReproducer);
            Util_XML_Get_Value_Integer(convertedXMLFileName, ATLANTIS_ATTRIBUTE, 0, TRUE, groupNode, integer_check, "RecruitType", &FunctGroupArray[groupIndex].recruitType);
            
			Util_XML_Get_Value_Integer(convertedXMLFileName, ATLANTIS_ATTRIBUTE, 0, TRUE, groupNode, binary_check, "IsFished", &FunctGroupArray[groupIndex].isFished);
			Util_XML_Get_Value_Integer(convertedXMLFileName, ATLANTIS_ATTRIBUTE, 0, TRUE, groupNode, binary_check, "IsImpacted", &FunctGroupArray[groupIndex].isImpacted);
			Util_XML_Get_Value_Integer(convertedXMLFileName, ATLANTIS_ATTRIBUTE, 0, TRUE, groupNode, integer_check, "IsTAC", &FunctGroupArray[groupIndex].isTAC);
			Util_XML_Get_Value_String(convertedXMLFileName, ATLANTIS_ATTRIBUTE, TRUE, groupNode, "GroupType", varStr);

            /* Check fished vs impacted set up */
            if (FunctGroupArray[groupIndex].isFished && !FunctGroupArray[groupIndex].isImpacted) {
                quit("Util_Read_Functional_Group_XML: you have %s as fished but not impacted, set impacted to 1 for this group to continue please (i.e. if isFished 1 then isImpacted is 1)\n", FunctGroupArray[groupIndex].groupCode);
            }
            
            if (FunctGroupArray[groupIndex].isImpacted) {
                bm->K_max_impacted_sp = groupIndex;
            }

			/* Find the correct invert type */
			found = FALSE;
			for (j = 0; j < (int) (sizeof(groupTypes) / sizeof(GroupTypesStruct)); j++) {
				if (strcmp(varStr, groupTypes[j].str) == 0) {
					FunctGroupArray[groupIndex].groupType = groupTypes[j].group_type;
					found = TRUE;
					break;
				}
			}

			if (found == FALSE){
				quit("Util_Read_Functional_Group_XML: The 'GroupType' parameter value %s is not defined \n", varStr);
			}

            if (FunctGroupArray[groupIndex].groupType == FISH_INVERT) {
                FunctGroupArray[groupIndex].groupType = FISH;
                FunctGroupArray[groupIndex].isBioTurbator = TRUE;
            }

			/* If we haven't found any invert type then it must be a vertebrate */
			if (FunctGroupArray[groupIndex].groupType == BIRD || FunctGroupArray[groupIndex].groupType == FISH
				|| FunctGroupArray[groupIndex].groupType == MAMMAL || FunctGroupArray[groupIndex].groupType == SHARK)
				FunctGroupArray[groupIndex].isVertebrate = TRUE;
			else
				FunctGroupArray[groupIndex].isVertebrate = FALSE;

			if(FunctGroupArray[groupIndex].isMobile == FALSE && FunctGroupArray[groupIndex].isVertebrate == TRUE){
				fprintf(stderr, "\n\nYou have set VerticallyMoves to false in your groups definition file for a vertebrate group %s\n", FunctGroupArray[groupIndex].groupCode);
				quit("Vertebrate groups must vertically move at present. You can just set the VERTday_ and VERTnight_ values to the same as your initial conditions vertical distribution but you must provide these values.\n");
			}

            if (FunctGroupArray[groupIndex].isVertebrate == FALSE){
                if ((FunctGroupArray[groupIndex].numCohorts * FunctGroupArray[groupIndex].numGeneTypes) > bm->K_max_invert_cohorts)
                    bm->K_max_invert_cohorts = FunctGroupArray[groupIndex].numCohorts * FunctGroupArray[groupIndex].numGeneTypes;
            }


			// Check if coral present
			if((FunctGroupArray[groupIndex].groupType == CORAL) || (FunctGroupArray[groupIndex].groupType == SPONGE)){
				bm->containsCoral = 1;
			}

            if (FunctGroupArray[groupIndex].groupType == SPONGE) {
                bm->containsSponge = 1;
            }

            Util_XML_Get_Value_Integer(convertedXMLFileName, ATLANTIS_ATTRIBUTE, 0, TRUE, groupNode, binary_check, "IsPredator", &FunctGroupArray[groupIndex].isPredator);
			Util_XML_Get_Value_Integer(convertedXMLFileName, ATLANTIS_ATTRIBUTE, 0, TRUE, groupNode, binary_check, "IsCover", &FunctGroupArray[groupIndex].isCover);

			Util_XML_Get_Value_Integer(convertedXMLFileName, ATLANTIS_ATTRIBUTE, 0, TRUE, groupNode, binary_check, "IsSiliconDep", &FunctGroupArray[groupIndex].isSiliconDependant);
			FunctGroupArray[groupIndex].secondNutrientTracerIndex = -1;
			Util_XML_Get_Value_Integer(convertedXMLFileName, ATLANTIS_ATTRIBUTE, 0, TRUE, groupNode, binary_check, "IsAssessed", &FunctGroupArray[groupIndex].isAssessed);
			Util_XML_Get_Value_Double(convertedXMLFileName, ATLANTIS_ATTRIBUTE, 0, TRUE, groupNode, binary_check, "IsCatchGrazer", &FunctGroupArray[groupIndex].speciesParams[catcheater_id]);
            
            if(FunctGroupArray[groupIndex].isAssessed > 1) {
                FunctGroupArray[groupIndex].isCloseKin = 1;
            }

			Util_XML_Get_Value_Integer(convertedXMLFileName, ATLANTIS_ATTRIBUTE, 0, TRUE, groupNode, binary_check, "OverWinters", &FunctGroupArray[groupIndex].isOverWinter);
			Util_XML_Get_Value_Integer(convertedXMLFileName, ATLANTIS_ATTRIBUTE, 0, TRUE, groupNode, binary_check, "isCultured", &FunctGroupArray[groupIndex].isCultured);
            
			Util_XML_Get_Value_Integer(convertedXMLFileName, ATLANTIS_ATTRIBUTE, 0, TRUE, groupNode, binary_check, "isHabDepend", &FunctGroupArray[groupIndex].isHabDepend);
            Util_XML_Get_Value_Integer(convertedXMLFileName, ATLANTIS_ATTRIBUTE, 0, TRUE, groupNode, integer_check, "numMoveEntries", &FunctGroupArray[groupIndex].numMoveEntries);

            Util_XML_Get_Value_Integer(convertedXMLFileName, ATLANTIS_ATTRIBUTE, 0, TRUE, groupNode, integer_check, "isBioEroder", &FunctGroupArray[groupIndex].isBioEroder);

            Util_XML_Get_Value_Integer(convertedXMLFileName, ATLANTIS_ATTRIBUTE, 0, TRUE, groupNode, integer_check, "isSupplemented", &FunctGroupArray[groupIndex].isSupplemented);

            if(bm->external_populations) {
                Util_XML_Get_Value_Integer(convertedXMLFileName, ATLANTIS_ATTRIBUTE, 0, TRUE, groupNode, binary_check, "isExternal", &FunctGroupArray[groupIndex].isExternal);
            } else {
                FunctGroupArray[groupIndex].isExternal = 0;
            }

            if ( FunctGroupArray[groupIndex].isBioEroder ) {
                bm->sp_boring_sponges = groupIndex;
            }

            if(bm->terrestrial_on){
				Util_XML_Get_Value_Integer(convertedXMLFileName, ATLANTIS_ATTRIBUTE, 0, TRUE, groupNode, binary_check, "isLandActive", &FunctGroupArray[groupIndex].isLandActive);
			} else {
				FunctGroupArray[groupIndex].isLandActive = 0;
			}

            FunctGroupArray[groupIndex].isLightEffected = 0;
            FunctGroupArray[groupIndex].isNoiseEffected = 0;
            if(bm->flag_pollutant_impacts){
                Util_XML_Get_Value_Integer(convertedXMLFileName, ATLANTIS_ATTRIBUTE, 0, TRUE, groupNode, binary_check, "isLightEffected", &FunctGroupArray[groupIndex].isLightEffected);
                Util_XML_Get_Value_Integer(convertedXMLFileName, ATLANTIS_ATTRIBUTE, 0, TRUE, groupNode, binary_check, "isNoiseEffected", &FunctGroupArray[groupIndex].isNoiseEffected);
            }
            
            if (FunctGroupArray[groupIndex].numMoveEntries <= 0 && FunctGroupArray[groupIndex].sp_geo_move == TRUE){
            	quit("ERROR: Based on your functional group definition file you have turned on horizontal movement for group %s, but you have set the number of entries different movement periods to 0. \n", FunctGroupArray[groupIndex].groupCode);
            }
            if (FunctGroupArray[groupIndex].numMoveEntries > 0 && FunctGroupArray[groupIndex].sp_geo_move == FALSE){
            	warn("WARNING: Based on your functional group definition file you have turned off horizontal movement for group %s, but you have set the number of entries different movement periods to greater than 0. \n. No seasonal movement will occur for this group.\n", FunctGroupArray[groupIndex].groupCode);
            }

            
            FunctGroupArray[groupIndex].RAssessFileNum = -1; // So initialised as off and only turned on if actually needed

			// Check the lines
			/* Is producer */
			FunctGroupArray[groupIndex].isProducer = FALSE;
			if (FunctGroupArray[groupIndex].isVertebrate == FALSE) {

				if (FunctGroupArray[groupIndex].groupType != LAB_DET && FunctGroupArray[groupIndex].groupType != REF_DET
						&& FunctGroupArray[groupIndex].groupType != CARRION)
					FunctGroupArray[groupIndex].isProducer = TRUE;

			}

			if(FunctGroupArray[groupIndex].groupType == REF_DET){
				if(RefDetIndex != -1)
					quit("ERROR: There can only be a single refractory retritus (REF_DET) group\n");
				RefDetIndex = groupIndex;
			}else if(FunctGroupArray[groupIndex].groupType == LAB_DET){
				if(LabDetIndex != -1)
					quit("ERROR: There can only be a single labile detritus (LAB_DET) group\n");
				LabDetIndex = groupIndex;
			}else if(FunctGroupArray[groupIndex].groupType == SED_BACT){
				if(SedBactIndex != -1)
					quit("ERROR: There can only be a single sediment bacteria (SED_BACT) group\n");
				SedBactIndex = groupIndex;
			}else if(FunctGroupArray[groupIndex].groupType == PL_BACT){
				if(pelagicBactIndex != -1)
					quit("ERROR: There can only be a single pelagic bacteria (PL_BACT) group\n");
				pelagicBactIndex = groupIndex;
			}else if(FunctGroupArray[groupIndex].groupType == CARRION){
				if(CarrionIndex != -1)
					quit("ERROR: There can only be a single carrion (CARRION) group\n");
				CarrionIndex = groupIndex;
			}else if(FunctGroupArray[groupIndex].groupType == ICE_BACT){
				if(IceBactIndex != -1)
					quit("ERROR: There can only be a single ice bacteria (ICE_BACT) group\n");
				IceBactIndex = groupIndex;
			}

		}
		xmlXPathFreeObject(groupList);
	}

	xmlFreeDoc(inputDoc);
	/* Shutdown libxml */
	xmlCleanupParser();

	/* Set the habitats of the groups based on the group type */
	Set_Group_Habitats(bm);


	/* now check that the detritus and bacteria groups have been found */
	if (SedBactIndex == -1){
		shouldQuit = TRUE;
		warn("ERROR: There must be one sediment bacteria group in your functional group input file.");
	}
	if (pelagicBactIndex == -1){
		shouldQuit = TRUE;
		warn("ERROR: There must be one pelagic bacteria group in your functional group input file.");
	}
	if (LabDetIndex == -1){
		shouldQuit = TRUE;
		warn("ERROR: There must be one Labile detritus group in your functional group input file.");
	}
	if (RefDetIndex == -1){
		shouldQuit = TRUE;
		warn("ERROR: There must be one Refractory detritus group in your functional group input file.");
	}
	if (CarrionIndex == -1){
		shouldQuit = TRUE;
		warn("ERROR: There must be one Carrion group in your functional group input file.");
	}

	/* if we should quit then do it now once all groups that are missing have been displayed. Means modellers will do a batch of changes at once rather than add one group at a time */
	if (shouldQuit == TRUE)
		quit("");

	// While we're here we need to intiialise the aquaculture feed slot, just in case its needed
	AquacultFeedIndex = bm->K_num_tot_sp;

	bm->containsMammals = FALSE;
	bm->containsBirds = FALSE;
	bm->containsTurf = FALSE;
	bm->containsMCPYr = FALSE;  //whether there is a case of multiple cohorts within a single year or not

	/* Work out the groupAgeType based on number of cohorts */
	bm->flagWCVert = 1;
	for (guild = 0; guild < bm->K_num_tot_sp; guild++) {
		if (FunctGroupArray[guild].isVertebrate == TRUE){
			FunctGroupArray[guild].groupAgeType = AGE_STRUCTURED;

			if(FunctGroupArray[guild].groupType == MAMMAL){
				bm->containsMammals = TRUE;
			}

			if(FunctGroupArray[guild].groupType == BIRD){
				bm->containsBirds = TRUE;
			}

			if(FunctGroupArray[guild].groupType == TURF){
				bm->containsTurf = TRUE;
			}

			if(FunctGroupArray[guild].habitatType != WC)
				bm->flagWCVert = 0;

		} else {

			/* Check for CREAM model */
//			if(FunctGroupArray[guild].groupType == CEP || FunctGroupArray[guild].groupType == PWN){
//				FunctGroupArray[guild].groupAgeType = AGE_STRUCTURED_BIOMASS;
//			}else{
				if (FunctGroupArray[guild].numCohorts > 1 ){

					if(FunctGroupArray[guild].numCohorts > 2 && FunctGroupArray[guild].groupType != SEAGRASS){
						warn("WARNING: Invertebrates have typically only had 2 cohorts - adult and juv. If things look odd this may be why (you have %d cohorts for %s)\n", FunctGroupArray[guild].numCohorts, FunctGroupArray[guild].groupCode);
					}
					if(FunctGroupArray[guild].groupType == SEAGRASS){
						FunctGroupArray[guild].groupAgeType = BIOMASS;
					}else{
						FunctGroupArray[guild].groupAgeType = AGE_STRUCTURED_BIOMASS;
						bm->containsMCPYr = TRUE;
					}
				} else {
					FunctGroupArray[guild].groupAgeType = BIOMASS;
				}
			//}
		}
	}

	/* Set the additional flags */
	for (guild = 0; guild < bm->K_num_tot_sp; guild++) {
		FunctGroupArray[guild].isInfauna = FALSE;
		FunctGroupArray[guild].isEpiFauna = FALSE;
		FunctGroupArray[guild].isPrimaryProducer = FALSE;
		FunctGroupArray[guild].isBacteria = FALSE;
		FunctGroupArray[guild].isMicroFauna = FALSE;
		FunctGroupArray[guild].isZooPlankton = FALSE;
		FunctGroupArray[guild].updatedDiet = 0;

		if (FunctGroupArray[guild].isVertebrate == FALSE) {

			/* Cover groups must be in the epibethic layer */
			if(FunctGroupArray[guild].habitatCoeffs[EPIFAUNA] == FALSE && FunctGroupArray[guild].isCover == TRUE){
				quit("ERROR: Cover groups must be active in the epibenthic layer. You have specified %s as a Cover group (isCover) but its not found in the epibenthic layer", FunctGroupArray[guild].groupCode);
			}

			if (FunctGroupArray[guild].groupType == SM_INF || FunctGroupArray[guild].groupType == LG_INF) {
				FunctGroupArray[guild].isInfauna = TRUE;
			}

            if (FunctGroupArray[guild].groupType == SED_EP_FF ||
                FunctGroupArray[guild].groupType == SED_EP_OTHER ||
                FunctGroupArray[guild].groupType == MOB_EP_OTHER ||
                FunctGroupArray[guild].groupType == PHYTOBEN ||
                FunctGroupArray[guild].groupType == MICROPHTYBENTHOS ||
                FunctGroupArray[guild].groupType == SEAGRASS ||
                FunctGroupArray[guild].groupType == TURF ||
                FunctGroupArray[guild].groupType == CORAL ||
                FunctGroupArray[guild].groupType == SPONGE) {
                FunctGroupArray[guild].isEpiFauna = TRUE;
            }

            if (FunctGroupArray[guild].groupType == SED_EP_FF ||
                FunctGroupArray[guild].groupType == SED_EP_OTHER ||
                FunctGroupArray[guild].groupType == MOB_EP_OTHER) {
                FunctGroupArray[guild].isBioTurbator = TRUE;
            }


			/* Set the primary producer flag */
			if (FunctGroupArray[guild].groupType == SM_PHY ||
					FunctGroupArray[guild].groupType == LG_PHY ||
					FunctGroupArray[guild].groupType == PHYTOBEN ||
					FunctGroupArray[guild].groupType == MICROPHTYBENTHOS ||
					FunctGroupArray[guild].groupType == DINOFLAG ||
					FunctGroupArray[guild].groupType == SEAGRASS ||
					FunctGroupArray[guild].groupType == TURF ||
					FunctGroupArray[guild].groupType == CORAL ||
					FunctGroupArray[guild].groupType == ICE_DIATOMS ||
					FunctGroupArray[guild].groupType == ICE_MIXOTROPHS ) {
				FunctGroupArray[guild].isPrimaryProducer = TRUE;
			}

			if (FunctGroupArray[guild].groupType == PL_BACT || FunctGroupArray[guild].groupType == SED_BACT
					|| FunctGroupArray[guild].groupType == ICE_BACT) {
				FunctGroupArray[guild].isBacteria = TRUE;
			}
			/* Set up the microfauna flags */
			if (FunctGroupArray[guild].groupType == SM_PHY ||
					FunctGroupArray[guild].groupType == LG_PHY ||
					FunctGroupArray[guild].groupType == DINOFLAG ||
					FunctGroupArray[guild].groupType == LG_ZOO ||
					FunctGroupArray[guild].groupType == MED_ZOO ||
					FunctGroupArray[guild].groupType == SM_ZOO ||
					FunctGroupArray[guild].groupType == SM_INF ||
					FunctGroupArray[guild].groupType == ICE_DIATOMS ||
					FunctGroupArray[guild].groupType == ICE_MIXOTROPHS
) {
				FunctGroupArray[guild].isMicroFauna = TRUE;
			}
			/* Set up the zooplankton flag */
			if (FunctGroupArray[guild].groupType == LG_ZOO ||
					FunctGroupArray[guild].groupType == MED_ZOO ||
					FunctGroupArray[guild].groupType == SM_ZOO) {
				FunctGroupArray[guild].isZooPlankton = TRUE;
			}
			/* Add in a check to make sure that Dinaflag groups are also predators. Otherwise spit a warning */
			if(FunctGroupArray[guild].groupType == DINOFLAG && FunctGroupArray[guild].isPredator == FALSE){
				warn("You have created a DINOFLAG group %s that isn't a predator. The Assimilation efficiency for this group will not be read in\n", FunctGroupArray[guild].groupCode);
			}
		}

		if(FunctGroupArray[guild].isBacteria == TRUE || FunctGroupArray[guild].isPredator == TRUE )
			FunctGroupArray[guild].isGrazer = TRUE;
		else
			FunctGroupArray[guild].isGrazer = FALSE;



		if(FunctGroupArray[guild].isVertebrate == TRUE){
			FunctGroupArray[guild].diagTol = 2;
		} else {
			if (FunctGroupArray[guild].groupType == CARRION)
				FunctGroupArray[guild].diagTol = 3;
			else if (FunctGroupArray[guild].groupType == SED_EP_OTHER ||
					FunctGroupArray[guild].groupType == MOB_EP_OTHER	||
					FunctGroupArray[guild].groupType == PWN ||
					FunctGroupArray[guild].groupType == CEP ||
					FunctGroupArray[guild].groupType == LG_ZOO ||
					FunctGroupArray[guild].groupType == LG_INF)
				FunctGroupArray[guild].diagTol = 2;
			else {
				FunctGroupArray[guild].diagTol = 1;  // So all phytoplankton, small zooplankton and filter feeders set the adaptive time step
				if(FunctGroupArray[guild].isCultured) // Except if an aquaculture species then just have it inflicted upon them
					FunctGroupArray[guild].diagTol = 0;
			}
		}

		FunctGroupArray[guild].isOncePerDt = TRUE;
		if (FunctGroupArray[guild].diagTol < 2)
			FunctGroupArray[guild].isOncePerDt = FALSE;

	}

	for (guild = 0; guild < bm->K_num_tot_sp; guild++) {
		FunctGroupArray[guild].isMacroFauna = FALSE;
		if (FunctGroupArray[guild].isVertebrate == TRUE || FunctGroupArray[guild].groupType == LG_INF || FunctGroupArray[guild].groupType == CEP
				|| FunctGroupArray[guild].groupType == PWN || FunctGroupArray[guild].groupType == SED_EP_FF || FunctGroupArray[guild].groupType
				== SED_EP_OTHER || FunctGroupArray[guild].groupType == MOB_EP_OTHER) {
			FunctGroupArray[guild].isMacroFauna = TRUE;
		}
	}

	/* Set the isDetritus flag */
	for (guild = 0; guild < bm->K_num_tot_sp; guild++) {
		FunctGroupArray[guild].isDetritus = FALSE;
		if (FunctGroupArray[guild].groupType == LAB_DET || FunctGroupArray[guild].groupType == REF_DET || FunctGroupArray[guild].groupType == CARRION) {
			FunctGroupArray[guild].isDetritus = TRUE;
		}
	}

	/* Check all the counts in the paramIO file are correct */
	count = 0;
	for (guild = 0; guild < bm->K_num_tot_sp; guild++) {
		if(FunctGroupArray[guild].isDetritus)
			count++;
	}
	if(count != bm->K_num_detritus){
		quit(
				"Util_Read_Functional_Group_XML: Number of detritus groups specified (%d) in your group definition file %s does not match the K_num_detritus value (%d) in your run file\n",
				count, fileName, bm->K_num_detritus);
	}

	detIndex = 0;
	/* Work out the index of the detritus groups */
	for (guild = 0; guild < bm->K_num_tot_sp; guild++) {
		if(FunctGroupArray[guild].isDetritus){
			FunctGroupArray[guild].detIndex = detIndex;
			detIndex++;
		}
	}


	// Allocate the tracer array memory for each functional group.
	//printf("Creating functional group arrays on read-in\n");

	for (i = 0; i < bm->K_num_tot_sp; i++) {

		/* Set to Null now so we can work out if its allocated later */
		FunctGroupArray[i].co_sp = NULL;
        FunctGroupArray[i].co_sp_catch = NULL;

		if (FunctGroupArray[i].isVertebrate == TRUE) {
			FunctGroupArray[i].structNTracers = Util_Alloc_Init_1D_Int(FunctGroupArray[i].numCohortsXnumGenes, -1);
			FunctGroupArray[i].resNTracers = Util_Alloc_Init_1D_Int(FunctGroupArray[i].numCohortsXnumGenes, -1);
			FunctGroupArray[i].NumsTracers = Util_Alloc_Init_1D_Int(FunctGroupArray[i].numCohortsXnumGenes, -1);
			FunctGroupArray[i].GrowthTracers = Util_Alloc_Init_1D_Int(FunctGroupArray[i].numCohortsXnumGenes, -1);
			FunctGroupArray[i].EatTracers = Util_Alloc_Init_1D_Int(FunctGroupArray[i].numCohortsXnumGenes, -1);
			FunctGroupArray[i].CatchTracers = Util_Alloc_Init_1D_Int(FunctGroupArray[i].numCohortsXnumGenes, -1);
			FunctGroupArray[i].DiscardTracers = Util_Alloc_Init_1D_Int(FunctGroupArray[i].numCohortsXnumGenes, -1);
			FunctGroupArray[i].totNTracers = Util_Alloc_Init_1D_Int(FunctGroupArray[i].numCohortsXnumGenes, -1);
			FunctGroupArray[i].grow = Util_Alloc_Init_2D_Double((RN_id + 1), FunctGroupArray[i].numCohortsXnumGenes, -1);
			FunctGroupArray[i].allgone = Util_Alloc_Init_1D_Int(FunctGroupArray[i].numCohortsXnumGenes, 0);

		} else {
			FunctGroupArray[i].totNTracers = Util_Alloc_Init_1D_Int(FunctGroupArray[i].numCohortsXnumGenes, -1);
			FunctGroupArray[i].prodnTracers = Util_Alloc_Init_1D_Int(FunctGroupArray[i].numCohortsXnumGenes, -1);
			FunctGroupArray[i].GrazingTracers = Util_Alloc_Init_1D_Int(FunctGroupArray[i].numCohortsXnumGenes, -1);
			FunctGroupArray[i].SP_IRR = 0.0;
			FunctGroupArray[i].Light_Adaptn_TracerID = -1;

			/* Initalise the values in case functional groups are not active */
			FunctGroupArray[i].uptakeDL = 0.0;
			FunctGroupArray[i].uptakeDR = 0.0;
			FunctGroupArray[i].prodnDON = 0.0;

			FunctGroupArray[i].releaseNH = Util_Alloc_Init_1D_Double(FunctGroupArray[i].numCohortsXnumGenes, 0.0);
			FunctGroupArray[i].uptakeNH = Util_Alloc_Init_1D_Double(FunctGroupArray[i].numCohortsXnumGenes, 0.0);
			FunctGroupArray[i].prodnDR = Util_Alloc_Init_1D_Double(FunctGroupArray[i].numCohortsXnumGenes, 0.0);
			FunctGroupArray[i].prodnDL = Util_Alloc_Init_1D_Double(FunctGroupArray[i].numCohortsXnumGenes, 0.0);
			FunctGroupArray[i].growth = Util_Alloc_Init_1D_Double(FunctGroupArray[i].numCohortsXnumGenes, 0.0);
			FunctGroupArray[i].mortality = Util_Alloc_Init_1D_Double(FunctGroupArray[i].numCohortsXnumGenes, 0.0);
			FunctGroupArray[i].grazing = Util_Alloc_Init_1D_Double(FunctGroupArray[i].numCohortsXnumGenes, 0.0);
			FunctGroupArray[i].lysis = Util_Alloc_Init_1D_Double(FunctGroupArray[i].numCohortsXnumGenes, 0.0);
			FunctGroupArray[i].sn = Util_Alloc_Init_1D_Double(FunctGroupArray[i].numCohortsXnumGenes, 0.0);
			FunctGroupArray[i].transDR = Util_Alloc_Init_1D_Double(FunctGroupArray[i].numCohortsXnumGenes, 0.0);
		}

       if (FunctGroupArray[i].isMobile) {
			FunctGroupArray[i].distrib_VERTICAL = Util_Alloc_Init_3D_Double(2, bm->wcnz, 2, 0.0);
		}
		if(FunctGroupArray[i].isOverWinter) {
			FunctGroupArray[i].distrib_OVERWINTER = Util_Alloc_Init_2D_Double(2, bm->wcnz, 0.0);
			bm->flagOverwinter = 1;
		}

		if(FunctGroupArray[i].isCultured)
			bm->flag_aquacult_on = 1;

        if(FunctGroupArray[i].isSupplemented)
            bm->flag_import_feed = 1;

		/* Allocate memory to store amount of this  group caught and discarded by each fishery */
		FunctGroupArray[i].CaughtByFisheryTracers = Util_Alloc_Init_1D_Int(bm->K_num_fisheries, 0);
		FunctGroupArray[i].DiscardedByFisheryTracers = Util_Alloc_Init_1D_Int(bm->K_num_fisheries, 0);

		FunctGroupArray[i].age_mat = Util_Alloc_Init_1D_Double(FunctGroupArray[i].numStages, 0.0);
		FunctGroupArray[i].cohort_stage = Util_Alloc_Init_1D_Int(FunctGroupArray[i].numCohortsXnumGenes, 0);

		FunctGroupArray[i].C_T15 = Util_Alloc_Init_1D_Double(FunctGroupArray[i].numCohortsXnumGenes, 0.0);
		FunctGroupArray[i].C_T15_per_day = Util_Alloc_Init_1D_Double(FunctGroupArray[i].numCohortsXnumGenes, 0.0);
		FunctGroupArray[i].SP_C = Util_Alloc_Init_1D_Double(FunctGroupArray[i].numCohortsXnumGenes, 0.0);
		FunctGroupArray[i].SP_C_per_day = Util_Alloc_Init_1D_Double(FunctGroupArray[i].numCohortsXnumGenes, 0.0);
		FunctGroupArray[i].scaled_C = Util_Alloc_Init_1D_Double(FunctGroupArray[i].numCohortsXnumGenes, 0.0);
		FunctGroupArray[i].mum_T15 = Util_Alloc_Init_1D_Double(FunctGroupArray[i].numCohortsXnumGenes, 0.0);
		FunctGroupArray[i].mum_T15_per_day = Util_Alloc_Init_1D_Double(FunctGroupArray[i].numCohortsXnumGenes, 0.0);
		FunctGroupArray[i].mum = Util_Alloc_Init_1D_Double(FunctGroupArray[i].numCohortsXnumGenes, 0.0);
		FunctGroupArray[i].scaled_mum = Util_Alloc_Init_1D_Double(FunctGroupArray[i].numCohortsXnumGenes, 0.0);
		FunctGroupArray[i].mum_per_day = Util_Alloc_Init_1D_Double(FunctGroupArray[i].numCohortsXnumGenes, 0.0);
		FunctGroupArray[i].CLEAR = Util_Alloc_Init_1D_Double(FunctGroupArray[i].numCohortsXnumGenes, 0.0);

        FunctGroupArray[i].needed_for_age_away = Util_Alloc_Init_1D_Int(FunctGroupArray[i].numCohortsXnumGenes, 0.0);

        if(bm->track_contaminants) {
          FunctGroupArray[i].C_growth_corr = Util_Alloc_Init_2D_Double(bm->K_num_tot_sp, FunctGroupArray[i].numCohortsXnumGenes, 0.0);
          FunctGroupArray[i].C_reprod_corr = Util_Alloc_Init_2D_Double(bm->K_num_tot_sp, FunctGroupArray[i].numCohortsXnumGenes, 0.0);
          FunctGroupArray[i].C_move_corr = Util_Alloc_Init_1D_Double(FunctGroupArray[i].numCohortsXnumGenes, 0.0);
        }

        //FunctGroupArray[i].GrazeLive = Util_Alloc_Init_1D_Double(FunctGroupArray[i].numCohortsXnumGenes, 0.0);
        FunctGroupArray[i].GrazeLive = Util_Alloc_Init_1D_Long_Double(FunctGroupArray[i].numCohortsXnumGenes, 0.0);

		FunctGroupArray[i].dead = Util_Alloc_Init_1D_Double(FunctGroupArray[i].numCohortsXnumGenes, 0.0);
		FunctGroupArray[i].deadGlobal = Util_Alloc_Init_1D_Double(FunctGroupArray[i].numCohortsXnumGenes, 0.0);
		FunctGroupArray[i].preyEaten = Util_Alloc_Init_2D_Long_Double(bm->num_active_habitats, FunctGroupArray[i].numCohortsXnumGenes, 0.0);
		FunctGroupArray[i].preyEatenGlobal = Util_Alloc_Init_3D_Long_Double(bm->num_active_habitats, bm->num_active_habitats, FunctGroupArray[i].numCohortsXnumGenes, 0.0);

        FunctGroupArray[i].max_scalar = Util_Alloc_Init_2D_Double(bm->K_num_tot_sp, FunctGroupArray[i].numCohortsXnumGenes, 0.0);

        FunctGroupArray[i].NeedMoveUpdate = Util_Alloc_Init_1D_Int(FunctGroupArray[i].numStages, 0);

		switch (FunctGroupArray[i].groupAgeType) {
		case AGE_STRUCTURED:
			FunctGroupArray[i].cohortSpeciesParams = Util_Alloc_Init_2D_Double(cohortDepParams, FunctGroupArray[i].numStages, 0.0);
			FunctGroupArray[i].spawnSpeciesParams = Util_Alloc_Init_2D_Double(spawnDepParams, FunctGroupArray[i].numSpawns, 0.0);
			FunctGroupArray[i].pSPEat = Util_Alloc_Init_3D_Double(bm->num_active_habitats, bm->K_num_tot_sp, FunctGroupArray[i].numStages, 0.0);
			FunctGroupArray[i].FSPB = Util_Alloc_Init_1D_Double(FunctGroupArray[i].numCohortsXnumGenes, 0.0);

            //TODO: Should really only allocate this is we are scaling - not really used yet?
			FunctGroupArray[i].scaled_FSPB = Util_Alloc_Init_1D_Double(FunctGroupArray[i].numCohortsXnumGenes, 0.0);
			FunctGroupArray[i].AGEnewden = Util_Alloc_Init_3D_Double(ncells, nlayer, FunctGroupArray[i].numCohortsXnumGenes, 0.0);
			FunctGroupArray[i].agingVERT = Util_Alloc_Init_3D_Double(3, nstock, FunctGroupArray[i].numCohortsXnumGenes, 0.0);
			FunctGroupArray[i].SizeNumDiscard = Util_Alloc_Init_3D_Double(bm->nbox, bm->K_num_fisheries, FunctGroupArray[i].numCohortsXnumGenes, 0.0);
			FunctGroupArray[i].SizeNumCaught = Util_Alloc_Init_3D_Double(bm->nbox, bm->K_num_fisheries, FunctGroupArray[i].numCohortsXnumGenes, 0.0);
            FunctGroupArray[i].SizeDiscard = Util_Alloc_Init_3D_Double(bm->nbox, bm->K_num_fisheries, FunctGroupArray[i].numCohortsXnumGenes, 0.0);
            FunctGroupArray[i].SizeCaught = Util_Alloc_Init_3D_Double(bm->nbox, bm->K_num_fisheries, FunctGroupArray[i].numCohortsXnumGenes, 0.0);
            FunctGroupArray[i].X_RS = Util_Alloc_Init_1D_Double(FunctGroupArray[i].numCohortsXnumGenes, 0.0);
            FunctGroupArray[i].RAssessSpringSurvey = Util_Alloc_Init_2D_Double(bm->nbox, FunctGroupArray[i].numCohortsXnumGenes, 0.0);
            FunctGroupArray[i].RAssessAutumnSurvey = Util_Alloc_Init_2D_Double(bm->nbox, FunctGroupArray[i].numCohortsXnumGenes, 0.0);
                
            FunctGroupArray[i].min_wgt = Util_Alloc_Init_1D_Double(FunctGroupArray[i].numCohortsXnumGenes, 0.0);
            FunctGroupArray[i].max_wgt = Util_Alloc_Init_1D_Double(FunctGroupArray[i].numCohortsXnumGenes, 0.0);
            FunctGroupArray[i].rolling_wgt = Util_Alloc_Init_2D_Double(bm->K_rolling_cap_num + 1, FunctGroupArray[i].numCohortsXnumGenes, 0.0);
            FunctGroupArray[i].min_B = Util_Alloc_Init_1D_Double(FunctGroupArray[i].numCohortsXnumGenes, 0.0);
            FunctGroupArray[i].max_B = Util_Alloc_Init_1D_Double(FunctGroupArray[i].numCohortsXnumGenes, 0.0);
            FunctGroupArray[i].rolling_B = Util_Alloc_Init_2D_Double(bm->K_rolling_cap_num + 1, FunctGroupArray[i].numCohortsXnumGenes, 0.0);
			break;
		case AGE_STRUCTURED_BIOMASS:
			FunctGroupArray[i].cohortSpeciesParams = Util_Alloc_Init_2D_Double(cohortDepParams, FunctGroupArray[i].numStages, 0.0);
			FunctGroupArray[i].spawnSpeciesParams = Util_Alloc_Init_2D_Double(spawnDepParams, FunctGroupArray[i].numSpawns, 0.0);
			FunctGroupArray[i].pSPEat = Util_Alloc_Init_3D_Double(bm->num_active_habitats, bm->K_num_tot_sp, FunctGroupArray[i].numStages, 0.0);

            // Needed for generalised demography call
			FunctGroupArray[i].FSPB = Util_Alloc_Init_1D_Double(FunctGroupArray[i].numCohortsXnumGenes, 0.0);
			FunctGroupArray[i].scaled_FSPB = Util_Alloc_Init_1D_Double(FunctGroupArray[i].numCohortsXnumGenes, 1.0);
			FunctGroupArray[i].AGEnewden = Util_Alloc_Init_3D_Double(ncells, nlayer, FunctGroupArray[i].numCohortsXnumGenes, 0.0);
			FunctGroupArray[i].agingVERT = Util_Alloc_Init_3D_Double(3, nstock, FunctGroupArray[i].numCohortsXnumGenes, 0.0);

			/* Initialise invertebrate cohort sizes */
			FunctGroupArray[i].INVpopratio = Util_Alloc_Init_2D_Double(FunctGroupArray[i].numCohorts, FunctGroupArray[i].numCohortsXnumGenes, 1.0/((double)FunctGroupArray[i].numCohorts));
			FunctGroupArray[i].tempINVpopratio = Util_Alloc_Init_2D_Double(invert_reprod_prm, FunctGroupArray[i].numCohortsXnumGenes, 0.0);
            FunctGroupArray[i].X_RS = Util_Alloc_Init_1D_Double(FunctGroupArray[i].numCohortsXnumGenes, 0.0);

            FunctGroupArray[i].min_B = Util_Alloc_Init_1D_Double(FunctGroupArray[i].numCohortsXnumGenes, 0.0);
            FunctGroupArray[i].max_B = Util_Alloc_Init_1D_Double(FunctGroupArray[i].numCohortsXnumGenes, 0.0);
            FunctGroupArray[i].rolling_B = Util_Alloc_Init_2D_Double(bm->K_rolling_cap_num + 1, FunctGroupArray[i].numCohortsXnumGenes, 0.0);
                
			break;
		case BIOMASS:
			/* Allow a slot per cohort for seagrass. Most groups this will be cohort == 1*/
			FunctGroupArray[i].cohortSpeciesParams = Util_Alloc_Init_2D_Double(cohortDepParams, FunctGroupArray[i].numStages, 0.0);
            if (FunctGroupArray[i].isCultured)
                FunctGroupArray[i].spawnSpeciesParams = Util_Alloc_Init_2D_Double(spawnDepParams, FunctGroupArray[i].numSpawns, 0.0);
            FunctGroupArray[i].pSPEat = Util_Alloc_Init_3D_Double(bm->num_active_habitats, bm->K_num_tot_sp, FunctGroupArray[i].numStages, 0.0);
            FunctGroupArray[i].X_RS = Util_Alloc_Init_1D_Double(FunctGroupArray[i].numCohortsXnumGenes, 0.0);
                
            FunctGroupArray[i].min_B = Util_Alloc_Init_1D_Double(FunctGroupArray[i].numCohortsXnumGenes, 0.0);
            FunctGroupArray[i].max_B = Util_Alloc_Init_1D_Double(FunctGroupArray[i].numCohortsXnumGenes, 0.0);
            FunctGroupArray[i].rolling_B = Util_Alloc_Init_2D_Double(bm->K_rolling_cap_num + 1, FunctGroupArray[i].numCohortsXnumGenes, 0.0);

			break;
		}
		/* Allocate memory for additional tracers */
		if(bm->track_atomic_ratio == TRUE){
			FunctGroupArray[i].addRatioTracers = Util_Alloc_Init_2D_Int(num_atomic_id, FunctGroupArray[i].numCohortsXnumGenes, 0);
			FunctGroupArray[i].addRatioFluxes = Util_Alloc_Init_2D_Double(num_atomic_id + 1, FunctGroupArray[i].numCohortsXnumGenes, 0.0);

			FunctGroupArray[i].ratioLost = Util_Alloc_Init_3D_Long_Double(num_atomic_id, FunctGroupArray[i].numCohortsXnumGenes, 3, 0.0);
			FunctGroupArray[i].ratioLostGlobal = Util_Alloc_Init_4D_Long_Double(num_atomic_id, FunctGroupArray[i].numCohortsXnumGenes, 3, 3, 0.0);
			FunctGroupArray[i].ratioGainedPred = Util_Alloc_Init_2D_Long_Double(num_atomic_id, FunctGroupArray[i].numCohortsXnumGenes, 0.0);
			//FunctGroupArray[i].ratioGainedPredGlobal = Util_Alloc_Init_4D_Long_Double(num_atomic_id, FunctGroupArray[i].numCohorts, 3, 3, 0.0);

			FunctGroupArray[i].addRatioLost = Util_Alloc_Init_2D_Double(num_atomic_id, FunctGroupArray[i].numCohortsXnumGenes, 0);

			FunctGroupArray[i].ratioLostPred = Util_Alloc_Init_3D_Long_Double(num_atomic_id, FunctGroupArray[i].numCohorts, 3, 0.0);
			FunctGroupArray[i].ratioLostPredGlobal = Util_Alloc_Init_4D_Long_Double(num_atomic_id, FunctGroupArray[i].numCohorts, 3, 3, 0.0);

			FunctGroupArray[i].ratioGained = Util_Alloc_Init_3D_Long_Double(num_atomic_id, FunctGroupArray[i].numCohorts, 3, 0.0);
			FunctGroupArray[i].ratioGainedGlobal = Util_Alloc_Init_4D_Long_Double(num_atomic_id, FunctGroupArray[i].numCohorts, 3, 3, 0.0);

		}

		if(bm->track_contaminants){
			FunctGroupArray[i].contaminantTracers = Util_Alloc_Init_2D_Int(bm->num_contaminants, FunctGroupArray[i].numCohorts, 0);
            FunctGroupArray[i].contamPropTracers = Util_Alloc_Init_2D_Int(bm->num_contaminants, FunctGroupArray[i].numCohorts, 0);
			FunctGroupArray[i].contaminantSpMort = Util_Alloc_Init_1D_Double(FunctGroupArray[i].numCohorts, 0.0);
			FunctGroupArray[i].calcCLinearMort = Util_Alloc_Init_2D_Double(3, FunctGroupArray[i].numCohortsXnumGenes, 0.0);
            FunctGroupArray[i].agingContam = Util_Alloc_Init_4D_Double((bm->wcnz+bm->sednz), bm->nbox, bm->num_contaminants, FunctGroupArray[i].numCohorts, 0);
            
            if(bm->track_contaminants){
                FunctGroupArray[i].reprodContam = Util_Alloc_Init_1D_Double(bm->num_contaminants, 0);
                FunctGroupArray[i].reprodContamCount = Util_Alloc_Init_1D_Double(bm->num_contaminants, 0);
                FunctGroupArray[i].LocalPopCount = Util_Alloc_Init_1D_Double(FunctGroupArray[i].numCohortsXnumGenes, 0);
            }
		}

		if (FunctGroupArray[i].numMoveEntries > 0)
			FunctGroupArray[i].distrib = Util_Alloc_Init_3D_Double(2, FunctGroupArray[i].numMoveEntries, bm->nbox, 0.0);
        else
            FunctGroupArray[i].distrib = Util_Alloc_Init_3D_Double(2, 1, bm->nbox, 0.0); // To allow for constraint of epibenthos to initial distributions
	}

	/* Add the DIN groupCode */
	strcpy(FunctGroupArray[bm->K_num_tot_sp].groupCode, "DIN");


/* Ignore this code - left in to aid testing of old models */
#ifdef ORIGINAL_GROUP_ORDER

	groupEatOrder = Util_Alloc_Init_1D_Int(bm->K_num_tot_sp, -1);
	groupMovementOrder = Util_Alloc_Init_1D_Int(bm->K_num_tot_sp, -1);
	TLorder2 = Util_Alloc_Init_1D_Int(bm->K_num_tot_sp, -1);
	TLorder3 = Util_Alloc_Init_1D_Int(bm->K_num_tot_sp, -1);
	wcOrder = Util_Alloc_Init_1D_Int(bm->K_num_tot_sp, -1);
	sedOrder = Util_Alloc_Init_1D_Int(bm->K_num_tot_sp, -1);
	epiOrder = Util_Alloc_Init_1D_Int(bm->K_num_tot_sp, -1);


	/* count the number of vert species */
	for(i = 0; i < bm->K_num_tot_sp; i++){
		if(FunctGroupArray[i].isVertebrate == TRUE)
			vertCount++;
	}
	eatIndex = vertCount;
	groupMovementOrder[eatIndex++] = PLguild_id;
	groupMovementOrder[eatIndex++] = ZGguild_id;
	groupMovementOrder[eatIndex++] = ZLguild_id;
	groupMovementOrder[eatIndex++] = ZMguild_id;
	groupMovementOrder[eatIndex++] = ZSguild_id;
	groupMovementOrder[eatIndex++] = CEPguild_id;
	groupMovementOrder[eatIndex++] = PWNguild_id;
	groupMovementOrder[eatIndex++] = BOguild_id;
	groupMovementOrder[eatIndex++] = BDguild_id;
	groupMovementOrder[eatIndex++] = BCguild_id;
	groupMovementOrder[eatIndex++] = BGguild_id;
	groupMovementOrder[eatIndex++] = BFSguild_id;
	groupMovementOrder[eatIndex++] = BFFguild_id;
	groupMovementOrder[eatIndex++] = BFDguild_id;
	groupMovementOrder[eatIndex++] = BMSguild_id;
	groupMovementOrder[eatIndex++] = BMLguild_id;
	groupMovementOrder[eatIndex++] = BMDguild_id;
	groupMovementOrder[eatIndex++] = SGguild_id;
	groupMovementOrder[eatIndex++] = MAguild_id;
	groupMovementOrder[eatIndex++] = MBguild_id;
	groupMovementOrder[eatIndex++] = DLguild_id;
	groupMovementOrder[eatIndex++] = DCguild_id;

	eatIndex = 0;
	setInvertTypeIndex(bm, LG_PHY, groupEatOrder, &eatIndex);
	setInvertTypeIndex(bm, DINOFLAG, groupEatOrder, &eatIndex);
	setInvertTypeIndex(bm, SM_PHY, groupEatOrder, &eatIndex);
	setInvertTypeIndex(bm, SM_ZOO, groupEatOrder, &eatIndex);
	groupEatOrder[eatIndex++] = ZMguild_id;
	groupEatOrder[eatIndex++] = ZLguild_id;
	groupEatOrder[eatIndex++] = ZGguild_id;

	setInvertTypeIndex(bm, CEP, groupEatOrder, &eatIndex);
	setInvertTypeIndex(bm, PWN, groupEatOrder, &eatIndex);
	setInvertTypeIndex(bm, PL_BACT, groupEatOrder, &eatIndex);
	setInvertTypeIndex(bm, SED_BACT, groupEatOrder, &eatIndex);
	setInvertTypeIndex(bm, SM_INF, groupEatOrder, &eatIndex);
	setInvertTypeIndex(bm, LG_INF, groupEatOrder, &eatIndex);
	setInvertTypeIndex(bm, SED_EP_FF, groupEatOrder, &eatIndex);
	setInvertTypeIndex(bm, SED_EP_OTHER, groupEatOrder, &eatIndex);
	groupEatOrder[eatIndex++] = BMSguild_id;
	groupEatOrder[eatIndex++] = BMLguild_id;
	groupEatOrder[eatIndex++] = BMDguild_id;

	groupEatOrder[eatIndex++] = MAguild_id;
	groupEatOrder[eatIndex++] = SGguild_id;
	groupEatOrder[eatIndex++] = MBguild_id;

	setInvertTypeIndex(bm, LAB_DET, groupEatOrder, &eatIndex);
	setInvertTypeIndex(bm, CARRION, groupEatOrder, &eatIndex);
	setInvertTypeIndex(bm, REF_DET, groupEatOrder, &eatIndex);


	//	for(i = 0; i < bm->K_num_tot_sp; i++){
	//		printf("groupEatOrder[%d] = %d\n", i, groupEatOrder[i]);
	//		printf("groupMovementOrder[%d] = %d\n", i, groupMovementOrder[i]);
	//	}
	eatIndex = 0;
	wcOrder[eatIndex++] = MBguild_id;
	wcOrder[eatIndex++] = PLguild_id;
	wcOrder[eatIndex++] = PSguild_id;
	wcOrder[eatIndex++] = DFguild_id;
	wcOrder[eatIndex++] = ZSguild_id;
	wcOrder[eatIndex++] = ZMguild_id;
	wcOrder[eatIndex++] = ZLguild_id;
	wcOrder[eatIndex++] = ZGguild_id;
	wcOrder[eatIndex++] = CEPguild_id;
	wcOrder[eatIndex++] = PWNguild_id;
	wcOrder[eatIndex++] = DLguild_id;
	wcOrder[eatIndex++] = DRguild_id;

	for (guild = 0; guild < bm->K_num_tot_sp; guild++)
		if (FunctGroupArray[guild].groupAgeType == AGE_STRUCTURED)
			wcOrder[eatIndex++] = guild;
	wcOrder[eatIndex++] = PBguild_id;
	wcOrder[eatIndex++] = DCguild_id;

	for (i = eatIndex; i < bm->K_num_tot_sp; i++)
		wcOrder[eatIndex++] = -1;

	eatIndex = 0;
	sedOrder[eatIndex++] = MBguild_id;
	sedOrder[eatIndex++] = PLguild_id;
	sedOrder[eatIndex++] = PSguild_id;
	sedOrder[eatIndex++] = DFguild_id;
	sedOrder[eatIndex++] = BOguild_id;
	sedOrder[eatIndex++] = BDguild_id;
	sedOrder[eatIndex++] = BCguild_id;
	sedOrder[eatIndex++] = BBguild_id;
	sedOrder[eatIndex++] = DLguild_id;
	sedOrder[eatIndex++] = DRguild_id;
	sedOrder[eatIndex++] = DCguild_id;

	for (i = eatIndex; i < bm->K_num_tot_sp; i++)
		sedOrder[eatIndex++] = -1;

	eatIndex = 0;

	setInvertTypeIndex(bm, PHYTOBEN, epiOrder, &eatIndex);
	setInvertTypeIndex(bm, SEAGRASS, epiOrder, &eatIndex);
	setInvertTypeIndex(bm, SED_EP_FF, epiOrder, &eatIndex);
	setInvertTypeIndex(bm, MOB_EP_OTHER, epiOrder, &eatIndex);
	setInvertTypeIndex(bm, SED_EP_OTHER, epiOrder, &eatIndex);
	setInvertTypeIndex(bm, CEP, epiOrder, &eatIndex);

	setInvertTypeIndex(bm, PWN, epiOrder, &eatIndex);

	for (guild = 0; guild < bm->K_num_tot_sp; guild++)
		if (FunctGroupArray[guild].groupAgeType == AGE_STRUCTURED)
			epiOrder[eatIndex++] = guild;

	for (i = eatIndex; i < bm->K_num_tot_sp; i++)
		epiOrder[eatIndex++] = -1;

	eatIndex = 0;
	TLorder2[eatIndex++] = PLguild_id;
	TLorder2[eatIndex++] = DFguild_id;
	TLorder2[eatIndex++] = PSguild_id;
	TLorder2[eatIndex++] = ZSguild_id;
	TLorder2[eatIndex++] = ZMguild_id;
	TLorder2[eatIndex++] = ZLguild_id;
	TLorder2[eatIndex++] = ZGguild_id;
	TLorder2[eatIndex++] = CEPguild_id;
	TLorder2[eatIndex++] = PWNguild_id;
	TLorder2[eatIndex++] = BDguild_id;
	TLorder2[eatIndex++] = BCguild_id;
	TLorder2[eatIndex++] = BFSguild_id;
	TLorder2[eatIndex++] = BFFguild_id;
	TLorder2[eatIndex++] = BFDguild_id;
	TLorder2[eatIndex++] = BGguild_id;
	TLorder2[eatIndex++] = BMSguild_id;
	TLorder2[eatIndex++] = BMLguild_id;
	TLorder2[eatIndex++] = BMDguild_id;
	TLorder2[eatIndex++] = MAguild_id;
	TLorder2[eatIndex++] = MBguild_id;
	TLorder2[eatIndex++] = SGguild_id;

	//for (i = 0; i < bm->K_num_vert_sp; i++) {
    for (guild = 0; guild < bm->K_num_tot_sp; guild++){
        if (FunctGroupArray[guild].groupAgeType == AGE_STRUCTURED)
            TLorder2[eatIndex++] = i;
	}
	TLorder2[eatIndex++] = DLguild_id;
	TLorder2[eatIndex++] = DRguild_id;
	TLorder2[eatIndex++] = DCguild_id;

	eatIndex = 0;

	for (i = 0; i < bm->K_num_tot_sp; i++) {
		if (FunctGroupArray[i].isVertebrate == FALSE && FunctGroupArray[i].isDetritus == FALSE) {
			TLorder3[eatIndex++] = i;
		}
	}

	for (i = 0; i < bm->K_num_tot_sp; i++) {
		if (FunctGroupArray[i].isVertebrate == TRUE) {
			TLorder3[eatIndex++] = i;
		}
	}

	for (i = 0; i < bm->K_num_tot_sp; i++) {
		if (FunctGroupArray[i].isDetritus == TRUE) {
			TLorder3[eatIndex++] = i;
		}
	}


//	eatIndex = 0;
//	for(i = 0; i < bm->K_num_tot_sp; i++){
//		//if(FunctGroupArray[i].isOncePerDt == FALSE){
//			sedOrder[eatIndex] = i;
//			wcOrder[eatIndex] = i;
//			epiOrder[eatIndex] = i;
//			eatIndex++;
//		//}
//	}

/*

	for(i = 0; i < bm->K_num_tot_sp; i++){
		//groupEatOrder[i] = i;
		groupMovementOrder[i] = i;
		TLorder2[i] = i;
		TLorder3[i] = i;
		wcOrder[i] = i;
		sedOrder[i] = i;
		epiOrder[i] = i;
	}
*/
#endif
	/* Setup the cover indices */
	Setup_Cover_Groups(bm, llogfp);

	if (verbose > 0){
	  printf("Read_Functional_Group_XML\n");
	}
	return TRUE;
}

/**
 *\brief Free up the functGroupArray memory.
 *
 *	Frees up each of the tracer arrays and then the functGroupArray array.
 */
void Free_Functional_Group_Memory(MSEBoxModel *bm) {
	int i;

    printf("Free Functional Group memory\n");
    
	if(verbose > 0){
	  printf("Free Functional Group memory\n");
	}
	for (i = 0; i < bm->K_num_tot_sp; i++) {

		i_free1d(FunctGroupArray[i].habitatCoeffs);

		if (FunctGroupArray[i].isVertebrate == TRUE) {
			i_free1d(FunctGroupArray[i].structNTracers);
			i_free1d(FunctGroupArray[i].resNTracers);
			i_free1d(FunctGroupArray[i].NumsTracers);
			i_free1d(FunctGroupArray[i].GrowthTracers);
			i_free1d(FunctGroupArray[i].EatTracers);
			i_free1d(FunctGroupArray[i].CatchTracers);
			i_free1d(FunctGroupArray[i].DiscardTracers);
			i_free1d(FunctGroupArray[i].totNTracers);
			i_free1d(FunctGroupArray[i].allgone);
			free2d(FunctGroupArray[i].grow);
		} else {
			i_free1d(FunctGroupArray[i].totNTracers);
			i_free1d(FunctGroupArray[i].prodnTracers);
			i_free1d(FunctGroupArray[i].GrazingTracers);
			free1d(FunctGroupArray[i].releaseNH);
			free1d(FunctGroupArray[i].uptakeNH);
			free1d(FunctGroupArray[i].prodnDR);
			free1d(FunctGroupArray[i].prodnDL);
			free1d(FunctGroupArray[i].growth);
			free1d(FunctGroupArray[i].mortality);
			free1d(FunctGroupArray[i].grazing);
			free1d(FunctGroupArray[i].lysis);
			free1d(FunctGroupArray[i].sn);
			free1d(FunctGroupArray[i].transDR);
			//free(FunctGroupArray[i].invertProps);
		}

		i_free1d(FunctGroupArray[i].CaughtByFisheryTracers);
		i_free1d(FunctGroupArray[i].DiscardedByFisheryTracers);

		free1d(FunctGroupArray[i].speciesParams);
		free1d(FunctGroupArray[i].age_mat);
		i_free1d(FunctGroupArray[i].cohort_stage);
		free1d(FunctGroupArray[i].C_T15);
		free1d(FunctGroupArray[i].C_T15_per_day);
		free1d(FunctGroupArray[i].SP_C_per_day);
		free1d(FunctGroupArray[i].SP_C);
		free1d(FunctGroupArray[i].mum_T15);
		free1d(FunctGroupArray[i].mum_T15_per_day);
		free1d(FunctGroupArray[i].mum);
		free1d(FunctGroupArray[i].scaled_mum);
		free1d(FunctGroupArray[i].scaled_C);
		free1d(FunctGroupArray[i].mum_per_day);
		free1d(FunctGroupArray[i].CLEAR);
		//free1d(FunctGroupArray[i].GrazeLive);
		d_free1longd(FunctGroupArray[i].GrazeLive);

        i_free1d(FunctGroupArray[i].needed_for_age_away);

        if(bm->track_contaminants) {
            free2d(FunctGroupArray[i].C_growth_corr);
            free2d(FunctGroupArray[i].C_reprod_corr);
            free1d(FunctGroupArray[i].C_move_corr);

            i_free2d(FunctGroupArray[i].contaminantTracers);
            i_free2d(FunctGroupArray[i].contamPropTracers);
            free(FunctGroupArray[i].contaminantSpMort);
            free2d(FunctGroupArray[i].calcCLinearMort);
            free4d(FunctGroupArray[i].agingContam);
            
            if(bm->track_contaminants){
                free(FunctGroupArray[i].reprodContam);
                free(FunctGroupArray[i].reprodContamCount);
                free(FunctGroupArray[i].LocalPopCount);
            }
        }

        free2d(FunctGroupArray[i].max_scalar);

		free1d(FunctGroupArray[i].dead);
		free1d(FunctGroupArray[i].deadGlobal);

		d_free2longd(FunctGroupArray[i].preyEaten);
		d_free3longd(FunctGroupArray[i].preyEatenGlobal);

        i_free1d(FunctGroupArray[i].NeedMoveUpdate);

		if (FunctGroupArray[i].isMobile) {
			free3d(FunctGroupArray[i].distrib_VERTICAL);
		}
		if(FunctGroupArray[i].isOverWinter){
			free2d(FunctGroupArray[i].distrib_OVERWINTER);
		}

        /* Free up the arrays used to keep track of P and C ratios */
		if(bm->track_atomic_ratio == TRUE){
			i_free2d(FunctGroupArray[i].addRatioTracers);
			d_free2d(FunctGroupArray[i].addRatioFluxes);
			d_free3longd(FunctGroupArray[i].ratioLost);
			d_free4longd(FunctGroupArray[i].ratioLostGlobal);
			d_free2longd(FunctGroupArray[i].ratioGainedPred);
			//d_free4longd(FunctGroupArray[i].ratioGainedPredGlobal);
			d_free2d(FunctGroupArray[i].addRatioLost);


			d_free3longd(FunctGroupArray[i].ratioLostPred);
			d_free4longd(FunctGroupArray[i].ratioLostPredGlobal);

			d_free3longd(FunctGroupArray[i].ratioGained);
			d_free4longd(FunctGroupArray[i].ratioGainedGlobal);

		}

		//if(FunctGroupArray[i].numMoveEntries > 0)
			d_free3d(FunctGroupArray[i].distrib);

		if(bm->flag_macro_model)
			free1d(FunctGroupArray[i].pSP_SG_eat);

        /* Free the group type dependant arrays */
		switch (FunctGroupArray[i].groupAgeType) {
		case AGE_STRUCTURED:
			free2d(FunctGroupArray[i].cohortSpeciesParams);
			free2d(FunctGroupArray[i].spawnSpeciesParams);
			free3d(FunctGroupArray[i].pSPEat);
			free1d(FunctGroupArray[i].FSPB);
            free1d(FunctGroupArray[i].scaled_FSPB);
            free1d(FunctGroupArray[i].X_RS);
			free3d(FunctGroupArray[i].AGEnewden);
			free3d(FunctGroupArray[i].agingVERT);
			free3d(FunctGroupArray[i].SizeNumDiscard);
			free3d(FunctGroupArray[i].SizeNumCaught);
            free3d(FunctGroupArray[i].SizeDiscard);
            free3d(FunctGroupArray[i].SizeCaught);
			free4d(FunctGroupArray[i].boxPopRatio);
            free2d(FunctGroupArray[i].RAssessSpringSurvey);
            free2d(FunctGroupArray[i].RAssessAutumnSurvey);
            free1d(FunctGroupArray[i].min_wgt);
            free1d(FunctGroupArray[i].max_wgt);
            free2d(FunctGroupArray[i].rolling_wgt);
            free1d(FunctGroupArray[i].min_B);
            free1d(FunctGroupArray[i].max_B);
            free2d(FunctGroupArray[i].rolling_B);
			break;
		case AGE_STRUCTURED_BIOMASS:
			free2d(FunctGroupArray[i].cohortSpeciesParams);
			free2d(FunctGroupArray[i].spawnSpeciesParams);
			free3d(FunctGroupArray[i].pSPEat);
			free2d(FunctGroupArray[i].INVpopratio);
			free2d(FunctGroupArray[i].tempINVpopratio);
			free3d(FunctGroupArray[i].AGEnewden);
			free3d(FunctGroupArray[i].agingVERT);
			free4d(FunctGroupArray[i].boxPopRatio);
			free1d(FunctGroupArray[i].FSPB);
            free1d(FunctGroupArray[i].scaled_FSPB);
            free1d(FunctGroupArray[i].X_RS);
            free1d(FunctGroupArray[i].min_B);
            free1d(FunctGroupArray[i].max_B);
            free2d(FunctGroupArray[i].rolling_B);
			break;
		case BIOMASS:
			free2d(FunctGroupArray[i].cohortSpeciesParams);
            if(FunctGroupArray[i].isCultured)
                free2d(FunctGroupArray[i].spawnSpeciesParams);
            free3d(FunctGroupArray[i].pSPEat);
            free1d(FunctGroupArray[i].X_RS);
            free1d(FunctGroupArray[i].min_B);
            free1d(FunctGroupArray[i].max_B);
            free2d(FunctGroupArray[i].rolling_B);
			break;
		}

        if (FunctGroupArray[i].co_sp)
            i_free1d(FunctGroupArray[i].co_sp);
        if (FunctGroupArray[i].co_sp_catch)
            free2d(FunctGroupArray[i].co_sp_catch);
	}

	free(FunctGroupArray);

#ifdef ORIGINAL_GROUP_ORDER
	i_free1d(groupEatOrder);
	i_free1d(groupMovementOrder);
	//i_free1d(TLorder);
	i_free1d(TLorder2);
	i_free1d(TLorder3);
	i_free1d(wcOrder);
	i_free1d(sedOrder);
	i_free1d(epiOrder);
#endif
}

/**
 * \brief Output the tracer array values to the given file. Can be a useful testing tool.
 *
 *
 */
void Output_Tracer_Arrays(MSEBoxModel *bm, FILE *fp) {
	int i, cohort;

	for (i = 0; i < bm->K_num_tot_sp; i++) {
		if (FunctGroupArray[i].isVertebrate == TRUE) {
			for (cohort = 0; cohort < FunctGroupArray[i].numCohortsXnumGenes; cohort++)
				fprintf(fp, "FunctGroupArray[%d].totNTracers = %d\n", i, FunctGroupArray[i].totNTracers[cohort]);

			for (cohort = 0; cohort < FunctGroupArray[i].numCohortsXnumGenes; cohort++)
				fprintf(fp, "FunctGroupArray[%d].structNTracers = %d\n", i, FunctGroupArray[i].structNTracers[cohort]);

			for (cohort = 0; cohort < FunctGroupArray[i].numCohortsXnumGenes; cohort++)
				fprintf(fp, "FunctGroupArray[%d].resNTracers = %d\n", i, FunctGroupArray[i].resNTracers[cohort]);

			for (cohort = 0; cohort < FunctGroupArray[i].numCohortsXnumGenes; cohort++)
				fprintf(fp, "FunctGroupArray[%d].NumsTracers = %d\n", i, FunctGroupArray[i].NumsTracers[cohort]);

			for (cohort = 0; cohort < FunctGroupArray[i].numCohortsXnumGenes; cohort++)
				fprintf(fp, "FunctGroupArray[%d].GrowthTracers = %d\n", i, FunctGroupArray[i].GrowthTracers[cohort]);

			for (cohort = 0; cohort < FunctGroupArray[i].numCohortsXnumGenes; cohort++)
				fprintf(fp, "FunctGroupArray[%d].EatTracers = %d\n", i, FunctGroupArray[i].EatTracers[cohort]);

			for (cohort = 0; cohort < FunctGroupArray[i].numCohortsXnumGenes; cohort++)
				fprintf(fp, "FunctGroupArray[%d].CatchTracers = %d\n", i, FunctGroupArray[i].CatchTracers[cohort]);

			for (cohort = 0; cohort < FunctGroupArray[i].numCohortsXnumGenes; cohort++)
				fprintf(fp, "FunctGroupArray[%d].DiscardTracers = %d\n", i, FunctGroupArray[i].DiscardTracers[cohort]);
		} else {
			for (cohort = 0; cohort < FunctGroupArray[i].numCohortsXnumGenes; cohort++) {
				fprintf(fp, "FunctGroupArray[%d].totNTracers = %d\n", i, FunctGroupArray[i].totNTracers[cohort]);
			}

			for (cohort = 0; cohort < FunctGroupArray[i].numCohortsXnumGenes; cohort++) {
				fprintf(fp, "FunctGroupArray[%d].prodnTracers = %d\n", i, FunctGroupArray[i].prodnTracers[cohort]);
			}

			for (cohort = 0; cohort < FunctGroupArray[i].numCohortsXnumGenes; cohort++) {
				fprintf(fp, "FunctGroupArray[%d].GrazingTracers = %d\n", i, FunctGroupArray[i].GrazingTracers[cohort]);
			}

		}
	}
}

/**
 * \brief Setup the cover indices of the reef, flat, soft and canyon.
 *
 * These are calculated based on the number of cover groups that are in the functional group definition file.
 */
void Setup_Cover_Groups(MSEBoxModel *bm, FILE *llogfp) {
	int sp;
	int coverIndex = 0;

	for (sp = 0; sp < bm->K_num_tot_sp; sp++) {
		if (FunctGroupArray[sp].isCover == TRUE) {
			FunctGroupArray[sp].coverID = coverIndex;
			coverIndex++;
		}
	}
	bm->REEFcover_id = coverIndex;
	coverIndex++;
	bm->FLATcover_id = coverIndex;
	coverIndex++;
	bm->SOFTcover_id = coverIndex;
	coverIndex++;
	bm->CANYONcover_id = coverIndex;

	if(bm->K_num_cover_types != coverIndex + 1){
		quit("Setup_Cover_Groups. Incorrect value in your run input file for 'K_num_cover_types'. There are %d cover types defined in your functional group definition file.\n", (coverIndex + 1));
	}
}


/**
 *	\brief Setup the habitat preferences for each group based on its groupType.
 *
 *
 *
 */
static void Set_Group_Habitats(MSEBoxModel *bm) {
	int guild;

	for (guild = 0; guild < bm->K_num_tot_sp; guild++) {

		FunctGroupArray[guild].habitatCoeffs = Util_Alloc_Init_1D_Int(bm->num_active_habitats, 0);

		FunctGroupArray[guild].habitatCoeffs[WC] = 0;
		FunctGroupArray[guild].habitatCoeffs[SED] = 0;
		FunctGroupArray[guild].habitatCoeffs[EPIFAUNA] = 0;
		FunctGroupArray[guild].habitatCoeffs[ICE_BASED] = 0;
		FunctGroupArray[guild].habitatCoeffs[LAND_BASED] = 0;


		if(bm->terrestrial_on){
			if(FunctGroupArray[guild].isLandActive > 0)
				FunctGroupArray[guild].habitatCoeffs[LAND_BASED]  = 1;
		}

		switch (FunctGroupArray[guild].groupType) {
		case NUM_GROUP_TYPES:
			/* Ignore - added to make sure we cover all groups */
			break;
		case BIRD:
		case MAMMAL:
		case SHARK:
        case FISH:
        case FISH_INVERT:
		case CEP:
		case PWN:
		case LG_ZOO:
		case SM_ZOO:
		case MED_ZOO:
		case PL_BACT:
			FunctGroupArray[guild].habitatType = WC;
			FunctGroupArray[guild].habitatCoeffs[WC] = 1;
			break;
		case DINOFLAG:
		case LG_PHY:
		case SM_PHY:
		case LAB_DET:
		case REF_DET:
		case CARRION:
			FunctGroupArray[guild].habitatType = WC;
			FunctGroupArray[guild].habitatCoeffs[WC] = 1;
			FunctGroupArray[guild].habitatCoeffs[SED] = 1;
			break;

		case SED_EP_FF:
		case MOB_EP_OTHER:
		case SED_EP_OTHER:
		case PHYTOBEN:
		case SEAGRASS:
		case TURF:
		case CORAL:
        case SPONGE:
			FunctGroupArray[guild].habitatType = EPIFAUNA;
			FunctGroupArray[guild].habitatCoeffs[EPIFAUNA] = 1;
			break;

		case LG_INF:
		case SM_INF:
		case SED_BACT:
			FunctGroupArray[guild].habitatType = SED;
			FunctGroupArray[guild].habitatCoeffs[SED] = 1;
			break;

		case MICROPHTYBENTHOS:
			FunctGroupArray[guild].habitatType = SED;
			FunctGroupArray[guild].habitatCoeffs[WC] = 1;
			FunctGroupArray[guild].habitatCoeffs[SED] = 1;
			FunctGroupArray[guild].habitatCoeffs[EPIFAUNA] = 1;
			break;
		case ICE_BACT:
		case ICE_DIATOMS: //(diatoms)
		case ICE_MIXOTROPHS: //(dinoflagellates)
		case ICE_ZOOBIOTA: 	//(small zooplankton)
			FunctGroupArray[guild].habitatType = ICE_BASED;
            FunctGroupArray[guild].habitatCoeffs[MIXED] = 1;
			FunctGroupArray[guild].habitatCoeffs[ICE_BASED] = 1;
			break;

		}

		/* Check to see if phytoplankton - */
		FunctGroupArray[guild].isWCActiveOnly = FALSE;
		if (FunctGroupArray[guild].habitatType == WC && (FunctGroupArray[guild].groupType == LG_PHY || FunctGroupArray[guild].groupType
				== SM_PHY)) {
			FunctGroupArray[guild].isWCActiveOnly = TRUE;
		}
	}
}
