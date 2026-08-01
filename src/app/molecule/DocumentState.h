// src/app/molecule/DocumentState.h
#ifndef DOCUMENTSTATE_H
#define DOCUMENTSTATE_H

// One per open document: owns the EditableMolecule, the undo/redo history,
// and the current selection. C++ replacement for src/worker/10-state.js's
// module-level _struct/_history/_historyPointer/_selection/_dirty state
// (sub-project 2 of the chem-core.js migration; see
// docs/superpowers/specs/2026-08-01-document-state-cpp-design.md).
//
// executeCommand/undo/redo are a direct line-for-line port of the three real
// JS functions of the same name, confirmed by direct source read: truncate
// any redo tail, run/invert the command, cap history at kHistorySize
// (dropping the oldest and compensating the pointer), clear selection on
// undo/redo.

#include "EditableMolecule.h"
#include "EditCommand.h"
#include "SelectionState.h"
#include <vector>

class DocumentState {
public:
    explicit DocumentState(const QString& initialStructure = {});

    EditableMolecule& molecule();

    void executeCommand(EditCommand cmd);
    void undo();
    void redo();
    bool isDirty() const;
    // Grounded requirement (not speculative): src/worker/30-templates.js's
    // loadBenzene() explicitly does `init(); addBenzeneRing(); _dirty = false`
    // -- a new document pre-populated with the starter benzene is NOT dirty.
    void markClean();
    bool canUndo() const;
    bool canRedo() const;

    void selectAtom(AtomId id);
    void selectBond(BondId id);
    void selectRxnArrow(RxnArrowId id);
    void selectRxnPlus(RxnPlusId id);
    void selectMultitailArrow(MultitailArrowId id);
    void clearSelection();

    void addAtomToSelection(AtomId id);
    void addBondToSelection(BondId id);
    void addRxnArrowToSelection(RxnArrowId id);
    void addRxnPlusToSelection(RxnPlusId id);
    void addMultitailArrowToSelection(MultitailArrowId id);

    void removeAtomFromSelection(AtomId id);
    void removeBondFromSelection(BondId id);
    void removeRxnArrowFromSelection(RxnArrowId id);
    void removeRxnPlusFromSelection(RxnPlusId id);
    void removeMultitailArrowFromSelection(MultitailArrowId id);

    // Selects every current atom/bond/rxnArrow/rxnPlus/multitailArrow. KNOWN
    // SIMPLIFICATION: the real 10-state.js selectAll() excludes atoms hidden
    // inside a contracted SUP functional-group sgroup (selecting the sgroup's
    // own pill id instead) -- that needs sgroup-contraction awareness
    // (type/isExpanded) EditableMolecule doesn't have yet. This selectAll()
    // selects raw atoms/bonds with no sgroup-contraction logic; revisit once
    // a later sub-project models contracted sgroups.
    void selectAll();
    SelectionState& selection();

    AtomId addAtom(const QString& symbol, double x, double y);
    BondId addBond(AtomId a, AtomId b, int order);
    void deleteAtom(AtomId id);   // plain-atom-and-incident-bonds case only; see the
                                  // sgroup-membership spike documented in EditableMolecule.h
                                  // for why the sgroup-aware branch is deferred
    void deleteBond(BondId id);

    void changeAtomLabel(AtomId id, const QString& newLabel);
    void setAtomMapping(AtomId id, int mappingNumber);
    void changeAtomCharge(AtomId id, int newCharge);
    void setAttachmentPoint(AtomId id, int order);
    void changeAtomIsotope(AtomId id, int isotope);
    void changeAtomRadical(AtomId id, int radical);
    void changeAtomValence(AtomId id, int valence);

    struct AtomProperties { QString label; int charge; int isotope; int radical; int explicitValence; };
    AtomProperties atomProperties(AtomId id) const;

    void changeBondOrder(BondId id, int newOrder);

    void setAtomQueryList(AtomId id, const QString& labelsCsv, bool notList);
    // KNOWN LIMITATION: undo after clearAtomQueryList does NOT restore the
    // original query list (EditableMolecule has no set-list-from-numbers
    // entry point bypassing the CSV parse) -- it only re-clears on redo.
    // Revisit if a later sub-project needs this restored exactly.
    void clearAtomQueryList(AtomId id, const QString& fallbackLabel);

    // Discrete transforms of the selected ATOMS about their unweighted
    // centroid -- a direct port of 20-edit.js's transformSelection(mode),
    // split into four named methods instead of its JS mode-string dispatch.
    // Each no-ops below 2 selected atoms, matching the real ids.length < 2
    // guard, and pushes exactly one undoable command.
    void rotateSelection90CW();
    void rotateSelection90CCW();
    void flipSelectionHorizontal();
    void flipSelectionVertical();

    // Per-image transforms (20-edit.js has none; these port 50-reactions.js's
    // moveImage/resizeImage). Take an explicit id rather than reading the
    // selection -- images are deliberately not a SelectionState entity type.
    // Both commit immediately as one undoable command; there is no live-drag
    // protocol for images in the real code either.
    void moveImage(ImageId id, double dx, double dy);
    void resizeImage(ImageId id, double scaleFactor);

private:
    enum class DiscreteTransform { RotateCW, RotateCCW, FlipH, FlipV };
    void applyDiscreteTransform(DiscreteTransform mode);

    EditableMolecule m_molecule;
    std::vector<EditCommand> m_history;
    int m_historyPointer = -1;
    bool m_dirty = false;
    SelectionState m_selection;
    static constexpr int kHistorySize = 50;
};

#endif // DOCUMENTSTATE_H
