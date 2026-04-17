/**
 \file
 \brief C file for handling diagnostic printouts
 \ingroup atEcology

 File:	atfluxbreakdown.c
 Author:	Beth Fulton
 Created:	8/9/2005
 Purpose:	C file for handling diagnostic printouts
 Revisions: 8/9/2004 Created flux printout routines

 5/12/2008 Beth Fulton
 Tried to bring the flux printouts into line with the new FunctGroupArray usage and loops

 02-02-2009 Beth Fulton
 Changed all use of WC, SED and EPIFAUNA to WC, SED and EPIFAUNA.
 Changed the Max_Flux_Check and Print_Flux functions to no longer expect the NETflux, Sedflux and epiflux arrays
 and to use the bm->atEcologyModule tracer arrays instead.

 03-02-2009 Bec Gorton
 Changed the age structured biomass tracer output to show the adult and juv tracer values.

 11-04-5009 Bec Gorton
 Changed the Print_Flux code to use the PhysioChem tracer array to print
 out the physic chem tracer information to the log file.

 **/

/*  Open library routines */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <sjwlib.h>
#include "atecology.h"

/**
 *	\brief Flux checking routines
 *
 *	Routine to check for largest flux
 *	- FIX need to update this so it really works
 */
void Max_Flux_Check(MSEBoxModel *bm, int callerid, int *fluxsp, int *fluxid, int *fluxpool, int *fluxn, double *localWCFlux, double *localSEDFlux,
		double *localEPIFlux, FILE *llogfp) {
	int ans, fluxnum, fcnum, flux_sp = 0, flagsp;
	double maxflux = -MAXDOUBLE, valcheck;
	int fgIndex, n;

	ans = -1;
	fluxnum = -1;
	fcnum = 0;

	switch (callerid) {
	case WC: /* Watercolumn box */

		/* Check to see if ecological group responsible for maxflux */
		for (fgIndex = 0; fgIndex < bm->K_num_tot_sp; fgIndex++) {
			flagsp = (int) (FunctGroupArray[fgIndex].speciesParams[flag_id]);
			if (flagsp && FunctGroupArray[fgIndex].habitatCoeffs[WC] > 0) {
				switch (FunctGroupArray[fgIndex].groupAgeType) {
				case AGE_STRUCTURED:
					for (n = 0; n < FunctGroupArray[fgIndex].numCohortsXnumGenes; n++) {

                        /* Biomass checks */
						valcheck = (FunctGroupArray[fgIndex].grow[n][SN_id] + FunctGroupArray[fgIndex].grow[n][RN_id])
								* VERTinfo[fgIndex][n][DEN_id] / bm->cell_vol;
						if (fabs(valcheck) > maxflux) {
							ans = FunctGroupArray[fgIndex].totNTracers[n];
							maxflux = valcheck;
							flux_sp = fgIndex;
							fluxnum = WC; // Our should this be growth_id ?
							fcnum = n;
						}
                        
                        /* Biomass check 2 - never used as first check will trip it so don't both doing the test
                        if (fabs(valcheck) > maxflux) {
                            ans = -fgIndex;
                            maxflux = valcheck;
                            flux_sp = fgIndex;
                            fluxnum = growth_id;
                            fcnum = n;
                        }
                         */
                        
                        /* Death check */
						valcheck = (FunctGroupArray[fgIndex].dead[n] + (double)FunctGroupArray[fgIndex].preyEaten[n][WC]) * (VERTinfo[fgIndex][n][SN_id]
								+ VERTinfo[fgIndex][n][RN_id]) / bm->cell_vol;
						if (fabs(valcheck) > maxflux) {
							ans = -fgIndex;
							maxflux = valcheck;
							flux_sp = fgIndex;
							fluxnum = death_id;
							fcnum = n;
						}
					}
					break;
				case BIOMASS:	/* Intentional */
				case AGE_STRUCTURED_BIOMASS:
					for (n = 0; n < FunctGroupArray[fgIndex].numCohortsXnumGenes; n++) {
						if (fabs(localWCFlux[FunctGroupArray[fgIndex].totNTracers[n]]) > maxflux) {
							ans = FunctGroupArray[fgIndex].totNTracers[n];
							maxflux = localWCFlux[FunctGroupArray[fgIndex].totNTracers[n]];
							flux_sp = fgIndex;
							fluxnum = WC;
							fcnum = n;
						}
					}
					break;
				}
			}
		}
		/* If it wasn't one of those then it'll be a nutrient - base it on N, but may need to include Si */
		if (fabs(localWCFlux[NH3_i]) > maxflux) {
			ans = NH3_i;
			maxflux = localWCFlux[NH3_i];
			fluxnum = WC;
			flux_sp = bm->K_num_tot_sp;
		}
		if (fabs(localWCFlux[NO3_i]) > maxflux) {
			ans = NO3_i;
			maxflux = localWCFlux[NO3_i];
            fluxnum = WC;
			flux_sp = bm->K_num_tot_sp;
		}
		if (fabs(localWCFlux[DON_i]) > maxflux) {
			ans = DON_i;
			maxflux = localWCFlux[DON_i];
            fluxnum = WC;
			flux_sp = bm->K_num_tot_sp;
		}
		break;
	case SED: /* Sediment box */

		/* Check to see if ecological group responsible for maxflux */
		for (fgIndex = 0; fgIndex < bm->K_num_tot_sp; fgIndex++) {
			flagsp = (int) (FunctGroupArray[fgIndex].speciesParams[flag_id]);
			if (flagsp && FunctGroupArray[fgIndex].habitatCoeffs[SED] > 0) {
				switch (FunctGroupArray[fgIndex].groupAgeType) {
				case AGE_STRUCTURED:
					quit(
							"Currently no age structured groups allowed inside the sediments (at least in maxflux) - if this is allowed now recode this fluxcheck\n");
					break;
				case AGE_STRUCTURED_BIOMASS:	/* Intentional */
				case BIOMASS:
					for (n = 0; n < FunctGroupArray[fgIndex].numCohortsXnumGenes; n++) {
						if (fabs(localSEDFlux[FunctGroupArray[fgIndex].totNTracers[n]]) > maxflux) {
							ans = FunctGroupArray[fgIndex].totNTracers[n];
							maxflux = localSEDFlux[FunctGroupArray[fgIndex].totNTracers[n]];
							flux_sp = fgIndex;
							fluxnum = SED;
							fcnum = n;
						}
					}
					break;
				}
			}
		}
		/* If it wasn't one of those then it'll be a nutrient - base it on N, but may need to include Si  */
		if (fabs(localSEDFlux[NH3_i]) > maxflux) {
			ans = NH3_i;
			maxflux = localSEDFlux[NH3_i];
			fluxnum = SED;
			flux_sp = bm->K_num_tot_sp;
		}
		if (fabs(localSEDFlux[NO3_i]) > maxflux) {
			ans = NO3_i;
			maxflux = localSEDFlux[NO3_i];
			fluxnum = SED;
			flux_sp = bm->K_num_tot_sp;
		}
		if (fabs(localSEDFlux[DON_i]) > maxflux) {
			ans = DON_i;
			maxflux = localSEDFlux[DON_i];
			fluxnum = SED;
			flux_sp = bm->K_num_tot_sp;
		}
		break;
	case EPIFAUNA: /* Epibenthic box */

		for (fgIndex = 0; fgIndex < bm->K_num_tot_sp; fgIndex++) {
			flagsp = (int) (FunctGroupArray[fgIndex].speciesParams[flag_id]);
			if (flagsp) {
				switch (FunctGroupArray[fgIndex].groupAgeType) {
				case AGE_STRUCTURED:
					if (FunctGroupArray[fgIndex].habitatCoeffs[EPIFAUNA] > 0) {
						quit("Currently no age structured 2D epibenthos allowed as yet - if this is allowed now recode this fluxcheck\n");
					} else if (flagsp && FunctGroupArray[fgIndex].habitatCoeffs[SED] > 0) {
						quit(
								"Currently no age structured groups allowed inside the sediments (at least in maxflux) - if this is allowed now recode this fluxcheck\n");
					} else {
						for (n = 0; n < FunctGroupArray[fgIndex].numCohortsXnumGenes; n++) {
                            /* Growth check 1 */
							valcheck = (FunctGroupArray[fgIndex].grow[n][SN_id] + FunctGroupArray[fgIndex].grow[n][RN_id])
									* VERTinfo[fgIndex][n][DEN_id] / bm->cell_vol;
							if (fabs(valcheck) > maxflux) {
								ans = FunctGroupArray[fgIndex].totNTracers[n]; // was 0
								maxflux = valcheck;
								flux_sp = fgIndex;
								fluxnum = WC; // Should this be growth_id?
								fcnum = n; // was 0
							}
                            /* Growth check 2 - as above this is not used so don't waste time doing the check
                            if (fabs(valcheck) > maxflux) {
                                ans = -fgIndex;
                                maxflux = valcheck;
                                flux_sp = fgIndex;
                                fluxnum = growth_id;
                                fcnum = n;
                            }
                             */
                            /* Death check */
							valcheck = (FunctGroupArray[fgIndex].dead[n] + (double)FunctGroupArray[fgIndex].preyEaten[n][WC]) * (VERTinfo[fgIndex][n][SN_id]
									+ VERTinfo[fgIndex][n][RN_id]) / bm->cell_vol;
							if (fabs(valcheck) > maxflux) {
								ans = -fgIndex;
								maxflux = valcheck;
								flux_sp = fgIndex;
								fluxnum = death_id;
								fcnum = n;
							}
						}
					}
					break;
				case AGE_STRUCTURED_BIOMASS:	/* Intentional */
				case BIOMASS:
					for (n = 0; n < FunctGroupArray[fgIndex].numCohortsXnumGenes; n++) {
						if (FunctGroupArray[fgIndex].habitatType == EPIFAUNA){
							if (fabs(localEPIFlux[FunctGroupArray[fgIndex].totNTracers[n]]) > maxflux) {
								ans = FunctGroupArray[fgIndex].totNTracers[n];
								maxflux = localEPIFlux[FunctGroupArray[fgIndex].totNTracers[n]];
								flux_sp = fgIndex;
								fluxnum = EPIFAUNA;
								fcnum = n;
							}
						} else {
							if (fabs(localWCFlux[FunctGroupArray[fgIndex].totNTracers[n]]) > maxflux) {
								ans = FunctGroupArray[fgIndex].totNTracers[n];
								maxflux = localWCFlux[FunctGroupArray[fgIndex].totNTracers[n]];
								fluxnum = WC;
								flux_sp = fgIndex;
								fcnum = n;
							}
							if (fabs(localSEDFlux[FunctGroupArray[fgIndex].totNTracers[n]]) > maxflux) {
								ans = FunctGroupArray[fgIndex].totNTracers[n];
								maxflux = localSEDFlux[FunctGroupArray[fgIndex].totNTracers[n]];
								fluxnum = SED;
								flux_sp = fgIndex;
								fcnum = n;
							}
						}
					}
					break;
				}
			}
		}

		/* If it wasn't one of those then it'll be a nutrient - base it on N, but may need to include Si */
		if (fabs(localWCFlux[NH3_i]) > maxflux) {
			ans = NH3_i;
			maxflux = localWCFlux[NH3_i];
			fluxnum = WC;
			flux_sp = bm->K_num_tot_sp;
		}
		if (fabs(localWCFlux[NO3_i]) > maxflux) {
			ans = NO3_i;
			maxflux = localWCFlux[NO3_i];
			fluxnum = WC;
			flux_sp = bm->K_num_tot_sp;
		}
		if (fabs(localWCFlux[DON_i]) > maxflux) {
			ans = DON_i;
			maxflux = localWCFlux[DON_i];
			fluxnum = WC;
			flux_sp = bm->K_num_tot_sp;
		}
		if (fabs(localSEDFlux[NH3_i]) > maxflux) {
			ans = NH3_i;
			maxflux = localSEDFlux[NH3_i];
			fluxnum = SED;
			flux_sp = bm->K_num_tot_sp;
		}
		if (fabs(localSEDFlux[NO3_i]) > maxflux) {
			ans = NO3_i;
			maxflux = localSEDFlux[NO3_i];
			fluxnum = SED;
			flux_sp = bm->K_num_tot_sp;
		}
		if (fabs(localSEDFlux[DON_i]) > maxflux) {
			ans = DON_i;
			maxflux = localSEDFlux[DON_i];
			fluxnum = SED;
			flux_sp = bm->K_num_tot_sp;
		}
		break;
	default:
		quit("No such callerid defined for Max_Flux_Check - how did it get here?\n");
		break;

	}

	*fluxid = ans;
	*fluxpool = fluxnum;
	*fluxn = fcnum;
	*fluxsp = flux_sp;

	return;

}

