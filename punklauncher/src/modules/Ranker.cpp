#include "Ranker.hpp"

#include <algorithm>
#include <QRegularExpression>

void Ranker::rank(QVariantList &items, const QString &rawQuery)
{
    QString query = rawQuery.toLower().trimmed();

    items.erase(
        std::remove_if(items.begin(), items.end(),
                       [this, query](const QVariant &v) {
                           QString name = v.toMap()["name"].toString().toLower();
                           return !fastMatch(name, query);
                       }),
        items.end()
        );

    std::sort(items.begin(), items.end(),
              [this, query](const QVariant &a, const QVariant &b) {

                  auto mapA = a.toMap();
                  auto mapB = b.toMap();

                  QString nameA = mapA["name"].toString().toLower();
                  QString nameB = mapB["name"].toString().toLower();

                  QString typeA = mapA["type"].toString().toLower();
                  QString typeB = mapB["type"].toString().toLower();


                  // 1. Text relevance (gate)
                  double textA = textScore(nameA, query);
                  double textB = textScore(nameB, query);

                  if (textA == 0 && textB == 0)
                      return nameA < nameB;
                  if (textA == 0) return false;
                  if (textB == 0) return true;

                  // 2. Prefix intent
                  int prefixLen = qMin(query.length(), 4);
                  QString prefix = query.left(prefixLen);

                  double intentA = qLn(m_prefixUsage[prefix].value(nameA, 0) + 1.0);
                  double intentB = qLn(m_prefixUsage[prefix].value(nameB, 0) + 1.0);

                  // 3. Global habit
                  double globalA = qLn(m_globalUsage.value(nameA, 0) + 1.0);
                  double globalB = qLn(m_globalUsage.value(nameB, 0) + 1.0);

                  // 4. Query-length adaptation
                  double intentWeight = clamp(query.length() / 4.0, 0.0, 1.0);
                  double habitWeight  = 1.0 - intentWeight;

                  // 5. Final score (multiplicative)
                  double scoreA =
                      textA *
                      (1.0 + intentWeight * intentA) *
                      (1.0 + habitWeight  * globalA) *
                      typeBoost(typeA);

                  double scoreB =
                      textB *
                      (1.0 + intentWeight * intentB) *
                      (1.0 + habitWeight  * globalB) *
                      typeBoost(typeB);

                  if (!qFuzzyCompare(scoreA, scoreB))
                      return scoreA > scoreB;

                  // tie-breaker
                  return nameA < nameB;
              });
}

void Ranker::registerExecution(const QString &itemName, const QString &rawQuery)
{
    QString name  = itemName.toLower();
    QString query = rawQuery.toLower().trimmed();

    // global habit
    m_globalUsage[name]++;

    // prefix intent
    if (!query.isEmpty()) {
        QString prefix = query.left(4);
        m_prefixUsage[prefix][name]++;
    }

    save();
}

double Ranker::textScore(const QString &name, const QString &query) const
{
    if (query.isEmpty())
        return 1.0;

    // 1. Exact prefix (strongest)
    if (name.startsWith(query))
        return 120.0 + query.length() * 12.0;

    // 2. Acronym match (VERY IMPORTANT)
    QString acro = acronym(name);
    if (acro.startsWith(query))
        return 110.0 + query.length() * 15.0;

    // 3. Word-boundary prefix
    for (const QString &part : name.split(QRegularExpression("[\\s._-]"),
                                          Qt::SkipEmptyParts)) {
        if (part.startsWith(query))
            return 90.0 + query.length() * 10.0;
    }

    // 4. Subsequence fallback
    int qi = 0;
    int streak = 0;
    double score = 0.0;

    for (int ni = 0; ni < name.length() && qi < query.length(); ++ni) {
        if (name[ni] == query[qi]) {
            qi++;
            streak++;
            score += 6.0 + streak * 2.5;
        } else {
            streak = 0;
        }
    }

    return (qi == query.length()) ? score : 0.0;
}


double Ranker::clamp(double v, double lo, double hi) const
{
    return qMax(lo, qMin(v, hi));
}

double Ranker::typeBoost(const QString &type) const
{
    if (type == "window")  return 1.25;
    if (type == "app")     return 1.15;
    if (type == "alias")   return 1.05;
    if (type == "command") return 1.00;

    return 1.0;
}

