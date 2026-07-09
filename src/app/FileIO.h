#ifndef FILEIO_H
#define FILEIO_H

#include <QObject>
#include <QFile>
#include <QTextStream>
#include <QUrl>
#include <QDebug>
#include <QtQml/qqml.h>

class FileIO : public QObject {
    Q_OBJECT
    QML_ELEMENT
public:
    Q_INVOKABLE bool write(const QString& fileUrl, const QString& data) {
        QUrl url(fileUrl);
        QFile file(url.isLocalFile() ? url.toLocalFile() : fileUrl);
        if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
            qWarning() << "FileIO write failed:" << file.errorString();
            return false;
        }
        QTextStream out(&file);
        out << data;
        file.close();
        return true;
    }

    Q_INVOKABLE QString read(const QString& fileUrl) {
        QUrl url(fileUrl);
        QFile file(url.isLocalFile() ? url.toLocalFile() : fileUrl);
        if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
            qWarning() << "FileIO read failed:" << file.errorString();
            return "";
        }
        QTextStream in(&file);
        return in.readAll();
    }
};

#endif // FILEIO_H
