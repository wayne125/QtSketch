#include <iostream>
#include <vector>
#include <map>
#include "indigo.h"

int main() {
    int indigo = indigoAllocSessionId();
    indigoSetSessionId(indigo);
    
    auto check = [](const char* smiles, const char* name) {
        int m = indigoLoadMoleculeFromString(smiles);
        indigoAromatize(m);
        int ringCount = indigoCountSSSR(m);
        int atomIter = indigoIterateAtoms(m);
        int numC = 0;
        while(int a = indigoNext(atomIter)) {
            if(indigoAtomicNumber(a) == 6) numC++;
            indigoFree(a);
        }
        indigoFree(atomIter);
        std::cout << name << " " << smiles << " ringCount=" << ringCount << " numC=" << numC << "\n";
        indigoFree(m);
    };
    
    check("C1C2CC3CC1CC(C2)C3", "adamantane");
    check("C12C3C4C1C5C2C3C45", "cubane");
    
    indigoFreeSessionId(indigo);
    return 0;
}
