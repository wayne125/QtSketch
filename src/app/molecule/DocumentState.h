// src/app/molecule/DocumentState.h
#ifndef DOCUMENTSTATE_H
#define DOCUMENTSTATE_H

// One per open document: owns the EditableMolecule, the undo/redo history,
// and the current selection. C++ replacement for src/worker/10-state.js's
// module-level _struct/_history/_historyPointer/_selection/_dirty state
// (sub-project 2 of the chem-core.js migration; see
// docs/superpowers/specs/2026-08-01-document-state-cpp-design.md).
//
// executeCommand/undo/redo are a direct line-for-line port of the three real
// JS functions of the same name, confirmed by direct source read: truncate
// any redo tail, run/invert the command, cap history at kHistorySize
// (dropping the oldest and compensating the pointer), clear selection on
// undo/redo.

#include "EditableMolecule.h"
#include "EditCommand.h"
#include "SelectionState.h"
#include <vector>

class DocumentState {
public:
    explicit DocumentState(const QString& initialStructure = {});

    EditableMolecule& molecule();

    void executeCommand(EditCommand cmd);
    void undo();
    void redo();
    bool isDirty() const;
    // Grounded requirement (not speculative): src/worker/30-templates.js's
    // loadBenzene() explicitly does `init(); addBenzeneRing(); _dirty = false`
    // -- a new document pre-populated with the starter benzene is NOT dirty.
    void markClean();
    bool canUndo() const;
    bool canRedo() const;

    void selectAtom(AtomId id);
    void selectBond(BondId id);
    void selectRxnArrow(RxnArrowId id);
    void selectRxnPlus(RxnPlusId id);
    void selectMultitailArrow(MultitailArrowId id);
    void clearSelection();

    void addAtomToSelection(AtomId id);
    void addBondToSelection(BondId id);
    void addRxnArrowToSelection(RxnArrowId id);
    void addRxnPlusToSelection(RxnPlusId id);
    void addMultitailArrowToSelection(MultitailArrowId id);

    void removeAtomFromSelection(AtomId id);
    void removeBondFromSelection(BondId id);
    void removeRxnArrowFromSelection(RxnArrowId id);
    void removeRxnPlusFromSelection(RxnPlusId id);
    void removeMultitailArrowFromSelection(MultitailArrowId id);

    // Selects every current atom/bond/rxnArrow/rxnPlus/multitailArrow. KNOWN
    // SIMPLIFICATION: the real 10-state.js selectAll() excludes atoms hidden
    // inside a contracted SUP functional-group sgroup (selecting the sgroup's
    // own pill id instead) -- that needs sgroup-contraction awareness
    // (type/isExpanded) EditableMolecule doesn't have yet. This selectAll()
    // selects raw atoms/bonds with no sgroup-contraction logic; revisit once
    // a later sub-project models contracted sgroups.
    void selectAll();
    SelectionState& selection();

    AtomId addAtom(const QString& symbol, double x, double y);
    BondId addBond(AtomId a, AtomId b, int order);
    void deleteAtom(AtomId id);   // plain-atom-and-incident-bonds case only; see the
                                  // sgroup-membership spike documented in EditableMolecule.h
                                  // for why the sgroup-aware branch is deferred
    void deleteBond(BondId id);

    void changeAtomLabel(AtomId id, const QString& newLabel);
    void setAtomMapping(AtomId id, int mappingNumber);
    void changeAtomCharge(AtomId id, int newCharge);
    void setAttachmentPoint(AtomId id, int order);
    void changeAtomIsotope(AtomId id, int isotope);
    void changeAtomRadical(AtomId id, int radical);
    void changeAtomValence(AtomId id, int valence);

    struct AtomProperties { QString label; int charge; int isotope; int radical; int explicitValence; };
    AtomProperties atomProperties(AtomId id) const;

    void changeBondOrder(BondId id, int newOrder);

    void setAtomQueryList(AtomId id, const QString& labelsCsv, bool notList);
    // KNOWN LIMITATION: undo after clearAtomQueryList does NOT restore the
    // original query list (EditableMolecule has no set-list-from-numbers
    // entry point bypassing the CSV parse) -- it only re-clears on redo.
    // Revisit if a later sub-project needs this restored exactly.
    void clearAtomQueryList(AtomId id, const QString& fallbackLabel);

    // Discrete transforms of the selected ATOMS about their unweighted
    // centroid -- a direct port of 20-edit.js's transformSelection(mode),
    // split into four named methods instead of its JS mode-string dispatch.
    // Each no-ops below 2 selected atoms, matching the real ids.length < 2
    // guard, and pushes exactly one undoable command.
    void rotateSelection90CW();
    void rotateSelection90CCW();
    void flipSelectionHorizontal();
    void flipSelectionVertical();

    // Per-image transforms (20-edit.js has none; these port 50-reactions.js's
    // moveImage/resizeImage). Take an explicit id rather than reading the
    // selection -- images are deliberately not a SelectionState entity type.
    // Both commit immediately as one undoable command; there is no live-drag
    // protocol for images in the real code either.
    void moveImage(ImageId id, double dx, double dy);
    void resizeImage(ImageId id, double scaleFactor);

