/**
 *	\file
 *	\brief Functions to load the assessment parameters from the manage.xml input document.
 *	Created: 26-10-2009 Bec Gorton CSIRO 2009
 *
 *	Changes:
 *
 *	08-04-2010 Bec Gorton
 *	Removed the CON input array as we can just use the isPredator flag in the functional group input file.
 *
 *	14-05-2010 Bec Gorton
 *	Added code to read in the assessDataStream_id species params.
 *
 *	2-06-2010 Bec Gorton
 *	Merged trunk changes 1698 into bec_dev. Added code to read in the tClassicalAssessStart parameter.
 *
 *	5/11/2010 Bec Gorton
 *	Moved over to using input XML documents rather than the old text files.
 */

/*  Open library routines */
#include <time.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>
#include <ctype.h>
#include <sjwlib.h>
#include <netcdf.h>
#include <atsample.h>
//#include <atSS3LinkLib.h>

/** Prototypes **/
void readTierAssessmentManagementXML(MSEBoxModel *bm, char *fileName, xmlNodePtr rootnode);
void readKeyTierParamXML(MSEBoxModel *bm, char *fileName, xmlNodePtr rootnode);

/**
 * Read in the input detritus values.
 *
 *
 */
void readZoneXML(MSEBoxModel *bm, char *fileName, xmlNodePtr parent, char *paramName, int paramIndex) {
	double *values = 0;
	int b;
	char *nodeName;

	nodeName =  Util_Get_Node_Name(parent);
	if(Util_XML_Read_Array_Double(ATLANTIS_ATTRIBUTE, fileName, nodeName, parent, no_checking, paramName, &values, bm->nfzones) == FALSE){
		quit("ERROR: Cannot read parameter %s in input file %s\n", paramName, fileName);
	}

	for (b = 0; b < bm->nfzones; b++) {
		detrit_import[paramIndex][b] = values[b];
	}
	free(nodeName);
	free(values);

}

void readAssessFlagXML(MSEBoxModel *bm, char *fileName, xmlNodePtr rootnode) {
	int numMacro;
	int b, nsb;
	xmlNodePtr groupingNode;
	double *zsi;

	printf("Assessment - Reading in Assess_Run_Params\n");
	groupingNode = Util_XML_Get_Node(ATLANTIS_ATTRIBUTE_SUB_GROUP, rootnode, "Assess_Run_Params");
	if (groupingNode == NULL)
		quit("readAssessFlagXML: Assess_Run_Params attribute group not found in input file %s.\n", fileName);

	Util_XML_Get_Value_Double(fileName,ATLANTIS_ATTRIBUTE, bm->ecotest, 1, groupingNode, no_checking, "tassessstart", &bm->tassessstart );

	if (bm->tassessstart > bm->tstop)
		bm->do_assessing = 0;
	else
		bm->do_assessing = 1;

	/* Basic assessment increment */
	Util_XML_Get_Value_Double(fileName,ATLANTIS_ATTRIBUTE, bm->ecotest, 1, groupingNode, no_checking, "tassessinc", &bm->tassessinc);
	/* Year of the stomach record increment */
	Util_XML_Get_Value_Double(fileName,ATLANTIS_ATTRIBUTE, bm->ecotest, 1, groupingNode, no_checking, "teatassessinc", &bm->teatassessinc);
	/* Time that classical assessment model starts */
	Util_XML_Get_Value_Double(fileName,ATLANTIS_ATTRIBUTE, bm->ecotest, 1, groupingNode, no_checking, "tClassicalAssessStart", &bm->tClassicalAssessStart);

	if (bm->tClassicalAssessStart < (bm->tassessstart + 3650.0))
		warn("Time between start of surveys (day %e) and start of assessments (day %e) is less than 10 years (3650.0 days), this may cause problems\n",
				bm->tassessstart, bm->tClassicalAssessStart);

	/* Whether generating or loading sampling frequency */
	Util_XML_Get_Value_Integer(fileName, ATLANTIS_ATTRIBUTE, bm->ecotest, 1, groupingNode, binary_check, "flaggen", &bm->flaggen);
	/* Whether using patchy sampling */
	bm->flagpatchy = (int)Util_XML_Read_Value(fileName, ATLANTIS_ATTRIBUTE,  bm->ecotest, 1, groupingNode, binary_check, "flagpatchy");
	/* Whether using patchy sampling */
	bm->flagcont = (int)Util_XML_Read_Value(fileName, ATLANTIS_ATTRIBUTE,  bm->ecotest, 1, groupingNode, binary_check, "flagcont");
	/* Minimum patchy sampling record increment */
	bm->minfreq = Util_XML_Read_Value(fileName, ATLANTIS_ATTRIBUTE,  bm->ecotest, 1, groupingNode, integer_check, "minfreq");

	if (bm->minfreq > bm->tassessinc)
		bm->minfreq = bm->tassessinc;
    
	/* Maximum patchy sampling record increment */
	bm->maxfreq = Util_XML_Read_Value(fileName, ATLANTIS_ATTRIBUTE,  bm->ecotest, 1, groupingNode, integer_check, "maxfreq");
    
    if (bm->maxfreq < bm->tassessinc)
        bm->maxfreq = bm->tassessinc;
    
	/* Whether doing ecosystem assessments or not */
	bm->flagecosysassess = (int)Util_XML_Read_Value(fileName, ATLANTIS_ATTRIBUTE,  bm->ecotest, 1, groupingNode, binary_check, "flagecosysassess");
	/* Whether have observer data or not */
	bm->flagobsdata = (int)Util_XML_Read_Value(fileName, ATLANTIS_ATTRIBUTE,  bm->ecotest, 1, groupingNode, binary_check, "flagobsdata");
	/* Whether have observer data or not */
	bm->flagareacorrect = (int)Util_XML_Read_Value(fileName, ATLANTIS_ATTRIBUTE,  bm->ecotest, 1, groupingNode, binary_check, "flagareacorrect");
	/* Number of fishing zones */
	bm->nfzones = (int)Util_XML_Read_Value(fileName, ATLANTIS_ATTRIBUTE,  bm->ecotest, 1, groupingNode, integer_check, "nfzones");
	/* Number of sampling boxes */
	bm->nsbox_i = (int)Util_XML_Read_Value(fileName, ATLANTIS_ATTRIBUTE,  bm->ecotest, 1, groupingNode, integer_check, "nsbox_i");
	bm->nsbox_o = (int)Util_XML_Read_Value(fileName, ATLANTIS_ATTRIBUTE,  bm->ecotest, 1, groupingNode, integer_check, "nsbox_o");

	/* Now the number of sites has been read in we can allocate arrays */
	printf("Create global assessment box arrays\n");
	nsb = max(bm->nsbox_i, bm->nsbox_o);
	bm->nsboxes = (int *) i_alloc1d(nsb); //number of sampling locations

	//bm->nsboxes_other = (int *) i_alloc1d(bm->nsbox_o); //number of sampling locations
	//bm->nsboxes_intense = (int *) i_alloc1d(bm->nsbox_i); //number of sampling locations

	/* Critical depth split for inshore-offshore division */
	bm->cdz = (int)Util_XML_Read_Value(fileName, ATLANTIS_ATTRIBUTE,  bm->ecotest, 1, groupingNode, no_checking, "crit_depth");

	bm->nsbox = bm->nsbox_i;
	bm->last_snap = 0;

	/* Number of groups in the model */
	bm->K_num_tot_assess = (int)Util_XML_Read_Value(fileName, ATLANTIS_ATTRIBUTE,  bm->ecotest, 1, groupingNode, integer_check, "K_num_tot_assess");
	bm->K_num_live = (int)Util_XML_Read_Value(fileName, ATLANTIS_ATTRIBUTE,  bm->ecotest, 1, groupingNode, integer_check, "K_num_live");
	bm->K_num_eweinvert = (int)Util_XML_Read_Value(fileName, ATLANTIS_ATTRIBUTE,  bm->ecotest, 1, groupingNode, integer_check, "K_num_eweinvert");
	//bm->K_num_charis = Util_XML_Read_Value(fileName, ATLANTIS_ATTRIBUTE,  bm->ecotest, 1, groupingNode, integer_check, "K_num_charis");

//	if (bm->K_num_eweinvert != (bm->K_num_invert - 3))
//		quit(
//				"The code assumes K_num_eweinvert = K_num_invert - 3 (ignores bacteria and meiobenthos groups). Please reset the parameters to match this assumption\n");

	/* Number of vertebrates with explicit length-weight relationships */
	bm->K_num_sampleage = (int)Util_XML_Read_Value(fileName, ATLANTIS_ATTRIBUTE,  bm->ecotest, 1, groupingNode, integer_check, "K_num_sampleage");

	if (bm->K_num_sampleage != (bm->K_num_max_cohort * bm->K_num_max_genetypes))
		quit( "Currently code assumes (K_num_cohorts * K_num_genotypes) = K_num_sampleage. This will be changed in future cases, but for now set K_num_sampleage (currently %d) to K_num_cohorts * K_num_genotypes value (%d)\n",
				bm->K_num_sampleage, (bm->K_num_max_cohort * bm->K_num_max_genetypes));

	/* Number of invertebrate groups who's production and consumption is sampled */
	//bm->K_num_prod = Util_XML_Read_Value(fileName, ATLANTIS_ATTRIBUTE,  bm->ecotest, 1, groupingNode, integer_check, "K_num_prod");
	//bm->K_num_eat = Util_XML_Read_Value(fileName, ATLANTIS_ATTRIBUTE,  bm->ecotest, 1, groupingNode, integer_check, "K_num_eat");

	/* Number of fished groups in the model */
	//bm->K_num_invfished = Util_XML_Read_Value(fileName, ATLANTIS_ATTRIBUTE,  bm->ecotest, 1, groupingNode, integer_check, "K_num_invfished");

	/* Maximum recorded age of any vertebrate in the model */
	bm->K_max_agekey = (int)Util_XML_Read_Value(fileName, ATLANTIS_ATTRIBUTE,  bm->ecotest, 1, groupingNode, integer_check, "K_max_agekey");

	/* Number of vertebrate size bins */
	K_num_stomaches = (int)Util_XML_Read_Value(fileName, ATLANTIS_ATTRIBUTE,  bm->ecotest, 1, groupingNode, integer_check, "K_num_stomaches");
	bm->K_num_size = (int)Util_XML_Read_Value(fileName, ATLANTIS_ATTRIBUTE,  bm->ecotest, 1, groupingNode, integer_check, "K_num_size");
	bm->K_num_pelbin = (int)Util_XML_Read_Value(fileName, ATLANTIS_ATTRIBUTE,  bm->ecotest, 1, groupingNode, integer_check, "K_num_pelbin");
	bm->K_num_sedbin = (int)Util_XML_Read_Value(fileName, ATLANTIS_ATTRIBUTE,  bm->ecotest, 1, groupingNode, integer_check, "K_num_sedbin");
	bm->K_num_abcbin = (int)Util_XML_Read_Value(fileName, ATLANTIS_ATTRIBUTE,  bm->ecotest, 1, groupingNode, integer_check, "K_num_abcbin");
	bm->K_num_trophbin = (int)Util_XML_Read_Value(fileName, ATLANTIS_ATTRIBUTE,  bm->ecotest, 1, groupingNode, integer_check, "K_num_trophbin");

	/* Check the number of macro fauna */
	numMacro = bm->K_num_tot_sp - bm->K_num_detritus;
	if (bm->K_num_abcbin < numMacro) {
		warn("Number of abc bins < number of groups to be binned - reseting K_num_abcbin = numMacro = %d\n", numMacro);
		bm->K_num_abcbin = numMacro;
	}

	bm->maxTL = (int)Util_XML_Read_Value(fileName, ATLANTIS_ATTRIBUTE,  bm->ecotest, 1, groupingNode, integer_check, "maxTL");

	/* Read flag indicating whether sampled diets and age info are output */
	bm->samplediet = (int)Util_XML_Read_Value(fileName, ATLANTIS_ATTRIBUTE,  bm->ecotest, 1, groupingNode, binary_check, "samplediet");
	bm->ageout = (int)Util_XML_Read_Value(fileName, ATLANTIS_ATTRIBUTE,  bm->ecotest, 1, groupingNode, binary_check, "ageout");

	if(Util_XML_Read_Array_Double(ATLANTIS_ATTRIBUTE, fileName, "Assess_Run_Params", groupingNode, integer_check, "zones", &zsi, bm->nbox) == FALSE){
		quit("Error: Unable to read parameter 'Assess_Run_Params/zones' from input file %s\n", fileName);
	}
	for (b = 0; b < bm->nbox; b++) {
		if (!zsi[b])
			quit(
					"readAssess: The sampling (monitoring) Zones must be numbered beginning with 1 not 0, please re-number the zones array in the assess (sampling) prm file\n");
		bm->boxes[b].zone = (int) (zsi[b]);
	}
	free1d(zsi);

	if(Util_XML_Read_Array_Integer(ATLANTIS_ATTRIBUTE, fileName, groupingNode, integer_check, "sites_intense", &bm->nsboxes_intense, bm->nsbox_i) == FALSE){
		quit("Error: Unable to read parameter 'Assess_Run_Params/sites_intense' from input file %s\n", fileName);
	}
	if(Util_XML_Read_Array_Integer(ATLANTIS_ATTRIBUTE, fileName, groupingNode, integer_check, "sites_other", &bm->nsboxes_other, bm->nsbox_o) == FALSE){
		quit("Error: Unable to read parameter 'Assess_Run_Params/sites_other' from input file %s\n", fileName);
	}

	for (b = 0; b < bm->nsbox_i; b++) {
		bm->nsboxes[b] = bm->nsboxes_intense[b];
	}
	if(Util_XML_Read_Array_Integer(ATLANTIS_ATTRIBUTE, fileName, groupingNode, integer_check, "InOffshore", &InOffshorei, bm->nfzones) == FALSE){
		quit("Error: Unable to read parameter 'Assess_Run_Params/InOffshore' from input file %s\n", fileName);
	}

	phys_samplingsize = (int)Util_XML_Read_Value(fileName, ATLANTIS_ATTRIBUTE,  bm->ecotest, 1, groupingNode, integer_check, "phys_samplingsize");

}

/**
 * \brief Read in the error input values.
 */
