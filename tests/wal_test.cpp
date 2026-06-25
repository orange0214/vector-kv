#include <gtest/gtest.h>

#include "vectorkv/wal.h"
#include "vectorkv/types.h"

#include <fstream>
#include <sstream>
#include <string>
#include <cstdio>

static std::string readFile(const std::string& path) {
    std::ifstream in(path);
    std::stringstream ss;
    ss << in.rdbuf();
    return ss.str();
}

TEST(WAL, AppendInsertWritesOneLine) {
    const std::string path = "wal_test.tmp";
    // 删除旧的重名文件
    std::remove(path.c_str());

    {
        vectorkv::WAL wal(path);

        vectorkv::VectorRecord rec;
        rec.id = "doc1";
        rec.vector = {1.0f, 2.0f, 3.0f};

        EXPECT_TRUE(wal.append_insert(rec));
    }

    EXPECT_EQ(readFile(path), "INSERT doc1 3 1 2 3\n");
}

TEST(WAL, AppendDeleteWritesOneLine) {
    const std::string path = "wal_test.tmp";
    std::remove(path.c_str());

    {
        vectorkv::WAL wal(path);

        std::string id = "doc1";

        EXPECT_TRUE(wal.append_delete(id));
    }

    EXPECT_EQ(readFile(path), "DELETE doc1\n");
}