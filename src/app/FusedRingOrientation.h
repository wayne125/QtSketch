#pragma once

#include <vector>
#include <string>

struct FusedRingOrientationResult {
    bool success = false;
    std::string error;
    std::vector<int> ringOrder;
    int ringsInHorizontalRow = 0;
    double ringsInUpperRightQuadrant = 0.0;
    double ringsInLowerLeftQuadrant = 0.0;
    double ringsAboveHorizontalRow = 0.0;
};

// Represents a fusion bond between two rings.
// dir is the global hex-grid direction from ring1 to ring2 (0 to 5).
// 0=Right, 1=BottomRight, 2=BottomLeft, 3=Left, 4=TopLeft, 5=TopRight
struct FusedRingEdge {
    int ring1;
    int ring2;
    int dir; 
};

struct FusedRingSystemInput {
    std::vector<int> ringSizes; // indexed by ring ID (0 to N-1)
    std::vector<FusedRingEdge> fusions;
};

FusedRingOrientationResult computePreferredOrientation(const FusedRingSystemInput &input);
