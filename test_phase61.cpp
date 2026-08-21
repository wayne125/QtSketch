#include "IupacNamer.h"
#include "indigo.h"
#include <iostream>
#include <string>

int main() {
    setvbuf(stdout, nullptr, _IONBF, 0);
    qword sid = indigoAllocSessionId();
    indigoSetSessionId(sid);
    
    std::cout << "Testing Phase 61 cases...\n";
    
    // Test case 1: boronic acid chain-wins
    {
        std::cout << "Testing CC1CCCCC1CCCB(O)O...\n";
        int m = indigoLoadMoleculeFromString("CC1CCCCC1CCCB(O)O");
        IupacResult r = IupacNamer::generateName(m);
        indigoFree(m);
        std::cout << "Result: success=" << r.success << " name='" << r.name.toStdString() << "' err='" << r.error.toStdString() << "'\n";
    }
    
    // Test case 2: phosphine chain-wins
    {
        std::cout << "Testing CC1CCCCC1CCCP...\n";
        int m = indigoLoadMoleculeFromString("CC1CCCCC1CCCP");
        IupacResult r = IupacNamer::generateName(m);
        indigoFree(m);
        std::cout << "Result: success=" << r.success << " name='" << r.name.toStdString() << "' err='" << r.error.toStdString() << "'\n";
    }
    
    // Test case 3: regression - pure acyclic boronic acid
    {
        std::cout << "Testing CCB(O)O...\n";
        int m = indigoLoadMoleculeFromString("CCB(O)O");
        IupacResult r = IupacNamer::generateName(m);
        indigoFree(m);
        std::cout << "Result: success=" << r.success << " name='" << r.name.toStdString() << "' err='" << r.error.toStdString() << "'\n";
    }
    
    // Test case 4: regression - pure acyclic phosphine
    {
        std::cout << "Testing CCP...\n";
        int m = indigoLoadMoleculeFromString("CCP");
        IupacResult r = IupacNamer::generateName(m);
        indigoFree(m);
        std::cout << "Result: success=" << r.success << " name='" << r.name.toStdString() << "' err='" << r.error.toStdString() << "'\n";
    }
    
    // Test case 5: regression - existing acid case
    {
        std::cout << "Testing CC1CCCCC1CCC(=O)O...\n";
        int m = indigoLoadMoleculeFromString("CC1CCCCC1CCC(=O)O");
        IupacResult r = IupacNamer::generateName(m);
        indigoFree(m);
        std::cout << "Result: success=" << r.success << " name='" << r.name.toStdString() << "' err='" << r.error.toStdString() << "'\n";
    }
    
    indigoFreeSessionId(sid);
    return 0;
}