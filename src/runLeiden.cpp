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
#include <iostream> // For debugging and logging

using namespace std;

/**
 * @brief Runs the Leiden community detection algorithm on a graph constructed from edge lists.
 *
 * @param partition_arr Array to store the resulting partition assignments.
 * @param src Array containing source node IDs for each edge.
 * @param dst Array containing destination node IDs for each edge.
 * @param n Number of nodes in the graph.
 * @param m Number of edges in the graph.
 * @param partitionType Type of partition to use ("CPM", "RBConfiguration", "Modularity").
 * @param resolution Resolution parameter for the partitioning algorithm.
 * @param randomSeed Seed for random number generation to ensure reproducibility.
 * @param iterations Maximum number of iterations for the optimization (use -1 for unlimited).
 * @param original_node_ids Vector containing the original node IDs.
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
    try {
        cout << "Starting runLeiden..." << endl;

        // 1. Validate Input Sizes
        cout << "Validating input sizes..." << endl;
        if (original_node_ids.size() != static_cast<size_t>(n)) {
            throw invalid_argument("Size of original_node_ids must be equal to n (number of nodes).");
        }
        if (src == nullptr || dst == nullptr) {
            throw invalid_argument("Source and destination arrays must not be null.");
        }

        // 2. Create Node ID Mappings
        cout << "Creating node ID mappings..." << endl;
        map<int64_t, int64_t> original_to_internal_map;
        for(int64_t i = 0; i < n; ++i) {
            original_to_internal_map[original_node_ids[i]] = i;
        }

        // 3. Set the Attribute Table BEFORE any graph operations
        cout << "Setting attribute table..." << endl;
        igraph_set_attribute_table(&igraph_cattribute_table);

        // 4. Initialize an Empty Undirected Graph
        cout << "Initializing igraph_t graph..." << endl;
        igraph_t graph;
        igraph_empty(&graph, 0, IGRAPH_UNDIRECTED);

        // 5. Add Vertices to the Graph
        cout << "Adding vertices..." << endl;
        igraph_add_vertices(&graph, n, 0); // Adds 'n' vertices with no attributes

        // 6. Prepare and Add Edge List (0-Based Indexing)
        cout << "Preparing edge list..." << endl;
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

            VECTOR(edges)[2*i] = it_src->second;     // Source vertex (0-based)
            VECTOR(edges)[2*i + 1] = it_dst->second; // Destination vertex (0-based)
        }

        // Add Edges to the Graph
        cout << "Adding edges to the graph..." << endl;
        igraph_add_edges(&graph, &edges, 0); // 0 indicates no edge attributes at this stage
        igraph_vector_int_destroy(&edges);    // Clean up the edge list vector

        // 7. Assign Edge Weights
        cout << "Assigning edge weights..." << endl;
        igraph_vector_t weight_vector;
        igraph_vector_init(&weight_vector, 0);      // Initialize vector with size 0
        igraph_vector_resize(&weight_vector, m);    // Resize to 'm' elements

        // Assign a default weight of 1.0 to all edges
        for(int64_t i = 0; i < m; ++i) {
            VECTOR(weight_vector)[i] = 1.0;
        }

        // Optionally, print the weight vector for debugging
        /*
        for(int64_t i = 0; i < m; ++i) {
            cout << "Edge " << i << " weight: " << VECTOR(weight_vector)[i] << endl;
        }
        */

        // Set the 'weight' edge attribute using the weight vector
        igraph_error_t err = igraph_cattribute_EAN_setv(&graph, "weight", &weight_vector);
        igraph_vector_destroy(&weight_vector); // Clean up the weight vector

        if(err != IGRAPH_SUCCESS){
            igraph_destroy(&graph);
            throw runtime_error("Failed to set 'weight' edge attributes.");
        }
        cout << "Edge weights assigned successfully." << endl;

        // 8. Graph Integrity Checks (Optional but Recommended)
        cout << "Verifying graph integrity..." << endl;

        // a. Number of Vertices and Edges
        cout << "Number of vertices: " << igraph_vcount(&graph) << endl;
        cout << "Number of edges: " << igraph_ecount(&graph) << endl;

        // b. List All Edges with Their Connections
        /*
        cout << "Listing all edges:" << endl;
        for(int64_t i = 0; i < igraph_ecount(&graph); ++i){
            igraph_integer_t from = 0, to = 0;
            igraph_edge(&graph, i, &from, &to);
            cout << "Edge " << i << ": " << from << " - " << to << endl;
        }
        */

        // c. Verify Edge Weights
        /*
        cout << "Verifying edge weights:" << endl;
        for(int64_t i = 0; i < igraph_ecount(&graph); ++i){
            double weight = igraph_cattribute_EAN(&graph, "weight", i);
            cout << "Edge " << i << " weight: " << weight << endl;
        }
        */

        // 9. Initialize Leiden GraphHelper
        cout << "Initializing Leiden GraphHelper..." << endl;
        unique_ptr<Graph> leidenGraph = make_unique<Graph>(&graph);
        if (!leidenGraph) {
            igraph_destroy(&graph);
            throw runtime_error("Failed to create Graph object for Leiden algorithm.");
        }
        cout << "Leiden GraphHelper initialized." << endl;

        // 10. Initialize the Optimiser
        cout << "Initializing Optimiser..." << endl;
        Optimiser optimiser;
        optimiser.set_rng_seed(randomSeed); // Set random seed for reproducibility

        // 11. Initialize the Appropriate Partition Type
        cout << "Initializing partition type: " << partitionType << endl;
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
        cout << "Partition initialized." << endl;

        // 12. Optimize the Partition
        cout << "Starting optimization..." << endl;
        if (iterations > 0) {
            for(int i = 0; i < iterations; ++i) {
                double improvement = optimiser.optimise_partition(partition.get());
                cout << "Iteration " << (i+1) << ", improvement: " << improvement << endl;
                if(improvement == 0.0) {
                    cout << "Convergence achieved after " << (i+1) << " iterations." << endl;
                    break; // Convergence achieved
                }
            }
        } else {
            // Run until no further improvement
            int iter = 0;
            while (true) {
                double improvement = optimiser.optimise_partition(partition.get());
                cout << "Optimization iteration " << (++iter) << ", improvement: " << improvement << endl;
                if(improvement == 0.0) {
                    cout << "Convergence achieved after " << iter << " iterations." << endl;
                    break; // Convergence achieved
                }
            }
        }
        cout << "Optimization completed." << endl;

        // 13. Retrieve Cluster Memberships
        cout << "Retrieving cluster memberships..." << endl;
        map<int64_t, int64_t> cluster_map;
        igraph_eit_t eit_partition;
        igraph_eit_create(&graph, igraph_ess_all(IGRAPH_EDGEORDER_ID), &eit_partition);
        set<int64_t> visited;

        while (!IGRAPH_EIT_END(eit_partition)) {
            igraph_integer_t current_edge = IGRAPH_EIT_GET(eit_partition);
            
            // Retrieve the source and target vertices of the current edge
            igraph_integer_t from_node, to_node;
            igraph_edge(&graph, current_edge, &from_node, &to_node);

            // Assign cluster memberships if not already assigned
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
        cout << "Cluster memberships retrieved." << endl;

        // 14. Ensure All Nodes, Including Isolated Ones, Are Included in the Cluster Map
        cout << "Ensuring all nodes are in cluster map..." << endl;
        for(int64_t v = 0; v < igraph_vcount(&graph); ++v) {
            if(cluster_map.find(v) == cluster_map.end()) {
                cluster_map[v] = partition->membership(v);
            }
        }
        cout << "All nodes are included in cluster map." << endl;

        // 15. Map Cluster Assignments Back to Original Node IDs
        cout << "Mapping cluster assignments back to original node IDs..." << endl;
        for(int64_t i = 0; i < n; ++i) {
            int64_t original_node_id = original_node_ids[i];
            int64_t internal_node_id = original_to_internal_map.at(original_node_id);
            partition_arr[i] = cluster_map[internal_node_id];
        }
        cout << "Cluster assignments mapped." << endl;

        // 16. Clean Up the Graph
        cout << "Cleaning up igraph graph..." << endl;
        igraph_destroy(&graph);
        cout << "runLeiden completed successfully." << endl;
    } catch (const std::exception &e) {
        cerr << "Exception in runLeiden: " << e.what() << endl;
    }
}
