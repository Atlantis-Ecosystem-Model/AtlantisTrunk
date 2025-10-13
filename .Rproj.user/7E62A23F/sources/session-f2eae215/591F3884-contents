/**
 \file
 \brief C file for the main ecological subroutines
 \ingroup atEcology

 File:	atecology.c
 Author:	Beth Fulton
 Created:	21/9/2004
 Purpose:	C file for the main ecological subroutines for south east
 Revisions: 8/8/2004 Created mseecom.c from the seecom.c file

 22/8/2004 Updated to match parameters now in bm->SP_prms

 21/9/2004 Created atecology.c from mseecom.c. And renamed
 FSL, FSO, FSF, FST and FSG to generic names so apply to
 other models/generic case.
 FSL -> FDC  (Ling -> Demersal_DC_Fish)
 FSO -> FDO  (Orange_Roughy -> Demersal_O_Fish)
 FSF -> FDB  (Flathead -> Demersal_B_Fish)
 FST -> FVO  (Bluefin_Tuna -> Pisciv_O_Fish)
 FSG -> SHB  (Gummy_Shark -> Shark_B)

 22/9/2004 Replaced numbers in switch statements with id
 guild numbers. Also added age structure to cephalopods.

 30/9/2004 Added extra trophic links to exiting groups so
 they match the generic diet matrix (see comments in
 atprocess.c)

 1/10/2004 Reorganised variable declarations so easier to see what
 variables are associated with what groups in Water_Column_Box(),
 Sediment_Box(), Epibenthic_Box(). And moved all invertbrate
 structural equivalency parameters (XX_sn) to core boxmodel
 (e.g. CEP_sn is now bm->CEP_sn) so assessment library can
 access them too.

 15/10/2004 Added BML and BFF.

 19/10/2004 Added Prawns

 24/10/2004 Added soft sediments

 26/10/2004 Added FBP, FVB, SP, FDE, FDF, SHR, SSK, REP, WDG

 19/1/2005 Added Microphytobenthos (MB)

 8/4/2005 Added Invert_Activities() to handle mortality, growth,
 assimilation and waste equations in a general way.

 9/4/2005 Added 	bm->INVERT_PREYeatenGlobal, bm->NutsProdGlobal,
 bm->NutsLostGlobal, bm->DetritusProdGlobal, bm->DetritusLostGlobal,
 bm->NutsProd, bm->NutsLost, bm->DetritusProd, bm->DetritusLost,
 created to simplify handling of handlgin of values spanning the
 it_count == 1 and subsequent iterations of the adaptive timestep.

 10/4/2005 Added Primary_Production() to handle all primary production

 19/7/2005 Added Extended catch and discards storage FstatPool entries

 16/8/2005 Put in default case for all switch statements

 1/9/2005 Added multiple stock structure (horizontal and vertical) and
 Catch per layer so can identify catch from vertical stocks

 25/9/2005 Added boxLayerInfo->DebugFluxInfo so can track fluxes of a specific group
 (defined by setting which_check to guild of choice)

 11/5/2007 Initialisation of BDreleaseNH and BOreleaseNH added (so not
 passing a memory address which effectively prevented denitrification
 in all but the very first pass of the adaptive timestep of each day).

 20/05/2008 Bec Gorton
 Fixed the code that sets DiagNETFLUX. The first cohort for a number of
 species was not being set.

 23/05/2008 Bec Gorton
 Using setInitalVertVectors(bm, Pool); in atbiolUtil to set the initial value of
 VERTinfo instead of setting the values for each functional group.
 Changed the code that sets the NETFLUX to loop over the species and cohorts.
 Added a new function setNETFLUX to complete this loop.

 29/05/2008 Bec Gorton
 Change the code to set the Pool values to use the indices stored in
 FunctGroupArray[fgIndex].totNTracers[0[ instead of the old tracers like
 Planktiv_S_Fish_N_i.

 30/06/2008 Bec Gorton
 Changed the code that sets the Size/age distributions for catch, discards (and bycatch)
 to use the Catch and Discard tracer indices stored in the functional group array.
 Also fixed a bug in Water_Column_Box and Sediment_Box. Code was setting the DiagNGain_i to 0 in the DiagPool
 instead of Pool. This was overwritting a number of tracers in the diagnostic pool.

 09-07-2008 Bec Gorton

 Changed the Epibenthic_Box function to use a function Calculate_Catch to update the catch and discard
 arrays.

 18/06/2008 Beth Fulton
 Replaced DiagPool[DiagNGain], DiagPool[DiagNLoss] and DiagPool[DiagNFlux]
 with Pool[] etc.

 1/7/2008 Beth Fulton
 To make fisheries code simpler to carry about moved all dt and cell_vol
 corrections on Catch and Discards to atharvest.c

 18/06/2008 Beth Fulton
 Replaced DiagPool[DiagNGain], DiagPool[DiagNLoss] and DiagPool[DiagNFlux]
 with Pool[] etc.

 1/7/2008 Beth Fulton
 To make fisheries code simpler to carry about moved all dt and cell_vol
 corrections on Catch and Discards to atharvest.c

 01/08/2008 Bec Gorton
 Moved the update catch code into a new function which can then be called for a
 any functional group where the catch values need to be updated.
 Also moved the invertActivities and Eat code for the invertebrates into functions.

 11/08/2008 Bec Gorton
 Moved the invert properties that are calculated in Primary_Production into the
 FuntGroup structure. This represents a more OO approach.

 03/10/2008 Bec Gorton
 Removed the  bm->INVERT_PREYeaten and bm->INVERT_PREYeatenGlobal arrays. These values
 have been moved into the funct group structure.

 03/11/2008 Bec Gorton
 Removed the dead arrays from Water_Column_Box and Epibenthic_Box. These dead values
 are now stored in the processProps structure. This has also removed the need to store
 the global dead values.

 02-02-2009 Beth Fulton
 Got rid of the use of WC, SED and EPIFAUNA - these have been replaced with WC,
 SED and EPIFAUNA. Also change the variable called small to small_num.


 04-03-2009 Bec Gorton
 Added the following variables to the atEcologyModule:

 int pelagicBactIndex;
 int sedBactIndex;
 int LabDetIndex;
 int CarrionIndex;

 these are used to replace the direct references to the DR, DL and PB groups. these indices
 are set in setEcologyFunctionPointers.

 Created process functions for each of these groups.

 23-03-2009 Bec Gorton
 Rewrote the ReminNet calculations. Removed many of the local variables from the
 Sediment_Box code.

 10-06-2009 Bec Gorton
 Added the SEAGRASS invertebrate type.

 21-07-2009 Bec Gorton
 Fixed a couple of bugs in the sp_IRR value calculations. These are not used at present but needed to be fixed
 to stop valgrind warnings.
 28-10-2009 Bec Gorton
 Removed references to K_num_fished_sp and K_num_impacted_sp

 02-11-2009 Bec Gorton
 Fixed a bug in the debug flux output code. The water column values were being used in the sediment code.

 03-11-2009 Bec Gorton
 Removed references to bm->SPtoCATid.


 05-11-2009 Bec Gorton
 Trunk Merge 1134 - Added support for getting mortality and detritus flux values back from the EwE model.
 These are then stored in the new linkage arrays in the atEcologyModule and included in the
 group rate of change equations.
 I have yet to include these values in the calcM mortality calculations as these need to be fixed first.


 29-01-2010 Bec Gorton
 Changed the code that updated the harvest tracers to now call a function Harvest_Update_Total_Catch in the harvest
 library.

 03-02-2010 Bec Gorton
 Changed the functions in this file to expect a series of arrays that store the current tracer and flux values
 for the box/layer we are doing calculations for instead of using arrays in atEcologyModule. In the future we might
 want to parallelise the code and shared arrays make this very hard.

 04-05-2010 Bec Gorton
 Change the habitat scaling code to use the new generic Util_Get_Accumulative_Change_Scale function
 to calculate the level of scaling on a habitat. This will also change the way that the scaling is done -
 its now accummulative so more than one scaling event can take place at any given timestep.

 16-08-2011 Beth Fulton
 Added DependDiscardsTot so that discards dependent on catch could be represented without the
 full harvest model being used

 17-04-2013 Bec Gorton
 ATL-346 - Added support of tracking P:N:C ratios in groups.
 **************************************************************************************

 General Notes:

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
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <sjwlib.h>
#include "atecology.h"
#include <atHarvestLib.h>

static void Update_Vertebrate_Tracers(MSEBoxModel *bm, FILE *llogfp, HABITAT_TYPES habitatType, BoxLayerValues *boxLayerInfo);
static void Reconcile_Global_Values(MSEBoxModel *bm, BoxLayerValues *boxLayerInfo, HABITAT_TYPES habitatType, FILE *llogfp);
static void Check_Prey_Is_Finite(MSEBoxModel *bm, FILE *llogfp, HABITAT_TYPES habitat_type);
static void Reset_Arrays(MSEBoxModel *bm, HABITAT_TYPES habitat_type, int it_count, BoxLayerValues *boxLayerInfo, FILE *llogfp);
void Scale_Detritus_Mortality(MSEBoxModel *bm, BoxLayerValues *boxLayerInfo);

/**
 * \brief Call the process function for the given group.
 *
 *
 */
static void Call_Group_Process_Function(MSEBoxModel *bm, FILE *llogfp, HABITAT_TYPES habitat, int guild, int cohort, BoxLayerValues *boxLayerInfo){
	switch (FunctGroupArray[guild].groupType) {

	case NUM_GROUP_TYPES:
		/* Ignore - added to make sure we cover all groups */
		break;
	case BIRD:
	case MAMMAL:
	case SHARK:
	case FISH:
    case FISH_INVERT:
		/* ignore the vertebrates (i.e. fully explicit age structured groups) */
		break;
	case DINOFLAG:
	case ICE_MIXOTROPHS:
		Dinoflag_Process(bm, llogfp, habitat, guild, cohort, boxLayerInfo);
		break;
	case LG_PHY:
	case SM_PHY:
	case PHYTOBEN:
	case SEAGRASS:
	case MICROPHTYBENTHOS:
	case TURF:
	case ICE_DIATOMS:
		Phytoplankton_Process(bm, llogfp, habitat, guild, cohort, boxLayerInfo);
		break;
	case SM_INF:
	case LG_INF:
	case SM_ZOO:
	case MED_ZOO:
	case LG_ZOO:
	case CEP:
	case PWN:
	case ICE_ZOOBIOTA:
		Invert_Consumers_Process(bm, llogfp, habitat, guild, cohort, boxLayerInfo);
		break;
	case CORAL:
    case SPONGE:
		Coral_Process(bm, llogfp, habitat, guild, cohort, boxLayerInfo);
		break;
	case PL_BACT:
		Pelagic_Bacteria_Process(bm, llogfp, habitat, guild, cohort, boxLayerInfo);
		break;
	case ICE_BACT:
		Ice_Bacteria_Process(bm, llogfp, habitat, guild, cohort, boxLayerInfo);
		break;
	case SED_BACT:
		Sediment_Bacterica_Process(bm, llogfp, habitat, guild, cohort, boxLayerInfo);
		break;
	case SED_EP_FF:
	case MOB_EP_OTHER:
		Epibenthic_Invert_Process(bm, llogfp, habitat, guild, cohort, boxLayerInfo);
		break;
	case SED_EP_OTHER:
		Sediment_Epi_Other_Process(bm, llogfp, habitat, guild, cohort, boxLayerInfo);
		break;
	case LAB_DET:
		Labile_Detritus_Process(bm, llogfp, habitat, guild, cohort, boxLayerInfo);
		break;
	case REF_DET:
		Refractory_Detritus_Process(bm, llogfp, habitat, guild, cohort, boxLayerInfo);
		break;
	case CARRION:
		Carrion_Process(bm, llogfp, habitat, guild, cohort, boxLayerInfo);
		break;
	}
}

/*************************************************************************//*
 *  This module describes the local rate of change of Tracers due to
 *  biogeochemical processes in the water column.
 *  This routine is the ecological model for water column cell only.
 *
 *  Inputs:
 *  *Pool: pointer to a vector of initial variable values,
 *
 *  Outputs:
 *      *TracerFlux: pointer to a vector of derivative values of variables,
 *      *Yf: pointer to a vector of flux values
 *
 *  All model parameters are global variables of this subroutine.
 *
 */
