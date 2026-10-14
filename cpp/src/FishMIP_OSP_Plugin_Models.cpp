#include "FishMIP_OSP_Plugin_Models.hpp"

#include <algorithm>
#include <atomic>
#include <cmath>
#include <filesystem>
#include <iostream>
#include <limits>
#include <numeric>
#include <set>
#include <stdexcept>
#include <thread>
#include <unordered_map>

#include "csv_utils.hpp"
#include "netcdf_utils.hpp"
#include "optim.hpp"
#include "pchip.hpp"

namespace fs = std::filesystem;

namespace fishmip {

    namespace {
        constexpr double kPi = 3.14159265358979323846;
        double sumVec(const std::vector<double> &v) { return std::accumulate(v.begin(), v.end(), 0.0); }
    } // namespace

    // ============================================================== Section 1
    // initialize the time constants of the simulation and the other constants considered in the PIMs
    // ==============================================================
    SimulationConfig PIM_configure_simulation(int nosp) {
        SimulationConfig cfg;
        cfg.nctry = 228;
        cfg.nsubregion = 17;
        cfg.nregion = 4;
        cfg.nEEZ = 153;
        cfg.nfleet = 2; // artisanal & industrial

        const int time_hist_min = 1850, time_hist_max = 2014;
        const std::size_t xsize_hist = static_cast<std::size_t>(time_hist_max - time_hist_min + 1);
        const int time_ssp_min = 2015, time_ssp_max = 2100;
        const std::size_t xsize_ssp = static_cast<std::size_t>(time_ssp_max - time_ssp_min + 1);

        if (nosp == 0) {
            std::cout << "=========================" << std::endl;
            std::cout << "Simulate historical (OSP0)" << std::endl;
            std::cout << "=========================" << std::endl;
            cfg.time_min = time_hist_min;
            cfg.time_max = time_hist_max;
            cfg.xsize = xsize_hist;
        } else {
            std::cout << "=============================" << std::endl;
            std::cout << "Simulate OSP" << nosp << "" << std::endl;
            std::cout << "=============================" << std::endl;
            cfg.time_min = time_ssp_min;
            cfg.time_max = time_ssp_max;
            cfg.xsize = xsize_hist + xsize_ssp;
        }
        cfg.nstocks = cfg.nEEZ + 1; // one stock per EEZ + one ABNJ stock
        cfg.EPS = 1e-20;
        return cfg;
    }

    // ============================================================== Section 2
    // Set catchabilities & other control parameters
    // ==============================================================
    ControlParameters PIM_get_control_parameters(NdArray3 &catchability_EEZ, NdArray3 &catchability_DWFN,
                                                 const std::string &paramsCsvPath) {
        // The Python source hard-codes these as literals in the function body
        // (lines 74-89); this port instead reads them from a small "name,value"
        // CSV so they can be tuned without recompiling. See
        // cpp/config/control_parameters.csv and PORTING_NOTES.md.
        std::unordered_map<std::string, double> p = readKeyValueCsv(paramsCsvPath);
        auto get = [&](const std::string &key) { return requireParam(p, key, paramsCsvPath); };

        const double catchability_artisanal_EEZ_2014 = get("catchability_artisanal_EEZ_2014");
        const double catchability_industrial_EEZ_2014 = get("catchability_industrial_EEZ_2014");
        const double catchability_artisanal_DWFN_2014 = get("catchability_artisanal_DWFN_2014");
        const double catchability_industrial_DWFN_2014 = get("catchability_industrial_DWFN_2014");

        const std::size_t nEEZ_a = catchability_EEZ.extent(1), xsize_a = catchability_EEZ.extent(2);
        for (std::size_t e = 0; e < nEEZ_a; ++e) {
            for (std::size_t t = 0; t < xsize_a; ++t) {
                catchability_EEZ(0, e, t) *= catchability_artisanal_EEZ_2014;
                catchability_EEZ(1, e, t) *= catchability_industrial_EEZ_2014;
            }
        }
        const std::size_t nEEZ_d = catchability_DWFN.extent(1), xsize_d = catchability_DWFN.extent(2);
        for (std::size_t e = 0; e < nEEZ_d; ++e) {
            for (std::size_t t = 0; t < xsize_d; ++t) {
                catchability_DWFN(0, e, t) *= catchability_artisanal_DWFN_2014;
                catchability_DWFN(1, e, t) *= catchability_industrial_DWFN_2014;
            }
        }

        const double lifespan_artis = get("lifespan_artis"); // life span of artisanal fishing vessel, in years
        const double lifespan_indus = get("lifespan_indus"); // life span of industrial fishing vessel, in years
        const double boat_remain = get("boat_remain");       // fraction of boats remaining after lifespan

        ControlParameters cp;
        cp.M = {-std::log(boat_remain) / lifespan_artis, -std::log(boat_remain) / lifespan_indus};
        cp.effort_growth_lim = static_cast<int>(get("effort_growth_lim")); // effort growth limitation if 1, else = no limitation
        cp.f_growth_rate_max = std::pow(get("f_growth_rate_max_base"),
                                        1.0 / get("f_growth_rate_max_root")); // maximum growth rate of fishing effort (yr-1)
        // half saturation of the effort growth rate = f_growth_rate_max / (2 * f_growth_rate_half_sat);
        // must be positive -- when >1 limits investment when df/dt is large, and increases it when df/dt is small.
        cp.f_growth_rate_half_sat = get("f_growth_rate_half_sat");
        return cp;
    }

    // ============================================================== Section 3
    // Configure stock assessment
    // ==============================================================
    StockAssessmentConfig PIM_configure_stock_assessment(int nstocks) {
        StockAssessmentConfig cfg;
        cfg.start_stock_assess = 110;
        cfg.parall_stock_assess = true;
        cfg.length_stock_assess = 0; // Length of the C / F time serie considered in the stock assessment. 0 means the longest available time serie
        cfg.verbose_stock_assessment = false;
        cfg.x0_prev.assign(static_cast<std::size_t>(nstocks), std::nullopt); // initialise x0 for stock assessment with none (for 1st year)
        return cfg;
    }

    // ============================================================== Section 4
    // Read drivers
    // ==============================================================
    namespace {

        // Equivalent of numpy.unique(vals, return_inverse=True): returns (1) the
        // sorted list of distinct values and (2) an array of indices indicating
        // which distinct value each element of `vals` corresponds to.
        std::pair<std::vector<double>, std::vector<int>> uniqueInverse(const std::vector<double> &vals) {
            std::vector<double> uniq = vals;
            std::sort(uniq.begin(), uniq.end());
            uniq.erase(std::unique(uniq.begin(), uniq.end()), uniq.end());
            std::vector<int> idx(vals.size());
            for (std::size_t i = 0; i < vals.size(); ++i) {
                idx[i] = static_cast<int>(std::lower_bound(uniq.begin(), uniq.end(), vals[i]) - uniq.begin());
            }
            return {uniq, idx};
        }

        NdArray3 loadFull3D(const fs::path &path) {
            NcDataset ds(path.string());
            NcVariable v = ds.readMainVariable();
            if (v.shape.size() != 3)
                throw std::runtime_error("expected a 3D variable in " + path.string());
            NdArray3 arr({v.shape[0], v.shape[1], v.shape[2]});
            std::copy(v.data.begin(), v.data.end(), arr.raw().begin());
            return arr;
        }

        NdArray2 loadSelected2D(const fs::path &path, const std::string &coordName, const std::vector<std::string> &wanted) {
            NcDataset ds(path.string());
            NcVariable v = ds.readMainVariable(); // (coord, time)
            std::vector<std::string> labels = ds.readStringCoord(coordName);
            NcVariable sel = selectByLabelAxis(v, 0, labels, wanted);
            NdArray2 arr({sel.shape[0], sel.shape[1]});
            std::copy(sel.data.begin(), sel.data.end(), arr.raw().begin());
            return arr;
        }

        NdArray3 loadSelected3D(const fs::path &path, const std::string &coordName, const std::vector<std::string> &wanted) {
            NcDataset ds(path.string());
            NcVariable v = ds.readMainVariable(); // (fleet, coord, time)
            std::vector<std::string> labels = ds.readStringCoord(coordName);
            NcVariable sel = selectByLabelAxis(v, 1, labels, wanted);
            NdArray3 arr({sel.shape[0], sel.shape[1], sel.shape[2]});
            std::copy(sel.data.begin(), sel.data.end(), arr.raw().begin());
            return arr;
        }

    } // namespace

