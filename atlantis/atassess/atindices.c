/**
 \file
 \brief  Routine to calculate effects indices
 \ingroup atassess

 File:           atindices.c

 Created:        8/8/2004

 Author:         Beth Fulton

 Purpose:        Routine to calculate effects indices
 Modified from general bay version of sampling model

 Arguments:      bm    - Pointer to box model.

 Returns:        void

 Revisions:      8/8/2004 Created from southeast indices.c

 11/9/2004 Removed indicators not required in MSE studies

 7/10/2004 Added arrays and strong vectors additional bioloigcal groups
 (PWN, BFF, BML, FBP, FDE, FDF, FVB, SHR, SSK, REP, SP, WDG).

 22/10/2004 Replaced K_num_tot with K_num_tot_assess

 19/1/2005 Added Microphytobenthos

 27-10-2009 Bec Gorton
 Moved the following parameters into the species params.
 lwprm[0][vert] = li_a = coefficient in allometric relationship between weight and length
 lwprm[1][vert] = li_b = exponent in allometric relationship between weight and length
 lwprm[2][vert] = size of length bin
 lwprm[3][vert] = size length bin start with (so if 0 then first bin size <= length bin
 but if x then first bin size <= length bin + x

 Also moved the max length information into the species params.

 14-05-2010 Bec Gorton
 Merged in trunk changes 1558

 16-04-2010 Bec Gorton
 Added code to store the standard deviation value that is calculated
 with the variance coefficents. The SD values are also calculated and stored
 for the physical properties that are sampled.
 *********************************************************************/

#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>
#include <math.h>
#include <sjwlib.h>
#include "atsample.h"

/**
 * \brief This calculates the indices being trialled (except for ECOPATH with ECOSIM dependent ones.
 * To save on redundancy...
 * For survey biomass as index of true biomass - use survey output files
 * For CPUE as index of biomass - use catch and effort from catch output files
 * For annual sampling size structure as index of true size structure - use size structure output files
 */
void Indices(MSEBoxModel *bm, FILE *ofp) {
	int z;
	int id = survey_id;

	if (verbose)
		fprintf(stderr, "Calculating indices\n");

	/* Initialise globals and bm->nfzones entries for index arrays */
	Init_Globals_And_Arrays(bm);

	/* Calculate Indices */
	for (z = 0; z < bm->nfzones + 1; z++) {
		/* Network based indices */
		fprintf(stderr, "Calculating network indices for zone: %d\n", z);

		Calc_Network_Info(bm, z, sample_id);
		Calc_SOI(bm, z, id, ofp);

		/* If this is the overall model area case then finish here */
		if (z == bm->nfzones)
			break;

		/* Other indices */
		fprintf(stderr, "Calculating other indices for zone: %d\n", z);

		Calc_Discards(bm, z, sample_id);
		Calc_PBR(bm, z, sample_id, ofp);
		Calc_Max_Size(bm, z, sample_id);
		Calc_Size_Mature_And_Condn(bm, z, sample_id, ofp);
		Calc_Size_Spectra(bm, z, sample_id);
		Calc_ABC(bm, z, sample_id);
		Calc_Hab_Complx(bm, z, sample_id);
		Calc_Reprod_Success(bm, z, sample_id, ofp);
		Calc_Monbet(bm, z, sample_id);
		Calc_PD_Ratio(bm, z, sample_id);
		Calc_Nppb(bm, z, sample_id);
		Calc_CV(bm, z);
	}

	Calc_Div(bm, ofp);
	Calc_Avgtl(bm, TL, sample_id, ofp);
	Calc_Trophic_Spectra(bm, TL, sample_id);

	return;
}

/**
 * \brief This calculates the diet based indices being trialled
 */
void Diet_Indices(MSEBoxModel *bm) {
	if (verbose)
		fprintf(stderr, "Calculating omnivory\n");

	/* Calculate Indices */
	Calc_OI(bm, TL, inshorediet, offshorediet, totareadiet);

	return;
}
/**
 * \brief This calculates the discards
 */
void Calc_Discards(MSEBoxModel *bm, int z, int id) {
	int j;
	double discards, catches;

	discards = 0.0;
	catches = 0.0;
	/* Vertebrates */
	for (j = 0; j < bm->K_num_tot_sp; j++) {
		if (FunctGroupArray[j].isVertebrate == TRUE) {
			discards += stockinfo[sdiscards_id][j][z][id];
			catches += stockinfo[stotcatch_id][j][z][id];
		}
	}
	/* Invertebrates (of interest, ignores infauna, macrophytes and gelatinous zooplankton */
	/* Just taking the fished inverts at present - excluding the impacted-only groups */
	for (j = 0; j < bm->K_num_tot_sp; j++) {
		//if (FunctGroupArray[j].isVertebrate == FALSE && (FunctGroupArray[j].isFished == TRUE || FunctGroupArray[j].isImpacted == TRUE)) {
		if (FunctGroupArray[j].isVertebrate == FALSE && FunctGroupArray[j].isFished == TRUE) {// && FunctGroupArray[j].isEpiFauna == FALSE
			//&& FunctGroupArray[j].isInfauna == FALSE) {
			discards += invstockinfo[fdiscards_id][j][z][id];
			catches += invstockinfo[fcatch_id][j][z][id];
		}
	}
	disrate[z] = discards / (catches + TINY);

	bm->globaldiscards += discards;
	bm->globalcatches += catches;

	/* For bay as a whole */
	if (z == (bm->nfzones - 1))
		disrate[z + 1] = bm->globaldiscards / (bm->globalcatches + TINY);

	return;
}

/**
 * \brief This calculates the discards Potential Biological Removals (PBR)
 * Note: Using mammal method for all vertebrates, with recommended parameter
 * settings from Wade 1998. Only done for vertebrates and cephalopods
 * For purposes of Nmin calculation the population at first data point is taken as N (also summed across the entire bay)
 * Also using average weight of each cephalopod = 1kg (1000g)
 *
 * With
 *
 * Fr = recovery factor
 * FunctGroupArray[j].speciesParams[r_max_id] = population production rate
 *
 */
void Calc_PBR(MSEBoxModel *bm, int z, int id, FILE *ofp) {
	int j;
	double Fr, Nmin, PBR, step1;
	double Totbiom, Nj;
	Fr = 0.5;

	for (j = 0; j < bm->K_num_tot_sp; j++) {
		//TODO: Check with Beth about how to handle this better - should perhaps be a new flag in the functional group input file.
		if (FunctGroupArray[j].groupType == CEP) {
			Totbiom = samplebiom[j][z][id] * zonearea[z] * bm->X_CN * mg_2_g / 0.5;
			Nj = Totbiom / 1000.0;
			Nmin = Nj / (exp(0.842 * sqrt(log(1.0 + 0.5 * 0.5))));
			PBR = Nmin * 0.5 * FunctGroupArray[j].speciesParams[r_max_id] * Fr;
			step1 = (invstockinfo[fcatch_id][j][z][id] * bm->X_CN * mg_2_g / 0.5) / 1000.0;
			PBRcat[j][z] = step1 / (PBR + TINY);

			PBRglobal[j][0] += Nj;
			PBRglobal[j][1] += step1;

		} else if (FunctGroupArray[j].isVertebrate == TRUE) {
			Nmin = stockinfo[sstocknums_id][j][z][id] / (exp(0.842 * sqrt(log(1.0 + 0.25 * 0.25))));
			PBR = Nmin * 0.5 * FunctGroupArray[j].speciesParams[r_max_id] * Fr;
			if ((FunctGroupArray[j].groupType != BIRD) && (FunctGroupArray[j].groupType != MAMMAL))
				step1 = stockinfo[scatchnums_id][j][z][id];
			else
				step1 = stockinfo[scatchnums_id][j][z][id] + stockinfo[sdiscardnums_id][j][z][id];
			PBRcat[j][z] = step1 / (PBR + TINY);

			PBRglobal[j][0] += Nmin;
			PBRglobal[j][1] += step1;
		}
	}
	/* For bay as a whole */
	if (z == (bm->nfzones - 1)) {
		for (j = 0; j < bm->K_num_tot_sp; j++) {
			if (FunctGroupArray[j].groupType == CEP) {
				Nmin = PBRglobal[j][0] / (exp(0.842 * sqrt(log(1.0 + 0.5 * 0.5))));
				PBR = Nmin * 0.5 * FunctGroupArray[j].speciesParams[r_max_id] * Fr;
				PBRcat[j][z + 1] = PBRglobal[j][1] / (PBR + TINY);

			} else if (FunctGroupArray[j].isVertebrate == TRUE) {
				PBR = PBRglobal[j][0] * 0.5 * FunctGroupArray[j].speciesParams[r_max_id] * Fr;
				PBRcat[j][z + 1] = PBRglobal[j][1] / (PBR + TINY);
			}
		}
	}

	return;

}
/**
 * \brief Calculate maximum size of catch indicator. First version has fixed max length
 *  for each group and calculates weighted max length using that and relative contribution
 *  to total catch. Second method uses a similar method but the max length used is not fixed
 *  but based on the actual max size captured. Third method is simply weighted average
 *  size of the animals caught.
 */
