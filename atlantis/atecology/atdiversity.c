/**
 \file
 \brief C file for evolution related routines
 \ingroup atecology

 File:	atdiversity.c
 Author:	Beth Fulton
 Created:	23/07/2013
 Purpose:	C file for all evolution subroutines
 Revisions:

 ****************************************************************************************************/

/*  Open library routines */
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include <math.h>
#include <sjwlib.h>
#include "atecology.h"

/* Prototypes of routines */
void Apply_Evol_Scalar(MSEBoxModel *bm, int species, int cohort, int ntrait, double evol_scalar, FILE *llogfp);
double Get_Trait_value(MSEBoxModel *bm, int species, int cohort, int ntrait, FILE *llogfp);
double Get_Orig_Trait_value(MSEBoxModel *bm, int species, int cohort, int ntrait, FILE *llogfp);
double Get_Mean_Orig_Value(MSEBoxModel *bm, int species, int age_class, int ntrait, FILE *llogfp);
double Get_Mean_Value(MSEBoxModel *bm, int species, int age_class, int ntrait, FILE *llogfp);

/*****************************************************************************************************************
 * Evolution routines
 */

/**
 *	\brief Find evolutionary relevant values
 */
void Find_Evolution_Stats(MSEBoxModel *bm, int species, FILE *llogfp, int init_case) {
    int cohort, base_chrt, ntrait, np, den = 0, b, k, sn = 0, rn =0, na;
    int age_classes = FunctGroupArray[species].ageClassSize;
	long double param_value, val_diff, min_var, tot_abund;
	double ngene = (double)(FunctGroupArray[species].numGeneTypes);
    double size_count = 0.0;

	if((!FunctGroupArray[species].speciesParams[flag_id]) || (!bm->flag_do_evolution) || (FunctGroupArray[species].numGeneTypes < 2))
		return;  // As not active or evolution not active

	if (init_case || ((bm->current_box == bm->first_box) && (bm->current_layer == bm->top_layer))) { // Get values if starting model or doing reproduction
		/* Compare mean of realised distribution of size-at-age vs theoretical
		 * value would have got if mean evenly distributed across all
		 * genetic_type-age-bins
		 */

		// Intialise
		for (cohort = 0; cohort < FunctGroupArray[species].numCohorts; cohort++) {
			DNA[species].tot_num[cohort] = 0.0;
			for (ntrait = 0; ntrait < K_num_traits; ntrait++) {
				for (np = 0; np < K_num_evol_prop; np++) {
					DNA[species].trait[ntrait][cohort][np] = 0.0;
				}
            
                if(!bm->dayt) { // Initialise the base value at start of the run
                    DNA[species].trait[ntrait][cohort][evol_theoretmean_id] = Get_Mean_Orig_Value(bm, species, cohort, ntrait, llogfp);
                    DNA[species].trait_shift[ntrait][cohort][0][evol_birth_var_id] = FunctGroupArray[species].speciesParams[trait_variance_id] * DNA[species].trait[ntrait][cohort][evol_theoretmean_id];  // Set initial variability levels

                    for(na=0; na<age_classes; na++ ){
                        DNA[species].trait_shift[ntrait][cohort][na][evol_birth_shift_id] = 1.0;
                        DNA[species].trait_shift[ntrait][cohort][na][evol_birth_var_id] = 0.0;
                    }

                }
            }
		}
        
		/* Step 1 of weighted average is to sum values over all individuals in that age group (adult age groups only) */
        tot_abund = 0.0;
        for (cohort = 0; cohort < FunctGroupArray[species].numCohortsXnumGenes; cohort++) {
			base_chrt = cohort / FunctGroupArray[species].numGeneTypes;
            
            /* Initialise */
            size_count = 0.0;
			DNA[species].num[cohort] = 0.0;  // Need nums per gene_by_cohort_bin for doing the weighted average step1
            DNA[species].sn[cohort] = 0.0;
            DNA[species].rn[cohort] = 0.0;
            
            switch (FunctGroupArray[species].groupAgeType) {
            case AGE_STRUCTURED:
                den = FunctGroupArray[species].NumsTracers[cohort];
                sn = FunctGroupArray[species].structNTracers[cohort];
                rn = FunctGroupArray[species].resNTracers[cohort];
                break;
            case BIOMASS:				/* Intentional follow through - base on total biomass rather than numbers for these biomass pool groups */
            case AGE_STRUCTURED_BIOMASS:
                den = FunctGroupArray[species].totNTracers[cohort];
                break;
            }

			// Add numbers per box and layer
			for ( b = 0 ; b < bm->nbox; b++ ){
				if (bm->boxes[b].type != BOUNDARY){
					for (k = 0; k < bm->boxes[b].nz; k++) {
						switch (FunctGroupArray[species].groupAgeType) {
						case AGE_STRUCTURED:
							DNA[species].tot_num[base_chrt] += bm->boxes[b].tr[k][den] * FunctGroupArray[species].habitatCoeffs[WC];
							DNA[species].num[cohort] += bm->boxes[b].tr[k][den] * FunctGroupArray[species].habitatCoeffs[WC];
                            tot_abund += bm->boxes[b].tr[k][den] * FunctGroupArray[species].habitatCoeffs[WC];
                            if ( bm->boxes[b].tr[k][den] > bm->min_dens) {
                                DNA[species].sn[cohort] += bm->boxes[b].tr[k][sn] * FunctGroupArray[species].habitatCoeffs[WC];
                                DNA[species].rn[cohort] += bm->boxes[b].tr[k][rn] * FunctGroupArray[species].habitatCoeffs[WC];
                                size_count += 1.0;
                            }
							break;
						case BIOMASS:				/* Intentional follow through - base on total biomass rather than numbers for these biomass pool groups */
						case AGE_STRUCTURED_BIOMASS:
                            if (FunctGroupArray[species].habitatType == EPIFAUNA){
                                DNA[species].tot_num[base_chrt] += bm->boxes[b].epi[den] * bm->boxes[b].area * FunctGroupArray[species].habitatCoeffs[EPIFAUNA];
                                DNA[species].num[cohort] += bm->boxes[b].epi[den] * bm->boxes[b].area * FunctGroupArray[species].habitatCoeffs[EPIFAUNA];
                                tot_abund += bm->boxes[b].epi[den] * bm->boxes[b].area * FunctGroupArray[species].habitatCoeffs[EPIFAUNA];
                            } else {
                                DNA[species].tot_num[base_chrt] += bm->boxes[b].tr[k][den] * bm->boxes[b].dz[k] * bm->boxes[b].area * FunctGroupArray[species].habitatCoeffs[WC];
                                DNA[species].num[cohort] += bm->boxes[b].tr[k][den] * bm->boxes[b].dz[k] * bm->boxes[b].area * FunctGroupArray[species].habitatCoeffs[WC];
                                tot_abund += bm->boxes[b].tr[k][den] * bm->boxes[b].dz[k] * bm->boxes[b].area * FunctGroupArray[species].habitatCoeffs[WC];
                            }
							break;
						}
					}
				}
			}
            
            // Finish getting the size information
            switch (FunctGroupArray[species].groupAgeType) {
                case AGE_STRUCTURED:
                    DNA[species].sn[cohort] /= (size_count + small_num);
                    DNA[species].rn[cohort] /= (size_count + small_num);
                    break;
                case BIOMASS:				/* Intentional follow through - base on total biomass rather than numbers for these biomass pool groups */
                case AGE_STRUCTURED_BIOMASS:
                    DNA[species].sn[cohort] += FunctGroupArray[species].sn[base_chrt];
                    DNA[species].rn[cohort] += FunctGroupArray[species].sn[base_chrt] * FunctGroupArray[species].X_RS[base_chrt];
                    break;
            }


			/* Loop over traits
			 * mean value += param_value * num_in_gene_by_cohort_bin
			 * theoretmean += param_value for that gene_by_cohort_bin
			 */
			for (ntrait = 0; ntrait < K_num_traits; ntrait++) {
				if(DNA[species].trait_active[ntrait] < 1)
					continue;  // As not active so don't do anything

				param_value = Get_Trait_value(bm, species, cohort, ntrait, llogfp);
				DNA[species].trait[ntrait][base_chrt][evol_mean_id] += (DNA[species].num[cohort] * param_value);
				DNA[species].trait[ntrait][base_chrt][evol_theoretmean_id] += param_value;
                
                fprintf(llogfp,"STATS %s Time: %e trait: %d base_chrt: %d cohort: %d evol_mean: %Le num: %e param_value: %Le theoret_mean: %Le\n",
                        FunctGroupArray[species].groupCode, bm->dayt, ntrait, base_chrt, cohort, DNA[species].trait[ntrait][base_chrt][evol_mean_id],
                        DNA[species].num[cohort], param_value, DNA[species].trait[ntrait][base_chrt][evol_theoretmean_id]);

			}
		}

		/* Finalise weighted average by dividing by total numbers in the age group */
		for (ntrait = 0; ntrait < K_num_traits; ntrait++) {
			if(DNA[species].trait_active[ntrait] < 1)
				continue;  // As not active so don't do anything

			for (cohort = 0; cohort < FunctGroupArray[species].numCohorts; cohort++) {
                if(DNA[species].trait_active[ntrait] < 1)
					continue;  // As not active so don't do anything

				DNA[species].trait[ntrait][cohort][evol_mean_id] /= DNA[species].tot_num[cohort];
				DNA[species].trait[ntrait][cohort][evol_theoretmean_id] /= ngene;
                
                fprintf(llogfp,"STATS %s Time: %e trait: %d cohort: %d evol_mean: %Le tot_num: %e theoret_mean: %Le\n",
                        FunctGroupArray[species].groupCode, bm->dayt, ntrait, cohort, DNA[species].trait[ntrait][cohort][evol_mean_id],
                        DNA[species].tot_num[cohort], DNA[species].trait[ntrait][cohort][evol_theoretmean_id]);

			}

		/* Step 2 calculate the standard deviation */
			for (cohort = 0; cohort < FunctGroupArray[species].numCohortsXnumGenes; cohort++) {
				base_chrt = cohort / FunctGroupArray[species].numGeneTypes;

                param_value = Get_Trait_value(bm, species, cohort, ntrait, llogfp);
				val_diff = param_value - DNA[species].trait[ntrait][base_chrt][evol_theoretmean_id];
				val_diff *= val_diff;  // squaring it step
				DNA[species].trait[ntrait][base_chrt][evol_var_id] += (val_diff / (ngene - 1.0));
                
                fprintf(llogfp,"STATS %s Time: %e trait: %d base_chrt: %d cohort: %d evol_var: %Le param_value: %Le theoret_mean: %Le val_diff: %Le ngene: %e\n",
                        FunctGroupArray[species].groupCode, bm->dayt, ntrait, base_chrt, cohort, DNA[species].trait[ntrait][base_chrt][evol_var_id],
                        param_value, DNA[species].trait[ntrait][base_chrt][evol_theoretmean_id], val_diff, ngene);
                
			}

		/* Laugh check the standard deviation */
            if (bm->flag_evolvar_capped) {
                for (cohort = 0; cohort < FunctGroupArray[species].numCohorts; cohort++) {
                    base_chrt = cohort * FunctGroupArray[species].numGeneTypes;
                    param_value = Get_Trait_value(bm, species, base_chrt, ntrait, llogfp);
                    min_var = FunctGroupArray[species].speciesParams[min_trait_variance_id] * FunctGroupArray[species].speciesParams[min_trait_variance_id] * param_value;
                    if (DNA[species].trait[ntrait][cohort][evol_var_id] < min_var)
                        DNA[species].trait[ntrait][cohort][evol_var_id] = min_var;
                }
            }
        }
    }
    
    return;
}

