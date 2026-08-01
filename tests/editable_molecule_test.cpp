// tests/editable_molecule_test.cpp
// Standalone tests for the EditableMolecule module (chem-core.js migration,
// sub-project 1). Test 0 is the ID-behavior spike from the design spec: it
// DOCUMENTS how Indigo indices behave under removal/re-add and VERIFIES the
// one property the design depends on (indigoClone preserves indices).
#include <cstdio>
#include <cstring>
#include "app/molecule/EditableMolecule.h"
#include "indigo.h"

static int g_pass = 0, g_fail = 0;
#define CHECK(cond, name) do { \
    if (cond) { ++g_pass; std::printf("[PASS] %s\n", name); } \
    else { ++g_fail; std::printf("[FAIL] %s (line %d)\n", name, __LINE__); } \
} while (0)

static void spike_idBehavior() {
    std::printf("--- Test 0: Indigo ID-behavior spike ---\n");
    int mol = indigoCreateMolecule();
    CHECK(mol >= 0, "spike: create empty molecule");

    // Add 5 carbons, record each one's index.
    int atomHandles[5], atomIdx[5];
    for (int i = 0; i < 5; ++i) {
        atomHandles[i] = indigoAddAtom(mol, "C");
        atomIdx[i] = indigoIndex(atomHandles[i]);
        indigoSetXYZ(atomHandles[i], i * 1.0f, 0.0f, 0.0f);
    }
    // Chain bonds 0-1-2-3-4.
    for (int i = 0; i < 4; ++i)
        indigoAddBond(atomHandles[i], atomHandles[i + 1], 1);

    // Remove atom #2 (the middle one).
    indigoRemove(atomHandles[2]);
    CHECK(indigoCountAtoms(mol) == 4, "spike: 4 atoms remain after removal");

    // OBSERVATION (not a hard requirement of the design): do survivors keep
    // their old indices? Print what we see either way.
    int survivorsKeptIndices = 1;
    const int surviving[4] = {0, 1, 3, 4};
    for (int s = 0; s < 4; ++s) {
        int a = indigoGetAtom(mol, atomIdx[surviving[s]]);
        if (a < 0) { survivorsKeptIndices = 0; break; }
        indigoFree(a);
    }
    std::printf("[INFO] spike: surviving atoms keep their pre-removal indices: %s\n",
                survivorsKeptIndices ? "YES" : "NO");

    // OBSERVATION: does a new atom reuse the freed index?
    int newAtom = indigoAddAtom(mol, "N");
    int newIdx = indigoIndex(newAtom);
    std::printf("[INFO] spike: freed index was %d, new atom got index %d (%s)\n",
                atomIdx[2], newIdx, newIdx == atomIdx[2] ? "REUSED" : "not reused");

    // HARD REQUIREMENT: indigoClone preserves indices (snapshot/restore
    // depends on this -- the ID indirection table maps external IDs to these
    // indices and is copied verbatim into snapshots).
    int clone = indigoClone(mol);
    CHECK(clone >= 0, "spike: clone succeeds");
    CHECK(indigoCountAtoms(clone) == indigoCountAtoms(mol),
          "spike: clone has same atom count");
    int cloneAtom = indigoGetAtom(clone, newIdx);
    CHECK(cloneAtom >= 0 && indigoAtomicNumber(cloneAtom) == 7,
          "spike: clone preserves the N atom at the same index");
    if (cloneAtom >= 0) indigoFree(cloneAtom);

    indigoFree(clone);
    indigoFree(mol);
}

static void test_constructionAndMolfile() {
    std::printf("--- Test 1: construction + molfile round-trip ---\n");

    EditableMolecule empty;
    CHECK(empty.isValid(), "empty molecule constructs valid");
    CHECK(empty.atomCount() == 0, "empty molecule has 0 atoms");

    // indigoLoadMoleculeFromString auto-detects SMILES.
    EditableMolecule ethanol(QStringLiteral("CCO"));
    CHECK(ethanol.isValid(), "ethanol constructs valid");
    CHECK(ethanol.atomCount() == 3, "ethanol has 3 heavy atoms");
    CHECK(ethanol.bondCount() == 2, "ethanol has 2 bonds");

    StringResult mf = ethanol.toMolfile();
    CHECK(mf.success, "toMolfile succeeds");
    CHECK(mf.value.contains(QStringLiteral("M  END")), "molfile has M  END");

    EditableMolecule bad(QStringLiteral("not_a_molecule((("));
    CHECK(!bad.isValid(), "garbage input yields invalid molecule");
    CHECK(!bad.lastError().isEmpty(), "invalid molecule carries an error string");
}

