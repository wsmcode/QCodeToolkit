#include "rustlanguagespec.h"

#include <Qsci/qscilexercpp.h>


QsciLexer *RustLanguageSpec::createLexer()
{
    return new QsciLexerCPP; // Qsci没有专门的Rust词法分析器，使用C++作为基础
}

void RustLanguageSpec::setupAPIs(QsciAPIs *apis)
{
    apis->add("fn");
    apis->add("let");
    apis->add("mut");
    apis->add("struct");
    apis->add("enum");
    apis->add("impl");
    apis->add("trait");
    apis->add("pub");
    apis->add("use");
    apis->add("mod");
    apis->add("match");
    apis->add("if");
    apis->add("else");
    apis->add("loop");
    apis->add("while");
    apis->add("for");
    apis->add("return");
}

void RustLanguageSpec::applySyntaxHighlighting(QsciLexer *lexer, const SyntaxColor &syntax)
{
    auto* cppLexer = dynamic_cast<QsciLexerCPP*>(lexer);
    if (!cppLexer) return;

    // 使用C++词法分析器的颜色设置
    cppLexer->setColor(syntax.keywordColor, QsciLexerCPP::Keyword);
    cppLexer->setColor(syntax.classColor, QsciLexerCPP::GlobalClass);
    cppLexer->setColor(syntax.functionColor, QsciLexerCPP::Identifier);
    cppLexer->setColor(syntax.stringColor, QsciLexerCPP::DoubleQuotedString);
    cppLexer->setColor(syntax.stringColor, QsciLexerCPP::SingleQuotedString);
    cppLexer->setColor(syntax.commentColor, QsciLexerCPP::Comment);
    cppLexer->setColor(syntax.commentColor, QsciLexerCPP::CommentLine);
    cppLexer->setColor(syntax.numberColor, QsciLexerCPP::Number);
}

QString RustLanguageSpec::languageName() const
{
    return "Rust";
}

QStringList RustLanguageSpec::fileExtensions() const
{
    return {"rs"};
}
