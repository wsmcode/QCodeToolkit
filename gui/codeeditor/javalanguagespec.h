#ifndef JAVALANGUAGESPEC_H
#define JAVALANGUAGESPEC_H

#include "languagespec.h"

class JavaLanguageSpec : public LanguageSpec
{
public:
    QsciLexer * createLexer() override;

    void setupAPIs(QsciAPIs *apis) override;
    void applySyntaxHighlighting(QsciLexer *lexer, const SyntaxColor &syntax) override;

    QString languageName() const override;
    QStringList fileExtensions() const override;
};

#endif // JAVALANGUAGESPEC_H
