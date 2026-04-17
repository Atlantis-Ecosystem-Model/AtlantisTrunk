/*
 * additionalTracer.c
 *
 *  Created on: Jul 30, 2012
 *      Author: bec
 *
 *
 *
 * 	The vertebrates ratios only use RN and SN to keep track of the ratio of P_N and C_N. Originally we were tracking the changes in total biomass for these groups but the rounding errors were too hard to deal with.
 */

/*  Open library routines */
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "string.h"
#include "atecology.h"

/*
 * 	Allocate the atomic ratio structure.
 *
 * 	Also sets up the names of the additional tracers for debugging purposes.
 *
 *
 */
void Ecology_Initialise_Atomic_Info(MSEBoxModel *bm) {
    int paramlen = 25;
    
	bm->atomicRatioInfo = (AtomicRatioStructure *) malloc(sizeof(AtomicRatioStructure));

	bm->atomicRatioInfo->atomicName = (char **) c_alloc2d(paramlen, num_atomic_id);
	bm->atomicRatioInfo->C_decomp = 0;
	bm->atomicRatioInfo->C_trans = 0;
	bm->atomicRatioInfo->C_respiration = 0;
	bm->atomicRatioInfo->RTOP = 0;
	bm->atomicRatioInfo->PCM_respiration = 0;
	bm->atomicRatioInfo->TOP_respiration = 0;
	bm->atomicRatioInfo->PCM_excretion = 0;
	bm->atomicRatioInfo->TOP_excretion = 0;
	bm->atomicRatioInfo->P_trans = 0;
	bm->atomicRatioInfo->TOP_death = 0;
	bm->atomicRatioInfo->K_Lc = 0;
	bm->atomicRatioInfo->PCM_decomp = 0;

	snprintf(bm->atomicRatioInfo->atomicName[p_id], paramlen, "Phosphorus");
	snprintf(bm->atomicRatioInfo->atomicName[c_id], paramlen, "Carbon");
}

/**
 * Free up the atomic info structure. Called on model shutdown.
 *
 */
void Ecology_Free_Atomic_Info(MSEBoxModel *bm) {

	c_free2d(bm->atomicRatioInfo->atomicName);
	free(bm->atomicRatioInfo);
}

/**
 *  Calculate P uptake in phytoplankton.
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
 *		  PSA = current cell concentration.
 *
 *	Really need to be calculating the flux in the ratio of P:N.
 *
 **/

double Calculate_Phosphorus_Uptake(MSEBoxModel *bm, int guild, double grow, double PRatio){

	double  uptake;
	//double P0 = FunctGroupArray[guild].speciesParams[P_min_internal_id];
	//double P1 = FunctGroupArray[guild].speciesParams[P_max_internal_id];
	//double PValue = biomass * PRatio;
	//double growth =PValue * mum * hN_sp * hI_sp * eddy_strength * bm->eddy_scale;

	/* Calculate gross production - TODO: may need to add the ability to have multiple genetypes here */

	uptake = grow * PRatio; //PValue * mum * hN_sp * hI_sp * eddy_strength * bm->eddy_scale;
	//uptake = growth * FunctGroupArray[guild].speciesParams[P_max_uptake_id] * ((P1 - PRatio)/(P1 - P0));// * PCM/( FunctGroupArray[guild].speciesParams[KP_id] + PCM);

	/* Save the uptake value for later */
	FunctGroupArray[guild].uptakeP = uptake;

	return uptake;
}

/**
 *  Calculate C uptake in phytoplankton.
 *
 *	Using same formulation as that used for P.
 *
 **/

double Calculate_Carbon_Uptake(MSEBoxModel *bm, int guild, double grow, double CRatio){

	double uptake = grow * CRatio;

	/* Save the uptake value for later */
	FunctGroupArray[guild].uptakeC = uptake;

	return uptake;
}
//
//double Calculate_Carbon_Uptake(MSEBoxModel *bm, int guild, double grow, double hN_sp, double hI_sp, double CCM, double CRatio, double biomass) {
//
////	double CSA, CCA;
////	double production, uptake, flux;
////	double mult = 1e-11;
//	double uptake;
//
//	double CValue = biomass * CRatio;
//	//double growth =PValue * mum * hN_sp * hI_sp * eddy_strength * bm->eddy_scale;
//
//	uptake = CValue * mum * hN_sp * hI_sp * eddy_strength * bm->eddy_scale;
//
//#ifdef NO_RATIO
//		//FunctGroupArray[guild].addRatioFluxes[0][c_id] = 0;
//#endif
//
//	/* Save the uptake value for later */
//	FunctGroupArray[guild].uptakeC = uptake;
//
//	return uptake;
//}

/**
 * 	Respiration of Phosphorus
 *
 *  = sum over each phytoplankton
 *
 *  	A * (PSA - PSAMIN) * RRESP * 1.08 ^(T - 20)
 *
 *
 * 	where
 * 		PSA = actual total phosphorus in phytoplankton cells in moles / mg dry weight.
 * 		PSAMIN = minimum quota of phosphorus in phytoplankton cells in moles / mg dry weight.
 * 		RRESP = photoplankton respiration rate in day-1.
 * 		T = temperature in deg C
 * 		A = phytoplankton concentration in mg dry wt/L
 *
 * 		1L = 1000m^3.
 *
 */
static int Calculate_Phosphorus_Respiration(MSEBoxModel *bm, BoxLayerValues *boxLayerInfo, HABITAT_TYPES habitatType, double *PCM_respiration,
double *TOP_respiration) {

	int sp;
	double PSA, PRatio, biomass, PSAMIN;
	double *tracerArray = getTracerArray(boxLayerInfo, habitatType);
	double resp, RRESP, top_resp;
	double T, Tcorr;

	PRatio = -1;

	resp = 0, top_resp = 0;

	T = tracerArray[Temp_i];
	Tcorr = pow(1.08, T - 20);

	for (sp = 0; sp < bm->K_num_tot_sp; sp++) {
		if ((int) (FunctGroupArray[sp].speciesParams[flag_id]) == TRUE) {
			if (FunctGroupArray[sp].isPrimaryProducer == TRUE) {

				if(FunctGroupArray[sp].habitatCoeffs[habitatType]){
					biomass = tracerArray[FunctGroupArray[sp].totNTracers[main_biomass_id]];
					PRatio = tracerArray[FunctGroupArray[sp].addRatioTracers[0][p_id]];

					/* Calculate the existing amount of phosphorus */
					PSA = PRatio;

					PSAMIN = FunctGroupArray[sp].speciesParams[Psa_min_id];

					/* Respiration rate per second */
					RRESP = FunctGroupArray[sp].speciesParams[phyto_resp_rate_id];

					resp += (biomass * (PSA - PSAMIN) * RRESP * Tcorr) * 1 / 1000.0;

					top_resp += (biomass * PSAMIN * RRESP * Tcorr) * 1 / 1000.0;

				}
			}
		}
	}

	*PCM_respiration = resp;
	*TOP_respiration = top_resp;

	return 0;
}

/**
 * Carbon Respiration..
 *
 *	Not at all sure about this.
 *
 *
 */

