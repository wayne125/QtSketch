// src/app/molecule/ClipboardPreview.h
#ifndef CLIPBOARDPREVIEW_H
#define CLIPBOARDPREVIEW_H

// Read-only, non-mutating preview of arbitrary MOL-format text (chem-core.js migration,
// sub-project 5d; see docs/superpowers/specs/2026-08-03-clipboard-copypaste-cpp-design.md). C++
// replacement for 40-serialize.js's getClipboardPreview. Standalone -- doesn't fit naturally on
// EditableMolecule (which owns a live, mutable document), DocumentState (undo/redo/selection),
// or SdfBatch (a staged multi-record import list with a different, normalized-thumbnail
// coordinate concept) -- none of those classes have a "parse arbitrary external text into a flat
// preview" responsibility.

#include <QString>
#include <QList>

struct ClipboardPreviewAtom { double x, y; QString label; };
struct ClipboardPreviewBond { double x1, y1, x2, y2; };
struct ClipboardPreview {
    QList<ClipboardPreviewAtom> atoms;
    QList<ClipboardPreviewBond> bonds;
    // RAW (unnormalized) bbox center -- NOT scaled into a unit box like SdfBatch::Thumbnail. A
    // paste-preview overlay needs real document coordinates. Defaults to (0, 0) if the molecule
    // has zero atoms, matching the real getClipboardPreview's `cx = minX !== null ? ... : 0`.
    double cx = 0, cy = 0;
};

ClipboardPreview buildClipboardPreview(const QString& molfileText);

#endif // CLIPBOARDPREVIEW_H
