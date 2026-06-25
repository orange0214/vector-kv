#pragma once

#include "vectorkv/types.h"
#include "vectorkv/vector_store.h"

namespace vectorkv {

class BruteForceIndex {
public:
    std::vector<SearchResult> search(
        const VectorStore& store,
        const std::vector<float>& query,
        int top_k
    ) const;

};

}