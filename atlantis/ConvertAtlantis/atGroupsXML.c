/*
 * atGroupsXML.c
 *
 *  Created on: 23/08/2011
 *      Author: bec
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
#include "convertXML.h"

void Convert_Groups_To_XML(MSEBoxModel *bm, char *fileName, char *outputFileName) {
	xmlDocPtr doc;
	xmlNodePtr rootnode;
	xmlNodePtr groupNode;
	FILE *inputFP;
	xmlNodePtr node, lookupNode;
	int numExpectedTokens = 32;
	int buflen = 2000;
	char ch, buf[2000], seps[] = ",";
	char *varStr;
	int count = 0;


    if(bm->external_populations){
        numExpectedTokens = numExpectedTokens + 1;
    }
    if(bm->terrestrial_on) {
		numExpectedTokens = numExpectedTokens + 1;
    }
    if(bm->flag_pollutant_impacts){
        numExpectedTokens = numExpectedTokens + 2;
    }

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
	Util_XML_Add_Node_Property(rootnode, AtlantisXMLObjectNAMES[ATLANTIS_ATTRIBUTE_GROUP][ATTRIBUTE_NAME], "AtlantisFunctionalGroups");

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
		quit("\n\nERROR: The functionalGroup input file does not have the correct number of columns %d it has %d columns\n\n", numExpectedTokens, count);

	}
    
    count = 0;
	while (fgets(buf, buflen, inputFP) != NULL) {

		/* Ignore the first line */
		if (strstr(buf, "Code") != NULL)
			continue;

		ch = buf[0];

		if (ch != '\t' && ch != '\n' && ch != '#' && ch != ',') {
            
            /* Create the node for the group */
			groupNode = Util_XML_Create_Node(ATLANTIS_ATTRIBUTE_SUB_GROUP, rootnode, "FunctionalGroup", "The defintion of a functional group", "", "");

			/* Group Code value. */
			varStr = strtok(buf, seps);
			Util_XML_Create_Node(ATLANTIS_ATTRIBUTE, groupNode, "GroupCode", "", "", varStr);

			/* Get the index. - ignore this value */
			varStr = strtok(NULL, seps);
            
            /* Is Active */
			varStr = strtok(NULL, seps);
			Util_XML_Create_Node(ATLANTIS_ATTRIBUTE, groupNode, "IsTurnedOn", "", "", varStr);

			/* Group Name */
			varStr = strtok(NULL, seps);
			Util_XML_Create_Node(ATLANTIS_ATTRIBUTE, groupNode, "Name", "", "", varStr);

			/* Long Name */
			varStr = strtok(NULL, seps);
			Util_XML_Create_Node(ATLANTIS_ATTRIBUTE, groupNode, "LongName", "", "", varStr);

            /* NumCohorts */
			varStr = strtok(NULL, seps);
			Util_XML_Create_Node(ATLANTIS_ATTRIBUTE, groupNode, "NumCohorts", "", "", varStr);

			/* NumGeneTypes */
			varStr = strtok(NULL, seps);
			Util_XML_Create_Node(ATLANTIS_ATTRIBUTE, groupNode, "NumGeneTypes", "", "", varStr);

			/* NumStages */
			varStr = strtok(NULL, seps);
			Util_XML_Create_Node(ATLANTIS_ATTRIBUTE, groupNode, "NumStages", "", "", varStr);

            /* NumSpawns */
			varStr = strtok(NULL, seps);
			Util_XML_Create_Node(ATLANTIS_ATTRIBUTE, groupNode, "NumSpawns", "", "", varStr);
 
			/* NumAgeClassSize */
			varStr = strtok(NULL, seps);
			Util_XML_Create_Node(ATLANTIS_ATTRIBUTE, groupNode, "NumAgeClassSize", "", "", varStr);

			/* NumStocks */
			varStr = strtok(NULL, seps);
			Util_XML_Create_Node(ATLANTIS_ATTRIBUTE, groupNode, "NumStocks", "", "", varStr);
            
            /* Vertically Mobile in day to day movement */
			varStr = strtok(NULL, seps);
			Util_XML_Create_Node(ATLANTIS_ATTRIBUTE, groupNode, "VerticallyMobile", "", "", varStr);

			/* Horizontally Mobile in day to day movement */
			varStr = strtok(NULL, seps);
			Util_XML_Create_Node(ATLANTIS_ATTRIBUTE, groupNode, "Mobile", "", "", varStr);

            /* Horizontally Mobile in day to day movement */
            varStr = strtok(NULL, seps);
            Util_XML_Create_Node(ATLANTIS_ATTRIBUTE, groupNode, "NumMigrations", "", "", varStr);

            /* Horizontally Mobile in day to day movement */
            varStr = strtok(NULL, seps);
            Util_XML_Create_Node(ATLANTIS_ATTRIBUTE, groupNode, "MultiYrMigrations", "", "", varStr);

            /* Where Reproduce - internal vs external  */
            varStr = strtok(NULL, seps);
            Util_XML_Create_Node(ATLANTIS_ATTRIBUTE, groupNode, "ExternalReproduction", "", "", varStr);

            /* Recruitment option  */
            varStr = strtok(NULL, seps);
            Util_XML_Create_Node(ATLANTIS_ATTRIBUTE, groupNode, "RecruitType", "", "", varStr);

            /* IsFished */
			varStr = strtok(NULL, seps);
			Util_XML_Create_Node(ATLANTIS_ATTRIBUTE, groupNode, "IsFished", "", "", varStr);

			/* IsImpacted */
			varStr = strtok(NULL, seps);
			Util_XML_Create_Node(ATLANTIS_ATTRIBUTE, groupNode, "IsImpacted", "", "", varStr);

			/* IsTAC */
			varStr = strtok(NULL, seps);
			Util_XML_Create_Node(ATLANTIS_ATTRIBUTE, groupNode, "IsTAC", "", "", varStr);

			/* GroupType */
			varStr = strtok(NULL, seps);
			node = Util_XML_Create_Node(ATLANTIS_ATTRIBUTE, groupNode, "GroupType", "", "", varStr);

            if(count == 0){
				lookupNode = Util_XML_Create_Lookup_Node(node);
				Util_XML_Create_Lookup_Value_Node(lookupNode, "LG_PHY", "Large Phytoplankton");
				Util_XML_Create_Lookup_Value_Node(lookupNode, "SM_PHY", "Small Phytoplankton");
				Util_XML_Create_Lookup_Value_Node(lookupNode, "SM_ZOO", "Small Zooplankton");
				Util_XML_Create_Lookup_Value_Node(lookupNode, "MED_ZOO", "Medium Zooplankton");
				Util_XML_Create_Lookup_Value_Node(lookupNode, "LG_ZOO", "Large Zooplankton");
				Util_XML_Create_Lookup_Value_Node(lookupNode, "CEP", "Cephalopod");
				Util_XML_Create_Lookup_Value_Node(lookupNode, "PWN", "Prawns");
				Util_XML_Create_Lookup_Value_Node(lookupNode, "PL_BACT", "Pelagic Bacteria");
				Util_XML_Create_Lookup_Value_Node(lookupNode, "SED_BACT", "Sediment Bacteria");
				Util_XML_Create_Lookup_Value_Node(lookupNode, "ICE_BACT", "Ice Bacteria");
				Util_XML_Create_Lookup_Value_Node(lookupNode, "SM_INF", "Small Infauna");
				Util_XML_Create_Lookup_Value_Node(lookupNode, "LG_INF", "Large Infauna");
				Util_XML_Create_Lookup_Value_Node(lookupNode, "SED_EP_FF", "Sediment Epibethic Filter Feeders");
				Util_XML_Create_Lookup_Value_Node(lookupNode, "SED_EP_OTHER", "Sediment Epibethic Others");
				Util_XML_Create_Lookup_Value_Node(lookupNode, "MOB_EP_OTHER", "Mobile Epibethic Others");
				Util_XML_Create_Lookup_Value_Node(lookupNode, "PHYTOBEN", "Phytoben");
				Util_XML_Create_Lookup_Value_Node(lookupNode, "TURF", "Turf");
				Util_XML_Create_Lookup_Value_Node(lookupNode, "LAB_DET", "Labile Detritus");
				Util_XML_Create_Lookup_Value_Node(lookupNode, "REF_DET", "Refractory detritus");
				Util_XML_Create_Lookup_Value_Node(lookupNode, "CARRION", "Carrion");
				Util_XML_Create_Lookup_Value_Node(lookupNode, "DINOFLAG", "Dinoflagellates");
				Util_XML_Create_Lookup_Value_Node(lookupNode, "SEAGRASS", "Seagrass");
				Util_XML_Create_Lookup_Value_Node(lookupNode, "FISH", "Fish");
                Util_XML_Create_Lookup_Value_Node(lookupNode, "FISH_INVERT", "Fished invertebrate");
				Util_XML_Create_Lookup_Value_Node(lookupNode, "MAMMAL", "Mammal");
				Util_XML_Create_Lookup_Value_Node(lookupNode, "SHARK", "Shark");
			}
            
            /* IsPredator */
			varStr = strtok(NULL, seps);
			Util_XML_Create_Node(ATLANTIS_ATTRIBUTE, groupNode, "IsPredator", "", "", varStr);

			/* IsCover */
			varStr = strtok(NULL, seps);
			Util_XML_Create_Node(ATLANTIS_ATTRIBUTE, groupNode, "IsCover", "", "", varStr);

			/* IsSiliconDep */
			varStr = strtok(NULL, seps);
			Util_XML_Create_Node(ATLANTIS_ATTRIBUTE, groupNode, "IsSiliconDep", "", "", varStr);

            /* IsAssessed */
			varStr = strtok(NULL, seps);
			Util_XML_Create_Node(ATLANTIS_ATTRIBUTE, groupNode, "IsAssessed", "", "", varStr);

			/* IsCatchGrazer */
			varStr = strtok(NULL, seps);
			Util_XML_Create_Node(ATLANTIS_ATTRIBUTE, groupNode, "IsCatchGrazer", "", "", varStr);

			/* OverWinters */
			varStr = strtok(NULL, seps);
			Util_XML_Create_Node(ATLANTIS_ATTRIBUTE, groupNode, "OverWinters", "", "", varStr);

            /* IsCultured */
			varStr = strtok(NULL, seps);
			Util_XML_Create_Node(ATLANTIS_ATTRIBUTE, groupNode, "isCultured", "", "", varStr);

            /* IsHabDepend */
			varStr = strtok(NULL, seps);
			Util_XML_Create_Node(ATLANTIS_ATTRIBUTE, groupNode, "isHabDepend", "", "", varStr);

			/* numMoveEntries */
			varStr = strtok(NULL, seps);
			Util_XML_Create_Node(ATLANTIS_ATTRIBUTE, groupNode, "numMoveEntries", "", "", varStr);

            /* isBioEroder */
            varStr = strtok(NULL, seps);
            Util_XML_Create_Node(ATLANTIS_ATTRIBUTE, groupNode, "isBioEroder", "", "", varStr);
            
            /* isSupplemented */
            varStr = strtok(NULL, seps);
            Util_XML_Create_Node(ATLANTIS_ATTRIBUTE, groupNode, "isSupplemented", "", "", varStr);
            
            /* isExternal */
            if(bm->external_populations) {
                varStr = strtok(NULL, seps);
                Util_XML_Create_Node(ATLANTIS_ATTRIBUTE, groupNode, "isExternal", "", "", varStr);
            }
            
            /* Mark if the group is a collision risk */
            if(bm->flagindustry_on) {
                varStr = strtok(NULL, seps);
                Util_XML_Create_Node(ATLANTIS_ATTRIBUTE, groupNode, "isCollisionRisk", "", "", varStr);
            }
            
            /* IsLandActive */
            if(bm->terrestrial_on){
                varStr = strtok(NULL, seps);
                Util_XML_Create_Node(ATLANTIS_ATTRIBUTE, groupNode, "isLandActive", "", "", varStr);
            }
            
            if(bm->flag_pollutant_impacts){
                /* IsLightEffected */
                varStr = strtok(NULL, seps);
                Util_XML_Create_Node(ATLANTIS_ATTRIBUTE, groupNode, "isLightEffected", "", "", varStr);

                /* IsNoiseEffected */
                varStr = strtok(NULL, seps);
                Util_XML_Create_Node(ATLANTIS_ATTRIBUTE, groupNode, "isNoiseEffected", "", "", varStr);
            }
            
            count++;

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
