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
#include <map>
#include <memory>

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
    void setAllEdgesWeight(double weight); // Optional for now: Set weights


private:
    igraph_t igraph_;
    std::unique_ptr<Graph> leidenGraph_;
    std::map<int, int> partitionMap_;

    // Configuration parameters
    double resolutionParameter_;
    std::string partitionType_;
    int randomSeed_;
    int iterations_;
};
#endif // MYGRAPH_H
