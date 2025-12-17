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

#include "ListProvider.hpp"
#include "AliasProvider.hpp"
#include "WindowProvider.hpp"
#include "AppProvider.hpp"

QVariantList ListProvider::search(const QString &query)
{
    QString q = query.toLower();

    QVariantList results = all;
    ranker.rank(results, query);

    return results;
}

void ListProvider::updateRank(const QString &name, const QString &query)
{
    ranker.registerExecution(name, query);
}

void ListProvider::scan()
{
    AliasProvider aliasProvider;
    aliases = aliasProvider.loadAliases();

    WindowProvider windowProvider;
    windows = windowProvider.loadWindows();

    AppProvider appProvider;
    apps = appProvider.loadApps();

    all.append(aliases);
    all.append(windows);
    all.append(apps);
}
