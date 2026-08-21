#include "app/IupacNamer.h"
#include <indigo.h>
#include <iostream>
#include <QCoreApplication>

int main(int argc, char** argv) {
    QCoreApplication app(argc, argv);
    long long sid = indigoAllocSessionId();
    indigoSetSessionId(sid);
    
    int m = indigoLoadMoleculeFromString("C1=CC=CCN1");
    IupacResult r = IupacNamer::generateName(m);
    indigoFree(m);
    
    std::cout << "Success: " << r.success << "\n";
    std::cout << "Name: " << r.name.toStdString() << "\n";
    std::cout << "Error: " << r.error.toStdString() << "\n";
    
    indigoReleaseSessionId(sid);
    return 0;
}
