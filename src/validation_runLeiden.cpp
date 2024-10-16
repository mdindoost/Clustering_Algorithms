// validation_runLeiden.cpp
#include "runLeiden.h"
#include "metrics.h" // Include the metrics header
#include <iostream>
#include <vector>
#include <map>
#include <fstream> // For exporting cluster assignments

int main() {
    // Define the number of nodes and edges for the Karate Club network
    const int64_t n = 34; // Number of nodes
    const int64_t m = 78; // Number of edges

    // Define the edge lists (original node IDs)
    // Zachary's Karate Club network edges (1-based indexing)
    int64_t src[] = {
        1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, // Node 1 connections
        2, 2, 2, 2, // Node 2 connections
        3, 3, 3, 3, // Node 3 connections
        4, 4, 4, 4, // Node 4 connections
        5, // Node 5 connections
        6, 6, // Node 6 connections
        7, 7, 7, 7, // Node 7 connections
        8, 8, // Node 8 connections
        9, // Node 9 connections
        10, 10, 10, 10, // Node 10 connections
        11, 11, // Node 11 connections
        12, // Node 12 connections
        13, 13, 13, 13, // Node 13 connections
        15, // Node 15 connections
        16, 16, // Node 16 connections
        17, 17, 17, // Node 17 connections
        18, 18, 18, // Node 18 connections
        19, 19, 19, // Node 19 connections
        20, 20, 20, 20, // Node 20 connections
        21, 21, 21, // Node 21 connections
        22, 22, 22, 22, // Node 22 connections
        23, 23, 23, // Node 23 connections
        24, 24, 24, // Node 24 connections
        25, 25, 25, 25, // Node 25 connections
        26, 26, 26, // Node 26 connections
        27, 27, // Node 27 connections
        28, 28, // Node 28 connections
        29, 29, // Node 29 connections
        30, 30, // Node 30 connections
        31, 31, // Node 31 connections
        32, 32, // Node 32 connections
        33, 33, // Node 33 connections
        34 // Node 34 connections (assuming connection to node 33)
    };

    int64_t dst[] = {
        2, 3, 4, 5, 6, 7, 8, 10, 11, 12, 13, 17, 19, // Node 1 connections
        3, 4, 8, 14, // Node 2 connections
        4, 8, 9, 13, // Node 3 connections
        5, 8, 10, 14, // Node 4 connections
        33, // Node 5 connections
        7, 17, // Node 6 connections
        8, 17, 18, 20, // Node 7 connections
        9, 20, // Node 8 connections
        20, // Node 9 connections
        11, 12, 13, 17, // Node 10 connections
        12, 13, // Node 11 connections
        13, // Node 12 connections
        16, 17, 18, 20, // Node 13 connections
        16, // Node 15 connections
        17, 18, // Node 16 connections
        18, 19, 21, // Node 17 connections
        19, 21, 24, // Node 18 connections
        20, 21, 24, // Node 19 connections
        22, 23, 25, 27, // Node 20 connections
        22, 23, 24, // Node 21 connections
        23, 24, 26, 28, // Node 22 connections
        24, 25, 28, // Node 23 connections
        25, 28, 31, // Node 24 connections
        26, 28, 30, 32, // Node 25 connections
        27, 30, 32, // Node 26 connections
        28, 32, // Node 27 connections
        29, 32, // Node 28 connections
        32, 31, // Node 29 connections
        32, 31, // Node 30 connections
        32, 33, // Node 31 connections
        33, 34, // Node 32 connections
        34, // Node 33 connections
        // Node 34 is assumed to be connected to node 33 if applicable
    };

    // Define the original node IDs ordered by internal node IDs (1 to 34)
    // For this example, internal node ID 0 corresponds to original node ID 1, and so on.
    std::vector<int64_t> original_node_ids;
    for(int64_t i = 1; i <= n; ++i) {
        original_node_ids.push_back(i);
    }

    // Initialize the partition array (output)
    // Size should be equal to n, and the i-th element corresponds to the i-th original node ID
    std::vector<int64_t> partition_arr(n, 0); // Using vector for flexibility

    // Define clustering parameters
    std::string partitionType = "CPM"; // Options: "CPM", "RBConfiguration", "Modularity"
    double resolution = 0.2;            // Resolution parameter for CPM
    int randomSeed = 42;                // Seed for reproducibility
    int iterations = 10;                // -1 for running until convergence

    try {
        // Run the Leiden algorithm
        runLeiden(partition_arr.data(), src, dst, n, m, partitionType, resolution, randomSeed, iterations, original_node_ids);
    }
    catch(const std::exception& e) {
        std::cerr << "Error running Leiden algorithm: " << e.what() << std::endl;
        return 1;
    }

    // Define Ground Truth Labels
    // Community 1: Original Node IDs 1-17 (Internal Node IDs 0-16)
    // Community 2: Original Node IDs 18-34 (Internal Node IDs 17-33)
    std::vector<int64_t> ground_truth_labels(n, 0);
    for(int64_t i = 17; i < n; ++i) { // Internal Node IDs 17-33
        ground_truth_labels[i] = 1;
    }

    // Output the cluster assignments
    std::cout << "Cluster Assignments:" << std::endl;
    for(int64_t i = 0; i < n; ++i) {
        std::cout << "Original Node " << original_node_ids[i] << ": Cluster " << partition_arr[i] << std::endl;
    }

    // Compute Validation Metrics
    double ari = computeARI(ground_truth_labels, partition_arr);
    double nmi = computeNMI(ground_truth_labels, partition_arr);

    std::cout << "\nValidation Metrics:" << std::endl;
    std::cout << "Adjusted Rand Index (ARI): " << ari << std::endl;
    std::cout << "Normalized Mutual Information (NMI): " << nmi << std::endl;

    // Cluster assignments to a file
    std::ofstream outfile("cluster_assignments.csv");
    if(outfile.is_open()) {
        outfile << "OriginalNodeID,DetectedCluster,GroundTruthCluster\n";
        for(int64_t i = 0; i < n; ++i) {
            outfile << original_node_ids[i] << "," << partition_arr[i] << "," << ground_truth_labels[i] << "\n";
        }
        outfile.close();
        std::cout << "\nCluster assignments exported to cluster_assignments.csv" << std::endl;
    }
    else {
        std::cerr << "Unable to open file for writing cluster assignments." << std::endl;
    }

    return 0;
}
