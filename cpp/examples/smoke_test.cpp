// Smoke test / usage example for the C++ port of FishMIP_OSP_Plugin_Models.py.
//
// Exercises every ported function at least once, including a real
// PIM_load_drivers() call against the actual Historical_drivers data files,
// and prints numeric checksums that were cross-checked against the Python
// module's output for the same inputs (see PORTING_NOTES.md "Validation").

#include <algorithm>
#include <cstdlib>
#include <iomanip>
#include <iostream>
#include <numeric>
#include <cmath>

#include "FishMIP_OSP_Plugin_Models.hpp"

using namespace fishmip;

namespace {

double checksum(const std::vector<double>& v) {
    double s = 0.0;
    for (double x : v)
        if (!std::isnan(x)) s += x;
    return s;
}
double checksum(const NdArray2& a) {
    double s = 0.0;
    for (double x : a.raw())
        if (!std::isnan(x)) s += x;
    return s;
}
double checksum(const NdArray3& a) {
    double s = 0.0;
    for (double x : a.raw())
        if (!std::isnan(x)) s += x;
    return s;
}

std::vector<std::size_t> findPositions(const std::vector<std::string>& haystack, const std::vector<std::string>& needles) {
    std::vector<std::size_t> pos(needles.size());
    for (std::size_t i = 0; i < needles.size(); ++i) {
        auto it = std::find(haystack.begin(), haystack.end(), needles[i]);
        pos[i] = static_cast<std::size_t>(it - haystack.begin());
    }
    return pos;
}

}  // namespace

