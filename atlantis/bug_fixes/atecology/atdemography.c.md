# atecology/atdemography.c — bug review

Generated 2026-11-19 from `atlantis/to-do-fixt-list.md`. **30 findings** (severity H: 4; confidence H and severity H: 3; on the priority list: 3), plus 7 lower-confidence notes.

Each finding's **Code at issue** is located by searching the current code for the code quoted in the review, not by line number. Use the **Search for** text with your editor's Find, because line numbers will keep changing as fixes go in. → marks the matching line. If only a close match or a name could be found, the block says so. ⚠ means the quoted code is no longer in the file: it may have been changed or already fixed. **Lines at review** are the October numbers. Line numbers in the description text are approximate.

Conf/Sev: confidence it's a bug / severity if it is (H = wrong results or crash, M = wrong in some configs, L = cosmetic, log, leak or latent). ✔ = the reviewer re-checked it by hand. ⭐ = on the high-priority list.

## Summary

| ID | Found at | Conf/Sev | Category | Short description | Decision | Done |
|---|---|---|---|---|---|---|
| [DEMO-01](#demo-01) ⭐ | 1720 | H/H | wrong default / logic | `double KWSR_force = 0.0; if (bm->flag_modify_KWSR && (KWSRforceid > -1)) {...} return KWSR_force;`. When `fla… |  | [ ] |
| [DEMO-02](#demo-02) ⭐ | 2710, 2743, 3749 | H/H | off-by-one / cohort×genotype | `nextcid = cohort + numGeneTypes; if (cohort < num_cohorts)` where `num_cohorts = numCohortsXnumGenes - 1`. Wi… |  | [ ] |
| [DEMO-03](#demo-03) ⭐ | 4459 | H/H | wrong formula | Semelparous case: `spawning_biomass_contrib = this_biom * (1.0 - scaled_FSPB[cohort]);`. In Update_Invert_Agin… |  | [ ] |
| [DEMO-04](#demo-04) | 4430 | H/M | buffer overrun | `for (stk = 0; stk < maxstock_id + 1; stk++) tot_yoy[species][stk] = 0.0;` and `tot_yoy[species][maxstock_id] … |  | [ ] |
| [DEMO-05](#demo-05) | 3768 | H/M | stale loop variable / OOB | `if (FunctGroupArray[species].agingVERT[cohort][k][DEN_id] > 0)`. Here `k` is left over from the stock loop at… |  | [ ] |
| [DEMO-06](#demo-06) | 3722, 3757 | H/M | cohort×genotype / wrong index | `cid = floor(cohort / numGeneTypes); nextcid = cid + numGeneTypes;` combines an age index with the genotype co… |  | [ ] |
| [DEMO-07](#demo-07) | 4641, 4456 | H/M | accumulator overwritten | `MigSpawn = flag_allow_spawn * spawn_active * prop_mat * MIGRATION[species].DEN[cohort][qid];` is assigned, no… |  | [ ] |
| [DEMO-08](#demo-08) | 4646, 4456 | M/M | logic | Semelparous branch: `MIGRATION[species].DEN[cohort][qid] *= (1.0 - spawn_active);` ignores both `flag_allow_sp… |  | [ ] |
| [DEMO-09](#demo-09) | 4377, 4430, 4947 | H/M | repeated per-cohort call | The recruitment block (`overall_checkday == sp_spawnday + additional_day`) runs inside the per-cohort loop wit… |  | [ ] |
| [DEMO-10](#demo-10) | 4898 | M/M | shadowed parameter | The `ngene` parameter is immediately overwritten by `for (ngene = 0; ngene < numGeneTypes; ngene++)`. The `spa… |  | [ ] |
| [DEMO-11](#demo-11) | 4337 | M/M | flag not reset | `sp_Spawn_Now` and `sp_Age_Now` are set to 1 when any cohort matches its Spawn_Now/Age_Now day and are never r… |  | [ ] |
| [DEMO-12](#demo-12) | 1034, 1018 | M/H | stale state | `EMBRYO[species].IndSpawn[cohort]` is set only for cohorts that actually spawn. A cohort that is skipped (`Spa… |  | [ ] |
| [DEMO-13](#demo-13) | 928 | H/M | flag not reset | `allowed_to_spawn` is set to 0 inside the `mid` loop when one migrant group is too small (`li < min_li_mat`) a… |  | [ ] |
| [DEMO-14](#demo-14) | 1079 | M/M | logic | The death factor `(1 - scaled_FSPB[cohort])` is applied to every cohort with FSPB > 0 whenever any cohort spaw… |  | [ ] |
| [DEMO-15](#demo-15) | 1952, 1885, 1906 | M/M | OOB index / logic | `mid = migIDmatch[qid]; if (mid < 0) recruit_outside = 0; ... else { MIGRATION[species].recruit[ngene][mid] = … |  | [ ] |
| [DEMO-16](#demo-16) 🔀 | 531, 525, 3402 | M/M | stale variable | The comment says that if general aging happens here too, this step should be skipped. But `need_update > -1` t… |  | [ ] |
| [DEMO-17](#demo-17) | 3166 | H/M | stale loop variable | `do_test = 0; if(rec_related) do_test = 1; else if(cohort \|\| (!cohort && k)) do_test = 1;` is evaluated outsid… |  | [ ] |
| [DEMO-18](#demo-18) | 3105, 3109 | H/M | flag not reset / wrong index | `stock_done[s_id]` is reset once per call (2992) but is shared across all `cohort` and `qid` iterations. After… |  | [ ] |
| [DEMO-19](#demo-19) | 5266, 5269 | H/M | wrong variable / logic | The loop runs over `sp` but the body uses `species` throughout (`FunctGroupArray[species].isCover`, `.coverID`… |  | [ ] |
| [DEMO-20](#demo-20) | 2074 | H/M | cohort×genotype | `for (n = 0; n < numCohorts; n++) tot_adult_den += scaled_FSPB[n] * tr[k][NumsTracers[n]];`. scaled_FSPB and N… |  | [ ] |
| [DEMO-21](#demo-21) | 2128, 4428 | H/L | accumulator overwritten | `tot_yoy[species][i] = (...)` sits inside the `ngene` loop, so only the last genotype's YOY is kept (the migra… |  | [ ] |
| [DEMO-22](#demo-22) | 2248, 2201, 4098 | M/M | double count / inconsistent | `if (fished_chrt == ngene) biomass += tr[k][den]; ... biomass += tr[k][den];` counts the fished cohort twice a… |  | [ ] |
| [DEMO-23](#demo-23) | 2544, 2687, 2552 | M/L | uninitialised | In the evolution branch, `*this_p_ageup` is never set. The caller's `this_prop_ageup` (declared without an ini… |  | [ ] |
| [DEMO-24](#demo-24) | 1783, 1796, 1747 | H/L | dead code / log spam | `direct_recruit_entry` is only ever set to 1, inside the same `if` that sets `recruit_outside = 1` (1775-1780)… |  | [ ] |
| [DEMO-25](#demo-25) 🔀 | 2853, 2856 | H/L | swapped index / format args | The debug check `bm->tempPopRatio[stock_id][ngene][cohort][0]` swaps dimensions: it should be `[stock_id][spec… |  | [ ] |
| [DEMO-26](#demo-26) | 2654, 2573 | H/L | wrong index / log spam | The check `!_finite(bm->tempPopRatio[stock_id][species][cohort][i])` uses `i == 0` after the loop and the wron… |  | [ ] |
| [DEMO-27](#demo-27) | 1619 | H/L | copy-paste message | The warning says "...as flagtempdepend_move = %d" and passes `bm->flagtempdepend_move`, but the guarding flag … |  | [ ] |
| [DEMO-28](#demo-28) | 1649 | L/L | identical branches | `if (sp_q10receff == 1) enviro_scalar *= Tscalar; else if (sp_q10receff == 2) enviro_scalar *= Tscalar;` has i… |  | [ ] |
| [DEMO-29](#demo-29) | 5197, 5070 | H/L | log spam | `fprintf(bm->logFile, "species %s, totsum = %e\n", ...)` is unconditional and runs for every cohort, every box… |  | [ ] |
| [DEMO-30](#demo-30) | 4432 | M/L | wrong index | `bm->calcTrackedMort[species][0][stock_id][start_id] += spawned_biomass;` always credits genotype 0. Elsewhere… |  | [ ] |

---

## DEMO-01

**wrong default / logic** · Conf H / Sev H ⭐ priority  
**Found at (current code):** `atdemography.c` 1720
**Lines at review (Oct 2026):** 1712-1726 (used 455-458, 1751-1754, 2115-2118, 2149-2152; atmovement.c:1298)

**Function:** `Get_Recruit_Size_Forcing()`

**Why it's thought to be a bug**

`double KWSR_force = 0.0; if (bm->flag_modify_KWSR && (KWSRforceid > -1)) {...} return KWSR_force;`. When `flag_modify_KWSR` is on, every species gets `KWSRforce_id = -1` unless it has a column in the KWSR forcing file (atbiolsetup.c:1076-1081; non-vertebrates always stay -1). For those species the function returns 0, so recruits are given SN = RN = 0 (Vertebrate_Reproduction 468-469, atmovement.c recruits). Function: `Get_Recruit_Size_Forcing()`.

**Code at issue** — `atdemography.c` line 1720 (the line the review pointed to, found by its text)  
Search for: `int KWSRforceid = (int) (FunctGroupArray[species].speciesParams[KWSRforce_id]);`

```c
 1718: 
 1719: double Get_Recruit_Size_Forcing(MSEBoxModel *bm, int species, int do_debug, FILE *llogfp) {
 1720→     int KWSRforceid = (int) (FunctGroupArray[species].speciesParams[KWSRforce_id]);
 1721:     double KWSR_sp = FunctGroupArray[species].speciesParams[KWSR_id];
 1722:     double KWSR_force = 0.0;
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
**Found at (current code):** `atdemography.c` 2710, `atdemography.c` 2743, `atdemography.c` 3749
**Lines at review (Oct 2026):** 2730, 2708, 2738-2772

**Function:** `Update_Migration_Array()`

**Why it's thought to be a bug**

`nextcid = cohort + numGeneTypes; if (cohort < num_cohorts)` where `num_cohorts = numCohortsXnumGenes - 1`. With numGeneTypes > 1, the cohorts in the oldest age class (e.g. cohort = NxG-2) pass this test, so `nextcid >= numCohortsXnumGenes` and MIGRATION DEN/SN/RN/aging/cohort_migrating`[nextcid]` are read and written past the end (allocated with `cohort` rows, atBiologyXMLParamIO.c:4348-4380). This is reachable when flagsenesce is on, because the oldest cohorts are then not skipped (line 383). Those cohorts also miss the plus-group branch. Update_Aging_Numbers (2669) uses the correct test. Function: `Update_Migration_Array()`.

**Code at issue** — `atdemography.c` line 2710 (closest match to the quoted code; wording differs slightly)  
Search for: `int num_cohorts = (FunctGroupArray[species].numCohortsXnumGenes) - 1;`

```c
 2708: void Update_Migration_Array(MSEBoxModel *bm, int species, int cohort, int maxstock_id, int do_debug, FILE *llogfp) {
 2709: 	int spmigrate = MIGRATION[species].num_in_queue;
 2710→     int num_cohorts = (FunctGroupArray[species].numCohortsXnumGenes) - 1;
 2711:     int qid = 0;
 2712:     double p_ageup, oldden, oldSN, oldRN, newSN, newRN, num_aging, growth_period, sizeScalar;
```

**Code at issue** — `atdemography.c` line 2743  
Search for: `[nextcid]`

```c
 2741:                 }
 2742:                 
 2743→                 oldden = MIGRATION[species].DEN[nextcid][qid];
 2744:                 oldSN = MIGRATION[species].SN[nextcid][qid];
 2745:                 oldRN = MIGRATION[species].RN[nextcid][qid];
```

**Code at issue** — `atdemography.c` line 3749  
Search for: `Update_Migration_Array()`

```c
 3747:                          Do allocation of aging into Migration array only when appropriate
 3748:                           - i.e. juveniles in model domain and adults not. If both outside model
 3749→                           domain then code in Update_Migration_Array() will handle it. Also if
 3750:                           juveniles are outside the domain and the adults are inside then this
 3751:                           is dealt with in Update_Migration_Array() too - see logic discussion
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
**Found at (current code):** `atdemography.c` 4459
**Lines at review (Oct 2026):** 4453-4460

**Function:** `Invertebrate_Reproduction()`

**Why it's thought to be a bug**

Semelparous case: `spawning_biomass_contrib = this_biom * (1.0 - scaled_FSPB[cohort]);`. In Update_Invert_Aging (5110) `spawning_biomass_contrib` is the biomass removed. The comment says the loss should be "all adults spawning". The code removes the immature fraction instead of the mature one, so a fully mature cohort (FSPB = 1) loses nothing and a partly mature cohort loses its non-spawners. This is the vertebrate survivor factor (Apply_Semelparous_Deaths 1098) used as a loss. Function: `Invertebrate_Reproduction()`.

**Code at issue** — `atdemography.c` line 4459 (closest match to the quoted code; wording differs slightly)  
Search for: `spawning_biomass_contrib = this_biom * (1.0 - FunctGroupArray[species].scaled_FSPB[cohort]`

```c
 4457: 				 */
 4458: 				if (spawning_biomass && (flagmother < 0)) {
 4459→ 					spawning_biomass_contrib = this_biom * (1.0 - FunctGroupArray[species].scaled_FSPB[cohort]);
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
**Found at (current code):** `atdemography.c` 4430
**Lines at review (Oct 2026):** 4341-4345, 4425

**Function:** `Invertebrate_Reproduction()`

**Why it's thought to be a bug**

`for (stk = 0; stk < maxstock_id + 1; stk++) tot_yoy[species][stk] = 0.0;` and `tot_yoy[species][maxstock_id] += ...`. `tot_yoy` is `Util_Alloc_Init_2D_Double(nstock, K_num_tot_sp)`, i.e. `[sp][K_num_stocks_per_sp]` (atbiolsetup.c:2931, nstock = K_num_stocks_per_sp at 2799). It has no extra "aggregate" slot. When a species has the maximum number of stocks, this writes into the next species' row, or past the end of the heap block for the last species. Function: `Invertebrate_Reproduction()`.

**Code at issue** — `atdemography.c` line 4430  
Search for: `tot_yoy[species][maxstock_id] +=`

```c
 4428: 					/* Update yoy sum - only if localised recruit as all other cases done in Store_Recruitment_Diagnostics() */
 4429: 					if (FunctGroupArray[species].recruitType) {
 4430→ 						tot_yoy[species][maxstock_id] += spawned_biomass;
 4431: 						tot_yoy[species][stock_id] += spawned_biomass;
 4432: 						bm->calcTrackedMort[species][0][stock_id][start_id] += spawned_biomass;  // TODO: Is this covered in general update anyways?
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
**Found at (current code):** `atdemography.c` 3768
**Lines at review (Oct 2026):** 3763

**Function:** `Ecology_Update_Vertebrate_Cohorts()`

**Why it's thought to be a bug**

`if (FunctGroupArray[species].agingVERT[cohort][k][DEN_id] > 0)`. Here `k` is left over from the stock loop at 3697-3703, so `k == maxstock`. agingVERT is `[ncxg][K_num_stocks_per_sp][3]` (atUtilGroupIO.c:837), so this either reads one row out of bounds or reads an unused stock slot (always 0). As a result, juveniles are never pushed into the MIGRATION array. Function: `Ecology_Update_Vertebrate_Cohorts()`.

**Code at issue** — `atdemography.c` line 3768  
Search for: `if (FunctGroupArray[species].agingVERT[cohort][k][DEN_id] > 0)`

```c
 3766:                                 
 3767:                                     // #TODO: Do we need to use MIGRATION[species].all_go[cohort][qid] here?
 3768→                                     if (FunctGroupArray[species].agingVERT[cohort][k][DEN_id] > 0) {
 3769:                                         warn("Juveniles have to suddenly flee domain as age for %s (time: %e)\n", FunctGroupArray[species].groupCode, bm->dayt);  // Warngin as doesn't really make sense to have to do this
 3770:                                     
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
**Found at (current code):** `atdemography.c` 3722, `atdemography.c` 3757
**Lines at review (Oct 2026):** 3717-3718, 3752-3753

**Function:** `Ecology_Update_Vertebrate_Cohorts()`

**Why it's thought to be a bug**

`cid = floor(cohort / numGeneTypes); nextcid = cid + numGeneTypes;` combines an age index with the genotype count. The next age class for this cohort is `cohort + numGeneTypes`. The test `(cohort < age_mat) && (nextcid >= age_mat)` also compares the cohort×gene index with age_mat, which is in age classes. Both are only correct when numGeneTypes = 1. With more genotypes, the wrong cohorts trigger the juvenile-to-migration transfer and the wrong box-0 tracers are overwritten (3797-3805). Function: `Ecology_Update_Vertebrate_Cohorts()`.

**Code at issue** — `atdemography.c` line 3722 (closest match to the quoted code; wording differs slightly)  
Search for: `cid = (int)(floor(cohort / FunctGroupArray[species].numGeneTypes));`

```c
 3720: 						avgrn = 0;
 3721: 						MIGRATION[species].totprop_mig[cohort] = 0.0;
 3722→ 						cid = (int)(floor(cohort / FunctGroupArray[species].numGeneTypes));
 3723: 						nextcid = cid + FunctGroupArray[species].numGeneTypes;
 3724:                         //stage = FunctGroupArray[species].cohort_stage[cohort];
```

**Code at issue** — `atdemography.c` line 3757  
Search for: `(cohort < age_mat) && (nextcid >= age_mat)`

```c
 3755:                          
 3756:                          */
 3757→                         if ((cohort < age_mat) && (nextcid >= age_mat)) {
 3758:                             if ((MIGRATION[species].totprop_mig[cohort] < bm->juv_transition_thresh) && (MIGRATION[species].totprop_mig[nextcid] > bm->mat_transition_thresh)) {
 3759:                                 for (qid = MIGRATION[species].num_in_queue_done; qid < MIGRATION[species].num_in_queue; qid++) {
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
**Found at (current code):** `atdemography.c` 4641, `atdemography.c` 4456
**Lines at review (Oct 2026):** 4624-4649 (used 4353)

**Function:** `Ecology_Do_External_InVert_Spawning()`

**Why it's thought to be a bug**

`MigSpawn = flag_allow_spawn * spawn_active * prop_mat * MIGRATION[species].DEN[cohort][qid];` is assigned, not accumulated, inside the qid loop. Only the last migration-queue entry's spawn is returned, but DEN is still reduced for every entry. Function: `Ecology_Do_External_InVert_Spawning()`.

**Code at issue** — `atdemography.c` line 4641  
Search for: `MigSpawn = flag_allow_spawn * spawn_active * prop_mat * MIGRATION[species].DEN[cohort][qid]`

```c
 4639: 		for (qid = MIGRATION[species].num_in_queue_done; qid < MIGRATION[species].num_in_queue; qid++) {
 4640:             flag_allow_spawn = (double)(MIGRATION[species].ReprodAllowed[cohort][qid]);
 4641→ 			MigSpawn = flag_allow_spawn * spawn_active * prop_mat * MIGRATION[species].DEN[cohort][qid]; // In this case DEN is the total biomass of the migrating group
 4642: 
 4643: 			/* Update biomass left in migrating cohort */
```

**Code at issue** — `atdemography.c` line 4456  
Search for: `Ecology_Do_External_InVert_Spawning()`

```c
 4454: 				/* If semelparous reproduction - update biomass to be lost (i.e. all adults spawning rather than just spawn produced)
 4455: 				 * TODO: May need to distinguish when only external stocks reproduce not internal stocks, in which case don't need to
 4456→ 				 * do this step as already sorted in Ecology_Do_External_InVert_Spawning()
 4457: 				 */
 4458: 				if (spawning_biomass && (flagmother < 0)) {
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
**Found at (current code):** `atdemography.c` 4646, `atdemography.c` 4456
**Lines at review (Oct 2026):** 4639-4641

**Function:** `Ecology_Do_External_InVert_Spawning()`

**Why it's thought to be a bug**

Semelparous branch: `MIGRATION[species].DEN[cohort][qid] *= (1.0 - spawn_active);` ignores both `flag_allow_spawn` (ReprodAllowed) and `prop_mat`. Migrants that are not allowed to reproduce, or are immature, are still killed in proportion to FSP. Function: `Ecology_Do_External_InVert_Spawning()`.

**Code at issue** — `atdemography.c` line 4646  
Search for: `MIGRATION[species].DEN[cohort][qid] *= (1.0 - spawn_active)`

```c
 4644: 			if (flagmother < 0) {
 4645: 				// If semelparous then all that spawned die
 4646→ 				MIGRATION[species].DEN[cohort][qid] *= (1.0 - spawn_active);
 4647: 			} else {
 4648: 				// Only matter spawned lost
```

**Code at issue** — `atdemography.c` line 4456  
Search for: `Ecology_Do_External_InVert_Spawning()`

```c
 4454: 				/* If semelparous reproduction - update biomass to be lost (i.e. all adults spawning rather than just spawn produced)
 4455: 				 * TODO: May need to distinguish when only external stocks reproduce not internal stocks, in which case don't need to
 4456→ 				 * do this step as already sorted in Ecology_Do_External_InVert_Spawning()
 4457: 				 */
 4458: 				if (spawning_biomass && (flagmother < 0)) {
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
**Found at (current code):** `atdemography.c` 4377, `atdemography.c` 4430, `atdemography.c` 4947
**Lines at review (Oct 2026):** 4376-4428, 4885-4956

**Why it's thought to be a bug**

The recruitment block (`overall_checkday == sp_spawnday + additional_day`) runs inside the per-cohort loop with no cohort guard. `Ecology_Find_Invert_Recruits`, `Find_Final_Invert_Recruit_Distribtuion`, `tot_yoy += spawned_biomass` and `calcTrackedMort[..][start_id] +=` therefore run once per cohort×gene. In Find_Final, `totrecruit += ...` runs on every call, and for `external_recruit` so does `MIGRATION.DEN[ngene][mid] += hab_scalar*spawned_biomass`. Recruits are therefore added numCohortsXnumGenes times (and numGeneTypes more times, see DEMO-10). Functions: `Invertebrate_Reproduction()`, `Find_Final_Invert_Recruit_Distribtuion()`.

**Code at issue** — `atdemography.c` line 4377 (closest match to the quoted code; wording differs slightly)  
Search for: `fprintf(llogfp, "%s-%d box: %d-%d, overall_checkday: %d, sp_spawnday: %d, additional_day: `

```c
 4375:                 /*
 4376: 				if (do_debug && (bm->which_check == species))
 4377→ 					fprintf(llogfp, "%s-%d box: %d-%d, overall_checkday: %d, sp_spawnday: %d, additional_day: %d\n", FunctGroupArray[species].groupCode, cohort, bm->current_box, bm->current_layer, overall_checkday, sp_spawnday, additional_day);
 4378:                  */
 4379:                 
```

**Code at issue** — `atdemography.c` line 4430 (closest match to the quoted code; wording differs slightly)  
Search for: `tot_yoy[species][maxstock_id] += spawned_biomass;`

```c
 4428: 					/* Update yoy sum - only if localised recruit as all other cases done in Store_Recruitment_Diagnostics() */
 4429: 					if (FunctGroupArray[species].recruitType) {
 4430→ 						tot_yoy[species][maxstock_id] += spawned_biomass;
 4431: 						tot_yoy[species][stock_id] += spawned_biomass;
 4432: 						bm->calcTrackedMort[species][0][stock_id][start_id] += spawned_biomass;  // TODO: Is this covered in general update anyways?
```

**Code at issue** — `atdemography.c` line 4947 (closest match to the quoted code; wording differs slightly)  
Search for: `MIGRATION[species].DEN[ngene][mid] += hab_scalar * spawned_biomass;`

```c
 4945: 					}
 4946: 					if (recruit_outside) {
 4947→ 						MIGRATION[species].DEN[ngene][mid] += hab_scalar * spawned_biomass;
 4948: 						MIGRATION[species].recruit[ngene][mid] = hab_scalar * spawned_biomass;
 4949:                     } else {
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
**Found at (current code):** `atdemography.c` 4898
**Lines at review (Oct 2026):** 4885, 4893

**Function:** `Find_Final_Invert_Recruit_Distribtuion()`

**Why it's thought to be a bug**

The `ngene` parameter is immediately overwritten by `for (ngene = 0; ngene < numGeneTypes; ngene++)`. The `spawned_biomass` computed for one genotype (4383) is therefore written to every genotype's num_recruits/totrecruit/MIGRATION entries. With numGeneTypes > 1, the last caller's value wins, or values accumulate with `+=`. Function: `Find_Final_Invert_Recruit_Distribtuion()`.

**Code at issue** — `atdemography.c` line 4898 (closest match to the quoted code; wording differs slightly)  
Search for: `for( ngene = 0; ngene < FunctGroupArray[species].numGeneTypes; ngene++){`

```c
 4896: 	double hab_scalar = Get_Habitat_Mediated_Recruit_Scalar(bm, species, llogfp);
 4897: 
 4898→ 	for( ngene = 0; ngene < FunctGroupArray[species].numGeneTypes; ngene++){
 4899: 		switch (FunctGroupArray[species].recruitType) {
 4900: 		case independent_recruit_distrib:
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
**Found at (current code):** `atdemography.c` 4337
**Lines at review (Oct 2026):** 4302-4322, 4499-4503

**Function:** `Invertebrate_Reproduction()`

**Why it's thought to be a bug**

`sp_Spawn_Now` and `sp_Age_Now` are set to 1 when any cohort matches its Spawn_Now/Age_Now day and are never reset per cohort. Every later cohort in the loop is then treated as spawning and aging, so `Get_Proportion_Aging` and aging shifts are applied to cohorts not due to age today. The second loop inherits the value too. Function: `Invertebrate_Reproduction()`.

**Code at issue** — `atdemography.c` line 4337 (located by name only; check this is the right place)  
Search for: `overall_checkday, FunctGroupArray[species].groupCode, cohort, bm->current_box, bm->current`

```c
 4335: 				if (do_debug && (bm->which_check == species))
 4336: 					fprintf(llogfp, "day %d, %s-%d box: %d-%d starting_biom: %e, sp_Spawn_Now = %d, sp_Age_Now: %d, spawn_day: %d, age_day: %d, overall_checkday: %d, sp_spawnday: %d, additional_day: %d, stuff_to_do: %d\n", 
 4337→ 						overall_checkday, FunctGroupArray[species].groupCode, cohort, bm->current_box, bm->current_layer, this_biom, sp_Spawn_Now, sp_Age_Now, EMBRYO[species].Spawn_Now[cohort][EMBRYO[species].next_spawn], EMBRYO[species].Age_Now[cohort][EMBRYO[species].next_age], overall_checkday, sp_spawnday, additional_day, stuff_to_do);
 4338:                 */
 4339:                 
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
**Found at (current code):** `atdemography.c` 1034, `atdemography.c` 1018
**Lines at review (Oct 2026):** 1004-1033 (used 469, 517, 2671)

**Function:** `Ecology_Do_Internal_Age_Structured_Spawning()`

**Why it's thought to be a bug**

`EMBRYO[species].IndSpawn[cohort]` is set only for cohorts that actually spawn. A cohort that is skipped (`Spawn_Now != today` or `li < min_li_mat`) keeps the IndSpawn value from the previous box on the same spawn day; it is only zeroed on non-spawn days (271-273). Since li depends on the local SN+RN, a cohort can spawn in box A and not in box B. In box B, `Update_Local_dPool` still subtracts the stale IndSpawn from RN (2927), and it also affects aging RN (2675) and recruit RN (469). Function: `Ecology_Do_Internal_Age_Structured_Spawning()`.

**Code at issue** — `atdemography.c` line 1034  
Search for: `EMBRYO[species].IndSpawn[cohort]`

```c
 1032:         /* Animals in this box spawn */
 1033: 		step1 = Ecology_Age_Structured_Spawn(species, KSPA_sp, FSP_sp, RSprop_sp, VERTinfo[species][cohort][SN_id], VERTinfo[species][cohort][RN_id], FunctGroupArray[species].scaled_FSPB[cohort], FunctGroupArray[species].X_RS[cohort], bm->flag_repcostSpawn, llogfp);
 1034→ 		EMBRYO[species].IndSpawn[cohort] = FunctGroupArray[species].scaled_FSPB[cohort] * step1;
 1035:         
 1036:         thisSSB = (VERTinfo[species][cohort][RN_id] + VERTinfo[species][cohort][SN_id]) * VERTinfo[species][cohort][DEN_id];
```

**Code at issue** — `atdemography.c` line 1018 (closest match to the quoted code; wording differs slightly)  
Search for: `if (li < FunctGroupArray[species].speciesParams[min_li_mat_id]) {`

```c
 1016: 		Wgt = VERTinfo[species][cohort][SN_id] + VERTinfo[species][cohort][RN_id];
 1017: 		li = Ecology_Get_Size(bm, species, Wgt, cohort);
 1018→ 		if (li < FunctGroupArray[species].speciesParams[min_li_mat_id]) {
 1019: 			allowed_to_spawn = 0;
 1020: 		}
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
**Found at (current code):** `atdemography.c` 928
**Lines at review (Oct 2026):** 921-934

**Function:** `Ecology_Do_External_Age_Structured_Spawning()`

**Why it's thought to be a bug**

`allowed_to_spawn` is set to 0 inside the `mid` loop when one migrant group is too small (`li < min_li_mat`) and is never reset to 1 for the next `mid`. All later migration-queue entries for that cohort are then skipped. Function: `Ecology_Do_External_Age_Structured_Spawning()`.

**Code at issue** — `atdemography.c` line 928 (closest match to the quoted code; wording differs slightly)  
Search for: `if (li < FunctGroupArray[species].speciesParams[min_li_mat_id]) {`

```c
  926:                     Wgt = MIGRATION[species].SN[cohort][mid] + MIGRATION[species].RN[cohort][mid];
  927:                     li = Ecology_Get_Size(bm, species, Wgt, cohort);
  928→                     if (li < FunctGroupArray[species].speciesParams[min_li_mat_id]) {
  929:                         allowed_to_spawn = 0;
  930:                     }
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
**Found at (current code):** `atdemography.c` 1079
**Lines at review (Oct 2026):** 1077-1108

**Function:** `Apply_Semelparous_Deaths()`

**Why it's thought to be a bug**

The death factor `(1 - scaled_FSPB[cohort])` is applied to every cohort with FSPB > 0 whenever any cohort spawned (`semelparous_parents`/`_migrants` are species-level flags). Cohorts whose Spawn_Now was not today, or which failed the size test, still die. It also writes unconditional `fprintf` lines for every cohort, box and spawn event. Function: `Apply_Semelparous_Deaths()`.

**Code at issue** — `atdemography.c` line 1079 (closest match to the quoted code; wording differs slightly)  
Search for: `if(FunctGroupArray[species].scaled_FSPB[cohort] > 0.0) {`

```c
 1077:     
 1078:     for (cohort = ((FunctGroupArray[species].numCohortsXnumGenes) - 1); cohort >= 0; cohort--) {
 1079→         if(FunctGroupArray[species].scaled_FSPB[cohort] > 0.0) {
 1080:             /* If semelparous external reproduction */
 1081:             if (semelparous_migrants) {
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
**Found at (current code):** `atdemography.c` 1952, `atdemography.c` 1885, `atdemography.c` 1906
**Lines at review (Oct 2026):** 1904-1958

**Function:** `Find_Final_Recruit_Distribution()`

**Why it's thought to be a bug**

`mid = migIDmatch[qid]; if (mid < 0) recruit_outside = 0; ... else { MIGRATION[species].recruit[ngene][mid] = 0; ...}`. The else branch runs exactly when `mid < 0`, so it writes `recruit[ngene][-1]` (heap corruption). In that same branch, `totrecruit += ...` (1954) is then wiped by the unconditional `totrecruit[...] = 0.0` (1963). Also, for external_recruit, AlignVsMigration never sets -1 (atannualbiology.c:956-963) and migIDmatch defaults to 0, so a missing match quietly sends recruits to queue entry 0. Function: `Find_Final_Recruit_Distribution()`.

**Code at issue** — `atdemography.c` line 1952  
Search for: `} else {`

```c
 1950:                         **/
 1951:                     }
 1952→                 } else {
 1953:                     MIGRATION[species].recruit[ngene][mid] = 0;
 1954:                     EMBRYO[species].num_recruits[bm->current_box][wclayer][ngene][qid] = EMBRYO[species].BulkRecruits[ngene] * enviro_scalar * vertdistrib * bm->recruit_hdistrib[ngene][bm->current_box][species];
```

**Code at issue** — `atdemography.c` line 1885 (closest match to the quoted code; wording differs slightly)  
Search for: `"Final recruit distrib Time: %e, box%d-%d, species %s ngene: %d, recruit_Type: %d, BulkRec`

```c
 1883:         if (do_debug && (bm->which_check == species)) {
 1884: 			fprintf( llogfp,
 1885→ 					"Final recruit distrib Time: %e, box%d-%d, species %s ngene: %d, recruit_Type: %d, BulkRecruits: %e, enviro_scalar: %e, hab_scalar: %e, vertdistrib: %e, recruit_hdistrib[box%d][%s]: %e, recryut_type: %d (EMBRYO[%s][%d]: %e)\n",
 1886: 					bm->dayt, bm->current_box, bm->current_layer, FunctGroupArray[species].groupCode, ngene,FunctGroupArray[species].recruitType, EMBRYO[species].BulkRecruits[ngene],
 1887: 					enviro_scalar, hab_scalar, vertdistrib, bm->current_box, FunctGroupArray[species].groupCode, bm->recruit_hdistrib[ngene][bm->current_box][species],
```

**Code at issue** — `atdemography.c` line 1906  
Search for: `totrecruit[`

```c
 1904:             //if (do_debug && (species == bm->which_check)) {
 1905:             if ((species == 73) || (species == 68) || (species == 66)) {
 1906→                 fprintf(llogfp,"Time: %e, %s box%d-%d ngene: %d, qid: %d, totrecruit: %e num_rec: %e (Bulkrecruits: %e, enviro_scalar: %e, vdistrib: %e, hdistrib: %e)\n", bm->dayt, FunctGroupArray[species].groupCode, bm->current_box, wclayer, ngene, qid, totrecruit[species][stock_id][ngene], EMBRYO[species].num_recruits[bm->current_box][wclayer][ngene][qid], EMBRYO[species].BulkRecruits[ngene], enviro_scalar, vertdistrib, bm->recruit_hdistrib[ngene][bm->current_box][species]);
 1907:             }
 1908:             /**/
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
**Found at (current code):** `atdemography.c` 531, `atdemography.c` 525, `atdemography.c` 3402
**Lines at review (Oct 2026):** 523-532

**Function:** `Vertebrate_Reproduction()`

**Why it's thought to be a bug**

The comment says that if general aging happens here too, this step should be skipped. But `need_update > -1` tests the local copy read at 522, before `Update_Age_Distrib(...,0)` sets `speciesParams[need_update_id] = -1` (3400-3401). The test is always true (0 or 1), so the recruit-related update also runs. It then overwrites the youngest cohorts' tempPopRatio/boxPopRatio with `recVERTpopratio`, which was not refreshed this step: when need_update = 1, Get_Latest_Additions takes the tempPopRatio branch (2850). Function: `Vertebrate_Reproduction()`.

> 🔀 **Changed by the merge of main (Nov 2026):** Update_Age_Distrib b-loop / [b] index (A-02). Check whether this finding is now fixed before acting.

**Code at issue** — `atdemography.c` line 531  
Search for: `need_update > -1`

```c
  529:                  aging occuring here too then skip this step as update already done */
  530:                 need_recupdate = (int) (FunctGroupArray[species].speciesParams[need_recupdate_id]);
  531→                 if (bm->flagagestruct && (need_update > -1) && need_recupdate && (((bm->current_box == bm->last_box) && !bm->current_layer)
  532:                         || ((sp_ddepend_move == 1) && !bm->current_layer))) {
  533: 
```

**Code at issue** — `atdemography.c` line 525  
Search for: `Update_Age_Distrib(`

```c
  523:                 if (bm->flagagestruct && need_update && (((bm->current_box == bm->last_box) && (!bm->current_layer))
  524:                         || ((sp_ddepend_move == 1) && !bm->current_layer))) {
  525→                     Update_Age_Distrib(bm, species, stock_id, sp_ddepend_move, 0, llogfp);  // 0 flag to indicate general aging
  526:                 }
  527: 
```

**Code at issue** — `atdemography.c` line 3402  
Search for: `speciesParams[need_update_id] = -1`

```c
 3400: 	 done age distribution recalculation */
 3401: 	if (need_recupdate)
 3402→ 		FunctGroupArray[species].speciesParams[need_update_id] = -1;
 3403: 	else
 3404: 		FunctGroupArray[species].speciesParams[need_update_id] = 0;
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
**Found at (current code):** `atdemography.c` 3166
**Lines at review (Oct 2026):** 3157-3159, 3290-3292

**Function:** `Update_Age_Distrib()`

**Why it's thought to be a bug**

`do_test = 0; if(rec_related) do_test = 1; else if(cohort || (!cohort && k)) do_test = 1;` is evaluated outside the `for (k...)` loop, where `k == classize` from the previous loop. The test is therefore always true, and the intended exemption for cohort 0 / sub-class 0 never applies: the minimum floor `maxvalue/sp_recruitRange` is forced on it. Update_Invert_Cohorts (5218) evaluates the same test correctly inside the k loop. Function: `Update_Age_Distrib()`.

**Code at issue** — `atdemography.c` line 3166  
Search for: `maxvalue / sp_recruitRange`

```c
 3164: 				else if(cohort || (!cohort && k)) do_test = 1;
 3165: 				for (k = 0; k < classize; k++) {
 3166→ 					if (do_test && (bm->tempPopRatio[stock_id][species][cohort][k] < (maxvalue / sp_recruitRange))) {
 3167: 						bm->tempPopRatio[stock_id][species][cohort][k] = maxvalue / sp_recruitRange;
 3168: 					}
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
**Found at (current code):** `atdemography.c` 3105, `atdemography.c` 3109
**Lines at review (Oct 2026):** 3081-3114

**Function:** `Update_Age_Distrib()`

**Why it's thought to be a bug**

`stock_done[s_id]` is reset once per call (2996) but is shared across all `cohort` and `qid` iterations. After the first cohort marks a stock done, no later cohort or queue entry updates tempPopRatio for that stock. The outer `for (i < maxstock_id)` loop is also redundant. In the same block, the `recVERTpopratio[stock_id]...` update (3108) uses the function argument `stock_id` instead of `s_id`. Function: `Update_Age_Distrib()`.

**Code at issue** — `atdemography.c` line 3105  
Search for: `stock_done[s_id]`

```c
 3103: 										s_id = bm->group_stock[species][ij][k];
 3104: 
 3105→ 										if(!stock_done[s_id]){
 3106: 											for (b = 1; b < classize; b++) {
 3107: 												bm->tempPopRatio[s_id][species][cohort][b] += (FunctGroupArray[species].boxPopRatio[ij][k][cohort][b-1] * MIGRATION[species].DEN[cohort][qid]);
```

**Code at issue** — `atdemography.c` line 3109  
Search for: `recVERTpopratio[stock_id]`

```c
 3107: 												bm->tempPopRatio[s_id][species][cohort][b] += (FunctGroupArray[species].boxPopRatio[ij][k][cohort][b-1] * MIGRATION[species].DEN[cohort][qid]);
 3108: 												if(rec_related)
 3109→ 													recVERTpopratio[stock_id][species][cohort][b] += (FunctGroupArray[species].boxPopRatio[ij][k][cohort][b-1] * MIGRATION[species].DEN[cohort][qid]);
 3110: 
 3111: 												if((!_finite(FunctGroupArray[species].boxPopRatio[ij][k][cohort][b])) || (!_finite(bm->tempPopRatio[s_id][species][cohort][b]))){
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
**Found at (current code):** `atdemography.c` 5266, `atdemography.c` 5269
**Lines at review (Oct 2026):** 5259-5268, 5288-5289

**Function:** `Get_Habitat_Mediated_Recruit_Scalar()`

**Why it's thought to be a bug**

The loop runs over `sp` but the body uses `species` throughout (`FunctGroupArray[species].isCover`, `.coverID`). The same term is added K_num_tot_sp times if the recruiting species is itself a cover group; otherwise nothing is added. `isCover == CORAL || isCover == SPONGE` compares a TRUE/FALSE flag to group-type constants, so `like_coral` is effectively never set; it should test `groupType`. `hab_scalar` also starts at 1.0 and is capped at 1.0, so positive habitat preferences can never raise it. Function: `Get_Habitat_Mediated_Recruit_Scalar()`.

**Code at issue** — `atdemography.c` line 5266  
Search for: `FunctGroupArray[species].isCover`

```c
 5264: 	if (flag_recruit_habdepend){
 5265: 		for(sp = 0; sp < bm->K_num_tot_sp; sp++){
 5266→ 			if (FunctGroupArray[species].isCover == TRUE) {
 5267: 				sp_like = bm->HABITATlike[species][rec_stage][FunctGroupArray[species].coverID];
 5268: 				hab_scalar += sp_like * bm->coveramt[FunctGroupArray[species].coverID][bm->current_box];   // This covers larvae liking turf etc and no bare sediments
```

**Code at issue** — `atdemography.c` line 5269 (closest match to the quoted code; wording differs slightly)  
Search for: `if((FunctGroupArray[species].isCover == CORAL) || (FunctGroupArray[species].isCover == SPO`

```c
 5267: 				sp_like = bm->HABITATlike[species][rec_stage][FunctGroupArray[species].coverID];
 5268: 				hab_scalar += sp_like * bm->coveramt[FunctGroupArray[species].coverID][bm->current_box];   // This covers larvae liking turf etc and no bare sediments
 5269→ 				if((FunctGroupArray[species].isCover == CORAL) || (FunctGroupArray[species].isCover == SPONGE)) {
 5270: 					like_coral = 1;
 5271: 				}
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
**Found at (current code):** `atdemography.c` 2074
**Lines at review (Oct 2026):** 2064-2070

**Function:** `Sanity_Check_Recruits()`

**Why it's thought to be a bug**

`for (n = 0; n < numCohorts; n++) tot_adult_den += scaled_FSPB[n] * tr[k][NumsTracers[n]];`. scaled_FSPB and NumsTracers are indexed by cohort×gene. With numGeneTypes > 1, this sums the wrong (young) entries and misses the adults, so the larvae-per-adult trim is wrong. Function: `Sanity_Check_Recruits()`.

**Code at issue** — `atdemography.c` line 2074 (closest match to the quoted code; wording differs slightly)  
Search for: `tot_adult_den += FunctGroupArray[species].scaled_FSPB[n] * bm->boxes[ij].tr[k][FunctGroupA`

```c
 2072:                 for (k = 0; k < bm->boxes[ij].nz; k++) {
 2073:                     for (n = 0; n < FunctGroupArray[species].numCohorts; n++) {
 2074→                         tot_adult_den += FunctGroupArray[species].scaled_FSPB[n] * bm->boxes[ij].tr[k][FunctGroupArray[species].NumsTracers[n]];
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
**Found at (current code):** `atdemography.c` 2128, `atdemography.c` 4428
**Lines at review (Oct 2026):** 2122-2126

**Function:** `Store_Recruitment_Diagnostics()`

**Why it's thought to be a bug**

`tot_yoy[species][i] = (...)` sits inside the `ngene` loop, so only the last genotype's YOY is kept (the migrant add at 2141 is `+=`). This affects the YOY diagnostic output (atbiolmessage.c:1631). Function: `Store_Recruitment_Diagnostics()`.

**Code at issue** — `atdemography.c` line 2128  
Search for: `tot_yoy[species][i] = (`

```c
 2126: 			yoy[ngene] += EMBRYO[species].Larvae[i][ngene][qid];
 2127: 			if (use_aggregate) {
 2128→                 tot_yoy[species][i] = (totrecruit[species][i][ngene] * (KWRR_sp + KWSR_sp));
 2129: 			} else {
 2130: 				tot_yoy[species][i] = (EMBRYO[species].Larvae[i][ngene][qid] * (KWRR_sp + KWSR_sp));
```

**Code at issue** — `atdemography.c` line 4428  
Search for: `Store_Recruitment_Diagnostics()`

```c
 4426: 					Find_Final_Invert_Recruit_Distribtuion(bm, species, ngene, qid, stock_id, spawned_biomass, wclayer, llogfp);
 4427: 
 4428→ 					/* Update yoy sum - only if localised recruit as all other cases done in Store_Recruitment_Diagnostics() */
 4429: 					if (FunctGroupArray[species].recruitType) {
 4430: 						tot_yoy[species][maxstock_id] += spawned_biomass;
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
**Found at (current code):** `atdemography.c` 2248, `atdemography.c` 2201, `atdemography.c` 4098
**Lines at review (Oct 2026):** 2237-2254, 2284-2285

**Function:** `Store_Recruitment_Diagnostics()`

**Why it's thought to be a bug**

`if (fished_chrt == ngene) biomass += tr[k][den]; ... biomass += tr[k][den];` counts the fished cohort twice and counts every youngest-cohort density plus larvae. `calcNstart[expect_id]` is then set to this value unconditionally (2288). Reset_Mortality (4166-4167, 4205) instead defines calcNstart as the fished cohort's numbers only. The two routines overwrite each other's pseudo-assessment starting N with different quantities. The history reset at 2167 is correctly limited to `fished_chrt < numGeneTypes`, but the final assignment is not. The non-AGE_STRUCTURED branch (2261) also does not skip BOUNDARY boxes, unlike the branch above it. Function: `Store_Recruitment_Diagnostics()`.

**Code at issue** — `atdemography.c` line 2248 (closest match to the quoted code; wording differs slightly)  
Search for: `biomass += bm->boxes[i].tr[k][den];`

```c
 2246: 					for (k = 0; k < bm->boxes[i].nz; k++) {
 2247: 						if (fished_chrt == ngene )
 2248→ 							biomass += bm->boxes[i].tr[k][den];
 2249: 						stock_id = bm->group_stock[species][i][k];
 2250: 
```

**Code at issue** — `atdemography.c` line 2201 (closest match to the quoted code; wording differs slightly)  
Search for: `bm->calcNstart[species][expect_id] = 0;`

```c
 2199: 
 2200: 		/* Set starting cohort size for pseduo assessment - Zero for update below (otherwise done in recruitment) */
 2201→ 		bm->calcNstart[species][expect_id] = 0;
 2202: 
 2203: 		//fprintf(llogfp, "%s set startN to %e\n", FunctGroupArray[species].groupCode, yoysum);
```

**Code at issue** — `atdemography.c` line 4098 (closest match to the quoted code; wording differs slightly)  
Search for: `if (fished_chrt >= FunctGroupArray[species].numGeneTypes) {`

```c
 4096: 		fished_chrt = ((FunctGroupArray[species].numCohortsXnumGenes) / 2);
 4097: 
 4098→ 	if (fished_chrt >= FunctGroupArray[species].numGeneTypes) {
 4099: 		/* Store last year's records in case they're needed in the assessment */
 4100: 		bm->calcMnum[species][hist_id] = bm->calcMnum[species][expect_id];
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
**Found at (current code):** `atdemography.c` 2544, `atdemography.c` 2687, `atdemography.c` 2552
**Lines at review (Oct 2026):** 2550-2581 (used 389-392, 2682)

**Function:** `Get_Numbers_Aging()`

**Why it's thought to be a bug**

In the evolution branch, `*this_p_ageup` is never set. The caller's `this_prop_ageup` (declared without an initial value at 116) is then passed to `Update_Aging_Numbers` and on to `Age_Contaminants_Store(..., this_p_ageup)` when contaminants are tracked. Line 2551 `ng = cohort - basecohort;` is also wrong (missing `* sp_numGeneTypes`), but it is overwritten before use. Function: `Get_Numbers_Aging()`.

**Code at issue** — `atdemography.c` line 2544  
Search for: `*this_p_ageup`

```c
 2542:  * \brief Get the numbers shifting age classes
 2543:  */
 2544→ double Get_Numbers_Aging(MSEBoxModel *bm, int species, int cohort, int *active_den, double *lostden_zero, double *this_p_ageup, int do_debug, FILE *llogfp) {
 2545: 	double p_ageup = 0;
 2546: 	double dennow = 0.0;
```

**Code at issue** — `atdemography.c` line 2687  
Search for: `Age_Contaminants_Store(`

```c
 2685:         
 2686:         if(bm->track_contaminants){
 2687→             Age_Contaminants_Store(bm, species, cohort, nextcid, dennow, this_p_ageup);
 2688:         }
 2689: 	}
```

**Code at issue** — `atdemography.c` line 2552  
Search for: `ng = cohort - basecohort`

```c
 2550: 	int sp_numGeneTypes = (int)(FunctGroupArray[species].numGeneTypes);
 2551: 	int basecohort = (int)floor (cohort / sp_numGeneTypes);
 2552→ 	ng = cohort - basecohort;
 2553: 
 2554: 	/* Find numbers shifting and shift numbers lost - ones added done in aging update */
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
**Found at (current code):** `atdemography.c` 1783, `atdemography.c` 1796, `atdemography.c` 1747
**Lines at review (Oct 2026):** 1742-1826

**Function:** `Recruit_Migration()`

**Why it's thought to be a bug**

`direct_recruit_entry` is only ever set to 1, inside the same `if` that sets `recruit_outside = 1` (1782-1787). So `if (recruit_outside && (!direct_recruit_entry))` (1796) can never be true and the whole MIGRATION add block is dead. In practice delivery happens in atmovement.c:1295. Line 1755 also writes an unconditional `fprintf(bm->logFile, ...)` for every gene × queue entry on every recruiting call. Function: `Recruit_Migration()`.

**Code at issue** — `atdemography.c` line 1783  
Search for: `recruit_outside = 1`

```c
 1781:                         /* Outside the model when recruitment happens */
 1782:                         if ((MIGRATION[species].Return_Now[mid] >= thisday) && (MIGRATION[species].Leave_Now[mid] <= thisday)) {
 1783→                             recruit_outside = 1;
 1784:                             FunctGroupArray[species].speciesParams[recruit_outside_id] = 1;
 1785:                             FunctGroupArray[species].speciesParams[direct_recruit_entry_id] = 0;
```

**Code at issue** — `atdemography.c` line 1796  
Search for: `if (recruit_outside && (!direct_recruit_entry))`

```c
 1794:                         **/
 1795:                     
 1796→                         if (recruit_outside && (!direct_recruit_entry)) {
 1797:                             oldDEN = MIGRATION[species].DEN[ngene][mid];
 1798:                             oldSN = MIGRATION[species].SN[ngene][mid];
```

**Code at issue** — `atdemography.c` line 1747  
Search for: `fprintf(bm->logFile,`

```c
 1745:     //int cohort_recruit_entry = FunctGroupArray[species].speciesParams[cohort_recruit_entry_id];
 1746:     
 1747→     //fprintf(bm->logFile, "Time: %e %s box%d-%d (vs first_box %d, top_layer %d) numGeneTypes: %d Recruit_Migration recruitType: %d num_in_queue: %d externalReproducer: %d\n",  bm->dayt, FunctGroupArray[species].groupCode, bm->current_box, bm->current_layer, bm->first_box, bm->top_layer, sp_numGeneTypes, FunctGroupArray[species].recruitType, MIGRATION[species].num_in_queue, FunctGroupArray[species].externalReproducer);
 1748: 
 1749:     if((FunctGroupArray[species].externalReproducer && (FunctGroupArray[species].recruitType != external_spawn_local_recruit)) || (FunctGroupArray[species].recruitType == external_recruit)) {
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
**Found at (current code):** `atdemography.c` 2853, `atdemography.c` 2856
**Lines at review (Oct 2026):** 2848-2853, 2872-2874, 2883-2885

**Function:** `Get_Latest_Additions()`

**Why it's thought to be a bug**

The debug check `bm->tempPopRatio[stock_id][ngene][cohort][0]` swaps dimensions: it should be `[stock_id][species][ngene][0]`, because tempPopRatio is `[nstock][sp][ncxg][maxspage]` (atbiolsetup.c:2895). In all three fprintfs the format `"%s-%d in Box%d-%d (ngene %d)"` is given `cohort, ngene, box, layer`, so the labels are misaligned. 2850 also prints every call for which_check without do_debug. Function: `Get_Latest_Additions()`.

> 🔀 **Changed by the merge of main (Nov 2026):** the code for this finding was changed by the merge. Check whether this finding is now fixed before acting.

**Code at issue** — `atdemography.c` line 2853 (closest match to the quoted code; wording differs slightly)  
Search for: `bm->tempPopRatio[stock_id][species][ngene][0] += coming_SPden[ngene]; //Used to say cohort`

```c
 2851: 	} else {
 2852: 		for ( ngene = 0; ngene < sp_numGeneTypes; ngene++ ){
 2853→ 			bm->tempPopRatio[stock_id][species][ngene][0] += coming_SPden[ngene]; //Used to say cohort for tempPopRatio
 2854: 
 2855: 			if((!_finite(bm->tempPopRatio[stock_id][species][ngene][0])) || (species == bm->which_check)){  // Nov 2026: main had [stock_id][ngene][cohort][0] (ngene in species slot)
```

**Code at issue** — `atdemography.c` line 2856 (closest match to the quoted code; wording differs slightly)  
Search for: `fprintf(llogfp,"Time: %e %s-%d in Box%d-%d (ngene %d) has tempPopRatio %e for stock %d sub`

```c
 2854: 
 2855: 			if((!_finite(bm->tempPopRatio[stock_id][species][ngene][0])) || (species == bm->which_check)){  // Nov 2026: main had [stock_id][ngene][cohort][0] (ngene in species slot)
 2856→ 				fprintf(llogfp,"Time: %e %s-%d in Box%d-%d (ngene %d) has tempPopRatio %e for stock %d subclass %d as dennow: %e\n",
 2857: 						bm->dayt, FunctGroupArray[species].groupCode, cohort, ngene, bm->current_box, bm->current_layer,
 2858: 						bm->tempPopRatio[stock_id][species][ngene][0], stock_id, 0, coming_SPden[ngene]);
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
**Found at (current code):** `atdemography.c` 2654, `atdemography.c` 2573
**Lines at review (Oct 2026):** 2649-2653

**Function:** `Update_Aging_Numbers()`

**Why it's thought to be a bug**

The check `!_finite(bm->tempPopRatio[stock_id][species][cohort][i])` uses `i == 0` after the loop and the wrong cohort; it should check `[nextcid][0]`, which is what is printed. The `|| (species == bm->which_check)` condition has no `do_debug`, so it prints on every aging event for the checked species. Function: `Update_Aging_Numbers()`.

**Code at issue** — `atdemography.c` line 2654  
Search for: `!_finite(bm->tempPopRatio[stock_id][species][cohort][i])`

```c
 2652: 			bm->tempPopRatio[stock_id][species][nextcid][0] += dennow;
 2653: 
 2654→ 			if((!_finite(bm->tempPopRatio[stock_id][species][cohort][i])) || (species == bm->which_check)){
 2655: 				fprintf(llogfp,"Time: %e %s-%d in Box%d-%d has tempPopRatio %e for stock %d subclass %d as dennow: %e\n",
 2656: 						bm->dayt, FunctGroupArray[species].groupCode, nextcid, bm->current_box, bm->current_layer,
```

**Code at issue** — `atdemography.c` line 2573  
Search for: `Update_Aging_Numbers()`

```c
 2571:         **/
 2572:         
 2573→ 		// Find number aging into the age-gene bin (well it will be age+1 but the +1 transition is done in Update_Aging_Numbers() to be consistent with non-evolution case
 2574: 		dennow = 0.0;
 2575: 		for (ng = 0; ng < FunctGroupArray[species].numGeneTypes; ng++){
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
**Found at (current code):** `atdemography.c` 1619
**Lines at review (Oct 2026):** 1610-1613

**Function:** `Get_Enviro_Recruit_Forcing()`

**Why it's thought to be a bug**

The warning says "...as flagtempdepend_move = %d" and passes `bm->flagtempdepend_move`, but the guarding flag is `bm->flagtempdepend_reprod`. The comment also says "movement". Function: `Get_Enviro_Recruit_Forcing()`.

**Code at issue** — `atdemography.c` line 1619  
Search for: `flagtempdepend_move`

```c
 1617:     if(!bm->flagtempdepend_reprod) { // Have turned off temperature dependent movement
 1618:         if(temp_sensitive_sp && bm->newmonth) {
 1619→             warn("Time: %e Zeroing temperature sensitivity for reproduction of %s even though flagtempsensitive %d as flagtempdepend_move = %d\n", bm->dayt, FunctGroupArray[species].groupCode, temp_sensitive_sp, bm->flagtempdepend_move);
 1620:         }
 1621:         temp_sensitive_sp = 0;
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
**Found at (current code):** `atdemography.c` 1649
**Lines at review (Oct 2026):** 1641-1645

**Function:** `Get_Enviro_Recruit_Forcing()`

**Why it's thought to be a bug**

`if (sp_q10receff == 1) enviro_scalar *= Tscalar; else if (sp_q10receff == 2) enviro_scalar *= Tscalar;` has identical branches. Either option 2 was meant to differ, or the branches should be merged. Function: `Get_Enviro_Recruit_Forcing()`.

**Code at issue** — `atdemography.c` line 1649 (the line the review pointed to, found by its text)  
Search for: `enviro_scalar *= Tscalar;`

```c
 1647: 
 1648: 		if (sp_q10receff == 1) {
 1649→ 			enviro_scalar *= Tscalar;
 1650: 		} else if (sp_q10receff == 2) {
 1651: 			enviro_scalar *= Tscalar;
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
**Found at (current code):** `atdemography.c` 5197, `atdemography.c` 5070
**Lines at review (Oct 2026):** 5192

**Function:** `Update_Invert_Cohorts()`

**Why it's thought to be a bug**

`fprintf(bm->logFile, "species %s, totsum = %e\n", ...)` is unconditional and runs for every cohort, every box with activity, and every call. The log grows large (compare the recent commit that removed a similar printf). Function: `Update_Invert_Cohorts()`.

**Code at issue** — `atdemography.c` line 5197  
Search for: `fprintf(bm->logFile, "species %s, totsum = %e\n",`

```c
 5195: 					//quit("%s-%d had ltotsum = 0\n", FunctGroupArray[species].groupCode, cohort);
 5196: 				}
 5197→ 				fprintf(bm->logFile, "species %s, totsum = %e\n",FunctGroupArray[species].groupCode, this_totsum);
 5198: 
 5199: 				/* Normalise and find maximum entry */
```

**Code at issue** — `atdemography.c` line 5070  
Search for: `Update_Invert_Cohorts()`

```c
 5068: 	 updating cohort biomass ratios (ignoring delays due to larval periods).
 5069: 	 Actually update of PopRatio is done after all cells are complete and before next
 5070→ 	 time step by calling Update_Invert_Cohorts() from the main routine.
 5071: 	 Also note that only include in the update if nonzero contributions.
 5072: 	 */
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
**Found at (current code):** `atdemography.c` 4432
**Lines at review (Oct 2026):** 4427

**Function:** `Invertebrate_Reproduction()`

**Why it's thought to be a bug**

`bm->calcTrackedMort[species][0][stock_id][start_id] += spawned_biomass;` always credits genotype 0. Elsewhere, recruits for genotype `ngene` live in cohort index `ngene`. Function: `Invertebrate_Reproduction()`.

**Code at issue** — `atdemography.c` line 4432  
Search for: `calcTrackedMort[species][0][stock_id][start_id] += spawned_biomass`

```c
 4430: 						tot_yoy[species][maxstock_id] += spawned_biomass;
 4431: 						tot_yoy[species][stock_id] += spawned_biomass;
 4432→ 						bm->calcTrackedMort[species][0][stock_id][start_id] += spawned_biomass;  // TODO: Is this covered in general update anyways?
 4433: 					}
 4434: 
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
