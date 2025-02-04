#ifndef RUN_LEIDEN_H
#define RUN_LEIDEN_H

#include <cstdint>

// Define float64_t explicitly if needed
typedef double float64_t;

// Modularity options as int64_t
enum ModularityType : int64_t {
    CPM,
    MODULARITY,
    SIGNIFICANCE,
    SURPRISE,
    RBCONFIGURATION,
    RBER
};

// Function to run Leiden clustering and store results in the `communities` array
void run_leiden(const int64_t src[], const int64_t dst[], int64_t NumEdges, int64_t NumNodes, 
                int64_t modularity_option, float64_t resolution, int64_t communities[], int64_t numCommunities);

#endif // RUN_LEIDEN_H
