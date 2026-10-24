#include "../src/app/RetainedRingTypes.h"
#include "../src/app/retained/HydrocarbonFamily.h"
#include "../src/app/retained/QuinolineFamily.h"
#include "../src/app/retained/IndoleFamily.h"
#include "../src/app/retained/TricyclicNFamily.h"
#include "../src/app/retained/PhenoxazineFamily.h"
#include "../src/app/retained/ChromeneXanthineFamily.h"
#include "../src/app/retained/PhosphorusArsenicFamily.h"
#include "indigo.h"
#include <iostream>
#include <vector>

int main() {
    unsigned long long sid = indigoAllocSessionId();
    indigoSetSessionId(sid);

    std::vector<RetainedRingSystem> registry;
    registerHydrocarbonFamily(registry);
    registerQuinolineFamily(registry);
    registerIndoleFamily(registry);
    registerTricyclicNFamily(registry);
    registerPhenoxazineFamily(registry);
    registerChromeneXanthineFamily(registry);
    registerPhosphorusArsenicFamily(registry);

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

    // Dedicated tetracene self-test (Part 1 of the skeleton-matching /
    // partial-hydrogenation follow-on task): confirm the entry exists,
    // has the right atom count and the exact locant sequence independently
    // derived and cross-checked against rubrene's real name
    // (5,6,11,12-tetraphenyltetracene), tetracene-5,12-dione, and
    // PubChem's "1,2,3,4,7,8,9,10-octahydrotetracene" -- see
    // HydrocarbonFamily.cpp's own doc comment on this entry for the full
    // derivation.
    {
        const RetainedRingSystem *tet = nullptr;
        for (const auto &sys : registry) {
            if (sys.name == "tetracene") { tet = &sys; break; }
        }
        std::vector<QString> expectedLocants = {
            "2","3","4","4a","5","5a","6","6a","7","8","9","10","10a","11","11a","12","12a","1"
        };
        if (tet == nullptr) {
            std::cout << "[FAIL] tetracene entry not found in registry\n";
            ++failed;
        } else if (tet->peripheralLocants.size() != 18) {
            std::cout << "[FAIL] tetracene peripheralLocants.size() != 18 (got "
                       << tet->peripheralLocants.size() << ")\n";
            ++failed;
        } else if (tet->peripheralLocants != expectedLocants) {
            std::cout << "[FAIL] tetracene peripheralLocants do not match the expected "
                          "derived sequence\n";
            ++failed;
        } else if (tet->referenceSmiles != "c1ccc2cc3cc4ccccc4cc3cc2c1") {
            std::cout << "[FAIL] tetracene referenceSmiles mismatch\n";
            ++failed;
        } else if (tet->fusionPrefix != "tetraceno") {
            std::cout << "[FAIL] tetracene fusionPrefix mismatch (got '"
                       << tet->fusionPrefix.toStdString() << "', expected 'tetraceno')\n";
            ++failed;
        } else {
            std::cout << "[PASS] tetracene entry present with 18 atoms, the "
                          "expected independently-derived locant sequence, and "
                          "fusionPrefix='tetraceno' (verified against Blue Book "
                          "FR-2.1.2)\n";
            ++passed;
        }
    }

    // Dedicated carbazole self-test (Task 1 of the retained-hydro-assembly
    // plan): confirm the entry exists, has 13 atoms, and its
    // indicatedHydrogenLocant is "9" (the carbazole N-H), independently
    // verified via a compiled Indigo neighbor dump and cross-checked
    // against this project's own pre-existing hydro-prefix regression
    // comment in tests/iupac_namer_test.cpp (~line 1120) that separately
    // derived carbazole's indicated-H locant as "9H".
    {
        const RetainedRingSystem *carb = nullptr;
        for (const auto &sys : registry) {
            if (sys.name == "carbazole") { carb = &sys; break; }
        }
        if (carb == nullptr) {
            std::cout << "[FAIL] carbazole entry not found in registry\n";
            ++failed;
        } else if (carb->peripheralLocants.size() != 13) {
            std::cout << "[FAIL] carbazole peripheralLocants.size() != 13 (got "
                       << carb->peripheralLocants.size() << ")\n";
            ++failed;
        } else if (carb->indicatedHydrogenLocant != "9") {
            std::cout << "[FAIL] carbazole indicatedHydrogenLocant != \"9\" (got '"
                       << carb->indicatedHydrogenLocant.toStdString() << "')\n";
            ++failed;
        } else {
            QString err = validateRetainedRingSystem(0, *carb);
            if (!err.isEmpty()) {
                std::cout << "[FAIL] carbazole validateRetainedRingSystem: " << err.toStdString() << "\n";
                ++failed;
            } else {
                std::cout << "[PASS] carbazole entry present with 13 atoms, "
                              "indicatedHydrogenLocant=\"9\", and self-consistent\n";
                ++passed;
            }
        }
    }

    // Dedicated phenomercurazine self-test: confirm entry exists, has 14 atoms,
    // indicatedHydrogenLocant="2" (2H quinonoid tautomer per Appendix 2 Note d),
    // and self-consistent.
    {
        const RetainedRingSystem *merc = nullptr;
        for (const auto &sys : registry) {
            if (sys.name == "phenomercurazine") { merc = &sys; break; }
        }
        if (merc == nullptr) {
            std::cout << "[FAIL] phenomercurazine entry not found in registry\n";
            ++failed;
        } else if (merc->peripheralLocants.size() != 14) {
            std::cout << "[FAIL] phenomercurazine peripheralLocants.size() != 14 (got "
                       << merc->peripheralLocants.size() << ")\n";
            ++failed;
        } else if (merc->indicatedHydrogenLocant != "2") {
            std::cout << "[FAIL] phenomercurazine indicatedHydrogenLocant != \"2\" (got '"
                       << merc->indicatedHydrogenLocant.toStdString() << "')\n";
            ++failed;
        } else {
            QString err = validateRetainedRingSystem(0, *merc);
            if (!err.isEmpty()) {
                std::cout << "[FAIL] phenomercurazine validateRetainedRingSystem: " << err.toStdString() << "\n";
                ++failed;
            } else {
                std::cout << "[PASS] phenomercurazine entry present with 14 atoms, "
                              "indicatedHydrogenLocant=\"2\", and self-consistent\n";
                ++passed;
            }
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
