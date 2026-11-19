# atmanage/atManageAnnual.c — bug review

Generated 2026-11-19 from `atlantis/to-do-fixt-list.md`. **23 findings** (severity H: 11; confidence H and severity H: 11; on the priority list: 11), plus 3 lower-confidence notes.

Line numbers were re-mapped on 2026-11-19 to the current code (after the merge of main). Each was traced through git history from the version the review used, then checked against the text of the line. **Lines (current code)** and line numbers in the text are now current; ≈ means the code there has changed so the position is approximate. The review's own numbers are kept as **Lines at review**; a number ending in ? could not be traced and is the review's number. ⚠ means the quoted code is no longer at those lines: it may have changed or already been fixed.

Conf/Sev: confidence it's a bug / severity if it is (H = wrong results or crash, M = wrong in some configs, L = cosmetic, log, leak or latent). ✔ = the reviewer re-checked it by hand. ⭐ = on the high-priority list.

## Summary

| ID | Lines (current) | Conf/Sev | Category | Short description | Decision | Done |
|---|---|---|---|---|---|---|
| [MNGA-01](#mnga-01) ⭐ | 333 | H/H | wrong variable / OOB | `spTotCumCatch = Harvest_Get_TotCumCatch(sp, nf, bm->thisyear);` is called outside any `nf` loop. `nf` is stal… |  | [ ] |
| [MNGA-02](#mnga-02) ⭐ | 365, 371 | H/H | wrong index / OOB | `FunctGroupArray[sp].co_sp_catch[nf][co_sp2]`: `co_sp_catch` is allocated `Util_Alloc_Init_2D_Double(K_max_co_… |  | [ ] |
| [MNGA-03](#mnga-03) | 357-385 | M/M | logic | `co_sp2_TotCumCatch` is reset to 0 and only set when `flagdyn_coupdate` is on. With `flagdyn_coupdate==0` (sta… |  | [ ] |
| [MNGA-04](#mnga-04) | 345-348 | H/L | wrong index (log) | `FunctGroupArray[co_sp].groupCode`: `co_sp` is the companion slot index (0..max_co_sp-1), not a group id, so t… |  | [ ] |
| [MNGA-05](#mnga-05) ⭐ | 604-608, 901-903, 1004 | H/H | logic (stuck counter) | `tac_resetcount_id` is incremented only at line 1001. Line 604 resets it to 0, and the early `return` at 898-9… |  | [ ] |
| [MNGA-06](#mnga-06) ⭐ | 769-780 (alloc atManageSetup.c:911-912) | H/H | OOB | `yr_id = floor(FC_period/364 + 0.5) + bm->thisyear; LeverQueue[sp][nf][yr_id] = ...`. LeverQueue is `[sp][nf][… |  | [ ] |
| [MNGA-07](#mnga-07) | 912-924 | M/L | inverted condition | The comment says "If phase out date elapsed turn off" but the code is `if (phase_out > dayt) phased_out = 0; e… |  | [ ] |
| [MNGA-08](#mnga-08) ⭐ | 1381-1390 | H/H | accumulation error | `Braw += totfishpop[othersp]...; Bcurr += Assess_Add_Error(bm, er_case, Braw, ...)`. `Braw` is cumulative (pri… |  | [ ] |
| [MNGA-09](#mnga-09) ⭐ | 1395-1400, 1485-1491 | H/H | off-by-one / wrong bound | Both loops run `groupIndex < max_co_sp` and pick `othersp = sp` only when `groupIndex >= K_max_co_sp`. Because… |  | [ ] |
| [MNGA-10](#mnga-10) ⭐ | 1424-1429 | H/H | wrong variable / OOB write | `for (groupIndex = 0; groupIndex < K_max_co_sp + 1; ...) Fcurr += ...; Fcurr_sp[groupIndex] /= counter;`. Afte… |  | [ ] |
| [MNGA-11](#mnga-11) | 1357-1360 | H/L | memory leak | `calcM_sp`, `calcF_sp`, `counter_sp` and `Fcurr_sp` are allocated on every call and never freed. Function: `Gu… |  | [ ] |
| [MNGA-12](#mnga-12) | 1428 | M/M | inconsistent tier dispatch | `if (tier < tier8)` sends dyntier4(10), dyntier1B0(11), sp_rollover(12), tier13 and tier14 to the tier8/9 (Fre… |  | [ ] |
| [MNGA-13](#mnga-13) ⭐ | 1527, 1617-1631 | H/H | uninitialised accumulator | `Braw` is declared at 1527 without initialisation and accumulated with `Braw += ...` in the SSB branch. It is … |  | [ ] |
| [MNGA-14](#mnga-14) ⭐ | 1805, 1867-1869 | H/H | state not reset in loop | `mFC` is computed once per fishery (1802), but inside the cohort loop `mFC = 0` is assigned when a cohort is o… |  | [ ] |
| [MNGA-15](#mnga-15) ⭐ | 1913-1920 | H/H | switch fall-through | `case assess_M_est:` has no `break`, so it always falls into `default: quit(...)`. It also sets `M` rather tha… |  | [ ] |
| [MNGA-16](#mnga-16) | 1941-1943 | H/M | `=` vs `+=` | Invertebrate branch: `for (k...) this_Biom = calcTrackedMort[sp][nc][k][start_id] * ...;` overwrites on each s… |  | [ ] |
| [MNGA-17](#mnga-17) | 1932-1947, 1963 | H/M | stale / uninitialised value | `this_expect_catch` is only assigned when `expectF > 0`, but line 1960 always adds it to `sp_fishery_expected_… |  | [ ] |
| [MNGA-18](#mnga-18) | 1827 | H/M | accumulator reset in wrong loop | `speciesParams[sp_fishery_expected_catch_id] = 0` sits inside the `nf` loop, so each fishery wipes the previou… |  | [ ] |
| [MNGA-19](#mnga-19) | 1985-1991 | M/M | missing bound | `rescale_scalar = (orig - deductions)/orig` goes negative when a species' deduction share exceeds its expected… |  | [ ] |
| [MNGA-20](#mnga-20) | 1734, 1771 | H/L | copy-paste (message) | The quit messages say `Per_Sp_Frescale:` inside `Ecosystem_Cap_Frescale()`. |  | [ ] |
| [MNGA-21](#mnga-21) | 2043 | H/L | off-by-one | `if ((co_sp < 0) \|\| (co_sp > bm->K_num_tot_sp)) continue;` lets `co_sp == K_num_tot_sp` through, which indexes… |  | [ ] |
| [MNGA-22](#mnga-22) ⭐ | 2072-2142 | H/H | accumulators never filled / not reset | `avg_scale`, `num_scale` and `min_scale` are initialised per fleet but never updated from `lever_scale`. So `n… |  | [ ] |
| [MNGA-23](#mnga-23) | 180-198, 205 | M/M | wrong expected value | The post-scaling check compares `newtoteffort` with `(1.0 - effort_scale) * toteffort` whether effort was scal… |  | [ ] |

---

## MNGA-01

**wrong variable / OOB** · Conf H / Sev H ⭐ priority  
**Lines (current code):** 333  
**Lines at review (Oct 2026):** 333

**Function:** `Make_Mgmt_Decisions()`

**Why it's thought to be a bug**

`spTotCumCatch = Harvest_Get_TotCumCatch(sp, nf, bm->thisyear);` is called outside any `nf` loop. `nf` is stale: `K_num_fisheries` (left over from the loop at 286 or the inner loop at 348), so it reads `TotCumCatch[sp][K_num_fisheries][yr]`, past the end of the fishery dimension. It also gives a per-fleet catch ratio that does not depend on fleet, while `co_sp2_TotCumCatch` (358) is per fleet. Function: `Make_Mgmt_Decisions()`.

**Current code** (`atmanage/atManageAnnual.c` lines 331–335)

```c
  331: 				continue;
  332:             
  333:             co_TYPE = (int) (FunctGroupArray[sp].speciesParams[coType_id]);
  334:             spTotCumCatch = Harvest_Get_TotCumCatch(sp, nf, bm->thisyear);
  335: 
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
**Lines (current code):** 365, 371  
**Lines at review (Oct 2026):** 365, 371

**Function:** `Make_Mgmt_Decisions()`

**Why it's thought to be a bug**

`FunctGroupArray[sp].co_sp_catch[nf][co_sp2]`: `co_sp_catch` is allocated `Util_Alloc_Init_2D_Double(K_max_co_sp, K_num_fisheries)` (atManageParamIO.c:933, so `[nf][K_max_co_sp]`) and filled as `co_sp_catch[nf][co_sp]` using the slot index (atManageParamIO.c:306). `co_sp2` is a functional-group id (up to K_num_tot_sp-1), so this reads out of bounds or picks the wrong ratio. Function: `Make_Mgmt_Decisions()`.

**Current code** (`atmanage/atManageAnnual.c` lines 363–373)

```c
  363:                         }
  364:                     } else {
  365:                         /* Always use initial (read-in) catch ratios */
  366:                         FC2_ratio = FunctGroupArray[sp].co_sp_catch[nf][co_sp2];
  367:                     }
  368:                     spA_TAC = bm->TACamt[sp][nf][now_id];
  369: 
  370:                     /* Cope with zero catches when quota still available */
  371:                     if ((spTotCumCatch == 0) && spA_TAC) {
  372:                         FC2_ratio = max(1.0, FunctGroupArray[sp].co_sp_catch[nf][co_sp2]);
  373:                     }
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
**Lines (current code):** 357-385  
**Lines at review (Oct 2026):** 354-382

**Function:** `Make_Mgmt_Decisions()`

**Why it's thought to be a bug**

`co_sp2_TotCumCatch` is reset to 0 and only set when `flagdyn_coupdate` is on. With `flagdyn_coupdate==0` (static read-in ratios), `if (!co_sp2_TotCumCatch && spC_TAC) dont_scale2 = 1;` is true whenever the companion has a TAC, so companion TACs are never adjusted in static-ratio mode. Function: `Make_Mgmt_Decisions()`.

**Current code** (`atmanage/atManageAnnual.c` lines 355–387)

```c
  355:                     co_sp2_TotCumCatch = 0.0;
  356:                     if (bm->flagdyn_coupdate) {
  357:                         /* Update catch ratios */
  358:                         if (co_sp2 != -1 && FunctGroupArray[co_sp2].isFished == TRUE) {
  359:                             co_sp2_TotCumCatch = Harvest_Get_TotCumCatch(co_sp2, nf, bm->thisyear);
  360:                             FC2_ratio = co_sp2_TotCumCatch / (spTotCumCatch + small_num);
  361:                         } else {
  362:                             FC2_ratio = 1.0;
  363:                         }
  364:                     } else {
  365:                         /* Always use initial (read-in) catch ratios */
  366:                         FC2_ratio = FunctGroupArray[sp].co_sp_catch[nf][co_sp2];
  367:                     }
  368:                     spA_TAC = bm->TACamt[sp][nf][now_id];
  369: 
  370:                     /* Cope with zero catches when quota still available */
  371:                     if ((spTotCumCatch == 0) && spA_TAC) {
  372:                         FC2_ratio = max(1.0, FunctGroupArray[sp].co_sp_catch[nf][co_sp2]);
  373:                     }
  374: 
  375:                     if (co_sp2 != -1 && FunctGroupArray[co_sp2].isFished == TRUE) {
  376:                         spC_TAC = bm->TACamt[co_sp2][nf][now_id];
  377:                         expect_spC = FC2_ratio * spA_TAC; // What expected TAC of companion to A should be
  378:                         dont_scale2 = 0;
  379: 
  380:                         /* For those cases where there is no actual catch despite wanting quota don't play with the TACs */
  381:                         if (!co_sp2_TotCumCatch && spC_TAC) {
  382:                             dont_scale2 = 1;
  383:                         }
  384:                     } else {
  385:                         dont_scale2 = 1;
  386:                     }
  387: 
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
**Lines (current code):** 345-348  
**Lines at review (Oct 2026):** 342-345

**Function:** `Make_Mgmt_Decisions()`

**Why it's thought to be a bug**

`FunctGroupArray[co_sp].groupCode`: `co_sp` is the companion slot index (0..max_co_sp-1), not a group id, so the log names the wrong group. Also `co_sp2 > 0` should be `>= 0` (group 0 is valid; -1 was already skipped at 339). Function: `Make_Mgmt_Decisions()`.

**Current code** (`atmanage/atManageAnnual.c` lines 343–350)

```c
  343:                 if (co_sp2 > 0) {
  344:                     fprintf(llogfp, "Time: %e, looking at %s vs co_sp: %s, co_sp2: %s, flagdyn_coupdate: %d\n", bm->dayt, FunctGroupArray[sp].groupCode, FunctGroupArray[co_sp].groupCode, FunctGroupArray[co_sp2].groupCode, bm->flagdyn_coupdate);
  345:                 } else {
  346:                     fprintf(llogfp, "Time: %e, looking at %s vs co_sp: %s, flagdyn_coupdate: %d\n", bm->dayt, FunctGroupArray[sp].groupCode, FunctGroupArray[co_sp].groupCode, bm->flagdyn_coupdate);
  347:                 }
  348: 
  349:                 for (nf = 0; nf < bm->K_num_fisheries; nf++) {
  350: 
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
**Lines (current code):** 604-608, 901-903, 1004  
**Lines at review (Oct 2026):** 601-605, 898-900, 1001

**Function:** `AMS_Tiered_Assessment()`

**Why it's thought to be a bug**

`tac_resetcount_id` is incremented only at line 1004. Line 607 resets it to 0, and the early `return` at 898-900 (TAC change < `TAC_crit_change`) skips the increment. Line 604 returns while `count < period` without incrementing. Once a year's RBC falls inside `TAC_crit_change`, the counter stays at 0 and `0 < tac_resetperiod` (>=1) holds every year, so that species' TAC is never reassessed for the rest of the run. Function: `AMS_Tiered_Assessment()`.

**Current code** (`atmanage/atManageAnnual.c` lines 602–610)

```c
  602:         if(FunctGroupArray[sp].speciesParams[tac_resetcount_id] < FunctGroupArray[sp].speciesParams[tac_resetperiod_id]) {
  603:             return;
  604:         }
  605: 		FunctGroupArray[sp].speciesParams[tac_resetcount_id] = 0; // Reinitialise the counter if going to reset TAC
  606: 		FunctGroupArray[sp].speciesParams[tac_resetnow_id] = 1;
  607: 
  608: 
  609: 		/** Make decisions on effort and total quota **/
  610: 		/* Use assessment model output to determine changes needed.
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
**Lines (current code):** 769-780 (alloc atManageSetup.c:911-912)  
**Lines at review (Oct 2026):** 766-777 (alloc atManageSetup.c:912-913)

**Function:** `AMS_Tiered_Assessment()`

**Why it's thought to be a bug**

`yr_id = floor(FC_period/364 + 0.5) + bm->thisyear; LeverQueue[sp][nf][yr_id] = ...`. LeverQueue is `[sp][nf][num_years]` with `num_years = ceil((tstop-tstart)/1 yr)`. In the last year(s) of the run, `yr_id >= num_years`, so this writes past the end of the array (heap corruption) whenever tier6 is used. Function: `AMS_Tiered_Assessment()`.

**Current code** (`atmanage/atManageAnnual.c` lines 767–782)

```c
  767: 				yr_id = (int) floor((bm->FISHERYprms[nf][FC_period_id] / 364.0) + 0.5);
  768: 				yr_id += bm->thisyear; /* Year management will take effect */
  769: 				switch (use_lever) {
  770: 				case use_none: /* Nothing to do */
  771: 					LeverQueue[sp][nf][yr_id] = 1.0;
  772: 					break;
  773: 				case use_effort: /* Effort change */
  774: 				case use_catch: /* Quota change */
  775: 				case use_space: /* Spatial management change */
  776: 				case use_season: /* Season change */
  777: 				case use_gear: /* Gear change */
  778: 					LeverQueue[sp][nf][yr_id] = Fratio;
  779: 					break;
  780: 				default:
  781: 					quit("No such lever for MSY based management (must be between %d for no lever to %d to for changing gear\n", use_none, use_gear);
  782: 					break;
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
**Lines (current code):** 912-924  
**Lines at review (Oct 2026):** 909-921

**Function:** `AMS_Tiered_Assessment()`

**Why it's thought to be a bug**

The comment says "If phase out date elapsed turn off" but the code is `if (phase_out > dayt) phased_out = 0; else if (phase_start > dayt) {ramp}`. Both comparisons are reversed: a future phase-out date zeroes the TAC now, and the ramp runs before phase_start. Latent only, because `phase_out_id`/`phase_start_id` are never read in (they stay 0, so phased_out=1). The ramp also divides by `period` with no zero check. Function: `AMS_Tiered_Assessment()`.

**Current code** (`atmanage/atManageAnnual.c` lines 910–926)

```c
  910: 			/* If phase out date elapsed turn off ability to have TAC for the group */
  911: 			if (bm->SP_FISHERYprms[sp][nf][phase_out_id] > bm->dayt) {
  912: 				bm->phased_out[sp][nf] = 0.0;
  913: 			} else if (bm->SP_FISHERYprms[sp][nf][phase_start_id] > bm->dayt) {
  914: 				end_date = bm->SP_FISHERYprms[sp][nf][phase_out_id];
  915: 				start_date = bm->SP_FISHERYprms[sp][nf][phase_start_id];
  916: 				period = end_date - start_date;
  917: 				TAC_scale = 1.0 - (bm->dayt - start_date) / period;
  918: 				bm->phased_out[sp][nf] = TAC_scale;
  919: 			} else {
  920: 				/* Do nothing so keep as 1.0 */
  921: 				bm->phased_out[sp][nf] = 1.0;
  922: 			}
  923: 		}
  924: 
  925: 		/* Set total quota changes - checking for "phased out" fisheries.
  926: 		 Then allocate across the fisheries.
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
**Lines (current code):** 1381-1390  
**Lines at review (Oct 2026):** 1378-1387

**Function:** `Guild_Frescale()`

**Why it's thought to be a bug**

`Braw += totfishpop[othersp]...; Bcurr += Assess_Add_Error(bm, er_case, Braw, ...)`. `Braw` is cumulative (primary plus all previous guild members), and each step adds the error-applied *cumulative* biomass to Bcurr. The guild biomass is massively overcounted (primary counted n+1 times, etc.), which skews the HCR toward FrefA/FrefH. Function: `Guild_Frescale()`.

**Current code** (`atmanage/atManageAnnual.c` lines 1379–1392)

```c
 1379:     for (groupIndex = 0; groupIndex < FunctGroupArray[sp].speciesParams[max_co_sp_id]; groupIndex++ ) {
 1380:         othersp = FunctGroupArray[sp].co_sp[groupIndex];
 1381:     
 1382:         Braw += bm->totfishpop[othersp] * bm->X_CN * mg_2_tonne;
 1383:         Bcurr += Assess_Add_Error(bm, er_case, Braw, est_bias, est_cv);
 1384:     
 1385:         BrefA += FunctGroupArray[othersp].speciesParams[BrefA_id] * bm->estBo[othersp];
 1386:         BrefB += FunctGroupArray[othersp].speciesParams[BrefB_id] * bm->estBo[othersp];
 1387:         Blim += FunctGroupArray[othersp].speciesParams[Blim_id] * bm->estBo[othersp];
 1388:     }
 1389:     FrefA = FunctGroupArray[sp].speciesParams[FrefA_id];
 1390:     FrefH = FunctGroupArray[sp].speciesParams[FrefH_id];
 1391:     
 1392:     /** Using perfect knowledge way of determining M and F - averaging over the species in the guild */
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
**Lines (current code):** 1395-1400, 1485-1491  
**Lines at review (Oct 2026):** 1392-1397, 1482-1488

**Function:** `Guild_Frescale()`

**Why it's thought to be a bug**

Both loops run `groupIndex < max_co_sp` and pick `othersp = sp` only when `groupIndex >= K_max_co_sp`. Because max_co_sp <= K_max_co_sp (co_sp has K_max_co_sp entries), the primary species is never processed. Its Fcurr is not included, and in the apply loop its `mFC_scale` is never set to F_rescale and `done_Co_sp_id` is never set. The comments ("+1 is for the primary species") show it should be included. Function: `Guild_Frescale()`.

**Current code** (`atmanage/atManageAnnual.c` lines 1393–1402)

```c
 1393:     for (groupIndex = 0; groupIndex < FunctGroupArray[sp].speciesParams[max_co_sp_id]; groupIndex++ ) {
 1394:         if (groupIndex < bm->K_max_co_sp) {
 1395:             othersp = FunctGroupArray[sp].co_sp[groupIndex];
 1396:         } else {
 1397:             othersp = sp;
 1398:         }
 1399:         
 1400:         calcF_sp[groupIndex] = 0.0;
 1401:         calcM_sp[groupIndex] = 0.0;
 1402:         counter_sp[groupIndex] = 0.0;
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
**Lines (current code):** 1424-1429  
**Lines at review (Oct 2026):** 1421-1426

**Function:** `Guild_Frescale()`

**Why it's thought to be a bug**

`for (groupIndex = 0; groupIndex < K_max_co_sp + 1; ...) Fcurr += ...; Fcurr_sp[groupIndex] /= counter;`. After the loop `groupIndex == K_max_co_sp+1`, so this writes one past the end of `Fcurr_sp` (allocated K_max_co_sp+1 at 1357), and `Fcurr` is never averaged: it stays a sum. The divisor also counts unused slots rather than guild members. Function: `Guild_Frescale()`.

**Current code** (`atmanage/atManageAnnual.c` lines 1422–1431)

```c
 1422:     Fcurr = 0.0;
 1423:     counter = (double)(bm->K_max_co_sp + 1);
 1424:     for (groupIndex = 0; groupIndex < bm->K_max_co_sp + 1; groupIndex++) {
 1425:         Fcurr += Fcurr_sp[groupIndex];
 1426:     }
 1427:     Fcurr_sp[groupIndex] /= counter;
 1428: 
 1429:     if (tier < tier8) {  // So not US (tier8) or Norway (tier 9)
 1430:         /* Tier 1 - Best quantitative assessment available */
 1431:         if (Bcurr >= BrefA) {
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
**Lines (current code):** 1357-1360  
**Lines at review (Oct 2026):** 1354-1357

**Function:** `Guild_Frescale()`

**Why it's thought to be a bug**

`calcM_sp`, `calcF_sp`, `counter_sp` and `Fcurr_sp` are allocated on every call and never freed. Function: `Guild_Frescale()`.

**Current code** (`atmanage/atManageAnnual.c` lines 1355–1362)

```c
 1355:     double *calcM_sp = Util_Alloc_Init_1D_Double(bm->K_max_co_sp + 1, 0.0); // The +1 is for the primary species
 1356:     double *calcF_sp = Util_Alloc_Init_1D_Double(bm->K_max_co_sp + 1, 0.0); // The +1 is for the primary species
 1357:     double *counter_sp = Util_Alloc_Init_1D_Double(bm->K_max_co_sp + 1, 0.0); // The +1 is for the primary species
 1358:     double *Fcurr_sp = Util_Alloc_Init_1D_Double(bm->K_max_co_sp + 1, 0.0); // The +1 is for the primary species
 1359:     double counter, Fcurr;
 1360: 
 1361:     int tier = (int) (FunctGroupArray[sp].speciesParams[tier_id]);
 1362:     int er_case = (int) (FunctGroupArray[sp].speciesParams[estError_id]);
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
**Lines (current code):** 1428  
**Lines at review (Oct 2026):** 1428

**Function:** `Guild_Frescale()`

**Why it's thought to be a bug**

`if (tier < tier8)` sends dyntier4(10), dyntier1B0(11), sp_rollover(12), tier13 and tier14 to the tier8/9 (FrefH) broken stick. `Per_Sp_Frescale()`/`Ecosystem_Cap_Frescale()` treat 10-12 and 14 as the tier1 stick and tier13 as escapement. The F_rescale branch at 1465 is also tier8-specific. Function: `Guild_Frescale()`.

**Current code** (`atmanage/atManageAnnual.c` lines 1426–1430)

```c
 1426:     }
 1427:     Fcurr_sp[groupIndex] /= counter;
 1428: 
 1429:     if (tier < tier8) {  // So not US (tier8) or Norway (tier 9)
 1430:         /* Tier 1 - Best quantitative assessment available */
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
**Lines (current code):** 1527, 1617-1631  
**Lines at review (Oct 2026):** 1527, 1617-1631

**Function:** `Ecosystem_Cap_Frescale()`

**Why it's thought to be a bug**

`Braw` is declared at 1527 without initialisation and accumulated with `Braw += ...` in the SSB branch. It is never reset per species, so the first species uses garbage and later species include all previous species' SSB. (`Per_Sp_Frescale()` initialises `Braw = 0.0` per call.) Function: `Ecosystem_Cap_Frescale()`.

**Current code** (`atmanage/atManageAnnual.c` lines 1525–1529)

```c
 1525: 
 1526: void Ecosystem_Cap_Frescale(MSEBoxModel *bm, FILE *llogfp) {
 1527:     int sp, nf, nc, cohort, ij, b, k, flagF, tier, er_case, maxstock, mFC_end_age, mFC_start_age, flagfcmpa, sel_curve, stage, basechrt;
 1528:     double max_mFC, F_rescale, FTARG, Bcurr, calcM, survival, Fcurr, calcF, Fstep1, this_mFC, M, est_bias, est_cv, BrefA, BrefB, BrefE, Blim, FrefA, FrefH, FrefLim, Braw, sel, this_expect_catch, sp_fishery_pref_weight, w_inv, tot_w_inv, counter, mFC, mFC_change_scale, mpa_scale, mpa_infringe, Wgt, li, gear_change_scale, this_Num, this_start, this_end, this_Biom, Z_Est, expectF, Catch_Eqn_Denom, orig_expected_catch, excess, deductions, new_expected_catch, rescale_scalar,tot_area, fishable_area;
 1529:     //double calcM;
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
**Lines (current code):** 1805, 1867-1869  
**Lines at review (Oct 2026):** 1802, 1864-1866

**Function:** `Ecosystem_Cap_Frescale()`

**Why it's thought to be a bug**

`mFC` is computed once per fishery (1805), but inside the cohort loop `mFC = 0` is assigned when a cohort is outside [mFC_start_age, mFC_end_age). Cohort 0 is normally younger than mFC_start_age, so mFC is zeroed at nc=0 and stays 0 for every older cohort: the expected catch is 0 and the system cap never binds (when `flag_sel_with_mFC==0`). Function: `Ecosystem_Cap_Frescale()`.

**Current code** (`atmanage/atManageAnnual.c` lines 1803–1807) — ⚠ the code quoted in the review is not at these lines any more (changed, or already fixed?)

```c
 1803:                     mFC = bm->SP_FISHERYprms[sp][nf][mFC_id] * 365.0;
 1804:                     mFC *= bm->SP_FISHERYprms[sp][nf][mFC_scale_id];  // Apply the broken stick scalar from above
 1805:                     
 1806:                     /* Get scenario scalars */
 1807:                     if (bm->flagchangeF){
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
**Lines (current code):** 1913-1920  
**Lines at review (Oct 2026):** 1910-1917

**Function:** `Ecosystem_Cap_Frescale()`

**Why it's thought to be a bug**

`case assess_M_est:` has no `break`, so it always falls into `default: quit(...)`. It also sets `M` rather than `calcM`, so calcM would be uninitialised or stale anyway. The quit text names `syst_cap_calc_method` rather than `M_est_method`. Function: `Ecosystem_Cap_Frescale()`.

**Current code** (`atmanage/atManageAnnual.c` lines 1911–1922)

```c
 1911:                                 case assess_M_est:
 1912:                                     if(!do_assess) {
 1913:                                         quit("Ecosystem_Cap_Frescale: For %s Can't use option %d for M_est_method if do_assess = 0\n", FunctGroupArray[sp].groupCode, assess_M_est);
 1914:                                     } else {
 1915:                                         M = bm->NAssess[sp][est_M_id];
 1916:                                     }
 1917:                                 default:
 1918:                                     quit("No such option for syst_cap_calc_method. Please set to 0 for fixed M (read in from harvest.prm), 1 for M = Z-F, 2 for fully dynamic estimation of M given in model predation, or 3 for M estimated in an assessment\n");
 1919:                             }
 1920:                             
 1921:                             
 1922:                             
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
**Lines (current code):** 1941-1943  
**Lines at review (Oct 2026):** 1938-1940

**Function:** `Ecosystem_Cap_Frescale()`

**Why it's thought to be a bug**

Invertebrate branch: `for (k...) this_Biom = calcTrackedMort[sp][nc][k][start_id] * ...;` overwrites on each stock, so only the last stock's biomass is used (the vertebrate branch at 1933 accumulates). Function: `Ecosystem_Cap_Frescale()`.

**Current code** (`atmanage/atManageAnnual.c` lines 1939–1945)

```c
 1939:                                     for (k = 0; k < maxstock; k++) {
 1940:                                         this_Biom = bm->calcTrackedMort[sp][nc][k][start_id] * bm->X_CN * mg_2_tonne;
 1941:                                     }
 1942:                                 }
 1943:                                 this_expect_catch = this_Biom * (expectF / Catch_Eqn_Denom) * survival;
 1944:                                 tot_expect_catch += this_expect_catch;
 1945:                             }
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
**Lines (current code):** 1932-1947, 1963  
**Lines at review (Oct 2026):** 1929-1944, 1960

**Function:** `Ecosystem_Cap_Frescale()`

**Why it's thought to be a bug**

`this_expect_catch` is only assigned when `expectF > 0`, but line 1963 always adds it to `sp_fishery_expected_catch_id`. Cohorts with zero F add the previous cohort's (or previous species', or on first use uninitialised) value. Function: `Ecosystem_Cap_Frescale()`.

**Current code** (`atmanage/atManageAnnual.c` lines 1930–1965)

```c
 1930:                             if(expectF > 0) { // Only do this if non-zero F to avoid time wasting loops
 1931:                                 if (FunctGroupArray[sp].isVertebrate == TRUE) {
 1932:                                     this_Num = 0.0;
 1933:                                     for (k = 0; k < maxstock; k++) {
 1934:                                         this_Num += bm->calcTrackedMort[sp][nc][k][start_id];
 1935:                                     }
 1936:                                     this_Biom = (Wgt * bm->X_CN * mg_2_tonne) * this_Num;
 1937:                                 } else {
 1938:                                     this_Biom = 0.0;
 1939:                                     for (k = 0; k < maxstock; k++) {
 1940:                                         this_Biom = bm->calcTrackedMort[sp][nc][k][start_id] * bm->X_CN * mg_2_tonne;
 1941:                                     }
 1942:                                 }
 1943:                                 this_expect_catch = this_Biom * (expectF / Catch_Eqn_Denom) * survival;
 1944:                                 tot_expect_catch += this_expect_catch;
 1945:                             }
 1946:                         } else {
 1947:                             if (!bm->use_time_avg_biom) {
 1948:                                 if (bm->dayt>0 && FunctGroupArray[sp].min_B[nc] > MAXDOUBLE) {
 1949:                                     quit("Ecosystem_Cap_Frescale: Time %e min_B for %s-%d is larger than is feasible (%e) something has gone wrong\n", bm->dayt, FunctGroupArray[sp].groupCode, nc, FunctGroupArray[sp].min_B[nc]);
 1950:                                 }
 1951:                                 if (bm->dayt>0 && FunctGroupArray[sp].max_B[nc] < MINDOUBLE) {
 1952:                                     quit("Ecosystem_Cap_Frescale: Time %e max_B for %s-%d is smaller than is feasible (%e) something has gone wrong\n", bm->dayt, FunctGroupArray[sp].groupCode, nc, FunctGroupArray[sp].max_B[nc]);
 1953:                                 }
 1954:                                 this_Biom = (FunctGroupArray[sp].min_B[nc] + FunctGroupArray[sp].max_B[nc]) / 2.0;
 1955:                             } else {
 1956:                                 this_Biom = FunctGroupArray[sp].rolling_B[nc][bm->K_rolling_cap_num];
 1957:                             }
 1958:                             this_expect_catch = mFC * mpa_scale * mFC_change_scale * sel * this_Biom * bm->X_CN * mg_2_tonne;
 1959:                             tot_expect_catch += this_expect_catch;
 1960:                         }
 1961:                         FunctGroupArray[sp].speciesParams[sp_fishery_expected_catch_id] += this_expect_catch;
 1962:                     }
 1963:                 }
 1964:             }
 1965:         }
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
**Lines (current code):** 1827  
**Lines at review (Oct 2026):** 1827

**Function:** `Ecosystem_Cap_Frescale()`

**Why it's thought to be a bug**

`speciesParams[sp_fishery_expected_catch_id] = 0` sits inside the `nf` loop, so each fishery wipes the previous fisheries' expected catch. Only the last flagF fishery's catch survives, while `tot_expect_catch` sums all fisheries. The per-species deduction scalar at 1981-1984 is therefore wrong in multi-fleet models. Function: `Ecosystem_Cap_Frescale()`.

**Current code** (`atmanage/atManageAnnual.c` lines 1825–1829)

```c
 1825:                     }
 1826: 
 1827:                     sel_curve = (int) (bm->FISHERYprms[nf][selcurve_id]);
 1828:                     FunctGroupArray[sp].speciesParams[sp_fishery_expected_catch_id] = 0;
 1829:                     for (nc = 0; nc < FunctGroupArray[sp].numCohortsXnumGenes; nc++) {
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
**Lines (current code):** 1985-1991  
**Lines at review (Oct 2026):** 1982-1988

**Function:** `Ecosystem_Cap_Frescale()`

**Why it's thought to be a bug**

`rescale_scalar = (orig - deductions)/orig` goes negative when a species' deduction share exceeds its expected catch, giving a negative `mFC_scale`. Function: `Ecosystem_Cap_Frescale()`.

**Current code** (`atmanage/atManageAnnual.c` lines 1983–1993)

```c
 1983:                 if (orig_expected_catch) { // only both of expected catch is non-zero
 1984:                     new_expected_catch = orig_expected_catch - deductions;
 1985:                     rescale_scalar = (new_expected_catch / orig_expected_catch);
 1986:                     for (nf = 0; nf < bm->K_num_fisheries; nf++) {
 1987:                         bm->SP_FISHERYprms[sp][nf][orig_mFC_scale_id] = bm->SP_FISHERYprms[sp][nf][mFC_scale_id]; // For reporting purposes
 1988:                         bm->SP_FISHERYprms[sp][nf][mFC_scale_id] *= rescale_scalar;
 1989:                     }
 1990:                 }
 1991:             }
 1992:         }
 1993:     }
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
**Lines (current code):** 1734, 1771  
**Lines at review (Oct 2026):** 1734, 1771

**Why it's thought to be a bug**

The quit messages say `Per_Sp_Frescale:` inside `Ecosystem_Cap_Frescale()`.

**Current code** (`atmanage/atManageAnnual.c` lines 1732–1771 (truncated))

```c
 1732:                     }
 1733:                     break;
 1734:                 default:
 1735:                     quit("Per_Sp_Frescale: We do not have any code for tier %d\n", tier);
 1736:                     break;
 1737:             }
 1738: 
 1739:             /* Rescale all F accordingly to be in line with single species expectations - basically F to deliver the Acceptable Biological Catch */
 1740:             if(!Fcurr && FTARG) {
 1741:                 F_rescale = FunctGroupArray[sp].speciesParams[F_restart_id];
 1742:             } else {
 1743:                 switch (tier) {
 1744:                     case tier0:     // Intentional flow throgh for all these cases
 1745:                     case tier1:
 1746:                     case tier2:
 1747:                     case tier3:
 1748:                     case tier4:
 1749:                     case tier5:
 1750:                     case tier6:
 1751:                     case tier7:
 1752:                     case dyntier4:
 1753:                     case dyntier1B0:
 1754:                     case sp_rollover:
 1755:                     case tier13:
 1756:                     case tier14:
 1757:                         Fstep1 = Fcurr / (FunctGroupArray[sp].speciesParams[maxmFC_id] * 365.0);  // As Fcurr is annual but mFC is daily
 1758:                         F_rescale = Fstep1 * (FTARG / (Fcurr + small_num));  // Re-scale existing F (need to do vs mFC rather than just Fcurr as code applies it against mFC)
 1759:                         break;
 1760:                     case tier9: // Norwegian way of doing it - to avoid overcatch during spin-up phase
 1761:                         Fstep1 = Fcurr / (FunctGroupArray[sp].speciesParams[maxmFC_id] * 365.0);  // As Fcurr is annual but mFC is daily
 1762:                         F_rescale = Fstep1 * (FTARG / (FrefA + small_num));
 1763:                         break;
 1764:                     case tier8:
 1765:                         /* In tier 8 version rescaling vs FrefA - although applied vs mFC
 1766:                          so just in case the user has not entered FrefA for mFC do a two step rescaling here */
 1767:                         this_mFC = FunctGroupArray[sp].speciesParams[maxmFC_id] * 365.0;  // As FrefA is annual but mFC is daily
 1768:                         Fstep1 = FrefA / this_mFC;
 1769:                         F_rescale = Fstep1 * (FTARG / (FrefA + small_num));  // Re-scale the F
 1770:                         break;
 1771:                     default:
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
**Lines (current code):** 2043  
**Lines at review (Oct 2026):** 2040

**Function:** `Check_Ecosystem_F_Harvest_Control_Rule()`

**Why it's thought to be a bug**

`if ((co_sp < 0) || (co_sp > bm->K_num_tot_sp)) continue;` lets `co_sp == K_num_tot_sp` through, which indexes FunctGroupArray out of bounds. Function: `Check_Ecosystem_F_Harvest_Control_Rule()`.

**Current code** (`atmanage/atManageAnnual.c` lines 2041–2045)

```c
 2041:                                 if ((co_sp < 0) || (co_sp > bm->K_num_tot_sp))
 2042:                                     continue;
 2043: 
 2044: 								/* Check to see that want to use a harvest control rule */
 2045: 								tier = (int)(FunctGroupArray[co_sp].speciesParams[tier_id]);
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
**Lines (current code):** 2072-2142  
**Lines at review (Oct 2026):** 2069-2139

**Function:** `Do_MSY_Management()`

**Why it's thought to be a bug**

`avg_scale`, `num_scale` and `min_scale` are initialised per fleet but never updated from `lever_scale`. So `new_scale` is 0 (avg) or MAXDOUBLE (min), and `effort_scale[nf][target_tac_id]` / season length are set to 0 or MAXDOUBLE whenever an effort, space or season lever is used. Also `update_scale`/`update_season` are initialised once outside the `nf` loop, so once set by one fleet they apply to every later fleet. Function: `Do_MSY_Management()`.

**Current code** (`atmanage/atManageAnnual.c` lines 2070–2109 (truncated))

```c
 2070: 	int sp, nf, use_lever, update_scale = 0, update_season = 0, fishstartday, fishendday, season_length, newfishendday, tempendday;
 2071: 	double new_scale = 0.0, avg_scale = 0.0, num_scale = 0.0, min_scale = 0.0, lever_scale = 0.0;
 2072: 
 2073: 	/* If not doing MSY related management leave routine */
 2074: 	if (!bm->do_MSY)
 2075: 		return;
 2076: 
 2077: 	/* MSY-based management updating */
 2078: 	for (nf = 0; nf < bm->K_num_fisheries; nf++) {
 2079: 		new_scale = 1.0;
 2080: 		avg_scale = 0.0;
 2081: 		num_scale = 0.0;
 2082: 		min_scale = MAXDOUBLE;
 2083: 		bm->FISHERYprms[nf][use_msy_effort_id] = 0;
 2084: 		for (sp = 0; sp < bm->K_num_tot_sp; sp++) {
 2085: 			if (FunctGroupArray[sp].isFished == TRUE) {
 2086: 				use_lever = (int) (FunctGroupArray[sp].speciesParams[use_lever_id]);
 2087: 				lever_scale = LeverQueue[sp][nf][bm->thisyear];
 2088: 
 2089: 				switch (use_lever) {
 2090: 				case use_none: /* Nothing to do */
 2091: 					break;
 2092: 				case use_effort: /* Effort change */
 2093: 				case use_season: /* Season change */
 2094: 				case use_space: /* Spatial management change */
 2095: 					break;
 2096: 				case use_catch: /* Quota change */
 2097: 					bm->TACamt[sp][nf][now_id] *= lever_scale;
 2098: 					break;
 2099: 				case use_gear: /* Gear change */
 2100: 					bm->SP_FISHERYprms[sp][nf][q_id] *= lever_scale;
 2101: 					break;
 2102: 				default:
 2103: 					quit("No such lever for MSY based management (must be between %d for no lever to %d to for changing gear\n", use_none, use_gear);
 2104: 					break;
 2105: 				}
 2106: 				switch (use_lever) {
 2107: 				case use_none: /* Nothing to do */
 2108: 				case use_catch: /* Quota change */
 2109: 				case use_gear: /* Gear change */
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
**Lines (current code):** 180-198, 205  
**Lines at review (Oct 2026):** 179-197, 204

**Function:** `Annual_Effort_Scale()`

**Why it's thought to be a bug**

The post-scaling check compares `newtoteffort` with `(1.0 - effort_scale) * toteffort` whether effort was scaled up by `(1+effort_scale)` or down by `(1-effort_scale/2)`. The check therefore nearly always fires and forces effort to `(1-effort_scale)*toteffort`, so effort is always reduced and the up-scaling is undone. Function: `Annual_Effort_Scale()`.

**Current code** (`atmanage/atManageAnnual.c` lines 178–207)

```c
  178: 			}
  179: 
  180: 			/* Check effort change isn't too radical - especially for use with pseudo assessment code */
  181: 			newtoteffort = 0;
  182: 			for (b = 0; b < bm->nbox; b++) {
  183: 				newtoteffort += bm->Effort[b][nf] * 365.0 * (86400.0 / bm->dt);
  184: 				/* Need to multiply by 365 to get daily rate to approximate annual rate */
  185: 			}
  186: 
  187: 			if (fabs(1 - (newtoteffort / ((1.0 - effort_scale) * toteffort))) > buffer_rounding) {
  188: 				for (b = 0; b < bm->nbox; b++) {
  189: 					lastEffort = bm->Effort[b][nf];
  190: 					bm->Effort[b][nf] *= (((1.0 - effort_scale) * toteffort) / (newtoteffort + small_num));
  191: 
  192: 					if ((bm->debug == debug_effort) && (bm->which_fleet == nf)) {
  193: 						fprintf(llogfp, "Time: %e  %s in %d newEffort = %e (effort_scale: %e, toteffort: %e, newtoteffort: %e, lastEffort: %e)\n", bm->dayt,
  194: 								FisheryArray[nf].fisheryCode, b, bm->Effort[b][nf], effort_scale, toteffort, newtoteffort, lastEffort);
  195: 					}
  196: 
  197: 				}
  198: 			}
  199: 
  200: 			/* Double check result of rescaling */
  201: 			newtoteffort = 0;
  202: 			for (b = 0; b < bm->nbox; b++) {
  203: 				newtoteffort += bm->Effort[b][nf] * 365.0 * (86400.0 / bm->dt);
  204: 			}
  205: 			if (fabs(1 - (newtoteffort / ((1.0 - effort_scale) * toteffort))) > buffer_rounding) {
  206: 				fprintf(llogfp,
  207: 						"Time: %e - Something is wrong in annual effort adjustment for %s as oldeffort: %e, neweffort: %e but expecting %e so resetting\n",
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
