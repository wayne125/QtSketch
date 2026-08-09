// tests/biopolymer_sequence_view_test.cpp
// Standalone tests for BiopolymerSequenceView (chem-core.js migration, sub-project 6d).
#include <cstdio>
#include "app/molecule/BiopolymerSequenceView.h"

static int g_pass = 0, g_fail = 0;
#define CHECK(cond, name) do { \
    if (cond) { ++g_pass; std::printf("[PASS] %s\n", name); } \
    else { ++g_fail; std::printf("[FAIL] %s (line %d)\n", name, __LINE__); } \
} while (0)

static void test_buildSequenceViewPeptide() {
    std::printf("--- Test: buildSequenceView, plain peptide sequence ---\n");
    BiopolymerSequenceView view;
    view.buildSequenceView(QStringLiteral("ACDEFG"), QStringLiteral("PEPTIDE"));

    QList<BioMonomer> mons = view.monomers();
    CHECK(mons.size() == 6, "6 monomers");
    CHECK(mons[0].label == QStringLiteral("A") && mons[0].alias == QStringLiteral("Ala")
          && mons[0].monomerClass == QStringLiteral("AminoAcid") && !mons[0].ambiguous,
          "first monomer is Alanine");
    CHECK(mons[5].label == QStringLiteral("G") && mons[5].alias == QStringLiteral("Gly"),
          "last monomer is Glycine");
    CHECK(mons[0].x == 0.0 && mons[0].y == 0.0, "first monomer at origin");
    CHECK(mons[1].x == 60.0 && mons[1].y == 0.0, "second monomer offset by cell width");

    QList<BioBond> bonds = view.bonds();
    CHECK(bonds.size() == 5, "5 linear bonds for 6 monomers");
    for (int i = 0; i < 5; ++i) {
        CHECK(bonds[i].fromId == mons[i].id && bonds[i].toId == mons[i + 1].id,
              "bond connects consecutive monomers in order");
    }
    CHECK(view.seqType() == QStringLiteral("PEPTIDE"), "seqType is PEPTIDE");
}

static void test_buildSequenceViewStripsFastaHeaderAndNonLetters() {
    std::printf("--- Test: buildSequenceView strips FASTA header and non-letters ---\n");
    BiopolymerSequenceView view;
    view.buildSequenceView(QStringLiteral(">header line\nAC12DE*\n"), QStringLiteral("PEPTIDE"));
    QList<BioMonomer> mons = view.monomers();
    CHECK(mons.size() == 4, "4 monomers: header stripped, digits/asterisk stripped");
    QString labels;
    for (const BioMonomer& m : mons) labels += m.label;
    CHECK(labels == QStringLiteral("ACDE"), "labels are A,C,D,E in order");
}

static void test_buildSequenceViewSkipsUnknownWithoutLayoutGap() {
    std::printf("--- Test: buildSequenceView skips unknown symbols without a layout gap ---\n");
    BiopolymerSequenceView rnaView;
    rnaView.buildSequenceView(QStringLiteral("ACXG"), QStringLiteral("RNA"));
    QList<BioMonomer> mons = rnaView.monomers();
    CHECK(mons.size() == 3, "3 monomers: X skipped (not an RNA code)");
    QString labels;
    for (const BioMonomer& m : mons) labels += m.label;
    CHECK(labels == QStringLiteral("ACG"), "labels are A,C,G -- X silently skipped");
    CHECK(mons[2].label == QStringLiteral("G") && mons[2].x == 120.0,
          "G takes layout slot 2 (accepted-count-based), not slot 3 (raw-char-index-based)");
}

static void test_buildSequenceViewReplacesNotAppends() {
    std::printf("--- Test: buildSequenceView called twice replaces rather than appends ---\n");
    BiopolymerSequenceView view;
    view.buildSequenceView(QStringLiteral("ACDE"), QStringLiteral("PEPTIDE"));
    CHECK(view.monomers().size() == 4, "setup: 4 monomers");
    view.buildSequenceView(QStringLiteral("GH"), QStringLiteral("PEPTIDE"));
    CHECK(view.monomers().size() == 2, "second call replaces, not appends");
}

static void test_buildSequenceViewProteinSynonym() {
    std::printf("--- Test: buildSequenceView accepts PROTEIN as a PEPTIDE synonym ---\n");
    BiopolymerSequenceView view;
    view.buildSequenceView(QStringLiteral("AC"), QStringLiteral("protein"));
    CHECK(view.seqType() == QStringLiteral("PEPTIDE"), "PROTEIN normalizes to PEPTIDE");
    CHECK(view.monomers().size() == 2 && view.monomers()[0].alias == QStringLiteral("Ala"),
          "resolved against the PEPTIDE table");
}

