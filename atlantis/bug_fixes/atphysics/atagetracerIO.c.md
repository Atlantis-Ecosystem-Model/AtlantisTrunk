# atphysics/atagetracerIO.c — bug review

Generated 2026-11-19 from `atlantis/to-do-fixt-list.md`. **4 findings** (severity H: 1; confidence H and severity H: 1; on the priority list: 1).

Each finding's **Code at issue** is located by searching the current code for the code quoted in the review, not by line number. Use the **Search for** text with your editor's Find, because line numbers will keep changing as fixes go in. → marks the matching line. If only a close match or a name could be found, the block says so. ⚠ means the quoted code is no longer in the file: it may have been changed or already fixed. **Lines at review** are the October numbers. Line numbers in the description text are approximate.

Conf/Sev: confidence it's a bug / severity if it is (H = wrong results or crash, M = wrong in some configs, L = cosmetic, log, leak or latent). ✔ = the reviewer re-checked it by hand. ⭐ = on the high-priority list.

## Summary

| ID | Found at | Conf/Sev | Category | Short description | Decision | Done |
|---|---|---|---|---|---|---|
| [PHY2-12](#phy2-12) ⭐ | 520, 504, 411 | H/H | loop nesting / stale buffer | `ncvarput(fid, vid, start, count, val)` (whole-nbox record) is called inside `for b` → `for nf`, after only `v… |  | [ ] |
| [PHY2-13](#phy2-13) | 109 | H/L | loop bound mismatch | The Info routines define `numCohortsXnumGenes * numStocks * numAges` variables, but the Data routines loop onl… |  | [ ] |
| [PHY2-14](#phy2-14) | 229 | M/M | wrong weighting | Weight at age is `sum_k N*w*boxPopRatio / sum_k N`. The numerator is weighted by this age class's population f… |  | [ ] |
| [PHY2-15](#phy2-15) | 335, 483 | M/L | inconsistent check | In `writeBMAnnAgeCatData` the species filter lacks the `isFished \|\| isImpacted` test used in `writeBMAnnAgeCat… |  | [ ] |

---

## PHY2-12

**loop nesting / stale buffer** · Conf H / Sev H ⭐ priority  
**Found at (current code):** `atagetracerIO.c` 520, `atagetracerIO.c` 504, `atagetracerIO.c` 411
**Lines at review (Oct 2026):** 465-522

**Function:** `writeBMAnnAgeCatData()`

**Why it's thought to be a bug**

`ncvarput(fid, vid, start, count, val)` (whole-nbox record) is called inside `for b` → `for nf`, after only `val[b]` has been set. The last write for each fishery happens at b = nbox-1. At that point `val[0..nbox-2]` hold the values from the last flagged fishery computed for those boxes, not this fishery's. So every fishery except the last flagged one is written with another fishery's per-box catch/discards in all boxes except the last. Function: `writeBMAnnAgeCatData()`.

**Code at issue** — `atagetracerIO.c` line 520  
Search for: `ncvarput(fid, vid, start, count, val)`

```c
  518:                                     
  519:                                         /* Write data */
  520→                                         ncvarput(fid, vid, start, count, val);
  521:                                     }
  522:                                 }
```

**Code at issue** — `atagetracerIO.c` line 504  
Search for: `val[b]`

```c
  502:                                             printf("writeBMAnnAgeCatData: writing %s\n", strname);
  503:                                     
  504→                                         val[b] = (doubleINPUT)(avgpopratio[b] * bm->fishstat[b][id] * (rel_access * bm->fishstat[b][tid] / tot));
  505:                                         
  506:                                         /** dummy = val[b] * (avgsze[b] * bm->X_CN * mg_2_tonne);
```

**Code at issue** — `atagetracerIO.c` line 411 (closest match to the quoted code; wording differs slightly)  
Search for: `val = (doubleINPUT *) alloc1dInput(bm->nbox);`

```c
  409:     
  410:     /* Allocate temporary storage for one epi variable */
  411→     val = (doubleINPUT *) alloc1dInput(bm->nbox);
  412:     avgpopratio = (double *) alloc1dInput(bm->nbox);
  413:     avgsze = (double *) alloc1dInput(bm->nbox);
```

**Suggested fix (from review):** Restructure: for each propit/nf, loop b to fill `val`, then call ncvarput once after the box loop

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

## PHY2-13

**loop bound mismatch** · Conf H / Sev L  
**Found at (current code):** `atagetracerIO.c` 109
**Lines at review (Oct 2026):** 109 & 341 vs 193 & 433

**Function:** `writeBMAnnAgeBioInfo()`

**Why it's thought to be a bug**

The Info routines define `numCohortsXnumGenes * numStocks * numAges` variables, but the Data routines loop only `numCohortsXnumGenes * numAges`. When numStocks > 1 the extra variables are defined but never written (left as fill values), and the names suggest age classes that don't exist. Function: `writeBMAnnAgeBioInfo()`, `writeBMAnnAgeCatInfo()`.

**Code at issue** — `atagetracerIO.c` line 109 (closest match to the quoted code; wording differs slightly)  
Search for: `for (cohort = 0; cohort < FunctGroupArray[sp].numCohortsXnumGenes * FunctGroupArray[sp].nu`

```c
  107:                 if(numAges < 1)
  108:                     numAges = 1;
  109→                 for (cohort = 0; cohort < FunctGroupArray[sp].numCohortsXnumGenes * FunctGroupArray[sp].numStocks * numAges; cohort++) {
  110:                     for (propit = 0; propit < out_catch_id; propit++) {
  111:                         
```

**Suggested fix (from review):** Drop `* numStocks` from the Info loops (or add a stock loop to the Data routines)

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

## PHY2-14

**wrong weighting** · Conf M / Sev M  
**Found at (current code):** `atagetracerIO.c` 229
**Lines at review (Oct 2026):** 227-234

**Function:** `writeBMAnnAgeBioData()`

**Why it's thought to be a bug**

Weight at age is `sum_k N*w*boxPopRatio / sum_k N`. The numerator is weighted by this age class's population fraction but the denominator is not, so the reported individual weight is scaled down by that fraction and is not a true mean weight. Function: `writeBMAnnAgeBioData()`.

**Code at issue** — `atagetracerIO.c` line 229 (the line the review pointed to, found by its text)  
Search for: `val[b] += bm->wctr[b][k][id] * (bm->wctr[b][k][sn] + bm->wctr[b][k][rn]) * bm->X_CN * mg_2`

```c
  227:                                     for (k = 0; k < bm->wcnz; k++) {  // Sum over all individuals in the box
  228:                                         if (bm->wctr[b][k][id] > bm->min_dens) {
  229→                                             val[b] += bm->wctr[b][k][id] * (bm->wctr[b][k][sn] + bm->wctr[b][k][rn]) * bm->X_CN * mg_2_g * FunctGroupArray[sp].boxPopRatio[b][k][cohort][ai];
  230:                                             tot += bm->wctr[b][k][id];
  231:                                         }
```

**Suggested fix (from review):** Use `tot += N * boxPopRatio[b][k][cohort][ai]` (or drop boxPopRatio from the numerator)

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

## PHY2-15

**inconsistent check** · Conf M / Sev L  
**Found at (current code):** `atagetracerIO.c` 335, `atagetracerIO.c` 483
**Lines at review (Oct 2026):** 476, 504

**Function:** `writeBMAnnAgeCatData()`

**Why it's thought to be a bug**

In `writeBMAnnAgeCatData` the species filter lacks the `isFished || isImpacted` test used in `writeBMAnnAgeCatInfo` (335). If `FisherySpeciesCatchFlags[sp][nf]` is TRUE for a species with neither flag set, `ncvarid` hits an undefined variable and is fatal under NC_FATAL. Function: `writeBMAnnAgeCatData()`.

**Code at issue** — `atagetracerIO.c` line 335 (closest match to the quoted code; wording differs slightly)  
Search for: `if (( (int)(FunctGroupArray[sp].speciesParams[flag_id])) && (FunctGroupArray[sp].isFished `

```c
  333:     for (sp = 0; sp < bm->K_num_tot_sp; sp++) {
  334:         if (FunctGroupArray[sp].groupAgeType == AGE_STRUCTURED) { // Used to be a test on FunctGroupArray[sp].isVertebrate
  335→             if (( (int)(FunctGroupArray[sp].speciesParams[flag_id])) && (FunctGroupArray[sp].isFished || FunctGroupArray[sp].isImpacted)){
  336:                 this_cohort = 0;
  337:                 numAges = FunctGroupArray[sp].ageClassSize;
```

**Code at issue** — `atagetracerIO.c` line 483  
Search for: `FisherySpeciesCatchFlags[sp][nf]`

```c
  481:                                 
  482:                                 for (nf = 0; nf < bm->K_num_fisheries; nf++) {
  483→                                     if (FisherySpeciesCatchFlags[sp][nf] == TRUE) {
  484:                                         rel_access = Harvest_How_Much_Fishery_Access(bm,sp,cohort,nf,avgsze[b]);
  485:                                         switch (propit) {
```

**Suggested fix (from review):** Apply the same species filter in both routines

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
