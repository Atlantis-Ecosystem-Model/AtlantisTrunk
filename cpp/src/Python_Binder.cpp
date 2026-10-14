
#ifdef PYTHON_WRAPPER
#include <pybind11/pybind11.h>
#include <pybind11/stl.h> // for std::map, std::vector, std::string conversions
#include <pybind11/numpy.h>
#include <cstring>
namespace py = pybind11;
#define stdout_macro std::cout
#define stderr_macro std::cerr

// #include "SimulationState.h"
#include <iostream>
#include "FishMIP_OSP_Plugin_Models.hpp"

namespace {
    // Read-only input: forgiving conversion (accepts int arrays, lists, non-contiguous -- pybind11 copies).
    template <std::size_t N>
    NdArray<N> toNdArray(const py::array_t<double, py::array::c_style | py::array::forcecast> &arr) {
        if (static_cast<std::size_t>(arr.ndim()) != N)
            throw std::runtime_error("expected a " + std::to_string(N) + "D array, got ndim=" + std::to_string(arr.ndim()));
        std::array<std::size_t, N> shape{};
        for (std::size_t d = 0; d < N; ++d) shape[d] = static_cast<std::size_t>(arr.shape(d));
        NdArray<N> out(shape);
        std::memcpy(out.data(), arr.data(), out.size() * sizeof(double));
        return out;
    }

    template <std::size_t N>
    py::array_t<double> fromNdArray(const NdArray<N> &nd) {
        std::vector<py::ssize_t> shape(nd.shape().begin(), nd.shape().end());
        py::array_t<double> out(shape);
        std::memcpy(out.mutable_data(), nd.data(), nd.size() * sizeof(double));
        return out;
    }

    // In-place mutated parameter: the pybind11 argument type itself is
    // `py::array_t<double, py::array::c_style>` (NO forcecast) so pybind11 only
    // accepts an already-double, already-C-contiguous array and never silently
    // copies -- that's what lets writeBack() below land in the CALLER's original
    // buffer. Reject anything else with a clear TypeError at the pybind11 layer.
    template <std::size_t N>
    NdArray<N> toNdArrayInPlace(py::array_t<double, py::array::c_style> &arr) {
        if (static_cast<std::size_t>(arr.ndim()) != N)
            throw std::runtime_error("expected a " + std::to_string(N) + "D float64 C-contiguous array, got ndim=" + std::to_string(arr.ndim()));
        std::array<std::size_t, N> shape{};
        for (std::size_t d = 0; d < N; ++d) shape[d] = static_cast<std::size_t>(arr.shape(d));
        NdArray<N> out(shape);
        std::memcpy(out.data(), arr.data(), out.size() * sizeof(double));
        return out;
    }

    template <std::size_t N>
    void writeBack(py::array_t<double, py::array::c_style> &arr, const NdArray<N> &nd) {
        std::memcpy(arr.mutable_data(), nd.data(), nd.size() * sizeof(double));
    }
}

// Bind an NdArray<N>-typed struct field as a numpy-array-valued property
// (def_readwrite doesn't work here: pybind11 has no type caster for NdArray).
// Cls is a C++ class, of type <NdArray<N>>
// PyCls: an objet of type pybind11::class_<Cls> (the Python class being defined
// N: the dimension of the table
// field: the name of the field.
// Note that #field converts the name of a field into a string. If field=tableau, #field="tableau"
// The first property is the getter function. Self points to the C++ function, takes the field and converts into a numpy array using
// fromNdArray function
// The second property is the setter: py::array::c_style  force a C order memory layout, py::array::forcecast allows for implicit conversions

#define FISHMIP_NDPROP(PyCls, Cls, N, field) \
    (PyCls).def_property(#field, \
        [](const Cls &self) { return fromNdArray<N>(self.field); }, \
        [](Cls &self, py::array_t<double, py::array::c_style | py::array::forcecast> arr) { self.field = toNdArray<N>(arr); })