/**
 *	\brief Find trait shift during reproduction
 */
void Find_Evolution_Shift(MSEBoxModel *bm, int species, FILE *llogfp) {
    int cohort, base_chrt, ntrait, den = 0, b, k;
    long double val_diff, tot_abund, larval_mean, spawn_abund, heritability, stdev_trait,
        step1, step2, step3, orig_value, old_value, stdev;
    double ngene = (double)(FunctGroupArray[species].numGeneTypes);
    int age_mat = (int) (FunctGroupArray[species].speciesParams[age_mat_id]);
    int startage = age_mat * FunctGroupArray[species].numGeneTypes;
    double KSPA_sp = FunctGroupArray[species].speciesParams[KSPA_id];
    double FSP_sp = FunctGroupArray[species].speciesParams[FSP_id];
    double RSprop_sp;
    
        
    if((!FunctGroupArray[species].speciesParams[flag_id]) || (!bm->flag_do_evolution) || (FunctGroupArray[species].numGeneTypes < 2))
        return;  // As not active or evolution not active
        
    if ((bm->current_box == bm->first_box) && (bm->current_layer == bm->top_layer)) {
        /* Get total numbers */
        tot_abund = 0.0;
        for (cohort = startage; cohort < FunctGroupArray[species].numCohortsXnumGenes; cohort++) {
			base_chrt = cohort / FunctGroupArray[species].numGeneTypes;
            
            switch (FunctGroupArray[species].groupAgeType) {
                case AGE_STRUCTURED:
                    den = FunctGroupArray[species].NumsTracers[cohort];
                    break;
                case BIOMASS:				/* Intentional follow through - base on total biomass rather than numbers for these biomass pool groups */
                case AGE_STRUCTURED_BIOMASS:
                    den = FunctGroupArray[species].totNTracers[cohort];
                    break;
            }
            
			// Add numbers per box and layer
			for ( b = 0 ; b < bm->nbox; b++ ){
				if (bm->boxes[b].type != BOUNDARY){
					for (k = 0; k < bm->boxes[b].nz; k++) {
						switch (FunctGroupArray[species].groupAgeType) {
                            case AGE_STRUCTURED:
                                tot_abund += bm->boxes[b].tr[k][den] * FunctGroupArray[species].habitatCoeffs[WC];
                                break;
                            case BIOMASS:				/* Intentional follow through - base on total biomass rather than numbers for these biomass pool groups */
                            case AGE_STRUCTURED_BIOMASS:
                                if (FunctGroupArray[species].habitatType == EPIFAUNA){
                                    tot_abund += bm->boxes[b].epi[den] * bm->boxes[b].area * FunctGroupArray[species].habitatCoeffs[EPIFAUNA];
                                } else {
                                    tot_abund += bm->boxes[b].tr[k][den] * bm->boxes[b].dz[k] * bm->boxes[b].area * FunctGroupArray[species].habitatCoeffs[WC];
                                }
                                break;
						}
					}
				}
			}
        }
        
        
        for (ntrait = 0; ntrait < K_num_traits; ntrait++) {
            if(DNA[species].trait_active[ntrait] < 1)
                continue;  // As not active so don't do anything

        /* Initialise */
            DNA[species].scaled_change[ntrait] = 0.0;
            
        /* Find relative val_diff in stdev units */
            for (cohort = age_mat; cohort < FunctGroupArray[species].numCohorts; cohort++) {
                val_diff = (DNA[species].trait[ntrait][cohort][evol_mean_id] - DNA[species].trait[ntrait][cohort][evol_theoretmean_id]);
                stdev_trait = sqrt(DNA[species].trait[ntrait][cohort][evol_var_id]);
                DNA[species].trait[ntrait][cohort][evol_relvaldiff_id] = val_diff / (stdev_trait + small_num);
                
                fprintf(llogfp,"SHIFT %s Time: %e trait: %d cohort: %d val_diff: %Le (mean: %Le theoretmean: %Le) stdev_trait: %Le evol_relvaldiff: %Le\n",
                        FunctGroupArray[species].groupCode, bm->dayt, ntrait, cohort, val_diff, DNA[species].trait[ntrait][cohort][evol_mean_id], DNA[species].trait[ntrait][cohort][evol_theoretmean_id], stdev_trait, DNA[species].trait[ntrait][cohort][evol_relvaldiff_id]);
                
            }
           
        /* Start calculating the larval mean shift - once have the new evolutionary mean per age class do calculation
           as weighted average of appropriate adult shifts. This gives the new larval mean and then that is
           applied in Do_Evolution (in combo with the fixed larval std-dev curve spread) to get the evol_scalar to apply
           to evolve the parameter value */
            spawn_abund = 0.0;
            for (cohort = startage; cohort < FunctGroupArray[species].numCohortsXnumGenes; cohort++) {
                base_chrt = cohort / FunctGroupArray[species].numGeneTypes;
                
                //if it is RSprop_sp that is evolving then get dynamically updated value of that genotype //AMAY
                if (DNA[species].trait_active[rsprop_trait_id]) {
                    RSprop_sp = bm->evolTraitValues[species][cohort][rsprop_trait_id]; //NOTE that here cohort refers to the genotype
                } else {
                    RSprop_sp = FunctGroupArray[species].speciesParams[RSprop_id]; // else just use the stable RSprop value
                }

                // Find spawn contribution - internal to model domain
                step1 = Ecology_Age_Structured_Spawn(species, KSPA_sp, FSP_sp, RSprop_sp, DNA[species].sn[cohort], DNA[species].rn[cohort], FunctGroupArray[species].scaled_FSPB[cohort], FunctGroupArray[species].X_RS[cohort], bm->flag_repcostSpawn, llogfp);
                step2 = FunctGroupArray[species].scaled_FSPB[cohort] * step1 * DNA[species].num[cohort];
                
                // Find spawn contribution - external to model domain
                step3 = Ecology_Age_Structured_Spawn(species, KSPA_sp, FSP_sp, RSprop_sp, MIGRATION[species].SN[cohort][expect_id], MIGRATION[species].RN[cohort][expect_id], FunctGroupArray[species].scaled_FSPB[cohort], FunctGroupArray[species].X_RS[cohort], bm->flag_repcostSpawn, llogfp);
                step2 += (FunctGroupArray[species].scaled_FSPB[cohort] * step3 * MIGRATION[species].DEN[cohort][expect_id]);
                
                //spawn_abund += FunctGroupArray[species].scaled_FSPB[cohort] * (DNA[species].num[cohort] + MIGRATION[species].DEN[cohort][expect_id]);  // Use this if only basing relative weighting on number of reproducing adults
                spawn_abund += step2;  // Use this if basing weighting on total spawn contributed
				DNA[species].scaled_change[ntrait] += DNA[species].trait[ntrait][base_chrt][evol_relvaldiff_id] * step2;
                
                /*
                fprintf(llogfp,"SHIFT %s Time: %e trait: %d cohort: %d step1: %Le step2: %Le step3: %Le num: %e scaled_FSPB: %e MIGden: %e spawn_abund: %Le evol_relvaldiff: %Le scaled_change: %Le\n",
                        FunctGroupArray[species].groupCode, bm->dayt, ntrait, cohort, step1, step2, step3, DNA[species].num[cohort], FunctGroupArray[species].scaled_FSPB[cohort], MIGRATION[species].DEN[cohort][expect_id], spawn_abund, DNA[species].trait[ntrait][base_chrt][evol_relvaldiff_id], DNA[species].scaled_change[ntrait]);
                */
            }
            DNA[species].scaled_change[ntrait] /= (spawn_abund + small_num);

            fprintf(llogfp,"SHIFT %s Time: %e trait: %d scaled_change: %Le spawn_abund: %Le\n",
                    FunctGroupArray[species].groupCode, bm->dayt, ntrait, DNA[species].scaled_change[ntrait], spawn_abund);
            
            if(bm->flag_inheritance) { // Using Asta method
                step1 = 0.0;
                for (cohort = 0; cohort < FunctGroupArray[species].numGeneTypes; cohort++) {
                    orig_value = Get_Trait_value(bm, species, cohort, ntrait, llogfp);
                    old_value = Get_Mean_Value(bm, species, 0, ntrait, llogfp);
                    
                    val_diff = (orig_value - old_value) * (orig_value - old_value);
                    step1 += (val_diff / (ngene - 1.0));
                    
                    fprintf(llogfp,"SHIFT %s Time: %e trait: %d cohort: %d step1: %Le val_diff: %Le old_value: %Le orig_value: %Le ngene: %e\n",
                            FunctGroupArray[species].groupCode, bm->dayt, ntrait, cohort, step1, val_diff, old_value, orig_value, ngene);
                    
                }
                stdev = sqrt(step1);
                orig_value = Get_Mean_Value(bm, species, 0, ntrait, llogfp);

                heritability = FunctGroupArray[species].speciesParams[inheritance_id];
                step2 = heritability * DNA[species].scaled_change[ntrait];
                
                larval_mean = orig_value + step2 * stdev;
                DNA[species].trait_shift[ntrait][0][0][evol_birth_shift_id] = larval_mean / (DNA[species].trait[ntrait][0][evol_theoretmean_id] + small_num); // Make this a scalar as have to scale trait parameters when do aging
                DNA[species].trait[ntrait][0][evol_mean_id] = larval_mean;
                
                fprintf(llogfp,"SHIFT %s Time: %e trait: %d evol_birth_shift: %Le larval_mean: %Le stdev: %Le step1: %Le step2: %Le orig_value: %Le heritability: %Le scaled_change: %Le\n",
                        FunctGroupArray[species].groupCode, bm->dayt, ntrait, DNA[species].trait_shift[ntrait][0][0][evol_birth_shift_id], larval_mean, stdev, step1, step2, orig_value, heritability, DNA[species].scaled_change[ntrait]);

            } else {
                larval_mean = 0.0;
                for (cohort = startage; cohort < FunctGroupArray[species].numCohortsXnumGenes; cohort++) {
                    base_chrt = cohort / FunctGroupArray[species].numGeneTypes;
                    larval_mean += DNA[species].trait[ntrait][base_chrt][evol_mean_id] * DNA[species].num[cohort];
                }
                larval_mean /= (tot_abund + small_num);
            }
            DNA[species].trait[ntrait][0][evol_mean_id] = larval_mean; // Assume variance unchanged
            
		}
	}
	return;
}

