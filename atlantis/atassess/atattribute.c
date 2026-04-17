/**
 \file
 \brief Routines to calculate true value of attributes.
 \ingroup atassess

 File:           atattribute.c

 Created:        8/8/2004

 Author:         Beth Fulton

 Purpose:        Routines to calculate true value of attributes.
 Modified from general bay version of sampling model

 Arguments:      bm    - Pointer to box model.

 Returns:        void

 Revisions:      8/8/2004 Created from southeast attribute.c

 12/9/2004 Adding annual_sample to reduce code for
 calculating attributes

 21/9/2004 Created atecology.c from mseecom.c. And renamed
 FSL, FSO, FSF, FST and FSG to generic names so apply to
 other models/generic case.
 FDC -> FSL  (Ling -> Demersal_DC_Fish)
 FSO -> FDO  (Orange_Roughy -> Demersal_O_Fish)
 FSF -> FDB  (Flathead -> Demersal_B_Fish)
 FST -> FVO  (Bluefin_Tuna -> Pisciv_O_Fish)
 FSG -> SHB  (Gummy_Shark -> Shark_B)

 22/9/2004 Replaced numbers in switch statements with id
 guild numbers.

 7/10/2004 Added write out for additional bioloigcal groups
 (PWN, BFF, BML, FBP, FDE, FDF, FVB, SHR, SSK, REP, SP, WDG)

 16/8/2005 Put in default case for all switch statements

 5/9/2006 Reset parameters so all fisheries can access all fished groups.
 Output still restricted however.

 27/5/2008 Added sample_now so could allow for continuous sampling

 29/5/2008 Added attrib_id entry to sampling arrays so that can allow
 for continuous sampling rather than annual only

 28/11/2008 Bec Gorton
 Moved the attribute files into the atAssessStructure so they
 can be correctly closed at the end of the model run. Functions associated with
 the atAssessStrcture can be found in atAssessStrcuture.c

 13-04-2010 Bec Gorton
 Cleaned up the definitions of the attribute file pointers. All attribute file pointers are only
 defined in this file. They are opened by a call to Write_Attributes and then closed when
 Close_Attribute_Files is called at the end of the model run.

 22-06-2010 Bec Gorton
 Merged trunk changes 1698 into bec_dev branch.

 4/11/2010 Bec Gorton
 Updated function names to conform to coding standard.

 11/07/2012 Bec Gorton
 Removed assumption that detritus groups would be defined last in the functional group file.

 *********************************************************************/

#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>
#include <math.h>
#include <sjwlib.h>
#include "atsample.h"

static void Invertebrate_Attributes(MSEBoxModel *bm);
static void Charismatic_Attributes(MSEBoxModel *bm, FILE *ofp);
static void Fish_Attributes(MSEBoxModel *bm, FILE *ofp);
static void Fisheries_Attributes(MSEBoxModel *bm, FILE *ofp);
static void Calculate_True_Values(MSEBoxModel *bm, FILE *ofp);

void Init_Attribute_Array(MSEBoxModel *bm, FILE *ofp, int id) {
	int b, k, j, sp;
	int detIndex;

	/* Only initialise at the start of the year - otherwise skip ahead */
	if (bm->annual_sample > 1)
		return;

	for (b = 0; b < bm->nfzones; b++) {
		for (k = 0; k < 3; k++) {
			sampleprocess[k][b][id] = 0.0;
			samplephys[k][b][id] = 0.0;
		}
		for (k = 3; k < num_sampled_phy_id - 2; k++) {
			samplephys[k][b][id] = 0.0;
		}
		for (k = 0; k < bm->K_num_tot_sp; k++) {
			if (FunctGroupArray[k].isVertebrate == FALSE) {
				if (FunctGroupArray[k].isDetritus == TRUE) {
					detIndex = FunctGroupArray[k].detIndex;
					sampledetbiom[detIndex][WC][b][id] = 0.0;
					sampledetbiom[detIndex][SED][b][id] = 0.0;
				} else {
					samplebiom[k][b][id] = 0.0;
				}
			}
		}

		for (k = 0; k < bm->K_num_tot_sp; k++) {
			if (FunctGroupArray[k].isVertebrate == FALSE && FunctGroupArray[k].isDetritus == FALSE) {
				sampleprod[k][b][id] = 0.0;
			}
		}
		for (k = 0; k < bm->K_num_tot_sp; k++) {
			if (FunctGroupArray[k].isVertebrate == FALSE && FunctGroupArray[k].isGrazer == TRUE) {
				sampleeat[k][b][id] = 0.0;
			}
		}

		for (sp = 0; sp < bm->K_num_tot_sp; sp++) {
			if (FunctGroupArray[sp].isVertebrate == TRUE) {
				for (k = 0; k < bm->K_num_size; k++) {
					for (j = 0; j < K_num_individ_char; j++) {
						individVERTinfo[j][sp][k][b][id] = 0.0;
					}
				}
				for (j = 0; j < 11; j++)
					stockinfo[j][sp][b][id] = 0.0;

				for (k = 0; k < bm->K_max_agekey; k++)
					for (j = 0; j < bm->K_num_size; j++)
						agelengthkey[j][k][sp][b][id] = 0.0;
			}
		}

		for (sp = 0; sp < bm->K_num_tot_sp; sp++) {
			for (j = 0; j < 3; j++)
				invstockinfo[j][sp][b][id] = 0.0;
		}
	}
	return;
}
/**
 *
 *  \brief Calculates true values of the attributes that the indices supposedly proxy
 */
