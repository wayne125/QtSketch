// src/app/molecule/EditableMolecule.cpp
#include "EditableMolecule.h"
#include "MoleculeSnapshot.h"
#include "indigo.h"
#include <algorithm>
#include <utility>

EditableMolecule::EditableMolecule(const QString& initialStructure) {
    m_session = indigoAllocSessionId();
    indigoSetSessionId(m_session);

    if (initialStructure.isEmpty()) {
        m_mol = indigoCreateMolecule();
    } else {
        m_mol = indigoLoadMoleculeFromString(initialStructure.toUtf8().constData());
    }
    if (m_mol < 0) {
        m_lastError = QString::fromUtf8(indigoGetLastError());
        return;
    }
    // Assign stable external IDs to whatever the initial structure loaded.
    int iter = indigoIterateAtoms(m_mol);
    if (iter >= 0) {
        int a;
        while ((a = indigoNext(iter)) > 0) {
            m_atomIdx.insert(m_nextAtomId++, indigoIndex(a));
            indigoFree(a);
        }
        indigoFree(iter);
    }
    iter = indigoIterateBonds(m_mol);
    if (iter >= 0) {
        int b;
        while ((b = indigoNext(iter)) > 0) {
            m_bondIdx.insert(m_nextBondId++, indigoIndex(b));
            indigoFree(b);
        }
        indigoFree(iter);
    }
}

EditableMolecule::~EditableMolecule() {
    indigoSetSessionId(m_session);
    if (m_mol >= 0)
        indigoFree(m_mol);
    indigoReleaseSessionId(m_session);
}

void EditableMolecule::activateSession() const {
    indigoSetSessionId(m_session);
}

void MoleculeSnapshot::freeHandle() {
    if (m_clone >= 0) {
        indigoSetSessionId(m_session);
        indigoFree(m_clone);
        m_clone = -1;
    }
}

int EditableMolecule::atomCount() const {
    if (m_mol < 0) return 0;
    activateSession();
    return indigoCountAtoms(m_mol);
}

int EditableMolecule::bondCount() const {
    if (m_mol < 0) return 0;
    activateSession();
    return indigoCountBonds(m_mol);
}

StringResult EditableMolecule::toMolfile() const {
    StringResult r;
    if (m_mol < 0) { r.error = QStringLiteral("invalid molecule"); return r; }
    activateSession();
    const char* mf = indigoMolfile(m_mol);
    if (!mf) { r.error = QString::fromUtf8(indigoGetLastError()); return r; }
    r.success = true;
    r.value = QString::fromUtf8(mf);
    return r;
}

AtomId EditableMolecule::addAtom(const QString& symbol, double x, double y) {
    if (m_mol < 0) return -1;
    activateSession();
    int a = indigoAddAtom(m_mol, symbol.toUtf8().constData());
    if (a < 0) { m_lastError = QString::fromUtf8(indigoGetLastError()); return -1; }
    indigoSetXYZ(a, static_cast<float>(x), static_cast<float>(y), 0.0f);
    int idx = indigoIndex(a);
    indigoFree(a);
    AtomId id = m_nextAtomId++;
    m_atomIdx.insert(id, idx);
    return id;
}

bool EditableMolecule::removeAtom(AtomId id) {
    if (m_mol < 0 || !m_atomIdx.contains(id)) return false;
    activateSession();
    int idx = m_atomIdx.value(id);
    int a = indigoGetAtom(m_mol, idx);
    if (a < 0) return false;

    // Record which bond indices die with this atom BEFORE removal, by
    // scanning all bonds for ones touching idx.
    QList<BondId> deadBonds;
    for (auto it = m_bondIdx.constBegin(); it != m_bondIdx.constEnd(); ++it) {
        int b = indigoGetBond(m_mol, it.value());
        if (b < 0) continue;
        int src = indigoSource(b), dst = indigoDestination(b);
        int srcIdx = indigoIndex(src), dstIdx = indigoIndex(dst);
        indigoFree(src); indigoFree(dst); indigoFree(b);
        if (srcIdx == idx || dstIdx == idx) deadBonds.append(it.key());
    }

    if (indigoRemove(a) < 0) {
        m_lastError = QString::fromUtf8(indigoGetLastError());
        indigoFree(a);
        return false;
    }
    m_atomIdx.remove(id);
    for (BondId bid : deadBonds) m_bondIdx.remove(bid);
    rebuildIndexTables();
    indigoFree(a);
    return true;
}

