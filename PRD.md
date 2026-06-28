# VectorKV PRD

## 1. Project Overview

**Project Name:** VectorKV

**Full Name:** VectorKV: A C++ In-Memory Vector Database with HNSW Search and WAL Recovery

**Project Type:** C++ systems project / AI infrastructure project

VectorKV is a lightweight in-memory vector database implemented in C++. It supports vector insertion, deletion, top-k similarity search, metadata storage, write-ahead log recovery, snapshot persistence, concurrent search, and benchmark tooling.

The goal is not to build a full Milvus, Pinecone, or Weaviate replacement. The goal is to build a focused, technically deep, resume-ready system that demonstrates C++ systems programming, vector search algorithms, persistence, concurrency, and performance measurement.

## 2. Motivation

Modern AI applications often rely on embedding search for retrieval-augmented generation, recommendation, semantic search, and document retrieval. Existing systems such as pgvector, Milvus, and Pinecone hide most of the infrastructure behind an API.

VectorKV aims to expose and implement the core mechanisms underneath a vector database:

- How vectors are stored and retrieved.
- How top-k similarity search works.
- How approximate nearest neighbor search improves latency.
- How write-ahead logs and snapshots prevent data loss.
- How concurrent reads are handled safely.
- How latency, throughput, and recall are measured.

This makes VectorKV a strong project for demonstrating backend, database, systems, and AI infrastructure skills.

## 3. Target Users

The target user is a developer who wants to store embedding vectors and query similar vectors efficiently.

Example records:

```text
id: "doc_123"
vector: [0.12, -0.33, 0.91, ...]
metadata: {"title": "C++ notes", "tag": "systems"}
```

The user should be able to:

- Insert a vector with metadata.
- Delete a vector by ID.
- Search for the most similar vectors to a query vector.
- Restart the database without losing data.
- Run benchmarks to compare brute-force search and HNSW search.
- Measure latency, throughput, and recall.

## 4. Goals

### 4.1 Functional Goals

VectorKV should support:

- `insert(id, vector, metadata)`
- `search(query_vector, top_k)`
- `delete(id)`
- `save_snapshot(path)`
- `load_snapshot(path)`
- WAL-based crash recovery
- Brute-force exact search baseline
- HNSW or HNSW-like approximate nearest neighbor search
- Concurrent read/search workloads
- Benchmarking for QPS, latency, and Recall@K

### 4.2 Technical Goals

VectorKV should demonstrate:

- Modern C++ project structure using CMake.
- Clean module boundaries.
- Unit tests for core components.
- File I/O and binary or structured persistence.
- Top-k similarity search.
- Graph-based approximate nearest neighbor indexing.
- Read/write coordination using C++ concurrency primitives.
- Benchmark-driven performance analysis.

### 4.3 Resume Goals

The final project should support resume bullets such as:

```text
Built VectorKV, a C++ in-memory vector database with HNSW-based approximate nearest neighbor search, WAL recovery, snapshot persistence, concurrent query execution, and benchmark tooling.
```

```text
Implemented a C++ vector database supporting HNSW indexing, WAL-based crash recovery, snapshot persistence, and multi-threaded search; reduced top-k query latency from X ms brute-force baseline to Y ms while maintaining Z Recall@10 on 100K vectors.
```

## 5. Non-Goals

The first version will not support:

- SQL query language.
- Full distributed deployment.
- GPU acceleration.
- Full transaction support.
- Authentication or authorization.
- Multi-tenancy.
- Complex metadata filtering.
- Production-grade replication.
- Full compatibility with existing vector database APIs.

These may be considered future extensions, but they are not required for the resume-ready version.

## 6. Core Use Cases

### 6.1 Insert Vector

The user inserts a vector with an ID and optional metadata.

```cpp
db.insert("doc1", {0.1f, 0.2f, 0.3f}, {{"title", "C++ notes"}});
```

Expected behavior:

- Validate vector dimension.
- Write the operation to WAL.
- Store the vector in memory.
- Add the vector to the active index.

### 6.2 Search Similar Vectors

The user provides a query vector and asks for the top K most similar vectors.

```cpp
auto results = db.search({0.11f, 0.19f, 0.31f}, 5);
```

Expected behavior:

- Validate query dimension.
- Search the active index.
- Return top K non-deleted results.
- Include similarity score and metadata.

