/**
 *	\brief The process functions for each of the functional group types.
 *	\ingroup atecology
 *
 *
 *	28-01-2008 Bec Gorton
 *	Changed the calls to Oxygen to always use the relevant species parameters as these are now always loaded.
 *	Conversion to XML code sets values to 0 for all relevant groups before overwriting with values found in the
 *	original input file. The results should be the same just more generic.
 *
 * 	02-02-2009 Beth Fulton
 *	Changed the calcMnumPerPred calculations in the FilterFeeder process function to add the
 *	amount grazed to the current calcMnumPerPred value.
 *	Also change the variable called small to small_num.
 *
 * 	03-03-2009 Bec Gorton
 * 	Added the process function for the DR, DL and DC groups.
 *
 *	10-06-2009 Bec Gorton
 *	Added the seagrass invertebrate group.
 *
 *	15-06-2009 Bec Gorton
 *	Added EPLANT values to the input files for invertebrates.
 *
 *	07-09-2009 Bec Gorton
 *	Added the additional linkageMortality values to the rate of change equations.
 *	Also changed some of the loss debug values to sum the lost biomass values together.
 *
 *	14-12-2009 Bec Gorton
 *	As the mindepth_id and maxdepth_id values are now used by all functional groups - these are no longer multipled by -1 when they are
 *	read in. This mean't the Epibenthic_Invert_Process code had to be changed to multiple the min and max depths by -1 when they are
 *	compared to the depth of the current layer.
 *
 *	29-01-2010 Bec Gorton
 *	Updated code to correctly calculate the mortality values.
 *
 *	26-02-2010 Bec Gorton
 *	Got rid of the atEcologyModule and moved all variables in this structure into the BoxLayerValues structure.
 *	Also moved the debug, detritus and nutrient arrays out of the box model and into the BoxLayerValues structure as well.
 *
 *	8-04-2010 Bec Gorton
 *	Changed the group process functions to use the new Ecology_Get_Species_LMort function
 * 	to get the linear mortality of a group - this function will calculate apply any scaling that should occur.
 *
 * 	10-05-2010 Bec Gorton
 * 	Got rid of the Ecology_Get_Species_LMort function. These calculation are now done in ParameterQ10.
 *
 *  14-05-2012 Beth Fulton
 * 	Added stuff for Savina seagrass model
 *
 *	05-07-2012 Bec Gorton
 *	Wrapped the broker linkage code in a define to simply the build process for those who don't need to link.
 *
 *  17-04-2013 Bec Gorton
 *  ATL-346 - Added support of tracking P:N:C ratios in groups.
 */

/*  Open library routines */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <sjwlib.h>
#include <atecology.h>
#include <string.h>

/**
 \file
 \brief The invertebrate process and rate of change functions.



 Zooplankton Grazing:

 The underlying grazing function is Holling Type 2, or hyperbolic, in form.
 This function is specified by two parameters: the maximum specific clearance
 rate, C, and the maximum ingestion rate. However, the maximum ingestion rate
 in the model is related to the grazer's maximum growth rate mum by a
 constant growth efficiency, E. The simplest case occurs for the small
 zooplankton, which feed on only one food type:

 Large zooplankton. These are crustacea (e.g. krill) and other large non-gelatinous plankton.

 Mesozooplankton. These feed on large phytoplankton, PL



 **/


/**
 * \brief Update the detritus flux values.
 */
