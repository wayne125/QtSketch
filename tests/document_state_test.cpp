// tests/document_state_test.cpp
// Standalone tests for DocumentState (chem-core.js migration, sub-project 2).
#include <cstdio>
#include "app/molecule/SelectionState.h"
#include "app/molecule/EditCommand.h"

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

int main() {
    test_selectionStateBasics();
    test_editCommandBasics();
    std::printf("Summary: %d passed, %d failed.\n", g_pass, g_fail);
    return g_fail == 0 ? 0 : 1;
}
