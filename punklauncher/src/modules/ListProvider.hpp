#ifndef LISTPROVIDER_HPP
#define LISTPROVIDER_HPP

#include <QObject>
#include <QVariantList>

#include "Ranker.hpp"

class ListProvider : public QObject {
    Q_OBJECT
public:
    explicit ListProvider(QObject *parent = nullptr) : QObject(parent) {
        scan();
    };
    Q_INVOKABLE QVariantList search(const QString &query);
    void updateRank(const QString &name, const QString &query);

private:
    QVariantList aliases;
    QVariantList apps;
    QVariantList windows;
    QVariantList all;

    Ranker ranker;

    void scan();
};

#endif // LISTPROVIDER_HPP
