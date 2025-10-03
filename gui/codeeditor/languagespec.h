#ifndef LANGUAGESPEC_H
#define LANGUAGESPEC_H

#include <Qsci/qsciapis.h>
#include <Qsci/qscilexer.h>
#include <editor_config.h>

class LanguageSpec
{
public:
    virtual ~LanguageSpec() = default;

    // 创建语法分析器
    virtual QsciLexer* createLexer() = 0;

    // 设置API补全
    virtual void setupAPIs(QsciAPIs *apis) = 0;

    // 应用语法高亮配置
    virtual void applySyntaxHighlighting(QsciLexer *lexer, const SyntaxColor &syntax) = 0;

    // 获取语言名称
    virtual QString languageName() const = 0;

    // 获取文件扩展名
    virtual QStringList fileExtensions() const = 0;
};

#endif // LANGUAGESPEC_H
