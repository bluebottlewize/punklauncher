#ifndef ANSIPARSER_HPP
#define ANSIPARSER_HPP

#include <QObject>
#include <QString>
#include <QVector>
#include <QRegularExpression>

struct Cell {
    char ch = ' ';
    QString color = "#ffffff";
};

class AnsiParser : public QObject {
    Q_OBJECT
    Q_PROPERTY(QString formattedText READ formattedText NOTIFY textChanged)

    struct Cell {
        QChar ch = ' ';
        QString color = "#ffffff";

        // Adding this constructor makes the {c, color} syntax work perfectly
        Cell(QChar character = ' ', QString clr = "#ffffff")
            : ch(character), color(clr) {}
    };

public:
    explicit AnsiParser(int rows = 25, int cols = 100, QObject *parent = nullptr)
        : QObject(parent), m_rows(rows), m_cols(cols), m_cursorX(0), m_cursorY(0) {
        m_grid.resize(m_rows, QVector<Cell>(m_cols));
    }

    Q_INVOKABLE void parse(const QString &input);
    QString formattedText() const;

signals:
    void textChanged();

private:
    int m_rows, m_cols;
    int m_cursorX, m_cursorY;
    QString m_currentColor = "#ffffff";
    QVector<QVector<Cell>> m_grid;

    void handleEscapeSequence(const QString &seq);
    void handleColorCode(int code);
    void processPlainText(const QString &text);
};

#endif