void Calc_Max_Size(MSEBoxModel *bm, int z, int id) {
	double totcatch, maxlngth, wgt, lmax, maxwgt, maxlngth1, maxlngth2, prop;
	int i, j;

	/* Method 1 */
	totcatch = 0;
	for (i = 0; i < bm->K_num_tot_sp; i++)
		if (FunctGroupArray[i].isVertebrate == TRUE)
			totcatch += stockinfo[stotcatch_id][i][z][id] / (zonearea[z] + TINY);

	for (i = 0; i < bm->K_num_tot_sp; i++) { // Targeted invertebrates only
		if (FunctGroupArray[i].isVertebrate == FALSE && FunctGroupArray[i].isFished == TRUE) {
			totcatch += invstockinfo[fcatch_id][i][z][id] / (zonearea[z] + TINY);
		}
	}

	maxlngth = 0;
	for (i = 0; i < bm->K_num_tot_sp; i++){
		if (FunctGroupArray[i].isVertebrate == TRUE){
			maxlngth += (stockinfo[stotcatch_id][i][z][id] / (zonearea[z] + TINY)) * FunctGroupArray[i].speciesParams[max_length_id] / (totcatch + TINY);
		}
	}

	for (i = 0; i < bm->K_num_tot_sp; i++) { // Targeted invertebrates only
		if (FunctGroupArray[i].isVertebrate == FALSE && FunctGroupArray[i].isFished == TRUE) {
			maxlngth += (invstockinfo[fcatch_id][i][z][id] / (zonearea[z] + TINY)) * FunctGroupArray[i].speciesParams[max_length_id] / (totcatch + TINY);
		}
	}

	max_lngth[z][0] = maxlngth;

	/* Method 2 and 3 */
	maxlngth = 0;
	maxlngth1 = 0;
	maxlngth2 = 0;
	for (i = 0; i < bm->K_num_tot_sp; i++) {
		if (FunctGroupArray[i].isVertebrate == TRUE) {
			if (FunctGroupArray[i].groupType == BIRD) {
				/* Treat seabirds a little differently to the rest */
				maxlngth += (stockinfo[stotcatch_id][i][z][id] / (zonearea[z] + TINY)) * FunctGroupArray[i].speciesParams[max_length_id] / (totcatch + TINY);
			} else {
				maxwgt = 1;
				for (j = 0; j < bm->K_num_size; j++) {
					prop = individVERTinfo[icatchnums_id][i][j][z][id];
					wgt = individVERTinfo[iweight_id][i][j][z][id];

					if (prop > 0)
						maxwgt = wgt;
					lmax = Get_Length(bm, wgt, i);
					maxlngth2 += (stockinfo[stotcatch_id][i][z][id] / (zonearea[z] + TINY)) * prop * lmax / (totcatch + TINY);
				}
				lmax = Get_Length(bm, maxwgt, i);
				maxlngth1 += (stockinfo[stotcatch_id][i][z][id] / (zonearea[z] + TINY)) * lmax / (totcatch + TINY);
			}
		}
	}
	/* Targeted invertebrates */
	for (i = 0; i < bm->K_num_tot_sp; i++) { // Targeted invertebrates only
		if (FunctGroupArray[i].isVertebrate == FALSE && FunctGroupArray[i].isFished == TRUE && FunctGroupArray[i].isEpiFauna == FALSE
				&& FunctGroupArray[i].isInfauna == FALSE) {
			maxlngth += (invstockinfo[fcatch_id][i][z][id] / (zonearea[z] + TINY)) * FunctGroupArray[i].speciesParams[max_length_id] / (totcatch + TINY);
		}
	}

	max_lngth[z][1] = maxlngth + maxlngth1;
	max_lngth[z][2] = maxlngth + maxlngth2;

	/* For bay as a whole */;
	if (z == (bm->nfzones - 1)) {
		for (i = 0; i < 3; i++)
			max_lngth[bm->nfzones][i] = 0.0;
		for (i = 0; i < bm->nfzones; i++) {
			max_lngth[bm->nfzones][0] += max_lngth[i][0] * zasum[i] / (bm->totarea + TINY);
			max_lngth[bm->nfzones][1] += max_lngth[i][1] * zasum[i] / (bm->totarea + TINY);
			max_lngth[bm->nfzones][2] += max_lngth[i][2] * zasum[i] / (bm->totarea + TINY);
		}
	}

	return;
}
/**
 * \brief Calculate size at maturity indicator for each vertebrate group
 * also calculates average condition (reserve:total weight ratio) for each vertebrate group
 */
void Calc_Size_Mature_And_Condn(MSEBoxModel *bm, int z, int id, FILE *ofp) {
	int j, k, indx;
	double step1, step2, wgt, lngth, coefft, stepA;

	for (indx = 0; indx < bm->K_num_tot_sp; indx++) {
		if (FunctGroupArray[indx].isVertebrate == TRUE) {
			/* Note: Using year classes so use agemat * age classsize */
			j = (int) (FunctGroupArray[indx].speciesParams[age_mat_id] * FunctGroupArray[indx].ageClassSize);
			if (j > bm->K_max_agekey)
				j = bm->K_max_agekey;
			step1 = 0.0;
			step2 = 0.0;
			stepA = 0.0;
			if (FunctGroupArray[indx].groupType != BIRD) {
				for (k = 0; k < bm->K_num_size; k++) {
					coefft = agelengthkey[k][j][indx][z][id];
					step1 += coefft;
				}

				for (k = 0; k < bm->K_num_size; k++) {
					/* Size mature */
					wgt = individVERTinfo[iweight_id][indx][k][z][id];
					coefft = agelengthkey[k][j][indx][z][id];
					lngth = Get_Length(bm, wgt, indx);

					if (step1 > 0.0)
						step2 += lngth * coefft / (step1 + TINY);
					else
						step2 = 0.0;

					/* Condition factor */
					stepA += individVERTinfo[icondn_id][indx][k][z][id] * individVERTinfo[istocknums_id][indx][k][z][id];
				}
			} else {
				for (k = 0; k < 3; k++) {
					/* Size mature */
					wgt = individVERTinfo[iweight_id][indx][k][z][id];
					lngth = Get_Length(bm, wgt, indx);


					step2 += lngth;
					/* Condition factor */
					stepA += individVERTinfo[icondn_id][indx][k][z][id] * individVERTinfo[istocknums_id][indx][k][z][id];
				}
			}

			stockinfo[ssizemat_id][indx][z][id] = step2;
			stockinfo[scondn_id][indx][z][id] = stepA;
		}
	}

	return;
}
/**
 * \brief This calculates the 	size spectra
 *
 * Assumes setup of pelbin of
 * pelbin[0][type] = Bacteria (biomass only)
 * pelbin[1][type] = Phytoplankton (biomass only)
 * pelbin[2][type] = Small zooplankton (biomass only)
 * pelbin[3][type] = Large zooplankton (biomass only)
 * pelbin[4][type] = 0-10cm fish and gelatinous zooplankton
 * pelbin[5][type] = 10-20cm fish and prawns
 * pelbin[6][type] = 20-30cm fish and cephalopods
 * pelbin[7][type] = 30-40 cm fish and other marine vertebrates
 * pelbin[8][type] = 40-50 cm fish and other marine vertebrates
 * pelbin[9][type] = 50-60 cm fish and other marine vertebrates
 * pelbin[10][type] = 60-70 cm fish and other marine vertebrates
 * pelbin[11][type] = 70-80 cm fish and other marine vertebrates
 * pelbin[12][type] = 80-90 cm fish and other marine vertebrates
 * pelbin[13][type] = 90-100 cm fish and other marine vertebrates
 * pelbin[14][type] = 100-110 cm fish and other marine vertebrates
 * pelbin[15][type] = 110-120cm fish and other marine vertebrates
 * pelbin[16][type] = 120-130cm fish and cephalopods (cephalopods biomass only)
 * pelbin[17][type] = 130-140 cm fish and other marine vertebrates
 * pelbin[18][type] = 140-150 cm fish and other marine vertebrates
 * pelbin[19][type] = 150-160 cm fish and other marine vertebrates
 * pelbin[20][type] = 160-170 cm fish and other marine vertebrates
 * pelbin[21][type] = 170-180 cm fish and other marine vertebrates
 * pelbin[22][type] = 180-190 cm fish and other marine vertebrates
 * pelbin[23][type] = 190-200 cm fish and other marine vertebrates
 * pelbin[24][type] = 200+ cm fish and other marine vertebrates
 * pelbin[25][type] = 250+ cm fish and other marine vertebrates
 * pelbin[26][type] = 300+ cm fish and other marine vertebrates
 * pelbin[27][type] = 350+ cm fish and other marine vertebrates
 * pelbin[28][type] = 400+ cm fish and other marine vertebrates
 * pelbin[29][type] = 450+ cm fish and other marine vertebrates
 * pelbin[30][type] = 500+ cm fish and other marine vertebrates
 * pelbin[31][type] = birds < 200g - change pelbinbird below if this entry number changes
 * pelbin[32][type] = birds 200 - 500g
 * pelbin[33][type] = birds 500 - 1000g
 * pelbin[34][type] = birds 1000g+
 *
 * with
 * pelbin[bin][0] = biomass (bioindex_id)
 * pelbin[bin][1] = numbers (abundindex_id)
 *
 * and
 * sedbin[0] = Bacteria
 * sedbin[1] = Microphytobenthos and Meiobenthos
 * sedbin[2] = Macrofauna and Megazoobenthos
 * sedbin[3] = Macrophytes
 */
