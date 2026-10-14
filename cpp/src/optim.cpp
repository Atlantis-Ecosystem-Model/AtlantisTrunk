#include "optim.hpp"

#include <algorithm>
#include <cmath>
#include <deque>
#include <limits>
#include <numeric>
#include <random>

namespace optim {

    namespace {

        std::vector<double> vsub(const std::vector<double> &a, const std::vector<double> &b) {
            std::vector<double> r(a.size());
            for (std::size_t i = 0; i < a.size(); ++i)
                r[i] = a[i] - b[i];
            return r;
        }
        std::vector<double> vadd(const std::vector<double> &a, const std::vector<double> &b) {
            std::vector<double> r(a.size());
            for (std::size_t i = 0; i < a.size(); ++i)
                r[i] = a[i] + b[i];
            return r;
        }
        std::vector<double> vscale(const std::vector<double> &a, double s) {
            std::vector<double> r(a.size());
            for (std::size_t i = 0; i < a.size(); ++i)
                r[i] = a[i] * s;
            return r;
        }
        double vdot(const std::vector<double> &a, const std::vector<double> &b) {
            double s = 0.0;
            for (std::size_t i = 0; i < a.size(); ++i)
                s += a[i] * b[i];
            return s;
        }
        double vnorm(const std::vector<double> &a) { return std::sqrt(vdot(a, a)); }

    } // namespace

    std::vector<double> clampToBounds(const std::vector<double> &x, const std::vector<Bound> &bounds) {
        std::vector<double> r = x;
        for (std::size_t i = 0; i < r.size(); ++i) {
            if (r[i] < bounds[i].lo)
                r[i] = bounds[i].lo;
            if (r[i] > bounds[i].hi)
                r[i] = bounds[i].hi;
        }
        return r;
    }

    // ================================================================ Nelder-Mead
    OptResult nelderMead(const ObjectiveFn &f, const std::vector<double> &x0,
                         const NelderMeadOptions &opts) {
        const std::size_t n = x0.size();
        const double alpha = 1.0, gamma = 2.0, rho = 0.5, sigma = 0.5;

        std::vector<std::vector<double>> sim(n + 1, x0);
        for (std::size_t i = 0; i < n; ++i) {
            double step = (x0[i] != 0.0) ? 0.05 * x0[i] : 0.00025;
            sim[i + 1][i] += step;
        }
        std::vector<double> fval(n + 1);
        for (std::size_t i = 0; i <= n; ++i)
            fval[i] = f(sim[i]);

        std::vector<std::size_t> order(n + 1);
        std::iota(order.begin(), order.end(), 0);

        int nit = 0;
        bool converged = false;
        for (; nit < opts.maxiter; ++nit) {
            std::sort(order.begin(), order.end(),
                      [&](std::size_t a, std::size_t b) { return fval[a] < fval[b]; });
            {
                std::vector<std::vector<double>> sortedSim(n + 1);
                std::vector<double> sortedF(n + 1);
                for (std::size_t i = 0; i <= n; ++i) {
                    sortedSim[i] = sim[order[i]];
                    sortedF[i] = fval[order[i]];
                }
                sim = sortedSim;
                fval = sortedF;
            }

            double fspread = 0.0;
            for (std::size_t i = 1; i <= n; ++i)
                fspread = std::max(fspread, std::abs(fval[i] - fval[0]));
            double xspread = 0.0;
            for (std::size_t i = 1; i <= n; ++i)
                for (std::size_t k = 0; k < n; ++k)
                    xspread = std::max(xspread, std::abs(sim[i][k] - sim[0][k]));
            if (fspread <= opts.fatol && xspread <= opts.xatol) {
                converged = true;
                break;
            }

            // Centroid of all points except the worst.
            std::vector<double> centroid(n, 0.0);
            for (std::size_t i = 0; i < n; ++i)
                centroid = vadd(centroid, sim[i]);
            centroid = vscale(centroid, 1.0 / static_cast<double>(n));

            const std::vector<double> &worst = sim[n];
            std::vector<double> xr = vadd(centroid, vscale(vsub(centroid, worst), alpha));
            double fr = f(xr);

            if (fval[0] <= fr && fr < fval[n - 1]) {
                sim[n] = xr;
                fval[n] = fr;
            } else if (fr < fval[0]) {
                std::vector<double> xe = vadd(centroid, vscale(vsub(xr, centroid), gamma));
                double fe = f(xe);
                if (fe < fr) {
                    sim[n] = xe;
                    fval[n] = fe;
                } else {
                    sim[n] = xr;
                    fval[n] = fr;
                }
            } else {
                std::vector<double> xc;
                double fc;
                if (fr < fval[n]) {
                    xc = vadd(centroid, vscale(vsub(xr, centroid), rho));
                } else {
                    xc = vadd(centroid, vscale(vsub(worst, centroid), rho));
                }
                fc = f(xc);
                if (fc < std::min(fr, fval[n])) {
                    sim[n] = xc;
                    fval[n] = fc;
                } else {
                    for (std::size_t i = 1; i <= n; ++i) {
                        sim[i] = vadd(sim[0], vscale(vsub(sim[i], sim[0]), sigma));
                        fval[i] = f(sim[i]);
                    }
                }
            }
            if (opts.disp && nit % 100 == 0) { /* progress hook left intentionally empty */
            }
        }

        std::sort(order.begin(), order.end(),
                  [&](std::size_t a, std::size_t b) { return fval[a] < fval[b]; });

        OptResult res;
        res.x = sim[order[0]];
        res.fun = fval[order[0]];
        res.success = converged;
        res.nit = nit;
        res.message = converged ? "converged" : "maximum iterations reached";
        return res;
    }