    DriverData PIM_load_drivers(int commod, int nctry, std::size_t xsize, const std::string &dataDir) {
        fs::path DATA_DIR(dataDir);
        DriverData out;

        CsvTable codes_IPBES = readCsv((DATA_DIR / "IPBES_regions_subregions_country_extended_UTF8_cor.csv").string(), ';');
        std::vector<std::string> countries = codes_IPBES.columnStrings("Country ISO");
        std::vector<std::string> countryNamesIPBES = codes_IPBES.columnStrings("Country (IPBES)");
        std::vector<double> regionsD = codes_IPBES.columnDoubles("IPBES region code");
        std::vector<double> subregionsD = codes_IPBES.columnDoubles("IPBES sub-region code"); // unique_regions: the list of distinct regions
                                                                                              //  region_idx: an array of indices indicating which unique region each country belongs to
        auto [uniqueRegions, region_idx] = uniqueInverse(regionsD);
        auto [uniqueSubregions, subregion_idx] = uniqueInverse(subregionsD);
        (void)uniqueRegions;
        (void)uniqueSubregions;

        CsvTable EEZ_ctry = readCsv((DATA_DIR / "EEZ_countries_WORK.csv").string(), ';');
        std::vector<std::string> eezCountryNames = EEZ_ctry.columnStrings("Country");
        std::vector<double> eezSurfAll = EEZ_ctry.columnDoubles("EEZ (km2)"); // EEZ surface in km2

        // codes_IPBES.merge(EEZ_ctry, how='inner', left_on='Country (IPBES)', right_on='Country')
        std::unordered_map<std::string, std::vector<std::size_t>> nameToEezRows;
        for (std::size_t r = 0; r < eezCountryNames.size(); ++r)
            nameToEezRows[eezCountryNames[r]].push_back(r);

        std::vector<std::string> EEZ_iso; // ISO codes of countries with EEZ
        std::vector<double> EEZ_surf_merged;
        std::vector<std::string> mergedNames;
        for (std::size_t r = 0; r < countryNamesIPBES.size(); ++r) {
            auto it = nameToEezRows.find(countryNamesIPBES[r]);
            if (it == nameToEezRows.end())
                continue;
            for (std::size_t eezRow : it->second) {
                EEZ_iso.push_back(countries[r]);
                EEZ_surf_merged.push_back(eezSurfAll[eezRow]);
                mergedNames.push_back(countryNamesIPBES[r]);
            }
        }

        // check what has been lost/added during merging
        {
            std::set<std::string> before(eezCountryNames.begin(), eezCountryNames.end());
            std::set<std::string> after(mergedNames.begin(), mergedNames.end());
            std::vector<std::string> disappeared, added;
            std::set_difference(before.begin(), before.end(), after.begin(), after.end(), std::back_inserter(disappeared));
            std::set_difference(after.begin(), after.end(), before.begin(), before.end(), std::back_inserter(added));
            std::cout << "Countries lost during IPBES/EEZ merging (" << disappeared.size() << "): {";
            for (std::size_t i = 0; i < disappeared.size(); ++i)
                std::cout << (i ? ", " : "") << disappeared[i];
            std::cout << "}" << std::endl;
            std::cout << "Countries added during IPBES/EEZ merging (" << added.size() << "): {";
            for (std::size_t i = 0; i < added.size(); ++i)
                std::cout << (i ? ", " : "") << added[i];
            std::cout << "}" << std::endl;
        }

        std::unordered_map<std::string, std::size_t> isoToIdx; // mapping iso -> position in countries
        for (std::size_t i = 0; i < countries.size(); ++i)
            isoToIdx[countries[i]] = i;

        std::vector<std::size_t> EEZ_indices; // indices of EEZ countries in the countries table
        for (auto &iso : EEZ_iso) {
            auto it = isoToIdx.find(iso);
            if (it != isoToIdx.end())
                EEZ_indices.push_back(it->second);
        }

        // remove potential duplication
        std::vector<std::string> EEZ_countries;
        {
            std::unordered_map<std::string, bool> seen;
            for (auto &iso : EEZ_iso) {
                if (!seen.count(iso)) {
                    seen[iso] = true;
                    EEZ_countries.push_back(iso);
                }
            }
        }
        {
            std::unordered_map<std::string, int> count;
            for (auto &iso : EEZ_iso)
                count[iso]++;
            std::vector<std::string> removed;
            std::unordered_map<std::string, bool> seenDup;
            for (auto &iso : EEZ_iso) {
                if (count[iso] > 1 && !seenDup.count(iso)) {
                    seenDup[iso] = true;
                    removed.push_back(iso);
                }
            }
            std::cout << "Codes ISO en double (supprimes par unique()):\n[";
            for (std::size_t i = 0; i < removed.size(); ++i)
                std::cout << (i ? ", " : "") << removed[i];
            std::cout << "]" << std::endl;
        }

        NdArray3 driver_X_fresh = loadFull3D(DATA_DIR / "driver_X_fresh.nc");
        NdArray3 driver_W_fresh = loadFull3D(DATA_DIR / "driver_W_fresh.nc");
        NdArray3 driver_X_caned = loadFull3D(DATA_DIR / "driver_X_caned.nc");
        NdArray3 driver_W_caned = loadFull3D(DATA_DIR / "driver_W_caned.nc");

        NdArray2 exported_frac = // restrict to coastal countries (with EEZ) only
            loadSelected2D(DATA_DIR / "exported_frac_national.nc", "iso", EEZ_countries);
        NdArray2 DWFN_invest_capacity_nat = // restrict to coastal countries (with EEZ) only
            loadSelected2D(DATA_DIR / "DWFN_investment_capacity_national.nc", "iso", EEZ_countries);

        NdArray3 driver_U_national = // restrict U to coastal countries (with EEZ) only
            loadSelected3D(DATA_DIR / "driver_U_national.nc", "country", EEZ_countries);
        NdArray3 driver_V_national = // restrict V to coastal countries (with EEZ) only
            loadSelected3D(DATA_DIR / "driver_V_national.nc", "country", EEZ_countries);
        NdArray3 access_nat = // restrict to coastal countries (with EEZ) only
            loadSelected3D(DATA_DIR / "access_right_national.nc", "iso", EEZ_countries);

        CsvTable params = readCsv((DATA_DIR / "parameters.csv").string(), ',');
        std::vector<double> epsilonCol = params.columnDoubles("epsilon");
        double epsilonSum = 0.0;
        std::size_t epsilonCount = 0;
        for (double v : epsilonCol) {
            if (!std::isnan(v)) {
                epsilonSum += v;
                ++epsilonCount;
            }
        }
        double epsilon = epsilonCount > 0 ? epsilonSum / static_cast<double>(epsilonCount)
                                          : std::numeric_limits<double>::quiet_NaN();
        std::vector<double> Qcap_max = params.columnDoubles("Qcap_max");
        double inter = 0.0; // data['intere'].mean() is commented out in the Python source

        NdArray3 catchability_EEZ = // restrict q to coastal countries (with EEZ) only
            loadSelected3D(DATA_DIR / "catchability_EEZ.nc", "iso", EEZ_countries);
        NdArray3 catchability_DWFN = // restrict q to coastal countries (with EEZ) only
            loadSelected3D(DATA_DIR / "catchability_DWFN.nc", "iso", EEZ_countries);

        NdArray3 comply_target_nat = // restrict comply to coastal countries (with EEZ) only
            loadSelected3D(DATA_DIR / "compliance_national.nc", "iso", EEZ_countries);
        for (double &v : comply_target_nat.raw())
            v *= 2.0;
        double risk_invest = 5.0; // characterizes the investment behaviour: 0 = overly prudential (0 investment)

        // This will be modified to read geo_agreg as a driver
        NdArray2 geo_agreg_artisanal_local({static_cast<std::size_t>(nctry), xsize}, 0.0);
        NdArray2 geo_agreg_artisanal_traded({static_cast<std::size_t>(nctry), xsize}, 0.0);
        NdArray2 geo_agreg_industrial({static_cast<std::size_t>(nctry), xsize}, 0.0);
        std::cout << "commodity " << commod << "" << std::endl;
        if (commod == 0) { // small coastal pelagics (e.g. sardines & anchovies)
            geo_agreg_artisanal_local.fill(0.3);
            geo_agreg_artisanal_traded.fill(3.0);
            geo_agreg_industrial.fill(3.0);
        } else if (commod == 1) { // large pelagics (e.g. tunas)
            geo_agreg_artisanal_local.fill(0.3);
            geo_agreg_artisanal_traded.fill(3.0);
            geo_agreg_industrial.fill(3.0);
        } else if (commod == 2) { // demersal & benthic
            geo_agreg_artisanal_local.fill(0.2);
            geo_agreg_artisanal_traded.fill(3.0);
            geo_agreg_industrial.fill(3.0);
        }

        out.countries = std::move(countries);
        out.EEZ_countries = std::move(EEZ_countries);
        out.EEZ_indices = std::move(EEZ_indices);
        out.EEZ_surf = std::move(EEZ_surf_merged);
        out.region_idx = std::move(region_idx);
        out.subregion_idx = std::move(subregion_idx);
        out.driver_X_fresh = std::move(driver_X_fresh);
        out.driver_W_fresh = std::move(driver_W_fresh);
        out.driver_X_caned = std::move(driver_X_caned);
        out.driver_W_caned = std::move(driver_W_caned);
        out.exported_frac = std::move(exported_frac);
        out.DWFN_invest_capacity_nat = std::move(DWFN_invest_capacity_nat);
        out.access_nat = std::move(access_nat);
        out.driver_U_national = std::move(driver_U_national);
        out.driver_V_national = std::move(driver_V_national);
        out.catchability_EEZ = std::move(catchability_EEZ);
        out.catchability_DWFN = std::move(catchability_DWFN);
        out.comply_target_nat = std::move(comply_target_nat);
        out.risk_invest = risk_invest;
        out.epsilon = epsilon;
        out.Qcap_max = std::move(Qcap_max);
        out.inter = inter;
        out.geo_agreg_artisanal_local = std::move(geo_agreg_artisanal_local);
        out.geo_agreg_artisanal_traded = std::move(geo_agreg_artisanal_traded);
        out.geo_agreg_industrial = std::move(geo_agreg_industrial);

        return out;
    }