static void Update_Detritus(MSEBoxModel *bm, BoxLayerValues *boxLayerInfo, HABITAT_TYPES habitatType, int guild, int processGuild, int cohort, double initialBiomass) {

	int isGlobal = (FunctGroupArray[guild].diagTol == 2 && it_count == 1);

	switch (habitatType) {

	case WC:
		FunctGroupArray[pelagicBactIndex].preyEaten[0][WC] += GRAZEinfo[pelagicBactIndex][0][WC];
		if(bm->track_atomic_ratio == TRUE){
			Loose_Element_From_Prey(bm, boxLayerInfo, habitatType, pelagicBactIndex, 0, GRAZEinfo[pelagicBactIndex][0][WC], WC, isGlobal);
			Loose_Element_From_Prey(bm, boxLayerInfo, habitatType, RefDetIndex, 0, GRAZEinfo[RefDetIndex][0][WC], WC, isGlobal);
			Loose_Element_From_Prey(bm, boxLayerInfo, habitatType, LabDetIndex, 0, GRAZEinfo[LabDetIndex][0][WC], WC, isGlobal);
		}

		bm->calcMnumPerPred[pelagicBactIndex][guild][current_id] += (GRAZEinfo[pelagicBactIndex][0][WC] * bm->boxes[bm->current_box].dz[bm->current_layer] * bm->boxes[bm->current_box].area * FunctGroupArray[guild].habitatCoeffs[WC]);
        bm->calcTrackedMort[pelagicBactIndex][0][0][ongoing_id] += (GRAZEinfo[pelagicBactIndex][0][WC] * bm->boxes[bm->current_box].dz[bm->current_layer] * bm->boxes[bm->current_box].area * FunctGroupArray[guild].habitatCoeffs[WC] * bm->dt);
		bm->calcTrackedMort[pelagicBactIndex][0][0][ongoingM2_id] += (GRAZEinfo[pelagicBactIndex][0][WC] * bm->boxes[bm->current_box].dz[bm->current_layer] * bm->boxes[bm->current_box].area * FunctGroupArray[guild].habitatCoeffs[WC] * FunctGroupArray[pelagicBactIndex].speciesParams[Mdt_id]);
		bm->calcTrackedPredMort[pelagicBactIndex][0][0][guild][ongoing_id] += (GRAZEinfo[pelagicBactIndex][0][WC] * bm->boxes[bm->current_box].dz[bm->current_layer] * bm->boxes[bm->current_box].area * FunctGroupArray[guild].habitatCoeffs[WC] * FunctGroupArray[pelagicBactIndex].speciesParams[Mdt_id]);

		/* These will have been added to the ratios with the other groups */
		boxLayerInfo->DetritusLost[WC][DRdet_id] += GRAZEinfo[RefDetIndex][0][WC];
		boxLayerInfo->DetritusLost[WC][DLdet_id] += GRAZEinfo[LabDetIndex][0][WC];

		boxLayerInfo->DetritusProd[WC][DRdet_id] += FunctGroupArray[guild].prodnDR[cohort];
		boxLayerInfo->DetritusProd[WC][DLdet_id] += FunctGroupArray[guild].prodnDL[cohort] + FunctGroupArray[guild].lysis[cohort];
            
        /*
        if(bm->dayt > bm->checkstart)
            fprintf(bm->logFile, "Time: %e box%d-%d %s wcDLprod: %Lf DLlost: %Lf graze %e prodnDet: %e lysis: %e\n", bm->dayt, bm->current_box, bm->current_layer, FunctGroupArray[guild].groupCode, boxLayerInfo->DetritusProd[WC][DLdet_id], boxLayerInfo->DetritusLost[WC][DLdet_id], GRAZEinfo[LabDetIndex][0][WC], FunctGroupArray[guild].prodnDL[cohort], FunctGroupArray[guild].lysis[cohort]);
         */


		if(bm->track_atomic_ratio == TRUE){
			Gain_Element(bm, boxLayerInfo, WC, RefDetIndex, 0, guild, cohort, FunctGroupArray[guild].prodnDR[cohort], habitatType, isGlobal);
			Gain_Element(bm, boxLayerInfo, WC, LabDetIndex, 0, guild, cohort, (FunctGroupArray[guild].prodnDL[cohort] + FunctGroupArray[guild].lysis[cohort]), habitatType, isGlobal);
		}

		boxLayerInfo->NutsProd[WC][NH_id] += FunctGroupArray[guild].releaseNH[cohort];

		if (FunctGroupArray[guild].diagTol == 2 && it_count == 1) {

			FunctGroupArray[pelagicBactIndex].preyEatenGlobal[0][WC][WC] += GRAZEinfo[pelagicBactIndex][0][WC];
			boxLayerInfo->DetritusLostGlobal[WC][WC][DRdet_id] += GRAZEinfo[RefDetIndex][0][WC];
			boxLayerInfo->DetritusLostGlobal[WC][WC][DLdet_id] += GRAZEinfo[LabDetIndex][0][WC];
			boxLayerInfo->DetritusProdGlobal[WC][WC][DLdet_id] += FunctGroupArray[guild].prodnDL[cohort];
			boxLayerInfo->DetritusProdGlobal[WC][WC][DRdet_id] += FunctGroupArray[guild].prodnDR[cohort];
			boxLayerInfo->NutsProdGlobal[WC][WC][NH_id] += FunctGroupArray[guild].releaseNH[cohort];
            
            /*
            if(bm->dayt > bm->checkstart)
                fprintf(bm->logFile, "Time: %e box%d-%d %s wcDLprodGLOBAL: %Lf DLlostGLOBAL: %Lf grazeGLOBAL %e prodnGLOBAL: %e\n",
                    bm->dayt, bm->current_box, bm->current_layer, FunctGroupArray[guild].groupCode, boxLayerInfo->DetritusProdGlobal[WC][WC][DLdet_id],
                    boxLayerInfo->DetritusLostGlobal[WC][WC][DLdet_id],
                    GRAZEinfo[LabDetIndex][0][WC], FunctGroupArray[guild].prodnDL[cohort]);
             */

		}

		/* Contaminants transfer */
		if(bm->track_contaminants){
            
            /* Gains in DR and DL due to mortality */
            //fprintf(bm->logFile,"Calling Group_Transfer_Contaminant from Update_Detritus WC WC - %s-%d with initialBiomass: %e prodnDR: %e prodnDL: %e\n", FunctGroupArray[guild].groupCode, cohort, initialBiomass, FunctGroupArray[guild].prodnDR[cohort], FunctGroupArray[guild].prodnDL[cohort]);
			Group_Transfer_Contaminant(bm, boxLayerInfo, WC, WC, RefDetIndex, 0, guild, cohort, FunctGroupArray[guild].prodnDR[cohort], 0, initialBiomass, bm->dtsz_stored, 0, 0);
			Group_Transfer_Contaminant(bm, boxLayerInfo, WC, WC, LabDetIndex, 0, guild, cohort, FunctGroupArray[guild].prodnDL[cohort] + FunctGroupArray[guild].lysis[cohort], 0, initialBiomass, bm->dtsz_stored, 0, 1);

			/* Release contaminants back into the water column */
            //fprintf(bm->logFile,"Calling Gain_Contaminants from Update_Detritus WC WC - %s-%d with initialBiomass: %e releaseNH: %e\n", FunctGroupArray[guild].groupCode, cohort, initialBiomass, FunctGroupArray[guild].releaseNH[cohort]);
            if(bm->flag_detritus_contam) {
                Gain_Contaminants(bm, boxLayerInfo, WC, WC, guild, cohort, FunctGroupArray[guild].releaseNH[cohort], 0, initialBiomass);
            }


			/* Now handle transfer due to predation - group gains contaminants due to eating detritus and bacteria */
            //fprintf(bm->logFile,"Calling Group_Transfer_Contaminant from Update_Detritus WC WC - %s-%d with grazebact: %e grazeDR: %e, grazeDL: %e EATINGinfobact: %e EATINGinfoDR: %e, EATINGinfoDL: %e\n", FunctGroupArray[guild].groupCode, cohort, GRAZEinfo[pelagicBactIndex][0][WC], GRAZEinfo[RefDetIndex][0][WC], GRAZEinfo[LabDetIndex][0][WC], EATINGinfo[pelagicBactIndex][0][WC], EATINGinfo[RefDetIndex][0][WC], EATINGinfo[LabDetIndex][0][WC]);
            
            Group_Transfer_Contaminant(bm, boxLayerInfo, WC, WC, guild, cohort, pelagicBactIndex, 0, GRAZEinfo[pelagicBactIndex][0][WC], 0, EATINGinfo[pelagicBactIndex][0][WC], bm->dtsz_stored, 1, 2);
            if(bm->flag_detritus_contam) { // If allowing transfer of contaminants due to eating detritus
                Group_Transfer_Contaminant(bm, boxLayerInfo, WC, WC, guild, cohort, RefDetIndex, 0, GRAZEinfo[RefDetIndex][0][WC], 0, EATINGinfo[RefDetIndex][0][WC], bm->dtsz_stored, 1, 3);
                Group_Transfer_Contaminant(bm, boxLayerInfo, WC, WC, guild, cohort, LabDetIndex, 0, GRAZEinfo[LabDetIndex][0][WC], 0, EATINGinfo[LabDetIndex][0][WC], bm->dtsz_stored, 1, 4);
            }
		}

		break;
	case SED:
		FunctGroupArray[SedBactIndex].preyEaten[0][SED] += GRAZEinfo[SedBactIndex][0][SED];

		if(bm->track_atomic_ratio == TRUE){
			Loose_Element_From_Prey(bm, boxLayerInfo, habitatType, SedBactIndex, 0, GRAZEinfo[SedBactIndex][0][SED], SED, isGlobal);
			Loose_Element_From_Prey(bm, boxLayerInfo, habitatType, RefDetIndex, 0, GRAZEinfo[RefDetIndex][0][SED], SED, isGlobal);
			Loose_Element_From_Prey(bm, boxLayerInfo, habitatType, LabDetIndex, 0, GRAZEinfo[LabDetIndex][0][SED], SED, isGlobal);
		}

		bm->calcMnumPerPred[SedBactIndex][guild][current_id] += (GRAZEinfo[SedBactIndex][0][SED] * bm->boxes[bm->current_box].sm.dz[bm->current_layer] * bm->boxes[bm->current_box].area * FunctGroupArray[guild].habitatCoeffs[SED]);
		bm->calcTrackedMort[SedBactIndex][0][0][ongoingM2_id] += (GRAZEinfo[SedBactIndex][0][SED] * bm->boxes[bm->current_box].sm.dz[bm->current_layer] * bm->boxes[bm->current_box].area * FunctGroupArray[guild].habitatCoeffs[SED] * FunctGroupArray[SedBactIndex].speciesParams[Mdt_id]);
		bm->calcTrackedPredMort[SedBactIndex][0][0][guild][ongoing_id] += (GRAZEinfo[SedBactIndex][0][SED] * bm->boxes[bm->current_box].sm.dz[bm->current_layer] * bm->boxes[bm->current_box].area * FunctGroupArray[guild].habitatCoeffs[SED] * FunctGroupArray[SedBactIndex].speciesParams[Mdt_id]);
 
		boxLayerInfo->DetritusLost[SED][DRdet_id] += GRAZEinfo[RefDetIndex][0][SED];
		boxLayerInfo->DetritusLost[SED][DLdet_id] += GRAZEinfo[LabDetIndex][0][SED];
		boxLayerInfo->DetritusProd[SED][DLdet_id] += FunctGroupArray[guild].prodnDL[cohort];// + FunctGroupArray[guild].lysis[cohort];
		boxLayerInfo->DetritusProd[SED][DRdet_id] += FunctGroupArray[guild].prodnDR[cohort];
		boxLayerInfo->NutsProd[SED][NH_id] += FunctGroupArray[guild].releaseNH[cohort];
            
        /*
        if(bm->dayt > bm->checkstart)
            fprintf(bm->logFile, "Time: %e box%d-%d %s sedDLprod: %Lf DLlost: %Lf graze %e prodnDet: %e\n",
                bm->dayt, bm->current_box, bm->current_layer, FunctGroupArray[guild].groupCode, boxLayerInfo->DetritusProd[SED][DLdet_id], boxLayerInfo->DetritusLost[SED][DLdet_id],
                GRAZEinfo[LabDetIndex][0][SED], FunctGroupArray[guild].prodnDL[cohort]);
         */


		if(bm->track_atomic_ratio == TRUE){
			Gain_Element(bm, boxLayerInfo, habitatType, RefDetIndex, 0, guild, cohort, FunctGroupArray[guild].prodnDR[cohort], SED, isGlobal);
			Gain_Element(bm, boxLayerInfo, habitatType, LabDetIndex, 0, guild, cohort, FunctGroupArray[guild].prodnDL[cohort], SED, isGlobal);

		}

		if (FunctGroupArray[guild].diagTol == 2 && it_count == 1) {
			/* Update global fluxes */
			FunctGroupArray[SedBactIndex].preyEatenGlobal[0][habitatType][SED] += GRAZEinfo[SedBactIndex][0][SED];
			boxLayerInfo->DetritusLostGlobal[SED][SED][DRdet_id] += GRAZEinfo[RefDetIndex][0][SED];
			boxLayerInfo->DetritusLostGlobal[SED][SED][DLdet_id] += GRAZEinfo[LabDetIndex][0][SED];
			boxLayerInfo->DetritusProdGlobal[SED][SED][DLdet_id] += FunctGroupArray[guild].prodnDL[cohort];
			boxLayerInfo->DetritusProdGlobal[SED][SED][DRdet_id] += FunctGroupArray[guild].prodnDR[cohort];
			boxLayerInfo->NutsProdGlobal[SED][SED][NH_id] += FunctGroupArray[guild].releaseNH[cohort];
            
            /*
            if(bm->dayt > bm->checkstart)
                fprintf(bm->logFile, "Time: %e box%d-%d %s sedDLprodGLOBAL: %Lf DLlostGLOBAL: %Lf grazeGLOBAL %e prodnGLOBAL: %e\n",
                    bm->dayt, bm->current_box, bm->current_layer, FunctGroupArray[guild].groupCode, boxLayerInfo->DetritusProdGlobal[SED][SED][DLdet_id],
                    boxLayerInfo->DetritusLostGlobal[SED][SED][DLdet_id],
                    GRAZEinfo[LabDetIndex][0][SED], FunctGroupArray[guild].prodnDL[cohort]);
             */
            
		}

		if(bm->track_contaminants){

            /* Gains in DR and DL due to mortality */
            //fprintf(bm->logFile,"Calling Group_Transfer_Contaminant from Update_Detritus SED SED - %s-%d with initialBiomass: %e prodnDR: %e prodnDL: %e\n", FunctGroupArray[guild].groupCode, cohort, initialBiomass, FunctGroupArray[guild].prodnDR[cohort], FunctGroupArray[guild].prodnDL[cohort]);
			Group_Transfer_Contaminant(bm, boxLayerInfo, SED, SED, RefDetIndex, 0, guild, cohort, FunctGroupArray[guild].prodnDR[cohort], 0, initialBiomass, bm->dtsz_stored, 0, 5);
			Group_Transfer_Contaminant(bm, boxLayerInfo, SED, SED, LabDetIndex, 0, guild, cohort, FunctGroupArray[guild].prodnDL[cohort], 0, initialBiomass, bm->dtsz_stored, 0, 6);

			/* Release contaminants back into the water column */
            //fprintf(bm->logFile,"Calling Gain_Contaminants from Update_Detritus SED SED\n");
            if(bm->flag_detritus_contam) {
                Gain_Contaminants(bm, boxLayerInfo, SED, SED, guild, cohort,FunctGroupArray[guild].releaseNH[cohort], 0, initialBiomass);
            }

			/* Contaminant tracer due to predation */
            //fprintf(bm->logFile,"Calling Group_Transfer_Contaminant from Update_Detritus SED SED - %s-%d with %e grazebact: %e grazeDR: %e, grazeDL: %e EATINGinfo bact: %e EATINGinfoDR: %e EATINGinfoDL: %e\n", FunctGroupArray[guild].groupCode, cohort, initialBiomass, GRAZEinfo[SedBactIndex][0][SED], GRAZEinfo[RefDetIndex][0][SED], GRAZEinfo[LabDetIndex][0][SED], EATINGinfo[SedBactIndex][0][SED],EATINGinfo[RefDetIndex][0][SED], EATINGinfo[LabDetIndex][0][SED]);

            Group_Transfer_Contaminant(bm, boxLayerInfo, SED, SED, guild, cohort, SedBactIndex, 0, GRAZEinfo[SedBactIndex][0][SED], 0, EATINGinfo[SedBactIndex][0][SED], bm->dtsz_stored, 1, 7);
            
            if(bm->flag_detritus_contam) {  // If allowing transfer of contaminants due to eating detritus
                Group_Transfer_Contaminant(bm, boxLayerInfo, SED, SED, guild, cohort, RefDetIndex, 0, GRAZEinfo[RefDetIndex][0][SED], 0, EATINGinfo[RefDetIndex][0][SED], bm->dtsz_stored, 1, 8);
                Group_Transfer_Contaminant(bm, boxLayerInfo, SED, SED, guild, cohort, LabDetIndex, 0, GRAZEinfo[LabDetIndex][0][SED], 0, EATINGinfo[LabDetIndex][0][SED], bm->dtsz_stored, 1, 9);
            }
		}

		break;
	case EPIFAUNA:

		FunctGroupArray[pelagicBactIndex].preyEaten[0][WC] += GRAZEinfo[pelagicBactIndex][0][WC];
		FunctGroupArray[SedBactIndex].preyEaten[0][SED] += GRAZEinfo[SedBactIndex][0][SED];

		bm->calcMnumPerPred[pelagicBactIndex][guild][current_id] += (GRAZEinfo[pelagicBactIndex][0][WC] * bm->boxes[bm->current_box].dz[bm->current_layer] * bm->boxes[bm->current_box].area * FunctGroupArray[guild].habitatCoeffs[WC]);
		bm->calcMnumPerPred[SedBactIndex][guild][current_id] += (GRAZEinfo[SedBactIndex][0][SED] * bm->boxes[bm->current_box].sm.dz[bm->current_layer] * bm->boxes[bm->current_box].area * FunctGroupArray[guild].habitatCoeffs[SED]);
		bm->calcTrackedMort[pelagicBactIndex][0][0][ongoingM2_id] += (GRAZEinfo[pelagicBactIndex][0][WC] * bm->boxes[bm->current_box].dz[bm->current_layer] * bm->boxes[bm->current_box].area * FunctGroupArray[guild].habitatCoeffs[WC] * FunctGroupArray[pelagicBactIndex].speciesParams[Mdt_id]);
		bm->calcTrackedMort[SedBactIndex][0][0][ongoingM2_id] += (GRAZEinfo[SedBactIndex][0][SED] * bm->boxes[bm->current_box].sm.dz[bm->current_layer] * bm->boxes[bm->current_box].area * FunctGroupArray[guild].habitatCoeffs[SED] * FunctGroupArray[SedBactIndex].speciesParams[Mdt_id]);
		bm->calcTrackedPredMort[pelagicBactIndex][0][0][guild][ongoing_id] += (GRAZEinfo[pelagicBactIndex][0][WC] * bm->boxes[bm->current_box].dz[bm->current_layer] * bm->boxes[bm->current_box].area * FunctGroupArray[guild].habitatCoeffs[WC] * FunctGroupArray[pelagicBactIndex].speciesParams[Mdt_id]);
		bm->calcTrackedPredMort[SedBactIndex][0][0][guild][ongoing_id] += (GRAZEinfo[SedBactIndex][0][SED] * bm->boxes[bm->current_box].sm.dz[bm->current_layer] * bm->boxes[bm->current_box].area * FunctGroupArray[guild].habitatCoeffs[SED] * FunctGroupArray[SedBactIndex].speciesParams[Mdt_id]);

		if(bm->track_atomic_ratio == TRUE){
			Loose_Element_From_Prey(bm, boxLayerInfo, WC, pelagicBactIndex, 0, GRAZEinfo[pelagicBactIndex][0][WC], EPIFAUNA, isGlobal);
			Loose_Element_From_Prey(bm, boxLayerInfo, SED, SedBactIndex, 0, GRAZEinfo[SedBactIndex][0][SED], EPIFAUNA, isGlobal);

			Loose_Element_From_Prey(bm, boxLayerInfo, WC, RefDetIndex, 0, GRAZEinfo[RefDetIndex][0][WC], EPIFAUNA, isGlobal);
			Loose_Element_From_Prey(bm, boxLayerInfo, SED, RefDetIndex, 0, GRAZEinfo[RefDetIndex][0][SED], EPIFAUNA, isGlobal);


			Loose_Element_From_Prey(bm, boxLayerInfo, WC, LabDetIndex, 0, GRAZEinfo[LabDetIndex][0][WC], EPIFAUNA, isGlobal);
			Loose_Element_From_Prey(bm, boxLayerInfo, SED, LabDetIndex, 0, GRAZEinfo[LabDetIndex][0][SED], EPIFAUNA, isGlobal);

		}

		boxLayerInfo->DetritusLost[WC][DRdet_id] += GRAZEinfo[RefDetIndex][0][WC];
		boxLayerInfo->DetritusLost[WC][DLdet_id] += GRAZEinfo[LabDetIndex][0][WC];

		boxLayerInfo->DetritusLost[SED][DRdet_id] += GRAZEinfo[RefDetIndex][0][SED];
		boxLayerInfo->DetritusLost[SED][DLdet_id] += GRAZEinfo[LabDetIndex][0][SED];
            

		if (FunctGroupArray[processGuild].groupType == SED_EP_OTHER || FunctGroupArray[processGuild].groupType == SED_EP_FF
				|| FunctGroupArray[processGuild].groupType == MOB_EP_OTHER) {

			boxLayerInfo->DetritusProd[SED][DLdet_id] += FunctGroupArray[guild].prodnDL[cohort];
			boxLayerInfo->DetritusProd[SED][DRdet_id] += FunctGroupArray[guild].prodnDR[cohort];


			if(bm->track_atomic_ratio == TRUE){
				Gain_Element(bm, boxLayerInfo, SED, RefDetIndex, 0, guild, cohort, FunctGroupArray[guild].prodnDR[cohort], EPIFAUNA, isGlobal);
				Gain_Element(bm, boxLayerInfo, SED, LabDetIndex, 0, guild, cohort, FunctGroupArray[guild].prodnDL[cohort], EPIFAUNA, isGlobal);
			}

			if (FunctGroupArray[guild].diagTol == 2 && it_count == 1){
				boxLayerInfo->NutsProd[SED][NH_id] += FunctGroupArray[guild].releaseNH[cohort];
			} else {
				boxLayerInfo->NutsProdGlobal[EPIFAUNA][SED][NH_id] += FunctGroupArray[guild].releaseNH[cohort]; // Was boxLayerInfo->NutsProd[WC][NH_id]
			}

		} else {
			boxLayerInfo->DetritusProd[WC][DLdet_id] += FunctGroupArray[guild].prodnDL[cohort];
			boxLayerInfo->DetritusProd[WC][DRdet_id] += FunctGroupArray[guild].prodnDR[cohort];
			boxLayerInfo->NutsProd[WC][NH_id] += FunctGroupArray[guild].releaseNH[cohort];


			if(bm->track_atomic_ratio == TRUE){
				Gain_Element(bm, boxLayerInfo, WC, RefDetIndex, 0, guild, cohort, FunctGroupArray[guild].prodnDR[cohort], EPIFAUNA, isGlobal);
				Gain_Element(bm, boxLayerInfo, WC, LabDetIndex, 0, guild, cohort, FunctGroupArray[guild].prodnDL[cohort], EPIFAUNA, isGlobal);
			}
		}
            
        if((bm->dayt > bm->checkstart) && (bm->checkbox == bm->current_box)) {
            fprintf(bm->logFile, "Time: %e box%d-%d %s epiDLprod: %Lf (%Lf) DLlost: %Lf (%Lf) graze %e (%e) prodnDet: %e\n",
                bm->dayt, bm->current_box, bm->current_layer, FunctGroupArray[guild].groupCode, boxLayerInfo->DetritusProd[WC][DLdet_id], boxLayerInfo->DetritusProd[SED][DLdet_id],
                boxLayerInfo->DetritusLost[WC][DLdet_id], boxLayerInfo->DetritusLost[SED][DLdet_id],
                GRAZEinfo[LabDetIndex][0][WC], GRAZEinfo[LabDetIndex][0][SED], FunctGroupArray[guild].prodnDL[cohort]);
        }

		/* Update global fluxes */
		if (FunctGroupArray[guild].diagTol == 2 && it_count == 1) {
			FunctGroupArray[pelagicBactIndex].preyEatenGlobal[0][EPIFAUNA][WC] += GRAZEinfo[pelagicBactIndex][0][WC];
			FunctGroupArray[SedBactIndex].preyEatenGlobal[0][EPIFAUNA][SED] += GRAZEinfo[SedBactIndex][0][SED];
			boxLayerInfo->DetritusLostGlobal[EPIFAUNA][WC][DRdet_id] += GRAZEinfo[RefDetIndex][0][WC];
			boxLayerInfo->DetritusLostGlobal[EPIFAUNA][WC][DLdet_id] += GRAZEinfo[LabDetIndex][0][WC];
			boxLayerInfo->DetritusLostGlobal[EPIFAUNA][SED][DRdet_id] += GRAZEinfo[RefDetIndex][0][SED];
			boxLayerInfo->DetritusLostGlobal[EPIFAUNA][SED][DLdet_id] += GRAZEinfo[LabDetIndex][0][SED];


			if (FunctGroupArray[processGuild].groupType == SED_EP_OTHER || FunctGroupArray[processGuild].groupType == SED_EP_FF
					|| FunctGroupArray[processGuild].groupType == MOB_EP_OTHER) {
				boxLayerInfo->DetritusProdGlobal[EPIFAUNA][SED][DLdet_id] += FunctGroupArray[guild].prodnDL[cohort];
				boxLayerInfo->DetritusProdGlobal[EPIFAUNA][SED][DRdet_id] += FunctGroupArray[guild].prodnDR[cohort];
				boxLayerInfo->NutsProdGlobal[EPIFAUNA][SED][NH_id] += FunctGroupArray[guild].releaseNH[cohort];

			} else {
				boxLayerInfo->DetritusProdGlobal[EPIFAUNA][WC][DLdet_id] += FunctGroupArray[guild].prodnDL[cohort];
				boxLayerInfo->DetritusProdGlobal[EPIFAUNA][WC][DRdet_id] += FunctGroupArray[guild].prodnDR[cohort];
				boxLayerInfo->NutsProdGlobal[EPIFAUNA][WC][NH_id] += FunctGroupArray[guild].releaseNH[cohort];
			}
            
            if((bm->dayt > bm->checkstart) && (bm->checkbox == bm->current_box)) {
                fprintf(bm->logFile, "Time: %e box%d-%d %s epiDLprodGLOBAL: %Lf (%Lf) DLlostGLOBAL: %Lf (%Lf) grazeGLOBAL %e (%e) prodnGLOBAL: %e\n",
                    bm->dayt, bm->current_box, bm->current_layer, FunctGroupArray[guild].groupCode,
                    boxLayerInfo->DetritusProdGlobal[EPIFAUNA][WC][DLdet_id], boxLayerInfo->DetritusProdGlobal[EPIFAUNA][SED][DLdet_id],
                    boxLayerInfo->DetritusLostGlobal[EPIFAUNA][WC][DLdet_id], boxLayerInfo->DetritusLostGlobal[EPIFAUNA][SED][DLdet_id],
                    GRAZEinfo[LabDetIndex][0][WC], GRAZEinfo[LabDetIndex][0][SED], FunctGroupArray[guild].prodnDL[cohort]);
            }
            
		}

		if(bm->track_contaminants){

			if (FunctGroupArray[processGuild].groupType == SED_EP_OTHER || FunctGroupArray[processGuild].groupType == SED_EP_FF
							|| FunctGroupArray[processGuild].groupType == MOB_EP_OTHER) {

				/* Contaminants transferred to detritus */
                //fprintf(bm->logFile,"Calling Group_Transfer_Contaminant from Update_Detritus EPIFAUNA SED - %s-%d with initialBiomass: %e prodnDR: %e prodnDL: %e\n", FunctGroupArray[guild].groupCode, cohort, initialBiomass, FunctGroupArray[guild].prodnDR[cohort], FunctGroupArray[guild].prodnDL[cohort]);
                
                Group_Transfer_Contaminant(bm, boxLayerInfo, EPIFAUNA, SED, RefDetIndex, 0, guild, cohort, FunctGroupArray[guild].prodnDR[cohort], 0, initialBiomass, bm->dtsz_stored, 0, 10);
				Group_Transfer_Contaminant(bm, boxLayerInfo, EPIFAUNA, SED, LabDetIndex, 0, guild, cohort, FunctGroupArray[guild].prodnDL[cohort], 0, initialBiomass, bm->dtsz_stored, 0, 11);

				if (FunctGroupArray[guild].diagTol == 2 && it_count == 1){
					/* Release contaminants back into the sediment*/
                    //fprintf(bm->logFile,"Calling Gain_Contaminants from Update_Detritus EPIFUNA SED SED_EP_FF it_count 1 %s-%d releaseNH: %e bio: %e\n", FunctGroupArray[guild].groupCode, cohort, FunctGroupArray[guild].releaseNH[cohort], initialBiomass);
					Gain_Contaminants(bm, boxLayerInfo, EPIFAUNA, SED, guild, cohort, FunctGroupArray[guild].releaseNH[cohort], 0, initialBiomass);

                } else if(bm->flag_detritus_contam) {
					/* Release contaminants back into the water column*/
                    //fprintf(bm->logFile,"Calling Gain_Contaminants from Update_Detritus EPIFUNA WC SED_EP_FF\n");
					Gain_Contaminants(bm, boxLayerInfo, EPIFAUNA, WC, guild, cohort,FunctGroupArray[guild].releaseNH[cohort], 0, initialBiomass);
				}

			} else {
				/* Contaminants transferred to detritus */
                //fprintf(bm->logFile,"Calling Group_Transfer_Contaminant from Update_Detritus EPIFAUNA WC - %s-%d with initialBiomass: %e prodnDR: %e prodnDL: %e\n", FunctGroupArray[guild].groupCode, cohort, initialBiomass, FunctGroupArray[guild].prodnDR[cohort], FunctGroupArray[guild].prodnDL[cohort]);

                Group_Transfer_Contaminant(bm, boxLayerInfo, EPIFAUNA, WC, RefDetIndex, 0, guild, cohort, FunctGroupArray[guild].prodnDR[cohort], 0, initialBiomass, bm->dtsz_stored, 0, 12);
				Group_Transfer_Contaminant(bm, boxLayerInfo, EPIFAUNA, WC, LabDetIndex, 0, guild, cohort, FunctGroupArray[guild].prodnDL[cohort], 0, initialBiomass, bm->dtsz_stored, 0, 13);

				/* Release contaminants back into the water column*/
                //fprintf(bm->logFile,"Calling Gain_Contaminants from Update_Detritus EPIFUNA WC\n");
                if(bm->flag_detritus_contam) {
                    Gain_Contaminants(bm, boxLayerInfo, EPIFAUNA, WC, guild, cohort,FunctGroupArray[guild].releaseNH[cohort], 0, initialBiomass);
                }
			}

			/* Transfer due to predation of detritus and bacteria */
            //fprintf(bm->logFile,"Calling Group_Transfer_Contaminant from Update_Detritus from feeding - %s-%d with %e SED grazebact: %e grazeDR: %e, grazeDL: %e EATINGinfo bact: %e EATINGinfoDR: %e EATINGinfoDL: %e and WC grazebact: %e grazeDR: %e, grazeDL: %e EATINGinfo bact: %e EATINGinfoDR: %e EATINGinfoDL: %e\n", FunctGroupArray[guild].groupCode, cohort, initialBiomass, GRAZEinfo[SedBactIndex][0][SED], GRAZEinfo[RefDetIndex][0][SED], GRAZEinfo[LabDetIndex][0][SED],  EATINGinfo[SedBactIndex][0][SED],EATINGinfo[RefDetIndex][0][SED], EATINGinfo[LabDetIndex][0][SED],  GRAZEinfo[pelagicBactIndex][0][WC], GRAZEinfo[RefDetIndex][0][WC], GRAZEinfo[LabDetIndex][0][WC], EATINGinfo[pelagicBactIndex][0][WC],EATINGinfo[RefDetIndex][0][WC], EATINGinfo[LabDetIndex][0][WC]);

            //fprintf(bm->logFile,"Calling Group_Transfer_Contaminant from Update_Detritus from feeding - %s-%d with WC grazebact: %e\n", FunctGroupArray[guild].groupCode, cohort, GRAZEinfo[pelagicBactIndex][0][WC]);
            
            Group_Transfer_Contaminant(bm, boxLayerInfo, EPIFAUNA, WC, guild, cohort, pelagicBactIndex, 0, GRAZEinfo[pelagicBactIndex][0][WC], 0, EATINGinfo[pelagicBactIndex][0][WC], bm->dtsz_stored, 1, 14);
			Group_Transfer_Contaminant(bm, boxLayerInfo, EPIFAUNA, SED, guild, cohort, SedBactIndex, 0, GRAZEinfo[SedBactIndex][0][SED], 0, EATINGinfo[SedBactIndex][0][SED], bm->dtsz_stored, 1, 15);

            if(bm->flag_detritus_contam) { // If allowing transfer of contaminants due to eating detritus
                Group_Transfer_Contaminant(bm, boxLayerInfo, EPIFAUNA, WC, guild, cohort, RefDetIndex, 0, GRAZEinfo[RefDetIndex][0][WC], 0, EATINGinfo[RefDetIndex][0][WC], bm->dtsz_stored, 1, 16);
                Group_Transfer_Contaminant(bm, boxLayerInfo, EPIFAUNA, WC, guild, cohort, LabDetIndex, 0, GRAZEinfo[LabDetIndex][0][WC], 0, EATINGinfo[LabDetIndex][0][WC], bm->dtsz_stored, 1, 17);
                
                Group_Transfer_Contaminant(bm, boxLayerInfo, EPIFAUNA, SED, guild, cohort, RefDetIndex, 0, GRAZEinfo[RefDetIndex][0][SED], 0, EATINGinfo[RefDetIndex][0][SED], bm->dtsz_stored, 1, 18);
                Group_Transfer_Contaminant(bm, boxLayerInfo, EPIFAUNA, SED, guild, cohort, LabDetIndex, 0, GRAZEinfo[LabDetIndex][0][SED], 0, EATINGinfo[LabDetIndex][0][SED], bm->dtsz_stored, 1, 19);
            }
		}
		break;
	case ICE_BASED:	/* Intentional follow through */
	case LAND_BASED:
		/* Do nothing yet */
		break;
    case MIXED:
        quit("How did we get here as should come through a primary habitat\n");
        break;
    default:
        quit("How did we get here as this isn't even a habitat type slot but nTotHabTypes\n");
        break;
	}
}

