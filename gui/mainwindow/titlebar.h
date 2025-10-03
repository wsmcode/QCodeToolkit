#ifndef TITLEBAR_H
#define TITLEBAR_H

#include <QWidget>
#include <QIcon>
/*****************************************************
*
* @file     titlebar.h
* @brief    标题栏类
*
* @description
*           ==== 布局 ====
*           ==== 核心功能 ====
*           ==== 使用说明 ====
*           ==== 注意 ====
*           当前把各种标题栏写一起了（带设置的、带置顶的）
*
* @author   无声目
* @date     2025/08/31
* @history
*****************************************************/

class QLabel;
class QPushButton;
class QPropertyAnimation;
class TitleBar : public QWidget
{
    Q_OBJECT
public:
    enum TitleBarTheme {
        Light,
        Dark
    };
    Q_ENUM(TitleBarTheme)
    using TTheme = TitleBarTheme;

    explicit TitleBar(QWidget *parent = nullptr);
    ~TitleBar();

    void setTheme(TTheme theme);
    TTheme currentTheme() const;

    // ==== 窗口状态变化 ====
    void ActivationChanged(bool active);
    void setMarginsWidth(int width);
    // ==== 无边框对外接口 ====
    void setWindowTitle(const QString &);
    void setWindowIcon(const QString &);
    void setWindowIcon(const QIcon &icon);
    void setTitleBarColor(const QColor& color);
    void setPinButton(const QString &checkedPath, const QString &uncheckedPath); // 设置"置顶按钮"
    void setPinButtonChecked(bool );

    QPushButton *getSetButton();
signals:
    void sig_close(); // 窗口关闭信号
    void sig_max(bool isMax); // 窗口最大化和还原信号

private slots:
    void onClicked();

protected:
    QSize sizeHint() const override;
    // 鼠标事件
    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;
    void mouseDoubleClickEvent(QMouseEvent *event) override;

private:
    // 初始化UI
    void initUI();
    // 创建全屏预览透明玻璃窗口
    void createDockingPreview();
    // svg图片渲染，颜色目前不支持透明通道
    QIcon renderSvgIcon(const QString& path, const QSize& size, const QColor& color = Qt::black);

    // 窗口还原
    void showNormal();
    // 窗口最大化
    void showMaximized();

    void setIcons();
    void updateIcons();
    void updateStyle();

    TTheme m_currentTheme = Light;
    QColor m_activeColor;
    QColor m_inactiveColor;
    QColor m_titleBarColor;

    // UI相关控件
    QLabel *m_logoLabel;        // 标题图标
    QLabel *m_titleTextLabel;   // 标题
    QPushButton *m_SetPBtn;     // 设置按钮
    QPushButton *m_MinPBtn;     // 最小化按钮
    QPushButton *m_MaxPBtn;     // 最大化和还原按钮
    QPushButton *m_closePBtn;   // 关闭按钮

    // ==== 图标缓存 ====
    // 正常图标
    QIcon m_minIcon;
    QIcon m_maxIcon;
    QIcon m_restoreIcon;
    QIcon m_closeIcon;
    QIcon m_setIcon;
    // 窗口未激活
    QIcon m_minIconInactive;
    QIcon m_maxIconInactive;
    QIcon m_restoreIconInactive;
    QIcon m_closeIconInactive;
    QIcon m_setIconInactive;

    QIcon m_setIconChecked;

    // 窗口拖动
    QPoint offset_pos; // 鼠标相对窗口位移
    // 窗口最大化时拖动
    QPoint m_dragStartPos;       // 记录拖动开始时的位置
    bool m_mouseRelease = true;  // 鼠标是否释放
    // 用于窗口还原
    QPoint m_windowPos; // 窗口最大化前位置记录
    // 全局UI属性设置
    int m_fixedHeight = 40;                 // 标题栏高度
    int m_pBtnfixedWidth = 50;              // 按钮宽度
    // 窗口外边距
    int m_marginsWidth = 20;
    // Snap Assist透明玻璃预览窗口（拖动到桌面顶部全屏）
    bool m_dockingPreview = false;                      // 标记是否正在显示停靠预览
    QWidget* m_previewWidget = nullptr;                 // 预览窗口
    QPropertyAnimation* m_previewAnimation = nullptr;   // 预览动画

    // 窗口最大化和还原
    bool m_isMaximized = false; // 最大化状态
    QRect m_normalSize;
};

#endif // TITLEBAR_H
