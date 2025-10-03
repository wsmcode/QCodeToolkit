#include "codeeditor.h"
#include "languagespecfactory.h"

//以C++语法作为例子，该语法分析器的头文件
#include <Qsci/qscilexercpp.h>
#include <Qsci/qscilexerpython.h>
#include <Qsci/qscilexerjava.h>
#include <Qsci/qscilexerjavascript.h>
//设置代码提示功能，依靠QsciAPIs类实现
#include <Qsci/qsciapis.h>

#include <QDebug>
#include <QKeyEvent>

CodeEditor::CodeEditor(QWidget *parent) : QsciScintilla(parent)
{
    setContextMenuPolicy(Qt::NoContextMenu); // 禁用默认右键菜单，使用自定义菜单
    setHorizontalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);

    setConfig(defaultConfig());

    QObject::connect(this, &CodeEditor::marginClicked, [=](int margin, int line, Qt::KeyboardModifiers){
        if(margin == 0) toggleBreakpoint(line);
    });
}

CodeEditor::~CodeEditor()
{
    if(m_languageSpec)
    {
        delete m_languageSpec;
        m_languageSpec = nullptr;
    }
}

void CodeEditor::setConfig(const EditorConfig &config)
{
    m_config = config;
    setupLexer(config.language);

    // ==== 应用通用设置 ====
    setUtf8(true);
    setFolding(config.codeFolding ? QsciScintilla::BoxedTreeFoldStyle : QsciScintilla::NoFoldStyle);

    // ==== 设置边线 ====
    // EdgeLine: 在指定列显示竖线，帮助控制代码宽度
    setEdgeMode(QsciScintilla::EdgeLine);
    setEdgeColumn(config.edgeColumn); // 设置边线所在的列数
    setEdgeColor(QColor(60, 60, 60)); // 边线颜色

    // ==== 设置括号匹配 ====
    // SloppyBraceMatch: 宽松的括号匹配，即使不在同一行也能匹配
    setBraceMatching(QsciScintilla::SloppyBraceMatch);
    setMatchedBraceBackgroundColor(QColor(180, 238, 180));  // 匹配括号背景色
    setMatchedBraceForegroundColor(Qt::red);                // 匹配括号前景色

    // ==== 设置缩进 ====
    setIndentationsUseTabs(!config.useSpacesForTabs);   // true=使用制表符，false=使用空格
    setIndentationWidth(config.tabWidth);               // 缩进宽度(字符数)
    setTabWidth(config.tabWidth);                       // 制表符宽度
    setAutoIndent(config.autoIndent);                   // 自动缩进
    setTabIndents(true);                                // Tab键执行缩进
    setBackspaceUnindents(true);                        // Backspace键取消缩进

    // ==== 设置选中文本颜色 ====
    setSelectionBackgroundColor(config.selectionColor); // 选中文本背景色
    setSelectionForegroundColor(Qt::white);             // 选中文本前景色

    // ==== 设置颜色主题 ====
    setColor(config.foregroundColor); // 设置默认文本颜色
    setPaper(config.backgroundColor); // 设置编辑器背景色

    // ==== 设置边距 ====
    setupMargins(config);

    // ==== 代码提示 ====
    if(config.autoCompletion)
    {
        // AcsAll: 从所有文档内容中获取补全建议
        setAutoCompletionSource(QsciScintilla::AcsAll);
        setAutoCompletionCaseSensitivity(false); // 不区分大小写
        setAutoCompletionReplaceWord(true);      // 替换当前单词
        setAutoCompletionShowSingle(true);       // 只有一个选项时也显示
        setAutoCompletionThreshold(2);           // 输入2个字符后显示补全
    }
    else
    {
        setAutoCompletionSource(QsciScintilla::AcsNone); // 禁用自动完成
    }

    emit languageChanged(config.language);
}

void CodeEditor::setLanguage(const Language &language)
{
    EditorConfig config = m_config;
    config.language = language;
    setConfig(config);
}