static int Calculate_Carbon_Respiration(MSEBoxModel *bm, BoxLayerValues *boxLayerInfo, HABITAT_TYPES habitatType, double *C_respiration) {

	int sp;
	double CA, PRatio, biomass, CMIN;
	double *tracerArray = getTracerArray(boxLayerInfo, habitatType);
	double resp, RRESP;
	double T, Tcorr;

	PRatio = -1;

	resp = 0;

	T = tracerArray[Temp_i];
	Tcorr = pow(1.08, T - 20);

	for (sp = 0; sp < bm->K_num_tot_sp; sp++) {
		if ((int) (FunctGroupArray[sp].speciesParams[flag_id]) == TRUE) {
			if(FunctGroupArray[sp].habitatCoeffs[habitatType]){

				if (FunctGroupArray[sp].isPrimaryProducer == TRUE) {

					biomass = tracerArray[FunctGroupArray[sp].totNTracers[main_biomass_id]];
					PRatio = tracerArray[FunctGroupArray[sp].addRatioTracers[0][c_id]];


					/* Calculate the existing amount of Carbon */
					CA = PRatio; // Was CA = PRatio * biomass but that would square biomass below in resp calculation, not square in the P calc above

					CMIN = FunctGroupArray[sp].speciesParams[C_min_id];

					/* Respiration rate per second */
					RRESP = FunctGroupArray[sp].speciesParams[phyto_resp_rate_id];
					resp += (biomass * (CA - CMIN) * RRESP * Tcorr) * 1 / 1000.0;

					/*
					 //if(strcmp(FunctGroupArray[sp].groupCode, "PL") == 0 &&
					 if(bm->current_box == 5 && bm->current_layer == 5){
					 fprintf(bm->logFile, "%s - resp = %e, biomass = %e, PRatio= %e, CA= %e, CMIN= %e, RRESP= %e, Tcorr = %e\n",
					 FunctGroupArray[sp].groupCode, resp, biomass, PRatio, CA, CMIN,RRESP, Tcorr);
					 }*/
				}
			}
		}
	}

	*C_respiration = resp;

	return 0;
}

/**
 * 		Decomposition of Phosphorus
 *
 * 	= TCROP * 1.08^(T - 20)  * sum over each phytoplankton ( A * (PSA - PSAMIN) * RLYS
 *
 * 	where
 *
 * 		TCROP = total phytoplankton concentration in mg dry weight/L.
 * 		T = temperature in deg C.
 * 		A = phytoplankton concentration in mg dry weight/L.
 * 		PSA = actual total phosphorus in phytoplankton cells in moles / mg dry weight.
 * 		PSAMIN = minimum quota of phosphorus in phytoplankton cells in moles / mg dry weight.
 * 		RLYS = phytoplankton decomposition rate in liter /mg day.
 *
 */
//static int Calculate_Phosphorus_Decomposition(MSEBoxModel *bm, BoxLayerValues *boxLayerInfo, HABITAT_TYPES habitatType, double *PCM_decomp, double *TOP_decomp) {
//
//	*PCM_decomp = 0;//decomp; // Already taken into account elsewhere.
//	*TOP_decomp = 0;
//
//	return 0;
//
//}

static int Calculate_Carbon_Decomposition(MSEBoxModel *bm, BoxLayerValues *boxLayerInfo, HABITAT_TYPES habitatType, double *C_decomp) {

	double decomp;

	decomp = (double) boxLayerInfo->NutsProd[habitatType][C_id];

	*C_decomp = decomp;

	return 0;
}

/**
 * 		Transformation of Phosphorus
 *
 *
 * 	= RTOP * TOP * 1.08^(T-20)
 *
 * 	where
 * 		RTOP = rates of transformation from available nutrient forms to available forms in days-1. - input parameter.
 * 		TOP = concentration of unavailable nutrient forms in moles/L.
 * 		T = temperature in deg C.
 */
static int Calculate_Phosphorus_Transformation(MSEBoxModel *bm, BoxLayerValues *boxLayerInfo, HABITAT_TYPES habitatType, double *trans) {

	double *tracerArray = getTracerArray(boxLayerInfo, habitatType);
	double RTOP, TOP, T;

	T = tracerArray[Temp_i];
	TOP = tracerArray[TOP_i];

	RTOP = bm->atomicRatioInfo->RTOP / (24.0 * 86400.0); /* divide to get rate per second */

//	if(bm->current_box == 1 && bm->current_layer == 3){
//		fprintf(bm->logFile, "RTOP = %e, bm->atomicRatioInfo->RTOP= %e\n",RTOP, bm->atomicRatioInfo->RTOP);
//	}

	*trans = RTOP * TOP * pow(1.08, T - 20);

	return 0;
}

/**
 * Transfer of Carbon to and from atmosphere.
 *
 *	Reaeration = A_sur * K_lc (CO2_sat - CO2)
 * *
 *	Where:
 *
 *	A_sur = surface area m^2
 *	K_lc = carbon exchange rate m/s. Not 100% sure about this.
 *	CO2_sat = CO2 saturation concentration
 *	CO2 = CO2 concentration
 *	where
 *	CO2_sat = 0.286e(-0.0314* T_s) * P_a
 *
 *	where:
 *
 *	T_s = water temperature deg C
 *	P_a = altitude correction factor due to height above sea level - not sure if we know this in Atlantis.
 *
 */
static int Calculate_Carbon_Transformation(MSEBoxModel *bm, BoxLayerValues *boxLayerInfo, HABITAT_TYPES habitatType, double *trans) {

	double *tracerArray = getTracerArray(boxLayerInfo, habitatType);
	double P_a, T, CO2_sat, CO2;
	double A_sur;

	/* If this is the top layer */
	if (bm->current_layer == bm->boxes[bm->current_box].nz) {
		T = tracerArray[Temp_i];
		P_a = 1.0;
		A_sur = bm->boxes[bm->current_box].area;

		CO2_sat = 0.286 * exp(-0.0314 * T * P_a);
		CO2 = tracerArray[CO2_i];

		*trans = A_sur * bm->atomicRatioInfo->K_Lc * (CO2_sat - CO2);
	}

	return 0;
}

/**
 * Overall Phosphorus flux equation
 *
 * dPCM/dt = boundary contribution - net uptake + respiration + zooplankton excretion +
 * 				decomposition + transformation + external loading.
 *
 *
 *	This is called after all functional group activities are done.
 *	So eating and dying is all done so can calculate the new ratios of all groups other than primary producers.
 *
 *  Excretion and death are handled elsewhere.
 */

int Calculate_Phosphorus(MSEBoxModel *bm, BoxLayerValues *boxLayerInfo, HABITAT_TYPES habitatType) {

	/* Uptake - already done in phytoplankton calculations */

	/* Respiration */
	Calculate_Phosphorus_Respiration(bm, boxLayerInfo, habitatType, &bm->atomicRatioInfo->PCM_respiration, &bm->atomicRatioInfo->TOP_respiration);

	/* Decomposition */
	//Calculate_Phosphorus_Decomposition(bm, boxLayerInfo, habitatType, &bm->atomicRatioInfo->PCM_decomp, &bm->atomicRatioInfo->TOP_decomp);

	/* Transformation */
	Calculate_Phosphorus_Transformation(bm, boxLayerInfo, habitatType, &bm->atomicRatioInfo->P_trans);

	return 0;
}

/**
 *	Overall Carbon flux calculations.
 *
 *	Called in atecology.c at the end of each water column, sediment or epibenthic layer.
 *
 *
 *
 *
 */
int Calculate_Carbon(MSEBoxModel *bm, BoxLayerValues *boxLayerInfo, HABITAT_TYPES habitatType) {

	Calculate_Carbon_Respiration(bm, boxLayerInfo, habitatType, &bm->atomicRatioInfo->C_respiration);
	Calculate_Carbon_Decomposition(bm, boxLayerInfo, habitatType, &bm->atomicRatioInfo->C_decomp);
	Calculate_Carbon_Transformation(bm, boxLayerInfo, habitatType, &bm->atomicRatioInfo->C_trans);

	return 0;

}
/**
 * 	Calculate the flux of the additional elements.
 *
 *
 */