void Water_Column_Box(MSEBoxModel *bm, double dtsz, BoxLayerValues *boxLayerInfo, FILE *llogfp) {

	/** Define local variables and data **/
	double totSp;
	/* Define biomass names */
	double DON, NH, NO, DR, DL;

	/* Define nutrient and primary producer names */
	double totalBiomass;
	long double grazeEaten;

	/* Define ecolofical indicators */
	double PDratio = 0, FVFPratio = 0, DivHere = 0, PDdenom = 0, PDnum = 0, FVFPnum = 0, FVFPdenom = 0;

	/* Define pool, flux and flag names */
	double wcFlux, wcFlux1, wcFlux2, wcFlux3, wcFishing;

	int guild, cohort, kij, flag_sp, nf, present_here, spdem, fished_chrt, flagimposecatch, spplank, ct;

    double sp_scalar = 0.0, Wgt = 1.0;

	int max_flux_id = -1;
	int flux_model_id = -1;
	int flux_sp = -1;
	int fluxnum = -1;
	int fgIndex;
	HABITAT_TYPES habitat_type = WC;
	int index;
	int isGlobal;
	double mortality_scalar = 1.0;
    double tot_dis, this_bio, realism_scalar;
    int i;

    if (verbose > 1) {
        printf("Water_Column_Box for box %d-%d\n", bm->current_box, bm->current_layer);
    }

    // Find local region
    //int reg_id = 0;
    //if(bm->useRBCTiers)
    //    reg_id = bm->regID[bm->current_box];
    
    boxLayerInfo->PB_DL = 0.0;
	boxLayerInfo->PB_DR = 0.0;
	boxLayerInfo->BB_DL = 0.0;
	boxLayerInfo->BB_DR = 0.0;

    boxLayerInfo->DLscale = 0.0;
	boxLayerInfo->DRscale = 0.0;
	boxLayerInfo->DCscale = 0.0;

    boxLayerInfo->sDLscale = 0.0;
	boxLayerInfo->sDRscale = 0.0;
	boxLayerInfo->sDCscale = 0.0;

    /*
	if ((verbose > 1) || ((bm->debug == debug_biology_process) && (bm->dayt >= bm->checkstart) && (bm->dayt < bm->checkstop)))
		fprintf(llogfp, "\nWater column processes running now (it_count = %d), t = %e\n", it_count, bm->dayt);
    */
    
    /* Initialise the arrays */
	Reset_Arrays(bm, WC, it_count, boxLayerInfo, llogfp);
    
    /* If we are tracking contaminants then work out q10 type corrections now and apply */
	if(bm->track_contaminants){
        Calculate_Species_Contaminant_Effects(bm, bm->current_box, bm->current_layer, dtsz, WC);
		Calculate_Contaminant_Q10_Corrections(bm, boxLayerInfo, WC);
		Apply_Q10_Corrections(bm);
	}

	/* Initialise diagnostic tracer */
	if (it_count == 1) {
		boxLayerInfo->localWCTracers[DiagNGain_i] = 0;
		boxLayerInfo->localWCTracers[DiagNLoss_i] = 0;
		boxLayerInfo->localWCTracers[DiagNFlux_i] = 0;
	}

    /*******************************************************************************
	 Get initial values for fish vectors
	 */
	Set_VERTinfo(bm, boxLayerInfo->localWCTracers, llogfp);

	if (bm->debug && ((bm->debug > debug_biom) && (bm->current_box == bm->checkbox || bm->checkbox > bm->nbox) && (bm->which_check < bm->K_num_tot_sp)
			&& FunctGroupArray[bm->which_check].isVertebrate == TRUE && (bm->dayt > bm->checkstart))) {
		fprintf(llogfp, "day: %e, box: %d, wclayer %d, ", bm->dayt, bm->current_box, bm->current_layer);
		for (cohort = 0; cohort < FunctGroupArray[bm->which_check].numCohortsXnumGenes; cohort++) {
			fprintf(llogfp, "vinfoSN%d %.20e vinfoRN%d %.20e vinfoDEN-%d %.20e\n", cohort, VERTinfo[bm->which_check][cohort][SN_id], cohort,
					VERTinfo[bm->which_check][cohort][RN_id], cohort, VERTinfo[bm->which_check][cohort][DEN_id]);
		}
	}
    
    /**
	 Transfer tracer values from array argument to local variables
	 (see Table above):
	 **/
    Util_Init_1D_Double(initialBiomass, bm->ntracer, 0.0);
    for (i = 0; i < bm->ntracer; i++) {
		initialBiomass[i] = boxLayerInfo->localWCTracers[i];
    }

	DL = boxLayerInfo->localWCTracers[FunctGroupArray[LabDetIndex].totNTracers[0]];
	DR = boxLayerInfo->localWCTracers[FunctGroupArray[RefDetIndex].totNTracers[0]];

	DON = boxLayerInfo->localWCTracers[DON_i];
	NH = boxLayerInfo->localWCTracers[NH3_i];
	NO = boxLayerInfo->localWCTracers[NO3_i];

	/* Initialise the invert properties */
	Init_Invert_Properties(bm);
    
	for (guild = 0; guild < bm->K_num_tot_sp; guild++) {
		if(FunctGroupArray[guild].speciesParams[flag_id] == TRUE){
			if (FunctGroupArray[guild].isVertebrate == FALSE && (FunctGroupArray[guild].habitatType == WC || FunctGroupArray[guild].habitatType == SED)
								&& FunctGroupArray[guild].isPrimaryProducer == TRUE) {
				if (FunctGroupArray[guild].Light_Adaptn_TracerID > 0)
					FunctGroupArray[guild].SP_IRR = boxLayerInfo->localWCTracers[FunctGroupArray[guild].Light_Adaptn_TracerID];
				else
					FunctGroupArray[guild].SP_IRR = 0;

			}
		}
	}

    /**
	 Construct PREYinfo vector (for use in Eat()
	 The PreyInfo array stores the biomass of each invertebrate group.
	 **/
	Construct_Prey_Info(bm, llogfp, boxLayerInfo, habitat_type);

	/* Check that they prey values are finite */
	Check_Prey_Is_Finite(bm, llogfp, habitat_type);

	/***
	 Determine detritus availability scaling
	 ***/
	boxLayerInfo->DLscale = min(1,k_refDL/(DL+small_num));
	boxLayerInfo->DRscale = min(1,k_refDR/(DR+small_num));
	boxLayerInfo->DCscale = 1.0;

	PREYinfo[RefDetIndex][0][WC] = DR * boxLayerInfo->DRscale;
	PREYinfo[LabDetIndex][0][WC] = DL * boxLayerInfo->DLscale;
    
    /********************** FISHERIES PROCESSES ********************************/
	/***
	 Fishing and induced incidental mortality on fish, invertebrates and benthic primary producers.
	 This is done first so that animals can steal from the catch (if appropriate).
	 ***/
    
	if (bm->flag_fisheries_on && (it_count == 1)) {

        if(bm->flagdepend_dis){
            // Zero bm->DependDiscardsTot array - for density dependent discard calculation
            for (fgIndex = 0; fgIndex < bm->K_num_tot_sp; fgIndex++) {
                for(nf=0; nf<bm->K_num_fisheries; nf++){
                    for(cohort=0; cohort<FunctGroupArray[fgIndex].numCohortsXnumGenes; cohort++){
                        bm->DependDiscardsTot[nf][fgIndex][cohort] = 0.0;
                    }
				}
			}
		}

		/**
		 Fisheries and bycatch (other groups are dealt with in Epibenthic_Box().
		 **/
		for (fgIndex = 0; fgIndex < bm->K_num_tot_sp; fgIndex++) {
			if (FunctGroupArray[fgIndex].speciesParams[flag_id]) {
				flagimposecatch = (int) (FunctGroupArray[fgIndex].speciesParams[flagimpose_id]);

				if (FunctGroupArray[fgIndex].isImpacted == TRUE && FunctGroupArray[fgIndex].habitatType == WC) {
					switch (FunctGroupArray[fgIndex].groupAgeType) {
					case AGE_STRUCTURED:
						// Note reverse order of the calculations - in line with older code bases
						for (cohort = (FunctGroupArray[fgIndex].numCohortsXnumGenes) - 1; cohort >= 0; cohort--) {
							if (flagimposecatch || ((VERTinfo[fgIndex][cohort][SN_id] > bm->min_pool) && (VERTinfo[fgIndex][cohort][DEN_id] > bm->min_dens))) {
								Calculate_Catch(bm, boxLayerInfo, llogfp, fgIndex, cohort, VERTinfo[fgIndex][cohort][SN_id], VERTinfo[fgIndex][cohort][RN_id],
										VERTinfo[fgIndex][cohort][DEN_id], 1.0, 1.0);
							}
						}
						break;
					case BIOMASS:	/* intentional follow thur */
					case AGE_STRUCTURED_BIOMASS:
						for (cohort = 0; cohort < FunctGroupArray[fgIndex].numCohortsXnumGenes; cohort++) {
							if (flagimposecatch || initialBiomass[FunctGroupArray[fgIndex].totNTracers[cohort]] > bm->min_pool) {
								Calculate_Catch(bm, boxLayerInfo, llogfp, fgIndex, cohort, initialBiomass[FunctGroupArray[fgIndex].totNTracers[cohort]], 0.0,
										1.0, 1.0, 1.0);
							}
						}
						break;
					}
				}
			}
		}

        // Update dead and discards
        if(bm->flagdepend_dis){
			for(guild=0; guild<bm->K_num_tot_sp; guild++){
				flag_sp = (int)(FunctGroupArray[guild].speciesParams[flag_id]);
				if(flag_sp){
					sp_scalar = 1.0;
					for(cohort=0; cohort<FunctGroupArray[guild].numCohortsXnumGenes; cohort++){
						/* Determine local biomass for vertebrates (invertebrates assume SC entry = Biomass */
						if (FunctGroupArray[guild].groupAgeType == AGE_STRUCTURED){
							Wgt = VERTinfo[guild][cohort][SN_id] + VERTinfo[guild][cohort][RN_id];
							if(Wgt < small_num)
								Wgt = small_num;     /* To avoid divide by zero problems */
                            
                            sp_scalar = bm->cell_vol / Wgt;  // Used to have bm->dt in denominator, but that doesn't make sense as already per s
                            //sp_scalar = 1 / (Wgt * bm->dt);  // Needs to be * bm->cell_vol as was calculated in terms of mg m-3 and need to get to nums
						}
                        
                        /* Reality check */
                        // Find total to be discarded
                        tot_dis = 0.0;
                        for(nf=0; nf<bm->K_num_fisheries; nf++){
                            tot_dis += bm->DependDiscardsTot[nf][guild][cohort] * bm->dt;
                        }
                       
                        if(!tot_dis)
                            continue;
                        
                        // See if taking more biomass than present and fix accordingly
                        realism_scalar = 1.0;
                        if (FunctGroupArray[guild].habitatType != EPIFAUNA) {
                            if (FunctGroupArray[guild].groupAgeType == AGE_STRUCTURED){
                                this_bio = Wgt * VERTinfo[guild][cohort][DEN_id];
                            } else {
                                this_bio = initialBiomass[FunctGroupArray[guild].totNTracers[cohort]];
                            }
                            if (tot_dis > this_bio)
                                realism_scalar = this_bio / (tot_dis + small_num);
                        } else {
                            realism_scalar = 0.0;
                            continue;
                        }

						for(nf=0; nf<bm->K_num_fisheries; nf++){
                            if(bm->DependDiscardsTot[nf][guild][cohort]) {
                                bm->DependDiscardsTot[nf][guild][cohort] *= realism_scalar;
                                
                                bm->Discards[bm->current_box][guild][nf] += (bm->DependDiscardsTot[nf][guild][cohort] * bm->dt * bm->cell_vol);
                                if(FunctGroupArray[guild].groupAgeType == AGE_STRUCTURED){
                                    FunctGroupArray[guild].SizeNumDiscard[cohort][nf][bm->current_box] += (bm->DependDiscardsTot[nf][guild][cohort] * bm->dt * bm->cell_vol);
                                    bm->FCdiscard[guild][cohort] += (bm->DependDiscardsTot[nf][guild][cohort] * bm->dt * bm->cell_vol);
                                }
                                boxLayerInfo->DetritusProd[WC][DCdet_id] += bm->DependDiscardsTot[nf][guild][cohort];

                                // Possible bug - will be done bm->K_num_fisheries times. - done in original code.
                                FunctGroupArray[guild].dead[cohort] += (bm->DependDiscardsTot[nf][guild][cohort] * sp_scalar);
                                
                                // Update Catchsum - moved here to remove an additional loop and so it has the reality check in place
                                CatchSum[guild][tscodiscard_id] += (bm->DependDiscardsTot[nf][guild][cohort] * bm->cell_vol * bm->dt);

                                /**
                                //if (guild == bm->which_check) {
                                if ((guild == 33) || ((guild > 8) && (guild < 12))) {
                                    fprintf(llogfp,"Time: %e watercol check box%d-%d %s-%d new discards increment: %e due to dependdiscards by %s so discard now: %e dead: %e sp_scalar: %e, Wgt: %e, CatchSum: %e realism_scalar: %e\n", bm->dayt, bm->current_box, bm->current_layer, FunctGroupArray[guild].groupCode, cohort, (bm->DependDiscardsTot[nf][guild][cohort] * bm->dt * bm->cell_vol), FisheryArray[nf].fisheryCode, bm->Discards[bm->current_box][guild][nf], FunctGroupArray[guild].dead[cohort], sp_scalar, Wgt, CatchSum[guild][tscodiscard_id], realism_scalar);
                                
                                    //fprintf(llogfp,"Time: %e box%d-%d %s-%d dead: %e due to dependdiscards by %s sp_scalar: %e so dead now: %e as Wgt: %e\n", bm->dayt, bm->current_box, bm->current_layer, FunctGroupArray[guild].groupCode, cohort, bm->DependDiscardsTot[nf][guild][cohort], FisheryArray[nf].fisheryCode, sp_scalar, FunctGroupArray[guild].dead[cohort], Wgt);
                                }
                                **/
                            }
						}
                        
                        // Update the global dead tracker so not missing the discards component
                        if ((FunctGroupArray[guild].isOncePerDt == FALSE) || (FunctGroupArray[guild].diagTol < 2))
                            FunctGroupArray[guild].deadGlobal[cohort] = FunctGroupArray[guild].dead[cohort];

					}
				}
 			}
        }
	}
    
    /* Do scaling of dead due to fishing in one go now */
	if(bm->scale_all_mortality == TRUE){
		for (guild = 0; guild < bm->K_num_tot_sp; guild++) {
			for(cohort=0; cohort<FunctGroupArray[guild].numCohorts; cohort++){
				/* If there is a scalar to apply then grab it now */
				mortality_scalar = Ecology_Get_Mortality_Scalar(bm, guild, cohort);

				FunctGroupArray[guild].dead[cohort] = FunctGroupArray[guild].dead[cohort] * mortality_scalar;
			}
		}
	}
    
    /********************** ECOLOGICAL INDICATORS *******************************/

	if (it_count == 1) {
		PDratio = 0;
		FVFPratio = 0;
		DivHere = 0;
		PDdenom = 0;
		PDnum = 0;
		FVFPnum = 0;
		FVFPdenom = 0;
		for (guild = 0; guild < bm->K_num_tot_sp; guild++) {
			if (FunctGroupArray[guild].isVertebrate == TRUE) {

				present_here = 0;
				flag_sp = (int) (FunctGroupArray[guild].speciesParams[flag_id]);
				spdem = (int) (FunctGroupArray[guild].speciesParams[flagdem_id]);
				spplank = (int) (FunctGroupArray[guild].speciesParams[flagplankfish_id]);

				for (cohort = 0; cohort < FunctGroupArray[guild].numCohortsXnumGenes; cohort++) {
					if (flag_sp && (VERTinfo[guild][cohort][SN_id] > bm->min_pool) && (VERTinfo[guild][cohort][DEN_id] > bm->min_dens)) {
						/* Pelagic / Demersal Biomass ratio */
						if (spdem) {
							/* Demersal fish */
							PDdenom += (VERTinfo[guild][cohort][SN_id] + VERTinfo[guild][cohort][RN_id]) * (VERTinfo[guild][cohort][DEN_id]);
						} else {
							/* Pelagic fish */
							PDnum += (VERTinfo[guild][cohort][SN_id] + VERTinfo[guild][cohort][RN_id]) * (VERTinfo[guild][cohort][DEN_id]);

							/* Piscivore / Planktivore ratio */
							if (spplank){
								/* Planktivorous fish biomass */
								FVFPnum += (VERTinfo[guild][cohort][SN_id] + VERTinfo[guild][cohort][RN_id]) * (VERTinfo[guild][cohort][DEN_id]);

							} else if (FunctGroupArray[guild].groupType == FISH) {
								/* Piscivorous fish biomass */
								FVFPdenom += (VERTinfo[guild][cohort][SN_id] + VERTinfo[guild][cohort][RN_id]) * (VERTinfo[guild][cohort][DEN_id]);
							}
						}
						present_here = 1;
					}
				}
				DivHere += present_here;
			}
		}

		PDratio = PDnum / (PDdenom + small_num);

		FVFPratio = FVFPnum / (FVFPdenom + small_num);


		/* Local diversity proxy */
		for (fgIndex = 0; fgIndex < bm->K_num_tot_sp; fgIndex++) {
			if (FunctGroupArray[fgIndex].speciesParams[flag_id] == TRUE && FunctGroupArray[fgIndex].isVertebrate == FALSE) {
				if (FunctGroupArray[fgIndex].habitatType == WC && FunctGroupArray[fgIndex].groupType != LAB_DET && FunctGroupArray[fgIndex].groupType
						!= REF_DET && FunctGroupArray[fgIndex].groupType != CARRION) {
					if (FunctGroupArray[fgIndex].groupAgeType == AGE_STRUCTURED_BIOMASS) {
						for (cohort = 0; cohort < FunctGroupArray[fgIndex].numCohortsXnumGenes; cohort++) {
							if ((int) FunctGroupArray[fgIndex].speciesParams[flag_id] == TRUE && initialBiomass[FunctGroupArray[fgIndex].totNTracers[cohort]]
									> bm->min_pool)
								DivHere++;
						}
					} else {
						if ((int) FunctGroupArray[fgIndex].speciesParams[flag_id] == TRUE && initialBiomass[FunctGroupArray[fgIndex].totNTracers[0]]
								> bm->min_pool)
							DivHere++;
					}
				}
			}
		}
	}
    
    /********************** ECOLOGICAL PROCESSES ********************************/
	/**
	 Pelagic Bacteria partitioning
	 **/
	Calculate_PelagicBact_Scale(bm, boxLayerInfo);

	/**
	 Phytoplankton growth:
	 The specific growth rate mu is the product of a maximum growth rate, mum, a
	 light-limitation factor hI, and a nutrient limitation factor, hN. The
	 formulation is identical for large and small phytoplankton, and suspended
	 microphytobenthos, but the parameters vary.
	 **/
	boxLayerInfo->DIN = NO + NH;

	if (!boxLayerInfo->DIN) {
		boxLayerInfo->DIN = bm->min_pool;
		fprintf(llogfp, "Time: %e, box: %d, layer: %d had to reset DIN in watercolumn to minpool as was = 0\n", bm->dayt, bm->current_box, bm->current_layer);
		warn("Time: %e, box: %d, layer: %d had to reset DIN in watercolumn to minpool as was = 0\n", bm->dayt, bm->current_box, bm->current_layer);
	}

	/**
	 Nutrient limitation allows for N or Si limitation, also calculate uptake of NO3,
	 NH4 and Si here. Disn't put day flag in phytoplankton as light limitation will stop growth.

	 Note that only lysis assumed in the water column, mortality (rather than lysis) occurs in
	 the sediment
	 **/


    for (guild = 0; guild < bm->K_num_tot_sp; guild++) {

		flag_sp = (int) (FunctGroupArray[guild].speciesParams[flag_id]);

		if (flag_sp && (FunctGroupArray[guild].isOncePerDt == FALSE || (it_count == 1 && FunctGroupArray[guild].isOncePerDt == TRUE))) {
			switch (FunctGroupArray[guild].groupAgeType) {

			case BIOMASS:
			case AGE_STRUCTURED_BIOMASS:
				if (FunctGroupArray[guild].habitatCoeffs[WC] > 0 ){

					boxLayerInfo->DebugInfo[guild][habitat_type][DiagnostNH_id] = 0;
					boxLayerInfo->DebugInfo[guild][habitat_type][DiagnostDL_id] = 0;
					boxLayerInfo->DebugInfo[guild][habitat_type][DiagnostDR_id] = 0;
					boxLayerInfo->DebugInfo[guild][habitat_type][DiagnostNHsed_id] = 0;
					boxLayerInfo->DebugInfo[guild][habitat_type][DiagnostDLsed_id] = 0;
					boxLayerInfo->DebugInfo[guild][habitat_type][DiagnostDRsed_id] = 0;

					if(FunctGroupArray[guild].groupType == CEP || FunctGroupArray[guild].groupType == PWN){
						if (bm->current_layer != 0 || waterboundary) {
							for (cohort = 0; cohort < FunctGroupArray[guild].numCohortsXnumGenes; cohort++){
								Call_Group_Process_Function(bm, llogfp, WC, guild, cohort, boxLayerInfo);
							}
						}
					} else {
						for (cohort = 0; cohort < FunctGroupArray[guild].numCohortsXnumGenes; cohort++) {
							Call_Group_Process_Function(bm, llogfp, WC, guild, cohort, boxLayerInfo);
						}
					}
				}
				break;
			case AGE_STRUCTURED:
				/* Vertebrates*/
				/*
				 Note that for those groups that can be up in the water column or down in the demersal layer
				 they will get called here if in water column layers, but down in the Epibenthic_Box() when in
				 the demersal layer.

				 Pelagic fish - planktivores are assumed to avoid anoxic waters, piscivores less so
				 (as may follow food down there).

				 All vertbrate activities now handled via Do_Vertebrate_Living() which loops over the
				 cohorts calling Vertebrate_Activities()
				 **/

					totSp = Do_Vertebrate_Living(bm, llogfp, guild, WC, boxLayerInfo, 0.0, 0.0, 0.0, PREYinfo, GRAZEinfo, CATCHGRAZEinfo, VERTinfo);
					boxLayerInfo->localWCTracers[FunctGroupArray[guild].totNTracers[0]] = totSp;

				break;
			}

			//fprintf(bm->logFile, "After %s, boxLayerInfo->NutsProd[WC][NH_id] = %e\n", FunctGroupArray[guild].groupCode, boxLayerInfo->NutsProd[WC][NH_id]);
		}
	}
	
	/**
	 DON breaks down at a fixed rate r_DON, so
	 **/
	boxLayerInfo->DONremin = r_DON * DON;

	if(bm->track_atomic_ratio){
		Calculate_Phosphorus(bm, boxLayerInfo, WC);
		Calculate_Carbon(bm, boxLayerInfo, WC);
	}

	if(bm->track_contaminants){
        Change_Contaminant_Levels(bm, boxLayerInfo, WC, dtsz, 0);
	}
    
    /** Benthic returns needed for open lower boundary in offshore (deep water) boxes
	 would be needed if model runs were on scale of 1000 years otherwise too negligible
	 to worry about (on advice of John Parslow) **/

	/*******************************************************************************************
	 Reconcile global consumption, and detritus and nutrient flows for adaptive timestep
	 */
	Reconcile_Global_Values(bm, boxLayerInfo, WC, llogfp);
    
	/*******************************************************************************************
	 The equations for rate of change are therefore:
	 */
	for (guild = 0; guild < bm->K_num_tot_sp; guild++) {
		if((int) (FunctGroupArray[guild].speciesParams[flag_id]) == TRUE){
			isGlobal = (FunctGroupArray[guild].diagTol == 2 && it_count == 1);
            
			if(FunctGroupArray[guild].isDetritus == TRUE)
				continue;

			switch (FunctGroupArray[guild].groupAgeType) {
			case AGE_STRUCTURED:

				/* the rest of the vertebrate tracers are updated in Update_Vertebrate_Tracers()*/
				/* For bottom water column layer save this info in Epibenthic_Box() */
				if (it_count == 1) {
					if (bm->current_layer) {
					for (cohort = 0; cohort < FunctGroupArray[guild].numCohortsXnumGenes; cohort++) {
							boxLayerInfo->localDiagFlux[FunctGroupArray[guild].EatTracers[cohort]] += (double)FunctGroupArray[guild].GrazeLive[cohort];
							boxLayerInfo->localDiagFlux[FunctGroupArray[guild].GrowthTracers[cohort]] += FunctGroupArray[guild].grow[cohort][SN_id]
									+ FunctGroupArray[guild].grow[cohort][RN_id];
						}
					}
				}
				break;
			case AGE_STRUCTURED_BIOMASS:
				/**
				 Age structured biomass groups - prawns and ceps
				 **/
				if (it_count == 1) {
					/* Get the total biomss */
					totalBiomass = 0.0;
					for (cohort = 0; cohort < FunctGroupArray[guild].numCohortsXnumGenes; cohort++) {
						totalBiomass += boxLayerInfo->localWCTracers[FunctGroupArray[guild].totNTracers[cohort]];
					}
					totalBiomass += small_num;

					boxLayerInfo->DebugFluxInfo[guild][WC][gain_id] = 0;
					boxLayerInfo->DebugFluxInfo[guild][WC][loss_id] = 0;

					for (cohort = 0; cohort < FunctGroupArray[guild].numCohortsXnumGenes; cohort++) {

						if(bm->flag_olddiet){
							/* Now divide the prey eaten values between the cohorts based on the initial biomass proportion */
							grazeEaten = 0.0;
							for (ct = 0; ct < FunctGroupArray[guild].numCohortsXnumGenes; ct++){
								grazeEaten += FunctGroupArray[guild].preyEaten[ct][habitat_type];
							}
							grazeEaten *= (boxLayerInfo->localWCTracers[FunctGroupArray[guild].totNTracers[cohort]] / totalBiomass);
						} else {
							grazeEaten = FunctGroupArray[guild].preyEaten[cohort][habitat_type];
						}

						boxLayerInfo->localWCFlux[FunctGroupArray[guild].totNTracers[cohort]] = FunctGroupArray[guild].growth[cohort] - FunctGroupArray[guild].mortality[cohort] - FunctGroupArray[guild].dead[cohort] - (double)grazeEaten;

						if(bm->track_atomic_ratio == TRUE){
							/* Predation and growth due to predation is handled seperately */
							Loose_Element(bm, boxLayerInfo, WC, guild, cohort,
									(FunctGroupArray[guild].dead[cohort] + FunctGroupArray[guild].mortality[cohort]),
									WC, isGlobal);
						}

						/* Do an add to take into account the cohorts */
						boxLayerInfo->DebugFluxInfo[guild][WC][gain_id] += FunctGroupArray[guild].growth[cohort];
						boxLayerInfo->DebugFluxInfo[guild][WC][loss_id] += FunctGroupArray[guild].mortality[cohort]
								+ FunctGroupArray[guild].dead[cohort] + (double)grazeEaten;

						/* Save growth and grazing values */
						if (FunctGroupArray[guild].isProducer) {
							boxLayerInfo->localDiagFlux[FunctGroupArray[guild].prodnTracers[0]] += FunctGroupArray[guild].growth[cohort];
						}
						if (FunctGroupArray[guild].isGrazer){
							boxLayerInfo->localDiagFlux[FunctGroupArray[guild].GrazingTracers[0]] += FunctGroupArray[guild].grazing[cohort];
						}

                        /**
                        if( guild == bm->which_check ){
                            fprintf(llogfp,"day: %f box%d-%d, %s-%d flux: %.20e grow: %.20e mort: %.20e dead: %.20e eaten: %Le preyEaten: %Le scalar: %.20e\n",
                                bm->dayt, bm->current_box, bm->current_layer, FunctGroupArray[guild].groupCode, cohort,
                                boxLayerInfo->localWCFlux[FunctGroupArray[guild].totNTracers[cohort]],
                                FunctGroupArray[guild].growth[cohort], FunctGroupArray[guild].mortality[cohort],
                                FunctGroupArray[guild].dead[cohort], grazeEaten, FunctGroupArray[guild].preyEaten[0][habitat_type],
                                boxLayerInfo->localWCTracers[FunctGroupArray[guild].totNTracers[cohort]] / totalBiomass);
                        }
                        **/

#ifdef BROKER_LINK_ENABLED
						/* Add the linkage flux if present */
						if(do_BrokerLinkage){
							boxLayerInfo->localWCFlux[FunctGroupArray[guild].totNTracers[cohort]] += -bm->linkageInterface->linkageWCMortality[bm->current_box][bm->current_layer][guild][cohort];
							boxLayerInfo->DebugFluxInfo[guild][habitat_type][loss_id] += bm->linkageInterface->linkageWCMortality[bm->current_box][bm->current_layer][guild][cohort];
						}
#endif
					}
				} else {
					boxLayerInfo->DebugFluxInfo[guild][WC][gain_id] = 0.0;
					boxLayerInfo->DebugFluxInfo[guild][WC][loss_id] = 0.0;
				}
				break;
			case BIOMASS:
				/*
				 * tracerFlux = growth - lysis - mortality - dead - preyEaten
				 *
				 */
				if (FunctGroupArray[guild].habitatCoeffs[WC] > 0) {

					if (FunctGroupArray[guild].isOncePerDt == FALSE || (it_count == 1 && FunctGroupArray[guild].isOncePerDt == TRUE)) {
						double dead = 0.0;


						cohort = 0;


						/* Get the dead due to fishing - If this is a finer scale group - use the global values else use the values that were calculated in this time step. */
						if (FunctGroupArray[guild].isOncePerDt == FALSE)
							dead = FunctGroupArray[guild].deadGlobal[0];
						else
							dead = FunctGroupArray[guild].dead[0];
                        
						boxLayerInfo->localWCFlux[FunctGroupArray[guild].totNTracers[0]] =
								FunctGroupArray[guild].growth[0]
								- FunctGroupArray[guild].lysis[0]
								- FunctGroupArray[guild].mortality[0]
								- dead
								- (double)FunctGroupArray[guild].preyEaten[0][habitat_type];

						/**
						if(it_count == 1){
                            if(guild == bm->which_check && fabs(boxLayerInfo->localWCFlux[FunctGroupArray[guild].totNTracers[cohort]]) > 0){
								fprintf(bm->logFile, "Group %s - Box %d, layer %d, flux = %e\n", FunctGroupArray[guild].groupCode, bm->current_box, bm->current_layer, boxLayerInfo->localWCFlux[FunctGroupArray[guild].totNTracers[cohort]]);
								fprintf(bm->logFile, "Growth = %e, mortality = %e, dead due to fishing = %e, eaten = %Le\n", FunctGroupArray[guild].growth[cohort], FunctGroupArray[guild].mortality[cohort], dead, FunctGroupArray[guild].preyEaten[0][habitat_type]);
							}
						}
                        **/

						if(bm->track_atomic_ratio == TRUE){

							/* Predation and growth due to is handled separately */
							Loose_Element(bm, boxLayerInfo, WC, guild, cohort,
									(FunctGroupArray[guild].lysis[0] + FunctGroupArray[guild].mortality[0] + dead),
									WC, isGlobal);

						}

						/**
						if((guild == bm->which_check) && (bm->current_box == bm->checkbox)){
							fprintf(llogfp,"Time: %e box%d-%d %s biom: %.20e, flux: %.20e, growth: %.20e, lysis: %.20e, mort: %.20e, dead: %.20e, eaten: %Le\n",
									bm->dayt, bm->current_box, bm->current_layer, FunctGroupArray[guild].groupCode,
									boxLayerInfo->localWCTracers[FunctGroupArray[guild].totNTracers[cohort]],
									boxLayerInfo->localWCFlux[FunctGroupArray[guild].totNTracers[0]],
									FunctGroupArray[guild].growth[0], FunctGroupArray[guild].lysis[0],
									FunctGroupArray[guild].mortality[0], dead, FunctGroupArray[guild].preyEaten[0][habitat_type]);
						}
						**/

						boxLayerInfo->DebugFluxInfo[guild][WC][gain_id] = FunctGroupArray[guild].growth[0];
						boxLayerInfo->DebugFluxInfo[guild][WC][loss_id] = FunctGroupArray[guild].mortality[0]
								+ FunctGroupArray[guild].lysis[0] + dead + (double)FunctGroupArray[guild].preyEaten[0][habitat_type];

						if (FunctGroupArray[guild].isProducer) {
							boxLayerInfo->localDiagFlux[FunctGroupArray[guild].prodnTracers[0]] += FunctGroupArray[guild].growth[0];
						}

						if (FunctGroupArray[guild].isGrazer) {
							boxLayerInfo->localDiagFlux[FunctGroupArray[guild].GrazingTracers[0]] += FunctGroupArray[guild].uptakeDL
									+ FunctGroupArray[guild].uptakeDR + FunctGroupArray[guild].grazing[0]
									+ FunctGroupArray[guild].maxPhagotrophy;
						}

#ifdef BROKER_LINK_ENABLED
						if(do_BrokerLinkage){
							/* If this is not detritus then subtract the ewe linkage flux */
							if (FunctGroupArray[guild].isDetritus == FALSE) {
								boxLayerInfo->localWCFlux[FunctGroupArray[guild].totNTracers[0]]
										+= -bm->linkageInterface->linkageWCMortality[bm->current_box][bm->current_layer][guild][cohort];

								boxLayerInfo->DebugFluxInfo[guild][WC][loss_id] += bm->linkageInterface->linkageWCMortality[bm->current_box][bm->current_layer][guild][cohort];

							} else {
								/* Else its added to the flux */
								boxLayerInfo->localWCFlux[FunctGroupArray[guild].totNTracers[0]]
										+= +bm->linkageInterface->linkageWCMortality[bm->current_box][bm->current_layer][guild][cohort];

								boxLayerInfo->DebugFluxInfo[guild][WC][gain_id] += bm->linkageInterface->linkageWCMortality[bm->current_box][bm->current_layer][guild][cohort];

							}
						}
#endif
						/* if this group has a second flux update that as well*/
						if (FunctGroupArray[guild].secondNutrientTracerIndex > 0) {
							boxLayerInfo->localWCFlux[FunctGroupArray[guild].secondNutrientTracerIndex]
									= boxLayerInfo->localWCFlux[FunctGroupArray[guild].totNTracers[0]] * FunctGroupArray[guild].secondNutrient;

						}
					}
				}
				break;
			}
		}
	}

    /* if scaling mortality values handle the detritus groups now - just do it in one go */
	if(bm->scale_all_mortality == TRUE){
		Scale_Detritus_Mortality(bm, boxLayerInfo);
	}
    
	/* These are separate as the order is important to get the same output files */
	/**
	 Labile detritus in the water column
	 **/

	Labile_Detritus_ROC(bm, llogfp, WC, boxLayerInfo);

	/**
	 Refractory detritus in water column
	 **/

	Refractory_Detritus_ROC(bm, llogfp, WC, boxLayerInfo);

	/**
	 *  Nutrients in the water column
	 *
	 **/
	for (index = 0; index < bm->K_num_physiochem; index++) {
		if (PhysioChemArray[index].ROCFunction != NULL) {
			PhysioChemArray[index].ROCFunction(bm, llogfp, WC, index, boxLayerInfo);
			if(bm->ecotest > 1){
				/* Check the tracer values */
				if(!_finite(boxLayerInfo->localWCFlux[*PhysioChemArray[index].tracerIndex])){
					quit("Water_Column_Box - Nutrient %s is infinite in box %d:%d\n", PhysioChemArray[index].name, bm->current_box, bm->current_layer);
				}
			}
		}
	}

	if (it_count == 1) {
		/**
		 Carcasses in the water column
		 **/
		Carrion_ROC(bm, llogfp, WC, boxLayerInfo);


        /**
		 Vertebrates - if in lowest watercolumn layer than this will all actually happen
		 in Epibenthic_Box()
		 **/
        Update_Vertebrate_Tracers(bm, llogfp, WC, boxLayerInfo);

		/** Update overall catch at age distributions with those from the current
		 water column layer
		 **/
		for (guild = 0; guild < bm->K_num_tot_sp; guild++) {
			if (FunctGroupArray[guild].speciesParams[flag_id] == TRUE) {
				for (nf = 0; nf < bm->K_num_fisheries; nf++) {
					for (kij = 0; kij < FunctGroupArray[guild].numCohortsXnumGenes; kij++) {
						bm->FCcaught[guild][kij] += bm->FCcaughttemp[guild][nf][kij];
					}
				}
			}
		}
        
		/*** Update total catch and discards ***/
		if (bm->flag_fisheries_on) {
			Harvest_Update_Total_Catch(bm, boxLayerInfo->localFishTracers, WC, llogfp);
		}

		/**
		 Diagnostic variables and ecological indicators
		 **/
		if (bm->current_layer == (bm->boxes[bm->current_box].nz - 1)) {
			/* If top layer just set global average for this box to the value for this layer */
			boxLayerInfo->localDiagTracers[PelDem_ratio_i] = PDratio;
			boxLayerInfo->localDiagTracers[PiscPlank_ratio_i] = FVFPratio;
			boxLayerInfo->localDiagTracers[DivCount_i] = DivHere;
		} else {
			/* Else increment the global average values */
			boxLayerInfo->localDiagTracers[PelDem_ratio_i] = PDratio / bm->boxes[bm->current_box].nz;
			boxLayerInfo->localDiagTracers[PiscPlank_ratio_i] = FVFPratio / bm->boxes[bm->current_box].nz;
			boxLayerInfo->localDiagTracers[PelDem_ratio_i] = PDratio / bm->boxes[bm->current_box].nz;
			if (DivHere > boxLayerInfo->localDiagTracers[DivCount_i])
				boxLayerInfo->localDiagTracers[DivCount_i] = DivHere;
		}
	}
    
	//TODO: Rewrite to allow for age structured biomass groups.
	/* Update mortality estimates - Start with vertebrates */
	for (guild = 0; guild < bm->K_num_tot_sp; guild++) {
		if(FunctGroupArray[guild].speciesParams[flag_id] == TRUE){
			if (FunctGroupArray[guild].isVertebrate == TRUE) {
				/* For use in pseudo F estimate */
				fished_chrt = (int) (FunctGroupArray[guild].speciesParams[Age95pcntV_id]);
				if (fished_chrt < ((FunctGroupArray[guild].numCohortsXnumGenes) / 2))
					fished_chrt = ((FunctGroupArray[guild].numCohortsXnumGenes)/ 2);

				/* This value will also take into account other forms of mortality - linear + quadratic. Think that
				 * theF value should just be loss due to fishing.
				 * Question is if the M value should include the other forms of mortality
				 * - might need to break the dead into fished and naturalDead.
				 *
				 * */
				bm->calcFnum[guild][current_id] += FunctGroupArray[guild].dead[fished_chrt];

				/* For use in pseudo M estimate */
				bm->calcMnum[guild][current_id] += (double)FunctGroupArray[guild].preyEaten[fished_chrt][WC];

			} else {
				for(cohort = 0; cohort < FunctGroupArray[guild].numCohortsXnumGenes; cohort++){
					//fished_chrt = 0;
		   			bm->calcTrackedMort[guild][0][0][ongoingF_id] += (FunctGroupArray[guild].dead[cohort] * bm->dt);
					bm->calcFnum[guild][current_id] += FunctGroupArray[guild].dead[cohort];

					/* For use in pseudo M estimate */
					bm->calcMnum[guild][current_id] += (FunctGroupArray[guild].mortality[cohort]
					  + (double)FunctGroupArray[guild].preyEaten[cohort][WC] + (double)FunctGroupArray[guild].preyEaten[cohort][SED]
					  + (double)FunctGroupArray[guild].preyEaten[cohort][EPIFAUNA]);

#ifdef BROKER_LINK_ENABLED
					if(do_BrokerLinkage){
						bm->calcMnum[guild][current_id] += bm->linkageInterface->linkageWCMortality[bm->current_box][bm->current_layer][guild][cohort];
					}
#endif
				}
			}
		}
	}

	/* Temperature */
	boxLayerInfo->localWCTracers[Temp_i] = H2Otemp;

	/*Update the nutrient tracers Chl_a, Recyc_N and MPB_PProd*/
	boxLayerInfo->localWCTracers[Chl_a_i] = 0.0;
	boxLayerInfo->localDiagFlux[Recyc_N_i] += RecycledNHglobal + boxLayerInfo->DONremin;

	for (guild = 0; guild < bm->K_num_tot_sp; guild++) {
		if (FunctGroupArray[guild].speciesParams[flag_id] == TRUE) {
			if (FunctGroupArray[guild].isVertebrate == FALSE) {
				/* Chl_a  */
				boxLayerInfo->localWCTracers[Chl_a_i] += FunctGroupArray[guild].chl;

				/* Recyc_N */
				/* only add the smaller groups - the rest are added to the global value */
				if (FunctGroupArray[guild].isOncePerDt == FALSE)
					boxLayerInfo->localDiagFlux[Recyc_N_i] += FunctGroupArray[guild].releaseNH[0];
				boxLayerInfo->localDiagFlux[Recyc_N_i] += FunctGroupArray[guild].remin;
			}
		}
	}


	boxLayerInfo->localWCTracers[Chl_a_i] = boxLayerInfo->localWCTracers[Chl_a_i] / X_CHLN;

	/* MPB_PProd */
	boxLayerInfo->localDiagFlux[Don_Prod_i] += FunctGroupArray[LabDetIndex].solDON + FunctGroupArray[RefDetIndex].solDON
			- boxLayerInfo->DONremin + FunctGroupArray[pelagicBactIndex].prodnDON;

	boxLayerInfo->localWCFlux[Nitrification_i] = FunctGroupArray[pelagicBactIndex].nitrif;


	/**
	 * If we are tracking the atomic tracers - calculate the flux values for this habitat before the arrays are cleared for the new habitat
	 */
	if(bm->track_atomic_ratio == TRUE){
		Calculate_Element_Flux(bm, boxLayerInfo->localWCTracers, boxLayerInfo->localWCFlux, WC, WC);
	}


	/* check diagnostics */
	if (bm->which_check < not_tracking_flux && bm->habitat_check == WC) {

		if (it_count == 1) {
			boxLayerInfo->localWCTracers[DiagNGain_i] = boxLayerInfo->DebugFluxInfo[bm->which_check][bm->habitat_check][gain_id] * dtsz;
			boxLayerInfo->localWCTracers[DiagNLoss_i] = boxLayerInfo->DebugFluxInfo[bm->which_check][bm->habitat_check][loss_id] * dtsz;
			boxLayerInfo->localWCTracers[DiagNFlux_i] = (boxLayerInfo->DebugFluxInfo[bm->which_check][bm->habitat_check][gain_id]
					- boxLayerInfo->DebugFluxInfo[bm->which_check][bm->habitat_check][loss_id]) * dtsz;

		} else {
			boxLayerInfo->localWCTracers[DiagNGain_i] += boxLayerInfo->DebugFluxInfo[bm->which_check][bm->habitat_check][gain_id] * dtsz;
			boxLayerInfo->localWCTracers[DiagNLoss_i] += boxLayerInfo->DebugFluxInfo[bm->which_check][bm->habitat_check][loss_id] * dtsz;
			boxLayerInfo->localWCTracers[DiagNFlux_i] += (boxLayerInfo->DebugFluxInfo[bm->which_check][bm->habitat_check][gain_id]
					- boxLayerInfo->DebugFluxInfo[bm->which_check][bm->habitat_check][loss_id]) * dtsz;
		}
	}
    
	if ((bm->checkNH || bm->checkDL || bm->checkDR) && ((bm->dayt >= bm->checkstart) && ((bm->checkbox > bm->nbox) || (bm->current_box == bm->checkbox))))
		Call_Diagnostics(bm, boxLayerInfo, WC, llogfp, 0);

	if (bm->checkbiom && ((bm->checkbox > bm->nbox) || (bm->current_box == bm->checkbox)) && (bm->dayt >= bm->checkstart)) {
		Ecology_Calculate_Total_Abundance(bm, bm->dt, 1, llogfp);
	}

	/* test flux balance */
	wcFlux = 0;
	wcFlux1 = 0;
	wcFlux2 = 0;
	wcFlux3 = 0;
	wcFishing = 0; /* Amount removed from the system by fishing */
	if (bm->ecotest > 1) {

		wcFlux1 = boxLayerInfo->localWCFlux[NH3_i] + boxLayerInfo->localWCFlux[NO3_i] + boxLayerInfo->localWCFlux[DON_i];
        
		for (guild = 0; guild < bm->K_num_tot_sp; guild++) {
			if(FunctGroupArray[guild].speciesParams[flag_id] == TRUE){
				if (FunctGroupArray[guild].habitatCoeffs[WC] > 0) {
					switch (FunctGroupArray[guild].groupAgeType) {
					case AGE_STRUCTURED_BIOMASS:
					case BIOMASS:
						if (FunctGroupArray[guild].groupType == LG_ZOO || FunctGroupArray[guild].groupType == MED_ZOO || FunctGroupArray[guild].groupType == CEP || FunctGroupArray[guild].groupType == PWN) {
							if (it_count == 1) {
								for (cohort = 0; cohort < FunctGroupArray[guild].numCohortsXnumGenes; cohort++){
									wcFlux3 += boxLayerInfo->localWCFlux[FunctGroupArray[guild].totNTracers[cohort]];
								}
							}
						} else {
							wcFlux1 += boxLayerInfo->localWCFlux[FunctGroupArray[guild].totNTracers[0]];
						}
						break;
					case AGE_STRUCTURED:
						if (it_count == 1) {
							for (cohort = 0; cohort < FunctGroupArray[guild].numCohortsXnumGenes; cohort++)
								wcFlux2 += (FunctGroupArray[guild].grow[cohort][SN_id] + FunctGroupArray[guild].grow[cohort][RN_id])
										* VERTinfo[guild][cohort][DEN_id] / bm->cell_vol + (FunctGroupArray[guild].dead[cohort]
										+ (double)FunctGroupArray[guild].preyEaten[cohort][WC]) * (VERTinfo[guild][cohort][SN_id] + VERTinfo[guild][cohort][RN_id]) / bm->cell_vol;


						}
						break;

					}
				}

				if (it_count == 1) {
					if(bm->flag_fisheries_on){
						if (FunctGroupArray[guild].isFished == TRUE) {
							/* Skip over those done in Epibenthic_Box() */
							/* select the vertebrate, age structured groups and large zooplankton */
							if (FunctGroupArray[guild].groupType == CEP || FunctGroupArray[guild].groupType== PWN
									|| FunctGroupArray[guild].isVertebrate == TRUE
									|| FunctGroupArray[guild].groupType == LG_ZOO || FunctGroupArray[guild].groupType == MED_ZOO) {

								for (nf = 0; nf < bm->K_num_fisheries; nf++) {
									if(!FisheryArray[nf].isRec){
										wcFishing += bm->Catch[bm->current_box][guild][nf][bm->current_layer] / (bm->cell_vol * bm->dt + small_num);
									} else {
										wcFishing += bm->RecCatch[bm->current_box][guild][nf] / (bm->cell_vol * bm->dt + small_num);
									}
								}
							}
						}
					}else{
						wcFishing = 0;
					}

				}
			}
		}

		if (it_count == 1) {
			wcFlux2global = wcFlux2;
			wcFlux3global = wcFlux3;
			wcFishingGlobal = wcFishing;
		} else {
			wcFlux2 = wcFlux2global;
			wcFlux3 = wcFlux3global;
			wcFishing = wcFishingGlobal;
		}

		wcFlux = wcFlux1 + wcFlux2 + wcFlux3 - wcFishing;

		if (fabs(wcFlux) > Flux_tol) {
			/* Figure out the culprit */

			Max_Flux_Check(bm, WC, &flux_sp, &max_flux_id, &flux_model_id, &fluxnum, boxLayerInfo->localWCFlux, boxLayerInfo->localSEDFlux,
					boxLayerInfo->localEPIFlux, llogfp);

			if (flux_model_id > 0) {
				switch (flux_model_id) {
				case WC:
					fprintf(llogfp, "Water_Column_Box: biggest watercolumn flux was wcFlux[%s] = %e\n", FunctGroupArray[flux_sp].groupCode,
							boxLayerInfo->localWCFlux[max_flux_id]);
					break;
				case SED:
				case EPIFAUNA:
					fprintf(llogfp, "Water_Column_Box: Can't give details of imbalance as flux location mis-set\n");
					break;
				case growth_id:
					fprintf(llogfp, "Water_Column_Box: biggest flux is due to the growth of %s-%d\n", FunctGroupArray[flux_sp].groupCode, fluxnum);
					break;
				case death_id:
					fprintf(llogfp, "Water_Column_Box: biggest flux is due to the death of %s-%d\n", FunctGroupArray[flux_sp].groupCode, fluxnum);
					break;
				default:
					quit("No such flux_model defined - how did it get here in the Epibenthic_Box calculations?\n");
					break;
				}
			} else
				fprintf(llogfp, "Can't give details of imbalance as flux location mis-set\n");

			/* Check diagnostic terms */
			if ((bm->checkNH || bm->checkDL || bm->checkDR) && ((bm->current_box == bm->checkbox) || (bm->dayt >= bm->checkstart)))
				Call_Diagnostics(bm, boxLayerInfo, EPIFAUNA, llogfp, 0);

			/* List all figures */
			fprintf(llogfp, "Water_Column_Box\n");

			Print_Flux(bm, WC, VERTinfo, noqnan_n_fail, wcFlux, wcFlux1, wcFlux2, wcFlux3, 0, 0, 0, wcFlux, wcFishing, boxLayerInfo->localWCFlux,
					boxLayerInfo->localSEDFlux, boxLayerInfo->localEPIFlux, llogfp);

			printf("\nbox: %d, layer: %d, on day: %e, wcFlux: %e (wcFlux1: %e, wcFlux2: %e, wcFlux3: %e)\n", bm->current_box, bm->current_layer, bm->dayt,
					wcFlux, wcFlux1, wcFlux2, wcFlux3);

			Textfile_Dump(bm, llogfp);
			quit("Error on flux balance in water column cell on day: %e, in box: %d, layer: %d, wcFlux: %e\n", bm->dayt, bm->current_box, bm->current_layer,
					wcFlux);

		}

		if (((verbose && (bm->current_box == bm->checkbox) && (bm->dayt >= bm->checkstart)) || (bm->debug == debug_prey_biology_process && bm->dayt
				>= bm->checkstart && bm->dayt < bm->checkstop))) {
			/* List all figures */

			fprintf(llogfp, "debug Water_Column_Box, bm->current_box = %d, currentLayer = %d, it_count = %d\n", bm->current_box, bm->current_layer, it_count);
			Print_Flux(bm, WC, VERTinfo, noqnanverbose, wcFlux, wcFlux1, wcFlux2, wcFlux3, 0, 0, 0, wcFlux, wcFishing, boxLayerInfo->localWCFlux,
					boxLayerInfo->localSEDFlux, boxLayerInfo->localEPIFlux, llogfp);

			if (bm->debug == debug_prey_biology_process && bm->dayt >= bm->checkstart && bm->dayt < bm->checkstop) {
				fprintf(llogfp, "Prey eaten values at end of water column box \n");
				Print_Eat_Diagnostics(bm, llogfp, 0, WC, 1);
			}
		}
	}
    
	if (!(_finite(boxLayerInfo->localWCFlux[NH3_i]))) {
		/* Check diagnostic terms */
		Call_Diagnostics(bm, boxLayerInfo, WC, llogfp, 1);

		/* List all figures */
		//Print_Flux(bm, WC, localWCFlux, 0, 0, qnancheck, wcFlux, wcFlux1, wcFlux2, wcFlux3,
		//       0, 0, 0, wcFlux, wcFishing, llogfp);

		Print_Flux(bm, WC, VERTinfo, noqnan_n_fail, wcFlux, wcFlux1, wcFlux2, wcFlux3, 0, 0, 0, wcFlux, wcFishing, boxLayerInfo->localWCFlux,
				boxLayerInfo->localSEDFlux, boxLayerInfo->localEPIFlux, llogfp);

		Textfile_Dump(bm, llogfp);
        quit("Water_Column_Box: Got an QNAN result box: %d, layer: %d on day: %e wcFlux: %e, volume = %e\n", bm->current_box, bm->current_layer, bm->dayt, wcFlux,
				bm->boxes[bm->current_box].volume[bm->current_layer]);

	}

	return;
}

