# atmanage/atManageTier.c — bug review

Generated 2026-11-19 from `atlantis/to-do-fixt-list.md`. **56 findings** (severity H: 24; confidence H and severity H: 23; on the priority list: 23), plus 8 lower-confidence notes.

Line numbers were re-mapped on 2026-11-19 to the current code (after the merge of main). Each was traced through git history from the version the review used, then checked against the text of the line. **Lines (current code)** and line numbers in the text are now current; ≈ means the code there has changed so the position is approximate. The review's own numbers are kept as **Lines at review**; a number ending in ? could not be traced and is the review's number. ⚠ means the quoted code is no longer at those lines: it may have changed or already been fixed.

Conf/Sev: confidence it's a bug / severity if it is (H = wrong results or crash, M = wrong in some configs, L = cosmetic, log, leak or latent). ✔ = the reviewer re-checked it by hand. ⭐ = on the high-priority list.

## Summary

| ID | Lines (current) | Conf/Sev | Category | Short description | Decision | Done |
|---|---|---|---|---|---|---|
| [TIER-01](#tier-01) ⭐ | ≈604 | H/H | wrong operator / sign | `tac_change = (TAC_old - TACpost)/TAC_old; if (tac_change > inc_check) TAC = TAC_old + inc_check*TAC_old; else… |  | [ ] |
| [TIER-02](#tier-02) ⭐ | ≈597 | H/H | wrong variable | `if (TAC < minCatch) TAC = FunctGroupArray[species].speciesParams[MinCatch_id];` sets the TAC to the fraction … |  | [ ] |
| [TIER-03](#tier-03) ⭐ | ≈637-638 | H/H | wrong index / OOB | `if (bm->TACamt[species][nf][now_id] > no_quota) bm->TACamt[species][nf][now_id] = no_quota;` is inside the `n… |  | [ ] |
| [TIER-04](#tier-04) ⭐ | 446-480 | H/H | ordering / wrong value used | `RBC = RBCparam[RBCest_id];` is read *before* `Apply_RBC_Buffers()` changes RBCest_id. The TAC is then worked … |  | [ ] |
| [TIER-05](#tier-05) | 341-345 (also 2073-2076) | H/M | accumulator / loop | `for (nf...) for (nreg < Nregions) current_catch += Harvest_Get_TotCumCatch(species, nf, bm->thisyear);` The i… |  | [ ] |
| [TIER-06](#tier-06) | 405-413 | M/M | switch fall-through | `case tier8: case tier9: case tier13: RBC = 0.0;` has no break, so it falls into `default` and quits with "No … |  | [ ] |
| [TIER-07](#tier-07) ⭐ | 3743-3751 | H/H | switch fall-through / wrong source | `case sp_rollover:` has no `break`, so it falls through to `default` and calls `quit("")`. It is also unreacha… |  | [ ] |
| [TIER-08](#tier-08) | 806-816 | M/M | off-by-one / sanity logic | `if (MinCyr > MaxCyr) MaxCyr = MinCyr;` can push MaxCyr past `year` (future, unfilled years), so it should pro… |  | [ ] |
| [TIER-09](#tier-09) | 822-847 | M/M | accumulator not reset | `Util_Init_1D_Double(discrate, nyears, 0.0)` zeroes indices 0..nyears-1 (the number of averaging years), not t… |  | [ ] |
| [TIER-10](#tier-10) | 825, 904-907 | M/M | uninitialised data / stale variable | `DiscType = ...DiscardType[nf]`: DiscardType is allocated with size nLen (atAssessTierSetup.c:130), is indexed… |  | [ ] |
| [TIER-11](#tier-11) | 746 | M/M | wrong divisor | `avprop_reg[nf][nreg] /= num_avg_year;` but the sum covers `startyear..year` (5 years, or fewer near the start… |  | [ ] |
| [TIER-12](#tier-12) | 1136-1161 | M/M | cohort vs age / double counting | `natm = Mnat[0][0][s][a][index2]` uses `a` (a cohort×genotype index, up to numCohortsXnumGenes) as the *age-in… |  | [ ] |
| [TIER-13](#tier-13) ⭐ | 1270, 1286 | H/H | off-by-one / heap overflow | `for (ia = 0; ia <= this_nselage; ia++) ... propgt05[ia] = 1;` reads `numatage[nselage]` and writes `propgt05[… |  | [ ] |
| [TIER-14](#tier-14) | 1260-1265 | M/M | 1-based leftover / dead code | `if ((a == AgeSel95+1) && (numatage[1] < numatage[2]))`: at this point `numatage[2]` has not been filled yet (… |  | [ ] |
| [TIER-15](#tier-15) ⭐ | 1312-1315 | H/H | off-by-one | `nz` is the index of the last stored Z (it starts at -1 and is incremented before use), but `for (i=0; i<nz; i… |  | [ ] |
| [TIER-16](#tier-16) | 1484, 1512 | H/M | uninitialised / not reset | `NCA` is initialised to 0 at its declaration and only updated when a year has data. It is not reset per fleet … |  | [ ] |
| [TIER-17](#tier-17) ⭐ | ≈1574 | H/H | copy-paste / wrong index | Restart branch: `fvalF[nF] = UnTransform(simplex[1][1], 0.01);` takes the S95 parameter of the second vertex. … |  | [ ] |
| [TIER-18](#tier-18) ⭐ | ≈1601, ≈1614, ≈1620, ≈1624 | H/H | off-by-one | `nF` is the index of the last fitted fleet (it starts at -1, then ++), but the loops `i < nF` and the divisor … |  | [ ] |
| [TIER-19](#tier-19) ⭐ | 1756 | H/H | off-by-one | `for (iy=0; iy < Ncage_id; iy++)`: Ncage_id is the last index (it starts at -1), so the last year of catch-at-… |  | [ ] |
| [TIER-20](#tier-20) | 1743-1746 | M/M | unreachable code | The plus-group branch `else if (a == Tier3_maxage)` can never run inside `for (a < Tier3_maxage)`, so the plus… |  | [ ] |
| [TIER-21](#tier-21) ⭐ | ≈1881-1884 (and 2892-2895) | H/H | wrong formula | `step1 = (F40-F20)*(Fcurr-F20); frbc = F48 / step1;` is dimensionally wrong (1/F). The linear ramp between F40… |  | [ ] |
| [TIER-22](#tier-22) | 1961-1990 | L/L | off-by-one | `fval` is updated at the *end* of the loop as `i/1000.0`, so iterations 0 and 1 both use F=0 and every later F… |  | [ ] |
| [TIER-23](#tier-23) | 2017-2035 | M/M | cohort vs age | YPR loops over `numCohorts`, but the `wt/mat/sel` arrays it is given are indexed by age in years (filled up to… |  | [ ] |
| [TIER-24](#tier-24) | 2121-2122 | H/M | 1-based leftover / OOB | `maxprop = avprop[1]; maxfleet = 1;` reads out of bounds when Nfleets==1 (avprop has size Nfleets) and default… |  | [ ] |
| [TIER-25](#tier-25) | 2337, ≈2203 | M/M | off-by-one | The CPUE target loop `iy < MaxCyr` excludes the last year, but the divisor `step1 = yrmax - yrmin + 1` counts … |  | [ ] |
| [TIER-26](#tier-26) | ≈2230-2244 | H/M | off-by-one | `startyear = year - m; for (iy=startyear; iy<=year; ...)` sums m+1 years but divides by m (or by `year-startye… |  | [ ] |
| [TIER-27](#tier-27) ⭐ | ≈2451, ≈2451, ≈2550 | H/H | copy-paste / wrong index | `k = simplex[0][0]; xpar[1] = simplex[0][0];` and in the objective `in_K = xpar[0];`. K is parameter 1 (commen… |  | [ ] |
| [TIER-28](#tier-28) ⭐ | ≈2561 | H/H | misplaced parenthesis | `step1 = (1 - (bt/K)^p) - T5_removal[iy-1]; B = bt + bt*r/p*step1;` multiplies the catch removal by `bt*r/p`. … |  | [ ] |
| [TIER-29](#tier-29) | 2596-2604 | M/H | off-by-one / uninitialised read | The projection loop `iy < index2` runs zero times for AssessDelay 0 and only to CurrentYear for delay 1. Then … |  | [ ] |
| [TIER-30](#tier-30) | 2641-2646 | H/M | off-by-one / heap overflow | `do { iy++; ... TSbiomass[iy] = ... } while (... && iy < 1000.0);` can write `TSbiomass[1000]`, but the array … |  | [ ] |
| [TIER-31](#tier-31) | 2749-2755 | H/L | off-by-one / OOB read | `do { i=i+1; } while ((avlen < T5_Avlen_atF[i]) && (i < nfint));` evaluates `T5_Avlen_atF[nfint]` (array size … |  | [ ] |
| [TIER-32](#tier-32) ⭐ | 2805-2815 | H/H | accumulator reset in wrong loop | `for (iy...) { sumcatch = 0.0; ... }` resets the sum every year, so DCAC uses only the final year's catch and … |  | [ ] |
| [TIER-33](#tier-33) | 2934-2940 | H/M | accumulator not reset | `localcatch` is not reset per year, so `HistCatch` ends up as cumulative total catch rather than the peak annu… |  | [ ] |
| [TIER-34](#tier-34) | 2948-2960 | M/M | wrong variable reset | `RBCspeciesParam[species][TriggerReached_id] = 0;` is reset per region, but the array `TriggerReached[nreg]` (… |  | [ ] |
| [TIER-35](#tier-35) ⭐ | 2992-2993, 3001, 3014, 3017, 3029-3033 | H/H | swapped arguments | `DoCPUE(bm, year, species, llogfp)`, `DoPrager(bm, year, species, ...)`, `Call_Tier3(bm, year, species, ...)` … |  | [ ] |
| [TIER-36](#tier-36) ⭐ | ≈3104-3116 | H/H | wrong operator / logic | `for (iy = year-1; iy >= 0; iy++)` counts *up* without bound (runs off the end of EffortData). Also `j = iy` i… |  | [ ] |
| [TIER-37](#tier-37) ⭐ | 3129 | H/H | off-by-one / heap overflow | `AnnCPUE[5-iy]` with iy going down to 0 writes `AnnCPUE[5]`, but AnnCPUE is allocated with 5 entries (atAssess… |  | [ ] |
| [TIER-38](#tier-38) ⭐ | 3210, 3244, 3263, 3349, 3362, 3377 | H/H | wrong index | `cosp = FunctGroupArray[sp].co_sp[sp];` uses the loop counter as the group index. It should be the focal speci… |  | [ ] |
| [TIER-39](#tier-39) ⭐ | 3312-3313 | H/H | stale index / OOB | `if (bm->SP_FISHERYprms[cosp][nf][q_id] > 0.0) toteffort += bm->CumEffort[nf][b];` sits after the nf loops hav… |  | [ ] |
| [TIER-40](#tier-40) | 3290-3322 | M/M | accumulator not reset | `areafished` and `areasp` (and `toteffort`) are initialised once at function entry and not per co-species, so … |  | [ ] |
| [TIER-41](#tier-41) ⭐ | 3382 | H/H | swapped args / wrong species | `Call_Tier3(bm, year, species, llogfp)` has its arguments swapped, and inside the loop over cosp it should ass… |  | [ ] |
| [TIER-42](#tier-42) | 3997-4015 | M/M | accumulator not reset / off-by-one | `catchwghtCPUE` is zeroed once outside the `lag` loop, so every lag adds into the same slots. The regression f… |  | [ ] |
| [TIER-43](#tier-43) | 4066 | M/M | wrong variable | `(FunctGroupArray[sp].isImpacted == FALSE)` tests the target species and not the candidate indicator `i`. Func… |  | [ ] |
| [TIER-44](#tier-44) | 4156-4189, 4178 | M/M | wrong constant / accumulator | The `yy` offset uses `GradientPeriod` (the lag count) and not `GradientWindow` (the array length). Indices can… |  | [ ] |
| [TIER-45](#tier-45) | 4206-4208 | H/M | copy-paste | The else-branch sets `CPUEtrend_id = 0` where it should set `CPUEdir_id = 0`, so the direction stays stale and… |  | [ ] |
| [TIER-46](#tier-46) ⭐ | 4229-4231, 4247-4249, 4301-4303, 4321-4323 | H/H | missing update | `if (diff < closest_slope) ref_sp = ind_sp;` never updates `closest_slope` (or `closest_slopeB`), so ref_sp en… |  | [ ] |
| [TIER-47](#tier-47) | 4246 | H/M | inconsistency | The other-category diff is missing `fabs()` (the same-category loop at 3949 has it), so any species with a mor… |  | [ ] |
| [TIER-48](#tier-48) | 4266, 4340 | H/M | wrong variable | `maxTier = mgt_category_id;` defaults to the enum index constant and not the species' current category (`my_ca… |  | [ ] |
| [TIER-49](#tier-49) ⭐ | 5235, ≈5240-5248 | H/H | state not reset / use-after-close | `chdir(dirname)` never returns to the parent directory, so later relative file I/O and next year's mkdir end u… |  | [ ] |
| [TIER-50](#tier-50) ⭐ | 5376-5381 | H/H | copy-paste | The third block under the "Year, Catch and CPUE" header writes `CatchData` a second time where CPUE (`CPUEgen`… |  | [ ] |
| [TIER-51](#tier-51) | ≈5364 | L/M | uninitialised counter | `t` is never incremented, so `t <= HistYrMax` is always true and the retro-catch adjustment applies to all yea… |  | [ ] |
| [TIER-52](#tier-52) | 5576 | H/L | statement inside comment | `...= 0.0; // temporary value ... bm->RBCestimation.RBCspeciesParam[species][EstB0_id] = 999.9;` puts the EstB… |  | [ ] |
| [TIER-53](#tier-53) | ≈5613-5620 | H/M | off-by-one / wrong index | The loop `yr < mgt_rbcYear + n - 1` sums n-1 RBCs but divides by n. The replacement loop writes `RBC_by_year[y… |  | [ ] |
| [TIER-54](#tier-54) | ≈3409 | M/L | wrong array | `tiertype = FunctGroupArray[species].speciesParams[tiertype_id]`: tiertype_id belongs to the RBC_SPECIES_PARAM… |  | [ ] |
| [TIER-55](#tier-55) | 3422-3424, 3458 | M/L | wrong index (log only) | `sb0 = SBioZero[0][0]` is used with `SpawnBio[Nstocks][Nregions]` (elsewhere the overall value is at [Nstocks]… |  | [ ] |
| [TIER-56](#tier-56) | 1866-1867, 2694-2695, 2745-2746 | H/L | debug leftover | Hard-coded `if (bm->dayt > 10584) printf(...)` debug prints left in live code. Functions: `newTier3Rule()`, `T… |  | [ ] |

---

## TIER-01

**wrong operator / sign** · Conf H / Sev H ⭐ priority  
**Lines (current code):** ≈604  
**Lines at review (Oct 2026):** 487-492

**Function:** `DoTierAssessment()`

**Why it's thought to be a bug**

`tac_change = (TAC_old - TACpost)/TAC_old; if (tac_change > inc_check) TAC = TAC_old + inc_check*TAC_old; else if (tac_change < bm->prop_decTAC) TAC = TAC_old - dec_check*TAC_old;` The sign is reversed: a big proposed *decrease* gives a positive tac_change, which then *raises* the TAC to the max. The second test compares against the raw multiplier `prop_decTAC` (e.g. 0.5) and not `-dec_check`, so almost every other case is pushed to the minimum TAC. prop_inc/decTAC are multipliers (see atManageAnnual.c:852-853). Also divides by zero when TAC_old==0. Function: `DoTierAssessment()`.

**Current code** (`atmanage/atManageTier.c` lines 602–606) — ≈ the code here has changed since the review; lines are approximate

```c
  602:             /*
  603:             if (do_debug) {
  604:                 fprintf(llogfp, "Time: %e, %s, year: %d TAC: %e tac_change: %e TAC_old: %e TACpost: %e  min_TACchange %e minCatch: %e MinCatch: %e lastHistCatch: %e inc_check: %e dec_check: %e prop_decTAC: %e min_TACchange: %e\n",
  605:                         bm->dayt, FunctGroupArray[species].groupCode, year, TAC, tac_change, TAC_old, bm->RBCestimation.RBCspeciesParam[species][TACpost_id], bm->RBCestimation.min_TACchange, minCatch, bm->RBCestimation.RBCspeciesParam[species][MinCatch_id], lastHistCatch, inc_check, dec_check, bm->prop_decTAC, bm->RBCestimation.min_TACchange);
  606:             }
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
**Lines (current code):** ≈597  
**Lines at review (Oct 2026):** 501-503

**Function:** `DoTierAssessment()`

**Why it's thought to be a bug**

`if (TAC < minCatch) TAC = FunctGroupArray[species].speciesParams[MinCatch_id];` sets the TAC to the fraction (e.g. 0.05) and not to the computed `minCatch` (fraction × last historical catch). Function: `DoTierAssessment()`.

**Current code** (`atmanage/atManageTier.c` lines 595–599) — ≈ the code here has changed since the review; lines are approximate

```c
  595: 
  596: 			// TAC = TACpost;   // only if want no constraints
  597: 			minCatch = bm->RBCestimation.RBCspeciesParam[species][MinCatch_id] * lastHistCatch;   // Nov 2026: was read from speciesParams
  598:             if (TAC < minCatch) {
  599:                 TAC = minCatch;   //	minCatch set to 5% of final year historic catch
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
**Lines (current code):** ≈637-638  
**Lines at review (Oct 2026):** 535-536

**Function:** `DoTierAssessment()`

**Why it's thought to be a bug**

`if (bm->TACamt[species][nf][now_id] > no_quota) bm->TACamt[species][nf][now_id] = no_quota;` is inside the `nfishery` loop but uses the stale `nf`, which equals K_num_fisheries (or Nfleets) left over from earlier loops. That is an out-of-bounds read/write on TACamt[species], and the cap is never applied to the fishery actually being updated. Function: `DoTierAssessment()`.

**Current code** (`atmanage/atManageTier.c` lines 635–640) — ≈ the code here has changed since the review; lines are approximate

```c
  635:             bm->TACamt[species][nfishery][RBCnow_id] = RBCnf;
  636:             
  637:             if (bm->TACamt[species][nfishery][now_id] > no_quota)
  638:                 bm->TACamt[species][nfishery][now_id] = no_quota;
  639:             
  640:             fprintf(llogfp, "Time: %e %s fishery %d TAC %e (RBC %e) TAC_old: %e, RBC_old: %e\n",
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
**Lines (current code):** 446-480  
**Lines at review (Oct 2026):** 394-399

**Function:** `DoTierAssessment()`

**Why it's thought to be a bug**

`RBC = RBCparam[RBCest_id];` is read *before* `Apply_RBC_Buffers()` changes RBCest_id. The TAC is then worked out from the local, unbuffered `RBC` (lines ≈471/≈471/629), so tier buffers have no effect on the TAC. Function: `DoTierAssessment()`.

**Current code** (`atmanage/atManageTier.c` lines 444–482)

```c
  444: 
  445:     // The RBC is the estimate from the assessment (after any averaging). It is not changed by the buffers below.
  446: 	RBC = bm->RBCestimation.RBCspeciesParam[species][RBCest_id];
  447: 
  448:     /** Nov 2026 (review item 31): buffers are applied after the RBC is calculated, on the way to the TAC.
  449:         TACbase starts as the RBC and the buffers act on it; the TAC is then calculated from TACbase.
  450:         (Previously the buffers overwrote RBCest - so the reported RBC was buffered - while the TAC was calculated
  451:         from the unbuffered RBC, so the buffers never reached the TAC.) **/
  452:     TACbase = RBC;
  453: 
  454:     /** Rollover years: the TAC path starts from the staleness-buffered value if using the Ratpack staleness
  455:         buffer, otherwise from last year's TAC (which already has the buffers in it).
  456:         Nov 2026 (review item 29): multi-year TAC decay - one more year of (1 - myTACbuffer) on last year's TAC in
  457:         rollover years within a multi-year TAC. Now applied whether or not UseTierBuffers is set (it used to be
  458:         inside Apply_RBC_Buffers so only worked with the tier buffers on). Not applied if using the Ratpack
  459:         staleness buffer. **/
  460:     if (tier == sp_rollover) {
  461:         if (staleTACbase >= 0.0) {
  462:             TACbase = staleTACbase;
  463:         } else {
  464:             TACbase = TAC_old;
  465:             TACbase = Apply_MYTAC_Decay(bm, species, year, TACbase, llogfp);
  466:         }
  467:     }
  468: 
  469:     /******* Apply US style tier buffers (OFL to ABC, i.e. RBC towards TAC) ******/
  470:     if (bm->RBCestimation.UseTierBuffers) {
  471:         TACbase = Apply_RBC_Buffers(bm, species, year, tier, TACbase, llogfp);   // Nov 2026: pass effective tier so rollover years are recognised
  472:     }
  473: 
  474:     /** Store the buffered value (after averaging and buffers) from an actual, successful assessment so the Ratpack
  475:         staleness buffer in later rollover years starts from the same value as this year's TAC (discards are removed
  476:         from both below) **/
  477:     if ((tier != sp_rollover) && !bm->RBCestimation.RBCspeciesParam[species][AssessFail_id]) {
  478:         bm->RBCestimation.RBCspeciesParam[species][Ratpack_baseRBC_id] = TACbase;
  479:         bm->RBCestimation.RBCspeciesArray[species].lastAssessRBC = RBC;   // unbuffered RBC, carried through rollover years
  480:     }
  481:     
  482: 	/******* Now calculate TAC from RBC *******/
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
**Lines (current code):** 341-345 (also 2073-2076)  
**Lines at review (Oct 2026):** 325-329 (also 1924-1927)

**Why it's thought to be a bug**

`for (nf...) for (nreg < Nregions) current_catch += Harvest_Get_TotCumCatch(species, nf, bm->thisyear);` The inner region loop does not depend on nreg, so the total catch is multiplied by Nregions. Functions: `DoTierAssessment()`, `Call_Tier3()`.

**Current code** (`atmanage/atManageTier.c` lines 339–347)

```c
  339: 		case 3:  // ----- true F
  340: 			bm->RBCestimation.RBCspeciesParam[species][Fcurr_id] = bm->RBCestimation.RBCspeciesParam[species][True_F_id];
  341: 			// if using true F use only final year's catch + true discard ***************
  342: 			current_catch = 0.0;
  343: 			for (nf = 0; nf < bm->K_num_fisheries; nf++)
  344: 				for (nreg = 0; nreg < Nregions; nreg++)
  345: 					current_catch += Harvest_Get_TotCumCatch(species, nf, bm->thisyear);   // True catchs and discards
  346: 			break;
  347: 		}
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
**Lines (current code):** 405-413  
**Lines at review (Oct 2026):** 376-384

**Function:** `DoTierAssessment()`

**Why it's thought to be a bug**

`case tier8: case tier9: case tier13: RBC = 0.0;` has no break, so it falls into `default` and quits with "No such tier". Function: `DoTierAssessment()`.

**Current code** (`atmanage/atManageTier.c` lines 403–415)

```c
  403:             bm->RBCestimation.RBCspeciesParam[species][RBCest_id] = TAC_old;
  404:         break;
  405:     case tier8:  // US method
  406:     case tier9:  // Norwegian method
  407:     case tier13:  // Icealndic escapement method
  408:         RBC = 0.0;
  409: 	default:
  410: 		printf("No such tier (%d)\n", tier);
  411: 		fprintf(llogfp,"No such tier (%d)\n", tier);
  412: 		quit("");
  413: 		break;
  414: 
  415: 	}
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
**Lines (current code):** 3743-3751  
**Lines at review (Oct 2026):** 3539-3549

**Function:** `Apply_RBC_Buffers()`

**Why it's thought to be a bug**

`case sp_rollover:` has no `break`, so it falls through to `default` and calls `quit("")`. It is also unreachable in practice because `tier` is re-read from `speciesParams[tier_id]` and not from the local `tier` that DoTierAssessment sets to sp_rollover (lines 279/289). As a result a rolled-over TAC gets the base tier's buffer (e.g. sig1) and not the MYTAC buffer. Function: `Apply_RBC_Buffers()`.

**Current code** (`atmanage/atManageTier.c` lines 3741–3753)

```c
 3741:             // Nothing to do for these cases as yet
 3742:             break;
 3743:         case sp_rollover:
 3744:             // Nov 2026 (review item 29): the multi-year TAC decay (myTACbuffer) is now applied in Apply_MYTAC_Decay,
 3745:             // called from DoTierAssessment whether or not UseTierBuffers is set, so nothing to do here.
 3746:             break;
 3747:         default:
 3748:             printf("No such tier (%d)\n", tier);
 3749:             fprintf(llogfp,"No such tier (%d)\n", tier);
 3750:             quit("");
 3751:             break;
 3752:             
 3753: 	}
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
**Lines (current code):** 806-816  
**Lines at review (Oct 2026):** 702-712

**Function:** `Tier_averages()`

**Why it's thought to be a bug**

`if (MinCyr > MaxCyr) MaxCyr = MinCyr;` can push MaxCyr past `year` (future, unfilled years), so it should probably be `MinCyr = MaxCyr`. `nyrs` is computed *before* `MaxCyr` is bumped from 1 to 2, so the mean at 763-771 adds an extra year but divides by the old count. Function: `Tier_averages()`.

**Current code** (`atmanage/atManageTier.c` lines 804–818)

```c
  804: 
  805:     // Sanity check
  806:     if (MaxCyr < 1) MaxCyr = 1;
  807:     if (MaxCyr > year) MaxCyr = year;
  808:     if (MinCyr < 1) MinCyr = 1;
  809:     if (MinCyr > MaxCyr) MaxCyr = MinCyr;
  810:     
  811: 	nyears = MaxCyr - MinCyr + 1;
  812: 	nyrs = (double)(nyears);
  813:     
  814:     if (MaxCyr == 1) {
  815:         MaxCyr = 2;  // Put here to trap case where start dat collection from run start so entry 0 is all zeroes so sets quote to zero as result, at least this way it will count the current years catch that first year
  816:     }
  817:     
  818:     fprintf(bm->logFile, "Time: %e %s year: %d MaxCyr: %d MinCyr: %d\n", bm->dayt, FunctGroupArray[species].groupCode, year, MaxCyr, MinCyr);
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
**Lines (current code):** 822-847  
**Lines at review (Oct 2026):** 718-743

**Function:** `Tier_averages()`

**Why it's thought to be a bug**

`Util_Init_1D_Double(discrate, nyears, 0.0)` zeroes indices 0..nyears-1 (the number of averaging years), not the absolute years `year-3..year` that are used afterwards. Inside the fleet loop `discrate[iy]` is only overwritten when `DiscYears[nf][iy] > 0`, so fleets without discard data inherit the previous fleet's rate (or values from line 911 of the previous call). Function: `Tier_averages()`.

**Current code** (`atmanage/atManageTier.c` lines 820–849)

```c
  820: // calculate average total catch (discard + retained) over last nyears years for each fleet, sum over fleets (for Tier 3 rule)
  821: 	curcatch = 0.0;
  822:     Util_Init_1D_Double(bm->RBCestimation.RBCspeciesArray[species].discrate, nyears, 0.0);
  823: 
  824: 	for (nf = 0; nf < Nfleets; nf++){
  825: 		DiscType = bm->RBCestimation.RBCspeciesArray[species].DiscardType[nf];
  826: 
  827:         // calculate weighted average discard rate over last 4 years by fleet
  828:         startyear = year-3;
  829:         if (startyear < 0) startyear = 0;
  830: 		for (iy = startyear; iy < year+1; iy++){
  831: 			if (bm->RBCestimation.RBCspeciesArray[species].DiscYears[nf][iy] > 0)   // presumably these will be generated if testing tier 3
  832: 			{
  833: 				if (DiscType == 1)
  834: 					bm->RBCestimation.RBCspeciesArray[species].discrate[iy] = bm->RBCestimation.RBCspeciesArray[species].DiscData[nf][sumregion][iy]
  835: 					               / (bm->RBCestimation.RBCspeciesArray[species].DiscData[nf][sumregion][iy]
  836: 					                  + bm->RBCestimation.RBCspeciesArray[species].CatchData[nf][sumregion][iy]);
  837: 				else if (DiscType == 2)
  838: 					bm->RBCestimation.RBCspeciesArray[species].discrate[iy] = bm->RBCestimation.RBCspeciesArray[species].DiscData[nf][sumregion][iy];
  839: 			}
  840:             
  841:             /**/
  842:             //if(do_debug) {
  843:                 fprintf(llogfp,"Time: %e %s iy: %d DiscYears: %d, discrate: %e\n", bm->dayt, FunctGroupArray[species].groupCode, iy,
  844:                     bm->RBCestimation.RBCspeciesArray[species].DiscYears[nf][iy], bm->RBCestimation.RBCspeciesArray[species].discrate[iy]);
  845:             //}
  846:             /**/
  847: 		}
  848: 		avrate = 8 * bm->RBCestimation.RBCspeciesArray[species].discrate[year];
  849:         indexcount = 8.0;
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
**Lines (current code):** 825, 904-907  
**Lines at review (Oct 2026):** 721, 800-803

**Function:** `Tier_averages()`

**Why it's thought to be a bug**

`DiscType = ...DiscardType[nf]`: DiscardType is allocated with size nLen (atAssessTierSetup.c:130), is indexed by fleet, and is never filled anywhere in the repo, so it is always 0. Neither DiscType branch (1/2) ever runs, so discard rates are always 0, or NaN when there is no catch (911). The all-fleet block at 800-803 also uses the `DiscType` left over from the last fleet and not each fleet's own. Function: `Tier_averages()`.

**Current code** (`atmanage/atManageTier.c` lines 823–827)

```c
  823: 
  824: 	for (nf = 0; nf < Nfleets; nf++){
  825: 		DiscType = bm->RBCestimation.RBCspeciesArray[species].DiscardType[nf];
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
**Lines (current code):** 746  
**Lines at review (Oct 2026):** 644

**Function:** `Tier_averages()`

**Why it's thought to be a bug**

`avprop_reg[nf][nreg] /= num_avg_year;` but the sum covers `startyear..year` (5 years, or fewer near the start). When num_avg_year != the number of years summed, avprop does not sum to 1. Function: `Tier_averages()`.

**Current code** (`atmanage/atManageTier.c` lines 744–748)

```c
  744: 				bm->RBCestimation.RBCspeciesArray[species].avprop_reg[nf][nreg] += bm->RBCestimation.RBCspeciesArray[species].propcatch_reg[nf][nreg][iy];
  745: 			}
  746: 			bm->RBCestimation.RBCspeciesArray[species].avprop_reg[nf][nreg] /= bm->RBCestimation.num_avg_year;
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
**Lines (current code):** 1136-1161  
**Lines at review (Oct 2026):** 997-1022

**Function:** `TierOneMimic()`

**Why it's thought to be a bug**

`natm = Mnat[0][0][s][a][index2]` uses `a` (a cohort×genotype index, up to numCohortsXnumGenes) as the *age-in-years* index of Mnat. `num_individuals` and `bodysize` are also summed over all individuals regardless of sex, and then added once per sex, so the yield is doubled when K_num_sexes=2. Function: `TierOneMimic()`.

**Current code** (`atmanage/atManageTier.c` lines 1134–1163)

```c
 1134: 	yield = 0;
 1135: 	for (s=0 ; s < bm->K_num_sexes; s++){
 1136: 		for (a=0; a < FunctGroupArray[species].numCohortsXnumGenes; a++){
 1137:             ia = (int)(a / FunctGroupArray[species].numGeneTypes);
 1138:             sn = FunctGroupArray[species].structNTracers[a];
 1139:             rn = FunctGroupArray[species].resNTracers[a];
 1140:             den = FunctGroupArray[species].NumsTracers[a];
 1141:             
 1142:             totcount = 0.0;
 1143:             num_individuals = 0.0;
 1144:             bodysize = 0.0;
 1145:             for (ij = 0; ij < bm->nbox; ij++) {
 1146:                 for (k = 0; k < bm->boxes[ij].nz; k++) {
 1147:                     if (bm->boxes[ij].tr[k][den] > bm->min_dens) {
 1148:                         bodysize += (bm->boxes[ij].tr[k][sn] + bm->boxes[ij].tr[k][rn]) * bm->X_CN * mg_2_kg;
 1149:                         num_individuals += bm->boxes[ij].tr[k][den];
 1150:                         totcount += 1;
 1151:                     }
 1152:                 }
 1153:             }
 1154:             if (totcount > 0.0) {  // Get the mean size
 1155:                 bodysize /= totcount;
 1156:             }
 1157:             
 1158:             natm = bm->RBCestimation.RBCspeciesArray[species].Mnat[0][0][s][a][index2];
 1159:             sf = bm->RBCestimation.RBCspeciesArray[species].agesel_bysex[s][ia] * ftarg;
 1160:             if( natm || sf ) {
 1161:                 yield += num_individuals * bodysize * (1.0 - exp(-(natm + sf))) * sf/(natm + sf);
 1162:             } else {
 1163:                 yield += 0.0;
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
**Lines (current code):** 1270, 1286  
**Lines at review (Oct 2026):** 1131, 1147

**Function:** `TierThree_CC()`

**Why it's thought to be a bug**

`for (ia = 0; ia <= this_nselage; ia++) ... propgt05[ia] = 1;` reads `numatage[nselage]` and writes `propgt05[nselage]`. Both are allocated with size nselage (atAssessTierSetup.c:466-468). The do-while at 1147 likewise reads `propgt05[ia+1]` with ia+1 == this_nselage. This is left over from Sally's 1-based code. Function: `TierThree_CC()`.

**Current code** (`atmanage/atManageTier.c` lines 1268–1288)

```c
 1268: 					bm->RBCestimation.RBCspeciesArray[species].ages[ia] = a;
 1269: 				}
 1270: 				for ( ia = 0; ia <= this_nselage; ia++){
 1271: 					if( (float)(bm->RBCestimation.RBCspeciesArray[species].numatage[ia] / sumnum) > 0.05)
 1272: 						bm->RBCestimation.RBCspeciesArray[species].propgt05[ia] = 1;
 1273:                     
 1274:                     /*
 1275:                     if (do_debug) {
 1276:                         fprintf(llogfp,"Time: %e %s ages-%d: %e, numatage: %e, propt05: %d\n",
 1277:                                 bm->dayt, FunctGroupArray[species].groupCode, ia, bm->RBCestimation.RBCspeciesArray[species].ages[ia], bm->RBCestimation.RBCspeciesArray[species].numatage[ia], bm->RBCestimation.RBCspeciesArray[species].propgt05[ia]);
 1278:                     }
 1279:                     */
 1280:                     
 1281: 				}
 1282: 
 1283: 				ia = 0;  // as ++ before use (was ia = 1 in Sally's code)
 1284: 				do{
 1285: 					ia++;
 1286: 					isum05 = bm->RBCestimation.RBCspeciesArray[species].propgt05[ia-1] + bm->RBCestimation.RBCspeciesArray[species].propgt05[ia] + bm->RBCestimation.RBCspeciesArray[species].propgt05[ia+1];
 1287: 				}
 1288: 				while ((ia <= this_nselage-2) && (isum05 > 0));  // change from ia<nselage-2 && isum05>0   14-8-08
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
**Lines (current code):** 1260-1265  
**Lines at review (Oct 2026):** 1121-1126

**Function:** `TierThree_CC()`

**Why it's thought to be a bug**

`if ((a == AgeSel95+1) && (numatage[1] < numatage[2]))`: at this point `numatage[2]` has not been filled yet (it is 0), so the test can never be true and the "drop first age if lower than next" logic never runs. In 0-based terms it should compare `numatage[0] < numatage[1]`. Function: `TierThree_CC()`.

**Current code** (`atmanage/atManageTier.c` lines 1258–1267)

```c
 1258: 					// the following throws out the first nos at age if nos < nos at next age (to avoid -Z)
 1259: 					//  make more comparable to doing this by eye
 1260: 					if ( (a == AgeSel95 + 1 ) && ( bm->RBCestimation.RBCspeciesArray[species].numatage[1] < bm->RBCestimation.RBCspeciesArray[species].numatage[2] ) ){
 1261: 						ia = ia - 1;
 1262: 						bm->RBCestimation.RBCspeciesArray[species].numatage[1] = bm->RBCestimation.RBCspeciesArray[species].numatage[2];
 1263: 						bm->RBCestimation.RBCspeciesArray[species].numatage[2] = 0;
 1264: 						sumnum = sumnum - (int)bm->RBCestimation.RBCspeciesArray[species].numatage[1];
 1265: 						this_nselage = nselage - 1;
 1266: 					}
 1267: 					sumnum += (int)bm->RBCestimation.RBCspeciesArray[species].numatage[ia];
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
**Lines (current code):** 1312-1315  
**Lines at review (Oct 2026):** 1173-1176

**Function:** `TierThree_CC()`

**Why it's thought to be a bug**

`nz` is the index of the last stored Z (it starts at -1 and is incremented before use), but `for (i=0; i<nz; i++)` and `meanz = sumz/nz` treat it as a count. The last Z is dropped, and a single valid Z (nz==0) is reported as an assessment failure. Function: `TierThree_CC()`.

**Current code** (`atmanage/atManageTier.c` lines 1310–1317)

```c
 1310: // average Z values over years and fleets
 1311: 	sumz = 0.0;
 1312: 	for (i=0; i < nz; i++)
 1313: 		sumz += bm->RBCestimation.RBCspeciesArray[species].z[i];
 1314: 	if (nz > 0){
 1315: 		meanz = sumz / nz;
 1316: 		bm->RBCestimation.RBCspeciesParam[species][Fcurr_id] =  meanz - bm->RBCestimation.RBCspeciesParam[species][Tier3_M_id] ;
 1317: 	}
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
**Lines (current code):** 1484, 1512  
**Lines at review (Oct 2026):** 1345, 1373

**Function:** `TierThree_CCsel()`

**Why it's thought to be a bug**

`NCA` is initialised to 0 at its declaration and only updated when a year has data. It is not reset per fleet (only `Ncage_id` is), so a fleet with no data reuses the previous fleet's NCA (and the first fleet uses 0). `if (NCA >= 0)` is therefore always true and the minimisation runs on stale or empty Obsageprop. Function: `TierThree_CCsel()`.

**Current code** (`atmanage/atManageTier.c` lines 1482–1514)

```c
 1482:  			if((bm->RBCestimation.RBCspeciesArray[species].AgeFltYr[1][nf][iy]>0) && (bm->RBCestimation.RBCspeciesArray[species].propcatch[nf][iy]> 0.3)) {
 1483: 				bm->RBCestimation.RBCspeciesParam[species][Ncage_id] += 1;     // number of years of catch-at-age data for this fleet
 1484: 				NCA = (int)bm->RBCestimation.RBCspeciesParam[species][Ncage_id];
 1485: 				// store numbers at age
 1486: 				sumage = 0;
 1487: 				for (a = 0; a < Tier3maxage; a++){
 1488: 					// calc num at age over all years used for this fleet
 1489: 					bm->RBCestimation.RBCspeciesArray[species].totnumatage[a] += bm->RBCestimation.RBCspeciesArray[species].AgeComp[nf][FEMALE][iy][1][a];
 1490: 					sumage += bm->RBCestimation.RBCspeciesArray[species].AgeComp[nf][FEMALE][iy][1][a];
 1491: 				}
 1492: 				for (a = 0; a < Tier3maxage; a++){
 1493: 					bm->RBCestimation.RBCspeciesArray[species].Obsageprop[NCA][a] = bm->RBCestimation.RBCspeciesArray[species].AgeComp[nf][FEMALE][iy][1][a] / (sumage + small_num);
 1494:                     /*
 1495: 					if (do_debug){
 1496: 						fprintf(llogfp, "Time: %e, %s f: %d, iy: %d a: %d Obsageprop[%d][%d]: %e sumage: %e (%e)\n",
 1497: 								bm->dayt, FunctGroupArray[species].groupCode, nf, iy, a, NCA, a, bm->RBCestimation.RBCspeciesArray[species].Obsageprop[NCA][a],
 1498:                                 sumage, bm->RBCestimation.RBCspeciesArray[species].totnumatage[a]);
 1499:                    }
 1500:                     */
 1501: 				}
 1502: 			}
 1503: 		}
 1504:         
 1505:         /*
 1506:         if (do_debug) {
 1507:             fprintf(llogfp, "Time %e %s NCA: %d\n",
 1508:                 bm->dayt, FunctGroupArray[species].groupCode, NCA);
 1509:        }
 1510:         */
 1511:         
 1512: 		if (NCA >= 0){    // only do the minimisation if have data from this fleet
 1513: 			// call powell to estimate, S50 and S95
 1514: 
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
**Lines (current code):** ≈1574  
**Lines at review (Oct 2026):** 1435

**Function:** `TierThree_CCsel()`

**Why it's thought to be a bug**

Restart branch: `fvalF[nF] = UnTransform(simplex[1][1], 0.01);` takes the S95 parameter of the second vertex. The F estimate is `simplex[0][0]`, as used at line 1551. Function: `TierThree_CCsel()`.

**Current code** (`atmanage/atManageTier.c` lines 1572–1576) — ≈ the code here has changed since the review; lines are approximate

```c
 1572: 				downhill_simplex(bm, EquilF_Funk_model, species, bm->RBCestimation.RBCspeciesArray[species].simplex, bm->RBCestimation.RBCspeciesArray[species].func, npar, ftol, iter, &ok, llogfp, bm->RBCestimation.RBCspeciesArray[species].xpar);
 1573: 
 1574: 				bm->RBCestimation.RBCspeciesArray[species].fvalF[nF] = UnTransform(bm->RBCestimation.RBCspeciesArray[species].simplex[0][0], 0.01);   // Nov 2026: was simplex[1][1] (S95 of a non-best vertex)
 1575:  
 1576: 			}
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
**Lines (current code):** ≈1601, ≈1614, ≈1620, ≈1624  
**Lines at review (Oct 2026):** 1459, 1472, 1478, 1482

**Function:** `TierThree_CCsel()`

**Why it's thought to be a bug**

`nF` is the index of the last fitted fleet (it starts at -1, then ++), but the loops `i < nF` and the divisor `nF` treat it as a count. With one fleet (nF=0) Fcurr=0 and agesel is all zeros; with more fleets the last fleet is dropped. Function: `TierThree_CCsel()`.

**Current code** (`atmanage/atManageTier.c` lines 1599–1622) — ≈ the code here has changed since the review; lines are approximate

```c
 1599: 	sumF = 0.0;
 1600: 	sumchi = 0.0;
 1601: 	for (i = 0; i < nFit; i++){
 1602: 		sumF += bm->RBCestimation.RBCspeciesArray[species].fvalF[i];
 1603: 		sumchi += bm->RBCestimation.RBCspeciesArray[species].chi[i];
 1604:         
 1605:         /*
 1606:         if (do_debug){
 1607:             printf("Time: %e, %s year: %d fleet: %d sumF: %e, sumchi: %e (Fcurr: %e)\n",
 1608:                     bm->dayt, FunctGroupArray[species].groupCode, year, i, sumF, sumchi, sumF/(nF + small_num));
 1609:             fprintf(llogfp, "Time: %e, %s year: %d fleet: %d sumF: %e, sumchi: %e (Fcurr: %e)\n",
 1610:                     bm->dayt, FunctGroupArray[species].groupCode, year, i, sumF, sumchi, sumF/(nF + small_num));
 1611:         }
 1612:         */
 1613: 	}
 1614: 	bm->RBCestimation.RBCspeciesParam[species][Fcurr_id] = sumF/(nFit + small_num);
 1615:     
 1616:     fprintf(llogfp, "Time %e %s year: %d Fcurr: %e sumF: %e nFit: %d\n",
 1617:             bm->dayt, FunctGroupArray[species].groupCode, year, bm->RBCestimation.RBCspeciesParam[species][Fcurr_id], sumF, nFit);
 1618: 
 1619:     // combine selectivity vectors over fleets
 1620: 	for (i = 0; i < nFit; i++){
 1621: 		bm->RBCestimation.RBCspeciesArray[species].chi[i] = bm->RBCestimation.RBCspeciesArray[species].chi[i] / (sumchi + small_num);
 1622: 	}
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
**Lines (current code):** 1756  
**Lines at review (Oct 2026):** 1614

**Function:** `EquilF_Funk()`

**Why it's thought to be a bug**

`for (iy=0; iy < Ncage_id; iy++)`: Ncage_id is the last index (it starts at -1), so the last year of catch-at-age is left out of the objective. With one year the objective is constant (ss=0), so the fit is meaningless. Function: `EquilF_Funk()`.

**Current code** (`atmanage/atManageTier.c` lines 1754–1758)

```c
 1754: 
 1755: 	ss = 0.0;
 1756: 	for (iy=0; iy < bm->RBCestimation.RBCspeciesParam[species][Ncage_id]; iy++){
 1757: 		for (a=0; a < bm->RBCestimation.RBCspeciesParam[species][Tier3_maxage_id]; a++) {
 1758:  			if (bm->RBCestimation.RBCspeciesArray[species].Obsageprop[iy][a] != 0.0)
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
**Lines (current code):** 1743-1746  
**Lines at review (Oct 2026):** 1601-1604

**Function:** `EquilF_Funk()`

**Why it's thought to be a bug**

The plus-group branch `else if (a == Tier3_maxage)` can never run inside `for (a < Tier3_maxage)`, so the plus group is never accumulated. Function: `EquilF_Funk()`.

**Current code** (`atmanage/atManageTier.c` lines 1741–1748)

```c
 1741: 		sel = 1.0 / (1.0 + exp(-log(19) * (a - inS50) / (inS95 - inS50)));
 1742: 		bm->RBCestimation.RBCspeciesArray[species].Funkz[a] = bm->RBCestimation.RBCspeciesParam[species][Tier3_M_id] + sel * fmort;
 1743: 		if ((a < bm->RBCestimation.RBCspeciesParam[species][Tier3_maxage_id]) && (a > 0))
 1744: 			bm->RBCestimation.RBCspeciesArray[species].num[a] = bm->RBCestimation.RBCspeciesArray[species].num[a-1] * exp(-bm->RBCestimation.RBCspeciesArray[species].Funkz[a-1]);
 1745: 		else if (a == bm->RBCestimation.RBCspeciesParam[species][Tier3_maxage_id])
 1746: 			bm->RBCestimation.RBCspeciesArray[species].num[a] = bm->RBCestimation.RBCspeciesArray[species].num[a-1] * exp(-bm->RBCestimation.RBCspeciesArray[species].Funkz[a-1]) / (1.0 - exp(-bm->RBCestimation.RBCspeciesArray[species].Funkz[a-1]));  // the last z(a-1) should be z(a)
 1747: 		bm->RBCestimation.RBCspeciesArray[species].yield[a] = bm->RBCestimation.RBCspeciesArray[species].num[a] * sel * fmort / bm->RBCestimation.RBCspeciesArray[species].Funkz[a] * (1 - exp(-bm->RBCestimation.RBCspeciesArray[species].Funkz[a]));  // will be equal anyway if fully-selected
 1748: //      catch nos a = numbers * fraction dead due to fishing * total no dead
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
**Lines (current code):** ≈1881-1884 (and 2892-2895)  
**Lines at review (Oct 2026):** 1737-1740 (and 2726-2729)

**Why it's thought to be a bug**

`step1 = (F40-F20)*(Fcurr-F20); frbc = F48 / step1;` is dimensionally wrong (1/F). The linear ramp between F40 and F20 should be `frbc = F48*(F20-Fcurr)/(F20-F40)`. The same code is copied into TierFiveSAFE. Functions: `newTier3Rule()`, `TierFiveSAFE()`.

**Current code** (`atmanage/atManageTier.c` lines 1879–1886) — ≈ the code here has changed since the review; lines are approximate

```c
 1879: 	else if (bm->RBCestimation.RBCspeciesParam[species][Tier3_replicate_old_id] > 0) {
 1880: 		// Original equation - kept so old simulations can be replicated (Tier3_replicate_old_XXX = 1).
 1881: 		// Note it is not a ramp (units of 1/F), e.g. F20 = 0.4, F40 = 0.2, F48 = 0.15, Fcurr = 0.3 gives frbc = 7.5
 1882: 		step1 = (bm->RBCestimation.RBCspeciesParam[species][F40_id] - bm->RBCestimation.RBCspeciesParam[species][F20_id])
 1883: 				* (bm->RBCestimation.RBCspeciesParam[species][Fcurr_id] - bm->RBCestimation.RBCspeciesParam[species][F20_id]);
 1884: 		frbc = bm->RBCestimation.RBCspeciesParam[species][F48_id] / step1;
 1885: 	} else {
 1886: 		// Nov 2026: SESSF Tier 3 ramp - linear from F48 at Fcurr = F40 down to zero at Fcurr = F20
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
**Lines (current code):** 1961-1990  
**Lines at review (Oct 2026):** 1812-1841

**Function:** `getF20_40_48()`

**Why it's thought to be a bug**

`fval` is updated at the *end* of the loop as `i/1000.0`, so iterations 0 and 1 both use F=0 and every later F lags by 0.001. Function: `getF20_40_48()`.

**Current code** (`atmanage/atManageTier.c` lines 1959–1992)

```c
 1959:      */
 1960:     
 1961: 	fval = 0.0;
 1962: 	nfint = (int)(1000 * bm->RBCestimation.RBCspeciesParam[species][Tier3_maxF_id]);   // no of F's over which to calculate YPR
 1963: 
 1964: 	for (i=0; i < nfint; i++){  // f = i/1000  f goes from 0 to 2
 1965: 		YPR(bm, species, fval, natM, &spbpr, wt, mat, agesel, llogfp);
 1966: 
 1967: 		if (i == 0) spb0 = spbpr;
 1968: 		spfrac = spbpr / spb0;
 1969: 
 1970: 		// rec_prog = spfrac / (alpha + beta * spfrac); // Used 2008 - 2015
 1971: 		rec_phd = (spfrac - alpha) / (beta * spfrac);
 1972: 
 1973: 		// spbf = spbpr * rec_prog; // Used 2008 - 2015
 1974: 		spbf = spbpr * rec_phd;
 1975: 
 1976: 		if (i == 0) spbf0 = spbf;
 1977: 
 1978: 		// F20, F40 F48 - originally 0.2, 0.4, 0.48 was hard wired but now have Blim, BrefB, BrefA used instead so can do flexible ref points under new Australian Harvest Strategies
 1979: 		if (spbf > (spbf0*FunctGroupArray[species].speciesParams[Blim_id])) bm->RBCestimation.RBCspeciesParam[species][F20_id] = fval;
 1980: 		if (spbf > (spbf0*FunctGroupArray[species].speciesParams[BrefB_id])) bm->RBCestimation.RBCspeciesParam[species][F40_id] = fval;
 1981: 		if (spbf > (spbf0*FunctGroupArray[species].speciesParams[BrefA_id])) bm->RBCestimation.RBCspeciesParam[species][F48_id] = fval;
 1982: 
 1983: 		/*
 1984: 		if (do_debug){
 1985: 			fprintf(llogfp, "Time: %e %s fval: %e yieldpr: %e rec_prog: %e rec_phd: %e spbf_prog: %e spbf_phd: %e\n",
 1986: 				bm->dayt, FunctGroupArray[species].groupCode, fval, yieldpr, rec_prog, rec_phd, spbf_prog, spbf_phd);
 1987: 		}
 1988: 		 */
 1989: 		fval = i / 1000.0;
 1990: 	}
 1991: 
 1992:     /*
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
**Lines (current code):** 2017-2035  
**Lines at review (Oct 2026):** 1868-1886

**Function:** `YPR()`

**Why it's thought to be a bug**

YPR loops over `numCohorts`, but the `wt/mat/sel` arrays it is given are indexed by age in years (filled up to MaxAge in TierOneMimic/trueF and up to Tier3_maxage in CC/CCsel). When ageClassSize>1, only the first numCohorts *years* are used, so the plus group is wrong. Function: `YPR()`.

**Current code** (`atmanage/atManageTier.c` lines 2015–2037)

```c
 2015: 	int a;
 2016:     
 2017:     Util_Init_1D_Double(bm->RBCestimation.RBCspeciesArray[species].num, FunctGroupArray[species].numCohorts, 0.0);
 2018:     Util_Init_1D_Double(bm->RBCestimation.RBCspeciesArray[species].Funkz, FunctGroupArray[species].numCohorts, 0.0);
 2019: 
 2020: 	*spbpr = 0.0;
 2021: 
 2022: 	for (a=0; a < FunctGroupArray[species].numCohorts; a++)
 2023: 		bm->RBCestimation.RBCspeciesArray[species].Funkz[a] = natM + sel[a] * fval;
 2024: 
 2025: 	bm->RBCestimation.RBCspeciesArray[species].num[0] = 1.0;
 2026: 
 2027:     //  calculate numbers at age relative to age 0
 2028: 	for (a=1; a < FunctGroupArray[species].numCohorts-1; a++)
 2029: 		bm->RBCestimation.RBCspeciesArray[species].num[a] = bm->RBCestimation.RBCspeciesArray[species].num[a-1] * exp(-bm->RBCestimation.RBCspeciesArray[species].Funkz[a-1]);
 2030: 	// plus group
 2031: 	bm->RBCestimation.RBCspeciesArray[species].num[FunctGroupArray[species].numCohorts-1] = bm->RBCestimation.RBCspeciesArray[species].num[FunctGroupArray[species].numCohorts-2] * exp(-bm->RBCestimation.RBCspeciesArray[species].Funkz[FunctGroupArray[species].numCohorts-2])/(1.0 - exp(-bm->RBCestimation.RBCspeciesArray[species].Funkz[FunctGroupArray[species].numCohorts-1]));
 2032: 
 2033:     // YPR
 2034: 	for (a=0; a< FunctGroupArray[species].numCohorts; a++)
 2035: 		*spbpr = *spbpr + bm->RBCestimation.RBCspeciesArray[species].num[a] * wt[a] * mat[a];
 2036: 
 2037: }
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
**Lines (current code):** 2121-2122  
**Lines at review (Oct 2026):** 1971-1972

**Function:** `TierFour()`

**Why it's thought to be a bug**

`maxprop = avprop[1]; maxfleet = 1;` reads out of bounds when Nfleets==1 (avprop has size Nfleets) and defaults to fleet 1 rather than 0. Function: `TierFour()`.

**Current code** (`atmanage/atManageTier.c` lines 2119–2124)

```c
 2119: // which fleet has highest proportion of catch (av over last 5 years) ?
 2120: 	// 17 May 2010 - should check whether CPUE exists for these years and fleets
 2121: 	maxprop = bm->RBCestimation.RBCspeciesArray[species].avprop[1];
 2122: 	maxfleet = 1;
 2123: 	for (nf = 0; nf < Nfleets; nf++)
 2124: 		if (bm->RBCestimation.RBCspeciesArray[species].avprop[nf] > maxprop){
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
**Lines (current code):** 2337, ≈2203  
**Lines at review (Oct 2026):** 2052, 2062

**Function:** `newTierFour()`

**Why it's thought to be a bug**

The CPUE target loop `iy < MaxCyr` excludes the last year, but the divisor `step1 = yrmax - yrmin + 1` counts it (and uses the unsanitised params). So CPUEtarg is biased low. Function: `newTierFour()` (same exclusive loop in `WriteCPUE()` 2177).

**Current code** (`atmanage/atManageTier.c` lines 2335–2339) — ≈ the code here has changed since the review; lines are approximate

```c
 2335:     if (MinCyr > MaxCyr) MaxCyr = MinCyr;
 2336: 
 2337: 	for (iy = MinCyr; iy < MaxCyr; iy++){
 2338: 		step1 = bm->RBCestimation.RBCspeciesArray[species].CPUEgen[fleetid][region][iy];
 2339: 		fprintf(fid, "%e %s %d %d %e %e %e\n",
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
**Lines (current code):** ≈2230-2244  
**Lines at review (Oct 2026):** 2078-2095

**Function:** `newTierFour()`

**Why it's thought to be a bug**

`startyear = year - m; for (iy=startyear; iy<=year; ...)` sums m+1 years but divides by m (or by `year-startyear` when clipped). CPUEav is overstated by (m+1)/m, which makes the RBC multiplier too high. Function: `newTierFour()`.

**Current code** (`atmanage/atManageTier.c` lines 2228–2246) — ≈ the code here has changed since the review; lines are approximate

```c
 2228: 	// calculate average CPUE over last m years
 2229: 	cpuesum = 0.0;
 2230:     startyear = year - (int)bm->RBCestimation.RBCspeciesParam[species][Tier4_m_id] + 1;   // Nov 2026: +1 restored so m years are used (was m+1 years divided by m)
 2231:     if (startyear < 0) startyear = 0;
 2232: 	for (iy = startyear; iy < year + 1; iy++){
 2233: 		cpuesum += bm->RBCestimation.RBCspeciesArray[species].CPUEgen[fleetid][region][iy];
 2234:        
 2235:         /**/
 2236:         //if (do_debug) {
 2237:             fprintf(llogfp, "newTierFour Time: %e %s iy: %d year: %d cpuesum: %e cpuegen: %e (fleet: %d, region: %d)\n",
 2238:                     bm->dayt, FunctGroupArray[species].groupCode, iy, year, cpuesum, bm->RBCestimation.RBCspeciesArray[species].CPUEgen[fleetid][region][iy], fleetid, region);
 2239:         //}
 2240:         /**/
 2241: 	}
 2242:     denom = (double)(year - startyear + 1);   // Number of years summed
 2243: 
 2244:     bm->RBCestimation.RBCspeciesParam[species][CPUEav_id] = cpuesum / (denom + small_num);
 2245: 
 2246: 	// RBC
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
**Lines (current code):** ≈2451, ≈2451, ≈2550  
**Lines at review (Oct 2026):** 2291, 2295, 2388

**Why it's thought to be a bug**

`k = simplex[0][0]; xpar[1] = simplex[0][0];` and in the objective `in_K = xpar[0];`. K is parameter 1 (comment: "xpar[1] = K"), but Bstart is used everywhere instead. The K dimension of the simplex is optimised for nothing, and depletion = BcurrSP/Bstart. Functions: `TierFive_SP()`, `SurplusProduction()`.

**Current code** (`atmanage/atManageTier.c` lines 2449–2453) — ≈ the code here has changed since the review; lines are approximate

```c
 2449: 
 2450: 	/* Nov 2026: K is now its own parameter (simplex[0][1]). Original handling, in case need to switch back:
 2451: 	   K was set equal to Bstart, i.e.  k = simplex[0][0]  (and xpar[1] below = simplex[0][0], in_K = xpar[0] in SurplusProduction) */
 2452: 	bstart = bm->RBCestimation.RBCspeciesArray[species].simplex[0][0];
 2453: 	k = bm->RBCestimation.RBCspeciesArray[species].simplex[0][1];
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
**Lines (current code):** ≈2561  
**Lines at review (Oct 2026):** 2397-2398

**Function:** `SurplusProduction()`

**Why it's thought to be a bug**

`step1 = (1 - (bt/K)^p) - T5_removal[iy-1]; B = bt + bt*r/p*step1;` multiplies the catch removal by `bt*r/p`. The projection loop at 2432-2433 correctly subtracts the removal outside. Function: `SurplusProduction()`.

**Current code** (`atmanage/atManageTier.c` lines 2559–2563) — ≈ the code here has changed since the review; lines are approximate

```c
 2559: 			/* Nov 2026: removals now subtracted after the production term (Pella-Tomlinson B + (r/p)B(1-(B/K)^p) - C).
 2560: 			   Original handling, in case need to switch back - removals were inside the production term:
 2561: 			     step1 = (1.0 - exp(p * log(bt/in_K))) - T5_removal[iy-1];
 2562: 			     expectB[iy] = bt + bt * in_r / p * step1;  */
 2563: 			step1 = 1.0 - exp(bm->RBCestimation.RBCspeciesParam[species][Tier5_p_id] * log(bt/in_K));
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
**Lines (current code):** 2596-2604  
**Lines at review (Oct 2026):** 2430-2438

**Function:** `SurplusProduction()`

**Why it's thought to be a bug**

The projection loop `iy < index2` runs zero times for AssessDelay 0 and only to CurrentYear for delay 1. Then `BcurrSP = expectB[index1]` reads an entry that was never computed (outside the Init range at 2380, so stale or 0). The fitting loop also stops at `iy < CurrentYear`. Function: `SurplusProduction()`.

**Current code** (`atmanage/atManageTier.c` lines 2594–2606)

```c
 2594: 
 2595: 	// project biomass to start of year+2 (because know catch in year+1 (year of assessment)
 2596: 	for (iy=bm->RBCestimation.RBCspeciesArray[species].CurrentYear; iy < index2; iy++){
 2597: 		bt = bm->RBCestimation.RBCspeciesArray[species].expectB[iy-1];
 2598: 		bm->RBCestimation.RBCspeciesArray[species].expectB[iy] = bt + bt * in_r / bm->RBCestimation.RBCspeciesParam[species][Tier5_p_id] * (1.0 - exp(bm->RBCestimation.RBCspeciesParam[species][Tier5_p_id] * log(bt / in_K)))
 2599: 				- bm->RBCestimation.RBCspeciesArray[species].T5_removal[iy-1];
 2600: 		if (bm->RBCestimation.RBCspeciesArray[species].expectB[iy] < 75.0) bm->RBCestimation.RBCspeciesArray[species].expectB[iy] = 75.0;
 2601: 	}
 2602: 
 2603: 	// BcurrSP is biomass at start of CurrentYear+2  (CurrentYear is last year of data, but do have catch for CurrentYear+1)
 2604: 	bm->RBCestimation.RBCspeciesParam[species][BcurrSP_id] = bm->RBCestimation.RBCspeciesArray[species].expectB[index1];    // save in global for use when solution found
 2605: 
 2606: 	return ll;      // minimise negative ll
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
**Lines (current code):** 2641-2646  
**Lines at review (Oct 2026):** 2475-2480

**Function:** `get_T5_F20_40_48()`

**Why it's thought to be a bug**

`do { iy++; ... TSbiomass[iy] = ... } while (... && iy < 1000.0);` can write `TSbiomass[1000]`, but the array is allocated with 1000 entries (atAssessTierSetup.c:504). Function: `get_T5_F20_40_48()`.

**Current code** (`atmanage/atManageTier.c` lines 2639–2648)

```c
 2639: 		iy = 0;
 2640: 		// project the population with this F until it stabilises
 2641: 		do {
 2642: 			iy++;
 2643: 			bt = bm->RBCestimation.RBCspeciesArray[species].TSbiomass[iy-1];
 2644: 			step1 = (1.0 - exp(bm->RBCestimation.RBCspeciesParam[species][Tier5_p_id] * log(bt/k)));
 2645: 			bm->RBCestimation.RBCspeciesArray[species].TSbiomass[iy] = bt + bt * r / bm->RBCestimation.RBCspeciesParam[species][Tier5_p_id] * step1 - bt * (1.0 - exp(-1.0 * fval));
 2646: 		}  while ((fabs(bm->RBCestimation.RBCspeciesArray[species].TSbiomass[iy] - bm->RBCestimation.RBCspeciesArray[species].TSbiomass[iy-1]) > 1.0) && (iy < 1000.0));
 2647: 
 2648: 		longterm_dep = bm->RBCestimation.RBCspeciesArray[species].TSbiomass[iy] / k;
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
**Lines (current code):** 2749-2755  
**Lines at review (Oct 2026):** 2583-2589

**Function:** `TierFive_AvLen()`

**Why it's thought to be a bug**

`do { i=i+1; } while ((avlen < T5_Avlen_atF[i]) && (i < nfint));` evaluates `T5_Avlen_atF[nfint]` (array size nFtier3 == nfint) before the bound check, and never tests index 1. Function: `TierFive_AvLen()`.

**Current code** (`atmanage/atManageTier.c` lines 2747–2757)

```c
 2747: 
 2748:     
 2749:     i = 1;
 2750: 	do { i = i+1; }
 2751: 	while ((avlen < bm->RBCestimation.RBCspeciesArray[species].T5_Avlen_atF[i])
 2752: 			&& (i < nfint));
 2753: 
 2754: 	bm->RBCestimation.RBCspeciesParam[species][Avlen_id] = avlen;
 2755: 	bm->RBCestimation.RBCspeciesParam[species][Fcurr_id] = bm->RBCestimation.RBCspeciesArray[species].T5_Fval[i-1];
 2756: 	bm->RBCestimation.RBCspeciesParam[species][PropAvlen_id] = nolen / (totlen + small_num);    // proportion of catch > reference length
 2757:     
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
**Lines (current code):** 2805-2815  
**Lines at review (Oct 2026):** 2639-2649

**Function:** `TierFiveDCAC()`

**Why it's thought to be a bug**

`for (iy...) { sumcatch = 0.0; ... }` resets the sum every year, so DCAC uses only the final year's catch and not the cumulative catch over `num_years`. Function: `TierFiveDCAC()`.

**Current code** (`atmanage/atManageTier.c` lines 2803–2817)

```c
 2803:     
 2804:     sumcatch = 0.0;
 2805:     for (iy = 0; iy < year+1; iy++) {
 2806: 		sumcatch = 0.0;
 2807: 		for (nreg = 0; nreg < Nregions ; nreg++){
 2808: 			for (nf = 0; nf< Nfleets; nf++){
 2809: 				sumcatch += bm->RBCestimation.RBCspeciesArray[species].CatchData[nf][nreg][iy];  // Use this for "pseudo-data" in MSE and summing oonly over assessment NumFisheries
 2810:                 if (bm->flagTACincludeDiscard) {
 2811:                     sumcatch += bm->RBCestimation.RBCspeciesArray[species].DiscData[nf][nreg][iy];
 2812:                 }
 2813: 			}
 2814: 		}
 2815:     }
 2816:     
 2817:     bm->RBCestimation.RBCspeciesParam[species][RBCest_id] = sumcatch / step1;
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
**Lines (current code):** 2934-2940  
**Lines at review (Oct 2026):** 2768-2774

**Function:** `TierSixRUSS()`

**Why it's thought to be a bug**

`localcatch` is not reset per year, so `HistCatch` ends up as cumulative total catch rather than the peak annual catch (compare TierSevenRUSS 3051, which resets it). Function: `TierSixRUSS()`.

**Current code** (`atmanage/atManageTier.c` lines 2932–2942)

```c
 2932:     
 2933:     // Get historical level of catch
 2934:     for (iy = 0; iy < bm->RBCestimation.RBCspeciesParam[species][HistYrMax_id]; iy++){
 2935:         for (nf = 0; nf< Nfleets; nf++){
 2936:             localcatch += bm->RBCestimation.RBCspeciesArray[species].CatchData[nf][Nregions][iy];
 2937:         }
 2938:         if (localcatch > HistCatch)
 2939:             HistCatch = localcatch;
 2940:     }
 2941: 
 2942:     // First, see if any triggers have been reached for any level in the previous year:
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
**Lines (current code):** 2948-2960  
**Lines at review (Oct 2026):** 2782-2794

**Function:** `TierSixRUSS()`

**Why it's thought to be a bug**

`RBCspeciesParam[species][TriggerReached_id] = 0;` is reset per region, but the array `TriggerReached[nreg]` (and `[Nregions]`) that the following loops set is never reset. Once a trigger has been reached it stays reached forever. Function: `TierSixRUSS()`.

**Current code** (`atmanage/atManageTier.c` lines 2946–2962)

```c
 2946:             regsumcatch += bm->RBCestimation.RBCspeciesArray[species].CatchData[nf][nreg][year];
 2947:         }
 2948:         bm->RBCestimation.RBCspeciesParam[species][TriggerReached_id] = 0;
 2949:         for (nt=0; nt < Ntriggers; nt++) {
 2950:             if (regsumcatch > (bm->RBCestimation.RBCspeciesArray[species].TriggerPoints[nt][nreg] * HistCatch))
 2951:                  bm->RBCestimation.RBCspeciesArray[species].TriggerReached[nreg] = nt;
 2952:         }
 2953:     }
 2954:     totsumcatch = 0.0;
 2955:     for (nf = 0; nf< Nfleets; nf++){
 2956:         totsumcatch += bm->RBCestimation.RBCspeciesArray[species].CatchData[nf][Nregions][year];
 2957:     }
 2958:     for (nt=0; nt < Ntriggers; nt++) {
 2959:         if (totsumcatch > (bm->RBCestimation.RBCspeciesArray[species].TriggerPoints[nt][Nregions] * HistCatch))
 2960:              bm->RBCestimation.RBCspeciesArray[species].TriggerReached[Nregions] = nt;
 2961:     }
 2962: 
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
**Lines (current code):** 2992-2993, 3001, 3014, 3017, 3029-3033  
**Lines at review (Oct 2026):** 2826-2827, 2835, 2848, 2851, 2863-2867

**Function:** `TierSixRUSS()`

**Why it's thought to be a bug**

`DoCPUE(bm, year, species, llogfp)`, `DoPrager(bm, year, species, ...)`, `Call_Tier3(bm, year, species, ...)` and `TierFiveSAFE(bm, year, species, totsumcatch, ...)` all pass (year, species) where the prototypes (lines 82-83, 2048, 59) expect (species, year). The wrong species is assessed, or there are out-of-bounds species indices. Function: `TierSixRUSS()`.

**Current code** (`atmanage/atManageTier.c` lines 2990–3016)

```c
 2990:                 
 2991:                 if(bm->RBCestimation.tier6_to_tier4) {
 2992:                     adjTAC = DoCPUE(bm, year, species, llogfp); // To get average CPUE
 2993:                     adjTAC = DoPrager(bm, year, species, llogfp);
 2994: 
 2995:                     // If we adjust from last existing level, we will likely end up at trigger level 3 again almost immediately so to be safe apply an addition reduction factor (RBCadjscalar = 0.3 in Nat's code)
 2996:                     adjTAC *= (1.0 - bm->RBCestimation.RBCspeciesParam[species][RBCadjscalar_id]);
 2997:                     prev_RBC = bm->RBCestimation.RBCspeciesParam[species][lastRBC_id];
 2998:                 
 2999:                 } else {
 3000:                     // Do length based assessment
 3001:                     Call_Tier3(bm, year, species, llogfp);
 3002:                     
 3003:                     // map back to local variables so don't need to make code below conditional on method used
 3004:                     adjTAC = 1.0;
 3005:                     prev_RBC = bm->RBCestimation.RBCspeciesParam[species][RBCest_id];
 3006:                 }
 3007:             }
 3008:             bm->RBCestimation.RBCspeciesParam[species][Closed_id] = 0;
 3009:             break;
 3010:         case 1:  // Level 1 trigger reached in at least one zone so react accordingly (unless not responding to level 1 triggers) - either with a CPUE-based empirical Tier 4 rule or a length-based Tier 3
 3011:             if (TriggerScen != LaxZoneTrigger) {
 3012:                 if(bm->RBCestimation.tier6_to_tier4)
 3013:                     // Do CPUE trend analysis
 3014:                     adjTAC = DoCPUE(bm, year, species, llogfp);
 3015:                 else {
 3016:                     // Do SAFE assessment
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
**Lines (current code):** ≈3104-3116  
**Lines at review (Oct 2026):** 2938-2950

**Function:** `DoCPUE()`

**Why it's thought to be a bug**

`for (iy = year-1; iy >= 0; iy++)` counts *up* without bound (runs off the end of EffortData). Also `j = iy` is assigned only when effort was *not* found, so j is not the most recent year with effort (if found immediately, j stays 0). Function: `DoCPUE()`.

**Current code** (`atmanage/atManageTier.c` lines 3102–3118) — ≈ the code here has changed since the review; lines are approximate

```c
 3102:     
 3103: 	// Calculate CPUE for the last x years, beginning with latest non-zero effort year
 3104: 	// Nov 2026: loop now counts back (was iy++ so ran forward off the end of the arrays) and j is the year found
 3105: 	for(iy = year-1; iy >= 0; iy--) { // establish most recent non-zero effort year
 3106:         for (f = 0; f < Nfleets; f++) {
 3107:             if (bm->RBCestimation.RBCspeciesArray[species].EffortData[f][Nregions][iy] > 0.0) {
 3108:                 is_done = 1;
 3109:                 break;
 3110:             }
 3111:         }
 3112:         if (is_done) {
 3113:             j = iy;
 3114:             break;
 3115:         }
 3116: 	}
 3117:     
 3118:     nyrs = 4;
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
**Lines (current code):** 3129  
**Lines at review (Oct 2026):** 2963

**Function:** `DoCPUE()`

**Why it's thought to be a bug**

`AnnCPUE[5-iy]` with iy going down to 0 writes `AnnCPUE[5]`, but AnnCPUE is allocated with 5 entries (atAssessTierSetup.c:215). Function: `DoCPUE()`.

**Current code** (`atmanage/atManageTier.c` lines 3127–3131)

```c
 3127:             totC += bm->RBCestimation.RBCspeciesArray[species].CatchData[f][Nregions][j-iy];
 3128:         }
 3129:         bm->RBCestimation.RBCspeciesArray[species].AnnCPUE[5-iy] = totC / totEff;
 3130: 	}
 3131:     
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
**Lines (current code):** 3210, 3244, 3263, 3349, 3362, 3377  
**Lines at review (Oct 2026):** 3044, 3078, 3097, 3183, 3196, 3211

**Function:** `TierSevenRUSS()`

**Why it's thought to be a bug**

`cosp = FunctGroupArray[sp].co_sp[sp];` uses the loop counter as the group index. It should be the focal species' companion list, `FunctGroupArray[species].co_sp[sp]` (co_sp is per guild: atManageParamIO.c:259/932). Function: `TierSevenRUSS()`.

**Current code** (`atmanage/atManageTier.c` lines 3208–3247 (truncated))

```c
 3208:     for (sp = 0; sp < bm->K_max_co_sp; sp++) {
 3209:         
 3210:         cosp = FunctGroupArray[sp].co_sp[sp];
 3211:         if ((cosp < 0) || (cosp > bm->K_max_impacted_sp))
 3212:             continue;
 3213:         
 3214:         // Get historical level of catch
 3215:         HistCatch = 0.0;
 3216:         for (iy = 0; iy < bm->RBCestimation.RBCspeciesParam[species][HistYrMax_id]; iy++){
 3217:             localcatch = 0.0;
 3218:             for (nf = 0; nf< bm->K_num_fisheries; nf++){
 3219:                 localcatch += Harvest_Get_TotCumCatch(cosp, nf, iy);
 3220:             }
 3221:             if (localcatch > HistCatch)
 3222:                 HistCatch = localcatch;
 3223:         }
 3224:         
 3225:         this_catch = 0.0;
 3226:         for (nf = 0; nf < bm->K_num_fisheries; nf++){
 3227:             this_catch += Harvest_Get_TotCumCatch(cosp, nf, year);
 3228:         }
 3229:         tot_co_catch += this_catch;
 3230:         bm->RBCestimation.RBCspeciesArray[species].CatchComp[cosp][year] = this_catch;
 3231:         
 3232:         // Do peak catch check
 3233:         for (nt=0; nt < Ntriggers; nt++) {
 3234:             if (this_catch > (bm->RBCestimation.RBCspeciesArray[species].TriggerPoints[nt][Nregions] * HistCatch))
 3235:                 bm->RBCestimation.RBCspeciesArray[species].TriggerReached[Nregions] = nt;
 3236:         }
 3237:     }
 3238:     
 3239:     /* Aggregate catch over the group and then apply similar triggers to tier 6 */
 3240:     CoHistCatch = 0.0;
 3241:     for (iy = 0; iy < bm->RBCestimation.RBCspeciesParam[species][HistYrMax_id]; iy++){
 3242:         hist_co_catch = 0.0;
 3243:         for (sp = 0; sp < bm->K_max_co_sp; sp++) {
 3244:             cosp = FunctGroupArray[sp].co_sp[sp];
 3245:             if ((cosp < 0) || (cosp > bm->K_max_impacted_sp))
 3246:                 continue;
 3247:             for (nf = 0; nf< bm->K_num_fisheries; nf++){
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
**Lines (current code):** 3312-3313  
**Lines at review (Oct 2026):** 3146-3147

**Function:** `TierSevenRUSS()`

**Why it's thought to be a bug**

`if (bm->SP_FISHERYprms[cosp][nf][q_id] > 0.0) toteffort += bm->CumEffort[nf][b];` sits after the nf loops have finished, so nf == K_num_fisheries and both arrays are read out of bounds. Function: `TierSevenRUSS()`.

**Current code** (`atmanage/atManageTier.c` lines 3310–3315)

```c
 3310:             }
 3311: 
 3312:             if (bm->SP_FISHERYprms[cosp][nf][q_id] > 0.0)
 3313:                 toteffort += bm->CumEffort[nf][b];
 3314: 
 3315:             if((localcatch / (totcatch + small_num)) > maxrelcatch)
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
**Lines (current code):** 3290-3322  
**Lines at review (Oct 2026):** 3124-3156

**Function:** `TierSevenRUSS()`

**Why it's thought to be a bug**

`areafished` and `areasp` (and `toteffort`) are initialised once at function entry and not per co-species, so `relarea` for later species includes earlier species' areas. Function: `TierSevenRUSS()`.

**Current code** (`atmanage/atManageTier.c` lines 3288–3324)

```c
 3288:     /* Now check if the proportion of the area fished changes > y% */
 3289:     /* Also check if the proportion of the catch from any one location is > z% */
 3290:         relarea = 0.0;
 3291:         maxrelcatch = 0.0;
 3292:         for(b=0; b<bm->nbox; b++) {
 3293:             biomhere = 0;
 3294:             totcatch = 0.0;
 3295:             for (nf = 0; nf < bm->K_num_fisheries; nf++){
 3296:                 totcatch += Harvest_Get_TotCumCatch(cosp, nf, year);
 3297:             }
 3298:             localcatch = 0.0;
 3299:             for(ij=0; ij<bm->boxes[b].nz; ij++) {
 3300:                 for (nf = 0; nf < bm->K_num_fisheries; nf++) {
 3301:                     if ((bm->CumCatch[cosp][nf][b][ij] + bm->CumDiscards[cosp][nf][b]) > 0.0){
 3302:                         localcatch += bm->CumCatch[cosp][nf][b][ij];
 3303:                     }
 3304:                 }
 3305:                 for (cohort = 0; cohort < FunctGroupArray[cosp].numCohorts * FunctGroupArray[cosp].numGeneTypes; cohort++) {
 3306:                     den = FunctGroupArray[cosp].NumsTracers[cohort];
 3307:                     if ( bm->boxes[b].tr[ij][den] > bm->min_dens)
 3308:                         biomhere = 1;
 3309:                 }
 3310:             }
 3311: 
 3312:             if (bm->SP_FISHERYprms[cosp][nf][q_id] > 0.0)
 3313:                 toteffort += bm->CumEffort[nf][b];
 3314: 
 3315:             if((localcatch / (totcatch + small_num)) > maxrelcatch)
 3316:                 maxrelcatch = localcatch / (totcatch + small_num);
 3317:             
 3318:             if(localcatch)
 3319:                 areafished += bm->boxes[b].area;
 3320:             if(biomhere)
 3321:                 areasp += bm->boxes[b].area;
 3322:             relarea = areafished / (areasp + small_num);
 3323:         }
 3324:         bm->RBCestimation.RBCspeciesArray[species].relareafished[cosp][year] = relarea;
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
**Lines (current code):** 3382  
**Lines at review (Oct 2026):** 3216

**Function:** `TierSevenRUSS()`

**Why it's thought to be a bug**

`Call_Tier3(bm, year, species, llogfp)` has its arguments swapped, and inside the loop over cosp it should assess `cosp`. Function: `TierSevenRUSS()`.

**Current code** (`atmanage/atManageTier.c` lines 3380–3384)

```c
 3380:         
 3381:         if (bm->RBCestimation.RBCspeciesArray[cosp].TriggerReached[Nregions] == 2)
 3382:             Call_Tier3(bm, year, species, llogfp);
 3383:         
 3384:         bm->RBCestimation.RBCspeciesParam[cosp][TriggerReached_id] = bm->RBCestimation.RBCspeciesArray[cosp].TriggerReached[Nregions];
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
**Lines (current code):** 3997-4015  
**Lines at review (Oct 2026):** 3718-3736

**Function:** `trigger_species_breakout()`

**Why it's thought to be a bug**

`catchwghtCPUE` is zeroed once outside the `lag` loop, so every lag adds into the same slots. The regression for lag k therefore includes data from lags > k. Also, when mgt_rbcYear==this_year, `yy` runs 1..TCP-1, so slot 0 is never filled and a spurious 0 enters the regression. Division by `totCatch` can also be 0. Function: `trigger_species_breakout()`.

**Current code** (`atmanage/atManageTier.c` lines 3995–4017)

```c
 3995:     /* (long-term) calculate 10-year trend */
 3996:     /* over past 4 years */
 3997:     Util_Init_1D_Double(bm->RBCestimation.catchwghtCPUE, bm->RBCestimation.TriggerCheckPeriod, 0.0);
 3998:     for (lag = bm->RBCestimation.LagPeriod; lag >= 0; lag--){
 3999:         for (y = (bm->RBCestimation.RBCspeciesArray[sp].mgt_rbcYear - bm->RBCestimation.TriggerCheckPeriod - lag); y < (this_year - 1 - lag); y++){
 4000:             totCatch = 0;
 4001:             for (r = 0; r < bm->RBCestimation.RBCspeciesParam[sp][NumRegions_id]; r++) {
 4002:                 for (f = 0; f < bm->RBCestimation.RBCspeciesParam[sp][NumFisheries_id]; f++) {
 4003:                     totCatch += bm->RBCestimation.RBCspeciesArray[sp].CatchData[f][r][y];
 4004:                 }
 4005:             }
 4006:         
 4007:             yy = y - (this_year - bm->RBCestimation.TriggerCheckPeriod) + 1 + lag;
 4008:             for (r = 0; r < bm->RBCestimation.RBCspeciesParam[sp][NumRegions_id]; r++) {
 4009:                 for (f = 0; f < bm->RBCestimation.RBCspeciesParam[sp][NumFisheries_id]; f++) {
 4010:                 
 4011:                     bm->RBCestimation.catchwghtCPUE[yy] += (bm->RBCestimation.RBCspeciesArray[sp].CatchData[f][r][y] / totCatch) * bm->RBCestimation.RBCspeciesArray[sp].CPUEgen[f][r][y];
 4012:                     //fprintf(bm->logFile, "LAG %d catched weighted CPUE %d --- %d %f\n" lag, y, yy, catchwghtCPUE[yy]);
 4013:                 }
 4014:             }
 4015:         }
 4016:         fprintf(bm->logFile, "Time: %e %s CPUE TRIGGER : TRIGGER species Long-term trend\n", bm->dayt, FunctGroupArray[sp].groupCode);
 4017:         for (y = 0; y < bm->RBCestimation.TriggerCheckPeriod; y++){
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
**Lines (current code):** 4066  
**Lines at review (Oct 2026):** 3787

**Function:** `CalculateNonIndicatorRBC()`

**Why it's thought to be a bug**

`(FunctGroupArray[sp].isImpacted == FALSE)` tests the target species and not the candidate indicator `i`. Function: `CalculateNonIndicatorRBC()`.

**Current code** (`atmanage/atManageTier.c` lines 4064–4068)

```c
 4064:         bm->RBCestimation.RBCspeciesParam[sp][TACpost_id] = bm->RBCestimation.RBCspeciesParam[sp][TACest_id]; // Last year's TAC
 4065:         for (i = 0; i < bm->K_num_tot_sp; i++) {
 4066:             if (sp == i || (FunctGroupArray[sp].isImpacted == FALSE) || bm->RBCestimation.RBCspeciesParam[sp][mgt_category_id] != bm->RBCestimation.RBCspeciesParam[i][mgt_category_id] || !bm->RBCestimation.RBCspeciesParam[i][mgt_indicator_id]) continue;
 4067:             nInds++;
 4068:             
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
**Lines (current code):** 4156-4189, 4178  
**Lines at review (Oct 2026):** 3877-3910, 3899

**Function:** `Check_Indicator_Links()`

**Why it's thought to be a bug**

The `yy` offset uses `GradientPeriod` (the lag count) and not `GradientWindow` (the array length). Indices can exceed the catchwght/catchwghtCPUE size (max(GradientWindow, TriggerCheckPeriod)), or be clipped so that many years pile into slot 0. The arrays are also not reset between lags (same issue as TIER-42). Function: `Check_Indicator_Links()`.

**Current code** (`atmanage/atManageTier.c` lines 4154–4191)

```c
 4154:             printf("Doing Check_Indicator_Links Doing species %s ", FunctGroupArray[sp].groupCode);
 4155:             
 4156:             Util_Init_1D_Double(bm->RBCestimation.catchwghtCPUE, bm->RBCestimation.GradientWindow, 0.0);
 4157:             Util_Init_1D_Double(bm->RBCestimation.catchwght, bm->RBCestimation.GradientWindow, 0.0);
 4158:             for (lag = bm->RBCestimation.GradientPeriod; lag >= 0; lag--){
 4159:                 
 4160:                 printf("Doing Check_Indicator_Links Doing species %s lag %d (RBCestimation.GradientPeriod: %d)\n", FunctGroupArray[sp].groupCode, lag, bm->RBCestimation.GradientPeriod);
 4161:                 
 4162:                 starty = (bm->RBCestimation.RBCspeciesArray[sp].mgt_rbcYear - bm->RBCestimation.GradientWindow - lag);
 4163:                 
 4164:                 if (starty < 0)
 4165:                     starty = 0;
 4166:                 endy = (year - 1 - lag);
 4167:                 if (endy < 0)
 4168:                     endy = 0;
 4169:                 
 4170:                 for (y = starty; y < endy; y++){
 4171:                     totCatch = 0;
 4172:                     for (r = 0; r < bm->RBCestimation.RBCspeciesParam[sp][NumRegions_id]; r++) {
 4173:                         for (f = 0; f < bm->RBCestimation.RBCspeciesParam[sp][NumFisheries_id]; f++) {
 4174:                             totCatch += bm->RBCestimation.RBCspeciesArray[sp].CatchData[f][r][y];
 4175:                         }
 4176:                     }
 4177:             
 4178:                     yy = y - (year - bm->RBCestimation.GradientPeriod) + 1 + lag;  // TODO: Check whether this should be a different period - is it ok like this?
 4179:                     
 4180:                     if (yy < 0)
 4181:                         yy = 0;
 4182: 
 4183:                     for (r = 0; r < bm->RBCestimation.RBCspeciesParam[sp][NumRegions_id]; r++) {
 4184:                         for (f = 0; f < bm->RBCestimation.RBCspeciesParam[sp][NumFisheries_id]; f++) {
 4185:                             bm->RBCestimation.catchwghtCPUE[yy] += (bm->RBCestimation.RBCspeciesArray[sp].CatchData[f][r][y] / totCatch) * bm->RBCestimation.RBCspeciesArray[sp].CPUEgen[f][r][y];
 4186:                             bm->RBCestimation.catchwght[yy] += bm->RBCestimation.RBCspeciesArray[sp].CatchData[f][r][y] / totCatch;
 4187:                         }
 4188:                     }
 4189:                 }
 4190:             
 4191:                 /*
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
**Lines (current code):** 4206-4208  
**Lines at review (Oct 2026):** 3927-3929

**Function:** `Check_Indicator_Links()`

**Why it's thought to be a bug**

The else-branch sets `CPUEtrend_id = 0` where it should set `CPUEdir_id = 0`, so the direction stays stale and the trend is wiped before it is used in steps B/C. Function: `Check_Indicator_Links()`.

**Current code** (`atmanage/atManageTier.c` lines 4204–4210)

```c
 4204:                 } else if (bm->RBCestimation.RBCspeciesParam[sp][CPUEtrend_id] < (0 - buffer)) {
 4205:                     bm->RBCestimation.RBCspeciesParam[sp][CPUEdir_id] = -1;
 4206:                 } else {
 4207:                     bm->RBCestimation.RBCspeciesParam[sp][CPUEtrend_id] = 0;
 4208:                 }
 4209:             }
 4210:         }
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
**Lines (current code):** 4229-4231, 4247-4249, 4301-4303, 4321-4323  
**Lines at review (Oct 2026):** 3950-3952, 3968-3970, 4022-4024, 4042-4044

**Function:** `Check_Indicator_Links()`

**Why it's thought to be a bug**

`if (diff < closest_slope) ref_sp = ind_sp;` never updates `closest_slope` (or `closest_slopeB`), so ref_sp ends up as the *last* candidate (possibly `sp` itself), not the closest. And `closest_slope < closest_slopeB` (4260/4334) compares MAXDOUBLE with MAXDOUBLE, so ref_spB is always chosen. Function: `Check_Indicator_Links()`.

**Current code** (`atmanage/atManageTier.c` lines 4227–4251)

```c
 4227:                     if (bm->RBCestimation.RBCspeciesParam[sp][mgt_category_id] == bm->RBCestimation.RBCspeciesParam[ind_sp][mgt_category_id]) {
 4228:                         diff = fabs(bm->RBCestimation.RBCspeciesParam[sp][CPUEtrend_id] - bm->RBCestimation.RBCspeciesParam[ind_sp][CPUEtrend_id]);
 4229:                         if (diff < closest_slope) {
 4230:                             ref_sp = ind_sp;
 4231:                         }
 4232:                         if (diff < buffer) {
 4233:                             bm->RBCestimation.count_sp_in_buffer[my_category]++;
 4234:                         }
 4235:                     }
 4236:                 }
 4237:             }
 4238:             
 4239:             // Now check other categories
 4240:             closest_slopeB = MAXDOUBLE;
 4241:             ref_spB = -1;
 4242:             for (ind_sp = 0; ind_sp < bm->K_num_tot_sp; ind_sp++) {
 4243:                 if( FunctGroupArray[ind_sp].isImpacted == TRUE) {
 4244:                     that_category = bm->RBCestimation.RBCspeciesParam[ind_sp][mgt_category_id];
 4245:                     if (bm->RBCestimation.RBCspeciesParam[sp][mgt_category_id] != bm->RBCestimation.RBCspeciesParam[ind_sp][mgt_category_id]) {
 4246:                         diff = bm->RBCestimation.RBCspeciesParam[sp][CPUEtrend_id] - bm->RBCestimation.RBCspeciesParam[ind_sp][CPUEtrend_id];
 4247:                         if (diff < closest_slopeB) {
 4248:                             ref_spB = ind_sp;
 4249:                         }
 4250:                         if (diff < buffer) {
 4251:                             bm->RBCestimation.count_sp_in_buffer[that_category]++;
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
**Lines (current code):** 4246  
**Lines at review (Oct 2026):** 3967

**Function:** `Check_Indicator_Links()`

**Why it's thought to be a bug**

The other-category diff is missing `fabs()` (the same-category loop at 3949 has it), so any species with a more negative trend counts as "closest". Function: `Check_Indicator_Links()`.

**Current code** (`atmanage/atManageTier.c` lines 4244–4248)

```c
 4244:                     that_category = bm->RBCestimation.RBCspeciesParam[ind_sp][mgt_category_id];
 4245:                     if (bm->RBCestimation.RBCspeciesParam[sp][mgt_category_id] != bm->RBCestimation.RBCspeciesParam[ind_sp][mgt_category_id]) {
 4246:                         diff = bm->RBCestimation.RBCspeciesParam[sp][CPUEtrend_id] - bm->RBCestimation.RBCspeciesParam[ind_sp][CPUEtrend_id];
 4247:                         if (diff < closest_slopeB) {
 4248:                             ref_spB = ind_sp;
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
**Lines (current code):** 4266, 4340  
**Lines at review (Oct 2026):** 3987, 4061

**Function:** `Check_Indicator_Links()`

**Why it's thought to be a bug**

`maxTier = mgt_category_id;` defaults to the enum index constant and not the species' current category (`my_category`). When no species falls inside the buffer, the category is set to an arbitrary number. Function: `Check_Indicator_Links()`.

**Current code** (`atmanage/atManageTier.c` lines 4264–4268)

```c
 4264:                 }
 4265:                 
 4266:                 maxTier = mgt_category_id;
 4267:                 maxCount = 0;
 4268:                 for (itier = 0; itier < K_num_indicator_tiers; itier++ ){
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
**Lines (current code):** 5235, ≈5240-5248  
**Lines at review (Oct 2026):** 4156, 4164-4172

**Function:** `RDynamicTier4Assessment()`

**Why it's thought to be a bug**

`chdir(dirname)` never returns to the parent directory, so later relative file I/O and next year's mkdir end up nested. `Util_Close_Output_File(bm->Dyn4File)` closes the file but leaves the pointer non-NULL (atUtil.c:200), so on the next call `if(!bm->Dyn4File)` is false and the code writes to a closed FILE*. Function: `RDynamicTier4Assessment()`.

**Current code** (`atmanage/atManageTier.c` lines 5233–5250) — ≈ the code here has changed since the review; lines are approximate

```c
 5233:     /* Change directory into the new directory */
 5234:     //pass your path in the function
 5235:     ret = chdir(dirname);
 5236:     /*if the change of directory was successful it will print successful otherwise it will print not successful*/
 5237:     if (ret < 0)
 5238:         quit("runDynamicTier4Assessment: chdir change of directory to %s not successful\n", dirname);
 5239:     else
 5240:         printf("runDynamicTier4Assessment: chdir change of directory to %s successful", dirname);
 5241:     
 5242:     /* Create and run bat files - Nov 2026: new files each time as each assessment is in its own directory
 5243:        (previously the file pointers were closed but not reset so later assessments wrote to closed files) */
 5244:     bm->Dyn4File = initDynTier4File(bm);
 5245:     bm->Dyn4CTLFile = initDynTier4CTLFile(bm);
 5246:     writeDynTier4inputfile(bm, species, year, bm->Dyn4File, bm->Dyn4CTLFile);
 5247:     Util_Close_Output_File(bm->Dyn4File);
 5248:     Util_Close_Output_File(bm->Dyn4CTLFile);
 5249:     bm->Dyn4File = NULL;
 5250:     bm->Dyn4CTLFile = NULL;
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
**Lines (current code):** 5376-5381  
**Lines at review (Oct 2026):** 4280-4282

**Function:** `writeDynTier4inputfile()`

**Why it's thought to be a bug**

The third block under the "Year, Catch and CPUE" header writes `CatchData` a second time where CPUE (`CPUEgen`) is expected, so the R assessment receives catch as CPUE. Function: `writeDynTier4inputfile()`.

**Current code** (`atmanage/atManageTier.c` lines 5374–5383)

```c
 5374: 
 5375:         // CPUE by fleet (all-region value), -1 = missing or fleet not used
 5376:         for (nf = 0; nf < Nfleets; nf++) {
 5377:             cpue = bm->RBCestimation.RBCspeciesArray[sp].CPUEgen[nf][sumregion][iy];
 5378:             if ((cpue <= 0.0) || ((CPUEfleet >= 0) && (CPUEfleet != nf)))
 5379:                 cpue = -1.0;
 5380:             fprintf(datfid,"%e ", cpue);
 5381:         }
 5382:         fprintf(datfid, "\n");
 5383:     }
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
**Lines (current code):** ≈5364  
**Lines at review (Oct 2026):** 4269

**Function:** `writeDynTier4inputfile()`

**Why it's thought to be a bug**

`t` is never incremented, so `t <= HistYrMax` is always true and the retro-catch adjustment applies to all years. Function: `writeDynTier4inputfile()`.

**Current code** (`atmanage/atManageTier.c` lines 5362–5366) — ⚠ the code quoted in the review is not at these lines any more (changed, or already fixed?)

```c
 5362:         fprintf(datfid,  "%d ", iy);
 5363: 
 5364:         // Removals by fleet
 5365:         for (nf = 0; nf < Nfleets; nf++) {
 5366:             catchy = bm->RBCestimation.RBCspeciesArray[sp].CatchData[nf][sumregion][iy];
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
**Lines (current code):** 5576  
**Lines at review (Oct 2026):** 4456

**Function:** `setRBC()`

**Why it's thought to be a bug**

`...= 0.0; // temporary value ...        bm->RBCestimation.RBCspeciesParam[species][EstB0_id] = 999.9;` puts the EstB0 assignment inside the comment, so EstB0 is not flagged on failure and EstDepletion (5587) mixes a valid B0 with Bcurr=999.9. Function: `setRBC()`.

**Current code** (`atmanage/atManageTier.c` lines 5574–5578)

```c
 5574: 
 5575:         bm->RBCestimation.RBCspeciesParam[species][AssessFail_id] = TRUE;
 5576:         bm->RBCestimation.RBCspeciesArray[species].RBC_by_year[year] = 0.0;               // temporary value, will be reset in DoAssessment        bm->RBCestimation.RBCspeciesParam[species][EstB0_id] = 999.9;
 5577:         bm->RBCestimation.RBCspeciesParam[species][EstBinit_id] = 999.9;
 5578:         bm->RBCestimation.RBCspeciesParam[species][EstBcurr_id] = 999.9;
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
**Lines (current code):** ≈5613-5620  
**Lines at review (Oct 2026):** 4481-4490

**Function:** `Averaged_RBC()`

**Why it's thought to be a bug**

The loop `yr < mgt_rbcYear + n - 1` sums n-1 RBCs but divides by n. The replacement loop writes `RBC_by_year[year]` repeatedly and not `[yr]`. Function: `Averaged_RBC()`.

**Current code** (`atmanage/atManageTier.c` lines 5611–5622) — ≈ the code here has changed since the review; lines are approximate

```c
 5611:     // Calculate the average RBC
 5612:     for (yr = startyr; yr < year + 1; yr++) {
 5613:         AveRBC += bm->RBCestimation.RBCspeciesArray[species].RBC_by_year[yr];
 5614:         nyr++;
 5615:     }
 5616:     if (nyr > 0)
 5617:         AveRBC /= (double)nyr;
 5618: 
 5619:     // Replace the RBC
 5620:     bm->RBCestimation.RBCspeciesArray[species].RBC_by_year[year] = AveRBC;
 5621:     bm->RBCestimation.RBCspeciesParam[species][RBCest_id] = AveRBC;
 5622: 
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
**Lines (current code):** ≈3409  
**Lines at review (Oct 2026):** 3243

**Function:** `Evaluation()`

**Why it's thought to be a bug**

`tiertype = FunctGroupArray[species].speciesParams[tiertype_id]`: tiertype_id belongs to the RBC_SPECIES_PARAMS enum (atlantisboxmodel.h:2620/2706). DoTierAssessment reads `RBCspeciesParam[species][tiertype_id]`, so the tier5 log branch tests an unrelated parameter. Function: `Evaluation()`.

**Current code** (`atmanage/atManageTier.c` lines 3407–3411) — ≈ the code here has changed since the review; lines are approximate

```c
 3407: 	double sb0;
 3408: 	int tier = (int) (FunctGroupArray[species].speciesParams[tier_id]);
 3409: 	int tiertype = (int) (bm->RBCestimation.RBCspeciesParam[species][tiertype_id]);   // Nov 2026: was read from speciesParams
 3410: 	double step1;
 3411: 	int fleetid = (int)(bm->RBCestimation.RBCspeciesParam[species][CPUEfleet_id]);
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
**Lines (current code):** 3422-3424, 3458  
**Lines at review (Oct 2026):** 3256-3258, 3292

**Function:** `Evaluation()`

**Why it's thought to be a bug**

`sb0 = SBioZero[0][0]` is used with `SpawnBio[Nstocks][Nregions]` (elsewhere the overall value is at [Nstocks][Nregions]). `step1 = CPUEgen[fleetid][r][year]` is computed with the stale `r` (=Nregions) outside the loop and printed for every region. Function: `Evaluation()`.

**Current code** (`atmanage/atManageTier.c` lines 3420–3459 (truncated))

```c
 3420: 
 3421: 	if (bm->RBCestimation.RBCspeciesParam[species][Regime_year_id] > 0)
 3422: 		sb0 = bm->RBCestimation.RBCspeciesArray[species].SBioZero_shift[0][0];
 3423: 	else
 3424: 		sb0 = bm->RBCestimation.RBCspeciesArray[species].SBioZero[0][0];
 3425: 
 3426: 	switch (tier){
 3427: 	case tier0: /* ----------- No future catches ---------- */
 3428: 		break;
 3429: 	case tier1: /* ----------- Tier 1 --------------------- */
 3430: 		fprintf(llogfp, "Time: %e %s year: %d MaxConvergCrit: %e sb0: %e, EstB0: %e, SpawnBio: %e, EstBcurr: %e, relSpawnBio: %e, EstDepletion: %e, SaveIseedx: %d, SaveIseedz: %d\n",
 3431: 			bm->dayt, FunctGroupArray[species].groupCode, year,
 3432: 			bm->RBCestimation.RBCspeciesParam[species][MaxConvergCrit_id], sb0,
 3433: 			bm->RBCestimation.RBCspeciesParam[species][EstB0_id],
 3434: 			bm->RBCestimation.RBCspeciesArray[species].SpawnBio[Nstocks][Nregions][year],
 3435: 			bm->RBCestimation.RBCspeciesParam[species][EstBcurr_id],
 3436: 			bm->RBCestimation.RBCspeciesArray[species].SpawnBio[Nstocks][Nregions][year] / sb0,
 3437: 			bm->RBCestimation.RBCspeciesParam[species][EstDepletion_id],
 3438: 			bm->RBCestimation.SaveIseedx, bm->RBCestimation.SaveIseedz);
 3439: 		break;
 3440:     case tier2: /* ------------ Tier 2 --------------------- */
 3441:         break;
 3442: 	case tier3: /* ------------ Tier 3 --------------------- */
 3443: 		fprintf(llogfp, "Time: %e %s year: %d F20: %e F40: %e F48: %e Fcurr: %e Tier3_mult: %e MinCyr: %e MaxCyr: %e\n",
 3444: 				bm->dayt, FunctGroupArray[species].groupCode, year,
 3445: 				bm->RBCestimation.RBCspeciesParam[species][F20_id], bm->RBCestimation.RBCspeciesParam[species][F40_id],
 3446: 				bm->RBCestimation.RBCspeciesParam[species][F48_id], bm->RBCestimation.RBCspeciesParam[species][Fcurr_id],
 3447: 				bm->RBCestimation.RBCspeciesParam[species][Tier3_mult_id],
 3448: 				bm->RBCestimation.RBCspeciesParam[species][MinCyr_id], bm->RBCestimation.RBCspeciesParam[species][MaxCyr_id]);
 3449: 		break;
 3450: 	case tier4: /* ------------ Tier 4 --------------------- */
 3451: 		fprintf(llogfp, "Time: %e %s year: %d CPUEfleet: %e CPUElim: %e CPUEtarg: %e Cmax: %e CPUEav: %e Cmult: %e CCcur: %e RBC: %e\n",
 3452: 				bm->dayt, FunctGroupArray[species].groupCode, year,
 3453: 				bm->RBCestimation.RBCspeciesParam[species][CPUEfleet_id],
 3454: 				bm->RBCestimation.RBCspeciesParam[species][CPUElim_id], bm->RBCestimation.RBCspeciesParam[species][CPUEtarg_id],
 3455: 				bm->RBCestimation.RBCspeciesParam[species][Cmax_id], bm->RBCestimation.RBCspeciesParam[species][CPUEav_id],
 3456: 				bm->RBCestimation.RBCspeciesParam[species][Cmult_id], bm->RBCestimation.RBCspeciesParam[species][CCcur_id],
 3457: 				bm->RBCestimation.RBCspeciesParam[species][RBCest_id]);
 3458: 		step1 = bm->RBCestimation.RBCspeciesArray[species].CPUEgen[fleetid][r][year];
 3459: 		for (r=0; r < Nregions; r++)
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
**Lines (current code):** 1866-1867, 2694-2695, 2745-2746  
**Lines at review (Oct 2026):** 1724-1725, 2528-2529, 2579-2580

**Why it's thought to be a bug**

Hard-coded `if (bm->dayt > 10584) printf(...)` debug prints left in live code. Functions: `newTier3Rule()`, `TierFive_AvLen()`.

**Current code** (`atmanage/atManageTier.c` lines 1864–1869)

```c
 1864: 	//int do_debug = 0;
 1865: 
 1866:     if (bm->dayt > 10584)
 1867:         printf("Applying final step of calc\n");
 1868: 
 1869:     /*
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
