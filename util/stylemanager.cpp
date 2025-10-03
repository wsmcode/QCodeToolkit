#include "stylemanager.h"

#include <QApplication>
#include <QFile>
#include <QWidget>
#include <QDebug>
#include <QRegularExpression>
#include <codeeditor.h>

StyleManager::StyleManager(QObject *parent) : QObject(parent)
{
    setThemeFile(Theme::LightTheme, ":/res/style/light_theme.qss");
    setThemeFile(Theme::DarkTheme, ":/res/style/dark_theme.qss");
    setEditorStyleFile(":/res/style/editor_style.conf");

    applyStyle();
}

void StyleManager::setTheme(Theme theme)
{
    if(m_currentTheme != theme)
    {
        m_currentTheme = theme;
        applyStyle();
        emit themeChanged(m_currentTheme);
    }
}

Theme StyleManager::currentTheme() const
{
    return m_currentTheme;
}

void StyleManager::registerWidget(QWidget *widget)
{
    if(widget && !m_registeredWidgets.contains(widget))
    {
        m_registeredWidgets.append(QPointer<QWidget>(widget));
        // 应用当前样式
        if(CodeEditor *editor = qobject_cast<CodeEditor*>(widget)) // 处理CodeEditor
        {
            applyEditorStyle(editor);
        }
        else
        {
            QString style = m_themeWidgetStyles.value(m_currentTheme).value("#" + widget->objectName());
            if(!style.isEmpty()) widget->setStyleSheet(style);
            else
            {
                qWarning() << "The widget style is empty: " << widget;
            }
        }
        QObject::connect(widget, &QObject::destroyed, [=](){m_registeredWidgets.removeAll(widget);});
    }
}

void StyleManager::unregisterWidget(QWidget *widget)
{
    m_registeredWidgets.removeAll(widget);
}

void StyleManager::setThemeFile(Theme theme, const QString &filePath)
{
    m_themeFiles[theme] = filePath;
    loadTheme(theme);
}

QString StyleManager::themeFile(Theme theme) const
{
    return m_themeFiles.value(theme);
}

void StyleManager::setEditorStyleFile(const QString &filePath)
{
    m_editorStyleFile = filePath;
    loadEditorStyles();
}

QString StyleManager::editorStyleFile() const
{
    return m_editorStyleFile;
}

bool StyleManager::loadTheme(Theme theme)
{
    QString path = m_themeFiles.value(theme);
    if(path.isEmpty())
    {
        qWarning() << "No file path set for theme:" << (theme == Theme::DarkTheme ? "DarkTheme" : "LightTheme");
        return false;
    }

    QFile file(path);
    if(!file.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        qWarning() << "Cannot open style file:" << path;
        return false;
    }

    QTextStream in(&file);
    QString content = in.readAll();
    m_themeContent[theme] = content;
    file.close();

    // 清除旧数据并解析新内容
    clearThemeData(theme); // 清除主题，防止主题存在而无法设置
    parseQssContent(theme, content);
    return true;
}

bool StyleManager::loadEditorStyles()
{
    if(m_editorStyleFile.isEmpty())
    {
        qWarning() << "No editor style file set";
        return false;
    }

    return parseEditorStyleFile(m_editorStyleFile);
}

void StyleManager::applyStyle()
{
    // 应用全局样式
    qApp->setStyleSheet(m_themeGlobalStyles.value(m_currentTheme));

    // 应用特定对象样式到已注册的控件
    const auto &widgetStyles = m_themeWidgetStyles.value(m_currentTheme);
    for(auto &widget : m_registeredWidgets)
    {
        if(widget)
        {
            if(CodeEditor *editor = qobject_cast<CodeEditor*>(widget.data()))
            {
                applyEditorStyle(editor);
            }
            else
            {
                QString style = widgetStyles.value("#" + widget->objectName());
                widget->setStyleSheet(style);
            }
        }
    }
}

QMap<QString, QColor> StyleManager::getEditorColors(Theme theme) const
{
    return m_editorColors.value(theme);
}

