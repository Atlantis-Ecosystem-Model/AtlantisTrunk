#pragma once
// C++ port of FishMIP_OSP_Plugin_Models.py.
//
// Function names, parameter order and semantics deliberately mirror the
// Python module 1:1 so the two can be read side by side; see
// cpp/PORTING_NOTES.md for every place behaviour had to change (numpy/pandas/
// xarray/scipy/numba/joblib -> Eigen-free C++17 + a hand-rolled optimizer
// suite + netCDF-C + a small CSV reader) and why.
//
// Multi-dimensional numpy arrays are represented with NdArray<N> (see
// ndarray.hpp); 1D arrays that are just lists of scalars use std::vector.
// Python's positional tuple returns become named structs so call sites stay
// readable.

#include <array>
#include <cstddef>
#include <optional>
#include <string>
#include <vector>

#include "ndarray.hpp"

namespace fishmip {

// ================================================================ Section 1
// PIM_configure_simulation
struct SimulationConfig {
    int nctry = 228;
    int nsubregion = 17;
    int nregion = 4;
    int nEEZ = 153;
    int nfleet = 2;    // artisanal & industrial
    int nstocks = 0;   // nEEZ + 1 (one stock per EEZ + one ABNJ stock)
    int time_min = 0;
    int time_max = 0;
    std::size_t xsize = 0;
    double EPS = 1e-20;
};

SimulationConfig PIM_configure_simulation(int nosp);

// ================================================================ Section 2
// PIM_get_control_parameters
struct ControlParameters {
    std::array<double, 2> M{};  // (artisanal, industrial) boat-attrition rate, yr^-1
    int effort_growth_lim = 1;
    double f_growth_rate_max = 0.0;
    double f_growth_rate_half_sat = 10.0;
};

// Scales catchability_EEZ / catchability_DWFN IN PLACE by the fixed 2014
// catchability constants (shape (nfleet, nEEZ, xsize) each), exactly as the
// Python function does. Unlike the Python source -- which hard-codes these
// constants (and the vessel-lifespan / effort-growth constants below them)
// in the function body -- this port reads them from a small "name,value"
// CSV (see cpp/config/control_parameters.csv) at `paramsCsvPath`, so they
// can be tuned without recompiling.
ControlParameters PIM_get_control_parameters(NdArray3& catchability_EEZ, NdArray3& catchability_DWFN,
                                              const std::string& paramsCsvPath);

// ================================================================ Section 3
// PIM_configure_stock_assessment
struct StockAssessmentConfig {
    int start_stock_assess = 110;
    bool parall_stock_assess = true;
    int length_stock_assess = 0;  // length of the C/F time series considered in the stock assessment; 0 means the longest available time series
    bool verbose_stock_assessment = false;

    // Here, the x0_prev object is a vector
    // either of empty objects (optional)
    // or of vector
    std::vector<std::optional<std::vector<double>>> x0_prev;  // per-stock warm start, initialised to nullopt for every stock (i.e. for year 1)
};

StockAssessmentConfig PIM_configure_stock_assessment(int nstocks);

// ================================================================ Section 4
// PIM_load_drivers
struct DriverData {
    std::vector<std::string> countries;       // ISO3 codes, IPBES table order (length ~ nctry)
    std::vector<std::string> EEZ_countries;    // unique ISO3 codes with an EEZ (length ~ nEEZ)
    std::vector<std::size_t> EEZ_indices;      // position of each EEZ_countries entry within `countries`
    std::vector<double> EEZ_surf;              // EEZ surface (km^2), aligned with the IPBES/EEZ merge (not de-duplicated)
    std::vector<int> region_idx;               // per-country index into the unique IPBES region codes
    std::vector<int> subregion_idx;            // per-country index into the unique IPBES sub-region codes

    NdArray3 driver_X_fresh;   // (nctry, commodity=4, time) -- raw file country order, NOT reindexed (see notes)
    NdArray3 driver_W_fresh;
    NdArray3 driver_X_caned;
    NdArray3 driver_W_caned;