void Update_Debug_Info(MSEBoxModel *bm, BoxLayerValues *boxLayerInfo, int habitatType, int guild, int cohort) {
	switch (habitatType) {
	case WC:
		/* Diagnostic information storage */
		if (FunctGroupArray[guild].groupAgeType == AGE_STRUCTURED_BIOMASS) {
			boxLayerInfo->DebugInfo[guild][WC][DiagnostNH_id] += FunctGroupArray[guild].releaseNH[cohort];
			boxLayerInfo->DebugInfo[guild][WC][DiagnostDL_id] += FunctGroupArray[guild].prodnDL[cohort] - GRAZEinfo[LabDetIndex][0][habitatType];
			boxLayerInfo->DebugInfo[guild][WC][DiagnostDR_id] += FunctGroupArray[guild].prodnDR[cohort] - GRAZEinfo[RefDetIndex][0][habitatType];
		} else {
			boxLayerInfo->DebugInfo[guild][WC][DiagnostNH_id] = FunctGroupArray[guild].releaseNH[cohort];
			boxLayerInfo->DebugInfo[guild][WC][DiagnostDL_id] = FunctGroupArray[guild].prodnDL[cohort] - GRAZEinfo[LabDetIndex][0][habitatType];
			boxLayerInfo->DebugInfo[guild][WC][DiagnostDR_id] = FunctGroupArray[guild].prodnDR[cohort] - GRAZEinfo[RefDetIndex][0][habitatType];
		}
		break;
	case SED:
		/* Diagnostic information storage */
		if (FunctGroupArray[guild].groupAgeType == AGE_STRUCTURED_BIOMASS) {
			boxLayerInfo->DebugInfo[guild][SED][DiagnostNHsed_id] += FunctGroupArray[guild].releaseNH[cohort];
			boxLayerInfo->DebugInfo[guild][SED][DiagnostDLsed_id] += FunctGroupArray[guild].prodnDL[cohort]
					- FunctGroupArray[guild].uptakeDL;
			boxLayerInfo->DebugInfo[guild][SED][DiagnostDRsed_id] += FunctGroupArray[guild].prodnDR[cohort]
					- FunctGroupArray[guild].uptakeDR;
		} else {
			boxLayerInfo->DebugInfo[guild][SED][DiagnostNHsed_id] = FunctGroupArray[guild].releaseNH[cohort];
			boxLayerInfo->DebugInfo[guild][SED][DiagnostDLsed_id] = FunctGroupArray[guild].prodnDL[cohort]
					- FunctGroupArray[guild].uptakeDL;
			boxLayerInfo->DebugInfo[guild][SED][DiagnostDRsed_id] = FunctGroupArray[guild].prodnDR[cohort]
					- FunctGroupArray[guild].uptakeDR;
		}

		if (FunctGroupArray[guild].groupType != SED_BACT) {
			boxLayerInfo->DebugInfo[guild][SED][DiagnostDLsed_id] += -GRAZEinfo[LabDetIndex][0][SED];
			boxLayerInfo->DebugInfo[guild][SED][DiagnostDRsed_id] += -GRAZEinfo[RefDetIndex][0][SED];
		}
		break;
	case EPIFAUNA:

		if (FunctGroupArray[guild].groupType == SED_EP_OTHER || FunctGroupArray[guild].groupType == SED_EP_FF || FunctGroupArray[guild].groupType
				== MOB_EP_OTHER) {
			/* Diagnostic information storage */
			if (FunctGroupArray[guild].groupType == SED_EP_OTHER) {
				boxLayerInfo->DebugInfo[guild][EPIFAUNA][DiagnostNHsed_id] = FunctGroupArray[guild].releaseNH[cohort];
			} else {
				if (FunctGroupArray[guild].diagTol == 2 && it_count == 1)
					boxLayerInfo->DebugInfo[guild][EPIFAUNA][DiagnostNHsed_id] = FunctGroupArray[guild].releaseNH[cohort];
				else
					boxLayerInfo->DebugInfo[guild][EPIFAUNA][DiagnostNH_id] = FunctGroupArray[guild].releaseNH[cohort];

			}

			boxLayerInfo->DebugInfo[guild][EPIFAUNA][DiagnostDL_id] = -GRAZEinfo[LabDetIndex][0][WC];
			boxLayerInfo->DebugInfo[guild][EPIFAUNA][DiagnostDR_id] = -GRAZEinfo[RefDetIndex][0][WC];
			boxLayerInfo->DebugInfo[guild][EPIFAUNA][DiagnostDLsed_id] = FunctGroupArray[guild].prodnDL[cohort] - GRAZEinfo[LabDetIndex][0][SED];
			boxLayerInfo->DebugInfo[guild][EPIFAUNA][DiagnostDRsed_id] = FunctGroupArray[guild].prodnDR[cohort] - GRAZEinfo[RefDetIndex][0][SED];

		} else {
			if (FunctGroupArray[guild].groupAgeType == AGE_STRUCTURED_BIOMASS) {
				/* Diagnostic information storage */
				boxLayerInfo->DebugInfo[guild][EPIFAUNA][DiagnostNH_id] += FunctGroupArray[guild].releaseNH[cohort] -FunctGroupArray[guild].uptakeNH[cohort] / wcLayerThick;
				boxLayerInfo->DebugInfo[guild][EPIFAUNA][DiagnostDL_id] += FunctGroupArray[guild].prodnDL[cohort] - GRAZEinfo[LabDetIndex][0][WC];
				boxLayerInfo->DebugInfo[guild][EPIFAUNA][DiagnostDR_id] += FunctGroupArray[guild].prodnDR[cohort] - GRAZEinfo[RefDetIndex][0][WC];
				boxLayerInfo->DebugInfo[guild][EPIFAUNA][DiagnostDLsed_id] += -GRAZEinfo[LabDetIndex][0][SED];
				boxLayerInfo->DebugInfo[guild][EPIFAUNA][DiagnostDRsed_id] += -GRAZEinfo[RefDetIndex][0][SED];
			} else {
				/* Diagnostic information storage */
				boxLayerInfo->DebugInfo[guild][EPIFAUNA][DiagnostNH_id] = FunctGroupArray[guild].releaseNH[cohort];
				boxLayerInfo->DebugInfo[guild][EPIFAUNA][DiagnostDL_id] = FunctGroupArray[guild].prodnDL[cohort] - GRAZEinfo[LabDetIndex][0][WC];
				boxLayerInfo->DebugInfo[guild][EPIFAUNA][DiagnostDR_id] = FunctGroupArray[guild].prodnDR[cohort] - GRAZEinfo[RefDetIndex][0][WC];
				boxLayerInfo->DebugInfo[guild][EPIFAUNA][DiagnostDLsed_id] = -GRAZEinfo[LabDetIndex][0][SED];
				boxLayerInfo->DebugInfo[guild][EPIFAUNA][DiagnostDRsed_id] = -GRAZEinfo[RefDetIndex][0][SED];
			}
		}

		if (!(FunctGroupArray[guild].diagTol == 2 && it_count == 1)) {
			boxLayerInfo->DebugInfo[guild][EPIFAUNA][DiagnostDR_id] += -FunctGroupArray[guild].transDR[cohort] / wcLayerThick;
			boxLayerInfo->DebugInfo[guild][EPIFAUNA][DiagnostDRsed_id] += FunctGroupArray[guild].transDR[cohort] / wcLayerThick;
		}
		break;
    case LAND_BASED:
    case ICE_BASED:
        break;
    case MIXED:
        quit("How did we get here as should come through a primary habitat\n");
        break;
    default:
        quit("How did we get here as this isn't even a habitat type slot but nTotHabTypes\n");
        break;
	}
}

/**
 *
 * Phytoplankton growth:
 * The specific growth rate mu is the product of a maximum growth rate, mum, a
 * light-limitation factor hI, and a nutrient limitation factor, hN. The
 * formulation is identical for large and small phytoplankton, and suspended
 * microphytobenthos, but the parameters vary.
 *	\brief The process function for group where the invertType is defined as
 *	either LG_PHY or SM_PHY.
 *
 *
 *	Nutrient limitation allows for N or Si limitation, also calculate uptake of NO3,
 *	NH4 and Si here. Disn't put day flag in phytoplankton as light limitation will stop growth.
 *
 *	Note that only lysis assumed in the water column, mortality (rather than lysis) occurs in
 *  the sediment
 *
 */
int Phytoplankton_Process(MSEBoxModel *bm, FILE *llogfp, HABITAT_TYPES habitatType, int guild, int cohort, BoxLayerValues *boxLayerInfo) {
	double NH, NO, NHs, NOs, Si, Fe, IRR, pH, initialBiomass, initialSedBiomass, initialEpiPhyteBiomass, mS_sp, mL_sp, mE_sp, area_hab;
	double *tracerArray;
	int lim_case = one_nut_lim, flag_sp;
	double uptakeNO, uptakeSi, uptakeFe, hN, uptakeP, uptakeC;
	double P, PRatio, C, CRatio;
	double mum = FunctGroupArray[guild].scaled_mum[0];
	double FDL = 1.0;
	int isGlobal = (FunctGroupArray[guild].diagTol == 2 && it_count == 1);

	flag_sp = (int) (FunctGroupArray[guild].speciesParams[flag_id]);
	if (flag_sp) {

		tracerArray = getTracerArray(boxLayerInfo, habitatType);

		NH = tracerArray[NH3_i];
		NO = tracerArray[NO3_i];
		IRR = tracerArray[Light_i];
		Si = tracerArray[Si_i];
		Fe = tracerArray[MicroNut_i];
		P = C = PRatio = CRatio = 0.0;
		if (bm->track_pH == TRUE) {
			pH = tracerArray[pH_i];
		} else {
			pH = 1.0;
		}

		if(bm->track_atomic_ratio == TRUE){
			PRatio = tracerArray[FunctGroupArray[guild].addRatioTracers[0][p_id]];
			CRatio = tracerArray[FunctGroupArray[guild].addRatioTracers[0][c_id]];
			P = tracerArray[P_i];
			C = tracerArray[C_i];
		}

		if (bm->flag_macro_model && FunctGroupArray[guild].groupType == SEAGRASS) {
			initialBiomass = tracerArray[FunctGroupArray[guild].totNTracers[main_biomass_id]];
			initialSedBiomass = tracerArray[FunctGroupArray[guild].totNTracers[below_ground_biomass_id]];
			initialEpiPhyteBiomass = tracerArray[FunctGroupArray[guild].totNTracers[epiphyte_biomass_id]];
		} else {
			initialBiomass = tracerArray[FunctGroupArray[guild].totNTracers[main_biomass_id]];
			initialSedBiomass = 0.0;
			initialEpiPhyteBiomass = 0.0;
		}
		switch (habitatType) {
		case WC:
			if (((FunctGroupArray[guild].groupType == LG_PHY || FunctGroupArray[guild].groupType == MICROPHTYBENTHOS)) && FunctGroupArray[guild].isSiliconDependant)
				lim_case = bm->flagnut;

			if(FunctGroupArray[guild].groupType == MICROPHTYBENTHOS)
				mum = mum * MB_wc;

			Primary_Production(bm, llogfp, guild, bm->flagmicro, lim_case, 0, initialBiomass,
					boxLayerInfo->DIN, NH, NO, Si, Fe, P, PRatio, C, CRatio, IRR, mum, 1.0, 0, 0, 0, &uptakeNO, &uptakeSi, &uptakeFe, &uptakeP, &uptakeC, &hN);

            // TODO: I think this is a bug and hangover from when did not have plankton mortality explciitly nut will screw with older models so include in bug trap flag
            if (bm->flag_replicated_old_PPmort) {
                /* Zero out the mortality value so we can use generic rate of change equations later */
                FunctGroupArray[guild].mortality[0] = 0.0;
            }

			boxLayerInfo->NutsLost[habitatType][NH_id] += FunctGroupArray[guild].uptakeNH[cohort];
			boxLayerInfo->NutsLost[habitatType][NO_id] += uptakeNO;
			boxLayerInfo->NutsLost[habitatType][Si_id] += uptakeSi;
			boxLayerInfo->NutsLost[habitatType][Fe_id] += uptakeFe;
			boxLayerInfo->NutsLost[habitatType][P_id] += uptakeP;
			boxLayerInfo->NutsLost[habitatType][C_id] += uptakeC;

			boxLayerInfo->DetritusProd[habitatType][DLdet_id] += FunctGroupArray[guild].lysis[cohort] + FunctGroupArray[guild].mortality[0];

			/* Contaminants transfer into detritus */
			if(bm->track_contaminants){
                //fprintf(bm->logFile,"Calling Group_Transfer_Contaminant from Phytoplankton_Process WC WC - %s-%d with initialBiomass: %e lysis: %e\n", FunctGroupArray[guild].groupCode, cohort, initialBiomass, FunctGroupArray[guild].lysis[cohort]);
                
                Group_Transfer_Contaminant(bm, boxLayerInfo, WC, WC, LabDetIndex, 0, guild, cohort,  (FunctGroupArray[guild].lysis[cohort] + FunctGroupArray[guild].mortality[0]), 0, initialBiomass, bm->dtsz_stored, 1, 20);  // Was case 0 for need_prop
			}
			FunctGroupArray[guild].chl = initialBiomass;

			/* Diagnostic information storage */
			boxLayerInfo->DebugInfo[guild][habitatType][DiagnostNH_id] = -FunctGroupArray[guild].uptakeNH[cohort];
			boxLayerInfo->DebugInfo[guild][habitatType][DiagnostDL_id] = FunctGroupArray[guild].lysis[cohort];

			if(bm->track_atomic_ratio == TRUE){
				/* Change in Biomass in phyto due to lysis handled in final flux calc so just need to account for gain in DL here */
				Gain_Element(bm, boxLayerInfo, habitatType, LabDetIndex, 0, guild, cohort, FunctGroupArray[guild].lysis[cohort], WC, isGlobal);

				/* Gain due to growth */
				Gain_Element(bm, boxLayerInfo, habitatType, guild, 0, guild, 0, FunctGroupArray[guild].growth[0],  WC, isGlobal);
			}

			break;
		case SED:

			if ((int) (FunctGroupArray[guild].speciesParams[flag_id])) {

				//TODO: Comment why this is different.
				if (FunctGroupArray[guild].groupType == MICROPHTYBENTHOS) {
					/*
					 * Microphytobenthos growth.
					 * The specific growth rate mu is the product of a maximum growth rate, mum, a
					 * light-limitation factor hI, and a nutrient limitation factor, hN.
					 */
					/* Nutrient limitation allows for N or Si limitation and calculate uptake of NO3, NH4
					 */
					Primary_Production(bm, llogfp, guild, bm->flagmicro, bm->flagnut, 0, initialBiomass,
							boxLayerInfo->DIN, NH, NO, Si, Fe, P, PRatio,  C, CRatio, IRR, FunctGroupArray[guild].scaled_mum[0], 1.0, 0, 0, 0, &uptakeNO, &uptakeSi, &uptakeFe, &uptakeP, &uptakeC, &hN);

					boxLayerInfo->NutsLost[habitatType][NH_id] += FunctGroupArray[guild].uptakeNH[cohort];
					boxLayerInfo->NutsLost[habitatType][NO_id] += uptakeNO;
					boxLayerInfo->NutsLost[habitatType][Si_id] += uptakeSi;
					boxLayerInfo->NutsLost[habitatType][Fe_id] += uptakeFe;
					boxLayerInfo->NutsLost[habitatType][P_id] += uptakeP;
					boxLayerInfo->NutsLost[habitatType][C_id] += uptakeC;

					boxLayerInfo->DetritusProd[habitatType][DLdet_id] += FunctGroupArray[guild].lysis[cohort];

					/* Contaminants transfer into detritus */
					if(bm->track_contaminants){
                        //fprintf(bm->logFile,"Calling Group_Transfer_Contaminant from Update_Detritus SED SED - %s-%d with initialBiomass: %e lysis: %e\n", FunctGroupArray[guild].groupCode, cohort, initialBiomass, FunctGroupArray[guild].lysis[cohort]);
                        
                        Group_Transfer_Contaminant(bm, boxLayerInfo, SED, SED, LabDetIndex, 0, guild, cohort,  FunctGroupArray[guild].lysis[cohort], 0, initialBiomass, bm->dtsz_stored, 1, 21);   // Was case 0 for need_prop
					}


					if(bm->track_atomic_ratio == TRUE){

						/* Change in Biomass in phyto due to lysis handled in final flux calc so just need to account for gain in DL here */
						Gain_Element(bm, boxLayerInfo, habitatType, LabDetIndex, 0, guild, cohort, FunctGroupArray[guild].lysis[cohort], SED, isGlobal);

						/* Gain due to growth */
						Gain_Element(bm, boxLayerInfo, habitatType, guild, 0, guild, 0, FunctGroupArray[guild].growth[0],  SED, isGlobal);
					}
					
					FunctGroupArray[guild].chl = initialBiomass;

					/* Diagnostic information storage */
					boxLayerInfo->DebugInfo[guild][habitatType][DiagnostNH_id] = -FunctGroupArray[guild].uptakeNH[cohort];
					boxLayerInfo->DebugInfo[guild][habitatType][DiagnostDL_id] = FunctGroupArray[guild].lysis[cohort];

				} else {
					/*Phytoplankton suffer natural mortality in the sediment */
					FunctGroupArray[guild].growth[cohort] = 0;

					mL_sp = Ecology_Get_Linear_Mortality(bm, guild, cohort, cohort);
					/* Not sure that we will need this but will include it for completeness */
					mE_sp = FunctGroupArray[guild].cohortSpeciesParams[cohort][mE_id];
                    mS_sp = Acidif_Mort(bm, guild);

					FunctGroupArray[guild].mortality[cohort] = (mS_sp + mL_sp + mE_sp) * initialBiomass;
					boxLayerInfo->DetritusProd[SED][DLdet_id] += FunctGroupArray[guild].mortality[cohort];

					/* Contaminants transfer into detritus */
					if(bm->track_contaminants){
                        //fprintf(bm->logFile,"Calling Group_Transfer_Contaminant from Update_Detritus SED SED - %s-%d with initialBiomass: %e mort: %e\n", FunctGroupArray[guild].groupCode, cohort, initialBiomass, FunctGroupArray[guild].mortality[cohort]);

                        Group_Transfer_Contaminant(bm, boxLayerInfo, SED, SED, LabDetIndex, 0, guild, cohort,  FunctGroupArray[guild].mortality[cohort], 0, initialBiomass, bm->dtsz_stored, 1, 22);   // Was case 0 for need_prop
					}


					bm->calcMLinearMort[guild][current_id] += (mL_sp * initialBiomass);
					bm->calcELinearMort[guild][current_id] += ((mS_sp + mE_sp) * initialBiomass);

					if(bm->track_contaminants)
						Contaminant_Record_Death(bm, guild, cohort, initialBiomass);

					if(bm->track_atomic_ratio == TRUE){

						/* Change in Biomass in phyto due to lysis handled in final flux calc so just need to account for gain in DL here */
						Gain_Element(bm, boxLayerInfo, habitatType, LabDetIndex, 0, guild, cohort, FunctGroupArray[guild].mortality[cohort], SED, isGlobal);
					}

					//bm->calcTrackedMort[guild][0][0][ongoingM1_id] += (mL_sp * initialBiomass) * bm->SP_prms[guild][Mdt_id]); - won't count this now as confusing when trying to get estimates for the active component
					FunctGroupArray[guild].chl = initialBiomass;

					/* Diagnostic information storage */
					boxLayerInfo->DebugInfo[guild][SED][DiagnostDLsed_id] = FunctGroupArray[guild].mortality[cohort];

					/* Kill off cysts too -TODO should we actually used the SED pool as the cyst storage facility rather than a cysts array? */
					cysts[guild][bm->current_box][cystenter_id] = (1.0 - (mS_sp + mL_sp + mE_sp)) * cysts[guild][bm->current_box][cystenter_id];
				}

			} else {
				FunctGroupArray[guild].mortality[cohort] = 0.0;
				FunctGroupArray[guild].chl = 0.0;
			}
			break;
		case EPIFAUNA:
			IRR = boxLayerInfo->localSEDTracers[Light_i];

			//TODO: Need to find out why this is so different/.
			if (FunctGroupArray[guild].groupType == SEAGRASS) {

                /* Seagrass version */
                
				NHs = boxLayerInfo->localSEDTracers[NH3_i];
				NOs = boxLayerInfo->localSEDTracers[NO3_i];
				NH = boxLayerInfo->localWCTracers[NH3_i];
				NO = boxLayerInfo->localWCTracers[NO3_i];

				area_hab = Get_Species_Area_Hab(bm, guild, 0, boxLayerInfo);

				if(bm->track_atomic_ratio == TRUE){
					P =  boxLayerInfo->localSEDTracers[P_i];
					C =  boxLayerInfo->localSEDTracers[C_i];
				}

				/* BEC CHANGED for CREAM TESTING */
				//area_hab = 1;

				if (bm->flag_macro_model) {
					Grow_Macrophytes(bm, llogfp, guild, cohort, 0, one_nut_lim, 1, initialBiomass, initialSedBiomass, initialEpiPhyteBiomass,
							boxLayerInfo->DINs, NHs, NOs, boxLayerInfo->DIN, NH, NO, Si, Fe, P, C, IRR, 1.0,
							FunctGroupArray[guild].speciesParams[mS_id] * boxLayerInfo->DIN, FunctGroupArray[guild].speciesParams[max_id], area_hab, &uptakeNO, &hN, PRatio, CRatio, &uptakeP, &uptakeC);

					switch(cohort){
						case main_biomass_id:
							FDL = FDL_SG_leaves;
							break;
						case epiphyte_biomass_id:
							FDL = 1.0;
							break;
						case below_ground_biomass_id:
							FDL = FDL_SG_roots;
							break;
					}
				} else {
					Primary_Production(bm, llogfp, guild, 0, one_nut_lim, 1, initialBiomass, boxLayerInfo->DINs, NHs, NOs, Si, Fe, P, PRatio, C, CRatio, IRR, FunctGroupArray[guild].scaled_mum[0], 1.0, FunctGroupArray[guild].speciesParams[mS_id] * boxLayerInfo->DIN, FunctGroupArray[guild].speciesParams[max_id], area_hab, &uptakeNO, &uptakeSi, &uptakeFe, &uptakeP, &uptakeC, &hN);
				}

                if (bm->flag_macro_model && (cohort == epiphyte_biomass_id))  {
                    boxLayerInfo->NutsLost[WC][NH_id] += FunctGroupArray[guild].uptakeNH[cohort] / wcLayerThick;
                    boxLayerInfo->NutsLost[WC][NO_id] += uptakeNO / wcLayerThick;
					boxLayerInfo->NutsLost[WC][P_id] += uptakeP / wcLayerThick;
					boxLayerInfo->NutsLost[WC][C_id] += uptakeC / wcLayerThick;
                } else {
					boxLayerInfo->NutsLost[SED][NH_id] += FunctGroupArray[guild].uptakeNH[cohort] / (smLayerThick * sporosity);
					boxLayerInfo->NutsLost[SED][NO_id] += uptakeNO / (smLayerThick * sporosity);
					boxLayerInfo->NutsLost[SED][P_id] += uptakeP / (smLayerThick * sporosity);
					boxLayerInfo->NutsLost[SED][C_id] += uptakeC / (smLayerThick * sporosity);
				}

				boxLayerInfo->DetritusProd[SED][DLdet_id] += (FDL * FunctGroupArray[guild].mortality[cohort] / smLayerThick);
				boxLayerInfo->DetritusProd[SED][DRdet_id] += ((1 - FDL) * FunctGroupArray[guild].mortality[cohort] / smLayerThick);

				/* Contaminants transfer into detritus */
				if(bm->track_contaminants){
                    //fprintf(bm->logFile,"Calling Group_Transfer_Contaminant from Update_Detritus EPIFAUNA SED - %s-%d with initialBiomass: %e FDL: %e\n", FunctGroupArray[guild].groupCode, cohort, initialBiomass, (FDL * FunctGroupArray[guild].mortality[cohort] / smLayerThick));

                    Group_Transfer_Contaminant(bm, boxLayerInfo, EPIFAUNA, SED, LabDetIndex, 0, guild, cohort, (FDL * FunctGroupArray[guild].mortality[cohort] / smLayerThick), 0, initialBiomass, bm->dtsz_stored, 1, 23);   // Was case 0 for need_prop
					Group_Transfer_Contaminant(bm, boxLayerInfo, EPIFAUNA, SED, RefDetIndex, 0, guild, cohort, ((1 - FDL) * FunctGroupArray[guild].mortality[cohort] / smLayerThick), 0, initialBiomass, bm->dtsz_stored, 1, 24);   // Was case 0 for need_prop
				}


				if(bm->track_atomic_ratio == TRUE){
					/* Change in Biomass in phyto due to moratlity handled in final flux calc so just need to account for gain in DL here */
					Gain_Element(bm, boxLayerInfo, SED, LabDetIndex, 0, guild, cohort, (FDL * FunctGroupArray[guild].mortality[cohort] / smLayerThick), EPIFAUNA, isGlobal);
					Gain_Element(bm, boxLayerInfo, SED, RefDetIndex, 0, guild, cohort, ((1 - FDL) * FunctGroupArray[guild].mortality[cohort] / smLayerThick), EPIFAUNA, isGlobal);

					/* Gain due to growth */
					Gain_Element(bm, boxLayerInfo, habitatType, guild, 0, guild, 0, FunctGroupArray[guild].growth[0],  EPIFAUNA, isGlobal);
				}

				/* Diagnostic information storage */
				boxLayerInfo->DebugInfo[guild][EPIFAUNA][DiagnostNHsed_id] = -FunctGroupArray[guild].uptakeNH[cohort] / (smLayerThick * sporosity);
				boxLayerInfo->DebugInfo[guild][EPIFAUNA][DiagnostDLsed_id] = FunctGroupArray[guild].mortality[cohort] / smLayerThick;
			} else {
                
                /* Algal version */
                
				NH = boxLayerInfo->localWCTracers[NH3_i];
				NO = boxLayerInfo->localWCTracers[NO3_i];
				area_hab = Get_Species_Area_Hab(bm, guild, 0, boxLayerInfo);

				if(bm->track_atomic_ratio == TRUE){
					P =  boxLayerInfo->localWCTracers[P_i];
					C =  boxLayerInfo->localWCTracers[C_i];
				}

				Primary_Production(bm, llogfp, guild, 0, one_nut_lim, 1, initialBiomass, boxLayerInfo->DIN, NH, NO, Si, Fe, P, PRatio, C, CRatio, IRR,
						FunctGroupArray[guild].scaled_mum[0], 1.0, FunctGroupArray[guild].speciesParams[mS_id] * surf_stress,
						FunctGroupArray[guild].speciesParams[max_id], area_hab, &uptakeNO, &uptakeSi, &uptakeFe, &uptakeP, &uptakeC, &hN);

				boxLayerInfo->NutsLost[WC][NH_id] += FunctGroupArray[guild].uptakeNH[cohort] / wcLayerThick;
				boxLayerInfo->NutsLost[WC][NO_id] += uptakeNO / wcLayerThick;
				boxLayerInfo->NutsLost[WC][P_id] += uptakeP / wcLayerThick;
				boxLayerInfo->NutsLost[WC][C_id] += uptakeC / wcLayerThick;

				if(bm->ecotest > 1){
					if(!_finite((double)boxLayerInfo->NutsLost[WC][NH_id])){
						printf("wcLayerThick = %e\n", wcLayerThick);
						printf("FunctGroupArray[guild].uptakeNH[cohort] = %e\n", FunctGroupArray[guild].uptakeNH[cohort]);
						quit("ERROR: uptakeNH is finite\n");

					}
				}

				boxLayerInfo->DetritusProd[WC][DLdet_id] += FunctGroupArray[guild].mortality[cohort] / wcLayerThick;

				/* Contaminants transfer into detritus */
				if(bm->track_contaminants){
                    //fprintf(bm->logFile,"Calling Group_Transfer_Contaminant from Update_Detritus EPIFAUNA SED - %s-%d with initialBiomass: %e mort: %e\n", FunctGroupArray[guild].groupCode, cohort, initialBiomass, FunctGroupArray[guild].mortality[cohort] / wcLayerThick);
                    
                    Group_Transfer_Contaminant(bm, boxLayerInfo, EPIFAUNA, SED, LabDetIndex, 0, guild, cohort,  FunctGroupArray[guild].mortality[cohort] / wcLayerThick, 0, initialBiomass, bm->dtsz_stored, 1, 25);   // Was case 0 for need_prop
				}

				if(bm->track_atomic_ratio == TRUE){

					/* Change in Biomass in phyto due to mortality handled in final flux calc so just need to account for gain in DL here */
					Gain_Element(bm, boxLayerInfo, WC, LabDetIndex, 0, guild, cohort, (FunctGroupArray[guild].mortality[cohort] / wcLayerThick), EPIFAUNA, isGlobal);

					/* Gain due to growth */
					Gain_Element(bm, boxLayerInfo, habitatType, guild, 0, guild, 0, FunctGroupArray[guild].growth[0],  EPIFAUNA, isGlobal);
				}

				/* Diagnostic information storage */
				boxLayerInfo->DebugInfo[guild][EPIFAUNA][DiagnostNH_id] = -FunctGroupArray[guild].uptakeNH[cohort] / wcLayerThick;
				boxLayerInfo->DebugInfo[guild][EPIFAUNA][DiagnostDL_id] = FunctGroupArray[guild].mortality[cohort] / wcLayerThick;
			}

			break;
		case ICE_BASED: // Primary producers completely constrained to ice
			IRR = boxLayerInfo->localICETracers[Light_i];

			if (FunctGroupArray[guild].groupType == LG_PHY || FunctGroupArray[guild].groupType == MICROPHTYBENTHOS)
				lim_case = bm->flagnut;

			Ice_PrimaryProduction(bm, llogfp, guild, bm->flagmicro, lim_case, 0, initialBiomass, boxLayerInfo->DIN, NH, NO, Si, Fe, P, PRatio, IRR, pH, 1.0, 0, 0, 0, &uptakeNO,
					&uptakeSi, &uptakeFe, &hN);

			boxLayerInfo->NutsLost[habitatType][NH_id] += FunctGroupArray[guild].uptakeNH[cohort];
			boxLayerInfo->NutsLost[habitatType][NO_id] += uptakeNO;

//			boxLayerInfo->NutsLost[habitatType][Si_id] += uptakeSi;
			boxLayerInfo->NutsLost[habitatType][Fe_id] += uptakeFe;
			boxLayerInfo->DetritusProd[habitatType][DLdet_id] += (FunctGroupArray[guild].lysis[cohort] + FunctGroupArray[guild].mortality[0]);
			FunctGroupArray[guild].chl = initialBiomass;

			/* Contaminants transfer into detritus */
			if(bm->track_contaminants){
                //fprintf(bm->logFile,"Calling Group_Transfer_Contaminant from Update_Detritus ICE ICE - %s-%d with initialBiomass: %e mort+lysis: %e\n", FunctGroupArray[guild].groupCode, cohort, initialBiomass, (FunctGroupArray[guild].lysis[cohort] + FunctGroupArray[guild].mortality[0]));

                Group_Transfer_Contaminant(bm, boxLayerInfo, ICE_BASED, ICE_BASED, LabDetIndex, 0, guild, cohort, (FunctGroupArray[guild].lysis[cohort] + FunctGroupArray[guild].mortality[0]), 0, initialBiomass, bm->dtsz_stored, 1, 26);   // Was case 0 for need_prop
			}

			/* Diagnostic information storage */
			boxLayerInfo->DebugInfo[guild][habitatType][DiagnostNH_id] = -FunctGroupArray[guild].uptakeNH[cohort];
			boxLayerInfo->DebugInfo[guild][habitatType][DiagnostDL_id] = (FunctGroupArray[guild].lysis[cohort] + FunctGroupArray[guild].mortality[0]);
			break;
		case LAND_BASED: // Primary producers completely constrained to land
			Land_PrimaryProduction(bm);
			break;
        case MIXED:
            quit("How did we get here as should come through a primary habitat\n");
            break;
        default:
			quit("Process type %d not recognised.\n", habitatType);
			break;
		}
	}

	return TRUE;
}

