/**
 \file
 \brief Routines to set-up namelists and read in netcdf file into data arrays.
 \ingroup atassess

 File:           atdatareading.c

 Created:        8/8/2004

 Author:         Beth Fulton

 Purpose:        Routines to set-up namelists and read in netcdf file into data arrays.
 Modified from general bay version of sampDemersal_DC model.

 Arguments:      bm - Pointer to box model.

 Returns:        void

 Revisions:      8/8/2004 Created from southeast datareading.c

 17/9/2004 Removed read-in of catch and effort data
 as they are taken dynamically from arrays created in
 the management and ecology libraries and stored in
 the core boxmodel structure

 21/9/2004 Created atdatareading.c from mseecom.c. And renamed
 FSL, FSO, FSF, FST and FSG to generic names so apply to
 other models/generic case.
 FSL -> FDC  (Ling -> Demersal_DC_Fish)
 FSO -> FDO  (Orange_Roughy -> Demersal_O_Fish)
 FSF -> FDB  (Flathead -> Demersal_B_Fish)
 FST -> FVO  (Bluefin_Tuna -> Pisciv_O_Fish)
 FSG -> SHB  (Gummy_Shark -> Shark_B)

 7/10/2004 Added readin of data for additional bioloigcal groups
 (PWN, BFF, BML, FBP, FDE, FDF, FVB, SHR, SSK, REP, SP, WDG)

 19/1/2005 Added Microphytobenthos (MB)

 10/6/2005 Added FDM, FDP and WHS

 27/8/2005 Updated PopulateArrays() so pointing at tracer, diagnostic
 and fisheries statistics lists as appropriate (used to all be in
 tracers but split out now and made more explicit)

 19-06-2008 Bec Gorton
 Fixed the code:
 biolprod[MBinv_id][b] += bm->diagnost[b][FunctGroupArray[MBguild_id].ProdnTracers[0]] * sed_dz;
 was using non guild values.

 14-05-2010 Bec Gorton
 Merged in trunk changes 1558.

 14-05-2012 Beth Fulton
 Added stuff for handling Savina style seagrass model

 11-07-2012 Bec Gorton
 Updated code to allow for new seagrass cohorts.

 *********************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <math.h>
#include <sjwlib.h>
#include <string.h>
#include <netcdf.h>
#include "atsample.h"

/**
 * \brief  This initialises the data array
 */
void Init_Data_Array(MSEBoxModel *bm) {
	int b, k, j, chrt;

	if ((verbose > 1))
		printf("Initialise data arrays\n");

	for (b = 0; b < bm->nbox; b++) {
		for (k = 0; k < 3; k++)
			physprocess[k][b] = 0.0;
		for (k = 0; k < num_sampled_phy_id; k++)
			phys[k][b] = 0.0;
		for (k = 0; k < bm->K_num_tot_sp; k++) {
			biolprod[k][b] = 0.0;
			biolbiom[k][b][WC] = 0.0;
			biolbiom[k][b][SED] = 0.0;
		}
		// TOD: Check with Beth about the bacteria being included in the biomeat arrays.
		for (k = 0; k < bm->K_num_tot_sp; k++) {
			if (FunctGroupArray[k].isVertebrate == FALSE && (FunctGroupArray[k].isPredator == TRUE || FunctGroupArray[k].isBacteria == TRUE)) {
				bioleat[k][b] = 0.0;
			}
		}

		for (k = 0; k < bm->K_num_tot_sp; k++) {
			if (FunctGroupArray[k].isVertebrate == TRUE) {
				for (chrt = 0; chrt < FunctGroupArray[k].numCohortsXnumGenes; chrt++) {
					for (j = 0; j < 7; j++) {
						biolVERTinfo[j][k][chrt][b] = 0.0;
					}
				}
			}
		}
	}

	return;
}

