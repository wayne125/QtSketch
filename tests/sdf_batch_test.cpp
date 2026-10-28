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

static void test_loadFromSdfText() {
    std::printf("--- Test 4: loadFromSdfText ---\n");
    QString sdf = QStringLiteral(
        "mol1\n"
        "  Test\n\n"
        "  1  0  0  0  0  0  0  0  0  0999 V2000\n"
        "    0.0000    0.0000    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0\n"
        "M  END\n"
        ">  <PROP1>\n"
        "value1\n"
        "\n"
        "$$$$\n"
        "mol2\n"
        "  Test\n\n"
        "  1  0  0  0  0  0  0  0  0  0999 V2000\n"
        "    0.0000    0.0000    0.0000 N   0  0  0  0  0  0  0  0  0  0  0  0\n"
        "M  END\n"
        "$$$$\n");

    SdfBatch batch;
    CHECK(batch.loadFromSdfText(sdf), "loadFromSdfText succeeds");
    CHECK(batch.recordCount() == 2, "two records loaded from the SDF text");
    CHECK(batch.labelAt(0) == QStringLiteral("mol1"), "label 0 comes from the molfile title line");
    CHECK(batch.labelAt(1) == QStringLiteral("mol2"), "label 1 comes from the molfile title line");
    QHash<QString, QString> props0 = batch.propsAt(0);
    CHECK(props0.value(QStringLiteral("PROP1")) == QStringLiteral("value1"), "propsAt reads the real SDF tag value");
    CHECK(batch.propsAt(1).isEmpty(), "record 2 has no property tags");
    CHECK(!batch.thumbnailAt(0).atoms.isEmpty(), "thumbnail computed for record 0");
}

static void test_loadFromSdfTextWithGLine() {
    std::printf("--- Test 5: loadFromSdfText tolerates a Ketcher G-line ---\n");
    QString sdf = QStringLiteral(
        "molG\n"
        "  Test\n\n"
        "  1  0  0  0  0  0  0  0  0  0999 V2000\n"
        "    0.0000    0.0000    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0\n"
        "G    1  0\n"
        "M  END\n"
        "$$$$\n");
    SdfBatch batch;
    CHECK(batch.loadFromSdfText(sdf), "loadFromSdfText succeeds on a record with a G-line");
    CHECK(batch.recordCount() == 1, "the G-line record is not dropped");
    CHECK(batch.labelAt(0) == QStringLiteral("molG"), "label still resolves correctly");
}

static void test_loadFromSdfTextEmpty() {
    std::printf("--- Test 6: loadFromSdfText on empty input ---\n");
    SdfBatch batch;
    CHECK(!batch.loadFromSdfText(QString()), "empty input returns false");
    CHECK(batch.recordCount() == 0, "batch stays empty");
}

static void test_loadFromSdfTextSkipsInvalid() {
    std::printf("--- Test 7: loadFromSdfText skips an unparseable record among valid ones ---\n");
    QString sdf = QStringLiteral(
        "good1\n"
        "  Test\n\n"
        "  1  0  0  0  0  0  0  0  0  0999 V2000\n"
        "    0.0000    0.0000    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0\n"
        "M  END\n"
        "$$$$\n"
        "this is not a valid molfile record at all\n"
        "$$$$\n"
        "good2\n"
        "  Test\n\n"
        "  1  0  0  0  0  0  0  0  0  0999 V2000\n"
        "    0.0000    0.0000    0.0000 N   0  0  0  0  0  0  0  0  0  0  0  0\n"
        "M  END\n"
        "$$$$\n");
    SdfBatch batch;
    CHECK(batch.loadFromSdfText(sdf), "loadFromSdfText succeeds overall");
    CHECK(batch.recordCount() == 2, "the one unparseable record is skipped, not aborting the batch");
    CHECK(batch.labelAt(0) == QStringLiteral("good1") && batch.labelAt(1) == QStringLiteral("good2"),
          "the two valid records are the ones that survived, in order");
}

static void test_thumbnailCapAppliesToThumbnailsOnly() {
    std::printf("--- Test 8: the 500 cap bounds thumbnails, not record count ---\n");
    QStringList mols;
    for (int i = 0; i < 501; ++i) mols << QStringLiteral("C");
    SdfBatch batch;
    CHECK(batch.loadFromMolfileList(mols), "loadFromMolfileList succeeds with 501 entries");
    CHECK(batch.recordCount() == 501, "all 501 records are stored -- record count is NOT capped");
    CHECK(!batch.thumbnailAt(499).atoms.isEmpty(), "the 500th record (index 499, last one under the cap) has a real thumbnail");
    CHECK(batch.thumbnailAt(500).atoms.isEmpty(), "the 501st record (index 500, past the cap) has an empty thumbnail");
    CHECK(!batch.molfileAt(500).isEmpty(), "but its molfile text is still stored -- only the thumbnail is skipped");
}

static void test_realign() {
    std::printf("--- Test 9: realign ---\n");
    SdfBatch batch;
    QStringList mols;
    mols << QStringLiteral("CCO") << QStringLiteral("N") << QStringLiteral("O");
    batch.loadFromMolfileList(mols);
    QString origMolfile1 = batch.molfileAt(1);
    QString origLabel1 = batch.labelAt(1);

    QStringList wrongLength;
    wrongLength << QStringLiteral("C");
    CHECK(!batch.realign(wrongLength), "realign with the wrong length returns false");
    CHECK(batch.recordCount() == 3, "wrong-length realign leaves the batch completely unchanged");
    CHECK(batch.molfileAt(1) == origMolfile1, "unchanged record 1 after a rejected realign");

    QStringList realigned;
    realigned << QStringLiteral("CC") << QStringLiteral("not valid at all $$$") << QStringLiteral("F");
    CHECK(batch.realign(realigned), "realign with matching length succeeds");
    CHECK(batch.recordCount() == 3, "record count unchanged after realign");
    CHECK(batch.molfileAt(1) == origMolfile1, "the entry whose replacement failed to parse keeps its OLD text");
    CHECK(batch.labelAt(1) == origLabel1, "the entry whose replacement failed to parse keeps its OLD label too");
    CHECK(batch.molfileAt(0) != origMolfile1, "entry 0 (a genuinely different molecule) was updated");
}

static void test_realignEmptyVsEmpty() {
    std::printf("--- Test 10: realign on an empty batch with an empty list ---\n");
    SdfBatch batch;
    CHECK(batch.realign(QStringList()), "realigning an empty batch with an empty list trivially succeeds (matching lengths, 0==0)");
    CHECK(batch.recordCount() == 0, "still empty");
}

int main() {
    unsigned long long session = indigoAllocSessionId();
    indigoSetSessionId(session);

    test_loadFromMolfileList();
    test_loadFromMolfileListSkipsInvalid();
    test_thumbnailAt();
    test_loadFromSdfText();
    test_loadFromSdfTextWithGLine();
    test_loadFromSdfTextEmpty();
    test_loadFromSdfTextSkipsInvalid();
    test_thumbnailCapAppliesToThumbnailsOnly();
    test_realign();
    test_realignEmptyVsEmpty();

    indigoReleaseSessionId(session);
    std::printf("Summary: %d passed, %d failed.\n", g_pass, g_fail);
    return g_fail == 0 ? 0 : 1;
}
