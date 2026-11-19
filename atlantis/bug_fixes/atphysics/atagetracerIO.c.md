# atphysics/atagetracerIO.c — bug review

Generated 2026-11-19 from `atlantis/to-do-fixt-list.md`. **4 findings** (severity H: 1; confidence H and severity H: 1; on the priority list: 1).

Line numbers were re-mapped on 2026-11-19 to the current code (after the merge of main). Each was traced through git history from the version the review used, then checked against the text of the line. **Lines (current code)** and line numbers in the text are now current; ≈ means the code there has changed so the position is approximate. The review's own numbers are kept as **Lines at review**; a number ending in ? could not be traced and is the review's number. ⚠ means the quoted code is no longer at those lines: it may have changed or already been fixed.

Conf/Sev: confidence it's a bug / severity if it is (H = wrong results or crash, M = wrong in some configs, L = cosmetic, log, leak or latent). ✔ = the reviewer re-checked it by hand. ⭐ = on the high-priority list.

## Summary

| ID | Lines (current) | Conf/Sev | Category | Short description | Decision | Done |
|---|---|---|---|---|---|---|
| [PHY2-12](#phy2-12) ⭐ | 465-522 | H/H | loop nesting / stale buffer | `ncvarput(fid, vid, start, count, val)` (whole-nbox record) is called inside `for b` → `for nf`, after only `v… |  | [ ] |
| [PHY2-13](#phy2-13) | 109 & 341 vs 193 & 433 | H/L | loop bound mismatch | The Info routines define `numCohortsXnumGenes * numStocks * numAges` variables, but the Data routines loop onl… |  | [ ] |
| [PHY2-14](#phy2-14) | 227-234 | M/M | wrong weighting | Weight at age is `sum_k N*w*boxPopRatio / sum_k N`. The numerator is weighted by this age class's population f… |  | [ ] |
| [PHY2-15](#phy2-15) | 476, 504 | M/L | inconsistent check | In `writeBMAnnAgeCatData` the species filter lacks the `isFished \|\| isImpacted` test used in `writeBMAnnAgeCat… |  | [ ] |

---

## PHY2-12

**loop nesting / stale buffer** · Conf H / Sev H ⭐ priority  
**Lines (current code):** 465-522  
**Lines at review (Oct 2026):** 465-522

**Function:** `writeBMAnnAgeCatData()`

**Why it's thought to be a bug**

`ncvarput(fid, vid, start, count, val)` (whole-nbox record) is called inside `for b` → `for nf`, after only `val[b]` has been set. The last write for each fishery happens at b = nbox-1. At that point `val[0..nbox-2]` hold the values from the last flagged fishery computed for those boxes, not this fishery's. So every fishery except the last flagged one is written with another fishery's per-box catch/discards in all boxes except the last. Function: `writeBMAnnAgeCatData()`.

**Current code** (`atphysics/atagetracerIO.c` lines 463–502 (truncated))

```c
  463:                         
  464:                         for (propit = out_catch_id; propit < (out_discards_id + 1); propit++) {
  465:                             for (b = 0; b < bm->nbox; b++) {
  466:                                 tot = 0.0;
  467:                                 for (nf = 0; nf < bm->K_num_fisheries; nf++) {
  468:                                     if (FisherySpeciesCatchFlags[sp][nf] == TRUE) {
  469:                                         rel_access = Harvest_How_Much_Fishery_Access(bm,sp,cohort,nf,avgsze[b]);
  470:                                     
  471:                                         if (propit == out_catch_id) {
  472:                                             tid = FunctGroupArray[sp].CaughtByFisheryTracers[nf];
  473:                                         } else {
  474:                                             tid = FunctGroupArray[sp].DiscardedByFisheryTracers[nf];
  475:                                         }
  476:                                         tot += rel_access * bm->fishstat[b][tid];
  477:                                     }
  478:                                 }
  479:                                 if (!tot)
  480:                                     tot = small_num;
  481:                                 
  482:                                 for (nf = 0; nf < bm->K_num_fisheries; nf++) {
  483:                                     if (FisherySpeciesCatchFlags[sp][nf] == TRUE) {
  484:                                         rel_access = Harvest_How_Much_Fishery_Access(bm,sp,cohort,nf,avgsze[b]);
  485:                                         switch (propit) {
  486:                                             case out_catch_id:
  487:                                                 snprintf(strname, sizeof(strname), "%s%d_Catch_%s", FunctGroupArray[sp].name, this_cohort+1,FisheryArray[nf].fisheryCode);
  488:                                                 tid = FunctGroupArray[sp].CaughtByFisheryTracers[nf];
  489:                                                 id = FunctGroupArray[sp].CatchTracers[cohort];  // This points to numbers caught at that age
  490:                                                 break;
  491:                                             case out_discards_id:
  492:                                                 snprintf(strname, sizeof(strname), "%s%d_Discard_%s", FunctGroupArray[sp].name, this_cohort+1,FisheryArray[nf].fisheryCode);
  493:                                                 tid = FunctGroupArray[sp].DiscardedByFisheryTracers[nf];
  494:                                                 id = FunctGroupArray[sp].DiscardTracers[cohort];  // This points to numbers discarded at that age
  495:                                                 break;
  496:                                             default:
  497:                                                 quit("writeBMAnnAgeCatInfo: How did you get this value as only 3 classes (propit: %d)\n", propit);
  498:                                                 break;
  499:                                         }
  500:                                     
  501:                                         if (verbose > 1)
  502:                                             printf("writeBMAnnAgeCatData: writing %s\n", strname);
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
**Lines (current code):** 109 & 341 vs 193 & 433  
**Lines at review (Oct 2026):** 109 & 341 vs 193 & 433

**Function:** `writeBMAnnAgeBioInfo()`

**Why it's thought to be a bug**

The Info routines define `numCohortsXnumGenes * numStocks * numAges` variables, but the Data routines loop only `numCohortsXnumGenes * numAges`. When numStocks > 1 the extra variables are defined but never written (left as fill values), and the names suggest age classes that don't exist. Function: `writeBMAnnAgeBioInfo()`, `writeBMAnnAgeCatInfo()`.

**Current code** (`atphysics/atagetracerIO.c` lines 107–111)

```c
  107:                 if(numAges < 1)
  108:                     numAges = 1;
  109:                 for (cohort = 0; cohort < FunctGroupArray[sp].numCohortsXnumGenes * FunctGroupArray[sp].numStocks * numAges; cohort++) {
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
**Lines (current code):** 227-234  
**Lines at review (Oct 2026):** 227-234

**Function:** `writeBMAnnAgeBioData()`

**Why it's thought to be a bug**

Weight at age is `sum_k N*w*boxPopRatio / sum_k N`. The numerator is weighted by this age class's population fraction but the denominator is not, so the reported individual weight is scaled down by that fraction and is not a true mean weight. Function: `writeBMAnnAgeBioData()`.

**Current code** (`atphysics/atagetracerIO.c` lines 225–236)

```c
  225:                                     val[b] = 0.0;
  226:                                     tot = 0.0;
  227:                                     for (k = 0; k < bm->wcnz; k++) {  // Sum over all individuals in the box
  228:                                         if (bm->wctr[b][k][id] > bm->min_dens) {
  229:                                             val[b] += bm->wctr[b][k][id] * (bm->wctr[b][k][sn] + bm->wctr[b][k][rn]) * bm->X_CN * mg_2_g * FunctGroupArray[sp].boxPopRatio[b][k][cohort][ai];
  230:                                             tot += bm->wctr[b][k][id];
  231:                                         }
  232:                                     }
  233:                                     if (tot > 0.0)
  234:                                         val[b] /= tot;
  235:                                 }
  236:                             }
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
**Lines (current code):** 476, 504  
**Lines at review (Oct 2026):** 476, 504

**Function:** `writeBMAnnAgeCatData()`

**Why it's thought to be a bug**

In `writeBMAnnAgeCatData` the species filter lacks the `isFished || isImpacted` test used in `writeBMAnnAgeCatInfo` (335). If `FisherySpeciesCatchFlags[sp][nf]` is TRUE for a species with neither flag set, `ncvarid` hits an undefined variable and is fatal under NC_FATAL. Function: `writeBMAnnAgeCatData()`.

**Current code** (`atphysics/atagetracerIO.c` lines 474–506)

```c
  474:                                             tid = FunctGroupArray[sp].DiscardedByFisheryTracers[nf];
  475:                                         }
  476:                                         tot += rel_access * bm->fishstat[b][tid];
  477:                                     }
  478:                                 }
  479:                                 if (!tot)
  480:                                     tot = small_num;
  481:                                 
  482:                                 for (nf = 0; nf < bm->K_num_fisheries; nf++) {
  483:                                     if (FisherySpeciesCatchFlags[sp][nf] == TRUE) {
  484:                                         rel_access = Harvest_How_Much_Fishery_Access(bm,sp,cohort,nf,avgsze[b]);
  485:                                         switch (propit) {
  486:                                             case out_catch_id:
  487:                                                 snprintf(strname, sizeof(strname), "%s%d_Catch_%s", FunctGroupArray[sp].name, this_cohort+1,FisheryArray[nf].fisheryCode);
  488:                                                 tid = FunctGroupArray[sp].CaughtByFisheryTracers[nf];
  489:                                                 id = FunctGroupArray[sp].CatchTracers[cohort];  // This points to numbers caught at that age
  490:                                                 break;
  491:                                             case out_discards_id:
  492:                                                 snprintf(strname, sizeof(strname), "%s%d_Discard_%s", FunctGroupArray[sp].name, this_cohort+1,FisheryArray[nf].fisheryCode);
  493:                                                 tid = FunctGroupArray[sp].DiscardedByFisheryTracers[nf];
  494:                                                 id = FunctGroupArray[sp].DiscardTracers[cohort];  // This points to numbers discarded at that age
  495:                                                 break;
  496:                                             default:
  497:                                                 quit("writeBMAnnAgeCatInfo: How did you get this value as only 3 classes (propit: %d)\n", propit);
  498:                                                 break;
  499:                                         }
  500:                                     
  501:                                         if (verbose > 1)
  502:                                             printf("writeBMAnnAgeCatData: writing %s\n", strname);
  503:                                     
  504:                                         val[b] = (doubleINPUT)(avgpopratio[b] * bm->fishstat[b][id] * (rel_access * bm->fishstat[b][tid] / tot));
  505:                                         
  506:                                         /** dummy = val[b] * (avgsze[b] * bm->X_CN * mg_2_tonne);
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
