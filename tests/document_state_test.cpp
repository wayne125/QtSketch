// tests/document_state_test.cpp
// Standalone tests for DocumentState (chem-core.js migration, sub-project 2).
#include <cstdio>
#include <cmath>
#include <algorithm>
#include "app/molecule/SelectionState.h"
#include "app/molecule/EditCommand.h"
#include "app/molecule/DocumentState.h"
#include "app/molecule/TemplateLibrary.h"
#include "indigo.h"

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

static void test_documentStateEditingOperations() {
    std::printf("--- Test 5: DocumentState editing operations ---\n");
    DocumentState doc;

    // addAtom / addBond / deleteBond / deleteAtom, with undo/redo.
    AtomId a1 = doc.addAtom(QStringLiteral("C"), 0, 0);
    AtomId a2 = doc.addAtom(QStringLiteral("O"), 1, 0);
    CHECK(doc.molecule().atomCount() == 2, "2 atoms added via DocumentState::addAtom");
    BondId b1 = doc.addBond(a1, a2, 1);
    CHECK(doc.molecule().bondCount() == 1, "bond added via DocumentState::addBond");

    doc.deleteBond(b1);
    CHECK(doc.molecule().bondCount() == 0, "deleteBond removes the bond");
    doc.undo();
    CHECK(doc.molecule().bondCount() == 1, "undo restores the deleted bond");

    doc.deleteAtom(a2);
    CHECK(doc.molecule().atomCount() == 1, "deleteAtom removes the atom");
    CHECK(doc.molecule().bondCount() == 0, "deleteAtom also removes its incident bond");
    doc.undo();
    CHECK(doc.molecule().atomCount() == 2, "undo restores the deleted atom");
    CHECK(doc.molecule().bondCount() == 1, "undo restores the incident bond too");

    // deleteAtom's undo recreates the atom under a NEW id (documented
    // limitation -- EditableMolecule has no restore-at-exact-id primitive),
    // so `a2`'s original value is now stale. Re-derive the current id of
    // the recreated oxygen atom for the rest of this test.
    for (AtomId id : doc.molecule().atomIds()) {
        if (id != a1) a2 = id;
    }

    // changeAtomLabel: guarded (no-op when unchanged).
    doc.changeAtomLabel(a1, QStringLiteral("N"));
    CHECK(doc.molecule().atomSymbol(a1) == QStringLiteral("N"), "changeAtomLabel changes the symbol");
    bool canUndoBeforeNoOpLabel = doc.canUndo();
    doc.changeAtomLabel(a1, QStringLiteral("N")); // same label: must no-op, no new history entry
    CHECK(doc.canUndo() == canUndoBeforeNoOpLabel, "changeAtomLabel to the SAME label pushes no history entry");
    doc.undo();
    CHECK(doc.molecule().atomSymbol(a1) == QStringLiteral("C"), "undo restores the original label");

    // changeAtomCharge: guarded.
    doc.changeAtomCharge(a1, 1);
    CHECK(doc.molecule().atomCharge(a1) == 1, "changeAtomCharge sets the charge");
    doc.undo();
    CHECK(doc.molecule().atomCharge(a1) == 0, "undo restores the original charge (0)");

    // changeAtomIsotope: NOT guarded (real JS always executes, even for an
    // unchanged value). Proof: call it twice with the SAME value, then undo
    // twice -- if it were guarded, the second call would push no history
    // entry and a single undo would already reach isotope 0.
    doc.changeAtomIsotope(a1, 5);
    CHECK(doc.molecule().atomIsotope(a1) == 5, "changeAtomIsotope sets isotope to 5");
    doc.changeAtomIsotope(a1, 5); // same value again -- must still push a history entry
    doc.undo();
    CHECK(doc.molecule().atomIsotope(a1) == 5,
          "first undo only undoes the second (no-op-value) call -- isotope still 5, proving it WAS pushed");
    doc.undo();
    CHECK(doc.molecule().atomIsotope(a1) == 0, "second undo undoes the real change, back to 0");

    // changeAtomRadical / changeAtomValence: real end-to-end undo of an actual change.
    doc.changeAtomRadical(a1, INDIGO_SINGLET);
    CHECK(doc.molecule().atomRadical(a1) == INDIGO_SINGLET, "changeAtomRadical sets the radical");
    doc.undo();
    CHECK(doc.molecule().atomRadical(a1) == 0, "undo restores radical to 0");

    doc.changeAtomValence(a1, 4);
    CHECK(doc.molecule().atomExplicitValence(a1) == 4, "changeAtomValence sets explicit valence");
    doc.undo();
    CHECK(doc.molecule().atomExplicitValence(a1) == -1, "undo restores valence to unset (-1)");

    // setAttachmentPoint: NOT guarded.
    doc.setAttachmentPoint(a1, 1);
    CHECK(doc.molecule().atomAttachmentOrder(a1) == 1, "setAttachmentPoint sets order 1");
    doc.undo();
    CHECK(doc.molecule().atomAttachmentOrder(a1) == 0, "undo restores attachment order to 0 (none)");

    // setAtomMapping: guarded.
    doc.setAtomMapping(a1, 5);
    CHECK(doc.molecule().atomAAM(a1) == 5, "setAtomMapping sets the AAM number");
    doc.undo();
    CHECK(doc.molecule().atomAAM(a1) == 0, "undo restores AAM to 0");

    // atomProperties: read-only, not a command.
    doc.changeAtomCharge(a1, 2);
    DocumentState::AtomProperties props = doc.atomProperties(a1);
    CHECK(props.label == QStringLiteral("C"), "atomProperties reports the current label");
    CHECK(props.charge == 2, "atomProperties reports the current charge");

    // changeBondOrder: guarded. Reuses the bond deleteAtom's undo already
    // restored between a1 and (the recreated) a2 -- adding another bond on
    // top of it would fail, since Indigo rejects a duplicate parallel edge
    // between the same atom pair (probe-confirmed: "already have edge
    // between vertices").
    BondId liveBond = doc.molecule().bondIds().first();
    doc.changeBondOrder(liveBond, 3);
    CHECK(doc.molecule().bondOrder(liveBond) == 3, "changeBondOrder sets the new order");
    doc.undo();
    CHECK(doc.molecule().bondOrder(liveBond) == 1, "undo restores the original order");

    // setAtomQueryList / clearAtomQueryList.
    doc.setAtomQueryList(a2, QStringLiteral("C,N"), false);
    CHECK(doc.molecule().hasAtomQueryList(a2), "setAtomQueryList sets the query list");
    doc.undo();
    CHECK(!doc.molecule().hasAtomQueryList(a2), "undo removes the query list");
    CHECK(doc.molecule().atomSymbol(a2) == QStringLiteral("O"), "undo restores the original label too");
}

static void test_discreteTransforms() {
    std::printf("--- Test 6: discrete 90-degree rotate / flip ---\n");

    // NOTE on seeding: these tests create entities via doc.molecule().addAtom(...)
    // rather than doc.addAtom(...). The latter is itself undoable, which would
    // leave history non-empty and make canUndo() -- a plain bool -- useless for
    // asserting "this operation pushed exactly one entry". Seeding through the
    // molecule directly keeps history empty, so canUndo() is an exact signal.

    // Two atoms at (0,0) and (2,0) -> centroid (1,0).
    {
        DocumentState doc;
        AtomId a1 = doc.molecule().addAtom(QStringLiteral("C"), 0, 0);
        AtomId a2 = doc.molecule().addAtom(QStringLiteral("C"), 2, 0);
        doc.selectAtom(a1);
        doc.addAtomToSelection(a2);
        CHECK(!doc.canUndo(), "history starts empty (atoms seeded via the molecule)");

        doc.rotateSelection90CW();
        CHECK(doc.canUndo(), "rotate CW pushes exactly one history entry");
        double x = 0, y = 0;
        // rotate_cw: (cx + (y-cy), cy - (x-cx)); for a1 (0,0): (1+0, 0-(-1)) = (1,1)
        CHECK(doc.molecule().atomPos(a1, x, y) && x == 1 && y == 1, "rotate CW moves a1 to (1,1)");
        // for a2 (2,0): (1+0, 0-(1)) = (1,-1)
        CHECK(doc.molecule().atomPos(a2, x, y) && x == 1 && y == -1, "rotate CW moves a2 to (1,-1)");
        doc.undo();
        CHECK(doc.molecule().atomPos(a1, x, y) && x == 0 && y == 0, "undo restores a1 exactly");
        CHECK(doc.molecule().atomPos(a2, x, y) && x == 2 && y == 0, "undo restores a2 exactly");
        CHECK(!doc.canUndo(), "history is empty again after undoing the only entry");
    }

    {
        DocumentState doc;
        AtomId a1 = doc.molecule().addAtom(QStringLiteral("C"), 0, 0);
        AtomId a2 = doc.molecule().addAtom(QStringLiteral("C"), 2, 0);
        doc.selectAtom(a1);
        doc.addAtomToSelection(a2);

        doc.rotateSelection90CCW();
        double x = 0, y = 0;
        // rotate_ccw: (cx - (y-cy), cy + (x-cx)); for a1 (0,0): (1-0, 0+(-1)) = (1,-1)
        CHECK(doc.molecule().atomPos(a1, x, y) && x == 1 && y == -1, "rotate CCW moves a1 to (1,-1)");
        CHECK(doc.molecule().atomPos(a2, x, y) && x == 1 && y == 1, "rotate CCW moves a2 to (1,1)");
    }

    {
        DocumentState doc;
        AtomId a1 = doc.molecule().addAtom(QStringLiteral("C"), 0, 3);
        AtomId a2 = doc.molecule().addAtom(QStringLiteral("C"), 2, 5);
        doc.selectAtom(a1);
        doc.addAtomToSelection(a2);
        // centroid (1,4). flip_h: (2*1 - x, y). flip_v: (x, 2*4 - y).
        doc.flipSelectionHorizontal();
        double x = 0, y = 0;
        CHECK(doc.molecule().atomPos(a1, x, y) && x == 2 && y == 3, "flip H mirrors a1 across cx");
        CHECK(doc.molecule().atomPos(a2, x, y) && x == 0 && y == 5, "flip H mirrors a2 across cx");
        doc.undo();
        CHECK(doc.molecule().atomPos(a1, x, y) && x == 0 && y == 3, "undo restores a1 after flip H");

        doc.selectAtom(a1);
        doc.addAtomToSelection(a2);
        doc.flipSelectionVertical();
        CHECK(doc.molecule().atomPos(a1, x, y) && x == 0 && y == 5, "flip V mirrors a1 across cy");
        CHECK(doc.molecule().atomPos(a2, x, y) && x == 2 && y == 3, "flip V mirrors a2 across cy");
    }

    // Fewer than 2 selected atoms: no-op, no history entry (real code's ids.length < 2 guard).
    {
        DocumentState doc;
        AtomId a1 = doc.molecule().addAtom(QStringLiteral("C"), 7, 7);
        doc.selectAtom(a1);
        doc.rotateSelection90CW();
        CHECK(!doc.canUndo(), "single-atom selection pushes no history entry");
        double x = 0, y = 0;
        CHECK(doc.molecule().atomPos(a1, x, y) && x == 7 && y == 7, "single-atom selection is unmoved");

        doc.clearSelection();
        doc.flipSelectionHorizontal();
        CHECK(!doc.canUndo(), "empty selection pushes no history entry");
    }
}

static void test_imageTransforms() {
    std::printf("--- Test 7: image move / resize ---\n");
    DocumentState doc;
    ImageId img = doc.molecule().addImage(1, 2, 4, 3, QByteArray("fakepng"));

    double x = 0, y = 0, w = 0, h = 0;
    doc.moveImage(img, 2.5, -1.5);
    CHECK(doc.molecule().imageRect(img, x, y, w, h) && x == 3.5 && y == 0.5,
          "moveImage offsets the image position");
    CHECK(w == 4 && h == 3, "moveImage leaves the size untouched");
    doc.undo();
    CHECK(doc.molecule().imageRect(img, x, y, w, h) && x == 1 && y == 2,
          "undo restores the original image position exactly");

    doc.resizeImage(img, 2.0);
    CHECK(doc.molecule().imageRect(img, x, y, w, h) && w == 8 && h == 6,
          "resizeImage scales the image size");
    CHECK(x == 1 && y == 2, "resizeImage leaves the position untouched");
    doc.undo();
    CHECK(doc.molecule().imageRect(img, x, y, w, h) && w == 4 && h == 3,
          "undo restores the original image size exactly");

    // Both undos above emptied the history, so !canUndo() is an exact signal here.
    CHECK(!doc.canUndo(), "history is empty after undoing both image commands");

    // scaleFactor 0 would make invert's 1/0 non-finite: guarded no-op.
    doc.resizeImage(img, 0.0);
    CHECK(!doc.canUndo(), "resizeImage(0) pushes no history entry");
    CHECK(doc.molecule().imageRect(img, x, y, w, h) && w == 4 && h == 3,
          "resizeImage(0) leaves the size unchanged");

    // Invalid ids no-op cleanly.
    doc.moveImage(9999, 1, 1);
    doc.resizeImage(9999, 2.0);
    CHECK(!doc.canUndo(), "invalid ImageId pushes no history entry");
}

