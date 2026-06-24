#include "vectorkv/vector_store.h"
#include "vectorkv/types.h"
#include <cstddef>

namespace vectorkv {

bool VectorStore::put(const VectorRecord& record) {
    // use the first record to decide the dimension
    if (dim_ == 0) {
        dim_ = record.vector.size();
    } else if (record.vector.size() != dim_) {
        return false; // dimension inconsistency, return false
    }
    records_[record.id] = record;
    return true;
}

bool VectorStore::remove(const std::string& id) {
    auto it = records_.find(id);
    // 这里的 it->second 是因为 records_ 是 std::unordered_map<std::string, VectorRecord>。
    // find 返回的是一个指向键值对 (key, value) 的迭代器，first 是 key（string），second 是 value（VectorRecord）。
    // 所以 it->second 就是找到的 VectorRecord 对象，再用 .deleted 访问其 deleted 字段。
    if (it == records_.end() || it->second.deleted) {
        return false; // 不存在或者已删除
    }
    it->second.deleted = true;
    return true;
}

// 为什么不返回引用而是返回地址?因为如果没找到则返回空指针,引用则不能为空
const VectorRecord* VectorStore::get(const std::string& id) const {
    auto it = records_.find(id);
    if (it == records_.end() || it->second.deleted) {
        return nullptr;
    }
    return &it->second;
}

size_t VectorStore::size() const {
    size_t count = 0;
    for (const auto& [id, record] : records_) {
        if (!record.deleted) {
            ++count;
        }
    }
    return count;
}

}