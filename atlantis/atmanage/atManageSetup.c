/**
 \file atManageSetup.c
 \brief C file for running the setup of the management library.
 \ingroup atManageLib
 \author Beth Fulton 8/8/2004


 Based on work by Ouyang and Fulton for Port Phillip Bay

 Note:
 1. Some names of variables or subroutines are self-explaining.
 2. In the names or comments, WC means Water Column, Sm Sediment.

 Revisions: 10/8/2004 Created from the msebiolsetup.c file

 17/8/2004 Moved all the management and effort arrays and parameters here
 except for FISHERY_prms which is created and initialised in
 atlantismain.c

 21/9/2004 Renamed FSL, FSO, FSF, FST and FSG to generic names so apply to
 other models/generic case.
 FSL -> FDC  (Ling -> Demersal_DC_Fish)
 FSO -> FDO  (Orange_Roughy -> Demersal_O_Fish)
 FSF -> FDB  (Flathead -> Demersal_B_Fish)
 FST -> FVO  (Bluefin_Tuna -> Pisciv_O_Fish)
 FSG -> SHB  (Gummy_Shark -> Shark_B)

 4/10/2004 Added parameter read-ins and array initalisation for
 new fisheries (ptrawlPWN, dtrawlFBP, midwcZL, trapFDE,
 dlineFDE, netFDE) and biological groups (BML, BFF, ZL, PWN,
 FBP, FDE, FDF, FVB, SSK, SHR, SP, REP, WDG)

 23/10/2004 Added midwcPWN and additional fisheries links

 18/1/2005 Added bm->FisheriesID and moved all sp_fisheries to nfisheries
 in readModelFCParameter() and modified initManage() tomatch for
 the parameter arrays where reading in species specific parameters
 foreach fishery (so now read in an entry for every fishery and then
 use bm->FisheriesID tomap to valid fisheries). Also modfified age specific
 selectivity to match (so read in adult and juvenile arrays for
 this parameter and then match with correct entry in overall parameter
 array).

 19/1/2005 Added Microphytobenthos (MB)

 8/6/2005 Added mowMA and moved target list from setDemPelFCID() to
 readFCModelParameter(). Also added readFCTargetValue() to read
 in strings of target species ids and added the 3 groups FDM, FDP, WHS.

 15/8/2005 Updated SPtoCATid to include all species now accessible by fisheries
 (either as target or bycatch)

 16/8/2005 Replaced numbers in readFCValue calls with read-in type id numbers (no_checking,
 binary_check, proportion_check, probability_check, integer_check). Also put
 in default case for all switch statements

 24/8/2005 Added basket quotas and started adding multiple stock structure and
 RegionalData so can have regional management, such as regional TACs. Also
 modified how MPAs read in so can have unlimioted number of rotational
 and changing MPAs. Readin via Setup_MPA_Lists()

 1/9/2005 Finished adding multiple stock structure (horizontal and vertical)

 27/9/2005 Added catch and effort time series usage

 17/8/2006 Fixed bug with Fcocatch array where reading it in as int not double
 so always effectively set to zero (in effect)

 5/9/2006 Removed limitation on fisheries per group, so did away
 with references to bm->FisheriesID and Fisheries_Indices. Kept
 as legacy code for now, but will ultimately be done away with.

 14/02/2008 - Bec Gorton

 Started moving the array loading code into functions. Following the standard format
 of using regular expressions for arrays.
 The manage code is being changed to use regular expressions to match the keys in the
 input files. The keys are also added to a hash table.
 When a key is matched to a regular expression it is also found in the has table.
 Each key is stored with a flag that is set to true when it is found in the input file.
 After the entire file is read the array of keys is checked to make sure they were all
 read in. If not an error message is printed and the program quits.

 The regular expressions and their associated functions are in atmanageRegEx.c.
 The hash table is generated in atmanageHashTable.c

 14/04/2008 Beth Fulton
 Added Kreg check so RegionalData array doesn't overflow if lots of regions.
 Also corrected bim to bm->BiM.

 26-05-2008 Bec Gorton
 Changed all references from bm->VERTind to the tracer arrays associated with
 each functional group.

 4/06/2008 Beth Fulton
 Removed Fisheries_Indices and FisheriesID as redundant in the new fully flexible structure

 30/09/2008 Bec Gorton
 Removed the bm->sp_Prms array. These values are now stored in the speciesParams
 and cohortSpeciesParams arrays in the FunctGroupArray structure.

 6/12/2008 Beth Fulton
 Added tEnd_id and tStart_id so can have start and end date for dynamic fisheries

 22-04-2009 Bec Gorton
 Removed the  bm->SPtoGUILD and bm->GUILDtoSP arrays. Also fixed some memory freeing
 code that was not freeing the memory arrays allocated when the management module is
 not turned on.

 16/6/2009 Beth Fulton
 Replaced SP_of_Concern[sp] with FunctGroupArray[sp].speciesParams[sp_concern_id]

 29-06-2009 Bec Gorton
 Moved the setupFisheryPrmStrings function into the new atHarvest module. These values
 are need when reading in the harvest input parameters.

 12-08-2009 Bec Gorton
 Change the
 origEffort_vdistrib = Util_Alloc_Init_2D_Double ( nfleets, bm->wcnz, 0.0 );
 to:
 origEffort_vdistrib = Util_Alloc_Init_2D_Double ( bm->wcnz, nfleets,  0.0 );
 so that i could use generic loading functions.

 28-10-2009 Bec Gorton
 Started removing references to K_num_fished_sp and K_num_impacted_sp.
 Change references to arrays that have been moved into a module structure.
 Change the input parsing code to use the new XML parsing code.

 02-11-2009 Bec Gorton
 Removed additional references to K_num_vert_sp, K_num_impacted_sp and K_num_fished_sp.

 03-11-2009 Bec Gorton
 Removed the readFCTargetValue() function and the declaration of the bm->SPtoCATid array.
 Also removed the readFCValue function.

 11/11/2009 Beth Fulton
 Added start_manage_id and end_manage_id so can have more flexible within run management options

 11/11/2009 Beth Fulton
 Added flagADJUSTmanage_id so can tie all forms of management (gear, spatial, effort, TAC, seasons)
 to stock size

 25-01-2010 Bec Gorton
 Got rid of the Not_fished_id definition. The basket_sp and companion species arrays will now be padded with -1
 values instead of Not_fished_id. The FunctGroupArray[sp].isFished value should also be checked to see if the companion/basket
 species is actually fished.

 29-01-2010 Bec Gorton
 Removed the bm->fisheryNAME - now using the fisheryCode in the fisheryArray.
 Renamed functions to comply with the coding standard.

 26-04-2010 Bec Gorton
 Fixed a bug in the code that reads in the Effort time series. The code was actually using the Effort
 time series.

 7-05-2010 Bec Gorton
 Deleted the setSPid function and the bm_>SPid array.

 05-05-2010 Bec Gorton
 Added code to initialise the new mFC_scale_id species fishery param. This is used to scale the mFC value
 when the broken stick management strategy is in use. The default value is 1.0 so it shouldn't have any effect
 when this is not turned on.

 25-05-2010 Beth Fulton
 Corrected  bm->SP_prms[b][spconcern_id] read-in as was point to [sp] not [b] originally

 01-02-2011 Beth Fulton
 Put in warning if no residuals for market (it used to cause a fail in econ code, but now just assumes
 zero residuals so warning needed here)

 03-10-2011 Beth Fulton
 Added tac_resetperiod_id read-in and initialised, plus added TotOldCumCatch[sp][nf] and TotOldCumDiscards[sp][nf] to deal with multiyear TAC

 11-10-2011 Beth Fulton
 Added a check whether bulkTAC and if it is (i.e. multiyear spanning TAC) then apply a multiplication of
 TACamt * tac_resetperiod_id length so that annual size TAC read-in converted to multi-year TAC



 ************************************************************************************

 Table 1. Summary of Model State Variables.

 Description	Symbol					Units
 ------------------              ------------
 Large phytoplankton				PL	mg N m-3
 Small phytoplankton	(pico)		PS	mg N m-3
 Dinoflagellates					DF	mg N m-3
 Seagrass						SG	mg N m-2
 Macroalgae						MA	mg N m-2
 Microphytobenthos				MB	mg N m-2
 Small planktivorous fish        FPS special
 Large planktivorous fish        FPL special
 Other planktivorous fish        FPO special
 Deep piscivorous fish           FVD special
 Vulnerable piscivorous fish     FVV special
 Shallow piscivorous fish        FVS special
 Other piscivorous fish          FVB special
 Large piscivorous fish (tuna)   FVT special
 Other tuna                      FVO special
 Migratory mesopelagics fish     FMM special
 Non-migratory mesopelagics fish FMN special
 Benthopelagics                  FBP special
 Herbivorous demersal fish       FDE special
 Shallow demersal fish           FDS special
 Other shallow demersal fish     FDB special
 Deep demersal fish              FDD special
 Other deep demersal fish        FDC special
 Longlived deep demersal fish    FDO special
 Flat deep demersal fish         FDF special
 Protected demersal fish         FDP special
 Miscellaneous demersal fish     FDM special
 Demersal sharks                 SHD special
 Dogfish                         SHC special
 Other demersal sharks           SHB special
 Pelagic sharks                  SHP special
 Reef sharks                     SHR special
 Skates and rays                 SSK special
 Seabirds                        SB  special
 Penguins                        SP  special
 Reptiles						REP special
 Pinnipeds                       PIN special
 Baleen whales                   WHB special
 Small toothed whales            WHS special
 Toothed whales                  WHT special
 Dugongs                         WDG special
 Prawns                          PWN mg N m-3
 Cephalopods                     CEP mg N m-3
 Gelatinous zooplankton			ZG	mg N m-3
 Large carnivorous zooplankton	ZL	mg N m-3
 Mesozooplankton					ZM	mg N m-3
 Small zooplankton				ZS	mg N m-3
 Pelagic associated bacteria		PB	mg N m-2
 Sedimentary bacteria			BB	mg N m-2
 Meiobenthos						BO	mg N m-2
 Deposit feeders					BD	mg N m-2
 Benthic infaunal carnivores		BC	mg N m-2
 Benthic grazers					BG	mg N m-2
 Benthic filter feeders shallow	BFS	mg N m-2
 Benthic filter feeders deep		BFD	mg N m-2
 Other benthic filter feeders	BFF	mg N m-2
 Macrozoobenthos	shallow			BMS	mg N m-2
 Macrozoobenthos	deep			BMD	mg N m-2
 Megazoobenthos      			BML	mg N m-2
 Labile detritus					DL	mg N m-3
 Refractory detritus				DR	mg N m-3
 Carrion							DC	mg N m-3
 Dissolved organic nitrogen		DON	mg N m-3
 Ammonia							NH	mg N m-3
 Nitrate							NO	mg N m-3
 Dissolved silica				Si	mg Si m-3
 Biogenic silica					DSi	mg Si m-3
 Dissolved oxygen				O2	mg O m-3
 Light							IRR	W m-2

 Those marked with a + also have Si pool too.
 Those marked special have three pools, a sructural N (mg N individual-1),
 reserve N (mg N individual-1) and numbers (per box)
 *************/

