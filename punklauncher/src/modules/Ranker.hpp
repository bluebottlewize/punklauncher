#ifndef RANKER_HPP
#define RANKER_HPP

#include <QObject>
#include <QVariantList>
#include <QMap>
#include <QFile>
#include <QJsonDocument>
#include <QJsonObject>
#include <QStandardPaths>
#include <QDir>
#include <QtMath>

class Ranker : public QObject {
    Q_OBJECT
public:
    explicit Ranker(QObject *parent = nullptr)
        : QObject(parent)
    {
        load();
    }

    void rank(QVariantList &items, const QString &query);
    void registerExecution(const QString &itemName, const QString &queryUsed);

private:
    QMap<QString, int> m_globalUsage;
    QMap<QString, QMap<QString, int>> m_prefixUsage;

    double textScore(const QString &name, const QString &query) const;
    double clamp(double v, double lo, double hi) const;
    double typeBoost(const QString &type) const;
    QString acronym(const QString &name) const;
    bool fastMatch(const QString &name, const QString &query) const;

    QString dataPath() const;
    void save();
    void load();
};

#endif // RANKER_HPP
