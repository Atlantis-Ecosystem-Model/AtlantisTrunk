/**
 * \file atFishMIP_OSP_PluginAdapter.cpp
 * \ingroup ateconomic
 *
 * Implements atFishMIP_OSP_PluginAdapter.h -- the ONLY translation unit in
 * Atlantis that includes FishMIP_OSP_Plugin_Models.hpp / ndarray.hpp /
 * csv_utils.hpp, and therefore the only place that touches the plug-in
 * library's C++-only types (NdArray, StockData, DriverData, std::vector,
 * std::optional, ...). Every fishmip:: call the bridge needs is marshalled
 * here, to and from the plain C scalars/arrays/structs declared in
 * atFishMIP_OSP_PluginAdapter.h. Nothing in the plug-in model's own files is
 * copied, reimplemented, or altered.
 *
 * Deliberately does NOT include any Atlantis header (atlantisboxmodel.h,
 * atEconomic.h, ...) -- it has no need to see MSEBoxModel or any other
 * Atlantis type. That also means it never collides with atlantisboxmodel.h's
 * `export` struct field (a C++ reserved word), unlike the single-file
 * (atFishMIP_OSP_Bridge.cpp) version this was split out of, which had to
 * work around that collision locally.
 *
 * See ../ateconomic/FishMIP_OSP_Bridge_NOTES.md for the original single-file
 * bridge and the reasoning behind this split (question raised 2026-09-08,
 * split applied same day).
 */

#include "atFishMIP_OSP_PluginAdapter.h"

#include <algorithm>
#include <cstddef>
#include <optional>
#include <stdexcept>
#include <string>
#include <unordered_map>
#include <vector>

#include "FishMIP_OSP_Plugin_Models.hpp"
#include "csv_utils.hpp"

using namespace fishmip;

namespace {

/* ==========================================================================
 * Driver set (FishMIP_Adapter_LoadDrivers / GetDriverValuesForYear).
 * ========================================================================== */
struct OSPDriverSet {
    bool loaded = false;
    int baseCalendarYear = 0;
    SimulationConfig sim;
    DriverData drivers;
    ControlParameters controlParams;
    int commod = 0;
    std::size_t countryPos = 0;  // reference country's position in drivers.countries (nctry-indexed arrays)
    std::size_t eezPos = 0;      // reference country's position in drivers.EEZ_countries (nEEZ-indexed arrays)
    bool hasEezPos = false;      // false if the reference country isn't in the EEZ list (landlocked/no EEZ)
};

/** The one loaded driver set for this run (there is only ever one Atlantis
 *  model domain per run, so one reference country is enough). */
OSPDriverSet &GlobalDriverSet() {
    static OSPDriverSet ds;
    return ds;
}

/* ==========================================================================
 * External global-market price (FishMIP_Adapter_LoadGlobalPrice).
 * ========================================================================== */
struct OSPGlobalPriceSeries {
    bool loaded = false;
    int baseCalendarYear = 0;
    std::vector<int> years;
    std::unordered_map<std::string, std::vector<double>> pricesByColumn;  // column name (species groupCode, or "price") -> series aligned with `years`
};

OSPGlobalPriceSeries &GlobalPriceSeriesStore() {
    static OSPGlobalPriceSeries s;
    return s;
}

/** Per-species warm-start cache for fit_one_stock()'s optimizer state,
 *  keyed by the caller's speciesId (the Atlantis species index) and
 *  persisted across annual calls within this run -- mirrors the toy model's
 *  own year-to-year x0_prev threading. */
std::optional<std::vector<double>> &FitWarmStartFor(int speciesId) {
    static std::vector<std::optional<std::vector<double>>> bySpecies;
    if (static_cast<int>(bySpecies.size()) <= speciesId)
        bySpecies.resize(static_cast<std::size_t>(speciesId) + 1, std::nullopt);
    return bySpecies[static_cast<std::size_t>(speciesId)];
}

}  // namespace

/* ==========================================================================
 * Driver loading.
 * ========================================================================== */
int FishMIP_Adapter_LoadDrivers(const char *dataDir, const char *controlParamsCsvPath,
                                 const char *referenceCountryISO3, int commod, int nosp, int baseCalendarYear) {
    OSPDriverSet &ds = GlobalDriverSet();
    ds = OSPDriverSet{};  // reset in case of a previous failed/partial load
    try {
        ds.sim = PIM_configure_simulation(nosp);
        ds.drivers = PIM_load_drivers(commod, ds.sim.nctry, ds.sim.xsize, dataDir);
        ds.controlParams =
            PIM_get_control_parameters(ds.drivers.catchability_EEZ, ds.drivers.catchability_DWFN, controlParamsCsvPath);
        ds.commod = commod;
        ds.baseCalendarYear = baseCalendarYear;

        const std::string ref(referenceCountryISO3);
        auto itC = std::find(ds.drivers.countries.begin(), ds.drivers.countries.end(), ref);
        if (itC == ds.drivers.countries.end()) {
            ds = OSPDriverSet{};
            return 0;  // reference country not found in the loaded IPBES/country table -- nothing usable to cache
        }
        ds.countryPos = static_cast<std::size_t>(itC - ds.drivers.countries.begin());

        auto itE = std::find(ds.drivers.EEZ_countries.begin(), ds.drivers.EEZ_countries.end(), ref);
        ds.hasEezPos = (itE != ds.drivers.EEZ_countries.end());
        if (ds.hasEezPos) ds.eezPos = static_cast<std::size_t>(itE - ds.drivers.EEZ_countries.begin());

        ds.loaded = true;
        return 1;
    } catch (const std::exception &) {
        // PIM_load_drivers()/PIM_get_control_parameters() throw std::runtime_error
        // on a missing file or a required control_parameters.csv key -- treat
        // either as "no drivers available", same as never having called this.
        ds = OSPDriverSet{};
        return 0;
    }
}