static void test_atomBondCrud() {
    std::printf("--- Test 2: atom/bond CRUD with stable IDs ---\n");
    EditableMolecule m;

    AtomId c1 = m.addAtom(QStringLiteral("C"), 0.0, 0.0);
    AtomId c2 = m.addAtom(QStringLiteral("C"), 1.0, 0.0);
    AtomId o1 = m.addAtom(QStringLiteral("O"), 2.0, 0.0);
    CHECK(c1 > 0 && c2 > 0 && o1 > 0, "addAtom returns positive IDs");
    CHECK(c1 != c2 && c2 != o1, "IDs are distinct");
    CHECK(m.atomCount() == 3, "3 atoms after adds");
    CHECK(m.atomSymbol(o1) == QStringLiteral("O"), "atomSymbol reads back");

    double x = -1, y = -1;
    CHECK(m.atomPos(c2, x, y) && x == 1.0 && y == 0.0, "atomPos reads back");

    BondId b1 = m.addBond(c1, c2, 1);
    BondId b2 = m.addBond(c2, o1, 2);
    CHECK(b1 > 0 && b2 > 0, "addBond returns positive IDs");
    CHECK(m.bondOrder(b2) == 2, "bondOrder reads back");

    // Remove the middle atom: its two incident bonds must die with it.
    CHECK(m.removeAtom(c2), "removeAtom succeeds");
    CHECK(m.atomCount() == 2, "2 atoms remain");
    CHECK(m.bondCount() == 0, "incident bonds removed with the atom");
    CHECK(m.bondOrder(b1) == -1, "stale BondId no longer resolves");
    CHECK(m.atomSymbol(c2).isEmpty(), "stale AtomId no longer resolves");

    // Survivors still resolve correctly through the indirection table even
    // if Indigo compacted/reused indices underneath.
    CHECK(m.atomSymbol(c1) == QStringLiteral("C"), "survivor c1 still resolves");
    CHECK(m.atomSymbol(o1) == QStringLiteral("O"), "survivor o1 still resolves");

    // A new atom must get a fresh ID, never a recycled one.
    AtomId n1 = m.addAtom(QStringLiteral("N"), 3.0, 0.0);
    CHECK(n1 != c2 && n1 > o1, "new ID is fresh, not recycled");
    CHECK(m.removeBond(m.addBond(c1, n1, 1)), "removeBond on a live bond succeeds");
    CHECK(!m.removeAtom(c2), "removing an already-removed ID fails cleanly");
}

static void test_extensionData() {
    std::printf("--- Test 3: extension data storage ---\n");
    EditableMolecule m(QStringLiteral("CCO"));
    QList<AtomId> ids = m.atomIds();

    m.setName(QStringLiteral("my ethanol"));
    CHECK(m.name() == QStringLiteral("my ethanol"), "name round-trips");

    CHECK(m.addTextAnnotation(1.0, 2.0, QStringLiteral("note")) >= 0, "text annotation added");
    CHECK(m.textAnnotationCount() == 1, "text annotation counted");

    CHECK(m.addRxnArrow(0, 0, 5, 0) >= 0, "rxn arrow added");
    CHECK(m.addRxnPlus(2.5, 0) >= 0, "rxn plus added");
    CHECK(m.rxnArrowCount() == 1 && m.rxnPlusCount() == 1, "arrow+plus counted");

    CHECK(m.addMultitailArrow({5,5, 0,4, 0,6}) >= 0, "multitail arrow added");
    CHECK(m.multitailArrowCount() == 1, "multitail counted");

    CHECK(m.addImage(0, 0, 4, 3, QByteArray("fakepng")) >= 0, "image added");
    CHECK(m.imageCount() == 1, "image counted");

    m.setStereoFlag(0, 2);
    CHECK(m.stereoFlag(0) == 2, "stereo flag round-trips");
    CHECK(m.stereoFlag(1) == -1, "unset stereo flag is -1");

    m.setAtomAAM(ids[0], 7);
    CHECK(m.atomAAM(ids[0]) == 7, "AAM round-trips");
    CHECK(m.atomAAM(ids[1]) == 0, "unset AAM is 0");

    m.setAtomCheckWarning(ids[2], true);
    CHECK(m.atomCheckWarning(ids[2]), "check warning round-trips");
    CHECK(!m.atomCheckWarning(ids[0]), "unset check warning is false");
}

static void test_sgroupPassthrough() {
    std::printf("--- Test 4: data sgroup passthrough ---\n");
    EditableMolecule m(QStringLiteral("CCO"));
    QList<AtomId> ids = m.atomIds();

    int sg = m.addDataSGroup({ids[0], ids[1]}, QStringLiteral("MYFIELD"), QStringLiteral("hello"));
    CHECK(sg >= 0, "data sgroup added");
    CHECK(m.dataSGroupCount() == 1, "data sgroup counted");

    StringResult mf = m.toMolfile();
    CHECK(mf.success && mf.value.contains(QStringLiteral("MYFIELD")), "sgroup survives molfile serialization");
}

