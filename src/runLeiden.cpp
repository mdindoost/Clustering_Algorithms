// runLeiden.cpp
#include "runLeiden.h"
#include <igraph/igraph.h>
#include <libleidenalg/GraphHelper.h>
#include <libleidenalg/Optimiser.h>
#include <libleidenalg/CPMVertexPartition.h>
#include <libleidenalg/RBConfigurationVertexPartition.h>
#include <libleidenalg/ModularityVertexPartition.h>
#include <memory>
#include <map>
#include <stdexcept>
#include <set>
#include <vector>

using namespace std;

/**
 * @brief Runs the Leiden community detection algorithm on a graph constructed from edge lists.
 */
void runLeiden(int64_t partition_arr[], 
              int64_t src[], 
              int64_t dst[], 
              int64_t n, 
              int64_t m,
              const string& partitionType, 
              double resolution, 
              int randomSeed, 
              int iterations,
              const vector<int64_t>& original_node_ids) {
    // Validate input sizes
    if (original_node_ids.size() != static_cast<size_t>(n)) {
        throw invalid_argument("Size of original_node_ids must be equal to n (number of nodes).");
    }

    // Create a mapping from original node IDs to internal node IDs (0 to n-1)
    map<int64_t, int64_t> original_to_internal_map;
    for(int64_t i = 0; i < n; ++i) {
        original_to_internal_map[original_node_ids[i]] = i;
    }

    // Initialize an empty undirected graph
    igraph_t graph;
    igraph_empty(&graph, 0, IGRAPH_UNDIRECTED);

    // Add vertices to the graph
    igraph_add_vertices(&graph, n, 0);

    // Prepare edge list for igraph (0-based indexing)
    igraph_vector_int_t edges;
    igraph_vector_int_init(&edges, 2 * m); // Each edge has a source and destination

    for(int64_t i = 0; i < m; ++i) {
        auto it_src = original_to_internal_map.find(src[i]);
        auto it_dst = original_to_internal_map.find(dst[i]);

        if(it_src == original_to_internal_map.end() || it_dst == original_to_internal_map.end()) {
            igraph_vector_int_destroy(&edges);
            igraph_destroy(&graph);
            throw invalid_argument("Source or destination node ID not found in original_node_ids.");
        }

        VECTOR(edges)[2*i] = it_src->second;
        VECTOR(edges)[2*i + 1] = it_dst->second;
    }

    // Add edges to the graph
    igraph_add_edges(&graph, &edges, 0);
    igraph_vector_int_destroy(&edges);

    // Set the attribute table to use C attributes
    igraph_set_attribute_table(&igraph_cattribute_table);

    // Assign a default weight of 1.0 to all edges
    for(int64_t i = 0; i < m; ++i) {
        double value = 1.0;
        igraph_cattribute_set_e_real(&graph, "weight", i, value);
    }

    // Create a unique_ptr to the Leiden GraphHelper
    unique_ptr<Graph> leidenGraph = make_unique<Graph>(&graph);
    if (!leidenGraph) {
        igraph_destroy(&graph);
        throw runtime_error("Failed to create Graph object for Leiden algorithm.");
    }

    // Initialize the optimiser
    Optimiser optimiser;
    optimiser.set_rng_seed(randomSeed);

    // Initialize the appropriate partition type
    unique_ptr<MutableVertexPartition> partition;
    if (partitionType == "CPM") {
        partition = make_unique<CPMVertexPartition>(leidenGraph.get(), resolution);
    } else if (partitionType == "RBConfiguration") {
        partition = make_unique<RBConfigurationVertexPartition>(leidenGraph.get(), resolution);
    } else if (partitionType == "Modularity") {
        partition = make_unique<ModularityVertexPartition>(leidenGraph.get());
    } else {
        igraph_destroy(&graph);
        throw invalid_argument("Unknown partition type: " + partitionType);
    }

    // Optimize the partition based on the number of iterations
    if (iterations > 0) {
        for(int i = 0; i < iterations; ++i) {
            double improvement = optimiser.optimise_partition(partition.get());
            // Optional: Uncomment the next line to see iteration progress
            // cout << "Iteration " << (i+1) << ", improvement: " << improvement << endl;
            if(improvement == 0.0) {
                break; // Convergence achieved
            }
        }
    } else {
        // Run until no further improvement
        while (true) {
            double improvement = optimiser.optimise_partition(partition.get());
            // Optional: Uncomment the next line to see iteration progress
            // cout << "Optimization iteration, improvement: " << improvement << endl;
            if(improvement == 0.0) {
                break; // Convergence achieved
            }
        }
    }

    // Retrieve cluster memberships
    map<int64_t, int64_t> cluster_map;
    igraph_eit_t eit_partition;
    igraph_eit_create(&graph, igraph_ess_all(IGRAPH_EDGEORDER_ID), &eit_partition);
    set<int64_t> visited;

    while (!IGRAPH_EIT_END(eit_partition)) {
        igraph_integer_t current_edge = IGRAPH_EIT_GET(eit_partition);
        int64_t from_node = IGRAPH_FROM(&graph, current_edge);
        int64_t to_node = IGRAPH_TO(&graph, current_edge);

        if (visited.find(from_node) == visited.end()) {
            visited.insert(from_node);
            cluster_map[from_node] = partition->membership(from_node);
        }
        if (visited.find(to_node) == visited.end()) {
            visited.insert(to_node);
            cluster_map[to_node] = partition->membership(to_node);
        }

        IGRAPH_EIT_NEXT(eit_partition);
    }
    igraph_eit_destroy(&eit_partition);

    // Ensure all nodes, including isolated ones, are included in the cluster map
    for(int64_t v = 0; v < igraph_vcount(&graph); ++v) {
        if(cluster_map.find(v) == cluster_map.end()) {
            cluster_map[v] = partition->membership(v);
        }
    }

    // Map cluster assignments back to original node IDs
    for(int64_t i = 0; i < n; ++i) {
        int64_t original_node_id = original_node_ids[i];
        int64_t internal_node_id = original_to_internal_map.at(original_node_id);
        partition_arr[i] = cluster_map[internal_node_id];
    }

    // Clean up the graph
    igraph_destroy(&graph);
}