static void test_liveDragMove() {
    std::printf("--- Test 8: live-drag move gesture ---\n");
    // Seeded via doc.molecule() so history starts empty -- see the seeding note
    // in test_discreteTransforms.
    DocumentState doc;
    AtomId a1 = doc.molecule().addAtom(QStringLiteral("C"), 0, 0);
    AtomId a2 = doc.molecule().addAtom(QStringLiteral("C"), 2, 0);
    doc.selectAtom(a1);
    doc.addAtomToSelection(a2);

    doc.moveSelectionLive(1, 1);
    doc.moveSelectionLive(0.5, -0.25);
    double x = 0, y = 0;
    CHECK(doc.molecule().atomPos(a1, x, y) && x == 1.5 && y == 0.75,
          "live move accumulates increments onto current positions");
    CHECK(!doc.canUndo(), "live move calls push NO history entry");

    doc.commitMove();
    CHECK(doc.canUndo(), "commitMove pushes exactly one history entry");
    CHECK(doc.molecule().atomPos(a1, x, y) && x == 1.5 && y == 0.75,
          "commitMove leaves the live-applied position in place (isFirst guard)");

    doc.undo();
    CHECK(doc.molecule().atomPos(a1, x, y) && x == 0 && y == 0, "undo restores a1 exactly");
    CHECK(doc.molecule().atomPos(a2, x, y) && x == 2 && y == 0, "undo restores a2 exactly");
    doc.redo();
    CHECK(doc.molecule().atomPos(a1, x, y) && x == 1.5 && y == 0.75, "redo re-applies the move");

    // commitMove with no movement pushes nothing.
    {
        DocumentState d2;
        AtomId b1 = d2.molecule().addAtom(QStringLiteral("C"), 0, 0);
        d2.selectAtom(b1);
        d2.commitMove();
        CHECK(!d2.canUndo(), "commitMove with zero delta pushes no history entry");
    }

    // Page-boundary clamp: the whole selection stops together at the edge.
    {
        DocumentState d3;
        AtomId c1 = d3.molecule().addAtom(QStringLiteral("C"), 0, 0);
        AtomId c2 = d3.molecule().addAtom(QStringLiteral("C"), 10, 0);
        d3.selectAtom(c1);
        d3.addAtomToSelection(c2);
        d3.moveSelectionLive(1000, 0);   // far past kPageMaxX (30)
        double cx1 = 0, cy1 = 0, cx2 = 0, cy2 = 0;
        d3.molecule().atomPos(c1, cx1, cy1);
        d3.molecule().atomPos(c2, cx2, cy2);
        CHECK(cx2 == 30, "clamped: the rightmost atom lands exactly on the page edge");
        CHECK(cx1 == 20, "clamped: the selection keeps its shape (10 units apart)");
    }

    // rxnArrow / rxnPlus / multitailArrow all translate with the selection.
    {
        DocumentState d4;
        RxnArrowId ar = d4.molecule().addRxnArrow(0, 0, 5, 0);
        RxnPlusId pl = d4.molecule().addRxnPlus(1, 1);
        MultitailArrowId mta = d4.molecule().addMultitailArrow({0, 0, 2, 2});
        d4.selectRxnArrow(ar);
        d4.addRxnPlusToSelection(pl);
        d4.addMultitailArrowToSelection(mta);

        d4.moveSelectionLive(3, 4);
        double x1 = 0, y1 = 0, x2 = 0, y2 = 0;
        CHECK(d4.molecule().rxnArrowEndpoints(ar, x1, y1, x2, y2) && x1 == 3 && y1 == 4 && x2 == 8 && y2 == 4,
              "live move translates BOTH rxnArrow endpoints");
        double px = 0, py = 0;
        CHECK(d4.molecule().rxnPlusPos(pl, px, py) && px == 4 && py == 5, "live move translates the rxnPlus");
        QList<double> pts = d4.molecule().multitailArrowPoints(mta);
        CHECK(pts.size() == 4 && pts[0] == 3 && pts[1] == 4 && pts[2] == 5 && pts[3] == 6,
              "live move translates EVERY multitailArrow point");

        d4.commitMove();
        d4.undo();
        CHECK(d4.molecule().rxnPlusPos(pl, px, py) && px == 1 && py == 1, "undo restores the rxnPlus");
        pts = d4.molecule().multitailArrowPoints(mta);
        CHECK(pts.size() == 4 && pts[0] == 0 && pts[3] == 2, "undo restores the multitailArrow points");
    }
}

static void test_liveDragRotate() {
    std::printf("--- Test 9: live-drag rotate gesture ---\n");
    DocumentState doc;
    // Symmetric pair about the origin -> centroid (0,0), easy exact math.
    // Seeded via doc.molecule() so history starts empty.
    AtomId a1 = doc.molecule().addAtom(QStringLiteral("C"), 1, 0);
    AtomId a2 = doc.molecule().addAtom(QStringLiteral("C"), -1, 0);
    doc.selectAtom(a1);
    doc.addAtomToSelection(a2);

    const double kPi = 3.14159265358979323846;
    doc.rotateSelectionLive(kPi / 2);   // +90 degrees about (0,0): (1,0) -> (0,1)
    double x = 0, y = 0;
    CHECK(doc.molecule().atomPos(a1, x, y) && std::fabs(x - 0.0) < 1e-9 && std::fabs(y - 1.0) < 1e-9,
          "live rotate by +90 moves (1,0) to (0,1)");
    CHECK(!doc.canUndo(), "live rotate calls push NO history entry");

    // A second incremental call accumulates: another +90 => 180 total.
    doc.rotateSelectionLive(kPi / 2);
    CHECK(doc.molecule().atomPos(a1, x, y) && std::fabs(x + 1.0) < 1e-9 && std::fabs(y - 0.0) < 1e-9,
          "a second increment accumulates to 180 degrees total");

    doc.commitRotate();
    CHECK(doc.canUndo(), "commitRotate pushes exactly one history entry");
    doc.undo();
    CHECK(doc.molecule().atomPos(a1, x, y) && std::fabs(x - 1.0) < 1e-9 && std::fabs(y - 0.0) < 1e-9,
          "undo restores a1 to its exact original position");
    CHECK(doc.molecule().atomPos(a2, x, y) && std::fabs(x + 1.0) < 1e-9 && std::fabs(y - 0.0) < 1e-9,
          "undo restores a2 to its exact original position");
    doc.redo();
    CHECK(doc.molecule().atomPos(a1, x, y) && std::fabs(x + 1.0) < 1e-9, "redo re-applies the rotation");

    // Fewer than 2 snapshot points: no-op (real code's pts.length < 2 guard).
    {
        DocumentState d2;
        AtomId b1 = d2.molecule().addAtom(QStringLiteral("C"), 5, 5);
        d2.selectAtom(b1);
        d2.rotateSelectionLive(kPi / 2);
        double bx = 0, by = 0;
        CHECK(d2.molecule().atomPos(b1, bx, by) && bx == 5 && by == 5, "single point: rotate is a no-op");
        d2.commitRotate();
        CHECK(!d2.canUndo(), "commitRotate after a no-op gesture pushes nothing");
    }

    // Page-boundary clamp: a rotation that would push a point off-page freezes.
    {
        DocumentState d3;
        AtomId c1 = d3.molecule().addAtom(QStringLiteral("C"), 29, 0);
        AtomId c2 = d3.molecule().addAtom(QStringLiteral("C"), -29, 0);
        d3.selectAtom(c1);
        d3.addAtomToSelection(c2);
        // 90 degrees would put both at y = +-29, outside kPageMaxY (21).
        d3.rotateSelectionLive(kPi / 2);
        double cx = 0, cy = 0;
        CHECK(d3.molecule().atomPos(c1, cx, cy) && cx == 29 && cy == 0,
              "out-of-page rotation is refused: positions unchanged");
    }

    // Multitail arrows are excluded from rotate but do not block it.
    {
        DocumentState d4;
        AtomId e1 = d4.molecule().addAtom(QStringLiteral("C"), 1, 0);
        AtomId e2 = d4.molecule().addAtom(QStringLiteral("C"), -1, 0);
        MultitailArrowId mta = d4.molecule().addMultitailArrow({4, 4, 6, 6});
        d4.selectAtom(e1);
        d4.addAtomToSelection(e2);
        d4.addMultitailArrowToSelection(mta);
        d4.rotateSelectionLive(kPi / 2);
        double ex = 0, ey = 0;
        CHECK(d4.molecule().atomPos(e1, ex, ey) && std::fabs(ey - 1.0) < 1e-9,
              "atoms still rotate when a multitail arrow is also selected");
        QList<double> pts = d4.molecule().multitailArrowPoints(mta);
        CHECK(pts.size() == 4 && pts[0] == 4 && pts[3] == 6,
              "multitail arrow points are NOT rotated (documented limitation)");
    }
}

static void test_liveDragScale() {
    std::printf("--- Test 10: live-drag scale gesture ---\n");
    // Seeded via doc.molecule() so history starts empty.
    DocumentState doc;
    AtomId a1 = doc.molecule().addAtom(QStringLiteral("C"), 0, 0);
    AtomId a2 = doc.molecule().addAtom(QStringLiteral("C"), 2, 0);
    doc.selectAtom(a1);
    doc.addAtomToSelection(a2);

    doc.scaleSelectionLive(2.0, 0.0, 0.0);   // anchor at the origin
    double x = 0, y = 0;
    CHECK(doc.molecule().atomPos(a1, x, y) && x == 0 && y == 0, "the anchor point itself stays fixed");
    CHECK(doc.molecule().atomPos(a2, x, y) && x == 4 && y == 0, "scale 2x doubles the distance from the anchor");
    CHECK(!doc.canUndo(), "live scale calls push NO history entry");

    // factor is ABSOLUTE, not incremental: 3.0 means 3x from gesture start,
    // NOT 2x then 3x (= 6x). This is the real code's semantics.
    doc.scaleSelectionLive(3.0, 0.0, 0.0);
    CHECK(doc.molecule().atomPos(a2, x, y) && x == 6 && y == 0,
          "a second call with factor 3 gives 3x from ORIGINAL, not 6x");

    doc.commitScale();
    CHECK(doc.canUndo(), "commitScale pushes exactly one history entry");
    doc.undo();
    CHECK(doc.molecule().atomPos(a2, x, y) && x == 2 && y == 0, "undo restores the exact original position");
    doc.redo();
    CHECK(doc.molecule().atomPos(a2, x, y) && x == 6 && y == 0, "redo re-applies the scale");

    // Factor is floored at 0.05 to stop a handle dragged through its anchor.
    {
        DocumentState d2;
        AtomId b1 = d2.molecule().addAtom(QStringLiteral("C"), 0, 0);
        AtomId b2 = d2.molecule().addAtom(QStringLiteral("C"), 10, 0);
        d2.selectAtom(b1);
        d2.addAtomToSelection(b2);
        d2.scaleSelectionLive(-5.0, 0.0, 0.0);   // negative would invert/collapse
        double bx = 0, by = 0;
        CHECK(d2.molecule().atomPos(b2, bx, by) && std::fabs(bx - 0.5) < 1e-9,
              "negative factor is floored to 0.05 (10 * 0.05 = 0.5)");
    }

    // Fewer than 2 snapshot points: no-op.
    {
        DocumentState d3;
        AtomId c1 = d3.molecule().addAtom(QStringLiteral("C"), 5, 5);
        d3.selectAtom(c1);
        d3.scaleSelectionLive(2.0, 0.0, 0.0);
        double cx = 0, cy = 0;
        CHECK(d3.molecule().atomPos(c1, cx, cy) && cx == 5 && cy == 5, "single point: scale is a no-op");
        d3.commitScale();
        CHECK(!d3.canUndo(), "commitScale after a no-op gesture pushes nothing");
    }

    // Page-boundary clamp: a scale that would push a point off-page freezes.
    {
        DocumentState d4;
        AtomId e1 = d4.molecule().addAtom(QStringLiteral("C"), 0, 0);
        AtomId e2 = d4.molecule().addAtom(QStringLiteral("C"), 20, 0);
        d4.selectAtom(e1);
        d4.addAtomToSelection(e2);
        d4.scaleSelectionLive(10.0, 0.0, 0.0);   // would put e2 at x=200, past kPageMaxX
        double ex = 0, ey = 0;
        CHECK(d4.molecule().atomPos(e2, ex, ey) && ex == 20 && ey == 0,
              "out-of-page scale is refused: positions unchanged");
    }
}

