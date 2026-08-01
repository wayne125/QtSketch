// src/app/molecule/EditableMolecule.cpp
#include "EditableMolecule.h"
#include "indigo.h"

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