    NdArray2 exported_frac;             // (nEEZ, time)
    NdArray2 DWFN_invest_capacity_nat;  // (nEEZ, time)
    NdArray3 access_nat;                // (nfleet=2, nEEZ, time)
    NdArray3 driver_U_national;         // (nfleet=2, nEEZ, time)
    NdArray3 driver_V_national;         // (nfleet=2, nEEZ, time)
    NdArray3 catchability_EEZ;          // (nfleet=2, nEEZ, time)
    NdArray3 catchability_DWFN;         // (nfleet=2, nEEZ, time)
    NdArray3 comply_target_nat;         // (nfleet=2, nEEZ, time), already x2 as in Python

    double risk_invest = 5.0;
    double epsilon = 0.0;
    std::vector<double> Qcap_max;  // parameters.csv column, raw row order (length = CSV row count)
    double inter = 0.0;            // hard-coded to 0 in the Python source

    NdArray2 geo_agreg_artisanal_local;   // (nctry, xsize)
    NdArray2 geo_agreg_artisanal_traded;  // (nctry, xsize)
    NdArray2 geo_agreg_industrial;        // (nctry, xsize)
};

DriverData PIM_load_drivers(int commod, int nctry, std::size_t xsize, const std::string& dataDir);

// ================================================================ Section 5
// PIM_define_variables
struct OSPVariables {
    NdArray3 access;                 // (nfleet, nstocks, xsize)
    NdArray3 effort_EEZ;             // (nfleet, nEEZ, xsize)
    NdArray3 effort_DWFN;            // (nfleet, nEEZ, xsize)
    NdArray4 allocate_effort_DWFN;   // (nfleet, nEEZ, nstocks, xsize)
    // (2*nfleet, nstocks, xsize): effort for stock assessment for four
    // fleets -- (artisanal, industrial) x (EEZ, DWFN) -- in every stock, every year.
    NdArray3 effort_for_stock_assess;

    NdArray2 catch_step_EEZ;         // (nfleet, nEEZ)
    NdArray3 catch_EEZ;              // (nfleet, nEEZ, xsize)
    NdArray3 catch_step_DWFN;        // (nfleet, nEEZ, nstocks)
    NdArray2 catch_step_DWFN_tot;    // (nfleet, nEEZ)
    NdArray3 catch_DWFN;             // (nfleet, nEEZ, xsize)
    NdArray2 catch_step_artis_local_allctry;   // (ncountry_coord, xsize)
    NdArray2 catch_step_artis_traded_allctry;  // (ncountry_coord, xsize)
    NdArray2 catch_step_indus_allctry;         // (ncountry_coord, xsize)
    // (2*nfleet, nstocks, xsize): catches for stock assessment for four
    // fleets -- (artisanal, industrial) x (EEZ, DWFN) -- in every stock, every year.
    NdArray3 catch_for_stock_assess;

    NdArray2 F_init;                 // (nfleet, nstocks)
    NdArray3 F_fleet_EEZ;            // (nfleet, nEEZ, xsize): F from artisanal and industrial local (EEZ-based) fleet in every EEZ
    NdArray3 F_fleet_DWFN;           // (nfleet, nEEZ, xsize): F from artisanal and industrial DWFN fleet from every coastal country (EEZ)
    NdArray4 F_DWFN_allocate;        // (nfleet, nEEZ, nstocks, xsize): F from artisanal and industrial DWFN fleet from every coastal country, in every stock (EEZs + ABNJ)
    NdArray2 F_tot;                  // (nstocks, xsize)
    std::vector<double> prop_EEZ_DWFN;  // (nstocks), initialised to 1

    // (nstocks): EEZ stocks are exploited by 4 fleets -- (artisanal, industrial) x (EEZ, DWFN) -- and the ABNJ stock by 2 (DWFN only).
    std::vector<int> nfleets_per_stock;
    std::vector<double> prop2target;     // (nstocks), initialised to 1
    std::vector<double> Fmsy_est;        // (nstocks), initialised from Fmsy
    std::vector<double> MSY_est;         // (nstocks), uninitialised (mirrors np.empty)
    std::vector<double> global_MSY;      // (xsize), constant = MSY[commod]
    std::vector<double> global_FMSY;     // (xsize), constant = sum(Fmsy)
    std::vector<double> target_EEZ;         // (nEEZ)
    std::vector<double> target_DWFN_fleet;  // (nEEZ)
    std::vector<double> target_DWFN_stocks; // (nstocks)
    std::vector<double> target_DWFN_init;   // (nEEZ), initialised to 1

