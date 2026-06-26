#include "vectorkv/vector_db.h"

namespace vectorkv {

VectorDB::VectorDB(const std::string& wal_path) {
    wal_.emplace(wal_path);
    recover();
}

void VectorDB::recover() {
    if (!wal_) {
        return;
    }
    wal_->replay(
        [this](const VectorRecord& r){store_.put(r);},
        [this](const std::string& id){store_.remove(id);}    
    );
}

bool VectorDB::insert(
    const std::string& id,
    const std::vector<float>& vector,
    const std::unordered_map<std::string, std::string>& metadata
) {
    VectorRecord record;
    record.id = id;
    record.vector = vector;
    record.metadata = metadata;
    record.deleted = false;

    if (wal_) {
        wal_->append_insert(record);
    }
    return store_.put(record);
}

bool VectorDB::remove(const std::string& id) {
    if (wal_) {
        wal_->append_delete(id);
    }
    return store_.remove(id);
}

std::vector<SearchResult> VectorDB::search(
    const std::vector<float>& query,
    int top_k
) {
    return index_.search(store_, query, top_k);
}

}