#include "app/IupacNamer.h"
#include <indigo.h>
#include <iostream>
#include <vector>
#include <QCoreApplication>

int main(int argc, char** argv) {
    QCoreApplication app(argc, argv);
    long long sid = indigoAllocSessionId();
    indigoSetSessionId(sid);
    
    std::vector<std::string> smiles = {
        "C1=CC=CCN1",
        "C1=CC=CC[NH]1",
        "C1=CC=CC[NH]1",
        "N1C=CC=CC1",
        "C1=CC=CC[NH]1"
    };
    for (auto s : smiles) {
        int m = indigoLoadMoleculeFromString(s.c_str());
        if (m < 0) {
            std::cout << s << " -> Indigo Error: " << indigoGetLastError() << "\n";
        } else {
            IupacResult r = IupacNamer::generateName(m);
            std::cout << s << " -> success=" << r.success << " name='" << r.name.toStdString() << "' error='" << r.error.toStdString() << "'\n";
            indigoFree(m);
        }
    }
    
    indigoReleaseSessionId(sid);
    return 0;
}
