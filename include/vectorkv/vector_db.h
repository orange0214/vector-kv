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
    explicit VectorDB(
        const std::string& wal_path, 
        const std::string& snapshot_path = ""
    );

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

    bool save_snapshot(const std::string& path);

    bool load_snapshot(const std::string& path);

    bool checkpoint();

    void set_auto_checkpoint_threshold(size_t write_count);

private:
    void recover();

    void maybe_checkpoint_after_write();

    VectorStore store_;
    BruteForceIndex index_;
    std::optional<WAL> wal_;
    std::string snap_path_;
    size_t auto_checkpoint_threshold_ = 0;
    size_t writes_since_checkpoint_ = 0;
};

}