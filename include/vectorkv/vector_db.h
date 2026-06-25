#pragma once

#include "vectorkv/types.h"
#include "vectorkv/vector_store.h"
#include "vectorkv/brute_force_index.h"
#include <string>
#include <unordered_map>
#include <vector>

namespace vectorkv {

class VectorDB {
public:
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
    VectorStore store_;
    BruteForceIndex index_;
};

}