static void test_snapshotRestore() {
    std::printf("--- Test 5: snapshot/restore undo round-trip ---\n");
    EditableMolecule m;
    AtomId a1 = m.addAtom(QStringLiteral("C"), 0, 0);
    AtomId a2 = m.addAtom(QStringLiteral("O"), 1, 0);
    m.addBond(a1, a2, 1);
    m.setName(QStringLiteral("before"));
    m.setAtomAAM(a1, 3);
    m.addTextAnnotation(0, 1, QStringLiteral("v1"));
    m.addDataSGroup({a1}, QStringLiteral("F"), QStringLiteral("d"));
    m.addRxnArrow(0, 0, 5, 0);
    m.addRxnPlus(2.5, 0);
    m.addMultitailArrow({5,5, 0,4, 0,6});
    m.addImage(0, 0, 4, 3, QByteArray("png1"));
    m.setStereoFlag(0, 1);
    m.setAtomCheckWarning(a2, true);

    MoleculeSnapshot snap = m.snapshot();

    AtomId a3 = m.addAtom(QStringLiteral("N"), 2, 0);
    m.addBond(a2, a3, 1);
    m.removeAtom(a1);
    m.setName(QStringLiteral("after"));
    m.setAtomAAM(a2, 9);
    m.addTextAnnotation(0, 2, QStringLiteral("v2"));
    CHECK(m.atomCount() == 2 && m.name() == QStringLiteral("after"), "mutations applied");

    CHECK(m.restore(snap), "restore succeeds");
    CHECK(m.atomCount() == 2, "atom count restored");
    CHECK(m.bondCount() == 1, "bond count restored");
    CHECK(m.name() == QStringLiteral("before"), "name restored");
    CHECK(m.atomAAM(a1) == 3, "AAM restored under the ORIGINAL AtomId");
    CHECK(m.atomSymbol(a1) == QStringLiteral("C"), "original AtomId resolves after restore");
    CHECK(m.textAnnotationCount() == 1, "extension list restored");
    CHECK(m.dataSGroupCount() == 1, "sgroup survives snapshot/restore");
    CHECK(m.rxnArrowCount() == 1 && m.rxnPlusCount() == 1, "rxn arrow+plus survive restore");
    CHECK(m.multitailArrowCount() == 1, "multitail arrow survives restore");
    CHECK(m.imageCount() == 1, "image survives restore");
    CHECK(m.stereoFlag(0) == 1, "stereo flag survives restore");
    CHECK(m.atomCheckWarning(a2), "check warning survives restore");

    m.removeAtom(a2);
    CHECK(m.restore(snap), "second restore of the same snapshot succeeds");
    CHECK(m.atomCount() == 2, "second restore rolls back again");

    AtomId a4 = m.addAtom(QStringLiteral("S"), 3, 0);
    CHECK(a4 > a3, "ID counter survives restore (no reuse of discarded IDs)");
}

static void test_queryFeatureSpike() {
    std::printf("--- Test 6: query-atom-features spike (spec risk 3) ---\n");
    // Molfile with an atom list ([N,O] on atom 1) -- V2000 query feature.
    const char* molWithAtomList =
        "\n  spike  \n\n"
        "  2  1  1  0  0  0  0  0  0  0999 V2000\n"
        "    0.0000    0.0000    0.0000 L   0  0  0  0  0  0  0  0  0  0  0  0\n"
        "    1.0000    0.0000    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0\n"
        "  1  2  1  0  0  0  0\n"
        "  1 T    2   7   8\n"
        "M  END\n";

    indigoSetSessionId(indigoAllocSessionId()); // fresh session, throwaway
    int plain = indigoLoadMoleculeFromString(molWithAtomList);
    std::printf("[INFO] spike: atom-list molfile into PLAIN molecule handle: %s\n",
                plain >= 0 ? "ACCEPTED" : indigoGetLastError());
    int query = indigoLoadQueryMoleculeFromString(molWithAtomList);
    std::printf("[INFO] spike: atom-list molfile into QUERY molecule handle: %s\n",
                query >= 0 ? "ACCEPTED" : indigoGetLastError());
    CHECK(plain >= 0 || query >= 0, "atom-list molfile loads via at least one loader");
    if (plain >= 0) indigoFree(plain);
    if (query >= 0) indigoFree(query);
}

