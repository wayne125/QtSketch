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

int main() {
    unsigned long long session = indigoAllocSessionId();
    indigoSetSessionId(session);

    test_elementData();

    indigoReleaseSessionId(session);
    std::printf("Summary: %d passed, %d failed.\n", g_pass, g_fail);
    return g_fail == 0 ? 0 : 1;
}
