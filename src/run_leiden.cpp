#include <iostream>
#include "igraph/igraph.h"
#include "libleidenalg/Optimiser.h"
#include "libleidenalg/CPMVertexPartition.h"
#include "libleidenalg/ModularityVertexPartition.h"
#include "libleidenalg/SignificanceVertexPartition.h"
#include "libleidenalg/SurpriseVertexPartition.h"
#include "run_leiden.h"

void run_leiden(const int64_t src[], const int64_t dst[], int64_t NumEdges, int64_t NumNodes, 
                ModularityType modularity_option, double resolution, int64_t communities[]) {
    
    igraph_t g;
    igraph_vector_int_t edges;
    igraph_vector_int_init(&edges, NumEdges * 2);

    // Fill igraph edges
    for (int64_t i = 0; i < NumEdges; i++) {
        VECTOR(edges)[2 * i] = src[i];
        VECTOR(edges)[2 * i + 1] = dst[i];
    }

    // Create the graph
    igraph_create(&g, &edges, NumNodes, IGRAPH_DIRECTED);
    igraph_vector_int_destroy(&edges);

    // Convert igraph_t to Leiden's Graph format
    Graph graph(&g);

    // Choose the modularity method
    MutableVertexPartition* partition = nullptr;
    switch (modularity_option) {
        case CPM:
            partition = new CPMVertexPartition(&graph, resolution);
            break;
        case MODULARITY:
            partition = new ModularityVertexPartition(&graph);
            break;
        case SIGNIFICANCE:
            partition = new SignificanceVertexPartition(&graph);
            break;
        case SURPRISE:
            partition = new SurpriseVertexPartition(&graph);
            break;
        default:
            std::cerr << "Error: Invalid modularity option selected." << std::endl;
            igraph_destroy(&g);
            return;
    }

    // Run Leiden optimization
    Optimiser optimiser;
    optimiser.optimise_partition(partition);

    // Store results in the provided `communities[]` array
    for (int64_t i = 0; i < NumNodes; i++) {
        communities[i] = partition->membership(i);
    }

    std::cout << "Leiden clustering complete." << std::endl;

    // Clean up
    delete partition;
    igraph_destroy(&g);
}
