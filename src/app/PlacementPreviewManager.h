#pragma once
#include <QObject>
#include <QPointF>
#include <QString>
#include <QVariantMap>
#include "PlacementEngines.h"

class V8Process;

class PlacementPreviewManager : public QObject {
    Q_OBJECT
public:
    explicit PlacementPreviewManager(V8Process* v8, QObject* parent = nullptr);
    
    void beginPreview(int startAtomId, const QPointF& startPixelPos, const QPointF& startChemPos, const QString& toolId);
    void updatePreview(const QPointF& currentMouse, double bondLength);
    PlacementResult commitPreview();
    void cancelPreview();

private:
    V8Process* m_v8;
    bool m_active;
    int m_startAtomId;
public:
    int startAtomId() const { return m_startAtomId; }
    QPointF startPixelPos() const { return m_startPixelPos; }
    QPointF startChemPos() const { return m_startChemPos; }
    QString toolId() const { return m_toolId; }
private:
    QPointF m_startPixelPos;
    QPointF m_startChemPos;
    QString m_toolId;
    PlacementResult m_lastResult;

    QList<double> getExistingAngles(int atomId) const;
    void publishToOverlay();
};


