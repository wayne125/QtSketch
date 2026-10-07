#pragma once

#include <vector>
#include <string>
#include <set>

struct FusedRingOrientationResult {
    bool success = false;
    std::string error;
    std::vector<int> ringOrder;
    int ringsInHorizontalRow = 0;
    double ringsInUpperRightQuadrant = 0.0;
    double ringsInLowerLeftQuadrant = 0.0;
    double ringsAboveHorizontalRow = 0.0;
    std::vector<std::pair<double,double>> ringPos;  // real (x,y) ring-center
                                                // positions; side length
                                                // normalized to 1.0.
    std::vector<double> ringRotation;  // each ring's own absolute rotation
                                        // (radians, the angle its own vertex
                                        // 0 sits at) in the SAME final
                                        // coordinate frame as ringPos (i.e.
                                        // already carries the winning
                                        // orientation's rotation+reflection
                                        // applied -- callers needing real
                                        // per-atom placement use this plus
                                        // ringPos to lay out each ring's own
                                        // vertices, mirroring how this file's
                                        // own placement algorithm does it).
};

// Represents a fusion bond between two rings.
// bondIndex is the 0-based position of this fusion's shared edge within
// ring1's own directed cycle (buildDirectedCycle's traversal order in
// FusedRingDirectionDetector.cpp), ranging over [0, ringSizes[ring1]-1].
// FusedRingOrientation.cpp converts this to a real angle using ring1's
// own regular-polygon geometry (angle = bondIndex * 2*PI / ringSizes[ring1]).
struct FusedRingEdge {
    int ring1;
    int ring2;
    int bondIndex;
};

struct FusedRingSystemInput {
    std::vector<int> ringSizes; // indexed by ring ID (0 to N-1)
    std::vector<FusedRingEdge> fusions;
    std::vector<std::set<int>> ringAtoms;
};

// Returns every orientation that ties for the best P-25.3.2.3 score (within
// floating-point tolerance) -- a topologically-symmetric branching system
// (e.g. 3 equivalent leaf rings off one central ring) can have several
// genuinely tied-best orientations, distinguishable only by which real
// heteroatom ends up in which ring. This module has no atom-identity data
// (only FusedRingSystemInput's own graph-of-rings topology) to break that
// tie correctly -- callers with real atom data (IupacNamer.cpp) should use
// this function and pick among the results themselves via the real Blue
// Book heteroatom-lowest-locant rule, rather than accepting an arbitrary one.
std::vector<FusedRingOrientationResult> computeAllPreferredOrientations(const FusedRingSystemInput &input);

// Convenience wrapper: returns just the first tied-best orientation. Use
// computeAllPreferredOrientations instead when the caller can tell tied
// candidates apart (e.g. by heteroatom position) and ties are possible for
// the input's topology (any branching/non-linear system).
FusedRingOrientationResult computePreferredOrientation(const FusedRingSystemInput &input);