/**
 *
 *	<b>Sediment</b>
 *  Infaunal benthic groups in the sediment:
 *	Formulations are as for other consumers (say zooplankton), but can only
 *	exploit resources within the sediment and die off if below the oxygen
 *  depth or oxygen is too low.
 *
 *  Nutrient limitation allows for N limitation and calculate uptake of NO3, NH4.
 * 	There is also a maximum biomass, MAmax.
 *
 *	<b>Epibenthic</b>
 *	In the case of macroalgae and seagrasses, additional mortality processes
 *	have been represented. The most important component of macroalgae are
 *	filamentous loosely attached or unattached algae. These are not found in
 *	shallow exposed areas in bays, even under conditions of high light and
 *	nutrients, because of the effects of wave action and turbulence. To represent
 *	these effects, a mortality term related to bottom stress has been included.
 *	Use global surf_stress as proxy containing errosion rates.
 *
 *	Nutrient limitation allows for N limitation and calculate uptake of NO3, NH4.
 *	Seagrasses are thought to be adversely affected by high nutrient levels,
 *	possibly through the growth of epiphytic algae. This is represented simply
 *	here by including a mortality term proportional to water column DIN.
 *
 */
int Invert_Consumers_Process(MSEBoxModel *bm, FILE *llogfp, HABITAT_TYPES habitatType, int guild, int cohort, BoxLayerValues *boxLayerInfo) {
	double DL = 0.0, DR = 0.0, mat_pcnt = 0.0, area_hab = 1.0, realised_mum = 0.0;
	double initialBiomass = 0;
	int feed_while_spawn = 1, preyID, hab, prey_chrt, stage;
	int addGrazeInfo = FALSE;
	int spawn_now = 0;
	double PB_scale = p_PBwc;
	double BB_scale = 0.0, mL_other = 0.0, DLsed = 0.0, DRsed = 0.0, avail_Ice_Bact= 0.0;
	double *tracerArray = getTracerArray(boxLayerInfo, habitatType);
	double hO_SP = 1.0;
	double O2 = tracerArray[Oxygen_i];
	double E2_sp = FunctGroupArray[guild].speciesParams[E2_id];
	double bact_DL = 0, bact_DR = 0, sedbact_DL = 0, sedbact_DR = 0;
	double eatBiomass = -1;

	if ( FunctGroupArray[guild].groupAgeType == BIOMASS )
		spawn_now = 0;
	else
		spawn_now = (int) (EMBRYO[guild].Spawn_Now[cohort][EMBRYO[guild].next_spawn]);

	switch(habitatType){
	case WC:
		initialBiomass = tracerArray[FunctGroupArray[guild].totNTracers[cohort]];
		bact_DL = boxLayerInfo->PB_DL;
		bact_DR = boxLayerInfo->PB_DR;
		DL = boxLayerInfo->localWCTracers[FunctGroupArray[LabDetIndex].totNTracers[0]];
		DR = boxLayerInfo->localWCTracers[FunctGroupArray[RefDetIndex].totNTracers[0]];
		break;
	case SED:
		initialBiomass = tracerArray[FunctGroupArray[guild].totNTracers[cohort]];
		sedbact_DL = boxLayerInfo->BB_DL;
		sedbact_DR = boxLayerInfo->BB_DR;
		DLsed = boxLayerInfo->localSEDTracers[FunctGroupArray[LabDetIndex].totNTracers[0]];
		DRsed = boxLayerInfo->localSEDTracers[FunctGroupArray[RefDetIndex].totNTracers[0]];
		break;
	case EPIFAUNA:
		initialBiomass = boxLayerInfo->localWCTracers[FunctGroupArray[guild].totNTracers[cohort]];
		bact_DL = boxLayerInfo->PB_DL;
		bact_DR = boxLayerInfo->PB_DR;
		if(FunctGroupArray[guild].groupType == PWN){
			sedbact_DL = boxLayerInfo->BB_DL;
			sedbact_DR = boxLayerInfo->BB_DR;
			DLsed = boxLayerInfo->localSEDTracers[FunctGroupArray[LabDetIndex].totNTracers[0]];
			DRsed = boxLayerInfo->localSEDTracers[FunctGroupArray[RefDetIndex].totNTracers[0]];
		}
		DL = boxLayerInfo->localWCTracers[FunctGroupArray[LabDetIndex].totNTracers[0]];
		DR = boxLayerInfo->localWCTracers[FunctGroupArray[RefDetIndex].totNTracers[0]];
		break;
	case ICE_BASED:
		initialBiomass = tracerArray[FunctGroupArray[guild].totNTracers[cohort]];
		bact_DL = boxLayerInfo->ICEB_DL;
		bact_DR = boxLayerInfo->ICEB_DR;
		DL = boxLayerInfo->localICETracers[FunctGroupArray[LabDetIndex].totNTracers[0]];
		DR = boxLayerInfo->localICETracers[FunctGroupArray[RefDetIndex].totNTracers[0]];
		break;
	case LAND_BASED:
		// Nothing to do as yet
		quit("Invert_Consumers_Process - no support for land based invert consumers yet \n");
		break;
    case MIXED:
        quit("How did we get here as should come through a primary habitat\n");
        break;
    default:
        quit("How did we get here as this isn't even a habitat type slot but nTotHabTypes\n");
        break;
	}

	/* Calculate the linear mortality due to oxygen */
	hO_SP = Oxygen(bm->O2case, O2, FunctGroupArray[guild].speciesParams[KO2_id], FunctGroupArray[guild].speciesParams[KO2LIM_id],
			FunctGroupArray[guild].speciesParams[mD_id]);

	/* The mL_other value will be added to the groups linear mortality value FunctGroupArray[guild].cohortSpeciesParams[cohort][mL_id]
	 * in Imvertebrate_activites*/
	mL_other = (1.0 - hO_SP) * FunctGroupArray[guild].speciesParams[mO_id];

	if (FunctGroupArray[guild].groupAgeType == AGE_STRUCTURED_BIOMASS) {
		feed_while_spawn = (int) (FunctGroupArray[guild].speciesParams[feed_while_spawn_id]);

		/* Getting rid of this results in a huge change to output - worried this is a big bug in the original code.
		 * Turning this on effectively add the grazeLive values twice - the are added once in vertebrate activities.
		 *
		 * */

		// TODO: Get rid of this - pretty sure its a bug in the orginal code.
		if (FunctGroupArray[guild].groupType == PWN && cohort == adult_id && habitatType != EPIFAUNA)
			addGrazeInfo = TRUE;
	}
    
	if ((int) (FunctGroupArray[guild].speciesParams[flag_id]) && (int) (FunctGroupArray[guild].speciesParams[active_id]) &&
			(current_depth <= (-1.0 * FunctGroupArray[guild].speciesParams[mindepth_id]) &&
			current_depth >= (-1.0 * FunctGroupArray[guild].speciesParams[maxdepth_id]) &&
            ((-1.0 * bm->boxes[bm->current_box].botz) <= FunctGroupArray[guild].speciesParams[maxtotdepth_id]))) {
    //if ((int) (FunctGroupArray[guild].speciesParams[flag_id]) && (int) (FunctGroupArray[guild].speciesParams[active_id]) &&
    //        (current_depth <= (-1.0 * FunctGroupArray[guild].speciesParams[mindepth_id]) &&
    //        current_depth >= (-1.0 * FunctGroupArray[guild].speciesParams[maxdepth_id]))) {

		if ((FunctGroupArray[guild].speciesParams[flag_lim_id] == simple_ben_lim) && (habitatType == SED))
			eatBiomass = max(small_num, initialBiomass * (1.0 - initialBiomass / (FunctGroupArray[guild].speciesParams[max_id] * (area_flat + area_soft) + small_num)));
		else
			eatBiomass = initialBiomass;

		/* All other feeding regimes, plus find available habitat */
		area_hab = 1.0;
		switch (habitatType) {
		case WC:
			/* If this is the top layer and ice is active allow grazing into the ice layer */
			if(bm->ice_on == TRUE){
				if(bm->current_layer == bm->boxes[bm->current_box].nz - 1){
					Calculate_Ice_Prey_Avail(bm, boxLayerInfo, guild, PREYinfo, &avail_Ice_Bact);
					/* for now ignoring the amount of ice bacteria available in ice - we are not scaling based on nutrient depth  so the avail_Ice_Bact value is ignore */
				}
			}

			/* Do nothing */
			break;

		case SED:
			/* Availability of sediment fauna */
			Calculate_Sediment_Prey_Avail(bm, boxLayerInfo, guild, PREYinfo, &BB_scale);
			BB_scale = BB_scale * p_BBben;

			PB_scale = 0;
			area_hab = Get_Species_Area_Hab(bm, guild, 0, boxLayerInfo);
			break;

		case EPIFAUNA:
			area_hab = Get_Species_Area_Hab(bm, guild, 0, boxLayerInfo);

			//sDLscale = min(1,k_refsDL/(DLsed+small_num));

			// Plant efficiency set to 0 for large groups?
			// This because the are handled in water column box?
			if (FunctGroupArray[guild].diagTol == 2 && it_count == 1) {
				E2_sp = 0.0;
			}

			Calculate_Sediment_Prey_Avail(bm, boxLayerInfo, guild, PREYinfo, &BB_scale);
			BB_scale = BB_scale * p_BBben;

			//TODO -  Get rid of the special case - will change output but differences should be small. - they are quite large
			if (FunctGroupArray[guild].groupType == PWN) {

				/* The prey availability value has already been scaled by sDLscale in atecology.
				 * Multiplying here actually results in a cancelling of the value when used in the calculations
				 * in invertebrate_activites.
				 */
				DLsed = DLsed * boxLayerInfo->sDLscale;

				// Should this always be used for all epibenthic groups?
				// Perhaps this should be a parameter?
				// Think it should be dependent in where the species sits in the water column?
				// Perhaps dependence on the location of the prey? If eating sediment groups then this value
				// should be used else should be using the water column value.
				PB_scale = p_PBben;
			}
			break;
		case ICE_BASED:
			FunctGroupArray[guild].iceBact_Scale = 1.0;
			break;
		case LAND_BASED:	/* Not supported */
			break;
        case MIXED:
            quit("How did we get here as should come through a primary habitat\n");
            break;
        default:
            quit("How did we get here as this isn't even a habitat type slot but nTotHabTypes\n");
            break;
		}

		stage = FunctGroupArray[guild].cohort_stage[cohort];
		if ((FunctGroupArray[guild].groupAgeType == AGE_STRUCTURED_BIOMASS) && (stage == adult_id)) {
			if ((int) (EMBRYO[guild].Spawn_Now[cohort][EMBRYO[guild].next_spawn] == TRUE))
				//mat_pcnt = FunctGroupArray[guild].INVpopratio[juv_id][adult_id];
				mat_pcnt = FunctGroupArray[guild].scaled_FSPB[stage];  // Should really be this as in the mature age class
				//mat_pcnt = 1.0;
		}

		/* If scaling growth by available habitat - do so now */
		realised_mum = FunctGroupArray[guild].scaled_mum[cohort];
        if(bm->flag_benthos_sediment_link) {
            realised_mum *= area_hab;
        }
        
        /*
        if (guild == bm->which_check) {
                fprintf(llogfp, "Time: %e box%d-%d %s has realised_mum: %e (%e) area_hab: %e flag_benthos_sediment_link: %d\n",
                        bm->dayt, bm->current_box, bm->current_layer, FunctGroupArray[guild].groupCode, realised_mum, FunctGroupArray[guild].scaled_mum[cohort], area_hab, bm->flag_benthos_sediment_link);
        }
         */

		Eat(bm, llogfp, (int) FunctGroupArray[guild].speciesParams[predcase_id], guild, cohort, eatBiomass, FunctGroupArray[guild].scaled_C[cohort] * hO_SP, realised_mum, FunctGroupArray[guild].speciesParams[KL_id], FunctGroupArray[guild].speciesParams[KU_id], FunctGroupArray[guild].speciesParams[vl_id], FunctGroupArray[guild].speciesParams[ht_id], FunctGroupArray[guild].speciesParams[E1_id], E2_sp, FunctGroupArray[guild].speciesParams[E3_id], FunctGroupArray[guild].speciesParams[E4_id], feed_while_spawn, spawn_now, mat_pcnt, PREYinfo, GRAZEinfo, CATCHGRAZEinfo, eatBiomass);

        /**
		 * Add up the grazeLive values in the eat order.
		 */
		for (preyID = 0; preyID < bm->K_num_tot_sp; preyID++) {
			for (prey_chrt = 0; prey_chrt < FunctGroupArray[preyID].numCohortsXnumGenes; prey_chrt++) {
				for (hab = WC; hab <  bm->num_active_habitats; hab++) {
					if(GRAZEinfo[preyID][prey_chrt][hab] > 0){
						if (bm->flag_olddiet && (FunctGroupArray[guild].diagTol == 2))
							UpdateTrackedMort(bm, llogfp, guild, cohort, habitatType, (HABITAT_TYPES)hab, preyID, prey_chrt, boxLayerInfo, 1.0, 1);

						else
							UpdateTrackedMort(bm, llogfp, guild, cohort, habitatType, (HABITAT_TYPES)hab, preyID, prey_chrt, boxLayerInfo, 1.0, 0);
						/* If this is done the grazeLive values will have been calculated twice  - the grazelive values are added in eat?*/
						if (addGrazeInfo == TRUE) {
							if (FunctGroupArray[preyID].groupType != LAB_DET && FunctGroupArray[preyID].groupType != REF_DET && FunctGroupArray[preyID].groupType
									!= CARRION) {
								FunctGroupArray[guild].GrazeLive[cohort] += GRAZEinfo[preyID][prey_chrt][hab];
							}
						}
					}
				}
			}
		}
        
        /*
		 *  Should this be added?
		 *	When calculating the living prey carrion is added
		 *	But when updating grazeLive carrion not added in eat - but its added below?
		 *
		 * */
		if (FunctGroupArray[guild].diagTol == 2 && it_count == 1 && habitatType == SED) {
			FunctGroupArray[guild].GrazeLive[cohort] += GRAZEinfo[CarrionIndex][0][SED];
		}

        Invert_Activities(bm, boxLayerInfo, habitatType, llogfp, guild, cohort, initialBiomass, area_hab, FunctGroupArray[guild].speciesParams[E1_id], FunctGroupArray[guild].speciesParams[E3_id],
				FunctGroupArray[guild].speciesParams[E4_id], bact_DL, bact_DR, sedbact_DL, sedbact_DR, PB_scale, BB_scale, mL_other,
				FunctGroupArray[guild].speciesParams[FDL_id], DL, DR, DLsed, DRsed, GRAZEinfo);

		//TODO: Replace below with this.
		//Update_Detritus(bm, boxLayerInfo, habitatType, guild, guild, cohort);
		//Update_Debug_Info(bm, habitatType, guild, cohort);
        
		// TODO: Get rid of the special case.
		if (habitatType == WC || FunctGroupArray[guild].groupType == CEP) {
			Update_Detritus(bm, boxLayerInfo, WC, guild, guild, cohort, initialBiomass);
			Update_Debug_Info(bm, boxLayerInfo, WC, guild, cohort);
		} else if (habitatType == SED) {
			Update_Detritus(bm, boxLayerInfo, SED, guild, guild, cohort, initialBiomass);
			Update_Debug_Info(bm, boxLayerInfo, SED, guild, cohort);
		} else {
			Update_Detritus(bm, boxLayerInfo, EPIFAUNA, guild, guild, cohort, initialBiomass);
			Update_Debug_Info(bm, boxLayerInfo, EPIFAUNA, guild, cohort);
		}

		if (FunctGroupArray[guild].diagTol == 2 && it_count == 1)
			RecycledNHglobal += FunctGroupArray[guild].releaseNH[cohort];


		if (bm->debug == debug_prey_biology_process && bm->dayt >= bm->checkstart && bm->dayt < bm->checkstop) {
			Print_Eat_Diagnostics(bm, llogfp, guild, habitatType, 1);
			/* Print out diagnostic values */
			Print_Eat_Diagnostics(bm, llogfp, guild, habitatType, 4);
			fprintf(llogfp, "Invert_Consumers_Process %s, eatBiomass = %.20e, - outcome DetritusProd[SED][DLdet_id = %.20Le, NutsProd[SED][NH_id] = %.20Le\n", FunctGroupArray[guild].groupCode,eatBiomass, boxLayerInfo->DetritusProd[SED][DLdet_id], boxLayerInfo->NutsProd[SED][NH_id]);
		}
	}
	return TRUE;
}