void Calculate_Element_Flux(MSEBoxModel *bm, double *tracerArray, double *fluxArray, HABITAT_TYPES habitat, HABITAT_TYPES callHabitat) {
	int sp, cohort, tracerIndex, c;
	double elementflux = 0.0, Density, eaten, totalBiomass;

	//Finalise_GrazeLive_Gain(bm, habitat, callHabitat);

	for (sp = 0; sp < bm->K_num_tot_sp; sp++) {


		if ((int) (FunctGroupArray[sp].speciesParams[flag_id]) == TRUE && (int) (FunctGroupArray[sp].speciesParams[active_id]) == TRUE) {

			if(FunctGroupArray[sp].habitatCoeffs[habitat] > 0){
				if (FunctGroupArray[sp].isOncePerDt == FALSE || (it_count == 1 && FunctGroupArray[sp].isOncePerDt == TRUE)) {
					for (tracerIndex = 0; tracerIndex < num_atomic_id; tracerIndex++) {


						for (cohort = 0; cohort < FunctGroupArray[sp].numCohorts; cohort++) {

							switch(FunctGroupArray[sp].groupAgeType){
							case AGE_STRUCTURED:
								Density = tracerArray[FunctGroupArray[sp].NumsTracers[cohort]]/bm->cell_vol;
								elementflux = ((double)FunctGroupArray[sp].ratioGainedPred[cohort][tracerIndex]/(Density + small_num)) - (double)FunctGroupArray[sp].ratioLost[habitat][cohort][tracerIndex];

                                fprintf(bm->logFile, "%s-%d Box %d-%d Density: %e Nums: %e cell_vol: %e ratioGainedPred: %e ratioLost: %e habitat: %d\n", FunctGroupArray[sp].groupCode, cohort, bm->current_box, bm->current_layer, Density, tracerArray[FunctGroupArray[sp].NumsTracers[cohort]], bm->cell_vol, ((double)FunctGroupArray[sp].ratioGainedPred[cohort][tracerIndex]), ((double)FunctGroupArray[sp].ratioLost[habitat][cohort][tracerIndex]), habitat);
                                    
								break;

							case AGE_STRUCTURED_BIOMASS:

								/* Get the total biomss */
								totalBiomass = 0.0;
								for (c = 0; c < FunctGroupArray[sp].numCohorts; c++) {
									totalBiomass += tracerArray[FunctGroupArray[sp].totNTracers[c]];
								}
								totalBiomass += small_num;

								eaten = ((double)FunctGroupArray[sp].ratioLostPred[habitat][0][tracerIndex]
									* tracerArray[FunctGroupArray[sp].totNTracers[cohort]]) / totalBiomass;

								elementflux = (double)FunctGroupArray[sp].ratioGained[habitat][cohort][tracerIndex] + (double)FunctGroupArray[sp].ratioGainedPred[cohort][tracerIndex]
									- (double)FunctGroupArray[sp].ratioLost[habitat][cohort][tracerIndex] - eaten;


								/**
                                //if(sp == 5 && tracerIndex == c_id && cohort == 1 && bm->current_box == 15 && bm->current_layer == 3){

									fprintf(bm->logFile, "Box = %d\n", bm->current_box);
									fprintf(bm->logFile, "before add flux = %e\n",fluxArray[FunctGroupArray[sp].addRatioTracers[cohort][tracerIndex]]);
									fprintf(bm->logFile, "FunctGroupArray[%d].ratioLostPred[%d][%d][%d] = %Le\n", sp, habitat, cohort, tracerIndex,  FunctGroupArray[sp].ratioLostPred[habitat][cohort][tracerIndex]);
									fprintf(bm->logFile, "%s - habitat %d, elementflux = %e, growth = %Le, growth due to pred = %Le, lost due to being eaten %e, ratioLost (mortality, lysis and dead)= %Le\n", FunctGroupArray[sp].groupCode, habitat, elementflux, FunctGroupArray[sp].ratioGained[habitat][cohort][tracerIndex], FunctGroupArray[sp].ratioGainedPred[cohort][tracerIndex], eaten, FunctGroupArray[sp].ratioLost[habitat][cohort][tracerIndex]);
								//}
                                **/

								break;

							case BIOMASS:
								if (FunctGroupArray[sp].isPrimaryProducer == TRUE) {
									/*elementflux = FunctGroupArray[sp].ratioGained[habitat][cohort][tracerIndex] +
											FunctGroupArray[sp].ratioGainedPred[cohort][tracerIndex] +
											FunctGroupArray[sp].addRatioFluxes[cohort][tracerIndex]
											- (FunctGroupArray[sp].ratioLost[habitat][cohort][tracerIndex]
											                                                  + FunctGroupArray[sp].addRatioLost[cohort][tracerIndex]
											                                                  + FunctGroupArray[sp].ratioLostPred[habitat][cohort][tracerIndex]);*/

									/* calculated elsewehere for PP groups */
									elementflux = 0;


								} else {
									elementflux = (double)FunctGroupArray[sp].ratioGained[habitat][cohort][tracerIndex] + (double)FunctGroupArray[sp].ratioGainedPred[cohort][tracerIndex]
										- (double)FunctGroupArray[sp].ratioLost[habitat][cohort][tracerIndex] - (double)FunctGroupArray[sp].ratioLostPred[habitat][cohort][tracerIndex];

									/**if(sp == 41 && bm->current_box == 1 && tracerIndex == p_id){
										fprintf(bm->logFile, "Calculate_Element_Flux -box %d,  %s - habitat %d, elementflux = %.20e, gained = %.20Le, growth due to pred = %.20Le, lost due to being eaten %.20Le, ratioLost (mortality, lysis and dead)= %.20Le\n",
												bm->current_box, FunctGroupArray[sp].groupCode, habitat, elementflux,
												FunctGroupArray[sp].ratioGained[habitat][cohort][tracerIndex],
												FunctGroupArray[sp].ratioGainedPred[cohort][tracerIndex],
												FunctGroupArray[sp].ratioLostPred[habitat][cohort][tracerIndex],
												FunctGroupArray[sp].ratioLost[habitat][cohort][tracerIndex]);
									}**/
								}
								break;
							}

							if(callHabitat == EPIFAUNA && (habitat == WC || habitat == SED)){
								fluxArray[FunctGroupArray[sp].addRatioTracers[cohort][tracerIndex]] += elementflux;
							}else{
								fluxArray[FunctGroupArray[sp].addRatioTracers[cohort][tracerIndex]] = elementflux;
							}

							/*if(sp == 41 && bm->current_box == 1 && tracerIndex == p_id){
								fprintf(bm->logFile, "Calculate_Element_Flux -box %d,  %s - habitat %d, elementflux = %.20e\n",
									bm->current_box, FunctGroupArray[sp].groupCode, habitat,fluxArray[FunctGroupArray[sp].addRatioTracers[cohort][tracerIndex]]);
							}*/

							if (!(_finite(elementflux))){
								fprintf(bm->logFile, "elementflux = %e\n", elementflux);
								abort();
							}
						}
					}
				}
			}
		}
	}

	/* Must be zeroed out of the loop as the amount eaten for age structure inverts is stored in the cohort = 0 slot and needs to be partitioned based in biomass */
	for (sp = 0; sp < bm->K_num_tot_sp; sp++) {
		for (cohort = 0; cohort < FunctGroupArray[sp].numCohorts; cohort++) {
			for (tracerIndex = 0; tracerIndex < num_atomic_id; tracerIndex++) {
				FunctGroupArray[sp].ratioLost[habitat][cohort][tracerIndex] = 0.0;
				//FunctGroupArray[sp].ratioGainedPred[cohort][tracerIndex] = 0.0;
				FunctGroupArray[sp].ratioLostPred[habitat][cohort][tracerIndex] = 0.0;
			}
		}
	}
}


//void Calculate_PP_Ratio(MSEBoxModel *bm, double *tracerArray, double *fluxArray, HABITAT_TYPES habitat, HABITAT_TYPES callHabitat) {
//
//	int sp;
//	double uptakeN, uptakeP, uptakeC;
//
//
//	for (sp = 0; sp < bm->K_num_tot_sp; sp++) {
//
//
//			if ((int) (FunctGroupArray[sp].speciesParams[flag_id]) == TRUE && (int) (FunctGroupArray[sp].speciesParams[active_id]) == TRUE && (FunctGroupArray[sp].isPrimaryProducer) {
//
//			}
//
//}


