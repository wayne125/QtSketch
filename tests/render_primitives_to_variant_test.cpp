// tests/render_primitives_to_variant_test.cpp
// Standalone tests for RenderPrimitivesToVariant (chem-core.js migration, sub-project 7a).
#include <cstdio>
#include <QVariantMap>
#include <QVariantList>
#include "app/molecule/RenderPrimitivesToVariant.h"
#include "app/molecule/SelectionState.h"
#include "app/molecule/EditableMolecule.h"
#include "app/molecule/RenderPrimitives.h"

static int g_pass = 0, g_fail = 0;
#define CHECK(cond, name) do { \
    if (cond) { ++g_pass; std::printf("[PASS] %s\n", name); } \
    else { ++g_fail; std::printf("[FAIL] %s (line %d)\n", name, __LINE__); } \
} while (0)

static void test_selectionStateToVariantEmpty() {
    std::printf("--- Test: selectionStateToVariant, empty selection ---\n");
    SelectionState sel;
    QVariantMap m = selectionStateToVariant(sel);
    CHECK(m.value("atom_ids").toList().isEmpty(), "atom_ids empty");
    CHECK(m.value("bond_ids").toList().isEmpty(), "bond_ids empty");
    CHECK(m.value("rxnArrow_ids").toList().isEmpty(), "rxnArrow_ids empty");
    CHECK(m.value("rxnPlus_ids").toList().isEmpty(), "rxnPlus_ids empty");
    CHECK(m.value("multitailArrow_ids").toList().isEmpty(), "multitailArrow_ids empty");
    CHECK(m.value("bbox").isNull(), "bbox is null");
}

static void test_selectionStateToVariantMixed() {
    std::printf("--- Test: selectionStateToVariant, mixed selection ---\n");
    SelectionState sel;
    sel.atoms.insert(1);
    sel.atoms.insert(2);
    sel.bonds.insert(5);
    sel.rxnArrows.insert(9);
    sel.rxnPluses.insert(3);
    sel.multitailArrows.insert(7);

    QVariantMap m = selectionStateToVariant(sel);
    QVariantList atomIds = m.value("atom_ids").toList();
    CHECK(atomIds.size() == 2, "2 atom ids");
    CHECK(atomIds.contains(1) && atomIds.contains(2), "correct atom ids present");
    CHECK(m.value("bond_ids").toList() == QVariantList{5}, "1 bond id");
    CHECK(m.value("rxnArrow_ids").toList() == QVariantList{9}, "1 rxnArrow id");
    CHECK(m.value("rxnPlus_ids").toList() == QVariantList{3}, "1 rxnPlus id");
    CHECK(m.value("multitailArrow_ids").toList() == QVariantList{7}, "1 multitailArrow id");
    CHECK(m.value("bbox").isNull(), "bbox still null (always null in every real code path)");
}

static void test_renderPrimitivesToVariantPlainMolecule() {
    std::printf("--- Test: renderPrimitivesToVariant, plain molecule (atom/bond field completeness) ---\n");
    EditableMolecule mol;
    AtomId a1 = mol.addAtom(QStringLiteral("C"), 0.0, 0.0);
    AtomId a2 = mol.addAtom(QStringLiteral("N"), 1.0, 0.0);
    BondId b1 = mol.addBond(a1, a2, 1);
    (void)b1;

    RenderPrimitives prims = RenderPrimitiveBuilder::build(mol, false);
    QVariantMap v = renderPrimitivesToVariant(prims);

    QVariantList atoms = v.value("atoms").toList();
    CHECK(atoms.size() == 2, "2 atoms");
    QVariantMap atom0 = atoms[0].toMap();
    CHECK(atom0.contains("id") && atom0.contains("x") && atom0.contains("y")
          && atom0.contains("label") && atom0.contains("element") && atom0.contains("charge")
          && atom0.contains("stereoLabel") && atom0.contains("cipLabel")
          && atom0.contains("stereoType") && atom0.contains("stereoGroup")
          && atom0.contains("color") && atom0.contains("isRGroup") && atom0.contains("atomicNum")
          && atom0.contains("atomicTitle") && atom0.contains("atomicMass")
          && atom0.contains("implicitHCount") && atom0.contains("hOnLeft")
          && atom0.contains("isotope") && atom0.contains("radical")
          && atom0.contains("explicitValence") && atom0.contains("aam")
          && atom0.contains("attachmentPoints") && atom0.contains("checkWarning")
          && atom0.contains("isAtomList") && atom0.contains("atomListElements")
          && atom0.contains("atomListNot"),
          "atom QVariantMap has every real-JSON field");
    CHECK(atom0.value("isSgroup").toBool() == false,
          "regular atom has isSgroup:false present (documented harmless difference from real JSON's key-absent)");

    QVariantList bonds = v.value("bonds").toList();
    CHECK(bonds.size() == 1, "1 bond");
    QVariantMap bond0 = bonds[0].toMap();
    CHECK(bond0.contains("id") && bond0.contains("begin") && bond0.contains("end")
          && bond0.contains("type") && bond0.contains("stereo") && bond0.contains("inAromaticRing")
          && bond0.contains("invalidStereo") && bond0.contains("checkWarning")
          && bond0.contains("beginIsSgroup") && bond0.contains("endIsSgroup")
          && bond0.contains("cipLabel") && bond0.contains("reactingCenterStatus"),
          "bond QVariantMap has every real-JSON field");
    CHECK(!bond0.contains("ringCenterX") && !bond0.contains("ringCenterY"),
          "a bond with no ring omits ringCenterX/Y keys entirely, matching real JSON's undefined-key-drop");

    QVariantMap atomsById = v.value("atomsById").toMap();
    CHECK(atomsById.size() == 2, "atomsById has 2 entries");
    CHECK(atomsById.contains(QString::number(a1)) && atomsById.contains(QString::number(a2)),
          "atomsById keyed by stringified atom id");

    QVariantMap bbox = v.value("bbox").toMap();
    CHECK(bbox.contains("minX") && bbox.contains("minY") && bbox.contains("maxX") && bbox.contains("maxY"),
          "bbox present with all four fields for a non-empty molecule");

    QVariantMap stereoFlags = v.value("stereoFlags").toMap();
    CHECK(stereoFlags.contains("type") && stereoFlags.contains("groupId"), "stereoFlags present");

    CHECK(v.value("rings").toList().isEmpty(), "no rings");
    CHECK(v.value("sgroups").toList().isEmpty(), "no sgroups");
    CHECK(v.value("texts").toList().isEmpty(), "no texts");
    CHECK(v.value("images").toList().isEmpty(), "no images");
    CHECK(v.value("rxnArrows").toList().isEmpty(), "no rxnArrows");
    CHECK(v.value("rxnPluses").toList().isEmpty(), "no rxnPluses");
    CHECK(v.value("multitailArrows").toList().isEmpty(), "no multitailArrows");
    CHECK(v.value("rgroups").toList().isEmpty(), "no rgroups");
    CHECK(v.value("brackets").toList().isEmpty(), "no brackets");
}

