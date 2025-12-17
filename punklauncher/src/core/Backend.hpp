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

class Backend : public QObject {
    Q_OBJECT
public:
    explicit Backend(QObject *parent = nullptr) : QObject(parent)
    {
        listProvider = new ListProvider(this);
    }

    Q_INVOKABLE QVariantList search(const QString &query);
    Q_INVOKABLE void execute(const QString &name, const QString &cmd, const QString &queryText, const QString &type);
    Q_INVOKABLE void close();


private:
    ListProvider *listProvider;
};

#endif // BACKEND_HPP
