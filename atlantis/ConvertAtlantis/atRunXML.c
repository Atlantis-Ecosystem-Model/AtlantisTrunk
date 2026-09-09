/*
 * atRunXML.c
 *
 *  Created on: 06/05/2011
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

void Convert_Run_To_XML(MSEBoxModel *bm, char *fileName, char *outputFileName) {
	xmlDocPtr doc;
	xmlNodePtr rootnode;
	xmlNodePtr groupingNode;
	FILE *fp;
	xmlNodePtr node, lookupNode;


	/* Init libxml */
	xmlInitParser();
	LIBXML_TEST_VERSION

	if ((fp = Open_Input_File(bm->inputFolder, fileName, "rt")) == NULL) {
		quit("Cannot open run input file %s%s\n", bm->inputFolder, fileName);
	}

	doc = xmlNewDoc((unsigned char *) "1.0");

	if (doc == NULL) {
		fprintf(stderr, "Document not created successfully. \n");
		return;
	}

	rootnode = xmlNewDocNode(doc, NULL, (const xmlChar*) AtlantisXMLObjectNAMES[ATLANTIS_ATTRIBUTE_GROUP], NULL);
	Util_XML_Add_Node_Property(rootnode, AtlantisXMLObjectNAMES[ATLANTIS_ATTRIBUTE_GROUP][ATTRIBUTE_NAME], "AtlantisRunParameters");

	xmlDocSetRootElement(doc, rootnode);

	groupingNode = Util_XML_Create_Node(ATLANTIS_ATTRIBUTE_SUB_GROUP, rootnode, "SubModelSetup", "Determine which sub-models are active", "", "");
    
    Util_XML_Parse_Create_Node(fp, fileName, groupingNode, "flagecon_on", "Flag showing whether want economics loaded and submodel run (1) or not (0)", "",
			XML_TYPE_BOOLEAN, "1");
	Util_XML_Parse_Create_Node(fp, fileName, groupingNode, "flag_fisheries_on", "Flag showing whether want fisheries loaded and submodel run (1) or not (0)",
			"", XML_TYPE_BOOLEAN,"1");
	Util_XML_Parse_Create_Node(fp, fileName, groupingNode, "flag_skip_biol",
			"Flag showing whether want biological model run (1) or not (0 - only used when debugging fisheries)", "", XML_TYPE_BOOLEAN,"0");
	Util_XML_Parse_Create_Node(fp, fileName, groupingNode, "flag_skip_phys",
			"Flag showing whether want physics loaded and submodel run (1) or not (0). Should only be used for debugging.", "", XML_TYPE_BOOLEAN,"1");

	Util_XML_Parse_Create_Node(fp, fileName, groupingNode, "check_dups",
				"Turn on to check for duplicates in prm files. Turning on will significantly slow down prm file read in - only use from time to time.", "", XML_TYPE_BOOLEAN,"1");
    
    Util_XML_Parse_Create_Node(fp, fileName, groupingNode, "flag_migration_on", "Flag showing whether want migrations loaded and submodel run (1) or not (0)",
                               "", XML_TYPE_BOOLEAN,"1");
    bm->flag_migration_on = (int) Util_XML_Read_Value(fileName, ATLANTIS_ATTRIBUTE, bm->ecotest, 1, groupingNode, binary_check, "flag_migration_on");

	groupingNode = Util_XML_Create_Node(ATLANTIS_ATTRIBUTE_SUB_GROUP, rootnode, "ScenarioOptions", "Run settings", "", "");

	Util_XML_Parse_Create_Node(fp, fileName, groupingNode, "title", "ScenarioName", "", XML_TYPE_STRING, "");
	Util_XML_Parse_Create_Node(fp, fileName, groupingNode, "dt", "Model Time Step in hours", "Hours", XML_TYPE_INTEGER, "12");
	Util_XML_Parse_Create_Node(fp, fileName, groupingNode, "tstop", "Stop model after the given number of days", "", XML_TYPE_INTEGER, "0");

	Util_XML_Parse_Create_Node(fp, fileName, groupingNode, "flagIsEstuary", "Flag indicating model is an estuary.", "", XML_TYPE_BOOLEAN,"0");
	Util_XML_Parse_Create_Node(fp, fileName, groupingNode, "flagAllowLand", "Flag indicating model can include land.", "", XML_TYPE_BOOLEAN,"0");
    Util_XML_Parse_Create_Node(fp, fileName, groupingNode, "flagIsPolar", "Flag indicating model is polar", "", XML_TYPE_BOOLEAN,"0");
    
    Util_XML_Parse_Create_Node(fp, fileName, groupingNode, "flagpassivetracer", "Flag indicating want a passive tracer to track to check transports", "", XML_TYPE_BOOLEAN,"0");

    Util_XML_Parse_Create_Node(fp, fileName, groupingNode, "flag_pollutant_impacts", "Whether including noise and light pollution", "", XML_TYPE_BOOLEAN,"0");

	Util_XML_Parse_Create_Node(fp, fileName, groupingNode, "fishmove", "Set to 0 to turn vertebrate movement off for debugging purposes", "", XML_TYPE_BOOLEAN,"1");
	Util_XML_Parse_Create_Node(fp, fileName, groupingNode, "flaghemisphere", "Flag for hemisphere model is in (0 = southern; 1 = northern).", "", XML_TYPE_BOOLEAN,"0");

	Util_XML_Parse_Create_Node(fp, fileName, groupingNode, "trackAtomicRatio", "Flag to turn on tracking atomic ratios.", "", XML_TYPE_BOOLEAN,"0");
    bm->track_atomic_ratio = (int) Util_XML_Read_Value(fileName, ATLANTIS_ATTRIBUTE, bm->ecotest, 1, groupingNode, binary_check, "trackAtomicRatio");
    if(bm->track_atomic_ratio) {
        Util_XML_Parse_Create_Node(fp, fileName, groupingNode, "flagratio_warn", "Flag to turn on atomic ratio warning messages.", "", XML_TYPE_BOOLEAN,"0");
        Util_XML_Parse_Create_Node(fp, fileName, groupingNode, "N_to_C", "Base ratio of Nitrogen to Carbon.", "", XML_TYPE_FLOAT,"0");
        Util_XML_Parse_Create_Node(fp, fileName, groupingNode, "N_to_P", "Base ratio of Nitrogen to Phosphorous.", "", XML_TYPE_FLOAT,"0");
    }
    
    Util_XML_Parse_Create_Node(fp, fileName, groupingNode, "trackWind", "Flag to turn on tracking wind using forcing files.", "", XML_TYPE_BOOLEAN,"0");
    bm->track_wind = (int) Util_XML_Read_Value(fileName, ATLANTIS_ATTRIBUTE, bm->ecotest, 1, groupingNode, binary_check, "trackWind");

    Util_XML_Parse_Create_Node(fp, fileName, groupingNode, "external_populations", "Flag indicating use of external population model", "", XML_TYPE_BOOLEAN,"0");
    Util_XML_Parse_Create_Node(fp, fileName, groupingNode, "flag_multiyr_migs", "Flag to turn on multiyear migrations.", "", XML_TYPE_BOOLEAN,"0");

    Util_XML_Parse_Create_Node(fp, fileName, groupingNode, "track_rugosity_arag", "Flag to turn on tracking of rugosity and aragonite saturaiton.", "", XML_TYPE_BOOLEAN,"0");
	Util_XML_Parse_Create_Node(fp, fileName, groupingNode, "track_pH", "Flag to turn on tracking of pH.", "", XML_TYPE_BOOLEAN,"0");
    Util_XML_Parse_Create_Node(fp, fileName, groupingNode, "flag_use_deltaH", "Flag to track delta H not pH when calculating pHcorr.", "", XML_TYPE_BOOLEAN,"0");

	Util_XML_Parse_Create_Node(fp, fileName, groupingNode, "mirror_invalid", "Flag to indicate how to deal with invalid hydrodynamic exchanges.", "", XML_TYPE_BOOLEAN,"0");

	Util_XML_Parse_Create_Node(fp, fileName, groupingNode, "flag_replicated_old", "Flag to replicate old model results before the trunk code was merged.", "", XML_TYPE_BOOLEAN,"0");
    Util_XML_Parse_Create_Node(fp, fileName, groupingNode, "flag_replicated_old_PPmort", "Flag to replicate old model code assumptions around primary production mortality before the trunk code was merged.", "", XML_TYPE_BOOLEAN,"0");
    
    Util_XML_Parse_Create_Node(fp, fileName, groupingNode, "flag_old_embryo_init", "Flag to use the old means of having embryoes carried over from spawning pre-model start.", "", XML_TYPE_BOOLEAN,"0");
    
    Util_XML_Parse_Create_Node(fp, fileName, groupingNode, "flag_replicate_old_calendar", "Flag to replicate old calendar for aging and spawning - when done once per year rather than having 1 queue for the entire model run.", "", XML_TYPE_BOOLEAN,"0");
    Util_XML_Parse_Create_Node(fp, fileName, groupingNode, "flag_sanity_check", "Flag to trigger sanity checks.", "", XML_TYPE_BOOLEAN,"0");
    Util_XML_Parse_Create_Node(fp, fileName, groupingNode, "store_aggregate_yoy", "Flag as to what YOY stored", "", XML_TYPE_BOOLEAN,"0");
    Util_XML_Parse_Create_Node(fp, fileName, groupingNode, "store_mig_array", "Flag as to what Migration Array stored", "", XML_TYPE_BOOLEAN,"0");
    
    /* Removed as was chucking up an odd ATTRIBUTE read that was had to sort out so just left the information with the Scenario options instead
 	groupingNode = Util_XML_Create_Node(ATLANTIS_ATTRIBUTE_SUB_GROUP, rootnode, "ContaminantSettings", "Contaminant Settings", "", "");
    */
    Util_XML_Parse_Create_Node(fp, fileName, groupingNode, "track_contaminants", "Flag to turn on tracking of contaminants", "", XML_TYPE_BOOLEAN,"0");
    bm->track_contaminants = (int) Util_XML_Read_Value(fileName, ATLANTIS_ATTRIBUTE, bm->ecotest, 1, groupingNode, integer_check, "track_contaminants");
  
	if(bm->track_contaminants == TRUE){
        Util_XML_Parse_Create_Node(fp, fileName, groupingNode, "flag_contam_sanity_check", "Flag to turn on checking of contaminant fluxes.", "", XML_TYPE_BOOLEAN,"0");
		Util_XML_Parse_Create_Node(fp, fileName, groupingNode, "num_contaminants", "The number of contaminants to track.", "", XML_TYPE_INTEGER,"0");

		Parse_File(bm, fp, fileName, groupingNode, "Contaminant_Names", "^contaminant_names",
					"The names of the contaminant tracers.",
					"", XML_TYPE_STRINGARRAY, -1, FALSE, NULL, NULL);

		Parse_File(bm, fp, fileName, groupingNode, "Contaminant_Units", "^contaminant_units",
					"The units of the contaminant tracers.",
					"", XML_TYPE_STRINGARRAY, -1, FALSE, NULL, NULL);
        
	}

	groupingNode = Util_XML_Create_Node(ATLANTIS_ATTRIBUTE_SUB_GROUP, rootnode, "OutputOptions", "Model Output Settings", "", "");
	Util_XML_Parse_Create_Node(fp, fileName, groupingNode, "toutstart", "Output start day", "", XML_TYPE_FLOAT, "0");
	Util_XML_Parse_Create_Node(fp, fileName, groupingNode, "toutinc", "Write output with this periodicity", "", XML_TYPE_FLOAT, "0");
	Util_XML_Parse_Create_Node(fp, fileName, groupingNode, "toutfinc", "Write fisheries output with this periodicity", "", XML_TYPE_FLOAT, "0");
	Util_XML_Parse_Create_Node(fp, fileName, groupingNode, "tburnday", "Day of run where burn-in conditions relaxed", "", XML_TYPE_FLOAT, "0");

	Util_XML_Parse_Create_Node(fp, fileName, groupingNode, "tsumout", "Write stock state summary with this periodicity", "", XML_TYPE_FLOAT, "0");
	Util_XML_Parse_Create_Node(fp, fileName, groupingNode, "fishout", "Switch to turn fisheries output on = 1, off = 0", "", XML_TYPE_BOOLEAN,"0");
	Util_XML_Parse_Create_Node(fp, fileName, groupingNode, "flag_age_output", "Age structured outputt on = 1, off = 0", "", XML_TYPE_BOOLEAN, "0");

	node = Util_XML_Parse_Create_Node(fp, fileName, groupingNode, "flagreusefile", "Switch to show want to append output file no = 0, yes = 1, replace = 2", "", XML_TYPE_LOOKUP, "0");

	lookupNode = Util_XML_Create_Lookup_Node(node);
	Util_XML_Create_Lookup_Value_Node(lookupNode, "0", "No");
	Util_XML_Create_Lookup_Value_Node(lookupNode, "1", "Yes");
	Util_XML_Create_Lookup_Value_Node(lookupNode, "2", "Replace");

	Util_XML_Parse_Create_Node(fp, fileName, groupingNode, "flagannual_Mest", "Write out mortality per predator annually (1) or not (0)", "", XML_TYPE_BOOLEAN,"0");


	groupingNode = Util_XML_Create_Node(ATLANTIS_ATTRIBUTE_SUB_GROUP, rootnode, "DiagnosticOutput", "The diagnostic output settings", "", "");
	Util_XML_Parse_Create_Node(fp, fileName, groupingNode, "verbose", "Detailed logged output", "", XML_TYPE_INTEGER, "0");
	Util_XML_Parse_Create_Node(fp, fileName, groupingNode, "checkbox", "Give detailed logged output for this box", "", XML_TYPE_INTEGER, "0");
	Util_XML_Parse_Create_Node(fp, fileName, groupingNode, "checkstart", "Start detailed logged output from this day in the model run", "", XML_TYPE_INTEGER, "0");
	Util_XML_Parse_Create_Node(fp, fileName, groupingNode, "checkstop", "Stop detailed logged output after this day in the model run", "", XML_TYPE_INTEGER, "0");
	Util_XML_Parse_Create_Node(fp, fileName, groupingNode, "fishtest", "Count up total population for each vertebrate after each main subroutine: 0=no, 1=yes",
			"", XML_TYPE_BOOLEAN,"0");

	Util_XML_Parse_Create_Node(fp, fileName, groupingNode, "flaggape", "Periodically list prey vs gape statistics (tuning diagnostic)", "", XML_TYPE_BOOLEAN,"0");
	Util_XML_Parse_Create_Node(fp, fileName, groupingNode, "flagchecksize", "Periodically list relative size (tuning diagnostic)", "", XML_TYPE_BOOLEAN,"0");
	Util_XML_Parse_Create_Node(fp, fileName, groupingNode, "flagagecheck", "Periodically list age structure per cohort (tuning diagnostic)", "", XML_TYPE_BOOLEAN,"0");
    Util_XML_Parse_Create_Node(fp, fileName, groupingNode, "flagdietcheck", "Whether want detailed diet output", "", XML_TYPE_BOOLEAN,"0");
    Util_XML_Parse_Create_Node(fp, fileName, groupingNode, "flagenviro_warn", "Whether want earnings anout environmental constraints on movement recorded to the logfile ", "", XML_TYPE_BOOLEAN,"0");
   
    Util_XML_Parse_Create_Node(fp, fileName, groupingNode, "flag_mig_in_bioindx", "Whether to include biomass from migration arrays ", "", XML_TYPE_BOOLEAN,"0");

    Util_XML_Parse_Create_Node(fp, fileName, groupingNode, "checkNH", "Give detailed logged output for NH in checkbox", "", XML_TYPE_BOOLEAN,"0");
    Util_XML_Parse_Create_Node(fp, fileName, groupingNode, "checkDL", "Give detailed logged output for DL in checkbox", "", XML_TYPE_BOOLEAN,"0");
	Util_XML_Parse_Create_Node(fp, fileName, groupingNode, "checkDR", "Give detailed logged output for DR in checkbox", "", XML_TYPE_BOOLEAN,"0");
    
    //Util_XML_Parse_Create_Node(fp, fileName, groupingNode, "masscheck", "Give conservation of mass check for species defined by which_check", "", XML_TYPE_BOOLEAN,"0");
	Util_XML_Parse_Create_Node(fp, fileName, groupingNode, "checkbiom", "Give detailed logged output for biomasses in checkbox", "", XML_TYPE_BOOLEAN,"0");
	Util_XML_Parse_Create_Node(fp, fileName, groupingNode, "which_fleet",
			"ID number of fleet to track (if don't want to track anything set to a value greater than the number of fleets)", "", XML_TYPE_INTEGER, "0");

	Util_XML_Parse_Create_Node(fp, fileName, groupingNode, "which_check",
			"ID number of group to track (if don't want to track anything set to a value greater than the number of groups)", "", XML_TYPE_INTEGER, "0");
	node = Util_XML_Parse_Create_Node(fp, fileName, groupingNode, "habitat_check", "Which habitat to check the above group in.", "", XML_TYPE_LOOKUP, "0");
	lookupNode = Util_XML_Create_Lookup_Node(node);
	Util_XML_Create_Lookup_Value_Node(lookupNode, "0", "Water Column");
	Util_XML_Create_Lookup_Value_Node(lookupNode, "1", "Sediment");
	Util_XML_Create_Lookup_Value_Node(lookupNode, "2", "Epibenthic Layer");

	Util_XML_Parse_Create_Node(fp, fileName, groupingNode, "move_check",
				"ID number of group where tracking movement (if don't want to track anything set to a value greater than the number of groups)", "", XML_TYPE_INTEGER, "0");

	node = Util_XML_Parse_Create_Node(fp, fileName, groupingNode, "debug", "Debugging option", "", XML_TYPE_BOOLEAN,"0");

	lookupNode = Util_XML_Create_Lookup_Node(node);
	Util_XML_Create_Lookup_Value_Node(lookupNode, "0", "No Debugging");
	Util_XML_Create_Lookup_Value_Node(lookupNode, "1", "Debug Fishing");
	Util_XML_Create_Lookup_Value_Node(lookupNode, "2", "Debug Discards");
	Util_XML_Create_Lookup_Value_Node(lookupNode, "3", "Debug Historical Fishing");
	Util_XML_Create_Lookup_Value_Node(lookupNode, "4", "Debug Assessments");
	Util_XML_Create_Lookup_Value_Node(lookupNode, "5", "Debug MPAs");
	Util_XML_Create_Lookup_Value_Node(lookupNode, "6", "Debug Effort");
	Util_XML_Create_Lookup_Value_Node(lookupNode, "7", "Debug Economic Effort");
	Util_XML_Create_Lookup_Value_Node(lookupNode, "8", "Debug Economics");
	Util_XML_Create_Lookup_Value_Node(lookupNode, "9", "Debug Quota Calculations");
	Util_XML_Create_Lookup_Value_Node(lookupNode, "10", "Debug Aging");
	Util_XML_Create_Lookup_Value_Node(lookupNode, "11", "Debug Recruitment");
	Util_XML_Create_Lookup_Value_Node(lookupNode, "12", "Debug Spawning");
	Util_XML_Create_Lookup_Value_Node(lookupNode, "13", "Debug Migration");
	Util_XML_Create_Lookup_Value_Node(lookupNode, "14", "Debug Movement");
	Util_XML_Create_Lookup_Value_Node(lookupNode, "15", "Debug Stocks");
	Util_XML_Create_Lookup_Value_Node(lookupNode, "16", "Debug Biomass Calculations");
	Util_XML_Create_Lookup_Value_Node(lookupNode, "17", "Debug Feeding");
	Util_XML_Create_Lookup_Value_Node(lookupNode, "18", "Debug everything");
	Util_XML_Create_Lookup_Value_Node(lookupNode, "19", "Not Used");
	Util_XML_Create_Lookup_Value_Node(lookupNode, "20", "Debug Biological Process Functions - Huge log files created.");
	Util_XML_Create_Lookup_Value_Node(lookupNode, "21", "Debug Group Process Functions - Huge log files created.");
	Util_XML_Create_Lookup_Value_Node(lookupNode, "22", "Debug CLAM-Atlantis Linkage");
	Util_XML_Create_Lookup_Value_Node(lookupNode, "23", "Debug Deposition");
	Util_XML_Create_Lookup_Value_Node(lookupNode, "24", "Debug Mortality Scalar Code");

	Util_XML_Parse_Create_Node(fp, fileName, groupingNode, "debug_it", "", "", XML_TYPE_BOOLEAN,"0");
	Util_XML_Parse_Create_Node(fp, fileName, groupingNode, "flagdetailedcatch", "", "", XML_TYPE_BOOLEAN, "0");

	groupingNode = Util_XML_Create_Node(ATLANTIS_ATTRIBUTE_SUB_GROUP, rootnode, "GroupOptions", "Parameters defining the numbers of certain types of groups.", "", "");
	Util_XML_Parse_Create_Node(fp, fileName, groupingNode, "K_num_tot_sp", "Total number of groups in the model", "", XML_TYPE_INTEGER, "0");
	//Util_XML_Parse_Create_Node(fp, fileName, groupingNode, "K_num_vert_sp", "total number of vertebrate groups in the model", "", XML_TYPE_INTEGER, "0");
	Util_XML_Parse_Create_Node(fp, fileName, groupingNode, "K_num_stocks_per_sp", "Maximum number of stocks per group in the model", "", XML_TYPE_INTEGER, "0");
	bm->K_num_stocks_per_sp = (int) Util_XML_Read_Value(fileName, ATLANTIS_ATTRIBUTE, bm->ecotest, 1, groupingNode, integer_check, "K_num_stocks_per_sp");
    
    Util_XML_Parse_Create_Node(fp, fileName, groupingNode, "K_num_detritus", "Total number of detritus groups in the model (labile and refractory; water column and sediments)", "", XML_TYPE_INTEGER, "0");
	Util_XML_Parse_Create_Node(fp, fileName, groupingNode, "K_num_bed_types", "Maximum number of seabed types (currently only reef, soft and flat)", "", XML_TYPE_INTEGER, "0");
	Util_XML_Parse_Create_Node(fp, fileName, groupingNode, "K_num_cover_types", "Maximum number of habitat types", "", XML_TYPE_INTEGER, "0");

	groupingNode = Util_XML_Create_Node(ATLANTIS_ATTRIBUTE_SUB_GROUP, rootnode, "FishingOptions", "Parameters the number of fleets.", "", "");

	Util_XML_Parse_Create_Node(fp, fileName, groupingNode, "K_num_catchqueue", "Length of the catch queue in days for use in running average of recent catch (typically a week or month)", "", XML_TYPE_INTEGER, "1");
	Util_XML_Parse_Create_Node(fp, fileName, groupingNode, "K_num_fisheries", "Maximum number of fisheries", "", XML_TYPE_INTEGER, "33");
	Util_XML_Parse_Create_Node(fp, fileName, groupingNode, "K_num_ports", "Number of ports in the model area", "", XML_TYPE_INTEGER, "5");

	Util_XML_Parse_Create_Node(fp, fileName, groupingNode, "K_num_reg", "Maximum number of management zonings used in the model", "", XML_TYPE_INTEGER, "5");
	Util_XML_Parse_Create_Node(fp, fileName, groupingNode, "K_num_markets", "Maxiumum number of markets in the model area", "", XML_TYPE_INTEGER, "5");
    Util_XML_Parse_Create_Node(fp, fileName, groupingNode, "K_num_basket", "Maxiumum number of species in basket quota", "", XML_TYPE_INTEGER, "5");
    
	groupingNode = Util_XML_Create_Node(ATLANTIS_ATTRIBUTE_SUB_GROUP, rootnode, "ScaleGroupDensities", "Scale the vertebrate groups in the model. Only values for the vertebrate groups are used- other groups should just be 1.0", "", "");
	Util_XML_Parse_Create_Node(fp, fileName, groupingNode, "flagscaleinit", "Flag to indicate whether scaling initial vertebrate densities (1=yes, 0=no)", "", XML_TYPE_BOOLEAN,"1");

	Util_XML_Create_Node_Next_Line(fp, fileName, groupingNode, "init_scalar", "Scale the vertebrate groups in the model. Only values for the vertebrate groups are used- other groups should just be 1.0", "",
			XML_TYPE_FLOATARRAY);




	/* Save to the output file */
	xmlSaveFormatFileDestFolder(bm->destFolder, outputFileName, doc, 1);
	xmlFreeDoc(doc);
	fclose(fp);

	/* Shutdown libxml */
	xmlCleanupParser();

	return;
}