/**
 *	\brief Do inheritance - during reproduction find offspring per genetype (based on traits of spawning adults)
 *
 *	Assume heritability of x (proportion of change transmitted as real genetic change),
 *	offspring then the shift in the mean of the starting theoretical distribution
 *	of offspring trait = x * std_dev_representation_of_size_differential.
 *	Need to do this calculation per age class reproducing and then group offspring
 *	based on final trait value to get new numbers per genetic_type-age-bin of the recruits
 *
 */
int Do_Inheritance(MSEBoxModel *bm, int species, int ngene, int basecohort, int cohort, FILE *llogfp) {
	int ntrait, i;
	int recieving_gene = 0, closest_match = 0, ng = 0;
	int age_classes = FunctGroupArray[species].ageClassSize;
	double num_age_classes = (double)(FunctGroupArray[species].ageClassSize);
	double mean_param_val = 0.0;
	double std_dev_param = 0.0;
	double larval_param_val, larval_std_dev, stdev_map, test_val, closest_val, evol_scalar;
	int avg_ng = 0;
	int num_active_traits = 0;

	if((!FunctGroupArray[species].speciesParams[flag_id]) || (!bm->flag_do_evolution) || (FunctGroupArray[species].numGeneTypes < 2))
		return ngene;  // As not active or evolution not active
    
    if(bm->flag_inheritance) {
        /* Either simple transference or case where assume the same distribution through time (whcih dealt with later) */
        return ngene;
    } else {
        closest_match = 0;
        for (ntrait = 0; ntrait < K_num_traits; ntrait++) {
            if(DNA[species].trait_active[ntrait] < 1)
                continue;  // As not active so don't do anything

            // Find the mean shift in this cohort
            mean_param_val = 0.0;
            std_dev_param = 0.0;
            for(i=0; i<age_classes; i++ ){
                mean_param_val += DNA[species].trait_shift[ntrait][basecohort][i][evol_birth_shift_id];
                std_dev_param += DNA[species].trait_shift[ntrait][basecohort][i][evol_birth_var_id];
            }
            mean_param_val = mean_param_val / num_age_classes;
            std_dev_param = std_dev_param / num_age_classes;
            DNA[species].trait_shift[ntrait][0][0][evol_parent_var_id] = std_dev_param;      // Is this just being over written by final age class, do we need to make this an average across basecohorts?

            // Match it to the larval distributions
            larval_param_val = DNA[species].trait_shift[ntrait][0][0][evol_birth_shift_id];
            larval_std_dev = DNA[species].trait_shift[ntrait][0][0][evol_birth_var_id];  // not sqrt() as stdev actually stored in Do_Evolution()

            stdev_map = DNA[species].stdev_gene_map[ngene];
            test_val = mean_param_val + larval_std_dev * std_dev_param;
            // Don't multiply by mean_val as only looking at relative not absolute shifts (i.e. abs_val = mean_val * rel_shift and ony working on rel_shift)
            closest_match = -1;
            closest_val = MAXDOUBLE;

            for(ng=0; ng < FunctGroupArray[species].numGeneTypes; ng++){
                stdev_map = DNA[species].stdev_gene_map[ng];
                evol_scalar = larval_param_val + larval_std_dev * stdev_map;
                // Don't multiply by mean_val as only looking at relative not absolute shifts (i.e. abs_val = mean_val * rel_shift and ony working on rel_shift)
                if (closest_val > (fabs(test_val - evol_scalar))) {
                    closest_val = (fabs(test_val - evol_scalar));
                    closest_match = ng;
                }
            }
            avg_ng += closest_match;
            num_active_traits++;
        }

        // Get average match and pass it back
        if (!num_active_traits) {
            recieving_gene = ngene;
        } else {
            recieving_gene = avg_ng / num_active_traits;
        }

        // Sanity checks
        if(recieving_gene > (FunctGroupArray[species].numGeneTypes - 1))
            recieving_gene = FunctGroupArray[species].numGeneTypes - 1;
        if(recieving_gene < 0)
            recieving_gene = 0;

        return recieving_gene;  // TODO: rather than hand off all babies to one gene type allow for a distribution
    }

}