    // ============================================================== Section 5
    // Define all OSP variables
    // ==============================================================
    OSPVariables PIM_define_variables(int nfleet, int nctry, int nstocks, int nEEZ, std::size_t xsize,
                                      const std::vector<std::string> &country_coord, const NdArray3 &access_nat,
                                      const std::vector<double> &Fmsy, const std::array<double, 3> &MSY, int commod) {
        if (access_nat.extent(2) != xsize) {
            throw std::runtime_error(
                "PIM_define_variables: access_nat's time extent does not match xsize -- this mirrors the "
                "ValueError numpy would raise on `access[:, :nEEZ, :] = access_nat[:, :, :]` when the driver "
                "arrays and the simulation's xsize disagree (see PORTING_NOTES.md).");
        }

        const std::size_t nf = static_cast<std::size_t>(nfleet);
        const std::size_t ns = static_cast<std::size_t>(nstocks);
        const std::size_t ne = static_cast<std::size_t>(nEEZ);
        const std::size_t nc = country_coord.size();

        OSPVariables v;

        v.access = NdArray3({nf, ns, xsize}, 0.0);
        for (std::size_t f = 0; f < nf; ++f)
            for (std::size_t e = 0; e < ne; ++e)
                for (std::size_t t = 0; t < xsize; ++t)
                    v.access(f, e, t) = access_nat(f, e, t);

        // Efforts
        v.effort_EEZ = NdArray3({nf, ne, xsize}, 0.0);
        v.effort_DWFN = NdArray3({nf, ne, xsize}, 0.0);
        v.allocate_effort_DWFN = NdArray4({nf, ne, ns, xsize}, 0.0);
        // effort for stock assessment for four fleets: (artisanal, industrial) x (EEZ, DWFN), in every stock, every year
        v.effort_for_stock_assess = NdArray3({2 * nf, ns, xsize}, 0.0);

        // Catches
        v.catch_step_EEZ = NdArray2({nf, ne}, 0.0);
        v.catch_EEZ = NdArray3({nf, ne, xsize}, 0.0);
        v.catch_step_DWFN = NdArray3({nf, ne, ns}, 0.0);
        v.catch_step_DWFN_tot = NdArray2({nf, ne}, 0.0);
        v.catch_DWFN = NdArray3({nf, ne, xsize}, 0.0);
        v.catch_step_artis_local_allctry = NdArray2({nc, xsize}, 0.0);
        v.catch_step_artis_traded_allctry = NdArray2({nc, xsize}, 0.0);
        v.catch_step_indus_allctry = NdArray2({nc, xsize}, 0.0);
        // catches for stock assessment for four fleets: (artisanal, industrial) x (EEZ, DWFN), in every stock, every year
        v.catch_for_stock_assess = NdArray3({2 * nf, ns, xsize}, 0.0);

        // Mortalities
        v.F_init = NdArray2({nf, ns}, 0.0);
        v.F_fleet_EEZ = NdArray3({nf, ne, xsize}, 0.0);         // F from artisanal and industrial local fleet in every EEZ
        v.F_fleet_DWFN = NdArray3({nf, ne, xsize}, 0.0);        // F from artisanal and industrial DWFN fleet from every coastal country (EEZ)
        v.F_DWFN_allocate = NdArray4({nf, ne, ns, xsize}, 0.0); // F from artisanal and industrial DWFN fleet from every coastal country, in every stock (EEZs + ABNJ)
        v.F_tot = NdArray2({ns, xsize}, 0.0);
        v.prop_EEZ_DWFN.assign(ns, 1.0);

        // Stock assessment & target allocation
        v.nfleets_per_stock.resize(ns);
        for (std::size_t s = 0; s < ns; ++s)
            v.nfleets_per_stock[s] = (s < ne) ? 2 * nfleet : nfleet; // EEZ stocks are exploited by 4 fleets and ABNJ by 2 fleet only
        v.prop2target.assign(ns, 1.0);
        v.Fmsy_est = Fmsy;
        v.MSY_est.assign(ns, 0.0); // np.empty in the Python source; zero-initialised here instead of left garbage
        v.global_MSY.assign(xsize, MSY[static_cast<std::size_t>(commod)]);
        v.global_FMSY.assign(xsize, sumVec(Fmsy));
        v.target_EEZ.assign(ne, 0.0);
        v.target_DWFN_fleet.assign(ne, 0.0);
        v.target_DWFN_stocks.assign(ns, 0.0);
        v.target_DWFN_init.assign(ne, 1.0);

        // Prices
        v.price_EEZ = NdArray3({nf, ne, xsize}, 0.0);
        v.price_DWFN = NdArray3({nf, ne, xsize}, 0.0);
        const double nan = std::numeric_limits<double>::quiet_NaN();
        v.price_artisanal_traded_allctry = NdArray2({nc, xsize}, nan);
        v.price_artisanal_local_allctry = NdArray2({nc, xsize}, nan);
        v.price_industrial_allctry = NdArray2({nc, xsize}, nan);
        v.ratio_exported_over_local_artisanal_prices.assign(ne, 0.0); // initialize the ratio of exported over local artisanal prices

        // Consumptions
        v.consumption_artisanal_traded_allctry = NdArray2({nc, xsize}, nan);
        v.consumption_artisanal_local_allctry = NdArray2({nc, xsize}, nan);
        v.consumption_industrial_allctry = NdArray2({nc, xsize}, nan);

        // Aquaculture
        v.prod_aqua = NdArray2({static_cast<std::size_t>(nctry), xsize}, 0.0);

        return v;
    }