BondId EditableMolecule::addBond(AtomId a, AtomId b, int order) {
    if (m_mol < 0 || !m_atomIdx.contains(a) || !m_atomIdx.contains(b)) return -1;
    activateSession();
    int ha = indigoGetAtom(m_mol, m_atomIdx.value(a));
    int hb = indigoGetAtom(m_mol, m_atomIdx.value(b));
    if (ha < 0 || hb < 0) return -1;
    int bond = indigoAddBond(ha, hb, order);
    indigoFree(ha); indigoFree(hb);
    if (bond < 0) { m_lastError = QString::fromUtf8(indigoGetLastError()); return -1; }
    int idx = indigoIndex(bond);
    indigoFree(bond);
    BondId id = m_nextBondId++;
    m_bondIdx.insert(id, idx);
    return id;
}

bool EditableMolecule::removeBond(BondId id) {
    if (m_mol < 0 || !m_bondIdx.contains(id)) return false;
    activateSession();
    int b = indigoGetBond(m_mol, m_bondIdx.value(id));
    if (b < 0) return false;
    if (indigoRemove(b) < 0) { indigoFree(b); return false; }
    m_bondIdx.remove(id);
    rebuildIndexTables();
    indigoFree(b);
    return true;
}

QString EditableMolecule::atomSymbol(AtomId id) const {
    if (m_mol < 0 || !m_atomIdx.contains(id)) return {};
    activateSession();
    int a = indigoGetAtom(m_mol, m_atomIdx.value(id));
    if (a < 0) return {};
    QString s = QString::fromUtf8(indigoSymbol(a));
    indigoFree(a);
    return s;
}

bool EditableMolecule::atomPos(AtomId id, double& x, double& y) const {
    if (m_mol < 0 || !m_atomIdx.contains(id)) return false;
    activateSession();
    int a = indigoGetAtom(m_mol, m_atomIdx.value(id));
    if (a < 0) return false;
    float* xyz = indigoXYZ(a);
    if (!xyz) { indigoFree(a); return false; }
    x = xyz[0]; y = xyz[1];
    indigoFree(a);
    return true;
}

int EditableMolecule::atomCharge(AtomId id) const {
    if (m_mol < 0 || !m_atomIdx.contains(id)) return 0;
    activateSession();
    int a = indigoGetAtom(m_mol, m_atomIdx.value(id));
    if (a < 0) return 0;
    int charge = 0;
    indigoGetCharge(a, &charge);
    indigoFree(a);
    return charge;
}

bool EditableMolecule::setAtomCharge(AtomId id, int charge) {
    if (m_mol < 0 || !m_atomIdx.contains(id)) return false;
    activateSession();
    int a = indigoGetAtom(m_mol, m_atomIdx.value(id));
    if (a < 0) return false;
    bool ok = indigoSetCharge(a, charge) >= 0;
    indigoFree(a);
    return ok;
}

int EditableMolecule::atomIsotope(AtomId id) const {
    if (m_mol < 0 || !m_atomIdx.contains(id)) return 0;
    activateSession();
    int a = indigoGetAtom(m_mol, m_atomIdx.value(id));
    if (a < 0) return 0;
    int iso = indigoIsotope(a);
    indigoFree(a);
    return iso > 0 ? iso : 0;
}

bool EditableMolecule::setAtomIsotope(AtomId id, int isotope) {
    if (m_mol < 0 || !m_atomIdx.contains(id)) return false;
    activateSession();
    int a = indigoGetAtom(m_mol, m_atomIdx.value(id));
    if (a < 0) return false;
    bool ok = indigoSetIsotope(a, isotope) >= 0;
    indigoFree(a);
    return ok;
}

int EditableMolecule::atomRadical(AtomId id) const {
    if (m_mol < 0 || !m_atomIdx.contains(id)) return 0;
    activateSession();
    int a = indigoGetAtom(m_mol, m_atomIdx.value(id));
    if (a < 0) return 0;
    int radical = 0;
    indigoGetRadical(a, &radical);
    indigoFree(a);
    return radical;
}

bool EditableMolecule::setAtomRadical(AtomId id, int radical) {
    if (m_mol < 0 || !m_atomIdx.contains(id)) return false;
    activateSession();
    int a = indigoGetAtom(m_mol, m_atomIdx.value(id));
    if (a < 0) return false;
    bool ok = indigoSetRadical(a, radical) >= 0;
    indigoFree(a);
    return ok;
}