void CodeEditor::setLanguage(const QString &language)
{
    if(language == "C" || language == "C++") setLanguage(Language::CPP);
    else if(language == "Python") setLanguage(Language::Python);
    else if(language == "Java") setLanguage(Language::Java);
    else if(language == "JavaScript") setLanguage(Language::JavaScript);
    else if(language == "Rust") setLanguage(Language::Rust);

}

EditorConfig CodeEditor::getConfig() const
{
    return m_config;
}

void CodeEditor::toggleBreakpoint(int line)
{
    if(markersAtLine(line) & (1 << BREAKPOINT_MARKER_NUM)) markerDelete(line, BREAKPOINT_MARKER_NUM);
    else markerAdd(line, BREAKPOINT_MARKER_NUM);
}

EditorConfig CodeEditor::defaultConfig()
{
    return EditorConfig(); // 默认设置
}

EditorConfig CodeEditor::cppConfig()
{
    return EditorConfig(Language::CPP);
}

EditorConfig CodeEditor::pythonConfig()
{
    return EditorConfig(Language::Python);
}

EditorConfig CodeEditor::javaConfig()
{
    return EditorConfig(Language::Java);
}

EditorConfig CodeEditor::javascriptConfig()
{
    return EditorConfig(Language::JavaScript);
}

EditorConfig CodeEditor::rustConfig()
{
    return EditorConfig(Language::Rust);
}

void CodeEditor::keyPressEvent(QKeyEvent *event)
{
    // 处理回车键自动换行
    if(event->key() == Qt::Key_Return)
    {
        smartNewlineInBrackets(event);
        if(event->isAccepted()) return;
    }

    if(m_config.autoBracketCompletion)
    {
        autoCompleteBracket(event);
    }
    else QsciScintilla::keyPressEvent(event);
}

void CodeEditor::autoCompleteBracket(QKeyEvent *event)
{
    QString text = event->text();
    // 只处理单个字符按钮
    if(text.length() != 1)
    {
        QsciScintilla::keyPressEvent(event);
        return;
    }

    QChar ch = text.at(0);
    int line, index;
    getCursorPosition(&line, &index);
    QString lineText = this->text(line);

    // 处理右括号的跳过逻辑
    if(ch == ')' || ch == ']' || ch == '}' || ch == '"' || ch == '\'')
    {
        // 检查光标右侧是否已经有相同的字符
        if(index < lineText.length() && lineText.at(index) == ch)
        {

            // 如果右侧已经有相同的字符，则跳过插入，只移动光标
            setCursorPosition(line, index + 1);
            event->accept();
            return;
        }
    }

    QsciScintilla::keyPressEvent(event);

    // 处理括号智能补全
    getCursorPosition(&line, &index);
    QString pairChar;
    // 定义需要自动补全的括号对
    if(ch == '(') pairChar = ")";
    else if(ch == '[') pairChar = "]";
    else if(ch == '{') pairChar = "}";
    else if(ch == '"') pairChar = "\"";
    else if(ch == '\'') pairChar = "'";
    else return;


    // 插入配对的括号
    insert(pairChar);
    // 将光标移回两个括号之间
    setCursorPosition(line, index);

    event->accept();
}

