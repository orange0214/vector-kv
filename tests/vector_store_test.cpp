#include <gtest/gtest.h>

#include "vectorkv/vector_store.h"
#include "vectorkv/types.h"

using vectorkv::VectorRecord;
using vectorkv::VectorStore;

static VectorRecord makeRecord(const std::string& id, std::vector<float> v) {
    VectorRecord r;
    r.id = id;
    r.vector = std::move(v);
    return r;
}

TEST(VectorStore, PutThenGetReturnsRecord) {
    VectorStore store;
    EXPECT_TRUE(store.put(makeRecord("doc1", {1.0f, 2.0f, 3.0f})));

    const VectorRecord* rec = store.get("doc1");
    // ASSERT_NE 的作用是「断言两个值不相等」。
    // 如果 rec == nullptr（即没找到），测试会立即失败并报错；
    // 只有 rec != nullptr 时才会继续往下执行。
    ASSERT_NE(rec, nullptr);
    EXPECT_EQ(rec->id, "doc1");
    EXPECT_EQ(rec->vector.size(), 3u);
    EXPECT_EQ(store.size(), 1u);
}

TEST(VectorStore, GetMissingReturnsNull) {
    VectorStore store;
    EXPECT_EQ(store.get("nope"), nullptr);
}

TEST(VectorStore, RemoveMarksDeleted) {
    VectorStore store;
    store.put(makeRecord("doc1", {1.0f, 2.0f}));
    EXPECT_TRUE(store.remove("doc1"));
    EXPECT_EQ(store.get("doc1"), nullptr);
    EXPECT_EQ(store.size(), 0u); 
}

TEST(VectorStore, RemoveMissingReturnsFalse) {
    VectorStore store;
    EXPECT_FALSE(store.remove("nope"));
}

TEST(VectorStore, RemoveTwiceReturnsFalse) {
    VectorStore store;
    store.put(makeRecord("doc1", {1.0f, 2.0f}));
    EXPECT_TRUE(store.remove("doc1"));
    EXPECT_FALSE(store.remove("doc1"));
}

TEST(VectorStore, DimensionMismatchRejected) {
    VectorStore store;
    EXPECT_TRUE(store.put(makeRecord("doc1", {1.0f, 2.0f, 3.0f})));  // 维度=3
    EXPECT_FALSE(store.put(makeRecord("doc2", {1.0f, 2.0f})));        // 维度=2,拒绝
    EXPECT_EQ(store.get("doc2"), nullptr);
    EXPECT_EQ(store.size(), 1u);
}

TEST(VectorStore, DuplicateIdOverwrites) {
    VectorStore store;
    store.put(makeRecord("doc1", {1.0f, 2.0f, 3.0f}));
    store.put(makeRecord("doc1", {4.0f, 5.0f, 6.0f}));  // 同 id 覆盖
    const VectorRecord* rec = store.get("doc1");
    ASSERT_NE(rec, nullptr);
    EXPECT_FLOAT_EQ(rec->vector[0], 4.0f);   // 是新值
    EXPECT_EQ(store.size(), 1u);             // 仍只有一条
}