#include <gtest/gtest.h>

#include "vectorkv/snapshot.h"
#include "vectorkv/types.h"

#include <cstdio>
#include <string>
#include <unordered_map>

TEST(Snapshot, SaveThenLoadRoundTrip) {
    const std::string path = "snapshot_test.tmp";
    std::remove(path.c_str());

    std::unordered_map<std::string, vectorkv::VectorRecord> records;
    {
        vectorkv::VectorRecord r;
        r.id = "doc1";
        r.vector = {1.0f, 2.0f, 3.0f};
        r.metadata["title"] = "cpp";
        records[r.id] = r;
    }
    {
        vectorkv::VectorRecord r;
        r.id = "doc2";
        r.vector = {4.0f, 5.0f, 6.0f};
        records[r.id] = r;
    }

    EXPECT_TRUE(vectorkv::SnapShot::save(path, records));

    std::unordered_map<std::string, vectorkv::VectorRecord> loaded;
    EXPECT_TRUE(vectorkv::SnapShot::load(
        path,
        [&loaded](const vectorkv::VectorRecord& r) { loaded[r.id] = r; }
    ));

    ASSERT_EQ(loaded.size(), 2u);
    EXPECT_EQ(loaded.at("doc1").vector.size(), 3u);
    EXPECT_FLOAT_EQ(loaded.at("doc1").vector[2], 3.0f);
    EXPECT_EQ(loaded.at("doc1").metadata.at("title"), "cpp");
    EXPECT_FLOAT_EQ(loaded.at("doc2").vector[0], 4.0f);

    std::remove(path.c_str());
}

TEST(Snapshot, SkipDeletedRecord) {
    const std::string path = "snapshot_test.tmp";
    std::remove(path.c_str());

    std::unordered_map<std::string, vectorkv::VectorRecord> records;
    {
        vectorkv::VectorRecord r;
        r.id = "doc1";
        r.vector = {1.0f, 2.0f, 3.0f};
        r.metadata["title"] = "cpp";
        records[r.id] = r;
    }
    {
        vectorkv::VectorRecord r;
        r.id = "doc2";
        r.vector = {4.0f, 5.0f, 6.0f};
        r.deleted = true;
        records[r.id] = r;
    }

    EXPECT_TRUE(vectorkv::SnapShot::save(path, records));
    std::unordered_map<std::string, vectorkv::VectorRecord> loaded;
    EXPECT_TRUE(vectorkv::SnapShot::load(
        path,
        [&loaded](const vectorkv::VectorRecord& r) { loaded[r.id] = r; }
    ));

    ASSERT_EQ(loaded.size(), 1u);
    EXPECT_EQ(loaded.at("doc1").id, "doc1");

    std::remove(path.c_str());
}

TEST(Snapshot, LoadMissingFileReturnsFalse) {
    const std::string path = "snapshot_does_not_exist.tmp";
    std::remove(path.c_str());
    int calls = 0;
    bool ok = vectorkv::SnapShot::load(
        path,
        [&calls](const vectorkv::VectorRecord&) { ++calls; }
    );
    EXPECT_FALSE(ok);
    EXPECT_EQ(calls, 0);
}