void Calculate_Attribute_Values(MSEBoxModel *bm, FILE *ofp) {
	int b, k, sp, jj, matage, nt;
	double step1, newnums, scalarsum;

	nt = (int) (bm->dt / 86400.0);

	if (verbose)
		fprintf(stderr, "Calculating true attribute values\n");

	/* Save old numbers of babies (young of the year) */
	for (b = 0; b < bm->nfzones; b++) {
		for (sp = 0; sp < bm->K_num_tot_sp; sp++) {
			if (FunctGroupArray[sp].isVertebrate == TRUE) {

				//if (FunctGroupArray[sp].speciesParams[isbird_id] > 1) {
				if (FunctGroupArray[sp].groupType == BIRD) {
					/* Seabirds and penguins */
					oldbaby[sp][b][attrib_id] = stockinfo[sstocknums_id][sp][b][attrib_id] * individVERTinfo[istocknums_id][sp][0][b][attrib_id];
				}
				if (FunctGroupArray[sp].groupType == MAMMAL) {
					/* Mammals */
					step1 = minpool;
					/* Note: Using year classes so use agemat * age classsize */
					matage = (int) (FunctGroupArray[sp].speciesParams[age_mat_id] * FunctGroupArray[sp].ageClassSize);
					if (matage > bm->K_max_agekey)
						matage = bm->K_max_agekey;

					if (matage > FunctGroupArray[sp].numCohorts)
						matage = FunctGroupArray[sp].numCohorts;

					for (jj = 0; jj < matage; jj++) {
						scalarsum = 0;
						for (k = 0; k < bm->K_num_size; k++) {
							newnums = stockinfo[sstocknums_id][sp][b][attrib_id] * individVERTinfo[istocknums_id][sp][k][b][attrib_id]
									* agelengthkey[k][jj][sp][b][attrib_id];
							scalarsum += individVERTinfo[istocknums_id][sp][k][b][attrib_id] * agelengthkey[k][jj][sp][b][attrib_id];
							step1 += newnums;
						}
					}
					oldbaby[sp][b][attrib_id] = step1;
				}
			}
		}
	}

	/* Now that the the number of babies has been recorded reset the data flag
	 Actually dropped the reset so that got results more typical of real data,
	 i.e. data not lagged.
	 firstdata = 0;
	 */

	/* Initialise arrays */
	Init_Attribute_Array(bm, ofp, attrib_id);

	/* Invertebrate attributes */
	Invertebrate_Attributes(bm);

	/* mammal attributes */
	Charismatic_Attributes(bm, ofp);

	/* Fisheries attributes */
	Fisheries_Attributes(bm, ofp);

	/* Fish attributes */
	Fish_Attributes(bm, ofp);

	/* Calculate Indices */
	Calculate_True_Values(bm, ofp);

	/* Write output files */
	if (bm->annual_assess)
		Write_Attributes(bm, nt);

	return;
}

/**
 * \brief This reads in the value of the tracer at each timestep
 */
void Invertebrate_Attributes(MSEBoxModel *bm) {
	int sp;
	int bb, b, i, z;
	double sample, min;
	int detIndex;

	/* Scientific sampling */

	if (verbose)
		fprintf(stderr, "Invert attributes\n");

	for (bb = 0; bb < bm->nsbox; bb++) {
		b = bm->nsboxes[bb];
		z = bm->boxes[b].zone - 1;

		/* Physical characteristics */
		sample = phys[salinity_id][b];
		samplephys[0][z][attrib_id] += sample * samplearea[bb];
		for (i = light_wc_id; i < don_wc_id; i++) {
			sample = phys[i][b];
			samplephys[i - 2][z][attrib_id] += sample * samplearea[bb];
		}
		/* Processes */
		for (i = 0; i < 3; i++) {
			sample = physprocess[i][b];
			sampleprocess[i][z][attrib_id] += sample * samplearea[bb];
		}
		/* Biomass */
		for (sp = 0; sp < bm->K_num_tot_sp; sp++) {
			if (FunctGroupArray[sp].isVertebrate == FALSE) {

				if (FunctGroupArray[sp].isDetritus == TRUE) {
					detIndex = FunctGroupArray[sp].detIndex;

					sample = biolbiom[sp][b][WC];
					sampledetbiom[detIndex][WC][z][attrib_id] += sample * samplearea[bb];

					sample = biolbiom[sp][b][SED];
					sampledetbiom[detIndex][SED][z][attrib_id] += sample * samplearea[bb];

				} else {
					sample = biolbiom[sp][b][WC];
					samplebiom[sp][z][attrib_id] += sample * samplearea[bb];
				}
			}
		}

		/* Production */
		for (sp = 0; sp < bm->K_num_tot_sp; sp++) {
			if (FunctGroupArray[sp].isVertebrate == FALSE && FunctGroupArray[sp].isDetritus == FALSE) {
				sample = biolprod[sp][b];
				sampleprod[sp][z][attrib_id] += sample * samplearea[bb];
			}
		}

		/* Consumption */
		for (sp = 0; sp < bm->K_num_tot_sp; sp++) {
			if (FunctGroupArray[sp].isVertebrate == FALSE && FunctGroupArray[sp].isGrazer == TRUE) {
				sample = bioleat[sp][b];
				sampleeat[sp][z][attrib_id] += sample * samplearea[bb];
			}
		}
	}

	/* Get minimum consumption term for inverts */
	for (z = 0; z < bm->nfzones; z++) {
		min = MAXDOUBLE;
		for (sp = 0; sp < bm->K_num_tot_sp; sp++) {
			if (FunctGroupArray[sp].isVertebrate == FALSE && FunctGroupArray[sp].isGrazer == TRUE && FunctGroupArray[sp].groupType != DINOFLAG) {
				if ((sampleeat[sp][z][attrib_id] > 0.0) && (sampleeat[sp][z][attrib_id] < min))
					min = sampleeat[sp][z][attrib_id];
			}
		}
		mineat[z][attrib_id] = min;
	}

	return;

}

/**
 *  \brief Fisheries independent sampling and also estimates of biomass etc based on catch for the
 *  mammal groups (mammals and birds)
 */