    NdArray3 price_EEZ;   // (nfleet, nEEZ, xsize)
    NdArray3 price_DWFN;  // (nfleet, nEEZ, xsize)
    NdArray2 price_artisanal_traded_allctry;   // (ncountry_coord, xsize)
    NdArray2 price_artisanal_local_allctry;    // (ncountry_coord, xsize)
    NdArray2 price_industrial_allctry;         // (ncountry_coord, xsize)
    std::vector<double> ratio_exported_over_local_artisanal_prices;  // (nEEZ), initialised to 0 (ratio of exported over local artisanal prices)

    NdArray2 consumption_artisanal_traded_allctry;  // (ncountry_coord, xsize)
    NdArray2 consumption_artisanal_local_allctry;   // (ncountry_coord, xsize)
    NdArray2 consumption_industrial_allctry;        // (ncountry_coord, xsize)

    NdArray2 prod_aqua;  // (nctry, xsize)
};

// MSY is indexed by `commod`: (small pelagics, oceanic predators, demersal &
// benthic), in tonnes/yr -- e.g. (4.E7, 6.5E6, 3E6) in the Python driving
// script. `Fmsy` is per-stock (length nstocks).
OSPVariables PIM_define_variables(int nfleet, int nctry, int nstocks, int nEEZ, std::size_t xsize,
                                   const std::vector<std::string>& country_coord, const NdArray3& access_nat,
                                   const std::vector<double>& Fmsy, const std::array<double, 3>& MSY, int commod);

// ================================================================ Section 6
// PIM_calculate_prices_consumptions / plugin_model_1_calc_price
struct PriceCalcResult {
    std::vector<double> price_ctry;
    std::vector<double> consumption_ctry;
};

// geo_agreg encodes the market's geographic extent per country: 0 = national,
// 1 = subregional, 2 = regional, 3 = global (PCHIP-interpolated in between).
PriceCalcResult plugin_model_1_calc_price(const std::vector<double>& geo_agreg, int nctry, int nregion, int nsubregion,
                                           const std::vector<int>& region_idx, const std::vector<int>& subregion_idx,
                                           const std::vector<double>& prod, const std::vector<double>& X,
                                           const std::vector<double>& W, double epsilon,
                                           const std::vector<double>& aqua);

struct PriceConsumptionStepResult {
    std::vector<double> price_artisanal_traded;
    std::vector<double> consumption_artisanal_traded;
    std::vector<double> price_industrial;
    std::vector<double> consumption_industrial;
    std::vector<double> price_artisanal_local;
    std::vector<double> consumption_artisanal_local;
    NdArray2 price_EEZ_t;   // (nfleet, nEEZ) == price_EEZ[:, :, t]
    NdArray2 price_DWFN_t;  // (nfleet, nEEZ) == price_DWFN[:, :, t]
    std::vector<double> ratio_exported_over_local_artisanal_prices;  // (nEEZ)
};

// Mutates price_EEZ, price_DWFN and the *_allctry tables at column t in place
// (as the Python version does via xarray .loc assignment), and returns the
// same values the Python function returns explicitly.
PriceConsumptionStepResult PIM_calculate_prices_consumptions(
    NdArray3& price_EEZ, NdArray3& price_DWFN, NdArray2& catch_step_artis_traded_allctry,
    NdArray2& catch_step_indus_allctry, NdArray2& catch_step_artis_local_allctry,
    NdArray2& price_artisanal_traded_allctry, NdArray2& consumption_artisanal_traded_allctry,
    NdArray2& price_industrial_allctry, NdArray2& consumption_industrial_allctry,
    NdArray2& price_artisanal_local_allctry, NdArray2& consumption_artisanal_local_allctry,
    const NdArray2& exported_frac, const NdArray2& catch_step_EEZ, const NdArray3& catch_step_DWFN,
    const std::vector<std::size_t>& eezPosInCountryCoord, const NdArray2& geo_agreg_artisanal_traded,
    const NdArray2& geo_agreg_industrial, const NdArray2& geo_agreg_artisanal_local, int nctry, int nregion,
    int nsubregion, const std::vector<int>& region_idx, const std::vector<int>& subregion_idx,
    const std::vector<double>& driv_X_f, const std::vector<double>& driv_W_f, const std::vector<double>& driv_X_c,
    const std::vector<double>& driv_W_c, double epsilon, const std::vector<double>& inter_prod_aq, double EPS,
    std::size_t t, double dt, int commod);

// ================================================================ Section 7
// mini_model_2_calc_aqua
std::vector<double> mini_model_2_calc_aqua(const std::vector<double>& price, const std::vector<double>& Aprod,
                                            const std::vector<double>& d_U, const std::vector<double>& d_V, double d_t);

// ================================================================ Section 8
// plugin_model_2_calc_effort
struct EffortCalcResult {
    NdArray2 f;                  // (nfleet_type, nfisheries) updated effort
    NdArray2 F;                  // (nfleet_type, nfisheries) fishing mortality per fleet
    std::vector<double> Ftot;    // (nfisheries) total fishing mortality
};

EffortCalcResult plugin_model_2_calc_effort(int nfleet_type, int nfisheries, const NdArray2& price, const NdArray2& Y,
                                             const NdArray2& effo, const NdArray2& q, const std::vector<double>& Ftarget,
                                             const NdArray2& comply, double risk, const NdArray2& d_U, const NdArray2& d_V,
                                             const std::vector<double>& M, double d_t, int effort_growth_lim,
                                             double f_growth_rate_max, double f_growth_rate_half_sat);

// ================================================================ Section 9
// dnorm_log
double dnorm_log(double obs, double pred, double sd);

// ================================================================ Section 10
// dynamics_loop (was @njit-compiled in Python; plain C++ needs no such hint)
struct DynamicsLoopResult {
    std::vector<double> B;      // length n+1 (B[0] is the caller-supplied initial biomass, usually K)
    std::vector<double> Bmoy;   // length n
    NdArray2 Cpred;             // (n, nf)
    std::vector<double> F_tot;  // length n
    double pen = 0.0;           // penalty accumulated from negative-biomass excursions
};

DynamicsLoopResult dynamics_loop(std::vector<double> B, double r, double k, double z, const std::vector<double>& q,
                                  const NdArray2& E, int n, int nf);

// ================================================================ Section 11
// objective (negative log-likelihood for the Pella-Tomlinson stock assessment)
struct StockData {
    int nfleets = 0;
    double prior_mean_r = 0.0;
    double prior_sd_logr = 1.0;
    double prior_mean_z = 0.0;
    double prior_sd_logz = 1.0;
    NdArray2 C;  // (n, nfleets) observed catches
    NdArray2 E;  // (n, nfleets) observed effort
};

// log_params (log-space): [logR, logK, logz, logQ_0..logQ_nf-1,
// logSigma_CPUE_0..logSigma_CPUE_nf-1, logSigma_C_0..logSigma_C_nf-1]
// For each fleet i and year t:
//   F_i(t)      = q_i * E_i(t)
//   F_total(t)  = sum_i F_i(t)
//   C_i_pred(t) = [F_i(t)/F_total(t)] * Expl(t) * B(t)   -- catch allocation
//   CPUE_i_pred = q_i * B_avg(t)                         -- implicit abundance index
// Returns the negative log-likelihood, based on log(CPUE_obs) and log(C_obs).
double objective(const std::vector<double>& log_params, const StockData& data);

// ================================================================ Section 12
// fit_one_stock / plugin_model_3_assess_stock
struct FitResult {
    double r = 0.0, k = 0.0, z = 0.0;
    std::vector<double> q, sigma_CPUE, sigma_C;
    double MSY = 0.0, FMSY = 0.0, BMSY = 0.0, MSYLOut = 0.0;
    std::vector<double> B, Bmoy, F;
    NdArray2 Cpred;
    double nll = 0.0;
    bool convergence = false;
    std::vector<double> x0;  // final optimizer state (log-space), for warm-starting the next year's fit
};

// Fits the Pella & Tomlinson stock assessment model: dB/dt = r.B.(1 - (B/K)^z) - F.B
//
// The priors on r & z (StockData::prior_mean_r/z, prior_sd_logr/z) must be
// set > 0 according to the biology of the species considered:
//   r < 0.05: long-lived species with low productivity (sharks, tuna)
//   r ~ 0.3:  medium-sized species (cod, hake)
//   r > 1.0:  small, highly productive pelagic species (anchovies, sardines)
//   r > 2.0 is virtually non-existent amongst commercially fished marine fish
//   Fox -> z = 0.1, Schaefer -> z = 1.
// Priors are lognormal (set in log-space) to keep r and z positive and
// asymmetric: r/z +/- SD = exp(log(mean) +/- sd).
//
// Minimisation algorithm selector, mirroring `mini` in the Python source:
//   0 Nelder-Mead
//   1 (parallel) differential evolution
//   2 L-BFGS-B
//   3 differential evolution -> automatic L-BFGS-B polish
//   4 differential evolution (parallel-style RNG use) -> explicit L-BFGS-B refinement
//   5 differential evolution (serial-style RNG use) -> explicit L-BFGS-B refinement
FitResult fit_one_stock(const StockData& data, int mini, bool verbose,
                         const std::optional<std::vector<double>>& x0_prev,
                         std::optional<double> K0 = std::nullopt);

FitResult plugin_model_3_assess_stock(int i, std::size_t t, const std::vector<int>& nfleets_per_stock,
                                       const NdArray3& catch_for_stock_assess, const NdArray3& effort_for_stock_assess,
                                       const std::vector<double>& r_pop, double m, int nfleet, int length, int mini1,
                                       int mini2, bool verbose, const std::optional<std::vector<double>>& x0_prev);

// ================================================================ Section 13
// PIM_assess_stocks
struct StockAssessResult {
    std::vector<double> Fmsy_est;
    std::vector<double> MSY_est;
    std::vector<std::optional<std::vector<double>>> x0_prev;
    double sum_MSY = 0.0;
    double sum_FMSY = 0.0;
};

// parallel_mode: 0 = sequential, 1 = parallel across stocks (std::thread pool
// standing in for joblib.Parallel; see PORTING_NOTES.md).
StockAssessResult PIM_assess_stocks(std::vector<double> Fmsy_est, std::vector<double> MSY_est, int parallel_mode,
                                     int nstocks, std::size_t t, std::size_t start_stock_assess,
                                     const std::vector<int>& nfleets_per_stock, const NdArray3& catch_for_stock_assess,
                                     const NdArray3& effort_for_stock_assess, const std::vector<double>& r_pop, double m,
                                     int nfleet, int length_stock_assess, bool verbose_stock_assessment,
                                     std::vector<std::optional<std::vector<double>>> x0_prev);

// ================================================================ Section 14
// PIM_allocate_target_local_DWFN
struct AllocateTargetResult {
    std::vector<double> target_EEZ;
    std::vector<double> target_DWFN_stocks;
    std::vector<double> target_DWFN_fleet;
};

// NOTE (carried over from the Python source): this PIM is expected to be
// revised later. `stock_surface` is specific to the Toy Model.
AllocateTargetResult PIM_allocate_target_local_DWFN(std::size_t t, int nEEZ, const std::vector<double>& Fmsy_est,
                                                     const NdArray3& F_fleet_EEZ, const NdArray3& F_fleet_DWFN,
                                                     const NdArray4& F_DWFN_allocate, const NdArray2& F_tot,
                                                     const std::vector<double>& prop2target,
                                                     const std::vector<double>& stock_surface);

}  // namespace fishmip