int FishMIP_Adapter_DriversLoaded(void) { return GlobalDriverSet().loaded ? 1 : 0; }

int FishMIP_Adapter_GetDriverBaseCalendarYear(void) { return GlobalDriverSet().baseCalendarYear; }

void FishMIP_Adapter_GetDriverValuesForYear(int calendarYear, int fleetType, FishMIP_DriverYearValues *out) {
    *out = FishMIP_DriverYearValues{};
    const OSPDriverSet &ds = GlobalDriverSet();
    if (!ds.loaded) return;

    long t = static_cast<long>(calendarYear) - static_cast<long>(ds.sim.time_min);
    if (t < 0) t = 0;
    if (t >= static_cast<long>(ds.sim.xsize)) t = static_cast<long>(ds.sim.xsize) - 1;
    const std::size_t tt = static_cast<std::size_t>(t);
    const std::size_t ft = static_cast<std::size_t>(fleetType == 1 ? 1 : 0);

    out->epsilon = ds.drivers.epsilon;
    out->risk_invest = ds.drivers.risk_invest;
    out->vessel_attrition_M = ds.controlParams.M[ft];
    out->f_growth_rate_max = ds.controlParams.f_growth_rate_max;
    out->f_growth_rate_half_sat = ds.controlParams.f_growth_rate_half_sat;
    out->effort_growth_lim = ds.controlParams.effort_growth_lim;

    out->demand_X_fresh = ds.drivers.driver_X_fresh(ds.countryPos, static_cast<std::size_t>(ds.commod), tt);
    out->demand_W_fresh = ds.drivers.driver_W_fresh(ds.countryPos, static_cast<std::size_t>(ds.commod), tt);
    out->demand_X_caned = ds.drivers.driver_X_caned(ds.countryPos, static_cast<std::size_t>(ds.commod), tt);
    out->demand_W_caned = ds.drivers.driver_W_caned(ds.countryPos, static_cast<std::size_t>(ds.commod), tt);

    if (ds.hasEezPos) {
        out->exported_frac = ds.drivers.exported_frac(ds.eezPos, tt);
        out->catchability_EEZ = ds.drivers.catchability_EEZ(ft, ds.eezPos, tt);
        out->compliance = ds.drivers.comply_target_nat(ft, ds.eezPos, tt);
        out->invest_driver_U = ds.drivers.driver_U_national(ft, ds.eezPos, tt);
        out->invest_driver_V = ds.drivers.driver_V_national(ft, ds.eezPos, tt);
    }
    out->ok = 1;
}

/* ==========================================================================
 * External global-market price.
 * ========================================================================== */
int FishMIP_Adapter_LoadGlobalPrice(const char *csvPath, const char *yearColumn, int baseCalendarYear) {
    OSPGlobalPriceSeries &s = GlobalPriceSeriesStore();
    s = OSPGlobalPriceSeries{};
    try {
        CsvTable t = readCsv(csvPath, ',');
        std::vector<double> yearsD = t.columnDoubles(yearColumn);
        if (yearsD.empty()) return 0;
        s.years.reserve(yearsD.size());
        for (double y : yearsD) s.years.push_back(static_cast<int>(y));
        for (const std::string &h : t.headers) {
            if (h == yearColumn) continue;
            s.pricesByColumn[h] = t.columnDoubles(h);
        }
        s.baseCalendarYear = baseCalendarYear;
        s.loaded = !s.pricesByColumn.empty();
        if (!s.loaded) s = OSPGlobalPriceSeries{};
        return s.loaded ? 1 : 0;
    } catch (const std::exception &) {
        s = OSPGlobalPriceSeries{};
        return 0;
    }
}

int FishMIP_Adapter_GetPriceBaseCalendarYear(void) { return GlobalPriceSeriesStore().baseCalendarYear; }

