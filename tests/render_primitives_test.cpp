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

int main() {
    unsigned long long session = indigoAllocSessionId();
    indigoSetSessionId(session);

    test_elementData();
    test_atomPrimitivesAndSgroupContraction();

    indigoReleaseSessionId(session);
    std::printf("Summary: %d passed, %d failed.\n", g_pass, g_fail);
    return g_fail == 0 ? 0 : 1;
}
