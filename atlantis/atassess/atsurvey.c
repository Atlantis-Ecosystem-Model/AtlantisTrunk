/**
 \file
 \brief  Routines to add sampling error and mimic data collection
 \ingroup atassess

 File:           atsurvey.c

 Created:        8/8/2004

 Author:         Beth Fulton

 Purpose:        Routines to add sampling error and mimic data collection
 Modified from general bay version of sampling model

 Arguments:      bm    - Pointer to box model.

 Returns:        void

 Revisions:      8/8/2004 Created from southeast survey.c

 10/9/2004 Added bm->annual_sample so don't need to
 repeat code, just loop until hit annual_sample and then
 do assessment and account keeping routines and sections

 12/9/2004 Added bm->flagecosysassess so only assess
 fisheries independent data when appropriate (i.e.
 when calculating ecosystem indicators not when using
 standard single species assessments

 22/9/2004 Replaced numbers in switch statements with id
 guild numbers.

 7/10/2004 Updated to include additional bioloigcal groups
 (PWN, BFF, BML, FBP, FDE, FDF, FVB, SHR, SSK, REP, SP, WDG)

 19/1/2005 Added Microphytobenthos (MB)

 16/8/2005 Put in default case for all switch statements

 20/8/2005 Generalised survey code so don't assume vertebrate
 identity with regard to ebing a charismatic, or fish etc.

 5/9/2006 Reset parameters so all fisheries can access all fished groups.
 Output still restricted however.

 29/5/2008 Added sample_id entry to sampling arrays so that can allow
 for continuous sampling rather than annual only

 28/05/2008 Bec Gorton
 Fixed a bug in the code zone aggregate code. The invertebrate values
 were being assessed as sp_id = CEPGuild_id + k which was out of the array range.
 This has been changed to sp_id = SPfishedid + k which works.

 29/5/2008 Added sample_id entry to sampling arrays so that can allow
 for continuous sampling rather than annual only

 3/01/2009 Beth Fulton
 Moved the bm->annual_sample iteration to before sampling so instead of testing versus
 ZERO for whether initialisation is needed, now need to test on if(bm->annual_sample == 1).
 Also added MSEBoxModel as required variable in call to Sort_Length_Weight().

 22-04-2009 Bec Gorton
 Changed the code that calculates the current stock id to use the Util_Get_Current_Stock_Index( function.

 25-03-2010 Bec Gorton
 Deleted the InitArray function as its a replication of the Init_Attribute_Array function.


 26-04-2010 Bec Gorton
 Added code to store the coefficent of variance values for the physical
 properties that are sampled so the SD values can be calculated. Also added code to
 sample the physical properties a given number of times. This is used in the Atlantis-CLAM
 integeration code so we can get a sampling SD.
 *********************************************************************/
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <ctype.h>
#include <math.h>
#include <sjwlib.h>
#include "atsample.h"

static void CharismaticEstimates(MSEBoxModel *bm, FILE *ofp);
static void InvertebrateSampling(MSEBoxModel *bm, FILE *ofp);
static void FishEstimates(MSEBoxModel *bm, FILE *ofp);
static void FisheriesRecords(MSEBoxModel *bm, FILE *ofp);

/**
 * \brief This reads in the value of the tracer at each timestep as well as the
 * Total coefficient of variation, omitting detritus (i.e. live groups only)
 */
void Sample_Stuff(MSEBoxModel *bm, FILE *ofp) {
	/* Invertebrate and physical samples */
	InvertebrateSampling(bm, ofp);

	/* Mammal Sampling */
	CharismaticEstimates(bm, ofp);

	/* Fisheries information collection */
	FisheriesRecords(bm, ofp);

	/* Fisheries independ data collection and biomass estimation */
	FishEstimates(bm, ofp);

	return;
}

/**
 * \brief Sample the physical properties.
 *
 *
 */
static void SamplePhysicalProperty(MSEBoxModel *bm, int propertyIndex, int bb, int box, int zone, double k_avg, double k_var) {
	double raw, sample;

	raw = phys[propertyIndex][box];
	sample = Assess_Add_Error(bm, flagphys, raw, k_avg, k_var);
	samplephys[propertyIndex - 2][zone][sample_id] += sample * samplearea[bb];

	cvphys[0][propertyIndex][zone] += sample;// * samplearea[bb];
	cvphys[1][propertyIndex][zone] += sample * sample;//(sample * samplearea[bb]) * (sample * samplearea[bb]);
	cvphys[2][propertyIndex][zone]++;
}

/**
 *  \brief Fisheries independent sampling of invertebrates and physical characteristics
 */