double FishMIP_Adapter_GetGlobalPriceForYear(const char *speciesGroupCode, int calendarYear) {
    const OSPGlobalPriceSeries &s = GlobalPriceSeriesStore();
    if (!s.loaded || s.years.empty()) return -1.0;

    auto it = s.pricesByColumn.find(speciesGroupCode);
    if (it == s.pricesByColumn.end()) it = s.pricesByColumn.find("price");
    if (it == s.pricesByColumn.end()) return -1.0;
    const std::vector<double> &series = it->second;
    if (series.size() != s.years.size()) return -1.0;

    if (calendarYear <= s.years.front()) return series.front();
    if (calendarYear >= s.years.back()) return series.back();
    for (std::size_t i = 0; i + 1 < s.years.size(); ++i) {
        if (calendarYear >= s.years[i] && calendarYear <= s.years[i + 1]) {
            if (s.years[i] == calendarYear) return series[i];
            double frac = static_cast<double>(calendarYear - s.years[i]) / static_cast<double>(s.years[i + 1] - s.years[i]);
            return series[i] + frac * (series[i + 1] - series[i]);
        }
    }
    return series.back();
}

/* ==========================================================================
 * Price / effort / assessment calls.
 * ========================================================================== */
void FishMIP_Adapter_CalcPrice(int nsp, const double *prod, const double *X, const double *W, double epsilon,
                                double *price_out) {
    const std::size_t n = static_cast<std::size_t>(nsp);
    std::vector<double> geo_agreg(n, 0.0);
    std::vector<double> prodV(prod, prod + n), Xv(X, X + n), Wv(W, W + n), aqua(n, 0.0);
    std::vector<int> region_idx(n), subregion_idx(n);
    for (std::size_t k = 0; k < n; ++k) region_idx[k] = subregion_idx[k] = static_cast<int>(k);

    PriceCalcResult pr =
        plugin_model_1_calc_price(geo_agreg, nsp, nsp, nsp, region_idx, subregion_idx, prodV, Xv, Wv, epsilon, aqua);
    std::copy(pr.price_ctry.begin(), pr.price_ctry.end(), price_out);
}

void FishMIP_Adapter_CalcEffort(int nfishery, const double *price, const double *Y, const double *effo,
                                 const double *q, const double *Ftarget, const double *comply, double risk,
                                 const double *d_U, const double *d_V, double M, double dt, int effort_growth_lim,
                                 double f_growth_rate_max, double f_growth_rate_half_sat, double *f_out) {
    const std::size_t nf = static_cast<std::size_t>(nfishery);
    auto toRow = [nf](const double *src) {
        NdArray2 a({1, nf}, 0.0);
        for (std::size_t i = 0; i < nf; ++i) a(0, i) = src[i];
        return a;
    };
    NdArray2 priceA = toRow(price), Ya = toRow(Y), effoA = toRow(effo), qA = toRow(q), complyA = toRow(comply),
             dUA = toRow(d_U), dVA = toRow(d_V);
    std::vector<double> FtargetV(Ftarget, Ftarget + nf);
    std::vector<double> Mv{M};  // nfleet_type collapsed to 1 -- see the header doc comment

    EffortCalcResult ec = plugin_model_2_calc_effort(1, nfishery, priceA, Ya, effoA, qA, FtargetV, complyA, risk, dUA,
                                                      dVA, Mv, dt, effort_growth_lim, f_growth_rate_max,
                                                      f_growth_rate_half_sat);
    for (std::size_t i = 0; i < nf; ++i) f_out[i] = ec.f(0, i);
}

void FishMIP_Adapter_RunAssessment(int speciesId, int nyears, int nfishery, const double *catchFlat,
                                    const double *effortFlat, double prior_mean_r, double prior_sd_logr,
                                    double prior_mean_z, double prior_sd_logz, int optimizer, int verbose,
                                    FishMIP_AssessmentResult *out) {
    *out = FishMIP_AssessmentResult{0, 0.0, 0.0};

    const std::size_t ny = static_cast<std::size_t>(nyears), nf = static_cast<std::size_t>(nfishery);
    NdArray2 C({ny, nf}, 0.0), E({ny, nf}, 0.0);
    for (std::size_t iy = 0; iy < ny; ++iy)
        for (std::size_t nfi = 0; nfi < nf; ++nfi) {
            C(iy, nfi) = catchFlat[iy * nf + nfi];
            E(iy, nfi) = effortFlat[iy * nf + nfi];
        }

    StockData sd;
    sd.nfleets = nfishery;
    sd.prior_mean_r = prior_mean_r;
    sd.prior_sd_logr = prior_sd_logr;
    sd.prior_mean_z = prior_mean_z;
    sd.prior_sd_logz = prior_sd_logz;
    sd.C = C;
    sd.E = E;

    std::optional<std::vector<double>> &warm = FitWarmStartFor(speciesId);
    FitResult fit = fit_one_stock(sd, optimizer, verbose != 0, warm);
    warm = fit.x0;  // warm-start next year's fit, same convention as the toy model's x0_prev

    if (!fit.convergence || !(fit.FMSY > 0.0) || !(fit.MSY > 0.0)) return;
    out->ok = 1;
    out->FMSY = fit.FMSY;
    out->MSY = fit.MSY;
}
