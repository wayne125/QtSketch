// tests/document_state_test.cpp
// Standalone tests for DocumentState (chem-core.js migration, sub-project 2).
#include <cstdio>
#include "app/molecule/SelectionState.h"
#include "app/molecule/EditCommand.h"
#include "app/molecule/DocumentState.h"

static int g_pass = 0, g_fail = 0;
#define CHECK(cond, name) do { \
    if (cond) { ++g_pass; std::printf("[PASS] %s\n", name); } \
    else { ++g_fail; std::printf("[FAIL] %s (line %d)\n", name, __LINE__); } \
} while (0)

static void test_selectionStateBasics() {
    std::printf("--- Test 0: SelectionState basics ---\n");
    SelectionState s;
    CHECK(s.isEmpty(), "fresh SelectionState is empty");

    s.atoms.insert(1);
    s.atoms.insert(2);
    s.bonds.insert(5);
    s.rxnArrows.insert(9);
    s.rxnPluses.insert(3);
    s.multitailArrows.insert(7);
    CHECK(!s.isEmpty(), "non-empty after inserts");
    CHECK(s.atoms.size() == 2, "2 atoms in the set");
    CHECK(s.atoms.contains(1) && s.atoms.contains(2), "both atom ids present");

    s.clear();
    CHECK(s.isEmpty(), "empty after clear()");
    CHECK(s.atoms.isEmpty() && s.bonds.isEmpty() && s.rxnArrows.isEmpty()
          && s.rxnPluses.isEmpty() && s.multitailArrows.isEmpty(),
          "every individual set is empty after clear()");
}

static void test_editCommandBasics() {
    std::printf("--- Test 1: EditCommand basics ---\n");
    int counter = 0;
    EditCommand cmd;
    cmd.execute = [&counter]() { counter += 5; };
    cmd.invert = [&counter]() { counter -= 5; };

    cmd.execute();
    CHECK(counter == 5, "execute() runs the forward closure");
    cmd.invert();
    CHECK(counter == 0, "invert() runs the inverse closure");
}

static void test_documentStateUndoRedo() {
    std::printf("--- Test 2: DocumentState construction, undo/redo, dirty ---\n");
    DocumentState doc;
    CHECK(!doc.isDirty(), "fresh DocumentState is not dirty");
    CHECK(!doc.canUndo(), "fresh DocumentState cannot undo");
    CHECK(!doc.canRedo(), "fresh DocumentState cannot redo");

    EditableMolecule& mol = doc.molecule();
    AtomId addedId = -1;
    EditCommand addCarbon;
    addCarbon.execute = [&mol, &addedId]() {
        addedId = mol.addAtom(QStringLiteral("C"), 0.0, 0.0);
    };
    addCarbon.invert = [&mol, &addedId]() {
        mol.removeAtom(addedId);
        addedId = -1;
    };

    doc.executeCommand(addCarbon);
    CHECK(doc.molecule().atomCount() == 1, "atom added by the command");
    CHECK(doc.isDirty(), "dirty after executeCommand");
    CHECK(doc.canUndo(), "canUndo true after one command");
    CHECK(!doc.canRedo(), "canRedo false with nothing undone yet");

    doc.undo();
    CHECK(doc.molecule().atomCount() == 0, "atom removed by undo (invert ran)");
    CHECK(!doc.canUndo(), "canUndo false after undoing the only command");
    CHECK(doc.canRedo(), "canRedo true after an undo");

    doc.redo();
    CHECK(doc.molecule().atomCount() == 1, "atom re-added by redo (execute reran)");
    CHECK(doc.canUndo(), "canUndo true again after redo");
    CHECK(!doc.canRedo(), "canRedo false again after redoing everything");

    doc.markClean();
    CHECK(!doc.isDirty(), "markClean() clears the dirty flag");

    EditCommand addOxygen;
    addOxygen.execute = [&mol]() { mol.addAtom(QStringLiteral("O"), 1.0, 0.0); };
    addOxygen.invert = [&mol]() { /* not exercised in this check */ };
    doc.executeCommand(addOxygen);
    CHECK(doc.isDirty(), "dirty again after a command following markClean");

    doc.undo();
    CHECK(doc.canRedo(), "redo available right after an undo");
    EditCommand addNitrogen;
    addNitrogen.execute = [&mol]() { mol.addAtom(QStringLiteral("N"), 2.0, 0.0); };
    addNitrogen.invert = [&mol]() { /* not exercised in this check */ };
    doc.executeCommand(addNitrogen);
    CHECK(!doc.canRedo(), "old redo branch truncated by executing a new command");
}

static void test_documentStateHistoryCap() {
    std::printf("--- Test 3: DocumentState history cap at 50 ---\n");
    DocumentState doc;
    EditableMolecule& mol = doc.molecule();
    for (int i = 0; i < 51; ++i) {
        EditCommand cmd;
        cmd.execute = [&mol]() { mol.addAtom(QStringLiteral("C"), 0.0, 0.0); };
        cmd.invert = [&mol]() {
            QList<AtomId> ids = mol.atomIds();
            if (!ids.isEmpty()) mol.removeAtom(ids.last());
        };
        doc.executeCommand(cmd);
    }
    CHECK(doc.molecule().atomCount() == 51, "51 atoms added");
    int undoCount = 0;
    while (doc.canUndo() && undoCount <= 51) {
        doc.undo();
        ++undoCount;
    }
    CHECK(undoCount == 50, "at most 50 undos possible after 51 commands (history capped)");
}

int main() {
    test_selectionStateBasics();
    test_editCommandBasics();
    test_documentStateUndoRedo();
    test_documentStateHistoryCap();
    std::printf("Summary: %d passed, %d failed.\n", g_pass, g_fail);
    return g_fail == 0 ? 0 : 1;
}
