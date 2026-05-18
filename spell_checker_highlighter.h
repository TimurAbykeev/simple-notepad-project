#ifndef SPELL_CHECKER_HIGHLIGHTER_H
#define SPELL_CHECKER_HIGHLIGHTER_H

#include "spell_checker.h"

#include <QRegularExpression>
#include <QSyntaxHighlighter>
#include <QTextCharFormat>

class spell_checker_highlighter : public QSyntaxHighlighter {
public:
    // Takes a pointer to an existing spell_checker so we don't load words.txt twice.
    explicit spell_checker_highlighter(const spell_checker* checker, QTextDocument* parent)
        : QSyntaxHighlighter(parent)
        , checker_(checker)
    {
        // Red wavy underline — the standard look for spell check errors
        format_.setUnderlineStyle(QTextCharFormat::SpellCheckUnderline);
        format_.setUnderlineColor(Qt::red);
    }

protected:
    // Qt calls this automatically for every paragraph (block) that changes.
    // We scan the block for words and underline any that fail the spell check.
    void highlightBlock(const QString& text) override
    {
        // Match sequences of word characters (letters, digits, apostrophes)
        static const QRegularExpression word_re(R"(\b[a-zA-Z']+\b)");

        auto it = word_re.globalMatch(text);
        while (it.hasNext()) {
            const auto match = it.next();
            if (!checker_->is_correct(match.captured())) {
                setFormat(match.capturedStart(), match.capturedLength(), format_);
            }
        }
    }

private:
    const spell_checker* checker_;
    QTextCharFormat format_;
};

#endif // SPELL_CHECKER_HIGHLIGHTER_H
