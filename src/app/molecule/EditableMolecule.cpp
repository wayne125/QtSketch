// src/app/molecule/EditableMolecule.cpp
#include "EditableMolecule.h"
#include "MoleculeSnapshot.h"
#include "indigo.h"
#include <algorithm>

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
    m_ext.texts.append({x, y, content});
    return m_ext.texts.size() - 1;
}
int EditableMolecule::textAnnotationCount() const { return m_ext.texts.size(); }

int EditableMolecule::addRxnArrow(double x1, double y1, double x2, double y2) {
    m_ext.rxnArrows.append({x1, y1, x2, y2});
    return m_ext.rxnArrows.size() - 1;
}
int EditableMolecule::rxnArrowCount() const { return m_ext.rxnArrows.size(); }

int EditableMolecule::addRxnPlus(double x, double y) {
    m_ext.rxnPluses.append({x, y});
    return m_ext.rxnPluses.size() - 1;
}
int EditableMolecule::rxnPlusCount() const { return m_ext.rxnPluses.size(); }

int EditableMolecule::addMultitailArrow(const QList<double>& headAndTails) {
    m_ext.multitailArrows.append({headAndTails});
    return m_ext.multitailArrows.size() - 1;
}
int EditableMolecule::multitailArrowCount() const { return m_ext.multitailArrows.size(); }

int EditableMolecule::addImage(double x, double y, double w, double h, const QByteArray& pngData) {
    m_ext.images.append({x, y, w, h, pngData});
    return m_ext.images.size() - 1;
}
int EditableMolecule::imageCount() const { return m_ext.images.size(); }

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
