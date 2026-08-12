#ifndef FILEIO_H
#define FILEIO_H

#include <QObject>
#include <QFile>
#include <QTextStream>
#include <QUrl>
#include <QDebug>
#include <QFileInfo>
#include <QDir>
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
        out.flush();
        bool ok = (out.status() == QTextStream::Ok) && (file.error() == QFile::NoError);
        file.close();
        if (!ok) {
            qWarning() << "FileIO write failed:" << file.errorString();
        }
        return ok;
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

    Q_INVOKABLE bool exists(const QString& fileUrl) {
        QUrl url(fileUrl);
        return QFile::exists(url.isLocalFile() ? url.toLocalFile() : fileUrl);
    }

    Q_INVOKABLE bool remove(const QString& fileUrl) {
        QUrl url(fileUrl);
        return QFile::remove(url.isLocalFile() ? url.toLocalFile() : fileUrl);
    }

    Q_INVOKABLE bool mkpath(const QString& dirUrl) {
        QUrl url(dirUrl);
        QString path = url.isLocalFile() ? url.toLocalFile() : dirUrl;
        QDir dir;
        return dir.mkpath(path);
    }

    Q_INVOKABLE QStringList listFiles(const QString& dirUrl, const QString& filter) {
        QUrl url(dirUrl);
        QString path = url.isLocalFile() ? url.toLocalFile() : dirUrl;
        QDir dir(path);
        return dir.entryList(QStringList() << filter, QDir::Files);
    }

    Q_INVOKABLE QString readImageAsDataUri(const QUrl &fileUrl) {
        QFile file(fileUrl.toLocalFile());
        if (file.size() > 5 * 1024 * 1024) {
            return "";
        }
        if (!file.open(QIODevice::ReadOnly)) {
            return "";
        }
        QString suffix = QFileInfo(fileUrl.toLocalFile()).suffix().toLower();
        QString mimeType;
        if (suffix == "png") mimeType = "image/png";
        else if (suffix == "jpg" || suffix == "jpeg") mimeType = "image/jpeg";
        else if (suffix == "gif") mimeType = "image/gif";
        else if (suffix == "bmp") mimeType = "image/bmp";
        else return "";

        return "data:" + mimeType + ";base64," + QString(file.readAll().toBase64());
    }
};

#endif // FILEIO_H
