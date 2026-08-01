// tests/document_state_test.cpp
// Standalone tests for DocumentState (chem-core.js migration, sub-project 2).
#include <cstdio>
#include <cmath>
#include "app/molecule/SelectionState.h"
#include "app/molecule/EditCommand.h"
#include "app/molecule/DocumentState.h"
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
    std::printf("Summary: %d passed, %d failed.\n", g_pass, g_fail);
    return g_fail == 0 ? 0 : 1;
}
