#include <iostream>
#include <string>
#include "app/MoleculeGraph.h"
#include "app/SmiToGraph.h"
#include "app/IupacNamer.h"
#include "indigo.h"

int main() {
    qword sid = indigoAllocSessionId();
    indigoSetSessionId(sid);
    std::string smiles = "O=C1CCC(=O)O1";
    int mol = indigoLoadMoleculeFromString(smiles.c_str());
    IupacResult res = IupacNamer::generateName(mol);
    std::cout << "SMILES: " << smiles << "\n";
    std::cout << "Success: " << res.success << "\n";
    std::cout << "Name: " << res.name.toStdString() << "\n";
    std::cout << "Error: " << res.error.toStdString() << "\n";
    return 0;
}
