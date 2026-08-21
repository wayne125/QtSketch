#include "indigo.h"
#include <iostream>
int main() {
    qword sid = indigoAllocSessionId();
    indigoSetSessionId(sid);
    const char* smiles[] = {"N1C=CC=CC1", "C1=CC=CCN1", "C1C=CC=CN1", "C1=CC=C(C)N1", "N1=CCCCC=C1", "C1CC=CC=CN1", "c1ccncc1", "c1cc[nH]c1"};
    for(int i=0; i<8; ++i) {
        int m = indigoLoadMoleculeFromString(smiles[i]);
        if (m < 0) std::cout << smiles[i] << " error: " << indigoGetLastError() << "\n";
        else std::cout << smiles[i] << " OK\n";
    }
    return 0;
}
