#pragma once

#include <vector>
#include "GraphTypes.h"
#include "FusedRingOrientation.h"

// Returns true if `cycle` is a valid simple cycle over the real bonds of
// `g`: every element distinct, every consecutive pair (including the
// last-to-first wraparound) a real bond in g.
bool validateRingCycle(const Graph &g, const std::vector<int> &cycle);

// Returns true if every FusedRingEdge in input.fusions names a real shared
// edge between its two rings: both bridgehead atoms are members of both
// rings' atom sets, and bondIndex is a valid position in ring1's own
// directed cycle (0 <= bondIndex < ringDirectedCycles[ring1].size()) whose
// cyclically-adjacent neighbor in that same cycle is the OTHER bridgehead.
bool validateFusionInput(const Graph &g, const FusedRingSystemInput &input);
