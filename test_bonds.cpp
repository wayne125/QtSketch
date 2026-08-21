#include <iostream>
#include "indigo.h"

int main() {
    qword sid = indigoAllocSessionId();
    indigoSetSessionId(sid);
    
    int m = indigoLoadMoleculeFromString("c1nncccn1");
    
    std::cout << "Before aromatize:" << std::endl;
    int bondIter = indigoIterateBonds(m);
    if (bondIter >= 0) {
        int bondHandle = 0;
        while ((bondHandle = indigoNext(bondIter)) != 0) {
            int order = indigoBondOrder(bondHandle);
            std::cout << "  Bond order: " << order << std::endl;
            indigoFree(bondHandle);
        }
        indigoFree(bondIter);
    }
    
    indigoAromatize(m);
    
    std::cout << "\nAfter aromatize:" << std::endl;
    bondIter = indigoIterateBonds(m);
    if (bondIter >= 0) {
        int bondHandle = 0;
        while ((bondHandle = indigoNext(bondIter)) != 0) {
            int order = indigoBondOrder(bondHandle);
            std::cout << "  Bond order: " << order << std::endl;
            indigoFree(bondHandle);
        }
        indigoFree(bondIter);
    }
    
    indigoFree(m);
    indigoReleaseSessionId(sid);
    return 0;
}
