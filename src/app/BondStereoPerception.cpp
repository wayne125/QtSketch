#include "BondStereoPerception.h"
#include "indigo.h"
#include <QByteArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QString>
#include <algorithm>

// Assumes generateName()'s multi-component rejection (indigoCountComponents(mol) > 1)
// stays in place: Indigo's JSON saver gives each disconnected component its own,
// separately-re-based "mol0"/"mol1"/... node with LOCAL atom indices, and this
// function flattens every molecule node's bonds into one map keyed only by index --
// if multi-component naming is ever supported, index collisions across components
// would silently attach a wrong E/Z letter to the wrong bond.
std::map<std::pair<int,int>, QChar> computeIndigoBondCIP(int mol) {
    std::map<std::pair<int,int>, QChar> result;
    // Assumes this runs in a dedicated/throwaway Indigo session (matching
    // IndigoService.cpp's identical pattern) -- this option is process/session-global
    // and is never reset, so setting it on the shared main session would make every
    // future indigoJson()/toKetJson() call on that session also emit "cip" fields.
    indigoSetOptionBool("json-saving-add-stereo-desc", 1);
    const char* ketStr = indigoJson(mol);
    if (!ketStr) return result;
    QJsonDocument ketDoc = QJsonDocument::fromJson(QByteArray(ketStr));
    QJsonObject ketRoot = ketDoc.object();
    QJsonArray nodes = ketRoot.value("root").toObject().value("nodes").toArray();
    for (const QJsonValue &nodeVal : nodes) {
        QString ref = nodeVal.toObject().value("$ref").toString();
        if (ref.isEmpty()) continue;
        QJsonObject molObj = ketRoot.value(ref).toObject();
        if (molObj.value("type").toString() != "molecule") continue;
        const QJsonArray bonds = molObj.value("bonds").toArray();
        for (const QJsonValue &bondVal : bonds) {
            QJsonObject bondObj = bondVal.toObject();
            QString label = bondObj.value("cip").toString();
            if (label != "E" && label != "Z") continue;
            QJsonArray bondAtoms = bondObj.value("atoms").toArray();
            if (bondAtoms.size() != 2) continue;
            int a1 = bondAtoms.at(0).toInt();
            int a2 = bondAtoms.at(1).toInt();
            result[{std::min(a1, a2), std::max(a1, a2)}] = label == "Z" ? QChar('Z') : QChar('E');
        }
    }
    return result;
}
