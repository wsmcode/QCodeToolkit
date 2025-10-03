#include "javascriptlanguagespec.h"

#include <Qsci/qscilexerjavascript.h>

QsciLexer *JavaScriptLanguageSpec::createLexer()
{
    return new QsciLexerJavaScript;
}

void JavaScriptLanguageSpec::setupAPIs(QsciAPIs *apis)
{
    apis->add("function");
    apis->add("var");
    apis->add("let");
    apis->add("const");
    apis->add("if");
    apis->add("else");
    apis->add("for");
    apis->add("while");
    apis->add("return");
    apis->add("class");
    apis->add("export");
    apis->add("import");
    apis->add("async");
    apis->add("await");
}

void JavaScriptLanguageSpec::applySyntaxHighlighting(QsciLexer *lexer, const SyntaxColor &syntax)
{
    auto* jsLexer = dynamic_cast<QsciLexerJavaScript*>(lexer);
    if (!jsLexer) return;

    jsLexer->setColor(syntax.keywordColor, QsciLexerJavaScript::Keyword);
    jsLexer->setColor(syntax.classColor, QsciLexerJavaScript::GlobalClass);
    jsLexer->setColor(syntax.functionColor, QsciLexerJavaScript::Identifier);
    jsLexer->setColor(syntax.stringColor, QsciLexerJavaScript::DoubleQuotedString);
    jsLexer->setColor(syntax.stringColor, QsciLexerJavaScript::SingleQuotedString);
    jsLexer->setColor(syntax.commentColor, QsciLexerJavaScript::Comment);
    jsLexer->setColor(syntax.commentColor, QsciLexerJavaScript::CommentLine);
    jsLexer->setColor(syntax.numberColor, QsciLexerJavaScript::Number);
}

QString JavaScriptLanguageSpec::languageName() const
{
    return "JavaScript";
}

QStringList JavaScriptLanguageSpec::fileExtensions() const
{
    return {"js", "mjs", "cjs"};
}
