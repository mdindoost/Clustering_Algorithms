// example_runLeiden.cpp
#include "runLeiden.h"
#include <iostream>
#include <vector>
#include <map>

int main() {

   
    // Define the number of nodes and edges
    // int64_t n = 5; // Number of nodes
    // int64_t m = 4; // Number of edges

    const int64_t n = 34; // Number of nodes
    const int64_t m = 78; // Number of edges

    // Define the edge lists (original node IDs)
    // Example graph:

    // int64_t src[] = {1, 2, 3, 4};
    // int64_t dst[] = {2, 3, 4, 5};

    // Karate Club network edges (1-based indexing FOR TEST)//From net
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
        34 // Node 34 connections (isolated or connected as per the dataset)
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



    // Define the original node IDs ordered by internal node IDs (0 to n-1)
    // For this example, internal node ID 0 corresponds to original node ID 1, and so on.
    
    
    //std::vector<int64_t> original_node_ids = {1, 2, 3, 4, 5};
    std::vector<int64_t> original_node_ids;
    for(int64_t i = 1; i <= n; ++i) {
        original_node_ids.push_back(i);
    }

    // Initialize the partition array (output)
    // Size should be equal to n, and the i-th element corresponds to the i-th original node ID
    //To OLIVER: I did it here for check but I think we should do it from **inside Arachne**. not here. 
    int64_t partition_arr[n];
    for(int64_t i = 0; i < n; ++i) {
        partition_arr[i] = 0; // Initialize all partitions to 0
    }
    // Define clustering parameters
    //std::string partitionType = "CPM"; // Options: "CPM", "RBConfiguration", "Modularity"
    //std::string partitionType = "RBConfiguration"; 
    std::string partitionType = "Modularity"; 
    double resolution = 0.001;            // Resolution parameter for CPM
    int randomSeed = 42;                // Seed for reproducibility
    //int iterations = -1;                // -1 for running until convergence
    int iterations = 100;                // -1 for running until convergence

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
