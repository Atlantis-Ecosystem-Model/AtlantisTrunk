#pragma once
// Fixed-knot monotone cubic Hermite (PCHIP) interpolation, replacing
// scipy.interpolate.PchipInterpolator(x_points, y_row)(geo_agreg[i]) as used
// in plugin_model_1_calc_price(), which always interpolates over the four
// fixed knots x = [0, 1, 2, 3] (national / subregional / regional / global
// market price estimates).
//
// The derivative estimates and end-condition adjustments below follow the
// Fritsch-Carlson method used internally by SciPy's PchipInterpolator,
// specialised to unit knot spacing (h = 1) since x_points is always
// [0, 1, 2, 3] here.

#include <array>
#include <cmath>

inline int signOf(double v) {
    if (v > 0.0) return 1;
    if (v < 0.0) return -1;
    return 0;
}

// Evaluate the PCHIP interpolant built from y = [y(0), y(1), y(2), y(3)] at xq.
inline double pchip4_eval(const std::array<double, 4>& y, double xq) {
    const double delta0 = y[1] - y[0];
    const double delta1 = y[2] - y[1];
    const double delta2 = y[3] - y[2];

    std::array<double, 4> d{};

    // Interior derivatives (unit spacing harmonic-mean weighting).
    if (signOf(delta0) != signOf(delta1) || delta0 == 0.0 || delta1 == 0.0) {
        d[1] = 0.0;
    } else {
        d[1] = 2.0 / (1.0 / delta0 + 1.0 / delta1);
    }
    if (signOf(delta1) != signOf(delta2) || delta1 == 0.0 || delta2 == 0.0) {
        d[2] = 0.0;
    } else {
        d[2] = 2.0 / (1.0 / delta1 + 1.0 / delta2);
    }

    // Left end condition (h0 = h1 = 1).
    {
        double d0 = (3.0 * delta0 - delta1) / 2.0;
        if (signOf(d0) != signOf(delta0)) {
            d0 = 0.0;
        } else if (signOf(delta0) != signOf(delta1) && std::abs(d0) > 3.0 * std::abs(delta0)) {
            d0 = 3.0 * delta0;
        }
        d[0] = d0;
    }
    // Right end condition (mirrored).
    {
        double d3 = (3.0 * delta2 - delta1) / 2.0;
        if (signOf(d3) != signOf(delta2)) {
            d3 = 0.0;
        } else if (signOf(delta2) != signOf(delta1) && std::abs(d3) > 3.0 * std::abs(delta2)) {
            d3 = 3.0 * delta2;
        }
        d[3] = d3;
    }

    // Pick the interval [i, i+1] containing xq (clamped to [0, 3]).
    double xc = xq;
    if (xc < 0.0) xc = 0.0;
    if (xc > 3.0) xc = 3.0;
    int i = static_cast<int>(xc);
    if (i > 2) i = 2;
    const double t = xc - static_cast<double>(i);

    const double t2 = t * t;
    const double t3 = t2 * t;
    const double h00 = 2.0 * t3 - 3.0 * t2 + 1.0;
    const double h10 = t3 - 2.0 * t2 + t;
    const double h01 = -2.0 * t3 + 3.0 * t2;
    const double h11 = t3 - t2;

    return h00 * y[i] + h10 * d[i] + h01 * y[i + 1] + h11 * d[i + 1];
}
