#pragma once
#include "../RetainedRingTypes.h"
#include <vector>

// Appendix 2 heterocyclic retained names -- phenoxazine-family tricyclics (entries 1-7).
// Dibenzo[b,e][1,4]heterazines: two benzo rings ortho-fused to a central 6-membered
// ring at OPPOSITE edges (positions 2-3 and 5-6 of the central ring), with N and
// X (O/S/Se/Te/P/As/Hg) at the TWO meso positions (1 and 4) of the central ring.
// This skeleton is DIFFERENT from anthracene/phenazine (which fuse through the meso
// positions) -- requires fresh SMILES and fresh Indigo verification for each entry.
void registerPhenoxazineFamily(std::vector<RetainedRingSystem> &registry);
