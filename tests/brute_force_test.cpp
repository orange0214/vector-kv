#include <gtest/gtest.h>
#include <utility>

#include "vectorkv/brute_force_index.h"
#include "vectorkv/vector_store.h"
#include "vectorkv/types.h"

using vectorkv::BruteForceIndex;
using vectorkv::SearchResult;
using vectorkv::VectorRecord;
using vectorkv::VectorStore;

static VectorRecord makeRecord(const std::string& id, std::vector<float> v) {
    VectorRecord r;
    r.id = id;
    r.vector = std::move(v);
    return r;
}

static VectorStore makeStore() {
    VectorStore store;
    store.put(makeRecord("same", {1.0f, 0.0f}));
    store.put(makeRecord("diag", {1.0f, 1.0f}));
    store.put(makeRecord("orth", {0.0f, 1.0f}));
    store.put(makeRecord("opp",  {-1.0f, 0.0f}));
    return store;
}

TEST(BruteForce, ReturnsInDescendingScoreOrder) {
    VectorStore store = makeStore();
    BruteForceIndex index;
    auto results = index.search(store, {1.0f, 0.0f}, 4);
    ASSERT_EQ(results.size(), 4u);
    EXPECT_EQ(results[0].id, "same");
    EXPECT_EQ(results[1].id, "diag");
    EXPECT_EQ(results[2].id, "orth");
    EXPECT_EQ(results[3].id, "opp");
    
    for (size_t i = 1; i < results.size(); ++i) {
        EXPECT_GE(results[i - 1].score, results[i].score);
    }
    EXPECT_NEAR(results[0].score, 1.0f, 1e-5);
    EXPECT_NEAR(results[3].score, -1.0f, 1e-5);
}

TEST(BruteForce, RespectsTopK) {
    VectorStore store = makeStore();
    BruteForceIndex index;

    auto results = index.search(store, {1.0f, 0.0f}, 2);
    
    ASSERT_EQ(results.size(), 2u);
    EXPECT_EQ(results[0].id, "same");
    EXPECT_EQ(results[1].id, "diag");
}


TEST(BruteForce, TopKLargerThanStoreReturnsAll) {
    VectorStore store = makeStore();
    BruteForceIndex index;
    auto results = index.search(store, {1.0f, 0.0f}, 100);
    EXPECT_EQ(results.size(), 4u);
}

TEST(BruteForce, ExcludesDeleted) {
    VectorStore store = makeStore();
    store.remove("same");                   
    BruteForceIndex index;
    auto results = index.search(store, {1.0f, 0.0f}, 4);
    ASSERT_EQ(results.size(), 3u);          
    EXPECT_EQ(results[0].id, "diag");       
    for (const auto& r : results) {
        EXPECT_NE(r.id, "same");            
    }
}

TEST(BruteForce, NonPositiveTopKReturnsEmpty) {
    VectorStore store = makeStore();
    BruteForceIndex index;
    EXPECT_TRUE(index.search(store, {1.0f, 0.0f}, 0).empty());
    EXPECT_TRUE(index.search(store, {1.0f, 0.0f}, -3).empty());
}

TEST(BruteForce, EmptyStoreReturnsEmpty) {
    VectorStore store;
    BruteForceIndex index;
    EXPECT_TRUE(index.search(store, {1.0f, 0.0f}, 5).empty());
}

TEST(BruteForce, CarriesMetadata) {
    VectorStore store;
    VectorRecord r = makeRecord("doc1", {1.0f, 0.0f});
    r.metadata["title"] = "hello";
    store.put(r);
    BruteForceIndex index;
    auto results = index.search(store, {1.0f, 0.0f}, 1);
    ASSERT_EQ(results.size(), 1u);
    EXPECT_EQ(results[0].metadata.at("title"), "hello");
}