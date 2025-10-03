#ifndef EDITOR_CONFIG_H
#define EDITOR_CONFIG_H

#include <QString>
#include <QFont>
#include <QColor>
#include <QFontDatabase>

// 语言类型枚举
enum class LanguageType {
    None,
    C,
    CPP,
    Python,
    Java,
    JavaScript,
    Rust,
    CSharp,
    Go,
    PHP,
    HTML,
    CSS,
    SQL,
    Markdown,
    Default
};
using Language = LanguageType;

enum class ThemeType {
    LightTheme,
    DarkTheme
};

// 语法高亮配置
struct SyntaxHighlighting {
    QColor keywordColor = QColor(136, 18, 128);       // 关键字颜色
    QColor classColor = QColor(26, 151, 126);         // 类名颜色
    QColor functionColor = QColor(17, 125, 189);      // 函数名颜色
    QColor variableColor = QColor(174, 58, 67);       // 变量颜色
    QColor stringColor = QColor(102, 145, 71);        // 字符串颜色
    QColor commentColor = QColor(106, 153, 85);       // 注释颜色
    QColor numberColor = QColor(159, 104, 52);        // 数字颜色
    QColor preprocessorColor = QColor(36, 132, 144);  // 预处理器颜色
};
using SyntaxColor = SyntaxHighlighting;

// 编辑器配置
struct EditorConfig {
    Language language = Language::CPP;
    QFont font;
    QColor backgroundColor = Qt::white;
    QColor foregroundColor = Qt::black;
    QColor marginBackgroundColor = QColor(245, 245, 245);
    QColor marginForegroundColor = QColor(96, 96, 96);
    QColor selectionColor = QColor(0, 120, 215);
    SyntaxHighlighting syntaxHighlighting;
    int tabWidth = 4;
    bool useSpacesForTabs = true;
    bool autoIndent = true;
    bool codeFolding = true;
    int edgeColumn = 800;
    bool lineNumbers = true;
    bool autoCompletion = true;
    bool autoBracketCompletion = true;

    EditorConfig()
    {
        font = QFontDatabase::systemFont(QFontDatabase::FixedFont);
        font.setPointSize(11);
    }


    EditorConfig(Language language)
    {
        this->language = language;

        // 语言特定配置
        switch (language) {
        case Language::Python:
            useSpacesForTabs = true;
            tabWidth = 4;
            break;
        case Language::Java:
            useSpacesForTabs = true;
            tabWidth = 4;
            break;
        case Language::JavaScript:
            useSpacesForTabs = true;
            tabWidth = 2;
            break;
        case Language::CPP:
        case Language::C:
            useSpacesForTabs = false;
            tabWidth = 4;
            break;
        default:
            break;
        }
    }
};

#endif // EDITOR_CONFIG_H
