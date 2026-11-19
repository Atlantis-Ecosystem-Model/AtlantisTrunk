# atecology/atdemography.c — bug review

Generated 2026-11-19 from `atlantis/to-do-fixt-list.md`. **30 findings** (severity H: 4; confidence H and severity H: 3; on the priority list: 3), plus 7 lower-confidence notes.

Line numbers were re-mapped on 2026-11-19 to the current code (after the merge of main). Each was traced through git history from the version the review used, then checked against the text of the line. **Lines (current code)** and line numbers in the text are now current; ≈ means the code there has changed so the position is approximate. The review's own numbers are kept as **Lines at review**; a number ending in ? could not be traced and is the review's number. ⚠ means the quoted code is no longer at those lines: it may have changed or already been fixed.

Conf/Sev: confidence it's a bug / severity if it is (H = wrong results or crash, M = wrong in some configs, L = cosmetic, log, leak or latent). ✔ = the reviewer re-checked it by hand. ⭐ = on the high-priority list.

## Summary

| ID | Lines (current) | Conf/Sev | Category | Short description | Decision | Done |
|---|---|---|---|---|---|---|
| [DEMO-01](#demo-01) ⭐ | 1719-1733 (used 455-458, 1758-1761, 2119-2122, 2153-2156; atmovement.c:1305) | H/H | wrong default / logic | `double KWSR_force = 0.0; if (bm->flag_modify_KWSR && (KWSRforceid > -1)) {...} return KWSR_force;`. When `fla… |  | [ ] |
| [DEMO-02](#demo-02) ⭐ | 2730, 2708, 2738-2772 | H/H | off-by-one / cohort×genotype | `nextcid = cohort + numGeneTypes; if (cohort < num_cohorts)` where `num_cohorts = numCohortsXnumGenes - 1`. Wi… |  | [ ] |
| [DEMO-03](#demo-03) ⭐ | 4452-4459 | H/H | wrong formula | Semelparous case: `spawning_biomass_contrib = this_biom * (1.0 - scaled_FSPB[cohort]);`. In Update_Invert_Agin… |  | [ ] |
| [DEMO-04](#demo-04) | 4341-4345, 4425 | H/M | buffer overrun | `for (stk = 0; stk < maxstock_id + 1; stk++) tot_yoy[species][stk] = 0.0;` and `tot_yoy[species][maxstock_id] … |  | [ ] |
| [DEMO-05](#demo-05) | 3763 | H/M | stale loop variable / OOB | `if (FunctGroupArray[species].agingVERT[cohort][k][DEN_id] > 0)`. Here `k` is left over from the stock loop at… |  | [ ] |
| [DEMO-06](#demo-06) | 3717-3718, 3752-3753 | H/M | cohort×genotype / wrong index | `cid = floor(cohort / numGeneTypes); nextcid = cid + numGeneTypes;` combines an age index with the genotype co… |  | [ ] |
| [DEMO-07](#demo-07) | 4623-4648 (used 4352) | H/M | accumulator overwritten | `MigSpawn = flag_allow_spawn * spawn_active * prop_mat * MIGRATION[species].DEN[cohort][qid];` is assigned, no… |  | [ ] |
| [DEMO-08](#demo-08) | 4638-4640 | M/M | logic | Semelparous branch: `MIGRATION[species].DEN[cohort][qid] *= (1.0 - spawn_active);` ignores both `flag_allow_sp… |  | [ ] |
| [DEMO-09](#demo-09) | 4375-4427, 4884-4955 | H/M | repeated per-cohort call | The recruitment block (`overall_checkday == sp_spawnday + additional_day`) runs inside the per-cohort loop wit… |  | [ ] |
| [DEMO-10](#demo-10) | 4885, 4893 | M/M | shadowed parameter | The `ngene` parameter is immediately overwritten by `for (ngene = 0; ngene < numGeneTypes; ngene++)`. The `spa… |  | [ ] |
| [DEMO-11](#demo-11) | 4301-4321, 4498-4502 | M/M | flag not reset | `sp_Spawn_Now` and `sp_Age_Now` are set to 1 when any cohort matches its Spawn_Now/Age_Now day and are never r… |  | [ ] |
| [DEMO-12](#demo-12) | 1005-1034 (used 469, 517, 2675) | M/H | stale state | `EMBRYO[species].IndSpawn[cohort]` is set only for cohorts that actually spawn. A cohort that is skipped (`Spa… |  | [ ] |
| [DEMO-13](#demo-13) | 922-935 | H/M | flag not reset | `allowed_to_spawn` is set to 0 inside the `mid` loop when one migrant group is too small (`li < min_li_mat`) a… |  | [ ] |
| [DEMO-14](#demo-14) | 1078-1115 | M/M | logic | The death factor `(1 - scaled_FSPB[cohort])` is applied to every cohort with FSPB > 0 whenever any cohort spaw… |  | [ ] |
| [DEMO-15](#demo-15) | 1911-1964 | M/M | OOB index / logic | `mid = migIDmatch[qid]; if (mid < 0) recruit_outside = 0; ... else { MIGRATION[species].recruit[ngene][mid] = … |  | [ ] |
| [DEMO-16](#demo-16) 🔀 | 523-532 | M/M | stale variable | The comment says that if general aging happens here too, this step should be skipped. But `need_update > -1` t… |  | [ ] |
| [DEMO-17](#demo-17) | 3161-3163, 3294-3296 | H/M | stale loop variable | `do_test = 0; if(rec_related) do_test = 1; else if(cohort \|\| (!cohort && k)) do_test = 1;` is evaluated outsid… |  | [ ] |
| [DEMO-18](#demo-18) | 3085-3118 | H/M | flag not reset / wrong index | `stock_done[s_id]` is reset once per call (2992) but is shared across all `cohort` and `qid` iterations. After… |  | [ ] |
| [DEMO-19](#demo-19) | 5258-5267, 5287-5288 | H/M | wrong variable / logic | The loop runs over `sp` but the body uses `species` throughout (`FunctGroupArray[species].isCover`, `.coverID`… |  | [ ] |
| [DEMO-20](#demo-20) | 2068-2074 | H/M | cohort×genotype | `for (n = 0; n < numCohorts; n++) tot_adult_den += scaled_FSPB[n] * tr[k][NumsTracers[n]];`. scaled_FSPB and N… |  | [ ] |
| [DEMO-21](#demo-21) | 2126-2130 | H/L | accumulator overwritten | `tot_yoy[species][i] = (...)` sits inside the `ngene` loop, so only the last genotype's YOY is kept (the migra… |  | [ ] |
| [DEMO-22](#demo-22) | 2241-2258, 2288-2289 | M/M | double count / inconsistent | `if (fished_chrt == ngene) biomass += tr[k][den]; ... biomass += tr[k][den];` counts the fished cohort twice a… |  | [ ] |
| [DEMO-23](#demo-23) | 2554-2585 (used 389-392, 2686) | M/L | uninitialised | In the evolution branch, `*this_p_ageup` is never set. The caller's `this_prop_ageup` (declared without an ini… |  | [ ] |
| [DEMO-24](#demo-24) | 1749-1833 | H/L | dead code / log spam | `direct_recruit_entry` is only ever set to 1, inside the same `if` that sets `recruit_outside = 1` (1775-1780)… |  | [ ] |
| [DEMO-25](#demo-25) 🔀 | 2852-2857, 2876-2878, 2887-2889 | H/L | swapped index / format args | The debug check `bm->tempPopRatio[stock_id][ngene][cohort][0]` swaps dimensions: it should be `[stock_id][spec… |  | [ ] |
| [DEMO-26](#demo-26) | 2653-2657 | H/L | wrong index / log spam | The check `!_finite(bm->tempPopRatio[stock_id][species][cohort][i])` uses `i == 0` after the loop and the wron… |  | [ ] |
| [DEMO-27](#demo-27) | 1617-1620 | H/L | copy-paste message | The warning says "...as flagtempdepend_move = %d" and passes `bm->flagtempdepend_move`, but the guarding flag … |  | [ ] |
| [DEMO-28](#demo-28) | 1648-1652 | L/L | identical branches | `if (sp_q10receff == 1) enviro_scalar *= Tscalar; else if (sp_q10receff == 2) enviro_scalar *= Tscalar;` has i… |  | [ ] |
| [DEMO-29](#demo-29) | 5191 | H/L | log spam | `fprintf(bm->logFile, "species %s, totsum = %e\n", ...)` is unconditional and runs for every cohort, every box… |  | [ ] |
| [DEMO-30](#demo-30) | 4426 | M/L | wrong index | `bm->calcTrackedMort[species][0][stock_id][start_id] += spawned_biomass;` always credits genotype 0. Elsewhere… |  | [ ] |

---

## DEMO-01

**wrong default / logic** · Conf H / Sev H ⭐ priority  
**Lines (current code):** 1719-1733 (used 455-458, 1758-1761, 2119-2122, 2153-2156; atmovement.c:1305)  
**Lines at review (Oct 2026):** 1712-1726 (used 455-458, 1751-1754, 2115-2118, 2149-2152; atmovement.c:1298)

**Function:** `Get_Recruit_Size_Forcing()`

**Why it's thought to be a bug**

`double KWSR_force = 0.0; if (bm->flag_modify_KWSR && (KWSRforceid > -1)) {...} return KWSR_force;`. When `flag_modify_KWSR` is on, every species gets `KWSRforce_id = -1` unless it has a column in the KWSR forcing file (atbiolsetup.c:1076-1081; non-vertebrates always stay -1). For those species the function returns 0, so recruits are given SN = RN = 0 (Vertebrate_Reproduction 468-469, atmovement.c recruits). Function: `Get_Recruit_Size_Forcing()`.

**Current code** (`atecology/atdemography.c` lines 1717–1756 (truncated))

```c
 1717: }
 1718: 
 1719: double Get_Recruit_Size_Forcing(MSEBoxModel *bm, int species, int do_debug, FILE *llogfp) {
 1720:     int KWSRforceid = (int) (FunctGroupArray[species].speciesParams[KWSRforce_id]);
 1721:     double KWSR_sp = FunctGroupArray[species].speciesParams[KWSR_id];
 1722:     double KWSR_force = 0.0;
 1723:     
 1724:     /* Forcing */
 1725:     if (bm->flag_modify_KWSR && (KWSRforceid > -1)) {
 1726:         KWSR_force = tsEval(bm->KWSR_force, KWSRforceid, bm->t);
 1727:         
 1728:         if (bm->flag_modify_KWSR == scale_KWSR)
 1729:             KWSR_force *= KWSR_sp;
 1730:     }
 1731:     
 1732:     return KWSR_force;
 1733: }
 1734: 
 1735: void Recruit_Migration(MSEBoxModel *bm, int species, int wclayer, int stock_id, int do_debug, FILE *llogfp) {
 1736: 
 1737: 	int sp_numGeneTypes = (int) (FunctGroupArray[species].numGeneTypes);
 1738: 	int ngene = 0, mid, lid, recruit_outside, direct_recruit_entry;
 1739: 	int thisday = 0;
 1740:     double oldDEN, oldSN, oldRN, embryo_recruits;
 1741:     //double orig_den;
 1742: 	double KWSR_sp = FunctGroupArray[species].speciesParams[KWSR_id];
 1743: 	double KWRR_sp = FunctGroupArray[species].speciesParams[KWRR_id];
 1744:     int larval_queue_extension = FunctGroupArray[species].speciesParams[larval_queue_extension_id];
 1745:     //int cohort_recruit_entry = FunctGroupArray[species].speciesParams[cohort_recruit_entry_id];
 1746:     
 1747:     //fprintf(bm->logFile, "Time: %e %s box%d-%d (vs first_box %d, top_layer %d) numGeneTypes: %d Recruit_Migration recruitType: %d num_in_queue: %d externalReproducer: %d\n",  bm->dayt, FunctGroupArray[species].groupCode, bm->current_box, bm->current_layer, bm->first_box, bm->top_layer, sp_numGeneTypes, FunctGroupArray[species].recruitType, MIGRATION[species].num_in_queue, FunctGroupArray[species].externalReproducer);
 1748: 
 1749:     if((FunctGroupArray[species].externalReproducer && (FunctGroupArray[species].recruitType != external_spawn_local_recruit)) || (FunctGroupArray[species].recruitType == external_recruit)) {
 1750:         if ((bm->current_box == bm->first_box) && (bm->current_layer == bm->top_layer)) {
 1751: 
 1752:             for(ngene = 0; ngene < sp_numGeneTypes; ngene++){
 1753:                 for (mid = 0; mid < MIGRATION[species].num_in_queue; mid++) {
 1754:                     
 1755:                     fprintf(bm->logFile, "Time: %e %s RecruitQueueMatch: %d\n", bm->dayt, FunctGroupArray[species].groupCode, MIGRATION[species].RecruitQueueMatch[mid]);
 1756:                     
```

**Suggested fix (from review):** Start with `KWSR_force = KWSR_sp` (the unforced value) and only override it when `KWSRforceid > -1`.

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

## DEMO-02

**off-by-one / cohort×genotype** · Conf H / Sev H ⭐ priority  
**Lines (current code):** 2730, 2708, 2738-2772  
**Lines at review (Oct 2026):** 2730, 2708, 2738-2772

**Function:** `Update_Migration_Array()`

**Why it's thought to be a bug**

`nextcid = cohort + numGeneTypes; if (cohort < num_cohorts)` where `num_cohorts = numCohortsXnumGenes - 1`. With numGeneTypes > 1, the cohorts in the oldest age class (e.g. cohort = NxG-2) pass this test, so `nextcid >= numCohortsXnumGenes` and MIGRATION DEN/SN/RN/aging/cohort_migrating`[nextcid]` are read and written past the end (allocated with `cohort` rows, atBiologyXMLParamIO.c:4348-4380). This is reachable when flagsenesce is on, because the oldest cohorts are then not skipped (line 383). Those cohorts also miss the plus-group branch. Update_Aging_Numbers (2669) uses the correct test. Function: `Update_Migration_Array()`.

**Current code** (`atecology/atdemography.c` lines 2728–2767 (truncated))

```c
 2728:     if (spmigrate) {
 2729:         if (FunctGroupArray[species].isVertebrate == TRUE){
 2730:             p_ageup = Get_Proportion_Aging(bm, species, cohort, do_debug, llogfp);  // TODO: Need to allow for these to have evolution act
 2731:         } else {
 2732:             p_ageup = shiftVERT[species][cohort][RN_id]; // p_ageup stored here for biomass pool age structured groups
 2733:         }
 2734: 
 2735:         if (cohort < num_cohorts) {
 2736:             for (qid = MIGRATION[species].num_in_queue_done; qid < MIGRATION[species].num_in_queue; qid++) {
 2737:                 if (FunctGroupArray[species].isVertebrate == TRUE){
 2738:                     p_ageup = Get_Proportion_Aging(bm, species, cohort, do_debug, llogfp);  // TODO: Need to allow for these to have evolution act
 2739:                 } else {
 2740:                     p_ageup = shiftVERT[species][cohort][RN_id]; // p_ageup stored here for biomass pool age structured groups
 2741:                 }
 2742:                 
 2743:                 oldden = MIGRATION[species].DEN[nextcid][qid];
 2744:                 oldSN = MIGRATION[species].SN[nextcid][qid];
 2745:                 oldRN = MIGRATION[species].RN[nextcid][qid];
 2746:                 
 2747:                 newSN = MIGRATION[species].SN[cohort][qid];
 2748:                 newRN = MIGRATION[species].RN[cohort][qid];
 2749:                 num_aging = (p_ageup * MIGRATION[species].DEN[cohort][qid]);
 2750: 
 2751:                 /* Update size
 2752:                  
 2753:                  Start with size scalar - only needed if not using external pop code to update size explicitly
 2754:                  */
 2755:                 sizeScalar = 1.0;
 2756:                 if (FunctGroupArray[species].isExternal == FALSE) {
 2757:                     if(!MIGRATION[species].IsAnnualMigration[qid]) {
 2758:                         growth_period = (double)(MIGRATION[species].MinYearsAway[qid]);
 2759:                         sizeScalar = 1.0 + (MIGRATION[species].growth[qid] / growth_period);
 2760:                     }
 2761:                 }
 2762:                 
 2763:                 MIGRATION[species].SN[nextcid][qid] = (oldden * oldSN + num_aging * newSN * sizeScalar) / (oldden + num_aging + small_num);
 2764:                 MIGRATION[species].RN[nextcid][qid] = (oldden * oldRN + num_aging * newRN * sizeScalar) / (oldden + num_aging + small_num);
 2765:                 
 2766:                 if(bm->track_contaminants){
 2767:                     Age_MigrantContaminants_Update(bm, species, nextcid, cohort, qid, oldden, num_aging);
```

**Suggested fix (from review):** `if (cohort <= (num_cohorts - numGeneTypes))`.

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

## DEMO-03

**wrong formula** · Conf H / Sev H ⭐ priority  
**Lines (current code):** 4452-4459  
**Lines at review (Oct 2026):** 4453-4460

**Function:** `Invertebrate_Reproduction()`

**Why it's thought to be a bug**

Semelparous case: `spawning_biomass_contrib = this_biom * (1.0 - scaled_FSPB[cohort]);`. In Update_Invert_Aging (5110) `spawning_biomass_contrib` is the biomass removed. The comment says the loss should be "all adults spawning". The code removes the immature fraction instead of the mature one, so a fully mature cohort (FSPB = 1) loses nothing and a partly mature cohort loses its non-spawners. This is the vertebrate survivor factor (Apply_Semelparous_Deaths 1098) used as a loss. Function: `Invertebrate_Reproduction()`.

**Current code** (`atecology/atdemography.c` lines 4450–4461)

```c
 4450: 						Update_Spawn_Queue(bm, species, cohort, maxstock_id, do_debug, llogfp);
 4451: 					}
 4452: 				}
 4453: 
 4454: 				/* If semelparous reproduction - update biomass to be lost (i.e. all adults spawning rather than just spawn produced)
 4455: 				 * TODO: May need to distinguish when only external stocks reproduce not internal stocks, in which case don't need to
 4456: 				 * do this step as already sorted in Ecology_Do_External_InVert_Spawning()
 4457: 				 */
 4458: 				if (spawning_biomass && (flagmother < 0)) {
 4459: 					spawning_biomass_contrib = this_biom * (1.0 - FunctGroupArray[species].scaled_FSPB[cohort]);
 4460: 				} else if (spawning_biomass) {
 4461: 					spawning_biomass_contrib = spawning_biomass * prop_biom_spawn_lost;  // This used to be zero, as spawn found now and don't want adult population reduced
```

**Suggested fix (from review):** `this_biom * FunctGroupArray[species].scaled_FSPB[cohort]`.

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

## DEMO-04

**buffer overrun** · Conf H / Sev M  
**Lines (current code):** 4341-4345, 4425  
**Lines at review (Oct 2026):** 4341-4345, 4425

**Function:** `Invertebrate_Reproduction()`

**Why it's thought to be a bug**

`for (stk = 0; stk < maxstock_id + 1; stk++) tot_yoy[species][stk] = 0.0;` and `tot_yoy[species][maxstock_id] += ...`. `tot_yoy` is `Util_Alloc_Init_2D_Double(nstock, K_num_tot_sp)`, i.e. `[sp][K_num_stocks_per_sp]` (atbiolsetup.c:2931, nstock = K_num_stocks_per_sp at 2799). It has no extra "aggregate" slot. When a species has the maximum number of stocks, this writes into the next species' row, or past the end of the heap block for the last species. Function: `Invertebrate_Reproduction()`.

**Current code** (`atecology/atdemography.c` lines 4339–4347)

```c
 4339:                 
 4340: 				if (!sp_Spawn_Now && !sp_Age_Now && !stuff_to_do) {
 4341: 					/* Not spawning or aging now so don't do anything */
 4342: 					continue;
 4343: 				}
 4344: 
 4345: 				/* Initialise YOY sum - including aggregate one stored in maxstock_id slot */
 4346: 				if (sp_Spawn_Now && ((bm->current_box == bm->first_box) && (bm->current_layer == bm->top_layer))) {
 4347: 					for ( stk = 0; stk < maxstock_id + 1; stk++) {
```

**Suggested fix (from review):** Allocate `tot_yoy` with nstock+1 columns, or drop the aggregate slot.

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

## DEMO-05

**stale loop variable / OOB** · Conf H / Sev M  
**Lines (current code):** 3763  
**Lines at review (Oct 2026):** 3763

**Function:** `Ecology_Update_Vertebrate_Cohorts()`

**Why it's thought to be a bug**

`if (FunctGroupArray[species].agingVERT[cohort][k][DEN_id] > 0)`. Here `k` is left over from the stock loop at 3697-3703, so `k == maxstock`. agingVERT is `[ncxg][K_num_stocks_per_sp][3]` (atUtilGroupIO.c:837), so this either reads one row out of bounds or reads an unused stock slot (always 0). As a result, juveniles are never pushed into the MIGRATION array. Function: `Ecology_Update_Vertebrate_Cohorts()`.

**Current code** (`atecology/atdemography.c` lines 3761–3765)

```c
 3761:                                     /**
 3762:                                      if(bm->which_check == species){
 3763:                                         fprintf(llogfp, "Time: %e, Looking at %s-%d with totprop_mig-%d (juv) %e totprop_mig-%d (mat): %e juv_transition_thresh: %e mat_transition_thresh: %e\n", bm->dayt, FunctGroupArray[species].groupCode, cohort, MIGRATION[species].totprop_mig[cohort], MIGRATION[species].totprop_mig[nextcid],bm->juv_transition_thresh,  bm->mat_transition_thresh);
 3764:                                      }
 3765:                                      **/
```

**Suggested fix (from review):** Sum over stocks (or test `tot_new_mat[cohort] > small_num`) before the inner `for (k...)` loop.

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

## DEMO-06

**cohort×genotype / wrong index** · Conf H / Sev M  
**Lines (current code):** 3717-3718, 3752-3753  
**Lines at review (Oct 2026):** 3717-3718, 3752-3753

**Function:** `Ecology_Update_Vertebrate_Cohorts()`

**Why it's thought to be a bug**

`cid = floor(cohort / numGeneTypes); nextcid = cid + numGeneTypes;` combines an age index with the genotype count. The next age class for this cohort is `cohort + numGeneTypes`. The test `(cohort < age_mat) && (nextcid >= age_mat)` also compares the cohort×gene index with age_mat, which is in age classes. Both are only correct when numGeneTypes = 1. With more genotypes, the wrong cohorts trigger the juvenile-to-migration transfer and the wrong box-0 tracers are overwritten (3797-3805). Function: `Ecology_Update_Vertebrate_Cohorts()`.

**Current code** (`atecology/atdemography.c` lines 3715–3754 (truncated))

```c
 3715:                     
 3716:                     
 3717: 					for (cohort = FunctGroupArray[species].numCohortsXnumGenes - 1; cohort > -1 ; cohort--) { // Running backwards so don't need embedded stage check to see different migration status for mig_status (if run forward then get wrong answer for young_in_old_out as not done the later stages when do the younger
 3718: 						finalmig = 0;
 3719: 						avgsn = 0;
 3720: 						avgrn = 0;
 3721: 						MIGRATION[species].totprop_mig[cohort] = 0.0;
 3722: 						cid = (int)(floor(cohort / FunctGroupArray[species].numGeneTypes));
 3723: 						nextcid = cid + FunctGroupArray[species].numGeneTypes;
 3724:                         //stage = FunctGroupArray[species].cohort_stage[cohort];
 3725:                         //next_stage = FunctGroupArray[species].cohort_stage[nextcid];
 3726:                         
 3727:                         // For each migration group?
 3728:                         num_in_MIG_total = 0.0;
 3729: 						for (qid = MIGRATION[species].num_in_queue_done; qid < MIGRATION[species].num_in_queue; qid++) {
 3730:                             num_in_MIG = MIGRATION[species].DEN[cohort][qid];
 3731:                             num_in_MIG_total += MIGRATION[species].DEN[cohort][qid];
 3732:                             
 3733:                             // Does prop_mig[cohort] have to be handled as migration_den[cohort+1] - as put against agingDEN which already cast to [cohort+1]? I don't think so as called form main loop so aging will have happened already on previous iteration of the model
 3734:                             // #TODO: check this assumption re cohort vs cohrot+1
 3735:                             MIGRATION[species].prop_mig[cohort][qid] = num_in_MIG / (totden[species][cohort] + num_in_MIG + small_num);
 3736:                         }
 3737:                         
 3738:                         MIGRATION[species].totprop_mig[cohort] += num_in_MIG_total / (totden[species][cohort] + num_in_MIG_total + small_num);
 3739:                             
 3740:                         if (MIGRATION[species].totprop_mig[cohort] > 1.0)
 3741:                             MIGRATION[species].totprop_mig[cohort] = 1.0;
 3742: 
 3743:                         /* Allocate those who would age-up into an age class outside the model domain
 3744:                         to the Migration array. Only do this if juveniles becoming adults and one away
 3745:                         while the other isn't
 3746:                          
 3747:                          Do allocation of aging into Migration array only when appropriate
 3748:                           - i.e. juveniles in model domain and adults not. If both outside model
 3749:                           domain then code in Update_Migration_Array() will handle it. Also if
 3750:                           juveniles are outside the domain and the adults are inside then this
 3751:                           is dealt with in Update_Migration_Array() too - see logic discussion
 3752:                           in Update_Migration_Array()
 3753:                          
 3754:                          By default mat_transition_thresh should be something high like 0.99 and juv_transition_thresh something lower like 0.1 or 0.5 etc, so this only comes into play when the majority of the adult popualtion is outside the model and juveniles not, but made them variables so more easily adjusted as needed
```

**Suggested fix (from review):** `nextcid = cohort + numGeneTypes;` and test `(cid < age_mat) && (cid + 1 >= age_mat)`.

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

## DEMO-07

**accumulator overwritten** · Conf H / Sev M  
**Lines (current code):** 4623-4648 (used 4352)  
**Lines at review (Oct 2026):** 4624-4649 (used 4353)

**Function:** `Ecology_Do_External_InVert_Spawning()`

**Why it's thought to be a bug**

`MigSpawn = flag_allow_spawn * spawn_active * prop_mat * MIGRATION[species].DEN[cohort][qid];` is assigned, not accumulated, inside the qid loop. Only the last migration-queue entry's spawn is returned, but DEN is still reduced for every entry. Function: `Ecology_Do_External_InVert_Spawning()`.

**Current code** (`atecology/atdemography.c` lines 4621–4650)

```c
 4621: 	return;
 4622: }
 4623: 
 4624: /**
 4625:  *
 4626:  * \brief External invertebrate spawning (occurring by adults outside the model domain)
 4627:  *
 4628:  */
 4629: double Ecology_Do_External_InVert_Spawning(MSEBoxModel *bm, int species, int cohort, int do_debug, FILE *llogfp) {
 4630: 	double MigSpawn = 0.0;
 4631: 	int qid;
 4632: 	int flagmother = (int) (FunctGroupArray[species].speciesParams[flagmother_id]);
 4633: 	double prop_mat = FunctGroupArray[species].scaled_FSPB[cohort];
 4634: 	double spawn_active = FunctGroupArray[species].speciesParams[FSP_id];
 4635:     double flag_allow_spawn = 1.0;
 4636: 
 4637: 	/* If spawn outside model and in first dynamic box then add in spawning due to migrants */
 4638: 	if ((FunctGroupArray[species].externalReproducer) && ((bm->current_box == bm->first_box) && (bm->current_layer == bm->top_layer))) {
 4639: 		for (qid = MIGRATION[species].num_in_queue_done; qid < MIGRATION[species].num_in_queue; qid++) {
 4640:             flag_allow_spawn = (double)(MIGRATION[species].ReprodAllowed[cohort][qid]);
 4641: 			MigSpawn = flag_allow_spawn * spawn_active * prop_mat * MIGRATION[species].DEN[cohort][qid]; // In this case DEN is the total biomass of the migrating group
 4642: 
 4643: 			/* Update biomass left in migrating cohort */
 4644: 			if (flagmother < 0) {
 4645: 				// If semelparous then all that spawned die
 4646: 				MIGRATION[species].DEN[cohort][qid] *= (1.0 - spawn_active);
 4647: 			} else {
 4648: 				// Only matter spawned lost
 4649: 				MIGRATION[species].DEN[cohort][qid] -= MigSpawn;
 4650: 			}
```

**Suggested fix (from review):** Use `MigSpawn +=` with a per-qid temporary for the DEN update.

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

## DEMO-08

**logic** · Conf M / Sev M  
**Lines (current code):** 4638-4640  
**Lines at review (Oct 2026):** 4639-4641

**Function:** `Ecology_Do_External_InVert_Spawning()`

**Why it's thought to be a bug**

Semelparous branch: `MIGRATION[species].DEN[cohort][qid] *= (1.0 - spawn_active);` ignores both `flag_allow_spawn` (ReprodAllowed) and `prop_mat`. Migrants that are not allowed to reproduce, or are immature, are still killed in proportion to FSP. Function: `Ecology_Do_External_InVert_Spawning()`.

**Current code** (`atecology/atdemography.c` lines 4636–4642)

```c
 4636: 
 4637: 	/* If spawn outside model and in first dynamic box then add in spawning due to migrants */
 4638: 	if ((FunctGroupArray[species].externalReproducer) && ((bm->current_box == bm->first_box) && (bm->current_layer == bm->top_layer))) {
 4639: 		for (qid = MIGRATION[species].num_in_queue_done; qid < MIGRATION[species].num_in_queue; qid++) {
 4640:             flag_allow_spawn = (double)(MIGRATION[species].ReprodAllowed[cohort][qid]);
 4641: 			MigSpawn = flag_allow_spawn * spawn_active * prop_mat * MIGRATION[species].DEN[cohort][qid]; // In this case DEN is the total biomass of the migrating group
 4642: 
```

**Suggested fix (from review):** `DEN *= (1.0 - flag_allow_spawn * prop_mat)` (or the intended spawning fraction).

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

## DEMO-09

**repeated per-cohort call** · Conf H / Sev M  
**Lines (current code):** 4375-4427, 4884-4955  
**Lines at review (Oct 2026):** 4376-4428, 4885-4956

**Why it's thought to be a bug**

The recruitment block (`overall_checkday == sp_spawnday + additional_day`) runs inside the per-cohort loop with no cohort guard. `Ecology_Find_Invert_Recruits`, `Find_Final_Invert_Recruit_Distribtuion`, `tot_yoy += spawned_biomass` and `calcTrackedMort[..][start_id] +=` therefore run once per cohort×gene. In Find_Final, `totrecruit += ...` runs on every call, and for `external_recruit` so does `MIGRATION.DEN[ngene][mid] += hab_scalar*spawned_biomass`. Recruits are therefore added numCohortsXnumGenes times (and numGeneTypes more times, see DEMO-10). Functions: `Invertebrate_Reproduction()`, `Find_Final_Invert_Recruit_Distribtuion()`.

**Current code** (`atecology/atdemography.c` lines 4373–4412 (truncated))

```c
 4373: 				}
 4374: 
 4375:                 /*
 4376: 				if (do_debug && (bm->which_check == species))
 4377: 					fprintf(llogfp, "%s-%d box: %d-%d, overall_checkday: %d, sp_spawnday: %d, additional_day: %d\n", FunctGroupArray[species].groupCode, cohort, bm->current_box, bm->current_layer, overall_checkday, sp_spawnday, additional_day);
 4378:                  */
 4379:                 
 4380: 				/* Reset spawning readiness - can't do it at the time due to multiple layer handling */
 4381: 				if (overall_checkday == (sp_spawnday + additional_day)) {
 4382: 					EMBRYO[species].readytospawn[stock_id] = 2; // Reset so ready for next spawning event (now this one is done)
 4383: 
 4384: 					/* Determine population spawning */
 4385: 					Determine_Stock_Prop(bm, species, maxstock_id, 1, &amt, llogfp);
 4386: 
 4387: 					/* Get base level for recruitment */
 4388: 					spawned_biomass = Ecology_Find_Invert_Recruits(bm, species, ngene, stock_id, plankton, llogfp);
 4389: 
 4390:                     /*
 4391: 					if (do_debug && (bm->which_check == species))
 4392: 						fprintf(llogfp, "%s-%d box: %d-%d, spawned_biomass = %e ", FunctGroupArray[species].groupCode, cohort, bm->current_box, bm->current_layer, spawned_biomass);
 4393:                      */
 4394:                     
 4395: 					/* Check for environmental effects */
 4396: 					enviro_scalar = 1.0;
 4397: 					enviro_scalar = Get_Enviro_Recruit_Forcing(bm, species, do_debug, llogfp);
 4398: 					spawned_biomass *= enviro_scalar;
 4399: 
 4400:                     /*
 4401: 					if (do_debug && (bm->which_check == species))
 4402: 						fprintf(llogfp, "enviro_scalar = %e ", enviro_scalar);
 4403:                      */
 4404:                     
 4405: 					/* Check for vertical distribution of new recruits */
 4406: 					if (FunctGroupArray[species].recruitType) {
 4407: 						vertdistrib = 1.0;
 4408: 					} else {
 4409: 						vertdistrib = Get_Vertical_Recruit_Position(species, wclayer, maxdeep, totaldeep);
 4410: 					}
 4411: 					spawned_biomass *= vertdistrib;
 4412: 
```

**Suggested fix (from review):** Do the recruitment block once per gene, e.g. only when `basecohort == 0`.

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

## DEMO-10

**shadowed parameter** · Conf M / Sev M  
**Lines (current code):** 4885, 4893  
**Lines at review (Oct 2026):** 4885, 4893

**Function:** `Find_Final_Invert_Recruit_Distribtuion()`

**Why it's thought to be a bug**

The `ngene` parameter is immediately overwritten by `for (ngene = 0; ngene < numGeneTypes; ngene++)`. The `spawned_biomass` computed for one genotype (4383) is therefore written to every genotype's num_recruits/totrecruit/MIGRATION entries. With numGeneTypes > 1, the last caller's value wins, or values accumulate with `+=`. Function: `Find_Final_Invert_Recruit_Distribtuion()`.

**Current code** (`atecology/atdemography.c` lines 4883–4895)

```c
 4883: 	return;
 4884: }
 4885: 
 4886: /*
 4887:  * \brief Spatial distribution of invertebrate new recruits
 4888:  *
 4889:  */
 4890: void Find_Final_Invert_Recruit_Distribtuion(MSEBoxModel *bm, int species, int ngene, int qid, int stock_id, double spawned_biomass, int wclayer, FILE *llogfp) {
 4891: 	int recruit_outside = 0, cohort = 0, bcohort = 0;
 4892: 	//int qid = EMBRYO[species].latest_in_larval_queue;
 4893:     int mid = 0;
 4894:     //int pid;
 4895: 	double thisday = 0;
```

**Suggested fix (from review):** Remove the inner loop and use the `ngene` argument.

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

## DEMO-11

**flag not reset** · Conf M / Sev M  
**Lines (current code):** 4301-4321, 4498-4502  
**Lines at review (Oct 2026):** 4302-4322, 4499-4503

**Function:** `Invertebrate_Reproduction()`

**Why it's thought to be a bug**

`sp_Spawn_Now` and `sp_Age_Now` are set to 1 when any cohort matches its Spawn_Now/Age_Now day and are never reset per cohort. Every later cohort in the loop is then treated as spawning and aging, so `Get_Proportion_Aging` and aging shifts are applied to cohorts not due to age today. The second loop inherits the value too. Function: `Invertebrate_Reproduction()`.

**Current code** (`atecology/atdemography.c` lines 4299–4323)

```c
 4299: 			spawning_biomass = 0;
 4300: 			spawning_biomass_contrib = 0;
 4301: 			additional_day = 0;
 4302: 			did_something = 0;
 4303: 			p_ageup = 0;
 4304: 			Ecology_Init_Spawn_Params(bm, species, maxstock_id, wclayer, llogfp);
 4305: 
 4306: 			/* Determine if time to spawn or age */
 4307: 			for (cohort = 0; cohort < FunctGroupArray[species].numCohortsXnumGenes; cohort++) {
 4308: 				basecohort = (int)(floor(cohort / FunctGroupArray[species].numGeneTypes));
 4309: 				ngene = cohort - (basecohort * FunctGroupArray[species].numGeneTypes);
 4310: 				this_biom = localWCTracers[FunctGroupArray[species].totNTracers[cohort]];
 4311: 
 4312: 				if (FunctGroupArray[species].recruitType)
 4313: 					additional_day = 0;
 4314: 				else
 4315: 					additional_day = 1;
 4316: 
 4317: 				sp_checkday = EMBRYO[species].Spawn_Now[cohort][EMBRYO[species].next_spawn];
 4318: 				qid = EMBRYO[species].next_spawn;
 4319: 				if (overall_checkday == sp_checkday) {
 4320: 					sp_Spawn_Now = 1;
 4321: 				}
 4322: 				sp_spawnday = sp_checkday;
 4323: 
```

**Suggested fix (from review):** Reset `sp_Spawn_Now = sp_Age_Now = 0` at the top of each cohort iteration, and keep a separate "any" flag for the queue update.

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

## DEMO-12

**stale state** · Conf M / Sev H  
**Lines (current code):** 1005-1034 (used 469, 517, 2675)  
**Lines at review (Oct 2026):** 1004-1033 (used 469, 517, 2671)

**Function:** `Ecology_Do_Internal_Age_Structured_Spawning()`

**Why it's thought to be a bug**

`EMBRYO[species].IndSpawn[cohort]` is set only for cohorts that actually spawn. A cohort that is skipped (`Spawn_Now != today` or `li < min_li_mat`) keeps the IndSpawn value from the previous box on the same spawn day; it is only zeroed on non-spawn days (271-273). Since li depends on the local SN+RN, a cohort can spawn in box A and not in box B. In box B, `Update_Local_dPool` still subtracts the stale IndSpawn from RN (2927), and it also affects aging RN (2675) and recruit RN (469). Function: `Ecology_Do_Internal_Age_Structured_Spawning()`.

**Current code** (`atecology/atdemography.c` lines 1003–1036)

```c
 1003:     *semelparous_parents = 0;
 1004: 
 1005: 	for (cohort = ((FunctGroupArray[species].numCohortsXnumGenes) - 1); cohort >= 0; cohort--) {
 1006: 		basecohort = (int)floor (cohort / sp_numGeneTypes);
 1007: 		ngene = cohort - (basecohort * sp_numGeneTypes);
 1008: 		recieve_ngene = ngene;
 1009: 
 1010: 		if(EMBRYO[species].Spawn_Now[cohort][EMBRYO[species].next_spawn] != overall_checkday)
 1011: 			continue;   // As not the cohort spawning
 1012: 
 1013: 		allowed_to_spawn = 1;
 1014: 
 1015: 		/* for those species with size-based age of maturity, check teh age class is big enough to spawn */
 1016: 		Wgt = VERTinfo[species][cohort][SN_id] + VERTinfo[species][cohort][RN_id];
 1017: 		li = Ecology_Get_Size(bm, species, Wgt, cohort);
 1018: 		if (li < FunctGroupArray[species].speciesParams[min_li_mat_id]) {
 1019: 			allowed_to_spawn = 0;
 1020: 		}
 1021: 
 1022: 		if (!allowed_to_spawn) // If not big enough to spawn skip ahead
 1023: 			continue;
 1024: 
 1025:         // Get dynamically evolving trait values if RSprop is an evolving trait
 1026:         if (bm->flag_do_evolution && DNA[species].trait_active[rsprop_trait_id]) {
 1027:             RSprop_sp = bm->evolTraitValues[species][cohort][rsprop_trait_id]; //NOTE that here cohort refers to the genotype
 1028:         } else {
 1029:             RSprop_sp = FunctGroupArray[species].speciesParams[RSprop_id]; // else just use the stable RSprop value (if there is one)
 1030:         }
 1031: 
 1032:         /* Animals in this box spawn */
 1033: 		step1 = Ecology_Age_Structured_Spawn(species, KSPA_sp, FSP_sp, RSprop_sp, VERTinfo[species][cohort][SN_id], VERTinfo[species][cohort][RN_id], FunctGroupArray[species].scaled_FSPB[cohort], FunctGroupArray[species].X_RS[cohort], bm->flag_repcostSpawn, llogfp);
 1034: 		EMBRYO[species].IndSpawn[cohort] = FunctGroupArray[species].scaled_FSPB[cohort] * step1;
 1035:         
 1036:         thisSSB = (VERTinfo[species][cohort][RN_id] + VERTinfo[species][cohort][SN_id]) * VERTinfo[species][cohort][DEN_id];
```

**Suggested fix (from review):** Set `EMBRYO[species].IndSpawn[cohort] = 0.0` at the top of each cohort iteration, before the `continue`s.

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

## DEMO-13

**flag not reset** · Conf H / Sev M  
**Lines (current code):** 922-935  
**Lines at review (Oct 2026):** 921-934

**Function:** `Ecology_Do_External_Age_Structured_Spawning()`

**Why it's thought to be a bug**

`allowed_to_spawn` is set to 0 inside the `mid` loop when one migrant group is too small (`li < min_li_mat`) and is never reset to 1 for the next `mid`. All later migration-queue entries for that cohort are then skipped. Function: `Ecology_Do_External_Age_Structured_Spawning()`.

**Current code** (`atecology/atdemography.c` lines 920–937)

```c
  920: 
  921:             for (mid = MIGRATION[species].num_in_queue_done; mid < MIGRATION[species].num_in_queue; mid++) {
  922:                 flag_allow_spawn = MIGRATION[species].ReprodAllowed[cohort][mid];
  923:                 if (flag_allow_spawn && (MIGRATION[species].DEN[cohort][mid] > bm->min_dens)) {
  924:                     
  925:                     /* for those species with size-based age of maturity, check the age class is big enough to spawn */
  926:                     Wgt = MIGRATION[species].SN[cohort][mid] + MIGRATION[species].RN[cohort][mid];
  927:                     li = Ecology_Get_Size(bm, species, Wgt, cohort);
  928:                     if (li < FunctGroupArray[species].speciesParams[min_li_mat_id]) {
  929:                         allowed_to_spawn = 0;
  930:                     }
  931:             
  932:                     //fprintf(llogfp, "Time: %e, %s cohort %d allowed_to_spawn B: %d\n", bm->dayt, FunctGroupArray[species].groupCode, cohort,  allowed_to_spawn);
  933: 
  934:                     if (!allowed_to_spawn) // If not big enough to spawn skip ahead
  935:                         continue;
  936:             
  937:                     step1 = Ecology_Age_Structured_Spawn(species, KSPA_sp, FSP_sp, RSprop_sp, MIGRATION[species].SN[cohort][mid], MIGRATION[species].RN[cohort][mid], FunctGroupArray[species].scaled_FSPB[cohort], FunctGroupArray[species].X_RS[cohort], bm->flag_repcostSpawn, llogfp);
```

**Suggested fix (from review):** Use a per-mid local, e.g. `int mid_ok = allowed_to_spawn;` inside the loop.

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

## DEMO-14

**logic** · Conf M / Sev M  
**Lines (current code):** 1078-1115  
**Lines at review (Oct 2026):** 1077-1108

**Function:** `Apply_Semelparous_Deaths()`

**Why it's thought to be a bug**

The death factor `(1 - scaled_FSPB[cohort])` is applied to every cohort with FSPB > 0 whenever any cohort spawned (`semelparous_parents`/`_migrants` are species-level flags). Cohorts whose Spawn_Now was not today, or which failed the size test, still die. It also writes unconditional `fprintf` lines for every cohort, box and spawn event. Function: `Apply_Semelparous_Deaths()`.

**Current code** (`atecology/atdemography.c` lines 1076–1115 (truncated))

```c
 1076:     int cohort, qid;
 1077:     
 1078:     for (cohort = ((FunctGroupArray[species].numCohortsXnumGenes) - 1); cohort >= 0; cohort--) {
 1079:         if(FunctGroupArray[species].scaled_FSPB[cohort] > 0.0) {
 1080:             /* If semelparous external reproduction */
 1081:             if (semelparous_migrants) {
 1082:                 for (qid = MIGRATION[species].num_in_queue_done; qid < MIGRATION[species].num_in_queue; qid++) {
 1083:                     if ((species == 73) || (species == 68) || (species == 66)) {
 1084:                         fprintf(llogfp,"Time: %e %s-%d qid: %d has starting MIGRATION_den_expect: %e", bm->dayt, FunctGroupArray[species].groupCode, cohort, qid,  MIGRATION[species].DEN[cohort][qid]);
 1085:                     }
 1086:                 
 1087:                     MIGRATION[species].DEN[cohort][qid] *= (1.0 - FunctGroupArray[species].scaled_FSPB[cohort]);
 1088:                 
 1089:                     if ((species == 73) || (species == 68) || (species == 66)) {
 1090:                         fprintf(llogfp," ending: %e as scaled_FSPB: %e\n", MIGRATION[species].DEN[cohort][qid], FunctGroupArray[species].scaled_FSPB[cohort]);
 1091:                     }
 1092:                 }
 1093:             }
 1094: 
 1095:     
 1096:             /* If semelparous reproduction internal to the model domain */
 1097:             if (semelparous_parents) {
 1098:                 if ((species == 73) || (species == 68) || (species == 66)) {
 1099:                     fprintf(llogfp,"Time: %e %s-%d current_box: %d has DEN: %e and shiftDEN: %e ", bm->dayt, FunctGroupArray[species].groupCode, cohort, bm->current_box, VERTinfo[species][cohort][DEN_id], shiftVERT[species][cohort][DEN_id]);
 1100:                 }
 1101:                 
 1102:                 /* In case need to do aging correct adult abundance directly */
 1103:                 VERTinfo[species][cohort][DEN_id] *= (1.0 - FunctGroupArray[species].scaled_FSPB[cohort]);
 1104:         
 1105:                 /* If not at same time aging then need to update the shift values - no need to multiply by (1.0 - FunctGroupArray[species].scaled_FSPB[cohort]) here as already applied that above
 1106:                  */
 1107: 
 1108:                  shiftVERT[species][cohort][DEN_id] = VERTinfo[species][cohort][DEN_id];
 1109:                  shiftVERTON[species][cohort]++;
 1110:             
 1111:                 if ((species == 73) || (species == 68) || (species == 66)) {
 1112:                     fprintf(llogfp," DEN ending: %e shift ending: %e as scaled_FSPB: %e\n", VERTinfo[species][cohort][DEN_id], shiftVERT[species][cohort][DEN_id], FunctGroupArray[species].scaled_FSPB[cohort]);
 1113:                 }
 1114: 
 1115:             }
```

**Suggested fix (from review):** Track a per-cohort "spawned" flag (e.g. IndSpawn[cohort] > 0 or per-mid flags) and apply deaths only to those. Guard the fprintf with do_debug.

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

## DEMO-15

**OOB index / logic** · Conf M / Sev M  
**Lines (current code):** 1911-1964  
**Lines at review (Oct 2026):** 1904-1958

**Function:** `Find_Final_Recruit_Distribution()`

**Why it's thought to be a bug**

`mid = migIDmatch[qid]; if (mid < 0) recruit_outside = 0; ... else { MIGRATION[species].recruit[ngene][mid] = 0; ...}`. The else branch runs exactly when `mid < 0`, so it writes `recruit[ngene][-1]` (heap corruption). In that same branch, `totrecruit += ...` (1954) is then wiped by the unconditional `totrecruit[...] = 0.0` (1963). Also, for external_recruit, AlignVsMigration never sets -1 (atannualbiology.c:956-963) and migIDmatch defaults to 0, so a missing match quietly sends recruits to queue entry 0. Function: `Find_Final_Recruit_Distribution()`.

**Current code** (`atecology/atdemography.c` lines 1909–1948 (truncated))

```c
 1909:                 
 1910: 			break;
 1911: 		case external_recruit:
 1912: 			if ((bm->current_box == bm->first_box) && (bm->current_layer == bm->top_layer)) {
 1913: 				/* If recruit outside model area and in first dynamic box then update
 1914: 				 the Migration array */
 1915:                 mid = EMBRYO[species].migIDmatch[qid];
 1916:                 recruit_outside = 1;
 1917:                 if (mid < 0) { // See AlignVsMigration() for how this is set
 1918:                     recruit_outside = 0;
 1919:                 }
 1920:                 
 1921:                         
 1922:                 /**
 1923:                 thisday = EMBRYO[species].StartDay[ngene][qid];
 1924: 
 1925:                 if ((species == 64) || (species == 69)) {
 1926:                     fprintf(bm->logFile, "Time: %e %s ngene: %d mid: %d thisday: %d Start: %d Return: %d recruit_outside: %d\n",
 1927:                             bm->dayt, FunctGroupArray[species].groupCode, ngene, mid, thisday, MIGRATION[species].Leave_Now[mid], MIGRATION[species].Return_Now[mid], recruit_outside);
 1928:                 }
 1929:                 **/
 1930:                 
 1931:                 if (recruit_outside) {
 1932:                     //oldDEN = MIGRATION[species].DEN[ngene][mid];
 1933:                     //oldSN = MIGRATION[species].SN[ngene][mid];
 1934:                     //oldRN = MIGRATION[species].RN[ngene][mid];
 1935: 
 1936:                     if(EMBRYO[species].BulkRecruits[ngene] * enviro_scalar > 0.0) {
 1937:                         //MIGRATION[species].DEN[ngene][mid] += EMBRYO[species].BulkRecruits[ngene] * enviro_scalar; // As all in youngest cohort maps to ngene not cohort in Migration
 1938:                         MIGRATION[species].recruit[ngene][mid] = EMBRYO[species].BulkRecruits[ngene] * enviro_scalar;
 1939:                         FunctGroupArray[species].speciesParams[direct_recruit_entry_id] = 1;
 1940: 
 1941:                         // Update size here too - used to reset to KMIG each year, but that is erroneous
 1942:                         //MIGRATION[species].SN[ngene][mid] = (oldSN * oldDEN + KWSR_sp *	EMBRYO[species].BulkRecruits[ngene] * enviro_scalar) / (EMBRYO[species].BulkRecruits[ngene] * enviro_scalar + oldDEN + small_num);
 1943:                         //MIGRATION[species].RN[ngene][mid] = (oldRN * oldDEN + KWRR_sp *	EMBRYO[species].BulkRecruits[ngene] * enviro_scalar) / (EMBRYO[species].BulkRecruits[ngene] * enviro_scalar + oldDEN + small_num);
 1944:                         
 1945:                         /**
 1946:                         if (species == 52) {
 1947:                             fprintf(bm->logFile, "Time: %e %s ngene: %d mid: %d MIGRATIONrecruit: %e BulkRecruits: %e enviro_scalar: %e\n",
 1948:                                 bm->dayt, FunctGroupArray[species].groupCode, ngene, mid, MIGRATION[species].recruit[ngene][mid], EMBRYO[species].BulkRecruits[ngene], enviro_scalar);
```

**Suggested fix (from review):** Skip the MIGRATION write when `mid < 0`, move the zeroing into the `recruit_outside` branch, and initialise migIDmatch to -1.

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

## DEMO-16

**stale variable** · Conf M / Sev M  
**Lines (current code):** 523-532  
**Lines at review (Oct 2026):** 523-532

**Function:** `Vertebrate_Reproduction()`

**Why it's thought to be a bug**

The comment says that if general aging happens here too, this step should be skipped. But `need_update > -1` tests the local copy read at 522, before `Update_Age_Distrib(...,0)` sets `speciesParams[need_update_id] = -1` (3400-3401). The test is always true (0 or 1), so the recruit-related update also runs. It then overwrites the youngest cohorts' tempPopRatio/boxPopRatio with `recVERTpopratio`, which was not refreshed this step: when need_update = 1, Get_Latest_Additions takes the tempPopRatio branch (2850). Function: `Vertebrate_Reproduction()`.

> 🔀 **Changed by the merge of main (Nov 2026):** Update_Age_Distrib b-loop / [b] index (A-02). Check whether this finding is now fixed before acting.

**Current code** (`atecology/atdemography.c` lines 521–534)

```c
  521:                 /* Update age distributions - only do at end for typical group, but do it every box for site attached */
  522:                 need_update = (int) (FunctGroupArray[species].speciesParams[need_update_id]);
  523:                 if (bm->flagagestruct && need_update && (((bm->current_box == bm->last_box) && (!bm->current_layer))
  524:                         || ((sp_ddepend_move == 1) && !bm->current_layer))) {
  525:                     Update_Age_Distrib(bm, species, stock_id, sp_ddepend_move, 0, llogfp);  // 0 flag to indicate general aging
  526:                 }
  527: 
  528:                 /* If no general aging but only new recruits still do update but of more limited extent (if general
  529:                  aging occuring here too then skip this step as update already done */
  530:                 need_recupdate = (int) (FunctGroupArray[species].speciesParams[need_recupdate_id]);
  531:                 if (bm->flagagestruct && (need_update > -1) && need_recupdate && (((bm->current_box == bm->last_box) && !bm->current_layer)
  532:                         || ((sp_ddepend_move == 1) && !bm->current_layer))) {
  533: 
  534:                     /*
```

**Suggested fix (from review):** Re-read `need_update = (int)speciesParams[need_update_id]` before the second test (or test `need_update != 1`).

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

## DEMO-17

**stale loop variable** · Conf H / Sev M  
**Lines (current code):** 3161-3163, 3294-3296  
**Lines at review (Oct 2026):** 3157-3159, 3290-3292

**Function:** `Update_Age_Distrib()`

**Why it's thought to be a bug**

`do_test = 0; if(rec_related) do_test = 1; else if(cohort || (!cohort && k)) do_test = 1;` is evaluated outside the `for (k...)` loop, where `k == classize` from the previous loop. The test is therefore always true, and the intended exemption for cohort 0 / sub-class 0 never applies: the minimum floor `maxvalue/sp_recruitRange` is forced on it. Update_Invert_Cohorts (5218) evaluates the same test correctly inside the k loop. Function: `Update_Age_Distrib()`.

**Current code** (`atecology/atdemography.c` lines 3159–3165)

```c
 3159: 				 cohorts are disproportinately lost and that not all individuals grow
 3160: 				 equally fast so some will sneak ahead/lag behind anyway */
 3161: 				totksum[stock_id] = 0;
 3162: 				do_test = 0;
 3163: 				if(rec_related) do_test = 1;
 3164: 				else if(cohort || (!cohort && k)) do_test = 1;
 3165: 				for (k = 0; k < classize; k++) {
```

**Suggested fix (from review):** Move the test inside the k loop: `if ((rec_related || cohort || k) && tempPopRatio < ...)`.

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

## DEMO-18

**flag not reset / wrong index** · Conf H / Sev M  
**Lines (current code):** 3085-3118  
**Lines at review (Oct 2026):** 3081-3114

**Function:** `Update_Age_Distrib()`

**Why it's thought to be a bug**

`stock_done[s_id]` is reset once per call (2996) but is shared across all `cohort` and `qid` iterations. After the first cohort marks a stock done, no later cohort or queue entry updates tempPopRatio for that stock. The outer `for (i < maxstock_id)` loop is also redundant. In the same block, the `recVERTpopratio[stock_id]...` update (3108) uses the function argument `stock_id` instead of `s_id`. Function: `Update_Age_Distrib()`.

**Current code** (`atecology/atdemography.c` lines 3083–3120)

```c
 3083: 			 * At present it just assumes the same distribution for ever, but in reality should
 3084: 			 * allow for more the same variability in cohort strength to flow through
 3085: 			 */
 3086:             for (cohort = 0; cohort < NumCohort; cohort++) {
 3087: 				//stage = FunctGroupArray[species].cohort_stage[cohort];
 3088: 				for (qid = MIGRATION[species].num_in_queue_done; qid < MIGRATION[species].num_in_queue; qid++) {
 3089:                     is_outside = 0;
 3090: 					// If all leave the arena then see if currently gone
 3091: 					if ( MIGRATION[species].all_go[cohort][qid] && (MIGRATION[species].Return_Period[qid] > 0.0)) {
 3092: 						if ((MIGRATION[species].Leave_Now[qid] <= bm->dayt) && (bm->dayt <= MIGRATION[species].Return_Now[qid])){
 3093: 							is_outside = 1;
 3094: 						}
 3095: 					}
 3096: 					if ( is_outside ){
 3097: 						/* Get a representative value for each stock */
 3098: 						for (i = 0; i < maxstock_id; i++) {
 3099: 							for (ij = 0; ij < bm->nbox; ij++) {
 3100: 								if (bm->boxes[ij].type != BOUNDARY) {
 3101: 									for (k = 0; k < bm->boxes[ij].nz; k++) {
 3102: 										/* Identify the stock under consideration */
 3103: 										s_id = bm->group_stock[species][ij][k];
 3104: 
 3105: 										if(!stock_done[s_id]){
 3106: 											for (b = 1; b < classize; b++) {
 3107: 												bm->tempPopRatio[s_id][species][cohort][b] += (FunctGroupArray[species].boxPopRatio[ij][k][cohort][b-1] * MIGRATION[species].DEN[cohort][qid]);
 3108: 												if(rec_related)
 3109: 													recVERTpopratio[stock_id][species][cohort][b] += (FunctGroupArray[species].boxPopRatio[ij][k][cohort][b-1] * MIGRATION[species].DEN[cohort][qid]);
 3110: 
 3111: 												if((!_finite(FunctGroupArray[species].boxPopRatio[ij][k][cohort][b])) || (!_finite(bm->tempPopRatio[s_id][species][cohort][b]))){
 3112: 													quit("tempPopRatio for %s-%d in box%d-%d is NAN for stock_id %d as boxPopRatio[%d] %e, MIGRATIONden[%d]: %e\n",
 3113: 															FunctGroupArray[species].groupCode, cohort, ij, k, s_id, b-1, FunctGroupArray[species].boxPopRatio[ij][k][cohort][b-1],
 3114: 															qid, MIGRATION[species].DEN[cohort][qid]);
 3115: 												}
 3116: 
 3117: 											} // Note boxPopRatio itself is never updated as assumed constant through time for now
 3118: 											stock_done[s_id] = 1;
 3119: 										}
 3120: 									}
```

**Suggested fix (from review):** Reset `stock_done` at the start of each cohort/qid iteration, and use `s_id` in 3104.

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

## DEMO-19

**wrong variable / logic** · Conf H / Sev M  
**Lines (current code):** 5258-5267, 5287-5288  
**Lines at review (Oct 2026):** 5259-5268, 5288-5289

**Function:** `Get_Habitat_Mediated_Recruit_Scalar()`

**Why it's thought to be a bug**

The loop runs over `sp` but the body uses `species` throughout (`FunctGroupArray[species].isCover`, `.coverID`). The same term is added K_num_tot_sp times if the recruiting species is itself a cover group; otherwise nothing is added. `isCover == CORAL || isCover == SPONGE` compares a TRUE/FALSE flag to group-type constants, so `like_coral` is effectively never set; it should test `groupType`. `hab_scalar` also starts at 1.0 and is capped at 1.0, so positive habitat preferences can never raise it. Function: `Get_Habitat_Mediated_Recruit_Scalar()`.

**Current code** (`atecology/atdemography.c` lines 5256–5290)

```c
 5256: 	int rec_stage = FunctGroupArray[species].numStages;  // As recruit stage stored after all other stages (as usually implict not explicit)
 5257: 	int sp;
 5258: 	int like_coral = 0;
 5259: 	double sp_like;
 5260: 	double hab_scalar = 1.0;
 5261: 	double Rcoefft = bm->RugCover_Coefft;
 5262: 	double Rconst = bm->RugCover_Const;
 5263: 
 5264: 	if (flag_recruit_habdepend){
 5265: 		for(sp = 0; sp < bm->K_num_tot_sp; sp++){
 5266: 			if (FunctGroupArray[species].isCover == TRUE) {
 5267: 				sp_like = bm->HABITATlike[species][rec_stage][FunctGroupArray[species].coverID];
 5268: 				hab_scalar += sp_like * bm->coveramt[FunctGroupArray[species].coverID][bm->current_box];   // This covers larvae liking turf etc and no bare sediments
 5269: 				if((FunctGroupArray[species].isCover == CORAL) || (FunctGroupArray[species].isCover == SPONGE)) {
 5270: 					like_coral = 1;
 5271: 				}
 5272: 			}
 5273: 		}
 5274: 	}
 5275: 
 5276: 
 5277: 	switch(bm->flag_refuge_model){
 5278: 	case no_refuge_model:
 5279: 	case orig_refuge_model:
 5280: 		/* Do nothing */
 5281: 		break;
 5282: 	case rugosity_refuge_model:
 5283: 		if(like_coral) {
 5284: 			hab_scalar *= (Rcoefft * log(LocalRugosity) + Rconst);
 5285: 		}
 5286: 		break;
 5287: 	default:
 5288: 		quit("No such refuge model set flag_refuge_model to 0, 1 or 2 (current;y %d)\n", bm->flag_refuge_model);
 5289: 		break;
 5290: 	}
```

**Suggested fix (from review):** Use `FunctGroupArray[sp]` inside the loop, test `FunctGroupArray[sp].groupType == CORAL/SPONGE`, and review the 1.0 start value against the cap.

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

## DEMO-20

**cohort×genotype** · Conf H / Sev M  
**Lines (current code):** 2068-2074  
**Lines at review (Oct 2026):** 2064-2070

**Function:** `Sanity_Check_Recruits()`

**Why it's thought to be a bug**

`for (n = 0; n < numCohorts; n++) tot_adult_den += scaled_FSPB[n] * tr[k][NumsTracers[n]];`. scaled_FSPB and NumsTracers are indexed by cohort×gene. With numGeneTypes > 1, this sums the wrong (young) entries and misses the adults, so the larvae-per-adult trim is wrong. Function: `Sanity_Check_Recruits()`.

**Current code** (`atecology/atdemography.c` lines 2066–2076)

```c
 2066:     int sp_numGeneTypes = (int) (FunctGroupArray[species].numGeneTypes);
 2067:     int qid = EMBRYO[species].next_larvae;
 2068:     
 2069:     if (recruit_sp == fixed_linear_recruit) {
 2070:         for(ij=0; ij < bm->nbox; ij++){
 2071:             if(bm->boxes[ij].type != BOUNDARY) {
 2072:                 for (k = 0; k < bm->boxes[ij].nz; k++) {
 2073:                     for (n = 0; n < FunctGroupArray[species].numCohorts; n++) {
 2074:                         tot_adult_den += FunctGroupArray[species].scaled_FSPB[n] * bm->boxes[ij].tr[k][FunctGroupArray[species].NumsTracers[n]];
 2075:                     }
 2076:                 }
```

**Suggested fix (from review):** Loop `n < numCohortsXnumGenes`.

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

## DEMO-21

**accumulator overwritten** · Conf H / Sev L  
**Lines (current code):** 2126-2130  
**Lines at review (Oct 2026):** 2122-2126

**Function:** `Store_Recruitment_Diagnostics()`

**Why it's thought to be a bug**

`tot_yoy[species][i] = (...)` sits inside the `ngene` loop, so only the last genotype's YOY is kept (the migrant add at 2141 is `+=`). This affects the YOY diagnostic output (atbiolmessage.c:1631). Function: `Store_Recruitment_Diagnostics()`.

**Current code** (`atecology/atdemography.c` lines 2124–2132)

```c
 2124:             
 2125: 			//yoysum += EMBRYO[species].Larvae[i][ngene][qid];
 2126: 			yoy[ngene] += EMBRYO[species].Larvae[i][ngene][qid];
 2127: 			if (use_aggregate) {
 2128:                 tot_yoy[species][i] = (totrecruit[species][i][ngene] * (KWRR_sp + KWSR_sp));
 2129: 			} else {
 2130: 				tot_yoy[species][i] = (EMBRYO[species].Larvae[i][ngene][qid] * (KWRR_sp + KWSR_sp));
 2131: 			}
 2132:             
```

**Suggested fix (from review):** Zero `tot_yoy[species][i]` before the ngene loop and use `+=`.

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

## DEMO-22

**double count / inconsistent** · Conf M / Sev M  
**Lines (current code):** 2241-2258, 2288-2289  
**Lines at review (Oct 2026):** 2237-2254, 2284-2285

**Function:** `Store_Recruitment_Diagnostics()`

**Why it's thought to be a bug**

`if (fished_chrt == ngene) biomass += tr[k][den]; ... biomass += tr[k][den];` counts the fished cohort twice and counts every youngest-cohort density plus larvae. `calcNstart[expect_id]` is then set to this value unconditionally (2288). Reset_Mortality (4166-4167, 4205) instead defines calcNstart as the fished cohort's numbers only. The two routines overwrite each other's pseudo-assessment starting N with different quantities. The history reset at 2167 is correctly limited to `fished_chrt < numGeneTypes`, but the final assignment is not. The non-AGE_STRUCTURED branch (2261) also does not skip BOUNDARY boxes, unlike the branch above it. Function: `Store_Recruitment_Diagnostics()`.

**Current code** (`atecology/atdemography.c` lines 2239–2278 (truncated))

```c
 2239:             }
 2240: 		}
 2241: 		// Now initialise the calcN and starting numbers in bm->calcTrackedMort
 2242: 		if (FunctGroupArray[species].groupAgeType == AGE_STRUCTURED) {
 2243: 			den = FunctGroupArray[species].NumsTracers[ngene];
 2244: 			for (i = 0; i < bm->nbox; i++) {
 2245: 				if (bm->boxes[i].type != BOUNDARY) {
 2246: 					for (k = 0; k < bm->boxes[i].nz; k++) {
 2247: 						if (fished_chrt == ngene )
 2248: 							biomass += bm->boxes[i].tr[k][den];
 2249: 						stock_id = bm->group_stock[species][i][k];
 2250: 
 2251: 						bm->calcTrackedMort[species][ngene][stock_id][start_id] += bm->boxes[i].tr[k][den];
 2252: 						biomass += bm->boxes[i].tr[k][den];
 2253: 					}
 2254: 				}
 2255: 			}
 2256: 			for (i = 0; i < maxstock_id; i++) {
 2257: 				bm->calcTrackedMort[species][ngene][i][start_id] += EMBRYO[species].Larvae[i][ngene][qid];
 2258: 				biomass += EMBRYO[species].Larvae[i][ngene][qid];
 2259: 			}
 2260: 		} else {
 2261: 			den = FunctGroupArray[species].totNTracers[ngene];
 2262: 			for (i = 0; i < bm->nbox; i++) {
 2263: 				// Loop through each pelagic layer
 2264: 				for (k = 0; k < bm->boxes[i].nz; k++) {
 2265: 					stock_id = bm->group_stock[species][i][k];
 2266: 					biomass += bm->boxes[i].tr[k][den] * bm->boxes[i].dz[k] * bm->boxes[i].area * FunctGroupArray[species].habitatCoeffs[WC];
 2267: 					chrt_biomass = bm->boxes[i].tr[k][den] * bm->boxes[i].dz[k] * bm->boxes[i].area * FunctGroupArray[species].habitatCoeffs[WC];
 2268: 					bm->calcTrackedMort[species][ngene][stock_id][start_id] += chrt_biomass;
 2269: 				}
 2270: 				// Get stocks based on assuming dealing with deepest water column layer
 2271: 				// layer 0
 2272: 				stock_id = bm->group_stock[species][i][0];
 2273: 				// Loop through each sediment layer
 2274: 				for (k = 0; k < bm->boxes[i].sm.nz; k++) {
 2275: 					biomass += bm->boxes[i].sm.tr[k][den] * bm->boxes[i].sm.dz[k] * bm->boxes[i].area * FunctGroupArray[species].habitatCoeffs[SED];
 2276: 					chrt_biomass = bm->boxes[i].sm.tr[k][den] * bm->boxes[i].sm.dz[k] * bm->boxes[i].area * FunctGroupArray[species].habitatCoeffs[SED];
 2277: 					bm->calcTrackedMort[species][ngene][stock_id][start_id] += chrt_biomass;
 2278: 				}
```

**Suggested fix (from review):** Remove the extra `biomass +=` at 2247 and only set calcNstart when `fished_chrt < sp_numGeneTypes`. Skip BOUNDARY boxes.

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

## DEMO-23

**uninitialised** · Conf M / Sev L  
**Lines (current code):** 2554-2585 (used 389-392, 2686)  
**Lines at review (Oct 2026):** 2550-2581 (used 389-392, 2682)

**Function:** `Get_Numbers_Aging()`

**Why it's thought to be a bug**

In the evolution branch, `*this_p_ageup` is never set. The caller's `this_prop_ageup` (declared without an initial value at 116) is then passed to `Update_Aging_Numbers` and on to `Age_Contaminants_Store(..., this_p_ageup)` when contaminants are tracked. Line 2551 `ng = cohort - basecohort;` is also wrong (missing `* sp_numGeneTypes`), but it is overwritten before use. Function: `Get_Numbers_Aging()`.

**Current code** (`atecology/atdemography.c` lines 2552–2587)

```c
 2552: 	ng = cohort - basecohort;
 2553: 
 2554: 	/* Find numbers shifting and shift numbers lost - ones added done in aging update */
 2555: 	if (bm->flag_do_evolution && bm->flagagestruct) {
 2556: 		// Number of individuals aging out of the age-gene bin in total
 2557: 		num_aging = DNA[species].phenotype_aging_up[cohort];
 2558: 		shiftVERT[species][cohort][DEN_id] = VERTinfo[species][cohort][DEN_id] - num_aging;
 2559: 
 2560: 		/* Store numbers lost so can be used in calculate size after new recruits arrive */
 2561: 		lostden = num_aging;
 2562: 		if (cohort < sp_numGeneTypes)
 2563: 			lostden_zero[cohort] = num_aging;
 2564: 
 2565:         /**
 2566:         //if (species == 22) {
 2567: 		if (do_debug && (bm->which_check == species)) {
 2568: 			fprintf(llogfp, "Time: %e, %s-%d shiftVERT[den]: %e, num_aging: %e, VERTinfo[den]: %e\n", bm->dayt,
 2569: 					FunctGroupArray[species].groupCode, cohort, shiftVERT[species][cohort][DEN_id], num_aging, VERTinfo[species][cohort][DEN_id]);
 2570: 		}
 2571:         **/
 2572:         
 2573: 		// Find number aging into the age-gene bin (well it will be age+1 but the +1 transition is done in Update_Aging_Numbers() to be consistent with non-evolution case
 2574: 		dennow = 0.0;
 2575: 		for (ng = 0; ng < FunctGroupArray[species].numGeneTypes; ng++){
 2576: 			dennow += DNA[species].phenotype_transition[cohort][ng];
 2577: 
 2578:             /**
 2579:             if (do_debug && (bm->which_check == species)) {
 2580:             //if (species == 22) {
 2581: 				fprintf(llogfp,"Time: %e box%d-%d %s-%d, ng %d has dennow: %e, phenotype_transition: %e\n",
 2582: 						bm->dayt, bm->current_box, bm->current_layer, FunctGroupArray[species].groupCode, cohort, ng, dennow, DNA[species].phenotype_transition[cohort][ng]);
 2583:             }
 2584:             **/
 2585: 		}
 2586: 
 2587: 	} else {
```

**Suggested fix (from review):** Set `*this_p_ageup = num_aging / (VERTinfo DEN + small_num)` in the evolution branch, and initialise this_prop_ageup.

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

## DEMO-24

**dead code / log spam** · Conf H / Sev L  
**Lines (current code):** 1749-1833  
**Lines at review (Oct 2026):** 1742-1826

**Function:** `Recruit_Migration()`

**Why it's thought to be a bug**

`direct_recruit_entry` is only ever set to 1, inside the same `if` that sets `recruit_outside = 1` (1782-1787). So `if (recruit_outside && (!direct_recruit_entry))` (1796) can never be true and the whole MIGRATION add block is dead. In practice delivery happens in atmovement.c:1295. Line 1755 also writes an unconditional `fprintf(bm->logFile, ...)` for every gene × queue entry on every recruiting call. Function: `Recruit_Migration()`.

**Current code** (`atecology/atdemography.c` lines 1747–1786 (truncated))

```c
 1747:     //fprintf(bm->logFile, "Time: %e %s box%d-%d (vs first_box %d, top_layer %d) numGeneTypes: %d Recruit_Migration recruitType: %d num_in_queue: %d externalReproducer: %d\n",  bm->dayt, FunctGroupArray[species].groupCode, bm->current_box, bm->current_layer, bm->first_box, bm->top_layer, sp_numGeneTypes, FunctGroupArray[species].recruitType, MIGRATION[species].num_in_queue, FunctGroupArray[species].externalReproducer);
 1748: 
 1749:     if((FunctGroupArray[species].externalReproducer && (FunctGroupArray[species].recruitType != external_spawn_local_recruit)) || (FunctGroupArray[species].recruitType == external_recruit)) {
 1750:         if ((bm->current_box == bm->first_box) && (bm->current_layer == bm->top_layer)) {
 1751: 
 1752:             for(ngene = 0; ngene < sp_numGeneTypes; ngene++){
 1753:                 for (mid = 0; mid < MIGRATION[species].num_in_queue; mid++) {
 1754:                     
 1755:                     fprintf(bm->logFile, "Time: %e %s RecruitQueueMatch: %d\n", bm->dayt, FunctGroupArray[species].groupCode, MIGRATION[species].RecruitQueueMatch[mid]);
 1756:                     
 1757:                     if (MIGRATION[species].RecruitQueueMatch[mid] > 0) {
 1758:                         if (bm->flag_modify_KWSR) {
 1759:                             KWSR_sp = Get_Recruit_Size_Forcing(bm, species, do_debug, llogfp);
 1760:                             KWRR_sp = KWSR_sp * FunctGroupArray[species].X_RS[ngene];
 1761:                         }
 1762: 
 1763:                         /* If recruit outside model area and in first dynamic box then update the Migration array */
 1764:                         recruit_outside = 0;
 1765: 
 1766:                         thisday = EMBRYO[species].StartDay[ngene][EMBRYO[species].next_recruit];
 1767:                     
 1768:                         /**
 1769:                         if(do_debug) {
 1770:                             fprintf(bm->logFile, "Time: %e %s mid: %d Return_Now = %d, Leave_Now = %d, thisday: %d\n",  bm->dayt, FunctGroupArray[species].groupCode, mid, MIGRATION[species].Return_Now[mid], MIGRATION[species].Leave_Now[mid], thisday);
 1771:                         }
 1772:                         **/
 1773: 
 1774:                         /**/
 1775:                         if (thisday < MIGRATION[species].Leave_Now[mid])
 1776:                             continue; // Skip ahead if haven't gotten this far in time yet
 1777:                         if ( thisday > MIGRATION[species].Return_Now[mid])
 1778:                             continue; // Skip ahead if time is alredy past the migration time
 1779:                         /**/
 1780:                     
 1781:                         /* Outside the model when recruitment happens */
 1782:                         if ((MIGRATION[species].Return_Now[mid] >= thisday) && (MIGRATION[species].Leave_Now[mid] <= thisday)) {
 1783:                             recruit_outside = 1;
 1784:                             FunctGroupArray[species].speciesParams[recruit_outside_id] = 1;
 1785:                             FunctGroupArray[species].speciesParams[direct_recruit_entry_id] = 0;
 1786:                             direct_recruit_entry = 1;
```

**Suggested fix (from review):** Remove the dead block, or read `direct_recruit_entry` from speciesParams if it was meant to be live. Guard the fprintf with do_debug.

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

## DEMO-25

**swapped index / format args** · Conf H / Sev L  
**Lines (current code):** 2852-2857, 2876-2878, 2887-2889  
**Lines at review (Oct 2026):** 2848-2853, 2872-2874, 2883-2885

**Function:** `Get_Latest_Additions()`

**Why it's thought to be a bug**

The debug check `bm->tempPopRatio[stock_id][ngene][cohort][0]` swaps dimensions: it should be `[stock_id][species][ngene][0]`, because tempPopRatio is `[nstock][sp][ncxg][maxspage]` (atbiolsetup.c:2895). In all three fprintfs the format `"%s-%d in Box%d-%d (ngene %d)"` is given `cohort, ngene, box, layer`, so the labels are misaligned. 2850 also prints every call for which_check without do_debug. Function: `Get_Latest_Additions()`.

> 🔀 **Changed by the merge of main (Nov 2026):** code shown above was changed by the merge. Check whether this finding is now fixed before acting.

**Current code** (`atecology/atdemography.c` lines 2850–2889 (truncated))

```c
 2850: 
 2851: 	} else {
 2852: 		for ( ngene = 0; ngene < sp_numGeneTypes; ngene++ ){
 2853: 			bm->tempPopRatio[stock_id][species][ngene][0] += coming_SPden[ngene]; //Used to say cohort for tempPopRatio
 2854: 
 2855: 			if((!_finite(bm->tempPopRatio[stock_id][species][ngene][0])) || (species == bm->which_check)){  // Nov 2026: main had [stock_id][ngene][cohort][0] (ngene in species slot)
 2856: 				fprintf(llogfp,"Time: %e %s-%d in Box%d-%d (ngene %d) has tempPopRatio %e for stock %d subclass %d as dennow: %e\n",
 2857: 						bm->dayt, FunctGroupArray[species].groupCode, cohort, ngene, bm->current_box, bm->current_layer,
 2858: 						bm->tempPopRatio[stock_id][species][ngene][0], stock_id, 0, coming_SPden[ngene]);
 2859: 			}
 2860: 
 2861: 		}
 2862: 	}
 2863: 
 2864: 	/* Add any recruits due to arrival in migration array */
 2865: 	if ((bm->current_box == bm->first_box) && (bm->current_layer == bm->top_layer)) {
 2866: 		for ( ngene = 0; ngene < sp_numGeneTypes; ngene++ ){
 2867: 			for ( qid = MIGRATION[species].num_in_queue_done; qid < MIGRATION[species].num_in_queue; qid++) {
 2868: 				if (MIGRATION[species].recruit[ngene][qid] > 0) {
 2869: 					/* Split across stocks */
 2870: 					migstock_id = (int) (MIGRATION[species].returnstock[qid]) - 1;
 2871: 					if (migstock_id < 0) {
 2872: 						/* Homogeneously spread across stocks */
 2873: 						for ( mxk = 0; mxk < maxstock_id; mxk++) {
 2874: 							bm->tempPopRatio[mxk][species][ngene][0] += (MIGRATION[species].recruit[ngene][qid] / (double)(maxstock_id));
 2875: 
 2876: 							if(!_finite(bm->tempPopRatio[mxk][species][ngene][0])){
 2877: 								fprintf(llogfp,"Time: %e %s-%d in Box%d-%d (ngene %d) has tempPopRatio %e for stock %d subclass %d as MIGrecruit[%d]: %e, maxstock: %d\n",
 2878: 										bm->dayt, FunctGroupArray[species].groupCode, cohort, ngene, bm->current_box, bm->current_layer,
 2879: 										bm->tempPopRatio[mxk][species][ngene][0], mxk, 0, qid, MIGRATION[species].recruit[ngene][qid], maxstock_id);
 2880: 							}
 2881: 
 2882: 						}
 2883: 					} else {
 2884: 						/* Directed to a specific stock */
 2885: 						bm->tempPopRatio[migstock_id][species][ngene][0] += MIGRATION[species].recruit[ngene][qid];
 2886: 
 2887: 						if(!_finite(bm->tempPopRatio[migstock_id][species][ngene][0])){
 2888: 							fprintf(llogfp,"Time: %e %s-%d in Box%d-%d (ngene %d) has tempPopRatio %e for stock %d subclass %d as MIGrecruit[%d]: %e\n",
 2889: 									bm->dayt, FunctGroupArray[species].groupCode, cohort, ngene, bm->current_box, bm->current_layer,
```

**Suggested fix (from review):** Fix the index to `[stock_id][species][ngene][0]` and reorder the args to `cohort, box, layer, ngene`.

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

## DEMO-26

**wrong index / log spam** · Conf H / Sev L  
**Lines (current code):** 2653-2657  
**Lines at review (Oct 2026):** 2649-2653

**Function:** `Update_Aging_Numbers()`

**Why it's thought to be a bug**

The check `!_finite(bm->tempPopRatio[stock_id][species][cohort][i])` uses `i == 0` after the loop and the wrong cohort; it should check `[nextcid][0]`, which is what is printed. The `|| (species == bm->which_check)` condition has no `do_debug`, so it prints on every aging event for the checked species. Function: `Update_Aging_Numbers()`.

**Current code** (`atecology/atdemography.c` lines 2651–2659)

```c
 2651: 		if (cohort <= (num_cohorts - sp_numGeneTypes)) {
 2652: 			bm->tempPopRatio[stock_id][species][nextcid][0] += dennow;
 2653: 
 2654: 			if((!_finite(bm->tempPopRatio[stock_id][species][cohort][i])) || (species == bm->which_check)){
 2655: 				fprintf(llogfp,"Time: %e %s-%d in Box%d-%d has tempPopRatio %e for stock %d subclass %d as dennow: %e\n",
 2656: 						bm->dayt, FunctGroupArray[species].groupCode, nextcid, bm->current_box, bm->current_layer,
 2657: 						bm->tempPopRatio[stock_id][species][nextcid][0], stock_id, 0, dennow);
 2658: 			}
 2659: 
```

**Suggested fix (from review):** Check `[nextcid][0]` and add `&& do_debug`.

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

## DEMO-27

**copy-paste message** · Conf H / Sev L  
**Lines (current code):** 1617-1620  
**Lines at review (Oct 2026):** 1610-1613

**Function:** `Get_Enviro_Recruit_Forcing()`

**Why it's thought to be a bug**

The warning says "...as flagtempdepend_move = %d" and passes `bm->flagtempdepend_move`, but the guarding flag is `bm->flagtempdepend_reprod`. The comment also says "movement". Function: `Get_Enviro_Recruit_Forcing()`.

**Current code** (`atecology/atdemography.c` lines 1615–1622)

```c
 1615:     //int recruit_sp = (int) (FunctGroupArray[species].speciesParams[flagrecruit_id]);
 1616:     
 1617:     if(!bm->flagtempdepend_reprod) { // Have turned off temperature dependent movement
 1618:         if(temp_sensitive_sp && bm->newmonth) {
 1619:             warn("Time: %e Zeroing temperature sensitivity for reproduction of %s even though flagtempsensitive %d as flagtempdepend_move = %d\n", bm->dayt, FunctGroupArray[species].groupCode, temp_sensitive_sp, bm->flagtempdepend_move);
 1620:         }
 1621:         temp_sensitive_sp = 0;
 1622:     }
```

**Suggested fix (from review):** Print `flagtempdepend_reprod`.

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

## DEMO-28

**identical branches** · Conf L / Sev L  
**Lines (current code):** 1648-1652  
**Lines at review (Oct 2026):** 1641-1645

**Function:** `Get_Enviro_Recruit_Forcing()`

**Why it's thought to be a bug**

`if (sp_q10receff == 1) enviro_scalar *= Tscalar; else if (sp_q10receff == 2) enviro_scalar *= Tscalar;` has identical branches. Either option 2 was meant to differ, or the branches should be merged. Function: `Get_Enviro_Recruit_Forcing()`.

**Current code** (`atecology/atdemography.c` lines 1646–1654)

```c
 1646: 			Tscalar = 1.0 / (FunctGroupArray[species].Tcorr * pHscalar);
 1647: 
 1648: 		if (sp_q10receff == 1) {
 1649: 			enviro_scalar *= Tscalar;
 1650: 		} else if (sp_q10receff == 2) {
 1651: 			enviro_scalar *= Tscalar;
 1652: 		} else {
 1653: 			/* simple pH effects */
 1654: 			enviro_scalar *= FunctGroupArray[species].pHcorr;
```

**Suggested fix (from review):** Confirm the intended behaviour for flagq10receff = 2.

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

## DEMO-29

**log spam** · Conf H / Sev L  
**Lines (current code):** 5191  
**Lines at review (Oct 2026):** 5192

**Function:** `Update_Invert_Cohorts()`

**Why it's thought to be a bug**

`fprintf(bm->logFile, "species %s, totsum = %e\n", ...)` is unconditional and runs for every cohort, every box with activity, and every call. The log grows large (compare the recent commit that removed a similar printf). Function: `Update_Invert_Cohorts()`.

**Current code** (`atecology/atdemography.c` lines 5189–5193)

```c
 5189:                 for (qid = MIGRATION[species].num_in_queue_done; qid < MIGRATION[species].num_in_queue; qid++) {
 5190:                     this_totsum += MIGRATION[species].aging[cohort][qid];
 5191:                 }
 5192: 
 5193: 				if (!this_totsum){
```

**Suggested fix (from review):** Guard with a debug flag or remove.

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

## DEMO-30

**wrong index** · Conf M / Sev L  
**Lines (current code):** 4426  
**Lines at review (Oct 2026):** 4427

**Function:** `Invertebrate_Reproduction()`

**Why it's thought to be a bug**

`bm->calcTrackedMort[species][0][stock_id][start_id] += spawned_biomass;` always credits genotype 0. Elsewhere, recruits for genotype `ngene` live in cohort index `ngene`. Function: `Invertebrate_Reproduction()`.

**Current code** (`atecology/atdemography.c` lines 4424–4428)

```c
 4424:                      */
 4425:                     
 4426: 					Find_Final_Invert_Recruit_Distribtuion(bm, species, ngene, qid, stock_id, spawned_biomass, wclayer, llogfp);
 4427: 
 4428: 					/* Update yoy sum - only if localised recruit as all other cases done in Store_Recruitment_Diagnostics() */
```

**Suggested fix (from review):** Use `[ngene]`.

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

Queue indices: `Vertebrate_Reproduction` (302-309) and `Find_Final_Recruit_Distribution` (1848-1862) apply `qid++` without the `num_in_larval_queue - 1` clamp that `Ecology_Find_Embryoes` (1142) and `Get_Recruits` (1372) use, so they could overrun Larvae/num_recruits at the end of a run. In the invertebrate path, Larvae are written at `next_larvae` (4537) but read at `next_recruit` (4657). num_recruits is written at `next_spawn` (qid from 4313 passed to 4421) and read at `next_recruit` (4997). This is only correct while the three counters stay in step.

- [ ] Investigate   - [ ] Ignore

**Your comments:**

> 

### LC-2

`Get_Settlers()` 2353-2357: for external_recruit, `MIGRATION.recruit[ngene][migIDmatch]` is overwritten with `recruits_coming` in every box and layer. For external recruits num_recruits is generally not set (Find_Final writes MIGRATION.recruit directly), so this may reset to 0 the value set at 1931 before atmovement.c consumes it. The same migIDmatch default-0 / -1 concern as DEMO-15 applies.

- [ ] Investigate   - [ ] Ignore

**Your comments:**

> 

### LC-3

`Update_Invert_Aging()` 5022/5089/5118: settlers (`num_recruits[...][next_recruit]`) are added to cohort 0 on every call where `did_something` is true (spawn, recruit and aging days, and every timestep of those days). num_recruits is never decremented for invertebrates, so recruits could be added more than once.

- [ ] Investigate   - [ ] Ignore

**Your comments:**

> 

### LC-4

`Get_Recruits()` 1362 uses `qid++` when SpawnRecruitOverlap, without the CounterNotDone condition that `Ecology_Find_Embryoes` (1140) uses. The comment says this is deliberate, but the two can then address different Larvae slots.

- [ ] Investigate   - [ ] Ignore

**Your comments:**

> 

### LC-5

`Ecology_Find_Embryoes()` 1145: the larvae sanity check only inspects ngene = 0 (ngene is still 0 before the loop).

- [ ] Investigate   - [ ] Ignore

**Your comments:**

> 

### LC-6

`Larval_Mortality()` returns 0 (wiping out recruitment) for any `intersp_depend_recruit` value other than 1 or 2.

- [ ] Investigate   - [ ] Ignore

**Your comments:**

> 

### LC-7

`Vertebrate_Reproduction()` 492 checks `active_den[cohort]` for a single leftover cohort index only.

- [ ] Investigate   - [ ] Ignore

**Your comments:**

> 
