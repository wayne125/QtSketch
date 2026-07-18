#pragma once
#include <QObject>
#include <QtQml/qqml.h>
#include <QPointF>
#include <QUrl>
#include "PlacementPreviewManager.h"

#include "v8_process.h"

class AppController : public QObject {
    Q_OBJECT
    QML_NAMED_ELEMENT(AppController)
    QML_SINGLETON

public:
    explicit AppController(QObject* parent = nullptr);
    
    Q_INVOKABLE void setV8Process(V8Process* v8);

    Q_INVOKABLE void handleDragStart(const QString& toolId, int hitAtomId, double pressX, double pressY, double chemX, double chemY);
    Q_INVOKABLE void handleDrag(double mouseX, double mouseY, double chemScale, double bondLength);
    Q_INVOKABLE bool handleDragEnd();

    // Chain tool's live drag preview - deliberately independent of handleDragStart/handleDrag/
    // handleDragEnd and PlacementPreviewManager: those are gated on dragging from an existing
    // atom (hitAtomId >= 0) and their commit path assumes single-atom-attach or ring semantics,
    // neither of which fits a chain that can start from empty canvas and commits via the
    // existing V8Process::addChain call already made from ChemCanvas.qml. This only touches the
    // overlay's previewAtoms/previewBonds for rendering; it never mutates the real structure.
    Q_INVOKABLE void updateChainPreview(double startChemX, double startChemY, double currentChemX, double currentChemY, double bondLength);
    Q_INVOKABLE void clearChainPreview();

    Q_INVOKABLE void copyImageToClipboard(const QUrl &imageUrl);
    Q_INVOKABLE QString tempPngPath() const;

    // Draws the (already-exported) PNG at imageUrl centered on a single PDF page
    // sized pageWidthMm x pageHeightMm with marginMm on every side. Returns true
    // on success. Print-to-PDF instead of a native print dialog: this app is a
    // QGuiApplication (no QtWidgets), so QPrintDialog isn't available without a
    // riskier Widgets migration this late in the session.
    Q_INVOKABLE bool exportPdf(const QUrl &imageUrl, const QUrl &pdfUrl,
                                double pageWidthMm, double pageHeightMm, double marginMm);

private:
    V8Process* m_v8 = nullptr;
    PlacementPreviewManager* m_previewManager = nullptr;
};



