#include <iostream>
#include <vector>

#include "vectorkv/version.h"
#include "vectorkv/vector_db.h"

// Minimal example confirming the core library links and runs.
// Real insert / search / delete demos will be added in later milestones.
int main() {
    std::cout << "VectorKV version: " << vectorkv::version() << "\n\n";

    vectorkv::VectorDB db;

    // 1. insert 3 dimensional vectors with metadata
    db.insert("doc1", {1.0f, 0.0f, 0.0f}, {{"title", "cpp notes"}});
    db.insert("doc2", {0.9f, 0.1f, 0.0f}, {{"title", "systems design"}});
    db.insert("doc3", {0.0f, 1.0f, 0.0f}, {{"title", "cooking"}});
    db.insert("doc4", {0.0f, 0.0f, 1.0f}, {{"title", "travel"}});
    std::cout << "Inserted 4 vectors.\n\n";

    // 2. query top-2 similarity vectors
    std::vector<float> query = {1.0f, 0.05f, 0.0f};
    int top_k = 2;
    std::cout << "Query = [1.0, 0.05, 0.0], top_k = " << top_k << "\n";

    auto results = db.search(query, top_k);

    // 3. output result
    std::cout << "Results:\n";
    for (const auto& r: results) {
        std::cout << "  id=" << r.id
                  << "  score=" << r.score;
        auto it = r.metadata.find("title");
        if (it != r.metadata.end()) {
            std::cout << "  title=\"" << it->second << "\"";
        }
        std::cout << "\n";
    }

    // 4. delete most similarity vector and do the query again
    std::cout << "\nRemoving doc1, searching again...\n";
    db.remove("doc1");
    for (const auto& r : db.search(query, top_k)) {
        std::cout << "  id=" << r.id << "  score=" << r.score << "\n";
    }
    
    return 0;
}