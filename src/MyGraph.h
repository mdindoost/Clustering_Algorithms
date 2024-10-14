#ifndef MYGRAPH_H
#define MYGRAPH_H

#include <igraph/igraph.h>
#include <libleidenalg/Optimiser.h>
#include <libleidenalg/CPMVertexPartition.h>

class MyGraph {
public:
    MyGraph();
    ~MyGraph();

    void createGraph();
    void applyLeidenAlgorithm();
    void printClusters();

private:
    igraph_t igraph;
    Graph* leidenGraph;
    std::vector<size_t> clusters;
};

#endif // MYGRAPH_H