int EditableMolecule::atomExplicitValence(AtomId id) const {
    if (m_mol < 0 || !m_atomIdx.contains(id)) return -1;
    activateSession();
    int a = indigoGetAtom(m_mol, m_atomIdx.value(id));
    if (a < 0) return -1;
    int valence = -1;
    int ok = indigoGetExplicitValence(a, &valence);
    indigoFree(a);
    return ok ? valence : -1;
}

bool EditableMolecule::setAtomExplicitValence(AtomId id, int valence) {
    if (m_mol < 0 || !m_atomIdx.contains(id)) return false;
    activateSession();
    int a = indigoGetAtom(m_mol, m_atomIdx.value(id));
    if (a < 0) return false;
    bool ok = indigoSetExplicitValence(a, valence) >= 0;
    indigoFree(a);
    return ok;
}

bool EditableMolecule::setAtomLabel(AtomId id, const QString& newSymbol) {
    if (m_mol < 0 || !m_atomIdx.contains(id)) return false;
    activateSession();
    int a = indigoGetAtom(m_mol, m_atomIdx.value(id));
    if (a < 0) return false;
    // Read every attribute indigoResetAtom might not preserve BEFORE resetting,
    // since its effect on charge/isotope/radical/explicit valence is unverified
    // and must not be assumed. Re-apply them unconditionally after a successful
    // reset -- a harmless no-op if it turns out they were already preserved.
    int oldCharge = 0; indigoGetCharge(a, &oldCharge);
    int oldIsotope = indigoIsotope(a); if (oldIsotope < 0) oldIsotope = 0;
    int oldRadical = 0; indigoGetRadical(a, &oldRadical);
    int oldValence = -1; bool hadValence = indigoGetExplicitValence(a, &oldValence) != 0;
    const char* oldSymbolRaw = indigoSymbol(a);
    QString oldSymbol = oldSymbolRaw ? QString::fromUtf8(oldSymbolRaw) : QString();

    int result = indigoResetAtom(a, newSymbol.toUtf8().constData());
    if (result < 0) {
        m_lastError = QString::fromUtf8(indigoGetLastError());
        indigoFree(a);
        return false;
    }
    // indigoResetAtom does not validate the symbol against the periodic table --
    // an unrecognized string (spike-confirmed: tests/editable_molecule_test.cpp
    // Test 9) silently succeeds as a pseudo-atom label instead of failing. Detect
    // that case via indigoIsPseudoatom and revert to the original symbol, since
    // this class (like plain Indigo molecule handles generally) does not support
    // pseudo-atoms/R-sites.
    if (indigoIsPseudoatom(a)) {
        indigoResetAtom(a, oldSymbol.toUtf8().constData());
        indigoSetCharge(a, oldCharge);
        indigoSetIsotope(a, oldIsotope);
        indigoSetRadical(a, oldRadical);
        if (hadValence) indigoSetExplicitValence(a, oldValence);
        indigoFree(a);
        return false;
    }
    indigoSetCharge(a, oldCharge);
    indigoSetIsotope(a, oldIsotope);
    indigoSetRadical(a, oldRadical);
    if (hadValence) indigoSetExplicitValence(a, oldValence);
    indigoFree(a);
    return true;
}

bool EditableMolecule::setBondOrderValue(BondId id, int order) {
    if (m_mol < 0 || !m_bondIdx.contains(id)) return false;
    activateSession();
    int b = indigoGetBond(m_mol, m_bondIdx.value(id));
    if (b < 0) return false;
    bool ok = indigoSetBondOrder(b, order) >= 0;
    indigoFree(b);
    return ok;
}

int EditableMolecule::atomAttachmentOrder(AtomId id) const {
    if (m_mol < 0 || !m_atomIdx.contains(id)) return 0;
    activateSession();
    int targetIdx = m_atomIdx.value(id);
    int maxOrder = indigoCountAttachmentPoints(m_mol);
    for (int order = 1; order <= maxOrder; ++order) {
        int iter = indigoIterateAttachmentPoints(m_mol, order);
        if (iter < 0) continue;
        int h;
        while ((h = indigoNext(iter)) > 0) {
            int idx = indigoIndex(h);
            indigoFree(h);
            if (idx == targetIdx) {
                indigoFree(iter);
                return order;
            }
        }
        indigoFree(iter);
    }
    return 0;
}

