#ifndef CPPLANGUAGESPEC_H
#define CPPLANGUAGESPEC_H

#include "languagespec.h"

class CppLanguageSpec : public LanguageSpec
{
public:
    QsciLexer * createLexer() override;

    void setupAPIs(QsciAPIs *apis) override;
    void applySyntaxHighlighting(QsciLexer *lexer, const SyntaxColor &syntax) override;

    QString languageName() const override;
    QStringList fileExtensions() const override;
};

#endif // CPPLANGUAGESPEC_H
