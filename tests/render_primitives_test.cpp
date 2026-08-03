// tests/render_primitives_test.cpp
// Standalone tests for RenderPrimitiveBuilder (chem-core.js migration, sub-project 4).
#include <cstdio>
#include "app/molecule/EditableMolecule.h"
#include "app/molecule/ElementData.h"
#include "app/molecule/RenderPrimitives.h"
#include "indigo.h"

static int g_pass = 0, g_fail = 0;
#define CHECK(cond, name) do { \
    if (cond) { ++g_pass; std::printf("[PASS] %s\n", name); } \
    else { ++g_fail; std::printf("[FAIL] %s (line %d)\n", name, __LINE__); } \
} while (0)

static void test_elementData() {
    std::printf("--- Test 1: ElementData lookup ---\n");
    CHECK(ElementData::colorFor(QStringLiteral("C")) == QStringLiteral("#000000"), "carbon color matches source");
    CHECK(ElementData::colorFor(QStringLiteral("O")) == QStringLiteral("#ff0d0d"), "oxygen color matches source");
    CHECK(ElementData::colorFor(QStringLiteral("Fe")) == QStringLiteral("#e06633"), "iron color matches source");
    CHECK(ElementData::colorFor(QStringLiteral("Au")) == QStringLiteral("#c19e1c"), "gold color matches source");
    CHECK(ElementData::colorFor(QStringLiteral("Zzz")).isEmpty(), "unknown symbol color is empty");

    const ElementData::ElementInfo* c = ElementData::infoFor(QStringLiteral("C"));
    CHECK(c != nullptr, "carbon info resolves");
    if (c) {
        CHECK(c->number == 6, "carbon atomic number is 6");
        CHECK(c->title == QStringLiteral("Carbon"), "carbon title matches source");
        CHECK(c->mass > 12.0 && c->mass < 12.1, "carbon mass matches source (~12.011)");
    }

    const ElementData::ElementInfo* au = ElementData::infoFor(QStringLiteral("Au"));
    CHECK(au != nullptr, "gold info resolves");
    if (au) {
        CHECK(au->number == 79, "gold atomic number is 79");
        CHECK(au->title == QStringLiteral("Gold"), "gold title matches source");
    }

    CHECK(ElementData::infoFor(QStringLiteral("Zzz")) == nullptr, "unknown symbol info is nullptr");
}

