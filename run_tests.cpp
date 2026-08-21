#include <iostream>
#include "IupacNamer.h"
#include "indigo.h"
int main() {
    auto test = [](const char* s) {
        int m = indigoLoadMoleculeFromString(s);
        auto r = IupacNamer::generateName(m);
        std::cout << s << " -> " << r.error.toStdString() << "\n";
    };
    test("C1=CC2=C3C1=CC=CC3=CC=C2");
    test("C1=CC2=C3C1=CC=C4C3=C(C=C2)C=C4");
    test("C1C2C=CC1C3=CC=CC=C23");
}
