 #ifndef FRAMELESSWIDGETBASE_H
#define FRAMELESSWIDGETBASE_H

#include <QWidget>
#include <QTimer>

#include <windows.h>
/*****************************************************
*
* @file     framelesswindowbase.h
* @brief    无边框窗口基类
*
* @description
*           ==== 布局 ====
*           1. 底部透明窗口（比显示窗口略大，用于实现阴影），其上使用QFrame作为实际显示窗口
*           2. 显示窗口布局（垂直布局）
*               1. 标题栏
*               2. 中心部件
*               3. 状态栏
*           ==== 核心功能 ====
*           1. 无边框
*           2. 窗口阴影
*           3. 标题栏双击放大、标题栏拖动最大化、标题栏拖动还原
*           ==== 使用说明 ====
*           1. 使用时可以通过centralWidget对窗口布局
*           2. 重载实现了部分函数接口
*           ==== 注意 ====
*           Windows的Aero Snap没有实现，简单实现了桌面顶部停靠全屏
*
* @author   无声目
* @date     2025/08/18
* @history
*****************************************************/
class TitleBar;
class QFrame;
class QPushButton;
class QGraphicsDropShadowEffect;
class FrameLessWindowBase : public QWidget
{
    Q_OBJECT
public:
    enum FrameLessTheme {
        Light,
        Dark
    };
    Q_ENUM(FrameLessTheme)
    using FTheme = FrameLessTheme;

    explicit FrameLessWindowBase(QWidget *parent = nullptr);

    void setTheme(FTheme theme);
    FTheme currentTheme() const;

    // ==== 重定义窗口相关代码 ====
    // 窗口大小相关
    void resize(int w, int h); // 是整个Frame的大小
    int width() const; // centralWidget的宽
    int height() const; // centralWidget的高
    // 标题栏相关
    void setWindowTitle(const QString& ); // 设置标题
    void setWindowIcon(const QString& path); // 设置图标
    void setWindowIcon(const QIcon& icon); // 设置图标
    void setTitleBarColor(const QColor& color); // 设置颜色
    // centralWidget
    QWidget* getCentralWidget();
    void setLayout(QLayout *layout);
    QLayout *layout() const;
    // 状态栏相关
    void setStatusBarHide(bool hidden);

    void setPinButton(const QString &checkedPath, const QString &uncheckedPath); // 设置"置顶按钮"
    void setPinButtonChecked(bool );

    QPushButton *getSetButton();

signals:
    // 窗口激活状态改变
    void windowActivationChanged(bool active);

    void sig_close(); // 窗口关闭信号

protected:
    // 处理 Windows 原生消息事件
    bool nativeEvent(const QByteArray &eventType, void *message, long *result) override;
    // 绘制事件
//    void paintEvent(QPaintEvent *event) override;

private:
    // 初始化UI
    void initFrameLessWindowBaseUI();
    // 创建顶部窗口
    QFrame* createTopWidget();
    // 消息处理函数
    bool handleNcHitTest(MSG* msg, long* result);
    bool handleMinMaxInfo(MSG* msg, long* result);
    void handleActivate(MSG* msg);


    void updateCentralWidgetStyle();
    void updateStatusBarStyle();
    void connectThemeSignals();

    FTheme m_currentTheme = Light;
    QColor m_bgColor;
    QColor m_widgetColor;
    QColor m_borderColor;

    // 窗口拉伸
    int m_innerBorderWidth = 2; // 鼠标可拉伸的内边框宽度
    int m_outlineBorderWidth = 10; // 鼠标可拉伸的外边框宽度
    // 窗口外边距
    const int m_marginsWidth = 20;

    // 无边框窗口
    QFrame* m_topWidget;
    TitleBar* m_titleBar;// 标题栏
    QWidget* m_centralWidget;
    QWidget* m_statusBar; // 试验用，最后改成自定义类

    bool m_statusBarHide = false;
    // 窗口最大化还原
    bool m_isMaximized = false;
};

#endif // FRAMELESSWIDGETBASE_H
