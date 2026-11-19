# atecology/atmovement.c — bug review

Generated 2026-11-19 from `atlantis/to-do-fixt-list.md`. **30 findings** (severity H: 3; confidence H and severity H: 2; on the priority list: 2), plus 11 lower-confidence notes.

Line numbers were re-mapped on 2026-11-19 to the current code (after the merge of main). Each was traced through git history from the version the review used, then checked against the text of the line. **Lines (current code)** and line numbers in the text are now current; ≈ means the code there has changed so the position is approximate. The review's own numbers are kept as **Lines at review**; a number ending in ? could not be traced and is the review's number. ⚠ means the quoted code is no longer at those lines: it may have changed or already been fixed.

Conf/Sev: confidence it's a bug / severity if it is (H = wrong results or crash, M = wrong in some configs, L = cosmetic, log, leak or latent). ✔ = the reviewer re-checked it by hand. ⭐ = on the high-priority list.

## Summary

| ID | Lines (current) | Conf/Sev | Category | Short description | Decision | Done |
|---|---|---|---|---|---|---|
| [MOVE-01](#move-01) ✔ ⭐ | 2033 | H/H | swapped indices | `newden[sp][n][chkbox][kk] += den_diff;` — newden is allocated `Util_Alloc_Init_4D_Double(ncells, nlayer, ncoh… |  | [ ] |
| [MOVE-02](#move-02) ⭐ | 2052-2053 | H/H | wrong operator | `bm->boxes[ij].tr[k][DL_id] = VERTinfo[..SN_id] * totden * den_diff;` — this sets the labile/refractory detrit… |  | [ ] |
| [MOVE-03](#move-03) | 1559 | H/M | wrong variable | `(-1.0 * bm->boxes[bm->current_box].botz) <= ...[maxtotdepth_id]` — this is inside the `ij` box loop, but it u… |  | [ ] |
| [MOVE-04](#move-04) | 3000, 3076-3078 | H/M | wrong type / truncation | `int qrt, next_qrt, this_HowFar, ...` — this_HowFar is an int but gets `bm->HowFar` (a double fraction in [0,1… |  | [ ] |
| [MOVE-05](#move-05) | 3385 | H/M | wrong index | `FunctGroupArray[sp].distrib[ij][next_qrt][cohort]` — distrib is allocated `Util_Alloc_Init_3D_Double(2, numMo… |  | [ ] |
| [MOVE-06](#move-06) | 3627, 3630, 3640 | H/M | wrong index | `amt * tempdistrib[k][cohort]` — tempdistrib is `[wcnz][K_num_max_stages]` (atbiolsetup.c:2898), and `Get_Vert… |  | [ ] |
| [MOVE-07](#move-07) | 3501 | H/M | wrong index | `MIGRATION[sp].DEN[cohort][qid] += migbiom * totboxden[invert_biosum];` — the total biomass for this cohort is… |  | [ ] |
| [MOVE-08](#move-08) | 3264, 3323, 3541, 3558, 823 | H/M | off-by-one / buffer overrun | `totboxden[invert_biosum+cohort]` with invert_biosum = 1 (atlantisboxmodel.h:813) reaches index numCohortsXnum… |  | [ ] |
| [MOVE-09](#move-09) | 3104-3109, 3379-3382, 3400 | H/M | off-by-one | `if (habitatType == EPIFAUNA) maxk = 0; ... for (k = 0; k < maxk; k++)` — with maxk = 0 the loop never runs, s… |  | [ ] |
| [MOVE-10](#move-10) | 3036-3039 | M/H | inverted condition | `if (bm->flagday) day_part = 0; else day_part = 1;` — `distrib_VERTICAL[0]` is "night" and `[1]` is "day" (atB… |  | [ ] |
| [MOVE-11](#move-11) | 4070-4075 | H/M | off-by-one | `else year = bm->larvalYears;` — connectivitymatrix only holds data for years `0..larvalYears-1` (read with co… |  | [ ] |
| [MOVE-12](#move-12) | 4011-4045 | H/M | logic (overwrite instead of AND) | Each check overwrites `go_there` (temperature fail -> 0, then salinity OK -> 1, then O2 OK -> 1). Only the las… |  | [ ] |
| [MOVE-13](#move-13) | 3803 | H/M | uninitialised/wrong variable | `bm->boxes[b].tr[k][bm->InvaderPID] *= (1.0 + InvadeScalar);` — in the `conditional_invader` case, InvadeScala… |  | [ ] |
| [MOVE-14](#move-14) | 3920-3922, 3929, 3932 | H/M | copy-paste / wrong variable | `bk = k; if(bk >= bm->boxes[nb].nz) bk = bm->boxes[nb].nz - 1;` — k loops `< boxes[nb].nz`, so this test can n… |  | [ ] |
| [MOVE-15](#move-15) | 3813-3816, 3823-3913, 3921-3926 | M/M | normalisation error | The local box's scalar is written to every layer but added to `tot_scalar` only once. Neighbour scalars are ad… |  | [ ] |
| [MOVE-16](#move-16) | 3833 | M/M | copy-paste | `sp_likeCANYON = HABITATlike[..][CANYONcover_id];` is the only habitat term not multiplied by the box's area f… |  | [ ] |
| [MOVE-17](#move-17) | 1620-1625 | H/M | use before compute | Sticky ddepend with flagmother: `if ((roc[ij][age_mat] / roc_wgt) > ...)` runs inside the `n` loop while `n < … |  | [ ] |
| [MOVE-18](#move-18) | 472-492 | M/M | double counting | `if(!prey_counted[prey][n])` is checked outside the `predstage` loop. Inside it, every predator stage whose ga… |  | [ ] |
| [MOVE-19](#move-19) 🔀 | 1169-1170, 1234-1246, 1249-1256 | H/L | dead code | `DEN` is clamped to 0 at 1159, so `if (DEN < 0.0)` at 1231 can never be true, and the SN/RN reset under flagav… |  | [ ] |
| [MOVE-20](#move-20) | 1273-1281 | M/M | logic | The larval_queue_extension reset happens inside the `n` (gene) and `qid` loops. After the first gene with `sto… |  | [ ] |
| [MOVE-21](#move-21) | 1399 | H/L | accumulator in wrong loop | `sp_amt += totden[sp][n];` sits inside the stock loop `ij < maxij`, so the total is multiplied by K_num_stocks… |  | [ ] |
| [MOVE-22](#move-22) | 1480, 4375/4409 | M/M | truncation | `contract_sp = (int)FunctGroupArray[sp].speciesParams[contract_tol_id];` (and `int contract_sp` in `Check_Cond… |  | [ ] |
| [MOVE-23](#move-23) | 2373-2376 | M/M | order dependency | `if(!totad[ngene]) recruit_hdistrib = distrib[..][adstage];` is inside the box/layer loop, while totad accumul… |  | [ ] |
| [MOVE-24](#move-24) | 2368-2369, 2380-2381 | M/M | inconsistent normalisation | The `recruit_hdistrib` numerator leaves out `noise_effect * light_effect`, but the `totad` denominator include… |  | [ ] |
| [MOVE-25](#move-25) | 2059, 2063-2100 | M/M | stale state | `LocalPopCount[n]` is only updated when `newden > 0`, so cells with no animals keep the previous cell's value … |  | [ ] |
| [MOVE-26](#move-26) | 1976-1988 vs 4309-4323 | M/M | double application | When enviro_depend is set, light/noise avoidance is applied to `tempdistrib` in `Get_Vertical_Distribution()` … |  | [ ] |
| [MOVE-27](#move-27) | 2779 | H/M | format mismatch | `quit("Time: %e box%d-%d %s-%d ...", bm->dayt, groupCode, n, ij, k)` — the arguments are in the wrong order (`… |  | [ ] |
| [MOVE-28](#move-28) | 2675, 2652, 2514/2518/2772/2776, 3633 | H/L | log text | 2672 prints `period = %e` with the int `Leave_Period[qid]` (UB). 2649 prints `Box %d;%d` with `ij, ij` (should… |  | [ ] |
| [MOVE-29](#move-29) | 2083 | M/L | off-by-one | The second suckling loop starts at `ad_cohort = ad_start_cohort`, which the first loop (2066) already handled,… |  | [ ] |
| [MOVE-30](#move-30) | 3187, 3463 | H/L | accumulator not reset | `numScalar_final` is never reset per cell, so after the first constrained cell every cell counts as "impacted"… |  | [ ] |

---

## MOVE-01

**swapped indices** · Conf H / Sev H ✔ re-checked ⭐ priority  
**Lines (current code):** 2033  
**Lines at review (Oct 2026):** 2030

**Function:** `Ecology_Total_Verts_And_Migration()`

**Why it's thought to be a bug**

`newden[sp][n][chkbox][kk] += den_diff;` — newden is allocated `Util_Alloc_Init_4D_Double(ncells, nlayer, ncoh, nsp)` => `[sp][n][layer][box]` (atbiolsetup.c:2869) and used everywhere else as `newden[sp][n][k][ij]`. Here the box index is put in the layer slot, so any `chkbox >= wcnz` writes out of bounds (memory corruption) and the wrong cell gets the displaced fish. Function: `Ecology_Total_Verts_And_Migration()` (flagenviro_displace branch).

**Current code** (`atecology/atmovement.c` lines 2031–2035)

```c
 2031:                                             check_done = 1;
 2032:                                             den_diff = 0;
 2033:                                         }
 2034:                                     }
 2035:                                     adbox++;
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
**Lines (current code):** 2052-2053  
**Lines at review (Oct 2026):** 2049-2050

**Function:** `Ecology_Total_Verts_And_Migration()`

**Why it's thought to be a bug**

`bm->boxes[ij].tr[k][DL_id] = VERTinfo[..SN_id] * totden * den_diff;` — this sets the labile/refractory detritus tracers to the killed biomass instead of adding to it, wiping out the detritus already in the cell. It also runs once per cohort, so each cohort overwrites the one before. The value is also mg N for the whole cell (per-individual N × numbers × proportion) written into a mg N m-3 tracer with no divide by volume. Function: `Ecology_Total_Verts_And_Migration()` (flagenviro_kill).

**Current code** (`atecology/atmovement.c` lines 2050–2055)

```c
 2050:                                 bm->boxes[ij].tr[k][DR_id] = VERTinfo[sp][n][RN_id] * totden[sp][n] * den_diff;
 2051:                             }
 2052:                             
 2053:                             
 2054:                             if(bm->track_contaminants && bm->flag_contamMaternalTransfer) {
 2055:                                 if((flagmother > 0) && (newden[sp][n][k][ij] > 0)) {
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
**Lines (current code):** 1559  
**Lines at review (Oct 2026):** 1552

**Function:** `Ecology_Total_Verts_And_Migration()`

**Why it's thought to be a bug**

`(-1.0 * bm->boxes[bm->current_box].botz) <= ...[maxtotdepth_id]` — this is inside the `ij` box loop, but it uses `bm->current_box`, which is old state left over from the process loop. The two checks next to it use `bm->boxes[ij].botz`, and atbiolsetup.c:3518 does the same max-total-depth test with `ij`. So the depth exclusion for forage-dependent movement uses the wrong box. Function: `Ecology_Total_Verts_And_Migration()`.

**Current code** (`atecology/atmovement.c` lines 1557–1561)

```c
 1557:                                 		( bm->boxes[ij].botz <= (-1.0 * FunctGroupArray[sp].speciesParams[mindepth_id]) &&
 1558:                                 		 bm->boxes[ij].botz >= (-1.0 * FunctGroupArray[sp].speciesParams[maxdepth_id]) &&
 1559:                                         ((-1.0 * bm->boxes[bm->current_box].botz) <= FunctGroupArray[sp].speciesParams[maxtotdepth_id]))) {
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
**Lines (current code):** 3000, 3076-3078  
**Lines at review (Oct 2026):** 2997, 3073-3075

**Function:** `Ecology_Invert_Migration()`

**Why it's thought to be a bug**

`int qrt, next_qrt, this_HowFar, ...` — this_HowFar is an int but gets `bm->HowFar` (a double fraction in [0,1), attime.c:209) or `FunctGroupArray[sp].HowFar`. It truncates to 0, so prescribed invertebrate movement (line 3385) never blends between seasons and jumps when the season changes. The vertebrate routine declares it `double` (line 867). Also, `FunctGroupArray[].HowFar` is itself declared `int` (atlantisUtil/include/atFunctGroup.h:94) but is assigned a fractional remainder (attime.c:524), so it is always 0 for vertebrates too whenever numMoveEntries != 4. Function: `Ecology_Invert_Migration()`.

**Current code** (`atecology/atmovement.c` lines 2998–3002)

```c
 2998:     int qrt, next_qrt, this_HowFar, agec, sp_Migrate_Years;
 2999: 	//int sp_debug;
 3000:     // do_debug;
 3001: 	int enviro_depend = 0;
 3002: 	double some_ice, midpoint;
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
**Lines (current code):** 3385  
**Lines at review (Oct 2026):** 3382

**Function:** `Ecology_Invert_Migration()`

**Why it's thought to be a bug**

`FunctGroupArray[sp].distrib[ij][next_qrt][cohort]` — distrib is allocated `Util_Alloc_Init_3D_Double(2, numMoveEntries, nbox)` => `[nbox][entries][2]`, indexed by stage (atUtilGroupIO.c:917; the vertebrate code uses `[stage]`, e.g. line 1935). Using `cohort` (0..numCohortsXnumGenes-1) reads out of bounds for an invert with more than 2 cohort×gene entries, and reads the wrong stage when genes > 1. Function: `Ecology_Invert_Migration()`.

**Current code** (`atecology/atmovement.c` lines 3383–3387)

```c
 3383:                             /*
 3384:                             if(sp_debug == 2){
 3385:                                 fprintf(llogfp,"day: %e, %s-newden[%d][%d]: %e, HowFar: %e, distrib%s-%d-%d-%d: %e, distrib%s-%d-%d-%d: %e\n",
 3386:                                     bm->dayt, FunctGroupArray[sp].groupCode, cohort, ij, newden[sp][cohort][0][ij], this_HowFar, FunctGroupArray[sp].groupCode, ij, next_qrt, cohort, FunctGroupArray[sp].distrib[ij][next_qrt][cohort], FunctGroupArray[sp].groupCode, ij, qrt, cohort, FunctGroupArray[sp].distrib[ij][qrt][cohort]);
 3387:                             }
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
**Lines (current code):** 3627, 3630, 3640  
**Lines at review (Oct 2026):** 3624, 3627, 3637

**Function:** `Ecology_Invert_Migration()`

**Why it's thought to be a bug**

`amt * tempdistrib[k][cohort]` — tempdistrib is `[wcnz][K_num_max_stages]` (atbiolsetup.c:2894), and `Get_Vertical_Distribution()` only fills `[k][juv_id]` and `[k][stage]` (tracker_id = stage, line 4157). Line 3623 correctly reads `tempdistrib[k][stage]`. Indexing by cohort goes out of bounds or reads unset columns when cohorts×genes > stages. Function: `Ecology_Invert_Migration()`.

**Current code** (`atecology/atmovement.c` lines 3625–3642)

```c
 3625:                                 if (!(_finite(bm->boxes[ij].tr[k][pid] ))) {
 3626:                                     printf("amt = %e,  tempdistrib[k][0] = %e, bm->boxes[ij].dz[k] = %e, bm->boxes[ij].area = %e \n",
 3627:                                             amt,  tempdistrib[k][cohort], bm->boxes[ij].dz[k], bm->boxes[ij].area);
 3628:                                     quit("Ecology_Invert_Migration - day: %e, box: %d, layer: %d %s set to %e.\n",
 3629:                                             bm->dayt, bm->current_box, bm->current_layer, FunctGroupArray[sp].groupCode,
 3630:                                             bm->boxes[ij].tr[k][pid] );
 3631:                                 }
 3632: 
 3633:                                 /**
 3634:                                 if (sp_debug == 2) {
 3635:                                     fprintf(llogfp, "Time: %e, sp %s in box: %d layer: %d, ends redistrib with %.20e (amt: %.20e tempdistrib: %.20e)\n",
 3636: 											bm->dayt, FunctGroupArray[sp].groupCode, ij, k, bm->boxes[ij].tr[k][pid], amt, tempdistrib[k][cohort]);
 3637:                                 }
 3638:                                 **/
 3639: 
 3640:                             }
 3641:                         }
 3642:                     }
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
**Lines (current code):** 3501  
**Lines at review (Oct 2026):** 3498

**Function:** `Ecology_Invert_Migration()`

**Why it's thought to be a bug**

`MIGRATION[sp].DEN[cohort][qid] += migbiom * totboxden[invert_biosum];` — the total biomass for this cohort is stored at `totboxden[invert_biosum+cohort]` (lines 3277, 3336, 3553, 3570). Cohort 0's biomass is used for every cohort, so migrant biomass for cohorts > 0 is wrong. Function: `Ecology_Invert_Migration()`.

**Current code** (`atecology/atmovement.c` lines 3499–3503)

```c
 3499:                                     MIGRATION[sp].num_stagger[qid] = 0;
 3500:                                     if(FunctGroupArray[sp].multiyr_mig) {
 3501:                                         sp_Migrate_Years = (int) (MIGRATION[sp].MinYearsAway[qid]);
 3502:                                         if ((sp_Migrate_Years > 1) && (stagger_return)) {
 3503:                                             MIGRATION[sp].num_stagger[qid] = MIGRATION[sp].Stagger[qid][stagger_years_id];
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
**Lines (current code):** 3264, 3323, 3541, 3558, 823  
**Lines at review (Oct 2026):** 3274, 3333, 3550, 3567, 818

**Why it's thought to be a bug**

`totboxden[invert_biosum+cohort]` with invert_biosum = 1 (atlantisboxmodel.h:813) reaches index numCohortsXnumGenes. totboxden is allocated with only `K_num_max_cohort*K_num_max_genetypes` (atbiolsetup.c:2918). atbiolsetup.c:2901-2903 computes `totdensize = K_max_invert_cohorts+1` "as need an additional slot for propsum in invertebrate movement code", but applies it to totden/totden_check, not totboxden. So it overruns when an invert has the maximum cohort×gene count. Functions: `Ecology_Invert_Migration()`, `Store_Min_Max_Avg()`.

**Current code** (`atecology/atmovement.c` lines 3262–3266)

```c
 3262:                 /*
 3263:                  if (sp_debug == 2) {
 3264:                     fprintf(llogfp, "Time: %e %s:%d total starting amt: %.20e ", bm->dayt, FunctGroupArray[sp].groupCode, cohort, totamt);
 3265: 						fprintf(llogfp, "spmigrate = %d ", spmigrate);
 3266: 						fprintf(llogfp, "sp_ddepend_move = %d ", sp_ddepend_move);
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
**Lines (current code):** 3104-3109, 3379-3382, 3400  
**Lines at review (Oct 2026):** 3101-3106, 3376-3379, 3397

**Why it's thought to be a bug**

`if (habitatType == EPIFAUNA) maxk = 0; ... for (k = 0; k < maxk; k++)` — with maxk = 0 the loop never runs, so epifauna skip all temperature, salinity, O2, ice, light, noise and contaminant limits, both for non-mobile groups (3095 block) and for prescribed movement. The epifauna `epi[pid]` branches inside those loops can never run.

**Current code** (`atecology/atmovement.c` lines 3102–3111)

```c
 3102:                     if (FunctGroupArray[fgIndex].habitatType == EPIFAUNA)
 3103:                         maxk = 0;
 3104:                     else
 3105:                         maxk = bm->boxes[ij].nz;
 3106:                     
 3107:                     for (k = 0; k < maxk; k++) {
 3108:                         /* Noise and light pollution avoidance */
 3109:                         if (FunctGroupArray[sp].isLightEffected) {
 3110:                             current_enviro = bm->boxes[ij].tr[k][Light_Pollution_i];
 3111:                             if (current_enviro > 0) {
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
**Lines (current code):** 3036-3039  
**Lines at review (Oct 2026):** 3033-3036

**Function:** `Ecology_Invert_Migration()`

**Why it's thought to be a bug**

`if (bm->flagday) day_part = 0; else day_part = 1;` — `distrib_VERTICAL[0]` is "night" and `[1]` is "day" (atBiologyXMLParamIO.c:93-94), and flagday = 1 means daytime (attime.c Time_ODay; comment at line 906). The vertebrate routine passes `day_part = bm->flagday`. Invertebrates therefore use the night profile in the day and the day profile at night (diel migration reversed), unless this reversal is a deliberate legacy choice. Function: `Ecology_Invert_Migration()`.

**Current code** (`atecology/atmovement.c` lines 3034–3041)

```c
 3034: 	if (bm->flagday)
 3035: 		day_part = 0;
 3036: 	else
 3037: 		day_part = 1;
 3038: 
 3039: 	/* Initialise local arrays */
 3040: 	for (sp = 0; sp < bm->K_num_tot_sp; sp++) {
 3041: 		if (FunctGroupArray[sp].isVertebrate == FALSE && FunctGroupArray[sp].isImpacted == TRUE) {
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
**Lines (current code):** 4070-4075  
**Lines at review (Oct 2026):** 4067-4072

**Function:** `Larval_Dispersal()`

**Why it's thought to be a bug**

`else year = bm->larvalYears;` — connectivitymatrix only holds data for years `0..larvalYears-1` (read with count `min(larvalYears, numRunYears)`, atbiolsetup.c:3955). Past the end of the data with larval_rewind off, the code indexes a slot that was never filled (all zeros), so larvaldistrib = 0 and there is no recruitment. Function: `Larval_Dispersal()`.

**Current code** (`atecology/atmovement.c` lines 4068–4077)

```c
 4068: 			year = 0;
 4069: 		else
 4070: 			year = bm->larvalYears;
 4071: 
 4072: 	}
 4073: 
 4074: 	//fprintf(llogfp, "updated - year = %d\n", year);
 4075: 
 4076: 	// Do the matrix multiplication of connectivity matrx * adults
 4077:     if (!bm->larvae_connect_only) {
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
**Lines (current code):** 4011-4045  
**Lines at review (Oct 2026):** 4008-4042

**Function:** `Invade_Spread()`

**Why it's thought to be a bug**

Each check overwrites `go_there` (temperature fail -> 0, then salinity OK -> 1, then O2 OK -> 1). Only the last active criterion counts. Light and noise set `go_there = ceil(coefft/enviro)`, which can override earlier failures. Invaders can spread into cells that fail temperature or salinity limits. Function: `Invade_Spread()`.

**Current code** (`atecology/atmovement.c` lines 4009–4047)

```c
 4009:             go_there = 0;
 4010:         else
 4011:             go_there = 1;
 4012:     }
 4013:     
 4014:     if (salt_sensitive_sp) {
 4015:         if ( (min_salt_sp > there_salt ) || (max_salt_sp < there_salt ))
 4016:             go_there = 0;
 4017:         else
 4018:             go_there = 1;
 4019:     }
 4020: 
 4021:     if (bm->flagO2depend) {
 4022:         if (there_o2 > min_O2_sp )
 4023:             go_there = 1;
 4024:         else
 4025:             go_there = 0;
 4026:     }
 4027:     
 4028:     /* Noise and light pollution avoidance */
 4029:     if (FunctGroupArray[sp].isLightEffected) {
 4030:         current_enviro = bm->boxes[nb].tr[k][Light_Pollution_i];
 4031:         if (current_enviro > 0) {
 4032:             go_there = ceil(FunctGroupArray[sp].speciesParams[light_coefft_id] / (current_enviro + small_num));
 4033:         }
 4034:     }
 4035:     
 4036:     if (FunctGroupArray[sp].isNoiseEffected) {
 4037:         current_enviro = bm->boxes[nb].tr[k][Noise_Pollution_i];
 4038:         if (current_enviro > 0) {
 4039:             go_there = ceil(FunctGroupArray[sp].speciesParams[noise_coefft_id] / (current_enviro + small_num));
 4040:         }
 4041:     }
 4042:     
 4043:     
 4044:     return go_there;
 4045:     
 4046: }
 4047: 
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
**Lines (current code):** 3803  
**Lines at review (Oct 2026):** 3800

**Function:** `Ecology_Invading_Species()`

**Why it's thought to be a bug**

`bm->boxes[b].tr[k][bm->InvaderPID] *= (1.0 + InvadeScalar);` — in the `conditional_invader` case, InvadeScalar is never set (it is only computed in `simple_invader`), so it stays 0.0. Invertebrate entrants are never scaled, while the vertebrate branch two lines above uses `bm->InvaderScalar`. Function: `Ecology_Invading_Species()`.

**Current code** (`atecology/atmovement.c` lines 3801–3805)

```c
 3801:                 }
 3802:             }
 3803:             
 3804:             /* Now spread to nearby boxes with correct conditions */
 3805:             tot_scalar = 0.0;
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
**Lines (current code):** 3920-3922, 3929, 3932  
**Lines at review (Oct 2026):** 3929-3931, 3938, 3941

**Function:** `Ecology_Invading_Species()`

**Why it's thought to be a bug**

`bk = k; if(bk >= bm->boxes[nb].nz) bk = bm->boxes[nb].nz - 1;` — k loops `< boxes[nb].nz`, so this test can never be true. It was clearly meant to clamp against the source box `b`, since `boxes[b].tr[bk]` is read next. When nb is deeper than b, unused layers of b are read. Function: `Ecology_Invading_Species()`.

**Current code** (`atecology/atmovement.c` lines 3918–3934)

```c
 3918:             	for (nb = 0; nb < bm->nbox; nb++) {
 3919:                 	for (k = 0; k < bm->boxes[nb].nz; k++) {
 3920:                 		bm->invader_spread_scalars[nb][k] /= tot_scalar;
 3921:                 	}
 3922:                 }
 3923: 
 3924:                 // Now do the invader dispersal
 3925:                 for (chkbox = 0; chkbox < bm->boxes[b].nUniqueAdj; chkbox++) {
 3926:                     nb = bm->boxes[b].uniqueAdjBoxes[chkbox];
 3927:                     for (k = 0; k < bm->boxes[nb].nz; k++) {
 3928:                         bk = k;
 3929:                         if(bk >= bm->boxes[nb].nz)
 3930:                             bk = bm->boxes[nb].nz - 1;
 3931:                         this_scalar = bm->invader_spread_scalars[nb][k];
 3932: 
 3933:                         // Proportion of numbers or biomass in local box b dispersing to destinaiton box nb
 3934:                         if (FunctGroupArray[bm->InvaderIndex].isVertebrate == TRUE) {
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
**Lines (current code):** 3813-3816, 3823-3913, 3921-3926  
**Lines at review (Oct 2026):** 3810-3813, 3820-3910, 3918-3923

**Function:** `Ecology_Invading_Species()`

**Why it's thought to be a bug**

The local box's scalar is written to every layer but added to `tot_scalar` only once. Neighbour scalars are added to `tot_scalar` once per invader cohort n (loop at 3820) but stored only once per [nb][k], with the last n winning. Each source layer then keeps `s_b/tot` and sends `s_nb,k/tot` to each neighbour layer, so the fractions for a layer do not add to 1 and invader biomass/numbers are not conserved. Function: `Ecology_Invading_Species()`.

**Current code** (`atecology/atmovement.c` lines 3811–3850 (truncated))

```c
 3811:             }
 3812:             tot_scalar += this_scalar;
 3813:             
 3814:             // Now do neighbouring boxes
 3815:             for (chkbox = 0; chkbox < bm->boxes[b].nUniqueAdj; chkbox++) {
 3816:                 nb = bm->boxes[b].uniqueAdjBoxes[chkbox];
 3817:                 this_depth = bm->boxes[nb].botz;
 3818:                 if ((this_depth <= (-1.0 * FunctGroupArray[bm->InvaderIndex].speciesParams[mindepth_id])) && (this_depth >= (-1.0 * bm->InvaderMaxDepth))) {
 3819:                     for (n = bm->minInvaderAge; n < bm->maxInvaderAge; n++) {
 3820:                         chrtstage = FunctGroupArray[bm->InvaderIndex].cohort_stage[n];
 3821: 
 3822:                         // Depth ok so check habitat
 3823:                         sp_like = 0;
 3824:                         if (FunctGroupArray[bm->InvaderIndex].isHabDepend > 0){
 3825:                             // Check geological habitats
 3826:                             sp_likeREEF = bm->HABITATlike[bm->InvaderIndex][chrtstage][bm->REEFcover_id] * bm->boxes[nb].reef;
 3827:                             sp_likeFLAT = bm->HABITATlike[bm->InvaderIndex][chrtstage][bm->FLATcover_id] * bm->boxes[nb].flat;
 3828:                             sp_likeSOFT = bm->HABITATlike[bm->InvaderIndex][chrtstage][bm->SOFTcover_id] * bm->boxes[nb].soft;
 3829:                             sp_likeCANYON = bm->HABITATlike[bm->InvaderIndex][chrtstage][bm->CANYONcover_id];
 3830:                             if ((sp_likeREEF > 0.0) || (sp_likeFLAT > 0.0) || (sp_likeSOFT > 0.0) || (sp_likeCANYON > 0.0))
 3831:                                 sp_like = 1;
 3832:                             
 3833:                             // If that not enough check biogenic habitats
 3834:                             if(!sp_like) {
 3835:                                 for(habsp = 0; habsp < bm->K_num_tot_sp; habsp++){
 3836:                                     if (FunctGroupArray[habsp].isCover == TRUE) {
 3837:                                         sp_like = bm->HABITATlike[bm->InvaderIndex][chrtstage][FunctGroupArray[habsp].coverID] * bm->coveramt[FunctGroupArray[habsp].coverID][nb];
 3838:                                         if (sp_like)
 3839:                                             break;
 3840:                                     }
 3841:                                 }
 3842:                             }
 3843:                         } else {
 3844:                             sp_like = 1;
 3845:                         }
 3846:                         
 3847:                         if (sp_like) {  // Habitat found in potential new location
 3848:                             for (k = 0; k < bm->boxes[nb].nz; k++) {
 3849:                                 // Then check the environment
 3850:                                 go_there = Invade_Spread(bm, bm->InvaderIndex, llogfp, b, nb, k, &dist);
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
**Lines (current code):** 3833  
**Lines at review (Oct 2026):** 3830

**Function:** `Ecology_Invading_Species()`

**Why it's thought to be a bug**

`sp_likeCANYON = HABITATlike[..][CANYONcover_id];` is the only habitat term not multiplied by the box's area fraction. REEF, FLAT and SOFT are multiplied by `boxes[nb].reef/flat/soft`, and `double canyon` exists in the box struct (atlantisboxmodel.h:3878). Any canyon-liking invader therefore treats every box as suitable habitat. Function: `Ecology_Invading_Species()`.

**Current code** (`atecology/atmovement.c` lines 3831–3835) — ⚠ the code quoted in the review is not at these lines any more (changed, or already fixed?)

```c
 3831:                                 sp_like = 1;
 3832:                             
 3833:                             // If that not enough check biogenic habitats
 3834:                             if(!sp_like) {
 3835:                                 for(habsp = 0; habsp < bm->K_num_tot_sp; habsp++){
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
**Lines (current code):** 1620-1625  
**Lines at review (Oct 2026):** 1613-1618

**Function:** `Ecology_Total_Verts_And_Migration()`

**Why it's thought to be a bug**

Sticky ddepend with flagmother: `if ((roc[ij][age_mat] / roc_wgt) > ...)` runs inside the `n` loop while `n < age_mat`, so roc[ij][age_mat] for this species and timestep has not been computed yet. It holds the value from the previous species or timestep. (The same test at 1864 runs after all roc are computed, so it is fine.) Function: `Ecology_Total_Verts_And_Migration()`.

**Current code** (`atecology/atmovement.c` lines 1618–1627)

```c
 1618: 							}
 1619: 
 1620: 							if (sp_ddepend_move == sticky_ddepend) {
 1621: 								/* If using 'sticky' density dependent movement determine what proportion has to move
 1622: 								 then move them. Use average sized individual for comparison purposes */
 1623: 								for (k = 0; k < bm->boxes[ij].nz; k++) {
 1624: 									if ((flagmother > 0) && (n < age_mat)) {
 1625: 										if ((roc[ij][age_mat] / roc_wgt) > (k_roc_food * VERTinfo[sp][n][SN_id])) {
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
**Lines (current code):** 472-492  
**Lines at review (Oct 2026):** 472-492

**Function:** `Get_Vert_Diet()`

**Why it's thought to be a bug**

`if(!prey_counted[prey][n])` is checked outside the `predstage` loop. Inside it, every predator stage whose gape window fits adds `(sn+rn)*den` to preyamt again, so prey are counted twice when a predator has 2 stages that both match. This always happens under flag_olddiet, where preyavail is the same for both stages. Function: `Get_Vert_Diet()`.

**Current code** (`atecology/atmovement.c` lines 470–494)

```c
  470: 			if (FunctGroupArray[pred].speciesParams[flag_id] == TRUE) {
  471: 				if(!prey_counted[prey][n]){
  472: 					for(predstage = 0; predstage < FunctGroupArray[pred].numStages; predstage++){
  473: 						if(bm->flag_olddiet){
  474: 							preyavail = FunctGroupArray[pred].pSPEat[juv_id][prey][habitat];  // As only tested for the juvenile case originally - dumb decision!
  475: 						} else
  476: 							preyavail = bm->pSPVERTeat[pred][prey][predstage][preystage];
  477: 
  478: 						if (preyavail) {  // So if any predator can get the prey in the mouth store the value
  479: 
  480: 							if (FunctGroupArray[pred].groupAgeType == AGE_STRUCTURED) {
  481: 								spmin = FunctGroupArray[pred].spMinMax[(predstage * 2)];
  482: 								spmax = FunctGroupArray[pred].spMinMax[(predstage * 2 + 1)];
  483: 								upper = FunctGroupArray[pred].speciesParams[KUP_id] * boxLayerTracers[spmax];
  484: 								lower = FunctGroupArray[pred].speciesParams[KLP_id] * boxLayerTracers[spmin];
  485: 							} else {
  486: 								upper = FunctGroupArray[pred].speciesParams[KUP_id] * FunctGroupArray[pred].sn[predstage];
  487: 								lower = FunctGroupArray[pred].speciesParams[KLP_id] * FunctGroupArray[pred].sn[predstage];
  488: 							}
  489: 							if ((bm->boxes[ij].tr[k][sn] >= lower) && (boxLayerTracers[sn] <= upper)) {
  490: 								preyamt[prey][ij][preystage] += (boxLayerTracers[sn] + boxLayerTracers[rn]) * boxLayerTracers[den];
  491: 								prey_counted[prey][n] = 1;
  492: 							}
  493: 
  494: 							/**
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
**Lines (current code):** 1169-1170, 1234-1246, 1249-1256  
**Lines at review (Oct 2026):** 1159-1160, 1231-1243, 1246-1253

**Function:** `Ecology_Total_Verts_And_Migration()`

**Why it's thought to be a bug**

`DEN` is clamped to 0 at 1159, so `if (DEN < 0.0)` at 1231 can never be true, and the SN/RN reset under flagavgmig never happens. The 1246 test `dayt > Return_Now+Return_Period` can never be true inside the enclosing condition at 1120 (`dayt < Return_Now+Return_Period`), so `Checked[qid]` is never set here. Function: `Ecology_Total_Verts_And_Migration()`.

> 🔀 **Changed by the merge of main (Nov 2026):** code shown above was changed by the merge. Check whether this finding is now fixed before acting.

**Current code** (`atecology/atmovement.c` lines 1167–1172) — ⚠ the code quoted in the review is not at these lines any more (changed, or already fixed?)

```c
 1167:                         MIGRATION[sp].DEN[n][qid] -= mignum;
 1168: 
 1169:                         if(MIGRATION[sp].DEN[n][qid] < 0)
 1170:                             MIGRATION[sp].DEN[n][qid] = 0.0;
 1171: 
 1172:                         oldden = totden[sp][n];
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
**Lines (current code):** 1273-1281  
**Lines at review (Oct 2026):** 1270-1278

**Function:** `Ecology_Total_Verts_And_Migration()`

**Why it's thought to be a bug**

The larval_queue_extension reset happens inside the `n` (gene) and `qid` loops. After the first gene with `stock_id == numStocks-1` resets it to 0, later genes skip their pre-model larvae, which are lost. If `stock_id < 0` it is set to 0 (line 1191), so the reset only fires when numStocks == 1. Function: `Ecology_Total_Verts_And_Migration()`.

**Current code** (`atecology/atmovement.c` lines 1271–1283)

```c
 1271:                           stock_id = 0; 
 1272:                         }
 1273:                         
 1274:                         if ((n < FunctGroupArray[sp].numGeneTypes) && FunctGroupArray[sp].speciesParams[recruit_outside_id]) {
 1275:                             // First start with any that might be coming from before model started
 1276:                             yoy_den = 0.0;
 1277:                             if(FunctGroupArray[sp].speciesParams[larval_queue_extension_id] > 0) {
 1278:                                 for (lid = 0; lid < FunctGroupArray[sp].speciesParams[larval_queue_extension_id]; lid++) {
 1279:                                     yoy_den += EMBRYO[sp].Larvae[stock_id][n][lid];
 1280:                                     EMBRYO[sp].Larvae[stock_id][n][lid] = 0;
 1281:                                 }
 1282:                                 // Reset larval_queue_extension so not called in subsequent years
 1283:                                 if (stock_id == (FunctGroupArray[sp].numStocks - 1)) {
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
**Lines (current code):** 1399  
**Lines at review (Oct 2026):** 1392

**Function:** `Ecology_Total_Verts_And_Migration()`

**Why it's thought to be a bug**

`sp_amt += totden[sp][n];` sits inside the stock loop `ij < maxij`, so the total is multiplied by K_num_stocks_per_sp. This loosens the "less than one individual present" skip at 1404. Function: `Ecology_Total_Verts_And_Migration()`.

**Current code** (`atecology/atmovement.c` lines 1397–1401)

```c
 1397: 				for (n = 0; n < FunctGroupArray[sp].numCohortsXnumGenes; n++) {
 1398: 					bm->stock_struct_prop[sp][n][ij] = init_stock_struct_prop[sp][n][ij] / (totden[sp][n] + small_num);
 1399: 					sp_amt += totden[sp][n];
 1400: 
 1401:                     /**
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
**Lines (current code):** 1480, 4375/4409  
**Lines at review (Oct 2026):** 1473, 4384/4416

**Why it's thought to be a bug**

`contract_sp = (int)FunctGroupArray[sp].speciesParams[contract_tol_id];` (and `int contract_sp` in `Check_Conditions()`) truncates the pH tolerance contraction, e.g. 0.5 -> 0. The invertebrate routine (3194) correctly keeps it as a double. Functions: `Ecology_Total_Verts_And_Migration()`, `Check_Conditions()`.

**Current code** (`atecology/atmovement.c` lines 1478–1482)

```c
 1478: 			pH_scale = (FunctGroupArray[sp].pHcorr - 1.0);
 1479: 			if( flagcontract_sp  && (pH_scale > 0.0)) {
 1480: 				contract_sp = (int)FunctGroupArray[sp].speciesParams[contract_tol_id];
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
**Lines (current code):** 2373-2376  
**Lines at review (Oct 2026):** 2383-2386

**Function:** `Ecology_Total_Verts_And_Migration()`

**Why it's thought to be a bug**

`if(!totad[ngene]) recruit_hdistrib = distrib[..][adstage];` is inside the box/layer loop, while totad accumulates across boxes. Boxes processed before the first adult is found get the prescribed distribution; later empty boxes get 0. The fallback is also not added to totad, so the later normalisation is off. Function: `Ecology_Total_Verts_And_Migration()`.

**Current code** (`atecology/atmovement.c` lines 2371–2378)

```c
 2371:                                 
 2372: 
 2373:                                 if( update_larval_distrib ) {
 2374:                                     for(bcohort = age_mat; bcohort < FunctGroupArray[sp].numCohorts; bcohort++){
 2375:                                         n = (FunctGroupArray[sp].numGeneTypes * bcohort) + ngene;
 2376: 									
 2377:                                         /* Sum over current distribution of adult age groups */
 2378:                                         bm->recruit_hdistrib[ngene][ij][sp] += newden[sp][n][k][ij] * temp_effect * salt_effect * o2_effect * ice_effect;
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
**Lines (current code):** 2368-2369, 2380-2381  
**Lines at review (Oct 2026):** 2378-2379, 2390-2391

**Function:** `Ecology_Total_Verts_And_Migration()`

**Why it's thought to be a bug**

The `recruit_hdistrib` numerator leaves out `noise_effect * light_effect`, but the `totad` denominator includes them. After normalisation (2399) the distribution does not sum to 1, and light/noise reduce the denominator instead of steering larvae away. Function: `Ecology_Total_Verts_And_Migration()`.

**Current code** (`atecology/atmovement.c` lines 2366–2383)

```c
 2366:                                     current_enviro = bm->boxes[ij].tr[k][Noise_Pollution_i];
 2367:                                     if (current_enviro > 0) {
 2368:                                         noise_effect = (FunctGroupArray[sp].speciesParams[noise_coefft_id] / (current_enviro + small_num));
 2369:                                     }
 2370:                                 }
 2371:                                 
 2372: 
 2373:                                 if( update_larval_distrib ) {
 2374:                                     for(bcohort = age_mat; bcohort < FunctGroupArray[sp].numCohorts; bcohort++){
 2375:                                         n = (FunctGroupArray[sp].numGeneTypes * bcohort) + ngene;
 2376: 									
 2377:                                         /* Sum over current distribution of adult age groups */
 2378:                                         bm->recruit_hdistrib[ngene][ij][sp] += newden[sp][n][k][ij] * temp_effect * salt_effect * o2_effect * ice_effect;
 2379:                                         totad[ngene] += newden[sp][n][k][ij] * temp_effect * salt_effect * o2_effect * ice_effect * noise_effect * light_effect;
 2380:                                         
 2381:                                     }
 2382:                                     
 2383:                                     if(!totad[ngene]) {  // If no adults about set back to the prescribed distribution
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
**Lines (current code):** 2059, 2063-2100  
**Lines at review (Oct 2026):** 2056, 2060-2097

**Function:** `Ecology_Total_Verts_And_Migration()`

**Why it's thought to be a bug**

`LocalPopCount[n]` is only updated when `newden > 0`, so cells with no animals keep the previous cell's value and trigger suckling transfers that should not happen. `suckler_contam_today_id = 1` (2100) is reset inside the per-cell (ij,k) loop, so the "skip on birth day" set in atdemography.c:473 only applies to the first cell. Function: `Ecology_Total_Verts_And_Migration()`.

**Current code** (`atecology/atmovement.c` lines 2057–2096 (truncated))

```c
 2057:                                 }
 2058:                             }
 2059: 						}
 2060:                         if( bm->track_contaminants && bm->flag_contamMaternalTransfer && (flagmother > 0) && (FunctGroupArray[sp].speciesParams[suckler_contam_today_id] > 0)) {
 2061:                             for (n = 0; n < age_mat; n++) {
 2062:                                 juv_num = FunctGroupArray[sp].LocalPopCount[n];
 2063:                                 not_done = 1;
 2064:                                 if(juv_num > bm->min_dens){ // Only do if juveniles present
 2065:                                     ad_start_cohort = (int)(floor(drandom(start_n, end_n))); // Find random starting point so not always the youngest adutlts feeding the young
 2066:                                     for (ad_cohort = ad_start_cohort; ad_cohort < FunctGroupArray[sp].numCohortsXnumGenes; ad_cohort++) { // Iterate up age classes
 2067:                                         ad_num = FunctGroupArray[sp].LocalPopCount[ad_cohort];
 2068:                                         if(ad_num > bm->min_dens) {
 2069:                                             Get_Suckling_Contaminants(bm, sp, ad_cohort, n);
 2070:                                             juv_num -= ad_num;
 2071:                                         }
 2072:                                         if(juv_num > 0) {
 2073:                                             // Do nothing as not finished yet
 2074:                                         } else {
 2075:                                             not_done = 0;
 2076:                                             break;
 2077:                                         }
 2078:                                     }
 2079:                                     if(not_done > 0){
 2080:                                         for (ad_cohort = ad_start_cohort; ad_cohort > (age_mat - 1); ad_cohort--) { // Iterate over the rest of the adults if we have to
 2081:                                             ad_num = FunctGroupArray[sp].LocalPopCount[ad_cohort];
 2082:                                             if(ad_num > bm->min_dens) {
 2083:                                                 Get_Suckling_Contaminants(bm, sp, ad_cohort, n);
 2084:                                                 juv_num -= ad_num;
 2085:                                             }
 2086:                                             if(juv_num > 0) {
 2087:                                                 // Do nothing as not finished yet
 2088:                                             } else {
 2089:                                                 not_done = 0;
 2090:                                                 break;
 2091:                                             }
 2092:                                         }
 2093:                                     }
 2094:                                 }
 2095:                             }
 2096:                         }
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
**Lines (current code):** 1976-1988 vs 4309-4323  
**Lines at review (Oct 2026):** 1973-1985 vs 4306-4320

**Why it's thought to be a bug**

When enviro_depend is set, light/noise avoidance is applied to `tempdistrib` in `Get_Vertical_Distribution()` (which feeds `vertdistrib` into newden) and then applied again to newden at 1973-1985. Inverts get the same double effect (3402-3414).

**Current code** (`atecology/atmovement.c` lines 1974–1990) — ⚠ the code quoted in the review is not at these lines any more (changed, or already fixed?)

```c
 1974:                                 current_enviro = bm->boxes[ij].tr[k][Light_Pollution_i];
 1975:                                 if (current_enviro > 0) {
 1976:                                     newden[sp][n][k][ij] *= (FunctGroupArray[sp].speciesParams[light_coefft_id] / (current_enviro + small_num));
 1977:                                 }
 1978:                             }
 1979:                             
 1980:                             if (FunctGroupArray[sp].isNoiseEffected) {
 1981:                                 current_enviro = bm->boxes[ij].tr[k][Noise_Pollution_i];
 1982:                                 if (current_enviro > 0) {
 1983:                                     newden[sp][n][k][ij] *= (FunctGroupArray[sp].speciesParams[noise_coefft_id] / (current_enviro + small_num));
 1984:                                 }
 1985:                             }
 1986: 
 1987: 							/**
 1988:                             if(sp ==64) {
 1989:                                 fprintf(llogfp,"Time %e %s-%d box%d-%d before enviro_depend newden: %e\n",
 1990:                                         bm->dayt, FunctGroupArray[sp].groupCode, n, ij, k, newden[sp][n][k][ij]);
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
**Lines (current code):** 2779  
**Lines at review (Oct 2026):** 2789

**Function:** `Ecology_Total_Verts_And_Migration()`

**Why it's thought to be a bug**

`quit("Time: %e box%d-%d %s-%d ...", bm->dayt, groupCode, n, ij, k)` — the arguments are in the wrong order (`%d` gets a char*, `%s` gets an int), so the error path will likely segfault instead of printing. Function: `Ecology_Total_Verts_And_Migration()`.

**Current code** (`atecology/atmovement.c` lines 2777–2781)

```c
 2777:                                 fprintf(llogfp, "ERROR Time: %e %s-%d box%d-%d this_tot_biom: %.20e, newden_sum-%d: %.20e, den: %.20e, totden: %.20e, newden: %.20e, SN-%d: %.20e, RN-%d: %.20e (vs SNinit %.20e, RNinit %.20e)\n", bm->dayt,
 2778:                                     FunctGroupArray[sp].groupCode, n, ij, k, this_tot_biom, n,
 2779:                                     newden_sum[n], bm->boxes[ij].tr[k][den], totden[sp][n],
 2780:                                     newden[sp][n][k][ij], n, bm->boxes[ij].tr[k][sn], n,
 2781:                                     bm->boxes[ij].tr[k][rn], VERTinfo[sp][n][SN_id], VERTinfo[sp][n][RN_id]);
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
**Lines (current code):** 2675, 2652, 2514/2518/2772/2776, 3633  
**Lines at review (Oct 2026):** 2672, 2649, 2511/2515/2769/2773, 3630

**Why it's thought to be a bug**

2672 prints `period = %e` with the int `Leave_Period[qid]` (UB). 2649 prints `Box %d;%d` with `ij, ij` (should be ij, k). The negative-size diagnostics and the invert quit print `bm->current_box, bm->current_layer` instead of `ij, k`.

**Current code** (`atecology/atmovement.c` lines 2673–2677)

```c
 2673: 
 2674: 												fprintf(stderr, "MigrateAway[%d][%d][%d] = %d\n", sp, stage, qid, MIGRATION[sp].Leave_Now[qid]);
 2675: 												fprintf(stderr, "MigratePeriod[%d][%d][%d] = %d\n",  sp, stage, qid, MIGRATION[sp].Leave_Period[qid]);
 2676: 												fprintf(stderr, "MigIOBox[%d][%d][%d][%d]) = %e\n", sp, ij, stage, qid, MIGRATION[sp].Box[ij][qid]);
 2677: 											}
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
**Lines (current code):** 2083  
**Lines at review (Oct 2026):** 2080

**Function:** `Ecology_Total_Verts_And_Migration()`

**Why it's thought to be a bug**

The second suckling loop starts at `ad_cohort = ad_start_cohort`, which the first loop (2069) already handled, so that cohort's contaminants are transferred twice. Function: `Ecology_Total_Verts_And_Migration()`.

**Current code** (`atecology/atmovement.c` lines 2081–2085)

```c
 2081:                                             ad_num = FunctGroupArray[sp].LocalPopCount[ad_cohort];
 2082:                                             if(ad_num > bm->min_dens) {
 2083:                                                 Get_Suckling_Contaminants(bm, sp, ad_cohort, n);
 2084:                                                 juv_num -= ad_num;
 2085:                                             }
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
**Lines (current code):** 3187, 3463  
**Lines at review (Oct 2026):** 3184, 3460

**Function:** `Ecology_Invert_Migration()`

**Why it's thought to be a bug**

`numScalar_final` is never reset per cell, so after the first constrained cell every cell counts as "impacted" in the flagenviro_warn log. Function: `Ecology_Invert_Migration()`.

**Current code** (`atecology/atmovement.c` lines 3185–3189)

```c
 3185:                         if(bm->flagenviro_warn && (numScalar_final != 1.0)) {
 3186:                             cells_impacted++;
 3187:                         }
 3188: 
 3189:                     }
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
