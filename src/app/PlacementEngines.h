#pragma once
#include <QObject>
#include <QPointF>
#include <QString>
#include <QList>
#include <QVariantMap>

struct PreviewAtom { 
    QPointF pos; 
    QString label; 
};

struct PreviewBond { 
    QPointF start; 
    QPointF end; 
};

struct PlacementResult {
    bool valid = false;
    QList<PreviewAtom> atoms;
    QList<PreviewBond> bonds;
};

class AtomPlacementEngine {
public:
    static PlacementResult compute(const QPointF& startPos, const QPointF& currentMouse, 
                                   const QString& toolAtom, const QList<double>& existingAngles, 
                                   double bondLength);
private:
    static double snapAngle(double rawAngle, const QList<double>& existingAngles);
};

class FragmentPlacementEngine {
public:
    static PlacementResult compute(const QPointF& startPos, const QPointF& currentMouse, 
                                   const QString& fragmentId, const QList<double>& existingAngles, 
                                   double bondLength);
};