/**
 *
 *	Coral processes - taken from Invert_Consumer_Processes with all explict activities covered in atcoral.c
 *
 */
int Coral_Process(MSEBoxModel *bm, FILE *llogfp, HABITAT_TYPES habitatType, int guild, int cohort, BoxLayerValues *boxLayerInfo) {
	double DL = 0.0, DR = 0.0;
	int feed_while_spawn = 1, preyID, hab, prey_chrt;
	int spawn_now = 0;
	double PB_scale = p_PBwc;
	double BB_scale = 0.0, mL_other = 0.0, DLsed = 0.0, DRsed = 0.0;
	double *tracerArray = getTracerArray(boxLayerInfo, habitatType);
	double hO_SP = 1.0;
	double O2 = tracerArray[Oxygen_i];
	double sed_level = 0.0;
	double E2_sp = FunctGroupArray[guild].speciesParams[E2_id];
	double bact_DL = 0, bact_DR = 0, sedbact_DL = 0, sedbact_DR = 0;
	double eatBiomass = -1;
	double NH, NO, Si, Fe, IRR, initialBiomass = 0.0, area_hab;
	int lim_case = one_nut_lim;
	double uptakeNO, uptakeSi, uptakeFe, hN, uptakeP, uptakeC;
	double P, PRatio, C, CRatio;
	double scaled_mum, rel_growth;
    
    if(bm->flagIsEstuary)
        sed_level = tracerArray[SED_i];
    
    if ( FunctGroupArray[guild].groupAgeType == BIOMASS )
		spawn_now = 0;
	else
		spawn_now = (int) (EMBRYO[guild].Spawn_Now[cohort][EMBRYO[guild].next_spawn]);

	Si = tracerArray[Si_i];
	Fe = tracerArray[MicroNut_i];
	P = C = PRatio = CRatio = 0.0;

	if(bm->track_atomic_ratio == TRUE){
		PRatio = tracerArray[FunctGroupArray[guild].addRatioTracers[0][p_id]];
		CRatio = tracerArray[FunctGroupArray[guild].addRatioTracers[0][c_id]];
		P = tracerArray[P_i];
		C = tracerArray[C_i];
	}

	switch(habitatType){
	case WC:
	case SED:
	case ICE_BASED:
	case LAND_BASED:
		quit("Coral can not be anything but an epibenthic (epifauna) group");
		break;
	case EPIFAUNA:
		initialBiomass = tracerArray[FunctGroupArray[guild].totNTracers[cohort]];
		bact_DL = boxLayerInfo->PB_DL;
		bact_DR = boxLayerInfo->PB_DR;
		DL = boxLayerInfo->localWCTracers[FunctGroupArray[LabDetIndex].totNTracers[0]];
		DR = boxLayerInfo->localWCTracers[FunctGroupArray[RefDetIndex].totNTracers[0]];
		break;
    case MIXED:
        quit("How did we get here as should come through a primary habitat\n");
        break;
    default:
        quit("How did we get here as this isn't even a habitat type slot but nTotHabTypes\n");
        break;
	}

	/* Calculate the linear mortality due to oxygen */
	hO_SP = Oxygen(bm->O2case, O2, FunctGroupArray[guild].speciesParams[KO2_id], FunctGroupArray[guild].speciesParams[KO2LIM_id],
			FunctGroupArray[guild].speciesParams[mD_id]);

	/* The mL_other value will be added to the groups linear mortality value FunctGroupArray[guild].cohortSpeciesParams[cohort][mL_id]
	 * in Imvertebrate_activites*/
	mL_other = (1.0 - hO_SP) * FunctGroupArray[guild].speciesParams[mO_id];

    /*
    fprintf(llogfp,"Time: %e box%d-%d doing %s - active: %e, mindepth: %e, maxdepth: %e, maxtotdepth: %e, current_depth: %e\n", bm->dayt, bm->current_box, bm->current_layer, FunctGroupArray[guild].groupCode, FunctGroupArray[guild].speciesParams[active_id], FunctGroupArray[guild].speciesParams[mindepth_id], FunctGroupArray[guild].speciesParams[maxdepth_id], FunctGroupArray[guild].speciesParams[maxtotdepth_id], current_depth);
    */
    if ((int) (FunctGroupArray[guild].speciesParams[flag_id]) && (int) (FunctGroupArray[guild].speciesParams[active_id]) &&
    		(current_depth <= (-1.0 * FunctGroupArray[guild].speciesParams[mindepth_id]) &&
    		current_depth >= (-1.0 * FunctGroupArray[guild].speciesParams[maxdepth_id]) &&
            ((-1.0 * bm->boxes[bm->current_box].botz) <= FunctGroupArray[guild].speciesParams[maxtotdepth_id]))) {
    //if ((int) (FunctGroupArray[guild].speciesParams[flag_id]) && (int) (FunctGroupArray[guild].speciesParams[active_id]) &&
    //    (current_depth <= (-1.0 * FunctGroupArray[guild].speciesParams[mindepth_id]) &&
    //     current_depth >= (-1.0 * FunctGroupArray[guild].speciesParams[maxdepth_id]))) {
        
          //  fprintf(llogfp,"Doing calculation for Coral_Process for %s-%d\n", FunctGroupArray[guild].groupCode, cohort);
            
		/**** Autotrophic processes ****/
		IRR = boxLayerInfo->localSEDTracers[Light_i];
		NH = boxLayerInfo->localWCTracers[NH3_i];
		NO = boxLayerInfo->localWCTracers[NO3_i];

        /* Scale growth based on having suitable area - so can reflect for corals that need to grow over turf or epilithic algar not macroalgae
           Also make sure the area_hab reflects this - use which ever is most limiting */
        area_hab = Get_Species_Area_Hab(bm, guild, 0, boxLayerInfo);
        scaled_mum = Coral_Space_Competition(bm, boxLayerInfo, llogfp, guild, cohort, FunctGroupArray[guild].scaled_mum[cohort]);
        rel_growth = scaled_mum / (FunctGroupArray[guild].scaled_mum[cohort] + small_num);
        if(rel_growth < area_hab)
            area_hab = rel_growth;

        Grow_Coral_Symbionts(bm, boxLayerInfo, llogfp, guild, cohort, lim_case, bm->flagmicro, initialBiomass, boxLayerInfo->DIN,
                NH, NO, Si, Fe, P, PRatio, C, CRatio, IRR, FunctGroupArray[guild].scaled_mum[cohort], 1.0,
				FunctGroupArray[guild].speciesParams[mS_id] * surf_stress, FunctGroupArray[guild].speciesParams[max_id],
				FunctGroupArray[guild].speciesParams[FDL_id], area_hab, &uptakeNO, &uptakeSi, &uptakeFe, &uptakeP, &uptakeC, &hN);

		boxLayerInfo->NutsLost[WC][NH_id] += FunctGroupArray[guild].uptakeNH[cohort] / wcLayerThick;
		boxLayerInfo->NutsLost[WC][NO_id] += uptakeNO / wcLayerThick;
		boxLayerInfo->NutsLost[WC][P_id] += uptakeP / wcLayerThick;
		boxLayerInfo->NutsLost[WC][C_id] += uptakeC / wcLayerThick;

		/* Do bleaching and recovery - here as a zooxanethllae related activity not a consumer activity */
		Do_Bleaching(bm, llogfp, guild, cohort, initialBiomass);

		/**** Heterotrophic processes ****/

		if ((FunctGroupArray[guild].speciesParams[flag_lim_id] == simple_ben_lim) && (habitatType == SED))
			eatBiomass = max(small_num, initialBiomass * (1.0 - initialBiomass / (FunctGroupArray[guild].speciesParams[max_id] * (area_flat + area_soft) + small_num)));
		else
			eatBiomass = initialBiomass;

		/* All other feeding regimes */
		Calculate_Sediment_Prey_Avail(bm, boxLayerInfo, guild, PREYinfo, &BB_scale);
		BB_scale = BB_scale * p_BBben;

		Eat(bm, llogfp, (int) FunctGroupArray[guild].speciesParams[predcase_id], guild, cohort, eatBiomass, FunctGroupArray[guild].scaled_C[cohort]
				* hO_SP, scaled_mum, FunctGroupArray[guild].speciesParams[KL_id],
				FunctGroupArray[guild].speciesParams[KU_id], FunctGroupArray[guild].speciesParams[vl_id], FunctGroupArray[guild].speciesParams[ht_id],
				FunctGroupArray[guild].speciesParams[E1_id], E2_sp, FunctGroupArray[guild].speciesParams[E3_id], FunctGroupArray[guild].speciesParams[E4_id],
				feed_while_spawn, spawn_now, 0.0, PREYinfo, GRAZEinfo, CATCHGRAZEinfo, eatBiomass);

		/**
		 * Add up the grazeLive values in the eat order.
		 */
		for (preyID = 0; preyID < bm->K_num_tot_sp; preyID++) {
			for (prey_chrt = 0; prey_chrt < FunctGroupArray[preyID].numCohortsXnumGenes; prey_chrt++) {
				for (hab = WC; hab < bm->num_active_habitats; hab++) { // Was <= EPIFAUNA
					if(GRAZEinfo[preyID][prey_chrt][hab] > 0){

						if (bm->flag_olddiet && (FunctGroupArray[guild].diagTol == 2))
							UpdateTrackedMort(bm, llogfp, guild, cohort, habitatType, (HABITAT_TYPES)hab, preyID, prey_chrt, boxLayerInfo, 1.0, 1);
						else
							UpdateTrackedMort(bm, llogfp, guild, cohort, habitatType, (HABITAT_TYPES)hab, preyID, prey_chrt, boxLayerInfo, 1.0, 0);
					}
				}
			}
		}

		Coral_Consumer_Activities(bm, habitatType, llogfp, guild, cohort, initialBiomass, FunctGroupArray[guild].speciesParams[max_id], IRR, area_hab,
				FunctGroupArray[guild].speciesParams[E1_id], FunctGroupArray[guild].speciesParams[E3_id], FunctGroupArray[guild].speciesParams[E4_id],
				bact_DL, bact_DR, sedbact_DL, sedbact_DR, PB_scale, BB_scale, mL_other,
				FunctGroupArray[guild].speciesParams[FDL_id], DL, DR, DLsed, DRsed, Si, GRAZEinfo);
        Coral_Limiting_Growth_Factors(bm, llogfp, guild, cohort, sed_level);

        boxLayerInfo->localWCTracers[Rugosity_i] = Calculate_Rugosity(bm, guild, cohort, llogfp, 1);
                
        //fprintf(llogfp,"Coral_Process: Time: %e box%d-%d %s-%d Rugosity: %e\n", bm->dayt, bm->current_box, bm->current_layer, FunctGroupArray[guild].groupCode, cohort, boxLayerInfo->localWCTracers[Rugosity_i]);

		Update_Detritus(bm, boxLayerInfo, EPIFAUNA, guild, guild, cohort, initialBiomass);
		Update_Debug_Info(bm, boxLayerInfo, EPIFAUNA, guild, cohort);

		if(bm->ecotest > 1){
			if(!_finite((double)boxLayerInfo->NutsLost[WC][NH_id])){
				printf("wcLayerThick = %e\n", wcLayerThick);
				printf("FunctGroupArray[guild].uptakeNH[cohort] = %e\n", FunctGroupArray[guild].uptakeNH[cohort]);
				quit("ERROR: uptakeNH is infinite in Coral_Processes\n");

			}
		}

		/* Diagnostic information storage */
		if (FunctGroupArray[guild].diagTol == 2 && it_count == 1)
			RecycledNHglobal += FunctGroupArray[guild].releaseNH[cohort];

		if (bm->debug == debug_prey_biology_process && bm->dayt >= bm->checkstart && bm->dayt < bm->checkstop) {
			Print_Eat_Diagnostics(bm, llogfp, guild, habitatType, 1);
			/* Print out diagnostic values */
			Print_Eat_Diagnostics(bm, llogfp, guild, habitatType, 4);
			fprintf(llogfp, "Coral_Process %s, eatBiomass = %.20e, - outcome DetritusProd[SED][DLdet_id = %.20Le, NutsProd[SED][NH_id] = %.20Le\n",
					FunctGroupArray[guild].groupCode,eatBiomass, boxLayerInfo->DetritusProd[SED][DLdet_id], boxLayerInfo->NutsProd[SED][NH_id]);
		}
	}
	return TRUE;
}

/**
 *
 *  /brief The DinoFlag functional groups process function.
 */

