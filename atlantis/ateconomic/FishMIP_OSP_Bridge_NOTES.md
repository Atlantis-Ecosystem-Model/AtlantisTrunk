# FishMIP-OSP <-> Atlantis bridge: notes

This documents `ateconomic/atFishMIP_OSP_Bridge.cpp` — what it does, what it
deliberately does not do, and every change elsewhere in the codebase (build
system, call sites, parameter I/O) that would be needed to actually run it.
**None of those other changes have been made** — no file besides
`atFishMIP_OSP_Bridge.cpp` (and this note) was touched, per instructions.

The file exposes one annual entry point, `FishMIP_OSP_Bridge_Annual(bm,
llogfp, mode, ftargetSource, priceSource)`, controlled by three independent
switches: `mode` (sections 1/3 below — which of the three top-level
modelling options is active), `ftargetSource` (section 7 — where the effort
target used by mode A comes from, including the two stock-assessment wiring
directions), and `priceSource` (section 8 — real driver-file loading and the
external global-market price option). It also exposes two one-time (not
annual) loader entry points, `FishMIP_OSP_Bridge_LoadDrivers()` and
`FishMIP_OSP_Bridge_LoadGlobalPrice()`, covered in section 8.

## 1. What the bridge file does

It reproduces the per-year control flow of `TOY_calculate_biomass_dynamics()`
in `FishMIP-OSP_Toy_Model.py` — read biomass, read this period's catch, price
the catch, update effort from the priced catch — but with every array read
from or written to a real Atlantis structure, and every plug-in computation
made through the *unmodified* functions in `FishMIP_OSP_Plugin_Models.hpp` /
`.cpp` (namespace `fishmip`). Nothing in the plug-in model's own files was
copied, reimplemented, or altered.

| Toy model / plug-in concept | Atlantis equivalent used by the bridge | Source |
|---|---|---|
| `biomass_stock[:, t]` (stock biomass, tonnes) | `bm->totfishpop[sp]` (mg N) `* bm->X_CN * mg_2_tonne` | `atecology/atbiolmessage.c` (`Calculate_Box_Biomass`, `Ecology_Calculate_Total_Abundance`); conversion pattern from `atmanage/atManageAnnual.c:1138` |
| `catch_step_EEZ` / `catch_step_DWFN` (catch per fleet per stock) | `Harvest_Get_TotCumCatch(sp, nf, bm->thisyear)` | `atharvest/include/atHarvestLib.h:64`, already used this way throughout `ateconomic/*.c` and `atmanage/*.c` |
| `effort_EEZ[:, :, t]` / `effort_DWFN[:, :, t]` (effort per fleet per stock) | `bm->Effort[box][fishery]`, summed over `box` for a fishery-wide total | `atharvest/atHarvestCatch.c`, `atmanage/atManageAnnual.c`; units "effective boat-days/day" per `ateconomic/ateconeffort.c:479` |
| `price_EEZ` / `price_DWFN` (fleet sale price) | `bm->SP_FISHERYprms[sp][nf][saleprice_id]` | set natively by `Get_Fish_Prices()`, `ateconomic/atquota.c` |
| `nstocks` (one per EEZ + ABNJ) | fished functional groups, `FunctGroupArray[sp].isFished` | `atlantisUtil/include/atFunctGroup.h` |
| `nfleet` x `nEEZ` (fleet type x country) | `bm->K_num_fisheries` (Atlantis fisheries are already discrete fleets — no further artisanal/industrial split) | `atlantismain/include/atlantisboxmodel.h:5296` |

Every read above is a **plain read of an existing Atlantis output** — the
bridge does not recompute biomass or realised catch itself, since Atlantis
has already integrated its own (much more detailed) biomass and harvest
dynamics for the year by the time this would run.

### The three requested modelling options

All three are one function, `FishMIP_OSP_Bridge_Annual(bm, llogfp, mode,
ftargetSource)`, switched on an `OSPBridgeMode` enum. (The second parameter,
`ftargetSource`, is a separate, orthogonal switch — see section 7.)

