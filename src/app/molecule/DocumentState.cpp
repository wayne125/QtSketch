// src/app/molecule/DocumentState.cpp
#include "DocumentState.h"
#include <memory>
#include <algorithm>

DocumentState::DocumentState(const QString& initialStructure)
    : m_molecule(initialStructure) {
}

EditableMolecule& DocumentState::molecule() {
    return m_molecule;
}

void DocumentState::executeCommand(EditCommand cmd) {
    // Remove any future redo states -- matches executeCommand's
    // `_history.splice(_historyPointer + 1)` exactly.
    if (m_historyPointer + 1 < static_cast<int>(m_history.size())) {
        m_history.resize(m_historyPointer + 1);
    }
    cmd.execute();
    m_history.push_back(std::move(cmd));
    ++m_historyPointer;
    if (static_cast<int>(m_history.size()) > kHistorySize) {
        m_history.erase(m_history.begin());
        --m_historyPointer;
    }
    m_dirty = true;
}

void DocumentState::undo() {
    if (m_historyPointer < 0) return;
    m_history[m_historyPointer].invert();
    --m_historyPointer;
    m_selection.clear();
    m_dirty = true;
}

void DocumentState::redo() {
    if (m_historyPointer >= static_cast<int>(m_history.size()) - 1) return;
    ++m_historyPointer;
    m_history[m_historyPointer].execute();
    m_selection.clear();
    m_dirty = true;
}

bool DocumentState::isDirty() const {
    return m_dirty;
}

void DocumentState::markClean() {
    m_dirty = false;
}

bool DocumentState::canUndo() const {
    return m_historyPointer >= 0;
}

bool DocumentState::canRedo() const {
    return m_historyPointer < static_cast<int>(m_history.size()) - 1;
}

void DocumentState::selectAtom(AtomId id) {
    m_selection.clear();
    m_selection.atoms.insert(id);
}
void DocumentState::selectBond(BondId id) {
    m_selection.clear();
    m_selection.bonds.insert(id);
}
void DocumentState::selectRxnArrow(RxnArrowId id) {
    m_selection.clear();
    m_selection.rxnArrows.insert(id);
}
void DocumentState::selectRxnPlus(RxnPlusId id) {
    m_selection.clear();
    m_selection.rxnPluses.insert(id);
}
void DocumentState::selectMultitailArrow(MultitailArrowId id) {
    m_selection.clear();
    m_selection.multitailArrows.insert(id);
}
void DocumentState::clearSelection() {
    m_selection.clear();
}

void DocumentState::addAtomToSelection(AtomId id) { m_selection.atoms.insert(id); }
void DocumentState::addBondToSelection(BondId id) { m_selection.bonds.insert(id); }
void DocumentState::addRxnArrowToSelection(RxnArrowId id) { m_selection.rxnArrows.insert(id); }
void DocumentState::addRxnPlusToSelection(RxnPlusId id) { m_selection.rxnPluses.insert(id); }
void DocumentState::addMultitailArrowToSelection(MultitailArrowId id) { m_selection.multitailArrows.insert(id); }

void DocumentState::removeAtomFromSelection(AtomId id) { m_selection.atoms.remove(id); }
void DocumentState::removeBondFromSelection(BondId id) { m_selection.bonds.remove(id); }
void DocumentState::removeRxnArrowFromSelection(RxnArrowId id) { m_selection.rxnArrows.remove(id); }
void DocumentState::removeRxnPlusFromSelection(RxnPlusId id) { m_selection.rxnPluses.remove(id); }
void DocumentState::removeMultitailArrowFromSelection(MultitailArrowId id) { m_selection.multitailArrows.remove(id); }

void DocumentState::selectAll() {
    m_selection.clear();
    for (AtomId id : m_molecule.atomIds()) m_selection.atoms.insert(id);
    for (BondId id : m_molecule.bondIds()) m_selection.bonds.insert(id);
    for (RxnArrowId id : m_molecule.rxnArrowIds()) m_selection.rxnArrows.insert(id);
    for (RxnPlusId id : m_molecule.rxnPlusIds()) m_selection.rxnPluses.insert(id);
    for (MultitailArrowId id : m_molecule.multitailArrowIds()) m_selection.multitailArrows.insert(id);
}

SelectionState& DocumentState::selection() {
    return m_selection;
}

AtomId DocumentState::addAtom(const QString& symbol, double x, double y) {
    auto idBox = std::make_shared<AtomId>(-1);
    EditableMolecule& mol = m_molecule;
    EditCommand cmd;
    cmd.execute = [&mol, idBox, symbol, x, y]() { *idBox = mol.addAtom(symbol, x, y); };
    cmd.invert = [&mol, idBox]() { mol.removeAtom(*idBox); };
    executeCommand(std::move(cmd));
    return *idBox;
}

