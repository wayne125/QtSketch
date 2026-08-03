// tests/sdf_batch_test.cpp
// Standalone tests for SdfBatch (chem-core.js migration, sub-project 5c).
#include <cstdio>
#include "app/molecule/SdfBatch.h"
#include "indigo.h"

static int g_pass = 0, g_fail = 0;
#define CHECK(cond, name) do { \
    if (cond) { ++g_pass; std::printf("[PASS] %s\n", name); } \
    else { ++g_fail; std::printf("[FAIL] %s (line %d)\n", name, __LINE__); } \
} while (0)

static void test_loadFromMolfileList() {
    std::printf("--- Test 1: loadFromMolfileList ---\n");
    SdfBatch batch;
    CHECK(batch.recordCount() == 0, "empty batch initially");

    QStringList mols;
    mols << QStringLiteral("CCO") << QStringLiteral("c1ccccc1");
    CHECK(batch.loadFromMolfileList(mols), "loadFromMolfileList succeeds");
    CHECK(batch.recordCount() == 2, "two records loaded");

    CHECK(!batch.molfileAt(0).isEmpty(), "molfileAt(0) returns real MOL text");
    CHECK(!batch.molfileAt(1).isEmpty(), "molfileAt(1) returns real MOL text");
    CHECK(batch.propsAt(0).isEmpty(), "propsAt is empty for the molfile-list path (no SDF tags)");
    CHECK(batch.propsAt(1).isEmpty(), "propsAt is empty for the molfile-list path (no SDF tags)");

    // Labels fall back to "Record N" since plain SMILES/MOL text with no title line has no name.
    CHECK(batch.labelAt(0) == QStringLiteral("Record 1"), "label falls back to Record 1");
    CHECK(batch.labelAt(1) == QStringLiteral("Record 2"), "label falls back to Record 2");
}

static void test_loadFromMolfileListSkipsInvalid() {
    std::printf("--- Test 2: loadFromMolfileList skips unparseable entries ---\n");
    SdfBatch batch;
    QStringList mols;
    mols << QStringLiteral("CCO") << QStringLiteral("not a valid molecule $$$") << QStringLiteral("N");
    CHECK(batch.loadFromMolfileList(mols), "loadFromMolfileList succeeds overall");
    CHECK(batch.recordCount() == 2, "the one invalid entry is skipped, not aborting the batch");
}

static void test_thumbnailAt() {
    std::printf("--- Test 3: thumbnailAt ---\n");
    SdfBatch batch;
    QStringList mols;
    mols << QStringLiteral("CCO");
    batch.loadFromMolfileList(mols);
    SdfBatch::Thumbnail t = batch.thumbnailAt(0);
    CHECK(!t.atoms.isEmpty(), "thumbnail has atoms");
    CHECK(t.bonds.size() >= 1, "thumbnail has at least one bond (ethanol has 2 heavy-atom bonds)");
    for (const auto& a : t.atoms) {
        CHECK(a.x >= 0.0 && a.x <= 1.0 && a.y >= 0.0 && a.y <= 1.0,
              "thumbnail atom coordinates fall within the padded unit box");
    }

    CHECK(batch.thumbnailAt(99).atoms.isEmpty(), "thumbnailAt on an out-of-range index returns an empty thumbnail");
}

int main() {
    unsigned long long session = indigoAllocSessionId();
    indigoSetSessionId(session);

    test_loadFromMolfileList();
    test_loadFromMolfileListSkipsInvalid();
    test_thumbnailAt();

    indigoReleaseSessionId(session);
    std::printf("Summary: %d passed, %d failed.\n", g_pass, g_fail);
    return g_fail == 0 ? 0 : 1;
}
