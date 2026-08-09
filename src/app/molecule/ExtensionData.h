// src/app/molecule/ExtensionData.h
#ifndef EXTENSIONDATA_H
#define EXTENSIONDATA_H

// Everything the app's document model carries that Indigo's molecule object
// does NOT represent. Mirrors the _struct.* members the JS worker uses today
// (audit list in the sub-project 1 design spec). Value type: snapshots copy
// it wholesale.
//
// ID design (sub-project 2): texts/rxnArrows/rxnPluses/multitailArrows/images
// use the same stable, monotonic, NEVER-reused-counter pattern as AtomId/BondId
// (see EditableMolecule.h) -- an undo-history entry holding a stale id must
// never later alias a different, newer entity after an earlier one is removed.

#include <QString>
#include <QList>
#include <QHash>
#include <QByteArray>

using TextId = int;
using RxnArrowId = int;
using RxnPlusId = int;
using MultitailArrowId = int;
using ImageId = int;

struct TextAnnotation { double x = 0, y = 0; QString content; bool bold = false, italic = false; };   // _struct.texts (plain string; Lexical-JSON conversion is serialization's job, sub-project 5)
struct RxnArrow {
    double x1 = 0, y1 = 0, x2 = 0, y2 = 0;
    QString mode = QStringLiteral("filled-triangle");   // buildRenderPrimitives' arr.mode || "filled-triangle"
    QString conditionsAbove, conditionsBelow;           // arr.conditionsText || {above:"", below:""}
    bool hasCurvature = false;
    double curvatureX = 0, curvatureY = 0;              // arr.curvature || null
};   // _struct.rxnArrows
struct RxnPlus       { double x = 0, y = 0; };                            // _struct.rxnPluses
struct MultitailArrow{ QList<double> headAndTails; };                     // _struct.multitailArrows (head x,y then tail x,y pairs)
struct ImageRef      { double x = 0, y = 0, w = 0, h = 0; QByteArray pngData; }; // _struct.images
struct AtomQueryList { QList<int> atomicNumbers; bool notList = false; };  // chem-core.js's atom.atomList sidecar (atom.label becomes the "L#" sentinel); NOT an Indigo concept -- plain molecule handles reject V2000 atom lists (sub-project 1 Test 6 finding)

struct RGroupEntry {   // _struct.rgroups entry (50-reactions.js's _makeRGroupEntry)
    QList<int> fragIds;     // member fragment indices; ordered, manually deduped (mirrors Pile.add())
    QString range;
    bool resth = false;
    int ifthen = 0;
    // Deliberately NO `index` field -- the real entry's `index` is always identical to the
    // `_struct.rgroups` map key it's stored under, pure redundancy, trivially recoverable from
    // the QHash<int, RGroupEntry> key below whenever sub-project 5 (serialization) needs it.
};
struct BracketBox { double minX = 0, minY = 0, maxX = 0, maxY = 0; };   // _struct.brackets entry

struct ExtensionData {
    QString name;                          // _struct.name (user-typed label)
    QHash<TextId, TextAnnotation> texts;
    QHash<RxnArrowId, RxnArrow> rxnArrows;
    QHash<RxnPlusId, RxnPlus> rxnPluses;
    QHash<MultitailArrowId, MultitailArrow> multitailArrows;
    QHash<ImageId, ImageRef> images;
    QHash<int, int> stereoFlags;           // fragmentIndex -> ABS/AND/OR flag (_struct.stereoFlags)
    QHash<int, int> atomAAM;               // AtomId -> atom-atom-mapping number
    QHash<int, bool> atomCheckWarnings;    // AtomId -> structure-check warning
    QString stereoFlagsType = QStringLiteral("abs");   // _struct.stereoFlags.type -- document-level,
    int stereoFlagsGroupId = 0;                        // NOT the same concept as the stereoFlags QHash above
    QHash<int, QString> atomCheckWarningTexts;         // AtomId -> real string warning type (see below)
    QHash<int, QString> bondCheckWarningTexts;         // BondId -> real string warning type
    QHash<int, QString> atomStereoCipLabels;   // AtomId -> external CIP-perception cipLabel (R/S/r/s),
                                                // DISTINCT from the live atomCipDescriptor accessor
    QHash<int, int> atomStereoTypes;           // AtomId -> external stereoType (INDIGO_ABS/OR/AND/EITHER)
    QHash<int, int> atomStereoGroups;          // AtomId -> external stereoGroup number
    QHash<int, QString> bondStereoCipLabels;   // BondId -> external CIP-perception cipLabel (E/Z)
    QHash<int, AtomQueryList> atomQueryLists;  // AtomId -> query list (keyed directly by the already-stable AtomId, no separate counter needed)
    QHash<int, RGroupEntry> rgroups;       // keyed by R-group NUMBER (e.g. 1 for R1), not a counter
    QList<BracketBox> brackets;            // push/pop-only stack, no id (matches _struct.brackets.push/.pop)

    TextId nextTextId = 1;
    RxnArrowId nextRxnArrowId = 1;
    RxnPlusId nextRxnPlusId = 1;
    MultitailArrowId nextMultitailArrowId = 1;
    ImageId nextImageId = 1;
};

#endif // EXTENSIONDATA_H