void Charismatic_Attributes(MSEBoxModel *bm, FILE *ofp) {

	int chrt, b, z, i, j, k, indx, bb, nraw, sp, naa, nsubages, ninsubage, ltc, cage, fishery_id, lbin;
	double wgt, lngth, raw, sum, vol, rawN, rawW;
	int cmaxage = bm->K_max_agekey - 1;
	int cmaxsize = bm->K_num_size - 1;

	fishery_id = 0;

	if (verbose)
		fprintf(stderr, "mammal attributes\n");

	/* Initialise the local arrays */

	for (z = 0; z < bm->nfzones; z++) {
		for (k = 0; k < 4; k++) {
			for (sp = 0; sp < bm->K_num_tot_sp; sp++) {
				/* Will just use the vertebrate slots but 0 them all */
				totnums[k][sp][z] = 0.0;
			}
		}
	}

	/* Initialise the bins */
	for (i = 0; i < bm->K_num_size; i++)
		for (b = 0; b < bm->nfzones; b++)
			for (sp = 0; sp < bm->K_num_tot_sp; sp++) {
				if (FunctGroupArray[sp].isVertebrate == TRUE) {
					if(FunctGroupArray[sp].groupType == BIRD || FunctGroupArray[sp].groupType == MAMMAL){
						for (j = 0; j < 5; j++)
							sizebins[j][sp][b][i][0][attrib_id] = 0.0;
					}
				}
			}

	for (sp = 0; sp < bm->K_num_tot_sp; sp++) {
		if (FunctGroupArray[sp].isVertebrate == TRUE) {
			if(FunctGroupArray[sp].groupType == BIRD || FunctGroupArray[sp].groupType == MAMMAL){
				for (b = 0; b < bm->nfzones; b++)
					for (j = 0; j < bm->K_max_agekey; j++)
						for (i = 0; i < bm->K_num_size; i++)
							agebins[sp][b][j][i][attrib_id] = 0.0;
			}
		}
	}

	/* Bird information */
	if (verbose)
		fprintf(stderr, "Sampling birds\n");

	for (b = 0; b < bm->nbox; b++) {
		if (bm->boxes[b].type != BOUNDARY) {
			z = bm->boxes[b].zone - 1;

			for (sp = 0; sp < bm->K_num_tot_sp; sp++) {
				if(FunctGroupArray[sp].groupType == BIRD){
					for (chrt = 0; chrt < FunctGroupArray[sp].numCohortsXnumGenes; chrt++) {
						/* Get information for nestlings, newly fledged and adult seabirds */
						switch (chrt) {
						case 0: /* Nestlings */
						case 1: /* Fledglings */
							lbin = chrt;
							break;
						default: /* All adults together */
							lbin = 2;
							break;
						}
						/* Get juvenile and newly fledged bird information */
						/* Weights */
						raw = biolVERTinfo[bstruct_id][sp][chrt][b] + biolVERTinfo[bres_id][sp][chrt][b];
						rawW = raw;
						individVERTinfo[iweight_id][sp][lbin][z][attrib_id] += raw;

						/* Numbers in stock */
						raw = biolVERTinfo[bstocknums_id][sp][chrt][b];
						rawN = raw;
						individVERTinfo[istocknums_id][sp][lbin][z][attrib_id] += raw;
                        if (chrt < FunctGroupArray[sp].numGeneTypes) {
                            oldbaby[sp][z][attrib_id] += raw;
                        }

						/* Numbers in catch*/
						raw = biolVERTinfo[bcatchnums_id][sp][chrt][b];
						individVERTinfo[icatchnums_id][sp][0][z][attrib_id] += raw;

						/* Biomass */
						individVERTinfo[ibiomass_id][sp][lbin][z][attrib_id] += rawW * rawN / (bm->boxes[b].area + TINY);

						/* Discards */
						raw = biolVERTinfo[bdiscards_id][sp][chrt][b];
						individVERTinfo[idiscards_id][sp][lbin][z][attrib_id] += raw;

					}
				}
			}
			/* Total mammal stock numbers */
			for (sp = 0; sp < bm->K_num_tot_sp; sp++) {
				if (FunctGroupArray[sp].groupType == MAMMAL) {
					for (chrt = 0; chrt < FunctGroupArray[sp].numCohortsXnumGenes; chrt++) {
						raw = biolVERTinfo[bstocknums_id][sp][chrt][b];
						stockinfo[sstocknums_id][sp][z][attrib_id] += raw;
					}
				}
			}
		}
	}
	for (z = 0; z < bm->nfzones; z++) {
		for (sp = 0; sp < bm->K_num_tot_sp; sp++) {
			if(FunctGroupArray[sp].groupType == BIRD){
				for (chrt = 0; chrt < 3; chrt++) {
					/* Bird stock biomasses */
					stockinfo[sbiomass_id][sp][z][attrib_id] += individVERTinfo[ibiomass_id][sp][chrt][z][attrib_id];

					/* Bird stock numbers */
					stockinfo[sstocknums_id][sp][z][attrib_id] += individVERTinfo[istocknums_id][sp][chrt][z][attrib_id];

					/* Bird catch numbers */
					stockinfo[scatchnums_id][sp][z][attrib_id] += individVERTinfo[icatchnums_id][sp][chrt][z][attrib_id];
				}
				for (chrt = 0; chrt < 3; chrt++) {
					/* Normalise individual bird numbers */
					individVERTinfo[istocknums_id][sp][chrt][z][attrib_id] /= (stockinfo[sstocknums_id][sp][z][attrib_id] + TINY);
					individVERTinfo[icatchnums_id][sp][chrt][z][attrib_id] /= (stockinfo[scatchnums_id][sp][z][attrib_id] + TINY);
				}
			}
		}
	}

	for (bb = 0; bb < bm->nsbox; bb++) {
		b = bm->nsboxes[bb];
		z = bm->boxes[b].zone - 1;
		vol = phys[vol_id][b];

		for (sp = 0; sp < bm->K_num_tot_sp; sp++) {
			if (FunctGroupArray[sp].groupType == MAMMAL) {
				indx = sp;
				fishery_id = (int) (FunctGroupArray[sp].speciesParams[primary_fishery_id]);
				/* Sampling mammal size structure per zone */
				for (chrt = 0; chrt < FunctGroupArray[sp].numCohortsXnumGenes; chrt++) {
					/* Size and weight */
					wgt = biolVERTinfo[bstruct_id][sp][chrt][b] + biolVERTinfo[bres_id][sp][chrt][b];
					lngth = Get_Length(bm, wgt, indx);

					/* Stock */
					raw = biolVERTinfo[bstocknums_id][sp][chrt][b];
					nraw = (int) (ceil(raw));
					Sort_Length_Weight(bm, 0, 1, sizestocknums_id, z, indx, fishery_id, lngth, nraw, wgt, cmaxsize, attrib_id, &ltc, ofp);

					nsubages = (int) (ceil(FunctGroupArray[sp].ageClassSize + 0.5));
                    if (nsubages < 1)
                        nsubages = 1;
					for (naa = 0; naa < nsubages; naa++) {
						double v = drandom(0.0, 1.0);
						/* Assume animal uniformly distributed in age class - may not be right! */
						//cage = (int) (floor((chrt + v) * FunctGroupArray[sp].ageClassSize + 0.5));

						//TODO Check with beth about changing to a round instead of a floor.
						cage = (int) (round((chrt + v) * FunctGroupArray[sp].ageClassSize + 0.5));

						ninsubage = nraw / nsubages;
						Length_Age_Key(bm, z, indx, cmaxsize, lngth, cage, cmaxage, ninsubage, attrib_id);
					}

					/* Catch */
					raw = biolVERTinfo[bcatchnums_id][sp][chrt][b];
					nraw = (int) (ceil(raw));
					Sort_Length_Weight(bm, 0, 1, sizecatchnums_id, z, indx, fishery_id, lngth, nraw, wgt, cmaxsize, attrib_id, &ltc, ofp);

					/* Discards */
					raw = biolVERTinfo[bdiscards_id][sp][chrt][b];
					nraw = (int) (ceil(raw));
					Sort_Length_Weight(bm, 0, 1, sizediscardnums_id, z, indx, fishery_id, lngth, nraw, wgt, cmaxsize, attrib_id, &ltc, ofp);

					if (bm->sample_now) {

						/* Mammal Production */
						raw = biolVERTinfo[bprod_id][sp][chrt][b];

						stockinfo[sprod_id][sp][z][attrib_id] += raw * samplearea[bb] * ceil(biolVERTinfo[bstocknums_id][sp][chrt][b]) / (vol + TINY);

						/* Mammal Consumption */
						raw = biolVERTinfo[beat_id][sp][chrt][b];
						stockinfo[seat_id][sp][z][attrib_id] += raw * samplearea[bb];
					}
				}
			}
		}

		for (sp = 0; sp < bm->K_num_tot_sp; sp++) {
			if(FunctGroupArray[sp].groupType == BIRD){
				for (chrt = 0; chrt < FunctGroupArray[sp].numCohortsXnumGenes; chrt++) {
					/* Bird production */
					raw = biolVERTinfo[bprod_id][sp][chrt][b];
					stockinfo[sprod_id][sp][z][attrib_id] += raw * samplearea[bb] * ceil(biolVERTinfo[bstocknums_id][sp][chrt][b]) / (vol + TINY);

					/* Bird consumption */
					raw = biolVERTinfo[beat_id][sp][chrt][b];
					stockinfo[seat_id][sp][z][attrib_id] += raw * samplearea[bb];
				}
			}
		}
	}

	/* Scaling size structure per zone so it sums to one */
	for (z = 0; z < bm->nfzones; z++) {
		for (sp = 0; sp < bm->K_num_tot_sp; sp++) {
			if (FunctGroupArray[sp].groupType == MAMMAL) {
				for (i = 0; i < bm->K_num_size; i++) {
					for (j = 1; j < 4; j++) {
						totnums[j][sp][z] += sizebins[j][sp][z][i][0][attrib_id];
					}
				}
			}
		}
		for (sp = 0; sp < bm->K_num_tot_sp; sp++) {
			if (FunctGroupArray[sp].groupType == MAMMAL) {
				indx = sp;
				for (i = 0; i < bm->K_num_size; i++) {

					/* Size distribution */
					individVERTinfo[istocknums_id][sp][i][z][attrib_id] = sizebins[sizestocknums_id][indx][z][i][0][attrib_id]
							/ (totnums[totstocknums_id][indx][z] + TINY);

					/* Average weight of individual in that size bin */
					individVERTinfo[iweight_id][sp][i][z][attrib_id] = sizebins[sizeweight_id][indx][z][i][0][attrib_id] / (totnums[totstocknums_id][indx][z]
							+ totnums[totcatchnums_id][indx][z] + totnums[totdiscardnums_id][indx][z] + TINY);

					/* Biomass */
					individVERTinfo[ibiomass_id][sp][i][z][attrib_id] = stockinfo[sstocknums_id][indx][z][attrib_id]
							* individVERTinfo[istocknums_id][sp][i][z][attrib_id] * individVERTinfo[iweight_id][sp][i][z][attrib_id];

					/* Stock biomass */
					stockinfo[sbiomass_id][sp][z][attrib_id] += individVERTinfo[ibiomass_id][sp][i][z][attrib_id];

					/* Size distribution of catch */
					individVERTinfo[icatchnums_id][sp][i][z][attrib_id] = sizebins[sizecatchnums_id][indx][z][i][0][attrib_id]
							/ (totnums[totcatchnums_id][indx][z] + TINY);

					/* Size distribution of discards */
					individVERTinfo[idiscards_id][sp][i][z][attrib_id] = sizebins[sizediscardnums_id][indx][z][i][0][attrib_id]
							/ (totnums[totdiscardnums_id][indx][z] + TINY);

					/* Age-length keys */
					sum = 0.0;
					for (j = 0; j < bm->K_max_agekey; j++) {
						agelengthkey[i][j][sp][z][attrib_id] = agebins[sp][z][j][i][attrib_id];
						
						/* Normalise so each length bin adds up to one across the ages */
						sum += agelengthkey[i][j][sp][z][attrib_id];
					}
					for (j = 0; j < bm->K_max_agekey; j++)
						agelengthkey[i][j][sp][z][attrib_id] /= (sum + TINY);
				}
			}
		}
	}

	return;

}

