#include <iostream>
#include "src/app/IupacNamer.h"
#include "indigo.h"

int main() {
    indigoInit();
    
    std::string smiles[] = {
        "CP(=O)(O)O",         // Should be methanephosphonic acid
        "CCCP(=O)(O)O",      // Should be propane-1-phosphonic acid  
        "c1ccccc1P(=O)(O)O", // Should be benzenophosphonic acid
        "C1CCCCC1P(=O)(O)O"  // Should be cyclohexanephosphonic acid
    };
    
    for (const auto& smile : smiles) {
        int mol = indigoLoadMoleculeFromString(smile.c_str());
        IupacResult result = IupacNamer::generateName(mol);
        indigoFree(mol);
        
        std::cout << "SMILES: " << smile << "\n";
        std::cout << "  Success: " << result.success << "\n";
        std::cout << "  Name: " << result.name.toStdString() << "\n";
        std::cout << "  Error: " << result.error.toStdString() << "\n\n";
    }
    
    indigoShutdown();
    return 0;
}