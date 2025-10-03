#include "framelesswindowbase.h"

#include "titlebar.h"
#include <windowsx.h>

#include <QFrame>
#include <QGraphicsDropShadowEffect>
#include <QPainter>
#include <QVBoxLayout>
#include <QDebug>

FrameLessWindowBase::FrameLessWindowBase(QWidget *parent) : QWidget(parent)
{
    // 设置无边框相关属性
    setWindowFlags(Qt::FramelessWindowHint | Qt::WindowMinMaxButtonsHint | Qt::Window);
    setAttribute(Qt::WA_TranslucentBackground);
    setAttribute(Qt::WA_StyleSheet, false);
    setAttribute(Qt::WA_StyledBackground);// 禁止父窗口影响子窗口样式

    // 初始化界面
    initFrameLessWindowBaseUI();
    qInfo() << "Frameless window initialized successfully";
}

void FrameLessWindowBase::setTheme(FTheme theme)
{
    m_currentTheme = theme;

    // 设置颜色
    if(theme == Dark)
    {
        m_bgColor = QColor(45, 45, 48);     // #2d2d30
        m_widgetColor = QColor(37, 37, 38); // #252526
        m_borderColor = QColor(62, 62, 66); // #3e3e42
    }
    else
    {
        m_bgColor = Qt::white;
        m_widgetColor = QColor(253, 253, 253); // #fdfdfd
        m_borderColor = QColor(224, 224, 224); // #e0e0e0
    }

    // 更新标题栏
    m_titleBar->setTheme(theme == FTheme::Dark ? TitleBar::TTheme::Dark : TitleBar::TTheme::Light);
    // 更新中央部件样式
    updateCentralWidgetStyle();
    // 更新状态栏样式
    updateStatusBarStyle();

    // 连接主题相关信号
    connectThemeSignals();
}

FrameLessWindowBase::FTheme FrameLessWindowBase::currentTheme() const
{
    return m_currentTheme;
}

// 窗口大小相关
void FrameLessWindowBase::resize(int w, int h)
{ QWidget::resize(QSize(w + 2 * m_marginsWidth, h + 2 * m_marginsWidth)); }

int FrameLessWindowBase::width() const { return m_centralWidget->width(); }
int FrameLessWindowBase::height() const { return m_centralWidget->height(); }

// 标题栏相关
void FrameLessWindowBase::setWindowTitle(const QString &name) { m_titleBar->setWindowTitle(name); }

void FrameLessWindowBase::setWindowIcon(const QString &path)
{
    m_titleBar->setWindowIcon(path);
    QWidget::setWindowIcon(QIcon(path));
}

void FrameLessWindowBase::setWindowIcon(const QIcon &icon)
{
    m_titleBar->setWindowIcon(icon);
    QWidget::setWindowIcon(icon);
}

void FrameLessWindowBase::setTitleBarColor(const QColor& color) { m_titleBar->setTitleBarColor(color); }

// centralWidget
QWidget *FrameLessWindowBase::getCentralWidget() { return m_centralWidget; }

void FrameLessWindowBase::setLayout(QLayout *layout)
{
    QLayout *oldLayout = m_centralWidget->layout();
    if(oldLayout)
    {
        oldLayout->setEnabled(false);
        delete oldLayout;
    }
    m_centralWidget->setLayout(layout);
}

QLayout *FrameLessWindowBase::layout() const { return m_centralWidget->layout(); }

void FrameLessWindowBase::setStatusBarHide(bool hidden)
{
    if(hidden)
    {
        m_statusBar->hide();
        m_statusBarHide = true;
    }
    else
    {
        m_statusBar->show();
        m_statusBarHide = false;
    }
    updateCentralWidgetStyle();
}

void FrameLessWindowBase::setPinButton(const QString &checkedPath, const QString &uncheckedPath)
{
    m_titleBar->setPinButton(checkedPath, uncheckedPath);
}

void FrameLessWindowBase::setPinButtonChecked(bool checked)
{
    m_titleBar->setPinButtonChecked(checked);
}

QPushButton *FrameLessWindowBase::getSetButton()
{
    return m_titleBar->getSetButton();
}

/******************************
* 处理 Windows 原生消息事件
*
* @param eventType 事件类型标识符（Windows 平台通常为 "windows_generic_MSG"）
* @param message   指向原生消息的指针（在 Windows 平台为 MSG 结构体指针）
* @param result    用于存储处理结果的指针（返回值给 Windows 系统）
* @return bool     返回 true 表示已处理该消息，false 表示未处理
*******************************/
bool FrameLessWindowBase::nativeEvent(const QByteArray &eventType, void *message, long *result)
{
    // 将通用消息指针转换为 Windows MSG 结构体指针
    MSG *msg = static_cast<MSG*>(message);
    if(!msg) return QWidget::nativeEvent(eventType, message, result);
    // 根据消息类型进行分发处理
    switch(msg->message)
    {
    case WM_NCHITTEST: // 用于处理非显示区域的鼠标点击，可以控制窗口的响应行为
        return handleNcHitTest(msg, result);
    case WM_GETMINMAXINFO: // 窗口最小/最大尺寸限制
        return handleMinMaxInfo(msg, result);
    case WM_ACTIVATE: // 窗口激活状态变化
        handleActivate(msg);
        break;
    default:
        break;
    }
    return QWidget::nativeEvent(eventType, message, result);
}

