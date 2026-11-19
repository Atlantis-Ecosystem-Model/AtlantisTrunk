# atecology/atmovement.c — bug review

Generated 2026-11-19 from `atlantis/to-do-fixt-list.md`. **30 findings** (severity H: 3; confidence H and severity H: 2; on the priority list: 2), plus 11 lower-confidence notes.

Each finding's **Code at issue** is located by searching the current code for the code quoted in the review, not by line number. Use the **Search for** text with your editor's Find, because line numbers will keep changing as fixes go in. → marks the matching line. If only a close match or a name could be found, the block says so. ⚠ means the quoted code is no longer in the file: it may have been changed or already fixed. **Lines at review** are the October numbers. Line numbers in the description text are approximate.

Conf/Sev: confidence it's a bug / severity if it is (H = wrong results or crash, M = wrong in some configs, L = cosmetic, log, leak or latent). ✔ = the reviewer re-checked it by hand. ⭐ = on the high-priority list.

## Summary

| ID | Found at | Conf/Sev | Category | Short description | Decision | Done |
|---|---|---|---|---|---|---|
| [MOVE-01](#move-01) ✔ ⭐ | 2030, 2043 | H/H | swapped indices | `newden[sp][n][chkbox][kk] += den_diff;` — newden is allocated `Util_Alloc_Init_4D_Double(ncells, nlayer, ncoh… |  | [ ] |
| [MOVE-02](#move-02) ⭐ | 2050 | H/H | wrong operator | `bm->boxes[ij].tr[k][DL_id] = VERTinfo[..SN_id] * totden * den_diff;` — this sets the labile/refractory detrit… |  | [ ] |
| [MOVE-03](#move-03) | 1559 | H/M | wrong variable | `(-1.0 * bm->boxes[bm->current_box].botz) <= ...[maxtotdepth_id]` — this is inside the `ij` box loop, but it u… |  | [ ] |
| [MOVE-04](#move-04) | 2998, 3074 | H/M | wrong type / truncation | `int qrt, next_qrt, this_HowFar, ...` — this_HowFar is an int but gets `bm->HowFar` (a double fraction in [0,1… |  | [ ] |
| [MOVE-05](#move-05) | 3386, 3343 | H/M | wrong index | `FunctGroupArray[sp].distrib[ij][next_qrt][cohort]` — distrib is allocated `Util_Alloc_Init_3D_Double(2, numMo… |  | [ ] |
| [MOVE-06](#move-06) | 3623, 1506, 4159 | H/M | wrong index | `amt * tempdistrib[k][cohort]` — tempdistrib is `[wcnz][K_num_max_stages]` (atbiolsetup.c:2898), and `Get_Vert… |  | [ ] |
| [MOVE-07](#move-07) | 3497, 3549 | H/M | wrong index | `MIGRATION[sp].DEN[cohort][qid] += migbiom * totboxden[invert_biosum];` — the total biomass for this cohort is… |  | [ ] |
| [MOVE-08](#move-08) | 3273, 3051 | H/M | off-by-one / buffer overrun | `totboxden[invert_biosum+cohort]` with invert_biosum = 1 (atlantisboxmodel.h:813) reaches index numCohortsXnum… |  | [ ] |
| [MOVE-09](#move-09) | 3107, 3113 | H/M | off-by-one | `if (habitatType == EPIFAUNA) maxk = 0; ... for (k = 0; k < maxk; k++)` — with maxk = 0 the loop never runs, s… |  | [ ] |
| [MOVE-10](#move-10) | 906 | M/H | inverted condition | `if (bm->flagday) day_part = 0; else day_part = 1;` — `distrib_VERTICAL[0]` is "night" and `[1]` is "day" (atB… |  | [ ] |
| [MOVE-11](#move-11) | 4069 | H/M | off-by-one | `else year = bm->larvalYears;` — connectivitymatrix only holds data for years `0..larvalYears-1` (read with co… |  | [ ] |
| [MOVE-12](#move-12) | 4032 | H/M | logic (overwrite instead of AND) | Each check overwrites `go_there` (temperature fail -> 0, then salinity OK -> 1, then O2 OK -> 1). Only the las… |  | [ ] |
| [MOVE-13](#move-13) | 3799, 3808 | H/M | uninitialised/wrong variable | `bm->boxes[b].tr[k][bm->InvaderPID] *= (1.0 + InvadeScalar);` — in the `conditional_invader` case, InvadeScala… |  | [ ] |
| [MOVE-14](#move-14) | 3919, 3937 | H/M | copy-paste / wrong variable | `bk = k; if(bk >= bm->boxes[nb].nz) bk = bm->boxes[nb].nz - 1;` — k loops `< boxes[nb].nz`, so this test can n… |  | [ ] |
| [MOVE-15](#move-15) | 3812 | M/M | normalisation error | The local box's scalar is written to every layer but added to `tot_scalar` only once. Neighbour scalars are ad… |  | [ ] |
| [MOVE-16](#move-16) | 3829 | M/M | copy-paste | `sp_likeCANYON = HABITATlike[..][CANYONcover_id];` is the only habitat term not multiplied by the box's area f… |  | [ ] |
| [MOVE-17](#move-17) | 1625 | H/M | use before compute | Sticky ddepend with flagmother: `if ((roc[ij][age_mat] / roc_wgt) > ...)` runs inside the `n` loop while `n < … |  | [ ] |
| [MOVE-18](#move-18) | 471, 490 | M/M | double counting | `if(!prey_counted[prey][n])` is checked outside the `predstage` loop. Inside it, every predator stage whose ga… |  | [ ] |
| [MOVE-19](#move-19) 🔀 | 1121, 1249 | H/L | dead code | `DEN` is clamped to 0 at 1159, so `if (DEN < 0.0)` at 1231 can never be true, and the SN/RN reset under flagav… |  | [ ] |
| [MOVE-20](#move-20) | 1283, 1270 | M/M | logic | The larval_queue_extension reset happens inside the `n` (gene) and `qid` loops. After the first gene with `sto… |  | [ ] |
| [MOVE-21](#move-21) | 1399, 1396 | H/L | accumulator in wrong loop | `sp_amt += totden[sp][n];` sits inside the stock loop `ij < maxij`, so the total is multiplied by K_num_stocks… |  | [ ] |
| [MOVE-22](#move-22) | 1480 | M/M | truncation | `contract_sp = (int)FunctGroupArray[sp].speciesParams[contract_tol_id];` (and `int contract_sp` in `Check_Cond… |  | [ ] |
| [MOVE-23](#move-23) | 2385 | M/M | order dependency | `if(!totad[ngene]) recruit_hdistrib = distrib[..][adstage];` is inside the box/layer loop, while totad accumul… |  | [ ] |
| [MOVE-24](#move-24) | 2379 | M/M | inconsistent normalisation | The `recruit_hdistrib` numerator leaves out `noise_effect * light_effect`, but the `totad` denominator include… |  | [ ] |
| [MOVE-25](#move-25) | 2056 | M/M | stale state | `LocalPopCount[n]` is only updated when `newden > 0`, so cells with no animals keep the previous cell's value … |  | [ ] |
| [MOVE-26](#move-26) | 1506 | M/M | double application | When enviro_depend is set, light/noise avoidance is applied to `tempdistrib` in `Get_Vertical_Distribution()` … |  | [ ] |
| [MOVE-27](#move-27) | 2789 | H/M | format mismatch | `quit("Time: %e box%d-%d %s-%d ...", bm->dayt, groupCode, n, ij, k)` — the arguments are in the wrong order (`… |  | [ ] |
| [MOVE-28](#move-28) | 2672, 2675, 2649 | H/L | log text | 2672 prints `period = %e` with the int `Leave_Period[qid]` (UB). 2649 prints `Box %d;%d` with `ij, ij` (should… |  | [ ] |
| [MOVE-29](#move-29) | 2080 | M/L | off-by-one | The second suckling loop starts at `ad_cohort = ad_start_cohort`, which the first loop (2066) already handled,… |  | [ ] |
| [MOVE-30](#move-30) | 3185 | H/L | accumulator not reset | `numScalar_final` is never reset per cell, so after the first constrained cell every cell counts as "impacted"… |  | [ ] |

---

## MOVE-01

**swapped indices** · Conf H / Sev H ✔ re-checked ⭐ priority  
**Found at (current code):** `atmovement.c` 2030, `atmovement.c` 2043
**Lines at review (Oct 2026):** 2030

**Function:** `Ecology_Total_Verts_And_Migration()`

**Why it's thought to be a bug**

`newden[sp][n][chkbox][kk] += den_diff;` — newden is allocated `Util_Alloc_Init_4D_Double(ncells, nlayer, ncoh, nsp)` => `[sp][n][layer][box]` (atbiolsetup.c:2869) and used everywhere else as `newden[sp][n][k][ij]`. Here the box index is put in the layer slot, so any `chkbox >= wcnz` writes out of bounds (memory corruption) and the wrong cell gets the displaced fish. Function: `Ecology_Total_Verts_And_Migration()` (flagenviro_displace branch).

**Code at issue** — `atmovement.c` line 2030  
Search for: `newden[sp][n][chkbox][kk] += den_diff`

```c
 2028:                                         is_suitable = Check_Realloc_Conditions(bm, sp, n, chkbox, kk);
 2029:                                         if (is_suitable) {
 2030→                                             newden[sp][n][chkbox][kk] += den_diff;
 2031:                                             check_done = 1;
 2032:                                             den_diff = 0;
```

**Code at issue** — `atmovement.c` line 2043  
Search for: `newden[sp][n][k][ij]`

```c
 2041:                             if (enviro_depend && bm->flagenviro_kill) {
 2042:                                 // Difference between original and final distributions
 2043→                                 den_diff = orig_newden - newden[sp][n][k][ij];
 2044: 
 2045:                                 // Biomass killed - SN to labile detritus, RN to refractory - could parameterise like with FGDL-like parameter
```

**Suggested fix (from review):** `newden[sp][n][kk][chkbox] += den_diff;`

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

## MOVE-02

**wrong operator** · Conf H / Sev H ⭐ priority  
**Found at (current code):** `atmovement.c` 2050
**Lines at review (Oct 2026):** 2049-2050

**Function:** `Ecology_Total_Verts_And_Migration()`

**Why it's thought to be a bug**

`bm->boxes[ij].tr[k][DL_id] = VERTinfo[..SN_id] * totden * den_diff;` — this sets the labile/refractory detritus tracers to the killed biomass instead of adding to it, wiping out the detritus already in the cell. It also runs once per cohort, so each cohort overwrites the one before. The value is also mg N for the whole cell (per-individual N × numbers × proportion) written into a mg N m-3 tracer with no divide by volume. Function: `Ecology_Total_Verts_And_Migration()` (flagenviro_kill).

**Code at issue** — `atmovement.c` line 2050 (closest match to the quoted code; wording differs slightly)  
Search for: `bm->boxes[ij].tr[k][DR_id] = VERTinfo[sp][n][RN_id] * totden[sp][n] * den_diff;`

```c
 2048: 
 2049:                                 bm->boxes[ij].tr[k][DL_id] = VERTinfo[sp][n][SN_id] * totden[sp][n] * den_diff;
 2050→                                 bm->boxes[ij].tr[k][DR_id] = VERTinfo[sp][n][RN_id] * totden[sp][n] * den_diff;
 2051:                             }
 2052:                             
```

**Suggested fix (from review):** Use `+=` and divide by `dz[k]*area`

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

## MOVE-03

**wrong variable** · Conf H / Sev M  
**Found at (current code):** `atmovement.c` 1559
**Lines at review (Oct 2026):** 1552

**Function:** `Ecology_Total_Verts_And_Migration()`

**Why it's thought to be a bug**

`(-1.0 * bm->boxes[bm->current_box].botz) <= ...[maxtotdepth_id]` — this is inside the `ij` box loop, but it uses `bm->current_box`, which is old state left over from the process loop. The two checks next to it use `bm->boxes[ij].botz`, and atbiolsetup.c:3518 does the same max-total-depth test with `ij`. So the depth exclusion for forage-dependent movement uses the wrong box. Function: `Ecology_Total_Verts_And_Migration()`.

**Code at issue** — `atmovement.c` line 1559  
Search for: `(-1.0 * bm->boxes[bm->current_box].botz) <=`

```c
 1557:                                 		( bm->boxes[ij].botz <= (-1.0 * FunctGroupArray[sp].speciesParams[mindepth_id]) &&
 1558:                                 		 bm->boxes[ij].botz >= (-1.0 * FunctGroupArray[sp].speciesParams[maxdepth_id]) &&
 1559→                                         ((-1.0 * bm->boxes[bm->current_box].botz) <= FunctGroupArray[sp].speciesParams[maxtotdepth_id]))) {
 1560:                                 //if ((int) (FunctGroupArray[sp].speciesParams[flag_id]) && (int) (FunctGroupArray[sp].speciesParams[active_id]) &&
 1561:                                 //    (current_depth <= (-1.0 * FunctGroupArray[sp].speciesParams[mindepth_id]) &&
```

**Suggested fix (from review):** `bm->boxes[ij].botz`

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

## MOVE-04

**wrong type / truncation** · Conf H / Sev M  
**Found at (current code):** `atmovement.c` 2998, `atmovement.c` 3074
**Lines at review (Oct 2026):** 2997, 3073-3075

**Function:** `Ecology_Invert_Migration()`

**Why it's thought to be a bug**

`int qrt, next_qrt, this_HowFar, ...` — this_HowFar is an int but gets `bm->HowFar` (a double fraction in [0,1), attime.c:209) or `FunctGroupArray[sp].HowFar`. It truncates to 0, so prescribed invertebrate movement (line 3385) never blends between seasons and jumps when the season changes. The vertebrate routine declares it `double` (line 867). Also, `FunctGroupArray[].HowFar` is itself declared `int` (atlantisUtil/include/atFunctGroup.h:94) but is assigned a fractional remainder (attime.c:524), so it is always 0 for vertebrates too whenever numMoveEntries != 4. Function: `Ecology_Invert_Migration()`.

**Code at issue** — `atmovement.c` line 2998  
Search for: `HowFar`

```c
 2996:     //double step1, step2;
 2997:     double migtime2;
 2998→     int qrt, next_qrt, this_HowFar, agec, sp_Migrate_Years;
 2999: 	//int sp_debug;
 3000:     // do_debug;
```

**Code at issue** — `atmovement.c` line 3074  
Search for: `FunctGroupArray[sp].HowFar`

```c
 3072:         
 3073:         if (FunctGroupArray[sp].numMoveEntries != 4) {
 3074→             this_HowFar = FunctGroupArray[sp].HowFar;
 3075:         } else {
 3076:             this_HowFar = bm->HowFar;
```

**Suggested fix (from review):** Declare `double this_HowFar`; change atFunctGroup.h:94 to `double HowFar`

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

## MOVE-05

**wrong index** · Conf H / Sev M  
**Found at (current code):** `atmovement.c` 3386, `atmovement.c` 3343
**Lines at review (Oct 2026):** 3382

**Function:** `Ecology_Invert_Migration()`

**Why it's thought to be a bug**

`FunctGroupArray[sp].distrib[ij][next_qrt][cohort]` — distrib is allocated `Util_Alloc_Init_3D_Double(2, numMoveEntries, nbox)` => `[nbox][entries][2]`, indexed by stage (atUtilGroupIO.c:917; the vertebrate code uses `[stage]`, e.g. line 1935). Using `cohort` (0..numCohortsXnumGenes-1) reads out of bounds for an invert with more than 2 cohort×gene entries, and reads the wrong stage when genes > 1. Function: `Ecology_Invert_Migration()`.

**Code at issue** — `atmovement.c` line 3386  
Search for: `FunctGroupArray[sp].distrib[ij][next_qrt][cohort]`

```c
 3384:                             if(sp_debug == 2){
 3385:                                 fprintf(llogfp,"day: %e, %s-newden[%d][%d]: %e, HowFar: %e, distrib%s-%d-%d-%d: %e, distrib%s-%d-%d-%d: %e\n",
 3386→                                     bm->dayt, FunctGroupArray[sp].groupCode, cohort, ij, newden[sp][cohort][0][ij], this_HowFar, FunctGroupArray[sp].groupCode, ij, next_qrt, cohort, FunctGroupArray[sp].distrib[ij][next_qrt][cohort], FunctGroupArray[sp].groupCode, ij, qrt, cohort, FunctGroupArray[sp].distrib[ij][qrt][cohort]);
 3387:                             }
 3388:                             */
```

**Code at issue** — `atmovement.c` line 3343  
Search for: `[stage]`

```c
 3341:                             fprintf(llogfp, "Time: %e, %s-%d Migration %d Total biomass back into model = %e ",
 3342:                                 bm->dayt, FunctGroupArray[sp].groupCode, cohort, qid, migbiom * bm->X_CN * mg_2_tonne);
 3343→                             fprintf(bm->logFile, "MIGRATION[sp].DEN[stage][qid] = %e\n", MIGRATION[sp].DEN[cohort][qid] * bm->X_CN * mg_2_tonne);
 3344:                         }
 3345:                         */
```

**Suggested fix (from review):** Use `[stage]`

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

## MOVE-06

**wrong index** · Conf H / Sev M  
**Found at (current code):** `atmovement.c` 3623, `atmovement.c` 1506, `atmovement.c` 4159
**Lines at review (Oct 2026):** 3624, 3627, 3637

**Function:** `Ecology_Invert_Migration()`

**Why it's thought to be a bug**

`amt * tempdistrib[k][cohort]` — tempdistrib is `[wcnz][K_num_max_stages]` (atbiolsetup.c:2894), and `Get_Vertical_Distribution()` only fills `[k][juv_id]` and `[k][stage]` (tracker_id = stage, line 4157). Line 3623 correctly reads `tempdistrib[k][stage]`. Indexing by cohort goes out of bounds or reads unset columns when cohorts×genes > stages. Function: `Ecology_Invert_Migration()`.

**Code at issue** — `atmovement.c` line 3623  
Search for: `amt * tempdistrib[k][cohort]`

```c
 3621: 
 3622:                                 /* Redistribute accordingly */
 3623→                                 bm->boxes[ij].tr[k][pid] = amt * tempdistrib[k][cohort] / (bm->boxes[ij].dz[k] * bm->boxes[ij].area + small_num);
 3624: 
 3625:                                 if (!(_finite(bm->boxes[ij].tr[k][pid] ))) {
```

**Code at issue** — `atmovement.c` line 1506  
Search for: `Get_Vertical_Distribution()`

```c
 1504: 
 1505: 			/* For forage and density dependent movement determine ideal forage locations */
 1506→ 			/* if this group is not vertically mobile then need the currentden for use in Get_Vertical_Distribution() */
 1507:             
 1508:             if (sp_ddepend_move > perscribed_move || FunctGroupArray[sp].isMobile == FALSE) {
```

**Code at issue** — `atmovement.c` line 4159  
Search for: `[k][juv_id]`

```c
 4157: 	// Initialise (just in case)
 4158: 	for (k = 0; k < bm->wcnz; k++) {
 4159→ 		tempdistrib[k][juv_id] = 0.0;
 4160: 		tempdistrib[k][adult_id] = 0.0;
 4161: 	}
```

**Suggested fix (from review):** `tempdistrib[k][stage]`

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

## MOVE-07

**wrong index** · Conf H / Sev M  
**Found at (current code):** `atmovement.c` 3497, `atmovement.c` 3549
**Lines at review (Oct 2026):** 3498

**Function:** `Ecology_Invert_Migration()`

**Why it's thought to be a bug**

`MIGRATION[sp].DEN[cohort][qid] += migbiom * totboxden[invert_biosum];` — the total biomass for this cohort is stored at `totboxden[invert_biosum+cohort]` (lines 3277, 3336, 3553, 3570). Cohort 0's biomass is used for every cohort, so migrant biomass for cohorts > 0 is wrong. Function: `Ecology_Invert_Migration()`.

**Code at issue** — `atmovement.c` line 3497  
Search for: `MIGRATION[sp].DEN[cohort][qid] += migbiom * totboxden[invert_biosum]`

```c
 3495:                                     /* Update the Migration array to take account of the new
 3496:                                         immigrants */
 3497→                                     MIGRATION[sp].DEN[cohort][qid] += migbiom * totboxden[invert_biosum];
 3498:                                     
 3499:                                     MIGRATION[sp].num_stagger[qid] = 0;
```

**Code at issue** — `atmovement.c` line 3549  
Search for: `totboxden[invert_biosum+cohort]`

```c
 3547:                                    in bottom-most water layer and set all other layers
 3548:                                    to zero - divide by volume so back in mgN m-3 */
 3549→                                 bm->boxes[ij].tr[0][pid] = totboxden[invert_biosum+cohort] * newden[sp][cohort][0][ij] / (bm->boxes[ij].dz[0] * bm->boxes[ij].area + small_num);
 3550:                                 
 3551: 								/*
```

**Suggested fix (from review):** `totboxden[invert_biosum+cohort]`

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

## MOVE-08

**off-by-one / buffer overrun** · Conf H / Sev M  
**Found at (current code):** `atmovement.c` 3273, `atmovement.c` 3051
**Lines at review (Oct 2026):** 3274, 3333, 3550, 3567, 818

**Why it's thought to be a bug**

`totboxden[invert_biosum+cohort]` with invert_biosum = 1 (atlantisboxmodel.h:813) reaches index numCohortsXnumGenes. totboxden is allocated with only `K_num_max_cohort*K_num_max_genetypes` (atbiolsetup.c:2918). atbiolsetup.c:2901-2903 computes `totdensize = K_max_invert_cohorts+1` "as need an additional slot for propsum in invertebrate movement code", but applies it to totden/totden_check, not totboxden. So it overruns when an invert has the maximum cohort×gene count. Functions: `Ecology_Invert_Migration()`, `Store_Min_Max_Avg()`.

**Code at issue** — `atmovement.c` line 3273  
Search for: `totboxden[invert_biosum+cohort]`

```c
 3271: 
 3272:                 /* Store biomass so can scale movement distributions */
 3273→                 totboxden[invert_biosum+cohort] = totamt;
 3274:                 
 3275:                 
```

**Code at issue** — `atmovement.c` line 3051  
Search for: `K_num_max_cohort * bm->K_num_max_genetypes`

```c
 3049: 	/* Determine the new distribution for the diel vertical migrating invertebrates */
 3050: 	for (fgIndex = 0; fgIndex < bm->K_num_tot_sp; fgIndex++) {
 3051→         Util_Init_1D_Double(totboxden, (bm->K_num_max_cohort * bm->K_num_max_genetypes), 0.0);
 3052:         
 3053: 		sp = fgIndex;
```

**Suggested fix (from review):** Allocate totboxden with `totdensize` (atbiolsetup.c:2918)

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

## MOVE-09

**off-by-one** · Conf H / Sev M  
**Found at (current code):** `atmovement.c` 3107, `atmovement.c` 3113
**Lines at review (Oct 2026):** 3101-3106, 3376-3379, 3397

**Why it's thought to be a bug**

`if (habitatType == EPIFAUNA) maxk = 0; ... for (k = 0; k < maxk; k++)` — with maxk = 0 the loop never runs, so epifauna skip all temperature, salinity, O2, ice, light, noise and contaminant limits, both for non-mobile groups (3095 block) and for prescribed movement. The epifauna `epi[pid]` branches inside those loops can never run.

**Code at issue** — `atmovement.c` line 3107  
Search for: `for (k = 0; k < maxk; k++)`

```c
 3105:                         maxk = bm->boxes[ij].nz;
 3106:                     
 3107→                     for (k = 0; k < maxk; k++) {
 3108:                         /* Noise and light pollution avoidance */
 3109:                         if (FunctGroupArray[sp].isLightEffected) {
```

**Code at issue** — `atmovement.c` line 3113  
Search for: `epi[pid]`

```c
 3111:                             if (current_enviro > 0) {
 3112:                                 if (FunctGroupArray[fgIndex].habitatType == EPIFAUNA)
 3113→                                     bm->boxes[ij].epi[pid] *= (FunctGroupArray[sp].speciesParams[light_coefft_id] / (current_enviro + small_num));
 3114:                                 else
 3115:                                     bm->boxes[ij].tr[k][pid] *= (FunctGroupArray[sp].speciesParams[light_coefft_id] / (current_enviro + small_num));
```

**Suggested fix (from review):** `maxk = 1` for EPIFAUNA

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

## MOVE-10

**inverted condition** · Conf M / Sev H  
**Found at (current code):** `atmovement.c` 906
**Lines at review (Oct 2026):** 3033-3036

**Function:** `Ecology_Invert_Migration()`

**Why it's thought to be a bug**

`if (bm->flagday) day_part = 0; else day_part = 1;` — `distrib_VERTICAL[0]` is "night" and `[1]` is "day" (atBiologyXMLParamIO.c:93-94), and flagday = 1 means daytime (attime.c Time_ODay; comment at line 906). The vertebrate routine passes `day_part = bm->flagday`. Invertebrates therefore use the night profile in the day and the day profile at night (diel migration reversed), unless this reversal is a deliberate legacy choice. Function: `Ecology_Invert_Migration()`.

**Code at issue** — `atmovement.c` line 906  
Search for: `day_part = bm->flagday`

```c
  904: 	Parameter_Q10(bm, &bm->boxes[1], ldayt, 1, 0, bm->boxes[1].nz - 1, midpoint, WC, llogfp);
  905: 
  906→ 	day_part = bm->flagday; // 1 = day time, 0 = night time
  907: 
  908: 	/* Initialise local arrays */
```

**Suggested fix (from review):** `day_part = bm->flagday;`

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

## MOVE-11

**off-by-one** · Conf H / Sev M  
**Found at (current code):** `atmovement.c` 4069
**Lines at review (Oct 2026):** 4067-4072

**Function:** `Larval_Dispersal()`

**Why it's thought to be a bug**

`else year = bm->larvalYears;` — connectivitymatrix only holds data for years `0..larvalYears-1` (read with count `min(larvalYears, numRunYears)`, atbiolsetup.c:3955). Past the end of the data with larval_rewind off, the code indexes a slot that was never filled (all zeros), so larvaldistrib = 0 and there is no recruitment. Function: `Larval_Dispersal()`.

**Code at issue** — `atmovement.c` line 4069  
Search for: `else`

```c
 4067: 		if (bm->larval_rewind == TRUE)
 4068: 			year = 0;
 4069→ 		else
 4070: 			year = bm->larvalYears;
 4071: 
```

**Suggested fix (from review):** `year = bm->larvalYears - 1;`

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

## MOVE-12

**logic (overwrite instead of AND)** · Conf H / Sev M  
**Found at (current code):** `atmovement.c` 4032
**Lines at review (Oct 2026):** 4008-4042

**Function:** `Invade_Spread()`

**Why it's thought to be a bug**

Each check overwrites `go_there` (temperature fail -> 0, then salinity OK -> 1, then O2 OK -> 1). Only the last active criterion counts. Light and noise set `go_there = ceil(coefft/enviro)`, which can override earlier failures. Invaders can spread into cells that fail temperature or salinity limits. Function: `Invade_Spread()`.

**Code at issue** — `atmovement.c` line 4032 (located by name only; check this is the right place)  
Search for: `go_there = ceil(FunctGroupArray[sp].speciesParams[light_coefft_id] / (current_enviro + sma`

```c
 4030:         current_enviro = bm->boxes[nb].tr[k][Light_Pollution_i];
 4031:         if (current_enviro > 0) {
 4032→             go_there = ceil(FunctGroupArray[sp].speciesParams[light_coefft_id] / (current_enviro + small_num));
 4033:         }
 4034:     }
```

**Suggested fix (from review):** Start with `go_there=1` and set it to 0 on any failure

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

## MOVE-13

**uninitialised/wrong variable** · Conf H / Sev M  
**Found at (current code):** `atmovement.c` 3799, `atmovement.c` 3808
**Lines at review (Oct 2026):** 3800

**Function:** `Ecology_Invading_Species()`

**Why it's thought to be a bug**

`bm->boxes[b].tr[k][bm->InvaderPID] *= (1.0 + InvadeScalar);` — in the `conditional_invader` case, InvadeScalar is never set (it is only computed in `simple_invader`), so it stays 0.0. Invertebrate entrants are never scaled, while the vertebrate branch two lines above uses `bm->InvaderScalar`. Function: `Ecology_Invading_Species()`.

**Code at issue** — `atmovement.c` line 3799  
Search for: `boxes[b].tr[k][bm->InvaderPID] *= (1.0 + InvadeScalar)`

```c
 3797:                         /* Invading species is an invertebrate so scale biomass in first cell
 3798:                          and spill it to adjacent cells through time */
 3799→                         bm->boxes[b].tr[k][bm->InvaderPID] *= (1.0 + InvadeScalar);
 3800:                     }
 3801:                 }
```

**Code at issue** — `atmovement.c` line 3808  
Search for: `InvaderScalar`

```c
 3806:             
 3807:             // Need to have one entry for the local box so they don't all bleed away
 3808→             this_scalar = bm->InvaderScalar;  // As a place holder for now - is just a relative weighting versus what flows to other boxes, which will be downscaled by distance to be moved
 3809:             for (k = 0; k < bm->boxes[b].nz; k++) {
 3810:             	bm->invader_spread_scalars[b][k] = this_scalar;
```

**Suggested fix (from review):** Use `bm->InvaderScalar`

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

## MOVE-14

**copy-paste / wrong variable** · Conf H / Sev M  
**Found at (current code):** `atmovement.c` 3919, `atmovement.c` 3937
**Lines at review (Oct 2026):** 3929-3931, 3938, 3941

**Function:** `Ecology_Invading_Species()`

**Why it's thought to be a bug**

`bk = k; if(bk >= bm->boxes[nb].nz) bk = bm->boxes[nb].nz - 1;` — k loops `< boxes[nb].nz`, so this test can never be true. It was clearly meant to clamp against the source box `b`, since `boxes[b].tr[bk]` is read next. When nb is deeper than b, unused layers of b are read. Function: `Ecology_Invading_Species()`.

**Code at issue** — `atmovement.c` line 3919  
Search for: `< bm->boxes[nb].nz`

```c
 3917:             if (tot_scalar > 0.0) {
 3918:             	for (nb = 0; nb < bm->nbox; nb++) {
 3919→                 	for (k = 0; k < bm->boxes[nb].nz; k++) {
 3920:                 		bm->invader_spread_scalars[nb][k] /= tot_scalar;
 3921:                 	}
```

**Code at issue** — `atmovement.c` line 3937  
Search for: `boxes[b].tr[bk]`

```c
 3935:                             for (n = bm->minInvaderAge; n < bm->maxInvaderAge; n++) {
 3936:                                 den = FunctGroupArray[bm->InvaderIndex].NumsTracers[n];
 3937→                                 bm->boxes[nb].tr[k][den] += (this_scalar * bm->boxes[b].tr[bk][den]);
 3938:                             }
 3939:                         } else {
```

**Suggested fix (from review):** `if (bk >= bm->boxes[b].nz) bk = bm->boxes[b].nz - 1;`

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

## MOVE-15

**normalisation error** · Conf M / Sev M  
**Found at (current code):** `atmovement.c` 3812
**Lines at review (Oct 2026):** 3810-3813, 3820-3910, 3918-3923

**Function:** `Ecology_Invading_Species()`

**Why it's thought to be a bug**

The local box's scalar is written to every layer but added to `tot_scalar` only once. Neighbour scalars are added to `tot_scalar` once per invader cohort n (loop at 3820) but stored only once per [nb][k], with the last n winning. Each source layer then keeps `s_b/tot` and sends `s_nb,k/tot` to each neighbour layer, so the fractions for a layer do not add to 1 and invader biomass/numbers are not conserved. Function: `Ecology_Invading_Species()`.

**Code at issue** — `atmovement.c` line 3812 (the line the review pointed to, found by its text)  
Search for: `tot_scalar += this_scalar;`

```c
 3810:             	bm->invader_spread_scalars[b][k] = this_scalar;
 3811:             }
 3812→             tot_scalar += this_scalar;
 3813:             
 3814:             // Now do neighbouring boxes
```

**Suggested fix (from review):** Build the scalars per source layer and normalise per layer, outside the cohort loop

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

## MOVE-16

**copy-paste** · Conf M / Sev M  
**Found at (current code):** `atmovement.c` 3829
**Lines at review (Oct 2026):** 3830

**Function:** `Ecology_Invading_Species()`

**Why it's thought to be a bug**

`sp_likeCANYON = HABITATlike[..][CANYONcover_id];` is the only habitat term not multiplied by the box's area fraction. REEF, FLAT and SOFT are multiplied by `boxes[nb].reef/flat/soft`, and `double canyon` exists in the box struct (atlantisboxmodel.h:3878). Any canyon-liking invader therefore treats every box as suitable habitat. Function: `Ecology_Invading_Species()`.

**Code at issue** — `atmovement.c` line 3829 (closest match to the quoted code; wording differs slightly)  
Search for: `sp_likeCANYON = bm->HABITATlike[bm->InvaderIndex][chrtstage][bm->CANYONcover_id];`

```c
 3827:                             sp_likeFLAT = bm->HABITATlike[bm->InvaderIndex][chrtstage][bm->FLATcover_id] * bm->boxes[nb].flat;
 3828:                             sp_likeSOFT = bm->HABITATlike[bm->InvaderIndex][chrtstage][bm->SOFTcover_id] * bm->boxes[nb].soft;
 3829→                             sp_likeCANYON = bm->HABITATlike[bm->InvaderIndex][chrtstage][bm->CANYONcover_id];
 3830:                             if ((sp_likeREEF > 0.0) || (sp_likeFLAT > 0.0) || (sp_likeSOFT > 0.0) || (sp_likeCANYON > 0.0))
 3831:                                 sp_like = 1;
```

**Suggested fix (from review):** `* bm->boxes[nb].canyon`

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

## MOVE-17

**use before compute** · Conf H / Sev M  
**Found at (current code):** `atmovement.c` 1625
**Lines at review (Oct 2026):** 1613-1618

**Function:** `Ecology_Total_Verts_And_Migration()`

**Why it's thought to be a bug**

Sticky ddepend with flagmother: `if ((roc[ij][age_mat] / roc_wgt) > ...)` runs inside the `n` loop while `n < age_mat`, so roc[ij][age_mat] for this species and timestep has not been computed yet. It holds the value from the previous species or timestep. (The same test at 1864 runs after all roc are computed, so it is fine.) Function: `Ecology_Total_Verts_And_Migration()`.

**Code at issue** — `atmovement.c` line 1625  
Search for: `if ((roc[ij][age_mat] / roc_wgt) >`

```c
 1623: 								for (k = 0; k < bm->boxes[ij].nz; k++) {
 1624: 									if ((flagmother > 0) && (n < age_mat)) {
 1625→ 										if ((roc[ij][age_mat] / roc_wgt) > (k_roc_food * VERTinfo[sp][n][SN_id])) {
 1626: 											boxden[ij][n] += currentden[sp][n][k][ij];
 1627: 											totboxden[n] += currentden[sp][n][k][ij];
```

**Suggested fix (from review):** Compute roc for all n first, then do the sticky boxden pass in a second loop

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

## MOVE-18

**double counting** · Conf M / Sev M  
**Found at (current code):** `atmovement.c` 471, `atmovement.c` 490
**Lines at review (Oct 2026):** 472-492

**Function:** `Get_Vert_Diet()`

**Why it's thought to be a bug**

`if(!prey_counted[prey][n])` is checked outside the `predstage` loop. Inside it, every predator stage whose gape window fits adds `(sn+rn)*den` to preyamt again, so prey are counted twice when a predator has 2 stages that both match. This always happens under flag_olddiet, where preyavail is the same for both stages. Function: `Get_Vert_Diet()`.

**Code at issue** — `atmovement.c` line 471  
Search for: `if(!prey_counted[prey][n])`

```c
  469: 		for (pred = 0; pred < bm->K_num_tot_sp; pred++) {
  470: 			if (FunctGroupArray[pred].speciesParams[flag_id] == TRUE) {
  471→ 				if(!prey_counted[prey][n]){
  472: 					for(predstage = 0; predstage < FunctGroupArray[pred].numStages; predstage++){
  473: 						if(bm->flag_olddiet){
```

**Code at issue** — `atmovement.c` line 490 (closest match to the quoted code; wording differs slightly)  
Search for: `preyamt[prey][ij][preystage] += (boxLayerTracers[sn] + boxLayerTracers[rn]) * boxLayerTrac`

```c
  488: 							}
  489: 							if ((bm->boxes[ij].tr[k][sn] >= lower) && (boxLayerTracers[sn] <= upper)) {
  490→ 								preyamt[prey][ij][preystage] += (boxLayerTracers[sn] + boxLayerTracers[rn]) * boxLayerTracers[den];
  491: 								prey_counted[prey][n] = 1;
  492: 							}
```

**Suggested fix (from review):** Break out of the predstage loop once `prey_counted[prey][n]=1`, or move the check inside

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

## MOVE-19

**dead code** · Conf H / Sev L  
**Found at (current code):** `atmovement.c` 1121, `atmovement.c` 1249
**Lines at review (Oct 2026):** 1159-1160, 1231-1243, 1246-1253

**Function:** `Ecology_Total_Verts_And_Migration()`

**Why it's thought to be a bug**

`DEN` is clamped to 0 at 1159, so `if (DEN < 0.0)` at 1231 can never be true, and the SN/RN reset under flagavgmig never happens. The 1246 test `dayt > Return_Now+Return_Period` can never be true inside the enclosing condition at 1120 (`dayt < Return_Now+Return_Period`), so `Checked[qid]` is never set here. Function: `Ecology_Total_Verts_And_Migration()`.

> 🔀 **Changed by the merge of main (Nov 2026):** the code for this finding was changed by the merge. Check whether this finding is now fixed before acting.

**Code at issue** — `atmovement.c` line 1121 (closest match to the quoted code; wording differs slightly)  
Search for: `if ((spmigrate && (MIGRATION[sp].Return_Period[qid] > 0)) && (bm->dayt >= MIGRATION[sp].Re`

```c
 1119:                     
 1120: 					/* Migrating species returning to the model */
 1121→ 					if ((spmigrate && (MIGRATION[sp].Return_Period[qid] > 0)) && (bm->dayt >= MIGRATION[sp].Return_Now[qid]) && (bm->dayt < (MIGRATION[sp].Return_Now[qid] + MIGRATION[sp].Return_Period[qid])) && (bm->t / 86400.0 > 0.0) && (MIGRATION[sp].DEN[n][qid] > 0)) {  // Only bother doing it if any individuals to return
 1122:                         
 1123:                         /**
```

**Code at issue** — `atmovement.c` line 1249  
Search for: `Checked[qid]`

```c
 1247:                         
 1248:                         /* Make sure empty if all migrants back */
 1249→                         if ((MIGRATION[sp].Checked[qid] < 1) && (bm->dayt > (MIGRATION[sp].Return_Now[qid] + MIGRATION[sp].Return_Period[qid]))) {
 1250: 							if (bm->flagavgmig) {
 1251:                                 MIGRATION[sp].SN[n][qid] = 0.0;
```

**Suggested fix (from review):** Test `<= 0.0` at 1231; move the 1246 block outside the return-window `if`

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

## MOVE-20

**logic** · Conf M / Sev M  
**Found at (current code):** `atmovement.c` 1283, `atmovement.c` 1270
**Lines at review (Oct 2026):** 1270-1278

**Function:** `Ecology_Total_Verts_And_Migration()`

**Why it's thought to be a bug**

The larval_queue_extension reset happens inside the `n` (gene) and `qid` loops. After the first gene with `stock_id == numStocks-1` resets it to 0, later genes skip their pre-model larvae, which are lost. If `stock_id < 0` it is set to 0 (line 1191), so the reset only fires when numStocks == 1. Function: `Ecology_Total_Verts_And_Migration()`.

**Code at issue** — `atmovement.c` line 1283 (closest match to the quoted code; wording differs slightly)  
Search for: `if (stock_id == (FunctGroupArray[sp].numStocks - 1)) {`

```c
 1281:                                 }
 1282:                                 // Reset larval_queue_extension so not called in subsequent years
 1283→                                 if (stock_id == (FunctGroupArray[sp].numStocks - 1)) {
 1284:                                     FunctGroupArray[sp].speciesParams[larval_queue_extension_id] = 0;
 1285:                                 }
```

**Code at issue** — `atmovement.c` line 1270  
Search for: `stock_id < 0`

```c
 1268:                         **/
 1269: 
 1270→                         if (stock_id < 0) {
 1271:                           stock_id = 0; 
 1272:                         }
```

**Suggested fix (from review):** Reset after the n/qid loops

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

## MOVE-21

**accumulator in wrong loop** · Conf H / Sev L  
**Found at (current code):** `atmovement.c` 1399, `atmovement.c` 1396
**Lines at review (Oct 2026):** 1392

**Function:** `Ecology_Total_Verts_And_Migration()`

**Why it's thought to be a bug**

`sp_amt += totden[sp][n];` sits inside the stock loop `ij < maxij`, so the total is multiplied by K_num_stocks_per_sp. This loosens the "less than one individual present" skip at 1404. Function: `Ecology_Total_Verts_And_Migration()`.

**Code at issue** — `atmovement.c` line 1399  
Search for: `sp_amt += totden[sp][n]`

```c
 1397: 				for (n = 0; n < FunctGroupArray[sp].numCohortsXnumGenes; n++) {
 1398: 					bm->stock_struct_prop[sp][n][ij] = init_stock_struct_prop[sp][n][ij] / (totden[sp][n] + small_num);
 1399→ 					sp_amt += totden[sp][n];
 1400: 
 1401:                     /**
```

**Code at issue** — `atmovement.c` line 1396  
Search for: `ij < maxij`

```c
 1394:             
 1395:             /* Normalise proportion of total abundance in each stock */
 1396→ 			for (ij = 0; ij < maxij; ij++) {
 1397: 				for (n = 0; n < FunctGroupArray[sp].numCohortsXnumGenes; n++) {
 1398: 					bm->stock_struct_prop[sp][n][ij] = init_stock_struct_prop[sp][n][ij] / (totden[sp][n] + small_num);
```

**Suggested fix (from review):** Accumulate outside the stock loop

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

## MOVE-22

**truncation** · Conf M / Sev M  
**Found at (current code):** `atmovement.c` 1480
**Lines at review (Oct 2026):** 1473, 4384/4416

**Why it's thought to be a bug**

`contract_sp = (int)FunctGroupArray[sp].speciesParams[contract_tol_id];` (and `int contract_sp` in `Check_Conditions()`) truncates the pH tolerance contraction, e.g. 0.5 -> 0. The invertebrate routine (3194) correctly keeps it as a double. Functions: `Ecology_Total_Verts_And_Migration()`, `Check_Conditions()`.

**Code at issue** — `atmovement.c` line 1480  
Search for: `contract_sp = (int)FunctGroupArray[sp].speciesParams[contract_tol_id]`

```c
 1478: 			pH_scale = (FunctGroupArray[sp].pHcorr - 1.0);
 1479: 			if( flagcontract_sp  && (pH_scale > 0.0)) {
 1480→ 				contract_sp = (int)FunctGroupArray[sp].speciesParams[contract_tol_id];
 1481: 				min_spawntemp_sp += contract_sp * pH_scale;
 1482: 				max_spawntemp_sp -= contract_sp * pH_scale;
```

**Suggested fix (from review):** Remove the cast; declare as double

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

## MOVE-23

**order dependency** · Conf M / Sev M  
**Found at (current code):** `atmovement.c` 2385
**Lines at review (Oct 2026):** 2383-2386

**Function:** `Ecology_Total_Verts_And_Migration()`

**Why it's thought to be a bug**

`if(!totad[ngene]) recruit_hdistrib = distrib[..][adstage];` is inside the box/layer loop, while totad accumulates across boxes. Boxes processed before the first adult is found get the prescribed distribution; later empty boxes get 0. The fallback is also not added to totad, so the later normalisation is off. Function: `Ecology_Total_Verts_And_Migration()`.

**Code at issue** — `atmovement.c` line 2385 (located by name only; check this is the right place)  
Search for: `bm->recruit_hdistrib[ngene][ij][sp] = FunctGroupArray[sp].distrib[ij][qrt][adstage];`

```c
 2383:                                     if(!totad[ngene]) {  // If no adults about set back to the prescribed distribution
 2384:                                         adstage = FunctGroupArray[sp].cohort_stage[age_mat];
 2385→                                         bm->recruit_hdistrib[ngene][ij][sp] = FunctGroupArray[sp].distrib[ij][qrt][adstage];
 2386:                                     }
 2387:                                     
```

**Suggested fix (from review):** Check `totad` after the box loop and apply the fallback to all boxes

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

## MOVE-24

**inconsistent normalisation** · Conf M / Sev M  
**Found at (current code):** `atmovement.c` 2379
**Lines at review (Oct 2026):** 2378-2379, 2390-2391

**Function:** `Ecology_Total_Verts_And_Migration()`

**Why it's thought to be a bug**

The `recruit_hdistrib` numerator leaves out `noise_effect * light_effect`, but the `totad` denominator includes them. After normalisation (2399) the distribution does not sum to 1, and light/noise reduce the denominator instead of steering larvae away. Function: `Ecology_Total_Verts_And_Migration()`.

**Code at issue** — `atmovement.c` line 2379  
Search for: `noise_effect * light_effect`

```c
 2377:                                         /* Sum over current distribution of adult age groups */
 2378:                                         bm->recruit_hdistrib[ngene][ij][sp] += newden[sp][n][k][ij] * temp_effect * salt_effect * o2_effect * ice_effect;
 2379→                                         totad[ngene] += newden[sp][n][k][ij] * temp_effect * salt_effect * o2_effect * ice_effect * noise_effect * light_effect;
 2380:                                         
 2381:                                     }
```

**Suggested fix (from review):** Apply the same factors to both

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

## MOVE-25

**stale state** · Conf M / Sev M  
**Found at (current code):** `atmovement.c` 2056
**Lines at review (Oct 2026):** 2056, 2060-2097

**Function:** `Ecology_Total_Verts_And_Migration()`

**Why it's thought to be a bug**

`LocalPopCount[n]` is only updated when `newden > 0`, so cells with no animals keep the previous cell's value and trigger suckling transfers that should not happen. `suckler_contam_today_id = 1` (2100) is reset inside the per-cell (ij,k) loop, so the "skip on birth day" set in atdemography.c:473 only applies to the first cell. Function: `Ecology_Total_Verts_And_Migration()`.

**Code at issue** — `atmovement.c` line 2056  
Search for: `LocalPopCount[n]`

```c
 2054:                             if(bm->track_contaminants && bm->flag_contamMaternalTransfer) {
 2055:                                 if((flagmother > 0) && (newden[sp][n][k][ij] > 0)) {
 2056→                                     FunctGroupArray[sp].LocalPopCount[n] = newden[sp][n][k][ij];
 2057:                                 }
 2058:                             }
```

**Suggested fix (from review):** Reset LocalPopCount every cell; move the 2097 reset after the box loop

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

## MOVE-26

**double application** · Conf M / Sev M  
**Found at (current code):** `atmovement.c` 1506
**Lines at review (Oct 2026):** 1973-1985 vs 4306-4320

**Why it's thought to be a bug**

When enviro_depend is set, light/noise avoidance is applied to `tempdistrib` in `Get_Vertical_Distribution()` (which feeds `vertdistrib` into newden) and then applied again to newden at 1973-1985. Inverts get the same double effect (3402-3414).

**Code at issue** — `atmovement.c` line 1506  
Search for: `Get_Vertical_Distribution()`

```c
 1504: 
 1505: 			/* For forage and density dependent movement determine ideal forage locations */
 1506→ 			/* if this group is not vertically mobile then need the currentden for use in Get_Vertical_Distribution() */
 1507:             
 1508:             if (sp_ddepend_move > perscribed_move || FunctGroupArray[sp].isMobile == FALSE) {
```

**Suggested fix (from review):** Apply in one place only

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

## MOVE-27

**format mismatch** · Conf H / Sev M  
**Found at (current code):** `atmovement.c` 2789
**Lines at review (Oct 2026):** 2789

**Function:** `Ecology_Total_Verts_And_Migration()`

**Why it's thought to be a bug**

`quit("Time: %e box%d-%d %s-%d ...", bm->dayt, groupCode, n, ij, k)` — the arguments are in the wrong order (`%d` gets a char*, `%s` gets an int), so the error path will likely segfault instead of printing. Function: `Ecology_Total_Verts_And_Migration()`.

**Code at issue** — `atmovement.c` line 2789  
Search for: `quit("Time: %e box%d-%d %s-%d`

```c
 2787:                                     bm->boxes[ij].tr[k][rn], VERTinfo[sp][n][SN_id], VERTinfo[sp][n][RN_id]);
 2788:                                 
 2789→                                 quit("Time: %e box%d-%d %s-%d had negative size or numbers after movement\n", bm->dayt, FunctGroupArray[sp].groupCode, n, ij, k);
 2790:                             }
 2791: 
```

**Suggested fix (from review):** Pass `bm->dayt, ij, k, groupCode, n`

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

## MOVE-28

**log text** · Conf H / Sev L  
**Found at (current code):** `atmovement.c` 2672, `atmovement.c` 2675, `atmovement.c` 2649
**Lines at review (Oct 2026):** 2672, 2649, 2511/2515/2769/2773, 3630

**Why it's thought to be a bug**

2672 prints `period = %e` with the int `Leave_Period[qid]` (UB). 2649 prints `Box %d;%d` with `ij, ij` (should be ij, k). The negative-size diagnostics and the invert quit print `bm->current_box, bm->current_layer` instead of `ij, k`.

**Code at issue** — `atmovement.c` line 2672  
Search for: `period = %e`

```c
 2670: 														ij, k,FunctGroupArray[sp].groupCode, n);
 2671: 												warn("migProp = %e, mignum- %e, newden = %e\n", migProp, mignum, newden[sp][n][k][ij]);
 2672→ 												warn("away = %d, period = %e, IOBox = %e\n", MIGRATION[sp].Leave_Now[qid], MIGRATION[sp].Leave_Period[qid], MIGRATION[sp].Box[ij][qid]);
 2673: 
 2674: 												fprintf(stderr, "MigrateAway[%d][%d][%d] = %d\n", sp, stage, qid, MIGRATION[sp].Leave_Now[qid]);
```

**Code at issue** — `atmovement.c` line 2675  
Search for: `Leave_Period[qid]`

```c
 2673: 
 2674: 												fprintf(stderr, "MigrateAway[%d][%d][%d] = %d\n", sp, stage, qid, MIGRATION[sp].Leave_Now[qid]);
 2675→ 												fprintf(stderr, "MigratePeriod[%d][%d][%d] = %d\n",  sp, stage, qid, MIGRATION[sp].Leave_Period[qid]);
 2676: 												fprintf(stderr, "MigIOBox[%d][%d][%d][%d]) = %e\n", sp, ij, stage, qid, MIGRATION[sp].Box[ij][qid]);
 2677: 											}
```

**Code at issue** — `atmovement.c` line 2649  
Search for: `Box %d;%d`

```c
 2647: 
 2648: 											if(newden[sp][n][k][ij] < 0){
 2649→ 												warn("before migration newden is less than 0. Box %d;%d group %s:%d\n", ij, ij, FunctGroupArray[sp].groupCode, n);
 2650: 												quit("");
 2651: 											}
```

**Suggested fix (from review):** Fix the specifiers and arguments

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

## MOVE-29

**off-by-one** · Conf M / Sev L  
**Found at (current code):** `atmovement.c` 2080
**Lines at review (Oct 2026):** 2080

**Function:** `Ecology_Total_Verts_And_Migration()`

**Why it's thought to be a bug**

The second suckling loop starts at `ad_cohort = ad_start_cohort`, which the first loop (2069) already handled, so that cohort's contaminants are transferred twice. Function: `Ecology_Total_Verts_And_Migration()`.

**Code at issue** — `atmovement.c` line 2080  
Search for: `ad_cohort = ad_start_cohort`

```c
 2078:                                     }
 2079:                                     if(not_done > 0){
 2080→                                         for (ad_cohort = ad_start_cohort; ad_cohort > (age_mat - 1); ad_cohort--) { // Iterate over the rest of the adults if we have to
 2081:                                             ad_num = FunctGroupArray[sp].LocalPopCount[ad_cohort];
 2082:                                             if(ad_num > bm->min_dens) {
```

**Suggested fix (from review):** Start at `ad_start_cohort - 1`

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

## MOVE-30

**accumulator not reset** · Conf H / Sev L  
**Found at (current code):** `atmovement.c` 3185
**Lines at review (Oct 2026):** 3184, 3460

**Function:** `Ecology_Invert_Migration()`

**Why it's thought to be a bug**

`numScalar_final` is never reset per cell, so after the first constrained cell every cell counts as "impacted" in the flagenviro_warn log. Function: `Ecology_Invert_Migration()`.

**Code at issue** — `atmovement.c` line 3185 (located by name only; check this is the right place)  
Search for: `if(bm->flagenviro_warn && (numScalar_final != 1.0)) {`

```c
 3183:                         }
 3184:                         
 3185→                         if(bm->flagenviro_warn && (numScalar_final != 1.0)) {
 3186:                             cells_impacted++;
 3187:                         }
```

**Suggested fix (from review):** Reset to 1.0 per cell

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

`n < age_mat`, `roc[ij][age_mat]`, `totroc[age_mat]` (1620, 1804-1808, 1865-1872, 2064) and `PartialMinAge/MaxAge` vs `n` (2629) compare an age-class/cohort number with the cohort×genotype index `n` (layout is `n = numGeneTypes*cohort + gene`, see 2314). These are wrong when numGeneTypes > 1. The same applies to `drandom(start_n=age_mat, end_n=numCohortsXnumGenes)` at 2065.

- [ ] Investigate   - [ ] Ignore

**Your comments:**

> 

### LC-2

In the flagenviro_displace branch (2020-2040), fish moved into a neighbour `chkbox > ij` are overwritten when that box is processed later (newden is assigned, not added to). The neighbour can also be a BOUNDARY/LAND box.

- [ ] Investigate   - [ ] Ignore

**Your comments:**

> 

### LC-3

Tidal channel code (2122-2148): the `<` test at 2119, the `>` at 2122 and the `>=` at 2131 are inconsistent. The `area*current_botz` weighting (2126) vs `area*dz[k]` (2136) depends on the sign of botz (negative botz makes nbox_spread <= 0). `tempdistrib[k][stage]` is left over from the last Get_Vertical_Distribution call, not from chkbox. The `else` branch zeroes neighbouring shallow boxes' densities rather than moving them.

- [ ] Investigate   - [ ] Ignore

**Your comments:**

> 

### LC-4

`Store_Min_Max_Avg()` line 763 returns unless *both* flagFonly and flag_systcap are set (`||` of negations). Check whether `&&` was intended.

- [ ] Investigate   - [ ] Ignore

**Your comments:**

> 

### LC-5

`avgsn/avgrn/finalmig` (2723-2725) are weighted by cumulative `totmig` (which includes earlier DEN) and pooled across cohorts. This only affects the box-0 diagnostic storage.

- [ ] Investigate   - [ ] Ignore

**Your comments:**

> 

### LC-6

`Get_Vertical_Distribution()` non-mobile branch with cohort == -1 uses cohorts 0 and 1 (`currentden[species][juv_id/adult_id]`) as the juvenile/adult profiles.

- [ ] Investigate   - [ ] Ignore

**Your comments:**

> 

### LC-7

For land boxes in the recruit_hdistrib update (2313-2331), the non-BOUNDARY layer loop also runs (its LAND exclusion is commented out), so land-box adults may be counted twice.

- [ ] Investigate   - [ ] Ignore

**Your comments:**

> 

### LC-8

In the invert enviro checks (3400-3466), only `newden[..][0][ij]` is used afterwards, so scalars applied for layers k > 0 have no effect. Only bottom-layer conditions matter (partly acknowledged by the TODO).

- [ ] Investigate   - [ ] Ignore

**Your comments:**

> 

### LC-9

`Get_Ice_Presence(..., ij, 0, ...)` at 3161/3446 uses layer 0 instead of k.

- [ ] Investigate   - [ ] Ignore

**Your comments:**

> 

### LC-10

Division risks: `mum_SP` = 0 (398), `MinYearsAway` = 0 (1217), `x*(1-IOBox)+y` (168).

- [ ] Investigate   - [ ] Ignore

**Your comments:**

> 

### LC-11

`preyamt` is allocated with only 2 stages (atbiolsetup.c:2915) and indexed by `preystage` up to numStages-1. That is fine while K_num_max_stages <= 2.

- [ ] Investigate   - [ ] Ignore

**Your comments:**

> 