/*****SEDIMENT MODULE.******************************************************************//**

 \brief This module calculates rate of change of tracers due to biogeochemical
 processes in sediments.

 This routine is the ecological model for sediment cells only.



 ************/
void Sediment_Box(MSEBoxModel *bm, double dtsz, BoxLayerValues *boxLayerInfo, FILE *llogfp) {

	/** Define local variables and data **/
	int cohort;
	/* Define biomass names */
	double DON, NH, NO;

	/* Define nutrient and primary producer names */
	double DIN;	/* DISSOLVED INORGANIC NITROGEN */

	/* Define detritus names */
	//double   DONremin;

	/* Define nitrification-denitrification names */
	double /*Remin, */ReminNet, /*Nitrification, */
	Nitrific_eff, Denitrific_eff;
	//Denitrification;

	/* Define pool, flux and flag names */
	double smFlux = 0;

	double flux_correct, DivHere = 0;

	int guild, flag_sp, fished_chrt, index;

	int max_flux_id = -1;
	int flux_model_id = -1;
	int flux_sp = -1;
	int fluxnum = -1;
	HABITAT_TYPES habitat_type = SED;
	int i, hab;
	int isGlobal;

    if (verbose > 1)
        printf("Sediment_Box for box %d-%d\n", bm->current_box, bm->current_layer);

    boxLayerInfo->sDLscale = 1.0;
	boxLayerInfo->sDRscale = 1.0;
	boxLayerInfo->sDCscale = 1.0;

	/* Initialise diagnostic tracer */
	Reset_Arrays(bm, SED, it_count, boxLayerInfo, llogfp);
	/* Initialise diagnostic tracer */
	if (it_count == 1) {
		boxLayerInfo->localSEDTracers[DiagNGain_i] = 0;
		boxLayerInfo->localSEDTracers[DiagNLoss_i] = 0;
		boxLayerInfo->localSEDTracers[DiagNFlux_i] = 0;
	}

	if ((verbose > 1) || ((bm->debug == debug_biology_process) && (bm->dayt >= bm->checkstart) && (bm->dayt < bm->checkstop)))
		fprintf(llogfp, "\nSediment processes running now (it_count = %d), box = %d, layer = %d, t = %e\n", it_count, bm->current_box, bm->current_layer,
				bm->dayt);

	/* If we are tracking contaminants then work out q10 type corrections now and apply */
	if(bm->track_contaminants){
        Calculate_Species_Contaminant_Effects(bm, bm->current_box, bm->current_layer, dtsz, SED);
		Calculate_Contaminant_Q10_Corrections(bm, boxLayerInfo, SED);
		Apply_Q10_Corrections(bm);
	}
	/**
	 Begin by transferring tracer values from array argument to local variables
	 (see Table above):
	 **/
    Util_Init_1D_Double(initialBiomass, bm->ntracer, 0.0);
	for (i = 0; i < bm->ntracer; i++)
		initialBiomass[i] = boxLayerInfo->localSEDTracers[i];

	DON = boxLayerInfo->localSEDTracers[DON_i];
	NH = boxLayerInfo->localSEDTracers[NH3_i];
	NO = boxLayerInfo->localSEDTracers[NO3_i];

	for (guild = 0; guild < bm->K_num_tot_sp; guild++) {
		if (FunctGroupArray[guild].isVertebrate == FALSE && FunctGroupArray[guild].habitatType == SED && (FunctGroupArray[guild].groupType
				== MICROPHTYBENTHOS || FunctGroupArray[guild].groupType == LG_PHY || FunctGroupArray[guild].groupType == DINOFLAG
				|| FunctGroupArray[guild].groupType == SM_PHY)) {
			if (FunctGroupArray[guild].Light_Adaptn_TracerID > 0)
				FunctGroupArray[guild].SP_IRR = boxLayerInfo->localSEDTracers[FunctGroupArray[guild].Light_Adaptn_TracerID];
			else
				FunctGroupArray[guild].SP_IRR = 1.0;
		}
	}
	/**
	 Construct PREYinfo vector (for use in Eat()
	 **/
	for (guild = 0; guild < bm->K_num_tot_sp; guild++) {
		if(FunctGroupArray[guild].speciesParams[flag_id]){
			if (FunctGroupArray[guild].isVertebrate == FALSE) {
				for(hab = 0; hab < bm->num_active_habitats; hab++){
					PREYinfo[guild][0][hab] = 0.0;
				}
				if (FunctGroupArray[guild].groupAgeType == AGE_STRUCTURED_BIOMASS) {
					for (cohort = 0; cohort < FunctGroupArray[guild].numCohortsXnumGenes; cohort++) {
						PREYinfo[guild][cohort][SED] = boxLayerInfo->localSEDTracers[FunctGroupArray[guild].totNTracers[cohort]]
								* FunctGroupArray[guild].habitatCoeffs[SED];
					}
				} else {
					PREYinfo[guild][0][SED] = boxLayerInfo->localSEDTracers[FunctGroupArray[guild].totNTracers[0]] * FunctGroupArray[guild].habitatCoeffs[SED];
				}
			}
		}
	}

	/* Check that the prey values are finite */
	Check_Prey_Is_Finite(bm, llogfp, habitat_type);

	/***
	 Note: no detritus availability scaling as all detritvores here are specialised for it,
	 not supplementing omnivorous diet
	 ***/

	/****************************** ECOLOGICAL INDICATORS *******************************/
	if (it_count == 1) {
		DivHere = 0;

		/* Local diversity proxy */
		for (guild = 0; guild < bm->K_num_tot_sp; guild++) {
			if (FunctGroupArray[guild].habitatType == SED) {
				if (FunctGroupArray[guild].speciesParams[flag_id] && (initialBiomass[FunctGroupArray[guild].totNTracers[0]] > bm->min_pool))
					DivHere++;
			}

		}
	}

	/**
	 * Initial the invertebrate variables.
	 */
	Init_Invert_Properties(bm);
    
	/****************************** ECOLOGICAL PROCESSES *********************************/
	/**
	 DIN
	 **/
	DIN = NO + NH;

	if (!DIN) {
		DIN = bm->min_pool;
		fprintf(llogfp, "Time: %e, box: %d, layer: %d had to reset DIN in sediment to minpool as was = 0\n", bm->dayt, bm->current_box, bm->current_layer);
		warn("Time: %e, box: %d, layer: %d had to reset DIN in sediment to minpool as was = 0\n", bm->dayt, bm->current_box, bm->current_layer);
	}

	/*
	 * Calculate the sediment bacteria partitioning.
	 */
	Calculate_SedBact_Scale(bm, SED, boxLayerInfo);

	/* now call the process function for each group in the sediment */
	for (index = 0; index < bm->K_num_tot_sp; index++) {
		guild = index;
		flag_sp = (int) (FunctGroupArray[guild].speciesParams[flag_id]);

		if (flag_sp && (FunctGroupArray[guild].isOncePerDt == FALSE || (it_count == 1 && FunctGroupArray[guild].isOncePerDt == TRUE))) {
			switch (FunctGroupArray[guild].groupAgeType) {

			case BIOMASS:
				if (FunctGroupArray[guild].habitatCoeffs[SED] > 0) {
					Call_Group_Process_Function(bm, llogfp, SED, guild, 0, boxLayerInfo);
				}

				break;
			case AGE_STRUCTURED_BIOMASS:
			case AGE_STRUCTURED:
				if (FunctGroupArray[guild].habitatCoeffs[SED] > 0) {
					quit("Age structured sediment groups not yet supported. Contact the Atlantis developers for more information\n");
				}
				/* do nothing */
				break;
			}
		}
	}

	if (it_count == 1) {

		/**
		 Bioturbation and Bioiirigation effects of feeding
		 **/
		Turbatn_contribs = 0.0;
		Irrig_contribs = 0.0;

		for (guild = 0; guild < bm->K_num_tot_sp; guild++) {
			if (FunctGroupArray[guild].isVertebrate == FALSE) {
				if (FunctGroupArray[guild].groupType == LG_INF)
					Turbatn_contribs += FunctGroupArray[guild].speciesParams[KTUR_id] * FunctGroupArray[guild].grazing[0];

				//if (FunctGroupArray[guild].groupType == LG_INF || FunctGroupArray[guild].groupType == SM_INF)
				if(FunctGroupArray[guild].isInfauna == TRUE)
					Irrig_contribs += FunctGroupArray[guild].speciesParams[KIRR_id] * FunctGroupArray[guild].grazing[0];
			}
		}
		if (bm->debug == debug_prey_biology_process && bm->dayt >= bm->checkstart && bm->dayt < bm->checkstop) {
			fprintf(llogfp, "WC - Turbatn_contribs = %.20e\n", Turbatn_contribs);
		}

	}

	/**
	 DON breaks down at a fixed rate r_DON. All other fluxes are calculated per
	 unit volume of sediment, but DON breakdown is calculated per unit pore water
	 volume. This is corrected by multiplying by sediment porosity:
	 **/
	boxLayerInfo->DONremin = r_DON * DON * sporosity;
	/**
	 Nitrification and denitrification.

	 In the sediment, ammonia produced through remineralization of detritus or
	 DON is subject to nitrification and denitrification. The proportions of the
	 ammonia produced which are nitrified and denitrified are assumed to depend
	 on the net ammonia production (respiration) rate, given by total respiration
	 - MPB production. The nitrification efficiency is assumed to fall off from a
	 maximum of Dmax at zero respiration to zero beyond a cut-off respiration
	 rate, R_0. The denitrification efficiency is assumed to increase from zero
	 at zero respiration rate, to 100% at a threshold respiration rate R_D. Both
	 R_0 and R_D are rates per m2, and respiration has to be corrected for layer
	 depth. Thus, in the sediment:
	 **/
	boxLayerInfo->Remin = FunctGroupArray[LabDetIndex].remin + boxLayerInfo->DONremin + FunctGroupArray[RefDetIndex].remin;

	/* Plan for this is to create a flag on setup to check to see if a group eats detritus
	 * If so then include this group in the following calculations
	 *
	 * - the only problem with this - BC does result in releaseNH - i would have
	 * thought it should be included if its not 0.
	 * - Check with beth about this again.
	 *
	 * So the only groups include here are infauna that eat detritus?
	 */

	ReminNet = boxLayerInfo->Remin;

	for (guild = 0; guild < bm->K_num_tot_sp; guild++) {
		if (FunctGroupArray[guild].isVertebrate == FALSE && FunctGroupArray[guild].speciesParams[flag_id] == TRUE) {
			for(cohort = 0; cohort < FunctGroupArray[guild].numCohortsXnumGenes; cohort++){

				if (FunctGroupArray[guild].groupType == MICROPHTYBENTHOS){
					ReminNet -= FunctGroupArray[guild].uptakeNH[cohort];

				}else if (FunctGroupArray[guild].groupType == SED_BACT){
					ReminNet += FunctGroupArray[guild].releaseNH[cohort];

				}else if(FunctGroupArray[guild].speciesParams[sp_remin_contrib_id] == TRUE){
					ReminNet += (FunctGroupArray[guild].releaseNH[cohort] * Fben_den);

				//}else if (FunctGroupArray[guild].groupType == SM_INF || FunctGroupArray[guild].groupType == LG_INF){
//					ReminNet += (FunctGroupArray[guild].releaseNH[cohort] * Fben_den);

				}
			}
		}
	}

	ReminNet = max(ReminNet, 0.0 );

	Nitrific_eff = Dmax * max ( (1.0 - ReminNet * smLayerThick / R_0), 0);

	/** Nitrification = Remin * Nitrific_eff;     **/
	boxLayerInfo->Nitrification = ReminNet * Nitrific_eff;
	Denitrific_eff = min (ReminNet * smLayerThick/ R_D, 1.0);

	boxLayerInfo->Denitrification = boxLayerInfo->Nitrification * Denitrific_eff;

	ReminNet = boxLayerInfo->Remin;
	/* Calculate the remaining */
	for (guild = 0; guild < bm->K_num_tot_sp; guild++) {
		if (FunctGroupArray[guild].groupType == MICROPHTYBENTHOS)
			ReminNet += -FunctGroupArray[guild].growth[0];
	}

	/* Diagnostic information storage */
	boxLayerInfo->DebugInfo[RefDetIndex][SED][DiagnostNHsed_id] = boxLayerInfo->Remin - boxLayerInfo->Nitrification;


	if(bm->track_atomic_ratio){
		Calculate_Phosphorus(bm, boxLayerInfo, SED);
		Calculate_Carbon(bm, boxLayerInfo, SED);
	}

    /* Uptake of contaminants */
    if(bm->track_contaminants){
        Change_Contaminant_Levels(bm, boxLayerInfo, SED, dtsz, bm->ntracer);
    }

	/*******************************************************************************************
	 Reconcile global consumption for adaptive timestep
	 */
	Reconcile_Global_Values(bm, boxLayerInfo, SED, llogfp);

	/**********************************************************************************
	 The equations for rate of change of tracers are:   TODO: Make the age-structured biomass option possible

	 */
	for (guild = 0; guild < bm->K_num_tot_sp; guild++) {
		if(FunctGroupArray[guild].speciesParams[flag_id]){

			if(FunctGroupArray[guild].isDetritus == TRUE)
				continue;

			isGlobal = (FunctGroupArray[guild].diagTol == 2 && it_count == 1);

			switch (FunctGroupArray[guild].groupAgeType) {
			case AGE_STRUCTURED:
				/* Do nothing here */
				break;
			case AGE_STRUCTURED_BIOMASS:
				/* Do nothing here */
				break;
			case BIOMASS:
				/*
				 * tracerFlux = growth - lysis - mortality - dead - preyEaten
				 *
				 */
				if (FunctGroupArray[guild].habitatCoeffs[SED] > 0) {
					if (FunctGroupArray[guild].diagTol < 2 || (it_count == 1 && FunctGroupArray[guild].diagTol == 2)) {
						boxLayerInfo->localSEDFlux[FunctGroupArray[guild].totNTracers[0]]
						                           = FunctGroupArray[guild].growth[0]
						                           - FunctGroupArray[guild].lysis[0]
						                           - FunctGroupArray[guild].mortality[0]
						                           - FunctGroupArray[guild].dead[0]
						                           - (double)FunctGroupArray[guild].preyEaten[0][habitat_type];

						/**
						if(guild == bm->which_check){
							fprintf(llogfp,"Time: %e box%d-%d %s biom: %.20e, sedflux: %.20e, growth: %.20e, lysis: %.20e, mort: %.20e, dead: %.20e, eaten: %Le\n",
									bm->dayt, bm->current_box, bm->current_layer, FunctGroupArray[guild].groupCode,
									boxLayerInfo->localSEDTracers[FunctGroupArray[guild].totNTracers[cohort]],
									boxLayerInfo->localSEDFlux[FunctGroupArray[guild].totNTracers[0]],
									FunctGroupArray[guild].growth[0], FunctGroupArray[guild].lysis[0],
									FunctGroupArray[guild].mortality[0], FunctGroupArray[guild].dead[0], FunctGroupArray[guild].preyEaten[0][habitat_type]);
						}
						**/

						if(bm->track_atomic_ratio == TRUE){

							/* Predation and growth due to is handled separately */
							Loose_Element(bm, boxLayerInfo, SED, guild, 0,
									(		FunctGroupArray[guild].dead[0] +
											FunctGroupArray[guild].lysis[0] +
											FunctGroupArray[guild].mortality[0]),
									SED, isGlobal);

						}

						/* If we are damping the biomass of this group then do so */
						if (FunctGroupArray[guild].speciesParams[flux_thresh_id] > 0) {
							/* If the biomass value is above the threshhold then limit */
							if (boxLayerInfo->localSEDFlux[FunctGroupArray[guild].totNTracers[0]] > FunctGroupArray[guild].speciesParams[flux_thresh_id]) {
								boxLayerInfo->localSEDFlux[FunctGroupArray[guild].totNTracers[0]]
										= boxLayerInfo->localSEDFlux[FunctGroupArray[guild].totNTracers[0]] * FunctGroupArray[guild].speciesParams[flux_damp_id];
							}
						}

						boxLayerInfo->DebugFluxInfo[guild][habitat_type][gain_id] = FunctGroupArray[guild].growth[0];
						boxLayerInfo->DebugFluxInfo[guild][habitat_type][loss_id] = FunctGroupArray[guild].mortality[0]
								+ (double)FunctGroupArray[guild].preyEaten[0][habitat_type];

						if (FunctGroupArray[guild].isProducer) {
							boxLayerInfo->localDiagFlux[FunctGroupArray[guild].prodnTracers[0]] += FunctGroupArray[guild].growth[0];
						}
						if (FunctGroupArray[guild].isGrazer) {
							boxLayerInfo->localDiagFlux[FunctGroupArray[guild].GrazingTracers[0]] += FunctGroupArray[guild].uptakeDL
									+ FunctGroupArray[guild].uptakeDR + FunctGroupArray[guild].grazing[0]
									+ FunctGroupArray[guild].maxPhagotrophy;
						}

#ifdef BROKER_LINK_ENABLED
						if(do_BrokerLinkage){
							if (FunctGroupArray[guild].isDetritus == FALSE) {
								boxLayerInfo->localSEDFlux[FunctGroupArray[guild].totNTracers[0]]
										+= -bm->linkageInterface->linkageSEDMortality[bm->current_box][bm->current_layer][guild][0];

								boxLayerInfo->DebugFluxInfo[guild][habitat_type][loss_id] += bm->linkageInterface->linkageSEDMortality[bm->current_box][bm->current_layer][guild][0];

							} else {

								boxLayerInfo->localSEDFlux[FunctGroupArray[guild].totNTracers[0]] += +bm->linkageInterface->linkageSEDDetritusFlux[bm->current_box][bm->current_layer][guild];

								boxLayerInfo->DebugFluxInfo[guild][habitat_type][loss_id] += bm->linkageInterface->linkageSEDDetritusFlux[bm->current_box][bm->current_layer][guild];
							}
						}
#endif
						if (FunctGroupArray[guild].secondNutrientTracerIndex > 0) {
							boxLayerInfo->localSEDFlux[FunctGroupArray[guild].secondNutrientTracerIndex]
									= boxLayerInfo->localSEDFlux[FunctGroupArray[guild].totNTracers[0]] * FunctGroupArray[guild].secondNutrient;
						}
					}
				}
				break;
			}
		}
	}

	/* if scaling mortality values handle the detritus groups now - just do it in one go */
	if(bm->scale_all_mortality == TRUE){
		Scale_Detritus_Mortality(bm, boxLayerInfo);
	}

	/**
	 Labile detritus in sediment
	 **/
	Labile_Detritus_ROC(bm, llogfp, SED, boxLayerInfo);

	/**
	 Carrion in sediment
	 **/
	if (it_count == 1) {
		Carrion_ROC(bm, llogfp, SED, boxLayerInfo);
	}
	/**
	 Refractory detritus in sediment
	 **/
	Refractory_Detritus_ROC(bm, llogfp, SED, boxLayerInfo);

	/**
	 *  Nutrients in the water column
	 *
	 **/
	for (index = 0; index < bm->K_num_physiochem; index++) {
		if (PhysioChemArray[index].ROCFunction != NULL) {
			PhysioChemArray[index].ROCFunction(bm, llogfp, SED, index, boxLayerInfo);
			if(bm->ecotest > 1){
				/* Check the tracer values */
				if(!_finite(boxLayerInfo->localSEDFlux[*PhysioChemArray[index].tracerIndex])){
					quit("Sediment_Box - Nutrient %s is infinite in box %d:%d\n", PhysioChemArray[index].name, bm->current_box, bm->current_layer);
				}
			}

		}
	}

	/**
	 Correct dissolved tracer fluxes for pore water volume:
	 **/
	boxLayerInfo->localSEDFlux[NH3_i] = boxLayerInfo->localSEDFlux[NH3_i] / sporosity;
	boxLayerInfo->localSEDFlux[NO3_i] = boxLayerInfo->localSEDFlux[NO3_i] / sporosity;
	boxLayerInfo->localSEDFlux[DON_i] = boxLayerInfo->localSEDFlux[DON_i] / sporosity;
	boxLayerInfo->localSEDFlux[Oxygen_i] = boxLayerInfo->localSEDFlux[Oxygen_i] / sporosity;

	boxLayerInfo->localSEDFlux[Si_i] = boxLayerInfo->localSEDFlux[Si_i] / sporosity;

	/**
	 Temperature
	 **/
	boxLayerInfo->localSEDTracers[Temp_i] = H2Otemp;

	/**
	 Diagnostic variables and ecological indicators
	 **/
	boxLayerInfo->localSEDTracers[Chl_a_i] = 0.0;
	for (guild = 0; guild < bm->K_num_tot_sp; guild++)
		if (FunctGroupArray[guild].isVertebrate == FALSE)
			boxLayerInfo->localSEDTracers[Chl_a_i] += FunctGroupArray[guild].chl;

	boxLayerInfo->localSEDTracers[Chl_a_i] = boxLayerInfo->localSEDTracers[Chl_a_i] / X_CHLN;

	if (it_count == 1) {
		boxLayerInfo->localDiagTracers[DivCount_i] += DivHere;
	}

	boxLayerInfo->localDiagTracers[PercentDenitri_i] = Nitrific_eff * Denitrific_eff * 100.0;
	boxLayerInfo->localDiagTracers[Nitrif_eff_i] = Nitrific_eff;
	boxLayerInfo->localDiagTracers[Denitrif_eff_i] = Denitrific_eff;

	boxLayerInfo->localDiagFlux[Don_Prod_i] += FunctGroupArray[LabDetIndex].solDON + FunctGroupArray[RefDetIndex].solDON
			- boxLayerInfo->DONremin;

	boxLayerInfo->localDiagFlux[Recyc_N_i] += ReminNet;

	boxLayerInfo->localSEDFlux[Denitrification_i] = boxLayerInfo->Denitrification;
	boxLayerInfo->localSEDFlux[Nitrification_i] = boxLayerInfo->Nitrification;

	/**
	 * If we are tracking the atomic tracers - calculate the flux values for this habitat before the arrays are cleared for the new habitat
	 */
	if(bm->track_atomic_ratio == TRUE){
		Calculate_Element_Flux(bm, boxLayerInfo->localSEDTracers, boxLayerInfo->localSEDFlux, SED, SED);
	}

	/* Update mortality estimates for sediment invertebrates */
	for (guild = 0; guild < bm->K_num_tot_sp; guild++) {
		if(FunctGroupArray[guild].speciesParams[flag_id]){
			if (FunctGroupArray[guild].isVertebrate == FALSE) {

				fished_chrt = 0;
				/* For use in pseudo M estimate */

				if (FunctGroupArray[guild].isDetritus == TRUE) {
					bm->calcMnum[guild][current_id] +=
							(FunctGroupArray[guild].mortality[fished_chrt] + (double)FunctGroupArray[guild].preyEaten[0][WC]);
				} else {
					hab = SED;

					for (cohort = 0; cohort < FunctGroupArray[guild].numCohortsXnumGenes; cohort++) {
						bm->calcMnum[guild][current_id] +=
							(FunctGroupArray[guild].mortality[fished_chrt]  + (double)FunctGroupArray[guild].preyEaten[cohort][hab]);
					}
				}
#ifdef BROKER_LINK_ENABLED
				if(do_BrokerLinkage){
					bm->calcMnum[guild][current_id] += bm->linkageInterface->linkageSEDMortality[bm->current_box][bm->current_layer][guild][fished_chrt];
				}
#endif
			}
		}
	}

	/* check diagnostics */
	if (bm->habitat_check == SED && bm->which_check < not_tracking_flux) {
		flux_correct = sporosity; // Correct nutrient fluxes in the sediment for sediment porosity
	} else
		flux_correct = 1.0;

	if (bm->which_check < not_tracking_flux && bm->habitat_check == SED) {

		if (it_count == 1) {
			boxLayerInfo->localSEDTracers[DiagNGain_i] = ((boxLayerInfo->DebugFluxInfo[bm->which_check][bm->habitat_check][gain_id]) / flux_correct) * dtsz;
			boxLayerInfo->localSEDTracers[DiagNLoss_i] = ((boxLayerInfo->DebugFluxInfo[bm->which_check][bm->habitat_check][loss_id]) / flux_correct) * dtsz;
			boxLayerInfo->localSEDTracers[DiagNFlux_i] = ((boxLayerInfo->DebugFluxInfo[bm->which_check][bm->habitat_check][gain_id]
					- boxLayerInfo->DebugFluxInfo[bm->which_check][bm->habitat_check][loss_id]) / flux_correct) * dtsz;
		} else {
			boxLayerInfo->localSEDTracers[DiagNGain_i] += ((boxLayerInfo->DebugFluxInfo[bm->which_check][bm->habitat_check][gain_id]) / flux_correct) * dtsz;
			boxLayerInfo->localSEDTracers[DiagNLoss_i] += ((boxLayerInfo->DebugFluxInfo[bm->which_check][bm->habitat_check][loss_id]) / flux_correct) * dtsz;
			boxLayerInfo->localSEDTracers[DiagNFlux_i] += ((boxLayerInfo->DebugFluxInfo[bm->which_check][bm->habitat_check][gain_id]
					- boxLayerInfo->DebugFluxInfo[bm->which_check][bm->habitat_check][loss_id]) / flux_correct) * dtsz;
		}
	}

	if ((bm->checkNH || bm->checkDL || bm->checkDR) && ((bm->dayt >= bm->checkstart) && ((bm->checkbox > bm->nbox) || (bm->current_box == bm->checkbox))))
		Call_Diagnostics(bm, boxLayerInfo, SED, llogfp, 0);

	if (bm->checkbiom && ((bm->checkbox > bm->nbox) || (bm->current_box == bm->checkbox)) && (bm->dayt >= bm->checkstart)) {
		Ecology_Calculate_Total_Abundance(bm, bm->dt, 1, llogfp);
	}

	/* test flux balance */
	if (bm->ecotest > 1) {

		smFlux = 0.0;

		for (guild = 0; guild < bm->K_num_tot_sp; guild++) {
			if(FunctGroupArray[guild].speciesParams[flag_id]){
				if (FunctGroupArray[guild].habitatCoeffs[SED] > 0 ){
					smFlux += boxLayerInfo->localSEDFlux[FunctGroupArray[guild].totNTracers[0]];
				}
			}
		}

		smFlux += (boxLayerInfo->localSEDFlux[NH3_i] * sporosity);
		smFlux += (boxLayerInfo->localSEDFlux[NO3_i] * sporosity);
		smFlux += (boxLayerInfo->localSEDFlux[DON_i] * sporosity);
		smFlux += (boxLayerInfo->localSEDFlux[Denitrification_i]);
        
		if (fabs(smFlux) > Flux_tol) {

			/* Figure out the culprit */
			Max_Flux_Check(bm, SED, &flux_sp, &max_flux_id, &flux_model_id, &fluxnum, boxLayerInfo->localWCFlux, boxLayerInfo->localSEDFlux,
					boxLayerInfo->localEPIFlux, llogfp);
			if (flux_model_id > 0) {
				switch (flux_model_id) {
				case SED:
					fprintf(llogfp, "Sediment_Box: biggest sediment flux was sedFlux[%s] = %e\n", FunctGroupArray[flux_sp].groupCode,
							boxLayerInfo->localSEDFlux[max_flux_id]);
					break;
				case WC:
				case EPIFAUNA:
				case growth_id:
				case death_id:
					fprintf(llogfp, "Sediment_Box: Can't give details of imbalance as flux location mis-set\n");
					break;
				default:
					quit("No such flux_model defined - how did it get here in the Sediment_Box calculations?\n");
					break;
				}
			} else
				fprintf(llogfp, "Can't give details of imbalance as flux location mis-set\n");

			/* Check diagnostic terms */
			if ((bm->checkNH || bm->checkDL || bm->checkDR) && ((bm->current_box == bm->checkbox) || (bm->dayt >= bm->checkstart)))
				Call_Diagnostics(bm, boxLayerInfo, EPIFAUNA, llogfp, 0);

			/* List all figures */
			fprintf(llogfp, "Sediment_Box\n");

			Print_Flux(bm, SED, VERTinfo, noqnan_n_fail, 0, 0, 0, 0, 0, smFlux, 0, smFlux, 0, boxLayerInfo->localWCFlux, boxLayerInfo->localSEDFlux,
					boxLayerInfo->localEPIFlux, llogfp);
			Textfile_Dump(bm, llogfp);
			quit("Error of flux balance in sediment box: %d on day: %e, smflux: %e\n", bm->current_box, bm->dayt, smFlux);
		}

		if ((verbose && (bm->current_box == bm->checkbox) && (bm->dayt >= bm->checkstart)) || (bm->debug == debug_prey_biology_process && bm->dayt
				>= bm->checkstart && bm->dayt < bm->checkstop)) {
			/* List all figures */
			fprintf(llogfp, "debug Sediment_Box\n");
			/* List all figures */
			Print_Flux(bm, SED, VERTinfo, noqnanverbose, 0, 0, 0, 0, 0, smFlux, 0, smFlux, 0, boxLayerInfo->localWCFlux, boxLayerInfo->localSEDFlux,
					boxLayerInfo->localEPIFlux, llogfp);

			fprintf(llogfp, "Prey eaten values at end of sediment column box \n");
			Print_Eat_Diagnostics(bm, llogfp, 0, SED, 1);
			// Print_Eat_Diagnostics(bm, llogfp, 0, EPIFAUNA, 1);

		}
	}
	if (!(_finite(boxLayerInfo->localSEDFlux[NH3_i]))) {
		/* Check diagnostic terms */
		Call_Diagnostics(bm, boxLayerInfo, SED, llogfp, 1);

		/* List all figures */
		Print_Flux(bm, SED, VERTinfo, qnancheck, 0, 0, 0, 0, 0, smFlux, 0, smFlux, 0, boxLayerInfo->localWCFlux, boxLayerInfo->localSEDFlux,
				boxLayerInfo->localEPIFlux, llogfp);

		Textfile_Dump(bm, llogfp);
        quit("Sediment_Box: Got an QNAN result box: %d, layer: %d on day: %e, smflux: %e\n", bm->current_box, bm->current_layer, bm->dayt, smFlux);
	}
}

