#include <gtest/gtest.h>

#include "vectorkv/vector_db.h"

using vectorkv::SearchResult;
using vectorkv::VectorDB;

TEST(VectorDB, InsertThenSearchFindsMostSimilarFirst) {
    VectorDB db;
    db.insert("same", {1.0f, 0.0f});
    db.insert("diag", {1.0f, 1.0f});
    db.insert("orth", {0.0f, 1.0f});
    auto results = db.search({1.0f, 0.0f}, 3);
    ASSERT_EQ(results.size(), 3u);
    EXPECT_EQ(results[0].id, "same");
    EXPECT_EQ(results[1].id, "diag");
}

TEST(VectorDB, RemoveExcludesFromSearch) {
    VectorDB db;
    db.insert("a", {1.0f, 0.0f});
    db.insert("b", {0.0f, 1.0f});
    EXPECT_TRUE(db.remove("a"));
    auto results = db.search({1.0f, 0.0f}, 10);
    ASSERT_EQ(results.size(), 1u);
    EXPECT_EQ(results[0].id, "b");
}

TEST(VectorDB, RemoveMissingReturnsFalse) {
    VectorDB db;
    EXPECT_FALSE(db.remove("nope"));
}

TEST(VectorDB, DuplicateInsertOverwrites) {
    VectorDB db;
    db.insert("doc1", {1.0f, 0.0f});
    db.insert("doc1", {0.0f, 1.0f});
    auto results = db.search({1.0f, 0.0f}, 10);
    ASSERT_EQ(results.size(), 1u);
    EXPECT_EQ(results[0].id, "doc1");
    EXPECT_NEAR(results[0].score, 0.0f, 1e-5);
}

TEST(VectorDB, DimensionMismatchRejected) {
    VectorDB db;
    EXPECT_TRUE(db.insert("doc1", {1.0f, 2.0f, 3.0f}));
    EXPECT_FALSE(db.insert("doc2", {1.0f, 2.0f}));     
    auto results = db.search({1.0f, 2.0f, 3.0f}, 10);
    ASSERT_EQ(results.size(), 1u);
    EXPECT_EQ(results[0].id, "doc1");
}

TEST(VectorDB, SearchReturnsMetadata) {
    VectorDB db;
    db.insert("doc1", {1.0f, 0.0f}, {{"title", "hello"}});
    auto results = db.search({1.0f, 0.0f}, 1);
    ASSERT_EQ(results.size(), 1u);
    EXPECT_EQ(results[0].metadata.at("title"), "hello");
}

TEST(VectorDB, SearchOnEmptyDbReturnsEmpty) {
    VectorDB db;
    EXPECT_TRUE(db.search({1.0f, 0.0f}, 5).empty());
}