void CodeEditor::smartNewlineInBrackets(QKeyEvent *event)
{
    int line, index;
    getCursorPosition(&line, &index);
    QString lineText = this->text(line);

    // 检查是否在括号中间
    if(index > 0 && index < lineText.length())
    {
        QChar leftChar = lineText.at(index - 1);
        QChar rightChar = lineText.at(index);

        // 检查是否在匹配的括号中间（{}）
        if(leftChar == '{' && rightChar == '}')
        {
            // 获取当前行缩进
            QString currentIndent;
            int i = 0;
            while(i < index && lineText[i].isSpace())
            {
                currentIndent += lineText[i++];
            }
            // 构建缩进字符串
            QString indentStr = currentIndent;

            // 增加一级缩进用于新行
            QString newIndentStr;
            if(m_config.useSpacesForTabs) newIndentStr = indentStr + QString(m_config.tabWidth, ' ');
            else newIndentStr = indentStr + '\t';

            // 获取光标后的内容（包括 } 及后面的文本）
            QString remainingText = lineText.mid(index);

            // 开始撤销操作（使整个操作可以一次性撤销）
            beginUndoAction();

            // 构建新的文本内容
            QString newText =
                "\n" + newIndentStr +             // 第一行：换行和缩进
                "\n" + indentStr + remainingText; // 第二行：换行、原缩进、右括号

            // 删除光标后的所有内容
            setSelection(line, index, line, lineText.length() - 1);
            removeSelectedText();

            insert(newText);

            // 将光标定位到中间行
            setCursorPosition(line + 1, newIndentStr.length());

            endUndoAction();

            event->accept();
            return;
        }
    }
    event->ignore();
}

void CodeEditor::setupMargins(const EditorConfig &config)
{
    setMarginsBackgroundColor(config.marginBackgroundColor); // 边距背景色
    setMarginsForegroundColor(config.marginForegroundColor); // 边距文字颜色

    // 边距0：断点/书签边距
    // === 断点标记设置 ===
    // 定义断点标记（红色圆形）
    setMarginType(0, QsciScintilla::SymbolMargin);
    setMarginWidth(0, 15);  // 较窄的边距用于断点
    setMarginSensitivity(0, true);
    markerDefine(QsciScintilla::Circle, BREAKPOINT_MARKER_NUM);
    setMarkerBackgroundColor(QColor(255, 0, 0), BREAKPOINT_MARKER_NUM);  // 红色背景
    setMarkerForegroundColor(QColor(255, 0, 0), BREAKPOINT_MARKER_NUM);          // 白色前景

    // 设置边距2只显示断点和书签标记
    setMarginMarkerMask(0, (1 << BREAKPOINT_MARKER_NUM));

    // 边距1：行号边距
    if(config.lineNumbers)
    {
        // 边距0：专门用于显示行号
        setMarginType(1, QsciScintilla::NumberMargin);  // 边距0用于显示行号
        setMarginLineNumbers(1, true);                  // 启用行号显示
        setMarginWidth(1, "0000");                      // 设置宽度，能容纳4位行号
        setMarginSensitivity(1, false);
        setMarginMarkerMask(1, 0);
    }
    else setMarginWidth(1, 0);

    // 边距2：折叠标记边距
    if(config.codeFolding)
    {
        // 边距1：用于代码折叠标记
        setMarginType(2, QsciScintilla::SymbolMargin);  // 设置为符号边距
        setMarginWidth(2, 16);                          // 固定宽度16像素
        setMarginSensitivity(2, true);                  // 允许点击折叠/展开
    }
    else setMarginWidth(2, 0);

    // 边距3：保留或禁用
    setMarginWidth(3, 0);
    setMarginWidth(4, 0);
}

void CodeEditor::setupLexer(const Language &language)
{
    setLexer(nullptr);
    m_lexer = nullptr;

    if(m_languageSpec)
    {
        delete m_languageSpec;
        m_languageSpec = nullptr;
    }

    // 创建新的语言规范
    m_languageSpec = LanguageSpecFactory::createSpec(language);
    if (!m_languageSpec) return;

    // 创建词法分析器
    QsciLexer *newLexer = m_languageSpec->createLexer();
    if(!newLexer) return;

    // 应用字体和语法高亮
    newLexer->setFont(m_config.font);
    m_languageSpec->applySyntaxHighlighting(newLexer, m_config.syntaxHighlighting);

    setLexer(newLexer);
    m_lexer = newLexer;

    setupAPIs();
}

void CodeEditor::setupAPIs()
{
    if(!m_lexer || !m_languageSpec) return;

    QsciAPIs *apis = new QsciAPIs(m_lexer);
    m_languageSpec->setupAPIs(apis);
    apis->prepare();
    m_lexer->setAPIs(apis);
}
