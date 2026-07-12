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