static void test_extensionEntityRemoval() {
    std::printf("--- Test 7: extension entity removal with stable IDs ---\n");
    EditableMolecule m;

    int t1 = m.addTextAnnotation(0, 0, QStringLiteral("a"));
    int t2 = m.addTextAnnotation(1, 1, QStringLiteral("b"));
    CHECK(m.textAnnotationCount() == 2, "2 texts after adds");
    CHECK(m.removeTextAnnotation(t1), "removeTextAnnotation succeeds for a live id");
    CHECK(m.textAnnotationCount() == 1, "1 text remains after removal");
    CHECK(!m.removeTextAnnotation(t1), "removing an already-removed id fails cleanly");
    int t3 = m.addTextAnnotation(2, 2, QStringLiteral("c"));
    CHECK(t3 != t1 && t3 > t2, "new text id is fresh, not recycled");

    int ra1 = m.addRxnArrow(0, 0, 1, 1);
    CHECK(m.removeRxnArrow(ra1), "removeRxnArrow succeeds");
    CHECK(m.rxnArrowCount() == 0, "0 rxn arrows after removal");
    CHECK(!m.removeRxnArrow(ra1), "double-remove of rxn arrow fails cleanly");

    int rp1 = m.addRxnPlus(0, 0);
    CHECK(m.removeRxnPlus(rp1), "removeRxnPlus succeeds");
    CHECK(m.rxnPlusCount() == 0, "0 rxn pluses after removal");

    int mta1 = m.addMultitailArrow({0,0, 1,1});
    CHECK(m.removeMultitailArrow(mta1), "removeMultitailArrow succeeds");
    CHECK(m.multitailArrowCount() == 0, "0 multitail arrows after removal");

    int img1 = m.addImage(0, 0, 1, 1, QByteArray("x"));
    CHECK(m.removeImage(img1), "removeImage succeeds");
    CHECK(m.imageCount() == 0, "0 images after removal");
}

static void test_atomAttributeAccessors() {
    std::printf("--- Test 8: atom attribute accessors (charge/isotope/radical/valence) ---\n");
    EditableMolecule m;
    AtomId c1 = m.addAtom(QStringLiteral("C"), 0, 0);

    CHECK(m.atomCharge(c1) == 0, "fresh atom has charge 0");
    CHECK(m.setAtomCharge(c1, 1), "setAtomCharge succeeds");
    CHECK(m.atomCharge(c1) == 1, "atomCharge reads back the set value");
    CHECK(m.setAtomCharge(c1, -1), "setAtomCharge accepts negative charge");
    CHECK(m.atomCharge(c1) == -1, "atomCharge reads back negative charge");

    CHECK(m.atomIsotope(c1) == 0, "fresh atom has isotope 0 (none)");
    CHECK(m.setAtomIsotope(c1, 13), "setAtomIsotope succeeds");
    CHECK(m.atomIsotope(c1) == 13, "atomIsotope reads back the set value");

    CHECK(m.atomRadical(c1) == 0, "fresh atom has radical 0 (none)");
    CHECK(m.setAtomRadical(c1, INDIGO_SINGLET), "setAtomRadical succeeds");
    CHECK(m.atomRadical(c1) == INDIGO_SINGLET, "atomRadical reads back the set value");

    CHECK(m.atomExplicitValence(c1) == -1, "fresh atom has explicit valence -1 (unset)");
    CHECK(m.setAtomExplicitValence(c1, 4), "setAtomExplicitValence succeeds");
    CHECK(m.atomExplicitValence(c1) == 4, "atomExplicitValence reads back the set value");

    AtomId invalid = 9999;
    CHECK(m.atomCharge(invalid) == 0, "invalid id: atomCharge returns sentinel 0");
    CHECK(!m.setAtomCharge(invalid, 1), "invalid id: setAtomCharge fails cleanly");
    CHECK(m.atomIsotope(invalid) == 0, "invalid id: atomIsotope returns sentinel 0");
    CHECK(!m.setAtomIsotope(invalid, 1), "invalid id: setAtomIsotope fails cleanly");
    CHECK(m.atomRadical(invalid) == 0, "invalid id: atomRadical returns sentinel 0");
    CHECK(!m.setAtomRadical(invalid, 1), "invalid id: setAtomRadical fails cleanly");
    CHECK(m.atomExplicitValence(invalid) == -1, "invalid id: atomExplicitValence returns sentinel -1");
    CHECK(!m.setAtomExplicitValence(invalid, 1), "invalid id: setAtomExplicitValence fails cleanly");
}

