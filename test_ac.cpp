#include "IupacNamer.h"
#include "indigo.h"
#include <iostream>

int main() {
    int m1 = indigoLoadMoleculeFromString("C1=CC2=C3C1=CC=CC3=CC=C2");
    IupacResult r1 = IupacNamer::generateName(m1);
    std::cout << "Acenaphthylene: " << r1.success << " " << r1.name.toStdString() << " " << r1.error.toStdString() << "\n";
    
    int m2 = indigoLoadMoleculeFromString("C1=CC2C=CC1C2");
    IupacResult r2 = IupacNamer::generateName(m2);
    std::cout << "Bicyclo diene: " << r2.success << " " << r2.name.toStdString() << " " << r2.error.toStdString() << "\n";
    return 0;
}
