#include <QQmlApplicationEngine>
#include <QQmlContext>
#include <QProcess>
#include <QStandardPaths>
#include <QDir>
#include <QFile>
#include <QJsonDocument>
#include <QJsonArray>
#include <QJsonObject>
#include <QDebug>
#include <QCoreApplication>

#include "AliasProvider.hpp"

static QJsonArray loadJsonArray(const QString &path)
{
    QFile file(path);
    if (!file.open(QIODevice::ReadOnly))
        return {};

    QJsonDocument doc = QJsonDocument::fromJson(file.readAll());
    return doc.isArray() ? doc.array() : QJsonArray{};
}

QJsonArray AliasProvider::loadConfig()
{
    QJsonArray defaultConfig = loadJsonArray(":/assets/config.json");

    QString userConfigPath = QStandardPaths::writableLocation(QStandardPaths::ConfigLocation) + "/punklauncher/config.json";

    QJsonArray userConfig;

    if (QFile::exists(userConfigPath)) {
        qDebug() << "Loading user config:" << userConfigPath;
        userConfig = loadJsonArray(userConfigPath);
        return userConfig;
    }
    else {
        qDebug() << "User config missing, using defaults only.";
        return defaultConfig;
    }
}

QVariantList AliasProvider::loadAliases()
{
    QVariantList aliases;

    QJsonArray config = loadConfig();
    const int count = config.size();

    qDebug() << count;

    for (int i = 0; i < count; ++i)
    {
        QJsonObject aliasObject = config[i].toObject();
        aliasObject["type"] = "alias";
        aliases.append(aliasObject);
    }

    return aliases;
}