- **(a) `OSP_MODE_A_EFFORT_ECONOMICS`** — *"only use biomass and catch
  processes from Atlantis but effort levels and economics all comes from the
  plug-in models."* Reads Atlantis biomass & realised catch; computes price
  via `plugin_model_1_calc_price` **and** next year's effort via
  `plugin_model_2_calc_effort`; writes both back
  (`SP_FISHERYprms[...][saleprice_id]`, `bm->Effort[box][fishery]`). This
  bypasses/replaces `Economic_Response()`, `Get_Fish_Prices()` and
  `Annual_Effort_Schedule()`.
- **(b) `OSP_MODE_B_MARKET_VALUE`** — *"use biomass, catch and effort
  processes from Atlantis but market value from the plug-in model."* Reads
  biomass, catch **and** effort, all left untouched — Atlantis's own
  `Economic_Response()` / `Annual_Effort_Schedule()` still decide next year's
  effort. Only `SP_FISHERYprms[...][saleprice_id]` is overwritten, in place
  of what `Get_Fish_Prices()` would have computed.
- **(c) `OSP_MODE_C_NATIVE_WITH_DRIVERS`** — *"all existing Atlantis
  processes but uses drivers that plug-in model would use."* Overwrites
  nothing Atlantis computes directly. `Get_Fish_Prices()`
  (`ateconomic/atquota.c:124-131`) computes
  `saleprice = market_wgt * (MARKETprms[market][sp][market_intercept_id] * SP_FISHERYprms[sp][nf][origprice_id] + trend*year + season + AR resid)`
  — i.e. `market_intercept_id` is a *ratio* against a reference price, not a
  price itself. The bridge translates the plug-in equilibrium price into that
  ratio and writes only `bm->MARKETprms[market][sp][market_intercept_id]`, so
  Atlantis's own trend/seasonal/autoregressive terms still apply on top.

### Call-site diagram (as found, not modified)

```
atlantismain.c:405   if (bm->flagecon_on) Economic_Annual(bm, logfp);
                         |
ateconomic.c:42       Economic_Annual(bm, llogfp)
                         |-- Economic_Response(bm, llogfp)          <- mode A replaces; mode C's driver injection must run BEFORE this
                         |-- Get_Fish_Prices(bm, llogfp)             <- mode A/B replace this output; mode C only feeds its input
                         |-- ... Update_Econ_Indx / output routines ...
                         |-- Annual_Quota_Allocation(bm, llogfp)
                         `-- Annual_Effort_Schedule(bm, llogfp)      <- mode A replaces
