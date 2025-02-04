#include <iostream>
#include "run_leiden.h"

int main() {
    std::cout << "Starting Leiden algorithm..." << std::endl;

    // Example graph
    int64_t src[] = {0, 1, 2, 3, 4, 5, 6, 7};
    int64_t dst[] = {1, 2, 3, 4, 5, 6, 7, 0};
    int64_t NumEdges = sizeof(src) / sizeof(src[0]);  // Number of edges
    int64_t NumNodes = 8; // Number of nodes

    // Array to store community assignments
    int64_t communities[NumNodes];

    // Choose modularity method (e.g., CPM)
    // run_leiden(src, dst, NumEdges, NumNodes, CPM, 0.1, communities);
    run_leiden(src, dst, NumEdges, NumNodes, static_cast<int64_t>(CPM), static_cast<float64_t>(0.1), communities);

    // Print community assignments
    std::cout << "Community assignments:" << std::endl;
    for (int64_t i = 0; i < NumNodes; i++) {
        std::cout << "Node " << i << " -> Community " << communities[i] << std::endl;
    }

    return 0;
}
