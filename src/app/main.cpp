#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QQuickStyle>
#include <QStyleHints>
#include <QFile>
#include <QTextStream>
#include <QStandardPaths>
#include <QDir>

#include <QUrl>
#include "IndigoService.h"
#include "ImagoService.h"
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
    // FluentWinUI3 gives every stock control the Windows 11 Fluent metrics
    // (32px control height, 4px radii, Fluent focus rings) instead of the Basic
    // style's touch-sized 100x40 buttons and 140x40 combo boxes, which were the
    // single cause of the oversized caption buttons, page-size box and zoom
    // slider. Must be set before the QGuiApplication is constructed.
    QQuickStyle::setStyle("FluentWinUI3");

    QGuiApplication app(argc, argv);
    // The app ships a single light theme. Without pinning the colour scheme the
    // Fluent style follows the OS setting, so every stock control (dialogs,
    // text fields, combo boxes) rendered dark inside the light app on a machine
    // set to dark mode -- the defect that made the dialogs look foreign.
    app.styleHints()->setColorScheme(Qt::ColorScheme::Light);
    // Required for the QML Settings backing store (tool-panel section states)
    app.setOrganizationName("sketch");
    app.setApplicationName("sketch");

    // AppDataLocation depends on organizationName/applicationName above, so this
    // must run after they're set. A relative "qml_errors.log" resolved to whatever
    // the launch CWD happened to be, and Truncate wiped a crashing run's log
    // evidence on the very next launch -- use a stable writable path and append.
    QString logDir = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
    QDir().mkpath(logDir);
    g_logFile = new QFile(logDir + "/qml_errors.log");
    if (g_logFile->open(QIODevice::WriteOnly | QIODevice::Append | QIODevice::Text)) {
        g_logStream = new QTextStream(g_logFile);
    }

    qInstallMessageHandler(myMessageOutput);

    QQmlApplicationEngine engine;
        
    engine.loadFromModule("Sketch.App", "MainWindow");
    if (engine.rootObjects().isEmpty()) {
        qWarning() << "Error: rootObjects is empty! Failed to load QML.";
        return -1;
    }

    return app.exec();
}