    // ===================================================== Differential evolution
    namespace {

        std::vector<std::vector<double>> latinHypercubeInit(std::size_t npop, const std::vector<Bound> &bounds,
                                                            std::mt19937 &rng) {
            const std::size_t ndim = bounds.size();
            std::vector<std::vector<double>> pop(npop, std::vector<double>(ndim));
            std::uniform_real_distribution<double> unit(0.0, 1.0);
            for (std::size_t d = 0; d < ndim; ++d) {
                std::vector<std::size_t> perm(npop);
                std::iota(perm.begin(), perm.end(), 0);
                std::shuffle(perm.begin(), perm.end(), rng);
                for (std::size_t i = 0; i < npop; ++i) {
                    double stratified = (static_cast<double>(perm[i]) + unit(rng)) / static_cast<double>(npop);
                    pop[i][d] = bounds[d].lo + stratified * (bounds[d].hi - bounds[d].lo);
                }
            }
            return pop;
        }

    } // namespace

    OptResult differentialEvolution(const ObjectiveFn &f, const std::vector<Bound> &bounds,
                                    const DifferentialEvolutionOptions &opts) {
        const std::size_t ndim = bounds.size();
        const std::size_t npop = std::max<std::size_t>(4, static_cast<std::size_t>(opts.popsize) * ndim);
        const double CR = 0.7; // SciPy default recombination
        std::mt19937 rng(opts.seed);
        std::uniform_real_distribution<double> unit(0.0, 1.0);
        std::uniform_real_distribution<double> fDither(0.5, 1.0); // SciPy default mutation=(0.5, 1)

        std::vector<std::vector<double>> pop;
        if (opts.latinHypercubeInit) {
            pop = latinHypercubeInit(npop, bounds, rng);
        } else {
            pop.assign(npop, std::vector<double>(ndim));
            for (auto &ind : pop)
                for (std::size_t d = 0; d < ndim; ++d)
                    ind[d] = bounds[d].lo + unit(rng) * (bounds[d].hi - bounds[d].lo);
        }
        std::vector<double> fitness(npop);
        for (std::size_t i = 0; i < npop; ++i)
            fitness[i] = f(pop[i]);

        auto bestIdx = [&]() {
            return static_cast<std::size_t>(std::min_element(fitness.begin(), fitness.end()) - fitness.begin());
        };

        int nit = 0;
        for (; nit < opts.maxiter; ++nit) {
            std::size_t ibest = bestIdx();
            std::vector<std::vector<double>> newPop = pop;
            std::vector<double> newFitness = fitness;

            for (std::size_t i = 0; i < npop; ++i) {
                std::size_t r1, r2;
                do {
                    r1 = static_cast<std::size_t>(unit(rng) * npop) % npop;
                } while (r1 == i || r1 == ibest);
                do {
                    r2 = static_cast<std::size_t>(unit(rng) * npop) % npop;
                } while (r2 == i || r2 == ibest || r2 == r1);

                double F = fDither(rng);
                std::vector<double> donor(ndim);
                for (std::size_t d = 0; d < ndim; ++d)
                    donor[d] = pop[ibest][d] + F * (pop[r1][d] - pop[r2][d]);

                std::vector<double> trial = pop[i];
                std::size_t jrand = static_cast<std::size_t>(unit(rng) * ndim) % ndim;
                for (std::size_t d = 0; d < ndim; ++d) {
                    if (unit(rng) < CR || d == jrand)
                        trial[d] = donor[d];
                }
                trial = clampToBounds(trial, bounds);

                double ftrial = f(trial);
                if (ftrial <= fitness[i]) {
                    if (opts.deferredUpdating) {
                        newPop[i] = trial;
                        newFitness[i] = ftrial;
                    } else {
                        pop[i] = trial;
                        fitness[i] = ftrial;
                        if (ftrial < fitness[ibest])
                            ibest = i;
                    }
                }
            }
            if (opts.deferredUpdating) {
                pop = newPop;
                fitness = newFitness;
            }

            double mean = std::accumulate(fitness.begin(), fitness.end(), 0.0) / static_cast<double>(npop);
            double var = 0.0;
            for (double v : fitness)
                var += (v - mean) * (v - mean);
            double sd = std::sqrt(var / static_cast<double>(npop));
            if (sd <= opts.tol * std::abs(mean))
                break;
        }

        std::size_t ibest = bestIdx();
        OptResult res;
        res.x = pop[ibest];
        res.fun = fitness[ibest];
        res.success = true;
        res.nit = nit;
        res.message = "differential evolution finished";

        if (opts.polish) {
            LBFGSBOptions lopts;
            lopts.maxiter = 1000;
            lopts.ftol = 1e-6;
            OptResult polished = boundedLBFGS(f, res.x, bounds, lopts);
            if (polished.fun <= res.fun) {
                res.x = polished.x;
                res.fun = polished.fun;
            }
        }
        return res;
    }

