// src/app/molecule/SelectionState.h
#ifndef SELECTIONSTATE_H
#define SELECTIONSTATE_H

// The C++ replacement for src/worker/10-state.js's _selection object (sub-
// project 2 of the chem-core.js migration; see
// docs/superpowers/specs/2026-08-01-document-state-cpp-design.md).
//
// Covers exactly the 5 entity types the real _selection object covers --
// confirmed by direct source read of 10-state.js. Deliberately does NOT
// include texts or images: every real selectItem()/selectAll() call site in
// that file only ever populates atom_ids/bond_ids/rxnArrow_ids/rxnPlus_ids/
// multitailArrow_ids. Texts and images use a separate, UI-side selection
// mechanism entirely (e.g. ChemCanvas.qml's own selectedImageId).
//
// Also deliberately has NO bbox field: the real _selection.bbox is set to
// `null` in every single code path in 10-state.js -- it is never populated,
// dead weight, not ported.

#include "EditableMolecule.h"
#include "ExtensionData.h"
#include <QSet>

struct SelectionState {
    QSet<AtomId> atoms;
    QSet<BondId> bonds;
    QSet<RxnArrowId> rxnArrows;
    QSet<RxnPlusId> rxnPluses;
    QSet<MultitailArrowId> multitailArrows;

    bool isEmpty() const {
        return atoms.isEmpty() && bonds.isEmpty() && rxnArrows.isEmpty()
            && rxnPluses.isEmpty() && multitailArrows.isEmpty();
    }
    void clear() {
        atoms.clear(); bonds.clear(); rxnArrows.clear();
        rxnPluses.clear(); multitailArrows.clear();
    }
};

#endif // SELECTIONSTATE_H
