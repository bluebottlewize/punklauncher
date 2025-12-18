#include <QApplication>
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include <QDir>
#include <QStandardPaths>
#include <QSettings>
#include <QRegularExpression>
#include <QWindow>
#include <QProcess>
#include <QDebug>
#include <QUrl>

#include <LayerShellQt/Window>

// #include "modules/AppBackend.h"
#include "core/Backend.hpp"
#include "modules/IconProvider.h"
#include "modules/AnsiParser/AnsiParser.hpp"

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    QQmlApplicationEngine engine;

    Backend backend;

    engine.rootContext()->setContextProperty("backend", &backend);
    engine.addImageProvider("appicon", new IconProvider);

    qmlRegisterType<AnsiParser>("TerminalTools", 1, 0, "AnsiParser");

    const QUrl url(QStringLiteral("qrc:/qml/Main.qml"));

    // Connect error signal to exit if load fails
    QObject::connect(&engine, &QQmlApplicationEngine::objectCreated,
                     &app, [url](QObject *obj, const QUrl &objUrl) {
                         if (!obj && url == objUrl) {
                             qCritical() << "ERROR: Failed to load QML file! Check your resource prefix.";
                             QCoreApplication::exit(-1);
                         }
                     }, Qt::QueuedConnection);

    engine.load(url);

    // 3. Debugging: Check if loaded
    if (engine.rootObjects().isEmpty()) {
        qCritical() << "CRITICAL: The QML engine loaded nothing. App will exit.";
        return -1;
    } else {
        qDebug() << "SUCCESS: QML loaded successfully.";
    }

    // 4. Wayland Layer Shell Setup
    QWindow *window = qobject_cast<QWindow*>(engine.rootObjects().first());

    if (window) {
        window->create(); // Ensure platform window exists

        if (auto *lsWindow = LayerShellQt::Window::get(window)) {
            qDebug() << "Attaching to Wayland Layer Shell...";
            lsWindow->setLayer(LayerShellQt::Window::LayerTop);
            lsWindow->setAnchors(LayerShellQt::Window::Anchors(
                LayerShellQt::Window::AnchorTop |
                LayerShellQt::Window::AnchorBottom |
                LayerShellQt::Window::AnchorLeft |
                LayerShellQt::Window::AnchorRight
                ));

            lsWindow->setKeyboardInteractivity(LayerShellQt::Window::KeyboardInteractivityOnDemand);

            // CHANGE 2: Remove margins (let QML handle positioning)
            lsWindow->setMargins({0, 0, 0, 0});

            lsWindow->setExclusiveZone(-1); // -1 means "Don't shift other windows"
        } else {
            qWarning() << "WARNING: Could not get LayerShellQt window. Are you running on Wayland?";
        }
        window->show();
    } else {
        qCritical() << "ERROR: Root object is not a Window!";
    }

    return app.exec();
}