static void test_atomLabelAndBondOrder() {
    std::printf("--- Test 9: setAtomLabel + setBondOrderValue ---\n");
    EditableMolecule m;
    AtomId c1 = m.addAtom(QStringLiteral("C"), 0, 0);
    m.setAtomCharge(c1, 1);
    m.setAtomIsotope(c1, 13);

    CHECK(m.setAtomLabel(c1, QStringLiteral("N")), "setAtomLabel to a valid element succeeds");
    CHECK(m.atomSymbol(c1) == QStringLiteral("N"), "atomSymbol reflects the new label");
    CHECK(m.atomCharge(c1) == 1, "charge survives the label change");
    CHECK(m.atomIsotope(c1) == 13, "isotope survives the label change");

    CHECK(!m.setAtomLabel(c1, QStringLiteral("NotAnElement")), "setAtomLabel rejects an invalid symbol");
    CHECK(m.atomSymbol(c1) == QStringLiteral("N"), "label unchanged after a rejected setAtomLabel");

    AtomId c2 = m.addAtom(QStringLiteral("C"), 1, 0);
    BondId b1 = m.addBond(c1, c2, 1);
    CHECK(m.bondOrder(b1) == 1, "fresh bond has order 1");
    CHECK(m.setBondOrderValue(b1, 2), "setBondOrderValue succeeds");
    CHECK(m.bondOrder(b1) == 2, "bondOrder reads back the new order");

    AtomId invalid = 9999;
    CHECK(!m.setAtomLabel(invalid, QStringLiteral("O")), "invalid atom id: setAtomLabel fails cleanly");
    CHECK(!m.setBondOrderValue(invalid, 1), "invalid bond id: setBondOrderValue fails cleanly");
}

static void test_attachmentPointAccessors() {
    std::printf("--- Test 10: attachment-point accessors ---\n");
    EditableMolecule m;
    AtomId c1 = m.addAtom(QStringLiteral("C"), 0, 0);
    AtomId c2 = m.addAtom(QStringLiteral("C"), 1, 0);

    CHECK(m.atomAttachmentOrder(c1) == 0, "fresh atom has attachment order 0 (none)");
    CHECK(m.setAtomAttachmentOrder(c1, 1), "setAtomAttachmentOrder(1) succeeds");
    CHECK(m.atomAttachmentOrder(c1) == 1, "atomAttachmentOrder reads back order 1");

    CHECK(m.setAtomAttachmentOrder(c2, 2), "setAtomAttachmentOrder(2) on a different atom succeeds");
    CHECK(m.atomAttachmentOrder(c2) == 2, "second atom reports its own order 2");
    CHECK(m.atomAttachmentOrder(c1) == 1, "first atom's order unaffected by the second set");

    CHECK(m.setAtomAttachmentOrder(c1, 0), "setAtomAttachmentOrder(0) clears it");
    CHECK(m.atomAttachmentOrder(c1) == 0, "atomAttachmentOrder reports 0 after clearing");

    AtomId invalid = 9999;
    CHECK(!m.setAtomAttachmentOrder(invalid, 1), "invalid id: setAtomAttachmentOrder fails cleanly");
    CHECK(m.atomAttachmentOrder(invalid) == 0, "invalid id: atomAttachmentOrder returns sentinel 0");
}

static void test_sgroupMembershipSpike() {
    std::printf("--- Test 11: sgroup-atom-membership spike (3a design question) ---\n");
    unsigned long long session = indigoAllocSessionId();
    indigoSetSessionId(session);

    int mol = indigoLoadMoleculeFromString("CCO");
    CHECK(mol >= 0, "spike: load CCO for sgroup spike");

    int atomIndices[2] = { 0, 1 };
    int sg = indigoAddDataSGroup(mol, 2, atomIndices, 0, nullptr, "F", "d");
    CHECK(sg >= 0, "spike: data sgroup created on the raw handle");

    // Does indigoIterateAtoms work when called on the SGROUP handle itself
    // (as opposed to the molecule handle)? indigo.h's own doc comment says
    // "for all atoms of the given molecule" -- test empirically anyway.
    int sgAtomIter = indigoIterateAtoms(sg);
    int sgAtomCount = 0;
    if (sgAtomIter >= 0) {
        int h;
        while ((h = indigoNext(sgAtomIter)) > 0) { ++sgAtomCount; indigoFree(h); }
        indigoFree(sgAtomIter);
    }
    std::printf("[INFO] spike: indigoIterateAtoms(sgroupHandle) iterator=%s, atom count=%d\n",
                sgAtomIter >= 0 ? "valid" : "invalid(-1)", sgAtomCount);

    // Does the generic indigoRemove(item) delete a WHOLE sgroup (its atoms and
    // bonds too), or just detach the sgroup record leaving atoms untouched, or
    // fail outright? Order matters: check counts before/after.
    int countBefore = indigoCountAtoms(mol);
    int removeResult = indigoRemove(sg);
    int countAfter = indigoCountAtoms(mol);
    std::printf("[INFO] spike: indigoRemove(sgroupHandle) returned %d; atom count before=%d after=%d\n",
                removeResult, countBefore, countAfter);

    CHECK(true, "spike: sgroup-membership investigation completed (see [INFO] lines above)");

    indigoFree(mol);
    indigoReleaseSessionId(session);
}