```

(Confirmed by reading `atlantismain/atlantismain.c:380-410` and
`ateconomic/ateconomic.c:42-108`.)

## 2. What this bridge deliberately does NOT do

- **No EEZ/country/DWFN spatial resolution.** The toy model's core structure
  — one commodity, ~153 EEZ "stocks" + 1 ABNJ stock, 2 fleet types (artisanal
  /industrial) each split into EEZ-based and distant-water (DWFN) fleets — has
  no natural Atlantis analogue. Atlantis already has its own spatial
  resolution (boxes) and its own fleet identity (fisheries); this bridge maps
  "one Atlantis species" onto "one toy-model stock" and "one Atlantis
  fishery" onto "one toy-model fleet", which is a **simplification**, not a
  faithful port of the EEZ/DWFN mechanism. `PIM_load_drivers()` (which reads
  the real country/EEZ driver `.nc`/`.csv` files) is never called.
- **`PIM_assess_stocks()` itself is not used** (though `fit_one_stock()`, the
  function it calls internally, now is — see section 7). `PIM_assess_stocks`'s
  catch/effort array shape bakes in the toy model's 4-fleet EEZ/DWFN split, for
  which Atlantis has no equivalent; the bridge calls `fit_one_stock()` per
  species directly instead, which needs no such reshaping. `PIM_allocate_
  target_local_DWFN()` is likewise not used — the bridge derives its per-
  fishery fishing-mortality target directly from `fit_one_stock`'s `FMSY` (or
  from Atlantis's own assessment output), rather than through that function's
  EEZ/ABNJ target-allocation logic.
- **No aquaculture term.** `mini_model_2_calc_aqua()` is unused; `aqua` is
  passed as all-zero to `plugin_model_1_calc_price`. Atlantis has no
  aquaculture production tracer to source this from without a further
  change.
- **Every "driver" the plug-in functions need beyond biomass/catch/effort is
  a placeholder**, clearly marked in the code: catchability (`q`), target
  fishing mortality, compliance, investment drivers (`d_U`/`d_V`), and the
  demand curve (`X`/`W`, elasticity). See section 4.

## 3. Build-system changes needed to actually compile/link this file

This is, by design, the **first C++ translation unit in the Atlantis
codebase** — every other file in `ateconomic/` (and the whole repo) is C,
compiled via `libateconomic_a_SOURCES` in `ateconomic/Makefile.am`, which is
a pure-C automake target (`CFLAGS=`, no `CXXFLAGS`/`AM_CXXFLAGS` anywhere).

1. **`atlantisboxmodel.h` has one C++ keyword collision**: a struct field
   named `export` (`ExportInfo *export;`, around line 3666). The bridge file
   works around this locally with `#define export
   atlantis_export_field_workaround` / `#undef export` bracketing its
   `#include <atEconomic.h>` — this edits nothing on disk and does not affect
   any other translation unit, but it is fragile (a second, future keyword
   collision anywhere in the transitively-included headers would need the
   same treatment, or the struct field would need renaming — which *would*
   be an edit to `atlantisboxmodel.h`).
   - **Verified**: with only that one workaround, `g++ -std=c++17
     -fsyntax-only` and `-c` both succeed compiling `#include <atEconomic.h>`
     (which pulls in `atlantisboxmodel.h`, `atUtilLib.h`, `atHarvestLib.h`,
     `atEconomicLib.h`, `sjwlib.h`) together with
     `FishMIP_OSP_Plugin_Models.hpp` in one translation unit, and
     `atFishMIP_OSP_Bridge.cpp` itself compiles clean with `-Wall -Wextra`
     (checked 2026-09-08, re-checked after adding driver loading). A **full
     link** was also attempted this time (`g++ ... bridge.o
     libfishmip_osp_plugin_models.a $(pkg-config --libs netcdf)`): every call
     into the plug-in library — `PIM_configure_simulation`, `PIM_load_
     drivers`, `PIM_get_control_parameters`, `fit_one_stock`, `readCsv`,
     `CsvTable::columnDoubles` — resolved successfully against the real
     compiled library and a real local netcdf-c install (`pkg-config
     --libs netcdf` → `-L.../netcdf/4.10.1_1/lib -lnetcdf` on this machine).
     The only unresolved symbols were Atlantis's own globals
     (`FunctGroupArray`, `Harvest_Get_TotCumCatch`), which is expected — the
     rest of Atlantis was not linked in this test. Linking the actual
     `atlantis` binary was still not attempted.
2. **`ateconomic/Makefile.am`** would need:
   - `atFishMIP_OSP_Bridge.cpp` added to `libateconomic_a_SOURCES`.
   - A C++ compiler enabled for this target (automake: `AM_CXXFLAGS`, and the
     top-level `configure.ac` would need `AC_PROG_CXX` if it does not already
     have it — not checked here).
   - An include path to wherever `FishMIP_OSP_Plugin_Models.hpp` /
     `ndarray.hpp` / `csv_utils.hpp` live. They currently live outside this
     repository, at
     `.../Fish-MIP/OSP/OSP_code/FishMIP-OSP_Toy_Model/cpp/include/`. Either
     vendor a copy of that `cpp/` directory into the Atlantis repo (e.g. a
     git submodule under `ateconomic/fishmip_osp/`) or add a configure-time
     `--with-fishmip-osp=PATH` option — a project decision, not made here.
   - A link against the plug-in's compiled code
     (`libfishmip_osp_plugin_models.a`, already built at
     `cpp/build/libfishmip_osp_plugin_models.a` in the OSP_code checkout) and
     against `netcdf-c` (e.g. via `pkg-config --cflags --libs netcdf` in
     `AM_CPPFLAGS`/`AM_LDFLAGS`) — **now a genuine, exercised dependency**:
     `FishMIP_OSP_Bridge_LoadDrivers()` calls `PIM_load_drivers()` directly,
     which reads the driver `.nc` files via netCDF-C.