int Dinoflag_Process(MSEBoxModel *bm, FILE *llogfp, HABITAT_TYPES habitatType, int guild, int cohort, BoxLayerValues *boxLayerInfo) {
	double NH, NO, Si, Fe, IRR, initialBiomass, DL, DR, DFscale, DFphagotroph, mortality_scalar;
	//double DFgrazeTot; Set but never used.
	double *tracerArray;
	int predcase_sp, inv_feed_while_spawn = 1, inv_spawn_now = 0;
	int hab, preyID, prey_chrt;
	double inv_mat_pcnt = 0.0; // As want all to spawn as included in growth for invertebrate pools
	double mL_sp = Ecology_Get_Linear_Mortality(bm, guild, cohort, cohort);
	double mE_sp = FunctGroupArray[guild].cohortSpeciesParams[cohort][mE_id];
    double mS_sp = Acidif_Mort(bm, guild);
	double uptakeNO, uptakeSi, uptakeFe, hN, uptakeP, uptakeC;
	double PRatio, P, CRatio, C;
	double sp_GrazeFeed = 0.0;
	int isGlobal = (FunctGroupArray[guild].diagTol == 2 && it_count == 1);

    if ((int) (FunctGroupArray[guild].speciesParams[flag_id]) && (int) (FunctGroupArray[guild].speciesParams[active_id]) &&
    		(current_depth <= (-1.0 * FunctGroupArray[guild].speciesParams[mindepth_id]) &&
    		current_depth >= (-1.0 * FunctGroupArray[guild].speciesParams[maxdepth_id]) &&
            ((-1.0 * bm->boxes[bm->current_box].botz) <= FunctGroupArray[guild].speciesParams[maxtotdepth_id]))) {
    //if ((int) (FunctGroupArray[guild].speciesParams[flag_id]) && (int) (FunctGroupArray[guild].speciesParams[active_id]) &&
    //    (current_depth <= (-1.0 * FunctGroupArray[guild].speciesParams[mindepth_id]) &&
    //     current_depth >= (-1.0 * FunctGroupArray[guild].speciesParams[maxdepth_id]))) {
            
		tracerArray = getTracerArray(boxLayerInfo, habitatType);
		initialBiomass = tracerArray[FunctGroupArray[guild].totNTracers[main_biomass_id]];

		switch (habitatType) {
		case WC:
		case ICE_BASED:

			predcase_sp = (int) (FunctGroupArray[guild].speciesParams[predcase_id]);

			NH = tracerArray[NH3_i];
			NO = tracerArray[NO3_i];
			IRR = tracerArray[Light_i];
			Si = tracerArray[Si_i];
			Fe = tracerArray[MicroNut_i];
			DL = tracerArray[FunctGroupArray[LabDetIndex].totNTracers[0]];
			DR = tracerArray[FunctGroupArray[RefDetIndex].totNTracers[0]];
			P = C = PRatio = CRatio = 0.0;

			if(bm->track_atomic_ratio == TRUE){
				PRatio = tracerArray[FunctGroupArray[guild].addRatioTracers[0][p_id]];
				CRatio = tracerArray[FunctGroupArray[guild].addRatioTracers[0][c_id]];
				P = tracerArray[P_i];
				C = tracerArray[C_i];
			}

			/* Photosynthetic growth - as per the other phototrophs, though there is
			 some increase in efficiency at low light levels, represented here by
			 increasing effective light available */

			if (((IRR / FunctGroupArray[guild].speciesParams[KI_id]) < 0.1) && IRR != 0.0){
				FunctGroupArray[guild].SP_IRR = FunctGroupArray[guild].speciesParams[KI_id] * (0.01 * IRR + 0.018);
			} else {
				FunctGroupArray[guild].SP_IRR = IRR;
			}

			Primary_Production(bm, llogfp, guild, bm->flagmicro, one_nut_lim, 0, initialBiomass, boxLayerInfo->DIN, NH, NO, Si, Fe, P, PRatio, C, CRatio,
					FunctGroupArray[guild].SP_IRR, FunctGroupArray[guild].scaled_mum[0], FunctGroupArray[guild].speciesParams[E1_id], 0, 0, 0, &uptakeNO, &uptakeSi, &uptakeFe, &uptakeP, &uptakeC, &hN);

			if(bm->track_atomic_ratio == TRUE){
				/* Change in Biomass in phyto due to lysis handled in final flux calc so just need to account for gain in DL here */
				Gain_Element(bm, boxLayerInfo, habitatType, LabDetIndex, 0, guild, cohort, FunctGroupArray[guild].lysis[0], WC, isGlobal);

				/* Gain due to growth */
				Gain_Element(bm, boxLayerInfo, habitatType, guild, 0, guild, 0, FunctGroupArray[guild].growth[0],  WC, isGlobal);
			}


            // TODO: I think this is a bug and hangover from when did not have plankton mortality explciitly nut will screw with older models so include in bug trap flag
            if (bm->flag_replicated_old_PPmort) {
                /* Zero out the mortality value so we can use generic rate of change equations later */
                FunctGroupArray[guild].mortality[0] = 0.0;
            }
                
			/* Mixotrophic growth - modified from Stickney, Hood and Stoecker, 2000.
			 The impact of mixotrophy on planktonic marine ecosystems. Ecol. Model. 125: 203-230.
			 First determine potential grazing - dinoflagellates ae reported to feed on-
			 flagellates, diatoms, bacteria and cryptophytes */

			/* All other feeding regimes */
			Eat(bm, llogfp, predcase_sp, guild, cohort, initialBiomass, FunctGroupArray[guild].scaled_C[cohort],
					FunctGroupArray[guild].scaled_mum[cohort], FunctGroupArray[guild].speciesParams[KL_id], FunctGroupArray[guild].speciesParams[KU_id],
					FunctGroupArray[guild].speciesParams[vl_id], FunctGroupArray[guild].speciesParams[ht_id], FunctGroupArray[guild].speciesParams[E1_id],
					FunctGroupArray[guild].speciesParams[E2_id], FunctGroupArray[guild].speciesParams[E3_id], FunctGroupArray[guild].speciesParams[E4_id],
					inv_feed_while_spawn, inv_spawn_now, inv_mat_pcnt, PREYinfo, GRAZEinfo, CATCHGRAZEinfo, initialBiomass);

			if(habitatType == WC){
				GRAZEinfo[pelagicBactIndex][0][habitatType] = (GRAZEinfo[LabDetIndex][0][habitatType] * boxLayerInfo->PB_DL / (DL + small_num)
					+ GRAZEinfo[RefDetIndex][0][habitatType] * boxLayerInfo->PB_DR / (DR + small_num)) * p_PBwc;
			} else {
				/* DinoFlag in the ice */
				GRAZEinfo[IceBactIndex][0][habitatType] = (GRAZEinfo[LabDetIndex][0][habitatType] * boxLayerInfo->ICEB_DL / (DL + small_num)
						+ GRAZEinfo[RefDetIndex][0][habitatType] * boxLayerInfo->ICEB_DR / (DR + small_num)) * p_IBice;
			}

			FunctGroupArray[guild].GrazeLive[cohort] += GRAZEinfo[pelagicBactIndex][0][habitatType] + GRAZEinfo[LabDetIndex][0][habitatType]
					+ GRAZEinfo[RefDetIndex][0][habitatType];

			// Unlikely to be culturing a dinoflagellate phagotroph, but just in case
			if(FunctGroupArray[guild].isCultured || FunctGroupArray[guild].isSupplemented)
				sp_GrazeFeed = GRAZEinfo[AquacultFeedIndex][0][WC];
			else
				sp_GrazeFeed = 0.0;

            if(bm->ice_on){
                FunctGroupArray[guild].GrazeLive[cohort] += GRAZEinfo[IceBactIndex][0][habitatType];
            }

			/* Phagotrophic growth tops photosynthetic growth up to the maximum possible
			 if nutrients non-limiting unless this is more than could be physically grazed
			 (as that's where the extra nutrient is coming from) and then grazing sets the
			 addition growth.*/
			DFphagotroph = min(((double)FunctGroupArray[guild].GrazeLive[cohort] + sp_GrazeFeed), FunctGroupArray[guild].maxPhagotrophy);

			FunctGroupArray[guild].growth[cohort] += (FunctGroupArray[guild].speciesParams[E1_id] * DFphagotroph);

			if(bm->track_atomic_ratio == TRUE){
				Transfer_To_Pred(bm, boxLayerInfo, GRAZEinfo, habitatType, guild, cohort, 1.0, FunctGroupArray[guild].speciesParams[E1_id], 0, 0, habitatType);
			}

			/* Scale grazing to match what is actually required if could graze more than
			 is needed to match potential nutrient non-limited phototrophic growth */
			DFscale = DFphagotroph / ((double)FunctGroupArray[guild].GrazeLive[cohort] + small_num);

			for (preyID = 0; preyID < bm->K_num_tot_sp; preyID++) {
				for (prey_chrt = 0; prey_chrt < FunctGroupArray[preyID].numCohortsXnumGenes; prey_chrt++) {
					for (hab = WC; hab <  bm->num_active_habitats; hab++) {
						if(GRAZEinfo[preyID][prey_chrt][hab] > 0){
							if(bm->flag_olddiet)
								UpdateTrackedMort(bm, llogfp, guild, cohort, habitatType, (HABITAT_TYPES)hab, preyID, prey_chrt, boxLayerInfo, DFscale, 1);
							else
								UpdateTrackedMort(bm, llogfp, guild, cohort, habitatType, (HABITAT_TYPES)hab, preyID, prey_chrt, boxLayerInfo, DFscale, 0);
						}
					}
				}
			}
			/* If there is a scalar to apply then grab it now */
			if(bm->scale_all_mortality == TRUE){
				mortality_scalar = Ecology_Get_Mortality_Scalar(bm, guild, cohort);
			} else {
				mortality_scalar = 1.0;
			}

			//DFgrazeTot = FunctGroupArray[guild].GrazeLive[cohort] * DFscale;

			/* Must modify nutrient stress related lysis of dinoflagellates to take
			 into account mitigating effects of phagotrophy) - if grazing more than
			 what's needed to garner nutrients for photosynthesis then as if hN = 1.0
			 otherwise have contribution of hN = the proportion of photosynthetic nutrient
			 need made up by the grazing. */
			FunctGroupArray[guild].lysis[cohort] = (mortality_scalar * (FunctGroupArray[guild].speciesParams[KLYS_id] * initialBiomass / (hN + DFscale * (1.0 - hN)
					+ 0.1)));

			/* Unlike other consumers there is no excretion (production of NH) as
			 assimilation efficiency not growth was used in determining N obtained by
			 phagotrophy. */

			boxLayerInfo->NutsLost[WC][NH_id] += FunctGroupArray[guild].uptakeNH[cohort];

			if(bm->ecotest > 1){
				if(!_finite((double)boxLayerInfo->NutsLost[WC][NH_id])){
					printf("FunctGroupArray[guild].uptakeNH[cohort] = %e\n", FunctGroupArray[guild].uptakeNH[cohort]);
					quit("ERROR - updateNH is infinite\n");

				}
			}

			boxLayerInfo->NutsLost[WC][NO_id] += uptakeNO;
			boxLayerInfo->NutsLost[WC][Si_id] += uptakeSi;
			boxLayerInfo->NutsLost[WC][Fe_id] += uptakeFe;

			boxLayerInfo->DetritusLost[WC][DRdet_id] += GRAZEinfo[RefDetIndex][0][WC];
			boxLayerInfo->DetritusLost[WC][DLdet_id] += GRAZEinfo[LabDetIndex][0][WC];

			boxLayerInfo->DetritusProd[WC][DLdet_id] += FunctGroupArray[guild].lysis[cohort] + FunctGroupArray[guild].mortality[0];

			/* Contaminants transfer */
			if(bm->track_contaminants){
				/* Gains in DL due to mortality */
                //fprintf(bm->logFile,"Calling Group_Transfer_Contaminant from Update_Detritus WC WC - %s-%d with initialBiomass: %e lysis: %e\n", FunctGroupArray[guild].groupCode, cohort, initialBiomass, FunctGroupArray[guild].lysis[cohort]);
                
                Group_Transfer_Contaminant(bm, boxLayerInfo, WC, WC, LabDetIndex, 0, guild, cohort, (FunctGroupArray[guild].lysis[cohort] + FunctGroupArray[guild].mortality[0]), 0, initialBiomass, bm->dtsz_stored, 1, 27);   // Was case 0 for need_prop
			}

			if(bm->track_atomic_ratio == TRUE){
				/* Change in bysisiomass due to lysys handled in final flux calc so just need to account for gain in DL here */
				Gain_Element(bm, boxLayerInfo, WC, LabDetIndex, 0, guild, cohort,FunctGroupArray[guild].lysis[cohort], WC, isGlobal);
				boxLayerInfo->NutsLost[WC][P_id] += uptakeP;
				boxLayerInfo->NutsLost[WC][C_id] += uptakeC;
			}

			FunctGroupArray[guild].chl = initialBiomass;

			/* Diagnostic information storage */
			boxLayerInfo->DebugInfo[guild][WC][DiagnostNH_id] = -FunctGroupArray[guild].uptakeNH[cohort];
			boxLayerInfo->DebugInfo[guild][WC][DiagnostDL_id] = FunctGroupArray[guild].lysis[cohort];

			break;
		case SED:
			FunctGroupArray[guild].mortality[cohort] = (mS_sp + mL_sp + mE_sp) * initialBiomass;
			boxLayerInfo->DetritusProd[SED][DLdet_id] += FunctGroupArray[guild].mortality[cohort];

			bm->calcMLinearMort[guild][current_id] += (mL_sp * initialBiomass);
			bm->calcELinearMort[guild][current_id] += ((mS_sp + mE_sp) * initialBiomass);

			if(bm->track_atomic_ratio == TRUE){

				/* Change in Biomass in phyto due to mortality handled in final flux calc so just need to account for gain in DL here */
				Gain_Element(bm, boxLayerInfo, SED, LabDetIndex, 0, guild, cohort, FunctGroupArray[guild].mortality[cohort], SED, isGlobal);
			}

			/* Contaminants transfer */
			if(bm->track_contaminants){
                
                /* Gains in DL due to mortality */
				Group_Transfer_Contaminant(bm, boxLayerInfo, SED, SED, LabDetIndex, 0, guild, cohort, FunctGroupArray[guild].mortality[cohort], 0, initialBiomass, bm->dtsz_stored, 1, 28);   // Was case 0 for need_prop

				/* Record the deaths due to contaminants */
				Contaminant_Record_Death(bm, guild, cohort, initialBiomass);
			}

			FunctGroupArray[guild].chl = initialBiomass;

			/* Diagnostic information storage */
			boxLayerInfo->DebugInfo[guild][SED][DiagnostDLsed_id] = FunctGroupArray[guild].mortality[cohort];
			break;
		case EPIFAUNA:
		case LAND_BASED:
			quit("Dinoflag_Process no support for epi or land based dinoflags\n");
			/* Do nothing */
			break;
        case MIXED:
            quit("How did we get here as should come through a primary habitat\n");
            break;
        default:
            quit("How did we get here as this isn't even a habitat type slot but nTotHabTypes\n");
            break;
		}

	}
	if (bm->debug == debug_prey_biology_process && bm->dayt >= bm->checkstart && bm->dayt < bm->checkstop) {
		fprintf(llogfp, "Dinoflag_Process outcome DetritusProd[SED][DLdet_id = %.20Le\n", boxLayerInfo->DetritusProd[SED][DLdet_id]);
		Print_Eat_Diagnostics(bm, logfp, guild, habitatType, 1);
	}
	return TRUE;
}
/**
 *  \brief The process function for the PelagicBacteria functional group.
 *
 *
 ***
 Pelagic bacteria

 Use available nutrients, DON and detritus as growth substrate. Perform some
 nitrification and detrital breakdown. There are two kinds of pelagic bacteria here,
 the first is nitrifying which use detritus as their resource base (as in ERSEM) and
 the second is free bacteria (not associated with detritus) as of Fasham.

 Pelagic bacteria - Parslow and Fulton
 Assume the bacteria are attached to and colonise detritus (with set maximum biomass of bacteria per
 biomass of detritus - this may be logistic, k_PB = 1, or growth rates may on fall off
 when PB is close to maximum, then k_PB = 3). Under this scheme the steady state breakdown
 of DL works out to be

 max(0.0, m_PB * ( 1 - m_PB / mum_PB)) * X_PB_DL * DL / EDL_PB

 where m_PB is total specific mortality rate (i.e due to natural, oxygen and grazing mortality )
 **
 *
 *
 */
int Pelagic_Bacteria_Process(MSEBoxModel *bm, FILE *llogfp, HABITAT_TYPES habitatType, int guild, int cohort, BoxLayerValues *boxLayerInfo) {
	double *tracerArray;
	double NH, PB, DL, DR, O2, hO_SP, potential_PB, mortality_scalar;
	double mL_sp = Ecology_Get_Linear_Mortality(bm, guild, cohort, cohort);
	double mE_sp = FunctGroupArray[guild].cohortSpeciesParams[cohort][mE_id];
    double mS_sp = Acidif_Mort(bm, guild);
	int isGlobal = (FunctGroupArray[guild].diagTol == 2 && it_count == 1);


	if ((int) (FunctGroupArray[guild].speciesParams[flag_id])) {

		tracerArray = getTracerArray(boxLayerInfo, habitatType);

		O2 = tracerArray[Oxygen_i];
		NH = tracerArray[NH3_i];
		DL = tracerArray[FunctGroupArray[LabDetIndex].totNTracers[0]];
		DR = tracerArray[FunctGroupArray[RefDetIndex].totNTracers[0]];
		PB = tracerArray[FunctGroupArray[guild].totNTracers[0]];

		if (bm->debug == debug_prey_biology_process && bm->dayt >= bm->checkstart && bm->dayt < bm->checkstop) {
			fprintf(
					llogfp,
					"Pelagic_Bacteria_Process parameters: mL_sp = %e, FunctGroupArray[guild].speciesParams[mO_id] = %e, FunctGroupArray[guild].speciesParams[E3_id] = %e, FunctGroupArray[guild].speciesParams[FDMort_id] = %e\n",
					mL_sp, FunctGroupArray[guild].speciesParams[mO_id], FunctGroupArray[guild].speciesParams[E3_id],
					FunctGroupArray[guild].speciesParams[FDMort_id]);
		}

		hO_SP = Oxygen(2, O2, FunctGroupArray[guild].speciesParams[KO2_id], FunctGroupArray[guild].speciesParams[KO2LIM_id],
				FunctGroupArray[guild].speciesParams[mD_id]);


		if (flagkdrop)
			potential_PB = FunctGroupArray[guild].scaled_mum[cohort] * (boxLayerInfo->PB_DL * (1.0 - pow(boxLayerInfo->PB_DL / (XPB_DL * DL), 3.0))
					+ boxLayerInfo->PB_DR * (1.0 - pow(boxLayerInfo->PB_DR / (XPB_DR * DR), 3.0)));
		else {
			potential_PB = FunctGroupArray[guild].scaled_mum[cohort] * PB * pow(max(0.0,(1.0 - PB / (XPB_DL * DL
									+ XPB_DR * DR + small_num))), k_PB);
		}

		/* If there is a scalar to apply then grab it now */
		if(bm->scale_all_mortality == TRUE){
			mortality_scalar = Ecology_Get_Mortality_Scalar(bm, guild, cohort);
		} else {
			mortality_scalar = 1.0;
		}

		FunctGroupArray[guild].growth[cohort] = max(0.0, potential_PB);

		FunctGroupArray[guild].uptakeDL = FunctGroupArray[guild].growth[cohort] * boxLayerInfo->PB_DL / (PB
				* FunctGroupArray[guild].speciesParams[E3_id] + small_num);

		FunctGroupArray[guild].uptakeDR = FunctGroupArray[guild].growth[cohort] * boxLayerInfo->PB_DR / (PB
				* FunctGroupArray[guild].speciesParams[E4_id] + small_num);


		FunctGroupArray[guild].mortality[cohort] = mortality_scalar * (((mS_sp + mE_sp + mL_sp) + (1.0 - hO_SP) * FunctGroupArray[guild].speciesParams[mO_id]) * PB);
        
        FunctGroupArray[guild].prodnDR[cohort] = (FunctGroupArray[guild].uptakeDL * (1.0
				- FunctGroupArray[guild].speciesParams[E3_id]) + FunctGroupArray[guild].mortality[cohort]
				* FunctGroupArray[guild].speciesParams[FDMort_id]) * FPB_DR;

		FunctGroupArray[guild].prodnDON = (FunctGroupArray[guild].uptakeDL * (1.0 - FunctGroupArray[guild].speciesParams[E3_id])
				+ FunctGroupArray[guild].uptakeDR * (1.0 - FunctGroupArray[guild].speciesParams[E4_id])
				+ FunctGroupArray[guild].mortality[cohort] * FunctGroupArray[guild].speciesParams[FDMort_id]) * FPB_DON * (1.0 - FPB_DR);

		FunctGroupArray[guild].releaseNH[cohort] = (FunctGroupArray[guild].uptakeDL * (1.0
				- FunctGroupArray[guild].speciesParams[E3_id])) * (1.0 - FPB_DR - FPB_DON * (1.0 - FPB_DR))
				+ FunctGroupArray[guild].uptakeDR * (1.0 - FunctGroupArray[guild].speciesParams[E4_id]) * (1.0 - FPB_DON * (1.0 - FPB_DR))
				+ FunctGroupArray[guild].mortality[cohort] * (1.0 - FunctGroupArray[guild].speciesParams[FDMort_id] * FPB_DR
						- FunctGroupArray[guild].speciesParams[FDMort_id] * FPB_DON * (1.0 - FPB_DR));
        
        FunctGroupArray[guild].nitrif = K_nit * NH * Susp_Sed / (K_conc + small_num);

		if(bm->track_atomic_ratio == TRUE){

			/* Gain due to growth */
			Gain_Element(bm, boxLayerInfo, habitatType, guild, cohort, guild, cohort, FunctGroupArray[guild].growth[cohort],  WC, isGlobal);

			/* DR produced */
			Gain_Element(bm, boxLayerInfo, habitatType, RefDetIndex, 0, guild, cohort, FunctGroupArray[guild].prodnDR[cohort],  WC, isGlobal);

			/* DR and DL loose element due to uptake */
			Loose_Element(bm, boxLayerInfo, habitatType, LabDetIndex, 0, FunctGroupArray[guild].uptakeDL,  WC, isGlobal);
			Loose_Element(bm, boxLayerInfo, habitatType, RefDetIndex, 0, FunctGroupArray[guild].uptakeDR,  WC, isGlobal);

			/* Now keep track of ratios in PB broken down. */
			Calculate_Element_Release(bm, boxLayerInfo, guild, (FunctGroupArray[guild].prodnDON + FunctGroupArray[guild].releaseNH[cohort]), habitatType, WC);
		}


		/* Diagnostic information storage */
		boxLayerInfo->DebugInfo[guild][WC][DiagnostNH_id] = FunctGroupArray[guild].releaseNH[cohort]
				- FunctGroupArray[guild].nitrif;
		boxLayerInfo->DebugInfo[guild][WC][DiagnostDL_id] = -FunctGroupArray[guild].uptakeDL;
		boxLayerInfo->DebugInfo[guild][WC][DiagnostDR_id] = FunctGroupArray[guild].prodnDR[cohort]
				- FunctGroupArray[guild].uptakeDR;

		boxLayerInfo->DebugFluxInfo[guild][habitatType][gain_id] = FunctGroupArray[guild].growth[cohort];
		boxLayerInfo->DebugFluxInfo[guild][habitatType][loss_id] = FunctGroupArray[guild].mortality[cohort];

#ifdef BROKER_LINK_ENABLED
		if(do_BrokerLinkage){
			boxLayerInfo->DebugFluxInfo[guild][habitatType][loss_id] += bm->linkageInterface->linkageWCMortality[bm->current_box][bm->current_layer][guild][cohort];
		}
#endif

		if (bm->debug == debug_prey_biology_process && bm->dayt >= bm->checkstart && bm->dayt < bm->checkstop) {
			fprintf(
					llogfp,
					"Pelagic_Bacteria_Process outcomes: growth = %e, uptakeDL = %e, uptakeDR = %e, mortality = %e, prodnDR = %e, prodnDON = %e, releaseNH = %e, nitrif = %e \n",
					FunctGroupArray[guild].growth[cohort], FunctGroupArray[guild].uptakeDL,
					FunctGroupArray[guild].uptakeDR, FunctGroupArray[guild].mortality[cohort],
					FunctGroupArray[guild].prodnDR[cohort], FunctGroupArray[guild].prodnDON,
					FunctGroupArray[guild].releaseNH[cohort], FunctGroupArray[guild].nitrif);
		}

	}
	return TRUE;
}

