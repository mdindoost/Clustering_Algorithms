#include "MyGraph.h"
#include <iostream>
#include <cstdlib>  // For srand()

MyGraph::MyGraph()
    : leidenGraph(nullptr),
      resolutionParameter(1.0),    // Default resolution parameter
      partitionType("CPM"),        // Default partition type
      randomSeed(42),              // Default random seed
      iterations(-1)               // Default: run until convergence
{
    // Initialize the igraph_t structure
    igraph_empty(&igraph, 0, IGRAPH_UNDIRECTED);
}

MyGraph::~MyGraph() {
    igraph_destroy(&igraph);
    if (leidenGraph) {
        delete leidenGraph;
    }
}
void MyGraph::printClusters() {
    std::cout << "Clusters:" << std::endl;
    for (size_t i = 0; i < clusters.size(); ++i) {
        std::cout << "Vertex " << i << ": Cluster " << clusters[i] << std::endl;
    }
}
void MyGraph::setResolutionParameter(double resolution) {
    resolutionParameter = resolution;
}

void MyGraph::setPartitionType(const std::string& type) {
    partitionType = type;
}

void MyGraph::setRandomSeed(int seed) {
    randomSeed = seed;
}

void MyGraph::setIterations(int iter) {
    iterations = iter;
}

void MyGraph::createGraph() {
    // Example: Create Zachary's Karate Club graph
    igraph_famous(&igraph, "Zachary");
    std::cout << "Graph created with " << igraph_vcount(&igraph) << " vertices and "
              << igraph_ecount(&igraph) << " edges." << std::endl;
}

void MyGraph::applyLeidenAlgorithm() {
    // Create the Leiden graph object
    leidenGraph = new Graph(&igraph);

    // Set random seed
    srand(randomSeed);

    // Choose the partition type based on the configuration
    MutableVertexPartition* partition = nullptr;

    if (partitionType == "CPM") {
        partition = new CPMVertexPartition(leidenGraph, resolutionParameter);
    } else if (partitionType == "RBConfiguration") {
        partition = new RBConfigurationVertexPartition(leidenGraph, resolutionParameter);
    } else if (partitionType == "Modularity") {
        partition = new ModularityVertexPartition(leidenGraph);
    } else {
        std::cerr << "Unknown partition type: " << partitionType << std::endl;
        return;
    }

    // Optimize the partition using the Leiden algorithm
    Optimiser optimiser;
    optimiser.set_rng_seed(randomSeed);

    // Manually control the number of iterations
    if (iterations > 0) {
        for (int i = 0; i < iterations; ++i) {
            double improvement = optimiser.optimise_partition(partition);
            std::cout << "Iteration " << (i + 1) << ", improvement: " << improvement << std::endl;
            // Optionally, check for convergence
            if (improvement == 0.0) {
                std::cout << "No further improvement; stopping early." << std::endl;
                break;
            }
        }
    } else {
        // Run until convergence
        double improvement = optimiser.optimise_partition(partition);
        std::cout << "Optimisation complete, total improvement: " << improvement << std::endl;
    }

    // Retrieve the clusters
    clusters = partition->membership();

    // Clean up
    delete partition;
}
