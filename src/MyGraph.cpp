#include "MyGraph.h"
#include <iostream>
#include <cstdlib>  // For srand()
#include <map>
#include <set>

MyGraph::MyGraph()
    : leidenGraph_(nullptr),
      resolutionParameter_(1.0),    // Default resolution parameter
      partitionType_("CPM"),        // Default partition type
      randomSeed_(42),              // Default random seed
      iterations_(-1)               // Default: run until convergence
{
    // Initialize the igraph_t structure as an empty undirected graph
    igraph_empty(&igraph_, 0, IGRAPH_UNDIRECTED);
}

MyGraph::~MyGraph() {
    igraph_destroy(&igraph_);
    // No need to delete leidenGraph_; unique_ptr handles it
}

void MyGraph::printClusters() {
    std::cout << "Clusters:" << std::endl;
    for (const auto& [vertex, cluster] : partitionMap_) {
    std::cout << "Vertex " << vertex << ":\t" << cluster << std::endl;
    }
}

void MyGraph::setResolutionParameter(double resolution) {
    resolutionParameter_ = resolution;
}

void MyGraph::setPartitionType(const std::string& type) {
    partitionType_ = type;
}

void MyGraph::setRandomSeed(int seed) {
    randomSeed_ = seed;
}

void MyGraph::setIterations(int iter) {
    iterations_ = iter;
}

void MyGraph::createGraph() {
    // Example: Create Zachary's Karate Club graph
    igraph_famous(&igraph_, "Zachary");
    std::cout << "Graph created with " << igraph_vcount(&igraph_) << " vertices and "
              << igraph_ecount(&igraph_) << " edges." << std::endl;
}

void MyGraph::setAllEdgesWeight(double weight) {
    igraph_eit_t eit;
    if (IGRAPH_SUCCESS != igraph_eit_create(&igraph_, igraph_ess_all(IGRAPH_EDGEORDER_ID), &eit)) {
        std::cerr << "Error creating edge iterator for setting weights." << std::endl;
        return;
    }

    while (!IGRAPH_EIT_END(eit)) {
        igraph_integer_t edge_id = IGRAPH_EIT_GET(eit);
        // Assuming SETEAN is a macro or function to set edge attributes
        // Replace with the actual method to set edge attributes in your environment
        SETEAN(&igraph_, "weight", edge_id, weight);
        IGRAPH_EIT_NEXT(eit);
    }
    igraph_eit_destroy(&eit);
}

void MyGraph::applyLeidenAlgorithm() {
    std::cout << "********************************* Leiden started" << std::endl;

    try {
        // Initialize leidenGraph using smart pointers
        leidenGraph_ = std::make_unique<Graph>(&igraph_);
        if (!leidenGraph_) {
            throw std::runtime_error("Failed to create Graph object for Leiden algorithm.");
        }

        // Initialize and set up optimizer
        Optimiser optimiser;
        optimiser.set_rng_seed(randomSeed_);

        // Choose the partition type
        std::unique_ptr<MutableVertexPartition> partition;
        if (partitionType_ == "CPM") {
            partition = std::make_unique<CPMVertexPartition>(leidenGraph_.get(), resolutionParameter_);
        } else if (partitionType_ == "RBConfiguration") {
            partition = std::make_unique<RBConfigurationVertexPartition>(leidenGraph_.get(), resolutionParameter_);
        } else if (partitionType_ == "Modularity") {
            partition = std::make_unique<ModularityVertexPartition>(leidenGraph_.get());
        } else {
            throw std::invalid_argument("Unknown partition type: " + partitionType_);
        }

        // Optimize the partition
        if (iterations_ > 0) {
            for (int i = 0; i < iterations_; ++i) {
                double improvement = optimiser.optimise_partition(partition.get());
                std::cout << "Iteration " << (i + 1) << ", improvement: " << improvement << std::endl;
                if (improvement == 0.0) {
                    std::cout << "No further improvement; stopping early." << std::endl;
                    break;
                }
            }
        } else {
            while (true) {
                double improvement = optimiser.optimise_partition(partition.get());
                std::cout << "Optimization iteration, improvement: " << improvement << std::endl;
                if (improvement == 0.0) {
                    std::cout << "No further improvement; optimization converged." << std::endl;
                    break;
                }
            }
        }

        // Retrieve and store clusters
        //clusters = partition->membership();

        // Update the partition map
        std::map<int, int> partition_map;
        igraph_eit_t eit;
        if (IGRAPH_SUCCESS != igraph_eit_create(&igraph_, igraph_ess_all(IGRAPH_EDGEORDER_ID), &eit)) {
            throw std::runtime_error("Failed to create edge iterator.");
        }
        std::set<int> visited;

        while (!IGRAPH_EIT_END(eit)) {
            igraph_integer_t current_edge = IGRAPH_EIT_GET(eit);
            int from_node = IGRAPH_FROM(&igraph_, current_edge);
            int to_node = IGRAPH_TO(&igraph_, current_edge);

            if (visited.find(from_node) == visited.end()) {
                visited.insert(from_node);
                partition_map[from_node] = partition->membership(from_node);
            }
            if (visited.find(to_node) == visited.end()) {
                visited.insert(to_node);
                partition_map[to_node] = partition->membership(to_node);
            }

            IGRAPH_EIT_NEXT(eit);
        }
        igraph_eit_destroy(&eit);

        // Include isolated nodes*******************************
        for (int v = 0; v < igraph_vcount(&igraph_); ++v) {
            if (partition_map.find(v) == partition_map.end()) {
                partition_map[v] = partition->membership(v);
            }
        }

        // Store the partition map
        partitionMap_ = partition_map;
    }
    catch (const std::exception& e) {
        std::cerr << "Error in applyLeidenAlgorithm: " << e.what() << std::endl;
        // Do we need Additional error handling or cleanup?
    }

    // No need to manually delete 'partition'; smart pointers handle it
}
