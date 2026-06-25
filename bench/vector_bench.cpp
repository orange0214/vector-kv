#include <algorithm>
#include <chrono>
#include <iostream>
#include <random>
#include <string>
#include <vector>

#include "vectorkv/vector_db.h"

using vectorkv::VectorDB;

int main(int argc, char** argv) {
    // Default parameters
    int num_vectors = 10000;
    int dim = 128;
    int num_queries = 1000;
    int top_k = 10;

    // Parse command line arguments
    for (int i = 1; i < argc; ++i) {
        std::string arg = argv[i];
        if      (arg == "--vectors" && i + 1 < argc) num_vectors = std::stoi(argv[++i]);
        else if (arg == "--dim"     && i + 1 < argc) dim         = std::stoi(argv[++i]);
        else if (arg == "--queries" && i + 1 < argc) num_queries = std::stoi(argv[++i]);
        else if (arg == "--top_k"   && i + 1 < argc) top_k       = std::stoi(argv[++i]);
    }

    // initialize random seed
    std::mt19937 rng(42);
    std::uniform_real_distribution<float> dist(-1.0f, 1.0f);
    auto make_vec = [&]() {
        std::vector<float> v(dim);
        for (auto& x : v) x = dist(rng);
        return v;
    };

    // create the db and insert N vectors
    VectorDB db;
    for (int i = 0; i < num_vectors; ++i) {
        db.insert("id_" + std::to_string(i), make_vec());
    }

    // generate Q query vectors
    std::vector<std::vector<float>> queries;
    queries.reserve(num_queries);
    for (int i = 0; i < num_queries; ++i) {
        queries.push_back(make_vec());
    }

    // search all queries and measure the time taken.
    std::vector<double> latencies_us;
    latencies_us.reserve(num_queries);

    auto bench_start = std::chrono::high_resolution_clock::now();
    for (const auto& q : queries) {
        auto t0 = std::chrono::high_resolution_clock::now();
        auto results = db.search(q, top_k);
        auto t1 = std::chrono::high_resolution_clock::now();
        latencies_us.push_back(
            std::chrono::duration<double, std::micro>(t1 - t0).count()
        );
    }
    auto bench_end = std::chrono::high_resolution_clock::now();
    double total_s = std::chrono::duration<double>(bench_end - bench_start).count();

    // calculate qps
    std::sort(latencies_us.begin(), latencies_us.end());
    auto pct = [&](double p) {
        size_t idx = static_cast<size_t>(p * latencies_us.size());
        if (idx >= latencies_us.size()) idx = latencies_us.size() - 1;
        return latencies_us[idx] / 1000.0;
    };
    double qps = num_queries / total_s;

    // output
    std::cout << "vectors: " << num_vectors << "\n";
    std::cout << "dimension: " << dim << "\n";
    std::cout << "queries: " << num_queries << "\n";
    std::cout << "top_k: " << top_k << "\n";
    std::cout << "QPS: " << qps << "\n";
    std::cout << "P50 latency: " << pct(0.50) << " ms\n";
    std::cout << "P95 latency: " << pct(0.95) << " ms\n";
    std::cout << "P99 latency: " << pct(0.99) << " ms\n";

    return 0;
}