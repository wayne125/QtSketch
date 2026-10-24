#include "RetainedRingTypes.h"
#include "indigo.h"
#include <set>

QString validateRetainedRingSystem(int /*mol*/, const RetainedRingSystem &sys) {
    int ref = indigoLoadMoleculeFromString(sys.referenceSmiles.toUtf8().constData());
    if (ref < 0) {
        return QString("'%1': referenceSmiles failed to parse: %2")
            .arg(sys.name).arg(indigoGetLastError());
    }
    int atomCount = 0;
    int iter = indigoIterateAtoms(ref);
    while (int atom = indigoNext(iter)) { ++atomCount; indigoFree(atom); }
    indigoFree(iter);
    indigoFree(ref);

    if (atomCount != static_cast<int>(sys.peripheralLocants.size())) {
        return QString("'%1': referenceSmiles has %2 atoms but peripheralLocants has %3 entries")
            .arg(sys.name).arg(atomCount).arg(sys.peripheralLocants.size());
    }

    std::set<QString> seen;
    for (const auto &loc : sys.peripheralLocants) {
        if (!seen.insert(loc).second) {
            return QString("'%1': duplicate locant '%2' in peripheralLocants")
                .arg(sys.name).arg(loc);
        }
    }

    std::set<QString> interiorSet;
    for (const auto &loc : sys.interiorLocants) {
        if (!seen.count(loc)) {
            return QString("'%1': interior locant '%2' not present in peripheralLocants")
                .arg(sys.name).arg(loc);
        }
        if (!interiorSet.insert(loc).second) {
            return QString("'%1': duplicate locant '%2' in interiorLocants")
                .arg(sys.name).arg(loc);
        }
    }

    std::vector<QString> outerLocants;
    for (const auto &loc : sys.peripheralLocants) {
        if (!interiorSet.count(loc)) {
            outerLocants.push_back(loc);
        }
    }
    int m = static_cast<int>(outerLocants.size());

    for (const auto &kv : sys.peripheralBondLetters) {
        const QString &locA = kv.second.first;
        const QString &locB = kv.second.second;
        if (interiorSet.count(locA) || interiorSet.count(locB)) {
            return QString("'%1': bond letter '%2' references interior locant (%3,%4)")
                .arg(sys.name).arg(QChar(kv.first)).arg(locA).arg(locB);
        }
        int idxA = -1, idxB = -1;
        for (int i = 0; i < m; ++i) {
            if (outerLocants[i] == locA) idxA = i;
            if (outerLocants[i] == locB) idxB = i;
        }
        if (idxA == -1 || idxB == -1) {
            return QString("'%1': bond letter '%2' references locant(s) not present in peripheralLocants")
                .arg(sys.name).arg(QChar(kv.first));
        }
        bool adjacent = ((idxA + 1) % m == idxB) || ((idxB + 1) % m == idxA);
        if (!adjacent) {
            return QString("'%1': bond letter '%2' pair (%3,%4) is not cyclically adjacent in peripheralLocants")
                .arg(sys.name).arg(QChar(kv.first)).arg(locA).arg(locB);
        }
    }
    return QString();
}