void InvertebrateSampling(MSEBoxModel *bm, FILE *ofp) {
	int bb, b, i, j, z, chrt, flag, sampleIndex;
	double raw, sample, k_avg, k_var, min, correct_size;
	int sp;
	int detIndex;

	/* Intialise CV arrays */
	for (z = 0; z < bm->nfzones; z++) {
		for (j = 0; j < bm->K_num_tot_sp; j++) {
			if (FunctGroupArray[j].isDetritus == FALSE) {
				for (chrt = 0; chrt < 3; chrt++)
					cvsample[chrt][j][z] = 0.0;
			}
		}

		zasum[z] = 0.0;

		zoneboxes[z] = 0;
		/* Init the physcial cv array */
		for (j = 0; j < num_sampled_phy_id; j++)
			for (chrt = 0; chrt < 3; chrt++)
				cvphys[chrt][j][z] = 0.0;
	}

	/* Calculate proportional area sampled at each site */
	for (bb = 0; bb < bm->nsbox; bb++) {
		b = bm->nsboxes[bb];
		z = bm->boxes[b].zone - 1;
		zasum[z] += bm->boxes[b].area;
		zoneboxes[z]++;
	}
	bm->totarea = 0;
	bm->inshorearea = 0.0;
	bm->offshorearea = 0.0;
	for (bb = 0; bb < bm->nfzones; bb++) {
		bm->totarea += zasum[bb];
		if (InOffshorei[bb])
			bm->inshorearea += zasum[bb];
		else
			bm->offshorearea += zasum[bb];
	}
	/* Normalise relative areas */
	bm->inshorearea /= (bm->totarea + TINY);
	bm->offshorearea /= (bm->totarea + TINY);

	for (bb = 0; bb < bm->nsbox; bb++) {
		b = bm->nsboxes[bb];
		z = bm->boxes[b].zone - 1;
		samplearea[bb] = bm->boxes[b].area / (zasum[z] + TINY);
		totalarea[bb] = bm->boxes[b].area / (bm->totarea + TINY);
	}

	/* Scientific sampling */

	if (verbose)
		fprintf(stderr, "Sampling invert data\n");

	for (bb = 0; bb < bm->nsbox; bb++) {
		b = bm->nsboxes[bb];
		z = bm->boxes[b].zone - 1;

		if (bm->flagecosysassess) {
			/* Physical characteristics */

			for (sampleIndex = 0; sampleIndex < phys_samplingsize; sampleIndex++) {

				SamplePhysicalProperty(bm, salinity_id, bb, b, z, k_avgsalt, k_varsalt);
				SamplePhysicalProperty(bm, temperature_id, bb, b, z, k_avgsalt, k_varsalt); // Intentioanl reulst of salt properties here as treating as well known physical properties

				for (i = light_wc_id; i < don_wc_id; i++) {
					SamplePhysicalProperty(bm, i, bb, b, z, k_avgphys, k_varphys);
				}
				for (i = don_wc_id; i < vol_id; i++) {
					SamplePhysicalProperty(bm, i, bb, b, z, k_avgnuts, k_varnuts);
				}
			}
			/* Processes */
			for (i = 0; i < 3; i++) {
				raw = physprocess[i][b];
				sample = Assess_Add_Error(bm, flagphys, raw, k_avgprocs, k_varprocs);
				sampleprocess[i][z][sample_id] += sample * samplearea[bb];
			}
		}

		for (sp = 0; sp < bm->K_num_tot_sp; sp++) {
			if (FunctGroupArray[sp].isVertebrate == FALSE) {

				/* If the flagecosysassess is on then we do all groups otherwise we just do the fished invertebrates */
				if (bm->flagecosysassess || FunctGroupArray[sp].isFished == TRUE) {

					if (FunctGroupArray[sp].isDetritus == FALSE) {

						raw = biolbiom[sp][b][WC];
						sample = Assess_Add_Error(bm, spErrorStructure[FunctGroupArray[sp].groupType].spErrorStructureFlag, raw,
								spErrorStructure[FunctGroupArray[sp].groupType].k_avg_Biomass[WC],
								spErrorStructure[FunctGroupArray[sp].groupType].k_var_Biomass[WC]);
						samplebiom[sp][z][sample_id] += sample * samplearea[bb];

						cvsample[0][sp][z] += sample * samplearea[bb];
						cvsample[1][sp][z] += (sample * samplearea[bb]) * (sample * samplearea[bb]);
						cvsample[2][sp][z]++;
					} else {

						detIndex = FunctGroupArray[sp].detIndex;
						raw = biolbiom[sp][b][WC];
						sample = Assess_Add_Error(bm, spErrorStructure[FunctGroupArray[sp].groupType].spErrorStructureFlag, raw,
								spErrorStructure[FunctGroupArray[sp].groupType].k_avg_Biomass[WC],
								spErrorStructure[FunctGroupArray[sp].groupType].k_var_Biomass[WC]);
						sampledetbiom[detIndex][WC][z][sample_id] += sample * samplearea[bb];

						raw = biolbiom[sp][b][SED];
						sample = Assess_Add_Error(bm, spErrorStructure[FunctGroupArray[sp].groupType].spErrorStructureFlag, raw,
								spErrorStructure[FunctGroupArray[sp].groupType].k_avg_Biomass[SED],
								spErrorStructure[FunctGroupArray[sp].groupType].k_var_Biomass[SED]);
						sampledetbiom[detIndex][SED][z][sample_id] += sample * samplearea[bb];
					}

				}

			}
		}
		if (bm->flagecosysassess) {
			for (sp = 0; sp < bm->K_num_tot_sp; sp++) {
				if (FunctGroupArray[sp].isVertebrate == FALSE && FunctGroupArray[sp].isDetritus == FALSE) {

					raw = biolprod[sp][b];
					sample = Assess_Add_Error(bm, flagprod, raw, spErrorStructure[FunctGroupArray[sp].groupType].k_avg_pp,
							spErrorStructure[FunctGroupArray[sp].groupType].k_var_pp);
					sampleprod[sp][z][sample_id] += sample * samplearea[bb];
				}
			}

			/* Leave this as a seperate loop so the calls to random number generators are in the same order as the original code */
			for (sp = 0; sp < bm->K_num_tot_sp; sp++) {
				if (FunctGroupArray[sp].isVertebrate == FALSE && FunctGroupArray[sp].isGrazer == TRUE) {
					raw = bioleat[sp][b];
					sample = Assess_Add_Error(bm, flageat, raw, spErrorStructure[FunctGroupArray[sp].groupType].k_avg_eat,
							spErrorStructure[FunctGroupArray[sp].groupType].k_var_eat);
					sampleeat[sp][z][sample_id] += sample * samplearea[bb];
				}
			}

			/* Samples of vertebrates for CVt purposes - for the purposes of sampling biomass
			 treat all vertebrates as having the same CV distribution as for fin-fish
			 */
			flag = flagfishbiom;
			k_avg = k_avgfish;
			k_var = k_varfish;
			for (i = 0; i < bm->K_num_tot_sp; i++) {
				if (FunctGroupArray[i].isVertebrate == TRUE) {
					for (chrt = 0; chrt < FunctGroupArray[i].numCohortsXnumGenes; chrt++) {
						raw = (biolVERTinfo[bstruct_id][i][chrt][b] + biolVERTinfo[bres_id][i][chrt][b]) * biolVERTinfo[bstocknums_id][i][chrt][b]
								/ (bm->boxes[b].area + TINY);
						sample = Assess_Add_Error(bm, flag, raw, k_avg, k_var);
						cvsample[0][i][z] += sample;
						cvsample[1][i][z] += (sample * sample);
						cvsample[2][i][z]++;
					}
				}
			}
		}

		/* Correct average back to per year rather than multiple times per year */
		if (bm->annual_sample == numsteps) {
			correct_size = numsteps;

			/* Physical characteristics */
			for (i = 0; i < num_sampled_phy_id - 2; i++)
				samplephys[i][z][sample_id] /= correct_size;
			/* Processes */
			for (i = 0; i < 3; i++)
				sampleprocess[i][z][sample_id] /= correct_size;
			/* Biomass */
			for (i = 0; i < bm->K_num_tot_sp; i++) {
				if (FunctGroupArray[i].isVertebrate == FALSE) {
					if (FunctGroupArray[i].isDetritus == TRUE) {
						detIndex = FunctGroupArray[i].detIndex;
						sampledetbiom[detIndex][WC][z][sample_id] /= correct_size;
						sampledetbiom[detIndex][SED][z][sample_id] /= correct_size;
					} else {
						samplebiom[i][z][sample_id] /= correct_size;
					}
				}
			}
			/* Production */
			for (i = 0; i < bm->K_num_tot_sp; i++) {
				if (FunctGroupArray[i].isVertebrate == FALSE && FunctGroupArray[i].isDetritus == FALSE) {
					sampleprod[i][z][sample_id] /= correct_size;
				}
			}

			/* Consumption */
			for (i = 0; i < bm->K_num_tot_sp; i++) {
				if (FunctGroupArray[i].isVertebrate == FALSE && FunctGroupArray[i].isGrazer == TRUE) {
					sampleeat[i][z][sample_id] /= correct_size;
				}
			}
		}
		if (verbose > 2 && bm->flagecosysassess) {
			for (i = 0; i < num_sampled_phy_id - 2; i++)
				fprintf(ofp, "samplephys[%d][%d]: %e\n", i, z, samplephys[i][z][sample_id]);
			/* Processes */
			for (i = 0; i < 3; i++)
				fprintf(ofp, "sampleprocess[%d][%d]: %e\n", i, z, sampleprocess[i][z][sample_id]);
			/* Biomass */
			for (i = 0; i < bm->K_num_tot_sp; i++) {
				if (FunctGroupArray[i].isVertebrate == FALSE) {
					if (FunctGroupArray[i].isDetritus == TRUE) {
						detIndex = FunctGroupArray[i].detIndex;
						fprintf(ofp, "sampledetbiom[%d][WC][%d]: %e\n", i, z, sampledetbiom[detIndex][WC][z][sample_id]);
						fprintf(ofp, "sampledetbiom[%d][SED][%d]: %e\n", i, z, sampledetbiom[detIndex][SED][z][sample_id]);
					} else {
						fprintf(ofp, "samplebiom[%d][%d]: %e\n", i, z, samplebiom[i][z][sample_id]);
					}
					//samplebiom[i][z][sample_id] /= correct_size;
				}
			}
			//fprintf(ofp,"samplebiom[%d][%d]: %e\n", i, z, samplebiom[i][z][sample_id]);
			/* Production */
			for (i = 0; i < bm->K_num_tot_sp; i++) {
				if (FunctGroupArray[i].isVertebrate == FALSE && FunctGroupArray[i].isDetritus == FALSE) {
					fprintf(ofp, "sampleprod[%d][%d]: %e\n", i, z, sampleprod[i][z][sample_id]);
				}
			}
			/* Consumption */
			for (i = 0; i < bm->K_num_tot_sp; i++) {
				if (FunctGroupArray[i].isVertebrate == FALSE && FunctGroupArray[i].isGrazer == TRUE) {
					fprintf(ofp, "sampleeat[%d][%d]: %e\n", i, z, sampleeat[i][z][sample_id]);
				}
			}
		}
	}

	/* Get minimum consumption term for inverts */
	if (bm->flagecosysassess) {
		for (z = 0; z < bm->nfzones; z++) {
			min = MAXDOUBLE;
			for (i = 0; i < bm->K_num_tot_sp; i++) {
				if (FunctGroupArray[i].isVertebrate == FALSE && FunctGroupArray[i].isGrazer == TRUE && FunctGroupArray[i].groupType != DINOFLAG) {
					if ((sampleeat[i][z][sample_id] > 0.0) && (sampleeat[i][z][sample_id] < min))
						min = sampleeat[i][z][sample_id];
				}
			}
			mineat[z][sample_id] = min;

			if (verbose > 2)
				fprintf(ofp, "mineat[%d]: %e\n", z, mineat[z][sample_id]);
		}
	}

	return;
}

/**
 * \brief Fisheries independent sampling and also estimates of biomass etc based on catch for the
 * mammal groups (mammals and birds)
 * */