/**
 *	\brief Diagnostic outputs
 *	- gives breakdown of what terms make up the NH, DL and DR fluxes
 *
 */
void Call_Diagnostics(MSEBoxModel *bm, BoxLayerValues *boxLayerInfo, int level_id, FILE *llogfp, int qnancall) {
	int i = 0;
	char *loc1 = "wc";
	char *loc2 = "sed";
	char *loc3 = "epi";
	char *location;

	if (level_id == WC)
		location = loc1;
	else if (level_id == SED)
		location = loc2;
	else
		location = loc3;

	if (bm->checkNH || qnancall) {
		/* Output NH */
		fprintf(llogfp, "\nTime: %e, box%d-layer%d, %swcFluxNH: %e (it_count %d)\n", bm->dayt, bm->current_box, bm->current_layer, location,
				boxLayerInfo->DebugInfo[bm->K_num_tot_sp + 1][level_id][DiagnostNH_id], it_count);

		for (i = 0; i < bm->K_num_tot_sp; i++) {
			fprintf(llogfp, "%s%s_wcFluxNH: %e\n", location, FunctGroupArray[i].groupCode, boxLayerInfo->DebugInfo[i][level_id][DiagnostNH_id]);
		}
		fprintf(llogfp, "%sRemin_wcFluxNH: %e\n", location, boxLayerInfo->DebugInfo[bm->K_num_tot_sp][level_id][DiagnostNH_id]);

		fprintf(llogfp, "\nTime: %e, box%d-layer%d, %ssedFluxNH: %e (it_count %d)\n", bm->dayt, bm->current_box, bm->current_layer, location,
				boxLayerInfo->DebugInfo[bm->K_num_tot_sp + 1][level_id][DiagnostNHsed_id], it_count);

		for (i = 0; i < bm->K_num_tot_sp; i++) {
			fprintf(llogfp, "%s%s_sedFluxNH: %e\n", location, FunctGroupArray[i].groupCode, boxLayerInfo->DebugInfo[i][level_id][DiagnostNHsed_id]);
		}
		fprintf(llogfp, "%sRemin_sedFluxNH: %e\n", location, boxLayerInfo->DebugInfo[bm->K_num_tot_sp][level_id][DiagnostNHsed_id]);

	}

	if (bm->checkDL || qnancall) {
		/* Output DL */
		fprintf(llogfp, "\nTime: %e, box%d-layer%d, %swcFluxDL: %e (it_count %d)\n", bm->dayt, bm->current_box, bm->current_layer, location,
				boxLayerInfo->DebugInfo[bm->K_num_tot_sp + 1][level_id][DiagnostDL_id], it_count);
		for (i = 0; i < bm->K_num_tot_sp; i++) {
			fprintf(llogfp, "%s%s_wcFluxDL: %e\n", location, FunctGroupArray[i].groupCode, boxLayerInfo->DebugInfo[i][level_id][DiagnostDL_id]);
		}
		fprintf(llogfp, "%sRemin_wcFluxDL: %e\n", location, boxLayerInfo->DebugInfo[bm->K_num_tot_sp][level_id][DiagnostDL_id]);

		fprintf(llogfp, "\nTime: %e, box%d-layer%d, %ssedFluxDL: %e (it_count %d)\n", bm->dayt, bm->current_box, bm->current_layer, location,
				boxLayerInfo->DebugInfo[bm->K_num_tot_sp + 1][level_id][DiagnostDLsed_id], it_count);
		for (i = 0; i < bm->K_num_tot_sp; i++) {
			fprintf(llogfp, "%s%s_sedFluxDL: %e\n", location, FunctGroupArray[i].groupCode, boxLayerInfo->DebugInfo[i][level_id][DiagnostDLsed_id]);
		}
		fprintf(llogfp, "%sRemin_sedFluxDL: %e\n", location, boxLayerInfo->DebugInfo[bm->K_num_tot_sp][level_id][DiagnostDLsed_id]);

	}

	if (bm->checkDR || qnancall) {
		/* Output DR */
		fprintf(llogfp, "\nTime: %e, box%d-layer%d, %swcFluxDR: %e (it_count %d)\n", bm->dayt, bm->current_box, bm->current_layer, location,
				boxLayerInfo->DebugInfo[bm->K_num_tot_sp + 1][level_id][DiagnostDR_id], it_count);
		for (i = 0; i < bm->K_num_tot_sp; i++) {
			fprintf(llogfp, "%s%s_wcFluxDR: %e\n", location, FunctGroupArray[i].groupCode, boxLayerInfo->DebugInfo[i][level_id][DiagnostDR_id]);
		}
		fprintf(llogfp, "%sRemin_wcFluxDR: %e\n", location, boxLayerInfo->DebugInfo[bm->K_num_tot_sp][level_id][DiagnostDR_id]);

		fprintf(llogfp, "\nTime: %e, box%d-layer%d, %ssedFluxDR: %e (it_count %d)\n", bm->dayt, bm->current_box, bm->current_layer, location,
				boxLayerInfo->DebugInfo[bm->K_num_tot_sp + 1][level_id][DiagnostDRsed_id], it_count);
		for (i = 0; i < bm->K_num_tot_sp; i++) {
			fprintf(llogfp, "%s%s_sedFluxDR: %e\n", location, FunctGroupArray[i].groupCode, boxLayerInfo->DebugInfo[i][level_id][DiagnostDRsed_id]);
		}
		fprintf(llogfp, "%sRemin_sedFluxDR: %e\n", location, boxLayerInfo->DebugInfo[bm->K_num_tot_sp][level_id][DiagnostDRsed_id]);
	}

	return;
}