/**
 *	\brief Evolution_Curve
 *
 *	Impose a Gaussian normal curve on the number of offspring per geneotype 
 * (distributed symmetricall around mid-point, which is assumed to equate to mean trait value)
 *
 */
void Evolution_Curve(MSEBoxModel *bm, int species, int stock_id, int qid, int do_debug, FILE *llogfp) {
    double TotEmbryoes = 0.0;
    int ng;
    
    if(bm->flag_inheritance != simple_inheritance)
        return;  // As not being used
    
    if((bm->current_box != bm->last_box) || (bm->current_layer > 0))
        return; // As only have to do it once
        
        
    /* From inheritance observations know that the std-dev shape curve doesn't shift generation to generation,
     only the mean shifts. So instead of doing closest match (like below option) just assume the same distribution as for
     initialisation so ngene -> ngene
     */
    for(ng=0; ng < FunctGroupArray[species].numGeneTypes; ng++){
        TotEmbryoes += EMBRYO[species].Larvae[stock_id][ng][qid];

        //fprintf(llogfp,"CURVE %s Time: %e ng: %d TotEmbryoes: %e Larve: %e\n", FunctGroupArray[species].groupCode, bm->dayt, ng, TotEmbryoes, EMBRYO[species].Larvae[stock_id][ng][qid]);
        
    }
    
    /* Assume standard nomal curve 
     1-SD: +/- 34%
     2-SD: additional +/- 13.5%
     3-SD: additional +/- 2%
     4-SD: additional +/- 0.5%

     Divide numGeneTypes over the distribution and fit Guassian curve across them
     */
    
    // TODO: Make this nicer, hard coded hack for now
        
    switch (FunctGroupArray[species].numGeneTypes) {
        case 0:
        case 1:
            EMBRYO[species].Larvae[stock_id][0][qid] = TotEmbryoes;
            break;
        case 2:
            EMBRYO[species].Larvae[stock_id][0][qid] = TotEmbryoes * 0.5;
            EMBRYO[species].Larvae[stock_id][1][qid] = TotEmbryoes * 0.5;
            break;
        case 3:
            EMBRYO[species].Larvae[stock_id][0][qid] = TotEmbryoes * 0.16;
            EMBRYO[species].Larvae[stock_id][1][qid] = TotEmbryoes * 0.68;
            EMBRYO[species].Larvae[stock_id][2][qid] = TotEmbryoes * 0.16;
            break;
        case 4:
            EMBRYO[species].Larvae[stock_id][0][qid] = TotEmbryoes * 0.16;
            EMBRYO[species].Larvae[stock_id][1][qid] = TotEmbryoes * 0.34;
            EMBRYO[species].Larvae[stock_id][2][qid] = TotEmbryoes * 0.34;
            EMBRYO[species].Larvae[stock_id][3][qid] = TotEmbryoes * 0.16;
            break;
        case 5:
            /* Old version
            EMBRYO[species].Larvae[stock_id][0][qid] = TotEmbryoes * 0.025;
            EMBRYO[species].Larvae[stock_id][1][qid] = TotEmbryoes * 0.135;
            EMBRYO[species].Larvae[stock_id][2][qid] = TotEmbryoes * 0.68;
            EMBRYO[species].Larvae[stock_id][3][qid] = TotEmbryoes * 0.135;
            EMBRYO[species].Larvae[stock_id][4][qid] = TotEmbryoes * 0.025;
             
            Asta version
            */
            EMBRYO[species].Larvae[stock_id][0][qid] = TotEmbryoes * 0.08;
            EMBRYO[species].Larvae[stock_id][1][qid] = TotEmbryoes * 0.2;
            EMBRYO[species].Larvae[stock_id][2][qid] = TotEmbryoes * 0.44;
            EMBRYO[species].Larvae[stock_id][3][qid] = TotEmbryoes * 0.2;
            EMBRYO[species].Larvae[stock_id][4][qid] = TotEmbryoes * 0.08;
            break;
        case 6:
            EMBRYO[species].Larvae[stock_id][0][qid] = TotEmbryoes * 0.025;
            EMBRYO[species].Larvae[stock_id][1][qid] = TotEmbryoes * 0.135;
            EMBRYO[species].Larvae[stock_id][2][qid] = TotEmbryoes * 0.34;
            EMBRYO[species].Larvae[stock_id][3][qid] = TotEmbryoes * 0.34;
            EMBRYO[species].Larvae[stock_id][4][qid] = TotEmbryoes * 0.135;
            EMBRYO[species].Larvae[stock_id][5][qid] = TotEmbryoes * 0.025;
            break;
        case 7:
            EMBRYO[species].Larvae[stock_id][0][qid] = TotEmbryoes * 0.005;
            EMBRYO[species].Larvae[stock_id][1][qid] = TotEmbryoes * 0.02;
            EMBRYO[species].Larvae[stock_id][2][qid] = TotEmbryoes * 0.135;
            EMBRYO[species].Larvae[stock_id][3][qid] = TotEmbryoes * 0.68;
            EMBRYO[species].Larvae[stock_id][4][qid] = TotEmbryoes * 0.135;
            EMBRYO[species].Larvae[stock_id][5][qid] = TotEmbryoes * 0.02;
            EMBRYO[species].Larvae[stock_id][6][qid] = TotEmbryoes * 0.005;
            break;
        case 8:
            EMBRYO[species].Larvae[stock_id][0][qid] = TotEmbryoes * 0.005;
            EMBRYO[species].Larvae[stock_id][1][qid] = TotEmbryoes * 0.02;
            EMBRYO[species].Larvae[stock_id][2][qid] = TotEmbryoes * 0.135;
            EMBRYO[species].Larvae[stock_id][3][qid] = TotEmbryoes * 0.34;
            EMBRYO[species].Larvae[stock_id][4][qid] = TotEmbryoes * 0.34;
            EMBRYO[species].Larvae[stock_id][5][qid] = TotEmbryoes * 0.135;
            EMBRYO[species].Larvae[stock_id][6][qid] = TotEmbryoes * 0.02;
            EMBRYO[species].Larvae[stock_id][7][qid] = TotEmbryoes * 0.005;
            break;
        case 9:
            EMBRYO[species].Larvae[stock_id][0][qid] = TotEmbryoes * 0.001;
            EMBRYO[species].Larvae[stock_id][1][qid] = TotEmbryoes * 0.004;
            EMBRYO[species].Larvae[stock_id][2][qid] = TotEmbryoes * 0.02;
            EMBRYO[species].Larvae[stock_id][3][qid] = TotEmbryoes * 0.135;
            EMBRYO[species].Larvae[stock_id][4][qid] = TotEmbryoes * 0.68;
            EMBRYO[species].Larvae[stock_id][5][qid] = TotEmbryoes * 0.135;
            EMBRYO[species].Larvae[stock_id][6][qid] = TotEmbryoes * 0.02;
            EMBRYO[species].Larvae[stock_id][7][qid] = TotEmbryoes * 0.004;
            EMBRYO[species].Larvae[stock_id][8][qid] = TotEmbryoes * 0.001;
            break;
        case 10:
            EMBRYO[species].Larvae[stock_id][0][qid] = TotEmbryoes * 0.001;
            EMBRYO[species].Larvae[stock_id][1][qid] = TotEmbryoes * 0.004;
            EMBRYO[species].Larvae[stock_id][2][qid] = TotEmbryoes * 0.02;
            EMBRYO[species].Larvae[stock_id][3][qid] = TotEmbryoes * 0.135;
            EMBRYO[species].Larvae[stock_id][4][qid] = TotEmbryoes * 0.34;
            EMBRYO[species].Larvae[stock_id][5][qid] = TotEmbryoes * 0.34;
            EMBRYO[species].Larvae[stock_id][6][qid] = TotEmbryoes * 0.135;
            EMBRYO[species].Larvae[stock_id][7][qid] = TotEmbryoes * 0.02;
            EMBRYO[species].Larvae[stock_id][8][qid] = TotEmbryoes * 0.004;
            EMBRYO[species].Larvae[stock_id][9][qid] = TotEmbryoes * 0.001;
            break;
        default:
            quit("This inheritance case for numGeneTypes %d\n", FunctGroupArray[species].numGeneTypes);
            break;
    
    }
 
    /*
    for(ng=0; ng < FunctGroupArray[species].numGeneTypes; ng++){
        fprintf(llogfp,"CURVE %s Time: %e ng: %d updated Larvae: %e\n",
                FunctGroupArray[species].groupCode, bm->dayt, ng, EMBRYO[species].Larvae[stock_id][ng][qid]);
        
    }
    */
    
}

