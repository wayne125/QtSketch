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
//
// Bond stereo (sub-project 3a design question): spike findings
// (tests/editable_molecule_test.cpp Test 12, alanine CC(N)C(=O)O loaded from
// a plain SMILES with no 2D coordinates): "stereocenter type before
// indigoAddStereocenter: 0" (none detected on load, as expected -- SMILES
// carries no wedge/dash). "indigoAddStereocenter(ABS) returned 1" (the call
// to force a specific tetrahedral arrangement at the stereocenter succeeds).
// "indigoMarkStereobonds returned 0" and "bonds with nonzero
// indigoBondStereo() after marking: 0" -- despite the forced stereocenter,
// NOT ONE incident bond ended up with a nonzero (wedge/dash) value from the
// read-only indigoBondStereo() query. Consequence: indigoAddStereocenter +
// indigoMarkStereobonds does NOT translate into a settable per-bond
// wedge/dash through this path on a molecule with no 2D layout to derive a
// direction from (plausible root cause: wedge/dash assignment needs existing
// atom coordinates to pick which bond to mark, which this spike's molecule
// never had). Bond stereo (wedge/dash) is therefore NOT supported by
// changeBondOrder in this sub-project; it stays an explicit, documented gap
// until serialization (sub-project 5) or a dedicated investigation with a
// coordinate-bearing molecule finds a working mechanism.
//
// Bond stereo INVERT (sub-project 3b design question): spike findings
// (tests/editable_molecule_test.cpp Test 14, a V2000 molfile WITH real 2D
// coordinates AND an explicit wedge flag in bond-block column 4 -- the setup
// 3a's Test 12 lacked):
//   "indigoBondStereo(bond0) as loaded = 5"
//   "indigoInvertStereo(bond0) returned -1 (REJECTED)"
//   "invert error: core: indigoInvertStereo: not a stereobond"
//   "indigoBondStereo(bond0) after 1 invert = 5"
//   "indigoBondStereo(bond0) after 2 inverts = 5 (round-trip OK)"
// Two consequences, both load-bearing:
// (1) A wedge bond IS readable -- unlike 3a's coordinate-less SMILES case,
//     indigoBondStereo reports a nonzero value once the molecule has real 2D
//     coordinates and an explicit molfile wedge flag. But the value is
//     Indigo's OWN enum, not the MDL molfile code: INDIGO_UP = 5 and
//     INDIGO_DOWN = 6 (indigo.h:439-440), whereas MDL/chem-core.js use 1 for
//     wedge-up and 6 for hash-down. The two encodings collide on 6 and
//     disagree on up, so any future code bridging them MUST translate rather
//     than pass the number through.
// (2) indigoInvertStereo REJECTS a wedge bond outright ("not a stereobond" --
//     it covers cis/trans stereobonds and stereocenters, not wedge/dash
//     direction). Combined with indigoBondStereo being read-only and 3a's
//     Test 12 ruling out the stereocenter-inference path, there is NO way to
//     change a bond's wedge/dash through the Indigo C API as vendored here.
// Therefore: DocumentState's flip transforms apply the coordinate mirror ONLY.
// chem-core.js's transformSelection also swaps wedge stereo (1 <-> 6) for
// bonds fully inside the selection so the depiction stays chemically
// consistent; that swap is an explicit, documented gap, not an oversight.
//
// Atom-merge mechanics (sub-project 3c design question): spike findings
// (tests/editable_molecule_test.cpp Test 16): (1) a raw indigoRemove(atom)
// DOES auto-remove that atom's incident bonds -- bond count dropped from 1
// to 0 after removing one of the bonded pair, with no separate bond-removal
// call. (2) In the ordinary (non-seam) case, creating the replacement bond
// onto the kept atom BEFORE removing the doomed atom succeeds, and the
// order passed to indigoAddBond is read back unchanged (no surprise there).
// (3) In the seam case -- kept already bonded to the same target -- creating
// the would-be duplicate bond is REJECTED ("already have edge between
// vertices"), exactly 3b's GT-T5 finding, and the pre-existing seam bond's
// order is left completely unchanged by the attempt or by the subsequent
// removal of the doomed atom. Consequence: mergeOverlappingAtoms can
// unconditionally try to create each replacement bond BEFORE removing the
// doomed atom, treat a negative return as the expected seam-dedupe outcome
// (not an error), and rely on removeAtom's cascade to clean up whatever of
// doomed's original bonds remain (either the ones that were never
// successfully rewired, or the ones that were, since the original bond
// object is a separate one from the replacement).

