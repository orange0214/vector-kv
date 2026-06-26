#include <gtest/gtest.h>
#include <cstdio>

#include "vectorkv/vector_db.h"

using vectorkv::VectorDB;

TEST(Recovery, DataSurvivesRestart) {
    const std::string path = "recovery_test.wal";
    std::remove(path.c_str());

    {
        VectorDB db(path);
        db.insert("a", {1.0f, 0.0f});
        db.insert("b", {0.0f, 1.0f});
        db.remove("a");
    }

    {
        VectorDB db(path);
        auto results = db.search({0.0f, 1.0f}, 10);

        ASSERT_EQ(results.size(), 1u); 
        EXPECT_EQ(results[0].id, "b");
    }

    std::remove(path.c_str());
}

TEST(Recovery, EmptyLogStartsEmpty) {
    const std::string path = "recovery_empty.wal";
    std::remove(path.c_str());

    VectorDB db(path);
    EXPECT_TRUE(db.search({1.0f, 0.0f}, 5).empty());

    std::remove(path.c_str());
}

TEST(Recovery, AppendsAccumulateAcrossRestarts) {
    const std::string path = "recovery_accum.wal";
    std::remove(path.c_str());

    { VectorDB db(path); db.insert("a", {1.0f, 0.0f}); }   
    { VectorDB db(path); db.insert("b", {0.0f, 1.0f}); }   
    {
        VectorDB db(path);
        EXPECT_EQ(db.search({1.0f, 0.0f}, 10).size(), 2u);
    }
    std::remove(path.c_str());
}

TEST(Recovery, MetadataSurvivesRestart) {
    const std::string path = "recovery_meta.wal";
    std::remove(path.c_str());

    { VectorDB db(path); db.insert("doc1", {1.0f, 0.0f}, {{"title", "cpp"}}); }
    
    {
        VectorDB db(path);
        auto results = db.search({1.0f, 0.0f}, 1);
        ASSERT_EQ(results.size(), 1u);
        EXPECT_EQ(results[0].metadata.at("title"), "cpp");
    }
    std::remove(path.c_str());
}