    // ================================================================ L-BFGS-B
    namespace {

        std::vector<double> finiteDiffGradient(const ObjectiveFn &f, const std::vector<double> &x,
                                               const std::vector<Bound> &bounds, double relStep) {
            std::vector<double> grad(x.size());
            std::vector<double> xp = x, xm = x;
            for (std::size_t i = 0; i < x.size(); ++i) {
                double h = relStep * std::max(1.0, std::abs(x[i]));
                double hi = std::min(bounds[i].hi - x[i], h);
                double lo = std::min(x[i] - bounds[i].lo, h);
                if (hi <= 0.0 && lo <= 0.0) {
                    grad[i] = 0.0;
                    continue;
                }
                if (hi <= 0.0) {
                    // backward difference only
                    xm[i] = x[i] - lo;
                    double f0 = f(x);
                    double fm = f(xm);
                    grad[i] = (f0 - fm) / lo;
                    xm[i] = x[i];
                } else if (lo <= 0.0) {
                    xp[i] = x[i] + hi;
                    double f0 = f(x);
                    double fp = f(xp);
                    grad[i] = (fp - f0) / hi;
                    xp[i] = x[i];
                } else {
                    xp[i] = x[i] + hi;
                    xm[i] = x[i] - lo;
                    double fp = f(xp);
                    double fm = f(xm);
                    grad[i] = (fp - fm) / (hi + lo);
                    xp[i] = x[i];
                    xm[i] = x[i];
                }
            }
            return grad;
        }