void Calc_Size_Spectra(MSEBoxModel *bm, int z, int id) {
	int j, k, bin, sizeclass, nextbin, bincount, startbin, sp, nbin, nbinbig, nbinleft, staysmall, staybig, sizeclasstransit, nextbinB, startbinbig;
	double wgt, prop_bin, binbita, numbin, numbinB, prop_binB, biK_num_sizeleft, binsze;
	int pelbinbird = bm->K_num_pelbin - 4;
	int pelbinbig = bm->K_num_pelbin - 11;

	bincount = 0;
	startbinbig = 0;
	/* TODO: Check with Beth about  this */
	nbinleft = 0;

	for (j = 0; j < bm->K_num_pelbin; j++)
		for (k = 0; k < 2; k++)
			pelbin[j][k][z] = 0;

	for (j = 0; j < bm->K_num_sedbin; j++)
		sedbin[j][z] = 0;

	/* The invertebrate types */
	for (sp = 0; sp < bm->K_num_tot_sp; sp++) {
		if (FunctGroupArray[sp].isVertebrate == FALSE) {
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

			case PL_BACT:
				pelbin[0][bioindex_id][z] += samplebiom[sp][z][id];
				pelbin[0][abundindex_id][z] += samplebiom[sp][z][id] / (FunctGroupArray[sp].speciesParams[avg_inv_size_id] + TINY);
				break;
			case LG_PHY:
			case DINOFLAG:
			case SM_PHY:
				pelbin[1][bioindex_id][z] += samplebiom[sp][z][id];
				pelbin[1][abundindex_id][z] += samplebiom[sp][z][id] / (FunctGroupArray[sp].speciesParams[avg_inv_size_id] + TINY);
				break;

			case SM_ZOO:
				pelbin[2][bioindex_id][z] += samplebiom[sp][z][id];
				pelbin[2][abundindex_id][z] += samplebiom[sp][z][id] / (FunctGroupArray[sp].speciesParams[avg_inv_size_id] + TINY);
				break;
			case MED_ZOO:
				pelbin[4][bioindex_id][z] += samplebiom[sp][z][id];
				pelbin[4][abundindex_id][z] += samplebiom[sp][z][id] / (FunctGroupArray[sp].speciesParams[avg_inv_size_id] + TINY);

//				pelbin[3][bioindex_id][z] += samplebiom[sp][z][id];
//				pelbin[3][abundindex_id][z] += samplebiom[sp][z][id] / (FunctGroupArray[sp].speciesParams[avg_inv_size_id] + TINY);

				break;
			case LG_ZOO:
				pelbin[3][bioindex_id][z] += samplebiom[sp][z][id];
				pelbin[3][abundindex_id][z] += samplebiom[sp][z][id] / (FunctGroupArray[sp].speciesParams[avg_inv_size_id] + TINY);
				break;
			case CEP:
				for (j = 5; j < 7; j++) {
					if (j == 5)
						prop_bin = 0.3;
					else
						prop_bin = 0.7;
					pelbin[j][bioindex_id][z] += prop_bin * samplebiom[sp][z][id];
					pelbin[j][abundindex_id][z] += prop_bin * samplebiom[sp][z][id] / (FunctGroupArray[sp].speciesParams[avg_inv_size_id] + TINY);
				}
				break;
			case PWN:
				pelbin[5][bioindex_id][z] += samplebiom[sp][z][id];
				pelbin[5][abundindex_id][z] += samplebiom[sp][z][id] / (FunctGroupArray[sp].speciesParams[avg_inv_size_id] + TINY);
				break;

				/* Bentic groups */
			case SED_BACT:
				sedbin[0][z] += samplebiom[sp][z][id];
				break;
			case SM_INF:
			case MICROPHTYBENTHOS:
				sedbin[1][z] += samplebiom[sp][z][id];
                break;
			case LG_INF:
			case SED_EP_FF:
			case SED_EP_OTHER:
			case MOB_EP_OTHER:
			case CORAL:
            case SPONGE:
				sedbin[2][z] += samplebiom[sp][z][id];
				break;
			case PHYTOBEN:
			case SEAGRASS:
			case TURF:
				sedbin[3][z] += samplebiom[sp][z][id];
				break;
			case CARRION:
			case REF_DET:
			case LAB_DET:
				/* do nothing */
				break;
			case ICE_BACT:
			case ICE_DIATOMS:
			case ICE_MIXOTROPHS:
			case ICE_ZOOBIOTA:
				/* TODO: How handle ice indices? */
				break;
			}
		} else {

		}
	}

	/* Vertebrates - FIX ---- due to lack of time had to hardwire assumption that
	 biomass bins = multiple of size bins (or vice versa). If they don't fit
	 so smoothly it currently dumps, but may want to put thought into fixing
	 this */
	for (sp = 0; sp < bm->K_num_tot_sp; sp++) {
		if (FunctGroupArray[sp].isVertebrate == TRUE) {
			if(FunctGroupArray[sp].groupType == BIRD){
				/* Birds */
				for (sizeclass = 0; sizeclass < 3; sizeclass++) {
					wgt = individVERTinfo[iweight_id][sp][sizeclass][z][id] * mg_2_g * bm->X_CN;
					if (wgt < 200.0) {
						bin = 0;
					} else if (wgt < 500.0) {
						bin = 1;
					} else if (wgt < 1000.0) {
						bin = 2;
					} else {
						bin = 3;
					}
					pelbin[pelbinbird + bin][bioindex_id][z] += individVERTinfo[ibiomass_id][sp][sizeclass][z][id];
					pelbin[pelbinbird + bin][abundindex_id][z] += individVERTinfo[istocknums_id][sp][sizeclass][z][id] * stockinfo[sstocknums_id][sp][z][id];
				}
			} else {
				/* All other vertebrates */
				staybig = 0;
				staysmall = 0;
				/* Determine starting bin and number of size classes per size bin */
				if (FunctGroupArray[sp].speciesParams[allometic_bin_start_id] < 210.0) {
					startbin = (int) (floor(FunctGroupArray[sp].speciesParams[allometic_bin_start_id] / 10.0));
					numbin = FunctGroupArray[sp].speciesParams[allometic_bin_size_id] / 10.0;
					staysmall = 1;
					if (FunctGroupArray[sp].speciesParams[allometic_bin_size_id] * (bm->K_num_size - 1) > 210.0)
						staysmall = 0;
				} else {
					startbin = (int) (floor((FunctGroupArray[sp].speciesParams[allometic_bin_start_id] - 200.0) / 50.0)) + pelbinbig;
					numbin = FunctGroupArray[sp].speciesParams[allometic_bin_size_id] / 50.0;
					staybig = 1;
				}

				if (numbin > 1)
					prop_bin = 1.0 / numbin;
				else {
					prop_bin = 1.0;
				}
				nbin = (int) (numbin);

				/* If all within either the 10cm or 50cm bins without overlapping the transition */
				if (staysmall || staybig) {
					for (sizeclass = 0; sizeclass < bm->K_num_size; sizeclass++) {
						nextbin = (int) floor(ROUNDGUARD + (4 + startbin + sizeclass * numbin)); // 4 is the number of plankton bins
						if (nextbin > pelbinbird - 1)
							nextbin = pelbinbird - 1;
						for (k = 0; k < nbin; k++) {
							bincount = nextbin + k;
							if (bincount > pelbinbird - 1)
								bincount = pelbinbird - 1;
							pelbin[bincount][bioindex_id][z] += prop_bin * individVERTinfo[ibiomass_id][sp][sizeclass][z][id];
							pelbin[bincount][abundindex_id][z] += prop_bin * individVERTinfo[istocknums_id][sp][sizeclass][z][id] * stockinfo[sstocknums_id][sp][z][id];
						}
					}
				} else {
					/* Set up info for transition case */
					numbinB = FunctGroupArray[sp].speciesParams[allometic_bin_size_id] / 50.0;
					if (numbinB > 1)
						prop_binB = 1.0 / numbinB;
					else {
						prop_binB = 1.0;
					}
					nbinbig = (int) (numbinB);

					sizeclasstransit = 0;
					for (sizeclass = 0; sizeclass < bm->K_num_size - 1; sizeclass++) {
						nextbin = (int) floor(ROUNDGUARD + (4 + startbin + sizeclass * numbin)); // 4 is the number of plankton bins
						nextbinB = (int) floor(ROUNDGUARD + (4 + startbin + (sizeclass + 1) * numbin));
						if (nextbin <= pelbinbig && nextbinB > pelbinbig) {
							sizeclasstransit = sizeclass;
							break;
						}
					}

					/* All 10cm cases */
					for (sizeclass = 0; sizeclass < sizeclasstransit; sizeclass++) {
						nextbin = (int) floor(ROUNDGUARD + (4 + startbin + sizeclass * numbin));
						for (k = 0; k < nbin; k++) {
							bincount = nextbin + k;
							if (bincount > pelbinbird - 1)
								bincount = pelbinbird - 1;
							pelbin[bincount][bioindex_id][z] += prop_bin * individVERTinfo[ibiomass_id][sp][sizeclass][z][id];
							pelbin[bincount][abundindex_id][z] += prop_bin * individVERTinfo[istocknums_id][sp][sizeclass][z][id] * stockinfo[sstocknums_id][sp][z][id];
						}
					}

					/* Transition case */
					nextbin = (int) floor(ROUNDGUARD + (4 + startbin + sizeclasstransit * numbin));
					for (k = 0; k < nbin; k++) {
						bincount = nextbin + k;
						if (bincount > pelbinbig) {
							startbinbig = pelbinbig;
							nbinleft = nbin - k;
							break;
						}
						pelbin[bincount][bioindex_id][z] += prop_bin * individVERTinfo[ibiomass_id][sp][sizeclasstransit][z][id];
						pelbin[bincount][abundindex_id][z] += prop_bin * individVERTinfo[istocknums_id][sp][sizeclasstransit][z][id] * stockinfo[sstocknums_id][sp][z][id];
					}
					biK_num_sizeleft = numbin * nbinleft / (nbin + TINY);
					binsze = biK_num_sizeleft / 5;
					binbita = ceil(binsze);
					if (binbita > 1) {
						prop_bin = 1.0 / binsze;
					} else {
						prop_bin = 1.0;
					}

					for (k = 0; k < binbita; k++) {
						bincount = startbinbig + k;
						if (startbinbig > pelbinbird - 1)
							bincount = pelbinbird - 1;
						pelbin[bincount][bioindex_id][z] += prop_bin * individVERTinfo[ibiomass_id][sp][sizeclasstransit][z][id];
						pelbin[bincount][abundindex_id][z] += prop_bin * individVERTinfo[istocknums_id][sp][sizeclasstransit][z][id] * stockinfo[sstocknums_id][sp][z][id];
					}
					startbinbig = bincount;

					/* All 50cm cases */
					for (sizeclass = sizeclasstransit + 1; sizeclass < bm->K_num_size; sizeclass++) {
						nextbin = (int) floor(ROUNDGUARD + (4 + startbinbig + sizeclass * numbinB));
						for (k = 0; k < nbinbig; k++) {
							bincount = nextbin + k;
							if (bincount > pelbinbird - 1)
								bincount = pelbinbird - 1;
							pelbin[bincount][bioindex_id][z] += prop_binB * individVERTinfo[ibiomass_id][sp][sizeclass][z][id];
							pelbin[bincount][abundindex_id][z] += prop_binB * individVERTinfo[istocknums_id][sp][sizeclass][z][id] * stockinfo[sstocknums_id][sp][z][id];

						}
					}
				}
			}
		}

	}

	for (bin = 0; bin < bm->K_num_pelbin; bin++)
		for (j = 0; j < 2; j++)
			pelbin[bin][j][bm->nfzones] += pelbin[bin][j][z];

	for (bin = 0; bin < bm->K_num_sedbin; bin++)
		sedbin[bin][bm->nfzones] += sedbin[bin][z];

	/* FIX - add calculation of the slope */

	return;
}
/**
 * \brief Sort the nums values - this function will include the microfauna.
 *
 *
 */
