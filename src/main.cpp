#include "MyGraph.h"

int main() {
    MyGraph myGraph;

    myGraph.createGraph();
    myGraph.applyLeidenAlgorithm();
    myGraph.printClusters();

    return 0;
}

