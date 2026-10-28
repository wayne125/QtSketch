#pragma once
#include "../RetainedRingTypes.h"
#include <vector>

// Appendix 2 heterocyclic retained names -- tricyclic N-heterocycles (entries 8-23).
//
// NOTE: The existing IupacNamer.cpp has special-case logic for WHOLE MOLECULES that
// are exactly acridine or carbazole (lines ~6522-6527). That logic handles the case
// where the entire molecule IS acridine/carbazole. This registry, by contrast, handles
// acridine/carbazole appearing as a COMPONENT of a larger fused system (e.g., benzo[...]acridine).
// These are separate use cases and do not conflict.
void registerTricyclicNFamily(std::vector<RetainedRingSystem> &registry);