static void test_dragStateResetOnUndoRedo() {
    std::printf("--- Test 11: undo/redo reset ALL drag state (deliberate deviation) ---\n");
    const double kPi = 3.14159265358979323846;

    // Each case seeds atoms via doc.molecule() (no history), then creates
    // EXACTLY ONE real history entry with changeAtomCharge. After the undo,
    // history is empty, so `!canUndo()` after the commit is an exact signal:
    // if the drag state had survived, the commit WOULD push an entry and
    // canUndo() would flip to true. An equality-against-a-captured-bool check
    // cannot detect that, because both states are "true".

    // Move: an uncommitted gesture must not survive an undo.
    {
        DocumentState doc;
        AtomId a1 = doc.molecule().addAtom(QStringLiteral("C"), 0, 0);
        AtomId a2 = doc.molecule().addAtom(QStringLiteral("C"), 2, 0);
        doc.selectAtom(a1);
        doc.addAtomToSelection(a2);
        doc.changeAtomCharge(a1, 1);           // the one and only history entry
        doc.moveSelectionLive(1, 0);           // gesture in flight, uncommitted
        doc.undo();                            // undoes the charge AND must clear drag state
        CHECK(!doc.canUndo(), "history is empty after undoing the only entry");
        doc.commitMove();
        CHECK(!doc.canUndo(),
              "commitMove after undo pushes nothing -- move drag state was cleared");
    }

    // Rotate: the snapshot must be discarded too (the real JS never clears it).
    {
        DocumentState doc;
        AtomId a1 = doc.molecule().addAtom(QStringLiteral("C"), 1, 0);
        AtomId a2 = doc.molecule().addAtom(QStringLiteral("C"), -1, 0);
        doc.selectAtom(a1);
        doc.addAtomToSelection(a2);
        doc.changeAtomCharge(a1, 1);
        doc.rotateSelectionLive(kPi / 2);
        doc.undo();
        CHECK(!doc.canUndo(), "history is empty after undoing the only entry");
        doc.commitRotate();
        CHECK(!doc.canUndo(),
              "commitRotate after undo pushes nothing -- rotate drag state was cleared");
    }

    // Scale, via redo() rather than undo(). Here history is NOT empty after the
    // redo, so canUndo() cannot distinguish the two outcomes -- assert on WHAT
    // the next undo consumes instead.
    {
        DocumentState doc;
        AtomId a1 = doc.molecule().addAtom(QStringLiteral("C"), 0, 0);
        AtomId a2 = doc.molecule().addAtom(QStringLiteral("C"), 2, 0);
        doc.selectAtom(a1);
        doc.addAtomToSelection(a2);
        doc.changeAtomCharge(a1, 1);
        doc.undo();                            // give redo something to do
        doc.selectAtom(a1);                    // undo cleared the selection
        doc.addAtomToSelection(a2);
        doc.scaleSelectionLive(2.0, 0.0, 0.0); // gesture in flight
        doc.redo();                            // re-applies the charge; must clear drag state
        CHECK(doc.molecule().atomCharge(a1) == 1, "redo re-applied the charge change");
        doc.commitScale();
        doc.undo();
        // If the scale state had survived, commitScale would have pushed a
        // command and this undo would have consumed IT, leaving charge at 1.
        CHECK(doc.molecule().atomCharge(a1) == 0,
              "undo consumed the charge entry, not a stale scale command -- scale drag state was cleared");
    }
}

static void test_addRing() {
    std::printf("--- Test 12: addRing (fusion-aware alternation) ---\n");
    const double kPi = 3.14159265358979323846;

    // Freestanding aromatic 6-ring: alternates 2/1/2/1/2/1.
    {
        DocumentState doc;
        QList<double> coords;
        for (int i = 0; i < 6; ++i) {
            double angle = (kPi / 3.0) * i;
            coords.append(std::cos(angle));
            coords.append(std::sin(angle));
        }
        CHECK(!doc.canUndo(), "history starts empty");
        doc.addRing(coords, true);
        CHECK(doc.canUndo(), "addRing pushes exactly one history entry");
        CHECK(doc.molecule().atomCount() == 6, "6 atoms created");
        CHECK(doc.molecule().bondCount() == 6, "6 bonds created");
        QList<AtomId> ids = doc.molecule().atomIds();
        for (int i = 0; i < 6; ++i) {
            BondId bid = doc.molecule().findBond(ids[i], ids[(i + 1) % 6]);
            CHECK(bid >= 0, "consecutive ring atoms are bonded");
        }
        doc.undo();
        CHECK(doc.molecule().atomCount() == 0, "undo removes every ring atom");
        CHECK(doc.molecule().bondCount() == 0, "undo removes every ring bond");
        CHECK(!doc.canUndo(), "history is empty again");
    }

    // Freestanding NonAromatic 6-ring stays all single.
    {
        DocumentState doc;
        QList<double> coords;
        for (int i = 0; i < 6; ++i) {
            double angle = (kPi / 3.0) * i;
            coords.append(std::cos(angle));
            coords.append(std::sin(angle));
        }
        doc.addRing(coords, false);
        QList<AtomId> ids = doc.molecule().atomIds();
        for (int i = 0; i < 6; ++i) {
            BondId bid = doc.molecule().findBond(ids[i], ids[(i + 1) % 6]);
            CHECK(doc.molecule().bondOrder(bid) == 1, "every bond stays single when aromatic=false");
        }
    }

    // A freestanding 5-ring stays all single even with aromatic=true (the
    // real code's `if (n !== 6) return` branch).
    {
        DocumentState doc;
        QList<double> coords;
        for (int i = 0; i < 5; ++i) {
            double angle = (2.0 * kPi / 5.0) * i;
            coords.append(std::cos(angle));
            coords.append(std::sin(angle));
        }
        doc.addRing(coords, true);
        QList<AtomId> ids = doc.molecule().atomIds();
        for (int i = 0; i < 5; ++i) {
            BondId bid = doc.molecule().findBond(ids[i], ids[(i + 1) % 5]);
            CHECK(doc.molecule().bondOrder(bid) == 1, "5-ring stays all single (n != 6 branch)");
        }
    }

    // FUSION SEAM REGRESSION (the exact naphthalene bug the real comment
    // describes): a benzene ring, then a second aromatic 6-ring sharing one
    // edge with it. The seam bond must keep its committed order, and the
    // new ring must alternate outward WITHOUT two adjacent double bonds at
    // the seam.
    {
        DocumentState doc;
        AtomId a0 = doc.molecule().addAtom(QStringLiteral("C"), 0.0, 0.0);
        AtomId a1 = doc.molecule().addAtom(QStringLiteral("C"), 1.5, 0.0);
        BondId seam = doc.molecule().addBond(a0, a1, 2);   // pre-existing, committed order 2
        CHECK(seam > 0, "setup: seam bond created");

        // Second ring shares the (a0,a1) edge: reuse their exact coordinates
        // for the first two ring vertices so mergeOverlappingAtoms fuses onto
        // them, then continue the hexagon outward.
        QList<double> coords;
        coords << 0.0 << 0.0 << 1.5 << 0.0;             // coincide with a0, a1
        coords << 2.25 << 1.3 << 1.5 << 2.6 << 0.0 << 2.6 << -0.75 << 1.3;
        doc.addRing(coords, true);

        // 2 pre-existing (a0,a1) + 6 new ring atoms - 2 fused away (the two
        // new atoms coinciding with a0/a1) = 6 total.
        CHECK(doc.molecule().atomCount() == 6, "2 pre-existing + 6 new - 2 fused onto a0/a1 = 6 total");
        // seam(a0-a1) + 2 replacement bonds (a0-to-new-neighbor,
        // a1-to-new-neighbor) + 3 untouched inner ring bonds = 6 total.
        CHECK(doc.molecule().bondCount() == 6, "6 bonds total in the fused bicyclic system");
        CHECK(doc.molecule().bondOrder(seam) == 2, "the seam bond's committed order is UNCHANGED");

        // Walk the new ring's atoms (everything except a0/a1) and confirm no
        // two bonds adjacent to the seam are both double (the naphthalene bug).
        QList<AtomId> allIds = doc.molecule().atomIds();
        QList<AtomId> newRingAtoms;
        for (AtomId id : allIds) if (id != a0 && id != a1) newRingAtoms.append(id);
        // Only 4, not 6: two of the new ring's 6 vertices coincided with
        // a0/a1 and were fused away, never surviving to this point.
        CHECK(newRingAtoms.size() == 4, "4 surviving non-seam atoms belong to the new ring");
        // The two edges leaving the seam (a0-to-new-neighbor and
        // a1-to-new-neighbor) must both be SINGLE -- alternating outward from
        // an already-double seam.
        int singleCount = 0, doubleCount = 0;
        for (AtomId id : newRingAtoms) {
            BondId toA0 = doc.molecule().findBond(a0, id);
            BondId toA1 = doc.molecule().findBond(a1, id);
            if (toA0 >= 0) { if (doc.molecule().bondOrder(toA0) == 1) ++singleCount; else ++doubleCount; }
            if (toA1 >= 0) { if (doc.molecule().bondOrder(toA1) == 1) ++singleCount; else ++doubleCount; }
        }
        CHECK(singleCount == 2 && doubleCount == 0,
              "both edges leaving the seam are single -- no adjacent double bonds at the fusion point");

        doc.undo();
        CHECK(doc.molecule().atomCount() == 2, "undo removes the 4 surviving new-ring atoms, leaving a0/a1");
        CHECK(doc.molecule().bondCount() == 1, "undo removes exactly the new-ring bonds, leaving the seam");
        CHECK(doc.molecule().bondOrder(seam) == 2, "the seam bond survives undo with its original order");
    }

    // Malformed coords no-op cleanly. Each case isolates ONE guard condition
    // so a bug in either check can't hide behind the other also tripping.
    {
        DocumentState doc;
        // Odd length but >= 6 doubles (7 total: 3.5 "points") -- exercises
        // the `% 2 != 0` check specifically, independent of the size check.
        doc.addRing({0.0, 0.0, 1.0, 0.0, 2.0, 0.0, 3.0});
        CHECK(!doc.canUndo(), "odd-length coords (>=6 doubles): no history entry");
        // Even length but only 2 points (4 doubles) -- exercises the `< 6`
        // check specifically, independent of the oddness check.
        doc.addRing({0.0, 0.0, 1.0, 0.0});
        CHECK(!doc.canUndo(), "fewer than 3 points (even length): no history entry");
    }
}

static void test_addChain() {
    std::printf("--- Test 13: addChain ---\n");

    // Horizontal chain, dist=6, bondLen=1.5 -> nBonds = round(6/1.5) = 4 ->
    // 5 atoms, 4 bonds. theta=0, half=PI/6.
    {
        DocumentState doc;
        CHECK(!doc.canUndo(), "history starts empty");
        doc.addChain(0.0, 0.0, 6.0, 0.0);
        CHECK(doc.canUndo(), "addChain pushes exactly one history entry");
        CHECK(doc.molecule().atomCount() == 5, "5 atoms for a 4-bond chain");
        CHECK(doc.molecule().bondCount() == 4, "4 bonds in the chain");

        // Hand-computed expected vertices (theta=0, half=PI/6, bondLen=1.5),
        // matching ChainPlacementEngine::compute's exact formula.
        const double kPi = 3.14159265358979323846;
        const double half = kPi / 6.0;
        const double bondLen = 1.5;
        double px = 0.0, py = 0.0;
        QList<AtomId> ids = doc.molecule().atomIds();
        CHECK(ids.size() == 5, "setup: 5 atom ids to check");
        for (int i = 0; i <= 4; ++i) {
            double x = 0, y = 0;
            CHECK(doc.molecule().atomPos(ids[i], x, y), "atom position readable");
            // 1e-5, not 1e-9: EditableMolecule::addAtom stores coordinates via
            // indigoSetXYZ, which takes float (32-bit), not double -- exact
            // round numbers (0.0, 1.0, -0.5, etc.) survive that round-trip
            // losslessly, but these expected values are irrational trig
            // results, which lose precision below float32's ~7 significant
            // digits. 1e-5 is comfortably above that rounding error for
            // values in this test's range (0 to ~5.2).
            CHECK(std::fabs(x - px) < 1e-5 && std::fabs(y - py) < 1e-5, "vertex position matches the formula");
            double ang = 0.0 + ((i % 2 == 0) ? half : -half);
            px += bondLen * std::cos(ang);
            py += bondLen * std::sin(ang);
        }

        doc.undo();
        CHECK(doc.molecule().atomCount() == 0, "undo removes every chain atom");
        CHECK(doc.molecule().bondCount() == 0, "undo removes every chain bond");
    }

    // A zero-length drag still lays at least one bond (max(1, ...)).
    {
        DocumentState doc;
        doc.addChain(2.0, 2.0, 2.0, 2.0);
        CHECK(doc.molecule().atomCount() == 2, "zero-length drag still produces a 2-atom chain");
        CHECK(doc.molecule().bondCount() == 1, "zero-length drag still produces 1 bond");
    }

    // Chain endpoint placed exactly on an existing atom grafts onto it
    // (total atom count reflects the merge, not a duplicate).
    {
        DocumentState doc;
        AtomId existing = doc.molecule().addAtom(QStringLiteral("C"), 0.0, 0.0);
        doc.addChain(0.0, 0.0, 3.0, 0.0);   // dist=3 -> nBonds=2 -> 3 atoms, first one coincides
        // 1 pre-existing + 3 new - 1 fused = 3 total.
        CHECK(doc.molecule().atomCount() == 3, "chain endpoint fuses onto the existing atom");
        CHECK(doc.molecule().atomSymbol(existing) == QStringLiteral("C"), "the existing atom survives");
        int chainBondsFromExisting = 0;
        for (BondId bid : doc.molecule().bondIds()) {
            AtomId a = -1, b = -1;
            doc.molecule().bondEndpoints(bid, a, b);
            if (a == existing || b == existing) ++chainBondsFromExisting;
        }
        CHECK(chainBondsFromExisting == 1, "the existing atom gained exactly one chain bond");
    }
}

