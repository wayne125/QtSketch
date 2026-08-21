#include <iostream>
#include <string>
#include <set>
#include "indigo.h"

int main() {
    std::string smiles = "O=C1C2CC3CC(C2)CC1C3";
    int m = indigoLoadMoleculeFromString(smiles.c_str());
    int ringCount = indigoCountSSSR(m);
    std::cout << "ringCount: " << ringCount << "\n";
    
    int sssrIter = indigoIterateSSSR(m);
    std::set<int> allSSSRNodes;
    int subMol = 0;
    while ((subMol = indigoNext(sssrIter)) != 0) {
        int ringAtomIter = indigoIterateAtoms(subMol);
        if (ringAtomIter >= 0) {
            int ringAtomHandle = 0;
            while ((ringAtomHandle = indigoNext(ringAtomIter)) != 0) {
                int idx = indigoIndex(ringAtomHandle);
                allSSSRNodes.insert(idx);
                indigoFree(ringAtomHandle);
            }
            indigoFree(ringAtomIter);
        }
        indigoFree(subMol);
    }
    indigoFree(sssrIter);
    
    std::cout << "allSSSRNodes size: " << allSSSRNodes.size() << "\n";
    return 0;
}