    // ============================================================== Section 6
    // Calculate price and consumption (plugin-model 1)
    // ==============================================================
    PriceCalcResult plugin_model_1_calc_price(const std::vector<double> &geo_agreg, int nctry, int nregion, int nsubregion,
                                              const std::vector<int> &region_idx, const std::vector<int> &subregion_idx,
                                              const std::vector<double> &prod, const std::vector<double> &X,
                                              const std::vector<double> &W, double epsilon,
                                              const std::vector<double> &aqua) {
        auto allEqual = [&](double val) {
            for (double g : geo_agreg)
                if (g != val)
                    return false;
            return true;
        };
        auto allLE = [&](double val) {
            for (double g : geo_agreg)
                if (!(g <= val))
                    return false;
            return true;
        };

        // production and consumption in tons and prices in $2005/kg
        std::vector<double> price_ctry(static_cast<std::size_t>(nctry));
        double sum_Y_glob = sumVec(prod);
        std::vector<double> sum_Y_reg(static_cast<std::size_t>(nregion), 0.0);
        std::vector<double> sum_Y_subreg(static_cast<std::size_t>(nsubregion), 0.0);

        if (allEqual(3.0)) { // geo_agreg == 3: global market
            double sum_X = sumVec(X), sum_W = sumVec(W), sum_aqua = sumVec(aqua);
            double tmp = sum_X - sum_Y_glob + sum_aqua;
            double ratio = (tmp < 0.0) ? 0.0 : tmp / sum_W;
            double p = std::pow(ratio, 1.0 / epsilon);
            std::fill(price_ctry.begin(), price_ctry.end(), p);
        } else {
            // geo_agreg == 3: global market
            double sum_X = sumVec(X), sum_W = sumVec(W), sum_aqua = sumVec(aqua);
            double tmp = sum_X - sum_Y_glob + sum_aqua;
            double ratio = (tmp < 0.0) ? 0.0 : tmp / sum_W;
            double price_glob = std::pow(ratio, 1.0 / epsilon);
            std::vector<double> price_ctry_glob(static_cast<std::size_t>(nctry), price_glob);

            // geo_agreg == 2: regional market
            std::vector<double> sX_r(static_cast<std::size_t>(nregion), 0.0), sW_r(static_cast<std::size_t>(nregion), 0.0),
                sAq_r(static_cast<std::size_t>(nregion), 0.0);
            for (int i = 0; i < nctry; ++i) {
                sX_r[region_idx[i]] += X[i];
                sW_r[region_idx[i]] += W[i];
                sum_Y_reg[region_idx[i]] += prod[i];
                sAq_r[region_idx[i]] += aqua[i];
            }
            std::vector<double> price_reg(static_cast<std::size_t>(nregion));
            for (int r = 0; r < nregion; ++r) {
                double t = sX_r[r] - sum_Y_reg[r] + sAq_r[r];
                double rr = (t < 0.0) ? 0.0 : t / sW_r[r];
                price_reg[r] = std::pow(rr, 1.0 / epsilon);
            }
            std::vector<double> price_ctry_reg(static_cast<std::size_t>(nctry));
            for (int i = 0; i < nctry; ++i)
                price_ctry_reg[i] = price_reg[region_idx[i]];

            // geo_agreg == 1: subregional market
            std::vector<double> sX_s(static_cast<std::size_t>(nsubregion), 0.0), sW_s(static_cast<std::size_t>(nsubregion), 0.0),
                sAq_s(static_cast<std::size_t>(nsubregion), 0.0);
            for (int i = 0; i < nctry; ++i) {
                sX_s[subregion_idx[i]] += X[i];
                sW_s[subregion_idx[i]] += W[i];
                sum_Y_subreg[subregion_idx[i]] += prod[i];
                sAq_s[subregion_idx[i]] += aqua[i];
            }
            std::vector<double> price_subreg(static_cast<std::size_t>(nsubregion));
            for (int s = 0; s < nsubregion; ++s) {
                double t = sX_s[s] - sum_Y_subreg[s] + sAq_s[s];
                double rr = (t < 0.0) ? 0.0 : t / sW_s[s];
                price_subreg[s] = std::pow(rr, 1.0 / epsilon);
            }
            std::vector<double> price_ctry_subreg(static_cast<std::size_t>(nctry));
            for (int i = 0; i < nctry; ++i)
                price_ctry_subreg[i] = price_subreg[subregion_idx[i]];

            // geo_agreg == 0: national market
            std::vector<double> price_ctry_nat(static_cast<std::size_t>(nctry));
            for (int i = 0; i < nctry; ++i) {
                double t = X[i] - prod[i] + aqua[i];
                double rr = (t < 0.0) ? 0.0 : t / W[i];
                price_ctry_nat[i] = std::pow(rr, 1.0 / epsilon);
            }

            // interpolate prices over different geographical extents of markets;
            // PCHIP interpolation has no negative overshoot problem compared to polynomial interpolation
            for (int i = 0; i < nctry; ++i) {
                std::array<double, 4> y{price_ctry_nat[i], price_ctry_subreg[i], price_ctry_reg[i], price_ctry_glob[i]};
                double val = pchip4_eval(y, geo_agreg[i]);
                price_ctry[i] = std::max(0.0, val);
            }
        }

        std::vector<double> consumption_ctry(static_cast<std::size_t>(nctry));
        for (int i = 0; i < nctry; ++i) {
            double tmp = X[i] - W[i] * std::pow(price_ctry[i], epsilon);
            consumption_ctry[i] = (tmp < 0.0) ? 0.0 : tmp;
        }

        if (allEqual(0.0)) { // geo_agreg == 0: national market
            for (int i = 0; i < nctry; ++i)
                consumption_ctry[i] = std::min(prod[i], consumption_ctry[i]);
        } else if (allLE(1.0)) { // 0 < geo_agreg <= 1: subregionalized market
            std::vector<double> consTmpSub(static_cast<std::size_t>(nsubregion), 0.0);
            for (int i = 0; i < nctry; ++i)
                consTmpSub[subregion_idx[i]] += consumption_ctry[i];
            std::vector<double> factor(static_cast<std::size_t>(nsubregion));
            for (int s = 0; s < nsubregion; ++s)
                factor[s] = (consTmpSub[s] > 0.0) ? sum_Y_subreg[s] / consTmpSub[s] : 0.0;
            for (int i = 0; i < nctry; ++i)
                consumption_ctry[i] *= factor[subregion_idx[i]];
        } else if (allLE(2.0)) { // 1 < geo_agreg <= 2: regionalized market
            std::vector<double> consTmpReg(static_cast<std::size_t>(nregion), 0.0);
            for (int i = 0; i < nctry; ++i)
                consTmpReg[region_idx[i]] += consumption_ctry[i];
            std::vector<double> factor(static_cast<std::size_t>(nregion));
            for (int r = 0; r < nregion; ++r)
                factor[r] = (consTmpReg[r] > 0.0) ? sum_Y_reg[r] / consTmpReg[r] : 0.0;
            for (int i = 0; i < nctry; ++i)
                consumption_ctry[i] *= factor[region_idx[i]];
        } else if (allLE(3.0)) { // 2 < geo_agreg <= 3: globalized market
            double consGlob = sumVec(consumption_ctry);
            double factor = (consGlob == 0.0) ? 0.0 : sum_Y_glob / consGlob;
            for (double &c : consumption_ctry)
                c *= factor;
        }

        return {price_ctry, consumption_ctry};
    }

    // ============================================================== Section 7
    // Calculate prices and consumptions for all fleet & countries
    // ==============================================================
    PriceConsumptionStepResult PIM_calculate_prices_consumptions(
        NdArray3 &price_EEZ, NdArray3 &price_DWFN, NdArray2 &catch_step_artis_traded_allctry,
        NdArray2 &catch_step_indus_allctry, NdArray2 &catch_step_artis_local_allctry,
        NdArray2 &price_artisanal_traded_allctry, NdArray2 &consumption_artisanal_traded_allctry,
        NdArray2 &price_industrial_allctry, NdArray2 &consumption_industrial_allctry,
        NdArray2 &price_artisanal_local_allctry, NdArray2 &consumption_artisanal_local_allctry,
        const NdArray2 &exported_frac, const NdArray2 &catch_step_EEZ, const NdArray3 &catch_step_DWFN,
        const std::vector<std::size_t> &eezPosInCountryCoord, const NdArray2 &geo_agreg_artisanal_traded,
        const NdArray2 &geo_agreg_industrial, const NdArray2 &geo_agreg_artisanal_local, int nctry, int nregion,
        int nsubregion, const std::vector<int> &region_idx, const std::vector<int> &subregion_idx,
        const std::vector<double> &driv_X_f, const std::vector<double> &driv_W_f, const std::vector<double> &driv_X_c,
        const std::vector<double> &driv_W_c, double epsilon, const std::vector<double> &inter_prod_aq, double EPS,
        std::size_t t, double dt, int commod) {
        (void)commod; // unused, mirrors the Python function's unused `commod` parameter

        const std::size_t nEEZ = eezPosInCountryCoord.size();
        const std::size_t ncountry = catch_step_artis_traded_allctry.extent(0);

        // exported_frac_realized could in principle be multiplied by a sigmoidal function of the
        // traded-versus-local price ratio (technological potential for exportation) -- left as a
        // straight pass-through in the Python source, same here.
        std::vector<double> exported_frac_realized(nEEZ);
        for (std::size_t e = 0; e < nEEZ; ++e) {
            exported_frac_realized[e] = exported_frac(e, t);
        }

        // compile EEZ and DWFN catches for both artisanal & industrial fleet
        NdArray2 catch_step_EEZ_tot({2, nEEZ}, 0.0);
        const std::size_t nstocksDim = catch_step_DWFN.extent(2);
        for (std::size_t fleet = 0; fleet < 2; ++fleet) {
            for (std::size_t e = 0; e < nEEZ; ++e) {
                double s = catch_step_EEZ(fleet, e);
                for (std::size_t st = 0; st < nstocksDim; ++st)
                    s += catch_step_DWFN(fleet, e, st);
                catch_step_EEZ_tot(fleet, e) = s;
                // std::cout << "balise tptp " << catch_step_EEZ_tot(fleet, e) << std::endl;
            }
        }

        // Traded artisanal products.
        // exported fraction of artisanal EEZ-based + artisanal DWFN fleet
        for (std::size_t e = 0; e < nEEZ; ++e)
            catch_step_artis_traded_allctry(eezPosInCountryCoord[e], t) = exported_frac_realized[e] * catch_step_EEZ_tot(0, e);
        std::vector<double> tmpTraded(ncountry), geoTraded(ncountry), geoIndus(ncountry), geoLocal(ncountry);
        for (std::size_t c = 0; c < ncountry; ++c) {
            // divide by dt to calculate corresponding yearly catches, because prices are calculated as a function of yearly catches
            tmpTraded[c] = catch_step_artis_traded_allctry(c, t) / dt;
            geoTraded[c] = geo_agreg_artisanal_traded(c, t);
            geoIndus[c] = geo_agreg_industrial(c, t);
            geoLocal[c] = geo_agreg_artisanal_local(c, t);
        }
        // calculate prices for exported artisanal fish
        PriceCalcResult tradedRes = plugin_model_1_calc_price(geoTraded, nctry, nregion, nsubregion, region_idx,
                                                              subregion_idx, tmpTraded, driv_X_f, driv_W_f, epsilon,
                                                              inter_prod_aq);
        for (std::size_t c = 0; c < ncountry; ++c) {
            price_artisanal_traded_allctry(c, t) = tradedRes.price_ctry[c];
            consumption_artisanal_traded_allctry(c, t) = tradedRes.consumption_ctry[c];
        }

        // Industrial products (traded).
        for (std::size_t e = 0; e < nEEZ; ++e)
            catch_step_indus_allctry(eezPosInCountryCoord[e], t) = catch_step_EEZ_tot(1, e);
        std::vector<double> tmpIndus(ncountry);
        for (std::size_t c = 0; c < ncountry; ++c)
            tmpIndus[c] = catch_step_indus_allctry(c, t) / dt;
        // calculate prices for industrial EEZ & DWFN fish
        PriceCalcResult indusRes = plugin_model_1_calc_price(geoIndus, nctry, nregion, nsubregion, region_idx,
                                                             subregion_idx, tmpIndus, driv_X_c, driv_W_c, epsilon,
                                                             inter_prod_aq);
        for (std::size_t c = 0; c < ncountry; ++c) {
            price_industrial_allctry(c, t) = indusRes.price_ctry[c];
            consumption_industrial_allctry(c, t) = indusRes.consumption_ctry[c];
        }

        // Locally sold artisanal products.
        for (std::size_t e = 0; e < nEEZ; ++e) // update allctry with EEZ countries
            catch_step_artis_local_allctry(eezPosInCountryCoord[e], t) =
                (1.0 - exported_frac_realized[e]) * catch_step_EEZ_tot(0, e);
        std::vector<double> tmpLocal(ncountry);
        for (std::size_t c = 0; c < ncountry; ++c) {
            tmpLocal[c] = catch_step_artis_local_allctry(c, t) / dt;
        }
        PriceCalcResult localRes = plugin_model_1_calc_price(geoLocal, nctry, nregion, nsubregion, region_idx,
                                                             subregion_idx, tmpLocal, driv_X_f, driv_W_f, epsilon,
                                                             inter_prod_aq);
        for (std::size_t c = 0; c < ncountry; ++c) {
            price_artisanal_local_allctry(c, t) = localRes.price_ctry[c];
            consumption_artisanal_local_allctry(c, t) = localRes.consumption_ctry[c];
        }

        for (std::size_t e = 0; e < nEEZ; ++e) {
            std::size_t pos = eezPosInCountryCoord[e];
            double traded = catch_step_artis_traded_allctry(pos, t);
            double local = catch_step_artis_local_allctry(pos, t);
            // artisanal price in a given EEZ country is the mean of exported price & local price, weighted
            // by the relative fraction of exported versus locally traded catches
            double num = price_artisanal_traded_allctry(pos, t) * traded + price_artisanal_local_allctry(pos, t) * local;
            price_EEZ(0, e, t) = num / catch_step_EEZ_tot(0, e);
            price_EEZ(1, e, t) = price_industrial_allctry(pos, t); // price is the same for industrial EEZ fleets & industrial DWFN
            price_DWFN(0, e, t) = price_EEZ(0, e, t);              // artisanal DWFN price is the same as the artisanal EEZ-based fleet
            // industrial DWFN price is the same as the industrial EEZ-based fleet, as they operate on the
            // same global canned tuna market
            price_DWFN(1, e, t) = price_industrial_allctry(pos, t);
        }

        std::vector<double> ratio(nEEZ);
        for (std::size_t e = 0; e < nEEZ; ++e) {
            std::size_t pos = eezPosInCountryCoord[e];
            ratio[e] = (price_artisanal_traded_allctry(pos, t) + EPS) / (price_artisanal_local_allctry(pos, t) + EPS);
        }

        PriceConsumptionStepResult out;
        out.price_artisanal_traded.resize(ncountry);
        out.consumption_artisanal_traded.resize(ncountry);
        out.price_industrial.resize(ncountry);
        out.consumption_industrial.resize(ncountry);
        out.price_artisanal_local.resize(ncountry);
        out.consumption_artisanal_local.resize(ncountry);
        for (std::size_t c = 0; c < ncountry; ++c) {
            out.price_artisanal_traded[c] = price_artisanal_traded_allctry(c, t);
            out.consumption_artisanal_traded[c] = consumption_artisanal_traded_allctry(c, t);
            out.price_industrial[c] = price_industrial_allctry(c, t);
            out.consumption_industrial[c] = consumption_industrial_allctry(c, t);
            out.price_artisanal_local[c] = price_artisanal_local_allctry(c, t);
            out.consumption_artisanal_local[c] = consumption_artisanal_local_allctry(c, t);
        }
        out.price_EEZ_t = NdArray2({2, nEEZ});
        out.price_DWFN_t = NdArray2({2, nEEZ});
        for (std::size_t f = 0; f < 2; ++f)
            for (std::size_t e = 0; e < nEEZ; ++e) {
                out.price_EEZ_t(f, e) = price_EEZ(f, e, t);
                out.price_DWFN_t(f, e) = price_DWFN(f, e, t);
            }
        out.ratio_exported_over_local_artisanal_prices = ratio;
        return out;
    }