### 6.3 Delete Vector

The user deletes a vector by ID.

```cpp
db.remove("doc1");
```

Expected behavior:

- Write delete operation to WAL.
- Mark the record as deleted.
- Exclude deleted records from future search results.

Physical graph cleanup may be deferred.

### 6.4 Recovery After Restart

The database should recover records after restart.

Expected behavior:

- Load the latest snapshot if available.
- Replay WAL entries **after** the snapshot (incremental operations only).
- Rebuild or restore the vector index.
- Return the same logical data state as before shutdown.

Without checkpointing, the WAL grows forever and recovery replays the entire log even when a snapshot exists. A **checkpoint** closes that gap: persist full state, then truncate the WAL so only post-checkpoint operations remain in the log.

### 6.5 Checkpoint (Snapshot + WAL Truncation)

A checkpoint captures the current logical database state and resets the WAL baseline.

Expected behavior:

- **Manual checkpoint:** `checkpoint()` (or `save_snapshot` followed by WAL truncate) writes the full state, then **truncates the WAL only after the snapshot is durably on disk**. Order matters: snapshot first, WAL truncate second — reversing risks data loss on crash.
- **Automatic checkpoint (Step B):** After a configurable number of writes (e.g. every N `insert`/`remove` calls), the database triggers the same checkpoint flow without user intervention. `search` does not count toward N.
- After checkpoint, recovery is: load snapshot + replay the **short** WAL tail.
- WAL entries before the checkpoint are redundant (their effect is already in the snapshot) and may be discarded safely.

Example checkpoint flow:

```text
1. insert/remove ... (WAL append)
2. checkpoint triggered (manual or automatic)
3. save_snapshot  →  full state on disk
4. truncate WAL   →  empty log; new writes append from here
5. restart        →  load snapshot + replay small WAL
```

### 6.6 Benchmark Search Performance

The user should be able to run:

```bash
./vector_bench --vectors 100000 --dim 384 --queries 10000 --threads 8 --top_k 10
```

Expected output:

```text
vectors: 100000
dimension: 384
threads: 8
top_k: 10
QPS: 12500
P50 latency: 0.7 ms
P95 latency: 2.3 ms
P99 latency: 5.8 ms
Recall@10: 0.92
```

## 7. API Design

### 7.1 Core Data Structures

```cpp
struct VectorRecord {
    std::string id;
    std::vector<float> vector;
    std::unordered_map<std::string, std::string> metadata;
    bool deleted = false;
};

struct SearchResult {
    std::string id;
    float score;
    std::unordered_map<std::string, std::string> metadata;
};
```

### 7.2 VectorDB API

```cpp
class VectorDB {
public:
    bool insert(
        const std::string& id,
        const std::vector<float>& vector,
        const std::unordered_map<std::string, std::string>& metadata);

    bool remove(const std::string& id);

    std::vector<SearchResult> search(
        const std::vector<float>& query,
        int top_k);

    bool save_snapshot(const std::string& path);

    bool load_snapshot(const std::string& path);

    // Checkpoint: save full state, then truncate WAL (snapshot must succeed first)
    bool checkpoint();

    // Automatic checkpoint every N insert/remove ops (0 = disabled)
    void set_auto_checkpoint_threshold(size_t write_count);
};
```

### 7.3 Optional CLI

```bash
./vectorkv insert doc1 "0.1,0.2,0.3" title="cpp notes"
./vectorkv search "0.1,0.2,0.29" --top_k 5
./vectorkv delete doc1
```

## 8. Technology Stack and Dependency Strategy

VectorKV should be developed as a C++ systems project rather than as a framework-first application. The core database mechanisms should be implemented directly, while mature libraries should be used for build, testing, benchmarking, command-line parsing, logging, and optional service exposure.

### 8.1 Required Core Technology

- **Language:** C++20
- **Build System:** CMake
- **Core Runtime:** Standard C++ library
- **Concurrency:** `std::thread`, `std::shared_mutex`, `std::unique_lock`, `std::shared_lock`
- **Storage I/O:** Standard file streams or low-level file APIs where needed

### 8.2 Recommended Supporting Libraries

