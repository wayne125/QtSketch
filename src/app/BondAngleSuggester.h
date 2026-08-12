#pragma once
#include <QVariantMap>
#include <QVariantList>
#include <optional>

// Shared angle-suggestion logic for click-to-place tools attaching to an existing atom.
// Extracted from V8Process::suggestBondEndpoint (verified this session against real
// BIOVIA-captured geometry: a single existing neighbor gets a 60-degree kink off the
// continuation of the incoming bond, alternating up/down via a one-hop grandparent lookback
// so a repeatedly-clicked chain tip zigzags flat; two existing neighbors get a symmetric
// 120/120/120 trigonal fill by bisecting the larger of the two gaps). Deliberately returns
// nullopt for 0 or 3+ existing neighbors -- those cases are each caller's own existing
// fallback (0-neighbor default-direction, 3+-neighbor grid-snap), not part of this helper.
class BondAngleSuggester {
public:
    // fromAtomId: the atom the new bond/attachment extends from.
    // atomsById: same shape V8Process::m_primitives["atomsById"] already has (string-keyed
    //   atom id -> {"x":.., "y":..}).
    // bondsList: same shape V8Process::m_primitives["bonds"] already has (list of
    //   {"begin":atomId, "end":atomId}).
    // Returns the suggested angle in radians (atan2 convention), or nullopt if fromAtomId
    // isn't found in atomsById or has 0 or 3+ existing bonds.
    static std::optional<double> suggestAngle(int fromAtomId, const QVariantMap& atomsById,
                                               const QVariantList& bondsList);
};
