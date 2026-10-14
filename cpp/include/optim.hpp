#pragma once
// Hand-rolled replacements for the scipy.optimize routines used by
// fit_one_stock() in FishMIP_OSP_Plugin_Models.py: Nelder-Mead,
// differential_evolution, and L-BFGS-B.
//
// IMPORTANT — see PORTING_NOTES.md, section "Optimizers": these are faithful
// re-implementations of the *published algorithms* (Nelder-Mead 1965,
// Fritsch-Carlson-style DE/best1bin, and a projected-gradient L-BFGS), not
// bindings to SciPy/Fortran L-BFGS-B. They are not guaranteed to reach
// bit-identical optima to the Python version; see the notes for details and
// mitigations.

#include <functional>
#include <string>
#include <vector>

namespace optim {

using ObjectiveFn = std::function<double(const std::vector<double>&)>;

struct Bound {
    double lo;
    double hi;
};

// ---------------------------------------------------------------- Nelder-Mead
struct NelderMeadOptions {
    int maxiter = 50000;
    double xatol = 1e-7;
    double fatol = 1e-7;
    bool disp = false;
};

struct OptResult {
    std::vector<double> x;
    double fun = 0.0;
    bool success = false;
    int nit = 0;
    std::string message;
};

OptResult nelderMead(const ObjectiveFn& f, const std::vector<double>& x0,
                      const NelderMeadOptions& opts = {});

// ------------------------------------------------------- Differential evolution
struct DifferentialEvolutionOptions {
    int maxiter = 1000;
    double tol = 1e-6;
    unsigned seed = 42;
    int popsize = 15;         // population = popsize * ndim, as in SciPy
    bool polish = false;      // refine the best member with boundedLBFGS afterwards
    bool latinHypercubeInit = true;
    bool deferredUpdating = true; // true: generational ("deferred"); false: steady-state ("immediate")
    bool disp = false;
};

OptResult differentialEvolution(const ObjectiveFn& f, const std::vector<Bound>& bounds,
                                 const DifferentialEvolutionOptions& opts = {});

// ------------------------------------------------------------------ L-BFGS-B
struct LBFGSBOptions {
    int maxiter = 1000;
    double ftol = 1e-6;
    int memory = 10;           // number of (s, y) pairs retained
    double fdStep = 1e-6;      // relative step for central finite-difference gradients
};

OptResult boundedLBFGS(const ObjectiveFn& f, const std::vector<double>& x0,
                        const std::vector<Bound>& bounds, const LBFGSBOptions& opts = {});

std::vector<double> clampToBounds(const std::vector<double>& x, const std::vector<Bound>& bounds);

}  // namespace optim
