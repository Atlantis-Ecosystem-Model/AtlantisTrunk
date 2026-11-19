# Shared tier-assessment code: suspected bugs for review

Prepared 2026-11-05 while porting the Ratpack (PuntRatpackR) assessments into Atlantis.

**Update 2026-11-19:** items 27 (option c), 28 and 29 are done, and item 30 is left as is. Item 31 (found while doing 29) is done as you modified it.

**Update 2026-11-13:** every item marked Fix or Modify has now been applied. Each one has a **Status** line under its decision. You applied 1, 5, 6 and 18 yourself. Item 7 was done later, on 2026-11-14. Section G lists problems found while fixing. Items 22–26 are all done.

Line numbers refer to the current working copy. Paths are relative to `atlantis/`.

**Effect on past runs**
- **None**: the code path currently quits or can't run, so no past results can change.
- **Changes results**: only runs that use this path are affected.

---

## A. TAC meta-rules (`DoTierAssessment`, `atmanage/atManageTier.c`)

### 1. TAC change limits were inverted (APPLIED in Phase 1, as you approved)
- **Where:** `atManageTier.c:503-528`
- **Was:** `tac_change = (TAC_old - TACpost)/TAC_old`. If it was > `inc_check` the TAC went **up**, and the second test compared against `prop_decTAC` (0.9) rather than `dec_check` (0.1).
- **Example:** with TAC_old = 100, `prop_incTAC` = 1.2 and `prop_decTAC` = 0.9:
  - a recommended 50 gave **120**;
  - a recommended 150 gave **90**;
  - a recommended 95 or 105 gave **90**.
- **Ratpack:** `Assessment.cpp:564-569` uses the same sign convention but the opposite actions.
- **Now:** `tac_change = (TACpost - TAC_old)/TAC_old`, capped at `+inc_check` and `-dec_check`. With no previous TAC (TAC_old = 0), neither the limits nor `min_TACchange` apply; before, this was a division by zero that gave TAC = 0.
- **Effect on past runs:** changes results for any tiered run (tier > 0, not in rollover) that uses `prop_incTAC`/`prop_decTAC`.
- **Decision:** applied.

### 2. Rolled-over TACs shrink every year (buffer and discard removal applied again)
- **Where:** `Apply_RBC_Buffers` (`atManageTier.c:3601`) and the RBC-to-TAC block (`atManageTier.c:457-460`)
- **Problem:**
  - In a multi-year-TAC or trigger rollover year, `DoTierAssessment` sets `tier = sp_rollover` and `RBCest = TAC_old`.
  - But `Apply_RBC_Buffers` re-reads the tier from `speciesParams[tier_id]` (e.g. tier 4), so it multiplies the old TAC by `Tier3Sig` again. Its `case sp_rollover` is never reached.
  - The RBC-to-TAC step then removes discards again: `TACdisc = RBC - discard_rate*RBC`, even though `TAC_old` is already a retained-catch TAC.
- **Example:** tier 4, `Tier3Sig` = 0.7, discard rate 0.1, 3-year MYTAC. Year 1 TAC = 100. Year 2 gives 100 × 0.7 × 0.9 = 63. Year 3 gives 39.7.
  - Without buffers it is still 100 → 90 → 81.
- **Ratpack:** the TAC vector is carried forward unchanged (`RatpackR.R:1262-1266`). Its only staleness adjustment is `SetBuffer`, which is now the opt-in `Buffer_stale_rate`.
- **Proposed fix:** in rollover years, skip the buffer (except the explicit MYTAC `myTACbuffer` decay) and skip discard removal, so the TAC rolls over unchanged. Concretely:
  - pass the effective `tier` into `Apply_RBC_Buffers`;
  - treat `sp_rollover` like the tier-1 retained case in the discard step.
- **Effect on past runs:** changes results for runs with `myTACperiod` > 1 or trigger management.
- **Decision:** ☐ Fix ☐ Leave ☐ Modify: Fix
- **Status (2026-11-13): done.** `Apply_RBC_Buffers` now takes the effective `tier` from `DoTierAssessment`, so rollover years no longer get the tier buffer again. In rollover years discards are no longer removed a second time, unless the Ratpack staleness buffer is on: that one is based on the last assessed RBC, which still includes discards. The MYTAC buffer now applies one year of `(1 - myTACbuffer)` to last year's TAC. Last year's TAC already includes the earlier years, so the overall decay is `(1 - myTACbuffer)^years`. The old `pow(..., buff_yr)` would have compounded it, but that line could never be reached.

### 3. `sp_rollover` case has no `break` in `Apply_RBC_Buffers`
- **Where:** `atManageTier.c:3635-3641`
- **Problem:** the case falls through to `default: quit("No such tier")`. Because of item 2 it's currently unreachable, but it becomes reachable once item 2 is fixed.
- **Proposed fix:** add `break;`.
- **Effect on past runs:** none.
- **Decision:** ☐ Fix ☐ Leave ☐ Modify: Fix
- **Status (2026-11-13): done.** `break;` added.

### 4. RBC parameters read from the wrong array
- **Where:**
  - `atManageTier.c:194` (`est_discard_rate_id`)
  - `527`, `529` (`MinCatch_id`)
  - `3317` (`tiertype_id` in `Evaluation`)
  - `3603-3605` (`Tier1Sig_id`, `Tier2Sig_id`, `Tier3Sig_id`)
  - `atassess/atassessParamIO.c:648` (`MultispAssessType_id`)