3. **The final Atlantis executable's link step** must be driven by a C++
   linker (`g++`/`clang++`, or `gcc`/`clang` with `-lstdc++` added
   explicitly) once any C++ object file is part of the link — whatever
   currently links the `atlantis` binary from all the library `.a` files
   would need this change.

## 4. Call-site wiring needed (not applied)

1. A one-line call to `FishMIP_OSP_Bridge_Annual(bm, llogfp, mode,
   ftargetSource, priceSource)` inside `Economic_Annual()`
   (`ateconomic/ateconomic.c:42`), positioned per the diagram in section 1
   depending on `mode`.
2. A **one-time** call to `FishMIP_OSP_Bridge_LoadDrivers()` (and, if used,
   `FishMIP_OSP_Bridge_LoadGlobalPrice()`) at model start-up — NOT inside
   `Economic_Annual()`, since these read whole driver files and are far too
   expensive to repeat every year. The natural spot is alongside Atlantis's
   own other one-time economic setup, e.g. `Economic_Init()`
   (`ateconomic/include/atEconomicLib.h`), called once before the main annual
   loop begins.
3. A new run-time switch to select `mode`, `ftargetSource`, `priceSource`,
   and the `FishMIP_OSP_Bridge_LoadDrivers()`/`LoadGlobalPrice()` arguments
   (data directory, control-parameters CSV path, reference country ISO3,
   commodity, base calendar year, global-price CSV path) — currently there
   is no `bm->flagOSPPluginMode`-style field for any of this. This would
   need:
   - New fields on `MSEBoxModel` (`atlantismain/include/atlantisboxmodel.h`)
     for the three enum switches, plus string/int fields for the loader
     arguments.
   - Read/write support in whichever parameter file/XML reader currently
     populates `bm->flagecon_on` and similar flags (`ateconomic/ateconParamIO.c`
     and/or the run-XML readers in `ConvertAtlantis/atRunXML.c`).
   - If `ftargetSource == OSP_FTARGET_PIM_ASSESSMENT` is selected alongside
     `bm->do_assessing` being on for the same species, the run configuration
     should also disable Atlantis's own Tier assessment for those
     species/fisheries (see section 7(i)) to avoid the redundant
     (harmless, but wasted) computation.
4. A small header (e.g. `ateconomic/include/atFishMIP_OSP_Bridge.h`) exposing
   `OSPBridgeMode`/`OSPFtargetSource`/`OSPPriceSource` and the three
   `FishMIP_OSP_Bridge_*` functions to the C callers in `ateconomic.c` — the
   declarations currently live only at the top of `atFishMIP_OSP_Bridge.cpp`
   itself, guarded `extern "C"`, which is sufficient for compiling that one
   file but not for calling any of them from a separate `.c` file without
   duplicating or `#include`-ing the declarations.

## 5. Parameters with no Atlantis-native source

`plugin_model_2_calc_effort` and `plugin_model_1_calc_price` need several
inputs Atlantis has no per-species/per-fishery parameter for today. The
bridge fills these with clearly-labelled placeholder constants (see
`BuildPlaceholderConfig()` and the inline "Placeholder demand curve" comment
in `atFishMIP_OSP_Bridge.cpp`) purely so the numeric example is well-defined
— they are not calibrated to any real Atlantis parameterisation:

| Parameter | Used by | Placeholder in bridge | What a real source would look like |
|---|---|---|---|
| Catchability `q` | effort dynamics | constant `1e-5` per fishery | a new `SP_FISHERYprms`-style column, or derived from Atlantis's own existing gear-selectivity/catchability parameters |
| Target fishing mortality `Ftarget` | effort dynamics | constant `0.2` per fishery when `ftargetSource == OSP_FTARGET_PLACEHOLDER` | now has two real (non-placeholder) sources — see section 7 |
| Compliance | effort dynamics | `1.0` (fully compliant) everywhere | a new management-compliance parameter |
| Investment drivers `d_U`/`d_V` | effort dynamics | `1.0`/`0.0` (neutral) | fuel cost, vessel price, or other cost/revenue driver time series |
| Demand curve `X`/`W`, elasticity `epsilon` | price | `X` = 1.5x current catch, `W` = 1, `epsilon` = -1.5 | real per-species demand data, analogous to the toy model's `driv_X_fresh`/`driv_W_fresh` (themselves loaded from the FishMIP-OSP `Historical_drivers` files) |

## 6. Making markets cross-species (extension point)

The bridge currently gives every fished species its own local market
(`geo_agreg == 0`, `region_idx[i] == subregion_idx[i] == i`,
`nregion == nsubregion == nctry == number of fished species`) when calling
`plugin_model_1_calc_price` — the simplest valid instantiation of that
function's four-level (national/subregional/regional/global) market
hierarchy, with no cross-species price interaction. To model several species
sharing one market (e.g. substitute commodities), group them under a shared
`region_idx`/`subregion_idx` value and raise `geo_agreg` above 0 for those
species, exactly as `PIM_calculate_prices_consumptions` does across
countries in the original toy model.

## 7. Stock-assessment wiring (`OSPFtargetSource`)

`plugin_model_2_calc_effort` (mode A only) needs a fishing-mortality target,
`Ftarget`. Where that target comes from is a second, independent switch,
`OSPFtargetSource`, passed alongside `mode`. It only has any effect when
`mode == OSP_MODE_A_EFFORT_ECONOMICS`.

### (i) `OSP_FTARGET_PIM_ASSESSMENT` — PIM's assessment drives effort *and* Atlantis's TACs

*"use the stock assessment from PIM to set [Ftarget], then [that is] used to
set the effort and to feed back to TACs."*

For each fished species, `RunPIMAssessment()`:

1. Builds that species' full catch history straight from Atlantis's own
   storage: `Harvest_Get_TotCumCatch(sp, nf, iy)` for every year `iy` from 0
   to `bm->thisyear`, for every fishery — no new storage needed, Atlantis
   already keeps this.
2. Builds a matching effort history. Atlantis has **no existing per-fishery,
   multi-year effort record** (`bm->EffortRecord` is indexed
   `[year][species][dataid]`, no fishery axis, and was built for Atlantis's
   own classical/Tier assessment input format, not this bridge's per-fishery
   breakdown — atassess/atclassical.c). So the bridge keeps its own rolling
   record (`EffortHistoryFor()`), appended to with one new row (this year's
   `bm->Effort[box][fishery]` summed over `box`, per fishery) every time the
   bridge runs. **Consequence: this history is only as long as the number of
   years the bridge itself has been running** — if the bridge is switched on
   partway through a run, the assessment "sees" a flat/constant effort for
   every year before that, which is a real (if unavoidable, without adding
   Atlantis-side storage) limitation on assessment quality in the first few
   years after turning this option on.