void readErrorStructureXML(MSEBoxModel *bm, char *fileName, xmlNodePtr rootnode) {

	xmlNodePtr groupingNode;

	printf("Assessment - Reading in Error_Structure\n");

	groupingNode = Util_XML_Get_Node(ATLANTIS_ATTRIBUTE_SUB_GROUP, rootnode, "Error_Structure");
	if (groupingNode == NULL)
		quit("readErrorStructureXML: Error_Structure attribute group not found in input file %s.\n", fileName);

	/* Sampling model error distribution parameters */
	flagphys = (int)Util_XML_Read_Value(fileName, ATLANTIS_ATTRIBUTE,  bm->ecotest, 1, groupingNode, integer_check, "flagphys");
	flaginvpbiom = (int)Util_XML_Read_Value(fileName, ATLANTIS_ATTRIBUTE,  bm->ecotest, 1, groupingNode, integer_check, "flaginvpbiom");
	flagepibiom = (int)Util_XML_Read_Value(fileName, ATLANTIS_ATTRIBUTE,  bm->ecotest, 1, groupingNode, integer_check, "flagepibiom");
	flaginfbiom = (int)Util_XML_Read_Value(fileName, ATLANTIS_ATTRIBUTE,  bm->ecotest, 1, groupingNode, integer_check, "flaginfbiom");
	flagdetbiom = (int)Util_XML_Read_Value(fileName, ATLANTIS_ATTRIBUTE,  bm->ecotest, 1, groupingNode, integer_check, "flagdetbiom");
	flagfishbiom = (int)Util_XML_Read_Value(fileName, ATLANTIS_ATTRIBUTE,  bm->ecotest, 1, groupingNode, integer_check, "flagfishbiom");
	flagverts = (int)Util_XML_Read_Value(fileName, ATLANTIS_ATTRIBUTE,  bm->ecotest, 1, groupingNode, integer_check, "flagverts");
	flagprod = (int)Util_XML_Read_Value(fileName, ATLANTIS_ATTRIBUTE,  bm->ecotest, 1, groupingNode, integer_check, "flagprod");
	flageat = (int)Util_XML_Read_Value(fileName, ATLANTIS_ATTRIBUTE,  bm->ecotest, 1, groupingNode, integer_check, "flageat");
	flagcatch = (int)Util_XML_Read_Value(fileName, ATLANTIS_ATTRIBUTE,  bm->ecotest, 1, groupingNode, integer_check, "flagcatch");
	flageffort = (int)Util_XML_Read_Value(fileName, ATLANTIS_ATTRIBUTE,  bm->ecotest, 1, groupingNode, integer_check, "flageffort");
	flagdiscrd = (int)Util_XML_Read_Value(fileName, ATLANTIS_ATTRIBUTE,  bm->ecotest, 1, groupingNode, integer_check, "flagdiscrd");
	flagcount = (int)Util_XML_Read_Value(fileName, ATLANTIS_ATTRIBUTE,  bm->ecotest, 1, groupingNode, integer_check, "flagcount");
	flagcurve = (int)Util_XML_Read_Value(fileName, ATLANTIS_ATTRIBUTE,  bm->ecotest, 1, groupingNode, integer_check, "flagcurve");
	flagprms = (int)Util_XML_Read_Value(fileName, ATLANTIS_ATTRIBUTE,  bm->ecotest, 1, groupingNode, integer_check, "flagprms");
	flagage = (int)Util_XML_Read_Value(fileName, ATLANTIS_ATTRIBUTE,  bm->ecotest, 1, groupingNode, integer_check, "flagage");
}
/**
 * \brief Read in the bias input values.
 */
void readBiasedSampleXML(MSEBoxModel *bm, char *fileName, xmlNodePtr rootnode) {

	xmlNodePtr groupingNode;

	printf("Assessment - Reading in Biased_Sample\n");

	groupingNode = Util_XML_Get_Node(ATLANTIS_ATTRIBUTE_SUB_GROUP, rootnode, "Biased_Sample");
	if (groupingNode == NULL)
		quit("readBiasedSampleXML: Biased_Sample attribute group not found in input file %s.\n", fileName);

	k_avgsalt = Util_XML_Read_Value(fileName, ATLANTIS_ATTRIBUTE,  bm->ecotest, 1, groupingNode, no_checking, "k_avgsalt");
	k_avgphys = Util_XML_Read_Value(fileName, ATLANTIS_ATTRIBUTE,  bm->ecotest, 1, groupingNode, no_checking, "k_avgphys");
	k_avgnuts = Util_XML_Read_Value(fileName, ATLANTIS_ATTRIBUTE,  bm->ecotest, 1, groupingNode, no_checking, "k_avgnuts");
	k_avgprocs = Util_XML_Read_Value(fileName, ATLANTIS_ATTRIBUTE,  bm->ecotest, 1, groupingNode, no_checking, "k_avgprocs");
	k_avgphytl = Util_XML_Read_Value(fileName, ATLANTIS_ATTRIBUTE,  bm->ecotest, 1, groupingNode, no_checking, "k_avgphytl");
	k_avgphyts = Util_XML_Read_Value(fileName, ATLANTIS_ATTRIBUTE,  bm->ecotest, 1, groupingNode, no_checking, "k_avgphyts");
	k_avgzoops = Util_XML_Read_Value(fileName, ATLANTIS_ATTRIBUTE,  bm->ecotest, 1, groupingNode, no_checking, "k_avgzoops");
	k_avgzoopl = Util_XML_Read_Value(fileName, ATLANTIS_ATTRIBUTE,  bm->ecotest, 1, groupingNode, no_checking, "k_avgzoopl");
	k_avgcep = Util_XML_Read_Value(fileName, ATLANTIS_ATTRIBUTE,  bm->ecotest, 1, groupingNode, no_checking, "k_avgcep");
	k_avgpbact = Util_XML_Read_Value(fileName, ATLANTIS_ATTRIBUTE,  bm->ecotest, 1, groupingNode, no_checking, "k_avgpbact");
	k_avgsbact = Util_XML_Read_Value(fileName, ATLANTIS_ATTRIBUTE,  bm->ecotest, 1, groupingNode, no_checking, "k_avgsbact");
	k_avginfs = Util_XML_Read_Value(fileName, ATLANTIS_ATTRIBUTE,  bm->ecotest, 1, groupingNode, no_checking, "k_avginfs");
	k_avginfl = Util_XML_Read_Value(fileName, ATLANTIS_ATTRIBUTE,  bm->ecotest, 1, groupingNode, no_checking, "k_avginfl");
	k_avgepif1 = Util_XML_Read_Value(fileName, ATLANTIS_ATTRIBUTE,  bm->ecotest, 1, groupingNode, no_checking, "k_avgepif1");
	k_avgepif2 = Util_XML_Read_Value(fileName, ATLANTIS_ATTRIBUTE,  bm->ecotest, 1, groupingNode, no_checking, "k_avgepif2");
	k_avgppben = Util_XML_Read_Value(fileName, ATLANTIS_ATTRIBUTE,  bm->ecotest, 1, groupingNode, no_checking, "k_avgppben");
	k_avgrdet = Util_XML_Read_Value(fileName, ATLANTIS_ATTRIBUTE,  bm->ecotest, 1, groupingNode, no_checking, "k_avgrdet");
	k_avgldet = Util_XML_Read_Value(fileName, ATLANTIS_ATTRIBUTE,  bm->ecotest, 1, groupingNode, no_checking, "k_avgldet");
	k_avgfish = Util_XML_Read_Value(fileName, ATLANTIS_ATTRIBUTE,  bm->ecotest, 1, groupingNode, no_checking, "k_avgfish");
	k_avgpelpp = Util_XML_Read_Value(fileName, ATLANTIS_ATTRIBUTE,  bm->ecotest, 1, groupingNode, no_checking, "k_avgpelpp");
	k_avgzoopp = Util_XML_Read_Value(fileName, ATLANTIS_ATTRIBUTE,  bm->ecotest, 1, groupingNode, no_checking, "k_avgzoopp");
	k_avgcepp = Util_XML_Read_Value(fileName, ATLANTIS_ATTRIBUTE,  bm->ecotest, 1, groupingNode, no_checking, "k_avgcepp");
	k_avgpbactp = Util_XML_Read_Value(fileName, ATLANTIS_ATTRIBUTE,  bm->ecotest, 1, groupingNode, no_checking, "k_avgpbactp");
	k_avgsbactp = Util_XML_Read_Value(fileName, ATLANTIS_ATTRIBUTE,  bm->ecotest, 1, groupingNode, no_checking, "k_avgsbactp");
	k_avginfsp = Util_XML_Read_Value(fileName, ATLANTIS_ATTRIBUTE,  bm->ecotest, 1, groupingNode, no_checking, "k_avginfsp");
	k_avginflp = Util_XML_Read_Value(fileName, ATLANTIS_ATTRIBUTE,  bm->ecotest, 1, groupingNode, no_checking, "k_avginflp");
	k_avgepi1p = Util_XML_Read_Value(fileName, ATLANTIS_ATTRIBUTE,  bm->ecotest, 1, groupingNode, no_checking, "k_avgepi1p");
	k_avgepi2p = Util_XML_Read_Value(fileName, ATLANTIS_ATTRIBUTE,  bm->ecotest, 1, groupingNode, no_checking, "k_avgepi2p");
	k_avgbenpp = Util_XML_Read_Value(fileName, ATLANTIS_ATTRIBUTE,  bm->ecotest, 1, groupingNode, no_checking, "k_avgbenpp");
	k_avgzoopeat = Util_XML_Read_Value(fileName, ATLANTIS_ATTRIBUTE,  bm->ecotest, 1, groupingNode, no_checking, "k_avgzoopeat");
	k_avgcepeat = Util_XML_Read_Value(fileName, ATLANTIS_ATTRIBUTE,  bm->ecotest, 1, groupingNode, no_checking, "k_avgcepeat");
	k_avgpbacteat = Util_XML_Read_Value(fileName, ATLANTIS_ATTRIBUTE,  bm->ecotest, 1, groupingNode, no_checking, "k_avgpbacteat");
	k_avgsbacteat = Util_XML_Read_Value(fileName, ATLANTIS_ATTRIBUTE,  bm->ecotest, 1, groupingNode, no_checking, "k_avgsbacteat");
	k_avginfseat = Util_XML_Read_Value(fileName, ATLANTIS_ATTRIBUTE,  bm->ecotest, 1, groupingNode, no_checking, "k_avginfseat");
	k_avginfleat = Util_XML_Read_Value(fileName, ATLANTIS_ATTRIBUTE,  bm->ecotest, 1, groupingNode, no_checking, "k_avginfleat");
	k_avgepi1eat = Util_XML_Read_Value(fileName, ATLANTIS_ATTRIBUTE,  bm->ecotest, 1, groupingNode, no_checking, "k_avgepi1eat");
	k_avgepi2eat = Util_XML_Read_Value(fileName, ATLANTIS_ATTRIBUTE,  bm->ecotest, 1, groupingNode, no_checking, "k_avgepi2eat");
	k_avgweight = Util_XML_Read_Value(fileName, ATLANTIS_ATTRIBUTE,  bm->ecotest, 1, groupingNode, no_checking, "k_avgweight");
	k_avgprodn = Util_XML_Read_Value(fileName, ATLANTIS_ATTRIBUTE,  bm->ecotest, 1, groupingNode, no_checking, "k_avgprodn");
	k_avgeat = Util_XML_Read_Value(fileName, ATLANTIS_ATTRIBUTE,  bm->ecotest, 1, groupingNode, no_checking, "k_avgeat");
	k_avgdiscrd = Util_XML_Read_Value(fileName, ATLANTIS_ATTRIBUTE,  bm->ecotest, 1, groupingNode, no_checking, "k_avgdiscrd");
	k_avgcatch = Util_XML_Read_Value(fileName, ATLANTIS_ATTRIBUTE,  bm->ecotest, 1, groupingNode, no_checking, "k_avgcatch");
	k_avgeffort = Util_XML_Read_Value(fileName, ATLANTIS_ATTRIBUTE,  bm->ecotest, 1, groupingNode, no_checking, "k_avgeffort");
	k_avgtdiscrd = Util_XML_Read_Value(fileName, ATLANTIS_ATTRIBUTE,  bm->ecotest, 1, groupingNode, no_checking, "k_avgtdiscrd");
	k_avgcount = Util_XML_Read_Value(fileName, ATLANTIS_ATTRIBUTE,  bm->ecotest, 1, groupingNode, no_checking, "k_avgcount");
	k_avgcobs = Util_XML_Read_Value(fileName, ATLANTIS_ATTRIBUTE,  bm->ecotest, 1, groupingNode, no_checking, "k_avgcobs");
	k_avgcurve = Util_XML_Read_Value(fileName, ATLANTIS_ATTRIBUTE,  bm->ecotest, 1, groupingNode, no_checking, "k_avgcurve");
	k_avgprms = Util_XML_Read_Value(fileName, ATLANTIS_ATTRIBUTE,  bm->ecotest, 1, groupingNode, no_checking, "k_avgprms");
	k_avgage = Util_XML_Read_Value(fileName, ATLANTIS_ATTRIBUTE,  bm->ecotest, 1, groupingNode, no_checking, "k_avgage");
}

/**
 * \brief Read in the error variance variables from the input file.
 */
