#pragma once
#include <QString>
#include <map>

// Strong-typed wrappers for the index spaces this file's fused-ring naming
// code mixes in ways that have caused real, previously-shipped bugs (see
// FusedRingOrientation.h's own documented history). Deliberately minimal:
// implicit construction from int is allowed (so existing call sites that
// pass a plain int compile without churn), but these are DISTINCT TYPES
// from each other and from plain int, so a function that expects one
// cannot silently accept another by accident at a typed call site.

struct GraphAtomId {
    int value;
    GraphAtomId() : value(-1) {}
    GraphAtomId(int v) : value(v) {}
    operator int() const { return value; }
    bool operator==(const GraphAtomId &o) const { return value == o.value; }
    bool operator<(const GraphAtomId &o) const { return value < o.value; }
};

struct IndigoAtomId {
    int value;
    IndigoAtomId() : value(-1) {}
    IndigoAtomId(int v) : value(v) {}
    operator int() const { return value; }
    bool operator==(const IndigoAtomId &o) const { return value == o.value; }
    bool operator<(const IndigoAtomId &o) const { return value < o.value; }
};

struct RingId {
    int value;
    RingId() : value(-1) {}
    RingId(int v) : value(v) {}
    operator int() const { return value; }
    bool operator==(const RingId &o) const { return value == o.value; }
    bool operator<(const RingId &o) const { return value < o.value; }
};

// Position of an atom WITHIN one ring's own directed cycle array
// (cyclesN[ring] or FusedRingSystemInput::ringDirectedCycles[ring]),
// ranging over [0, ringSize-1] -- NOT a raw molecular-graph neighbor-list
// index, which is a completely different number for the same physical
// bond and was the exact source of a real, previously-shipped bug.
struct CycleEdgeIndex {
    int value;
    CycleEdgeIndex() : value(-1) {}
    CycleEdgeIndex(int v) : value(v) {}
    operator int() const { return value; }
    bool operator==(const CycleEdgeIndex &o) const { return value == o.value; }
};

// Result type for ring system naming -- used by both IupacNamer.cpp
// and RetainedRingMatcher.cpp. Defined here (not in either .cpp) so there
// is exactly one definition visible to both translation units.
struct RingSystemNameResult {
    bool success = false;
    QString error;
    QString baseName;
    std::map<int, QString> locantMap;
};

#include <climits>
#include <vector>

struct RingSubstituentCandidateScore {
    int rootLocant = INT_MAX;
    std::vector<int> sortedBranchLocants;
    std::vector<int> citationLocants;

    bool operator<(const RingSubstituentCandidateScore &o) const {
        if (rootLocant != o.rootLocant) return rootLocant < o.rootLocant;
        if (sortedBranchLocants != o.sortedBranchLocants) return sortedBranchLocants < o.sortedBranchLocants;
        return citationLocants < o.citationLocants;
    }
    bool operator==(const RingSubstituentCandidateScore &o) const {
        return rootLocant == o.rootLocant &&
               sortedBranchLocants == o.sortedBranchLocants &&
               citationLocants == o.citationLocants;
    }
};

bool rankRingSubstituentCandidate(
    const RingSystemNameResult &candidate,
    int rootAtom,
    const std::vector<std::pair<int, QString>> &extraBranches,
    RingSubstituentCandidateScore &outScore);

bool rankCandidate(
    const RingSystemNameResult &candidate,
    int rootAtom,
    const std::vector<std::pair<int, QString>> &extraBranches,
    int &outRootLocant,
    std::vector<int> &outBranchLocants);

int compareRingSubstituentCandidates(
    const RingSystemNameResult &a,
    const RingSystemNameResult &b,
    int rootAtom,
    const std::vector<std::pair<int, QString>> &extraBranches);
