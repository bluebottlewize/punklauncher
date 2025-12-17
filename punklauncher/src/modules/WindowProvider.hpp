#ifndef WINDOWPROVIDER_H
#define WINDOWPROVIDER_H

#include <QObject>

class WindowProvider : public QObject {
    Q_OBJECT
public:
    explicit WindowProvider(QObject *parent = nullptr) : QObject(parent) {
    }
    QVariantList loadWindows();

    struct NiriWindow {
        QString id;
        QString title;
        QString appID;
        QString workspace;
    };

private:
    QList<NiriWindow> getNiriWindows();
};

#endif // WINDOWPROVIDER_H
