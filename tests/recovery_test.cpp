#include <gtest/gtest.h>
#include <cstdio>
#include <fstream>
#include <sstream>
#include <string>

#include "vectorkv/vector_db.h"

using vectorkv::VectorDB;

static std::string readFile(const std::string& path) {
    std::ifstream in(path);
    std::stringstream ss;
    ss << in.rdbuf();
    return ss.str();
}

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

TEST(Recovery, RecoveryWithSnapshotAndWAL) {
    const std::string wal_path = "recovery_snap.wal";
    const std::string snap_path = "recovery_snap.snapshot";
    std::remove(wal_path.c_str());
    std::remove(snap_path.c_str());

    {
        VectorDB db(wal_path, snap_path);
        db.insert("a", {1.0f, 0.0f});
        db.insert("b", {0.0f, 1.0f});
        db.save_snapshot(snap_path);
        db.insert("c", {0.0f, 0.0f});
    }

    {
        VectorDB db(wal_path, snap_path);
        EXPECT_EQ(db.search({1.0f, 0.0f}, 10).size(), 3u);
    }

    std::remove(wal_path.c_str());
    std::remove(snap_path.c_str());
}

TEST(Recovery, SnapshotThenDeleteViaWal) {
    const std::string wal_path = "recovery_snap_delete.wal";
    const std::string snap_path = "recovery_snap_delete.snapshot";
    std::remove(wal_path.c_str());
    std::remove(snap_path.c_str());
    {
        VectorDB db(wal_path, snap_path);
        db.insert("a", {1.0f, 0.0f});
        db.insert("b", {0.0f, 1.0f});
        db.save_snapshot(snap_path);   
        db.remove("a");                
    }
    {
        VectorDB db(wal_path, snap_path);
        auto results = db.search({0.0f, 1.0f}, 10);
        ASSERT_EQ(results.size(), 1u);
        EXPECT_EQ(results[0].id, "b");
    }
    std::remove(wal_path.c_str());
    std::remove(snap_path.c_str());
}

TEST(Recovery, LoadSnapshotViaPublicApi) {
    const std::string snap_path = "recovery_manual.snapshot";
    std::remove(snap_path.c_str());

    {
        VectorDB db;
        db.insert("doc1", {1.0f, 0.0f}, {{"title", "cpp"}});
        EXPECT_TRUE(db.save_snapshot(snap_path));
    }

    {
        VectorDB db;
        EXPECT_TRUE(db.load_snapshot(snap_path));
        auto results = db.search({1.0f, 0.0f}, 1);
        ASSERT_EQ(results.size(), 1u);
        EXPECT_EQ(results[0].id, "doc1");
        EXPECT_EQ(results[0].metadata.at("title"), "cpp");
    }

    std::remove(snap_path.c_str());
}

TEST(Recovery, CheckpointTruncatesWal) {
    const std::string wal_path = "recovery_checkpoint.wal";
    const std::string snap_path = "recovery_checkpoint.snapshot";
    std::remove(wal_path.c_str());
    std::remove(snap_path.c_str());
    {
        VectorDB db(wal_path, snap_path);
        db.insert("a", {1.0f, 0.0f});
        db.insert("b", {0.0f, 1.0f});
        EXPECT_TRUE(db.checkpoint());
        db.insert("c", {0.5f, 0.5f});
    }
    {
        VectorDB db(wal_path, snap_path);
        EXPECT_EQ(db.search({1.0f, 0.0f}, 10).size(), 3u);
    }
    std::remove(wal_path.c_str());
    std::remove(snap_path.c_str());
}

TEST(Recovery, CheckpointLeavesOnlyWalTail) {
    const std::string wal_path = "recovery_checkpoint_tail.wal";
    const std::string snap_path = "recovery_checkpoint_tail.snapshot";
    std::remove(wal_path.c_str());
    std::remove(snap_path.c_str());

    {
        VectorDB db(wal_path, snap_path);
        db.insert("a", {1.0f, 0.0f});
        db.insert("b", {0.0f, 1.0f});
        EXPECT_TRUE(db.checkpoint());
        db.insert("c", {0.5f, 0.5f});
    }

    EXPECT_EQ(readFile(wal_path), "INSERT c 2 0.5 0.5\n");

    std::remove(wal_path.c_str());
    std::remove(snap_path.c_str());
}

TEST(Recovery, CheckpointWithoutSnapshotPathFails) {
    const std::string wal_path = "recovery_checkpoint_no_snap.wal";
    std::remove(wal_path.c_str());

    VectorDB db(wal_path);
    db.insert("a", {1.0f, 0.0f});
    EXPECT_FALSE(db.checkpoint());

    std::remove(wal_path.c_str());
}