/************ EPIBENTHIC MODULE***************************************************************

 This module calculates rate of change of tracers due to biogeochemical
 processes associated with epibenthos in the adjacent water column and
 sediment cells.

 Begin by transferring tracer values from array argument to local variables
 (see Table above). We must now distinguish between water column tracers and
 sediment tracers in the local variables. We add an s to denote sediment tracers:

 Need to correct for thickness of water column layer wcLayerThick,
 and sediment layer smLayerThick.

 *********/

void Epibenthic_Box(MSEBoxModel *bm, double dtsz, BoxLayerValues *boxLayerInfo, FILE *llogfp) {

	double totSp = 0;
	/* Define biomass names */
	double NH, NHs, NO, NOs, DL, DR, DLsed, DRsed, DCsed;
	double totalBiomass = 0;

	/* Define ecological indicators */
	double DivHere = 0, InfEpiRatio = 0;

	/* Define pool, flux and flag names */
	double wcFlux = 0, wcFlux1 = 0, wcFlux2 = 0, wcFlux3 = 0, wcFlux4 = 0, smFlux = 0, smFlux1 = 0, smFlux2 = 0, epiFlux = 0, epiFlux1 = 0, epiFlux2 = 0,
			epiFishing = 0, TotFlux = 0;

	double sp_scalar = 0.0, Wgt = 1.0;
	int guild, kij, i, flag_sp, nf, ct, flagimposecatch;
	int max_flux_id = -1;
	int flux_model_id = -1;
	int flux_sp = -1;
	int fluxnum = -1;
	int fgIndex, cohort, hab, index;
	HABITAT_TYPES habitat_type = EPIFAUNA;
	double Epifauna = 0, Infauna = 0;
    double mortality_scalar = 1.0;
    double tot_dis, this_bio = 0, realism_scalar;
    
	int isGlobal;
    
    if (verbose > 1) {
        printf("Epibenthic_Box for box %d-%d\n", bm->current_box, bm->current_layer);
    }

	/* Set pointers to sediment and epibenthic cells */

	/***/
	/* Set initial tracer values - note vertebrate values are set when
	 Water_Column_Box are called
	 */
    Util_Init_1D_Double(initialSedBiomass, bm->ntracer, 0.0);
    Util_Init_1D_Double(initialEpiBiomass, bm->ntracer, 0.0);
    Util_Init_1D_Double(initialWaterBiomass, bm->ntracer, 0.0);

	for (i = 0; i < bm->ntracer; i++) {
		initialSedBiomass[i] = boxLayerInfo->localSEDTracers[i];
		initialEpiBiomass[i] = boxLayerInfo->localEPITracers[i];
		initialWaterBiomass[i] = boxLayerInfo->localWCTracers[i];
	}
	if(bm->terrestrial_on == TRUE){
        Util_Init_1D_Double(initialLandBiomass, bm->nland, 0.0);
		for(i = 0; i < bm->nland; i++){
			initialLandBiomass[i] = boxLayerInfo->localLANDTracers[i];
		}
	}
    
    if(bm->ice_on == TRUE){
        Util_Init_1D_Double(initialIceBiomass, bm->nicetracer, 0.0);
        for (i = 0; i < bm->nicetracer; i++) {
            initialIceBiomass[i] = boxLayerInfo->localICETracers[i];
        }
    }

	DL = boxLayerInfo->localWCTracers[FunctGroupArray[LabDetIndex].totNTracers[0]];
	DR = boxLayerInfo->localWCTracers[FunctGroupArray[RefDetIndex].totNTracers[0]];
	NH = boxLayerInfo->localWCTracers[NH3_i];
	NO = boxLayerInfo->localWCTracers[NO3_i];

	NHs = boxLayerInfo->localSEDTracers[NH3_i];
	NOs = boxLayerInfo->localSEDTracers[NO3_i];
	DCsed = boxLayerInfo->localSEDTracers[FunctGroupArray[CarrionIndex].totNTracers[0]];
	DLsed = boxLayerInfo->localSEDTracers[FunctGroupArray[LabDetIndex].totNTracers[0]];
	DRsed = boxLayerInfo->localSEDTracers[FunctGroupArray[RefDetIndex].totNTracers[0]];

	/* run water column bottom cell */
	Water_Column_Box(bm, dtsz, boxLayerInfo, llogfp);

	/* run top sediment cell */
	Sediment_Box(bm, dtsz, boxLayerInfo, llogfp);

	if ((verbose > 1) || ((bm->debug == debug_biology_process) && (bm->dayt >= bm->checkstart) && (bm->dayt < bm->checkstop)))
		fprintf(llogfp, "\nEpibenthic processes running now (it_count = %d), box = %d, layer = %d, t = %e\n", it_count, bm->current_box, bm->current_layer,
				bm->dayt);

	Reset_Arrays(bm, EPIFAUNA, it_count, boxLayerInfo, llogfp);


	/* If we are tracking contaminants then work out q10 type corrections now and apply */
	if(bm->track_contaminants){
        Calculate_Species_Contaminant_Effects(bm, bm->current_box, bm->current_layer, dtsz, EPIFAUNA);
		Calculate_Contaminant_Q10_Corrections(bm, boxLayerInfo, EPIFAUNA);
		Apply_Q10_Corrections(bm);
	}

	/* Initialise the invert properties */
	Init_Invert_Properties(bm);

	for (guild = 0; guild < bm->K_num_tot_sp; guild++) {
		if(FunctGroupArray[guild].speciesParams[flag_id]){
			if (FunctGroupArray[guild].isVertebrate == FALSE && FunctGroupArray[guild].habitatType == EPIFAUNA && (FunctGroupArray[guild].groupType == PHYTOBEN
					|| FunctGroupArray[guild].groupType == SEAGRASS || FunctGroupArray[guild].groupType == MICROPHTYBENTHOS)) {

				if (FunctGroupArray[guild].Light_Adaptn_TracerID > 0)
					FunctGroupArray[guild].SP_IRR = boxLayerInfo->localWCTracers[FunctGroupArray[guild].Light_Adaptn_TracerID];
				else
					FunctGroupArray[guild].SP_IRR = 1.0;

			}
		}
	}

	if (bm->debug && ((bm->debug > debug_biom) && (bm->current_box == bm->checkbox || bm->checkbox > bm->nbox) && (bm->which_check < bm->K_num_tot_sp)
			&& FunctGroupArray[bm->which_check].isVertebrate == TRUE && (bm->dayt > bm->checkstart))) {
		fprintf(llogfp, "day: %e, box: %d, wclayer %d, ", bm->dayt, bm->current_box, bm->current_layer);
		for (cohort = 0; cohort < FunctGroupArray[bm->which_check].numCohorts * FunctGroupArray[bm->which_check].numGeneTypes; cohort++) {
			fprintf(llogfp, "vinfoSN%d %e vinfoRN%d %e vinfoDEN-%d %e\n", cohort, VERTinfo[bm->which_check][cohort][SN_id], cohort,
					VERTinfo[bm->which_check][cohort][RN_id], cohort, VERTinfo[bm->which_check][cohort][DEN_id]);
		}
	}

	/**************************************************************************
	 Determine detritus availability scaling
	 ***/
	boxLayerInfo->DLscale = min(1,k_refDL/(DL+small_num));
	boxLayerInfo->DRscale = min(1,k_refDR/(DR+small_num));
	boxLayerInfo->sDLscale = min(1,k_refsDL/(DLsed+small_num));
	boxLayerInfo->sDRscale = 1.0;
	boxLayerInfo->sDCscale = 1.0;

	/**
	 Construct PREYinfo vector (for use in Eat()
	 **/
	for (guild = 0; guild < bm->K_num_tot_sp; guild++) {
		if(FunctGroupArray[guild].speciesParams[flag_id]){
			if (FunctGroupArray[guild].isVertebrate == FALSE) {
				/* Setup prey of each seagrass cohort */
				if (FunctGroupArray[guild].groupAgeType == AGE_STRUCTURED_BIOMASS) {
					for (cohort = 0; cohort < FunctGroupArray[guild].numCohortsXnumGenes; cohort++) {
						PREYinfo[guild][cohort][WC] = boxLayerInfo->localWCTracers[FunctGroupArray[guild].totNTracers[cohort]]
								* FunctGroupArray[guild].habitatCoeffs[WC];
						PREYinfo[guild][cohort][SED] = boxLayerInfo->localSEDTracers[FunctGroupArray[guild].totNTracers[cohort]]
								* FunctGroupArray[guild].habitatCoeffs[SED];
						PREYinfo[guild][cohort][EPIFAUNA] = boxLayerInfo->localEPITracers[FunctGroupArray[guild].totNTracers[cohort]]
								* FunctGroupArray[guild].habitatCoeffs[EPIFAUNA];
					}
				} else {
					PREYinfo[guild][0][EPIFAUNA] = boxLayerInfo->localEPITracers[FunctGroupArray[guild].totNTracers[0]] * FunctGroupArray[guild].habitatCoeffs[EPIFAUNA];
					PREYinfo[guild][0][WC] = boxLayerInfo->localWCTracers[FunctGroupArray[guild].totNTracers[0]] * FunctGroupArray[guild].habitatCoeffs[WC];

					if (FunctGroupArray[guild].groupType != LG_PHY &&
							FunctGroupArray[guild].groupType != SM_PHY &&
							FunctGroupArray[guild].groupType != DINOFLAG &&
							FunctGroupArray[guild].groupType != SM_INF) {
						PREYinfo[guild][0][SED] = boxLayerInfo->localSEDTracers[FunctGroupArray[guild].totNTracers[0]]
								* FunctGroupArray[guild].habitatCoeffs[SED];
					}
				}
			}
		}
	}

	PREYinfo[LabDetIndex][0][WC] = PREYinfo[LabDetIndex][0][WC] * boxLayerInfo->DLscale;
	PREYinfo[RefDetIndex][0][WC] = PREYinfo[RefDetIndex][0][WC] * boxLayerInfo->DRscale;
	PREYinfo[LabDetIndex][0][SED] = PREYinfo[LabDetIndex][0][SED] * boxLayerInfo->sDLscale;

	/* Check that the prey values are finite */
	Check_Prey_Is_Finite(bm, llogfp, habitat_type);

	/********************** FISHERIES PROCESSES ********************************/
	/***
	 Fishing and induced incidental mortality on invertebrates and benthic primary producers.
	 This is done first so that animals can steal from the catch (if appropriate).
	 Note that pelagic invertebrates and all vertebrate fisheries and bycatch are dealt
	 with in WaterColumn().
	 ***/
	if (bm->flag_fisheries_on && (it_count == 1)) {

        if(bm->flagdepend_dis){
            for (fgIndex = 0; fgIndex < bm->K_num_tot_sp; fgIndex++) {
                if(FunctGroupArray[fgIndex].speciesParams[flag_id]){
                    // Zero bm->DependDiscardsTot array - for density dependent discard calculation
                    if(FunctGroupArray[fgIndex].habitatType != EPIFAUNA) {
                        for(nf=0; nf<bm->K_num_fisheries; nf++){
                            for(cohort=0; cohort<FunctGroupArray[fgIndex].numCohortsXnumGenes; cohort++){
                                bm->DependDiscardsTot[nf][fgIndex][cohort] = 0.0;
                            }
                        }
					}
	            }
			}
		}
		/**
		 Pelagic invertebrate fisheries and bycatch
		 **/
		for (fgIndex = 0; fgIndex < bm->K_num_tot_sp; fgIndex++) {
			if(FunctGroupArray[fgIndex].speciesParams[flag_id]){

	            // Do invertebrate fishing
				if ((int) (FunctGroupArray[fgIndex].speciesParams[flag_id]) && FunctGroupArray[fgIndex].isImpacted == TRUE) {

					switch (FunctGroupArray[fgIndex].habitatType) {
					case SED:
						if ((int) (FunctGroupArray[fgIndex].speciesParams[flagimpose_id])
								|| boxLayerInfo->localSEDTracers[FunctGroupArray[fgIndex].totNTracers[0]] > bm->min_pool) {
							Calculate_Catch(bm, boxLayerInfo, llogfp, fgIndex, 0, boxLayerInfo->localSEDTracers[FunctGroupArray[fgIndex].totNTracers[0]], 0.0,
									1.0, 1.0, 1.0);
						}
						break;
					case EPIFAUNA:
						switch (FunctGroupArray[fgIndex].groupAgeType) {
						case AGE_STRUCTURED:
							// This is a new addition, but should not break old code as age structured typically only WC
							flagimposecatch = (int) (FunctGroupArray[fgIndex].speciesParams[flagimpose_id]);
							for (cohort = (FunctGroupArray[fgIndex].numCohortsXnumGenes) - 1; cohort >= 0; cohort--) {
								if (flagimposecatch || ((VERTinfo[fgIndex][cohort][SN_id] > bm->min_pool) && (VERTinfo[fgIndex][cohort][DEN_id] > bm->min_dens))) {
									Calculate_Catch(bm, boxLayerInfo, llogfp, fgIndex, cohort, VERTinfo[fgIndex][cohort][SN_id], VERTinfo[fgIndex][cohort][RN_id],
											VERTinfo[fgIndex][cohort][DEN_id], 1.0, 1.0);
								}
							}
							break;
						case BIOMASS:	/* intentional follow thur */
						case AGE_STRUCTURED_BIOMASS:
							for (cohort = 0; cohort < FunctGroupArray[fgIndex].numCohortsXnumGenes; cohort++) {
								flagimposecatch = (int) (FunctGroupArray[fgIndex].speciesParams[flagimpose_id]);
								if (flagimposecatch || initialEpiBiomass[FunctGroupArray[fgIndex].totNTracers[cohort]] > bm->min_pool) {
									Calculate_Catch(bm, boxLayerInfo, llogfp, fgIndex, cohort, initialEpiBiomass[FunctGroupArray[fgIndex].totNTracers[cohort]], 0.0, 1.0,
											smLayerThick, wcLayerThick);
								}

							}
							break;
						}

						break;
					case WC:
					case ICE_BASED:
					case LAND_BASED:
						// TODO: May need to change this for ice and land
						break;
                    case MIXED:
                        quit("How did we get here as should come through a primary habitat\n");
                        break;
					}
				}
			}
		}


        // Update dead and discards
        if(bm->flagdepend_dis){
			for(guild=0; guild<bm->K_num_tot_sp; guild++){
				flag_sp = (int)FunctGroupArray[guild].speciesParams[flag_id];
				if(flag_sp){
					sp_scalar = 1.0;
					for(cohort=0; cohort<FunctGroupArray[guild].numCohortsXnumGenes; cohort++){
						/* Determine local biomass for vertebrates (invertebrates assume SC entry = Biomass */
						if (FunctGroupArray[guild].groupAgeType == AGE_STRUCTURED){
							Wgt = VERTinfo[guild][cohort][SN_id] + VERTinfo[guild][cohort][RN_id];
							if(Wgt < small_num)
								Wgt = small_num;     /* To avoid divide by zero problems */

                            sp_scalar = bm->cell_vol / Wgt;  // Used to have bm->dt in denominator, but that doesn't make sense as already per s
                            //sp_scalar = 1 / (Wgt * bm->dt);  // Needs to be * bm->cell_vol as was calculated in terms of mg m-3 and need to get to nums
						}

                        /* Reality check */
                        // Find total to be discarded
                        tot_dis = 0.0;
                        for(nf=0; nf<bm->K_num_fisheries; nf++){
                            tot_dis += bm->DependDiscardsTot[nf][guild][cohort] * bm->dt;
                        }
                        
                        if(!tot_dis)
                            continue;
                        
                        // See if taking more biomass than present and fix accordingly
                        if (FunctGroupArray[guild].groupAgeType == AGE_STRUCTURED){
                            this_bio = Wgt * VERTinfo[guild][cohort][DEN_id];
                        } else {
                            switch (FunctGroupArray[guild].habitatType) {
                                case WC:
                                    this_bio = initialWaterBiomass[FunctGroupArray[guild].totNTracers[cohort]];
                                    break;
                                case SED:
                                    this_bio = initialSedBiomass[FunctGroupArray[guild].totNTracers[cohort]];
                                    break;
                                case EPIFAUNA:
                                    this_bio = initialEpiBiomass[FunctGroupArray[guild].totNTracers[cohort]];
                                    sp_scalar *= wcLayerThick; // To get in to mg m-2 from mg-3
                                    break;
                                case LAND_BASED:
                                    this_bio = initialLandBiomass[FunctGroupArray[guild].totNTracers[cohort]];
                                    break;
                                case ICE_BASED:
                                    this_bio = initialIceBiomass[FunctGroupArray[guild].totNTracers[cohort]];
                                    break;
                                case MIXED:
                                    quit("How did we get here as should come through a primary habitat\n");
                                    break;
                            }
                        }
                        realism_scalar = 1.0;
                        if (tot_dis > this_bio)
                            realism_scalar = this_bio / (tot_dis + small_num);
                        
						for(nf=0; nf<bm->K_num_fisheries; nf++){
                            if(bm->DependDiscardsTot[nf][guild][cohort]) {
                                bm->DependDiscardsTot[nf][guild][cohort] *= realism_scalar;
                                bm->Discards[bm->current_box][guild][nf] += (bm->DependDiscardsTot[nf][guild][cohort] * bm->dt * bm->cell_vol);

                                if(FunctGroupArray[guild].groupAgeType == AGE_STRUCTURED){
                                    FunctGroupArray[guild].SizeNumDiscard[cohort][nf][bm->current_box] += bm->DependDiscardsTot[nf][guild][cohort];
                                    bm->FCdiscard[guild][cohort] += bm->DependDiscardsTot[nf][guild][cohort];
                                }
                                boxLayerInfo->DetritusProd[SED][DCdet_id] += bm->DependDiscardsTot[nf][guild][cohort];

                                FunctGroupArray[guild].dead[cohort] += bm->DependDiscardsTot[nf][guild][cohort] * sp_scalar;
                                
                                // Update Catchsum - moved here to remove an additional loop and so it has the reality check in place
                                CatchSum[guild][tscodiscard_id] += (bm->DependDiscardsTot[nf][guild][cohort] * bm->cell_vol * bm->dt);
                            
                                /**
                                if (guild == bm->which_check)
                                    fprintf(llogfp,"Time: %e box%d-%d %s-%d dead: %e discards: %e due to dependdiscards by %s sp_scalar: %e so dead now: %e Wgt: %e CatchSum: %e realism_scalar: %e\n", bm->dayt, bm->current_box, bm->current_layer, FunctGroupArray[guild].groupCode, cohort, bm->Discards[bm->current_box][guild][nf], bm->DependDiscardsTot[nf][guild][cohort], FisheryArray[nf].fisheryCode, sp_scalar, FunctGroupArray[guild].dead[cohort], Wgt, CatchSum[guild][tscodiscard_id], realism_scalar);
                                **/
                            }
						}
                        
                        // Update the global dead tracker so not missing the discards component
                        if ((FunctGroupArray[guild].isOncePerDt == FALSE) || (FunctGroupArray[guild].diagTol < 2))
                            FunctGroupArray[guild].deadGlobal[cohort] = FunctGroupArray[guild].dead[cohort];

					}
				}
			}
        }
	}

	/* Do scaling of dead due to fishing in one go now */
	if(bm->scale_all_mortality == TRUE){
		for (guild = 0; guild < bm->K_num_tot_sp; guild++) {
			for(cohort=0; cohort<FunctGroupArray[guild].numCohorts; cohort++){
				/* If there is a scalar to apply then grab it now */
				mortality_scalar = Ecology_Get_Mortality_Scalar(bm, guild, cohort);

				FunctGroupArray[guild].dead[cohort] = FunctGroupArray[guild].dead[cohort] * mortality_scalar;
 			}
		}
	}


	/****************************** ECOLOGICAL INDICATORS *******************************/
	if (it_count == 1) {
		DivHere = 0;

		/* Local diversity proxy */
		for (guild = 0; guild < bm->K_num_tot_sp; guild++) {
			if(FunctGroupArray[guild].speciesParams[flag_id]){
				for(cohort = 0; cohort < FunctGroupArray[guild].numCohortsXnumGenes; cohort++){
					if (FunctGroupArray[guild].habitatType == EPIFAUNA) {
						if (FunctGroupArray[guild].speciesParams[flag_id] && (boxLayerInfo->localEPITracers[FunctGroupArray[guild].totNTracers[cohort]] > bm->min_pool)) {
							DivHere++;
						}
					}

					//TODO: Find a better way to do this. Super ugly.
					if (FunctGroupArray[guild].habitatType == EPIFAUNA) {
						Epifauna += initialEpiBiomass[FunctGroupArray[guild].totNTracers[cohort]];
					} else if (FunctGroupArray[guild].habitatCoeffs[EPIFAUNA] > 0) {
						if (FunctGroupArray[guild].groupType == MICROPHTYBENTHOS)
							Epifauna += initialWaterBiomass[FunctGroupArray[guild].totNTracers[cohort]] * smLayerThick;
						else
							Epifauna += initialEpiBiomass[FunctGroupArray[guild].totNTracers[cohort]];
					} else if (FunctGroupArray[guild].isInfauna == TRUE){
						Infauna += initialSedBiomass[FunctGroupArray[guild].totNTracers[cohort]];
					}
				}
			}
		}

		/* Infauna vs Epifauna ratio - convert infauna to m-2 from m-3 first.
		 Note that MB count as epifauna here as live on sediment surface
		 */

		InfEpiRatio = (Infauna * smLayerThick) / (Epifauna + small_num);
	}

	/****************************** ECOLOGICAL PROCESSES *********************************/
	/**
	 Determine habitat degradation scalar. For those cases where coastal development
	 and pollution have degraded the base available habitat for biogenic habitat
	 constituents such as the macrophytes and filter feeders.
	 **/
	
	//printf("Degradation scalar");
	
	for (kij = 0; kij < bm->K_num_bed_types; kij++) {
	  

		if (Box_degradedi[bm->current_box] && bm->flagdegrade) {
			BED_scale[kij] = Util_Get_Accumulative_Change_Scale(bm, BEDchange_max_num, BEDchange[kij]);
			
            /*
            if (bm->debug == clam_debug) {
				fprintf(llogfp, "Time: %e Habitat Scaling bed_type %d in box %d scaling factor = %e\n", bm->dayt, kij, bm->current_box, BED_scale[kij]);
			}
             */
		} else {
			BED_scale[kij] = 1.0;
		}
	}

	/**
	 Macrophytobenthos growth.
	 The specific growth rate mu is the product of a maximum growth rate, mum, a
	 light-limitation factor hI, and a nutrient limitation factor, hN. IRR is now
	 the light intensity at the sediment water interface.

	 Macroalgae get nutrients from water column:
	 **/
	boxLayerInfo->DIN = NO + NH + small_num; // Note add small so as notm dividing by zero at any point

	if (!boxLayerInfo->DIN) {
		boxLayerInfo->DIN = bm->min_pool;
		fprintf(llogfp, "Time: %e, box: %d, layer: %d had to reset DIN in epibenthic to minpool as was = 0\n", bm->dayt, bm->current_box, bm->current_layer);
		warn("Time: %e, box: %d, layer: %d had to reset DIN in epibenthic to minpool as was = 0\n", bm->dayt, bm->current_box, bm->current_layer);
	}

	/**
	 Seagrass get nutrients from the top sediment layer:
	 **/
	boxLayerInfo->DINs = NOs + NHs + small_num; // Note add small so as not dividing by zero at any point

	Calculate_PelagicBact_Scale(bm, boxLayerInfo);
	Calculate_SedBact_Scale(bm, EPIFAUNA, boxLayerInfo);

	/* now call the process function for each group in the sediment */
	for (index = 0; index < bm->K_num_tot_sp; index++) {
		guild = index;

		flag_sp = (int) (FunctGroupArray[guild].speciesParams[flag_id]);

		if (flag_sp && (FunctGroupArray[guild].isOncePerDt == FALSE || (it_count == 1 && FunctGroupArray[guild].isOncePerDt == TRUE))) {

			switch (FunctGroupArray[guild].groupAgeType) {

			case BIOMASS:	/* Intentional follow through */
			case AGE_STRUCTURED_BIOMASS:
				if (FunctGroupArray[guild].habitatCoeffs[EPIFAUNA] > 0 || FunctGroupArray[guild].groupType == CEP || FunctGroupArray[guild].groupType == PWN) {
					if(FunctGroupArray[guild].groupType == MICROPHTYBENTHOS)
						continue;
					for (cohort = 0; cohort < FunctGroupArray[guild].numCohortsXnumGenes; cohort++){
						Call_Group_Process_Function(bm, llogfp, EPIFAUNA, guild, cohort, boxLayerInfo);
					}
				}
				break;
			case AGE_STRUCTURED:
				/** Vertebrates	**
				 Pelagic fish which access the bottom layers where they can - as in epibenthic model
				 then know already that bm->current_layer == 0 (by definition) so no test required

				 All vertbrate activities now handled via Do_Vertebrate_Living() which loops over the
				 cohorts calling Vertebrate_Activities()
				 **/
					totSp = Do_Vertebrate_Living(bm, llogfp, guild, EPIFAUNA, boxLayerInfo, DLsed, DRsed, DCsed, PREYinfo, GRAZEinfo, CATCHGRAZEinfo, VERTinfo);
					boxLayerInfo->localWCTracers[FunctGroupArray[guild].totNTracers[0]] = totSp;
				break;
			}
		}
	}

	if (it_count == 1) {
		/* Bioturbation effects of feeding*/
		for (guild = 0; guild < bm->K_num_tot_sp; guild++) {
			if ( FunctGroupArray[guild].speciesParams[flag_id] && FunctGroupArray[guild].isVertebrate == FALSE) {
				if (FunctGroupArray[guild].groupType == SED_EP_OTHER || FunctGroupArray[guild].groupType == MOB_EP_OTHER) {
					Turbatn_contribs += FunctGroupArray[guild].speciesParams[KTUR_id] * FunctGroupArray[guild].grazing[0];
				}
			}
		}
		if (bm->debug == debug_prey_biology_process && bm->dayt >= bm->checkstart && bm->dayt < bm->checkstop) {
			fprintf(llogfp, "SED - Turbatn_contribs = %.20e\n", Turbatn_contribs);
		}
	}

	/* TODO: Not done here as already done in WC and SED?
	if(bm->track_atomic_ratio){
		Calculate_Phosphorus(bm, boxLayerInfo, EPIFAUNA);
		Calculate_Carbon(bm, boxLayerInfo, EPIFAUNA);
	}*/

	if(bm->track_contaminants){
        Change_Contaminant_Levels(bm, boxLayerInfo, EPIFAUNA, dtsz, 0);
	}
    
	/*******************************************************************************************
	 Reconcile global consumption for adaptive timestep
	 */
	Reconcile_Global_Values(bm, boxLayerInfo, EPIFAUNA, llogfp);

	/***********************************************************************************
	 Equations for rate of change of tracers:

	 Basic equation is as per normal.

	 Flux = growth - mortality - dead - eaten.

	 Note: if it_count > 1 and the group is a finer scale group then the deadGlobal value is used. This value will have
	 been calculated for this box/layer when it_count == 1.

	 */
	for (guild = 0; guild < bm->K_num_tot_sp; guild++) {
		isGlobal = (FunctGroupArray[guild].diagTol == 2 && it_count == 1);

		if(FunctGroupArray[guild].speciesParams[flag_id]){

			switch (FunctGroupArray[guild].groupAgeType) {
			case AGE_STRUCTURED:
				/* Do nothing here */
				break;
			case AGE_STRUCTURED_BIOMASS:	/* Intentional follow thur */
			case BIOMASS:
				/*
				 * tracerFlux = growth - lysis - mortality - dead - preyEaten
				 *
				 */

				if (FunctGroupArray[guild].isDetritus == FALSE) {
					//TODO: Check with Beth about this - BO values are only updated when it_count ==1 but the diagTol value is 1 so it
					// should be updated for each adaptive timestep - perhaps a bug in the original code.
					//if ((FunctGroupArray[guild].diagTol < 2 || (it_count == 1 && FunctGroupArray[guild].diagTol == 2)) && (!(guild == BOguild_id && it_count > 1))) {
					if ((FunctGroupArray[guild].diagTol < 2 || (it_count == 1 && FunctGroupArray[guild].diagTol == 2))) {

						boxLayerInfo->DebugFluxInfo[guild][WC][loss_id] = 0;
						boxLayerInfo->DebugFluxInfo[guild][WC][gain_id] = 0;

						if (FunctGroupArray[guild].habitatType == EPIFAUNA) {

							/* Get the total biomass */
							totalBiomass = 0.0;
							for (cohort = 0; cohort < FunctGroupArray[guild].numCohortsXnumGenes; cohort++) {
								totalBiomass += boxLayerInfo->localEPITracers[FunctGroupArray[guild].totNTracers[cohort]];
							}
							totalBiomass += small_num;

							for (cohort = 0; cohort < FunctGroupArray[guild].numCohortsXnumGenes; cohort++) {
								double dead;
								long double grazeEaten;

								/* Get the dead due to fishing - If this is a finer scale group - use the global values else use the values that were calculated in this time step. */
								if (FunctGroupArray[guild].diagTol < 2)
									dead = FunctGroupArray[guild].deadGlobal[cohort];
								else
									dead = FunctGroupArray[guild].dead[cohort];

								if(bm->flag_olddiet){
									if((FunctGroupArray[guild].numCohortsXnumGenes) > 1){
										grazeEaten = 0.0;
										for (ct = 0; ct < FunctGroupArray[guild].numCohortsXnumGenes; ct++){
											grazeEaten += FunctGroupArray[guild].preyEaten[ct][WC];
										}
										grazeEaten *= (boxLayerInfo->localWCTracers[FunctGroupArray[guild].totNTracers[cohort]] / totalBiomass);
									}else{
										grazeEaten = FunctGroupArray[guild].preyEaten[0][EPIFAUNA];
									}

								} else {
									grazeEaten = FunctGroupArray[guild].preyEaten[cohort][EPIFAUNA];
								}

								if(FunctGroupArray[guild].groupType == SEAGRASS && bm->flag_macro_model == TRUE){
									grazeEaten = FunctGroupArray[guild].preyEaten[0][cohort];   // TODO: check this is really around the right way
								}
								boxLayerInfo->localEPIFlux[FunctGroupArray[guild].totNTracers[cohort]] = FunctGroupArray[guild].growth[cohort]
										- FunctGroupArray[guild].mortality[cohort] - dead - (double)grazeEaten;

								if(bm->track_atomic_ratio == TRUE){
									/* Predation and growth due to predation is handled separately */
									Loose_Element(bm, boxLayerInfo, EPIFAUNA, guild, cohort,
											(dead + FunctGroupArray[guild].mortality[cohort]),
											EPIFAUNA, isGlobal);
								}

								/**
								if(bm->which_check == guild){
									fprintf(llogfp,"day: %f box%d-%d, %s-%d epiflux: %.20e grow: %.20e mort: %.20e dead: %.20e eaten: %Le preyEaten: %Le dead[]: %e, deadGlobal[]: %e\n",
										bm->dayt, bm->current_box, bm->current_layer, FunctGroupArray[guild].groupCode, cohort,
										boxLayerInfo->localEPIFlux[FunctGroupArray[guild].totNTracers[cohort]],
										FunctGroupArray[guild].growth[cohort], FunctGroupArray[guild].mortality[cohort],
										dead, grazeEaten, FunctGroupArray[guild].preyEaten[0][habitat_type], FunctGroupArray[guild].dead[cohort], FunctGroupArray[guild].deadGlobal[cohort]);
								}
								**/

								boxLayerInfo->DebugFluxInfo[guild][WC][gain_id] = FunctGroupArray[guild].growth[cohort];
								boxLayerInfo->DebugFluxInfo[guild][WC][loss_id] = FunctGroupArray[guild].mortality[cohort] + dead;

#ifdef BROKER_LINK_ENABLED
								if(do_BrokerLinkage){
									boxLayerInfo->localEPIFlux[FunctGroupArray[guild].totNTracers[cohort]] -= bm->linkageInterface->linkageEPIMortality[bm->current_box][guild][cohort] ;
									boxLayerInfo->DebugFluxInfo[guild][WC][loss_id] += bm->linkageInterface->linkageEPIMortality[bm->current_box][guild][cohort];
								}
#endif
							}
						}else{

							if (FunctGroupArray[guild].habitatCoeffs[WC] > 0) {

								/* Get the total biomass */
								totalBiomass = 0.0;
								for (cohort = 0; cohort < FunctGroupArray[guild].numCohortsXnumGenes; cohort++) {
									totalBiomass += boxLayerInfo->localWCTracers[FunctGroupArray[guild].totNTracers[cohort]];
								}
								totalBiomass += small_num;

								for (cohort = 0; cohort < FunctGroupArray[guild].numCohortsXnumGenes; cohort++) {
									long double grazeEaten;

									if(bm->flag_olddiet){
										grazeEaten = 0.0;
										if((FunctGroupArray[guild].numCohortsXnumGenes) > 1){
											for (ct = 0; ct < FunctGroupArray[guild].numCohortsXnumGenes; ct++){
												grazeEaten += FunctGroupArray[guild].preyEaten[ct][WC];
											}
											grazeEaten *= (boxLayerInfo->localWCTracers[FunctGroupArray[guild].totNTracers[cohort]] / totalBiomass);
										} else {
											grazeEaten = FunctGroupArray[guild].preyEaten[cohort][WC];
										}
									} else {
										grazeEaten = FunctGroupArray[guild].preyEaten[cohort][WC];
									}

									boxLayerInfo->localWCFlux[FunctGroupArray[guild].totNTracers[cohort]] +=
											FunctGroupArray[guild].growth[cohort] - FunctGroupArray[guild].mortality[cohort]
											- (double)grazeEaten;

									/**
									if(guild == bm->which_check){
										fprintf(llogfp,"Time: %e box%d-%d %s biom: %.20e, epiwcflux: %.20e, growth: %.20e, mort: %.20e, eaten: %Le\n",
												bm->dayt, bm->current_box, bm->current_layer, FunctGroupArray[guild].groupCode,
												boxLayerInfo->localWCTracers[FunctGroupArray[guild].totNTracers[cohort]],
												boxLayerInfo->localWCFlux[FunctGroupArray[guild].totNTracers[0]],
												FunctGroupArray[guild].growth[0], FunctGroupArray[guild].mortality[0], grazeEaten);
									}
									**/

									if(bm->track_atomic_ratio == TRUE){

										/* Predation and growth due to predation is handled separately */
										Loose_Element(bm, boxLayerInfo, WC, guild, cohort,
											(FunctGroupArray[guild].mortality[cohort]),
											EPIFAUNA, isGlobal);
									}

#ifdef BROKER_LINK_ENABLED
									if(do_BrokerLinkage){
										boxLayerInfo->localWCFlux[FunctGroupArray[guild].totNTracers[cohort]] -= bm->linkageInterface->linkageWCMortality[bm->current_box][bm->current_layer][guild][cohort];
									}
#endif
									boxLayerInfo->DebugFluxInfo[guild][WC][gain_id] += FunctGroupArray[guild].growth[cohort];
									boxLayerInfo->DebugFluxInfo[guild][WC][loss_id] += FunctGroupArray[guild].mortality[cohort] + (double)grazeEaten;

									if (FunctGroupArray[guild].secondNutrientTracerIndex > 0) {
										boxLayerInfo->localWCFlux[FunctGroupArray[guild].secondNutrientTracerIndex]
												+= -((double)FunctGroupArray[guild].preyEaten[cohort][WC] * FunctGroupArray[guild].secondNutrient);

									}
								}
							}
							if (FunctGroupArray[guild].habitatCoeffs[SED] > 0) {

								/* Get the total biomass */
								totalBiomass = 0.0;
								for (cohort = 0; cohort < FunctGroupArray[guild].numCohortsXnumGenes; cohort++) {
									totalBiomass += boxLayerInfo->localSEDTracers[FunctGroupArray[guild].totNTracers[cohort]];
								}
								totalBiomass += small_num;

								for (cohort = 0; cohort < FunctGroupArray[guild].numCohortsXnumGenes; cohort++) {
									long double grazeEaten;
									double dead;

									if (FunctGroupArray[guild].diagTol < 2)
										dead = FunctGroupArray[guild].deadGlobal[cohort];
									else
										dead = FunctGroupArray[guild].dead[cohort];

									if(bm->flag_olddiet){
										grazeEaten = 0.0;
										if((FunctGroupArray[guild].numCohortsXnumGenes) > 1){
											for (ct = 0; ct < FunctGroupArray[guild].numCohortsXnumGenes; ct++){
												grazeEaten += FunctGroupArray[guild].preyEaten[ct][SED];
											}
											grazeEaten *= (boxLayerInfo->localWCTracers[FunctGroupArray[guild].totNTracers[cohort]] / totalBiomass);
										} else {
											grazeEaten = FunctGroupArray[guild].preyEaten[cohort][SED];
										}
									} else {
										grazeEaten = FunctGroupArray[guild].preyEaten[cohort][SED];
									}
									boxLayerInfo->localSEDFlux[FunctGroupArray[guild].totNTracers[cohort]] += (-dead - (double)grazeEaten);

									if(bm->track_atomic_ratio == TRUE){

										/* Predation and growth due to predation is handled separately */
										Loose_Element(bm, boxLayerInfo, SED, guild, cohort,
												(dead),
												EPIFAUNA, isGlobal);
									}

									/**
									if(guild == bm->which_check){
										fprintf(llogfp,"Time: %e box%d-%d %s biom: %.20e, sedepiflux: %.20e, dead: %.20e, eaten: %Le\n",
												bm->dayt, bm->current_box, bm->current_layer, FunctGroupArray[guild].groupCode,
												boxLayerInfo->localSEDTracers[FunctGroupArray[guild].totNTracers[cohort]],
												boxLayerInfo->localSEDFlux[FunctGroupArray[guild].totNTracers[0]],
												dead, grazeEaten);
									}
									**/

									if (FunctGroupArray[guild].secondNutrientTracerIndex > 0) {
										boxLayerInfo->localSEDFlux[FunctGroupArray[guild].secondNutrientTracerIndex]
												+= -((double)FunctGroupArray[guild].preyEaten[cohort][SED] * FunctGroupArray[guild].secondNutrient);
									}
								}
							}
						}

						for (hab = WC; hab < bm->num_active_habitats; hab++){
							for (cohort = 0; cohort < FunctGroupArray[guild].numCohortsXnumGenes; cohort++) {
								boxLayerInfo->DebugFluxInfo[guild][WC][loss_id] += (double)FunctGroupArray[guild].preyEaten[cohort][hab];
							}
						}
					}
				}
				break;
			}
		}
	}

	/**
	 *  Nutrients in the water column
	 *
	 **/
	for (index = 0; index < bm->K_num_physiochem; index++) {
		if (PhysioChemArray[index].ROCFunction != NULL) {
			PhysioChemArray[index].ROCFunction(bm, llogfp, EPIFAUNA, index, boxLayerInfo);
			if(bm->ecotest > 1){
				/* Check the tracer values */
				if(!_finite(boxLayerInfo->localWCFlux[*PhysioChemArray[index].tracerIndex])){
					quit("Epibenthic_Box - Nutrient %s is infinite in watercolumn box %d:%d\n", PhysioChemArray[index].name, bm->current_box, bm->current_layer);
				}
				/* Check the tracer values */
				if(!_finite(boxLayerInfo->localSEDFlux[*PhysioChemArray[index].tracerIndex])){
					quit("Epibenthic_Box- Nutrient %s is infinite in sediment box %d:%d\n", PhysioChemArray[index].name, bm->current_box, bm->current_layer);
				}
			}

		}
	}

	/* if scaling mortality values handle the detritus groups now - just do it in one go */
	if(bm->scale_all_mortality == TRUE){
		Scale_Detritus_Mortality(bm, boxLayerInfo);
	}

	/**
	 * Labile detritis in the water column and sediment.
	 **/
	Labile_Detritus_ROC(bm, llogfp, EPIFAUNA, boxLayerInfo);

	/**
	 Refractory Detritus in the water column
	 **/
	Refractory_Detritus_ROC(bm, llogfp, EPIFAUNA, boxLayerInfo);

	if (it_count == 1) {

		Carrion_ROC(bm, llogfp, EPIFAUNA, boxLayerInfo);

		/**
		 Vertebrate fluxes
		 **/
        Update_Vertebrate_Tracers(bm, llogfp, EPIFAUNA, boxLayerInfo);

		/*** Update total catch and discards - for sediment and epibenthic groups only ***/
		if (bm->flag_fisheries_on) {
			Harvest_Update_Total_Catch(bm, boxLayerInfo->localFishTracers, EPIFAUNA, llogfp);
		}
	}

	/**
	 * If we are tracking the atomic tracers - calculate the flux values for this habitat before the arrays are cleared for the new habitat
	 */
	if(bm->track_atomic_ratio == TRUE){
		Calculate_Element_Flux(bm, boxLayerInfo->localWCTracers, boxLayerInfo->localWCFlux, WC, EPIFAUNA);
		Calculate_Element_Flux(bm, boxLayerInfo->localSEDTracers, boxLayerInfo->localSEDFlux, SED, EPIFAUNA);
		Calculate_Element_Flux(bm, boxLayerInfo->localEPITracers, boxLayerInfo->localEPIFlux, EPIFAUNA, EPIFAUNA);
	}

	/**
	 Diagnostic variables
	 **/

	/* Begin with those that remain unchanged whether there is
	 a single watercolumn layer or this is the bottom of
	 many layers. That is the epibenthic values which will
	 have zero values for overlying watercolumn anyway
	 */

	for (fgIndex = 0; fgIndex < bm->K_num_tot_sp; fgIndex++) {
		if(FunctGroupArray[fgIndex].speciesParams[flag_id]){

			if ((FunctGroupArray[fgIndex].isOncePerDt == TRUE && it_count == 1) || FunctGroupArray[fgIndex].isOncePerDt == FALSE) {
				switch (FunctGroupArray[fgIndex].groupAgeType) {
				case AGE_STRUCTURED:
					/* If not in surface layer then add to whats already there don't overwrite it */
					if (bm->current_layer < bm->boxes[bm->current_box].nz - 1) {
						for (cohort = 0; cohort < FunctGroupArray[fgIndex].numCohortsXnumGenes; cohort++) {
							boxLayerInfo->localDiagFlux[FunctGroupArray[fgIndex].EatTracers[cohort]] += (double)FunctGroupArray[fgIndex].GrazeLive[cohort];
							boxLayerInfo->localDiagFlux[FunctGroupArray[fgIndex].GrowthTracers[cohort]]
									+= FunctGroupArray[fgIndex].grow[cohort][SN_id] + FunctGroupArray[fgIndex].grow[cohort][RN_id];
						}
					} else {
						/* In the surface layer begin the record (nothing to append) */
						for (cohort = 0; cohort < FunctGroupArray[fgIndex].numCohortsXnumGenes; cohort++) {

							boxLayerInfo->localDiagFlux[FunctGroupArray[fgIndex].EatTracers[cohort]] = (double)FunctGroupArray[fgIndex].GrazeLive[cohort];
							boxLayerInfo->localDiagFlux[FunctGroupArray[fgIndex].GrowthTracers[cohort]] = FunctGroupArray[fgIndex].grow[cohort][SN_id]
									+ FunctGroupArray[fgIndex].grow[cohort][RN_id];
						}
					}
					break;
				case AGE_STRUCTURED_BIOMASS:
				case BIOMASS:
					if (FunctGroupArray[fgIndex].groupType != CEP) {
						for (cohort = 0; cohort < FunctGroupArray[fgIndex].numCohortsXnumGenes; cohort++) {
							if (FunctGroupArray[fgIndex].isGrazer) {
								boxLayerInfo->localDiagFlux[FunctGroupArray[fgIndex].GrazingTracers[0]] += FunctGroupArray[fgIndex].grazing[cohort];
							}
							if (FunctGroupArray[fgIndex].isProducer) {
								boxLayerInfo->localDiagFlux[FunctGroupArray[fgIndex].prodnTracers[0]] += FunctGroupArray[fgIndex].growth[cohort];
							}
						}
					}
					break;

				}
			}
		}
	}

	/* Do the mortality estimates */
	for (guild = 0; guild < bm->K_num_tot_sp; guild++) {
		if(FunctGroupArray[guild].speciesParams[flag_id] == TRUE){
			if (FunctGroupArray[guild].isVertebrate == TRUE) {
				/* For use in pseudo M estimate */
				for (cohort = 0; cohort < FunctGroupArray[guild].numCohortsXnumGenes; cohort++){
					bm->calcMnum[guild][current_id] += (double)FunctGroupArray[guild].preyEaten[cohort][WC]; // Summation here as have already been through WaterColumn
				}

			} else {
				/* For use in pseudo M estimate */
				bm->calcMnum[guild][current_id] += FunctGroupArray[guild].mortality[0]
					+ ((FunctGroupArray[guild].habitatType == EPIFAUNA) * (double)FunctGroupArray[guild].preyEaten[0][WC] )
					+ ((FunctGroupArray[guild].habitatType == EPIFAUNA) * (double)FunctGroupArray[guild].preyEaten[0][SED] )
					+ ((FunctGroupArray[guild].habitatType == EPIFAUNA) * (double)FunctGroupArray[guild].preyEaten[0][EPIFAUNA] );

			}
		}
	}

	/**
	 Determine habitat coverage
	 **/
	for (guild = 0; guild < bm->K_num_tot_sp; guild++) {
		if (FunctGroupArray[guild].speciesParams[flag_id] == TRUE && FunctGroupArray[guild].isCover == TRUE) {
			double biomass = 0;
			/* If the habitat type is epifauna use the that biomass - else if its sediment use sediment (covers the MB case) */
			switch (FunctGroupArray[guild].habitatType) {
			case WC:
				biomass = initialWaterBiomass[FunctGroupArray[guild].totNTracers[0]];
				break;
			case SED:
				biomass = initialSedBiomass[FunctGroupArray[guild].totNTracers[0]];
				break;
			case EPIFAUNA:
				biomass = 0.0;
				for (cohort = 0; cohort < FunctGroupArray[guild].numCohortsXnumGenes; cohort++) {
					biomass += initialEpiBiomass[FunctGroupArray[guild].totNTracers[cohort]];
				}
				break;
			case LAND_BASED:
			
				quit("Not yet supporting land based habitats");
				biomass = 0.0;
				for (cohort = 0; cohort < FunctGroupArray[guild].numCohortsXnumGenes; cohort++) {
					biomass += initialLandBiomass[FunctGroupArray[guild].totNTracers[cohort]];
				}
				break;
			case ICE_BASED:
				for (cohort = 0; cohort < FunctGroupArray[guild].numCohortsXnumGenes; cohort++) {
					biomass = initialIceBiomass[FunctGroupArray[guild].totNTracers[cohort]];
				}
				break;
            case MIXED:
                quit("How did we get here as should come through a primary habitat\n");
                break;
			}

			if (FunctGroupArray[guild].speciesParams[flag_lim_id] == simple_ben_lim) {
				boxLayerInfo->localEPITracers[FunctGroupArray[guild].CoverTracer] = min(1.0, biomass / FunctGroupArray[guild].speciesParams[max_id]);
			} else if (FunctGroupArray[guild].speciesParams[flag_lim_id] == ersem_ben_lim) {
				boxLayerInfo->localEPITracers[FunctGroupArray[guild].CoverTracer]
						= min(1.0, biomass / (50.0 * FunctGroupArray[guild].speciesParams[thresh_id]));
			}
		}
	}


	/**
	 Diagnostic variables and ecological indicators
	 **/

	if (it_count == 1) {
		boxLayerInfo->localDiagTracers[DivCount_i] += DivHere;
		boxLayerInfo->localDiagTracers[InfEpi_ratio_i] += InfEpiRatio;
	}

	boxLayerInfo->localSEDTracers[Stress_i] = surf_stress;

	/** check diagnostics **/
	if (bm->which_check < not_tracking_flux) {

		/* Only += here as done basic assignment (=) in Water_Column_Box() or Sediment_Box() */
		boxLayerInfo->localWCTracers[DiagNGain_i] += boxLayerInfo->DebugFluxInfo[bm->which_check][bm->habitat_check][gain_id] * dtsz;
		boxLayerInfo->localWCTracers[DiagNLoss_i] += boxLayerInfo->DebugFluxInfo[bm->which_check][bm->habitat_check][loss_id] * dtsz;
		boxLayerInfo->localWCTracers[DiagNFlux_i] += (boxLayerInfo->DebugFluxInfo[bm->which_check][bm->habitat_check][gain_id]
				- boxLayerInfo->DebugFluxInfo[bm->which_check][bm->habitat_check][loss_id]) * dtsz;

	}

	if ((bm->checkNH || bm->checkDL || bm->checkDR) && ((bm->checkbox > bm->nbox) || (bm->current_box == bm->checkbox)) && (bm->dayt >= bm->checkstart))
		Call_Diagnostics(bm, boxLayerInfo, EPIFAUNA, llogfp, 0);

	if (bm->checkbiom && ((bm->checkbox > bm->nbox) || (bm->current_box == bm->checkbox)) && (bm->dayt >= bm->checkstart)) {
		Ecology_Calculate_Total_Abundance(bm, bm->dt, 1, llogfp);
	}

	smFlux1 = 0;
	smFlux2 = 0;
	wcFlux1 = 0;
	wcFlux2 = 0;
	wcFlux3 = 0;
	wcFlux4 = 0;
	//wcFishingGlobal = 0;
	epiFlux1 = 0;
	epiFlux2 = 0;
	//epiFishingGlobal = 0;

	/** test balance **/
	if (bm->ecotest > 1) {

		wcFlux1 = boxLayerInfo->localWCFlux[NH3_i] + boxLayerInfo->localWCFlux[NO3_i] + boxLayerInfo->localWCFlux[DON_i];

		smFlux1 = (boxLayerInfo->localSEDFlux[NH3_i] + boxLayerInfo->localSEDFlux[NO3_i] + boxLayerInfo->localSEDFlux[DON_i]) * sporosity
				+ boxLayerInfo->localSEDFlux[Denitrification_i];
        
		epiFlux1 = 0.0;
		wcFlux2 = 0.0;
		wcFlux3 = 0.0;
		wcFlux4 = 0.0;
		smFlux2 = 0.0;

		for (guild = 0; guild < bm->K_num_tot_sp; guild++) {
			if (FunctGroupArray[guild].speciesParams[flag_id]){
				switch (FunctGroupArray[guild].groupAgeType) {
				case BIOMASS:
				case AGE_STRUCTURED_BIOMASS:
					if (FunctGroupArray[guild].isOncePerDt == TRUE) {

						if (it_count == 1) {
							for (cohort = 0; cohort < FunctGroupArray[guild].numCohortsXnumGenes; cohort++){
								if(FunctGroupArray[guild].groupType == CEP || FunctGroupArray[guild].groupType == PWN) {

									wcFlux3 += boxLayerInfo->localWCFlux[FunctGroupArray[guild].totNTracers[cohort]];
								}
								else{
									if (FunctGroupArray[guild].habitatType == EPIFAUNA) {
										if (((FunctGroupArray[guild].groupType != CORAL) && (FunctGroupArray[guild].groupType != SPONGE)) || !bleaching_has_occurred[guild]) {
											epiFlux2 += boxLayerInfo->localEPIFlux[FunctGroupArray[guild].totNTracers[cohort]];
										 }
									} else {
										wcFlux4 += boxLayerInfo->localWCFlux[FunctGroupArray[guild].totNTracers[cohort]];
										smFlux2 += boxLayerInfo->localSEDFlux[FunctGroupArray[guild].totNTracers[cohort]];
									}
								}
							}
						} else {
							wcFlux3 = wcFlux3aglobal;
							wcFlux4 = wcFlux4global;
							smFlux2 = smFlux2global;
							epiFlux2 = epiFlux2global;
						}
					} else {
						if (FunctGroupArray[guild].habitatType == EPIFAUNA) {
                            if (((FunctGroupArray[guild].groupType != CORAL) && (FunctGroupArray[guild].groupType != SPONGE)) || !bleaching_has_occurred[guild]) {
                                /* allow for multiple seagrass cohorts */
                                for (cohort = 0; cohort < FunctGroupArray[guild].numCohortsXnumGenes; cohort++){
                                    epiFlux1 += boxLayerInfo->localEPIFlux[FunctGroupArray[guild].totNTracers[cohort]];
                                 }
							}
						} else {
							wcFlux1 += boxLayerInfo->localWCFlux[FunctGroupArray[guild].totNTracers[0]];
							smFlux1 += boxLayerInfo->localSEDFlux[FunctGroupArray[guild].totNTracers[0]];
						}

					}
					break;
				case AGE_STRUCTURED:
					if (it_count == 1) {
						for (cohort = 0; cohort < FunctGroupArray[guild].numCohortsXnumGenes; cohort++)
							wcFlux2 += (FunctGroupArray[guild].grow[cohort][SN_id] + FunctGroupArray[guild].grow[cohort][RN_id])
									* VERTinfo[guild][cohort][DEN_id] / bm->cell_vol + (FunctGroupArray[guild].dead[cohort]
									+ (double)FunctGroupArray[guild].preyEaten[cohort][WC]) * (VERTinfo[guild][cohort][SN_id] + VERTinfo[guild][cohort][RN_id]) / bm->cell_vol;
					} else {
						wcFlux2 = wcFlux2aglobal;
					}
					break;

				}
			}
		}

		/* Now scale the epibenthic tracers */
		epiFlux1 = epiFlux1 / smLayerThick;

		if (it_count == 1) {

			epiFlux2 = epiFlux2 / smLayerThick;

			/* Amount removed from the system by fishing */
			epiFishing = 0;

			if (bm->flag_fisheries_on){
				for (guild = 0; guild < bm->K_num_tot_sp; guild++) {
					if (FunctGroupArray[guild].speciesParams[flag_id] && FunctGroupArray[guild].isImpacted == TRUE) {
						/* Skip over those done in Epibenthic_Box() */
						/* select the vertebrate, age structured groups and large zooplankton */
						if (FunctGroupArray[guild].habitatType == EPIFAUNA || FunctGroupArray[guild].groupType == LG_INF || FunctGroupArray[guild].groupType
								== MICROPHTYBENTHOS) {
							for (nf = 0; nf < bm->K_num_fisheries; nf++) {
								if(!FisheryArray[nf].isRec){
									epiFishing += bm->Catch[bm->current_box][guild][nf][bm->current_layer] / (bm->cell_vol * bm->dt + small_num);
								} else {
									epiFishing += bm->RecCatch[bm->current_box][guild][nf] / (bm->cell_vol * bm->dt + small_num);
								}
							}
						}
					}
				}
			}

			epiFishingGlobal = epiFishing;

			wcFlux2aglobal = wcFlux2;
			wcFlux3aglobal = wcFlux3;
			wcFlux4global = wcFlux4;
			smFlux2global = smFlux2;
			epiFlux2global = epiFlux2;
		} else {
			wcFlux2 = wcFlux2aglobal;
			wcFlux3 = wcFlux3aglobal;
			wcFlux4 = wcFlux4global;
			smFlux2 = smFlux2global;
			epiFlux2 = epiFlux2global;
			epiFishing = epiFishingGlobal;
		}

		/* Using wcFishingGlobal from WaterColumBox()- as all non-epibenthic fishing happens in Water_Column_Box() */
		wcFlux = wcFlux1 + wcFlux2 + wcFlux3 + wcFlux4 - wcFishingGlobal;
		smFlux = smFlux1 + smFlux2;
		epiFlux = epiFlux1 + epiFlux2 - epiFishingGlobal;

		TotFlux = wcFlux + smFlux + epiFlux;

		if (fabs(TotFlux) > Flux_tol) {
			/*Figure out the culprit */
			Max_Flux_Check(bm, EPIFAUNA, &flux_sp, &max_flux_id, &flux_model_id, &fluxnum, boxLayerInfo->localWCFlux, boxLayerInfo-> localSEDFlux,
					boxLayerInfo->localEPIFlux, llogfp);
			if (flux_model_id > 0) {
				switch (flux_model_id) {
				case WC:
					fprintf(llogfp, "Epibenthic_Box: biggest watercolumn flux was wcFlux[%s] = %e\n", FunctGroupArray[flux_sp].groupCode,
							boxLayerInfo->localWCFlux[max_flux_id]);
					break;
				case SED:
					fprintf(llogfp, "Epibenthic_Box: biggest sediment flux was sedFlux[%s] = %e\n", FunctGroupArray[flux_sp].groupCode,
							boxLayerInfo->localSEDFlux[max_flux_id]);
					break;
				case EPIFAUNA:
					fprintf(llogfp, "Epibenthic_Box: biggest epibenthic flux was epiFlux[%s] = %e\n", FunctGroupArray[flux_sp].groupCode,
							boxLayerInfo->localEPIFlux[max_flux_id]);
					break;
				case LAND_BASED:
					fprintf(llogfp, "Epibenthic_Box: biggest land flux was landFlux[%s] = %e\n", FunctGroupArray[flux_sp].groupCode,
							boxLayerInfo->localLANDFlux[max_flux_id]);
					break;
				case ICE_BASED:
					/* Do nothing */
					break;
				case growth_id:
					fprintf(llogfp, "Epibenthic_Box: biggest flux is due to the growth of %s-%d\n", FunctGroupArray[flux_sp].groupCode, fluxnum);
					break;
				case death_id:
					fprintf(llogfp, "Epibenthic_Box: biggest flux is due to the death of %s-%d\n", FunctGroupArray[flux_sp].groupCode, fluxnum);
					break;
				default:
					quit("No such flux_model defined - how did it get here in the Epibenthic_Box calculations?\n");
					break;
				}
			} else
				fprintf(llogfp, "Can't give details of imbalance as flux location mis-set\n");

			/* Check diagnostic terms */
			if ((bm->checkNH || bm->checkDL || bm->checkDR) && ((bm->checkbox > bm->nbox) || (bm->current_box == bm->checkbox)) && (bm->dayt >= bm->checkstart))
				Call_Diagnostics(bm, boxLayerInfo, EPIFAUNA, llogfp, 0);

			/* List all figures */
			fprintf(llogfp, "Epibenthic_Box\n");
			Print_Flux(bm, EPIFAUNA, VERTinfo, noqnan_n_fail, wcFlux, wcFlux1, wcFlux2, wcFlux3, wcFlux4, smFlux, epiFlux, TotFlux, epiFishing,
					boxLayerInfo->localWCFlux, boxLayerInfo->localSEDFlux, boxLayerInfo->localEPIFlux, llogfp);

			Textfile_Dump(bm, llogfp);
			quit("Error of flux balance in Epibenthic box: %d, on day: %e, wcflux: %e, smflux: %e, epiflux: %e, Totflux: %e\n", bm->current_box, bm->dayt,
					wcFlux, smFlux, epiFlux, TotFlux);
		}
	}

	if ((verbose && (bm->current_box == bm->checkbox) && (bm->dayt >= bm->checkstart)) || (bm->debug == debug_prey_biology_process && bm->dayt
			>= bm->checkstart && bm->dayt < bm->checkstop)) {
		/* Print out each flux */
		fprintf(llogfp, "debug Epibenthic_Box\n");
		Print_Flux(bm, EPIFAUNA, VERTinfo, noqnanverbose, wcFlux, wcFlux1, wcFlux2, wcFlux3, wcFlux4, smFlux, epiFlux, TotFlux, epiFishing,
				boxLayerInfo->localWCFlux, boxLayerInfo->localSEDFlux, boxLayerInfo->localEPIFlux, llogfp);

		fprintf(llogfp, "Flux results box: %d, layer: %d, on day: %e, wcflux: %e, smflux: %e, epiflux: %e, Totflux: %e\n", bm->current_box, bm->current_layer,
				bm->dayt, wcFlux, smFlux, epiFlux, TotFlux);

		fprintf(llogfp, "Prey eaten values at end of epibenthic column box \n");

		Print_Eat_Diagnostics(bm, llogfp, 0, EPIFAUNA, 1);
	}

	if (!(_finite(boxLayerInfo->localWCFlux[NH3_i])) || !(_finite(boxLayerInfo->localSEDFlux[NH3_i])) || (FunctGroupArray[SedBactIndex].speciesParams[flag_id] && !(_finite(
			boxLayerInfo->localSEDFlux[FunctGroupArray[SedBactIndex].totNTracers[0]])))) {
		/* Check diagnostic terms */
		Call_Diagnostics(bm, boxLayerInfo, EPIFAUNA, llogfp, 1);

		/* Print out each flux */
		Print_Flux(bm, EPIFAUNA, VERTinfo, qnancheck, wcFlux, wcFlux1, wcFlux2, wcFlux3, wcFlux4, smFlux, epiFlux, TotFlux, epiFishing,
				boxLayerInfo->localWCFlux, boxLayerInfo->localSEDFlux, boxLayerInfo-> localEPIFlux, llogfp);

		Textfile_Dump(bm, llogfp);
        quit("Epibenthic_Box: Got an QNAN result box: %d, on day: %e, wcflux: %e, smflux: %e, epiflux: %e, Totflux: %e\n", bm->current_box, bm->dayt, wcFlux, smFlux, epiFlux,
				TotFlux);
	}

}



