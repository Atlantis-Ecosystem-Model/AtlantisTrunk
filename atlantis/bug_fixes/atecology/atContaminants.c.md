# atecology/atContaminants.c — bug review

Generated 2026-11-19 from `atlantis/to-do-fixt-list.md`. **19 findings** (severity H: 6; confidence H and severity H: 3; on the priority list: 3).

Each finding's **Code at issue** is located by searching the current code for the code quoted in the review, not by line number. Use the **Search for** text with your editor's Find, because line numbers will keep changing as fixes go in. → marks the matching line. If only a close match or a name could be found, the block says so. ⚠ means the quoted code is no longer in the file: it may have been changed or already fixed. **Lines at review** are the October numbers. Line numbers in the description text are approximate.

Conf/Sev: confidence it's a bug / severity if it is (H = wrong results or crash, M = wrong in some configs, L = cosmetic, log, leak or latent). ✔ = the reviewer re-checked it by hand. ⭐ = on the high-priority list.

## Summary

| ID | Found at | Conf/Sev | Category | Short description | Decision | Done |
|---|---|---|---|---|---|---|
| [CONT-01](#cont-01) 🔀 ⭐ | 486, 613, 303 | H/H | buffer overrun (cohort vs cohort×genotype) | `FunctGroupArray[i].contaminantTracers = Util_Alloc_Init_2D_Int(bm->num_contaminants, numCohorts, 0)` (atUtilG… |  | [ ] |
| [CONT-02](#cont-02) ⭐ | 309 | H/H | wrong variable / unit mismatch | `diffden[ij][k] = this_currentden[sp][cohort][k][ij] - bm->boxes[ij].tr[k][den];` — `currentden` passed from a… |  | [ ] |
| [CONT-03](#cont-03) | 314, 302, 341 | M/H | wrong tracer | `localcontam[...] = bm->boxes[ij].tr[k][cid]` with `cid = contaminant_tracer` (the *environmental* contaminant… |  | [ ] |
| [CONT-04](#cont-04) | 345 | M/M | wrong denominator | `tr[clayer][pid] = new_num_in_box / (bm->boxes[ij].tr[clayer][den] + small_num)` — numerator built from `this_… |  | [ ] |
| [CONT-05](#cont-05) 🔀 ⭐ | 755, 685, 432 | H/H | wrong tracer array | For `habitat == EPIFAUNA` (called from atecology.c:2553) `tracerArray = getTracerArray(boxLayerInfo, EPIFAUNA)… |  | [ ] |
| [CONT-06](#cont-06) 🔀 | 1215, 898, 796 | M/H | wrong tracer array | For EPIFAUNA habitat `tracerArray` is set to WC, but `cGroupLevel = tracerArray[FunctGroupArray[sp].contaminan… |  | [ ] |
| [CONT-07](#cont-07) 🔀 | 993 | M/M | wrong variable / unit mismatch | `amountTransfer` is overwritten with a proportion (`amountTransfer / totalBiomass`) at 966/974, but at 1049 `a… |  | [ ] |
| [CONT-08](#cont-08) 🔀 | 1024, 1099 | M/M | wrong tracer array | `tracerArray` chosen by `fromGuild` habitat (EPIFAUNA → epi array) but used for the *toGuild* `totNTracers[toC… |  | [ ] |
| [CONT-09](#cont-09) | 1490 | M/M | wrong operator (copy) | `if (step2 < C_reprod_corr[cohort]) C_reprod_corr[cohort] *= step2;` — comment says "taking the minimum value … |  | [ ] |
| [CONT-10](#cont-10) | 1477 | M/M | cohort vs cohort×genotype | `if (cohort < age_mat) return;` — `cohort` is the cohort×genotype index (caller loops `numCohortsXnumGenes`, l… |  | [ ] |
| [CONT-11](#cont-11) | 1599 | H/L | printf format mismatch | `quit("No such contaminant interaction option (%d), ...")` has a `%d` but no argument — undefined behaviour. F… |  | [ ] |
| [CONT-12](#cont-12) | 1836 | H/M | wrong index (cohort×genotype) | `cid_prev = FunctGroupArray[sp].contaminantTracers[cohort-1][cIndex];` — the source cohort for aging is `cohor… |  | [ ] |
| [CONT-13](#cont-13) 🔀 | 456 | M/H | unit mismatch | `time_step = bm->dayt - Leave_Now[mid]` is in days (Leave_Now set in days, atmigration.c:386), but `half_life`… |  | [ ] |
| [CONT-14](#cont-14) | 1914, 1919 | M/M | uninitialised logic / incomplete | `newContam = 0.0` (assignment commented out `//newContam = ;`) so every batch of leavers dilutes the migrant p… |  | [ ] |
| [CONT-15](#cont-15) | 1950 | M/M | unit mismatch / division by zero | `reprodContam += cGroupLevel * maternal_transfer_rate` (unweighted concentration sum over cells) while `reprod… |  | [ ] |
| [CONT-16](#cont-16) 🔀 | 2034 | L/M | cohort vs cohort×genotype | `drandom(age_mat, numCohortsXnumGenes)` mixes age-class units (age_mat) with cohort×genotype count and ignores… |  | [ ] |
| [CONT-17](#cont-17) | 2230 | M/M | wrong index | `bm->forceTracerInput[cIndex].total_input` indexes the forcing-tracer array by contaminant index, but `Init_Co… |  | [ ] |
| [CONT-18](#cont-18) 🔀 | 860, 163 | L/L | wrong index (layer collision) | `expose_time[sp][cohort][current_box][current_layer]` is sized `wcnz+sednz` layers but in SED habitat `current… |  | [ ] |
| [CONT-19](#cont-19) | 1438 | L/M | wrong operator | `if ((isPrimaryProducer == TRUE) && (isMicroFauna == TRUE))` — a group is unlikely to be both, so the Laender … |  | [ ] |

---

## CONT-01

**buffer overrun (cohort vs cohort×genotype)** · Conf H / Sev H ⭐ priority  
**Found at (current code):** `atContaminants.c` 486, `atContaminants.c` 613, `atContaminants.c` 303
**Lines at review (Oct 2026):** 307, 480, 592, 707, 768, 1008, 1174, 1498, 1541, 1655, 1780, 1794, 1864, 2000, 2023, 2043, 2212 (and atUtilGroupIO.c:910-914)

**Why it's thought to be a bug**

`FunctGroupArray[i].contaminantTracers = Util_Alloc_Init_2D_Int(bm->num_contaminants, numCohorts, 0)` (atUtilGroupIO.c:909) gives `[numCohorts][ncont]`; same for `contamPropTracers` (931), `contaminantSpMort` (912, 1D `numCohorts`) and `agingContam` (914, first dim `numCohorts`). atBuildTracer.c:385 also only builds tracers for `cohort < numCohorts`. But every loop in this file runs `cohort < numCohortsXnumGenes` and indexes `contaminantTracers[cohort][cIndex]`, `contamPropTracers[cohort][...]`, `contaminantSpMort[cohort]`, and `agingContam[nextcid]` (nextcid = cohort+numGeneTypes, atdemography.c:2634). With numGeneTypes>1 this reads/writes past the allocations (and uses garbage tracer ids). Functions: `Calculate_Containinant_Flux_Habitat()`, `Calculate_Contaminants_Flux()`, `Species_Contaminant_Uptake()`, `Calculate_Species_Contaminant_Decay()`, `Calculate_Species_Contaminant_Effects()`, `Age_Contaminants_Store()`, `Check_Contam_Totals()`, etc.

> 🔀 **Changed by the merge of main (Nov 2026):** the code for this finding was changed by the merge. Check whether this finding is now fixed before acting.

**Code at issue** — `atContaminants.c` line 486 (closest match to the quoted code; wording differs slightly)  
Search for: `for (cohort = 0; cohort < FunctGroupArray[groupIndex].numCohortsXnumGenes; cohort++) {`

```c
  484: 		/* loss of flux is sum of decay and all the uptake groups */
  485: 		for (groupIndex = 0; groupIndex < bm->K_num_tot_sp; groupIndex++) {
  486→ 			for (cohort = 0; cohort < FunctGroupArray[groupIndex].numCohortsXnumGenes; cohort++) {
  487: 				if (FunctGroupArray[groupIndex].speciesParams[flag_id] == TRUE) {
  488:                     uptakeSum += bm->contaminantStructure[cIndex]->sp_uptake[groupIndex][cohort][habitat];
```

**Code at issue** — `atContaminants.c` line 613  
Search for: `contaminantTracers[cohort][cIndex]`

```c
  611: 								bm->contaminantStructure[cIndex]->sp_amount_decayed[groupIndex][cohort];
  612: 
  613→                         groupFluxArray[FunctGroupArray[groupIndex].contaminantTracers[cohort][cIndex]] = flux;
  614:                         
  615:                         /**/
```

**Code at issue** — `atContaminants.c` line 303  
Search for: `contamPropTracers[cohort][`

```c
  301:     for (cIndex = 0; cIndex < bm->num_contaminants; cIndex++) {
  302:         cid = bm->contaminantStructure[cIndex]->contaminant_tracer;
  303→         pid = FunctGroupArray[sp].contamPropTracers[cohort][cIndex];
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
**Found at (current code):** `atContaminants.c` 309
**Lines at review (Oct 2026):** 313, 345, 348, 380, 383

**Function:** `Move_Vert_Contaminated()`

**Why it's thought to be a bug**

`diffden[ij][k] = this_currentden[sp][cohort][k][ij] - bm->boxes[ij].tr[k][den];` — `currentden` passed from atmovement.c:2930 holds *proportions* of the total (`currentden = tr[k][den] / totden`, atmovement.c:1545) taken before movement, while `tr[k][den]` at the call point already holds the post-move *numbers* (atmovement.c:2800). So diffden subtracts numbers from a proportion (almost always negative, and sign is reversed vs the intended "new − old"), and lines 341/344/376/379 multiply proportion by `this_currentden` as if it were a count. The contaminated-proportion reallocation is therefore meaningless. Function: `Move_Vert_Contaminated()`.

**Code at issue** — `atContaminants.c` line 309  
Search for: `diffden[ij][k] = this_currentden[sp][cohort][k][ij] - bm->boxes[ij].tr[k][den]`

```c
  307:                 for (k = 0; k < bm->boxes[ij].nz; k++) {
  308:                     // Change in numbers
  309→                     diffden[ij][k] = this_currentden[sp][cohort][k][ij] - bm->boxes[ij].tr[k][den];
  310: 
  311:                     //fprintf(bm->logFile, "Time: %e box%d-%d %s-%d for %s has diffden: %e\n", bm->dayt, ij, k, FunctGroupArray[sp].groupCode, cohort, bm->contaminantStructure[cIndex]->contaminant_name, diffden[ij][k]);
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
**Found at (current code):** `atContaminants.c` 314, `atContaminants.c` 302, `atContaminants.c` 341
**Lines at review (Oct 2026):** 318, 345, 380

**Function:** `Move_Vert_Contaminated()`

**Why it's thought to be a bug**

`localcontam[...] = bm->boxes[ij].tr[k][cid]` with `cid = contaminant_tracer` (the *environmental* contaminant concentration), and lines 341/376 overwrite `tr[clayer][cid]` / `tr[next_box][clayer][cid]` with a fish-number-weighted average. Moving animals thus rewrites the water-column contaminant concentration (non-conservative). Intended quantity is the group's own tissue tracer `FunctGroupArray[sp].contaminantTracers[cohort][cIndex]`. Function: `Move_Vert_Contaminated()`.

**Code at issue** — `atContaminants.c` line 314  
Search for: `localcontam[`

```c
  312: 
  313:                     // Number of contaminanted that must have moved
  314→                     localcontam[ij][k][cIndex] = bm->boxes[ij].tr[k][cid];
  315:                     if (diffden[ij][k] < 0.0) {
  316:                         num_contam_moved[ij][k][cIndex] = floor(bm->boxes[ij].tr[k][pid] * -1.0 * diffden[ij][k]);  // Used to say current_box, current_layer but tha makes no sense as calling from mvoement code so would always be final layer of final box.
```

**Code at issue** — `atContaminants.c` line 302 (closest match to the quoted code; wording differs slightly)  
Search for: `cid = bm->contaminantStructure[cIndex]->contaminant_tracer;`

```c
  300:     
  301:     for (cIndex = 0; cIndex < bm->num_contaminants; cIndex++) {
  302→         cid = bm->contaminantStructure[cIndex]->contaminant_tracer;
  303:         pid = FunctGroupArray[sp].contamPropTracers[cohort][cIndex];
  304:         
```

**Code at issue** — `atContaminants.c` line 341  
Search for: `tr[clayer][cid]`

```c
  339: 
  340:                             // New contam level - average of what is there and what moved in
  341→                             bm->boxes[ij].tr[clayer][cid] = (bm->boxes[ij].tr[clayer][cid] * bm->boxes[ij].tr[clayer][pid] * this_currentden[sp][cohort][clayer][ij] + num_possible * localcontam[ij][k][cIndex]) / (num_possible + bm->boxes[ij].tr[clayer][pid] * this_currentden[sp][cohort][clayer][ij] + small_num);
  342: 
  343:                             // Adjust local proportion
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
**Found at (current code):** `atContaminants.c` 345
**Lines at review (Oct 2026):** 349, 384

**Function:** `Move_Vert_Contaminated()`

**Why it's thought to be a bug**

`tr[clayer][pid] = new_num_in_box / (bm->boxes[ij].tr[clayer][den] + small_num)` — numerator built from `this_currentden` (lines 344/379) but denominator is `tr[...][den]`; inconsistent sources, proportion is then clamped to 1 hiding the error. Function: `Move_Vert_Contaminated()`.

**Code at issue** — `atContaminants.c` line 345  
Search for: `tr[clayer][pid] = new_num_in_box / (bm->boxes[ij].tr[clayer][den] + small_num)`

```c
  343:                             // Adjust local proportion
  344:                             new_num_in_box = bm->boxes[ij].tr[clayer][pid] * this_currentden[sp][cohort][clayer][ij] + num_possible;
  345→                             bm->boxes[ij].tr[clayer][pid] = new_num_in_box / (bm->boxes[ij].tr[clayer][den] + small_num);  // Denominator used to say layer k not clayer, but that does not make sense as should be recieving layer as recalcualting proportion contaminanted in that layer not the old layer they left.
  346: 
  347:                             //fprintf(bm->logFile, "Time %e box%d-%d %s %d %s has after move prop: %e with new_num_in_box %e den: %e\n", bm->dayt, ij, clayer, FunctGroupArray[sp].groupCode, cohort, bm->contaminantStructure[cIndex]->contaminant_name, bm->boxes[ij].tr[clayer][pid], new_num_in_box, bm->boxes[ij].tr[k][den]);
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
**Found at (current code):** `atContaminants.c` 755, `atContaminants.c` 685, `atContaminants.c` 432
**Lines at review (Oct 2026):** 427-431, 435

**Function:** `Change_Contaminant_Levels()`

**Why it's thought to be a bug**

For `habitat == EPIFAUNA` (called from atecology.c:2553) `tracerArray = getTracerArray(boxLayerInfo, EPIFAUNA)` = `localEPITracers`, then `cLevel = tracerArray[contaminant_tracer]` indexes the epibenthic array with a water-column tracer id (wrong value, possible OOB). `Degrade_Contaminants()` then sets `amount_decayed` from this, which is subtracted in the EPIFAUNA branch of `Calculate_Contaminants_Flux()` (lines 576-577). Compare `Species_Contaminant_Uptake()` line ≈757 and `Calculate_Species_Contaminant_Effects()` line 1658 which use the WC array for the environmental level under EPIFAUNA. Function: `Change_Contaminant_Levels()`.

> 🔀 **Changed by the merge of main (Nov 2026):** the code for this finding was changed by the merge. Check whether this finding is now fixed before acting.

**Code at issue** — `atContaminants.c` line 755  
Search for: `habitat == EPIFAUNA`

```c
  753: 
  754: 	/* For EPIFAUNA groups: environmental concentration comes from WC, but organism concentration lives in EPIFAUNA */
  755→ 	if (habitat == EPIFAUNA) {
  756: 		environmentTracerArray = getTracerArray(boxLayerInfo, WC);      /* Water column concentration (forced from outside) */
  757: 		organismTracerArray = getTracerArray(boxLayerInfo, EPIFAUNA);   /* Organism concentration in EPIFAUNA */
```

**Code at issue** — `atContaminants.c` line 685  
Search for: `tracerArray = getTracerArray(boxLayerInfo, EPIFAUNA)`

```c
  683: 
  684:     if(FunctGroupArray[species].habitatType == EPIFAUNA){
  685→       tracerArray = getTracerArray(boxLayerInfo, EPIFAUNA);
  686:     } else {
  687:       tracerArray = getTracerArray(boxLayerInfo, globalHabitat);
```

**Code at issue** — `atContaminants.c` line 432 (closest match to the quoted code; wording differs slightly)  
Search for: `cLevel = tracerArray[bm->contaminantStructure[cIndex]->contaminant_tracer];`

```c
  430:     for (cIndex = 0; cIndex < bm->num_contaminants; cIndex++) {
  431:         /* Grab the level in the water column or the sediment */
  432→         cLevel = tracerArray[bm->contaminantStructure[cIndex]->contaminant_tracer];
  433: 
  434:         Calculate_Species_Contaminant_Decay(bm, boxLayerInfo, habitat, dtsz, cIndex, cLevel);
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
**Found at (current code):** `atContaminants.c` 1215, `atContaminants.c` 898, `atContaminants.c` 796
**Lines at review (Oct 2026):** 748-752, 770

**Function:** `Species_Contaminant_Uptake()`

**Why it's thought to be a bug**

For EPIFAUNA habitat `tracerArray` is set to WC, but `cGroupLevel = tracerArray[FunctGroupArray[sp].contaminantTracers[cohort][cIndex]]` — for epibenthic groups those tracer ids index the epi array (`epi[pid]` is correctly used at line 898). So group level is read from the wrong array, corrupting uptake (linear/sigmoid uses `cLevel - cGroupLevel`). `Gain_Contaminants()` (684-688) handles this correctly. Function: `Species_Contaminant_Uptake()`.

> 🔀 **Changed by the merge of main (Nov 2026):** the code for this finding was changed by the merge. Check whether this finding is now fixed before acting.

**Code at issue** — `atContaminants.c` line 1215  
Search for: `cGroupLevel = tracerArray[FunctGroupArray[sp].contaminantTracers[cohort][cIndex]]`

```c
 1213:             for(cohort = 0; cohort < FunctGroupArray[sp].numCohortsXnumGenes; cohort++){
 1214:                 
 1215→                 local_cGroupLevel = tracerArray[FunctGroupArray[sp].contaminantTracers[cohort][cIndex]];  // Need to use local cGroupLevel as needs to be per cohort not at species level
 1216: 
 1217:                 /* The current concentration in the group */
```

**Code at issue** — `atContaminants.c` line 898  
Search for: `epi[pid]`

```c
  896:                             break;
  897:                         case EPIFAUNA:
  898→                             bm->boxes[bm->current_box].epi[pid] += propContam;
  899:                             if (bm->boxes[bm->current_box].epi[pid] > 1.0) {
  900:                                 bm->boxes[bm->current_box].epi[pid] = 1.0;
```

**Code at issue** — `atContaminants.c` line 796  
Search for: `cLevel - cGroupLevel`

```c
  794:                     case linear_contaminant_uptake_id: /* The linear contaminant formula accounting for internal content */
  795:                         //cUptake = uptake_rate * cLevel;
  796→                         cUptake = uptake_rate * (cLevel - cGroupLevel);
  797:                         if (cUptake < 0) {
  798:                             cUptake = 0;
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
**Found at (current code):** `atContaminants.c` 993
**Lines at review (Oct 2026):** 966, 974, 1049-1050

**Function:** `Group_Transfer_Contaminant()`

**Why it's thought to be a bug**

`amountTransfer` is overwritten with a proportion (`amountTransfer / totalBiomass`) at 966/974, but at 1049 `amt_exchanged = amountTransfer * dtsz; prop_exchanged = amt_exchanged / toGuild_totalBiomass` treats it as a biomass amount — proportion divided by receiver biomass. Function: `Group_Transfer_Contaminant()`.

> 🔀 **Changed by the merge of main (Nov 2026):** the code for this finding was changed by the merge. Check whether this finding is now fixed before acting.

**Code at issue** — `atContaminants.c` line 993 (closest match to the quoted code; wording differs slightly)  
Search for: `amountTransfer = amountTransfer / (totalBiomass + small_num);`

```c
  991: 	if (FunctGroupArray[fromGuild].groupAgeType == AGE_STRUCTURED) {
  992:         totalBiomass = (spSPinfo[fromGuild][fromCohort][SN_id] + spSPinfo[fromGuild][fromCohort][RN_id]) * spSPinfo[fromGuild][fromCohort][DEN_id] / bm->cell_vol;
  993→         amountTransfer = amountTransfer / (totalBiomass + small_num);
  994: 	} else {
  995: 		//totalBiomass = tracerArray[FunctGroupArray[fromGuild].totNTracers[fromCohort]];
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
**Found at (current code):** `atContaminants.c` 1024, `atContaminants.c` 1099
**Lines at review (Oct 2026):** 957-961, 997, 1058

**Function:** `Group_Transfer_Contaminant()`

**Why it's thought to be a bug**

`tracerArray` chosen by `fromGuild` habitat (EPIFAUNA → epi array) but used for the *toGuild* `totNTracers[toCohort]` (≈1024) and `NumsTracers[toCohort]` (≈1099). When prey is epifaunal and predator is not (or vice versa) the predator's indices are read from the wrong array. Function: `Group_Transfer_Contaminant()`.

> 🔀 **Changed by the merge of main (Nov 2026):** the code for this finding was changed by the merge. Check whether this finding is now fixed before acting.

**Code at issue** — `atContaminants.c` line 1024  
Search for: `totNTracers[toCohort]`

```c
 1022:             }
 1023:         } else {
 1024→             toGuild_totalBiomass = predatorTracerArray[FunctGroupArray[toGuild].totNTracers[toCohort]];
 1025:             
 1026:             if(isnan(toGuild_totalBiomass)) {
```

**Code at issue** — `atContaminants.c` line 1099  
Search for: `NumsTracers[toCohort]`

```c
 1097:             min_num = 0.0;
 1098:             if(FunctGroupArray[toGuild].groupAgeType == AGE_STRUCTURED ){
 1099→                 this_num = predatorTracerArray[FunctGroupArray[toGuild].NumsTracers[toCohort]];
 1100:                 min_num = 1.0 / (this_num + small_num);
 1101:             } else {
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
**Found at (current code):** `atContaminants.c` 1490
**Lines at review (Oct 2026):** 1436, 1449-1450

**Function:** `Get_ContamReproductionEffects()`

**Why it's thought to be a bug**

`if (step2 < C_reprod_corr[cohort]) C_reprod_corr[cohort] *= step2;` — comment says "taking the minimum value only" but it multiplies, compounding the effect across contaminants. Function: `Get_ContamReproductionEffects()`.

**Code at issue** — `atContaminants.c` line 1490 (the line the review pointed to, found by its text)  
Search for: `if (step2 < FunctGroupArray[species].C_reprod_corr[cohort]) // As taking the minimum value`

```c
 1488:         step1 = 1.0;
 1489:       step2 = 1.0 - step1;
 1490→       if (step2 < FunctGroupArray[species].C_reprod_corr[cohort]) // As taking the minimum value only
 1491:         FunctGroupArray[species].C_reprod_corr[cohort] *= step2;
 1492:     }
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
**Found at (current code):** `atContaminants.c` 1477
**Lines at review (Oct 2026):** 1433-1437

**Function:** `Get_ContamReproductionEffects()`

**Why it's thought to be a bug**

`if (cohort < age_mat) return;` — `cohort` is the cohort×genotype index (caller loops `numCohortsXnumGenes`, line 1683) while `age_mat` is in age classes. Function: `Get_ContamReproductionEffects()`.

**Code at issue** — `atContaminants.c` line 1477 (closest match to the quoted code; wording differs slightly)  
Search for: `if ( cohort < age_mat ) // To young for it to matter.`

```c
 1475:   double step1, step2, L_sp_r, b_sp_r, a_sp_r;
 1476:   
 1477→   if ( cohort < age_mat ) // To young for it to matter.
 1478:     return;
 1479:   
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
**Found at (current code):** `atContaminants.c` 1599
**Lines at review (Oct 2026):** 1558

**Function:** `Get_Contam_Amplification()`

**Why it's thought to be a bug**

`quit("No such contaminant interaction option (%d), ...")` has a `%d` but no argument — undefined behaviour. Function: `Get_Contam_Amplification()`.

**Code at issue** — `atContaminants.c` line 1599  
Search for: `quit("No such contaminant interaction option (%d),`

```c
 1597:                 break;
 1598:             default:
 1599→                 quit("No such contaminant interaction option (%d), must be no interaciton (0), additive (1), multiplicative (2), most limiting (3)\n");
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
**Found at (current code):** `atContaminants.c` 1836
**Lines at review (Oct 2026):** 1795-1797

**Function:** `Age_Contaminants_Update()`

**Why it's thought to be a bug**

`cid_prev = FunctGroupArray[sp].contaminantTracers[cohort-1][cIndex];` — the source cohort for aging is `cohort - numGeneTypes` (nextcid = cohort + sp_numGeneTypes, atdemography.c:2634); with genotypes, `cohort-1` is a different genotype of the same/younger age. Function: `Age_Contaminants_Update()`.

**Code at issue** — `atContaminants.c` line 1836  
Search for: `cid_prev = FunctGroupArray[sp].contaminantTracers[cohort-1][cIndex]`

```c
 1834:     for (cIndex = 0; cIndex < bm->num_contaminants; cIndex++) {
 1835:         cid = FunctGroupArray[sp].contaminantTracers[cohort][cIndex];
 1836→         cid_prev = FunctGroupArray[sp].contaminantTracers[cohort-1][cIndex];
 1837:         bm->boxes[ij].tr[k][cid] += FunctGroupArray[sp].agingContam[cohort][cIndex][ij][k];
 1838:         bm->boxes[ij].tr[k][cid_prev] -= FunctGroupArray[sp].agingContam[cohort][cIndex][ij][k];
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
**Found at (current code):** `atContaminants.c` 456
**Lines at review (Oct 2026):** 1834-1847

**Function:** `ContaminantMigrationIn()`

**Why it's thought to be a bug**

`time_step = bm->dayt - Leave_Now[mid]` is in days (Leave_Now set in days, atmigration.c:386), but `half_life` is in seconds (Degrade_Contaminants line 456 uses `dtsz / half_life` with dtsz in seconds). Decay during migration is underestimated by a factor 86400. Function: `ContaminantMigrationIn()`.

> 🔀 **Changed by the merge of main (Nov 2026):** ContaminantMigrationIn ordering/time_step (M-01). Check whether this finding is now fixed before acting.

**Code at issue** — `atContaminants.c` line 456 (closest match to the quoted code; wording differs slightly)  
Search for: `newValue = cLevel * pow(0.5, dtsz / bm->contaminantStructure[cIndex]->half_life);`

```c
  454: 	double newValue;
  455: 
  456→ 	newValue = cLevel * pow(0.5, dtsz / bm->contaminantStructure[cIndex]->half_life);
  457:     bm->contaminantStructure[cIndex]->amount_decayed = (cLevel - newValue) / dtsz;
  458: 
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
**Found at (current code):** `atContaminants.c` 1914, `atContaminants.c` 1919
**Lines at review (Oct 2026):** 1873-1880

**Function:** `ContaminantMigrationOut()`

**Why it's thought to be a bug**

`newContam = 0.0` (assignment commented out `//newContam = ;`) so every batch of leavers dilutes the migrant pool's contaminant level towards zero: `(origContam*oldden + 0*num_leaving)/(...)`. Leavers' tissue burden is lost. Function: `ContaminantMigrationOut()`.

**Code at issue** — `atContaminants.c` line 1914  
Search for: `newContam = 0.0`

```c
 1912: void ContaminantMigrationOut(MSEBoxModel *bm, int sp, int cohort, int mid, double oldden, double num_leaving) {
 1913:     int cIndex;
 1914→     double newContam = 0.0, origContam;
 1915:     
 1916:     // For species without live birth and maternal care
```

**Code at issue** — `atContaminants.c` line 1919  
Search for: `//newContam =`

```c
 1917:     for (cIndex = 0; cIndex < bm->num_contaminants; cIndex++) {
 1918:         origContam = MIGRATION[sp].contam[cohort][mid][cIndex];
 1919→         //newContam = ;
 1920:         if(oldden > 0) {
 1921:             MIGRATION[sp].contam[cohort][mid][cIndex] = (origContam * oldden + newContam * num_leaving) / (num_leaving + oldden + small_num);
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
**Found at (current code):** `atContaminants.c` 1950
**Lines at review (Oct 2026):** 1909-1910, 1924

**Why it's thought to be a bug**

`reprodContam += cGroupLevel * maternal_transfer_rate` (unweighted concentration sum over cells) while `reprodContamCount += this_den` (numbers); `Store_Recruit_Contaminants()` then divides the two — not a density-weighted mean, and divides by zero when no spawners contributed (NaN). Functions: `Get_Parental_Contaminants()`, `Store_Recruit_Contaminants()`.

**Code at issue** — `atContaminants.c` line 1950 (closest match to the quoted code; wording differs slightly)  
Search for: `FunctGroupArray[sp].reprodContam[cIndex] += (cGroupLevel * maternal_transfer_rate);`

```c
 1948:            cid = FunctGroupArray[sp].contaminantTracers[cohort][cIndex];
 1949:            cGroupLevel = bm->boxes[bm->current_box].tr[bm->current_layer][cid];
 1950→            FunctGroupArray[sp].reprodContam[cIndex] += (cGroupLevel * maternal_transfer_rate);
 1951:            FunctGroupArray[sp].reprodContamCount[cIndex] += this_den;
 1952:            bm->boxes[bm->current_box].tr[bm->current_layer][cid] -= (cGroupLevel * maternal_transfer_rate);
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
**Found at (current code):** `atContaminants.c` 2034
**Lines at review (Oct 2026):** 1993-1995

**Function:** `Get_Settler_Contaminants()`

**Why it's thought to be a bug**

`drandom(age_mat, numCohortsXnumGenes)` mixes age-class units (age_mat) with cohort×genotype count and ignores `ngene`, so the chosen "mother" cohort may be the wrong genotype/age (also overruns contaminantTracers per CONT-01). Function: `Get_Settler_Contaminants()`.

> 🔀 **Changed by the merge of main (Nov 2026):** Get_Settler_Contaminants segfault fix. Check whether this finding is now fixed before acting.

**Code at issue** — `atContaminants.c` line 2034 (the line the review pointed to, found by its text)  
Search for: `double start_n = FunctGroupArray[sp].speciesParams[age_mat_id];`

```c
 2032:     int flagmother = (int) (FunctGroupArray[sp].speciesParams[flagmother_id]);
 2033:     double this_reprodContam, maternal_transfer_rate, cGroupLevel;
 2034→     double start_n = FunctGroupArray[sp].speciesParams[age_mat_id];
 2035:     double end_n = (double)FunctGroupArray[sp].numCohortsXnumGenes;
 2036:     int cohort = (int)(floor(drandom(start_n, end_n))); // Find a random adult cohort to use
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
**Found at (current code):** `atContaminants.c` 2230
**Lines at review (Oct 2026):** 2177, 2250, 2254

**Function:** `Check_Contam_Totals()`

**Why it's thought to be a bug**

`bm->forceTracerInput[cIndex].total_input` indexes the forcing-tracer array by contaminant index, but `Init_Contaminants()` (204-208) shows contaminants are matched to forcing entries by name, not position. If any non-contaminant forcing tracer precedes them, the wrong total is checked (spurious quit or missed check). Function: `Check_Contam_Totals()`.

**Code at issue** — `atContaminants.c` line 2230  
Search for: `forceTracerInput[cIndex].total_input`

```c
 2228:         **/
 2229:         
 2230→         if(!bm->forceTracerInput[cIndex].total_input) {
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
**Found at (current code):** `atContaminants.c` 860, `atContaminants.c` 163
**Lines at review (Oct 2026):** 840, 1266

**Why it's thought to be a bug**

`expose_time[sp][cohort][current_box][current_layer]` is sized `wcnz+sednz` layers but in SED habitat `current_layer` is a sediment layer index (0..), colliding with WC layer slots. Same for `sp_point`/`sp_maxDoseToDate`. Functions: `Species_Contaminant_Uptake()`, `Get_ContamMortEffects()`.

> 🔀 **Changed by the merge of main (Nov 2026):** the code for this finding was changed by the merge. Check whether this finding is now fixed before acting.

**Code at issue** — `atContaminants.c` line 860  
Search for: `expose_time[sp][cohort][bm->current_box][bm->current_layer]`

```c
  858:                     // Initiate exposure time
  859:                     if (!cGroupLevel && cUptake) {
  860→                         bm->contaminantStructure[cIndex]->expose_time[sp][cohort][bm->current_box][bm->current_layer] = bm->dayt;
  861:                     }
  862:                         
```

**Code at issue** — `atContaminants.c` line 163  
Search for: `wcnz+bm->sednz`

```c
  161: 
  162:         bm->contaminantStructure[cIndex]->sp_instantDoseMortality = Util_Alloc_Init_1D_Double(bm->K_num_tot_sp, 0);
  163→ 		bm->contaminantStructure[cIndex]->sp_maxDoseToDate = Util_Alloc_Init_4D_Double((bm->wcnz+bm->sednz), bm->nbox, max_chrt, bm->K_num_tot_sp, 0.0);
  164: 		bm->contaminantStructure[cIndex]->sp_MoveEffect = Util_Alloc_Init_1D_Double(bm->K_num_tot_sp, 0.0);
  165:         bm->contaminantStructure[cIndex]->sp_ReprodEffect = Util_Alloc_Init_1D_Double(bm->K_num_tot_sp, 0.0);
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
**Found at (current code):** `atContaminants.c` 1438
**Lines at review (Oct 2026):** 1397

**Why it's thought to be a bug**

`if ((isPrimaryProducer == TRUE) && (isMicroFauna == TRUE))` — a group is unlikely to be both, so the Laender EC50 branch is effectively dead; `

**Code at issue** — `atContaminants.c` line 1438 (closest match to the quoted code; wording differs slightly)  
Search for: `if ((FunctGroupArray[species].isPrimaryProducer == TRUE) && (FunctGroupArray[species].isMi`

```c
 1436:         case InVitro_model:  // Growth effects as of InVitro
 1437:             if (cGroupLevel > bm->contaminantStructure[cIndex]->sp_GrowthThresh[species]) {
 1438→                 if ((FunctGroupArray[species].isPrimaryProducer == TRUE) && (FunctGroupArray[species].isMicroFauna == TRUE)){
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