3. Calls `fit_one_stock()` — the Pella & Tomlinson biomass-dynamics fit,
   exactly the function `PIM_assess_stocks()` itself calls per stock (see
   section 2 for why `PIM_assess_stocks()` itself isn't used) — with generic
   placeholder priors (`prior_mean_r = 0.3`, Schaefer `prior_mean_z = 1.0`;
   see the doc comment on `fit_one_stock` in
   `FishMIP_OSP_Plugin_Models.hpp` for what species-specific values should
   look like) and a per-species warm-start cache (`FitWarmStartFor()`) so
   each year's fit starts from the previous year's converged optimizer
   state, mirroring the toy model's own `x0_prev` threading.
4. On convergence, `FMSY` becomes that species' fishing-mortality target
   (catch-weighted down to one-per-fishery, same as price, in the main
   per-fishery loop), and `MSY` is fed back into Atlantis's own TAC fields by
   `FeedPIMAssessmentIntoAtlantisTAC()`:
   `bm->TACamt[sp][nf][now_id]` (the field Atlantis's own TAC checks read,
   e.g. `atharvest/atHarvestIndex.c:225`,
   `atimplementation/atImplementationAnnual.c:301`) and
   `bm->SP_FISHERYprms[sp][nf][TAC_id]` (the "origin" TAC that
   `atimplementation/atImplementationAnnual.c`'s non-assessment
   "scenario-painting" TAC-scaling path reads as `TACorig`), allocated across
   fisheries in proportion to each fishery's existing TAC share — the same
   rule Atlantis's own Tier system uses to split a stock-wide TAC
   (`atmanage/atManageTier.c:522`).
5. If there isn't yet enough history (`< assess_min_years`, default 5 years)
   or the fit doesn't converge, that species falls back to the placeholder
   `Ftarget` for that year, and no TAC is written.

**Interaction with Atlantis's own assessment**: if `bm->do_assessing` is on,
Atlantis's own Tier system (`atmanage/atManageTier.c`, called from
`Manage_Annual_Calculations()`, which runs *before* `Economic_Annual()` — see
the call-site diagram) will already have set `bm->TACamt` for the year.
Calling this option's TAC feedback afterwards (as intended, from within/after
`Economic_Annual()`) simply overwrites that with the plug-in's own figure —
harmless, but wasteful, and a real deployment would normally turn off
Atlantis's own Tier assessment for whichever species/fisheries use this
option.

### (ii) `OSP_FTARGET_ATLANTIS_ASSESSMENT` — Atlantis's assessment drives PIM's effort calculation

*"for Atlantis stock assessment to feed the PIM effort calculations."*

