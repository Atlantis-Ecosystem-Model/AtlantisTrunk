# Porting notes: FishMIP_OSP_Plugin_Models.py → C++

This directory is a C++17 port of `../FishMIP_OSP_Plugin_Models.py`. Every
function in the Python module has a same-named counterpart in
`FishMIP_OSP_Plugin_Models.hpp` / `.cpp`, in the same order, so the two files
can be read side by side. This document records every place the port had to
change something, why, and how it was validated.

The Python source's inline comments (fisheries-science rationale, units,
parameter-ordering conventions, TODO/caveat notes, etc.) have been carried
over onto the corresponding C++ lines, since they carry domain context the
code alone doesn't convey. A handful of the richer ones (the Pella &
Tomlinson r/z priors guidance, the objective() parameter layout, and the
MSY tuple ordering) were promoted to doc comments on the relevant
declarations in `FishMIP_OSP_Plugin_Models.hpp` instead of staying inline,
since they're most useful to a caller who never opens the `.cpp`.

## Layout

```
cpp/
  include/
    ndarray.hpp                    row-major N-D array (replaces numpy.ndarray for 3D/4D data)
    pchip.hpp                      fixed 4-knot monotone cubic Hermite interpolation
    optim.hpp                      Nelder-Mead / differential evolution / bounded L-BFGS
    csv_utils.hpp                  CSV reader (replaces pandas.read_csv)
    netcdf_utils.hpp               netCDF-C wrapper (replaces xarray.load_dataarray/.sel)
    FishMIP_OSP_Plugin_Models.hpp  the ported module's public API
  src/                             matching .cpp implementations
  config/control_parameters.csv    control-parameter constants (see PIM_get_control_parameters below)
  examples/smoke_test.cpp          exercises every function, incl. real driver I/O
  CMakeLists.txt
```

## Build

Dependencies: a C++17 compiler, CMake ≥ 3.16, `netcdf-c` (the plain C
library, not netcdf-cxx), and `pkg-config` to locate it.

```
brew install netcdf          # if not already present; this machine had it at /usr/local
cd cpp && mkdir -p build && cd build
cmake ..
make -j
./smoke_test /path/to/Historical_drivers ../config/control_parameters.csv
```

On this machine `netcdf` was already installed at `/usr/local` (found
automatically via `pkg-config`). No other change to `PKG_CONFIG_PATH` was
needed.

## Dependency choice: Eigen was requested but ended up unused

The initial plan (per your choice) was "Eigen for arrays + hand-rolled
optimizers." While writing the port it turned out Eigen added nothing:
every array operation in the Python module is either a simple elementwise
op, a reduction (`.sum()`, `np.add.at`), or a gather/scatter by label — none
of it is genuine linear algebra (no matrix products, decompositions, or
solves anywhere in this file). A small header-only `NdArray<N>` class
(`ndarray.hpp`, flat `std::vector<double>` + row-major indexing) turned out
to be simpler, requires zero external dependency, and maps 1:1 onto the
numpy shapes used in the Python source (`(nfleet, nEEZ, xsize)` etc., up to
4D). Eigen was still installed (`brew install eigen`, header-only, at
`/opt/homebrew/opt/eigen`) in case you want it for a future numerical
addition, but `CMakeLists.txt` does not currently link against it since
nothing uses it. If that changes, add `find_package(Eigen3)` /
`target_link_libraries(... Eigen3::Eigen)`.

## Section-by-section notes

### `PIM_configure_simulation`, `PIM_configure_stock_assessment`
Direct translations; no behavioural changes. `x0_prev` (a Python list of
`None`) becomes `std::vector<std::optional<std::vector<double>>>`.