static void sort_values(MSEBoxModel *bm, int index, int z) {
	int sp, chrt;
	int rank;
	int counter = 0;
	for (sp = 0; sp < bm->K_num_tot_sp; sp++) {
		numsort[sp] = 0;
		if (FunctGroupArray[sp].isMacroFauna == TRUE || FunctGroupArray[sp].isMicroFauna == TRUE) {
			counter++;
		}
	}

	/* First for numbers */
	for (sp = 0; sp < bm->K_num_tot_sp; sp++) {
		/* Skip det and sediment bacteria */
		if (FunctGroupArray[sp].isMacroFauna == TRUE || FunctGroupArray[sp].isMicroFauna == TRUE) {
			rank = 0;
			for (chrt = 0; chrt < bm->K_num_tot_sp; chrt++) {
				if (FunctGroupArray[chrt].isMacroFauna == TRUE || FunctGroupArray[chrt].isMicroFauna == TRUE) {
					if (nums[sp][index] > nums[chrt][index])
						rank++;
				}
			}
			numsort[sp] = counter - 1 - rank;
		}
	}

	/* Update final ranked values */
	for (sp = 0; sp < bm->K_num_tot_sp; sp++) {
		if (FunctGroupArray[sp].isVertebrate == TRUE || FunctGroupArray[sp].isMacroFauna == TRUE || FunctGroupArray[sp].isMicroFauna == TRUE) {
			endnums[numsort[sp]][index][z] = nums[sp][index];
		}
	}
}
/**
 * \brief Sort the nums values - this function will not include the microfauna.
 *
 *
 */
static void sort_values_big(MSEBoxModel *bm, int index, int z) {
	int sp, chrt;
	int rank;
	int counter = 0;

	for (sp = 0; sp < bm->K_num_tot_sp; sp++) {
		numsort[sp] = 0;
		if (FunctGroupArray[sp].isMacroFauna == TRUE) {
			counter++;
		}
	}
	/* First for numbers */
	for (sp = 0; sp < bm->K_num_tot_sp; sp++) {
		/* Skip det and sediment bacteria */
		if (FunctGroupArray[sp].isMacroFauna == TRUE) {
			rank = 0;
			for (chrt = 0; chrt < bm->K_num_tot_sp; chrt++) {
				if (FunctGroupArray[chrt].isMacroFauna == TRUE) {
					if (nums[sp][index] > nums[chrt][index])
						rank++;

				}
			}
			numsort[sp] = counter - 1 - rank;
		}
	}

	/* Update final ranked values */
	for (sp = 0; sp < bm->K_num_tot_sp; sp++) {
		if (FunctGroupArray[sp].isMacroFauna == TRUE) {
			endnumsbig[numsort[sp]][index][z] = nums[sp][index];
		}
	}
}

/**/
static void sort_global_values(MSEBoxModel *bm, int index, int z, int typeIndex) {
	int sp, chrt, rank;
	int counter = 0;

	for (sp = 0; sp < bm->K_num_tot_sp; sp++) {
		numsort[sp] = 0;
		if (FunctGroupArray[sp].isVertebrate == TRUE || FunctGroupArray[sp].isMacroFauna == TRUE || FunctGroupArray[sp].isMicroFauna == TRUE) {
			counter++;
		}
	}

	/* First for numbers */
	for (sp = 0; sp < bm->K_num_tot_sp; sp++) {
		/* Skip det and sediment bacteria */
		if (FunctGroupArray[sp].isVertebrate == TRUE || FunctGroupArray[sp].isMacroFauna == TRUE || FunctGroupArray[sp].isMicroFauna == TRUE) {
			rank = 0;
			for (chrt = 0; chrt < bm->K_num_tot_sp; chrt++) {
				if (FunctGroupArray[chrt].isVertebrate == TRUE || FunctGroupArray[chrt].isMacroFauna == TRUE || FunctGroupArray[chrt].isMicroFauna == TRUE) {
					if (globalnums[sp][index][typeIndex] > globalnums[chrt][index][typeIndex])
						rank++;

				}
			}
			numsort[sp] = counter - 1 - rank;
		}
	}

	/* Update final ranked values */
	for (sp = 0; sp < bm->K_num_tot_sp; sp++) {
		if (FunctGroupArray[sp].isVertebrate == TRUE || FunctGroupArray[sp].isMacroFauna == TRUE || FunctGroupArray[sp].isMicroFauna == TRUE) {
			endnums[numsort[sp]][index][z] = globalnums[sp][index][typeIndex];
		}
	}
}

/**
 * \brief This function is used to sort the values for the 'large' groups.
 * The groups are only considered if the groups are vertebrates or macrofauna.
 *
 */
static void sort_global_values_big(MSEBoxModel *bm, int index, int z, int typeIndex) {
	int sp, chrt, rank;
	int counter = 0;

	for (sp = 0; sp < bm->K_num_tot_sp; sp++) {
		numsort[sp] = 0;
		if (FunctGroupArray[sp].isVertebrate == TRUE || FunctGroupArray[sp].isMacroFauna == TRUE) {
			counter++;
		}
	}
	/* First for numbers */
	for (sp = 0; sp < bm->K_num_tot_sp; sp++) {
		/* Skip det and sediment bacteria */
		if (FunctGroupArray[sp].isVertebrate == TRUE || FunctGroupArray[sp].isMacroFauna == TRUE) {
			rank = 0;
			for (chrt = 0; chrt < bm->K_num_tot_sp; chrt++) {
                if (globalnums[sp][index][typeIndex] > globalnums[chrt][index][typeIndex]) {
                    rank++;
                }
			}
			numsort[sp] = counter - 1 - rank;
		}
	}

	/* Update final ranked values */
	for (sp = 0; sp < bm->K_num_tot_sp; sp++) {
		if (FunctGroupArray[sp].isVertebrate == TRUE || FunctGroupArray[sp].isMacroFauna == TRUE) {
			endnumsbig[numsort[sp]][index][z] = globalnums[sp][index][typeIndex];
		}
	}
}

/**
 * \brief This calculates the ABC curves
 * Note: omitting benthic primary producers and bacteria (so K_num_live = K_num_vert_sp + MAinv_id) assumes:
 *
 * nums[id][trait] where id is from setConstantID() in datareading.c
 * and
 * nums[rank][0] = numbers;
 * nums[rank][1] = biomass;
 */
void Calc_ABC(MSEBoxModel *bm, int z, int id) {
	int sp, chrt;

	for (sp = 0; sp < bm->K_num_tot_sp; sp++) {
		for (chrt = 0; chrt < FunctGroupArray[sp].numStages; chrt++)
			nums[sp][chrt] = 0;
	}

	for (sp = 0; sp < bm->K_num_tot_sp; sp++) {
		/* Skip det and sediment bacteria */
		if (FunctGroupArray[sp].isDetritus == TRUE || FunctGroupArray[sp].isBacteria == TRUE)
			continue;

		if (FunctGroupArray[sp].isVertebrate == TRUE) {
			nums[sp][0] = stockinfo[sstocknums_id][sp][z][id] / (zonearea[z] + TINY);
			nums[sp][1] = stockinfo[sbiomass_id][sp][z][id] / (zonearea[z] + TINY);

		} else if (FunctGroupArray[sp].isMacroFauna == TRUE || FunctGroupArray[sp].isMicroFauna == TRUE) {
			nums[sp][0] = samplebiom[sp][z][id] / (FunctGroupArray[sp].speciesParams[avg_inv_size_id] + TINY);
			nums[sp][1] = samplebiom[sp][z][id];

//			fprintf(bm->logFile, "nums[%s][0] = %e, samplebiom[sp][z][id] = %e, FunctGroupArray[sp].speciesParams[avg_inv_size_id] = %e\n",FunctGroupArray[sp].groupCode, nums[sp][0], samplebiom[sp][z][id], FunctGroupArray[sp].speciesParams[avg_inv_size_id]);
//			fprintf(bm->logFile, "nums[%s][1] = %e\n",FunctGroupArray[sp].groupCode, nums[sp][1]);

		}
	}

	/* Don't bother checking groups as we don't care about adding 0 values */
	for (sp = 0; sp < bm->K_num_tot_sp; sp++) {
		for (chrt = 0; chrt < FunctGroupArray[sp].numStages; chrt++)
			globalnums[sp][chrt][0] += nums[sp][chrt];
	}

	/* First for numbers */
	sort_values(bm, 0, z);

	/* Now for biomass */
	sort_values(bm, 1, z);

	/* Repeat without microfauna */
	for (sp = 0; sp < bm->K_num_tot_sp; sp++) {
        for (chrt = 0; chrt < FunctGroupArray[sp].numStages; chrt++) {
            nums[sp][chrt] = 0;
        }
	}

	for (sp = 0; sp < bm->K_num_tot_sp; sp++) {
		/* Skip det and sediment bacteria */
		if (FunctGroupArray[sp].isMacroFauna == TRUE) {
			nums[sp][0] = samplebiom[sp][z][id] / (FunctGroupArray[sp].speciesParams[avg_inv_size_id] + TINY);
			nums[sp][1] = samplebiom[sp][z][id];

		}
		if (FunctGroupArray[sp].isVertebrate == TRUE) {
			nums[sp][0] = stockinfo[sstocknums_id][sp][z][id] / (zonearea[z] + TINY);
			nums[sp][1] = stockinfo[sbiomass_id][sp][z][id] / (zonearea[z] + TINY);
		}
	}

	for (sp = 0; sp < bm->K_num_tot_sp; sp++) {
		for (chrt = 0; chrt < FunctGroupArray[sp].numStages; chrt++) {
			globalnums[sp][chrt][1] += nums[sp][chrt];
		}
	}

	/* First for numbers */
	sort_values_big(bm, abundindex_id, z);

	/* Now for biomass */
	sort_values_big(bm, bioindex_id, z);

	/* For bay as a whole */
	if (z == (bm->nfzones - 1)) {

		/* First for numbers */
		sort_global_values(bm, abundindex_id, bm->nfzones, 0);

		/* Then biomass */
		sort_global_values(bm, bioindex_id, bm->nfzones, 0);

		/* Wihout microfauna sort into descending order */
		/* First for numbers */
		sort_global_values_big(bm, abundindex_id, bm->nfzones, 1);

		/* Then biomass */
		sort_global_values_big(bm, bioindex_id, bm->nfzones, 1);

	}

	/* FIX - add calculation of W statistic */

	return;
}