/**
 * \brief  Sums over the sample timestep so you have total fisheries catch (etc) for the period
 * as the collected records
 */
void Fisheries_Attributes(MSEBoxModel *bm, FILE *ofp) {
	int b, i, k, z, chrt, j, bb, nraw, nf, ij, fishery_id, flagspeffortmodel, sp, ltc;
	double raw, wgt, lngth, correct_size;
	int cmaxsize = bm->K_num_size - 1;

	if (verbose) {
		fprintf(stderr, "Fisheries attributes\n");
		fprintf(ofp, "Fisheries attributes\n");
	}

	/* Reset the totals to zero */
	if (bm->annual_sample == 1) {
		for (b = 0; b < bm->nbox; b++) {
			for (i = 0; i < bm->K_num_tot_sp; i++) {
				if (FunctGroupArray[i].isImpacted == TRUE) {
					for (nf = 0; nf < bm->K_num_fisheries; nf++) {
						for (k = 0; k < 3; k++)
							fishery[k][nf][i][b][attrib_id] = 0.0;
					}
				}
			}
		}
	}

	/* Get totals for the year (e.g. sum over the year)
	 */
	if (bm->annual_sample == numsteps) {
		for (b = 0; b < bm->nbox; b++) {
			if (bm->boxes[b].type != BOUNDARY && bm->boxes[b].type != LAND) {
				for (k = 0; k < bm->K_num_tot_sp; k++) {
					if (FunctGroupArray[k].isFished == TRUE) {
						for (nf = 0; nf < bm->K_num_fisheries; nf++) {
							/* Assign values */
							flagspeffortmodel = (int) (bm->FISHERYprms[nf][flageffortmodel_id]);
							if ((flagspeffortmodel != rec_effort) && (flagspeffortmodel != rec_econ_model)) {
								/* Only collect states for commerical fisheries */
								for (ij = 0; ij < bm->boxes[b].nz; ij++)
									fishery[fcatch_id][nf][k][b][attrib_id] += bm->CumCatch[k][nf][b][ij];
								fishery[feffort_id][nf][k][b][attrib_id] += bm->CumEffort[nf][b];
								fishery[fdiscards_id][nf][k][b][attrib_id] += bm->CumDiscards[k][nf][b];
							}
						}
					}
				}

				for (k = 0; k < bm->K_num_tot_sp; k++) {
					if (FunctGroupArray[k].isFished == FALSE && FunctGroupArray[k].isImpacted == TRUE) {
						/* Ignore the groups that are not birds and not mammals - these will have been accounted for above. */
						if (FunctGroupArray[k].groupType != BIRD && FunctGroupArray[k].groupType != MAMMAL)
							continue;
						/* Collect data for discard groups */
						for (nf = 0; nf < bm->K_num_fisheries; nf++) {
							/* Asign values */
							fishery[feffort_id][nf][k][b][attrib_id] += bm->CumEffort[nf][b];
							fishery[fdiscards_id][nf][k][b][attrib_id] += bm->CumDiscards[k][nf][b];
						}
					}
				}
			}
		}

		/* Aggregate by zone */
		for (b = 0; b < bm->nbox; b++) {
			if (bm->boxes[b].type != BOUNDARY  && bm->boxes[b].type != LAND) {
				z = bm->boxes[b].zone - 1;
				/* Aggregate by zone */

				for (k = 0; k < bm->K_num_tot_sp; k++) {
					if (FunctGroupArray[k].isVertebrate == FALSE && (FunctGroupArray[k].isFished == TRUE || FunctGroupArray[k].isImpacted == TRUE)) {
						/* Invertebrates - replace with invstockinfo */
						for (nf = 0; nf < bm->K_num_fisheries; nf++) {
							invstockinfo[fcatch_id][k][z][attrib_id] += fishery[fcatch_id][nf][k][b][attrib_id];
							invstockinfo[feffort_id][k][z][attrib_id] += fishery[feffort_id][nf][k][b][attrib_id];
							invstockinfo[fdiscards_id][k][z][attrib_id] += fishery[fdiscards_id][nf][k][b][attrib_id];
						}

					}
				}
				/* Vertebrates */
				for (k = 0; k < bm->K_num_tot_sp; k++) {
					if (FunctGroupArray[k].isVertebrate == TRUE) {

						for (nf = 0; nf < bm->K_num_fisheries; nf++) {
							stockinfo[stotcatch_id][k][z][attrib_id] += fishery[fcatch_id][nf][k][b][attrib_id];
							stockinfo[stoteffort_id][k][z][attrib_id] += fishery[feffort_id][nf][k][b][attrib_id];
							stockinfo[sdiscards_id][k][z][attrib_id] += fishery[fdiscards_id][nf][k][b][attrib_id];
						}
					}
				}
			}
		}
	}

	/* Size based data */
	if (verbose)
		fprintf(stderr, "Fisheries sizes\n");

	/* Initialise the bins */
	if (bm->annual_sample == 1) {
		for (i = 0; i < bm->K_num_size; i++) {
			for (b = 0; b < bm->nfzones; b++) {
				for (k = 0; k < bm->K_num_tot_sp; k++) {
					if (FunctGroupArray[k].isVertebrate == TRUE) {
						for (j = 0; j < 5; j++) {
							sizebins[j][k][b][i][1][attrib_id] = 0.0;
						}
					}
				}
			}
		}
	}

	for (bb = 0; bb < bm->nsbox; bb++) {
		b = bm->nsboxes[bb];
		z = bm->boxes[b].zone - 1;

		for (sp = 0; sp < bm->K_num_tot_sp; sp++) {
			if (FunctGroupArray[sp].isVertebrate == TRUE && (FunctGroupArray[sp].groupType != BIRD)
					&& (FunctGroupArray[sp].groupType != MAMMAL)) {
				fishery_id = (int) (FunctGroupArray[sp].speciesParams[primary_fishery_id]);
				for (chrt = 0; chrt < FunctGroupArray[sp].numCohortsXnumGenes; chrt++) {
					wgt = biolVERTinfo[bstruct_id][sp][chrt][b] + biolVERTinfo[bres_id][sp][chrt][b];
					lngth = Get_Length(bm, wgt, sp);
					raw = biolVERTinfo[bcatchnums_id][sp][chrt][b];

					nraw = (int) (ceil(raw));
					Sort_Length_Weight(bm, 1, 1, sizecatchnums_id, z, sp, fishery_id, lngth, nraw, wgt, cmaxsize, attrib_id, &ltc, ofp);

					raw = biolVERTinfo[bdiscards_id][sp][chrt][b] / (wgt + TINY);
					nraw = (int) (ceil(raw));
					Sort_Length_Weight(bm, 1, 1, sizediscardnums_id, z, sp, fishery_id, lngth, nraw, wgt, cmaxsize, attrib_id, &ltc, ofp);
				}
			}
		}
	}

	/* Scaling size structure per zone so it sums to one - only do this annually (i.e. when sampling complete) */
	if (bm->annual_sample == numsteps) {
		for (z = 0; z < bm->nfzones; z++) {
			for (i = 0; i < bm->K_num_size; i++) {
				for (b = 0; b < bm->K_num_tot_sp; b++) {
					if (FunctGroupArray[b].isVertebrate == TRUE && (FunctGroupArray[b].groupType != BIRD)
							&& (FunctGroupArray[b].groupType != MAMMAL)) {
						for (j = 2; j < 4; j++) {
							totnums[j][b][z] += sizebins[j][b][z][i][1][attrib_id];
						}
					}
				}
			}
			for (sp = 0; sp < bm->K_num_tot_sp; sp++) {
				if (FunctGroupArray[sp].isVertebrate == TRUE && FunctGroupArray[sp].groupType != BIRD) {
					for (i = 0; i < bm->K_num_size; i++) {
						if(FunctGroupArray[sp].groupType != MAMMAL){
							/* Store fish info */
							/* In case never get a fisheries independent estimate for this use caught and
							 discarded fish to give an average weight of individual in that size bin,
							 this value will be replaced by a fisheries independent one if it is collected */

							individVERTinfo[iweight_id][sp][i][z][attrib_id] = sizebins[sizeweight_id][sp][z][i][1][attrib_id]
									/ (sizebins[sizecatchnums_id][sp][z][i][1][attrib_id] + sizebins[sizediscardnums_id][sp][z][i][1][attrib_id] + TINY);

							/* Size distribution of catch */
							individVERTinfo[icatchnums_id][sp][i][z][attrib_id] = sizebins[sizecatchnums_id][sp][z][i][1][attrib_id]
									/ (totnums[totcatchnums_id][sp][z] + TINY);

							/* Size distribution of discards */
							individVERTinfo[idiscards_id][sp][i][z][attrib_id] = sizebins[sizediscardnums_id][sp][z][i][1][attrib_id]
									/ (totnums[totdiscardnums_id][sp][z] + TINY);
						} else {
							/* Store mammal mammal info */
							/* Total number of mammals caught */
							stockinfo[scatchnums_id][sp][z][attrib_id] += stockinfo[stotcatch_id][sp][z][attrib_id]
									* individVERTinfo[icatchnums_id][sp][i][z][attrib_id] / (individVERTinfo[iweight_id][sp][i][z][attrib_id] + TINY);

							/* Total number of mammals in discards */
							stockinfo[sdiscardnums_id][sp][z][attrib_id] += stockinfo[sdiscards_id][sp][z][attrib_id]
									* individVERTinfo[idiscards_id][sp][i][z][attrib_id] / (individVERTinfo[iweight_id][sp][i][z][attrib_id] + TINY);
						}
					}
				} else {
					for (i = 0; i < 3; i++) {
						/* Total number of seabirds caught */
						stockinfo[scatchnums_id][sp][z][attrib_id] += stockinfo[stotcatch_id][sp][z][attrib_id]
								* individVERTinfo[icatchnums_id][sp][i][z][attrib_id] / (individVERTinfo[iweight_id][sp][i][z][attrib_id] + TINY);

						/* Total number of seabirds in discards */
						stockinfo[sdiscardnums_id][sp][z][attrib_id] += stockinfo[sdiscards_id][sp][z][attrib_id]
								* individVERTinfo[idiscards_id][sp][i][z][attrib_id] / (individVERTinfo[iweight_id][sp][i][z][attrib_id] + TINY);
					}
				}
			}
		}

		/* Correct vertebrate estimates for number of samples in the year */
		correct_size = numsteps;
		for (z = 0; z < bm->nfzones; z++) {
			for (sp = 0; sp < bm->K_num_tot_sp; sp++) {
				if (FunctGroupArray[sp].isVertebrate == FALSE && (FunctGroupArray[sp].isFished == TRUE || FunctGroupArray[sp].isImpacted == TRUE)) {
					for (j = 0; j < (fdiscards_id + 1); j++) {
						invstockinfo[j][sp][z][attrib_id] /= correct_size;
					}
				}
			}
		}
	}

	return;

}