void readErrorVarianceXML(MSEBoxModel *bm, char *fileName, xmlNodePtr rootnode) {

	xmlNodePtr groupingNode;

	printf("Assessment - Reading in Error_Variance\n");

	groupingNode = Util_XML_Get_Node(ATLANTIS_ATTRIBUTE_SUB_GROUP, rootnode, "Error_Variance");
	if (groupingNode == NULL)
		quit("readErrorVarianceXML: Error_Variance attribute group not found in input file %s.\n", fileName);

	k_varsalt = Util_XML_Read_Value(fileName, ATLANTIS_ATTRIBUTE,  bm->ecotest, 1, groupingNode, no_checking, "k_varsalt");
	k_varphys = Util_XML_Read_Value(fileName, ATLANTIS_ATTRIBUTE,  bm->ecotest, 1, groupingNode, no_checking, "k_varphys");
	k_varnuts = Util_XML_Read_Value(fileName, ATLANTIS_ATTRIBUTE,  bm->ecotest, 1, groupingNode, no_checking, "k_varnuts");
	k_varprocs = Util_XML_Read_Value(fileName, ATLANTIS_ATTRIBUTE,  bm->ecotest, 1, groupingNode, no_checking, "k_varprocs");
	k_varphytl = Util_XML_Read_Value(fileName, ATLANTIS_ATTRIBUTE,  bm->ecotest, 1, groupingNode, no_checking, "k_varphytl");
	k_varphyts = Util_XML_Read_Value(fileName, ATLANTIS_ATTRIBUTE,  bm->ecotest, 1, groupingNode, no_checking, "k_varphyts");
	k_varzoops = Util_XML_Read_Value(fileName, ATLANTIS_ATTRIBUTE,  bm->ecotest, 1, groupingNode, no_checking, "k_varzoops");
	k_varzoopl = Util_XML_Read_Value(fileName, ATLANTIS_ATTRIBUTE,  bm->ecotest, 1, groupingNode, no_checking, "k_varzoopl");
	k_varcep = Util_XML_Read_Value(fileName, ATLANTIS_ATTRIBUTE,  bm->ecotest, 1, groupingNode, no_checking, "k_varcep");
	k_varpbact = Util_XML_Read_Value(fileName, ATLANTIS_ATTRIBUTE,  bm->ecotest, 1, groupingNode, no_checking, "k_varpbact");
	k_varsbact = Util_XML_Read_Value(fileName, ATLANTIS_ATTRIBUTE,  bm->ecotest, 1, groupingNode, no_checking, "k_varsbact");
	k_varinfs = Util_XML_Read_Value(fileName, ATLANTIS_ATTRIBUTE,  bm->ecotest, 1, groupingNode, no_checking, "k_varinfs");
	k_varinfl = Util_XML_Read_Value(fileName, ATLANTIS_ATTRIBUTE,  bm->ecotest, 1, groupingNode, no_checking, "k_varinfl");
	k_varepif1 = Util_XML_Read_Value(fileName, ATLANTIS_ATTRIBUTE,  bm->ecotest, 1, groupingNode, no_checking, "k_varepif1");
	k_varepif2 = Util_XML_Read_Value(fileName, ATLANTIS_ATTRIBUTE,  bm->ecotest, 1, groupingNode, no_checking, "k_varepif2");
	k_varppben = Util_XML_Read_Value(fileName, ATLANTIS_ATTRIBUTE,  bm->ecotest, 1, groupingNode, no_checking, "k_varppben");
	k_varrdet = Util_XML_Read_Value(fileName, ATLANTIS_ATTRIBUTE,  bm->ecotest, 1, groupingNode, no_checking, "k_varrdet");
	k_varldet = Util_XML_Read_Value(fileName, ATLANTIS_ATTRIBUTE,  bm->ecotest, 1, groupingNode, no_checking, "k_varldet");
	k_varfish = Util_XML_Read_Value(fileName, ATLANTIS_ATTRIBUTE,  bm->ecotest, 1, groupingNode, no_checking, "k_varfish");
	k_varpelpp = Util_XML_Read_Value(fileName, ATLANTIS_ATTRIBUTE,  bm->ecotest, 1, groupingNode, no_checking, "k_varpelpp");
	k_varzoopp = Util_XML_Read_Value(fileName, ATLANTIS_ATTRIBUTE,  bm->ecotest, 1, groupingNode, no_checking, "k_varzoopp");
	k_varcepp = Util_XML_Read_Value(fileName, ATLANTIS_ATTRIBUTE,  bm->ecotest, 1, groupingNode, no_checking, "k_varcepp");
	k_varpbactp = Util_XML_Read_Value(fileName, ATLANTIS_ATTRIBUTE,  bm->ecotest, 1, groupingNode, no_checking, "k_varpbactp");
	k_varsbactp = Util_XML_Read_Value(fileName, ATLANTIS_ATTRIBUTE,  bm->ecotest, 1, groupingNode, no_checking, "k_varsbactp");
	k_varinfsp = Util_XML_Read_Value(fileName, ATLANTIS_ATTRIBUTE,  bm->ecotest, 1, groupingNode, no_checking, "k_varinfsp");
	k_varinflp = Util_XML_Read_Value(fileName, ATLANTIS_ATTRIBUTE,  bm->ecotest, 1, groupingNode, no_checking, "k_varinflp");
	k_varepi1p = Util_XML_Read_Value(fileName, ATLANTIS_ATTRIBUTE,  bm->ecotest, 1, groupingNode, no_checking, "k_varepi1p");
	k_varepi2p = Util_XML_Read_Value(fileName, ATLANTIS_ATTRIBUTE,  bm->ecotest, 1, groupingNode, no_checking, "k_varepi2p");
	k_varbenpp = Util_XML_Read_Value(fileName, ATLANTIS_ATTRIBUTE,  bm->ecotest, 1, groupingNode, no_checking, "k_varbenpp");
	k_varzoopeat = Util_XML_Read_Value(fileName, ATLANTIS_ATTRIBUTE,  bm->ecotest, 1, groupingNode, no_checking, "k_varzoopeat");
	k_varcepeat = Util_XML_Read_Value(fileName, ATLANTIS_ATTRIBUTE,  bm->ecotest, 1, groupingNode, no_checking, "k_varcepeat");
	k_varpbacteat = Util_XML_Read_Value(fileName, ATLANTIS_ATTRIBUTE,  bm->ecotest, 1, groupingNode, no_checking, "k_varpbacteat");
	k_varsbacteat = Util_XML_Read_Value(fileName, ATLANTIS_ATTRIBUTE,  bm->ecotest, 1, groupingNode, no_checking, "k_varsbacteat");
	k_varinfseat = Util_XML_Read_Value(fileName, ATLANTIS_ATTRIBUTE,  bm->ecotest, 1, groupingNode, no_checking, "k_varinfseat");
	k_varinfleat = Util_XML_Read_Value(fileName, ATLANTIS_ATTRIBUTE,  bm->ecotest, 1, groupingNode, no_checking, "k_varinfleat");
	k_varepi1eat = Util_XML_Read_Value(fileName, ATLANTIS_ATTRIBUTE,  bm->ecotest, 1, groupingNode, no_checking, "k_varepi1eat");
	k_varepi2eat = Util_XML_Read_Value(fileName, ATLANTIS_ATTRIBUTE,  bm->ecotest, 1, groupingNode, no_checking, "k_varepi2eat");
	k_varweight = Util_XML_Read_Value(fileName, ATLANTIS_ATTRIBUTE,  bm->ecotest, 1, groupingNode, no_checking, "k_varweight");
	k_varprodn = Util_XML_Read_Value(fileName, ATLANTIS_ATTRIBUTE,  bm->ecotest, 1, groupingNode, no_checking, "k_varprodn");
	k_vareat = Util_XML_Read_Value(fileName, ATLANTIS_ATTRIBUTE,  bm->ecotest, 1, groupingNode, no_checking, "k_vareat");
	k_vardiscrd = Util_XML_Read_Value(fileName, ATLANTIS_ATTRIBUTE,  bm->ecotest, 1, groupingNode, no_checking, "k_vardiscrd");
	k_varcatch = Util_XML_Read_Value(fileName, ATLANTIS_ATTRIBUTE,  bm->ecotest, 1, groupingNode, no_checking, "k_varcatch");
	k_vareffort = Util_XML_Read_Value(fileName, ATLANTIS_ATTRIBUTE,  bm->ecotest, 1, groupingNode, no_checking, "k_vareffort");
	k_vartdiscrd = Util_XML_Read_Value(fileName, ATLANTIS_ATTRIBUTE,  bm->ecotest, 1, groupingNode, no_checking, "k_vartdiscrd");
	k_varcount = Util_XML_Read_Value(fileName, ATLANTIS_ATTRIBUTE,  bm->ecotest, 1, groupingNode, no_checking, "k_varcount");
	k_varcobs = Util_XML_Read_Value(fileName, ATLANTIS_ATTRIBUTE,  bm->ecotest, 1, groupingNode, no_checking, "k_varcobs");
	k_varcurve = Util_XML_Read_Value(fileName, ATLANTIS_ATTRIBUTE,  bm->ecotest, 1, groupingNode, no_checking, "k_varcurve");
	k_varprms = Util_XML_Read_Value(fileName, ATLANTIS_ATTRIBUTE,  bm->ecotest, 1, groupingNode, no_checking, "k_varprms");
	k_varage = Util_XML_Read_Value(fileName, ATLANTIS_ATTRIBUTE,  bm->ecotest, 1, groupingNode, no_checking, "k_varage");
}

/**
 * \brief Read in the size based parameters.
 */
void readVertParamsXML(MSEBoxModel *bm, char *fileName, xmlNodePtr rootnode) {

	int needobs, b;
	xmlNodePtr groupingNode;

	printf("Assessment - Reading in Size_Parameters\n");

	groupingNode = Util_XML_Get_Node(ATLANTIS_ATTRIBUTE_SUB_GROUP, rootnode, "Size_Parameters");
	if (groupingNode == NULL)
		quit("readVertParamsXML: Size_Parameters attribute group not found in input file %s.\n", fileName);

	Util_XML_Read_Species_Param(bm, fileName, groupingNode, samplesize_id);

	needobs = 0;
	for (b = 0; b < bm->K_num_tot_sp; b++) {
		if (FunctGroupArray[b].isVertebrate == TRUE) {
			if (FunctGroupArray[b].speciesParams[samplesize_id] > 0)
				needobs = 1;
		}
	}
	if (needobs && (!bm->flagobsdata))
		warn("Have flagobsdata = 0, but also have non zero vertebrate sample sizes, this means you will have fisheries independent data but no catchnum data\n");

	Util_XML_Read_Species_Param(bm, fileName, groupingNode, allometic_li_a_id);
	Util_XML_Read_Species_Param(bm, fileName, groupingNode, allometic_li_b_id);
	Util_XML_Read_Species_Param(bm, fileName, groupingNode, allometic_bin_size_id);
	Util_XML_Read_Species_Param(bm, fileName, groupingNode, allometic_bin_start_id);

	Util_XML_Read_Species_Param(bm, fileName, groupingNode, max_length_id);
}

/**
 * \brief Read in the fishing selectivity values.
 */
void readAssessFishingSelectivityXML(MSEBoxModel *bm, char *fileName, xmlNodePtr rootnode) {

	xmlNodePtr groupingNode;

	printf("Assessment - Reading in Fishing_Selectivity\n");

	groupingNode = Util_XML_Get_Node(ATLANTIS_ATTRIBUTE_SUB_GROUP, rootnode, "Fishing_Selectivity");
	if (groupingNode == NULL)
		quit("Fishing_Selectivity: Fishing_Selectivity attribute group not found in input file %s.\n", fileName);

	Util_XML_Read_Fishery_Param(bm, fileName, groupingNode, q_lsm_id);
	Util_XML_Read_Fishery_Param(bm, fileName, groupingNode, q_sigma_id);
	Util_XML_Read_Fishery_Param(bm, fileName, groupingNode, assess_selcurve_id);

	Util_XML_Read_Vert_Fishery_Param(bm, fileName, groupingNode, avail_id);
}

/**
 * \brief Read in the detritus input parameters.
 */
void readDetritusXML(MSEBoxModel *bm, char *fileName, xmlNodePtr rootnode) {

	xmlNodePtr groupingNode;
	xmlNodePtr attributeNode, groupNode;
	int guild, index;

	printf("Assessment - Reading in Detritus_Input\n");

	groupingNode = Util_XML_Get_Node(ATLANTIS_ATTRIBUTE_SUB_GROUP, rootnode, "Detritus_Input");
	if (groupingNode == NULL)
		quit("readDetritusXML: Detritus_Input attribute group not found in input file %s.\n", fileName);

	readZoneXML(bm, fileName, groupingNode, "input_DLwc", 0);
	readZoneXML(bm, fileName, groupingNode, "input_DRwc", 1);
	readZoneXML(bm, fileName, groupingNode, "input_DLsed", 2);
	readZoneXML(bm, fileName, groupingNode, "input_DRsed", 3);

	/* Read in the discard fate values for detritus */
	attributeNode = Util_XML_Get_Node(ATLANTIS_ATTRIBUTE, groupingNode, "Discard_Fate");
	if (attributeNode == NULL)
		quit("Detritus_Input/Discard_Fate attribute group not found in input file %s.\n", fileName);

	index = 0;
	for (guild = 0; guild < bm->K_num_tot_sp; guild++) {
		if (FunctGroupArray[guild].isDetritus == TRUE) {
			groupNode = Util_XML_Get_Node(ATLANTIS_GROUP_ATTRIBUTE, attributeNode, FunctGroupArray[guild].groupCode);
			if (groupNode == NULL)
				quit("Detritus_Input/Discard_Fate/%s attribute group not found.\n", FunctGroupArray[guild].groupCode);

			DiscardFate[index][WC] = Util_XML_Read_Value(fileName, ATLANTIS_ATTRIBUTE,  bm->ecotest, 1, groupNode, no_checking, "WC");
			DiscardFate[index][SED] = Util_XML_Read_Value(fileName, ATLANTIS_ATTRIBUTE,  bm->ecotest, 1, groupNode, no_checking, "SED");
			index++;
		}
	}

	Util_XML_Read_Species_Param(bm, fileName, groupingNode, r_max_id);
	Util_XML_Read_Species_Param(bm, fileName, groupingNode, avg_inv_size_id);
}

void readAssessmentParamsXML(MSEBoxModel *bm, char *fileName, xmlNodePtr rootnode) {

	int sp, dataid, check_assess = 0;
	xmlNodePtr groupingNode;

	printf("Assessment - Reading in Assessment_Parameters\n");

	groupingNode = Util_XML_Get_Node(ATLANTIS_ATTRIBUTE_SUB_GROUP, rootnode, "Assessment_Parameters");
	if (groupingNode == NULL)
		quit("readAssessmentParamsXML: Assessment_Parameters attribute group not found in input file %s.\n", fileName);

	/* Assessment parameters */
	bm->Assess_Tol = Util_XML_Read_Value(fileName, ATLANTIS_ATTRIBUTE,  bm->ecotest, 1, groupingNode, no_checking, "Assess_Tol");
	bm->Assess_Btol = Util_XML_Read_Value(fileName, ATLANTIS_ATTRIBUTE,  bm->ecotest, 1, groupingNode, no_checking, "Assess_Btol");
	bm->Assess_Max_Int = (int)Util_XML_Read_Value(fileName, ATLANTIS_ATTRIBUTE,  bm->ecotest, 1, groupingNode, integer_check, "Assess_Max_Int");

	/* Length of assessment model run out */
	bm->assessnyr = (int)Util_XML_Read_Value(fileName, ATLANTIS_ATTRIBUTE,  bm->ecotest, 1, groupingNode, integer_check, "AssessNyr");

	Util_XML_Read_Species_Param(bm, fileName, groupingNode, assess_flag_id);

	/* Now check these values and change if required */
	bm->do_TACassessing = 0;
	for (sp = 0; sp < bm->K_num_tot_sp; sp++) {
		if (FunctGroupArray[sp].isFished == TRUE) {

			check_assess += (int)(FunctGroupArray[sp].speciesParams[assess_flag_id]);
			switch (FunctGroupArray[sp].groupAgeType) {
			case AGE_STRUCTURED_BIOMASS: /* intentional fall though */
			case BIOMASS:
				/* So far only doing estimates for vertebrate species */
				if (FunctGroupArray[sp].speciesParams[assess_flag_id] == TRUE)
					warn("%s originally set to being assessed in assess (sampling) prm, but no biomass pool groups may be assessed as yet\n",
							FunctGroupArray[sp].groupCode);

				FunctGroupArray[sp].speciesParams[assess_flag_id] = 0.0;
				break;

			case AGE_STRUCTURED:
				/* Do nothing */
				if (FunctGroupArray[sp].speciesParams[assess_flag_id] > 0)
					bm->do_TACassessing = 1;
				break;

			}
		}
	}
	if(!check_assess && bm->pseudo_assess){
		bm->pseudo_assess = 0;
		warn("Resetting pseudo_assess as nothing being assessed at all\n");
	}

	Util_XML_Read_Species_Param(bm, fileName, groupingNode, assess_bootstrap_id);
	Util_XML_Read_Species_Param(bm, fileName, groupingNode, assess_nat_mort_id);
	Util_XML_Read_Species_Param(bm, fileName, groupingNode, flag_prod_model_id);
	Util_XML_Read_Species_Param(bm, fileName, groupingNode, top_pcnt_id);
	Util_XML_Read_Species_Param(bm, fileName, groupingNode, bot_pcnt_id);
	Util_XML_Read_Species_Param(bm, fileName, groupingNode, assessDataStream_id);

	/* Check data stream fidelity */
	for (sp = 0; sp < bm->K_num_tot_sp; sp++) {
		if ((FunctGroupArray[sp].groupType != BIRD && FunctGroupArray[sp].groupType != MAMMAL)) {
			dataid = (int) (FunctGroupArray[sp].speciesParams[assessDataStream_id]);
			if (dataid == survey_id)
				warn("Data stream for assessment of %s set to survey - how get survey CPUE estimate? Better to reset and use commercial for now\n",
						FunctGroupArray[sp].groupCode);
		}
	}
    
    Util_XML_Read_Vert_Fishery_Param(bm, fileName, groupingNode, assess_nf_id);
}

