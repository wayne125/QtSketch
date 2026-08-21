#include "IupacNamer.h"
#include "indigo.h"
#include <iostream>

int main() {
    qword sid = indigoAllocSessionId();
    indigoSetSessionId(sid);
    int mol = indigoLoadMoleculeFromString("NNC(=O)C1CCCCC1");
    IupacResult res = IupacNamer::generateName(mol);
    std::cout << res.name.toStdString() << std::endl;
    indigoFree(mol);
    indigoReleaseSessionId(sid);
}