/**
 * \brief This calculates the discards Habitat complexity
 * Using infauna / epifauna as index 1
 * Using total epifauna as index 2
 * Note: omitting bacteria and microphytobenthos from infauna
 */
void Calc_Hab_Complx(MSEBoxModel *bm, int z, int id) {
	int sp = 0;
	double step1, step2;

	step1 = 0.0;
	step2 = 0.0;

	for (sp = 0; sp < bm->K_num_tot_sp; sp++) {
		if (FunctGroupArray[sp].isInfauna == TRUE) {
			step1 += samplebiom[sp][z][id];
		} else if (FunctGroupArray[sp].isEpiFauna == TRUE) {
			step2 += samplebiom[sp][z][id];
		}
	}

	habindx[0][z] = step1 / (step2 + TINY);
	habindx[1][z] = step2;

	bm->globalHabstep1 += step1;
	bm->globalHabstep2 += step2;

	/* For bay as a whole */
	if (z == (bm->nfzones - 1)) {
		habindx[0][z + 1] = bm->globalHabstep1 / (bm->globalHabstep2 + TINY);
		habindx[1][z + 1] = bm->globalHabstep2;
	}

	return;

}

/**
 * \brief This calculates the reproductive success
 * As a proxy for now using age 1 / age 0 for birds
 * For mammals use age mature / age 0 as well as age weaned / age 0
 * fledge assumes x < bm->K_num_charis:
 * fledge[x][zone] = proportion reaching maturity
 * fledge[2x][zone] = proportion reaching weaning
 */
void Calc_Reprod_Success(MSEBoxModel *bm, int z, int id, FILE *ofp) {
	int k, chrt, sp, matage, jj;
	double step1, step2, step3, newnums, scalarsum;

	for (sp = 0; sp < bm->K_num_tot_sp; sp++) {
		if(FunctGroupArray[sp].speciesParams[flag_id] == TRUE){
			if(FunctGroupArray[sp].groupType == BIRD || FunctGroupArray[sp].groupType == MAMMAL){
				/* Get young of year values */
				if (!firstdata) {
					step1 = oldbaby[sp][z][id];
				} else {
					if(FunctGroupArray[sp].groupType == BIRD){
						step1 = stockinfo[sstocknums_id][sp][z][id] * individVERTinfo[istocknums_id][sp][0][z][id];
						if (step1 == 0.0)
							step1 = minpool;
					} else {
						step1 = 0.0;
						/* Note: Using year classes so use agemat * age classsize */
						matage = (int) floor(ROUNDGUARD + (FunctGroupArray[sp].speciesParams[age_mat_id] * FunctGroupArray[sp].ageClassSize));
						if (matage > bm->K_max_agekey)
							matage = bm->K_max_agekey;
						for (jj = 0; jj < matage; jj++) {
							scalarsum = 0;
							for (k = 0; k < bm->K_num_size; k++) {
								newnums = stockinfo[sstocknums_id][sp][z][id] * individVERTinfo[istocknums_id][sp][k][z][id] * agelengthkey[k][sp][jj][z][id];
								scalarsum += individVERTinfo[istocknums_id][sp][k][z][id] * agelengthkey[k][jj][sp][z][id];
								step1 += newnums;
							}
						}
						if (step1 == 0.0)
							step1 = minpool;
					}
				}
				/* Determine older groups (e.g. fledglings) */
				if(FunctGroupArray[sp].groupType == BIRD){
					step2 = stockinfo[sstocknums_id][sp][z][id] * individVERTinfo[istocknums_id][sp][1][z][id];
					step3 = step2;
				} else {
					step2 = 0.0;
					chrt = (int) (FunctGroupArray[sp].speciesParams[age_mat_id] * FunctGroupArray[sp].ageClassSize - 1);
					if (chrt > bm->K_max_agekey)
						chrt = bm->K_max_agekey;
					for (k = 0; k < bm->K_num_size; k++) {
						newnums = stockinfo[sstocknums_id][sp][z][id] * individVERTinfo[istocknums_id][sp][k][z][id] * agelengthkey[k][chrt][sp][z][id];
						step2 += newnums;
					}

					step3 = 0.0;
					chrt = 2;
					for (k = 0; k < bm->K_num_size; k++) {
						newnums = stockinfo[sstocknums_id][sp][z][id] * individVERTinfo[istocknums_id][sp][k][z][id] * agelengthkey[k][chrt][sp][z][id];
						step3 += newnums;
					}
				}

				fledge[sp][z] = step2 / (step1 + TINY);
				fledge[sp + bm->K_num_tot_sp][z] = step3 / (step1 + TINY);
				globalfledge[0][sp] += step1;
				globalfledge[1][sp] += step2;
				globalfledge[2][sp] += step3;
			}
		}
	}

	/* For bay as a whole */
	if (z == (bm->nfzones - 1)) {
		for (sp = 0; sp < bm->K_num_tot_sp; sp++) {
			if(FunctGroupArray[sp].groupType == BIRD || FunctGroupArray[sp].groupType == MAMMAL){
				fledge[sp][z + 1] = globalfledge[1][sp] / (globalfledge[0][sp] + TINY);
				fledge[sp + bm->K_num_tot_sp][z + 1] = globalfledge[2][sp] / (globalfledge[0][sp] + TINY);
			}
		}
	}

	return;
}
/**
 * \brief This calculates chl ns din in water column
 *
 * samplephys[5] is DIN and sampleprocess[2] is chla
 */
void Calc_Monbet(MSEBoxModel *bm, int z, int id)

{
	monbet[0][z] = sampleprocess[2][z][id];
	monbet[1][z] = samplephys[din_wc_id - 2][z][id];
	monbet[0][bm->nfzones] += sampleprocess[2][z][id] * (zonearea[z] / (bm->totarea + TINY));
	monbet[1][bm->nfzones] += samplephys[din_wc_id - 2][z][id] * (zonearea[z] / (bm->totarea + TINY));

	return;
}
/**
 * \brief This calculates P:D and PS:ZP
 * These are ratios of differing function groups in the fish community
 * P:D = pelagic fish : demersal fish
 * 1st PS:ZP = piscivorous fish : planktivorous fish (not counting demersal fish as piscivorous fish)
 * 2nd PS:ZP = piscivorous fish : planktivorous fish (counting demersal fish as piscivorous fish)
 */
void Calc_PD_Ratio(MSEBoxModel *bm, int z, int id) {
	int sp;
	double step1, step2;

	step1 = 0.0;
	step2 = 0.0;
	for (sp = 0; sp < bm->K_num_tot_sp; sp++) {
		if (FunctGroupArray[sp].isVertebrate == TRUE) {
			if (FunctGroupArray[sp].speciesParams[flagdem_id] < 1)
				/* Pelagic fish biomass */
				step1 += stockinfo[sbiomass_id][sp][z][id];
			else
				/* Demersal fish biomass */
				step2 += stockinfo[sbiomass_id][sp][z][id];
		}
	}

	pd[0][z] = step1 / (step2 + TINY);
	globalpd[0][0] += step1;
	globalpd[1][0] += step2;

	step1 = 0.0;
	step2 = 0.0;
	for (sp = 0; sp < bm->K_num_tot_sp; sp++) {
		if (FunctGroupArray[sp].isVertebrate == TRUE) {
			if (FunctGroupArray[sp].speciesParams[flagdem_id] < 1) {
				if(FunctGroupArray[sp].speciesParams[flagplankfish_id])
					/* Planktivorous surface fish biomass */
					step1 += stockinfo[sbiomass_id][sp][z][id];
				else if (FunctGroupArray[sp].groupType == FISH)
					/* Pisivorous non-demersal fish biomass */
					step2 += stockinfo[sbiomass_id][sp][z][id];
			}
		}
	}

	pd[1][z] = step1 / (step2 + TINY);
	globalpd[0][1] += step1;
	globalpd[1][1] += step2;

	step1 = 0.0;
	step2 = 0.0;
	for (sp = 0; sp < bm->K_num_tot_sp; sp++) {
		if (FunctGroupArray[sp].isVertebrate == TRUE) {
			if(FunctGroupArray[sp].speciesParams[flagplankfish_id])
				/* Planktivorous fish biomass */
				step1 += stockinfo[sbiomass_id][sp][z][id];
			else if (FunctGroupArray[sp].groupType == FISH)
				/* Piscivorous fish biomass */
				step2 += stockinfo[sbiomass_id][sp][z][id];
		}
	}

	pd[2][z] = step1 / (step2 + TINY);
	globalpd[0][2] += step1;
	globalpd[1][2] += step2;

	step1 = 0.0;
	step2 = 0.0;
	for (sp = 0; sp < bm->K_num_tot_sp; sp++) {
		if (FunctGroupArray[sp].isVertebrate == TRUE) {
			if (FunctGroupArray[sp].speciesParams[flagdem_id] < 1)
				/* Pelagic fish catch */
				step1 += stockinfo[stotcatch_id][sp][z][id];
			else
				/* Demersal fish catch */
				step2 += stockinfo[stotcatch_id][sp][z][id];
		}
	}

	pdcat[0][z] = step1 / (step2 + TINY);
	globalpd[0][3] += step1;
	globalpd[1][3] += step2;

	step1 = 0.0;
	step2 = 0.0;
	for (sp = 0; sp < bm->K_num_tot_sp; sp++) {
		if (FunctGroupArray[sp].isVertebrate == TRUE) {
			if (FunctGroupArray[sp].speciesParams[flagdem_id] < 1) {
				if(FunctGroupArray[sp].speciesParams[flagplankfish_id])
					/* Planktivorous surface fish catch */
					step1 += stockinfo[stotcatch_id][sp][z][id];
				else if (FunctGroupArray[sp].groupType == FISH)
					/* Pisivorous non-demersal fish catch */
					step2 += stockinfo[stotcatch_id][sp][z][id];
			}
		}
	}

	pdcat[1][z] = step1 / (step2 + TINY);
	globalpd[0][4] += step1;
	globalpd[1][4] += step2;

	step1 = 0.0;
	step2 = 0.0;
	for (sp = 0; sp < bm->K_num_tot_sp; sp++) {
		if (FunctGroupArray[sp].isVertebrate == TRUE) {
			if(FunctGroupArray[sp].speciesParams[flagplankfish_id])
				/* Planktivorous fish catch */
				step1 += stockinfo[stotcatch_id][sp][z][id];
			else if (FunctGroupArray[sp].groupType == FISH)
				/* Piscivorous fish catch */
				step2 += stockinfo[stotcatch_id][sp][z][id];
		}
	}
	pdcat[2][z] = step1 / (step2 + TINY);
	globalpd[0][5] += step1;
	globalpd[1][5] += step2;

	/* For bay as a whole */
	if (z == (bm->nfzones - 1)) {
		for (sp = 0; sp < 3; sp++) {
			pd[sp][z + 1] = globalpd[0][sp] / (globalpd[1][sp] + TINY);
			pdcat[sp][z + 1] = globalpd[0][3 + sp] / (globalpd[1][3 + sp] + TINY);
		}
	}

	return;

}
/**
 * \brief This calculates NPP/B
 * Net primary production / totbiomass (omitting detritus)
 */