static void test_insertFunctionalGroup() {
    std::printf("--- Test 14: insertFunctionalGroup ---\n");
    TemplateLibrary lib(
        QStringLiteral(SKETCH_SOURCE_DIR "/templates/fg.sdf"),
        QStringLiteral(SKETCH_SOURCE_DIR "/templates/library.sdf"),
        QStringLiteral(SKETCH_SOURCE_DIR "/templates/salts-and-solvents.sdf"));

    // Plain paste: no target atom, centers on (cx, cy).
    {
        DocumentState doc;
        CHECK(!doc.canUndo(), "history starts empty");
        doc.insertFunctionalGroup(lib, QStringLiteral("Ac"), 10.0, 10.0);
        CHECK(doc.canUndo(), "insertFunctionalGroup pushes exactly one history entry");
        CHECK(doc.molecule().atomCount() == 3, "\"Ac\" plain paste creates 3 atoms");
        CHECK(doc.molecule().bondCount() == 2, "\"Ac\" plain paste creates 2 bonds");
        doc.undo();
        CHECK(doc.molecule().atomCount() == 0, "undo removes every pasted atom");
        CHECK(doc.molecule().bondCount() == 0, "undo removes every pasted bond");
    }

    // Graft onto an existing atom: "Ac"'s attach atom (SUP attachment point
    // at atom index 0, confirmed by direct file read) fuses onto the target,
    // which survives with the template's remaining bonds attached.
    {
        DocumentState doc;
        AtomId target = doc.molecule().addAtom(QStringLiteral("C"), 0.0, 0.0);
        doc.insertFunctionalGroup(lib, QStringLiteral("Ac"), 0.0, 0.0, target, true);

        // "Ac" has 3 atoms; grafting fuses its attach atom onto the
        // pre-existing target, so 1 pre-existing + 3 new - 1 fused = 3 total.
        CHECK(doc.molecule().atomCount() == 3, "graft: 1 pre-existing + 3 new - 1 fused = 3 total");
        CHECK(doc.molecule().atomSymbol(target) == QStringLiteral("C"), "target atom survives");
        int bondsFromTarget = 0;
        for (BondId bid : doc.molecule().bondIds()) {
            AtomId a = -1, b = -1;
            doc.molecule().bondEndpoints(bid, a, b);
            if (a == target || b == target) ++bondsFromTarget;
        }
        // "Ac"'s attach atom (atom index 0, the carbonyl carbon) is bonded
        // to BOTH other atoms within the fragment (bond block: "2 1 1" and
        // "1 3 2", i.e. atom 1 = attach atom is bonded to atom 2 [CH3] and
        // atom 3 [O]) -- confirmed by direct read of the real fg.sdf record,
        // not assumed. graftAtomOnto rewires ALL of the attach atom's
        // incident bonds onto the target, so the target gains 2 bonds, not 1.
        CHECK(bondsFromTarget == 2, "target atom gained both of the grafted attach atom's bonds");
        doc.undo();
        CHECK(doc.molecule().atomCount() == 1, "undo restores to just the pre-existing target atom");
    }

    // Missing template: placeholder-atom fallback.
    {
        DocumentState doc;
        doc.insertFunctionalGroup(lib, QStringLiteral("NotARealTemplateName"), 2.0, 3.0);
        CHECK(doc.molecule().atomCount() == 1, "missing template falls back to one placeholder atom");
        QList<AtomId> ids = doc.molecule().atomIds();
        CHECK(doc.molecule().atomSymbol(ids[0]) == QStringLiteral("NotARealTemplateName"),
              "placeholder atom is labeled with the requested (missing) template name");
        doc.undo();
        CHECK(doc.molecule().atomCount() == 0, "undo removes the placeholder atom");
    }
}


static void test_documentStateSelection() {
    std::printf("--- Test 4: DocumentState selection ---\n");
    DocumentState doc;
    EditableMolecule& mol = doc.molecule();
    AtomId a1 = mol.addAtom(QStringLiteral("C"), 0, 0);
    AtomId a2 = mol.addAtom(QStringLiteral("O"), 1, 0);
    BondId b1 = mol.addBond(a1, a2, 1);
    int ra1 = mol.addRxnArrow(0, 0, 5, 0);
    int rp1 = mol.addRxnPlus(2, 0);
    int mta1 = mol.addMultitailArrow({5,5, 0,4, 0,6});

    CHECK(doc.selection().isEmpty(), "fresh DocumentState has empty selection");

    // selectX REPLACES the whole selection (matches every real _selection
    // literal in 10-state.js -- each clears the other 4 sets).
    doc.selectAtom(a1);
    CHECK(doc.selection().atoms.contains(a1) && doc.selection().atoms.size() == 1,
          "selectAtom selects exactly that atom");
    CHECK(doc.selection().bonds.isEmpty(), "selectAtom clears bonds");

    doc.selectBond(b1);
    CHECK(doc.selection().bonds.contains(b1), "selectBond selects that bond");
    CHECK(doc.selection().atoms.isEmpty(), "selectBond clears atoms (replace, not add)");

    doc.selectRxnArrow(ra1);
    CHECK(doc.selection().rxnArrows.contains(ra1), "selectRxnArrow works");
    doc.selectRxnPlus(rp1);
    CHECK(doc.selection().rxnPluses.contains(rp1), "selectRxnPlus works");
    doc.selectMultitailArrow(mta1);
    CHECK(doc.selection().multitailArrows.contains(mta1), "selectMultitailArrow works");

    doc.clearSelection();
    CHECK(doc.selection().isEmpty(), "clearSelection empties everything");

    // add*ToSelection: additive, does not clear other types.
    doc.addAtomToSelection(a1);
    doc.addAtomToSelection(a2);
    doc.addBondToSelection(b1);
    CHECK(doc.selection().atoms.size() == 2, "addAtomToSelection is additive");
    CHECK(doc.selection().bonds.contains(b1), "addBondToSelection didn't clear atoms");

    doc.removeAtomFromSelection(a1);
    CHECK(!doc.selection().atoms.contains(a1) && doc.selection().atoms.contains(a2),
          "removeAtomFromSelection removes only the specified atom");
    CHECK(doc.selection().bonds.contains(b1), "removeAtomFromSelection didn't touch bonds");

    doc.clearSelection();
    doc.selectAll();
    CHECK(doc.selection().atoms.size() == 2, "selectAll selects both atoms");
    CHECK(doc.selection().bonds.size() == 1, "selectAll selects the bond");
    CHECK(doc.selection().rxnArrows.size() == 1, "selectAll selects the rxn arrow");
    CHECK(doc.selection().rxnPluses.size() == 1, "selectAll selects the rxn plus");
    CHECK(doc.selection().multitailArrows.size() == 1, "selectAll selects the multitail arrow");

    // Spec requirement: selection clears after both undo() and redo() (the
    // real 10-state.js undo()/redo() both reset _selection to all-empty).
    // undo()/redo() were implemented in Task 5 before selection() existed to
    // assert against -- verified here, now that it does.
    EditCommand addSulfur;
    addSulfur.execute = [&mol]() { mol.addAtom(QStringLiteral("S"), 3, 0); };
    addSulfur.invert = [&mol]() {
        QList<AtomId> ids = mol.atomIds();
        if (!ids.isEmpty()) mol.removeAtom(ids.last());
    };
    doc.executeCommand(addSulfur);
    doc.selectAll();
    CHECK(!doc.selection().isEmpty(), "selection is non-empty right before undo");
    doc.undo();
    CHECK(doc.selection().isEmpty(), "undo() clears the selection");

    doc.selectAll();
    CHECK(!doc.selection().isEmpty(), "selection is non-empty right before redo");
    doc.redo();
    CHECK(doc.selection().isEmpty(), "redo() clears the selection");
}

static void test_insertLibraryTemplateFused() {
    std::printf("--- Test 15: insertLibraryTemplateFused ---\n");
    TemplateLibrary lib(
        QStringLiteral(SKETCH_SOURCE_DIR "/templates/fg.sdf"),
        QStringLiteral(SKETCH_SOURCE_DIR "/templates/library.sdf"),
        QStringLiteral(SKETCH_SOURCE_DIR "/templates/salts-and-solvents.sdf"));

    // Real fusion success: "Indole" (9 atoms, 10 bonds, bondIdx=6) fused onto
    // a document bond. The seam bond's committed order must survive.
    {
        DocumentState doc;
        AtomId a0 = doc.molecule().addAtom(QStringLiteral("C"), 0.0, 0.0);
        AtomId a1 = doc.molecule().addAtom(QStringLiteral("C"), 1.5, 0.0);
        BondId seam = doc.molecule().addBond(a0, a1, 2);
        CHECK(!doc.canUndo(), "history starts empty (seeded via the molecule directly)");

        doc.insertLibraryTemplateFused(lib, QStringLiteral("Indole"), 0.75, 3.0, seam);
        CHECK(doc.canUndo(), "insertLibraryTemplateFused pushes exactly one history entry");
        CHECK(doc.molecule().bondOrder(seam) == 2, "the seam bond's committed order is UNCHANGED");
        // "Indole" has 9 atoms; a successful bond-fusion merges exactly 2 of
        // them onto the pre-existing a0/a1 (the fusion bond's own two atoms),
        // so 2 pre-existing + 9 new - 2 fused = 9 total.
        CHECK(doc.molecule().atomCount() == 9, "2 pre-existing + 9 new - 2 fused onto the seam = 9 total");
        doc.undo();
        CHECK(doc.molecule().atomCount() == 2, "undo restores to just a0/a1");
        CHECK(doc.molecule().bondCount() == 1, "undo restores to just the seam bond");
        CHECK(doc.molecule().bondOrder(seam) == 2, "the seam bond survives undo with its original order");
    }

    // Fallback: template with no fusion metadata (any fg.sdf entry).
    {
        DocumentState doc;
        AtomId a0 = doc.molecule().addAtom(QStringLiteral("C"), 0.0, 0.0);
        AtomId a1 = doc.molecule().addAtom(QStringLiteral("C"), 1.5, 0.0);
        BondId b = doc.molecule().addBond(a0, a1, 1);
        doc.insertLibraryTemplateFused(lib, QStringLiteral("Ac"), 0.0, 0.0, b);
        // Falls back to plain insertFunctionalGroup(lib, "Ac", cx, cy) --
        // "Ac" has 3 atoms, no graft (fallback never passes a target atom).
        CHECK(doc.molecule().atomCount() == 2 + 3, "no-fusion-metadata fallback pastes \"Ac\" plainly");
    }

    // Fallback: targetBondId that doesn't resolve.
    {
        DocumentState doc;
        doc.insertLibraryTemplateFused(lib, QStringLiteral("Indole"), 0.0, 0.0, 9999);
        CHECK(doc.molecule().atomCount() == 9, "missing target bond: falls back to plain \"Indole\" paste");
    }
}

