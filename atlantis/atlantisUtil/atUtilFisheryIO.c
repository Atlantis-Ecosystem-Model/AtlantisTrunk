/************************************************************************//**
 *	\file
 *	\brief This file contains function associated with loading the fishery information.
 *	\brief \ingroup atUtil
 *
 * Revisions:
 29-06-2009 Bec Gorton
 Created the fisheries input file.

 28-10-2009 Bec Gorton
 Added code to close the fishery input file.

 21-06-2012 Bec Gorton
 Added code handle converting the fisheries input file from cvs to xml.

 */
/*  Open library routines */
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>
#include <ctype.h>
#include <sjwlib.h>
#include <netcdf.h>
#include <atlantisboxmodel.h>
#include <atUtilLib.h>
#include <convertXML.h>


/* Test change */

/**
 * /brief This function reads in the fishery definition input file.
 *
 *	The data from the functional group definition file is read in the Fishery array.
 *	This is an array of bm->K_num_fisheries length of FisheryStruct structures. *
 *
 */
int Util_Read_Fisheries_XML(MSEBoxModel *bm, char *fileName, FILE *llogfp) {

	xmlDocPtr inputDoc;
	xmlXPathObjectPtr fisheryList;
	int fisheryIndex, numFisheries;
	xmlNodePtr fisheryNode;
	char convertedXMLFileName[STRLEN];

	if(strlen(fileName) == 0){
		fprintf(
					stderr,
					"\n\nERROR. Util_Read_Fisheries_XML: Trying to read in the fishery definition file, but no file was provided.\n\n See the -q option below.\n\n");
			Util_Usage(1);
			exit(-1);
	}

	if (strstr(fileName, ".csv") != NULL) {

		/* Convert the file to XML. */
		/* Build the converted filename */
		snprintf(convertedXMLFileName, sizeof(convertedXMLFileName), "%s", fileName);
		*(strstr(convertedXMLFileName, ".csv")) = '\0';
		strcat(convertedXMLFileName, ".xml");

		/* Convert the input file to XML - the XML file will be stored in the destination folder if present.*/
		Convert_Fisheries_To_XML(bm, fileName, convertedXMLFileName);
		inputDoc = xmlReadFileDestFolder(bm->destFolder, convertedXMLFileName, NULL, 0);

	}else{
		snprintf(convertedXMLFileName, sizeof(convertedXMLFileName), "%s", fileName);
		inputDoc = xmlReadFileDestFolder("", convertedXMLFileName, NULL, 0);


	}

	/*
	if (strstr(fileName, ".csv") != NULL) {
		fprintf(
				stderr,
				"\n\nERROR. Util_Read_Fisheries_XML: Trying to read in the fishery information from a csv file. This should now be read in from an XML input file\n\n");
		Util_Usage(1);
		exit(-1);
	}
	*/

	if (!inputDoc) {
		fprintf(
				stderr,
				"\n\nERROR. Util_Read_Fisheries_XML: Failed to open fishery input file %s\nThis may be because the file does not exist or it may not be valid XML.\n",
				fileName);
		exit(-1);

	}
	bm->K_max_num_subfleet= 5; // -1;

	/* Read in each of the functional groups */
	fisheryList = Util_XML_Get_Node_List(ATLANTIS_ATTRIBUTE_SUB_GROUP, inputDoc->children, "Fishery");

	if (fisheryList && fisheryList->nodesetval->nodeNr > 0) {

		/* Check the number of groups match the values in the run input file*/
		numFisheries = fisheryList->nodesetval->nodeNr;
		if (numFisheries != bm->K_num_fisheries) {
			quit("Util_Read_Fisheries_XML: Number of fisheries specified (%d) in your fishery defintion file %s does not match the K_num_fisheries value (%d) in your run file\n",
					numFisheries, fileName, bm->K_num_fisheries);
		}

		FisheryArray = (FisheryStruct *) malloc(sizeof(FisheryStruct) * (size_t)(bm->K_num_fisheries + 1));


		/* Now read in the groups. */
		for (fisheryIndex = 0; fisheryIndex < numFisheries; fisheryIndex++) {

			fisheryNode = fisheryList->nodesetval->nodeTab[fisheryIndex];

			Util_XML_Get_Value_String(fileName, ATLANTIS_ATTRIBUTE, TRUE, fisheryNode, "Code", FisheryArray[fisheryIndex].fisheryCode);
			if (verbose > 0)
				printf("Reading in definition of %s\n",  FisheryArray[fisheryIndex].fisheryCode);
			Util_XML_Get_Value_Integer(fileName, ATLANTIS_ATTRIBUTE, 0, TRUE, fisheryNode, integer_check, "Index", &FisheryArray[fisheryIndex].index);
			Util_XML_Get_Value_String(fileName, ATLANTIS_ATTRIBUTE, TRUE, fisheryNode, "Name", FisheryArray[fisheryIndex].name);
			Util_XML_Get_Value_Integer(fileName, ATLANTIS_ATTRIBUTE, 0, TRUE, fisheryNode, integer_check, "IsRec", &FisheryArray[fisheryIndex].isRec);
			Util_XML_Get_Value_Integer(fileName, ATLANTIS_ATTRIBUTE, 0, TRUE, fisheryNode, integer_check, "NumSubFleets", &bm->FISHERYprms[fisheryIndex][nsubfleets_id]);

			if( bm->FISHERYprms[fisheryIndex][nsubfleets_id] > bm->K_max_num_subfleet)
				bm->K_max_num_subfleet = (int)bm->FISHERYprms[fisheryIndex][nsubfleets_id];
		}
		xmlXPathFreeObject(fisheryList);
	}else{
		quit("ERROR: Error in the fishery XML input file %s\n", fileName);
	}

	xmlFreeDoc(inputDoc);
	/* Shutdown libxml */
	xmlCleanupParser();
	return TRUE;
}

/**
 *\brief Free up the FisheryArray memory.
 *
 *	Frees up each of the tracer arrays and then the FisheryArray array.
 */
void Free_Fishery_Def_Memory(MSEBoxModel *bm) {
    
    printf("Free FisheryArray\n");
	free(FisheryArray);
}

