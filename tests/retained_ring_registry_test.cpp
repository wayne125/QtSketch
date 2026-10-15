#include "../src/app/RetainedRingTypes.h"
#include "../src/app/retained/HydrocarbonFamily.h"
#include "../src/app/retained/QuinolineFamily.h"
#include "indigo.h"
#include <iostream>
#include <vector>

int main() {
    unsigned long long sid = indigoAllocSessionId();
    indigoSetSessionId(sid);

    std::vector<RetainedRingSystem> registry;
    registerHydrocarbonFamily(registry);
    registerQuinolineFamily(registry);

    int passed = 0, failed = 0;
    for (const auto &sys : registry) {
        QString err = validateRetainedRingSystem(0, sys);
        if (err.isEmpty()) {
            std::cout << "[PASS] " << sys.name.toStdString() << " self-consistent\n";
            ++passed;
        } else {
            std::cout << "[FAIL] " << err.toStdString() << "\n";
            ++failed;
        }
    }

    // Deliberate negative test: a transposed locant must be caught, not
    // just a wrong-length list (this is the Review Focus item -- prove it
    // here, don't just assert it in the plan).
    RetainedRingSystem broken = registry[0]; // naphthalene
    std::swap(broken.peripheralLocants[0], broken.peripheralLocants[1]);
    QString err = validateRetainedRingSystem(0, broken);
    if (!err.isEmpty()) {
        std::cout << "[PASS] transposed-locant naphthalene correctly rejected: "
                   << err.toStdString() << "\n";
        ++passed;
    } else {
        std::cout << "[FAIL] transposed-locant naphthalene was NOT rejected\n";
        ++failed;
    }

    std::cout << "\nTOTAL PASSED: " << passed << "\nTOTAL FAILED: " << failed << "\n";
    indigoReleaseSessionId(sid);
    return failed == 0 ? 0 : 1;
}
