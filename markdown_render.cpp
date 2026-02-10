#include "markdown_render.h"
#include <QStringList>
#include <QRegularExpression>

// =====================================================
// Lightweight Markdown → HTML for ChatGPT responses
//
// Processing order matters:
//   1. Fenced code blocks (``` ... ```) — extracted first
//      to protect their content from inline processing
//   2. Block-level: headings, lists, paragraphs
//   3. Inline: bold, italic, inline code
// =====================================================

static QString processInline(const QString &line)
{
    QString out = line;

    // Inline code: `code` → <code>code</code>
    // (must come before bold/italic to avoid conflicts)
    static QRegularExpression rxCode("`([^`]+)`");
    out.replace(rxCode, R"(<code style="background:#2a2a2a; padding:1px 4px; border-radius:3px; font-family:monospace;">\1</code>)");

    // Bold: **text** → <b>text</b>
    static QRegularExpression rxBold(R"(\*\*(.+?)\*\*)");
    out.replace(rxBold, "<b>\\1</b>");

    // Italic: *text* → <i>text</i>  (but not inside **)
    static QRegularExpression rxItalic(R"((?<!\*)\*([^*]+)\*(?!\*))");
    out.replace(rxItalic, "<i>\\1</i>");

    return out;
}

QString MarkdownRender::toHtml(const QString &markdown)
{
    if (markdown.isEmpty())
        return QString();

    QString result;
    QStringList lines = markdown.split('\n');

    bool inCodeBlock = false;
    QString codeBlockContent;
    QString codeBlockLang;

    bool inUl = false;   // unordered list active
    bool inOl = false;   // ordered list active

    for (int i = 0; i < lines.size(); ++i) {
        const QString &rawLine = lines[i];

        // ==============================================
        // Fenced code blocks: ```lang ... ```
        // ==============================================
        if (rawLine.trimmed().startsWith("```")) {
            if (!inCodeBlock) {
                // Opening fence
                inCodeBlock = true;
                codeBlockContent.clear();
                codeBlockLang = rawLine.trimmed().mid(3).trimmed();
            } else {
                // Closing fence — emit the block
                inCodeBlock = false;
                QString escaped = codeBlockContent.toHtmlEscaped();
                result += QStringLiteral(
                    "<pre style=\"background:#1a1a1a; color:#d4d4d4; "
                    "padding:10px; border-radius:6px; "
                    "font-family:monospace; white-space:pre-wrap; "
                    "margin:8px 0; overflow-x:auto;\">"
                    "<code>%1</code></pre>").arg(escaped);
            }
            continue;
        }

        if (inCodeBlock) {
            if (!codeBlockContent.isEmpty())
                codeBlockContent += '\n';
            codeBlockContent += rawLine;
            continue;
        }

        QString line = rawLine;

        // ==============================================
        // Headings: ### text
        // ==============================================
        if (line.startsWith("### ")) {
            if (inUl) { result += "</ul>"; inUl = false; }
            if (inOl) { result += "</ol>"; inOl = false; }
            result += "<h4 style=\"margin:10px 0 4px 0;\">" + processInline(line.mid(4)) + "</h4>";
            continue;
        }
        if (line.startsWith("## ")) {
            if (inUl) { result += "</ul>"; inUl = false; }
            if (inOl) { result += "</ol>"; inOl = false; }
            result += "<h3 style=\"margin:12px 0 4px 0;\">" + processInline(line.mid(3)) + "</h3>";
            continue;
        }
        if (line.startsWith("# ")) {
            if (inUl) { result += "</ul>"; inUl = false; }
            if (inOl) { result += "</ol>"; inOl = false; }
            result += "<h2 style=\"margin:14px 0 6px 0;\">" + processInline(line.mid(2)) + "</h2>";
            continue;
        }

        // ==============================================
        // Unordered list: - item  or  * item
        // ==============================================
        static QRegularExpression rxUl(R"(^[\-\*]\s+(.+))");
        auto ulMatch = rxUl.match(line);
        if (ulMatch.hasMatch()) {
            if (inOl) { result += "</ol>"; inOl = false; }
            if (!inUl) { result += "<ul style=\"margin:4px 0 4px 20px;\">"; inUl = true; }
            result += "<li>" + processInline(ulMatch.captured(1)) + "</li>";
            continue;
        }

        // ==============================================
        // Ordered list: 1. item
        // ==============================================
        static QRegularExpression rxOl(R"(^[0-9]+\.\s+(.+))");
        auto olMatch = rxOl.match(line);
        if (olMatch.hasMatch()) {
            if (inUl) { result += "</ul>"; inUl = false; }
            if (!inOl) { result += "<ol style=\"margin:4px 0 4px 20px;\">"; inOl = true; }
            result += "<li>" + processInline(olMatch.captured(1)) + "</li>";
            continue;
        }

        // ==============================================
        // Close any open list if we hit a non-list line
        // ==============================================
        if (inUl) { result += "</ul>"; inUl = false; }
        if (inOl) { result += "</ol>"; inOl = false; }

        // ==============================================
        // Empty line → paragraph break
        // ==============================================
        if (line.trimmed().isEmpty()) {
            result += "<br>";
            continue;
        }

        // ==============================================
        // Normal text line — apply inline formatting
        // ==============================================
        result += processInline(line) + "<br>";
    }

    // Close any unclosed lists
    if (inUl) result += "</ul>";
    if (inOl) result += "</ol>";

    // Close unclosed code block (defensive)
    if (inCodeBlock && !codeBlockContent.isEmpty()) {
        QString escaped = codeBlockContent.toHtmlEscaped();
        result += QStringLiteral(
            "<pre style=\"background:#1a1a1a; color:#d4d4d4; "
            "padding:10px; border-radius:6px; "
            "font-family:monospace; white-space:pre-wrap; "
            "margin:8px 0;\"><code>%1</code></pre>").arg(escaped);
    }

    return result;
}

