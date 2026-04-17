/**

 \file
 \brief This file contains functions to convert the harvest prm file to the implemention.XML file. Only the management implementation parameters will be pulled
 into the new XML file.

 Changes:

 08-04-2010 Bec Gorton
 Removed the CON input array as we can just use the isConsumer flag in the functional group input file.

 22-06-2010 Bec Gorton
 Merged trunk changes 1698 into bec_dev.

 30-09-2010 Bec Gorton
 Updated the Discard_Fate xml. The original order in the input file is DL, DC, DR. I want this to be more flexible.


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
#include "convertXML.h"

char *habitatStrings[] =
	{ "WC", "SED" };


void createVertebrateSpeciesParam(MSEBoxModel *bm, char *fileName, xmlNodePtr parent, char *str, char *valueStr) {

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
			quit("ERROR: Parameter %s is not the required length in file %s. A value is required for each group.\n", str, fileName);
		}

		if (FunctGroupArray[speciesIndex].isVertebrate == TRUE) {
			/* get the species node */
			Util_XML_Set_Node_Value(ATLANTIS_GROUP_ATTRIBUTE, parent, FunctGroupArray[speciesIndex].groupCode, varstr);
		}
	}
}


void createVertCEPSpeciesParam(MSEBoxModel *bm, char *fileName, xmlNodePtr parent, char *str, char *valueStr) {
	int speciesIndex;
	char *varstr;
	char seps[] = " ";

	/* Now find the node for each group and use the appropriate value in the list */
	for (speciesIndex = 0; speciesIndex < bm->K_num_tot_sp; speciesIndex++) {

		if (speciesIndex == 0)
			varstr = strtok(valueStr, seps);
		else
			varstr = strtok(NULL, seps);

		if(varstr == NULL){
			quit("createVertCEPSpeciesParam: Cannot find a value for %s in attribute %s\n", FunctGroupArray[speciesIndex].groupCode, str);
		}

		if (FunctGroupArray[speciesIndex].isVertebrate || FunctGroupArray[speciesIndex].groupType == CEP) {
			/* get the species node */
			Util_XML_Set_Node_Value(ATLANTIS_GROUP_ATTRIBUTE, parent, FunctGroupArray[speciesIndex].groupCode, varstr);
		} else {
			Util_XML_Set_Node_Value(ATLANTIS_GROUP_ATTRIBUTE, parent, FunctGroupArray[speciesIndex].groupCode, "0.0");
		}

	}
}

void createLiveInvertsSpeciesParam(MSEBoxModel *bm, char *fileName, xmlNodePtr parent, char *str, char *valueStr) {
	int speciesIndex;
	char *varstr;
	char seps[] = " ";

	/* Now find the node for each group and use the appropriate value in the list */
	for (speciesIndex = 0; speciesIndex < bm->K_num_tot_sp; speciesIndex++) {

		if (speciesIndex == 0)
			varstr = strtok(valueStr, seps);
		else
			varstr = strtok(NULL, seps);

		if(varstr == NULL){
			quit("Parse Assessment File: Cannot find a value for %s in the %s array in file %s.\n", FunctGroupArray[speciesIndex].groupCode,
					str, fileName);
		}

		if (FunctGroupArray[speciesIndex].isVertebrate == FALSE && FunctGroupArray[speciesIndex].isDetritus == FALSE) {
			/* get the species node */
			Util_XML_Set_Node_Value(ATLANTIS_GROUP_ATTRIBUTE, parent, FunctGroupArray[speciesIndex].groupCode, varstr);

		}
	}
}

/**
 *	\brief Create the XML structure to hold the vertebrate seasonal distribution data.
 */
void InitDiscardEffortXML(MSEBoxModel *bm, xmlNodePtr parent) {

	int guild, stage;
	xmlNodePtr newNode;

	/* Create a node for each functional group*/
	for (guild = 0; guild < bm->K_num_tot_sp; guild++) {
		if (FunctGroupArray[guild].isDetritus == TRUE) {
			newNode = Util_XML_Create_Node(ATLANTIS_GROUP_ATTRIBUTE, parent, FunctGroupArray[guild].groupCode, "", "", "");
			for (stage = 0; stage <= SED; stage++) {
				Util_XML_Create_Node(ATLANTIS_ATTRIBUTE, newNode, habitatStrings[stage], "", "", "");
			}
		}
	}
}

void createDiscardFateSpeciesParam(MSEBoxModel *bm, char *fileName, xmlNodePtr parent, char *str, char *valueStr) {
	int speciesIndex, stage;
	char *varstr, *ptr;
	char seps[] = " ";
	xmlNodePtr attributeGroupNode;

	/* get the functional group name */
	char tempStr[STRLEN];

	strcpy(tempStr, str);
	ptr = tempStr + strlen("Discard_Fate_");

	speciesIndex = Util_Get_FG_Index(bm, ptr);
	if(speciesIndex == -1){
		quit("Species string %s not recognised in file %s parameter %s\n", ptr, fileName, str);
	}

	attributeGroupNode = Util_XML_Get_Node(ATLANTIS_GROUP_ATTRIBUTE, parent, ptr);
	if (attributeGroupNode == NULL)
		quit("createDiscardFateSpeciesParam -%s attribute group not found.\n", ptr);

	for (stage = 0; stage <= SED; stage++) {
		if (stage == 0)
			varstr = strtok(valueStr, seps);
		else
			varstr = strtok(NULL, seps);

		Util_XML_Set_Node_Value(ATLANTIS_ATTRIBUTE, attributeGroupNode, habitatStrings[stage], varstr);

	}
}

void AssessFlagXML(MSEBoxModel *bm, FILE *fp, char *fileName, xmlDocPtr doc, xmlNodePtr rootnode) {

	xmlNodePtr groupingNode, node, lookupNode;
	groupingNode = Util_XML_Create_Node(ATLANTIS_ATTRIBUTE_SUB_GROUP, rootnode, "Assess_Run_Params", "Run parameters", "", "");

	Util_XML_Parse_Create_Node(fp, fileName, groupingNode, "tassessstart", "Assesment start day", "d", XML_TYPE_FLOAT, "1");
	Util_XML_Parse_Create_Node(fp, fileName, groupingNode, "tassessinc", "Basic assessment increment - Collect data every x days", "d", XML_TYPE_FLOAT, "1");
	Util_XML_Parse_Create_Node(fp, fileName, groupingNode, "teatassessinc", "Collect diet information every x number of timesteps", "d", XML_TYPE_FLOAT, "1");

	Util_XML_Parse_Create_Node(fp, fileName, groupingNode, "tClassicalAssessStart", "Time that classical assessment model starts", "d", XML_TYPE_FLOAT, "1");

	Util_XML_Parse_Create_Node(fp, fileName, groupingNode, "flaggen", "Whether to generate random numbers (1) or read them in (0)", "", XML_TYPE_BOOLEAN,"1");
	Util_XML_Parse_Create_Node(fp, fileName, groupingNode, "flagpatchy", "Whether want patchy temporal sampling (1) or monitoring at a regular frequency (0)", "", XML_TYPE_BOOLEAN,"1");
	Util_XML_Parse_Create_Node(fp, fileName, groupingNode, "flagcont", "Whether want to have within year continuous sampling (1) or vertebrate surveys only annual (0) even if physical properties collected more frequently","", XML_TYPE_BOOLEAN,"1");
	Util_XML_Parse_Create_Node(fp, fileName, groupingNode, "flagareacorrect","Whether to average sampling totals over space and time (1) or just number of sampling trips in the year (0) ", "", XML_TYPE_BOOLEAN,"1");

    Util_XML_Parse_Create_Node(fp, fileName, groupingNode, "minfreq", "Minimum frequency of sampling", "d", XML_TYPE_FLOAT, "1");
	Util_XML_Parse_Create_Node(fp, fileName, groupingNode, "maxfreq", "Maximum frequency of sampling", "d", XML_TYPE_FLOAT, "1");

	Util_XML_Parse_Create_Node(fp, fileName, groupingNode, "flagecosysassess", "Whether assessing ecosystem as well as specific species", "", XML_TYPE_BOOLEAN,"1");
	Util_XML_Parse_Create_Node(fp, fileName, groupingNode, "flagobsdata", "Whether using on board observers to collect catch information (for size distributions of catch etc)", "",
			XML_TYPE_BOOLEAN, "1");

	Util_XML_Parse_Create_Node(fp, fileName, groupingNode, "nfzones", "Number of fishing zones", "", XML_TYPE_INTEGER, "1");
	Util_XML_Parse_Create_Node(fp, fileName, groupingNode, "nsbox_i", "MNumber of sampling boxes (intensive sampling - or only sampling if only one design used)", "", XML_TYPE_INTEGER, "1");
	Util_XML_Parse_Create_Node(fp, fileName, groupingNode, "nsbox_o", "Number of sampling boxes (less intensive sampling)", "", XML_TYPE_INTEGER, "1");
	Util_XML_Parse_Create_Node(fp, fileName, groupingNode, "crit_depth", "Depth defining inshore-offshore split", "m", XML_TYPE_FLOAT, "1");

	Util_XML_Parse_Create_Node(fp, fileName, groupingNode, "K_num_tot_assess",
			"Total number of biological groups in the assessment model (juveniles and adults handled separately for vertebrates)", "", XML_TYPE_INTEGER, "1");
	Util_XML_Parse_Create_Node(fp, fileName, groupingNode, "K_num_live", "Total number of living groups in the model (flora and fauna)", "", XML_TYPE_INTEGER, "1");
	Util_XML_Parse_Create_Node(fp, fileName, groupingNode, "K_num_eweinvert", "Total number of invertebrate groups considered in the network calculations", "", XML_TYPE_INTEGER, "1");
	//Util_XML_Parse_Create_Node(fp, fileName, groupingNode, "K_num_charis", "Total number of charismatic groups in the model (birds and mammals)", "", XML_TYPE_INTEGER, "1");

	Util_XML_Parse_Create_Node(fp, fileName, groupingNode, "K_num_sampleage", "Total number of separate age classes sampled per group by the sampling model", "", XML_TYPE_INTEGER, "1");
	//Util_XML_Parse_Create_Node(fp, fileName, groupingNode, "K_num_prod", "Total number of invertebrates production info is collected for (including flora)", "", XML_TYPE_INTEGER, "1");
	//Util_XML_Parse_Create_Node(fp, fileName, groupingNode, "K_num_eat", "Total number of invertebrates consumption info is collected for (including flora)", "", XML_TYPE_INTEGER, "1");
	//Util_XML_Parse_Create_Node(fp, fileName, groupingNode, "K_num_invfished", "Total number of invertebrate groups impacted by fishing (including flora)", "", XML_TYPE_INTEGER, "1");

	Util_XML_Parse_Create_Node(fp, fileName, groupingNode, "K_max_agekey", "Maximum number of age classes tracked per group in the sampling model's agelengthkey", "", XML_TYPE_INTEGER, "1");
	Util_XML_Parse_Create_Node(fp, fileName, groupingNode, "K_num_stomaches", "Maximum number of stomaches sampled", "", XML_TYPE_INTEGER, "1");
	Util_XML_Parse_Create_Node(fp, fileName, groupingNode, "K_num_size", "Maximum number of size classes tracked per group in the sampling model's agelengthkey", "", XML_TYPE_INTEGER, "1");
	Util_XML_Parse_Create_Node(fp, fileName, groupingNode, "K_num_pelbin", "Number of bins used in the pelagic biomass size spectra analysis", "", XML_TYPE_INTEGER, "1");
	Util_XML_Parse_Create_Node(fp, fileName, groupingNode, "K_num_sedbin", "Number of bins used in the benthic biomass size spectra analysis", "", XML_TYPE_INTEGER, "1");
	Util_XML_Parse_Create_Node(fp, fileName, groupingNode, "K_num_abcbin", "Number of bins used in the abundance-biomass curve analysis", "", XML_TYPE_INTEGER, "1");
	Util_XML_Parse_Create_Node(fp, fileName, groupingNode, "K_num_trophbin", "Number of bins used in the trophic spectra curve analysis", "", XML_TYPE_INTEGER, "1");
	Util_XML_Parse_Create_Node(fp, fileName, groupingNode, "maxTL", "Maxium number of trophic levels considered in Lindeman network analysis", "", XML_TYPE_INTEGER, "1");

	Util_XML_Parse_Create_Node(fp, fileName, groupingNode, "samplediet", "Flag indicating whether sampled diets are output: 0 = no, 1 = yes", "", XML_TYPE_INTEGER, "1");
	Util_XML_Parse_Create_Node(fp, fileName, groupingNode, "ageout", "Flag indicating whether sampled age structures are output: 0 = no, 1 = yes", "", XML_TYPE_INTEGER, "1");

	Util_XML_Create_Node_Next_Line(fp, fileName, groupingNode, "zones", "Zone each box belongs to", "", XML_TYPE_INTEGER);
	Util_XML_Create_Node_Next_Line(fp, fileName, groupingNode, "sites_intense", "Identity of each of the intensive sampling boxes (or only set if using one design only)", "",
			XML_TYPE_INTEGER);
	Util_XML_Create_Node_Next_Line(fp, fileName, groupingNode, "sites_other", "Identity of the other sites", "", XML_TYPE_INTEGER);

	node = Util_XML_Create_Node_Next_Line(fp, fileName, groupingNode, "InOffshore", "Zone each box belongs to (1=inshore, 0 = offshore)", "", XML_TYPE_INTEGER);
	lookupNode = Util_XML_Create_Lookup_Node(node);
	Util_XML_Create_Lookup_Value_Node(lookupNode, "0", "Offshore");
	Util_XML_Create_Lookup_Value_Node(lookupNode, "1", "Inshore");

	Util_XML_Parse_Create_Node(fp, fileName, groupingNode, "phys_samplingsize", "Number of times the physical properties are sampled in the assessment module.", "", XML_TYPE_INTEGER, "1");
}

