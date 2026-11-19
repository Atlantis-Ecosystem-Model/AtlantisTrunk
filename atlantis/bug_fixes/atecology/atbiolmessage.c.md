# atecology/atbiolmessage.c — bug review

Generated 2026-11-19 from `atlantis/to-do-fixt-list.md`. **11 findings** (severity H: 0; confidence H and severity H: 0; on the priority list: 0), plus 2 lower-confidence notes.

Each finding's **Code at issue** is located by searching the current code for the code quoted in the review, not by line number. Use the **Search for** text with your editor's Find, because line numbers will keep changing as fixes go in. → marks the matching line. If only a close match or a name could be found, the block says so. ⚠ means the quoted code is no longer in the file: it may have been changed or already fixed. **Lines at review** are the October numbers. Line numbers in the description text are approximate.

Conf/Sev: confidence it's a bug / severity if it is (H = wrong results or crash, M = wrong in some configs, L = cosmetic, log, leak or latent). ✔ = the reviewer re-checked it by hand. ⭐ = on the high-priority list.

## Summary

| ID | Found at | Conf/Sev | Category | Short description | Decision | Done |
|---|---|---|---|---|---|---|
| [BMSG-01](#bmsg-01) | 966, 990, 994 | H/M | accumulator not reset | `coverindx = 0;` is set once before the box loop (966), but inside the loop `coverindx += epi[CoverTracer]` ke… |  | [ ] |
| [BMSG-02](#bmsg-02) | 2857, 2757 | H/M | double counting | For vertebrates, the first `n` loop adds `MIGRATION[sp].DEN/SN/RN[n][i]` into `juv_mig_sum`/`ad_mig_sum`. Then… |  | [ ] |
| [BMSG-03](#bmsg-03) | 910 | M/M | header/data mismatch | For invertebrate migrators the header writes 3 columns for each `n` and for each `i < num_migrate`. The writer… |  | [ ] |
| [BMSG-04](#bmsg-04) | 530, 435 | M/M | wrong scope | The ice-layer block (`if(bm->ice_on)`) sits inside `if (bm->boxes[ij].botz >= bm->maxwcbotz)`, which is meant … |  | [ ] |
| [BMSG-05](#bmsg-05) | 360 | M/M | missing update | `bm->boxBiomass[ij][sp]` is incremented for water-column groups (281, 360) and land (601), but not in the sedi… |  | [ ] |
| [BMSG-06](#bmsg-06) | 2735 | M/M | cohort vs cohort x genotype | `n` runs over `numCohortsXnumGenes` but is compared directly with `age_mat` (an age-class index). Elsewhere th… |  | [ ] |
| [BMSG-07](#bmsg-07) | 311 | M/M | cohort vs cohort x genotype | `fished_chrt` comes from `Age95pcntV_id` (a cohort index), and its floor is set from `numCohortsXnumGenes/2`. … |  | [ ] |
| [BMSG-08](#bmsg-08) | 1077 | M/L | wrong condition | `if ((flagstockout == 1) \|\| call_type)` also holds for diagnostic box calls (`call_type == 1`, from atecology.… |  | [ ] |
| [BMSG-09](#bmsg-09) | 1026, 1034 | M/L | out-of-bounds read | The BSS slope is computed only when `smallID != -1 && largeID != -1`, but the debug fprintf right after it use… |  | [ ] |
| [BMSG-10](#bmsg-10) | 694, 726 | M/L | cohort vs cohort x genotype | The initial-condition checks loop `n < numCohorts`, while the tracer arrays (`structNTracers`, `totNTracers`) … |  | [ ] |
| [BMSG-11](#bmsg-11) | 414 | L/L | wrong variable | `bm->diagnosticBiom[sp] += ...NO3...` uses the loop variable `sp`, left over after the species loop, while the… |  | [ ] |

---

## BMSG-01

**accumulator not reset** · Conf H / Sev M  
**Found at (current code):** `atbiolmessage.c` 966, `atbiolmessage.c` 990, `atbiolmessage.c` 994
**Lines at review (Oct 2026):** 966, 987-994

**Function:** `Ecology_Calculate_Total_Abundance()`

**Why it's thought to be a bug**

`coverindx = 0;` is set once before the box loop (966), but inside the loop `coverindx += epi[CoverTracer]` keeps adding and then `ecolindx[coverindx_id] += coverindx / (nbox-nboundary-nland)` runs for every box. So box 1's cover gets counted once per box, box 2's once per remaining box, and so on. The cover indicator is a weighted cumulative sum, not a mean. Function: `Ecology_Calculate_Total_Abundance()`.

**Code at issue** — `atbiolmessage.c` line 966  
Search for: `coverindx = 0`

```c
  964: 		bm->ecolindx[InfEpi_id] = 0;
  965: 		bm->ecolindx[coverindx_id] = 0;
  966→ 		coverindx = 0;
  967: 
  968: 		for (ij = 0; ij < ncells; ij++) {
```

**Code at issue** — `atbiolmessage.c` line 990 (closest match to the quoted code; wording differs slightly)  
Search for: `coverindx += bm->boxes[ij].epi[FunctGroupArray[k].CoverTracer];`

```c
  988: 					if(FunctGroupArray[k].speciesParams[flag_id] == TRUE){
  989: 						if (FunctGroupArray[k].CoverTracer != -1) {
  990→ 							coverindx += bm->boxes[ij].epi[FunctGroupArray[k].CoverTracer];
  991: 						}
  992: 					}
```

**Code at issue** — `atbiolmessage.c` line 994  
Search for: `ecolindx[coverindx_id] += coverindx / (bm->nbox - bm->nboundary - bm->nland)`

```c
  992: 					}
  993: 				}
  994→ 				bm->ecolindx[coverindx_id] += coverindx / (bm->nbox - bm->nboundary - bm->nland);
  995: 
  996: 				/*
```

**Suggested fix (from review):** Move `coverindx = 0;` inside the `for (ij...)` loop, before the `k` loop (or add straight into `ecolindx`).

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

## BMSG-02

**double counting** · Conf H / Sev M  
**Found at (current code):** `atbiolmessage.c` 2857, `atbiolmessage.c` 2757
**Lines at review (Oct 2026):** 2736-2778

**Function:** `Write_Migration_File()`

**Why it's thought to be a bug**

For vertebrates, the first `n` loop adds `MIGRATION[sp].DEN/SN/RN[n][i]` into `juv_mig_sum`/`ad_mig_sum`. Then the `j` loop uses `index = j*num_migrate + i`, which at `j=0` is `i` again, so queue slot `i` is added twice. Migrating biomass and Prop_Migrating in Migration.txt are too high. Function: `Write_Migration_File()`.

**Code at issue** — `atbiolmessage.c` line 2857 (closest match to the quoted code; wording differs slightly)  
Search for: `fprintf(fid,"%e %s %d %d %e %e %e %d %d %d\n", bm->dayt, FunctGroupArray[sp].groupCode, n,`

```c
 2855:                         num_rec = MIGRATION[sp].recruit[n][i];
 2856:                     }
 2857→                     fprintf(fid,"%e %s %d %d %e %e %e %d %d %d\n", bm->dayt, FunctGroupArray[sp].groupCode, n, i, (MIGRATION[sp].DEN[n][i] + num_rec), MIGRATION[sp].SN[n][i], MIGRATION[sp].RN[n][i], MIGRATION[sp].Leave_Now[i], MIGRATION[sp].Return_Now[i], MIGRATION[sp].cohort_migrating[n][i]);
 2858:                 }
 2859:             }
```

**Code at issue** — `atbiolmessage.c` line 2757 (closest match to the quoted code; wording differs slightly)  
Search for: `index = (j * FunctGroupArray[sp].num_migrate) + i;`

```c
 2755:                     for (j = 0; j < numValues; j++){
 2756:                         
 2757→                         index = (j * FunctGroupArray[sp].num_migrate) + i;
 2758:                         for(n = 0; n < FunctGroupArray[sp].numCohortsXnumGenes; n++){
 2759:                             
```

**Suggested fix (from review):** Delete the migrating-sum lines from the first loop (keep only `juv_sum`/`ad_sum` there), or start `j` at 1.

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

## BMSG-03

**header/data mismatch** · Conf M / Sev M  
**Found at (current code):** `atbiolmessage.c` 910
**Lines at review (Oct 2026):** 2697-2703 vs 2790-2808

**Function:** `Init_Migration_File()`

**Why it's thought to be a bug**

For invertebrate migrators the header writes 3 columns for each `n` and for each `i < num_migrate`. The writer writes 3 values for each `n` and each `i` in `[num_in_queue_done, num_in_queue)`, which is a different, time-varying count. Migration.txt columns are misaligned and the row width changes from row to row. Function: `Init_Migration_File()` / `Write_Migration_File()`.

**Code at issue** — `atbiolmessage.c` line 910 (closest match to the quoted code; wording differs slightly)  
Search for: `for (ij = MIGRATION[sp].num_in_queue_done; ij < MIGRATION[sp].num_in_queue; ij++) {`

```c
  908: 			/* Vertebrates */
  909: 			for (sp = 0; sp < bm->K_num_tot_sp; sp++) {
  910→                 for (ij = MIGRATION[sp].num_in_queue_done; ij < MIGRATION[sp].num_in_queue; ij++) {
  911:                     for (cohort = 0; cohort < FunctGroupArray[sp].numCohortsXnumGenes; cohort++) {
  912:                         if (FunctGroupArray[sp].isVertebrate == TRUE) {
```

**Suggested fix (from review):** Use the same loop bound in both, e.g. aggregate over the queue per `i < num_migrate` as the vertebrate branch does.

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

## BMSG-04

**wrong scope** · Conf M / Sev M  
**Found at (current code):** `atbiolmessage.c` 530, `atbiolmessage.c` 435
**Lines at review (Oct 2026):** 435, 530-574

**Function:** `Calculate_Box_Biomass()`

**Why it's thought to be a bug**

The ice-layer block (`if(bm->ice_on)`) sits inside `if (bm->boxes[ij].botz >= bm->maxwcbotz)`, which is meant to leave out the non-dynamic sediments and epibenthos of oceanic cells. Sea ice over deep oceanic boxes is dynamic, but it is left out of totbiom, reg_prop and targetspbiom. Function: `Calculate_Box_Biomass()`.

**Code at issue** — `atbiolmessage.c` line 530  
Search for: `if(bm->ice_on)`

```c
  528: 
  529: 
  530→ 		if(bm->ice_on){
  531: 
  532: 			/* Ice layers  */
```

**Code at issue** — `atbiolmessage.c` line 435  
Search for: `if (bm->boxes[ij].botz >= bm->maxwcbotz)`

```c
  433: 	/* Only include benthos and epibenthics if not oceanic cell - oceanic cells should not be counted as their sediments are not dynamic */
  434: 	//if (isInitPops == TRUE || bm->boxes[ij].botz >= bm->maxwcbotz) {
  435→ 	if (bm->boxes[ij].botz >= bm->maxwcbotz) {
  436: 		for (k = 0; k < bm->boxes[ij].sm.nz; k++) { // Loop through each sediment layer
  437: 			for (sp = 0; sp < bm->K_num_tot_sp; sp++) {
```

**Suggested fix (from review):** Move the ice block outside the `botz >= maxwcbotz` if-block.

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

## BMSG-05

**missing update** · Conf M / Sev M  
**Found at (current code):** `atbiolmessage.c` 360
**Lines at review (Oct 2026):** 457-458, 504-505, 552-553

**Function:** `Calculate_Box_Biomass()`

**Why it's thought to be a bug**

`bm->boxBiomass[ij][sp]` is incremented for water-column groups (281, 360) and land (601), but not in the sediment, epibenthic or ice branches. BoxBiomass.txt (`Write_BoxBiomass`) therefore leaves out all infauna, epibenthos and ice biomass and only shows any WC fraction. Function: `Calculate_Box_Biomass()`.

**Code at issue** — `atbiolmessage.c` line 360  
Search for: `boxBiomass[ij][sp]`

```c
  358: 							bm->totbiom[sp] += biomass;
  359: 							bm->totagepop[sp][n] += biomass;
  360→ 							bm->boxBiomass[ij][sp] += biomass;
  361: 
  362: 							/**
```

**Suggested fix (from review):** Add `bm->boxBiomass[ij][sp] += biomass;` in the sed, epi and ice branches (non-diagnostic path).

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

## BMSG-06

**cohort vs cohort x genotype** · Conf M / Sev M  
**Found at (current code):** `atbiolmessage.c` 2735
**Lines at review (Oct 2026):** 2735, 2743, 2763

**Function:** `Write_Migration_File()`

**Why it's thought to be a bug**

`n` runs over `numCohortsXnumGenes` but is compared directly with `age_mat` (an age-class index). Elsewhere the convention is `basecohort = n / numGeneTypes` (atdemography.c:373, 500). With `numGeneTypes > 1` the juvenile/adult split is wrong. Function: `Write_Migration_File()`.

**Code at issue** — `atbiolmessage.c` line 2735 (the line the review pointed to, found by its text)  
Search for: `age_mat = (int) (FunctGroupArray[sp].speciesParams[age_mat_id]);`

```c
 2733:            	if(FunctGroupArray[sp].isVertebrate == TRUE){
 2734: 
 2735→            		age_mat = (int) (FunctGroupArray[sp].speciesParams[age_mat_id]);
 2736:            		for(i = 0; i < FunctGroupArray[sp].num_migrate; i++){
 2737:            			juv_mig_sum = juv_sum = ad_mig_sum = ad_sum = 0;
```

**Suggested fix (from review):** Compare `(n / FunctGroupArray[sp].numGeneTypes) < age_mat`.

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

## BMSG-07

**cohort vs cohort x genotype** · Conf M / Sev M  
**Found at (current code):** `atbiolmessage.c` 311
**Lines at review (Oct 2026):** 266-268, 310

**Function:** `Calculate_Box_Biomass()`

**Why it's thought to be a bug**

`fished_chrt` comes from `Age95pcntV_id` (a cohort index), and its floor is set from `numCohortsXnumGenes/2`. It is then compared with `n`, the cohort x genotype index. With genotypes, `calcNstart[sp][hist_id]` counts the wrong (single genotype) slot. Function: `Calculate_Box_Biomass()`.

**Code at issue** — `atbiolmessage.c` line 311  
Search for: `calcNstart[sp][hist_id]`

```c
  309: 								totden[sp][n] += bm->boxes[ij].tr[k][den];
  310: 								if (n == fished_chrt) {
  311→ 									bm->calcNstart[sp][hist_id] += bm->boxes[ij].tr[k][den];
  312: 								}
  313: 								bm->calcNstartPerPred[sp][hist_id] += bm->boxes[ij].tr[k][den];
```

**Suggested fix (from review):** Use `(n / numGeneTypes) == fished_chrt`, with the floor based on `numCohorts/2`.

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

## BMSG-08

**wrong condition** · Conf M / Sev L  
**Found at (current code):** `atbiolmessage.c` 1077
**Lines at review (Oct 2026):** 1077-1095

**Function:** `Ecology_Calculate_Total_Abundance()`

**Why it's thought to be a bug**

`if ((flagstockout == 1) || call_type)` also holds for diagnostic box calls (`call_type == 1`, from atecology.c:1253, 1959, 3020, 3616). So `Ecology_Output_Mort_Per_Pred_Estimates()` writes extra MortPerPred/SpecificPredMort rows for every debugged box/layer call. Function: `Ecology_Calculate_Total_Abundance()`.

**Code at issue** — `atbiolmessage.c` line 1077  
Search for: `if ((flagstockout == 1) || call_type)`

```c
 1075: 
 1076: 		/* If non-annual call proceed with standard log.txt output */
 1077→ 		if ((flagstockout == 1) || call_type) {
 1078: 
 1079: 
```

**Suggested fix (from review):** Wrap the Mort_Per_Pred call in `if (!call_type && !bm->flagannual_Mest)`.

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

## BMSG-09

**out-of-bounds read** · Conf M / Sev L  
**Found at (current code):** `atbiolmessage.c` 1026, `atbiolmessage.c` 1034
**Lines at review (Oct 2026):** 1026-1036

**Function:** `Ecology_Calculate_Total_Abundance()`

**Why it's thought to be a bug**

The BSS slope is computed only when `smallID != -1 && largeID != -1`, but the debug fprintf right after it uses `FunctGroupArray[bm->smallID].groupCode` and `totfishpop[bm->smallID]` without that guard, so it reads index -1. Function: `Ecology_Calculate_Total_Abundance()`.

**Code at issue** — `atbiolmessage.c` line 1026  
Search for: `smallID != -1 && bm->largeID != -1`

```c
 1024: 				}
 1025: 			}
 1026→ 			if(bm->smallID != -1 && bm->largeID != -1){
 1027: 				/* Find BSS - numerator and denominator to be all in kg in calculation */
 1028: 				bm->ecolindx[BSSslope_id] = ((bm->totfishpop[bm->smallID] - bm->totfishpop[bm->largeID]) * bm->X_CN * mg_2_tonne) / (log(bm->smallsize) - log(
```

**Code at issue** — `atbiolmessage.c` line 1034  
Search for: `FunctGroupArray[bm->smallID].groupCode`

```c
 1032: 				fprintf(llogfp,
 1033: 						"BSSslope: %e, fishpop[small-%s]: %e, fishpop[large-%s]: %e, log(smallsize): %e, smallsize: %e, log(largesize): %e, largesize: %e\n",
 1034→ 						bm->ecolindx[BSSslope_id], FunctGroupArray[bm->smallID].groupCode, bm->totfishpop[bm->smallID], FunctGroupArray[bm->largeID].groupCode,
 1035: 						bm->totfishpop[bm->largeID], (double)log(bm->smallsize), bm->smallsize, (double)log(bm->largesize), bm->largesize);
 1036: 			}
```

**Suggested fix (from review):** Move the debug print inside the `smallID/largeID != -1` block.

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

## BMSG-10

**cohort vs cohort x genotype** · Conf M / Sev L  
**Found at (current code):** `atbiolmessage.c` 694, `atbiolmessage.c` 726
**Lines at review (Oct 2026):** 694, 726

**Function:** `Check_Layer_Initial_Biomass()`

**Why it's thought to be a bug**

The initial-condition checks loop `n < numCohorts`, while the tracer arrays (`structNTracers`, `totNTracers`) run over `numCohortsXnumGenes`. Genotypes beyond the first are never checked for negative or misplaced biomass. Function: `Check_Layer_Initial_Biomass()`.

**Code at issue** — `atbiolmessage.c` line 694 (the line the review pointed to, found by its text)  
Search for: `for (n = 0; n < FunctGroupArray[sp].numCohorts; n++) {`

```c
  692: 			case AGE_STRUCTURED:
  693: 
  694→ 				for (n = 0; n < FunctGroupArray[sp].numCohorts; n++) {
  695: 					sn = FunctGroupArray[sp].structNTracers[n];
  696: 					rn = FunctGroupArray[sp].resNTracers[n];
```

**Code at issue** — `atbiolmessage.c` line 726 (the line the review pointed to, found by its text)  
Search for: `for (n = 0; n < FunctGroupArray[sp].numCohorts; n++) {`

```c
  724: 			case AGE_STRUCTURED_BIOMASS:
  725: 				if (FunctGroupArray[sp].groupType != REF_DET && FunctGroupArray[sp].groupType != LAB_DET) {
  726→ 					for (n = 0; n < FunctGroupArray[sp].numCohorts; n++) {
  727: 
  728: 						pid = FunctGroupArray[sp].totNTracers[n];
```

**Suggested fix (from review):** Loop to `numCohortsXnumGenes`.

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

## BMSG-11

**wrong variable** · Conf L / Sev L  
**Found at (current code):** `atbiolmessage.c` 414
**Lines at review (Oct 2026):** 411-414

**Function:** `Calculate_Box_Biomass()`

**Why it's thought to be a bug**

`bm->diagnosticBiom[sp] += ...NO3...` uses the loop variable `sp`, left over after the species loop, while the NH3 block (395) uses `nid` (`DIN_id`). It works only because `sp == K_num_tot_sp == DIN_id` (atparamIO.c:367). Function: `Calculate_Box_Biomass()`.

**Code at issue** — `atbiolmessage.c` line 414  
Search for: `diagnosticBiom[sp] +=`

```c
  412: 		nid = bm->DIN_id;
  413: 		if(isDiagnostic == TRUE){
  414→ 			bm->diagnosticBiom[sp] += bm->boxes[ij].tr[k][pid] * bm->boxes[ij].dz[k] * bm->boxes[ij].area; // Intentional overload storing all nutrients in final slot
  415: 		}else{
  416: 			bm->totbiom[nid] += bm->boxes[ij].tr[k][pid] * bm->boxes[ij].dz[k] * bm->boxes[ij].area;
```

**Suggested fix (from review):** Use `bm->diagnosticBiom[nid]`.

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

atbiolmessage.c 1802/1825: `mort_scale = 365.0 / (365.0 - update_date)` divides by zero if `calcupdate_date` is 365 (Write_MortPerPred guards this case; Write_Mort does not).

- [ ] Investigate   - [ ] Ignore

**Your comments:**

> 

### LC-2

atbiolmessage.c 742-748, 793-799: the "biomass in inactive layer" warnings for biomass-pool groups multiply by `dz[k]`/`sm.dz[k]`, which are likely 0 for inactive layers, so these warnings may never fire.

- [ ] Investigate   - [ ] Ignore

**Your comments:**

> 
