// src/app/molecule/EditableMolecule.cpp
#include "EditableMolecule.h"
#include "MoleculeSnapshot.h"
#include "indigo.h"
#include <algorithm>
#include <utility>
#include <cmath>
#include <QSet>

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
    assignFreshAtomBondIds();
}

void EditableMolecule::assignFreshAtomBondIds() {
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

bool EditableMolecule::loadFrom(const QString& molfileOrSmiles) {
    activateSession();
    int fresh = indigoLoadMoleculeFromString(molfileOrSmiles.toUtf8().constData());
    if (fresh < 0) {
        m_lastError = QString::fromUtf8(indigoGetLastError());
        return false;
    }
    if (m_mol >= 0) indigoFree(m_mol);
    m_mol = fresh;
    m_atomIdx.clear();
    m_bondIdx.clear();
    m_sgroupIdx.clear();
    m_sgroupExpanded.clear();
    m_ext = ExtensionData{};
    assignFreshAtomBondIds();
    return true;
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

StringResult EditableMolecule::submoleculeMolfile(const QList<AtomId>& atomIds, const QList<BondId>& bondIds) const {
    StringResult r;
    if (m_mol < 0) { r.error = QStringLiteral("invalid molecule"); return r; }
    activateSession();

    QSet<AtomId> effectiveAtoms(atomIds.begin(), atomIds.end());
    for (BondId bid : bondIds) {
        AtomId a = -1, b = -1;
        if (!bondEndpoints(bid, a, b)) { r.error = QStringLiteral("unknown bond id"); return r; }
        effectiveAtoms.insert(a);
        effectiveAtoms.insert(b);
    }

    QList<int> vertexIdx;
    for (AtomId aid : effectiveAtoms) {
        if (!m_atomIdx.contains(aid)) { r.error = QStringLiteral("unknown atom id"); return r; }
        vertexIdx.append(m_atomIdx.value(aid));
    }
    QList<int> edgeIdx;
    for (BondId bid : bondIds) {
        if (!m_bondIdx.contains(bid)) { r.error = QStringLiteral("unknown bond id"); return r; }
        edgeIdx.append(m_bondIdx.value(bid));
    }

    if (vertexIdx.isEmpty()) { r.error = QStringLiteral("empty selection"); return r; }

    int sub = indigoCreateEdgeSubmolecule(m_mol, vertexIdx.size(), vertexIdx.data(), edgeIdx.size(), edgeIdx.data());
    if (sub < 0) { r.error = QString::fromUtf8(indigoGetLastError()); return r; }

    const char* mf = indigoMolfile(sub);
    if (!mf) {
        r.error = QString::fromUtf8(indigoGetLastError());
        indigoFree(sub);
        return r;
    }
    r.success = true;
    r.value = QString::fromUtf8(mf);
    indigoFree(sub);
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

bool EditableMolecule::setAtomPos(AtomId id, double x, double y) {
    if (m_mol < 0 || !m_atomIdx.contains(id)) return false;
    activateSession();
    int a = indigoGetAtom(m_mol, m_atomIdx.value(id));
    if (a < 0) return false;
    bool ok = indigoSetXYZ(a, static_cast<float>(x), static_cast<float>(y), 0.0f) >= 0;
    indigoFree(a);
    return ok;
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

bool EditableMolecule::bondEndpoints(BondId id, AtomId& a, AtomId& b) const {
    if (m_mol < 0 || !m_bondIdx.contains(id)) return false;
    activateSession();
    int bond = indigoGetBond(m_mol, m_bondIdx.value(id));
    if (bond < 0) return false;
    int src = indigoSource(bond), dst = indigoDestination(bond);
    int srcIdx = indigoIndex(src), dstIdx = indigoIndex(dst);
    indigoFree(src); indigoFree(dst); indigoFree(bond);
    for (auto it = m_atomIdx.constBegin(); it != m_atomIdx.constEnd(); ++it) {
        if (it.value() == srcIdx) a = it.key();
        if (it.value() == dstIdx) b = it.key();
    }
    return true;
}

BondId EditableMolecule::findBond(AtomId a, AtomId b) const {
    for (BondId bid : bondIds()) {
        AtomId ea = -1, eb = -1;
        if (!bondEndpoints(bid, ea, eb)) continue;
        if ((ea == a && eb == b) || (ea == b && eb == a)) return bid;
    }
    return -1;
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

QList<TextId> EditableMolecule::textAnnotationIds() const { return m_ext.texts.keys(); }
QList<ImageId> EditableMolecule::imageIds() const { return m_ext.images.keys(); }

bool EditableMolecule::textAnnotationContent(TextId id, double& x, double& y, QString& content, bool& bold, bool& italic) const {
    if (!m_ext.texts.contains(id)) return false;
    const TextAnnotation& t = m_ext.texts.value(id);
    x = t.x; y = t.y; content = t.content; bold = t.bold; italic = t.italic;
    return true;
}

bool EditableMolecule::setTextAnnotation(TextId id, const QString& content, bool bold, bool italic) {
    if (!m_ext.texts.contains(id)) return false;
    TextAnnotation& t = m_ext.texts[id];
    t.content = content; t.bold = bold; t.italic = italic;
    return true;
}

bool EditableMolecule::imageData(ImageId id, double& x, double& y, double& w, double& h, QByteArray& pngData) const {
    if (!m_ext.images.contains(id)) return false;
    const ImageRef& img = m_ext.images.value(id);
    x = img.x; y = img.y; w = img.w; h = img.h; pngData = img.pngData;
    return true;
}

bool EditableMolecule::rxnArrowEndpoints(RxnArrowId id, double& x1, double& y1, double& x2, double& y2) const {
    if (!m_ext.rxnArrows.contains(id)) return false;
    const RxnArrow& a = m_ext.rxnArrows[id];
    x1 = a.x1; y1 = a.y1; x2 = a.x2; y2 = a.y2;
    return true;
}

bool EditableMolecule::setRxnArrowEndpoints(RxnArrowId id, double x1, double y1, double x2, double y2) {
    if (!m_ext.rxnArrows.contains(id)) return false;
    m_ext.rxnArrows[id] = {x1, y1, x2, y2};
    return true;
}

bool EditableMolecule::rxnPlusPos(RxnPlusId id, double& x, double& y) const {
    if (!m_ext.rxnPluses.contains(id)) return false;
    const RxnPlus& p = m_ext.rxnPluses[id];
    x = p.x; y = p.y;
    return true;
}

bool EditableMolecule::setRxnPlusPos(RxnPlusId id, double x, double y) {
    if (!m_ext.rxnPluses.contains(id)) return false;
    m_ext.rxnPluses[id] = {x, y};
    return true;
}

QList<double> EditableMolecule::multitailArrowPoints(MultitailArrowId id) const {
    if (!m_ext.multitailArrows.contains(id)) return {};
    return m_ext.multitailArrows[id].headAndTails;
}

bool EditableMolecule::setMultitailArrowPoints(MultitailArrowId id, const QList<double>& points) {
    if (!m_ext.multitailArrows.contains(id)) return false;
    m_ext.multitailArrows[id].headAndTails = points;
    return true;
}

bool EditableMolecule::imageRect(ImageId id, double& x, double& y, double& w, double& h) const {
    if (!m_ext.images.contains(id)) return false;
    const ImageRef& im = m_ext.images[id];
    x = im.x; y = im.y; w = im.w; h = im.h;
    return true;
}

bool EditableMolecule::setImageRect(ImageId id, double x, double y, double w, double h) {
    if (!m_ext.images.contains(id)) return false;
    ImageRef& im = m_ext.images[id];
    im.x = x; im.y = y; im.w = w; im.h = h;   // pngData deliberately preserved
    return true;
}

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
    s.m_sgroupIdx = m_sgroupIdx;
    s.m_sgroupExpanded = m_sgroupExpanded;
    s.m_nextSGroupId = m_nextSGroupId;
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
    m_sgroupIdx = snap.m_sgroupIdx;
    m_sgroupExpanded = snap.m_sgroupExpanded;
    if (snap.m_nextSGroupId > m_nextSGroupId) m_nextSGroupId = snap.m_nextSGroupId;
    return true;
}

QList<BondId> EditableMolecule::graftAtomOnto(AtomId doomed, AtomId kept) {
    QList<BondId> createdBonds;
    if (m_mol < 0 || !m_atomIdx.contains(doomed) || !m_atomIdx.contains(kept)) return createdBonds;

    // Capture doomed's incident bonds BEFORE mutating anything -- the same
    // capture-before-destroy shape DocumentState::deleteAtom (sub-project 3a)
    // uses.
    struct Neighbor { AtomId other; int order; };
    QList<Neighbor> neighbors;
    for (BondId bid : bondIds()) {
        AtomId ea = -1, eb = -1;
        if (!bondEndpoints(bid, ea, eb)) continue;
        if (ea == doomed) neighbors.append({eb, bondOrder(bid)});
        else if (eb == doomed) neighbors.append({ea, bondOrder(bid)});
    }

    for (const Neighbor& nb : neighbors) {
        if (nb.other == kept) continue;   // would become a self-loop; dropped,
                                           // matching the real code's b.begin===b.end dedupe
        BondId newBond = addBond(kept, nb.other, nb.order);
        // A negative return is the SEAM case (3c's Test 16, Q3): kept is
        // already bonded to nb.other, so Indigo rejects the duplicate
        // parallel edge -- the pre-existing bond survives untouched. Not an
        // error; simply nothing to record.
        if (newBond >= 0) createdBonds.append(newBond);
    }

    // removeAtom cascades to delete doomed's remaining incident bonds
    // automatically (3c's Test 16, Q1). Confirmed safe even when doomed is a
    // superatom's member (sub-project 3d's own probe): the sgroup survives
    // intact, its attachment-point iteration simply yields no further
    // entries afterward.
    removeAtom(doomed);
    return createdBonds;
}

EditableMolecule::MergeResult EditableMolecule::mergeOverlappingAtoms(double tolerance) {
    MergeResult result;
    // Ascending id order == insertion order for our never-reused AtomId
    // counter -- the exact invariant fuseOverlappingAtoms relies on to make
    // "kept" always the earlier (pre-existing) atom in a coincident pair.
    QList<AtomId> ids = atomIds();
    QSet<AtomId> doomedSet;

    for (int i = 0; i < ids.size(); ++i) {
        AtomId keptId = ids[i];
        if (doomedSet.contains(keptId)) continue;
        double kx = 0, ky = 0;
        if (!atomPos(keptId, kx, ky)) continue;

        for (int j = i + 1; j < ids.size(); ++j) {
            AtomId doomedId = ids[j];
            if (doomedSet.contains(doomedId)) continue;
            double dx = 0, dy = 0;
            if (!atomPos(doomedId, dx, dy)) continue;
            double ddx = kx - dx, ddy = ky - dy;
            if (std::sqrt(ddx * ddx + ddy * ddy) >= tolerance) continue;

            QList<BondId> newBonds = graftAtomOnto(doomedId, keptId);
            result.createdBonds.append(newBonds);
            result.mergedAway.insert(doomedId, keptId);
            doomedSet.insert(doomedId);
        }
    }
    return result;
}

QList<AtomId> EditableMolecule::addBenzeneRing(double cx, double cy) {
    static const double kPi = 3.14159265358979323846;
    const double r = 1.5;   // chem-core's StandardBondLength
    QList<AtomId> addedAtoms;
    for (int i = 0; i < 6; ++i) {
        double angle = (kPi / 3.0) * i - kPi / 2.0;
        double px = cx + r * std::cos(angle);
        double py = cy + r * std::sin(angle);
        addedAtoms.append(addAtom(QStringLiteral("C"), px, py));
    }
    for (int j = 0; j < 6; ++j) {
        int bType = (j % 2 == 0) ? 2 : 1;
        addBond(addedAtoms[j], addedAtoms[(j + 1) % 6], bType);
    }
    return addedAtoms;
}

EditableMolecule::InsertResult EditableMolecule::insertStructure(const QString& sourceMolfile,
                                                                   const std::function<QPointF(double, double)>& transform) {
    InsertResult result;
    if (m_mol < 0 || sourceMolfile.isEmpty()) return result;
    activateSession();

    // Reparse the Molfile text into a fresh handle IN THIS (now-active)
    // session -- this is the confirmed-safe way to receive a structure from
    // TemplateLibrary's own, different session; a raw handle from that
    // session cannot be used here directly (confirmed by direct probe:
    // cross-session handle reuse either fails or silently aliases an
    // unrelated object). Since this handle is a private, fresh reparse (not
    // shared with anything), it's safe to mutate directly and free once done.
    int clone = indigoLoadMoleculeFromString(sourceMolfile.toUtf8().constData());
    if (clone < 0) return result;
    int atomIter = indigoIterateAtoms(clone);
    if (atomIter >= 0) {
        int a;
        while ((a = indigoNext(atomIter)) > 0) {
            float* xyz = indigoXYZ(a);
            if (xyz) {
                QPointF p = transform(xyz[0], xyz[1]);
                indigoSetXYZ(a, static_cast<float>(p.x()), static_cast<float>(p.y()), 0.0f);
            }
            indigoFree(a);
        }
        indigoFree(atomIter);
    }

    // Snapshot before state for diffing. rebuildIndexTables() is NOT used
    // here: it only re-syncs indigo indices for AtomIds/BondIds this class
    // ALREADY tracks, guarded by "tracked count == current indigo count" --
    // it has no mechanism to register a brand-new atom/bond indigoMerge just
    // added (that guard fails the moment counts differ, silently leaving
    // every merged-in atom/bond untracked). New ids are assigned directly
    // below instead, the same way addAtom()/addBond() do.
    QSet<int> atomIdxBefore = QSet<int>(m_atomIdx.begin(), m_atomIdx.end());
    QSet<int> bondIdxBefore = QSet<int>(m_bondIdx.begin(), m_bondIdx.end());
    QSet<int> sgroupIdxBefore;
    {
        // indigoGetSuperatom(mol, i) takes an ABSOLUTE sgroup index across every sgroup type,
        // not a superatom-specific ordinal -- confirmed by direct probe: with a data sgroup at
        // absolute index 0 and a superatom at absolute index 1, looping i from 0 to
        // indigoCountSuperatoms()-1 (i.e. i=0 only, since there's 1 superatom) calls
        // indigoGetSuperatom(mol, 0), which targets the DATA sgroup and fails with "Sgroup with
        // index 0 is not a Superatom" -- silently swallowed by the `sup >= 0` guard, so the real
        // superatom at index 1 is never reached. indigoIterateSuperatoms is the correct,
        // type-safe iterator (mirrors indigoIterateDataSGroups) and has no such index confusion.
        int iter = indigoIterateSuperatoms(m_mol);
        if (iter >= 0) {
            int sup;
            while ((sup = indigoNext(iter)) > 0) { sgroupIdxBefore.insert(indigoIndex(sup)); indigoFree(sup); }
            indigoFree(iter);
        }
    }
    // Also record the clone's own atom index -> its position, so after
    // merging we can match destination atoms back to source indices by
    // iteration order (confirmed stable by direct probe).
    QList<int> cloneAtomIndicesInOrder;
    {
        int it = indigoIterateAtoms(clone);
        if (it >= 0) {
            int a;
            while ((a = indigoNext(it)) > 0) { cloneAtomIndicesInOrder.append(indigoIndex(a)); indigoFree(a); }
            indigoFree(it);
        }
    }

    int mergeRc = indigoMerge(m_mol, clone);
    indigoFree(clone);
    if (mergeRc < 0) { m_lastError = QString::fromUtf8(indigoGetLastError()); return result; }

    // New atoms: any post-merge indigo index not already tracked gets a
    // fresh, never-reused AtomId (same assignment addAtom() uses). indigoMerge
    // appends in the source's own order and leaves pre-existing atoms' own
    // indices unchanged (confirmed by direct probe), so iterating m_mol in
    // order and collecting just the untracked ones reproduces the source's
    // relative atom order -- these are NOT the same absolute index VALUES as
    // cloneAtomIndicesInOrder (the destination's own pre-existing atoms
    // already occupy the low indices, shifting the merged-in ones), so the
    // correspondence must be zipped POSITIONALLY, not matched by value.
    QList<AtomId> newAtomIds;
    {
        int it = indigoIterateAtoms(m_mol);
        if (it >= 0) {
            int a;
            while ((a = indigoNext(it)) > 0) {
                int idx = indigoIndex(a);
                if (!atomIdxBefore.contains(idx)) {
                    AtomId id = m_nextAtomId++;
                    m_atomIdx.insert(id, idx);
                    newAtomIds.append(id);
                }
                indigoFree(a);
            }
            indigoFree(it);
        }
    }
    result.createdAtoms = newAtomIds;
    for (int i = 0; i < newAtomIds.size() && i < cloneAtomIndicesInOrder.size(); ++i) {
        result.sourceIndexToNewAtomId.insert(cloneAtomIndicesInOrder[i], newAtomIds[i]);
    }

    // New bonds: same idea, using m_nextBondId.
    {
        int it = indigoIterateBonds(m_mol);
        if (it >= 0) {
            int b;
            while ((b = indigoNext(it)) > 0) {
                int idx = indigoIndex(b);
                if (!bondIdxBefore.contains(idx)) {
                    BondId id = m_nextBondId++;
                    m_bondIdx.insert(id, idx);
                    result.createdBonds.append(id);
                }
                indigoFree(b);
            }
            indigoFree(it);
        }
    }

    // New sgroups: any superatom index not present before the merge. Assign
    // each a fresh stable SGroupId, defaulting expanded=true (chem-core's own
    // default for sg.data.expanded). Uses indigoIterateSuperatoms, NOT an
    // indigoCountSuperatoms-bounded indigoGetSuperatom(mol, i) loop -- see the
    // sgroupIdxBefore block above for why that pattern is broken.
    int supIter = indigoIterateSuperatoms(m_mol);
    if (supIter >= 0) {
        int sup;
        while ((sup = indigoNext(supIter)) > 0) {
            int idx = indigoIndex(sup);
            indigoFree(sup);
            if (sgroupIdxBefore.contains(idx)) continue;
            SGroupId sgId = m_nextSGroupId++;
            m_sgroupIdx.insert(sgId, idx);
            m_sgroupExpanded.insert(sgId, true);
            result.createdSGroups.append(sgId);
        }
        indigoFree(supIter);
    }

    return result;
}

bool EditableMolecule::superatomAttachAtom(SGroupId id, AtomId& out) const {
    if (m_mol < 0 || !m_sgroupIdx.contains(id)) return false;
    activateSession();
    int sup = indigoGetSuperatom(m_mol, m_sgroupIdx.value(id));
    if (sup < 0) return false;
    int apIter = indigoIterateSGroupAttachmentPoints(sup);
    bool found = false;
    if (apIter >= 0) {
        int ap = indigoNext(apIter);
        if (ap > 0) {
            int atomIdx = indigoGetSGroupAttachmentPointAtomIdx(ap);
            for (auto it = m_atomIdx.constBegin(); it != m_atomIdx.constEnd(); ++it) {
                if (it.value() == atomIdx) { out = it.key(); found = true; break; }
            }
            indigoFree(ap);
        }
        indigoFree(apIter);
    }
    indigoFree(sup);
    return found;
}

void EditableMolecule::setSGroupExpanded(SGroupId id, bool expanded) {
    if (!m_sgroupIdx.contains(id)) return;
    m_sgroupExpanded.insert(id, expanded);
}

bool EditableMolecule::sgroupExpanded(SGroupId id) const {
    if (!m_sgroupIdx.contains(id)) return false;
    return m_sgroupExpanded.value(id, true);
}

QString EditableMolecule::stereoFlagsType() const { return m_ext.stereoFlagsType; }
int EditableMolecule::stereoFlagsGroupId() const { return m_ext.stereoFlagsGroupId; }
void EditableMolecule::setStereoFlagsDocument(const QString& type, int groupId) {
    m_ext.stereoFlagsType = type;
    m_ext.stereoFlagsGroupId = groupId;
}

QString EditableMolecule::atomCheckWarningText(AtomId id) const {
    return m_ext.atomCheckWarningTexts.value(id, QString());
}
QString EditableMolecule::bondCheckWarningText(BondId id) const {
    return m_ext.bondCheckWarningTexts.value(id, QString());
}

QString EditableMolecule::rxnArrowMode(RxnArrowId id) const {
    return m_ext.rxnArrows.value(id).mode;
}
QString EditableMolecule::rxnArrowConditionsAbove(RxnArrowId id) const {
    return m_ext.rxnArrows.value(id).conditionsAbove;
}
QString EditableMolecule::rxnArrowConditionsBelow(RxnArrowId id) const {
    return m_ext.rxnArrows.value(id).conditionsBelow;
}
bool EditableMolecule::rxnArrowCurvature(RxnArrowId id, double& x, double& y) const {
    if (!m_ext.rxnArrows.contains(id)) return false;
    const RxnArrow& arrow = m_ext.rxnArrows.value(id);
    if (!arrow.hasCurvature) return false;
    x = arrow.curvatureX;
    y = arrow.curvatureY;
    return true;
}

bool EditableMolecule::setRxnArrowMode(RxnArrowId id, const QString& mode) {
    if (!m_ext.rxnArrows.contains(id)) return false;
    m_ext.rxnArrows[id].mode = mode;
    return true;
}

bool EditableMolecule::setRxnArrowConditions(RxnArrowId id, const QString& above, const QString& below) {
    if (!m_ext.rxnArrows.contains(id)) return false;
    m_ext.rxnArrows[id].conditionsAbove = above;
    m_ext.rxnArrows[id].conditionsBelow = below;
    return true;
}

bool EditableMolecule::setRxnArrowCurvature(RxnArrowId id, double x, double y, bool has) {
    if (!m_ext.rxnArrows.contains(id)) return false;
    RxnArrow& r = m_ext.rxnArrows[id];
    r.hasCurvature = has;
    r.curvatureX = has ? x : 0;
    r.curvatureY = has ? y : 0;
    return true;
}

int EditableMolecule::implicitHydrogenCount(AtomId id) const {
    if (m_mol < 0 || !m_atomIdx.contains(id)) return 0;
    activateSession();
    int a = indigoGetAtom(m_mol, m_atomIdx.value(id));
    if (a < 0) return 0;
    // indigoCountHydrogens takes an out-parameter -- it does NOT return the count
    // directly (confirmed by the real compile error this produced: "too few
    // arguments to function 'int indigoCountHydrogens(int, int*)'"). It returns
    // 0 (not 1) when the count isn't definitely known (e.g. a query atom), in
    // which case *hydro is left unwritten -- h stays at its 0 initializer.
    int h = 0;
    indigoCountHydrogens(a, &h);
    indigoFree(a);
    return h > 0 ? h : 0;
}

QList<AtomId> EditableMolecule::neighborAtomIds(AtomId id) const {
    QList<AtomId> result;
    if (m_mol < 0 || !m_atomIdx.contains(id)) return result;
    for (auto it = m_bondIdx.constBegin(); it != m_bondIdx.constEnd(); ++it) {
        AtomId ea = -1, eb = -1;
        if (!bondEndpoints(it.key(), ea, eb)) continue;
        if (ea == id) result.append(eb);
        else if (eb == id) result.append(ea);
    }
    return result;
}

QList<EditableMolecule::RingMembership> EditableMolecule::ringMembership() const {
    QList<RingMembership> result;
    if (m_mol < 0) return result;
    activateSession();
    int sssr = indigoIterateSSSR(m_mol);
    if (sssr < 0) return result;
    int ring;
    while ((ring = indigoNext(sssr)) > 0) {
        RingMembership rm;
        int aIter = indigoIterateAtoms(ring);
        if (aIter >= 0) {
            int a;
            while ((a = indigoNext(aIter)) > 0) {
                int idx = indigoIndex(a);
                for (auto it = m_atomIdx.constBegin(); it != m_atomIdx.constEnd(); ++it) {
                    if (it.value() == idx) { rm.atoms.append(it.key()); break; }
                }
                indigoFree(a);
            }
            indigoFree(aIter);
        }
        int bIter = indigoIterateBonds(ring);
        if (bIter >= 0) {
            int b;
            while ((b = indigoNext(bIter)) > 0) {
                int idx = indigoIndex(b);
                for (auto it = m_bondIdx.constBegin(); it != m_bondIdx.constEnd(); ++it) {
                    if (it.value() == idx) { rm.bonds.append(it.key()); break; }
                }
                indigoFree(b);
            }
            indigoFree(bIter);
        }
        indigoFree(ring);
        result.append(rm);
    }
    indigoFree(sssr);
    return result;
}

int EditableMolecule::atomFragmentIndex(AtomId id) const {
    if (m_mol < 0 || !m_atomIdx.contains(id)) return -1;
    activateSession();
    int a = indigoGetAtom(m_mol, m_atomIdx.value(id));
    if (a < 0) return -1;
    int ci = indigoComponentIndex(a);
    indigoFree(a);
    return ci;
}

QList<AtomId> EditableMolecule::atomIdsInFragment(int fragIndex) const {
    QList<AtomId> result;
    for (AtomId id : atomIds()) {
        if (atomFragmentIndex(id) == fragIndex) result.append(id);
    }
    return result;
}

int EditableMolecule::stereocenterType(AtomId id) const {
    if (m_mol < 0 || !m_atomIdx.contains(id)) return 0;
    activateSession();
    int a = indigoGetAtom(m_mol, m_atomIdx.value(id));
    if (a < 0) return 0;
    int t = indigoStereocenterType(a);
    indigoFree(a);
    return t;
}

int EditableMolecule::stereocenterGroup(AtomId id) const {
    if (m_mol < 0 || !m_atomIdx.contains(id)) return 0;
    activateSession();
    int a = indigoGetAtom(m_mol, m_atomIdx.value(id));
    if (a < 0) return 0;
    int g = indigoStereocenterGroup(a);
    indigoFree(a);
    return g;
}

int EditableMolecule::bondStereoDirection(BondId id) const {
    if (m_mol < 0 || !m_bondIdx.contains(id)) return 0;
    activateSession();
    int b = indigoGetBond(m_mol, m_bondIdx.value(id));
    if (b < 0) return 0;
    int s = indigoBondStereo(b);
    indigoFree(b);
    return s;
}

int EditableMolecule::atomCipDescriptor(AtomId id) const {
    if (m_mol < 0 || !m_atomIdx.contains(id)) return 0;
    activateSession();
    int idx = m_atomIdx.value(id);
    int clone = indigoClone(m_mol);
    if (clone < 0) return 0;
    if (indigoAddCIPStereoDescriptors(clone) < 0) { indigoFree(clone); return 0; }
    int a = indigoGetAtom(clone, idx);
    if (a < 0) { indigoFree(clone); return 0; }
    int cip = indigoStereocenterCIPDescriptor(a);
    indigoFree(a);
    indigoFree(clone);
    return cip > 0 ? cip : 0;
}

QList<SGroupId> EditableMolecule::sgroupIds() const {
    return m_sgroupIdx.keys();
}

QList<AtomId> EditableMolecule::sgroupMemberAtomIds(SGroupId id) const {
    QList<AtomId> result;
    if (m_mol < 0 || !m_sgroupIdx.contains(id)) return result;
    activateSession();
    int sup = indigoGetSuperatom(m_mol, m_sgroupIdx.value(id));
    if (sup < 0) return result;
    int aIter = indigoIterateAtoms(sup);
    if (aIter >= 0) {
        int a;
        while ((a = indigoNext(aIter)) > 0) {
            int idx = indigoIndex(a);
            for (auto it = m_atomIdx.constBegin(); it != m_atomIdx.constEnd(); ++it) {
                if (it.value() == idx) { result.append(it.key()); break; }
            }
            indigoFree(a);
        }
        indigoFree(aIter);
    }
    indigoFree(sup);
    return result;
}

bool EditableMolecule::addRGroupEntry(int rgroupNumber) {
    if (m_ext.rgroups.contains(rgroupNumber)) return false;
    m_ext.rgroups.insert(rgroupNumber, RGroupEntry{});
    return true;
}

bool EditableMolecule::removeRGroupEntry(int rgroupNumber) {
    return m_ext.rgroups.remove(rgroupNumber) > 0;
}

bool EditableMolecule::setRGroupLogic(int rgroupNumber, const QString& range, bool resth, int ifthen) {
    if (!m_ext.rgroups.contains(rgroupNumber)) return false;
    RGroupEntry& rg = m_ext.rgroups[rgroupNumber];
    rg.range = range; rg.resth = resth; rg.ifthen = ifthen;
    return true;
}

bool EditableMolecule::addRGroupFragment(int rgroupNumber, int fragId) {
    if (!m_ext.rgroups.contains(rgroupNumber)) return false;
    RGroupEntry& rg = m_ext.rgroups[rgroupNumber];
    if (rg.fragIds.contains(fragId)) return false;
    rg.fragIds.append(fragId);
    return true;
}

bool EditableMolecule::removeRGroupFragment(int rgroupNumber, int fragId) {
    if (!m_ext.rgroups.contains(rgroupNumber)) return false;
    return m_ext.rgroups[rgroupNumber].fragIds.removeOne(fragId);
}

QList<int> EditableMolecule::rgroupNumbers() const { return m_ext.rgroups.keys(); }

bool EditableMolecule::rgroupLogic(int rgroupNumber, QString& range, bool& resth, int& ifthen) const {
    if (!m_ext.rgroups.contains(rgroupNumber)) return false;
    const RGroupEntry& rg = m_ext.rgroups.value(rgroupNumber);
    range = rg.range; resth = rg.resth; ifthen = rg.ifthen;
    return true;
}

QList<int> EditableMolecule::rgroupFragmentIds(int rgroupNumber) const {
    return m_ext.rgroups.value(rgroupNumber).fragIds;
}

void EditableMolecule::pushBracket(double minX, double minY, double maxX, double maxY) {
    m_ext.brackets.append(BracketBox{minX, minY, maxX, maxY});
}

bool EditableMolecule::popBracket() {
    if (m_ext.brackets.isEmpty()) return false;
    m_ext.brackets.removeLast();
    return true;
}

int EditableMolecule::bracketCount() const { return m_ext.brackets.size(); }

bool EditableMolecule::bracketAt(int index, double& minX, double& minY, double& maxX, double& maxY) const {
    if (index < 0 || index >= m_ext.brackets.size()) return false;
    const BracketBox& b = m_ext.brackets.at(index);
    minX = b.minX; minY = b.minY; maxX = b.maxX; maxY = b.maxY;
    return true;
}

