include(FetchContent)

# ---------------------------------------------------------------------------
# Google Benchmark
# ---------------------------------------------------------------------------
FetchContent_Declare(
    benchmark
    GIT_REPOSITORY https://github.com/google/benchmark.git
    GIT_TAG        v1.9.1
    GIT_SHALLOW    TRUE
)

# Do not build Google Benchmark's own tests.
set(BENCHMARK_ENABLE_TESTING OFF CACHE BOOL "" FORCE)
set(BENCHMARK_ENABLE_INSTALL OFF CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(benchmark)

# ---------------------------------------------------------------------------
# Benchmark target
#
# Add new *_bench.cpp files / targets as benchmarks are implemented.
# ---------------------------------------------------------------------------
add_executable(vectorkv_bench
    vector_bench.cpp

)

target_link_libraries(vectorkv_bench
    PRIVATE
        VectorKV::core
)
