# Atlantis performance optimisation log

Numerical results must stay the same. A change is accepted only after a timed run of the same NESP one-year case and a numerical comparison against the archived baseline.

## ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ ##
## ~ Baseline case ~ ##
## ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ ##

| Item | Value |
|---|---|
| Executable | `performance/nesp_1y/bin/atlantisMerged` (frozen copy) |
| SHA256 | `8fc1952e399e86df99ab7f3254aecd7116215d1a9aefa1d93c007c65bed82252` |
| Git commit | `16792e4001a2dc563a97296cc00c3598dd7403d3` |
| Compiler | gcc/g++ 11.4.0 |
| Flags actually used to build the C objects | `-g -O2 -std=c99 -mtune=generic -march=x86-64` |
| Flags in `atlantis/PreRules.am` today | `-g -O2 -mtune=generic` |
| Machine | `seafood-hf`, Intel Core Ultra 7 165U, 14 threads |
| Case | NESP biology, fisheries off, economics off |
| Inputs | `performance/nesp_1y/model/` (real copy, gitignored) |
| Cadence | `dt` 12 hour, `tstop` 365 day, `toutinc` 7 day |
| Domain | 147 boxes, 7 layers |

A plain `make` from the current `PreRules.am` would rebuild the C core without `-O2`. The frozen binary is the baseline until a rebuild is regression-tested.

Other tools are using the shared model and the shared executable. Do not edit the NESP files under `Projects/NESP_ParksAustralia`, and do not leave a rebuilt `atlantis/atlantismain/atlantisMerged` in place. Experiments use `performance/nesp_1y/model/`. A candidate build goes through `performance/build_candidate.sh`, which saves the new binary under `performance/nesp_1y/bin/` and copies the frozen baseline back over the shared path.

`perf` cannot be used on this machine: `kernel.perf_event_paranoid` is 4 and `sudo` requires a password. Function-level timing will use a separate `gprof` (`-pg`) build after the numerical baseline exists. That profiling binary is not the baseline executable.

## ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ ##
## ~ Experiments ~ ##
## ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ ##

The timing reference is `baseline_rep1`. Further identical repeats were stopped on purpose. `baseline_rep2` is incomplete and is not used.

### 0. Establish the baseline

Optimisation: none. Freeze the binary, archive the live run, time one complete repeat.

Reason: later speedups are measured against this run, and outputs are checked for exact equality.

Files/functions modified: none in the model source. Added `performance/compare_atlantis_outputs.py`, `performance/run_nesp_timed.sh`, `performance/establish_baseline.sh`, and gitignore rules for the model and runs.

Expected performance benefit: none.

Baseline runtime: wall 52:48.54, user 3164.37 s, system 1.80 s, 99% CPU.

Candidate runtime: not applicable.

Speed improvement: not applicable.

Memory change: maximum resident set 580352 kB.

Output differences: every NetCDF variable present in both the archived live run and `baseline_rep1` had `n_differ=0`. The archive is missing some files the live cleanup deleted (`outputTOT.nc`, `outputPROD.nc`, and several mortality texts). Later candidates are compared with `baseline_rep1`.

Accepted/rejected: accepted as the reference run.

Notes: the run is one-core and CPU-bound. System time is 1.8 s, so disk is not the limit. stderr held 12122 `WARNING:` lines (1.4 MB).

### 1. Silence runtime warnings (`-Q`)

Optimisation: a `-Q` flag skips `warn()` and the nutrient-limitation log lines. The limitation value `hN_sp` is still calculated. The candidate is also a clean rebuild with `-O2 -mtune=generic`, matching the frozen binary's flags.

Reason: the baseline wrote about 12,000 nutrient-limitation warnings. Those messages are not model state.

Files/functions modified: `atlantis/sjwlib/warn.c`, `atlantis/sjwlib/include/sjwlib.h`, `atlantis/atlantismain/atlantismain.c`, the nutrient-limit branch in `atlantis/atecology/atprocess.c`, and `OPT` in `atlantis/PreRules.am`.

Expected performance benefit: small. The warning text is about 2 MB.

Baseline runtime: wall 52:48.54, user 3164.37 s.