    // ============================================================== Section 8
    // Calculate Aquaculture dynamics (plugin-model 2-aqua)
    // ==============================================================
    std::vector<double> mini_model_2_calc_aqua(const std::vector<double> &price, const std::vector<double> &Aprod,
                                               const std::vector<double> &d_U, const std::vector<double> &d_V,
                                               double d_t) {
        std::vector<double> prod_aqua(price.size());
        for (std::size_t i = 0; i < price.size(); ++i) {
            double tmpA = price[i] * d_U[i] - d_V[i];
            prod_aqua[i] = Aprod[i] * std::exp(1e-5 * tmpA * d_t);
        }
        return prod_aqua;
    }

    // ============================================================== Section 9
    // Calculate Effort dynamics (plugin-model 2)
    // ==============================================================
    EffortCalcResult plugin_model_2_calc_effort(int nfleet_type, int nfisheries, const NdArray2 &price, const NdArray2 &Y,
                                                const NdArray2 &effo, const NdArray2 &q, const std::vector<double> &Ftarget,
                                                const NdArray2 &comply, double risk, const NdArray2 &d_U, const NdArray2 &d_V,
                                                const std::vector<double> &M, double d_t, int effort_growth_lim,
                                                double f_growth_rate_max, double f_growth_rate_half_sat) {
        const std::size_t nft = static_cast<std::size_t>(nfleet_type), nfy = static_cast<std::size_t>(nfisheries);

        NdArray2 F({nft, nfy}, 0.0);
        for (std::size_t ft = 0; ft < nft; ++ft)
            for (std::size_t j = 0; j < nfy; ++j)
                F(ft, j) = q(ft, j) * effo(ft, j);

        std::vector<double> Ftot(nfy, 0.0);
        for (std::size_t j = 0; j < nfy; ++j)
            for (std::size_t ft = 0; ft < nft; ++ft)
                Ftot[j] += F(ft, j);

        // ventilation of Ftarget per fleet. NOTE: no zero-guard on the Ftot[j] division here --
        // the Python source has a masked "safe division" variant of this commented out; matched here
        // by leaving the division unguarded, same as the Python source's live code path.
        NdArray2 F_target({nft, nfy}, 0.0);
        for (std::size_t ft = 0; ft < nft; ++ft)
            for (std::size_t j = 0; j < nfy; ++j)
                F_target(ft, j) = Ftarget[j] * F(ft, j) / Ftot[j];

        // investment deceleration factor to avoid overshooting the target: function of total F
        std::vector<double> invest_slow(nfy);
        for (std::size_t j = 0; j < nfy; ++j)
            invest_slow[j] = (Ftot[j] < Ftarget[j]) ? 1.0 - std::pow(Ftot[j] / Ftarget[j], risk) : 0.0;

        NdArray2 f({nft, nfy}, 0.0);
        for (std::size_t fleet = 0; fleet < nft; ++fleet) {
            std::vector<double> invest(nfy), source_f(nfy), sink_f(nfy);
            for (std::size_t j = 0; j < nfy; ++j) {
                // progressive decrease of investments when approaching the target, for compliant boats only
                invest[j] = comply(fleet, j) * invest_slow[j] + (1.0 - comply(fleet, j));
                source_f[j] = invest[j] * Y(fleet, j) * price(fleet, j) * d_U(fleet, j);
                sink_f[j] = invest[j] * d_V(fleet, j) + M[fleet];
            }
            if (effort_growth_lim == 1) {
                for (std::size_t j = 0; j < nfy; ++j) {
                    double tmp_sink = sink_f[j] * effo(fleet, j);
                    double f_growth_positivity = (source_f[j] - tmp_sink) / effo(fleet, j);
                    double K = f_growth_rate_max / (2.0 * f_growth_rate_half_sat);
                    // effort growth rate limitation
                    double f_growth_rate_lim = effo(fleet, j) * f_growth_rate_max * f_growth_positivity / (K + f_growth_positivity);
                    double denom_inv = source_f[j] - invest[j] * d_V(fleet, j) * effo(fleet, j);
                    // avoids potential division by 0
                    double f_inv_reduc = (std::abs(denom_inv) > 1e-30)
                                             ? (f_growth_rate_lim + M[fleet] * effo(fleet, j)) / denom_inv
                                             : 1.0;
                    // limits investment fraction to limit fishing effort growth to a realistic range
                    double f_inv_lim = (f_growth_positivity > 0.0) ? f_inv_reduc : 1.0;
                    source_f[j] *= f_inv_lim;
                    sink_f[j] *= f_inv_lim;
                }
            }
            // semi-implicit approximation of effort dynamics
            for (std::size_t j = 0; j < nfy; ++j)
                f(fleet, j) = (effo(fleet, j) + source_f[j] * d_t) / (1.0 + sink_f[j] * d_t);
        }

        NdArray2 Ftmp({nft, nfy}, 0.0);
        for (std::size_t ft = 0; ft < nft; ++ft)
            for (std::size_t j = 0; j < nfy; ++j)
                Ftmp(ft, j) = q(ft, j) * f(ft, j);
        std::vector<double> Ftmp_tot(nfy, 0.0);
        for (std::size_t j = 0; j < nfy; ++j)
            for (std::size_t ft = 0; ft < nft; ++ft)
                Ftmp_tot[j] += Ftmp(ft, j);

        NdArray2 Fout({nft, nfy}, 0.0);
        for (std::size_t ft = 0; ft < nft; ++ft) {
            for (std::size_t j = 0; j < nfy; ++j) {
                double overshoot = (Ftmp_tot[j] >= Ftarget[j]) ? 1.0 : 0.0; // 0 if F < target and 1 if F >= target
                double management = overshoot * comply(ft, j);
                // F = F_target for the compliant fraction of the fleet, if F is above target
                Fout(ft, j) = management * F_target(ft, j) + (1.0 - management) * Ftmp(ft, j);
            }
        }
        std::vector<double> FtotFinal(nfy, 0.0);
        for (std::size_t j = 0; j < nfy; ++j) {
            for (std::size_t ft = 0; ft < nft; ++ft) {
                FtotFinal[j] += Fout(ft, j);
            }
        }

        return {f, Fout, FtotFinal};
    }

