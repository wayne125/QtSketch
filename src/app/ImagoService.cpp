#include "ImagoService.h"
#include "imago_c.h"
#include <QtConcurrent>
#include <QPointer>
#include <QCoreApplication>
#include <QMetaObject>

static void emitOnGuiThread(QPointer<ImagoService> self, std::function<void(ImagoService *)> fn) {
    QMetaObject::invokeMethod(qApp, [self, fn]() {
        if (self) fn(self.data());
    }, Qt::QueuedConnection);
}

ImagoService::ImagoService(QObject *parent) : QObject(parent) {}
ImagoService::~ImagoService() {}

void ImagoService::recognizeImage(const QUrl &fileUrl) {
    QString path = fileUrl.toLocalFile();
    if (path.isEmpty()) { emit imageRecognized("", 0, "Invalid file path."); return; }
    QPointer<ImagoService> self = this;
    (void)QtConcurrent::run([self, path]() {
        QString result, error;
        int warnings = 0;
        imago_qword sid = imagoAllocSessionId();
        imagoSetSessionId(sid);
        try {
            if (imagoLoadImageFromFile(path.toUtf8().constData())) {
                if (imagoFilterImage()) {
                    if (imagoRecognize(&warnings)) {
                        const char* mol = imagoGetMol();
                        if (mol) result = QString::fromUtf8(mol);
                        else error = QString::fromUtf8(imagoGetLastError());
                    } else {
                        error = QString::fromUtf8(imagoGetLastError());
                    }
                } else {
                    error = QString("Image filtering failed: %1").arg(imagoGetLastError());
                }
            } else {
                error = QString("Failed to load image: %1").arg(imagoGetLastError());
            }
        } catch (const std::exception &e) {
            error = QString::fromUtf8(e.what());
        } catch (...) {
            error = "Unknown exception during image recognition.";
        }
        imagoReleaseSessionId(sid);
        emitOnGuiThread(self, [result, warnings, error](ImagoService *s) {
            emit s->imageRecognized(result, warnings, error);
        });
    });
}