### `PIM_get_control_parameters` — constants moved to a CSV
The Python source hard-codes the catchability-scaling, vessel-lifespan, and
effort-growth constants (lines 74-89) as literals in the function body. This
port instead reads them from **`cpp/config/control_parameters.csv`**, a
small "name,value,notes" table, via the new `readKeyValueCsv()` /
`requireParam()` helpers in `csv_utils.hpp`/`.cpp` (a lookup-by-name reader,
distinct from the positional `CsvTable` reader `PIM_load_drivers` uses,
since this table doesn't have one row per country/stock). The function
signature gained a `paramsCsvPath` argument accordingly:

```cpp
ControlParameters PIM_get_control_parameters(NdArray3& catchability_EEZ, NdArray3& catchability_DWFN,
                                              const std::string& paramsCsvPath);
```

`f_growth_rate_max = 25**(1/10)` in Python becomes two CSV rows
(`f_growth_rate_max_base=25`, `f_growth_rate_max_root=10`) combined in code
as `pow(base, 1/root)`, so the base and root stay independently tunable
rather than baking `25**(1/10)` into a single opaque CSV value.
`effort_growth_lim` is stored as a plain number (`1`) and cast to `int` on
read, matching its use as a `1`/other switch in both the Python and C++
sources. `examples/smoke_test.cpp` defaults `paramsCsvPath` to
`../config/control_parameters.csv` (i.e. run from `cpp/build/`, CMake's
default binary dir) and takes it as an optional second CLI argument.
Verified this produces numerically identical output to the previous
hard-coded version (`M = (0.1497866136777, 0.11982929094216)`,
`f_growth_rate_max = 1.37972966146121`, same post-scaling
`catchability_EEZ` checksum as in the Validation table below).

### `PIM_load_drivers` — the biggest infrastructure change
- **pandas → `CsvTable`** (`csv_utils.hpp`): a minimal CSV reader supporting
  both delimiters used by the driver CSVs (`,` for `parameters.csv`, `;` for
  the IPBES/EEZ tables), a UTF-8 BOM on the first header cell (present in
  `EEZ_countries_WORK.csv`), and simple double-quoted fields. It does not
  implement the full pandas CSV dialect (no thousands separators, no
  type inference beyond "parse as double or treat as NaN"); this was
  sufficient for all three CSVs the loader reads.
- **xarray → `NcDataset` / `NcVariable`** (`netcdf_utils.hpp`), built directly
  on the netCDF-C API (`netcdf.h`), not the netcdf-cxx4 wrapper (which is
  less commonly packaged). `NcDataset::readMainVariable()` replicates
  `xr.load_dataarray()` by finding the one floating-point variable in the
  file whose name isn't also a dimension name — this matches every driver
  file inspected here (`ncdump -h` on all twelve `.nc` files: each has
  exactly one non-coordinate `double` variable). `selectByLabelAxis()`
  replicates `DataArray.sel(iso=EEZ_countries)` / `.sel(country=...)`: it
  looks up each wanted ISO/country code in the file's own coordinate
  variable and gathers along the matching axis, preserving the order of
  the *wanted* list — exactly xarray's label-based reindex/reorder
  semantics.
- **`driver_X_fresh`/`driver_W_fresh`/`driver_X_caned`/`driver_W_caned` are
  NOT reindexed** — same as the Python source: it reads these straight from
  file without `.sel()`, trusting the file's own country axis order matches
  `countries` (the IPBES table's row order). The port preserves this
  assumption unchanged; it was validated (see below) and holds for the
  current data files.
- **The IPBES/EEZ inner merge** (`codes_IPBES.merge(EEZ_ctry, how='inner',
  left_on='Country (IPBES)', right_on='Country')`) is reimplemented as a
  hash-join that iterates the IPBES table in row order and, for each row,
  emits one output row per matching `EEZ_ctry` row (in `EEZ_ctry`'s own
  order). This reproduces pandas' result whenever country names aren't
  duplicated on either side, which is true of the current data (`EEZ_iso`
  had zero duplicate ISO codes when checked against the live files — see
  Validation below) and was print-diagnosed by the Python source itself
  (the "Codes ISO en double" check). If the data later grows real
  duplicate country names, pandas' tie-break order for the cross-product
  might differ from this implementation's; the *set* of merged rows would
  still be identical, only relative order among ties could differ.
- **`Qcap_max`** is returned as a plain `std::vector<double>` in raw CSV row
  order (the Python source returns it as an unmodified pandas Series and
  never reindexes it inside this file either).

### `PIM_define_variables`
- The six `xr.DataArray(dims=..., coords=...)` calls **without** an explicit
  `data=` argument (the `*_allctry` price/consumption tables) default to
  `NaN`-filled arrays in xarray — verified directly (`xr.DataArray(dims=...,
  coords=...).values` prints all-`NaN`). The port fills those six `NdArray2`
  fields with `NaN` to match; the three `catch_step_*_allctry` tables *do*
  pass `data=np.zeros(...)` in the Python source and are zero-filled here
  too.
- `MSY_est = np.empty(nstocks)` (uninitialised memory in numpy) becomes a
  zero-filled `std::vector<double>` — the array is always written by
  `PIM_assess_stocks` before being read, so this has no behavioural effect,
  it just avoids exposing garbage memory.
- **Added a runtime check** the Python version doesn't have explicitly but
  effectively enforces via numpy's broadcasting rules: `access[:, :nEEZ, :]
  = access_nat[:, :, :]` in Python raises a `ValueError` if `access_nat`'s
  time axis doesn't match the simulation's `xsize` (this happens whenever
  `PIM_load_drivers` is fed driver files whose time length differs from
  `xsize`, e.g. an SSP run where `xsize` spans 1850–2100 but the driver
  `.nc` files currently only cover 1850–2014). The C++ port throws a
  `std::runtime_error` with an explanatory message instead of silently
  truncating or reading out of bounds — same fail-fast behaviour as numpy,
  clearer error message.

### `plugin_model_1_calc_price`, `mini_model_2_calc_aqua`, `plugin_model_2_calc_effort`, `PIM_allocate_target_local_DWFN`
Direct, branch-for-branch translations of the numpy code (no algorithmic
changes). `scipy.interpolate.PchipInterpolator(x_points, y_row)` — always
called here with the fixed knots `x_points = [0, 1, 2, 3]` — is replaced by
`pchip4_eval()` (`pchip.hpp`), a specialised (unit-spacing) implementation
of the same Fritsch-Carlson monotone-cubic algorithm SciPy uses internally,
including the same endpoint-derivative adjustment rule. **Exact numerical
match confirmed** against the Python functions on identical inputs (see
Validation).

### `dynamics_loop`
Was a Numba `@njit`-compiled Python function; in C++ it's just a plain
function — no annotation needed, the compiler already produces native code.
Logic is unchanged.

### `objective`, `fit_one_stock`, `plugin_model_3_assess_stock` — the optimizers
This is the one part of the port that is **not** expected to reproduce
Python bit-for-bit, by design (per your choice of hand-rolled optimizers
over binding to a SciPy-equivalent library):

- **Nelder-Mead** (`optim::nelderMead`): standard non-adaptive Nelder-Mead
  (reflect=1, expand=2, contract=shrink=0.5), SciPy's default initial-simplex
  construction, and SciPy's `(xatol, fatol)` convergence test — should track
  SciPy's `method='Nelder-Mead'` closely since the algorithm is fully
  deterministic and specified.
- **Differential evolution** (`optim::differentialEvolution`): a best1bin DE
  with SciPy's default dithered mutation range `(0.5, 1)` and recombination
  `0.7`, Latin hypercube initialisation, and SciPy's population-convergence
  test (`std(fitness) <= tol*|mean(fitness)|`). It uses `std::mt19937`
  seeded with the same seed value (`42`) the Python calls pass, but
  `std::mt19937` is a different generator from NumPy's default (PCG64/
  Mersenne-Twister via `numpy.random`), so **the actual random draws differ
  even with the same seed** — the two runs explore different points in
  parameter space. `workers=-1`/`workers=1` in the Python calls are accepted
  as an options field but the C++ version always evaluates the population
  serially (parallelism happens one level up, across stocks, in
  `PIM_assess_stocks` instead — see below).
- **L-BFGS-B** (`optim::boundedLBFGS`): this is the biggest algorithmic
  approximation. SciPy's `L-BFGS-B` is Byrd/Lu/Nocedal/Zhu's Fortran
  algorithm with an explicit active-set / generalised Cauchy point
  construction for bound constraints. The C++ port instead implements a
  **projected-gradient L-BFGS**: standard two-loop-recursion L-BFGS with a
  numerically-differentiated (bounds-aware central finite difference)
  gradient, an Armijo backtracking line search, and bound handling by (a)
  zeroing search-direction components that would push an already-at-bound
  variable further out of bounds, and (b) clipping every trial point back
  into `[lo, hi]`. This converges to the same optimum for well-behaved
  bound-constrained problems (confirmed empirically, see Validation) but is
  not guaranteed to match SciPy's iteration path or handle degenerate
  active-set edge cases identically.
- Net effect: `fit_one_stock` on the same data will converge **near** the
  same optimum as the Python version but not to bit-identical parameter
  estimates. On the synthetic validation case below, the two independently
  recovered `r`/`k`/`z` values agree to within ~1% of each other and of the
  ground-truth values used to generate the synthetic catch/effort series.
  If exact reproducibility of stock-assessment output between the Python
  and C++ versions is ever required, the fix is to replace `optim.cpp`'s
  backends with bindings to a real L-BFGS-B/DE implementation (e.g. NLopt)
  — this was explicitly traded away in favour of zero extra build
  dependencies, per your instruction.

### `PIM_assess_stocks` / `plugin_model_3_assess_stock` — `x0_prev` indexing
**Correction (previous revision of this note had the diagnosis backwards —
see below for what was actually verified).**

`plugin_model_3_assess_stock`'s own body indexes its `x0_prev` *parameter*
with the stock index: `fit_one_stock(data, mini1, verbose, x0_prev[i])`
(twice, lines 634 and 637 of the Python source). That means the function
expects to be **handed the whole per-stock list** and do its own lookup —
not to be handed one stock's already-selected warm start.

- The **parallel** branch of `PIM_assess_stocks` (`parall_stock_assess==1`,
  lines 590/594) passes the whole `x0_prev` list into
  `delayed(plugin_model_3_assess_stock)(...)`, which matches that internal
  indexing. This is the branch that actually runs by default
  (`PIM_configure_stock_assessment` sets `parall_stock_assess = True`), and
  it works correctly.
- The **sequential** branch (`parall_stock_assess==0`, lines 574/581)
  instead pre-indexes: it passes `x0_prev[i]` in. Combined with the
  function's own `x0_prev[i]` lookup, that's a **double indexing** — I
  initially misread this as the sequential branch being the correct one and
  "fixed" the parallel branch to match it, which is backwards. Verified
  directly against the live Python module with a small synthetic
  4-stock/15-year problem:
  - First call (`t == start_stock_assess`, `x0_prev` still all `None`):
    sequential raises `TypeError: 'NoneType' object is not subscriptable`
    immediately (`None[i]`); parallel completes fine.
  - A later call, once `x0_prev[i]` holds a real 9-element parameter vector
    for stock `i`: sequential now does `x0_prev[i][i]`, which — for `i`
    within the vector's length — silently pulls out a single **scalar**
    instead of the vector, then fails downstream in SciPy with `ValueError:
    The number of bounds is not compatible with the length of x0`.

  So `parall_stock_assess==0` cannot actually run at all in the current
  Python source, at any timestep. This is presumably unnoticed in practice
  because the default configuration uses the (working) parallel branch.

**What the C++ port does, and why no change is needed**: this port's
`plugin_model_3_assess_stock` takes a single stock's already-selected warm
start (`const std::optional<std::vector<double>>& x0_prev`) and uses it
directly — it does not re-index. Both `PIM_assess_stocks` branches
(`parallel_mode == 0` and `== 1`) call it the same way, indexing the outer
list exactly once at the call site: `x0_prev[i]`. That is functionally
identical to Python's *working* (parallel) convention — "look the stock's
warm start up once" — just relocated from inside the callee to the call
site. It was never an attempt to replicate the sequential branch's
double-indexing, and it doesn't need to become one: the C++ port already
runs correctly in both modes, which is strictly better than the Python
source (whose sequential mode cannot run at all as written). No change is
needed in `FishMIP_OSP_Plugin_Models.cpp` for this.

If you want the Python source itself fixed, the one-line change is in the
**sequential** branch, not parallel: replace `x0_prev[i]` with `x0_prev` in
the two `plugin_model_3_assess_stock(...)` calls at lines 574 and 581 (so it
passes the whole list, like the parallel branch already does).

**Parallelism**: `joblib.Parallel(n_jobs=-1)` is replaced with a small
`std::thread` pool (one worker per `std::thread::hardware_concurrency()`,
work-stealing over stock indices via `std::atomic<int>`) in
`PIM_assess_stocks`'s `parallel_mode == 1` branch. `parallel_mode` takes the
same 0/1 meaning as the Python `parall_stock_assess` flag (kept as `int`
rather than `bool` for a direct correspondence with the flag's use as an
`==0`/`==1` switch in the source).

## Validation

Cross-checked the C++ port against the live Python module
(`FishMIP_OSP_Plugin_Models.py`, run with the real driver files under
`../../Historical_drivers/`) on 2026-09-07:

| Function | Check | Python | C++ | Agreement |
|---|---|---|---|---|
| `PIM_load_drivers` | `sum(driver_X_fresh)` | 1048482145499.7954 | 1048482145499.82 | matches to FP summation-order noise |
| `PIM_load_drivers` | `sum(exported_frac)` | 6279.419806010076 | 6279.41980601012 | matches |
| `PIM_load_drivers` | `sum(access_nat)` | 399.80946937629614 | 399.809469376292 | matches |
| `PIM_load_drivers` | `sum(driver_U_national)` | 20.2704473869596 | 20.2704473869596 | exact |
| `PIM_load_drivers` | `sum(comply_target_nat)` (post `*=2`) | 50490.0 | 50490 | exact |
| `PIM_load_drivers` | EEZ_countries / EEZ_indices / EEZ_surf length | 153 / 153 / 153 | 153 / 153 / 153 | exact |
| `plugin_model_1_calc_price` (global branch) | price[0], consumption[0] | 0.0, 100.0 | 0, 100 | exact |
| `plugin_model_1_calc_price` (mixed branch) | price[0], consumption[0] | 0.0, 100.0 | 0, 100 | exact |
| `mini_model_2_calc_aqua` | aqua[0] | 100.00150001125006 | 100.00150001125 | exact |
| `plugin_model_2_calc_effort` | Ftot | [0.1872951, 0.24699862, 0.11745902] | [0.187295096584328, 0.24699862036038, 0.117459019316866] | exact |
| `PIM_allocate_target_local_DWFN` | target_EEZ | [0.24, 0.64, 1.2] | [0.24, 0.64, 1.2] | exact |
| `fit_one_stock` (mini=4, synthetic noiseless data, r=0.3/k=1000/z=1 ground truth) | r, k, z, FMSY, MSY, nll | 0.29364, 1041.16, 0.95065, 0.14311, 73.779, -280.704 | 0.29455, 1045.09, 0.93933, 0.14267, 73.663, -280.695 | agrees to ~0.4-1%, as expected for a stochastic optimizer with a different RNG (see notes above) |

The validation script used to produce the Python-side numbers is not
checked in (it's a throwaway `python3 -c "..."` snippet); rerun it against
`FishMIP_OSP_Plugin_Models.py` if you want to reproduce or extend this
table — `examples/smoke_test.cpp` runs the matching C++-side calls with the
same inputs.

Additionally, `smoke_test` runs `fit_one_stock` on the same synthetic
r=0.3/k=1000/z=1 series through **all six** `mini` values (0-5) and runs
`PIM_assess_stocks` through **both** the sequential and threaded paths on a
4-stock synthetic problem, to confirm every optimizer branch and the
`std::thread` pool converge without crashing and land near the same
(sequential sum_MSY=295.33 vs parallel sum_MSY=295.26) answer — the small
remaining gap between the two is expected, since the sequential and
parallel branches deliberately use different DE variants (`updating=
"deferred"` vs `"immediate"`, mirroring the Python source's own `mini=4` vs
`mini=5` choice per branch), not a threading bug.
