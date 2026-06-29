#include "vectorkv/vector_db.h"
#include "vectorkv/snapshot.h"
#include "vectorkv/types.h"
#include <cstddef>
#include <string>

namespace vectorkv {

VectorDB::VectorDB(const std::string& wal_path, const std::string& snapshot_path) {
    wal_.emplace(wal_path);
    snap_path_ = snapshot_path;
    recover();
}

void VectorDB::recover() {
    // snapshot
    if (!snap_path_.empty()) {
        SnapShot::load(snap_path_, [this](const VectorRecord& r){store_.put(r);});
    }
    // WAL
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
    if (!store_.put(record)) {
        return false;
    }
    maybe_checkpoint_after_write();
    return true;
}

bool VectorDB::remove(const std::string& id) {
    if (wal_) {
        wal_->append_delete(id);
    }
    if (!store_.remove(id)) {
        return false;
    }
    maybe_checkpoint_after_write();
    return true;
}

std::vector<SearchResult> VectorDB::search(
    const std::vector<float>& query,
    int top_k
) {
    return index_.search(store_, query, top_k);
}

bool VectorDB::save_snapshot(const std::string& path) {
    return SnapShot::save(path, store_.records());
}

bool VectorDB::load_snapshot(const std::string& path) {
    return SnapShot::load(
        path, 
        [this](const VectorRecord& r){store_.put(r);});
}

bool VectorDB::checkpoint() {
    if (!wal_ || snap_path_.empty()) {
        return false;
    }
    if (!SnapShot::save(snap_path_, store_.records())) {
        return false;
    }
    if (!wal_->truncate()) {
        return false;
    }

    writes_since_checkpoint_ = 0;

    return true;
}

void VectorDB::set_auto_checkpoint_threshold(size_t write_count) {
    auto_checkpoint_threshold_ = write_count;
}

void VectorDB::maybe_checkpoint_after_write() {
    if (!wal_ || snap_path_.empty() || auto_checkpoint_threshold_ == 0) {
        return;
    }
    ++writes_since_checkpoint_;
    if (writes_since_checkpoint_ >= auto_checkpoint_threshold_) {
        checkpoint();
    }
} 

}