void Calc_Nppb(MSEBoxModel *bm, int z, int id) {
	int sp;
	double step1, step2, stepinv, stepvert;

	step1 = 0.0;
	step2 = 0.0;
	stepinv = 0.0;
	stepvert = 0.0;
	for (sp = 0; sp < bm->K_num_tot_sp; sp++) {
		if (FunctGroupArray[sp].isVertebrate == FALSE) {

			if (FunctGroupArray[sp].isPrimaryProducer == TRUE) {
				step1 += sampleprod[sp][z][id];
			}
			if (FunctGroupArray[sp].isDetritus == FALSE) {
				stepinv += samplebiom[sp][z][id];
				step2 += samplebiom[sp][z][id];
			}
		} else {
			stepvert += stockinfo[sbiomass_id][sp][z][id] / (zonearea[z] + TINY);
			step2 += stockinfo[sbiomass_id][sp][z][id] / (zonearea[z] + TINY);
		}
	}

	nppb[z] = step1 / (step2 + TINY);
	bm->globalstep1 += step1;
	bm->globalstep2 += step2;

	/* Total biomass records */
	totsamplebiom[0][z] = stepinv;
	totsamplebiom[1][z] = stepvert;
	totsamplebiom[2][z] = stepinv + stepvert;
	totsamplebiom[0][bm->nfzones] += stepinv;
	totsamplebiom[1][bm->nfzones] += stepvert;
	totsamplebiom[2][bm->nfzones] += stepinv + stepvert;

	/* For bay as a whole */
	if (z == (bm->nfzones - 1)) {
		nppb[z + 1] = bm->globalstep1 / (bm->globalstep2 + TINY);
	}

	return;

}

/**
 * \brief - Calculate the sigma for the physical properties.
 *
 */
void Calculate_Phys_SD(MSEBoxModel *bm, int z) {
	int i;
	double step1, step2;

	for (i = 0; i < num_sampled_phy_id; i++) {
		step1 = cvphys[1][i][z] - ((cvphys[0][i][z] * cvphys[0][i][z]) / cvphys[2][i][z]);
		step2 = step1 / (cvphys[2][i][z] - 1.0 + TINY);
		physicalSigma[i][z] = sqrt(step2);
	}
}

/**
 * \brief This calculates Total coefficient of variation
 *
 * Omitting detritus (i.e. live groups only)
 */
void Calc_CV(MSEBoxModel *bm, int z) {
	int j;
	double mean, step1, step2, cv;
	double sigma;

	cvt[z] = 0.0;

	for (j = 0; j < bm->K_num_tot_sp; j++) {
		if (FunctGroupArray[j].isDetritus == FALSE) {
			mean = cvsample[0][j][z] / (cvsample[2][j][z] + TINY);
			step1 = cvsample[1][j][z] - (mean * mean);
			step2 = step1 / (cvsample[2][j][z] - 1.0 + TINY);
			sigma = sqrt(step2);

			cv = 100.0 * sigma / (mean + TINY);

			cvt[z] += cv;    // TODO: Fix this to be cv calculated on total biomass in then zone
			cvt[bm->nfzones] += cv; // TODO: Fix this to be cv calculated on aggegate values
		}
	}

	return;

}
/**
 * \brief This calculates Average trophic level and assumes:
 * avgtl[0][z] = mean trophic level of the catch
 * avgtl[1][z] = mean trophic level of the system
 * avgtl[2][z] = mean trophic level of the catch using constant (fixed at inital level) trophic level for each group
 * avgtl[3][z] = mean trophic level of the system using constant (fixed at inital level) trophic level for each group
 * avgtl[4][z] = mean trophic level of catch vertebrates only
 * avgtl[5][z] = mean trophic level of catch vertebrates only using constant trophic level for each group
 * avgtl[6][z] = mean trophic level of vertebrates
 * avgtl[8][z] = mean trophic level of vertebrates using a constant trophic level for each group
 */
void Calc_Avgtl(MSEBoxModel *bm, double ***tl, int id, FILE *ofp) {
	int z, bb, b, a, sp, stage;
	double catches, sample, d, biom, totcatch, totbiom, catchesST, biomST, totcatchA, totcatchAST, biomA, biomAST;
	int habitat;

	if (bm->dayt == bm->tassessstart) {
		for (z = 0; z < bm->K_num_tot_sp; z++) {
			for (stage = 0; stage < FunctGroupArray[z].numStages; stage++) {
				tl[3][z][stage] = tl[0][z][stage];
				tl[4][z][stage] = tl[1][z][stage];
				tl[5][z][stage] = tl[2][z][stage];
			}
		}
	}

	for (z = 0; z < bm->nfzones + 1; z++)
		for (bb = 0; bb < 8; bb++)
			avgtl[bb][z] = 0.0;

	for (bb = 0; bb < bm->nsbox; bb++) {
		b = bm->nsboxes[bb];
		z = bm->boxes[b].zone - 1;
		d = phys[wc_depth_id][b];
		/*** Of the catch ***/
		/* Inshore contribution */
		if (d < bm->cdz)
			a = 0;
		else
			a = 1;

		/* Vertebrates */
		catches = 0.0;
		catchesST = 0.0;
		totcatch = 0.0;
		totcatchA = 0.0;
		totcatchAST = 0.0;

		for (sp = 0; sp < bm->K_num_tot_sp; sp++) {
			if (FunctGroupArray[sp].isVertebrate == TRUE) {
				for (stage = 0; stage < K_num_invageclass; stage++) {
					if (stage == juv_id)
						sample = 1.0 - pop_fraction[1][sp][z];
					else
						sample = pop_fraction[1][sp][z];

					catches += stockinfo[stotcatch_id][sp][z][id] * sample * tl[a][sp][stage] / (zonearea[z] + TINY);
					catchesST += stockinfo[stotcatch_id][sp][z][id] * sample * tl[a + 3][sp][stage] / (zonearea[z] + TINY);
					totcatch += stockinfo[stotcatch_id][sp][z][id] * sample / (zonearea[z] + TINY);

					/* Overall case */
					totcatchA += stockinfo[stotcatch_id][sp][z][id] * sample * tl[2][sp][stage] / (zonearea[z] + TINY);
					totcatchAST += stockinfo[stotcatch_id][sp][z][id] * sample * tl[5][sp][stage] / (zonearea[z] + TINY);

				}
			}
		}

		avgtl[4][z] += catches * samplearea[bb] / (totcatch + TINY);
		avgtl[4][bm->nfzones] += catches * totalarea[bb] / (totcatch + TINY);
		avgtl[4][bm->nfzones + 1] += totcatchA * samplearea[bb] / (totcatch + TINY);
		avgtl[5][z] += catchesST * samplearea[bb] / (totcatch + TINY);
		avgtl[5][bm->nfzones] += catchesST * totalarea[bb] / (totcatch + TINY);
		avgtl[5][bm->nfzones + 1] += totcatchAST * samplearea[bb] / (totcatch + TINY);

		/* Invertebrates */
		for (sp = 0; sp < bm->K_num_tot_sp; sp++) {
			if (FunctGroupArray[sp].isVertebrate == FALSE && (FunctGroupArray[sp].isFished == TRUE || FunctGroupArray[sp].isImpacted == TRUE)) {

				/* Assume the the fished invertebrates don't include detritus */
				catches += invstockinfo[fcatch_id][sp][z][id] * tl[a][sp][0];
				catchesST += invstockinfo[fcatch_id][sp][z][id] * tl[a + 3][sp][0];
				totcatch += invstockinfo[fcatch_id][sp][z][id];

				/* Overall case */
				totcatchA += invstockinfo[fcatch_id][sp][z][id] * tl[2][sp][0];
				totcatchAST += invstockinfo[fcatch_id][sp][z][id] * tl[5][sp][0];
			}
		}

		avgtl[0][z] += catches * samplearea[bb] / (totcatch + TINY);
		avgtl[0][bm->nfzones] += catches * totalarea[bb] / (totcatch + TINY);
		avgtl[0][bm->nfzones + 1] += totcatchA * samplearea[bb] / (totcatch + TINY);
		avgtl[2][z] += catchesST * samplearea[bb] / (totcatch + TINY);
		avgtl[2][bm->nfzones] += catchesST * totalarea[bb] / (totcatch + TINY);
		avgtl[2][bm->nfzones + 1] += totcatchAST * samplearea[bb] / (totcatch + TINY);

		/*** Overall ***/
		/* Fish */
		biom = 0.0;
		biomST = 0.0;
		totbiom = 0.0;
		biomA = 0.0;
		biomAST = 0.0;

		for (sp = 0; sp < bm->K_num_tot_sp; sp++) {
			if (FunctGroupArray[sp].isVertebrate == TRUE) {
				for (stage = 0; stage < K_num_invageclass; stage++) {
					if (stage == juv_id)
						sample = 1.0 - pop_fraction[0][sp][z];
					else
						sample = pop_fraction[0][sp][z];

					biom += stockinfo[sbiomass_id][sp][z][id] * sample * tl[a][sp][stage] / (zonearea[z] + TINY);
					biomST += stockinfo[sbiomass_id][sp][z][id] * sample * tl[a + 3][sp][stage] / (zonearea[z] + TINY);
					totbiom += stockinfo[sbiomass_id][sp][z][id] * sample / (zonearea[z] + TINY);

					/* Overall case */
					biomA += stockinfo[sbiomass_id][sp][z][id] * sample * tl[2][sp][stage] / (zonearea[z] + TINY);
					biomAST += stockinfo[sbiomass_id][sp][z][id] * sample * tl[5][sp][stage] / (zonearea[z] + TINY);
				}
			}
		}

		avgtl[6][z] += biom * samplearea[bb] / (totbiom + TINY);
		avgtl[6][bm->nfzones] += biom * totalarea[bb] / (totbiom + TINY);
		avgtl[6][bm->nfzones + 1] += biomA * samplearea[bb] / (totbiom + TINY);
		avgtl[7][z] += biomST * samplearea[bb] / (totbiom + TINY);
		avgtl[7][bm->nfzones] += biomST * totalarea[bb] / (totbiom + TINY);
		avgtl[7][bm->nfzones + 1] += biomAST * samplearea[bb] / (totbiom + TINY);

		for (sp = 0; sp < bm->K_num_tot_sp; sp++) {
			if (FunctGroupArray[sp].isVertebrate == FALSE && FunctGroupArray[sp].isAssessed == TRUE) {
				if (FunctGroupArray[sp].isDetritus == FALSE) {
					biom += samplebiom[sp][z][id] * tl[a][sp][0];
					biomST += samplebiom[sp][z][id] * tl[a + 3][sp][0];
					totbiom += samplebiom[sp][z][id];

					/* Overall case */
					biomA += samplebiom[sp][z][id] * tl[2][sp][0];
					biomAST += samplebiom[sp][z][id] * tl[5][sp][0];
				} else {

					int detIndex = FunctGroupArray[sp].detIndex;

					for (habitat = WC; habitat <= SED; habitat++) {

						biom += sampledetbiom[detIndex][habitat][z][id] * tl[a][sp][0];
						biomST += sampledetbiom[detIndex][habitat][z][id] * tl[a + 3][sp][0];
						totbiom += sampledetbiom[detIndex][habitat][z][id];

						/* Overall case */
						biomA += sampledetbiom[detIndex][habitat][z][id] * tl[2][sp][0];
						biomAST += sampledetbiom[detIndex][habitat][z][id] * tl[5][sp][0];
					}

				}
			}
		}

		avgtl[1][z] += biom * samplearea[bb] / (totbiom + TINY);
		avgtl[1][bm->nfzones] += biom * totalarea[bb] / (totbiom + TINY);
		avgtl[1][bm->nfzones + 1] += biomA * samplearea[bb] / (totbiom + TINY);
		avgtl[3][z] += biomST * samplearea[bb] / (totbiom + TINY);
		avgtl[3][bm->nfzones] += biomST * totalarea[bb] / (totbiom + TINY);
		avgtl[3][bm->nfzones + 1] += biomAST * samplearea[bb] / (totbiom + TINY);

	}

	return;
}
/**
 * \brief This calculates trophic spectra (bins catches based on trophic level rounded to halves e.g 2, 2.5, 3, 3.5 etc)
 */
