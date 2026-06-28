# VectorKV

A C++ in-memory vector database with WAL crash recovery and (planned) HNSW search.

VectorKV implements the core mechanisms behind embedding-search systems:
vector storage, top-k similarity search, persistence, and benchmark-driven
evaluation. See `PRD.md` for the full design and roadmap.

> Status: **in-memory MVP with WAL + snapshot recovery**. Vector storage, cosine
> similarity, exact brute-force top-k search, and a unified `VectorDB` API are
> implemented and tested, plus optional WAL replay and snapshot save/load on
> restart. **Checkpoint (WAL truncate on snapshot) and automatic checkpoint**
> are documented next steps — see `PRD.md` §6.5. HNSW index is a later milestone.

## Features

- `insert` / `search` / `remove` via a single `VectorDB` API
- Cosine similarity over `std::vector<float>`
- Exact brute-force top-k search (priority-queue based), used as the
  correctness baseline and future Recall@K ground truth
- String IDs with arbitrary string metadata; dimension validation; soft delete
- Optional write-ahead log (WAL): operations are logged before mutating memory
  and replayed on restart (`VectorDB(wal_path)`)
- Optional snapshot save/load for full-state persistence (`save_snapshot` /
  `load_snapshot`; recovery loads snapshot then replays WAL)
- **Planned:** checkpoint truncates WAL after a successful snapshot so the log
  stays bounded; automatic checkpoint every N writes (see Persistence below)
- CLI benchmark reporting QPS and P50/P95/P99 latency
- Unit tests (GoogleTest) covering every module

## Requirements

- A C++20 compiler (Clang, GCC, or MSVC)
- CMake >= 3.20
- Git (used by CMake `FetchContent` to pull GoogleTest / Google Benchmark)
- Internet access for the first configure (dependencies are downloaded)

## Project layout

```text
VectorKV/
  CMakeLists.txt              # Root build: core library + options
  include/vectorkv/          # Public headers
    types.h                  #   VectorRecord, SearchResult
    distance.h               #   cosine_similarity
    vector_store.h           #   in-memory record storage
    brute_force_index.h      #   exact top-k search
    wal.h                    #   write-ahead log (append + replay)
    snapshot.h               #   full-state save/load
    vector_db.h              #   public insert/search/remove API
  src/                       # Core library sources (vectorkv_core)
  tests/                     # Unit tests (GoogleTest)
  bench/                     # CLI benchmark (vector_bench)
  examples/                  # Standalone example (basic_demo)
```

## Build

```bash
# Configure (downloads test/bench dependencies on first run)
cmake -S . -B build

# Build everything
cmake --build build -j
```

Build options (toggle with `-D<OPTION>=ON/OFF` at configure time):

| Option                      | Default | Description                    |
| --------------------------- | ------- | ------------------------------ |
| `VECTORKV_BUILD_TESTS`      | `ON`    | Build unit tests (GoogleTest)  |
| `VECTORKV_BUILD_BENCHMARKS` | `ON`    | Build the CLI benchmark        |
| `VECTORKV_BUILD_EXAMPLES`   | `ON`    | Build example executables      |

Example (library + tests only, faster configure):

```bash
cmake -S . -B build -DVECTORKV_BUILD_BENCHMARKS=OFF -DVECTORKV_BUILD_EXAMPLES=OFF
cmake --build build -j
```

## Usage

```cpp
#include "vectorkv/vector_db.h"

vectorkv::VectorDB db;

db.insert("doc1", {1.0f, 0.0f, 0.0f}, {{"title", "cpp notes"}});
db.insert("doc2", {0.9f, 0.1f, 0.0f}, {{"title", "systems design"}});

auto results = db.search({1.0f, 0.05f, 0.0f}, /*top_k=*/2);
for (const auto& r : results) {
    // r.id, r.score, r.metadata
}

db.remove("doc1");
```

To enable crash recovery, construct `VectorDB` with a WAL file path. Every
`insert`/`remove` is appended to the log before memory is mutated, and the log
is replayed on construction, so the data is restored after a restart:

```cpp
{
    vectorkv::VectorDB db("vectorkv.wal");
    db.insert("doc1", {1.0f, 0.0f, 0.0f}, {{"title", "cpp notes"}});
}   // process exits

// later / new process: same path replays the log and restores the data
vectorkv::VectorDB db("vectorkv.wal");
auto results = db.search({1.0f, 0.0f, 0.0f}, /*top_k=*/1);  // finds "doc1"
```

### Persistence: snapshot + WAL + checkpoint

With both WAL and snapshot paths, recovery is **snapshot first, then WAL tail**:

```cpp
vectorkv::VectorDB db("vectorkv.wal", "vectorkv.snapshot");
db.insert("doc1", {1.0f, 0.0f, 0.0f});
db.save_snapshot("vectorkv.snapshot");  // manual full-state dump
```

**Current behavior:** WAL is not truncated after `save_snapshot`, so the log can
still grow and recovery may replay the entire WAL (correct, but not optimal).

**Planned (Step A — checkpoint):** after a successful snapshot, truncate the
WAL so only operations *after* the checkpoint remain. Recovery then replays a
short log instead of full history.

```cpp
db.checkpoint();  // planned: save_snapshot + truncate WAL (snapshot must finish first)
```

**Planned (Step B — automatic checkpoint):** trigger checkpoint every N writes
without manual calls:

```cpp
db.set_auto_checkpoint_threshold(1000);  // planned: checkpoint after 1000 insert/remove ops
```

See `PRD.md` §6.5 and Week 3 deliverables for the full design.

## Run

```bash
# Unit tests
ctest --test-dir build --output-on-failure

# Example demo (insert -> search -> remove)
./build/bin/basic_demo

# Benchmark (defaults: 10000 vectors, dim 128, 1000 queries, top_k 10)
./build/bin/vectorkv_bench --vectors 5000 --dim 128 --queries 500 --top_k 10
```

Example demo output:

```text
Query = [1.0, 0.05, 0.0], top_k = 2
Results:
  id=doc1  score=0.998752  title="cpp notes"
  id=doc2  score=0.998158  title="systems design"
```

Example benchmark output (brute-force baseline; numbers depend on hardware):

```text
vectors: 5000
dimension: 128
queries: 500
top_k: 10
QPS: 2482.78
P50 latency: 0.365958 ms
P95 latency: 0.59075 ms
P99 latency: 0.705084 ms
```

These brute-force numbers are the baseline the upcoming HNSW index will be
compared against for speedup and Recall@K.

## Roadmap

- [x] CMake project, `vectorkv_core` library, GoogleTest setup
- [x] Cosine similarity, `VectorStore`, brute-force top-k, `VectorDB` API
- [x] CLI benchmark with QPS and latency percentiles
- [x] WAL append/replay and crash recovery (`VectorDB(wal_path)`)
- [x] Snapshot save/load and snapshot + WAL recovery tests
- [ ] Checkpoint: truncate WAL after successful snapshot (Step A)
- [ ] Automatic checkpoint every N writes (Step B)
- [ ] HNSW approximate nearest neighbor index
- [ ] Recall@K and HNSW-vs-brute-force comparison
- [ ] Concurrent search