/**
 * \brief Fisheries independent sampling and also estimates of biomass etc based on catch
 */
void Fish_Attributes(MSEBoxModel *bm, FILE *ofp) {
	int chrt, z, b, bb, i, j, k, indx, nraw, naa, sp, ltc, nsubages, cage, ninsubage, matage, fishery_id;
	double lngth, bioms, raw, wgt, sum, step1, eaten, min, raw1, raw2, contrib1, contrib2, vol, correct_size;

	int cmaxage = bm->K_max_agekey - 1;
	int cmaxsize = bm->K_num_size - 1;

	if (verbose)
		fprintf(stderr, "Fish attributes\n");

	/* Initialise the bins */
	if (bm->annual_sample == 1) {
		for (i = 0; i < bm->K_num_size; i++)
			for (b = 0; b < bm->nfzones; b++)
				for (k = 0; k < bm->K_num_tot_sp; k++)
					if (FunctGroupArray[k].isVertebrate == TRUE && (FunctGroupArray[k].groupType != BIRD)
							&& (FunctGroupArray[k].groupType != MAMMAL)) {
						for (j = 0; j < 5; j++)
							sizebins[j][k][b][i][0][attrib_id] = 0.0;
					}

		for (k = 0; k < bm->K_num_tot_sp; k++)
			if (FunctGroupArray[k].isVertebrate == TRUE && (FunctGroupArray[k].groupType != BIRD) && (FunctGroupArray[k].groupType != MAMMAL)) {
				for (b = 0; b < bm->nfzones; b++)
					for (j = 0; j < bm->K_max_agekey; j++)
						for (i = 0; i < bm->K_num_size; i++)
							agebins[k][b][j][i][attrib_id] = 0.0;
			}

		for (j = 0; j < bm->nfzones; j++)
			for (b = 0; b < bm->K_num_tot_sp; b++)
				if (FunctGroupArray[b].isVertebrate == TRUE && (FunctGroupArray[b].groupType != BIRD)
						&& (FunctGroupArray[b].groupType != MAMMAL)) {
					biom[b][j][attrib_id] = 0.0;
				}

		for (i = 0; i < bm->K_num_tot_sp; i++)
			if (FunctGroupArray[i].isVertebrate == TRUE && (FunctGroupArray[i].groupType != BIRD) && (FunctGroupArray[i].groupType != MAMMAL)) {
				for (z = 0; z < bm->nfzones; z++)
					for (j = 0; j < bm->K_num_size; j++)
						nsq[i][z][j][attrib_id] = 0;
			}
	}

	for (bb = 0; bb < bm->nsbox; bb++) {
		b = bm->nsboxes[bb];
		z = bm->boxes[b].zone - 1;
		vol = phys[vol_id][b];

		for (sp = 0; sp < bm->K_num_tot_sp; sp++) {
			if (FunctGroupArray[sp].isVertebrate == TRUE && (FunctGroupArray[sp].groupType != BIRD)
					&& (FunctGroupArray[sp].groupType != MAMMAL)) {
				indx = sp;
				fishery_id = (int) (FunctGroupArray[sp].speciesParams[primary_fishery_id]);
				for (chrt = 0; chrt < FunctGroupArray[sp].numCohortsXnumGenes; chrt++) {
					wgt = biolVERTinfo[bstruct_id][sp][chrt][b] + biolVERTinfo[bres_id][sp][chrt][b];
					lngth = Get_Length(bm, wgt, indx);
					raw = biolVERTinfo[bstocknums_id][sp][chrt][b];
					bioms = raw * wgt;
					biom[indx][z][attrib_id] += bioms;
					nraw = (int) (floor(raw));
					Sort_Length_Weight(bm, 0, 1, sizestocknums_id, z, indx, fishery_id, lngth, nraw, wgt, cmaxsize, attrib_id, &ltc, ofp);
					nsubages = (int) (ceil(FunctGroupArray[sp].ageClassSize + 0.5));
                    if (nsubages < 1)
                        nsubages = 1;
					for (naa = 0; naa < nsubages; naa++) {
						/* Assume animal uniformly distributed in age class - may not be right! */
						//TODO Check with beth about changing to a round instead of a floor.
						cage = (int) (round((chrt + drandom(0.0, 1.0)) * FunctGroupArray[sp].ageClassSize + 0.5));
						ninsubage = nraw / nsubages;
						Length_Age_Key(bm, z, indx, cmaxsize, lngth, cage, cmaxage, ninsubage, attrib_id);

					}
					/* Get estimate of catchability */
					Sort_Length_Weight(bm, 0, 2, sizeselectnums_id, z, indx, fishery_id, lngth, nraw, wgt, cmaxsize, attrib_id, &ltc, ofp);

					if (bm->sample_now) {
						/* Production */
						raw = biolVERTinfo[bprod_id][sp][chrt][b];
						stockinfo[sprod_id][indx][z][attrib_id] += raw * samplearea[bb] * ceil(biolVERTinfo[bstocknums_id][sp][chrt][b]) / (vol + TINY);

						/* Consumption */
						raw = biolVERTinfo[beat_id][sp][chrt][b];
						stockinfo[seat_id][indx][z][attrib_id] += raw * samplearea[bb];
					}
				}
			}
		}
	}

	/* Scaling size structure per zone so it sums to one - only do this annually (i.e. once all sampling complete) */
	if (bm->annual_sample == numsteps) {
		for (z = 0; z < bm->nfzones; z++) {

			for (sp = 0; sp < bm->K_num_tot_sp; sp++) {
				if (FunctGroupArray[sp].isVertebrate == TRUE && (FunctGroupArray[sp].groupType != BIRD)
						&& (FunctGroupArray[sp].groupType != MAMMAL)) {
					totnums[totstocknums_id][sp][z] = 0.0;

					/* Stock biomass */
					stockinfo[sbiomass_id][sp][z][attrib_id] = biom[sp][z][attrib_id] / 2.0;

					for (i = 0; i < bm->K_num_size; i++) {
						/* Catchability */
						individVERTinfo[iq_id][sp][i][z][attrib_id] = sizebins[sizeselectnums_id][sp][z][i][0][attrib_id] / (nsq[sp][z][i][attrib_id] + TINY);

						/* Size distribution of stock */
						individVERTinfo[istocknums_id][sp][i][z][attrib_id] = sizebins[sizestocknums_id][sp][z][i][0][attrib_id];

						/* Average weight of individual in that size bin */
						step1 = sizebins[sizeweight_id][sp][z][i][0][attrib_id] / (sizebins[sizestocknums_id][sp][z][i][0][attrib_id] + TINY);
						if (step1 > 0)
							individVERTinfo[iweight_id][sp][i][z][attrib_id] = step1;


						/* Numbers in the catch */
						stockinfo[scatchnums_id][sp][z][attrib_id] += stockinfo[stotcatch_id][sp][z][attrib_id]
								* individVERTinfo[icatchnums_id][sp][i][z][attrib_id] / (individVERTinfo[iweight_id][sp][i][z][attrib_id] + TINY);


						/* Numbers in the discards */
						stockinfo[sdiscardnums_id][sp][z][attrib_id] += stockinfo[sdiscards_id][sp][z][attrib_id]
								* individVERTinfo[idiscards_id][sp][i][z][attrib_id] / (individVERTinfo[iweight_id][sp][i][z][attrib_id] + TINY);
						/* Age-length keys */
						sum = 0.0;
						for (j = 0; j < bm->K_max_agekey; j++) {
							agelengthkey[i][j][sp][z][attrib_id] = agebins[sp][z][j][i][attrib_id];
							/* Normalise so each length bin adds up to one across the ages */
							sum += agelengthkey[i][j][sp][z][attrib_id];
						}
						for (j = 0; j < bm->K_max_agekey; j++)
							agelengthkey[i][j][sp][z][attrib_id] /= (sum + TINY);

						/* Total numbers */
						totnums[totstocknums_id][sp][z] += individVERTinfo[istocknums_id][sp][i][z][attrib_id];
					}

					/* Rescale size distribution so it sums to one */
					for (i = 0; i < bm->K_num_size; i++) {
						individVERTinfo[istocknums_id][sp][i][z][attrib_id] = individVERTinfo[istocknums_id][sp][i][z][attrib_id]
								/ (totnums[totstocknums_id][sp][z] + TINY);

						/* Do stuff that was dependent on rescaled size distribution */
						/* Biomass per size bin - corrected for selectivity */
						individVERTinfo[ibiomass_id][sp][i][z][attrib_id] = stockinfo[sbiomass_id][sp][z][attrib_id]
								* individVERTinfo[iweight_id][sp][i][z][attrib_id];

						/* Numbers in the stock */
						stockinfo[sstocknums_id][sp][z][attrib_id] += stockinfo[sbiomass_id][sp][z][attrib_id]
								* individVERTinfo[istocknums_id][sp][i][z][attrib_id] / (individVERTinfo[iweight_id][sp][i][z][attrib_id] + TINY);
					}
				}
			}
		}

		/* Calculate minimum consumption and population make-up for vertebbrates */
		for (z = 0; z < bm->nfzones; z++) {
			min = MAXDOUBLE;

			for (sp = 0; sp < bm->K_num_tot_sp; sp++) {
				if (FunctGroupArray[sp].isVertebrate == TRUE) {
					if (FunctGroupArray[sp].groupType != BIRD) {

						/* Initialise */
						raw1 = 0.0;
						raw2 = 0.0;

						/* Get data */
						/* Note: Using year classes so use agemat * age classsize */
						matage = (int) (FunctGroupArray[sp].speciesParams[age_mat_id] * FunctGroupArray[sp].ageClassSize);
						if (matage > bm->K_max_agekey)
							matage = bm->K_max_agekey;
						if (matage > FunctGroupArray[sp].numCohorts)
							matage = FunctGroupArray[sp].numCohorts;

						for (chrt = 0; chrt < matage; chrt++) {
							for (i = 0; i < bm->K_num_size; i++) {
								contrib1 = individVERTinfo[istocknums_id][sp][i][z][attrib_id] * agelengthkey[i][chrt][sp][z][attrib_id];
								raw1 += contrib1;

								contrib2 = individVERTinfo[icatchnums_id][sp][i][z][attrib_id] * agelengthkey[i][chrt][sp][z][attrib_id];
								raw2 += contrib2;
							}
						}
						/* Percentage of the stock that are juveniles */
						pop_fraction[0][sp][z] = max(0, raw1);

						/* Percentage of the catch that are juveniles */
						pop_fraction[1][sp][z] = max(0, raw2);
						/* Minimum consumption - test juveniles then adult population components */
						eaten = stockinfo[seat_id][sp][z][attrib_id] * raw1;
						if ((eaten > 0.0) && (eaten < min))
							min = eaten;
						raw = 1.0 - pop_fraction[0][sp][z];
						eaten = stockinfo[seat_id][sp][z][attrib_id] * raw;
						if ((eaten > 0.0) && (eaten < min))
							min = eaten;
					} else {
						/* Birds */
						if ((stockinfo[seat_id][sp][z][attrib_id] > 0.0) && (stockinfo[seat_id][sp][z][attrib_id] < min))
							min = stockinfo[seat_id][sp][z][attrib_id];
					}
				}
			}
			/* Store result for the zone */
			if (min < mineat[z][attrib_id])
				mineat[z][attrib_id] = min;
		}

		/* Correct vertebrate estimates for number of samples in the year */
		correct_size = numsteps;
		for (z = 0; z < bm->nfzones; z++) {
			for (sp = 0; sp < bm->K_num_tot_sp; sp++) {
				if (FunctGroupArray[sp].isVertebrate == TRUE) {
					for (j = 0; j < (sdiscards_id + 1); j++) { // As sizemat and scondn calculated as indicators
						stockinfo[j][sp][z][attrib_id] /= correct_size;
					}
				}
			}

			for (sp = 0; sp < bm->K_num_tot_sp; sp++) {
				if(FunctGroupArray[sp].groupType == BIRD || FunctGroupArray[sp].groupType == MAMMAL){
					oldbaby[sp][z][attrib_id] /= correct_size;
				}
			}
		}
	}
	return;
}

