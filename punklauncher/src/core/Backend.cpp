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

void Backend::handleReadyRead() {
    // Append new data to the string and notify QML
    m_lastOutput += QString::fromUtf8(m_managedProcess.readAllStandardOutput());
    m_lastOutput += QString::fromUtf8(m_managedProcess.readAllStandardError());
    emit lastOutputChanged();
}

void Backend::execute(const QString &name, const QString &cmd, const QString &queryText, const QString &type)
{
    // listProvider->updateRank(name, queryText);

    if (type == "alias")
    {
        // --- STREAMING LOGIC FOR ALIASES ---
        if (m_managedProcess.state() != QProcess::NotRunning) {
            m_managedProcess.kill();
        }

        m_lastOutput = ""; // Clear previous output
        emit lastOutputChanged();

        QString cleanCmd = cmd;
        cleanCmd.remove(QRegularExpression("%[A-Za-z]"));

        qDebug() << cmd;

        m_managedProcess.start("/bin/sh", QStringList() << "-c" << cleanCmd);
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
