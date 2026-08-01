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

int main() {
    unsigned long long session = indigoAllocSessionId();
    indigoSetSessionId(session);

    spike_idBehavior();
    test_constructionAndMolfile();

    indigoReleaseSessionId(session);
    std::printf("Summary: %d passed, %d failed.\n", g_pass, g_fail);
    return g_fail == 0 ? 0 : 1;
}
