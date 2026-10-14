#include "csv_utils.hpp"

#include <cmath>
#include <fstream>
#include <limits>
#include <sstream>
#include <stdexcept>

namespace {

    std::vector<std::string> splitLine(const std::string &line, char delimiter) {
        std::vector<std::string> fields;
        std::string cur;
        bool inQuotes = false;
        for (std::size_t i = 0; i < line.size(); ++i) {
            char c = line[i];
            if (inQuotes) {
                if (c == '"') {
                    if (i + 1 < line.size() && line[i + 1] == '"') {
                        cur += '"';
                        ++i;
                    } else {
                        inQuotes = false;
                    }
                } else {
                    cur += c;
                }
            } else {
                if (c == '"') {
                    inQuotes = true;
                } else if (c == delimiter) {
                    fields.push_back(cur);
                    cur.clear();
                } else {
                    cur += c;
                }
            }
        }
        fields.push_back(cur);
        return fields;
    }

    std::string stripCR(std::string s) {
        while (!s.empty() && (s.back() == '\r' || s.back() == '\n'))
            s.pop_back();
        return s;
    }

    void stripBom(std::string &s) {
        static const std::string bom = "\xEF\xBB\xBF";
        if (s.size() >= 3 && s.compare(0, 3, bom) == 0)
            s.erase(0, 3);
    }

} // namespace

CsvTable readCsv(const std::string &path, char delimiter) {
    std::ifstream in(path);
    if (!in.is_open()) {
        throw std::runtime_error("readCsv: could not open " + path);
    }

    CsvTable table;
    std::string line;
    bool first = true;
    while (std::getline(in, line)) {
        line = stripCR(line);
        if (first) {
            stripBom(line);
        }
        if (line.empty())
            continue;
        std::vector<std::string> fields = splitLine(line, delimiter);
        if (first) {
            table.headers = fields;
            for (std::size_t i = 0; i < fields.size(); ++i)
                table.index_[fields[i]] = static_cast<int>(i);
            first = false;
        } else {
            table.rows.push_back(fields);
        }
    }
    return table;
}

std::vector<std::string> CsvTable::columnStrings(const std::string &name) const {
    int idx = colIndex(name);
    if (idx < 0)
        throw std::runtime_error("CsvTable: column not found: " + name);
    std::vector<std::string> out(rows.size());
    for (std::size_t r = 0; r < rows.size(); ++r) {
        out[r] = static_cast<std::size_t>(idx) < rows[r].size() ? rows[r][idx] : std::string();
    }
    return out;
}

std::vector<double> CsvTable::columnDoubles(const std::string &name) const {
    int idx = colIndex(name);
    if (idx < 0)
        throw std::runtime_error("CsvTable: column not found: " + name);
    std::vector<double> out(rows.size());
    for (std::size_t r = 0; r < rows.size(); ++r) {
        const std::string &v = static_cast<std::size_t>(idx) < rows[r].size() ? rows[r][idx] : std::string();
        if (v.empty()) {
            out[r] = std::numeric_limits<double>::quiet_NaN();
        } else {
            try {
                out[r] = std::stod(v);
            } catch (const std::exception &) {
                out[r] = std::numeric_limits<double>::quiet_NaN();
            }
        }
    }
    return out;
}

std::unordered_map<std::string, double> readKeyValueCsv(const std::string &path, const std::string &keyColumn,
                                                        const std::string &valueColumn) {
    CsvTable table = readCsv(path, ',');
    std::vector<std::string> keys = table.columnStrings(keyColumn);
    std::vector<double> values = table.columnDoubles(valueColumn);

    std::unordered_map<std::string, double> out;
    out.reserve(keys.size());
    for (std::size_t i = 0; i < keys.size(); ++i)
        out[keys[i]] = values[i];
    return out;
}

double requireParam(const std::unordered_map<std::string, double> &params, const std::string &key,
                    const std::string &path) {
    auto it = params.find(key);
    if (it == params.end()) {
        throw std::runtime_error("readKeyValueCsv: missing parameter '" + key + "' in " + path);
    }
    return it->second;
}
