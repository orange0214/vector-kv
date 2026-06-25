#include "vectorkv/brute_force_index.h"
#include "vectorkv/distance.h"
#include "vectorkv/types.h"
#include <queue>

namespace vectorkv {

std::vector<SearchResult> BruteForceIndex::search(
    const VectorStore& store,
    const std::vector<float>& query,
    int top_k
) const {
    std::vector<SearchResult> results;
    // return zero if top_k <= 0
    if (top_k <= 0) {
        return results;
    }

    // std::pair 相当于 Python 中的 tuple —— (float, str)。
    // 这里 Candidate 表示一个候选匹配对，包含相似度分数(float)和向量ID(string)。
    using Candidate = std::pair<float, std::string>;
    // 堆里存 (score, id)。pair 默认先比 first,正好按 score 比。
    // std::greater 把它变成最小堆:堆顶 = 当前 k 个里分数最小的那个。
    std::priority_queue<Candidate, std::vector<Candidate>, std::greater<Candidate>> heap;

    for (const auto& [id, record] : store.records()) {
        // check deleted
        if (record.deleted) {
            continue;
        }
        // check dimension
        if (record.vector.size() != query.size()) {
            continue;
        }

        float score = cosine_similarity(query, record.vector);

        // static_cast<size_t>(top_k) 的意思是：把 top_k（int 类型）强制转换为 size_t 类型，
        if (heap.size() < static_cast<size_t>(top_k)) {
            heap.emplace(score, id);
        } else if (score > heap.top().first) {
            heap.pop();
            heap.emplace(score, id);
        }
    }

    // 预分配 results 的容量为堆的大小，避免后续 push_back 时多次扩容。
    results.reserve(heap.size());
    while (!heap.empty()) {
        const auto& [score, id] = heap.top();
        SearchResult sr;
        sr.id = id;
        sr.score = score;
        if (const VectorRecord* rec = store.get(id)) {
            sr.metadata = rec->metadata;
        }
        // std::move(sr) 的意思是将 sr 的所有权移交给 results 向量，
        // 避免一次不必要的拷贝。move 本身不会复制数据，而是让 sr 变成“空壳”状态。
        // 这种用法提高了性能，尤其是 sr 持有大量数据时。
        results.push_back(std::move(sr));
        heap.pop();
    }
    std::reverse(results.begin(), results.end());

    return results;
}

}