void PP_uptake(MSEBoxModel *bm, int sp, double uptakeN, double uptakeP, double uptakeC){

	FunctGroupArray[sp].addRatioFluxes[0][c_id] = uptakeC;
	FunctGroupArray[sp].addRatioFluxes[0][p_id] = uptakeP;
	FunctGroupArray[sp].addRatioFluxes[0][num_atomic_id] = uptakeN;  // Overloaded intentional, the addRatioFluxes array has been sized to cope

	/*
	if(sp == 20 && bm->current_box == 1){

		fprintf(bm->logFile, "it_count = %d, Group %s, Nuptake = %e, Puptake = %e, Cuptake = %e\n",it_count, FunctGroupArray[sp].groupCode, uptakeN, uptakeP, uptakeC);
		//abort();
	}*/

}
/**
 * Called from Adapt_Diff_Method to calculate the new tracer values for the ratio tracers.
 *
 *
 *
 *
 */
void Integrate_Ratio_Variables(MSEBoxModel *bm, double *tracerArray, double *fluxArray, double tsz, double dtsz, HABITAT_TYPES habitat) {
    double step1 = 0;
	int sp, cohort, tracerIndex;
	double ratio, elementflux, elementValue, B, Bflux = 0, newN;
	double oldB, oldRatio;
	double newValue;
	double timeValue;
#ifdef NO_RATIO
	double diff;
#endif


    if(verbose) {
        printf("Doing Integrate_Ratio_Variables\n");
    }
    
	for (sp = 0; sp < bm->K_num_tot_sp; sp++) {
		Bflux = 0;

		if ((int) (FunctGroupArray[sp].speciesParams[flag_id]) == TRUE && (int) (FunctGroupArray[sp].speciesParams[active_id]) == TRUE) {

			if (FunctGroupArray[sp].habitatCoeffs[habitat] > 0) {
				if (FunctGroupArray[sp].isOncePerDt == FALSE || (it_count == 1 && FunctGroupArray[sp].isOncePerDt == TRUE)) {

					if (it_count == 1 && FunctGroupArray[sp].isOncePerDt == TRUE)
						timeValue = tsz;
					else
						timeValue = dtsz;

					for (tracerIndex = 0; tracerIndex < num_atomic_id; tracerIndex++) {

						/* Zero value*/
						elementflux = 0;
						
						for (cohort = 0; cohort < FunctGroupArray[sp].numCohorts; cohort++) {
							/* Get the new ratio for this group */
							oldRatio = tracerArray[FunctGroupArray[sp].addRatioTracers[cohort][tracerIndex]];

							if(FunctGroupArray[sp].isPrimaryProducer == TRUE){

								if(tracerArray[FunctGroupArray[sp].totNTracers[cohort]] <= bm->min_pool)
									continue;

								B = tracerArray[FunctGroupArray[sp].totNTracers[cohort]];
								Bflux = fluxArray[FunctGroupArray[sp].totNTracers[cohort]];
								oldB = B - Bflux * timeValue;

								if(oldB <= bm->min_pool || !Bflux)
									continue;

								elementValue = oldRatio * oldB;
								newValue = elementValue + (FunctGroupArray[sp].addRatioFluxes[cohort][tracerIndex] * timeValue);

								newN = oldB + (FunctGroupArray[sp].addRatioFluxes[cohort][num_atomic_id] * timeValue);
								ratio = newValue / newN;

#ifdef NO_RATIO
								elementflux = Bflux;
#endif
                                if(bm->flagratio_warn) {
                                    if(fabs(ratio - oldRatio) > buffer_rounding){
                                        warn("Integrate_Ratio_Variables Stop1: Group %s Box %d layer %d newN: %e oldB: %e newValue: %e it_count: %d ratio: %e B: %e habitat: %d elementValue: %e  oldRatio: %e oldB: %e num_atomic: %e timeValue: %e Bflux: %e\n", FunctGroupArray[sp].groupCode, bm->current_box, bm->current_layer, newN, oldB, newValue, it_count, ratio, B, habitat, elementValue, oldRatio, oldB, FunctGroupArray[sp].addRatioFluxes[cohort][num_atomic_id], timeValue, Bflux);
                                    }
                                }

								fprintf(bm->logFile, "Group %s Box %d layer %d newN: %e oldB: %e newValue: %e it_count: %d ratio: %e B: %e habitat: %d timeValue: %e oldRatio: %e Bflux: %e oldN: %e elementValue: %e\n", FunctGroupArray[sp].groupCode, bm->current_box, bm->current_layer, newN, oldB, newValue, it_count, ratio, B, habitat, timeValue, oldRatio, Bflux, FunctGroupArray[sp].addRatioFluxes[cohort][num_atomic_id], elementValue);


								/* Think this is right */
								tracerArray[FunctGroupArray[sp].addRatioTracers[cohort][tracerIndex]] = ratio;


#ifndef NO_RATIO
								if (ratio > 1.0 || ratio < 0) {
									fprintf(stderr, "B = %e, Bflux = %e\n", B, Bflux);

									printf("\n\nElement %s, %s:%d, box %d:%d, it_count = %d, habitat = %d, oldRatio = %e, newRatio = %e, elementFlux = %.20e, element = %e, (newElementValue = %e), Biomass = %e, Bflux = %e, T = %e\n",
											bm->atomicRatioInfo->atomicName[tracerIndex], FunctGroupArray[sp].groupCode, cohort, bm->current_box,
											bm->current_layer, it_count, habitat, oldRatio, ratio, elementflux, elementValue, newValue, B, Bflux, timeValue);

									printf("Bflux = %.20e, B = %.20e, oldB = %.20e, oldB + Bflux*T = %.20e\n", Bflux, B, oldB, oldB + Bflux * timeValue);


									quit("Integrate_Ratio_Variables Stop2:  - Box %d:%d - Group %s:%d pRatio is greater than 1.0 (%e)\n", bm->current_box, bm->current_layer,
											FunctGroupArray[sp].groupCode, cohort, ratio);
								}
#endif

							} else {



								if (FunctGroupArray[sp].groupAgeType == AGE_STRUCTURED) {

									/* If the numbers are down to min school - don't bother changing ration */
									if (tracerArray[FunctGroupArray[sp].NumsTracers[cohort]] <= bm->min_dens)
										continue;

									if(tracerArray[FunctGroupArray[sp].NumsTracers[cohort]] == 1e-16)
										continue;

									B = (tracerArray[FunctGroupArray[sp].structNTracers[cohort]] + tracerArray[FunctGroupArray[sp].resNTracers[cohort]]);

									Bflux = (fluxArray[FunctGroupArray[sp].structNTracers[cohort]] + fluxArray[FunctGroupArray[sp].resNTracers[cohort]]);
									oldB = B - Bflux * timeValue;

									/*
									if(tracerIndex == p_id && sp == 32 && cohort == 0 && bm->current_box == 1 && bm->current_layer == 0){
										fprintf(bm->logFile, "B = %.8e, oldB = %.8e, Bflux = %.8e\n", B, oldB, Bflux);

										fprintf(bm->logFile, "fluxArray[FunctGroupArray[sp].resNTracers[cohort]]= %e\n",fluxArray[FunctGroupArray[sp].resNTracers[cohort]]);
										fprintf(bm->logFile, "fluxArray[FunctGroupArray[sp].structNTracers[cohort]]= %e\n",fluxArray[FunctGroupArray[sp].structNTracers[cohort]]);

									}*/


								} else {
									if(tracerArray[FunctGroupArray[sp].totNTracers[cohort]] <= bm->min_pool)
										continue;

									B = tracerArray[FunctGroupArray[sp].totNTracers[cohort]];
									Bflux = fluxArray[FunctGroupArray[sp].totNTracers[cohort]];
									oldB = B - Bflux * timeValue;

								}
								if (B > 0 && fabs(Bflux) > 0) {

									elementValue = oldRatio * oldB;
									elementflux = fluxArray[FunctGroupArray[sp].addRatioTracers[cohort][tracerIndex]];

	#ifdef TURN_OFF_P_IN_VERTEBRATES
                                    if(FunctGroupArray[sp].isVertebrate == TRUE)
                                        continue;
	#endif

									if((elementValue + (elementflux * timeValue)) <= 0.0)
										continue;

									/* Calculate the new ratio */
									newValue = elementValue + (elementflux * timeValue);
									ratio = newValue / B;

									tracerArray[FunctGroupArray[sp].addRatioTracers[cohort][tracerIndex]] = ratio;

#ifndef NO_RATIO

                                    if(bm->flagratio_warn) {
                                        if(tracerIndex == c_id){
                                            step1 = fabs(ratio - bm->N_to_C); // Was 0.1754
                                            if(step1 > buffer_rounding){ // was buffer_ratio
                                                warn( "Integrate_Ratio_Variables Stop3 at %e ratio is too different from %e: C - Group %s:%d Box %d layer %d oldB = %e newValue= %e it_count: %d ratio: %.20e B: %e habitat: %d Bflux: %.20e elementflux: %.20e flux ratio: %.20e\n", step1, bm->N_to_C, FunctGroupArray[sp].groupCode, cohort,  bm->current_box, bm->current_layer, oldB, newValue, it_count, ratio, B, habitat, Bflux, elementflux, elementflux/Bflux);
                                            }
                                        } else {
                                            step1 = fabs(ratio - bm->N_to_P); // was 3e-5
                                            if(step1 > buffer_rounding){ // was buffer_ratio
                                                warn("Integrate_Ratio_Variables Stop4 at %e ratio is too different from %e P - Group %s-%d Box %d layer %d oldB: %e oldRatio: %e newValue: %e it_count: %d ratio: %e B: %e habitat: %d elementValue: %e elementflux: %e timeValue: %e\n", step1, bm->N_to_P, FunctGroupArray[sp].groupCode, cohort, bm->current_box, bm->current_layer, oldB, oldRatio, newValue, it_count, ratio, B, habitat, elementValue, elementflux, timeValue);
                                            }

                                        }
                                        step1 = fabs(ratio - oldRatio);
                                        if(step1 > buffer_rounding){ // was buffer_ratio
                                            warn("Integrate_Ratio_Variables Stop5 at %e difference in ratios too large: Group %s-%d Box %d layer %d oldB: %e newValue: %e it_count: %d ratio: %e, B: %e habitat %d\n", step1, FunctGroupArray[sp].groupCode, cohort, bm->current_box, bm->current_layer,   oldB, newValue, it_count, ratio, B, habitat);
                                        }


                                        /**/
                                        //if(fabs(ratio - oldRatio) > buffer_rounding){
                                            printf("Group %s-%d Box %d layer %d oldB: %e newValue: %e elementValue: %e elementflux: %e it_count: %d ratio: %e, B: %e, Bflux: %e habitat= %d\n", FunctGroupArray[sp].groupCode, cohort, bm->current_box, bm->current_layer, oldB, newValue, elementValue, elementflux, it_count, ratio, B, Bflux, habitat);

                                            fprintf(bm->logFile, "Group %s-%d Box %d layer %d newN: %e oldB: %e newValue: %e it_count: %d ratio: %e B: %e habitat: %d timeValue: %e oldRatio: %e Bflux: %e oldN: %e elementValue: %e elementflux: %e\n", FunctGroupArray[sp].groupCode, cohort,  bm->current_box, bm->current_layer, newN, oldB, newValue, it_count, ratio, B, habitat, timeValue, oldRatio, Bflux, FunctGroupArray[sp].addRatioFluxes[cohort][num_atomic_id], elementValue, elementflux);

                                            //quit("\n");
                                        //}
                                        /**/

                                        /**
                                        if(tracerIndex == p_id && sp == 32 && cohort == 0 && bm->current_box == 1 && bm->current_layer == 0){
                                            fprintf(bm->logFile, "B = %.8e, oldB = %.8e, Bflux = %.8e\n", B, oldB, Bflux);

                                            fprintf(bm->logFile, "New ratio = %e\n", tracerArray[FunctGroupArray[sp].addRatioTracers[cohort][tracerIndex]]);

                                        }
                                        **/
                                    }
									/*Check the tracers */
									if (!(_finite(tracerArray[FunctGroupArray[sp].addRatioTracers[cohort][tracerIndex]]))) {
										quit("Integrate_Ratio_Variables tracer is infinite. Group %s, cohort %d, B= %e, elementValue = %e, elementflux= %e, newValue= %e, ratio = %e\n",
											FunctGroupArray[sp].groupCode, cohort, B, elementValue, elementflux, newValue, ratio);
									}

									if (ratio > 1.0 || ratio < 0) {
										quit("Integrate_Ratio_Variables Stop6 - Box %d:%d - Group %s:%d pRatio is greater than 1.0 (%e) B: %e, Bflux: %e\nElement %s %s:%d box %d:%d it_count: %d habitat: %d oldRatio: %e newRatio: %e elementFlux: %.20e element: %e (newElementValue: %e) Biomass: %e Bflux: %e T: %e\nBflux: %.20e B: %.20e oldB: %.20e oldB + Bflux*T: %.20e\n", bm->current_box, bm->current_layer, FunctGroupArray[sp].groupCode, cohort, ratio, B, Bflux, bm->atomicRatioInfo->atomicName[tracerIndex], FunctGroupArray[sp].groupCode, cohort, bm->current_box, bm->current_layer, it_count, habitat, oldRatio, ratio, elementflux, elementValue, newValue, B, Bflux, timeValue, Bflux, B, oldB, oldB + Bflux * timeValue);
									}
#endif
								}
							}


#ifdef NO_RATIO
							diff = (elementflux - Bflux);

							if(fabs(diff) > buffer_rounding){
								fprintf(bm->logFile, "tracer = %d\n", FunctGroupArray[sp].addRatioTracers[cohort][tracerIndex]);
								fprintf(bm->logFile,"\n\nElement %s, %s:%d, box %d:%d, it_count = %d, habitat = %d, oldRatio = %e, newRatio = %e, elementFlux = %.20e, element = %e, (newElementValue = %e), Biomass = %e, Bflux = %e, T = %e\n",
										bm->atomicRatioInfo->atomicName[tracerIndex], FunctGroupArray[sp].groupCode, cohort, bm->current_box,
										bm->current_layer, it_count, habitat, oldRatio, ratio, elementflux, elementValue, newValue, B, Bflux, timeValue);
								fprintf(bm->logFile, "Bflux = %.20e, B = %.20e, oldB = %.20e, oldB + Bflux*T = %.20e\n", Bflux, B, oldB, oldB + Bflux * timeValue);
								fprintf(bm->logFile, "diff = %.20e\n", diff);
								quit("Integrate_Ratio_Variables Stop7: fabs(diff) > %e as diff: %e", buffer_rounding, diff);
							}
#endif

						}

					}
				}
			}
		}
	}
}

