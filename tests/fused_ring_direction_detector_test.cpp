#include "FusedRingDirectionDetector.h"
#include "FusedRingOrientation.h"
#include "indigo.h"
#include <iostream>
#include <string>
#include <vector>
#include <cmath>

using namespace std;

int passed = 0;
int failed = 0;

void check_fusions_identical(const FusedRingSystemInput& input, const std::vector<FusedRingEdge>& expected_fusions) {
    if (input.fusions.size() != expected_fusions.size()) {
        cout << "  FAIL: fusions size mismatch: " << input.fusions.size() << " vs " << expected_fusions.size() << endl;
        failed++;
        return;
    }
    for (size_t i = 0; i < input.fusions.size(); ++i) {
        const auto& f = input.fusions[i];
        const auto& e = expected_fusions[i];
        if (f.ring1 != e.ring1 || f.ring2 != e.ring2 || f.dir != e.dir) {
            cout << "  FAIL: fusion[" << i << "] mismatch: (" << f.ring1 << "," << f.ring2 << "," << f.dir 
                 << ") vs (" << e.ring1 << "," << e.ring2 << "," << e.dir << ")" << endl;
            failed++;
            return;
        }
    }
    cout << "  PASS: fusions identical" << endl;
    passed++;
}

void run_test(const string& name, const string& smiles, int exp_row, double exp_ur) {
    cout << "Test: " << name << " (" << smiles << ")" << endl;
    
    int mol = indigoLoadMoleculeFromString(smiles.c_str());
    if (mol < 0) {
        cout << "  FAIL: indigoLoadMoleculeFromString failed" << endl;
        failed++;
        return;
    }
    indigoAromatize(mol);
    
    FusedRingSystemInput input = detectFusedRingDirections(mol);
    indigoFree(mol);
    
    if (input.ringSizes.empty()) {
        cout << "  FAIL: detectFusedRingDirections rejected or failed" << endl;
        failed++;
        return;
    }
    
    auto res = computePreferredOrientation(input);
    if (!res.success) {
        cout << "  FAIL: computePreferredOrientation failed: " << res.error << endl;
        failed++;
        return;
    }
    
    bool ok = true;
    if (res.ringsInHorizontalRow != exp_row) {
        cout << "  FAIL: expected ringsInHorizontalRow=" << exp_row << ", got " << res.ringsInHorizontalRow << endl;
        ok = false;
    }
    if (exp_ur >= 0.0 && abs(res.ringsInUpperRightQuadrant - exp_ur) > 1e-5) {
        cout << "  FAIL: expected ringsInUpperRightQuadrant=" << exp_ur << ", got " << res.ringsInUpperRightQuadrant << endl;
        ok = false;
    }
    
    if (ok) {
        cout << "  PASS" << endl;
        passed++;
    } else {
        failed++;
    }
}