/**
 * \brief Routine to print out fluxes to log file
 *
 */
void Print_Flux(MSEBoxModel *bm, int level_id, double ***spSPinfo, int qnancheck_id, double wcFlux, double wcFlux1, double wcFlux2, double wcFlux3,
		double wcFlux4, double smFlux, double epiFlux, double TotFlux, double FishingFlux, double *localWCFlux, double *localSEDFlux, double *localEPIFlux,
		FILE *llogfp) {
	int n = 0;
	int fgIndex, index, nf;
	int flagsp;
	int vert_note = 0;

	switch (level_id) {
	case WC:
		fprintf(llogfp, "\n");
		fprintf(llogfp, "day: %.6e, box: %d, layer: %d\n", bm->dayt, bm->current_box, bm->current_layer);

		for (fgIndex = bm->K_num_tot_sp - 1; fgIndex > -1; fgIndex--) {
			flagsp = (int) (FunctGroupArray[fgIndex].speciesParams[flag_id]);
			if (flagsp && FunctGroupArray[fgIndex].habitatCoeffs[WC] > 0  && !FunctGroupArray[fgIndex].isDetritus) {
				switch (FunctGroupArray[fgIndex].groupAgeType) {
				case AGE_STRUCTURED:
					if (it_count == 1) {
						for (n = 0; n < FunctGroupArray[fgIndex].numCohortsXnumGenes; n++) {
							fprintf(llogfp, "%s-%d growth flux: %.20e\n", FunctGroupArray[fgIndex].fullName, n,
									(FunctGroupArray[fgIndex].grow[n][SN_id] + FunctGroupArray[fgIndex].grow[n][RN_id])
											* spSPinfo[fgIndex][n][DEN_id] / bm->cell_vol);
							fprintf(llogfp, "%s-%d mortality flux: %.20Le (eaten: %.20Le)\n", FunctGroupArray[fgIndex].fullName, n,
									(FunctGroupArray[fgIndex].dead[n] + FunctGroupArray[fgIndex].preyEaten[n][WC]) * (spSPinfo[fgIndex][n][SN_id]
											+ spSPinfo[fgIndex][n][RN_id]) / bm->cell_vol, FunctGroupArray[fgIndex].preyEaten[n][WC] * (spSPinfo[fgIndex][n][SN_id]
											+ spSPinfo[fgIndex][n][RN_id]) / bm->cell_vol);
							for (nf = 0; nf < bm->K_num_fisheries; nf++) {
                                if (bm->Catch[bm->current_box][fgIndex][nf][bm->current_layer] > 0)
                                    fprintf(llogfp, "%s-%d fishing %e\n", FisheryArray[nf].fisheryCode, nf, bm->Catch[bm->current_box][fgIndex][nf][bm->current_layer] / (bm->cell_vol * bm->dt + small_num));
							}

							fprintf(llogfp, "%s-%d flux: %.20e\n", FunctGroupArray[fgIndex].fullName, n, localWCFlux[FunctGroupArray[fgIndex].NumsTracers[n]]);

						}
					} else {
						vert_note = 1;
					}
					break;
				case AGE_STRUCTURED_BIOMASS:	/* Intentional */
				case BIOMASS:
					if (FunctGroupArray[fgIndex].numCohorts > 1){
						for (n = 0; n < FunctGroupArray[fgIndex].numCohortsXnumGenes; n++) {
							fprintf(llogfp, "%s-%d (%s-%d) N flux: %.20e\n", FunctGroupArray[fgIndex].fullName, n, FunctGroupArray[fgIndex].groupCode, n,
									localWCFlux[FunctGroupArray[fgIndex].totNTracers[n]]);
						}
					}else{
						fprintf(llogfp, "%s (%s) N flux: %.20e\n", FunctGroupArray[fgIndex].fullName, FunctGroupArray[fgIndex].groupCode,
								localWCFlux[FunctGroupArray[fgIndex].totNTracers[0]]);
					}
					break;
				}
			}
		}

		for (index = 0; index < bm->K_num_physiochem; index++) {
			fprintf(llogfp, "%s N flux: %.20e\n", PhysioChemArray[index].name, localWCFlux[*PhysioChemArray[index].tracerIndex]);
		}
		if (qnancheck_id == qnancheck) {
			fprintf(llogfp,
                    "Print_Flux1: Got an QNAN result box: %d, layer: %d, on day: %.6e, wcFlux: %.6e (wcFlux1: %.6e, wcFlux2: %.6e, wcFlux3: %.6e, wcFishing: %.6e)\n",
					bm->current_box, bm->current_layer, bm->dayt, wcFlux, wcFlux1, wcFlux2, wcFlux3, FishingFlux);
		} else if (qnancheck_id == noqnanverbose) {
			fprintf(llogfp, "box: %d, layer: %d, on day: %.6e, wcFlux: %.6e (wcFlux1: %.6e, wcFlux2: %.6e, wcFlux3: %.6e, wcFishing: %.6e)\n", bm->current_box,
					bm->current_layer, bm->dayt, wcFlux, wcFlux1, wcFlux2, wcFlux3, FishingFlux);
		} else {
			fprintf(llogfp,
					"balance check fail box: %d, layer: %d, on day: %.6e, wcFlux: %.6e (wcFlux1: %.6e, wcFlux2: %.6e, wcFlux3: %.6e, wcFishing: %.6e)\n",
					bm->current_box, bm->current_layer, bm->dayt, wcFlux, wcFlux1, wcFlux2, wcFlux3, FishingFlux);
		}
		fprintf(llogfp, "where wcFlux1 is nutrients and small plankton, wcFlux2 is vertebrates and wcFlux3 is large zooplankton, cephalopods and prawns\n");
		break;
	case SED:

		fprintf(llogfp, "\n");
		fprintf(llogfp, "day: %.6e, box: %d\n", bm->dayt, bm->current_box);

		for (fgIndex = bm->K_num_tot_sp - 1; fgIndex > -1; fgIndex--) {
			flagsp = (int) (FunctGroupArray[fgIndex].speciesParams[flag_id]);
			if (flagsp && FunctGroupArray[fgIndex].habitatCoeffs[SED] > 0 && !FunctGroupArray[fgIndex].isDetritus) {
				switch (FunctGroupArray[fgIndex].groupAgeType) {
				case AGE_STRUCTURED:
					/* Nothing to do here as yet */
					break;
				case AGE_STRUCTURED_BIOMASS:
					for (n = 0; n < FunctGroupArray[fgIndex].numCohortsXnumGenes; n++) {
						fprintf(llogfp, "sed%s-%d (%s-%d) N sedflux: %.20e\n", FunctGroupArray[fgIndex].fullName, n, FunctGroupArray[fgIndex].groupCode, n,
								localSEDFlux[FunctGroupArray[fgIndex].totNTracers[n]]);
					}
					fprintf(llogfp, "WARNING: Could be any of the %d cohorts doing this\n", FunctGroupArray[fgIndex].numCohorts);
					break;
				case BIOMASS:
					fprintf(llogfp, "sed%s (%s) N sedflux: %.20e\n", FunctGroupArray[fgIndex].fullName, FunctGroupArray[fgIndex].groupCode,
							localSEDFlux[FunctGroupArray[fgIndex].totNTracers[0]]);
					break;
				}
			}
		}

		for (index = 0; index < bm->K_num_physiochem; index++) {
			fprintf(llogfp, "%s N sedflux: %.20e, %s*spor = %.20e\n", PhysioChemArray[index].name, localSEDFlux[*PhysioChemArray[index].tracerIndex],
					PhysioChemArray[index].name, localSEDFlux[*PhysioChemArray[index].tracerIndex] * sporosity);
		}

		fprintf(llogfp, "sporosity: %.20e\n", sporosity);
		fprintf(llogfp, "Denitrification flux: %.20e\n", localSEDFlux[Denitrification_i]);

		if (qnancheck_id == qnancheck) {
            fprintf(llogfp, "Print_Flux2: Got an QNAN result box: %d, layer: %d on day: %.6e, smflux: %.6e\n", bm->current_box, bm->current_layer, bm->dayt, smFlux);
		} else if (qnancheck_id == noqnanverbose) {
			fprintf(llogfp, "box: %d, layer: %d on day: %.6e, smflux: %.6e\n", bm->current_box, bm->current_layer, bm->dayt, smFlux);
		} else {
			fprintf(llogfp, "balance check fail box: %d, layer: %d on day: %.6e, smflux: %.6e\n", bm->current_box, bm->current_layer, bm->dayt, smFlux);
		}
		break;
	case EPIFAUNA:
		fprintf(llogfp, "\n");
		fprintf(llogfp, "day: %.6e, box: %d\n", bm->dayt, bm->current_box);

		for (fgIndex = bm->K_num_tot_sp - 1; fgIndex > -1; fgIndex--) {
			flagsp = (int) (FunctGroupArray[fgIndex].speciesParams[flag_id]);
			if (flagsp && FunctGroupArray[fgIndex].habitatCoeffs[WC] > 0 && !FunctGroupArray[fgIndex].isDetritus) {
				switch (FunctGroupArray[fgIndex].groupAgeType) {
				case AGE_STRUCTURED:

					if (it_count == 1) {
						for (n = 0; n < FunctGroupArray[fgIndex].numCohortsXnumGenes; n++) {
							fprintf(llogfp, "%s-%d growth epiflux: %.20e\n", FunctGroupArray[fgIndex].fullName, n,
									(FunctGroupArray[fgIndex].grow[n][SN_id] + FunctGroupArray[fgIndex].grow[n][RN_id])
											* spSPinfo[fgIndex][n][DEN_id] / bm->cell_vol);
							fprintf(llogfp, "%s-%d mortality epiflux: %.20Le (eaten: %.20Le)\n", FunctGroupArray[fgIndex].fullName, n,
									(FunctGroupArray[fgIndex].dead[n] + FunctGroupArray[fgIndex].preyEaten[n][WC]) * (spSPinfo[fgIndex][n][SN_id]
											+ spSPinfo[fgIndex][n][RN_id]) / bm->cell_vol, FunctGroupArray[fgIndex].preyEaten[n][WC] * (spSPinfo[fgIndex][n][SN_id]
											+ spSPinfo[fgIndex][n][RN_id]) / bm->cell_vol);

							fprintf(llogfp, "%s-%d epi wcflux: %.20e\n", FunctGroupArray[fgIndex].fullName, n, localWCFlux[FunctGroupArray[fgIndex].NumsTracers[n]]);

						}
					} else {
						vert_note = 1;
					}
					break;
				case AGE_STRUCTURED_BIOMASS:
					for (n = 0; n < FunctGroupArray[fgIndex].numCohortsXnumGenes; n++) {
						fprintf(llogfp, "%s-%d (%s-%d) N epi wcflux: %.20e\n", FunctGroupArray[fgIndex].fullName, n, FunctGroupArray[fgIndex].groupCode, n,
								localWCFlux[FunctGroupArray[fgIndex].totNTracers[n]]);
					}

					break;
				case BIOMASS:

					fprintf(llogfp, "%s (%s) N epi wcflux: %.20e\n", FunctGroupArray[fgIndex].fullName, FunctGroupArray[fgIndex].groupCode,
							localWCFlux[FunctGroupArray[fgIndex].totNTracers[0]]);
					break;
				}
			} else if (flagsp && FunctGroupArray[fgIndex].habitatCoeffs[SED] > 0 && !FunctGroupArray[fgIndex].isDetritus) {
				switch (FunctGroupArray[fgIndex].groupAgeType) {
				case AGE_STRUCTURED:
					/* Nothing to do here as yet */
					break;
				case AGE_STRUCTURED_BIOMASS:
					for (n = 0; n < FunctGroupArray[fgIndex].numCohortsXnumGenes; n++) {
						fprintf(llogfp, "sed%s-%d (%s-%d) N epi sedflux: %.20e\n", FunctGroupArray[fgIndex].fullName, n, FunctGroupArray[fgIndex].groupCode, n,
								localSEDFlux[FunctGroupArray[fgIndex].totNTracers[n]]);
					}

					break;
				case BIOMASS:

					fprintf(llogfp, "sed%s (%s) N epi sedflux: %.20e\n", FunctGroupArray[fgIndex].fullName, FunctGroupArray[fgIndex].groupCode,
							localSEDFlux[FunctGroupArray[fgIndex].totNTracers[0]]);
					break;
				}
			} else if (flagsp && FunctGroupArray[fgIndex].habitatCoeffs[EPIFAUNA] > 0 && !FunctGroupArray[fgIndex].isDetritus) {
				switch (FunctGroupArray[fgIndex].groupAgeType) {
				case AGE_STRUCTURED:
					/* Nothing to do here as yet */
					fprintf(llogfp, "No 2D age structured groups allowed as yet - check and rewrite this code if now untrue\n");
					break;
				case AGE_STRUCTURED_BIOMASS:	/* Intentional */
				case BIOMASS:
					if(FunctGroupArray[fgIndex].numCohorts > 0){
						for (n = 0; n < FunctGroupArray[fgIndex].numCohortsXnumGenes; n++) {
							fprintf(llogfp, "%s-%d (%s-%d) N epiflux: %.20e\n", FunctGroupArray[fgIndex].fullName, n, FunctGroupArray[fgIndex].groupCode, n,
									localEPIFlux[FunctGroupArray[fgIndex].totNTracers[n]]);
						}
					} else {
						fprintf(llogfp, "%s (%s) N epiflux: %.20e\n", FunctGroupArray[fgIndex].fullName, FunctGroupArray[fgIndex].groupCode,
								localEPIFlux[FunctGroupArray[fgIndex].totNTracers[0]]);
					}
					break;
				}
			}
		}

		for (index = 0; index < bm->K_num_physiochem; index++) {
			fprintf(llogfp, "epi %s N wc flux: %.20e\n", PhysioChemArray[index].name, localWCFlux[*PhysioChemArray[index].tracerIndex]);
		}

		for (index = 0; index < bm->K_num_physiochem; index++) {
			fprintf(llogfp, "epi %s N sedflux: %.20e, %s*spor = %.20e\n", PhysioChemArray[index].name, localSEDFlux[*PhysioChemArray[index].tracerIndex],
					PhysioChemArray[index].name, localSEDFlux[*PhysioChemArray[index].tracerIndex] * sporosity);
		}

		if (vert_note) {
			fprintf(llogfp, "Imbalance did not include vertebrates in this case\n");
		}

		if (qnancheck_id == qnancheck) {
            fprintf(llogfp, "Print_Flux3: Got an QNAN result box: %d, on day: %.6e, wcflux: %.6e, smflux: %.6e, epiflux: %.6e, Totflux: %.6e (with wcFishing: %.6e)\n",
					bm->current_box, bm->dayt, wcFlux, smFlux, epiFlux, TotFlux, FishingFlux);
		} else if (qnancheck_id == noqnanverbose) {
			fprintf(
					llogfp,
					"box: %d, on day: %.6e, wcflux: %.6e (wcFlux1: %.6e, wcFlux2: %.6e, wcFlux3: %.6e, wcFlux4: %.6e), smflux: %.6e, epiflux: %.6e (epiFishing: %.6e), Totflux: %.6e\n",
					bm->current_box, bm->dayt, wcFlux, wcFlux1, wcFlux2, wcFlux3, wcFlux4, smFlux, epiFlux, FishingFlux, TotFlux);
		} else {
			fprintf(
					llogfp,
					"balance check fail box: %d, on day: %.6e, wcflux: %.6e (wcFlux1: %.6e, wcFlux2: %.6e, wcFlux3: %.6e, wcFlux4: %.6e), smflux: %.6e, epiflux: %.6e (epiFishing: %.6e), Totflux: %.6e\n",
					bm->current_box, bm->dayt, wcFlux, wcFlux1, wcFlux2, wcFlux3, wcFlux4, smFlux, epiFlux, FishingFlux, TotFlux);
		}
		fprintf(llogfp,
				"where wcFlux1 is nutrients and small plankton, wcFlux2 is vertebrates, wcFlux3 is cephalopods and prawns, wcFlux4 is large zooplankton\n");
		fprintf(llogfp, "and smFlux1 is nutrients and small plankton, smFlux2 is infauna, epiFlux1 is macrophytes and epiFlux2 is epibenthos\n");
		break;
	}

	/* Close the log file if failed balance check or failed qnan check - moved to atlantismain.c as other things to do first.
	 if(qnancheck_id != noqnanverbose){
	 fclose(llogfp);
	 }
	 */

	return;
}