static void test_bondStereoSpike() {
    std::printf("--- Test 12: bond-stereo mechanism spike (3a design question) ---\n");
    unsigned long long session = indigoAllocSessionId();
    indigoSetSessionId(session);

    // Alanine (CC(N)C(=O)O) has a real stereocenter at atom index 1 -- used to
    // test whether forcing a specific tetrahedral arrangement via
    // indigoAddStereocenter, then indigoMarkStereobonds, causes any incident
    // bond's (read-only) indigoBondStereo() to report a nonzero wedge/dash.
    int mol = indigoLoadMoleculeFromString("CC(N)C(=O)O");
    CHECK(mol >= 0, "spike: load alanine for bond-stereo spike");

    int centerAtom = indigoGetAtom(mol, 1);
    CHECK(centerAtom >= 0, "spike: got the stereocenter atom handle");
    int beforeType = indigoStereocenterType(centerAtom);
    std::printf("[INFO] spike: stereocenter type before indigoAddStereocenter: %d\n", beforeType);

    QList<int> neiIdx;
    int neiIter = indigoIterateNeighbors(centerAtom);
    if (neiIter >= 0) {
        int h;
        while ((h = indigoNext(neiIter)) > 0) { neiIdx.append(indigoIndex(h)); indigoFree(h); }
        indigoFree(neiIter);
    }
    int v1 = neiIdx.size() > 0 ? neiIdx[0] : -1;
    int v2 = neiIdx.size() > 1 ? neiIdx[1] : -1;
    int v3 = neiIdx.size() > 2 ? neiIdx[2] : -1;
    int setResult = indigoAddStereocenter(centerAtom, INDIGO_ABS, v1, v2, v3, -1);
    std::printf("[INFO] spike: indigoAddStereocenter(ABS) returned %d\n", setResult);

    int markResult = indigoMarkStereobonds(mol);
    std::printf("[INFO] spike: indigoMarkStereobonds returned %d\n", markResult);

    int bondIter = indigoIterateBonds(mol);
    int stereoBondsFound = 0;
    if (bondIter >= 0) {
        int b;
        while ((b = indigoNext(bondIter)) > 0) {
            if (indigoBondStereo(b) != 0) ++stereoBondsFound;
            indigoFree(b);
        }
        indigoFree(bondIter);
    }
    std::printf("[INFO] spike: bonds with nonzero indigoBondStereo() after marking: %d\n", stereoBondsFound);

    CHECK(true, "spike: bond-stereo mechanism investigation completed (see [INFO] lines above)");

    indigoFree(centerAtom);
    indigoFree(mol);
    indigoReleaseSessionId(session);
}

static void test_atomQueryList() {
    std::printf("--- Test 13: query atom list sidecar ---\n");
    EditableMolecule m;
    AtomId c1 = m.addAtom(QStringLiteral("C"), 0, 0);

    CHECK(!m.hasAtomQueryList(c1), "fresh atom has no query list");

    CHECK(m.setAtomQueryList(c1, QStringLiteral("C, N , O"), false), "setAtomQueryList succeeds");
    CHECK(m.hasAtomQueryList(c1), "hasAtomQueryList true after setting");
    CHECK(m.atomSymbol(c1) == QStringLiteral("L#"), "atomSymbol becomes the L# sentinel");
    QList<int> nums = m.atomQueryListNumbers(c1);
    CHECK(nums.size() == 3, "3 distinct elements parsed from the CSV (C, N, O)");
    CHECK(nums.contains(6) && nums.contains(7) && nums.contains(8),
          "parsed atomic numbers are exactly carbon/nitrogen/oxygen");
    CHECK(!m.atomQueryListIsNotList(c1), "notList flag is false");

    CHECK(m.setAtomQueryList(c1, QStringLiteral("F,Cl"), true), "re-setting the query list succeeds");
    CHECK(m.atomQueryListIsNotList(c1), "notList flag is true after re-setting with notList=true");
    CHECK(m.atomQueryListNumbers(c1).size() == 2, "2 elements after re-setting");

    CHECK(m.clearAtomQueryList(c1, QStringLiteral("N")), "clearAtomQueryList succeeds");
    CHECK(!m.hasAtomQueryList(c1), "hasAtomQueryList false after clearing");
    CHECK(m.atomSymbol(c1) == QStringLiteral("N"), "atomSymbol reset to the fallback label");

    CHECK(!m.setAtomQueryList(c1, QStringLiteral("NotAnElement,AlsoNot"), false),
          "setAtomQueryList with zero valid labels fails cleanly (matches _makeAtomList returning null)");
    CHECK(!m.hasAtomQueryList(c1), "no query list was set from all-invalid input");

    AtomId invalid = 9999;
    CHECK(!m.setAtomQueryList(invalid, QStringLiteral("C"), false), "invalid id: setAtomQueryList fails cleanly");
    CHECK(!m.clearAtomQueryList(invalid, QStringLiteral("C")), "invalid id: clearAtomQueryList fails cleanly");
}

