# VectorKV

A C++ in-memory vector database with HNSW search and WAL recovery.

> Status: **project skeleton**. The CMake build framework, dependency wiring
> (GoogleTest + Google Benchmark), and target structure are in place. Database
> business logic (VectorStore, brute-force search, HNSW, WAL, snapshot,
> metrics) will be added in later milestones — see `PRD.md`.

## Requirements

- A C++20 compiler (Clang, GCC, or MSVC)
- CMake >= 3.20
- Git (used by CMake `FetchContent` to pull GoogleTest and Google Benchmark)
- Internet access for the first configure (dependencies are downloaded)

## Project layout

```text
VectorKV/
  CMakeLists.txt        # Root build: core library + options
  include/vectorkv/     # Public headers
  src/                  # Core library sources
  tests/                # Unit tests (GoogleTest)
  bench/                # Benchmarks (Google Benchmark)
  examples/             # Standalone example executables
```

## Build

```bash
# Configure (downloads GoogleTest + Google Benchmark on first run)
cmake -S . -B build

# Build everything
cmake --build build -j
```

Build options (toggle with `-D<OPTION>=ON/OFF` at configure time):

| Option                    | Default | Description                    |
| ------------------------- | ------- | ------------------------------ |
| `VECTORKV_BUILD_TESTS`      | `ON`    | Build unit tests (GoogleTest)  |
| `VECTORKV_BUILD_BENCHMARKS` | `ON`    | Build benchmarks               |
| `VECTORKV_BUILD_EXAMPLES`   | `ON`    | Build example executables      |

Example (library + tests only, faster configure):

```bash
cmake -S . -B build -DVECTORKV_BUILD_BENCHMARKS=OFF -DVECTORKV_BUILD_EXAMPLES=OFF
cmake --build build -j
```

## Run

```bash
# Unit tests
ctest --test-dir build --output-on-failure

# Example
./build/bin/basic_demo

# Benchmarks
./build/bin/vectorkv_bench
```
