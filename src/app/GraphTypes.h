#pragma once

#include <vector>

// Graph structure definitions for the molecule graph. Extracted (not
// duplicated) from IupacNamer.cpp, which now includes this header instead
// of defining these types itself -- a single shared definition, included
// by both IupacNamer.cpp and NamingValidation.cpp/.h.

struct GraphNode {
    int id;           // 0..N-1 index in heavy atom graph
    int indigoIdx;    // original indigo atom index
    int atomicNumber; // Z (6=C, 7=N, 8=O, 16=S, 9=F, 17=Cl, 35=Br, 53=I)
    int totalH;       // total hydrogen count (explicit + implicit)
    std::vector<int> neighbors; // neighbor indices in GraphNode array
    std::vector<int> bondOrders;// bond orders (1, 2, 3, 4=aromatic)
};

struct GraphBond {
    int u, v; // node indices
    int order; // 1, 2, 3, 4
};

struct Graph {
    std::vector<GraphNode> nodes;
    std::vector<GraphBond> bonds;
};
