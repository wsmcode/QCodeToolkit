#include "cpplanguagespec.h"

#include <Qsci/qscilexercpp.h>

QsciLexer *CppLanguageSpec::createLexer()
{
    return new QsciLexerCPP;
}

void CppLanguageSpec::setupAPIs(QsciAPIs *apis)
{
    // C++关键字和常用API
    apis->add("#include");
    apis->add("#define");
    apis->add("#undef");
    apis->add("#ifndef");
    apis->add("#ifdef");
    apis->add("#endif");
    apis->add("#pragma");
    apis->add("#pragma once");

    // 类型
    apis->add("class");
    apis->add("struct");
    apis->add("enum");
    apis->add("union");
    apis->add("template");
    apis->add("typename");

    // 访问修饰符
    apis->add("public");
    apis->add("private");
    apis->add("protected");

    // 类型限定符
    apis->add("void");
    apis->add("int");
    apis->add("float");
    apis->add("double");
    apis->add("char");
    apis->add("bool");
    apis->add("long");
    apis->add("short");
    apis->add("unsigned");
    apis->add("signed");

    // 其他关键字
    apis->add("const");
    apis->add("static");
    apis->add("virtual");
    apis->add("explicit");
    apis->add("inline");
    apis->add("friend");
    apis->add("namespace");
    apis->add("using");
    apis->add("typedef");
}

void CppLanguageSpec::applySyntaxHighlighting(QsciLexer *lexer, const SyntaxColor &syntax)
{
    auto* cppLexer = dynamic_cast<QsciLexerCPP*>(lexer);
    if (!cppLexer) return;

    // 设置各种语法元素的颜色
    cppLexer->setColor(syntax.keywordColor, QsciLexerCPP::Keyword);
    cppLexer->setColor(syntax.classColor, QsciLexerCPP::GlobalClass);
    cppLexer->setColor(syntax.functionColor, QsciLexerCPP::Identifier);
//    cppLexer->setColor(syntax.variableColor, QsciLexerCPP::Variable);
    cppLexer->setColor(syntax.stringColor, QsciLexerCPP::DoubleQuotedString);
    cppLexer->setColor(syntax.stringColor, QsciLexerCPP::SingleQuotedString);
    cppLexer->setColor(syntax.commentColor, QsciLexerCPP::Comment);
    cppLexer->setColor(syntax.commentColor, QsciLexerCPP::CommentLine);
    cppLexer->setColor(syntax.numberColor, QsciLexerCPP::Number);
    cppLexer->setColor(syntax.preprocessorColor, QsciLexerCPP::PreProcessor);
}

QString CppLanguageSpec::languageName() const
{
    return "C++";
}

QStringList CppLanguageSpec::fileExtensions() const
{
    return {"cpp", "cxx", "cc", "c", "h", "hpp", "hxx"};
}
