/*
 * Note: This file is mostly the Qt syntax-highlighting example.
 * It contains C++ syntax-highlighting that will eventually be
 * removed.
 */

#ifndef MARKDOWNSYNTAX_H
#define MARKDOWNSYNTAX_H

#include <QSyntaxHighlighter>
#include <QTextCharFormat>
#include <QRegularExpression>

QT_BEGIN_NAMESPACE
class QTextDocument;
QT_END_NAMESPACE

class MarkdownSyntax : QSyntaxHighlighter
{
public:
    MarkdownSyntax(QTextDocument *parent = nullptr);
protected:
    void highlightBlock(const QString &text) override;
private:
    QRegularExpression keyword(QString regex);

    struct HighlightingRule
    {
        QRegularExpression pattern;
        QTextCharFormat format;
    };
    QVector<HighlightingRule> highlightingRules;

    QRegularExpression commentStartExpression;
    QRegularExpression commentEndExpression;

    // Markdown Formatting
    QTextCharFormat Headings[6];


    QTextCharFormat keywordFormat;
    QTextCharFormat classFormat;
    QTextCharFormat singleLineCommentFormat;
    QTextCharFormat multiLineCommentFormat;
    QTextCharFormat quotationFormat;
    QTextCharFormat functionFormat;
};

#endif // MARKDOWNSYNTAX_H