void readRAssessFlagXML(MSEBoxModel *bm, char *fileName, xmlNodePtr rootnode) {
    int nf;
    xmlNodePtr groupingNode, childGroupingNode;
    char nameString[BMSLEN];

    printf("Assessment - Reading in RAssess_Params\n");
    groupingNode = Util_XML_Get_Node(ATLANTIS_ATTRIBUTE_SUB_GROUP, rootnode, "RAssess_Params");
    if (groupingNode == NULL)
        quit("readRAssessFlagXML: RAssess_Params attribute group not found in input file %s.\n", fileName);

    bm->RAssessSpringSurveyDay = -1;
    bm->RAssessAutumnSurveyDay = -1;
    bm->UsingRAssess = (int)(Util_XML_Read_Value(fileName, ATLANTIS_ATTRIBUTE,  bm->ecotest, 1, groupingNode, binary_check, "UsingRAssess"));
    
    if (bm->UsingRAssess) {
        bm->RAssessSpringSurveyDay = (int)(Util_XML_Read_Value(fileName, ATLANTIS_ATTRIBUTE,  bm->ecotest, 1, groupingNode, integer_check, "RAssessSpringSurveyDay"));
        bm->RAssessAutumnSurveyDay = (int)(Util_XML_Read_Value(fileName, ATLANTIS_ATTRIBUTE,  bm->ecotest, 1, groupingNode, integer_check, "RAssessAutumnSurveyDay"));
        bm->RAssessFisheryID = (int)Util_XML_Read_Value(fileName, ATLANTIS_ATTRIBUTE,  bm->ecotest, 1, groupingNode, integer_check, "RAssessFisheryID");
        bm->RAssessFixedM = (int)Util_XML_Read_Value(fileName, ATLANTIS_ATTRIBUTE,  bm->ecotest, 1, groupingNode, binary_check, "RAssessFixedM");
        bm->RAssessRefYear = (int)Util_XML_Read_Value(fileName, ATLANTIS_ATTRIBUTE,  bm->ecotest, 1, groupingNode, integer_check, "RAssessRefYear");
        
        bm->RAssessRuseScript = (int)Util_XML_Read_Value(fileName, ATLANTIS_ATTRIBUTE,  bm->ecotest, 1, groupingNode, integer_check, "RAssessRuseScript");
        
        Util_XML_Get_Value_String(fileName, ATLANTIS_ATTRIBUTE, 1, groupingNode, "RassessColDelimiter", bm->RassessColDelimiter);
        Util_XML_Get_Value_String(fileName, ATLANTIS_ATTRIBUTE, 1, groupingNode, "RAssessRinName", bm->RAssessRinName);
        Util_XML_Get_Value_String(fileName, ATLANTIS_ATTRIBUTE, 1, groupingNode, "RAssessRoutName", bm->RAssessRoutName);
        
        bm->NumRAssessScriptNames = (int)Util_XML_Read_Value(fileName, ATLANTIS_ATTRIBUTE,  bm->ecotest, 1, groupingNode, integer_check, "NumRAssessScriptNames");
        bm->RAssessRscriptName = c_alloc2d(BMSLEN, bm->NumRAssessScriptNames);
        for (nf = 0; nf < bm->NumRAssessScriptNames; nf ++) {
            snprintf(nameString, sizeof(nameString), "RAssessRscriptName%d", nf);
            Util_XML_Get_Value_String(fileName, ATLANTIS_ATTRIBUTE, 1, groupingNode, nameString, bm->RAssessRscriptName[nf]);
        }
        
        childGroupingNode = Util_XML_Get_Node(ATLANTIS_ATTRIBUTE_SUB_GROUP, groupingNode, "speciesRAssess");
        if (childGroupingNode == NULL)
            quit("RAssessFlagXML: speciesRAssess attribute group not found in input file %s.\n", fileName);
        
        Util_XML_Read_Species_Param(bm, fileName, childGroupingNode, whichRAssess_id);
    }
}


/**
 *	This routine reads all parameters of the management models
 *
 *  Input:  char *filename: The name of the management xml input file.
 *
 */
int readModelAssessmentParameters(MSEBoxModel *bm, char *filename) {
    int sp;
	xmlDocPtr inputDoc;
	inputDoc = xmlReadFileDestFolder(bm->destFolder, filename, NULL, 0);

	printf("Assessment parameter file loading\n");
	printf("0                                 100\n");
	printf("***");

	/* read in the flags */
	readAssessFlagXML(bm, filename, inputDoc->children);

	/* detritus inputs and flows */
	printf("Create assessment detritus flow vectors\n");

	detrit_import = (double **) alloc2d(bm->nfzones, bm->K_num_detritus * 2);
	DiscardFate = (double **) alloc2d(2, bm->K_num_detritus);

	/* Read the rest of the model run parameters */
	readErrorStructureXML(bm, filename, inputDoc->children);
	readBiasedSampleXML(bm, filename,inputDoc->children);
	readErrorVarianceXML(bm, filename,inputDoc->children);
	readVertParamsXML(bm, filename,inputDoc->children);
	readAssessFishingSelectivityXML(bm, filename,inputDoc->children);
	readDetritusXML(bm, filename,inputDoc->children);
	readAssessmentParamsXML(bm, filename, inputDoc->children);
    readRAssessFlagXML(bm, filename, inputDoc->children);

	if(bm->useRBCTiers){
        /* Preallocate the arrays and load parameters */
        bm->RBCestimation.RBCspeciesArray = (RBCarrays *) malloc(sizeof(RBCarrays) * (unsigned long int)bm->K_num_tot_sp);
        bm->RBCestimation.RBCspeciesParam = Util_Alloc_Init_2D_Double(num_rbc_species_params_id, bm->K_num_tot_sp, 0.0);
        
        bm->CloseKinEst = (CloseKinstructure *) malloc(sizeof(CloseKinstructure) * (size_t)bm->K_num_tot_sp);

        readKeyTierParamXML(bm, filename, inputDoc->children);
        PreAllocate_Index_Setting(bm);
        Tier_Assessment_Allocate(bm);
		readTierAssessmentManagementXML(bm, filename, inputDoc->children);
        
        /* Finalise the setup of the parameters */
        Tier_Assessment_Setup(bm);
        Tier_Assessment_PostLoad_Allocate(bm);  // This creates arrays used in the assessments itself that were once created and freed within the routines
        
        /* Just for testing purposes
        for(sp=0; sp<bm->K_num_tot_sp; sp++){
            if(FunctGroupArray[sp].speciesParams[assess_flag_id] > 0)
                GenData(bm, sp, 0);
        }
        */
        
	}
    
    if (bm->useMultispAssess) {
        bm->RBCestimation.earliestYr = MAXINT;
        for(sp=0; sp<bm->K_num_tot_sp; sp++){
            if (speciesParamStructArray[sp].paramType == SP_IMPACTED) {
                if ((FunctGroupArray[sp].speciesParams[MultispAssessType_id] > 0) && ((int)(FunctGroupArray[sp].speciesParams[MultispAssessType_id]) < (IndicatorSpPGMSY + 1))) {
                    bm->PGMSY_on = TRUE;
                    // Make sure the R file is in place
                
                    fprintf(bm->logFile,"Set PGMSY_on as %s has MultispAssessType %e vs threshold %d\n", FunctGroupArray[sp].groupCode,FunctGroupArray[sp].speciesParams[MultispAssessType_id], (IndicatorSpPGMSY + 1));
                
                }
            }
            if ((FunctGroupArray[sp].isFished == TRUE)  && ((int)(bm->RBCestimation.RBCspeciesParam[sp][AssessStart_id]) < bm->RBCestimation.earliestYr)) {
                bm->RBCestimation.earliestYr = bm->RBCestimation.RBCspeciesParam[sp][AssessStart_id];
            }
            
            fprintf(bm->logFile,"Setting earliestYr as %s has isFished: %d AssessStart %d vs earliestYr %d\n", FunctGroupArray[sp].groupCode, FunctGroupArray[sp].isFished, ((int)(bm->RBCestimation.RBCspeciesParam[sp][AssessStart_id])), bm->RBCestimation.earliestYr);
        }
        bm->RBCestimation.ReviewYr = bm->RBCestimation.earliestYr;
    }
    
    fflush(bm->logFile);
    
	/* Shutdown libxml */
	xmlFreeDoc(inputDoc);

	xmlCleanupParser();

	return TRUE;
}


/******************************************** Parameters for tiered assessments *************************************************/

xmlNodePtr getParentNode(MSEBoxModel *bm, char *fileName, xmlNodePtr parent, char *nodeName, char *errorString, int errorBuf) {

	char *parentNodeName = Util_Get_Node_Name(parent);
	xmlNodePtr attributeGroup;

	snprintf(errorString, errorBuf, "%s/%s", parentNodeName, nodeName);
	attributeGroup = Util_XML_Get_Node(ATLANTIS_ATTRIBUTE, parent, nodeName);
	if (attributeGroup == NULL)
		quit("readGroupAssessTierXMLData - %s attribute group not found.\n", errorString);

	return attributeGroup;
}

void readGroupAssessTierXMLData(MSEBoxModel *bm, char *fileName, xmlNodePtr attributeGroup, int guild, double *array, ATLANTIS_CHECK_TYPES checkType,
		char *errorString, int length) {
	double *values = 0;
	int index;

	if (FunctGroupArray[guild].isFished == TRUE) {
		if (Util_XML_Read_Array_Double(ATLANTIS_GROUP_ATTRIBUTE, fileName, errorString, attributeGroup, checkType, FunctGroupArray[guild].groupCode, &values,
				length) == FALSE) {
			quit("Error: Unable to find parameter '%s/%s' in input file %s\n", errorString, FunctGroupArray[guild].groupCode, fileName);
		}

		for (index = 0; index < length; index++)
			array[index] = values[index];
		free(values);

	}

}

/**
 * \brief Read in the stock, sex and age specific values
 *
 */
void readStockAgeAssessTierXMLData(MSEBoxModel *bm, char *fileName, xmlNodePtr attributeGroup, int guild, double ***array, ATLANTIS_CHECK_TYPES checkType,
                                char *errorString, int length) {
	double *values = 0;
	int index, ns, nstock, nloop;
    xmlNodePtr speciesNode, sexNode;
    char str[STRLEN];
    char eString[STRLEN*2];
    
    nloop = (int)bm->RBCestimation.RBCspeciesParam[guild][NumRegions_id];
    if (FunctGroupArray[guild].speciesParams[assess_flag_id] < 1)
        nloop = 1;
    
    if (bm->assess_share_params)
        nloop = 1;
    

    if (verbose)
        printf("Reading stock based assessment parameters\n");

    if (FunctGroupArray[guild].isFished == TRUE) {
            
        speciesNode = Util_XML_Get_Node(ATLANTIS_GROUP_ATTRIBUTE, attributeGroup, FunctGroupArray[guild].groupCode);
        if (speciesNode == NULL)
            quit("%s/%s species attribute group not found.\n", errorString, FunctGroupArray[guild].groupCode);
            
        for (ns = 0; ns < bm->K_num_sexes; ns++) {
            snprintf(str, sizeof(str), "%s", sexStrings[ns]);
            snprintf(eString, sizeof(eString), "%s/%s/%s\n", errorString, FunctGroupArray[guild].groupCode, str);
                
            sexNode = Util_XML_Get_Node(ATLANTIS_SEX_ATTRIBUTE, speciesNode, str);
            if (sexNode == NULL)
                quit("%s gender attribute group not found.\n", eString);
                
            for (nstock = 0; nstock < nloop; nstock++) {
                snprintf(str, sizeof(str), "stock%d", nstock+1);
        
                if (Util_XML_Read_Array_Double(ATLANTIS_ATTRIBUTE, fileName, eString, sexNode, checkType, str, &values, length) == FALSE) {
                    quit("Error: Unable to find parameter '%s/%s/%s' in input file %s\n", errorString, FunctGroupArray[guild].groupCode, str, fileName);
                }

                for (index = 0; index < length; index++) {
                    array[nstock][ns][index] = values[index];
                }
                free(values);
            }
        }
        
        
        // Copy to other stocks
        if ((bm->assess_share_params) && (FunctGroupArray[guild].speciesParams[assess_flag_id] > 0)) {
            for (nstock = nloop; nstock < bm->RBCestimation.RBCspeciesParam[guild][NumRegions_id]; nstock++) {
                for (ns = 0; ns < bm->K_num_sexes; ns++) {
                    for (index = 0; index < length; index++) {
                        array[nstock][ns][index] = array[0][ns][index];
                    }
            	}
            }
        }
	}
}

/**
 * \brief Read in the stock, sex and age specific values
 *
 */
void readStockAssessTierXMLData(MSEBoxModel *bm, char *fileName, xmlNodePtr attributeGroup, int guild, double **array, ATLANTIS_CHECK_TYPES checkType,
                                   char *errorString, int length) {
	double *values = 0;
	int index, nstock;
    xmlNodePtr speciesNode;
    char str[STRLEN];
    char eString[STRLEN*2];
    int nloop = (int)bm->RBCestimation.RBCspeciesParam[guild][NumRegions_id];
    
    if (bm->assess_share_params)
        nloop = 1;
    
    if ((FunctGroupArray[guild].isFished == TRUE) && (FunctGroupArray[guild].speciesParams[assess_flag_id] > 0)){
        
        speciesNode = Util_XML_Get_Node(ATLANTIS_GROUP_ATTRIBUTE, attributeGroup, FunctGroupArray[guild].groupCode);
        if (speciesNode == NULL)
            quit("%s/%s species attribute group not found.\n", errorString, FunctGroupArray[guild].groupCode);

        for (nstock = 0; nstock < nloop; nstock++) {
            snprintf(str, sizeof(str), "stock%d", nstock+1);
            snprintf(eString, sizeof(eString), "%s/%s/%s\n", errorString, FunctGroupArray[guild].groupCode, str);
            
            if (Util_XML_Read_Array_Double(ATLANTIS_ATTRIBUTE, fileName, eString, speciesNode, checkType, str, &values, length) == FALSE) {
                quit("Error: Unable to find parameter '%s/%s/%s' in input file %s\n", errorString, FunctGroupArray[guild].groupCode, str, fileName);
            }
                
            for (index = 0; index < length; index++) {
                array[nstock][index] = values[index];
            }
            free(values);
        }
        
        // Copy to other stocks
        if (bm->assess_share_params) {
            for (nstock = nloop; nstock < bm->RBCestimation.RBCspeciesParam[guild][NumRegions_id]; nstock++) {
                for (index = 0; index < length; index++) {
                    array[nstock][index] = array[0][index];
                }
            }
        }

	}
}


/**
 * \brief Read in the species parameters from the given file into the RBCestimate parameter array.
 *
 */
void Util_XML_Read_Species_RBCParam(MSEBoxModel *bm, char *fileName, xmlNodePtr parent, int paramID) {
	int guild;
	xmlNodePtr attributeGroup;
	int index;

	/* Get the index of this paramID in the speciesParamStructArray structure - done this way so the
	 * values in speciesParamStructArray don't have to be in any particular order
	 */
	index = Util_XML_Get_Param_Index(RBCSpeciesParamStructArray, num_rbc_species_params_id, paramID);

	if (verbose > 1)
		printf("Read species parameters %s\n", RBCSpeciesParamStructArray[index].tag);

	attributeGroup = Util_XML_Get_Node(ATLANTIS_ATTRIBUTE, parent, RBCSpeciesParamStructArray[index].tag);
	if (attributeGroup == NULL)
		quit("%s/%s attribute group not found in file %s.\n", parent->name, RBCSpeciesParamStructArray[index].tag, fileName);

	switch (RBCSpeciesParamStructArray[index].paramType) {

	/* All functional groups */
		case SP_FISHED:
			for (guild = 0; guild < bm->K_num_tot_sp; guild++) {
				if ((FunctGroupArray[guild].speciesParams[flag_id] == TRUE) && (FunctGroupArray[guild].isFished == TRUE)) {
						bm->RBCestimation.RBCspeciesParam[guild][paramID] = Util_XML_Get_Species_Node_Value(attributeGroup, index, guild, RBCSpeciesParamStructArray);
				}
			}
			break;

			/* Impacted species variables */
		case SP_IMPACTED:
			for (guild = 0; guild < bm->K_num_tot_sp; guild++) {
				if ((FunctGroupArray[guild].speciesParams[flag_id] == TRUE) && (FunctGroupArray[guild].isImpacted == TRUE)) {
					bm->RBCestimation.RBCspeciesParam[guild][paramID] = Util_XML_Get_Species_Node_Value(attributeGroup, index, guild, RBCSpeciesParamStructArray);
				}
			}
			break;

		default:
			quit("Util_XML_Read_Species_RBCParam: paramID %s is not found. Add to the switch statement in Util_XML_Read_Species_RBCParam.\n",
					RBCSpeciesParamStructArray[index].tag);
			break;
	}

}