static void test_atomPrimitivesAndSgroupContraction() {
    std::printf("--- Test 2: atom primitives + sgroup contraction ---\n");

    // Implicit-H display: heteroatoms/terminal atoms always show H; carbons only with showExplicitH.
    {
        EditableMolecule m(QStringLiteral("CCO"));   // ethanol: terminal C, middle C, O
        QList<AtomId> ids = m.atomIds();

        RenderPrimitives noExplicit = RenderPrimitiveBuilder::build(m, false);
        CHECK(noExplicit.atoms.size() == 3, "ethanol produces 3 atom primitives");
        AtomPrim* terminalC = nullptr; AtomPrim* middleC = nullptr; AtomPrim* oxy = nullptr;
        for (AtomPrim& a : noExplicit.atoms) {
            if (a.id == ids[0]) terminalC = &a;
            else if (a.id == ids[1]) middleC = &a;
            else if (a.id == ids[2]) oxy = &a;
        }
        CHECK(terminalC && terminalC->label == QStringLiteral("CH3"),
              "terminal carbon shows its 3 implicit H even without showExplicitH (terminal rule)");
        CHECK(middleC && middleC->label == QStringLiteral("C"),
              "non-terminal carbon shows NO implicit H without showExplicitH");
        CHECK(oxy && oxy->label == QStringLiteral("OH"),
              "oxygen (heteroatom) shows its implicit H even without showExplicitH");

        RenderPrimitives withExplicit = RenderPrimitiveBuilder::build(m, true);
        AtomPrim* middleC2 = nullptr;
        for (AtomPrim& a : withExplicit.atoms) if (a.id == ids[1]) middleC2 = &a;
        CHECK(middleC2 && middleC2->label == QStringLiteral("CH2"),
              "non-terminal carbon shows implicit H when showExplicitH is true");
    }

    // Element color/number/title/mass sourced from ElementData.
    {
        EditableMolecule m(QStringLiteral("O"));
        RenderPrimitives rp = RenderPrimitiveBuilder::build(m, false);
        CHECK(rp.atoms.size() == 1, "single-atom molecule produces 1 atom primitive");
        const AtomPrim& o = rp.atoms[0];
        CHECK(o.element == QStringLiteral("O"), "element field is the plain symbol");
        CHECK(o.color == QStringLiteral("#ff0d0d"), "color sourced from ElementData matches source");
        CHECK(o.atomicNum == 8, "atomicNum is 8 for oxygen");
        CHECK(o.atomicTitle == QStringLiteral("Oxygen"), "atomicTitle sourced from ElementData");
    }

    // Atom-list (query-list) label formatting: sub-project 3a's setAtomQueryList produces an
    // "L#" atom carrying a query-list sidecar; the render primitive must format it as
    // "[Sym1,Sym2]" (or "![...]" for a NOT-list), using ElementData::symbolForNumber to map the
    // stored atomic numbers back to symbols.
    {
        EditableMolecule m(QStringLiteral("C"));
        AtomId id = m.atomIds()[0];
        CHECK(m.setAtomQueryList(id, QStringLiteral("Cl,Br"), false), "setup: setAtomQueryList succeeds");

        RenderPrimitives rp = RenderPrimitiveBuilder::build(m, false);
        CHECK(rp.atoms.size() == 1, "one atom primitive");
        const AtomPrim& a = rp.atoms[0];
        CHECK(a.isAtomList, "isAtomList is true for a query-list atom");
        CHECK(!a.atomListNot, "atomListNot is false (not a NOT-list)");
        CHECK(a.atomListElements == QStringLiteral("Cl,Br"), "atomListElements resolves the real symbols");
        CHECK(a.label == QStringLiteral("[Cl,Br]"), "label is formatted as \"[Cl,Br]\"");

        CHECK(m.setAtomQueryList(id, QStringLiteral("F,I"), true), "setup: re-set as a NOT-list");
        RenderPrimitives rp2 = RenderPrimitiveBuilder::build(m, false);
        CHECK(rp2.atoms[0].atomListNot, "atomListNot is true for a NOT-list");
        CHECK(rp2.atoms[0].label == QStringLiteral("![F,I]"), "label is formatted as \"![F,I]\" for a NOT-list");
    }

    // isRGroup: an atom labeled "R1" is flagged as an r-group placeholder and colored distinctly.
    {
        EditableMolecule m;
        m.addAtom(QStringLiteral("R1"), 0.0, 0.0);
        RenderPrimitives rp = RenderPrimitiveBuilder::build(m, false);
        CHECK(rp.atoms.size() == 1, "one atom primitive");
        CHECK(rp.atoms[0].isRGroup, "an \"R1\"-labeled atom is flagged isRGroup");
        CHECK(rp.atoms[0].color == QStringLiteral("#7B68EE"), "an r-group atom gets the distinct r-group color");
    }
    {
        EditableMolecule m;
        m.addAtom(QStringLiteral("R9"), 0.0, 0.0);   // out of the R1-R8 range
        RenderPrimitives rp = RenderPrimitiveBuilder::build(m, false);
        CHECK(!rp.atoms[0].isRGroup, "\"R9\" is NOT flagged isRGroup (out of the R1-R8 range)");
    }

    // hOnLeft: true when the atom's neighbors sit predominantly to its right (H drawn on the
    // left to avoid overlapping the bond), false when they sit to the left.
    {
        EditableMolecule m;
        AtomId left = m.addAtom(QStringLiteral("O"), 0.0, 0.0);
        AtomId right = m.addAtom(QStringLiteral("C"), 1.0, 0.0);   // neighbor is to the RIGHT of `left`
        m.addBond(left, right, 1);
        RenderPrimitives rp = RenderPrimitiveBuilder::build(m, false);
        AtomPrim* leftPrim = nullptr;
        for (AtomPrim& a : rp.atoms) if (a.id == left) leftPrim = &a;
        CHECK(leftPrim && leftPrim->hOnLeft, "an atom whose neighbor sits to its right has hOnLeft=true");
    }
    {
        EditableMolecule m;
        AtomId right = m.addAtom(QStringLiteral("O"), 1.0, 0.0);
        AtomId left = m.addAtom(QStringLiteral("C"), 0.0, 0.0);   // neighbor is to the LEFT of `right`
        m.addBond(right, left, 1);
        RenderPrimitives rp = RenderPrimitiveBuilder::build(m, false);
        AtomPrim* rightPrim = nullptr;
        for (AtomPrim& a : rp.atoms) if (a.id == right) rightPrim = &a;
        CHECK(rightPrim && !rightPrim->hOnLeft, "an atom whose neighbor sits to its left has hOnLeft=false");
    }

    // Sgroup contraction: sub-project 3d's real "Ac" functional group, inserted via
    // insertStructure directly (bypassing DocumentState, matching this migration's seeding
    // discipline). One synthetic SgroupPrim; ALL 3 member atoms absent from the main atom list
    // (using the new sgroupMemberAtomIds, not just the attach atom).
    {
        EditableMolecule m;
        QString acMolfile = QStringLiteral(
            "Ac\n"
            "  Ketcher\n\n"
            "  3  2  0  0  0  0  0  0  0  0999 V2000\n"
            "    3.3951   -3.5754    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0\n"
            "    2.6785   -3.9891    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0\n"
            "    3.3951   -2.7480    0.0000 O   0  0  0  0  0  0  0  0  0  0  0  0\n"
            "  2  1  1  0  0  0  0\n"
            "  1  3  2  0  0  0  0\n"
            "M  STY  1   1 SUP\n"
            "M  SLB  1   1   1\n"
            "M  SAL   1  3   1   2   3\n"
            "M  SAP   1  1   1   0\n"
            "M  SMT   1 Ac\n"
            "M  END\n");
        EditableMolecule::InsertResult r = m.insertStructure(acMolfile, [](double x, double y) { return QPointF(x, y); });
        CHECK(r.createdSGroups.size() == 1, "setup: \"Ac\" produces exactly one sgroup");
        CHECK(m.sgroupExpanded(r.createdSGroups[0]), "setup: sgroup starts expanded");

        RenderPrimitives expanded = RenderPrimitiveBuilder::build(m, false);
        CHECK(expanded.sgroups.isEmpty(), "an EXPANDED sgroup produces no SgroupPrim");
        CHECK(expanded.atoms.size() == 3, "an EXPANDED sgroup's member atoms are all present normally");

        m.setSGroupExpanded(r.createdSGroups[0], false);
        RenderPrimitives collapsed = RenderPrimitiveBuilder::build(m, false);
        CHECK(collapsed.sgroups.size() == 1, "a COLLAPSED sgroup produces exactly one SgroupPrim");
        CHECK(collapsed.atoms.size() == 1, "a COLLAPSED sgroup's 3 member atoms are ALL replaced by 1 synthetic entry");
        if (!collapsed.atoms.isEmpty()) {
            CHECK(collapsed.atoms[0].isSgroup, "the synthetic entry is flagged isSgroup");
        }
    }
}

