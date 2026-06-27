#include "vectorkv/snapshot.h"
#include "vectorkv/types.h"
#include <fstream>
#include <iomanip>
#include <limits>
#include <sstream>
#include <string>
#include <unordered_map>

namespace vectorkv {

bool SnapShot::save(
    const std::string& path,
    const std::unordered_map<std::string, VectorRecord>& records
) {
    std::ofstream out(path);
    if (!out.is_open()) {
        return false;
    }
    out << std::setprecision(std::numeric_limits<float>::max_digits10);

    for (const auto& [id, record] : records) {
        if (record.deleted) {
            continue;
        }
        out << record.id << " " << record.vector.size();
        for (float v : record.vector) {
            out << " " << v;
        }
        for (const auto& [key, value] : record.metadata) {
            out << " " << key << "=" << value;
        }
        out << "\n";
    }
    
    out.flush();
    return out.good();
}

bool SnapShot::load(const std::string &path, 
    const std::function<void (const VectorRecord &)> &on_record
) {
    std::ifstream in(path);
    if (!in.is_open()) {
        return false;
    }

    std::string line;
    while (std::getline(in, line)) {
        std::istringstream iss(line);

        VectorRecord rec;
        size_t dim = 0;
        iss >> rec.id >> dim;

        rec.vector.resize(dim);
        for (size_t i = 0; i < dim; ++i) {
            iss >> rec.vector[i];
        }

        std::string token;
        while (iss >> token) {
            auto pos = token.find('=');
            if (pos != std::string::npos) {
                rec.metadata[token.substr(0, pos)] = token.substr(pos + 1);
            }
        }

        on_record(rec);
    }
    return true;
}

}