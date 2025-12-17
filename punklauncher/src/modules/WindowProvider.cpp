#include "WindowProvider.hpp"

#include <QRegularExpression>
#include <QProcess>
#include <QVariantList>

QVariantList WindowProvider::loadWindows()
{
    QVariantList windows;
    QList<WindowProvider::NiriWindow> openWindows = getNiriWindows();

    for (const auto &win : openWindows)
    {
        QVariantMap map;
        map["name"] = win.appID + " - " + win.title;
        map["exec"] = "niri msg action focus-window --id " + win.id;
        map["icon"] = win.appID.toLower();
        map["type"] = "window";

        windows.append(map);
    }

    return windows;
}

QList<WindowProvider::NiriWindow> WindowProvider::getNiriWindows()
{
    QList<WindowProvider::NiriWindow> windows;

    QProcess p;
    p.start("niri", QStringList() << "msg" << "windows");
    p.waitForFinished();
    QString output = p.readAllStandardOutput();

    QTextStream stream(&output);
    QString line;
    WindowProvider::NiriWindow current;
    bool parsingWindow = false;

    while (stream.readLineInto(&line)) {
        line = line.trimmed();

        if (line.startsWith("Window ID")) {
            // Save previous if valid
            if (parsingWindow && !current.id.isEmpty()) {
                windows.append(current);
            }

            // Reset for new window
            current = WindowProvider::NiriWindow();
            parsingWindow = true;

            QRegularExpression re("Window ID (\\d+):");
            auto match = re.match(line);
            if (match.hasMatch()) current.id = match.captured(1);
        }

        else if (line.startsWith("Title:")) {
            int start = line.indexOf('"');
            int end = line.lastIndexOf('"');
            if (start != -1 && end > start) {
                current.title = line.mid(start + 1, end - start - 1);
            }
        }

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