/**************************************************************************//**
 *	\brief Reads a list of time series of TAC_mult changes from a ts file.
 *
 *	@param bm Pointer to box model
 *	@param key Pointer to name of series to read
 *	@param ts Pointer to array of TimeSeries structures to populate - not yet allocated.
 *	@param t_units Pointer to time units used by the hydrological model
 *	@param err Pointer to the error function to be called if there is an error.
 *****************************************************************************/
/* Fill in the values for the time series */
void Fill_TACmult(MSEBoxModel *bm){
	int b, sp, scale_index, titer;
	double scalar, tstit;
	char str[50];

	if(bm->tsTACmult != NULL){
		/* Set up the indexing on the names in the forcing file */
		for (sp = 0; sp < bm->K_num_tot_sp; sp++) {
			if ((FunctGroupArray[sp].speciesParams[flag_id] == TRUE) && ((FunctGroupArray[sp].isImpacted == TRUE) || (FunctGroupArray[sp].speciesParams[sp] == TRUE))) {
				snprintf(str, sizeof(str), "%s", FunctGroupArray[sp].groupCode);
				for (b = 0; b < bm->tsTACmult->nv; b++) {
					if (strcmp(str, bm->tsTACmult->varname[b]) == 0){
						scale_index = b;
						if(scale_index != -1){
							for(titer = 0; titer < bm->RBCestimation.RBCspeciesParam[sp][Nyears_id]; titer++){
								tstit = titer * 365.0 * 86400.0;
								scalar = tsEval(bm->tsTACmult, scale_index, tstit);
								bm->RBCestimation.RBCspeciesArray[sp].TAC_mult[titer] = scalar;

								//printf("%s yr: %d TAC_mult %e\n", FunctGroupArray[sp].groupCode, titer, bm->RBCestimation.RBCspeciesArray[sp].TAC_mult[titer]);
							}
						}
					}
				}
			}
		}
	}
}

void get_TAC_mult(MSEBoxModel *bm, char *key, TimeSeries **ts, char *t_units, void(*err)(char *format, ...)) {
	FILE *fp;
	char buf[BMSLEN];

	printf("Reading in TAC_mult time series forcing file\n");
	if (verbose > 1)
        printf("Reading in TAC_mult time series forcing file\n");

	/* Open the file */
	if ((fp = fopen(bm->assessprmIfname, "r")) == NULL)
		quit("get_TAC_mult: Can't open %s\n", bm->forceIfname);

	/** Get time series **/
	/* Find time series name in parameter file */
	set_keyprm_errfn(err);
	if (!readkeyprm_s(fp, key, buf)) {
		fclose(fp);
		return;
	}

	if (verbose)
		fprintf(stderr, "get_TAC_mult: reading from %s\n", key);

	/* Allocate space for time series */
	if (*ts == NULL && (*ts = (TimeSeries *) malloc(sizeof(TimeSeries))) == NULL)
		quit("get_TAC_mult: Can't allocate memory for %s time series\n", key);

	/* Read recruitment series data */
	tsRead(bm->inputFolder, buf, *ts);

	/* Close the file */
	fclose(fp);

	Fill_TACmult(bm);

	return;
}


/*
 * Tier Assessment XML parameter read
 */
void readKeyTierParamXML(MSEBoxModel *bm, char *fileName, xmlNodePtr rootnode) {
	xmlNodePtr groupingNode, childGroupingNode;
    
	//printf("Creating Tier Assessment arrays on read-in\n");
    
	groupingNode = Util_XML_Get_Node(ATLANTIS_ATTRIBUTE_SUB_GROUP, rootnode, "Tier_Assessment");
	if (groupingNode == NULL)
		quit("readTierAssessmentManagementXML: Tier_Assessment attribute group not found in input file %s.\n", fileName);
    
    bm->assess_share_params = (int)Util_XML_Read_Value(fileName, ATLANTIS_ATTRIBUTE, bm->ecotest, 1, groupingNode, binary_check, "assess_share_params");
	bm->K_num_sexes = (int)Util_XML_Read_Value(fileName, ATLANTIS_ATTRIBUTE, bm->ecotest, 1, groupingNode, binary_check, "Num_Sexes");
    bm->RBCestimation.HistYrMin = (int)Util_XML_Read_Value(fileName, ATLANTIS_ATTRIBUTE, bm->ecotest, 1, groupingNode, integer_check, "HistYrMin");
	bm->RBCestimation.HistYrMax = (int)Util_XML_Read_Value(fileName, ATLANTIS_ATTRIBUTE, bm->ecotest, 1, groupingNode, integer_check, "HistYrMax");
	bm->RBCestimation.OverallMaxAge = (int)Util_XML_Read_Value(fileName, ATLANTIS_ATTRIBUTE, bm->ecotest, 1, groupingNode, integer_check, "OverallMaxAge");
    
    bm->RBCestimation.tier6_to_tier4 = (int)Util_XML_Read_Value(fileName, ATLANTIS_ATTRIBUTE, bm->ecotest, 1, groupingNode, integer_check, "tier6_to_tier4");
    bm->RBCestimation.delTAE = Util_XML_Read_Value(fileName, ATLANTIS_ATTRIBUTE, bm->ecotest, 1, groupingNode, no_checking, "delTAE");
    
	/* Now allocate the rest of the memory */
    bm->RBCestimation.UseAtlantisPGMSY = (int) (Util_XML_Read_Value(fileName, ATLANTIS_ATTRIBUTE, bm->ecotest, 1, groupingNode, binary_check, "UseAtlantisPGMSY"));
	bm->RBCestimation.UseSS = (int) (Util_XML_Read_Value(fileName, ATLANTIS_ATTRIBUTE, bm->ecotest, 1, groupingNode, binary_check, "UseSS"));
    bm->RBCestimation.AssessDelay = (int) (Util_XML_Read_Value(fileName, ATLANTIS_ATTRIBUTE, bm->ecotest, 1, groupingNode, integer_check, "AssessDelay"));
    bm->RBCestimation.UseTierBuffers = (int) (Util_XML_Read_Value(fileName, ATLANTIS_ATTRIBUTE, bm->ecotest, 1, groupingNode, binary_check, "UseTierBuffers"));
    bm->RBCestimation.myTACbuffer = (int) (Util_XML_Read_Value(fileName, ATLANTIS_ATTRIBUTE, bm->ecotest, 1, groupingNode, no_checking, "myTACbuffer"));
    bm->RBCestimation.myTACperiod = (int) (Util_XML_Read_Value(fileName, ATLANTIS_ATTRIBUTE, bm->ecotest, 1, groupingNode, integer_check, "myTACperiod"));
    bm->RBCestimation.sexratio_cv = Util_XML_Read_Value(fileName, ATLANTIS_ATTRIBUTE, bm->ecotest, 1, groupingNode, no_checking, "sexratio_cv");
    
    bm->RBCestimation.ReviewPeriod = (int) Util_XML_Read_Value(fileName, ATLANTIS_ATTRIBUTE, bm->ecotest, 1, groupingNode, integer_check, "ReviewPeriod");
    bm->RBCestimation.TriggerCheckPeriod = (int) Util_XML_Read_Value(fileName, ATLANTIS_ATTRIBUTE, bm->ecotest, 1, groupingNode, integer_check, "TriggerCheckPeriod");
    bm->RBCestimation.LagPeriod = (int) Util_XML_Read_Value(fileName, ATLANTIS_ATTRIBUTE, bm->ecotest, 1, groupingNode, integer_check, "LagPeriod");
    bm->RBCestimation.GradientWindow = (int) Util_XML_Read_Value(fileName, ATLANTIS_ATTRIBUTE, bm->ecotest, 1, groupingNode, integer_check, "GradientWindow");
    bm->RBCestimation.GradientPeriod = (int) Util_XML_Read_Value(fileName, ATLANTIS_ATTRIBUTE, bm->ecotest, 1, groupingNode, integer_check, "GradientPeriod");
    bm->RBCestimation.GradientBuffer = Util_XML_Read_Value(fileName, ATLANTIS_ATTRIBUTE, bm->ecotest, 1, groupingNode, no_checking, "GradientBuffer");
    bm->RBCestimation.UseCategory = (int) Util_XML_Read_Value(fileName, ATLANTIS_ATTRIBUTE, bm->ecotest, 1, groupingNode, binary_check, "UseCategory");
    bm->RBCestimation.UseClosest = (int) Util_XML_Read_Value(fileName, ATLANTIS_ATTRIBUTE, bm->ecotest, 1, groupingNode, binary_check, "UseClosest");
    bm->RBCestimation.UseTriggerMgmt = (int) Util_XML_Read_Value(fileName, ATLANTIS_ATTRIBUTE, bm->ecotest, 1, groupingNode, binary_check, "UseTriggerMgmt");
    bm->RBCestimation.ProjYr = (int) Util_XML_Read_Value(fileName, ATLANTIS_ATTRIBUTE, bm->ecotest, 1, groupingNode, integer_check, "ProjYr");
    
    bm->RBCestimation.ThresholdDepletion = Util_XML_Read_Value(fileName, ATLANTIS_ATTRIBUTE, bm->ecotest, 1, groupingNode, no_checking, "ThresholdDepletion");
    bm->RBCestimation.ThresholdBound = Util_XML_Read_Value(fileName, ATLANTIS_ATTRIBUTE, bm->ecotest, 1, groupingNode, no_checking, "ThresholdBound");
    bm->RBCestimation.MaxIteration = (int) Util_XML_Read_Value(fileName, ATLANTIS_ATTRIBUTE, bm->ecotest, 1, groupingNode, integer_check, "MaxIteration");
    
	childGroupingNode = Util_XML_Get_Node(ATLANTIS_ATTRIBUTE_SUB_GROUP, groupingNode, "Seeds");
	if (childGroupingNode == NULL)
		quit("readTierAssessmentManagementXML: Seeds attribute group not found in input file %s.\n", fileName);
    
	bm->RBCestimation.Iseedx = (int)Util_XML_Read_Value(fileName, ATLANTIS_ATTRIBUTE, bm->ecotest, 1, childGroupingNode, no_checking, "Iseedx");
	bm->RBCestimation.Iseedy = (int)Util_XML_Read_Value(fileName, ATLANTIS_ATTRIBUTE, bm->ecotest, 1, childGroupingNode, no_checking, "Iseedy");
	bm->RBCestimation.Iseedz = (int)Util_XML_Read_Value(fileName, ATLANTIS_ATTRIBUTE, bm->ecotest, 1, childGroupingNode, no_checking, "Iseedz");
    
	// If seeds not provided generate them off the clock
	if (!bm->RBCestimation.Iseedx) bm->RBCestimation.Iseedx = (int)clock();
	if (!bm->RBCestimation.Iseedy) bm->RBCestimation.Iseedy = (int)clock();
	if (!bm->RBCestimation.Iseedz) bm->RBCestimation.Iseedz = (int)clock();
    
    /* SS related flags */
    bm->RBCestimation.SSForecastType = (int) (Util_XML_Read_Value(fileName, ATLANTIS_ATTRIBUTE, bm->ecotest, 1, groupingNode, integer_check, "SSForecastType"));
    bm->RBCestimation.SSFscalar = Util_XML_Read_Value(fileName, ATLANTIS_ATTRIBUTE, bm->ecotest, 1, groupingNode, no_checking, "SSFscalar");
    
    bm->RBCestimation.SSControlRule = (int) (Util_XML_Read_Value(fileName, ATLANTIS_ATTRIBUTE, bm->ecotest, 1, groupingNode, integer_check, "SSControlRule"));
    bm->RBCestimation.SSControlRuleB = Util_XML_Read_Value(fileName, ATLANTIS_ATTRIBUTE, bm->ecotest, 1, groupingNode, no_checking, "SSControlRuleB");
    bm->RBCestimation.SSControlRuleTargF = Util_XML_Read_Value(fileName, ATLANTIS_ATTRIBUTE, bm->ecotest, 1, groupingNode, no_checking, "SSControlRuleTargF");
    bm->RBCestimation.SSLoop = (int) (Util_XML_Read_Value(fileName, ATLANTIS_ATTRIBUTE, bm->ecotest, 1, groupingNode, integer_check, "SSLoop"));
    bm->RBCestimation.SSLoopWithRandRec = (int) (Util_XML_Read_Value(fileName, ATLANTIS_ATTRIBUTE, bm->ecotest, 1, groupingNode, integer_check, "SSLoopWithRandRec"));
    bm->RBCestimation.SSTol = Util_XML_Read_Value(fileName, ATLANTIS_ATTRIBUTE, bm->ecotest, 1, groupingNode, no_checking, "SSTol");
    bm->RBCestimation.SSDepletionBasis = (int) (Util_XML_Read_Value(fileName, ATLANTIS_ATTRIBUTE, bm->ecotest, 1, groupingNode, integer_check, "SSDepletionBasis"));
    bm->RBCestimation.SSFractX = Util_XML_Read_Value(fileName, ATLANTIS_ATTRIBUTE, bm->ecotest, 1, groupingNode, no_checking, "SSFractX");
    bm->RBCestimation.SS_SPRreport = (int) (Util_XML_Read_Value(fileName, ATLANTIS_ATTRIBUTE, bm->ecotest, 1, groupingNode, integer_check, "SS_SPRreport"));
    bm->RBCestimation.SS_Freport = (int) (Util_XML_Read_Value(fileName, ATLANTIS_ATTRIBUTE, bm->ecotest, 1, groupingNode, integer_check, "SS_Freport"));
    bm->RBCestimation.SSFreportBasis = (int) (Util_XML_Read_Value(fileName, ATLANTIS_ATTRIBUTE, bm->ecotest, 1, groupingNode, integer_check, "SSFreportBasis"));
    bm->RBCestimation.SSnoBiasAdj = (int) (Util_XML_Read_Value(fileName, ATLANTIS_ATTRIBUTE, bm->ecotest, 1, groupingNode, integer_check, "SSnoBiasAdj"));
    bm->RBCestimation.SSnumGrowthPatterns = (int) (Util_XML_Read_Value(fileName, ATLANTIS_ATTRIBUTE, bm->ecotest, 1, groupingNode, integer_check, "SSnumGrowthPatterns"));
    
    bm->RBCestimation.Nmetiers = (int)Util_XML_Read_Value(fileName, ATLANTIS_ATTRIBUTE, bm->ecotest, 1, groupingNode, integer_check, "Nmetiers");
    
    /** assessment specifications */
    childGroupingNode = Util_XML_Get_Node(ATLANTIS_ATTRIBUTE_SUB_GROUP, groupingNode, "GenSS");
	if (childGroupingNode == NULL)
		quit("readTierAssessmentManagementXML: GenSS attribute group not found in input file %s.\n", fileName);
    
	Util_XML_Read_Species_RBCParam(bm, fileName, childGroupingNode, MaxAge_id);
	Util_XML_Read_Species_RBCParam(bm, fileName, childGroupingNode, Nlen_id);
    Util_XML_Read_Species_RBCParam(bm, fileName, childGroupingNode, NumFisheries_id);
    Util_XML_Read_Species_RBCParam(bm, fileName, childGroupingNode, NumRegions_id);
    Util_XML_Read_Species_RBCParam(bm, fileName, childGroupingNode, Nsexes_id);
    
    // Added from Ratpack
    Util_XML_Read_Species_RBCParam(bm, fileName, childGroupingNode, LFSSlim_id);
    Util_XML_Read_Species_RBCParam(bm, fileName, childGroupingNode, AFSSlim_id);
    Util_XML_Read_Species_RBCParam(bm, fileName, childGroupingNode, NumSurvey_id);
    Util_XML_Read_Species_RBCParam(bm, fileName, childGroupingNode, Regime_year_assess_id);
    Util_XML_Read_Species_RBCParam(bm, fileName, childGroupingNode, NblockPattern_id);
    Util_XML_Read_Species_RBCParam(bm, fileName, childGroupingNode, SRBlock_id);
    Util_XML_Read_Species_RBCParam(bm, fileName, childGroupingNode, assRecDevMinYear_id);
    
    // Multispecies
    Util_XML_Read_Species_RBCParam(bm, fileName, childGroupingNode, MultispAssessType_id);
    Util_XML_Read_Species_RBCParam(bm, fileName, childGroupingNode, mgt_indicator_id);
    Util_XML_Read_Species_RBCParam(bm, fileName, childGroupingNode, init_mgt_category_id);
    Util_XML_Read_Species_RBCParam(bm, fileName, childGroupingNode, init_mgt_sp_id);
    
    // PGMSY
    Util_XML_Read_Species_RBCParam(bm, fileName, childGroupingNode, PGMSYBHalpha_id);
    Util_XML_Read_Species_RBCParam(bm, fileName, childGroupingNode, PGMSYBHbeta_id);
 
    
	/** tier specific rules */
	// tier 3
    childGroupingNode = Util_XML_Get_Node(ATLANTIS_ATTRIBUTE_SUB_GROUP, groupingNode, "Tier3");
	if (childGroupingNode == NULL)
		quit("readTierAssessmentManagementXML: Tier3 attribute group not found in input file %s.\n", fileName);
    
	Util_XML_Read_Species_RBCParam(bm, fileName, childGroupingNode, Tier3_maxage_id);
	Util_XML_Read_Species_RBCParam(bm, fileName, childGroupingNode, CCsel_years_id);
    
    bm->useGenMnomial = (int) (Util_XML_Read_Value(fileName, ATLANTIS_ATTRIBUTE,  bm->ecotest, 1, groupingNode, binary_check, "useGenMnomial"));

    // tier 6
    
    childGroupingNode = Util_XML_Get_Node(ATLANTIS_ATTRIBUTE_SUB_GROUP, groupingNode, "Tier6");
	if (childGroupingNode == NULL)
		quit("readTierAssessmentManagementXML: Tier6 attribute group not found in input file %s.\n", fileName);
    Util_XML_Read_Species_RBCParam(bm, fileName, childGroupingNode, TriggerResponseScen_id);
    
    bm->RBCestimation.NumTriggers = (int) (Util_XML_Read_Value(fileName, ATLANTIS_ATTRIBUTE, bm->ecotest, 1, groupingNode, integer_check, "NumTriggers"));
    if(Util_XML_Read_Array_Double(ATLANTIS_ATTRIBUTE, fileName, "Tier_Assessment", groupingNode, no_checking, "TriggerPoints", &TrigPts, bm->RBCestimation.NumTriggers) == FALSE){
		quit("Error: Unable to read parameter 'Tier6/TriggerPoints' from input file %s\n", fileName);
	}
    
    /* Close kin parameters */
    bm->CloseKinEst->UseCloseKin = Util_XML_Read_Value(fileName, ATLANTIS_ATTRIBUTE, bm->ecotest, 1, groupingNode, integer_check, "UseCloseKin");
    bm->CloseKinEst->nTotSamps = Util_XML_Read_Value(fileName, ATLANTIS_ATTRIBUTE, bm->ecotest, 1, groupingNode, integer_check, "nTotSamps");
    bm->CloseKinEst->first_sy = Util_XML_Read_Value(fileName, ATLANTIS_ATTRIBUTE, bm->ecotest, 1, groupingNode, integer_check, "first_sy");
    bm->CloseKinEst->last_sy = Util_XML_Read_Value(fileName, ATLANTIS_ATTRIBUTE, bm->ecotest, 1, groupingNode, integer_check, "last_sy");
    bm->CloseKinEst->first_dy = Util_XML_Read_Value(fileName, ATLANTIS_ATTRIBUTE, bm->ecotest, 1, groupingNode, integer_check, "first_dy");
    bm->CloseKinEst->Nscale = Util_XML_Read_Value(fileName, ATLANTIS_ATTRIBUTE, bm->ecotest, 1, groupingNode, integer_check, "Nscale");
    bm->CloseKinEst->ThreshPercLost = Util_XML_Read_Value(fileName, ATLANTIS_ATTRIBUTE, bm->ecotest, 1, groupingNode, integer_check, "ThreshPercLost");
    bm->CloseKinEst->samp_power = Util_XML_Read_Value(fileName, ATLANTIS_ATTRIBUTE, bm->ecotest, 1, groupingNode, integer_check, "samp_power");
    bm->CloseKinEst->a_ck_thresh = Util_XML_Read_Value(fileName, ATLANTIS_ATTRIBUTE, bm->ecotest, 1, groupingNode, integer_check, "a_ck_thresh");
    bm->CloseKinEst->a_ck_thresh2 = Util_XML_Read_Value(fileName, ATLANTIS_ATTRIBUTE, bm->ecotest, 1, groupingNode, no_checking, "a_ck_thresh2");
    bm->CloseKinEst->a_ck_scalar = Util_XML_Read_Value(fileName, ATLANTIS_ATTRIBUTE, bm->ecotest, 1, groupingNode, no_checking, "a_ck_scalar");
    bm->CloseKinEst->a_ck_scalar2 = Util_XML_Read_Value(fileName, ATLANTIS_ATTRIBUTE, bm->ecotest, 1, groupingNode, no_checking, "a_ck_scalar2");
    bm->CloseKinEst->thresh_mat = Util_XML_Read_Value(fileName, ATLANTIS_ATTRIBUTE, bm->ecotest, 1, groupingNode, no_checking, "thresh_mat");
    bm->CloseKinEst->HSP_false_neg_loss_rate = Util_XML_Read_Value(fileName, ATLANTIS_ATTRIBUTE, bm->ecotest, 1, groupingNode, no_checking, "HSP_false_neg_loss_rate");

}


