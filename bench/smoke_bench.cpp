#include <benchmark/benchmark.h>

#include "vectorkv/version.h"

// Smoke benchmark: confirms the benchmark harness links against the core
// library. Real QPS / latency / recall benchmarks will be added later.
static void BM_Version(benchmark::State& state) {
    for (auto _ : state) {
        benchmark::DoNotOptimize(vectorkv::version());
    }
}
BENCHMARK(BM_Version);

BENCHMARK_MAIN();
