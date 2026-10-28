#include "BondAngleSuggester.h"
#define _USE_MATH_DEFINES
#include <cmath>
#include <QString>
#include <algorithm>

std::optional<double> BondAngleSuggester::suggestAngle(int fromAtomId, const QVariantMap& atomsById,
                                                          const QVariantList& bondsList) {
    QVariantMap fromAtom = atomsById.value(QString::number(fromAtomId)).toMap();
    if (fromAtom.isEmpty()) return std::nullopt;
    double cx = fromAtom.value(QStringLiteral("x")).toDouble();
    double cy = fromAtom.value(QStringLiteral("y")).toDouble();

    QList<double> existingAngles;
    QList<int> neighborIds;
    for (const QVariant& bVar : bondsList) {
        QVariantMap b = bVar.toMap();
        int beginId = b.value(QStringLiteral("begin")).toInt();
        int endId = b.value(QStringLiteral("end")).toInt();
        int otherId = -1;
        if (beginId == fromAtomId) otherId = endId;
        else if (endId == fromAtomId) otherId = beginId;
        if (otherId == -1) continue;
        QVariantMap other = atomsById.value(QString::number(otherId)).toMap();
        if (other.isEmpty()) continue;
        double ox = other.value(QStringLiteral("x")).toDouble();
        double oy = other.value(QStringLiteral("y")).toDouble();
        existingAngles.append(std::atan2(oy - cy, ox - cx));
        neighborIds.append(otherId);
    }

    if (existingAngles.size() == 1) {
        int neighborId = neighborIds[0];
        double existingAngle = existingAngles[0];
        double continuation = existingAngle + M_PI;

        double turnSign = 1.0;
        QVariantMap neighborAtom = atomsById.value(QString::number(neighborId)).toMap();
        if (!neighborAtom.isEmpty()) {
            double nx = neighborAtom.value(QStringLiteral("x")).toDouble();
            double ny = neighborAtom.value(QStringLiteral("y")).toDouble();
            for (const QVariant& bVar : bondsList) {
                QVariantMap b = bVar.toMap();
                int beginId = b.value(QStringLiteral("begin")).toInt();
                int endId = b.value(QStringLiteral("end")).toInt();
                int grandparentId = -1;
                if (beginId == neighborId && endId != fromAtomId) grandparentId = endId;
                else if (endId == neighborId && beginId != fromAtomId) grandparentId = beginId;
                if (grandparentId == -1) continue;
                QVariantMap grandparent = atomsById.value(QString::number(grandparentId)).toMap();
                if (grandparent.isEmpty()) continue;
                double gx = grandparent.value(QStringLiteral("x")).toDouble();
                double gy = grandparent.value(QStringLiteral("y")).toDouble();
                double dirIn = std::atan2(ny - gy, nx - gx);
                double dirOut = existingAngle + M_PI;
                double delta = dirOut - dirIn;
                while (delta > M_PI) delta -= 2 * M_PI;
                while (delta < -M_PI) delta += 2 * M_PI;
                turnSign = (delta >= 0) ? -1.0 : 1.0;
                break;
            }
        }
        return continuation + turnSign * (M_PI / 3.0);
    }

    if (existingAngles.size() == 2) {
        double lo = existingAngles[0];
        double hi = existingAngles[1];
        if (lo > hi) std::swap(lo, hi);
        double width1 = hi - lo;
        double width2 = 2 * M_PI - width1;
        return (width1 >= width2) ? (lo + width1 / 2.0) : (lo + width1 / 2.0 + M_PI);
    }

    return std::nullopt;
}

std::optional<double> BondAngleSuggester::suggestFallbackAngle(int fromAtomId, const QVariantMap& atomsById,
                                                                 const QVariantList& bondsList) {
    QVariantMap fromAtom = atomsById.value(QString::number(fromAtomId)).toMap();
    if (fromAtom.isEmpty()) return std::nullopt;
    double cx = fromAtom.value(QStringLiteral("x")).toDouble();
    double cy = fromAtom.value(QStringLiteral("y")).toDouble();

    QList<double> neighbors;
    for (const QVariant& bVar : bondsList) {
        QVariantMap b = bVar.toMap();
        int beginId = b.value(QStringLiteral("begin")).toInt();
        int endId = b.value(QStringLiteral("end")).toInt();
        int otherId = -1;
        if (beginId == fromAtomId) otherId = endId;
        else if (endId == fromAtomId) otherId = beginId;
        if (otherId == -1) continue;
        QVariantMap other = atomsById.value(QString::number(otherId)).toMap();
        if (other.isEmpty()) continue;
        double ox = other.value(QStringLiteral("x")).toDouble();
        double oy = other.value(QStringLiteral("y")).toDouble();
        neighbors.append(std::atan2(oy - cy, ox - cx));
    }

    if (neighbors.isEmpty()) return 0.0;
    if (neighbors.size() == 1) return neighbors.first() + 2.61799;   // matches
                                                                       // V8Process::getLargestEmptyAngle's
                                                                       // own 1-neighbor constant exactly

    std::sort(neighbors.begin(), neighbors.end());
    double maxGap = 0;
    double bestAngle = 0;
    for (int i = 0; i < neighbors.size(); i++) {
        double a1 = neighbors[i];
        double a2 = neighbors[(i + 1) % neighbors.size()];
        double gap = a2 - a1;
        while (gap <= 0) gap += 2 * M_PI;
        if (gap > maxGap) { maxGap = gap; bestAngle = a1 + gap / 2; }
    }
    return bestAngle;
}