static void test_bondStereoInvertSpike() {
    std::printf("--- Test 14: indigoInvertStereo-on-a-bond spike (3b design question) ---\n");
    unsigned long long session = indigoAllocSessionId();
    indigoSetSessionId(session);

    // A molfile with an explicit wedge bond (stereo code 1) -- SMILES cannot
    // carry wedge/dash, and sub-project 3a's Test 12 proved a coordinate-less
    // molecule never gets stereobonds marked, so this spike MUST start from a
    // molfile that already has both 2D coordinates and an explicit wedge flag.
    // Bond block column 4 is the stereo code: "1" = wedge up.
    const char* molfile =
        "\n  spike\n\n"
        "  4  3  0  0  0  0  0  0  0  0999 V2000\n"
        "    0.0000    0.0000    0.0000 C   0  0\n"
        "    1.0000    0.0000    0.0000 N   0  0\n"
        "    0.0000    1.0000    0.0000 O   0  0\n"
        "   -1.0000    0.0000    0.0000 F   0  0\n"
        "  1  2  1  1  0  0  0\n"
        "  1  3  1  0  0  0  0\n"
        "  1  4  1  0  0  0  0\n"
        "M  END\n";

    int mol = indigoLoadMoleculeFromString(molfile);
    CHECK(mol >= 0, "spike: molfile with an explicit wedge bond loads");
    if (mol < 0) {
        std::printf("[INFO] spike: load error: %s\n", indigoGetLastError());
        indigoReleaseSessionId(session);
        return;
    }

    int bond0 = indigoGetBond(mol, 0);
    CHECK(bond0 >= 0, "spike: got bond 0 handle");
    int stereoBefore = indigoBondStereo(bond0);
    std::printf("[INFO] spike: indigoBondStereo(bond0) as loaded = %d\n", stereoBefore);

    // Q1: does indigoInvertStereo accept a BOND handle at all?
    int invert1 = indigoInvertStereo(bond0);
    std::printf("[INFO] spike: indigoInvertStereo(bond0) returned %d%s\n",
                invert1, invert1 < 0 ? " (REJECTED)" : "");
    if (invert1 < 0) std::printf("[INFO] spike: invert error: %s\n", indigoGetLastError());

    // Q2: did the reported stereo actually change, and to what?
    indigoFree(bond0);
    bond0 = indigoGetBond(mol, 0);
    int stereoAfter1 = indigoBondStereo(bond0);
    std::printf("[INFO] spike: indigoBondStereo(bond0) after 1 invert = %d\n", stereoAfter1);

    // Q3: is it its own inverse? (the EditCommand's invert() calls it again)
    indigoInvertStereo(bond0);
    indigoFree(bond0);
    bond0 = indigoGetBond(mol, 0);
    int stereoAfter2 = indigoBondStereo(bond0);
    std::printf("[INFO] spike: indigoBondStereo(bond0) after 2 inverts = %d (round-trip %s)\n",
                stereoAfter2, stereoAfter2 == stereoBefore ? "OK" : "BROKEN");

    CHECK(true, "spike: bond-stereo invert investigation completed (see [INFO] lines above)");

    indigoFree(bond0);
    indigoFree(mol);
    indigoReleaseSessionId(session);
}