        std::vector<double> projectDirection(const std::vector<double> &x, const std::vector<double> &grad,
                                             const std::vector<double> &dir, const std::vector<Bound> &bounds) {
            std::vector<double> d = dir;
            const double eps = 1e-12;
            for (std::size_t i = 0; i < x.size(); ++i) {
                bool atLower = x[i] <= bounds[i].lo + eps;
                bool atUpper = x[i] >= bounds[i].hi - eps;
                if ((atLower && grad[i] > 0.0) || (atUpper && grad[i] < 0.0))
                    d[i] = 0.0;
            }
            return d;
        }

    } // namespace

    OptResult boundedLBFGS(const ObjectiveFn &f, const std::vector<double> &x0,
                           const std::vector<Bound> &bounds, const LBFGSBOptions &opts) {
        std::vector<double> x = clampToBounds(x0, bounds);
        double fx = f(x);
        std::vector<double> grad = finiteDiffGradient(f, x, bounds, opts.fdStep);

        std::deque<std::vector<double>> sList, yList;
        std::deque<double> rhoList;

        bool converged = false;
        int iter = 0;
        for (; iter < opts.maxiter; ++iter) {
            // Two-loop recursion for the L-BFGS search direction.
            std::vector<double> q = grad;
            std::vector<double> alphaHist(sList.size());
            for (int i = static_cast<int>(sList.size()) - 1; i >= 0; --i) {
                double alpha_i = rhoList[i] * vdot(sList[i], q);
                alphaHist[i] = alpha_i;
                q = vsub(q, vscale(yList[i], alpha_i));
            }
            double gamma = 1.0;
            if (!sList.empty()) {
                const auto &s = sList.back();
                const auto &y = yList.back();
                double ys = vdot(y, s), yy = vdot(y, y);
                if (yy > 1e-16)
                    gamma = ys / yy;
            }
            std::vector<double> z = vscale(q, gamma);
            for (std::size_t i = 0; i < sList.size(); ++i) {
                double beta = rhoList[i] * vdot(yList[i], z);
                z = vadd(z, vscale(sList[i], alphaHist[i] - beta));
            }
            std::vector<double> dir = vscale(z, -1.0);
            dir = projectDirection(x, grad, dir, bounds);

            double dg = vdot(dir, grad);
            if (dg >= 0.0) {
                // Not a descent direction (can happen after projection): fall back to
                // projected steepest descent.
                dir = projectDirection(x, grad, vscale(grad, -1.0), bounds);
                dg = vdot(dir, grad);
            }
            if (vnorm(dir) < 1e-14) {
                converged = true;
                break;
            }

            // Backtracking line search (Armijo), then clip to bounds.
            double step = 1.0;
            const double c1 = 1e-4;
            std::vector<double> xNew;
            double fNew = fx;
            bool improved = false;
            for (int ls = 0; ls < 40; ++ls) {
                xNew = clampToBounds(vadd(x, vscale(dir, step)), bounds);
                fNew = f(xNew);
                if (fNew <= fx + c1 * step * dg) {
                    improved = true;
                    break;
                }
                step *= 0.5;
            }
            if (!improved) {
                converged = false;
                break;
            }

            std::vector<double> gradNew = finiteDiffGradient(f, xNew, bounds, opts.fdStep);
            std::vector<double> s = vsub(xNew, x);
            std::vector<double> y = vsub(gradNew, grad);
            double sy = vdot(s, y);
            if (sy > 1e-10) {
                if (static_cast<int>(sList.size()) >= opts.memory) {
                    sList.pop_front();
                    yList.pop_front();
                    rhoList.pop_front();
                }
                sList.push_back(s);
                yList.push_back(y);
                rhoList.push_back(1.0 / sy);
            }

            double relChange = std::abs(fNew - fx) / std::max({1.0, std::abs(fx), std::abs(fNew)});
            x = xNew;
            grad = gradNew;
            double fOld = fx;
            fx = fNew;
            if (relChange < opts.ftol) {
                converged = true;
                (void)fOld;
                ++iter;
                break;
            }
        }

        OptResult res;
        res.x = x;
        res.fun = fx;
        res.success = converged;
        res.nit = iter;
        res.message = converged ? "converged" : "maximum iterations reached / line search failed";
        return res;
    }

} // namespace optim