The reverse direction: no plug-in assessment runs at all.
`FtargetFromAtlantisAssessment()` reads Atlantis's own existing
Tier-assessment output — `bm->RBCestimation.RBCspeciesParam[sp][RBCest_id]`
(the Tier system's Recommended Biological Catch, tonnes) and
`[sp][EstBcurr_id]` (its estimated current biomass, tonnes), both populated
by `atmanage/atManageTier.c` (allocated per species in
`atassess/atassessParamIO.c:617`) — and computes `RBCest / EstBcurr` as a
proxy fishing-mortality target (an implied exploitation rate), since
`plugin_model_2_calc_effort` wants an actual F (yr⁻¹), not a catch tonnage,
and Atlantis's Tier system does not appear to expose a single named
"Fmsy"-type field directly.

If `bm->RBCestimation` has not been populated for a species (e.g.
`bm->do_assessing` is off, or the species isn't Tier-assessed), this falls
back to the placeholder `Ftarget` for that species/year, same as (i).

**This option never writes to `bm->TACamt`/`TAC_id`** — Atlantis's own TAC
setting (whichever mechanism, Tier-based or scenario-painting) is left
completely alone; only the plug-in's effort-dynamics equation is informed by
it.

### What neither option does

Neither touches `PIM_allocate_target_local_DWFN()` (the toy model's own
EEZ/ABNJ target-splitting step) — both derive one fishing-mortality target
per species directly (from `fit_one_stock`'s `FMSY` or from
`RBCest/EstBcurr`), then catch-weight it down to one-per-fishery in the same
loop that aggregates price, rather than reproducing that function's
EEZ/DWFN-specific allocation logic (see section 2 for why).

## 8. Real driver loading and the external global-market price

Two more independent capabilities, both added on top of everything above
without changing any of it: loading the toy model's own real driver files so
the bridge's placeholder economics (section 5) are replaced by real data, and
an option to treat "global-market price" as an external input rather than
something this bridge computes.

### 8.1 `FishMIP_OSP_Bridge_LoadDrivers()` — real PIM drivers, not placeholders

Call **once**, before the annual loop starts (see section 4.2) —
`FishMIP_OSP_Bridge_LoadDrivers(dataDir, controlParamsCsvPath,
referenceCountryISO3, commod, nosp, baseCalendarYear)`. It calls, directly
and unmodified:

- `PIM_configure_simulation(nosp)` — the same time-axis setup the toy model
  uses (`time_min`/`time_max`/`xsize`, historical 1850-2014 or an OSP/SSP
  extension to 2100).
- `PIM_load_drivers(commod, sim.nctry, sim.xsize, dataDir)` — reads the SAME
  twelve driver `.nc` files and three CSVs (`IPBES_regions_...csv`,
  `EEZ_countries_WORK.csv`, `parameters.csv`) the toy model itself reads from
  `dataDir` (i.e. FishMIP-OSP's `Historical_drivers/` directory), for the
  chosen `commod` (0/1/2 = small pelagics/large pelagics/demersal & benthic).
- `PIM_get_control_parameters(catchability_EEZ, catchability_DWFN,
  controlParamsCsvPath)` — reads the same `control_parameters.csv` (vessel
  attrition, effort-growth constants) and scales the loaded catchability
  arrays in place, exactly as the toy model does.

**The "reference country" simplification**: all of the above are
country/EEZ-indexed (~228 countries, ~153 with an EEZ), because the toy
model itself is a global, multi-country model. Atlantis has no country/EEZ
axis of its own — it models one region. So the loader looks up
`referenceCountryISO3` (e.g. `"AUS"`) once, in both `drivers.countries`
(for the demand-curve arrays) and `drivers.EEZ_countries` (for
access/catchability/compliance/investment-driver arrays — silently
unavailable, left at 0, if the reference country has no EEZ entry), and
every later per-year lookup returns that ONE country's values. Nothing about
the rest-of-world's 227 other countries is used. This means the loaded
driver set answers "what would this one real country's demand curve/
catchability/compliance/investment drivers have looked like this year", not
"what is the market-clearing outcome accounting for global supply and
demand" — the latter is exactly what section 8.2 covers, since it fundamentally
cannot be derived from one country's drivers alone.

**What gets overridden in `OSPBridgeConfig`, per year** (`ApplyLoadedDrivers()`,
called every year from `FishMIP_OSP_Bridge_Annual()`, a no-op if
`LoadDrivers()` was never called or failed):

| `OSPBridgeConfig` field | Loaded from | Per fishery, or once? |
|---|---|---|
| `demand_X_uniform`/`demand_W_uniform` (Step 3's `X`/`W`) | `driver_X_fresh`/`driver_W_fresh` at (reference country position, `commod`, year) | Once — applied to every fished species uniformly (see caveat below) |
| `epsilon` | `drivers.epsilon` (mean of `parameters.csv`'s `epsilon` column) | Once (global scalar in the toy model too) |
| `risk_invest` | `drivers.risk_invest` (`5.0`, hard-coded in `PIM_load_drivers` itself) | Once (global scalar) |
| `f_growth_rate_max`, `f_growth_rate_half_sat`, `effort_growth_lim` | `ControlParameters` from `PIM_get_control_parameters` | Once (global scalars) |
| `catchability[nf]` | `catchability_EEZ(fleetType, eezPos, year)` | Per fishery, using `fisheryFleetType[nf]` |
| `compliance[nf]` | `comply_target_nat(fleetType, eezPos, year)` | Per fishery |
| `invest_driver_U[nf]`/`invest_driver_V[nf]` | `driver_U_national`/`driver_V_national(fleetType, eezPos, year)` | Per fishery |
| `vessel_attrition_M[0]` | `ControlParameters::M[fleetType]` | Set from whichever fishery was processed last in the loop (all Atlantis fisheries share one `M` slot — see "Fleet-dimension mapping" in Step 4's comment; if fisheries mix fleet types this is a known simplification) |

`driver_X_caned`/`driver_W_caned` and `exported_frac` are loaded and exposed
on `OSPDriverYearValues` but not yet consulted anywhere in
`FishMIP_OSP_Bridge_Annual()` — they matter for the toy model's
fresh-vs-canned / DWFN-specific market split, which this bridge does not
model (see section 2); left as an extension point.

**Caveat — one demand curve for every species**: the loaded driver set
carries exactly one commodity's demand curve for one reference country.
Every fished species in Atlantis is given that same `X`/`W` pair (Step 3
still runs `plugin_model_1_calc_price` once per species with its own `prod`
= that species' own catch, so prices still differ species to species — only
the demand curve's shape is shared). A real deployment with several
differently-traded species groups would want to load several commodities
(one `LoadDrivers()` call's worth of state per commodity, extending
`OSPDriverSet` to a small map) and assign each Atlantis species to one — not
implemented here.

**Per-fishery fleet type**: `cfg.fisheryFleetType[nf]` (artisanal = 0,
industrial = 1) picks which of the loaded driver set's two fleet-type rows a
given Atlantis fishery draws from. Atlantis fisheries carry no such label
natively, so every fishery defaults to artisanal (0) — another placeholder,
alongside those in section 5, that a real deployment should source from an
actual per-fishery classification.

### 8.2 `FishMIP_OSP_Bridge_LoadGlobalPrice()` — treating the global market as external input

*"the option to read in a price set by a global market (so the bits PIMS
would normally cover in a global model but that I will need to assume is
external)."*

Call once (or whenever the external series changes) —
`FishMIP_OSP_Bridge_LoadGlobalPrice(csvPath, yearColumn, baseCalendarYear)` —
then pass `priceSource = OSP_PRICE_EXTERNAL_GLOBAL` to
`FishMIP_OSP_Bridge_Annual()`. This does **not** call any `fishmip::`
function — the whole point is that a genuine global-market clearing price
needs the full multi-country supply/demand accounting `PIM_load_drivers()`
+ `plugin_model_1_calc_price()` do across all ~228 countries at once, which
a single-region Atlantis run cannot reproduce (see 8.1's closing paragraph).
Instead of trying, this option lets the price be supplied directly as data.

**CSV format**: a `yearColumn` (e.g. `"year"`) plus one column per Atlantis
functional-group code (`FunctGroupArray[sp].groupCode`, e.g. a column headed
`FPS` for a species with that group code) — or, if no species-specific
column exists, a shared column named `"price"` used for every species that
has no column of its own. Uses the plug-in's own `readCsv()`/
`CsvTable::columnDoubles()` (`csv_utils.hpp`) to parse it — no new CSV
parsing code was written for this. Values between listed years are linearly
interpolated; years before/after the covered range hold at the nearest
endpoint (same convention as 8.1's driver-year lookups).

**How it combines with everything else**: applied as an *override*, right
after Step 3 computes `price_by_sp` via `plugin_model_1_calc_price` as
usual (mode-independent — it applies under mode A, B or C alike, since all
three consult `price_by_sp`). Any species with a usable value in the loaded
series gets that value instead; any species without one keeps the
locally-computed plugin price. So a CSV that only covers e.g. the tuna
species Atlantis fishes is safe to use even though it says nothing about the
model's other fished species.

**Verified**: `readCsv()`/`CsvTable::columnDoubles()` (used by both
`FishMIP_OSP_Bridge_LoadDrivers()`'s and `FishMIP_OSP_Bridge_
LoadGlobalPrice()`'s implementations) were confirmed, along with every other
new `fishmip::` call, to link successfully against the real compiled plug-in
library in the full-link test described in section 3.