/*****SEDIMENT MODULE.******************************************************************//**

 \brief This module calculates rate of change of tracers due to biogeochemical
 processes in ice.

 This routine is the ecological model for ice cells only.



 ************/
void Ice_Box(MSEBoxModel *bm, double dtsz, BoxLayerValues *boxLayerInfo, FILE *llogfp) {

	/** Define local variables and data **/
	//int cohort;
	/* Define biomass names */
	double  NH, NO;

//DON	double DON;
	/* Define nutrient and primary producer names */

	/* Define detritus names */
	//double   DONremin;

	/* Define nitrification-denitrification names */
	//double /*Remin, */ReminNet, /*Nitrification, */
	//Nitrific_eff, Denitrific_eff;
	//Denitrification;

	/* Define pool, flux and flag names */
	//double iceFlux = 0;

	double  DivHere = 0;
	int max_flux_id = -1;
	int flux_model_id = -1;
	int flux_sp = -1;
	int fluxnum = -1;

	int guild, flag_sp, fished_chrt, index, cohort;

//	int max_flux_id = -1;
	//int flux_model_id = -1;
	//int flux_sp = -1;
	//int fluxnum = -1;
	HABITAT_TYPES habitat_type = ICE_BASED;
	int i, hab;
	int isGlobal;
	double iceFlux = 0.0;
    
    if (verbose > 1) {
        printf("Ice_Box for box %d-%d\n", bm->current_box, bm->current_layer);
    }

	boxLayerInfo->sDLscale = 1.0;
	boxLayerInfo->sDRscale = 1.0;
	boxLayerInfo->sDCscale = 1.0;


	/* Initialise diagnostic tracer */
	Reset_Arrays(bm, SED, it_count, boxLayerInfo, llogfp);


	/* Initialise diagnostic tracer */
	if (it_count == 1) {
		boxLayerInfo->localICETracers[DiagNGain_i] = 0;
		boxLayerInfo->localICETracers[DiagNLoss_i] = 0;
		boxLayerInfo->localICETracers[DiagNFlux_i] = 0;
	}

	if ((verbose > 1) || ((bm->debug == debug_biology_process) && (bm->dayt >= bm->checkstart) && (bm->dayt < bm->checkstop)))
		fprintf(llogfp, "\nIce processes running now (it_count = %d), box = %d, layer = %d, t = %e\n", it_count, bm->current_box, bm->current_layer,
				bm->dayt);

	/**
	 Begin by transferring tracer values from array argument to local variables
	 (see Table above):
	 **/
    Util_Init_1D_Double(initialBiomass, bm->ntracer, 0.0);
	for (i = 0; i < bm->ntracer; i++){
		initialBiomass[i] = boxLayerInfo->localICETracers[i];
	}

	//DON = boxLayerInfo->localICETracers[DON_i];
	NH = boxLayerInfo->localICETracers[NH3_i];
	NO = boxLayerInfo->localICETracers[NO3_i];

	for (guild = 0; guild < bm->K_num_tot_sp; guild++) {
		if (FunctGroupArray[guild].isVertebrate == FALSE && FunctGroupArray[guild].habitatType == SED && (FunctGroupArray[guild].groupType
				== MICROPHTYBENTHOS || FunctGroupArray[guild].groupType == LG_PHY || FunctGroupArray[guild].groupType == DINOFLAG
				|| FunctGroupArray[guild].groupType == SM_PHY)) {
			if (FunctGroupArray[guild].Light_Adaptn_TracerID > 0)
				FunctGroupArray[guild].SP_IRR = boxLayerInfo->localICETracers[FunctGroupArray[guild].Light_Adaptn_TracerID];
			else
				FunctGroupArray[guild].SP_IRR = 1.0;
		}
	}
	/**
	 Construct PREYinfo vector (for use in Eat()
	 **/
	for (guild = 0; guild < bm->K_num_tot_sp; guild++) {
		if(FunctGroupArray[guild].speciesParams[flag_id]){
			if (FunctGroupArray[guild].isVertebrate == FALSE) {

				for(hab = 0; hab < bm->num_active_habitats; hab++){
					PREYinfo[guild][0][hab] = 0.0;
				}

				if (FunctGroupArray[guild].groupAgeType == AGE_STRUCTURED_BIOMASS) {
					for (cohort = 0; cohort < FunctGroupArray[guild].numCohorts; cohort++) {
						PREYinfo[guild][cohort][ICE_BASED] += boxLayerInfo->localICETracers[FunctGroupArray[guild].totNTracers[cohort]]
								* FunctGroupArray[guild].habitatCoeffs[ICE_BASED];
					}
				} else {
					PREYinfo[guild][0][ICE_BASED] = boxLayerInfo->localICETracers[FunctGroupArray[guild].totNTracers[0]] * FunctGroupArray[guild].habitatCoeffs[ICE_BASED];
				}
			}
		}
	}

	/* Check that the prey values are finite */
	Check_Prey_Is_Finite(bm, llogfp, habitat_type);

	/***
	 Note: no detritus availability scaling as all detritvores here are specialised for it,
	 not supplementing omnivorous diet
	 ***/

	/****************************** ECOLOGICAL INDICATORS *******************************/
	if (it_count == 1) {
		DivHere = 0;

		/* Local diversity proxy */
		for (guild = 0; guild < bm->K_num_tot_sp; guild++) {
			if (FunctGroupArray[guild].habitatType == ICE_BASED) {
				if (FunctGroupArray[guild].speciesParams[flag_id] && (initialBiomass[FunctGroupArray[guild].totNTracers[0]] > bm->min_pool))
					DivHere++;
			}

		}
	}

	/**
	 * Initial the invertebrate variables.
	 */
	Init_Invert_Properties(bm);

	/****************************** ECOLOGICAL PROCESSES *********************************/
	/**
	 DIN - store here so we can use it later in the group process functions
	 **/
	boxLayerInfo->DIN = NO + NH;

	if (!boxLayerInfo->DIN) {
		boxLayerInfo->DIN = bm->min_pool;
		fprintf(llogfp, "Time: %e, box: %d, layer: %d had to reset DIN in ice to minpool as was = 0\n", bm->dayt, bm->current_box, bm->current_layer);
		warn("Time: %e, box: %d, layer: %d had to reset DIN in ice to minpool as was = 0\n", bm->dayt, bm->current_box, bm->current_layer);
	}

	/*
	 * Calculate the ice bacteria partitioning.
	 */
	Calculate_IceBact_Scale(bm, ICE_BASED, boxLayerInfo);


	/* now call the process function for each group in the ice */
	for (guild = 0; guild < bm->K_num_tot_sp; guild++) {

		if (guild != -1) {
			flag_sp = (int) (FunctGroupArray[guild].speciesParams[flag_id]);

			if (flag_sp && (FunctGroupArray[guild].isOncePerDt == FALSE || (it_count == 1 && FunctGroupArray[guild].isOncePerDt == TRUE))) {
				switch (FunctGroupArray[guild].groupAgeType) {

				case BIOMASS:
					if (FunctGroupArray[guild].habitatCoeffs[ICE_BASED] > 0) {
						Call_Group_Process_Function(bm, llogfp, ICE_BASED, guild, 0, boxLayerInfo);
					}

					break;
				case AGE_STRUCTURED_BIOMASS:
				case AGE_STRUCTURED:
					if (FunctGroupArray[guild].habitatCoeffs[ICE_BASED] > 0) {
						quit("Age structured ICE_BASED groups not yet supported. Contact the Atlantis developers for more information\n");
					}
					/* do nothing */
					break;
				}
			}
		}
	}


	if(bm->track_atomic_ratio){
		Calculate_Phosphorus(bm, boxLayerInfo, ICE_BASED);
		Calculate_Carbon(bm, boxLayerInfo, ICE_BASED);
	}

	/*******************************************************************************************
	 Reconcile global consumption for adaptive timestep
	 */
	Reconcile_Global_Values(bm, boxLayerInfo, ICE_BASED, llogfp);

	/**********************************************************************************
	 The equations for rate of change of tracers are:

	 */
	for (guild = 0; guild < bm->K_num_tot_sp; guild++) {
		if(FunctGroupArray[guild].speciesParams[flag_id]){

			if(FunctGroupArray[guild].isDetritus == TRUE)
				continue;

			isGlobal = (FunctGroupArray[guild].diagTol == 2 && it_count == 1);

			switch (FunctGroupArray[guild].groupAgeType) {
			case AGE_STRUCTURED:
				/* Do nothing here */
				break;
			case AGE_STRUCTURED_BIOMASS:
				/* Do nothing here */
				break;
			case BIOMASS:
				/*
				 * tracerFlux = growth - lysis - mortality - dead - preyEaten
				 *
				 */
				if (FunctGroupArray[guild].habitatCoeffs[ICE_BASED] > 0) {
					if (FunctGroupArray[guild].diagTol < 2 || (it_count == 1 && FunctGroupArray[guild].diagTol == 2)) {
						boxLayerInfo->localICEFlux[FunctGroupArray[guild].totNTracers[0]]
						                           = FunctGroupArray[guild].growth[0]
						                           - FunctGroupArray[guild].lysis[0]
						                           - FunctGroupArray[guild].mortality[0]
						                           - FunctGroupArray[guild].dead[0]
						                           - (double)FunctGroupArray[guild].preyEaten[0][habitat_type];

						if(bm->track_atomic_ratio == TRUE){

							/* Predation and growth due to is handled separately */
							Loose_Element(bm, boxLayerInfo, ICE_BASED, guild, 0,
									(		FunctGroupArray[guild].dead[0] +
											FunctGroupArray[guild].lysis[0] +
											FunctGroupArray[guild].mortality[0]),
											ICE_BASED, isGlobal);

						}

						/* If we are damping the biomass of this group then do so */
						if (FunctGroupArray[guild].speciesParams[flux_thresh_id] > 0) {
							/* If the biomass value is above the threshhold then limit */
							if (boxLayerInfo->localICEFlux[FunctGroupArray[guild].totNTracers[0]] > FunctGroupArray[guild].speciesParams[flux_thresh_id]) {
								boxLayerInfo->localICEFlux[FunctGroupArray[guild].totNTracers[0]]
										= boxLayerInfo->localICEFlux[FunctGroupArray[guild].totNTracers[0]] * FunctGroupArray[guild].speciesParams[flux_damp_id];
							}
						}

						boxLayerInfo->DebugFluxInfo[guild][habitat_type][gain_id] = FunctGroupArray[guild].growth[0];
						boxLayerInfo->DebugFluxInfo[guild][habitat_type][loss_id] = FunctGroupArray[guild].mortality[0]
								+ (double)FunctGroupArray[guild].preyEaten[0][habitat_type];

						if (FunctGroupArray[guild].isProducer) {
							boxLayerInfo->localDiagFlux[FunctGroupArray[guild].prodnTracers[0]] += FunctGroupArray[guild].growth[0];
						}
						if (FunctGroupArray[guild].isGrazer) {
							boxLayerInfo->localDiagFlux[FunctGroupArray[guild].GrazingTracers[0]] += FunctGroupArray[guild].uptakeDL
									+ FunctGroupArray[guild].uptakeDR + FunctGroupArray[guild].grazing[0]
									+ FunctGroupArray[guild].maxPhagotrophy;
						}

#ifdef BROKER_LINK_ENABLED
						if(do_BrokerLinkage){
							if (FunctGroupArray[guild].isDetritus == FALSE) {
								boxLayerInfo->localSEDFlux[FunctGroupArray[guild].totNTracers[0]]
										+= -bm->linkageInterface->linkageSEDMortality[bm->current_box][bm->current_layer][guild][0];

								boxLayerInfo->DebugFluxInfo[guild][habitat_type][loss_id] += bm->linkageInterface->linkageSEDMortality[bm->current_box][bm->current_layer][guild][0];

							} else {

								boxLayerInfo->localSEDFlux[FunctGroupArray[guild].totNTracers[0]] += +bm->linkageInterface->linkageSEDDetritusFlux[bm->current_box][bm->current_layer][guild];

								boxLayerInfo->DebugFluxInfo[guild][habitat_type][loss_id] += bm->linkageInterface->linkageSEDDetritusFlux[bm->current_box][bm->current_layer][guild];
							}
						}
#endif
						if (FunctGroupArray[guild].secondNutrientTracerIndex > 0) {
							boxLayerInfo->localICEFlux[FunctGroupArray[guild].secondNutrientTracerIndex]
									= boxLayerInfo->localICEFlux[FunctGroupArray[guild].totNTracers[0]] * FunctGroupArray[guild].secondNutrient;
						}
					}
				}
				break;
			}
		}
	}
	/**
	 *  Nutrients in the water column
	 *
	 **/
	for (index = 0; index < bm->K_num_physiochem; index++) {
		if (PhysioChemArray[index].ROCFunction != NULL) {
			PhysioChemArray[index].ROCFunction(bm, llogfp, ICE_BASED, index, boxLayerInfo);
			if(bm->ecotest > 1){
				/* Check the tracer values */
				if(!_finite(boxLayerInfo->localICEFlux[*PhysioChemArray[index].tracerIndex])){
					quit("Ice - Nutrient %s is infinite in box %d:%d\n", PhysioChemArray[index].name, bm->current_box, bm->current_layer);
				}
			}

		}
	}


	/**
	 Diagnostic variables and ecological indicators
	 **/
	boxLayerInfo->localICETracers[Chl_a_i] = 0.0;
	for (guild = 0; guild < bm->K_num_tot_sp; guild++)
		if (FunctGroupArray[guild].isVertebrate == FALSE)
			boxLayerInfo->localICETracers[Chl_a_i] += FunctGroupArray[guild].chl;

	boxLayerInfo->localICETracers[Chl_a_i] = boxLayerInfo->localICETracers[Chl_a_i] / X_CHLN;

	if (it_count == 1) {
		boxLayerInfo->localDiagTracers[DivCount_i] += DivHere;
	}

	/**
	 * If we are tracking the atomic tracers - calculate the flux values for this habitat before the arrays are cleared for the new habitat
	 */
	if(bm->track_atomic_ratio == TRUE){
		Calculate_Element_Flux(bm, boxLayerInfo->localICETracers, boxLayerInfo->localICEFlux, ICE_BASED, ICE_BASED);
	}

	/* Update mortality estimates for ice invertebrates */
	for (guild = 0; guild < bm->K_num_tot_sp; guild++) {
		if(FunctGroupArray[guild].speciesParams[flag_id]){
			if (FunctGroupArray[guild].isVertebrate == FALSE) {

				fished_chrt = 0;
				/* For use in pseudo M estimate */
					bm->calcMnum[guild][current_id] +=
							(FunctGroupArray[guild].mortality[fished_chrt] + (double)FunctGroupArray[guild].preyEaten[0][ICE_BASED]);
			}
		}
	}


	if (bm->which_check < not_tracking_flux && bm->habitat_check == ICE_BASED) {

		if (it_count == 1) {
			boxLayerInfo->localICETracers[DiagNGain_i] = boxLayerInfo->DebugFluxInfo[bm->which_check][bm->habitat_check][gain_id] * dtsz;
			boxLayerInfo->localICETracers[DiagNLoss_i] = boxLayerInfo->DebugFluxInfo[bm->which_check][bm->habitat_check][loss_id] * dtsz;
			boxLayerInfo->localICETracers[DiagNFlux_i] = (boxLayerInfo->DebugFluxInfo[bm->which_check][bm->habitat_check][gain_id]
					- boxLayerInfo->DebugFluxInfo[bm->which_check][bm->habitat_check][loss_id]) * dtsz;

		} else {
			boxLayerInfo->localICETracers[DiagNGain_i] += boxLayerInfo->DebugFluxInfo[bm->which_check][bm->habitat_check][gain_id] * dtsz;
			boxLayerInfo->localICETracers[DiagNLoss_i] += boxLayerInfo->DebugFluxInfo[bm->which_check][bm->habitat_check][loss_id] * dtsz;
			boxLayerInfo->localICETracers[DiagNFlux_i] += (boxLayerInfo->DebugFluxInfo[bm->which_check][bm->habitat_check][gain_id]
					- boxLayerInfo->DebugFluxInfo[bm->which_check][bm->habitat_check][loss_id]) * dtsz;
		}
	}


	if ((bm->checkNH || bm->checkDL || bm->checkDR) && ((bm->dayt >= bm->checkstart) && ((bm->checkbox > bm->nbox) || (bm->current_box == bm->checkbox))))
		Call_Diagnostics(bm, boxLayerInfo, ICE_BASED, llogfp, 0);

	if (bm->checkbiom && ((bm->checkbox > bm->nbox) || (bm->current_box == bm->checkbox)) && (bm->dayt >= bm->checkstart)) {
		Ecology_Calculate_Total_Abundance(bm, bm->dt, 1, llogfp);
	}

	/* test flux balance */
	if (bm->ecotest > 1) {

		iceFlux = 0.0;

		for (guild = 0; guild < bm->K_num_tot_sp; guild++) {
			if(FunctGroupArray[guild].speciesParams[flag_id]){
				if (FunctGroupArray[guild].habitatCoeffs[ICE_BASED] > 0 ){
					iceFlux += boxLayerInfo->localSEDFlux[FunctGroupArray[guild].totNTracers[0]];
				}
			}
		}

//		smFlux += (boxLayerInfo->localSEDFlux[NH3_i] * sporosity);
//		smFlux += (boxLayerInfo->localSEDFlux[NO3_i] * sporosity);
//		smFlux += (boxLayerInfo->localSEDFlux[DON_i] * sporosity);
//		smFlux += (boxLayerInfo->localSEDFlux[Denitrification_i]);

		if (fabs(iceFlux) > Flux_tol) {

			/* Figure out the culprit */
			Max_Flux_Check(bm, ICE_BASED, &flux_sp, &max_flux_id, &flux_model_id, &fluxnum, boxLayerInfo->localICEFlux, boxLayerInfo->localICEFlux,
					boxLayerInfo->localICEFlux, llogfp);
			if (flux_model_id > 0) {
				switch (flux_model_id) {
				case SED:
					fprintf(llogfp, "Ice_Box: biggest ice flux was iceFlux[%s] = %e\n", FunctGroupArray[flux_sp].groupCode,
							boxLayerInfo->localICEFlux[max_flux_id]);
					break;
				case WC:
				case EPIFAUNA:
				case LAND_BASED:
				case growth_id:
				case death_id:
					fprintf(llogfp, "ICE_Box: Can't give details of imbalance as flux location mis-set\n");
					break;
				default:
					quit("No such flux_model defined - how did it get here in the ICE_Box calculations?\n");
					break;
				}
			} else
				fprintf(llogfp, "Can't give details of imbalance as flux location mis-set\n");

			/* Check diagnostic terms */
			if ((bm->checkNH || bm->checkDL || bm->checkDR) && ((bm->current_box == bm->checkbox) || (bm->dayt >= bm->checkstart)))
				Call_Diagnostics(bm, boxLayerInfo, ICE_BASED, llogfp, 0);

			/* List all figures */
			fprintf(llogfp, "ICE_Box\n");

			Print_Flux(bm, ICE_BASED, VERTinfo, noqnan_n_fail, 0, 0, 0, 0, 0, iceFlux, 0, iceFlux, 0, boxLayerInfo->localICEFlux, boxLayerInfo->localICEFlux,
					boxLayerInfo->localICEFlux, llogfp);
			Textfile_Dump(bm, llogfp);
			quit("Error of flux balance in ice box: %d on day: %e, iceflux: %e\n", bm->current_box, bm->dayt, iceFlux);
		}

		if ((verbose && (bm->current_box == bm->checkbox) && (bm->dayt >= bm->checkstart)) || (bm->debug == debug_prey_biology_process && bm->dayt
				>= bm->checkstart && bm->dayt < bm->checkstop)) {
			/* List all figures */
			fprintf(llogfp, "debug Ice_Box\n");
			/* List all figures */
			Print_Flux(bm, ICE_BASED, VERTinfo, noqnanverbose, 0, 0, 0, 0, 0, iceFlux, 0, iceFlux, 0, boxLayerInfo->localWCFlux, boxLayerInfo->localSEDFlux,
					boxLayerInfo->localEPIFlux, llogfp);

			fprintf(llogfp, "Prey eaten values at end of ice column box \n");
			Print_Eat_Diagnostics(bm, llogfp, 0, ICE_BASED, 1);
			// Print_Eat_Diagnostics(bm, llogfp, 0, EPIFAUNA, 1);

		}
	}
	if (!(_finite(boxLayerInfo->localSEDFlux[NH3_i]))) {
		/* Check diagnostic terms */
		Call_Diagnostics(bm, boxLayerInfo, SED, llogfp, 1);

		/* List all figures */
		Print_Flux(bm, ICE_BASED, VERTinfo, qnancheck, 0.0, 0.0, 0.0, 0.0, 0.0, iceFlux, 0.0, iceFlux, 0.0, boxLayerInfo->localWCFlux, boxLayerInfo->localSEDFlux,
				boxLayerInfo->localEPIFlux, llogfp);

		Textfile_Dump(bm, llogfp);
        quit("Ice_box: Got an QNAN result box: %d, layer: %d on day: %e, iceflux: %e\n", bm->current_box, bm->current_layer, bm->dayt, iceFlux);
	}

}

