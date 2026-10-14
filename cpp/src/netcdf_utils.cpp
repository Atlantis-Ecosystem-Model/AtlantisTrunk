#include "netcdf_utils.hpp"

#include <netcdf.h>

#include <algorithm>
#include <stdexcept>
#include <unordered_map>

namespace {

    void checkNc(int status, const std::string &what) {
        if (status != NC_NOERR) {
            throw std::runtime_error("netCDF error (" + what + "): " + nc_strerror(status));
        }
    }

} // namespace

NcDataset::NcDataset(const std::string &path) {
    checkNc(nc_open(path.c_str(), NC_NOWRITE, &ncid_), "open " + path);
}

NcDataset::~NcDataset() { nc_close(ncid_); }

NcVariable NcDataset::readMainVariable() const {
    int ndims = 0;
    checkNc(nc_inq_ndims(ncid_, &ndims), "inq_ndims");
    std::vector<std::string> dimNames(static_cast<std::size_t>(ndims));
    for (int d = 0; d < ndims; ++d) {
        char name[NC_MAX_NAME + 1] = {0};
        checkNc(nc_inq_dimname(ncid_, d, name), "inq_dimname");
        dimNames[static_cast<std::size_t>(d)] = name;
    }

    int nvars = 0;
    checkNc(nc_inq_nvars(ncid_, &nvars), "inq_nvars");

    int mainVarId = -1;
    for (int v = 0; v < nvars; ++v) {
        char name[NC_MAX_NAME + 1] = {0};
        nc_type type;
        checkNc(nc_inq_var(ncid_, v, name, &type, nullptr, nullptr, nullptr), "inq_var");
        if (type != NC_DOUBLE)
            continue;
        bool isCoordLike = std::find(dimNames.begin(), dimNames.end(), std::string(name)) != dimNames.end();
        if (!isCoordLike) {
            if (mainVarId != -1) {
                throw std::runtime_error("NcDataset: more than one candidate data variable found");
            }
            mainVarId = v;
        }
    }
    if (mainVarId == -1)
        throw std::runtime_error("NcDataset: no floating-point data variable found");

    int vndims = 0;
    checkNc(nc_inq_varndims(ncid_, mainVarId, &vndims), "inq_varndims");
    std::vector<int> dimIds(static_cast<std::size_t>(vndims));
    checkNc(nc_inq_vardimid(ncid_, mainVarId, dimIds.data()), "inq_vardimid");

    NcVariable result;
    result.shape.resize(static_cast<std::size_t>(vndims));
    for (int d = 0; d < vndims; ++d) {
        std::size_t len = 0;
        checkNc(nc_inq_dimlen(ncid_, dimIds[static_cast<std::size_t>(d)], &len), "inq_dimlen");
        result.shape[static_cast<std::size_t>(d)] = len;
    }
    result.data.resize(result.size());
    checkNc(nc_get_var_double(ncid_, mainVarId, result.data.data()), "get_var_double");
    return result;
}

std::vector<std::string> NcDataset::readStringCoord(const std::string &name) const {
    int varid = 0;
    checkNc(nc_inq_varid(ncid_, name.c_str(), &varid), "inq_varid " + name);

    int vndims = 0;
    checkNc(nc_inq_varndims(ncid_, varid, &vndims), "inq_varndims " + name);
    if (vndims != 2)
        throw std::runtime_error("NcDataset: expected 2D char coord for " + name);
    std::vector<int> dimIds(2);
    checkNc(nc_inq_vardimid(ncid_, varid, dimIds.data()), "inq_vardimid " + name);
    std::size_t n = 0, strlen = 0;
    checkNc(nc_inq_dimlen(ncid_, dimIds[0], &n), "inq_dimlen " + name);
    checkNc(nc_inq_dimlen(ncid_, dimIds[1], &strlen), "inq_dimlen " + name);

    std::vector<char> buf(n * strlen);
    checkNc(nc_get_var_text(ncid_, varid, buf.data()), "get_var_text " + name);

    std::vector<std::string> out(n);
    for (std::size_t i = 0; i < n; ++i) {
        std::string s(buf.data() + i * strlen, strlen);
        while (!s.empty() && (s.back() == '\0' || s.back() == ' '))
            s.pop_back();
        out[i] = s;
    }
    return out;
}

NcVariable selectByLabelAxis(const NcVariable &src, std::size_t axis,
                             const std::vector<std::string> &axisLabels,
                             const std::vector<std::string> &wantedLabels) {
    if (src.shape[axis] != axisLabels.size()) {
        throw std::runtime_error("selectByLabelAxis: axis length does not match label list");
    }
    std::unordered_map<std::string, std::size_t> lookup;
    lookup.reserve(axisLabels.size());
    for (std::size_t i = 0; i < axisLabels.size(); ++i)
        lookup[axisLabels[i]] = i;

    std::vector<std::size_t> gatherIdx(wantedLabels.size());
    for (std::size_t i = 0; i < wantedLabels.size(); ++i) {
        auto it = lookup.find(wantedLabels[i]);
        if (it == lookup.end()) {
            throw std::runtime_error("selectByLabelAxis: label not found: " + wantedLabels[i]);
        }
        gatherIdx[i] = it->second;
    }

    NcVariable out;
    out.shape = src.shape;
    out.shape[axis] = wantedLabels.size();
    out.data.resize(out.size());

    const std::size_t ndims = src.shape.size();
    std::vector<std::size_t> srcStrides(ndims), outStrides(ndims);
    srcStrides[ndims - 1] = 1;
    outStrides[ndims - 1] = 1;
    for (std::size_t d = ndims - 1; d-- > 0;) {
        srcStrides[d] = srcStrides[d + 1] * src.shape[d + 1];
        outStrides[d] = outStrides[d + 1] * out.shape[d + 1];
    }

    std::vector<std::size_t> idx(ndims, 0);
    for (std::size_t flatOut = 0; flatOut < out.size(); ++flatOut) {
        std::size_t rem = flatOut;
        for (std::size_t d = 0; d < ndims; ++d) {
            idx[d] = rem / outStrides[d];
            rem %= outStrides[d];
        }
        std::size_t flatSrc = 0;
        for (std::size_t d = 0; d < ndims; ++d) {
            std::size_t srcIdxD = (d == axis) ? gatherIdx[idx[d]] : idx[d];
            flatSrc += srcIdxD * srcStrides[d];
        }
        out.data[flatOut] = src.data[flatSrc];
    }
    return out;
}
