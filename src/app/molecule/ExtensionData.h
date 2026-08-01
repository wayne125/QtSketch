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

struct TextAnnotation { double x = 0, y = 0; QString content; };          // _struct.texts (plain string; Lexical-JSON conversion is serialization's job, sub-project 5)
struct RxnArrow      { double x1 = 0, y1 = 0, x2 = 0, y2 = 0; };          // _struct.rxnArrows
struct RxnPlus       { double x = 0, y = 0; };                            // _struct.rxnPluses
struct MultitailArrow{ QList<double> headAndTails; };                     // _struct.multitailArrows (head x,y then tail x,y pairs)
struct ImageRef      { double x = 0, y = 0, w = 0, h = 0; QByteArray pngData; }; // _struct.images
struct AtomQueryList { QList<int> atomicNumbers; bool notList = false; };  // chem-core.js's atom.atomList sidecar (atom.label becomes the "L#" sentinel); NOT an Indigo concept -- plain molecule handles reject V2000 atom lists (sub-project 1 Test 6 finding)

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
    QHash<int, AtomQueryList> atomQueryLists;  // AtomId -> query list (keyed directly by the already-stable AtomId, no separate counter needed)

    TextId nextTextId = 1;
    RxnArrowId nextRxnArrowId = 1;
    RxnPlusId nextRxnPlusId = 1;
    MultitailArrowId nextMultitailArrowId = 1;
    ImageId nextImageId = 1;
};

#endif // EXTENSIONDATA_H