void Calc_Trophic_Spectra(MSEBoxModel *bm, double ***tl, int id) {
	int z, bb, b, a, sp, tlindx;
	double sample, d;
	int stage;

	for (z = 0; z < bm->nfzones + 1; z++)
		for (bb = 0; bb < bm->K_num_trophbin; bb++)
			for (a = 0; a < 2; a++)
				trophspect[bb][a][z] = 0.0;

	for (bb = 0; bb < bm->nsbox; bb++) {
		b = bm->nsboxes[bb];
		z = bm->boxes[b].zone - 1;
		d = phys[wc_depth_id][b];
		/*** Of the catch ***/
		/* Inshore contribution */
		if (d < bm->cdz)
			a = 0;
		else
			a = 1;

		for (sp = 0; sp < bm->K_num_tot_sp; sp++) {
			if (FunctGroupArray[sp].isFished == TRUE || FunctGroupArray[sp].isImpacted == TRUE) {

				if (FunctGroupArray[sp].isVertebrate == TRUE) {
					for (stage = 0; stage < K_num_invageclass; stage++) {
						if (stage == juv_id)
							sample = 1.0 - pop_fraction[1][sp][z];
						else
							sample = pop_fraction[1][sp][z];

						/* Zone specific and total, dynamic trophic level through time */
						tlindx = (int) (floor((tl[a][sp][stage] - 1.0) / 0.5));
						if (tlindx < 0)
							tlindx = 0;
						if (tlindx >= bm->K_num_trophbin)
							tlindx = bm->K_num_trophbin - 1;
						trophspect[tlindx][0][z] += stockinfo[stotcatch_id][sp][z][id] * sample;
						trophspect[tlindx][0][bm->nfzones] += stockinfo[stotcatch_id][sp][z][id] * sample * samplearea[bb];

						/* Zone specific and total, static trophic level through time */
						tlindx = (int) (floor((tl[a][sp][stage] - 1.0) / 0.5));
                        if (tlindx < 0) {
                            tlindx = 0;
                        }
                        if (tlindx >= bm->K_num_trophbin) {
                            tlindx = bm->K_num_trophbin - 1;
                        }
						trophspect[tlindx][1][z] += stockinfo[stotcatch_id][sp][z][id] * sample;
						trophspect[tlindx][1][bm->nfzones] += stockinfo[stotcatch_id][sp][z][id] * sample * samplearea[bb];

						/* Overall area, dynamic trophic level through time */
						tlindx = (int) (floor((tl[2][sp][stage] - 1.0) / 0.5));
						if (tlindx < 0)
							tlindx = 0;
						if (tlindx >= bm->K_num_trophbin)
							tlindx = bm->K_num_trophbin - 1;
						trophspect[tlindx][0][bm->nfzones + 1] += stockinfo[stotcatch_id][sp][z][id] * sample * samplearea[bb];

						/* Overall area, static trophic level through time */
						tlindx = (int) (floor((tl[5][sp][stage] - 1.0) / 0.5));
						if (tlindx < 0)
							tlindx = 0;
						if (tlindx >= bm->K_num_trophbin)
							tlindx = bm->K_num_trophbin - 1;
						trophspect[tlindx][1][bm->nfzones + 1] += stockinfo[stotcatch_id][sp][z][id] * sample * samplearea[bb];
					}
				}

				else {
					sample = 1.0;

					stage = 0;
					/* Zone specific and total, dynamic trophic level through time */
					tlindx = (int) (floor((tl[a][sp][stage] - 1.0) / 0.5));
					if (tlindx < 0)
						tlindx = 0;
					if (tlindx >= bm->K_num_trophbin)
						tlindx = bm->K_num_trophbin - 1;
					trophspect[tlindx][0][z] += invstockinfo[fcatch_id][sp][z][id];
					trophspect[tlindx][0][bm->nfzones] += invstockinfo[fcatch_id][sp][z][id] * samplearea[bb];

					/* Zone specific and total, static trophic level through time */
					tlindx = (int) (floor((tl[a + 3][sp][stage] - 1.0) / 0.5));
					if (tlindx < 0)
						tlindx = 0;
					if (tlindx >= bm->K_num_trophbin)
						tlindx = bm->K_num_trophbin - 1;
					trophspect[tlindx][1][z] += invstockinfo[fcatch_id][sp][z][id];
					trophspect[tlindx][1][bm->nfzones] += invstockinfo[fcatch_id][sp][z][id] * samplearea[bb];

					/* Overall area, dynamic trophic level through time */
					tlindx = (int) (floor((tl[2][sp][stage] - 1.0) / 0.5));
					if (tlindx < 0)
						tlindx = 0;
					if (tlindx >= bm->K_num_trophbin)
						tlindx = bm->K_num_trophbin - 1;
					trophspect[tlindx][0][bm->nfzones + 1] += invstockinfo[fcatch_id][sp][z][id] * samplearea[bb];

					/* Overall area, static trophic level through time */
					tlindx = (int) (floor((tl[5][sp][stage] - 1.0) / 0.5));
					if (tlindx < 0)
						tlindx = 0;
					if (tlindx >= bm->K_num_trophbin)
						tlindx = bm->K_num_trophbin - 1;
					trophspect[tlindx][1][bm->nfzones + 1] += invstockinfo[fcatch_id][sp][z][id] * samplearea[bb];

				}
			}
		}
	}

	return;
}
/**
 * \brief  This calculates  Diversity indices
 * For functional diversity double count non-planktivorous fish and sharks as diet changes with age
 */