PYBIND11_MODULE(FishMIP_OSP_PIM_Models, m) {
    m.doc() = "FishMIP OSP Plugin Model Python bindings";

    py::class_<fishmip::SimulationConfig>(m, "SimulationConfig")
        .def(py::init<>())
        .def_readwrite("nctry", &fishmip::SimulationConfig::nctry)
        .def_readwrite("nsubregion", &fishmip::SimulationConfig::nsubregion)
        .def_readwrite("nregion", &fishmip::SimulationConfig::nregion)
        .def_readwrite("nEEZ", &fishmip::SimulationConfig::nEEZ)
        .def_readwrite("nfleet", &fishmip::SimulationConfig::nfleet)
        .def_readwrite("nstocks", &fishmip::SimulationConfig::nstocks)
        .def_readwrite("time_min", &fishmip::SimulationConfig::time_min)
        .def_readwrite("time_max", &fishmip::SimulationConfig::time_max)
        .def_readwrite("xsize", &fishmip::SimulationConfig::xsize)
        .def_readwrite("EPS", &fishmip::SimulationConfig::EPS)
        .def("__str__", [](const fishmip::SimulationConfig &c) {
            std::ostringstream oss;
            oss << "SimulationConfig("
                << "nctry=" << c.nctry << ", "
                << "nsubregion=" << c.nsubregion << ", "
                << "nregion=" << c.nregion << ", "
                << "nEEZ=" << c.nEEZ << ", "
                << "nfleet=" << c.nfleet << ", "
                << "nstocks=" << c.nstocks << ", "
                << "time_min=" << c.time_min << ", "
                << "time_max=" << c.time_max << ", "
                << "xsize=" << c.xsize << ", "
                << "EPS=" << c.EPS
                << ")";
            return oss.str();
        })
        .def("__repr__", [](const fishmip::SimulationConfig &c) {
            std::ostringstream oss;
            oss << "<SimulationConfig nctry=" << c.nctry
                << " nsubregion=" << c.nsubregion
                << " nregion=" << c.nregion
                << " nEEZ=" << c.nEEZ
                << " nfleet=" << c.nfleet
                << " nstocks=" << c.nstocks
                << " time_min=" << c.time_min
                << " time_max=" << c.time_max
                << " xsize=" << c.xsize
                << " EPS=" << c.EPS
                << ">";
            return oss.str();
        });

    // ---------------------------------------------------------------- ControlParameters
    py::class_<fishmip::ControlParameters> controlParameters(m, "ControlParameters");
    controlParameters.def(py::init<>())
        .def_readwrite("M", &fishmip::ControlParameters::M)
        .def_readwrite("effort_growth_lim", &fishmip::ControlParameters::effort_growth_lim)
        .def_readwrite("f_growth_rate_max", &fishmip::ControlParameters::f_growth_rate_max)
        .def_readwrite("f_growth_rate_half_sat", &fishmip::ControlParameters::f_growth_rate_half_sat);

    // ---------------------------------------------------------------- StockAssessmentConfig
    py::class_<fishmip::StockAssessmentConfig> stockAssessmentConfig(m, "StockAssessmentConfig");
    stockAssessmentConfig.def(py::init<>())
        .def_readwrite("start_stock_assess", &fishmip::StockAssessmentConfig::start_stock_assess)
        .def_readwrite("parall_stock_assess", &fishmip::StockAssessmentConfig::parall_stock_assess)
        .def_readwrite("length_stock_assess", &fishmip::StockAssessmentConfig::length_stock_assess)
        .def_readwrite("verbose_stock_assessment", &fishmip::StockAssessmentConfig::verbose_stock_assessment)
        .def_readwrite("x0_prev", &fishmip::StockAssessmentConfig::x0_prev);

    // ---------------------------------------------------------------- DriverData
    py::class_<fishmip::DriverData> driverData(m, "DriverData");
    driverData.def(py::init<>())
        .def_readwrite("countries", &fishmip::DriverData::countries)
        .def_readwrite("EEZ_countries", &fishmip::DriverData::EEZ_countries)
        .def_readwrite("EEZ_indices", &fishmip::DriverData::EEZ_indices)
        .def_readwrite("EEZ_surf", &fishmip::DriverData::EEZ_surf)
        .def_readwrite("region_idx", &fishmip::DriverData::region_idx)
        .def_readwrite("subregion_idx", &fishmip::DriverData::subregion_idx)
        .def_readwrite("risk_invest", &fishmip::DriverData::risk_invest)
        .def_readwrite("epsilon", &fishmip::DriverData::epsilon)
        .def_readwrite("Qcap_max", &fishmip::DriverData::Qcap_max)
        .def_readwrite("inter", &fishmip::DriverData::inter);
    FISHMIP_NDPROP(driverData, fishmip::DriverData, 3, driver_X_fresh);
    FISHMIP_NDPROP(driverData, fishmip::DriverData, 3, driver_W_fresh);
    FISHMIP_NDPROP(driverData, fishmip::DriverData, 3, driver_X_caned);
    FISHMIP_NDPROP(driverData, fishmip::DriverData, 3, driver_W_caned);
    FISHMIP_NDPROP(driverData, fishmip::DriverData, 2, exported_frac);
    FISHMIP_NDPROP(driverData, fishmip::DriverData, 2, DWFN_invest_capacity_nat);
    FISHMIP_NDPROP(driverData, fishmip::DriverData, 3, access_nat);
    FISHMIP_NDPROP(driverData, fishmip::DriverData, 3, driver_U_national);
    FISHMIP_NDPROP(driverData, fishmip::DriverData, 3, driver_V_national);
    FISHMIP_NDPROP(driverData, fishmip::DriverData, 3, catchability_EEZ);
    FISHMIP_NDPROP(driverData, fishmip::DriverData, 3, catchability_DWFN);
    FISHMIP_NDPROP(driverData, fishmip::DriverData, 3, comply_target_nat);
    FISHMIP_NDPROP(driverData, fishmip::DriverData, 2, geo_agreg_artisanal_local);
    FISHMIP_NDPROP(driverData, fishmip::DriverData, 2, geo_agreg_artisanal_traded);
    FISHMIP_NDPROP(driverData, fishmip::DriverData, 2, geo_agreg_industrial);

    // ---------------------------------------------------------------- OSPVariables
    py::class_<fishmip::OSPVariables> ospVariables(m, "OSPVariables");
    ospVariables.def(py::init<>())
        .def_readwrite("prop_EEZ_DWFN", &fishmip::OSPVariables::prop_EEZ_DWFN)
        .def_readwrite("nfleets_per_stock", &fishmip::OSPVariables::nfleets_per_stock)
        .def_readwrite("prop2target", &fishmip::OSPVariables::prop2target)
        .def_readwrite("Fmsy_est", &fishmip::OSPVariables::Fmsy_est)
        .def_readwrite("MSY_est", &fishmip::OSPVariables::MSY_est)
        .def_readwrite("global_MSY", &fishmip::OSPVariables::global_MSY)
        .def_readwrite("global_FMSY", &fishmip::OSPVariables::global_FMSY)
        .def_readwrite("target_EEZ", &fishmip::OSPVariables::target_EEZ)
        .def_readwrite("target_DWFN_fleet", &fishmip::OSPVariables::target_DWFN_fleet)
        .def_readwrite("target_DWFN_stocks", &fishmip::OSPVariables::target_DWFN_stocks)
        .def_readwrite("target_DWFN_init", &fishmip::OSPVariables::target_DWFN_init)
        .def_readwrite("ratio_exported_over_local_artisanal_prices", &fishmip::OSPVariables::ratio_exported_over_local_artisanal_prices);
    FISHMIP_NDPROP(ospVariables, fishmip::OSPVariables, 3, access);
    FISHMIP_NDPROP(ospVariables, fishmip::OSPVariables, 3, effort_EEZ);
    FISHMIP_NDPROP(ospVariables, fishmip::OSPVariables, 3, effort_DWFN);
    FISHMIP_NDPROP(ospVariables, fishmip::OSPVariables, 4, allocate_effort_DWFN);
    FISHMIP_NDPROP(ospVariables, fishmip::OSPVariables, 3, effort_for_stock_assess);
    FISHMIP_NDPROP(ospVariables, fishmip::OSPVariables, 2, catch_step_EEZ);
    FISHMIP_NDPROP(ospVariables, fishmip::OSPVariables, 3, catch_EEZ);
    FISHMIP_NDPROP(ospVariables, fishmip::OSPVariables, 3, catch_step_DWFN);
    FISHMIP_NDPROP(ospVariables, fishmip::OSPVariables, 2, catch_step_DWFN_tot);
    FISHMIP_NDPROP(ospVariables, fishmip::OSPVariables, 3, catch_DWFN);
    FISHMIP_NDPROP(ospVariables, fishmip::OSPVariables, 2, catch_step_artis_local_allctry);
    FISHMIP_NDPROP(ospVariables, fishmip::OSPVariables, 2, catch_step_artis_traded_allctry);
    FISHMIP_NDPROP(ospVariables, fishmip::OSPVariables, 2, catch_step_indus_allctry);
    FISHMIP_NDPROP(ospVariables, fishmip::OSPVariables, 3, catch_for_stock_assess);
    FISHMIP_NDPROP(ospVariables, fishmip::OSPVariables, 2, F_init);
    FISHMIP_NDPROP(ospVariables, fishmip::OSPVariables, 3, F_fleet_EEZ);
    FISHMIP_NDPROP(ospVariables, fishmip::OSPVariables, 3, F_fleet_DWFN);
    FISHMIP_NDPROP(ospVariables, fishmip::OSPVariables, 4, F_DWFN_allocate);
    FISHMIP_NDPROP(ospVariables, fishmip::OSPVariables, 2, F_tot);
    FISHMIP_NDPROP(ospVariables, fishmip::OSPVariables, 3, price_EEZ);
    FISHMIP_NDPROP(ospVariables, fishmip::OSPVariables, 3, price_DWFN);
    FISHMIP_NDPROP(ospVariables, fishmip::OSPVariables, 2, price_artisanal_traded_allctry);
    FISHMIP_NDPROP(ospVariables, fishmip::OSPVariables, 2, price_artisanal_local_allctry);
    FISHMIP_NDPROP(ospVariables, fishmip::OSPVariables, 2, price_industrial_allctry);
    FISHMIP_NDPROP(ospVariables, fishmip::OSPVariables, 2, consumption_artisanal_traded_allctry);
    FISHMIP_NDPROP(ospVariables, fishmip::OSPVariables, 2, consumption_artisanal_local_allctry);
    FISHMIP_NDPROP(ospVariables, fishmip::OSPVariables, 2, consumption_industrial_allctry);
    FISHMIP_NDPROP(ospVariables, fishmip::OSPVariables, 2, prod_aqua);

    // ---------------------------------------------------------------- PriceCalcResult
    py::class_<fishmip::PriceCalcResult>(m, "PriceCalcResult")
        .def(py::init<>())
        .def_readwrite("price_ctry", &fishmip::PriceCalcResult::price_ctry)
        .def_readwrite("consumption_ctry", &fishmip::PriceCalcResult::consumption_ctry);

    // ---------------------------------------------------------------- PriceConsumptionStepResult
    py::class_<fishmip::PriceConsumptionStepResult> priceConsumptionStepResult(m, "PriceConsumptionStepResult");
    priceConsumptionStepResult.def(py::init<>())
        .def_readwrite("price_artisanal_traded", &fishmip::PriceConsumptionStepResult::price_artisanal_traded)
        .def_readwrite("consumption_artisanal_traded", &fishmip::PriceConsumptionStepResult::consumption_artisanal_traded)
        .def_readwrite("price_industrial", &fishmip::PriceConsumptionStepResult::price_industrial)
        .def_readwrite("consumption_industrial", &fishmip::PriceConsumptionStepResult::consumption_industrial)
        .def_readwrite("price_artisanal_local", &fishmip::PriceConsumptionStepResult::price_artisanal_local)
        .def_readwrite("consumption_artisanal_local", &fishmip::PriceConsumptionStepResult::consumption_artisanal_local)
        .def_readwrite("ratio_exported_over_local_artisanal_prices", &fishmip::PriceConsumptionStepResult::ratio_exported_over_local_artisanal_prices);
    FISHMIP_NDPROP(priceConsumptionStepResult, fishmip::PriceConsumptionStepResult, 2, price_EEZ_t);
    FISHMIP_NDPROP(priceConsumptionStepResult, fishmip::PriceConsumptionStepResult, 2, price_DWFN_t);

    // ---------------------------------------------------------------- EffortCalcResult
    py::class_<fishmip::EffortCalcResult> effortCalcResult(m, "EffortCalcResult");
    effortCalcResult.def(py::init<>())
        .def_readwrite("Ftot", &fishmip::EffortCalcResult::Ftot);
    FISHMIP_NDPROP(effortCalcResult, fishmip::EffortCalcResult, 2, f);
    FISHMIP_NDPROP(effortCalcResult, fishmip::EffortCalcResult, 2, F);

    // ---------------------------------------------------------------- DynamicsLoopResult
    py::class_<fishmip::DynamicsLoopResult> dynamicsLoopResult(m, "DynamicsLoopResult");
    dynamicsLoopResult.def(py::init<>())
        .def_readwrite("B", &fishmip::DynamicsLoopResult::B)
        .def_readwrite("Bmoy", &fishmip::DynamicsLoopResult::Bmoy)
        .def_readwrite("F_tot", &fishmip::DynamicsLoopResult::F_tot)
        .def_readwrite("pen", &fishmip::DynamicsLoopResult::pen);
    FISHMIP_NDPROP(dynamicsLoopResult, fishmip::DynamicsLoopResult, 2, Cpred);

    // ---------------------------------------------------------------- StockData
    py::class_<fishmip::StockData> stockData(m, "StockData");
    stockData.def(py::init<>())
        .def_readwrite("nfleets", &fishmip::StockData::nfleets)
        .def_readwrite("prior_mean_r", &fishmip::StockData::prior_mean_r)
        .def_readwrite("prior_sd_logr", &fishmip::StockData::prior_sd_logr)
        .def_readwrite("prior_mean_z", &fishmip::StockData::prior_mean_z)
        .def_readwrite("prior_sd_logz", &fishmip::StockData::prior_sd_logz);
    FISHMIP_NDPROP(stockData, fishmip::StockData, 2, C);
    FISHMIP_NDPROP(stockData, fishmip::StockData, 2, E);

    // ---------------------------------------------------------------- FitResult
    py::class_<fishmip::FitResult> fitResult(m, "FitResult");
    fitResult.def(py::init<>())
        .def_readwrite("r", &fishmip::FitResult::r)
        .def_readwrite("k", &fishmip::FitResult::k)
        .def_readwrite("z", &fishmip::FitResult::z)
        .def_readwrite("q", &fishmip::FitResult::q)
        .def_readwrite("sigma_CPUE", &fishmip::FitResult::sigma_CPUE)
        .def_readwrite("sigma_C", &fishmip::FitResult::sigma_C)
        .def_readwrite("MSY", &fishmip::FitResult::MSY)
        .def_readwrite("FMSY", &fishmip::FitResult::FMSY)
        .def_readwrite("BMSY", &fishmip::FitResult::BMSY)
        .def_readwrite("MSYLOut", &fishmip::FitResult::MSYLOut)
        .def_readwrite("B", &fishmip::FitResult::B)
        .def_readwrite("Bmoy", &fishmip::FitResult::Bmoy)
        .def_readwrite("F", &fishmip::FitResult::F)
        .def_readwrite("nll", &fishmip::FitResult::nll)
        .def_readwrite("convergence", &fishmip::FitResult::convergence)
        .def_readwrite("x0", &fishmip::FitResult::x0);
    FISHMIP_NDPROP(fitResult, fishmip::FitResult, 2, Cpred);

    // ---------------------------------------------------------------- StockAssessResult
    py::class_<fishmip::StockAssessResult>(m, "StockAssessResult")
        .def(py::init<>())
        .def_readwrite("Fmsy_est", &fishmip::StockAssessResult::Fmsy_est)
        .def_readwrite("MSY_est", &fishmip::StockAssessResult::MSY_est)
        .def_readwrite("x0_prev", &fishmip::StockAssessResult::x0_prev)
        .def_readwrite("sum_MSY", &fishmip::StockAssessResult::sum_MSY)
        .def_readwrite("sum_FMSY", &fishmip::StockAssessResult::sum_FMSY);

    // ---------------------------------------------------------------- AllocateTargetResult
    py::class_<fishmip::AllocateTargetResult>(m, "AllocateTargetResult")
        .def(py::init<>())
        .def_readwrite("target_EEZ", &fishmip::AllocateTargetResult::target_EEZ)
        .def_readwrite("target_DWFN_stocks", &fishmip::AllocateTargetResult::target_DWFN_stocks)
        .def_readwrite("target_DWFN_fleet", &fishmip::AllocateTargetResult::target_DWFN_fleet);

    // py::module_ parameters_mod = m.def_submodule("parameters", "Parameters related functions");
    // parameters_mod.def("get_parameter", &get_parameter, py::arg("parameter_name"),
    //                    "Get the value of a parameter by name");
    // parameters_mod.def("init_simulation", &init_simulation, py::arg("filename"),
    //                    "Initialize the global SimulationState from a configuration file");
    // py::module_ m = m.def_submodule("pim", "PIM related functions");

    m.def("PIM_configure_simulation", &fishmip::PIM_configure_simulation, py::arg("nosp"), "Configure the simulation parameters");

    m.def("PIM_get_control_parameters",
        [](py::array_t<double, py::array::c_style> catchability_EEZ,
           py::array_t<double, py::array::c_style> catchability_DWFN,
           const std::string &paramsCsvPath) {
            NdArray3 eez = toNdArrayInPlace<3>(catchability_EEZ);
            NdArray3 dwfn = toNdArrayInPlace<3>(catchability_DWFN);
            fishmip::ControlParameters cp = fishmip::PIM_get_control_parameters(eez, dwfn, paramsCsvPath);
            writeBack<3>(catchability_EEZ, eez);
            writeBack<3>(catchability_DWFN, dwfn);
            return cp;
        },
        py::arg("catchability_EEZ"), py::arg("catchability_DWFN"), py::arg("paramsCsvPath"),
        "Get control parameters from CSV, scaling catchability_EEZ/catchability_DWFN in place");

    m.def("PIM_configure_stock_assessment", &fishmip::PIM_configure_stock_assessment, py::arg("nstocks"), "Configure stock assessment parameters");

    m.def("PIM_load_drivers", &fishmip::PIM_load_drivers, py::arg("commod"), py::arg("nctry"), py::arg("xsize"), py::arg("dataDir"), "Load driver data");

    m.def("PIM_define_variables",
        [](int nfleet, int nctry, int nstocks, int nEEZ, std::size_t xsize,
           const std::vector<std::string> &country_coord,
           py::array_t<double, py::array::c_style | py::array::forcecast> access_nat,
           const std::vector<double> &Fmsy, const std::array<double, 3> &MSY, int commod) {
            NdArray3 access_nat_nd = toNdArray<3>(access_nat);
            return fishmip::PIM_define_variables(nfleet, nctry, nstocks, nEEZ, xsize, country_coord, access_nat_nd, Fmsy, MSY, commod);
        },
        py::arg("nfleet"), py::arg("nctry"), py::arg("nstocks"), py::arg("nEEZ"), py::arg("xsize"),
        py::arg("country_coord"), py::arg("access_nat"), py::arg("Fmsy"), py::arg("MSY"), py::arg("commod"),
        "Initialise the OSPVariables state for a simulation run");

    m.def("plugin_model_1_calc_price", &fishmip::plugin_model_1_calc_price,
        py::arg("geo_agreg"), py::arg("nctry"), py::arg("nregion"), py::arg("nsubregion"),
        py::arg("region_idx"), py::arg("subregion_idx"), py::arg("prod"), py::arg("X"), py::arg("W"),
        py::arg("epsilon"), py::arg("aqua"),
        "Compute per-country price and consumption from a geographic-aggregation market model");

    m.def("PIM_calculate_prices_consumptions",
        [](py::array_t<double, py::array::c_style> price_EEZ,
           py::array_t<double, py::array::c_style> price_DWFN,
           py::array_t<double, py::array::c_style> catch_step_artis_traded_allctry,
           py::array_t<double, py::array::c_style> catch_step_indus_allctry,
           py::array_t<double, py::array::c_style> catch_step_artis_local_allctry,
           py::array_t<double, py::array::c_style> price_artisanal_traded_allctry,
           py::array_t<double, py::array::c_style> consumption_artisanal_traded_allctry,
           py::array_t<double, py::array::c_style> price_industrial_allctry,
           py::array_t<double, py::array::c_style> consumption_industrial_allctry,
           py::array_t<double, py::array::c_style> price_artisanal_local_allctry,
           py::array_t<double, py::array::c_style> consumption_artisanal_local_allctry,
           py::array_t<double, py::array::c_style | py::array::forcecast> exported_frac,
           py::array_t<double, py::array::c_style | py::array::forcecast> catch_step_EEZ,
           py::array_t<double, py::array::c_style | py::array::forcecast> catch_step_DWFN,
           const std::vector<std::size_t> &eezPosInCountryCoord,
           py::array_t<double, py::array::c_style | py::array::forcecast> geo_agreg_artisanal_traded,
           py::array_t<double, py::array::c_style | py::array::forcecast> geo_agreg_industrial,
           py::array_t<double, py::array::c_style | py::array::forcecast> geo_agreg_artisanal_local,
           int nctry, int nregion, int nsubregion,
           const std::vector<int> &region_idx, const std::vector<int> &subregion_idx,
           const std::vector<double> &driv_X_f, const std::vector<double> &driv_W_f,
           const std::vector<double> &driv_X_c, const std::vector<double> &driv_W_c,
           double epsilon, const std::vector<double> &inter_prod_aq, double EPS,
           std::size_t t, double dt, int commod) {
            NdArray3 price_EEZ_nd = toNdArrayInPlace<3>(price_EEZ);
            NdArray3 price_DWFN_nd = toNdArrayInPlace<3>(price_DWFN);
            NdArray2 catch_step_artis_traded_allctry_nd = toNdArrayInPlace<2>(catch_step_artis_traded_allctry);
            NdArray2 catch_step_indus_allctry_nd = toNdArrayInPlace<2>(catch_step_indus_allctry);
            NdArray2 catch_step_artis_local_allctry_nd = toNdArrayInPlace<2>(catch_step_artis_local_allctry);
            NdArray2 price_artisanal_traded_allctry_nd = toNdArrayInPlace<2>(price_artisanal_traded_allctry);
            NdArray2 consumption_artisanal_traded_allctry_nd = toNdArrayInPlace<2>(consumption_artisanal_traded_allctry);
            NdArray2 price_industrial_allctry_nd = toNdArrayInPlace<2>(price_industrial_allctry);
            NdArray2 consumption_industrial_allctry_nd = toNdArrayInPlace<2>(consumption_industrial_allctry);
            NdArray2 price_artisanal_local_allctry_nd = toNdArrayInPlace<2>(price_artisanal_local_allctry);
            NdArray2 consumption_artisanal_local_allctry_nd = toNdArrayInPlace<2>(consumption_artisanal_local_allctry);

            NdArray2 exported_frac_nd = toNdArray<2>(exported_frac);
            NdArray2 catch_step_EEZ_nd = toNdArray<2>(catch_step_EEZ);
            NdArray3 catch_step_DWFN_nd = toNdArray<3>(catch_step_DWFN);
            NdArray2 geo_agreg_artisanal_traded_nd = toNdArray<2>(geo_agreg_artisanal_traded);
            NdArray2 geo_agreg_industrial_nd = toNdArray<2>(geo_agreg_industrial);
            NdArray2 geo_agreg_artisanal_local_nd = toNdArray<2>(geo_agreg_artisanal_local);

            fishmip::PriceConsumptionStepResult result = fishmip::PIM_calculate_prices_consumptions(
                price_EEZ_nd, price_DWFN_nd, catch_step_artis_traded_allctry_nd,
                catch_step_indus_allctry_nd, catch_step_artis_local_allctry_nd,
                price_artisanal_traded_allctry_nd, consumption_artisanal_traded_allctry_nd,
                price_industrial_allctry_nd, consumption_industrial_allctry_nd,
                price_artisanal_local_allctry_nd, consumption_artisanal_local_allctry_nd,
                exported_frac_nd, catch_step_EEZ_nd, catch_step_DWFN_nd,
                eezPosInCountryCoord, geo_agreg_artisanal_traded_nd,
                geo_agreg_industrial_nd, geo_agreg_artisanal_local_nd, nctry, nregion,
                nsubregion, region_idx, subregion_idx,
                driv_X_f, driv_W_f, driv_X_c,
                driv_W_c, epsilon, inter_prod_aq, EPS,
                t, dt, commod);

            writeBack<3>(price_EEZ, price_EEZ_nd);
            writeBack<3>(price_DWFN, price_DWFN_nd);
            writeBack<2>(catch_step_artis_traded_allctry, catch_step_artis_traded_allctry_nd);
            writeBack<2>(catch_step_indus_allctry, catch_step_indus_allctry_nd);
            writeBack<2>(catch_step_artis_local_allctry, catch_step_artis_local_allctry_nd);
            writeBack<2>(price_artisanal_traded_allctry, price_artisanal_traded_allctry_nd);
            writeBack<2>(consumption_artisanal_traded_allctry, consumption_artisanal_traded_allctry_nd);
            writeBack<2>(price_industrial_allctry, price_industrial_allctry_nd);
            writeBack<2>(consumption_industrial_allctry, consumption_industrial_allctry_nd);
            writeBack<2>(price_artisanal_local_allctry, price_artisanal_local_allctry_nd);
            writeBack<2>(consumption_artisanal_local_allctry, consumption_artisanal_local_allctry_nd);

            return result;
        },
        py::arg("price_EEZ"), py::arg("price_DWFN"), py::arg("catch_step_artis_traded_allctry"),
        py::arg("catch_step_indus_allctry"), py::arg("catch_step_artis_local_allctry"),
        py::arg("price_artisanal_traded_allctry"), py::arg("consumption_artisanal_traded_allctry"),
        py::arg("price_industrial_allctry"), py::arg("consumption_industrial_allctry"),
        py::arg("price_artisanal_local_allctry"), py::arg("consumption_artisanal_local_allctry"),
        py::arg("exported_frac"), py::arg("catch_step_EEZ"), py::arg("catch_step_DWFN"),
        py::arg("eezPosInCountryCoord"), py::arg("geo_agreg_artisanal_traded"),
        py::arg("geo_agreg_industrial"), py::arg("geo_agreg_artisanal_local"),
        py::arg("nctry"), py::arg("nregion"), py::arg("nsubregion"),
        py::arg("region_idx"), py::arg("subregion_idx"),
        py::arg("driv_X_f"), py::arg("driv_W_f"), py::arg("driv_X_c"), py::arg("driv_W_c"),
        py::arg("epsilon"), py::arg("inter_prod_aq"), py::arg("EPS"),
        py::arg("t"), py::arg("dt"), py::arg("commod"),
        "Compute prices/consumptions at step t, mutating the *_EEZ/_DWFN/_allctry arrays in place");

    m.def("mini_model_2_calc_aqua", &fishmip::mini_model_2_calc_aqua,
        py::arg("price"), py::arg("Aprod"), py::arg("d_U"), py::arg("d_V"), py::arg("d_t"),
        "Compute aquaculture production update");

    m.def("plugin_model_2_calc_effort",
        [](int nfleet_type, int nfisheries,
           py::array_t<double, py::array::c_style | py::array::forcecast> price,
           py::array_t<double, py::array::c_style | py::array::forcecast> Y,
           py::array_t<double, py::array::c_style | py::array::forcecast> effo,
           py::array_t<double, py::array::c_style | py::array::forcecast> q,
           const std::vector<double> &Ftarget,
           py::array_t<double, py::array::c_style | py::array::forcecast> comply,
           double risk,
           py::array_t<double, py::array::c_style | py::array::forcecast> d_U,
           py::array_t<double, py::array::c_style | py::array::forcecast> d_V,
           const std::vector<double> &M, double d_t, int effort_growth_lim,
           double f_growth_rate_max, double f_growth_rate_half_sat) {
            NdArray2 price_nd = toNdArray<2>(price);
            NdArray2 Y_nd = toNdArray<2>(Y);
            NdArray2 effo_nd = toNdArray<2>(effo);
            NdArray2 q_nd = toNdArray<2>(q);
            NdArray2 comply_nd = toNdArray<2>(comply);
            NdArray2 d_U_nd = toNdArray<2>(d_U);
            NdArray2 d_V_nd = toNdArray<2>(d_V);
            return fishmip::plugin_model_2_calc_effort(nfleet_type, nfisheries, price_nd, Y_nd, effo_nd, q_nd,
                                                         Ftarget, comply_nd, risk, d_U_nd, d_V_nd, M, d_t,
                                                         effort_growth_lim, f_growth_rate_max, f_growth_rate_half_sat);
        },
        py::arg("nfleet_type"), py::arg("nfisheries"), py::arg("price"), py::arg("Y"), py::arg("effo"),
        py::arg("q"), py::arg("Ftarget"), py::arg("comply"), py::arg("risk"), py::arg("d_U"), py::arg("d_V"),
        py::arg("M"), py::arg("d_t"), py::arg("effort_growth_lim"), py::arg("f_growth_rate_max"),
        py::arg("f_growth_rate_half_sat"),
        "Compute updated fishing effort/mortality per fleet");

    m.def("dnorm_log", &fishmip::dnorm_log, py::arg("obs"), py::arg("pred"), py::arg("sd"), "Log-density of a normal distribution");

    m.def("dynamics_loop",
        [](std::vector<double> B, double r, double k, double z, const std::vector<double> &q,
           py::array_t<double, py::array::c_style | py::array::forcecast> E, int n, int nf) {
            NdArray2 E_nd = toNdArray<2>(E);
            return fishmip::dynamics_loop(std::move(B), r, k, z, q, E_nd, n, nf);
        },
        py::arg("B"), py::arg("r"), py::arg("k"), py::arg("z"), py::arg("q"), py::arg("E"), py::arg("n"), py::arg("nf"),
        "Run the Pella-Tomlinson biomass dynamics loop");

    m.def("objective", &fishmip::objective, py::arg("log_params"), py::arg("data"),
        "Negative log-likelihood for the Pella-Tomlinson stock assessment");

    m.def("fit_one_stock", &fishmip::fit_one_stock,
        py::arg("data"), py::arg("mini"), py::arg("verbose"), py::arg("x0_prev"), py::arg("K0") = std::nullopt,
        "Fit the Pella-Tomlinson stock assessment model to one stock's catch/effort series");

    m.def("plugin_model_3_assess_stock",
        [](int i, std::size_t t, const std::vector<int> &nfleets_per_stock,
           py::array_t<double, py::array::c_style | py::array::forcecast> catch_for_stock_assess,
           py::array_t<double, py::array::c_style | py::array::forcecast> effort_for_stock_assess,
           const std::vector<double> &r_pop, double m, int nfleet, int length, int mini1, int mini2,
           bool verbose, const std::optional<std::vector<double>> &x0_prev) {
            NdArray3 catch_nd = toNdArray<3>(catch_for_stock_assess);
            NdArray3 effort_nd = toNdArray<3>(effort_for_stock_assess);
            return fishmip::plugin_model_3_assess_stock(i, t, nfleets_per_stock, catch_nd, effort_nd, r_pop, m,
                                                          nfleet, length, mini1, mini2, verbose, x0_prev);
        },
        py::arg("i"), py::arg("t"), py::arg("nfleets_per_stock"), py::arg("catch_for_stock_assess"),
        py::arg("effort_for_stock_assess"), py::arg("r_pop"), py::arg("m"), py::arg("nfleet"), py::arg("length"),
        py::arg("mini1"), py::arg("mini2"), py::arg("verbose"), py::arg("x0_prev"),
        "Fit a single stock's assessment model for the given year");

    m.def("PIM_assess_stocks",
        [](std::vector<double> Fmsy_est, std::vector<double> MSY_est, int parallel_mode, int nstocks,
           std::size_t t, std::size_t start_stock_assess, const std::vector<int> &nfleets_per_stock,
           py::array_t<double, py::array::c_style | py::array::forcecast> catch_for_stock_assess,
           py::array_t<double, py::array::c_style | py::array::forcecast> effort_for_stock_assess,
           const std::vector<double> &r_pop, double m, int nfleet, int length_stock_assess,
           bool verbose_stock_assessment, std::vector<std::optional<std::vector<double>>> x0_prev) {
            NdArray3 catch_nd = toNdArray<3>(catch_for_stock_assess);
            NdArray3 effort_nd = toNdArray<3>(effort_for_stock_assess);
            return fishmip::PIM_assess_stocks(std::move(Fmsy_est), std::move(MSY_est), parallel_mode, nstocks, t,
                                               start_stock_assess, nfleets_per_stock, catch_nd, effort_nd, r_pop, m,
                                               nfleet, length_stock_assess, verbose_stock_assessment, std::move(x0_prev));
        },
        py::arg("Fmsy_est"), py::arg("MSY_est"), py::arg("parallel_mode"), py::arg("nstocks"), py::arg("t"),
        py::arg("start_stock_assess"), py::arg("nfleets_per_stock"), py::arg("catch_for_stock_assess"),
        py::arg("effort_for_stock_assess"), py::arg("r_pop"), py::arg("m"), py::arg("nfleet"),
        py::arg("length_stock_assess"), py::arg("verbose_stock_assessment"), py::arg("x0_prev"),
        py::call_guard<py::gil_scoped_release>(),
        "Assess all stocks for the given year (releases the GIL: pure C++ numeric work, optionally multi-threaded)");

    m.def("PIM_allocate_target_local_DWFN",
        [](std::size_t t, int nEEZ, const std::vector<double> &Fmsy_est,
           py::array_t<double, py::array::c_style | py::array::forcecast> F_fleet_EEZ,
           py::array_t<double, py::array::c_style | py::array::forcecast> F_fleet_DWFN,
           py::array_t<double, py::array::c_style | py::array::forcecast> F_DWFN_allocate,
           py::array_t<double, py::array::c_style | py::array::forcecast> F_tot,
           const std::vector<double> &prop2target, const std::vector<double> &stock_surface) {
            NdArray3 F_fleet_EEZ_nd = toNdArray<3>(F_fleet_EEZ);
            NdArray3 F_fleet_DWFN_nd = toNdArray<3>(F_fleet_DWFN);
            NdArray4 F_DWFN_allocate_nd = toNdArray<4>(F_DWFN_allocate);
            NdArray2 F_tot_nd = toNdArray<2>(F_tot);
            return fishmip::PIM_allocate_target_local_DWFN(t, nEEZ, Fmsy_est, F_fleet_EEZ_nd, F_fleet_DWFN_nd,
                                                             F_DWFN_allocate_nd, F_tot_nd, prop2target, stock_surface);
        },
        py::arg("t"), py::arg("nEEZ"), py::arg("Fmsy_est"), py::arg("F_fleet_EEZ"), py::arg("F_fleet_DWFN"),
        py::arg("F_DWFN_allocate"), py::arg("F_tot"), py::arg("prop2target"), py::arg("stock_surface"),
        "Allocate the local/DWFN fishing targets across EEZ and DWFN stocks");
}

#endif