BondId DocumentState::addBond(AtomId a, AtomId b, int order) {
    auto idBox = std::make_shared<BondId>(-1);
    EditableMolecule& mol = m_molecule;
    EditCommand cmd;
    cmd.execute = [&mol, idBox, a, b, order]() { *idBox = mol.addBond(a, b, order); };
    cmd.invert = [&mol, idBox]() { mol.removeBond(*idBox); };
    executeCommand(std::move(cmd));
    return *idBox;
}

void DocumentState::deleteAtom(AtomId id) {
    // Plain-atom-and-incident-bonds case only in this sub-project (see the
    // sgroup-membership spike, Task 4). EditableMolecule::removeAtom already
    // cascades incident-bond removal internally (sub-project 1) for the
    // forward direction; the inverse must recreate the atom AND every bond
    // that existed on it, so incident-bond data is captured BEFORE removal
    // via bondEndpoints (Step 4) -- mirroring 20-edit.js's deleteAtomById
    // collecting bondsData first.
    EditableMolecule& mol = m_molecule;
    if (!mol.atomIds().contains(id)) return;

    QString label = mol.atomSymbol(id);
    double x = 0, y = 0;
    mol.atomPos(id, x, y);

    struct SavedBond { AtomId other; int order; bool idIsSource; };
    QList<SavedBond> savedBonds;
    for (BondId bid : mol.bondIds()) {
        AtomId ea = -1, eb = -1;
        if (!mol.bondEndpoints(bid, ea, eb)) continue;
        if (ea == id) savedBonds.append({eb, mol.bondOrder(bid), true});
        else if (eb == id) savedBonds.append({ea, mol.bondOrder(bid), false});
    }

    EditCommand cmd;
    cmd.execute = [&mol, id]() { mol.removeAtom(id); };
    cmd.invert = [&mol, label, x, y, savedBonds]() {
        // newId is a fresh local created fresh on every invocation of this
        // closure -- NOT a captured variable, so no dangling-reference risk.
        AtomId newId = mol.addAtom(label, x, y);
        for (const SavedBond& sb : savedBonds) {
            if (sb.idIsSource) mol.addBond(newId, sb.other, sb.order);
            else mol.addBond(sb.other, newId, sb.order);
        }
    };
    executeCommand(std::move(cmd));
}

void DocumentState::deleteBond(BondId id) {
    EditableMolecule& mol = m_molecule;
    AtomId a = -1, b = -1;
    if (!mol.bondEndpoints(id, a, b)) return;
    int order = mol.bondOrder(id);

    EditCommand cmd;
    cmd.execute = [&mol, id]() { mol.removeBond(id); };
    cmd.invert = [&mol, a, b, order]() { mol.addBond(a, b, order); };
    executeCommand(std::move(cmd));
}

void DocumentState::changeAtomLabel(AtomId id, const QString& newLabel) {
    EditableMolecule& mol = m_molecule;
    QString oldLabel = mol.atomSymbol(id);
    if (oldLabel.isEmpty() || oldLabel == newLabel) return; // guarded, matches real JS
    EditCommand cmd;
    cmd.execute = [&mol, id, newLabel]() { mol.setAtomLabel(id, newLabel); };
    cmd.invert = [&mol, id, oldLabel]() { mol.setAtomLabel(id, oldLabel); };
    executeCommand(std::move(cmd));
}

void DocumentState::setAtomMapping(AtomId id, int mappingNumber) {
    EditableMolecule& mol = m_molecule;
    int oldAam = mol.atomAAM(id);
    if (oldAam == mappingNumber) return; // guarded, matches real JS
    EditCommand cmd;
    cmd.execute = [&mol, id, mappingNumber]() { mol.setAtomAAM(id, mappingNumber); };
    cmd.invert = [&mol, id, oldAam]() { mol.setAtomAAM(id, oldAam); };
    executeCommand(std::move(cmd));
}

void DocumentState::changeAtomCharge(AtomId id, int newCharge) {
    newCharge = std::max(-3, std::min(3, newCharge));
    EditableMolecule& mol = m_molecule;
    int oldCharge = mol.atomCharge(id);
    if (oldCharge == newCharge) return; // guarded, matches real JS
    EditCommand cmd;
    cmd.execute = [&mol, id, newCharge]() { mol.setAtomCharge(id, newCharge); };
    cmd.invert = [&mol, id, oldCharge]() { mol.setAtomCharge(id, oldCharge); };
    executeCommand(std::move(cmd));
}

void DocumentState::setAttachmentPoint(AtomId id, int order) {
    // NOT guarded -- real 20-edit.js's setAttachmentPoint always executes.
    EditableMolecule& mol = m_molecule;
    int oldOrder = mol.atomAttachmentOrder(id);
    EditCommand cmd;
    cmd.execute = [&mol, id, order]() { mol.setAtomAttachmentOrder(id, order); };
    cmd.invert = [&mol, id, oldOrder]() { mol.setAtomAttachmentOrder(id, oldOrder); };
    executeCommand(std::move(cmd));
}

