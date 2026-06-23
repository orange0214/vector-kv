#include <iostream>

#include "vectorkv/version.h"

// Minimal example confirming the core library links and runs.
// Real insert / search / delete demos will be added in later milestones.
int main() {
    std::cout << "VectorKV version: " << vectorkv::version() << '\n';
    return 0;
}