- **Problem:** these ids belong to the `RBC_SPECIES_PARAMS` enum, but they index `FunctGroupArray[sp].speciesParams[]`, so they read whichever biological parameter has the same index number.
  - `Tier1Sig_id`'s index in `speciesParams` is some unrelated biology parameter, so the "buffer" applied is effectively arbitrary.
  - The discard rate used to convert RBC to TAC is also arbitrary unless `Tier_averages` overwrites it. It does overwrite it for tiers > 0.
- **Proposed fix:** use `bm->RBCestimation.RBCspeciesParam[sp][X_id]` everywhere.
- **Effect on past runs:**
  - Changes results for runs with `UseTierBuffers` = 1, for `MinCatch` and for the PGMSY on/off check.
  - The `Evaluation` line is logging only.
- **Decision:** ☐ Fix ☐ Leave ☐ Modify: Fix
- **Status (2026-11-13): done.** All five locations now read `RBCspeciesParam`. In `atassessParamIO.c` the same `if` also tested `speciesParamStructArray[sp].paramType`, which indexes the parameter table by species. It now tests `FunctGroupArray[sp].isImpacted`.

### 5. `MinCatch` floor uses the raw parameter instead of the computed minimum
- **Where:** `atManageTier.c:527-529`
- **Code:**
  ```c
  minCatch = MinCatch * lastHistCatch;
  if (TAC < minCatch) TAC = MinCatch;
  ```
- **Problem:** with `MinCatch` = 0.05 and a last historical catch of 200 t, the threshold is 10 t, but the TAC is then set to 0.05 (kg). Item 4 also applies to this line.
- **Ratpack:** `if (TAC < Min.TAC) TAC <- Min.TAC`, applied only if Last.TAC > 0.
- **Proposed fix:** `TAC = minCatch;`, and apply it only when `TAC_old > 0`.
- **Effect on past runs:** changes results whenever the floor binds.
- **Decision:** ☐ Fix ☐ Leave ☐ Modify: applied

### 6. Fleet TAC cap uses a stale loop index
- **Where:** `atManageTier.c:561-562`
- **Code:** `if (bm->TACamt[species][nf][now_id] > no_quota) bm->TACamt[species][nf][now_id] = no_quota;` sits inside a loop over `nfishery`.
- **Problem:** `nf` is left over from an earlier loop, so the cap is applied to one fixed fishery (or out of range) rather than to each fishery.
- **Proposed fix:** `nf` → `nfishery`.
- **Effect on past runs:** changes results only when a fleet TAC exceeds `no_quota` (rare).
- **Decision:** ☐ Fix ☐ Leave ☐ Modify: applied

### 7. `RBC_by_year` stores the TAC, not the RBC
- **Where:** `atManageTier.c:545`
- **Code:** `RBC_by_year[year] = TAC;`
- **Problem:** `Averaged_RBC`, `trigger_species_breakout` and the indicator-link code read `RBC_by_year` expecting RBCs. They actually get post-limit TACs.
- **Proposed fix:** store the RBC (before buffers and limits). If anything needs the TAC history, add a separate `TAC_by_year`.
- **Effect on past runs:** changes results for runs using `UseRBCAveraging` or trigger/indicator management.
- **Decision:** ☐ Fix ☐ Leave ☐ Modify: Leave for now
- **Status (2026-11-14): done, at your later request ("make it RBC_by_year consistently, but also store TAC_by_year").**
  - `RBC_by_year` now always holds the RBC: the assessment's RBC after any averaging, before buffers and TAC meta-rules.
  - A new `TAC_by_year` array holds the TAC; it's allocated and freed next to `RBC_by_year`.
  - In rollover years (multi-year TAC or trigger), the RBC from the last assessment is carried forward.
  - Before assessments start, and in the Atlantis PGMSY update (`atPGMSY.c`), RBC and TAC are the same number, so it's stored in both arrays.
  - All the code that reads `RBC_by_year` (RBC averaging, the trigger catch check and the indicator delta-RBC) wants RBCs, so it now gets them.

- **Correction (2026-11-14, later):** the trigger catch check was wrongly switched to compare catch with the RBC. Ratpack compares last year's catch with last year's TAC, so it now uses `TAC_by_year`.
### 8. `mgt_rbcYear` is never set
- **Where:** it is read at `atManageTier.c:3800-3802`, `3844`, `4007`, `5315`, `5362-5369`, but nothing ever assigns it.
- **Problem:** the struct is `malloc`ed, so the value is whatever is in memory, often 0. The RBC averaging, trigger and indicator checks then index from the wrong year, possibly outside the array.
- **Ratpack:** `mgt.rbcYear` is the first year the new RBC applies (data year + 1).
- **Proposed fix:** initialise it to 0 in `Tier_Assessment_Setup`, and set it to `year + 1` (after the delay adjustment) at the start of each real assessment in `DoTierAssessment`.
- **Effect on past runs:** changes results for runs using averaging or trigger/indicator management. Those results currently depend on uninitialised memory.
- **Decision:** ☐ Fix ☐ Leave ☐ Modify: Fix
- **Status (2026-11-13): done.** Initialised to 0 in `Tier_Assessment_Setup` and set to `year + 1` at each actual assessment (not in rollover years).

- **Correction (2026-11-14, later):** `mgt_rbcYear` is now updated every year (data year + 1), as in Ratpack, not only in assessment years. It's set before the trigger check, which reads the catch in `mgt_rbcYear - 1`. Before, that was the year of the last assessment rather than last year.
### 9. `Averaged_RBC` sums n−1 years, divides by n, and writes to the wrong index
- **Where:** `atManageTier.c:5357-5376`
- **Problem:**
  - The loop `yr < mgt_rbcYear + nRBCaverage - 1` sums `nRBCaverage − 1` values, then divides by `nRBCaverage`.
  - The second loop writes `RBC_by_year[year]` (not `[yr]`) on every pass.