void CharismaticEstimates(MSEBoxModel *bm, FILE *ofp) {

	int chrt, b, z, i, j, k, bb, nsample, samplen1, samplen2, sp, ltc, cage, fishery_id;
	double wgt, lngth, raw = 0, sample, sampleN, sampleW, rawn, sum, vol;
	int lbin;
	/* double correct_size = 0; Not used here as all vertebrates corrected at the end of FishEstimates() */

	int cmaxage = bm->K_max_agekey - 1;
	int cmaxsize = bm->K_num_size - 1;

	if (verbose) {
		fprintf(stderr, "Sampling mammal data\n");
		fprintf(ofp, "Sampling mammal data\n");
	}

	/* Initialise the local arrays */
	for (z = 0; z < bm->nfzones; z++) {
		for (k = 0; k < 3; k++) {
			for (chrt = 0; chrt < bm->K_num_sampleage; chrt++) {
				samplenums[chrt][k] = 0.0;
			}
		}
	}

	for (z = 0; z < bm->nfzones; z++) {
		for (k = 0; k < 4; k++) {
			for (i = 0; i < bm->K_num_tot_sp; i++) {
				if (FunctGroupArray[i].isVertebrate == TRUE) {
					totnums[k][i][z] = 0.0;
				}
			}
		}
	}

	/* Initialise the bins */
	if (bm->annual_sample == 1) {
		for (i = 0; i < bm->K_num_size; i++)
			for (b = 0; b < bm->nfzones; b++)
				for (k = 0; k < bm->K_num_tot_sp; k++) {
					if ((FunctGroupArray[k].groupType == BIRD) || (FunctGroupArray[k].groupType == MAMMAL)) {
						for (j = 0; j < 5; j++)
							sizebins[j][k][b][i][0][sample_id] = 0.0;
					}
				}

		for (k = 0; k < bm->K_num_tot_sp; k++) {
			if ((FunctGroupArray[k].groupType == BIRD) || (FunctGroupArray[k].groupType == MAMMAL)) {
				for (b = 0; b < bm->nfzones; b++)
					for (j = 0; j < bm->K_max_agekey; j++)
						for (i = 0; i < bm->K_num_size; i++)
							agebins[k][b][j][i][sample_id] = 0.0;
			}
		}

		/* Bird information */
		if (verbose)
			fprintf(stderr, "Sampling birds\n");

		for (b = 0; b < bm->nbox; b++) {
			if (bm->boxes[b].type != BOUNDARY) {
				z = bm->boxes[b].zone - 1;
				vol = phys[vol_id][b];

				for (sp = 0; sp < bm->K_num_tot_sp; sp++) {
					if (FunctGroupArray[sp].isVertebrate == TRUE) {
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

								raw = biolVERTinfo[bstruct_id][sp][chrt][b] + biolVERTinfo[bres_id][sp][chrt][b];
								sample = Assess_Add_Error(bm, flagverts, raw, k_avgweight, k_varweight);
								sampleW = sample;
								individVERTinfo[iweight_id][sp][lbin][z][sample_id] += sample;

								/* Numbers in stock */
								raw = biolVERTinfo[bstocknums_id][sp][chrt][b];
								sample = Assess_Add_Error(bm, flagcount, raw, k_avgcount, k_varcount);
								sampleN = sample;
								individVERTinfo[istocknums_id][sp][lbin][z][sample_id] += sample;

								/* Numbers in catch*/
								raw = biolVERTinfo[bcatchnums_id][sp][chrt][b];
								sample = Assess_Add_Error(bm, flagcount, raw, k_avgcobs, k_varcobs);
								individVERTinfo[icatchnums_id][sp][lbin][z][sample_id] += sample;

								/* Biomass */
								individVERTinfo[ibiomass_id][sp][lbin][z][sample_id] += sampleW * sampleN / (bm->boxes[b].area + TINY);

								/* Discards */
								raw = biolVERTinfo[bdiscards_id][sp][chrt][b];
								sample = Assess_Add_Error(bm, flagcount, raw, k_avgcobs, k_varcobs);
								individVERTinfo[idiscards_id][sp][lbin][z][sample_id] += sample;

								/* Condition */
								raw = biolVERTinfo[bres_id][sp][chrt][b];
								sample = Assess_Add_Error(bm, flagverts, raw, k_avgweight, k_varweight);
								individVERTinfo[icondn_id][sp][lbin][z][sample_id] += (sample / (sampleW + TINY)) * (bm->boxes[b].area / (bm->totarea + TINY));
							}
						}
					}
				}
			}
		}

		for (z = 0; z < bm->nfzones; z++) {
			for (sp = 0; sp < bm->K_num_tot_sp; sp++) {
				if (FunctGroupArray[sp].isVertebrate == TRUE) {
					if(FunctGroupArray[sp].groupType == BIRD){
						for (chrt = 0; chrt < FunctGroupArray[sp].numCohortsXnumGenes; chrt++) {
							/* Bird stock biomass */
							stockinfo[sbiomass_id][sp][z][sample_id] += individVERTinfo[ibiomass_id][sp][chrt][z][sample_id];

							/* Bird stock numbers */
							stockinfo[sstocknums_id][sp][z][sample_id] += individVERTinfo[istocknums_id][sp][chrt][z][sample_id];

							/* Bird catch numbers */
							stockinfo[scatchnums_id][sp][z][sample_id] += individVERTinfo[icatchnums_id][sp][chrt][z][sample_id];
						}
						for (chrt = 0; chrt < FunctGroupArray[sp].numCohortsXnumGenes; chrt++) {
							/* Normalise individual bird numbers */
							individVERTinfo[istocknums_id][sp][chrt][z][sample_id] /= (stockinfo[sstocknums_id][sp][z][sample_id] + TINY);
							individVERTinfo[icatchnums_id][sp][chrt][z][sample_id] /= (stockinfo[scatchnums_id][sp][z][sample_id] + TINY);
						}
					}
				}
			}
		}

		for (bb = 0; bb < bm->nsbox; bb++) {
			b = bm->nsboxes[bb];
			z = bm->boxes[b].zone - 1;
			vol = phys[vol_id][b];

			for (sp = 0; sp < bm->K_num_tot_sp; sp++) {
				if (FunctGroupArray[sp].isVertebrate == TRUE) {
					if(FunctGroupArray[sp].groupType == BIRD){
						for (chrt = 0; chrt < FunctGroupArray[sp].numCohortsXnumGenes; chrt++) {
							/* Bird production */
							raw = biolVERTinfo[bprod_id][sp][chrt][b];
							sample = Assess_Add_Error(bm, flagprod, raw, k_avgprodn, k_varprodn);
							stockinfo[sprod_id][sp][z][sample_id] += sample * samplearea[bb] * ceil(biolVERTinfo[bstocknums_id][sp][chrt][b]) / (vol + TINY);

							/* Bird consumption */
							raw = biolVERTinfo[beat_id][sp][chrt][b];
							sample = Assess_Add_Error(bm, flageat, raw, k_avgeat, k_vareat);
							stockinfo[seat_id][sp][z][sample_id] += sample * samplearea[bb];
						}
					}
				}
			}
		}
	}

	/* Mammal information */
	if (verbose)
		fprintf(stderr, "Sampling mammals\n");

	for (b = 0; b < bm->nbox; b++) {
		if (bm->boxes[b].type != BOUNDARY) {
			z = bm->boxes[b].zone - 1;

			/* Total mammal stock numbers */
			for (j = 0; j < bm->K_num_tot_sp; j++) {
				if (FunctGroupArray[j].isVertebrate == TRUE) {
					if (FunctGroupArray[j].groupType == MAMMAL) {
						for (chrt = 0; chrt < FunctGroupArray[j].numCohorts * FunctGroupArray[j].numGeneTypes; chrt++) {
							raw = biolVERTinfo[bstocknums_id][j][chrt][b];
							sample = Assess_Add_Error(bm, flagcount, raw, k_avgcount, k_varcount);
							stockinfo[sstocknums_id][j][z][sample_id] += sample;
						}
					}
				}
			}
		}
	}

	for (bb = 0; bb < bm->nsbox; bb++) {
		b = bm->nsboxes[bb];
		z = bm->boxes[b].zone - 1;
		vol = phys[vol_id][b];

		for (sp = 0; sp < bm->K_num_tot_sp; sp++) {
			if (FunctGroupArray[sp].isVertebrate == TRUE) {
				if(FunctGroupArray[sp].groupType == MAMMAL){
					/* Determine subsample numbers
					 [0][time_yr] = sample from the stock
					 [1][time_yr] = sample from the catch
					 [2][time_yr] = sample from the discards

					 time_yr = 0 = end of year
					 time_yr = 1 = mid year
					 */
					for (chrt = 0; chrt < 3; chrt++)
						totn[chrt] = 0;

					for (chrt = 0; chrt < FunctGroupArray[sp].numCohortsXnumGenes; chrt++) {
						rawn = biolVERTinfo[bstocknums_id][sp][chrt][b];
						sample = Assess_Add_Error(bm, flagcount, rawn, k_avgcount, k_varcount);
						samplenums[chrt][samplestock_id] = ceil(sample);
						totn[samplestock_id] += samplenums[chrt][samplestock_id];
						rawn = biolVERTinfo[bcatchnums_id][sp][chrt][b];
						sample = Assess_Add_Error(bm, flagcount, rawn, k_avgcobs, k_varcobs);
						samplenums[chrt][samplecatch_id] = ceil(sample);
						totn[samplecatch_id] += samplenums[chrt][samplecatch_id];
                        raw = biolVERTinfo[bstruct_id][sp][chrt][b] + biolVERTinfo[bres_id][sp][chrt][b];
						rawn = biolVERTinfo[bdiscards_id][sp][chrt][b] / (raw + TINY);
						sample = Assess_Add_Error(bm, flagcount, rawn, k_avgcobs, k_varcobs);
						samplenums[chrt][samplediscard_id] = ceil(sample);
						totn[samplediscard_id] += samplenums[chrt][samplediscard_id];
					}
					for (chrt = 0; chrt < FunctGroupArray[sp].numCohortsXnumGenes; chrt++) {
						for (i = samplestock_id; i < samplediscard_id + 1; i++) {
							samplenums[chrt][i] /= (totn[i] + TINY);
						}
					}

					/* Sampling mammal size structure per zone */
					fishery_id = (int) (FunctGroupArray[sp].speciesParams[primary_fishery_id]);
					for (chrt = 0; chrt < FunctGroupArray[sp].numCohortsXnumGenes; chrt++) {
						
                        /*
                        if (verbose > 1)
							fprintf(stderr, "Box: %d mammal sampling %s cohort: %d\n", b, FunctGroupArray[sp].groupCode, chrt);
                         */
						/* Individual size */
						raw = biolVERTinfo[bstruct_id][sp][chrt][b] + biolVERTinfo[bres_id][sp][chrt][b];

						/* Stock*/
						sample = samplenums[chrt][samplestock_id] * totn[samplestock_id];
						samplen1 = (int) (ceil(sample));
						samplen2 = (int) floor(ROUNDGUARD + (FunctGroupArray[sp].speciesParams[samplesize_id] * samplenums[chrt][samplestock_id]));

						if (!samplen2 && (samplen1 > 0))
							samplen2 = 1;

						if (samplen1 < samplen2)
							nsample = samplen1;
						else
							nsample = samplen2;
						i = 0;
						while (nsample && i <= nsample) {
							wgt = Assess_Add_Error(bm, flagverts, raw, k_avgweight, k_varweight);
							lngth = Get_Length(bm, wgt, sp);
							Sort_Length_Weight(bm, 0, 1, sizestocknums_id, z, sp, fishery_id, lngth, 1, wgt, cmaxsize, sample_id, &ltc, ofp);
							//TODO Check with beth about changing to a round instead of a floor.
							cage = (int)(round((chrt + drandom(0.0, 1.0)) * FunctGroupArray[sp].ageClassSize + 0.5));

							Length_Age_Key(bm, z, sp, cmaxsize, lngth, cage, cmaxage, 1, sample_id);

							/* Condition */
							if (bm->sample_now) {
								raw = biolVERTinfo[bres_id][sp][chrt][b];
								sample = Assess_Add_Error(bm, flagverts, raw, k_avgweight, k_varweight);
								individVERTinfo[icondn_id][sp][ltc][z][sample_id] += (sample / (wgt + TINY)) * samplearea[bb] / (nsample + TINY);
							}

							i++;
						}

						/* Catch */
						sample = samplenums[chrt][samplecatch_id] * totn[samplecatch_id];
						samplen1 = (int) (ceil(sample));
						samplen2 = (int) floor(ROUNDGUARD + (FunctGroupArray[sp].speciesParams[samplesize_id] * samplenums[chrt][samplecatch_id]));

						if (!samplen2 && (samplen1 > 0))
							samplen2 = 1;

						if (samplen1 < samplen2)
							nsample = samplen1;
						else
							nsample = samplen2;
						i = 0;
						while (nsample && i <= nsample) {
							wgt = Assess_Add_Error(bm, flagverts, raw, k_avgweight, k_varweight);
							lngth = Get_Length(bm, wgt, sp);
							Sort_Length_Weight(bm, 0, 1, sizecatchnums_id, z, sp, fishery_id, lngth, 1, wgt, cmaxsize, sample_id, &ltc, ofp);
							i++;
						}

						/* Discards */
						sample = samplenums[chrt][samplediscard_id] * totn[samplediscard_id];
						samplen1 = (int) (ceil(sample));
						samplen2 = (int) floor(ROUNDGUARD + (FunctGroupArray[sp].speciesParams[samplesize_id] * samplenums[chrt][samplediscard_id]));

						if (!samplen2 && (samplen1 > 0))
							samplen2 = 1;

						if (samplen1 < samplen2)
							nsample = samplen1;
						else
							nsample = samplen2;
						i = 0;
						while (nsample && i <= nsample) {
							wgt = Assess_Add_Error(bm, flagverts, raw, k_avgweight, k_varweight);
							lngth = Get_Length(bm, wgt, sp);
							Sort_Length_Weight(bm, 0, 1, sizediscardnums_id, z, sp, fishery_id, lngth, 1, wgt, cmaxsize, sample_id, &ltc, ofp);
							i++;

						}

						if (bm->sample_now) {
							/* Production by mammals */
							raw = biolVERTinfo[bprod_id][sp][chrt][b];
							sample = Assess_Add_Error(bm, flagprod, raw, k_avgprodn, k_varprodn);
							stockinfo[sprod_id][sp][z][sample_id] += sample * samplearea[bb] * ceil(biolVERTinfo[bstocknums_id][sp][chrt][b]) / (vol + TINY);

							/* Consumption by mammals */
							raw = biolVERTinfo[beat_id][sp][chrt][b];
							sample = Assess_Add_Error(bm, flageat, raw, k_avgeat, k_vareat);
							stockinfo[seat_id][sp][z][sample_id] += sample * samplearea[bb];
						}
					}
				}
			}
		}
	}

	if (verbose)
		fprintf(stderr, "Update mammal stock and individ arrays\n");

	/* Correct biomass and producrion estimates for number of samples taken in the year */
	if (bm->annual_sample == numsteps) {
		/* Scaling size structure per zone so it sums to one - only do this annually (i.e. once sampling complete) */
		for (z = 0; z < bm->nfzones; z++) {
			for (sp = 0; sp < bm->K_num_tot_sp; sp++) {
				if (FunctGroupArray[sp].isVertebrate == TRUE) {
					if(FunctGroupArray[sp].groupType == MAMMAL){
						for (i = 0; i < bm->K_num_size; i++)
							for (j = 1; j < 4; j++)
								totnums[j][sp][z] += sizebins[j][sp][z][i][0][sample_id];
					}
				}
			}
			for (sp = 0; sp < bm->K_num_tot_sp; sp++) {
				if (FunctGroupArray[sp].isVertebrate == TRUE) {
					if(FunctGroupArray[sp].groupType == MAMMAL){
						for (i = 0; i < bm->K_num_size; i++) {
							/* Size distribution */
							individVERTinfo[istocknums_id][sp][i][z][sample_id] = sizebins[sizestocknums_id][sp][z][i][0][sample_id]
									/ (totnums[totstocknums_id][sp][z] + TINY);

							/* Average weight of individual in that size bin */
							individVERTinfo[iweight_id][sp][i][z][sample_id] = sizebins[sizeweight_id][sp][z][i][0][sample_id]
									/ (totnums[totstocknums_id][sp][z] + totnums[totcatchnums_id][sp][z] + totnums[totdiscardnums_id][sp][z] + TINY);

							/* Biomass */
							individVERTinfo[ibiomass_id][sp][i][z][sample_id] = stockinfo[sstocknums_id][sp][z][sample_id]
									* individVERTinfo[istocknums_id][sp][i][z][sample_id] * individVERTinfo[iweight_id][sp][i][z][sample_id];

							/* Stock biomass */
							stockinfo[sbiomass_id][sp][z][sample_id] += individVERTinfo[ibiomass_id][sp][i][z][sample_id];

							/* Size distribution of catch */
							individVERTinfo[icatchnums_id][sp][i][z][sample_id] = sizebins[sizecatchnums_id][sp][z][i][0][sample_id]
									/ (totnums[totcatchnums_id][sp][z] + TINY);

							/* Size distribution of discards */
							individVERTinfo[idiscards_id][sp][i][z][sample_id] = sizebins[sizediscardnums_id][sp][z][i][0][sample_id]
									/ (totnums[totdiscardnums_id][sp][z] + TINY);

							/* Age-length keys */
							sum = 0.0;
							for (j = 0; j < bm->K_max_agekey; j++) {
								agelengthkey[i][j][sp][z][sample_id] = agebins[sp][z][j][i][sample_id];
								/* Normalise so each length bin adds up to one across the ages */
								sum += agelengthkey[i][j][sp][z][sample_id];
							}
							for (j = 0; j < bm->K_max_agekey; j++)
								agelengthkey[i][j][sp][z][sample_id] /= (sum + TINY);
						}

					}
				}
			}
		}
	}
	return;
}
/**
 *  \brief Sums over the sample timestep so you have total fisheries catch (etc) for the period
 *  as the collected records
 **/