/*  Open library routines */
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>
#include "atManage.h"
#include "atManagePrivate.h"
#include <convertXML.h>
#include "atAssessLib.h"
#include "atHarvestLib.h"  // so can access error and cdf distribution types
#include "atSS3LinkLib.h"

static int Init_Manage_Flags(MSEBoxModel *bm, FILE *llogfp);
static void Allocate_Arrays_Pre_Load(MSEBoxModel *bm);
static void Allocate_Arrays_Post_Load(MSEBoxModel *bm);

static void Calculate_Effort_Depth(MSEBoxModel *bm);
static void Calculate_Flag_Impose(MSEBoxModel *bm);
static int Is_Council_Needed(MSEBoxModel *bm, FILE *llogfp);
static void Calculate_BiTAC(MSEBoxModel *bm, FILE *llogfp);
/********************************************************************//**
 * Initialisation.
 *
 * This routine is to initialise management global data,
 * and is called by main() in transport module.
 *
 * The routine Manage_Init() is called once at the beginning of
 * the run. It carries out the following steps.
 *
 * 1. Read harvest and management parameter files
 *
 * 2. It allocates space for management, catch and effort vectors used in the
 * harvest and management models.
 *
 */
void Manage_Init(MSEBoxModel *bm, FILE *llogfp) {

	char fisheriesfile[120];
	char convertedXMLFileName[STRLEN];

	double totTAC = 0;
    double max_F;
	int fishery_id, guild, flagimposeeffort, sp, b, i, flagrecruit_sp, nf, count_it, ij, flagF, flagfcmpa,
			 flag_sp, flagspeffortmodel, bim, in_quota, no_mEff, has_effort, has_F, need_LeverQueue, num_years, tier;
	int nfleets = bm->K_num_fisheries;
	//int flagimposediscard;
	bm->flagimposeglobal = 0;

	printf("Initialise management\n");

//	WriteSSFiles(bm, 2, 2008, bm->logFile);


//	/* Init arrays */
//	Allocate_Arrays_Pre_Load(bm);
//
//	/* Setup the management performance names */
//	Set_Manage_Index_Names(bm);
//
//	/* Open the management performance output file */
//	Open_Management_Index_File(bm);
//
//	Open_Manage_Output_Files(bm);

	if (!bm->flag_fisheries_on) {
		/* If not using fishing don't bother with any of the parameters, set the ID match and leave */
		printf("Skipping fisheries as not active\n");
		/* Create basket quota management arrays */
		bm->sp_basket = Util_Alloc_Init_2D_Int(1, bm->K_num_basket, bm->K_num_tot_sp);
		bm->TAC_over = Util_Alloc_Init_3D_Int(1, bm->K_num_fisheries, bm->K_num_tot_sp, 0);
        bm->rolling_cap_initialised = Util_Alloc_Init_2D_Int((bm->K_num_max_cohort * bm->K_num_max_genetypes), bm->K_num_tot_sp, 0);
		return;
	}

	/* Init arrays */
	Allocate_Arrays_Pre_Load(bm);

	/* Setup the management performance names */
	Set_Manage_Index_Names(bm);

	/* Open the management performance output file */
	Open_Management_Index_File(bm);

	Open_Manage_Output_Files(bm);

	/* Build global vectors ******************************************************************
	 Using sjwlib routines for doubles and ints but spell it out in standard c for Name case */
	strcpy(fisheriesfile, bm->fishprmIfname);

	/* Build the converted filename */
	sprintf(convertedXMLFileName, "%s", bm->ncOfname);
	*(strstr(convertedXMLFileName, ".nc")) = '\0';
	strcat(convertedXMLFileName, "_management.xml");

	/* Convert the input file to XML */
	/* Read fisheries parameters ***************************************************/
	printf("Start reading fisheries parameters from %s.\n", convertedXMLFileName);
	Convert_Management_To_XML(bm, bm->fishprmIfname, convertedXMLFileName);

	Read_Manage_Paramaters(bm, convertedXMLFileName);
	Init_Manage_Flags(bm, llogfp);

	/* Allocate the arrays where the size is based on values read in from input file */
	Allocate_Arrays_Post_Load(bm);

	/* Check fishing flag ok */
	if ((bm->flagincidmort || bm->flagfinfish) && !bm->flag_fisheries_on)
		bm->flag_fisheries_on = 1;

	/* Determine if using annual management based on stock or endangered groups sizes
	 so can set flag indicating whether or not to keep a tally of total stock size
	 for this groups */
	printf("Check array sizes needed\n");
	if (bm->flagendangered)
		bm->need_pops = 1;

	if (!bm->need_pops) {
		for (sp = 0; sp < bm->K_num_fisheries; sp++) {
			b = (int) (bm->FISHERYprms[sp][flagmanage_id]);
			if (b) {
				bm->need_pops = 1;
				break;
			}
		}
	}
	if (!bm->need_pops) {
		for (sp = 0; sp < bm->K_num_tot_sp; sp++) {
			if (FunctGroupArray[sp].isVertebrate == TRUE) {
				flagrecruit_sp = (int) (FunctGroupArray[sp].speciesParams[flagrecruit_id]);
				if (flagrecruit_sp == 3 || flagrecruit_sp > 5)
					bm->need_pops = 1;
			}
		}
	}

	/* Parameter check *************************************************************/
	for(nf = 0; nf < bm->K_num_fisheries; nf++){
		if(FisheryArray[nf].isRec == TRUE){
			if(bm->FISHERYprms[nf][flageffortmodel_id] == 5 || bm->FISHERYprms[nf][flageffortmodel_id] == 3) {
					bm->FISHERYprms[nf][flageffortmodel_id] = 4;
					fprintf(llogfp,
							"Effort model for recreational fishing was set to dynamic effort, this is only appropriate for commerical fleets, so effort model reset to 4\n");
					warn("Effort model for recreational fishing was set to dynamic effort, this is only appropriate for commerical fleets, so effort model reset to 4\n");
				}

		}
	}
//	if (bm->FISHERYprms[recfish_id][flageffortmodel_id] == 5 || bm->FISHERYprms[recfish_id][flageffortmodel_id] == 3) {
//		bm->FISHERYprms[recfish_id][flageffortmodel_id] = 4;
//		fprintf(llogfp,
//				"Effort model for recreational fishing was set to dynamic effort, this is only appropriate for commerical fleets, so effort model reset to 4\n");
//		warn("Effort model for recreational fishing was set to dynamic effort, this is only appropriate for commerical fleets, so effort model reset to 4\n");
//	}

	/* Determine if we will be visiting a council in US-like system*/
	bm->Council_needed = Is_Council_Needed(bm, llogfp);

	/* If size based selectivity curves used in fisheries impacting squid warn that
	 stage based constant selectivity will be used for cephalopods and prawns */
//	if (bm->FISHERYprms[midwcCEP_id][selcurve_id] > 1) {
//		fprintf(llogfp, "Manage_Init: size-based selectivity curve set for midwcCEP fishery so stage based selectivity constants adopted for CEP\n");
//		warn("Manage_Init: size-based selectivity curve set for midwcCEP fishery so stage based selectivity constants adopted for CEP\n");
//	}
//	if (bm->FISHERYprms[jigCEP_id][selcurve_id] > 1) {
//		fprintf(llogfp, "Manage_Init: size-based selectivity curve set for jigCEP fishery so stage based selectivity constants adopted for CEP\n");
//		warn("Manage_Init: size-based selectivity curve set for jigCEP fishery so stage based selectivity constants adopted for CEP\n");
//	}
//	if (bm->FISHERYprms[dtrawlCEP_id][selcurve_id] > 1) {
//		fprintf(llogfp, "Manage_Init: size-based selectivity curve set for dtrawlCEP fishery so stage based selectivity constants adopted for CEP\n");
//		warn("Manage_Init: size-based selectivity curve set for dtrawlCEP fishery so stage based selectivity constants adopted for CEP\n");
//	}
//	if (bm->FISHERYprms[dtrawlFDB_id][selcurve_id] > 1) {
//		fprintf(llogfp, "Manage_Init: size-based selectivity curve set for dtrawlFDB fishery so stage based selectivity constants adopted for CEP\n");
//		warn("Manage_Init: size-based selectivity curve set for dtrawlFDB fishery so stage based selectivity constants adopted for CEP\n");
//	}
//	if (bm->FISHERYprms[ptrawlPWN_id][selcurve_id] > 1) {
//		fprintf(llogfp, "Manage_Init: size-based selectivity curve set for ptrawlPWN fishery so stage based selectivity constants adopted for PWN\n");
//		warn("Manage_Init: size-based selectivity curve set for ptrawlPWN fishery so stage based selectivity constants adopted for PWN\n");
//	}
    
	/* initialised the effort scale array */
	for (sp = 0; sp < bm->K_num_fisheries; sp++) {
		for (i = 0; i < 2; i++) {
			effort_scale[sp][i] = 1;
			effort_scale[sp][i + 2] = 0;
		}
	}

	printf("Populate fisheries arrays\n");

	/* Populate arrays ***********************************************************/
	/* Set vertebrate array values (i.e. populate the array with values, which are
	 actual parameter values read in previously or defaults to be used if no
	 parameters exist) */

	Calculate_Effort_Depth(bm);

	/* Set up initial BiTAC and regional scalars based on BiTACReg1 and BiTACReg2 */
	Calculate_BiTAC(bm, llogfp);

	/* Calculate the global flagimpose value */
	Calculate_Flag_Impose(bm);

	/* TAC and Fisheries_Indices Check */
	if (bm->debug == debug_assess) {
		for (sp = 0; sp < bm->K_num_tot_sp; sp++) {
			if (FunctGroupArray[sp].isFished == TRUE) {
				flag_sp = (int) (FunctGroupArray[sp].speciesParams[flag_id]);
				if (flag_sp) {
					totTAC = 0;
					fprintf(llogfp, "Time: %e %s", bm->dayt, FunctGroupArray[sp].groupCode);
					for (nf = 0; nf < bm->K_num_fisheries; nf++) {
						in_quota = bm->inQuota[nf][sp];
						fprintf(llogfp, "nf = %d, in_quota = %d\n", nf, in_quota);
						if (in_quota) {
							totTAC += bm->TACamt[sp][nf][now_id];
							fprintf(llogfp, " %s=%e", FisheryArray[nf].fisheryCode, bm->TACamt[sp][nf][now_id]);
						}
					}
					if (totTAC < 0)
						totTAC = 0;
					fprintf(llogfp, " TAC = %f (%e)", totTAC, totTAC); /* No conversion necessary as already in tonnes */
					FunctGroupArray[sp].speciesParams[TACorig_id] = totTAC;
				}
				fprintf(llogfp, "\n");
			}
		}
	}

	/* Make sure old and new TACs match in the beginning */
	for (sp = 0; sp < bm->K_num_tot_sp; sp++) {
		if (FunctGroupArray[sp].isTAC) {
			for (b = 0; b < nfleets; b++) {
				bm->TACamt[sp][b][old_id] = bm->TACamt[sp][b][now_id];
				for (bim = 0; bim < 6; bim++) {
					bm->BiTACamt[bim][sp][b][old_id] = bm->BiTACamt[bim][sp][b][now_id];
				}
			}
		}
	}

	/* Set flag to say whether any catch imposed on the group and check
	 whether discarding settings make sense if imposing catch */
	for (sp = 0; sp < bm->K_num_tot_sp; sp++) {
		if (FunctGroupArray[sp].isFished == TRUE) {
			for (i = 0; i < bm->K_num_fisheries; i++) {
				/* Check if catch imposed */
				if (bm->SP_FISHERYprms[sp][i][flagimposecatch_id] > 0) {
					FunctGroupArray[sp].speciesParams[flagimpose_id] = 1;
					/* Check for sensible discarding parameters */
					if (!bm->SP_FISHERYprms[sp][i][flagdiscard_id] && bm->SP_FISHERYprms[sp][i][FFCDR_id] > 0.1) {
						warn("%s on %s has imposed catch, but also constant discarding that is > 0.1 (%e) - is that correct?\n", FisheryArray[i].fisheryCode,
								FunctGroupArray[sp].groupCode, bm->SP_FISHERYprms[sp][i][FFCDR_id]);
					}
				}
			}

			/* Identify target and limit reference points */
            switch ((int)(whichrefi[sp])) {
            case forage_secondary_tier: /* Forage (secondary) species reference points */
                FunctGroupArray[sp].speciesParams[BrefA_id] = bm->forage_refA;
                FunctGroupArray[sp].speciesParams[BrefB_id] = bm->forage_refB;
                FunctGroupArray[sp].speciesParams[BrefC_id] = bm->forage_refC;
                FunctGroupArray[sp].speciesParams[BrefD_id] = bm->forage_refD;
                FunctGroupArray[sp].speciesParams[BrefE_id] = bm->forage_refE;
                FunctGroupArray[sp].speciesParams[Blim_id] = bm->forage_lim_ref;
                break;
            case target_tier: /* Target species reference points */
				FunctGroupArray[sp].speciesParams[BrefA_id] = bm->targ_refA;
				FunctGroupArray[sp].speciesParams[BrefB_id] = bm->targ_refB;
				FunctGroupArray[sp].speciesParams[BrefC_id] = bm->targ_refC;
				FunctGroupArray[sp].speciesParams[BrefD_id] = bm->targ_refD;
				FunctGroupArray[sp].speciesParams[BrefE_id] = bm->targ_refE;
				FunctGroupArray[sp].speciesParams[Blim_id] = bm->lim_ref;
                break;
            case byproduct_tier: /* Byproduct species reference points */
                FunctGroupArray[sp].speciesParams[BrefA_id] = bm->byproduct_refA;
                FunctGroupArray[sp].speciesParams[BrefB_id] = bm->byproduct_refB;
                FunctGroupArray[sp].speciesParams[BrefC_id] = bm->byproduct_refC;
                FunctGroupArray[sp].speciesParams[BrefD_id] = bm->byproduct_refD;
                FunctGroupArray[sp].speciesParams[Blim_id] = bm->byproduct_lim_ref;
                break;
            case bycatch_tier: /* Bycatch species reference points */
                FunctGroupArray[sp].speciesParams[BrefA_id] = bm->bycatch_refA;
                FunctGroupArray[sp].speciesParams[BrefB_id] = bm->bycatch_refB;
                FunctGroupArray[sp].speciesParams[BrefC_id] = bm->bycatch_refC;
                FunctGroupArray[sp].speciesParams[BrefD_id] = bm->bycatch_refD;
                FunctGroupArray[sp].speciesParams[Blim_id] = bm->bycatch_lim_ref;
                break;
			}
		}
	}

	for (i = 0; i < nfleets; i++) {
      count_it = 0;
      bm->FISHERYprms[i][flagMFCdisplace_id] = 0;
      for (sp = 0; sp < bm->K_num_tot_sp; sp++) {
            if (FunctGroupArray[sp].isFished == TRUE) {
                  if (bm->FISHERYtarget[i][sp]) {
                        count_it++;
                        //fprintf(llogfp, "%s is target of %s\n", FunctGroupArray[sp].groupCode, FisheryArray[i].fisheryCode);
                  }
            }
            if((bm->SP_FISHERYprms[sp][i][flagF_id] > 0 ) && bm->flagdisplace) {
                  bm->FISHERYprms[i][flagMFCdisplace_id] = 1; // mFC fishery and displace effort on
            }
      }
      bm->FISHERYprms[i][ntargets_id] = count_it;
}


	for (i = 0; i < nfleets; i++) {
		for (guild = 0; guild < bm->K_num_tot_sp; guild++) {
			if (FunctGroupArray[guild].groupAgeType == AGE_STRUCTURED || FunctGroupArray[guild].groupAgeType == AGE_STRUCTURED_BIOMASS) {

				// Note using "* FunctGroupArray[sp].numGeneTypes" here as harvest code refers back to age classes only for this parameter 
				for (b = 0; b < FunctGroupArray[guild].numCohorts; b++) {
					bm->CatchTS_agedistribOrig[i][guild][b] = bm->CatchTS_agedistrib[i][guild][b];
					bm->DiscardTS_agedistribOrig[i][guild][b] = bm->DiscardTS_agedistrib[i][guild][b];
				}
			}
		}
	}
    
	/* Get discard co-occuring ids if needed */
	need_discard = 0;
	for (sp = 0; sp < bm->K_num_tot_sp; sp++) {
		if (FunctGroupArray[sp].isImpacted == TRUE) {
			for (fishery_id = 0; fishery_id < bm->K_num_fisheries; fishery_id++) {
				if (bm->SP_FISHERYprms[sp][fishery_id][flagdiscard_id] == depend_discard) {
					need_discard++;
				}
			}
		}
	}

    bm->flagdepend_dis = 0;
    if(need_discard)
    	bm->flagdepend_dis = 1;
    /*
	 Check the change effort values are valid
	 */
	if (bm->flagchangeeffort) {

		int fisheryIndex;

		for (fisheryIndex = 0; fisheryIndex < bm->K_num_fisheries; fisheryIndex++) {
			for (i = 0; i < bm->FISHERYprms[fisheryIndex][EFF_num_changes_id]; i++) {
				if (EFFORTchange[fisheryIndex][i][mult_id] < 0)
					quit("Effort_mult_%s must be >0, if not using it set it to 1.0, if want decrease make it a fraction\n", Util_Get_Fishery_Name(bm,
							fisheryIndex));
				if (EFFORTchange[fisheryIndex][i][period_id] == 0)
					quit("EffortChange_period_%s must be >0, if not using it set it to 1, if want decrease make it a fraction\n", Util_Get_Fishery_Name(bm,
							fisheryIndex));
			}
		}
	}

	/* Sanity check on port activation */
	for (i = 0; i < bm->K_num_ports; i++) {
		if (bm->Port_info[i][port_start_id] > 0)
			warn("Port %d will not start to day %e - OK if delayed start intended\n", i, bm->Port_info[i][port_start_id]);
	}

	/* Initialise fishery seasons */
	for (sp = 0; sp < bm->K_num_fisheries; sp++) {
		SEASONAL[sp][0] = (int) (bm->FISHERYprms[sp][seasonopen_id]);
		SEASONAL[sp][1] = (int) (bm->FISHERYprms[sp][seasonclose_id]);
	}

	for (b = 0; b < bm->nbox; b++) {
		/* Determine overlap with fisheries */
		Basic_Habitat_Overlap(bm, b, llogfp);
	}

	/* Initialise catch and effort timeseries - start with checking if needed */
	/*flagimposecatch = 0;
	flagimposediscard = 0;
	for (b = 0; b < bm->K_num_fisheries; b++) {
		for (sp = 0; sp < bm->K_num_tot_sp; sp++) {
			if (bm->SP_FISHERYprms[sp][b][flagimposecatch_id] > 0)
				flagimposecatch = 1;
			if (bm->SP_FISHERYprms[sp][b][flagdiscard_id] == impose_discard)
				flagimposediscard = 1;

		}
	}*/

	flagimposeeffort = 0;
	for (b = 0; b < bm->K_num_fisheries; b++) {
		if ((int) (bm->FISHERYprms[b][flageffortmodel_id]) == readts_effort)
			flagimposeeffort = 1;
	}

	/* Initialise effort timeseries */
	if (flagimposeeffort) {
		Harvest_Read_Time_Series(bm->inputFolder, bm->forceIfname, "Effort", bm->t_units, &bm->tsEffort, &bm->ntsEffort, &bm->tsEfforttype);

		if (bm->ntsEffort) {
			FisheryTimeSeries *this_ts = &bm->tsEffort[0];

			for (sp = 0; sp < bm->K_num_fisheries; sp++) {
				bm->tseffortid[sp] = -1;
				for (b = 0; b < this_ts->ts.nv; b++) {
					if (strcmp(FisheryArray[sp].fisheryCode, this_ts->ts.varname[b]) == 0)
						bm->tseffortid[sp] = b;
				}
				if (bm->tseffortid[sp] == -1)
					warn("Did not find time series of efforts for %s\n", FisheryArray[sp].fisheryCode);
			}
		} else
			quit("Expecting effort time series but didn't find any\n");
	} else
		bm->tsEffort = NULL;

	/* Get residuals - only one time series allowed for now */
	Harvest_Read_Time_Series(bm->inputFolder, bm->forceIfname, "Residuals", bm->t_units, &bm->tsResid, &bm->ntsResid, &bm->tsResidtype);
	if (bm->ntsResid) {
		FisheryTimeSeries *this_ts = &bm->tsResid[0];

		for (sp = 0; sp < bm->K_num_tot_sp; sp++) {
			if (FunctGroupArray[sp].isFished == TRUE) {
				bm->tseconid[sp] = -1;
				for (b = 0; b < this_ts->ts.nv; b++) {
					if (strcmp(FunctGroupArray[sp].groupCode, this_ts->ts.varname[b]) == 0)
						bm->tseconid[sp] = b;
				}
				if (bm->tseconid[sp] == -1)
					quit("Did not find time series of price residuals for %s in file %s\n", FunctGroupArray[sp].groupCode, this_ts->ts.name);
			}
		}
	} else {
    	warn("No market residual time series provided, this means that the pure market fish price model will be used rather than exact historical prices through any historical period\n");
    }

	/* Set up desired cohorts for high grading purposes */
	if (bm->flaghighgrading) {
		for (sp = 0; sp < bm->K_num_tot_sp; sp++) {
			if (FunctGroupArray[sp].isFished == TRUE) {
				fishery_id = 0;
			    // Note using "* FunctGroupArray[sp].numGeneTypes" here as harvest code refers back to age classes only for this parameter 
				for (ij = (FunctGroupArray[sp].numCohorts) - 1; ij >= 0; ij--) {
					if (bm->CatchTS_agedistrib[fishery_id][sp][ij] > bm->highgrade_thresh) {
						bm->SP_FISHERYprms[sp][fishery_id][desired_chrt_id] = ij;
					}
				}

				/* Let all fisheries impacting the group know the new desired size for high grading */
				for (fishery_id = 1; fishery_id < bm->K_num_fisheries; fishery_id++) {
					bm->SP_FISHERYprms[sp][fishery_id][desired_chrt_id] = bm->SP_FISHERYprms[sp][0][desired_chrt_id];
				}
			}
		}
	}

	/* Check to see what recreational fishing model is in use - important for population updating */
	bm->flagForceRec = 0;
	for (fishery_id = 0; fishery_id < bm->K_num_fisheries; fishery_id++) {
		bm->FISHERYprms[fishery_id][flagrecfish_id] = 0;

		/* Determine effort model type */
		flagspeffortmodel = (int) (bm->FISHERYprms[fishery_id][flageffortmodel_id]);

		if (flagspeffortmodel == rec_effort) {
			bm->flagForceRec = 1;
			bm->FISHERYprms[fishery_id][flagrecfish_id] = 1;
		} else if (flagspeffortmodel == rec_econ_model) {
			bm->FISHERYprms[fishery_id][flagrecfish_id] = 1;
		}

		/* Also check if CPUE based efort model in use - so know if need port updating */
		switch (flagspeffortmodel) {
		case const_effort:
		case const_qrt_effort:
		case cpue_effort:
		case array_effort:
		case wgt_effort:
		case rec_effort:
		case ideal_distrib_effort:
		case cpue_dyneffort:
		case readts_effort:
		case rec_econ_model:
		case econ_model_effort:
			bm->FISHERYprms[fishery_id][fisheries_need_port_id] = 0;
			break;
		case dist_effort:
		case alt_cpue_effort:
		case alt_cpue_scale_effort:
			bm->FISHERYprms[fishery_id][fisheries_need_port_id] = 1;
			break;
		default:
			quit("No such effort model %d for fishery %s\n", flagspeffortmodel, FisheryArray[fishery_id].fisheryCode);
			break;
		}
	}
    
    bm->do_syst_cap = 0;
    if (bm->do_sumB_HCR == per_system_cap) {
        bm->do_syst_cap = 1;
    }

	/* Set up check to see if only F forcing being used per species */
	for (sp = 0; sp < bm->K_num_tot_sp; sp++) {
		if (FunctGroupArray[sp].isFished == TRUE) {
			has_F = 0;
			has_effort = 0;
			for (fishery_id = 0; fishery_id < bm->K_num_fisheries; fishery_id++) {
				flagF = (int) (bm->SP_FISHERYprms[sp][fishery_id][flagF_id]);
				if (flagF) {
					has_F = 1;
				}
                
               // fprintf(llogfp, "%s fleet %s has effortlevel %e\n", FunctGroupArray[sp].groupCode, FisheryArray[fishery_id].name, bm->FISHERYprms[fishery_id][EffortLevel_id]);


				if (bm->FISHERYprms[fishery_id][EffortLevel_id] == 0) {
					no_mEff = 1;
					for (ij = 0; ij < 4; ij++) {
						if (mEff[fishery_id][ij] > 0)
							no_mEff = 0;
                        
                        
                    //    fprintf(llogfp, "%s fleet %s qrt %d has meff %e\n", FunctGroupArray[sp].groupCode, FisheryArray[fishery_id].name,  ij, mEff[fishery_id][ij]);

					}

					if (no_mEff < 1)
						has_effort = 1;
				} else
					has_effort = 1;
			}

			if ((has_F) && (!has_effort)) {
				FunctGroupArray[sp].speciesParams[flagFonly_id] = 1;
			}
            
           // fprintf(llogfp, "%s has flagFonly %d\n", FunctGroupArray[sp].groupCode, (int)(FunctGroupArray[sp].speciesParams[flagFonly_id]));

            FunctGroupArray[sp].speciesParams[estError_id] = estErrori[sp];
            FunctGroupArray[sp].speciesParams[estCV_id] = estCVi[sp];
            FunctGroupArray[sp].speciesParams[estBias_id] = estBiasi[sp];
            
            //printf("%s has estError: %e, estCV: %e, estBias: %e\n", FunctGroupArray[sp].groupCode, FunctGroupArray[sp].speciesParams[estError_id], FunctGroupArray[sp].speciesParams[estCV_id], FunctGroupArray[sp].speciesParams[estBias_id]);
            
			FunctGroupArray[sp].speciesParams[FrefA_id] = FrefAi[sp];
            FunctGroupArray[sp].speciesParams[FrefH_id] = FrefHi[sp];
            FunctGroupArray[sp].speciesParams[FrefLim_id] = FrefLimi[sp];
            
            FunctGroupArray[sp].speciesParams[F_restart_id] = FreStarti[sp];
            
            FunctGroupArray[sp].speciesParams[flag_systcap_sp_id] = FlagSystCapSPi[sp];
            FunctGroupArray[sp].speciesParams[sp_fishery_pref_id] = SystCapSPprefi[sp];
            
            if(!do_assess) {
                FunctGroupArray[sp].speciesParams[assess_nat_mort_id] = AssessMorti[sp];
            }
            
            
    /* Also check to see if shot-by-shot CPUE is needed */
            max_F = 0.0;
			for (fishery_id = 0; fishery_id < bm->K_num_fisheries; fishery_id++) {
                if (bm->SP_FISHERYprms[sp][fishery_id][q_id] && bm->SP_FISHERYprms[sp][fishery_id][flagPerShotCPUE_id]) {
                    bm->FISHERYprms[fishery_id][flagneed_shots_id] = 1;
                }
    /* And what the maximum starting F is */
                if (!bm->maxF_aggregate) {
                    if (bm->SP_FISHERYprms[sp][fishery_id][mFC_id] > max_F) {
                        max_F = bm->SP_FISHERYprms[sp][fishery_id][mFC_id];
                    }
                } else {
                    max_F += bm->SP_FISHERYprms[sp][fishery_id][mFC_id];
                }
                
            }
            FunctGroupArray[sp].speciesParams[maxmFC_id] = max_F;

		}

	}

	/* Check to see which (if any) stock based management routines are in play */
    bm->flagdisableTAC = 0;
	for (fishery_id = 0; fishery_id < bm->K_num_fisheries; fishery_id++) {
		bm->FISHERYprms[fishery_id][flagADJUSTmanage_id] = 0;

		/* Use perfect knowledge effort adjustments */
		if (bm->FISHERYprms[fishery_id][flagmanage_id] == 1)
			bm->FISHERYprms[fishery_id][flagADJUSTmanage_id] = 1;

		/* Use perfect knowledge seasonal adjustments */
		if (bm->FISHERYprms[fishery_id][flagchangeseason_id] == 1)
			bm->FISHERYprms[fishery_id][flagADJUSTmanage_id] = 1;

		/* Use perfect knowledge TAC adjustments */
		if (bm->FISHERYprms[fishery_id][TACchange_id] == change_dynamic)
			bm->FISHERYprms[fishery_id][flagADJUSTmanage_id] = 1;
        
        /* Check to see if TACs are even disabled */
        if (bm->FISHERYprms[fishery_id][TACchange_id] == disable_tac)
            bm->flagdisableTAC = 1;
                        
		/* Use perfect knowledge gear adjustments */
		if (bm->FISHERYprms[fishery_id][flagchangeSEL_id] == change_dynamic)
			bm->FISHERYprms[fishery_id][flagADJUSTmanage_id] = 1;

		/* Use perfect knowledge spatial management adjustments */
		flagfcmpa = (int) (bm->FISHERYprms[fishery_id][flagmpa_id]);
		switch (flagfcmpa) {
		case no_mpa: /* No spatial management - so set = 1.0 */
		case fix_mpa: /* Spatial management fixed at initial distribution */
		case cycle_mpa: /* Rotational spatial management */
		case mix_fix_rolling_mpa: /* Mix of fixed and rolling spatial management */
		case mix_f_r_spawn_mpa: /* Mix of fixed, rolling and seasonal spawning closures used in management */
		case depth_stock_mpa: /* US-like depth closures triggered when TAC exceeded */
		case council_stock_mpa: /* US-like closures (but based on biom distrib of groups) triggered when TAC exceeded */
		case pet_mpa: /* Spatial management responds to the size of charismatic species population (using fixed mpas) */
		case cycle_pet_mpa: /* Spatial management responds to the size of charismatic species population (using changing mpa locations) */
        case catch_mpa: /* Catch based spatal closures */
			/*** Nothing to do ***/
			break;
		case stock_mpa: /* Spatial management responds to the size of the target species for this fishery (using fixed mpas) */
		case cycle_stock_mpa: /* Spatial management responds to the size of the target
		 species for this fishery (using changing mpa locations) */
		case stock_pet_mpa: /* Spatial management responds to the size of the target
		 species for this fishery and the charismatic species populations (using fixed mpas) */
		case cycle_stock_pet_mpa: /* Spatial management responds to the size of the target
		 species for this fishery and the charismatic species populations (using changing mpa locations) */
			bm->FISHERYprms[fishery_id][flagADJUSTmanage_id] = 1;
			break;
		default:
			quit("No such mpa case defined (%d) - value must be between %d and %d currently\n", flagfcmpa, no_mpa, cycle_stock_pet_mpa);
			break;
		}
	}

	/* Check to see if any of the species use US-like MSY-based (tier6) management and
	 initialise the LeverQueue accordingly
	 */
	need_LeverQueue = 0;
	LeverQueue = 0;
	for (sp = 0; sp < bm->K_num_tot_sp; sp++) {
		if (FunctGroupArray[sp].isFished == TRUE) {
			FunctGroupArray[sp].speciesParams[use_lever_id] = LeverUsei[sp];
			tier = (int) (FunctGroupArray[sp].speciesParams[tier_id]);
			if (tier == tier6) {
				need_LeverQueue++;
			}
		}
	}
	bm->do_MSY = 0;
	if (need_LeverQueue) {
		/* Initialise LeverQueue array as using US-like MSY-based management */
		num_years = (int) (ceil((bm->tstop - bm->tstart) / (365.0 * 86400.0)));
		LeverQueue = Util_Alloc_Init_3D_Double(num_years, bm->K_num_fisheries, bm->K_num_tot_sp, 1.0);
		bm->do_MSY = 1;
	}

	/* Determine distance from fishing sites to ports */
	Distance_to_Port(bm);
    
	/* Free memory for those parameter arrays no longer needed (as allocated
	 to species/time/cohort specific entries of larger arrays */
	printf("Free unneeded memory\n");
	free1d(whichrefi);
    free1d(FrefAi);
    free1d(FrefHi);
    free1d(FrefLimi);
	free1d(LeverUsei);
    
    free1d(estErrori);
    free1d(estCVi);
    free1d(estBiasi);

    free1d(FreStarti);
    
    free1d(FlagSystCapSPi);
    free1d(SystCapSPprefi);
    
    if(!do_assess) {
        free1d(AssessMorti);
    }

    /* Generate CPUE data negative biominal distributions - moved to economics library as need subfleet array which not created here yet
    if(bm->flagStoreShotCPUE){
        CreateCPUEGenArrays(bm, llogfp);
        GenerateCPUEDistribution(bm, llogfp);
    }
    */

    if(verbose) {
        printf("Finished freeing memory in Manage_Init\n");
    }
    
	return;
}

