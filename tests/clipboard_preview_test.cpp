// tests/clipboard_preview_test.cpp
// Standalone tests for ClipboardPreview (chem-core.js migration, sub-project 5d).
#include <cstdio>
#include <cmath>
#include "app/molecule/ClipboardPreview.h"
#include "indigo.h"

static int g_pass = 0, g_fail = 0;
#define CHECK(cond, name) do { \
    if (cond) { ++g_pass; std::printf("[PASS] %s\n", name); } \
    else { ++g_fail; std::printf("[FAIL] %s (line %d)\n", name, __LINE__); } \
} while (0)

static void test_buildClipboardPreview() {
    std::printf("--- Test 1: buildClipboardPreview ---\n");
    QString molfile = QStringLiteral(
        "\n     RDKit          2D\n\n"
        "  2  1  0  0  0  0  0  0  0  0999 V2000\n"
        "    0.0000    0.0000    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0\n"
        "    1.0000    0.0000    0.0000 O   0  0  0  0  0  0  0  0  0  0  0  0\n"
        "  1  2  1  0\n"
        "M  END\n");
    ClipboardPreview preview = buildClipboardPreview(molfile);
    CHECK(preview.atoms.size() == 2, "preview has 2 atoms");
    CHECK(preview.bonds.size() == 1, "preview has 1 bond");

    bool foundC = false, foundO = false;
    for (const auto& a : preview.atoms) {
        if (a.label == QStringLiteral("C")) foundC = true;
        if (a.label == QStringLiteral("O")) foundO = true;
    }
    CHECK(foundC && foundO, "atom labels round-trip");

    const auto& b = preview.bonds.first();
    bool endpointsMatch = (std::abs(b.x1 - 0.0) < 1e-6 && std::abs(b.x2 - 1.0) < 1e-6) ||
                          (std::abs(b.x2 - 0.0) < 1e-6 && std::abs(b.x1 - 1.0) < 1e-6);
    CHECK(endpointsMatch, "bond endpoint coordinates match the atom positions");

    CHECK(std::abs(preview.cx - 0.5) < 1e-6, "cx is the RAW (unscaled) bbox center, not a normalized unit-box value");
    CHECK(std::abs(preview.cy - 0.0) < 1e-6, "cy is the raw bbox center");
}

static void test_buildClipboardPreviewEmpty() {
    std::printf("--- Test 2: buildClipboardPreview on empty/unparseable input ---\n");
    ClipboardPreview preview = buildClipboardPreview(QString());
    CHECK(preview.atoms.isEmpty(), "empty input produces an empty preview");
    CHECK(preview.bonds.isEmpty(), "empty input produces no bonds");
    CHECK(preview.cx == 0.0 && preview.cy == 0.0, "empty input defaults cx/cy to 0,0");

    ClipboardPreview preview2 = buildClipboardPreview(QStringLiteral("not a valid molecule $$$"));
    CHECK(preview2.atoms.isEmpty(), "unparseable input produces an empty preview");
}

int main() {
    unsigned long long session = indigoAllocSessionId();
    indigoSetSessionId(session);

    test_buildClipboardPreview();
    test_buildClipboardPreviewEmpty();

    indigoReleaseSessionId(session);
    std::printf("Summary: %d passed, %d failed.\n", g_pass, g_fail);
    return g_fail == 0 ? 0 : 1;
}
