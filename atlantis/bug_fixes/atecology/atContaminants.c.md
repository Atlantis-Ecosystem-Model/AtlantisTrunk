# atecology/atContaminants.c — bug review

Generated 2026-11-19 from `atlantis/to-do-fixt-list.md`. **19 findings** (severity H: 6; confidence H and severity H: 3; on the priority list: 3).

Line numbers in **Listed lines** are from the review (10–12 Oct 2026). The code shown is from the working tree on 2026-11-19. Where the code has moved, the snippet follows it and says so. ⚠ means the quoted code could not be found: it may have been changed or already fixed.

Conf/Sev: confidence it's a bug / severity if it is (H = wrong results or crash, M = wrong in some configs, L = cosmetic, log, leak or latent). ✔ = the reviewer re-checked it by hand. ⭐ = on the high-priority list.

## Summary

| ID | Listed lines | Conf/Sev | Category | Short description | Decision | Done |
|---|---|---|---|---|---|---|
| [CONT-01](#cont-01) ⭐ | 307, 480, 592, 707, 768, 1008, 1174, 1498, 1541, 1655, 1780, 1794, 1864, 2000, 2023, 2043, 2212 (and atUtilGroupIO.c:910-914) | H/H | buffer overrun (cohort vs cohort×genotype) | `FunctGroupArray[i].contaminantTracers = Util_Alloc_Init_2D_Int(bm->num_contaminants, numCohorts, 0)` (atUtilG… |  | [ ] |
| [CONT-02](#cont-02) ⭐ | 313, 345, 348, 380, 383 | H/H | wrong variable / unit mismatch | `diffden[ij][k] = this_currentden[sp][cohort][k][ij] - bm->boxes[ij].tr[k][den];` — `currentden` passed from a… |  | [ ] |
| [CONT-03](#cont-03) | 318, 345, 380 | M/H | wrong tracer | `localcontam[...] = bm->boxes[ij].tr[k][cid]` with `cid = contaminant_tracer` (the *environmental* contaminant… |  | [ ] |
| [CONT-04](#cont-04) | 349, 384 | M/M | wrong denominator | `tr[clayer][pid] = new_num_in_box / (bm->boxes[ij].tr[clayer][den] + small_num)` — numerator built from `this_… |  | [ ] |
| [CONT-05](#cont-05) ⭐ | 427-431, 435 | H/H | wrong tracer array | For `habitat == EPIFAUNA` (called from atecology.c:2553) `tracerArray = getTracerArray(boxLayerInfo, EPIFAUNA)… |  | [ ] |
| [CONT-06](#cont-06) | 748-752, 770 | M/H | wrong tracer array | For EPIFAUNA habitat `tracerArray` is set to WC, but `cGroupLevel = tracerArray[FunctGroupArray[sp].contaminan… |  | [ ] |
| [CONT-07](#cont-07) | 966, 974, 1049-1050 | M/M | wrong variable / unit mismatch | `amountTransfer` is overwritten with a proportion (`amountTransfer / totalBiomass`) at 966/974, but at 1049 `a… |  | [ ] |
| [CONT-08](#cont-08) | 957-961, 997, 1058 | M/M | wrong tracer array | `tracerArray` chosen by `fromGuild` habitat (EPIFAUNA → epi array) but used for the *toGuild* `totNTracers[toC… |  | [ ] |
| [CONT-09](#cont-09) | 1436, 1449-1450 | M/M | wrong operator (copy) | `if (step2 < C_reprod_corr[cohort]) C_reprod_corr[cohort] *= step2;` — comment says "taking the minimum value … |  | [ ] |
| [CONT-10](#cont-10) | 1433-1437 | M/M | cohort vs cohort×genotype | `if (cohort < age_mat) return;` — `cohort` is the cohort×genotype index (caller loops `numCohortsXnumGenes`, l… |  | [ ] |
| [CONT-11](#cont-11) | 1558 | H/L | printf format mismatch | `quit("No such contaminant interaction option (%d), ...")` has a `%d` but no argument — undefined behaviour. F… |  | [ ] |
| [CONT-12](#cont-12) | 1795-1797 | H/M | wrong index (cohort×genotype) | `cid_prev = FunctGroupArray[sp].contaminantTracers[cohort-1][cIndex];` — the source cohort for aging is `cohor… |  | [ ] |
| [CONT-13](#cont-13) | 1834-1847 | M/H | unit mismatch | `time_step = bm->dayt - Leave_Now[mid]` is in days (Leave_Now set in days, atmigration.c:386), but `half_life`… |  | [ ] |
| [CONT-14](#cont-14) | 1873-1880 | M/M | uninitialised logic / incomplete | `newContam = 0.0` (assignment commented out `//newContam = ;`) so every batch of leavers dilutes the migrant p… |  | [ ] |
| [CONT-15](#cont-15) | 1909-1910, 1924 | M/M | unit mismatch / division by zero | `reprodContam += cGroupLevel * maternal_transfer_rate` (unweighted concentration sum over cells) while `reprod… |  | [ ] |
| [CONT-16](#cont-16) | 1993-1995 | L/M | cohort vs cohort×genotype | `drandom(age_mat, numCohortsXnumGenes)` mixes age-class units (age_mat) with cohort×genotype count and ignores… |  | [ ] |
| [CONT-17](#cont-17) | 2177, 2250, 2254 | M/M | wrong index | `bm->forceTracerInput[cIndex].total_input` indexes the forcing-tracer array by contaminant index, but `Init_Co… |  | [ ] |
| [CONT-18](#cont-18) | 840, 1266 | L/L | wrong index (layer collision) | `expose_time[sp][cohort][current_box][current_layer]` is sized `wcnz+sednz` layers but in SED habitat `current… |  | [ ] |
| [CONT-19](#cont-19) | 1397 | L/M | wrong operator | `if ((isPrimaryProducer == TRUE) && (isMicroFauna == TRUE))` — a group is unlikely to be both, so the Laender … |  | [ ] |

---

## CONT-01

**buffer overrun (cohort vs cohort×genotype)** · Conf H / Sev H ⭐ priority  
**Listed lines:** 307, 480, 592, 707, 768, 1008, 1174, 1498, 1541, 1655, 1780, 1794, 1864, 2000, 2023, 2043, 2212 (and atUtilGroupIO.c:910-914)

**Why it's thought to be a bug**

`FunctGroupArray[i].contaminantTracers = Util_Alloc_Init_2D_Int(bm->num_contaminants, numCohorts, 0)` (atUtilGroupIO.c:910) gives `[numCohorts][ncont]`; same for `contamPropTracers` (911), `contaminantSpMort` (912, 1D `numCohorts`) and `agingContam` (914, first dim `numCohorts`). atBuildTracer.c:378 also only builds tracers for `cohort < numCohorts`. But every loop in this file runs `cohort < numCohortsXnumGenes` and indexes `contaminantTracers[cohort][cIndex]`, `contamPropTracers[cohort][...]`, `contaminantSpMort[cohort]`, and `agingContam[nextcid]` (nextcid = cohort+numGeneTypes, atdemography.c:2630). With numGeneTypes>1 this reads/writes past the allocations (and uses garbage tracer ids). Functions: `Calculate_Containinant_Flux_Habitat()`, `Calculate_Contaminants_Flux()`, `Species_Contaminant_Uptake()`, `Calculate_Species_Contaminant_Decay()`, `Calculate_Species_Contaminant_Effects()`, `Age_Contaminants_Store()`, `Check_Contam_Totals()`, etc.

**Current code** (`atecology/atContaminants.c` lines 305–309)

```c
  305:     for (cIndex = 0; cIndex < bm->num_contaminants; cIndex++) {
  306:         cid = bm->contaminantStructure[cIndex]->contaminant_tracer;
  307:         pid = FunctGroupArray[sp].contamPropTracers[cohort][cIndex];
  308:         
  309:         for (ij = 0; ij < bm->nbox; ij++) {
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
**Listed lines:** 313, 345, 348, 380, 383

**Function:** `Move_Vert_Contaminated()`

**Why it's thought to be a bug**

`diffden[ij][k] = this_currentden[sp][cohort][k][ij] - bm->boxes[ij].tr[k][den];` — `currentden` passed from atmovement.c:2927 holds *proportions* of the total (`currentden = tr[k][den] / totden`, atmovement.c:1542) taken before movement, while `tr[k][den]` at the call point already holds the post-move *numbers* (atmovement.c:2797). So diffden subtracts numbers from a proportion (almost always negative, and sign is reversed vs the intended "new − old"), and lines 345/348/380/383 multiply proportion by `this_currentden` as if it were a count. The contaminated-proportion reallocation is therefore meaningless. Function: `Move_Vert_Contaminated()`.

**Current code** (`atecology/atContaminants.c` lines 311–350)

```c
  311:                 for (k = 0; k < bm->boxes[ij].nz; k++) {
  312:                     // Change in numbers
  313:                     diffden[ij][k] = this_currentden[sp][cohort][k][ij] - bm->boxes[ij].tr[k][den];
  314: 
  315:                     //fprintf(bm->logFile, "Time: %e box%d-%d %s-%d for %s has diffden: %e\n", bm->dayt, ij, k, FunctGroupArray[sp].groupCode, cohort, bm->contaminantStructure[cIndex]->contaminant_name, diffden[ij][k]);
  316: 
  317:                     // Number of contaminanted that must have moved
  318:                     localcontam[ij][k][cIndex] = bm->boxes[ij].tr[k][cid];
  319:                     if (diffden[ij][k] < 0.0) {
  320:                         num_contam_moved[ij][k][cIndex] = floor(bm->boxes[ij].tr[k][pid] * -1.0 * diffden[ij][k]);  // Used to say current_box, current_layer but tha makes no sense as calling from mvoement code so would always be final layer of final box.
  321:                     }
  322:                 }
  323:             }
  324:         }
  325: 
  326:         /* Now reallocate */
  327:         for (ij = 0; ij < bm->nbox; ij++) {
  328:             if (bm->boxes[ij].type != BOUNDARY) {
  329:                 for (k = 0; k < bm->boxes[ij].nz; k++) {
  330:                     // Move contaminated individuals
  331:                     if (num_contam_moved[ij][k][cIndex] < 1.0) {
  332:                         continue; //Nothing to do as nothing moved
  333:                     }
  334: 
  335:                     /* Individuals have moved away - so move them to nearest box with new additional individuals */
  336:                     // Do layers in same box
  337:                     for (clayer = 0; clayer < bm->wcnz; clayer++) {
  338:                         if ( clayer == k )
  339:                             continue;
  340: 
  341:                         if (diffden[ij][clayer] > 0.0) {
  342:                             num_possible = min(num_contam_moved[ij][k][cIndex], diffden[ij][clayer]);
  343: 
  344:                             // New contam level - average of what is there and what moved in
  345:                             bm->boxes[ij].tr[clayer][cid] = (bm->boxes[ij].tr[clayer][cid] * bm->boxes[ij].tr[clayer][pid] * this_currentden[sp][cohort][clayer][ij] + num_possible * localcontam[ij][k][cIndex]) / (num_possible + bm->boxes[ij].tr[clayer][pid] * this_currentden[sp][cohort][clayer][ij] + small_num);
  346: 
  347:                             // Adjust local proportion
  348:                             new_num_in_box = bm->boxes[ij].tr[clayer][pid] * this_currentden[sp][cohort][clayer][ij] + num_possible;
  349:                             bm->boxes[ij].tr[clayer][pid] = new_num_in_box / (bm->boxes[ij].tr[clayer][den] + small_num);  // Denominator used to say layer k not clayer, but that does not make sense as should be recieving layer as recalcualting proportion contaminanted in that layer not the old layer they left.
  350: 
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
**Listed lines:** 318, 345, 380

**Function:** `Move_Vert_Contaminated()`

**Why it's thought to be a bug**

`localcontam[...] = bm->boxes[ij].tr[k][cid]` with `cid = contaminant_tracer` (the *environmental* contaminant concentration), and lines 345/380 overwrite `tr[clayer][cid]` / `tr[next_box][clayer][cid]` with a fish-number-weighted average. Moving animals thus rewrites the water-column contaminant concentration (non-conservative). Intended quantity is the group's own tissue tracer `FunctGroupArray[sp].contaminantTracers[cohort][cIndex]`. Function: `Move_Vert_Contaminated()`.

**Current code** (`atecology/atContaminants.c` lines 316–320)

```c
  316: 
  317:                     // Number of contaminanted that must have moved
  318:                     localcontam[ij][k][cIndex] = bm->boxes[ij].tr[k][cid];
  319:                     if (diffden[ij][k] < 0.0) {
  320:                         num_contam_moved[ij][k][cIndex] = floor(bm->boxes[ij].tr[k][pid] * -1.0 * diffden[ij][k]);  // Used to say current_box, current_layer but tha makes no sense as calling from mvoement code so would always be final layer of final box.
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
**Listed lines:** 349, 384

**Function:** `Move_Vert_Contaminated()`

**Why it's thought to be a bug**

`tr[clayer][pid] = new_num_in_box / (bm->boxes[ij].tr[clayer][den] + small_num)` — numerator built from `this_currentden` (lines 348/383) but denominator is `tr[...][den]`; inconsistent sources, proportion is then clamped to 1 hiding the error. Function: `Move_Vert_Contaminated()`.

**Current code** (`atecology/atContaminants.c` lines 347–386)

```c
  347:                             // Adjust local proportion
  348:                             new_num_in_box = bm->boxes[ij].tr[clayer][pid] * this_currentden[sp][cohort][clayer][ij] + num_possible;
  349:                             bm->boxes[ij].tr[clayer][pid] = new_num_in_box / (bm->boxes[ij].tr[clayer][den] + small_num);  // Denominator used to say layer k not clayer, but that does not make sense as should be recieving layer as recalcualting proportion contaminanted in that layer not the old layer they left.
  350: 
  351:                             //fprintf(bm->logFile, "Time %e box%d-%d %s %d %s has after move prop: %e with new_num_in_box %e den: %e\n", bm->dayt, ij, clayer, FunctGroupArray[sp].groupCode, cohort, bm->contaminantStructure[cIndex]->contaminant_name, bm->boxes[ij].tr[clayer][pid], new_num_in_box, bm->boxes[ij].tr[k][den]);
  352: 
  353:                             // Sanity check
  354:                             if (bm->boxes[ij].tr[clayer][pid] < 0.0) {
  355:                                 bm->boxes[ij].tr[clayer][pid] = 0.0;
  356:                             }
  357:                             if (bm->boxes[ij].tr[clayer][pid] > 1.0) {
  358:                                 bm->boxes[ij].tr[clayer][pid] = 1.0;
  359:                             }
  360: 
  361:                             // Update the counters - both of source and sink
  362:                             num_contam_moved[ij][k][cIndex] -= num_possible;
  363:                             diffden[ij][clayer] -= num_possible;
  364:                         }
  365: 
  366:                     }
  367: 
  368:                     // Other boxes
  369:                     nextij = 0;
  370:                     while ((num_contam_moved[ij][k][cIndex] > 1.0) && (nextij < bm->nbox)) {
  371:                         next_box = bm->boxes[ij].distID[nextij];
  372: 
  373:                         if(next_box != ij) {
  374:                             for (clayer = 0; clayer < bm->wcnz; clayer++) {
  375:                                 if (diffden[next_box][clayer] > 0.0) {
  376:                                     //Space for entrants
  377:                                     num_possible = min(num_contam_moved[ij][k][cIndex], diffden[next_box][clayer]);
  378: 
  379:                                     // New contam level - average of what is there and what moved in
  380:                                     bm->boxes[next_box].tr[clayer][cid] = (bm->boxes[next_box].tr[clayer][cid] * bm->boxes[next_box].tr[clayer][pid] * this_currentden[sp][cohort][clayer][next_box] + num_possible * localcontam[ij][k][cIndex]) / (num_possible + bm->boxes[next_box].tr[clayer][pid] * this_currentden[sp][cohort][clayer][next_box] + small_num);
  381: 
  382:                                     // Adjust local proportion
  383:                                     new_num_in_box = bm->boxes[next_box].tr[clayer][pid] * this_currentden[sp][cohort][clayer][next_box] + num_possible;
  384:                                     bm->boxes[next_box].tr[clayer][pid] =  new_num_in_box / (bm->boxes[next_box].tr[clayer][den] + small_num);
  385: 
  386:                                     //fprintf(bm->logFile, "Time %e next_box%d-%d %s %d %s has after move prop: %e with  new_num_in_box %e den: %e\n", bm->dayt, next_box, clayer, FunctGroupArray[sp].groupCode, cohort, bm->contaminantStructure[cIndex]->contaminant_name, bm->boxes[next_box].tr[clayer][pid], new_num_in_box, bm->boxes[next_box].tr[clayer][den]);
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
**Listed lines:** 427-431, 435

**Function:** `Change_Contaminant_Levels()`

**Why it's thought to be a bug**

For `habitat == EPIFAUNA` (called from atecology.c:2553) `tracerArray = getTracerArray(boxLayerInfo, EPIFAUNA)` = `localEPITracers`, then `cLevel = tracerArray[contaminant_tracer]` indexes the epibenthic array with a water-column tracer id (wrong value, possible OOB). `Degrade_Contaminants()` then sets `amount_decayed` from this, which is subtracted in the EPIFAUNA branch of `Calculate_Contaminants_Flux()` (lines 570-571). Compare `Species_Contaminant_Uptake()` line 749 and `Calculate_Species_Contaminant_Effects()` line 1617 which use the WC array for the environmental level under EPIFAUNA. Function: `Change_Contaminant_Levels()`.

**Current code** (`atecology/atContaminants.c` lines 425–437)

```c
  425:     int cIndex;
  426:     double cLevel;
  427:     double *tracerArray = getTracerArray(boxLayerInfo, habitat);
  428: 
  429:     for (cIndex = 0; cIndex < bm->num_contaminants; cIndex++) {
  430:         /* Grab the level in the water column or the sediment */
  431:         cLevel = tracerArray[bm->contaminantStructure[cIndex]->contaminant_tracer];
  432: 
  433:         Calculate_Species_Contaminant_Decay(bm, boxLayerInfo, habitat, dtsz, cIndex, cLevel);
  434:         Species_Contaminant_Uptake(bm, boxLayerInfo, habitat, dtsz, cIndex);
  435:         Degrade_Contaminants(bm, boxLayerInfo, habitat, dtsz, cIndex, cLevel);
  436:     }
  437:     Calculate_Contaminants_Flux(bm, boxLayerInfo, habitat);
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
**Listed lines:** 748-752, 770

**Function:** `Species_Contaminant_Uptake()`

**Why it's thought to be a bug**

For EPIFAUNA habitat `tracerArray` is set to WC, but `cGroupLevel = tracerArray[FunctGroupArray[sp].contaminantTracers[cohort][cIndex]]` — for epibenthic groups those tracer ids index the epi array (`epi[pid]` is correctly used at line 878). So group level is read from the wrong array, corrupting uptake (linear/sigmoid uses `cLevel - cGroupLevel`). `Gain_Contaminants()` (678-682) handles this correctly. Function: `Species_Contaminant_Uptake()`.

**Current code** (`atecology/atContaminants.c` lines 746–772)

```c
  746: 	CONTAMINANT_UPTAKE_OPTION uptake_option;
  747: 
  748: 	if (habitat == EPIFAUNA) {
  749: 		tracerArray = getTracerArray(boxLayerInfo, WC);
  750: 	} else {
  751: 		tracerArray = getTracerArray(boxLayerInfo, habitat);
  752: 	}
  753: 
  754: 	/* For each contaminant calculate the uptake */
  755: 
  756:     /* Grab the level in the water column or the sediment */
  757:     cLevel = tracerArray[bm->contaminantStructure[cIndex]->contaminant_tracer];
  758:     if(cLevel > bm->min_pool){
  759:         //fprintf(bm->logFile, "time %e, box %d, layer %d, cLevel = %e\n", bm->dayt, bm->current_box, bm->current_layer, cLevel);
  760:         for (sp = 0; sp < bm->K_num_tot_sp; sp++) {
  761: 
  762:             /* Contact can occur for all active groups that are present in this habitat */
  763:             if ((FunctGroupArray[sp].speciesParams[flag_id] == TRUE) && (FunctGroupArray[sp].isDetritus == FALSE) && (FunctGroupArray[sp].habitatCoeffs[habitat] > 0)) {
  764:                 
  765:                 uptake_option = (CONTAMINANT_UPTAKE_OPTION)bm->contaminantStructure[cIndex]->sp_uptake_option[sp];
  766:                 uptake_rate = bm->contaminantStructure[cIndex]->sp_uptake_rate[sp];
  767: 
  768:                 for(cohort = 0; cohort < FunctGroupArray[sp].numCohortsXnumGenes; cohort++){
  769:                     /* The current concentration in the group */
  770:                     cGroupLevel = tracerArray[FunctGroupArray[sp].contaminantTracers[cohort][cIndex]];
  771:                     cUptake = 0;
  772: 
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
**Listed lines:** 966, 974, 1049-1050

**Function:** `Group_Transfer_Contaminant()`

**Why it's thought to be a bug**

`amountTransfer` is overwritten with a proportion (`amountTransfer / totalBiomass`) at 966/974, but at 1049 `amt_exchanged = amountTransfer * dtsz; prop_exchanged = amt_exchanged / toGuild_totalBiomass` treats it as a biomass amount — proportion divided by receiver biomass. Function: `Group_Transfer_Contaminant()`.

**Current code** (`atecology/atContaminants.c` lines 964–968)

```c
  964: 	if (FunctGroupArray[fromGuild].groupAgeType == AGE_STRUCTURED) {
  965:         totalBiomass = (spSPinfo[fromGuild][fromCohort][SN_id] + spSPinfo[fromGuild][fromCohort][RN_id]) * spSPinfo[fromGuild][fromCohort][DEN_id] / bm->cell_vol;
  966:         amountTransfer = amountTransfer / (totalBiomass + small_num);
  967: 	} else {
  968: 		//totalBiomass = tracerArray[FunctGroupArray[fromGuild].totNTracers[fromCohort]];
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
**Listed lines:** 957-961, 997, 1058

**Function:** `Group_Transfer_Contaminant()`

**Why it's thought to be a bug**

`tracerArray` chosen by `fromGuild` habitat (EPIFAUNA → epi array) but used for the *toGuild* `totNTracers[toCohort]` (997) and `NumsTracers[toCohort]` (1058). When prey is epifaunal and predator is not (or vice versa) the predator's indices are read from the wrong array. Function: `Group_Transfer_Contaminant()`.

**Current code** (`atecology/atContaminants.c` lines 955–963)

```c
  955:     }
  956: 
  957: 	if(FunctGroupArray[fromGuild].habitatType == EPIFAUNA){
  958: 	  tracerArray = getTracerArray(boxLayerInfo, EPIFAUNA);
  959: 	} else {
  960: 	  tracerArray = getTracerArray(boxLayerInfo, habitat);
  961: 	}
  962: 
  963: 	/* Work out the proportion transfered */
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
**Listed lines:** 1436, 1449-1450

**Function:** `Get_ContamReproductionEffects()`

**Why it's thought to be a bug**

`if (step2 < C_reprod_corr[cohort]) C_reprod_corr[cohort] *= step2;` — comment says "taking the minimum value only" but it multiplies, compounding the effect across contaminants. Function: `Get_ContamReproductionEffects()`.

**Current code** (`atecology/atContaminants.c` lines 1434–1452)

```c
 1434:   double step1, step2, L_sp_r, b_sp_r, a_sp_r;
 1435:   
 1436:   if ( cohort < age_mat ) // To young for it to matter.
 1437:     return;
 1438:   
 1439:   switch (bm->flag_contamReprodModel){
 1440:   case NoReprodEffects: // No repro effects
 1441:     FunctGroupArray[species].C_reprod_corr[cohort] = 1.0;
 1442:     break;
 1443:   case Lovindeeretal: //Equation 13 Lovideer et al. paper  
 1444:     if (cGroupLevel > chronicLevel) {
 1445:       step1 = bm->contaminantStructure[cIndex]->sp_ReprodEffect[species] * cGroupLevel;
 1446:       if (step1 > 1.0)
 1447:         step1 = 1.0;
 1448:       step2 = 1.0 - step1;
 1449:       if (step2 < FunctGroupArray[species].C_reprod_corr[cohort]) // As taking the minimum value only
 1450:         FunctGroupArray[species].C_reprod_corr[cohort] *= step2;
 1451:     }
 1452:     break;
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
**Listed lines:** 1433-1437

**Function:** `Get_ContamReproductionEffects()`

**Why it's thought to be a bug**

`if (cohort < age_mat) return;` — `cohort` is the cohort×genotype index (caller loops `numCohortsXnumGenes`, line 1642) while `age_mat` is in age classes. Function: `Get_ContamReproductionEffects()`.

**Current code** (`atecology/atContaminants.c` lines 1431–1439)

```c
 1431: void Get_ContamReproductionEffects(MSEBoxModel *bm, double cGroupLevel, int species, int cohort, int cIndex) {
 1432:   double chronicLevel = bm->contaminantStructure[cIndex]->sp_maxChronicConc[species];  // chronic level
 1433:   int age_mat = (int) (FunctGroupArray[species].speciesParams[age_mat_id]);
 1434:   double step1, step2, L_sp_r, b_sp_r, a_sp_r;
 1435:   
 1436:   if ( cohort < age_mat ) // To young for it to matter.
 1437:     return;
 1438:   
 1439:   switch (bm->flag_contamReprodModel){
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
**Listed lines:** 1558

**Function:** `Get_Contam_Amplification()`

**Why it's thought to be a bug**

`quit("No such contaminant interaction option (%d), ...")` has a `%d` but no argument — undefined behaviour. Function: `Get_Contam_Amplification()`.

**Current code** (`atecology/atContaminants.c` lines 1556–1560)

```c
 1556:                 break;
 1557:             default:
 1558:                 quit("No such contaminant interaction option (%d), must be no interaciton (0), additive (1), multiplicative (2), most limiting (3)\n");
 1559:                 break;
 1560:         }
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
**Listed lines:** 1795-1797

**Function:** `Age_Contaminants_Update()`

**Why it's thought to be a bug**

`cid_prev = FunctGroupArray[sp].contaminantTracers[cohort-1][cIndex];` — the source cohort for aging is `cohort - numGeneTypes` (nextcid = cohort + sp_numGeneTypes, atdemography.c:2630); with genotypes, `cohort-1` is a different genotype of the same/younger age. Function: `Age_Contaminants_Update()`.

**Current code** (`atecology/atContaminants.c` lines 1793–1799)

```c
 1793:     for (cIndex = 0; cIndex < bm->num_contaminants; cIndex++) {
 1794:         cid = FunctGroupArray[sp].contaminantTracers[cohort][cIndex];
 1795:         cid_prev = FunctGroupArray[sp].contaminantTracers[cohort-1][cIndex];
 1796:         bm->boxes[ij].tr[k][cid] += FunctGroupArray[sp].agingContam[cohort][cIndex][ij][k];
 1797:         bm->boxes[ij].tr[k][cid_prev] -= FunctGroupArray[sp].agingContam[cohort][cIndex][ij][k];
 1798:         // Add and subtract stuff being transferred (need to add and subtract here as not moved elsewhere anymore for clarity sake)
 1799:         //TODO: Need to think about how to handle this if the new age group isn't present in this water column and layer...
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
**Listed lines:** 1834-1847

**Function:** `ContaminantMigrationIn()`

**Why it's thought to be a bug**

`time_step = bm->dayt - Leave_Now[mid]` is in days (Leave_Now set in days, atmigration.c:386), but `half_life` is in seconds (Degrade_Contaminants line 450 uses `dtsz / half_life` with dtsz in seconds). Decay during migration is underestimated by a factor 86400. Function: `ContaminantMigrationIn()`.

**Current code** (`atecology/atContaminants.c` lines 1832–1849)

```c
 1832:     int cIndex;
 1833:     double base_contam, this_half_life, final_contam;
 1834:     double day_start = (double)(MIGRATION[sp].Leave_Now[mid]);
 1835:     double day_end = bm->dayt;
 1836:     double time_step = day_end - day_start;
 1837: 
 1838:     for (cIndex = 0; cIndex < bm->num_contaminants; cIndex++) {
 1839:         base_contam = MIGRATION[sp].contam[cohort][mid][cIndex];
 1840:         
 1841:         if(bm->flag_contam_halflife_spbased) {
 1842:             this_half_life = bm->contaminantStructure[cIndex]->sp_decay_half_life[sp];
 1843:         } else {
 1844:             this_half_life = bm->contaminantStructure[cIndex]->half_life;
 1845:         }
 1846: 
 1847:         final_contam = base_contam * pow(0.5, time_step / this_half_life);
 1848:                 
 1849:         /* Alternative code -- Using the decay constant instead of the half life
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
**Listed lines:** 1873-1880

**Function:** `ContaminantMigrationOut()`

**Why it's thought to be a bug**

`newContam = 0.0` (assignment commented out `//newContam = ;`) so every batch of leavers dilutes the migrant pool's contaminant level towards zero: `(origContam*oldden + 0*num_leaving)/(...)`. Leavers' tissue burden is lost. Function: `ContaminantMigrationOut()`.

**Current code** (`atecology/atContaminants.c` lines 1871–1882)

```c
 1871: void ContaminantMigrationOut(MSEBoxModel *bm, int sp, int cohort, int mid, double oldden, double num_leaving) {
 1872:     int cIndex;
 1873:     double newContam = 0.0, origContam;
 1874:     
 1875:     // For species without live birth and maternal care
 1876:     for (cIndex = 0; cIndex < bm->num_contaminants; cIndex++) {
 1877:         origContam = MIGRATION[sp].contam[cohort][mid][cIndex];
 1878:         //newContam = ;
 1879:         if(oldden > 0) {
 1880:             MIGRATION[sp].contam[cohort][mid][cIndex] = (origContam * oldden + newContam * num_leaving) / (num_leaving + oldden + small_num);
 1881:         } else {
 1882:             MIGRATION[sp].contam[cohort][mid][cIndex] = newContam;
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
**Listed lines:** 1909-1910, 1924

**Why it's thought to be a bug**

`reprodContam += cGroupLevel * maternal_transfer_rate` (unweighted concentration sum over cells) while `reprodContamCount += this_den` (numbers); `Store_Recruit_Contaminants()` then divides the two — not a density-weighted mean, and divides by zero when no spawners contributed (NaN). Functions: `Get_Parental_Contaminants()`, `Store_Recruit_Contaminants()`.

**Current code** (`atecology/atContaminants.c` lines 1907–1926)

```c
 1907:            cid = FunctGroupArray[sp].contaminantTracers[cohort][cIndex];
 1908:            cGroupLevel = bm->boxes[bm->current_box].tr[bm->current_layer][cid];
 1909:            FunctGroupArray[sp].reprodContam[cIndex] += (cGroupLevel * maternal_transfer_rate);
 1910:            FunctGroupArray[sp].reprodContamCount[cIndex] += this_den;
 1911:            bm->boxes[bm->current_box].tr[bm->current_layer][cid] -= (cGroupLevel * maternal_transfer_rate);
 1912:            // As lost to offspring instantaneously (as would be in external spawners)
 1913:        }
 1914:     }
 1915: }
 1916: 
 1917: /* Store contaminants per offspring until needed */
 1918: void Store_Recruit_Contaminants(MSEBoxModel *bm, int sp, int stock_id, int ngene, int qid) {
 1919:     int cIndex;
 1920:     int flagmother = (int) (FunctGroupArray[sp].speciesParams[flagmother_id]);
 1921:     
 1922:     if (bm->flag_contamMaternalTransfer && (flagmother < 1)) {  // Only those not using maternal care transfer here
 1923:         for (cIndex = 0; cIndex < bm->num_contaminants; cIndex++) {
 1924:             EMBRYO[sp].Contam[stock_id][ngene][qid][cIndex] = FunctGroupArray[sp].reprodContam[cIndex] / FunctGroupArray[sp].reprodContamCount[cIndex];  // Average contributed across the entire spawning population
 1925:         }
 1926:     }
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
**Listed lines:** 1993-1995

**Function:** `Get_Settler_Contaminants()`

**Why it's thought to be a bug**

`drandom(age_mat, numCohortsXnumGenes)` mixes age-class units (age_mat) with cohort×genotype count and ignores `ngene`, so the chosen "mother" cohort may be the wrong genotype/age (also overruns contaminantTracers per CONT-01). Function: `Get_Settler_Contaminants()`.

**Current code** (`atecology/atContaminants.c` lines 1991–1997)

```c
 1991:     int flagmother = (int) (FunctGroupArray[sp].speciesParams[flagmother_id]);
 1992:     double this_reprodContam, maternal_transfer_rate, cGroupLevel;
 1993:     double start_n = FunctGroupArray[sp].speciesParams[age_mat_id];
 1994:     double end_n = (double)FunctGroupArray[sp].numCohortsXnumGenes;
 1995:     int cohort = (int)(floor(drandom(start_n, end_n))); // Find a random adult cohort to use
 1996:     
 1997:     if (flagmother > 0) {
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
**Listed lines:** 2177, 2250, 2254

**Function:** `Check_Contam_Totals()`

**Why it's thought to be a bug**

`bm->forceTracerInput[cIndex].total_input` indexes the forcing-tracer array by contaminant index, but `Init_Contaminants()` (208-212) shows contaminants are matched to forcing entries by name, not position. If any non-contaminant forcing tracer precedes them, the wrong total is checked (spurious quit or missed check). Function: `Check_Contam_Totals()`.

**Current code** (`atecology/atContaminants.c` lines 2175–2179)

```c
 2175:         **/
 2176:         
 2177:         if(!bm->forceTracerInput[cIndex].total_input) {
 2178:             continue;  // Nothing to do so skip ahead
 2179:         }
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
**Listed lines:** 840, 1266

**Why it's thought to be a bug**

`expose_time[sp][cohort][current_box][current_layer]` is sized `wcnz+sednz` layers but in SED habitat `current_layer` is a sediment layer index (0..), colliding with WC layer slots. Same for `sp_point`/`sp_maxDoseToDate`. Functions: `Species_Contaminant_Uptake()`, `Get_ContamMortEffects()`.

**Current code** (`atecology/atContaminants.c` lines 838–842)

```c
  838:                     // Initiate exposure time
  839:                     if (!cGroupLevel && cUptake) {
  840:                         bm->contaminantStructure[cIndex]->expose_time[sp][cohort][bm->current_box][bm->current_layer] = bm->dayt;
  841:                     }
  842:                         
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
**Listed lines:** 1397

**Why it's thought to be a bug**

`if ((isPrimaryProducer == TRUE) && (isMicroFauna == TRUE))` — a group is unlikely to be both, so the Laender EC50 branch is effectively dead; `

**Current code** (`atecology/atContaminants.c` lines 1395–1399)

```c
 1395:         case InVitro_model:  // Growth effects as of InVitro
 1396:             if (cGroupLevel > bm->contaminantStructure[cIndex]->sp_GrowthThresh[species]) {
 1397:                 if ((FunctGroupArray[species].isPrimaryProducer == TRUE) && (FunctGroupArray[species].isMicroFauna == TRUE)){
 1398:                     step1 = 1.0 + pow((cGroupLevel / bm->contaminantStructure[cIndex]->sp_EC50[species]), bm->contaminantStructure[cIndex]->sp_ECslope[species]);
 1399:                     FunctGroupArray[species].C_growth_corr[cohort] *= 1.0 / step1; // As of Laender et al 2008
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
