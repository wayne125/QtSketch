// src/app/molecule/BiopolymerSequenceView.h
#ifndef BIOPOLYMERSEQUENCEVIEW_H
#define BIOPOLYMERSEQUENCEVIEW_H

// Hand-rolled, chem-core/Indigo-independent biopolymer sequence view (chem-core.js migration,
// sub-project 6d; see docs/superpowers/specs/2026-08-04-biopolymer-sequence-view-cpp-design.md).
// Ports src/worker/70-biopolymer.js in full. Deliberately touches NO EditableMolecule/Indigo
// state at all -- the real file's own header comment explains why: a v1 that reused chem-core's
// own DrawingEntitiesManager was abandoned because several of its submodules were stub-empty
// Proxies. This is just plain position/label/class/connectivity data for a read-only display.
// None of its 3 mutating methods are undoable -- the real functions have no makeCmd/
// executeCommand call anywhere in this file either.

#include <QString>
#include <QList>
#include <QHash>

struct BioMonomer {
    int id = 0;
    QString label;         // single-letter code, e.g. "A"
    QString alias;         // e.g. "Ala" (peptide) or "A"/"dA" (RNA/DNA)
    double x = 0, y = 0;
    QString monomerClass;  // "AminoAcid" or "Base"
    bool ambiguous = false;
};

struct BioBond {
    int fromId = 0;
    int toId = 0;
};

class BiopolymerSequenceView {
public:
    // bioBuildSequenceView: full replace (clears first, unconditionally). No-op body (leaves the
    // just-cleared empty state) if sequenceText is empty. seqType() is set to the normalized
    // value (empty->"PEPTIDE", uppercased, "PROTEIN"->"PEPTIDE") UNCONDITIONALLY, even if that
    // value isn't a recognized table -- the character-lookup table independently falls back to
    // PEPTIDE in that case, but the STORED seqType is never coerced. Unknown residue symbols are
    // silently skipped (no logging infrastructure equivalent needed for a to-be-wired-later read
    // model).
    void buildSequenceView(const QString& sequenceText, const QString& seqType);

    // bioAddMonomer: appends one monomer to the end of the CURRENT list. symbol's FIRST character
    // (trimmed, uppercased) is looked up. seqType empty means "keep the current seqType()".
    // No-op entirely (nothing added, seqType() unchanged) if the resolved symbol isn't in the
    // selected table. On success, seqType() is updated the same verbatim-normalized way as
    // buildSequenceView.
    void addMonomer(const QString& symbol, const QString& seqType = QString());

    // bioDeleteMonomer: no-op if id doesn't exist. Reconnects the deleted monomer's previous and
    // next neighbors (if both existed) with a new bond, then recompacts every remaining
    // monomer's x/y to the snake layout based on its NEW sequence position.
    void deleteMonomer(int id);

    QList<BioMonomer> monomers() const;
    QList<BioBond> bonds() const;
    QString seqType() const;

private:
    struct MonomerTemplate {
        QString alias;
        QString monomerClass;
        bool ambiguous = false;
    };

    static QString normalizeSeqType(const QString& seqType, const QString& fallback);
    static const QHash<QString, MonomerTemplate>& tableFor(const QString& normalizedType);
    void recompactLayout();

    int m_nextId = 1;
    QList<BioMonomer> m_monomers;
    QList<BioBond> m_bonds;
    QString m_seqType = QStringLiteral("PEPTIDE");

    static constexpr int kRowLen = 20;
    static constexpr double kCellWidth = 60.0;
};

#endif // BIOPOLYMERSEQUENCEVIEW_H
