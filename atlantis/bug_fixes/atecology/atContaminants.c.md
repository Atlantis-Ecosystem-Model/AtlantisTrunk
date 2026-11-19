# atecology/atContaminants.c — bug review

Generated 2026-11-19 from `atlantis/to-do-fixt-list.md`. **19 findings** (severity H: 6; confidence H and severity H: 3; on the priority list: 3).

Line numbers were re-mapped on 2026-11-19 to the current code (after the merge of main). Each was traced through git history from the version the review used, then checked against the text of the line. **Lines (current code)** and line numbers in the text are now current; ≈ means the code there has changed so the position is approximate. The review's own numbers are kept as **Lines at review**; a number ending in ? could not be traced and is the review's number. ⚠ means the quoted code is no longer at those lines: it may have changed or already been fixed.

Conf/Sev: confidence it's a bug / severity if it is (H = wrong results or crash, M = wrong in some configs, L = cosmetic, log, leak or latent). ✔ = the reviewer re-checked it by hand. ⭐ = on the high-priority list.

## Summary

| ID | Lines (current) | Conf/Sev | Category | Short description | Decision | Done |
|---|---|---|---|---|---|---|
| [CONT-01](#cont-01) 🔀 ⭐ | 303, 486, 598, 713, 780, ≈1035, 1215, 1539, 1582, 1696, 1821, 1835, 1905, 2053, 2076, 2096, 2265 (and atUtilGroupIO.c:909-913) | H/H | buffer overrun (cohort vs cohort×genotype) | `FunctGroupArray[i].contaminantTracers = Util_Alloc_Init_2D_Int(bm->num_contaminants, numCohorts, 0)` (atUtilG… |  | [ ] |
| [CONT-02](#cont-02) ⭐ | 309, 341, 344, 376, 379 | H/H | wrong variable / unit mismatch | `diffden[ij][k] = this_currentden[sp][cohort][k][ij] - bm->boxes[ij].tr[k][den];` — `currentden` passed from a… |  | [ ] |
| [CONT-03](#cont-03) | 314, 341, 376 | M/H | wrong tracer | `localcontam[...] = bm->boxes[ij].tr[k][cid]` with `cid = contaminant_tracer` (the *environmental* contaminant… |  | [ ] |
| [CONT-04](#cont-04) | 345, 380 | M/M | wrong denominator | `tr[clayer][pid] = new_num_in_box / (bm->boxes[ij].tr[clayer][den] + small_num)` — numerator built from `this_… |  | [ ] |
| [CONT-05](#cont-05) 🔀 ⭐ | 423-432, 436 | H/H | wrong tracer array | For `habitat == EPIFAUNA` (called from atecology.c:2553) `tracerArray = getTracerArray(boxLayerInfo, EPIFAUNA)… |  | [ ] |
| [CONT-06](#cont-06) 🔀 | 755-762, ≈783 | M/H | wrong tracer array | For EPIFAUNA habitat `tracerArray` is set to WC, but `cGroupLevel = tracerArray[FunctGroupArray[sp].contaminan… |  | [ ] |
| [CONT-07](#cont-07) 🔀 | 993, 1001, 1090-1091 | M/M | wrong variable / unit mismatch | `amountTransfer` is overwritten with a proportion (`amountTransfer / totalBiomass`) at 966/974, but at 1049 `a… |  | [ ] |
| [CONT-08](#cont-08) 🔀 | 978-988, ≈1024, ≈1099 | M/M | wrong tracer array | `tracerArray` chosen by `fromGuild` habitat (EPIFAUNA → epi array) but used for the *toGuild* `totNTracers[toC… |  | [ ] |
| [CONT-09](#cont-09) | 1477, 1490-1491 | M/M | wrong operator (copy) | `if (step2 < C_reprod_corr[cohort]) C_reprod_corr[cohort] *= step2;` — comment says "taking the minimum value … |  | [ ] |
| [CONT-10](#cont-10) | 1474-1478 | M/M | cohort vs cohort×genotype | `if (cohort < age_mat) return;` — `cohort` is the cohort×genotype index (caller loops `numCohortsXnumGenes`, l… |  | [ ] |
| [CONT-11](#cont-11) | 1599 | H/L | printf format mismatch | `quit("No such contaminant interaction option (%d), ...")` has a `%d` but no argument — undefined behaviour. F… |  | [ ] |
| [CONT-12](#cont-12) | 1836-1838 | H/M | wrong index (cohort×genotype) | `cid_prev = FunctGroupArray[sp].contaminantTracers[cohort-1][cIndex];` — the source cohort for aging is `cohor… |  | [ ] |
| [CONT-13](#cont-13) 🔀 | 1875-1888 | M/H | unit mismatch | `time_step = bm->dayt - Leave_Now[mid]` is in days (Leave_Now set in days, atmigration.c:386), but `half_life`… |  | [ ] |
| [CONT-14](#cont-14) | 1914-1921 | M/M | uninitialised logic / incomplete | `newContam = 0.0` (assignment commented out `//newContam = ;`) so every batch of leavers dilutes the migrant p… |  | [ ] |
| [CONT-15](#cont-15) | 1950-1951, 1965 | M/M | unit mismatch / division by zero | `reprodContam += cGroupLevel * maternal_transfer_rate` (unweighted concentration sum over cells) while `reprod… |  | [ ] |
| [CONT-16](#cont-16) 🔀 | 2034-2036 | L/M | cohort vs cohort×genotype | `drandom(age_mat, numCohortsXnumGenes)` mixes age-class units (age_mat) with cohort×genotype count and ignores… |  | [ ] |
| [CONT-17](#cont-17) | 2230, 2303, 2307 | M/M | wrong index | `bm->forceTracerInput[cIndex].total_input` indexes the forcing-tracer array by contaminant index, but `Init_Co… |  | [ ] |
| [CONT-18](#cont-18) 🔀 | 860, 1307 | L/L | wrong index (layer collision) | `expose_time[sp][cohort][current_box][current_layer]` is sized `wcnz+sednz` layers but in SED habitat `current… |  | [ ] |
| [CONT-19](#cont-19) | 1438 | L/M | wrong operator | `if ((isPrimaryProducer == TRUE) && (isMicroFauna == TRUE))` — a group is unlikely to be both, so the Laender … |  | [ ] |

---

## CONT-01

**buffer overrun (cohort vs cohort×genotype)** · Conf H / Sev H ⭐ priority  
**Lines (current code):** 303, 486, 598, 713, 780, ≈1035, 1215, 1539, 1582, 1696, 1821, 1835, 1905, 2053, 2076, 2096, 2265 (and atUtilGroupIO.c:909-913)  
**Lines at review (Oct 2026):** 307, 480, 592, 707, 768, 1008, 1174, 1498, 1541, 1655, 1780, 1794, 1864, 2000, 2023, 2043, 2212 (and atUtilGroupIO.c:910-914)

**Why it's thought to be a bug**

`FunctGroupArray[i].contaminantTracers = Util_Alloc_Init_2D_Int(bm->num_contaminants, numCohorts, 0)` (atUtilGroupIO.c:909) gives `[numCohorts][ncont]`; same for `contamPropTracers` (931), `contaminantSpMort` (912, 1D `numCohorts`) and `agingContam` (914, first dim `numCohorts`). atBuildTracer.c:385 also only builds tracers for `cohort < numCohorts`. But every loop in this file runs `cohort < numCohortsXnumGenes` and indexes `contaminantTracers[cohort][cIndex]`, `contamPropTracers[cohort][...]`, `contaminantSpMort[cohort]`, and `agingContam[nextcid]` (nextcid = cohort+numGeneTypes, atdemography.c:2634). With numGeneTypes>1 this reads/writes past the allocations (and uses garbage tracer ids). Functions: `Calculate_Containinant_Flux_Habitat()`, `Calculate_Contaminants_Flux()`, `Species_Contaminant_Uptake()`, `Calculate_Species_Contaminant_Decay()`, `Calculate_Species_Contaminant_Effects()`, `Age_Contaminants_Store()`, `Check_Contam_Totals()`, etc.

> 🔀 **Changed by the merge of main (Nov 2026):** code shown above was changed by the merge. Check whether this finding is now fixed before acting.

**Current code** (`atecology/atContaminants.c` lines 301–305) — ≈ the code here has changed since the review; lines are approximate

```c
  301:     for (cIndex = 0; cIndex < bm->num_contaminants; cIndex++) {
  302:         cid = bm->contaminantStructure[cIndex]->contaminant_tracer;
  303:         pid = FunctGroupArray[sp].contamPropTracers[cohort][cIndex];
  304:         
  305:         for (ij = 0; ij < bm->nbox; ij++) {
```

**Suggested fix (from review):** Allocate these with `numCohortsXnumGenes` and build tracers per cohort×genotype (or map cohort×gene to age class consistently)

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

## CONT-02

**wrong variable / unit mismatch** · Conf H / Sev H ⭐ priority  
**Lines (current code):** 309, 341, 344, 376, 379  
**Lines at review (Oct 2026):** 313, 345, 348, 380, 383

**Function:** `Move_Vert_Contaminated()`

**Why it's thought to be a bug**

`diffden[ij][k] = this_currentden[sp][cohort][k][ij] - bm->boxes[ij].tr[k][den];` — `currentden` passed from atmovement.c:2930 holds *proportions* of the total (`currentden = tr[k][den] / totden`, atmovement.c:1545) taken before movement, while `tr[k][den]` at the call point already holds the post-move *numbers* (atmovement.c:2800). So diffden subtracts numbers from a proportion (almost always negative, and sign is reversed vs the intended "new − old"), and lines 341/344/376/379 multiply proportion by `this_currentden` as if it were a count. The contaminated-proportion reallocation is therefore meaningless. Function: `Move_Vert_Contaminated()`.

**Current code** (`atecology/atContaminants.c` lines 307–346)

```c
  307:                 for (k = 0; k < bm->boxes[ij].nz; k++) {
  308:                     // Change in numbers
  309:                     diffden[ij][k] = this_currentden[sp][cohort][k][ij] - bm->boxes[ij].tr[k][den];
  310: 
  311:                     //fprintf(bm->logFile, "Time: %e box%d-%d %s-%d for %s has diffden: %e\n", bm->dayt, ij, k, FunctGroupArray[sp].groupCode, cohort, bm->contaminantStructure[cIndex]->contaminant_name, diffden[ij][k]);
  312: 
  313:                     // Number of contaminanted that must have moved
  314:                     localcontam[ij][k][cIndex] = bm->boxes[ij].tr[k][cid];
  315:                     if (diffden[ij][k] < 0.0) {
  316:                         num_contam_moved[ij][k][cIndex] = floor(bm->boxes[ij].tr[k][pid] * -1.0 * diffden[ij][k]);  // Used to say current_box, current_layer but tha makes no sense as calling from mvoement code so would always be final layer of final box.
  317:                     }
  318:                 }
  319:             }
  320:         }
  321: 
  322:         /* Now reallocate */
  323:         for (ij = 0; ij < bm->nbox; ij++) {
  324:             if (bm->boxes[ij].type != BOUNDARY) {
  325:                 for (k = 0; k < bm->boxes[ij].nz; k++) {
  326:                     // Move contaminated individuals
  327:                     if (num_contam_moved[ij][k][cIndex] < 1.0) {
  328:                         continue; //Nothing to do as nothing moved
  329:                     }
  330: 
  331:                     /* Individuals have moved away - so move them to nearest box with new additional individuals */
  332:                     // Do layers in same box
  333:                     for (clayer = 0; clayer < bm->wcnz; clayer++) {
  334:                         if ( clayer == k )
  335:                             continue;
  336: 
  337:                         if (diffden[ij][clayer] > 0.0) {
  338:                             num_possible = min(num_contam_moved[ij][k][cIndex], diffden[ij][clayer]);
  339: 
  340:                             // New contam level - average of what is there and what moved in
  341:                             bm->boxes[ij].tr[clayer][cid] = (bm->boxes[ij].tr[clayer][cid] * bm->boxes[ij].tr[clayer][pid] * this_currentden[sp][cohort][clayer][ij] + num_possible * localcontam[ij][k][cIndex]) / (num_possible + bm->boxes[ij].tr[clayer][pid] * this_currentden[sp][cohort][clayer][ij] + small_num);
  342: 
  343:                             // Adjust local proportion
  344:                             new_num_in_box = bm->boxes[ij].tr[clayer][pid] * this_currentden[sp][cohort][clayer][ij] + num_possible;
  345:                             bm->boxes[ij].tr[clayer][pid] = new_num_in_box / (bm->boxes[ij].tr[clayer][den] + small_num);  // Denominator used to say layer k not clayer, but that does not make sense as should be recieving layer as recalcualting proportion contaminanted in that layer not the old layer they left.
  346: 
```

**Suggested fix (from review):** Pass/compute old numbers (`currentden*totden`) and use `diffden = new_num - old_num`; use numbers consistently in the averaging

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

## CONT-03

**wrong tracer** · Conf M / Sev H  
**Lines (current code):** 314, 341, 376  
**Lines at review (Oct 2026):** 318, 345, 380

**Function:** `Move_Vert_Contaminated()`

**Why it's thought to be a bug**

`localcontam[...] = bm->boxes[ij].tr[k][cid]` with `cid = contaminant_tracer` (the *environmental* contaminant concentration), and lines 341/376 overwrite `tr[clayer][cid]` / `tr[next_box][clayer][cid]` with a fish-number-weighted average. Moving animals thus rewrites the water-column contaminant concentration (non-conservative). Intended quantity is the group's own tissue tracer `FunctGroupArray[sp].contaminantTracers[cohort][cIndex]`. Function: `Move_Vert_Contaminated()`.

**Current code** (`atecology/atContaminants.c` lines 312–351 (truncated))

```c
  312: 
  313:                     // Number of contaminanted that must have moved
  314:                     localcontam[ij][k][cIndex] = bm->boxes[ij].tr[k][cid];
  315:                     if (diffden[ij][k] < 0.0) {
  316:                         num_contam_moved[ij][k][cIndex] = floor(bm->boxes[ij].tr[k][pid] * -1.0 * diffden[ij][k]);  // Used to say current_box, current_layer but tha makes no sense as calling from mvoement code so would always be final layer of final box.
  317:                     }
  318:                 }
  319:             }
  320:         }
  321: 
  322:         /* Now reallocate */
  323:         for (ij = 0; ij < bm->nbox; ij++) {
  324:             if (bm->boxes[ij].type != BOUNDARY) {
  325:                 for (k = 0; k < bm->boxes[ij].nz; k++) {
  326:                     // Move contaminated individuals
  327:                     if (num_contam_moved[ij][k][cIndex] < 1.0) {
  328:                         continue; //Nothing to do as nothing moved
  329:                     }
  330: 
  331:                     /* Individuals have moved away - so move them to nearest box with new additional individuals */
  332:                     // Do layers in same box
  333:                     for (clayer = 0; clayer < bm->wcnz; clayer++) {
  334:                         if ( clayer == k )
  335:                             continue;
  336: 
  337:                         if (diffden[ij][clayer] > 0.0) {
  338:                             num_possible = min(num_contam_moved[ij][k][cIndex], diffden[ij][clayer]);
  339: 
  340:                             // New contam level - average of what is there and what moved in
  341:                             bm->boxes[ij].tr[clayer][cid] = (bm->boxes[ij].tr[clayer][cid] * bm->boxes[ij].tr[clayer][pid] * this_currentden[sp][cohort][clayer][ij] + num_possible * localcontam[ij][k][cIndex]) / (num_possible + bm->boxes[ij].tr[clayer][pid] * this_currentden[sp][cohort][clayer][ij] + small_num);
  342: 
  343:                             // Adjust local proportion
  344:                             new_num_in_box = bm->boxes[ij].tr[clayer][pid] * this_currentden[sp][cohort][clayer][ij] + num_possible;
  345:                             bm->boxes[ij].tr[clayer][pid] = new_num_in_box / (bm->boxes[ij].tr[clayer][den] + small_num);  // Denominator used to say layer k not clayer, but that does not make sense as should be recieving layer as recalcualting proportion contaminanted in that layer not the old layer they left.
  346: 
  347:                             //fprintf(bm->logFile, "Time %e box%d-%d %s %d %s has after move prop: %e with new_num_in_box %e den: %e\n", bm->dayt, ij, clayer, FunctGroupArray[sp].groupCode, cohort, bm->contaminantStructure[cIndex]->contaminant_name, bm->boxes[ij].tr[clayer][pid], new_num_in_box, bm->boxes[ij].tr[k][den]);
  348: 
  349:                             // Sanity check
  350:                             if (bm->boxes[ij].tr[clayer][pid] < 0.0) {
  351:                                 bm->boxes[ij].tr[clayer][pid] = 0.0;
```

**Suggested fix (from review):** Use `contaminantTracers[cohort][cIndex]` for `cid`

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

## CONT-04

**wrong denominator** · Conf M / Sev M  
**Lines (current code):** 345, 380  
**Lines at review (Oct 2026):** 349, 384

**Function:** `Move_Vert_Contaminated()`

**Why it's thought to be a bug**

`tr[clayer][pid] = new_num_in_box / (bm->boxes[ij].tr[clayer][den] + small_num)` — numerator built from `this_currentden` (lines 344/379) but denominator is `tr[...][den]`; inconsistent sources, proportion is then clamped to 1 hiding the error. Function: `Move_Vert_Contaminated()`.

**Current code** (`atecology/atContaminants.c` lines 343–382)

```c
  343:                             // Adjust local proportion
  344:                             new_num_in_box = bm->boxes[ij].tr[clayer][pid] * this_currentden[sp][cohort][clayer][ij] + num_possible;
  345:                             bm->boxes[ij].tr[clayer][pid] = new_num_in_box / (bm->boxes[ij].tr[clayer][den] + small_num);  // Denominator used to say layer k not clayer, but that does not make sense as should be recieving layer as recalcualting proportion contaminanted in that layer not the old layer they left.
  346: 
  347:                             //fprintf(bm->logFile, "Time %e box%d-%d %s %d %s has after move prop: %e with new_num_in_box %e den: %e\n", bm->dayt, ij, clayer, FunctGroupArray[sp].groupCode, cohort, bm->contaminantStructure[cIndex]->contaminant_name, bm->boxes[ij].tr[clayer][pid], new_num_in_box, bm->boxes[ij].tr[k][den]);
  348: 
  349:                             // Sanity check
  350:                             if (bm->boxes[ij].tr[clayer][pid] < 0.0) {
  351:                                 bm->boxes[ij].tr[clayer][pid] = 0.0;
  352:                             }
  353:                             if (bm->boxes[ij].tr[clayer][pid] > 1.0) {
  354:                                 bm->boxes[ij].tr[clayer][pid] = 1.0;
  355:                             }
  356: 
  357:                             // Update the counters - both of source and sink
  358:                             num_contam_moved[ij][k][cIndex] -= num_possible;
  359:                             diffden[ij][clayer] -= num_possible;
  360:                         }
  361: 
  362:                     }
  363: 
  364:                     // Other boxes
  365:                     nextij = 0;
  366:                     while ((num_contam_moved[ij][k][cIndex] > 1.0) && (nextij < bm->nbox)) {
  367:                         next_box = bm->boxes[ij].distID[nextij];
  368: 
  369:                         if(next_box != ij) {
  370:                             for (clayer = 0; clayer < bm->wcnz; clayer++) {
  371:                                 if (diffden[next_box][clayer] > 0.0) {
  372:                                     //Space for entrants
  373:                                     num_possible = min(num_contam_moved[ij][k][cIndex], diffden[next_box][clayer]);
  374: 
  375:                                     // New contam level - average of what is there and what moved in
  376:                                     bm->boxes[next_box].tr[clayer][cid] = (bm->boxes[next_box].tr[clayer][cid] * bm->boxes[next_box].tr[clayer][pid] * this_currentden[sp][cohort][clayer][next_box] + num_possible * localcontam[ij][k][cIndex]) / (num_possible + bm->boxes[next_box].tr[clayer][pid] * this_currentden[sp][cohort][clayer][next_box] + small_num);
  377: 
  378:                                     // Adjust local proportion
  379:                                     new_num_in_box = bm->boxes[next_box].tr[clayer][pid] * this_currentden[sp][cohort][clayer][next_box] + num_possible;
  380:                                     bm->boxes[next_box].tr[clayer][pid] =  new_num_in_box / (bm->boxes[next_box].tr[clayer][den] + small_num);
  381: 
  382:                                     //fprintf(bm->logFile, "Time %e next_box%d-%d %s %d %s has after move prop: %e with  new_num_in_box %e den: %e\n", bm->dayt, next_box, clayer, FunctGroupArray[sp].groupCode, cohort, bm->contaminantStructure[cIndex]->contaminant_name, bm->boxes[next_box].tr[clayer][pid], new_num_in_box, bm->boxes[next_box].tr[clayer][den]);
```

**Suggested fix (from review):** Use the same (post-move) numbers in numerator and denominator

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

## CONT-05

**wrong tracer array** · Conf H / Sev H ⭐ priority  
**Lines (current code):** 423-432, 436  
**Lines at review (Oct 2026):** 427-431, 435

**Function:** `Change_Contaminant_Levels()`

**Why it's thought to be a bug**

For `habitat == EPIFAUNA` (called from atecology.c:2553) `tracerArray = getTracerArray(boxLayerInfo, EPIFAUNA)` = `localEPITracers`, then `cLevel = tracerArray[contaminant_tracer]` indexes the epibenthic array with a water-column tracer id (wrong value, possible OOB). `Degrade_Contaminants()` then sets `amount_decayed` from this, which is subtracted in the EPIFAUNA branch of `Calculate_Contaminants_Flux()` (lines 576-577). Compare `Species_Contaminant_Uptake()` line ≈757 and `Calculate_Species_Contaminant_Effects()` line 1658 which use the WC array for the environmental level under EPIFAUNA. Function: `Change_Contaminant_Levels()`.

> 🔀 **Changed by the merge of main (Nov 2026):** code shown above was changed by the merge. Check whether this finding is now fixed before acting.

**Current code** (`atecology/atContaminants.c` lines 421–438)

```c
  421:     int cIndex, sp, cohort;
  422:     double cLevel;
  423:     double *tracerArray = getTracerArray(boxLayerInfo, habitat);
  424: 
  425:     /* DEBUG: Log when contaminant processing happens
  426:     if (bm->dayt < 2.0 && habitat == WC) {
  427:         fprintf(bm->logFile, "DEBUG_TIMING: Change_Contaminant_Levels START for habitat WC at time=%e\n", bm->dayt);
  428:     } */
  429: 
  430:     for (cIndex = 0; cIndex < bm->num_contaminants; cIndex++) {
  431:         /* Grab the level in the water column or the sediment */
  432:         cLevel = tracerArray[bm->contaminantStructure[cIndex]->contaminant_tracer];
  433: 
  434:         Calculate_Species_Contaminant_Decay(bm, boxLayerInfo, habitat, dtsz, cIndex, cLevel);
  435:         Species_Contaminant_Uptake(bm, boxLayerInfo, habitat, dtsz, cIndex);
  436:         Degrade_Contaminants(bm, boxLayerInfo, habitat, dtsz, cIndex, cLevel);
  437:     }
  438:     Calculate_Contaminants_Flux(bm, boxLayerInfo, habitat);
```

**Suggested fix (from review):** For EPIFAUNA read `cLevel` from `getTracerArray(boxLayerInfo, WC)`

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

## CONT-06

**wrong tracer array** · Conf M / Sev H  
**Lines (current code):** 755-762, ≈783  
**Lines at review (Oct 2026):** 748-752, 770

**Function:** `Species_Contaminant_Uptake()`

**Why it's thought to be a bug**

For EPIFAUNA habitat `tracerArray` is set to WC, but `cGroupLevel = tracerArray[FunctGroupArray[sp].contaminantTracers[cohort][cIndex]]` — for epibenthic groups those tracer ids index the epi array (`epi[pid]` is correctly used at line 898). So group level is read from the wrong array, corrupting uptake (linear/sigmoid uses `cLevel - cGroupLevel`). `Gain_Contaminants()` (684-688) handles this correctly. Function: `Species_Contaminant_Uptake()`.

> 🔀 **Changed by the merge of main (Nov 2026):** code shown above was changed by the merge. Check whether this finding is now fixed before acting.

**Current code** (`atecology/atContaminants.c` lines 753–785) — ≈ the code here has changed since the review; lines are approximate

```c
  753: 
  754: 	/* For EPIFAUNA groups: environmental concentration comes from WC, but organism concentration lives in EPIFAUNA */
  755: 	if (habitat == EPIFAUNA) {
  756: 		environmentTracerArray = getTracerArray(boxLayerInfo, WC);      /* Water column concentration (forced from outside) */
  757: 		organismTracerArray = getTracerArray(boxLayerInfo, EPIFAUNA);   /* Organism concentration in EPIFAUNA */
  758: 	} else {
  759: 		/* For other habitats, both come from the same habitat */
  760: 		environmentTracerArray = getTracerArray(boxLayerInfo, habitat);
  761: 		organismTracerArray = getTracerArray(boxLayerInfo, habitat);
  762: 	}
  763: 
  764: 
  765: 	/* For each contaminant calculate the uptake */
  766: 
  767:     /* Grab the level in the water column or the sediment (environmental concentration) */
  768:     cLevel = environmentTracerArray[bm->contaminantStructure[cIndex]->contaminant_tracer];
  769: 
  770:     if(cLevel > bm->min_pool_cont){
  771:         //fprintf(bm->logFile, "time %e, box %d, layer %d, cLevel = %e\n", bm->dayt, bm->current_box, bm->current_layer, cLevel);
  772:         for (sp = 0; sp < bm->K_num_tot_sp; sp++) {
  773: 
  774:             /* Contact can occur for all active groups that are present in this habitat */
  775:             if ((FunctGroupArray[sp].speciesParams[flag_id] == TRUE) && (FunctGroupArray[sp].isDetritus == FALSE) && (FunctGroupArray[sp].habitatCoeffs[habitat] > 0)) {
  776:                 
  777:                 uptake_option = (CONTAMINANT_UPTAKE_OPTION)bm->contaminantStructure[cIndex]->sp_uptake_option[sp];
  778:                 uptake_rate = bm->contaminantStructure[cIndex]->sp_uptake_rate[sp];
  779: 
  780:                 for(cohort = 0; cohort < FunctGroupArray[sp].numCohortsXnumGenes; cohort++){
  781:                     int h;
  782:                     /* The current concentration in the group (from organism's habitat) */
  783:                     cGroupLevel = organismTracerArray[FunctGroupArray[sp].contaminantTracers[cohort][cIndex]];
  784:                     /* Add any transfers from this timestep that haven't been integrated into tracer yet */
  785:                     /* sp_transfer is indexed by prey habitat, not predator habitat, so sum across all habitats */
```

**Suggested fix (from review):** Read group levels from `getTracerArray(boxLayerInfo, EPIFAUNA)` when the group is epifaunal

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

## CONT-07

**wrong variable / unit mismatch** · Conf M / Sev M  
**Lines (current code):** 993, 1001, 1090-1091  
**Lines at review (Oct 2026):** 966, 974, 1049-1050

**Function:** `Group_Transfer_Contaminant()`

**Why it's thought to be a bug**

`amountTransfer` is overwritten with a proportion (`amountTransfer / totalBiomass`) at 966/974, but at 1049 `amt_exchanged = amountTransfer * dtsz; prop_exchanged = amt_exchanged / toGuild_totalBiomass` treats it as a biomass amount — proportion divided by receiver biomass. Function: `Group_Transfer_Contaminant()`.

> 🔀 **Changed by the merge of main (Nov 2026):** code shown above was changed by the merge. Check whether this finding is now fixed before acting.

**Current code** (`atecology/atContaminants.c` lines 991–1003)

```c
  991: 	if (FunctGroupArray[fromGuild].groupAgeType == AGE_STRUCTURED) {
  992:         totalBiomass = (spSPinfo[fromGuild][fromCohort][SN_id] + spSPinfo[fromGuild][fromCohort][RN_id]) * spSPinfo[fromGuild][fromCohort][DEN_id] / bm->cell_vol;
  993:         amountTransfer = amountTransfer / (totalBiomass + small_num);
  994: 	} else {
  995: 		//totalBiomass = tracerArray[FunctGroupArray[fromGuild].totNTracers[fromCohort]];
  996: 
  997:         if(!initialBiomass)  // No actual transfer possible
  998:             return 0;
  999: 
 1000:         totalBiomass = initialBiomass;
 1001:         amountTransfer = amountTransfer / (totalBiomass + small_num);
 1002: 	}
 1003: 
```

**Suggested fix (from review):** Keep the original amount in a separate variable and use it for `amt_exchanged`

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

## CONT-08

**wrong tracer array** · Conf M / Sev M  
**Lines (current code):** 978-988, ≈1024, ≈1099  
**Lines at review (Oct 2026):** 957-961, 997, 1058

**Function:** `Group_Transfer_Contaminant()`

**Why it's thought to be a bug**

`tracerArray` chosen by `fromGuild` habitat (EPIFAUNA → epi array) but used for the *toGuild* `totNTracers[toCohort]` (≈1024) and `NumsTracers[toCohort]` (≈1099). When prey is epifaunal and predator is not (or vice versa) the predator's indices are read from the wrong array. Function: `Group_Transfer_Contaminant()`.

> 🔀 **Changed by the merge of main (Nov 2026):** code shown above was changed by the merge. Check whether this finding is now fixed before acting.

**Current code** (`atecology/atContaminants.c` lines 976–1015 (truncated)) — ≈ the code here has changed since the review; lines are approximate

```c
  976: 
  977: 	/* Get tracer arrays - separate for prey and predator since they may be in different habitats */
  978: 	if(FunctGroupArray[fromGuild].habitatType == EPIFAUNA){
  979: 	  preyTracerArray = getTracerArray(boxLayerInfo, EPIFAUNA);
  980: 	} else {
  981: 	  preyTracerArray = getTracerArray(boxLayerInfo, habitat);
  982: 	}
  983: 
  984: 	if(FunctGroupArray[toGuild].habitatType == EPIFAUNA){
  985: 	  predatorTracerArray = getTracerArray(boxLayerInfo, EPIFAUNA);
  986: 	} else {
  987: 	  predatorTracerArray = getTracerArray(boxLayerInfo, habitat);
  988: 	}
  989: 
  990: 	/* Work out the proportion transfered */
  991: 	if (FunctGroupArray[fromGuild].groupAgeType == AGE_STRUCTURED) {
  992:         totalBiomass = (spSPinfo[fromGuild][fromCohort][SN_id] + spSPinfo[fromGuild][fromCohort][RN_id]) * spSPinfo[fromGuild][fromCohort][DEN_id] / bm->cell_vol;
  993:         amountTransfer = amountTransfer / (totalBiomass + small_num);
  994: 	} else {
  995: 		//totalBiomass = tracerArray[FunctGroupArray[fromGuild].totNTracers[fromCohort]];
  996: 
  997:         if(!initialBiomass)  // No actual transfer possible
  998:             return 0;
  999: 
 1000:         totalBiomass = initialBiomass;
 1001:         amountTransfer = amountTransfer / (totalBiomass + small_num);
 1002: 	}
 1003: 
 1004:     /* Sanity check */
 1005:     if (amountTransfer > 1.0) {
 1006:         amountTransfer = 1.0;
 1007:     }
 1008:     
 1009:     /**
 1010: 	if ((isnan(propTransfer)) || (propTransfer > 1.0)) {
 1011: 		fprintf(stderr, "Group_Transfer_Contaminant group propTransfer level is NAN or prop > 1 - From %s-%d, in box %d-%d, propTransfer: %e amountTransfer = %e, totalBiomass = %e\n", FunctGroupArray[fromGuild].groupCode, fromCohort, bm->current_box, bm->current_layer, propTransfer, amountTransfer, totalBiomass);
 1012: 		quit("Group_Transfer_Contaminant nan or prop > 1\n");
 1013: 	}
 1014:     **/
 1015: 
```

**Suggested fix (from review):** Select a separate tracer array based on `FunctGroupArray[toGuild].habitatType`

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

## CONT-09

**wrong operator (copy)** · Conf M / Sev M  
**Lines (current code):** 1477, 1490-1491  
**Lines at review (Oct 2026):** 1436, 1449-1450

**Function:** `Get_ContamReproductionEffects()`

**Why it's thought to be a bug**

`if (step2 < C_reprod_corr[cohort]) C_reprod_corr[cohort] *= step2;` — comment says "taking the minimum value only" but it multiplies, compounding the effect across contaminants. Function: `Get_ContamReproductionEffects()`.

**Current code** (`atecology/atContaminants.c` lines 1475–1493)

```c
 1475:   double step1, step2, L_sp_r, b_sp_r, a_sp_r;
 1476:   
 1477:   if ( cohort < age_mat ) // To young for it to matter.
 1478:     return;
 1479:   
 1480:   switch (bm->flag_contamReprodModel){
 1481:   case NoReprodEffects: // No repro effects
 1482:     FunctGroupArray[species].C_reprod_corr[cohort] = 1.0;
 1483:     break;
 1484:   case Lovindeeretal: //Equation 13 Lovideer et al. paper  
 1485:     if (cGroupLevel > chronicLevel) {
 1486:       step1 = bm->contaminantStructure[cIndex]->sp_ReprodEffect[species] * cGroupLevel;
 1487:       if (step1 > 1.0)
 1488:         step1 = 1.0;
 1489:       step2 = 1.0 - step1;
 1490:       if (step2 < FunctGroupArray[species].C_reprod_corr[cohort]) // As taking the minimum value only
 1491:         FunctGroupArray[species].C_reprod_corr[cohort] *= step2;
 1492:     }
 1493:     break;
```

**Suggested fix (from review):** `C_reprod_corr[cohort] = step2;`

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

## CONT-10

**cohort vs cohort×genotype** · Conf M / Sev M  
**Lines (current code):** 1474-1478  
**Lines at review (Oct 2026):** 1433-1437

**Function:** `Get_ContamReproductionEffects()`

**Why it's thought to be a bug**

`if (cohort < age_mat) return;` — `cohort` is the cohort×genotype index (caller loops `numCohortsXnumGenes`, line 1683) while `age_mat` is in age classes. Function: `Get_ContamReproductionEffects()`.

**Current code** (`atecology/atContaminants.c` lines 1472–1480)

```c
 1472: void Get_ContamReproductionEffects(MSEBoxModel *bm, double cGroupLevel, int species, int cohort, int cIndex) {
 1473:   double chronicLevel = bm->contaminantStructure[cIndex]->sp_maxChronicConc[species];  // chronic level
 1474:   int age_mat = (int) (FunctGroupArray[species].speciesParams[age_mat_id]);
 1475:   double step1, step2, L_sp_r, b_sp_r, a_sp_r;
 1476:   
 1477:   if ( cohort < age_mat ) // To young for it to matter.
 1478:     return;
 1479:   
 1480:   switch (bm->flag_contamReprodModel){
```

**Suggested fix (from review):** Compare `cohort / numGeneTypes` with age_mat

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

## CONT-11

**printf format mismatch** · Conf H / Sev L  
**Lines (current code):** 1599  
**Lines at review (Oct 2026):** 1558

**Function:** `Get_Contam_Amplification()`

**Why it's thought to be a bug**

`quit("No such contaminant interaction option (%d), ...")` has a `%d` but no argument — undefined behaviour. Function: `Get_Contam_Amplification()`.

**Current code** (`atecology/atContaminants.c` lines 1597–1601)

```c
 1597:                 break;
 1598:             default:
 1599:                 quit("No such contaminant interaction option (%d), must be no interaciton (0), additive (1), multiplicative (2), most limiting (3)\n");
 1600:                 break;
 1601:         }
```

**Suggested fix (from review):** Pass `bm->flag_contamInteractModel`

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

## CONT-12

**wrong index (cohort×genotype)** · Conf H / Sev M  
**Lines (current code):** 1836-1838  
**Lines at review (Oct 2026):** 1795-1797

**Function:** `Age_Contaminants_Update()`

**Why it's thought to be a bug**

`cid_prev = FunctGroupArray[sp].contaminantTracers[cohort-1][cIndex];` — the source cohort for aging is `cohort - numGeneTypes` (nextcid = cohort + sp_numGeneTypes, atdemography.c:2634); with genotypes, `cohort-1` is a different genotype of the same/younger age. Function: `Age_Contaminants_Update()`.

**Current code** (`atecology/atContaminants.c` lines 1834–1840)

```c
 1834:     for (cIndex = 0; cIndex < bm->num_contaminants; cIndex++) {
 1835:         cid = FunctGroupArray[sp].contaminantTracers[cohort][cIndex];
 1836:         cid_prev = FunctGroupArray[sp].contaminantTracers[cohort-1][cIndex];
 1837:         bm->boxes[ij].tr[k][cid] += FunctGroupArray[sp].agingContam[cohort][cIndex][ij][k];
 1838:         bm->boxes[ij].tr[k][cid_prev] -= FunctGroupArray[sp].agingContam[cohort][cIndex][ij][k];
 1839:         // Add and subtract stuff being transferred (need to add and subtract here as not moved elsewhere anymore for clarity sake)
 1840:         //TODO: Need to think about how to handle this if the new age group isn't present in this water column and layer...
```

**Suggested fix (from review):** `cohort - FunctGroupArray[sp].numGeneTypes`

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

## CONT-13

**unit mismatch** · Conf M / Sev H  
**Lines (current code):** 1875-1888  
**Lines at review (Oct 2026):** 1834-1847

**Function:** `ContaminantMigrationIn()`

**Why it's thought to be a bug**

`time_step = bm->dayt - Leave_Now[mid]` is in days (Leave_Now set in days, atmigration.c:386), but `half_life` is in seconds (Degrade_Contaminants line 456 uses `dtsz / half_life` with dtsz in seconds). Decay during migration is underestimated by a factor 86400. Function: `ContaminantMigrationIn()`.

> 🔀 **Changed by the merge of main (Nov 2026):** ContaminantMigrationIn ordering/time_step (M-01). Check whether this finding is now fixed before acting.

**Current code** (`atecology/atContaminants.c` lines 1873–1890)

```c
 1873:     int cIndex;
 1874:     double base_contam, this_half_life, final_contam;
 1875:     double day_start = (double)(MIGRATION[sp].Leave_Now[mid]);
 1876:     double day_end = bm->dayt;
 1877:     double time_step = day_end - day_start;
 1878: 
 1879:     for (cIndex = 0; cIndex < bm->num_contaminants; cIndex++) {
 1880:         base_contam = MIGRATION[sp].contam[cohort][mid][cIndex];
 1881:         
 1882:         if(bm->flag_contam_halflife_spbased) {
 1883:             this_half_life = bm->contaminantStructure[cIndex]->sp_decay_half_life[sp];
 1884:         } else {
 1885:             this_half_life = bm->contaminantStructure[cIndex]->half_life;
 1886:         }
 1887: 
 1888:         final_contam = base_contam * pow(0.5, time_step / this_half_life);
 1889:                 
 1890:         /* Alternative code -- Using the decay constant instead of the half life
```

**Suggested fix (from review):** `time_step = (bm->dayt - Leave_Now[mid]) * 86400.0`

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

## CONT-14

**uninitialised logic / incomplete** · Conf M / Sev M  
**Lines (current code):** 1914-1921  
**Lines at review (Oct 2026):** 1873-1880

**Function:** `ContaminantMigrationOut()`

**Why it's thought to be a bug**

`newContam = 0.0` (assignment commented out `//newContam = ;`) so every batch of leavers dilutes the migrant pool's contaminant level towards zero: `(origContam*oldden + 0*num_leaving)/(...)`. Leavers' tissue burden is lost. Function: `ContaminantMigrationOut()`.

**Current code** (`atecology/atContaminants.c` lines 1912–1923)

```c
 1912: void ContaminantMigrationOut(MSEBoxModel *bm, int sp, int cohort, int mid, double oldden, double num_leaving) {
 1913:     int cIndex;
 1914:     double newContam = 0.0, origContam;
 1915:     
 1916:     // For species without live birth and maternal care
 1917:     for (cIndex = 0; cIndex < bm->num_contaminants; cIndex++) {
 1918:         origContam = MIGRATION[sp].contam[cohort][mid][cIndex];
 1919:         //newContam = ;
 1920:         if(oldden > 0) {
 1921:             MIGRATION[sp].contam[cohort][mid][cIndex] = (origContam * oldden + newContam * num_leaving) / (num_leaving + oldden + small_num);
 1922:         } else {
 1923:             MIGRATION[sp].contam[cohort][mid][cIndex] = newContam;
```

**Suggested fix (from review):** Set `newContam` from the leavers' tissue concentration

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

## CONT-15

**unit mismatch / division by zero** · Conf M / Sev M  
**Lines (current code):** 1950-1951, 1965  
**Lines at review (Oct 2026):** 1909-1910, 1924

**Why it's thought to be a bug**

`reprodContam += cGroupLevel * maternal_transfer_rate` (unweighted concentration sum over cells) while `reprodContamCount += this_den` (numbers); `Store_Recruit_Contaminants()` then divides the two — not a density-weighted mean, and divides by zero when no spawners contributed (NaN). Functions: `Get_Parental_Contaminants()`, `Store_Recruit_Contaminants()`.

**Current code** (`atecology/atContaminants.c` lines 1948–1967)

```c
 1948:            cid = FunctGroupArray[sp].contaminantTracers[cohort][cIndex];
 1949:            cGroupLevel = bm->boxes[bm->current_box].tr[bm->current_layer][cid];
 1950:            FunctGroupArray[sp].reprodContam[cIndex] += (cGroupLevel * maternal_transfer_rate);
 1951:            FunctGroupArray[sp].reprodContamCount[cIndex] += this_den;
 1952:            bm->boxes[bm->current_box].tr[bm->current_layer][cid] -= (cGroupLevel * maternal_transfer_rate);
 1953:            // As lost to offspring instantaneously (as would be in external spawners)
 1954:        }
 1955:     }
 1956: }
 1957: 
 1958: /* Store contaminants per offspring until needed */
 1959: void Store_Recruit_Contaminants(MSEBoxModel *bm, int sp, int stock_id, int ngene, int qid) {
 1960:     int cIndex;
 1961:     int flagmother = (int) (FunctGroupArray[sp].speciesParams[flagmother_id]);
 1962:     
 1963:     if (bm->flag_contamMaternalTransfer && (flagmother < 1)) {  // Only those not using maternal care transfer here
 1964:         for (cIndex = 0; cIndex < bm->num_contaminants; cIndex++) {
 1965:             EMBRYO[sp].Contam[stock_id][ngene][qid][cIndex] = FunctGroupArray[sp].reprodContam[cIndex] / FunctGroupArray[sp].reprodContamCount[cIndex];  // Average contributed across the entire spawning population
 1966:         }
 1967:     }
```

**Suggested fix (from review):** Accumulate `cGroupLevel*rate*this_den`; guard count>0

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

## CONT-16

**cohort vs cohort×genotype** · Conf L / Sev M  
**Lines (current code):** 2034-2036  
**Lines at review (Oct 2026):** 1993-1995

**Function:** `Get_Settler_Contaminants()`

**Why it's thought to be a bug**

`drandom(age_mat, numCohortsXnumGenes)` mixes age-class units (age_mat) with cohort×genotype count and ignores `ngene`, so the chosen "mother" cohort may be the wrong genotype/age (also overruns contaminantTracers per CONT-01). Function: `Get_Settler_Contaminants()`.

> 🔀 **Changed by the merge of main (Nov 2026):** Get_Settler_Contaminants segfault fix. Check whether this finding is now fixed before acting.

**Current code** (`atecology/atContaminants.c` lines 2032–2038)

```c
 2032:     int flagmother = (int) (FunctGroupArray[sp].speciesParams[flagmother_id]);
 2033:     double this_reprodContam, maternal_transfer_rate, cGroupLevel;
 2034:     double start_n = FunctGroupArray[sp].speciesParams[age_mat_id];
 2035:     double end_n = (double)FunctGroupArray[sp].numCohortsXnumGenes;
 2036:     int cohort = (int)(floor(drandom(start_n, end_n))); // Find a random adult cohort to use
 2037:     
 2038:    
```

**Suggested fix (from review):** Pick an adult age class and convert to `age*numGeneTypes + ngene`

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

## CONT-17

**wrong index** · Conf M / Sev M  
**Lines (current code):** 2230, 2303, 2307  
**Lines at review (Oct 2026):** 2177, 2250, 2254

**Function:** `Check_Contam_Totals()`

**Why it's thought to be a bug**

`bm->forceTracerInput[cIndex].total_input` indexes the forcing-tracer array by contaminant index, but `Init_Contaminants()` (204-208) shows contaminants are matched to forcing entries by name, not position. If any non-contaminant forcing tracer precedes them, the wrong total is checked (spurious quit or missed check). Function: `Check_Contam_Totals()`.

**Current code** (`atecology/atContaminants.c` lines 2228–2232)

```c
 2228:         **/
 2229:         
 2230:         if(!bm->forceTracerInput[cIndex].total_input) {
 2231:             continue;  // Nothing to do so skip ahead
 2232:         }
```

**Suggested fix (from review):** Store the matched forcing index per contaminant in Init_Contaminants and use it

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

## CONT-18

**wrong index (layer collision)** · Conf L / Sev L  
**Lines (current code):** 860, 1307  
**Lines at review (Oct 2026):** 840, 1266

**Why it's thought to be a bug**

`expose_time[sp][cohort][current_box][current_layer]` is sized `wcnz+sednz` layers but in SED habitat `current_layer` is a sediment layer index (0..), colliding with WC layer slots. Same for `sp_point`/`sp_maxDoseToDate`. Functions: `Species_Contaminant_Uptake()`, `Get_ContamMortEffects()`.

> 🔀 **Changed by the merge of main (Nov 2026):** code shown above was changed by the merge. Check whether this finding is now fixed before acting.

**Current code** (`atecology/atContaminants.c` lines 858–862)

```c
  858:                     // Initiate exposure time
  859:                     if (!cGroupLevel && cUptake) {
  860:                         bm->contaminantStructure[cIndex]->expose_time[sp][cohort][bm->current_box][bm->current_layer] = bm->dayt;
  861:                     }
  862:                         
```

**Suggested fix (from review):** Offset sediment layers by `wcnz`

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

## CONT-19

**wrong operator** · Conf L / Sev M  
**Lines (current code):** 1438  
**Lines at review (Oct 2026):** 1397

**Why it's thought to be a bug**

`if ((isPrimaryProducer == TRUE) && (isMicroFauna == TRUE))` — a group is unlikely to be both, so the Laender EC50 branch is effectively dead; `

**Current code** (`atecology/atContaminants.c` lines 1436–1440)

```c
 1436:         case InVitro_model:  // Growth effects as of InVitro
 1437:             if (cGroupLevel > bm->contaminantStructure[cIndex]->sp_GrowthThresh[species]) {
 1438:                 if ((FunctGroupArray[species].isPrimaryProducer == TRUE) && (FunctGroupArray[species].isMicroFauna == TRUE)){
 1439:                     step1 = 1.0 + pow((cGroupLevel / bm->contaminantStructure[cIndex]->sp_EC50[species]), bm->contaminantStructure[cIndex]->sp_ECslope[species]);
 1440:                     FunctGroupArray[species].C_growth_corr[cohort] *= 1.0 / step1; // As of Laender et al 2008
```

**Suggested fix (from review):** —

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