/**
 * Keep track of elements that are broken down by a group. The brokenDown value given is assumed to be the N value so we need to multiply this by the ratio
 * for each additional tracer to get the actual amount broken down.
 *
 * This is then stored in the boxLayerInfo->NutsProd array for later use.
 *
 *
 *
 *
 */
void Calculate_Element_Release(MSEBoxModel *bm, BoxLayerValues *boxLayerInfo, int guild, double brokenDown, HABITAT_TYPES habitat, HABITAT_TYPES globalHabitat) {

	int tracerIndex;
	double tracerValue, changeValue;
	double *tracerArray = getTracerArray(boxLayerInfo, habitat);

	for (tracerIndex = 0; tracerIndex <= p_id; tracerIndex++) {

		tracerValue = tracerArray[FunctGroupArray[guild].addRatioTracers[0][tracerIndex]];
		changeValue = tracerValue * brokenDown;
#ifdef NO_RATIO
		changeValue = brokenDown;
#endif

		switch (tracerIndex) {
		case p_id: /* P */

			boxLayerInfo->NutsProd[habitat][P_id] += changeValue; /* proportion that breaks down */

			if (FunctGroupArray[guild].diagTol == 2 && it_count == 1) {
				boxLayerInfo->NutsProdGlobal[globalHabitat][habitat][P_id] += changeValue;
			}
			break;
		case c_id: /* C */

			boxLayerInfo->NutsProd[habitat][C_id] += changeValue; /* proportion that breaks down */

			if (FunctGroupArray[guild].diagTol == 2 && it_count == 1) {
				boxLayerInfo->NutsProdGlobal[globalHabitat][habitat][C_id] += changeValue;
			}
			break;
		default:
			/* do nothing for now */
			break;

		}
	}
}

