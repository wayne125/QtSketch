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

    // Task 6 adds: selectAtom/selectBond/selectRxnArrow/selectRxnPlus/
    // selectMultitailArrow/clearSelection/add*ToSelection/remove*FromSelection/
    // selectAll/selection().

private:
    EditableMolecule m_molecule;
    std::vector<EditCommand> m_history;
    int m_historyPointer = -1;
    bool m_dirty = false;
    SelectionState m_selection;
    static constexpr int kHistorySize = 50;
};

#endif // DOCUMENTSTATE_H