void FrameLessWindowBase::initFrameLessWindowBaseUI()
{
    // ==== 实际显示窗口 ====
    QVBoxLayout *mainVLayout = new QVBoxLayout(this);
    m_topWidget = createTopWidget();

    mainVLayout->addWidget(m_topWidget);
    mainVLayout->setContentsMargins(m_marginsWidth, m_marginsWidth, m_marginsWidth, m_marginsWidth);

    QWidget::setLayout(mainVLayout);

    // 窗口阴影
    QGraphicsDropShadowEffect *shadowEffect = new QGraphicsDropShadowEffect(m_topWidget);
    shadowEffect->setOffset(0, 0);
    shadowEffect->setColor(QColor(0, 0, 0, 90));// 阴影颜色
    shadowEffect->setBlurRadius(m_marginsWidth);// 阴影半径
    m_topWidget->setGraphicsEffect(shadowEffect); // 这句话必须有
    QObject::connect(m_titleBar, &TitleBar::sig_close, [=](){
        close();
        emit sig_close();
    });
    // 连接焦点变化信号
    QObject::connect(this, &FrameLessWindowBase::windowActivationChanged, [=](bool active){
        m_titleBar->ActivationChanged(active);
        if(!m_isMaximized)
            shadowEffect->setColor(active ? QColor(63,63,63,90) : QColor(63,63,63,30));
    });
    setTheme(m_currentTheme);
}

QFrame *FrameLessWindowBase::createTopWidget()
{
    QFrame *topWidget = new QFrame;
//    topWidget->setFrameShape(QFrame::NoFrame);
    m_titleBar = new TitleBar(topWidget);
    m_titleBar->setMarginsWidth(m_marginsWidth);
    m_centralWidget = new QWidget(topWidget);
    m_statusBar = new QWidget(topWidget);

    m_statusBar->setFixedHeight(25);
    m_statusBar->setObjectName("statusBar");
    updateStatusBarStyle();

    m_centralWidget->setObjectName("centralWidget");
    updateCentralWidgetStyle();
    QVBoxLayout *vLayout = new QVBoxLayout(topWidget);
    vLayout->setContentsMargins(0, 0, 0, 0);
    vLayout->setSpacing(0);
    vLayout->addWidget(m_titleBar);
    vLayout->addWidget(m_centralWidget, 1);
    vLayout->addWidget(m_statusBar);
    topWidget->resize(800, 600);

    return topWidget;
}

/******************************
* 用于处理非显示区域的鼠标点击，可以控制窗口的响应行为（WM_NCHITTEST）
*
* @param msg     Windows 消息结构体指针
* @param result  输出参数，存储命中测试结果
* @return bool   是否已处理该消息
*******************************/
bool FrameLessWindowBase::handleNcHitTest(MSG *msg, long *result)
{
    // 最大化状态下不处理边框拖拽，交给系统默认处理
    if(m_isMaximized)
        return QWidget::nativeEvent("windows_generic_MSG", msg, result);

    // 从消息参数中提取鼠标屏幕坐标
    // GET_X_LPARAM: 从 lParam 中提取 X 坐标
    // GET_Y_LPARAM: 从 lParam 中提取 Y 坐标
    const POINT cursorPos = {GET_X_LPARAM(msg->lParam), GET_Y_LPARAM(msg->lParam)};

    // 获取窗口在屏幕上的矩形区域
    RECT winRect;
    HWND hWnd = reinterpret_cast<HWND>(winId()); // 获取窗口句柄
    if(!GetWindowRect(hWnd, &winRect)) return false; // 获取窗口矩形失败

    // 计算鼠标相对于窗口左上角的坐标  - 2 * m_shadow_width
    const int width = winRect.right - winRect.left - 2 * m_marginsWidth; // 窗口宽度
    const int height = winRect.bottom - winRect.top - 2 * m_marginsWidth; // 窗口高度
    const int x = cursorPos.x - winRect.left - m_marginsWidth;           // 窗口内 X 坐标
    const int y = cursorPos.y - winRect.top - m_marginsWidth;            // 窗口内 Y 坐标

    // 计算边界检测参考线（避免重复计算）
    const int rightEdge = width - m_innerBorderWidth;
    const int bottomEdge = height - m_innerBorderWidth;

    // ==== 检测鼠标所在窗口区域并返回对应区域标识  ====
    // 1. 优先检测四个角落区域
    if(x < m_innerBorderWidth && y < m_innerBorderWidth)
        *result = HTTOPLEFT; // 左上角
    else if(x >= rightEdge && y < m_innerBorderWidth)
        *result = HTTOPRIGHT; // 右上角
    else if(x < m_innerBorderWidth && y >= bottomEdge)
        *result = HTBOTTOMLEFT; // 左下角
    else if(x >= rightEdge && y >= bottomEdge)
        *result = HTBOTTOMRIGHT; // 右下角

    // 2. 检测四条边界
    else if(x < m_innerBorderWidth && x > -m_outlineBorderWidth)
        *result = HTLEFT; // 左侧边框
    else if(x >= rightEdge && x < width + m_outlineBorderWidth)
        *result = HTRIGHT; // 右侧边框
    else if(y < m_innerBorderWidth + 5 && y > -m_outlineBorderWidth + 5)
        *result = HTTOP; // 顶部边框
    else if(y >= bottomEdge && y < height + m_outlineBorderWidth)
        *result = HTBOTTOM; // 底部边框
    else // 非边框区域交给基类处理（可能用于标题栏拖拽等）
        return QWidget::nativeEvent("windows_generic_MSG", msg, result);

    return true;  // 已处理该消息
}

