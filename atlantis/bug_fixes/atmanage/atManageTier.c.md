# atmanage/atManageTier.c — bug review

Generated 2026-11-19 from `atlantis/to-do-fixt-list.md`. **56 findings** (severity H: 24; confidence H and severity H: 23; on the priority list: 23), plus 8 lower-confidence notes.

Each finding's **Code at issue** is located by searching the current code for the code quoted in the review, not by line number. Use the **Search for** text with your editor's Find, because line numbers will keep changing as fixes go in. → marks the matching line. If only a close match or a name could be found, the block says so. ⚠ means the quoted code is no longer in the file: it may have been changed or already fixed. **Lines at review** are the October numbers. Line numbers in the description text are approximate.

Conf/Sev: confidence it's a bug / severity if it is (H = wrong results or crash, M = wrong in some configs, L = cosmetic, log, leak or latent). ✔ = the reviewer re-checked it by hand. ⭐ = on the high-priority list.

## Summary

| ID | Found at | Conf/Sev | Category | Short description | Decision | Done |
|---|---|---|---|---|---|---|
| [TIER-01](#tier-01) ⭐ | 580 | H/H | wrong operator / sign | `tac_change = (TAC_old - TACpost)/TAC_old; if (tac_change > inc_check) TAC = TAC_old + inc_check*TAC_old; else… |  | [ ] |
| [TIER-02](#tier-02) ⭐ | 605 | H/H | wrong variable | `if (TAC < minCatch) TAC = FunctGroupArray[species].speciesParams[MinCatch_id];` sets the TAC to the fraction … |  | [ ] |
| [TIER-03](#tier-03) ⭐ | 637 | H/H | wrong index / OOB | `if (bm->TACamt[species][nf][now_id] > no_quota) bm->TACamt[species][nf][now_id] = no_quota;` is inside the `n… |  | [ ] |
| [TIER-04](#tier-04) ⭐ | 458 | H/H | ordering / wrong value used | `RBC = RBCparam[RBCest_id];` is read *before* `Apply_RBC_Buffers()` changes RBCest_id. The TAC is then worked … |  | [ ] |
| [TIER-05](#tier-05) | 343 | H/M | accumulator / loop | `for (nf...) for (nreg < Nregions) current_catch += Harvest_Get_TotCumCatch(species, nf, bm->thisyear);` The i… |  | [ ] |
| [TIER-06](#tier-06) | 405 | M/M | switch fall-through | `case tier8: case tier9: case tier13: RBC = 0.0;` has no break, so it falls into `default` and quits with "No … |  | [ ] |
| [TIER-07](#tier-07) ⭐ | 3751, 3409 | H/H | switch fall-through / wrong source | `case sp_rollover:` has no `break`, so it falls through to `default` and calls `quit("")`. It is also unreacha… |  | [ ] |
| [TIER-08](#tier-08) | 809, 790 | M/M | off-by-one / sanity logic | `if (MinCyr > MaxCyr) MaxCyr = MinCyr;` can push MaxCyr past `year` (future, unfilled years), so it should pro… |  | [ ] |
| [TIER-09](#tier-09) | 822, 818, 834 | M/M | accumulator not reset | `Util_Init_1D_Double(discrate, nyears, 0.0)` zeroes indices 0..nyears-1 (the number of averaging years), not t… |  | [ ] |
| [TIER-10](#tier-10) | 825 | M/M | uninitialised data / stale variable | `DiscType = ...DiscardType[nf]`: DiscardType is allocated with size nLen (atAssessTierSetup.c:130), is indexed… |  | [ ] |
| [TIER-11](#tier-11) | 746 | M/M | wrong divisor | `avprop_reg[nf][nreg] /= num_avg_year;` but the sum covers `startyear..year` (5 years, or fewer near the start… |  | [ ] |
| [TIER-12](#tier-12) | 1159 | M/M | cohort vs age / double counting | `natm = Mnat[0][0][s][a][index2]` uses `a` (a cohort×genotype index, up to numCohortsXnumGenes) as the *age-in… |  | [ ] |
| [TIER-13](#tier-13) ⭐ | 1271, 1223, 1287 | H/H | off-by-one / heap overflow | `for (ia = 0; ia <= this_nselage; ia++) ... propgt05[ia] = 1;` reads `numatage[nselage]` and writes `propgt05[… |  | [ ] |
| [TIER-14](#tier-14) | 1261 | M/M | 1-based leftover / dead code | `if ((a == AgeSel95+1) && (numatage[1] < numatage[2]))`: at this point `numatage[2]` has not been filled yet (… |  | [ ] |
| [TIER-15](#tier-15) ⭐ | 1313, 1316 | H/H | off-by-one | `nz` is the index of the last stored Z (it starts at -1 and is incremented before use), but `for (i=0; i<nz; i… |  | [ ] |
| [TIER-16](#tier-16) | 1513 | H/M | uninitialised / not reset | `NCA` is initialised to 0 at its declaration and only updated when a year has data. It is not reset per fleet … |  | [ ] |
| [TIER-17](#tier-17) ⭐ | 1575 | H/H | copy-paste / wrong index | Restart branch: `fvalF[nF] = UnTransform(simplex[1][1], 0.01);` takes the S95 parameter of the second vertex. … |  | [ ] |
| [TIER-18](#tier-18) ⭐ | 1460 | H/H | off-by-one | `nF` is the index of the last fitted fleet (it starts at -1, then ++), but the loops `i < nF` and the divisor … |  | [ ] |
| [TIER-19](#tier-19) ⭐ | 1757 | H/H | off-by-one | `for (iy=0; iy < Ncage_id; iy++)`: Ncage_id is the last index (it starts at -1), so the last year of catch-at-… |  | [ ] |
| [TIER-20](#tier-20) | 1744 | M/M | unreachable code | The plus-group branch `else if (a == Tier3_maxage)` can never run inside `for (a < Tier3_maxage)`, so the plus… |  | [ ] |
| [TIER-21](#tier-21) ⭐ | 1885 | H/H | wrong formula | `step1 = (F40-F20)*(Fcurr-F20); frbc = F48 / step1;` is dimensionally wrong (1/F). The linear ramp between F40… |  | [ ] |
| [TIER-22](#tier-22) | 1966 | L/L | off-by-one | `fval` is updated at the *end* of the loop as `i/1000.0`, so iterations 0 and 1 both use F=0 and every later F… |  | [ ] |
| [TIER-23](#tier-23) | 2014 | M/M | cohort vs age | YPR loops over `numCohorts`, but the `wt/mat/sel` arrays it is given are indexed by age in years (filled up to… |  | [ ] |
| [TIER-24](#tier-24) | 2122 | H/M | 1-based leftover / OOB | `maxprop = avprop[1]; maxfleet = 1;` reads out of bounds when Nfleets==1 (avprop has size Nfleets) and default… |  | [ ] |
| [TIER-25](#tier-25) | 2204 | M/M | off-by-one | The CPUE target loop `iy < MaxCyr` excludes the last year, but the divisor `step1 = yrmax - yrmin + 1` counts … |  | [ ] |
| [TIER-26](#tier-26) | 2243 | H/M | off-by-one | `startyear = year - m; for (iy=startyear; iy<=year; ...)` sums m+1 years but divides by m (or by `year-startye… |  | [ ] |
| [TIER-27](#tier-27) ⭐ | 2452 | H/H | copy-paste / wrong index | `k = simplex[0][0]; xpar[1] = simplex[0][0];` and in the objective `in_K = xpar[0];`. K is parameter 1 (commen… |  | [ ] |
| [TIER-28](#tier-28) ⭐ | 2562 | H/H | misplaced parenthesis | `step1 = (1 - (bt/K)^p) - T5_removal[iy-1]; B = bt + bt*r/p*step1;` multiplies the catch removal by `bt*r/p`. … |  | [ ] |
| [TIER-29](#tier-29) | 2597, 2605 | M/H | off-by-one / uninitialised read | The projection loop `iy < index2` runs zero times for AssessDelay 0 and only to CurrentYear for delay 1. Then … |  | [ ] |
| [TIER-30](#tier-30) | 2642, 2646 | H/M | off-by-one / heap overflow | `do { iy++; ... TSbiomass[iy] = ... } while (... && iy < 1000.0);` can write `TSbiomass[1000]`, but the array … |  | [ ] |
| [TIER-31](#tier-31) | 2752 | H/L | off-by-one / OOB read | `do { i=i+1; } while ((avlen < T5_Avlen_atF[i]) && (i < nfint));` evaluates `T5_Avlen_atF[nfint]` (array size … |  | [ ] |
| [TIER-32](#tier-32) ⭐ | 2806 | H/H | accumulator reset in wrong loop | `for (iy...) { sumcatch = 0.0; ... }` resets the sum every year, so DCAC uses only the final year's catch and … |  | [ ] |
| [TIER-33](#tier-33) | 2939 | H/M | accumulator not reset | `localcatch` is not reset per year, so `HistCatch` ends up as cumulative total catch rather than the peak annu… |  | [ ] |
| [TIER-34](#tier-34) | 2949, 2952, 2957 | M/M | wrong variable reset | `RBCspeciesParam[species][TriggerReached_id] = 0;` is reset per region, but the array `TriggerReached[nreg]` (… |  | [ ] |
| [TIER-35](#tier-35) ⭐ | 2993, 3002, 3018 | H/H | swapped arguments | `DoCPUE(bm, year, species, llogfp)`, `DoPrager(bm, year, species, ...)`, `Call_Tier3(bm, year, species, ...)` … |  | [ ] |
| [TIER-36](#tier-36) ⭐ | 3106 | H/H | wrong operator / logic | `for (iy = year-1; iy >= 0; iy++)` counts *up* without bound (runs off the end of EffortData). Also `j = iy` i… |  | [ ] |
| [TIER-37](#tier-37) ⭐ | 3130 | H/H | off-by-one / heap overflow | `AnnCPUE[5-iy]` with iy going down to 0 writes `AnnCPUE[5]`, but AnnCPUE is allocated with 5 entries (atAssess… |  | [ ] |
| [TIER-38](#tier-38) ⭐ | 3211 | H/H | wrong index | `cosp = FunctGroupArray[sp].co_sp[sp];` uses the loop counter as the group index. It should be the focal speci… |  | [ ] |
| [TIER-39](#tier-39) ⭐ | 3313 | H/H | stale index / OOB | `if (bm->SP_FISHERYprms[cosp][nf][q_id] > 0.0) toteffort += bm->CumEffort[nf][b];` sits after the nf loops hav… |  | [ ] |
| [TIER-40](#tier-40) | 3323 | M/M | accumulator not reset | `areafished` and `areasp` (and `toteffort`) are initialised once at function entry and not per co-species, so … |  | [ ] |
| [TIER-41](#tier-41) ⭐ | 3383 | H/H | swapped args / wrong species | `Call_Tier3(bm, year, species, llogfp)` has its arguments swapped, and inside the loop over cosp it should ass… |  | [ ] |
| [TIER-42](#tier-42) | 4000 | M/M | accumulator not reset / off-by-one | `catchwghtCPUE` is zeroed once outside the `lag` loop, so every lag adds into the same slots. The regression f… |  | [ ] |
| [TIER-43](#tier-43) | 4067 | M/M | wrong variable | `(FunctGroupArray[sp].isImpacted == FALSE)` tests the target species and not the candidate indicator `i`. Func… |  | [ ] |
| [TIER-44](#tier-44) | 4157 | M/M | wrong constant / accumulator | The `yy` offset uses `GradientPeriod` (the lag count) and not `GradientWindow` (the array length). Indices can… |  | [ ] |
| [TIER-45](#tier-45) | 4206 | H/M | copy-paste | The else-branch sets `CPUEtrend_id = 0` where it should set `CPUEdir_id = 0`, so the direction stays stale and… |  | [ ] |
| [TIER-46](#tier-46) ⭐ | 4261 | H/H | missing update | `if (diff < closest_slope) ref_sp = ind_sp;` never updates `closest_slope` (or `closest_slopeB`), so ref_sp en… |  | [ ] |
| [TIER-47](#tier-47) | 4229 | H/M | inconsistency | The other-category diff is missing `fabs()` (the same-category loop at 3949 has it), so any species with a mor… |  | [ ] |
| [TIER-48](#tier-48) | 4267 | H/M | wrong variable | `maxTier = mgt_category_id;` defaults to the enum index constant and not the species' current category (`my_ca… |  | [ ] |
| [TIER-49](#tier-49) ⭐ | 5236, 5248 | H/H | state not reset / use-after-close | `chdir(dirname)` never returns to the parent directory, so later relative file I/O and next year's mkdir end u… |  | [ ] |
| [TIER-50](#tier-50) ⭐ | 5378 | H/H | copy-paste | The third block under the "Year, Catch and CPUE" header writes `CatchData` a second time where CPUE (`CPUEgen`… |  | [ ] |
| [TIER-51](#tier-51) | 3242 | L/M | uninitialised counter | `t` is never incremented, so `t <= HistYrMax` is always true and the retro-catch adjustment applies to all yea… |  | [ ] |
| [TIER-52](#tier-52) | 5577 | H/L | statement inside comment | `...= 0.0; // temporary value ... bm->RBCestimation.RBCspeciesParam[species][EstB0_id] = 999.9;` puts the EstB… |  | [ ] |
| [TIER-53](#tier-53) | 5606 | H/M | off-by-one / wrong index | The loop `yr < mgt_rbcYear + n - 1` sums n-1 RBCs but divides by n. The replacement loop writes `RBC_by_year[y… |  | [ ] |
| [TIER-54](#tier-54) | 3410 | M/L | wrong array | `tiertype = FunctGroupArray[species].speciesParams[tiertype_id]`: tiertype_id belongs to the RBC_SPECIES_PARAM… |  | [ ] |
| [TIER-55](#tier-55) | 3425, 3435, 3459 | M/L | wrong index (log only) | `sb0 = SBioZero[0][0]` is used with `SpawnBio[Nstocks][Nregions]` (elsewhere the overall value is at [Nstocks]… |  | [ ] |
| [TIER-56](#tier-56) | 1867 | H/L | debug leftover | Hard-coded `if (bm->dayt > 10584) printf(...)` debug prints left in live code. Functions: `newTier3Rule()`, `T… |  | [ ] |

---

## TIER-01

**wrong operator / sign** · Conf H / Sev H ⭐ priority  
**Found at (current code):** `atManageTier.c` 580
**Lines at review (Oct 2026):** 487-492

**Function:** `DoTierAssessment()`

**Why it's thought to be a bug**

`tac_change = (TAC_old - TACpost)/TAC_old; if (tac_change > inc_check) TAC = TAC_old + inc_check*TAC_old; else if (tac_change < bm->prop_decTAC) TAC = TAC_old - dec_check*TAC_old;` The sign is reversed: a big proposed *decrease* gives a positive tac_change, which then *raises* the TAC to the max. The second test compares against the raw multiplier `prop_decTAC` (e.g. 0.5) and not `-dec_check`, so almost every other case is pushed to the minimum TAC. prop_inc/decTAC are multipliers (see atManageAnnual.c:852-853). Also divides by zero when TAC_old==0. Function: `DoTierAssessment()`.

**Code at issue** — `atManageTier.c` line 580  
Search for: `- dec_check`

```c
  578: 					TAC = TAC_old * (1.0 + inc_check);
  579: 				else if (tac_change < -dec_check)
  580→ 					TAC = TAC_old * (1.0 - dec_check);
  581: 				else
  582: 					TAC = bm->RBCestimation.RBCspeciesParam[species][TACpost_id];
```

**Suggested fix (from review):** `tac_change = (TACpost - TAC_old)/(TAC_old+small_num); if (tac_change > inc_check) ... else if (tac_change < -dec_check) ...`

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

## TIER-02

**wrong variable** · Conf H / Sev H ⭐ priority  
**Found at (current code):** `atManageTier.c` 605
**Lines at review (Oct 2026):** 501-503

**Function:** `DoTierAssessment()`

**Why it's thought to be a bug**

`if (TAC < minCatch) TAC = FunctGroupArray[species].speciesParams[MinCatch_id];` sets the TAC to the fraction (e.g. 0.05) and not to the computed `minCatch` (fraction × last historical catch). Function: `DoTierAssessment()`.

**Code at issue** — `atManageTier.c` line 605 (located by name only; check this is the right place)  
Search for: `bm->dayt, FunctGroupArray[species].groupCode, year, TAC, tac_change, TAC_old, bm->RBCestim`

```c
  603:             if (do_debug) {
  604:                 fprintf(llogfp, "Time: %e, %s, year: %d TAC: %e tac_change: %e TAC_old: %e TACpost: %e  min_TACchange %e minCatch: %e MinCatch: %e lastHistCatch: %e inc_check: %e dec_check: %e prop_decTAC: %e min_TACchange: %e\n",
  605→                         bm->dayt, FunctGroupArray[species].groupCode, year, TAC, tac_change, TAC_old, bm->RBCestimation.RBCspeciesParam[species][TACpost_id], bm->RBCestimation.min_TACchange, minCatch, bm->RBCestimation.RBCspeciesParam[species][MinCatch_id], lastHistCatch, inc_check, dec_check, bm->prop_decTAC, bm->RBCestimation.min_TACchange);
  606:             }
  607:             */
```

**Suggested fix (from review):** `TAC = minCatch;`

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

## TIER-03

**wrong index / OOB** · Conf H / Sev H ⭐ priority  
**Found at (current code):** `atManageTier.c` 637
**Lines at review (Oct 2026):** 535-536

**Function:** `DoTierAssessment()`

**Why it's thought to be a bug**

`if (bm->TACamt[species][nf][now_id] > no_quota) bm->TACamt[species][nf][now_id] = no_quota;` is inside the `nfishery` loop but uses the stale `nf`, which equals K_num_fisheries (or Nfleets) left over from earlier loops. That is an out-of-bounds read/write on TACamt[species], and the cap is never applied to the fishery actually being updated. Function: `DoTierAssessment()`.

**Code at issue** — `atManageTier.c` line 637 (located by name only; check this is the right place)  
Search for: `if (bm->TACamt[species][nfishery][now_id] > no_quota)`

```c
  635:             bm->TACamt[species][nfishery][RBCnow_id] = RBCnf;
  636:             
  637→             if (bm->TACamt[species][nfishery][now_id] > no_quota)
  638:                 bm->TACamt[species][nfishery][now_id] = no_quota;
  639:             
```

**Suggested fix (from review):** Use `nfishery`

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

## TIER-04

**ordering / wrong value used** · Conf H / Sev H ⭐ priority  
**Found at (current code):** `atManageTier.c` 458
**Lines at review (Oct 2026):** 394-399

**Function:** `DoTierAssessment()`

**Why it's thought to be a bug**

`RBC = RBCparam[RBCest_id];` is read *before* `Apply_RBC_Buffers()` changes RBCest_id. The TAC is then worked out from the local, unbuffered `RBC` (lines ≈471/≈471/629), so tier buffers have no effect on the TAC. Function: `DoTierAssessment()`.

**Code at issue** — `atManageTier.c` line 458 (located by name only; check this is the right place)  
Search for: `inside Apply_RBC_Buffers so only worked with the tier buffers on). Not applied if using th`

```c
  456:         Nov 2026 (review item 29): multi-year TAC decay - one more year of (1 - myTACbuffer) on last year's TAC in
  457:         rollover years within a multi-year TAC. Now applied whether or not UseTierBuffers is set (it used to be
  458→         inside Apply_RBC_Buffers so only worked with the tier buffers on). Not applied if using the Ratpack
  459:         staleness buffer. **/
  460:     if (tier == sp_rollover) {
```

**Suggested fix (from review):** Move the `RBC = ...RBCest_id` assignment to after `Apply_RBC_Buffers()`

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

## TIER-05

**accumulator / loop** · Conf H / Sev M  
**Found at (current code):** `atManageTier.c` 343
**Lines at review (Oct 2026):** 325-329 (also 1924-1927)

**Why it's thought to be a bug**

`for (nf...) for (nreg < Nregions) current_catch += Harvest_Get_TotCumCatch(species, nf, bm->thisyear);` The inner region loop does not depend on nreg, so the total catch is multiplied by Nregions. Functions: `DoTierAssessment()`, `Call_Tier3()`.

**Code at issue** — `atManageTier.c` line 343  
Search for: `for (nf`

```c
  341: 			// if using true F use only final year's catch + true discard ***************
  342: 			current_catch = 0.0;
  343→ 			for (nf = 0; nf < bm->K_num_fisheries; nf++)
  344: 				for (nreg = 0; nreg < Nregions; nreg++)
  345: 					current_catch += Harvest_Get_TotCumCatch(species, nf, bm->thisyear);   // True catchs and discards
```

**Suggested fix (from review):** Drop the nreg loop

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

## TIER-06

**switch fall-through** · Conf M / Sev M  
**Found at (current code):** `atManageTier.c` 405
**Lines at review (Oct 2026):** 376-384

**Function:** `DoTierAssessment()`

**Why it's thought to be a bug**

`case tier8: case tier9: case tier13: RBC = 0.0;` has no break, so it falls into `default` and quits with "No such tier". Function: `DoTierAssessment()`.

**Code at issue** — `atManageTier.c` line 405 (the line the review pointed to, found by its text)  
Search for: `case tier8:  // US method`

```c
  403:             bm->RBCestimation.RBCspeciesParam[species][RBCest_id] = TAC_old;
  404:         break;
  405→     case tier8:  // US method
  406:     case tier9:  // Norwegian method
  407:     case tier13:  // Icealndic escapement method
```

**Suggested fix (from review):** Add `break` (or a clear "not implemented" quit)

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

## TIER-07

**switch fall-through / wrong source** · Conf H / Sev H ⭐ priority  
**Found at (current code):** `atManageTier.c` 3751, `atManageTier.c` 3409
**Lines at review (Oct 2026):** 3539-3549

**Function:** `Apply_RBC_Buffers()`

**Why it's thought to be a bug**

`case sp_rollover:` has no `break`, so it falls through to `default` and calls `quit("")`. It is also unreachable in practice because `tier` is re-read from `speciesParams[tier_id]` and not from the local `tier` that DoTierAssessment sets to sp_rollover (lines 279/289). As a result a rolled-over TAC gets the base tier's buffer (e.g. sig1) and not the MYTAC buffer. Function: `Apply_RBC_Buffers()`.

**Code at issue** — `atManageTier.c` line 3751  
Search for: `quit("")`

```c
 3749:             printf("No such tier (%d)\n", tier);
 3750:             fprintf(llogfp,"No such tier (%d)\n", tier);
 3751→             quit("");
 3752:             break;
 3753:             
```

**Code at issue** — `atManageTier.c` line 3409  
Search for: `speciesParams[tier_id]`

```c
 3407: 	int  r;
 3408: 	double sb0;
 3409→ 	int tier = (int) (FunctGroupArray[species].speciesParams[tier_id]);
 3410: 	int tiertype = (int) (bm->RBCestimation.RBCspeciesParam[species][tiertype_id]);   // Nov 2026: was read from speciesParams
 3411: 	double step1;
```

**Suggested fix (from review):** Add `break`; pass the effective tier in as a parameter

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

## TIER-08

**off-by-one / sanity logic** · Conf M / Sev M  
**Found at (current code):** `atManageTier.c` 809, `atManageTier.c` 790
**Lines at review (Oct 2026):** 702-712

**Function:** `Tier_averages()`

**Why it's thought to be a bug**

`if (MinCyr > MaxCyr) MaxCyr = MinCyr;` can push MaxCyr past `year` (future, unfilled years), so it should probably be `MinCyr = MaxCyr`. `nyrs` is computed *before* `MaxCyr` is bumped from 1 to 2, so the mean at 763-771 adds an extra year but divides by the old count. Function: `Tier_averages()`.

**Code at issue** — `atManageTier.c` line 809  
Search for: `if (MinCyr > MaxCyr) MaxCyr = MinCyr`

```c
  807:     if (MaxCyr > year) MaxCyr = year;
  808:     if (MinCyr < 1) MinCyr = 1;
  809→     if (MinCyr > MaxCyr) MaxCyr = MinCyr;
  810:     
  811: 	nyears = MaxCyr - MinCyr + 1;
```

**Code at issue** — `atManageTier.c` line 790  
Search for: `MinCyr = MaxCyr`

```c
  788:         if (MaxCyr > year) {
  789:             MaxCyr = year;
  790→             MinCyr = MaxCyr - diffyr;
  791:         }
  792:             
```

**Suggested fix (from review):** Clamp MinCyr instead; recompute nyrs after the MaxCyr fix-up

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

## TIER-09

**accumulator not reset** · Conf M / Sev M  
**Found at (current code):** `atManageTier.c` 822, `atManageTier.c` 818, `atManageTier.c` 834
**Lines at review (Oct 2026):** 718-743

**Function:** `Tier_averages()`

**Why it's thought to be a bug**

`Util_Init_1D_Double(discrate, nyears, 0.0)` zeroes indices 0..nyears-1 (the number of averaging years), not the absolute years `year-3..year` that are used afterwards. Inside the fleet loop `discrate[iy]` is only overwritten when `DiscYears[nf][iy] > 0`, so fleets without discard data inherit the previous fleet's rate (or values from line 911 of the previous call). Function: `Tier_averages()`.

**Code at issue** — `atManageTier.c` line 822 (closest match to the quoted code; wording differs slightly)  
Search for: `Util_Init_1D_Double(bm->RBCestimation.RBCspeciesArray[species].discrate, nyears, 0.0);`

```c
  820: // calculate average total catch (discard + retained) over last nyears years for each fleet, sum over fleets (for Tier 3 rule)
  821: 	curcatch = 0.0;
  822→     Util_Init_1D_Double(bm->RBCestimation.RBCspeciesArray[species].discrate, nyears, 0.0);
  823: 
  824: 	for (nf = 0; nf < Nfleets; nf++){
```

**Code at issue** — `atManageTier.c` line 818 (closest match to the quoted code; wording differs slightly)  
Search for: `fprintf(bm->logFile, "Time: %e %s year: %d MaxCyr: %d MinCyr: %d\n", bm->dayt, FunctGroupA`

```c
  816:     }
  817:     
  818→     fprintf(bm->logFile, "Time: %e %s year: %d MaxCyr: %d MinCyr: %d\n", bm->dayt, FunctGroupArray[species].groupCode, year, MaxCyr, MinCyr);
  819: 
  820: // calculate average total catch (discard + retained) over last nyears years for each fleet, sum over fleets (for Tier 3 rule)
```

**Code at issue** — `atManageTier.c` line 834  
Search for: `discrate[iy]`

```c
  832: 			{
  833: 				if (DiscType == 1)
  834→ 					bm->RBCestimation.RBCspeciesArray[species].discrate[iy] = bm->RBCestimation.RBCspeciesArray[species].DiscData[nf][sumregion][iy]
  835: 					               / (bm->RBCestimation.RBCspeciesArray[species].DiscData[nf][sumregion][iy]
  836: 					                  + bm->RBCestimation.RBCspeciesArray[species].CatchData[nf][sumregion][iy]);
```

**Suggested fix (from review):** Reset `discrate[startyear..year]` at the start of each fleet iteration

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

## TIER-10

**uninitialised data / stale variable** · Conf M / Sev M  
**Found at (current code):** `atManageTier.c` 825
**Lines at review (Oct 2026):** 721, 800-803

**Function:** `Tier_averages()`

**Why it's thought to be a bug**

`DiscType = ...DiscardType[nf]`: DiscardType is allocated with size nLen (atAssessTierSetup.c:130), is indexed by fleet, and is never filled anywhere in the repo, so it is always 0. Neither DiscType branch (1/2) ever runs, so discard rates are always 0, or NaN when there is no catch (911). The all-fleet block at 800-803 also uses the `DiscType` left over from the last fleet and not each fleet's own. Function: `Tier_averages()`.

**Code at issue** — `atManageTier.c` line 825  
Search for: `DiscType =`

```c
  823: 
  824: 	for (nf = 0; nf < Nfleets; nf++){
  825→ 		DiscType = bm->RBCestimation.RBCspeciesArray[species].DiscardType[nf];
  826: 
  827:         // calculate weighted average discard rate over last 4 years by fleet
```

**Suggested fix (from review):** Allocate DiscardType with K_num_fisheries and read it from params; look up per fleet in the 799 loop

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

## TIER-11

**wrong divisor** · Conf M / Sev M  
**Found at (current code):** `atManageTier.c` 746
**Lines at review (Oct 2026):** 644

**Function:** `Tier_averages()`

**Why it's thought to be a bug**

`avprop_reg[nf][nreg] /= num_avg_year;` but the sum covers `startyear..year` (5 years, or fewer near the start). When num_avg_year != the number of years summed, avprop does not sum to 1. Function: `Tier_averages()`.

**Code at issue** — `atManageTier.c` line 746 (closest match to the quoted code; wording differs slightly)  
Search for: `bm->RBCestimation.RBCspeciesArray[species].avprop_reg[nf][nreg] /= bm->RBCestimation.num_a`

```c
  744: 				bm->RBCestimation.RBCspeciesArray[species].avprop_reg[nf][nreg] += bm->RBCestimation.RBCspeciesArray[species].propcatch_reg[nf][nreg][iy];
  745: 			}
  746→ 			bm->RBCestimation.RBCspeciesArray[species].avprop_reg[nf][nreg] /= bm->RBCestimation.num_avg_year;
  747: 			bm->RBCestimation.RBCspeciesArray[species].avprop[nf] += bm->RBCestimation.RBCspeciesArray[species].avprop_reg[nf][nreg];
  748: 		}
```

**Suggested fix (from review):** Divide by `(year - startyear + 1)`

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

## TIER-12

**cohort vs age / double counting** · Conf M / Sev M  
**Found at (current code):** `atManageTier.c` 1159
**Lines at review (Oct 2026):** 997-1022

**Function:** `TierOneMimic()`

**Why it's thought to be a bug**

`natm = Mnat[0][0][s][a][index2]` uses `a` (a cohort×genotype index, up to numCohortsXnumGenes) as the *age-in-years* index of Mnat. `num_individuals` and `bodysize` are also summed over all individuals regardless of sex, and then added once per sex, so the yield is doubled when K_num_sexes=2. Function: `TierOneMimic()`.

**Code at issue** — `atManageTier.c` line 1159 (closest match to the quoted code; wording differs slightly)  
Search for: `natm = bm->RBCestimation.RBCspeciesArray[species].Mnat[0][0][s][a][index2];`

```c
 1157:             }
 1158:             
 1159→             natm = bm->RBCestimation.RBCspeciesArray[species].Mnat[0][0][s][a][index2];
 1160:             sf = bm->RBCestimation.RBCspeciesArray[species].agesel_bysex[s][ia] * ftarg;
 1161:             if( natm || sf ) {
```

**Suggested fix (from review):** Map cohort to age (`ia*ageClassSize`); split numbers by sex or drop the sex loop

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

## TIER-13

**off-by-one / heap overflow** · Conf H / Sev H ⭐ priority  
**Found at (current code):** `atManageTier.c` 1271, `atManageTier.c` 1223, `atManageTier.c` 1287
**Lines at review (Oct 2026):** 1131, 1147

**Function:** `TierThree_CC()`

**Why it's thought to be a bug**

`for (ia = 0; ia <= this_nselage; ia++) ... propgt05[ia] = 1;` reads `numatage[nselage]` and writes `propgt05[nselage]`. Both are allocated with size nselage (atAssessTierSetup.c:466-468). The do-while at 1147 likewise reads `propgt05[ia+1]` with ia+1 == this_nselage. This is left over from Sally's 1-based code. Function: `TierThree_CC()`.

**Code at issue** — `atManageTier.c` line 1271  
Search for: `for ( ia = 0; ia <= this_nselage; ia++)`

```c
 1269: 					bm->RBCestimation.RBCspeciesArray[species].ages[ia] = a;
 1270: 				}
 1271→ 				for ( ia = 0; ia <= this_nselage; ia++){
 1272: 					if( (float)(bm->RBCestimation.RBCspeciesArray[species].numatage[ia] / sumnum) > 0.05)
 1273: 						bm->RBCestimation.RBCspeciesArray[species].propgt05[ia] = 1;
```

**Code at issue** — `atManageTier.c` line 1223 (closest match to the quoted code; wording differs slightly)  
Search for: `Util_Init_1D_Double(bm->RBCestimation.RBCspeciesArray[species].numatage, nselage, 0.0);`

```c
 1221:     Util_Init_1D_Double(bm->RBCestimation.RBCspeciesArray[species].wt, maxage, 0.0);
 1222:     
 1223→     Util_Init_1D_Double(bm->RBCestimation.RBCspeciesArray[species].numatage, nselage, 0.0);
 1224:     Util_Init_1D_Double(bm->RBCestimation.RBCspeciesArray[species].ages, nselage, 0.0);
 1225:     Util_Init_1D_Int(bm->RBCestimation.RBCspeciesArray[species].propgt05, nselage, 0);
```

**Code at issue** — `atManageTier.c` line 1287  
Search for: `propgt05[ia+1]`

```c
 1285: 				do{
 1286: 					ia++;
 1287→ 					isum05 = bm->RBCestimation.RBCspeciesArray[species].propgt05[ia-1] + bm->RBCestimation.RBCspeciesArray[species].propgt05[ia] + bm->RBCestimation.RBCspeciesArray[species].propgt05[ia+1];
 1288: 				}
 1289: 				while ((ia <= this_nselage-2) && (isum05 > 0));  // change from ia<nselage-2 && isum05>0   14-8-08
```

**Suggested fix (from review):** Use `ia < this_nselage`; bound the do-while at `ia+1 < this_nselage`

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

## TIER-14

**1-based leftover / dead code** · Conf M / Sev M  
**Found at (current code):** `atManageTier.c` 1261
**Lines at review (Oct 2026):** 1121-1126

**Function:** `TierThree_CC()`

**Why it's thought to be a bug**

`if ((a == AgeSel95+1) && (numatage[1] < numatage[2]))`: at this point `numatage[2]` has not been filled yet (it is 0), so the test can never be true and the "drop first age if lower than next" logic never runs. In 0-based terms it should compare `numatage[0] < numatage[1]`. Function: `TierThree_CC()`.

**Code at issue** — `atManageTier.c` line 1261 (closest match to the quoted code; wording differs slightly)  
Search for: `if ( (a == AgeSel95 + 1 ) && ( bm->RBCestimation.RBCspeciesArray[species].numatage[1] < bm`

```c
 1259: 					// the following throws out the first nos at age if nos < nos at next age (to avoid -Z)
 1260: 					//  make more comparable to doing this by eye
 1261→ 					if ( (a == AgeSel95 + 1 ) && ( bm->RBCestimation.RBCspeciesArray[species].numatage[1] < bm->RBCestimation.RBCspeciesArray[species].numatage[2] ) ){
 1262: 						ia = ia - 1;
 1263: 						bm->RBCestimation.RBCspeciesArray[species].numatage[1] = bm->RBCestimation.RBCspeciesArray[species].numatage[2];
```

**Suggested fix (from review):** Translate to 0-based indices

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

## TIER-15

**off-by-one** · Conf H / Sev H ⭐ priority  
**Found at (current code):** `atManageTier.c` 1313, `atManageTier.c` 1316
**Lines at review (Oct 2026):** 1173-1176

**Function:** `TierThree_CC()`

**Why it's thought to be a bug**

`nz` is the index of the last stored Z (it starts at -1 and is incremented before use), but `for (i=0; i<nz; i++)` and `meanz = sumz/nz` treat it as a count. The last Z is dropped, and a single valid Z (nz==0) is reported as an assessment failure. Function: `TierThree_CC()`.

**Code at issue** — `atManageTier.c` line 1313  
Search for: `for (i=0; i < nz; i++)`

```c
 1311: // average Z values over years and fleets
 1312: 	sumz = 0.0;
 1313→ 	for (i=0; i < nz; i++)
 1314: 		sumz += bm->RBCestimation.RBCspeciesArray[species].z[i];
 1315: 	if (nz > 0){
```

**Code at issue** — `atManageTier.c` line 1316  
Search for: `meanz = sumz / nz`

```c
 1314: 		sumz += bm->RBCestimation.RBCspeciesArray[species].z[i];
 1315: 	if (nz > 0){
 1316→ 		meanz = sumz / nz;
 1317: 		bm->RBCestimation.RBCspeciesParam[species][Fcurr_id] =  meanz - bm->RBCestimation.RBCspeciesParam[species][Tier3_M_id] ;
 1318: 	}
```

**Suggested fix (from review):** Use `nz+1` as the count (`i <= nz`, `if (nz >= 0)`, `/(nz+1)`)

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

## TIER-16

**uninitialised / not reset** · Conf H / Sev M  
**Found at (current code):** `atManageTier.c` 1513
**Lines at review (Oct 2026):** 1345, 1373

**Function:** `TierThree_CCsel()`

**Why it's thought to be a bug**

`NCA` is initialised to 0 at its declaration and only updated when a year has data. It is not reset per fleet (only `Ncage_id` is), so a fleet with no data reuses the previous fleet's NCA (and the first fleet uses 0). `if (NCA >= 0)` is therefore always true and the minimisation runs on stale or empty Obsageprop. Function: `TierThree_CCsel()`.

**Code at issue** — `atManageTier.c` line 1513  
Search for: `if (NCA >= 0)`

```c
 1511:         */
 1512:         
 1513→ 		if (NCA >= 0){    // only do the minimisation if have data from this fleet
 1514: 			// call powell to estimate, S50 and S95
 1515: 
```

**Suggested fix (from review):** Set `NCA = -1` at the top of each fleet iteration

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

## TIER-17

**copy-paste / wrong index** · Conf H / Sev H ⭐ priority  
**Found at (current code):** `atManageTier.c` 1575
**Lines at review (Oct 2026):** 1435

**Function:** `TierThree_CCsel()`

**Why it's thought to be a bug**

Restart branch: `fvalF[nF] = UnTransform(simplex[1][1], 0.01);` takes the S95 parameter of the second vertex. The F estimate is `simplex[0][0]`, as used at line 1551. Function: `TierThree_CCsel()`.

**Code at issue** — `atManageTier.c` line 1575 (closest match to the quoted code; wording differs slightly)  
Search for: `bm->RBCestimation.RBCspeciesArray[species].fvalF[nF] = UnTransform(bm->RBCestimation.RBCsp`

```c
 1573: 				downhill_simplex(bm, EquilF_Funk_model, species, bm->RBCestimation.RBCspeciesArray[species].simplex, bm->RBCestimation.RBCspeciesArray[species].func, npar, ftol, iter, &ok, llogfp, bm->RBCestimation.RBCspeciesArray[species].xpar);
 1574: 
 1575→ 				bm->RBCestimation.RBCspeciesArray[species].fvalF[nF] = UnTransform(bm->RBCestimation.RBCspeciesArray[species].simplex[0][0], 0.01);   // Nov 2026: was simplex[1][1] (S95 of a non-best vertex)
 1576:  
 1577: 			}
```

**Suggested fix (from review):** `UnTransform(simplex[0][0], 0.01)`

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

## TIER-18

**off-by-one** · Conf H / Sev H ⭐ priority  
**Found at (current code):** `atManageTier.c` 1460
**Lines at review (Oct 2026):** 1459, 1472, 1478, 1482

**Function:** `TierThree_CCsel()`

**Why it's thought to be a bug**

`nF` is the index of the last fitted fleet (it starts at -1, then ++), but the loops `i < nF` and the divisor `nF` treat it as a count. With one fleet (nF=0) Fcurr=0 and agesel is all zeros; with more fleets the last fleet is dropped. Function: `TierThree_CCsel()`.

**Code at issue** — `atManageTier.c` line 1460 (the review quoted no code: this is the line it pointed to, found by its text; check)  
Search for: `for (s = 0; s < bm->K_num_sexes; s++){`

```c
 1458: 	kpar = 0.0;
 1459: 	t0 = 0.0;
 1460→ 	for (s = 0; s < bm->K_num_sexes; s++){
 1461: 		linf += bm->RBCestimation.RBCspeciesArray[species].Tier3_Linf[s];
 1462: 		kpar += bm->RBCestimation.RBCspeciesArray[species].Tier3_k[s];
```

**Suggested fix (from review):** Use `nF+1` as the count

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

## TIER-19

**off-by-one** · Conf H / Sev H ⭐ priority  
**Found at (current code):** `atManageTier.c` 1757
**Lines at review (Oct 2026):** 1614

**Function:** `EquilF_Funk()`

**Why it's thought to be a bug**

`for (iy=0; iy < Ncage_id; iy++)`: Ncage_id is the last index (it starts at -1), so the last year of catch-at-age is left out of the objective. With one year the objective is constant (ss=0), so the fit is meaningless. Function: `EquilF_Funk()`.

**Code at issue** — `atManageTier.c` line 1757 (closest match to the quoted code; wording differs slightly)  
Search for: `for (iy=0; iy < bm->RBCestimation.RBCspeciesParam[species][Ncage_id]; iy++){`

```c
 1755: 
 1756: 	ss = 0.0;
 1757→ 	for (iy=0; iy < bm->RBCestimation.RBCspeciesParam[species][Ncage_id]; iy++){
 1758: 		for (a=0; a < bm->RBCestimation.RBCspeciesParam[species][Tier3_maxage_id]; a++) {
 1759:  			if (bm->RBCestimation.RBCspeciesArray[species].Obsageprop[iy][a] != 0.0)
```

**Suggested fix (from review):** `iy <= Ncage_id`

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

## TIER-20

**unreachable code** · Conf M / Sev M  
**Found at (current code):** `atManageTier.c` 1744
**Lines at review (Oct 2026):** 1601-1604

**Function:** `EquilF_Funk()`

**Why it's thought to be a bug**

The plus-group branch `else if (a == Tier3_maxage)` can never run inside `for (a < Tier3_maxage)`, so the plus group is never accumulated. Function: `EquilF_Funk()`.

**Code at issue** — `atManageTier.c` line 1744 (the line the review pointed to, found by its text)  
Search for: `if ((a < bm->RBCestimation.RBCspeciesParam[species][Tier3_maxage_id]) && (a > 0))`

```c
 1742: 		sel = 1.0 / (1.0 + exp(-log(19) * (a - inS50) / (inS95 - inS50)));
 1743: 		bm->RBCestimation.RBCspeciesArray[species].Funkz[a] = bm->RBCestimation.RBCspeciesParam[species][Tier3_M_id] + sel * fmort;
 1744→ 		if ((a < bm->RBCestimation.RBCspeciesParam[species][Tier3_maxage_id]) && (a > 0))
 1745: 			bm->RBCestimation.RBCspeciesArray[species].num[a] = bm->RBCestimation.RBCspeciesArray[species].num[a-1] * exp(-bm->RBCestimation.RBCspeciesArray[species].Funkz[a-1]);
 1746: 		else if (a == bm->RBCestimation.RBCspeciesParam[species][Tier3_maxage_id])
```

**Suggested fix (from review):** Apply the plus-group formula at `a == Tier3_maxage-1`

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

## TIER-21

**wrong formula** · Conf H / Sev H ⭐ priority  
**Found at (current code):** `atManageTier.c` 1885
**Lines at review (Oct 2026):** 1737-1740 (and 2726-2729)

**Why it's thought to be a bug**

`step1 = (F40-F20)*(Fcurr-F20); frbc = F48 / step1;` is dimensionally wrong (1/F). The linear ramp between F40 and F20 should be `frbc = F48*(F20-Fcurr)/(F20-F40)`. The same code is copied into TierFiveSAFE. Functions: `newTier3Rule()`, `TierFiveSAFE()`.

**Code at issue** — `atManageTier.c` line 1885 (the line the review pointed to, found by its text)  
Search for: `frbc = bm->RBCestimation.RBCspeciesParam[species][F48_id] / step1;`

```c
 1883: 		step1 = (bm->RBCestimation.RBCspeciesParam[species][F40_id] - bm->RBCestimation.RBCspeciesParam[species][F20_id])
 1884: 				* (bm->RBCestimation.RBCspeciesParam[species][Fcurr_id] - bm->RBCestimation.RBCspeciesParam[species][F20_id]);
 1885→ 		frbc = bm->RBCestimation.RBCspeciesParam[species][F48_id] / step1;
 1886: 	} else {
 1887: 		// Nov 2026: SESSF Tier 3 ramp - linear from F48 at Fcurr = F40 down to zero at Fcurr = F20
```

**Suggested fix (from review):** `frbc = F48 * (F20 - Fcurr) / (F20 - F40);`

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

## TIER-22

**off-by-one** · Conf L / Sev L  
**Found at (current code):** `atManageTier.c` 1966
**Lines at review (Oct 2026):** 1812-1841

**Function:** `getF20_40_48()`

**Why it's thought to be a bug**

`fval` is updated at the *end* of the loop as `i/1000.0`, so iterations 0 and 1 both use F=0 and every later F lags by 0.001. Function: `getF20_40_48()`.

**Code at issue** — `atManageTier.c` line 1966 (the line the review pointed to, found by its text)  
Search for: `YPR(bm, species, fval, natM, &spbpr, wt, mat, agesel, llogfp);`

```c
 1964: 
 1965: 	for (i=0; i < nfint; i++){  // f = i/1000  f goes from 0 to 2
 1966→ 		YPR(bm, species, fval, natM, &spbpr, wt, mat, agesel, llogfp);
 1967: 
 1968: 		if (i == 0) spb0 = spbpr;
```

**Suggested fix (from review):** Set `fval = i/1000.0` at the top of the loop

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

## TIER-23

**cohort vs age** · Conf M / Sev M  
**Found at (current code):** `atManageTier.c` 2014
**Lines at review (Oct 2026):** 1868-1886

**Function:** `YPR()`

**Why it's thought to be a bug**

YPR loops over `numCohorts`, but the `wt/mat/sel` arrays it is given are indexed by age in years (filled up to MaxAge in TierOneMimic/trueF and up to Tier3_maxage in CC/CCsel). When ageClassSize>1, only the first numCohorts *years* are used, so the plus group is wrong. Function: `YPR()`.

**Code at issue** — `atManageTier.c` line 2014 (closest match to the quoted code; wording differs slightly)  
Search for: `void YPR(MSEBoxModel *bm, int species, double fval, double natM, double *spbpr, double *wt`

```c
 2012: //
 2013: //******************************************************************************
 2014→ void YPR(MSEBoxModel *bm, int species, double fval, double natM, double *spbpr, double *wt, double *mat, double *sel, FILE *llogfp){
 2015: 
 2016: 	int a;
```

**Suggested fix (from review):** Loop over the age length actually used (MaxAge/Tier3_maxage), passed in as a parameter

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

## TIER-24

**1-based leftover / OOB** · Conf H / Sev M  
**Found at (current code):** `atManageTier.c` 2122
**Lines at review (Oct 2026):** 1971-1972

**Function:** `TierFour()`

**Why it's thought to be a bug**

`maxprop = avprop[1]; maxfleet = 1;` reads out of bounds when Nfleets==1 (avprop has size Nfleets) and defaults to fleet 1 rather than 0. Function: `TierFour()`.

**Code at issue** — `atManageTier.c` line 2122 (the line the review pointed to, found by its text)  
Search for: `maxprop = bm->RBCestimation.RBCspeciesArray[species].avprop[1];`

```c
 2120: // which fleet has highest proportion of catch (av over last 5 years) ?
 2121: 	// 17 May 2010 - should check whether CPUE exists for these years and fleets
 2122→ 	maxprop = bm->RBCestimation.RBCspeciesArray[species].avprop[1];
 2123: 	maxfleet = 1;
 2124: 	for (nf = 0; nf < Nfleets; nf++)
```

**Suggested fix (from review):** `maxprop = avprop[0]; maxfleet = 0;`

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

## TIER-25

**off-by-one** · Conf M / Sev M  
**Found at (current code):** `atManageTier.c` 2204
**Lines at review (Oct 2026):** 2052, 2062

**Function:** `newTierFour()`

**Why it's thought to be a bug**

The CPUE target loop `iy < MaxCyr` excludes the last year, but the divisor `step1 = yrmax - yrmin + 1` counts it (and uses the unsanitised params). So CPUEtarg is biased low. Function: `newTierFour()` (same exclusive loop in `WriteCPUE()` 2177).

**Code at issue** — `atManageTier.c` line 2204  
Search for: `iy < MaxCyr`

```c
 2202: 
 2203:     nsum = 0;
 2204→ 	for (iy = MinCyr; iy < MaxCyr + 1; iy++){   // Nov 2026: now includes MaxCyr (was iy < MaxCyr but divided by the full number of years)
 2205: 		cpuesum += bm->RBCestimation.RBCspeciesArray[species].CPUEgen[fleetid][region][iy];
 2206:         nsum++;
```

**Suggested fix (from review):** `iy <= MaxCyr`; divide by `MaxCyr-MinCyr+1`

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

## TIER-26

**off-by-one** · Conf H / Sev M  
**Found at (current code):** `atManageTier.c` 2243
**Lines at review (Oct 2026):** 2078-2095

**Function:** `newTierFour()`

**Why it's thought to be a bug**

`startyear = year - m; for (iy=startyear; iy<=year; ...)` sums m+1 years but divides by m (or by `year-startyear` when clipped). CPUEav is overstated by (m+1)/m, which makes the RBC multiplier too high. Function: `newTierFour()`.

**Code at issue** — `atManageTier.c` line 2243  
Search for: `year - startyear`

```c
 2241:         /**/
 2242: 	}
 2243→     denom = (double)(year - startyear + 1);   // Number of years summed
 2244: 
 2245:     bm->RBCestimation.RBCspeciesParam[species][CPUEav_id] = cpuesum / (denom + small_num);
```

**Suggested fix (from review):** `startyear = year - m + 1` (or divide by `year-startyear+1`)

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

## TIER-27

**copy-paste / wrong index** · Conf H / Sev H ⭐ priority  
**Found at (current code):** `atManageTier.c` 2452
**Lines at review (Oct 2026):** 2291, 2295, 2388

**Why it's thought to be a bug**

`k = simplex[0][0]; xpar[1] = simplex[0][0];` and in the objective `in_K = xpar[0];`. K is parameter 1 (comment: "xpar[1] = K"), but Bstart is used everywhere instead. The K dimension of the simplex is optimised for nothing, and depletion = BcurrSP/Bstart. Functions: `TierFive_SP()`, `SurplusProduction()`.

**Code at issue** — `atManageTier.c` line 2452 (closest match to the quoted code; wording differs slightly)  
Search for: `K was set equal to Bstart, i.e.  k = simplex[0][0]  (and xpar[1] below = simplex[0][0], in`

```c
 2450: 
 2451: 	/* Nov 2026: K is now its own parameter (simplex[0][1]). Original handling, in case need to switch back:
 2452→ 	   K was set equal to Bstart, i.e.  k = simplex[0][0]  (and xpar[1] below = simplex[0][0], in_K = xpar[0] in SurplusProduction) */
 2453: 	bstart = bm->RBCestimation.RBCspeciesArray[species].simplex[0][0];
 2454: 	k = bm->RBCestimation.RBCspeciesArray[species].simplex[0][1];
```

**Suggested fix (from review):** Use `simplex[0][1]` / `xpar[1]`

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

## TIER-28

**misplaced parenthesis** · Conf H / Sev H ⭐ priority  
**Found at (current code):** `atManageTier.c` 2562
**Lines at review (Oct 2026):** 2397-2398

**Function:** `SurplusProduction()`

**Why it's thought to be a bug**

`step1 = (1 - (bt/K)^p) - T5_removal[iy-1]; B = bt + bt*r/p*step1;` multiplies the catch removal by `bt*r/p`. The projection loop at 2432-2433 correctly subtracts the removal outside. Function: `SurplusProduction()`.

**Code at issue** — `atManageTier.c` line 2562 (located by name only; check this is the right place)  
Search for: `step1 = (1.0 - exp(p * log(bt/in_K))) - T5_removal[iy-1];`

```c
 2560: 			/* Nov 2026: removals now subtracted after the production term (Pella-Tomlinson B + (r/p)B(1-(B/K)^p) - C).
 2561: 			   Original handling, in case need to switch back - removals were inside the production term:
 2562→ 			     step1 = (1.0 - exp(p * log(bt/in_K))) - T5_removal[iy-1];
 2563: 			     expectB[iy] = bt + bt * in_r / p * step1;  */
 2564: 			step1 = 1.0 - exp(bm->RBCestimation.RBCspeciesParam[species][Tier5_p_id] * log(bt/in_K));
```

**Suggested fix (from review):** `B = bt + bt*r/p*(1-(bt/K)^p) - T5_removal[iy-1]`

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

## TIER-29

**off-by-one / uninitialised read** · Conf M / Sev H  
**Found at (current code):** `atManageTier.c` 2597, `atManageTier.c` 2605
**Lines at review (Oct 2026):** 2430-2438

**Function:** `SurplusProduction()`

**Why it's thought to be a bug**

The projection loop `iy < index2` runs zero times for AssessDelay 0 and only to CurrentYear for delay 1. Then `BcurrSP = expectB[index1]` reads an entry that was never computed (outside the Init range at 2380, so stale or 0). The fitting loop also stops at `iy < CurrentYear`. Function: `SurplusProduction()`.

**Code at issue** — `atManageTier.c` line 2597  
Search for: `iy < index2`

```c
 2595: 
 2596: 	// project biomass to start of year+2 (because know catch in year+1 (year of assessment)
 2597→ 	for (iy=bm->RBCestimation.RBCspeciesArray[species].CurrentYear; iy < index2; iy++){
 2598: 		bt = bm->RBCestimation.RBCspeciesArray[species].expectB[iy-1];
 2599: 		bm->RBCestimation.RBCspeciesArray[species].expectB[iy] = bt + bt * in_r / bm->RBCestimation.RBCspeciesParam[species][Tier5_p_id] * (1.0 - exp(bm->RBCestimation.RBCspeciesParam[species][Tier5_p_id] * log(bt / in_K)))
```

**Code at issue** — `atManageTier.c` line 2605 (closest match to the quoted code; wording differs slightly)  
Search for: `bm->RBCestimation.RBCspeciesParam[species][BcurrSP_id] = bm->RBCestimation.RBCspeciesArray`

```c
 2603: 
 2604: 	// BcurrSP is biomass at start of CurrentYear+2  (CurrentYear is last year of data, but do have catch for CurrentYear+1)
 2605→ 	bm->RBCestimation.RBCspeciesParam[species][BcurrSP_id] = bm->RBCestimation.RBCspeciesArray[species].expectB[index1];    // save in global for use when solution found
 2606: 
 2607: 	return ll;      // minimise negative ll
```

**Suggested fix (from review):** Project through `iy <= index1`; init expectB to the full size

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

## TIER-30

**off-by-one / heap overflow** · Conf H / Sev M  
**Found at (current code):** `atManageTier.c` 2642, `atManageTier.c` 2646
**Lines at review (Oct 2026):** 2475-2480

**Function:** `get_T5_F20_40_48()`

**Why it's thought to be a bug**

`do { iy++; ... TSbiomass[iy] = ... } while (... && iy < 1000.0);` can write `TSbiomass[1000]`, but the array is allocated with 1000 entries (atAssessTierSetup.c:504). Function: `get_T5_F20_40_48()`.

**Code at issue** — `atManageTier.c` line 2642  
Search for: `do {`

```c
 2640: 		iy = 0;
 2641: 		// project the population with this F until it stabilises
 2642→ 		do {
 2643: 			iy++;
 2644: 			bt = bm->RBCestimation.RBCspeciesArray[species].TSbiomass[iy-1];
```

**Code at issue** — `atManageTier.c` line 2646  
Search for: `TSbiomass[iy] =`

```c
 2644: 			bt = bm->RBCestimation.RBCspeciesArray[species].TSbiomass[iy-1];
 2645: 			step1 = (1.0 - exp(bm->RBCestimation.RBCspeciesParam[species][Tier5_p_id] * log(bt/k)));
 2646→ 			bm->RBCestimation.RBCspeciesArray[species].TSbiomass[iy] = bt + bt * r / bm->RBCestimation.RBCspeciesParam[species][Tier5_p_id] * step1 - bt * (1.0 - exp(-1.0 * fval));
 2647: 		}  while ((fabs(bm->RBCestimation.RBCspeciesArray[species].TSbiomass[iy] - bm->RBCestimation.RBCspeciesArray[species].TSbiomass[iy-1]) > 1.0) && (iy < 1000.0));
 2648: 
```

**Suggested fix (from review):** `while (... && iy < 999)`

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

## TIER-31

**off-by-one / OOB read** · Conf H / Sev L  
**Found at (current code):** `atManageTier.c` 2752
**Lines at review (Oct 2026):** 2583-2589

**Function:** `TierFive_AvLen()`

**Why it's thought to be a bug**

`do { i=i+1; } while ((avlen < T5_Avlen_atF[i]) && (i < nfint));` evaluates `T5_Avlen_atF[nfint]` (array size nFtier3 == nfint) before the bound check, and never tests index 1. Function: `TierFive_AvLen()`.

**Code at issue** — `atManageTier.c` line 2752 (the line the review pointed to, found by its text)  
Search for: `while ((avlen < bm->RBCestimation.RBCspeciesArray[species].T5_Avlen_atF[i])`

```c
 2750:     i = 1;
 2751: 	do { i = i+1; }
 2752→ 	while ((avlen < bm->RBCestimation.RBCspeciesArray[species].T5_Avlen_atF[i])
 2753: 			&& (i < nfint));
 2754: 
```

**Suggested fix (from review):** Put `i < nfint` first and start at i=0

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

## TIER-32

**accumulator reset in wrong loop** · Conf H / Sev H ⭐ priority  
**Found at (current code):** `atManageTier.c` 2806
**Lines at review (Oct 2026):** 2639-2649

**Function:** `TierFiveDCAC()`

**Why it's thought to be a bug**

`for (iy...) { sumcatch = 0.0; ... }` resets the sum every year, so DCAC uses only the final year's catch and not the cumulative catch over `num_years`. Function: `TierFiveDCAC()`.

**Code at issue** — `atManageTier.c` line 2806  
Search for: `for (iy`

```c
 2804:     
 2805:     sumcatch = 0.0;
 2806→     for (iy = 0; iy < year+1; iy++) {
 2807: 		sumcatch = 0.0;
 2808: 		for (nreg = 0; nreg < Nregions ; nreg++){
```

**Suggested fix (from review):** Remove the reset inside the loop

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

## TIER-33

**accumulator not reset** · Conf H / Sev M  
**Found at (current code):** `atManageTier.c` 2939
**Lines at review (Oct 2026):** 2768-2774

**Function:** `TierSixRUSS()`

**Why it's thought to be a bug**

`localcatch` is not reset per year, so `HistCatch` ends up as cumulative total catch rather than the peak annual catch (compare TierSevenRUSS 3051, which resets it). Function: `TierSixRUSS()`.

**Code at issue** — `atManageTier.c` line 2939 (the line the review pointed to, found by its text)  
Search for: `if (localcatch > HistCatch)`

```c
 2937:             localcatch += bm->RBCestimation.RBCspeciesArray[species].CatchData[nf][Nregions][iy];
 2938:         }
 2939→         if (localcatch > HistCatch)
 2940:             HistCatch = localcatch;
 2941:     }
```

**Suggested fix (from review):** Set `localcatch = 0.0` at the top of the iy loop

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

## TIER-34

**wrong variable reset** · Conf M / Sev M  
**Found at (current code):** `atManageTier.c` 2949, `atManageTier.c` 2952, `atManageTier.c` 2957
**Lines at review (Oct 2026):** 2782-2794

**Function:** `TierSixRUSS()`

**Why it's thought to be a bug**

`RBCspeciesParam[species][TriggerReached_id] = 0;` is reset per region, but the array `TriggerReached[nreg]` (and `[Nregions]`) that the following loops set is never reset. Once a trigger has been reached it stays reached forever. Function: `TierSixRUSS()`.

**Code at issue** — `atManageTier.c` line 2949  
Search for: `RBCspeciesParam[species][TriggerReached_id] = 0`

```c
 2947:             regsumcatch += bm->RBCestimation.RBCspeciesArray[species].CatchData[nf][nreg][year];
 2948:         }
 2949→         bm->RBCestimation.RBCspeciesParam[species][TriggerReached_id] = 0;
 2950:         for (nt=0; nt < Ntriggers; nt++) {
 2951:             if (regsumcatch > (bm->RBCestimation.RBCspeciesArray[species].TriggerPoints[nt][nreg] * HistCatch))
```

**Code at issue** — `atManageTier.c` line 2952  
Search for: `TriggerReached[nreg]`

```c
 2950:         for (nt=0; nt < Ntriggers; nt++) {
 2951:             if (regsumcatch > (bm->RBCestimation.RBCspeciesArray[species].TriggerPoints[nt][nreg] * HistCatch))
 2952→                  bm->RBCestimation.RBCspeciesArray[species].TriggerReached[nreg] = nt;
 2953:         }
 2954:     }
```

**Code at issue** — `atManageTier.c` line 2957  
Search for: `[Nregions]`

```c
 2955:     totsumcatch = 0.0;
 2956:     for (nf = 0; nf< Nfleets; nf++){
 2957→         totsumcatch += bm->RBCestimation.RBCspeciesArray[species].CatchData[nf][Nregions][year];
 2958:     }
 2959:     for (nt=0; nt < Ntriggers; nt++) {
```

**Suggested fix (from review):** Reset `TriggerReached[nreg] = 0` (and `[Nregions]`) before the trigger loops

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

## TIER-35

**swapped arguments** · Conf H / Sev H ⭐ priority  
**Found at (current code):** `atManageTier.c` 2993, `atManageTier.c` 3002, `atManageTier.c` 3018
**Lines at review (Oct 2026):** 2826-2827, 2835, 2848, 2851, 2863-2867

**Function:** `TierSixRUSS()`

**Why it's thought to be a bug**

`DoCPUE(bm, year, species, llogfp)`, `DoPrager(bm, year, species, ...)`, `Call_Tier3(bm, year, species, ...)` and `TierFiveSAFE(bm, year, species, totsumcatch, ...)` all pass (year, species) where the prototypes (lines 82-83, 2048, 59) expect (species, year). The wrong species is assessed, or there are out-of-bounds species indices. Function: `TierSixRUSS()`.

**Code at issue** — `atManageTier.c` line 2993  
Search for: `DoCPUE(bm, year, species, llogfp)`

```c
 2991:                 
 2992:                 if(bm->RBCestimation.tier6_to_tier4) {
 2993→                     adjTAC = DoCPUE(bm, year, species, llogfp); // To get average CPUE
 2994:                     adjTAC = DoPrager(bm, year, species, llogfp);
 2995: 
```

**Code at issue** — `atManageTier.c` line 3002  
Search for: `Call_Tier3(bm, year, species,`

```c
 3000:                 } else {
 3001:                     // Do length based assessment
 3002→                     Call_Tier3(bm, year, species, llogfp);
 3003:                     
 3004:                     // map back to local variables so don't need to make code below conditional on method used
```

**Code at issue** — `atManageTier.c` line 3018  
Search for: `TierFiveSAFE(bm, year, species, totsumcatch,`

```c
 3016:                 else {
 3017:                     // Do SAFE assessment
 3018→                     TierFiveSAFE(bm, year, species, totsumcatch, llogfp);
 3019:                     
 3020:                     // map back to local variables so don't need to make code below conditional on method used
```

**Suggested fix (from review):** Swap to `(bm, species, year, ...)`

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

## TIER-36

**wrong operator / logic** · Conf H / Sev H ⭐ priority  
**Found at (current code):** `atManageTier.c` 3106
**Lines at review (Oct 2026):** 2938-2950

**Function:** `DoCPUE()`

**Why it's thought to be a bug**

`for (iy = year-1; iy >= 0; iy++)` counts *up* without bound (runs off the end of EffortData). Also `j = iy` is assigned only when effort was *not* found, so j is not the most recent year with effort (if found immediately, j stays 0). Function: `DoCPUE()`.

**Code at issue** — `atManageTier.c` line 3106 (closest match to the quoted code; wording differs slightly)  
Search for: `for(iy = year-1; iy >= 0; iy--) { // establish most recent non-zero effort year`

```c
 3104: 	// Calculate CPUE for the last x years, beginning with latest non-zero effort year
 3105: 	// Nov 2026: loop now counts back (was iy++ so ran forward off the end of the arrays) and j is the year found
 3106→ 	for(iy = year-1; iy >= 0; iy--) { // establish most recent non-zero effort year
 3107:         for (f = 0; f < Nfleets; f++) {
 3108:             if (bm->RBCestimation.RBCspeciesArray[species].EffortData[f][Nregions][iy] > 0.0) {
```

**Suggested fix (from review):** `iy--`; set `j = iy` when `is_done`

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

## TIER-37

**off-by-one / heap overflow** · Conf H / Sev H ⭐ priority  
**Found at (current code):** `atManageTier.c` 3130
**Lines at review (Oct 2026):** 2963

**Function:** `DoCPUE()`

**Why it's thought to be a bug**

`AnnCPUE[5-iy]` with iy going down to 0 writes `AnnCPUE[5]`, but AnnCPUE is allocated with 5 entries (atAssessTierSetup.c:215). Function: `DoCPUE()`.

**Code at issue** — `atManageTier.c` line 3130  
Search for: `AnnCPUE[5-iy]`

```c
 3128:             totC += bm->RBCestimation.RBCspeciesArray[species].CatchData[f][Nregions][j-iy];
 3129:         }
 3130→         bm->RBCestimation.RBCspeciesArray[species].AnnCPUE[5-iy] = totC / totEff;
 3131: 	}
 3132:     
```

**Suggested fix (from review):** `AnnCPUE[4-iy]`

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

## TIER-38

**wrong index** · Conf H / Sev H ⭐ priority  
**Found at (current code):** `atManageTier.c` 3211
**Lines at review (Oct 2026):** 3044, 3078, 3097, 3183, 3196, 3211

**Function:** `TierSevenRUSS()`

**Why it's thought to be a bug**

`cosp = FunctGroupArray[sp].co_sp[sp];` uses the loop counter as the group index. It should be the focal species' companion list, `FunctGroupArray[species].co_sp[sp]` (co_sp is per guild: atManageParamIO.c:259/932). Function: `TierSevenRUSS()`.

**Code at issue** — `atManageTier.c` line 3211  
Search for: `cosp = FunctGroupArray[sp].co_sp[sp]`

```c
 3209:     for (sp = 0; sp < bm->K_max_co_sp; sp++) {
 3210:         
 3211→         cosp = FunctGroupArray[sp].co_sp[sp];
 3212:         if ((cosp < 0) || (cosp > bm->K_max_impacted_sp))
 3213:             continue;
```

**Suggested fix (from review):** `FunctGroupArray[species].co_sp[sp]`

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

## TIER-39

**stale index / OOB** · Conf H / Sev H ⭐ priority  
**Found at (current code):** `atManageTier.c` 3313
**Lines at review (Oct 2026):** 3146-3147

**Function:** `TierSevenRUSS()`

**Why it's thought to be a bug**

`if (bm->SP_FISHERYprms[cosp][nf][q_id] > 0.0) toteffort += bm->CumEffort[nf][b];` sits after the nf loops have finished, so nf == K_num_fisheries and both arrays are read out of bounds. Function: `TierSevenRUSS()`.

**Code at issue** — `atManageTier.c` line 3313  
Search for: `if (bm->SP_FISHERYprms[cosp][nf][q_id] > 0.0)`

```c
 3311:             }
 3312: 
 3313→             if (bm->SP_FISHERYprms[cosp][nf][q_id] > 0.0)
 3314:                 toteffort += bm->CumEffort[nf][b];
 3315: 
```

**Suggested fix (from review):** Wrap in `for (nf = 0; nf < K_num_fisheries; nf++)`

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

## TIER-40

**accumulator not reset** · Conf M / Sev M  
**Found at (current code):** `atManageTier.c` 3323
**Lines at review (Oct 2026):** 3124-3156

**Function:** `TierSevenRUSS()`

**Why it's thought to be a bug**

`areafished` and `areasp` (and `toteffort`) are initialised once at function entry and not per co-species, so `relarea` for later species includes earlier species' areas. Function: `TierSevenRUSS()`.

**Code at issue** — `atManageTier.c` line 3323 (located by name only; check this is the right place)  
Search for: `relarea = areafished / (areasp + small_num);`

```c
 3321:             if(biomhere)
 3322:                 areasp += bm->boxes[b].area;
 3323→             relarea = areafished / (areasp + small_num);
 3324:         }
 3325:         bm->RBCestimation.RBCspeciesArray[species].relareafished[cosp][year] = relarea;
```

**Suggested fix (from review):** Reset areafished/areasp at the top of each cosp iteration

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

## TIER-41

**swapped args / wrong species** · Conf H / Sev H ⭐ priority  
**Found at (current code):** `atManageTier.c` 3383
**Lines at review (Oct 2026):** 3216

**Function:** `TierSevenRUSS()`

**Why it's thought to be a bug**

`Call_Tier3(bm, year, species, llogfp)` has its arguments swapped, and inside the loop over cosp it should assess `cosp`. Function: `TierSevenRUSS()`.

**Code at issue** — `atManageTier.c` line 3383  
Search for: `Call_Tier3(bm, year, species, llogfp)`

```c
 3381:         
 3382:         if (bm->RBCestimation.RBCspeciesArray[cosp].TriggerReached[Nregions] == 2)
 3383→             Call_Tier3(bm, year, species, llogfp);
 3384:         
 3385:         bm->RBCestimation.RBCspeciesParam[cosp][TriggerReached_id] = bm->RBCestimation.RBCspeciesArray[cosp].TriggerReached[Nregions];
```

**Suggested fix (from review):** `Call_Tier3(bm, cosp, year, llogfp)`

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

## TIER-42

**accumulator not reset / off-by-one** · Conf M / Sev M  
**Found at (current code):** `atManageTier.c` 4000
**Lines at review (Oct 2026):** 3718-3736

**Function:** `trigger_species_breakout()`

**Why it's thought to be a bug**

`catchwghtCPUE` is zeroed once outside the `lag` loop, so every lag adds into the same slots. The regression for lag k therefore includes data from lags > k. Also, when mgt_rbcYear==this_year, `yy` runs 1..TCP-1, so slot 0 is never filled and a spurious 0 enters the regression. Division by `totCatch` can also be 0. Function: `trigger_species_breakout()`.

**Code at issue** — `atManageTier.c` line 4000 (the line the review pointed to, found by its text)  
Search for: `for (y = (bm->RBCestimation.RBCspeciesArray[sp].mgt_rbcYear - bm->RBCestimation.TriggerChe`

```c
 3998:     Util_Init_1D_Double(bm->RBCestimation.catchwghtCPUE, bm->RBCestimation.TriggerCheckPeriod, 0.0);
 3999:     for (lag = bm->RBCestimation.LagPeriod; lag >= 0; lag--){
 4000→         for (y = (bm->RBCestimation.RBCspeciesArray[sp].mgt_rbcYear - bm->RBCestimation.TriggerCheckPeriod - lag); y < (this_year - 1 - lag); y++){
 4001:             totCatch = 0;
 4002:             for (r = 0; r < bm->RBCestimation.RBCspeciesParam[sp][NumRegions_id]; r++) {
```

**Suggested fix (from review):** Init inside the lag loop; fix the yy offset (drop the `+1`)

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

## TIER-43

**wrong variable** · Conf M / Sev M  
**Found at (current code):** `atManageTier.c` 4067
**Lines at review (Oct 2026):** 3787

**Function:** `CalculateNonIndicatorRBC()`

**Why it's thought to be a bug**

`(FunctGroupArray[sp].isImpacted == FALSE)` tests the target species and not the candidate indicator `i`. Function: `CalculateNonIndicatorRBC()`.

**Code at issue** — `atManageTier.c` line 4067  
Search for: `(FunctGroupArray[sp].isImpacted == FALSE)`

```c
 4065:         bm->RBCestimation.RBCspeciesParam[sp][TACpost_id] = bm->RBCestimation.RBCspeciesParam[sp][TACest_id]; // Last year's TAC
 4066:         for (i = 0; i < bm->K_num_tot_sp; i++) {
 4067→             if (sp == i || (FunctGroupArray[sp].isImpacted == FALSE) || bm->RBCestimation.RBCspeciesParam[sp][mgt_category_id] != bm->RBCestimation.RBCspeciesParam[i][mgt_category_id] || !bm->RBCestimation.RBCspeciesParam[i][mgt_indicator_id]) continue;
 4068:             nInds++;
 4069:             
```

**Suggested fix (from review):** `FunctGroupArray[i].isImpacted`

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

## TIER-44

**wrong constant / accumulator** · Conf M / Sev M  
**Found at (current code):** `atManageTier.c` 4157
**Lines at review (Oct 2026):** 3877-3910, 3899

**Function:** `Check_Indicator_Links()`

**Why it's thought to be a bug**

The `yy` offset uses `GradientPeriod` (the lag count) and not `GradientWindow` (the array length). Indices can exceed the catchwght/catchwghtCPUE size (max(GradientWindow, TriggerCheckPeriod)), or be clipped so that many years pile into slot 0. The arrays are also not reset between lags (same issue as TIER-42). Function: `Check_Indicator_Links()`.

**Code at issue** — `atManageTier.c` line 4157 (the line the review pointed to, found by its text)  
Search for: `Util_Init_1D_Double(bm->RBCestimation.catchwghtCPUE, bm->RBCestimation.GradientWindow, 0.0`

```c
 4155:             printf("Doing Check_Indicator_Links Doing species %s ", FunctGroupArray[sp].groupCode);
 4156:             
 4157→             Util_Init_1D_Double(bm->RBCestimation.catchwghtCPUE, bm->RBCestimation.GradientWindow, 0.0);
 4158:             Util_Init_1D_Double(bm->RBCestimation.catchwght, bm->RBCestimation.GradientWindow, 0.0);
 4159:             for (lag = bm->RBCestimation.GradientPeriod; lag >= 0; lag--){
```

**Suggested fix (from review):** Use GradientWindow in the offset; reset per lag

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

## TIER-45

**copy-paste** · Conf H / Sev M  
**Found at (current code):** `atManageTier.c` 4206
**Lines at review (Oct 2026):** 3927-3929

**Function:** `Check_Indicator_Links()`

**Why it's thought to be a bug**

The else-branch sets `CPUEtrend_id = 0` where it should set `CPUEdir_id = 0`, so the direction stays stale and the trend is wiped before it is used in steps B/C. Function: `Check_Indicator_Links()`.

**Code at issue** — `atManageTier.c` line 4206 (located by name only; check this is the right place)  
Search for: `bm->RBCestimation.RBCspeciesParam[sp][CPUEdir_id] = -1;`

```c
 4204:                     bm->RBCestimation.RBCspeciesParam[sp][CPUEdir_id] = 1;
 4205:                 } else if (bm->RBCestimation.RBCspeciesParam[sp][CPUEtrend_id] < (0 - buffer)) {
 4206→                     bm->RBCestimation.RBCspeciesParam[sp][CPUEdir_id] = -1;
 4207:                 } else {
 4208:                     bm->RBCestimation.RBCspeciesParam[sp][CPUEtrend_id] = 0;
```

**Suggested fix (from review):** `RBCspeciesParam[sp][CPUEdir_id] = 0;`

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

## TIER-46

**missing update** · Conf H / Sev H ⭐ priority  
**Found at (current code):** `atManageTier.c` 4261
**Lines at review (Oct 2026):** 3950-3952, 3968-3970, 4022-4024, 4042-4044

**Function:** `Check_Indicator_Links()`

**Why it's thought to be a bug**

`if (diff < closest_slope) ref_sp = ind_sp;` never updates `closest_slope` (or `closest_slopeB`), so ref_sp ends up as the *last* candidate (possibly `sp` itself), not the closest. And `closest_slope < closest_slopeB` (4260/4334) compares MAXDOUBLE with MAXDOUBLE, so ref_spB is always chosen. Function: `Check_Indicator_Links()`.

**Code at issue** — `atManageTier.c` line 4261  
Search for: `closest_slope < closest_slopeB`

```c
 4259:             if (bm->RBCestimation.UseClosest) {
 4260:                 // Use closest regardless of management category
 4261→                 if ( closest_slope < closest_slopeB ){
 4262:                     bm->RBCestimation.RBCspeciesParam[sp][mgt_sp_id] = ref_sp; // Species from the same category
 4263:                 } else {
```

**Suggested fix (from review):** Add `closest_slope = diff;` / `closest_slopeB = diff;`; skip `ind_sp == sp`

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

## TIER-47

**inconsistency** · Conf H / Sev M  
**Found at (current code):** `atManageTier.c` 4229
**Lines at review (Oct 2026):** 3967

**Function:** `Check_Indicator_Links()`

**Why it's thought to be a bug**

The other-category diff is missing `fabs()` (the same-category loop at 3949 has it), so any species with a more negative trend counts as "closest". Function: `Check_Indicator_Links()`.

**Code at issue** — `atManageTier.c` line 4229 (located by name only; check this is the right place)  
Search for: `diff = fabs(bm->RBCestimation.RBCspeciesParam[sp][CPUEtrend_id] - bm->RBCestimation.RBCspe`

```c
 4227:                 if( FunctGroupArray[ind_sp].isImpacted == TRUE) {
 4228:                     if (bm->RBCestimation.RBCspeciesParam[sp][mgt_category_id] == bm->RBCestimation.RBCspeciesParam[ind_sp][mgt_category_id]) {
 4229→                         diff = fabs(bm->RBCestimation.RBCspeciesParam[sp][CPUEtrend_id] - bm->RBCestimation.RBCspeciesParam[ind_sp][CPUEtrend_id]);
 4230:                         if (diff < closest_slope) {
 4231:                             ref_sp = ind_sp;
```

**Suggested fix (from review):** `diff = fabs(...)`

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

## TIER-48

**wrong variable** · Conf H / Sev M  
**Found at (current code):** `atManageTier.c` 4267
**Lines at review (Oct 2026):** 3987, 4061

**Function:** `Check_Indicator_Links()`

**Why it's thought to be a bug**

`maxTier = mgt_category_id;` defaults to the enum index constant and not the species' current category (`my_category`). When no species falls inside the buffer, the category is set to an arbitrary number. Function: `Check_Indicator_Links()`.

**Code at issue** — `atManageTier.c` line 4267  
Search for: `maxTier = mgt_category_id`

```c
 4265:                 }
 4266:                 
 4267→                 maxTier = mgt_category_id;
 4268:                 maxCount = 0;
 4269:                 for (itier = 0; itier < K_num_indicator_tiers; itier++ ){
```

**Suggested fix (from review):** `maxTier = my_category;`

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

## TIER-49

**state not reset / use-after-close** · Conf H / Sev H ⭐ priority  
**Found at (current code):** `atManageTier.c` 5236, `atManageTier.c` 5248
**Lines at review (Oct 2026):** 4156, 4164-4172

**Function:** `RDynamicTier4Assessment()`

**Why it's thought to be a bug**

`chdir(dirname)` never returns to the parent directory, so later relative file I/O and next year's mkdir end up nested. `Util_Close_Output_File(bm->Dyn4File)` closes the file but leaves the pointer non-NULL (atUtil.c:200), so on the next call `if(!bm->Dyn4File)` is false and the code writes to a closed FILE*. Function: `RDynamicTier4Assessment()`.

**Code at issue** — `atManageTier.c` line 5236  
Search for: `chdir(dirname)`

```c
 5234:     /* Change directory into the new directory */
 5235:     //pass your path in the function
 5236→     ret = chdir(dirname);
 5237:     /*if the change of directory was successful it will print successful otherwise it will print not successful*/
 5238:     if (ret < 0)
```

**Code at issue** — `atManageTier.c` line 5248  
Search for: `Util_Close_Output_File(bm->Dyn4File)`

```c
 5246:     bm->Dyn4CTLFile = initDynTier4CTLFile(bm);
 5247:     writeDynTier4inputfile(bm, species, year, bm->Dyn4File, bm->Dyn4CTLFile);
 5248→     Util_Close_Output_File(bm->Dyn4File);
 5249:     Util_Close_Output_File(bm->Dyn4CTLFile);
 5250:     bm->Dyn4File = NULL;
```

**Suggested fix (from review):** Set both pointers to NULL after closing; `chdir("..")` at the end

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

## TIER-50

**copy-paste** · Conf H / Sev H ⭐ priority  
**Found at (current code):** `atManageTier.c` 5378
**Lines at review (Oct 2026):** 4280-4282

**Function:** `writeDynTier4inputfile()`

**Why it's thought to be a bug**

The third block under the "Year, Catch and CPUE" header writes `CatchData` a second time where CPUE (`CPUEgen`) is expected, so the R assessment receives catch as CPUE. Function: `writeDynTier4inputfile()`.

**Code at issue** — `atManageTier.c` line 5378 (located by name only; check this is the right place)  
Search for: `cpue = bm->RBCestimation.RBCspeciesArray[sp].CPUEgen[nf][sumregion][iy];`

```c
 5376:         // CPUE by fleet (all-region value), -1 = missing or fleet not used
 5377:         for (nf = 0; nf < Nfleets; nf++) {
 5378→             cpue = bm->RBCestimation.RBCspeciesArray[sp].CPUEgen[nf][sumregion][iy];
 5379:             if ((cpue <= 0.0) || ((CPUEfleet >= 0) && (CPUEfleet != nf)))
 5380:                 cpue = -1.0;
```

**Suggested fix (from review):** Write `CPUEgen[nf][sumregion][iy]`

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

## TIER-51

**uninitialised counter** · Conf L / Sev M  
**Found at (current code):** `atManageTier.c` 3242
**Lines at review (Oct 2026):** 4269

**Function:** `writeDynTier4inputfile()`

**Why it's thought to be a bug**

`t` is never incremented, so `t <= HistYrMax` is always true and the retro-catch adjustment applies to all years. Function: `writeDynTier4inputfile()`.

**Code at issue** — `atManageTier.c` line 3242 (located by name only; check this is the right place)  
Search for: `for (iy = 0; iy < bm->RBCestimation.RBCspeciesParam[species][HistYrMax_id]; iy++){`

```c
 3240:     /* Aggregate catch over the group and then apply similar triggers to tier 6 */
 3241:     CoHistCatch = 0.0;
 3242→     for (iy = 0; iy < bm->RBCestimation.RBCspeciesParam[species][HistYrMax_id]; iy++){
 3243:         hist_co_catch = 0.0;
 3244:         for (sp = 0; sp < bm->K_max_co_sp; sp++) {
```

**Suggested fix (from review):** Use `iy` (or increment t)

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

## TIER-52

**statement inside comment** · Conf H / Sev L  
**Found at (current code):** `atManageTier.c` 5577
**Lines at review (Oct 2026):** 4456

**Function:** `setRBC()`

**Why it's thought to be a bug**

`...= 0.0; // temporary value ...        bm->RBCestimation.RBCspeciesParam[species][EstB0_id] = 999.9;` puts the EstB0 assignment inside the comment, so EstB0 is not flagged on failure and EstDepletion (5587) mixes a valid B0 with Bcurr=999.9. Function: `setRBC()`.

**Code at issue** — `atManageTier.c` line 5577  
Search for: `= 0.0;               // temporary value`

```c
 5575: 
 5576:         bm->RBCestimation.RBCspeciesParam[species][AssessFail_id] = TRUE;
 5577→         bm->RBCestimation.RBCspeciesArray[species].RBC_by_year[year] = 0.0;               // temporary value, will be reset in DoAssessment        bm->RBCestimation.RBCspeciesParam[species][EstB0_id] = 999.9;
 5578:         bm->RBCestimation.RBCspeciesParam[species][EstBinit_id] = 999.9;
 5579:         bm->RBCestimation.RBCspeciesParam[species][EstBcurr_id] = 999.9;
```

**Suggested fix (from review):** Move the EstB0 assignment onto its own line

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

## TIER-53

**off-by-one / wrong index** · Conf H / Sev M  
**Found at (current code):** `atManageTier.c` 5606
**Lines at review (Oct 2026):** 4481-4490

**Function:** `Averaged_RBC()`

**Why it's thought to be a bug**

The loop `yr < mgt_rbcYear + n - 1` sums n-1 RBCs but divides by n. The replacement loop writes `RBC_by_year[year]` repeatedly and not `[yr]`. Function: `Averaged_RBC()`.

**Code at issue** — `atManageTier.c` line 5606  
Search for: `RBC_by_year[year]`

```c
 5604: 
 5605:     // Store the current RBC so it is included in the average
 5606→     bm->RBCestimation.RBCspeciesArray[species].RBC_by_year[year] = bm->RBCestimation.RBCspeciesParam[species][RBCest_id];
 5607: 
 5608:     startyr = year - bm->RBCestimation.nRBCaverage + 1;
```

**Suggested fix (from review):** Loop `yr < mgt_rbcYear + n`; write `RBC_by_year[yr]`

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

## TIER-54

**wrong array** · Conf M / Sev L  
**Found at (current code):** `atManageTier.c` 3410
**Lines at review (Oct 2026):** 3243

**Function:** `Evaluation()`

**Why it's thought to be a bug**

`tiertype = FunctGroupArray[species].speciesParams[tiertype_id]`: tiertype_id belongs to the RBC_SPECIES_PARAMS enum (atlantisboxmodel.h:2620/2706). DoTierAssessment reads `RBCspeciesParam[species][tiertype_id]`, so the tier5 log branch tests an unrelated parameter. Function: `Evaluation()`.

**Code at issue** — `atManageTier.c` line 3410  
Search for: `RBCspeciesParam[species][tiertype_id]`

```c
 3408: 	double sb0;
 3409: 	int tier = (int) (FunctGroupArray[species].speciesParams[tier_id]);
 3410→ 	int tiertype = (int) (bm->RBCestimation.RBCspeciesParam[species][tiertype_id]);   // Nov 2026: was read from speciesParams
 3411: 	double step1;
 3412: 	int fleetid = (int)(bm->RBCestimation.RBCspeciesParam[species][CPUEfleet_id]);
```

**Suggested fix (from review):** `bm->RBCestimation.RBCspeciesParam[species][tiertype_id]`

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

## TIER-55

**wrong index (log only)** · Conf M / Sev L  
**Found at (current code):** `atManageTier.c` 3425, `atManageTier.c` 3435, `atManageTier.c` 3459
**Lines at review (Oct 2026):** 3256-3258, 3292

**Function:** `Evaluation()`

**Why it's thought to be a bug**

`sb0 = SBioZero[0][0]` is used with `SpawnBio[Nstocks][Nregions]` (elsewhere the overall value is at [Nstocks][Nregions]). `step1 = CPUEgen[fleetid][r][year]` is computed with the stale `r` (=Nregions) outside the loop and printed for every region. Function: `Evaluation()`.

**Code at issue** — `atManageTier.c` line 3425 (closest match to the quoted code; wording differs slightly)  
Search for: `sb0 = bm->RBCestimation.RBCspeciesArray[species].SBioZero[0][0];`

```c
 3423: 		sb0 = bm->RBCestimation.RBCspeciesArray[species].SBioZero_shift[0][0];
 3424: 	else
 3425→ 		sb0 = bm->RBCestimation.RBCspeciesArray[species].SBioZero[0][0];
 3426: 
 3427: 	switch (tier){
```

**Code at issue** — `atManageTier.c` line 3435  
Search for: `SpawnBio[Nstocks][Nregions]`

```c
 3433: 			bm->RBCestimation.RBCspeciesParam[species][MaxConvergCrit_id], sb0,
 3434: 			bm->RBCestimation.RBCspeciesParam[species][EstB0_id],
 3435→ 			bm->RBCestimation.RBCspeciesArray[species].SpawnBio[Nstocks][Nregions][year],
 3436: 			bm->RBCestimation.RBCspeciesParam[species][EstBcurr_id],
 3437: 			bm->RBCestimation.RBCspeciesArray[species].SpawnBio[Nstocks][Nregions][year] / sb0,
```

**Code at issue** — `atManageTier.c` line 3459 (closest match to the quoted code; wording differs slightly)  
Search for: `step1 = bm->RBCestimation.RBCspeciesArray[species].CPUEgen[fleetid][r][year];`

```c
 3457: 				bm->RBCestimation.RBCspeciesParam[species][Cmult_id], bm->RBCestimation.RBCspeciesParam[species][CCcur_id],
 3458: 				bm->RBCestimation.RBCspeciesParam[species][RBCest_id]);
 3459→ 		step1 = bm->RBCestimation.RBCspeciesArray[species].CPUEgen[fleetid][r][year];
 3460: 		for (r=0; r < Nregions; r++)
 3461: 			fprintf(llogfp, "%e %s %d %d %e %e %e\n",
```

**Suggested fix (from review):** Use [Nstocks][Nregions]; move step1 into the loop

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

## TIER-56

**debug leftover** · Conf H / Sev L  
**Found at (current code):** `atManageTier.c` 1867
**Lines at review (Oct 2026):** 1724-1725, 2528-2529, 2579-2580

**Why it's thought to be a bug**

Hard-coded `if (bm->dayt > 10584) printf(...)` debug prints left in live code. Functions: `newTier3Rule()`, `TierFive_AvLen()`.

**Code at issue** — `atManageTier.c` line 1867  
Search for: `if (bm->dayt > 10584)`

```c
 1865: 	//int do_debug = 0;
 1866: 
 1867→     if (bm->dayt > 10584)
 1868:         printf("Applying final step of calc\n");
 1869: 
```

**Suggested fix (from review):** Remove

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

20:35:48 rule (1096, 2476): `ftarg = F40*(dep/0.2 - 1)` between 0.2 and 0.35 gives 0.75·F40 at 0.35, then jumps to F48. It looks copied from a 20:40 rule. Check the intended breakpoint and scaling.

- [ ] Investigate   - [ ] Ignore

**Your comments:**

> 

### LC-2

`get_T5_F20_40_48()` hard-codes 0.48/0.40 rather than BrefA/BrefB (as used in getF20_40_48), and never sets F20 (it stays 0).

- [ ] Investigate   - [ ] Ignore

**Your comments:**

> 

### LC-3

`SurplusProduction()` / `TierFive_SP()` use `HistYrMin_id` directly as an array index into expectB/T5_removal/CatchData. This is only safe if HistYrMin is 0-based (not a calendar year).

- [ ] Investigate   - [ ] Ignore

**Your comments:**

> 

### LC-4

Several places index `year-4`/`year-3` without clamping at 0 (`TierThree_CC` 1092, `TierFour` 1981). The weighted discard average (848-862) excludes year 0 (`year-1 > 0` should probably be `>= 0`).

- [ ] Investigate   - [ ] Ignore

**Your comments:**

> 

### LC-5

`Check_Indicator_Links()`: `buffer = GradientBuffer * CPUEtrend` is relative to the trend itself, so with GradientBuffer<1 any non-zero trend is classified ±1. A buffer computed from a negative trend also makes `diff < buffer` impossible. `ref_sp`/`ref_spB` can stay -1 and then be used as an index in `CalculateNonIndicatorRBC()` (4085).

- [ ] Investigate   - [ ] Ignore

**Your comments:**

> 

### LC-6

`setRBC()` never sets `RBCest_id` (the RBC read is TODO, d0=0), so DoTierAssessment uses a stale RBCest for dynamic tier 4. The convergence threshold of 100.1 is inconsistent with the comment's 0.1.

- [ ] Investigate   - [ ] Ignore

**Your comments:**

> 

### LC-7

`getSlope()`, `DoCPUE()`, `regressionSlope()`, `TierThree_trueF()` (fmax) and `TierOneMimic()` (fmax at 947) have unguarded divisions that give NaN/inf when there is no data.

- [ ] Investigate   - [ ] Ignore

**Your comments:**

> 

### LC-8

`TierThree_CCsel()` and `EquilF_Funk()` read `AgeComp[...][a]` for a < Tier3_maxage, but AgeComp's age dimension is the user-set `OverallMaxAge`. Nothing checks that OverallMaxAge >= Tier3_maxage.

- [ ] Investigate   - [ ] Ignore

**Your comments:**

> 
