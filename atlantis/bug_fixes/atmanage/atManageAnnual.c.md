# atmanage/atManageAnnual.c — bug review

Generated 2026-11-19 from `atlantis/to-do-fixt-list.md`. **23 findings** (severity H: 11; confidence H and severity H: 11; on the priority list: 11), plus 3 lower-confidence notes.

Each finding's **Code at issue** is located by searching the current code for the code quoted in the review, not by line number. Use the **Search for** text with your editor's Find, because line numbers will keep changing as fixes go in. → marks the matching line. If only a close match or a name could be found, the block says so. ⚠ means the quoted code is no longer in the file: it may have been changed or already fixed. **Lines at review** are the October numbers. Line numbers in the description text are approximate.

Conf/Sev: confidence it's a bug / severity if it is (H = wrong results or crash, M = wrong in some configs, L = cosmetic, log, leak or latent). ✔ = the reviewer re-checked it by hand. ⭐ = on the high-priority list.

## Summary

| ID | Found at | Conf/Sev | Category | Short description | Decision | Done |
|---|---|---|---|---|---|---|
| [MNGA-01](#mnga-01) ⭐ | 334, 12 | H/H | wrong variable / OOB | `spTotCumCatch = Harvest_Get_TotCumCatch(sp, nf, bm->thisyear);` is called outside any `nf` loop. `nf` is stal… |  | [ ] |
| [MNGA-02](#mnga-02) ⭐ | 366, 12 | H/H | wrong index / OOB | `FunctGroupArray[sp].co_sp_catch[nf][co_sp2]`: `co_sp_catch` is allocated `Util_Alloc_Init_2D_Double(K_max_co_… |  | [ ] |
| [MNGA-03](#mnga-03) | 12 | M/M | logic | `co_sp2_TotCumCatch` is reset to 0 and only set when `flagdyn_coupdate` is on. With `flagdyn_coupdate==0` (sta… |  | [ ] |
| [MNGA-04](#mnga-04) | 344, 12 | H/L | wrong index (log) | `FunctGroupArray[co_sp].groupCode`: `co_sp` is the companion slot index (0..max_co_sp-1), not a group id, so t… |  | [ ] |
| [MNGA-05](#mnga-05) ⭐ | 794 | H/H | logic (stuck counter) | `tac_resetcount_id` is incremented only at line 1001. Line 604 resets it to 0, and the early `return` at 898-9… |  | [ ] |
| [MNGA-06](#mnga-06) ⭐ | 794 | H/H | OOB | `yr_id = floor(FC_period/364 + 0.5) + bm->thisyear; LeverQueue[sp][nf][yr_id] = ...`. LeverQueue is `[sp][nf][… |  | [ ] |
| [MNGA-07](#mnga-07) | 794 | M/L | inverted condition | The comment says "If phase out date elapsed turn off" but the code is `if (phase_out > dayt) phased_out = 0; e… |  | [ ] |
| [MNGA-08](#mnga-08) ⭐ | 1382 | H/H | accumulation error | `Braw += totfishpop[othersp]...; Bcurr += Assess_Add_Error(bm, er_case, Braw, ...)`. `Braw` is cumulative (pri… |  | [ ] |
| [MNGA-09](#mnga-09) ⭐ | 1393, 1397 | H/H | off-by-one / wrong bound | Both loops run `groupIndex < max_co_sp` and pick `othersp = sp` only when `groupIndex >= K_max_co_sp`. Because… |  | [ ] |
| [MNGA-10](#mnga-10) ⭐ | 1424, 1427 | H/H | wrong variable / OOB write | `for (groupIndex = 0; groupIndex < K_max_co_sp + 1; ...) Fcurr += ...; Fcurr_sp[groupIndex] /= counter;`. Afte… |  | [ ] |
| [MNGA-11](#mnga-11) | 1413 | H/L | memory leak | `calcM_sp`, `calcF_sp`, `counter_sp` and `Fcurr_sp` are allocated on every call and never freed. Function: `Gu… |  | [ ] |
| [MNGA-12](#mnga-12) | 1429 | M/M | inconsistent tier dispatch | `if (tier < tier8)` sends dyntier4(10), dyntier1B0(11), sp_rollover(12), tier13 and tier14 to the tier8/9 (Fre… |  | [ ] |
| [MNGA-13](#mnga-13) ⭐ | 1623, 1121 | H/H | uninitialised accumulator | `Braw` is declared at 1527 without initialisation and accumulated with `Braw += ...` in the SSB branch. It is … |  | [ ] |
| [MNGA-14](#mnga-14) ⭐ | 1865 | H/H | state not reset in loop | `mFC` is computed once per fishery (1802), but inside the cohort loop `mFC = 0` is assigned when a cohort is o… |  | [ ] |
| [MNGA-15](#mnga-15) ⭐ | 1917 | H/H | switch fall-through | `case assess_M_est:` has no `break`, so it always falls into `default: quit(...)`. It also sets `M` rather tha… |  | [ ] |
| [MNGA-16](#mnga-16) | 1940 | H/M | `=` vs `+=` | Invertebrate branch: `for (k...) this_Biom = calcTrackedMort[sp][nc][k][start_id] * ...;` overwrites on each s… |  | [ ] |
| [MNGA-17](#mnga-17) | 1930 | H/M | stale / uninitialised value | `this_expect_catch` is only assigned when `expectF > 0`, but line 1960 always adds it to `sp_fishery_expected_… |  | [ ] |
| [MNGA-18](#mnga-18) | 1828 | H/M | accumulator reset in wrong loop | `speciesParams[sp_fishery_expected_catch_id] = 0` sits inside the `nf` loop, so each fishery wipes the previou… |  | [ ] |
| [MNGA-19](#mnga-19) | 1528 | M/M | missing bound | `rescale_scalar = (orig - deductions)/orig` goes negative when a species' deduction share exceeds its expected… |  | [ ] |
| [MNGA-20](#mnga-20) | 1100 | H/L | copy-paste (message) | The quit messages say `Per_Sp_Frescale:` inside `Ecosystem_Cap_Frescale()`. |  | [ ] |
| [MNGA-21](#mnga-21) | 2041, 788 | H/L | off-by-one | `if ((co_sp < 0) \|\| (co_sp > bm->K_num_tot_sp)) continue;` lets `co_sp == K_num_tot_sp` through, which indexes… |  | [ ] |
| [MNGA-22](#mnga-22) ⭐ | 2139 | H/H | accumulators never filled / not reset | `avg_scale`, `num_scale` and `min_scale` are initialised per fleet but never updated from `lever_scale`. So `n… |  | [ ] |
| [MNGA-23](#mnga-23) | 187 | M/M | wrong expected value | The post-scaling check compares `newtoteffort` with `(1.0 - effort_scale) * toteffort` whether effort was scal… |  | [ ] |

---

## MNGA-01

**wrong variable / OOB** · Conf H / Sev H ⭐ priority  
**Found at (current code):** `atManageAnnual.c` 334, `atManageAnnual.c` 12
**Lines at review (Oct 2026):** 333

**Function:** `Make_Mgmt_Decisions()`

**Why it's thought to be a bug**

`spTotCumCatch = Harvest_Get_TotCumCatch(sp, nf, bm->thisyear);` is called outside any `nf` loop. `nf` is stale: `K_num_fisheries` (left over from the loop at 286 or the inner loop at 348), so it reads `TotCumCatch[sp][K_num_fisheries][yr]`, past the end of the fishery dimension. It also gives a per-fleet catch ratio that does not depend on fleet, while `co_sp2_TotCumCatch` (358) is per fleet. Function: `Make_Mgmt_Decisions()`.

**Code at issue** — `atManageAnnual.c` line 334  
Search for: `spTotCumCatch = Harvest_Get_TotCumCatch(sp, nf, bm->thisyear)`

```c
  332:             
  333:             co_TYPE = (int) (FunctGroupArray[sp].speciesParams[coType_id]);
  334→             spTotCumCatch = Harvest_Get_TotCumCatch(sp, nf, bm->thisyear);
  335: 
  336:             for (co_sp = 0; co_sp < FunctGroupArray[sp].speciesParams[max_co_sp_id]; co_sp++ ) {
```

**Code at issue** — `atManageAnnual.c` line 12  
Search for: `Make_Mgmt_Decisions()`

```c
   10:  for reporting aggregate fisheries statistics. Also expanded Annual_Fisheries_Mgmt()
   11:  and Apply_Annual_Fisheries_Mgmt() to set up, execute or faciltate
   12→  these new routines. Also added Make_Mgmt_Decisions() to handle dynamic TAC setting.
   13: 
   14:  1/7/2006 Added bm->FCtsCarryOver so can correct for any immediate shortfalls
```

**Suggested fix (from review):** Move the call inside the `for (nf...)` loop at 348, or sum over nf.

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

## MNGA-02

**wrong index / OOB** · Conf H / Sev H ⭐ priority  
**Found at (current code):** `atManageAnnual.c` 366, `atManageAnnual.c` 12
**Lines at review (Oct 2026):** 365, 371

**Function:** `Make_Mgmt_Decisions()`

**Why it's thought to be a bug**

`FunctGroupArray[sp].co_sp_catch[nf][co_sp2]`: `co_sp_catch` is allocated `Util_Alloc_Init_2D_Double(K_max_co_sp, K_num_fisheries)` (atManageParamIO.c:933, so `[nf][K_max_co_sp]`) and filled as `co_sp_catch[nf][co_sp]` using the slot index (atManageParamIO.c:306). `co_sp2` is a functional-group id (up to K_num_tot_sp-1), so this reads out of bounds or picks the wrong ratio. Function: `Make_Mgmt_Decisions()`.

**Code at issue** — `atManageAnnual.c` line 366  
Search for: `FunctGroupArray[sp].co_sp_catch[nf][co_sp2]`

```c
  364:                     } else {
  365:                         /* Always use initial (read-in) catch ratios */
  366→                         FC2_ratio = FunctGroupArray[sp].co_sp_catch[nf][co_sp2];
  367:                     }
  368:                     spA_TAC = bm->TACamt[sp][nf][now_id];
```

**Code at issue** — `atManageAnnual.c` line 12  
Search for: `Make_Mgmt_Decisions()`

```c
   10:  for reporting aggregate fisheries statistics. Also expanded Annual_Fisheries_Mgmt()
   11:  and Apply_Annual_Fisheries_Mgmt() to set up, execute or faciltate
   12→  these new routines. Also added Make_Mgmt_Decisions() to handle dynamic TAC setting.
   13: 
   14:  1/7/2006 Added bm->FCtsCarryOver so can correct for any immediate shortfalls
```

**Suggested fix (from review):** Use `co_sp_catch[nf][co_sp]` in both places.

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

## MNGA-03

**logic** · Conf M / Sev M  
**Found at (current code):** `atManageAnnual.c` 12
**Lines at review (Oct 2026):** 354-382

**Function:** `Make_Mgmt_Decisions()`

**Why it's thought to be a bug**

`co_sp2_TotCumCatch` is reset to 0 and only set when `flagdyn_coupdate` is on. With `flagdyn_coupdate==0` (static read-in ratios), `if (!co_sp2_TotCumCatch && spC_TAC) dont_scale2 = 1;` is true whenever the companion has a TAC, so companion TACs are never adjusted in static-ratio mode. Function: `Make_Mgmt_Decisions()`.

**Code at issue** — `atManageAnnual.c` line 12  
Search for: `Make_Mgmt_Decisions()`

```c
   10:  for reporting aggregate fisheries statistics. Also expanded Annual_Fisheries_Mgmt()
   11:  and Apply_Annual_Fisheries_Mgmt() to set up, execute or faciltate
   12→  these new routines. Also added Make_Mgmt_Decisions() to handle dynamic TAC setting.
   13: 
   14:  1/7/2006 Added bm->FCtsCarryOver so can correct for any immediate shortfalls
```

**Suggested fix (from review):** Always fetch `co_sp2_TotCumCatch` (or only apply the zero-catch guard when `flagdyn_coupdate` is set).

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

## MNGA-04

**wrong index (log)** · Conf H / Sev L  
**Found at (current code):** `atManageAnnual.c` 344, `atManageAnnual.c` 12
**Lines at review (Oct 2026):** 342-345

**Function:** `Make_Mgmt_Decisions()`

**Why it's thought to be a bug**

`FunctGroupArray[co_sp].groupCode`: `co_sp` is the companion slot index (0..max_co_sp-1), not a group id, so the log names the wrong group. Also `co_sp2 > 0` should be `>= 0` (group 0 is valid; -1 was already skipped at 339). Function: `Make_Mgmt_Decisions()`.

**Code at issue** — `atManageAnnual.c` line 344  
Search for: `FunctGroupArray[co_sp].groupCode`

```c
  342: 
  343:                 if (co_sp2 > 0) {
  344→                     fprintf(llogfp, "Time: %e, looking at %s vs co_sp: %s, co_sp2: %s, flagdyn_coupdate: %d\n", bm->dayt, FunctGroupArray[sp].groupCode, FunctGroupArray[co_sp].groupCode, FunctGroupArray[co_sp2].groupCode, bm->flagdyn_coupdate);
  345:                 } else {
  346:                     fprintf(llogfp, "Time: %e, looking at %s vs co_sp: %s, flagdyn_coupdate: %d\n", bm->dayt, FunctGroupArray[sp].groupCode, FunctGroupArray[co_sp].groupCode, bm->flagdyn_coupdate);
```

**Code at issue** — `atManageAnnual.c` line 12  
Search for: `Make_Mgmt_Decisions()`

```c
   10:  for reporting aggregate fisheries statistics. Also expanded Annual_Fisheries_Mgmt()
   11:  and Apply_Annual_Fisheries_Mgmt() to set up, execute or faciltate
   12→  these new routines. Also added Make_Mgmt_Decisions() to handle dynamic TAC setting.
   13: 
   14:  1/7/2006 Added bm->FCtsCarryOver so can correct for any immediate shortfalls
```

**Suggested fix (from review):** Print `FunctGroupArray[co_sp2].groupCode`; drop the `>0` test.

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

## MNGA-05

**logic (stuck counter)** · Conf H / Sev H ⭐ priority  
**Found at (current code):** `atManageAnnual.c` 794
**Lines at review (Oct 2026):** 601-605, 898-900, 1001

**Function:** `AMS_Tiered_Assessment()`

**Why it's thought to be a bug**

`tac_resetcount_id` is incremented only at line 1004. Line 607 resets it to 0, and the early `return` at 898-900 (TAC change < `TAC_crit_change`) skips the increment. Line 604 returns while `count < period` without incrementing. Once a year's RBC falls inside `TAC_crit_change`, the counter stays at 0 and `0 < tac_resetperiod` (>=1) holds every year, so that species' TAC is never reassessed for the rest of the run. Function: `AMS_Tiered_Assessment()`.

**Code at issue** — `atManageAnnual.c` line 794  
Search for: `AMS_Tiered_Assessment()`

```c
  792:         case tier13:
  793:             RBC = 0.0;
  794→             quit("Need to add code for this tier in AMS_Tiered_Assessment()\n");
  795:             break;
  796: 		default:
```

**Suggested fix (from review):** Increment the counter at line 604 before returning, and at the 898 return (or move the increment to just after 605).

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

## MNGA-06

**OOB** · Conf H / Sev H ⭐ priority  
**Found at (current code):** `atManageAnnual.c` 794
**Lines at review (Oct 2026):** 766-777 (alloc atManageSetup.c:912-913)

**Function:** `AMS_Tiered_Assessment()`

**Why it's thought to be a bug**

`yr_id = floor(FC_period/364 + 0.5) + bm->thisyear; LeverQueue[sp][nf][yr_id] = ...`. LeverQueue is `[sp][nf][num_years]` with `num_years = ceil((tstop-tstart)/1 yr)`. In the last year(s) of the run, `yr_id >= num_years`, so this writes past the end of the array (heap corruption) whenever tier6 is used. Function: `AMS_Tiered_Assessment()`.

**Code at issue** — `atManageAnnual.c` line 794  
Search for: `AMS_Tiered_Assessment()`

```c
  792:         case tier13:
  793:             RBC = 0.0;
  794→             quit("Need to add code for this tier in AMS_Tiered_Assessment()\n");
  795:             break;
  796: 		default:
```

**Suggested fix (from review):** Skip or clamp when `yr_id >= num_years` (store num_years globally), or allocate num_years + max lag.

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

## MNGA-07

**inverted condition** · Conf M / Sev L  
**Found at (current code):** `atManageAnnual.c` 794
**Lines at review (Oct 2026):** 909-921

**Function:** `AMS_Tiered_Assessment()`

**Why it's thought to be a bug**

The comment says "If phase out date elapsed turn off" but the code is `if (phase_out > dayt) phased_out = 0; else if (phase_start > dayt) {ramp}`. Both comparisons are reversed: a future phase-out date zeroes the TAC now, and the ramp runs before phase_start. Latent only, because `phase_out_id`/`phase_start_id` are never read in (they stay 0, so phased_out=1). The ramp also divides by `period` with no zero check. Function: `AMS_Tiered_Assessment()`.

**Code at issue** — `atManageAnnual.c` line 794  
Search for: `AMS_Tiered_Assessment()`

```c
  792:         case tier13:
  793:             RBC = 0.0;
  794→             quit("Need to add code for this tier in AMS_Tiered_Assessment()\n");
  795:             break;
  796: 		default:
```

**Suggested fix (from review):** `if (dayt >= phase_out) 0; else if (dayt > phase_start) ramp; else 1`.

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

## MNGA-08

**accumulation error** · Conf H / Sev H ⭐ priority  
**Found at (current code):** `atManageAnnual.c` 1382
**Lines at review (Oct 2026):** 1378-1387

**Function:** `Guild_Frescale()`

**Why it's thought to be a bug**

`Braw += totfishpop[othersp]...; Bcurr += Assess_Add_Error(bm, er_case, Braw, ...)`. `Braw` is cumulative (primary plus all previous guild members), and each step adds the error-applied *cumulative* biomass to Bcurr. The guild biomass is massively overcounted (primary counted n+1 times, etc.), which skews the HCR toward FrefA/FrefH. Function: `Guild_Frescale()`.

**Code at issue** — `atManageAnnual.c` line 1382  
Search for: `Braw += bm->totfishpop[othersp]`

```c
 1380:         othersp = FunctGroupArray[sp].co_sp[groupIndex];
 1381:     
 1382→         Braw += bm->totfishpop[othersp] * bm->X_CN * mg_2_tonne;
 1383:         Bcurr += Assess_Add_Error(bm, er_case, Braw, est_bias, est_cv);
 1384:     
```

**Suggested fix (from review):** Use a per-species `double Bsp = totfishpop[othersp]*X_CN*mg_2_tonne; Braw += Bsp; Bcurr += Assess_Add_Error(..., Bsp, ...)`.

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

## MNGA-09

**off-by-one / wrong bound** · Conf H / Sev H ⭐ priority  
**Found at (current code):** `atManageAnnual.c` 1393, `atManageAnnual.c` 1397
**Lines at review (Oct 2026):** 1392-1397, 1482-1488

**Function:** `Guild_Frescale()`

**Why it's thought to be a bug**

Both loops run `groupIndex < max_co_sp` and pick `othersp = sp` only when `groupIndex >= K_max_co_sp`. Because max_co_sp <= K_max_co_sp (co_sp has K_max_co_sp entries), the primary species is never processed. Its Fcurr is not included, and in the apply loop its `mFC_scale` is never set to F_rescale and `done_Co_sp_id` is never set. The comments ("+1 is for the primary species") show it should be included. Function: `Guild_Frescale()`.

**Code at issue** — `atManageAnnual.c` line 1393 (closest match to the quoted code; wording differs slightly)  
Search for: `for (groupIndex = 0; groupIndex < FunctGroupArray[sp].speciesParams[max_co_sp_id]; groupIn`

```c
 1391:     
 1392:     /** Using perfect knowledge way of determining M and F - averaging over the species in the guild */
 1393→     for (groupIndex = 0; groupIndex < FunctGroupArray[sp].speciesParams[max_co_sp_id]; groupIndex++ ) {
 1394:         if (groupIndex < bm->K_max_co_sp) {
 1395:             othersp = FunctGroupArray[sp].co_sp[groupIndex];
```

**Code at issue** — `atManageAnnual.c` line 1397  
Search for: `othersp = sp`

```c
 1395:             othersp = FunctGroupArray[sp].co_sp[groupIndex];
 1396:         } else {
 1397→             othersp = sp;
 1398:         }
 1399:         
```

**Suggested fix (from review):** Loop `groupIndex <= max_co_sp` and use `othersp = (groupIndex < max_co_sp) ? co_sp[groupIndex] : sp`.

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

## MNGA-10

**wrong variable / OOB write** · Conf H / Sev H ⭐ priority  
**Found at (current code):** `atManageAnnual.c` 1424, `atManageAnnual.c` 1427
**Lines at review (Oct 2026):** 1421-1426

**Function:** `Guild_Frescale()`

**Why it's thought to be a bug**

`for (groupIndex = 0; groupIndex < K_max_co_sp + 1; ...) Fcurr += ...; Fcurr_sp[groupIndex] /= counter;`. After the loop `groupIndex == K_max_co_sp+1`, so this writes one past the end of `Fcurr_sp` (allocated K_max_co_sp+1 at 1357), and `Fcurr` is never averaged: it stays a sum. The divisor also counts unused slots rather than guild members. Function: `Guild_Frescale()`.

**Code at issue** — `atManageAnnual.c` line 1424  
Search for: `for (groupIndex = 0; groupIndex < bm->K_max_co_sp + 1`

```c
 1422:     Fcurr = 0.0;
 1423:     counter = (double)(bm->K_max_co_sp + 1);
 1424→     for (groupIndex = 0; groupIndex < bm->K_max_co_sp + 1; groupIndex++) {
 1425:         Fcurr += Fcurr_sp[groupIndex];
 1426:     }
```

**Code at issue** — `atManageAnnual.c` line 1427 (closest match to the quoted code; wording differs slightly)  
Search for: `Fcurr_sp[groupIndex] /= counter;`

```c
 1425:         Fcurr += Fcurr_sp[groupIndex];
 1426:     }
 1427→     Fcurr_sp[groupIndex] /= counter;
 1428: 
 1429:     if (tier < tier8) {  // So not US (tier8) or Norway (tier 9)
```

**Suggested fix (from review):** `Fcurr /= (number of species actually summed);`.

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

## MNGA-11

**memory leak** · Conf H / Sev L  
**Found at (current code):** `atManageAnnual.c` 1413
**Lines at review (Oct 2026):** 1354-1357

**Function:** `Guild_Frescale()`

**Why it's thought to be a bug**

`calcM_sp`, `calcF_sp`, `counter_sp` and `Fcurr_sp` are allocated on every call and never freed. Function: `Guild_Frescale()`.

**Code at issue** — `atManageAnnual.c` line 1413 (located by name only; check this is the right place)  
Search for: `calcM_sp[groupIndex] /= counter_sp[groupIndex];`

```c
 1411:             }
 1412:         }
 1413→         calcM_sp[groupIndex] /= counter_sp[groupIndex];
 1414:         calcF_sp[groupIndex] /= counter_sp[groupIndex];
 1415:         Fcurr_sp[groupIndex] = calcF_sp[groupIndex];
```

**Suggested fix (from review):** Add `free1d()` for all four before return.

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

## MNGA-12

**inconsistent tier dispatch** · Conf M / Sev M  
**Found at (current code):** `atManageAnnual.c` 1429
**Lines at review (Oct 2026):** 1428

**Function:** `Guild_Frescale()`

**Why it's thought to be a bug**

`if (tier < tier8)` sends dyntier4(10), dyntier1B0(11), sp_rollover(12), tier13 and tier14 to the tier8/9 (FrefH) broken stick. `Per_Sp_Frescale()`/`Ecosystem_Cap_Frescale()` treat 10-12 and 14 as the tier1 stick and tier13 as escapement. The F_rescale branch at 1465 is also tier8-specific. Function: `Guild_Frescale()`.

**Code at issue** — `atManageAnnual.c` line 1429  
Search for: `if (tier < tier8)`

```c
 1427:     Fcurr_sp[groupIndex] /= counter;
 1428: 
 1429→     if (tier < tier8) {  // So not US (tier8) or Norway (tier 9)
 1430:         /* Tier 1 - Best quantitative assessment available */
 1431:         if (Bcurr >= BrefA) {
```

**Suggested fix (from review):** Use the same `switch(tier)` as `Per_Sp_Frescale()`.

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

## MNGA-13

**uninitialised accumulator** · Conf H / Sev H ⭐ priority  
**Found at (current code):** `atManageAnnual.c` 1623, `atManageAnnual.c` 1121
**Lines at review (Oct 2026):** 1527, 1617-1631

**Function:** `Ecosystem_Cap_Frescale()`

**Why it's thought to be a bug**

`Braw` is declared at 1527 without initialisation and accumulated with `Braw += ...` in the SSB branch. It is never reset per species, so the first species uses garbage and later species include all previous species' SSB. (`Per_Sp_Frescale()` initialises `Braw = 0.0` per call.) Function: `Ecosystem_Cap_Frescale()`.

**Code at issue** — `atManageAnnual.c` line 1623  
Search for: `Braw +=`

```c
 1621:                         for (ij = 0; ij < bm->nbox; ij++) {
 1622:                             for (b = 0; b < bm->boxes[ij].nz; b++) {
 1623→                                  Braw += ((bm->boxes[ij].tr[b][FunctGroupArray[sp].structNTracers[cohort]] + bm->boxes[ij].tr[b][FunctGroupArray[sp].resNTracers[cohort]]) *
 1624:                                  bm->boxes[ij].tr[b][FunctGroupArray[sp].NumsTracers[cohort]] *
 1625:                                  FunctGroupArray[sp].habitatCoeffs[WC] *
```

**Code at issue** — `atManageAnnual.c` line 1121  
Search for: `Braw = 0.0`

```c
 1119:     double FrefLim = FunctGroupArray[sp].speciesParams[FrefLim_id];
 1120: 
 1121→     double Braw = 0.0, Bcurr;
 1122: 
 1123:     if(!do_assess) {  // Where do_assess set at atlantismain.c level as requires Assess_Resources() call in atasseess lib
```

**Suggested fix (from review):** Set `Braw = 0.0;` at the top of the per-species block (before line 1616).

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

## MNGA-14

**state not reset in loop** · Conf H / Sev H ⭐ priority  
**Found at (current code):** `atManageAnnual.c` 1865
**Lines at review (Oct 2026):** 1802, 1864-1866

**Function:** `Ecosystem_Cap_Frescale()`

**Why it's thought to be a bug**

`mFC` is computed once per fishery (1805), but inside the cohort loop `mFC = 0` is assigned when a cohort is outside [mFC_start_age, mFC_end_age). Cohort 0 is normally younger than mFC_start_age, so mFC is zeroed at nc=0 and stays 0 for every older cohort: the expected catch is 0 and the system cap never binds (when `flag_sel_with_mFC==0`). Function: `Ecosystem_Cap_Frescale()`.

**Code at issue** — `atManageAnnual.c` line 1865 (the line the review pointed to, found by its text)  
Search for: `if((basechrt < mFC_start_age) || (basechrt >= mFC_end_age))`

```c
 1863:                             /* Only put this filter on if not using selectivity as the filter */
 1864:                             if(FunctGroupArray[sp].isVertebrate == TRUE || FunctGroupArray[sp].groupAgeType == AGE_STRUCTURED_BIOMASS){
 1865→                                 if((basechrt < mFC_start_age) || (basechrt >= mFC_end_age))
 1866:                                 /* Actually too young to be caught so don't apply the mortality here */
 1867:                                     mFC = 0;
```

**Suggested fix (from review):** Use a per-cohort local, e.g. `double mFC_c = in_age_range ? mFC : 0;`, and use it at 1923/1957.

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

## MNGA-15

**switch fall-through** · Conf H / Sev H ⭐ priority  
**Found at (current code):** `atManageAnnual.c` 1917
**Lines at review (Oct 2026):** 1910-1917

**Function:** `Ecosystem_Cap_Frescale()`

**Why it's thought to be a bug**

`case assess_M_est:` has no `break`, so it always falls into `default: quit(...)`. It also sets `M` rather than `calcM`, so calcM would be uninitialised or stale anyway. The quit text names `syst_cap_calc_method` rather than `M_est_method`. Function: `Ecosystem_Cap_Frescale()`.

**Code at issue** — `atManageAnnual.c` line 1917  
Search for: `default:`

```c
 1915:                                         M = bm->NAssess[sp][est_M_id];
 1916:                                     }
 1917→                                 default:
 1918:                                     quit("No such option for syst_cap_calc_method. Please set to 0 for fixed M (read in from harvest.prm), 1 for M = Z-F, 2 for fully dynamic estimation of M given in model predation, or 3 for M estimated in an assessment\n");
 1919:                             }
```

**Suggested fix (from review):** `calcM = bm->NAssess[sp][est_M_id]; break;` and fix the message.

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

## MNGA-16

**`=` vs `+=`** · Conf H / Sev M  
**Found at (current code):** `atManageAnnual.c` 1940
**Lines at review (Oct 2026):** 1938-1940

**Function:** `Ecosystem_Cap_Frescale()`

**Why it's thought to be a bug**

Invertebrate branch: `for (k...) this_Biom = calcTrackedMort[sp][nc][k][start_id] * ...;` overwrites on each stock, so only the last stock's biomass is used (the vertebrate branch at 1933 accumulates). Function: `Ecosystem_Cap_Frescale()`.

**Code at issue** — `atManageAnnual.c` line 1940 (closest match to the quoted code; wording differs slightly)  
Search for: `this_Biom = bm->calcTrackedMort[sp][nc][k][start_id] * bm->X_CN * mg_2_tonne;`

```c
 1938:                                     this_Biom = 0.0;
 1939:                                     for (k = 0; k < maxstock; k++) {
 1940→                                         this_Biom = bm->calcTrackedMort[sp][nc][k][start_id] * bm->X_CN * mg_2_tonne;
 1941:                                     }
 1942:                                 }
```

**Suggested fix (from review):** Change to `this_Biom +=`.

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

## MNGA-17

**stale / uninitialised value** · Conf H / Sev M  
**Found at (current code):** `atManageAnnual.c` 1930
**Lines at review (Oct 2026):** 1929-1944, 1960

**Function:** `Ecosystem_Cap_Frescale()`

**Why it's thought to be a bug**

`this_expect_catch` is only assigned when `expectF > 0`, but line 1963 always adds it to `sp_fishery_expected_catch_id`. Cohorts with zero F add the previous cohort's (or previous species', or on first use uninitialised) value. Function: `Ecosystem_Cap_Frescale()`.

**Code at issue** — `atManageAnnual.c` line 1930  
Search for: `expectF > 0`

```c
 1928:                             // Assume all stocks and genetypes being pooled to get the Numbers at age
 1929:                             // TODO: Do per Stock and geneotype separately
 1930→                             if(expectF > 0) { // Only do this if non-zero F to avoid time wasting loops
 1931:                                 if (FunctGroupArray[sp].isVertebrate == TRUE) {
 1932:                                     this_Num = 0.0;
```

**Suggested fix (from review):** Set `this_expect_catch = 0.0;` at the top of the cohort loop.

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

## MNGA-18

**accumulator reset in wrong loop** · Conf H / Sev M  
**Found at (current code):** `atManageAnnual.c` 1828
**Lines at review (Oct 2026):** 1827

**Function:** `Ecosystem_Cap_Frescale()`

**Why it's thought to be a bug**

`speciesParams[sp_fishery_expected_catch_id] = 0` sits inside the `nf` loop, so each fishery wipes the previous fisheries' expected catch. Only the last flagF fishery's catch survives, while `tot_expect_catch` sums all fisheries. The per-species deduction scalar at 1981-1984 is therefore wrong in multi-fleet models. Function: `Ecosystem_Cap_Frescale()`.

**Code at issue** — `atManageAnnual.c` line 1828  
Search for: `speciesParams[sp_fishery_expected_catch_id] = 0`

```c
 1826: 
 1827:                     sel_curve = (int) (bm->FISHERYprms[nf][selcurve_id]);
 1828→                     FunctGroupArray[sp].speciesParams[sp_fishery_expected_catch_id] = 0;
 1829:                     for (nc = 0; nc < FunctGroupArray[sp].numCohortsXnumGenes; nc++) {
 1830:                         basechrt = nc / FunctGroupArray[sp].numGeneTypes;
```

**Suggested fix (from review):** Move the reset to before `for (nf...)` at 1781.

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

## MNGA-19

**missing bound** · Conf M / Sev M  
**Found at (current code):** `atManageAnnual.c` 1528
**Lines at review (Oct 2026):** 1982-1988

**Function:** `Ecosystem_Cap_Frescale()`

**Why it's thought to be a bug**

`rescale_scalar = (orig - deductions)/orig` goes negative when a species' deduction share exceeds its expected catch, giving a negative `mFC_scale`. Function: `Ecosystem_Cap_Frescale()`.

**Code at issue** — `atManageAnnual.c` line 1528 (located by name only; check this is the right place)  
Search for: `double max_mFC, F_rescale, FTARG, Bcurr, calcM, survival, Fcurr, calcF, Fstep1, this_mFC, `

```c
 1526: void Ecosystem_Cap_Frescale(MSEBoxModel *bm, FILE *llogfp) {
 1527:     int sp, nf, nc, cohort, ij, b, k, flagF, tier, er_case, maxstock, mFC_end_age, mFC_start_age, flagfcmpa, sel_curve, stage, basechrt;
 1528→     double max_mFC, F_rescale, FTARG, Bcurr, calcM, survival, Fcurr, calcF, Fstep1, this_mFC, M, est_bias, est_cv, BrefA, BrefB, BrefE, Blim, FrefA, FrefH, FrefLim, Braw, sel, this_expect_catch, sp_fishery_pref_weight, w_inv, tot_w_inv, counter, mFC, mFC_change_scale, mpa_scale, mpa_infringe, Wgt, li, gear_change_scale, this_Num, this_start, this_end, this_Biom, Z_Est, expectF, Catch_Eqn_Denom, orig_expected_catch, excess, deductions, new_expected_catch, rescale_scalar,tot_area, fishable_area;
 1529:     //double calcM;
 1530:     
```

**Suggested fix (from review):** Clamp `rescale_scalar` to >= 0 (and redistribute the remainder).

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

## MNGA-20

**copy-paste (message)** · Conf H / Sev L  
**Found at (current code):** `atManageAnnual.c` 1100
**Lines at review (Oct 2026):** 1734, 1771

**Why it's thought to be a bug**

The quit messages say `Per_Sp_Frescale:` inside `Ecosystem_Cap_Frescale()`.

**Code at issue** — `atManageAnnual.c` line 1100 (located by name only; check this is the right place)  
Search for: `void Per_Sp_Frescale (MSEBoxModel *bm, FILE *llogfp, int sp) {`

```c
 1098: *
 1099: *******************************************************************************/
 1100→ void Per_Sp_Frescale (MSEBoxModel *bm, FILE *llogfp, int sp) {
 1101:     int cohort, ij, b, nf, flagF, nc, k;
 1102:     double FTARG, F_rescale, Fcurr, calcF, Fstep1, this_mFC, counter;
```

**Suggested fix (from review):** Fix the function name in the text.

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

## MNGA-21

**off-by-one** · Conf H / Sev L  
**Found at (current code):** `atManageAnnual.c` 2041, `atManageAnnual.c` 788
**Lines at review (Oct 2026):** 2040

**Function:** `Check_Ecosystem_F_Harvest_Control_Rule()`

**Why it's thought to be a bug**

`if ((co_sp < 0) || (co_sp > bm->K_num_tot_sp)) continue;` lets `co_sp == K_num_tot_sp` through, which indexes FunctGroupArray out of bounds. Function: `Check_Ecosystem_F_Harvest_Control_Rule()`.

**Code at issue** — `atManageAnnual.c` line 2041  
Search for: `if ((co_sp < 0) || (co_sp > bm->K_num_tot_sp))`

```c
 2039: 								co_sp = (int)(FunctGroupArray[sp].co_sp[i]);
 2040:                                 
 2041→                                 if ((co_sp < 0) || (co_sp > bm->K_num_tot_sp))
 2042:                                     continue;
 2043: 
```

**Code at issue** — `atManageAnnual.c` line 788  
Search for: `Check_Ecosystem_F_Harvest_Control_Rule()`

```c
  786: 		case tier7:
  787: 			RBC = 0.0;	/* Deal with comiler warnings - RBC will be used uninitialized later.*/
  788→ 					// Do nothing as its for cross species rules see Check_Ecosystem_F_Harvest_Control_Rule();
  789: 			break;
  790:         case tier8:
```

**Suggested fix (from review):** Use `>=`.

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

## MNGA-22

**accumulators never filled / not reset** · Conf H / Sev H ⭐ priority  
**Found at (current code):** `atManageAnnual.c` 2139
**Lines at review (Oct 2026):** 2069-2139

**Function:** `Do_MSY_Management()`

**Why it's thought to be a bug**

`avg_scale`, `num_scale` and `min_scale` are initialised per fleet but never updated from `lever_scale`. So `new_scale` is 0 (avg) or MAXDOUBLE (min), and `effort_scale[nf][target_tac_id]` / season length are set to 0 or MAXDOUBLE whenever an effort, space or season lever is used. Also `update_scale`/`update_season` are initialised once outside the `nf` loop, so once set by one fleet they apply to every later fleet. Function: `Do_MSY_Management()`.

**Code at issue** — `atManageAnnual.c` line 2139  
Search for: `effort_scale[nf][target_tac_id]`

```c
 2137: 		/* Update effort and spatial management */
 2138: 		if (update_scale) {
 2139→ 			effort_scale[nf][target_tac_id] = new_scale;
 2140: 		}
 2141: 
```

**Suggested fix (from review):** In effort/space/season cases do `avg_scale += lever_scale; num_scale++; min_scale = min(min_scale, lever_scale);`, and reset `update_scale = update_season = 0` at the top of each nf iteration.

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

## MNGA-23

**wrong expected value** · Conf M / Sev M  
**Found at (current code):** `atManageAnnual.c` 187
**Lines at review (Oct 2026):** 179-197, 204

**Function:** `Annual_Effort_Scale()`

**Why it's thought to be a bug**

The post-scaling check compares `newtoteffort` with `(1.0 - effort_scale) * toteffort` whether effort was scaled up by `(1+effort_scale)` or down by `(1-effort_scale/2)`. The check therefore nearly always fires and forces effort to `(1-effort_scale)*toteffort`, so effort is always reduced and the up-scaling is undone. Function: `Annual_Effort_Scale()`.

**Code at issue** — `atManageAnnual.c` line 187  
Search for: `(1.0 - effort_scale) * toteffort`

```c
  185: 			}
  186: 
  187→ 			if (fabs(1 - (newtoteffort / ((1.0 - effort_scale) * toteffort))) > buffer_rounding) {
  188: 				for (b = 0; b < bm->nbox; b++) {
  189: 					lastEffort = bm->Effort[b][nf];
```

**Suggested fix (from review):** Store the scalar actually applied (`1+effort_scale` or `1-effort_scale/2`) and use it in both checks.

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

atManageAnnual.c:1810: `Get_Fishery_Group_Change_Scale(bm, nf, sp, mFC_num_changes_id, mFC_num_changes_id, ...)` uses the number of changes as the on/off flag. This works only when the count is non-zero, and harvest uses a different argument (atHarvestCatch.c:232, which itself passes `bm->flagchangeF` as a param index, which looks wrong too).

- [ ] Investigate   - [ ] Ignore

**Your comments:**

> 

### LC-2

atManageAnnual.c:1896: `Z_Est = 1 - end/start` is a proportion, not an instantaneous rate, but it is compared with and subtracted from instantaneous `Fcurr` (should be `-log(end/start)`).

- [ ] Investigate   - [ ] Ignore

**Your comments:**

> 

### LC-3

atManageAnnual.c:519 (and atManageSetup.c:485): the `"\n"` is printed outside the `if (flag_sp)` block, so blank log lines appear for unflagged species (cosmetic).

- [ ] Investigate   - [ ] Ignore

**Your comments:**

> 
