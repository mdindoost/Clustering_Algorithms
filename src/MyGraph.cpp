#include "MyGraph.h"
#include <iostream>

MyGraph::MyGraph() {
    // Initialize the igraph_t structure
    igraph_empty(&igraph, 0, IGRAPH_UNDIRECTED);
    leidenGraph = nullptr;
}

MyGraph::~MyGraph() {
    igraph_destroy(&igraph);
    if (leidenGraph) {
        delete leidenGraph;
    }
}

void MyGraph::createGraph() {
    // Example: Create a Zachary's Karate Club graph
    igraph_famous(&igraph, "Zachary");
    std::cout << "Graph created with " << igraph_vcount(&igraph) << " vertices and "
              << igraph_ecount(&igraph) << " edges." << std::endl;
}

void MyGraph::applyLeidenAlgorithm() {
    // Create the Leiden graph object
    leidenGraph = new Graph(&igraph);

    // Set up the partition using CPM
    CPMVertexPartition partition(leidenGraph, 0.05);

    // Optimize the partition using the Leiden algorithm
    Optimiser optimiser;
    optimiser.optimise_partition(&partition);

    // Retrieve the clusters
    clusters = partition.membership();
}

void MyGraph::printClusters() {
    std::cout << "Clusters:" << std::endl;
    for (size_t i = 0; i < clusters.size(); ++i) {
        std::cout << "Vertex " << i << ": Cluster " << clusters[i] << std::endl;
    }
}

