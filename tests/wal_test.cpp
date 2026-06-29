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

TEST(WAL, ReplayReadsBackInsertsAndDeletes) {
    const std::string path = "wal_replay_test.tmp";
    std::remove(path.c_str());

    // write
    {
        vectorkv::WAL wal(path);
        vectorkv::VectorRecord rec;
        rec.id = "doc1";
        rec.vector = {1.0f, 2.0f, 3.0f};
        rec.metadata["title"] = "cpp";
        wal.append_insert(rec);
        wal.append_delete("doc2");
    }

    // replay
    std::vector<vectorkv::VectorRecord> inserts;
    std::vector<std::string> deletes;
    {
        vectorkv::WAL wal(path);
        wal.replay(
            [&inserts](const vectorkv::VectorRecord& r){inserts.push_back(r);},
            [&deletes](const std::string& id){deletes.push_back(id);}
        );
    }

    ASSERT_EQ(inserts.size(), 1u);
    EXPECT_EQ(inserts[0].id, "doc1");
    ASSERT_EQ(inserts[0].vector.size(), 3u);
    EXPECT_FLOAT_EQ(inserts[0].vector[0], 1.0f);
    EXPECT_FLOAT_EQ(inserts[0].vector[2], 3.0f);
    EXPECT_EQ(inserts[0].metadata.at("title"), "cpp");
    ASSERT_EQ(deletes.size(), 1u);
    EXPECT_EQ(deletes[0], "doc2");
}

TEST(WAL, TruncateClearsPriorEntries) {
    const std::string path = "wal_truncate_test.tmp";
    std::remove(path.c_str());

    {
        vectorkv::WAL wal(path);

        vectorkv::VectorRecord old_rec;
        old_rec.id = "old";
        old_rec.vector = {1.0f, 2.0f};
        EXPECT_TRUE(wal.append_insert(old_rec));
        EXPECT_TRUE(wal.append_delete("old"));
    }

    {
        vectorkv::WAL wal(path);
        EXPECT_TRUE(wal.truncate());
        vectorkv::VectorRecord new_rec;
        new_rec.id = "new";
        new_rec.vector = {3.0f, 4.0f};
        EXPECT_TRUE(wal.append_insert(new_rec));
    }

    EXPECT_EQ(readFile(path), "INSERT new 2 3 4\n");
    std::remove(path.c_str());
}