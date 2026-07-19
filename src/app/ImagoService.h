#ifndef IMAGOSERVICE_H
#define IMAGOSERVICE_H

#include <QObject>
#include <QString>
#include <QUrl>
#include <QtQml/qqml.h>

class ImagoService : public QObject {
    Q_OBJECT
    QML_ELEMENT
public:
    explicit ImagoService(QObject *parent = nullptr);
    ~ImagoService();

    Q_INVOKABLE void recognizeImage(const QUrl &fileUrl);

Q_SIGNALS:
    void imageRecognized(const QString &molfile, int warningsCount, const QString &error);
};

#endif // IMAGOSERVICE_H