/* Move water column, sediment and epibenthic data from storage to data arrays
 with assumed data structure of

 biolbiom[PLinv_id][box] = average (mg m-2) diatoms in water column
 biolbiom[PSinv_id][box] = average (mg m-2) picoplankton in water column
 biolbiom[DFinv_id][box] = average (mg m-2) dinoflagellates in water column
 biolbiom[ZSinv_id][box] = average (mg m-2) microzooplankton in water column
 biolbiom[ZMinv_id][box] = average (mg m-2) mesozooplankton in water column
 biolbiom[ZLinv_id][box] = average (mg m-2) carnivorous mesozooplankton in water column
 biolbiom[ZGinv_id][box] = average (mg m-2) gelatinous zooplankton in water column
 biolbiom[CEPinv_id][box] = average (mg m-2) cephalopods in water column
 biolbiom[PWNinv_id][box] = average (mg m-2) prawns in water column
 biolbiom[PBinv_id][box] = average (mg m-2) pelagic bacteria in water column
 biolbiom[BBinv_id][box] = average (mg m-2) sediment bacteria in sediment
 biolbiom[BOinv_id][box] = average (mg m-2) meibenthos in sediment
 biolbiom[BDinv_id][box] = average (mg m-2) benthic deposit feeders in sediment
 biolbiom[BCinv_id][box] = average (mg m-2) benthic infaunal carnivores in sediment
 biolbiom[MBinv_id][box] = average (mg m-2) microphytobenthos in sediment
 biolbiom[BFSinv_id][box] = average (mg m-2) shallow filter feeders on sediment
 biolbiom[BFFinv_id][box] = average (mg m-2) other filter feeders on sediment
 biolbiom[BFDinv_id][box] = average (mg m-2) deep filter feeders on sediment
 biolbiom[BGinv_id][box] = average (mg m-2) benthic grazers on sediment
 biolbiom[BMSinv_id][box] = average (mg m-2) shallow macrozoobenthos on sediment
 biolbiom[BMLinv_id][box] = average (mg m-2) megazoobenthos on sediment
 biolbiom[BMDinv_id][box] = average (mg m-2) deep macrozoobenthos on sediment
 biolbiom[MAinv_id][box] = average (mg m-2) macroalgae on sediment
 biolbiom[SGinv_id][box] = average (mg m-2) seagrass on sediment
 biolbiom[DLinv_id][box] = average (mg m-2) labile detritus in water column
 biolbiom[DCinv_id][box] = average (mg m-2) carrion in water column
 biolbiom[DRinv_id][box] = average (mg m-2) refractory detritus in water column
 biolbiom[DLsedinv_id][box] = average (mg m-2) labile detritus in sediment
 biolbiom[DCsedinv_id][box] = average (mg m-2) carrion in sediment
 biolbiom[DRsedinv_id][box] = average (mg m-2) refractory detritus in sediment

 biolprod[PLinv_id][box] = total production (mg m-2) diatoms in water column
 biolprod[PSinv_id][box] = total production (mg m-2) picoplankton in water column
 biolprod[DFinv_id][box] = total production (mg m-2) dinoflagellates in water column
 biolprod[ZSinv_id][box] = total production (mg m-2) microzooplankton in water column
 biolprod[ZMinv_id][box] = total production (mg m-2) mesozooplankton in water column
 biolprod[ZLinv_id][box] = total production (mg m-2) carnivorous mesozooplankton in water column
 biolprod[ZGinv_id][box] = total production (mg m-2) gelatinous zooplankton in water column
 biolprod[CEPinv_id][box] = total production (mg m-2) cephalopods in water column
 biolprod[PWNinv_id][box] = total production (mg m-2) prawns in water column
 biolprod[PBinv_id][box] = total production (mg m-2) pelagic bacteria in water column
 biolprod[BBinv_id][box] = total production (mg m-2) sediment bacteria in sediment
 biolprod[BOinv_id][box] = total production (mg m-2) meibenthos in sediment
 biolprod[BDinv_id][box] = total production (mg m-2) benthic deposit feeders in sediment
 biolprod[BCinv_id][box] = total production (mg m-2) benthic infaunal carnivores in sediment
 biolprod[MBinv_id][box] = total production (mg m-2) microphytobenthos in sediment
 biolprod[BFSinv_id][box] = total production (mg m-2) shallow filter feeders on sediment
 biolprod[BFFinv_id][box] = total production (mg m-2) other filter feeders on sediment
 biolprod[BFDinv_id][box] = total production (mg m-2) deep filter feeders on sediment
 biolprod[BGinv_id][box] = total production (mg m-2) benthic grazers on sediment
 biolprod[BMSinv_id][box] = total production (mg m-2) shallow macrozoobenthos on sediment
 biolprod[BMLinv_id][box] = total production (mg m-2) megazoobenthos on sediment
 biolprod[BMDinv_id][box] = total production (mg m-2) deep macrozoobenthos on sediment
 biolprod[MAinv_id][box] = total production (mg m-2) macroalgae on sediment
 biolprod[SGinv_id][box] = total production (mg m-2) seagrass on sediment

 bioleat[ZSeat_id][box] = total grazing (mg m-2) microzooplankton in water column
 bioleat[ZMeat_id][box] = total grazing (mg m-2) mesozooplankton in water column
 bioleat[ZLeat_id][box] = total grazing (mg m-2) carnivorous mesozooplankton in water column
 bioleat[ZGeat_id][box] = total grazing (mg m-2) gelatinous zooplankton in water column
 bioleat[CEPeat_id][box] = total grazing (mg m-2) cephalopods in water column
 bioleat[PWNeat_id][box] = total grazing (mg m-2) prawns in water column
 bioleat[PBeat_id][box] = total grazing (mg m-2) pelagic bacteria in water column
 bioleat[BBeat_id][box] = total grazing (mg m-2) sediment bacteria in sediment
 bioleat[BOeat_id][box] = total grazing (mg m-2) meibenthos in sediment
 bioleat[BDeat_id][box] = total grazing (mg m-2) benthic deposit feeders in sediment
 bioleat[BCeat_id][box] = total grazing (mg m-2) benthic infaunal carnivores in sediment
 bioleat[BFSeat_id][box] = total grazing (mg m-2) shallow filter feeders on sediment
 bioleat[BFFeat_id][box] = total grazing (mg m-2) other filter feeders on sediment
 bioleat[BFDeat_id][box] = total grazing (mg m-2) deep filter feeders on sediment
 bioleat[BFDeat_id][box] = total grazing (mg m-2) benthic filter feeders on sediment
 bioleat[BMSeat_id][box] = total grazing (mg m-2) shallow macrozoobenthos on sediment
 bioleat[BMLeat_id][box] = total grazing (mg m-2) megazoobenthos on sediment
 bioleat[BMDeat_id][box] = total grazing (mg m-2) deep macrozoobenthos on sediment

 biolVERTinfo[0][group][cohort][box] = structural weight (mg) for vertebrate group VERT
 biolVERTinfo[1][group][cohort][box] = reserve weight (mg) for vertebrate group VERT
 biolVERTinfo[2][group][cohort][box] = numbers in the stock for vertebrate group VERT
 biolVERTinfo[3][group][cohort][box] = numbers in the catch for vertebrate group VERT
 biolVERTinfo[4][group][cohort][box] = production (mg m-2) for vertebrate group VERT
 biolVERTinfo[5][group][cohort][box] = consumption (mg m-2) for vertebrate group VERT
 biolVERTinfo[6][group][cohort][box] = discards (mg m-2) for vertebrate group XX

 CumCatch[fishery][group][box][layer] = total catch (mg m-2) of group
 CumDiscards[fishery][group][box] = total discards (mg m-2) of group
 CumEffort[fishery][box] = total effort of fishery

 Note:
 1) Group and fishery indexes as read in from the functional group input file.
 2) Only collect fisheries independent data if appropriate

 */