/**
 *  \brief The process function for the SedimentBacteria functional group.
 *
 *	Use available nutrients and detritus as growth substrate. Perform some
 detrital breakdown - but other chemical breakdown and nitrification still retained
 explicitly below. There are two kinds of bacteria here, the first is aerobic and the
 second is anaerobic (or very top part of that community as most will be
 below that part of the sediment represented by the model).
 Assumes all unassimilated material is excreted (unlike other consumers where
 split across ammonia and detritus), though products of
 mortality are split across DL and NH in the usual way.
 *
 *
 */
int Sediment_Bacterica_Process(MSEBoxModel *bm, FILE *llogfp, HABITAT_TYPES habitatType, int guild, int cohort, BoxLayerValues *boxLayerInfo) {
	double mL_sp, mE_sp, mS_sp, biomass, hO_SP, BB_DL, BB_DR, potential_BB, DL, DR, O2, mortality_scalar;
	double *tracerArray = getTracerArray(boxLayerInfo, habitatType);
	int isGlobal = (FunctGroupArray[guild].diagTol == 2 && it_count == 1);


	if ((int) (FunctGroupArray[guild].speciesParams[flag_id])) {

		mL_sp = Ecology_Get_Linear_Mortality(bm, guild, cohort, cohort);
		mE_sp = FunctGroupArray[guild].cohortSpeciesParams[cohort][mE_id];
        mS_sp = Acidif_Mort(bm, guild);
		DL = tracerArray[FunctGroupArray[LabDetIndex].totNTracers[0]];
		DR = tracerArray[FunctGroupArray[RefDetIndex].totNTracers[0]];
		O2 = tracerArray[Oxygen_i];
		biomass = tracerArray[FunctGroupArray[guild].totNTracers[0]];
		tracerArray = getTracerArray(boxLayerInfo, habitatType);
        
        if (!biomass) {
            fprintf(llogfp, "Zero sediment bacterial biomass in box %d layer %d\n", bm->current_box, bm->current_layer);
            warn("Zero sediment bacterial biomass in box %d layer %d\n", bm->current_box, bm->current_layer);
        }

		//TODO: See if i can merge this code with the code in Calculate_SedBact_Scale().
		hO_SP = Oxygen(1, O2, FunctGroupArray[guild].speciesParams[KO2_id], FunctGroupArray[guild].speciesParams[KO2LIM_id],
				FunctGroupArray[guild].speciesParams[mD_id]);

		BB_DL = biomass * Bact_stim * XBB_DL * DL * hO_SP / (Bact_stim * XBB_DL * DL * hO_SP + Bact_stim * XBB_DR * DR * hO_SP + small_num);
		BB_DR = biomass * Bact_stim * XBB_DR * DR * hO_SP / (Bact_stim * XBB_DL * DL * hO_SP + Bact_stim * XBB_DR * DR * hO_SP + small_num);

		if (flagkdrop) {
			potential_BB = FunctGroupArray[guild].scaled_mum[cohort] * (BB_DL * (1.0 - pow(BB_DL / (XBB_DL * DL), 3.0)) + BB_DR * (1.0 - pow(BB_DR
					/ (XBB_DR * DR), 3.0)));
		} else {/* Gain due to growth */
			if(bm->track_atomic_ratio == TRUE){
				Gain_Element(bm, boxLayerInfo, habitatType, guild, cohort, guild, cohort, FunctGroupArray[guild].growth[cohort],  WC, isGlobal);
			}
			potential_BB = biomass * FunctGroupArray[guild].scaled_mum[cohort] * pow(
					max(0.0,(1.0 - biomass / (Bact_stim * XBB_DL * DL * hO_SP + Bact_stim * XBB_DR * DR * hO_SP + small_num))), k_BB);
		}

		/* If there is a scalar to apply then grab it now */
		if(bm->scale_all_mortality == TRUE){
			mortality_scalar = Ecology_Get_Mortality_Scalar(bm, guild, cohort);
		} else {
			mortality_scalar = 1.0;
		}


		FunctGroupArray[guild].growth[cohort] = max(0.0, potential_BB);
		FunctGroupArray[guild].uptakeDL = FunctGroupArray[guild].growth[cohort] * BB_DL / (biomass
				* FunctGroupArray[guild].speciesParams[E3_id] + small_num);
		FunctGroupArray[guild].uptakeDR = FunctGroupArray[guild].growth[cohort] * BB_DR / (biomass
				* FunctGroupArray[guild].speciesParams[E4_id] + small_num);
		FunctGroupArray[guild].mortality[cohort] = mortality_scalar * (((mS_sp + mL_sp + mE_sp) + (1.0 - hO_SP) * FunctGroupArray[guild].speciesParams[mO_id]) * biomass);

		bm->calcTrackedMort[guild][cohort][0][ongoingM1_id] += (FunctGroupArray[guild].mortality[cohort] * FunctGroupArray[guild].speciesParams[Mdt_id]);

		FunctGroupArray[guild].prodnDR[cohort] = (FunctGroupArray[guild].uptakeDL * (1.0
				- FunctGroupArray[guild].speciesParams[E3_id]) + FunctGroupArray[guild].mortality[cohort]
				* FunctGroupArray[guild].speciesParams[FDMort_id]) * FBB_DR;
		FunctGroupArray[guild].prodnDON = (FunctGroupArray[guild].uptakeDL * (1.0 - FunctGroupArray[guild].speciesParams[E3_id])
				+ FunctGroupArray[guild].uptakeDR * (1.0 - FunctGroupArray[guild].speciesParams[E4_id])
				+ FunctGroupArray[guild].mortality[cohort] * FunctGroupArray[guild].speciesParams[FDMort_id]) * FBB_DON * (1.0 - FBB_DR);
		FunctGroupArray[guild].releaseNH[cohort] = (FunctGroupArray[guild].uptakeDL * (1.0
				- FunctGroupArray[guild].speciesParams[E3_id])) * (1.0 - FBB_DR - FBB_DON * (1.0 - FBB_DR)) + FunctGroupArray[guild].uptakeDR
				* (1.0 - FunctGroupArray[guild].speciesParams[E4_id]) * (1.0 - FBB_DON * (1.0 - FBB_DR))
				+ FunctGroupArray[guild].mortality[cohort] * (1.0 - FunctGroupArray[guild].speciesParams[FDMort_id] * FBB_DR
						- FunctGroupArray[guild].speciesParams[FDMort_id] * FBB_DON * (1.0 - FBB_DR));

		if(bm->track_atomic_ratio == TRUE){

			/* Gain due to growth */
			Gain_Element(bm, boxLayerInfo, habitatType, guild, cohort, guild, cohort, FunctGroupArray[guild].growth[cohort],  WC, isGlobal);

			/* DR and DL loose element due to uptake */
			Loose_Element(bm, boxLayerInfo, habitatType, LabDetIndex, 0, FunctGroupArray[guild].uptakeDL,  WC, isGlobal);
			Loose_Element(bm, boxLayerInfo, habitatType, RefDetIndex, 0, FunctGroupArray[guild].uptakeDR,  WC, isGlobal);

			/* DR produced */
			Gain_Element(bm, boxLayerInfo, habitatType, RefDetIndex, 0, guild, cohort, FunctGroupArray[guild].prodnDR[cohort],  WC, isGlobal);

			/* Now keep track of ratios in BB mortality*/
			Calculate_Element_Release(bm, boxLayerInfo, guild, (FunctGroupArray[guild].prodnDON + FunctGroupArray[guild].releaseNH[cohort]), habitatType, SED);
		}

		/* Diagnostic information storage */
		boxLayerInfo->DebugInfo[guild][SED][DiagnostNHsed_id] = FunctGroupArray[guild].releaseNH[cohort];
		boxLayerInfo->DebugInfo[guild][SED][DiagnostDLsed_id] = -FunctGroupArray[guild].uptakeDL;
		boxLayerInfo->DebugInfo[guild][SED][DiagnostDRsed_id] = FunctGroupArray[guild].prodnDR[cohort]
				- FunctGroupArray[guild].uptakeDR;
	}
	return TRUE;
}
/**
 *  \brief The process function for the FilterFeeder and mobile epibenthos (SED_EP_FF or MOB_EP_OTHER)
 * functional groups.
 *
 *
 *  The filter feeder groups.
 *
 *  BFS - Shallow water filter feeders
 *  BFF - Other filter feeders
 *  BFD - Deep water filter feeders
 *
 *  And epibenthos like 
 *
 *  BMS - Shallow epibenthos
 *  BML - Megazoobenthos
 *
 */
int Epibenthic_Invert_Process(MSEBoxModel *bm, FILE *llogfp, HABITAT_TYPES habitatType, int guild, int cohort, BoxLayerValues *boxLayerInfo) {
	int preyID, hab, prey_chrt;
	double *tracerArray = getTracerArray(boxLayerInfo, habitatType);
    double biomass, eatBiomass, hO_SP, area_hab, NumSp, x_Sp, BB_scale, realised_mum;
	double Crwd_Effect = 1.0;
	double O2 = boxLayerInfo->localWCTracers[Oxygen_i];
	double DR = boxLayerInfo->localWCTracers[FunctGroupArray[RefDetIndex].totNTracers[0]];
	double DL = boxLayerInfo->localWCTracers[FunctGroupArray[LabDetIndex].totNTracers[0]];
	double DLsed = boxLayerInfo->localSEDTracers[FunctGroupArray[LabDetIndex].totNTracers[0]];
	double DRsed = boxLayerInfo->localSEDTracers[FunctGroupArray[RefDetIndex].totNTracers[0]];
	int inv_feed_while_spawn = 1;
	int inv_spawn_now = 0;
	double inv_mat_pcnt = 0.0; // As want all to spawn as included in growth for invertebrate pools
	int predcase_sp = (int) (FunctGroupArray[guild].speciesParams[predcase_id]);
	int isGlobal = (FunctGroupArray[guild].diagTol == 2 && it_count == 1);
	double loss;
	double avail_Ice_Bact = 0.0;
    
    /*
    fprintf(llogfp,"Time: %e box%d-%d doing %s - active: %e, mindepth: %e, maxdepth: %e, maxtotdepth: %e, current_depth: %e\n", bm->dayt, bm->current_box, bm->current_layer, FunctGroupArray[guild].groupCode, FunctGroupArray[guild].speciesParams[active_id], FunctGroupArray[guild].speciesParams[mindepth_id], FunctGroupArray[guild].speciesParams[maxdepth_id], FunctGroupArray[guild].speciesParams[maxtotdepth_id], current_depth);
     */
	/* Check that we are within the acceptable depth range for this group */
    if ((int) (FunctGroupArray[guild].speciesParams[flag_id]) && (int) (FunctGroupArray[guild].speciesParams[active_id]) &&
    		(current_depth <= (-1.0 * FunctGroupArray[guild].speciesParams[mindepth_id]) &&
    		current_depth >= (-1.0 * FunctGroupArray[guild].speciesParams[maxdepth_id]) &&
            ((-1.0 * bm->boxes[bm->current_box].botz) <= FunctGroupArray[guild].speciesParams[maxtotdepth_id]))) {
            
		biomass = tracerArray[FunctGroupArray[guild].totNTracers[cohort]];
		eatBiomass = biomass;
		area_hab = Get_Species_Area_Hab(bm, guild, cohort, boxLayerInfo);

		// Removed check that saw only groups with if FunctGroupArray[guild].diagTol < 2 have this check, that is now covered in read in instead
        // This means this so all epibenthos can now trigger habitat limitation
        // Removed habitat_dependency of this test
        if (FunctGroupArray[guild].speciesParams[flag_lim_id] == simple_ben_lim) {
            eatBiomass = max(small_num, biomass * (1.0 - biomass / (FunctGroupArray[guild].speciesParams[max_id] * area_hab + small_num)));

        } else if (FunctGroupArray[guild].speciesParams[flag_lim_id] == ersem_ben_lim) {
            NumSp = biomass - FunctGroupArray[guild].speciesParams[low_id] * area_hab;
            if (NumSp > 0.0) {
                x_Sp = NumSp * Util_Mich_Ment(NumSp, FunctGroupArray[guild].speciesParams[sat_id] * area_hab);
                Crwd_Effect = 1.0 - Util_Mich_Ment(x_Sp, FunctGroupArray[guild].speciesParams[thresh_id] * area_hab);
                Crwd_Effect *= Crwd_Effect;  // Used to be linear, but now quadratic so drops right away
            } else {
                Crwd_Effect = 1.0;
            }
            eatBiomass = biomass;
        }

		/* Calculate the oxygen limitation */
		hO_SP = Oxygen(bm->O2case, O2, FunctGroupArray[guild].speciesParams[KO2_id], FunctGroupArray[guild].speciesParams[KO2LIM_id],
				FunctGroupArray[guild].speciesParams[mD_id]);

		if (bm->debug == debug_prey_biology_process && bm->dayt >= bm->checkstart && bm->dayt < bm->checkstop) {
			fprintf(llogfp, "EpibenthicInvert process: %s - hO_SP = %e, Crwd_Effect = %e, low = %e, area_hab = %e, sat = %e, thresh = %e\n",
					FunctGroupArray[guild].groupCode, hO_SP, Crwd_Effect,
					FunctGroupArray[guild].speciesParams[low_id], area_hab, FunctGroupArray[guild].speciesParams[sat_id],
					FunctGroupArray[guild].speciesParams[thresh_id]);

		}

		/* Calculate the availability of sediment fauna and thus the BB_scale value */
		Calculate_Sediment_Prey_Avail(bm, boxLayerInfo, guild, PREYinfo, &BB_scale);
		BB_scale = BB_scale * p_BBben;

		/* If this is the top layer and ice is active allow grazing into the ice layer */
		if(bm->ice_on == TRUE){
			if(bm->current_layer == bm->boxes[bm->current_box].nz - 1){
				Calculate_Ice_Prey_Avail(bm, boxLayerInfo, guild, PREYinfo, &avail_Ice_Bact);
				/* for now ignoring the amount of ice bacteria available in ice - we are not scaling based on nutrient depth  so the avail_Ice_Bact value is ignore */
			}
		}

		/* If scaling growth by available habitat - do so now */
		realised_mum = FunctGroupArray[guild].scaled_mum[cohort];
		if(bm->flag_benthos_sediment_link) {
			realised_mum *= area_hab;
        } else {
            realised_mum = FunctGroupArray[guild].scaled_mum[cohort];
        }
        
        /*
        if(guild == bm->which_check) {
            fprintf(llogfp, "Time: %e box%d-%d %s has realised_mum: %e (%e) area_hab: %e flag_benthos_sediment_link: %d\n",
                bm->dayt, bm->current_box, bm->current_layer, FunctGroupArray[guild].groupCode, realised_mum, FunctGroupArray[guild].scaled_mum[cohort], area_hab, bm->flag_benthos_sediment_link);
         }
         */

		/* All other feeding regimes */
		Eat(bm, llogfp, predcase_sp, guild, cohort, eatBiomass, FunctGroupArray[guild].scaled_C[cohort] * hO_SP * Crwd_Effect,
            realised_mum, FunctGroupArray[guild].speciesParams[KL_id], FunctGroupArray[guild].speciesParams[KU_id],
				FunctGroupArray[guild].speciesParams[vl_id], FunctGroupArray[guild].speciesParams[ht_id], FunctGroupArray[guild].speciesParams[E1_id],
				FunctGroupArray[guild].speciesParams[E2_id], FunctGroupArray[guild].speciesParams[E3_id], FunctGroupArray[guild].speciesParams[E4_id],
				inv_feed_while_spawn, inv_spawn_now, inv_mat_pcnt, PREYinfo, GRAZEinfo, CATCHGRAZEinfo, eatBiomass);

		/* Update the prey eaten arrays so the prey biomass values can be adjusted accordingly */
		for (preyID = 0; preyID < bm->K_num_tot_sp; preyID++) {
			for (prey_chrt = 0; prey_chrt < FunctGroupArray[preyID].numCohortsXnumGenes; prey_chrt++) {
				for (hab = WC; hab <  bm->num_active_habitats; hab++) {
					if(GRAZEinfo[preyID][prey_chrt][hab] > 0){
						if (bm->flag_olddiet && (FunctGroupArray[guild].diagTol == 2))
							UpdateTrackedMort(bm, llogfp, guild, cohort, habitatType, (HABITAT_TYPES)hab, preyID, prey_chrt, boxLayerInfo, 1.0, 1);
						else
							UpdateTrackedMort(bm, llogfp, guild, cohort, habitatType, (HABITAT_TYPES)hab, preyID, prey_chrt, boxLayerInfo, 1.0, 0);
					}
				}
			}
		}
        
		/** BFS also take up DR, leading to transfer to the bay bed  **/
        FunctGroupArray[guild].transDR[cohort] = DR * FunctGroupArray[guild].CLEAR[cohort] * FunctGroupArray[guild].speciesParams[k_trans_id];
		if (FunctGroupArray[guild].diagTol < 2) {
			/** BFS also take up DR, leading to transfer to the bay bed  **/
			FunctGroupArray[guild].transDR[cohort] = DR * FunctGroupArray[guild].CLEAR[cohort]
					* FunctGroupArray[guild].speciesParams[k_trans_id];
			if(bm->track_atomic_ratio == TRUE){
				/* RF is lost in the water column */
				loss = FunctGroupArray[guild].transDR[cohort]/wcLayerThick;
				Loose_Element(bm, boxLayerInfo, WC,  RefDetIndex, 0, loss, EPIFAUNA, isGlobal);

				loss = FunctGroupArray[guild].transDR[cohort]/smLayerThick;
				Gain_Element(bm, boxLayerInfo, SED, RefDetIndex, 0,  guild, cohort, loss, EPIFAUNA, isGlobal);
			}
		} else {
			FunctGroupArray[guild].transDR[cohort] = 0.0; // TODO: Fix - should this occur?
		}

		/**
		 Natural mortality.- Benthic filter feeders suffer quadratic mortality:
		 Production of detritus, release of NH3 - Benthic feeders produce both NH3 and
		 labile detritus. There are different fractions for feeding on labile detritus.
		 **/
		Invert_Activities(bm, boxLayerInfo, habitatType, llogfp, guild, cohort, biomass, area_hab, FunctGroupArray[guild].speciesParams[E1_id], FunctGroupArray[guild].speciesParams[E3_id],
				FunctGroupArray[guild].speciesParams[E4_id], boxLayerInfo->PB_DL, boxLayerInfo->PB_DR, boxLayerInfo->BB_DL, boxLayerInfo->BB_DR, p_PBben,
				BB_scale, (1.0 - hO_SP) * FunctGroupArray[guild].speciesParams[mO_id], FunctGroupArray[guild].speciesParams[FDL_id], DL, DR, DLsed
						* boxLayerInfo->sDLscale, DRsed, GRAZEinfo);

		/* Update the detritus arrays */
		Update_Detritus(bm, boxLayerInfo, EPIFAUNA, guild, guild, cohort, biomass);

		Update_Debug_Info(bm, boxLayerInfo, EPIFAUNA, guild, cohort);

		if (bm->debug == debug_prey_biology_process && bm->dayt >= bm->checkstart && bm->dayt < bm->checkstop) {
			Print_Eat_Diagnostics(bm, logfp, guild, habitatType, 1);
			//fprintf(llogfp, "Epibenthic_Invert_Process outcome %s DetritusProd[SED][DLdet_id = %.20Le, NutsProd[SED][NH_id] = %.20Le\n", FunctGroupArray[guild].groupCode, boxLayerInfo->DetritusProd[SED][DLdet_id], boxLayerInfo->NutsProd[SED][NH_id]);
		}

	} else {
		FunctGroupArray[guild].transDR[cohort] = 0.0;
	}


	return TRUE;
}

