#pragma once

#include <functional>
#include <string>
#include <fstream>

#include "vectorkv/types.h"


namespace vectorkv {

class WAL {
public:

    explicit WAL(const std::string& path);
    // 禁止拷贝构造
    WAL(const WAL&) = delete;
    // 操作符= 重载,不能通过=给类赋值
    WAL& operator=(const WAL&) = delete;

    bool append_insert(const VectorRecord& record);

    bool append_delete(const std::string& id);

    void replay(
        const std::function<void(const VectorRecord&)>& on_insert,
        const std::function<void(const std::string&)>& on_delete
    );

private:
    std::string path_;
    std::ofstream out_;

};

}