static void test_buildSequenceViewRnaAndDna() {
    std::printf("--- Test: buildSequenceView resolves RNA and DNA tables ---\n");
    BiopolymerSequenceView rna;
    rna.buildSequenceView(QStringLiteral("ACGUN"), QStringLiteral("RNA"));
    QList<BioMonomer> rmons = rna.monomers();
    CHECK(rmons.size() == 5, "5 RNA monomers");
    CHECK(rmons[3].label == QStringLiteral("U") && rmons[3].alias == QStringLiteral("U")
          && !rmons[3].ambiguous, "U is a standard RNA base");
    CHECK(rmons[4].label == QStringLiteral("N") && rmons[4].ambiguous,
          "N is an ambiguous RNA code");

    BiopolymerSequenceView dna;
    dna.buildSequenceView(QStringLiteral("ACGT"), QStringLiteral("DNA"));
    QList<BioMonomer> dmons = dna.monomers();
    CHECK(dmons.size() == 4, "4 DNA monomers");
    CHECK(dmons[3].label == QStringLiteral("T") && dmons[3].alias == QStringLiteral("dT")
          && !dmons[3].ambiguous, "T resolves to dT in the DNA table");
}

static void test_buildSequenceViewUnrecognizedSeqTypeStoresVerbatim() {
    std::printf("--- Test: unrecognized seqType stores verbatim, table falls back to PEPTIDE ---\n");
    BiopolymerSequenceView view;
    view.buildSequenceView(QStringLiteral("AC"), QStringLiteral("XYZ"));
    CHECK(view.seqType() == QStringLiteral("XYZ"),
          "seqType() stores the unrecognized value verbatim -- NOT coerced to PEPTIDE");
    CHECK(view.monomers().size() == 2 && view.monomers()[0].alias == QStringLiteral("Ala"),
          "character lookup still falls back to the PEPTIDE table");
}

static void test_buildSequenceViewEmptyAfterStripping() {
    std::printf("--- Test: non-empty input reducing to zero letters produces an empty view ---\n");
    BiopolymerSequenceView view;
    view.buildSequenceView(QStringLiteral("AC"), QStringLiteral("PEPTIDE"));
    CHECK(view.monomers().size() == 2, "setup: 2 monomers before the all-invalid-char call");
    view.buildSequenceView(QStringLiteral("   123\n***"), QStringLiteral("PEPTIDE"));
    CHECK(view.monomers().isEmpty(), "reduces to empty (cleared, then the char loop does nothing)");
    CHECK(view.bonds().isEmpty(), "bonds empty too");
}

static void test_buildSequenceViewTrueEmptyIsNoOp() {
    std::printf("--- Test: a truly empty sequenceText clears without crashing ---\n");
    BiopolymerSequenceView view;
    view.buildSequenceView(QStringLiteral("AC"), QStringLiteral("PEPTIDE"));
    view.buildSequenceView(QString(), QStringLiteral("PEPTIDE"));
    CHECK(view.monomers().isEmpty(), "empty sequenceText clears the prior sequence");
}

static void test_addMonomerAppendsAndBonds() {
    std::printf("--- Test: addMonomer appends and bonds to the prior last monomer ---\n");
    BiopolymerSequenceView view;
    view.buildSequenceView(QStringLiteral("AC"), QStringLiteral("PEPTIDE"));
    int lastId = view.monomers().last().id;
    view.addMonomer(QStringLiteral("D"));
    QList<BioMonomer> mons = view.monomers();
    CHECK(mons.size() == 3, "3 monomers after addMonomer");
    CHECK(mons[2].label == QStringLiteral("D") && mons[2].alias == QStringLiteral("Asp"),
          "new monomer is Aspartic Acid");
    QList<BioBond> bonds = view.bonds();
    CHECK(bonds.size() == 2, "2 bonds now");
    CHECK(bonds[1].fromId == lastId && bonds[1].toId == mons[2].id,
          "new bond connects the prior last monomer to the new one");
}

static void test_addMonomerOnEmptyViewNoBond() {
    std::printf("--- Test: addMonomer on an empty view creates no bond ---\n");
    BiopolymerSequenceView view;
    view.addMonomer(QStringLiteral("A"), QStringLiteral("PEPTIDE"));
    CHECK(view.monomers().size() == 1, "1 monomer");
    CHECK(view.bonds().isEmpty(), "no bond created for the first monomer");
}

static void test_addMonomerOmittedSeqTypeKeepsCurrent() {
    std::printf("--- Test: addMonomer with omitted seqType keeps the current one ---\n");
    BiopolymerSequenceView view;
    view.buildSequenceView(QStringLiteral("ACGU"), QStringLiteral("RNA"));
    view.addMonomer(QStringLiteral("C"));
    CHECK(view.seqType() == QStringLiteral("RNA"), "seqType stays RNA");
    CHECK(view.monomers().last().alias == QStringLiteral("C"), "resolved against the RNA table");
}