void Calculate_BiTAC(MSEBoxModel *bm, FILE *llogfp) {
	int bim, sp, nreg, nf, i, fishery_id;
	int Kreg;
	double sumBiTAC;

	/* Read in BiTACs for use later */
	for (bim = 0; bim < 6; bim++) {

		/* FIX - need to generalise regional TAC so don't need to use this 2 region - north-south - assumption check */
		if (bm->K_num_active_reg > 2)
			Kreg = 2;
		else
			Kreg = bm->K_num_active_reg;

		/* Get total values and regional splits sorted */
		for (sp = 0; sp < bm->K_num_tot_sp; sp++) {
			if (FunctGroupArray[sp].isImpacted == TRUE) {

				/* Total combined % of total TAC */
				for (nreg = 0; nreg < Kreg; nreg++) {
					bm->BiTAC_sp[bim][nreg][sp][now_id] = bm->BiTAC_sp[bim][nreg][sp][old_id];
				}
				for (nreg = 0; nreg < totalreg_id; nreg++) {
					bm->BiTAC_sp[bim][totalreg_id][sp][nreg] = bm->BiTAC_sp[bim][0][sp][nreg] + bm->BiTAC_sp[bim][1][sp][nreg];
				}

				/* Regional splits - assumes only two regions and assumes using cumTAC management (US-like system) */
				for (nf = 0; nf < bm->K_num_fisheries; nf++) {
					if (bm->FISHERYprms[nf][flagmanage_id] == cumTACclosure_mgmt) {
						for (nreg = 0; nreg < Kreg; nreg++) {
							bm->RegionalData[sp][nreg][reg_tac_id] = bm->BiTAC_sp[bim][nreg][sp][now_id] / (bm->BiTAC_sp[bim][totalreg_id][sp][now_id]
									+ small_num);
						}
					}
				}
			}
		}
	}

	/*	Group specific fisheries parameters (fishery specific parameters are
	 read in directly in readModelFCParameter()
	 */
	for (i = 0; i < bm->K_num_fisheries; i++) {
		if (bm->FISHERYprms[i][flagmanage_id] == coBRTACtrans_mgmt)
			quit(
					"Code maybe incomplete, contact the idiot who wrote this and ask if the <companion, regional and basket TACs where basket groups can transition to individual TAC> option is complete yet\n");

		//
		for (sp = 0; sp < bm->K_num_tot_sp; sp++) {
			if (FunctGroupArray[sp].isImpacted == TRUE) {
				if (bm->SP_FISHERYprms[sp][i][p_escape_id] > 1)
					warn("Manage_Init: p_escape is > 1 (it's %e) for species or group with id number %d\n", bm->SP_FISHERYprms[sp][i][p_escape_id],
							FunctGroupArray[sp].groupCode);

				if (FunctGroupArray[sp].isFished == TRUE) {
					bm->TACamt[sp][i][now_id] = bm->SP_FISHERYprms[sp][i][TAC_id];

					if (bm->SP_FISHERYprms[sp][i][TAC_id] < no_quota) {
						for (bim = 0; bim < 6; bim++)
							bm->BiTACamt[bim][sp][i][now_id] = bm->BiTAC_sp[bim][totalreg_id][sp][old_id] * bm->TACamt[sp][i][now_id];
					} else {
						for (bim = 0; bim < 6; bim++)
							bm->BiTACamt[bim][sp][i][now_id] = no_quota;
					}
				} else {
					bm->SP_FISHERYprms[sp][i][TAC_id] = MAXDOUBLE;
					bm->TACamt[sp][i][now_id] = no_quota;
				}

				bm->SP_FISHERYprms[sp][i][catch_allowed] = bm->SP_FISHERYprms[sp][i][TAC_id];
				bm->SP_FISHERYprms[sp][i][mFC_scale_id] = 1.0;

				if (bm->SP_FISHERYprms[sp][i][TAC_id] < no_quota)
					bm->SP_FISHERYprms[sp][i][flagquota_id] = 1;

				if (bm->FISHERYprms[i][flagmanage_id] < 2)
					bm->SP_FISHERYprms[sp][i][catch_allowed] = no_quota;

				if (FunctGroupArray[sp].isFished == TRUE) {
					if (bm->SP_FISHERYprms[sp][i][catch_allowed] > bm->SP_FISHERYprms[sp][i][trip_lim_id])
						bm->SP_FISHERYprms[sp][i][catch_allowed] = bm->SP_FISHERYprms[sp][i][trip_lim_id];
				}
			}
		}

		/* Convert TACs from tonnes to kg */
		for (sp = 0; sp < bm->K_num_tot_sp; sp++) {
			if (FunctGroupArray[sp].isImpacted == TRUE) {

				bm->SP_FISHERYprms[sp][i][trip_lim_id] *= (kg_2_mg / bm->X_CN);

				/* If at MAXDOUBLE already don't rescale */
				if (bm->SP_FISHERYprms[sp][i][TAC_id] < MAXDOUBLE) {
					bm->SP_FISHERYprms[sp][i][TAC_id] *= 1000.0;
					bm->TACamt[sp][i][now_id] *= 1000.0;
					for (bim = 0; bim < 6; bim++) {
						bm->BiTACamt[bim][sp][i][now_id] *= 1000.0;
					}
				}
				/* Convert catch_allowed from tonnes to mg */
				if (bm->SP_FISHERYprms[sp][i][catch_allowed] < no_quota) {
					bm->SP_FISHERYprms[sp][i][catch_allowed] *= (tonne_2_mg / bm->X_CN);
					/* Identify fisheries participating in quotas (catch allowed set to TAC so use catch allowed here) */
					bm->inQuota[i][sp] = 1;
				}

				if (bm->inQuota[i][sp]) {
					for (bim = 0; bim < 6; bim++)
						bm->BiTACamt[bim][sp][i][now_id] = no_quota;
				}
			}
		}
		if (!i)
			bm->FISHERYprms[i][flagconflict_id] = 0;

		for (fishery_id = 0; fishery_id < bm->K_num_fisheries; fishery_id++) {
			if (gear_conflict[i][fishery_id])
				bm->FISHERYprms[i][flagconflict_id] = 1;
		}
	}

	/* Convert regional BiTAC from proportion to actual TAC for use later */
	for (sp = 0; sp < bm->K_num_tot_sp; sp++) {
		if (FunctGroupArray[sp].isImpacted == TRUE) {
			for (nreg = 0; nreg < bm->K_num_active_reg; nreg++) {
				for (bim = 0; bim < 6; bim++) {
					sumBiTAC = 0;
					for (i = 0; i < bm->K_num_fisheries; i++) {
                        if(!FunctGroupArray[sp].isTAC || (bm->TACamt[sp][nf][now_id] < no_quota)) {
                            sumBiTAC += bm->BiTAC_sp[bim][nreg][sp][now_id] * bm->TACamt[sp][i][now_id];
                        }
					}
					bm->BiTAC_sp[bim][nreg][sp][now_id] = sumBiTAC;
				}
			}
		}
	}
}