/**
 *	\brief Do random gene expression
 *	To allow for plasticity in expression of the genetics
 *	when age then take a random % < y% (y defined by user)
 *	and shift it to neighbouring bins (smaller or larger)
 *
 *	Here just calculate the multiplier to apply in the aging routines
 */
void Do_Phenotyic_Expression(MSEBoxModel *bm, int species, FILE *llogfp) {
	double p_ageup, dennow, num_here, scalar;
	double max_prop_shift = FunctGroupArray[species].speciesParams[max_prop_shift_id];
	int cohort, ngene, basecohort, ng;
	int sp_numGeneTypes = (int) (FunctGroupArray[species].numGeneTypes);
	int i = FunctGroupArray[species].ageClassSize - 1;
	if (i < 0)
		i = 0;
    
	if (bm->flag_do_evolution) {  // So want full evolutionary change and phenotype expression variance
		if (!bm->flagagestruct) {
			// There should be a warning that this is a potentially non-sensical combination during start-up
		} else {

			/* Populate the matrix phenotype_transition which is the number of each age-gene bin flowing to
			 * each gene type of next age class (don't need all age-gene bin x all age-gene bin as can't skip
			 * ages and can't age backwards so only map to ngene possibilities
			 */
			for (cohort = 0; cohort < FunctGroupArray[species].numCohortsXnumGenes; cohort++){
				for (ngene = 0; ngene < FunctGroupArray[species].numGeneTypes; ngene++){
					DNA[species].phenotype_transition[cohort][ngene] = 0.0;
				}
			}

			for (cohort = 0; cohort < FunctGroupArray[species].numCohortsXnumGenes; cohort++){
				basecohort = (int)floor (cohort / sp_numGeneTypes);
				ng = cohort - (basecohort * sp_numGeneTypes);
				p_ageup = FunctGroupArray[species].boxPopRatio[bm->current_box][bm->current_layer][cohort][i];
				dennow = VERTinfo[species][cohort][DEN_id] * p_ageup;
				DNA[species].phenotype_aging_up[cohort] = dennow;  // Store numbers aging out of this age-gene combo
                
                if (!bm->flag_do_var_express) {
                    // All just make a straight aging transition as no variation in phenotypic expression
                    DNA[species].phenotype_transition[cohort][ng] += dennow;
                } else {
                    // Allowing for variable expression - find the gene-age combos they will go to (don't put straight to age+1 as that will happen in Update_Aging_Numbers()
                    for (ngene = 0; ngene < FunctGroupArray[species].numGeneTypes; ngene++){
                        if (ngene != ng) { // work out who wanders away first
                            if (ngene < (FunctGroupArray[species].numGeneTypes - 1))
                                scalar = drandom(0.0, max_prop_shift);  // So get random re-assortment between phenotypes (from each gene type)
                            else
                                scalar = 1.0;
                            num_here = scalar * dennow;
                            DNA[species].phenotype_transition[cohort][ngene] += num_here;
                            dennow -= num_here;
                            if (dennow < 0.0) dennow = 0.0;  // All already allocated to a gene type so no more to sort
                        }
                    }
                    // Those who don't vary from genetic determinism now get passed on
                    DNA[species].phenotype_transition[cohort][ng] += dennow;
                }
			}
		}
	}

	return;
}


/**
 *	\brief Do trait evolution - shift the trait based on where selective pressure is trending
 *	Apply shift in values for youngest age class - will update as age through life
 *
 *  Assume heritability of x (proportion of change transmitted as real genetic change),
 *	offspring then the shift in the mean of the starting theoretical distribution
 *	of offspring trait = x * std_dev_representation_of_size_differential.
 *  Apply the shift to get the value to apply in the calculations
 *
 */