#include <QString>
#include <QHash>
#include <QList>
#include <QPointF>
#include <functional>
#include "ExtensionData.h"
#include "MoleculeSnapshot.h"

using AtomId = int;
using BondId = int;
using SGroupId = int;

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
    bool setAtomQueryList(AtomId id, const QString& labelsCsv, bool notList);
    bool clearAtomQueryList(AtomId id, const QString& fallbackLabel);
    bool hasAtomQueryList(AtomId id) const;
    QList<int> atomQueryListNumbers(AtomId id) const;
    bool atomQueryListIsNotList(AtomId id) const;
    bool atomPos(AtomId id, double& x, double& y) const;
    bool setAtomPos(AtomId id, double x, double y);
    int bondOrder(BondId id) const;
    bool bondEndpoints(BondId id, AtomId& a, AtomId& b) const;
    BondId findBond(AtomId a, AtomId b) const;   // -1 if no such bond
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
    bool rxnArrowEndpoints(RxnArrowId id, double& x1, double& y1, double& x2, double& y2) const;
    bool setRxnArrowEndpoints(RxnArrowId id, double x1, double y1, double x2, double y2);
    bool rxnPlusPos(RxnPlusId id, double& x, double& y) const;
    bool setRxnPlusPos(RxnPlusId id, double x, double y);
    QList<double> multitailArrowPoints(MultitailArrowId id) const;
    bool setMultitailArrowPoints(MultitailArrowId id, const QList<double>& points);
    bool imageRect(ImageId id, double& x, double& y, double& w, double& h) const;
    bool setImageRect(ImageId id, double x, double y, double w, double h);
    void setStereoFlag(int fragmentIndex, int flag);
    int stereoFlag(int fragmentIndex) const;
    void setAtomAAM(AtomId id, int mapNumber);
    int atomAAM(AtomId id) const;
    void setAtomCheckWarning(AtomId id, bool warn);
    bool atomCheckWarning(AtomId id) const;

    // Document-level stereo-display toggle (_struct.stereoFlags = {type, groupId}), genuinely
    // consumed by MainToolbar.qml's ABS/REL combo box. Deliberately SEPARATE from the
    // per-fragment stereoFlag/setStereoFlag above, which models an unrelated, real chem-core
    // concept (a per-fragment ABS/AND/OR/MIXED value that is actually COMPUTED from atom stereo
    // labels, never stored) -- reusing that field's QHash<int,int> shape here would be wrong: it
    // cannot represent one document-wide {type, groupId} pair. Read-only: the setter
    // (setStereoFlags, src/worker/50-reactions.js) is an undoable editing operation, out of scope
    // here (sub-project 3e).
    QString stereoFlagsType() const;
    int stereoFlagsGroupId() const;

    // Real string check-warning TYPE (e.g. "valence", set by external structure-check results --
    // src/worker/60-analysis.js's setCheckIssues), NOT the same as the boolean
    // atomCheckWarning/setAtomCheckWarning above (sub-project 1 mismodeled the real field as a
    // yes/no flag). Bonds have no check-warning storage at all before this. Read-only for the
    // same reason as stereoFlagsType.
    QString atomCheckWarningText(AtomId id) const;
    QString bondCheckWarningText(BondId id) const;

    // Reaction-arrow display fields buildRenderPrimitives reads but sub-project 1 never stored:
    // display mode, above/below condition text, and optional curvature control point. Read-only:
    // setRxnArrowMode/setRxnArrowConditions (src/worker/50-reactions.js) are undoable editing
    // operations, out of scope here (sub-project 3e).
    QString rxnArrowMode(RxnArrowId id) const;
    QString rxnArrowConditionsAbove(RxnArrowId id) const;
    QString rxnArrowConditionsBelow(RxnArrowId id) const;
    bool rxnArrowCurvature(RxnArrowId id, double& x, double& y) const;

    // Pure read-only Indigo queries -- no mutation. implicitHydrogenCount wraps
    // indigoCountHydrogens; neighborAtomIds wraps this class's own bond-endpoint
    // bookkeeping (not a raw Indigo neighbor-iteration call, since AtomId is our own stable id,
    // not Indigo's raw index).
    int implicitHydrogenCount(AtomId id) const;
    QList<AtomId> neighborAtomIds(AtomId id) const;

    // One SSSR ring per entry, atom/bond ids in the ring object's own iteration order.
    // Confirmed by direct probe: SSSR ring objects support indigoIterateAtoms/indigoIterateBonds
    // directly, exactly like a real molecule handle -- no atom-pair-to-bond re-derivation needed.
    struct RingMembership {
        QList<AtomId> atoms;
        QList<BondId> bonds;
    };
    QList<RingMembership> ringMembership() const;

    int addDataSGroup(const QList<AtomId>& atoms, const QString& description, const QString& data);
    int dataSGroupCount() const;

    MoleculeSnapshot snapshot() const;
    bool restore(const MoleculeSnapshot&);

    // Re-expresses fuseOverlappingAtoms (10-state.js:179-228) in terms Indigo
    // actually supports: Indigo has no "reassign a bond's endpoints" API, so a
    // coincident-atom merge is done as capture-incident-bonds, try to create
    // each as a replacement bond onto the kept atom, then remove the doomed
    // atom (removeAtom's cascade cleans up whatever of doomed's original
    // bonds remain). See the header's "Atom-merge mechanics" spike paragraph
    // for the three Indigo behaviors this depends on.
    struct MergeResult {
        QHash<AtomId, AtomId> mergedAway;   // doomed id -> kept id
        QList<BondId> createdBonds;         // NEW replacement bonds created while rewiring
    };
    MergeResult mergeOverlappingAtoms(double tolerance = 0.1);

    // Rewires `doomed`'s incident bonds onto `kept` (creating each as a new
    // bond, or dropping it as a rejected-seam parallel edge), then removes
    // `doomed`. Returns the newly created replacement bonds. Extracted from
    // mergeOverlappingAtoms's own per-pair body (sub-project 3c) so
    // insertFunctionalGroup's explicit-substitution graft (sub-project 3d)
    // can reuse the identical mechanism instead of duplicating it.
    QList<BondId> graftAtomOnto(AtomId doomed, AtomId kept);

    // Non-undoable document-seeding helper (30-templates.js's addBenzeneRing)
    // -- NOT a DocumentState command; mutates directly and has no history
    // entry, matching the real function exactly.
    QList<AtomId> addBenzeneRing(double cx, double cy);

    // Re-expresses chem-core's tempStruct.mergeInto(_struct, ..., aidMap) in
    // terms of indigoMerge, whose own return value is a plain status code,
    // not an id mapping. Confirmed by direct probe against the real
    // indigo.dll (not assumed): indigoMerge appends the source's atoms/bonds
    // to this molecule in the source's OWN iteration order, and automatically
    // propagates any SUP sgroups (with attachment points) the source carries.
    // Before/after diffing therefore correctly discovers every created id,
    // including sgroups -- no manual sgroup reconstruction is needed.
    //
    // Takes Molfile TEXT, not a raw Indigo handle: TemplateLibrary owns its
    // own Indigo session (separate from this EditableMolecule's), and a raw
    // handle is a small integer scoped to whichever session created it --
    // confirmed by direct probe that reusing one across sessions either fails
    // or silently aliases an unrelated object. TemplateLibrary::molfileText()
    // is the confirmed-safe way to cross that boundary.
    struct InsertResult {
        QList<AtomId> createdAtoms;
        QList<BondId> createdBonds;
        QList<SGroupId> createdSGroups;
        QHash<int, AtomId> sourceIndexToNewAtomId;   // source molecule's internal atom index -> new AtomId
    };
    InsertResult insertStructure(const QString& sourceMolfile,
                                  const std::function<QPointF(double, double)>& transform);

    // Superatom (SUP sgroup) support. A new stable, never-reused SGroupId
    // (like AtomId/BondId) rather than the existing addDataSGroup precedent's
    // raw, reusable indigoIndex() -- these ARE looked up again later
    // (toggleSgroupExpanded), so a reused index could alias a different,
    // newer sgroup. There is no public constructor: insertStructure discovers
    // and assigns ids for merge-propagated sgroups internally, and nothing in
    // this sub-project's scope needs to build one from bare atoms.
    bool superatomAttachAtom(SGroupId id, AtomId& out) const;   // first attachment point's atom, if any
    void setSGroupExpanded(SGroupId id, bool expanded);
    bool sgroupExpanded(SGroupId id) const;

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

    QHash<SGroupId, int> m_sgroupIdx;   // external stable ID -> indigo superatom index
    SGroupId m_nextSGroupId = 1;        // monotonic, never reused
    QHash<int, bool> m_sgroupExpanded;  // SGroupId -> expanded flag (Indigo has no such concept)

    void activateSession() const; // indigoSetSessionId(m_session)
    void rebuildIndexTables();
};

#endif // EDITABLEMOLECULE_H