/**
 * \brief Determine if we are going to be visiting the council in a US-like system.
 *
 * @return 1 if we are going to be visiting councils, 0 otherwise.
 */
int Is_Council_Needed(MSEBoxModel *bm, FILE *llogfp) {

	int council_needed = 0;
	int nf, flagspeffortmodel;

	for (nf = 0; nf < bm->K_num_fisheries; nf++) {
		/* Check whether effort model uses previous days effort in setting new effort */
		flagspeffortmodel = (int) (bm->FISHERYprms[nf][flageffortmodel_id]);
		if ((flagspeffortmodel < wgt_effort) || (flagspeffortmodel == ideal_distrib_effort)) {
			bm->FISHERYprms[nf][flaguse_preveffort_id] = 0;
		} else
			bm->FISHERYprms[nf][flaguse_preveffort_id] = 1;
		/* Check to see if need to visit councils in a US-like system */
		flagspeffortmodel = (int) (bm->FISHERYprms[nf][flagmanage_id]);
		bm->FISHERYprms[nf][visitcouncil_id] = 0;
		if (flagspeffortmodel == cumTACclosure_mgmt) {
			council_needed = 1;
			bm->FISHERYprms[nf][visitcouncil_id] = 1;
		}
	}

	if (council_needed && bm->K_num_reg > 2) {
		quit("Current US-like management using cumulative trip limits and spatial closures can only support two management regions\n");
	}

	return council_needed;
}

