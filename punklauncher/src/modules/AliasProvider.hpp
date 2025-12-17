#ifndef ALIASPROVIDER_H
#define ALIASPROVIDER_H

#include <QObject>

class AliasProvider : public QObject {
    Q_OBJECT
public:
    explicit AliasProvider(QObject *parent = nullptr) : QObject(parent) {
    }
    QVariantList loadAliases();

private:
    QJsonArray loadConfig();
};

#endif // ALIASPROVIDER_H