void FisheriesRecords(MSEBoxModel *bm, FILE *ofp) {
	int b, i, k, z, samplen1, samplen2, chrt, j, bb, nsample, sp, fishery_id, flagspeffortmodel, nf, ltc, ij;
	double raw, sample, rawwgt, wgt, lngth, correct_size;
	int cmaxsize = bm->K_num_size - 1;

	int Yr = (int) floor(ROUNDGUARD + ((bm->dayt - bm->tassessstart) / 365.0));

	// printf("Yr = %d\n", Yr);
	if (verbose) {
		fprintf(stderr, "Sampling fisheries data\n");
		fprintf(ofp, "Sampling fisheries data\n");
	}

	/* Reset the totals to zero */
	if (bm->annual_sample == 1) {
		for (b = 0; b < bm->nbox; b++) {
			for (sp = 0; sp < bm->K_num_tot_sp; sp++) {
				if (FunctGroupArray[sp].isImpacted == TRUE) {
					for (nf = 0; nf < bm->K_num_fisheries; nf++) {
						for (k = 0; k < 3; k++)
							fishery[k][nf][sp][b][sample_id] = 0.0;
					}
				}
			}
		}
	}

	/* Assuming the following data structure
	 fishery[0][group][b] = catch of XX
	 fishery[1][group][b] = effort on XX
	 fishery[2][group][b] = discards of XX

	 Get totals since last sample period (e.g. sum over the year).
	 Note: Do NOT * bm->boxes[b].area here as already done in original operating model.

	 Collect annual fisheries data
	 */
	if (bm->annual_sample == numsteps) {
		for (b = 0; b < bm->nbox; b++) {
			if (bm->boxes[b].type != BOUNDARY) {
				/* Collect data for target groups */
				for (sp = 0; sp < bm->K_num_tot_sp; sp++) {
					if (FunctGroupArray[sp].isFished == TRUE) {
						for (nf = 0; nf < bm->K_num_fisheries; nf++) {
							/* Assign values */
							flagspeffortmodel = (int) (bm->FISHERYprms[nf][flageffortmodel_id]);
							if ((flagspeffortmodel != rec_effort) && (flagspeffortmodel != rec_econ_model)) {
								/* Only collect states for commercial fisheries */

								for (ij = 0; ij < bm->boxes[b].nz; ij++)
									fishery[fcatch_id][nf][sp][b][sample_id] += bm->CumCatch[sp][nf][b][ij];
								fishery[feffort_id][nf][sp][b][sample_id] += bm->CumEffort[nf][b];
								fishery[fdiscards_id][nf][sp][b][sample_id] += bm->CumDiscards[sp][nf][b];
							}
						}
					}
				}

				for (sp = 0; sp < bm->K_num_tot_sp; sp++) {
					if (FunctGroupArray[sp].isFished == FALSE && FunctGroupArray[sp].isImpacted == TRUE) {

						//TODO: check with Beth if this should only collect states for commercial fisheries.
						for (nf = 0; nf < bm->K_num_fisheries; nf++) {
							/* Assign values */
							fishery[feffort_id][nf][sp][b][sample_id] += bm->CumEffort[nf][b];
							fishery[fdiscards_id][nf][sp][b][sample_id] += bm->CumDiscards[sp][nf][b];
						}
					}
				}
			}
		}

		/* Add error to the totals */
		for (b = 0; b < bm->nbox; b++) {

			if (bm->boxes[b].type != BOUNDARY) {
				z = bm->boxes[b].zone - 1;
				for (sp = 0; sp < bm->K_num_tot_sp; sp++) {
					if (FunctGroupArray[sp].isImpacted == TRUE) {
						for (nf = 0; nf < bm->K_num_fisheries; nf++) {
							raw = fishery[fcatch_id][nf][sp][b][sample_id];
							sample = Assess_Add_Error(bm, flagcatch, raw, k_avgcatch, k_varcatch);
							fishery[fcatch_id][nf][sp][b][sample_id] = sample;

							raw = fishery[feffort_id][nf][sp][b][sample_id];
							sample = Assess_Add_Error(bm, flageffort, raw, k_avgeffort, k_vareffort);
							fishery[feffort_id][nf][sp][b][sample_id] = sample;

							raw = fishery[fdiscards_id][nf][sp][b][sample_id];
							sample = Assess_Add_Error(bm, flagdiscrd, raw, k_avgtdiscrd, k_vartdiscrd);
							fishery[fdiscards_id][nf][sp][b][sample_id] = sample;

						}
					}
				}

				/* Aggregate by zone */
				/* Invertebrates */
				for (sp = 0; sp < bm->K_num_tot_sp; sp++) {
					if (FunctGroupArray[sp].isVertebrate == FALSE && (FunctGroupArray[sp].isFished == TRUE || FunctGroupArray[sp].isImpacted == TRUE)) {

						for (nf = 0; nf < bm->K_num_fisheries; nf++) {
							invstockinfo[fcatch_id][sp][z][sample_id] += fishery[fcatch_id][nf][sp][b][sample_id];
							invstockinfo[feffort_id][sp][z][sample_id] += fishery[feffort_id][nf][sp][b][sample_id];
							invstockinfo[fdiscards_id][sp][z][sample_id] += fishery[fdiscards_id][nf][sp][b][sample_id];

							/* Collect assessment data */
							bm->EffortRecord[Yr][sp][commerical_id] += fishery[feffort_id][nf][sp][b][sample_id];
						}
					}
				}
				/* Vertebrates */
				for (sp = 0; sp < bm->K_num_tot_sp; sp++) {
					if (FunctGroupArray[sp].isVertebrate == TRUE) {
						for (nf = 0; nf < bm->K_num_fisheries; nf++) {
							stockinfo[stotcatch_id][sp][z][sample_id] += fishery[fcatch_id][nf][sp][b][sample_id];
							stockinfo[stoteffort_id][sp][z][sample_id] += fishery[feffort_id][nf][sp][b][sample_id];
							stockinfo[sdiscards_id][sp][z][sample_id] += fishery[fdiscards_id][nf][sp][b][sample_id];

							/* Collect assessment data */
							bm->EffortRecord[Yr][sp][commerical_id] += fishery[feffort_id][nf][sp][b][sample_id];
						}
					}
				}
			}
		}
	}

	/* Size based data */
	if (verbose)
		fprintf(stderr, "Sampling fisheries size data\n");

	/* Initialise the bins */
	if (bm->annual_sample == 1) {
		for (i = 0; i < bm->K_num_size; i++) {
			for (b = 0; b < bm->nfzones; b++) {
				for (sp = 0; sp < bm->K_num_tot_sp; sp++) {
					if (FunctGroupArray[sp].isVertebrate == TRUE) {
						for (j = 0; j < 5; j++) {
							sizebins[j][sp][b][i][1][sample_id] = 0.0;
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
			if (FunctGroupArray[sp].isVertebrate == TRUE) {
				//if ((FunctGroupArray[sp].groupType != BIRD) && (FunctGroupArray[sp].groupType != MAMMAL)) {
				if ((FunctGroupArray[sp].groupType != BIRD && FunctGroupArray[sp].groupType != MAMMAL)) {
					/* Determine subsample numbers */
					for (chrt = 0; chrt < 3; chrt++)
						totn[chrt] = 0.0;

					for (chrt = 0; chrt < FunctGroupArray[sp].numCohortsXnumGenes; chrt++) {
						/* Numbers in catch */
						raw = biolVERTinfo[bcatchnums_id][sp][chrt][b];
						sample = Assess_Add_Error(bm, flagcount, raw, k_avgcobs, k_varcobs);
						samplenums[chrt][samplecatch_id] = ceil(sample);
						totn[samplecatch_id] += samplenums[chrt][samplecatch_id];

						/* Collect assessment data */
						bm->CatchRecord[Yr][sp][chrt][commerical_id] += sample;

						/* Numbers in discards */
						rawwgt = biolVERTinfo[bstruct_id][sp][chrt][b] + biolVERTinfo[bres_id][sp][chrt][b];
						raw = biolVERTinfo[bdiscards_id][sp][chrt][b] / (rawwgt + TINY);
						sample = Assess_Add_Error(bm, flagcount, raw, k_avgcobs, k_varcobs);
						samplenums[chrt][samplediscard_id] = ceil(sample);
						totn[samplediscard_id] += samplenums[chrt][samplediscard_id];

						/* Collect assessment data - add discards to landings to get totoal catch */
						bm->CatchRecord[Yr][sp][chrt][commerical_id] += sample;
					}
					for (chrt = 0; chrt < FunctGroupArray[sp].numCohortsXnumGenes; chrt++) {
						for (k = samplecatch_id; k < samplediscard_id + 1; k++) {
							samplenums[chrt][k] /= (totn[k] + TINY);
						}
					}
					fishery_id = (int) (FunctGroupArray[sp].speciesParams[primary_fishery_id]);
					for (chrt = 0; chrt < FunctGroupArray[sp].numCohortsXnumGenes; chrt++) {
                        
                        /*
						if (verbose > 1)
							fprintf(stderr, "Box: %d Fisheries sampling %s cohort: %d\n", b, FunctGroupArray[sp].groupCode, chrt);
                         */
                        
						rawwgt = biolVERTinfo[bstruct_id][sp][chrt][b] + biolVERTinfo[bres_id][sp][chrt][b];
						sample = samplenums[chrt][samplecatch_id] * totn[samplecatch_id];
						samplen1 = (int) (ceil(sample));
						samplen2 = (int) floor(ROUNDGUARD + (FunctGroupArray[sp].speciesParams[samplesize_id] * samplenums[chrt][samplecatch_id]));

						if (!samplen2 && (samplen1 > 0))
							samplen2 = 1;

						if (samplen1 < samplen2)
							nsample = samplen1;
						else
							nsample = samplen2;
						i = 0;

						while (nsample && i <= nsample) {
							wgt = Assess_Add_Error(bm, flagverts, rawwgt, k_avgweight, k_varweight);
							lngth = Get_Length(bm, wgt, sp);
							Sort_Length_Weight(bm, 1, 1, sizecatchnums_id, z, sp, fishery_id, lngth, 1, wgt, cmaxsize, sample_id, &ltc, ofp);

							/* Condition */
							if (bm->sample_now) {
								rawwgt = biolVERTinfo[bres_id][sp][chrt][b];
								sample = Assess_Add_Error(bm, flagverts, rawwgt, k_avgweight, k_varweight);
								individVERTinfo[icondn_id][sp][ltc][z][sample_id] += (sample / (wgt + TINY)) * samplearea[bb] / (nsample + TINY);
							}

							i++;
						}

						sample = samplenums[chrt][samplediscard_id] * totn[samplediscard_id];
						samplen1 = (int) (ceil(sample));
						samplen2 = (int) floor(ROUNDGUARD + (FunctGroupArray[sp].speciesParams[samplesize_id] * samplenums[chrt][samplediscard_id]));

						if (!samplen2 && (samplen1 > 0))
							samplen2 = 1;

						if (samplen1 < samplen2)
							nsample = samplen1;
						else
							nsample = samplen2;
						i = 0;
						while (nsample && i <= nsample) {
							wgt = Assess_Add_Error(bm, flagverts, rawwgt, k_avgweight, k_varweight);
							lngth = Get_Length(bm, wgt, sp);
							Sort_Length_Weight(bm, 1, 1, sizediscardnums_id, z, sp, fishery_id, lngth, 1, wgt, cmaxsize, sample_id, &ltc, ofp);
							i++;
						}
					}
				}
			}
		}
	}

	if (verbose)
		fprintf(stderr, "Update fish stock and individ arrays\n");

	/* Scaling size structure per zone so it sums to one - only do this annually (i.e. once sampling complete) */
	if (bm->annual_sample == numsteps) {
		for (z = 0; z < bm->nfzones; z++) {
			for (i = 0; i < bm->K_num_size; i++) {
				for (sp = 0; sp < bm->K_num_tot_sp; sp++) {
					if (FunctGroupArray[sp].isVertebrate == TRUE) {
						//if ((FunctGroupArray[sp].groupType != BIRD) && (FunctGroupArray[sp].groupType != MAMMAL)) {
						if ((FunctGroupArray[sp].groupType != BIRD && FunctGroupArray[sp].groupType != MAMMAL)) {
							for (j = 2; j < 4; j++)
								totnums[j][sp][z] += sizebins[j][sp][z][i][1][sample_id];
						}
					}
				}
			}
			for (sp = 0; sp < bm->K_num_tot_sp; sp++) {
				if (FunctGroupArray[sp].isVertebrate == TRUE) {
					if (FunctGroupArray[sp].groupType != BIRD) {
						for (i = 0; i < bm->K_num_size; i++) {
							if(FunctGroupArray[sp].groupType != MAMMAL){
								/* Store fish info */
								/* In case never get a fisheries independent estimate for this use
								 caught and discarded fish to give an average weight of individual
								 in that size bin, this value will be replaced by a fisheries
								 independent one if it is collected */
								individVERTinfo[iweight_id][sp][i][z][sample_id] = sizebins[sizeweight_id][sp][z][i][1][sample_id]
										/ (sizebins[sizecatchnums_id][sp][z][i][1][sample_id] + sizebins[sizediscardnums_id][sp][z][i][1][sample_id] + TINY);

								/* Size distribution of catch */
								individVERTinfo[icatchnums_id][sp][i][z][sample_id] = sizebins[sizecatchnums_id][sp][z][i][1][sample_id]
										/ (totnums[totcatchnums_id][sp][z] + TINY);

								/* Size distribution of discards */
								individVERTinfo[idiscards_id][sp][i][z][sample_id] = sizebins[sizediscardnums_id][sp][z][i][1][sample_id]
										/ (totnums[totdiscardnums_id][sp][z] + TINY);
							} else {
								/* Store mammal bycatch info */
								/* Total number of mammals caught */
								stockinfo[scatchnums_id][sp][z][sample_id] += stockinfo[stotcatch_id][sp][z][sample_id]
										* individVERTinfo[icatchnums_id][sp][i][z][sample_id] / (individVERTinfo[iweight_id][sp][i][z][sample_id] + TINY);

								/* Total number of mammals in discards */
								stockinfo[sdiscardnums_id][sp][z][sample_id] += stockinfo[sdiscards_id][sp][z][sample_id]
										* individVERTinfo[idiscards_id][sp][i][z][sample_id] / (individVERTinfo[iweight_id][sp][i][z][sample_id] + TINY);

							}
						}
					} else {
						for (i = 0; i < 3; i++) {
							/* Total number of seabirds caught */
							stockinfo[scatchnums_id][sp][z][sample_id] += stockinfo[stotcatch_id][sp][z][sample_id]
									* individVERTinfo[icatchnums_id][sp][i][z][sample_id] / (individVERTinfo[iweight_id][sp][i][z][sample_id] + TINY);

							/* Total number of seabirds in discards */
							stockinfo[sdiscardnums_id][sp][z][sample_id] += stockinfo[sdiscards_id][sp][z][sample_id]
									* individVERTinfo[idiscards_id][sp][i][z][sample_id] / (individVERTinfo[iweight_id][sp][i][z][sample_id] + TINY);
						}
					}
				}
			}
		}

		/* Correct invertebrate estimates for number of samples in the year */
		correct_size = numsteps;
		for (z = 0; z < bm->nfzones; z++) {
			for (sp = 0; sp < bm->K_num_tot_sp; sp++) {
				if (FunctGroupArray[sp].isVertebrate == FALSE && (FunctGroupArray[sp].isFished == TRUE || FunctGroupArray[sp].isImpacted == TRUE)) {
					for (j = 0; j < (fdiscards_id + 1); j++) {
						invstockinfo[j][sp][z][sample_id] /= correct_size;
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
void FishEstimates(MSEBoxModel *bm, FILE *ofp) {
	int chrt, z, b, bb, i, j, k, samplen1, samplen2, nsample, sp, nzk, ltc, cage, matage, fishery_id, sp_AgeClassSize, stock_id, ai;
	double rawn, sample, rawwgt, q, lngth, avail, bioms, wgt, rawlngth, vol, raw, sum, step1, eaten, min, raw1, raw2, contrib1, contrib2, correct_size;

	int cmaxage = bm->K_max_agekey - 1;
	int cmaxsize = bm->K_num_size - 1;

	int Yr = (int) floor(ROUNDGUARD + ((bm->dayt - bm->tassessstart) / 365.0));

	if (verbose)
		fprintf(stderr, "Sampling fisheries independent data\n");

	/* Initialise the bins */
	if (bm->annual_sample == 1) {
		for (i = 0; i < bm->K_num_size; i++)
			for (b = 0; b < bm->nfzones; b++)
				for (k = 0; k < bm->K_num_tot_sp; k++) {
					if (FunctGroupArray[k].isVertebrate == TRUE) {
						if ((FunctGroupArray[k].groupType != BIRD) && (FunctGroupArray[k].groupType != MAMMAL)) {
							for (j = 0; j < 2; j++)
								sizebins[j][k][b][i][0][sample_id] = 0.0;
						}
					}
				}

		for (k = 0; k < bm->K_num_tot_sp; k++) {
			if (FunctGroupArray[k].isVertebrate == TRUE) {
				if ((FunctGroupArray[k].groupType != BIRD) && (FunctGroupArray[k].groupType != MAMMAL)) {
					for (b = 0; b < bm->nfzones; b++)
						for (j = 0; j < bm->K_max_agekey; j++)
							for (i = 0; i < bm->K_num_size; i++)
								agebins[k][b][j][i][sample_id] = 0.0;
				}
			}
		}

		for (j = 0; j < bm->nfzones; j++)
			for (b = 0; b < bm->K_num_tot_sp; b++) {
				if (FunctGroupArray[b].isVertebrate == TRUE) {

					if ((FunctGroupArray[b].groupType != BIRD) && (FunctGroupArray[b].groupType != MAMMAL)) {
						biom[b][j][sample_id] = 0.0;
					}
				}
			}

		for (i = 0; i < bm->K_num_tot_sp; i++) {
			if (FunctGroupArray[i].isVertebrate == TRUE) {
				if ((FunctGroupArray[i].groupType != BIRD) && (FunctGroupArray[i].groupType != MAMMAL)) {
                    for (z = 0; z < bm->nfzones; z++) {
                        for (j = 0; j < bm->K_num_size; j++) {
							nsq[i][z][j][sample_id] = 0;
                        }
                    }
				}
			}
		}
	}

	/* Get pop ratios */
	for (sp = 0; sp < bm->K_num_tot_sp; sp++) {
		if (FunctGroupArray[sp].isVertebrate == TRUE) {
            for (z = 0; z < bm->nfzones; z++) {
				checkedz[z] = 0;
            }

			sp_AgeClassSize = (int) (FunctGroupArray[sp].ageClassSize);
			for (bb = 0; bb < bm->nsbox; bb++) {
				b = bm->nsboxes[bb];
				z = bm->boxes[b].zone - 1;

				if (!checkedz[z]) {
					/* If not already done this zone find current
					 fine scale age distribution within each cohort
					 */
					for (nzk = 0; nzk < bm->boxes[b].nz; nzk++) {
						if (!checkedz[z]) {
							/* Determine relative depth */
							stock_id = bm->group_stock[sp][b][nzk];

							for (chrt = 0; chrt < FunctGroupArray[sp].numCohortsXnumGenes; chrt++) {
								for (ai = 0; ai < sp_AgeClassSize; ai++) {
									zoneVERTpopratio[sp][chrt][ai][z] = bm->tempPopRatio[stock_id][sp][chrt][ai];
								}
							}
							checkedz[z] = 1;
						}
					}
				}
			}
		}
	}

	for (bb = 0; bb < bm->nsbox; bb++) {
		b = bm->nsboxes[bb];
		z = bm->boxes[b].zone - 1;
		vol = phys[vol_id][b];

		for (sp = 0; sp < bm->K_num_tot_sp; sp++) {
			if (FunctGroupArray[sp].isVertebrate == TRUE) {
				if ((FunctGroupArray[sp].groupType != BIRD) && (FunctGroupArray[sp].groupType != MAMMAL)) {
					fishery_id = (int) (FunctGroupArray[sp].speciesParams[primary_fishery_id]);
					/* Determine subsample numbers */
					for (chrt = 0; chrt < 3; chrt++)
						totn[chrt] = 0.0;

					for (chrt = 0; chrt < bm->K_num_max_cohort * bm->K_num_max_genetypes; chrt++) {
						rawn = biolVERTinfo[bstocknums_id][sp][chrt][b];
						sample = Assess_Add_Error(bm, flagcount, rawn, k_avgcount, k_varcount);
						samplenums[chrt][samplestock_id] = ceil(sample);
						totn[samplestock_id] += samplenums[chrt][samplestock_id];

						/* Collect assessment data */
						bm->CatchRecord[Yr][sp][chrt][survey_id] += sample;

					}
					for (chrt = 0; chrt < bm->K_num_max_cohort * bm->K_num_max_genetypes; chrt++)
						samplenums[chrt][samplestock_id] /= (totn[samplestock_id] + TINY);

					for (chrt = 0; chrt < bm->K_num_max_cohort * bm->K_num_max_genetypes; chrt++) {
                        
                        /*
						if (verbose > 1)
							fprintf(stderr, "Box: %d Independ sampling of %s cohort: %d\n", b, FunctGroupArray[sp].groupCode, chrt);
                         */

						rawwgt = biolVERTinfo[bstruct_id][sp][chrt][b] + biolVERTinfo[bres_id][sp][chrt][b];
						rawlngth = Get_Length(bm, rawwgt, sp);
						//avail = aprm[sp][fishery_id];
						avail = bm->SP_FISHERYprms[sp][fishery_id][avail_id];
						q = Selectivity(bm, rawlngth, fishery_id, avail, 0, ofp);
						sample = samplenums[chrt][samplestock_id] * totn[samplestock_id];
						bioms = sample * q * rawwgt;
						biom[sp][z][sample_id] += bioms;
						samplen1 = (int) ceil(sample * q);
						samplen2 = (int) floor(ROUNDGUARD + (FunctGroupArray[sp].speciesParams[samplesize_id] * samplenums[chrt][samplestock_id]));

						if (!samplen1 && (samplen2 > 0))
							samplen1 = 1;
						if (!samplen2 && (samplen1 > 0))
							samplen2 = 1;

						if (samplen1 < samplen2)
							nsample = samplen1;
						else
							nsample = samplen2;

						i = 0;

						while (nsample && i <= nsample) {
							wgt = Assess_Add_Error(bm, flagverts, rawwgt, k_avgweight, k_varweight);
							lngth = Get_Length(bm, wgt, sp);
							Sort_Length_Weight(bm, 0, 1, sizestocknums_id, z, sp, fishery_id, lngth, 1, wgt, cmaxsize, sample_id, &ltc, ofp);
							/* Assume animal uniformly distributed in age class - may not be right! */

							//TODO Check with beth about changing to a round instead of a floor.
							cage = (int) (round((chrt +  drandom(0.0, 1.0)) * FunctGroupArray[sp].ageClassSize + 0.5));

							Length_Age_Key(bm, z, sp, cmaxsize, lngth, cage, cmaxage, 1, sample_id);
							/* Get estimate of catchability */
							Sort_Length_Weight(bm, 0, 2, sizeselectnums_id, z, sp, fishery_id, lngth, 1, wgt, cmaxsize, sample_id, &ltc, ofp);
							i++;
						}

						if (bm->sample_now) {
							/* Production */
							raw = biolVERTinfo[bprod_id][sp][chrt][b];
							sample = Assess_Add_Error(bm, flagprod, raw, k_avgprodn, k_varprodn);

							stockinfo[sprod_id][sp][z][sample_id] += sample * samplearea[bb] * ceil(biolVERTinfo[bstocknums_id][sp][chrt][b]) / (vol + TINY);

							/* Consumption */
							raw = biolVERTinfo[beat_id][sp][chrt][b];
							sample = Assess_Add_Error(bm, flageat, raw, k_avgeat, k_vareat);
							stockinfo[seat_id][sp][z][sample_id] += sample * samplearea[bb];
						}
					}
				}
			}
		}
	}

	if (verbose)
		fprintf(stderr, "Update fish stock and individ arrays\n");

	/* Scaling size structure per zone so it sums to one - only do this annually (i.e. once sampling complete) */
	if (bm->annual_sample == numsteps) {
		for (z = 0; z < bm->nfzones; z++) {
			for (b = 0; b < bm->K_num_tot_sp; b++) {
				if (FunctGroupArray[b].isVertebrate == TRUE) {
					if ((FunctGroupArray[b].groupType != BIRD) && (FunctGroupArray[b].groupType != MAMMAL)) {
						totnums[totstocknums_id][b][z] = 0.0;
						for (i = 0; i < bm->K_num_size; i++) {
							totnums[totstocknums_id][b][z] += sizebins[sizestocknums_id][b][z][i][0][sample_id];
						}
					}
				}
			}
			/* Assign final values */
			for (sp = 0; sp < bm->K_num_tot_sp; sp++) {
				if (FunctGroupArray[sp].isVertebrate == TRUE) {
					if ((FunctGroupArray[sp].groupType != BIRD) && (FunctGroupArray[sp].groupType != MAMMAL)) {
						/* Stock biomass */
						stockinfo[sbiomass_id][sp][z][sample_id] = biom[sp][z][sample_id] / 2.0;

						for (i = 0; i < bm->K_num_size; i++) {

							fprintf(bm->logFile, "nsq[sp][z][i][sample_id] = %d\n", nsq[sp][z][i][sample_id]);
							/* Catchability */
							individVERTinfo[iq_id][sp][i][z][sample_id] = sizebins[sizeselectnums_id][sp][z][i][0][sample_id] / (nsq[sp][z][i][sample_id]
									+ TINY);

							/* Size distribution of stock - correcting for selectivity */
							individVERTinfo[istocknums_id][sp][i][z][sample_id] = (sizebins[sizestocknums_id][sp][z][i][0][sample_id]
									/ (totnums[totstocknums_id][sp][z] + TINY)) / (individVERTinfo[iq_id][sp][i][z][sample_id] + TINY);

							fprintf(ofp, "Time: %e, %s-%d, z: %d, individVERTinfo: %e, sizebin; %e, totnums: %e, iq: %e\n", bm->dayt,
									FunctGroupArray[sp].groupCode, i, z, individVERTinfo[istocknums_id][sp][i][z][sample_id],
									sizebins[sizestocknums_id][sp][z][i][0][sample_id], totnums[totstocknums_id][sp][z],
									individVERTinfo[iq_id][sp][i][z][sample_id]);

							/* Average weight of individual in that size bin */
							step1 = sizebins[sizeweight_id][sp][z][i][0][sample_id] / (sizebins[sizestocknums_id][sp][z][i][0][sample_id] + TINY);
							if (step1 > 0)
								individVERTinfo[iweight_id][sp][i][z][sample_id] = step1;

							/* Numbers in the catch */
							stockinfo[scatchnums_id][sp][z][sample_id] += stockinfo[stotcatch_id][sp][z][sample_id]
									* individVERTinfo[icatchnums_id][sp][i][z][sample_id] / (individVERTinfo[iweight_id][sp][i][z][sample_id] + TINY);

							/* Numbers in the discards */
							stockinfo[sdiscardnums_id][sp][z][sample_id] += stockinfo[sdiscards_id][sp][z][sample_id]
									* individVERTinfo[idiscards_id][sp][i][z][sample_id] / (individVERTinfo[iweight_id][sp][i][z][sample_id] + TINY);

							/* Age-length keys */
							sum = 0.0;
							for (j = 0; j < bm->K_max_agekey; j++) {
								agelengthkey[i][j][sp][z][sample_id] = agebins[sp][z][j][i][sample_id];
								/* Normalise so each length bin adds up to one across the ages */
								sum += agelengthkey[i][j][sp][z][sample_id];
							}
							for (j = 0; j < bm->K_max_agekey; j++)
								agelengthkey[i][j][sp][z][sample_id] /= (sum + TINY);
						}

						/* Reset total numbers */
						totnums[totstocknums_id][sp][z] = 0.0;
						for (i = 0; i < bm->K_num_size; i++) {
							totnums[totstocknums_id][sp][z] += individVERTinfo[istocknums_id][sp][i][z][sample_id];
						}

						/* Rescale size distribution so it sums to one */
						for (i = 0; i < bm->K_num_size; i++) {
							individVERTinfo[istocknums_id][sp][i][z][sample_id] = individVERTinfo[istocknums_id][sp][i][z][sample_id]
									/ (totnums[totstocknums_id][sp][z] + TINY);

							fprintf(ofp, "Time: %e %s-%d new individVERT: %e (totnums: %e)\n", bm->dayt, FunctGroupArray[sp].groupCode, i,
									individVERTinfo[istocknums_id][sp][i][z][sample_id], totnums[totstocknums_id][sp][z]);

							/* Do stuff that was dependent on rescaled size distribution */
							/* Biomass per size bin - corrected for selectivity */
							individVERTinfo[ibiomass_id][sp][i][z][sample_id] = stockinfo[sbiomass_id][sp][z][sample_id]
									* individVERTinfo[istocknums_id][sp][i][z][sample_id];

							/* Numbers in the stock - corrected for selectivity */
							stockinfo[sstocknums_id][sp][z][sample_id] += stockinfo[sbiomass_id][sp][z][sample_id]
									* individVERTinfo[istocknums_id][sp][i][z][sample_id] / (individVERTinfo[iweight_id][sp][i][z][sample_id] + TINY);

							fprintf(ofp, "Time: %e %s-%d z: %d, stocknums: %e, stockinfo: %e, individVERT: %e, individWGT: %e\n", bm->dayt,
									FunctGroupArray[sp].groupCode, i, z, stockinfo[sstocknums_id][sp][z][sample_id], stockinfo[sbiomass_id][sp][z][sample_id],
									individVERTinfo[istocknums_id][sp][i][z][sample_id], individVERTinfo[iweight_id][sp][i][z][sample_id]);
						}
					}
				}
			}
		}

		/* Calculate minimum consumption and population make-up for vertebrates */
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

								contrib1 = individVERTinfo[istocknums_id][sp][i][z][sample_id] * agelengthkey[i][chrt][sp][z][sample_id];
								raw1 += contrib1;
								contrib2 = individVERTinfo[icatchnums_id][sp][i][z][sample_id] * agelengthkey[i][chrt][sp][z][sample_id];
								raw2 += contrib2;
							}
						}
						/* Percentage of the stock that are juveniles */
						pop_fraction[0][sp][z] = max(0, raw1);

						/* Percentage of the catch that are juveniles */
						pop_fraction[1][sp][z] = max(0, raw2);
						/* Minimum consumption - test juveniles then adult population components */
						eaten = stockinfo[seat_id][sp][z][sample_id] * raw1;
						if ((eaten > 0.0) && (eaten < min))
							min = eaten;
						raw = 1.0 - pop_fraction[0][sp][z];
						eaten = stockinfo[seat_id][sp][z][sample_id] * raw;
						if ((eaten > 0.0) && (eaten < min))
							min = eaten;
					} else {
						/* Birds */
						if ((stockinfo[seat_id][sp][z][sample_id] > 0.0) && (stockinfo[seat_id][sp][z][sample_id] < min))
							min = stockinfo[seat_id][sp][z][sample_id];
					}
				}
			}
			/* Store result for the zone */
			if (min < mineat[z][sample_id])
				mineat[z][sample_id] = min;

			if (verbose > 2) {
				fprintf(ofp, "mineat[%d]: %e\n", z, mineat[z][sample_id]);
				for (sp = 0; sp < bm->K_num_tot_sp; sp++) {
					if (FunctGroupArray[sp].isVertebrate == TRUE) {
						fprintf(ofp, "stockinfo[sbiomass_id][%s][%d]: %e\n", FunctGroupArray[sp].groupCode, z, stockinfo[sbiomass_id][sp][z][sample_id]);
						fprintf(ofp, "stockinfo[sstocknums_id][%s][%d]: %e\n", FunctGroupArray[sp].groupCode, z, stockinfo[sstocknums_id][sp][z][sample_id]);
						fprintf(ofp, "stockinfo[scatchnums_id][%s][%d]: %e\n", FunctGroupArray[sp].groupCode, z, stockinfo[scatchnums_id][sp][z][sample_id]);
						fprintf(ofp, "stockinfo[sprod_id][%s][%d]: %e\n", FunctGroupArray[sp].groupCode, z, stockinfo[sprod_id][sp][z][sample_id]);
						fprintf(ofp, "stockinfo[seat_id][%s][%d]: %e\n", FunctGroupArray[sp].groupCode, z, stockinfo[seat_id][sp][z][sample_id]);
						fprintf(ofp, "stockinfo[sdiscardnums_id][%s][%d]: %e\n", FunctGroupArray[sp].groupCode, z, stockinfo[sdiscardnums_id][sp][z][sample_id]);
						fprintf(ofp, "stockinfo[stotcatch_id][%s][%d]: %e\n", FunctGroupArray[sp].groupCode, z, stockinfo[stotcatch_id][sp][z][sample_id]);
						fprintf(ofp, "stockinfo[stoteffort_id][%s][%d]: %e\n", FunctGroupArray[sp].groupCode, z, stockinfo[stoteffort_id][sp][z][sample_id]);
						fprintf(ofp, "stockinfo[sdiscards_id][%s][%d]: %e\n", FunctGroupArray[sp].groupCode, z, stockinfo[sdiscards_id][sp][z][sample_id]);
						fprintf(ofp, "stockinfo[ssizemat_id][%s][%d]: %e\n", FunctGroupArray[sp].groupCode, z, stockinfo[ssizemat_id][sp][z][sample_id]);
						fprintf(ofp, "stockinfo[scondn_id][%s][%d]: %e\n", FunctGroupArray[sp].groupCode, z, stockinfo[scondn_id][sp][z][sample_id]);
					}
				}

				for (sp = 0; sp < bm->K_num_tot_sp; sp++) {
					if (FunctGroupArray[sp].isVertebrate == TRUE) {
						for (j = 0; j < bm->K_num_size; j++) {
							fprintf(ofp, "individVERTinfo[iweight_id][%s][%d][%d]: %e\n", FunctGroupArray[sp].groupCode, j, z, individVERTinfo[iweight_id][sp][j][z][sample_id]);
							fprintf(ofp, "individVERTinfo[istocknums_id][%s][%d][%d]: %e\n", FunctGroupArray[sp].groupCode, j, z, individVERTinfo[istocknums_id][sp][j][z][sample_id]);
							fprintf(ofp, "individVERTinfo[icatchnums_id][%s][%d][%d]: %e\n", FunctGroupArray[sp].groupCode, j, z, individVERTinfo[icatchnums_id][sp][j][z][sample_id]);
							fprintf(ofp, "individVERTinfo[ibiomass_id][%s][%d][%d]: %e\n", FunctGroupArray[sp].groupCode, j, z, individVERTinfo[ibiomass_id][sp][j][z][sample_id]);
							fprintf(ofp, "individVERTinfo[idiscards_id][%s][%d][%d]: %e\n", FunctGroupArray[sp].groupCode, j, z, individVERTinfo[idiscards_id][sp][j][z][sample_id]);
							fprintf(ofp, "individVERTinfo[iq_id][%s][%d][%d]: %e\n", FunctGroupArray[sp].groupCode, j, z, individVERTinfo[iq_id][sp][j][z][sample_id]);
							fprintf(ofp, "individVERTinfo[icondn_id][%s][%d][%d]: %e\n", FunctGroupArray[sp].groupCode, j, z, individVERTinfo[icondn_id][sp][j][z][sample_id]);
						}
					}
				}
			}
		}

		/* Correct vertebrate estimates for number of samples in the year (including charismatics) */
		correct_size = numsteps;
		for (z = 0; z < bm->nfzones; z++) {
			for (sp = 0; sp < bm->K_num_tot_sp; sp++) {
				if (FunctGroupArray[sp].isVertebrate == TRUE) {
					for (j = 0; j < (sdiscards_id + 1); j++) { // As sizemat and scondn calculated as indicators
						stockinfo[j][sp][z][sample_id] /= correct_size;
					}
				}
			}
			for (sp = 0; sp < bm->K_num_tot_sp; sp++) {
				if(FunctGroupArray[sp].groupType == BIRD || FunctGroupArray[sp].groupType == MAMMAL){
					oldbaby[sp][z][sample_id] /= correct_size;
				}
			}
		}
	}

	return;
}

/*
 * \brief Sorting animals into size bins
 If flag set = 0 then want length only
 flag = 1 want weight of the fish as well as the length
 flag = 2 want selectivity estimate for fish that size

 Also returns the size class the animal was sorted into so that condition factors can be stored
 for that animal in the entries for that size class
 */
void Sort_Length_Weight(MSEBoxModel *bm, int casetype, int flag, int ni, int z, int i, int fishery_id, double l, double num, double w, int ci, int id,
		int *lengthclass, FILE *ofp) {
	int chk = 0, k = 0;
	double val = 0, q = 0;

	while ((chk == 0) && (k < ci)) {
		val = FunctGroupArray[i].speciesParams[allometic_bin_start_id] + FunctGroupArray[i].speciesParams[allometic_bin_size_id] * (k + 1);
		if (l <= val) {
			switch (flag) {
			case 0:
				sizebins[ni][i][z][k][casetype][id] += num;
				break;
			case 1:
				sizebins[sizeweight_id][i][z][k][casetype][id] += w * num;
				sizebins[ni][i][z][k][casetype][id] += num;

				break;
			case 2:
				q = Selectivity(bm, l, fishery_id, 1, 1, ofp);
				sizebins[sizeselectnums_id][i][z][k][casetype][id] += q;
				nsq[i][z][k][id]++;
				break;
			default:
				quit("flag for Sort_Length_Weight, must be 0, 1 or 2\n");
				break;
			}

			chk = 1;
			/* Return index of the size bin the animal was sorted into */
			*lengthclass = k;
		}
		k++;
	}
	if (chk == 0) {
		switch (flag) {
		case 0:
			sizebins[ni][i][z][ci][casetype][id] += num;
			break;
		case 1:
			sizebins[sizeweight_id][i][z][ci][casetype][id] += w * num;
			sizebins[ni][i][z][ci][casetype][id] += num;
			break;
		case 2:
			q = Selectivity(bm, l, fishery_id, 1, 1, ofp);
			sizebins[sizeselectnums_id][i][z][ci][casetype][id] += q;
			nsq[i][z][ci][id]++;
			break;
		default:
			quit("flag for Sort_Length_Weight, must be 0, 1 or 2\n");
			break;
		}
		/* Return index of the size bin the animal was sorted into */
		*lengthclass = ci;
	}
}

/**
 * \brief Age sampled fish and create age length key
 */
void Length_Age_Key(MSEBoxModel *bm, int z, int i, int ni, double l, int c, int cmax, double n, int id) {
	int chk = 0, k = 0, samplec;
	double val, sample;

	sample = Assess_Add_Error(bm, flagage, c, k_avgage, k_varage);
	samplec = (int) (sample);
	if (samplec > cmax)
		samplec = cmax;

	k = 0;
	while ((chk == 0) && (k < ni)) {
		val = FunctGroupArray[i].speciesParams[allometic_bin_start_id] + (FunctGroupArray[i].speciesParams[allometic_bin_size_id] * (k + 1));
		if (l <= val) {
			agebins[i][z][samplec][k][id] += n;
			chk = 1;
		}
		k++;
	}
	if (chk == 0) {
		agebins[i][z][samplec][ni][id] += n;
	}
}

double Get_Length(MSEBoxModel *bm, double w, int sp) {
	double ans, wgt, w_on_a, lt, bpow;

	//wgt = w * 5.7 * mg_2_g;
    wgt = w * bm->k_wetdry * bm->X_CN * mg_2_g;
	w_on_a = wgt / FunctGroupArray[sp].speciesParams[allometic_li_a_id];
	bpow = (1.0 / (FunctGroupArray[sp].speciesParams[allometic_li_b_id] + TINY));

	lt = pow(w_on_a, bpow);
	ans = ceil(lt * 100.0);
	ans /= 100.0;
    
	return (ans);
}
/**
 * Never used.
 */
double Get_Weight(double l, int sp) {
	double ans, wgt, bpow;

	bpow = FunctGroupArray[sp].speciesParams[allometic_li_b_id];
	wgt = FunctGroupArray[sp].speciesParams[allometic_li_a_id] * pow(l, bpow);
	ans = wgt;

	return (ans);
}

double Selectivity(MSEBoxModel *bm, double li, int i, double av, int err, FILE *ofp) {

	int q_curve;
	double ans = 0, q_lsm, q_sigma, step1, step2, step3, raw, sample, bpow, apow;

	q_curve = (int) (bm->FISHERYprms[i][assess_selcurve_id]);
	q_lsm = bm->FISHERYprms[i][q_lsm_id];
	q_sigma = bm->FISHERYprms[i][q_sigma_id];

	if (err) {
		if (flagcurve == 4) {
			//q_curve = q_curve;  Do nothing
		} else {
			raw = q_curve;
			sample = Assess_Add_Error(bm, flagcurve, raw, k_avgcurve, k_varcurve);
			q_curve = (int) (floor(sample + 0.5));
		}
		raw = q_lsm;
		q_lsm = Assess_Add_Error(bm, flagprms, raw, k_avgprms, k_varprms);
		raw = q_sigma;
		q_sigma = Assess_Add_Error(bm, flagprms, raw, k_avgprms, k_varprms);
	}

	switch (q_curve) {
	case const_q: /* Constant proportion of age class */
		ans = q_lsm;
		break;
	case logistic_q: /* Dynamic so selectivity based on size, logistic */
		ans = 1.0 / (1.0 + exp(-q_sigma * (li - q_lsm)));
		break;
	case normal_q: /* Dynamic so selectivity based on size, normal */
		step1 = li - q_lsm;
		step2 = -(step1 * step1) / (2.0 * q_sigma * q_sigma + TINY);
		ans = exp(step2);
		break;
	case lognorm_q: /* Dynamic so selectivity based on size, lognormal */
		step1 = 1.0 / (q_sigma * sqrt(2.0 * 3.141592654) + TINY);
		step2 = (log(li) - q_lsm) * (log(li) - q_lsm);
		step3 = 2.0 * q_sigma * q_sigma + TINY;
		ans = step1 * exp(-step2 / step3);
		break;
	case gamma_q: /* Dynamic so selectivity based on size, gamma */
		step1 = q_lsm * q_lsm + 4.0 * q_sigma * q_sigma;
		step2 = ((sqrt(step1) - q_lsm) / 2.0) + TINY;
		apow = li / (q_lsm + TINY);
		bpow = q_lsm / step2;
		ans = pow(apow, bpow) * exp((q_lsm - li) / step2);
		break;
	default:
		quit("No such selectivtiy curve defined (%d) - how did it get here?\n", q_curve);
		break;
	}

	/* Put in sanity check */
	if (ans < 0.0)
		ans = 0.0;

	ans *= av;

	return (ans);
}

double Assess_Add_Error(MSEBoxModel *bm, int er_case, double true_val, double a, double v) {
	double ans = 0.0, step1, step2, step3;

	//return true_val;
	//double rndnum = drandom(0.0, 1.0);

	switch (er_case) {
	case uniform_err: /* Uniform */
		step1 = -a * true_val;
		step2 = a * true_val;
		ans = true_val + drandom(step1, step2);
		break;
	case normal_err: /* Normal */
		step1 = drandom(0.0, 1.0);
		step2 = drandom(0.0, 1.0);
		step3 = sqrt(-2.0 * log(step1)) * cos(2.0 * 3.1415926 * step2);
		ans = step3 * sqrt(v) + true_val * a;
		break;
	case lognorm_err: /* Lognormal - note - v/2 is the bias correction term
	 needed when generating lognormal by exponentiating a standard normal */
		step1 = drandom(0.0, 1.0);
		step2 = drandom(0.0, 1.0);
		step3 = sqrt(-2.0 * log(step1)) * cos(2.0 * 3.1415926 * step2);
		ans = exp(step3 * sqrt(v) - v / 2.0) * (true_val * a);
		break;
	default:
		quit("No such error case defined - how did it get here?\n");
		break;
	}
    
    //fprintf(bm->logFile, "er_case: %d true_val: %e a: %e, v: %e step1: %e step2 %e step3 %e ans %e\n",
    //        er_case, true_val, a, v, step1, step2, step3, ans);

	return (ans);
}

