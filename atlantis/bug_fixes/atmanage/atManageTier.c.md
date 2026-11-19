# atmanage/atManageTier.c — bug review

Generated 2026-11-19 from `atlantis/to-do-fixt-list.md`. **56 findings** (severity H: 24; confidence H and severity H: 23; on the priority list: 23), plus 8 lower-confidence notes.

Line numbers in **Listed lines** are from the review (10–12 Oct 2026). The code shown is from the working tree on 2026-11-19. Where the code has moved, the snippet follows it and says so. ⚠ means the quoted code could not be found: it may have been changed or already fixed.

Conf/Sev: confidence it's a bug / severity if it is (H = wrong results or crash, M = wrong in some configs, L = cosmetic, log, leak or latent). ✔ = the reviewer re-checked it by hand. ⭐ = on the high-priority list.

## Summary

| ID | Listed lines | Conf/Sev | Category | Short description | Decision | Done |
|---|---|---|---|---|---|---|
| [TIER-01](#tier-01) ⭐ | 487-492 | H/H | wrong operator / sign | `tac_change = (TAC_old - TACpost)/TAC_old; if (tac_change > inc_check) TAC = TAC_old + inc_check*TAC_old; else… |  | [ ] |
| [TIER-02](#tier-02) ⭐ | 501-503 | H/H | wrong variable | `if (TAC < minCatch) TAC = FunctGroupArray[species].speciesParams[MinCatch_id];` sets the TAC to the fraction … |  | [ ] |
| [TIER-03](#tier-03) ⭐ | 535-536 | H/H | wrong index / OOB | `if (bm->TACamt[species][nf][now_id] > no_quota) bm->TACamt[species][nf][now_id] = no_quota;` is inside the `n… |  | [ ] |
| [TIER-04](#tier-04) ⭐ | 394-399 | H/H | ordering / wrong value used | `RBC = RBCparam[RBCest_id];` is read *before* `Apply_RBC_Buffers()` changes RBCest_id. The TAC is then worked … |  | [ ] |
| [TIER-05](#tier-05) | 325-329 (also 1924-1927) | H/M | accumulator / loop | `for (nf...) for (nreg < Nregions) current_catch += Harvest_Get_TotCumCatch(species, nf, bm->thisyear);` The i… |  | [ ] |
| [TIER-06](#tier-06) | 376-384 | M/M | switch fall-through | `case tier8: case tier9: case tier13: RBC = 0.0;` has no break, so it falls into `default` and quits with "No … |  | [ ] |
| [TIER-07](#tier-07) ⭐ | 3539-3549 | H/H | switch fall-through / wrong source | `case sp_rollover:` has no `break`, so it falls through to `default` and calls `quit("")`. It is also unreacha… |  | [ ] |
| [TIER-08](#tier-08) | 702-712 | M/M | off-by-one / sanity logic | `if (MinCyr > MaxCyr) MaxCyr = MinCyr;` can push MaxCyr past `year` (future, unfilled years), so it should pro… |  | [ ] |
| [TIER-09](#tier-09) | 718-743 | M/M | accumulator not reset | `Util_Init_1D_Double(discrate, nyears, 0.0)` zeroes indices 0..nyears-1 (the number of averaging years), not t… |  | [ ] |
| [TIER-10](#tier-10) | 721, 800-803 | M/M | uninitialised data / stale variable | `DiscType = ...DiscardType[nf]`: DiscardType is allocated with size nLen (atAssessTierSetup.c:130), is indexed… |  | [ ] |
| [TIER-11](#tier-11) | 644 | M/M | wrong divisor | `avprop_reg[nf][nreg] /= num_avg_year;` but the sum covers `startyear..year` (5 years, or fewer near the start… |  | [ ] |
| [TIER-12](#tier-12) | 997-1022 | M/M | cohort vs age / double counting | `natm = Mnat[0][0][s][a][index2]` uses `a` (a cohort×genotype index, up to numCohortsXnumGenes) as the *age-in… |  | [ ] |
| [TIER-13](#tier-13) ⭐ | 1131, 1147 | H/H | off-by-one / heap overflow | `for (ia = 0; ia <= this_nselage; ia++) ... propgt05[ia] = 1;` reads `numatage[nselage]` and writes `propgt05[… |  | [ ] |
| [TIER-14](#tier-14) | 1121-1126 | M/M | 1-based leftover / dead code | `if ((a == AgeSel95+1) && (numatage[1] < numatage[2]))`: at this point `numatage[2]` has not been filled yet (… |  | [ ] |
| [TIER-15](#tier-15) ⭐ | 1173-1176 | H/H | off-by-one | `nz` is the index of the last stored Z (it starts at -1 and is incremented before use), but `for (i=0; i<nz; i… |  | [ ] |
| [TIER-16](#tier-16) | 1345, 1373 | H/M | uninitialised / not reset | `NCA` is initialised to 0 at its declaration and only updated when a year has data. It is not reset per fleet … |  | [ ] |
| [TIER-17](#tier-17) ⭐ | 1435 | H/H | copy-paste / wrong index | Restart branch: `fvalF[nF] = UnTransform(simplex[1][1], 0.01);` takes the S95 parameter of the second vertex. … |  | [ ] |
| [TIER-18](#tier-18) ⭐ | 1459, 1472, 1478, 1482 | H/H | off-by-one | `nF` is the index of the last fitted fleet (it starts at -1, then ++), but the loops `i < nF` and the divisor … |  | [ ] |
| [TIER-19](#tier-19) ⭐ | 1614 | H/H | off-by-one | `for (iy=0; iy < Ncage_id; iy++)`: Ncage_id is the last index (it starts at -1), so the last year of catch-at-… |  | [ ] |
| [TIER-20](#tier-20) | 1601-1604 | M/M | unreachable code | The plus-group branch `else if (a == Tier3_maxage)` can never run inside `for (a < Tier3_maxage)`, so the plus… |  | [ ] |
| [TIER-21](#tier-21) ⭐ | 1737-1740 (and 2726-2729) | H/H | wrong formula | `step1 = (F40-F20)*(Fcurr-F20); frbc = F48 / step1;` is dimensionally wrong (1/F). The linear ramp between F40… |  | [ ] |
| [TIER-22](#tier-22) | 1812-1841 | L/L | off-by-one | `fval` is updated at the *end* of the loop as `i/1000.0`, so iterations 0 and 1 both use F=0 and every later F… |  | [ ] |
| [TIER-23](#tier-23) | 1868-1886 | M/M | cohort vs age | YPR loops over `numCohorts`, but the `wt/mat/sel` arrays it is given are indexed by age in years (filled up to… |  | [ ] |
| [TIER-24](#tier-24) | 1971-1972 | H/M | 1-based leftover / OOB | `maxprop = avprop[1]; maxfleet = 1;` reads out of bounds when Nfleets==1 (avprop has size Nfleets) and default… |  | [ ] |
| [TIER-25](#tier-25) | 2052, 2062 | M/M | off-by-one | The CPUE target loop `iy < MaxCyr` excludes the last year, but the divisor `step1 = yrmax - yrmin + 1` counts … |  | [ ] |
| [TIER-26](#tier-26) | 2078-2095 | H/M | off-by-one | `startyear = year - m; for (iy=startyear; iy<=year; ...)` sums m+1 years but divides by m (or by `year-startye… |  | [ ] |
| [TIER-27](#tier-27) ⭐ | 2291, 2295, 2388 | H/H | copy-paste / wrong index | `k = simplex[0][0]; xpar[1] = simplex[0][0];` and in the objective `in_K = xpar[0];`. K is parameter 1 (commen… |  | [ ] |
| [TIER-28](#tier-28) ⭐ | 2397-2398 | H/H | misplaced parenthesis | `step1 = (1 - (bt/K)^p) - T5_removal[iy-1]; B = bt + bt*r/p*step1;` multiplies the catch removal by `bt*r/p`. … |  | [ ] |
| [TIER-29](#tier-29) | 2430-2438 | M/H | off-by-one / uninitialised read | The projection loop `iy < index2` runs zero times for AssessDelay 0 and only to CurrentYear for delay 1. Then … |  | [ ] |
| [TIER-30](#tier-30) | 2475-2480 | H/M | off-by-one / heap overflow | `do { iy++; ... TSbiomass[iy] = ... } while (... && iy < 1000.0);` can write `TSbiomass[1000]`, but the array … |  | [ ] |
| [TIER-31](#tier-31) | 2583-2589 | H/L | off-by-one / OOB read | `do { i=i+1; } while ((avlen < T5_Avlen_atF[i]) && (i < nfint));` evaluates `T5_Avlen_atF[nfint]` (array size … |  | [ ] |
| [TIER-32](#tier-32) ⭐ | 2639-2649 | H/H | accumulator reset in wrong loop | `for (iy...) { sumcatch = 0.0; ... }` resets the sum every year, so DCAC uses only the final year's catch and … |  | [ ] |
| [TIER-33](#tier-33) | 2768-2774 | H/M | accumulator not reset | `localcatch` is not reset per year, so `HistCatch` ends up as cumulative total catch rather than the peak annu… |  | [ ] |
| [TIER-34](#tier-34) | 2782-2794 | M/M | wrong variable reset | `RBCspeciesParam[species][TriggerReached_id] = 0;` is reset per region, but the array `TriggerReached[nreg]` (… |  | [ ] |
| [TIER-35](#tier-35) ⭐ | 2826-2827, 2835, 2848, 2851, 2863-2867 | H/H | swapped arguments | `DoCPUE(bm, year, species, llogfp)`, `DoPrager(bm, year, species, ...)`, `Call_Tier3(bm, year, species, ...)` … |  | [ ] |
| [TIER-36](#tier-36) ⭐ | 2938-2950 | H/H | wrong operator / logic | `for (iy = year-1; iy >= 0; iy++)` counts *up* without bound (runs off the end of EffortData). Also `j = iy` i… |  | [ ] |
| [TIER-37](#tier-37) ⭐ | 2963 | H/H | off-by-one / heap overflow | `AnnCPUE[5-iy]` with iy going down to 0 writes `AnnCPUE[5]`, but AnnCPUE is allocated with 5 entries (atAssess… |  | [ ] |
| [TIER-38](#tier-38) ⭐ | 3044, 3078, 3097, 3183, 3196, 3211 | H/H | wrong index | `cosp = FunctGroupArray[sp].co_sp[sp];` uses the loop counter as the group index. It should be the focal speci… |  | [ ] |
| [TIER-39](#tier-39) ⭐ | 3146-3147 | H/H | stale index / OOB | `if (bm->SP_FISHERYprms[cosp][nf][q_id] > 0.0) toteffort += bm->CumEffort[nf][b];` sits after the nf loops hav… |  | [ ] |
| [TIER-40](#tier-40) | 3124-3156 | M/M | accumulator not reset | `areafished` and `areasp` (and `toteffort`) are initialised once at function entry and not per co-species, so … |  | [ ] |
| [TIER-41](#tier-41) ⭐ | 3216 | H/H | swapped args / wrong species | `Call_Tier3(bm, year, species, llogfp)` has its arguments swapped, and inside the loop over cosp it should ass… |  | [ ] |
| [TIER-42](#tier-42) | 3718-3736 | M/M | accumulator not reset / off-by-one | `catchwghtCPUE` is zeroed once outside the `lag` loop, so every lag adds into the same slots. The regression f… |  | [ ] |
| [TIER-43](#tier-43) | 3787 | M/M | wrong variable | `(FunctGroupArray[sp].isImpacted == FALSE)` tests the target species and not the candidate indicator `i`. Func… |  | [ ] |
| [TIER-44](#tier-44) | 3877-3910, 3899 | M/M | wrong constant / accumulator | The `yy` offset uses `GradientPeriod` (the lag count) and not `GradientWindow` (the array length). Indices can… |  | [ ] |
| [TIER-45](#tier-45) | 3927-3929 | H/M | copy-paste | The else-branch sets `CPUEtrend_id = 0` where it should set `CPUEdir_id = 0`, so the direction stays stale and… |  | [ ] |
| [TIER-46](#tier-46) ⭐ | 3950-3952, 3968-3970, 4022-4024, 4042-4044 | H/H | missing update | `if (diff < closest_slope) ref_sp = ind_sp;` never updates `closest_slope` (or `closest_slopeB`), so ref_sp en… |  | [ ] |
| [TIER-47](#tier-47) | 3967 | H/M | inconsistency | The other-category diff is missing `fabs()` (the same-category loop at 3949 has it), so any species with a mor… |  | [ ] |
| [TIER-48](#tier-48) | 3987, 4061 | H/M | wrong variable | `maxTier = mgt_category_id;` defaults to the enum index constant and not the species' current category (`my_ca… |  | [ ] |
| [TIER-49](#tier-49) ⭐ | 4156, 4164-4172 | H/H | state not reset / use-after-close | `chdir(dirname)` never returns to the parent directory, so later relative file I/O and next year's mkdir end u… |  | [ ] |
| [TIER-50](#tier-50) ⭐ | 4280-4282 | H/H | copy-paste | The third block under the "Year, Catch and CPUE" header writes `CatchData` a second time where CPUE (`CPUEgen`… |  | [ ] |
| [TIER-51](#tier-51) | 4269 | L/M | uninitialised counter | `t` is never incremented, so `t <= HistYrMax` is always true and the retro-catch adjustment applies to all yea… |  | [ ] |
| [TIER-52](#tier-52) | 4456 | H/L | statement inside comment | `...= 0.0; // temporary value ... bm->RBCestimation.RBCspeciesParam[species][EstB0_id] = 999.9;` puts the EstB… |  | [ ] |
| [TIER-53](#tier-53) | 4481-4490 | H/M | off-by-one / wrong index | The loop `yr < mgt_rbcYear + n - 1` sums n-1 RBCs but divides by n. The replacement loop writes `RBC_by_year[y… |  | [ ] |
| [TIER-54](#tier-54) | 3243 | M/L | wrong array | `tiertype = FunctGroupArray[species].speciesParams[tiertype_id]`: tiertype_id belongs to the RBC_SPECIES_PARAM… |  | [ ] |
| [TIER-55](#tier-55) | 3256-3258, 3292 | M/L | wrong index (log only) | `sb0 = SBioZero[0][0]` is used with `SpawnBio[Nstocks][Nregions]` (elsewhere the overall value is at [Nstocks]… |  | [ ] |
| [TIER-56](#tier-56) | 1724-1725, 2528-2529, 2579-2580 | H/L | debug leftover | Hard-coded `if (bm->dayt > 10584) printf(...)` debug prints left in live code. Functions: `newTier3Rule()`, `T… |  | [ ] |

---

## TIER-01

**wrong operator / sign** · Conf H / Sev H ⭐ priority  
**Listed lines:** 487-492

**Function:** `DoTierAssessment()`

**Why it's thought to be a bug**

`tac_change = (TAC_old - TACpost)/TAC_old; if (tac_change > inc_check) TAC = TAC_old + inc_check*TAC_old; else if (tac_change < bm->prop_decTAC) TAC = TAC_old - dec_check*TAC_old;` The sign is reversed: a big proposed *decrease* gives a positive tac_change, which then *raises* the TAC to the max. The second test compares against the raw multiplier `prop_decTAC` (e.g. 0.5) and not `-dec_check`, so almost every other case is pushed to the minimum TAC. prop_inc/decTAC are multipliers (see atManageAnnual.c:849-850). Also divides by zero when TAC_old==0. Function: `DoTierAssessment()`.

**Current code** (`atmanage/atManageTier.c` lines 575–584) — code appears to have moved +90 lines since the review

```c
  575: 				tac_change = (bm->RBCestimation.RBCspeciesParam[species][TACpost_id] - TAC_old) / TAC_old;
  576: 
  577: 				if (tac_change > inc_check)
  578: 					TAC = TAC_old * (1.0 + inc_check);
  579: 				else if (tac_change < -dec_check)
  580: 					TAC = TAC_old * (1.0 - dec_check);
  581: 				else
  582: 					TAC = bm->RBCestimation.RBCspeciesParam[species][TACpost_id];
  583: 			} else {
  584: 				tac_change = 0.0;
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
**Listed lines:** 501-503

**Function:** `DoTierAssessment()`

**Why it's thought to be a bug**

`if (TAC < minCatch) TAC = FunctGroupArray[species].speciesParams[MinCatch_id];` sets the TAC to the fraction (e.g. 0.05) and not to the computed `minCatch` (fraction × last historical catch). Function: `DoTierAssessment()`.

**Current code** (`atmanage/atManageTier.c` lines 499–505)

```c
  499: 		//  those used in previous year
  500: 		// TODO: Ask Sally if AssessFail case should only apply to Tier1 as the note says
  501: 		if (bm->RBCestimation.RBCspeciesParam[species][AssessFail_id]) {
  502: 			RBC = 0.0;
  503: 			TAC = 0.0;
  504: 			bm->RBCestimation.RBCspeciesParam[species][TACpost_id] = 0.0;
  505: 			for (nf = 0; nf < bm->K_num_fisheries; nf++) {
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
**Listed lines:** 535-536

**Function:** `DoTierAssessment()`

**Why it's thought to be a bug**

`if (bm->TACamt[species][nf][now_id] > no_quota) bm->TACamt[species][nf][now_id] = no_quota;` is inside the `nfishery` loop but uses the stale `nf`, which equals K_num_fisheries (or Nfleets) left over from earlier loops. That is an out-of-bounds read/write on TACamt[species], and the cap is never applied to the fishery actually being updated. Function: `DoTierAssessment()`.

**Current code** (`atmanage/atManageTier.c` lines 533–538)

```c
  533: 				maxprop = 0;
  534: 				CPUEfleet = 0;
  535: 				for (nf = 0; nf < Nfleets; nf++){
  536: 					if (bm->RBCestimation.RBCspeciesArray[species].avprop[nf] > maxprop) {
  537: 						maxprop = bm->RBCestimation.RBCspeciesArray[species].avprop[nf];
  538: 						CPUEfleet = nf;
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
**Listed lines:** 394-399

**Function:** `DoTierAssessment()`

**Why it's thought to be a bug**

`RBC = RBCparam[RBCest_id];` is read *before* `Apply_RBC_Buffers()` changes RBCest_id. The TAC is then worked out from the local, unbuffered `RBC` (lines 442/444/527), so tier buffers have no effect on the TAC. Function: `DoTierAssessment()`.

**Current code** (`atmanage/atManageTier.c` lines 397–406) — code appears to have moved +5 lines since the review

```c
  397:            If there has been no successful assessment yet, carry forward last year's RBC. */
  398:         if (bm->RBCestimation.RBCspeciesArray[species].lastAssessRBC > 0.0)
  399:             bm->RBCestimation.RBCspeciesParam[species][RBCest_id] = bm->RBCestimation.RBCspeciesArray[species].lastAssessRBC;
  400:         else if (year > 0)
  401:             bm->RBCestimation.RBCspeciesParam[species][RBCest_id] = bm->RBCestimation.RBCspeciesArray[species].RBC_by_year[year - 1];
  402:         else
  403:             bm->RBCestimation.RBCspeciesParam[species][RBCest_id] = TAC_old;
  404:         break;
  405:     case tier8:  // US method
  406:     case tier9:  // Norwegian method
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
**Listed lines:** 325-329 (also 1924-1927)

**Why it's thought to be a bug**

`for (nf...) for (nreg < Nregions) current_catch += Harvest_Get_TotCumCatch(species, nf, bm->thisyear);` The inner region loop does not depend on nreg, so the total catch is multiplied by Nregions. Functions: `DoTierAssessment()`, `Call_Tier3()`.

**Current code** (`atmanage/atManageTier.c` lines 323–331)

```c
  323: 	case tier3: /* ------------ Tier 3 --------------------- */
  324: 		bm->RBCestimation.RBCspeciesParam[species][True_F_id] = TierThree_trueF(bm, species, year, AgeSel95, llogfp);
  325: 		bm->RBCestimation.RBCspeciesParam[species][True_F48_id] = bm->RBCestimation.RBCspeciesParam[species][F48_id];
  326: 		//-------- calculate current F  (global Fcurr)
  327: 
  328: 		switch ((int)(bm->RBCestimation.RBCspeciesParam[species][Tier3_Fcalc_id])){
  329: 		case 1: // ----- catch curves (old T3)
  330: 			if ( AgeSel95 > (FunctGroupArray[species].numCohorts-1)) {
  331: 				quit("Catch curves will not work for this species because age at full selectivity > maximum age\n");
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
**Listed lines:** 376-384

**Function:** `DoTierAssessment()`

**Why it's thought to be a bug**

`case tier8: case tier9: case tier13: RBC = 0.0;` has no break, so it falls into `default` and quits with "No such tier". Function: `DoTierAssessment()`.

**Current code** (`atmanage/atManageTier.c` lines 403–415) — code appears to have moved +29 lines since the review

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
**Listed lines:** 3539-3549

**Function:** `Apply_RBC_Buffers()`

**Why it's thought to be a bug**

`case sp_rollover:` has no `break`, so it falls through to `default` and calls `quit("")`. It is also unreachable in practice because `tier` is re-read from `speciesParams[tier_id]` and not from the local `tier` that DoTierAssessment sets to sp_rollover (lines 267/277). As a result a rolled-over TAC gets the base tier's buffer (e.g. sig1) and not the MYTAC buffer. Function: `Apply_RBC_Buffers()`.

**Current code** (`atmanage/atManageTier.c` lines 3406–3420) — code appears to have moved -131 lines since the review

```c
 3406: 	int  r;
 3407: 	double sb0;
 3408: 	int tier = (int) (FunctGroupArray[species].speciesParams[tier_id]);
 3409: 	int tiertype = (int) (bm->RBCestimation.RBCspeciesParam[species][tiertype_id]);   // Nov 2026: was read from speciesParams
 3410: 	double step1;
 3411: 	int fleetid = (int)(bm->RBCestimation.RBCspeciesParam[species][CPUEfleet_id]);
 3412: 	int Nregions = (int)(bm->RBCestimation.RBCspeciesParam[species][NumRegions_id]);
 3413:     int Nstocks = FunctGroupArray[species].numStocks;
 3414: 
 3415:     //printf("Doing Evaluation for %s with tier %d\n", FunctGroupArray[species].groupCode, tier);
 3416:     
 3417:     for (r=0; r < Nregions; r++) {
 3418: 		WriteResults(bm, species, r, year, llogfp, fid);
 3419:     }
 3420: 
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
**Listed lines:** 702-712

**Function:** `Tier_averages()`

**Why it's thought to be a bug**

`if (MinCyr > MaxCyr) MaxCyr = MinCyr;` can push MaxCyr past `year` (future, unfilled years), so it should probably be `MinCyr = MaxCyr`. `nyrs` is computed *before* `MaxCyr` is bumped from 1 to 2, so the mean at 763-771 adds an extra year but divides by the old count. Function: `Tier_averages()`.

**Current code** (`atmanage/atManageTier.c` lines 700–714)

```c
  700: 	sumregion = Nregions;  // sum over regions
  701:     
  702: 	/* Sally deleted code snippet AA was here */
  703: 
  704: 	// work out proportion of catch taken by each fleet and region in each of last 5 years of data in the assessment
  705: 	// assessments don't know about regions, but need to know the proportion of catch by region
  706: 	// and fleet so can allocate TACs
  707:     startyear = year-4;
  708:     if (startyear < 0) startyear = 0;
  709: 	for (iy = startyear; iy < year+1; iy++) {
  710: 		sumcatch = 0.0;
  711: 		for (nreg = 0; nreg < Nregions ; nreg++){
  712: 			for (nf = 0; nf< Nfleets; nf++){
  713: 				//sumcatch += Harvest_Get_TotCumCatch(species, nf, iy);   Use this if want true catch and for loop is over K_num_fisheries
  714:                 sumcatch += bm->RBCestimation.RBCspeciesArray[species].CatchData[nf][nreg][iy];  // Use this for "pseudo-data" in MSE and summing oonly over assessment NumFisheries
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
**Listed lines:** 718-743

**Function:** `Tier_averages()`

**Why it's thought to be a bug**

`Util_Init_1D_Double(discrate, nyears, 0.0)` zeroes indices 0..nyears-1 (the number of averaging years), not the absolute years `year-3..year` that are used afterwards. Inside the fleet loop `discrate[iy]` is only overwritten when `DiscYears[nf][iy] > 0`, so fleets without discard data inherit the previous fleet's rate (or values from line 807 of the previous call). Function: `Tier_averages()`.

**Current code** (`atmanage/atManageTier.c` lines 684–713) — code appears to have moved -32 lines since the review

```c
  684: 	double nyrs;
  685: 
  686:     printf("Doing Tier_averages - initalise popcatch and avprop\n");
  687:     
  688:     Util_Init_2D_Double(bm->RBCestimation.RBCspeciesArray[species].propcatch, Nfleets, InitnYears, 0.0);
  689:     Util_Init_3D_Double(bm->RBCestimation.RBCspeciesArray[species].propcatch_reg, Nfleets, Nregions, InitnYears, 0.0);
  690:     Util_Init_1D_Double(bm->RBCestimation.RBCspeciesArray[species].avprop, Nfleets, 0.0);
  691:     Util_Init_2D_Double(bm->RBCestimation.RBCspeciesArray[species].avprop_reg, Nfleets, Nregions, 0.0);
  692:     
  693:     /*
  694:     int do_debug = 0;
  695: 	if((bm->debug == debug_quota) && (bm->which_check == species)){
  696: 		do_debug = 1;
  697: 	}
  698:      */
  699:     
  700: 	sumregion = Nregions;  // sum over regions
  701:     
  702: 	/* Sally deleted code snippet AA was here */
  703: 
  704: 	// work out proportion of catch taken by each fleet and region in each of last 5 years of data in the assessment
  705: 	// assessments don't know about regions, but need to know the proportion of catch by region
  706: 	// and fleet so can allocate TACs
  707:     startyear = year-4;
  708:     if (startyear < 0) startyear = 0;
  709: 	for (iy = startyear; iy < year+1; iy++) {
  710: 		sumcatch = 0.0;
  711: 		for (nreg = 0; nreg < Nregions ; nreg++){
  712: 			for (nf = 0; nf< Nfleets; nf++){
  713: 				//sumcatch += Harvest_Get_TotCumCatch(species, nf, iy);   Use this if want true catch and for loop is over K_num_fisheries
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
**Listed lines:** 721, 800-803

**Function:** `Tier_averages()`

**Why it's thought to be a bug**

`DiscType = ...DiscardType[nf]`: DiscardType is allocated with size nLen (atAssessTierSetup.c:130), is indexed by fleet, and is never filled anywhere in the repo, so it is always 0. Neither DiscType branch (1/2) ever runs, so discard rates are always 0, or NaN when there is no catch (807). The all-fleet block at 800-803 also uses the `DiscType` left over from the last fleet and not each fleet's own. Function: `Tier_averages()`.

**Current code** (`atmanage/atManageTier.c` lines 684–688) — code appears to have moved -35 lines since the review

```c
  684: 	double nyrs;
  685: 
  686:     printf("Doing Tier_averages - initalise popcatch and avprop\n");
  687:     
  688:     Util_Init_2D_Double(bm->RBCestimation.RBCspeciesArray[species].propcatch, Nfleets, InitnYears, 0.0);
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
**Listed lines:** 644

**Function:** `Tier_averages()`

**Why it's thought to be a bug**

`avprop_reg[nf][nreg] /= num_avg_year;` but the sum covers `startyear..year` (5 years, or fewer near the start). When num_avg_year != the number of years summed, avprop does not sum to 1. Function: `Tier_averages()`.

**Current code** (`atmanage/atManageTier.c` lines 659–663) — code appears to have moved +17 lines since the review

```c
  659: 
  660: /********************************************************************************************************
  661:  * \brief  Tier_averages
  662:  * Description: calculate average total catch  over last x (4) years for each fleet, sum over fleets
  663:  *              get average discard rate combined over all fleets for last x (4) years
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
**Listed lines:** 997-1022

**Function:** `TierOneMimic()`

**Why it's thought to be a bug**

`natm = Mnat[0][0][s][a][index2]` uses `a` (a cohort×genotype index, up to numCohortsXnumGenes) as the *age-in-years* index of Mnat. `num_individuals` and `bodysize` are also summed over all individuals regardless of sex, and then added once per sex, so the yield is doubled when K_num_sexes=2. Function: `TierOneMimic()`.

**Current code** (`atmanage/atManageTier.c` lines 995–1024)

```c
  995:     int hcr_option = (int)(bm->RBCestimation.RBCspeciesParam[species][Tier1_HCRoption_id]);
  996:     int Nregions = (int)(bm->RBCestimation.RBCspeciesParam[species][NumRegions_id]);
  997:     int Nstocks = FunctGroupArray[species].numStocks;
  998:     double totcount = 0.0;
  999:     double num_individuals = 0.0;
 1000:     double bodysize = 0.0;
 1001:     int s, a, ia, sn, rn, den, ij, k;
 1002:     int index1, index2;
 1003:    // int do_debug = 1;
 1004:     int maxage = (int)(bm->RBCestimation.RBCspeciesParam[species][MaxAge_id]);
 1005:     
 1006:     // So got meaningful data as assessing as go here
 1007:     if(bm->RBCestimation.AssessDelay > 1) {
 1008:         index1 = year+1;
 1009:         index2 = year+2;
 1010:     } else if (bm->RBCestimation.AssessDelay) {
 1011:         index1 = year+1;
 1012:         index2 = year+1;
 1013:     } else {
 1014:         index1 = year;
 1015:         index2 = year;
 1016:     }
 1017:     
 1018:     Util_Init_2D_Double(bm->RBCestimation.RBCspeciesArray[species].agesel_bysex, FunctGroupArray[species].numCohorts, bm->K_num_sexes, 0.0);
 1019:     Util_Init_2D_Double(bm->RBCestimation.RBCspeciesArray[species].fval, FunctGroupArray[species].numCohorts, bm->K_num_sexes, 0.0);
 1020: 
 1021:     Util_Init_1D_Double(bm->RBCestimation.RBCspeciesArray[species].agesel, maxage, 0.0);
 1022:     Util_Init_1D_Double(bm->RBCestimation.RBCspeciesArray[species].mat, maxage, 0.0);
 1023:     Util_Init_1D_Double(bm->RBCestimation.RBCspeciesArray[species].wt, maxage, 0.0);
 1024: 
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
**Listed lines:** 1131, 1147

**Function:** `TierThree_CC()`

**Why it's thought to be a bug**

`for (ia = 0; ia <= this_nselage; ia++) ... propgt05[ia] = 1;` reads `numatage[nselage]` and writes `propgt05[nselage]`. Both are allocated with size nselage (atAssessTierSetup.c:461-463). The do-while at 1147 likewise reads `propgt05[ia+1]` with ia+1 == this_nselage. This is left over from Sally's 1-based code. Function: `TierThree_CC()`.

**Current code** (`atmanage/atManageTier.c` lines 1200–1220) — code appears to have moved +71 lines since the review

```c
 1200: 	// int region;
 1201: 	double sumz, meanz = 0, step1, sp_a, sp_b;
 1202: 	int nselage, this_nselage;
 1203: 	double age, len, temp;
 1204: 	int Nfleets = (int)(bm->RBCestimation.RBCspeciesParam[species][NumFisheries_id]);
 1205: 	int AgeSel95 = (int)(bm->RBCestimation.RBCspeciesParam[species][AgeSel95_id]);
 1206:     int maxage = (int)(bm->RBCestimation.RBCspeciesParam[species][MaxAge_id]);
 1207: 
 1208:     /*
 1209:     int do_debug = 1;
 1210: 	if (do_debug)
 1211: 		fprintf(llogfp,"Tier 3 catch curve calculations for %s assessment in year %d\n", FunctGroupArray[species].groupCode, year);
 1212:      */
 1213:     
 1214:     // This is the method Neil uses - use growth and selectivity params read in
 1215: 	nselage = (int)(bm->RBCestimation.RBCspeciesParam[species][Tier3_maxage_id] - bm->RBCestimation.RBCspeciesParam[species][AgeSel95_id]);
 1216: 	//region = 1;
 1217:     
 1218:     Util_Init_1D_Double(bm->RBCestimation.RBCspeciesArray[species].agesel, maxage, 0.0);
 1219:     Util_Init_1D_Double(bm->RBCestimation.RBCspeciesArray[species].mat, maxage, 0.0);
 1220:     Util_Init_1D_Double(bm->RBCestimation.RBCspeciesArray[species].wt, maxage, 0.0);
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
**Listed lines:** 1121-1126

**Function:** `TierThree_CC()`

**Why it's thought to be a bug**

`if ((a == AgeSel95+1) && (numatage[1] < numatage[2]))`: at this point `numatage[2]` has not been filled yet (it is 0), so the test can never be true and the "drop first age if lower than next" logic never runs. In 0-based terms it should compare `numatage[0] < numatage[1]`. Function: `TierThree_CC()`.

**Current code** (`atmanage/atManageTier.c` lines 1185–1194) — code appears to have moved +66 lines since the review

```c
 1185: /******************************************************************************
 1186:  *
 1187: \brief:  TierThree_CC
 1188:  *      TierThree_CC:     estimate Fcurr using catch curves
 1189:  * Description: estimate Fcurr using catch curves (old Tier3)
 1190:  *
 1191:  *
 1192:  * called by : DoAssessment
 1193:  * calls :  getSlope, Tier3_averages, Tier3rule
 1194:  * created  : From code by Sally Wayte
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
**Listed lines:** 1173-1176

**Function:** `TierThree_CC()`

**Why it's thought to be a bug**

`nz` is the index of the last stored Z (it starts at -1 and is incremented before use), but `for (i=0; i<nz; i++)` and `meanz = sumz/nz` treat it as a count. The last Z is dropped, and a single valid Z (nz==0) is reported as an assessment failure. Function: `TierThree_CC()`.

**Current code** (`atmanage/atManageTier.c` lines 1185–1192) — code appears to have moved +14 lines since the review

```c
 1185: /******************************************************************************
 1186:  *
 1187: \brief:  TierThree_CC
 1188:  *      TierThree_CC:     estimate Fcurr using catch curves
 1189:  * Description: estimate Fcurr using catch curves (old Tier3)
 1190:  *
 1191:  *
 1192:  * called by : DoAssessment
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
**Listed lines:** 1345, 1373

**Function:** `TierThree_CCsel()`

**Why it's thought to be a bug**

`NCA` is initialised to 0 at its declaration and only updated when a year has data. It is not reset per fleet (only `Ncage_id` is), so a fleet with no data reuses the previous fleet's NCA (and the first fleet uses 0). `if (NCA >= 0)` is therefore always true and the minimisation runs on stale or empty Obsageprop. Function: `TierThree_CCsel()`.

**Current code** (`atmanage/atManageTier.c` lines 1389–1421) — code appears to have moved +46 lines since the review

```c
 1389: //******************************************************************************
 1390: //
 1391: // Name:  TierThree_CCsel
 1392: //      TierThree_CCsel:  calculate current F using Andre's YPR catch curve with selectivity
 1393: // Description: calculate current F using Andre's yield per recruit (new Tier 3)
 1394: //               catch curves with selectivity (now calling this Equilibrium F method)
 1395: //
 1396: //
 1397: // called by : DoAssessment
 1398: // calls :  downhill_simplex, Transform, UnTransform
 1399: // created  : April 2008 Sally
 1400: //
 1401: //******************************************************************************
 1402: void TierThree_CCsel(MSEBoxModel *bm, int species, int year, FILE *llogfp){
 1403: 
 1404: 	int s, a, iy, i, j, nF, nf, nFit, NCA = 0;
 1405: 	int iter = bm->RBCestimation.Tier_Max_Int;
 1406: 	double sumage, init_S95, init_S50, step1;
 1407: 	double ftol;
 1408: 	double sumF, sumchi;
 1409: 	double linf,kpar,t0;
 1410: 	double age,len;
 1411: 	double Tier3_M = bm->RBCestimation.RBCspeciesParam[species][Tier3_M_id];
 1412: 	double Tier3_h = bm->RBCestimation.RBCspeciesParam[species][Tier3_h_id];
 1413: 	double Tier3_a = bm->RBCestimation.RBCspeciesParam[species][Tier3_a_id];
 1414: 	double Tier3_b = bm->RBCestimation.RBCspeciesParam[species][Tier3_b_id];
 1415: 	double Tier3_F = bm->RBCestimation.RBCspeciesParam[species][Tier3_F_id];
 1416: 	double Tier3_matlen = bm->RBCestimation.RBCspeciesParam[species][Tier3_matlen_id];
 1417: 	double Tier3_S50 = bm->RBCestimation.RBCspeciesParam[species][Tier3_S50_id];
 1418: 	double Tier3_S95 = bm->RBCestimation.RBCspeciesParam[species][Tier3_S95_id];
 1419: 	double Nsexes = (double)(bm->K_num_sexes);
 1420: 	int CCsel_years = (int)bm->RBCestimation.RBCspeciesParam[species][CCsel_years_id];
 1421: 	int Nfleets = (int)(bm->RBCestimation.RBCspeciesParam[species][NumFisheries_id]);
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
**Listed lines:** 1435

**Function:** `TierThree_CCsel()`

**Why it's thought to be a bug**

Restart branch: `fvalF[nF] = UnTransform(simplex[1][1], 0.01);` takes the S95 parameter of the second vertex. The F estimate is `simplex[0][0]`, as used at line 1412. Function: `TierThree_CCsel()`.

**Current code** (`atmanage/atManageTier.c` lines 1440–1444) — code appears to have moved +7 lines since the review

```c
 1440:     Util_Init_2D_Double(bm->RBCestimation.RBCspeciesArray[species].sel, Nfleets, Tier3maxage, 0.0);
 1441:     Util_Init_1D_Double(bm->RBCestimation.RBCspeciesArray[species].chi, Nfleets, 0.0);
 1442:     Util_Init_1D_Double(bm->RBCestimation.RBCspeciesArray[species].fvalF, Nfleets, 0.0);
 1443:     
 1444:     npar = 3;
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
**Listed lines:** 1459, 1472, 1478, 1482

**Function:** `TierThree_CCsel()`

**Why it's thought to be a bug**

`nF` is the index of the last fitted fleet (it starts at -1, then ++), but the loops `i < nF` and the divisor `nF` treat it as a count. With one fleet (nF=0) Fcurr=0 and agesel is all zeros; with more fleets the last fleet is dropped. Function: `TierThree_CCsel()`.

**Current code** (`atmanage/atManageTier.c` lines 1400–1423) — code appears to have moved -57 lines since the review

```c
 1400: //
 1401: //******************************************************************************
 1402: void TierThree_CCsel(MSEBoxModel *bm, int species, int year, FILE *llogfp){
 1403: 
 1404: 	int s, a, iy, i, j, nF, nf, nFit, NCA = 0;
 1405: 	int iter = bm->RBCestimation.Tier_Max_Int;
 1406: 	double sumage, init_S95, init_S50, step1;
 1407: 	double ftol;
 1408: 	double sumF, sumchi;
 1409: 	double linf,kpar,t0;
 1410: 	double age,len;
 1411: 	double Tier3_M = bm->RBCestimation.RBCspeciesParam[species][Tier3_M_id];
 1412: 	double Tier3_h = bm->RBCestimation.RBCspeciesParam[species][Tier3_h_id];
 1413: 	double Tier3_a = bm->RBCestimation.RBCspeciesParam[species][Tier3_a_id];
 1414: 	double Tier3_b = bm->RBCestimation.RBCspeciesParam[species][Tier3_b_id];
 1415: 	double Tier3_F = bm->RBCestimation.RBCspeciesParam[species][Tier3_F_id];
 1416: 	double Tier3_matlen = bm->RBCestimation.RBCspeciesParam[species][Tier3_matlen_id];
 1417: 	double Tier3_S50 = bm->RBCestimation.RBCspeciesParam[species][Tier3_S50_id];
 1418: 	double Tier3_S95 = bm->RBCestimation.RBCspeciesParam[species][Tier3_S95_id];
 1419: 	double Nsexes = (double)(bm->K_num_sexes);
 1420: 	int CCsel_years = (int)bm->RBCestimation.RBCspeciesParam[species][CCsel_years_id];
 1421: 	int Nfleets = (int)(bm->RBCestimation.RBCspeciesParam[species][NumFisheries_id]);
 1422: 	int ok, startyear;
 1423:     int npar = 3;
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
**Listed lines:** 1614

**Function:** `EquilF_Funk()`

**Why it's thought to be a bug**

`for (iy=0; iy < Ncage_id; iy++)`: Ncage_id is the last index (it starts at -1), so the last year of catch-at-age is left out of the objective. With one year the objective is constant (ss=0), so the fit is meaningless. Function: `EquilF_Funk()`.

**Current code** (`atmanage/atManageTier.c` lines 1570–1574) — code appears to have moved -42 lines since the review

```c
 1570: 				}
 1571: 
 1572: 				downhill_simplex(bm, EquilF_Funk_model, species, bm->RBCestimation.RBCspeciesArray[species].simplex, bm->RBCestimation.RBCspeciesArray[species].func, npar, ftol, iter, &ok, llogfp, bm->RBCestimation.RBCspeciesArray[species].xpar);
 1573: 
 1574: 				bm->RBCestimation.RBCspeciesArray[species].fvalF[nF] = UnTransform(bm->RBCestimation.RBCspeciesArray[species].simplex[0][0], 0.01);   // Nov 2026: was simplex[1][1] (S95 of a non-best vertex)
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
**Listed lines:** 1601-1604

**Function:** `EquilF_Funk()`

**Why it's thought to be a bug**

The plus-group branch `else if (a == Tier3_maxage)` can never run inside `for (a < Tier3_maxage)`, so the plus group is never accumulated. Function: `EquilF_Funk()`.

**Current code** (`atmanage/atManageTier.c` lines 1621–1628) — code appears to have moved +22 lines since the review

```c
 1621: 		bm->RBCestimation.RBCspeciesArray[species].chi[i] = bm->RBCestimation.RBCspeciesArray[species].chi[i] / (sumchi + small_num);
 1622: 	}
 1623: 	for (a = 0; a < bm->RBCestimation.RBCspeciesParam[species][Tier3_maxage_id]; a++){
 1624: 		for (i = 0; i < nFit; i++){
 1625: 			bm->RBCestimation.RBCspeciesArray[species].agesel[a] += bm->RBCestimation.RBCspeciesArray[species].chi[i] * bm->RBCestimation.RBCspeciesArray[species].sel[i][a];
 1626:             
 1627:             /*
 1628:             if (do_debug){
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
**Listed lines:** 1737-1740 (and 2726-2729)

**Why it's thought to be a bug**

`step1 = (F40-F20)*(Fcurr-F20); frbc = F48 / step1;` is dimensionally wrong (1/F). The linear ramp between F40 and F20 should be `frbc = F48*(F20-Fcurr)/(F20-F40)`. The same code is copied into TierFiveSAFE. Functions: `newTier3Rule()`, `TierFiveSAFE()`.

**Current code** (`atmanage/atManageTier.c` lines 1859–1866) — code appears to have moved +124 lines since the review

```c
 1859: //
 1860: //******************************************************************************
 1861: void newTier3Rule(MSEBoxModel *bm, int species, int year, double current_catch, FILE *llogfp, FILE *fid)
 1862: {
 1863: 	double frbc, step1;
 1864: 	//int do_debug = 0;
 1865: 
 1866:     if (bm->dayt > 10584)
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
**Listed lines:** 1812-1841

**Function:** `getF20_40_48()`

**Why it's thought to be a bug**

`fval` is updated at the *end* of the loop as `i/1000.0`, so iterations 0 and 1 both use F=0 and every later F lags by 0.001. Function: `getF20_40_48()`.

**Current code** (`atmanage/atManageTier.c` lines 1809–1842)

```c
 1809: 	}
 1810: 
 1811: 	getF20_40_48(bm, species, bm->RBCestimation.RBCspeciesArray[species].agesel, bm->RBCestimation.RBCspeciesArray[species].wt, bm->RBCestimation.RBCspeciesArray[species].mat, bm->RBCestimation.RBCspeciesArray[species].Mnat[0][0][0][0][year], bm->RBCestimation.RBCspeciesParam[species][Hsteep_id], llogfp);
 1812: 
 1813: 	return fmax;
 1814: 
 1815: }
 1816: 
 1817: 
 1818: //******************************************************************************
 1819: //
 1820: // Name:  Tier3rule
 1821: //      Tier3rule:        calculate RBC using the original Tier 3 rule, given Fcurr, Ccur and M
 1822: // Description: calculate RBC using the original Tier 3 rule, given Fcurr, Ccur and M
 1823: //
 1824: //
 1825: // called by : DoAssessment
 1826: // calls :
 1827: // created  : Mar 2008 Sally
 1828: //
 1829: // Note replaced the fixed constants with variable parameters so for reference
 1830: //  tier3RuleA = 0.0295
 1831: //  tier3RuleB = 0.2313
 1832: //  tier3RuleC = 1.209
 1833: //  tier3RuleD = 1.0413
 1834: //  tier3RuleE = 0.283
 1835: //
 1836: //******************************************************************************
 1837: void Tier3Rule(MSEBoxModel *bm, int species, int year, double current_catch, FILE *llogfp)
 1838: {
 1839: 	double p,r,mnat;
 1840: 
 1841: 	mnat = bm->RBCestimation.RBCspeciesArray[species].Mnat[0][0][0][0][year];
 1842: 	r = bm->RBCestimation.RBCspeciesParam[species][Fcurr_id] / mnat;
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
**Listed lines:** 1868-1886

**Function:** `YPR()`

**Why it's thought to be a bug**

YPR loops over `numCohorts`, but the `wt/mat/sel` arrays it is given are indexed by age in years (filled up to MaxAge in TierOneMimic/trueF and up to Tier3_maxage in CC/CCsel). When ageClassSize>1, only the first numCohorts *years* are used, so the plus group is wrong. Function: `YPR()`.

**Current code** (`atmanage/atManageTier.c` lines 1953–1975) — code appears to have moved +87 lines since the review

```c
 1953: 		fprintf(llogfp, "Time %e %s Calculate F20, F40, F48 - steepness: %e B-H stock-recruit alpha: %e B-H stock-recruit beta : %e (natM: %e)\n",
 1954: 				bm->dayt, FunctGroupArray[species].groupCode, hsteep, alpha, beta, natM);
 1955: 		for (a=0; a < FunctGroupArray[species].numCohorts; a++) {
 1956: 			fprintf(llogfp, "Time: %e %s a: %d wt: %e mat: %e agesel: %e\n", bm->dayt, FunctGroupArray[species].groupCode, a, wt[a], mat[a], agesel[a]);
 1957: 		}
 1958: 	}
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
**Listed lines:** 1971-1972

**Function:** `TierFour()`

**Why it's thought to be a bug**

`maxprop = avprop[1]; maxfleet = 1;` reads out of bounds when Nfleets==1 (avprop has size Nfleets) and defaults to fleet 1 rather than 0. Function: `TierFour()`.

**Current code** (`atmanage/atManageTier.c` lines 2102–2107) — code appears to have moved +133 lines since the review

```c
 2102: void TierFour (MSEBoxModel *bm, int species, int year, double current_catch, FILE *llogfp)
 2103: {
 2104: 	int iy, nf, maxfleet, ii;
 2105: 	double slope, maxprop;
 2106: 	//int do_debug = 0;
 2107: 	int Nfleets = (int)(bm->RBCestimation.RBCspeciesParam[species][NumFisheries_id]);
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
**Listed lines:** 2052, 2062

**Function:** `newTierFour()`

**Why it's thought to be a bug**

The CPUE target loop `iy < MaxCyr` excludes the last year, but the divisor `step1 = yrmax - yrmin + 1` counts it (and uses the unsanitised params). So CPUEtarg is biased low. Function: `newTierFour()` (same exclusive loop in `WriteCPUE()` 2177).

**Current code** (`atmanage/atManageTier.c` lines 2192–2206) — code appears to have moved +142 lines since the review

```c
 2192: 	cpuesum = 0.0;
 2193:     
 2194:     MinCyr = (int)(bm->RBCestimation.RBCspeciesParam[species][Tier4_CPUEyrmin_id]);
 2195:     MaxCyr = (int)(bm->RBCestimation.RBCspeciesParam[species][Tier4_CPUEyrmax_id]);
 2196:     // Sanity check
 2197:     if (MaxCyr < 1) MaxCyr = 1;
 2198:     if (MaxCyr > year) MaxCyr = year;
 2199:     if (MinCyr < 1) MinCyr = 1;
 2200:     if (MinCyr > MaxCyr) MaxCyr = MinCyr;
 2201: 
 2202:     nsum = 0;
 2203: 	for (iy = MinCyr; iy < MaxCyr + 1; iy++){   // Nov 2026: now includes MaxCyr (was iy < MaxCyr but divided by the full number of years)
 2204: 		cpuesum += bm->RBCestimation.RBCspeciesArray[species].CPUEgen[fleetid][region][iy];
 2205:         nsum++;
 2206:         
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
**Listed lines:** 2078-2095

**Function:** `newTierFour()`

**Why it's thought to be a bug**

`startyear = year - m; for (iy=startyear; iy<=year; ...)` sums m+1 years but divides by m (or by `year-startyear` when clipped). CPUEav is overstated by (m+1)/m, which makes the RBC multiplier too high. Function: `newTierFour()`.

**Current code** (`atmanage/atManageTier.c` lines 2074–2095)

```c
 2074: 			for (nf = 0; nf < bm->K_num_fisheries; nf++)
 2075: 				for (nreg = 0; nreg < Nregions; nreg++)
 2076: 					current_catch += Harvest_Get_TotCumCatch(species, nf, bm->thisyear);   // True catchs and discards
 2077: 			break;
 2078:     }
 2079:     
 2080:     // ---------  apply tier 3 rule to get RBC
 2081:     if (tiertype == orig_tier_rule) {   // original rule
 2082:         Tier3Rule(bm, species, year, current_catch, llogfp);
 2083:     } else if (tiertype == new_tier_rule) {    // Neils' new rule
 2084:         newTier3Rule(bm, species, year, current_catch, llogfp, tier5fp);
 2085:     }
 2086:     
 2087:     return;
 2088: }
 2089: 
 2090: //******************************************************************************
 2091: //
 2092: // Name:  TierFour
 2093: //      TierFour:         do an old-style tier 4 assessment and get RBC for next year
 2094: // Description: do a tier 4 assessment and get RBC for next year (old T4)
 2095: //
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
**Listed lines:** 2291, 2295, 2388

**Why it's thought to be a bug**

`k = simplex[0][0]; xpar[1] = simplex[0][0];` and in the objective `in_K = xpar[0];`. K is parameter 1 (comment: "xpar[1] = K"), but Bstart is used everywhere instead. The K dimension of the simplex is optimised for nothing, and depletion = BcurrSP/Bstart. Functions: `TierFive_SP()`, `SurplusProduction()`.

**Current code** (`atmanage/atManageTier.c` lines 2346–2350) — code appears to have moved +57 lines since the review

```c
 2346: //******************************************************************************
 2347: //
 2348: // Name: TierFive_SP:      do a tier 5 assessment - surplus production model - and get RBC for next year
 2349: // Description: do a tier 5 assessment - surplus production model
 2350: //              and get RBC for next year
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
**Listed lines:** 2397-2398

**Function:** `SurplusProduction()`

**Why it's thought to be a bug**

`step1 = (1 - (bt/K)^p) - T5_removal[iy-1]; B = bt + bt*r/p*step1;` multiplies the catch removal by `bt*r/p`. The projection loop at 2432-2433 correctly subtracts the removal outside. Function: `SurplusProduction()`.

**Current code** (`atmanage/atManageTier.c` lines 2416–2421) — code appears to have moved +21 lines since the review

```c
 2416: 			catchy += bm->RBCestimation.RBCspeciesArray[species].CatchData[nf][sumregion][iy];
 2417: 		}
 2418: 		bm->RBCestimation.RBCspeciesArray[species].T5_removal[iy] = discy + catchy;
 2419: 		
 2420:         /*
 2421:         if (do_debug){
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
**Listed lines:** 2430-2438

**Function:** `SurplusProduction()`

**Why it's thought to be a bug**

The projection loop `iy < index2` runs zero times for AssessDelay 0 and only to CurrentYear for delay 1. Then `BcurrSP = expectB[index1]` reads an entry that was never computed (outside the Init range at 2380, so stale or 0). The fitting loop also stops at `iy < CurrentYear`. Function: `SurplusProduction()`.

**Current code** (`atmanage/atManageTier.c` lines 2441–2453) — code appears to have moved +13 lines since the review

```c
 2441: 		for (j=0; j < npar; j++)
 2442: 			bm->RBCestimation.RBCspeciesArray[species].xpar[j] = bm->RBCestimation.RBCspeciesArray[species].simplex[i][j];
 2443: 		bm->RBCestimation.RBCspeciesArray[species].func[i] = SurplusProduction(bm, species, bm->RBCestimation.RBCspeciesArray[species].xpar, llogfp);
 2444: 	}
 2445: 
 2446: 
 2447: // call simplex to estimate Bstart, K, r
 2448: 	downhill_simplex(bm, SurplusProduction_model, species, bm->RBCestimation.RBCspeciesArray[species].simplex, bm->RBCestimation.RBCspeciesArray[species].func, npar, ftol, iter, &ok, llogfp, bm->RBCestimation.RBCspeciesArray[species].xpar);
 2449: 
 2450: 	/* Nov 2026: K is now its own parameter (simplex[0][1]). Original handling, in case need to switch back:
 2451: 	   K was set equal to Bstart, i.e.  k = simplex[0][0]  (and xpar[1] below = simplex[0][0], in_K = xpar[0] in SurplusProduction) */
 2452: 	bstart = bm->RBCestimation.RBCspeciesArray[species].simplex[0][0];
 2453: 	k = bm->RBCestimation.RBCspeciesArray[species].simplex[0][1];
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
**Listed lines:** 2475-2480

**Function:** `get_T5_F20_40_48()`

**Why it's thought to be a bug**

`do { iy++; ... TSbiomass[iy] = ... } while (... && iy < 1000.0);` can write `TSbiomass[1000]`, but the array is allocated with 1000 entries (atAssessTierSetup.c:499). Function: `get_T5_F20_40_48()`.

**Current code** (`atmanage/atManageTier.c` lines 2468–2477) — code appears to have moved -5 lines since the review

```c
 2468: 
 2469: 	// calculate F20, F40 and F48
 2470: 	get_T5_F20_40_48(bm, species, bstart, k, r, llogfp);
 2471: 
 2472: 	// now apply the 20:35:48 harvest control rule
 2473: 	if (current_depletion<0.2)
 2474: 		ftarg = 0.0;
 2475: 	else if (current_depletion<0.35)
 2476: 		ftarg = bm->RBCestimation.RBCspeciesParam[species][F40_id] * (current_depletion / 0.2 -1.0);
 2477: 	else
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
**Listed lines:** 2583-2589

**Function:** `TierFive_AvLen()`

**Why it's thought to be a bug**

`do { i=i+1; } while ((avlen < T5_Avlen_atF[i]) && (i < nfint));` evaluates `T5_Avlen_atF[nfint]` (array size nFtier3 == nfint) before the bound check, and never tests index 1. Function: `TierFive_AvLen()`.

**Current code** (`atmanage/atManageTier.c` lines 2622–2632) — code appears to have moved +41 lines since the review

```c
 2622: {
 2623: 	double fval, bt, longterm_dep, step1;
 2624: 	int i, iy, nfint;
 2625: 
 2626: 	Util_Init_1D_Double(bm->RBCestimation.RBCspeciesArray[species].TSbiomass, 1000, 0.0);
 2627: 
 2628: 	bm->RBCestimation.RBCspeciesParam[species][F20_id] = 0.0;
 2629: 	bm->RBCestimation.RBCspeciesArray[species].TSbiomass[0] = bstart;
 2630: 
 2631: 	nfint = (int)(1000 * bm->RBCestimation.RBCspeciesParam[species][Tier3_maxF_id]);   // max no of F's over which to calculate YPR
 2632: 	//bm->RBCestimation.RBCspeciesParam[species][Nfint_id] = nfint;  // As set globally at the start
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
**Listed lines:** 2639-2649

**Function:** `TierFiveDCAC()`

**Why it's thought to be a bug**

`for (iy...) { sumcatch = 0.0; ... }` resets the sum every year, so DCAC uses only the final year's catch and not the cumulative catch over `num_years`. Function: `TierFiveDCAC()`.

**Current code** (`atmanage/atManageTier.c` lines 2770–2784) — code appears to have moved +133 lines since the review

```c
 2770: //******************************************************************************
 2771: //
 2772: // Name:  TierFiveDCAC
 2773: //      TierFiveDCAC:   do a tier 5 assessment using Alec McCall's
 2774: //                        depletion corrected average catch
 2775: //
 2776: // called by : DoAssessment
 2777: // calls :
 2778: // created  : Oct 2014 Beth Fulton
 2779: //
 2780: //******************************************************************************
 2781: void TierFiveDCAC(MSEBoxModel *bm, int species, int year, double current_catch, FILE *llogfp) {
 2782:     int startyear = 0;
 2783:     int Nregions = (int)(bm->RBCestimation.RBCspeciesParam[species][NumRegions_id]);
 2784:     int Nstocks = FunctGroupArray[species].numStocks;
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
**Listed lines:** 2768-2774

**Function:** `TierSixRUSS()`

**Why it's thought to be a bug**

`localcatch` is not reset per year, so `HistCatch` ends up as cumulative total catch rather than the peak annual catch (compare TierSevenRUSS 3051, which resets it). Function: `TierSixRUSS()`.

**Current code** (`atmanage/atManageTier.c` lines 2909–2919) — code appears to have moved +143 lines since the review

```c
 2909: //******************************************************************************
 2910: //
 2911: // Name:  TierSixRUSS
 2912: //      TierSizRUSS:   catch trigger - see
 2913: //
 2914: // called by : DoAssessment
 2915: // calls :
 2916: // created  : Oct 2014 Beth Fulton
 2917: //
 2918: //******************************************************************************
 2919: void TierSixRUSS(MSEBoxModel *bm, int species, int year, FILE *llogfp) {
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
**Listed lines:** 2782-2794

**Function:** `TierSixRUSS()`

**Why it's thought to be a bug**

`RBCspeciesParam[species][TriggerReached_id] = 0;` is reset per region, but the array `TriggerReached[nreg]` (and `[Nregions]`) that the following loops set is never reset. Once a trigger has been reached it stays reached forever. Function: `TierSixRUSS()`.

**Current code** (`atmanage/atManageTier.c` lines 2780–2796)

```c
 2780: //******************************************************************************
 2781: void TierFiveDCAC(MSEBoxModel *bm, int species, int year, double current_catch, FILE *llogfp) {
 2782:     int startyear = 0;
 2783:     int Nregions = (int)(bm->RBCestimation.RBCspeciesParam[species][NumRegions_id]);
 2784:     int Nstocks = FunctGroupArray[species].numStocks;
 2785:     int Nfleets = (int)(bm->RBCestimation.RBCspeciesParam[species][NumFisheries_id]);
 2786:     double sb0 = bm->RBCestimation.RBCspeciesArray[species].SBioZero[Nstocks][Nregions];
 2787:     double num_years = (double)(year + 1);
 2788:     double depletion, WYopt, step1, nat_mort_est, t_agemat, sumcatch;
 2789:     int iy, nreg, nf;
 2790:     
 2791:     // Get simple estimate of natural mortality
 2792:     t_agemat = FunctGroupArray[species].speciesParams[age_mat_id] * FunctGroupArray[species].ageClassSize;
 2793:     if(!t_agemat)
 2794:         t_agemat = small_num;
 2795:     nat_mort_est = bm->RBCestimation.tier5JensenConst / t_agemat;
 2796:     
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
**Listed lines:** 2826-2827, 2835, 2848, 2851, 2863-2867

**Function:** `TierSixRUSS()`

**Why it's thought to be a bug**

`DoCPUE(bm, year, species, llogfp)`, `DoPrager(bm, year, species, ...)`, `Call_Tier3(bm, year, species, ...)` and `TierFiveSAFE(bm, year, species, totsumcatch, ...)` all pass (year, species) where the prototypes (lines 75-76, 1899, 57) expect (species, year). The wrong species is assessed, or there are out-of-bounds species indices. Function: `TierSixRUSS()`.

**Current code** (`atmanage/atManageTier.c` lines 2909–2935) — code appears to have moved +85 lines since the review

```c
 2909: //******************************************************************************
 2910: //
 2911: // Name:  TierSixRUSS
 2912: //      TierSizRUSS:   catch trigger - see
 2913: //
 2914: // called by : DoAssessment
 2915: // calls :
 2916: // created  : Oct 2014 Beth Fulton
 2917: //
 2918: //******************************************************************************
 2919: void TierSixRUSS(MSEBoxModel *bm, int species, int year, FILE *llogfp) {
 2920:     
 2921: 	int Nfleets = (int)(bm->RBCestimation.RBCspeciesParam[species][NumFisheries_id]);
 2922:     int Nregions = (int)(bm->RBCestimation.RBCspeciesParam[species][NumRegions_id]);
 2923: 	int nf, nreg, nt, ij, nd, iy;
 2924:     int TriggerScen = (int)(bm->RBCestimation.RBCspeciesParam[species][TriggerResponseScen_id]);
 2925:     int MaxTrigger = 0;
 2926:     int FisheryClosed = (int)(bm->RBCestimation.RBCspeciesParam[species][Closed_id]);
 2927:     int Ntriggers = bm->RBCestimation.NumTriggers;
 2928:     double regsumcatch, totsumcatch, adjTAC, prev_RBC, localcatch;
 2929:     double HistCatch = 0.0;
 2930: 
 2931: 	localcatch = 0.0;
 2932:     
 2933:     // Get historical level of catch
 2934:     for (iy = 0; iy < bm->RBCestimation.RBCspeciesParam[species][HistYrMax_id]; iy++){
 2935:         for (nf = 0; nf< Nfleets; nf++){
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
**Listed lines:** 2938-2950

**Function:** `DoCPUE()`

**Why it's thought to be a bug**

`for (iy = year-1; iy >= 0; iy++)` counts *up* without bound (runs off the end of EffortData). Also `j = iy` is assigned only when effort was *not* found, so j is not the most recent year with effort (if found immediately, j stays 0). Function: `DoCPUE()`.

**Current code** (`atmanage/atManageTier.c` lines 2940–2956) — code appears to have moved +4 lines since the review

```c
 2940:     }
 2941: 
 2942:     // First, see if any triggers have been reached for any level in the previous year:
 2943:     for (nreg = 0; nreg < Nregions; nreg++) {
 2944:         regsumcatch = 0.0;
 2945:         for (nf = 0; nf< Nfleets; nf++){
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
**Listed lines:** 2963

**Function:** `DoCPUE()`

**Why it's thought to be a bug**

`AnnCPUE[5-iy]` with iy going down to 0 writes `AnnCPUE[5]`, but AnnCPUE is allocated with 5 entries (atAssessTierSetup.c:215). Function: `DoCPUE()`.

**Current code** (`atmanage/atManageTier.c` lines 2990–2994) — code appears to have moved +29 lines since the review

```c
 2990:                 
 2991:                 if(bm->RBCestimation.tier6_to_tier4) {
 2992:                     adjTAC = DoCPUE(bm, year, species, llogfp); // To get average CPUE
 2993:                     adjTAC = DoPrager(bm, year, species, llogfp);
 2994: 
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
**Listed lines:** 3044, 3078, 3097, 3183, 3196, 3211

**Function:** `TierSevenRUSS()`

**Why it's thought to be a bug**

`cosp = FunctGroupArray[sp].co_sp[sp];` uses the loop counter as the group index. It should be the focal species' companion list, `FunctGroupArray[species].co_sp[sp]` (co_sp is per guild: atManageParamIO.c:259/932). Function: `TierSevenRUSS()`.

**Current code** (`atmanage/atManageTier.c` lines 3042–3081 (truncated))

```c
 3042:             // i.e. no fishing this year - store current TAC for later use and close fishery by zeroing TAC
 3043:             adjTAC = 0.0;
 3044:             bm->RBCestimation.RBCspeciesParam[species][lastRBC_id] = bm->RBCestimation.RBCspeciesParam[species][TAC_old_id];
 3045:             bm->RBCestimation.RBCspeciesParam[species][Closed_id] = 1;
 3046:     }
 3047:     bm->RBCestimation.RBCspeciesParam[species][RBCest_id] = prev_RBC * adjTAC;
 3048:     
 3049:     
 3050:     /* Now do spatial management */
 3051:     switch (TriggerScen) {
 3052:         case NoTrigger:  // No adjustment to previous year's quota or spatial management
 3053:         case ZoneTrigger:  // Trigger by zone
 3054:         case TotTrigger:   // Trigger system independent of zone
 3055:         case LaxZoneTrigger:  // Trigger by zone but no response to lowest level trigger being tripped
 3056:             // No extra spatial management to do as done via TAC or finer zone closures ignored
 3057:             break;
 3058:         case TightZoneTrigger: // If Level 2 or 3 trigger reached in a zone then zone is closed
 3059:             if (MaxTrigger > 1) {
 3060:                 for (ij=0; ij < bm->nbox; ij++) {
 3061:                     for (nreg = 0; nreg < Nregions; nreg++) {
 3062:                         if ((Nregions == 1) || (bm->RBCestimation.RBCspeciesArray[species].regID[ij] == nreg)) {
 3063:                             for (nf = 0; nf < bm->K_num_fisheries; nf++) {
 3064:                                 // If target the species then close the fishery
 3065:                                 if(bm->FISHERYtarget[nf][species] || (bm->SP_FISHERYprms[species][nf][q_id] > bm->RBCestimation.RBCspeciesParam[species][trigq_id])) {
 3066:                                     for (nd = 0; nd < 365; nd++) {
 3067:                                         bm->MPAlist[year+1][nd][ij][nf] = 0;
 3068:                                     }
 3069:                                 }
 3070:                             }
 3071:                         }
 3072:                     }
 3073:                 }
 3074:             }
 3075:             break;
 3076:         default:
 3077:             quit("No such Trigger Response Scenario defined for Tier 6 please reset TriggerResponseScen for %s (currently %d)\n", FunctGroupArray[species].groupCode, TriggerScen);
 3078:             break;
 3079:     }
 3080: 
 3081:     return;
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
**Listed lines:** 3146-3147

**Function:** `TierSevenRUSS()`

**Why it's thought to be a bug**

`if (bm->SP_FISHERYprms[cosp][nf][q_id] > 0.0) toteffort += bm->CumEffort[nf][b];` sits after the nf loops have finished, so nf == K_num_fisheries and both arrays are read out of bounds. Function: `TierSevenRUSS()`.

**Current code** (`atmanage/atManageTier.c` lines 3063–3068) — code appears to have moved -81 lines since the review

```c
 3063:                             for (nf = 0; nf < bm->K_num_fisheries; nf++) {
 3064:                                 // If target the species then close the fishery
 3065:                                 if(bm->FISHERYtarget[nf][species] || (bm->SP_FISHERYprms[species][nf][q_id] > bm->RBCestimation.RBCspeciesParam[species][trigq_id])) {
 3066:                                     for (nd = 0; nd < 365; nd++) {
 3067:                                         bm->MPAlist[year+1][nd][ij][nf] = 0;
 3068:                                     }
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
**Listed lines:** 3124-3156

**Function:** `TierSevenRUSS()`

**Why it's thought to be a bug**

`areafished` and `areasp` (and `toteffort`) are initialised once at function entry and not per co-species, so `relarea` for later species includes earlier species' areas. Function: `TierSevenRUSS()`.

**Current code** (`atmanage/atManageTier.c` lines 3179–3215) — code appears to have moved +57 lines since the review

```c
 3179: //******************************************************************************
 3180: //
 3181: // Name:  TierSevenRUSS
 3182: //      SevenRUSS:   group based assessment
 3183: //          Primary member of group called then do assessment (i.e. set only one
 3184: //          species as tiered 7 assessed and then it will assess that species and
 3185: //          its companion species (as defined by co_sp array)
 3186: //
 3187: // called by : DoAssessment
 3188: // calls :
 3189: // created  : Oct 2014 Beth Fulton
 3190: //
 3191: //******************************************************************************
 3192: void TierSevenRUSS(MSEBoxModel *bm, int species, int year, FILE *llogfp) {
 3193:     double this_catch, last_catch, totcatch, localcatch, toteffort = 0.0, HistCatch, CoHistCatch, diff;
 3194:     int Nregions = (int)(bm->RBCestimation.RBCspeciesParam[species][NumRegions_id]);
 3195:     int Ntriggers = bm->RBCestimation.NumTriggers;
 3196:     int iy, nt, sp, cosp, nf, b, biomhere, ij, cohort, den;
 3197:     int startyear = year - (int)bm->RBCestimation.tier7period;
 3198:     double areafished = 0.0;
 3199:     double areasp = 0.0;
 3200:     double relarea = 0.0;
 3201:     double maxrelcatch = 0.0;
 3202:     double tot_co_catch = 0.0;
 3203:     double hist_co_catch = 0.0;
 3204: 
 3205: 	if (startyear < 0) startyear = 0;
 3206: 
 3207:     /* Check peak catch */
 3208:     for (sp = 0; sp < bm->K_max_co_sp; sp++) {
 3209:         
 3210:         cosp = FunctGroupArray[sp].co_sp[sp];
 3211:         if ((cosp < 0) || (cosp > bm->K_max_impacted_sp))
 3212:             continue;
 3213:         
 3214:         // Get historical level of catch
 3215:         HistCatch = 0.0;
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
**Listed lines:** 3216

**Function:** `TierSevenRUSS()`

**Why it's thought to be a bug**

`Call_Tier3(bm, year, species, llogfp)` has its arguments swapped, and inside the loop over cosp it should assess `cosp`. Function: `TierSevenRUSS()`.

**Current code** (`atmanage/atManageTier.c` lines 3214–3218)

```c
 3214:         // Get historical level of catch
 3215:         HistCatch = 0.0;
 3216:         for (iy = 0; iy < bm->RBCestimation.RBCspeciesParam[species][HistYrMax_id]; iy++){
 3217:             localcatch = 0.0;
 3218:             for (nf = 0; nf< bm->K_num_fisheries; nf++){
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
**Listed lines:** 3718-3736

**Function:** `trigger_species_breakout()`

**Why it's thought to be a bug**

`catchwghtCPUE` is zeroed once outside the `lag` loop, so every lag adds into the same slots. The regression for lag k therefore includes data from lags > k. Also, when mgt_rbcYear==this_year, `yy` runs 1..TCP-1, so slot 0 is never filled and a spurious 0 enters the regression. Division by `totCatch` can also be 0. Function: `trigger_species_breakout()`.

**Current code** (`atmanage/atManageTier.c` lines 3920–3942) — code appears to have moved +204 lines since the review

```c
 3920:  * \brief  Trigger species breakout routine
 3921:  */
 3922: int trigger_species_breakout(MSEBoxModel *bm, int sp, int this_year){
 3923: 
 3924:     //TODO: set up this_year = bm->RBCestimation.RBCspeciesParam[sp].mgt.rbcYear
 3925:     
 3926:     int f = 0, r = 0, y = 0;
 3927:     //int LTcount = 0;
 3928:     int LTincr = FALSE;
 3929:     int STincr = FALSE;
 3930:     int TACcaptured = FALSE;
 3931:     double totCatch = 0;
 3932:     double lastCPUE = 0;
 3933:     int yy = 0;
 3934:     int lag = 4;
 3935:     double lastC = 0;
 3936:     double ans = 0;
 3937:     
 3938:     printf("Checking if %s is prevAssessment %e\n", FunctGroupArray[sp].groupCode, bm->RBCestimation.RBCspeciesParam[sp][prevAssessment_id]);
 3939: 
 3940:     if (!bm->RBCestimation.RBCspeciesParam[sp][prevAssessment_id]){
 3941:         //no previous assessment has been done AND it is a trigger species, best to do an assessment
 3942:          fprintf(bm->logFile, "Time: %e %s TRIGGER species BREAKOUT: no previous assessment\n", bm->dayt, FunctGroupArray[sp].groupCode);
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
**Listed lines:** 3787

**Function:** `CalculateNonIndicatorRBC()`

**Why it's thought to be a bug**

`(FunctGroupArray[sp].isImpacted == FALSE)` tests the target species and not the candidate indicator `i`. Function: `CalculateNonIndicatorRBC()`.

**Current code** (`atmanage/atManageTier.c` lines 3785–3789)

```c
 3785: 
 3786:     fprintf(llogfp, "Time: %e %s year: %d multi-year TAC decay (1 - myTACbuffer): %e (years since assessment: %e) %e becomes %e on the way to the TAC\n",
 3787:             bm->dayt, FunctGroupArray[species].groupCode, year, (1.0 - bm->RBCestimation.myTACbuffer), buff_yr, TACbase, newTACbase);
 3788: 
 3789:     return newTACbase;
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
**Listed lines:** 3877-3910, 3899

**Function:** `Check_Indicator_Links()`

**Why it's thought to be a bug**

The `yy` offset uses `GradientPeriod` (the lag count) and not `GradientWindow` (the array length). Indices can exceed the catchwght/catchwghtCPUE size (max(GradientWindow, TriggerCheckPeriod)), or be clipped so that many years pile into slot 0. The arrays are also not reset between lags (same issue as TIER-42). Function: `Check_Indicator_Links()`.

**Current code** (`atmanage/atManageTier.c` lines 3875–3912)

```c
 3875:         // Do nothing as no assessments running yet
 3876:     } else {
 3877:     
 3878:         if (year == bm->RBCestimation.ReviewYr) {
 3879:             Check_Indicator_Links(bm, year);
 3880:             bm->RBCestimation.ReviewYr = year + bm->RBCestimation.ReviewPeriod;
 3881:         }
 3882:     
 3883:         for (species = 0; species < bm->K_num_tot_sp; species++) {
 3884:             if (FunctGroupArray[species].isTAC > 1) {
 3885:                 assessType = (int) (bm->RBCestimation.RBCspeciesParam[species][MultispAssessType_id]);
 3886: 
 3887:                 printf("DoMultiStockAssessment Time: %e year %d doing species %s with assessType: %d\n", bm->dayt, year, FunctGroupArray[species].groupCode, assessType);
 3888:             
 3889:                 fprintf(bm->logFile, "DoMultiStockAssessment Time: %e year %d doing species %s with assessType: %d\n", bm->dayt, year, FunctGroupArray[species].groupCode, assessType);
 3890:             
 3891:                 switch(assessType) {
 3892:                     case MultiSpProd:
 3893:                         // Need to add call to multispecies production model
 3894:                         break;
 3895:                     case IndicatorSp:
 3896:                     case IndicatorSpPGMSY:
 3897:                         // Indicator species assessed in main tier code, this spot does the non indicator species
 3898:                         CalculateNonIndicatorRBC(bm, species, bm->RBCestimation.sim, year);
 3899:                         break;
 3900:                     case PGMSY:
 3901:                     case SingleSpOnly:
 3902:                     case NoAssess:
 3903:                     default: // Do nothing (when set to 0 or PGMSY as have flow through from PGMSY)
 3904:                         break;
 3905:                 }
 3906:                 
 3907:     
 3908:                 // Store the results - do we need separate loops for this?
 3909:                 if (assessType > NoAssess) {
 3910:                     Evaluation(bm, species, year, llogfp, bm->tierRBCfp);
 3911:                 }
 3912:              }
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
**Listed lines:** 3927-3929

**Function:** `Check_Indicator_Links()`

**Why it's thought to be a bug**

The else-branch sets `CPUEtrend_id = 0` where it should set `CPUEdir_id = 0`, so the direction stays stale and the trend is wiped before it is used in steps B/C. Function: `Check_Indicator_Links()`.

**Current code** (`atmanage/atManageTier.c` lines 3877–3883) — code appears to have moved -48 lines since the review

```c
 3877:     
 3878:         if (year == bm->RBCestimation.ReviewYr) {
 3879:             Check_Indicator_Links(bm, year);
 3880:             bm->RBCestimation.ReviewYr = year + bm->RBCestimation.ReviewPeriod;
 3881:         }
 3882:     
 3883:         for (species = 0; species < bm->K_num_tot_sp; species++) {
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
**Listed lines:** 3950-3952, 3968-3970, 4022-4024, 4042-4044

**Function:** `Check_Indicator_Links()`

**Why it's thought to be a bug**

`if (diff < closest_slope) ref_sp = ind_sp;` never updates `closest_slope` (or `closest_slopeB`), so ref_sp ends up as the *last* candidate (possibly `sp` itself), not the closest. And `closest_slope < closest_slopeB` (3981/4055) compares MAXDOUBLE with MAXDOUBLE, so ref_spB is always chosen. Function: `Check_Indicator_Links()`.

**Current code** (`atmanage/atManageTier.c` lines 3877–3883) — code appears to have moved -71 lines since the review

```c
 3877:     
 3878:         if (year == bm->RBCestimation.ReviewYr) {
 3879:             Check_Indicator_Links(bm, year);
 3880:             bm->RBCestimation.ReviewYr = year + bm->RBCestimation.ReviewPeriod;
 3881:         }
 3882:     
 3883:         for (species = 0; species < bm->K_num_tot_sp; species++) {
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
**Listed lines:** 3967

**Function:** `Check_Indicator_Links()`

**Why it's thought to be a bug**

The other-category diff is missing `fabs()` (the same-category loop at 3949 has it), so any species with a more negative trend counts as "closest". Function: `Check_Indicator_Links()`.

**Current code** (`atmanage/atManageTier.c` lines 3877–3881) — code appears to have moved -88 lines since the review

```c
 3877:     
 3878:         if (year == bm->RBCestimation.ReviewYr) {
 3879:             Check_Indicator_Links(bm, year);
 3880:             bm->RBCestimation.ReviewYr = year + bm->RBCestimation.ReviewPeriod;
 3881:         }
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
**Listed lines:** 3987, 4061

**Function:** `Check_Indicator_Links()`

**Why it's thought to be a bug**

`maxTier = mgt_category_id;` defaults to the enum index constant and not the species' current category (`my_category`). When no species falls inside the buffer, the category is set to an arbitrary number. Function: `Check_Indicator_Links()`.

**Current code** (`atmanage/atManageTier.c` lines 3877–3881) — code appears to have moved -108 lines since the review

```c
 3877:     
 3878:         if (year == bm->RBCestimation.ReviewYr) {
 3879:             Check_Indicator_Links(bm, year);
 3880:             bm->RBCestimation.ReviewYr = year + bm->RBCestimation.ReviewPeriod;
 3881:         }
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
**Listed lines:** 4156, 4164-4172

**Function:** `RDynamicTier4Assessment()`

**Why it's thought to be a bug**

`chdir(dirname)` never returns to the parent directory, so later relative file I/O and next year's mkdir end up nested. `Util_Close_Output_File(bm->Dyn4File)` closes the file but leaves the pointer non-NULL (atUtil.c:200), so on the next call `if(!bm->Dyn4File)` is false and the code writes to a closed FILE*. Function: `RDynamicTier4Assessment()`.

**Current code** (`atmanage/atManageTier.c` lines 3621–3641) — code appears to have moved -533 lines since the review

```c
 3621:     Util_Close_Output_File(bm->tierRBCfp);
 3622:     Util_Close_Output_File(histCPUEfp);
 3623:     Util_Close_Output_File(tier5fp);
 3624:         
 3625: }
 3626: 
 3627: /**
 3628:  *	\brief Routines to initialise the v Biomass information file
 3629:  *
 3630:  */
 3631: static FILE * Init_tierRBC_File(MSEBoxModel *bm) {
 3632: 	FILE *fid;
 3633: 	char fname[STRLEN];
 3634: 	int nf;
 3635:     int Nfleets = bm->K_num_fisheries;
 3636:     
 3637: 	/** Create filename **/
 3638: 	snprintf(fname, sizeof(fname), "%sTierRBC.txt", bm->startfname);
 3639:     
 3640:     /** Create file **/
 3641:     if ( (fid=Util_fopen(bm, fname, "w")) == NULL )
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
**Listed lines:** 4280-4282

**Function:** `writeDynTier4inputfile()`

**Why it's thought to be a bug**

The third block under the "Year, Catch and CPUE" header writes `CatchData` a second time where CPUE (`CPUEgen`) is expected, so the R assessment receives catch as CPUE. Function: `writeDynTier4inputfile()`.

**Current code** (`atmanage/atManageTier.c` lines 4183–4189) — code appears to have moved -95 lines since the review

```c
 4183:                     for (r = 0; r < bm->RBCestimation.RBCspeciesParam[sp][NumRegions_id]; r++) {
 4184:                         for (f = 0; f < bm->RBCestimation.RBCspeciesParam[sp][NumFisheries_id]; f++) {
 4185:                             bm->RBCestimation.catchwghtCPUE[yy] += (bm->RBCestimation.RBCspeciesArray[sp].CatchData[f][r][y] / totCatch) * bm->RBCestimation.RBCspeciesArray[sp].CPUEgen[f][r][y];
 4186:                             bm->RBCestimation.catchwght[yy] += bm->RBCestimation.RBCspeciesArray[sp].CatchData[f][r][y] / totCatch;
 4187:                         }
 4188:                     }
 4189:                 }
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
**Listed lines:** 4269

**Function:** `writeDynTier4inputfile()`

**Why it's thought to be a bug**

`t` is never incremented, so `t <= HistYrMax` is always true and the retro-catch adjustment applies to all years. Function: `writeDynTier4inputfile()`.

**Current code** (`atmanage/atManageTier.c` lines 4267–4271) — ⚠ quoted code not found in current file (changed or already fixed?)

```c
 4267:                 maxCount = 0;
 4268:                 for (itier = 0; itier < K_num_indicator_tiers; itier++ ){
 4269:                     if (bm->RBCestimation.count_sp_in_buffer[itier] > maxCount) {
 4270:                         maxCount = bm->RBCestimation.count_sp_in_buffer[itier];
 4271:                         maxTier = itier;
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
**Listed lines:** 4456

**Function:** `setRBC()`

**Why it's thought to be a bug**

`...= 0.0; // temporary value ...        bm->RBCestimation.RBCspeciesParam[species][EstB0_id] = 999.9;` puts the EstB0 assignment inside the comment, so EstB0 is not flagged on failure and EstDepletion (4467) mixes a valid B0 with Bcurr=999.9. Function: `setRBC()`.

**Current code** (`atmanage/atManageTier.c` lines 4454–4458) — ⚠ quoted code not found in current file (changed or already fixed?)

```c
 4454:                     maxFabc = Flim * ((B2B40 - alpha) / (1.0 - alpha));
 4455:                 else
 4456:                     maxFabc = 0.0;
 4457:             }
 4458:             break;
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
**Listed lines:** 4481-4490

**Function:** `Averaged_RBC()`

**Why it's thought to be a bug**

The loop `yr < mgt_rbcYear + n - 1` sums n-1 RBCs but divides by n. The replacement loop writes `RBC_by_year[year]` repeatedly and not `[yr]`. Function: `Averaged_RBC()`.

**Current code** (`atmanage/atManageTier.c` lines 4160–4173) — code appears to have moved -319 lines since the review

```c
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
**Listed lines:** 3243

**Function:** `Evaluation()`

**Why it's thought to be a bug**

`tiertype = FunctGroupArray[species].speciesParams[tiertype_id]`: tiertype_id belongs to the RBC_SPECIES_PARAMS enum (atlantisboxmodel.h:2513/2706). DoTierAssessment reads `RBCspeciesParam[species][tiertype_id]`, so the tier5 log branch tests an unrelated parameter. Function: `Evaluation()`.

**Current code** (`atmanage/atManageTier.c` lines 3393–3397) — code appears to have moved +152 lines since the review

```c
 3393: //******************************************************************************
 3394: //
 3395: // Name:  Evaluation
 3396: //      Evaluation:       write results for later evaluation
 3397: // Description: write results for later evaluation
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
**Listed lines:** 3256-3258, 3292

**Function:** `Evaluation()`

**Why it's thought to be a bug**

`sb0 = SBioZero[0][0]` is used with `SpawnBio[Nstocks][Nregions]` (elsewhere the overall value is at [Nstocks][Nregions]). `step1 = CPUEgen[fleetid][r][year]` is computed with the stale `r` (=Nregions) outside the loop and printed for every region. Function: `Evaluation()`.

**Current code** (`atmanage/atManageTier.c` lines 3254–3293 (truncated))

```c
 3254:     for (nt=0; nt < Ntriggers; nt++) {
 3255:         if (tot_co_catch > (bm->RBCestimation.RBCspeciesArray[species].TriggerPoints[nt][Nregions] * CoHistCatch))
 3256:             bm->RBCestimation.RBCspeciesArray[species].TriggerReached[Nregions] = nt;
 3257:     }
 3258: 
 3259:     
 3260:     /* Check to see if proportional composition of the catch of any species
 3261:      changes > x% (either in 1 step in 1 yr or cumulatively over 3 yrs) */
 3262:     for (sp = 0; sp < bm->K_max_co_sp; sp++) {
 3263:         cosp = FunctGroupArray[sp].co_sp[sp];
 3264:         if ((cosp < 0) || (cosp > bm->K_max_impacted_sp))
 3265:             continue;
 3266:         
 3267:         bm->RBCestimation.RBCspeciesArray[species].CatchComp[cosp][year] /= (tot_co_catch + small_num);
 3268:         // Check against previous tier7period (e.g. 3) years
 3269:         nt = 0;
 3270:         last_catch = 0.0;
 3271:         for (iy = startyear; iy < year; iy++) {
 3272:             diff = fabs(bm->RBCestimation.RBCspeciesArray[species].CatchComp[cosp][year] - bm->RBCestimation.RBCspeciesArray[species].CatchComp[cosp][iy]);
 3273:             this_catch = bm->RBCestimation.RBCspeciesArray[species].CatchComp[cosp][iy];
 3274:             if (diff > bm->RBCestimation.tier7maxcompchange) {  // Check for proportional change (has it changed by more than y% overall?)
 3275:                 if (bm->RBCestimation.RBCspeciesArray[species].TriggerReached[Nregions] < 1)
 3276:                     bm->RBCestimation.RBCspeciesArray[species].TriggerReached[Nregions] = 1;
 3277:             }
 3278:             diff = last_catch - this_catch;  // Check annual change (has it declined by > x%)
 3279:             if (diff > bm->RBCestimation.tier7maxannchange)
 3280:                 nt++;
 3281:             last_catch = this_catch;
 3282:         }
 3283:         if (nt >= bm->RBCestimation.tier7period) {
 3284:             if (bm->RBCestimation.RBCspeciesArray[species].TriggerReached[Nregions] < 1)
 3285:                 bm->RBCestimation.RBCspeciesArray[species].TriggerReached[Nregions] = 1;
 3286:         }
 3287:         
 3288:     /* Now check if the proportion of the area fished changes > y% */
 3289:     /* Also check if the proportion of the catch from any one location is > z% */
 3290:         relarea = 0.0;
 3291:         maxrelcatch = 0.0;
 3292:         for(b=0; b<bm->nbox; b++) {
 3293:             biomhere = 0;
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
**Listed lines:** 1724-1725, 2528-2529, 2579-2580

**Why it's thought to be a bug**

Hard-coded `if (bm->dayt > 10584) printf(...)` debug prints left in live code. Functions: `newTier3Rule()`, `TierFive_AvLen()`.

**Current code** (`atmanage/atManageTier.c` lines 1764–1769) — code appears to have moved +42 lines since the review

```c
 1764: 	if (do_debug) {
 1765: 		fprintf(llogfp,"Time: %e %s s50: %e s95: %e F: %e ss: %e\n",
 1766: 				bm->dayt, FunctGroupArray[species].groupCode, inS50, inS95, fmort, ss);
 1767: 	}
 1768:      */
 1769:     
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

20:35:48 rule (983, 2314): `ftarg = F40*(dep/0.2 - 1)` between 0.2 and 0.35 gives 0.75·F40 at 0.35, then jumps to F48. It looks copied from a 20:40 rule. Check the intended breakpoint and scaling.

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

Several places index `year-4`/`year-3` without clamping at 0 (`TierThree_CC` 1092, `TierFour` 1981). The weighted discard average (744-758) excludes year 0 (`year-1 > 0` should probably be `>= 0`).

- [ ] Investigate   - [ ] Ignore

**Your comments:**

> 

### LC-5

`Check_Indicator_Links()`: `buffer = GradientBuffer * CPUEtrend` is relative to the trend itself, so with GradientBuffer<1 any non-zero trend is classified ±1. A buffer computed from a negative trend also makes `diff < buffer` impossible. `ref_sp`/`ref_spB` can stay -1 and then be used as an index in `CalculateNonIndicatorRBC()` (3806).

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