static void test_addMonomerUnknownSymbolIsNoOp() {
    std::printf("--- Test: addMonomer with an unknown symbol is a no-op ---\n");
    BiopolymerSequenceView view;
    view.buildSequenceView(QStringLiteral("ACGU"), QStringLiteral("RNA"));
    view.addMonomer(QStringLiteral("X"), QStringLiteral("RNA"));
    CHECK(view.monomers().size() == 4, "count unchanged");
    CHECK(view.seqType() == QStringLiteral("RNA"), "seqType unchanged (assignment is post-success only)");
}

static void test_deleteMonomerMiddleReconnects() {
    std::printf("--- Test: deleteMonomer on a middle monomer reconnects neighbors and recompacts ---\n");
    BiopolymerSequenceView view;
    view.buildSequenceView(QStringLiteral("ACDEFG"), QStringLiteral("PEPTIDE"));
    QList<BioMonomer> before = view.monomers();
    int middleId = before[2].id;

    view.deleteMonomer(middleId);

    QList<BioMonomer> after = view.monomers();
    CHECK(after.size() == 5, "5 monomers left");
    QString labels;
    for (const BioMonomer& m : after) labels += m.label;
    CHECK(labels == QStringLiteral("ACEFG"), "D removed, sequence order preserved");

    QList<BioBond> bonds = view.bonds();
    CHECK(bonds.size() == 4, "4 bonds -- gap spliced closed");
    bool foundSplice = false;
    for (const BioBond& b : bonds) if (b.fromId == before[1].id && b.toId == before[3].id) foundSplice = true;
    CHECK(foundSplice, "a new bond directly connects C (idx1) to E (idx3), splicing the gap");

    for (int i = 0; i < after.size(); ++i) {
        CHECK(after[i].x == static_cast<double>((i % 20) * 60) && after[i].y == static_cast<double>((i / 20) * 120),
              "monomer recompacted to its new sequence-order layout slot");
    }
}

static void test_deleteMonomerEndNoReconnect() {
    std::printf("--- Test: deleteMonomer on an end monomer removes its bond with no reconnection ---\n");
    BiopolymerSequenceView view;
    view.buildSequenceView(QStringLiteral("ACD"), QStringLiteral("PEPTIDE"));
    int lastId = view.monomers().last().id;
    view.deleteMonomer(lastId);
    CHECK(view.monomers().size() == 2, "2 monomers left");
    CHECK(view.bonds().size() == 1, "1 bond left (A-C), nothing to reconnect");
}

static void test_deleteMonomerOnlyLeavesEmpty() {
    std::printf("--- Test: deleteMonomer on the only monomer leaves an empty view ---\n");
    BiopolymerSequenceView view;
    view.addMonomer(QStringLiteral("A"), QStringLiteral("PEPTIDE"));
    int id = view.monomers().first().id;
    view.deleteMonomer(id);
    CHECK(view.monomers().isEmpty(), "empty");
    CHECK(view.bonds().isEmpty(), "no bonds");
}

static void test_deleteMonomerUnknownIdIsNoOp() {
    std::printf("--- Test: deleteMonomer with an unknown id is a no-op ---\n");
    BiopolymerSequenceView view;
    view.buildSequenceView(QStringLiteral("AC"), QStringLiteral("PEPTIDE"));
    view.deleteMonomer(99999);
    CHECK(view.monomers().size() == 2, "count unchanged");
}

struct ExpectedMonomerTemplate {
    const char* seqType;
    const char* code;
    const char* alias;
    const char* monomerClass;
    bool ambiguous;
};