/**
 * \brief Determine how much of the effort in the box is applied actually at this depth.
 * If considering a an area that is too shallow to have all
 * the water column layers present	rescale (done in set-up, result read-in here
 */

void Calculate_Effort_Depth(MSEBoxModel *bm) {

	int maxdeep, totaldeep, diffdeep, layerk, ij, b, k, nf;
	double vertdistrib, sumnz1, sumnz2;

	maxdeep = bm->wcnz;
	for (b = 0; b < bm->wcnz; b++) {
		/* Determine how much of the effort in the box is applied actually at this depth.
		 If considering a an area that is too shallow to have all
		 the water column layers present	rescale (done in set-up, result read-in here */

		for (nf = 0; nf < bm->K_num_fisheries; nf++) {
			for (ij = 0; ij < bm->nbox; ij++) {
				totaldeep = bm->boxes[ij].nz;
				if (b < totaldeep) {
					diffdeep = 0;
					layerk = 0;
					if (totaldeep < maxdeep) {
						diffdeep = maxdeep - totaldeep;
						sumnz1 = 0.0;
						sumnz2 = 0.0;
						for (k = 0; k < bm->boxes[ij].nz; k++) {
							layerk = k + diffdeep;
							sumnz1 += origEffort_vdistrib[nf][layerk];
							sumnz2 += origEffort_vdistrib[nf][k];
						}
						if (sumnz1 < sumnz2) {
							vertdistrib = origEffort_vdistrib[nf][b] / sumnz2;
						} else if (!sumnz1) {
							if (bm->FISHERYprms[nf][flagdempelfishery_id] && (b == 0)) {
								vertdistrib = 1;
							} else if (!bm->FISHERYprms[nf][flagdempelfishery_id] && (b == totaldeep - 1)) {
								vertdistrib = 1;
							} else {
								vertdistrib = 0;
							}
						} else {
							layerk = b + diffdeep;
							vertdistrib = origEffort_vdistrib[nf][layerk] / sumnz1;
						}
					} else
						vertdistrib = origEffort_vdistrib[nf][b];
				} else
					vertdistrib = 0;
				Effort_vdistrib[ij][b][nf] = vertdistrib;
			}
		}
	}
	free2d(origEffort_vdistrib);

}