| Area | Recommended Choice | Purpose | Required for MVP |
| --- | --- | --- | --- |
| Build system | CMake | Project configuration, targets, include paths, build types | Yes |
| Unit testing | GoogleTest | Unit tests for `VectorStore`, `HNSWIndex`, `WAL`, recovery, and distance functions | Yes |
| Benchmarking | Google Benchmark or simple custom benchmark runner | Measure QPS, latency, index build time, and Recall@K | Yes |
| CLI parsing | CLI11 or simple hand-written parser | Support demo commands and benchmark arguments | Optional for MVP |
| RPC framework | gRPC with Protobuf | Expose VectorKV as a remote service in a later phase | No, later extension |
| Logging | spdlog or fmt plus standard output/error | Structured logs for insert, search, recovery, and benchmark runs | Optional but recommended |

### 8.3 What Should Be Implemented In-House

The following parts are the core learning and resume value of the project and should be implemented directly:

- Vector record storage.
- Cosine similarity and optional L2 distance.
- Top-k search using a priority queue.
- Brute-force exact search baseline.
- HNSW or HNSW-like approximate nearest neighbor index.
- WAL append and replay logic.
- Snapshot save and load logic.
- Read/write concurrency control around database state.
- Recall@K calculation.
- Latency percentile reporting if not using a benchmark library for this directly.

### 8.4 What Should Not Be Reimplemented Initially

The following parts are not central to the project and should use existing tools or libraries:

- Build orchestration: use CMake instead of hand-written compiler scripts.
- Unit test framework: use GoogleTest instead of writing a custom test framework.
- Microbenchmark harness: use Google Benchmark or a small custom runner, but avoid building a complex benchmark framework.
- CLI argument parsing: use CLI11 or a minimal hand-written parser.
- RPC transport: use gRPC later instead of implementing a custom network protocol in the first version.
- Logging formatting: use spdlog or fmt when structured logging becomes useful.

### 8.5 Dependency Philosophy

Dependencies should support engineering quality without hiding the important system internals. A good rule:

```text
Use libraries for project infrastructure.
Implement the database, indexing, persistence, and concurrency mechanisms yourself.
```

This keeps the project focused on the skills it is meant to demonstrate: C++ systems programming, algorithm implementation, persistence, performance analysis, and AI infrastructure fundamentals.

## 9. System Architecture

```text
Client / CLI / Benchmark
        |
        v
VectorDB API
        |
        +--------------------+
        |                    |
        v                    v
Vector Store            Vector Index
id -> vector            HNSW graph
id -> metadata          ANN search
id -> deleted flag
        |
        v
Persistence Layer
WAL + Snapshot
        |
        v
Metrics / Benchmark
QPS, latency, recall
```

## 10. Module Breakdown

### 10.1 VectorStore

Responsibilities:

- Store vector records in memory.
- Map external string IDs to internal records.
- Track deleted records.
- Validate vector dimensions.

Possible data structure:

```cpp
std::unordered_map<std::string, VectorRecord> records;
```

### 10.2 BruteForceIndex

Responsibilities:

- Provide exact top-k search.
- Serve as correctness baseline.
- Serve as benchmark ground truth for Recall@K.

Search algorithm:

```text
For each non-deleted vector:
  compute similarity(query, vector)
  maintain top_k candidates in a priority queue
return candidates sorted by score
```

### 10.3 HNSWIndex

Responsibilities:

- Store graph-based approximate nearest neighbor index.
- Support vector insertion.
- Support top-k approximate search.
- Expose configurable parameters such as `M` and `ef_search`.

Initial implementation may use a simplified single-layer HNSW-like graph:

- Each node stores up to `M` neighbor IDs.
- Insert connects the new node to the closest `M` existing nodes.
- Search starts from an entry point and expands candidates using a priority queue.

Future implementation may add:

- Multi-layer graph.
- Random level assignment.
- Layer-by-layer greedy descent.

### 10.4 WAL

Responsibilities:

- Append insert and delete operations before mutating in-memory state.
- Flush writes to disk.
- Replay operations during recovery.
- **Truncate (reset) the log after a successful checkpoint**, so the WAL only holds operations since the last snapshot.

Example WAL format:

```text
INSERT doc1 3 0.1 0.2 0.3 title=cpp
DELETE doc1
```

The first implementation can use a text format for readability. A later version may use binary encoding.

### 10.5 Snapshot

Responsibilities:

- Persist the current logical state of the database.
- Reduce recovery time by avoiding **full** WAL replay (requires WAL truncate at checkpoint — Step A).
- Store vector dimension, records, metadata; skip soft-deleted records in the snapshot file.

