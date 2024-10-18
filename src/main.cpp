#include "MyGraph.h"
#include <iostream>
#include <cstdlib>  // For atof and atoi

int main(int argc, char* argv[]) {
    MyGraph myGraph;

    // Default configuration
    double resolution = 1.0;
    std::string partitionType = "CPM";
    int randomSeed = 42;
    int iterations = -1;

    // Parse command-line arguments
    for (int i = 1; i < argc; ++i) {
        std::string arg = argv[i];
        if ((arg == "--resolution" || arg == "-r") && i + 1 < argc) {
            resolution = atof(argv[++i]);
        } else if ((arg == "--partition" || arg == "-p") && i + 1 < argc) {
            partitionType = argv[++i];
        } else if ((arg == "--seed" || arg == "-s") && i + 1 < argc) {
            randomSeed = atoi(argv[++i]);
        } else if ((arg == "--iterations" || arg == "-i") && i + 1 < argc) {
            iterations = atoi(argv[++i]);
        } else if (arg == "--help" || arg == "-h") {
            std::cout << "Usage: clustering_app [options]\n"
                      << "Options:\n"
                      << "  --resolution, -r   Set the resolution parameter (default: 1.0)\n"
                      << "  --partition, -p    Set the partition type (CPM, RBConfiguration, Modularity)\n"
                      << "                     (default: CPM)\n"
                      << "  --seed, -s         Set the random seed (default: 42)\n"
                      << "  --iterations, -i   Set the number of iterations (-1 for convergence, default: -1)\n"
                      << "  --help, -h         Show this help message\n";
            return 0;
        } else {
            std::cerr << "Unknown argument: " << arg << std::endl;
            std::cerr << "Use --help or -h for usage information." << std::endl;
            return 1;
        }
    }

    // Set configuration options
    myGraph.setResolutionParameter(resolution);
    myGraph.setPartitionType(partitionType);
    myGraph.setRandomSeed(randomSeed);
    myGraph.setIterations(iterations);

    // Create the graph (Karate Club)
    myGraph.createGraph();

    // Apply Leiden algorithm
    myGraph.applyLeidenAlgorithm();

    // Print clusters
    myGraph.printClusters();

    return 0;
}