Candidate runtime: wall 48:33.56, user 2909.98 s, system 1.53 s, 99% CPU. Binary `performance/nesp_1y/bin/atlantisMerged_quiet_O2`.

Speed improvement: 255 s less wall time, about 8%. User time fell by the same amount.

Memory change: maximum resident set 580856 kB, against 580352 kB.

Output differences: none. Comparison with `baseline_rep1` reported `IDENTICAL=True`.

Accepted/rejected: accepted. Outputs match, and the year is shorter. The shared executable was restored to the frozen baseline.

Notes: stderr for this run is 130 bytes and contains no `WARNING:` lines. The log is 0.9 MB smaller. That text is too small, and system time barely changed, so the 4 minutes of user CPU should not be credited to `-Q` alone. The clean `-O2` rebuild is part of the same binary. A later rebuild without `-Q` would separate the two.

### 2. Time the per-box biology loop

Optimisation: none. Add a clock around the box loop in `runNextTimeStep` and print one summary at the end of the year.

Reason: parallel biology is only worth the rewrite if that loop is most of the year.

Files/functions modified: `atlantis/atlantismain/atlantismain.c`. Binary `performance/nesp_1y/bin/atlantisMerged_loop_timer`. The shared executable was restored.

Expected performance benefit: none. The clock does not change the equations.

Baseline runtime: wall 52:48.54. Current serial reference with `-Q`: wall 48:33.56.

Candidate runtime: wall 50:29.09, user 3024.75 s, system 1.61 s, 99% CPU. Maximum resident set 580704 kB. A `gprof` rebuild shared the machine for the first minute, so this wall time is not a speed comparison with `quiet_O2`.

Speed improvement: none expected. Inside the timesteps, the box loop took 2976.18 s and the rest of each timestep took 50.73 s. That is 98.3% of timestep time.

Memory change: 580704 kB, against 580856 kB for `quiet_O2`.

Output differences: none. Comparison with `baseline_rep1` reported `IDENTICAL=True`.

Accepted/rejected: the measurement is accepted. The gate for a parallel box loop is passed, because the loop is almost all of the year. The parallel code is still blocked on the per-thread Q10 working copy from the audit.

Notes: the summary lines are `BOX_LOOP_SECONDS`, `TIMESTEP_REST_SECONDS`, and `BOX_LOOP_FRACTION`. Startup and the final output write are outside those totals. Migration, physics, and boundaries are inside the 50.73 s.

## ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ ##
## ~ Audit of the box loop ~ ##
## ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ ##

The loop is in `runNextTimeStep` in `atlantis/atlantismain/atlantismain.c`. Water boxes call `Ecology_Box_Biology`. Land boxes call `Ecology_Land_Biology_Process`. Boundary boxes are skipped, except one external-population call. Migration, physics, and boundaries are outside the loop and must stay on one core. Fisheries are off in this NESP case, so the habitat-overlap call inside the loop does not run.

A second thread cannot be turned on yet. The boxes do not only share `boxLayerInfo`. Each box rewrites live parameters that the next box then reads.

### Per-box scratch

These values are rewritten for the box or layer being processed. Two threads need two copies.

- `boxLayerInfo`, allocated once in `atbiolsetup.c` and cleared at the start of every box in `Box_Bio_Process`.
- Layer scalars in `atbiologyPrivate.h`, set from the current cell in `Properties_At_Depth` (`atlantis/atecology/atbiophysics.c`): `H2Otemp`, `current_SALT`, `current_PH`, `current_WIND`, layer thicknesses, areas, and the other depth scalars next to them.
- Q10 rates rewritten at the end of `Apply_Q10_Corrections` in `atlantis/atecology/atq10.c`: `r_DL`, `r_DR`, `r_DC`, `r_DON`, `r_DSi`, `K_nit`, `R_0`, `R_D`. The reference values (`r_DL_T15` and the other `*_T15` rates) are read and should stay shared.
- Fields on the model struct written for the current box: `current_box`, `current_layer`, `current_corr`, `Tcorr`, `temp_influence`.
- Fields on `FunctGroupArray` rewritten for every group by `Parameter_Q10`: `Tcorr`, `TcorrEff`, `Scorr`, `pHcorr`, `PolluteCorr`, `scaled_mum`, `scaled_C`.
- Live parameter slots overwritten from the `*_T15` slots in the same function: `speciesParams` entries such as `KI`, `E1` to `E4`, and `mS`, and `cohortSpeciesParams` entries `mL` and `mQ`. The `*_T15` slots stay shared.
- Per-step result slots on `FunctGroupArray`: `growth`, `mortality`, `uptakeNH`, `uptakeDL`, `uptakeDR`, `releaseNH`, `prodnDL`, `prodnDR`, `grazing`, `lysis`.