    // ---- Live-drag gestures ------------------------------------------------
    // Repeated *Live calls during a UI drag mutate positions directly and push
    // NOTHING onto the undo history; the matching commit* call at gesture end
    // pushes exactly one undoable command covering the whole gesture. Direct
    // port of 20-edit.js's moveSelection/commitMove pair.
    //
    // NOTE the deliberate per-gesture asymmetry, ported exactly and NOT
    // normalized: move takes an INCREMENTAL (dx,dy) applied to current
    // positions; rotate takes an INCREMENTAL angle accumulated into a running
    // total re-applied from the original snapshot; scale takes an ABSOLUTE
    // factor measured from gesture start. See each method's port note.
    void moveSelectionLive(double dx, double dy);
    void commitMove();

    // Rotate about the centroid of the snapshotted selection points.
    // angleDelta is INCREMENTAL (radians); the running total is re-applied to
    // the ORIGINAL snapshot every call, which is what keeps a long drag free
    // of floating-point drift. Multitail arrows are excluded -- see the
    // limitation note on snapshotSelectionPoints below.
    void rotateSelectionLive(double angleDelta);
    void commitRotate();

    // Uniform scale about a caller-supplied FIXED anchor (the dragged handle's
    // geometric opposite on the bbox), NOT the centroid -- matching real
    // corner-drag resize behaviour and deliberately unlike rotate.
    //
    // NOTE: `factor` is ABSOLUTE (total scale measured from gesture start), not
    // incremental like rotate's angleDelta. Calling this with 2.0 then 3.0
    // yields 3x from the original, not 6x. Ported exactly from the real code.
    void scaleSelectionLive(double factor, double anchorX, double anchorY);
    void commitScale();

private:
    enum class DiscreteTransform { RotateCW, RotateCCW, FlipH, FlipV };
    void applyDiscreteTransform(DiscreteTransform mode);

    // Page bounds, copied verbatim from 10-state.js:238-239.
    static constexpr double kPageMinX = -30.0, kPageMaxX = 30.0;
    static constexpr double kPageMinY = -21.0, kPageMaxY = 21.0;

    // Move-gesture drag state (mirrors the real _drag* module-level vars).
    double m_dragDeltaX = 0.0, m_dragDeltaY = 0.0;
    QList<AtomId> m_dragAtomIds;
    QList<RxnArrowId> m_dragArrowIds;
    QList<RxnPlusId> m_dragPlusIds;
    QList<MultitailArrowId> m_dragMultitailIds;
    bool m_dragHasOrigBBox = false;
    double m_dragBBoxMinX = 0.0, m_dragBBoxMinY = 0.0, m_dragBBoxMaxX = 0.0, m_dragBBoxMaxY = 0.0;

    void applyMoveDelta(const QList<AtomId>& atomIds, const QList<RxnArrowId>& arrowIds,
                        const QList<RxnPlusId>& plusIds, const QList<MultitailArrowId>& mtaIds,
                        double dx, double dy);
    void resetMoveDragState();

    // Clears move, rotate AND scale gesture state. DELIBERATE DEVIATION from
    // 10-state.js, which resets only four move vars in undo() and nothing at
    // all in redo(): stale rotate/scale snapshot state surviving an undo lets a
    // later live call re-apply a transform computed from positions the undo has
    // already invalidated, corrupting coordinates. Called by undo() and redo().
    void resetAllDragState();

    // One snapshotted, transformable point. Shared by the rotate and scale
    // gestures (move does not use it -- it works from raw id lists, exactly
    // like the real moveSelection). Mirrors the real _snapshotSelectionPoints
    // tagged-point records; an RxnArrow contributes two independent points.
    //
    // KNOWN LIMITATION (deliberate): multitail arrows are NOT represented here.
    // chem-core's rotate/scale transform only a multitail arrow's spineTop
    // anchor point, and our MultitailArrow has no anchor -- only absolute
    // points -- so there is no faithful mapping. They participate in move
    // (translation is representation-independent) and are excluded from
    // rotate/scale rather than given invented rigid-body behaviour.
    struct TransformPoint {
        enum class Kind { Atom, RxnArrowP1, RxnArrowP2, RxnPlus };
        Kind kind;
        int id;
        double x, y;
    };
    QList<TransformPoint> snapshotSelectionPoints() const;
    static void writeTransformPoint(EditableMolecule& mol, const TransformPoint& pt, double nx, double ny);

    // Rotate-gesture drag state (mirrors the real _rotateDrag* vars).
    QList<TransformPoint> m_rotateOrigPos;
    double m_rotateCenterX = 0.0, m_rotateCenterY = 0.0;
    double m_rotateTotalAngle = 0.0;

    // Scale-gesture drag state (mirrors the real _scaleDrag* vars).
    QList<TransformPoint> m_scaleOrigPos;
    double m_scaleAnchorX = 0.0, m_scaleAnchorY = 0.0;
    double m_scaleTotalFactor = 1.0;

    EditableMolecule m_molecule;
    std::vector<EditCommand> m_history;
    int m_historyPointer = -1;
    bool m_dirty = false;
    SelectionState m_selection;
    static constexpr int kHistorySize = 50;
};

#endif // DOCUMENTSTATE_H