**Checkpoint semantics (Step A — WAL truncate on snapshot):**

When `checkpoint()` or a successful snapshot write completes and a WAL is active:

1. Write the snapshot (full `store_` state) to disk and flush.
2. Only then truncate the WAL file to empty (or reopen append mode on a fresh file).
3. Never truncate before the snapshot is durable — a crash between truncate and snapshot would lose data.

**Automatic checkpoint (Step B):**

- `VectorDB` tracks write operations (`insert` / `remove`).
- When the count since the last checkpoint reaches `auto_checkpoint_threshold` (configurable, default e.g. 1000), invoke the internal checkpoint flow automatically.
- Alternative trigger (future): WAL file size exceeds a byte limit.

The HNSW index may either be:

- Rebuilt from vector records after loading the snapshot.
- Persisted directly in a later version.

### 10.6 Metrics and Benchmark

Responsibilities:

- Measure QPS.
- Measure P50, P95, and P99 latency.
- Measure Recall@K against brute-force search.
- Measure index build time.
- Optionally measure memory usage.

## 11. Concurrency Model

Vector database workloads are usually read-heavy. The first concurrent version should optimize for concurrent search.

Recommended model:

- `search`: shared read lock
- `insert`: exclusive write lock
- `delete`: exclusive write lock
- `snapshot`: copy state under lock, then write to disk outside the critical path

Example:

```cpp
std::shared_mutex db_mutex;

std::vector<SearchResult> search(...) {
    std::shared_lock lock(db_mutex);
    // read vector store and index
}

bool insert(...) {
    std::unique_lock lock(db_mutex);
    // write WAL, update vector store, update index
}

bool remove(...) {
    std::unique_lock lock(db_mutex);
    // write WAL, mark deleted
}
```

Initial concurrency target:

- Support multi-threaded search.
- Keep insert and delete single-writer.
- Avoid concurrent HNSW mutation complexity in the first version.

## 12. Similarity Metrics

The first version should support cosine similarity.

Cosine similarity:

```text
score = dot(a, b) / (norm(a) * norm(b))
```

Optional later metrics:

- L2 distance
- Inner product

## 13. Benchmark Design

### 13.1 Performance Benchmark

Variables:

- Vector count: 10K, 100K, 1M
- Dimension: 128, 384, 768
- Query count: 1K, 10K, 100K
- Threads: 1, 2, 4, 8, 16
- Top K: 5, 10, 50

Metrics:

- QPS
- P50 latency
- P95 latency
- P99 latency
- Index build time
- Memory usage if feasible

### 13.2 Recall Benchmark

Use brute-force exact search as ground truth.

```text
Recall@10 = number of overlapping IDs between brute-force top 10 and HNSW top 10 / 10
```

Example README table:

```text
Dataset: 100K vectors, dim=384, top_k=10

Index       P99 Latency     Recall@10
BruteForce  120 ms          1.00
HNSW        6 ms            0.92
```

## 14. Project Structure

```text
VectorKV/
  CMakeLists.txt
  README.md

  include/
    vector_db.h
    vector_store.h
    brute_force_index.h
    hnsw_index.h
    wal.h
    snapshot.h
    distance.h
    metrics.h

  src/
    vector_db.cpp
    vector_store.cpp
    brute_force_index.cpp
    hnsw_index.cpp
    wal.cpp
    snapshot.cpp
    distance.cpp
    metrics.cpp

  tests/
    vector_store_test.cpp
    brute_force_test.cpp
    hnsw_test.cpp
    wal_test.cpp
    recovery_test.cpp

  bench/
    vector_bench.cpp
    recall_bench.cpp

  examples/
    basic_demo.cpp
    semantic_search_demo.cpp

  data/
    sample_vectors.txt
```

## 15. Milestones

### Week 1: Project Setup and Brute-Force Search

Deliverables:

- CMake project setup.
- Initial `vectorkv_core` library target.
- GoogleTest setup for unit tests.
- `VectorStore`.
- `insert`, `delete`, and brute-force `search`.
- Cosine similarity.
- Basic unit tests.

### Week 2: Top-K Search and Benchmark Baseline

Deliverables:

- Priority queue based top-k search.
- Brute-force benchmark using Google Benchmark or a simple custom runner.
- Latency and QPS measurement.
- CLI argument parsing for benchmark parameters using CLI11 or a minimal hand-written parser.
- Basic README with usage examples.

### Week 3: WAL and Snapshot Persistence

Deliverables:

- [x] WAL append for insert and delete.
- [x] WAL replay on startup.
- [x] Snapshot save and load.
- [x] Recovery tests (WAL-only and snapshot + WAL).
- [ ] **Checkpoint (Step A):** truncate WAL after successful snapshot.
- [ ] **Automatic checkpoint (Step B):** trigger checkpoint every N writes.
- [ ] Checkpoint integration tests (bounded WAL after checkpoint; recovery after auto-checkpoint).

### Week 4: HNSW-like Index V1

Deliverables:

- Graph node representation.
- Neighbor list management.
- Single-layer HNSW-like insertion.
- Approximate top-k search.

### Week 5: Recall and Latency Comparison

Deliverables:

- Recall@K benchmark.
- Brute-force vs HNSW latency comparison.
- HNSW parameter tuning for `M` and `ef_search`.

### Week 6: Concurrent Search

Deliverables:

- `std::shared_mutex` based read/write coordination.
- Multi-threaded search benchmark.
- P50, P95, P99 latency reporting.

### Week 7: Engineering Polish

Deliverables:

- Error handling.
- More complete tests.
- Structured logging using spdlog or fmt if needed.
- Cleaner README.
- Architecture diagram.

### Week 8: Optional Service Layer and Resume Packaging

Deliverables:

- Optional gRPC API using Protobuf.
- Optional Docker demo.
- Final benchmark table.
- Resume bullet points.
- Project screenshots or terminal demos.

## 16. Acceptance Criteria

The project is considered resume-ready when it satisfies:

- Supports vector insert, search, and delete.
- Supports WAL-based recovery.
- Supports snapshot persistence.
- Implements brute-force exact search baseline.
- Implements HNSW or HNSW-like ANN search.
- Reports Recall@K using brute-force as ground truth.
- Supports concurrent search.
- Reports QPS and P50/P95/P99 latency.
- Contains unit tests for key modules.
- Uses CMake to build library, example, test, and benchmark targets.
- Uses GoogleTest or an equivalent standard testing setup.
- Uses Google Benchmark or a clearly documented custom benchmark runner.
- Contains a clear README with architecture and benchmark results.

Ideal target:

```text
Dataset size: 100K vectors
Dimension: 384
Top K: 10
Recall@10: >= 0.90
HNSW speedup: >= 10x over brute-force
Concurrent search: supports 8 query threads
Recovery: data survives restart
```

## 17. Risks and Mitigations

### Risk: HNSW is difficult to implement correctly

Mitigation:

- Start with brute-force search.
- Implement a simplified single-layer graph first.
- Add full multi-layer HNSW only after the simple version works.

### Risk: Project scope becomes too large

Mitigation:

- Treat distributed deployment, GPU acceleration, and service APIs as optional.
- Focus first on local in-memory database, persistence, ANN index, and benchmark.

### Risk: Concurrency bugs become hard to debug

Mitigation:

- Build single-threaded correctness first.
- Add only concurrent search in the first concurrent version.
- Keep insert and delete under exclusive write lock.

### Risk: Benchmark numbers are not impressive

Mitigation:

- Always compare against brute-force baseline.
- Report both speed and recall.
- Tune `M` and `ef_search`.
- Use realistic vector dimensions such as 128, 384, or 768.

## 18. Future Extensions

Potential future features:

- Full multi-layer HNSW.
- Metadata filtering.
- Binary snapshot format.
- HTTP API.
- gRPC and Protobuf API.
- Docker-based demo.
- Memory-mapped vector storage.
- Segment-based storage.
- Background compaction.
- Replication between nodes.
- Distributed sharding.

## 19. Final Project Narrative

VectorKV should be presented as a modern AI infrastructure project:

```text
VectorKV is a C++ in-memory vector database that implements the core components behind embedding search systems: vector storage, approximate nearest neighbor indexing, persistence, recovery, concurrency, and benchmark-driven evaluation.
```

The key resume narrative:

```text
This project demonstrates that I can build low-level C++ systems, reason about performance, implement non-trivial indexing algorithms, handle persistence and recovery, and evaluate tradeoffs using real benchmark metrics.
```