bool EditableMolecule::setAtomAttachmentOrder(AtomId id, int order) {
    if (m_mol < 0 || !m_atomIdx.contains(id)) return false;
    activateSession();
    int targetIdx = m_atomIdx.value(id);

    // Two probe-confirmed Indigo API quirks make this harder than it looks:
    // (1) indigoClearAttachmentPoints only accepts the MOLECULE handle, not an
    // atom handle (passing an atom handle fails with -1) -- and it wipes every
    // atom's attachment points, not just one. (2) indigoSetAttachmentPoint
    // ADDS the atom to a new order rather than replacing any existing one (an
    // atom set to order 1 then order 2 ends up a member of BOTH orders). To
    // make this a true single-atom "set" (each atom in at most one order),
    // capture every OTHER atom's current (order, atomIdx) pairs, clear the
    // whole molecule's attachment points, then re-apply the captured pairs
    // plus this atom's new order (if any).
    int maxOrder = indigoCountAttachmentPoints(m_mol);
    QList<std::pair<int, int>> others; // (order, atomIdx), excludes targetIdx
    for (int ord = 1; ord <= maxOrder; ++ord) {
        int iter = indigoIterateAttachmentPoints(m_mol, ord);
        if (iter < 0) continue;
        int h;
        while ((h = indigoNext(iter)) > 0) {
            int idx = indigoIndex(h);
            indigoFree(h);
            if (idx != targetIdx) others.append({ord, idx});
        }
        indigoFree(iter);
    }

    if (maxOrder > 0 && indigoClearAttachmentPoints(m_mol) < 0) return false;

    for (const auto& pr : others) {
        int atomHandle = indigoGetAtom(m_mol, pr.second);
        if (atomHandle >= 0) {
            indigoSetAttachmentPoint(atomHandle, pr.first);
            indigoFree(atomHandle);
        }
    }

    if (order > 0) {
        int a = indigoGetAtom(m_mol, targetIdx);
        if (a < 0) return false;
        bool ok = indigoSetAttachmentPoint(a, order) >= 0;
        indigoFree(a);
        return ok;
    }
    return true;
}

bool EditableMolecule::setAtomQueryList(AtomId id, const QString& labelsCsv, bool notList) {
    if (m_mol < 0 || !m_atomIdx.contains(id)) return false;
    activateSession();

    QList<int> atomicNumbers;
    const QStringList labels = labelsCsv.split(QLatin1Char(','), Qt::SkipEmptyParts);
    for (const QString& rawLabel : labels) {
        const QString label = rawLabel.trimmed();
        if (label.isEmpty()) continue;
        // Element-symbol -> atomic-number lookup via a throwaway atom on the
        // real molecule (removed immediately) rather than a ported periodic
        // table -- Indigo already knows what a valid element symbol is.
        int probe = indigoAddAtom(m_mol, label.toUtf8().constData());
        if (probe < 0) continue;  // not a valid element symbol; skip, matching _makeAtomList's filter
        int number = indigoAtomicNumber(probe);
        indigoRemove(probe);
        indigoFree(probe);
        if (number > 0 && !atomicNumbers.contains(number)) atomicNumbers.append(number);
    }
    if (atomicNumbers.isEmpty()) return false;  // matches _makeAtomList returning null on zero valid labels

    // Cannot go through the public setAtomLabel() here: "L#" is itself a
    // pseudo-atom label (probe-confirmed: indigoResetAtom(a, "L#") makes
    // indigoIsPseudoatom(a) true), and setAtomLabel deliberately REJECTS any
    // reset that produces a pseudoatom (see its own comment) to reject
    // garbage/R-group input for ordinary label changes. "L#" is this class's
    // one legitimate internal pseudo-label use (the query-list sentinel), so
    // it's applied directly via indigoResetAtom, bypassing that rejection.
    int a = indigoGetAtom(m_mol, m_atomIdx.value(id));
    if (a < 0) return false;
    int oldCharge = 0; indigoGetCharge(a, &oldCharge);
    int oldIsotope = indigoIsotope(a); if (oldIsotope < 0) oldIsotope = 0;
    int oldRadical = 0; indigoGetRadical(a, &oldRadical);
    int oldValence = -1; bool hadValence = indigoGetExplicitValence(a, &oldValence) != 0;
    if (indigoResetAtom(a, "L#") < 0) {
        m_lastError = QString::fromUtf8(indigoGetLastError());
        indigoFree(a);
        return false;
    }
    indigoSetCharge(a, oldCharge);
    indigoSetIsotope(a, oldIsotope);
    indigoSetRadical(a, oldRadical);
    if (hadValence) indigoSetExplicitValence(a, oldValence);
    indigoFree(a);

    AtomQueryList list;
    list.atomicNumbers = atomicNumbers;
    list.notList = notList;
    m_ext.atomQueryLists.insert(id, list);
    return true;
}

