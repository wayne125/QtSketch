#include "PlacementPreviewManager.h"
#include "v8_process.h"
#include <QJsonArray>
#include <QJsonObject>
#include <QJsonDocument>
#include <cmath>
#include <QDebug>

PlacementPreviewManager::PlacementPreviewManager(V8Process* v8, QObject* parent)
    : QObject(parent), m_v8(v8), m_active(false), m_startAtomId(-1)
{
}

void PlacementPreviewManager::beginPreview(int startAtomId, const QPointF& startPixelPos, const QPointF& startChemPos, const QString& toolId) {
    m_active = true;
    m_startAtomId = startAtomId;
    m_startPixelPos = startPixelPos;
    m_startChemPos = startChemPos;
    m_toolId = toolId;
    m_lastResult.valid = false;
    m_cachedAngles.clear();
    if (m_v8) {
        QVariantMap primitives = m_v8->primitives();
        QVariantList bonds = primitives.value("bonds").toList();
        QVariantMap atomsById = primitives.value("atomsById").toMap();
        for (const QVariant& bv : bonds) {
            QVariantMap b = bv.toMap();
            int beginId = b.value("begin").toInt();
            int endId = b.value("end").toInt();
            QVariantMap beginAtom = atomsById.value(QString::number(beginId)).toMap();
            QVariantMap endAtom = atomsById.value(QString::number(endId)).toMap();
            if (beginAtom.isEmpty() || endAtom.isEmpty()) continue;
            double bx = beginAtom.value("x").toDouble(), by = beginAtom.value("y").toDouble();
            double ex = endAtom.value("x").toDouble(), ey = endAtom.value("y").toDouble();
            m_cachedAngles[beginId].append(std::atan2(ey - by, ex - bx));
            m_cachedAngles[endId].append(std::atan2(by - ey, bx - ex));
        }
    }
}

QList<double> PlacementPreviewManager::getExistingAngles(int atomId) const {
    return m_cachedAngles.value(atomId);
}

void PlacementPreviewManager::updatePreview(const QPointF& currentMouse, double chemScale, double bondLength) {
    if (!m_active) return;
    if (chemScale == 0.0) return; // guard against a divide-by-zero if ever called with a degenerate scale

    QPointF currentChem(m_startChemPos.x() + (currentMouse.x() - m_startPixelPos.x()) / chemScale,
                        m_startChemPos.y() + (currentMouse.y() - m_startPixelPos.y()) / chemScale);
    
    QList<double> angles = getExistingAngles(m_startAtomId);
    
    if (m_toolId.startsWith("ATOM_")) {
        QString label = m_toolId.mid(5); 
        m_lastResult = AtomPlacementEngine::compute(m_startChemPos, currentChem, label, angles, bondLength);
    } else if (m_toolId.startsWith("FG_") || m_toolId.startsWith("SS_") || m_toolId.startsWith("LIB_") || m_toolId.startsWith("TEMPLATE_")) {
        m_lastResult = FragmentPlacementEngine::compute(m_startChemPos, currentChem, m_toolId, angles, bondLength);
    }
    
    publishToOverlay();
}

void PlacementPreviewManager::publishToOverlay() {
    if (!m_v8) return;
    
    QVariantMap overlay = m_v8->overlayState();
    
    QVariantList pAtoms;
    for (const auto& a : m_lastResult.atoms) {
        QVariantMap amap;
        amap["x"] = a.pos.x();
        amap["y"] = a.pos.y();
        amap["label"] = a.label;
        pAtoms.append(amap);
    }
    
    QVariantList pBonds;
    for (const auto& b : m_lastResult.bonds) {
        QVariantMap bmap;
        bmap["startX"] = b.start.x();
        bmap["startY"] = b.start.y();
        bmap["endX"] = b.end.x();
        bmap["endY"] = b.end.y();
        pBonds.append(bmap);
    }
    
    overlay["previewAtoms"] = pAtoms;
    overlay["previewBonds"] = pBonds;
    
    m_v8->setOverlayState(overlay);
}

PlacementResult PlacementPreviewManager::commitPreview() {
    if (!m_active) {
        return PlacementResult();
    }
    PlacementResult res = m_lastResult;
    m_active = false;
    m_cachedAngles.clear();
    m_lastResult.valid = false;
    return res;
}

void PlacementPreviewManager::cancelPreview() {
    m_active = false;
    m_cachedAngles.clear();
    m_lastResult.valid = false;
    
    if (m_v8) {
        QVariantMap overlay = m_v8->overlayState();
        overlay.remove("previewAtoms");
        overlay.remove("previewBonds");
        m_v8->setOverlayState(overlay);
    }
}
