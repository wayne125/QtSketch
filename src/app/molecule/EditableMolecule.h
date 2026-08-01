// src/app/molecule/EditableMolecule.h
#ifndef EDITABLEMOLECULE_H
#define EDITABLEMOLECULE_H

// Long-lived editable wrapper around one Indigo molecule handle -- the C++
// replacement for chem-core.js's Struct (sub-project 1 of the migration; see
// docs/superpowers/specs/2026-08-01-editable-molecule-cpp-design.md).
//
// ID design: public AtomId/BondId are monotonic, NEVER-reused ints mapped to
// Indigo's internal indices via QHash. Chosen regardless of Indigo's own
// index behavior so that IDs held in undo snapshots can never alias a
// later-created atom.
// Spike findings (tests/editable_molecule_test.cpp, Test 0):
//   "Spike finding (tests/editable_molecule_test.cpp Test 0): surviving atoms keep their
//   pre-removal indices after another atom is removed, BUT a newly-added atom CAN reuse a
//   just-freed index (observed: freed index 2 was reused by the next add). This is exactly
//   why AtomId/BondId are a separate never-reused counter, not raw Indigo indices -- reuse
//   would let an old undo-snapshot's ID silently alias a different, newer atom."
// Verified hard requirement: indigoClone preserves atom/bond indices.

#include <QString>
#include <QHash>

using AtomId = int;
using BondId = int;

struct StringResult {
    bool success = false;
    QString value;
    QString error;
};

class EditableMolecule {
public:
    // initialStructure: molfile or SMILES (indigoLoadMoleculeFromString
    // auto-detects); empty string = new blank molecule.
    explicit EditableMolecule(const QString& initialStructure = {});
    ~EditableMolecule();

    EditableMolecule(const EditableMolecule&) = delete;
    EditableMolecule& operator=(const EditableMolecule&) = delete;

    bool isValid() const { return m_mol >= 0; }
    QString lastError() const { return m_lastError; }

    int atomCount() const;
    int bondCount() const;
    StringResult toMolfile() const;

private:
    // One Indigo session per instance, held for the object's lifetime
    // (unlike IndigoService's alloc/release-per-call one-shot pattern).
    unsigned long long m_session = 0;
    int m_mol = -1;               // Indigo molecule handle, -1 = invalid
    QString m_lastError;

    QHash<AtomId, int> m_atomIdx; // external stable ID -> indigo atom index
    QHash<BondId, int> m_bondIdx; // external stable ID -> indigo bond index
    AtomId m_nextAtomId = 1;      // monotonic, never reused
    BondId m_nextBondId = 1;

    void activateSession() const; // indigoSetSessionId(m_session)
};

#endif // EDITABLEMOLECULE_H