`FunctGroupArray` itself must not be made thread-private. It also holds state that has to remain shared, including `boxPopRatio`, tracer index lists, and the original `mum` and `*_T15` parameters.

### Shared state indexed by box

Tracer arrays and `bm->boxes[b]` are the model state. A thread may write the box it is processing. It must not write another box in the same timestep.

### Cross-box updates

These run inside the box loop but use more than the current box. They have to finish after the other boxes, still on one thread.

- `atlantis/atecology/atdemography.c` has 26 checks for the first or last box. The last box updates age structure for a normal group (`Update_Age_Distrib`), writes `boxPopRatio` for every box, stores recruitment diagnostics, and advances the spawn queue. The first box handles external reproducers and some spawn bookkeeping.
- Sums that are not indexed by box. `bm->calcTrackedMort[...] +=` and `tot_yoy[...] +=` add every box into the same cells. A parallel sum changes the order of the additions. Mortality text can then differ in the last digits even when each box's own arithmetic is unchanged. `output.nc` is the test that matters for the biology. If that file differs, the parallel run is rejected.

### What this means for the next code change

Copying `boxLayerInfo` is not enough. Each thread also needs its own Q10 working copy: the scratch scalars, the live parameter slots, and the per-step growth and mortality slots. The original parameters and the tracer arrays stay shared. The first-box and last-box steps stay after the other boxes.

That is a wide edit through `atq10.c` and the biology files that read those live slots. `loop_timer` showed the box loop is 98.3% of timestep time, so this is the right place to parallelise. No OpenMP flag goes on until a one-thread run of that working copy matches `baseline_rep1`.

### 3. Function profile, 30 days

Optimisation: none. A separate binary compiled with `-O2 -pg`.

Reason: `perf` cannot sample on this machine. The profile ranks functions inside a normal timestep.

Files/functions modified: none kept in the normal build. Binary `performance/nesp_1y/bin/atlantisMerged_gprof`. The shared executable was restored before and after this build.

Candidate runtime: wall 5:08.01 for 30 days, user 307.39 s, exit 0. This is not a full-year speed number. `gprof` instrumentation makes the run slower than the timed binary.

Profile, share of time `gprof` could assign:

- `Eat` 33.7% (7.4 million calls). Callers are `Do_Vertebrate_Living` (3.9 million) and `Invert_Consumers_Process` (2.7 million).
- `Check_Prey_Is_Finite` 19.1%.
- `Avail_Fish` 6.0% (2.8 billion calls), called only from `Eat`.
- `Adapt_Diff_Method` 5.6%. This drives the per-box biology.
- `Get_Gape_Lim_Prey` 5.3% (3.5 billion calls), called only from `Eat`.

`transportBM` is 0.16%. `boundaries` and `vertical_mixing` are about 0.01% each. `Apply_Q10_Corrections` is 0.11%.

Accepted/rejected: accepted as a ranking. The bottleneck is prey checking inside `Eat`, and that work sits in the box loop. A parallel box loop speeds that up across cores. A later single-core change would look at `Avail_Fish` and `Get_Gape_Lim_Prey`, which are called billions of times.

### 4. Skip the finite-prey scan

Optimisation: return immediately from `Check_Prey_Is_Finite` unless prey-biology debug is on. The scan only logs non-finite prey.

Reason: `gprof` assigned it 19% of a 30-day profile.

Files/functions modified: `atlantis/atecology/atecology.c`, then reverted after the test. Binary `performance/nesp_1y/bin/atlantisMerged_nofinite`. The shared executable was restored.

Baseline runtime: 30 days with `atlantisMerged_quiet_O2`, wall 5:15.09, user 314.27 s.

Candidate runtime: wall 5:15.69, user 314.77 s.

