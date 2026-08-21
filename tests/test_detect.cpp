#include "indigo.h"
#include <iostream>
#include <vector>

struct GraphNode {
    int id;
    int indigoIdx;
    int atomicNumber;
    std::vector<int> neighbors;
    std::vector<int> bondOrders;
};

int main() {
    qword sid = indigoAllocSessionId();
    indigoSetSessionId(sid);
    int mol = indigoLoadMoleculeFromString("CCCCC(=O)NN");
    
    std::vector<GraphNode> nodes;
    std::vector<int> heavyAtomIndices;
    int iter = indigoIterateAtoms(mol);
    int atom;
    while ((atom = indigoNext(iter)) != 0) {
        int idx = indigoIndex(atom);
        int z = indigoAtomicNumber(atom);
        if (z > 1) { heavyAtomIndices.push_back(idx); }
        indigoFree(atom);
    }
    indigoFree(iter);

    for (size_t i = 0; i < heavyAtomIndices.size(); ++i) {
        int idx = heavyAtomIndices[i];
        int aObj = indigoGetAtom(mol, idx);
        GraphNode n; n.id = i; n.indigoIdx = idx; n.atomicNumber = indigoAtomicNumber(aObj);
        int nIter = indigoIterateNeighbors(aObj);
        int nei;
        while ((nei = indigoNext(nIter)) != 0) {
            int nIdx = indigoIndex(nei);
            if (indigoAtomicNumber(nei) > 1) {
                for (size_t j = 0; j < heavyAtomIndices.size(); ++j) {
                    if (heavyAtomIndices[j] == nIdx) {
                        n.neighbors.push_back(j);
                        int bondHandle = indigoBond(nei);
                        n.bondOrders.push_back(indigoBondOrder(bondHandle));
                        indigoFree(bondHandle);
                        break;
                    }
                }
            }
            indigoFree(nei);
        }
        indigoFree(nIter);
        indigoFree(aObj);
        nodes.push_back(n);
    }

    for (size_t i = 0; i < nodes.size(); ++i) {
        if (nodes[i].atomicNumber == 6) {
            std::vector<int> singleN, doubleO;
            for (size_t j = 0; j < nodes[i].neighbors.size(); ++j) {
                int nei = nodes[i].neighbors[j];
                int z = nodes[nei].atomicNumber;
                int order = nodes[i].bondOrders[j];
                if (z == 8 && order == 2) doubleO.push_back(nei);
                if (z == 7 && order == 1) singleN.push_back(nei);
            }
            if (!doubleO.empty() && !singleN.empty()) {
                bool isHydrazide = false;
                for (int sN : singleN) {
                    for (size_t k = 0; k < nodes[sN].neighbors.size(); ++k) {
                        int nei = nodes[sN].neighbors[k];
                        if (nei != i && nodes[nei].atomicNumber == 7 && nodes[sN].bondOrders[k] == 1) {
                            if (nodes[nei].neighbors.size() == 1) {
                                isHydrazide = true; break;
                            } else {
                                std::cout << "N2 has neighbors: " << nodes[nei].neighbors.size() << "\n";
                            }
                        }
                    }
                }
                std::cout << "Carbon " << i << " isHydrazide: " << isHydrazide << "\n";
            }
        }
    }
    indigoFree(mol);
    indigoReleaseSessionId(sid);
}
