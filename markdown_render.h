#pragma once

#include <QString>

// =====================================================
// Lightweight Markdown → HTML converter
// Designed for ChatGPT API response formatting.
//
// Supports:
//   - Paragraph breaks (\n\n)
//   - Fenced code blocks (```lang ... ```)
//   - Inline code (`code`)
//   - Bold (**text**)
//   - Italic (*text*)
//   - Unordered lists (- item, * item)
//   - Ordered lists (1. item)
//   - Headings (### text)
//
// No external dependencies. Pure Qt string processing.
// =====================================================

class MarkdownRender
{
public:
    /// Convert markdown text to Qt-compatible HTML
    static QString toHtml(const QString &markdown);
};

