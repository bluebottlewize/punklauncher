#ifndef APPPROVIDER_H
#define APPPROVIDER_H

#include <QObject>

class AppProvider : public QObject {
    Q_OBJECT
public:
    explicit AppProvider(QObject *parent = nullptr) : QObject(parent) {
    }
    QVariantList loadApps();

private:
    QJsonArray loadConfig();
};

#endif // APPPROVIDER_H