void Do_Evolution(MSEBoxModel *bm, int species, FILE *llogfp) {
	int ntrait, cohort;
	double mean_param_val, std_dev_param, heritability, old_theoret_mean, abs_evol_change, std_dev_parents, new_value;
    long double old_value, orig_value, step1, val_diff, stdev, evol_scalar;
    double ngene = (double)(FunctGroupArray[species].numGeneTypes);

	if((!FunctGroupArray[species].speciesParams[flag_id]) || (!bm->flag_do_evolution) || (FunctGroupArray[species].numGeneTypes < 2))
		return;  // As not active or evolution not active

	if ((bm->current_box == bm->first_box) && (bm->current_layer == bm->top_layer)) {  // As only want to do it once per event
		/* Loop over traits - only first youngest age class so only have to loop through gene groups in that age group */
		for (ntrait = 0; ntrait < K_num_traits; ntrait++) {

			if(DNA[species].trait_active[ntrait] < 1)
				continue;  // As not active so don't do anything

            if(bm->flag_inheritance) {
                // Now do the trait update - take evolutionary mean taken form above to get evol_scalar to apply
                orig_value = Get_Mean_Value(bm, species, 0, ntrait, llogfp);
                evol_scalar = DNA[species].trait[ntrait][0][evol_mean_id] / (orig_value + small_num);

                fprintf(llogfp,"DOEVOL %s Time: %e trait: %d orig_value: %Le evol_scalarA: %Le\n",
                        FunctGroupArray[species].groupCode, bm->dayt, ntrait, orig_value, evol_scalar);
                
                // Check shift not too extreme - otherwise get multiplicative explosion as cohorts die away
                if( evol_scalar > (1.0 + bm->max_rate_evol))
                    evol_scalar = (1.0 + bm->max_rate_evol);
                if( evol_scalar < (1.0 - bm->max_rate_evol))
                    evol_scalar = (1.0 - bm->max_rate_evol);

                fprintf(llogfp,"DOEVOL %s Time: %e trait: %d evol_scalarB: %Le\n",
                        FunctGroupArray[species].groupCode, bm->dayt, ntrait, evol_scalar);
                
                for (cohort = 0; cohort < FunctGroupArray[species].numGeneTypes; cohort++) {
                    Apply_Evol_Scalar(bm, species, cohort, ntrait, evol_scalar, llogfp);
                    
                    new_value = Get_Trait_value(bm, species, cohort, ntrait, llogfp);
                    
                    fprintf(llogfp,"DOEVOL %s-%d Time: %e trait: %d new_value: %e\n",
                            FunctGroupArray[species].groupCode, cohort, bm->dayt, ntrait, new_value);
                }
                
                // Find new stdev
                step1 = 0.0;
                for (cohort = 0; cohort < FunctGroupArray[species].numGeneTypes; cohort++) {
                    orig_value = DNA[species].trait[ntrait][0][evol_mean_id];
                    old_value = Get_Trait_value(bm, species, cohort, ntrait, llogfp);
                    val_diff = (orig_value - old_value) * (orig_value - old_value);
                    step1 += (val_diff / (ngene - 1.0));
                    
                    fprintf(llogfp,"DOEVOL %s Time: %e trait: %d cohort %d orig_value: %Le old_value: %Le val_diff: %Le step1: %Le evol_mean: %Le ngene: %e\n",
                            FunctGroupArray[species].groupCode, bm->dayt, ntrait, cohort, orig_value, old_value, val_diff, step1, DNA[species].trait[ntrait][0][evol_mean_id], ngene);

                }
                stdev = sqrt(step1);
                DNA[species].trait[ntrait][0][evol_var_id] = step1;
                DNA[species].trait_shift[ntrait][0][0][evol_birth_var_id] = stdev;  // not sqrt() as stdev assumed to be stored

                
                fprintf(llogfp,"DOEVOL %s Time: %e trait: %d evol_birth_shift: %Le stdev: %Le step1: %Le\n",
                        FunctGroupArray[species].groupCode, bm->dayt, ntrait, DNA[species].trait_shift[ntrait][0][0][evol_birth_var_id], stdev, step1);
                
            } else {
                /** More complicated form of shift to allow for diversity turnover **/
            
                /* Find final scalar for this genotype
                 * For this want to have old_theoret_mean + shift * heritability + normal_variation
                 * Use the relationship
                 *
                 * abs_evol_change = (((new_mean - old_theoret_mean) / std_dev_parents) * heritability) * std_dev_larvae
                 *
                 * then
                 * evol_scalar = abs_evol_change / old_theoret_mean
                 *
                 * Don't multiply by absolute mean_val as getting rel_val shift as evol_scalar
                 * then multiplied by property_val to get end result
                 *
                 */
                /* Start with shift in mean (make it proportional so can do shifts as go up age classes)
                 * and store it in the youngest age class of the youngest age bin
                 */
                mean_param_val = DNA[species].trait[ntrait][0][evol_mean_id];
                heritability = FunctGroupArray[species].speciesParams[inheritance_id];
                old_theoret_mean = DNA[species].trait[ntrait][0][evol_theoretmean_id];
                std_dev_param = sqrt(DNA[species].trait[ntrait][0][evol_var_id]);
                std_dev_parents = DNA[species].trait_shift[ntrait][0][0][evol_parent_var_id];
                
                DNA[species].trait_shift[ntrait][0][0][evol_birth_var_id] = std_dev_param;
                DNA[species].trait_shift[ntrait][0][0][evol_birth_shift_id] = mean_param_val / DNA[species].trait[ntrait][0][evol_theoretmean_id];
                
                abs_evol_change = (((mean_param_val - old_theoret_mean) / std_dev_parents) * heritability) * std_dev_param;

                for (cohort = 0; cohort < FunctGroupArray[species].numGeneTypes; cohort++) {
                    /* Get evolutionary scalar for inheritance and trait shifts */
                    //stdev_map = DNA[species].stdev_gene_map[ngene];
                    //evol_scalar =  1.0 + inherited_shift * mean_shift + std_dev_param * stdev_map;

                    evol_scalar = 1.0 + abs_evol_change / old_theoret_mean;

                    // Check shift not too extreme
                    if( evol_scalar > (1.0 + bm->max_rate_evol))
                        evol_scalar = (1.0 + bm->max_rate_evol);
                    if( evol_scalar < (1.0 - bm->max_rate_evol))
                        evol_scalar = (1.0 - bm->max_rate_evol);

                    Apply_Evol_Scalar(bm, species, cohort, ntrait, evol_scalar, llogfp);
                }
			}
		}
	}
}


/**
 *	\brief Take shift in trait with species through aging - so not just youngest age class shifts
 *
 *  Shift in the mean of the starting theoretical distribution
 *	of offspring trait = x * std_dev_representation_of_size_differential.
 *  Apply the shift to get the value to apply in the calculations
 */
