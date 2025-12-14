#ifndef ICONPROVIDER_H
#define ICONPROVIDER_H

#include <QQuickImageProvider>
#include <QIcon>
#include <QPixmap>
#include <QFile>
#include <QDir>

#include <iostream>

using namespace std;

class IconProvider : public QQuickImageProvider {
public:
    IconProvider() : QQuickImageProvider(QQuickImageProvider::Pixmap) {}

    QPixmap requestPixmap(const QString &id, QSize *size, const QSize &requestedSize) override {

        cout << id.toStdString() << endl;
        QString customPath = QDir::homePath() + "/.local/share/icons/purplepunk/apps/" + id + ".png";

        int width = requestedSize.width() > 0 ? requestedSize.width() : 64;
        int height = requestedSize.height() > 0 ? requestedSize.height() : 64;
        if (size) *size = QSize(width, height);

        if (QFile::exists(customPath)) {
            cout << customPath.toStdString() << endl;

            QPixmap customPix(customPath);

            if (!customPix.isNull()) {
                // Success!
                return customPix.scaled(width, height, Qt::KeepAspectRatio, Qt::SmoothTransformation);
            } else {
                // File exists but failed to load (Corrupt? Permission?)
                qWarning() << "CRITICAL: Found file but failed to load image:" << customPath;
            }

            return QPixmap(customPath).scaled(size->width(), size->width(), Qt::KeepAspectRatio, Qt::SmoothTransformation);
        }

        // 1. Get the icon from the system theme
        QIcon icon = QIcon::fromTheme(id);

        // If system theme fails, try to load it as a direct file path (fallback)
        if (icon.isNull() && !id.startsWith("/")) {
            icon = QIcon(id);
        }

        // 2. Determine size (Default to 64x64 if QML doesn't specify)
        // int width = requestedSize.width() > 0 ? requestedSize.width() : 64;
        // int height = requestedSize.height() > 0 ? requestedSize.height() : 64;

        if (size) *size = QSize(width, height);

        // 3. Return the pixmap
        // Try to get the actual available size first to avoid blurry scaling
        return icon.pixmap(width, height);
    }
};

#endif // ICONPROVIDER_H