QString Ranker::acronym(const QString &name) const
{
    QString result;
    bool take = true;

    for (int i = 0; i < name.size(); ++i) {
        const QChar c = name[i];

        if (!c.isLetterOrNumber()) {
            take = true;
            continue;
        }

        if (take) {
            result.append(c);
            take = false;
        }

        // camelCase boundary: fireFox → fF
        if (i + 1 < name.size() && name[i + 1].isUpper()) {
            take = true;
        }
    }

    return result.toLower();
}

bool Ranker::fastMatch(const QString &name, const QString &query) const
{
    if (query.isEmpty())
        return true;

    // 1. Direct substring (very cheap)
    if (name.contains(query))
        return true;

    // 2. Acronym subsequence
    QString acro = acronym(name);
    int qi = 0;
    for (QChar c : acro) {
        if (qi < query.size() && c == query[qi])
            qi++;
    }
    if (qi == query.size())
        return true;

    // 3. General subsequence (cheap, no scoring)
    qi = 0;
    for (QChar c : name) {
        if (qi < query.size() && c == query[qi])
            qi++;
    }
    return qi == query.size();
}


QString Ranker::dataPath() const
{
    QString dir = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
    QDir().mkpath(dir);
    return dir + "/ranker_v1.json";
}

void Ranker::save()
{
    QJsonObject root;

    QJsonObject global;
    for (auto it = m_globalUsage.begin(); it != m_globalUsage.end(); ++it)
        global[it.key()] = it.value();
    root["global"] = global;

    QJsonObject prefixes;
    for (auto pit = m_prefixUsage.begin(); pit != m_prefixUsage.end(); ++pit) {
        QJsonObject inner;
        for (auto iit = pit.value().begin(); iit != pit.value().end(); ++iit)
            inner[iit.key()] = iit.value();
        prefixes[pit.key()] = inner;
    }
    root["prefixes"] = prefixes;

    QFile f(dataPath());
    if (f.open(QIODevice::WriteOnly))
        f.write(QJsonDocument(root).toJson());
}

// void Ranker::load()
// {
//     qDebug() << "hello";

//     qDebug() << "hello";


//     QFile f(dataPath());
//     if (!f.open(QIODevice::ReadOnly))
//         return;

//     QJsonObject root = QJsonDocument::fromJson(f.readAll()).object();

//     qDebug() << "hello";

//     for (auto it = root["global"].toObject().begin();
//          it != root["global"].toObject().end(); ++it)
//     {    qDebug() << "hello1";

//         m_globalUsage[it.key()] = it.value().toInt();
//         qDebug() << "hello2";

//     }

//     qDebug() << "hello";

//     for (auto pit = root["prefixes"].toObject().begin();
//          pit != root["prefixes"].toObject().end(); ++pit) {
//         QJsonObject inner = pit.value().toObject();
//         for (auto iit = inner.begin(); iit != inner.end(); ++iit)
//             m_prefixUsage[pit.key()][iit.key()] = iit.value().toInt();
//     }

//     qDebug() << "hello";

//     qDebug() << "hello";

//     qDebug() << "hello";

// }

void Ranker::load()
{
    QFile f(dataPath());
    if (!f.open(QIODevice::ReadOnly))
        return;

    QJsonDocument doc = QJsonDocument::fromJson(f.readAll());
    QJsonObject root = doc.object();

    // Fix: Capture the object so it doesn't get destroyed during the loop
    QJsonObject globalObj = root["global"].toObject();

    // Now .begin() and .end() refer to the SAME instance (globalObj)
    for (auto it = globalObj.begin(); it != globalObj.end(); ++it) {
        m_globalUsage[it.key()] = it.value().toInt();
    }

    QJsonObject prefixesObj = root["prefixes"].toObject();
    for (auto pit = prefixesObj.begin(); pit != prefixesObj.end(); ++pit) {
        // Fix: Capture the inner object as well
        QJsonObject innerObj = pit.value().toObject();
        auto &internalMap = m_prefixUsage[pit.key()];

        for (auto iit = innerObj.begin(); iit != innerObj.end(); ++iit) {
            internalMap[iit.key()] = iit.value().toInt();
        }
    }
}
