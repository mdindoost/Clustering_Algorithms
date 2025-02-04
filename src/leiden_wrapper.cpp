#include <iostream>
#include "run_leiden.h"

void test_modularity_option(int64_t modularity_option, const std::string& name) {
    std::cout << "Testing Leiden algorithm with " << name << "..." << std::endl;

    // Example graph
    int64_t src[] = {0, 1, 2, 3, 4, 5, 6, 7};
    int64_t dst[] = {1, 2, 3, 4, 5, 6, 7, 0};
    int64_t NumEdges = sizeof(src) / sizeof(src[0]);  // Number of edges
    int64_t NumNodes = 8; // Number of nodes

    // Array to store community assignments
    int64_t communities[NumNodes];

    // Run Leiden with the selected modularity option
    run_leiden(src, dst, NumEdges, NumNodes, modularity_option, static_cast<double>(0.1), communities);

    // Print community assignments
    std::cout << name << " Community Assignments:" << std::endl;
    for (int64_t i = 0; i < NumNodes; i++) {
        std::cout << "Node " << i << " -> Community " << communities[i] << std::endl;
    }
    std::cout << "----------------------------------------\n";
}

int main() {
    std::cout << "Starting Leiden algorithm tests..." << std::endl;

    test_modularity_option(CPM, "CPM");
    test_modularity_option(MODULARITY, "Modularity");
    test_modularity_option(SIGNIFICANCE, "Significance");
    test_modularity_option(SURPRISE, "Surprise");
    test_modularity_option(RBCONFIGURATION, "RBConfiguration");
    test_modularity_option(RBER, "RBER");

    std::cout << "All modularity tests completed!" << std::endl;
    return 0;
}