static void test_toggleSgroupExpanded() {
    std::printf("--- Test 16: toggleSgroupExpanded ---\n");
    TemplateLibrary lib(
        QStringLiteral(SKETCH_SOURCE_DIR "/templates/fg.sdf"),
        QStringLiteral(SKETCH_SOURCE_DIR "/templates/library.sdf"),
        QStringLiteral(SKETCH_SOURCE_DIR "/templates/salts-and-solvents.sdf"));

    // Seeded via doc.molecule() directly (not doc.insertFunctionalGroup,
    // which is itself undoable) so history starts empty and canUndo() is an
    // exact, unambiguous signal for the checks below -- the same seeding
    // discipline used throughout this migration whenever a test needs to
    // prove "this call pushed exactly one/zero history entries."
    DocumentState doc;
    QString acMolfile = lib.molfileText(lib.functionalGroup(QStringLiteral("Ac")));
    EditableMolecule::InsertResult r = doc.molecule().insertStructure(
        acMolfile, [](double x, double y) { return QPointF(x, y); });
    CHECK(r.createdSGroups.size() == 1, "setup: \"Ac\" produces exactly one sgroup");
    SGroupId sgId = r.createdSGroups[0];
    CHECK(doc.molecule().sgroupExpanded(sgId), "newly-inserted sgroup starts expanded=true");
    CHECK(!doc.canUndo(), "history is empty (sgroup seeded via the molecule directly)");

    doc.toggleSgroupExpanded(sgId);
    CHECK(doc.canUndo(), "toggleSgroupExpanded pushes exactly one history entry");
    CHECK(!doc.molecule().sgroupExpanded(sgId), "toggle flips expanded true -> false");
    doc.undo();
    CHECK(doc.molecule().sgroupExpanded(sgId), "undo restores expanded=true");
    CHECK(!doc.canUndo(), "history is empty again after undoing the only entry");

    doc.toggleSgroupExpanded(9999);
    CHECK(!doc.canUndo(), "invalid SGroupId pushes no history entry");
    CHECK(doc.molecule().sgroupExpanded(sgId), "invalid-id toggle call left the real sgroup untouched");
}

static void test_rxnArrowLifecycle() {
    std::printf("--- Test 17: rxn-arrow lifecycle ---\n");
    DocumentState doc;

    RxnArrowId a1 = doc.addRxnArrow(0, 0);
    double x1 = 0, y1 = 0, x2 = 0, y2 = 0;
    CHECK(doc.molecule().rxnArrowEndpoints(a1, x1, y1, x2, y2), "addRxnArrow creates a real arrow");
    CHECK(x1 == 0 && y1 == 0 && x2 == 1.5 * 2.5 && y2 == 0, "default second endpoint is 2.5 bond-lengths to the right");
    CHECK(doc.molecule().rxnArrowMode(a1) == QStringLiteral("filled-triangle"), "default mode is filled-triangle");
    doc.undo();
    CHECK(doc.molecule().rxnArrowIds().isEmpty(), "undo removes the created arrow");

    // Recreate a1 for real -- the throwaway above only proved undo works; everything below
    // needs a persistent first arrow to compare counts against.
    a1 = doc.addRxnArrow(0, 0);

    RxnArrowId a2 = doc.addRxnArrow(1, 1, QStringLiteral("resonance"));
    CHECK(doc.molecule().rxnArrowMode(a2) == QStringLiteral("resonance"), "explicit mode is applied at creation");

    RxnArrowId curved = doc.addCurvedArrow(0, 0, 1, 2, 3, 0);
    CHECK(doc.molecule().rxnArrowMode(curved) == QStringLiteral("curved-mechanism"), "addCurvedArrow sets curved-mechanism mode");
    double cx = 0, cy = 0;
    CHECK(doc.molecule().rxnArrowCurvature(curved, cx, cy) && cx == 1 && cy == 2, "addCurvedArrow stores the control point");
    doc.undo();
    CHECK(!doc.molecule().rxnArrowIds().contains(curved), "undo removes the curved arrow too");

    doc.setRxnArrowMode(a2, QStringLiteral("hollow-triangle"));
    CHECK(doc.molecule().rxnArrowMode(a2) == QStringLiteral("hollow-triangle"), "setRxnArrowMode changes the mode");
    bool canUndoBefore = doc.canUndo();
    doc.setRxnArrowMode(a2, QStringLiteral("hollow-triangle")); // unchanged -- must no-op
    CHECK(doc.canUndo() == canUndoBefore, "setRxnArrowMode to the SAME mode pushes no history entry");
    doc.undo();
    CHECK(doc.molecule().rxnArrowMode(a2) == QStringLiteral("resonance"), "undo restores the original mode");

    doc.setRxnArrowConditions(a2, QStringLiteral("Pd/C"), QStringLiteral("H2"));
    CHECK(doc.molecule().rxnArrowConditionsAbove(a2) == QStringLiteral("Pd/C"), "conditions above set");
    CHECK(doc.molecule().rxnArrowConditionsBelow(a2) == QStringLiteral("H2"), "conditions below set");
    doc.undo();
    CHECK(doc.molecule().rxnArrowConditionsAbove(a2).isEmpty(), "undo restores conditions to empty");

    doc.setRxnArrowConditions(a2, QStringLiteral("cond"), QStringLiteral("cond2"));
    doc.molecule().setRxnArrowCurvature(a2, 9, 9, true); // no DocumentState-level command for this exists (matches the real JS: curvature is only ever set via addCurvedArrow) -- mutate the molecule directly so the deleteRxnArrow capture-before-delete below has real curvature to preserve
    doc.deleteRxnArrow(a2);
    CHECK(doc.molecule().rxnArrowIds().size() == 1, "deleteRxnArrow removes the arrow (only a1 remains)");
    doc.undo();
    CHECK(doc.molecule().rxnArrowIds().size() == 2, "undo recreates the deleted arrow (fresh id)");
    bool foundRestored = false;
    for (RxnArrowId id : doc.molecule().rxnArrowIds()) {
        if (id != a1 && doc.molecule().rxnArrowConditionsAbove(id) == QStringLiteral("cond")) foundRestored = true;
    }
    CHECK(foundRestored, "restored arrow keeps its mode/conditions/curvature from before deletion");

    doc.deleteRxnArrow(9999);
    CHECK(!doc.canUndo() || doc.molecule().rxnArrowIds().size() == 2, "deleteRxnArrow on an unknown id is a no-op");
}

static void test_rxnPlusCommands() {
    std::printf("--- Test 23: rxn-plus add/delete ---\n");
    DocumentState doc;

    RxnPlusId p1 = doc.addRxnPlus(2, 3);
    double x = 0, y = 0;
    CHECK(doc.molecule().rxnPlusPos(p1, x, y) && x == 2 && y == 3, "addRxnPlus creates a real rxn-plus at the given position");
    doc.undo();
    CHECK(doc.molecule().rxnPlusIds().isEmpty(), "undo removes the created rxn-plus");

    RxnPlusId p2 = doc.addRxnPlus(5, 5);
    doc.deleteRxnPlus(p2);
    CHECK(doc.molecule().rxnPlusIds().isEmpty(), "deleteRxnPlus removes it");
    doc.undo();
    CHECK(doc.molecule().rxnPlusIds().size() == 1, "undo recreates it (fresh id)");
    RxnPlusId restored = doc.molecule().rxnPlusIds().first();
    CHECK(doc.molecule().rxnPlusPos(restored, x, y) && x == 5 && y == 5, "restored rxn-plus keeps its original position");

    doc.deleteRxnPlus(9999);
    CHECK(!doc.canUndo() || doc.molecule().rxnPlusIds().size() == 1, "deleteRxnPlus on an unknown id is a no-op");
}

static void test_multitailArrowCommands() {
    std::printf("--- Test 24: multitail arrow add/delete/add-tail ---\n");
    DocumentState doc;

    MultitailArrowId id = doc.addMultitailArrow(0, 0);
    QList<double> pts = doc.molecule().multitailArrowPoints(id);
    CHECK(pts.size() == 2, "addMultitailArrow starts with head only, zero tails -- matches the real tailsYOffset=[]");
    double headX = pts[0], headY = pts[1];
    CHECK(headX == 2.0 * 1.5 && headY == 0.0, "head is offset (2*bondLength, 0) from the creation point");
    doc.undo();
    CHECK(doc.molecule().multitailArrowIds().isEmpty(), "undo removes the created arrow");

    MultitailArrowId id2 = doc.addMultitailArrow(0, 0);
    doc.addMultitailArrowTail(id2);
    QList<double> pts1 = doc.molecule().multitailArrowPoints(id2);
    CHECK(pts1.size() == 4, "one addMultitailArrowTail call adds exactly one tail (2 more numbers)");
    double firstTailY = pts1[3];
    CHECK(firstTailY == (0.0 + 3.0 * 1.5) / 2.0, "first tail is centered at the midpoint of the full spine (widest gap)");
    doc.undo();
    CHECK(doc.molecule().multitailArrowPoints(id2).size() == 2, "undo removes the added tail");

    doc.addMultitailArrowTail(id2);
    doc.addMultitailArrowTail(id2);
    QList<double> pts2 = doc.molecule().multitailArrowPoints(id2);
    CHECK(pts2.size() == 6, "two addMultitailArrowTail calls produce two tails");
    CHECK(pts2[3] != pts2[5], "the two tails land at different Y positions (widest-gap search, not a fixed spot)");
    doc.undo();
    CHECK(doc.molecule().multitailArrowPoints(id2).size() == 4, "undo removes only the SECOND tail");

    doc.addMultitailArrowTail(9999);
    CHECK(!doc.canUndo() || doc.molecule().multitailArrowPoints(id2).size() == 4,
          "addMultitailArrowTail on an unknown id is a no-op");

    doc.deleteMultitailArrow(id2);
    CHECK(doc.molecule().multitailArrowIds().isEmpty(), "deleteMultitailArrow removes it");
    doc.undo();
    CHECK(doc.molecule().multitailArrowIds().size() == 1, "undo recreates it (fresh id) with the same points");
    MultitailArrowId restored = doc.molecule().multitailArrowIds().first();
    CHECK(doc.molecule().multitailArrowPoints(restored).size() == 4, "restored arrow keeps its one tail");

    doc.deleteMultitailArrow(9999);
    CHECK(!doc.canUndo() || doc.molecule().multitailArrowIds().size() == 1, "deleteMultitailArrow on an unknown id is a no-op");
}

static void test_setStereoFlags() {
    std::printf("--- Test 18: setStereoFlags ---\n");
    DocumentState doc;
    CHECK(doc.molecule().stereoFlagsType() == QStringLiteral("abs"), "default type is abs");
    CHECK(doc.molecule().stereoFlagsGroupId() == 0, "default groupId is 0");

    doc.setStereoFlags(QStringLiteral("rel"), 2);
    CHECK(doc.molecule().stereoFlagsType() == QStringLiteral("rel"), "setStereoFlags sets type");
    CHECK(doc.molecule().stereoFlagsGroupId() == 2, "setStereoFlags sets groupId");
    bool canUndoBefore = doc.canUndo();
    doc.setStereoFlags(QStringLiteral("rel"), 2); // unchanged -- must no-op
    CHECK(doc.canUndo() == canUndoBefore, "unchanged setStereoFlags pushes no history entry");
    doc.undo();
    CHECK(doc.molecule().stereoFlagsType() == QStringLiteral("abs"), "undo restores type to abs");
    CHECK(doc.molecule().stereoFlagsGroupId() == 0, "undo restores groupId to 0");

    // Falsy args default like the real JS (type || 'abs', groupId || 0).
    doc.setStereoFlags(QString(), 0);
    canUndoBefore = doc.canUndo();
    doc.setStereoFlags(QString(), 0);
    CHECK(doc.canUndo() == canUndoBefore, "empty-string type still normalizes to the current abs/0 state -- no-op");
}

