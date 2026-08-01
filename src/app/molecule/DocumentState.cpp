// src/app/molecule/DocumentState.cpp
#include "DocumentState.h"

DocumentState::DocumentState(const QString& initialStructure)
    : m_molecule(initialStructure) {
}

EditableMolecule& DocumentState::molecule() {
    return m_molecule;
}

void DocumentState::executeCommand(EditCommand cmd) {
    // Remove any future redo states -- matches executeCommand's
    // `_history.splice(_historyPointer + 1)` exactly.
    if (m_historyPointer + 1 < static_cast<int>(m_history.size())) {
        m_history.resize(m_historyPointer + 1);
    }
    cmd.execute();
    m_history.push_back(std::move(cmd));
    ++m_historyPointer;
    if (static_cast<int>(m_history.size()) > kHistorySize) {
        m_history.erase(m_history.begin());
        --m_historyPointer;
    }
    m_dirty = true;
}

void DocumentState::undo() {
    if (m_historyPointer < 0) return;
    m_history[m_historyPointer].invert();
    --m_historyPointer;
    m_selection.clear();
    m_dirty = true;
}

void DocumentState::redo() {
    if (m_historyPointer >= static_cast<int>(m_history.size()) - 1) return;
    ++m_historyPointer;
    m_history[m_historyPointer].execute();
    m_selection.clear();
    m_dirty = true;
}

bool DocumentState::isDirty() const {
    return m_dirty;
}

void DocumentState::markClean() {
    m_dirty = false;
}

bool DocumentState::canUndo() const {
    return m_historyPointer >= 0;
}

bool DocumentState::canRedo() const {
    return m_historyPointer < static_cast<int>(m_history.size()) - 1;
}

void DocumentState::selectAtom(AtomId id) {
    m_selection.clear();
    m_selection.atoms.insert(id);
}
void DocumentState::selectBond(BondId id) {
    m_selection.clear();
    m_selection.bonds.insert(id);
}
void DocumentState::selectRxnArrow(RxnArrowId id) {
    m_selection.clear();
    m_selection.rxnArrows.insert(id);
}
void DocumentState::selectRxnPlus(RxnPlusId id) {
    m_selection.clear();
    m_selection.rxnPluses.insert(id);
}
void DocumentState::selectMultitailArrow(MultitailArrowId id) {
    m_selection.clear();
    m_selection.multitailArrows.insert(id);
}
void DocumentState::clearSelection() {
    m_selection.clear();
}

void DocumentState::addAtomToSelection(AtomId id) { m_selection.atoms.insert(id); }
void DocumentState::addBondToSelection(BondId id) { m_selection.bonds.insert(id); }
void DocumentState::addRxnArrowToSelection(RxnArrowId id) { m_selection.rxnArrows.insert(id); }
void DocumentState::addRxnPlusToSelection(RxnPlusId id) { m_selection.rxnPluses.insert(id); }
void DocumentState::addMultitailArrowToSelection(MultitailArrowId id) { m_selection.multitailArrows.insert(id); }

void DocumentState::removeAtomFromSelection(AtomId id) { m_selection.atoms.remove(id); }
void DocumentState::removeBondFromSelection(BondId id) { m_selection.bonds.remove(id); }
void DocumentState::removeRxnArrowFromSelection(RxnArrowId id) { m_selection.rxnArrows.remove(id); }
void DocumentState::removeRxnPlusFromSelection(RxnPlusId id) { m_selection.rxnPluses.remove(id); }
void DocumentState::removeMultitailArrowFromSelection(MultitailArrowId id) { m_selection.multitailArrows.remove(id); }

void DocumentState::selectAll() {
    m_selection.clear();
    for (AtomId id : m_molecule.atomIds()) m_selection.atoms.insert(id);
    for (BondId id : m_molecule.bondIds()) m_selection.bonds.insert(id);
    for (RxnArrowId id : m_molecule.rxnArrowIds()) m_selection.rxnArrows.insert(id);
    for (RxnPlusId id : m_molecule.rxnPlusIds()) m_selection.rxnPluses.insert(id);
    for (MultitailArrowId id : m_molecule.multitailArrowIds()) m_selection.multitailArrows.insert(id);
}

SelectionState& DocumentState::selection() {
    return m_selection;
}