- **Example:** with `nRBCaverage` = 3 and RBCs of 100 each, the "average" is 66.7.
- **Proposed fix:** average the most recent `nRBCaverage` assessed RBCs, ending at the current one: `for (yr = year - n + 1; yr < year + 1; yr++)`, and write only `RBC_by_year[year]`. This depends on items 7 and 8.
- **Effect on past runs:** changes results for runs with `UseRBCAveraging_XXX` = 1.
- **Decision:** ☐ Fix ☐ Leave ☐ Modify: Fix
- **Status (2026-11-13): done.** The current RBC is stored first, then the last `nRBCaverage` years (up to and including this one) are averaged. Averaging is also no longer applied in rollover years. Since item 7 is now done (2026-11-14), every year in the average is an RBC. The R-linked PGMSY path (`Read_PGMSY_output` in `atRlinkRBC.c`) had its own copy of the old faulty averaging; it now averages the same way.

---

## B. Tier-specific rules

### 10. `newTierFour`: the CPUE target leaves out the last reference year but divides by the full count
- **Where:** `atManageTier.c:2115` (`newTierFour`, target-CPUE loop).
- **Code:** `for (iy = MinCyr; iy < MaxCyr; iy++)` sums `MaxCyr − MinCyr` years, then divides by `(Tier4_CPUEyrmax − Tier4_CPUEyrmin + 1)`.
- **Example:** reference years 10–14 with CPUE of 1 each give CPUEtarg = 4/5 = 0.8 instead of 1. The target is biased low, so RBCs are biased high.
- **Ratpack:** C++ `_TierFoursetup` sums `Tier4_CPUEyrmin..Tier4_CPUEyrmax` inclusive.
- **Proposed fix:** `iy < MaxCyr + 1`, with the divisor = number of years actually summed.
- **Effect on past runs:** changes results for tier 4 with tiertype 2.
- **Decision:** ☐ Fix ☐ Leave ☐ Modify: Fix
- **Status (2026-11-13): done.** The loop now includes `MaxCyr`, and the divisor is the number of years actually summed.

### 11. `newTierFour`: the recent-CPUE average sums m+1 years but divides by m
- **Where:** `atManageTier.c:2141-2143` (`newTierFour`, recent-average loop).
- **Code:** `startyear = year - Tier4_m;` then `for (iy = startyear; iy <= year; iy++)`. The comment reads "Used to have +1 on end".
- **Example:** with m = 4 and CPUE of 1 each, CPUEav = 5/4 = 1.25. The average is biased high by (m+1)/m, so RBCs are biased high.
- **Ratpack:** sums `dataYear − m + 1 .. dataYear` (m years) and divides by m.
- **Proposed fix:** `startyear = year - Tier4_m + 1;` and `iy < year + 1`.
- **Effect on past runs:** changes results for tier 4 with tiertype 2.
- **Decision:** ☐ Fix ☐ Leave ☐ Modify: Fix
- **Status (2026-11-13): done.**

### 12. `newTierFour` and `TierFour` use CPUE for region 0, not all regions
- **Where:** `region = 0; // weighted sum over regions` in both functions (e.g. `atManageTier.c:2090`).
- **Problem:** the all-region catch-weighted CPUE is stored at index `Nregions` (`atSS3DataGen.c:637-650`), not 0. With `NumRegions` > 1, these rules use region 0 only. With 1 region the result is the same.
- **Ratpack:** has only one region.
- **Proposed fix:** use `Nregions`.
- **Effect on past runs:** changes results only for multi-region assessments.
- **Decision:** ☐ Fix ☐ Leave ☐ Modify: Fix
- **Status (2026-11-13): done** in both `newTierFour` and `TierFour`. Not changed: the post-assessment CPUE rule in `DoTierAssessment` (`PostRule_id`) also uses `CPUEgen[CPUEfleet][0]`, i.e. region 0 (see item 25).

### 13. `newTier3Rule`: the F ramp formula has the wrong dimensions
- **Where:** `atManageTier.c:1796-1804`
- **Code:**
  ```c
  frbc = F48 / ((F40 - F20) * (Fcurr - F20));
  ```
- **Problem:** this has units of 1/F and is not a ramp. With F20 = 0.4, F40 = 0.2, F48 = 0.15 and Fcurr = 0.3, it gives frbc = 0.15 / (−0.2 × −0.1) = **7.5**. `Tier3_mult` is capped at 3, so the RBC is about 3 × current catch.
- **Expected (SESSF Tier 3):** F_RBC = F48 × (F20 − Fcurr)/(F20 − F40) between F40 and F20. That gives 0.075 in the example.
- **Ratpack:** I couldn't find a Tier 3 rule in ratpackMSE to compare against, so please confirm the intended form.
- **Proposed fix:** `frbc = F48 * (F20 - Fcurr) / (F20 - F40);`
- **Effect on past runs:** changes results for tier 3 with tiertype 2, and for tier 5 with tiertype 2 (avlen), which also calls it.
- **Decision:** ☐ Fix ☐ Leave ☐ Modify: please include the fix but put old eqaution inf "tier2_replicate_old" flag so it is not lost.
- **Status (2026-11-13): done.** The corrected ramp is the default. The old equation is kept behind a new per-species flag. I named it **`Tier3_replicate_old_XXX`** rather than "tier2_replicate_old", because it switches the Tier 3 rule; it's easy to rename if you want. It sits in the optional Ratpack block (default 0), so old prm files convert without it. Set it to 1 to replicate old runs.