void ErrorStructureXML(MSEBoxModel *bm, FILE *fp, char *fileName, xmlDocPtr doc, xmlNodePtr rootnode) {

	xmlNodePtr groupingNode;
	groupingNode = Util_XML_Create_Node(ATLANTIS_ATTRIBUTE_SUB_GROUP, rootnode, "Error_Structure", "Flags for setting the error structure used", "", "");

	Util_XML_Parse_Create_Node(fp, fileName, groupingNode, "flagphys", "Flag for setting error structure used for physical and processes: 0 = uniform, 1 = normal, 2 = lognormal", "",
			XML_TYPE_INTEGER, "1");
	Util_XML_Parse_Create_Node(fp, fileName, groupingNode, "flaginvpbiom",
			"Flag for setting error structure used for pelagic invertebrate biomass: 0 = uniform, 1 = normal, 2 = lognormal", "", XML_TYPE_INTEGER, "1");
	Util_XML_Parse_Create_Node(fp, fileName, groupingNode, "flagepibiom",
			"Flag for setting error structure used for benthic epifauna invertebrate biomass: 0 = uniform, 1 = normal, 2 = lognormal", "", XML_TYPE_INTEGER, "1");
	Util_XML_Parse_Create_Node(fp, fileName, groupingNode, "flaginfbiom",
			"Flag for setting error structure used for benthic infauna invertebrate biomass: 0 = uniform, 1 = normal, 2 = lognormal", "", XML_TYPE_INTEGER, "1");
	Util_XML_Parse_Create_Node(fp, fileName, groupingNode, "flagdetbiom", "Flag for setting error structure used for detritus: 0 = uniform, 1 = normal, 2 = lognormal", "", XML_TYPE_INTEGER, "1");
	Util_XML_Parse_Create_Node(fp, fileName, groupingNode, "flagfishbiom", "Flag for setting error structure used for detritus: 0 = uniform, 1 = normal, 2 = lognormal", "", XML_TYPE_INTEGER, "1");
	Util_XML_Parse_Create_Node(fp, fileName, groupingNode, "flagverts", "Flag for setting error structure used for vertebrate info: 0 = uniform, 1 = normal, 2 = lognormal", "", XML_TYPE_INTEGER,
			"1");
	Util_XML_Parse_Create_Node(fp, fileName, groupingNode, "flagprod", "Flag for setting error structure used for production: 0 = uniform, 1 = normal, 2 = lognormal", "", XML_TYPE_INTEGER, "1");
	Util_XML_Parse_Create_Node(fp, fileName, groupingNode, "flageat", "Flag for setting error structure used for consumption: 0 = uniform, 1 = normal, 2 = lognormal", "", XML_TYPE_INTEGER, "1");
	Util_XML_Parse_Create_Node(fp, fileName, groupingNode, "flagcatch", "Flag for setting error structure used for catch: 0 = uniform, 1 = normal, 2 = lognormal", "", XML_TYPE_INTEGER, "1");
	Util_XML_Parse_Create_Node(fp, fileName, groupingNode, "flageffort", "Flag for setting error structure used for effort: 0 = uniform, 1 = normal,2 = lognormal", "", XML_TYPE_INTEGER, "1");
	Util_XML_Parse_Create_Node(fp, fileName, groupingNode, "flagdiscrd", "Flag for setting error structure used for discard: 0 = uniform, 1 = normal, 2 = lognormal", "", XML_TYPE_INTEGER, "1");
	Util_XML_Parse_Create_Node(fp, fileName, groupingNode, "flagcount", "Flag for setting error structure used for bird counts: 0 = uniform, 1 = normal, 2 = lognormal", "", XML_TYPE_INTEGER, "1");
	Util_XML_Parse_Create_Node(fp, fileName, groupingNode, "flagcurve",
			"Flag for setting error structure used for selectivity curve estimate: 0 = uniform, 1 = normal, 2 = lognormal, 4 = know correct curve", "",
			XML_TYPE_INTEGER, "1");
	Util_XML_Parse_Create_Node(fp, fileName, groupingNode, "flagprms", "Flag for setting error structure used for selectivity curve parameters: 0 = uniform, 1 = normal, 2 = lognormal",
			"", XML_TYPE_INTEGER, "1");
	Util_XML_Parse_Create_Node(fp, fileName, groupingNode, "flagage", "Flag for setting error structure used for aging: 0 = uniform, 1 = normal, 2 = lognormal", "", XML_TYPE_INTEGER, "1");
}

void BiasedSampleXML(MSEBoxModel *bm, FILE *fp, char *fileName, xmlDocPtr doc, xmlNodePtr rootnode) {

	xmlNodePtr groupingNode;
	groupingNode = Util_XML_Create_Node(ATLANTIS_ATTRIBUTE_SUB_GROUP, rootnode, "Biased_Sample", "Biased sample mean as percentage of true mean", "", "");

	Util_XML_Parse_Create_Node(fp, fileName, groupingNode, "k_avgsalt", "For salinity", "", XML_TYPE_INTEGER, "1");
	Util_XML_Parse_Create_Node(fp, fileName, groupingNode, "k_avgphys", "For physical properties (light, oxygen)", "", XML_TYPE_INTEGER, "1");
	Util_XML_Parse_Create_Node(fp, fileName, groupingNode, "k_avgnuts", "For nutrients", "", XML_TYPE_INTEGER, "1");
	Util_XML_Parse_Create_Node(fp, fileName, groupingNode, "k_avgprocs", "For processes (denitrification, nitrification) and chlorophyll a", "", XML_TYPE_INTEGER, "1");
	Util_XML_Parse_Create_Node(fp, fileName, groupingNode, "k_avgphytl", "For large phytoplankton", "", XML_TYPE_INTEGER, "1");
	Util_XML_Parse_Create_Node(fp, fileName, groupingNode, "k_avgphyts", "For small phytoplankton", "", XML_TYPE_INTEGER, "1");
	Util_XML_Parse_Create_Node(fp, fileName, groupingNode, "k_avgzoops", "For small zooplankton", "", XML_TYPE_INTEGER, "1");
	Util_XML_Parse_Create_Node(fp, fileName, groupingNode, "k_avgzoopl", "For large zooplankton", "", XML_TYPE_INTEGER, "1");
	Util_XML_Parse_Create_Node(fp, fileName, groupingNode, "k_avgcep", "For cephalopods", "", XML_TYPE_INTEGER, "1");
	Util_XML_Parse_Create_Node(fp, fileName, groupingNode, "k_avgpbact", "For pelagic bacteria", "", XML_TYPE_INTEGER, "1");
	Util_XML_Parse_Create_Node(fp, fileName, groupingNode, "k_avgsbact", "For sediment bacteria", "", XML_TYPE_INTEGER, "1");
	Util_XML_Parse_Create_Node(fp, fileName, groupingNode, "k_avginfs", "For small infauna (meiobenthos)", "", XML_TYPE_INTEGER, "1");
	Util_XML_Parse_Create_Node(fp, fileName, groupingNode, "k_avginfl", "For large infauna (deposit feeders and infaunal carnivores)", "", XML_TYPE_INTEGER, "1");
	Util_XML_Parse_Create_Node(fp, fileName, groupingNode, "k_avgepif1", "For epifauna type 1 (filter feeders and grazers)", "", XML_TYPE_INTEGER, "1");
	Util_XML_Parse_Create_Node(fp, fileName, groupingNode, "k_avgepif2", "For epifauna type 2 (macrozoobenthos)", "", XML_TYPE_INTEGER, "1");
	Util_XML_Parse_Create_Node(fp, fileName, groupingNode, "k_avgppben", "For benthic primary producers", "", XML_TYPE_INTEGER, "1");
	Util_XML_Parse_Create_Node(fp, fileName, groupingNode, "k_avgrdet", "For refractory detritus", "", XML_TYPE_INTEGER, "1");
	Util_XML_Parse_Create_Node(fp, fileName, groupingNode, "k_avgldet", "For labile detritus", "", XML_TYPE_INTEGER, "1");
	Util_XML_Parse_Create_Node(fp, fileName, groupingNode, "k_avgfish", "For fish and other vertebrate biomass", "", XML_TYPE_INTEGER, "1");
	Util_XML_Parse_Create_Node(fp, fileName, groupingNode, "k_avgpelpp", "For pelagic primary production", "", XML_TYPE_INTEGER, "1");
	Util_XML_Parse_Create_Node(fp, fileName, groupingNode, "k_avgzoopp", "For zooplankton production", "", XML_TYPE_INTEGER, "1");
	Util_XML_Parse_Create_Node(fp, fileName, groupingNode, "k_avgcepp", "For cephalopod production", "", XML_TYPE_INTEGER, "1");
	Util_XML_Parse_Create_Node(fp, fileName, groupingNode, "k_avgpbactp", "For pelagic bacterial production", "", XML_TYPE_INTEGER, "1");
	Util_XML_Parse_Create_Node(fp, fileName, groupingNode, "k_avgsbactp", "For sediment bacterial production", "", XML_TYPE_INTEGER, "1");
	Util_XML_Parse_Create_Node(fp, fileName, groupingNode, "k_avginfsp", "For small infauna (meiobenthos) production", "", XML_TYPE_INTEGER, "1");
	Util_XML_Parse_Create_Node(fp, fileName, groupingNode, "k_avginflp", "For large infauna (deposit feeders and infaunal carnivores) production", "", XML_TYPE_INTEGER, "1");
	Util_XML_Parse_Create_Node(fp, fileName, groupingNode, "k_avgepi1p", "For epifauna type 1(filter feeder) production", "", XML_TYPE_INTEGER, "1");
	Util_XML_Parse_Create_Node(fp, fileName, groupingNode, "k_avgepi2p", "For epifauna type 2 (grazers and macrozoobenthos) production", "", XML_TYPE_INTEGER, "1");
	Util_XML_Parse_Create_Node(fp, fileName, groupingNode, "k_avgbenpp", "For benthic primary production", "", XML_TYPE_INTEGER, "1");
	Util_XML_Parse_Create_Node(fp, fileName, groupingNode, "k_avgzoopeat", "For zooplankton consumption", "", XML_TYPE_INTEGER, "1");
	Util_XML_Parse_Create_Node(fp, fileName, groupingNode, "k_avgcepeat", "For cephalopod consumption", "", XML_TYPE_INTEGER, "1");
	Util_XML_Parse_Create_Node(fp, fileName, groupingNode, "k_avgpbacteat", "For pelagic bacterial consumption", "", XML_TYPE_INTEGER, "1");
	Util_XML_Parse_Create_Node(fp, fileName, groupingNode, "k_avgsbacteat", "For sediment bacterial consumption", "", XML_TYPE_INTEGER, "1");
	Util_XML_Parse_Create_Node(fp, fileName, groupingNode, "k_avginfseat", "For small infauna (meiobenthos) consumption", "", XML_TYPE_INTEGER, "1");
	Util_XML_Parse_Create_Node(fp, fileName, groupingNode, "k_avginfleat", "For large infauna (deposit feeders and infaunal carnivores) consumption", "", XML_TYPE_INTEGER, "1");
	Util_XML_Parse_Create_Node(fp, fileName, groupingNode, "k_avgepi1eat", "For epifauna type 1 (filter feeder) consumption", "", XML_TYPE_INTEGER, "1");
	Util_XML_Parse_Create_Node(fp, fileName, groupingNode, "k_avgepi2eat", "For epifauna type 2 (grazers and macrozoobenthos) consumption", "", XML_TYPE_INTEGER, "1");
	Util_XML_Parse_Create_Node(fp, fileName, groupingNode, "k_avgweight", "For vertebrate weights", "", XML_TYPE_INTEGER, "1");
	Util_XML_Parse_Create_Node(fp, fileName, groupingNode, "k_avgprodn", "For vertebrate production", "", XML_TYPE_INTEGER, "1");
	Util_XML_Parse_Create_Node(fp, fileName, groupingNode, "k_avgeat", "For vertebrate consumption", "", XML_TYPE_INTEGER, "1");
	Util_XML_Parse_Create_Node(fp, fileName, groupingNode, "k_avgdiscrd", "For vertebrate discard rates", "", XML_TYPE_INTEGER, "1");
	Util_XML_Parse_Create_Node(fp, fileName, groupingNode, "k_avgcatch", "For vertebrate total catch", "", XML_TYPE_INTEGER, "1");
	Util_XML_Parse_Create_Node(fp, fileName, groupingNode, "k_avgeffort", "For vertebrate total effort", "", XML_TYPE_INTEGER, "1");
	Util_XML_Parse_Create_Node(fp, fileName, groupingNode, "k_avgtdiscrd", "For vertebrate total discards", "", XML_TYPE_INTEGER, "1");
	Util_XML_Parse_Create_Node(fp, fileName, groupingNode, "k_avgcount", "For counts", "", XML_TYPE_INTEGER, "1");
	Util_XML_Parse_Create_Node(fp, fileName, groupingNode, "k_avgcobs", "For numbers observed in the catch", "", XML_TYPE_INTEGER, "1");
	Util_XML_Parse_Create_Node(fp, fileName, groupingNode, "k_avgcurve", "For selectivity curve used", "", XML_TYPE_INTEGER, "1");
	Util_XML_Parse_Create_Node(fp, fileName, groupingNode, "k_avgprms", "For parameters of selectivity curves", "", XML_TYPE_INTEGER, "1");
	Util_XML_Parse_Create_Node(fp, fileName, groupingNode, "k_avgage", "For aging", "", XML_TYPE_INTEGER, "1");
}