/**
 * \brief Add the values in the dietArray to the targetArray.
 */
static void Add_Diet_Info(MSEBoxModel *bm, double ****targetArray, double ****dietArray) {
	int prey, preyStage, pred, predStage;

	for (pred = 0; pred < bm->K_num_tot_sp; pred++) {
		if (FunctGroupArray[pred].isAssessed == TRUE && FunctGroupArray[pred].isPredator == TRUE) {
			for (predStage = 0; predStage < K_num_invageclass; predStage++) {
				for (prey = 0; prey < bm->K_num_tot_sp; prey++) {
					if (FunctGroupArray[prey].isAssessed == TRUE) {
						for (preyStage = 0; preyStage < K_num_invageclass; preyStage++) {
							targetArray[pred][predStage][prey][preyStage] += dietArray[pred][predStage][prey][preyStage];
						}
					}
				}
			}
		}
	}
}

//static void Add_Diet_Info(MSEBoxModel *bm, double ****targetArray, double **dietArray) {
//	int prey, preyStage, pred, predStage;
//
//	for (pred = 0; pred < bm->K_num_tot_sp; pred++) {
//		if (FunctGroupArray[pred].isAssessed == TRUE && FunctGroupArray[pred].isPredator == TRUE) {
//			for (predStage = 0; predStage < K_num_invageclass; predStage++) {
//				for (prey = 0; prey < bm->K_num_tot_sp; prey++) {
//					if (FunctGroupArray[prey].isAssessed == TRUE) {
//						for (preyStage = 0; preyStage < K_num_invageclass; preyStage++) {
//							targetArray[pred][predStage][prey][preyStage] += dietArray[pred * (predStage + 1)][prey * (preyStage + 1)];
//						}
//					}
//				}
//			}
//		}
//	}
//}
/**
 *  \brief Calculate the overall diet composition matrix.
 */
