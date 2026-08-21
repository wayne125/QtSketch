#include "IupacNamer.h"
#include "indigo.h"
#include <iostream>

int main() {
    qword sid = indigoAllocSessionId();
    indigoSetSessionId(sid);
    int mol = indigoLoadMoleculeFromString("CCCCC(=O)NN");
    IupacResult res = IupacNamer::generateName(mol);
    std::cout << "Name: " << res.name.toStdString() << "\n";
    indigoFree(mol);
    indigoReleaseSessionId(sid);
}
