/**
 *	\file This file contains rate of change functions for nutrients.
 *
 *
 *	Revision History:
 *
 *  21-04-2009 Bec Gorton
 *  Added the Refractory_Detritus_ROC function to calculate the DR rate of change.
 *  Added the rest of the nutrient flux calculations.
 *
 *  05-07-2012 Bec Gorton
 *	Wrapped the broker linkage code in a define to simply the build process for those who don't need to link.
 *
 * 17-04-2013 Bec Gorton
 * ATL-346 - Added support of tracking P:N:C ratios in groups.
 *
 *	@author Bec Gorton 20/April/2009
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <math.h>
#include <string.h>
#include <sjwlib.h>
#include "atecology.h"


/**
 * \brief Calculate the Refractory Detritus flux.
 *
 *
 */
void Refractory_Detritus_ROC(MSEBoxModel *bm, FILE *llogfp, HABITAT_TYPES habitatType, BoxLayerValues *boxLayerInfo) {

	int guild, cohort;
	double gain, loss;

	switch (habitatType) {
	case WC:
		gain = 0.0;
		loss = 0.0;

		for (guild = 0; guild < bm->K_num_tot_sp; guild++) {
			if (FunctGroupArray[guild].groupType == LAB_DET || FunctGroupArray[guild].groupType == REF_DET || FunctGroupArray[guild].groupType == PL_BACT) {
				gain += FunctGroupArray[guild].prodnDR[0] ;
				loss += FunctGroupArray[guild].remin + FunctGroupArray[guild].solDON + FunctGroupArray[guild].uptakeDR;
			}
		}

		boxLayerInfo->localWCFlux[FunctGroupArray[RefDetIndex].totNTracers[0]] = gain - loss + (double)boxLayerInfo->DetritusProd[WC][DRdet_id]
				- (double)boxLayerInfo->DetritusLost[WC][DRdet_id];

		boxLayerInfo->DebugInfo[bm->K_num_tot_sp][WC][DiagnostDR_id] = gain - loss;
		boxLayerInfo->DebugInfo[bm->K_num_tot_sp + 1][WC][DiagnostDR_id] = boxLayerInfo->localWCFlux[FunctGroupArray[RefDetIndex].totNTracers[0]];

		boxLayerInfo->DebugFluxInfo[RefDetIndex][WC][gain_id] = gain + (double)boxLayerInfo->DetritusProd[WC][DRdet_id];
		boxLayerInfo->DebugFluxInfo[RefDetIndex][WC][loss_id] = loss + (double)boxLayerInfo->DetritusLost[WC][DRdet_id];

#ifdef BROKER_LINK_ENABLED
		if(do_BrokerLinkage){
			boxLayerInfo->localWCFlux[FunctGroupArray[RefDetIndex].totNTracers[0]] +=
				bm->linkageInterface->linkageWCDetritusFlux[bm->current_box][bm->current_layer][RefDetIndex];

			boxLayerInfo->DebugFluxInfo[RefDetIndex][WC][gain_id]
				+= bm->linkageInterface->linkageWCDetritusFlux[bm->current_box][bm->current_layer][RefDetIndex];
		}
#endif

		break;
	case SED:
	case LAND_BASED:
		gain = 0.0;
		loss = 0.0;
		for (guild = 0; guild < bm->K_num_tot_sp; guild++) {
			if (FunctGroupArray[guild].groupType == LAB_DET || FunctGroupArray[guild].groupType == REF_DET || FunctGroupArray[guild].groupType
					== SED_BACT) {
				gain += FunctGroupArray[guild].prodnDR[0] ;
				loss += FunctGroupArray[guild].remin + FunctGroupArray[guild].solDON + FunctGroupArray[guild].uptakeDR;
			}
		}

		boxLayerInfo->localSEDFlux[FunctGroupArray[RefDetIndex].totNTracers[0]] = gain - loss + (double)boxLayerInfo->DetritusProd[SED][DRdet_id]
				- (double)boxLayerInfo->DetritusLost[SED][DRdet_id];

		boxLayerInfo->DebugInfo[bm->K_num_tot_sp][SED][DiagnostDRsed_id] = gain - loss;
		boxLayerInfo->DebugInfo[bm->K_num_tot_sp + 1][SED][DiagnostDRsed_id] = boxLayerInfo->localSEDFlux[FunctGroupArray[RefDetIndex].totNTracers[0]];

		boxLayerInfo->DebugFluxInfo[RefDetIndex][SED][gain_id] = gain + (double)boxLayerInfo->DetritusProd[SED][DRdet_id];

		boxLayerInfo->DebugFluxInfo[RefDetIndex][SED][loss_id] = loss + (double)boxLayerInfo->DetritusLost[SED][DRdet_id];

#ifdef BROKER_LINK_ENABLED
		if(do_BrokerLinkage){
			boxLayerInfo->localSEDFlux[FunctGroupArray[RefDetIndex].totNTracers[0]] +=
				bm->linkageInterface->linkageSEDDetritusFlux[bm->current_box][bm->current_layer][RefDetIndex];
			boxLayerInfo->DebugFluxInfo[RefDetIndex][SED][gain_id] += bm->linkageInterface->linkageSEDDetritusFlux[bm->current_box][bm->current_layer][RefDetIndex];
		}
#endif
		break;
	case EPIFAUNA:

		loss = 0.0;
		for (guild = 0; guild < bm->K_num_tot_sp; guild++) {
			if (FunctGroupArray[guild].isVertebrate == FALSE) {
				for(cohort = 0; cohort < FunctGroupArray[guild].numCohortsXnumGenes; cohort++)
					loss += FunctGroupArray[guild].transDR[cohort];
			}
		}

		boxLayerInfo->localWCFlux[FunctGroupArray[RefDetIndex].totNTracers[0]] += (double)boxLayerInfo->DetritusProd[WC][DRdet_id]
				- (double)boxLayerInfo->DetritusLost[WC][DRdet_id] - loss / wcLayerThick;
		boxLayerInfo->DebugInfo[bm->K_num_tot_sp][EPIFAUNA][DiagnostDR_id] = -loss / wcLayerThick;
		boxLayerInfo->DebugInfo[bm->K_num_tot_sp + 1][EPIFAUNA][DiagnostDR_id] = boxLayerInfo->localWCFlux[FunctGroupArray[RefDetIndex].totNTracers[0]];

		boxLayerInfo->DebugFluxInfo[RefDetIndex][WC][gain_id] = (double)boxLayerInfo->DetritusProd[WC][DRdet_id];
		boxLayerInfo->DebugFluxInfo[RefDetIndex][WC][loss_id] = (double)boxLayerInfo->DetritusLost[WC][DRdet_id] + loss / wcLayerThick;

		/**
		 Refractory Detritus in the sediment. Material from discarded benthic invertebrate and primary
		 producer bycatch is deposited in refractory detritus as little available biomass involved.
		 **/
		boxLayerInfo->localSEDFlux[FunctGroupArray[RefDetIndex].totNTracers[0]] += loss / smLayerThick + (double)boxLayerInfo->DetritusProd[SED][DRdet_id]
				- (double)boxLayerInfo->DetritusLost[SED][DRdet_id];

		boxLayerInfo->DebugInfo[bm->K_num_tot_sp][EPIFAUNA][DiagnostDRsed_id] = loss / smLayerThick;
		boxLayerInfo->DebugInfo[bm->K_num_tot_sp + 1][EPIFAUNA][DiagnostDRsed_id] = boxLayerInfo->localSEDFlux[FunctGroupArray[RefDetIndex].totNTracers[0]];

		boxLayerInfo->DebugFluxInfo[RefDetIndex][SED][gain_id] = (double)boxLayerInfo->DetritusProd[SED][DRdet_id] + loss / smLayerThick;
		boxLayerInfo->DebugFluxInfo[RefDetIndex][SED][loss_id] = (double)boxLayerInfo->DetritusLost[SED][DRdet_id];

		break;
	case ICE_BASED:
		gain = 0.0;
		loss = 0.0;

		for (guild = 0; guild < bm->K_num_tot_sp; guild++) {
			if (FunctGroupArray[guild].groupType == LAB_DET || FunctGroupArray[guild].groupType == REF_DET || FunctGroupArray[guild].groupType == ICE_BACT) {
				gain += FunctGroupArray[guild].prodnDR[0] ;
				loss += FunctGroupArray[guild].remin + FunctGroupArray[guild].solDON + FunctGroupArray[guild].uptakeDR;
			}
		}

		boxLayerInfo->localICEFlux[FunctGroupArray[RefDetIndex].totNTracers[0]] = gain - loss + (double)boxLayerInfo->DetritusProd[ICE_BASED][DRdet_id]
				- (double)boxLayerInfo->DetritusLost[ICE_BASED][DRdet_id];

		boxLayerInfo->DebugInfo[bm->K_num_tot_sp][ICE_BASED][DiagnostDR_id] = gain - loss;
		boxLayerInfo->DebugInfo[bm->K_num_tot_sp + 1][ICE_BASED][DiagnostDR_id] = boxLayerInfo->localICEFlux[FunctGroupArray[RefDetIndex].totNTracers[0]];

		boxLayerInfo->DebugFluxInfo[RefDetIndex][ICE_BASED][gain_id] = gain + (double)boxLayerInfo->DetritusProd[ICE_BASED][DRdet_id];
		boxLayerInfo->DebugFluxInfo[RefDetIndex][ICE_BASED][loss_id] = loss + (double)boxLayerInfo->DetritusLost[ICE_BASED][DRdet_id];

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
 * \brief Calculate the Labile Detritus flux.
 *
 *
 */
void Labile_Detritus_ROC(MSEBoxModel *bm, FILE *llogfp, HABITAT_TYPES habitatType, BoxLayerValues *boxLayerInfo) {

	int guild;
	double loss;

	switch (habitatType) {
	case WC:
		loss = FunctGroupArray[LabDetIndex].brokenDown + FunctGroupArray[pelagicBactIndex].uptakeDL;

		boxLayerInfo->localWCFlux[FunctGroupArray[LabDetIndex].totNTracers[0]] = (double)boxLayerInfo->DetritusProd[WC][DLdet_id]
				- (double)boxLayerInfo->DetritusLost[WC][DLdet_id] - FunctGroupArray[LabDetIndex].brokenDown - FunctGroupArray[pelagicBactIndex].uptakeDL;

		/* The remin value */
		boxLayerInfo->DebugInfo[bm->K_num_tot_sp][WC][DiagnostDL_id] = -loss;

		/* The DL flux */
		boxLayerInfo->DebugInfo[bm->K_num_tot_sp + 1][WC][DiagnostDL_id] = boxLayerInfo->localWCFlux[FunctGroupArray[LabDetIndex].totNTracers[0]];

		boxLayerInfo->DebugFluxInfo[LabDetIndex][WC][gain_id] = (double)boxLayerInfo->DetritusProd[WC][DLdet_id];
		boxLayerInfo->DebugFluxInfo[LabDetIndex][WC][loss_id] = (double)boxLayerInfo->DetritusLost[WC][DLdet_id] + loss;

		/*
		fprintf(llogfp,"Time: %e box%d-%d DL ROC flux: %e prod: %Le, lost: %Le, brokenDown: %e, uptakeDL: %e\n",
				bm->dayt, bm->current_box, bm->current_layer, boxLayerInfo->localWCFlux[FunctGroupArray[LabDetIndex].totNTracers[0]],
				boxLayerInfo->DetritusProd[WC][DLdet_id],  boxLayerInfo->DetritusLost[WC][DLdet_id],
				FunctGroupArray[LabDetIndex].brokenDown, FunctGroupArray[pelagicBactIndex].uptakeDL);
		*/

#ifdef BROKER_LINK_ENABLED
		if(do_BrokerLinkage){
			boxLayerInfo->localWCFlux[FunctGroupArray[LabDetIndex].totNTracers[0]]
				+= bm->linkageInterface->linkageWCDetritusFlux[bm->current_box][bm->current_layer][LabDetIndex];
			boxLayerInfo->DebugFluxInfo[LabDetIndex][WC][gain_id]
				+= bm->linkageInterface->linkageWCDetritusFlux[bm->current_box][bm->current_layer][LabDetIndex];
		}
#endif

		break;
	case SED:
	case LAND_BASED:
		loss = 0.0;
		for (guild = 0; guild < bm->K_num_tot_sp; guild++) {
			if (FunctGroupArray[guild].groupType == LAB_DET || FunctGroupArray[guild].groupType == SED_BACT) {
				loss += FunctGroupArray[guild].brokenDown + FunctGroupArray[guild].uptakeDL;
			}
		}
		boxLayerInfo->localSEDFlux[FunctGroupArray[LabDetIndex].totNTracers[0]] = (double)boxLayerInfo->DetritusProd[SED][DLdet_id]
				- (double)boxLayerInfo->DetritusLost[SED][DLdet_id] - loss;

		boxLayerInfo->DebugInfo[bm->K_num_tot_sp][SED][DiagnostDLsed_id] = -loss;

		boxLayerInfo->DebugInfo[bm->K_num_tot_sp + 1][SED][DiagnostDLsed_id] = boxLayerInfo->localSEDFlux[FunctGroupArray[LabDetIndex].totNTracers[0]];

		boxLayerInfo->DebugFluxInfo[LabDetIndex][SED][gain_id] = (double)boxLayerInfo->DetritusProd[SED][DLdet_id];
		boxLayerInfo->DebugFluxInfo[LabDetIndex][SED][loss_id] = (double)boxLayerInfo->DetritusLost[SED][DLdet_id] + loss;

#ifdef BROKER_LINK_ENABLED
		if(do_BrokerLinkage){
			boxLayerInfo->localSEDFlux[FunctGroupArray[LabDetIndex].totNTracers[0]] +=
					bm->linkageInterface->linkageSEDDetritusFlux[bm->current_box][bm->current_layer][LabDetIndex];
			boxLayerInfo->DebugFluxInfo[LabDetIndex][SED][gain_id] += bm->linkageInterface->linkageSEDDetritusFlux[bm->current_box][bm->current_layer][LabDetIndex];
		}
#endif
		break;
	case EPIFAUNA:
		/**
		 Labile detritus in the water column
		 **/
		boxLayerInfo->localWCFlux[FunctGroupArray[LabDetIndex].totNTracers[0]] += (double)boxLayerInfo->DetritusProd[WC][DLdet_id]
				- (double)boxLayerInfo->DetritusLost[WC][DLdet_id];

		boxLayerInfo->DebugInfo[bm->K_num_tot_sp + 1][EPIFAUNA][DiagnostDL_id] = boxLayerInfo->localWCFlux[FunctGroupArray[LabDetIndex].totNTracers[0]];

		boxLayerInfo->DebugFluxInfo[LabDetIndex][WC][gain_id] = (double)boxLayerInfo->DetritusProd[WC][DLdet_id];
		boxLayerInfo->DebugFluxInfo[LabDetIndex][WC][loss_id] = (double)boxLayerInfo->DetritusLost[WC][DLdet_id];

		/**
		 Labile detritus in sediment.
		 **/
		boxLayerInfo->localSEDFlux[FunctGroupArray[LabDetIndex].totNTracers[0]] += (double)boxLayerInfo->DetritusProd[SED][DLdet_id]
				- (double)boxLayerInfo->DetritusLost[SED][DLdet_id];

		boxLayerInfo->DebugInfo[bm->K_num_tot_sp + 1][EPIFAUNA][DiagnostDLsed_id] = boxLayerInfo->localSEDFlux[FunctGroupArray[LabDetIndex].totNTracers[0]];

		boxLayerInfo->DebugFluxInfo[LabDetIndex][SED][gain_id] = (double)boxLayerInfo->DetritusProd[SED][DLdet_id];
		boxLayerInfo->DebugFluxInfo[LabDetIndex][SED][loss_id] = (double)boxLayerInfo->DetritusLost[SED][DLdet_id];

		break;
	case ICE_BASED:
		loss = FunctGroupArray[LabDetIndex].brokenDown + FunctGroupArray[IceBactIndex].uptakeDL;

		boxLayerInfo->localICEFlux[FunctGroupArray[LabDetIndex].totNTracers[0]] = (double)boxLayerInfo->DetritusProd[ICE_BASED][DLdet_id]
				- (double)boxLayerInfo->DetritusLost[ICE_BASED][DLdet_id] - FunctGroupArray[LabDetIndex].brokenDown - FunctGroupArray[IceBactIndex].uptakeDL;

		/* The remin value */
		boxLayerInfo->DebugInfo[bm->K_num_tot_sp][ICE_BASED][DiagnostDL_id] = -loss;

		/* The DL flux */
		boxLayerInfo->DebugInfo[bm->K_num_tot_sp + 1][ICE_BASED][DiagnostDL_id] = boxLayerInfo->localICEFlux[FunctGroupArray[LabDetIndex].totNTracers[0]];

		boxLayerInfo->DebugFluxInfo[LabDetIndex][ICE_BASED][gain_id] = (double)boxLayerInfo->DetritusProd[ICE_BASED][DLdet_id];
		boxLayerInfo->DebugFluxInfo[LabDetIndex][ICE_BASED][loss_id] = (double)boxLayerInfo->DetritusLost[ICE_BASED][DLdet_id] + loss;

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
 * \brief Calculate the Carrion flux.
 *
 *
 */
void Carrion_ROC(MSEBoxModel *bm, FILE *llogfp, HABITAT_TYPES habitatType, BoxLayerValues *boxLayerInfo) {

	switch (habitatType) {
	case WC:
		boxLayerInfo->localWCFlux[FunctGroupArray[CarrionIndex].totNTracers[0]] = (double)boxLayerInfo->DetritusProd[WC][DCdet_id]
				- FunctGroupArray[CarrionIndex].brokenDown - (double)FunctGroupArray[CarrionIndex].preyEaten[0][WC];

		//fprintf(bm->logFile, "boxLayerInfo->localWCFlux[FunctGroupArray[CarrionIndex].totNTracers[0]] = %e\n",boxLayerInfo->localWCFlux[FunctGroupArray[CarrionIndex].totNTracers[0]]);

		boxLayerInfo->DebugFluxInfo[CarrionIndex][WC][gain_id] = (double)boxLayerInfo->DetritusProd[WC][DCdet_id];
		boxLayerInfo->DebugFluxInfo[CarrionIndex][WC][loss_id] = FunctGroupArray[CarrionIndex].brokenDown
				+ (double)FunctGroupArray[CarrionIndex].preyEaten[0][WC];

#ifdef BROKER_LINK_ENABLED
		if(do_BrokerLinkage){
			boxLayerInfo->localWCFlux[FunctGroupArray[CarrionIndex].totNTracers[0]] += bm->linkageInterface->linkageWCDetritusFlux[bm->current_box][bm->current_layer][CarrionIndex];
			boxLayerInfo->DebugFluxInfo[CarrionIndex][WC][gain_id] += bm->linkageInterface->linkageWCDetritusFlux[bm->current_box][bm->current_layer][CarrionIndex];
		}
#endif

		break;
	case SED:
	case LAND_BASED:
		boxLayerInfo->localSEDFlux[FunctGroupArray[CarrionIndex].totNTracers[0]] = -FunctGroupArray[CarrionIndex].brokenDown
				- (double)FunctGroupArray[CarrionIndex].preyEaten[0][SED];
		boxLayerInfo->DebugFluxInfo[CarrionIndex][SED][loss_id] = FunctGroupArray[CarrionIndex].brokenDown
				+ (double)FunctGroupArray[CarrionIndex].preyEaten[0][SED];

#ifdef BROKER_LINK_ENABLED
		if(do_BrokerLinkage){
			boxLayerInfo->localSEDFlux[FunctGroupArray[CarrionIndex].totNTracers[0]] +=
					bm->linkageInterface->linkageSEDDetritusFlux[bm->current_box][bm->current_layer][CarrionIndex];
		}
#endif
		break;
	case EPIFAUNA:
		/**
		 Carrion in the water column (settling from above usually)
		 **/

		boxLayerInfo->localWCFlux[FunctGroupArray[CarrionIndex].totNTracers[0]] += -(double)FunctGroupArray[CarrionIndex].preyEaten[0][WC]
				+ (double)boxLayerInfo->DetritusProd[WC][DCdet_id] - (double)boxLayerInfo->DetritusLost[WC][DCdet_id];

		boxLayerInfo->DebugFluxInfo[CarrionIndex][WC][gain_id] = (double)boxLayerInfo->DetritusProd[WC][DCdet_id];
		boxLayerInfo->DebugFluxInfo[CarrionIndex][WC][loss_id] = (double)FunctGroupArray[CarrionIndex].preyEaten[0][WC]
				+ (double)boxLayerInfo->DetritusLost[WC][DCdet_id];

		/**
		 Carrion in the sediments carcasses of discarded vertebrate and cephalopod bycatch is
		 deposited here as sink quickly.
		 **/
		boxLayerInfo->localSEDFlux[FunctGroupArray[CarrionIndex].totNTracers[0]] += -(double)FunctGroupArray[CarrionIndex].preyEaten[0][SED]
				+ (double)boxLayerInfo->DetritusProd[SED][DCdet_id] - (double)boxLayerInfo->DetritusLost[SED][DCdet_id];
		boxLayerInfo->DebugFluxInfo[CarrionIndex][SED][gain_id] = (double)boxLayerInfo->DetritusProd[SED][DCdet_id];
		boxLayerInfo->DebugFluxInfo[CarrionIndex][SED][loss_id] = (double)boxLayerInfo->DetritusLost[SED][DCdet_id]
				+ (double)FunctGroupArray[CarrionIndex].preyEaten[0][SED];

		break;
	case ICE_BASED:
		boxLayerInfo->localICEFlux[FunctGroupArray[CarrionIndex].totNTracers[0]] = (double)boxLayerInfo->DetritusProd[ICE_BASED][DCdet_id]
				- FunctGroupArray[CarrionIndex].brokenDown - (double)FunctGroupArray[CarrionIndex].preyEaten[0][ICE_BASED];

		boxLayerInfo->DebugFluxInfo[CarrionIndex][ICE_BASED][gain_id] = (double)boxLayerInfo->DetritusProd[ICE_BASED][DCdet_id];
		boxLayerInfo->DebugFluxInfo[CarrionIndex][ICE_BASED][loss_id] = FunctGroupArray[CarrionIndex].brokenDown
				+ (double)FunctGroupArray[CarrionIndex].preyEaten[0][ICE_BASED];

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
 *  \brief The Ammonium tracer flux calculation function.
 *  	NH3
 *
 */

void Ammonium_ROC(MSEBoxModel *bm, FILE *llogfp, HABITAT_TYPES habitatType, int index, BoxLayerValues *boxLayerInfo) {

	double gain, loss;
    double dzz, numsec = 86400.0;

	switch (habitatType) {
	case WC:
		gain = boxLayerInfo->DONremin + FunctGroupArray[LabDetIndex].remin + FunctGroupArray[RefDetIndex].remin
				+ FunctGroupArray[pelagicBactIndex].releaseNH[0];
		loss = FunctGroupArray[pelagicBactIndex].nitrif;

		boxLayerInfo->localWCFlux[*PhysioChemArray[index].tracerIndex] = gain - loss + (double)boxLayerInfo->NutsProd[WC][NH_id]
				- (double)boxLayerInfo->NutsLost[WC][NH_id];
            
		/* Check the values are finite */
		if(!_finite(boxLayerInfo->localWCFlux[*PhysioChemArray[index].tracerIndex])){
			printf("boxLayerInfo->DONremin = %e\n", boxLayerInfo->DONremin);
			printf("FunctGroupArray[LabDetIndex].remin = %e\n", FunctGroupArray[LabDetIndex].remin);
			printf("FunctGroupArray[RefDetIndex].remin = %e\n", FunctGroupArray[RefDetIndex].remin);
			printf("FunctGroupArray[pelagicBactIndex].releaseNH[0] = %e\n", FunctGroupArray[pelagicBactIndex].releaseNH[0]);
			printf("boxLayerInfo->NutsProd[WC][NH_id] = %Le\n",boxLayerInfo->NutsProd[WC][NH_id]);
			printf("boxLayerInfo->NutsLost[WC][NH_id] = %Le\n", boxLayerInfo->NutsLost[WC][NH_id]);
			quit("ERROR: NH3 flux is infinite\n");

		}
		boxLayerInfo->DebugInfo[bm->K_num_tot_sp][WC][DiagnostNH_id] = gain - loss;
		boxLayerInfo->DebugInfo[bm->K_num_tot_sp + 1][WC][DiagnostNH_id] = boxLayerInfo->localWCFlux[NH3_i];

		boxLayerInfo->DebugFluxInfo[PhysioChemArray[index].debugIndex][WC][gain_id] = gain + (double)boxLayerInfo->NutsProd[WC][NH_id];
		boxLayerInfo->DebugFluxInfo[PhysioChemArray[index].debugIndex][WC][loss_id] = loss + (double)boxLayerInfo->NutsLost[WC][NH_id];

        /*
        if(bm->current_box == bm->checkbox){
            fprintf(bm->logFile, "Time: %e box %d-%d NH flux %e boxLayerInfo->DONremin = %e ", bm->dayt, bm->current_box, bm->current_layer,
                    boxLayerInfo->localWCFlux[*PhysioChemArray[index].tracerIndex], boxLayerInfo->DONremin);
            fprintf(bm->logFile, "FunctGroupArray[LabDetIndex].reminc = %e ", FunctGroupArray[LabDetIndex].remin);
            fprintf(bm->logFile, "FunctGroupArray[RefDetIndex].remin = %e ", FunctGroupArray[RefDetIndex].remin);
            fprintf(bm->logFile, "FunctGroupArray[pelagicBactIndex].releaseNH[0] = %e ", FunctGroupArray[pelagicBactIndex].releaseNH[0]);
            fprintf(bm->logFile, "FunctGroupArray[pelagicBactIndex].nitrif = %e ", FunctGroupArray[pelagicBactIndex].nitrif);
            fprintf(bm->logFile, "boxLayerInfo->NutsProd[WC][NH_id] = %Le ",boxLayerInfo->NutsProd[WC][NH_id]);
            fprintf(bm->logFile, "boxLayerInfo->NutsLost[WC][NH_id] = %Le\n", boxLayerInfo->NutsLost[WC][NH_id]);
                
        }
        */
            
		break;
	case SED:
	case LAND_BASED:
		/**
		 Ammonium in sediment
		 **/

		boxLayerInfo->localSEDFlux[*PhysioChemArray[index].tracerIndex] = boxLayerInfo->Remin - boxLayerInfo->Nitrification
				+ FunctGroupArray[SedBactIndex].releaseNH[0] +(double) boxLayerInfo->NutsProd[SED][NH_id] -
				(double)boxLayerInfo->NutsLost[SED][NH_id];

		boxLayerInfo->DebugInfo[bm->K_num_tot_sp][SED][DiagnostNHsed_id] = boxLayerInfo->Remin - boxLayerInfo->Nitrification
				+ FunctGroupArray[SedBactIndex].releaseNH[0];
		boxLayerInfo->DebugInfo[bm->K_num_tot_sp + 1][SED][DiagnostNHsed_id] = boxLayerInfo->localSEDFlux[NH3_i];

		boxLayerInfo->DebugFluxInfo[PhysioChemArray[index].debugIndex][SED][gain_id] = boxLayerInfo->Remin
				+ FunctGroupArray[SedBactIndex].releaseNH[0] + (double)boxLayerInfo->NutsProd[SED][NH_id];
		boxLayerInfo->DebugFluxInfo[PhysioChemArray[index].debugIndex][SED][loss_id] = boxLayerInfo->Nitrification + (double)boxLayerInfo->NutsLost[SED][NH_id];

        /*
        if(bm->current_box == bm->checkbox){
            fprintf(bm->logFile, "Time: %e box %d-%d SED NH flux %e boxLayerInfo->Remin = %e ", bm->dayt, bm->current_box, bm->current_layer,
                    boxLayerInfo->localSEDFlux[*PhysioChemArray[index].tracerIndex], boxLayerInfo->Remin);
            fprintf(bm->logFile, "FunctGroupArray[SedBactIndex].releaseNH[0] = %e ", FunctGroupArray[SedBactIndex].releaseNH[0]);
            fprintf(bm->logFile, "boxLayerInfo->Nitrification = %e ", boxLayerInfo->Nitrification);
            fprintf(bm->logFile, "boxLayerInfo->NutsProd[SED][NH_id] = %Le ",boxLayerInfo->NutsProd[SED][NH_id]);
            fprintf(bm->logFile, "boxLayerInfo->NutsLost[SED][NH_id] = %Le\n", boxLayerInfo->NutsLost[SED][NH_id]);
                
        }
        */
		break;
	case EPIFAUNA:
		/* Water Column*/

		boxLayerInfo->localWCFlux[*PhysioChemArray[index].tracerIndex] += (double)boxLayerInfo->NutsProd[WC][NH_id] - (double)boxLayerInfo->NutsLost[WC][NH_id];

		boxLayerInfo->DebugInfo[bm->K_num_tot_sp + 1][EPIFAUNA][DiagnostNH_id] = boxLayerInfo->localWCFlux[NH3_i];

		boxLayerInfo->DebugFluxInfo[PhysioChemArray[index].debugIndex][WC][gain_id] = (double)boxLayerInfo->NutsProd[WC][NH_id];
		boxLayerInfo->DebugFluxInfo[PhysioChemArray[index].debugIndex][WC][loss_id] = (double)boxLayerInfo->NutsLost[WC][NH_id];

		/* Sediment */
		boxLayerInfo->localSEDFlux[NH3_i] += (double)boxLayerInfo->NutsProd[SED][NH_id] - (double)boxLayerInfo->NutsLost[SED][NH_id];

		boxLayerInfo->DebugInfo[bm->K_num_tot_sp + 1][EPIFAUNA][DiagnostNHsed_id] = boxLayerInfo->localSEDFlux[NH3_i];

		boxLayerInfo->DebugFluxInfo[PhysioChemArray[index].debugIndex][SED][gain_id] = (double)boxLayerInfo->NutsProd[SED][NH_id];
		boxLayerInfo->DebugFluxInfo[PhysioChemArray[index].debugIndex][SED][loss_id] = (double)boxLayerInfo->NutsLost[SED][NH_id];

        /*
        if(bm->current_box == bm->checkbox){
            fprintf(bm->logFile, "Time: %e box %d-%d EPI wcNH flux %e sedNH flux %e ", bm->dayt, bm->current_box, bm->current_layer,
                    boxLayerInfo->localWCFlux[*PhysioChemArray[index].tracerIndex], boxLayerInfo->localSEDFlux[NH3_i] );
            fprintf(bm->logFile, "boxLayerInfo->NutsProd[WC][NH_id] = %Le ",boxLayerInfo->NutsProd[WC][NH_id]);
            fprintf(bm->logFile, "boxLayerInfo->NutsLost[WC][NH_id] = %Le ", boxLayerInfo->NutsLost[WC][NH_id]);
            fprintf(bm->logFile, "boxLayerInfo->NutsProd[SED][NH_id] = %Le ",boxLayerInfo->NutsProd[SED][NH_id]);
            fprintf(bm->logFile, "boxLayerInfo->NutsLost[SED][NH_id] = %Le\n", boxLayerInfo->NutsLost[SED][NH_id]);
                
        }
        */
		break;
	case ICE_BASED:
		gain = boxLayerInfo->DONremin + FunctGroupArray[LabDetIndex].remin + FunctGroupArray[RefDetIndex].remin
				+ FunctGroupArray[IceBactIndex].releaseNH[0];
		loss = FunctGroupArray[IceBactIndex].nitrif;

		boxLayerInfo->localICEFlux[*PhysioChemArray[index].tracerIndex] = gain - loss + (double)boxLayerInfo->NutsProd[ICE_BASED][NH_id]
				- (double)boxLayerInfo->NutsLost[ICE_BASED][NH_id];

		boxLayerInfo->DebugInfo[bm->K_num_tot_sp][ICE_BASED][DiagnostNH_id] = gain - loss;
		boxLayerInfo->DebugInfo[bm->K_num_tot_sp + 1][ICE_BASED][DiagnostNH_id] = boxLayerInfo->localICEFlux[NH3_i];

		boxLayerInfo->DebugFluxInfo[PhysioChemArray[index].debugIndex][ICE_BASED][gain_id] = gain + (double)boxLayerInfo->NutsProd[ICE_BASED][NH_id];
		boxLayerInfo->DebugFluxInfo[PhysioChemArray[index].debugIndex][ICE_BASED][loss_id] = loss + (double)boxLayerInfo->NutsLost[ICE_BASED][NH_id];

		break;
    case MIXED:
        quit("How did we get here as should come through a primary habitat\n");
        break;
    default:
        quit("How did we get here as this isn't even a habitat type slot but nTotHabTypes\n");
        break;
	}
    
	if(bm->include_atmosphere){
		/** Get NH3 values from atmosphare on the surface cell **/
		if (bm->current_layer == (bm->boxes[bm->current_box].nz - 1)) {
			dzz = bm->boxes[bm->current_box].dz[bm->current_layer];
			if (dzz != 0.0) {
			   boxLayerInfo->localWCFlux[*PhysioChemArray[index].tracerIndex] += (bm->atmospheric_NH / dzz) / numsec;
			   boxLayerInfo->DebugFluxInfo[PhysioChemArray[index].debugIndex][WC][gain_id] += (bm->atmospheric_NH / dzz) / numsec;
        
                if (bm->include_outgas) {
                    if (boxLayerInfo->localWCTracers[NH3_i] > (bm->outgas_prop * 12500.0)) { // Where 12500.0 is the saturation point
                        boxLayerInfo->localWCFlux[*PhysioChemArray[index].tracerIndex] -= (bm->outgas_NH / dzz) / numsec;
                        boxLayerInfo->DebugFluxInfo[PhysioChemArray[index].debugIndex][WC][loss_id] -= (bm->atmospheric_NH / dzz) / numsec;
                        
                       //if ((bm->current_box == 165) && bm->newmonth)
                       //     fprintf(llogfp,"Time: %e box%d-%d outgassed\n", bm->dayt, bm->current_box, bm->current_layer);
                    }
                }
            }
        }
    }
}

/**
 *  \brief The Detrital Silica tracer flux calculation function.
 *
 *  Det_Si
 *
 */
void Detrital_Silica_ROC(MSEBoxModel *bm, FILE *llogfp, HABITAT_TYPES habitatType, int index, BoxLayerValues *boxLayerInfo) {

	int guild, cohort;
	double Si_Flux, DSi;

	switch (habitatType) {
	case WC:
		DSi = boxLayerInfo->localWCTracers[Det_Si_i];

		Si_Flux = 0.0;
		for (guild = 0; guild < bm->K_num_tot_sp; guild++) {
			if (FunctGroupArray[guild].isSiliconDependant) {
				for (cohort = 0; cohort < FunctGroupArray[guild].numCohortsXnumGenes; cohort++) {
					Si_Flux += (FunctGroupArray[guild].mortality[cohort] + FunctGroupArray[guild].lysis[cohort]
							+ (double)FunctGroupArray[guild].preyEaten[cohort][habitatType]);

	//				Si_Flux += (FunctGroupArray[guild].lysis[0] + FunctGroupArray[guild].preyEaten[cohort][habitatType]);  // TODO: Why was this only lysis and consumed?
                    /*
                    if (bm->dayt >= bm->checkstart) {
                        fprintf(llogfp,"FunctGroupArray[%s].lysis[%d] = %e\n", FunctGroupArray[guild].groupCode, cohort, FunctGroupArray[guild].lysis[cohort]);
						fprintf(llogfp,"FunctGroupArray[guild].preyEaten[%d][habitatType]= %Le\n", cohort, FunctGroupArray[guild].preyEaten[cohort][habitatType]);

                    }
                    */
                    
					if(!_finite(Si_Flux)){
						printf("FunctGroupArray[%s].lysis[%d] = %e\n", FunctGroupArray[guild].groupCode, cohort, FunctGroupArray[guild].lysis[cohort]);
						printf("FunctGroupArray[guild].preyEaten[%d][habitatType]= %Le\n", cohort, FunctGroupArray[guild].preyEaten[cohort][habitatType]);
						quit("Detrital_Silica_ROC. Si_flux is infinite.\n");

					}
				}
			}
		}
		boxLayerInfo->localWCFlux[*PhysioChemArray[index].tracerIndex] = Si_Flux * bm->X_SiN - r_DSi * DSi;
		boxLayerInfo->DebugFluxInfo[PhysioChemArray[index].debugIndex][WC][gain_id] = Si_Flux * bm->X_SiN;
		boxLayerInfo->DebugFluxInfo[PhysioChemArray[index].debugIndex][WC][loss_id] = r_DSi * DSi;
		break;
	case SED:
	case LAND_BASED:

		DSi = boxLayerInfo->localSEDTracers[*PhysioChemArray[index].tracerIndex];
		Si_Flux = 0;
		for (guild = 0; guild < bm->K_num_tot_sp; guild++) {
			if (FunctGroupArray[guild].isSiliconDependant) {
				for (cohort = 0; cohort < FunctGroupArray[guild].numCohortsXnumGenes; cohort++) {
					Si_Flux += (FunctGroupArray[guild].mortality[cohort] + FunctGroupArray[guild].lysis[cohort]
							+ (double)FunctGroupArray[guild].preyEaten[cohort][habitatType]);
				}
			}
		}
		boxLayerInfo->localSEDFlux[*PhysioChemArray[index].tracerIndex] = Si_Flux * bm->X_SiN - r_DSi * DSi;
		boxLayerInfo->DebugFluxInfo[PhysioChemArray[index].debugIndex][SED][gain_id] = Si_Flux * bm->X_SiN;
		boxLayerInfo->DebugFluxInfo[PhysioChemArray[index].debugIndex][SED][loss_id] = r_DSi * DSi;

		break;
	case EPIFAUNA:
		Si_Flux = 0.0;
		for (guild = 0; guild < bm->K_num_tot_sp; guild++) {
			if (FunctGroupArray[guild].isSiliconDependant) {
				for (cohort = 0; cohort < FunctGroupArray[guild].numCohortsXnumGenes; cohort++) {
					if(FunctGroupArray[guild].groupType == MICROPHTYBENTHOS){
						Si_Flux += FunctGroupArray[guild].dead[cohort];
						Si_Flux += ((double)FunctGroupArray[guild].preyEaten[cohort][WC] + (double)FunctGroupArray[guild].preyEaten[cohort][SED]);

					} else {
						Si_Flux += ((double)FunctGroupArray[guild].preyEaten[cohort][WC]);
					}
				}
			}
		}

		boxLayerInfo->localSEDFlux[*PhysioChemArray[index].tracerIndex] += (Si_Flux * bm->X_SiN);
		boxLayerInfo->DebugFluxInfo[PhysioChemArray[index].debugIndex][SED][gain_id] = (Si_Flux * bm->X_SiN);
		break;
	case ICE_BASED:
		DSi = boxLayerInfo->localICETracers[Det_Si_i];

		Si_Flux = 0.0;
		for (guild = 0; guild < bm->K_num_tot_sp; guild++) {
			if (FunctGroupArray[guild].isSiliconDependant) {
				Si_Flux += (FunctGroupArray[guild].mortality[0] + FunctGroupArray[guild].lysis[0]);
						//+ FunctGroupArray[guild].preyEaten[0][habitatType]);
			}
		}

		boxLayerInfo->localICEFlux[*PhysioChemArray[index].tracerIndex] = Si_Flux * bm->X_SiN - r_DSi * DSi;
		boxLayerInfo->DebugFluxInfo[PhysioChemArray[index].debugIndex][ICE_BASED][gain_id] = Si_Flux * bm->X_SiN;
		boxLayerInfo->DebugFluxInfo[PhysioChemArray[index].debugIndex][ICE_BASED][loss_id] = r_DSi * DSi;
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
 *  \brief The Nitrate tracer flux calculation function.
 *  NO3_i
 *
 */
void Nitrate_ROC(MSEBoxModel *bm, FILE *llogfp, HABITAT_TYPES habitatType, int index, BoxLayerValues *boxLayerInfo) {

    double dzz, numsec = 86400.0;
    
	switch (habitatType) {
	case WC:

		boxLayerInfo->localWCFlux[*PhysioChemArray[index].tracerIndex] = FunctGroupArray[pelagicBactIndex].nitrif
				- (double)boxLayerInfo->NutsLost[WC][NO_id];

		boxLayerInfo->DebugFluxInfo[PhysioChemArray[index].debugIndex][WC][gain_id] = FunctGroupArray[pelagicBactIndex].nitrif;
		boxLayerInfo->DebugFluxInfo[PhysioChemArray[index].debugIndex][WC][loss_id] = (double)boxLayerInfo->NutsLost[WC][NO_id];
            
        /*
        if(bm->current_box == bm->checkbox){
            fprintf(bm->logFile, "Time: %e box %d-%d NO flux %e ", bm->dayt, bm->current_box, bm->current_layer,
                    boxLayerInfo->localWCFlux[*PhysioChemArray[index].tracerIndex]);
            fprintf(bm->logFile, "FunctGroupArray[pelagicBactIndex].nitrif = %e ", FunctGroupArray[pelagicBactIndex].nitrif);
            fprintf(bm->logFile, "boxLayerInfo->NutsLost[WC][NO_id] = %Le\n", boxLayerInfo->NutsLost[WC][NO_id]);
                
        }
        */
		break;
	case SED:
	case LAND_BASED:

		boxLayerInfo->localSEDFlux[*PhysioChemArray[index].tracerIndex] = boxLayerInfo->Nitrification - boxLayerInfo->Denitrification
				- (double)boxLayerInfo->NutsLost[SED][NO_id];

		boxLayerInfo->DebugFluxInfo[PhysioChemArray[index].debugIndex][SED][gain_id] = boxLayerInfo->Nitrification;
		boxLayerInfo->DebugFluxInfo[PhysioChemArray[index].debugIndex][SED][loss_id] = boxLayerInfo->Denitrification + (double)boxLayerInfo->NutsLost[SED][NO_id];
            
        /*
        if(bm->current_box == bm->checkbox){
            fprintf(bm->logFile, "Time: %e box %d-%d SED NO flux %e ", bm->dayt, bm->current_box, bm->current_layer,
                        boxLayerInfo->localSEDFlux[*PhysioChemArray[index].tracerIndex]);
            fprintf(bm->logFile, "boxLayerInfo->Nitrification = %e ", boxLayerInfo->Nitrification);
            fprintf(bm->logFile, "boxLayerInfo->Denitrification = %e ", boxLayerInfo->Denitrification);
            fprintf(bm->logFile, "boxLayerInfo->NutsLost[SED][NO_id] = %Le\n", boxLayerInfo->NutsLost[SED][NO_id]);
                
        }
        */
		break;
	case EPIFAUNA:
		boxLayerInfo->localWCFlux[*PhysioChemArray[index].tracerIndex] += -(double)boxLayerInfo->NutsLost[WC][NO_id];

		boxLayerInfo->DebugFluxInfo[PhysioChemArray[index].debugIndex][WC][loss_id] = (double)boxLayerInfo->NutsLost[WC][NO_id];

		boxLayerInfo->localSEDFlux[*PhysioChemArray[index].tracerIndex] += -(double)boxLayerInfo->NutsLost[SED][NO_id];
		boxLayerInfo->DebugFluxInfo[PhysioChemArray[index].debugIndex][SED][loss_id] = (double)boxLayerInfo->NutsLost[SED][NO_id];
          
        /*
        if(bm->current_box == bm->checkbox){
            fprintf(bm->logFile, "Time: %e box %d-%d EPI wcNO flux %e sedflux: %e ", bm->dayt, bm->current_box, bm->current_layer,
                    boxLayerInfo->localWCFlux[*PhysioChemArray[index].tracerIndex], boxLayerInfo->localSEDFlux[*PhysioChemArray[index].tracerIndex]);
            fprintf(bm->logFile, "boxLayerInfo->NutsLost[WC][NO_id] = %Le ", boxLayerInfo->NutsLost[WC][NO_id]);
            fprintf(bm->logFile, "boxLayerInfo->NutsLost[SED][NO_id] = %Le\n", boxLayerInfo->NutsLost[SED][NO_id]);
            
        }
        */
            
		break;
	case ICE_BASED:
		boxLayerInfo->localICEFlux[*PhysioChemArray[index].tracerIndex] = FunctGroupArray[IceBactIndex].nitrif
				- (double)boxLayerInfo->NutsLost[ICE_BASED][NO_id];

		boxLayerInfo->DebugFluxInfo[PhysioChemArray[index].debugIndex][ICE_BASED][gain_id] = FunctGroupArray[IceBactIndex].nitrif;
		boxLayerInfo->DebugFluxInfo[PhysioChemArray[index].debugIndex][ICE_BASED][loss_id] = (double)boxLayerInfo->NutsLost[ICE_BASED][NO_id];
		break;
    case MIXED:
        quit("How did we get here as should come through a primary habitat\n");
        break;
    default:
        quit("How did we get here as this isn't even a habitat type slot but nTotHabTypes\n");
        break;
	}
    
	if(bm->include_atmosphere){
		/** Get NO3 values from atmosphare on the surface cell **/
		if (bm->current_layer == (bm->boxes[bm->current_box].nz - 1)) {
			dzz = bm->boxes[bm->current_box].dz[bm->current_layer];
			if (dzz != 0.0) {
				boxLayerInfo->localWCFlux[*PhysioChemArray[index].tracerIndex] += (bm->atmospheric_NO / dzz) / numsec;
				boxLayerInfo->DebugFluxInfo[PhysioChemArray[index].debugIndex][WC][gain_id] += (bm->atmospheric_NO / dzz) / numsec;
                
                if (bm->include_outgas) {
                    if (boxLayerInfo->localWCTracers[NO3_i] > (bm->outgas_prop * 12500.0)) { // Where 12500.0 is the saturation point
                        boxLayerInfo->localWCFlux[*PhysioChemArray[index].tracerIndex] -= (bm->outgas_NO / dzz) / numsec;
                        boxLayerInfo->DebugFluxInfo[PhysioChemArray[index].debugIndex][WC][loss_id] -= (bm->atmospheric_NO / dzz) / numsec;
                    }
                }
                
			}
		}
	}

}
/**
 *  \brief The Micronutrient tracer flux calculation function.
 *
 *  Micronutrient flux is dependant on N flux minus uptake due to primary producers.
 *
 */
void Micronutrient_ROC(MSEBoxModel *bm, FILE *llogfp, HABITAT_TYPES habitatType, int index, BoxLayerValues *boxLayerInfo) {

	double gain = 0.0;
    double dzz, numsec = 86400.0;

	if (bm->flagmicro) {
		switch (habitatType) {
		case WC:
			gain = (boxLayerInfo->localWCFlux[NH3_i] + FunctGroupArray[pelagicBactIndex].nitrif + (double)boxLayerInfo->NutsLost[WC][NH_id]) * bm->X_FeN;

			boxLayerInfo->localWCFlux[*PhysioChemArray[index].tracerIndex] = gain - (double)boxLayerInfo->NutsLost[WC][Fe_id];

			boxLayerInfo->DebugFluxInfo[PhysioChemArray[index].debugIndex][WC][gain_id] = gain;
			boxLayerInfo->DebugFluxInfo[PhysioChemArray[index].debugIndex][WC][loss_id] = (double)boxLayerInfo->NutsLost[WC][Fe_id];

			break;
		case SED:
		case LAND_BASED:

			gain = (boxLayerInfo->localSEDFlux[NH3_i] + boxLayerInfo->Nitrification + (double)boxLayerInfo->NutsLost[SED][NH_id]) * bm->X_FeN;

			boxLayerInfo->localSEDFlux[*PhysioChemArray[index].tracerIndex] = gain - (double)boxLayerInfo->NutsLost[SED][Fe_id];

			boxLayerInfo->DebugFluxInfo[PhysioChemArray[index].debugIndex][SED][gain_id] = gain;
			boxLayerInfo->DebugFluxInfo[PhysioChemArray[index].debugIndex][SED][loss_id] = (double)boxLayerInfo->NutsLost[SED][Fe_id];

			break;
		case EPIFAUNA:

			gain = (double)boxLayerInfo->NutsProd[WC][NH_id] * bm->X_FeN;
			boxLayerInfo->localWCFlux[*PhysioChemArray[index].tracerIndex] += gain - (double)boxLayerInfo->NutsLost[WC][Fe_id];

			boxLayerInfo->DebugFluxInfo[PhysioChemArray[index].debugIndex][WC][gain_id] = gain;
			boxLayerInfo->DebugFluxInfo[PhysioChemArray[index].debugIndex][WC][loss_id] = (double)boxLayerInfo->NutsLost[WC][Fe_id];

			gain = (double)boxLayerInfo->NutsProd[SED][NH_id] * bm->X_FeN;
			boxLayerInfo->localSEDFlux[*PhysioChemArray[index].tracerIndex] += gain - (double)boxLayerInfo->NutsLost[SED][Fe_id];

			boxLayerInfo->DebugFluxInfo[PhysioChemArray[index].debugIndex][SED][gain_id] = gain;
			boxLayerInfo->DebugFluxInfo[PhysioChemArray[index].debugIndex][SED][loss_id] = (double)boxLayerInfo->NutsLost[SED][Fe_id];

			break;

		case ICE_BASED:
			gain = (boxLayerInfo->localICEFlux[NH3_i] + FunctGroupArray[IceBactIndex].nitrif + (double)boxLayerInfo->NutsLost[ICE_BASED][NH_id]) * bm->X_FeN;

			boxLayerInfo->localICEFlux[*PhysioChemArray[index].tracerIndex] = gain - (double)boxLayerInfo->NutsLost[ICE_BASED][Fe_id];

			boxLayerInfo->DebugFluxInfo[PhysioChemArray[index].debugIndex][ICE_BASED][gain_id] = gain;
			boxLayerInfo->DebugFluxInfo[PhysioChemArray[index].debugIndex][ICE_BASED][loss_id] = (double)boxLayerInfo->NutsLost[ICE_BASED][Fe_id];
			break;
        case MIXED:
            quit("How did we get here as should come through a primary habitat\n");
            break;
        default:
            quit("How did we get here as this isn't even a habitat type slot but nTotHabTypes\n");
            break;
		}
	} else {
		/* Set all the tracers and debug values to 0 */
		boxLayerInfo->localWCFlux[*PhysioChemArray[index].tracerIndex] = 0.0;
		boxLayerInfo->localSEDFlux[*PhysioChemArray[index].tracerIndex] = 0.0;

		boxLayerInfo->DebugFluxInfo[PhysioChemArray[index].debugIndex][WC][gain_id] = 0.0;
		boxLayerInfo->DebugFluxInfo[PhysioChemArray[index].debugIndex][WC][loss_id] = 0.0;
		boxLayerInfo->DebugFluxInfo[PhysioChemArray[index].debugIndex][SED][gain_id] = 0.0;
		boxLayerInfo->DebugFluxInfo[PhysioChemArray[index].debugIndex][SED][loss_id] = 0.0;

		if(bm->ice_on) {
			boxLayerInfo->DebugFluxInfo[PhysioChemArray[index].debugIndex][ICE_BASED][gain_id] = 0.0;
			boxLayerInfo->DebugFluxInfo[PhysioChemArray[index].debugIndex][ICE_BASED][loss_id] = 0.0;
		}
	}
    
	if(bm->include_atmosphere){
		/** Get atmosphare input on the surface cell **/
		if (bm->current_layer == (bm->boxes[bm->current_box].nz - 1)) {
			dzz = bm->boxes[bm->current_box].dz[bm->current_layer];
			if (dzz != 0.0) {
				boxLayerInfo->localWCFlux[*PhysioChemArray[index].tracerIndex] += (bm->atmospheric_F / dzz) / numsec;
				boxLayerInfo->DebugFluxInfo[PhysioChemArray[index].debugIndex][WC][gain_id] += (bm->atmospheric_F / dzz) / numsec;
			}
		}
	}

}
/**
 *  \brief The DON tracer flux calculation function.
 *
 *  Dissolved organic nitrogen.
 *
 */
void DON_ROC(MSEBoxModel *bm, FILE *llogfp, HABITAT_TYPES habitatType, int index, BoxLayerValues *boxLayerInfo) {

	switch (habitatType) {
	case WC:
		boxLayerInfo->localWCFlux[*PhysioChemArray[index].tracerIndex] = FunctGroupArray[LabDetIndex].solDON
				+ FunctGroupArray[RefDetIndex].solDON - boxLayerInfo->DONremin + FunctGroupArray[pelagicBactIndex].prodnDON;

		boxLayerInfo->DebugFluxInfo[PhysioChemArray[index].debugIndex][WC][gain_id] = FunctGroupArray[LabDetIndex].solDON
				+ FunctGroupArray[RefDetIndex].solDON + FunctGroupArray[pelagicBactIndex].prodnDON;
		boxLayerInfo->DebugFluxInfo[PhysioChemArray[index].debugIndex][WC][loss_id] = boxLayerInfo->DONremin;
		break;
	case SED:
	case LAND_BASED:
		boxLayerInfo->localSEDFlux[DON_i] = FunctGroupArray[LabDetIndex].solDON + FunctGroupArray[RefDetIndex].solDON
				- boxLayerInfo->DONremin + FunctGroupArray[SedBactIndex].prodnDON;

		boxLayerInfo->DebugFluxInfo[PhysioChemArray[index].debugIndex][SED][gain_id] = FunctGroupArray[LabDetIndex].solDON
				+ FunctGroupArray[RefDetIndex].solDON + FunctGroupArray[SedBactIndex].prodnDON;
		boxLayerInfo->DebugFluxInfo[PhysioChemArray[index].debugIndex][SED][loss_id] = boxLayerInfo->DONremin;

		break;
	case EPIFAUNA:
		/* Do nothing */
		break;
	case ICE_BASED:
		boxLayerInfo->localICEFlux[*PhysioChemArray[index].tracerIndex] = FunctGroupArray[LabDetIndex].solDON + FunctGroupArray[RefDetIndex].solDON - boxLayerInfo->DONremin + FunctGroupArray[IceBactIndex].prodnDON;

		boxLayerInfo->DebugFluxInfo[PhysioChemArray[index].debugIndex][ICE_BASED][gain_id] = FunctGroupArray[LabDetIndex].solDON
				+ FunctGroupArray[RefDetIndex].solDON + FunctGroupArray[IceBactIndex].prodnDON;
		boxLayerInfo->DebugFluxInfo[PhysioChemArray[index].debugIndex][ICE_BASED][loss_id] = boxLayerInfo->DONremin;

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
 *  \brief The Oxygen tracer flux calculation function.
 *
 */
void Oxygen_ROC(MSEBoxModel *bm, FILE *llogfp, HABITAT_TYPES habitatType, int index, BoxLayerValues *boxLayerInfo) {
    
    double dzz, temp_sq, added_o2, numsec = 86400.0;

	switch (habitatType) {
	case WC:
		boxLayerInfo->localWCFlux[Oxygen_i] = -bm->X_ON * (boxLayerInfo->localWCFlux[NH3_i] + boxLayerInfo->localWCFlux[NO3_i]);
		boxLayerInfo->DebugFluxInfo[PhysioChemArray[index].debugIndex][WC][loss_id] = bm->X_ON * (boxLayerInfo->localWCFlux[NH3_i]
				+ boxLayerInfo->localWCFlux[NO3_i]);

		break;
	case SED:
	case LAND_BASED:
		boxLayerInfo->localSEDFlux[Oxygen_i] = -bm->X_ON * (boxLayerInfo->localSEDFlux[NH3_i] + boxLayerInfo->localSEDFlux[NO3_i]);
		boxLayerInfo->DebugFluxInfo[PhysioChemArray[index].debugIndex][SED][loss_id] = bm->X_ON * (boxLayerInfo->localSEDFlux[NH3_i] + boxLayerInfo->localSEDFlux[NO3_i]);

		break;
	case EPIFAUNA:

		boxLayerInfo->localWCFlux[Oxygen_i] += -bm->X_ON * (boxLayerInfo->localWCFlux[NH3_i] + boxLayerInfo->localWCFlux[NO3_i]);
		boxLayerInfo->DebugFluxInfo[PhysioChemArray[index].debugIndex][WC][loss_id] = bm->X_ON * (boxLayerInfo->localWCFlux[NH3_i] + boxLayerInfo->localWCFlux[NO3_i]);

		boxLayerInfo->localSEDFlux[Oxygen_i] += -bm->X_ON * (boxLayerInfo->localSEDFlux[NH3_i] + boxLayerInfo->localSEDFlux[NO3_i]);
		boxLayerInfo->DebugFluxInfo[PhysioChemArray[index].debugIndex][SED][loss_id] = bm->X_ON * (boxLayerInfo->localSEDFlux[NH3_i] + boxLayerInfo->localSEDFlux[NO3_i]);

		break;

	case ICE_BASED:
		boxLayerInfo->localICEFlux[Oxygen_i] = -bm->X_ON * (FunctGroupArray[LabDetIndex].solDON + FunctGroupArray[RefDetIndex].solDON - boxLayerInfo->DONremin + FunctGroupArray[IceBactIndex].prodnDON);

		boxLayerInfo->DebugFluxInfo[PhysioChemArray[index].debugIndex][ICE_BASED][gain_id] = FunctGroupArray[LabDetIndex].solDON
				+ FunctGroupArray[RefDetIndex].solDON + FunctGroupArray[IceBactIndex].prodnDON;
		boxLayerInfo->DebugFluxInfo[PhysioChemArray[index].debugIndex][ICE_BASED][loss_id] = boxLayerInfo->DONremin;
		break;
    case MIXED:
        quit("How did we get here as should come through a primary habitat\n");
        break;
    default:
        quit("How did we get here as this isn't even a habitat type slot but nTotHabTypes\n");
        break;
	}
    
	if(bm->include_atmosphere){
		/** Get O2 values from atmosphare on the surface cell. This uses a relationship for temperature dependent air-sea O2 flux
		 calculated off Figure 1 in Gruber et al Global Biogeochemical Cycles paper "Air-sea flux of oxygen estimated from bulk data:
		 Implications for the marine and atmospheric oxygen cycles" **/
		if (bm->current_layer == (bm->boxes[bm->current_box].nz - 1)) {
			dzz = bm->boxes[bm->current_box].dz[bm->current_layer];
			temp_sq = H2Otemp * H2Otemp;
			added_o2 = bm->atmospheric_O2 - 0.0008 * temp_sq * temp_sq + 0.0594 * temp_sq * H2Otemp - 1.1908 * temp_sq + 0.613 * H2Otemp;
			if (dzz != 0.0) {
				boxLayerInfo->localWCFlux[*PhysioChemArray[index].tracerIndex] += (added_o2 / dzz) / numsec;
				boxLayerInfo->DebugFluxInfo[PhysioChemArray[index].debugIndex][WC][gain_id] += (added_o2 / dzz) / numsec;
                
                if (bm->include_outgas) {
                    if (boxLayerInfo->localWCTracers[Oxygen_i] > (bm->outgas_prop * 8000.0)) { // Where 8000.0 is the saturation point
                        boxLayerInfo->localWCFlux[*PhysioChemArray[index].tracerIndex] -= (bm->outgas_O2 / dzz) / numsec;
                        boxLayerInfo->DebugFluxInfo[PhysioChemArray[index].debugIndex][WC][loss_id] -= (bm->atmospheric_O2 / dzz) / numsec;
                    }
                }

			}
		}
	}
    
    //fprintf(bm->logFile, "Time: %e Added %e o2 to box%d-%d\n", bm->dayt, added_o2, bm->current_box, bm->current_layer);

}
/**
 *  \brief The Dissolved Silica tracer flux calculation function.
 *
 */
void Dissolved_Silica_ROC(MSEBoxModel *bm, FILE *llogfp, HABITAT_TYPES habitatType, int index, BoxLayerValues *boxLayerInfo) {

	double DSi;
    double dzz, numsec = 86400.0;

	switch (habitatType) {
	case WC:
		DSi = boxLayerInfo->localWCTracers[Det_Si_i];
		boxLayerInfo->localWCFlux[Si_i] = r_DSi * DSi - (double)boxLayerInfo->NutsLost[WC][Si_id];
		boxLayerInfo->DebugFluxInfo[PhysioChemArray[index].debugIndex][WC][gain_id] = r_DSi * DSi;
		boxLayerInfo->DebugFluxInfo[PhysioChemArray[index].debugIndex][WC][loss_id] = (double)boxLayerInfo->NutsLost[WC][Si_id];
		break;
	case SED:
	case LAND_BASED:
		DSi = boxLayerInfo->localSEDTracers[Det_Si_i];
		boxLayerInfo->localSEDFlux[Si_i] = r_DSi * DSi - (double)boxLayerInfo->NutsLost[SED][Si_id];
		boxLayerInfo->DebugFluxInfo[PhysioChemArray[index].debugIndex][SED][gain_id] = r_DSi * DSi;
		boxLayerInfo->DebugFluxInfo[PhysioChemArray[index].debugIndex][SED][loss_id] = (double)boxLayerInfo->NutsLost[SED][Si_id];
		break;
	case EPIFAUNA:
		/* Do nothing */
		break;
	case ICE_BASED:
		DSi = boxLayerInfo->localICETracers[Det_Si_i];
		boxLayerInfo->localICEFlux[Si_i] = r_DSi * DSi - (double)boxLayerInfo->NutsLost[ICE_BASED][Si_id];
		boxLayerInfo->DebugFluxInfo[PhysioChemArray[index].debugIndex][ICE_BASED][gain_id] = r_DSi * DSi;
		boxLayerInfo->DebugFluxInfo[PhysioChemArray[index].debugIndex][ICE_BASED][loss_id] = (double)boxLayerInfo->NutsLost[ICE_BASED][Si_id];
		break;
    case MIXED:
        quit("How did we get here as should come through a primary habitat\n");
        break;
    default:
        quit("How did we get here as this isn't even a habitat type slot but nTotHabTypes\n");
        break;
	}
    
	if(bm->include_atmosphere){
		/** Get Si values from atmosphare on the surface cell **/
		if (bm->current_layer == (bm->boxes[bm->current_box].nz - 1)) {
			dzz = bm->boxes[bm->current_box].dz[bm->current_layer];
			if (dzz != 0.0) {
				boxLayerInfo->localWCFlux[*PhysioChemArray[index].tracerIndex] += (bm->atmospheric_Si / dzz) / numsec;
				boxLayerInfo->DebugFluxInfo[PhysioChemArray[index].debugIndex][WC][gain_id] += (bm->atmospheric_Si / dzz) / numsec;
			}
		}
	}

}

/**
 * \brief The Carbon tracer flux caluclations.
 *
 *
 */
void Carbon_ROC(MSEBoxModel *bm, FILE *llogfp, HABITAT_TYPES habitatType, int index, BoxLayerValues *boxLayerInfo) {

	double C_flux;
    double dzz, numsec = 86400.0;

	switch (habitatType) {
	case WC:
		C_flux = bm->atomicRatioInfo->C_respiration
				+  bm->atomicRatioInfo->C_decomp
				+ bm->atomicRatioInfo->C_trans
				- (double)boxLayerInfo->NutsLost[habitatType][C_id]
				+ (double)boxLayerInfo->NutsProd[habitatType][C_id];

		boxLayerInfo->localWCFlux[C_i] = C_flux;
		break;
	case SED:
	case LAND_BASED:

		C_flux = bm->atomicRatioInfo->C_respiration
					+ bm->atomicRatioInfo->C_decomp
					+ bm->atomicRatioInfo->C_trans
					- (double)boxLayerInfo->NutsLost[habitatType][C_id]
					+ (double)boxLayerInfo->NutsProd[habitatType][C_id];

		boxLayerInfo->localSEDFlux[C_i] = C_flux;

		break;
	case EPIFAUNA:
		boxLayerInfo->localWCFlux[C_i] += (double)boxLayerInfo->NutsProd[WC][C_id];
		boxLayerInfo->localSEDFlux[C_i] += (double)boxLayerInfo->NutsProd[SED][C_id];
		break;

	case ICE_BASED:
		/* Do nothing for now */
		break;
    case MIXED:
        quit("How did we get here as should come through a primary habitat\n");
        break;
    default:
        quit("How did we get here as this isn't even a habitat type slot but nTotHabTypes\n");
        break;
	}
    
	if(bm->include_atmosphere){
		/** Get CO2 values from atmosphare on the surface cell **/
		if (bm->current_layer == (bm->boxes[bm->current_box].nz - 1)) {
			dzz = bm->boxes[bm->current_box].dz[bm->current_layer];
			if (dzz != 0.0) {
				boxLayerInfo->localWCFlux[*PhysioChemArray[index].tracerIndex] += (bm->atmospheric_CO2 / dzz) / numsec;
				boxLayerInfo->DebugFluxInfo[PhysioChemArray[index].debugIndex][WC][gain_id] += (bm->atmospheric_CO2 / dzz) / numsec;
                
                if (bm->include_outgas) { /* TODO: Check what saturation point of CO2 is */
                    if (boxLayerInfo->localWCTracers[CO2_i] > (bm->outgas_prop * 15000.0)) { // Where 15000.0 is the saturation point
                        boxLayerInfo->localWCFlux[*PhysioChemArray[index].tracerIndex] -= (bm->outgas_CO2 / dzz) / numsec;
                        boxLayerInfo->DebugFluxInfo[PhysioChemArray[index].debugIndex][WC][loss_id] -= (bm->atmospheric_CO2 / dzz) / numsec;
                    }
                }
                
			}
		}
	}
}

/**
 * \brief The Phosphorus tracer flux caluclations.
 *
 *
 *	From Bierman 1981 Modelling pf Phytoplankton-Nutrient dynamics in Saginaw Bay, Lake Huron.
 *
 *
 *	dPCM/dt = 	boundary contribution - net uptake
 *				+ respiration + zooplankton excretion
 *				+ decomposition + transformation + external loading
 *
 *
 *	respiration = for each zoooplankton((PSA(l) - PSAMIN(l)) * RRESP(l)1.08(T-20)
 *
 * 	zooplankton excretion = for each zoooplankton((PSA(l) - PSAMIN(l)) * RAGRZD(l)
 *
 * 	decomposition = TCROP * 1.08(T-20) * for each photoplankton((PSA(l) - PSAMIN(l)RLYS(l)
 *
 *	transformation = RTOP * TOP * 1.08(T-20).
 *
 * 	where 		PSA = actual total phosphorus in phytoplankton cells in moles/mg dry wgt.
 * 				PSAMIN = min qouta of phosphorus in phytoplankton cells in moles/,g dry wgt.
 * 						= PO(l)/FACT(l)
 *
 * 					where PO(l) = min cell quota of phosphorus per phytoplankton cell in moles/mg dry wt.
 * 							= value per group.
 * 					FACT(l) = phytoplankton cell size in mg dry wt.cell. value per group.
 * 				RAGRZD = rate at which phytoplakton l is grazed by zooplankton.
 * 				RRESP = phytoplankton respiration rate in mg A/lday (value per group)
 *				RLYS(l) = photoplankton decomposition rate in liters/m per day. = value per group.
 *				RTOP = rate of transformation from available nutrient forms to avaliable forms in day^-1.
 *
 *				TOP = concentration of available nutrients.
 *
 *	dPSA(l)/dt = boundary contribution + net uptake - gross production.
 *
 *	where:
 *	net uptake = R1PM(l) * FT(l) * FL(l) (0.322 x 10^-4) (moles P/mg P) * ( 1/(1 + PK1(l)*PCA(l)) - 1 / (1 + PK1(l)*PCM(l)))
 *
 *	gross production = PSA(l) * SPGR(l)
 *
 *	boundary contribution = Q/V * ABD(l) (PSABD(l) - PSA(l) ) / A(l) - handled by hydro code.
 *
 *
 *	where R1PM(l) = max phosphorus uptake rate per day = value per group.
 *		  FT(l) = temp limitation
 *		  FL(l) = light limitation
 *		  PK1(l) = affinity coefficient for phosphorus uptake mechanism in liter.mole. - value per group.
 *		  PCA(l) = intracellular available phosphorus concentrations
 *		  		= PSA/CONCP
 *		  		where CONCP = value per group.
 *		  PCM(l) = concentration of phosphorus in water column in moles/L.
 *
 *		  SPGR(l) = phytoplankton specific growth rate per day.
 */
void Phosphorus_ROC(MSEBoxModel *bm, FILE *llogfp, HABITAT_TYPES habitatType, int index, BoxLayerValues *boxLayerInfo) {

	double PCM_flux;
    double dzz, numsec = 86400.0;

	if(bm->track_atomic_ratio == TRUE){
		switch (habitatType) {
			case WC:

				PCM_flux = bm->atomicRatioInfo->PCM_respiration
						//+  bm->atomicRatioInfo->PCM_decomp
						+ bm->atomicRatioInfo->P_trans
						- (double)boxLayerInfo->NutsLost[habitatType][P_id]
						+ (double)boxLayerInfo->NutsProd[habitatType][P_id];

				boxLayerInfo->localWCFlux[P_i] = PCM_flux;

//				if(bm->current_box == 1 && bm->current_layer == 3){
//					fprintf(bm->logFile, "P flux = %e, rep = %e, decomp = %e, trans = %e, lost = %Le, prod = %Le\n", boxLayerInfo->localWCFlux[P_i],
//							bm->atomicRatioInfo->PCM_respiration, bm->atomicRatioInfo->PCM_decomp, bm->atomicRatioInfo->P_trans, boxLayerInfo->NutsLost[WC][P_id], boxLayerInfo->NutsProd[habitatType][P_id] );
//				}

				break;
			case SED:
			case LAND_BASED:


				PCM_flux = bm->atomicRatioInfo->PCM_respiration
					+ bm->atomicRatioInfo->PCM_decomp
					+ bm->atomicRatioInfo->P_trans
					- (double)boxLayerInfo->NutsLost[habitatType][P_id]
					+ (double)boxLayerInfo->NutsProd[habitatType][P_id];

				boxLayerInfo->localSEDFlux[P_i] = PCM_flux;

				break;
			case EPIFAUNA:
				boxLayerInfo->localWCFlux[P_i] += ((double)boxLayerInfo->NutsProd[WC][P_id] - (double)boxLayerInfo->NutsLost[WC][P_id]);
				boxLayerInfo->localSEDFlux[P_i] += ((double)boxLayerInfo->NutsProd[SED][P_id] - (double)boxLayerInfo->NutsLost[SED][P_id]);

				break;
			case ICE_BASED:
				/* Do nothing for now */
				break;
            case MIXED:
                quit("How did we get here as should come through a primary habitat\n");
                break;
            default:
                quit("How did we get here as this isn't even a habitat type slot but nTotHabTypes\n");
                break;
		}
	}
    
	if(bm->include_atmosphere){
		/** Get P values from atmosphare on the surface cell **/
		if (bm->current_layer == (bm->boxes[bm->current_box].nz - 1)) {
			dzz = bm->boxes[bm->current_box].dz[bm->current_layer];
			if (dzz != 0.0) {
				boxLayerInfo->localWCFlux[*PhysioChemArray[index].tracerIndex] += (bm->atmospheric_P / dzz) / numsec;
				boxLayerInfo->DebugFluxInfo[PhysioChemArray[index].debugIndex][WC][gain_id] += (bm->atmospheric_P / dzz) / numsec;
			}
		}
	}

}

/*
 * Equations for unavailable P.
 *
 * Sinking?
 *
 */

void TOP_ROC(MSEBoxModel *bm, FILE *llogfp, HABITAT_TYPES habitatType, int index, BoxLayerValues *boxLayerInfo) {

	double TOP_flux;

	if(bm->track_atomic_ratio == FALSE){
		TOP_flux = 0;
	}else{
		TOP_flux = bm->atomicRatioInfo->TOP_respiration + bm->atomicRatioInfo->TOP_decomp + bm->atomicRatioInfo->TOP_excretion
				+ bm->atomicRatioInfo->TOP_death - bm->atomicRatioInfo->P_trans;
	}


	//fprintf(bm->logFile, "TOP_flux = %e\n", TOP_flux);

	TOP_flux = 0;

	switch (habitatType) {
	case WC:
		boxLayerInfo->localWCFlux[TOP_i] = TOP_flux;
		break;
	case SED:
	case LAND_BASED:
		boxLayerInfo->localSEDFlux[TOP_i] = TOP_flux;
		break;
	case EPIFAUNA:
		break;
	case ICE_BASED:
		/* Do nothing for now */
		break;
    case MIXED:
        quit("How did we get here as should come through a primary habitat\n");
        break;
    default:
        quit("How did we get here as this isn't even a habitat type slot but nTotHabTypes\n");
        break;
	}
}
