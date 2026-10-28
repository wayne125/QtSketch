#pragma once
#include "../RetainedRingTypes.h"
#include <vector>

// Appendix 1 hydrocarbon retained names -- deliberately scoped to the
// common systems actually likely to appear as an attached-component
// prefix (naphtho-, anthra-, etc.) in a real molecule, NOT the full
// Appendix 1 list (see this plan's Global Constraints: the ~45 exotic
// multi-ring mega-systems like polyphenylene/decacene/coronene/helicenes
// are an explicit, documented scope cut).
void registerHydrocarbonFamily(std::vector<RetainedRingSystem> &registry);
