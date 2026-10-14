#pragma once
// Thin wrapper over the netCDF-C API replacing xarray.load_dataarray() /
// DataArray.sel() as used in PIM_load_drivers(). Every driver file here holds
// exactly one floating-point data variable plus a handful of coordinate
// variables (an "iso"/"country" ISO3 code axis, an integer "time" axis, and
// sometimes a "commodity" or "fleet type" label axis) — see PORTING_NOTES.md
// for the netCDF layouts this was written against (inspected with `ncdump -h`
// on the actual driver files).

#include <cstddef>
#include <string>
#include <vector>

// A variable read from a netCDF file: `shape` gives the extents in the order
// the dimensions were declared, and `data` is the flat row-major (C-order)
// buffer — the same memory layout numpy/xarray would hand back.
struct NcVariable {
    std::vector<std::size_t> shape;
    std::vector<double> data;

    std::size_t size() const {
        std::size_t s = 1;
        for (auto d : shape) s *= d;
        return s;
    }
};

class NcDataset {
public:
    explicit NcDataset(const std::string& path);
    ~NcDataset();
    NcDataset(const NcDataset&) = delete;
    NcDataset& operator=(const NcDataset&) = delete;

    // Reads the file's sole floating-point data variable (the one whose name
    // does not match any dimension name), equivalent to xr.load_dataarray().
    NcVariable readMainVariable() const;

    // Reads a fixed-width char(n, strlen) coordinate variable (e.g. "iso",
    // "country") into trimmed strings, equivalent to ds.coords[name].values.
    std::vector<std::string> readStringCoord(const std::string& name) const;

private:
    int ncid_;
};

// Gathers `src` (shape `shape`, row-major) along `axis`, reordering/selecting
// entries so that the labels on that axis end up in the order given by
// `wantedLabels`, using `axisLabels` (the coordinate values as currently laid
// out in `src`) to look each one up. Equivalent to DataArray.sel(<dim>=labels).
NcVariable selectByLabelAxis(const NcVariable& src, std::size_t axis,
                              const std::vector<std::string>& axisLabels,
                              const std::vector<std::string>& wantedLabels);
