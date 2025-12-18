#ifndef BACKEND_HPP
#define BACKEND_HPP

#include <QObject>
#include <QVariant>
#include <QDir>
#include <QStandardPaths>
#include <QSettings>
#include <QProcess>
#include <QCoreApplication>
#include <QRegularExpression>
#include <QTextStream>

#include "src/modules/ListProvider.hpp"

class Backend : public QObject
{

    Q_OBJECT
    Q_PROPERTY(QString lastOutput READ lastOutput NOTIFY lastOutputChanged)

public:
    explicit Backend(QObject *parent = nullptr) : QObject(parent)
    {
        listProvider = new ListProvider(this);

        connect(&m_managedProcess, &QProcess::readyReadStandardOutput, this, &Backend::handleReadyRead);
        connect(&m_managedProcess, &QProcess::readyReadStandardError, this, &Backend::handleReadyRead);
    }

    Q_INVOKABLE QVariantList search(const QString &query);
    Q_INVOKABLE void execute(const QString &name, const QString &cmd, const QString &queryText, const QString &type);
    Q_INVOKABLE void close();

    QString lastOutput() const { return m_lastOutput; }

signals:
    void lastOutputChanged();

private slots:
    void handleReadyRead();

private:
    QString m_lastOutput;
    QProcess m_managedProcess;

    ListProvider *listProvider;
};

#endif // BACKEND_HPP
