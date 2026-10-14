
// #include "SimulationState.h"
#include <iostream>
#include "FishMIP_OSP_Plugin_Models.hpp"
#include <Rcpp.h>
using namespace Rcpp;
#define stdout_macro Rcpp::Rcout
#define stderr_macro Rcpp::Rcerr

//' Configure PIM Simulation
//'
//' @param nosp Species index
//' @return A list with simulation configuration parameters
//' @export
// [[Rcpp::export]]
Rcpp::List PIM_configure_simulation(int nosp) {
    fishmip::SimulationConfig cfg = fishmip::PIM_configure_simulation(nosp);
    return Rcpp::List::create(
        Rcpp::Named("nctry")      = cfg.nctry,
        Rcpp::Named("nsubregion") = cfg.nsubregion,
        Rcpp::Named("nregion")    = cfg.nregion,
        Rcpp::Named("nEEZ")       = cfg.nEEZ,
        Rcpp::Named("nfleet")     = cfg.nfleet,
        Rcpp::Named("nstocks")    = cfg.nstocks,
        Rcpp::Named("time_min")   = cfg.time_min,
        Rcpp::Named("time_max")   = cfg.time_max,
        Rcpp::Named("xsize")      = static_cast<int>(cfg.xsize),
        Rcpp::Named("EPS")        = cfg.EPS
    );
}