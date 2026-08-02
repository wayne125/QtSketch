// tests/document_state_test.cpp
// Standalone tests for DocumentState (chem-core.js migration, sub-project 2).
#include <cstdio>
#include <cmath>
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
    std::printf("Summary: %d passed, %d failed.\n", g_pass, g_fail);
    return g_fail == 0 ? 0 : 1;
}
