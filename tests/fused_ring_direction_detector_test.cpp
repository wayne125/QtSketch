#include "FusedRingDirectionDetector.h"
#include "FusedRingOrientation.h"
#include "indigo.h"
#include <iostream>
#include <string>
#include <cmath>

using namespace std;

int passed = 0;
int failed = 0;

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
    
    cout << "\nTOTAL PASSED: " << passed << endl;
    cout << "TOTAL FAILED: " << failed << endl;
    
    return failed == 0 ? 0 : 1;
}