void Calc_Div(MSEBoxModel *bm, FILE *ofp) {
	int z, bb, b, chrt, rank, nagemat_sp;
	double sample;
	int sp, j;

	for (z = 0; z < bm->nfzones; z++) {
		for (j = 0; j < 2; j++)
			divindx[j][z] = 0;
		for (sp = 0; sp < bm->K_num_tot_sp; sp++) {
			divsp[sp][z] = 0;
			divfn[sp][z] = 0;
		}
	}
	for (bb = 0; bb < bm->nsbox; bb++) {
		b = bm->nsboxes[bb];
		z = bm->boxes[b].zone - 1;

		for (sp = 0; sp < bm->K_num_tot_sp; sp++) {
			if (FunctGroupArray[sp].isVertebrate == FALSE && FunctGroupArray[sp].isDetritus == FALSE) {
				if ((biolbiom[sp][b][WC] > minpool) && (divsp[sp][z] == 0)) {
					divsp[sp][z]++;
				}
			}
		}
		for (sp = 0; sp < bm->K_num_tot_sp; sp++) {

			if (FunctGroupArray[sp].isVertebrate == TRUE) {
				nagemat_sp = (int) (FunctGroupArray[sp].speciesParams[age_mat_id]);
				/* Note: Working with operating model-type age classes so use agemat */
				for (chrt = 0; chrt < nagemat_sp; chrt++) {
					sample = (biolVERTinfo[bstruct_id][sp][chrt][b] + biolVERTinfo[bres_id][sp][chrt][b]) * biolVERTinfo[bstocknums_id][sp][chrt][b];
					if ((sample > minpool) && (!divsp[sp][z])) {
						divsp[sp][z]++;
						break;
					}
				}
				/* Note: Working with operating model-type age classes so use agemat */
				for (chrt = nagemat_sp; chrt < FunctGroupArray[sp].numCohortsXnumGenes; chrt++) {
					sample = (biolVERTinfo[bstruct_id][sp][chrt][b] + biolVERTinfo[bres_id][sp][chrt][b]) * biolVERTinfo[bstocknums_id][sp][chrt][b];
					if ((sample > minpool) && (!divsp[sp][z])) {
						divsp[sp][z]++;
					}
					if ((sample > minpool) && (!divfn[sp][z])) {
						divfn[sp][z]++;
						break;
					}
				}
			}
		}
	}
	for (z = 0; z < bm->nfzones; z++) {
		rank = 0;
		for (j = 0; j < bm->K_num_tot_sp; j++) {
			if (divsp[j][z] > 0)
				rank++;
		}
		divindx[0][z] = rank;
		for (j = 0; j < bm->K_num_tot_sp; j++) {
			if (FunctGroupArray[j].isVertebrate == TRUE) {
				if (divfn[j][z] > 0)
					rank++;
			}
		}
		divindx[1][z] = rank;
	}

	for (j = 0; j < bm->K_num_tot_sp; j++) {
		for (z = 0; z < bm->nfzones; z++) {
			if (divsp[j][z] > 0) {
				divindx[0][bm->nfzones]++;
				break;
			}
		}
	}
	for (j = 0; j < bm->K_num_tot_sp; j++) {
		if (FunctGroupArray[j].isVertebrate == TRUE) {
			for (z = 0; z < bm->nfzones; z++) {
				if (divfn[j][z] > 0) {
					divindx[1][bm->nfzones]++;
					break;
				}
			}
		}
	}

	return;
}

static void Calculate_DietIO(MSEBoxModel *bm, double ***tl, double ****diet, int index) {
	int pred, predStage, prey, preyStage;
	double step1, step2;

	for (pred = 0; pred < bm->K_num_tot_sp; pred++) {
		if (FunctGroupArray[pred].isAssessed == TRUE) {
			for (predStage = 0; predStage < K_num_invageclass; predStage++) {

				//fprintf(bm->logFile, "step1 = 0\n");
				step1 = 0.0;
				for (prey = 0; prey < bm->K_num_tot_sp; prey++) {
					if (FunctGroupArray[prey].isAssessed == TRUE) {
						for (preyStage = 0; preyStage < K_num_invageclass; preyStage++) {
							step1 += (tl[index][prey][preyStage] * diet[pred][predStage][prey][preyStage]);
						}
					}
				}

				step2 = 0.0;
				for (prey = 0; prey < bm->K_num_tot_sp; prey++) {
					if (FunctGroupArray[prey].isAssessed == TRUE) {
						for (preyStage = 0; preyStage < K_num_invageclass; preyStage++) {
							step2 += ((tl[index][prey][preyStage] - step1) * (tl[index][prey][preyStage] - step1)) * diet[pred][predStage][prey][preyStage];

						}
					}
				}

				oi[index][pred][predStage] = step2;

			}
		}
	}
}
/**
 * \brief This calculates Ominivory Index for each group
 */
void Calc_OI(MSEBoxModel *bm, double ***tl, double ****indiet, double ****offdiet, double ****totdiet) {

	/* Calculate OI and avgOI */
	Calculate_DietIO(bm, tl, indiet, 0);
	Calculate_DietIO(bm, tl, offdiet, 1);
	Calculate_DietIO(bm, tl, totdiet, 2);

	return;
}
/**
 * \brief This calculates System Ominivory Index
 * Using SOI = sum_over_j(Qj * sum_over_i(Eij * (TLi - (TLj - 1))^2))
 * where Eij = proportion of diet of pred j made up of prey i
 * Qj = total consumption by pred j
 */
void Calc_SOI(MSEBoxModel *bm, int z, int id, FILE *ofp) {
	int pred, predStage, zz;
	double step3, step4, step5, sigma, d, oij, sysom, sysdenom, min;

	d = 0;
	if (z == bm->nfzones) {
		/* Determine overall minimum consumption */
		min = MAXDOUBLE;
		for (zz = 0; zz < bm->nfzones; zz++) {
			if (mineat[zz][id] < min)
				min = mineat[zz][id];
		}
		mineat[z][id] = min;
	}

	soi[z] = 0.0;
	if (z != bm->nfzones) {
		if (InOffshorei[z])
			d = bm->cdz - 1;
		else
			d = bm->cdz + 1;
	}
	/* In contribution */
	sysom = 0.0;
	sysdenom = 0.0;
	for (pred = 0; pred < bm->K_num_tot_sp; pred++) {
		if (FunctGroupArray[pred].isAssessed == TRUE) {
			for (predStage = 0; predStage < K_num_invageclass; predStage++) {
				sigma = eatnetwk[pred + 1][predStage][z];

				if (z != bm->nfzones) {
					if (d < bm->cdz) {
						oij = oi[0][pred][predStage];
					} else {
						oij = oi[1][pred][predStage];
					}
				} else
					oij = oi[2][pred][predStage];
				step3 = sigma / (mineat[z][id] + TINY);
				step4 = 0.0;
				step5 = 0.0;

				if (step3 > 0.0) {
					step4 = log(step3) * oij;
					step5 = log(step3);
					sysom += step4;
					sysdenom += step5;
				}
			}
		}
	}

	if (sysdenom > 0.0)
		soi[z] = sysom / sysdenom;
	else
		soi[z] = sysom;

	return;
}

void Init_Globals_And_Arrays(MSEBoxModel *bm) {
	int i, j, k, sp;

	bm->globalHabstep1 = 0;
	bm->globalHabstep2 = 0;
	bm->globalstep1 = 0;
	bm->globalstep2 = 0;
	bm->globalAscen = 0;
	bm->globalOverhead = 0;
	bm->globalCapacity = 0;
	bm->globalTruPut = 0;
	bm->globalsysom = 0;
	bm->globalsysdenom = 0;
	bm->globaldiscards = 0;
	bm->globalcatches = 0;
	bm->globalNcep = 0;
	bm->globalPBRcatcep = 0;
	disrate[bm->nfzones] = 0.0;
	cvt[bm->nfzones] = 0.0;
	nppb[bm->nfzones] = 0;
	soi[bm->nfzones] = 0.0;
	for (i = 0; i < 2; i++) {
		habindx[i][bm->nfzones] = 0.0;
		divindx[i][bm->nfzones] = 0;
		monbet[i][bm->nfzones] = 0;
	}
	for (i = 0; i < 3; i++) {
		totsamplebiom[i][bm->nfzones] = 0.0;
		pd[i][bm->nfzones] = 0.0;
		pdcat[i][bm->nfzones] = 0.0;
	}

	for (sp = 0; sp < bm->K_num_tot_sp; sp++) {
		if(FunctGroupArray[sp].groupType == BIRD || FunctGroupArray[sp].groupType == MAMMAL){
			fledge[sp][bm->nfzones] = 0.0;
			fledge[sp + bm->K_num_tot_sp][bm->nfzones] = 0.0;

			for (j = 0; j < 3; j++)
				globalfledge[j][sp] = 0;
		}
	}
	for (i = 0; i < 8; i++)
		avgtl[i][bm->nfzones] = 0.0;
	for (i = 0; i < bm->K_num_sedbin; i++)
		sedbin[i][bm->nfzones] = 0;
	for (i = 0; i < K_netwk_properties; i++)
		netwkindx[i][bm->nfzones] = 0;
	for (j = 0; j < 2; j++)
		for (i = 0; i < 6; i++)
			globalpd[j][i] = 0;
	for (j = 0; j < bm->K_num_pelbin; j++)
		for (i = 0; i < 2; i++)
			pelbin[j][i][bm->nfzones] = 0;

	for (j = 0; j < bm->K_num_tot_sp; j++) {
		for (i = 0; i < 2; i++) {
			PBRglobal[j][i] = 0;
			for (k = 0; k < 2; k++)
				globalnums[j][i][k] = 0;
		}

	}
	for (i = 0; i < bm->nfzones + 1; i++) {
		for (k = 0; k < 3; k++)
			max_lngth[i][k] = 0.0;
	}
	for (i = 0; i < bm->nfzones; i++) {
		for (k = 0; k < bm->K_num_tot_sp; k++) {
			if (FunctGroupArray[k].isFished == TRUE) {
				bm->rep_box_of_zone[k][i][juv_id] = 0;
				bm->rep_box_of_zone[k][i][adult_id] = 0;
			}
		}
	}
	for (i = 0; i < bm->K_num_trophbin; i++) {
		for (j = 0; j < 2; j++) {
			trophspect[i][j][bm->nfzones] = 0;
			trophspect[i][j][bm->nfzones + 1] = 0;
		}
	}

	return;
}
