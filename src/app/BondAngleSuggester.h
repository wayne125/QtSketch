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

    // Fallback for the 0-or-3-plus-neighbor cases suggestAngle deliberately excludes (see its
    // own doc comment above). Implements the same largest-empty-angle-bisection algorithm
    // V8Process::getLargestEmptyAngle (v8_process.cpp) already uses for TEMPLATE_ ring tools'
    // own hub-atom case -- and the same algorithm ChemDraw's own patent literature describes
    // ("place an interaction line starting in the direction of the largest opening angle formed
    // by the outgoing bonds of the interacting atom") -- reimplemented here against the same
    // atomsById/bondsList shape suggestAngle takes, rather than V8Process's primitives-based
    // copy, so a caller building its own atomsById/bondsList directly from a real molecule graph
    // doesn't have to route through V8Process's separately-maintained rendered-primitives
    // snapshot to use it.
    // Returns nullopt only if fromAtomId isn't found in atomsById at all; otherwise always
    // returns a value (0 neighbors -> angle 0; 1 neighbor -> angle-to-that-neighbor + 2.61799
    // radians (150 degrees); 2+ neighbors -> bisects the largest gap among all existing
    // neighbor angles).
    static std::optional<double> suggestFallbackAngle(int fromAtomId, const QVariantMap& atomsById,
                                                        const QVariantList& bondsList);
};