/**
 *
 *
 *
 */
void Loose_Element(MSEBoxModel *bm, BoxLayerValues *boxLayerInfo, HABITAT_TYPES habitat, int guild, int cohort, long double amount, HABITAT_TYPES globalHabitat, int isGlobal){

	double pRatio;
	double *tracerArray;
	int tracerIndex;
	double pLost;

	if(FunctGroupArray[guild].habitatType == EPIFAUNA){
		tracerArray = getTracerArray(boxLayerInfo, EPIFAUNA);
	}else{
		tracerArray = getTracerArray(boxLayerInfo, habitat);
	}


	/* Not sure this is 100% correct. Need to the total amount eaten per second */
	if (FunctGroupArray[guild].isVertebrate == TRUE){
		amount = amount * (tracerArray[FunctGroupArray[guild].structNTracers[cohort]] + tracerArray[FunctGroupArray[guild].resNTracers[cohort]]);
	}

	for (tracerIndex = 0; tracerIndex < num_atomic_id; tracerIndex++) {

		/* get the ratio of the biomass being transferred */
		pRatio = tracerArray[FunctGroupArray[guild].addRatioTracers[cohort][tracerIndex]];

		/* Find the amount of P being lost */
		pLost = pRatio * (double)amount;
#ifdef NO_RATIO
		pLost = amount;
#endif


		/* Keep track of the element lost*/
		FunctGroupArray[guild].ratioLost[habitat][cohort][tracerIndex] += pLost;

		/* Handle global values for later use */
		if(isGlobal){
			/* Amount lost */
			FunctGroupArray[guild].ratioLostGlobal[globalHabitat][habitat][cohort][tracerIndex] += pLost;
		}
	}
}

/**
 *
 *
 *
 */
void Loose_Element_From_Prey(MSEBoxModel *bm, BoxLayerValues *boxLayerInfo, HABITAT_TYPES habitat, int guild, int cohort, long double amount, HABITAT_TYPES globalHabitat, int isGlobal){

	double pRatio;
	double *tracerArray;
	int tracerIndex;
	double pLost;

	if(FunctGroupArray[guild].habitatType == EPIFAUNA){
		tracerArray = getTracerArray(boxLayerInfo, EPIFAUNA);
	}else{
		tracerArray = getTracerArray(boxLayerInfo, habitat);
	}


	/* Not sure this is 100% correct. Need to the total amount eaten per second */
	if (FunctGroupArray[guild].isVertebrate == TRUE){
		amount = amount * (tracerArray[FunctGroupArray[guild].structNTracers[cohort]] + tracerArray[FunctGroupArray[guild].resNTracers[cohort]]);
	}

	for (tracerIndex = 0; tracerIndex < num_atomic_id; tracerIndex++) {

		/* get the ratio of the biomass being transferred */
		pRatio = tracerArray[FunctGroupArray[guild].addRatioTracers[cohort][tracerIndex]];

		/* Find the amount of P being lost */
		pLost = pRatio * (double)amount;
#ifdef NO_RATIO
		pLost = amount;
#endif


		/* Keep track of the element lost*/
		FunctGroupArray[guild].ratioLostPred[habitat][cohort][tracerIndex] += pLost;

		/**
		if(guild == 43 && tracerIndex == p_id && bm->current_box == 14 && habitat == 1){
			fprintf(bm->logFile, "Loose_Element_From_Prey -  FunctGroupArray[%d].ratioLostPred[%d][%d][%d]= %.20Le, pLost= %e\n", guild, habitat, cohort, tracerIndex, FunctGroupArray[guild].ratioLostPred[habitat][cohort][tracerIndex], pLost);
		}**/


		/* Handle global values for later use */
		if(isGlobal){
			/* Amount lost */
			FunctGroupArray[guild].ratioLostPredGlobal[globalHabitat][habitat][cohort][tracerIndex] += pLost;

				/*if(tracerIndex == p_id  && guild == 13 && cohort == 1 ){
					fprintf(bm->logFile, "P lost global = %.20Le, pLost= %.20e\n",
							FunctGroupArray[guild].ratioLostGlobal[globalHabitat][habitat][cohort][tracerIndex],
							pLost);
				}*/

		}
	}
}


/**
 * Used to transfer P from groups to detritus.
 *
 *
 */
void Transfer_Element(MSEBoxModel *bm, BoxLayerValues *boxLayerInfo, HABITAT_TYPES habitatType, int toGuild, int toCohort, int fromGuild, int fromCohort, double amount, HABITAT_TYPES globalHabitatType, int isGlobal){
	double pRatio, pLost;
	double *tracerArray;
	int tracerIndex;

	if(FunctGroupArray[fromGuild].habitatType == EPIFAUNA){
		tracerArray = getTracerArray(boxLayerInfo, EPIFAUNA);
	}else{
		tracerArray = getTracerArray(boxLayerInfo, habitatType);
	}

	if (amount > 0) {

		/* Not sure this is 100% correct. Need to the total amount amount per second */
		if (FunctGroupArray[fromGuild].isVertebrate == TRUE){
			amount = amount * (tracerArray[FunctGroupArray[fromGuild].structNTracers[fromCohort]] + tracerArray[FunctGroupArray[fromGuild].resNTracers[fromCohort]]);
		}

		for (tracerIndex = 0; tracerIndex < num_atomic_id; tracerIndex++) {

			/* get the ratio of the biomass being transferred */
			pRatio = tracerArray[FunctGroupArray[fromGuild].addRatioTracers[fromCohort][tracerIndex]];

			/* Find the amount of P being transferred */
			pLost = pRatio * amount;
#ifdef NO_RATIO
		pLost = amount;
#endif

			/* Keep track of the element lost/gained due to predation */
			FunctGroupArray[fromGuild].ratioLost[habitatType][fromCohort][tracerIndex] += pLost;
			FunctGroupArray[toGuild].ratioGained[habitatType][toCohort][tracerIndex] += pLost;


			/*if(tracerIndex == p_id  && toGuild == 6 && toCohort == 1){
				fprintf(bm->logFile, "Transfer_Element - P lost in %s:cohort%d(habitat %d) due to being eaten by %s :cohort%d is %e (amount = %e, pRatio= %e), total gained = %.20Le\n",
						FunctGroupArray[fromGuild].groupCode, fromCohort, habitatType, FunctGroupArray[toGuild].groupCode,toCohort,  pLost,amount,pRatio,
						FunctGroupArray[fromGuild].ratioLost[habitatType][fromCohort][tracerIndex]);
			}*/

			/* Handle global values for later use */
			if(isGlobal){
				/* Amount gained due to being amount */
				FunctGroupArray[fromGuild].ratioLostGlobal[globalHabitatType][habitatType][fromCohort][tracerIndex] += pLost;

				/* Amount lost due to being amount */
				FunctGroupArray[toGuild].ratioGainedGlobal[globalHabitatType][habitatType][toCohort][tracerIndex] += pLost;

				/*if(tracerIndex == p_id  && toGuild == 30 && toCohort == 0 ){
					fprintf(bm->logFile, "Transfer_Element - P gained gllobal = %.20Le\n", FunctGroupArray[toGuild].ratioGainedPredGlobal[globalHabitatType][habitatType][toCohort][tracerIndex]);
					fprintf(bm->logFile, "Transfer_Element - P lost gllobal = %.20Le\n", FunctGroupArray[toGuild].ratioLostGlobal[globalHabitatType][habitatType][toCohort][tracerIndex]);

				}*/
			}
		}
	}
}