/**
 * \brief Update the vertebrate tracer values.
 */
static void Update_Vertebrate_Tracers(MSEBoxModel *bm, FILE *llogfp, HABITAT_TYPES habitatType, BoxLayerValues *boxLayerInfo) {

	int guild, cohort;//, habitat;

	//double eat;
	/**
	 Vertebrates - if in lowest watercolumn layer than this will all actually happen
	 in Epibenthic_Box()
	 **/

	if (habitatType == EPIFAUNA || (habitatType == WC && (bm->current_layer != 0 || waterboundary))) {

		for (guild = 0; guild < bm->K_num_tot_sp; guild++) {
			if(FunctGroupArray[guild].speciesParams[flag_id]){
				if (FunctGroupArray[guild].groupAgeType == AGE_STRUCTURED) {
					for (cohort = 0; cohort < FunctGroupArray[guild].numCohortsXnumGenes; cohort++) {

						boxLayerInfo->DebugFluxInfo[guild][WC][gain_id] = FunctGroupArray[guild].grow[cohort][RN_id]
								+ FunctGroupArray[guild].grow[cohort][SN_id];
						boxLayerInfo->DebugFluxInfo[guild][WC][loss_id] = FunctGroupArray[guild].dead[cohort]
								+ (double)FunctGroupArray[guild].preyEaten[cohort][WC];

#ifdef BROKER_LINK_ENABLED
						if(do_BrokerLinkage){
							boxLayerInfo->DebugFluxInfo[guild][WC][loss_id] += bm->linkageInterface->linkageWCMortality[bm->current_box][bm->current_layer][guild][cohort];
						}
#endif

						if (!(_finite(FunctGroupArray[guild].grow[cohort][RN_id])) ||
								!(_finite(FunctGroupArray[guild].grow[cohort][SN_id])) || !(_finite(FunctGroupArray[guild].dead[cohort]))
								|| !(_finite((double)FunctGroupArray[guild].preyEaten[cohort][WC]))) {
							quit("day: %e box-%d-%d %s-%d has growSN: %e, growRN: %e dead: %e, eaten: %e\n", bm->dayt, bm->current_box, bm->current_layer,
									FunctGroupArray[guild].groupCode, cohort, FunctGroupArray[guild].grow[cohort][SN_id],
									FunctGroupArray[guild].grow[cohort][RN_id], FunctGroupArray[guild].dead[cohort],
									FunctGroupArray[guild].preyEaten[cohort][WC]);
						}

						/* Set the TracerFlux values */
						boxLayerInfo->localWCFlux[FunctGroupArray[guild].structNTracers[cohort]] = FunctGroupArray[guild].grow[cohort][SN_id];
						boxLayerInfo->localWCFlux[FunctGroupArray[guild].resNTracers[cohort]] = FunctGroupArray[guild].grow[cohort][RN_id];

						if (habitatType == WC) {
							boxLayerInfo->localWCFlux[FunctGroupArray[guild].NumsTracers[cohort]] = -(FunctGroupArray[guild].dead[cohort]
									+ (double)FunctGroupArray[guild].preyEaten[cohort][WC]);

                            
                            /**
                            if((guild == bm->which_check) && (bm->current_box == bm->checkbox)){
                                fflush(llogfp);
                                fprintf(llogfp, "%s-%d numbers flux: %e, dead: %e, preyEaten: %Le\n", FunctGroupArray[guild].groupCode, cohort,
                                    boxLayerInfo->localWCFlux[FunctGroupArray[guild].NumsTracers[cohort]],
                                    FunctGroupArray[guild].dead[cohort], FunctGroupArray[guild].preyEaten[cohort][WC]);
                                fflush(llogfp);
                            }
                            **/

#ifdef BROKER_LINK_ENABLED
							if(do_BrokerLinkage){
								boxLayerInfo->localWCFlux[FunctGroupArray[guild].NumsTracers[cohort]]
								                          += bm->linkageInterface->linkageWCMortality[bm->current_box][bm->current_layer][guild][cohort];
							}
#endif

						} else {

							boxLayerInfo->localWCFlux[FunctGroupArray[guild].NumsTracers[cohort]] += -(FunctGroupArray[guild].dead[cohort]
									+ (double)FunctGroupArray[guild].preyEaten[cohort][WC]);

#ifdef BROKER_LINK_ENABLED
							if(do_BrokerLinkage){
								boxLayerInfo->localWCFlux[FunctGroupArray[guild].NumsTracers[cohort]] +=
										bm->linkageInterface->linkageWCMortality[bm->current_box][bm->current_layer][guild][cohort];
							}
#endif
						}


						if(!_finite(boxLayerInfo->localWCFlux[FunctGroupArray[guild].NumsTracers[cohort]])){
                            printf("Group %s:%d is an nan value for its numbers. Number dead are %e and tot num remaining: %e\n", FunctGroupArray[guild].groupCode, cohort, FunctGroupArray[guild].dead[cohort], boxLayerInfo->localWCFlux[FunctGroupArray[guild].NumsTracers[cohort]]);
							printf("Eaten = %Le\n", FunctGroupArray[guild].preyEaten[cohort][WC]);
							quit("ERROR: Num dead is infinite\n");
						}

						/**
						 //if(bm->newmonth && (bm->which_check == guild)){
						if((bm->which_check == guild) && (bm->dayt > bm->checkstart)){
							fprintf(llogfp, "Time: %e box%d-%d %s-%d totnumdead: %Le dead: %.20e eaten: %Le SNgrow: %.20e RNgrow: %.20e\n",
									bm->dayt, bm->current_box, bm->current_layer, FunctGroupArray[guild].groupCode, cohort,
									(FunctGroupArray[guild].dead[cohort] + FunctGroupArray[guild].preyEaten[cohort][WC]),
									FunctGroupArray[guild].dead[cohort], FunctGroupArray[guild].preyEaten[cohort][WC],
									FunctGroupArray[guild].grow[cohort][SN_id], FunctGroupArray[guild].grow[cohort][RN_id]);
						 }
						**/
					}
				}
			}
		}

		if (bm->debug && ((bm->debug > debug_biom) && (bm->which_check < bm->K_num_tot_sp) && (bm->current_box == bm->checkbox || bm->checkbox > bm->nbox)
				&& (bm->dayt > bm->checkstart))) {
			/* Check that we are interested in a vertebrate group */
			if (FunctGroupArray[bm->which_check].groupAgeType == AGE_STRUCTURED) {
				fprintf(llogfp, "day: %e, box: %d, layer %d, ", bm->dayt, bm->current_box, bm->current_layer);
				for (cohort = 0; cohort < FunctGroupArray[bm->which_check].numCohorts * FunctGroupArray[bm->which_check].numGeneTypes; cohort++) {
					fprintf(llogfp, "%s %s grow-SN%d %e growRN%d %e dead-%d %e eaten-%d %Le\n", ((habitatType == WC) ? "wc" : "epi"),
							FunctGroupArray[bm->which_check].groupCode, cohort, FunctGroupArray[bm->which_check].grow[cohort][SN_id], cohort,
							FunctGroupArray[bm->which_check].grow[cohort][RN_id], cohort,
							FunctGroupArray[bm->which_check].dead[cohort], cohort, FunctGroupArray[bm->which_check].preyEaten[cohort][WC]);
				}
			}
		}
	} else {

		/* If in bottom of water column update eaten only as rest will occur in Epibenthic_Box() when this function is called.*/
		for (guild = 0; guild < bm->K_num_tot_sp; guild++) {
			if(FunctGroupArray[guild].speciesParams[flag_id]){
				if (FunctGroupArray[guild].groupAgeType == AGE_STRUCTURED) {
					for (cohort = 0; cohort < FunctGroupArray[guild].numCohortsXnumGenes; cohort++) {
						boxLayerInfo->DebugFluxInfo[guild][habitatType][loss_id] = FunctGroupArray[guild].dead[cohort]
								+ (double)FunctGroupArray[guild].preyEaten[cohort][WC];

#ifdef BROKER_LINK_ENABLED
						if(do_BrokerLinkage){
							boxLayerInfo->DebugFluxInfo[guild][habitatType][loss_id]
								+= bm->linkageInterface->linkageSEDMortality[bm->current_box][bm->current_layer][guild][cohort];
						}
#endif
						/* Set the TracerFlux values */
						boxLayerInfo->localWCFlux[FunctGroupArray[guild].NumsTracers[cohort]] = -(FunctGroupArray[guild].dead[cohort]
								+ (double)FunctGroupArray[guild].preyEaten[cohort][WC]);

#ifdef BROKER_LINK_ENABLED
						if(do_BrokerLinkage){
							boxLayerInfo->localWCFlux[FunctGroupArray[guild].NumsTracers[cohort]] +=
									bm->linkageInterface->linkageSEDMortality[bm->current_box][bm->current_layer][guild][cohort];
						}
#endif

						if(!_finite(boxLayerInfo->localWCFlux[FunctGroupArray[guild].NumsTracers[cohort]])){
                            printf("Group %s:%d is an nan value for its numbers. Number dead are %e with num in box at: %e\n", FunctGroupArray[guild].groupCode, cohort, FunctGroupArray[guild].dead[cohort], boxLayerInfo->localWCFlux[FunctGroupArray[guild].NumsTracers[cohort]]);
							printf("Eaten = %Le\n", FunctGroupArray[guild].preyEaten[cohort][WC]);
							quit("ERROR Vert Tracers: Number dead is infinite.\n");
						}

						/**
                        if((guild == bm->which_check) && (bm->current_box == bm->checkbox)) {
						//if((bm->which_check == guild) && (bm->dayt > bm->checkstart)) {
                        //if ((guild == 33) || ((guild > 8) && (guild < 12))) {
                            fflush(llogfp);
							fprintf(llogfp, "Time: %e box%d-%d %s-%d totnumdead: %Le dead: %.20e eaten: %Le SNgrow: %.20e RNgrow: %.20e\n",
									bm->dayt, bm->current_box, bm->current_layer, FunctGroupArray[guild].groupCode, cohort,
									(FunctGroupArray[guild].dead[cohort] + FunctGroupArray[guild].preyEaten[cohort][WC]),
									FunctGroupArray[guild].dead[cohort], FunctGroupArray[guild].preyEaten[cohort][WC],
									FunctGroupArray[guild].grow[cohort][SN_id], FunctGroupArray[guild].grow[cohort][RN_id]);
                            fflush(llogfp);
						 }
                         
                         //                    fprintf(bm->logFile, "%s:%d - SN: %e, RN: %e, NUMS: %e\n",
                         //                        FunctGroupArray[guild].groupCode, cohort,
                         //                        boxLayerInfo->localWCFlux[FunctGroupArray[guild].structNTracers[cohort]],
                         //                        boxLayerInfo->localWCFlux[FunctGroupArray[guild].resNTracers[cohort]],
                         //                        boxLayerInfo->localWCFlux[FunctGroupArray[guild].NumsTracers[cohort]]);

						**/
					}
				}
			}
		}

        /*
		if (bm->debug && ((bm->debug > debug_biom) && (bm->which_check < bm->K_num_tot_sp) && FunctGroupArray[bm->which_check].isVertebrate == TRUE
				&& (bm->current_box == bm->checkbox || bm->checkbox > bm->nbox) && (bm->dayt > bm->checkstart))) {
			fprintf(llogfp, "day: %e, box: %d, layer %d, ", bm->dayt, bm->current_box, bm->current_layer);
			for (cohort = 0; cohort < FunctGroupArray[bm->which_check].numCohorts * FunctGroupArray[bm->which_check].numGeneTypes; cohort++) {
				fprintf(llogfp, "%s dead-%d %e eaten-%d %Le\n", FunctGroupArray[bm->which_check].groupCode, cohort,
						FunctGroupArray[bm->which_check].dead[cohort], cohort, FunctGroupArray[bm->which_check].preyEaten[cohort][WC]);
			}
		}
         */
	}
}