### 14. Downhill simplex (Amoeba) can't fit the tier 3 CCsel or tier 5 surplus-production models
- **Where:** `atassess/atsolve.c:81-250`
- **Problems:**
  - `mpts = ndim` should be `ndim + 1`: a simplex has n+1 vertices, so one vertex is never updated.
  - `Funk()` for `EquilF_Funk_model` and `SurplusProduction_model` ignores the trial point `X`. It evaluates the objective at `xpar` and returns 0, so every trial point looks equally good.
  - On convergence the best point is swapped into `p[1]`, but callers read `simplex[0]`.
- **Effect:**
  - `TierFive_SP` and `TierThree_CCsel` return close to their starting values.
  - In `TierFive_SP`, `ok` is never set, so it is uninitialised.
- **Proposed fix:**
  - correct `mpts`;
  - have `Funk` copy `X` into `xpar` and return the objective for these two models;
  - swap the best point into `p[0]`;
  - set `*ok`.

  The new dynamic Tier 4 code uses its own optimiser, so it isn't affected.
- **Effect on past runs:** changes results for tier 3 with `Tier3_Fcalc` 2, and for tier 5 with tiertype 1.
- **Decision:** ☐ Fix ☐ Leave ☐ Modify: Fix
- **Status (2026-11-13): done; see item 23, which has since fixed `Amoeba` itself.** The first version of this fix was limited to the tier code: `Amoeba` is also used by the classical sampling assessments (`atclassical.c`) and the regression fit (`atsolve.c`). To keep those results unchanged, `downhill_simplex` (used only by the tier code) now has its own corrected Nelder-Mead in `atManageTier.c`: it uses n+1 vertices, puts the best point in row 0, leaves `xpar` at the best point and sets `ok`. `Funk` now evaluates `EquilF_Funk` and `SurplusProduction` at the trial point. Also fixed: the `TierThree_CCsel` restart read `simplex[1][1]` (S95 of a non-best vertex) and now reads `simplex[0][0]`. Tested on a 3-parameter function: it converges to the exact minimum.

### 15. `DoCPUE` loop counts the wrong way
- **Where:** `atManageTier.c:3012`
- **Code:** `for (iy = year-1; iy >= 0; iy++)` is meant to search back for the most recent year with non-zero effort.
- **Problem:** `iy++` runs forward off the end of the array, unless a non-zero year is found first.
- **Proposed fix:** `iy--`.
- **Effect on past runs:** changes results only for tier 6 with a CPUE response. Those results currently depend on out-of-range memory.
- **Decision:** ☐ Fix ☐ Leave ☐ Modify: Fix
- **Status (2026-11-13): done.** The loop counts back, and `j` is now set to the year found. Before, `j` was only set on zero-effort years.

---

## C. Dynamic Tier 4: R path (tiertype 1)

### 16. `RDynamicTier4Assessment` can't produce a valid RBC
- **Where:** `atManageTier.c` `RDynamicTier4Assessment`, `writeDynTier4inputfile`, `setRBC`
- **Problems:**
  - The writer outputs the catch columns twice and no CPUE, and `t` is never incremented.
  - `setRBC` sets `d0 = 0; // #TODO REPLACE THIS`.
  - The working directory is changed into `DynTier4_sim_*` and never changed back, so later output files go to the wrong place.
  - `RAssessRscriptName[1]` is only allocated when `UsingRAssess` or `useMultispAssess` is set.
- **Note:** before this work, `dyntier4` also quit in `Tier_averages`, which had no case for it, so this path never ran in a past simulation. I've added `dyntier4` and `tier14` to that switch, which is needed for the new tiertype 5.
- **Proposed fix:**
  - write the CPUE matrix (−1 = missing) and the ctl keys read by `runDynTier4onRTMB.R`;
  - `chdir` back;
  - parse the RBC;
  - guard the script name;
  - ship a corrected `runDynTier4onRTMB.R`, which fixes the R-side bugs: the `Icue`/`Icpue` loop, the ramp denominator and the `logr` map typo.

  This would be the route for ProcessError = 2 (random effects).
- **Effect on past runs:** none.
- **Decision:** ☐ Fix ☐ Leave ☐ Modify: Fix
- **Status (2026-11-13): done.**
  - The writer uses the `DT4_` parameters (the `mgt_tier4_` fields were never set), writes total removals and CPUE (−1 = missing or fleet not used), uses data-year indices 0..year, and adds `# Process error` and `# Hockey stick alpha beta`.
  - New input files are opened for each assessment (before, closed file pointers were reused), and the code changes back to the original directory afterwards.
  - The script is `RAssessRscriptName1` if one is given, otherwise `runDynTier4onRTMB.R` in the run directory.
  - `setRBC` reads the RBC.
  - The corrected script is `atmanage/Rscripts/runDynTier4onRTMB.R`. Besides the listed fixes, it now passes `eval.max`/`iter.max` to `nlminb`; the original gave them to `MakeADFun`, which ignores them, so fits with process error stopped at 200 evaluations.
  - Checked on simulated data against the native C version: RBC 154.0 vs 153.8 with no process error, and 146.0 vs 145.4 with option 1.
  - Dropped: the unused `PropClosed` retro-catch branch (its fields were never set).

---

## D. Stock Synthesis (tier 1 with `UseSS`) linkage