void Allocate_Arrays_Pre_Load(MSEBoxModel *bm) {
	int nfleets = bm->K_num_fisheries;

	if(verbose > 0)
		printf("Creating management PreLoad arrays\n");

	/* These are always allocated even if the management option is off */
	bm->reg_season = (int *) i_alloc1d(bm->K_num_reg);
	bm->phased_out = Util_Alloc_Init_2D_Double(nfleets, bm->K_num_tot_sp, 1.0);
	bm->BiTAC_sp = Util_Alloc_Init_4D_Double(2, bm->K_num_tot_sp, totalreg_id + 1, 6, 0.0);;
	bm->BiTACamt = Util_Alloc_Init_4D_Double(2, nfleets, bm->K_num_tot_sp, 6, 0.0);
	bm->TACamt = Util_Alloc_Init_3D_Double(K_num_TAC_entries, nfleets, bm->K_num_tot_sp, 0.0);
	bm->TripCatch = Util_Alloc_Init_2D_Double(nfleets, bm->K_num_tot_sp, 0.0);
	bm->TotCumRecCatch = Util_Alloc_Init_2D_Double(nfleets, bm->K_num_tot_sp, 0.0);
	// bm->TotCumCatch = Util_Alloc_Init_3D_Double(max_years, nfleets, bm->K_num_tot_sp, 0.0 );
    bm->TotOldCumCatch = Util_Alloc_Init_2D_Double( nfleets, bm->K_num_tot_sp, 0.0 );
	bm->TotCumBiCatch = Util_Alloc_Init_2D_Double(nfleets, bm->K_num_tot_sp, 0.0);
	bm->inQuota = Util_Alloc_Init_2D_Int(bm->K_num_tot_sp, nfleets, 0);
	bm->TotCumDiscards = Util_Alloc_Init_2D_Double(nfleets, bm->K_num_tot_sp, 0.0);
	bm->TotOldCumDiscards = Util_Alloc_Init_2D_Double( nfleets, bm->K_num_tot_sp, 0.0 );


	/* Performance indicators */
	manageindxNAME = (char **) c_alloc2d(20, K_num_manage_indx);
	manageindx = Util_Alloc_Init_2D_Double(K_num_manage_indx, bm->K_num_fisheries, 0.0);

	if (!bm->flag_fisheries_on) {
		/* If not using fishing don't bother with any of the parameters, set the ID match and leave */
		printf("Skipping fisheries as not active\n");

		return;
	}
	FC_case = Util_Alloc_Init_2D_Double(bm->K_num_fisheries, bm->K_num_tot_sp, 0.0);
	/* Allocate and init memory for default MPAs and read-in MPA arrays */
	bm->MPA = Util_Alloc_Init_2D_Double(bm->K_num_fisheries, bm->nbox, 1.0);


	//catchind = Util_Alloc_Init_2D_Int( nfleets, bm->K_num_tot_sp, 0);
	DistPeak = Util_Alloc_Init_1D_Double(nfleets, 0.0);

	effort_scale = Util_Alloc_Init_2D_Double(4, nfleets, 0.0);
	Effort_vdistrib = Util_Alloc_Init_3D_Double(nfleets, bm->wcnz, bm->nbox, 0.0);

	flagdropeffort = Util_Alloc_Init_1D_Int(nfleets, 0);
	FC_hdistrib = Util_Alloc_Init_2D_Double(nfleets, bm->nbox, 0.0);

	oldFishEndDay = Util_Alloc_Init_1D_Double(nfleets, 0.0);

	p_fishi = Util_Alloc_Init_3D_Double(nfleets, bm->K_num_max_stages, bm->K_num_tot_sp, 0.0);
	p_fish_origi = Util_Alloc_Init_4D_Double(nfleets, bm->K_num_max_stages, bm->K_num_tot_sp, bm->nbox, 0.0);
	prev_mult = Util_Alloc_Init_1D_Double(nfleets, 1.0);
    
	scale_effort = Util_Alloc_Init_1D_Double(nfleets, 0.0);
	SEASONAL = Util_Alloc_Init_2D_Double(2, nfleets, 0.0);

	mEff = Util_Alloc_Init_2D_Double(4, bm->K_num_fisheries, 0.0);
	gear_conflict = Util_Alloc_Init_2D_Double(bm->K_num_fisheries, bm->K_num_fisheries, 0.0);

	MPAendangeredlist = Util_Alloc_Init_3D_Double(bm->K_num_fisheries, bm->nbox, 365, 1.0);
	MPAoverfishedlist = Util_Alloc_Init_2D_Double(bm->K_num_tot_sp, bm->nbox, 1.0);

	origEffort_vdistrib = Util_Alloc_Init_2D_Double(bm->wcnz, bm->K_num_fisheries, 0.0);
    
    bm->ContamClosed = Util_Alloc_Init_1D_Double(bm->nbox, 0.0);

}

