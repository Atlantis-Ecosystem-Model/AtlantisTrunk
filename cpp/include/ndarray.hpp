#pragma once
// Minimal row-major (C-order) dense N-dimensional array, standing in for the
// numpy.ndarray objects used throughout FishMIP_OSP_Plugin_Models.py.
//
// Eigen::MatrixXd/VectorXd are used elsewhere in this port for genuinely 1D/2D
// linear-algebra-flavoured data; this class covers the 3D/4D "numpy tensor"
// arrays (e.g. shape (nfleet, nEEZ, xsize)) for which Eigen's core module has
// no native equivalent.

#include <array>
#include <cstddef>
#include <numeric>
#include <stdexcept>
#include <vector>

template <std::size_t N>
class NdArray {
public:
    NdArray() { shape_.fill(0); }

    explicit NdArray(std::array<std::size_t, N> shape, double fill_value = 0.0)
        : shape_(shape), data_(computeSize(shape), fill_value) {}

    static std::size_t computeSize(const std::array<std::size_t, N>& shape) {
        std::size_t s = 1;
        for (auto d : shape) {
            s *= d;
        }
        return s;
    }

    const std::array<std::size_t, N>& shape() const { return shape_; }
    std::size_t extent(std::size_t axis) const { return shape_[axis]; }
    std::size_t size() const { return data_.size(); }

    double* data() { return data_.data(); }
    const double* data() const { return data_.data(); }

    void fill(double v) { std::fill(data_.begin(), data_.end(), v); }

    template <typename... Idx>
    std::size_t flatIndex(Idx... idx) const {
        static_assert(sizeof...(Idx) == N, "NdArray: wrong number of indices");
        std::size_t indices[N] = {static_cast<std::size_t>(idx)...};
        std::size_t flat = 0;
        for (std::size_t d = 0; d < N; ++d) flat = flat * shape_[d] + indices[d];
        return flat;
    }

    template <typename... Idx>
    double& operator()(Idx... idx) {
        return data_[flatIndex(idx...)];
    }

    template <typename... Idx>
    double operator()(Idx... idx) const {
        return data_[flatIndex(idx...)];
    }

    std::vector<double>& raw() { return data_; }
    const std::vector<double>& raw() const { return data_; }

private:
    std::array<std::size_t, N> shape_{};
    std::vector<double> data_;
};

using NdArray1 = NdArray<1>;
using NdArray2 = NdArray<2>;
using NdArray3 = NdArray<3>;
using NdArray4 = NdArray<4>;
