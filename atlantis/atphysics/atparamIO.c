/*******************************************************************//**
 \file
 \brief  Routine to read Box Model parameters
 File:           atparamIO.c

 Created:        Mon Nov 21 13:58:12 EST 1994

 Author:         Stephen Walker
 CSIRO Division of Oceanography

 Purpose:        Routine to read Box Model parameters. These parameters
 may change from run to run, but are usually constant for any given
 run. The parameters are assumed to be in an ascii file,
 where blank lines and lines beginning with # (comment
 lines) are ignored, and each line containing
 parameter data starts with a key which is used to find
 the parameter. For example:

 # End time for model run
 tstop	86400

 Arguments:      name - Name of parameter file.
 bm    - Pointer to box model.

 Returns:        void

 Revisions:      8/8/2004 EA Fulton
 Ported across the code from the southeast (sephys) model

 21/8/2004 EA Fulton
 Added essential biological and fisheries parameters to
 end of read-in in readRunParams()

 17/11/2004 EA Fulton
 Converted original routine definitions from
 void
 routine_name(blah,blahblah)
 int blah;
 double blahblah;

 to

 void routine_name(int blah, double blahblah)


 29/3/2004 Added bm->DIN_id so can track "stock size" of DIN,
 set this dynamically to K_num_tot_sp rather than defined
 constant as always want tit to be the highest/last entry
 in the totbiom, totfishpop, totinitpop arrays.


 22-04-2009 Bec Gorton
 Added the habitat check so the user can specify the habitat that
 they want the which_check species diagnostic output for. So from now in
 if you want the DR values in the sediment use the habitat_check value of 1 and
 a which_check value of the index of DR.
 Removed the bm->spSTRNAME array.


 23-07-2009 Bec Gorton
 Added the new flagIsEstuary flag to the run prm file. If this is true
 the initial conditions file must have the new SED tracer - this is then
 used in the light attenuation equation in biophysics to model light attenuation
 due to sediment in the water column.

 28-10-2009 Bec Gorton
 Removed the code that set the bm->K_num_fishedtac_sp value.

 30-10-2009 Bec Gorton
 Removed the K_num_invcohorts value.

 *********************************************************************/
#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>
#include <math.h>
#include <string.h>
#include <sjwlib.h>
#include <atlantisboxmodel.h>
#include <atUtilLib.h>
#include <convertXML.h>

static int Read_Run_Parameters(MSEBoxModel *bm, char *filename);

int Util_Read_Run_XML(MSEBoxModel *bm, char *fileName) {

	char biologyfile[120];
	char *convertedXMLFileName = (char *)malloc(sizeof(char) * strlen(fileName) + 1);

	strcpy(biologyfile, bm->runprmIfname);

	/* Check to see if we are reading in an XML file or a prm file */
	if (strstr(biologyfile, ".xml") == NULL) {
		/* Build the converted filename */
		sprintf(convertedXMLFileName, "%s", biologyfile);
		*(strstr(convertedXMLFileName, ".prm")) = '\0';
		strcat(convertedXMLFileName, ".xml");

		printf("convertedXMLFileName = %s\n", convertedXMLFileName);

		/* Convert the input file to XML */
		Convert_Run_To_XML(bm, biologyfile, convertedXMLFileName);
	} else {
		sprintf(convertedXMLFileName, "%s", biologyfile);
	}

	Read_Run_Parameters(bm, convertedXMLFileName);
    
	free(convertedXMLFileName);
	return TRUE;
}

/**
 *	This routine reads all parameters of the ecological models
 *
 *  Input:  char *filename: pointer to biology input XML file.
 *
 */
