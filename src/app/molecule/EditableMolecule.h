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
//
// Query atom lists (spec risk 3): plain molecule handles REJECT V2000 atom
// lists outright -- indigoLoadMoleculeFromString returns an error ("atom
// lists are allowed only for queries"); only indigoLoadQueryMoleculeFromString
// accepts them (spike: editable_molecule_test Test 6). Consequence: query
// atom lists cannot live on the plain Indigo handle this class wraps -- they
// must join ExtensionData (or a separate query-molecule-backed type) in a
// later sub-project when query-feature editing is designed.
//
// SGroup-atom-membership (sub-project 3a design question): spike findings
// (tests/editable_molecule_test.cpp Test 11, alanine-free CCO + a 2-atom data
// sgroup): "indigoIterateAtoms(sgroupHandle) iterator=valid, atom count=2" --
// calling indigoIterateAtoms on the SGROUP handle itself (not the molecule
// handle) DOES work and enumerates exactly that sgroup's member atoms, so
// sgroup-atom membership IS queryable through this path, contrary to this
// header's original open question. "indigoRemove(sgroupHandle) returned 1;
// atom count before=3 after=3" -- removing a sgroup handle succeeds and only
// detaches the sgroup record; the molecule's atom count is unchanged, i.e.
// indigoRemove does NOT cascade-delete a sgroup's member atoms. Consequence
// for Task 7's deleteAtom: deleting a sgroup-member atom does not need to
// special-case sgroup cleanup for atom survival (removing the atom directly
// via the existing removeAtom path is safe and does not corrupt the sgroup
// record's atom list on its own), but this class does NOT yet update a
// sgroup's atom-index list when one of its member atoms is removed via
// removeAtom -- that bookkeeping gap is a documented, explicit limitation
// carried into a later sub-project, not attempted here.

#include <QString>
#include <QHash>
#include <QList>
#include "ExtensionData.h"
#include "MoleculeSnapshot.h"

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

    // Atom/bond CRUD with stable never-reused external IDs
    AtomId addAtom(const QString& symbol, double x, double y);
    bool removeAtom(AtomId id);
    BondId addBond(AtomId a, AtomId b, int order);
    bool removeBond(BondId id);
    QString atomSymbol(AtomId id) const;
    int atomCharge(AtomId id) const;
    bool setAtomCharge(AtomId id, int charge);
    int atomIsotope(AtomId id) const;
    bool setAtomIsotope(AtomId id, int isotope);
    int atomRadical(AtomId id) const;
    bool setAtomRadical(AtomId id, int radical);
    int atomExplicitValence(AtomId id) const;
    bool setAtomExplicitValence(AtomId id, int valence);
    bool setAtomLabel(AtomId id, const QString& newSymbol);
    bool setBondOrderValue(BondId id, int order);
    int atomAttachmentOrder(AtomId id) const;
    bool setAtomAttachmentOrder(AtomId id, int order);
    bool atomPos(AtomId id, double& x, double& y) const;
    int bondOrder(BondId id) const;
    QList<AtomId> atomIds() const;
    QList<BondId> bondIds() const;

    // Extension data storage accessors
    void setName(const QString& n);
    QString name() const;
    int addTextAnnotation(double x, double y, const QString& content);
    int textAnnotationCount() const;
    int addRxnArrow(double x1, double y1, double x2, double y2);
    int rxnArrowCount() const;
    int addRxnPlus(double x, double y);
    int rxnPlusCount() const;
    int addMultitailArrow(const QList<double>& headAndTails);
    int multitailArrowCount() const;
    int addImage(double x, double y, double w, double h, const QByteArray& pngData);
    int imageCount() const;
    bool removeTextAnnotation(TextId id);
    bool removeRxnArrow(RxnArrowId id);
    bool removeRxnPlus(RxnPlusId id);
    bool removeMultitailArrow(MultitailArrowId id);
    bool removeImage(ImageId id);
    QList<RxnArrowId> rxnArrowIds() const;
    QList<RxnPlusId> rxnPlusIds() const;
    QList<MultitailArrowId> multitailArrowIds() const;
    void setStereoFlag(int fragmentIndex, int flag);
    int stereoFlag(int fragmentIndex) const;
    void setAtomAAM(AtomId id, int mapNumber);
    int atomAAM(AtomId id) const;
    void setAtomCheckWarning(AtomId id, bool warn);
    bool atomCheckWarning(AtomId id) const;

    int addDataSGroup(const QList<AtomId>& atoms, const QString& description, const QString& data);
    int dataSGroupCount() const;

    MoleculeSnapshot snapshot() const;
    bool restore(const MoleculeSnapshot&);

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

    ExtensionData m_ext;

    void activateSession() const; // indigoSetSessionId(m_session)
    void rebuildIndexTables();
};

#endif // EDITABLEMOLECULE_H
