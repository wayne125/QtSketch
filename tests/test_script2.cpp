#include "indigo.h"
#include <iostream>

int main() {
    qword sid = indigoAllocSessionId();
    indigoSetSessionId(sid);
    int mol = indigoLoadMoleculeFromString("O=C(NN)C1CCCCC1");
    std::cout << indigoCanonicalSmiles(mol) << std::endl;
    indigoFree(mol);
    indigoReleaseSessionId(sid);
}
