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

int main() {
    unsigned long long session = indigoAllocSessionId();
    indigoSetSessionId(session);

    spike_idBehavior();
    test_constructionAndMolfile();
    test_atomBondCrud();
    test_extensionData();
    test_sgroupPassthrough();
    test_snapshotRestore();

    indigoReleaseSessionId(session);
    std::printf("Summary: %d passed, %d failed.\n", g_pass, g_fail);
    return g_fail == 0 ? 0 : 1;
}
