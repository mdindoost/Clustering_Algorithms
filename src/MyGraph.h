#ifndef MYGRAPH_H
#define MYGRAPH_H

#include <igraph/igraph.h>
#include <libleidenalg/GraphHelper.h>
#include <libleidenalg/Optimiser.h>
#include <libleidenalg/CPMVertexPartition.h>
#include <libleidenalg/RBConfigurationVertexPartition.h>
#include <libleidenalg/ModularityVertexPartition.h>
#include <vector>
#include <string>

class MyGraph {
public:
    MyGraph();
    ~MyGraph();

    void createGraph();
    void applyLeidenAlgorithm();
    void printClusters();

    // Setters for configuration options
    void setResolutionParameter(double resolution);
    void setPartitionType(const std::string& type);
    void setRandomSeed(int seed);
    void setIterations(int iterations);

private:
    igraph_t igraph;
    Graph* leidenGraph;
    std::vector<size_t> clusters;

    // Configuration parameters
    double resolutionParameter;
    std::string partitionType;
    int randomSeed;
    int iterations;
};

#endif // MYGRAPH_H