// Transcribed INDEPENDENTLY from src/worker/70-biopolymer.js:14-74 a second time (not copy-pasted
// from the production table) so a copy-paste error there isn't just echoed back at itself.
static const ExpectedMonomerTemplate kExpectedTemplates[] = {
    {"PEPTIDE", "A", "Ala", "AminoAcid", false},
    {"PEPTIDE", "C", "Cys", "AminoAcid", false},
    {"PEPTIDE", "D", "Asp", "AminoAcid", false},
    {"PEPTIDE", "E", "Glu", "AminoAcid", false},
    {"PEPTIDE", "F", "Phe", "AminoAcid", false},
    {"PEPTIDE", "G", "Gly", "AminoAcid", false},
    {"PEPTIDE", "H", "His", "AminoAcid", false},
    {"PEPTIDE", "I", "Ile", "AminoAcid", false},
    {"PEPTIDE", "K", "Lys", "AminoAcid", false},
    {"PEPTIDE", "L", "Leu", "AminoAcid", false},
    {"PEPTIDE", "M", "Met", "AminoAcid", false},
    {"PEPTIDE", "N", "Asn", "AminoAcid", false},
    {"PEPTIDE", "P", "Pro", "AminoAcid", false},
    {"PEPTIDE", "Q", "Gln", "AminoAcid", false},
    {"PEPTIDE", "R", "Arg", "AminoAcid", false},
    {"PEPTIDE", "S", "Ser", "AminoAcid", false},
    {"PEPTIDE", "T", "Thr", "AminoAcid", false},
    {"PEPTIDE", "V", "Val", "AminoAcid", false},
    {"PEPTIDE", "W", "Trp", "AminoAcid", false},
    {"PEPTIDE", "Y", "Tyr", "AminoAcid", false},
    {"PEPTIDE", "B", "Asx", "AminoAcid", true},
    {"PEPTIDE", "Z", "Glx", "AminoAcid", true},
    {"PEPTIDE", "J", "Xle", "AminoAcid", true},
    {"PEPTIDE", "X", "Xaa", "AminoAcid", true},
    {"RNA", "A", "A", "Base", false},
    {"RNA", "C", "C", "Base", false},
    {"RNA", "G", "G", "Base", false},
    {"RNA", "U", "U", "Base", false},
    {"RNA", "N", "N", "Base", true},
    {"RNA", "B", "B", "Base", true},
    {"RNA", "V", "V", "Base", true},
    {"RNA", "D", "D", "Base", true},
    {"RNA", "H", "H", "Base", true},
    {"RNA", "K", "K", "Base", true},
    {"RNA", "M", "M", "Base", true},
    {"RNA", "W", "W", "Base", true},
    {"RNA", "Y", "Y", "Base", true},
    {"RNA", "R", "R", "Base", true},
    {"RNA", "S", "S", "Base", true},
    {"DNA", "A", "dA", "Base", false},
    {"DNA", "C", "dC", "Base", false},
    {"DNA", "G", "dG", "Base", false},
    {"DNA", "T", "dT", "Base", false},
    {"DNA", "N", "N", "Base", true},
    {"DNA", "B", "B", "Base", true},
    {"DNA", "V", "V", "Base", true},
    {"DNA", "D", "D", "Base", true},
    {"DNA", "H", "H", "Base", true},
    {"DNA", "K", "K", "Base", true},
    {"DNA", "M", "M", "Base", true},
    {"DNA", "W", "W", "Base", true},
    {"DNA", "Y", "Y", "Base", true},
    {"DNA", "R", "R", "Base", true},
    {"DNA", "S", "S", "Base", true},
};

static void test_exhaustiveMonomerTemplateTable() {
    std::printf("--- Test: exhaustive monomer-template table check (all %zu codes) ---\n",
                 sizeof(kExpectedTemplates) / sizeof(kExpectedTemplates[0]));
    for (const ExpectedMonomerTemplate& exp : kExpectedTemplates) {
        BiopolymerSequenceView view;
        view.buildSequenceView(QString::fromLatin1(exp.code), QString::fromLatin1(exp.seqType));
        QList<BioMonomer> mons = view.monomers();
        QString label = QStringLiteral("%1/%2").arg(exp.seqType).arg(exp.code);
        if (mons.size() != 1) {
            CHECK(false, qPrintable(QStringLiteral("%1: expected exactly 1 monomer").arg(label)));
            continue;
        }
        CHECK(mons[0].alias == QString::fromLatin1(exp.alias),
              qPrintable(QStringLiteral("%1: alias matches").arg(label)));
        CHECK(mons[0].monomerClass == QString::fromLatin1(exp.monomerClass),
              qPrintable(QStringLiteral("%1: class matches").arg(label)));
        CHECK(mons[0].ambiguous == exp.ambiguous,
              qPrintable(QStringLiteral("%1: ambiguous flag matches").arg(label)));
    }
}

int main() {
    test_buildSequenceViewPeptide();
    test_buildSequenceViewStripsFastaHeaderAndNonLetters();
    test_buildSequenceViewSkipsUnknownWithoutLayoutGap();
    test_buildSequenceViewReplacesNotAppends();
    test_buildSequenceViewProteinSynonym();
    test_buildSequenceViewRnaAndDna();
    test_buildSequenceViewUnrecognizedSeqTypeStoresVerbatim();
    test_buildSequenceViewEmptyAfterStripping();
    test_buildSequenceViewTrueEmptyIsNoOp();
    test_addMonomerAppendsAndBonds();
    test_addMonomerOnEmptyViewNoBond();
    test_addMonomerOmittedSeqTypeKeepsCurrent();
    test_addMonomerUnknownSymbolIsNoOp();
    test_deleteMonomerMiddleReconnects();
    test_deleteMonomerEndNoReconnect();
    test_deleteMonomerOnlyLeavesEmpty();
    test_deleteMonomerUnknownIdIsNoOp();
    test_exhaustiveMonomerTemplateTable();
    std::printf("Summary: %d passed, %d failed.\n", g_pass, g_fail);
    return g_fail == 0 ? 0 : 1;
}