/**
 * Used to keep track of growth in invertebrates and gained in detritus.
 *
 *
 */

void Gain_Element(MSEBoxModel *bm, BoxLayerValues *boxLayerInfo, HABITAT_TYPES habitatType, int toGuild, int toCohort, int fromGuild, int fromCohort, double amount, HABITAT_TYPES globalHabitatType, int isGlobal){
	double pRatio;
	double *tracerArray;// = getTracerArray(boxLayerInfo, habitatType);
	int tracerIndex;
	double pLost;

	if(FunctGroupArray[fromGuild].habitatType == EPIFAUNA)
		tracerArray = getTracerArray(boxLayerInfo, EPIFAUNA);
	else
		tracerArray = getTracerArray(boxLayerInfo, habitatType);


	for (tracerIndex = 0; tracerIndex < num_atomic_id; tracerIndex++) {

		/* get the ratio of the biomass being transferred */
		pRatio = tracerArray[FunctGroupArray[fromGuild].addRatioTracers[fromCohort][tracerIndex]];

		/* Find the amount of P being transferred */
		pLost = pRatio * amount;
#ifdef NO_RATIO
		pLost = amount;
#endif

		/* Keep track of the element lost/gained due to toGuildation */
		FunctGroupArray[toGuild].ratioGained[habitatType][toCohort][tracerIndex] += pLost;

		/*if(tracerIndex == p_id  && toGuild == 6  && toCohort == 1 && amount > 0){
			fprintf(bm->logFile, "Gain_Element - P gained in %s:cohort%d(habitat %d) due to  eating %s :cohort%d is %e (amount = %e, pRatio= %e), total gained = %.20Le\n",
					FunctGroupArray[toGuild].groupCode, toCohort, habitatType, FunctGroupArray[fromGuild].groupCode,fromCohort,  pLost,amount,pRatio,
					FunctGroupArray[toGuild].ratioGainedPred[habitatType][toCohort][tracerIndex]);
		}*/

		/* Handle global values for later use */
		if(isGlobal){
			/* Amount gained */
			FunctGroupArray[toGuild].ratioGainedGlobal[globalHabitatType][habitatType][toCohort][tracerIndex] += pLost;

		}
	}
}


/**
 * Used to keep track of growth in invertebrates and gained in detritus.
 *
 *
 */

static void Gain_Element_Predation(MSEBoxModel *bm, BoxLayerValues *boxLayerInfo, HABITAT_TYPES habitatType, int toGuild, int toCohort, int fromGuild, int fromCohort, double amount){
	double pRatio;
	double *tracerArray;// = getTracerArray(boxLayerInfo, habitatType);
	int tracerIndex;
	double pLost;

	if(FunctGroupArray[fromGuild].habitatType == EPIFAUNA)
		tracerArray = getTracerArray(boxLayerInfo, EPIFAUNA);
	else
		tracerArray = getTracerArray(boxLayerInfo, habitatType);


	for (tracerIndex = 0; tracerIndex < num_atomic_id; tracerIndex++) {

		/* get the ratio of the biomass being transferred */
		pRatio = tracerArray[FunctGroupArray[fromGuild].addRatioTracers[fromCohort][tracerIndex]];

		/* Find the amount of P being transferred */
		pLost = pRatio * amount;
#ifdef NO_RATIO
		pLost = amount;
#endif

		/* Keep track of the element lost/gained due to toGuildation */
		FunctGroupArray[toGuild].ratioGainedPred[toCohort][tracerIndex] += pLost;

		/**/
		//if(amount > 0 && tracerIndex == c_id && toGuild == 5 && toCohort == 1 && bm->current_box == 15 && bm->current_layer == 3){
        if(amount > 0 && toGuild == 0){
			fprintf(bm->logFile, "habitatType = %d, after %s, amount = %e, ratio = %e, FunctGroupArray[toGuild].ratioGainedPred[toCohort][tracerIndex] = %Le\n",
					habitatType, FunctGroupArray[fromGuild].groupCode, amount, pRatio, FunctGroupArray[toGuild].ratioGainedPred[toCohort][tracerIndex]);

		}
        /**/
	}
}

/**
 *
 * Need to account for changes in habitat between epibenthic groups and non-epibenthic groups.
 *
 * Called by Invert_Activities().
 */

void Transfer_To_Pred(MSEBoxModel *bm, BoxLayerValues *boxLayerInfo, double ***spGRAZEinfo, int currentHabitat, int guild, int cohort, double scalar, double E_SP, double EDR_SP, double EDL_SP, HABITAT_TYPES globalHabitatType){
	int prey, habitat;
	double amount;
	int kij;

	/**
	 * Add up the grazeLive values in the eat order.
	 */
	for (prey = 0; prey < bm->K_num_tot_sp; prey++) {
		if (FunctGroupArray[prey].isVertebrate == FALSE){// && FunctGroupArray[prey].isPrimaryProducer == FALSE) {
			for (kij = 0; kij < FunctGroupArray[prey].numCohortsXnumGenes; kij++) {
				for (habitat = WC; habitat < bm->num_active_habitats; habitat++) {
					if (FunctGroupArray[prey].groupType != LAB_DET && FunctGroupArray[prey].groupType != REF_DET && FunctGroupArray[prey].groupType != CARRION) {
						amount = scalar * E_SP * spGRAZEinfo[prey][kij][habitat];

						if(habitat == EPIFAUNA){
							if (FunctGroupArray[prey].groupType != MICROPHTYBENTHOS) {
								amount = amount / smLayerThick;
							}
						}


						Gain_Element_Predation(bm, boxLayerInfo, (HABITAT_TYPES)habitat, guild, cohort, prey, 0, amount);

					}
					else if (FunctGroupArray[prey].groupType == LAB_DET){
						Gain_Element_Predation(bm, boxLayerInfo, (HABITAT_TYPES)habitat, guild, cohort, LabDetIndex, 0, scalar * EDL_SP * spGRAZEinfo[prey][kij][habitat]);
					}else if(FunctGroupArray[prey].groupType == REF_DET){
						Gain_Element_Predation(bm, boxLayerInfo, (HABITAT_TYPES)habitat, guild, cohort, RefDetIndex, 0, scalar * EDR_SP * spGRAZEinfo[prey][kij][habitat]);
					}
				}
			}
		}
	}
}

/**
 * Transfer the invertebrate prey biomass to the predator which is a vertebrate.
 *
 *
 */
