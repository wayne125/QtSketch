#include <iostream>
#include <string>
#include <vector>
#include "app/MoleculeGraph.h"
#include "app/SmiToGraph.h"
#include "indigo.h"

int main() {
    qword sid = indigoAllocSessionId();
    indigoSetSessionId(sid);
    std::string smiles = "O=C1CCC(=O)O1";
    MoleculeGraph g = smiToGraph(smiles);
    for (size_t i = 0; i < g.nodes.size(); ++i) {
        std::cout << "Node " << i << ": Z=" << g.nodes[i].atomicNumber << "\n";
        for (size_t j = 0; j < g.nodes[i].neighbors.size(); ++j) {
            std::cout << "  Nei " << g.nodes[i].neighbors[j] << " (order " << g.nodes[i].bondOrders[j] << ")\n";
        }
    }
    return 0;
}