static void test_renderPrimitivesToVariantContractedSgroup() {
    std::printf("--- Test: renderPrimitivesToVariant, contracted sgroup pseudo-atom injection ---\n");
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
    EditableMolecule mol(molfile);
    CHECK(mol.sgroupIds().size() == 1, "setup: 1 sgroup");
    SGroupId sid = mol.sgroupIds().first();
    CHECK(mol.sgroupExpanded(sid), "setup: sgroup starts expanded (default)");
    mol.setSGroupExpanded(sid, false); // contract it

    RenderPrimitives prims = RenderPrimitiveBuilder::build(mol, false);
    QVariantMap v = renderPrimitivesToVariant(prims);

    QVariantList atoms = v.value("atoms").toList();
    bool foundPseudoAtom = false;
    for (const QVariant& av : atoms) {
        QVariantMap am = av.toMap();
        if (am.value("isSgroup").toBool()) {
            foundPseudoAtom = true;
            // label is always empty here, NOT the molfile's "Ac" -- a pre-existing, already-
            // documented sub-project-4 gap (RenderPrimitives.cpp: "this port has no stored
            // sgroup label anywhere"), independently reconfirmed by sub-project 6a's own probe.
            CHECK(am.value("label").toString().isEmpty(), "pseudo-atom label is empty (documented gap, not a regression)");
            CHECK(am.contains("attachAtomId"), "pseudo-atom has attachAtomId");
        }
    }
    CHECK(foundPseudoAtom, "a contracted-sgroup pseudo-atom is present in the atoms list");

    QVariantList sgroups = v.value("sgroups").toList();
    CHECK(sgroups.size() == 1, "1 entry in the separate sgroups array too");

    QVariantMap atomsById = v.value("atomsById").toMap();
    CHECK(atomsById.contains(QString::number(sid)), "atomsById also keyed by the sgroup's own id");
}

static void test_renderPrimitivesToVariantRingCenterPresentForAromaticRing() {
    std::printf("--- Test: renderPrimitivesToVariant, aromatic ring bonds carry ring-center fields ---\n");
    EditableMolecule mol;
    QList<AtomId> ring = mol.addBenzeneRing(0.0, 0.0);
    CHECK(ring.size() == 6, "setup: benzene ring has 6 atoms");

    RenderPrimitives prims = RenderPrimitiveBuilder::build(mol, false);
    QVariantMap v = renderPrimitivesToVariant(prims);

    QVariantList bonds = v.value("bonds").toList();
    CHECK(bonds.size() == 6, "6 ring bonds");
    bool anyHasRingCenter = false;
    for (const QVariant& bv : bonds) {
        QVariantMap bm = bv.toMap();
        if (bm.contains("ringCenterX") && bm.contains("ringCenterY")) anyHasRingCenter = true;
    }
    CHECK(anyHasRingCenter, "at least one ring bond carries ringCenterX/Y keys");
    CHECK(!v.value("rings").toList().isEmpty(), "rings array is non-empty");
}