/**
 *  Print the GRAZEInfo values for diagnostic purposes.
 *
 */
static void Print_GrazeInfo_Values(MSEBoxModel *bm, FILE *llogfp, int guild, int habitat, HABITAT_TYPES habitatType) {

	int cohort;

	for (cohort = 0; cohort < FunctGroupArray[guild].numCohorts * FunctGroupArray[guild].numGeneTypes; cohort++) {
		switch (habitat) {
		case WC:
			fprintf(llogfp, "grazeInfo %s (%d) wc %.14e\n", FunctGroupArray[guild].groupCode, guild, GRAZEinfo[guild][cohort][habitat]);
			break;
		case SED:
			fprintf(llogfp, "grazeInfo %s (%d) sed %.14e\n", FunctGroupArray[guild].groupCode, guild, GRAZEinfo[guild][cohort][habitat]);
			break;
		case EPIFAUNA:
			fprintf(llogfp, "grazeInfo %s (%d) epi %.14e\n", FunctGroupArray[guild].groupCode, guild, GRAZEinfo[guild][cohort][habitat]);
			break;
		}
	}
}

/**
 * Print the spEat values
 *
 *
 */
static void Print_SPEAT_Value(MSEBoxModel *bm, FILE *llogfp, int guild, int habitat, HABITAT_TYPES habitatType) {
	int prey;
	int cohort;
	int stage;

	for (cohort = 0; cohort < FunctGroupArray[guild].numCohorts * FunctGroupArray[guild].numGeneTypes; cohort++) {
		stage = FunctGroupArray[guild].cohort_stage[cohort];
		switch (habitat) {
		case WC:
			for (prey = 0; prey < bm->K_num_tot_sp; prey++)
				if (strcmp(FunctGroupArray[prey].name, "") != 0)
					if(FunctGroupArray[guild].pSPEat[stage][prey][WC] > 0)
						fprintf(llogfp, "wc %s-%d, %s, spEat = %e\n", FunctGroupArray[guild].groupCode, cohort, FunctGroupArray[prey].groupCode, FunctGroupArray[guild].pSPEat[stage][prey][WC]);
			break;
		case SED:
			for (prey = 0; prey < bm->K_num_tot_sp; prey++)
				if (strcmp(FunctGroupArray[prey].name, "") != 0)
					if(FunctGroupArray[guild].pSPEat[stage][prey][SED] > 0)
						fprintf(llogfp, "sed %s-%d, %s, spEat = %e\n", FunctGroupArray[guild].groupCode, cohort, FunctGroupArray[prey].groupCode,  FunctGroupArray[guild].pSPEat[stage][prey][SED]);

			break;
		case EPIFAUNA:
			for (prey = 0; prey < bm->K_num_tot_sp; prey++) {
				if (strcmp(FunctGroupArray[prey].name, "") != 0)
					if(FunctGroupArray[guild].pSPEat[stage][prey][EPIFAUNA] > 0)
						fprintf(llogfp, "epi %s-%d, %s, spEat = %e\n", FunctGroupArray[guild].groupCode, cohort, FunctGroupArray[prey].groupCode, FunctGroupArray[guild].pSPEat[stage][prey][EPIFAUNA]);
			}
			break;
		}
	}
}