static void test_ringsBondsAndAuxiliaryPrimitives() {
    std::printf("--- Test 3: ring/bond primitives + auxiliary primitives ---\n");

    // Real aromatic ring: benzene. hasBondType4=true (real aromatic order), inscribed circle case.
    {
        EditableMolecule m(QStringLiteral("c1ccccc1"));
        RenderPrimitives rp = RenderPrimitiveBuilder::build(m, false);
        CHECK(rp.rings.size() == 1, "benzene produces exactly one ring primitive");
        if (!rp.rings.isEmpty()) {
            CHECK(rp.rings[0].isAromatic, "the ring is aromatic");
            CHECK(rp.rings[0].hasBondType4, "the ring has real aromatic bond order (hasBondType4)");
            CHECK(rp.rings[0].atoms.size() == 6, "the ring primitive lists all 6 atoms");
        }
        CHECK(rp.bonds.size() == 6, "benzene produces 6 bond primitives");
        for (const BondPrim& b : rp.bonds) {
            CHECK(b.inAromaticRing, "every bond is flagged inAromaticRing");
            CHECK(b.hasRingCenter, "every bond has a ring-center offset");
        }
    }

    // Manually-alternating Kekule 6-ring: isAromatic=true via the 3-double-bond heuristic,
    // hasBondType4=false (no real aromatic order anywhere).
    {
        EditableMolecule m;
        QList<AtomId> ring;
        for (int i = 0; i < 6; ++i) ring.append(m.addAtom(QStringLiteral("C"), i * 1.0, 0.0));
        for (int i = 0; i < 6; ++i) {
            int order = (i % 2 == 0) ? 2 : 1;
            m.addBond(ring[i], ring[(i + 1) % 6], order);
        }
        RenderPrimitives rp = RenderPrimitiveBuilder::build(m, false);
        CHECK(rp.rings.size() == 1, "the Kekule ring produces exactly one ring primitive");
        if (!rp.rings.isEmpty()) {
            CHECK(rp.rings[0].isAromatic, "the Kekule ring is aromatic via the 6-ring/3-double-bond heuristic");
            CHECK(!rp.rings[0].hasBondType4, "the Kekule ring has NO real aromatic bond order");
        }
    }

    // Non-ring, non-aromatic case: ethane. No rings, no aromatic bonds, no ring-center offsets.
    {
        EditableMolecule m(QStringLiteral("CC"));
        RenderPrimitives rp = RenderPrimitiveBuilder::build(m, false);
        CHECK(rp.rings.isEmpty(), "ethane has no rings");
        CHECK(rp.bonds.size() == 1, "ethane has 1 bond");
        if (!rp.bonds.isEmpty()) {
            CHECK(!rp.bonds[0].inAromaticRing, "the bond is not in an aromatic ring");
            CHECK(!rp.bonds[0].hasRingCenter, "the bond has no ring-center offset");
        }
    }

    // invalidStereo, real end-to-end case: a real wedge bond on a well-formed tetrahedral
    // stereocenter (C bonded to N via a wedge, plus O and F -- 3 real substituents, so none of
    // isCorrectStereoCenter's 3 early-return-false branches trigger) must read invalidStereo=false.
    // This is the SAME molfile sub-project 3a/3b already confirmed produces a real nonzero
    // indigoBondStereo (UP=5) -- reused here rather than inventing an unverified new one.
    {
        EditableMolecule m(QStringLiteral(
            "\n  spike\n\n"
            "  4  3  0  0  0  0  0  0  0  0999 V2000\n"
            "    0.0000    0.0000    0.0000 C   0  0\n"
            "    1.0000    0.0000    0.0000 N   0  0\n"
            "    0.0000    1.0000    0.0000 O   0  0\n"
            "   -1.0000    0.0000    0.0000 F   0  0\n"
            "  1  2  1  1  0  0  0\n"
            "  1  3  1  0  0  0  0\n"
            "  1  4  1  0  0  0  0\n"
            "M  END\n"));
        CHECK(m.isValid(), "setup: the wedge-bond molfile loads");
        RenderPrimitives rp = RenderPrimitiveBuilder::build(m, false);
        BondPrim* wedgeBond = nullptr;
        for (BondPrim& b : rp.bonds) {
            if (b.stereo != 0) wedgeBond = &b;
        }
        CHECK(wedgeBond != nullptr, "the wedge bond reports a nonzero stereo direction");
        if (wedgeBond) {
            CHECK(!wedgeBond->invalidStereo,
                  "a well-formed stereocenter (3 real substituents) reads invalidStereo=false");
        }
    }

    // Cross-bond substitution: a collapsed sgroup's cross-bond (target atom outside the
    // sgroup, bonded to the sgroup's own attach atom) gets its hidden endpoint substituted with
    // the sgroup's synthetic id, NOT skipped -- matching buildRenderPrimitives exactly for a
    // cross-bond (as opposed to a fully-internal bond, which IS skipped).
    {
        EditableMolecule m;
        AtomId target = m.addAtom(QStringLiteral("N"), 5.0, 5.0);
        QString acMolfile = QStringLiteral(
            "Ac\n"
            "  Ketcher\n\n"
            "  3  2  0  0  0  0  0  0  0  0999 V2000\n"
            "    3.3951   -3.5754    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0\n"
            "    2.6785   -3.9891    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0\n"
            "    3.3951   -2.7480    0.0000 O   0  0  0  0  0  0  0  0  0  0  0  0\n"
            "  2  1  1  0  0  0  0\n"
            "  1  3  2  0  0  0  0\n"
            "M  STY  1   1 SUP\n"
            "M  SLB  1   1   1\n"
            "M  SAL   1  3   1   2   3\n"
            "M  SAP   1  1   1   0\n"
            "M  SMT   1 Ac\n"
            "M  END\n");
        EditableMolecule::InsertResult r = m.insertStructure(acMolfile, [](double x, double y) { return QPointF(x, y); });
        AtomId attach = -1;
        CHECK(m.superatomAttachAtom(r.createdSGroups[0], attach), "setup: attach atom resolves");
        m.addBond(target, attach, 1);   // a real cross-bond: target (outside) bonded to attach (inside the sgroup)
        m.setSGroupExpanded(r.createdSGroups[0], false);

        RenderPrimitives rp = RenderPrimitiveBuilder::build(m, false);
        bool foundCrossBond = false;
        for (const BondPrim& b : rp.bonds) {
            bool touchesTarget = (b.begin == target || b.end == target);
            bool touchesSgroup = (b.beginIsSgroup || b.endIsSgroup);
            if (touchesTarget && touchesSgroup) foundCrossBond = true;
        }
        CHECK(foundCrossBond, "the cross-bond survives, substituted to reference the synthetic sgroup id");
    }

    // Auxiliary primitives: text, image, rxn arrow (with mode/conditions/curvature defaults),
    // rxn plus, multitail arrow -- straight field round-trips from ExtensionData.
    {
        EditableMolecule m;
        int textId = m.addTextAnnotation(1.0, 2.0, QStringLiteral("note"));
        m.setTextAnnotation(textId, QStringLiteral("note"), true, false);
        int imageId = m.addImage(0.0, 0.0, 4.0, 3.0, QByteArray("fakepng"));
        int arrowId = m.addRxnArrow(0.0, 0.0, 5.0, 0.0);
        int plusId = m.addRxnPlus(2.5, 0.0);
        int mtaId = m.addMultitailArrow({5, 5, 0, 4, 0, 6});

        RenderPrimitives rp = RenderPrimitiveBuilder::build(m, false);

        CHECK(rp.texts.size() == 1, "one text primitive");
        if (!rp.texts.isEmpty()) {
            CHECK(rp.texts[0].id == textId, "text id round-trips");
            CHECK(rp.texts[0].content == QStringLiteral("note"), "text content round-trips");
            CHECK(rp.texts[0].bold && !rp.texts[0].italic, "real bold/italic values round-trip (no longer hardcoded false)");
        }

        CHECK(rp.images.size() == 1, "one image primitive");
        if (!rp.images.isEmpty()) {
            CHECK(rp.images[0].id == imageId, "image id round-trips");
            CHECK(rp.images[0].w == 4.0 && rp.images[0].h == 3.0, "image dimensions round-trip");
        }

        CHECK(rp.rxnArrows.size() == 1, "one rxn-arrow primitive");
        if (!rp.rxnArrows.isEmpty()) {
            CHECK(rp.rxnArrows[0].id == arrowId, "rxn-arrow id round-trips");
            CHECK(rp.rxnArrows[0].mode == QStringLiteral("filled-triangle"), "rxn-arrow mode defaults correctly");
            CHECK(rp.rxnArrows[0].conditionsAbove.isEmpty() && rp.rxnArrows[0].conditionsBelow.isEmpty(),
                  "rxn-arrow conditions default to empty");
            CHECK(!rp.rxnArrows[0].hasCurvature, "rxn-arrow curvature defaults to absent");
        }

        CHECK(rp.rxnPluses.size() == 1, "one rxn-plus primitive");
        if (!rp.rxnPluses.isEmpty()) CHECK(rp.rxnPluses[0].id == plusId, "rxn-plus id round-trips");

        CHECK(rp.multitailArrows.size() == 1, "one multitail-arrow primitive");
        if (!rp.multitailArrows.isEmpty()) {
            CHECK(rp.multitailArrows[0].id == mtaId, "multitail-arrow id round-trips");
            CHECK(rp.multitailArrows[0].headX == 5 && rp.multitailArrows[0].headY == 5,
                  "multitail-arrow head position round-trips");
            // {5,5, 0,4, 0,6} is head(5,5) + 2 tail points (0,4) and (0,6) -- the same 6-number
            // shape sub-project 1's own tests use for addMultitailArrow.
            CHECK(rp.multitailArrows[0].tails.size() == 2, "multitail-arrow has 2 tail points");
        }
    }

    // Bbox: empty molecule has valid=false.
    {
        EditableMolecule m;
        RenderPrimitives rp = RenderPrimitiveBuilder::build(m, false);
        CHECK(!rp.bbox.valid, "an empty molecule's bbox is invalid");
    }

    // stereoFlagsType/GroupId default correctly (common case: never set).
    {
        EditableMolecule m(QStringLiteral("CCO"));
        RenderPrimitives rp = RenderPrimitiveBuilder::build(m, false);
        CHECK(rp.stereoFlagsType == QStringLiteral("abs"), "stereoFlagsType defaults to \"abs\"");
        CHECK(rp.stereoFlagsGroupId == 0, "stereoFlagsGroupId defaults to 0");
    }

    // atomCheckWarningText/bondCheckWarningText round-trip a real string, independent of the
    // existing boolean atomCheckWarning API.
    {
        EditableMolecule m(QStringLiteral("CCO"));
        QList<AtomId> ids = m.atomIds();
        // No public setter exists for the new string check-warning storage (sub-project 3e's
        // job) -- this confirms the DEFAULT (empty) round-trips correctly through the builder,
        // matching Test 22's own coverage of the accessor default.
        RenderPrimitives rp = RenderPrimitiveBuilder::build(m, false);
        for (const AtomPrim& a : rp.atoms) {
            CHECK(a.checkWarning.isEmpty(), "checkWarning defaults to empty for every atom");
        }
    }
}

