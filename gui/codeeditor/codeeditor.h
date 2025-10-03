#ifndef CODEEDITOR_H
#define CODEEDITOR_H

//QsciScintilla作为QWidget的控件，需要添加该控件的头文件
#include <Qsci/qsciscintilla.h>
#include <editor_config.h>

class LanguageSpec;
class CodeEditor : public QsciScintilla
{
    Q_OBJECT
public:
    explicit CodeEditor(QWidget *parent = nullptr);
    ~CodeEditor();

    // 设置编辑器配置
    void setConfig(const EditorConfig &config);

    // 快速设置语言
    void setLanguage(const Language &language);
    void setLanguage(const QString &language);

    // 获取当前配置
    EditorConfig getConfig() const;

    // 断点和书签
    void toggleBreakpoint(int line);
    // 预设配置
    static EditorConfig defaultConfig();
    static EditorConfig cppConfig();
    static EditorConfig pythonConfig();
    static EditorConfig javaConfig();
    static EditorConfig javascriptConfig();
    static EditorConfig rustConfig();

    // 根据文件扩展名检测语言
//    static Language detectLanguageFromExtension(const QString& filePath);

signals:
    void languageChanged(Language newLanguage);

protected:
    void keyPressEvent(QKeyEvent *event) override;

private:
    void setupMargins(const EditorConfig &config);
    void setupLexer(const Language &language);
    void setupAPIs();

    // 自动括号补全
    void autoCompleteBracket(QKeyEvent *event);
    void smartNewlineInBrackets(QKeyEvent *event);

    EditorConfig m_config;
    QsciLexer *m_lexer = nullptr;
    LanguageSpec* m_languageSpec = nullptr;

    // 断点标记编号
    static const int BREAKPOINT_MARKER_NUM = 10;
};

#endif // CODEEDITOR_H
