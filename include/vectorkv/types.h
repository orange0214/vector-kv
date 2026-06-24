#pragma once

#include <string>
#include <vector>
#include <unordered_map>

namespace vectorkv {

struct VectorRecord {
    std::string id;
    std::vector<float> vector;
    std::unordered_map<std::string, std::string> metadata;
    bool deleted = false;
};

struct SearchResult {
    std::string id;
    float score;
    std::unordered_map<std::string, std::string> metadata;
};

}