    // ============================================================== Section 10
    // Lognormal density
    // ==============================================================
    double dnorm_log(double obs, double pred, double sd) {
        return -0.5 * std::log(2 * kPi) - std::log(sd) - 0.5 * std::pow((obs - pred) / sd, 2.0);
    }

    // ============================================================== Section 11
    // The yearly biomass-dynamics time loop shared by objective() and
    // fit_one_stock(); computes and returns B, Bmoy, Cpred, F_tot, and the
    // negative-biomass penalty. (In the Python source this was @njit-compiled by
    // Numba to accelerate it; here it's just a plain function -- the C++
    // compiler already produces native code, no annotation needed.)
    DynamicsLoopResult dynamics_loop(std::vector<double> B, double r, double k, double z, const std::vector<double> &q,
                                     const NdArray2 &E, int n, int nf) {
        DynamicsLoopResult out;
        out.B = std::move(B);
        out.Bmoy.assign(static_cast<std::size_t>(n), 0.0);                                      // average yearly biomass, for calculating CPUE
        out.Cpred = NdArray2({static_cast<std::size_t>(n), static_cast<std::size_t>(nf)}, 0.0); // predicted catches per fleet
        out.F_tot.assign(static_cast<std::size_t>(n), 0.0);                                     // fishing mortality for each fleet
        out.pen = 0.0;                                                                          // penalty to avoid negative biomass

        std::vector<double> F_fleet(static_cast<std::size_t>(nf));
        for (int t = 0; t < n; ++t) {
            // Fishing mortality for each fleet
            double F_total = 0.0;
            for (int i = 0; i < nf; ++i) {
                F_fleet[i] = q[i] * E(t, i);
                F_total += F_fleet[i];
            }

            // Biomass dynamics
            double Expl = 1.0 - std::exp(-F_total);
            double B_next = out.B[t] + r * out.B[t] * (1.0 - std::pow(out.B[t] / k, z)) - Expl * out.B[t];
            if (B_next <= 0.0) {
                out.pen += 1000.0 * std::abs(B_next);
                B_next = 1e-6;
            }
            out.B[t + 1] = B_next;
            out.Bmoy[t] = (out.B[t] + out.B[t + 1]) / 2.0;
            out.F_tot[t] = F_total;

            // Allocate catches per fleet proportionally to F_i / F_total
            if (F_total > 0.0) {
                for (int i = 0; i < nf; ++i)
                    out.Cpred(t, i) = (F_fleet[i] / F_total) * Expl * out.B[t];
            }
        }
        return out;
    }

    // ============================================================== Section 12
    // Calculate negative log-likelihood
    // ==============================================================
    double objective(const std::vector<double> &log_params, const StockData &data) {
        // Extract data
        const int nf = data.nfleets;
        const int n = static_cast<int>(data.C.extent(0));

        // Extract parameters
        double r = std::exp(log_params[0]);
        double k = std::exp(log_params[1]);
        double z = std::exp(log_params[2]);
        std::vector<double> q(nf), sigma_CPUE(nf), sigma_C(nf);
        for (int i = 0; i < nf; ++i) {
            q[i] = std::exp(log_params[3 + i]);                // q per fleet
            sigma_CPUE[i] = std::exp(log_params[3 + nf + i]);  // sigma per fleet
            sigma_C[i] = std::exp(log_params[3 + 2 * nf + i]); // sigma per fleet
        }

        // Initialization of B & K
        std::vector<double> B(static_cast<std::size_t>(n) + 1, 0.0);
        B[0] = k;
        DynamicsLoopResult dl = dynamics_loop(std::move(B), r, k, z, q, data.E, n, nf);

        // Likelihood of CPUE (obs C/E vs predicted q_i * B_moy)
        double nll_cpue = 0.0;
        for (int t = 0; t < n; ++t) {
            for (int i = 0; i < nf; ++i) {
                double Eti = data.E(t, i);
                // cpue_obs: nan when E == 0
                double cpue_obs = (Eti > 0.0) ? data.C(t, i) / Eti : std::numeric_limits<double>::quiet_NaN();
                double cpue_pred = dl.Bmoy[t] * q[i];
                if (cpue_obs > 0.0 && cpue_pred > 0.0) {
                    nll_cpue -= dnorm_log(std::log(cpue_obs), std::log(cpue_pred), sigma_CPUE[i]);
                }
            }
        }

        // Likelihood of catches
        double nll_C = 0.0;
        for (int t = 0; t < n; ++t) {
            for (int i = 0; i < nf; ++i) {
                double Cobs = data.C(t, i), Cpred = dl.Cpred(t, i);
                if (Cobs > 0.0 && Cpred > 0.0)
                    nll_C -= dnorm_log(std::log(Cobs), std::log(Cpred), sigma_C[i]);
            }
        }

        // Priors
        double prior_r = 0.5 * std::pow((std::log(r) - std::log(data.prior_mean_r)) / data.prior_sd_logr, 2.0); // lognormal prior on r
        double prior_z = 0.5 * std::pow((std::log(z) - std::log(data.prior_mean_z)) / data.prior_sd_logz, 2.0); // lognormal prior on z

        return dl.pen + nll_cpue + nll_C + prior_r + prior_z;
    }