void Allocate_Arrays_Post_Load(MSEBoxModel *bm) {

	if(verbose > 0)
		printf("Allocating management post load array");

	//printf("Creating management PostLoad arrays\n");

	bm->TAC_over = Util_Alloc_Init_3D_Int(bm->K_num_basket, bm->K_num_fisheries, bm->K_num_tot_sp, 0);
    
	if(bm->useRBCTiers)
		Open_Tiered_Output_File(bm);
}

/* Memory rationalisation ************************************************************/
/**
 * \brief Free memory for those parameter arrays no longer needed (as allocated
 * to species/time/cohort specific entries of larger arrays in the main setup code)
 */

/**
 *
 * \brief This routine reads all parameters of the fisheries models
 *
 *  Input:  char *filename: pointer to constant filename
 *
 */
int Init_Manage_Flags(MSEBoxModel *bm, FILE *llogfp) {
	int index;
	int i, nf, change2oneQ, change2oneF, change2oneD;

	/* This is never used */
	//bm->invert_incidmort = readFCValue ( bm->ecotest, infile, no_checking, "invert_incidmort" );

	for (nf = 0; nf < bm->K_num_ports; nf++) {
		if (bm->Port_info[nf][port_start_id] > 0) {
			bm->flagnewport = 1;
		}
	}

	bm->K_num_basket = 1;
	for (i = 0; i < bm->K_num_tot_sp; i++) {
		if (FunctGroupArray[i].isImpacted == TRUE) {

			if (FunctGroupArray[i].speciesParams[basket_size_id] > bm->K_num_basket)
				bm->K_num_basket = (int) (FunctGroupArray[i].speciesParams[basket_size_id]);
		}
	}

	for (nf = 0; nf < bm->K_num_fisheries; nf++) {
		change2oneQ = 0;
		change2oneF = 0;
		change2oneD = 0;

		/* Don't do the loop unless check is necessary */
		if (bm->flagchangep && bm->flagchangeF && bm->flagchangediscard)
			break;

		if (!bm->flagchangeq) {
			for (index = 0; index < bm->K_num_tot_sp; index++) {
				if (bm->SP_FISHERYprms[index][nf][flagQchange_id])
					change2oneQ = 1;
			}
			if (change2oneQ) {
				warn("As flagQchange > 0 for %s then setting general flagchangeq = 1\n", FisheryArray[nf].fisheryCode);
				bm->flagchangeq = 1;
				break;
			}
		}

		/* Forced F changes */
		if (!bm->flagchangeF) {
			for (index = 0; index < bm->K_num_tot_sp; index++) {
				if (bm->SP_FISHERYprms[index][nf][flagFchange_id])
					change2oneF = 1;
			}

			if (change2oneF) {
				warn("As flagFchange > 0 for %s then setting general flagchangeF = 1\n", FisheryArray[nf].fisheryCode);
				bm->flagchangeF = 1;
				break;
			}
		}

		/* Check discard changes */
		if (!bm->flagchangediscard) {
			for (index = 0; index < bm->K_num_tot_sp; index++) {
				if (bm->SP_FISHERYprms[index][nf][flagchangeDISCRD_id])
					change2oneD = 1;
			}

			if (change2oneD) {
				warn("As flagchangeDISCRD > 0 for %s then setting general flagchangediscard = 1\n", FisheryArray[nf].fisheryCode);
				bm->flagchangediscard = 1;
				break;
			}
		}
	}

	/* Read-in and populate MPA changes matrix, including changes in zoning for each box per fishery */
	printf("Setting up MPA lists\n");

	Setup_MPA_Lists(bm, llogfp);

	return TRUE;
}

/**
 * \brief Free fishery arrays
 */