/******************************
* 处理窗口最小/最大尺寸限制（WM_GETMINMAXINFO）
*
* @param msg     Windows 消息结构体指针
* @param result  输出参数，设置返回结果（通常为0）
* @return bool   总是返回 true 表示已处理
*******************************/
bool FrameLessWindowBase::handleMinMaxInfo(MSG *msg, long *result)
{
    // 转换消息参数为 MINMAXINFO 结构体指针
    MINMAXINFO* mmi = reinterpret_cast<MINMAXINFO*>(msg->lParam);
    // 设置窗口最小跟踪尺寸（用户不能将窗口缩小至此尺寸以下）
    mmi->ptMinTrackSize.x = minimumWidth(); // 最小宽度
    mmi->ptMinTrackSize.y = minimumHeight(); // 最小高度
    *result = 0; // 必须设置为0
    return true;  // 已处理该消息
}

/******************************
* 处理窗口激活状态变化（WM_ACTIVATE）
*
* @param msg Windows 消息结构体指针
*******************************/
void FrameLessWindowBase::handleActivate(MSG *msg)
{
    // LOWORD(wParam): 获取激活状态的低位字
    // WA_INACTIVE (0): 窗口变为非活动状态
    // 其他值: 窗口变为活动状态
    const bool active = (LOWORD(msg->wParam) != WA_INACTIVE);
    // 发射窗口激活状态变化信号
    emit windowActivationChanged(active);
}

void FrameLessWindowBase::updateCentralWidgetStyle()
{
    m_centralWidget->setStyleSheet(QString(R"(
                                           #centralWidget {
                                               background: %1;
                                               border-bottom-left-radius:%2px;
                                               border-bottom-right-radius:%2px;
                                               border: 1px solid %3;
                                               border-top: none;
                                           })")
                                   .arg(m_widgetColor.name())
                                   .arg(m_statusBarHide ? 10 : 0)
                                   .arg(m_borderColor.name()));
}

void FrameLessWindowBase::updateStatusBarStyle()
{
    m_statusBar->setStyleSheet(QString(R"(
                                       #statusBar {
                                           background: %1;
                                           border-bottom-left-radius:10px;
                                           border-bottom-right-radius:10px;
                                           border: 1px solid %2;
                                           border-top: none;
                                       })").arg(m_widgetColor.name()).arg(m_borderColor.name()));
}

void FrameLessWindowBase::connectThemeSignals()
{
    // 断开之前的连接
    disconnect(m_titleBar, &TitleBar::sig_max, nullptr, nullptr);

    // 根据当前主题连接信号
    QObject::connect(m_titleBar, &TitleBar::sig_max, [=](int isMax){
        m_isMaximized = isMax;

        // 更新阴影效果
        QGraphicsDropShadowEffect* shadow =
            qobject_cast<QGraphicsDropShadowEffect*>(m_topWidget->graphicsEffect());
        shadow->setColor(isMax ? QColor(0, 0, 0, 0) : QColor(0, 0, 0, 90));

        // 更新边框样式
        QString borderStyle = isMax ? "transparent" : m_borderColor.name();

        QString statusBarStyle = QString(R"(
                                         #statusBar {
                                             background: %1;
                                             border-bottom-left-radius:%2px;
                                             border-bottom-right-radius:%2px;
                                             border: 1px solid %3;
                                             border-top: none;
                                         })").arg(m_widgetColor.name()).arg(isMax ? 0 : 10).arg(borderStyle);

        QString centralWidgetStyle = QString(R"(
                                             #centralWidget {
                                                 background: %1;
                                                 border-bottom-left-radius:%2px;
                                                 border-bottom-right-radius:%2px;
                                                 border: 1px solid %3;
                                                 border-top: none;
                                             })")
                .arg(m_widgetColor.name())
                .arg(isMax ? 0 : 10)
                .arg(borderStyle);

        if(!m_statusBarHide) m_statusBar->setStyleSheet(statusBarStyle);
        else m_centralWidget->setStyleSheet(centralWidgetStyle);
    });
}
