#ifndef RUN_LEIDEN_H
#define RUN_LEIDEN_H

#include <cstdint> // for int64_t

// Enum to define different modularity options
enum ModularityType {
    CPM,
    MODULARITY,
    SIGNIFICANCE,
    SURPRISE
};

// Function to run Leiden clustering and store results in the `communities` array
void run_leiden(const int64_t src[], const int64_t dst[], int64_t NumEdges, int64_t NumNodes, 
                ModularityType modularity_option, double resolution, int64_t communities[]);

#endif // RUN_LEIDEN_H