void Manage_Free(MSEBoxModel *bm) {

//	/* Close the management performance output file */
//	Close_Management_Index_File(bm);
//	Close_Manage_Output_Files(bm);
//
//	free2d(bm->TripCatch);
//	/** The management performance indicators */
//	c_free2d(manageindxNAME);
//	free2d(manageindx);

	if (!bm->flag_fisheries_on) {
		/* If not using fishing don't bother with any of these arrays */

		return;
	}
    
    if (bm->useRBCTiers) {
        Close_Tiered_Output_Files(bm);
    }
    

	i_free1d(bm->reg_season);
	free2d(bm->TotCumRecCatch);

	free2d(bm->phased_out);
	free3d(bm->TACamt);
	free4d(bm->BiTACamt);
	free4d(bm->BiTAC_sp);
	free2d(bm->TotOldCumCatch);
	free2d(bm->TotCumBiCatch);
	free2d(bm->TotCumDiscards);
	free2d(bm->TotOldCumDiscards);
	i_free2d(bm->inQuota);


	/* Close the management performance output file */
	Close_Management_Index_File(bm);
	Close_Manage_Output_Files(bm);

	free2d(bm->TripCatch);
	/** The management performance indicators */
	c_free2d(manageindxNAME);
	free2d(manageindx);

	printf("Freeing fishery specific arrays\n");


	i_free1d(MPAKeyMap);
	// i_free2d ( catchind );
	free1d(DistPeak);
	free2d(effort_scale);
	free3d(Effort_vdistrib);

	i_free1d(flagdropeffort);
	free2d(FC_hdistrib);

	free1d(oldFishEndDay);

	free3d(p_fishi);
	free4d(p_fish_origi);
	free1d(prev_mult);
    
    free3d(qSTOCK);
    
	free1d(scale_effort);
	free2d(SEASONAL);

	free2d(FC_case);
	free2d(mEff);
	free2d(gear_conflict);
	free3d(MPAendangeredlist);
	free2d(MPAoverfishedlist);
    
    free1d(bm->ContamClosed);

	if (bm->flag_fisheries_on == TRUE) {
		free3d(CAPchange);
		free3d(EFFORTchange);
	}
	if (LeverQueue)
		free3d(LeverQueue);
    
    if(bm->flagStoreShotCPUE) {
        free3d(cpue_cdf);
        i_free1d(bm->box_done);
        i_free1d(bm->shot_loc);
        free1d(bm->alloced_effort);
        free3d(bm->alloced_catch);
        free2d(bm->alloced_discard);
        free1d(bm->prop_depth);
        free1d(bm->prop_effort);
        free3d(bm->ShotData);
        free1d(tot_alloced);

    }

	/** Free the fisheries time series memory */
	Harvest_Free_Time_Series(bm->tsResid, bm->ntsResid);

	return;
}

void Calculate_Flag_Impose(MSEBoxModel *bm) {

	int fleet, sp, nimp, maximp, b;
	int *impose_classes; // local array used in setting of flagimposeglobal

	/* Indices needed to define array size */
	impose_classes = Util_Alloc_Init_1D_Int(K_num_impose_classes, 0);

	for (fleet = 0; fleet < bm->K_num_fisheries; fleet++) {
		for (sp = 0; sp < bm->K_num_tot_sp; sp++) {
			if (FunctGroupArray[sp].isImpacted == TRUE) {
				/* Find out what setting of flagimpose global should be */
				for (nimp = 0; nimp < K_num_impose_classes; nimp++) {
					if (bm->SP_FISHERYprms[sp][fleet][flagimposecatch_id] > 0)
						impose_classes[nimp] += 1;
				}

	            /* If using Bulk TAC then scale up TACamt accordingly */
	            if( bm->bulkTAC && (FunctGroupArray[sp].speciesParams[tac_resetperiod_id] > 1.0)) {
	            	bm->TACamt[sp][fleet][now_id] *= FunctGroupArray[sp].speciesParams[tac_resetperiod_id];
	            }


			}
		}
	}

	/* Find final flagimposeglobal setting - most common result across all groups */
	maximp = 0;
	for (nimp = 0; nimp < K_num_impose_classes; nimp++) {
		if (impose_classes[nimp] > maximp) {
			maximp = impose_classes[nimp];
			bm->flagimposeglobal = nimp;
		}
	}

	/* Free local array */
	i_free1d(impose_classes);

	for (sp = 0; sp < bm->K_num_tot_sp; sp++) {
		if (FunctGroupArray[sp].isFished == TRUE) {
			b = 0;
			for (fleet = 0; fleet < bm->K_num_fisheries; fleet++) {
				if (bm->SP_FISHERYprms[sp][fleet][flagimposecatch_id] > 0)
					b++;
				if ((!bm->SP_FISHERYprms[sp][fleet][FC_reportscale_id]) && (bm->SP_FISHERYprms[sp][fleet][flagimposecatch_id] > 0)) {
					warn("%s fishing for %s imposed catches on, but got reportscale = 0 so no effective catch possible!\n", FunctGroupArray[sp].groupCode,
							FisheryArray[fleet].fisheryCode);
				}
			}
			if (b > 1)
				quit("%s has > 1 fishery with flagimposecatch = 1 - with current code this will produce erroneously large catch, reset or recode\n",
						FunctGroupArray[sp].groupCode);
		}
	}

}

/***************************************************************************************************************************************************************
 *
 * \brief Generate the cdf for the distriibutiosn to be used in creating the shot level CPUE data
 */
void GenerateCPUEDistribution(MSEBoxModel *bm, FILE *llogfp) {
    
    int i, sp, cdf_sp, nf;
    int cdf_case = bm->PerShotCDFtype;
    double step1 = 0, step2 = 0;
    
    // First generate the relevant arrays
    cpue_cdf = Util_Alloc_Init_3D_Double(2, bm->K_num_catchbin, bm->K_num_tot_sp, 0.0);
    
    // Now populate the cdf
    for (sp = 0; sp < bm->K_num_tot_sp; sp++) {
        
        cdf_sp = 0;
        for (nf = 0; nf < bm->K_num_fisheries; nf++) {
            if(bm->SP_FISHERYprms[sp][nf][flagPerShotCPUE_id]) {
                cdf_sp = 1;
                break;
            }
        }
        
        if (cdf_sp) {
            for (i=0; i < bm->K_num_catchbin; i++) {
                switch (cdf_case) {
                    case negbiom_type:
                        step1 = negbinomial_cdf_values(i, (int)FunctGroupArray[sp].speciesParams[cpue_cdf_poor_r_id], FunctGroupArray[sp].speciesParams[cpue_cdf_poor_p_id]);
                        step2 = negbinomial_cdf_values(i, (int)FunctGroupArray[sp].speciesParams[cpue_cdf_top_r_id], FunctGroupArray[sp].speciesParams[cpue_cdf_poor_p_id]);
                        break;
                    default: quit("Other distributions are possible - refer to cdflib and instantiate\n");
                        break;
                }
        
                cpue_cdf[sp][i][0] = step1;        // Case for "poor" skippers
                cpue_cdf[sp][i][1] = step2;        // Case for "effective" skippers
        
            }
        }
        
    }
    
    
    return;
}

/***************************************************************************************************************************************************************
 *
 * \brief Generate the cdf for the distriibutiosn to be used in creating the shot level CPUE data
 */
void CreateCPUEGenArrays(MSEBoxModel *bm, FILE *llogfp) {
    int nf, ns, boat_count;
    int num_shots_per_day = (int)(ceil(24 / bm->K_min_shotlength));
    int max_poss_shots;

    bm->K_max_num_boats = 0;
    for (nf = 0; nf < bm->K_num_fisheries; nf++) {
        boat_count = 0;
        if (bm->FISHERYprms[nf][flageffortmodel_id] > rec_econ_model) {
            for (ns = 0; ns < bm->FISHERYprms[nf][nsubfleets_id]; ns++) {
                boat_count += (int)bm->SUBFLEET_ECONprms[nf][ns][nboat_id];
            }
            if (boat_count > bm->K_max_num_boats )
                bm->K_max_num_boats = boat_count;
        }
    }
    
    max_poss_shots = bm->K_max_num_subfleet * bm->K_max_num_boats * num_shots_per_day + bm->nbox + 1;

    bm->box_done = Util_Alloc_Init_1D_Int(bm->nbox, 0);
    bm->shot_loc = Util_Alloc_Init_1D_Int(max_poss_shots, 0);
    bm->alloced_effort = Util_Alloc_Init_1D_Double(bm->nbox, 0.0);
    bm->alloced_catch = Util_Alloc_Init_3D_Double(bm->wcnz, bm->nbox, bm->K_num_tot_sp, 0.0);
    bm->alloced_discard = Util_Alloc_Init_2D_Double(bm->nbox, bm->K_num_tot_sp, 0.0);
    bm->prop_depth = Util_Alloc_Init_1D_Double(bm->wcnz, 0.0);
    bm->prop_effort = Util_Alloc_Init_1D_Double(bm->nbox, 0.0);
    bm->ShotData = Util_Alloc_Init_3D_Double(K_num_shot_data_entries, bm->K_num_tot_sp, max_poss_shots, 0.0);
    tot_alloced = Util_Alloc_Init_1D_Double(bm->K_num_tot_sp, 0.0);

    return;
}