/**
 * Print the preyEaten and preyEatenGlobal values.
 *
 *
 */
static void Print_PreyEaten_Value(MSEBoxModel *bm, FILE *llogfp, int guild, int habitat, HABITAT_TYPES habitatType) {
	int cohort;

	for (cohort = 0; cohort < FunctGroupArray[guild].numCohorts * FunctGroupArray[guild].numGeneTypes; cohort++)
		switch (habitat) {
		case WC:
			fprintf(llogfp, "preyEaten %s (%d) wc %.14Le\n", FunctGroupArray[guild].groupCode, guild, FunctGroupArray[guild].preyEaten[cohort][habitat]);
			fprintf(llogfp, "preyEatenGlobal %s (%d) wc %.14Le\n", FunctGroupArray[guild].groupCode, guild,
					FunctGroupArray[guild].preyEatenGlobal[cohort][habitatType][habitat]);
			break;
		case SED:
			fprintf(llogfp, "preyEaten %s (%d) sed %.14Le\n", FunctGroupArray[guild].groupCode, guild, FunctGroupArray[guild].preyEaten[cohort][habitat]);
			fprintf(llogfp, "preyEatenGlobal %s (%d) sed %.14Le\n", FunctGroupArray[guild].groupCode, guild,
					FunctGroupArray[guild].preyEatenGlobal[cohort][habitatType][habitat]);
			break;
		case EPIFAUNA:
			fprintf(llogfp, "preyEaten %s (%d) epi %.14Le\n", FunctGroupArray[guild].groupCode, guild, FunctGroupArray[guild].preyEaten[cohort][habitat]);
			fprintf(llogfp, "preyEatenGlobal %s (%d) epi %.14Le\n", FunctGroupArray[guild].groupCode, guild,
					FunctGroupArray[guild].preyEatenGlobal[cohort][habitatType][habitat]);
			break;
		}
}

