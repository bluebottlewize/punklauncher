#include "AppProvider.hpp"

#include <QStandardPaths>
#include <QSettings>
#include <QDir>
#include <QRegularExpression>

QVariantList AppProvider::loadApps()
{
    QVariantList apps;
    const QStringList dirs = QStandardPaths::standardLocations(QStandardPaths::ApplicationsLocation);

    for (const QString &dirPath : dirs) {
        QDir dir(dirPath);
        QStringList files = dir.entryList({"*.desktop"}, QDir::Files);
        for (const QString &file : files)
        {
            QSettings desktopFile(dir.filePath(file), QSettings::IniFormat);
            desktopFile.beginGroup("Desktop Entry");

            if (desktopFile.value("NoDisplay", false).toBool() ||
                desktopFile.value("Type").toString() != "Application") continue;

            QString name = desktopFile.value("Name").toString();
            QString icon = desktopFile.value("Icon").toString();
            QString exec = desktopFile.value("Exec").toString();
            exec.remove(QRegularExpression(" %[a-zA-Z]"));

            if (!name.isEmpty() && !exec.isEmpty())
            {
                QVariantMap app;
                app["name"] = name;

                // If icon is a path, use it. If it's a theme name, QML handles it differently.
                // For simplicity, we pass the raw string.
                app["icon"] = icon;

                app["exec"] = exec;
                app["type"] = "app";
                apps.append(app);
            }
            desktopFile.endGroup();
        }
    }

    return apps;
}