bool EditableMolecule::clearAtomQueryList(AtomId id, const QString& fallbackLabel) {
    if (m_mol < 0 || !m_atomIdx.contains(id)) return false;
    if (!m_ext.atomQueryLists.contains(id)) return false;
    if (!setAtomLabel(id, fallbackLabel)) return false;
    m_ext.atomQueryLists.remove(id);
    return true;
}

bool EditableMolecule::hasAtomQueryList(AtomId id) const {
    return m_ext.atomQueryLists.contains(id);
}

QList<int> EditableMolecule::atomQueryListNumbers(AtomId id) const {
    return m_ext.atomQueryLists.value(id).atomicNumbers;
}

bool EditableMolecule::atomQueryListIsNotList(AtomId id) const {
    return m_ext.atomQueryLists.value(id).notList;
}

int EditableMolecule::bondOrder(BondId id) const {
    if (m_mol < 0 || !m_bondIdx.contains(id)) return -1;
    activateSession();
    int b = indigoGetBond(m_mol, m_bondIdx.value(id));
    if (b < 0) return -1;
    int order = indigoBondOrder(b);
    indigoFree(b);
    return order;
}

QList<AtomId> EditableMolecule::atomIds() const {
    auto k = m_atomIdx.keys();
    std::sort(k.begin(), k.end());
    return k;
}

QList<BondId> EditableMolecule::bondIds() const {
    auto k = m_bondIdx.keys();
    std::sort(k.begin(), k.end());
    return k;
}

void EditableMolecule::rebuildIndexTables() {
    activateSession();
    QList<AtomId> aIds = m_atomIdx.keys();
    std::sort(aIds.begin(), aIds.end());
    QList<int> aIdx;
    int iter = indigoIterateAtoms(m_mol);
    if (iter >= 0) {
        int h;
        while ((h = indigoNext(iter)) > 0) { aIdx.append(indigoIndex(h)); indigoFree(h); }
        indigoFree(iter);
    }
    if (aIds.size() == aIdx.size())
        for (int i = 0; i < aIds.size(); ++i) m_atomIdx[aIds[i]] = aIdx[i];

    QList<BondId> bIds = m_bondIdx.keys();
    std::sort(bIds.begin(), bIds.end());
    QList<int> bIdx;
    iter = indigoIterateBonds(m_mol);
    if (iter >= 0) {
        int h;
        while ((h = indigoNext(iter)) > 0) { bIdx.append(indigoIndex(h)); indigoFree(h); }
        indigoFree(iter);
    }
    if (bIds.size() == bIdx.size())
        for (int i = 0; i < bIds.size(); ++i) m_bondIdx[bIds[i]] = bIdx[i];
}

void EditableMolecule::setName(const QString& n) { m_ext.name = n; }
QString EditableMolecule::name() const { return m_ext.name; }

int EditableMolecule::addTextAnnotation(double x, double y, const QString& content) {
    TextId id = m_ext.nextTextId++;
    m_ext.texts.insert(id, {x, y, content});
    return id;
}
int EditableMolecule::textAnnotationCount() const { return m_ext.texts.size(); }

int EditableMolecule::addRxnArrow(double x1, double y1, double x2, double y2) {
    RxnArrowId id = m_ext.nextRxnArrowId++;
    m_ext.rxnArrows.insert(id, {x1, y1, x2, y2});
    return id;
}
int EditableMolecule::rxnArrowCount() const { return m_ext.rxnArrows.size(); }

int EditableMolecule::addRxnPlus(double x, double y) {
    RxnPlusId id = m_ext.nextRxnPlusId++;
    m_ext.rxnPluses.insert(id, {x, y});
    return id;
}
int EditableMolecule::rxnPlusCount() const { return m_ext.rxnPluses.size(); }

int EditableMolecule::addMultitailArrow(const QList<double>& headAndTails) {
    MultitailArrowId id = m_ext.nextMultitailArrowId++;
    m_ext.multitailArrows.insert(id, {headAndTails});
    return id;
}
int EditableMolecule::multitailArrowCount() const { return m_ext.multitailArrows.size(); }

