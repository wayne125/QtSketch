#include "indigo.h"
#include <iostream>

int main() {
    qword sid = indigoAllocSessionId();
    indigoSetSessionId(sid);
    int mol = indigoLoadMoleculeFromString("CCCCC(=O)NN");
    int iter = indigoIterateAtoms(mol);
    int atom;
    while ((atom = indigoNext(iter))) {
        int z = indigoAtomicNumber(atom);
        if (z == 1) { indigoFree(atom); continue; }
        std::cout << "Atom " << indigoIndex(atom) << " Z=" << z << "\n";
        int nIter = indigoIterateNeighbors(atom);
        int nei;
        while ((nei = indigoNext(nIter))) {
            int nAtom = indigoNeiVertex(nei);
            if (indigoAtomicNumber(nAtom) != 1) {
                std::cout << "  Nei " << indigoIndex(nAtom) << " Z=" << indigoAtomicNumber(nAtom) << "\n";
            }
            indigoFree(nei);
        }
        indigoFree(nIter);
        indigoFree(atom);
    }
    indigoFree(iter);
    indigoFree(mol);
    indigoReleaseSessionId(sid);
}
