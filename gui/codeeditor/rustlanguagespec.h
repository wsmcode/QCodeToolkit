#ifndef RUSTLANGUAGESPEC_H
#define RUSTLANGUAGESPEC_H

#include "languagespec.h"

class RustLanguageSpec : public LanguageSpec
{
public:
    QsciLexer * createLexer() override;

    void setupAPIs(QsciAPIs *apis) override;
    void applySyntaxHighlighting(QsciLexer *lexer, const SyntaxColor &syntax) override;

    QString languageName() const override;
    QStringList fileExtensions() const override;
};

#endif // RUSTLANGUAGESPEC_H