/*
 * Tier Assessment XML parameter read
 */
void readTierAssessmentManagementXML(MSEBoxModel *bm, char *fileName, xmlNodePtr rootnode) {
	xmlNodePtr groupingNode, childGroupingNode, attributeGroup;
	char errorString[STRLEN];
	int guild, b, nf;
    double *dummy;
    char nameString[BMSLEN];

	//printf("Creating Tier Assessment arrays on read-in\n");

	groupingNode = Util_XML_Get_Node(ATLANTIS_ATTRIBUTE_SUB_GROUP, rootnode, "Tier_Assessment");
	if (groupingNode == NULL)
		quit("readTierAssessmentManagementXML: Tier_Assessment attribute group not found in input file %s.\n", fileName);
    
    /* SS related flags */
	bm->RBCestimation.CPUElim = Util_XML_Read_Value(fileName, ATLANTIS_ATTRIBUTE, bm->ecotest, 1, groupingNode, no_checking, "CPUElim");
	bm->RBCestimation.num_avg_year = Util_XML_Read_Value(fileName, ATLANTIS_ATTRIBUTE, bm->ecotest, 1, groupingNode, no_checking, "num_avg_year");
	bm->RBCestimation.Tier1_cv = Util_XML_Read_Value(fileName, ATLANTIS_ATTRIBUTE, bm->ecotest, 1, groupingNode, no_checking, "Tier1_cv");

	bm->RBCestimation.tier3RuleA = Util_XML_Read_Value(fileName, ATLANTIS_ATTRIBUTE, bm->ecotest, 1, groupingNode, no_checking, "tier3RuleA");
	bm->RBCestimation.tier3RuleB = Util_XML_Read_Value(fileName, ATLANTIS_ATTRIBUTE, bm->ecotest, 1, groupingNode, no_checking, "tier3RuleB");
	bm->RBCestimation.tier3RuleC = Util_XML_Read_Value(fileName, ATLANTIS_ATTRIBUTE, bm->ecotest, 1, groupingNode, no_checking, "tier3RuleC");
	bm->RBCestimation.tier3RuleD = Util_XML_Read_Value(fileName, ATLANTIS_ATTRIBUTE, bm->ecotest, 1, groupingNode, no_checking, "tier3RuleD");
	bm->RBCestimation.tier3RuleE = Util_XML_Read_Value(fileName, ATLANTIS_ATTRIBUTE, bm->ecotest, 1, groupingNode, no_checking, "tier3RuleE");
	bm->RBCestimation.tier3RuleF = Util_XML_Read_Value(fileName, ATLANTIS_ATTRIBUTE, bm->ecotest, 1, groupingNode, no_checking, "tier3RuleF");
    
	bm->RBCestimation.tier5JensenConst = Util_XML_Read_Value(fileName, ATLANTIS_ATTRIBUTE, bm->ecotest, 1, groupingNode, no_checking, "tier5JensenConst");
	bm->RBCestimation.tier5limscalar = Util_XML_Read_Value(fileName, ATLANTIS_ATTRIBUTE, bm->ecotest, 1, groupingNode, no_checking, "tier5limscalar");
	bm->RBCestimation.tier5omega = Util_XML_Read_Value(fileName, ATLANTIS_ATTRIBUTE, bm->ecotest, 1, groupingNode, no_checking, "tier5omega");
	bm->RBCestimation.tier5DCACscalar = Util_XML_Read_Value(fileName, ATLANTIS_ATTRIBUTE, bm->ecotest, 1, groupingNode, no_checking, "tier5DCACscalar");
	bm->RBCestimation.tier5DCACconst = Util_XML_Read_Value(fileName, ATLANTIS_ATTRIBUTE, bm->ecotest, 1, groupingNode, no_checking, "tier5DCACconst");

 	bm->RBCestimation.tier7areachange = Util_XML_Read_Value(fileName, ATLANTIS_ATTRIBUTE, bm->ecotest, 1, groupingNode, no_checking, "tier7areachange");
	bm->RBCestimation.tier7relarea = Util_XML_Read_Value(fileName, ATLANTIS_ATTRIBUTE, bm->ecotest, 1, groupingNode, no_checking, "tier7relarea");
	bm->RBCestimation.tier7cpuechange = Util_XML_Read_Value(fileName, ATLANTIS_ATTRIBUTE, bm->ecotest, 1, groupingNode, no_checking, "tier7cpuechange");
	bm->RBCestimation.tier7maxcompchange = Util_XML_Read_Value(fileName, ATLANTIS_ATTRIBUTE, bm->ecotest, 1, groupingNode, no_checking, "tier7maxcompchange");
	bm->RBCestimation.tier7maxannchange = Util_XML_Read_Value(fileName, ATLANTIS_ATTRIBUTE, bm->ecotest, 1, groupingNode, no_checking, "tier7maxannchange");
	bm->RBCestimation.tier7period = Util_XML_Read_Value(fileName, ATLANTIS_ATTRIBUTE, bm->ecotest, 1, groupingNode, no_checking, "tier7period");
    
	bm->RBCestimation.Use_TAC_mult = (int)Util_XML_Read_Value(fileName, ATLANTIS_ATTRIBUTE, bm->ecotest, 1, groupingNode, no_checking, "Use_TAC_mult");
	get_TAC_mult(bm,"TAC_mult", &bm->tsTACmult, bm->t_units, warn);

	bm->RBCestimation.min_TACchange = (int)Util_XML_Read_Value(fileName, ATLANTIS_ATTRIBUTE, bm->ecotest, 1, groupingNode, no_checking, "RBC_TACchange_min");

    bm->RBCestimation.MULTISPestNspecies = (int)Util_XML_Read_Value(fileName, ATLANTIS_ATTRIBUTE, bm->ecotest, 1, groupingNode, integer_check, "MULTISPestNspecies");
    bm->RBCestimation.nRBCaverage = (int)Util_XML_Read_Value(fileName, ATLANTIS_ATTRIBUTE, bm->ecotest, 1, groupingNode, integer_check, "nRBCaverage");
    bm->RBCestimation.MaxCritConvergeValue = Util_XML_Read_Value(fileName, ATLANTIS_ATTRIBUTE, bm->ecotest, 1, groupingNode, no_checking, "MaxCritConvergeValue");
    bm->RBCestimation.ConvergeThresh = Util_XML_Read_Value(fileName, ATLANTIS_ATTRIBUTE, bm->ecotest, 1, groupingNode, no_checking, "ConvergeThresh");
    
    if (bm->useMultispAssess) {        
        Util_XML_Get_Value_String(fileName, ATLANTIS_ATTRIBUTE, 1, groupingNode, "ss3Name", bm->ss3Name);
        
        bm->NumRAssessScriptNames = (int)Util_XML_Read_Value(fileName, ATLANTIS_ATTRIBUTE,  bm->ecotest, 1, groupingNode, integer_check, "NumRAssessScriptNames");
        bm->RAssessRscriptName = c_alloc2d(BMSLEN, bm->NumRAssessScriptNames);
        for (nf = 0; nf < bm->NumRAssessScriptNames; nf ++) {
            snprintf(nameString, sizeof(nameString), "RAssessRscriptName%d", nf);
            Util_XML_Get_Value_String(fileName, ATLANTIS_ATTRIBUTE, 1, groupingNode, nameString, bm->RAssessRscriptName[nf]);
        }
        
        if(Util_XML_Read_Array_Integer(ATLANTIS_ATTRIBUTE, fileName, groupingNode, integer_check, "PGMSYlinks", &PGMSYLinksInputs, bm->K_num_fisheries) == FALSE){
            quit("Error: Unable to read parameter 'PGMSYlinks' from input file %s\n", fileName);
        }

    }

	/* Should change these all to check on if we are doing tier assessment for group instead of if they are being fished.
	 * We are not going to do a tier assessment for all groups in the model.
	 */
    
    childGroupingNode = Util_XML_Get_Node(ATLANTIS_ATTRIBUTE_SUB_GROUP, groupingNode, "CVs");
	if (childGroupingNode == NULL)
		quit("readTierAssessmentManagementXML: CVs attribute group not found in input file %s.\n", fileName);
    
    attributeGroup = getParentNode(bm, fileName, childGroupingNode, "DiscardCV", errorString, sizeof(errorString));
	for (guild = 0; guild < bm->K_num_tot_sp; guild++) {
        if (FunctGroupArray[guild].speciesParams[assess_flag_id] > 0)
            readGroupAssessTierXMLData(bm, fileName, attributeGroup, guild, bm->RBCestimation.RBCspeciesArray[guild].DiscCV, no_checking, errorString, (int)bm->RBCestimation.RBCspeciesParam[guild][NumFisheries_id]);
	}

    attributeGroup = getParentNode(bm, fileName, childGroupingNode, "CatchCV", errorString, sizeof(errorString));
	for (guild = 0; guild < bm->K_num_tot_sp; guild++) {
        if (FunctGroupArray[guild].speciesParams[assess_flag_id] > 0)
            readGroupAssessTierXMLData(bm, fileName, attributeGroup, guild, bm->RBCestimation.RBCspeciesArray[guild].CatchCV, no_checking, errorString, (int)bm->RBCestimation.RBCspeciesParam[guild][NumFisheries_id]);
	}

    attributeGroup = getParentNode(bm, fileName, childGroupingNode, "EffortCV", errorString, sizeof(errorString));
	for (guild = 0; guild < bm->K_num_tot_sp; guild++) {
        if (FunctGroupArray[guild].speciesParams[assess_flag_id] > 0)
            readGroupAssessTierXMLData(bm, fileName, attributeGroup, guild, bm->RBCestimation.RBCspeciesArray[guild].EffortCV, no_checking, errorString, (int)bm->RBCestimation.RBCspeciesParam[guild][NumFisheries_id]);
	}
    
    
	childGroupingNode = Util_XML_Get_Node(ATLANTIS_ATTRIBUTE_SUB_GROUP, groupingNode, "CPUE");
	if (childGroupingNode == NULL)
		quit("readTierAssessmentManagementXML: CPUE attribute group not found in input file %s.\n", fileName);

	attributeGroup = getParentNode(bm, fileName, childGroupingNode, "CPUECV", errorString, sizeof(errorString));
	for (guild = 0; guild < bm->K_num_tot_sp; guild++) {
        if (FunctGroupArray[guild].speciesParams[assess_flag_id] > 0)
            readGroupAssessTierXMLData(bm, fileName, attributeGroup, guild, bm->RBCestimation.RBCspeciesArray[guild].CPUEcv, no_checking, errorString, (int)bm->RBCestimation.RBCspeciesParam[guild][NumFisheries_id]);
	}

	attributeGroup = getParentNode(bm, fileName, childGroupingNode, "CPUEvar", errorString, sizeof(errorString));
	for (guild = 0; guild < bm->K_num_tot_sp; guild++) {
        if (FunctGroupArray[guild].speciesParams[assess_flag_id] > 0)
            readGroupAssessTierXMLData(bm, fileName, attributeGroup, guild, bm->RBCestimation.RBCspeciesArray[guild].CPUEvar, no_checking, errorString, (int)bm->RBCestimation.RBCspeciesParam[guild][NumFisheries_id]);
	}
	attributeGroup = getParentNode(bm, fileName, childGroupingNode, "CPUEcorr", errorString, sizeof(errorString));
	for (guild = 0; guild < bm->K_num_tot_sp; guild++) {
        if (FunctGroupArray[guild].speciesParams[assess_flag_id] > 0)
            readGroupAssessTierXMLData(bm, fileName, attributeGroup, guild, bm->RBCestimation.RBCspeciesArray[guild].CPUEcorr, no_checking, errorString, (int)bm->RBCestimation.RBCspeciesParam[guild][NumFisheries_id]);
	}
    
    /*
     attributeGroup = getParentNode(bm, fileName, childGroupingNode, "CPUEqmu", errorString, sizeof(errorString));
     for (guild = 0; guild < bm->K_num_tot_sp; guild++) {
     if (FunctGroupArray[guild].speciesParams[assess_flag_id] > 0)
        readGroupAssessTierXMLData(bm, fileName, attributeGroup, guild, bm->RBCestimation.RBCspeciesArray[guild].CPUEqmu, no_checking, errorString,  bm->K_num_fisheries);
     }
     
     attributeGroup = getParentNode(bm, fileName, childGroupingNode, "CPUEpow", errorString, sizeof(errorString));
     for (guild = 0; guild < bm->K_num_tot_sp; guild++) {
     if (FunctGroupArray[guild].speciesParams[assess_flag_id] > 0)
        readGroupAssessTierXMLData(bm, fileName, attributeGroup, guild, bm->RBCestimation.RBCspeciesArray[guild].CPUEpow, no_checking, errorString,  bm->K_num_fisheries);
     }
     */
    
    attributeGroup = getParentNode(bm, fileName, childGroupingNode, "PGMSY_q", errorString, sizeof(errorString));
    for (guild = 0; guild < bm->K_num_tot_sp; guild++) {
        if (FunctGroupArray[guild].speciesParams[assess_flag_id] > 0)
            readGroupAssessTierXMLData(bm, fileName, attributeGroup, guild, bm->RBCestimation.RBCspeciesArray[guild].PGMSY_q, no_checking, errorString, (int)bm->RBCestimation.RBCspeciesParam[guild][NumFisheries_id]);
    }
    
    attributeGroup = getParentNode(bm, fileName, childGroupingNode, "PGMSY_sel_lsm", errorString, sizeof(errorString));
    for (guild = 0; guild < bm->K_num_tot_sp; guild++) {
        if (FunctGroupArray[guild].speciesParams[assess_flag_id] > 0)
            readGroupAssessTierXMLData(bm, fileName, attributeGroup, guild, bm->RBCestimation.RBCspeciesArray[guild].PGMSY_sel_lsm, no_checking, errorString, (int)bm->RBCestimation.RBCspeciesParam[guild][NumFisheries_id]);
    }

    attributeGroup = getParentNode(bm, fileName, childGroupingNode, "PGMSY_sel_sigma", errorString, sizeof(errorString));
    for (guild = 0; guild < bm->K_num_tot_sp; guild++) {
        if (FunctGroupArray[guild].speciesParams[assess_flag_id] > 0)
            readGroupAssessTierXMLData(bm, fileName, attributeGroup, guild, bm->RBCestimation.RBCspeciesArray[guild].PGMSY_sel_sigma, no_checking, errorString, (int)bm->RBCestimation.RBCspeciesParam[guild][NumFisheries_id]);
    }
    
    attributeGroup = getParentNode(bm, fileName, childGroupingNode, "PGMSY_sel_curve", errorString, sizeof(errorString));
    for (guild = 0; guild < bm->K_num_tot_sp; guild++) {
        if (FunctGroupArray[guild].speciesParams[assess_flag_id] > 0)
            readGroupAssessTierXMLData(bm, fileName, attributeGroup, guild, bm->RBCestimation.RBCspeciesArray[guild].PGMSY_selcurve, no_checking, errorString, (int)bm->RBCestimation.RBCspeciesParam[guild][NumFisheries_id]);
    }
    
    childGroupingNode = Util_XML_Get_Node(ATLANTIS_ATTRIBUTE_SUB_GROUP, groupingNode, "StartSS");
	if (childGroupingNode == NULL)
		quit("readTierAssessmentManagementXML: StartSS attribute group not found in input file %s.\n", fileName);

    attributeGroup = getParentNode(bm, fileName, childGroupingNode, "Start_SelInflect", errorString, sizeof(errorString));
	for (guild = 0; guild < bm->K_num_tot_sp; guild++) {
        if (FunctGroupArray[guild].speciesParams[assess_flag_id] > 0)
            readGroupAssessTierXMLData(bm, fileName, attributeGroup, guild, bm->RBCestimation.RBCspeciesArray[guild].Start_SelInflect, no_checking, errorString, (int)bm->RBCestimation.RBCspeciesParam[guild][NumFisheries_id]);
	}

    attributeGroup = getParentNode(bm, fileName, childGroupingNode, "Start_SelWidth", errorString, sizeof(errorString));
	for (guild = 0; guild < bm->K_num_tot_sp; guild++) {
        if (FunctGroupArray[guild].speciesParams[assess_flag_id] > 0)
            readGroupAssessTierXMLData(bm, fileName, attributeGroup, guild, bm->RBCestimation.RBCspeciesArray[guild].Start_SelWidth, no_checking, errorString, (int)bm->RBCestimation.RBCspeciesParam[guild][NumFisheries_id]);
	}
    
    attributeGroup = getParentNode(bm, fileName, childGroupingNode, "Start_RetInflect", errorString, sizeof(errorString));
	for (guild = 0; guild < bm->K_num_tot_sp; guild++) {
        if (FunctGroupArray[guild].speciesParams[assess_flag_id] > 0)
            readGroupAssessTierXMLData(bm, fileName, attributeGroup, guild, bm->RBCestimation.RBCspeciesArray[guild].Start_RetInflect, no_checking, errorString, (int)bm->RBCestimation.RBCspeciesParam[guild][NumFisheries_id]);
	}
    
    attributeGroup = getParentNode(bm, fileName, childGroupingNode, "Start_RetSlope", errorString, sizeof(errorString));
	for (guild = 0; guild < bm->K_num_tot_sp; guild++) {
        if (FunctGroupArray[guild].speciesParams[assess_flag_id] > 0)
            readGroupAssessTierXMLData(bm, fileName, attributeGroup, guild, bm->RBCestimation.RBCspeciesArray[guild].Start_RetSlope, no_checking, errorString, (int)bm->RBCestimation.RBCspeciesParam[guild][NumFisheries_id]);
	}
 
    childGroupingNode = Util_XML_Get_Node(ATLANTIS_ATTRIBUTE_SUB_GROUP, groupingNode, "Varadj");
	if (childGroupingNode == NULL)
		quit("readTierAssessmentManagementXML: Varadj attribute group not found in input file %s.\n", fileName);
    
    attributeGroup = getParentNode(bm, fileName, childGroupingNode, "Varadj_CPUE", errorString, sizeof(errorString));
	for (guild = 0; guild < bm->K_num_tot_sp; guild++) {
        if (FunctGroupArray[guild].speciesParams[assess_flag_id] > 0)
            readGroupAssessTierXMLData(bm, fileName, attributeGroup, guild, bm->RBCestimation.RBCspeciesArray[guild].Varadj_CPUE, no_checking, errorString, (int)bm->RBCestimation.RBCspeciesParam[guild][NumFisheries_id]);
	}
 
    attributeGroup = getParentNode(bm, fileName, childGroupingNode, "Varadj_discard", errorString, sizeof(errorString));
	for (guild = 0; guild < bm->K_num_tot_sp; guild++) {
        if (FunctGroupArray[guild].speciesParams[assess_flag_id] > 0)
            readGroupAssessTierXMLData(bm, fileName, attributeGroup, guild, bm->RBCestimation.RBCspeciesArray[guild].Varadj_discard, no_checking, errorString, (int)bm->RBCestimation.RBCspeciesParam[guild][NumFisheries_id]);
	}
 
    attributeGroup = getParentNode(bm, fileName, childGroupingNode, "Varadj_length", errorString, sizeof(errorString));
	for (guild = 0; guild < bm->K_num_tot_sp; guild++) {
        if (FunctGroupArray[guild].speciesParams[assess_flag_id] > 0)
            readGroupAssessTierXMLData(bm, fileName, attributeGroup, guild, bm->RBCestimation.RBCspeciesArray[guild].Varadj_length, no_checking, errorString,
            		(int)bm->RBCestimation.RBCspeciesParam[guild][NumFisheries_id]);
	}
    
    attributeGroup = getParentNode(bm, fileName, childGroupingNode, "Varadj_age", errorString, sizeof(errorString));
	for (guild = 0; guild < bm->K_num_tot_sp; guild++) {
        if (FunctGroupArray[guild].speciesParams[assess_flag_id] > 0)
            readGroupAssessTierXMLData(bm, fileName, attributeGroup, guild, bm->RBCestimation.RBCspeciesArray[guild].Varadj_age, no_checking, errorString, (int)bm->RBCestimation.RBCspeciesParam[guild][NumFisheries_id]);
	}
    
	/** assessment specifications */
    childGroupingNode = Util_XML_Get_Node(ATLANTIS_ATTRIBUTE_SUB_GROUP, groupingNode, "GenSS");
	if (childGroupingNode == NULL)
		quit("readTierAssessmentManagementXML: GenSS attribute group not found in input file %s.\n", fileName);
    
	Util_XML_Read_Species_RBCParam(bm, fileName, childGroupingNode, DiscType_id);
	Util_XML_Read_Species_RBCParam(bm, fileName, childGroupingNode, MaxH_id);
	Util_XML_Read_Species_RBCParam(bm, fileName, childGroupingNode, Growthage_L1_id);
	Util_XML_Read_Species_RBCParam(bm, fileName, childGroupingNode, Growthage_L2_id);
	Util_XML_Read_Species_RBCParam(bm, fileName, childGroupingNode, MinCatch_id);
    Util_XML_Read_Species_RBCParam(bm, fileName, childGroupingNode, AssessStart_id);
    
	/** Other paremeters  */
	Util_XML_Read_Species_RBCParam(bm, fileName, childGroupingNode, MG_offset_id);
	Util_XML_Read_Species_RBCParam(bm, fileName, childGroupingNode, Regime_shift_assess_id);
	Util_XML_Read_Species_RBCParam(bm, fileName, childGroupingNode, RecDevBack_id);
    
	Util_XML_Read_Species_RBCParam(bm, fileName, childGroupingNode, Hsteep_id);
	Util_XML_Read_Species_RBCParam(bm, fileName, childGroupingNode, Agesel_Pattern_id);
	Util_XML_Read_Species_RBCParam(bm, fileName, childGroupingNode, AssessFreq_id);
    Util_XML_Read_Species_RBCParam(bm, fileName, childGroupingNode, BallParkF_id);
    Util_XML_Read_Species_RBCParam(bm, fileName, childGroupingNode, BallParkYr_id);
	Util_XML_Read_Species_RBCParam(bm, fileName, childGroupingNode, NumChangeLambda_id);
	Util_XML_Read_Species_RBCParam(bm, fileName, childGroupingNode, num_enviro_obs_id);
	Util_XML_Read_Species_RBCParam(bm, fileName, childGroupingNode, num_growth_morphs_id);
    
	// size parameters
	Util_XML_Read_Species_RBCParam(bm, fileName, childGroupingNode, Nsex_samp_id);
	Util_XML_Read_Species_RBCParam(bm, fileName, childGroupingNode, Nyfuture_id);
	Util_XML_Read_Species_RBCParam(bm, fileName, childGroupingNode, Lbin_id);    
    Util_XML_Read_Species_RBCParam(bm, fileName, childGroupingNode, thresh_mat_id);
    Util_XML_Read_Species_RBCParam(bm, fileName, childGroupingNode, femsexratio_id);
    
	// SS3 Biological parameters
	Util_XML_Read_Species_RBCParam(bm, fileName, childGroupingNode, flagLAdirect_id);
	//Util_XML_Read_Species_RBCParam(bm, fileName, childGroupingNode, flagSLAdirect_id);
	Util_XML_Read_Species_RBCParam(bm, fileName, childGroupingNode, flagWAdirect_id);

	// recruitment parameters
	Util_XML_Read_Species_RBCParam(bm, fileName, childGroupingNode, SigmaR1_id);
	Util_XML_Read_Species_RBCParam(bm, fileName, childGroupingNode, SigmaR2_id);
	Util_XML_Read_Species_RBCParam(bm, fileName, childGroupingNode, SigmaR_future_id);
	Util_XML_Read_Species_RBCParam(bm, fileName, childGroupingNode, PSigmaR1_id);
	Util_XML_Read_Species_RBCParam(bm, fileName, childGroupingNode, Regime_year_id);
	Util_XML_Read_Species_RBCParam(bm, fileName, childGroupingNode, RecDevMinYr_id);
	Util_XML_Read_Species_RBCParam(bm, fileName, childGroupingNode, RecDevMaxYr_id);
	Util_XML_Read_Species_RBCParam(bm, fileName, childGroupingNode, RecDevFlag_id);
	Util_XML_Read_Species_RBCParam(bm, fileName, childGroupingNode, AutoCorRecDev_id);
    
	/** tier specific rules */
    childGroupingNode = Util_XML_Get_Node(ATLANTIS_ATTRIBUTE_SUB_GROUP, groupingNode, "Tier1");
	if (childGroupingNode == NULL)
		quit("readTierAssessmentManagementXML: Tier1 attribute group not found in input file %s.\n", fileName);

	Util_XML_Read_Species_RBCParam(bm, fileName, childGroupingNode, tiertype_id);
	Util_XML_Read_Species_RBCParam(bm, fileName, childGroupingNode, T1_steep_phase_id);
    
	// tier 3
    childGroupingNode = Util_XML_Get_Node(ATLANTIS_ATTRIBUTE_SUB_GROUP, groupingNode, "Tier3");
	if (childGroupingNode == NULL)
		quit("readTierAssessmentManagementXML: Tier3 attribute group not found in input file %s.\n", fileName);
    
	Util_XML_Read_Species_RBCParam(bm, fileName, childGroupingNode, Tier3_Fcalc_id);
	Util_XML_Read_Species_RBCParam(bm, fileName, childGroupingNode, Tier3_time_id);
	Util_XML_Read_Species_RBCParam(bm, fileName, childGroupingNode, Tier3_M_id);
	Util_XML_Read_Species_RBCParam(bm, fileName, childGroupingNode, Tier3_S25_id);
	Util_XML_Read_Species_RBCParam(bm, fileName, childGroupingNode, Tier3_S50_id);
	Util_XML_Read_Species_RBCParam(bm, fileName, childGroupingNode, Tier3_F_id);
	Util_XML_Read_Species_RBCParam(bm, fileName, childGroupingNode, Tier3_h_id);
	Util_XML_Read_Species_RBCParam(bm, fileName, childGroupingNode, Tier3_matlen_id);
	Util_XML_Read_Species_RBCParam(bm, fileName, childGroupingNode, Tier3_maxF_id);

	attributeGroup = getParentNode(bm, fileName, childGroupingNode, "Tier3_Linf", errorString, sizeof(errorString));
	for (guild = 0; guild < bm->K_num_tot_sp; guild++) {
        if (FunctGroupArray[guild].speciesParams[assess_flag_id] > 0)
            readGroupAssessTierXMLData(bm, fileName, attributeGroup, guild, bm->RBCestimation.RBCspeciesArray[guild].Tier3_Linf, no_checking, errorString, bm->K_num_sexes);
	}

	attributeGroup = getParentNode(bm, fileName, childGroupingNode, "Tier3_k", errorString, sizeof(errorString));
	for (guild = 0; guild < bm->K_num_tot_sp; guild++) {
        if (FunctGroupArray[guild].speciesParams[assess_flag_id] > 0)
            readGroupAssessTierXMLData(bm, fileName, attributeGroup, guild, bm->RBCestimation.RBCspeciesArray[guild].Tier3_k, no_checking, errorString, bm->K_num_sexes);
	}

	attributeGroup = getParentNode(bm, fileName, childGroupingNode, "Tier3_t0", errorString, sizeof(errorString));
	for (guild = 0; guild < bm->K_num_tot_sp; guild++) {
        if (FunctGroupArray[guild].speciesParams[assess_flag_id] > 0)
            readGroupAssessTierXMLData(bm, fileName, attributeGroup, guild, bm->RBCestimation.RBCspeciesArray[guild].Tier3_t0, no_checking, errorString, bm->K_num_sexes);
	}

	// tier 4
    childGroupingNode = Util_XML_Get_Node(ATLANTIS_ATTRIBUTE_SUB_GROUP, groupingNode, "Tier4");
	if (childGroupingNode == NULL)
		quit("readTierAssessmentManagementXML: Tier4 attribute group not found in input file %s.\n", fileName);
    
	Util_XML_Read_Species_RBCParam(bm, fileName, childGroupingNode, Tier4_avtime_id);
	Util_XML_Read_Species_RBCParam(bm, fileName, childGroupingNode, Tier4_CPUEyrmin_id);
	Util_XML_Read_Species_RBCParam(bm, fileName, childGroupingNode, Tier4_CPUEyrmax_id);
	Util_XML_Read_Species_RBCParam(bm, fileName, childGroupingNode, Tier4_m_id);
	Util_XML_Read_Species_RBCParam(bm, fileName, childGroupingNode, Tier4_alpha_id);
    Util_XML_Read_Species_RBCParam(bm, fileName, childGroupingNode, Tier4_Cmaxmult_id);
    Util_XML_Read_Species_RBCParam(bm, fileName, childGroupingNode, Tier4_Bo_correct_id);
    
    // tier 4 reference levels
	Util_XML_Read_Species_RBCParam(bm, fileName, childGroupingNode, Maturity_Inflect_id);
	Util_XML_Read_Species_RBCParam(bm, fileName, childGroupingNode, Maturity_Slope_id);
	Util_XML_Read_Species_RBCParam(bm, fileName, childGroupingNode, PostRule_id);
	Util_XML_Read_Species_RBCParam(bm, fileName, childGroupingNode, CPUEmult_id);
	Util_XML_Read_Species_RBCParam(bm, fileName, childGroupingNode, MaxChange_id);


	//tier 5
    childGroupingNode = Util_XML_Get_Node(ATLANTIS_ATTRIBUTE_SUB_GROUP, groupingNode, "Tier5");
	if (childGroupingNode == NULL)
		quit("readTierAssessmentManagementXML: Tier5 attribute group not found in input file %s.\n", fileName);

    Util_XML_Read_Species_RBCParam(bm, fileName, childGroupingNode, Tier5_length_id);
	Util_XML_Read_Species_RBCParam(bm, fileName, childGroupingNode, Tier5_S50_id);
	Util_XML_Read_Species_RBCParam(bm, fileName, childGroupingNode, Tier5_cv_id);
	Util_XML_Read_Species_RBCParam(bm, fileName, childGroupingNode, Tier5_flt_id);
	Util_XML_Read_Species_RBCParam(bm, fileName, childGroupingNode, Tier5_reg_id);
	Util_XML_Read_Species_RBCParam(bm, fileName, childGroupingNode, Tier5_p_id);
	Util_XML_Read_Species_RBCParam(bm, fileName, childGroupingNode, Tier5sel_id);
	Util_XML_Read_Species_RBCParam(bm, fileName, childGroupingNode, Tier5q_id);
    
	attributeGroup = getParentNode(bm, fileName, childGroupingNode, "Tier5_Bstart", errorString, sizeof(errorString));
	for (guild = 0; guild < bm->K_num_tot_sp; guild++) {
        if (FunctGroupArray[guild].speciesParams[assess_flag_id] > 0)
            readGroupAssessTierXMLData(bm, fileName, attributeGroup, guild, bm->RBCestimation.RBCspeciesArray[guild].Tier5_Bstart, no_checking, errorString, 3);
	}

	attributeGroup = getParentNode(bm, fileName, childGroupingNode, "Tier5_r", errorString, sizeof(errorString));
	for (guild = 0; guild < bm->K_num_tot_sp; guild++) {
        if (FunctGroupArray[guild].speciesParams[assess_flag_id] > 0)
            readGroupAssessTierXMLData(bm, fileName, attributeGroup, guild, bm->RBCestimation.RBCspeciesArray[guild].Tier5_r, no_checking, errorString, 3);
	}

	/** age related **/
    attributeGroup = getParentNode(bm, fileName, groupingNode, "Ageing_error", errorString, sizeof(errorString));
	for (guild = 0; guild < bm->K_num_tot_sp; guild++) {
        if (FunctGroupArray[guild].speciesParams[assess_flag_id] > 0)
            readGroupAssessTierXMLData(bm, fileName, attributeGroup, guild, bm->RBCestimation.RBCspeciesArray[guild].Ageing_error, no_checking, errorString, (int)bm->RBCestimation.RBCspeciesParam[guild][AccumAge_id]);
	}
    
    dummy = Util_Alloc_Init_1D_Double(bm->nbox, 0.0);
    attributeGroup = getParentNode(bm, fileName, groupingNode, "RegStructure", errorString, sizeof(errorString));
	for (guild = 0; guild < bm->K_num_tot_sp; guild++) {
        if (FunctGroupArray[guild].speciesParams[assess_flag_id] > 0){
            readGroupAssessTierXMLData(bm, fileName, attributeGroup, guild, dummy, no_checking, errorString, bm->nbox);

            for (b=0; b < bm->nbox; b++) {
                bm->RBCestimation.RBCspeciesArray[guild].regID[b] = (int)dummy[b];
            }
        }
    }
    free1d(dummy);

    /** Life history related - need for SS parameterisation **/
    // by sex, stock and age
	attributeGroup = getParentNode(bm, fileName, groupingNode, "Mzero", errorString, sizeof(errorString));
    for (guild = 0; guild < bm->K_num_tot_sp; guild++) {
        if (FunctGroupArray[guild].speciesParams[assess_flag_id] > 0)
            readStockAgeAssessTierXMLData(bm, fileName, attributeGroup, guild, bm->RBCestimation.RBCspeciesArray[guild].Mzero, no_checking, errorString, (int)bm->RBCestimation.RBCspeciesParam[guild][MaxAge_id]);
    }
    
	attributeGroup = getParentNode(bm, fileName, groupingNode, "MeanLenAge", errorString, sizeof(errorString));
    for (guild = 0; guild < bm->K_num_tot_sp; guild++) {
         if (FunctGroupArray[guild].speciesParams[assess_flag_id] > 0)
             if (!bm->RBCestimation.RBCspeciesParam[guild][flagLAdirect_id])
                 readStockAgeAssessTierXMLData(bm, fileName, attributeGroup, guild, bm->RBCestimation.RBCspeciesArray[guild].MeanLenAge, no_checking, errorString, (int)bm->RBCestimation.RBCspeciesParam[guild][MaxAge_id]);
    }
    
    /*
	attributeGroup = getParentNode(bm, fileName, groupingNode, "SigmaLenAge", errorString, sizeof(errorString));
    for (guild = 0; guild < bm->K_num_tot_sp; guild++) {
        if (FunctGroupArray[guild].speciesParams[assess_flag_id] > 0)
            if (bm->RBCestimation.RBCspeciesParam[guild][flagSLAdirect_id] > 0)
                readStockAgeAssessTierXMLData(bm, fileName, attributeGroup, guild, bm->RBCestimation.RBCspeciesArray[guild].SigmaLenAge, no_checking, errorString, bm->RBCestimation.RBCspeciesParam[guild][MaxAge_id]);
    }
    */
    
    // by sex and stock
	attributeGroup = getParentNode(bm, fileName, groupingNode, "CvLA0", errorString, sizeof(errorString));
    for (guild = 0; guild < bm->K_num_tot_sp; guild++) {
        if (FunctGroupArray[guild].speciesParams[assess_flag_id] > 0)
            readStockAssessTierXMLData(bm, fileName, attributeGroup, guild, bm->RBCestimation.RBCspeciesArray[guild].CvLA0, no_checking, errorString, bm->K_num_sexes);
    }

	attributeGroup = getParentNode(bm, fileName, groupingNode, "CvLAmax", errorString, sizeof(errorString));
    for (guild = 0; guild < bm->K_num_tot_sp; guild++) {
        if (FunctGroupArray[guild].speciesParams[assess_flag_id] > 0)
            readStockAssessTierXMLData(bm, fileName, attributeGroup, guild, bm->RBCestimation.RBCspeciesArray[guild].CvLAmax, no_checking, errorString, bm->K_num_sexes);
    }

    attributeGroup = getParentNode(bm, fileName, groupingNode, "Wtlen_a", errorString, sizeof(errorString));
    for (guild = 0; guild < bm->K_num_tot_sp; guild++) {
        if (FunctGroupArray[guild].speciesParams[assess_flag_id] > 0)
            readStockAssessTierXMLData(bm, fileName, attributeGroup, guild, bm->RBCestimation.RBCspeciesArray[guild].Wtlen_a, no_checking, errorString, bm->K_num_sexes);
    }

	attributeGroup = getParentNode(bm, fileName, groupingNode, "Wtlen_b", errorString, sizeof(errorString));
    for (guild = 0; guild < bm->K_num_tot_sp; guild++) {
        if (FunctGroupArray[guild].speciesParams[assess_flag_id] > 0)
            readStockAssessTierXMLData(bm, fileName, attributeGroup, guild, bm->RBCestimation.RBCspeciesArray[guild].Wtlen_b, no_checking, errorString, bm->K_num_sexes);
    }

	attributeGroup = getParentNode(bm, fileName, groupingNode, "VBLinf", errorString, sizeof(errorString));
    for (guild = 0; guild < bm->K_num_tot_sp; guild++) {
        if (FunctGroupArray[guild].speciesParams[assess_flag_id] > 0)
            readStockAssessTierXMLData(bm, fileName, attributeGroup, guild, bm->RBCestimation.RBCspeciesArray[guild].VBLinf, no_checking, errorString, bm->K_num_sexes);
    }

    attributeGroup = getParentNode(bm, fileName, groupingNode, "VBk", errorString, sizeof(errorString));
    for (guild = 0; guild < bm->K_num_tot_sp; guild++) {
        if (FunctGroupArray[guild].speciesParams[assess_flag_id] > 0)
            readStockAssessTierXMLData(bm, fileName, attributeGroup, guild, bm->RBCestimation.RBCspeciesArray[guild].VBk, no_checking, errorString, bm->K_num_sexes);
    }

    attributeGroup = getParentNode(bm, fileName, groupingNode, "VBt0", errorString, sizeof(errorString));
    for (guild = 0; guild < bm->K_num_tot_sp; guild++) {
        if (FunctGroupArray[guild].speciesParams[assess_flag_id] > 0)
            readStockAssessTierXMLData(bm, fileName, attributeGroup, guild, bm->RBCestimation.RBCspeciesArray[guild].VBt0, no_checking, errorString, bm->K_num_sexes);
    }

    attributeGroup = getParentNode(bm, fileName, groupingNode, "Fecund", errorString, sizeof(errorString));
    for (guild = 0; guild < bm->K_num_tot_sp; guild++) {
       if (FunctGroupArray[guild].speciesParams[assess_flag_id] > 0)
            readStockAssessTierXMLData(bm, fileName, attributeGroup, guild, bm->RBCestimation.RBCspeciesArray[guild].Fecund, no_checking, errorString, (int)bm->RBCestimation.RBCspeciesParam[guild][MaxAge_id]);
    }
    
    attributeGroup = getParentNode(bm, fileName, groupingNode, "SSMort", errorString, sizeof(errorString));
	for (guild = 0; guild < bm->K_num_tot_sp; guild++) {
        if (FunctGroupArray[guild].speciesParams[assess_flag_id] > 0)
            readStockAssessTierXMLData(bm, fileName, attributeGroup, guild, bm->RBCestimation.RBCspeciesArray[guild].SSMort, no_checking, errorString, bm->K_num_sexes);

	}
    
    // Buffered tiers
    childGroupingNode = Util_XML_Get_Node(ATLANTIS_ATTRIBUTE_SUB_GROUP, groupingNode, "TIERsigma");
	if (childGroupingNode == NULL)
		quit("readTierAssessmentManagementXML: US sigma attribute group not found in input file %s.\n", fileName);
    Util_XML_Read_Species_RBCParam(bm, fileName, childGroupingNode, Tier1Sig_id);
    Util_XML_Read_Species_RBCParam(bm, fileName, childGroupingNode, Tier2Sig_id);
    Util_XML_Read_Species_RBCParam(bm, fileName, childGroupingNode, Tier3Sig_id);
    
    Util_XML_Read_Species_RBCParam(bm, fileName, childGroupingNode, isTriggerSpecies_id);
    Util_XML_Read_Species_RBCParam(bm, fileName, childGroupingNode, UseRBCAveraging_id);
    Util_XML_Read_Species_RBCParam(bm, fileName, childGroupingNode, trigger_threshold_id);


}