static void test_rgroupCommands() {
    std::printf("--- Test 19: R-group commands ---\n");
    DocumentState doc;
    AtomId a1 = doc.addAtom(QStringLiteral("C"), 0, 0);
    AtomId a2 = doc.addAtom(QStringLiteral("C"), 5, 5);   // separate fragment (no bond to a1)

    CHECK(doc.addRGroup(1), "addRGroup(1) succeeds");
    CHECK(!doc.addRGroup(1), "addRGroup(1) again fails");
    doc.undo();
    CHECK(doc.molecule().rgroupNumbers().isEmpty(), "undo removes the R-group");
    doc.redo();
    CHECK(doc.molecule().rgroupNumbers() == QList<int>{1}, "redo recreates it");

    doc.setRGroupLogic(1, QStringLiteral("1,2"), true, 1);
    QString range; bool resth = false; int ifthen = 0;
    doc.molecule().rgroupLogic(1, range, resth, ifthen);
    CHECK(range == QStringLiteral("1,2") && resth && ifthen == 1, "setRGroupLogic applied");
    doc.undo();
    doc.molecule().rgroupLogic(1, range, resth, ifthen);
    CHECK(range.isEmpty() && !resth && ifthen == 0, "undo restores default logic fields");
    doc.setRGroupLogic(1, QStringLiteral("1,2"), true, 1); // reapply so the rest of this test has it set

    doc.selectAtom(a1);
    doc.addAtomToSelection(a2);
    doc.addRGroupMember(1);
    QList<int> members = doc.molecule().rgroupFragmentIds(1);
    CHECK(members.size() == 2, "addRGroupMember registers both selected atoms' (distinct) fragments");
    doc.undo();
    CHECK(doc.molecule().rgroupFragmentIds(1).isEmpty(), "undo removes both member fragments");
    doc.redo();
    CHECK(doc.molecule().rgroupFragmentIds(1).size() == 2, "redo restores both");

    bool canUndoBefore = doc.canUndo();
    doc.addRGroupMember(1); // same selection again -- no NEW fragments -- must no-op
    CHECK(doc.canUndo() == canUndoBefore, "addRGroupMember with nothing new to add pushes no history entry");

    int fragToRemove = members.first();
    doc.removeRGroupMember(1, fragToRemove);
    CHECK(doc.molecule().rgroupFragmentIds(1).size() == 1, "removeRGroupMember removes one fragment");
    doc.undo();
    CHECK(doc.molecule().rgroupFragmentIds(1).size() == 2, "undo restores it");

    doc.removeRGroupMember(1, 9999);
    CHECK(!doc.canUndo() || doc.molecule().rgroupFragmentIds(1).size() == 2,
          "removeRGroupMember with a non-member fragId is a no-op");

    CHECK(doc.deleteRGroup(1), "deleteRGroup succeeds");
    CHECK(doc.molecule().rgroupNumbers().isEmpty(), "R-group gone");
    doc.undo();
    CHECK(doc.molecule().rgroupNumbers() == QList<int>{1}, "undo recreates the R-group");
    doc.molecule().rgroupLogic(1, range, resth, ifthen);
    CHECK(range == QStringLiteral("1,2") && resth && ifthen == 1, "undo restores its logic fields too");
    CHECK(doc.molecule().rgroupFragmentIds(1).size() == 2, "undo restores its member fragments too");

    CHECK(!doc.deleteRGroup(9999), "deleteRGroup fails for an unknown R-group number");
}

static void test_textCommands() {
    std::printf("--- Test 20: text commands ---\n");
    DocumentState doc;

    TextId t1 = doc.addText(QStringLiteral("hello"), 1, 2, false, false);
    double x = 0, y = 0; QString content; bool bold = false, italic = false;
    CHECK(doc.molecule().textAnnotationContent(t1, x, y, content, bold, italic), "addText creates a real annotation");
    CHECK(content == QStringLiteral("hello") && x == 1 && y == 2, "content and position match");
    doc.undo();
    CHECK(doc.molecule().textAnnotationIds().isEmpty(), "undo removes the created text");

    // Recreate t1 for real -- the throwaway above only proved undo works; everything below needs
    // a persistent first annotation to compare counts against.
    t1 = doc.addText(QStringLiteral("hello"), 1, 2, false, false);
    bool canUndoBeforeNoOps = doc.canUndo();

    CHECK(doc.addText(QString(), 0, 0, false, false) == -1, "addText on an empty string returns -1 (no-op)");
    CHECK(doc.addText(QStringLiteral("   "), 0, 0, false, false) == -1, "addText on a whitespace-only string returns -1 (no-op)");
    CHECK(doc.canUndo() == canUndoBeforeNoOps, "neither no-op addText call pushed a history entry");

    TextId t2 = doc.addText(QStringLiteral("world"), 0, 0, false, false);
    doc.updateText(t2, QStringLiteral("WORLD"), true, true);
    CHECK(doc.molecule().textAnnotationContent(t2, x, y, content, bold, italic), "re-read after updateText");
    CHECK(content == QStringLiteral("WORLD") && bold && italic, "updateText changed content/bold/italic");
    doc.undo();
    CHECK(doc.molecule().textAnnotationContent(t2, x, y, content, bold, italic), "re-read after undo");
    CHECK(content == QStringLiteral("world") && !bold && !italic, "undo restores the original content/bold/italic");

    doc.deleteText(t2);
    CHECK(doc.molecule().textAnnotationIds().size() == 1, "deleteText removes the annotation");
    doc.undo();
    CHECK(doc.molecule().textAnnotationIds().size() == 2, "undo recreates it (fresh id)");

    doc.deleteText(9999);
    CHECK(!doc.canUndo() || doc.molecule().textAnnotationIds().size() == 2, "deleteText on an unknown id is a no-op");
}

static void test_addBracketSelection() {
    std::printf("--- Test 21: addBracketSelection ---\n");
    DocumentState doc;
    AtomId a1 = doc.addAtom(QStringLiteral("C"), 0, 0);
    AtomId a2 = doc.addAtom(QStringLiteral("C"), 4, 3);
    doc.addBond(a1, a2, 1);

    bool canUndoBeforeEmpty = doc.canUndo();
    doc.addBracketSelection(); // nothing selected -- no-op
    CHECK(doc.canUndo() == canUndoBeforeEmpty, "addBracketSelection with an empty selection is a no-op");

    doc.selectAtom(a1);
    doc.addAtomToSelection(a2);
    doc.addBracketSelection();
    CHECK(doc.molecule().bracketCount() == 1, "addBracketSelection pushes one bracket");
    double minX = 0, minY = 0, maxX = 0, maxY = 0;
    doc.molecule().bracketAt(0, minX, minY, maxX, maxY);
    CHECK(minX == 0 - 0.8 && minY == 0 - 0.8 && maxX == 4 + 0.8 && maxY == 3 + 0.8,
          "bracket bbox covers both atoms padded by 0.8, matching the real function");
    doc.undo();
    CHECK(doc.molecule().bracketCount() == 0, "undo pops the bracket");

    // Bond-only selection also contributes both endpoints' positions.
    doc.clearSelection();
    BondId b1 = doc.molecule().bondIds().first();
    doc.addBondToSelection(b1);
    doc.addBracketSelection();
    CHECK(doc.molecule().bracketCount() == 1, "a bond-only selection also produces a bracket");
    doc.molecule().bracketAt(0, minX, minY, maxX, maxY);
    CHECK(maxX == 4 + 0.8, "bond's far endpoint contributes to the bbox");
}

static void test_imageCommands() {
    std::printf("--- Test 22: image add/delete ---\n");
    DocumentState doc;

    ImageId img1 = doc.addImage(QByteArray("fakepng"), 1, 2, 4, 3);
    double x = 0, y = 0, w = 0, h = 0; QByteArray png;
    CHECK(doc.molecule().imageData(img1, x, y, w, h, png), "addImage creates a real image");
    CHECK(x == 1 && y == 2 && w == 4 && h == 3, "center/half-extents match the arguments");
    CHECK(png == QByteArray("fakepng"), "image data round-trips");
    doc.undo();
    CHECK(doc.molecule().imageIds().isEmpty(), "undo removes the created image");

    CHECK(doc.addImage(QByteArray(), 0, 0, 1, 1) == -1, "addImage with empty data returns -1 (no-op)");
    CHECK(!doc.canUndo(), "the no-op addImage call pushed no history entry");

    ImageId img2 = doc.addImage(QByteArray("otherpng"), 0, 0, 1, 1);
    doc.deleteImage(img2);
    CHECK(doc.molecule().imageIds().isEmpty(), "deleteImage removes the image");
    doc.undo();
    CHECK(doc.molecule().imageIds().size() == 1, "undo recreates it (fresh id)");
    ImageId restored = doc.molecule().imageIds().first();
    CHECK(doc.molecule().imageData(restored, x, y, w, h, png) && png == QByteArray("otherpng"),
          "restored image keeps its original data/geometry");

    doc.deleteImage(9999);
    CHECK(!doc.canUndo() || doc.molecule().imageIds().size() == 1, "deleteImage on an unknown id is a no-op");
}

static void test_deserializeMol() {
    std::printf("--- Test 25: deserializeMol ---\n");
    DocumentState doc;
    AtomId a1 = doc.addAtom(QStringLiteral("C"), 0, 0);
    doc.molecule().setName(QStringLiteral("original"));
    doc.selectAtom(a1);
    CHECK(!doc.selection().atoms.isEmpty(), "setup: something is selected before deserializeMol");

    doc.deserializeMol(QStringLiteral("c1ccccc1"));
    CHECK(doc.molecule().atomCount() == 6, "deserializeMol replaces the document with benzene's 6 atoms");
    CHECK(doc.molecule().name().isEmpty(), "the old name is gone (extension data wiped, matching the real JS)");
    CHECK(doc.selection().atoms.isEmpty(), "selection is cleared by deserializeMol itself, not just by a later undo/redo");

    doc.undo();
    CHECK(doc.molecule().atomCount() == 1, "undo restores the original 1-atom document");
    CHECK(doc.molecule().name() == QStringLiteral("original"), "undo restores the original name too");

    doc.redo();
    CHECK(doc.molecule().atomCount() == 6, "redo re-applies the load");

    bool canUndoBefore = doc.canUndo();
    doc.deserializeMol(QStringLiteral("not a valid molecule at all $$$"));
    CHECK(doc.canUndo() == canUndoBefore, "a parse failure pushes no history entry (no-op)");
    CHECK(doc.molecule().atomCount() == 6, "a parse failure leaves the current document completely unchanged");
}

static void test_clearCanvas() {
    std::printf("--- Test: clearCanvas ---\n");
    DocumentState doc;
    AtomId a1 = doc.addAtom(QStringLiteral("C"), 0, 0);
    AtomId a2 = doc.addAtom(QStringLiteral("N"), 1, 0);
    doc.addBond(a1, a2, 1);
    doc.selectAtom(a1);
    EditableMolecule& mol = doc.molecule();
    CHECK(mol.atomCount() == 2, "setup: 2 atoms before clearCanvas");
    CHECK(!doc.selection().atoms.isEmpty(), "setup: something selected before clearCanvas");

    doc.clearCanvas();

    CHECK(mol.atomCount() == 0, "document is empty after clearCanvas");
    CHECK(doc.selection().isEmpty(), "selection cleared after clearCanvas");
    CHECK(doc.isDirty(), "document is dirty after clearCanvas (an executed command, not markClean)");
    CHECK(doc.canUndo(), "undo available");

    doc.undo();
    CHECK(mol.atomCount() == 2, "undo restores the original 2 atoms");
    QString sym1, sym2;
    for (AtomId id : mol.atomIds()) {
        if (sym1.isEmpty()) sym1 = mol.atomSymbol(id); else sym2 = mol.atomSymbol(id);
    }
    CHECK((sym1 == QStringLiteral("C") && sym2 == QStringLiteral("N")) ||
          (sym1 == QStringLiteral("N") && sym2 == QStringLiteral("C")),
          "restored atoms have their original symbols");
    CHECK(mol.bondCount() == 1, "restored bond too");
    CHECK(doc.selection().isEmpty(),
          "selection is EMPTY after undo, NOT restored -- undo()'s own generic post-invert "
          "clear always wins, matching the real JS's clearCanvas (its own selection-restore "
          "is dead code there for the identical reason)");
}

static void test_loadBenzene() {
    std::printf("--- Test: loadBenzene ---\n");
    DocumentState doc;
    AtomId a1 = doc.addAtom(QStringLiteral("C"), 0, 0);
    (void)a1;
    EditableMolecule& mol = doc.molecule();
    CHECK(doc.canUndo(), "setup: history has an entry before loadBenzene");
    CHECK(doc.isDirty(), "setup: document is dirty before loadBenzene");

    doc.loadBenzene();

    CHECK(mol.atomCount() == 6, "6 atoms after loadBenzene");
    int carbonCount = 0;
    for (AtomId id : mol.atomIds()) if (mol.atomSymbol(id) == QStringLiteral("C")) ++carbonCount;
    CHECK(carbonCount == 6, "all 6 atoms are carbon");
    CHECK(mol.bondCount() == 6, "6 bonds (a ring)");
    CHECK(doc.selection().isEmpty(), "selection is empty after loadBenzene");

    CHECK(!doc.canUndo(), "history wiped -- cannot undo past loadBenzene");
    CHECK(!doc.canRedo(), "history wiped -- nothing to redo either");
    CHECK(!doc.isDirty(), "a freshly-loaded benzene document is NOT dirty");
}

static void test_setMoleculeName() {
    std::printf("--- Test 26: setMoleculeName ---\n");
    DocumentState doc;
    CHECK(doc.molecule().name().isEmpty(), "default name is empty");

    doc.setMoleculeName(QStringLiteral("Aspirin"));
    CHECK(doc.molecule().name() == QStringLiteral("Aspirin"), "setMoleculeName sets the name");
    bool canUndoBefore = doc.canUndo();
    doc.setMoleculeName(QStringLiteral("Aspirin")); // unchanged -- must no-op
    CHECK(doc.canUndo() == canUndoBefore, "setMoleculeName to the SAME name pushes no history entry");
    doc.undo();
    CHECK(doc.molecule().name().isEmpty(), "undo restores the original (empty) name");

    doc.redo();
    CHECK(doc.molecule().name() == QStringLiteral("Aspirin"), "redo re-applies the name");
}

