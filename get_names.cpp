#include <iostream>
#include "indigo.h"
#include "src/app/IupacNamer.h"
int main() {
  qulonglong sid = indigoAllocSessionId();
  indigoSetSessionId(sid);
  for (const char* smi : {"Oc1ccccc1CCC", "Oc1cccnc1CCC", "Oc1cccnc1"}) {
    int m = indigoLoadMoleculeFromString(smi);
    IupacResult r = IupacNamer::generateName(m);
    std::cout << smi << " -> " << r.name.toStdString() << std::endl;
    indigoFree(m);
  }
  indigoReleaseSessionId(sid);
  return 0;
}