Speed improvement: none.

Output differences: none. `IDENTICAL=True` against the 30-day reference.

Accepted/rejected: rejected. The source change was reverted. `gprof` inflated a function that is called millions of times. On the normal `-O2` binary the scan is not a measurable part of the 5 minutes.

## Experiment 5: does box order matter? (probe for parallel boxes)

Optimisation: none. This is a probe for running boxes on more than one core. The middle boxes (strictly between the first and last water box) were run serially in reverse order.

Reason: parallel threads finish boxes in a different order. If the order changes the results, a parallel run cannot be identical without replaying cross-box sums in box order.

Files/functions modified: `atlantis/atlantismain/atlantismain.c` (box body moved to `runBoxBiology`, loop split into first / middle / last stages, OpenMP path and the probe kept behind `#ifdef`s that are off in normal builds). `atlantis/atecology/atbiology.c` and `atEcologyLib.h` got thread helper stubs. `performance/build_candidate.sh` now takes `BUILD_OPT` and `CLEAN_OBJECTS=1`. Binary `performance/nesp_1y/bin/atlantisMerged_reverse_probe`. The shared executable was restored.

ThreadSanitizer, 2 threads, 1 day: about 200 shared locations. These include scratch values every box overwrites (layer temperature, salinity and pH, Q10 rates, `boxLayerInfo`, prey and eating arrays, `FunctGroupArray` work fields), cross-box mortality and diet totals, and `EMBRYO` reproduction state.

Output differences: not identical to `baseline_30d`. Text diagnostics are identical. 998 of 3867 netCDF variables differ, with a maximum relative difference of 3.2e-11 and none above 1e-9. The differences start in vertebrates.

Accepted/rejected: probe only, nothing accepted. An exact parallel run needs ordered replay of every cross-box sum the ecology reads, plus per-thread copies of the shared scratch state.

## Experiment 6: run the middle boxes on several threads (OpenMP)

Optimisation: in OpenMP builds, the water boxes strictly between the first and last water box run on a team of threads. The first and last box still run on the main thread, because they trigger whole-model bookkeeping (age distributions, recruitment, spawning queues). The number of threads comes from `OMP_NUM_THREADS`. When it is not set, the model uses 1 thread and runs the original serial loop.

Reason: the box loop is 97-98% of the timestep time (Experiment 2), and the feeding work inside it is spread over all boxes.

What each extra thread gets, filled from the main thread's values at the start of every parallel stage:

- Its own per-box working values (`H2Otemp`, salinity, pH, Q10 rates `r_DL` ..., flux trackers, `it_count`). These are now thread-local globals (`atBoxWorkspace.h`, `BOX_THREAD_LOCAL`).
- Its own box work arrays (`VERTinfo`, `PREYinfo`, `EATINGinfo`, `GRAZEinfo`, `FEEDinfo`, `boxLayerInfo`, ...). Allocation moved to `Ecology_Allocate_Box_Work_Arrays`.
- Its own copy of `FunctGroupArray`, with its own live parameters (`speciesParams`, `cohortSpeciesParams`) and per-box results (`growth`, `mortality`, `grow`, `preyEaten`, `GrazeLive`, ...).
- Its own copy of the model structure, with its own mortality, diet and catch totals (`calcMnum`, `calcTrackedMort`, `calcTrackedPredMort`, `DIET_check`, `FCcaught`, ...). After the parallel boxes these totals are added into the main totals, thread 1 first, then thread 2, and so on.

Shared and protected:

- Spawning and recruitment (`Vertebrate_Reproduction`, `Invertebrate_Reproduction`) update the per-species queues in `EMBRYO` that all boxes share. They run inside an OpenMP critical section, one box-layer at a time.
- A barrier makes every thread finish copying before any box starts. Without it, the main thread's first box changed values while the other thread was still copying them. That gave differences of up to 6% after 30 days, and a ThreadSanitizer run that never finished.

The parallel path is switched off (serial loop) when fisheries, contaminants, atomic-ratio tracking, ice, external populations, `flagdietcheck`, `debug` or the fish check are on, or when a land box lies between the first and last water box. Those options touch shared state the thread copies do not cover. NESP uses none of them.

Files/functions modified:

- `atlantis/atlantismain/atlantismain.c`: `runBoxBiology`, parallel box loop in `runNextTimeStep`, 1-thread default in `main`, thread-local `FunctGroupArray`.
- `atlantis/atecology/atbiology.c`: `Box_Layer_Reproduction`, thread workspace (`Box_Biology_Can_Run_In_Parallel`, `Box_Biology_Parallel_Begin`, `Box_Biology_Thread_Start`, `Box_Biology_Thread_Model`, `Box_Biology_Thread_End`, `Box_Biology_Parallel_End`).
- `atlantis/atecology/atbiolsetup.c`: `Ecology_Allocate_Box_Work_Arrays`, `Ecology_Get_Box_Work_Arrays`, `Ecology_Set_Box_Work_Arrays`, `Ecology_Copy_Box_Work_Arrays`.
- New header `atlantis/atecology/include/atBoxWorkspace.h`. Declarations made thread-local in `atecology.h`, `atbiology.h`, `atbiologyPrivate.h`, `atlantisboxmodel.h`, `atFunctGroup.h`, `atManage.h`, `atsample.h`. Duplicate declarations removed from `atHarvest.h`, `atEconomic.h`, `atManage.h`.
- Build: OpenMP needs `-fopenmp` (`BUILD_OPT="-O2 -mtune=generic -fopenmp"`). A normal build without it is the original serial code.

Binary: `performance/nesp_1y/bin/atlantisMerged_box_omp_final` (OpenMP, `-O2`). The shared executable was restored after every build.

Checks:

- ThreadSanitizer, 2 threads, 1 day (`runs/tsan_ws_r2`): the only reports left are the OpenMP runtime handing the loop bounds to the threads (false alarms, the runtime is not instrumented).
- 1 thread, 30 days (`runs/box_ws_30d`): `IDENTICAL=True` against `baseline_30d`.

| Run | Threads | Wall | Box loop | Largest relative difference |
|---|---|---|---|---|
| `baseline_30d` | serial build | 5:15.09 | | reference |
| `box_ws_30d` | 1 | 5:08.97 | 302 s | 0 (identical) |
| `box_omp2_v3_30d` | 2 | 2:35.89 | 149 s | 3.1e-15 |
| `box_omp4_v3_30d` | 4 | 2:16.19 | 128 s | 3.3e-14 |
| `box_omp2_pcores_30d` | 2, pinned to cpu0 and cpu2 | 3:07.73 | 180 s | 9.5e-16 |
| `quiet_O2` (1 year) | serial build | 48:33.56 | | |
| `box_omp2_1y` (1 year) | 2 | 36:19.64 | 2109 s | 3.1e-11 against `baseline_rep1` |

No value in any run differs by more than 1e-9 (relative), and no run has a non-finite mismatch. The differences come from adding the box totals in a different order.

Speed improvement: 2.0x over 30 days with 2 threads, 2.3x with 4 threads. Over the full year, 2 threads gave 1.34x (48:34 to 36:20). Month by month the year gain was 1.3-1.45x from the start. The short 30-day runs fit inside the laptop's turbo power budget, while a 36-minute run with two busy cores runs at the lower sustained clock. Threads 3 and 4 land on the slower E-cores, and spawning runs one box at a time, so 4 threads add little over 2 on this laptop. Pinning both threads to P-cores was slower.

Accepted/rejected: accepted with the rounding tolerance the user chose (maximum relative difference below 1e-9). Runs without `OMP_NUM_THREADS` keep the exact serial results.

## Experiment 7: skip feeding calculations for zero diet links

Optimisation: at the start of `Calculate_PreyAvail` (called by `Eat` for every prey group, prey cohort and predator cohort, in every layer and substep), return straight away when the diet matrix entry `pSPVERTeat[predator][prey][predator stage][prey stage]` is 0. The function then sets `EATINGinfo` to 0 for the habitats it covers, which is exactly what the full calculation gives. The calls to `Get_Gape_Lim_Prey` and `Avail_Fish` are skipped.

Reason: the 30-day profile put `Eat`, `Avail_Fish` and `Get_Gape_Lim_Prey` at about 45% of the run, with billions of calls. A counting build (`-DATLANTIS_COUNT_ZERO_LINKS`, removed afterwards) showed that 78% of `Calculate_PreyAvail` calls are for a zero diet link (1-day run, 100 million calls).