static void test_attachmentPointsAndRGroupBracketPrimitives() {
    std::printf("--- Test 4: attachmentPoints, R-group, and bracket primitives ---\n");

    // AtomPrim::attachmentPoints -- not r-group-specific, just the atom-level V2000 attachment
    // point bitmask (see RenderPrimitives.h). Order 1 -> bit 0 (value 1), order 2 -> bit 1 (value 2).
    {
        EditableMolecule m;
        AtomId a1 = m.addAtom(QStringLiteral("C"), 0, 0);
        AtomId a2 = m.addAtom(QStringLiteral("C"), 1, 0);
        m.setAtomAttachmentOrder(a1, 1);
        m.setAtomAttachmentOrder(a2, 2);

        RenderPrimitives rp = RenderPrimitiveBuilder::build(m, false);
        int found = 0;
        for (const AtomPrim& a : rp.atoms) {
            if (a.id == a1) { CHECK(a.attachmentPoints == 1, "order-1 atom gets bitmask 1"); found++; }
            if (a.id == a2) { CHECK(a.attachmentPoints == 2, "order-2 atom gets bitmask 2"); found++; }
        }
        CHECK(found == 2, "both attachment-point atoms were found in the primitive list");
    }

    // R-group + bracket primitives, mirroring 10-state.js:1374-1393's real output shape.
    {
        EditableMolecule m(QStringLiteral("CC.CC"));   // two disjoint fragments, same setup as
                                                        // sub-project 3e's own atomFragmentIndex test
        QList<AtomId> ids = m.atomIds();
        int frag0 = m.atomFragmentIndex(ids[0]);
        int frag1 = m.atomFragmentIndex(ids[2]);

        m.addRGroupEntry(1);
        m.setRGroupLogic(1, QStringLiteral("1,2"), true, 2);
        m.addRGroupFragment(1, frag0);
        m.addRGroupFragment(1, frag1);

        m.pushBracket(-1, -2, 3, 4);

        RenderPrimitives rp = RenderPrimitiveBuilder::build(m, false);

        CHECK(rp.rgroups.size() == 1, "one R-group primitive");
        if (!rp.rgroups.isEmpty()) {
            const RGroupPrim& rg = rp.rgroups[0];
            CHECK(rg.number == 1, "R-group number round-trips");
            CHECK(rg.range == QStringLiteral("1,2") && rg.resth && rg.ifthen == 2, "R-group logic fields round-trip");
            CHECK(rg.members.size() == 2, "both member fragments are present");
            int totalAtoms = 0;
            for (const RGroupMemberPrim& mem : rg.members) totalAtoms += mem.atomIds.size();
            CHECK(totalAtoms == 4, "each member fragment resolves to its real 2-atom set (4 atoms total)");
        }

        CHECK(rp.brackets.size() == 1, "one bracket primitive");
        if (!rp.brackets.isEmpty()) {
            CHECK(rp.brackets[0].minX == -1 && rp.brackets[0].minY == -2 &&
                  rp.brackets[0].maxX == 3 && rp.brackets[0].maxY == 4, "bracket bbox round-trips");
        }
    }
}

int main() {
    unsigned long long session = indigoAllocSessionId();
    indigoSetSessionId(session);

    test_elementData();
    test_atomPrimitivesAndSgroupContraction();
    test_ringsBondsAndAuxiliaryPrimitives();
    test_attachmentPointsAndRGroupBracketPrimitives();

    indigoReleaseSessionId(session);
    std::printf("Summary: %d passed, %d failed.\n", g_pass, g_fail);
    return g_fail == 0 ? 0 : 1;
}
