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

int main() {
    test_selectionStateToVariantEmpty();
    test_selectionStateToVariantMixed();
    std::printf("Summary: %d passed, %d failed.\n", g_pass, g_fail);
    return g_fail == 0 ? 0 : 1;
}