void Calculate_Overall_Diet_Composition(MSEBoxModel *bm, double ****totDiet, double ****indiet, double ****offdiet) {

	int pred, predStage, prey, preyStage;
	/* Overall diet composition */
	for (pred = 0; pred < bm->K_num_tot_sp; pred++) {
		if (FunctGroupArray[pred].isAssessed == TRUE && FunctGroupArray[pred].isPredator == TRUE) {
			for (prey = 0; prey < bm->K_num_tot_sp; prey++) {
				if (FunctGroupArray[prey].isAssessed == TRUE) {
					for (predStage = 0; predStage < K_num_invageclass; predStage++) {
						for (preyStage = 0; preyStage < K_num_invageclass; preyStage++) {
							totDiet[pred][predStage][prey][preyStage] += offdiet[pred][predStage][prey][preyStage] * bm->offshorearea
									+ indiet[pred][predStage][prey][preyStage] * bm->inshorearea;
						}
					}
				}
			}
		}
	}
}
/**
 * \brief  This routine calculates the diet matrices
 */
void Calculate_Real_Stomachs(MSEBoxModel *bm, FILE *ofp) {
	int bb, b;
	double d;

	if (verbose)
		fprintf(stderr, "Calculating true diet matrix\n");

	/* Intialise diets */
	Util_Init_4D_Double(intruediet, bm->K_num_tot_sp, 2, bm->K_num_tot_sp, 2, 0.0);
	Util_Init_4D_Double(offtruediet, bm->K_num_tot_sp, 2, bm->K_num_tot_sp, 2, 0.0);
	Util_Init_4D_Double(tottruediet, bm->K_num_tot_sp, 2, bm->K_num_tot_sp, 2, 0.0);

	for (bb = 0; bb < bm->nsbox; bb++) {
		b = bm->nsboxes[bb];
		d = phys[wc_depth_id][b];
		//z = bm->boxes[b].zone - 1;

		Setup_Avail_Food(bm, b, ofp);

		if (d < bm->cdz) {
			/* Inshore diets */
			Add_Diet_Info(bm, intruediet, availfood);
			//Add_Diet_Info(bm, intruediet, new_availfood);

		} else {
			/* Offshore diets */
			Add_Diet_Info(bm, offtruediet, availfood);
			//Add_Diet_Info(bm, offtruediet, new_availfood);
		}
	}

	/* Overall diet composition */
	Calculate_Overall_Diet_Composition(bm, tottruediet, intruediet, offtruediet);

	/* Normalise diet composition */
	Normalise_AvailFood(bm, intruediet);
	Normalise_AvailFood(bm, offtruediet);
	Normalise_AvailFood(bm, tottruediet);

	Calculate_Trophic_Levels(bm, trueTL, intruediet, offtruediet, tottruediet, ofp);

	return;

}

