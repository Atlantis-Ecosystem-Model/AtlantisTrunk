#pragma once
// Small CSV reader replacing the pandas.read_csv() calls in PIM_load_drivers().
// Handles the two delimiter styles used by the driver CSVs (',' for
// parameters.csv, ';' for the IPBES/EEZ tables), a UTF-8 BOM on the first
// field (seen in EEZ_countries_WORK.csv), and simple double-quoted fields.

#include <string>
#include <unordered_map>
#include <vector>

class CsvTable {
public:
    std::vector<std::string> headers;
    std::vector<std::vector<std::string>> rows;  // rows[r][c]

    std::size_t nrows() const { return rows.size(); }

    int colIndex(const std::string& name) const {
        auto it = index_.find(name);
        return it == index_.end() ? -1 : it->second;
    }

    std::vector<std::string> columnStrings(const std::string& name) const;
    std::vector<double> columnDoubles(const std::string& name) const;  // blank -> NaN

    std::unordered_map<std::string, int> index_;
};

// Reads a delimited text file. `delimiter` is the field separator (',' or ';').
CsvTable readCsv(const std::string& path, char delimiter = ',');

// Reads a "long" name/value parameters CSV (one row per scalar parameter,
// e.g. `name,value,notes`) into a lookup map. Used for small hand-edited
// control-parameter tables where a wide one-row-per-run CSV would be harder
// to read/diff than a row-per-parameter one.
std::unordered_map<std::string, double> readKeyValueCsv(const std::string& path,
                                                          const std::string& keyColumn = "name",
                                                          const std::string& valueColumn = "value");

// Looks `key` up in a key/value map (as returned by readKeyValueCsv), throwing
// std::runtime_error with a helpful message (including `path`, for
// diagnostics) if it's missing.
double requireParam(const std::unordered_map<std::string, double>& params, const std::string& key,
                     const std::string& path);
