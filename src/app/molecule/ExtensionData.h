// src/app/molecule/ExtensionData.h
#ifndef EXTENSIONDATA_H
#define EXTENSIONDATA_H

// Everything the app's document model carries that Indigo's molecule object
// does NOT represent. Mirrors the _struct.* members the JS worker uses today
// (audit list in the design spec). Value type: snapshots copy it wholesale.

#include <QString>
#include <QList>
#include <QHash>
#include <QByteArray>

struct TextAnnotation { double x = 0, y = 0; QString content; };          // _struct.texts (plain string; Lexical-JSON conversion is serialization's job, sub-project 5)
struct RxnArrow      { double x1 = 0, y1 = 0, x2 = 0, y2 = 0; };          // _struct.rxnArrows
struct RxnPlus       { double x = 0, y = 0; };                            // _struct.rxnPluses
struct MultitailArrow{ QList<double> headAndTails; };                     // _struct.multitailArrows (head x,y then tail x,y pairs)
struct ImageRef      { double x = 0, y = 0, w = 0, h = 0; QByteArray pngData; }; // _struct.images

struct ExtensionData {
    QString name;                          // _struct.name (user-typed label)
    QList<TextAnnotation> texts;
    QList<RxnArrow> rxnArrows;
    QList<RxnPlus> rxnPluses;
    QList<MultitailArrow> multitailArrows;
    QList<ImageRef> images;
    QHash<int, int> stereoFlags;           // fragmentIndex -> ABS/AND/OR flag (_struct.stereoFlags)
    QHash<int, int> atomAAM;               // AtomId -> atom-atom-mapping number
    QHash<int, bool> atomCheckWarnings;    // AtomId -> structure-check warning
};

#endif // EXTENSIONDATA_H
