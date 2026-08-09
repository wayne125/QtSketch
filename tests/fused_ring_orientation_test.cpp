#include "../src/app/FusedRingOrientation.h"
#include <iostream>
#include <cmath>
#include <cassert>

using namespace std;

int passed = 0;
int failed = 0;

void run_test(const string& name, const FusedRingSystemInput& input, int exp_row, double exp_ur) {
    cout << "Test: " << name << endl;
    auto res = computePreferredOrientation(input);
    if (!res.success) {
        cout << "  FAIL: returned error: " << res.error << endl;
        failed++;
        return;
    }
    
    bool ok = true;
    if (res.ringsInHorizontalRow != exp_row) {
        cout << "  FAIL: expected ringsInHorizontalRow=" << exp_row << ", got " << res.ringsInHorizontalRow << endl;
        ok = false;
    }
    if (abs(res.ringsInUpperRightQuadrant - exp_ur) > 1e-5) {
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
    // Case 1: Naphthalene
    FusedRingSystemInput naph;
    naph.ringSizes = {6, 6};
    naph.fusions = {{0, 1, 0}};
    run_test("Naphthalene", naph, 2, 0.5); 

    // Case 2: Anthracene
    FusedRingSystemInput anthra;
    anthra.ringSizes = {6, 6, 6};
    anthra.fusions = {{0, 1, 0}, {1, 2, 0}};
    run_test("Anthracene", anthra, 3, 0.75);

    // Case 3: Phenanthrene
    FusedRingSystemInput phenan;
    phenan.ringSizes = {6, 6, 6};
    phenan.fusions = {{0, 1, 0}, {1, 2, 5}};
    run_test("Phenanthrene", phenan, 2, 1.5);
    
    // Case 4: Naphthacene (Tetracene)
    FusedRingSystemInput tetra;
    tetra.ringSizes = {6, 6, 6, 6};
    tetra.fusions = {{0, 1, 0}, {1, 2, 0}, {2, 3, 0}};
    run_test("Naphthacene", tetra, 4, 1.0); 
    
    // Case 5: Branching / Triple junction cycle
    FusedRingSystemInput phenal;
    phenal.ringSizes = {6, 6, 6};
    phenal.fusions = {{0, 1, 0}, {1, 2, 4}, {2, 0, 2}};
    cout << "Test: Phenalene" << endl;
    auto res = computePreferredOrientation(phenal);
    if (!res.success && res.error.find("Non-tree") != string::npos) {
        cout << "  PASS: Rejected non-tree graph properly" << endl;
        passed++;
    } else {
        cout << "  FAIL: Expected non-tree rejection, got success=" << res.success << endl;
        failed++;
    }
    
    cout << endl;
    cout << "TOTAL PASSED: " << passed << endl;
    cout << "TOTAL FAILED: " << failed << endl;
    
    return failed == 0 ? 0 : 1;
}