/**
 * \brief The Function to perform the process calculations for the SED_EP_OTHER functional groups.
 *
 */
int Sediment_Epi_Other_Process(MSEBoxModel *bm, FILE *llogfp, HABITAT_TYPES habitatType, int guild, int cohort, BoxLayerValues *boxLayerInfo) {
	int predcase_sp, sp, prey, prey_chrt, preyID, hab;
	double biomass, eatBiomass, BB_scale, hO_SP, depth_scalar = 1.0, prey_avail = 0.0, area_hab, realised_mum;
	int inv_feed_while_spawn = 1;
	int inv_spawn_now = 0;
	double inv_mat_pcnt = 0.0; // As want all to spawn as included in growth for invertebrate pools
	double b = 0;

	double O2 = boxLayerInfo->localWCTracers[Oxygen_i];
	double DL = boxLayerInfo->localWCTracers[FunctGroupArray[LabDetIndex].totNTracers[0]];
	double DR = boxLayerInfo->localWCTracers[FunctGroupArray[RefDetIndex].totNTracers[0]];
	double DLsed = boxLayerInfo->localSEDTracers[FunctGroupArray[LabDetIndex].totNTracers[0]];
	double DRsed = boxLayerInfo->localSEDTracers[FunctGroupArray[RefDetIndex].totNTracers[0]];

	biomass = boxLayerInfo->localEPITracers[FunctGroupArray[guild].totNTracers[cohort]];
	predcase_sp = (int) (FunctGroupArray[guild].speciesParams[predcase_id]);

    if ((int) (FunctGroupArray[guild].speciesParams[flag_id]) && (int) (FunctGroupArray[guild].speciesParams[active_id]) &&
    		(current_depth <= (-1.0 * FunctGroupArray[guild].speciesParams[mindepth_id]) &&
    		current_depth >= (-1.0 * FunctGroupArray[guild].speciesParams[maxdepth_id]) &&
            ((-1.0 * bm->boxes[bm->current_box].botz) <= FunctGroupArray[guild].speciesParams[maxtotdepth_id]))) {
    //if ((int) (FunctGroupArray[guild].speciesParams[flag_id]) && (int) (FunctGroupArray[guild].speciesParams[active_id]) &&
    //    (current_depth <= (-1.0 * FunctGroupArray[guild].speciesParams[mindepth_id]) &&
    //     current_depth >= (-1.0 * FunctGroupArray[guild].speciesParams[maxdepth_id]))) {
            
		if (FunctGroupArray[guild].speciesParams[flag_lim_id] == simple_ben_lim)
			eatBiomass = max(small_num, biomass * (1.0 - biomass / FunctGroupArray[guild].speciesParams[max_id]));
		else
			eatBiomass = biomass;

		hO_SP = Oxygen(bm->O2case, O2, FunctGroupArray[guild].speciesParams[KO2_id], FunctGroupArray[guild].speciesParams[KO2LIM_id],
				FunctGroupArray[guild].speciesParams[mD_id]);

		Calculate_Sediment_Prey_Avail(bm, boxLayerInfo, guild, PREYinfo, &BB_scale);
		BB_scale = BB_scale * p_BBben;

		/* BG can access PS and PL in the sediment */
		if (FunctGroupArray[guild].speciesParams[KDEP_id] < O2depth) {
			/* Calculate the depth scalar - otherwise it will just be 1.0 */
			depth_scalar = (O2depth - FunctGroupArray[guild].speciesParams[KDEP_id]) / (O2depth + small_num);
		}

		for (prey = 0; prey < bm->K_num_tot_sp; prey++) {
			if(FunctGroupArray[prey].speciesParams[flag_id] == TRUE){

				if (FunctGroupArray[prey].groupType == LG_PHY || FunctGroupArray[prey].groupType == SM_PHY || FunctGroupArray[prey].groupType == DINOFLAG) {
					for (prey_chrt = 0; prey_chrt < FunctGroupArray[prey].numCohorts * FunctGroupArray[prey].numGeneTypes; prey_chrt++) {
						Avail(0, boxLayerInfo->localSEDTracers[FunctGroupArray[prey].totNTracers[prey_chrt]], FunctGroupArray[guild].speciesParams[KDEP_id], &prey_avail);

						prey_avail = prey_avail * depth_scalar;

						/* Update prey availability */
						PREYinfo[prey][prey_chrt][WC] = prey_avail;
						PREYinfo[prey][prey_chrt][SED] = 0;
					}
				}
			}
		}

		/* If scaling growth by available habitat - do so now */
		area_hab = Get_Species_Area_Hab(bm, guild, cohort, boxLayerInfo);
		realised_mum = FunctGroupArray[guild].scaled_mum[cohort];
        if(bm->flag_benthos_sediment_link) {
            realised_mum *= area_hab;
        } else {
            realised_mum = FunctGroupArray[guild].scaled_mum[cohort];
        }
       
        /*
        if(guild == bm->which_check)
            fprintf(llogfp, "Time: %e box%d-%d %s has realised_mum: %e (%e) area_hab: %e flag_benthos_sediment_link: %d\n",
                bm->dayt, bm->current_box, bm->current_layer, FunctGroupArray[guild].groupCode, realised_mum, FunctGroupArray[guild].scaled_mum[cohort], area_hab, bm->flag_benthos_sediment_link);
         */

		/* All other feeding regimes. The term (1. - BO / BOmax) is a crowding term to limit
		 the total population size that benthic deposit feeders can grow to, as they are
		 restricted to the oxygenated zone and so have a limited habitat  */
		Eat(bm, llogfp, predcase_sp, guild, cohort, eatBiomass, FunctGroupArray[guild].scaled_C[cohort] * hO_SP,
            realised_mum, FunctGroupArray[guild].speciesParams[KL_id], FunctGroupArray[guild].speciesParams[KU_id],
				FunctGroupArray[guild].speciesParams[vl_id], FunctGroupArray[guild].speciesParams[ht_id], FunctGroupArray[guild].speciesParams[E1_id],
				FunctGroupArray[guild].speciesParams[E2_id], FunctGroupArray[guild].speciesParams[E3_id], FunctGroupArray[guild].speciesParams[E4_id],
				inv_feed_while_spawn, inv_spawn_now, inv_mat_pcnt, PREYinfo, GRAZEinfo, CATCHGRAZEinfo, eatBiomass);

		for (preyID = 0; preyID < bm->K_num_tot_sp; preyID++) {
			for (prey_chrt = 0; prey_chrt < FunctGroupArray[preyID].numCohortsXnumGenes; prey_chrt++) {
				for (hab = WC; hab <  bm->num_active_habitats; hab++) {
					if(GRAZEinfo[preyID][prey_chrt][hab] > 0){
						if(bm->flag_olddiet)
							UpdateTrackedMort(bm, llogfp, guild, cohort, habitatType, (HABITAT_TYPES)hab, preyID, prey_chrt, boxLayerInfo, 1.0, 1);
						else
							UpdateTrackedMort(bm, llogfp, guild, cohort, habitatType, (HABITAT_TYPES)hab, preyID, prey_chrt, boxLayerInfo, 1.0, 0);
					}
				}
			}
		}

		/* Done this way so we get the same result as the orginal code */
		FunctGroupArray[guild].GrazeLive[cohort] += b;

		Invert_Activities(bm, boxLayerInfo, habitatType, llogfp, guild, cohort, biomass, area_hab, FunctGroupArray[guild].speciesParams[E2_id], FunctGroupArray[guild].speciesParams[E3_id],
				FunctGroupArray[guild].speciesParams[E4_id], boxLayerInfo->PB_DL, boxLayerInfo->PB_DR, boxLayerInfo->BB_DL, boxLayerInfo->BB_DR, p_PBben,
				BB_scale, (1.0 - hO_SP) * FunctGroupArray[guild].speciesParams[mO_id], FunctGroupArray[guild].speciesParams[FDL_id], DL, DR, DLsed
						* boxLayerInfo->sDLscale, DRsed, GRAZEinfo);

		Update_Detritus(bm, boxLayerInfo, EPIFAUNA, guild, guild, cohort, biomass);
		/* Diagnostic information storage */
		Update_Debug_Info(bm, boxLayerInfo, EPIFAUNA, guild, cohort);

		if (bm->debug == debug_prey_biology_process && bm->dayt >= bm->checkstart && bm->dayt < bm->checkstop) {
			Print_Eat_Diagnostics(bm, logfp, guild, habitatType, 1);
			fprintf(llogfp, "Sediment_Epi_Other_Process outcome %s DetritusProd[SED][DLdet_id = %.20Le, NutsProd[SED][NH_id] = %.20Le\n",
					FunctGroupArray[guild].groupCode, boxLayerInfo->DetritusProd[SED][DLdet_id], boxLayerInfo->NutsProd[SED][NH_id]);
		}
	}

	/* reset planktonic groups */
	for (sp = 0; sp < bm->K_num_tot_sp; sp++) {
		if(FunctGroupArray[sp].speciesParams[flag_id] == TRUE){
			if (FunctGroupArray[sp].groupType == LG_PHY || FunctGroupArray[sp].groupType == DINOFLAG || FunctGroupArray[sp].groupType == SM_PHY) {
				for (prey_chrt = 0; prey_chrt < FunctGroupArray[sp].numCohortsXnumGenes; prey_chrt++) {
					PREYinfo[sp][prey_chrt][WC] = boxLayerInfo->localWCTracers[FunctGroupArray[sp].totNTracers[prey_chrt]];
				}
			}
		}
	}

	return TRUE;
}
/**
 *  \brief The process function for the Lab det functional group.
 *
 * 	Remineralization of detritus.- only used when bacteria off.
 *
 *	Labile detritus breaks down at a fixed specific rate r_DL. A fraction FDSON_D
 *  is converted to DON, and a fraction FDR_DL to refractory detritus, DR, with
 *	the remainder released as ammonia, NH.
 *
 */
int Labile_Detritus_Process(MSEBoxModel *bm, FILE *llogfp, HABITAT_TYPES habitatType, int guild, int cohort, BoxLayerValues *boxLayerInfo) {
	double DL;
	int isGlobal = (FunctGroupArray[guild].diagTol == 2 && it_count == 1);


	if (habitatType == WC)
		DL = boxLayerInfo->localWCTracers[FunctGroupArray[guild].totNTracers[cohort]];
	else
		DL = boxLayerInfo->localSEDTracers[FunctGroupArray[guild].totNTracers[cohort]];

	if (((int) (FunctGroupArray[pelagicBactIndex].speciesParams[flag_id]) && habitatType == WC)
			|| ((int) (FunctGroupArray[SedBactIndex].speciesParams[flag_id]) && habitatType == SED)) {
		FunctGroupArray[guild].brokenDown = 0.0;
		FunctGroupArray[guild].remin = 0.0;
		FunctGroupArray[guild].solDON = 0.0;
		FunctGroupArray[guild].prodnDR[0] = 0.0;
	} else {

		FunctGroupArray[guild].brokenDown = r_DL * DL;
		FunctGroupArray[guild].remin = FunctGroupArray[guild].brokenDown * (1.0 - FDR_DL - FDON_D * (1.0 - FDR_DL));
		FunctGroupArray[guild].solDON = FunctGroupArray[guild].brokenDown * FDON_D * (1.0 - FDR_DL);
		FunctGroupArray[guild].prodnDR[0] = FunctGroupArray[guild].brokenDown * FDR_DL;

		if(bm->track_atomic_ratio){

			/* Keep track of P that is broken down */
			/* Not sure about actual value here should it be remin + solDON ?*/
			Calculate_Element_Release(bm, boxLayerInfo, guild, (FunctGroupArray[guild].remin + FunctGroupArray[guild].solDON), habitatType, WC);

			/* proportion of DL that break down to DR - keep track of P ratio in DR */
			Transfer_Element(bm, boxLayerInfo, habitatType, RefDetIndex, 0, guild, cohort, FunctGroupArray[guild].prodnDR[0], WC, isGlobal);
		}


		if (habitatType == WC) {
			boxLayerInfo->DebugInfo[guild][WC][DiagnostNH_id] = FunctGroupArray[guild].remin;
			boxLayerInfo->DebugInfo[guild][WC][DiagnostDL_id] = -FunctGroupArray[guild].brokenDown;
			boxLayerInfo->DebugInfo[guild][WC][DiagnostDR_id] = FunctGroupArray[guild].prodnDR[0];
		} else {
			boxLayerInfo->DebugInfo[guild][SED][DiagnostNHsed_id] = FunctGroupArray[guild].remin;
			boxLayerInfo->DebugInfo[guild][SED][DiagnostDLsed_id] = -FunctGroupArray[guild].brokenDown;
			boxLayerInfo->DebugInfo[guild][SED][DiagnostDRsed_id] = FunctGroupArray[guild].prodnDR[0];
		}
	}

	return TRUE;
}

/**
 *  \brief The process function for the Lab det functional group.
 *
 * Refractory detritus breaks down at a rate r_DR, and a proportion FDSON_D is
 * solubilised to DON, so:
 *	This only happens if pelagic bacteria is not turned on.
 *
 */
int Refractory_Detritus_Process(MSEBoxModel *bm, FILE *llogfp, HABITAT_TYPES habitatType, int guild, int cohort, BoxLayerValues *boxLayerInfo) {
	double DR;

	if (habitatType == WC)
		DR = boxLayerInfo->localWCTracers[FunctGroupArray[guild].totNTracers[cohort]];
	else
		DR = boxLayerInfo->localSEDTracers[FunctGroupArray[guild].totNTracers[cohort]];

	if (((int) (FunctGroupArray[pelagicBactIndex].speciesParams[flag_id]) && habitatType == WC)
			|| ((int) (FunctGroupArray[SedBactIndex].speciesParams[flag_id]) && habitatType == SED)) {
		FunctGroupArray[guild].remin = 0.0;
		FunctGroupArray[guild].solDON = 0.0;
	} else {
		FunctGroupArray[guild].remin = r_DR * DR * (1.0 - FDON_D);
		FunctGroupArray[guild].solDON = r_DR * DR * FDON_D;

		/* If we are tracking ratios keep track of amount breaking down */
		if(bm->track_atomic_ratio){
			Calculate_Element_Release(bm, boxLayerInfo, guild, r_DR * DR, habitatType, WC);
		}

		if (habitatType == WC)
			boxLayerInfo->DebugInfo[guild][habitatType][DiagnostNH_id] = FunctGroupArray[guild].remin;
		else
			boxLayerInfo->DebugInfo[guild][habitatType][DiagnostNHsed_id] = FunctGroupArray[guild].remin;
	}
	return TRUE;
}

/**
 *  \brief The process function for the Lab det functional group.
 *
 * 	Carrion is assumed to be the whole carcasses before action of bacteria.
 * 	Once the carrion has disintegrated into labile detritus (happens on the order of a day
 *	then it is available to bacteria (not exactly right, but is easier to handle and
 *	doesn't introduce a great lag). Here carrion break down from big chunks (carcasses)
 *	to labile and refractory detritus.
 *
 */
int Carrion_Process(MSEBoxModel *bm, FILE *llogfp, HABITAT_TYPES habitatType, int guild, int cohort, BoxLayerValues *boxLayerInfo) {
	double DC;


	if (habitatType == WC)
		DC = boxLayerInfo->localWCTracers[FunctGroupArray[guild].totNTracers[cohort]];
	else
		DC = boxLayerInfo->localSEDTracers[FunctGroupArray[guild].totNTracers[cohort]];

	if (it_count == 1) {
		FunctGroupArray[guild].brokenDown = r_DC * DC;
		FunctGroupArray[guild].prodnDR[0] = FunctGroupArray[guild].brokenDown * FDR_DC;
		FunctGroupArray[guild].prodnDL[0]  = FunctGroupArray[guild].brokenDown * (1.0 - FDR_DC);

		boxLayerInfo->DetritusProd[habitatType][DRdet_id] += FunctGroupArray[guild].prodnDR[0] ;
		boxLayerInfo->DetritusProd[habitatType][DLdet_id] += FunctGroupArray[guild].prodnDL[0] ;

		if(bm->track_atomic_ratio == TRUE){
			Transfer_Element(bm, boxLayerInfo, habitatType, RefDetIndex, 0, guild, cohort, FunctGroupArray[guild].prodnDR[0], habitatType, TRUE);
			Transfer_Element(bm, boxLayerInfo, habitatType, LabDetIndex, 0, guild, cohort, FunctGroupArray[guild].prodnDL[0], habitatType, TRUE);
		}

		/* Contaminants transfer */
		if(bm->track_contaminants){
			/* Gains in DL and DR due to mortality */
            //fprintf(bm->logFile,"Calling Group_Transfer_Contaminant from Update_Detritus WC WC - %s-%d with DC: %e prodDL: %e prodDR: %e\n", FunctGroupArray[guild].groupCode, cohort, DC, FunctGroupArray[guild].prodnDL[0], FunctGroupArray[guild].prodnDR[0]);
            
            Group_Transfer_Contaminant(bm, boxLayerInfo, habitatType, habitatType, LabDetIndex, 0, guild, cohort, FunctGroupArray[guild].prodnDL[0], 0, DC, bm->dtsz_stored, 1, 29);    // Was case 0 for need_prop
			Group_Transfer_Contaminant(bm, boxLayerInfo, habitatType, habitatType, RefDetIndex, 0, guild, cohort, FunctGroupArray[guild].prodnDR[0], 0, DC, bm->dtsz_stored, 1, 30);    // Was case 0 for need_prop
		}

		/* Update global fluxes */
		boxLayerInfo->DetritusProdGlobal[habitatType][habitatType][DRdet_id] += FunctGroupArray[guild].prodnDR[0] ;
		boxLayerInfo->DetritusProdGlobal[habitatType][habitatType][DLdet_id] += FunctGroupArray[guild].prodnDL[0] ;

		if (habitatType == WC) {

			/* Diagnostic information storage */
			boxLayerInfo->DebugInfo[guild][WC][DiagnostDL_id] = FunctGroupArray[guild].prodnDL[0] ;
			boxLayerInfo->DebugInfo[guild][WC][DiagnostDR_id] = FunctGroupArray[guild].prodnDR[0] ;
		} else {

			/* Diagnostic information storage */
			boxLayerInfo->DebugInfo[guild][SED][DiagnostDRsed_id] = FunctGroupArray[CarrionIndex].prodnDR[0] ;
			boxLayerInfo->DebugInfo[guild][SED][DiagnostDLsed_id] = FunctGroupArray[CarrionIndex].prodnDL[0] ;

		}

	}
	return TRUE;
}
