// src/app/molecule/RenderPrimitives.h
#ifndef RENDERPRIMITIVES_H
#define RENDERPRIMITIVES_H

// C++ replacement for 10-state.js's buildRenderPrimitives (sub-project 4 of the chem-core.js
// migration; see docs/superpowers/specs/2026-08-02-render-primitives-cpp-design.md). Pure,
// read-only transformation: EditableMolecule -> a typed render-state snapshot. No mutation, no
// undo/redo -- unlike editing operations, this is stateless with respect to the document's
// history.
//
// Deliberately NOT wired into the app yet -- stays standalone and tested, exactly like
// sub-projects 1-3, until dispatch-layer removal (sub-project 7).
//
// Documented gaps, not silently guessed at: r-groups and brackets (no data model exists in any
// prior sub-project -- AtomPrim::attachmentPoints is always 0, and there are no rgroups/brackets
// output fields at all), BondPrim::reactingCenterStatus (reaction-specific, always 0, no reaction
// objects exist in this port yet), BondPrim::cipLabel (always empty -- confirmed by direct probe
// that no bond-level E/Z CIP path exists through Indigo's public C API).

#include "EditableMolecule.h"
#include <QString>
#include <QList>
#include <QByteArray>
#include <QPointF>

struct AtomPrim {
    AtomId id;
    double x, y;
    QString label, element, color, atomicTitle, checkWarning;
    QString atomListElements;   // comma-joined symbols, only when isAtomList
    int charge = 0, isotope = 0, radical = 0, explicitValence = -1, aam = 0;
    int attachmentPoints = 0;   // r-group bitmask -- always 0, deferred with r-groups
    int stereoType = 0, stereoGroup = 0, atomicNum = 0, implicitHCount = 0;
    double atomicMass = 0;
    QString stereoLabel, cipLabel;
    bool isRGroup = false, hOnLeft = false, isAtomList = false, atomListNot = false;
    bool isSgroup = false;   // true for a contracted-sgroup synthetic entry
};

struct BondPrim {
    BondId id;
    AtomId begin, end;
    int type = 1;             // bond order, including Indigo's aromatic (4)
    int stereo = 0;
    bool inAromaticRing = false, invalidStereo = false, beginIsSgroup = false, endIsSgroup = false;
    QString checkWarning, cipLabel;   // cipLabel always empty -- see file header
    int reactingCenterStatus = 0;     // always 0 -- see file header
    bool hasRingCenter = false;
    double ringCenterX = 0, ringCenterY = 0;   // valid only if hasRingCenter
};

struct RingPrim {
    int id;
    QList<AtomId> atoms;
    double x, y, radius;
    bool isAromatic, hasBondType4;
};

struct SgroupPrim {
    int id;
    QString label;
    double x, y;
    AtomId attachAtomId;   // -1 if none
};

struct TextPrim { int id; double x, y; QString content; bool bold, italic; };
struct ImagePrim { int id; double x, y, w, h; QByteArray bitmap; };
struct RxnArrowPrim {
    int id;
    double p1x, p1y, p2x, p2y;
    QString mode;
    QString conditionsAbove, conditionsBelow;
    bool hasCurvature = false; double curvatureX = 0, curvatureY = 0;
};
struct RxnPlusPrim { int id; double x, y; };
struct MultitailArrowPrim {
    int id;
    double headX, headY;
    QList<QPointF> tails;
};

struct BBox { bool valid = false; double minX = 0, minY = 0, maxX = 0, maxY = 0; };

struct RenderPrimitives {
    QList<AtomPrim> atoms;
    QList<BondPrim> bonds;
    QList<RingPrim> rings;
    QList<SgroupPrim> sgroups;
    QList<TextPrim> texts;
    QList<ImagePrim> images;
    QList<RxnArrowPrim> rxnArrows;
    QList<RxnPlusPrim> rxnPluses;
    QList<MultitailArrowPrim> multitailArrows;
    BBox bbox;
    QString stereoFlagsType;
    int stereoFlagsGroupId = 0;
};

class RenderPrimitiveBuilder {
public:
    static RenderPrimitives build(const EditableMolecule& mol, bool showExplicitH);
};

#endif // RENDERPRIMITIVES_H
