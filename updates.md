# Atlantis optimisation updates

Last updated: 2026-11-13 (UTC+11)

## Tier assessment bug fixes (2026-11-13)

These are the fixes approved in `ratpack_port_bug_review.md`; each item's Status line there has the details. **Tiered runs that use these code paths will give different results from before.**

| Item | Fix | Runs affected |
|---|---|---|
| 2, 3 | Rolled-over TACs are no longer buffered again or have discards removed twice. The MYTAC buffer decays once per year. | `myTACperiod` > 1, or trigger management |
| 4 | Discard rate, `MinCatch`, the tier buffers (`Tier1-3Sig`) and `MultispAssessType` are read from the assessment parameters, not the biological ones. | `UseTierBuffers`, `MinCatch`, PGMSY switch-on |
| 8, 9 | `mgt_rbcYear` is now set. RBC averaging uses the last `nRBCaverage` years, and not in rollover years. | `UseRBCAveraging`, trigger or indicator management |
| 7 | `RBC_by_year` now always stores RBCs (it was given the TAC); a new `TAC_by_year` stores TACs. In rollover years the last assessed RBC is carried forward. RBC averaging in the R-linked PGMSY path is fixed to match. | `UseRBCAveraging`, trigger or indicator management |
| 7, 8 follow-up | The trigger catch check compares catch with last year's TAC (as in Ratpack). `mgt_rbcYear` is updated every year. The staleness buffer starts from the RBC after averaging and buffers, from the last successful assessment. | trigger management, `Buffer_stale_rate` |
| 10–12 | Tier 4 CPUE target and recent-average windows corrected, and the all-region CPUE is used (old and new Tier 4). | tier 4 |
| 13 | Tier 3 F ramp corrected. Set `Tier3_replicate_old_XXX 1` for the old equation. | tier 3 or tier 5 with tiertype 2 |
| 14, 23 | `Amoeba` (downhill simplex) corrected for all callers: n+1 vertices and the best point returned in row 0. The tier 3 CCsel and tier 5 surplus-production fits actually fit now, and the production model's q-trend regression can run. | tier 3 `Tier3_Fcalc` 2, tier 5 tiertype 1, and the classical sampling assessments (production model and VPA) |
| 15 | Tier 6 CPUE search counts back through the years. | tier 6 with a CPUE response |
| 22 | Tier 3 CCsel Fcurr averages over all fleets fitted. Before, the last fleet was dropped, so Fcurr = 0 with one fleet. | tier 3 `Tier3_Fcalc` 2 |
| 24 | Tier 5 surplus production estimates K separately. Before, K = Bstart; the original handling is noted in the code comments. | tier 5 tiertype 1 |
| 26 | Tier 5 surplus production: catches are subtracted after the production term (standard Pella-Tomlinson). Before, they were multiplied by `B·r/p`; the original equation is kept in a comment. | tier 5 tiertype 1 |
| 25 | Post-assessment CPUE rule uses the all-region CPUE. | `PostRule` > 0 with more than one region |
| 16 | The R dynamic Tier 4 path (tier 10, tiertype 1) works. Corrected script: `atmanage/Rscripts/runDynTier4onRTMB.R`. | none (this path couldn't run before) |
| 17 | SS3 output files are read from the run folder, and `ss3Name` is read when `UseSS` is set. | tier 1 with SS |
| 19 | Assessment parameter regexes are anchored. | none expected |
| 20 | `myTACbuffer` and `RBC_TACchange_min` are no longer truncated to integers. | runs with fractional values |
| 21 | Guard on the first-year CPUE deviation. | none expected |

## Ratpack assessments and harvest control rules (2026-11-05)

New options ported from André Punt's R Ratpack (`PuntRatpackR`: `MISC HCR/` and `R Inserts/Assessment.R`). All of them are off by default. The new parameters are optional in the assessment prm file, so older prm files convert and run as before.

| Option | How to switch it on | What it does |
|---|---|---|
| Native C dynamic Tier 4 | `tier 10` and `tiertype_XXX 5` | Pella-Tomlinson model fitted to total removals and CPUE, then a hockey-stick, ACLIM or constant-harvest-rate rule gives the RBC. Parameters start with `DT4_`. |
| ACLIM harvest control rules 1–10 | `HCRtype_XXX 1`–`10` | Port of `HCR_ACLIM.R`. Used by dynamic Tier 4, tier 14 and Tier 1 mimic option 2. Parameters start with `HCR_`. |
| Perfect-information ACLIM tier (Ratpack Tier_5) | `tier 14` | Tier 1 mimic depletion (with `Tier1_cv` error) plus the ACLIM rule, applied to F40 or F48 (`HCR_Fref`). |
| Tier 1 mimic rule choice | `Tier1_HCRoption_XXX` | 0 = original rule (unchanged), 1 = continuous ramp from Blim to BrefB with F at BrefA, 2 = ACLIM. |
| Tier 4 Ratpack scaling | `Tier4_UseHCRtarget_XXX 1` | Multiplies the new Tier 4 RBC by `mgt_HCRtarget/0.48`, as Ratpack does. |
| Tier 4 catch cap | `Tier4_UseMaxCatch_XXX 1` and `Tier4_MaxCatch_XXX` | Absolute cap on the Tier 4 RBC, as in Ratpack `Run_Tier4b`. |
| Staleness buffer | `Buffer_stale_rate_XXX` > 0 (and `Buffer_min_XXX`) | In multi-year-TAC or trigger rollover years the RBC is the last assessed RBC × max(`Buffer_min`, 1 − years since assessment × `Buffer_stale_rate`), as in Ratpack `SetBuffer`. |

Checks:
- The C ACLIM rule matches `HCR_ACLIM.R` to within 5e-11 over all 10 types.
- Dynamic Tier 4 matches Ratpack's RTMB model on simulated data. Depletion agrees to 1e-4 with no process error and 3e-5 with process-error option 1.
- The C version conditions on catch instead of fitting it with sd 0.05, and it does not support random effects (option 2).

**Behaviour change: TAC change limits.** The limits in `DoTierAssessment` (`atManageTier.c`) were inverted. A recommended cut raised the TAC by `prop_incTAC`, a recommended rise lowered it by `prop_decTAC`, and the TAC could not stay within the limits. They now work as documented, matching Ratpack:
- Increases are capped at `prop_incTAC`.
- Decreases are capped at `prop_decTAC`.
- Otherwise the post-assessment TAC is used.
- With no previous TAC, neither the limits nor `min_TACchange` apply.

Tiered runs that use `prop_incTAC` and `prop_decTAC` will give different TACs from before.

## Compiler tuning: about 25% faster, exact

No model code changed. Building with `-O3` plus profile-guided optimisation makes the model about 25% faster, with bit-for-bit identical results. Profile-guided optimisation means the compiler first builds a measuring version, runs 30 days once, then rebuilds using the measured branch and call counts.

| Setting | Gain in back-to-back pairs | Kept? |
|---|---|---|
| `-O3` instead of `-O2` | 10% and 19% | yes |
| `-march=native -ffp-contract=off` | -13% and +4% | no |
| Link-time optimisation | +1% and -13% | no |
| Profile-guided, on top of `-O3` | 11% and 12% | yes |

- Every candidate is identical to the baseline over 30 days. The winner is also identical over a full year.
- With 2 threads, the profile-guided OpenMP build ran 23% faster than the previous OpenMP build (4:27 to 3:26), within rounding (max 6e-15).
- The 1-year run took 1:01, but other programs were busy at the time, so only the paired 30-day times show the speed.

To build it (the defaults in `PreRules.am` are unchanged), use `CLEAN_OBJECTS=1` and run `performance/build_candidate.sh` with `BUILD_OPT="-g -O3 -mtune=generic -fprofile-generate"`. Run the result for 30 days, then rebuild with `BUILD_OPT="-g -O3 -mtune=generic -fprofile-use -fprofile-correction"`. For OpenMP, add `-fopenmp` to both builds and train with `OMP_NUM_THREADS=2`. The OpenMP build needs its own profile.

The binaries are `performance/nesp_1y/bin/atlantisMerged_cc_O3_pgo` (serial) and `atlantisMerged_cc_O3_pgo_omp`. Details are in `PERFORMANCE_OPTIMISATION.md`, Experiment 8. The shared executable is unchanged, and nothing is committed.

## Feeding: zero diet links skipped, about 8% faster, exact

78% of the prey-availability calls in `Eat` were for predator-prey pairs whose diet-matrix entry (`pSPVERTeat`) is 0, so they always gave 0. `Calculate_PreyAvail` now returns 0 for those straight away (`atprocess.c`).

- Serial results are bit-for-bit identical to the baseline over 30 days and a full year.
- With 2 threads they stay within rounding (max 4e-15).
- Timed back to back with the unchanged binary: 8% faster serial (5:14 to 4:48, and 6:04 to 5:35) and 9% faster with 2 threads (3:17 to 2:59).
- The machine is slower today than last night, so compare only within a pair.

The GPU was ruled out: the laptop has only integrated graphics, no GPU toolkit is installed, and the feeding code is small, branchy and uses `long double`.

Details are in `PERFORMANCE_OPTIMISATION.md`, Experiment 7. The shared executable is unchanged, and nothing is committed.

## Parallel boxes: working, within rounding

Boxes now run on several threads in an OpenMP build. Set `OMP_NUM_THREADS=2` (or 4) to use it. Without that setting the model runs on 1 thread and gives exactly the old results.


| Run                    | Wall time | Largest relative difference |     |     |
| ---------------------- | --------- | --------------------------- | --- | --- |
| 30 days, serial        | 5:15      | reference                   |     |     |
| 30 days, 1 thread      | 5:09      | 0 (identical)               |     |     |
| 30 days, 2 threads     | 2:36      | 3e-15                       |     |     |
| 2:16                   | 3e-14     | 30 days, 4 threads          |     |     |
| 1 year, serial (quiet) | 48:34     | reference                   |     |     |
| 1 year, 2 threads      | 36:20     | 3e-11                       |     |     |


No value differs by more than 1e-9, the tolerance you chose. Over a year, the gain on this laptop is 1.34x rather than the 2x seen in 30 days. The long run settles at a lower clock with two busy cores, threads 3 and 4 run on the slower E-cores, and spawning runs one box at a time. On a machine with more full-speed cores (such as HPC), the gain should be closer to the 30-day numbers.

Two bugs were found and fixed on the way. The extra thread copied the main thread's state while the main thread was already changing it. That caused differences of up to 6% and made the ThreadSanitizer check hang. Spawning and recruitment also write shared per-species queues, so they now run one box at a time.

The parallel path switches itself off when fisheries, contaminants, ice, atomic-ratio tracking, external populations, diet-check output or debug output are on. NESP uses none of these. Details are in `PERFORMANCE_OPTIMISATION.md`, Experiment 6. The binary is `performance/nesp_1y/bin/atlantisMerged_box_omp_final`. The shared executable other tools use is unchanged. Nothing is committed.

## Earlier: box order changes the results

A 30-day serial run with the middle boxes processed in reverse order (`runs/reverse_probe_30d`) is not identical to `baseline_30d`. All the text diagnostics (mortality, diet, biomass) match exactly. 998 of 3867 netCDF variables differ, at rounding level: the largest relative difference is 3e-11, and none reach 1e-9. The differences start in migratory and age-structured vertebrates (gannets, penguins, shearwaters, sharks, kingfish, territorial demersal fish) and spread to nutrients and plankton.

So the ecology adds up some cross-box totals in box order and uses them. Two threads can only match exactly if every one of those totals is replayed in box order, on top of per-thread copies of about 200 shared scratch values that ThreadSanitizer found. Decision needed: build that (large and uncertain), accept rounding-level differences, or stop.

## Earlier status

`loop_timer` finished at 15:06, exit 0. Of the time spent inside timesteps, the box loop was 2976 seconds and everything else was 51 seconds. That is 98.3%. Migration, physics, and boundaries are in that 51 seconds. The wall clock was 50:29, a little slower than the 48:34 quiet run, because a compile shared the machine at the start. This run is the split, not a new speed result. Outputs match `baseline_rep1` exactly.

`perf` cannot sample processes on this machine. The 30-day `gprof` profile finished in 5 minutes 8 seconds, exit 0. It ranks a normal timestep, not a full year. With `-O2`, some small functions are counted inside their callers.

The time is feeding, inside the box loop:


| Function               | Share of profiled time | Calls       |
| ---------------------- | ---------------------- | ----------- |
| `Eat`                  | 34%                    | 7.4 million |
| `Check_Prey_Is_Finite` | 19%                    | 5.4 million |
| `Avail_Fish`           | 6%                     | 2.8 billion |
| `Adapt_Diff_Method`    | 6%                     | 20 thousand |
| `Get_Gape_Lim_Prey`    | 5%                     | 3.5 billion |


`Eat` is called from vertebrate feeding (`Do_Vertebrate_Living`, about half of the calls) and invertebrate feeding (`Invert_Consumers_Process`, about a third). Transport, vertical mixing, and boundaries are each under 0.2%. `Apply_Q10_Corrections` is 0.1%: the Q10 values must be copied per thread for a correct parallel run, but they are not where the time goes.

## 30-day prey-check test

Skipping `Check_Prey_Is_Finite` did not speed the model up. The unchanged 30-day run took 5:15.09. The run without the scan took 5:15.69. Outputs were identical. `gprof` had charged that function 19% because it is called millions of times; on the normal binary that cost is too small to measure. The check is back in the source. The executable other tools use was not changed.

## Audit of the box loop

The boxes cannot yet run at the same time. Each box clears one shared `boxLayerInfo`, then `Parameter_Q10` rewrites the live growth and mortality parameters for every group. The next box reads those rewritten values. Two boxes at once would mix their temperatures into the same parameters.

What each thread would need:

- Its own `boxLayerInfo`.
- Its own copy of the layer scalars (`H2Otemp`, salinity, pH, the Q10 rates `r_DL`, `r_DR`, and the related rates).
- Its own copy of the live parameter slots that Q10 overwrites (`scaled_mum`, `KI`, `E1` to `E4`, `mL`, `mQ`, and the per-step growth and mortality slots).

What stays shared:

- The tracer arrays and the original `*_T15` parameters.
- Age structure, `boxPopRatio`, and recruitment. Those are updated on the first or last box and have to run after the other boxes, on one thread.
- Sums such as `calcTrackedMort`. They add every box into the same cells, so a parallel run can change the last digits of the mortality text even when each box is calculated the same way.

No OpenMP flag is switched on. The next code change is that per-thread working copy, and only if this timer run shows the box loop is at least half of the year.

## Runs kept

- `baseline_rep1`: 52:48.54. Reference for exact output comparison.
- `quiet_O2`: 48:33.56. Warnings off, outputs identical to the baseline.
- `loop_timer`: running.
- `baseline_rep2` was stopped on purpose. It is not a complete run.
- The shared NESP model files were not edited.

