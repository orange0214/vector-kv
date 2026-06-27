#pragma once

#include "vectorkv/types.h"

#include <unordered_map>
#include <string>
#include <functional>

namespace vectorkv {

class SnapShot{

public:

    static bool save(
        const std::string& path, 
        const std::unordered_map<std::string, VectorRecord>& records);

    static bool load(
        const std::string& path, 
        const std::function<void(const VectorRecord&)>& on_record);
};

}