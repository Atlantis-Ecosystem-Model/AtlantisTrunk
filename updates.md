# Atlantis optimisation updates

Last updated: 2026-10-06 14:45 (UTC+11)

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

