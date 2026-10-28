#include "../src/app/NamingValidation.h"
#include "../src/app/FusedRingOrientation.h"
#include <iostream>
#include <vector>
#include <set>

using namespace std;

int passed = 0;
int failed = 0;

// Helper to create a simple graph for a benzene ring (6 carbon atoms in a cycle)
Graph createBenzeneGraph() {
    Graph g;
    g.nodes.resize(6);
    for (int i = 0; i < 6; ++i) {
        g.nodes[i].id = i;
        g.nodes[i].indigoIdx = i;
        g.nodes[i].atomicNumber = 6;  // Carbon
        g.nodes[i].totalH = 1;
        // Each carbon connected to 2 neighbors in the ring
        g.nodes[i].neighbors = { (i-1+6)%6, (i+1)%6 };
        g.nodes[i].bondOrders = {4, 4};  // Aromatic bonds
    }
    return g;
}

// Test 1: Valid ring cycle should pass validateRingCycle
void testValidRingCycle() {
    Graph g = createBenzeneGraph();
    vector<int> validCycle = {0, 1, 2, 3, 4, 5};  // Valid benzene cycle
    
    bool result = validateRingCycle(g, validCycle);
    if (result) {
        cout << "[PASS] Test 1: Valid benzene ring cycle passes validateRingCycle" << endl;
        passed++;
    } else {
        cout << "[FAIL] Test 1: Valid benzene ring cycle should pass validateRingCycle" << endl;
        failed++;
    }
}

// Test 2: Scrambled/invalid cycle should fail validateRingCycle
void testInvalidRingCycle() {
    Graph g = createBenzeneGraph();
    vector<int> scrambledCycle = {0, 2, 1, 3, 4, 5};  // Invalid: 0-2 are not directly connected
    
    bool result = validateRingCycle(g, scrambledCycle);
    if (!result) {
        cout << "[PASS] Test 2: Scrambled benzene ring cycle fails validateRingCycle" << endl;
        passed++;
    } else {
        cout << "[FAIL] Test 2: Scrambled benzene ring cycle should fail validateRingCycle" << endl;
        failed++;
    }
}

// Test 3: FusedRingSystemInput with deliberately wrong bondIndex (out of range) should fail
void testInvalidBondIndexOutOfRange() {
    Graph g = createBenzeneGraph();
    
    FusedRingSystemInput input;
    input.ringSizes = {6, 6};
    input.ringAtoms = {{0, 1, 2, 3, 4, 5}, {0, 1, 2, 3, 4, 5}};
    
    // Create fusion with out-of-range bondIndex
    FusedRingEdge edge;
    edge.ring1 = 0;
    edge.ring2 = 1;
    edge.bondIndex = CycleEdgeIndex(99);  // Clearly out of range for a 6-membered ring
    input.fusions.push_back(edge);
    
    // Set ringDirectedCycles with valid cycles
    input.ringDirectedCycles = {{0, 1, 2, 3, 4, 5}, {0, 1, 2, 3, 4, 5}};
    
    bool result = validateFusionInput(g, input);
    if (!result) {
        cout << "[PASS] Test 3: FusedRingSystemInput with out-of-range bondIndex (99) fails validateFusionInput" << endl;
        passed++;
    } else {
        cout << "[FAIL] Test 3: FusedRingSystemInput with out-of-range bondIndex should fail validateFusionInput" << endl;
        failed++;
    }
}

// Test 4: FusedRingSystemInput with correct bondIndex should pass
void testValidFusionInput() {
    Graph g = createBenzeneGraph();
    
    FusedRingSystemInput input;
    input.ringSizes = {6, 6};
    input.ringAtoms = {{0, 1, 2, 3, 4, 5}, {0, 1, 2, 3, 4, 5}};
    
    // Create fusion with correct bondIndex
    // In ring0's cycle [0,1,2,3,4,5], atom 0 is at position 0, atom 1 is at position 1
    // The edge between atoms 0 and 1 is at bondIndex 0 (between positions 0 and 1)
    FusedRingEdge edge;
    edge.ring1 = 0;
    edge.ring2 = 1;
    edge.bondIndex = CycleEdgeIndex(0);  // Valid: edge between cycle[0] and cycle[1]
    input.fusions.push_back(edge);
    
    // Set ringDirectedCycles
    input.ringDirectedCycles = {{0, 1, 2, 3, 4, 5}, {0, 1, 2, 3, 4, 5}};
    
    bool result = validateFusionInput(g, input);
    if (result) {
        cout << "[PASS] Test 4: Correctly-built FusedRingSystemInput passes validateFusionInput" << endl;
        passed++;
    } else {
        cout << "[FAIL] Test 4: Correctly-built FusedRingSystemInput should pass validateFusionInput" << endl;
        failed++;
    }
}

// Test 5: FusedRingSystemInput with an IN-RANGE but WRONG-index-space
// bondIndex should fail -- this is the real bug class that motivated this
// round (a raw atom-neighbor-list position that happens to be numerically
// valid for the ring size, but doesn't correspond to the actual shared
// edge between the two fused rings). Test 3 only covers a trivially
// out-of-range value; this covers the subtler, actually-dangerous case.
void testInvalidBondIndexWrongSpaceInRange() {
    Graph g = createBenzeneGraph();

    FusedRingSystemInput input;
    input.ringSizes = {6, 6};
    // ring0 = {0,1,2,3,4,5}, ring1 = {2,3,8,9,10,11} fused to ring0 sharing
    // the real edge (2,3) -- the physical bridgeheads are atoms 2 and 3.
    input.ringAtoms = {{0, 1, 2, 3, 4, 5}, {2, 3, 8, 9, 10, 11}};

    FusedRingEdge edge;
    edge.ring1 = 0;
    edge.ring2 = 1;
    // WRONG: bondIndex 0 is in-range (0 <= 0 < 6) and points to a real edge
    // of ring0 (cycle[0]=0, cycle[1]=1), but atoms 0 and 1 are NOT the real
    // bridgeheads shared with ring1 -- this is exactly the wrong-index-
    // space mistake (e.g. a raw neighbor-list position for a different
    // atom) that this round's type-safety work is meant to catch.
    edge.bondIndex = CycleEdgeIndex(0);
    input.fusions.push_back(edge);

    input.ringDirectedCycles = {{0, 1, 2, 3, 4, 5}, {2, 3, 8, 9, 10, 11}};

    bool result = validateFusionInput(g, input);
    if (!result) {
        cout << "[PASS] Test 5: In-range but wrong-index-space bondIndex fails validateFusionInput" << endl;
        passed++;
    } else {
        cout << "[FAIL] Test 5: In-range but wrong-index-space bondIndex should fail validateFusionInput" << endl;
        failed++;
    }
}

int main() {
    testValidRingCycle();
    testInvalidRingCycle();
    testInvalidBondIndexOutOfRange();
    testValidFusionInput();
    testInvalidBondIndexWrongSpaceInRange();

    cout << endl;
    cout << "TOTAL PASSED: " << passed << endl;
    cout << "TOTAL FAILED: " << failed << endl;
    
    return failed == 0 ? 0 : 1;
}
