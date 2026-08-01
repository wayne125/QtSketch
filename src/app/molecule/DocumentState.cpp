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