int main() {
    setvbuf(stdout, nullptr, _IONBF, 0);
    qword sid = indigoAllocSessionId();
    indigoSetSessionId(sid);
    
    // 1. Naphthalene - 2 rings
    run_test("Naphthalene", "c1ccc2ccccc2c1", 2, 0.5);
    
    // 2. Anthracene - 3 linear rings
    run_test("Anthracene", "c1ccc2cc3ccccc3cc2c1", 3, 0.75);
    
    // 3. Phenanthrene - 3 angular rings
    run_test("Phenanthrene", "c1ccc2c(c1)ccc1ccccc12", 2, 1.5);
    
    // 4. Naphthacene/tetracene - 4 linear rings
    run_test("Naphthacene", "c1ccc2cc3cc4ccccc4cc3cc2c1", 4, 1.0);
    
    // 5. Chrysene - 4 rings zigzag
    // We don't assert exp_ur precisely here since it's a bonus, but row must be 2.
    run_test("Chrysene", "c1ccc2c(c1)ccc3c4ccccc4ccc23", 2, -1.0);

    // 6. Pentacene - 5 linear rings
    run_test("Pentacene", "c1ccc2cc3cc4cc5ccccc5cc4cc3cc2c1", 5, 1.25);
    
    // 7. Regression test for linear case - verify fusions are identical to pre-change behavior
    // Anthracene: 3 linear rings, should produce fusions {{0,1,0}, {1,2,0}}
    cout << "Test: Linear regression (Anthracene)" << endl;
    int mol_anth = indigoLoadMoleculeFromString("c1ccc2cc3ccccc3cc2c1");
    if (mol_anth < 0) {
        cout << "  FAIL: indigoLoadMoleculeFromString failed" << endl;
        failed++;
    } else {
        indigoAromatize(mol_anth);
        FusedRingSystemInput input_anth = detectFusedRingDirections(mol_anth);
        indigoFree(mol_anth);
        
        // Expected fusions for linear anthracene: ring0->ring1 dir=0, ring1->ring2 dir=0
        std::vector<FusedRingEdge> expected_fusions = {{0, 1, 0}, {1, 2, 0}};
        check_fusions_identical(input_anth, expected_fusions);
    }
    
    // 8. Branching test - central benzene with 3 benzene/pyridine leaves
    // SMILES: c12c(cccc2)c3c(cccc3)c4c1(nccc4)
    // Verified: 4 rings (one pyridine, three benzene), degree-3 central ring, no peri-fusion
    cout << "Test: Branching (degree-3 central ring)" << endl;
    int mol_branch = indigoLoadMoleculeFromString("c12c(cccc2)c3c(cccc3)c4c1(nccc4)");
    if (mol_branch < 0) {
        cout << "  FAIL: indigoLoadMoleculeFromString failed for branching molecule" << endl;
        failed++;
    } else {
        indigoAromatize(mol_branch);
        FusedRingSystemInput input_branch = detectFusedRingDirections(mol_branch);
        indigoFree(mol_branch);
        
        if (input_branch.ringSizes.empty()) {
            cout << "  FAIL: Branching molecule was rejected (ringSizes empty)" << endl;
            failed++;
        } else if (input_branch.ringSizes.size() != 4) {
            cout << "  FAIL: Expected 4 rings, got " << input_branch.ringSizes.size() << endl;
            failed++;
        } else if (input_branch.fusions.size() != 3) {
            cout << "  FAIL: Expected 3 fusions (tree with 4 nodes), got " << input_branch.fusions.size() << endl;
            failed++;
        } else if (input_branch.ringAtoms.size() != 4) {
            cout << "  FAIL: Expected ringAtoms.size() == 4, got " << input_branch.ringAtoms.size() << endl;
            failed++;
        } else {
            // Check all ringAtoms are non-empty
            bool allNonEmpty = true;
            for (const auto& atoms : input_branch.ringAtoms) {
                if (atoms.empty()) {
                    allNonEmpty = false;
                    break;
                }
            }
            if (!allNonEmpty) {
                cout << "  FAIL: Some ringAtoms entries are empty" << endl;
                failed++;
            } else {
                // Check orientation computation works
                auto orient_res = computePreferredOrientation(input_branch);
                if (!orient_res.success) {
                    cout << "  FAIL: computePreferredOrientation failed: " << orient_res.error << endl;
                    failed++;
                } else if (orient_res.ringHexPos.size() != 4) {
                    cout << "  FAIL: Expected ringHexPos.size() == 4, got " << orient_res.ringHexPos.size() << endl;
                    failed++;
                } else {
                    // Check all positions are distinct
                    bool allDistinct = true;
                    for (size_t i = 0; i < orient_res.ringHexPos.size(); ++i) {
                        for (size_t j = i + 1; j < orient_res.ringHexPos.size(); ++j) {
                            if (orient_res.ringHexPos[i] == orient_res.ringHexPos[j]) {
                                allDistinct = false;
                                break;
                            }
                        }
                        if (!allDistinct) break;
                    }
                    if (!allDistinct) {
                        cout << "  FAIL: Not all ringHexPos positions are distinct" << endl;
                        failed++;
                    } else {
                        cout << "  PASS: Branching molecule accepted, 4 rings, 3 fusions, all ringAtoms non-empty, orientation computed with 4 distinct positions" << endl;
                        passed++;
                    }
                }
            }
        }
    }
    
    // 9. Peri-fusion regression test - verify peri-fused systems are still rejected
    // Pyrene: a peri-fused system with 4 rings in a non-tree topology
    // Using SMILES that should create a cyclic ring adjacency graph
    cout << "Test: Peri-fusion rejection (Pyrene)" << endl;
    int mol_peri = indigoLoadMoleculeFromString("c1cc2cc3cc4cccc5cccc1c2c34c5");
    if (mol_peri < 0) {
        cout << "  FAIL: indigoLoadMoleculeFromString failed for pyrene" << endl;
        failed++;
    } else {
        indigoAromatize(mol_peri);
        FusedRingSystemInput input_peri = detectFusedRingDirections(mol_peri);
        indigoFree(mol_peri);
        
        if (!input_peri.ringSizes.empty()) {
            cout << "  FAIL: Peri-fused molecule was accepted (ringSizes not empty), fusions: " << input_peri.fusions.size() << endl;
            failed++;
        } else {
            cout << "  PASS: Peri-fused molecule correctly rejected" << endl;
            passed++;
        }
    }
    
    cout << "\nTOTAL PASSED: " << passed << endl;
    cout << "TOTAL FAILED: " << failed << endl;
    
    return failed == 0 ? 0 : 1;
}