void Populate_Arrays(MSEBoxModel *bm, FILE *ofp)
/* Transfers read in data from netcdf file to appropriate data arrays
 As assumes only want info per area, with no consideration of depth,
 values are read in and averaged or totalled ignoring depth structure.
 */
{
	int b, k, z, ij, nf;
	double sed_dz;
	int sp, cohort, stage;

	for (b = 0; b < bm->nbox; b++) {
		if (bm->boxes[b].type != BOUNDARY) {
			//smtopk = bm->boxes[b].sm.topk;

			/* Get areas and depths */
			sed_dz = 0;
			for (k = 0; k < bm->boxes[b].sm.nz; k++) {
				sed_dz += bm->boxes[b].sm.dz[k];
			}
			z = bm->boxes[b].zone - 1;
			zonearea[z] += bm->boxes[b].area;

			/* Move physical data from storage to data arrays
			 with assumed data structure of

			 phys[0][box][record_in_time] = water column depth
			 phys[1][box][record_in_time] = sediment depth
			 phys[2][box][record_in_time] = saltinity
			 phys[3][box][record_in_time] = light in water column
			 phys[4][box][record_in_time] = light at sediment surface
			 phys[5][box][record_in_time] = oxygen in sediment
			 phys[6][box][record_in_time] = don in water column
			 phys[7][box][record_in_time] = din in water column
			 phys[8][box][record_in_time] = volume
			 phys[9][box][record_in_time] = temperature
			 phys[10][box][record_in_time] = TSS
			 phys[11][box][record_in_time] = Nitrogen in water column

			 Note that identity of samplephys[i] = phys[i-2]
			 so samplephys[0] = phys[2] = salinity
			 etc

			 */
			for (k = 0; k < bm->boxes[b].nz; k++) {
				phys[wc_depth_id][b] += bm->boxes[b].dz[k];
			}

			if (bm->flagecosysassess) {
				for (k = 0; k < bm->boxes[b].sm.nz; k++) {
					phys[sed_depth_id][b] += bm->boxes[b].sm.dz[k];
					phys[light_sed_id][b] += bm->sedtr[b][k][Light_i];
					phys[o2_sed_id][b] += bm->sedtr[b][k][Oxygen_i];
				}

				for (k = 0; k < bm->wcnz; k++) {
					phys[salinity_id][b] += bm->wctr[b][k][Salinity_i];
					phys[light_wc_id][b] += bm->wctr[b][k][Light_i];
					phys[don_wc_id][b] += bm->wctr[b][k][DON_i];
					phys[din_wc_id][b] += (bm->wctr[b][k][NH3_i] + bm->wctr[b][k][NO3_i]);
					phys[vol_id][b] += bm->boxes[b].volume[k];
					phys[temperature_id][b] += bm->wctr[b][k][Temp_i];
					phys[o2_wc_id][b] += bm->wctr[b][k][Oxygen_i];
                    
                    if(bm->flagIsEstuary)
                        phys[TSS_id][b] += bm->wctr[b][k][SED_i];
                    
				}

				/* If verbose enough read back the data values */
				if (verbose > 2) {
					for (k = 0; k < 9; k++)
						fprintf(ofp, "phys[%d][%d]: %e\n", k, b, phys[k][b]);
				}

				/* Move physical process data from storage to data arrays
				 with assumed data structure of

				 physprocess[0][box][record_in_time] = total denitrification (mg m-2) in sediment
				 physprocess[1][box][record_in_time] = total nitrification (mg m-2) in sediment
				 physprocess[2][box][record_in_time] = average chlorophyll a (mg m-3) in water column

				 Note that identity of sampleprocess[i] = physprocess[i]
				 so sampleprocess[0] = physprocess[0] = total deintirifcation
				 etc

				 */
				for (k = 0; k < bm->sednz; k++) {
					physprocess[0][b] += bm->sedtr[b][k][Denitrification_i];
					physprocess[1][b] += bm->sedtr[b][k][Nitrification_i];
				}

				for (k = 0; k < bm->wcnz; k++)
					physprocess[2][b] += bm->wctr[b][k][Chl_a_i] * (bm->boxes[b].dz[k] / (phys[wc_depth_id][b] + TINY));

				if (verbose > 2) {
					for (k = 0; k < 3; k++)
						fprintf(ofp, "physprocess[%d][%d]: %e\n", k, b, physprocess[k][b]);
				}
			}

			/**
			 Water column biomass data - water column microphytobenthos omitted
			 */
			for (sp = 0; sp < bm->K_num_tot_sp; sp++) {
				if (FunctGroupArray[sp].isVertebrate == FALSE) {
					double sum = 0;

					switch (FunctGroupArray[sp].groupType) {
					case NUM_GROUP_TYPES:
						/* Ignore - added to make sure we cover all groups */
						break;
					case BIRD:
					case MAMMAL:
					case SHARK:
					case FISH:
                    case FISH_INVERT:
						/* ignore the vertebrates and age structured fished invertebrates (for now) */
						break;
					case SED_BACT:
					case SM_INF:
					case LG_INF:
					case MICROPHTYBENTHOS:
						if (bm->flagecosysassess)
							/* Sediment biomass data */
							for (k = 0; k < bm->sednz; k++)
								biolbiom[sp][b][SED] += bm->sedtr[b][k][FunctGroupArray[sp].totNTracers[0]] * bm->boxes[b].sm.dz[k];

						break;

					case SEAGRASS:

						if (bm->flag_macro_model) {
							/* Epibenthic biomass data */
							biolbiom[sp][b][WC] += bm->epi[b][FunctGroupArray[sp].totNTracers[main_biomass_id]];
							biolbiom[sp][b][WC] += bm->epi[b][FunctGroupArray[sp].totNTracers[below_ground_biomass_id]];
							biolbiom[sp][b][WC] += bm->epi[b][FunctGroupArray[sp].totNTracers[epiphyte_biomass_id]];
						}else{
							biolbiom[sp][b][WC] += bm->epi[b][FunctGroupArray[sp].totNTracers[0]];
						}
						break;

					case PHYTOBEN:
					case TURF:
						if (bm->flagecosysassess) {
							if (FunctGroupArray[sp].habitatType == SED)
								/* Sediment biomass data */
								for (k = 0; k < bm->sednz; k++){
									biolbiom[sp][b][SED] += bm->sedtr[b][k][FunctGroupArray[sp].totNTracers[0]] * bm->boxes[b].sm.dz[k];
								}
							else {
								biolbiom[sp][b][WC] += bm->epi[b][FunctGroupArray[sp].totNTracers[0]];
							}
						}
						break;
					case REF_DET:
					case LAB_DET:
					case CARRION:
						if (bm->flagecosysassess) {

							/* Sediment biomass data */
							for (k = 0; k < bm->sednz; k++)
								biolbiom[sp][b][SED] += bm->sedtr[b][k][FunctGroupArray[sp].totNTracers[0]] * bm->boxes[b].sm.dz[k];

							for (k = 0; k < bm->wcnz; k++)
								biolbiom[sp][b][WC] += bm->wctr[b][k][FunctGroupArray[sp].totNTracers[0]] * bm->boxes[b].dz[k];
						}
						break;
					case SED_EP_FF:
					case CORAL:
                    case SPONGE:
					case MOB_EP_OTHER:
					case SED_EP_OTHER:
						/* Epibenthic biomass data */
						sum = 0;
						for (cohort = 0; cohort < FunctGroupArray[sp].numCohortsXnumGenes; cohort++) {
							sum += bm->epi[b][FunctGroupArray[sp].totNTracers[cohort]];
						}
						biolbiom[sp][b][WC] = sum;
						break;

					case LG_ZOO:
					case MED_ZOO:
						for (k = 0; k < bm->wcnz; k++) {
							//if (sp == ZLguild_id)
							if (FunctGroupArray[sp].isFished == TRUE)
								biolbiom[sp][b][WC] += bm->wctr[b][k][FunctGroupArray[sp].totNTracers[0]] * bm->boxes[b].dz[k];
							else if (bm->flagecosysassess)
								biolbiom[sp][b][WC] += bm->wctr[b][k][FunctGroupArray[sp].totNTracers[0]] * bm->boxes[b].dz[k];
						}
						break;
					case ICE_BACT:	/* Intentional follow throughs */
					case ICE_DIATOMS:
					case ICE_MIXOTROPHS:
					case ICE_ZOOBIOTA:

						/* TODO: How handle ice indices? */
						break;
					case SM_ZOO:
					case LG_PHY:
					case SM_PHY:
					case PL_BACT:
					case DINOFLAG:
						if (bm->flagecosysassess) {
							for (k = 0; k < bm->wcnz; k++) {
								biolbiom[sp][b][WC] += bm->wctr[b][k][FunctGroupArray[sp].totNTracers[0]] * bm->boxes[b].dz[k];
							}
						}

						break;
					case CEP:
					case PWN:
						for (k = 0; k < bm->wcnz; k++) {
							sum = 0;
							for (cohort = 0; cohort < FunctGroupArray[sp].numCohortsXnumGenes; cohort++) {
								sum += bm->wctr[b][k][FunctGroupArray[sp].totNTracers[cohort]];
							}
							biolbiom[sp][b][WC] += sum * bm->boxes[b].dz[k];
						}
						break;
					}
				}
			}

			for (k = 0; k < bm->wcnz; k++) {

				/** Stock and data - rest of observer data below in "sed" section **/
				/* Vertebrates */
				for (sp = 0; sp < bm->K_num_tot_sp; sp++) {
					if (FunctGroupArray[sp].isVertebrate == TRUE) {
						for (cohort = 0; cohort < FunctGroupArray[sp].numCohortsXnumGenes; cohort++) {

							biolVERTinfo[bstruct_id][sp][cohort][b] += bm->wctr[b][k][FunctGroupArray[sp].structNTracers[cohort]]
									* (bm->boxes[b].dz[k] / (phys[wc_depth_id][b] + TINY));
							biolVERTinfo[bres_id][sp][cohort][b] += bm->wctr[b][k][FunctGroupArray[sp].resNTracers[cohort]] * (bm->boxes[b].dz[k]
									/ (phys[wc_depth_id][b] + TINY));
							biolVERTinfo[bstocknums_id][sp][cohort][b] += bm->wctr[b][k][FunctGroupArray[sp].NumsTracers[cohort]];

							/*
							 fprintf(ofp, "Time: %e, %s-%d in box %d-%d SN: %e (realSN: %e, dz: %e, depth: %e), RN: %e (realRN: %e), nums: %e (realNums: %e)\n",
							 bm->dayt, FunctGroupArray[sp].groupCode, cohort, b, k, biolVERTinfo[bstruct_id][sp][cohort][b],
							 bm->wctr[b][k][FunctGroupArray[sp].structNTracers[cohort]], bm->boxes[b].dz[k], phys[wc_depth_id][b],
							 biolVERTinfo[bres_id][sp][cohort][b], bm->wctr[b][k][FunctGroupArray[sp].resNTracers[cohort]],
							 biolVERTinfo[bstocknums_id][sp][cohort][b], bm->wctr[b][k][FunctGroupArray[sp].NumsTracers[cohort]]);
							 */

							/* Store representative boxes */
							if ((biolVERTinfo[bstruct_id][sp][cohort][b]) && (biolVERTinfo[bres_id][sp][cohort][b])
									&& (biolVERTinfo[bstocknums_id][sp][cohort][b])) {
								stage = FunctGroupArray[sp].cohort_stage[cohort];
								bm->rep_box_of_zone[sp][z][stage] = b;
							}
						}
					}
				}
			}

			if (verbose > 2) {
				if (bm->flagecosysassess) {
					for (k = 0; k < bm->K_num_tot_sp; k++) {
						if (FunctGroupArray[k].isVertebrate == FALSE) {
							fprintf(ofp, "biolbiom[%s][%d][WC]: %e\n", FunctGroupArray[k].groupCode, b, biolbiom[k][b][WC]);
						}
					}
				}

				for (k = 0; k < bm->K_num_tot_sp; k++) {
					if (FunctGroupArray[k].isVertebrate == TRUE) {
						for (z = 0; z < FunctGroupArray[k].numCohortsXnumGenes; z++)
							for (ij = 0; ij < 7; ij++)
								fprintf(ofp, "biolVERT[%d][%d][%d][%d]: %e\n", ij, k, z, b, biolVERTinfo[ij][k][z][b]);
					}
				}
			}

			/** Production and consumption data - i.e. fisheries independent data **/
			if (bm->flagecosysassess) {

				/* Invertebrates */

				for (sp = 0; sp < bm->K_num_tot_sp; sp++) {
					if (FunctGroupArray[sp].isVertebrate == FALSE && FunctGroupArray[sp].isDetritus == FALSE) {

						switch (FunctGroupArray[sp].habitatType) {
						case WC:
							biolprod[sp][b] += bm->diagnost[b][FunctGroupArray[sp].prodnTracers[0]] * (-1.0 * bm->boxes[b].botz);
							break;
						case SED:
							biolprod[sp][b] += bm->diagnost[b][FunctGroupArray[sp].prodnTracers[0]] * sed_dz;
							break;
						case EPIFAUNA:
						case LAND_BASED:
							biolprod[sp][b] += bm->diagnost[b][FunctGroupArray[sp].prodnTracers[0]];
							break;
						case ICE_BASED:
							biolprod[sp][b] += bm->diagnost[b][FunctGroupArray[sp].prodnTracers[0]] * bm->boxes[b].ice.max_ice_depth;
							break;
                        case MIXED:
                            quit("How did we get here as should come through a primary habitat\n");
                            break;
						}
					}
				}

				for (sp = 0; sp < bm->K_num_tot_sp; sp++) {
					if (FunctGroupArray[sp].isVertebrate == FALSE && (FunctGroupArray[sp].isPredator == TRUE || FunctGroupArray[sp].isBacteria == TRUE)) {

						switch (FunctGroupArray[sp].habitatType) {
						case WC:
							bioleat[sp][b] += bm->diagnost[b][FunctGroupArray[sp].GrazingTracers[0]] * (-1.0 * bm->boxes[b].botz);
							break;
						case SED:
							bioleat[sp][b] += bm->diagnost[b][FunctGroupArray[sp].GrazingTracers[0]] * sed_dz;
							break;
						case EPIFAUNA:
						case LAND_BASED:
							bioleat[sp][b] += bm->diagnost[b][FunctGroupArray[sp].GrazingTracers[0]];
							break;
						case ICE_BASED:
							bioleat[sp][b] += bm->diagnost[b][FunctGroupArray[sp].GrazingTracers[0]] * bm->boxes[b].ice.max_ice_depth;
							break;
                        case MIXED:
                            quit("How did we get here as should come through a primary habitat\n");
                            break;
						}
					}
				}
				for (sp = 0; sp < bm->K_num_tot_sp; sp++) {
					if (FunctGroupArray[sp].isVertebrate == TRUE) {
						for (cohort = 0; cohort < FunctGroupArray[sp].numCohortsXnumGenes; cohort++) {
							biolVERTinfo[bprod_id][sp][cohort][b] += bm->diagnost[b][FunctGroupArray[sp].GrowthTracers[cohort]]
									* biolVERTinfo[bstocknums_id][sp][cohort][b] * (-1.0 * bm->boxes[b].botz);
							biolVERTinfo[beat_id][sp][cohort][b] += bm->diagnost[b][FunctGroupArray[sp].EatTracers[cohort]] * (-1.0 * bm->boxes[b].botz);
						}
					}
				}

				if (verbose > 2) {
					for (k = 0; k < bm->K_num_tot_sp; k++) {
						if (FunctGroupArray[k].isVertebrate == FALSE && FunctGroupArray[k].isDetritus == FALSE) {
							fprintf(ofp, "biolprod[%d][%d]: %e\n", k, b, biolprod[k][b]);
						}
					}
					for (k = 0; k < bm->K_num_tot_sp; k++) {
						if (FunctGroupArray[k].isVertebrate == FALSE && FunctGroupArray[k].isGrazer == FALSE) {
							fprintf(ofp, "bioleat[%d][%d]: %e\n", k, b, bioleat[k][b]);
						}
					}
				}
			}

			/** Harvest data - this is not actually entered/read-in here. Instead for
			 commerical and recreational catch data, effort data and discard data
			 use bm->CumCatch, bm->CumEffort, bm->CumDiscards
			 **/

			/** Observer data - use this to feed cumulative catch, discards etc size data,
			 for "by fishery" results use bm->SizeCatch and bm->SizeNumDiscard instead
			 (i.e. do data generation straight off them without going via biolVERTinfo)
			 **/
			if (bm->flagobsdata) {
				for (k = 0; k < bm->K_num_tot_sp; k++) {
					if (FunctGroupArray[k].isVertebrate == TRUE) {

						for (ij = 0; ij < FunctGroupArray[k].numCohortsXnumGenes; ij++) {
							for (nf = 0; nf < bm->K_num_fisheries; nf++) {
								/* Cumulative catch size data  */
								biolVERTinfo[bcatchnums_id][k][ij][b] += FunctGroupArray[k].SizeNumCaught[ij][nf][b];
								/* Cumulative discards size data */
								biolVERTinfo[bdiscards_id][k][ij][b] += FunctGroupArray[k].SizeNumDiscard[ij][nf][b];
							}
						}
					}
				}
			}
		}
	}

	return;
}