void ErrorVarianceXML(MSEBoxModel *bm, FILE *fp, char *fileName, xmlDocPtr doc, xmlNodePtr rootnode) {

	xmlNodePtr groupingNode;
	groupingNode = Util_XML_Create_Node(ATLANTIS_ATTRIBUTE_SUB_GROUP, rootnode, "Error_Variance", "Error variance (as percentage of mean)", "", "");

	Util_XML_Parse_Create_Node(fp, fileName, groupingNode, "k_varsalt", "For salinity", "", XML_TYPE_INTEGER, "1");
	Util_XML_Parse_Create_Node(fp, fileName, groupingNode, "k_varphys", "For physical properties (light, oxygen)", "", XML_TYPE_INTEGER, "1");
	Util_XML_Parse_Create_Node(fp, fileName, groupingNode, "k_varnuts", "For nutrients", "", XML_TYPE_INTEGER, "1");
	Util_XML_Parse_Create_Node(fp, fileName, groupingNode, "k_varprocs", "For processes (denitrification, nitrification) and chlorophyll a", "", XML_TYPE_INTEGER, "1");
	Util_XML_Parse_Create_Node(fp, fileName, groupingNode, "k_varphytl", "For large phytoplankton", "", XML_TYPE_INTEGER, "1");
	Util_XML_Parse_Create_Node(fp, fileName, groupingNode, "k_varphyts", "For small phytoplankton", "", XML_TYPE_INTEGER, "1");
	Util_XML_Parse_Create_Node(fp, fileName, groupingNode, "k_varzoops", "For small zooplankton", "", XML_TYPE_INTEGER, "1");
	Util_XML_Parse_Create_Node(fp, fileName, groupingNode, "k_varzoopl", "For large zooplankton", "", XML_TYPE_INTEGER, "1");
	Util_XML_Parse_Create_Node(fp, fileName, groupingNode, "k_varcep", "For cephalopods", "", XML_TYPE_INTEGER, "1");
	Util_XML_Parse_Create_Node(fp, fileName, groupingNode, "k_varpbact", "For pelagic bacteria", "", XML_TYPE_INTEGER, "1");
	Util_XML_Parse_Create_Node(fp, fileName, groupingNode, "k_varsbact", "For sediment bacteria", "", XML_TYPE_INTEGER, "1");
	Util_XML_Parse_Create_Node(fp, fileName, groupingNode, "k_varinfs", "For small infauna (meiobenthos)", "", XML_TYPE_INTEGER, "1");
	Util_XML_Parse_Create_Node(fp, fileName, groupingNode, "k_varinfl", "For large infauna (deposit feeders and infaunal carnivores)", "", XML_TYPE_INTEGER, "1");
	Util_XML_Parse_Create_Node(fp, fileName, groupingNode, "k_varepif1", "For epifauna type 1 (filter feeders and grazers)", "", XML_TYPE_INTEGER, "1");
	Util_XML_Parse_Create_Node(fp, fileName, groupingNode, "k_varepif2", "For epifauna type 2 (macrozoobenthos)", "", XML_TYPE_INTEGER, "1");
	Util_XML_Parse_Create_Node(fp, fileName, groupingNode, "k_varppben", "For benthic primary producers", "", XML_TYPE_INTEGER, "1");
	Util_XML_Parse_Create_Node(fp, fileName, groupingNode, "k_varrdet", "For refractory detritus", "", XML_TYPE_INTEGER, "1");
	Util_XML_Parse_Create_Node(fp, fileName, groupingNode, "k_varldet", "For labile detritus", "", XML_TYPE_INTEGER, "1");
	Util_XML_Parse_Create_Node(fp, fileName, groupingNode, "k_varfish", "For fish and other vertebrate biomass", "", XML_TYPE_INTEGER, "1");
	Util_XML_Parse_Create_Node(fp, fileName, groupingNode, "k_varpelpp", "For pelagic primary production", "", XML_TYPE_INTEGER, "1");
	Util_XML_Parse_Create_Node(fp, fileName, groupingNode, "k_varzoopp", "For zooplankton production", "", XML_TYPE_INTEGER, "1");
	Util_XML_Parse_Create_Node(fp, fileName, groupingNode, "k_varcepp", "For cephalopod production", "", XML_TYPE_INTEGER, "1");
	Util_XML_Parse_Create_Node(fp, fileName, groupingNode, "k_varpbactp", "For pelagic bacterial production", "", XML_TYPE_INTEGER, "1");
	Util_XML_Parse_Create_Node(fp, fileName, groupingNode, "k_varsbactp", "For sediment bacterial production", "", XML_TYPE_INTEGER, "1");
	Util_XML_Parse_Create_Node(fp, fileName, groupingNode, "k_varinfsp", "For small infauna (meiobenthos) production", "", XML_TYPE_INTEGER, "1");
	Util_XML_Parse_Create_Node(fp, fileName, groupingNode, "k_varinflp", "For large infauna (deposit feeders and infaunal carnivores) production", "", XML_TYPE_INTEGER, "1");
	Util_XML_Parse_Create_Node(fp, fileName, groupingNode, "k_varepi1p", "For epifauna type 1 (filter feeder) production", "", XML_TYPE_INTEGER, "1");
	Util_XML_Parse_Create_Node(fp, fileName, groupingNode, "k_varepi2p", "For epifauna type 2 (grazers and macrozoobenthos) production", "", XML_TYPE_INTEGER, "1");
	Util_XML_Parse_Create_Node(fp, fileName, groupingNode, "k_varbenpp", "For benthic primary production", "", XML_TYPE_INTEGER, "1");
	Util_XML_Parse_Create_Node(fp, fileName, groupingNode, "k_varzoopeat", "For zooplankton consumption", "", XML_TYPE_INTEGER, "1");
	Util_XML_Parse_Create_Node(fp, fileName, groupingNode, "k_varcepeat", "For cephalopod consumption", "", XML_TYPE_INTEGER, "1");
	Util_XML_Parse_Create_Node(fp, fileName, groupingNode, "k_varpbacteat", "For pelagic bacterial consumption", "", XML_TYPE_INTEGER, "1");
	Util_XML_Parse_Create_Node(fp, fileName, groupingNode, "k_varsbacteat", "For sediment bacterial consumption", "", XML_TYPE_INTEGER, "1");
	Util_XML_Parse_Create_Node(fp, fileName, groupingNode, "k_varinfseat", "For small infauna (meiobenthos) consumption", "", XML_TYPE_INTEGER, "1");
	Util_XML_Parse_Create_Node(fp, fileName, groupingNode, "k_varinfleat", "For large infauna (deposit feeders and infaunal carnivores) consumption", "", XML_TYPE_INTEGER, "1");
	Util_XML_Parse_Create_Node(fp, fileName, groupingNode, "k_varepi1eat", "For epifauna type 1 (filter feeder) consumption", "", XML_TYPE_INTEGER, "1");
	Util_XML_Parse_Create_Node(fp, fileName, groupingNode, "k_varepi2eat", "For epifauna type 2 (grazers and macrozoobenthos) consumption", "", XML_TYPE_INTEGER, "1");
	Util_XML_Parse_Create_Node(fp, fileName, groupingNode, "k_varweight", "For vertebrate weights", "", XML_TYPE_INTEGER, "1");
	Util_XML_Parse_Create_Node(fp, fileName, groupingNode, "k_varprodn", "For vertebrate production", "", XML_TYPE_INTEGER, "1");
	Util_XML_Parse_Create_Node(fp, fileName, groupingNode, "k_vareat", "For vertebrate consumption", "", XML_TYPE_INTEGER, "1");
	Util_XML_Parse_Create_Node(fp, fileName, groupingNode, "k_vardiscrd", "For vertebrate discards", "", XML_TYPE_INTEGER, "1");
	Util_XML_Parse_Create_Node(fp, fileName, groupingNode, "k_varcatch", "For vertebrate total catch", "", XML_TYPE_INTEGER, "1");
	Util_XML_Parse_Create_Node(fp, fileName, groupingNode, "k_vareffort", "For vertebrate total effort", "", XML_TYPE_INTEGER, "1");
	Util_XML_Parse_Create_Node(fp, fileName, groupingNode, "k_vartdiscrd", "For vertebrate total discards", "", XML_TYPE_INTEGER, "1");
	Util_XML_Parse_Create_Node(fp, fileName, groupingNode, "k_varcount", "For counts", "", XML_TYPE_INTEGER, "1");
	Util_XML_Parse_Create_Node(fp, fileName, groupingNode, "k_varcobs", "For numbers observed in the catch", "", XML_TYPE_INTEGER, "1");
	Util_XML_Parse_Create_Node(fp, fileName, groupingNode, "k_varcurve", "For selectivity curve used", "", XML_TYPE_INTEGER, "1");
	Util_XML_Parse_Create_Node(fp, fileName, groupingNode, "k_varprms", "For parameters of selectivity curves", "", XML_TYPE_INTEGER, "1");
	Util_XML_Parse_Create_Node(fp, fileName, groupingNode, "k_varage", "For aging", "", XML_TYPE_INTEGER, "1");
}

void VertParamsXML(MSEBoxModel *bm, FILE *fp, char *fileName, xmlDocPtr doc, xmlNodePtr rootnode) {

	xmlNodePtr groupingNode;
	groupingNode = Util_XML_Create_Node(ATLANTIS_ATTRIBUTE_SUB_GROUP, rootnode, "Size_Parameters", "", "", "");

	Parse_File(bm, fp, fileName, groupingNode, "SampleSize", "samplesize", "Sample sizes for catch sub-samplings", "", XML_TYPE_INTEGERARRAY, bm->K_num_tot_sp, TRUE,
			Init_Vert_Species_Zero_ValuesXML, createVertebrateSpeciesParam);
	Parse_File(bm, fp, fileName, groupingNode, "Allometic_Coefficient", "li_a", "Coefficient of allometic length-weight relation", "", XML_TYPE_FLOATARRAY, bm->K_num_tot_sp, TRUE,
			Init_Vert_Species_Zero_ValuesXML, createVertebrateSpeciesParam);
	Parse_File(bm, fp, fileName, groupingNode, "Allometic_Exponent", "li_b$", "Exponent in allometic length-weight relation", "", XML_TYPE_FLOATARRAY, bm->K_num_tot_sp, TRUE,
			Init_Vert_Species_Zero_ValuesXML, createVertebrateSpeciesParam);

	Parse_File(bm, fp, fileName, groupingNode, "Allometic_Bin_Size", "li_bin", "Size (cm) of the size (length) bins", "cm", XML_TYPE_FLOATARRAY, bm->K_num_tot_sp, TRUE,
			Init_Vert_Species_Zero_ValuesXML, createVertebrateSpeciesParam);

	Parse_File(bm, fp, fileName, groupingNode, "Allometic_Bin_Start", "li_start", "Size (cm) of smallest size bin", "cm", XML_TYPE_FLOATARRAY, bm->K_num_tot_sp, TRUE,
			Init_Vert_Species_Zero_ValuesXML, createVertebrateSpeciesParam);

	Parse_File(bm, fp, fileName, groupingNode, "Max_Length", "li_max", "Maximum length of each harvested group (cm) - vertebrates and invertebrates", "cm", XML_TYPE_FLOATARRAY,  bm->K_num_tot_sp,
			TRUE, Create_Impacted_Species_XMLNodes, createImpactedSpeciesParam);
}

void assessFishingSelectivityXML(MSEBoxModel *bm, FILE *fp, char *fileName, xmlDocPtr doc, xmlNodePtr rootnode) {

	xmlNodePtr groupingNode, node, lookupNode;
	groupingNode = Util_XML_Create_Node(ATLANTIS_ATTRIBUTE_SUB_GROUP, rootnode, "Fishing_Selectivity", "Fishing selectivity information for each fishery", "",
			"");

	Parse_File(bm, fp, fileName, groupingNode, "Population_Half_Length", "q_lsm", "Length half the population is selected", "", XML_TYPE_FLOATARRAY, bm->K_num_fisheries, TRUE, Create_Fishery_XMLNodes,
			createFisheriesParam);

	Parse_File(bm, fp, fileName, groupingNode, "Selectivity_Spread", "q_sigma", "Spread of the selectivity curve", "", XML_TYPE_FLOATARRAY, bm->K_num_fisheries, TRUE, Create_Fishery_XMLNodes,
			createFisheriesParam);

	node = Parse_File(bm, fp, fileName, groupingNode, "Selectivity_Option", "selcurve", "flag for selectivity curve", "", XML_TYPE_FLOATARRAY, bm->K_num_fisheries, TRUE, Create_Fishery_XMLNodes,
			createFisheriesParam);

	lookupNode = Util_XML_Create_Lookup_Node(node);
	Util_XML_Create_Lookup_Value_Node(lookupNode, "0", "Use constant selectivity");
	Util_XML_Create_Lookup_Value_Node(lookupNode, "1", "use logistic selectivity");
	Util_XML_Create_Lookup_Value_Node(lookupNode, "2", "use normal selectivity");
	Util_XML_Create_Lookup_Value_Node(lookupNode, "3", "use lognormal selectivity");
	Util_XML_Create_Lookup_Value_Node(lookupNode, "4", "use gamma selectivity");

	Create_Harvest_Fishery_Group_ParamXML(bm, fp, fileName, groupingNode, avail_id,
			"Availabilty (acessibility of stock) of fish, per fishery", "", XML_TYPE_FLOAT);


}

void detritusXML(MSEBoxModel *bm, FILE *fp, char *fileName, xmlDocPtr doc, xmlNodePtr rootnode) {

	xmlNodePtr groupingNode;
	groupingNode = Util_XML_Create_Node(ATLANTIS_ATTRIBUTE_SUB_GROUP, rootnode, "Detritus_Input", "Detritus inputs to each zone", "", "");

	Util_XML_Create_Node_Next_Line(fp, fileName, groupingNode, "input_DLwc", "Water column DL input into each zone.", "", XML_TYPE_FLOAT);
	Util_XML_Create_Node_Next_Line(fp, fileName, groupingNode, "input_DRwc", "Water colume DR input into each zone.", "", XML_TYPE_FLOAT);
	Util_XML_Create_Node_Next_Line(fp, fileName, groupingNode, "input_DLsed", "Sediment DL input into each zone.", "", XML_TYPE_FLOAT);
	Util_XML_Create_Node_Next_Line(fp, fileName, groupingNode, "input_DRsed", "Sediment DR input into each zone.", "", XML_TYPE_FLOAT);

	Parse_File(bm, fp, fileName, groupingNode, "Discard_Fate", "Discard_Fate", "Proportion of discards flowing to each of the detritus pools", "", XML_TYPE_FLOATARRAY, -1, TRUE,
			InitDiscardEffortXML, createDiscardFateSpeciesParam);

	Parse_File(bm, fp, fileName, groupingNode, "R_max", "R_max", "PBR (Potential Biological Removals) parameter for each vertebrate and cephalopods", "", XML_TYPE_FLOATARRAY,
			bm->K_num_tot_sp, TRUE, Init_Species_Zero_ValuesXML, createVertCEPSpeciesParam);

	Parse_File(bm, fp, fileName, groupingNode, "avg_inv_size", "avg_inv_size", "Average invertebrate length (mm). Zero values for vertebrates.", "mm", XML_TYPE_FLOATARRAY,
			bm->K_num_tot_sp, TRUE, Init_Species_Zero_ValuesXML, createLiveInvertsSpeciesParam);
}

