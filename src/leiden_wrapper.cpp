#include <iostream>
#include "igraph/igraph.h"
#include "libleidenalg/Optimiser.h"
#include "libleidenalg/CPMVertexPartition.h"
#include "libleidenalg/GraphHelper.h"  // Ensure this is included

void run_leiden() {
    // Create a simple graph (Zachary's Karate Club graph)
    igraph_t g;
    igraph_famous(&g, "Zachary");

    // Convert igraph_t to Leiden's Graph format
    Graph graph(&g);

    // Define a partition using CPM
    CPMVertexPartition part(&graph, 0.05);  // Pass Leiden's Graph

    // Run Leiden optimization
    Optimiser optimiser;
    optimiser.optimise_partition(&part);

    // Output results
    std::cout << "Leiden clustering complete." << std::endl;

    // Clean up
    igraph_destroy(&g);
}

// Add the main function
int main() {
    std::cout << "Starting Leiden algorithm..." << std::endl;
    run_leiden();
    return 0;
}
