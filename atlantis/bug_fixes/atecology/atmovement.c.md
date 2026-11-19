# atecology/atmovement.c — bug review

Generated 2026-11-19 from `atlantis/to-do-fixt-list.md`. **30 findings** (severity H: 3; confidence H and severity H: 2; on the priority list: 2), plus 11 lower-confidence notes.

Line numbers in **Listed lines** are from the review (10–12 Oct 2026). The code shown is from the working tree on 2026-11-19. Where the code has moved, the snippet follows it and says so. ⚠ means the quoted code could not be found: it may have been changed or already fixed.

Conf/Sev: confidence it's a bug / severity if it is (H = wrong results or crash, M = wrong in some configs, L = cosmetic, log, leak or latent). ✔ = the reviewer re-checked it by hand. ⭐ = on the high-priority list.

## Summary

| ID | Listed lines | Conf/Sev | Category | Short description | Decision | Done |
|---|---|---|---|---|---|---|
| [MOVE-01](#move-01) ✔ ⭐ | 2030 | H/H | swapped indices | `newden[sp][n][chkbox][kk] += den_diff;` — newden is allocated `Util_Alloc_Init_4D_Double(ncells, nlayer, ncoh… |  | [ ] |
| [MOVE-02](#move-02) ⭐ | 2049-2050 | H/H | wrong operator | `bm->boxes[ij].tr[k][DL_id] = VERTinfo[..SN_id] * totden * den_diff;` — this sets the labile/refractory detrit… |  | [ ] |
| [MOVE-03](#move-03) | 1552 | H/M | wrong variable | `(-1.0 * bm->boxes[bm->current_box].botz) <= ...[maxtotdepth_id]` — this is inside the `ij` box loop, but it u… |  | [ ] |
| [MOVE-04](#move-04) | 2997, 3073-3075 | H/M | wrong type / truncation | `int qrt, next_qrt, this_HowFar, ...` — this_HowFar is an int but gets `bm->HowFar` (a double fraction in [0,1… |  | [ ] |
| [MOVE-05](#move-05) | 3382 | H/M | wrong index | `FunctGroupArray[sp].distrib[ij][next_qrt][cohort]` — distrib is allocated `Util_Alloc_Init_3D_Double(2, numMo… |  | [ ] |
| [MOVE-06](#move-06) | 3624, 3627, 3637 | H/M | wrong index | `amt * tempdistrib[k][cohort]` — tempdistrib is `[wcnz][K_num_max_stages]` (atbiolsetup.c:2898), and `Get_Vert… |  | [ ] |
| [MOVE-07](#move-07) | 3498 | H/M | wrong index | `MIGRATION[sp].DEN[cohort][qid] += migbiom * totboxden[invert_biosum];` — the total biomass for this cohort is… |  | [ ] |
| [MOVE-08](#move-08) | 3274, 3333, 3550, 3567, 818 | H/M | off-by-one / buffer overrun | `totboxden[invert_biosum+cohort]` with invert_biosum = 1 (atlantisboxmodel.h:813) reaches index numCohortsXnum… |  | [ ] |
| [MOVE-09](#move-09) | 3101-3106, 3376-3379, 3397 | H/M | off-by-one | `if (habitatType == EPIFAUNA) maxk = 0; ... for (k = 0; k < maxk; k++)` — with maxk = 0 the loop never runs, s… |  | [ ] |
| [MOVE-10](#move-10) | 3033-3036 | M/H | inverted condition | `if (bm->flagday) day_part = 0; else day_part = 1;` — `distrib_VERTICAL[0]` is "night" and `[1]` is "day" (atB… |  | [ ] |
| [MOVE-11](#move-11) | 4067-4072 | H/M | off-by-one | `else year = bm->larvalYears;` — connectivitymatrix only holds data for years `0..larvalYears-1` (read with co… |  | [ ] |
| [MOVE-12](#move-12) | 4008-4042 | H/M | logic (overwrite instead of AND) | Each check overwrites `go_there` (temperature fail -> 0, then salinity OK -> 1, then O2 OK -> 1). Only the las… |  | [ ] |
| [MOVE-13](#move-13) | 3800 | H/M | uninitialised/wrong variable | `bm->boxes[b].tr[k][bm->InvaderPID] *= (1.0 + InvadeScalar);` — in the `conditional_invader` case, InvadeScala… |  | [ ] |
| [MOVE-14](#move-14) | 3929-3931, 3938, 3941 | H/M | copy-paste / wrong variable | `bk = k; if(bk >= bm->boxes[nb].nz) bk = bm->boxes[nb].nz - 1;` — k loops `< boxes[nb].nz`, so this test can n… |  | [ ] |
| [MOVE-15](#move-15) | 3810-3813, 3820-3910, 3918-3923 | M/M | normalisation error | The local box's scalar is written to every layer but added to `tot_scalar` only once. Neighbour scalars are ad… |  | [ ] |
| [MOVE-16](#move-16) | 3830 | M/M | copy-paste | `sp_likeCANYON = HABITATlike[..][CANYONcover_id];` is the only habitat term not multiplied by the box's area f… |  | [ ] |
| [MOVE-17](#move-17) | 1613-1618 | H/M | use before compute | Sticky ddepend with flagmother: `if ((roc[ij][age_mat] / roc_wgt) > ...)` runs inside the `n` loop while `n < … |  | [ ] |
| [MOVE-18](#move-18) | 472-492 | M/M | double counting | `if(!prey_counted[prey][n])` is checked outside the `predstage` loop. Inside it, every predator stage whose ga… |  | [ ] |
| [MOVE-19](#move-19) | 1159-1160, 1231-1243, 1246-1253 | H/L | dead code | `DEN` is clamped to 0 at 1159, so `if (DEN < 0.0)` at 1231 can never be true, and the SN/RN reset under flagav… |  | [ ] |
| [MOVE-20](#move-20) | 1270-1278 | M/M | logic | The larval_queue_extension reset happens inside the `n` (gene) and `qid` loops. After the first gene with `sto… |  | [ ] |
| [MOVE-21](#move-21) | 1392 | H/L | accumulator in wrong loop | `sp_amt += totden[sp][n];` sits inside the stock loop `ij < maxij`, so the total is multiplied by K_num_stocks… |  | [ ] |
| [MOVE-22](#move-22) | 1473, 4384/4416 | M/M | truncation | `contract_sp = (int)FunctGroupArray[sp].speciesParams[contract_tol_id];` (and `int contract_sp` in `Check_Cond… |  | [ ] |
| [MOVE-23](#move-23) | 2383-2386 | M/M | order dependency | `if(!totad[ngene]) recruit_hdistrib = distrib[..][adstage];` is inside the box/layer loop, while totad accumul… |  | [ ] |
| [MOVE-24](#move-24) | 2378-2379, 2390-2391 | M/M | inconsistent normalisation | The `recruit_hdistrib` numerator leaves out `noise_effect * light_effect`, but the `totad` denominator include… |  | [ ] |
| [MOVE-25](#move-25) | 2056, 2060-2097 | M/M | stale state | `LocalPopCount[n]` is only updated when `newden > 0`, so cells with no animals keep the previous cell's value … |  | [ ] |
| [MOVE-26](#move-26) | 1973-1985 vs 4306-4320 | M/M | double application | When enviro_depend is set, light/noise avoidance is applied to `tempdistrib` in `Get_Vertical_Distribution()` … |  | [ ] |
| [MOVE-27](#move-27) | 2789 | H/M | format mismatch | `quit("Time: %e box%d-%d %s-%d ...", bm->dayt, groupCode, n, ij, k)` — the arguments are in the wrong order (`… |  | [ ] |
| [MOVE-28](#move-28) | 2672, 2649, 2511/2515/2769/2773, 3630 | H/L | log text | 2672 prints `period = %e` with the int `Leave_Period[qid]` (UB). 2649 prints `Box %d;%d` with `ij, ij` (should… |  | [ ] |
| [MOVE-29](#move-29) | 2080 | M/L | off-by-one | The second suckling loop starts at `ad_cohort = ad_start_cohort`, which the first loop (2066) already handled,… |  | [ ] |
| [MOVE-30](#move-30) | 3184, 3460 | H/L | accumulator not reset | `numScalar_final` is never reset per cell, so after the first constrained cell every cell counts as "impacted"… |  | [ ] |

---

## MOVE-01

**swapped indices** · Conf H / Sev H ✔ re-checked ⭐ priority  
**Listed lines:** 2030

**Function:** `Ecology_Total_Verts_And_Migration()`

**Why it's thought to be a bug**

`newden[sp][n][chkbox][kk] += den_diff;` — newden is allocated `Util_Alloc_Init_4D_Double(ncells, nlayer, ncoh, nsp)` => `[sp][n][layer][box]` (atbiolsetup.c:2869) and used everywhere else as `newden[sp][n][k][ij]`. Here the box index is put in the layer slot, so any `chkbox >= wcnz` writes out of bounds (memory corruption) and the wrong cell gets the displaced fish. Function: `Ecology_Total_Verts_And_Migration()` (flagenviro_displace branch).

**Current code** (`atecology/atmovement.c` lines 2027–2031)

```c
 2027:                                             newden[sp][n][chkbox][kk] += den_diff;
 2028:                                             check_done = 1;
 2029:                                             den_diff = 0;
 2030:                                         }
 2031:                                     }
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
**Listed lines:** 2049-2050

**Function:** `Ecology_Total_Verts_And_Migration()`

**Why it's thought to be a bug**

`bm->boxes[ij].tr[k][DL_id] = VERTinfo[..SN_id] * totden * den_diff;` — this sets the labile/refractory detritus tracers to the killed biomass instead of adding to it, wiping out the detritus already in the cell. It also runs once per cohort, so each cohort overwrites the one before. The value is also mg N for the whole cell (per-individual N × numbers × proportion) written into a mg N m-3 tracer with no divide by volume. Function: `Ecology_Total_Verts_And_Migration()` (flagenviro_kill).

**Current code** (`atecology/atmovement.c` lines 2045–2050)

```c
 2045: 
 2046:                                 bm->boxes[ij].tr[k][DL_id] = VERTinfo[sp][n][SN_id] * totden[sp][n] * den_diff;
 2047:                                 bm->boxes[ij].tr[k][DR_id] = VERTinfo[sp][n][RN_id] * totden[sp][n] * den_diff;
 2048:                             }
 2049:                             
 2050:                             
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
**Listed lines:** 1552

**Function:** `Ecology_Total_Verts_And_Migration()`

**Why it's thought to be a bug**

`(-1.0 * bm->boxes[bm->current_box].botz) <= ...[maxtotdepth_id]` — this is inside the `ij` box loop, but it uses `bm->current_box`, which is old state left over from the process loop. The two checks next to it use `bm->boxes[ij].botz`, and atbiolsetup.c:3290 does the same max-total-depth test with `ij`. So the depth exclusion for forage-dependent movement uses the wrong box. Function: `Ecology_Total_Verts_And_Migration()`.

**Current code** (`atecology/atmovement.c` lines 1554–1558) — code appears to have moved +4 lines since the review

```c
 1554:                                 		( bm->boxes[ij].botz <= (-1.0 * FunctGroupArray[sp].speciesParams[mindepth_id]) &&
 1555:                                 		 bm->boxes[ij].botz >= (-1.0 * FunctGroupArray[sp].speciesParams[maxdepth_id]) &&
 1556:                                         ((-1.0 * bm->boxes[bm->current_box].botz) <= FunctGroupArray[sp].speciesParams[maxtotdepth_id]))) {
 1557:                                 //if ((int) (FunctGroupArray[sp].speciesParams[flag_id]) && (int) (FunctGroupArray[sp].speciesParams[active_id]) &&
 1558:                                 //    (current_depth <= (-1.0 * FunctGroupArray[sp].speciesParams[mindepth_id]) &&
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
**Listed lines:** 2997, 3073-3075

**Function:** `Ecology_Invert_Migration()`

**Why it's thought to be a bug**

`int qrt, next_qrt, this_HowFar, ...` — this_HowFar is an int but gets `bm->HowFar` (a double fraction in [0,1), attime.c:209) or `FunctGroupArray[sp].HowFar`. It truncates to 0, so prescribed invertebrate movement (line 3382) never blends between seasons and jumps when the season changes. The vertebrate routine declares it `double` (line 867). Also, `FunctGroupArray[].HowFar` is itself declared `int` (atlantisUtil/include/atFunctGroup.h:94) but is assigned a fractional remainder (attime.c:524), so it is always 0 for vertebrates too whenever numMoveEntries != 4. Function: `Ecology_Invert_Migration()`.

**Current code** (`atecology/atmovement.c` lines 2993–2997)

```c
 2993:     //double step1, step2;
 2994:     double migtime2;
 2995:     int qrt, next_qrt, this_HowFar, agec, sp_Migrate_Years;
 2996: 	//int sp_debug;
 2997:     // do_debug;
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
**Listed lines:** 3382

**Function:** `Ecology_Invert_Migration()`

**Why it's thought to be a bug**

`FunctGroupArray[sp].distrib[ij][next_qrt][cohort]` — distrib is allocated `Util_Alloc_Init_3D_Double(2, numMoveEntries, nbox)` => `[nbox][entries][2]`, indexed by stage (atUtilGroupIO.c:918; the vertebrate code uses `[stage]`, e.g. line 1932). Using `cohort` (0..numCohortsXnumGenes-1) reads out of bounds for an invert with more than 2 cohort×gene entries, and reads the wrong stage when genes > 1. Function: `Ecology_Invert_Migration()`.

**Current code** (`atecology/atmovement.c` lines 3380–3384)

```c
 3380:                             /*
 3381:                             if(sp_debug == 2){
 3382:                                 fprintf(llogfp,"day: %e, %s-newden[%d][%d]: %e, HowFar: %e, distrib%s-%d-%d-%d: %e, distrib%s-%d-%d-%d: %e\n",
 3383:                                     bm->dayt, FunctGroupArray[sp].groupCode, cohort, ij, newden[sp][cohort][0][ij], this_HowFar, FunctGroupArray[sp].groupCode, ij, next_qrt, cohort, FunctGroupArray[sp].distrib[ij][next_qrt][cohort], FunctGroupArray[sp].groupCode, ij, qrt, cohort, FunctGroupArray[sp].distrib[ij][qrt][cohort]);
 3384:                             }
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
**Listed lines:** 3624, 3627, 3637

**Function:** `Ecology_Invert_Migration()`

**Why it's thought to be a bug**

`amt * tempdistrib[k][cohort]` — tempdistrib is `[wcnz][K_num_max_stages]` (atbiolsetup.c:2898), and `Get_Vertical_Distribution()` only fills `[k][juv_id]` and `[k][stage]` (tracker_id = stage, line 4154). Line 3620 correctly reads `tempdistrib[k][stage]`. Indexing by cohort goes out of bounds or reads unset columns when cohorts×genes > stages. Function: `Ecology_Invert_Migration()`.

**Current code** (`atecology/atmovement.c` lines 3622–3639)

```c
 3622:                                 if (!(_finite(bm->boxes[ij].tr[k][pid] ))) {
 3623:                                     printf("amt = %e,  tempdistrib[k][0] = %e, bm->boxes[ij].dz[k] = %e, bm->boxes[ij].area = %e \n",
 3624:                                             amt,  tempdistrib[k][cohort], bm->boxes[ij].dz[k], bm->boxes[ij].area);
 3625:                                     quit("Ecology_Invert_Migration - day: %e, box: %d, layer: %d %s set to %e.\n",
 3626:                                             bm->dayt, bm->current_box, bm->current_layer, FunctGroupArray[sp].groupCode,
 3627:                                             bm->boxes[ij].tr[k][pid] );
 3628:                                 }
 3629: 
 3630:                                 /**
 3631:                                 if (sp_debug == 2) {
 3632:                                     fprintf(llogfp, "Time: %e, sp %s in box: %d layer: %d, ends redistrib with %.20e (amt: %.20e tempdistrib: %.20e)\n",
 3633: 											bm->dayt, FunctGroupArray[sp].groupCode, ij, k, bm->boxes[ij].tr[k][pid], amt, tempdistrib[k][cohort]);
 3634:                                 }
 3635:                                 **/
 3636: 
 3637:                             }
 3638:                         }
 3639:                     }
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
**Listed lines:** 3498

**Function:** `Ecology_Invert_Migration()`

**Why it's thought to be a bug**

`MIGRATION[sp].DEN[cohort][qid] += migbiom * totboxden[invert_biosum];` — the total biomass for this cohort is stored at `totboxden[invert_biosum+cohort]` (lines 3274, 3333, 3550, 3567). Cohort 0's biomass is used for every cohort, so migrant biomass for cohorts > 0 is wrong. Function: `Ecology_Invert_Migration()`.

**Current code** (`atecology/atmovement.c` lines 3496–3500)

```c
 3496:                                     MIGRATION[sp].num_stagger[qid] = 0;
 3497:                                     if(FunctGroupArray[sp].multiyr_mig) {
 3498:                                         sp_Migrate_Years = (int) (MIGRATION[sp].MinYearsAway[qid]);
 3499:                                         if ((sp_Migrate_Years > 1) && (stagger_return)) {
 3500:                                             MIGRATION[sp].num_stagger[qid] = MIGRATION[sp].Stagger[qid][stagger_years_id];
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
**Listed lines:** 3274, 3333, 3550, 3567, 818

**Why it's thought to be a bug**

`totboxden[invert_biosum+cohort]` with invert_biosum = 1 (atlantisboxmodel.h:813) reaches index numCohortsXnumGenes. totboxden is allocated with only `K_num_max_cohort*K_num_max_genetypes` (atbiolsetup.c:2922). atbiolsetup.c:2905-2907 computes `totdensize = K_max_invert_cohorts+1` "as need an additional slot for propsum in invertebrate movement code", but applies it to totden/totden_check, not totboxden. So it overruns when an invert has the maximum cohort×gene count. Functions: `Ecology_Invert_Migration()`, `Store_Min_Max_Avg()`.

**Current code** (`atecology/atmovement.c` lines 3268–3272) — code appears to have moved -4 lines since the review

```c
 3268: 
 3269:                 /* Store biomass so can scale movement distributions */
 3270:                 totboxden[invert_biosum+cohort] = totamt;
 3271:                 
 3272:                 
```

**Suggested fix (from review):** Allocate totboxden with `totdensize` (atbiolsetup.c:2922)

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
**Listed lines:** 3101-3106, 3376-3379, 3397

**Why it's thought to be a bug**

`if (habitatType == EPIFAUNA) maxk = 0; ... for (k = 0; k < maxk; k++)` — with maxk = 0 the loop never runs, so epifauna skip all temperature, salinity, O2, ice, light, noise and contaminant limits, both for non-mobile groups (3095 block) and for prescribed movement. The epifauna `epi[pid]` branches inside those loops can never run.

**Current code** (`atecology/atmovement.c` lines 3099–3108)

```c
 3099:                     if (FunctGroupArray[fgIndex].habitatType == EPIFAUNA)
 3100:                         maxk = 0;
 3101:                     else
 3102:                         maxk = bm->boxes[ij].nz;
 3103:                     
 3104:                     for (k = 0; k < maxk; k++) {
 3105:                         /* Noise and light pollution avoidance */
 3106:                         if (FunctGroupArray[sp].isLightEffected) {
 3107:                             current_enviro = bm->boxes[ij].tr[k][Light_Pollution_i];
 3108:                             if (current_enviro > 0) {
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
**Listed lines:** 3033-3036

**Function:** `Ecology_Invert_Migration()`

**Why it's thought to be a bug**

`if (bm->flagday) day_part = 0; else day_part = 1;` — `distrib_VERTICAL[0]` is "night" and `[1]` is "day" (atBiologyXMLParamIO.c:93-94), and flagday = 1 means daytime (attime.c Time_ODay; comment at line 906). The vertebrate routine passes `day_part = bm->flagday`. Invertebrates therefore use the night profile in the day and the day profile at night (diel migration reversed), unless this reversal is a deliberate legacy choice. Function: `Ecology_Invert_Migration()`.

**Current code** (`atecology/atmovement.c` lines 3031–3038)

```c
 3031: 	if (bm->flagday)
 3032: 		day_part = 0;
 3033: 	else
 3034: 		day_part = 1;
 3035: 
 3036: 	/* Initialise local arrays */
 3037: 	for (sp = 0; sp < bm->K_num_tot_sp; sp++) {
 3038: 		if (FunctGroupArray[sp].isVertebrate == FALSE && FunctGroupArray[sp].isImpacted == TRUE) {
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
**Listed lines:** 4067-4072

**Function:** `Larval_Dispersal()`

**Why it's thought to be a bug**

`else year = bm->larvalYears;` — connectivitymatrix only holds data for years `0..larvalYears-1` (read with count `min(larvalYears, numRunYears)`, atbiolsetup.c:3727). Past the end of the data with larval_rewind off, the code indexes a slot that was never filled (all zeros), so larvaldistrib = 0 and there is no recruitment. Function: `Larval_Dispersal()`.

**Current code** (`atecology/atmovement.c` lines 4065–4074)

```c
 4065: 			year = 0;
 4066: 		else
 4067: 			year = bm->larvalYears;
 4068: 
 4069: 	}
 4070: 
 4071: 	//fprintf(llogfp, "updated - year = %d\n", year);
 4072: 
 4073: 	// Do the matrix multiplication of connectivity matrx * adults
 4074:     if (!bm->larvae_connect_only) {
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
**Listed lines:** 4008-4042

**Function:** `Invade_Spread()`

**Why it's thought to be a bug**

Each check overwrites `go_there` (temperature fail -> 0, then salinity OK -> 1, then O2 OK -> 1). Only the last active criterion counts. Light and noise set `go_there = ceil(coefft/enviro)`, which can override earlier failures. Invaders can spread into cells that fail temperature or salinity limits. Function: `Invade_Spread()`.

**Current code** (`atecology/atmovement.c` lines 4006–4044)

```c
 4006:             go_there = 0;
 4007:         else
 4008:             go_there = 1;
 4009:     }
 4010:     
 4011:     if (salt_sensitive_sp) {
 4012:         if ( (min_salt_sp > there_salt ) || (max_salt_sp < there_salt ))
 4013:             go_there = 0;
 4014:         else
 4015:             go_there = 1;
 4016:     }
 4017: 
 4018:     if (bm->flagO2depend) {
 4019:         if (there_o2 > min_O2_sp )
 4020:             go_there = 1;
 4021:         else
 4022:             go_there = 0;
 4023:     }
 4024:     
 4025:     /* Noise and light pollution avoidance */
 4026:     if (FunctGroupArray[sp].isLightEffected) {
 4027:         current_enviro = bm->boxes[nb].tr[k][Light_Pollution_i];
 4028:         if (current_enviro > 0) {
 4029:             go_there = ceil(FunctGroupArray[sp].speciesParams[light_coefft_id] / (current_enviro + small_num));
 4030:         }
 4031:     }
 4032:     
 4033:     if (FunctGroupArray[sp].isNoiseEffected) {
 4034:         current_enviro = bm->boxes[nb].tr[k][Noise_Pollution_i];
 4035:         if (current_enviro > 0) {
 4036:             go_there = ceil(FunctGroupArray[sp].speciesParams[noise_coefft_id] / (current_enviro + small_num));
 4037:         }
 4038:     }
 4039:     
 4040:     
 4041:     return go_there;
 4042:     
 4043: }
 4044: 
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
**Listed lines:** 3800

**Function:** `Ecology_Invading_Species()`

**Why it's thought to be a bug**

`bm->boxes[b].tr[k][bm->InvaderPID] *= (1.0 + InvadeScalar);` — in the `conditional_invader` case, InvadeScalar is never set (it is only computed in `simple_invader`), so it stays 0.0. Invertebrate entrants are never scaled, while the vertebrate branch two lines above uses `bm->InvaderScalar`. Function: `Ecology_Invading_Species()`.

**Current code** (`atecology/atmovement.c` lines 3778–3782) — code appears to have moved -20 lines since the review

```c
 3778: 		}
 3779: 		break;
 3780:     case conditional_invader:  /* Based on depth, temperature and salinity preferences, habitat and prey */
 3781:         for (b = 0; b < bm->nbox; b++) {
 3782:             Util_Init_2D_Double(bm->invader_spread_scalars, bm->nbox, bm->wcnz, 0.0); // has to be here because the normalisation is over all boxes not just neigbouring boxes and then use that value to immediately go round and scale all the tracers
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
**Listed lines:** 3929-3931, 3938, 3941

**Function:** `Ecology_Invading_Species()`

**Why it's thought to be a bug**

`bk = k; if(bk >= bm->boxes[nb].nz) bk = bm->boxes[nb].nz - 1;` — k loops `< boxes[nb].nz`, so this test can never be true. It was clearly meant to clamp against the source box `b`, since `boxes[b].tr[bk]` is read next. When nb is deeper than b, unused layers of b are read. Function: `Ecology_Invading_Species()`.

**Current code** (`atecology/atmovement.c` lines 3925–3941)

```c
 3925:                         bk = k;
 3926:                         if(bk >= bm->boxes[nb].nz)
 3927:                             bk = bm->boxes[nb].nz - 1;
 3928:                         this_scalar = bm->invader_spread_scalars[nb][k];
 3929: 
 3930:                         // Proportion of numbers or biomass in local box b dispersing to destinaiton box nb
 3931:                         if (FunctGroupArray[bm->InvaderIndex].isVertebrate == TRUE) {
 3932:                             for (n = bm->minInvaderAge; n < bm->maxInvaderAge; n++) {
 3933:                                 den = FunctGroupArray[bm->InvaderIndex].NumsTracers[n];
 3934:                                 bm->boxes[nb].tr[k][den] += (this_scalar * bm->boxes[b].tr[bk][den]);
 3935:                             }
 3936:                         } else {
 3937:                             bm->boxes[nb].tr[k][bm->InvaderPID] += this_scalar * bm->boxes[b].tr[bk][bm->InvaderPID];
 3938:                         }
 3939:                     }
 3940:                 }
 3941:                 // Reset local numbers or biomass to reflect those that dispersed
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
**Listed lines:** 3810-3813, 3820-3910, 3918-3923

**Function:** `Ecology_Invading_Species()`

**Why it's thought to be a bug**

The local box's scalar is written to every layer but added to `tot_scalar` only once. Neighbour scalars are added to `tot_scalar` once per invader cohort n (loop at 3820) but stored only once per [nb][k], with the last n winning. Each source layer then keeps `s_b/tot` and sends `s_nb,k/tot` to each neighbour layer, so the fractions for a layer do not add to 1 and invader biomass/numbers are not conserved. Function: `Ecology_Invading_Species()`.

**Current code** (`atecology/atmovement.c` lines 3807–3814)

```c
 3807:             	bm->invader_spread_scalars[b][k] = this_scalar;
 3808:             }
 3809:             tot_scalar += this_scalar;
 3810:             
 3811:             // Now do neighbouring boxes
 3812:             for (chkbox = 0; chkbox < bm->boxes[b].nUniqueAdj; chkbox++) {
 3813:                 nb = bm->boxes[b].uniqueAdjBoxes[chkbox];
 3814:                 this_depth = bm->boxes[nb].botz;
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
**Listed lines:** 3830

**Function:** `Ecology_Invading_Species()`

**Why it's thought to be a bug**

`sp_likeCANYON = HABITATlike[..][CANYONcover_id];` is the only habitat term not multiplied by the box's area fraction. REEF, FLAT and SOFT are multiplied by `boxes[nb].reef/flat/soft`, and `double canyon` exists in the box struct (atlantisboxmodel.h:3710). Any canyon-liking invader therefore treats every box as suitable habitat. Function: `Ecology_Invading_Species()`.

**Current code** (`atecology/atmovement.c` lines 3824–3828) — code appears to have moved -4 lines since the review

```c
 3824:                             sp_likeFLAT = bm->HABITATlike[bm->InvaderIndex][chrtstage][bm->FLATcover_id] * bm->boxes[nb].flat;
 3825:                             sp_likeSOFT = bm->HABITATlike[bm->InvaderIndex][chrtstage][bm->SOFTcover_id] * bm->boxes[nb].soft;
 3826:                             sp_likeCANYON = bm->HABITATlike[bm->InvaderIndex][chrtstage][bm->CANYONcover_id];
 3827:                             if ((sp_likeREEF > 0.0) || (sp_likeFLAT > 0.0) || (sp_likeSOFT > 0.0) || (sp_likeCANYON > 0.0))
 3828:                                 sp_like = 1;
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
**Listed lines:** 1613-1618

**Function:** `Ecology_Total_Verts_And_Migration()`

**Why it's thought to be a bug**

Sticky ddepend with flagmother: `if ((roc[ij][age_mat] / roc_wgt) > ...)` runs inside the `n` loop while `n < age_mat`, so roc[ij][age_mat] for this species and timestep has not been computed yet. It holds the value from the previous species or timestep. (The same test at 1864 runs after all roc are computed, so it is fine.) Function: `Ecology_Total_Verts_And_Migration()`.

**Current code** (`atecology/atmovement.c` lines 1610–1619)

```c
 1610: 									fprintf(llogfp,"Time: %e box%d %s-%d totroc: %.20e, roc: %.20e, flaghabdepend: %d, refuge_status[0]: %.20e, refuge_status[1]: %.20e, roc_wgt: %.20e, E1_sp: %.20e, clear: %.20e\n",
 1611: 										bm->dayt, ij, FunctGroupArray[sp].groupCode, n, totroc[n], roc[ij][n], bm->flaghabdepend,
 1612: 										bm->refuge_status[sp][ij][0], bm->refuge_status[sp][ij][1], roc_wgt, E1_sp, clear);
 1613: 								}
 1614: 								**/
 1615: 							}
 1616: 
 1617: 							if (sp_ddepend_move == sticky_ddepend) {
 1618: 								/* If using 'sticky' density dependent movement determine what proportion has to move
 1619: 								 then move them. Use average sized individual for comparison purposes */
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
**Listed lines:** 472-492

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
**Listed lines:** 1159-1160, 1231-1243, 1246-1253

**Function:** `Ecology_Total_Verts_And_Migration()`

**Why it's thought to be a bug**

`DEN` is clamped to 0 at 1159, so `if (DEN < 0.0)` at 1231 can never be true, and the SN/RN reset under flagavgmig never happens. The 1246 test `dayt > Return_Now+Return_Period` can never be true inside the enclosing condition at 1120 (`dayt < Return_Now+Return_Period`), so `Checked[qid]` is never set here. Function: `Ecology_Total_Verts_And_Migration()`.

**Current code** (`atecology/atmovement.c` lines 1147–1152) — code appears to have moved -10 lines since the review

```c
 1147:                         //if ((sp == 33) || ((sp > 8) && (sp < 12))) {
 1148:                         if (sp == 33) {
 1149:                             fprintf(bm->logFile, "Time: %e, %s qid: %d migtime2 = %e, period = %d, migtime = %e, migrateReturn = %d, away = %d with original migDEN: %e stagger_scalar: %e num_stagger: %e\n", bm->dayt, FunctGroupArray[sp].groupCode, qid, migtime2, MIGRATION[sp].Return_Period[qid], migtime, MIGRATION[sp].Return_Now[qid], MIGRATION[sp].Leave_Now[qid], MIGRATION[sp].DEN[n][qid], stagger_scalar, (double)(MIGRATION[sp].num_stagger[qid]));
 1150: 						}
 1151:                        **/
 1152:                         
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
**Listed lines:** 1270-1278

**Function:** `Ecology_Total_Verts_And_Migration()`

**Why it's thought to be a bug**

The larval_queue_extension reset happens inside the `n` (gene) and `qid` loops. After the first gene with `stock_id == numStocks-1` resets it to 0, later genes skip their pre-model larvae, which are lost. If `stock_id < 0` it is set to 0 (line 1188), so the reset only fires when numStocks == 1. Function: `Ecology_Total_Verts_And_Migration()`.

**Current code** (`atecology/atmovement.c` lines 1266–1278)

```c
 1266: 
 1267:                         if (stock_id < 0) {
 1268:                           stock_id = 0; 
 1269:                         }
 1270:                         
 1271:                         if ((n < FunctGroupArray[sp].numGeneTypes) && FunctGroupArray[sp].speciesParams[recruit_outside_id]) {
 1272:                             // First start with any that might be coming from before model started
 1273:                             yoy_den = 0.0;
 1274:                             if(FunctGroupArray[sp].speciesParams[larval_queue_extension_id] > 0) {
 1275:                                 for (lid = 0; lid < FunctGroupArray[sp].speciesParams[larval_queue_extension_id]; lid++) {
 1276:                                     yoy_den += EMBRYO[sp].Larvae[stock_id][n][lid];
 1277:                                     EMBRYO[sp].Larvae[stock_id][n][lid] = 0;
 1278:                                 }
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
**Listed lines:** 1392

**Function:** `Ecology_Total_Verts_And_Migration()`

**Why it's thought to be a bug**

`sp_amt += totden[sp][n];` sits inside the stock loop `ij < maxij`, so the total is multiplied by K_num_stocks_per_sp. This loosens the "less than one individual present" skip at 1404. Function: `Ecology_Total_Verts_And_Migration()`.

**Current code** (`atecology/atmovement.c` lines 1391–1395)

```c
 1391:             
 1392:             /* Normalise proportion of total abundance in each stock */
 1393: 			for (ij = 0; ij < maxij; ij++) {
 1394: 				for (n = 0; n < FunctGroupArray[sp].numCohortsXnumGenes; n++) {
 1395: 					bm->stock_struct_prop[sp][n][ij] = init_stock_struct_prop[sp][n][ij] / (totden[sp][n] + small_num);
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
**Listed lines:** 1473, 4384/4416

**Why it's thought to be a bug**

`contract_sp = (int)FunctGroupArray[sp].speciesParams[contract_tol_id];` (and `int contract_sp` in `Check_Conditions()`) truncates the pH tolerance contraction, e.g. 0.5 -> 0. The invertebrate routine (3204) correctly keeps it as a double. Functions: `Ecology_Total_Verts_And_Migration()`, `Check_Conditions()`.

**Current code** (`atecology/atmovement.c` lines 1472–1476)

```c
 1472:             
 1473:             /* Make parameter adjustments due to acidification */
 1474: 			flagcontract_sp = (int)FunctGroupArray[sp].speciesParams[flagcontract_tol_id];
 1475: 			pH_scale = (FunctGroupArray[sp].pHcorr - 1.0);
 1476: 			if( flagcontract_sp  && (pH_scale > 0.0)) {
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
**Listed lines:** 2383-2386

**Function:** `Ecology_Total_Verts_And_Migration()`

**Why it's thought to be a bug**

`if(!totad[ngene]) recruit_hdistrib = distrib[..][adstage];` is inside the box/layer loop, while totad accumulates across boxes. Boxes processed before the first adult is found get the prescribed distribution; later empty boxes get 0. The fallback is also not added to totad, so the later normalisation is off. Function: `Ecology_Total_Verts_And_Migration()`.

**Current code** (`atecology/atmovement.c` lines 2380–2387)

```c
 2380:                                     if(!totad[ngene]) {  // If no adults about set back to the prescribed distribution
 2381:                                         adstage = FunctGroupArray[sp].cohort_stage[age_mat];
 2382:                                         bm->recruit_hdistrib[ngene][ij][sp] = FunctGroupArray[sp].distrib[ij][qrt][adstage];
 2383:                                     }
 2384:                                     
 2385:                                     
 2386: 								} else {
 2387:                                     bm->recruit_hdistrib[ngene][ij][sp] =  bm->recruit_hdistrib_orig[ngene][ij][sp] * temp_effect * salt_effect * o2_effect * ice_effect;
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
**Listed lines:** 2378-2379, 2390-2391

**Function:** `Ecology_Total_Verts_And_Migration()`

**Why it's thought to be a bug**

The `recruit_hdistrib` numerator leaves out `noise_effect * light_effect`, but the `totad` denominator includes them. After normalisation (2409) the distribution does not sum to 1, and light/noise reduce the denominator instead of steering larvae away. Function: `Ecology_Total_Verts_And_Migration()`.

**Current code** (`atecology/atmovement.c` lines 2376–2393)

```c
 2376:                                         totad[ngene] += newden[sp][n][k][ij] * temp_effect * salt_effect * o2_effect * ice_effect * noise_effect * light_effect;
 2377:                                         
 2378:                                     }
 2379:                                     
 2380:                                     if(!totad[ngene]) {  // If no adults about set back to the prescribed distribution
 2381:                                         adstage = FunctGroupArray[sp].cohort_stage[age_mat];
 2382:                                         bm->recruit_hdistrib[ngene][ij][sp] = FunctGroupArray[sp].distrib[ij][qrt][adstage];
 2383:                                     }
 2384:                                     
 2385:                                     
 2386: 								} else {
 2387:                                     bm->recruit_hdistrib[ngene][ij][sp] =  bm->recruit_hdistrib_orig[ngene][ij][sp] * temp_effect * salt_effect * o2_effect * ice_effect;
 2388:                                     totad[ngene] += bm->recruit_hdistrib_orig[ngene][ij][sp] * temp_effect * salt_effect * o2_effect * ice_effect * noise_effect * light_effect;
 2389:                                 }
 2390:                                 
 2391:                                 /**
 2392:                                 if(sp == bm->which_check){
 2393:                                     fprintf(llogfp,"movement.c Time: %e, %s box-%d recruit_hdistrib: %e (is_maternal_raised: %d orig: %e, temp_effect: %e, salt_effect: %e, o2_effect: %e, ice_effect: %e, newden: %e totad: %e update_larval_distrib:%d)\n",
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
**Listed lines:** 2056, 2060-2097

**Function:** `Ecology_Total_Verts_And_Migration()`

**Why it's thought to be a bug**

`LocalPopCount[n]` is only updated when `newden > 0`, so cells with no animals keep the previous cell's value and trigger suckling transfers that should not happen. `suckler_contam_today_id = 1` (2097) is reset inside the per-cell (ij,k) loop, so the "skip on birth day" set in atdemography.c:473 only applies to the first cell. Function: `Ecology_Total_Verts_And_Migration()`.

**Current code** (`atecology/atmovement.c` lines 2054–2093 (truncated))

```c
 2054:                                 }
 2055:                             }
 2056: 						}
 2057:                         if( bm->track_contaminants && bm->flag_contamMaternalTransfer && (flagmother > 0) && (FunctGroupArray[sp].speciesParams[suckler_contam_today_id] > 0)) {
 2058:                             for (n = 0; n < age_mat; n++) {
 2059:                                 juv_num = FunctGroupArray[sp].LocalPopCount[n];
 2060:                                 not_done = 1;
 2061:                                 if(juv_num > bm->min_dens){ // Only do if juveniles present
 2062:                                     ad_start_cohort = (int)(floor(drandom(start_n, end_n))); // Find random starting point so not always the youngest adutlts feeding the young
 2063:                                     for (ad_cohort = ad_start_cohort; ad_cohort < FunctGroupArray[sp].numCohortsXnumGenes; ad_cohort++) { // Iterate up age classes
 2064:                                         ad_num = FunctGroupArray[sp].LocalPopCount[ad_cohort];
 2065:                                         if(ad_num > bm->min_dens) {
 2066:                                             Get_Suckling_Contaminants(bm, sp, ad_cohort, n);
 2067:                                             juv_num -= ad_num;
 2068:                                         }
 2069:                                         if(juv_num > 0) {
 2070:                                             // Do nothing as not finished yet
 2071:                                         } else {
 2072:                                             not_done = 0;
 2073:                                             break;
 2074:                                         }
 2075:                                     }
 2076:                                     if(not_done > 0){
 2077:                                         for (ad_cohort = ad_start_cohort; ad_cohort > (age_mat - 1); ad_cohort--) { // Iterate over the rest of the adults if we have to
 2078:                                             ad_num = FunctGroupArray[sp].LocalPopCount[ad_cohort];
 2079:                                             if(ad_num > bm->min_dens) {
 2080:                                                 Get_Suckling_Contaminants(bm, sp, ad_cohort, n);
 2081:                                                 juv_num -= ad_num;
 2082:                                             }
 2083:                                             if(juv_num > 0) {
 2084:                                                 // Do nothing as not finished yet
 2085:                                             } else {
 2086:                                                 not_done = 0;
 2087:                                                 break;
 2088:                                             }
 2089:                                         }
 2090:                                     }
 2091:                                 }
 2092:                             }
 2093:                         }
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
**Listed lines:** 1973-1985 vs 4306-4320

**Why it's thought to be a bug**

When enviro_depend is set, light/noise avoidance is applied to `tempdistrib` in `Get_Vertical_Distribution()` (which feeds `vertdistrib` into newden) and then applied again to newden at 1973-1985. Inverts get the same double effect (3399-3411).

**Current code** (`atecology/atmovement.c` lines 1950–1966) — code appears to have moved -21 lines since the review

```c
 1950:                                 for (rij = 0; rij < bm->boxes[ij].HomeRangeInfo[sp].n; rij++) {
 1951:                                     rangeid = bm->boxes[ij].HomeRangeInfo[sp].ids[rij];
 1952:                                     newden[sp][n][k][ij] += vertdistrib * bm->HomeRangeTotal[sp][n][rangeid] * bm->boxes[ij].HomeRangeInfo[sp].rangewgt[rij];
 1953:                                 }
 1954:                                 if (bm->flag_move_habdepend) {
 1955:                                     newden[sp][n][k][ij] *= bm->refuge_status[sp][ij][stage];
 1956:                                 }
 1957:                                 break;
 1958:                             default:
 1959:                                 quit("No such ddepend flag value (%d) for %s\n", sp_ddepend_move, FunctGroupArray[sp].groupCode);
 1960:                                 break;
 1961:                             }
 1962: 
 1963: 							/* Do a check to see how we are travelling so far */
 1964: 							if(newden[sp][n][k][ij] < 0){
 1965: 								quit("Vertebrate movement. Density is less than 0 for group %s, cohort %d in box %d:%d, sp_ddepend_move = %d\n",
 1966: 										FunctGroupArray[sp].groupCode, n, ij, k, sp_ddepend_move);
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
**Listed lines:** 2789

**Function:** `Ecology_Total_Verts_And_Migration()`

**Why it's thought to be a bug**

`quit("Time: %e box%d-%d %s-%d ...", bm->dayt, groupCode, n, ij, k)` — the arguments are in the wrong order (`%d` gets a char*, `%s` gets an int), so the error path will likely segfault instead of printing. Function: `Ecology_Total_Verts_And_Migration()`.

**Current code** (`atecology/atmovement.c` lines 2784–2788) — code appears to have moved -3 lines since the review

```c
 2784:                                     bm->boxes[ij].tr[k][rn], VERTinfo[sp][n][SN_id], VERTinfo[sp][n][RN_id]);
 2785:                                 
 2786:                                 quit("Time: %e box%d-%d %s-%d had negative size or numbers after movement\n", bm->dayt, FunctGroupArray[sp].groupCode, n, ij, k);
 2787:                             }
 2788: 
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
**Listed lines:** 2672, 2649, 2511/2515/2769/2773, 3630

**Why it's thought to be a bug**

2672 prints `period = %e` with the int `Leave_Period[qid]` (UB). 2649 prints `Box %d;%d` with `ij, ij` (should be ij, k). The negative-size diagnostics and the invert quit print `bm->current_box, bm->current_layer` instead of `ij, k`.

**Current code** (`atecology/atmovement.c` lines 2670–2674)

```c
 2670: 
 2671: 												fprintf(stderr, "MigrateAway[%d][%d][%d] = %d\n", sp, stage, qid, MIGRATION[sp].Leave_Now[qid]);
 2672: 												fprintf(stderr, "MigratePeriod[%d][%d][%d] = %d\n",  sp, stage, qid, MIGRATION[sp].Leave_Period[qid]);
 2673: 												fprintf(stderr, "MigIOBox[%d][%d][%d][%d]) = %e\n", sp, ij, stage, qid, MIGRATION[sp].Box[ij][qid]);
 2674: 											}
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
**Listed lines:** 2080

**Function:** `Ecology_Total_Verts_And_Migration()`

**Why it's thought to be a bug**

The second suckling loop starts at `ad_cohort = ad_start_cohort`, which the first loop (2066) already handled, so that cohort's contaminants are transferred twice. Function: `Ecology_Total_Verts_And_Migration()`.

**Current code** (`atecology/atmovement.c` lines 2078–2082)

```c
 2078:                                             ad_num = FunctGroupArray[sp].LocalPopCount[ad_cohort];
 2079:                                             if(ad_num > bm->min_dens) {
 2080:                                                 Get_Suckling_Contaminants(bm, sp, ad_cohort, n);
 2081:                                                 juv_num -= ad_num;
 2082:                                             }
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
**Listed lines:** 3184, 3460

**Function:** `Ecology_Invert_Migration()`

**Why it's thought to be a bug**

`numScalar_final` is never reset per cell, so after the first constrained cell every cell counts as "impacted" in the flagenviro_warn log. Function: `Ecology_Invert_Migration()`.

**Current code** (`atecology/atmovement.c` lines 3180–3184)

```c
 3180:                         }
 3181:                         
 3182:                         if(bm->flagenviro_warn && (numScalar_final != 1.0)) {
 3183:                             cells_impacted++;
 3184:                         }
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

`n < age_mat`, `roc[ij][age_mat]`, `totroc[age_mat]` (1617, 1801-1805, 1862-1869, 2061) and `PartialMinAge/MaxAge` vs `n` (2626) compare an age-class/cohort number with the cohort×genotype index `n` (layout is `n = numGeneTypes*cohort + gene`, see 2314). These are wrong when numGeneTypes > 1. The same applies to `drandom(start_n=age_mat, end_n=numCohortsXnumGenes)` at 2065.

- [ ] Investigate   - [ ] Ignore

**Your comments:**

> 

### LC-2

In the flagenviro_displace branch (2017-2037), fish moved into a neighbour `chkbox > ij` are overwritten when that box is processed later (newden is assigned, not added to). The neighbour can also be a BOUNDARY/LAND box.

- [ ] Investigate   - [ ] Ignore

**Your comments:**

> 

### LC-3

Tidal channel code (2119-2145): the `<` test at 2119, the `>` at 2122 and the `>=` at 2131 are inconsistent. The `area*current_botz` weighting (2123) vs `area*dz[k]` (2133) depends on the sign of botz (negative botz makes nbox_spread <= 0). `tempdistrib[k][stage]` is left over from the last Get_Vertical_Distribution call, not from chkbox. The `else` branch zeroes neighbouring shallow boxes' densities rather than moving them.

- [ ] Investigate   - [ ] Ignore

**Your comments:**

> 

### LC-4

`Store_Min_Max_Avg()` line 763 returns unless *both* flagFonly and flag_systcap are set (`||` of negations). Check whether `&&` was intended.

- [ ] Investigate   - [ ] Ignore

**Your comments:**

> 

### LC-5

`avgsn/avgrn/finalmig` (2720-2722) are weighted by cumulative `totmig` (which includes earlier DEN) and pooled across cohorts. This only affects the box-0 diagnostic storage.

- [ ] Investigate   - [ ] Ignore

**Your comments:**

> 

### LC-6

`Get_Vertical_Distribution()` non-mobile branch with cohort == -1 uses cohorts 0 and 1 (`currentden[species][juv_id/adult_id]`) as the juvenile/adult profiles.

- [ ] Investigate   - [ ] Ignore

**Your comments:**

> 

### LC-7

For land boxes in the recruit_hdistrib update (2310-2328), the non-BOUNDARY layer loop also runs (its LAND exclusion is commented out), so land-box adults may be counted twice.

- [ ] Investigate   - [ ] Ignore

**Your comments:**

> 

### LC-8

In the invert enviro checks (3397-3463), only `newden[..][0][ij]` is used afterwards, so scalars applied for layers k > 0 have no effect. Only bottom-layer conditions matter (partly acknowledged by the TODO).

- [ ] Investigate   - [ ] Ignore

**Your comments:**

> 

### LC-9

`Get_Ice_Presence(..., ij, 0, ...)` at 3161/3446 uses layer 0 instead of k.

- [ ] Investigate   - [ ] Ignore

**Your comments:**

> 

### LC-10

Division risks: `mum_SP` = 0 (398), `MinYearsAway` = 0 (1214), `x*(1-IOBox)+y` (168).

- [ ] Investigate   - [ ] Ignore

**Your comments:**

> 

### LC-11

`preyamt` is allocated with only 2 stages (atbiolsetup.c:2919) and indexed by `preystage` up to numStages-1. That is fine while K_num_max_stages <= 2.

- [ ] Investigate   - [ ] Ignore

**Your comments:**

> 