Why the results cannot change: `pSPVERTeat` is only set when the biology parameters are read. For a zero link, every habitat's availability is 0 times a finite value, so each prey total only gains +0. The skip applies only when all of these hold. Otherwise the original code runs.

- New diet calculations (`flag_olddiet 0`)
- No debug output
- Not seagrass under the macrophyte model
- Not a catch-eating predator with fisheries on
- Finite pH scalars

The one theoretical difference: the original would turn a non-finite prey value times 0 into a non-finite availability, whereas the skip gives 0. That only happens in a run that is already broken.

Not done: the optional second skip in the grazing loop of `Eat`. For zero links, that loop can read leftover `EATINGinfo` values (inactive prey, and non-water-column habitats of once-per-step prey). It also divides by layer thicknesses, and the min-max feeding option gives 0/0 there. An exact skip would need a long, fragile guard.

Files/functions modified: `atlantis/atecology/atprocess.c`, `Calculate_PreyAvail`. Binaries `performance/nesp_1y/bin/atlantisMerged_zero_link_skip` (serial `-O2`) and `atlantisMerged_zero_link_skip_omp` (OpenMP). The shared executable was restored after every build.

The machine was noticeably slower today than when the earlier baselines were timed, so speed is measured in back-to-back pairs with the unchanged binary:

| Pair | Unchanged | With skip | Faster by |
|---|---|---|---|
| 30 days, serial (skip first) | 6:04.38 (`quiet_O2_recheck_30d`) | 5:34.96 (`zero_link_skip_30d`) | 8.1% |
| 30 days, serial (unchanged first) | 5:13.58 (`quiet_O2_recheck2_30d`) | 4:47.70 (`zero_link_skip_rep2_30d`) | 8.3% |
| 30 days, 2 threads | 3:16.93 (`box_omp2_recheck_30d`) | 2:59.31 (`zero_link_skip_omp2_rep2_30d`) | 9.0% |

Output differences:

- Serial, 30 days: `IDENTICAL=True` against `baseline_30d` (both repeats).
- Serial, 1 year (`zero_link_skip_1y`): `IDENTICAL=True` against `baseline_rep1`. Wall 58:36.62, not comparable with the 48:34 from the evening before because of the slower machine today.
- 2 threads, 30 days: maximum relative difference 4.2e-15, none above 1e-9.

Speed improvement: about 8% serial and 9% with 2 threads, with exact results.

Accepted/rejected: accepted.

## Experiment 8: exact compiler tuning

Optimisation: build the current best code (Experiment 7) with stronger compiler settings, with no source changes. Four candidates were tried in turn, each against the previous best:

- A: `-O3`
- B: `-O3 -march=native -ffp-contract=off`
- C: link-time optimisation (`-flto=4`)
- D: profile-guided optimisation

Reason: the code is now dominated by many small, branchy feeding functions. Inlining across files (link-time optimisation) and laying out branches from a measured profile (profile-guided optimisation) target exactly that.

Why the results cannot change: none of these settings allow the compiler to reorder floating-point arithmetic, because `-ffast-math` is never used. `-march=native` would enable fused multiply-add, which rounds differently, so it is paired with `-ffp-contract=off`. Profile-guided optimisation only changes code layout, inlining and branch order, not arithmetic. Every candidate was still checked for `IDENTICAL=True` against `baseline_30d`.

Method:

- Builds use `performance/build_candidate.sh`. It now accepts `BUILD_MAKE_ARGS` (link-time optimisation needs `AR=gcc-ar RANLIB=gcc-ranlib`) and keeps `*.gcda` profile files when cleaning objects.
- 30-day timings use the new `performance/run_30d.sh`.
- Each candidate was timed in two back-to-back pairs with the previous best, swapping which binary runs first. It was kept only if both pairs showed at least about 3% gain. The same `-O3` binary ranged from 4:18 to 5:28 across today's runs, so only times within a pair are compared.