    // ============================================================== Section 13
    // Fit stock assessment model
    // ==============================================================
    FitResult fit_one_stock(const StockData &data, int mini, bool verbose, const std::optional<std::vector<double>> &x0_prev,
                            std::optional<double> K0opt) {
        const NdArray2 &C = data.C;
        const NdArray2 &E = data.E;
        const int nf = data.nfleets;
        const int n = static_cast<int>(C.extent(0));

        // Initialization
        std::vector<double> x0;
        if (x0_prev.has_value()) {
            // use x0 if available (for every year except the first): initialize with stock assessment from previous year
            x0 = *x0_prev;
        } else {
            // initialize parameters if no stock assessment was conducted before
            double r0 = data.prior_mean_r;
            double z0 = data.prior_mean_z;
            double K0;
            if (K0opt.has_value()) {
                K0 = *K0opt;
            } else {
                double maxC = -std::numeric_limits<double>::infinity();
                for (int t = 0; t < n; ++t)
                    for (int i = 0; i < nf; ++i) {
                        double v = C(t, i);
                        if (!std::isnan(v) && v > maxC)
                            maxC = v;
                    }
                K0 = 3.0 * maxC;
            }
            std::vector<double> q0(nf);
            for (int i = 0; i < nf; ++i) {
                std::vector<double> cpue;
                for (int t = 0; t < n; ++t)
                    if (E(t, i) > 0.0)
                        cpue.push_back(C(t, i) / E(t, i));
                if (!cpue.empty()) {
                    std::sort(cpue.begin(), cpue.end());
                    std::size_t m = cpue.size();
                    double med = (m % 2 == 1) ? cpue[m / 2] : 0.5 * (cpue[m / 2 - 1] + cpue[m / 2]);
                    q0[i] = med / K0; // q_i initial: median(CPUE_i) / K0
                } else {
                    q0[i] = 1e-4;
                }
            }
            x0.reserve(static_cast<std::size_t>(3 + 3 * nf));
            x0.push_back(std::log(r0));
            x0.push_back(std::log(K0));
            x0.push_back(std::log(z0));
            for (int i = 0; i < nf; ++i)
                x0.push_back(std::log(q0[i]));
            for (int i = 0; i < nf; ++i)
                x0.push_back(std::log(0.2)); // initial sigma_CPUE
            for (int i = 0; i < nf; ++i)
                x0.push_back(std::log(0.2)); // initial sigma_C
        }

        optim::ObjectiveFn obj = [&](const std::vector<double> &p) { return objective(p, data); };

        optim::OptResult result;
        if (mini == 0) {
            optim::NelderMeadOptions o;
            o.maxiter = 50000;
            o.xatol = 1e-7;
            o.fatol = 1e-7;
            result = optim::nelderMead(obj, x0, o);
        }
        if (mini >= 1) {
            // Bounds in log-space
            double C_max = -std::numeric_limits<double>::infinity(); // maximum observed catches
            for (int t = 0; t < n; ++t) {
                double s = 0.0;
                for (int i = 0; i < nf; ++i)
                    s += C(t, i);
                if (s > C_max)
                    C_max = s;
            }
            double K_min = C_max;         // K >= C_max
            double K_max = C_max * 100.0; // 100 is arbitrary

            std::vector<double> q_min(nf), q_max(nf);
            for (int i = 0; i < nf; ++i) {
                double cmin = std::numeric_limits<double>::infinity(), cmax = -std::numeric_limits<double>::infinity();
                bool any = false;
                for (int t = 0; t < n; ++t) {
                    if (E(t, i) > 0.0) {
                        any = true;
                        double v = C(t, i) / E(t, i);
                        cmin = std::min(cmin, v);
                        cmax = std::max(cmax, v);
                    }
                }
                if (!any) {
                    // in case a given fleet is always 0, set arbitrary bounds
                    q_min[i] = 1e-10;
                    q_max[i] = 1e-1;
                } else {
                    q_min[i] = cmin / K_max;
                    q_max[i] = cmax / K_min;
                }
            }

            std::vector<optim::Bound> bounds;
            bounds.reserve(static_cast<std::size_t>(3 + 3 * nf));
            // logR (r < 0.05: long-lived species -- sharks, tunas; r > 1.0: small pelagics -- anchovies, sardines)
            bounds.push_back({std::log(0.01), std::log(2.0)});
            bounds.push_back({std::log(K_min), std::log(K_max)}); // K >= C_max and K_max is arbitrary
            // z = 0.01 is Fox (high productivity at low B), z = 1 is Schaefer, z > 1 (high productivity at high B)
            bounds.push_back({std::log(0.01), std::log(10.0)});
            for (int i = 0; i < nf; ++i)
                bounds.push_back({std::log(q_min[i]), std::log(q_max[i])});
            // sigma < 0.1 (CV < 10%: very precise CPUE); sigma > 1 (poorly informative CPUE)
            for (int i = 0; i < nf; ++i)
                bounds.push_back({std::log(0.01), std::log(2.0)});
            // sigma < 0.1 (CV < 10%: very precise catches); sigma > 1 (poorly informative catches)
            for (int i = 0; i < nf; ++i)
                bounds.push_back({std::log(0.01), std::log(2.0)});
            if (static_cast<int>(bounds.size()) != 3 + 3 * nf) {
                throw std::runtime_error("fit_one_stock: bounds size mismatch");
            }

            if (mini == 1) { // parallelized differential evolution
                optim::DifferentialEvolutionOptions o;
                o.latinHypercubeInit = true;
                o.deferredUpdating = true;
                o.polish = true; // SciPy's default (not overridden at this call site)
                o.seed = 42;
                o.maxiter = 1000;
                o.tol = 1e-6;
                result = optim::differentialEvolution(obj, bounds, o);
            } else if (mini == 2) { // L-BFGS-B
                optim::LBFGSBOptions o;
                o.maxiter = 1000;
                o.ftol = 1e-6;
                result = optim::boundedLBFGS(obj, x0, bounds, o);
            } else if (mini == 3) { // hybrid: 1st parallelized differential evolution then automatic refinement with L-BFGS-B
                optim::DifferentialEvolutionOptions o;
                o.latinHypercubeInit = true;
                o.deferredUpdating = true;
                o.polish = true;
                o.seed = 42;
                o.maxiter = 1000;
                o.tol = 1e-6;
                result = optim::differentialEvolution(obj, bounds, o);
            } else if (mini == 4) { // hybrid: 1st parallelized differential evolution then parameterizable refinement with L-BFGS-B
                optim::DifferentialEvolutionOptions o;
                o.latinHypercubeInit = true;
                o.deferredUpdating = true;
                o.polish = false;
                o.popsize = 8;
                o.seed = 42;
                o.maxiter = 300;
                o.tol = 1e-4;
                optim::OptResult deRes = optim::differentialEvolution(obj, bounds, o);
                optim::LBFGSBOptions lo;
                lo.maxiter = 1000;
                lo.ftol = 1e-6;
                result = optim::boundedLBFGS(obj, deRes.x, bounds, lo);
            } else if (mini == 5) { // hybrid: 1st NOT parallelized differential evolution then parameterizable refinement with L-BFGS-B
                optim::DifferentialEvolutionOptions o;
                o.latinHypercubeInit = true;
                o.deferredUpdating = false; // "immediate" updating, single-worker in the Python source
                o.polish = false;
                o.popsize = 8;
                o.seed = 42;
                o.maxiter = 500;
                o.tol = 1e-4;
                optim::OptResult deRes = optim::differentialEvolution(obj, bounds, o);
                optim::LBFGSBOptions lo;
                lo.maxiter = 1000;
                lo.ftol = 1e-6;
                result = optim::boundedLBFGS(obj, deRes.x, bounds, lo);
            }
        }

        if (verbose && !result.success) {
            std::cerr << "Warning: convergence not reached: " << result.message << std::endl;
        }

        // Extraction of results
        const std::vector<double> &lp = result.x;
        double r_est = std::exp(lp[0]);
        double k_est = std::exp(lp[1]);
        double z_est = std::exp(lp[2]);
        std::vector<double> q_est(nf), sig_CPUE_est(nf), sig_C_est(nf);
        for (int i = 0; i < nf; ++i) {
            q_est[i] = std::exp(lp[3 + i]);
            sig_CPUE_est[i] = std::exp(lp[3 + nf + i]);
            sig_C_est[i] = std::exp(lp[3 + 2 * nf + i]);
        }

        // Calculate estimated reference points
        double MSYLOut = std::pow(1.0 / (z_est + 1.0), 1.0 / z_est); // BMSY / K
        double BMSY = MSYLOut * k_est;
        double MSY = r_est * BMSY * (1.0 - std::pow(MSYLOut, z_est));
        double FMSY = r_est * z_est / (z_est + 1.0);

        // Final trajectories of variables
        std::vector<double> B(static_cast<std::size_t>(n) + 1, 0.0);
        B[0] = k_est;
        DynamicsLoopResult dl = dynamics_loop(std::move(B), r_est, k_est, z_est, q_est, E, n, nf);

        FitResult fr;
        fr.r = r_est;
        fr.k = k_est;
        fr.z = z_est;
        fr.q = q_est;
        fr.sigma_CPUE = sig_CPUE_est;
        fr.sigma_C = sig_C_est;
        fr.MSY = MSY;
        fr.FMSY = FMSY;
        fr.BMSY = BMSY;
        fr.MSYLOut = MSYLOut;
        fr.B = dl.B;
        fr.Bmoy = dl.Bmoy;
        fr.F = dl.F_tot;
        fr.Cpred = dl.Cpred;
        fr.nll = result.fun;
        fr.convergence = result.success;
        fr.x0 = lp;
        return fr;
    }

    // ============================================================== Section 14
    // Plugin Model 3 stock assessment for one stock
    // ==============================================================
    FitResult plugin_model_3_assess_stock(int i, std::size_t t, const std::vector<int> &nfleets_per_stock,
                                          const NdArray3 &catch_for_stock_assess, const NdArray3 &effort_for_stock_assess,
                                          const std::vector<double> &r_pop, double m, int nfleet, int length, int mini1,
                                          int mini2, bool verbose, const std::optional<std::vector<double>> &x0_prev) {
        (void)nfleet; // unused, mirrors the Python function's unused `nfleet` parameter
        const int nfPerStock = nfleets_per_stock[static_cast<std::size_t>(i)];

        std::size_t t_start;
        if (length == 0) {
            // consider the first non-0 year instead of the full time series, to accelerate computation
            std::size_t first_nonzero = 0;
            bool found = false;
            for (std::size_t tt = 0; tt <= t && !found; ++tt) {
                double s = 0.0;
                for (int f = 0; f < nfPerStock; ++f)
                    s += catch_for_stock_assess(f, i, tt);
                if (s > 0.0) {
                    first_nonzero = tt;
                    found = true;
                }
            }
            t_start = found ? first_nonzero : 0;
        } else {
            t_start = (t > static_cast<std::size_t>(length)) ? (t - static_cast<std::size_t>(length)) : 0;
        }

        // consider that EEZ stocks are harvested by 4 fleets and ABNJ by 2 -- i.e. nfPerStock fleets: (artisanal, industrial) x (EEZ, DWFN)
        const std::size_t T = t - t_start + 1;
        StockData data;
        data.nfleets = nfPerStock;
        data.C = NdArray2({T, static_cast<std::size_t>(nfPerStock)});
        data.E = NdArray2({T, static_cast<std::size_t>(nfPerStock)});
        for (std::size_t tt = 0; tt < T; ++tt) {
            for (int f = 0; f < nfPerStock; ++f) {
                data.C(tt, f) = catch_for_stock_assess(f, i, t_start + tt);
                data.E(tt, f) = effort_for_stock_assess(f, i, t_start + tt);
            }
        }
        data.prior_mean_r = r_pop[static_cast<std::size_t>(i)];
        // in logspace to ensure positivity and asymmetry of r/z. To set values, r/z +/- SD = exp(log(mean) +/- sd)
        data.prior_sd_logr = 1.0;
        data.prior_mean_z = m - 1.0;
        data.prior_sd_logz = 1.0;

        FitResult res = fit_one_stock(data, mini1, verbose, x0_prev);
        if (!res.convergence) {
            std::cout << "Convergence problem for assessing stock " << i << " with minimize = " << mini1
                      << ", shift to minimize = " << mini2 << std::endl;
            res = fit_one_stock(data, mini2, verbose, x0_prev);
            if (!res.convergence)
                std::cout << "stock " << i << " still not converged" << std::endl;
            else
                std::cout << "stock " << i << " finally converged" << std::endl;
        }
        return res;
    }

