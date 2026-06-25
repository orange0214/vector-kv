# VectorKV

A C++ in-memory vector database with (planned) HNSW search and WAL recovery.

VectorKV implements the core mechanisms behind embedding-search systems:
vector storage, top-k similarity search, persistence, and benchmark-driven
evaluation. See `PRD.md` for the full design and roadmap.

> Status: **in-memory MVP**. Vector storage, cosine similarity, exact
> brute-force top-k search, and a unified `VectorDB` API are implemented and
> tested, with a CLI benchmark for QPS/latency. WAL recovery, snapshots, and
> the HNSW index are upcoming milestones.

## Features

- `insert` / `search` / `remove` via a single `VectorDB` API
- Cosine similarity over `std::vector<float>`
- Exact brute-force top-k search (priority-queue based), used as the
  correctness baseline and future Recall@K ground truth
- String IDs with arbitrary string metadata; dimension validation; soft delete
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
- [ ] WAL + snapshot persistence and crash recovery
- [ ] HNSW approximate nearest neighbor index
- [ ] Recall@K and HNSW-vs-brute-force comparison
- [ ] Concurrent search