### 17. `Run_SS3` frees a stack buffer, and the readers look in the wrong folder
- **Where:** `atmanage/atSS3assess.c:176-200`; `atSS3Link/atSS3LinkIO.c:36, 204, 302`
- **Problems:**
  - `ss3_abs = realpath(bm->ss3Name, buf)` returns `buf`, which is on the stack. `free(ss3_abs)` then frees stack memory, which is undefined behaviour and can crash.
  - After `chdir(cwd)`, `Read_SS3_Par_File` and `Read_SS3_Report_File` open bare `"ss3.par"` and `"Report.sso"` in the original directory, not the run folder. Their folder argument is unused.
  - A failed `fopen` is logged, but `fgets` is then called on NULL.
  - `ss3Name` is only read when `useMultispAssess` is set (`atassessParamIO.c:1156`).
- **Proposed fix:**
  - remove the `free`;
  - build `dirName/ss3.par` and `dirName/Report.sso` paths;
  - return or flag `AssessFail` when the open fails;
  - read `ss3Name` whenever `UseSS` is set.
- **Effect on past runs:** none intended. The current behaviour either crashes or reads stale files from the run directory.
- **Decision:** ☐ Fix ☐ Leave ☐ Modify: Fix
- **Status (2026-11-13): done.** The readers open `folderName/file` (falling back to the bare name for the legacy caller, which has already changed into the folder), return straight away if the open fails, and close their files. The stack `free` is removed. `ss3Name` is converted and read when `UseSS` or `useMultispAssess` is set.

---

## E. Parameter input

### 18. Duplicate XML tags
- **Where:** `atlantisUtil/atUtilXML.c`
  - `FLAG_REGIMEYR` is used for both `Regime_year_id` and `Regime_year_assess_id`.
  - `FLAG_NUMCHANGELAM` is used for `NumChangeLambda_id`, `num_enviro_obs_id` and `num_growth_morphs_id`.
- **Problem:** the converter and the reader find nodes by tag, so these parameters share one node. Both `Regime_year` values become whichever was written last, and the same happens for the three counts.
- **Proposed fix:** give each a unique tag (`FLAG_REGIMEYRASSESS`, `FLAG_NUMENVIROOBS`, `FLAG_NUMGROWTHMORPHS`). Old prm files still convert because the regexes are unchanged.
- **Effect on past runs:** changes results where the duplicated parameters had different values in the prm file.
- **Decision:** ☐ Fix ☐ Leave ☐ Modify: applied

### 19. Some regexes aren't anchored
- **Where:** about 30 entries in `RBCSpeciesParamStructArray`, e.g. `"MaxAge_"`, `"Nlen_"`, `"SigmaR1_"`, `"Regime_year_"`, `"MultispAssessType_"`.
- **Problem:**
  - `"Regime_year_"` also matches `Regime_year_assess_XXX` lines. The species-code extraction then gets `assess_XXX`, which isn't a group, so it's silently skipped. This is harmless today but fragile.
  - Any future name that contains one of these strings will clash.
- **Proposed fix:** prefix each with `^`.
- **Effect on past runs:** none expected. I'll diff the converted XML before and after to confirm.
- **Decision:** ☐ Fix ☐ Leave ☐ Modify: Fix
- **Status (2026-11-13): done.** All 32 unanchored regexes are now anchored. I checked 40 assessment prm files on this machine. The only token that matched other than at its start was `SigmaR1_` inside `PSigmaR1_XXX`, which gave an invalid group code (`_XXX`) that was skipped. So no converted values change.

### 20. Fractional settings cast to int
- **Where:** `atassess/atassessParamIO.c:977` (`myTACbuffer`) and `:1148` (`RBC_TACchange_min` → `min_TACchange`)
- **Problem:** `(int)` truncates 0.05 or 0.1 to 0, so the MYTAC buffer and the minimum-change rule are silently switched off.
- **Proposed fix:** read both as double.
- **Effect on past runs:** changes results for runs that set fractional values, which were effectively ignored until now.
- **Decision:** ☐ Fix ☐ Leave ☐ Modify: Fix
- **Status (2026-11-13): done.** Both are read as double (the struct fields were already double).

---

## F. Data generation

### 21. CPUE random walk reads index −1 in year 0
- **Where:** `atSS3Link/atSS3DataGen.c:615` (`CPUEdevs[f][r][iyr-1]`)
- **Problem:** this reads out of range if `GenData` is ever called with year index 0. Normally the first call is at year index ≥ 1, so this is a guard rather than a live bug.
- **Proposed fix:** use 0 for the previous deviation when `iyr == 0`.
- **Effect on past runs:** none expected.
- **Decision:** ☐ Fix ☐ Leave ☐ Modify: Fix
- **Status (2026-11-13): done.** The previous deviation is taken as 0 when `iyr == 0`.

---

## G. Found while fixing

### 22. `TierThree_CCsel`: Fcurr averages over one fleet too few
- **Where:** `atManageTier.c`, end of `TierThree_CCsel` (`for (i = 0; i < nF; i++)`, then `sumF/(nF + small_num)`)
- **Problem:** `nF` is the index of the last fleet fitted (it starts at −1), not the number of fleets.
  - With one fleet, `nF` = 0, so Fcurr = 0.
  - With n fleets, the last fleet is left out and the sum is divided by n − 1.
- **Proposed fix:** loop `i < nF + 1` and divide by `nF + 1`.
- **Effect on past runs:** changes results for tier 3 with `Tier3_Fcalc` 2.
- **Decision:** ☐ Fix ☐ Leave ☐ Modify: Fix
- **Status (2026-11-14): done.** A new count `nFit = nF + 1` is used for the F average and for combining the selectivity vectors.