static int Read_Run_Parameters(MSEBoxModel *bm, char *fileName) {
	xmlDocPtr inputDoc = NULL;
	int dt_scale, cIndex;
	xmlNodePtr groupingNode;
	char **values;

	inputDoc = xmlReadFileDestFolder(bm->destFolder, fileName, NULL, 0);

	groupingNode = Util_XML_Get_Node(ATLANTIS_ATTRIBUTE_SUB_GROUP, inputDoc->children, "SubModelSetup");
	if (groupingNode == NULL)
		quit("Setup attribute group not found in input file %s.\n", fileName);

	bm->flagecon_on = (int) Util_XML_Read_Value(fileName, ATLANTIS_ATTRIBUTE, bm->ecotest, 1, groupingNode, binary_check, "flagecon_on");
	bm->flag_fisheries_on = (int) Util_XML_Read_Value(fileName, ATLANTIS_ATTRIBUTE, bm->ecotest, 1, groupingNode, binary_check, "flag_fisheries_on");
    bm->flag_migration_on = (int) Util_XML_Read_Value(fileName, ATLANTIS_ATTRIBUTE, bm->ecotest, 1, groupingNode, binary_check, "flag_migration_on");
	bm->flag_skip_biol = (int) Util_XML_Read_Value(fileName, ATLANTIS_ATTRIBUTE, bm->ecotest, 1, groupingNode, binary_check, "flag_skip_biol");
	bm->flag_skip_phys = (int) Util_XML_Read_Value(fileName, ATLANTIS_ATTRIBUTE, bm->ecotest, 1, groupingNode, binary_check, "flag_skip_phys");
	bm->check_dups = (int) Util_XML_Read_Value(fileName, ATLANTIS_ATTRIBUTE, bm->ecotest, 1, groupingNode, binary_check, "check_dups");

	if (!bm->flagecon_on)
		warn("flagecon_on in %s set to zero - assume economics model disabled\n", fileName);

	if (!bm->flag_fisheries_on)
		warn("flag_fisheries_on in %s set to zero - assume fisheries and management models disabled\n", fileName);

	if (bm->flagecon_on && !bm->flag_fisheries_on)
		quit("Can't have economic model (flagecon_on) active without fisheries (flag_fisheries_on) active - reset one or the other in %s\n", fileName);

	if (bm->flag_skip_biol)
		warn("flag_skip_biol in %s set to one - biology is disabled\n", fileName);

	if (bm->flag_skip_phys)
		warn("flag_skip_phys in %s set to one - physics is disabled\n", fileName);

	if(bm->check_dups  == TRUE){
		warn("check_dups in %s set to one - prm read in will be very very slow. Turn off once no duplicates are found.\n", fileName);
	}

	if(!do_economics && bm->flagecon_on){
		quit("Can't have economic model (flagecon_on) active without providing an economics input file.\n");
	}

    groupingNode = Util_XML_Get_Node(ATLANTIS_ATTRIBUTE_SUB_GROUP, inputDoc->children, "ScenarioOptions");
	if (groupingNode == NULL)
		quit("ScenarioOptions attribute group not found in input file %s.\n", fileName);

	Util_XML_Get_Value_String(fileName, ATLANTIS_ATTRIBUTE, 1, groupingNode, "title", bm->title);
	Util_XML_Get_Value_Time(fileName, ATLANTIS_ATTRIBUTE, bm->ecotest, 1, groupingNode, no_checking, "dt", &bm->dt);
	Util_XML_Get_Value_Time(fileName, ATLANTIS_ATTRIBUTE, bm->ecotest, 1, groupingNode, no_checking, "tstop", &bm->tstop);

	printf("tstop: %f\n", bm->tstop/86400.0);

	bm->flagIsEstuary = (int) Util_XML_Read_Value(fileName, ATLANTIS_ATTRIBUTE, bm->ecotest, 1, groupingNode, binary_check, "flagIsEstuary");
    bm->flagIsPolar = (int) Util_XML_Read_Value(fileName, ATLANTIS_ATTRIBUTE, bm->ecotest, 1, groupingNode, binary_check, "flagIsPolar");
    
    bm->flagpassivetracer = (int) Util_XML_Read_Value(fileName, ATLANTIS_ATTRIBUTE, bm->ecotest, 1, groupingNode, binary_check, "flagpassivetracer");
	bm->terrestrial_on = (int) Util_XML_Read_Value(fileName, ATLANTIS_ATTRIBUTE, bm->ecotest, 1, groupingNode, binary_check, "flagAllowLand");
    
    bm->flag_pollutant_impacts = (int) Util_XML_Read_Value(fileName, ATLANTIS_ATTRIBUTE, bm->ecotest, 1, groupingNode, binary_check, "flag_pollutant_impacts");

	bm->fishmove = (int) Util_XML_Read_Value(fileName, ATLANTIS_ATTRIBUTE, bm->ecotest, 1, groupingNode, binary_check, "fishmove");
    if (!bm->fishmove)
        warn("Vertebrate movement has been turned off (fishmove set to 0)\n");

	bm->flaghemisphere = (int) Util_XML_Read_Value(fileName, ATLANTIS_ATTRIBUTE, bm->ecotest, 1, groupingNode, binary_check, "flaghemisphere");

	/* Read in information about additional tracers */
	bm->track_atomic_ratio = (int) Util_XML_Read_Value(fileName, ATLANTIS_ATTRIBUTE, bm->ecotest, 1, groupingNode, binary_check, "trackAtomicRatio");
    if(bm->track_atomic_ratio) {
        bm->flagratio_warn = (int) Util_XML_Read_Value(fileName, ATLANTIS_ATTRIBUTE, bm->ecotest, 1, groupingNode, binary_check, "flagratio_warn");
        bm->N_to_C = Util_XML_Read_Value(fileName, ATLANTIS_ATTRIBUTE, bm->ecotest, 1, groupingNode, no_checking, "N_to_C");
        bm->N_to_P = Util_XML_Read_Value(fileName, ATLANTIS_ATTRIBUTE, bm->ecotest, 1, groupingNode, no_checking, "N_to_P");
    }
    
    bm->external_populations = (int) Util_XML_Read_Value(fileName, ATLANTIS_ATTRIBUTE, bm->ecotest, 1, groupingNode, binary_check, "external_populations");
    bm->flag_multiyr_migs = (int) Util_XML_Read_Value(fileName, ATLANTIS_ATTRIBUTE, bm->ecotest, 1, groupingNode, binary_check, "flag_multiyr_migs");
    
	bm->track_rugosity_arag = (int) Util_XML_Read_Value(fileName, ATLANTIS_ATTRIBUTE, bm->ecotest, 1, groupingNode, integer_check, "track_rugosity_arag");
	bm->track_pH = (int) Util_XML_Read_Value(fileName, ATLANTIS_ATTRIBUTE, bm->ecotest, 1, groupingNode, integer_check, "track_pH");
    bm->flag_use_deltaH = (int) Util_XML_Read_Value(fileName, ATLANTIS_ATTRIBUTE, bm->ecotest, 1, groupingNode, integer_check, "flag_use_deltaH");
    
    if(bm->track_rugosity_arag && !bm->track_pH)
        quit("You can not have track_rugosity_arag = %d and track_pH = %d, set track_pH to 1 to make this work or set track_rugosity_arag to zero if you want to disable it\n", bm->track_rugosity_arag, bm->track_pH );
    
    bm->mirror_invalid = (int) Util_XML_Read_Value(fileName, ATLANTIS_ATTRIBUTE, bm->ecotest, 1, groupingNode, integer_check, "mirror_invalid");
	
    bm->flag_replicated_old = (int) Util_XML_Read_Value(fileName, ATLANTIS_ATTRIBUTE, bm->ecotest, 1, groupingNode, binary_check, "flag_replicated_old");
    bm->flag_replicated_old_PPmort = (int) Util_XML_Read_Value(fileName, ATLANTIS_ATTRIBUTE, bm->ecotest, 1, groupingNode, binary_check, "flag_replicated_old_PPmort");

    bm->flag_old_embryo_init = (int) Util_XML_Read_Value(fileName, ATLANTIS_ATTRIBUTE, bm->ecotest, 1, groupingNode, binary_check, "flag_old_embryo_init");
    bm->flag_replicate_old_calendar = (int) Util_XML_Read_Value(fileName, ATLANTIS_ATTRIBUTE, bm->ecotest, 1, groupingNode, binary_check, "flag_replicate_old_calendar");
    bm->flag_sanity_check = (int) Util_XML_Read_Value(fileName, ATLANTIS_ATTRIBUTE, bm->ecotest, 1, groupingNode, binary_check, "flag_sanity_check");
    
    bm->store_aggregate_yoy = (int) Util_XML_Read_Value(fileName, ATLANTIS_ATTRIBUTE, bm->ecotest, 1, groupingNode, binary_check, "store_aggregate_yoy");
    bm->store_mig_array = (int) Util_XML_Read_Value(fileName, ATLANTIS_ATTRIBUTE, bm->ecotest, 1, groupingNode, binary_check, "store_mig_array");
    
    /* Read in the contaminant values */
    /* Removed as was chucking up an odd ATTRIBUTE read that was had to sort out so just left the information with the Scenario options instead
    groupingNode = Util_XML_Get_Node(ATLANTIS_ATTRIBUTE_SUB_GROUP, inputDoc->children, "ContaminantSettings");
    if (groupingNode == NULL)
        quit("ContaminantSettings attribute group not found in input file %s.\n", fileName);
     */
     //bm->track_contaminants = (int) Util_XML_Read_Value(fileName, ATLANTIS_ATTRIBUTE, bm->ecotest, 1, groupingNode, integer_check, "track_contaminants");

    if(bm->track_contaminants == TRUE){
        
        printf("Reading contaminant parameters\n");
        
        bm->flag_contam_sanity_check = (int) Util_XML_Read_Value(fileName, ATLANTIS_ATTRIBUTE, bm->ecotest, 1, groupingNode, integer_check, "flag_contam_sanity_check");
        bm->num_contaminants = (int) Util_XML_Read_Value(fileName, ATLANTIS_ATTRIBUTE, bm->ecotest, 1, groupingNode, integer_check, "num_contaminants");
        if (bm->num_contaminants <= 1)
            bm->flag_contamInteractModel = no_contam_interact;

        bm->contaminantStructure = (ContaminantStructure **)malloc(sizeof(ContaminantStructure *) * (unsigned long int)bm->num_contaminants);
        for(cIndex = 0; cIndex < bm->num_contaminants; cIndex++){
            bm->contaminantStructure[cIndex] = (ContaminantStructure *)malloc(sizeof(ContaminantStructure ));
        }
        
        printf("String about to read contaminant_names\n");
        
        /* Read in the names */
        if(Util_XML_Read_Array_String(ATLANTIS_ATTRIBUTE, fileName, "contaminant_names", groupingNode, "contaminant_names", &values, bm->num_contaminants) == FALSE){
            quit("Error: Unable to read parameter 'Contaminant_Names' from input file %s\n", fileName);
        }
        
        printf("String name set up\n");

        for(cIndex = 0; cIndex < bm->num_contaminants ; cIndex ++){
            strcpy(bm->contaminantStructure[cIndex]->contaminant_name, values[cIndex]);

            printf("name = %s, value = %s\n", values[cIndex], bm->contaminantStructure[cIndex]->contaminant_name);
        }
        printf("Loaded values and about to free values\n");
        
        //c_free2d(values);
        //printf("value = %s\n", bm->contaminantStructure[0]->contaminant_name);

        /* Read in the units */
        if(Util_XML_Read_Array_String(ATLANTIS_ATTRIBUTE, fileName, "contaminant_units", groupingNode, "contaminant_units", &values,
                                bm->num_contaminants) == FALSE){
                    quit("Error: Unable to read parameter 'Contaminant_Units' from input file %s\n", fileName);
                }

        for(cIndex = 0; cIndex < bm->num_contaminants ; cIndex ++){
            strcpy(bm->contaminantStructure[cIndex]->contaminant_unit , values[cIndex]);
        }

        c_free2d(values);
    }

    
	groupingNode = Util_XML_Get_Node(ATLANTIS_ATTRIBUTE_SUB_GROUP, inputDoc->children, "OutputOptions");
	if (groupingNode == NULL)
		quit("OutputOptions attribute group not found in input file %s.\n", fileName);

	Util_XML_Get_Value_Time(fileName, ATLANTIS_ATTRIBUTE, bm->ecotest, 1, groupingNode, no_checking, "toutstart", &bm->toutstart);
	Util_XML_Get_Value_Time(fileName, ATLANTIS_ATTRIBUTE, bm->ecotest, 1, groupingNode, no_checking, "toutinc", &bm->toutinc);
	Util_XML_Get_Value_Time(fileName, ATLANTIS_ATTRIBUTE, bm->ecotest, 1, groupingNode, no_checking, "toutfinc", &bm->toutfinc);
	Util_XML_Get_Value_Time(fileName, ATLANTIS_ATTRIBUTE, bm->ecotest, 1, groupingNode, no_checking, "tburnday", &bm->tburnday);
	Util_XML_Get_Value_Time(fileName, ATLANTIS_ATTRIBUTE, bm->ecotest, 1, groupingNode, no_checking, "tsumout", &bm->tsumout);

	bm->flag_age_output = (int) Util_XML_Read_Value(fileName, ATLANTIS_ATTRIBUTE, bm->ecotest, 1, groupingNode, binary_check, "flag_age_output");
	bm->fishout = (int) Util_XML_Read_Value(fileName, ATLANTIS_ATTRIBUTE, bm->ecotest, 1, groupingNode, binary_check, "fishout");

    if (bm->flag_age_output < 2)
        warn("Only txt based age out will be output. Set this to 2 for netcdf output\n");

    if(bm->fishout && !bm->flag_fisheries_on){
		warn("Can't have fishout turned on without the fishery model on. Have turned off fishout\n");
		bm->fishout = FALSE;
	}
	bm->flagreusefile = (int) Util_XML_Read_Value(fileName, ATLANTIS_ATTRIBUTE, bm->ecotest, 1, groupingNode, integer_check, "flagreusefile");
	bm->flagannual_Mest = (int) Util_XML_Read_Value(fileName, ATLANTIS_ATTRIBUTE, bm->ecotest, 1, groupingNode, binary_check, "flagannual_Mest");
    
	if (!bm->flagannual_Mest)
		warn("flagannual_Mest set to %d, this means that estimates based on less than an annual cycle of the model will be included in the output file\n",
				bm->flagannual_Mest);

	groupingNode = Util_XML_Get_Node(ATLANTIS_ATTRIBUTE_SUB_GROUP, inputDoc->children, "DiagnosticOutput");
	if (groupingNode == NULL)
		quit("DiagnosticOutput attribute group not found in input file %s.\n", fileName);

	verbose = (int) Util_XML_Read_Value(fileName, ATLANTIS_ATTRIBUTE, bm->ecotest, 1, groupingNode, integer_check, "verbose");
	bm->checkbox = (int) Util_XML_Read_Value(fileName, ATLANTIS_ATTRIBUTE, bm->ecotest, 1, groupingNode, integer_check, "checkbox");
	//bm->checkstart = (int) Util_XML_Read_Value(fileName, ATLANTIS_ATTRIBUTE, bm->ecotest, 1, groupingNode, integer_check, "checkstart");

	Util_XML_Get_Value_Time(fileName, ATLANTIS_ATTRIBUTE, bm->ecotest, 1, groupingNode, no_checking, "checkstart", &bm->checkstart);
	Util_XML_Get_Value_Time(fileName, ATLANTIS_ATTRIBUTE, bm->ecotest, 1, groupingNode, no_checking, "checkstop", &bm->checkstop);

//	bm->checkstop = (int) Util_XML_Read_Value(fileName, ATLANTIS_ATTRIBUTE, bm->ecotest, 1, groupingNode, integer_check, "checkstop");

	/* Correct checkstart and checkstop from seconds to days */
	bm->checkstart /= 86400.0;
	bm->checkstop /= 86400.0;

	fishtest = (int) Util_XML_Read_Value(fileName, ATLANTIS_ATTRIBUTE, bm->ecotest, 1, groupingNode, integer_check, "fishtest");
	bm->flaggape = (int) Util_XML_Read_Value(fileName, ATLANTIS_ATTRIBUTE, bm->ecotest, 1, groupingNode, binary_check, "flaggape");
	bm->flagchecksize = (int) Util_XML_Read_Value(fileName, ATLANTIS_ATTRIBUTE, bm->ecotest, 1, groupingNode, binary_check, "flagchecksize");
	bm->flagagecheck = (int) Util_XML_Read_Value(fileName, ATLANTIS_ATTRIBUTE, bm->ecotest, 1, groupingNode, binary_check, "flagagecheck");
    bm->flagdietcheck = (int) Util_XML_Read_Value(fileName, ATLANTIS_ATTRIBUTE, bm->ecotest, 1, groupingNode, binary_check, "flagdietcheck");
    bm->flagenviro_warn = (int) Util_XML_Read_Value(fileName, ATLANTIS_ATTRIBUTE, bm->ecotest, 1, groupingNode, binary_check, "flagenviro_warn");

    bm->flag_mig_in_bioindx = (int) Util_XML_Read_Value(fileName, ATLANTIS_ATTRIBUTE, bm->ecotest, 1, groupingNode, binary_check, "flag_mig_in_bioindx");

	bm->checkNH = (int) Util_XML_Read_Value(fileName, ATLANTIS_ATTRIBUTE, bm->ecotest, 1, groupingNode, binary_check, "checkNH");
	bm->checkDL = (int) Util_XML_Read_Value(fileName, ATLANTIS_ATTRIBUTE, bm->ecotest, 1, groupingNode, binary_check, "checkDL");
	bm->checkDR = (int) Util_XML_Read_Value(fileName, ATLANTIS_ATTRIBUTE, bm->ecotest, 1, groupingNode, binary_check, "checkDR");
    //bm->masscheck = (int) Util_XML_Read_Value(fileName, ATLANTIS_ATTRIBUTE, bm->ecotest, 1, groupingNode, binary_check, "masscheck");
	bm->checkbiom = (int) Util_XML_Read_Value(fileName, ATLANTIS_ATTRIBUTE, bm->ecotest, 1, groupingNode, binary_check, "checkbiom");
    //bm->masscheck = (int) Util_XML_Read_Value(fileName, ATLANTIS_ATTRIBUTE, bm->ecotest, 1, groupingNode, binary_check, "masscheck");
	bm->which_fleet = (int) Util_XML_Read_Value(fileName, ATLANTIS_ATTRIBUTE, bm->ecotest, 1, groupingNode, integer_check, "which_fleet");
	bm->which_check = (int) Util_XML_Read_Value(fileName, ATLANTIS_ATTRIBUTE, bm->ecotest, 1, groupingNode, integer_check, "which_check");
	bm->move_check = (int) Util_XML_Read_Value(fileName, ATLANTIS_ATTRIBUTE, bm->ecotest, 1, groupingNode, integer_check, "move_check");
	bm->debug = (int) Util_XML_Read_Value(fileName, ATLANTIS_ATTRIBUTE, bm->ecotest, 1, groupingNode, integer_check, "debug");
	bm->debug_it = (int) Util_XML_Read_Value(fileName, ATLANTIS_ATTRIBUTE, bm->ecotest, 1, groupingNode, binary_check, "debug_it");
	bm->flagdetailedcatch = (int) Util_XML_Read_Value(fileName, ATLANTIS_ATTRIBUTE, bm->ecotest, 1, groupingNode, binary_check, "flagdetailedcatch");

	if (bm->debug) {
		warn("Debugging code - Activated\n");
		warn("Debugging code - Log file may grow VERY large if this is a long run\n");
		warn("Debugging code - Run times could be much slower than usual\n");
	}

	groupingNode = Util_XML_Get_Node(ATLANTIS_ATTRIBUTE_SUB_GROUP, inputDoc->children, "GroupOptions");
	if (groupingNode == NULL)
		quit("GroupOptions attribute group not found in input file %s.\n", fileName);

	bm->K_num_tot_sp = (int) Util_XML_Read_Value(fileName, ATLANTIS_ATTRIBUTE, bm->ecotest, 1, groupingNode, integer_check, "K_num_tot_sp");
	bm->K_num_stocks_per_sp = (int) Util_XML_Read_Value(fileName, ATLANTIS_ATTRIBUTE, bm->ecotest, 1, groupingNode, integer_check, "K_num_stocks_per_sp");
	bm->K_num_detritus = (int) Util_XML_Read_Value(fileName, ATLANTIS_ATTRIBUTE, bm->ecotest, 1, groupingNode, integer_check, "K_num_detritus");
	bm->K_num_bed_types = (int) Util_XML_Read_Value(fileName, ATLANTIS_ATTRIBUTE, bm->ecotest, 1, groupingNode, integer_check, "K_num_bed_types");
	bm->K_num_cover_types = (int) Util_XML_Read_Value(fileName, ATLANTIS_ATTRIBUTE, bm->ecotest, 1, groupingNode, integer_check, "K_num_cover_types");

	if (bm->K_num_bed_types != 3)
		quit("Model code currently assumes three types of seabed, rough (reef), flat and soft. Reset K_num_bed_types to 3, contact the Atlantis team if you need this changed.\n");

	bm->DIN_id = bm->K_num_tot_sp;

	groupingNode = Util_XML_Get_Node(ATLANTIS_ATTRIBUTE_SUB_GROUP, inputDoc->children, "FishingOptions");

	if (groupingNode == NULL)
		quit("FishingOptions attribute group not found in input file %s.\n", fileName);

	bm->K_num_catchqueue = (int) Util_XML_Read_Value(fileName, ATLANTIS_ATTRIBUTE, bm->ecotest, 1, groupingNode, integer_check, "K_num_catchqueue");
	bm->K_num_fisheries = (int) Util_XML_Read_Value(fileName, ATLANTIS_ATTRIBUTE, bm->ecotest, 1, groupingNode, integer_check, "K_num_fisheries");
	bm->K_num_ports = (int) Util_XML_Read_Value(fileName, ATLANTIS_ATTRIBUTE, bm->ecotest, 1, groupingNode, integer_check, "K_num_ports");
	bm->K_num_reg = (int) Util_XML_Read_Value(fileName, ATLANTIS_ATTRIBUTE, bm->ecotest, 1, groupingNode, integer_check, "K_num_reg");
	bm->K_num_markets = (int) Util_XML_Read_Value(fileName, ATLANTIS_ATTRIBUTE, bm->ecotest, 1, groupingNode, integer_check, "K_num_markets");
    bm->K_num_basket = (int) Util_XML_Read_Value(fileName, ATLANTIS_ATTRIBUTE, bm->ecotest, 1, groupingNode, integer_check, "K_num_basket");

	dt_scale = (int) (floor((86400 / bm->dt) + 0.5));
	bm->K_num_catchqueue *= dt_scale;
	if (bm->K_num_catchqueue < 1)
		bm->K_num_catchqueue = 1;


	groupingNode = Util_XML_Get_Node(ATLANTIS_ATTRIBUTE_SUB_GROUP, inputDoc->children, "ScaleGroupDensities");
	if (groupingNode == NULL)
		quit("ScaleGroupDensities attribute group not found in input file %s.\n", fileName);

	bm->flagscaleinit = (int) Util_XML_Read_Value(fileName, ATLANTIS_ATTRIBUTE, bm->ecotest, 1, groupingNode, binary_check, "flagscaleinit");

	/* In case of emergencies (such as emocc) have vertebrate density scalar
	 to bring values in cdf up to actual values intended */

	if (Util_XML_Read_Array_Double(ATLANTIS_ATTRIBUTE, fileName, "ScaleGroupDensities", groupingNode, no_checking, "init_scalar", &bm->init_scalar, bm->K_num_tot_sp) == FALSE) {
		quit("Error: Unable to read parameter 'ScaleGroupDensities/init_scalar' from input file %s\n", fileName);
	}

    
	/* Print a heading on stdout if verbose enough */
	if (verbose > 1) {
        printf("Read_Run_Parameters about to do keyprm_verbose\n");
		printf("# Run parameters\n");

		keyprm_verbose = max(0,verbose - 2);
	}
    
    printf("Read_Run_Parameters end\n");

	xmlFreeDoc(inputDoc);
	/* Shutdown libxml */
	xmlCleanupParser();
	return TRUE;
}