int EditableMolecule::addImage(double x, double y, double w, double h, const QByteArray& pngData) {
    ImageId id = m_ext.nextImageId++;
    m_ext.images.insert(id, {x, y, w, h, pngData});
    return id;
}
int EditableMolecule::imageCount() const { return m_ext.images.size(); }

bool EditableMolecule::removeTextAnnotation(TextId id) { return m_ext.texts.remove(id) > 0; }
bool EditableMolecule::removeRxnArrow(RxnArrowId id) { return m_ext.rxnArrows.remove(id) > 0; }
bool EditableMolecule::removeRxnPlus(RxnPlusId id) { return m_ext.rxnPluses.remove(id) > 0; }
bool EditableMolecule::removeMultitailArrow(MultitailArrowId id) { return m_ext.multitailArrows.remove(id) > 0; }
bool EditableMolecule::removeImage(ImageId id) { return m_ext.images.remove(id) > 0; }
QList<RxnArrowId> EditableMolecule::rxnArrowIds() const { return m_ext.rxnArrows.keys(); }
QList<RxnPlusId> EditableMolecule::rxnPlusIds() const { return m_ext.rxnPluses.keys(); }
QList<MultitailArrowId> EditableMolecule::multitailArrowIds() const { return m_ext.multitailArrows.keys(); }

void EditableMolecule::setStereoFlag(int fragmentIndex, int flag) { m_ext.stereoFlags.insert(fragmentIndex, flag); }
int EditableMolecule::stereoFlag(int fragmentIndex) const { return m_ext.stereoFlags.value(fragmentIndex, -1); }

void EditableMolecule::setAtomAAM(AtomId id, int mapNumber) { m_ext.atomAAM.insert(id, mapNumber); }
int EditableMolecule::atomAAM(AtomId id) const { return m_ext.atomAAM.value(id, 0); }

void EditableMolecule::setAtomCheckWarning(AtomId id, bool warn) { m_ext.atomCheckWarnings.insert(id, warn); }
bool EditableMolecule::atomCheckWarning(AtomId id) const { return m_ext.atomCheckWarnings.value(id, false); }

int EditableMolecule::addDataSGroup(const QList<AtomId>& atoms, const QString& description, const QString& data) {
    if (m_mol < 0) return -1;
    activateSession();
    QList<int> idx;
    for (AtomId id : atoms) {
        if (!m_atomIdx.contains(id)) return -1;
        idx.append(m_atomIdx.value(id));
    }
    int sg = indigoAddDataSGroup(m_mol, static_cast<int>(idx.size()), idx.data(), 0, nullptr,
                                 description.toUtf8().constData(), data.toUtf8().constData());
    if (sg < 0) { m_lastError = QString::fromUtf8(indigoGetLastError()); return -1; }
    int sgIdx = indigoIndex(sg);
    indigoFree(sg);
    return sgIdx;
}

int EditableMolecule::dataSGroupCount() const {
    if (m_mol < 0) return 0;
    activateSession();
    int n = 0;
    int iter = indigoIterateDataSGroups(m_mol);
    if (iter >= 0) {
        int h;
        while ((h = indigoNext(iter)) > 0) { ++n; indigoFree(h); }
        indigoFree(iter);
    }
    return n;
}

MoleculeSnapshot EditableMolecule::snapshot() const {
    MoleculeSnapshot s;
    if (m_mol < 0) return s;
    activateSession();
    s.m_clone = indigoClone(m_mol);
    s.m_session = m_session;
    s.m_ext = m_ext;
    s.m_atomIdx = m_atomIdx;
    s.m_bondIdx = m_bondIdx;
    s.m_nextAtomId = m_nextAtomId;
    s.m_nextBondId = m_nextBondId;
    return s;
}

bool EditableMolecule::restore(const MoleculeSnapshot& snap) {
    if (!snap.isValid() || snap.m_session != m_session) return false;
    activateSession();
    int fresh = indigoClone(snap.m_clone);
    if (fresh < 0) { m_lastError = QString::fromUtf8(indigoGetLastError()); return false; }
    if (m_mol >= 0) indigoFree(m_mol);
    m_mol = fresh;
    m_ext = snap.m_ext;
    m_atomIdx = snap.m_atomIdx;
    m_bondIdx = snap.m_bondIdx;
    if (snap.m_nextAtomId > m_nextAtomId) m_nextAtomId = snap.m_nextAtomId;
    if (snap.m_nextBondId > m_nextBondId) m_nextBondId = snap.m_nextBondId;
    return true;
}
