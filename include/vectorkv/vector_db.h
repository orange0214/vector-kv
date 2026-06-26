#pragma once

#include "vectorkv/types.h"
#include "vectorkv/vector_store.h"
#include "vectorkv/brute_force_index.h"
#include "vectorkv/wal.h"
#include <string>
#include <unordered_map>
#include <vector>
#include<optional>

namespace vectorkv {

class VectorDB {
public:

    VectorDB() = default;
    explicit VectorDB(const std::string& wal_path);

    bool insert(
        const std::string& id,
        const std::vector<float>& vector,
        const std::unordered_map<std::string, std::string>& metadata = {}
    );

    bool remove(const std::string& id);

    std::vector<SearchResult> search(
        const std::vector<float>& query,
        int top_k
    );

private:
    void recover();

    VectorStore store_;
    BruteForceIndex index_;
    std::optional<WAL> wal_;
};

}