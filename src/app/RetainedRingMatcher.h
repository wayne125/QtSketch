#pragma once
#include <optional>
#include <vector>
#include "NamingTypes.h"

// Tries to decompose the molecule's fused-ring system into retained
// polycyclic components (base + attached prefixes) per Blue Book
// FR-2.2/FR-2.3. Returns a result on success, std::nullopt if no
// retained-system match is found (caller should fall back to the
// existing monocyclic fusion logic).
std::optional<RingSystemNameResult> tryRetainedBaseDecomposition(int mol);