/* Parameters for R based Assessments */
void RAssessFlagXML(MSEBoxModel *bm, FILE *fp, char *fileName, xmlDocPtr doc, xmlNodePtr rootnode) {

    int nf;
    char nameString[BMSLEN];
    xmlNodePtr groupingNode, childGroupingNode;
    groupingNode = Util_XML_Create_Node(ATLANTIS_ATTRIBUTE_SUB_GROUP, rootnode, "RAssess_Params", "Parameters for R based Assessments", "", "");

    Util_XML_Parse_Create_Node(fp, fileName, groupingNode, "UsingRAssess", "Whether assessments is using RAssess calls", "", XML_TYPE_BOOLEAN,"0");
    bm->UsingRAssess = (int) (Util_XML_Read_Value(fileName, ATLANTIS_ATTRIBUTE,  bm->ecotest, 1, groupingNode, binary_check, "UsingRAssess"));
    
    if (bm->UsingRAssess) {
        Util_XML_Parse_Create_Node(fp, fileName, groupingNode, "RAssessFisheryID", "Fishery doing RAssess assessment survey","", XML_TYPE_INTEGER,"0");
        Util_XML_Parse_Create_Node(fp, fileName, groupingNode, "RAssessSpringSurveyDay", "Time of year of RAssess spring survey","", XML_TYPE_INTEGER,"240");
        Util_XML_Parse_Create_Node(fp, fileName, groupingNode, "RAssessAutumnSurveyDay", "Time of year of RAssess autumn survey","", XML_TYPE_INTEGER,"60");
        Util_XML_Parse_Create_Node(fp, fileName, groupingNode, "RAssessFixedM", "Flag ndicating whether using fixed M in RAssess assessment","", XML_TYPE_BOOLEAN,"1");
        Util_XML_Parse_Create_Node(fp, fileName, groupingNode, "RAssessRefYear", "First year of RAssess output file","", XML_TYPE_INTEGER,"60");
        
        Util_XML_Parse_Create_Node(fp, fileName, groupingNode, "RAssessRuseScript", "Whether using R script (1) or embeeded R (0)","", XML_TYPE_INTEGER,"0");
        bm->RAssessRuseScript = (int)Util_XML_Read_Value(fileName, ATLANTIS_ATTRIBUTE,  bm->ecotest, 1, groupingNode, integer_check, "RAssessRuseScript");
        
        Util_XML_Parse_Create_Node(fp, fileName, groupingNode, "RassessColDelimiter", "Column delimiter", "", XML_TYPE_STRING, "");
        Util_XML_Parse_Create_Node(fp, fileName, groupingNode, "RAssessRinName", "Base of name for RAssess input files", "", XML_TYPE_STRING, "");
        Util_XML_Parse_Create_Node(fp, fileName, groupingNode, "RAssessRoutName", "Base of name for RAssess output files", "", XML_TYPE_STRING, "");
        
        Util_XML_Parse_Create_Node(fp, fileName, groupingNode, "NumRAssessScriptNames", "Number of R scripts being used in RAssess calls", "", XML_TYPE_BOOLEAN,"0");
        bm->NumRAssessScriptNames = (int)Util_XML_Read_Value(fileName, ATLANTIS_ATTRIBUTE,  bm->ecotest, 1, groupingNode, integer_check, "NumRAssessScriptNames");
        
        for (nf = 0; nf < bm->NumRAssessScriptNames; nf ++) {
            snprintf(nameString, sizeof(nameString), "RAssessRscriptName%d", nf);
            Util_XML_Parse_Create_Node(fp, fileName, groupingNode, nameString, "Base of name for RAssess R script", "", XML_TYPE_STRING, "");
        }
        childGroupingNode = Util_XML_Create_Node(ATLANTIS_ATTRIBUTE_SUB_GROUP, groupingNode, "speciesRAssess", "speciesRAssess", "", "");
        Create_Species_ParamXML(bm, fileName, fp, childGroupingNode, whichRAssess_id, "Which RAssess R script to use per species", "", XML_TYPE_INTEGER, "-1");
    }
}


/************************************************************************************** Tiered assessment related routines ****************************************************************/

/**
 *	\brief Create the XML structure to hold the stock by sex parameters.
 *
 *
 *	<paramX>
 *		<FXX>
 *			<sex0>
 *				<stock1>
 *				</stock1>
 *			</sex0>
 *		</FXX>
 *
 *	</paramX>
 *
 */
void InitAssessStockAgeXML(MSEBoxModel *bm, xmlNodePtr parent) {
    
	xmlNodePtr groupNode;
	xmlNodePtr sexNode;
	int guild, ns, stock;
	char str[50];
    
	/* Create a node for each functional group*/
	for (guild = 0; guild < bm->K_num_tot_sp; guild++) {
		if (FunctGroupArray[guild].isFished == TRUE) {
			groupNode = Util_XML_Create_Node(ATLANTIS_GROUP_ATTRIBUTE, parent, FunctGroupArray[guild].groupCode, "", "", "");
            
			for (ns = 0; ns < bm->K_num_sexes; ns++) {
				snprintf(str, sizeof(str), "%s", sexStrings[ns]);
				sexNode = Util_XML_Create_Node(ATLANTIS_SEX_ATTRIBUTE, groupNode, str, "", "", "");
                
				for (stock = 0; stock < FunctGroupArray[guild].numStocks; stock++) {
					snprintf(str, sizeof(str), "stock%d", stock + 1);
					Util_XML_Create_Node(ATLANTIS_ATTRIBUTE, sexNode, str, "", "", "");
				}
			}
		}
	}
}

void AssessStockXMLFunction(MSEBoxModel *bm, char *fileName, xmlNodePtr parent, char *str, char *valueStr) {
    
    char *lastLetter;
	char *secondlastLetter;
    char *sexnum;
	char strPtr[10];
	char speciesStr[STRLEN];
	char paramStr[STRLEN];
	char step1Str[STRLEN];
	char attributeName[STRLEN];
	int i, stockIndex, speciesIndex, ns;
	xmlNodePtr speciesNode;
	xmlNodePtr sexNode;
	//xmlNodePtr paramNode;
    char keys[] = "1234567890";
    
    strcpy(speciesStr, str);
    strcpy(paramStr, str);
	strcpy(step1Str, str);
	lastLetter = step1Str + strlen(step1Str) - 1;
	secondlastLetter = step1Str + strlen(step1Str) - 2;
    
    // Assumes string has the form FXXi_STRStockj where STR is the parameter name, i is the sex (either 0 or 1) and j is the stock number 1+
    i = (int)strcspn (speciesStr, keys);  // location of first number
    sexnum = strpbrk (speciesStr, keys);
    ns = atoi(sexnum);
    strncpy ( strPtr, speciesStr, (size_t)i);
	strPtr[i] = '\0';  // null character added manually
	speciesIndex = Util_Get_FG_Index_From_Token(bm, strPtr, fileName, str, TRUE);

	if (strstr(str, "Mzero") != NULL) {
		snprintf(paramStr, sizeof(paramStr), "Mzero");
	} else if (strstr(str, "MeanLenAge") != NULL) {
		snprintf(paramStr, sizeof(paramStr), "MeanLenAge");
	} else if (strstr(str, "SigmaLenAge") != NULL) {
		snprintf(paramStr, sizeof(paramStr), "SigmaLenAge");
	}
    
	// Find stock index
	if(isdigit(secondlastLetter[0])) {  // Is the first digit a number if yes then a two digit case
		stockIndex = atoi(secondlastLetter);
	} else {  // No make it a single string case
		stockIndex = atoi(lastLetter);
	}

	//paramNode = Util_XML_Get_Or_Create_Node(ATLANTIS_ATTRIBUTE, parent, paramStr);  as done in InitAssessStockAgeXML()
	speciesNode = Util_XML_Get_Or_Create_Node(ATLANTIS_GROUP_ATTRIBUTE, parent, FunctGroupArray[speciesIndex].groupCode);
	sexNode = Util_XML_Get_Or_Create_Node(ATLANTIS_SEX_ATTRIBUTE, speciesNode, sexStrings[ns]);
    
    snprintf(attributeName, sizeof(attributeName), "stock%d", stockIndex);
	Util_XML_Set_Node_Value(ATLANTIS_ATTRIBUTE, sexNode, attributeName, valueStr);
    
}

/**
 *	\brief Create the XML structure to hold the stock by sex parameters.
 *
 *
 *	<paramX>
 *		<FXX>
 *			<stock1>
 *			</stock1>
 *		</FXX>
 *
 *	</paramX>
 *
 */
void InitAssessSexXML(MSEBoxModel *bm, xmlNodePtr parent) {
    
	xmlNodePtr groupNode;
	int guild, stock;
	char str[50];
    
	/* Create a node for each functional group*/
	for (guild = 0; guild < bm->K_num_tot_sp; guild++) {
		if (FunctGroupArray[guild].isFished == TRUE) {
			groupNode = Util_XML_Create_Node(ATLANTIS_GROUP_ATTRIBUTE, parent, FunctGroupArray[guild].groupCode, "", "", "");
            
			for (stock = 0; stock < FunctGroupArray[guild].numStocks; stock++) {
                snprintf(str, sizeof(str), "stock%d", stock + 1);
                Util_XML_Create_Node(ATLANTIS_ATTRIBUTE, groupNode, str, "", "", "");
			}
		}
	}
}

void AssessSexXMLFunction(MSEBoxModel *bm, char *fileName, xmlNodePtr parent, char *str, char *valueStr) {
    
	char *lastLetter;
	char *secondlastLetter;
    char strPtr[10];
    char speciesStr[STRLEN];
	char paramStr[STRLEN];
	char step1Str[STRLEN];
	char attributeName[STRLEN];
	int i, stockIndex, speciesIndex;
	xmlNodePtr speciesNode;
	//xmlNodePtr paramNode;
    char keys[] = "_";
    
    strcpy(speciesStr, str);
	strcpy(paramStr, str);
	strcpy(step1Str, str);
	lastLetter = step1Str + strlen(step1Str) - 1;
	secondlastLetter = step1Str + strlen(step1Str) - 2;
    
    // Assumes string has the form FXX_STRStockj where STR is the parameter name and j is the stock number 1+
    i = (int)strcspn (speciesStr, keys);  // location of first number
    strncpy ( strPtr, speciesStr, (size_t)i );
	strPtr[i] = '\0';  // null character added manually
	speciesIndex = Util_Get_FG_Index_From_Token(bm, strPtr, fileName, str, TRUE);
    
	if (strstr(str, "CvLA0") != NULL) {
		snprintf(paramStr, sizeof(paramStr), "CvLA0");
	} else if (strstr(str, "CvLAmax") != NULL) {
		snprintf(paramStr, sizeof(paramStr), "CvLAmax");
	} else if (strstr(str, "Wtlena") != NULL) {
		snprintf(paramStr, sizeof(paramStr), "Wtlena");
	} else if (strstr(str, "Wtlenb") != NULL) {
		snprintf(paramStr, sizeof(paramStr), "Wtlenb");
	} else if (strstr(str, "Fecund") != NULL) {
		snprintf(paramStr, sizeof(paramStr), "Fecund");
	} else if (strstr(str, "VBLinf") != NULL) {
		snprintf(paramStr, sizeof(paramStr), "VBLinf");
	} else if (strstr(str, "VBk") != NULL) {
		snprintf(paramStr, sizeof(paramStr), "VBk");
	} else if (strstr(str, "VBt0") != NULL) {
		snprintf(paramStr, sizeof(paramStr), "VBt0");
	} else if (strstr(str, "SSMort") != NULL) {
		snprintf(paramStr, sizeof(paramStr), "SSMort");
	}
    
	// Find stock index
	if(isdigit(secondlastLetter[0])) {  // Is the first digit a number if yes then a two digit case
		stockIndex = atoi(secondlastLetter);
	} else {  // No make it a single string case
		stockIndex = atoi(lastLetter);
	}
    
	//paramNode = Util_XML_Get_Or_Create_Node(ATLANTIS_ATTRIBUTE, parent, paramStr);
	speciesNode = Util_XML_Get_Or_Create_Node(ATLANTIS_GROUP_ATTRIBUTE, parent, FunctGroupArray[speciesIndex].groupCode);
    snprintf(attributeName, sizeof(attributeName), "stock%d", stockIndex);
	Util_XML_Set_Node_Value(ATLANTIS_ATTRIBUTE, speciesNode, attributeName, valueStr);
    
}

/**
 *	\brief Create the XML structure to hold the stock assessment (SS3) parameters.
 *
 */
