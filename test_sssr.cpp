#include "indigo.h"
#include <iostream>
#include <vector>
#include <set>

int main() {
    int m = indigoLoadMoleculeFromString("C1C2C=CC1C3=CC=CC=C23");
    indigoAromatize(m);
    int sssrIter = indigoIterateSSSR(m);
    int subMol;
    std::vector<std::set<int>> rings;
    while ((subMol = indigoNext(sssrIter)) != 0) {
        std::set<int> rAtoms;
        int ringAtomIter = indigoIterateAtoms(subMol);
        int ringAtomHandle;
        while ((ringAtomHandle = indigoNext(ringAtomIter)) != 0) {
            rAtoms.insert(indigoIndex(ringAtomHandle));
            indigoFree(ringAtomHandle);
        }
        indigoFree(ringAtomIter);
        rings.push_back(rAtoms);
        indigoFree(subMol);
    }
    indigoFree(sssrIter);
    
    for (size_t i = 0; i < rings.size(); ++i) {
        for (size_t j = i + 1; j < rings.size(); ++j) {
            int shared = 0;
            for (int n : rings[i]) if (rings[j].count(n)) shared++;
            std::cout << "Ring " << i << " and " << j << " share " << shared << " atoms\n";
        }
    }
    return 0;
}
