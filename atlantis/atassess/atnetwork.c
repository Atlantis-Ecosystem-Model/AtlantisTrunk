/**
 \file
 \brief Routine to calculate network based indices
 \ingroup atassess

 File:           atnetwork.c

 Created:        20/12/2003

 Author:         Beth Fulton

 Purpose:        Routine to calculate network based indices. Much of	the code is adapted
 from Villy Christensen's Ecopath with Ecosim visual basic code
 Modified from general bay version of sampling model

 Arguments:      bm - Pointer to box model.

 Returns:        void

 Revisions:      8/8/2004 Created from southeast network.c

 11/9/2004 Removed Ecopath routines

 22/9/2004 Replaced numbers in switch statements with id
 guild numbers.

 22/10/2004 Replaced K_num_tot with K_num_tot_assess

 16/8/2005 Put in default case for all switch statements


 *********************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <math.h>
#include <sjwlib.h>
#include "atsample.h"

/**
 * \brief This calculates the network information for use by network indices
 * Assumes no immigration, emigration for all groups despite the fact fish move around
 * and plankton are advected as the production, consumption etc estimates come from within
 * the main biological calculations in Atlantis and the advection etc terms aren't recorded there.
 */
void Calc_Network_Info(MSEBoxModel *bm, int z, int id) {
	int k, sp, detIndex;
	double eatsum, exsum, imsum, prodsum, biomsum, respsum, u, pcalc, ge, aprop, afprop;
	int maxStage, stage;

	for (k = 0; k < K_netwk_properties; k++)
		netwkindx[k][z] = 0.0;

	if (verbose > 1)
		printf("Calculating network info\n");

	eatnetwk[0][0][z] = 0;
	prodnetwk[0][0][z] = 0.0;
	biomnetwk[0][0][z] = 0.0;
	exportnetwk[0][0][z] = 0;
	respnetwk[0][0][z] = 0; // TODO: Fix - calculation of this currently not sensible
	importnetwk[0][0][z] = 0.0;
	mortnetwk[0][0][z] = 0.0;

	for (sp = 0; sp < bm->K_num_tot_sp; sp++) {
		if (FunctGroupArray[sp].isAssessed == TRUE) {
			/* Work out the max stage (or location index) */
			if (FunctGroupArray[sp].isVertebrate == TRUE || FunctGroupArray[sp].isDetritus == TRUE) {
				maxStage = adult_id; /* Allow for wc and sed values for detritus */
			} else {
				maxStage = juv_id;
			}

			for (stage = 0; stage <= maxStage; stage++) {
				if (z != bm->nfzones) {

					if (FunctGroupArray[sp].isVertebrate == TRUE) {

						if (stage == juv_id) {
							aprop = 1.0 - pop_fraction[0][sp][z];
							afprop = 1.0 - pop_fraction[1][sp][z];
						} else {
							aprop = pop_fraction[0][sp][z];
							afprop = pop_fraction[1][sp][z];
						}

						eatnetwk[sp + 1][stage][z] = stockinfo[seat_id][sp][z][id] * aprop;
						prodnetwk[sp + 1][stage][z] = stockinfo[sprod_id][sp][z][id] * aprop;
						u = drandom(1.0, 3.0);
						ge = u / 10.0;
						pcalc = ge * eatnetwk[sp + 1][stage][z];
						if (pcalc < prodnetwk[sp + 1][stage][z])
							prodnetwk[sp + 1][stage][z] = pcalc;


						biomnetwk[sp + 1][stage][z] = stockinfo[sbiomass_id][sp][z][id] * aprop / (zonearea[z] + TINY);
						exportnetwk[sp + 1][stage][z] = stockinfo[stotcatch_id][sp][z][id] * afprop / (zonearea[z] + TINY);
						importnetwk[sp + 1][stage][z] = 0.0;

					} else {
						if (FunctGroupArray[sp].isBacteria == FALSE) {
							switch (FunctGroupArray[sp].groupType) {

							case NUM_GROUP_TYPES:
								/* Ignore - added to make sure we cover all groups */
								break;
							case BIRD:
							case MAMMAL:
							case SHARK:
                            case FISH:
                            case FISH_INVERT:
								/* ignore the vertebrates and fully age structured fished invertebrates (for now) */
								break;

							case LG_PHY:
							case SM_PHY:
							case MICROPHTYBENTHOS:
							case DINOFLAG: /* Phytoplankton and Microphytobenthos */

								eatnetwk[sp + 1][stage][z] = 0;
								prodnetwk[sp + 1][stage][z] = sampleprod[sp][z][id];
								biomnetwk[sp + 1][stage][z] = samplebiom[sp][z][id];
								exportnetwk[sp + 1][stage][z] = 0;
								importnetwk[sp + 1][stage][z] = 0.0;

								break;
							case SM_ZOO:
							case MED_ZOO:
							case LG_ZOO: /* Zooplankton */
								eatnetwk[sp + 1][stage][z] = sampleeat[sp][z][id];
								prodnetwk[sp + 1][stage][z] = sampleprod[sp][z][id];
								u = drandom(1.0, 3.0);
								ge = u / 10.0;
								pcalc = ge * eatnetwk[sp + 1][stage][z];
								if (pcalc < prodnetwk[sp + 1][stage][z])
									prodnetwk[sp + 1][stage][z] = pcalc;
								biomnetwk[sp + 1][stage][z] = samplebiom[sp][z][id];
								exportnetwk[sp + 1][stage][z] = 0;
								importnetwk[sp + 1][stage][z] = 0.0;

								break;

							case CEP: /* Cephalopods */
							case PWN: /* and Squid */
							case LG_INF:
							case CORAL:
                            case SPONGE:
							case SED_EP_FF:
							case SED_EP_OTHER:
							case MOB_EP_OTHER: /* and benthos */
								eatnetwk[sp + 1][stage][z] = sampleeat[sp][z][id];
								prodnetwk[sp + 1][stage][z] = sampleprod[sp][z][id];
								u = drandom(1.0, 3.0);
								ge = u / 10.0;
								pcalc = ge * eatnetwk[sp + 1][stage][z];
								if (pcalc < prodnetwk[sp + 1][stage][z])
									prodnetwk[sp + 1][stage][z] = pcalc;
								biomnetwk[sp + 1][stage][z] = samplebiom[sp][z][id];
								exportnetwk[sp + 1][stage][z] = invstockinfo[fcatch_id][sp][z][id] / (zonearea[z] + TINY);
								importnetwk[sp + 1][stage][z] = 0.0;

								break;

							case PHYTOBEN: /* Macrophytes */
							case SEAGRASS:
							case TURF:
								eatnetwk[sp + 1][stage][z] = 0;
								prodnetwk[sp + 1][stage][z] = sampleprod[sp][z][id];
								biomnetwk[sp + 1][stage][z] = samplebiom[sp][z][id];
								exportnetwk[sp + 1][stage][z] = invstockinfo[fcatch_id][sp][z][id] / (zonearea[z] + TINY);
								importnetwk[sp + 1][stage][z] = 0.0;
								break;
							case LAB_DET:
							case REF_DET:
								detIndex = FunctGroupArray[sp].detIndex;
								eatnetwk[sp+1][stage][z] = 0;
								prodnetwk[sp+1][stage][z] = 0;
								biomnetwk[sp+1][stage][z] = sampledetbiom[detIndex][stage][z][id] +
										DiscardFate[detIndex][stage] * sampledetbiom[FunctGroupArray[CarrionIndex].detIndex][0][z][id];
								exportnetwk[sp+1][stage][z] = 0;
								importnetwk[sp+1][stage][z] = detrit_import[(stage * 2) + detIndex][z];
								respnetwk[sp+1][stage][z] = 0;
								break;

							case CARRION:
							case SM_INF:
							case SED_BACT:
							case PL_BACT:
								/*Do nothing */
								break;
							case ICE_BACT:
							case ICE_DIATOMS:
							case ICE_MIXOTROPHS:
							case ICE_ZOOBIOTA:
								/* TODO: How handle ice indices? */
								break;
							}
						}
					}

					mortnetwk[sp + 1][stage][z] = prodnetwk[sp + 1][stage][z] / (biomnetwk[sp + 1][stage][z] + TINY);

					netwkindx[netwk_cons_id][z] += eatnetwk[sp + 1][stage][z];
					netwkindx[netwk_prod_id][z] += prodnetwk[sp + 1][stage][z];
					netwkindx[netwk_biom_id][z] += biomnetwk[sp + 1][stage][z];
					netwkindx[netwk_ex_id][z] += exportnetwk[sp + 1][stage][z];
					netwkindx[netwk_imm_req_id][z] += importnetwk[sp + 1][stage][z];
				} else {
					/* For system as a whole */
					eatsum = 0;
					prodsum = 0;
					biomsum = 0;
					exsum = 0;
					imsum = 0;
					respsum = 0;
					for (k = 0; k < bm->nfzones; k++) {
						eatsum += eatnetwk[sp + 1][stage][k];
						prodsum += prodnetwk[sp + 1][stage][k];
						biomsum += biomnetwk[sp + 1][stage][k];
						exsum += exportnetwk[sp + 1][stage][k];
						imsum += importnetwk[sp + 1][stage][k];
						respsum += respnetwk[sp + 1][stage][k];
					}
					eatnetwk[sp + 1][stage][z] = eatsum;
					prodnetwk[sp + 1][stage][z] = prodsum;
					biomnetwk[sp + 1][stage][z] = biomsum;
					exportnetwk[sp + 1][stage][z] = exsum;
					importnetwk[sp + 1][stage][z] = imsum;
					respnetwk[sp + 1][stage][z] = respsum;

					mortnetwk[sp + 1][stage][z] = prodsum / (biomsum + TINY);
					netwkindx[netwk_cons_id][z] += eatnetwk[sp + 1][stage][z];
					netwkindx[netwk_prod_id][z] += prodnetwk[sp + 1][stage][z];
					netwkindx[netwk_biom_id][z] += biomnetwk[sp + 1][stage][z];
					netwkindx[netwk_ex_id][z] += exportnetwk[sp + 1][stage][z];

//					fprintf(bm->logFile, "%s - netwkindx[netwk_ex_id][z] = %e, exportnetwk[j+1][z] = %e\n",
//							FunctGroupArray[sp].groupCode, netwkindx[netwk_ex_id][z], exportnetwk[sp + 1][stage][z]);


					netwkindx[netwk_imm_req_id][z] += importnetwk[sp + 1][stage][z];

					/* For ECOPATH calculation purposes, make sure there is an entry
					 for each group in each zone. Where a group is missing set the
					 biomass to minpool and the production etc estimates to the level
					 matching the average per biomass unit from the areas with the
					 group present. Assume no export or import for these "missing"
					 groups in the zone they're missing from */
					for (k = 0; k < bm->nfzones; k++) {
						if (!biomnetwk[sp + 1][stage][k]) {
							biomnetwk[sp + 1][stage][k] = minpool;
							eatnetwk[sp + 1][stage][k] = eatsum * biomnetwk[sp + 1][stage][k] / (biomsum + TINY);
							prodnetwk[sp + 1][stage][k] = prodsum * biomnetwk[sp + 1][stage][k] / (biomsum + TINY);
							respnetwk[sp + 1][stage][k] = respsum * biomnetwk[sp + 1][stage][k] / (biomsum + TINY);
						}
					}
				}

			}
		}
	}

	Calc_Total_PP(bm, z);

	return;

}

/**
 * \brief Calculates total primary production
 */
void Calc_Total_PP(MSEBoxModel *bm, int z) {
	int sp;
	double totPP = 0;

	for (sp = 0; sp < bm->K_num_tot_sp; sp++) {
		if (FunctGroupArray[sp].isAssessed == TRUE && FunctGroupArray[sp].isDetritus == FALSE) {
			if (FunctGroupArray[sp].isPredator == FALSE || FunctGroupArray[sp].groupType == DINOFLAG) {
				totPP += prodnetwk[sp + 1][0][z];
			}
		}
	}

	netwkindx[netwk_pprod_id][z] = totPP;
	return;
}