### 23. Classical assessments use the same faulty `Amoeba`
- **Where:** `atassess/atsolve.c` (`Amoeba`), called from `atclassical.c:228, 456` and `atsolve.c:595`
- **Problem:** the same `mpts = ndim` and best-point-in-row-1 issues as item 14. Left unchanged so classical assessment results stay as they are.
- **Proposed fix:** apply the item 14 corrections to `Amoeba` itself, or point those callers at the new routine.
- **Effect on past runs:** changes results for the classical (non-tier) assessments.
- **Decision:** ☐ Fix ☐ Leave ☐ Modify: applied (I think, please check)
- **Checked 2026-11-14:** yes, all of the item 23 changes are on disk (`atsolve.c`, `atclassical.c`, `atsamplesetup.c` and `downhill_simplex` in `atManageTier.c`); nothing was lost in the VS Code crash.
- **Status (2026-11-13, later): done, as you asked ("fix Amoeba for all cases").**
  - `Amoeba` (`atassess/atsolve.c`) now follows Numerical Recipes. It uses all ndim+1 vertices, returns the best point in row 0 with `ilow` = 0 whether or not it converges, and recalculates `psum` correctly after a contraction.
  - All callers now set up ndim+1 vertices: both `atclassical.c` assessments, and `Linear_Regression`. Before, the last parameter was never perturbed.
  - `P` and `Y` in `atsamplesetup.c` gained the extra row.
  - The q-trend regression in the production model (`Prod_Calc`, q_dynamic) could never have run: it was passed the production model's `ndim` instead of 2, and its simplex array `PP` was never allocated. It now uses 2 parameters, `PP` is allocated and freed, and `YY` has at least 3 entries.
  - `downhill_simplex` calls the fixed `Amoeba` again, so the separate tier copy added under item 14 has been removed.
  - Tested on known functions with 2–5 parameters: it finds the exact minimum.
  - **This changes results for the classical (non-tier) sampling assessments.**

### 24. `TierFive_SP`: K is set equal to Bstart
- **Where:** `TierFive_SP` and `SurplusProduction`
- **Problem:** both use `xpar[0]` (and `simplex[0][0]`) for Bstart and K. `xpar[1]` is estimated but never used, so the model assumes the stock starts at K. That may be intended, but the third parameter is then wasted.
- **Decision:** ☐ Fix ☐ Leave ☐ Modify: fix but add note about original handling in case I need to switch back
- **Status (2026-11-14): done.** K is now its own estimated parameter: `simplex[0][1]` / `xpar[1]`, starting at `Tier5_Bstart[0]` like Bstart. The original handling is noted in comments in `TierFive_SP` and `SurplusProduction`; to switch back, set `in_K = xpar[0]` and use `simplex[0][0]` for k. Note that K has no bounds check (only Bstart and r are checked against `Tier5_Bstart`/`Tier5_r`).

### 25. Post-assessment CPUE rule uses region 0
- **Where:** `DoTierAssessment`, `PostRule_id` block (`CPUEgen[CPUEfleet][0][...]`)
- **Problem:** the same issue as item 12. For multi-region assessments only region 0 is used.
- **Proposed fix:** use index `Nregions`.
- **Decision:** ☐ Fix ☐ Leave ☐ Modify: Fix
- **Status (2026-11-14): done.** The CPUE ratio now uses index `Nregions`.

### 26. Surplus production model subtracts removals inside the production term
- **Where:** `SurplusProduction` (`atManageTier.c`), biomass update:
  ```c
  step1 = (1.0 - exp(p * log(bt/in_K))) - T5_removal[iy-1];
  expectB[iy] = bt + bt * in_r / p * step1;
  ```
- **Problem:** the removals are multiplied by `bt * r / p`, so they enter as a fraction of production rather than as tonnes. The usual Pella-Tomlinson form is `B[t+1] = B + (r/p)·B·(1 − (B/K)^p) − C`.
  - Example: with B = 1000, r = 0.4, p = 1 and C = 100, it gives B[t+1] = 1000 + 400 × (1 − 1000/K − 100), a hugely negative number that is then floored at 75.
- **Proposed fix:** `expectB[iy] = bt + bt * in_r / p * (1.0 - pow(bt/in_K, p)) - T5_removal[iy-1];` with a comment noting the original.
- **Effect on past runs:** changes results for tier 5 with tiertype 1 (already changed by items 14, 23 and 24).
- **Decision:** Fix (approved 2026-11-14)
- **Status (2026-11-14): done.** The removals are now subtracted after the production term. The original equation is kept in a comment in `SurplusProduction` in case you need to switch back.

## H. Year-to-year check of multi-year TACs, rollover, buffers and meta-rules (2026-11-14)

I traced an assessment year followed by rollover years after item 7. Three things in my own changes needed correcting, and I've done them:
- **Trigger check:** now compares catch with the TAC (see item 7).
- **`mgt_rbcYear`:** now updated every year (see item 8).
- **Staleness buffer base RBC:** it was stored before averaging and before the tier buffer (`Tier1-3Sig`), and also after a failed assessment. With a tier buffer below 1, the TAC could therefore jump up in the first rollover year. It's now stored after averaging and buffers, and only after a successful assessment.

These now work consistently from year to year:
- **Multi-year TAC rollover without staleness:** the TAC rolls over from last year's TAC, with no second buffer or discard removal. It decays by `(1 - myTACbuffer)` per year, then goes through the change limits and minimum catch.
- **Rollover with staleness:** buffered base RBC × max(`Buffer_min`, 1 − years × rate), then discards are removed, as in the assessment year.
- **`RBC_by_year`:** carries the assessed RBC through rollover years, so the indicator delta-RBC is 1 when nothing has been reassessed. `TAC_by_year` records what was actually set.

Four points need your decision:

### 27. RBC averaging is not what Ratpack does
- **Ratpack** (`ratpackmse/Assessment.cpp:116-124`): each assessment fills `RBC_by_year` with its projected RBCs for future years (from `rbcYear` onwards). The averaging then smooths **the next `RBCaverage` years of that projection** and uses the average for each of those years.
- **Atlantis now** (item 9): a moving average of **this year's RBC and the previous assessments' RBCs**. That is a different rule; it damps changes between assessments.
- **Why they differ:** apart from SS3 forecasts and dynamic Tier 4, Atlantis tiers produce only one RBC, not a projection, so Ratpack's version can't be reproduced for most tiers. For tiers that give a constant projection, such as Ratpack Tier 4, Ratpack's averaging changes nothing.
- **Options:**
  - (a) keep the current moving average and document it as an Atlantis rule;
  - (b) make dynamic Tier 4 (C and R versions) and SS3 store their projected RBCs, and average forward as Ratpack does, with no averaging for single-RBC tiers;
  - (c) offer both behind a flag.
- **Decision:** ☐ (a) ☐ (b) ☐ (c) ☐ Modify: Do c please
- **Status (2026-11-19): done.** The existing per-species flag `UseRBCAveraging_XX` now picks the rule, so no new parameter is needed:
  - **0:** no averaging.
  - **1:** the Atlantis moving average (unchanged, `Averaged_RBC`).
  - **2:** the Ratpack rule (new `Ratpack_Averaged_RBC`). The RBC becomes the average of the first `nRBCaverage` years of this assessment's projected RBCs. Tiers that give a single RBC are left unchanged, as Ratpack would treat them as a constant projection.
  - Any other value stops the run with an error message.
- **Where the projections come from:**
  - **C dynamic Tier 4 (tiertype 5):** `RatpackDynTier4Assessment` now projects forward `nRBCaverage` years with the same harvest control rule, as Ratpack's `runDynTier4` does: catch = RBC, `B[t+1] = B + rB(1 − (B/K)^z) − C`, and B is floored at 1. The first year is still the RBC. With an ACLIM rule (`HCRtype` > 0), each projected year writes an ACLIM line to the log.
  - **R dynamic Tier 4 (tiertype 1):** `setRBC` reads the `Catch projection` block that `runDynTier4onRTMB.R` already writes to `dynTier4.rep`.
  - **SS3 (tier 1 with `UseSS`):** `Read_SS3_Report_File` keeps reading rows after the first `FORECAST:_With_F_to_match_adjusted_catch` row and sums `Retain_B` over fleets for each year, as it does for the first row. It stops at the first row that doesn't start with a number. **Not tested:** I had no SS3.30 `Report.sso` with this table to check against. If only one row is found, nothing is averaged.
- **Where the projection is kept:** a new array `RBC_proj` (and count `nRBC_proj`) in the species RBC structure, capped at 100 years (`MAX_RBC_PROJ`). It is cleared at the start of every assessment year. It's separate from `RBC_by_year`, so rollover years can't overwrite it.
- **Not covered:** PGMSY (`atRlinkRBC.c`). It now averages only with option 1; option 2 gives no averaging there. Its R output does have projection rows, so it could be added if you need it.
- **Effect on past runs:** none. Options 0 and 1 behave as before.
- Also changed: `atlantisboxmodel.h` (constants, new fields), `atAssessTierSetup.c` (allocate/free), and the XML description of `UseRBCAveraging` in `atAssesstoXML.c`.

### 28. `min_TACchange` can cancel the multi-year TAC decay
- **Where:** `DoTierAssessment`, minimum-change rule.
- **Problem:** in rollover years the yearly decay (`myTACbuffer`, or the staleness rate) is usually small. If it's smaller than `RBC_TACchange_min`, the TAC is reset to `TAC_old` every year, so the decay never happens.
  - Example: `myTACbuffer` 0.05 and `RBC_TACchange_min` 0.1 give a flat TAC.
- **Proposed fix:** don't apply the minimum-change rule in rollover years.
- **Decision:** ☐ Fix ☐ Leave ☐ Modify: Fix - do not apply minimum-change in rollover years
- **Status (2026-11-19): done.** The `min_TACchange` rule is now skipped when the effective tier is `sp_rollover`. The change limits (`prop_incTAC`/`prop_decTAC`) and `MinCatch` still apply in rollover years.
- **Side effect to be aware of:** this covers all rollover years, not only those with a decay. With `PostRule` > 0 (item 30, which compounds by design), small yearly CPUE adjustments in rollover years used to be cancelled by the minimum-change rule. Now they go through. If you only want the exemption when a decay is active (`myTACbuffer` > 0 or `Buffer_stale_rate` > 0), that's a one-line change.

### 29. `myTACbuffer` only works when `UseTierBuffers` = 1
- **Where:** the decay is applied inside `Apply_RBC_Buffers`, which only runs when `UseTierBuffers` is on. So turning on the multi-year TAC decay also turns on the tier buffers (`Tier1-3Sig`), and vice versa.
- **Proposed fix:** apply the `myTACbuffer` decay in rollover years whether or not `UseTierBuffers` is set. The staleness buffer is already independent of it.
- **Decision:** ☐ Fix ☐ Leave ☐ Modify: Fix. This is a new option though so check whether it will work with the other kinds of TAC setting when not using UseTierBuffers and the code in the 
  - *(Your note is cut off after "and the code in the". Please finish it if there was something else to check.)*