/*
 * \brief Reconcile global consumption, and detritus and nutrient flows for adaptive timestep
 */
static void Reconcile_Global_Values(MSEBoxModel *bm, BoxLayerValues *boxLayerInfo, HABITAT_TYPES habitatType, FILE *llogfp) {
	int guild, hab, kij, cohort;
	int tracerIndex;

	if (it_count != 1) {
		for (guild = 0; guild < bm->K_num_tot_sp; guild++) {
			if (FunctGroupArray[guild].speciesParams[flag_id] && FunctGroupArray[guild].isVertebrate == FALSE) {
				for (cohort = 0; cohort < FunctGroupArray[guild].numCohortsXnumGenes; cohort++) {
					for (hab = WC; hab < bm->num_active_habitats; hab++) {
						FunctGroupArray[guild].preyEaten[cohort][hab] += FunctGroupArray[guild].preyEatenGlobal[cohort][habitatType][hab];

                        if(bm->track_atomic_ratio == TRUE){
                            for(tracerIndex = 0; tracerIndex < num_atomic_id; tracerIndex++){
                                FunctGroupArray[guild].ratioLost[hab][cohort][tracerIndex] += FunctGroupArray[guild].ratioLostGlobal[habitatType][hab][cohort][tracerIndex];
                                FunctGroupArray[guild].ratioLostPred[hab][cohort][tracerIndex] += FunctGroupArray[guild].ratioLostPredGlobal[habitatType][hab][cohort][tracerIndex];

                                if(FunctGroupArray[guild].isDetritus == TRUE){
                                    //FunctGroupArray[guild].ratioGainedPred[hab][cohort][tracerIndex] += FunctGroupArray[guild].ratioGainedPredGlobal[habitatType][hab][cohort][tracerIndex];
                                    FunctGroupArray[guild].ratioGained[hab][cohort][tracerIndex] += FunctGroupArray[guild].ratioGainedGlobal[habitatType][hab][cohort][tracerIndex];
                                }

                                /**
                                //							if ((bm->debug == debug_atomic) && (bm->dayt >= bm->checkstart) && (bm->dayt < bm->checkstop) && ((bm->checkbox > bm->nbox) || (bm->current_box == bm->checkbox)) && (guild == bm->which_check)) {
                                    fprintf(bm->logFile, "FunctGroupArray[guild].ratioLost[%d][0][tracerIndex] = %.20Le, gained = %.20Le\n", hab, FunctGroupArray[guild].ratioGained[hab][cohort][tracerIndex], FunctGroupArray[guild].ratioGainedGlobal[habitatType][hab][cohort][tracerIndex]);
                                //							}
                                **/
                            }
						}
					}
				}
			}
		}
		for (guild = 0; guild < K_num_nutrients; guild++) {
			for (kij = 0; kij < EPIFAUNA; kij++) {
				boxLayerInfo->NutsProd[kij][guild] += boxLayerInfo->NutsProdGlobal[habitatType][kij][guild];
				boxLayerInfo->NutsLost[kij][guild] += boxLayerInfo->NutsLostGlobal[habitatType][kij][guild];
				boxLayerInfo->DetritusProd[kij][guild] += boxLayerInfo->DetritusProdGlobal[habitatType][kij][guild];
				boxLayerInfo->DetritusLost[kij][guild] += boxLayerInfo->DetritusLostGlobal[habitatType][kij][guild];

			}
		}

		if(bm->track_contaminants){
			Reconcile_Global_Contaminant_Values(bm, habitatType);
		}
	}
}