/**
 * Print the preyInfo array.
 *
 *
 */
static void Print_PreyInfo_Value(MSEBoxModel *bm, FILE *llogfp, int guild, int habitat, HABITAT_TYPES habitatType) {
	int cohort;

	for (cohort = 0; cohort < FunctGroupArray[guild].numCohorts * FunctGroupArray[guild].numGeneTypes; cohort++) {
		switch (habitat) {
		case WC:
			fprintf(llogfp, "preyValue %s (%d) wc %.14e\n", FunctGroupArray[guild].groupCode, guild, PREYinfo[guild][cohort][habitat]);
			break;
		case SED:
			fprintf(llogfp, "preyValue %s (%d) sed %.14e\n", FunctGroupArray[guild].groupCode, guild, PREYinfo[guild][cohort][habitat]);
			break;
		case EPIFAUNA:
			fprintf(llogfp, "preyValue %s (%d) epi %.14e\n", FunctGroupArray[guild].groupCode, guild, PREYinfo[guild][cohort][habitat]);
			break;
		}
	}
}

/**
 * Print the vertebrate prey information - this is the data stored in the VERTinfo array.
 *
 *
 */
void Print_VertPreyInfo_Value(MSEBoxModel *bm, FILE *llogfp, int guild, int habitat, HABITAT_TYPES habitatType) {
	int cohort;

	for (cohort = 0; cohort < FunctGroupArray[guild].numCohorts * FunctGroupArray[guild].numGeneTypes; cohort++) {
		switch (habitat) {
		case WC:
			fprintf(llogfp, "preyValue %s (%d:%d) wc %.14e\n", FunctGroupArray[guild].groupCode, guild, cohort, VERTinfo[guild][cohort][habitat]);
			break;
		case SED:
			fprintf(llogfp, "preyValue %s (%d:%d) sed %.14e\n", FunctGroupArray[guild].groupCode, guild, cohort, VERTinfo[guild][cohort][habitat]);
			break;
		case EPIFAUNA:
			fprintf(llogfp, "preyValue %s (%d:%d) epi %.14e\n", FunctGroupArray[guild].groupCode, guild, cohort, VERTinfo[guild][cohort][habitat]);
			break;
		}
	}
}

