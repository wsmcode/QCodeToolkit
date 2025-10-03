#include "pythonlanguagespec.h"

#include <Qsci/qscilexerpython.h>

QsciLexer *PythonLanguageSpec::createLexer()
{
    return new QsciLexerPython;
}

void PythonLanguageSpec::setupAPIs(QsciAPIs *apis)
{
    // Python关键字
    apis->add("def");
    apis->add("class");
    apis->add("import");
    apis->add("from");
    apis->add("as");
    apis->add("if");
    apis->add("else");
    apis->add("elif");
    apis->add("for");
    apis->add("while");
    apis->add("try");
    apis->add("except");
    apis->add("finally");
    apis->add("with");
    apis->add("return");
    apis->add("yield");
    apis->add("pass");
    apis->add("break");
    apis->add("continue");

    // 内置函数和类型
    apis->add("print");
    apis->add("len");
    apis->add("range");
    apis->add("list");
    apis->add("dict");
    apis->add("set");
    apis->add("tuple");
    apis->add("str");
    apis->add("int");
    apis->add("float");
    apis->add("bool");
}

void PythonLanguageSpec::applySyntaxHighlighting(QsciLexer *lexer, const SyntaxColor &syntax)
{
    auto* pythonLexer = dynamic_cast<QsciLexerPython*>(lexer);
    if (!pythonLexer) return;

    pythonLexer->setColor(syntax.keywordColor, QsciLexerPython::Keyword);
    pythonLexer->setColor(syntax.classColor, QsciLexerPython::ClassName);
    pythonLexer->setColor(syntax.functionColor, QsciLexerPython::FunctionMethodName);
    pythonLexer->setColor(syntax.stringColor, QsciLexerPython::DoubleQuotedString);
    pythonLexer->setColor(syntax.stringColor, QsciLexerPython::SingleQuotedString);
    pythonLexer->setColor(syntax.commentColor, QsciLexerPython::Comment);
    pythonLexer->setColor(syntax.numberColor, QsciLexerPython::Number);
}

QString PythonLanguageSpec::languageName() const
{
    return "Python";
}

QStringList PythonLanguageSpec::fileExtensions() const
{
    return {"py", "pyw", "pyx"};
}