static void test_renderPrimitivesToVariantNonEntityPrims() {
    std::printf("--- Test: renderPrimitivesToVariant, text/image/rxnArrow/rxnPlus/multitailArrow ---\n");
    EditableMolecule mol;
    mol.addTextAnnotation(0.0, 0.0, QStringLiteral("hello"));
    mol.addImage(0.0, 0.0, 1.0, 1.0, QByteArray("data:image/png;base64,AAAA"));
    int arrowNoCurvature = mol.addRxnArrow(0.0, 0.0, 5.0, 0.0);
    int arrowWithCurvature = mol.addRxnArrow(0.0, 5.0, 5.0, 5.0);
    mol.setRxnArrowCurvature(arrowWithCurvature, 2.5, 6.0, true);
    (void)arrowNoCurvature;
    mol.addRxnPlus(0.0, -5.0);
    mol.addMultitailArrow({0.0, -10.0});

    RenderPrimitives prims = RenderPrimitiveBuilder::build(mol, false);
    QVariantMap v = renderPrimitivesToVariant(prims);

    QVariantList texts = v.value("texts").toList();
    CHECK(texts.size() == 1, "1 text");
    CHECK(texts[0].toMap().value("content").toString() == QStringLiteral("hello"),
          "text content converts as plain QVariant(QString), no JSON-unwrapping");

    QVariantList images = v.value("images").toList();
    CHECK(images.size() == 1, "1 image");
    QVariant bitmapVal = images[0].toMap().value("bitmap");
    CHECK(bitmapVal.typeId() == QMetaType::QString,
          "image.bitmap converts to an actual QString, not a QByteArray");
    CHECK(bitmapVal.toString() == QStringLiteral("data:image/png;base64,AAAA"),
          "image.bitmap round-trips the exact data-URI text");

    QVariantList rxnArrows = v.value("rxnArrows").toList();
    CHECK(rxnArrows.size() == 2, "2 rxnArrows");
    bool sawNullCurvature = false, sawRealCurvature = false;
    for (const QVariant& av : rxnArrows) {
        QVariantMap am = av.toMap();
        CHECK(am.contains("curvature"), "curvature key always present (never omitted, unlike bond ring-center)");
        if (am.value("curvature").isNull()) sawNullCurvature = true;
        else {
            sawRealCurvature = true;
            QVariantMap curv = am.value("curvature").toMap();
            CHECK(curv.value("x").toDouble() == 2.5 && curv.value("y").toDouble() == 6.0,
                  "the arrow WITH curvature carries the correct x/y");
        }
    }
    CHECK(sawNullCurvature, "the arrow with no curvature has curvature: null");
    CHECK(sawRealCurvature, "the arrow with curvature has a real {x,y} map");

    CHECK(v.value("rxnPluses").toList().size() == 1, "1 rxnPlus");
    CHECK(v.value("multitailArrows").toList().size() == 1, "1 multitailArrow");
    QVariantMap mta = v.value("multitailArrows").toList()[0].toMap();
    // addMultitailArrow({headX, headY}) intentionally starts with ZERO tails (this port's own
    // established convention, DocumentState.h: "addMultitailArrow starts with head only, zero
    // tails") -- the "tails" key must still be PRESENT (an empty list), not absent.
    CHECK(mta.contains("tails") && mta.value("tails").toList().isEmpty(),
          "multitailArrow's tails key is present and correctly empty for a head-only arrow");
}

static void test_renderPrimitivesToVariantRGroup() {
    std::printf("--- Test: renderPrimitivesToVariant, rgroup member/fragId nesting ---\n");
    EditableMolecule mol;
    AtomId a = mol.addAtom(QStringLiteral("C"), 0.0, 0.0);
    (void)a;
    CHECK(mol.addRGroupEntry(1), "setup: R-group 1 added");
    mol.addRGroupFragment(1, mol.atomFragmentIndex(a));

    RenderPrimitives prims = RenderPrimitiveBuilder::build(mol, false);
    QVariantMap v = renderPrimitivesToVariant(prims);

    QVariantList rgroups = v.value("rgroups").toList();
    CHECK(rgroups.size() == 1, "1 rgroup entry");
    QVariantMap rg0 = rgroups[0].toMap();
    CHECK(rg0.value("number").toInt() == 1, "rgroup number is 1");
    CHECK(rg0.contains("range") && rg0.contains("resth") && rg0.contains("ifthen"), "rgroup logic fields present");
    QVariantList members = rg0.value("members").toList();
    CHECK(!members.isEmpty(), "rgroup has at least one member fragment");
    CHECK(members[0].toMap().contains("fragId") && members[0].toMap().contains("atomIds"),
          "member has fragId and atomIds");
}

int main() {
    test_selectionStateToVariantEmpty();
    test_selectionStateToVariantMixed();
    test_renderPrimitivesToVariantPlainMolecule();
    test_renderPrimitivesToVariantContractedSgroup();
    test_renderPrimitivesToVariantRingCenterPresentForAromaticRing();
    test_renderPrimitivesToVariantNonEntityPrims();
    test_renderPrimitivesToVariantRGroup();
    std::printf("Summary: %d passed, %d failed.\n", g_pass, g_fail);
    return g_fail == 0 ? 0 : 1;
}
