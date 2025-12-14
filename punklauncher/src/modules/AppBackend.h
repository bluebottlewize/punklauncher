#ifndef APPBACKEND_H
#define APPBACKEND_H

#include <QObject>
#include <QVariant>
#include <QDir>
#include <QStandardPaths>
#include <QSettings>
#include <QProcess>
#include <QCoreApplication>
#include <QRegularExpression>
#include <QTextStream>

#include "UsageRanker.h"

// Helper class to handle App Logic
class AppBackend : public QObject {
    Q_OBJECT
public:
    explicit AppBackend(QObject *parent = nullptr) : QObject(parent) {
        // Cache apps on startup to make search fast
        scanApps();
        qDebug() << "Launching:" << m_allApps.size();
    }

    Q_INVOKABLE QVariantList search(const QString &query) {
        QVariantList results;
        QString q = query.toLower();

        // 1. FETCH OPEN WINDOWS (Niri)
        // We do this first so they appear at the top
        QList<NiriWindow> openWindows = getNiriWindows();

        for (const auto &win : openWindows) {
            // Filter: Does title or AppID match query?
            if (q.isEmpty() ||
                win.title.toLower().contains(q) ||
                win.appID.toLower().contains(q)) {

                QVariantMap map;
                map["name"] = win.title; // Show "Spotify Free"

                // KEY TRICK: The command is simply to tell Niri to focus this ID
                // The launcher will run this via /bin/sh, which works perfectly.
                map["exec"] = "niri msg action focus-window --id " + win.id;

                // Use AppID for icon (lowercase is safer for icon themes)
                map["icon"] = win.appID.toLower();

                // Tag it so QML knows it's a running window (for styling)
                map["isWindow"] = true;

                results.append(map);
            }
        }

        // 2. FETCH STANDARD APPS (Existing logic)
        QVariantList apps;
        for (const QVariant &v : m_allApps) {
            QVariantMap map = v.toMap();
            if (q.isEmpty() || map["name"].toString().toLower().contains(q)) {
                map["isWindow"] = false; // Regular app
                apps.append(map);
            }
        }

        // 3. RANKING (Optional: You can decide if Windows always stay on top)
        // For now, we append apps after windows
        m_ranker.rank(apps, q);
        results.append(apps);

        return results;
    }

    // Q_INVOKABLE QVariantList search(const QString &query) {
    //     QVariantList filtered;
    //     QString q = query.toLower();

    //     qDebug() << "C++ Search Triggered with:" << query; // <--- Check "Application Output" tab

    //     // 1. Filter
    //     for (const QVariant &v : m_allApps) {
    //         QVariantMap map = v.toMap();
    //         if (q.isEmpty() || map["name"].toString().toLower().contains(q)) {
    //             filtered.append(map);
    //         }
    //     }

    //     // 2. Rank (Pass the query so Ranker knows context)
    //     m_ranker.rank(filtered, q);

    //     qDebug() << "Launching:" << filtered.size();


    //     return filtered;
    // }

    // QML calls this to launch an app
    Q_INVOKABLE void launch(const QString &cmd) {
        if (cmd.isEmpty()) return;

        // Clean up the command (Remove %u, %F, etc.)
        // We use a regex that matches % followed by a single letter,
        // optionally inside quotes, to prevent leaving dangling quotes.
        QString cleanCmd = cmd;
        cleanCmd.remove(QRegularExpression("%[A-Za-z]"));

        // DEBUG: See exactly what is being run
        qDebug() << "Launching:" << cleanCmd;

        // THE FIX:
        // Instead of splitting arguments manually, pass the WHOLE string to /bin/sh.
        // This handles quotes, environment variables (DISPLAY=...),
        // and chained commands (&&) automatically.
        QProcess::startDetached("/bin/sh", QStringList() << "-c" << cleanCmd);
    }

    // OVERLOAD: New Launch function that tracks name
    Q_INVOKABLE void launchApp(const QString &name, const QString &cmd, const QString &queryText) {
        m_ranker.registerLaunch(name, queryText);
        launch(cmd);
    }

    // QML calls this to close
    Q_INVOKABLE void close() {
        QCoreApplication::quit();
    }

private:
    UsageRanker m_ranker; // The modular algorithm
    QVariantList m_allApps; // Cache

    void scanApps() {
        m_allApps.clear();
        const QStringList dirs = QStandardPaths::standardLocations(QStandardPaths::ApplicationsLocation);

        for (const QString &dirPath : dirs) {
            QDir dir(dirPath);
            QStringList files = dir.entryList({"*.desktop"}, QDir::Files);
            for (const QString &file : files) {
                QSettings desktopFile(dir.filePath(file), QSettings::IniFormat);
                desktopFile.beginGroup("Desktop Entry");

                if (desktopFile.value("NoDisplay", false).toBool() ||
                    desktopFile.value("Type").toString() != "Application") continue;

                QString name = desktopFile.value("Name").toString();
                QString icon = desktopFile.value("Icon").toString();
                QString exec = desktopFile.value("Exec").toString();
                exec.remove(QRegularExpression(" %[a-zA-Z]"));

                if (!name.isEmpty() && !exec.isEmpty()) {
                    QVariantMap app;
                    app["name"] = name;
                    // If icon is a path, use it. If it's a theme name, QML handles it differently.
                    // For simplicity, we pass the raw string.
                    app["icon"] = icon;
                    app["exec"] = exec;
                    m_allApps.append(app);
                }
                desktopFile.endGroup();
            }
        }
    }

    struct NiriWindow {
        QString id;
        QString title;
        QString appID;
        QString workspace;
    };

    QList<NiriWindow> getNiriWindows() {
        QList<NiriWindow> windows;

        // 1. Run the command
        QProcess p;
        p.start("niri", QStringList() << "msg" << "windows");
        p.waitForFinished();
        QString output = p.readAllStandardOutput();

        // 2. Parse Line-by-Line
        QTextStream stream(&output);
        QString line;
        NiriWindow current;
        bool parsingWindow = false;

        while (stream.readLineInto(&line)) {
            line = line.trimmed();

            // A. Start of a new block: "Window ID 7:"
            if (line.startsWith("Window ID")) {
                // Save previous if valid
                if (parsingWindow && !current.id.isEmpty()) {
                    windows.append(current);
                }

                // Reset for new window
                current = NiriWindow();
                parsingWindow = true;

                // Extract ID: "Window ID 7:" -> "7"
                QRegularExpression re("Window ID (\\d+):");
                auto match = re.match(line);
                if (match.hasMatch()) current.id = match.captured(1);
            }
            // B. Extract Title: Title: "Spotify Free"
            else if (line.startsWith("Title:")) {
                int start = line.indexOf('"');
                int end = line.lastIndexOf('"');
                if (start != -1 && end > start) {
                    current.title = line.mid(start + 1, end - start - 1);
                }
            }
            // C. Extract App ID: App ID: "Spotify"
            else if (line.startsWith("App ID:")) {
                int start = line.indexOf('"');
                int end = line.lastIndexOf('"');
                if (start != -1 && end > start) {
                    current.appID = line.mid(start + 1, end - start - 1);
                }
            }
        }
        // Save the very last one
        if (parsingWindow && !current.id.isEmpty()) {
            windows.append(current);
        }

        return windows;
    }
};

#endif
