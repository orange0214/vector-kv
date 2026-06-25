#include "vectorkv/vector_db.h"

namespace vectorkv {

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

    return store_.put(record);
}

bool VectorDB::remove(const std::string& id) {
    return store_.remove(id);
}

std::vector<SearchResult> VectorDB::search(
    const std::vector<float>& query,
    int top_k
) {
    return index_.search(store_, query, top_k);
}

}