void DocumentState::changeAtomIsotope(AtomId id, int isotope) {
    // NOT guarded -- real 20-edit.js's changeAtomIsotope always executes.
    isotope = std::max(0, isotope);
    EditableMolecule& mol = m_molecule;
    int oldIsotope = mol.atomIsotope(id);
    EditCommand cmd;
    cmd.execute = [&mol, id, isotope]() { mol.setAtomIsotope(id, isotope); };
    cmd.invert = [&mol, id, oldIsotope]() { mol.setAtomIsotope(id, oldIsotope); };
    executeCommand(std::move(cmd));
}

void DocumentState::changeAtomRadical(AtomId id, int radical) {
    // NOT guarded -- real 20-edit.js's changeAtomRadical always executes.
    // No clamp: unlike charge (-3..3, a real formal-charge range) or isotope
    // (>=0), radical is Indigo's own enum (INDIGO_SINGLET=101/DOUBLET=102/
    // TRIPLET=103, plus 0 for none) -- an [0,3] clamp here (MDL molfile RAD
    // code range, a different, incompatible encoding) would silently
    // truncate every real radical value before it ever reaches
    // EditableMolecule::setAtomRadical, which passes the value straight to
    // indigoSetRadical unmodified (Task 1).
    EditableMolecule& mol = m_molecule;
    int oldRadical = mol.atomRadical(id);
    EditCommand cmd;
    cmd.execute = [&mol, id, radical]() { mol.setAtomRadical(id, radical); };
    cmd.invert = [&mol, id, oldRadical]() { mol.setAtomRadical(id, oldRadical); };
    executeCommand(std::move(cmd));
}

void DocumentState::changeAtomValence(AtomId id, int valence) {
    // NOT guarded -- real 20-edit.js's changeAtomValence always executes.
    EditableMolecule& mol = m_molecule;
    int oldValence = mol.atomExplicitValence(id);
    EditCommand cmd;
    cmd.execute = [&mol, id, valence]() { mol.setAtomExplicitValence(id, valence); };
    cmd.invert = [&mol, id, oldValence]() { mol.setAtomExplicitValence(id, oldValence); };
    executeCommand(std::move(cmd));
}

DocumentState::AtomProperties DocumentState::atomProperties(AtomId id) const {
    AtomProperties props;
    props.label = m_molecule.atomSymbol(id);
    props.charge = m_molecule.atomCharge(id);
    props.isotope = m_molecule.atomIsotope(id);
    props.radical = m_molecule.atomRadical(id);
    props.explicitValence = m_molecule.atomExplicitValence(id);
    return props;
}

void DocumentState::changeBondOrder(BondId id, int newOrder) {
    EditableMolecule& mol = m_molecule;
    int oldOrder = mol.bondOrder(id);
    if (oldOrder == newOrder) return; // guarded, matches real JS's changeBondType
    EditCommand cmd;
    cmd.execute = [&mol, id, newOrder]() { mol.setBondOrderValue(id, newOrder); };
    cmd.invert = [&mol, id, oldOrder]() { mol.setBondOrderValue(id, oldOrder); };
    executeCommand(std::move(cmd));
}

void DocumentState::setAtomQueryList(AtomId id, const QString& labelsCsv, bool notList) {
    EditableMolecule& mol = m_molecule;
    QString oldLabel = mol.atomSymbol(id);
    EditCommand cmd;
    cmd.execute = [&mol, id, labelsCsv, notList]() { mol.setAtomQueryList(id, labelsCsv, notList); };
    cmd.invert = [&mol, id, oldLabel]() { mol.clearAtomQueryList(id, oldLabel); };
    executeCommand(std::move(cmd));
}

// KNOWN LIMITATION: undo after clearAtomQueryList does NOT restore the
// original query list -- EditableMolecule has no "set query list directly
// from a saved numbers list" entry point bypassing the CSV/symbol parse
// (setAtomQueryList's only entry point takes a CSV string), so a correct
// invert would need that entry point added first. Out of scope for 3a;
// clearAtomQueryList's invert is documented as a no-op below rather than a
// silently-wrong "looks like it restores something" implementation. Revisit
// if a later sub-project needs this restored exactly.
void DocumentState::clearAtomQueryList(AtomId id, const QString& fallbackLabel) {
    EditableMolecule& mol = m_molecule;
    if (!mol.hasAtomQueryList(id)) return;
    EditCommand cmd;
    cmd.execute = [&mol, id, fallbackLabel]() { mol.clearAtomQueryList(id, fallbackLabel); };
    cmd.invert = []() { /* documented no-op -- see KNOWN LIMITATION comment above */ };
    executeCommand(std::move(cmd));
}