void Do_Aging_Update_Trait(MSEBoxModel *bm, int species, FILE *llogfp) {
	int cohort, basecohort, ntrait, b, k, last_cohort, den, last_den;
	long double evol_scalar, p_ageup, current_val, val_shift, new_val;
    double existing_value, new_value;
	int sp_numGeneTypes = (int) (FunctGroupArray[species].numGeneTypes);
	int i = FunctGroupArray[species].ageClassSize - 1;
	
    if (i < 0)
		i = 0;

	if((!FunctGroupArray[species].speciesParams[flag_id]) || (!bm->flag_do_evolution) || (FunctGroupArray[species].numGeneTypes < 2))
		return;  // As not active or evolution not active

    if (verbose) {
        printf("Creating Aging trait arrays\n");
    }

    Util_Init_1D_Double(numbers_entering, bm->K_num_max_cohort * bm->K_num_max_genetypes, 0.0);
    Util_Init_1D_Double(numbers_already_present, bm->K_num_max_cohort * bm->K_num_max_genetypes, 0.0);
    
	if ((bm->current_box == bm->first_box) && (bm->current_layer == bm->top_layer)) { // As only want to do it once per event
		// Find relative weightings based on numbers in cohort and aging into it - so can do weighted average for the property

		for (cohort = ((FunctGroupArray[species].numCohortsXnumGenes) - 1); cohort >= 0; cohort--){
			last_cohort = cohort - sp_numGeneTypes;
			if(last_cohort < 0)
				last_cohort = 0;
			last_den = FunctGroupArray[species].NumsTracers[last_cohort];
			den = FunctGroupArray[species].NumsTracers[cohort];
			for(b=0; b<bm->nbox; b++){
				if(bm->boxes[b].type != BOUNDARY){
					for(k=0; k<bm->boxes[b].nz; k++){
						p_ageup = FunctGroupArray[species].boxPopRatio[b][k][last_cohort][i];
						numbers_entering[cohort] += bm->boxes[b].tr[k][last_den] * p_ageup;
						p_ageup = FunctGroupArray[species].boxPopRatio[b][k][cohort][i];
						numbers_already_present[cohort] += bm->boxes[b].tr[k][den] * (1.0 - p_ageup);
					}
				}
			}
		}

		for (cohort = ((FunctGroupArray[species].numCohortsXnumGenes) - 1); cohort >= 0; cohort--){
			basecohort = (int)floor (cohort / sp_numGeneTypes);
			if (basecohort < 1)
				continue;  // As no need to update the youngest age classes

			// Start with relative shift in mean
			for (ntrait = 0; ntrait < K_num_traits; ntrait++) {

				if(DNA[species].trait_active[ntrait] < 1)
					continue;  // As not active so don't do anything


				DNA[species].trait_shift[ntrait][basecohort][0][evol_birth_shift_id] = DNA[species].trait_shift[ntrait][basecohort-1][i][evol_birth_shift_id];
				DNA[species].trait_shift[ntrait][basecohort][0][evol_birth_var_id] = DNA[species].trait_shift[ntrait][basecohort-1][i][evol_birth_var_id];

				// Find final scalar for this genotype
				val_shift = DNA[species].trait_shift[ntrait][basecohort][0][evol_birth_shift_id];
				current_val = Get_Trait_value(bm, species, cohort, ntrait, llogfp);
				new_val = current_val * val_shift * numbers_entering[cohort] + current_val * numbers_already_present[cohort];
				new_val /= (numbers_entering[cohort] + numbers_already_present[cohort] + small_num);
				evol_scalar = new_val / (current_val + small_num);
				
                /* This check is not needed as just transferring statistics and values not doing actual evolution
				// Check shift not too extreme
				if( evol_scalar > (1.0 + bm->max_rate_evol))
					evol_scalar = (1.0 + bm->max_rate_evol);
				if( evol_scalar < (1.0 - bm->max_rate_evol))
					evol_scalar = (1.0 - bm->max_rate_evol);

                */
                
                existing_value = Get_Trait_value(bm, species, cohort, ntrait, llogfp);
                    
                fprintf(llogfp,"AGING %s Time: %e trait: %d cohort %d new_evol_shift: %Le numbers_entering: %e numbers_already_present: %e val_shift: %Le current_val: %Le new_val: %Le (before normalise: %Le) denom: %e evol_scalar: %Le\n", FunctGroupArray[species].groupCode, bm->dayt, ntrait, cohort, DNA[species].trait_shift[ntrait][basecohort][0][evol_birth_shift_id], numbers_entering[cohort], numbers_already_present[cohort], val_shift, current_val, new_val, (current_val * val_shift * numbers_entering[cohort] + current_val * numbers_already_present[cohort]), (numbers_entering[cohort] + numbers_already_present[cohort] + small_num), evol_scalar);
                
                 
				Apply_Evol_Scalar(bm, species, cohort, ntrait, evol_scalar, llogfp);
                
                new_value = Get_Trait_value(bm, species, cohort, ntrait, llogfp);

                fprintf(llogfp,"AGING %s-%d Time: %e trait: %d existing_value: %e new_value: %e\n",
                        FunctGroupArray[species].groupCode, cohort, bm->dayt, ntrait, existing_value, new_value);
                
                
			}
		}
	}

	return;
}

/**
 *	\brief Apply shift in trait with species through aging or birth
 *
 */
void Apply_Evol_Scalar(MSEBoxModel *bm, int species, int cohort, int ntrait, double evol_scalar, FILE *llogfp) {

	switch (ntrait) {
	case growth_trait_id:  		/* Growth rate */
		switch (FunctGroupArray[species].groupAgeType) {
		case AGE_STRUCTURED:
            
            //fprintf(llogfp, "Time: %e %s-%d updating mum trait from %e by evol_scalar: %e", bm->dayt, FunctGroupArray[species].groupCode, cohort, FunctGroupArray[species].mum[cohort], evol_scalar);

			FunctGroupArray[species].mum[cohort] *= evol_scalar;
                
            //fprintf(llogfp, " mum now %e\n", FunctGroupArray[species].mum[cohort]);
                
			break;
		case BIOMASS:
		case AGE_STRUCTURED_BIOMASS:
			FunctGroupArray[species].mum[cohort] *= evol_scalar;
			break;
		}
		break;
	case consumption_trait_id:
		switch (FunctGroupArray[species].groupAgeType) {
		case AGE_STRUCTURED:
			FunctGroupArray[species].SP_C[cohort] *= evol_scalar;
			break;
		case BIOMASS:
		case AGE_STRUCTURED_BIOMASS:
			FunctGroupArray[species].SP_C[cohort] *= evol_scalar;
			break;
		}
		break;
	case maturation_trait_id: 	/* Size of maturity */
		if(FunctGroupArray[species].speciesParams[min_li_mat_id] > 0) {
			// If using explicit size at maturation parameter - TODO: Will need to make this have a genetic component
			FunctGroupArray[species].speciesParams[min_li_mat_id] *= evol_scalar;
		} else {
			FunctGroupArray[species].scaled_FSPB[cohort] *= evol_scalar;
            
            if (FunctGroupArray[species].scaled_FSPB[cohort] > 1.0)
                FunctGroupArray[species].scaled_FSPB[cohort] = 1.0;
            
            fprintf(bm->logFile, "Time %e DANGER DANGER DANGER How did you get here? %s-%d has scaled_FSPB: %e\n", bm->dayt, FunctGroupArray[species].groupCode, cohort, FunctGroupArray[species].scaled_FSPB[cohort]);
		}
		break;
	case reproduce_trait_id:  	/* Reproductive investment - TODO: Will need to make this have a genetic component */
		FunctGroupArray[species].speciesParams[FSP_id] *= evol_scalar;
		break;
	case temp_trait_id:  		/* Temperature tolerance - will need to cover range and effect */
		// TODO: Need to diversify how these traits work or have co-evolution of traits; and allow options for bounds to expand (i.e. not just one end of window of tolerance to move but both)
		if (bm->flag_bound_change)
			FunctGroupArray[species].speciesParams[max_move_temp_id] *= evol_scalar;
		else
			FunctGroupArray[species].speciesParams[q10_optimal_temp_id] *= evol_scalar;
		break;
	case pH_trait_id:  			/* pH tolerance and level of impact - will need to cover range and effect  */
		// TODO: As with temperature will need to make this have a genetic component and deal with more aspects of the traits
		if (bm->flag_bound_change)
			FunctGroupArray[species].speciesParams[pH_constB_id] *= evol_scalar;
		else
			FunctGroupArray[species].speciesParams[pH_constA_id] *= evol_scalar;
		break;
	case salt_trait_id:			/* Salinity tolerance - will need to cover range and effect  */
		// TODO: As with temperature will need to make this have a genetic component and deal with more aspects of the traits
		if (bm->flag_bound_change)
			FunctGroupArray[species].speciesParams[min_move_salt_id] *= evol_scalar;
		else
			FunctGroupArray[species].speciesParams[salt_correction_id] *= evol_scalar;
		break;
	default:
		quit(" No such trait as yet\n");
		break;
	}
	return;
}

double Get_Trait_value(MSEBoxModel *bm, int species, int cohort, int ntrait, FILE *llogfp) {
	double param_value = 0.0;

	switch (ntrait) {
	case growth_trait_id:  		/* Growth rate */
		switch (FunctGroupArray[species].groupAgeType) {
		case AGE_STRUCTURED:
			param_value = FunctGroupArray[species].mum[cohort];
			break;
		case BIOMASS:
		case AGE_STRUCTURED_BIOMASS:
			param_value = FunctGroupArray[species].mum[cohort];
			break;
		}
		break;
	case consumption_trait_id:
		switch (FunctGroupArray[species].groupAgeType) {
		case AGE_STRUCTURED:
			param_value = FunctGroupArray[species].SP_C[cohort];
			break;
		case BIOMASS:
		case AGE_STRUCTURED_BIOMASS:
			param_value = FunctGroupArray[species].SP_C[cohort];
			break;
		}
		break;
	case maturation_trait_id: 	/* Size of maturity */
		if(FunctGroupArray[species].speciesParams[min_li_mat_id] > 0) {
			// If using explicit size at maturation parameter - TODO: Will need to make this have a genetic component
			param_value = FunctGroupArray[species].speciesParams[min_li_mat_id];
		} else {
			param_value = FunctGroupArray[species].scaled_FSPB[cohort];
		}
		break;
	case reproduce_trait_id:  	/* Reproductive investment - TODO: Will need to make this have a genetic component */
		param_value = FunctGroupArray[species].speciesParams[FSP_id];
		break;
	case temp_trait_id:  		/* Temperature tolerance - will need to cover range and effect */
		// TODO: Need to diversify how these traits work or have co-evolution of traits; and allow options for bounds to expand (i.e. not just one end of window of tolerance to move but both)
		if (bm->flag_bound_change)
			param_value = FunctGroupArray[species].speciesParams[max_move_temp_id];
		else
			param_value = FunctGroupArray[species].speciesParams[q10_optimal_temp_id];
		break;
	case pH_trait_id:  			/* pH tolerance and level of impact - will need to cover range and effect  */
		// TODO: As with temperature will need to make this have a genetic component and deal with more aspects of the traits
		if (bm->flag_bound_change)
			param_value = FunctGroupArray[species].speciesParams[pH_constB_id];
		else
			param_value = FunctGroupArray[species].speciesParams[pH_constA_id];
		break;
	case salt_trait_id:			/* Salinity tolerance - will need to cover range and effect  */
		// TODO: As with temperature will need to make this have a genetic component and deal with more aspects of the traits
		if (bm->flag_bound_change)
			param_value = FunctGroupArray[species].speciesParams[min_move_salt_id];
		else
			param_value = FunctGroupArray[species].speciesParams[salt_correction_id];
		break;
	default:
		quit(" No such trait as yet\n");
		break;
	}

	return param_value;
}

