#include "javalanguagespec.h"

#include <Qsci/qscilexerjava.h>

QsciLexer *JavaLanguageSpec::createLexer()
{
    return new QsciLexerJava;
}

void JavaLanguageSpec::setupAPIs(QsciAPIs *apis)
{
    apis->add("public");
    apis->add("class");
    apis->add("void");
    apis->add("static");
    apis->add("import");
    apis->add("package");
    apis->add("private");
    apis->add("protected");
    apis->add("final");
    apis->add("interface");
    apis->add("extends");
    apis->add("implements");
}

void JavaLanguageSpec::applySyntaxHighlighting(QsciLexer *lexer, const SyntaxColor &syntax)
{
    auto* javaLexer = dynamic_cast<QsciLexerJava*>(lexer);
    if (!javaLexer) return;

    javaLexer->setColor(syntax.keywordColor, QsciLexerJava::Keyword);
    javaLexer->setColor(syntax.classColor, QsciLexerJava::GlobalClass);
    javaLexer->setColor(syntax.functionColor, QsciLexerJava::Identifier);
    javaLexer->setColor(syntax.stringColor, QsciLexerJava::DoubleQuotedString);
    javaLexer->setColor(syntax.stringColor, QsciLexerJava::SingleQuotedString);
    javaLexer->setColor(syntax.commentColor, QsciLexerJava::Comment);
    javaLexer->setColor(syntax.numberColor, QsciLexerJava::Number);
}

QString JavaLanguageSpec::languageName() const
{
    return "Java";
}

QStringList JavaLanguageSpec::fileExtensions() const
{
    return {"java"};
}
