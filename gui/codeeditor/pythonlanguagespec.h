#ifndef PYTHONLANGUAGESPEC_H
#define PYTHONLANGUAGESPEC_H

#include "languagespec.h"

class PythonLanguageSpec : public LanguageSpec
{
public:
    QsciLexer * createLexer() override;

    void setupAPIs(QsciAPIs *apis) override;
    void applySyntaxHighlighting(QsciLexer *lexer, const SyntaxColor &syntax) override;

    QString languageName() const override;
    QStringList fileExtensions() const override;
};

#endif // PYTHONLANGUAGESPEC_H