| Candidate | Pair 1 (previous best, candidate) | Pair 2 (candidate, previous best) | Decision |
|---|---|---|---|
| A `-O3` vs `-O2` | 5:05.13, 4:34.63 (10.0% faster) | 4:18.34, 5:19.18 (19.1% faster) | kept |
| B `-march=native -ffp-contract=off` vs A | 4:29.80, 5:03.96 (12.7% slower) | 5:15.25, 5:28.38 (4.0% faster) | rejected |
| C `-flto=4` vs A | 4:57.72, 4:55.89 (0.6% faster) | 5:48.36, 5:07.95 (13.1% slower) | rejected |
| D profile-guided vs A | 4:47.04, 4:14.80 (11.2% faster) | 4:15.70, 4:51.69 (12.3% faster) | kept |

All four candidates gave `IDENTICAL=True` over 30 days. B was also rejected because a `-march=native` binary only runs on CPUs like this laptop's. C built and linked correctly (link-time optimisation confirmed) but gave no gain.

Profile-guided build, serial: train with `-g -O3 -mtune=generic -fprofile-generate`, run 30 days once (`runs/pgo_training_30d`, 5:53) to write the `.gcda` files next to the objects, then rebuild with `-g -O3 -mtune=generic -fprofile-use -fprofile-correction`. Only 15 `sjwlib` files that the run never uses had no profile.

Profile-guided build, OpenMP: GCC refuses to reuse the serial profile with `-fopenmp` (control-flow mismatch), so the OpenMP build has its own profile. It was trained with `-fopenmp -fprofile-generate` and `OMP_NUM_THREADS=2` for 30 days (`runs/pgo_omp_training_30d`). That run took 27:27, because both threads update the same profile counters. Then it was rebuilt with `-fopenmp -fprofile-use -fprofile-correction`. All `.gcda` files were deleted afterwards.

Winning settings (the `PreRules.am` defaults are not changed):

- Serial: training build `BUILD_OPT="-g -O3 -mtune=generic -fprofile-generate"`, one 30-day run, then `BUILD_OPT="-g -O3 -mtune=generic -fprofile-use -fprofile-correction"`, both with `CLEAN_OBJECTS=1`. No `BUILD_MAKE_ARGS` needed.
- OpenMP: the same, with `-fopenmp` added to both builds and the training run made with `OMP_NUM_THREADS=2`.
- Without profile-guided optimisation, `BUILD_OPT="-g -O3 -mtune=generic"` alone already gives most of the `-O3` gain.

Files/functions modified: `performance/build_candidate.sh` (`BUILD_MAKE_ARGS`, `.gcda` files kept), new `performance/run_30d.sh`. No model source changed. Binaries in `performance/nesp_1y/bin/`:

- `atlantisMerged_cc_O3_pgo` (serial winner)
- `atlantisMerged_cc_O3_pgo_omp` (OpenMP winner)
- `atlantisMerged_cc_O3`
- `atlantisMerged_cc_O3_native`
- `atlantisMerged_cc_O3_lto`
- `atlantisMerged_cc_O3_pgo_train` and `atlantisMerged_cc_O3_pgo_omp_train` (training builds)

The shared executable was restored to the frozen baseline (sha256 `8fc1952e...`) after every build.

Output differences:

- Serial, 30 days: `IDENTICAL=True` against `baseline_30d` for every candidate.
- Serial, 1 year (`cc_pgo_1y`): `IDENTICAL=True` against `baseline_rep1`. Wall 1:01:31, not comparable with earlier 1-year runs: other desktop processes were using more than one core during the run.
- OpenMP, 2 threads, 30 days (`cc_pgo_omp2_30d`): maximum relative difference 5.8e-15, none above 1e-9, no non-finite mismatches. Timed in a pair with the Experiment 7 OpenMP build: 4:27.15 (`omp_ref_vsPGO_30d`, `-O2`) against 3:26.22 (profile-guided), 22.8% faster.

Speed improvement: `-O3` gave 10–19% over `-O2`, and profile-guided optimisation gave a further 11–12% over `-O3`. Together that is about 25% less wall time than the Experiment 7 serial build over 30 days (estimated from the chained pairs), and 23% with 2 threads (measured directly). Serial results are exact.

Accepted/rejected: accepted (`-O3` plus profile-guided optimisation). `-march=native` and link-time optimisation rejected.
