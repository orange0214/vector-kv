#pragma once

#include "vectorkv/types.h"
#include <string>
#include <unordered_map>

namespace vectorkv {

class VectorStore {
public:
    // Returns false if vector dimensions are inconsistent
    bool put(const VectorRecord& record);

    bool remove(const std::string& id);

    // 第一个const用于返回值：表示get返回的指针指向的VectorRecord对象是不可修改的（保护Record只读）。
    // 第二个const用于成员函数：表示该成员函数不会修改当前VectorStore对象的成员变量，
    // 可保证在常量对象（const VectorStore）上调用。
    const VectorRecord* get(const std::string& id) const;

    // 用于外部获得 records_ 进行 brute force
    const std::unordered_map<std::string, VectorRecord>& records() const {
        return records_;
    }

    size_t size() const;

private:
    std::unordered_map<std::string, VectorRecord> records_;
    size_t dim_ = 0;
};
}