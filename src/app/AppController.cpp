#include "AppController.h"
#include "PlacementEngines.h"
#include "v8_process.h"
#include <QDebug>
#include <QJsonArray>
#include <QJsonObject>
#include <QJsonDocument>
#include <QGuiApplication>
#include <QClipboard>
#include <QImage>
#include <QStandardPaths>
#include <QDir>
#include <QUrl>
#include <QPdfWriter>
#include <QPainter>
#include <QPageSize>
#include <QPageLayout>

namespace {
// handleDragEnd (Q_INVOKABLE, no arguments) has no access to screen-space
// mouse coordinates or the live chemScale at commit time -- threading either
// through would mean touching PlacementPreviewManager and/or AppController.h
// plus their QML call sites, all outside this fix's scope. Caching the most
// recent chemScale seen via handleDrag, confined to this translation unit,
// is the smaller, more surgical way to make the real-drag threshold below
// scale-aware.
double s_lastChemScale = 1.0;
}

AppController::AppController(QObject* parent) : QObject(parent) {}

void AppController::setV8Process(V8Process* v8) {
    m_v8 = v8;
    if (m_previewManager) m_previewManager->deleteLater();
    m_previewManager = new PlacementPreviewManager(v8, this);
}

void AppController::handleDragStart(const QString& toolId, int hitAtomId, double pressX, double pressY, double chemX, double chemY) {
    if (!m_previewManager) return;
    if (hitAtomId >= 0 && (toolId.startsWith("ATOM_") || toolId.startsWith("FG_") || toolId.startsWith("SS_") || toolId.startsWith("LIB_") || toolId.startsWith("TEMPLATE_"))) {
        m_previewManager->beginPreview(hitAtomId, QPointF(pressX, pressY), QPointF(chemX, chemY), toolId);
    }
}

void AppController::handleDrag(double mouseX, double mouseY, double chemScale, double bondLength) {
    if (chemScale > 0.0) s_lastChemScale = chemScale;
    if (m_previewManager) {
        m_previewManager->updatePreview(QPointF(mouseX, mouseY), chemScale, bondLength);
    }
}

bool AppController::handleDragEnd() {
    if (m_previewManager) {
        PlacementResult result = m_previewManager->commitPreview();
        if (result.valid && m_v8) {
            bool isRealDrag = false;
            if (result.atoms.size() > 0) {
                double dx = result.atoms[0].pos.x() - m_previewManager->startChemPos().x();
                double dy = result.atoms[0].pos.y() - m_previewManager->startChemPos().y();
                // chemScale is pixels-per-chem-unit (ChemCanvas.qml), so a chem-space
                // delta of (dx, dy) corresponds to an on-screen pixel delta of
                // (dx*chemScale, dy*chemScale). Converting back to pixel space before
                // comparing keeps this "was it a real drag?" check consistent across
                // zoom levels instead of a fixed chem-space threshold whose effective
                // pixel sensitivity swings with chemScale. 4 (px^2, i.e. ~2px) matches
                // the equivalent real-drag-vs-click threshold already used for image
                // dragging in ChemCanvas.qml.
                double screenDx = dx * s_lastChemScale;
                double screenDy = dy * s_lastChemScale;
                if (screenDx*screenDx + screenDy*screenDy > 4.0) isRealDrag = true;
            }
            if (!isRealDrag) { m_previewManager->cancelPreview(); return false; }
            // Send IPC command to v8_worker to finalize addition
            
            // We'll create a generic bulk add operation, or use existing methods.
            // For now, if it's just a single bond and atom:
            // For FGs and Atoms, we just need a single bond
            if (result.atoms.size() == 1 && result.bonds.size() == 1) {
                QString label = result.atoms[0].label;
                double x = result.atoms[0].pos.x();
                double y = result.atoms[0].pos.y();
                
                QString toolId = m_previewManager->toolId();
                if (toolId.startsWith("FG_") || toolId.startsWith("SS_") || toolId.startsWith("LIB_")) {
                    QString fgName = toolId.startsWith("LIB_") ? toolId.mid(4) : toolId.mid(3);
                    m_v8->insertFunctionalGroup(fgName, x, y, m_previewManager->startAtomId(), toolId.startsWith("FG_") ? m_fgFullStructure : false);
                } else {
                    m_v8->addBondAndAtom(m_previewManager->startAtomId(), label, x, y, 1, 0); 
                }

            } else if (result.atoms.size() > 1) {
                QList<QVariant> coords;
                for (const auto& a : result.atoms) {
                    coords.append(a.pos.x());
                    coords.append(a.pos.y());
                }
                // Only the benzene tool produces an aromatic (alternating) ring;
                // numeric TEMPLATE_N tools place saturated rings.
                m_v8->addRing(coords, m_previewManager->toolId() == QStringLiteral("TEMPLATE_BENZENE"));
            }
            m_previewManager->cancelPreview();
            return true;
        }
        m_previewManager->cancelPreview();
        return false;
    }
    return false;
}

void AppController::updateChainPreview(double startChemX, double startChemY, double currentChemX, double currentChemY, double bondLength) {
    if (!m_v8) return;
    PlacementResult result = ChainPlacementEngine::compute(
        QPointF(startChemX, startChemY), QPointF(currentChemX, currentChemY), bondLength);

    QVariantMap overlay = m_v8->overlayState();
    QVariantList pAtoms;
    for (const auto& a : result.atoms) {
        QVariantMap amap;
        amap["x"] = a.pos.x();
        amap["y"] = a.pos.y();
        amap["label"] = a.label;
        pAtoms.append(amap);
    }
    QVariantList pBonds;
    for (const auto& b : result.bonds) {
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

void AppController::clearChainPreview() {
    if (!m_v8) return;
    QVariantMap overlay = m_v8->overlayState();
    overlay.remove("previewAtoms");
    overlay.remove("previewBonds");
    m_v8->setOverlayState(overlay);
}

void AppController::copyImageToClipboard(const QUrl &imageUrl) {
    QImage img(imageUrl.toLocalFile());
    if (!img.isNull())
        QGuiApplication::clipboard()->setImage(img);
}

bool AppController::exportPdf(const QUrl &imageUrl, const QUrl &pdfUrl,
                               double pageWidthMm, double pageHeightMm, double marginMm) {
    QString path = imageUrl.toLocalFile();
    if (!imageUrl.isLocalFile() || path.isEmpty()) return false;

    QImage img(path);
    if (img.isNull()) return false;

    QPdfWriter writer(pdfUrl.toLocalFile());
    writer.setPageSize(QPageSize(QSizeF(pageWidthMm, pageHeightMm), QPageSize::Millimeter));
    writer.setPageMargins(QMarginsF(marginMm, marginMm, marginMm, marginMm), QPageLayout::Millimeter);
    writer.setResolution(300);

    QPainter painter(&writer);
    const QRect target = painter.viewport();
    QSize scaled = img.size();
    scaled.scale(target.size(), Qt::KeepAspectRatio);
    QRect centered(target.x() + (target.width() - scaled.width()) / 2,
                    target.y() + (target.height() - scaled.height()) / 2,
                    scaled.width(), scaled.height());
    painter.drawImage(centered, img);
    return painter.end();
}

QString AppController::tempPngPath() const {
    return QDir(QStandardPaths::writableLocation(QStandardPaths::TempLocation)).filePath("sketch_export.png");
}