    // ============================================================== Section 14
    // Plugin Model 5 run stock assessment for all stocks
    // ==============================================================
    StockAssessResult PIM_assess_stocks(std::vector<double> Fmsy_est, std::vector<double> MSY_est, int parallel_mode,
                                        int nstocks, std::size_t t, std::size_t start_stock_assess,
                                        const std::vector<int> &nfleets_per_stock, const NdArray3 &catch_for_stock_assess,
                                        const NdArray3 &effort_for_stock_assess, const std::vector<double> &r_pop, double m,
                                        int nfleet, int length_stock_assess, bool verbose_stock_assessment,
                                        std::vector<std::optional<std::vector<double>>> x0_prev) {
        // NOTE: the Python source's plugin_model_3_assess_stock() indexes its own
        // x0_prev parameter with the stock index (x0_prev[i]), so it expects the
        // *whole* per-stock list -- which is what the Python parallel branch passes
        // (and works). The Python *sequential* branch instead pre-indexes with
        // x0_prev[i] before the call, causing a double-indexing bug that makes it
        // unable to run at all (verified against the live Python module -- see
        // PORTING_NOTES.md). This port's plugin_model_3_assess_stock takes a single
        // already-selected stock's warm start and does not re-index, so indexing
        // once here (for both parallel_mode values) is correct and matches the
        // Python parallel branch's working behaviour; it does not inherit the
        // Python sequential branch's bug.
        auto runOne = [&](int i, int mini1, int mini2) {
            return plugin_model_3_assess_stock(i, t, nfleets_per_stock, catch_for_stock_assess, effort_for_stock_assess,
                                               r_pop, m, nfleet, length_stock_assess, mini1, mini2,
                                               verbose_stock_assessment, x0_prev[static_cast<std::size_t>(i)]);
        };

        if (parallel_mode == 0) {
            const int mini1 = (t == start_stock_assess) ? 4 : 2;
            const int mini2 = 4;
            if (t == start_stock_assess)
                std::cout << "Start yearly stock assessment for " << nstocks << " stocks, sequential computing" << std::endl;
            for (int i = 0; i < nstocks; ++i) {
                FitResult res = runOne(i, mini1, mini2);
                if (res.convergence) {
                    Fmsy_est[static_cast<std::size_t>(i)] = res.FMSY;
                    MSY_est[static_cast<std::size_t>(i)] = res.MSY;
                    x0_prev[static_cast<std::size_t>(i)] = res.x0;
                }
            }
        } else {
            const int mini1 = (t == start_stock_assess) ? 5 : 2;
            const int mini2 = 5;
            if (t == start_stock_assess) {
                std::cout << "Start yearly stock assessment for " << nstocks << " stocks, parallel computing" << std::endl;
            }
            std::vector<FitResult> results(static_cast<std::size_t>(nstocks));
            const unsigned nThreads = std::max(1u, std::thread::hardware_concurrency());
            std::atomic<int> nextIdx{0};
            auto worker = [&]() {
                int i;
                while ((i = nextIdx.fetch_add(1)) < nstocks) {
                    results[static_cast<std::size_t>(i)] = runOne(i, mini1, mini2);
                }
            };
            std::vector<std::thread> pool;
            pool.reserve(nThreads);
            for (unsigned w = 0; w < nThreads; ++w)
                pool.emplace_back(worker);
            for (auto &th : pool)
                th.join();
            // results is an ordered vector: results[i] for stock i
            for (int i = 0; i < nstocks; ++i) {
                if (results[static_cast<std::size_t>(i)].convergence) {
                    Fmsy_est[static_cast<std::size_t>(i)] = results[static_cast<std::size_t>(i)].FMSY;
                    MSY_est[static_cast<std::size_t>(i)] = results[static_cast<std::size_t>(i)].MSY;
                    x0_prev[static_cast<std::size_t>(i)] = results[static_cast<std::size_t>(i)].x0;
                }
            }
        }

        StockAssessResult out;
        out.sum_MSY = sumVec(MSY_est);
        out.sum_FMSY = sumVec(Fmsy_est);
        out.Fmsy_est = std::move(Fmsy_est);
        out.MSY_est = std::move(MSY_est);
        out.x0_prev = std::move(x0_prev);
        return out;
    }

    // ============================================================== Section 15
    // Allocate yearly target to local and DWFN fleet for every stock
    // ==============================================================
    AllocateTargetResult PIM_allocate_target_local_DWFN(std::size_t t, int nEEZ, const std::vector<double> &Fmsy_est,
                                                        const NdArray3 &F_fleet_EEZ, const NdArray3 &F_fleet_DWFN,
                                                        const NdArray4 &F_DWFN_allocate, const NdArray2 &F_tot,
                                                        const std::vector<double> &prop2target,
                                                        const std::vector<double> &stock_surface) {
        const double alpha_DWFN_min = 0.2; // minimum floor of 20% reserved for DWFN fleets, even when F_DWFN = 0
        const std::size_t tPrev = (t > 0) ? t - 1 : 0;
        const std::size_t nfleet = F_fleet_EEZ.extent(0);
        const std::size_t nstocks = F_tot.extent(0);
        const std::size_t ne = static_cast<std::size_t>(nEEZ);

        // Allocates the target to EEZ-based fleet (both artisanal & industrial) in every EEZ
        std::vector<double> F_fleet_EEZ_sum(ne, 0.0);
        for (std::size_t e = 0; e < ne; ++e) {
            double s = 0.0;
            for (std::size_t f = 0; f < nfleet; ++f)
                s += F_fleet_EEZ(f, e, tPrev);
            F_fleet_EEZ_sum[e] = s;
        }
        std::vector<double> F_tot_prev(nstocks);
        for (std::size_t s = 0; s < nstocks; ++s)
            F_tot_prev[s] = F_tot(s, tPrev);

        std::vector<double> ratio_EEZ(ne);
        for (std::size_t e = 0; e < ne; ++e) {
            double r = (F_tot_prev[e] > 0.0) ? F_fleet_EEZ_sum[e] / F_tot_prev[e] : 1.0 - alpha_DWFN_min;
            ratio_EEZ[e] = std::min(r, 1.0 - alpha_DWFN_min);
        }
        // NOTE: the use of stock_surface here is specific to the Toy Model.
        std::vector<double> target_EEZ(ne);
        for (std::size_t e = 0; e < ne; ++e)
            target_EEZ[e] = prop2target[e] * Fmsy_est[e] * ratio_EEZ[e] * stock_surface[e];

        // Calculate the fraction of the target that is allocated to all DWFN fleets in every stock (EEZ + ABNJ)
        std::vector<double> F_DWFN_sum(nstocks, 0.0);
        const std::size_t nAllocFleet = F_DWFN_allocate.extent(0), nAllocEEZ = F_DWFN_allocate.extent(1);
        for (std::size_t s = 0; s < nstocks; ++s) {
            double sum = 0.0;
            for (std::size_t f = 0; f < nAllocFleet; ++f)
                for (std::size_t e = 0; e < nAllocEEZ; ++e)
                    sum += F_DWFN_allocate(f, e, s, tPrev);
            F_DWFN_sum[s] = sum;
        }
        std::vector<double> ratio_DWFN(nstocks);
        for (std::size_t s = 0; s < nstocks; ++s) {
            double r = (F_tot_prev[s] > 0.0) ? F_DWFN_sum[s] / F_tot_prev[s] : alpha_DWFN_min;
            ratio_DWFN[s] = std::max(r, alpha_DWFN_min);
        }
        std::vector<double> target_DWFN_stocks(nstocks);
        for (std::size_t s = 0; s < nstocks; ++s)
            target_DWFN_stocks[s] = prop2target[s] * Fmsy_est[s] * stock_surface[s] * ratio_DWFN[s];

        // total effort of every EEZ-based (artisanal + industrial) DWFN fleet
        std::vector<double> tmp2(ne, 0.0);
        for (std::size_t e = 0; e < ne; ++e) {
            double sum = 0.0;
            for (std::size_t f = 0; f < nfleet; ++f)
                sum += F_fleet_DWFN(f, e, tPrev);
            tmp2[e] = sum;
        }
        double tmp2_sum = sumVec(tmp2);
        std::vector<double> ratio_tmp2(ne);
        for (std::size_t e = 0; e < ne; ++e)
            ratio_tmp2[e] = (tmp2[e] > 0.0) ? tmp2[e] / ((tmp2_sum > 0.0) ? tmp2_sum : 1.0) : 1.0;

        // allocates the sum of the targets in every stock to every EEZ-based DWFN fleet (both artisanal & industrial)
        double targetDWFNStocksSum = sumVec(target_DWFN_stocks);
        std::vector<double> target_DWFN_fleet(ne);
        for (std::size_t e = 0; e < ne; ++e)
            target_DWFN_fleet[e] = targetDWFNStocksSum * ratio_tmp2[e];

        return {target_EEZ, target_DWFN_stocks, target_DWFN_fleet};
    }

} // namespace fishmip
