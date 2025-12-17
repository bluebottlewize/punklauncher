#include "Backend.hpp"
#include <QDebug>

static void runCommandDetached(const QString &cmd)
{
    if (cmd.isEmpty()) return;

    QString cleanCmd = cmd;
    cleanCmd.remove(QRegularExpression("%[A-Za-z]"));

    qDebug() << "Launching:" << cleanCmd;

    QProcess::startDetached("/bin/sh", QStringList() << "-c" << cleanCmd);
}

QVariantList Backend::search(const QString &query)
{
    qDebug() << query.toStdString();
    return listProvider->search(query);
}

void Backend::execute(const QString &name, const QString &cmd, const QString &queryText, const QString &type)
{
    listProvider->updateRank(name, queryText);

    if (type == "alias")
    {
        runCommandDetached(cmd);
    }
    else if (type == "window")
    {
        runCommandDetached(cmd);
    }
    else if (type == "app")
    {
        runCommandDetached(cmd);
    }

    return;
}

void Backend::close() {
    QCoreApplication::quit();
}
