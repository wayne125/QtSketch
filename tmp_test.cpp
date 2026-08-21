#include <iostream>
#include <string>
#include "app/IupacNamer.h"
#include "app/MoleculeGraph.h"
#include "app/SmiToGraph.h"
#include "indigo.h"

int main() {
    qword sid = indigoAllocSessionId();
    indigoSetSessionId(sid);
    QString smiles = "O=C1CCC(=O)O1";
    MoleculeGraph g = smiToGraph(smiles.toStdString());
    IupacNamer namer;
    auto result = namer.nameCompound(g);
    std::cout << "SMILES: " << smiles.toStdString() << "\n";
    std::cout << "Success: " << std::get<0>(result) << "\n";
    std::cout << "Name: " << std::get<1>(result).toStdString() << "\n";
    std::cout << "Error: " << std::get<2>(result).toStdString() << "\n";
    return 0;
}