// KNOWN LIMITATION: chem-core.js's transformSelection also swaps wedge-bond
// stereo (codes 1<->6) for bonds fully inside the selection on a flip, so the
// depiction stays chemically consistent. Indigo exposes no way to do that --
// indigoBondStereo is read-only, the stereocenter-inference path was ruled out
// by sub-project 3a's Test 12, and indigoInvertStereo was ruled out by Test 14
// ("not a stereobond"; see EditableMolecule.h). Flips here apply the coordinate
// mirror ONLY; the stereo swap is an explicit, documented gap for a later
// sub-project.
void DocumentState::applyDiscreteTransform(DiscreteTransform mode) {
    EditableMolecule& mol = m_molecule;

    // Atoms only, exactly like the real transformSelection.
    struct SavedPos { AtomId id; double x, y; };
    QList<SavedPos> oldPos;
    double cx = 0, cy = 0;
    for (AtomId id : m_selection.atoms) {
        double x = 0, y = 0;
        if (!mol.atomPos(id, x, y)) continue;
        oldPos.append({id, x, y});
        cx += x; cy += y;
    }
    if (oldPos.size() < 2) return;   // matches the real ids.length < 2 guard
    cx /= oldPos.size();
    cy /= oldPos.size();

    EditCommand cmd;
    cmd.execute = [&mol, mode, oldPos, cx, cy]() {
        for (const SavedPos& p : oldPos) {
            double nx = p.x, ny = p.y;
            switch (mode) {
                case DiscreteTransform::RotateCW:  nx = cx + (p.y - cy); ny = cy - (p.x - cx); break;
                case DiscreteTransform::RotateCCW: nx = cx - (p.y - cy); ny = cy + (p.x - cx); break;
                case DiscreteTransform::FlipH:     nx = 2 * cx - p.x;    ny = p.y;             break;
                case DiscreteTransform::FlipV:     nx = p.x;             ny = 2 * cy - p.y;    break;
            }
            mol.setAtomPos(p.id, nx, ny);
        }
    };
    cmd.invert = [&mol, oldPos]() {
        for (const SavedPos& p : oldPos) mol.setAtomPos(p.id, p.x, p.y);
    };
    executeCommand(std::move(cmd));
}

void DocumentState::rotateSelection90CW()      { applyDiscreteTransform(DiscreteTransform::RotateCW); }
void DocumentState::rotateSelection90CCW()     { applyDiscreteTransform(DiscreteTransform::RotateCCW); }
void DocumentState::flipSelectionHorizontal()  { applyDiscreteTransform(DiscreteTransform::FlipH); }
void DocumentState::flipSelectionVertical()    { applyDiscreteTransform(DiscreteTransform::FlipV); }

void DocumentState::moveImage(ImageId id, double dx, double dy) {
    EditableMolecule& mol = m_molecule;
    double x = 0, y = 0, w = 0, h = 0;
    if (!mol.imageRect(id, x, y, w, h)) return;   // matches the real `if (!img) return`

    EditCommand cmd;
    cmd.execute = [&mol, id, dx, dy]() {
        double cx = 0, cy = 0, cw = 0, ch = 0;
        if (mol.imageRect(id, cx, cy, cw, ch)) mol.setImageRect(id, cx + dx, cy + dy, cw, ch);
    };
    cmd.invert = [&mol, id, dx, dy]() {
        double cx = 0, cy = 0, cw = 0, ch = 0;
        if (mol.imageRect(id, cx, cy, cw, ch)) mol.setImageRect(id, cx - dx, cy - dy, cw, ch);
    };
    executeCommand(std::move(cmd));
}

void DocumentState::resizeImage(ImageId id, double scaleFactor) {
    // DELIBERATE ADDITION, not in the real JS: a zero factor would make
    // invert's 1/scaleFactor non-finite and permanently corrupt the image's
    // stored size. ImageRef holds plain doubles with no validation of its own.
    if (scaleFactor == 0.0) return;

    EditableMolecule& mol = m_molecule;
    double x = 0, y = 0, w = 0, h = 0;
    if (!mol.imageRect(id, x, y, w, h)) return;

    EditCommand cmd;
    cmd.execute = [&mol, id, scaleFactor]() {
        double cx = 0, cy = 0, cw = 0, ch = 0;
        if (mol.imageRect(id, cx, cy, cw, ch)) mol.setImageRect(id, cx, cy, cw * scaleFactor, ch * scaleFactor);
    };
    cmd.invert = [&mol, id, scaleFactor]() {
        double cx = 0, cy = 0, cw = 0, ch = 0;
        if (mol.imageRect(id, cx, cy, cw, ch)) mol.setImageRect(id, cx, cy, cw / scaleFactor, ch / scaleFactor);
    };
    executeCommand(std::move(cmd));
}