void TierAssessmentXML(MSEBoxModel *bm, FILE *fp, char *fileName, xmlDocPtr doc, xmlNodePtr rootnode) {
	xmlNodePtr groupingNode, childGroupingNode;
    int nf;
    char nameString[BMSLEN];

	if (bm->useRBCTiers == FALSE)
		return;

	groupingNode = Util_XML_Create_Node(ATLANTIS_ATTRIBUTE_SUB_GROUP, rootnode, "Tier_Assessment", "Tier Assessment Parameters", "", "");

	Util_XML_Parse_Create_Node(fp, fileName, groupingNode, "assess_share_params", "Whether all stocks assessed for an individual species share same parameters.", "", XML_TYPE_BOOLEAN, "0");
	Util_XML_Get_Value_Integer(fileName, ATLANTIS_ATTRIBUTE, bm->ecotest, 1, groupingNode, binary_check, "assess_share_params", &bm->assess_share_params);
    
    Util_XML_Parse_Create_Node(fp, fileName, groupingNode, "Num_Sexes", "Number of sexes in the model.", "", XML_TYPE_BOOLEAN, "0");
	Util_XML_Get_Value_Integer(fileName, ATLANTIS_ATTRIBUTE, bm->ecotest, 1, groupingNode, binary_check, "Num_Sexes", &bm->K_num_sexes);

    Util_XML_Parse_Create_Node(fp, fileName, groupingNode, "UseAtlantisPGMSY", "Flag indicating whether using Atlantis version of PGMSY (1) or R version (0)", "", XML_TYPE_BOOLEAN, "0");
    Util_XML_Parse_Create_Node(fp, fileName, groupingNode, "UseSS", "Flag indicating whether using SS for Tier 1 (1) or if using perfect info + error (0)", "", XML_TYPE_BOOLEAN, "0");
    Util_XML_Parse_Create_Node(fp, fileName, groupingNode, "AssessDelay", "Length of delay in the assessment process (years from data collection to RBC setting)", "", XML_TYPE_INTEGER, "2");
    Util_XML_Parse_Create_Node(fp, fileName, groupingNode, "UseTierBuffers", "Flag indicating whether using US tier scalars (1) or not (0)", "", XML_TYPE_BOOLEAN, "0");
    Util_XML_Parse_Create_Node(fp, fileName, groupingNode, "myTACbuffer", "Scalar applied to RBC in subsequent years of MYTAC multiyear TAC", "", XML_TYPE_FLOAT, "0");
    Util_XML_Parse_Create_Node(fp, fileName, groupingNode, "myTACperiod", "How long MYTAC multiyear TAC lasts", "", XML_TYPE_INTEGER, "0");
    Util_XML_Parse_Create_Node(fp, fileName, groupingNode, "sexratio_cv", "sexratio female component", "", XML_TYPE_FLOAT, "0");
    
    childGroupingNode = Util_XML_Create_Node(ATLANTIS_ATTRIBUTE_SUB_GROUP, groupingNode, "Seeds", "Seeds", "", "");
	Util_XML_Parse_Create_Node(fp, fileName, childGroupingNode, "Iseedz",
			"Seed for operating model selectivity. Set to 0 if want new random no stream for each run", "", XML_TYPE_FLOAT, "0");
	Util_XML_Parse_Create_Node(fp, fileName, childGroupingNode, "Iseedx", "Seed for sigmaRs", "", XML_TYPE_FLOAT, "0");
	Util_XML_Parse_Create_Node(fp, fileName, childGroupingNode, "Iseedy", "Seed for fake Tier 1", "", XML_TYPE_FLOAT, "0");

    Util_XML_Parse_Create_Node(fp, fileName, groupingNode, "SSForecastType", "SS Forecast type", "", XML_TYPE_INTEGER, "0");
    Util_XML_Parse_Create_Node(fp, fileName, groupingNode, "SSFscalar", "SS F scalar", "", XML_TYPE_FLOAT, "0");
	Util_XML_Parse_Create_Node(fp, fileName, groupingNode, "SSControlRule", "SS Control Rule method", "", XML_TYPE_INTEGER, "0");
    Util_XML_Parse_Create_Node(fp, fileName, groupingNode, "SSControlRuleB", "SS Control Rule biomass", "", XML_TYPE_FLOAT, "0");
    Util_XML_Parse_Create_Node(fp, fileName, groupingNode, "SSControlRuleTargF", "SS Control Rule target F", "", XML_TYPE_FLOAT, "0");
	Util_XML_Parse_Create_Node(fp, fileName, groupingNode, "SSLoop", "SS forecast loops", "", XML_TYPE_INTEGER, "0");
	Util_XML_Parse_Create_Node(fp, fileName, groupingNode, "SSLoopWithRandRec", "SS forecast loop where stochastic ercruitment introduced", "", XML_TYPE_INTEGER, "0");
    
    Util_XML_Parse_Create_Node(fp, fileName, groupingNode, "SSTol", "SS convergence tolerance", "", XML_TYPE_FLOAT, "0");
	Util_XML_Parse_Create_Node(fp, fileName, groupingNode, "SSDepletionBasis", "SS Depletion basis", "", XML_TYPE_INTEGER, "0");
    Util_XML_Parse_Create_Node(fp, fileName, groupingNode, "SSFractX", "SS Depletion denominator", "", XML_TYPE_FLOAT, "0");
	Util_XML_Parse_Create_Node(fp, fileName, groupingNode, "SS_SPRreport", "SS SPR reporting flag", "", XML_TYPE_INTEGER, "0");
	Util_XML_Parse_Create_Node(fp, fileName, groupingNode, "SS_Freport", "SS F reporting flag", "", XML_TYPE_INTEGER, "0");
	Util_XML_Parse_Create_Node(fp, fileName, groupingNode, "SSFreportBasis", "SS F reporting basis", "", XML_TYPE_INTEGER, "0");
	Util_XML_Parse_Create_Node(fp, fileName, groupingNode, "SSnoBiasAdj", "SS last year of no bias adjustment - relative to model start", "", XML_TYPE_INTEGER, "0");
	Util_XML_Parse_Create_Node(fp, fileName, groupingNode, "SSnumGrowthPatterns", "SS Number of growth patterns", "", XML_TYPE_INTEGER, "0");
   
    Util_XML_Parse_Create_Node(fp, fileName, groupingNode, "Nmetiers", "Active metiers in assessment vs operating model", "", XML_TYPE_INTEGER, "1");
    
	Util_XML_Parse_Create_Node(fp, fileName, groupingNode, "useGenMnomial", "Whether using useGenMnomial for length/age data generation (1) yes (0) no", "", XML_TYPE_BOOLEAN,"0");
	Util_XML_Parse_Create_Node(fp, fileName, groupingNode, "CPUElim", "CPUE limit reference point as a proportion of target reference point", "", XML_TYPE_FLOAT, "0");
	Util_XML_Parse_Create_Node(fp, fileName, groupingNode, "num_avg_year", "Number of years over which to average prop of catch", "", XML_TYPE_FLOAT, "0");

	Util_XML_Parse_Create_Node(fp, fileName, groupingNode, "Tier1_cv", "Tier 1 CV", "", XML_TYPE_FLOAT, "0");
    
    // Tier 3 rule parameters
	Util_XML_Parse_Create_Node(fp, fileName, groupingNode, "tier3RuleA", "tier3RuleA", "", XML_TYPE_FLOAT, "0");
	Util_XML_Parse_Create_Node(fp, fileName, groupingNode, "tier3RuleB", "tier3RuleB", "", XML_TYPE_FLOAT, "0");
	Util_XML_Parse_Create_Node(fp, fileName, groupingNode, "tier3RuleC", "tier3RuleC", "", XML_TYPE_FLOAT, "0");
	Util_XML_Parse_Create_Node(fp, fileName, groupingNode, "tier3RuleD", "tier3RuleD", "", XML_TYPE_FLOAT, "0");
	Util_XML_Parse_Create_Node(fp, fileName, groupingNode, "tier3RuleE", "tier3RuleE", "", XML_TYPE_FLOAT, "0");
	Util_XML_Parse_Create_Node(fp, fileName, groupingNode, "tier3RuleF", "tier3RuleF", "", XML_TYPE_FLOAT, "0");

	Util_XML_Parse_Create_Node(fp, fileName, groupingNode, "tier5JensenConst", "tier5JensenConst", "", XML_TYPE_FLOAT, "0");
	Util_XML_Parse_Create_Node(fp, fileName, groupingNode, "tier5limscalar", "tier5limscalar", "", XML_TYPE_FLOAT, "0");
	Util_XML_Parse_Create_Node(fp, fileName, groupingNode, "tier5omega", "tier5omega", "", XML_TYPE_FLOAT, "0");
	Util_XML_Parse_Create_Node(fp, fileName, groupingNode, "tier5DCACscalar", "tier5DCACscalar", "", XML_TYPE_FLOAT, "0");
	Util_XML_Parse_Create_Node(fp, fileName, groupingNode, "tier5DCACconst", "tier5DCACconst", "", XML_TYPE_FLOAT, "0");
    
	Util_XML_Parse_Create_Node(fp, fileName, groupingNode, "tier7areachange", "tier7areachange", "", XML_TYPE_FLOAT, "0");
	Util_XML_Parse_Create_Node(fp, fileName, groupingNode, "tier7relarea", "tier7relarea", "", XML_TYPE_FLOAT, "0");
	Util_XML_Parse_Create_Node(fp, fileName, groupingNode, "tier7cpuechange", "tier7cpuechange", "", XML_TYPE_FLOAT, "0");
	Util_XML_Parse_Create_Node(fp, fileName, groupingNode, "tier7maxcompchange", "tier7maxcompchange", "", XML_TYPE_FLOAT, "0");
	Util_XML_Parse_Create_Node(fp, fileName, groupingNode, "tier7maxannchange", "tier7maxannchange", "", XML_TYPE_FLOAT, "0");
	Util_XML_Parse_Create_Node(fp, fileName, groupingNode, "tier7period", "tier7period", "", XML_TYPE_FLOAT, "0");
    
	Util_XML_Parse_Create_Node(fp, fileName, groupingNode, "RBC_TACchange_min", "RBC_TACchange_min", "", XML_TYPE_FLOAT, "0");
	Util_XML_Parse_Create_Node(fp, fileName, groupingNode, "Use_TAC_mult", "Use_TAC_mult", "", XML_TYPE_BOOLEAN, "0");
	Util_XML_Parse_Create_Node(fp, fileName, groupingNode, "HistYrMin", "First historical year", "", XML_TYPE_INTEGER, "0");
	Util_XML_Parse_Create_Node(fp, fileName, groupingNode, "HistYrMax", "Last historical year", "", XML_TYPE_INTEGER, "0");
	Util_XML_Parse_Create_Node(fp, fileName, groupingNode, "OverallMaxAge", "Greatest max age over all species", "", XML_TYPE_INTEGER, "0");
    
	Util_XML_Parse_Create_Node(fp, fileName, groupingNode, "tier6_to_tier4", "Switch to tier 4 from tier 6 once triggered (1) or to tier 3 (0)", "", XML_TYPE_INTEGER, "0");
	Util_XML_Parse_Create_Node(fp, fileName, groupingNode, "delTAE", "Delta E for empirical CPUE rule used with tier 6", "", XML_TYPE_FLOAT, "0");
    
    // Ratpack parameters
    Util_XML_Parse_Create_Node(fp, fileName, groupingNode, "MULTISPestNspecies", "Multispecies assessment number of species involved", "", XML_TYPE_INTEGER, "1");
    Util_XML_Parse_Create_Node(fp, fileName, groupingNode, "nRBCaverage", "Period over which to avaerage the RBC", "", XML_TYPE_INTEGER, "5");
    Util_XML_Parse_Create_Node(fp, fileName, groupingNode, "MaxCritConvergeValue", "Convergence criterion", "", XML_TYPE_FLOAT, "0.1");
    Util_XML_Parse_Create_Node(fp, fileName, groupingNode, "ConvergeThresh", "Convergence threshold", "", XML_TYPE_FLOAT, "0.01");
    
    if (bm->useMultispAssess) {
        
        Util_XML_Parse_Create_Node(fp, fileName, groupingNode, "ss3Name", "ss3 executable name", "", XML_TYPE_STRING, "");
        
        Util_XML_Parse_Create_Node(fp, fileName, groupingNode, "NumRAssessScriptNames", "Number of R scripts being used in R MSHS calls", "", XML_TYPE_BOOLEAN,"0");
        bm->NumRAssessScriptNames = (int)Util_XML_Read_Value(fileName, ATLANTIS_ATTRIBUTE,  bm->ecotest, 1, groupingNode, integer_check, "NumRAssessScriptNames");
        
        for (nf = 0; nf < bm->NumRAssessScriptNames; nf ++) {
            snprintf(nameString, sizeof(nameString), "RAssessRscriptName%d", nf);
            Util_XML_Parse_Create_Node(fp, fileName, groupingNode, nameString, "Base of name for R script", "", XML_TYPE_STRING, "");
        }
        
        Util_XML_Create_Node_Next_Line(fp, fileName, groupingNode, "PGMSYlinks", "Metier links for PGMSY", "", XML_TYPE_INTEGER);
    }
    
    // More tier parameters - per fleet
    childGroupingNode = Util_XML_Create_Node(ATLANTIS_ATTRIBUTE_SUB_GROUP, groupingNode, "CVs", "CVs", "", "");
	Parse_File(bm, fp, fileName, childGroupingNode, "DiscardCV", "^discard_cv_", "Discard cv by fleet", "", XML_TYPE_FLOATARRAY, bm->K_num_fisheries, TRUE, Init_Species_Zero_ValuesXML, Species_Last_XMLFunction);
	Parse_File(bm, fp, fileName, childGroupingNode, "CatchCV", "^catch_cv_", "Catch cv by fleet", "", XML_TYPE_FLOATARRAY, bm->K_num_fisheries, TRUE, Init_Species_Zero_ValuesXML, Species_Last_XMLFunction);
	Parse_File(bm, fp, fileName, childGroupingNode, "EffortCV", "^effort_cv_", "Effort cv by fleet", "", XML_TYPE_FLOATARRAY, bm->K_num_fisheries, TRUE,
               Init_Species_Zero_ValuesXML, Species_Last_XMLFunction);
    
	childGroupingNode = Util_XML_Create_Node(ATLANTIS_ATTRIBUTE_SUB_GROUP, groupingNode, "CPUE", "CPUE", "", "");
	Parse_File(bm, fp, fileName, childGroupingNode, "CPUECV", "^cpue_cv_", "CPUE cv by fleet", "", XML_TYPE_FLOATARRAY, bm->K_num_fisheries, TRUE, Init_Species_Zero_ValuesXML, Species_Last_XMLFunction);
	Parse_File(bm, fp, fileName, childGroupingNode, "CPUEvar", "^cpue_var_", "CPUE variance catchability by fleet", "", XML_TYPE_FLOATARRAY, bm->K_num_fisheries, TRUE, Init_Species_Zero_ValuesXML, Species_Last_XMLFunction);
	Parse_File(bm, fp, fileName, childGroupingNode, "CPUEcorr", "^cpue_corr_", "CPUE correlation catchability by fleet", "", XML_TYPE_FLOATARRAY, bm->K_num_fisheries, TRUE, Init_Species_Zero_ValuesXML, Species_Last_XMLFunction);
    
    /*
     Parse_File(bm, fp, fileName, childGroupingNode, "CPUEqmu", "^cpue_qmu_", "CPUE average catchability by fleet", "", XML_TYPE_FLOATARRAY, bm->K_num_fisheries,
     TRUE, Init_Species_Zero_ValuesXML, Species_Last_XMLFunction);
     Parse_File(bm, fp, fileName, childGroupingNode, "CPUEpow", "^cpue_pow_", "CPUE power catchability by fleet", "", XML_TYPE_FLOATARRAY, bm->K_num_fisheries,
     TRUE, Init_Species_Zero_ValuesXML, Species_Last_XMLFunction);
     */

    Parse_File(bm, fp, fileName, childGroupingNode, "PGMSY_q", "^PGMSY_q_", "Projection assumed catchability per fleet per species", "", XML_TYPE_FLOATARRAY, bm->K_num_fisheries, TRUE, Init_Species_Zero_ValuesXML, Species_Last_XMLFunction);

   Parse_File(bm, fp, fileName, childGroupingNode, "PGMSY_sel_lsm", "^PGMSY_sel_lsm_", "Inflection point for projection assumed selectivity curve per fleet per species", "", XML_TYPE_FLOATARRAY, bm->K_num_fisheries, TRUE, Init_Species_Zero_ValuesXML, Species_Last_XMLFunction);

    Parse_File(bm, fp, fileName, childGroupingNode, "PGMSY_sel_sigma", "^PGMSY_sel_sigma_", "Sigma for projection assumed selectivity curve per fleet per species", "", XML_TYPE_FLOATARRAY, bm->K_num_fisheries, TRUE, Init_Species_Zero_ValuesXML, Species_Last_XMLFunction);

    Parse_File(bm, fp, fileName, childGroupingNode, "PGMSY_sel_curve", "^PGMSY_sel_curve_", "Projection assumed selectivity curve per fleet per species", "", XML_TYPE_FLOATARRAY, bm->K_num_fisheries, TRUE, Init_Species_Zero_ValuesXML, Species_Last_XMLFunction);

    childGroupingNode = Util_XML_Create_Node(ATLANTIS_ATTRIBUTE_SUB_GROUP, groupingNode, "StartSS", "StartSS", "", "");
	Parse_File(bm, fp, fileName, childGroupingNode, "Start_SelInflect", "^Start_SelInflect_", "Starting values of selectivity parameters for SS control file", "", XML_TYPE_FLOATARRAY, bm->K_num_fisheries, TRUE, Init_Species_Zero_ValuesXML, Species_Last_XMLFunction);
	Parse_File(bm, fp, fileName, childGroupingNode, "Start_SelWidth", "^Start_SelWidth_", "Starting values of selectivity parameters for SS control file", "", XML_TYPE_FLOATARRAY, bm->K_num_fisheries, TRUE, Init_Species_Zero_ValuesXML, Species_Last_XMLFunction);
    Parse_File(bm, fp, fileName, childGroupingNode, "Start_RetInflect", "^Start_RetInflect_", "Starting values of retention parameters for SS control file", "", XML_TYPE_FLOATARRAY, bm->K_num_fisheries, TRUE, Init_Species_Zero_ValuesXML, Species_Last_XMLFunction);
    Parse_File(bm, fp, fileName, childGroupingNode, "Start_RetSlope", "^Start_RetSlope_", "Starting values of retention parameters for SS control file", "", XML_TYPE_FLOATARRAY, bm->K_num_fisheries, TRUE, Init_Species_Zero_ValuesXML, Species_Last_XMLFunction);
    
	childGroupingNode = Util_XML_Create_Node(ATLANTIS_ATTRIBUTE_SUB_GROUP, groupingNode, "Varadj", "Varadj", "", "");
    Parse_File(bm, fp, fileName, childGroupingNode, "Varadj_CPUE", "^Varadj_CPUE_", "Variance adjustment for CPUE (SS control file)", "", XML_TYPE_FLOATARRAY, bm->K_num_fisheries, TRUE, Init_Species_Zero_ValuesXML, Species_Last_XMLFunction);
    Parse_File(bm, fp, fileName, childGroupingNode, "Varadj_discard", "^Varadj_discard_", "Variance adjustment for discards (SS control file)", "", XML_TYPE_FLOATARRAY, bm->K_num_fisheries, TRUE, Init_Species_Zero_ValuesXML, Species_Last_XMLFunction);
    Parse_File(bm, fp, fileName, childGroupingNode, "Varadj_length", "^Varadj_length_", "Variance adjustment for length (SS control file)", "", XML_TYPE_FLOATARRAY, bm->K_num_fisheries, TRUE, Init_Species_Zero_ValuesXML, Species_Last_XMLFunction);
    Parse_File(bm, fp, fileName, childGroupingNode, "Varadj_age", "^Varadj_age_", "Variance adjustment for age (SS control file)", "", XML_TYPE_FLOATARRAY, bm->K_num_fisheries, TRUE, Init_Species_Zero_ValuesXML, Species_Last_XMLFunction);

    if ( verbose > 0 )
        printf("Start to load tiered assessment arrays\n");
    
    // assessment specifications
	childGroupingNode = Util_XML_Create_Node(ATLANTIS_ATTRIBUTE_SUB_GROUP, groupingNode, "GenSS", "GenSS", "", "");
	Create_RBC_Species_ParamXML(bm, fileName, fp, childGroupingNode, DiscType_id, "Type of discards 1 = biomass, 2 = fraction", "", XML_TYPE_INTEGER, "0");
	Create_RBC_Species_ParamXML(bm, fileName, fp, childGroupingNode, MaxH_id, "Max exploitation rate", "", XML_TYPE_FLOAT, "0");
	Create_RBC_Species_ParamXML(bm, fileName, fp, childGroupingNode, Growthage_L1_id, "Growth age for L1 for SS control file - linear age below this age", "", XML_TYPE_FLOAT, "0");
	Create_RBC_Species_ParamXML(bm, fileName, fp, childGroupingNode, Growthage_L2_id, "Growth age for L2 for SS control file", "", XML_TYPE_FLOAT, "0");
	Create_RBC_Species_ParamXML(bm, fileName, fp, childGroupingNode, MinCatch_id, "Proportion of final historical year catch to use if RBC = 0", "", XML_TYPE_FLOAT, "0");
	Create_RBC_Species_ParamXML(bm, fileName, fp, childGroupingNode, AssessStart_id, "flag indicating which year to start assessments", "", XML_TYPE_INTEGER, "0");
    
    //  other params
	Create_RBC_Species_ParamXML(bm, fileName, fp, childGroupingNode, Hsteep_id, "Recruitment steepness", "", XML_TYPE_FLOAT, "0");
	Create_RBC_Species_ParamXML(bm, fileName, fp, childGroupingNode, Agesel_Pattern_id, "Age selectivity pattern (for SS control file)", "", XML_TYPE_FLOAT, "0");
	Create_RBC_Species_ParamXML(bm, fileName, fp, childGroupingNode, AssessFreq_id, "Perform an assessment every AssessFreq years", "", XML_TYPE_FLOAT, "0");
	Create_RBC_Species_ParamXML(bm, fileName, fp, childGroupingNode, MG_offset_id, "Mortality-growth parameter offset option", "", XML_TYPE_FLOAT, "0");
	Create_RBC_Species_ParamXML(bm, fileName, fp, childGroupingNode, RecDevBack_id, "Number of years back from current for estimating rec devs (age of recruit)", "", XML_TYPE_FLOAT, "0");
	Create_RBC_Species_ParamXML(bm, fileName, fp, childGroupingNode, Regime_shift_assess_id, "=1 if want to use regime shift in tier 1 assess, else 0", "", XML_TYPE_INTEGER, "0");
    Create_RBC_Species_ParamXML(bm, fileName, fp, childGroupingNode, Regime_year_assess_id, "Year of regime change assessment", "", XML_TYPE_INTEGER, "0");
    Create_RBC_Species_ParamXML(bm, fileName, fp, childGroupingNode, BallParkF_id, "F ball park F for early years", "", XML_TYPE_FLOAT, "0");
    Create_RBC_Species_ParamXML(bm, fileName, fp, childGroupingNode, BallParkYr_id, "F ball park year (-1 to disable)", "", XML_TYPE_FLOAT, "0");
	Create_RBC_Species_ParamXML(bm, fileName, fp, childGroupingNode, NumChangeLambda_id, "Number of changes to make to default lambdas", "", XML_TYPE_INTEGER, "0");
    Create_RBC_Species_ParamXML(bm, fileName, fp, childGroupingNode, num_enviro_obs_id, "Number of environmental observations", "", XML_TYPE_INTEGER, "0");
	Create_RBC_Species_ParamXML(bm, fileName, fp, childGroupingNode, num_growth_morphs_id, "Number of morphs within growth pattern", "", XML_TYPE_INTEGER, "0");
    
	// size parameters
	Create_RBC_Species_ParamXML(bm, fileName, fp, childGroupingNode, Nsex_samp_id, "=1 if want to combine sexes in age and length samples", "", XML_TYPE_INTEGER, "0");
	Create_RBC_Species_ParamXML(bm, fileName, fp, childGroupingNode, MaxAge_id, "Maximum age", "", XML_TYPE_FLOAT, "0");
	Create_RBC_Species_ParamXML(bm, fileName, fp, childGroupingNode, Nyfuture_id, " number of future years", "", XML_TYPE_INTEGER, "0");
	Create_RBC_Species_ParamXML(bm, fileName, fp, childGroupingNode, Nlen_id, "Number of length bins", "", XML_TYPE_INTEGER, "0");
	Create_RBC_Species_ParamXML(bm, fileName, fp, childGroupingNode, Lbin_id, "size of length bin (cm)", "", XML_TYPE_FLOAT, "0");
    
    Create_RBC_Species_ParamXML(bm, fileName, fp, childGroupingNode, NumFisheries_id, "flag indicating number of fisheries (metiers) for tiered assessments", "", XML_TYPE_INTEGER, "0");
    Create_RBC_Species_ParamXML(bm, fileName, fp, childGroupingNode, NumRegions_id, "flag indicating number of regions for tiered assessments", "", XML_TYPE_INTEGER, "0");
    Create_RBC_Species_ParamXML(bm, fileName, fp, childGroupingNode, Nsexes_id, "Number of sexes in assessment", "", XML_TYPE_INTEGER, "0");
    
    Create_RBC_Species_ParamXML(bm, fileName, fp, childGroupingNode, thresh_mat_id, "Maturity trheshold for close kin", "", XML_TYPE_FLOAT, "1.0");
    Create_RBC_Species_ParamXML(bm, fileName, fp, childGroupingNode, femsexratio_id, "Sex ratio, female component", "", XML_TYPE_FLOAT, "0.5");

    // Added from Ratpack
    Create_RBC_Species_ParamXML(bm, fileName, fp, childGroupingNode, LFSSlim_id, "Length sub-sample size for the assessment", "", XML_TYPE_INTEGER, "0");
    Create_RBC_Species_ParamXML(bm, fileName, fp, childGroupingNode, AFSSlim_id, "Age sub-sample size for the assessment", "", XML_TYPE_INTEGER, "0");
    Create_RBC_Species_ParamXML(bm, fileName, fp, childGroupingNode, NumSurvey_id, "Number of surveys in assessment", "", XML_TYPE_INTEGER, "0");
    Create_RBC_Species_ParamXML(bm, fileName, fp, childGroupingNode, NblockPattern_id, "Number of blocks in the assessment", "", XML_TYPE_INTEGER, "0");
    Create_RBC_Species_ParamXML(bm, fileName, fp, childGroupingNode, SRBlock_id, "Block defining regime shift", "", XML_TYPE_FLOAT, "0");
    Create_RBC_Species_ParamXML(bm, fileName, fp, childGroupingNode, assRecDevMinYear_id, "Year where recreuitment deviation kicks in", "", XML_TYPE_INTEGER, "0");
    
    // Mutlispecies
    Create_RBC_Species_ParamXML(bm, fileName, fp, childGroupingNode, MultispAssessType_id, "What kind of multispecies assessment being used", "", XML_TYPE_INTEGER, "0");
    Create_RBC_Species_ParamXML(bm, fileName, fp, childGroupingNode, mgt_indicator_id, "Management indicator species", "", XML_TYPE_INTEGER, "0");
    Create_RBC_Species_ParamXML(bm, fileName, fp, childGroupingNode, init_mgt_category_id, "Initial management category for indicator species harvest strategy", "", XML_TYPE_INTEGER, "0");
    Create_RBC_Species_ParamXML(bm, fileName, fp, childGroupingNode, init_mgt_sp_id, "Initial management indicator species", "", XML_TYPE_INTEGER, "0");
                                   
    Create_RBC_Species_ParamXML(bm, fileName, fp, childGroupingNode, PGMSYBHalpha_id, "Beverton Holt alpha assumed in PGMSY projection", "", XML_TYPE_INTEGER, "0");
    Create_RBC_Species_ParamXML(bm, fileName, fp, childGroupingNode, PGMSYBHbeta_id, "Beverton Holt beta assumed in PGMSY projection", "", XML_TYPE_INTEGER, "0");

                                   
    Util_XML_Parse_Create_Node(fp, fileName, groupingNode, "ReviewPeriod", "Review period for indicator species for multispecies harvest strategy", "", XML_TYPE_INTEGER, "0");
    Util_XML_Parse_Create_Node(fp, fileName, groupingNode, "TriggerCheckPeriod", "Reference period for trigger species checks for multispecies harvest strategy", "", XML_TYPE_INTEGER, "0");
    Util_XML_Parse_Create_Node(fp, fileName, groupingNode, "LagPeriod", "Lag period for indicator species assesment for multispecies harvest strategy", "", XML_TYPE_INTEGER, "0");
    Util_XML_Parse_Create_Node(fp, fileName, groupingNode, "GradientWindow", "Windo for judging the species pairing for indicator species assesment for multispecies harvest strategy", "", XML_TYPE_INTEGER, "0");
    Util_XML_Parse_Create_Node(fp, fileName, groupingNode, "GradientPeriod", "Lag period for species pairing step of indicator species assesment for multispecies harvest strategy", "", XML_TYPE_INTEGER, "0");
    Util_XML_Parse_Create_Node(fp, fileName, groupingNode, "GradientBuffer", "Buffer when using indicator species harvest startegy and trying to match species - multispecies harvest strategy", "", XML_TYPE_FLOAT, "0");
    Util_XML_Parse_Create_Node(fp, fileName, groupingNode, "UseCategory", "Whether using category or species pair for setting TAC in multispecies harvest strategy", "", XML_TYPE_INTEGER, "0");
    Util_XML_Parse_Create_Node(fp, fileName, groupingNode, "UseClosest", "Whether using closest species when pairing for setting TAC in multispecies harvest strategy", "", XML_TYPE_INTEGER, "0");
    Util_XML_Parse_Create_Node(fp, fileName, groupingNode, "UseTriggerMgmt", "Whether using trigger based assessments to get quasi- multispecies harvest strategy", "", XML_TYPE_INTEGER, "0");
    Util_XML_Parse_Create_Node(fp, fileName, groupingNode, "ProjYr", "Length of the projection period for the assessments", "", XML_TYPE_INTEGER, "0");
    Util_XML_Parse_Create_Node(fp, fileName, groupingNode, "ThresholdDepletion", "Threshold for depletion levels for PGMSY", "", XML_TYPE_FLOAT, "0");
    Util_XML_Parse_Create_Node(fp, fileName, groupingNode, "ThresholdBound", "Bound on depletion levels for PGMSY", "", XML_TYPE_FLOAT, "0");
    Util_XML_Parse_Create_Node(fp, fileName, groupingNode, "MaxIteration", "Maximum iterations allowed in PGMS", "", XML_TYPE_INTEGER, "0");

	// SS3 Biological parameters
	Create_RBC_Species_ParamXML(bm, fileName, fp, childGroupingNode, flagLAdirect_id, "Flag showing whether length at age input directly (1) or not (0)", "", XML_TYPE_INTEGER, "0");
	Create_RBC_Species_ParamXML(bm, fileName, fp, childGroupingNode, flagSLAdirect_id, "Flag showing if std dev of length at age input directly (1) or not (0)", "", XML_TYPE_INTEGER, "0");
	Create_RBC_Species_ParamXML(bm, fileName, fp, childGroupingNode, flagWAdirect_id, "Flag showing if weight at age input directly (1) or not (0)", "", XML_TYPE_INTEGER, "0");
    
    // recruitment parameters
	Create_RBC_Species_ParamXML(bm, fileName, fp, childGroupingNode, SigmaR1_id, "Std dev recruitment 1 in past", "", XML_TYPE_FLOAT, "0");
	Create_RBC_Species_ParamXML(bm, fileName, fp, childGroupingNode, SigmaR2_id, "Std dev recruitment 2", "", XML_TYPE_FLOAT, "0");
	Create_RBC_Species_ParamXML(bm, fileName, fp, childGroupingNode, SigmaR_future_id, "sigmaR for future recruitment", "", XML_TYPE_FLOAT, "0");
	Create_RBC_Species_ParamXML(bm, fileName, fp, childGroupingNode, PSigmaR1_id, "Probability of normal recruitment", "", XML_TYPE_FLOAT, "0");
	Create_RBC_Species_ParamXML(bm, fileName, fp, childGroupingNode, Regime_year_id, "Start year for regime shift", "", XML_TYPE_FLOAT, "0");
	Create_RBC_Species_ParamXML(bm, fileName, fp, childGroupingNode, RecDevMinYr_id, "First year of recruitment deviations", "", XML_TYPE_FLOAT, "0");
	Create_RBC_Species_ParamXML(bm, fileName, fp, childGroupingNode, RecDevMaxYr_id, "Last year of recruitment deviations", "", XML_TYPE_FLOAT, "0");
	Create_RBC_Species_ParamXML(bm, fileName, fp, childGroupingNode, RecDevFlag_id, "Flag indicating whether historic recruitment deviationss read in (1) or not (0)", "", XML_TYPE_INTEGER, "0");
	Create_RBC_Species_ParamXML(bm, fileName, fp, childGroupingNode, AutoCorRecDev_id, "Flag indicating whether recruitment deviationss are auto-correlated (1) or not (0)", "", XML_TYPE_INTEGER, "0");
    
    /* Tier rules */
    // tier 1
	childGroupingNode = Util_XML_Create_Node(ATLANTIS_ATTRIBUTE_SUB_GROUP, groupingNode, "Tier1", "Tier1", "", "");
	Create_RBC_Species_ParamXML(bm, fileName, fp, childGroupingNode, T1_steep_phase_id, "Flag indicating if want T1 to estimate steepness in phase n, else -n", "", XML_TYPE_FLOAT, "0");
	Create_RBC_Species_ParamXML(bm, fileName, fp, childGroupingNode, tiertype_id, "Flag indicating which type of tier rule to use, for tier 3 1=original , 2=Neil's, for tier 4 1= original Tier 4, 2= new Tier 4 and for tier 5 1= Surplus production, 2= av length", "", XML_TYPE_FLOAT, "0");

    // tier3
    childGroupingNode = Util_XML_Create_Node(ATLANTIS_ATTRIBUTE_SUB_GROUP, groupingNode, "Tier3", "Tier3", "", "");
	Create_RBC_Species_ParamXML(bm, fileName, fp, childGroupingNode, Tier3_Fcalc_id, "Type of tier3 F calc 1 = catch-curve, 2=ASPM , 3 = true F", "", XML_TYPE_INTEGER, "0");
	Create_RBC_Species_ParamXML(bm, fileName, fp, childGroupingNode, Tier3_time_id, "Time period for calc Ccurr : 1=4 years, 2=no ages selected", "", XML_TYPE_INTEGER, "0");
	Create_RBC_Species_ParamXML(bm, fileName, fp, childGroupingNode, Tier3_maxage_id, "Maximum age for Tier 3", "", XML_TYPE_FLOAT, "0");
	Create_RBC_Species_ParamXML(bm, fileName, fp, childGroupingNode, Tier3_M_id, "M input for tier 3", "", XML_TYPE_FLOAT, "0");
	Create_RBC_Species_ParamXML(bm, fileName, fp, childGroupingNode, Tier3_S25_id, "Length at 25% selectivity by fleet - input for tier 3", "", XML_TYPE_FLOAT, "0");
	Create_RBC_Species_ParamXML(bm, fileName, fp, childGroupingNode, Tier3_S50_id, "Length at 50% selectivity by fleet - input for tier 3", "", XML_TYPE_FLOAT, "0");
	Create_RBC_Species_ParamXML(bm, fileName, fp, childGroupingNode, Tier3_F_id, "Starting estimate of F for tier 3", "", XML_TYPE_FLOAT, "0");
	Create_RBC_Species_ParamXML(bm, fileName, fp, childGroupingNode, Tier3_h_id, "Steepness input for Tier 3", "", XML_TYPE_FLOAT, "0");
	Create_RBC_Species_ParamXML(bm, fileName, fp, childGroupingNode, Tier3_matlen_id, "Knife-edge length at maturity, input for Tier 3", "", XML_TYPE_FLOAT, "0");
	Create_RBC_Species_ParamXML(bm, fileName, fp, childGroupingNode, Tier3_maxF_id, "Max F to use in Fref calcs (2 for flathead, 4 for whiting)", "", XML_TYPE_FLOAT, "0");
	Create_RBC_Species_ParamXML(bm, fileName, fp, childGroupingNode, CCsel_years_id, "Number of years of catch-at-age data to use in CCsel method", "", XML_TYPE_FLOAT, "0");

	Parse_File(bm, fp, fileName, childGroupingNode, "Tier3_Linf", "^Tier3_Linf_", "Tier 3 Linf per sex, Linf f and m used in true model", "", XML_TYPE_FLOATARRAY, bm->K_num_sexes, TRUE, Init_Species_Zero_ValuesXML, Species_Last_XMLFunction);

	Parse_File(bm, fp, fileName, childGroupingNode, "Tier3_k", "^Tier3_k_", "Tier 3 k per sex, from ss rep file growth parameters", "", XML_TYPE_FLOATARRAY,
			bm->K_num_sexes, TRUE, Init_Species_Zero_ValuesXML, Species_Last_XMLFunction);

	Parse_File(bm, fp, fileName, childGroupingNode, "Tier3_t0", "^Tier3_t0_", "Tier 3 t0 per sex, from ss rep file growth parameters", "", XML_TYPE_FLOATARRAY,
			bm->K_num_sexes, TRUE, Init_Species_Zero_ValuesXML, Species_Last_XMLFunction);
    
    // tier 4
    childGroupingNode = Util_XML_Create_Node(ATLANTIS_ATTRIBUTE_SUB_GROUP, groupingNode, "Tier4", "Tier4", "", "");
	Create_RBC_Species_ParamXML(bm, fileName, fp, childGroupingNode, Tier4_Cmaxmult_id, "TAC in final historic year", "", XML_TYPE_FLOAT, "0");
	Create_RBC_Species_ParamXML(bm, fileName, fp, childGroupingNode, Tier4_avtime_id, "Time period for calc Ccurr : 1 = 4 years, 2 = same as years for CPUE target", "", XML_TYPE_FLOAT, "0");
	Create_RBC_Species_ParamXML(bm, fileName, fp, childGroupingNode, Tier4_CPUEyrmin_id, "Year in which to start averaging CPUE target", "", XML_TYPE_INTEGER, "0");
	Create_RBC_Species_ParamXML(bm, fileName, fp, childGroupingNode, Tier4_CPUEyrmax_id, "Number of years over which to calc CPUE target", "", XML_TYPE_INTEGER, "0");
	Create_RBC_Species_ParamXML(bm, fileName, fp, childGroupingNode, Tier4_alpha_id, "Alpha value for use in Tier 4 calculations", "", XML_TYPE_FLOAT, "0");
	Create_RBC_Species_ParamXML(bm, fileName, fp, childGroupingNode, Tier4_m_id, "Average CPUE over last m yearsin Tier 4 calculations", "", XML_TYPE_FLOAT, "0");
    Create_RBC_Species_ParamXML(bm, fileName, fp, childGroupingNode, Tier4_Bo_correct_id, "Scalar for Tier 4 calculations to correct for whether at B0 in reference period or not", "", XML_TYPE_FLOAT, "0");
    
    // reference levels for new Tier 4 rule
	Create_RBC_Species_ParamXML(bm, fileName, fp, childGroupingNode, Maturity_Inflect_id, "Maturity inflection for SS control file", "", XML_TYPE_FLOAT, "0");
	Create_RBC_Species_ParamXML(bm, fileName, fp, childGroupingNode, Maturity_Slope_id, "Maturity slope for SS control file", "", XML_TYPE_FLOAT, "0");
	Create_RBC_Species_ParamXML(bm, fileName, fp, childGroupingNode, PostRule_id, "use post-assessment rule 1 or 2 or not", "", XML_TYPE_INTEGER, "0");
	//Create_RBC_Species_ParamXML(bm, fileName, fp, childGroupingNode, Ncage_id, "Actual number of years catch-at-age in CCsel (used to pass info to EquilF_Funk)", "", XML_TYPE_FLOAT, "0");
	Create_RBC_Species_ParamXML(bm, fileName, fp, childGroupingNode, CPUEmult_id, "CPUE change multiplier in post-assess rule", "", XML_TYPE_INTEGER, "0");
	Create_RBC_Species_ParamXML(bm, fileName, fp, childGroupingNode, MaxChange_id, "Maximum change of TAC allowed from post-assess rule", "", XML_TYPE_FLOAT, "0");
    
    // tier 5
    childGroupingNode = Util_XML_Create_Node(ATLANTIS_ATTRIBUTE_SUB_GROUP, groupingNode, "Tier5", "Tier5", "", "");
	Parse_File(bm, fp, fileName, childGroupingNode, "Tier5_Bstart", "^Tier5_Bstart_", "Tier 5 initial value B0 plus lower and upper bounds", "", XML_TYPE_FLOATARRAY, 3, TRUE, Init_Species_Zero_ValuesXML, Species_Last_XMLFunction);

	Parse_File(bm, fp, fileName, childGroupingNode, "Tier5_r", "^Tier5_r_", "Tier 5 r, initial value of r plus lower and upper bounds", "", XML_TYPE_FLOATARRAY,
			3, TRUE, Init_Species_Zero_ValuesXML, Species_Last_XMLFunction);

	Create_RBC_Species_ParamXML(bm, fileName, fp, childGroupingNode, Tier5_length_id, "Reference length of full selection for avlen method", "", XML_TYPE_FLOAT, "0");
	Create_RBC_Species_ParamXML(bm, fileName, fp, childGroupingNode, Tier5_S50_id, "Length of knife-edge sel (50% sel) for avlen method", "", XML_TYPE_FLOAT, "0");
	Create_RBC_Species_ParamXML(bm, fileName, fp, childGroupingNode, Tier5_cv_id, "CV of length-at-age used in Avlen assessment", "", XML_TYPE_FLOAT, "0");
	Create_RBC_Species_ParamXML(bm, fileName, fp, childGroupingNode, Tier5_flt_id, "Which fleet's CPUE to use in SP model", "", XML_TYPE_INTEGER, "0");
	Create_RBC_Species_ParamXML(bm, fileName, fp, childGroupingNode, Tier5_reg_id, "Which region's CPUE to use in SP model", "", XML_TYPE_INTEGER, "0");
	Create_RBC_Species_ParamXML(bm, fileName, fp, childGroupingNode, Tier5_p_id, "Asymmetry parameter for surplus production model", "", XML_TYPE_FLOAT, "0");
	Create_RBC_Species_ParamXML(bm, fileName, fp, childGroupingNode, Tier5sel_id, "Selecivity assumed in SAFE assessment model", "", XML_TYPE_FLOAT, "0");
	Create_RBC_Species_ParamXML(bm, fileName, fp, childGroupingNode, Tier5q_id, "Catchability assumed in SAFE assessment model", "", XML_TYPE_FLOAT, "0");
    
    // tier 6
    childGroupingNode = Util_XML_Create_Node(ATLANTIS_ATTRIBUTE_SUB_GROUP, groupingNode, "Tier6", "Tier6", "", "");
    Create_RBC_Species_ParamXML(bm, fileName, fp, childGroupingNode, TriggerResponseScen_id, "WHat to do if Tier 6 triggered", "", XML_TYPE_INTEGER, "0");
    Create_RBC_Species_ParamXML(bm, fileName, fp, childGroupingNode, trigger_threshold_id, "WHat to the proportional change for triggered action", "", XML_TYPE_FLOAT, "0");    
    
    Util_XML_Parse_Create_Node(fp, fileName, groupingNode, "NumTriggers", "NumTriggers", "", XML_TYPE_INTEGER, "0");
	Util_XML_Create_Node_Next_Line(fp, fileName, groupingNode, "TriggerPoints", "Trigger points versus historical catch levels", "", XML_TYPE_FLOAT);

    // Close kin parameters
    Util_XML_Parse_Create_Node(fp, fileName, groupingNode, "UseCloseKin", "UseCloseKin", "Whether using close kin", XML_TYPE_INTEGER, "4000");
    Util_XML_Parse_Create_Node(fp, fileName, groupingNode, "nTotSamps", "nTotSamps", "Sample size for close kin", XML_TYPE_INTEGER, "4000");
    Util_XML_Parse_Create_Node(fp, fileName, groupingNode, "first_sy", "first_sy", "First year of close kin data collection", XML_TYPE_INTEGER, "0");
    Util_XML_Parse_Create_Node(fp, fileName, groupingNode, "last_sy", "last_sy", "Final year of close kin data collection", XML_TYPE_INTEGER, "20");
    Util_XML_Parse_Create_Node(fp, fileName, groupingNode, "first_dy", "first_dy", "When first use close kin data", XML_TYPE_INTEGER, "15");
    Util_XML_Parse_Create_Node(fp, fileName, groupingNode, "Nscale", "Nscale", "", XML_TYPE_INTEGER, "1000");
    Util_XML_Parse_Create_Node(fp, fileName, groupingNode, "ThreshPercLost", "ThreshPercLost", "", XML_TYPE_INTEGER, "5");
    Util_XML_Parse_Create_Node(fp, fileName, groupingNode, "samp_power", "samp_power", "", XML_TYPE_INTEGER, "1");
    Util_XML_Parse_Create_Node(fp, fileName, groupingNode, "a_ck_thresh", "a_ck_thresh", "", XML_TYPE_INTEGER, "5");
    Util_XML_Parse_Create_Node(fp, fileName, groupingNode, "a_ck_thresh2", "a_ck_thresh2", "", XML_TYPE_INTEGER, "10");
    Util_XML_Parse_Create_Node(fp, fileName, groupingNode, "a_ck_scalar", "a_ck_scalar", "", XML_TYPE_FLOAT, "10.0");
    Util_XML_Parse_Create_Node(fp, fileName, groupingNode, "a_ck_scalar2", "a_ck_scalar2", "", XML_TYPE_FLOAT, "2.0");
    Util_XML_Parse_Create_Node(fp, fileName, groupingNode, "thresh_mat", "thresh_mat", "", XML_TYPE_FLOAT, "0.1");
    Util_XML_Parse_Create_Node(fp, fileName, groupingNode, "HSP_false_neg_loss_rate", "HSP_false_neg_loss_rate", "", XML_TYPE_FLOAT, "0.75");
    
    /* General arrays */
	Parse_File(bm, fp, fileName, groupingNode, "Ageing_error", "^Ageing_error_", "Ageing error", "", XML_TYPE_FLOATARRAY, bm->K_max_agekey, TRUE,
			Init_Species_Zero_ValuesXML, Species_Last_XMLFunction);
    
    Parse_File(bm, fp, fileName, groupingNode, "RegStructure", "^regID_", "Regions for the assessment", "", XML_TYPE_FLOATARRAY, bm->nbox, TRUE,
               Init_Species_Zero_ValuesXML, Species_Last_XMLFunction);

    // by sex, stock and age
    Parse_File(bm, fp, fileName, groupingNode, "Mzero", "MzeroStock[0-9]{0,2}$", "Starting assessment natural mortality by stock, sex and age", "", XML_TYPE_FLOATARRAY, bm->K_max_agekey, TRUE, InitAssessStockAgeXML, AssessStockXMLFunction);

    Parse_File(bm, fp, fileName, groupingNode, "MeanLenAge", "MeanLenAgeStock[0-9]{0,2}$", "Length at age by stock and sex for assessment", "", XML_TYPE_FLOATARRAY, bm->K_max_agekey, TRUE, InitAssessStockAgeXML, AssessStockXMLFunction);
    
    //Parse_File(bm, fp, fileName, groupingNode, "SigmaLenAge", "SigmaLenAge_[1-2]Stock[0-9]{0,2}$", "Stdev of length-at-age at age for assessment", "", XML_TYPE_FLOATARRAY, bm->K_max_agekey, TRUE, InitAssessStockAgeXML, AssessSigmaLenAgeXMLFunction);
    
    // by sex and stock
    Parse_File(bm, fp, fileName, groupingNode, "CvLA0", "_CvLA0Stock[0-9]{0,2}$", "CV of length-at-age by stock and sex for age 0", "", XML_TYPE_FLOATARRAY, bm->K_num_sexes, TRUE, InitAssessSexXML, AssessSexXMLFunction);

    Parse_File(bm, fp, fileName, groupingNode, "CvLAmax", "_CvLAmaxStock[0-9]{0,2}$", "CV of length-at-age by stock and sex for max age", "", XML_TYPE_FLOATARRAY, bm->K_num_sexes, TRUE, InitAssessSexXML, AssessSexXMLFunction);

    Parse_File(bm, fp, fileName, groupingNode, "Wtlen_a", "_WtlenaStock[0-9]{0,2}$", "Weight-length relationship coefficient by stock and sex", "", XML_TYPE_FLOATARRAY, bm->K_num_sexes, TRUE, InitAssessSexXML, AssessSexXMLFunction);

    Parse_File(bm, fp, fileName, groupingNode, "Wtlen_b", "_WtlenbStock[0-9]{0,2}$", "Weight-length relationship exponent by stock and sex", "", XML_TYPE_FLOATARRAY, bm->K_num_sexes, TRUE, InitAssessSexXML, AssessSexXMLFunction);

    Parse_File(bm, fp, fileName, groupingNode, "Fecund", "_FecundStock[0-9]{0,2}$", "Fecundity at age by stock", "", XML_TYPE_FLOATARRAY, bm->K_num_sexes, TRUE, InitAssessSexXML, AssessSexXMLFunction);

    Parse_File(bm, fp, fileName, groupingNode, "VBLinf", "_VBLinfStock[0-9]{0,2}$", "Von Bert Linfinity by stock and sex", "", XML_TYPE_FLOATARRAY, bm->K_num_sexes, TRUE, InitAssessSexXML, AssessSexXMLFunction);

    Parse_File(bm, fp, fileName, groupingNode, "VBk", "_VBkStock[0-9]{0,2}$", "Von B k by stock and sex", "", XML_TYPE_FLOATARRAY, bm->K_num_sexes, TRUE, InitAssessSexXML, AssessSexXMLFunction);

    Parse_File(bm, fp, fileName, groupingNode, "VBt0", "_VBt0Stock[0-9]{0,2}$", "Von B t0 by stock and sex", "", XML_TYPE_FLOATARRAY, bm->K_num_sexes, TRUE, InitAssessSexXML, AssessSexXMLFunction);
    
    Parse_File(bm, fp, fileName, groupingNode, "SSMort", "_SSMortStock[0-9]{0,2}$", "M for SS control file", "", XML_TYPE_FLOATARRAY, bm->K_num_sexes, TRUE, InitAssessSexXML, AssessSexXMLFunction);

    // US tiers
    childGroupingNode = Util_XML_Create_Node(ATLANTIS_ATTRIBUTE_SUB_GROUP, groupingNode, "TIERsigma", "TIERsigma", "", "");
    
	Create_RBC_Species_ParamXML(bm, fileName, fp, childGroupingNode, Tier1Sig_id, "Scalar to apply when taking OFL to ABC (RBC to TAC) - tier 1", "", XML_TYPE_FLOAT, "0.9");
	Create_RBC_Species_ParamXML(bm, fileName, fp, childGroupingNode, Tier2Sig_id, "Scalar to apply when taking OFL to ABC (RBC to TAC) - tier 2", "", XML_TYPE_FLOAT, "0.85");
	Create_RBC_Species_ParamXML(bm, fileName, fp, childGroupingNode, Tier3Sig_id, "Scalar to apply when taking OFL to ABC (RBC to TAC) - tier 3", "", XML_TYPE_FLOAT, "0.7");

    Create_RBC_Species_ParamXML(bm, fileName, fp, childGroupingNode, isTriggerSpecies_id, "is a Trigger species for the multispecies rules", "", XML_TYPE_INTEGER, "0");
    Create_RBC_Species_ParamXML(bm, fileName, fp, childGroupingNode, trigger_threshold_id, "Change taht triggers a nreakout for the trigger based assessments", "", XML_TYPE_FLOAT, "0");
    Create_RBC_Species_ParamXML(bm, fileName, fp, childGroupingNode, UseRBCAveraging_id, "flag tp indicate RBC averaging", "", XML_TYPE_INTEGER, "0");

    printf("Leaving create\n");
}