static void test_copySelection() {
    std::printf("--- Test 27: copySelection ---\n");
    DocumentState doc;
    CHECK(doc.copySelection().isEmpty(), "empty selection returns an empty string");

    AtomId a1 = doc.addAtom(QStringLiteral("C"), 0, 0);
    AtomId a2 = doc.addAtom(QStringLiteral("O"), 1, 0);
    BondId b1 = doc.addBond(a1, a2, 1);
    doc.selectAtom(a1);
    doc.addAtomToSelection(a2);
    doc.addBondToSelection(b1);

    bool canUndoBefore = doc.canUndo();
    QString copied = doc.copySelection();
    CHECK(!copied.isEmpty(), "copySelection returns real MOL text for a non-empty selection");
    int reparsed = indigoLoadMoleculeFromString(copied.toUtf8().constData());
    CHECK(reparsed >= 0, "copied text reparses cleanly");
    if (reparsed >= 0) {
        CHECK(indigoCountAtoms(reparsed) == 2 && indigoCountBonds(reparsed) == 1, "copied structure has exactly the selected atoms/bond");
        indigoFree(reparsed);
    }
    CHECK(doc.canUndo() == canUndoBefore, "copySelection is a pure read -- it pushes no history entry");
}

static void test_insertStructureAt() {
    std::printf("--- Test 28: insertStructureAt ---\n");
    DocumentState doc;

    doc.insertStructureAt(QStringLiteral("CCO"), 10, 5);
    CHECK(doc.molecule().atomCount() == 3, "insertStructureAt merges the source's atoms into the document");

    // Confirm the pasted structure's bbox center landed at (10, 5).
    double minX = 1e9, minY = 1e9, maxX = -1e9, maxY = -1e9;
    for (AtomId id : doc.molecule().atomIds()) {
        double x = 0, y = 0;
        doc.molecule().atomPos(id, x, y);
        minX = std::min(minX, x); maxX = std::max(maxX, x);
        minY = std::min(minY, y); maxY = std::max(maxY, y);
    }
    CHECK(std::abs((minX + maxX) / 2.0 - 10.0) < 1e-6, "pasted structure's bbox center X is at cx");
    CHECK(std::abs((minY + maxY) / 2.0 - 5.0) < 1e-6, "pasted structure's bbox center Y is at cy");

    doc.undo();
    CHECK(doc.molecule().atomCount() == 0, "undo removes every atom/bond the paste created");
    doc.redo();
    CHECK(doc.molecule().atomCount() == 3, "redo re-applies the paste");

    bool canUndoBefore = doc.canUndo();
    doc.insertStructureAt(QString(), 0, 0);
    CHECK(doc.canUndo() == canUndoBefore, "empty source is a no-op -- no history entry");
    doc.insertStructureAt(QStringLiteral("not a valid molecule $$$"), 0, 0);
    CHECK(doc.canUndo() == canUndoBefore, "unparseable source is a no-op -- no history entry");
    CHECK(doc.molecule().atomCount() == 3, "document is unchanged by either no-op call");
}

static void test_insertStructureAtClampsToPageBounds() {
    std::printf("--- Test 29: insertStructureAt clamps to page bounds ---\n");
    DocumentState doc;
    doc.insertStructureAt(QStringLiteral("C"), 9999.0, -9999.0);
    CHECK(doc.molecule().atomCount() == 1, "structure is still inserted even with wildly out-of-range coordinates");
    double x = 0, y = 0;
    doc.molecule().atomPos(doc.molecule().atomIds().first(), x, y);
    CHECK(x <= 30.0 && y >= -21.0, "resulting position is clamped within the page bounds, not left at the raw input");
}

static void test_copyPasteRoundTrip() {
    std::printf("--- Test 30: copySelection -> insertStructureAt round-trip ---\n");
    DocumentState doc;
    AtomId a1 = doc.addAtom(QStringLiteral("C"), 0, 0);
    AtomId a2 = doc.addAtom(QStringLiteral("O"), 1, 0);
    doc.addBond(a1, a2, 1);
    doc.selectAtom(a1);
    doc.addAtomToSelection(a2);
    doc.addBondToSelection(doc.molecule().bondIds().first());

    QString copied = doc.copySelection();
    CHECK(!copied.isEmpty(), "setup: copySelection produced real text");

    doc.insertStructureAt(copied, 20, 20);
    CHECK(doc.molecule().atomCount() == 4, "the document now has both the original and the pasted atoms");
}

static void test_deleteSelectionEntitiesBondedPair() {
    std::printf("--- Test: deleteSelectionEntities on two bonded, individually-selected atoms ---\n");
    DocumentState doc;
    EditableMolecule& mol = doc.molecule();
    AtomId a = doc.addAtom(QStringLiteral("C"), 0.0, 0.0);
    AtomId b = doc.addAtom(QStringLiteral("N"), 1.0, 0.0);
    BondId bond = doc.addBond(a, b, 1);
    CHECK(mol.atomCount() == 2 && mol.bondCount() == 1, "setup: 2 atoms, 1 bond");

    doc.selectAtom(a);
    doc.addAtomToSelection(b);
    doc.deleteSelectionEntities();

    CHECK(mol.atomCount() == 0, "both atoms gone");
    CHECK(mol.bondCount() == 0, "bond gone (cascade)");
    CHECK(doc.canUndo(), "undo available");

    doc.undo();
    CHECK(mol.atomCount() == 2, "both atoms restored (fresh ids)");
    CHECK(mol.bondCount() == 1, "bond restored between the new ids");
    QList<AtomId> restored = mol.atomIds();
    CHECK(restored.size() == 2, "exactly 2 atoms after undo");
    BondId restoredBond = mol.findBond(restored[0], restored[1]);
    CHECK(restoredBond != -1, "restored bond connects the two restored atoms");
    (void)bond;
}

static void test_deleteSelectionEntitiesWholePill() {
    std::printf("--- Test: deleteSelectionEntities on a whole sgroup pill selected directly ---\n");
    // Built programmatically, NOT via a molfile constructor: AtomId/SGroupId are independent
    // counters both starting at 1, so a molfile-loaded sgroup (the FIRST sgroup ever created
    // for a fresh molecule) always gets SGroupId=1 -- which collides with AtomId=1 (the first
    // atom) whenever one exists, exactly the numeric-collision hazard this spec's own
    // deleteSelectionEntities design explicitly guards against. A decoy atom is added and
    // removed first so SGroupId=1 does NOT alias any real, currently-existing AtomId, letting
    // this test actually exercise the whole-pill-selection path instead of silently
    // misclassifying it as "atom 1 selected" (which corrupts sgroup state and crashes on undo).
    DocumentState doc;
    EditableMolecule& mol = doc.molecule();
    AtomId decoy = mol.addAtom(QStringLiteral("Xe"), 100.0, 100.0);
    mol.removeAtom(decoy);   // frees AtomId 1 so it no longer collides with the sgroup id below

    AtomId a1 = mol.addAtom(QStringLiteral("C"), 0.0, 0.0);
    AtomId a2 = mol.addAtom(QStringLiteral("C"), 1.0, 0.0);
    AtomId a3 = mol.addAtom(QStringLiteral("O"), 2.0, 0.0);
    mol.addBond(a1, a2, 1);
    mol.addBond(a2, a3, 1);
    SGroupId sid = mol.createSuperatomFromAtoms({a1, a2, a3});
    CHECK(sid != -1, "setup: sgroup created");
    CHECK(mol.sgroupIds().size() == 1, "1 sgroup on setup");
    CHECK(!mol.atomIds().contains(sid), "setup: sgroup id does not collide with any real atom id");

    doc.selection().atoms.insert(sid);   // whole-pill selection: the SGroupId stands in
                                          // for its atoms in m_selection.atoms, per spec.
    doc.deleteSelectionEntities();

    CHECK(mol.sgroupIds().isEmpty(), "sgroup grouping gone");
    CHECK(mol.atomCount() == 3, "member atoms untouched");

    doc.undo();
    CHECK(mol.sgroupIds().size() == 1, "sgroup restored on undo");
    CHECK(mol.atomCount() == 3, "still exactly the original 3 atoms (never removed)");
}

static void test_deleteSelectionEntitiesPartialMember() {
    std::printf("--- Test: deleteSelectionEntities on ONE of a sgroup's several members ---\n");
    const char* molfile =
        "acFixture\n"
        "  Ketcher\n\n"
        "  3  2  0  0  0  0  0  0  0  0999 V2000\n"
        "    0.0000    0.0000    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0\n"
        "    1.0000    0.0000    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0\n"
        "    2.0000    0.0000    0.0000 O   0  0  0  0  0  0  0  0  0  0  0  0\n"
        "  1  2  1  0  0  0  0\n"
        "  2  3  1  0  0  0  0\n"
        "M  STY  1   1 SUP\n"
        "M  SAL   1  3   1   2   3\n"
        "M  SMT   1 Ac\n"
        "M  END\n";
    DocumentState doc(molfile);
    EditableMolecule& mol = doc.molecule();
    QList<AtomId> allAtoms = mol.atomIds();
    CHECK(allAtoms.size() == 3, "3 atoms on load");
    AtomId victim = allAtoms.last();   // the terminal O, member of the sgroup

    doc.selectAtom(victim);
    doc.deleteSelectionEntities();

    CHECK(mol.atomCount() == 2, "1 atom removed");
    CHECK(mol.sgroupIds().size() == 1, "sgroup survives (trimmed, not fully removed)");
    SGroupId sid = mol.sgroupIds().first();
    CHECK(mol.sgroupMemberAtomIds(sid).size() == 2, "sgroup member list trimmed to 2");

    doc.undo();
    CHECK(mol.atomCount() == 3, "atom restored");
    CHECK(mol.sgroupIds().size() == 1, "still 1 sgroup after undo");
    CHECK(mol.sgroupMemberAtomIds(mol.sgroupIds().first()).size() == 3,
          "sgroup's full original 3-atom member set restored");
}

static void test_deleteSelectionEntitiesMixedTypes() {
    std::printf("--- Test: deleteSelectionEntities on a mix of atoms/bonds/rxnArrow/rxnPlus ---\n");
    DocumentState doc;
    EditableMolecule& mol = doc.molecule();
    AtomId a = doc.addAtom(QStringLiteral("C"), 0.0, 0.0);
    RxnArrowId arrow = doc.addRxnArrow(5.0, 0.0);
    RxnPlusId plus = doc.addRxnPlus(-5.0, 0.0);
    CHECK(mol.atomCount() == 1 && mol.rxnArrowCount() == 1 && mol.rxnPlusCount() == 1,
          "setup: 1 atom, 1 arrow, 1 plus");

    doc.selectAtom(a);
    doc.addRxnArrowToSelection(arrow);
    doc.addRxnPlusToSelection(plus);
    doc.deleteSelectionEntities();

    CHECK(mol.atomCount() == 0, "atom gone");
    CHECK(mol.rxnArrowCount() == 0, "arrow gone");
    CHECK(mol.rxnPlusCount() == 0, "plus gone");

    doc.undo();
    CHECK(mol.atomCount() == 1, "atom restored");
    CHECK(mol.rxnArrowCount() == 1, "arrow restored");
    CHECK(mol.rxnPlusCount() == 1, "plus restored");
}

static void test_cutSelection() {
    std::printf("--- Test: cutSelection (copy-then-delete round trip) ---\n");
    DocumentState doc;
    EditableMolecule& mol = doc.molecule();
    AtomId a = doc.addAtom(QStringLiteral("C"), 0.0, 0.0);
    AtomId b = doc.addAtom(QStringLiteral("O"), 1.0, 0.0);
    doc.addBond(a, b, 1);

    doc.selectAtom(a);
    doc.addAtomToSelection(b);
    QString expectedCopy = doc.copySelection();
    CHECK(!expectedCopy.isEmpty(), "setup: copySelection produces content before cut");

    QString cutResult = doc.cutSelection();
    CHECK(cutResult == expectedCopy, "cutSelection returns the same text copySelection would have");
    CHECK(mol.atomCount() == 0, "selected content is gone from the live document");

    doc.undo();
    CHECK(mol.atomCount() == 2, "single undo restores everything");
}

