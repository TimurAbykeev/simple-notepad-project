#ifndef SPELL_CHECKER_H
#define SPELL_CHECKER_H

#include <QFile>
#include <QTextStream>
#include <set>
#include <string>
#include <vector>

class spell_checker {
public:
    // Loads the word list from the given file path into a set for O(log n) lookup.
    // One lowercase word per line; non-alphabetic entries are ignored.
    explicit spell_checker(const QString& words_file)
    {
        QFile file(words_file);
        if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
            return;
        }
        QTextStream in(&file);
        while (!in.atEnd()) {
            const auto line = in.readLine().trimmed().toLower().toStdString();
            if (!line.empty()) {
                words_.insert(line);
            }
        }
    }

    // Returns true if the word is spelled correctly.
    // The word is lowercased and stripped of non-alphabetic characters before lookup.
    bool is_correct(const QString& word) const
    {
        const auto cleaned = clean(word);
        if (cleaned.empty()) {
            return true; // numbers, punctuation etc. are not checked
        }
        return words_.count(cleaned) > 0;
    }

    // Returns up to max_count suggestions for a misspelled word.
    // Strategy: find all dictionary words within edit distance 1
    // (one insertion, deletion, substitution, or transposition).
    std::vector<QString> suggestions(const QString& word, const int max_count = 5) const
    {
        const auto target = clean(word);
        if (target.empty()) {
            return {};
        }

        std::vector<QString> result;
        for (const auto& dict_word : words_) {
            if (edit_distance_1(target, dict_word)) {
                result.push_back(QString::fromStdString(dict_word));
                if (static_cast<int>(result.size()) >= max_count) {
                    break;
                }
            }
        }
        return result;
    }

private:
    std::set<std::string> words_;

    // Lowercases the word and removes non-alphabetic characters.
    static std::string clean(const QString& word)
    {
        std::string result;
        for (const QChar ch : word.toLower()) {
            if (ch.isLetter()) {
                result += ch.toLatin1();
            }
        }
        return result;
    }

    // Returns true if the two strings differ by at most one edit
    // (insertion, deletion, substitution, or transposition).
    static bool edit_distance_1(const std::string& a, const std::string& b)
    {
        const auto la = a.size();
        const auto lb = b.size();

        if (la == lb) {
            // Check for substitution or transposition
            int diffs = 0;
            for (std::size_t i = 0; i < la; ++i) {
                if (a[i] != b[i]) {
                    ++diffs;
                }
            }
            if (diffs == 1) {
                return true; // one substitution
            }
            if (diffs == 2 && la >= 2) {
                // Check transposition: ab -> ba
                for (std::size_t i = 0; i + 1 < la; ++i) {
                    if (a[i] == b[i + 1] && a[i + 1] == b[i]) {
                        std::string swapped = a;
                        std::swap(swapped[i], swapped[i + 1]);
                        if (swapped == b) {
                            return true;
                        }
                    }
                }
            }
            return false;
        }

        if (la + 1 == lb) {
            return one_deletion(b, a); // b has one extra char compared to a
        }
        if (lb + 1 == la) {
            return one_deletion(a, b); // a has one extra char compared to b
        }

        return false;
    }

    // Returns true if removing exactly one character from 'longer' gives 'shorter'.
    static bool one_deletion(const std::string& longer, const std::string& shorter)
    {
        const auto n = shorter.size();
        for (std::size_t skip = 0; skip <= n; ++skip) {
            bool match = true;
            for (std::size_t i = 0; i < n; ++i) {
                if (longer[i < skip ? i : i + 1] != shorter[i]) {
                    match = false;
                    break;
                }
            }
            if (match) {
                return true;
            }
        }
        return false;
    }
};

#endif // SPELL_CHECKER_H