/**
 * Print the EATINGinfo array.
 *
 *
 */
static void Print_EATINGinfo_Value(MSEBoxModel *bm, FILE *llogfp, int guild, int habitat, HABITAT_TYPES habitatType) {
	int cohort;

	for (cohort = 0; cohort < FunctGroupArray[guild].numCohorts * FunctGroupArray[guild].numGeneTypes; cohort++) {
		switch (habitat) {
		case WC:
			fprintf(llogfp, "preyAvailValue %s (%d) wc %.14e\n", FunctGroupArray[guild].groupCode, guild, EATINGinfo[guild][cohort][habitat]);
			break;
		case SED:
			fprintf(llogfp, "preyAvailValue %s (%d) sed %.14e\n", FunctGroupArray[guild].groupCode, guild, EATINGinfo[guild][cohort][habitat]);
			break;
		case EPIFAUNA:
			fprintf(llogfp, "preyAvailValue %s (%d) epi %.14e\n", FunctGroupArray[guild].groupCode, guild, EATINGinfo[guild][cohort][habitat]);
			break;
		}
	}
}

/**
 * \brief - diagnostic output.
 *
 *
 * diagType:
 *
 * 0 - print out the values in the PreyInfo array.
 * 1 - print out the values in the FunctGroupArray[guild].preyEaten[0][habitat].
 * 2 - print out the values in the GRAZEInfo array.
 * 3 - print out the values in the spEAT array.
 * 4 - print out the prey availability values.
 *
 *
 *
 *
 */