/**
 * \brief Function to check that the current values in PreyINFo are finite.
 * If any values are not finite a warning message is printed to the given log file.
 *
 */
static void Check_Prey_Is_Finite(MSEBoxModel *bm, FILE *llogfp, HABITAT_TYPES habitat_type) {
	int hab, guild, cohort;

	for (hab = 0; hab < bm->num_active_habitats; hab++) {
		for (guild = 0; guild < bm->K_num_tot_sp; guild++) {
			for (cohort = 0; cohort < FunctGroupArray[guild].numCohortsXnumGenes; cohort++) {
				if (FunctGroupArray[guild].speciesParams[flag_id] && FunctGroupArray[guild].isVertebrate == FALSE){
					if (!(_finite(PREYinfo[guild][cohort][hab]))) {
						fprintf(llogfp, "%e box%d-%d %s is %e in PREYinfo\n", bm->dayt, bm->current_box, bm->current_layer, FunctGroupArray[guild].groupCode,
								PREYinfo[guild][cohort][hab]);
					}
				}
			}
		}
	}
	if (bm->debug == debug_prey_biology_process && bm->dayt >= bm->checkstart && bm->dayt < bm->checkstop) {
		Print_Eat_Diagnostics(bm, llogfp, 0, habitat_type, 0);
		Print_Eat_Diagnostics(bm, llogfp, 0, habitat_type, 1);
	}
}

/**
 * \brief Initialise the arrays that are used in the box processing functions.
 *
 *
 */
void Reset_Arrays(MSEBoxModel *bm, HABITAT_TYPES habitat_type, int it_count, BoxLayerValues *boxLayerInfo, FILE *llogfp) {

	int guild, kij, k, tracerIndex, hab, cohort;

	for (guild = 0; guild < bm->K_num_tot_sp; guild++) {
		if (FunctGroupArray[guild].speciesParams[flag_id]){
			if (habitat_type == WC || habitat_type == EPIFAUNA) {
				for (kij = 0; kij < FunctGroupArray[guild].numCohortsXnumGenes; kij++) {
                    FunctGroupArray[guild].dead[kij] = 0.0;
                    
					/* Initalise consumption, catch, detritus and nutrient flow arrays */
					FunctGroupArray[guild].preyEaten[kij][WC] = 0.0;

					if (habitat_type == WC) {
						FunctGroupArray[guild].GrazeLive[kij] = 0;
						if(FunctGroupArray[guild].isVertebrate == TRUE){
							for (k = 0; k < FunctGroupArray[guild].numStages; k++)
								FunctGroupArray[guild].grow[kij][k] = 0.0;
						}
						/* Initalise consumption, catch, detritus and nutrient flow arrays */
						for (k = 0; k < bm->K_num_fisheries; k++) {
							bm->FCcaughttemp[guild][k][kij] = 0.0;
						}
					}
				}
			}

			for (hab = 0; hab < bm->num_active_habitats; hab++) {
				for (cohort = 0; cohort < FunctGroupArray[guild].numCohortsXnumGenes; cohort++) {
					PREYinfo[guild][cohort][hab] = 0.0;
					FunctGroupArray[guild].preyEaten[cohort][hab] = 0.0;
					/* Set the global value to zero on the first iteration in this time step */
					if (it_count == 1)
						FunctGroupArray[guild].preyEatenGlobal[cohort][habitat_type][hab] = 0.0;
				}
			}
		}
	}

	if (bm->which_check < not_tracking_flux) {
		boxLayerInfo->DebugFluxInfo[bm->which_check][habitat_type][gain_id] = 0;
		boxLayerInfo->DebugFluxInfo[bm->which_check][habitat_type][loss_id] = 0;
	}

	if (it_count == 1) {
		RecycledNHglobal = 0;
	}

	for (guild = 0; guild < K_num_nutrients; guild++) {
		/* The epi value for this will not be used */
		for (kij = 0; kij < bm->num_active_habitats; kij++) {
			boxLayerInfo->NutsProd[kij][guild] = 0.0;
			boxLayerInfo->NutsLost[kij][guild] = 0.0;
			boxLayerInfo->DetritusProd[kij][guild] = 0.0;
			boxLayerInfo->DetritusLost[kij][guild] = 0.0;

			if (it_count == 1) {
				boxLayerInfo->NutsProdGlobal[habitat_type][kij][guild] = 0.0;
				boxLayerInfo->NutsLostGlobal[habitat_type][kij][guild] = 0.0;
				boxLayerInfo->DetritusProdGlobal[habitat_type][kij][guild] = 0.0;
				boxLayerInfo->DetritusLostGlobal[habitat_type][kij][guild] = 0.0;
			}
		}
	}

	if(bm->track_atomic_ratio == TRUE){
		for (guild = 0; guild < bm->K_num_tot_sp; guild++) {
			if (FunctGroupArray[guild].speciesParams[flag_id]){
				for (cohort = 0; cohort < FunctGroupArray[guild].numCohorts; cohort++) {

					for (hab = 0; hab < bm->num_active_habitats; hab++) {
						for(tracerIndex = 0; tracerIndex < num_atomic_id; tracerIndex++){
							if (it_count == 1){
								FunctGroupArray[guild].ratioLostGlobal[habitat_type][hab][cohort][tracerIndex] = 0;
								FunctGroupArray[guild].ratioLostPredGlobal[habitat_type][hab][cohort][tracerIndex] = 0;

								//FunctGroupArray[guild].ratioGainedPredGlobal[habitat_type][hab][cohort][tracerIndex] = 0;
								FunctGroupArray[guild].ratioGainedGlobal[habitat_type][hab][cohort][tracerIndex] = 0;

							}
							FunctGroupArray[guild].ratioLostPred[hab][cohort][tracerIndex] = 0;
							FunctGroupArray[guild].ratioLost[hab][cohort][tracerIndex] = 0;
							FunctGroupArray[guild].ratioGainedPred[cohort][tracerIndex] = 0;
							FunctGroupArray[guild].ratioGained[hab][cohort][tracerIndex] = 0;

						}
					}
				}
			}
		}
	}

	if(bm->track_contaminants){
		Init_Contaminant_Transfer_Values(bm);
	}

}

/**
 * If we are using external spatial mortality and we are scaling all mortality not just linear mortality
 * we need to scale the detritus groups if requested.
 *
 * If the group has not been included we will just be scaling by 1.0 so no change.
 *
 *
 */

void Scale_Detritus_Mortality(MSEBoxModel *bm, BoxLayerValues *boxLayerInfo){

	int kij;
	double mortality_scalar;

	/* If there is a scalar to apply then grab it now */
	mortality_scalar = Ecology_Get_Mortality_Scalar(bm, RefDetIndex, 0);

	for (kij = 0; kij < bm->num_active_habitats; kij++) {
		boxLayerInfo->DetritusLost[kij][DRdet_id] = mortality_scalar * boxLayerInfo->DetritusLost[kij][DRdet_id];
	}

	/* If there is a scalar to apply then grab it now */
	mortality_scalar = Ecology_Get_Mortality_Scalar(bm, LabDetIndex, 0);

	for (kij = 0; kij < bm->num_active_habitats; kij++) {
		boxLayerInfo->DetritusLost[kij][DLdet_id] = mortality_scalar * boxLayerInfo->DetritusLost[kij][DLdet_id];
	}

	/* If there is a scalar to apply then grab it now */
	mortality_scalar = Ecology_Get_Mortality_Scalar(bm, CarrionIndex, 0);

	for (kij = 0; kij < bm->num_active_habitats; kij++) {
		boxLayerInfo->DetritusLost[kij][DCdet_id] = mortality_scalar * boxLayerInfo->DetritusLost[kij][DCdet_id];
	}

}
