#ifndef JAVASCRIPTLANGUAGESPEC_H
#define JAVASCRIPTLANGUAGESPEC_H

#include "languagespec.h"

class JavaScriptLanguageSpec : public LanguageSpec
{
public:
    QsciLexer * createLexer() override;

    void setupAPIs(QsciAPIs *apis) override;
    void applySyntaxHighlighting(QsciLexer *lexer, const SyntaxColor &syntax) override;

    QString languageName() const override;
    QStringList fileExtensions() const override;
};

#endif // JAVASCRIPTLANGUAGESPEC_H