- **Status (2026-11-19): done.** The decay is now a separate routine, `Apply_MYTAC_Decay`. `DoTierAssessment` calls it in every rollover year, whether or not `UseTierBuffers` is set. The `sp_rollover` case in `Apply_RBC_Buffers` now does nothing, so the decay can't be applied twice.
- **How it works with the other ways a TAC is set (checked):**
  - **Multi-year TAC rollover:** decays by `(1 − myTACbuffer)` in each year after the assessment year (years 1 to period−1). It isn't applied in the assessment year itself.
  - **Trigger rollover outside a multi-year TAC:** no decay, because years since assessment is 0. This is the same as before.
  - **Ratpack staleness buffer (`Buffer_stale_rate` > 0):** the decay is skipped and the staleness buffer is used instead. This is the same as before.
  - **`myTACbuffer` = 0 (the default):** nothing happens, so past runs are unchanged unless they set `myTACbuffer` > 0 with `UseTierBuffers` = 0.
  - **Failed assessment, tier 0, PGMSY and multispecies adjustments:** not affected. Failed assessments and tier 0 aren't rollover years, and PGMSY and multispecies adjustments act after `DoTierAssessment`.
  - **Change limits, `MinCatch` and `PostRule`:** still applied after the decay. The minimum-change rule no longer cancels it (item 28).
- **Placement:** the decay is applied before the RBC that becomes the TAC is read, so it does reach the TAC. Before, it didn't, because of the problem described in item 31.

### 30. Post-assessment CPUE rule is applied again in every rollover year
- **Where:** `DoTierAssessment`, `PostRule_id` block.
- **Problem:** with `PostRule` > 0, the rolled-over TAC is adjusted by the latest CPUE ratio every year, so during a multi-year TAC the adjustments compound. This may be intended (an annual CPUE check between assessments), but it means a multi-year TAC is not fixed.
- **Proposed fix (if not intended):** apply the post-assessment rule only in assessment years.
- **Decision:** ☐ Fix ☐ Leave ☐ Modify: Leave it is supposed to be compounding
- **Status (2026-11-19): left as is.** No code change. See the side effect noted under item 28.

### 31. Tier buffers (`Tier1-3Sig`) never reach the TAC (found while doing item 29)
- **Where:** `DoTierAssessment`. The local `RBC` is read from `RBCest` *before* `Apply_RBC_Buffers` is called (also true in the committed version). The discard removal and every TAC step after it use that local `RBC`. So the buffered `RBCest` is never used to set the TAC.
- **Effects:**
  - With `UseTierBuffers` = 1, the `Tier1Sig`/`Tier2Sig`/`Tier3Sig` scalars change only `RBCest` (used for reporting), not the TAC.
  - Before item 29, this also meant the `myTACbuffer` decay never reached the TAC.
  - The Ratpack staleness base RBC (Section H) is stored *after* the buffers, but the assessment-year TAC is set from the *unbuffered* RBC. So with a tier buffer below 1, the TAC drops in the first rollover year rather than jumping up.
- **Proposed fix:** set the TAC from the buffered value: re-read `RBC = RBCest` after `Apply_RBC_Buffers`. Keep `RBC_by_year` as the unbuffered RBC, as its comment says.
- **Effect on past runs:** changes results for runs with `UseTierBuffers` = 1 and any `TierNSig` ≠ 1.
- **Decision (2026-11-19):** Modify. The RBC is the estimate. Buffers are applied after it to get to the TAC. In rollover years, the staleness penalty applies to both: (a) the TAC path, to give the new TAC the fleets respond to, and (b) the RBC, to give the `RBC_by_year` value that is stored.
- **Status (2026-11-19): done** (`atManageTier.c`; new field in `atlantisboxmodel.h`; initialised in `atAssessTierSetup.c`).
  - **RBC stays the estimate:** `RBCest` and the local `RBC` are no longer changed by any buffer. They are what `RBC_by_year`, the per-fleet RBC and the results file report.
  - **TAC path (`TACbase`):** starts as the RBC. The `myTACbuffer` decay and the tier buffers act on `TACbase`. `Apply_RBC_Buffers` and `Apply_MYTAC_Decay` now return the buffered value instead of overwriting `RBCest`. The TAC is calculated from `TACbase` (discards removed, then `PostRule`, the change limits and `MinCatch`). A log line records each buffer step.
  - **Rollover years, the RBC:** the RBC from the last successful assessment (new field `lastAssessRBC`), not last year's TAC (which already has buffers in it). With the staleness buffer it is stale × that RBC. If there has been no successful assessment yet, last year's RBC is carried forward.
  - **Rollover years, the TAC path:**
    - With the staleness buffer: stale × the last assessment's buffered value (`Ratpack_baseRBC`), then discards removed. As the buffers are multipliers, this is the same as reapplying them to the staleness-adjusted RBC.
    - Without it: last year's TAC, then the `myTACbuffer` decay.
  - **Staleness doesn't compound:** it is always measured from the last assessment, which is why the unbuffered RBC is kept separately in `lastAssessRBC`.
- **Effect on past runs:**
  - **`UseTierBuffers` = 1 with any `TierNSig` ≠ 1:** the TAC is now buffered, and the reported RBC is no longer buffered.
  - **Rollover years:** the reported RBC (`RBCest`) is now the last assessed RBC, not last year's TAC.
  - **Staleness buffer runs:** `RBC_by_year` now falls during rollover years, so indicator species delta-RBC (`CalculateNonIndicatorRBC`) is no longer 1 in those years.

## Not changed, as you requested
- `TierOneMimic`'s original discontinuous rule (`depgen < 0.35 → F40*(depgen/0.2-1)`, else `F48`) is untouched. The continuous ramp and the ACLIM rule are available through `Tier1_HCRoption`.
- The `DynamicTier4Assessment` stub is untouched. The native C version is a separate function, selected by tiertype 5.
