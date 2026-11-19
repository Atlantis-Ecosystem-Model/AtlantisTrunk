# atecology/atdiversity.c — bug review

Generated 2026-11-19 from `atlantis/to-do-fixt-list.md`. **9 findings** (severity H: 1; confidence H and severity H: 1; on the priority list: 1), plus 1 lower-confidence notes.

Line numbers were re-mapped on 2026-11-19 to the current code (after the merge of main). Each was traced through git history from the version the review used, then checked against the text of the line. **Lines (current code)** and line numbers in the text are now current; ≈ means the code there has changed so the position is approximate. The review's own numbers are kept as **Lines at review**; a number ending in ? could not be traced and is the review's number. ⚠ means the quoted code is no longer at those lines: it may have changed or already been fixed.

Conf/Sev: confidence it's a bug / severity if it is (H = wrong results or crash, M = wrong in some configs, L = cosmetic, log, leak or latent). ✔ = the reviewer re-checked it by hand. ⭐ = on the high-priority list.

## Summary

| ID | Lines (current) | Conf/Sev | Category | Short description | Decision | Done |
|---|---|---|---|---|---|---|
| [NUTR-08](#nutr-08) ⭐ | 959-1023, 884-957 | H/H | missing switch case → crash | `TRAIT_INDICES` includes `rsmid_trait_id` and `rsprop_trait_id` (atlantisboxmodel.h:2030-2031), which are used… |  | [ ] |
| [NUTR-09](#nutr-09) | 306-307 | H/M | wrong id constant | `MIGRATION[species].SN[cohort][expect_id]`, `.RN[...][expect_id]` and `.DEN[cohort][expect_id]` use `expect_id… |  | [ ] |
| [NUTR-10](#nutr-10) | 1025-1039, 1116-1130 (called at 62) | H/M | index unit confusion | `startage = age_class` and `endage = age_class + numGeneTypes` treat the age class as a flat cohort×gene index… |  | [ ] |
| [NUTR-11](#nutr-11) | 113-122, 253-260 | H/M | loop nesting / overcount | For BIOMASS/AGE_STRUCTURED_BIOMASS EPIFAUNA groups, `bm->boxes[b].epi[den] * area` is added inside the `for (k… |  | [ ] |
| [NUTR-12](#nutr-12) | 61-68 | H/M | overwritten initialisation | Line 63 sets `trait_shift[ntrait][cohort][0][evol_birth_var_id]` to initial variability. The loop at 65-68 the… |  | [ ] |
| [NUTR-13](#nutr-13) | 61-62, 153, 172 | M/M | accumulator not reset | At dayt==0, `evol_theoretmean_id` is first set to `Get_Mean_Orig_Value(...)` (62), then `+= param_value` for e… |  | [ ] |
| [NUTR-14](#nutr-14) | 635-648 | M/M | logic error | When variable expression is on, the last genotype (`ngene == numGeneTypes-1`) gets `scalar = 1.0`. For every `… |  | [ ] |
| [NUTR-15](#nutr-15) | 171 | M/L | division by zero | `trait[..][evol_mean_id] /= DNA[species].tot_num[cohort]` has no guard. An extinct age class gives NaN that sp… |  | [ ] |
| [NUTR-16](#nutr-16) | 756, 758 | M/M | division by zero | `mean_param_val / trait[..][evol_theoretmean_id]` and `/ std_dev_parents` have no small_num guard. `evol_paren… |  | [ ] |

---

## NUTR-08

**missing switch case → crash** · Conf H / Sev H ⭐ priority  
**Lines (current code):** 959-1023, 884-957  
**Lines at review (Oct 2026):** 959-1023, 884-957

**Why it's thought to be a bug**

`TRAIT_INDICES` includes `rsmid_trait_id` and `rsprop_trait_id` (atlantisboxmodel.h:2061-2062), which are used elsewhere (atprocess.c:2516?, atdemography.c:915, and Get_Orig_Trait_value 1102-1107). `Get_Trait_value()` and `Apply_Evol_Scalar()` have no cases for them and fall to `quit(" No such trait as yet")`. Find_Evolution_Stats (151, 184, 199), Find_Evolution_Shift (326), Do_Evolution (702-714) and Do_Aging_Update_Trait (846-867) call these for every active trait, so activating either trait aborts the run. Functions: `Get_Trait_value()`, `Apply_Evol_Scalar()`.

**Current code** (`atecology/atdiversity.c` lines 957–996 (truncated))

```c
  957: }
  958: 
  959: double Get_Trait_value(MSEBoxModel *bm, int species, int cohort, int ntrait, FILE *llogfp) {
  960: 	double param_value = 0.0;
  961: 
  962: 	switch (ntrait) {
  963: 	case growth_trait_id:  		/* Growth rate */
  964: 		switch (FunctGroupArray[species].groupAgeType) {
  965: 		case AGE_STRUCTURED:
  966: 			param_value = FunctGroupArray[species].mum[cohort];
  967: 			break;
  968: 		case BIOMASS:
  969: 		case AGE_STRUCTURED_BIOMASS:
  970: 			param_value = FunctGroupArray[species].mum[cohort];
  971: 			break;
  972: 		}
  973: 		break;
  974: 	case consumption_trait_id:
  975: 		switch (FunctGroupArray[species].groupAgeType) {
  976: 		case AGE_STRUCTURED:
  977: 			param_value = FunctGroupArray[species].SP_C[cohort];
  978: 			break;
  979: 		case BIOMASS:
  980: 		case AGE_STRUCTURED_BIOMASS:
  981: 			param_value = FunctGroupArray[species].SP_C[cohort];
  982: 			break;
  983: 		}
  984: 		break;
  985: 	case maturation_trait_id: 	/* Size of maturity */
  986: 		if(FunctGroupArray[species].speciesParams[min_li_mat_id] > 0) {
  987: 			// If using explicit size at maturation parameter - TODO: Will need to make this have a genetic component
  988: 			param_value = FunctGroupArray[species].speciesParams[min_li_mat_id];
  989: 		} else {
  990: 			param_value = FunctGroupArray[species].scaled_FSPB[cohort];
  991: 		}
  992: 		break;
  993: 	case reproduce_trait_id:  	/* Reproductive investment - TODO: Will need to make this have a genetic component */
  994: 		param_value = FunctGroupArray[species].speciesParams[FSP_id];
  995: 		break;
  996: 	case temp_trait_id:  		/* Temperature tolerance - will need to cover range and effect */
```

**Suggested fix (from review):** Add cases reading and scaling `bm->evolTraitValues[species][cohort][rsmid/rsprop_trait_id]`.

**Your response** (tick one; add detail in comments)
- [ ] A. Agree it's a bug — apply the suggested fix
- [ ] B. Agree it's a bug — but fix it differently (describe below)
- [ ] C. Not a bug / intended behaviour — leave as is
- [ ] D. Unsure — Claude to investigate further and report back
- [ ] E. Real but defer (low priority / later)

**Your comments:**

> 

**Outcome** _(Claude fills this in after the fix: date, change made, commit batch)_:

---

## NUTR-09

**wrong id constant** · Conf H / Sev M  
**Lines (current code):** 306-307  
**Lines at review (Oct 2026):** 306-307

**Function:** `Find_Evolution_Shift()`

**Why it's thought to be a bug**

`MIGRATION[species].SN[cohort][expect_id]`, `.RN[...][expect_id]` and `.DEN[cohort][expect_id]` use `expect_id`, a Black Book economics id (`#define expect_id 0`, atlantisboxmodel.h:1570). The second index of these arrays is the migration-event index qid (allocated `[cohort][maxnum]`, atBiologyXMLParamIO.c:4379-4381, used as `[cohort][qid]` in atmigration.c:770). So only migration event 0 counts toward the external spawn contribution, and the rest are ignored. Function: `Find_Evolution_Shift()`.

**Current code** (`atecology/atdiversity.c` lines 304–309)

```c
  304:                 
  305:                 // Find spawn contribution - external to model domain
  306:                 step3 = Ecology_Age_Structured_Spawn(species, KSPA_sp, FSP_sp, RSprop_sp, MIGRATION[species].SN[cohort][expect_id], MIGRATION[species].RN[cohort][expect_id], FunctGroupArray[species].scaled_FSPB[cohort], FunctGroupArray[species].X_RS[cohort], bm->flag_repcostSpawn, llogfp);
  307:                 step2 += (FunctGroupArray[species].scaled_FSPB[cohort] * step3 * MIGRATION[species].DEN[cohort][expect_id]);
  308:                 
  309:                 //spawn_abund += FunctGroupArray[species].scaled_FSPB[cohort] * (DNA[species].num[cohort] + MIGRATION[species].DEN[cohort][expect_id]);  // Use this if only basing relative weighting on number of reproducing adults
```

**Suggested fix (from review):** Sum over `qid < maxnum` (or the active event) instead of `expect_id`.

**Your response** (tick one; add detail in comments)
- [ ] A. Agree it's a bug — apply the suggested fix
- [ ] B. Agree it's a bug — but fix it differently (describe below)
- [ ] C. Not a bug / intended behaviour — leave as is
- [ ] D. Unsure — Claude to investigate further and report back
- [ ] E. Real but defer (low priority / later)

**Your comments:**

> 

**Outcome** _(Claude fills this in after the fix: date, change made, commit batch)_:

---

## NUTR-10

**index unit confusion** · Conf H / Sev M  
**Lines (current code):** 1025-1039, 1116-1130 (called at 62)  
**Lines at review (Oct 2026):** 1025-1039, 1116-1130 (called at 62)

**Why it's thought to be a bug**

`startage = age_class` and `endage = age_class + numGeneTypes` treat the age class as a flat cohort×gene index. Find_Evolution_Stats line 62 calls `Get_Mean_Orig_Value(bm, species, cohort, ...)` with `cohort` = age class 0..numCohorts-1, so for age class a>0 it averages slots a..a+ngene-1 (mixed ages and genotypes) instead of `a*ngene .. a*ngene+ngene-1`. Calls with 0 are unaffected. Functions: `Get_Mean_Value()`, `Get_Mean_Orig_Value()`.

**Current code** (`atecology/atdiversity.c` lines 1023–1041)

```c
 1023: }
 1024: 
 1025: double Get_Mean_Value(MSEBoxModel *bm, int species, int age_class, int ntrait, FILE *llogfp) {
 1026:     double param_value = 0.0;
 1027:     int cohort;
 1028:     int startage = age_class;
 1029:     int endage = age_class + FunctGroupArray[species].numGeneTypes;
 1030:     double ngene = (double)(FunctGroupArray[species].numGeneTypes);
 1031:     
 1032:     double step1 = 0.0;
 1033:     for (cohort = startage; cohort < endage; cohort++) {
 1034:         step1 += Get_Trait_value(bm, species, cohort, ntrait, llogfp);
 1035:     }
 1036:     param_value = step1 / ngene;
 1037:     
 1038:     return param_value;
 1039: }
 1040: 
 1041: double Get_Orig_Trait_value(MSEBoxModel *bm, int species, int cohort, int ntrait, FILE *llogfp) {
```

**Suggested fix (from review):** `startage = age_class * numGeneTypes; endage = startage + numGeneTypes;`

**Your response** (tick one; add detail in comments)
- [ ] A. Agree it's a bug — apply the suggested fix
- [ ] B. Agree it's a bug — but fix it differently (describe below)
- [ ] C. Not a bug / intended behaviour — leave as is
- [ ] D. Unsure — Claude to investigate further and report back
- [ ] E. Real but defer (low priority / later)

**Your comments:**

> 

**Outcome** _(Claude fills this in after the fix: date, change made, commit batch)_:

---

## NUTR-11

**loop nesting / overcount** · Conf H / Sev M  
**Lines (current code):** 113-122, 253-260  
**Lines at review (Oct 2026):** 113-122, 253-260

**Why it's thought to be a bug**

For BIOMASS/AGE_STRUCTURED_BIOMASS EPIFAUNA groups, `bm->boxes[b].epi[den] * area` is added inside the `for (k < nz)` layer loop, so each box's epibenthic biomass counts nz times. Boxes with more layers get extra weight in tot_num, num and tot_abund. Functions: `Find_Evolution_Stats()`, `Find_Evolution_Shift()`.

**Current code** (`atecology/atdiversity.c` lines 111–124)

```c
  111: 							break;
  112: 						case BIOMASS:				/* Intentional follow through - base on total biomass rather than numbers for these biomass pool groups */
  113: 						case AGE_STRUCTURED_BIOMASS:
  114:                             if (FunctGroupArray[species].habitatType == EPIFAUNA){
  115:                                 DNA[species].tot_num[base_chrt] += bm->boxes[b].epi[den] * bm->boxes[b].area * FunctGroupArray[species].habitatCoeffs[EPIFAUNA];
  116:                                 DNA[species].num[cohort] += bm->boxes[b].epi[den] * bm->boxes[b].area * FunctGroupArray[species].habitatCoeffs[EPIFAUNA];
  117:                                 tot_abund += bm->boxes[b].epi[den] * bm->boxes[b].area * FunctGroupArray[species].habitatCoeffs[EPIFAUNA];
  118:                             } else {
  119:                                 DNA[species].tot_num[base_chrt] += bm->boxes[b].tr[k][den] * bm->boxes[b].dz[k] * bm->boxes[b].area * FunctGroupArray[species].habitatCoeffs[WC];
  120:                                 DNA[species].num[cohort] += bm->boxes[b].tr[k][den] * bm->boxes[b].dz[k] * bm->boxes[b].area * FunctGroupArray[species].habitatCoeffs[WC];
  121:                                 tot_abund += bm->boxes[b].tr[k][den] * bm->boxes[b].dz[k] * bm->boxes[b].area * FunctGroupArray[species].habitatCoeffs[WC];
  122:                             }
  123: 							break;
  124: 						}
```

**Suggested fix (from review):** Move the EPIFAUNA accumulation outside the k loop (or only when k==0).

**Your response** (tick one; add detail in comments)
- [ ] A. Agree it's a bug — apply the suggested fix
- [ ] B. Agree it's a bug — but fix it differently (describe below)
- [ ] C. Not a bug / intended behaviour — leave as is
- [ ] D. Unsure — Claude to investigate further and report back
- [ ] E. Real but defer (low priority / later)

**Your comments:**

> 

**Outcome** _(Claude fills this in after the fix: date, change made, commit batch)_:

---

## NUTR-12

**overwritten initialisation** · Conf H / Sev M  
**Lines (current code):** 61-68  
**Lines at review (Oct 2026):** 61-68

**Function:** `Find_Evolution_Stats()`

**Why it's thought to be a bug**

Line 63 sets `trait_shift[ntrait][cohort][0][evol_birth_var_id]` to initial variability. The loop at 65-68 then sets `evol_birth_var_id = 0.0` for every na, including na=0, which wipes it. Initial birth variability is always 0, which later becomes the `std_dev_parents` divisor in Do_Evolution:758. Function: `Find_Evolution_Stats()`.

**Current code** (`atecology/atdiversity.c` lines 59–70)

```c
   59: 				}
   60:             
   61:                 if(!bm->dayt) { // Initialise the base value at start of the run
   62:                     DNA[species].trait[ntrait][cohort][evol_theoretmean_id] = Get_Mean_Orig_Value(bm, species, cohort, ntrait, llogfp);
   63:                     DNA[species].trait_shift[ntrait][cohort][0][evol_birth_var_id] = FunctGroupArray[species].speciesParams[trait_variance_id] * DNA[species].trait[ntrait][cohort][evol_theoretmean_id];  // Set initial variability levels
   64: 
   65:                     for(na=0; na<age_classes; na++ ){
   66:                         DNA[species].trait_shift[ntrait][cohort][na][evol_birth_shift_id] = 1.0;
   67:                         DNA[species].trait_shift[ntrait][cohort][na][evol_birth_var_id] = 0.0;
   68:                     }
   69: 
   70:                 }
```

**Suggested fix (from review):** Do the `na` loop first, then set line 63's value (or start the loop at na=1).

**Your response** (tick one; add detail in comments)
- [ ] A. Agree it's a bug — apply the suggested fix
- [ ] B. Agree it's a bug — but fix it differently (describe below)
- [ ] C. Not a bug / intended behaviour — leave as is
- [ ] D. Unsure — Claude to investigate further and report back
- [ ] E. Real but defer (low priority / later)

**Your comments:**

> 

**Outcome** _(Claude fills this in after the fix: date, change made, commit batch)_:

---

## NUTR-13

**accumulator not reset** · Conf M / Sev M  
**Lines (current code):** 61-62, 153, 172  
**Lines at review (Oct 2026):** 61-62, 153, 172

**Function:** `Find_Evolution_Stats()`

**Why it's thought to be a bug**

At dayt==0, `evol_theoretmean_id` is first set to `Get_Mean_Orig_Value(...)` (62), then `+= param_value` for each genotype (153), then `/= ngene` (172). The result is (orig_mean + Σvalues)/ngene, which is too high by orig_mean/ngene. Function: `Find_Evolution_Stats()`.

**Current code** (`atecology/atdiversity.c` lines 59–64)

```c
   59: 				}
   60:             
   61:                 if(!bm->dayt) { // Initialise the base value at start of the run
   62:                     DNA[species].trait[ntrait][cohort][evol_theoretmean_id] = Get_Mean_Orig_Value(bm, species, cohort, ntrait, llogfp);
   63:                     DNA[species].trait_shift[ntrait][cohort][0][evol_birth_var_id] = FunctGroupArray[species].speciesParams[trait_variance_id] * DNA[species].trait[ntrait][cohort][evol_theoretmean_id];  // Set initial variability levels
   64: 
```

**Suggested fix (from review):** Store the orig mean in a separate field, or don't seed theoretmean before the accumulation.

**Your response** (tick one; add detail in comments)
- [ ] A. Agree it's a bug — apply the suggested fix
- [ ] B. Agree it's a bug — but fix it differently (describe below)
- [ ] C. Not a bug / intended behaviour — leave as is
- [ ] D. Unsure — Claude to investigate further and report back
- [ ] E. Real but defer (low priority / later)

**Your comments:**

> 

**Outcome** _(Claude fills this in after the fix: date, change made, commit batch)_:

---

## NUTR-14

**logic error** · Conf M / Sev M  
**Lines (current code):** 635-648  
**Lines at review (Oct 2026):** 635-648

**Function:** `Do_Phenotyic_Expression()`

**Why it's thought to be a bug**

When variable expression is on, the last genotype (`ngene == numGeneTypes-1`) gets `scalar = 1.0`. For every `ng != numGeneTypes-1`, all remaining `dennow` goes to the last genotype before reaching line 648, so nothing stays with its own genotype and the last bin soaks up the rest. Only ng == last keeps residuals. Function: `Do_Phenotyic_Expression()`.

**Current code** (`atecology/atdiversity.c` lines 633–650)

```c
  633:                 } else {
  634:                     // Allowing for variable expression - find the gene-age combos they will go to (don't put straight to age+1 as that will happen in Update_Aging_Numbers()
  635:                     for (ngene = 0; ngene < FunctGroupArray[species].numGeneTypes; ngene++){
  636:                         if (ngene != ng) { // work out who wanders away first
  637:                             if (ngene < (FunctGroupArray[species].numGeneTypes - 1))
  638:                                 scalar = drandom(0.0, max_prop_shift);  // So get random re-assortment between phenotypes (from each gene type)
  639:                             else
  640:                                 scalar = 1.0;
  641:                             num_here = scalar * dennow;
  642:                             DNA[species].phenotype_transition[cohort][ngene] += num_here;
  643:                             dennow -= num_here;
  644:                             if (dennow < 0.0) dennow = 0.0;  // All already allocated to a gene type so no more to sort
  645:                         }
  646:                     }
  647:                     // Those who don't vary from genetic determinism now get passed on
  648:                     DNA[species].phenotype_transition[cohort][ng] += dennow;
  649:                 }
  650: 			}
```

**Suggested fix (from review):** Don't use scalar=1 for the last gene; use random shift for all ngene != ng and leave the remainder for ng.

**Your response** (tick one; add detail in comments)
- [ ] A. Agree it's a bug — apply the suggested fix
- [ ] B. Agree it's a bug — but fix it differently (describe below)
- [ ] C. Not a bug / intended behaviour — leave as is
- [ ] D. Unsure — Claude to investigate further and report back
- [ ] E. Real but defer (low priority / later)

**Your comments:**

> 

**Outcome** _(Claude fills this in after the fix: date, change made, commit batch)_:

---

## NUTR-15

**division by zero** · Conf M / Sev L  
**Lines (current code):** 171  
**Lines at review (Oct 2026):** 171

**Function:** `Find_Evolution_Stats()`

**Why it's thought to be a bug**

`trait[..][evol_mean_id] /= DNA[species].tot_num[cohort]` has no guard. An extinct age class gives NaN that spreads into Find_Evolution_Shift (277) and Write_Evolution. Function: `Find_Evolution_Stats()`.

**Current code** (`atecology/atdiversity.c` lines 169–173)

```c
  169: 					continue;  // As not active so don't do anything
  170: 
  171: 				DNA[species].trait[ntrait][cohort][evol_mean_id] /= DNA[species].tot_num[cohort];
  172: 				DNA[species].trait[ntrait][cohort][evol_theoretmean_id] /= ngene;
  173:                 
```

**Suggested fix (from review):** Divide by `(tot_num + small_num)`.

**Your response** (tick one; add detail in comments)
- [ ] A. Agree it's a bug — apply the suggested fix
- [ ] B. Agree it's a bug — but fix it differently (describe below)
- [ ] C. Not a bug / intended behaviour — leave as is
- [ ] D. Unsure — Claude to investigate further and report back
- [ ] E. Real but defer (low priority / later)

**Your comments:**

> 

**Outcome** _(Claude fills this in after the fix: date, change made, commit batch)_:

---

## NUTR-16

**division by zero** · Conf M / Sev M  
**Lines (current code):** 756, 758  
**Lines at review (Oct 2026):** 756, 758

**Function:** `Do_Evolution()`

**Why it's thought to be a bug**

`mean_param_val / trait[..][evol_theoretmean_id]` and `/ std_dev_parents` have no small_num guard. `evol_parent_var_id` is only set in Do_Inheritance (406), and initial birth var is 0 (see NUTR-12), so this can give inf/NaN evol_scalar (the clamp at 768-771 passes NaN through). Function: `Do_Evolution()`.

**Current code** (`atecology/atdiversity.c` lines 754–760)

```c
  754:                 
  755:                 DNA[species].trait_shift[ntrait][0][0][evol_birth_var_id] = std_dev_param;
  756:                 DNA[species].trait_shift[ntrait][0][0][evol_birth_shift_id] = mean_param_val / DNA[species].trait[ntrait][0][evol_theoretmean_id];
  757:                 
  758:                 abs_evol_change = (((mean_param_val - old_theoret_mean) / std_dev_parents) * heritability) * std_dev_param;
  759: 
  760:                 for (cohort = 0; cohort < FunctGroupArray[species].numGeneTypes; cohort++) {
```

**Suggested fix (from review):** Add `+ small_num` to both denominators.

**Your response** (tick one; add detail in comments)
- [ ] A. Agree it's a bug — apply the suggested fix
- [ ] B. Agree it's a bug — but fix it differently (describe below)
- [ ] C. Not a bug / intended behaviour — leave as is
- [ ] D. Unsure — Claude to investigate further and report back
- [ ] E. Real but defer (low priority / later)

**Your comments:**

> 

**Outcome** _(Claude fills this in after the fix: date, change made, commit batch)_:

---

## Lower-confidence notes (speculative — check before acting)

These came from section B.3 of the list. They were not raised as findings. Tick if you want one followed up.

### LC-1

atdiversity.c `Find_Evolution_Stats()` step 2 (185): the variance is taken around `evol_theoretmean_id` rather than `evol_mean_id`. This may be intentional.

- [ ] Investigate   - [ ] Ignore

**Your comments:**

> 
