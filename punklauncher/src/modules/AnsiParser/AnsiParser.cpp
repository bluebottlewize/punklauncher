#include "AnsiParser.hpp"

void AnsiParser::parse(const QString &input) {
    m_cursorX = 0;
    m_cursorY = 0;
    m_currentColor = "#ffffff";

    // Wipe grid
    for (int y = 0; y < m_rows; ++y)
        for (int x = 0; x < m_cols; ++x)
            m_grid[y][x] = Cell(' ', "#ffffff");

    static QRegularExpression escRegex("\033\\[([0-9;]*)([a-zA-Z])");
    int lastPos = 0;
    auto it = escRegex.globalMatch(input);

    while (it.hasNext()) {
        auto match = it.next();
        processPlainText(input.mid(lastPos, match.capturedStart() - lastPos));
        handleEscapeSequence(match.captured(0));
        lastPos = match.capturedEnd();
    }

    if (lastPos < input.length()) {
        processPlainText(input.mid(lastPos));
    }

    emit textChanged();
}

void AnsiParser::processPlainText(const QString &text) {
    for (QChar c : text) {
        if (c == '\n') {
            m_cursorY++;
            m_cursorX = 0;
        } else if (c == '\r') {
            m_cursorX = 0;
        } else if (m_cursorY < m_rows && m_cursorX < m_cols) {
            m_grid[m_cursorY][m_cursorX] = Cell(c, m_currentColor);
            m_cursorX++;
        }
    }
}

void AnsiParser::handleEscapeSequence(const QString &seq) {
    static QRegularExpression re("\033\\[(\\d*)(;?\\d*)*([A-Za-z])");
    auto match = re.match(seq);

    // FIX: If captured(1) is empty, val1 depends on the type.
    // For 'm' it should be 0 (reset). For 'A', 'B', etc., it should be 1.
    QString type = match.captured(3);
    QString param1 = match.captured(1);
    QString param2 = match.captured(2).startsWith(";") ? match.captured(2).mid(1) : "";

    int val1 = param1.isEmpty() ? (type == "m" ? 0 : 1) : param1.toInt();
    int val2 = param2.isEmpty() ? -1 : param2.toInt();

    if (type == "A") m_cursorY = qMax(0, m_cursorY - val1);
    else if (type == "B") m_cursorY = qMin(m_rows - 1, m_cursorY + val1);
    else if (type == "C") m_cursorX = qMin(m_cols - 1, m_cursorX + val1);
    else if (type == "D") m_cursorX = qMax(0, m_cursorX - val1);
    else if (type == "G") m_cursorX = qBound(0, val1 - 1, m_cols - 1);
    else if (type == "H" || type == "f") {
        // Handle \033[H (0,0) or \033[row;colH
        m_cursorY = param1.isEmpty() ? 0 : qBound(0, val1 - 1, m_rows - 1);
        m_cursorX = param2.isEmpty() ? 0 : qBound(0, val2 - 1, m_cols - 1);
    }
    else if (type == "m") {
        // If it's a multi-parameter code like [1;34m
        if (val2 != -1) handleColorCode(val2);
        else handleColorCode(val1);
    }
}

void AnsiParser::handleColorCode(int code) {
    // 0 or empty is Reset
    if (code == 0) {
        m_currentColor = "#ffffff";
        return;
    }

    // fastfetch and distros often use 90-97 for bright variants
    switch (code) {
    case 30: case 90: m_currentColor = "#282a36"; break; // Black
    case 31: case 91: m_currentColor = "#ff5555"; break; // Red
    case 32: case 92: m_currentColor = "#50fa7b"; break; // Green
    case 33: case 93: m_currentColor = "#f1fa8c"; break; // Yellow
    case 34: case 94: m_currentColor = "#8341FF"; break; // Purple/Blue
    case 35: case 95: m_currentColor = "#ff79c6"; break; // Magenta
    case 36: case 96: m_currentColor = "#8be9fd"; break; // Cyan (Fixed from your #550000)
    case 37: case 97: m_currentColor = "#f8f8f2"; break; // White
    default: break;
    }
}

QString AnsiParser::formattedText() const {
    QString html = "<html><body style='white-space: pre;'>";

    for (int y = 0; y < m_rows; ++y) {
        QString lineHtml = "";
        QString currentLineColor = "";
        bool rowHasData = false;

        for (int x = 0; x < m_cols; ++x) {
            const Cell &cell = m_grid[y][x];
            if (cell.ch != ' ') rowHasData = true;

            if (cell.color != currentLineColor) {
                if (!currentLineColor.isEmpty()) lineHtml += "</span>";
                lineHtml += QString("<span style='color:%1;'>").arg(cell.color);
                currentLineColor = cell.color;
            }

            if (cell.ch == '<') lineHtml += "&lt;";
            else if (cell.ch == '>') lineHtml += "&gt;";
            else if (cell.ch == '&') lineHtml += "&amp;";
            else lineHtml += cell.ch;
        }
        lineHtml += "</span>";

        // Only add the row to the final output if it has characters
        if (rowHasData) {
            html += lineHtml + "\n";
        }
    }
    html += "</body></html>";
    return html;
}