static void test_positionAccessors() {
    std::printf("--- Test 15: position accessors for every repositionable entity ---\n");
    EditableMolecule m;

    // Atom
    AtomId a1 = m.addAtom(QStringLiteral("C"), 1.5, 2.5);
    double ax = 0, ay = 0;
    CHECK(m.atomPos(a1, ax, ay) && ax == 1.5 && ay == 2.5, "atom starts at its add-time position");
    CHECK(m.setAtomPos(a1, -3.25, 4.75), "setAtomPos succeeds");
    CHECK(m.atomPos(a1, ax, ay) && ax == -3.25 && ay == 4.75, "setAtomPos round-trips");

    // RxnArrow
    RxnArrowId ar = m.addRxnArrow(0, 0, 5, 0);
    double x1 = 0, y1 = 0, x2 = 0, y2 = 0;
    CHECK(m.rxnArrowEndpoints(ar, x1, y1, x2, y2) && x1 == 0 && y1 == 0 && x2 == 5 && y2 == 0,
          "rxnArrowEndpoints reads the add-time endpoints");
    CHECK(m.setRxnArrowEndpoints(ar, 1, 2, 3, 4), "setRxnArrowEndpoints succeeds");
    CHECK(m.rxnArrowEndpoints(ar, x1, y1, x2, y2) && x1 == 1 && y1 == 2 && x2 == 3 && y2 == 4,
          "setRxnArrowEndpoints round-trips");

    // RxnPlus
    RxnPlusId pl = m.addRxnPlus(2.5, 0);
    double px = 0, py = 0;
    CHECK(m.rxnPlusPos(pl, px, py) && px == 2.5 && py == 0, "rxnPlusPos reads the add-time position");
    CHECK(m.setRxnPlusPos(pl, -1.5, 6.5), "setRxnPlusPos succeeds");
    CHECK(m.rxnPlusPos(pl, px, py) && px == -1.5 && py == 6.5, "setRxnPlusPos round-trips");

    // MultitailArrow
    MultitailArrowId mta = m.addMultitailArrow({5, 5, 0, 4, 0, 6});
    QList<double> pts = m.multitailArrowPoints(mta);
    CHECK(pts.size() == 6 && pts[0] == 5 && pts[5] == 6, "multitailArrowPoints reads the add-time points");
    CHECK(m.setMultitailArrowPoints(mta, {1, 2, 3, 4}), "setMultitailArrowPoints succeeds");
    pts = m.multitailArrowPoints(mta);
    CHECK(pts.size() == 4 && pts[0] == 1 && pts[3] == 4, "setMultitailArrowPoints round-trips");

    // Image
    ImageId img = m.addImage(0, 0, 4, 3, QByteArray("fakepng"));
    double ix = 0, iy = 0, iw = 0, ih = 0;
    CHECK(m.imageRect(img, ix, iy, iw, ih) && ix == 0 && iy == 0 && iw == 4 && ih == 3,
          "imageRect reads the add-time rect");
    CHECK(m.setImageRect(img, 1, 2, 8, 6), "setImageRect succeeds");
    CHECK(m.imageRect(img, ix, iy, iw, ih) && ix == 1 && iy == 2 && iw == 8 && ih == 6,
          "setImageRect round-trips");

    // Invalid ids fail cleanly.
    const int bad = 9999;
    CHECK(!m.setAtomPos(bad, 0, 0), "invalid id: setAtomPos fails cleanly");
    CHECK(!m.atomPos(bad, ax, ay), "invalid id: atomPos fails cleanly");
    CHECK(!m.rxnArrowEndpoints(bad, x1, y1, x2, y2), "invalid id: rxnArrowEndpoints fails cleanly");
    CHECK(!m.setRxnArrowEndpoints(bad, 0, 0, 0, 0), "invalid id: setRxnArrowEndpoints fails cleanly");
    CHECK(!m.rxnPlusPos(bad, px, py), "invalid id: rxnPlusPos fails cleanly");
    CHECK(!m.setRxnPlusPos(bad, 0, 0), "invalid id: setRxnPlusPos fails cleanly");
    CHECK(m.multitailArrowPoints(bad).isEmpty(), "invalid id: multitailArrowPoints returns empty");
    CHECK(!m.setMultitailArrowPoints(bad, {1, 2}), "invalid id: setMultitailArrowPoints fails cleanly");
    CHECK(!m.imageRect(bad, ix, iy, iw, ih), "invalid id: imageRect fails cleanly");
    CHECK(!m.setImageRect(bad, 0, 0, 1, 1), "invalid id: setImageRect fails cleanly");
}

int main() {
    unsigned long long session = indigoAllocSessionId();
    indigoSetSessionId(session);

    spike_idBehavior();
    test_constructionAndMolfile();
    test_atomBondCrud();
    test_extensionData();
    test_sgroupPassthrough();
    test_snapshotRestore();
    test_queryFeatureSpike();
    test_extensionEntityRemoval();
    test_atomAttributeAccessors();
    test_atomLabelAndBondOrder();
    test_attachmentPointAccessors();
    test_sgroupMembershipSpike();
    test_bondStereoSpike();
    test_atomQueryList();
    test_bondStereoInvertSpike();
    test_positionAccessors();

    indigoReleaseSessionId(session);
    std::printf("Summary: %d passed, %d failed.\n", g_pass, g_fail);
    return g_fail == 0 ? 0 : 1;
}