void assessmentParamsXML(MSEBoxModel *bm, FILE *fp, char *fileName, xmlDocPtr doc, xmlNodePtr rootnode) {

	xmlNodePtr groupingNode, node, lookupNode;
	groupingNode = Util_XML_Create_Node(ATLANTIS_ATTRIBUTE_SUB_GROUP, rootnode, "Assessment_Parameters", "", "", "");

	Util_XML_Parse_Create_Node(fp, fileName, groupingNode, "Assess_Tol", "Fractional convergence tolerance of minimisation", "", XML_TYPE_FLOAT, "1");
	Util_XML_Parse_Create_Node(fp, fileName, groupingNode, "Assess_Btol", "Acceptable proportional buffer on convergence", "", XML_TYPE_INTEGER, "1");
	Util_XML_Parse_Create_Node(fp, fileName, groupingNode, "Assess_Max_Int", "Maxiumum number of iterations of Amoeba ", "", XML_TYPE_INTEGER, "1");
	Util_XML_Parse_Create_Node(fp, fileName, groupingNode, "AssessNyr", "Length of longrun projections in Fref estimation", "", XML_TYPE_FLOAT, "1");

	node = Parse_File(bm, fp, fileName, groupingNode, "flag_assess", "flag_assess", "Assessment parameter set-up for fished group", "", XML_TYPE_LOOKUPARRAY, bm->K_num_tot_sp,
			TRUE, Create_Fished_Species_XMLNodes, createFishedSpeciesParam);

	lookupNode = Util_XML_Create_Lookup_Node(node);
	Util_XML_Create_Lookup_Value_Node(lookupNode, "0", "None");
	Util_XML_Create_Lookup_Value_Node(lookupNode, "1", "Schafer model");
	Util_XML_Create_Lookup_Value_Node(lookupNode, "2", "Adapt VPA");
	Util_XML_Create_Lookup_Value_Node(lookupNode, "3", "MSVPA");
	Util_XML_Create_Lookup_Value_Node(lookupNode, "4", "CAB");
	Util_XML_Create_Lookup_Value_Node(lookupNode, "5", "CAB mimic");
	Util_XML_Create_Lookup_Value_Node(lookupNode, "6", "qlinear regression");
	Util_XML_Create_Lookup_Value_Node(lookupNode, "7", "CPUE linear regression");

	Parse_File(bm, fp, fileName, groupingNode, "assess_bootstrap", "assess_bootstrap", "", "", XML_TYPE_INTEGERARRAY, bm->K_num_tot_sp, TRUE, Create_Fished_Species_XMLNodes,
			createFishedSpeciesParam);
	Parse_File(bm, fp, fileName, groupingNode, "assess_nat_mort", "assess_nat_mort", "", "", XML_TYPE_FLOATARRAY, bm->K_num_tot_sp, TRUE, Create_Fished_Species_XMLNodes,
			createFishedSpeciesParam);
	Parse_File(bm, fp, fileName, groupingNode, "flag_prod_model", "flag_prod_model", "", "", XML_TYPE_INTEGERARRAY, bm->K_num_tot_sp, TRUE, Create_Fished_Species_XMLNodes,
			createFishedSpeciesParam);

	Parse_File(bm, fp, fileName, groupingNode, "top_pcnt", "top_pcnt", "", "", XML_TYPE_INTEGERARRAY, bm->K_num_tot_sp, TRUE, Create_Fished_Species_XMLNodes, createFishedSpeciesParam);
	Parse_File(bm, fp, fileName, groupingNode, "bot_pcnt", "bot_pcnt", "", "", XML_TYPE_INTEGERARRAY, bm->K_num_tot_sp, TRUE, Create_Fished_Species_XMLNodes, createFishedSpeciesParam);

	Parse_File(bm, fp, fileName, groupingNode, "assess_datastream", "assess_datastream", "", "", XML_TYPE_INTEGERARRAY, bm->K_num_tot_sp, TRUE, Create_Fished_Species_XMLNodes,
			createFishedSpeciesParam);
    
    Create_Harvest_Fishery_Group_ParamXML(bm, fp, fileName, groupingNode, assess_nf_id, "Mapping of fleet to assessment data streams", "", XML_TYPE_INTEGER);
}