static void test_setStereoDescriptorsValid() {
    std::printf("--- Test: setStereoDescriptors applies valid JSON ---\n");
    DocumentState doc;
    AtomId a1 = doc.addAtom(QStringLiteral("C"), 0, 0);
    AtomId a2 = doc.addAtom(QStringLiteral("C"), 1, 0);
    doc.addBond(a1, a2, 1);
    EditableMolecule& mol = doc.molecule();

    QList<AtomId> order = mol.atomIdsInIndigoOrder();
    CHECK(order.size() == 2, "setup: 2 atoms");
    int idx0 = order.indexOf(a1), idx1 = order.indexOf(a2);

    QString json = QString(
        "{\"atoms\":{\"%1\":{\"cipLabel\":\"R\",\"type\":1,\"group\":2}},"
        "\"bonds\":{\"%2-%3\":{\"cipLabel\":\"E\"}}}"
    ).arg(idx0).arg(qMin(idx0, idx1)).arg(qMax(idx0, idx1));

    bool historyBefore = doc.canUndo();
    doc.setStereoDescriptors(json);

    CHECK(mol.atomStoredCipLabel(a1) == QStringLiteral("R"), "atom cipLabel applied");
    CHECK(mol.atomStoredStereoType(a1) == 1, "atom stereoType applied");
    CHECK(mol.atomStoredStereoGroup(a1) == 2, "atom stereoGroup applied");
    CHECK(mol.atomStoredCipLabel(a2).isEmpty(), "the OTHER atom is untouched");
    BondId bond = mol.findBond(a1, a2);
    CHECK(mol.bondStoredCipLabel(bond) == QStringLiteral("E"), "bond cipLabel applied");
    CHECK(doc.canUndo() == historyBefore, "no history entry pushed");
}

static void test_setStereoDescriptorsReuseOrderRegression() {
    std::printf("--- Test: setStereoDescriptors resolves correctly after an index-reuse edit ---\n");
    DocumentState doc;
    EditableMolecule& mol = doc.molecule();
    AtomId a1 = doc.addAtom(QStringLiteral("C"), 0, 0);
    AtomId a2 = doc.addAtom(QStringLiteral("N"), 1, 0);
    AtomId a3 = doc.addAtom(QStringLiteral("O"), 2, 0);
    mol.removeAtom(a2);
    AtomId a4 = doc.addAtom(QStringLiteral("Cl"), 3, 0);

    QList<AtomId> order = mol.atomIdsInIndigoOrder();
    int idxOfA4 = order.indexOf(a4);
    QString json = QString("{\"atoms\":{\"%1\":{\"cipLabel\":\"S\",\"type\":1,\"group\":0}},\"bonds\":{}}")
                       .arg(idxOfA4);

    doc.setStereoDescriptors(json);

    CHECK(mol.atomStoredCipLabel(a4) == QStringLiteral("S"),
          "the atom that reused the freed index gets the label -- NOT a1/a3");
    CHECK(mol.atomStoredCipLabel(a1).isEmpty(), "a1 untouched");
    CHECK(mol.atomStoredCipLabel(a3).isEmpty(), "a3 untouched");
}

static void test_setStereoDescriptorsResetsOnEachCall() {
    std::printf("--- Test: setStereoDescriptors replaces, does not accumulate ---\n");
    DocumentState doc;
    EditableMolecule& mol = doc.molecule();
    AtomId a1 = doc.addAtom(QStringLiteral("C"), 0, 0);
    int idx0 = mol.atomIdsInIndigoOrder().indexOf(a1);

    doc.setStereoDescriptors(QString("{\"atoms\":{\"%1\":{\"cipLabel\":\"R\",\"type\":1,\"group\":0}},\"bonds\":{}}").arg(idx0));
    CHECK(mol.atomStoredCipLabel(a1) == QStringLiteral("R"), "first call applies R");

    doc.setStereoDescriptors(QString("{\"atoms\":{},\"bonds\":{}}"));
    CHECK(mol.atomStoredCipLabel(a1).isEmpty(), "second call with empty payload clears it (reset ran)");
}

static void test_setStereoDescriptorsMalformedJsonIsNoOp() {
    std::printf("--- Test: setStereoDescriptors ignores malformed JSON ---\n");
    DocumentState doc;
    EditableMolecule& mol = doc.molecule();
    AtomId a1 = doc.addAtom(QStringLiteral("C"), 0, 0);
    int idx0 = mol.atomIdsInIndigoOrder().indexOf(a1);
    doc.setStereoDescriptors(QString("{\"atoms\":{\"%1\":{\"cipLabel\":\"R\",\"type\":1,\"group\":0}},\"bonds\":{}}").arg(idx0));
    CHECK(mol.atomStoredCipLabel(a1) == QStringLiteral("R"), "setup: R applied");

    doc.setStereoDescriptors(QStringLiteral("not valid json{{{"));
    CHECK(mol.atomStoredCipLabel(a1) == QStringLiteral("R"), "malformed JSON leaves prior data unchanged");
}

static void test_setStereoDescriptorsOutOfRangeIndexSkipped() {
    std::printf("--- Test: setStereoDescriptors skips an out-of-range index safely ---\n");
    DocumentState doc;
    EditableMolecule& mol = doc.molecule();
    AtomId a1 = doc.addAtom(QStringLiteral("C"), 0, 0);
    int idx0 = mol.atomIdsInIndigoOrder().indexOf(a1);

    QString json = QString(
        "{\"atoms\":{\"%1\":{\"cipLabel\":\"R\",\"type\":1,\"group\":0},"
        "\"99\":{\"cipLabel\":\"S\",\"type\":1,\"group\":0}},\"bonds\":{}}"
    ).arg(idx0);
    doc.setStereoDescriptors(json);

    CHECK(mol.atomStoredCipLabel(a1) == QStringLiteral("R"),
          "the valid entry still applies even though the payload also has an out-of-range one");
}

static void test_setCheckIssuesValid() {
    std::printf("--- Test: setCheckIssues applies atom- and bond-target issues ---\n");
    DocumentState doc;
    EditableMolecule& mol = doc.molecule();
    AtomId a1 = doc.addAtom(QStringLiteral("C"), 0, 0);
    AtomId a2 = doc.addAtom(QStringLiteral("C"), 1, 0);
    BondId bond = doc.addBond(a1, a2, 1);

    QList<AtomId> atomOrder = mol.atomIdsInIndigoOrder();
    QList<BondId> bondOrder = mol.bondIdsInIndigoOrder();
    int atomIdx = atomOrder.indexOf(a1);
    int bondIdx = bondOrder.indexOf(bond);

    bool historyBefore = doc.canUndo();
    QString json = QString(
        "{\"issues\":["
        "{\"target\":\"atom\",\"type\":\"valence\",\"ids\":[%1]},"
        "{\"target\":\"bond\",\"type\":\"overlap_atom\",\"ids\":[%2]}"
        "]}"
    ).arg(atomIdx).arg(bondIdx);
    doc.setCheckIssues(json);

    CHECK(mol.atomCheckWarningText(a1) == QStringLiteral("valence"), "atom checkWarning applied");
    CHECK(mol.atomCheckWarningText(a2).isEmpty(), "the OTHER atom is untouched");
    CHECK(mol.bondCheckWarningText(bond) == QStringLiteral("overlap_atom"), "bond checkWarning applied");
    CHECK(doc.canUndo() == historyBefore, "no history entry pushed");
}

static void test_setCheckIssuesDefaultTargetIsAtom() {
    std::printf("--- Test: setCheckIssues defaults missing/unrecognized target to atom ---\n");
    DocumentState doc;
    EditableMolecule& mol = doc.molecule();
    AtomId a1 = doc.addAtom(QStringLiteral("C"), 0, 0);
    int idx = mol.atomIdsInIndigoOrder().indexOf(a1);

    doc.setCheckIssues(QString("{\"issues\":[{\"type\":\"radical\",\"ids\":[%1]}]}").arg(idx));
    CHECK(mol.atomCheckWarningText(a1) == QStringLiteral("radical"),
          "an issue with no target field applies to the atom, matching the real JS's || \"atom\" default");
}

static void test_setCheckIssuesResetsOnEachCall() {
    std::printf("--- Test: setCheckIssues replaces, does not accumulate ---\n");
    DocumentState doc;
    EditableMolecule& mol = doc.molecule();
    AtomId a1 = doc.addAtom(QStringLiteral("C"), 0, 0);
    int idx = mol.atomIdsInIndigoOrder().indexOf(a1);

    doc.setCheckIssues(QString("{\"issues\":[{\"target\":\"atom\",\"type\":\"valence\",\"ids\":[%1]}]}").arg(idx));
    CHECK(mol.atomCheckWarningText(a1) == QStringLiteral("valence"), "first call applies");

    doc.setCheckIssues(QStringLiteral("{\"issues\":[]}"));
    CHECK(mol.atomCheckWarningText(a1).isEmpty(), "second call with no issues clears it (reset ran)");
}

static void test_setCheckIssuesMalformedJsonIsNoOp() {
    std::printf("--- Test: setCheckIssues ignores malformed JSON ---\n");
    DocumentState doc;
    EditableMolecule& mol = doc.molecule();
    AtomId a1 = doc.addAtom(QStringLiteral("C"), 0, 0);
    int idx = mol.atomIdsInIndigoOrder().indexOf(a1);
    doc.setCheckIssues(QString("{\"issues\":[{\"target\":\"atom\",\"type\":\"valence\",\"ids\":[%1]}]}").arg(idx));
    CHECK(mol.atomCheckWarningText(a1) == QStringLiteral("valence"), "setup: applied");

    doc.setCheckIssues(QStringLiteral("{not json"));
    CHECK(mol.atomCheckWarningText(a1) == QStringLiteral("valence"), "malformed JSON leaves prior data unchanged");
}

static void test_setCheckIssuesReuseOrderRegression() {
    std::printf("--- Test: setCheckIssues resolves correctly after a bond index-reuse edit ---\n");
    DocumentState doc;
    EditableMolecule& mol = doc.molecule();
    AtomId a1 = doc.addAtom(QStringLiteral("C"), 0, 0);
    AtomId a2 = doc.addAtom(QStringLiteral("C"), 1, 0);
    AtomId a3 = doc.addAtom(QStringLiteral("C"), 2, 0);
    BondId b1 = doc.addBond(a1, a2, 1);
    doc.addBond(a2, a3, 1);
    mol.removeBond(b1);
    BondId b3 = doc.addBond(a1, a3, 2);

    QList<BondId> order = mol.bondIdsInIndigoOrder();
    int idx = order.indexOf(b3);
    doc.setCheckIssues(QString("{\"issues\":[{\"target\":\"bond\",\"type\":\"overlap_atom\",\"ids\":[%1]}]}").arg(idx));

    CHECK(mol.bondCheckWarningText(b3) == QStringLiteral("overlap_atom"),
          "the bond that reused the freed index gets the warning");
}

int main() {
    test_selectionStateBasics();
    test_editCommandBasics();
    test_documentStateUndoRedo();
    test_documentStateHistoryCap();
    test_documentStateSelection();
    test_documentStateEditingOperations();
    test_discreteTransforms();
    test_imageTransforms();
    test_liveDragMove();
    test_liveDragRotate();
    test_liveDragScale();
    test_dragStateResetOnUndoRedo();
    test_addRing();
    test_addChain();
    test_insertFunctionalGroup();
    test_insertLibraryTemplateFused();
    test_toggleSgroupExpanded();
    test_rxnArrowLifecycle();
    test_rxnPlusCommands();
    test_multitailArrowCommands();
    test_setStereoFlags();
    test_rgroupCommands();
    test_textCommands();
    test_addBracketSelection();
    test_imageCommands();
    test_deserializeMol();
    test_clearCanvas();
    test_loadBenzene();
    test_setMoleculeName();
    test_copySelection();
    test_insertStructureAt();
    test_insertStructureAtClampsToPageBounds();
    test_copyPasteRoundTrip();
    test_deleteSelectionEntitiesBondedPair();
    test_deleteSelectionEntitiesWholePill();
    test_deleteSelectionEntitiesPartialMember();
    test_deleteSelectionEntitiesMixedTypes();
    test_cutSelection();
    test_setStereoDescriptorsValid();
    test_setStereoDescriptorsReuseOrderRegression();
    test_setStereoDescriptorsResetsOnEachCall();
    test_setStereoDescriptorsMalformedJsonIsNoOp();
    test_setStereoDescriptorsOutOfRangeIndexSkipped();
    test_setCheckIssuesValid();
    test_setCheckIssuesDefaultTargetIsAtom();
    test_setCheckIssuesResetsOnEachCall();
    test_setCheckIssuesMalformedJsonIsNoOp();
    test_setCheckIssuesReuseOrderRegression();
    std::printf("Summary: %d passed, %d failed.\n", g_pass, g_fail);
    return g_fail == 0 ? 0 : 1;
}