void Vertebrates_Transfer_To_Pred(MSEBoxModel *bm, BoxLayerValues *boxLayerInfo, double ***spGRAZEinfo, int guild, int cohort, double growth_scalar,
		double E_SP, double E_Plant, double E3_sp, double E4_sp, double SPgrazeDL, double SPgrazeDR,  HABITAT_TYPES globalHabitatType, double density){
	int prey, habitat;
	double amount;
	int h, kij;
	double densityScalar;

	densityScalar = growth_scalar;

	for (prey = 0; prey < bm->K_num_tot_sp; prey++) {

		if (FunctGroupArray[prey].isPrimaryProducer == TRUE) {
			for (kij = 0; kij < FunctGroupArray[prey].numCohortsXnumGenes; kij++) {

				for (h = WC; h < bm->num_active_habitats; h++) {

					/* so so fuggly  - In for testing with original code.*/
					if(strcmp(FunctGroupArray[prey].groupCode, "MB") == 0){
						if(h == WC){
							habitat = SED;
						}else if(h == SED){
							habitat = WC;
						}else{
							habitat = h;
						}
					}
					else{
						habitat = h;
					}

					/* if this prey is epibenthic scale to get units in m^2 not m^3*/
					if (FunctGroupArray[prey].habitatType == EPIFAUNA){
						Gain_Element_Predation(bm, boxLayerInfo, WC, guild, cohort, prey, 0, densityScalar* E_Plant * spGRAZEinfo[prey][kij][habitat] / smLayerThick);

					}else{
						Gain_Element_Predation(bm, boxLayerInfo, WC, guild, cohort, prey, 0, densityScalar * E_Plant * spGRAZEinfo[prey][kij][habitat]);
					}
				}
			}
		}
	}


	/**
	 * Add up the grazeLive values in the eat order.
	 */
	for (prey = 0; prey < bm->K_num_tot_sp; prey++) {
		if (FunctGroupArray[prey].isVertebrate == FALSE && FunctGroupArray[prey].isPrimaryProducer == FALSE) {
			for (kij = 0; kij < FunctGroupArray[prey].numCohortsXnumGenes; kij++) {
				for (habitat = WC; habitat < bm->num_active_habitats; habitat++) {
					if (FunctGroupArray[prey].groupType != LAB_DET && FunctGroupArray[prey].groupType != REF_DET && FunctGroupArray[prey].groupType
							!= CARRION) {

						if(FunctGroupArray[prey].habitatType == EPIFAUNA){
							amount = densityScalar* E_SP * GRAZEinfo[prey][kij][habitat]/smLayerThick;
						}else{
							amount = densityScalar* E_SP * GRAZEinfo[prey][kij][habitat];
						}

						Gain_Element_Predation(bm, boxLayerInfo, WC, guild, cohort, prey, 0, amount);
					} else if (FunctGroupArray[prey].groupType == LAB_DET) {
						Gain_Element_Predation(bm, boxLayerInfo, (HABITAT_TYPES)habitat, guild, cohort, LabDetIndex, 0, densityScalar * E3_sp * spGRAZEinfo[prey][kij][habitat]);

					} else if(FunctGroupArray[prey].groupType == REF_DET) {
						Gain_Element_Predation(bm, boxLayerInfo, (HABITAT_TYPES)habitat, guild, cohort, RefDetIndex, 0, densityScalar * E4_sp * spGRAZEinfo[prey][kij][habitat]);

					}
				}
			}
		}
	}
}


/**
 * Called from Partition_Grazed_Fish().
 *
 *
 *
 */

void Gain_Fish_Element(MSEBoxModel *bm, BoxLayerValues *boxLayerInfo, HABITAT_TYPES habitat, int fromGuild, int fromCohort, int toGuild, int toCohort, double amount, HABITAT_TYPES globalHabitat, int isGlobal){

	double pRatio;
	double *tracerArray;
	int tracerIndex;
	double pLost;
	double E1_sp = FunctGroupArray[toGuild].speciesParams[E1_id];

	tracerArray = getTracerArray(boxLayerInfo, habitat);

	for (tracerIndex = 0; tracerIndex < num_atomic_id; tracerIndex++) {

		/* get the ratio of the biomass being transferred */
		pRatio = tracerArray[FunctGroupArray[fromGuild].addRatioTracers[fromCohort][tracerIndex]];

		/* Find the amount of P being lost */
		pLost = pRatio * amount;
#ifdef NO_RATIO
	pLost = amount;
#endif

		/* Keep track of the element lost*/
			FunctGroupArray[toGuild].ratioGainedPred[toCohort][tracerIndex] += (pLost * E1_sp);

			/*if(tracerIndex == c_id  && toGuild == 5 && toCohort == 1){
				fprintf(bm->logFile, "Gain_Fish_Element - P gained in %s:%d due to predation of %s:%d (habitat %d) is %e (amount = %e, pRatio= %e, E1_sp= %e), total gained = %.20Le\n",
						FunctGroupArray[toGuild].groupCode, toCohort, FunctGroupArray[fromGuild].groupCode, fromCohort,  habitat, pLost*E1_sp, amount, pRatio, E1_sp,
						FunctGroupArray[toGuild].ratioGainedPred[toCohort][tracerIndex]);
			}*/

	}
}


/********************************************* Alternative Phosphorous Code **************************************************************/

/* From CSIRO EMS model
 *
 *  Description: Carries out phosphorus adsorption-desorption in both water column and sediments,
 *               and considers two size classes of sediments (Fine and Mud)
 *
 *  Immobilisation only occurs in the sediments.
 *
 */


void p_adsorption_setup(MSEBoxModel *bm, BoxLayerValues *boxLayerInfo, HABITAT_TYPES habitatType) {
    double *tracerArray = getTracerArray(boxLayerInfo, habitatType);
    double Tfactor;

    /* make sure we grab the tracers for the correct layer type */
    if(!tracerArray[Tfactor_i]) {
        tracerArray[Tfactor_i] = 1.0;
    }
    Tfactor = tracerArray[Tfactor_i];

    tracerArray[Pads_r_i] = bm->Pads_r_t0 * Tfactor;
    tracerArray[r_immob_PIP_i] = bm->r_immob_PIP_t0 * Tfactor;
    tracerArray[TIP_i] += tracerArray[PIP_i] + tracerArray[PIPI_i] + tracerArray[PIP_Dust_i];
}

void p_adsorption_calc(MSEBoxModel *bm, BoxLayerValues *boxLayerInfo, HABITAT_TYPES habitatType)
{
    double *tracerArray = getTracerArray(boxLayerInfo, habitatType);
    double porosity, net_desorp, PIP_immob, net_desorp_Dust, PIP_immob_Dust;
    double Pads_r = tracerArray[Pads_r_i];
    double r_immob_PIP = tracerArray[r_immob_PIP_i];
    double Oxygen = tracerArray[Oxygen_i];
    double DIP = tracerArray[DIP_i];
    double EFI = tracerArray[EFI_i];
    double PIP = tracerArray[PIP_i];
    double PIP_Dust = tracerArray[PIP_Dust_i];
    double Dust = tracerArray[Dust_i];

    if(habitatType != SED) {
        porosity = 1.0;
    } else {
        porosity = bm->boxes[bm->current_box].sm.porosity[bm->current_layer];
    }
        
    EFI = max(bm->min_pool, (EFI - Dust));
    Dust = max(bm->min_pool, Dust);

    net_desorp = Pads_r * (PIP / (EFI * bm->Pads_K) -  DIP * (Oxygen / (bm->Pads_KO + Oxygen)));
    PIP_immob = r_immob_PIP * PIP;
    
    tracerArray[DIP_i] += (net_desorp) / porosity;
    tracerArray[PIP_i] -= (net_desorp + PIP_immob);
    tracerArray[PIPI_i] += PIP_immob;

    net_desorp_Dust = Pads_r * ( PIP_Dust / (Dust * bm->Pads_K) - DIP * (Oxygen / (bm->Pads_KO + Oxygen)));
    PIP_immob_Dust = r_immob_PIP * PIP_Dust;
    
    tracerArray[DIP_i] += (net_desorp_Dust) / porosity;
    tracerArray[PIP_Dust_i] -= (net_desorp_Dust + PIP_immob_Dust);
    tracerArray[PIPI_i] += PIP_immob_Dust;
    tracerArray[TIP_i] += tracerArray[PIP_i] + tracerArray[PIPI_i] + tracerArray[PIP_Dust_i];
    
    return;

}