int main(int argc, char** argv) {
    std::string dataDir = argc > 1 ? argv[1]
                                    : "../Drivers";
    // Defaults to ../config/control_parameters.csv, i.e. run from cpp/build/ (CMake's default binary dir).
    std::string paramsCsvPath = argc > 2 ? argv[2] : "../config/control_parameters.csv";
    std::cout << std::setprecision(15);

    // ---- Section 1-3 ----
    SimulationConfig sim = PIM_configure_simulation(0);
    std::cout << "nctry=" << sim.nctry << " nEEZ=" << sim.nEEZ << " nstocks=" << sim.nstocks
              << " xsize=" << sim.xsize << std::endl;
    StockAssessmentConfig sac = PIM_configure_stock_assessment(sim.nstocks);

    // ---- Section 4: real driver I/O ----
    int commod = 0;
    DriverData dd = PIM_load_drivers(commod, sim.nctry, sim.xsize, dataDir);
    std::cout << "countries=" << dd.countries.size() << " EEZ_countries=" << dd.EEZ_countries.size()
              << " EEZ_indices=" << dd.EEZ_indices.size() << " EEZ_surf=" << dd.EEZ_surf.size() << std::endl;
    std::cout << "checksum driver_X_fresh = " << checksum(dd.driver_X_fresh) << std::endl;
    std::cout << "checksum exported_frac = " << checksum(dd.exported_frac) << std::endl;
    std::cout << "checksum access_nat = " << checksum(dd.access_nat) << std::endl;
    std::cout << "checksum driver_U_national = " << checksum(dd.driver_U_national) << std::endl;
    std::cout << "checksum catchability_EEZ (pre control-params) = " << checksum(dd.catchability_EEZ) << std::endl;
    std::cout << "checksum comply_target_nat = " << checksum(dd.comply_target_nat) << std::endl;
    std::cout << "epsilon = " << dd.epsilon << "  Qcap_max[0..4] = ";
    for (int i = 0; i < 5; ++i) std::cout << dd.Qcap_max[i] << " ";
    std::cout << std::endl;

    ControlParameters cp = PIM_get_control_parameters(dd.catchability_EEZ, dd.catchability_DWFN, paramsCsvPath);
    std::cout << "checksum catchability_EEZ (post control-params) = " << checksum(dd.catchability_EEZ) << std::endl;
    std::cout << "M = (" << cp.M[0] << ", " << cp.M[1] << ")  f_growth_rate_max=" << cp.f_growth_rate_max << std::endl;

    // ---- Section 5 ----
    std::array<double, 3> MSY = {4.E7, 6.5E6, 3.E6};
    std::vector<double> Fmsy(static_cast<std::size_t>(sim.nstocks), 0.2);
    OSPVariables v = PIM_define_variables(sim.nfleet, sim.nctry, sim.nstocks, sim.nEEZ, sim.xsize, dd.countries,
                                           dd.access_nat, Fmsy, MSY, commod);
    std::cout << "access shape ok, sum=" << checksum(v.access) << std::endl;

    // ---- Section 6: price/consumption, both plugin_model_1_calc_price branches ----
    {
        // Pure global market branch (geo_agreg == 3 everywhere).
        std::vector<double> geo(static_cast<std::size_t>(sim.nctry), 3.0);
        std::vector<double> prod(static_cast<std::size_t>(sim.nctry), 100.0);
        std::vector<double> X(static_cast<std::size_t>(sim.nctry), 50.0);
        std::vector<double> W(static_cast<std::size_t>(sim.nctry), 1000.0);
        std::vector<double> aqua(static_cast<std::size_t>(sim.nctry), 0.0);
        PriceCalcResult r = plugin_model_1_calc_price(geo, sim.nctry, sim.nregion, sim.nsubregion, dd.region_idx,
                                                        dd.subregion_idx, prod, X, W, dd.epsilon, aqua);
        std::cout << "global-market price[0]=" << r.price_ctry[0] << " consumption[0]=" << r.consumption_ctry[0] << std::endl;
    }
    {
        // Mixed-market branch (as used for locally-sold artisanal catches).
        std::vector<double> geo(static_cast<std::size_t>(sim.nctry), 0.3);
        std::vector<double> prod(static_cast<std::size_t>(sim.nctry), 100.0);
        std::vector<double> X(static_cast<std::size_t>(sim.nctry), 50.0);
        std::vector<double> W(static_cast<std::size_t>(sim.nctry), 1000.0);
        std::vector<double> aqua(static_cast<std::size_t>(sim.nctry), 0.0);
        PriceCalcResult r = plugin_model_1_calc_price(geo, sim.nctry, sim.nregion, sim.nsubregion, dd.region_idx,
                                                        dd.subregion_idx, prod, X, W, dd.epsilon, aqua);
        std::cout << "mixed-market price[0]=" << r.price_ctry[0] << " consumption[0]=" << r.consumption_ctry[0] << std::endl;
    }

    // Full price/consumption step at t=0, wired against real drivers.
    std::vector<std::size_t> eezPos = findPositions(dd.countries, dd.EEZ_countries);
    std::vector<double> driv_X_f(sim.nctry), driv_W_f(sim.nctry), driv_X_c(sim.nctry), driv_W_c(sim.nctry);
    for (int c = 0; c < sim.nctry; ++c) {
        driv_X_f[c] = dd.driver_X_fresh(c, commod, 0);
        driv_W_f[c] = dd.driver_W_fresh(c, commod, 0);
        driv_X_c[c] = dd.driver_X_caned(c, commod, 0);
        driv_W_c[c] = dd.driver_W_caned(c, commod, 0);
    }
    std::vector<double> interProdAq(sim.nctry, 0.0);  // inter == 0 in the Python source
    // Give catch_step_EEZ some non-zero synthetic values to avoid a degenerate all-zero price step.
    for (int e = 0; e < sim.nEEZ; ++e) {
        v.catch_step_EEZ(0, e) = 10.0;
        v.catch_step_EEZ(1, e) = 100.0;
    }
    PriceConsumptionStepResult pc = PIM_calculate_prices_consumptions(
        v.price_EEZ, v.price_DWFN, v.catch_step_artis_traded_allctry, v.catch_step_indus_allctry,
        v.catch_step_artis_local_allctry, v.price_artisanal_traded_allctry, v.consumption_artisanal_traded_allctry,
        v.price_industrial_allctry, v.consumption_industrial_allctry, v.price_artisanal_local_allctry,
        v.consumption_artisanal_local_allctry, dd.exported_frac, v.catch_step_EEZ, v.catch_step_DWFN, eezPos,
        dd.geo_agreg_artisanal_traded, dd.geo_agreg_industrial, dd.geo_agreg_artisanal_local, sim.nctry, sim.nregion,
        sim.nsubregion, dd.region_idx, dd.subregion_idx, driv_X_f, driv_W_f, driv_X_c, driv_W_c, dd.epsilon,
        interProdAq, sim.EPS, 0, 1.0, commod);
    std::cout << "price_EEZ[0,0,0]=" << pc.price_EEZ_t(0, 0) << " price_EEZ[1,0,0]=" << pc.price_EEZ_t(1, 0) << std::endl;

    // ---- Section 7: aquaculture ----
    {
        std::vector<double> price(5, 2.0), Aprod(5, 100.0), dU(5, 1.0), dV(5, 0.5);
        std::vector<double> aq = mini_model_2_calc_aqua(price, Aprod, dU, dV, 1.0);
        std::cout << "aqua[0]=" << aq[0] << std::endl;
    }

    // ---- Section 8: effort dynamics ----
    {
        int nft = 2, nfy = 3;
        NdArray2 price({2, 3}, 5.0), Y({2, 3}, 1.0), effo({2, 3}, 10.0), q({2, 3}, 0.01);
        std::vector<double> Ftarget = {0.2, 0.3, 0.1};
        NdArray2 comply({2, 3}, 0.8), dU({2, 3}, 1.0), dV({2, 3}, 0.2);
        std::vector<double> M = {cp.M[0], cp.M[1]};
        EffortCalcResult ec = plugin_model_2_calc_effort(nft, nfy, price, Y, effo, q, Ftarget, comply, 5.0, dU, dV, M,
                                                          1.0, 1, cp.f_growth_rate_max, cp.f_growth_rate_half_sat);
        std::cout << "Ftot=[" << ec.Ftot[0] << ", " << ec.Ftot[1] << ", " << ec.Ftot[2] << "]\n";
    }

    // ---- Section 10-12: dynamics_loop / objective / fit_one_stock on synthetic data ----
    {
        int n = 30, nf = 2;
        NdArray2 Ctrue({static_cast<std::size_t>(n), static_cast<std::size_t>(nf)});
        NdArray2 E({static_cast<std::size_t>(n), static_cast<std::size_t>(nf)});
        double r_true = 0.3, k_true = 1000.0, z_true = 1.0;
        std::vector<double> q_true = {5e-4, 2e-4};
        std::vector<double> B(n + 1, 0.0);
        B[0] = k_true;
        for (int t = 0; t < n; ++t) {
            E(t, 0) = 200.0 + 5.0 * t;
            E(t, 1) = 400.0 + 2.0 * t;
        }
        DynamicsLoopResult dl = dynamics_loop(B, r_true, k_true, z_true, q_true, E, n, nf);
        for (int t = 0; t < n; ++t)
            for (int i = 0; i < nf; ++i) Ctrue(t, i) = dl.Cpred(t, i);

        StockData sd;
        sd.nfleets = nf;
        sd.prior_mean_r = 0.25;
        sd.prior_sd_logr = 1.0;
        sd.prior_mean_z = 0.9;
        sd.prior_sd_logz = 1.0;
        sd.C = Ctrue;
        sd.E = E;

        for (int mini : {0, 1, 2, 3, 4, 5}) {
            FitResult fit = fit_one_stock(sd, mini, /*verbose=*/false, std::nullopt);
            std::cout << "fit_one_stock(mini=" << mini << "): converged=" << fit.convergence << " r=" << fit.r
                      << " k=" << fit.k << " z=" << fit.z << " FMSY=" << fit.FMSY << " MSY=" << fit.MSY
                      << " nll=" << fit.nll << std::endl;
        }
        std::cout << "  (true r=" << r_true << " k=" << k_true << " z=" << z_true << ")\n";

        // ---- Section 13: PIM_assess_stocks, both sequential and threaded paths ----
        int nstocksSmall = 4;
        std::vector<int> nfleetsPerStockSmall(static_cast<std::size_t>(nstocksSmall), nf);
        NdArray3 catchForAssess({static_cast<std::size_t>(nf), static_cast<std::size_t>(nstocksSmall),
                                  static_cast<std::size_t>(n)});
        NdArray3 effortForAssess({static_cast<std::size_t>(nf), static_cast<std::size_t>(nstocksSmall),
                                   static_cast<std::size_t>(n)});
        for (int s = 0; s < nstocksSmall; ++s)
            for (int t = 0; t < n; ++t)
                for (int f = 0; f < nf; ++f) {
                    catchForAssess(f, s, t) = Ctrue(t, f);
                    effortForAssess(f, s, t) = E(t, f);
                }
        std::vector<double> r_pop(static_cast<std::size_t>(nstocksSmall), 0.25);
        std::vector<double> FmsyEstSmall(static_cast<std::size_t>(nstocksSmall), 0.1);
        std::vector<double> MSYEstSmall(static_cast<std::size_t>(nstocksSmall), 0.0);
        std::vector<std::optional<std::vector<double>>> x0PrevSmall(static_cast<std::size_t>(nstocksSmall),
                                                                       std::nullopt);
        StockAssessResult seqRes =
            PIM_assess_stocks(FmsyEstSmall, MSYEstSmall, /*parallel_mode=*/0, nstocksSmall, /*t=*/static_cast<std::size_t>(n - 1),
                               /*start_stock_assess=*/static_cast<std::size_t>(n - 1), nfleetsPerStockSmall,
                               catchForAssess, effortForAssess, r_pop, /*m=*/z_true + 1.0, nf, /*length=*/0,
                               /*verbose=*/false, x0PrevSmall);
        std::cout << "PIM_assess_stocks(sequential): sum_MSY=" << seqRes.sum_MSY << " sum_FMSY=" << seqRes.sum_FMSY
                   << std::endl;
        StockAssessResult parRes =
            PIM_assess_stocks(FmsyEstSmall, MSYEstSmall, /*parallel_mode=*/1, nstocksSmall, /*t=*/static_cast<std::size_t>(n - 1),
                               /*start_stock_assess=*/static_cast<std::size_t>(n - 1), nfleetsPerStockSmall,
                               catchForAssess, effortForAssess, r_pop, /*m=*/z_true + 1.0, nf, /*length=*/0,
                               /*verbose=*/false, x0PrevSmall);
        std::cout << "PIM_assess_stocks(parallel):   sum_MSY=" << parRes.sum_MSY << " sum_FMSY=" << parRes.sum_FMSY
                   << std::endl;
    }

    // ---- Section 14: target allocation ----
    {
        int nEEZ = 3;
        NdArray3 F_fleet_EEZ({2, 3, 2}, 0.5);
        NdArray3 F_fleet_DWFN({2, 3, 2}, 0.2);
        NdArray4 F_DWFN_allocate({2, 3, 4, 2}, 0.1);
        NdArray2 F_tot({4, 2}, 1.0);
        std::vector<double> Fmsy_est3 = {0.3, 0.4, 0.5, 0.2};
        std::vector<double> prop2target3(4, 1.0);
        std::vector<double> stock_surface3 = {1.0, 2.0, 3.0, 1.0};
        AllocateTargetResult ar = PIM_allocate_target_local_DWFN(1, nEEZ, Fmsy_est3, F_fleet_EEZ, F_fleet_DWFN,
                                                                  F_DWFN_allocate, F_tot, prop2target3, stock_surface3);
        std::cout << "target_EEZ=[" << ar.target_EEZ[0] << ", " << ar.target_EEZ[1] << ", " << ar.target_EEZ[2]
                  << "]\n";
    }

    std::cout << "smoke_test: all sections executed without throwing.\n";
    return 0;
}