void Print_Eat_Diagnostics(MSEBoxModel *bm, FILE *llogfp, int guild, HABITAT_TYPES habitatType, int diagType) {
	int sp;
	void (*fp)(MSEBoxModel *bm, FILE *llogfp, int guild, int habitat, HABITAT_TYPES habitatType) = NULL; // Function pointer

	switch (diagType) {
	case 0:
		fp = Print_PreyInfo_Value;
		fprintf(logfp, "Prey info values\n");
		break;
	case 1:
		fp = Print_PreyEaten_Value;
		fprintf(logfp, "Prey eaten values after %d, %s\n", guild, FunctGroupArray[guild].groupCode);
		break;
	case 2:
		fp = Print_GrazeInfo_Values;
		fprintf(logfp, "GrazeInfo values after %d, %s\n", guild, FunctGroupArray[guild].groupCode);
		break;
	case 3:
		fp = Print_SPEAT_Value;
		fprintf(logfp, "spEat values\n");
		break;
	case 4:
		fp = Print_EATINGinfo_Value;
		fprintf(logfp, "EATINGinfo values\n");
		break;
	}

	switch (habitatType) {
	case WC:
		for (sp = 0; sp < bm->K_num_tot_sp; sp++) {
			if (FunctGroupArray[sp].isVertebrate == FALSE && FunctGroupArray[sp].habitatCoeffs[WC] > 0) {
				fp(bm, llogfp, sp, WC, habitatType);
			}
		}
		break;
	case SED:
		for (sp = 0; sp < bm->K_num_tot_sp; sp++) {
			if (FunctGroupArray[sp].isVertebrate == FALSE && FunctGroupArray[sp].habitatCoeffs[SED] > 0) {
				fp(bm, llogfp, sp, SED, habitatType);
			}
		}
		break;

	case EPIFAUNA:
		for (sp = 0; sp < bm->K_num_tot_sp; sp++) {
			if (FunctGroupArray[sp].isVertebrate == FALSE) {
				if (FunctGroupArray[sp].habitatCoeffs[WC] > 0) {
					fp(bm, llogfp, sp, WC, habitatType);
				}
			}
		}
		for (sp = 0; sp < bm->K_num_tot_sp; sp++) {
			if (FunctGroupArray[sp].isVertebrate == FALSE) {
				if (FunctGroupArray[sp].habitatCoeffs[SED] > 0 && FunctGroupArray[sp].groupType != LG_PHY && FunctGroupArray[sp].groupType != SM_PHY
						&& FunctGroupArray[sp].groupType != DINOFLAG) {
					fp(bm, llogfp, sp, SED, habitatType);
				}
			}
		}
		for (sp = 0; sp < bm->K_num_tot_sp; sp++) {
			if (FunctGroupArray[sp].isVertebrate == FALSE) {
				if (FunctGroupArray[sp].habitatType == EPIFAUNA){
					fp(bm, llogfp, sp, EPIFAUNA, habitatType);
				}
			}
		}

		break;
	case ICE_BASED:
		for (sp = 0; sp < bm->K_num_tot_sp; sp++) {
			if (FunctGroupArray[sp].isVertebrate == FALSE && FunctGroupArray[sp].habitatCoeffs[ICE_BASED] > 0) {
				fp(bm, llogfp, sp, ICE_BASED, habitatType);
			}
		}
		break;
	case LAND_BASED:
		for (sp = 0; sp < bm->K_num_tot_sp; sp++) {
			if (FunctGroupArray[sp].isVertebrate == FALSE && FunctGroupArray[sp].habitatCoeffs[LAND_BASED] > 0) {
				fp(bm, llogfp, sp, LAND_BASED, habitatType);
			}
		}
        break;
    case MIXED:
        quit("How did we get here as should come through a primary habitat\n");
        break;
    default:
        quit("How did we get here as this isn't even a habitat type slot but nTotHabTypes\n");
        break;
	}

}