double Get_Mean_Value(MSEBoxModel *bm, int species, int age_class, int ntrait, FILE *llogfp) {
    double param_value = 0.0;
    int cohort;
    int startage = age_class;
    int endage = age_class + FunctGroupArray[species].numGeneTypes;
    double ngene = (double)(FunctGroupArray[species].numGeneTypes);
    
    double step1 = 0.0;
    for (cohort = startage; cohort < endage; cohort++) {
        step1 += Get_Trait_value(bm, species, cohort, ntrait, llogfp);
    }
    param_value = step1 / ngene;
    
    return param_value;
}

double Get_Orig_Trait_value(MSEBoxModel *bm, int species, int cohort, int ntrait, FILE *llogfp) {
	double param_value = 0.0;
    
    if((ntrait > consumption_trait_id) && bm->newmonth)
        warn("Asking for evolution of trait that doesn't store original param value - likely need code update (ask Beth and Bec about this)\n");
    
    switch (ntrait) {
        case growth_trait_id:  		/* Growth rate */
            switch (FunctGroupArray[species].groupAgeType) {
                case AGE_STRUCTURED:
                    param_value = FunctGroupArray[species].mum_T15[cohort];
                    break;
                case BIOMASS:
                case AGE_STRUCTURED_BIOMASS:
                    param_value = FunctGroupArray[species].mum_T15[cohort];
                    break;
            }
            break;
        case consumption_trait_id:
            switch (FunctGroupArray[species].groupAgeType) {
                case AGE_STRUCTURED:
                    param_value = FunctGroupArray[species].C_T15[cohort];
                    break;
                case BIOMASS:
                case AGE_STRUCTURED_BIOMASS:
                    param_value = FunctGroupArray[species].C_T15[cohort];
                    break;
            }
            break;
        case maturation_trait_id: 	/* Size of maturity */
            if(FunctGroupArray[species].speciesParams[min_li_mat_id] > 0) {
                // If using explicit size at maturation parameter - TODO: Will need to make this have a genetic component
                param_value = FunctGroupArray[species].speciesParams[min_li_mat_id];
            } else {
                param_value = FunctGroupArray[species].scaled_FSPB[cohort];
            }
            break;
        case reproduce_trait_id:  	/* Reproductive investment - TODO: Will need to make this have a genetic component */
            param_value = FunctGroupArray[species].speciesParams[FSP_id];
            break;
        case temp_trait_id:  		/* Temperature tolerance - will need to cover range and effect */
            // TODO: Need to diversify how these traits work or have co-evolution of traits; and allow options for bounds to expand (i.e. not just one end of window of tolerance to move but both)
            if (bm->flag_bound_change)
                param_value = FunctGroupArray[species].speciesParams[max_move_temp_id];
            else
                param_value = FunctGroupArray[species].speciesParams[q10_optimal_temp_id];
            break;
        case pH_trait_id:  			/* pH tolerance and level of impact - will need to cover range and effect  */
            // TODO: As with temperature will need to make this have a genetic component and deal with more aspects of the traits
            if (bm->flag_bound_change)
                param_value = FunctGroupArray[species].speciesParams[pH_constB_id];
            else
                param_value = FunctGroupArray[species].speciesParams[pH_constA_id];
            break;
        case salt_trait_id:			/* Salinity tolerance - will need to cover range and effect  */
            // TODO: As with temperature will need to make this have a genetic component and deal with more aspects of the traits
            if (bm->flag_bound_change)
                param_value = FunctGroupArray[species].speciesParams[min_move_salt_id];
            else
                param_value = FunctGroupArray[species].speciesParams[salt_correction_id];
            break;
        case rsmid_trait_id:
            param_value = bm->evolTraitOriginalValues[species][cohort][rsmid_trait_id];
            break;
        case rsprop_trait_id:
            param_value = bm->evolTraitOriginalValues[species][cohort][rsprop_trait_id];
            break;
        default:
            quit(" No such trait as yet\n");
            break;
	}
    
	return param_value;
}

double Get_Mean_Orig_Value(MSEBoxModel *bm, int species, int age_class, int ntrait, FILE *llogfp) {
    double param_value = 0.0;
    int cohort;
    int startage = age_class;
    int endage = age_class + FunctGroupArray[species].numGeneTypes;
    double ngene = (double)(FunctGroupArray[species].numGeneTypes);

    double step1 = 0.0;
    for (cohort = startage; cohort < endage; cohort++) {
        step1 += Get_Orig_Trait_value(bm, species, cohort, ntrait, llogfp);
    }
    param_value = step1 / ngene;
    
    return param_value;
}

/************************************************************************************************************************************
 * Evolution reporting routines - called from atbiolmessage.c
 */

/**
 *  \brief Create evolution output file
 *
 */
FILE * Init_Evol_File(MSEBoxModel *bm)
{
	FILE *fid;
	char fname[STRLEN];

	/** Create filename **/
	snprintf(fname, sizeof(fname), "%sEvolIndx.txt", bm->startfname);
	printf("Creating %s\n", fname);

    /** Create file **/
    if ( (fid=Util_fopen(bm, fname, "w")) == NULL )
        quit("Init_Evol_File: Can't open %s\n",fname);

    /** Column definitions **/
	fprintf(fid, "Time Species Cohort Trait Mean TheoreticalMean Var\n");

	/* Return file pointer */
	return (fid);
}


/**
 * \brief Write evolution related output
 * Desired output:
 *   Realised size at age distribution
 *   Theoretical size at age distribution
 *   TL
 *
 *   NOTE:
 *   Numbers taken from each genetic_type-age-bin by
 *   predation and fishing recorded separately in the
 *   tracked mortality file
 *
 */
void Write_Evolution(FILE *fid, MSEBoxModel *bm, FILE *llogfp) {
	int sp, cohort, ntrait;
    
	/* Write absolute values */
	for (sp = 0; sp < bm->K_num_tot_sp; sp++) {
        // Update stats just in case
        Find_Evolution_Stats(bm, sp, llogfp, 0);

		for (cohort = 0; cohort < FunctGroupArray[sp].numCohorts; cohort++) {
			for (ntrait = 0; ntrait < K_num_traits; ntrait++) {
				if(DNA[sp].trait_active[ntrait] < 1)  // Don't report if not active
					continue;

				fprintf(fid, "%e %s %d %d %Le %Le %Le\n", bm->dayt, FunctGroupArray[sp].groupCode, cohort, ntrait,
						DNA[sp].trait[ntrait][cohort][evol_mean_id], DNA[sp].trait[ntrait][cohort][evol_theoretmean_id],
						DNA[sp].trait[ntrait][cohort][evol_var_id]);

			}
		}
	}
	return;
}

