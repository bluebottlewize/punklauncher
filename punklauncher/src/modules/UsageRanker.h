#ifndef USAGERANKER_H
#define USAGERANKER_H

#pragma once
#include <QObject>
#include <QMap>
#include <QFile>
#include <QJsonDocument>
#include <QJsonObject>
#include <QStandardPaths>
#include <QDir>
#include <QVariant>

class UsageRanker : public QObject {
    Q_OBJECT
public:
    UsageRanker(QObject *parent = nullptr) : QObject(parent) {
        loadUsage();
    }

    // --- UPDATED ALGORITHM ---
    // Now accepts the 'currentQuery' to bias results
    void rank(QVariantList &apps, const QString &currentQuery) {
        QString query = currentQuery.toLower().trimmed();

        std::sort(apps.begin(), apps.end(), [this, query](const QVariant &a, const QVariant &b) {
            QString nameA = a.toMap()["name"].toString();
            QString nameB = b.toMap()["name"].toString();

            // 1. Context Score: How often was this app picked for THIS specific query?
            // We give this a huge multiplier (e.g., x10) because it's user intent.
            int contextScoreA = m_queryScores.value(query).value(nameA, 0) * 10;
            int contextScoreB = m_queryScores.value(query).value(nameB, 0) * 10;

            // 2. Global Score: How often is this app used in general?
            int globalScoreA = m_globalScores.value(nameA, 0);
            int globalScoreB = m_globalScores.value(nameB, 0);

            int totalA = contextScoreA + globalScoreA;
            int totalB = contextScoreB + globalScoreB;

            if (totalA != totalB) return totalA > totalB; // Higher score wins

            // 3. Tie-breaker: Alphabetical
            return nameA.compare(nameB, Qt::CaseInsensitive) < 0;
        });
    }

    // --- UPDATED TRACKING ---
    // Now records WHAT you typed when you launched it
    void registerLaunch(const QString &appName, const QString &queryUsed) {
        // 1. Increment Global Score
        m_globalScores[appName]++;

        // 2. Increment Context Score (only if user actually typed something)
        QString query = queryUsed.toLower().trimmed();
        if (!query.isEmpty()) {
            m_queryScores[query][appName]++;
        }

        saveUsage();
    }

private:
    QMap<QString, int> m_globalScores;

    // Map< QueryString, Map< AppName, Count > >
    // Example: "fi" -> { "Firefox": 5, "File Manager": 1 }
    QMap<QString, QMap<QString, int>> m_queryScores;

    QString getFilePath() {
        QString path = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
        QDir().mkpath(path);
        return path + "/usage_v2.json";
    }

    void saveUsage() {
        QJsonObject root;

        // Save Global
        QJsonObject globalJson;
        for (auto it = m_globalScores.begin(); it != m_globalScores.end(); ++it)
            globalJson[it.key()] = it.value();
        root["global"] = globalJson;

        // Save Context queries
        QJsonObject queryJson;
        for (auto it = m_queryScores.begin(); it != m_queryScores.end(); ++it) {
            QJsonObject innerMap;
            auto apps = it.value();
            for (auto appIt = apps.begin(); appIt != apps.end(); ++appIt) {
                innerMap[appIt.key()] = appIt.value();
            }
            queryJson[it.key()] = innerMap;
        }
        root["queries"] = queryJson;

        QFile file(getFilePath());
        if (file.open(QIODevice::WriteOnly)) {
            file.write(QJsonDocument(root).toJson());
        }
    }

    void loadUsage() {
        QFile file(getFilePath());
        if (file.open(QIODevice::ReadOnly)) {
            QJsonObject root = QJsonDocument::fromJson(file.readAll()).object();

            // Load Global
            QJsonObject globalJson = root["global"].toObject();
            for (auto it = globalJson.begin(); it != globalJson.end(); ++it)
                m_globalScores[it.key()] = it.value().toInt();

            // Load Context
            QJsonObject queryJson = root["queries"].toObject();
            for (auto it = queryJson.begin(); it != queryJson.end(); ++it) {
                QJsonObject innerMap = it.value().toObject();
                for (auto appIt = innerMap.begin(); appIt != innerMap.end(); ++appIt) {
                    m_queryScores[it.key()][appIt.key()] = appIt.value().toInt();
                }
            }
        }
    }
};

#endif // USAGERANKER_H