void StyleManager::parseQssContent(Theme theme, const QString &content)
{
//    // 全局样式
//    QRegularExpression globalPattern(R"((.*?)(?=#[\w\s\-\:\,\.\#\*\[\]\=\~\|\^\$\>\+\_]+{))");
//    globalPattern.setPatternOptions(QRegularExpression::DotMatchesEverythingOption);

//    QRegularExpressionMatch globalMatch = globalPattern.match(content);
//    if(globalMatch.hasMatch())
//    {
//        m_themeGlobalStyles[theme] = globalMatch.captured(1).trimmed();
//    }
//    else
//    {
//        // 如果没有特定对象样式，整个内容都是全局样式
//        m_themeGlobalStyles[theme] = content.trimmed();
//    }

//    // 特定对象样式
//    QRegularExpression selectorPattern(R"((#[\w\s\-\:\,\.\#\*\[\]\=\~\|\^\$\>\+\_]+)\s*{([^}]*)(?=}))");
//    selectorPattern.setPatternOptions(QRegularExpression::DotMatchesEverythingOption);
//    QRegularExpressionMatchIterator it = selectorPattern.globalMatch(content);
//    QMap<QString, QString> widgetStyle;
//    while(it.hasNext())
//    {
//        QRegularExpressionMatch match = it.next();
//        QString selector = match.captured(1).trimmed();
//        QString style = match.captured(2).trimmed();

//        QRegularExpression baseNamePattern(R"(#(\w+))");
//        QRegularExpressionMatch selectorMatch = baseNamePattern.match(selector);
//        QString objectName;
//        if(selectorMatch.hasMatch()) objectName = "#" + selectorMatch.captured(1);

//        // 检查样式块中是否有嵌套花括号
//        if(style.contains('{') || style.contains('}'))
//        {
//            qWarning() << "Nested braces found in style block for selector:" << selector;
//            continue;
//        }


//        QString currentStyle = widgetStyle.value(objectName, "");
//        if(!currentStyle.isEmpty()) currentStyle += "\n";
//        currentStyle += selector + " { " + style + " }";

//        widgetStyle[objectName] = currentStyle;
//    }
//    m_themeWidgetStyles[theme] = widgetStyle;

    QString simplifiedContent = content;
    // 移除注释
    simplifiedContent.remove(QRegularExpression("/\\*.*?\\*/", QRegularExpression::DotMatchesEverythingOption));
    // 简化空白字符
    simplifiedContent = simplifiedContent.simplified();

    // 分离全局样式和特定选择器样式
    QString globalStyle;
    QMap<QString, QString> widgetStyles;

    // 使用更健壮的方法解析CSS
    QRegularExpression rulePattern(R"((.*?)\s*\{([^}]*)\})");
    rulePattern.setPatternOptions(QRegularExpression::DotMatchesEverythingOption);

    int pos = 0;
    QRegularExpressionMatch match;
    while((match = rulePattern.match(simplifiedContent, pos)).hasMatch())
    {
        QString selector = match.captured(1).trimmed();
        QString rules = match.captured(2).trimmed();
        pos = match.capturedEnd();

        if(selector.isEmpty() || rules.isEmpty()) continue;

        // 判断是否是全局样式（不包含选择器）
        if(!selector.contains('#')) globalStyle += selector + " { " + rules + " }\n";
        else
        {
            // 提取对象名
            QRegularExpression idPattern(R"(#(\w+))");
            QRegularExpressionMatch idMatch = idPattern.match(selector);
            if(idMatch.hasMatch())
            {
                QString objectName = "#" + idMatch.captured(1);
                QString currentStyle = widgetStyles.value(objectName, "");

                if(!currentStyle.isEmpty()) currentStyle += "\n";
                currentStyle += selector + " { " + rules + " }";
                widgetStyles[objectName] = currentStyle;
            }
        }
    }

    // 存储解析结果
    m_themeGlobalStyles[theme] = globalStyle;
    m_themeWidgetStyles[theme] = widgetStyles;
}

void StyleManager::clearThemeData(Theme theme)
{
    m_themeGlobalStyles.remove(theme);
    m_themeWidgetStyles.remove(theme);
}

bool StyleManager::parseEditorStyleFile(const QString &filePath)
{
    QFile file(filePath);
    if(!file.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        qWarning() << "Cannot open editor style file:" << filePath;
        return false;
    }

    QTextStream in(&file);
    Theme currentTheme = Theme::LightTheme;
    m_editorColors.clear();

    while(!in.atEnd())
    {
        QString line = in.readLine().trimmed();

        if(line.isEmpty() || line.startsWith(';')) continue; // 跳过空行和注释

        // 检查主题节
        if(line.startsWith('[') && line.endsWith(']'))
        {
            QString themeName = line.mid(1, line.length() - 2);

            if(themeName == "LightTheme") currentTheme = Theme::LightTheme;
            else if(themeName == "DarkTheme") currentTheme = Theme::DarkTheme;

            continue;
        }
        // 解析键值对
        int equalsPos = line.indexOf('=');
        if(equalsPos > 0)
        {
            QString key = line.left(equalsPos).trimmed();
            QString value = line.mid(equalsPos + 1).trimmed();

            if(value.startsWith('#') && value.length() >= 7)
            {
                QColor color(value);
                if(color.isValid()) m_editorColors[currentTheme][key] = color;
            }
        }
    }

    file.close();
    qDebug() << "Loaded editor styles for" << m_editorColors.size() << "themes";
    return true;
}

void StyleManager::applyEditorStyle(CodeEditor *editor)
{
    if(!editor) return;

    auto themeColors = m_editorColors.value(m_currentTheme);
    if(themeColors.isEmpty())
    {
        qWarning() << "No editor colors found for current theme";
        return;
    }

    // 获取当前配置并更新颜色
    EditorConfig config = editor->getConfig();

    // 更新基本颜色
    if(themeColors.contains("backgroundColor"))
        config.backgroundColor = themeColors["backgroundColor"];
    if(themeColors.contains("foregroundColor"))
        config.foregroundColor = themeColors["foregroundColor"];
    if(themeColors.contains("marginBackgroundColor"))
        config.marginBackgroundColor = themeColors["marginBackgroundColor"];
    if(themeColors.contains("marginForegroundColor"))
        config.marginForegroundColor = themeColors["marginForegroundColor"];
    if(themeColors.contains("selectionColor"))
        config.selectionColor = themeColors["selectionColor"];

    // 更新语法高亮颜色
    if(themeColors.contains("keywordColor"))
        config.syntaxHighlighting.keywordColor = themeColors["keywordColor"];
    if(themeColors.contains("classColor"))
        config.syntaxHighlighting.classColor = themeColors["classColor"];
    if(themeColors.contains("functionColor"))
        config.syntaxHighlighting.functionColor = themeColors["functionColor"];
    if(themeColors.contains("variableColor"))
        config.syntaxHighlighting.variableColor = themeColors["variableColor"];
    if(themeColors.contains("stringColor"))
        config.syntaxHighlighting.stringColor = themeColors["stringColor"];
    if(themeColors.contains("commentColor"))
        config.syntaxHighlighting.commentColor = themeColors["commentColor"];
    if(themeColors.contains("numberColor"))
        config.syntaxHighlighting.numberColor = themeColors["numberColor"];
    if(themeColors.contains("preprocessorColor"))
        config.syntaxHighlighting.preprocessorColor = themeColors["preprocessorColor"];

    // 应用更新后的配置
    editor->setConfig(config);
}
