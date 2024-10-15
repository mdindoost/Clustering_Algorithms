// example_runLeiden.cpp
#include "runLeiden.h"
#include <iostream>
#include <vector>
#include <map>

int main() {
    // Define the number of nodes and edges
    int64_t n = 5; // Number of nodes
    int64_t m = 4; // Number of edges

    // Define the edge lists (original node IDs)
    // Example graph:
    // 1 - 2
    // 2 - 3
    // 3 - 4
    // 4 - 5
    int64_t src[] = {1, 2, 3, 4};
    int64_t dst[] = {2, 3, 4, 5};

    // Define the original node IDs ordered by internal node IDs (0 to n-1)
    // For this example, internal node ID 0 corresponds to original node ID 1, and so on.
    std::vector<int64_t> original_node_ids = {1, 2, 3, 4, 5};

    // Initialize the partition array (output)
    // Size should be equal to n, and the i-th element corresponds to the i-th original node ID
    int64_t partition_arr[n] = {0};

    // Define clustering parameters
    std::string partitionType = "CPM"; // Options: "CPM", "RBConfiguration", "Modularity"
    double resolution = 1.0;            // Resolution parameter for CPM
    int randomSeed = 42;                // Seed for reproducibility
    int iterations = -1;                // -1 for running until convergence

    try {
        // Run the Leiden algorithm
        runLeiden(partition_arr, src, dst, n, m, partitionType, resolution, randomSeed, iterations, original_node_ids);
    }
    catch(const std::exception& e) {
        std::cerr << "Error running Leiden algorithm: " << e.what() << std::endl;
        return 1;
    }

    // Output the cluster assignments
    std::cout << "Cluster Assignments:" << std::endl;
    for(int64_t i = 0; i < n; ++i) {
        std::cout << "Original Node " << original_node_ids[i] << ": Cluster " << partition_arr[i] << std::endl;
    }

    return 0;
}