void Convert_Assess_To_XML(MSEBoxModel *bm, char *fileName, char *outputFileName) {
	xmlDocPtr doc;
	xmlNodePtr rootnode;
	FILE *fp;

	/* Init libxml */
	xmlInitParser();
	LIBXML_TEST_VERSION

	if ((fp = Open_Input_File(bm->inputFolder,fileName, "rt")) == NULL) {
		quit("Cannot open assess input file. %s%s\n", bm->inputFolder, fileName);
	}

	doc = xmlNewDoc((unsigned char *) "1.0");

	if (doc == NULL) {
		fprintf(stderr, "Document not created successfully. \n");
		return;
	}

	rootnode = xmlNewDocNode(doc, NULL, (const xmlChar*) "AtlantisAssess", NULL);
	xmlDocSetRootElement(doc, rootnode);

	AssessFlagXML(bm, fp, fileName, doc, rootnode);
	ErrorStructureXML(bm, fp, fileName, doc, rootnode);
	BiasedSampleXML(bm, fp, fileName, doc, rootnode);
	ErrorVarianceXML(bm, fp, fileName, doc, rootnode);
	VertParamsXML(bm, fp, fileName, doc, rootnode);
	assessFishingSelectivityXML(bm, fp, fileName, doc, rootnode);
	//dietInformationXML(bm, fp, fileName, doc, rootnode);
	detritusXML(bm, fp, fileName, doc, rootnode);
	assessmentParamsXML(bm, fp, fileName, doc, rootnode);
    RAssessFlagXML(bm, fp, fileName, doc, rootnode);

	TierAssessmentXML(bm, fp, fileName, doc, rootnode);

	/* Save to the output file */
	xmlSaveFormatFileDestFolder(bm->destFolder, outputFileName, doc, 1);
	xmlFreeDoc(doc);
	fclose(fp);

	/* Shutdown libxml */
	xmlCleanupParser();

	return;
}

