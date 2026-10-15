#pragma once

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
