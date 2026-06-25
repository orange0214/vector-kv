#include "vectorkv/wal.h"
#include <iomanip>
#include <limits>

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

}