/**
 * \brief  This calculates true values of some indices based
 * on real values not samples
 */
void Calculate_True_Values(MSEBoxModel *bm, FILE *ofp) {
	int z;
	int id = attrib_id;

	if (verbose)
		fprintf(stderr, "Calculating true indices\n");

	/* Initialise globals and bm->nfzones entries for index arrays */
	Init_Globals_And_Arrays(bm);

	/* Calculate Indices */
	for (z = 0; z < bm->nfzones + 1; z++) {
		Calc_Network_Info(bm, z, attrib_id);
		Calc_SOI(bm, z, id, ofp);

		/* If this is the overall model area case then finish here */
		if (z == bm->nfzones)
			break;

		Calc_Size_Mature_And_Condn(bm, z, attrib_id, ofp);
		Calc_Size_Spectra(bm, z, attrib_id);
		Calc_CV(bm, z);
		Calculate_Phys_SD(bm, z);
	}

	Calc_Div(bm, ofp);

	/* Calculate Diet based Indices */
	Calc_Avgtl(bm, trueTL, attrib_id, ofp);
	Calc_Trophic_Spectra(bm, trueTL, attrib_id);

	return;
}

/**
 * brief  This calculates the diet based indices being trialled
 */
void Calculate_True_Diet_Indices(MSEBoxModel *bm) {
	if (verbose)
		fprintf(stderr, "Calculating true omnivory\n");

	/* Calculate Indices */
	Calc_OI(bm, trueTL, intruediet, offtruediet, tottruediet);
	return;

}

