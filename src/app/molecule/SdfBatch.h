// src/app/molecule/SdfBatch.h
#ifndef SDFBATCH_H
#define SDFBATCH_H

// Staged, browsable list of imported structures (chem-core.js migration, sub-project 5c; see
// docs/superpowers/specs/2026-08-03-sdf-batch-cpp-design.md). C++ replacement for
// 40-serialize.js's module-level _sdfBatchRecords array -- entirely separate from the live
// document (DocumentState/EditableMolecule); a caller promotes one staged record into the real
// document via the ALREADY-EXISTING DocumentState::deserializeMol(batch.molfileAt(index)), no new
// DocumentState code needed at all.
//
// Records are stored as MOL-format TEXT, not live EditableMolecule objects -- EditableMolecule is
// neither copyable nor movable (deleted copy constructor, and a user-declared destructor
// suppresses the implicit move operations too), so a QList<EditableMolecule> is not viable.

#include <QString>
#include <QStringList>
#include <QList>
#include <QHash>
#include <optional>

class SdfBatch {
public:
    struct ThumbnailAtom { double x, y; QString label; };
    struct ThumbnailBond { double x1, y1, x2, y2; int type; };
    struct Thumbnail { QList<ThumbnailAtom> atoms; QList<ThumbnailBond> bonds; };

    SdfBatch();
    ~SdfBatch();

    SdfBatch(const SdfBatch&) = delete;
    SdfBatch& operator=(const SdfBatch&) = delete;

    // 40-serialize.js's deserializeRdfBatch AND deserializeIndigoBatch -- both real JS functions
    // have byte-identical bodies (confirmed by direct source read: same logic, same reqId,
    // different call sites only), so this one method serves both. No properties (this path never
    // has SDF tag data). Unparseable entries are skipped, not aborted. Replaces every existing
    // record unconditionally, even if the result is empty -- returns false only if `molfiles`
    // itself is empty.
    bool loadFromMolfileList(const QStringList& molfiles);

    int recordCount() const;
    QString molfileAt(int index) const;
    QString labelAt(int index) const;
    QHash<QString, QString> propsAt(int index) const;
    Thumbnail thumbnailAt(int index) const;

private:
    struct BatchRecord {
        QString molfileText;
        QString label;
        QHash<QString, QString> props;
        Thumbnail thumbnail;
    };

    unsigned long long m_session = 0;
    QList<BatchRecord> m_records;

    void activateSession() const;

    // Core helper, given an ALREADY-VALID molecule handle. Does NOT free `handle` -- that's the
    // caller's job. Captures molfile text via indigoMolfile(handle), label via indigoName(handle)
    // (falling back to "Record " + (indexForLabel+1) if empty), properties via
    // indigoIterateProperties(handle) if withProps, and a thumbnail (via a transient
    // EditableMolecule built from the captured molfile text) if computeThumbnail is true.
    BatchRecord ingestFromHandle(int handle, int indexForLabel, bool withProps, bool computeThumbnail) const;

    // Parses `text` via indigoLoadMoleculeFromString, and on success delegates to
    // ingestFromHandle (withProps=false -- this path never has SDF tags) then frees the parse
    // handle. Returns std::nullopt on a parse failure.
    std::optional<BatchRecord> ingestFromText(const QString& text, int indexForLabel, bool computeThumbnail) const;
};

#endif // SDFBATCH_H
