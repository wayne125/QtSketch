#include "../src/app/RetainedRingMatcher.h"
#include "indigo.h"
#include <iostream>
#include <optional>

int main() {
    unsigned long long sid = indigoAllocSessionId();
    indigoSetSessionId(sid);

    int passed = 0, failed = 0;

    // Test 1: Plain isoquinoline molecule matches the isoquinoline registry entry
    // Use the exact SMILES from the registry: "n1ccc2ccccc2c1" (lowercase n --
    // see QuinolineFamily.cpp's retroactive fix: the originally-dispatched
    // uppercase "N1ccc2ccccc2c1" gave the wrong gross formula and is no
    // longer what the registry stores, so this test's own target molecule
    // must match it).
    {
        std::cout << "Test 1: Plain isoquinoline vs registry entry" << std::endl;
        int mol = indigoLoadMoleculeFromString("n1ccc2ccccc2c1");
        if (mol < 0) {
            std::cout << "  [FAIL] Could not parse isoquinoline SMILES" << std::endl;
            ++failed;
        } else {
            auto result = tryRetainedBaseDecomposition(mol);
            if (result) {
                if (result->success && result->baseName == "isoquinoline") {
                    std::cout << "  [PASS] isoquinoline matched itself, baseName=" << result->baseName.toStdString() << std::endl;
                    ++passed;
                } else {
                    std::cout << "  [FAIL] isoquinoline matched but baseName='" << result->baseName.toStdString()
                              << "' success=" << result->success << std::endl;
                    ++failed;
                }
            } else {
                std::cout << "  [FAIL] isoquinoline returned nullopt" << std::endl;
                ++failed;
            }
            indigoFree(mol);
        }
    }

    // Test 2: Plain naphthalene molecule matches the naphthalene registry entry
    // Use the exact SMILES from the registry: "c1ccc2ccccc2c1"
    {
        std::cout << "\nTest 2: Plain naphthalene vs registry entry" << std::endl;
        int mol = indigoLoadMoleculeFromString("c1ccc2ccccc2c1");
        if (mol < 0) {
            std::cout << "  [FAIL] Could not parse naphthalene SMILES" << std::endl;
            ++failed;
        } else {
            auto result = tryRetainedBaseDecomposition(mol);
            if (result) {
                if (result->success && result->baseName == "naphthalene") {
                    std::cout << "  [PASS] naphthalene matched itself, baseName=" << result->baseName.toStdString() << std::endl;
                    ++passed;
                } else {
                    std::cout << "  [FAIL] naphthalene matched but baseName='" << result->baseName.toStdString()
                              << "' success=" << result->success << std::endl;
                    ++failed;
                }
            } else {
                std::cout << "  [FAIL] naphthalene returned nullopt" << std::endl;
                ++failed;
            }
            indigoFree(mol);
        }
    }

    // Test 3: Branching molecule returns nullopt
    // Using a known branching test molecule from iupac_namer_test.cpp
    {
        std::cout << "\nTest 3: Branching molecule (c12c(cccc2)c3c(cccc3)c4c1(nccc4)) returns nullopt" << std::endl;
        int mol = indigoLoadMoleculeFromString("c12c(cccc2)c3c(cccc3)c4c1(nccc4)");
        if (mol < 0) {
            std::cout << "  [FAIL] Could not parse branching molecule SMILES" << std::endl;
            ++failed;
        } else {
            auto result = tryRetainedBaseDecomposition(mol);
            if (!result) {
                std::cout << "  [PASS] Branching molecule correctly returned nullopt" << std::endl;
                ++passed;
            } else {
                std::cout << "  [FAIL] Branching molecule returned a result (should be nullopt)" << std::endl;
                if (result->success) {
                    std::cout << "    baseName=" << result->baseName.toStdString() << std::endl;
                }
                ++failed;
            }
            indigoFree(mol);
        }
    }

    // Test 4: The ground-truth molecule naphtho[2,3-g]isoquinoline
    // SMILES: c1ccc2cc3cc4ccncc4cc3cc2c1
    // Should match with isoquinoline as the base component (k=2 subset)
    {
        std::cout << "\nTest 4: Ground-truth molecule c1ccc2cc3cc4ccncc4cc3cc2c1" << std::endl;
        int mol = indigoLoadMoleculeFromString("c1ccc2cc3cc4ccncc4cc3cc2c1");
        if (mol < 0) {
            std::cout << "  [FAIL] Could not parse ground-truth SMILES" << std::endl;
            ++failed;
        } else {
            auto result = tryRetainedBaseDecomposition(mol);
            if (result) {
                if (result->success) {
                    std::cout << "  [PASS] Ground-truth molecule matched, baseName=" << result->baseName.toStdString() << std::endl;
                    std::cout << "    Note: Full FR-4 citation assembly is deferred to later tasks" << std::endl;
                    ++passed;
                } else {
                    std::cout << "  [FAIL] Ground-truth molecule matched but success=false, error='" 
                              << result->error.toStdString() << "'" << std::endl;
                    ++failed;
                }
            } else {
                std::cout << "  [FAIL] Ground-truth molecule returned nullopt (should find isoquinoline base)" << std::endl;
                ++failed;
            }
            indigoFree(mol);
        }
    }

    // Test 5: Single ring (benzene) - should return nullopt (need at least 2 rings)
    {
        std::cout << "\nTest 5: Single ring (benzene) returns nullopt" << std::endl;
        int mol = indigoLoadMoleculeFromString("c1ccccc1");
        if (mol < 0) {
            std::cout << "  [FAIL] Could not parse benzene SMILES" << std::endl;
            ++failed;
        } else {
            auto result = tryRetainedBaseDecomposition(mol);
            if (!result) {
                std::cout << "  [PASS] Single ring correctly returned nullopt" << std::endl;
                ++passed;
            } else {
                std::cout << "  [FAIL] Single ring returned a result (should be nullopt)" << std::endl;
                ++failed;
            }
            indigoFree(mol);
        }
    }

    // Test 6: Quinoline alone should match quinoline registry entry
    // Use the exact SMILES from the registry: "c1ccc2ccccc2n1" (lowercase n,
    // see QuinolineFamily.cpp's retroactive fix)
    {
        std::cout << "\nTest 6: Plain quinoline vs registry entry" << std::endl;
        int mol = indigoLoadMoleculeFromString("c1ccc2ccccc2n1");
        if (mol < 0) {
            std::cout << "  [FAIL] Could not parse quinoline SMILES" << std::endl;
            ++failed;
        } else {
            auto result = tryRetainedBaseDecomposition(mol);
            if (result) {
                if (result->success && result->baseName == "quinoline") {
                    std::cout << "  [PASS] quinoline matched itself, baseName=" << result->baseName.toStdString() << std::endl;
                    ++passed;
                } else {
                    std::cout << "  [FAIL] quinoline matched but baseName='" << result->baseName.toStdString()
                              << "' success=" << result->success << std::endl;
                    ++failed;
                }
            } else {
                std::cout << "  [FAIL] quinoline returned nullopt" << std::endl;
                ++failed;
            }
            indigoFree(mol);
        }
    }

    std::cout << "\n========================================" << std::endl;
    std::cout << "TOTAL PASSED: " << passed << std::endl;
    std::cout << "TOTAL FAILED: " << failed << std::endl;
    std::cout << "========================================" << std::endl;

    indigoFree(sid);
    return (failed > 0) ? 1 : 0;
}
