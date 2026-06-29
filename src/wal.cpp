#include "vectorkv/wal.h"
#include "vectorkv/types.h"
#include <cstddef>
#include <fstream>
#include <iomanip>
#include <limits>
#include <sstream>

namespace vectorkv {

WAL::WAL(const std::string& path) : path_(path), out_(path, std::ios::app) {
    out_ << std::setprecision(std::numeric_limits<float>::max_digits10);
}

bool WAL::append_insert(const VectorRecord& record) {
    if (!out_.is_open()) {
        return false;
    }
    out_ << "INSERT"<< " " << record.id << " " << record.vector.size();
    for (float v : record.vector) {
        out_ << " " << v;
    }
    for (const auto& [key, value] : record.metadata) {
        out_ << " " << key << "=" << value;
    }
    out_ << "\n";
    
    out_.flush();
    return out_.good();
}

bool WAL::append_delete(const std::string& id) {
    if (!out_.is_open()) {
        return false;
    }
    out_ << "DELETE" << " " << id << "\n";
    out_.flush();
    return out_.good();
}

bool WAL::truncate() {
    out_.close();
    {
        std::ofstream clear_file(path_, std::ios::trunc);
        if (!clear_file.is_open()) {
            return false;
        }
    }
    out_.open(path_, std::ios::app);
    if (!out_.is_open()) {
        return false;
    }
    out_ << std::setprecision(std::numeric_limits<float>::max_digits10);
    return out_.good();
}

void WAL::replay(
    const std::function<void(const VectorRecord&)>& on_insert,
    const std::function<void(const std::string&)>& on_delete
) {
    std::ifstream in(path_);
    if (!in.is_open()) {
        return;
    }

    std::string line;
    while (std::getline(in, line)) {
        std::istringstream iss(line);
        std::string op;
        iss >> op;

        if (op == "INSERT") {
            VectorRecord rec;
            size_t dim = 0;
            iss >> rec.id >> dim;

            rec.vector.resize(dim);
            for (size_t i = 0; i < dim; ++i) {
                iss >> rec.vector[i];
            }

            // metadata
            std::string token;
            while (iss >> token) {
                auto pos = token.find('=');
                if (pos != std::string::npos) {
                    rec.metadata[token.substr(0, pos)] = token.substr(pos + 1);
                }
            }

            on_insert(rec);
        } else if (op == "DELETE") {
            std::string id;
            iss >> id;
            on_delete(id);
        }
    }
}

}