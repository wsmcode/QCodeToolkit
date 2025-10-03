#ifndef STYLEMANAGER_H
#define STYLEMANAGER_H

#include <QObject>
#include <QMap>
#include <QColor>
#include <QPointer>

enum class Theme {
    LightTheme,
    DarkTheme
};
using StyleTheme = Theme;

class CodeEditor;
class StyleManager : public QObject
{
    Q_OBJECT
public:
    // 单例模式
    static StyleManager* getStyleManager()
    {
        static StyleManager s;
        return &s;
    }
    // 删除拷贝构造函数和赋值运算符
    StyleManager(const StyleManager&) = delete;
    StyleManager& operator=(const StyleManager&) = delete;

    void setTheme(Theme theme);// 设置当前主题
    Theme currentTheme() const; // 获取当前主题

    // 注册控件
    void registerWidget(QWidget *widget);
    void unregisterWidget(QWidget *widget);

    void setThemeFile(Theme theme, const QString &filePath);
    QString themeFile(Theme theme) const;

    // 设置编辑器样式文件
    void setEditorStyleFile(const QString &filePath);
    QString editorStyleFile() const;

    bool loadTheme(Theme theme);
    bool loadEditorStyles(); // 加载编辑器样式

    // 应用样式到所有已注册的控件
    void applyStyle();

    // 获取编辑器颜色配置
    QMap<QString, QColor> getEditorColors(Theme theme) const;

signals:
    void themeChanged(Theme newTheme);

private:
    explicit StyleManager(QObject *parent = nullptr);

    // 解析QSS内容，分离全局样式和特定对象样式
    void parseQssContent(Theme theme, const QString& content);
    void clearThemeData(Theme theme);

    // 解析编辑器样式文件
    bool parseEditorStyleFile(const QString &filePath);
    // 应用编辑器样式
    void applyEditorStyle(CodeEditor *editor);

    Theme m_currentTheme = Theme::LightTheme;
    QList<QPointer<QWidget>> m_registeredWidgets; // 使用 QPointer 自动检测有效性
    QMap<Theme, QString> m_themeFiles;
    QMap<Theme, QString> m_themeContent;
    QMap<Theme, QString> m_themeGlobalStyles;
    QMap<Theme, QMap<QString, QString>> m_themeWidgetStyles;

    QString m_editorStyleFile;
    QMap<Theme, QMap<QString, QColor>> m_editorColors;
};

#endif // STYLEMANAGER_H
