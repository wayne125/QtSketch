#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QFile>
#include <QTextStream>

#include <QUrl>
#include "IndigoService.h"
#include "FileIO.h"
#include "AppController.h"

static QFile* g_logFile = nullptr;
static QTextStream* g_logStream = nullptr;

void myMessageOutput(QtMsgType type, const QMessageLogContext &context, const QString &msg)
{
    if (g_logStream) {
        *g_logStream << msg << "\n";
        g_logStream->flush();
    }
    // Also print to stderr so we can see it in terminal!
    fprintf(stderr, "%s\n", msg.toLocal8Bit().constData());
    fflush(stderr);
}

int main(int argc, char *argv[])
{
    g_logFile = new QFile("qml_errors.log");
    if (g_logFile->open(QIODevice::WriteOnly | QIODevice::Truncate | QIODevice::Text)) {
        g_logStream = new QTextStream(g_logFile);
    }
    
    qInstallMessageHandler(myMessageOutput);


    QGuiApplication app(argc, argv);
    // Required for the QML Settings backing store (tool-panel section states)
    app.setOrganizationName("sketch");
    app.setApplicationName("sketch");
    QQmlApplicationEngine engine;
        
    engine.loadFromModule("Sketch.App", "MainWindow");
    if (engine.rootObjects().isEmpty()) {
        qWarning() << "Error: rootObjects is empty! Failed to load QML.";
        return -1;
    }

    return app.exec();
}


