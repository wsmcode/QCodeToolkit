#include "titlebar.h"

#include <QFile>
#include <QHBoxLayout>
#include <QLabel>
#include <QPainter>
#include <QScreen>
#include <QWindow>
#include <QPropertyAnimation>
#include <QPushButton>
#include <QSvgRenderer>
#include <QDebug>
#include <QMouseEvent>
#include <QStyle>
#include <QApplication>

TitleBar::TitleBar(QWidget *parent) : QWidget(parent)
{
    setFixedHeight(m_fixedHeight);
    setAttribute(Qt::WA_StyleSheet, false);
    m_windowPos = window()->pos(); // 当前窗口在当前屏幕的位置

    m_normalSize = normalGeometry();

    setIcons();
    initUI();
    createDockingPreview();
    qInfo() << "Title bar initialized successfully";

}

TitleBar::~TitleBar()
{
    if(m_previewWidget)
    {
        delete m_previewWidget;
        m_previewWidget = nullptr;
    }
}

void TitleBar::setTheme(TTheme theme)
{
    m_currentTheme = theme;

    // 设置颜色
    if(theme == Dark)
    {
        m_titleBarColor = QColor(45, 45, 48);        // #2d2d30
        m_activeColor = QColor(240, 240, 240);   // #f0f0f0
        m_inactiveColor = QColor(170, 170, 170); // #aaaaaa
    }
    else
    {
        m_titleBarColor = "#f5f5f5";
        m_activeColor = Qt::black;
        m_inactiveColor = QColor(153, 153, 153); // #999999
    }
    // 更新图标
    updateIcons();
    // 更新样式
    updateStyle();
}

TitleBar::TTheme TitleBar::currentTheme() const
{
    return m_currentTheme;
}
// 初始化UI
void TitleBar::initUI()
{
    // 设置样式
    setAttribute(Qt::WA_StyledBackground);// 禁止父窗口影响子窗口样式
    setObjectName("titleBar");

    // 标题图标
    m_logoLabel = new QLabel(this);
    m_logoLabel->setFixedSize(m_fixedHeight, m_fixedHeight);
    // 标题
    m_titleTextLabel = new QLabel(this);
    m_titleTextLabel->setText("标题");
    m_titleTextLabel->setProperty("activation", 1);
    m_titleTextLabel->setObjectName("titleText");
    m_titleTextLabel->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
    // 设置按钮
    m_SetPBtn = new QPushButton(this);
    m_SetPBtn->setIcon(m_setIcon);
    m_SetPBtn->setFixedSize(m_pBtnfixedWidth, m_fixedHeight);
    m_SetPBtn->setStyleSheet(R"(QPushButton {border:none;}
                             QPushButton:hover { background: grey; })");
    // 最小化按钮
    m_MinPBtn = new QPushButton(this);
    m_MinPBtn->setIcon(m_minIcon);
    m_MinPBtn->setFixedSize(m_pBtnfixedWidth, m_fixedHeight);
    m_MinPBtn->setStyleSheet(R"(QPushButton {border:none;}
                             QPushButton:hover { background: grey; })");
    // 最大化和还原按钮
    m_MaxPBtn = new QPushButton(this);
    m_MaxPBtn->setIcon(m_maxIcon);
    m_MaxPBtn->setFixedSize(m_pBtnfixedWidth, m_fixedHeight);
    m_MaxPBtn->setStyleSheet(R"(QPushButton {border:none;}
                             QPushButton:hover { background: grey; })");
    // 关闭按钮
    m_closePBtn = new QPushButton(this);
    m_closePBtn->setIcon(m_closeIcon);
    m_closePBtn->setFixedSize(m_pBtnfixedWidth, m_fixedHeight);
    m_closePBtn->setProperty("windowState", m_isMaximized ? 1 : 0);
    m_closePBtn->setObjectName("closepbtn");

    // 添加布局
    QHBoxLayout* hLayout = new QHBoxLayout(this);
    hLayout->addWidget(m_logoLabel);
    hLayout->addWidget(m_titleTextLabel);
    hLayout->addStretch();
    hLayout->addWidget(m_SetPBtn);
    hLayout->addWidget(m_MinPBtn);
    hLayout->addWidget(m_MaxPBtn);
    hLayout->addWidget(m_closePBtn);

    hLayout->setContentsMargins(0, 0, 0, 0);
    hLayout->setSpacing(0);

    QObject::connect(m_MinPBtn, &QPushButton::clicked, this, &TitleBar::onClicked);
    QObject::connect(m_MaxPBtn, &QPushButton::clicked, this, &TitleBar::onClicked);
    QObject::connect(m_closePBtn, &QPushButton::clicked, this, &TitleBar::onClicked);
    setTheme(m_currentTheme);
}

// 创建全屏预览透明玻璃窗口
void TitleBar::createDockingPreview()
{
    // 创建全屏预览透明玻璃窗口
    m_previewWidget = new QWidget; // 不要设置父对象
    m_previewWidget->setWindowFlags(Qt::FramelessWindowHint | Qt::Tool);
    m_previewWidget->setAttribute(Qt::WA_TranslucentBackground);
    QHBoxLayout *layout = new QHBoxLayout(m_previewWidget);
    layout->setContentsMargins(0, 0, 0, 0);
    QWidget *showWidget = new QWidget;
    layout->addWidget(showWidget);
    showWidget->setObjectName("showWidget");
    showWidget->setStyleSheet(R"(#showWidget {
                                  background-color: rgba(240, 240, 240, 30);
                                  border: 1px solid rgba(180, 180, 180, 200);
                                  border-radius: 8px;
                              })");
    m_previewWidget->hide();
    // 创建动画
    m_previewAnimation = new QPropertyAnimation(m_previewWidget, "geometry");
    m_previewAnimation->setDuration(200);
    m_previewAnimation->setEasingCurve(QEasingCurve::OutQuad);
}

// svg图片渲染，颜色目前不支持透明通道
QIcon TitleBar::renderSvgIcon(const QString &path, const QSize &size, const QColor &color)
{
    // 获取设备像素比
    const qreal dpr = devicePixelRatioF();

    // 创建目标尺寸的pixmap(考虑DPI缩放)
    QPixmap pixmap(size * dpr);
    pixmap.fill(Qt::transparent);

    // 读取 SVG 文件内容
    QFile file(path);
    if(!file.open(QIODevice::ReadOnly))
        return QIcon();
    QString svgData = file.readAll();
    file.close();
    // 替换 currentColor 为实际颜色值
    svgData.replace("currentColor", color.name(QColor::HexRgb));

    // SVG渲染
    QSvgRenderer renderer(svgData.toUtf8());
    if(renderer.isValid())
    {
        QPainter painter(&pixmap);
        //painter.setRenderHint(QPainter::Antialiasing);
        // 保持比例居中渲染
        QRectF targetRect(0, 0, size.width() * dpr, size.height() * dpr);
        renderer.render(&painter, targetRect);
        painter.end();
    }
    pixmap.setDevicePixelRatio(dpr);
    return QIcon(pixmap);
}

// 窗口还原
void TitleBar::showNormal()
{
    if(m_isMaximized)
    {
        m_isMaximized = false;
        window()->setGeometry(m_normalSize);
        setProperty("windowState", m_isMaximized ? 1 : 0);
        m_closePBtn->setProperty("windowState", m_isMaximized ? 1 : 0);
        m_closePBtn->style()->unpolish(m_closePBtn);
        m_closePBtn->style()->polish(m_closePBtn);
        style()->unpolish(this);
        style()->polish(this);
    }
}

// 窗口最大化
void TitleBar::showMaximized()
{
    if(!m_isMaximized)
    {
        m_isMaximized = true;
        m_normalSize = window()->normalGeometry();
        QRect maximizedScreen = window()->windowHandle()->screen()->availableGeometry();
        window()->setGeometry(maximizedScreen.adjusted(-m_marginsWidth, -m_marginsWidth,
                                                       m_marginsWidth, m_marginsWidth));
        setProperty("windowState", m_isMaximized ? 1 : 0);
        m_closePBtn->setProperty("windowState", m_isMaximized ? 1 : 0);
        m_closePBtn->style()->unpolish(m_closePBtn);
        m_closePBtn->style()->polish(m_closePBtn);
        style()->unpolish(this);
        style()->polish(this);
    }
}

void TitleBar::setIcons()
{
    if(m_currentTheme == Dark)
    {
        // 暗色主题使用浅色图标
        m_minIcon = renderSvgIcon(":/res/icon/min.svg", QSize(16, 16), Qt::white);
        m_maxIcon = renderSvgIcon(":/res/icon/max.svg", QSize(16, 16), Qt::white);
        m_restoreIcon = renderSvgIcon(":/res/icon/restore.svg", QSize(16, 16), Qt::white);
        m_closeIcon = renderSvgIcon(":/res/icon/close.svg", QSize(10, 10), Qt::white);
        m_setIcon = renderSvgIcon(":/res/icon/set.svg", QSize(20, 20), Qt::white);

        m_minIconInactive = renderSvgIcon(":/res/icon/min.svg", QSize(16, 16), m_inactiveColor);
        m_maxIconInactive = renderSvgIcon(":/res/icon/max.svg", QSize(16, 16), m_inactiveColor);
        m_restoreIconInactive = renderSvgIcon(":/res/icon/restore.svg", QSize(16, 16), m_inactiveColor);
        m_closeIconInactive = renderSvgIcon(":/res/icon/close.svg", QSize(10, 10), m_inactiveColor);
        m_setIconInactive = renderSvgIcon(":/res/icon/set.svg", QSize(20, 20), m_inactiveColor);
    }
    else
    {
        // 亮色主题使用深色图标
        m_minIcon = renderSvgIcon(":/res/icon/min.svg", QSize(16, 16), Qt::black);
        m_maxIcon = renderSvgIcon(":/res/icon/max.svg", QSize(16, 16), Qt::black);
        m_restoreIcon = renderSvgIcon(":/res/icon/restore.svg", QSize(16, 16), Qt::black);
        m_closeIcon = renderSvgIcon(":/res/icon/close.svg", QSize(10, 10), Qt::black);
        m_setIcon = renderSvgIcon(":/res/icon/set.svg", QSize(20, 20), Qt::black);

        m_minIconInactive = renderSvgIcon(":/res/icon/min.svg", QSize(16, 16), m_inactiveColor);
        m_maxIconInactive = renderSvgIcon(":/res/icon/max.svg", QSize(16, 16), m_inactiveColor);
        m_restoreIconInactive = renderSvgIcon(":/res/icon/restore.svg", QSize(16, 16), m_inactiveColor);
        m_closeIconInactive = renderSvgIcon(":/res/icon/close.svg", QSize(10, 10), m_inactiveColor);
        m_setIconInactive = renderSvgIcon(":/res/icon/set.svg", QSize(20, 20), m_inactiveColor);
    }
}

void TitleBar::updateIcons()
{
    setIcons();

    // 更新按钮图标
    m_SetPBtn->setIcon(m_SetPBtn->isChecked() ? m_setIconChecked : m_setIcon);
    m_MaxPBtn->setIcon(m_isMaximized ? m_restoreIcon : m_maxIcon);
    m_MinPBtn->setIcon(m_minIcon);
    m_closePBtn->setIcon(m_closeIcon);
}

void TitleBar::updateStyle()
{
    QFont font = QApplication::font();
    font.setPointSize(9);
    setFont(font);
    setStyleSheet(QString(R"(
                          #titleBar{
                              font-family: "Microsoft YaHei";
                              background-color: %1;
                              border-top-left-radius: %2px;
                              border-top-right-radius: %2px;
                              font-size: 9pt;
                          }
                          #titleBar[windowState="1"] {
                              background-color: %1;
                              border-top-left-radius: 0px;
                              border-top-right-radius: 0px;
                              font-size: 9pt;
                          }
                          #titleBar[windowState="0"] {
                              background-color: %1;
                              border-top-left-radius: %2px;
                              border-top-right-radius: %2px;
                              font-size: 9pt;
                          }
                          #closepbtn {
                              border:none;
                              border-top-right-radius:%2px;
                          }
                          #closepbtn:hover {
                              background: red;
                          }
                          #closepbtn[windowState="1"] {
                              border-top-right-radius:0px;
                          }
                          #closepbtn[windowState="0"] {
                              border-top-right-radius:%2px;
                          }
                          #titleText {
                              font-family: "Microsoft YaHei";
                              color: %3;
                          }
                          #titleText[activation="1"] {
                              color: %3;
                          }
                          #titleText[activation="0"] {
                              color: %4;
                          }
                          )").arg(m_titleBarColor.name()).arg(10)
                          .arg(m_activeColor.name()).arg(m_inactiveColor.name()));

    // 触发样式更新
    m_titleTextLabel->style()->unpolish(m_titleTextLabel);
    m_titleTextLabel->style()->polish(m_titleTextLabel);
    m_closePBtn->style()->unpolish(m_closePBtn);
    m_closePBtn->style()->polish(m_closePBtn);
    style()->unpolish(this);
    style()->polish(this);

}

// 窗口焦点改变
void TitleBar::ActivationChanged(bool active)
{
    m_titleTextLabel->setProperty("activation", active ? 1 : 0);

    m_titleTextLabel->style()->unpolish(m_titleTextLabel);
    m_titleTextLabel->style()->polish(m_titleTextLabel);

    m_SetPBtn->setIcon(m_SetPBtn->isChecked() ? m_setIconChecked : (active ? m_setIcon : m_setIconInactive));
    m_MaxPBtn->setIcon(m_isMaximized
                       ? (active ? m_restoreIcon : m_restoreIconInactive)
                       : (active ? m_maxIcon :  m_maxIconInactive));
    m_MinPBtn->setIcon(active ? m_minIcon : m_minIconInactive);
    m_closePBtn->setIcon(active ? m_closeIcon : m_closeIconInactive);
}

void TitleBar::setMarginsWidth(int width)
{
    m_marginsWidth = width;
}

void TitleBar::setWindowTitle(const QString & name)
{
    m_titleTextLabel->setText(name);
    QWidget::setWindowTitle(name);
}

void TitleBar::setWindowIcon(const QString & path)
{
    QPixmap pix;
    QIcon icon = renderSvgIcon(path, QSize(20, 20));
    QPixmap pixmap(path);
    if(!icon.isNull()) pixmap = icon.pixmap(m_fixedHeight, m_fixedHeight);
    else pixmap = pixmap.scaled(m_fixedHeight, m_fixedHeight, Qt::KeepAspectRatio, Qt::SmoothTransformation);
    m_logoLabel->setAlignment(Qt::AlignCenter);
    m_logoLabel->setPixmap(pixmap);

}

void TitleBar::setWindowIcon(const QIcon &icon)
{
    m_logoLabel->setAlignment(Qt::AlignCenter);
    m_logoLabel->setPixmap(icon.pixmap(25, 25));
}

void TitleBar::setTitleBarColor(const QColor &color)
{
    m_titleBarColor = color;
    updateStyle();
}

// 在独立窗口时使用
void TitleBar::setPinButton(const QString &checkedPath, const QString &uncheckedPath)
{
    m_setIcon = renderSvgIcon(uncheckedPath, QSize(20, 20));
    m_setIconInactive = renderSvgIcon(uncheckedPath, QSize(20, 20), m_inactiveColor);
    m_setIconChecked = renderSvgIcon(checkedPath, QSize(20, 20));
}

void TitleBar::setPinButtonChecked(bool checked)
{
    m_SetPBtn->setCheckable(true);
    QObject::connect(m_SetPBtn, &QPushButton::toggled, [=](bool checked){
//        Qt::WindowFlags flags = window()->windowFlags();
        if(checked)
        {
            m_SetPBtn->setIcon(m_setIconChecked);

//            flags |= Qt::WindowStaysOnTopHint; // 添加置顶标志
            window()->setWindowFlag(Qt::WindowStaysOnTopHint, true);
        }
        else
        {
            m_SetPBtn->setIcon(m_setIcon);
//            flags &= ~Qt::WindowStaysOnTopHint; // 移除置顶标志
            window()->setWindowFlag(Qt::WindowStaysOnTopHint, false);
        }
//        qDebug() << "flags" << flags;
//        window()->setWindowFlags(flags); // 设置新的窗口标志
        window()->show(); // 必须调用show()使标志生效
    });
    m_SetPBtn->setChecked(checked);
}

QPushButton *TitleBar::getSetButton()
{
    return m_SetPBtn;
}

void TitleBar::onClicked()
{
    QPushButton* pBtn = qobject_cast<QPushButton*>(sender());

    if(pBtn == m_MinPBtn)
    {
        window()->showMinimized();
    }
    else if(pBtn == m_MaxPBtn)
    {
        if(m_isMaximized)
        {
            showNormal();
            window()->move(m_windowPos);
            m_MaxPBtn->setIcon(m_maxIcon);
            emit sig_max(false);
        }
        else
        {
            m_windowPos = window()->pos();
            if(m_windowPos == QPoint(0, 0)) window()->move(0, 1);
            showMaximized();
            m_MaxPBtn->setIcon(m_restoreIcon);
            emit sig_max(true);
        }
    }
    else if(pBtn == m_closePBtn)
    {
        emit sig_close();
    }
}

QSize TitleBar::sizeHint() const
{
    return QSize(800, m_fixedHeight);
}

void TitleBar::mousePressEvent(QMouseEvent *event)
{
    if(event->button() == Qt::LeftButton)
    {
        m_mouseRelease = false;
        m_dockingPreview = false;
        m_dragStartPos = event->globalPos();

        if(!m_isMaximized)
        {
            // 记录鼠标在窗口内的偏移量 (鼠标位置 - 窗口左上角位置)
            offset_pos = event->globalPos() - window()->pos();
            event->accept(); // 标记事件已处理
        }
        else
        {
            m_dragStartPos = event->pos();
            // 更新偏移量
            offset_pos = event->pos();
            event->accept();
        }
    }
    else event->ignore();
}

void TitleBar::mouseMoveEvent(QMouseEvent *event)
{
    if(event->buttons() & Qt::LeftButton)
    {
        //if(!window()->isMaximized() && !m_mouseRelease)
        if(!m_isMaximized && !m_mouseRelease)
        {
            // 新窗口位置 = 当前鼠标全局位置 - 鼠标在窗口内的偏移量
            window()->move(event->globalPos() - offset_pos);
            QScreen* screen = window()->windowHandle()->screen();
            QRect screenRect = screen->availableGeometry();
            // 检查是否到达屏幕顶部
//            qDebug() << "event->globalPos().y()" << event->globalPos().y();
//            qDebug() << "screen->geometry().top()" << screen->geometry().top();
            if(event->globalPos().y() == screen->geometry().top())
            {
                //qDebug() << "m_dockingPreview" << m_dockingPreview;
                if(!m_dockingPreview)
                {
                    m_dockingPreview = true;
                    // 计算预览区域（屏幕区域减去边框）
                    QRect previewRect = screenRect.adjusted(5, 5, -5, -5);
                    // 设置动画起始位置：从鼠标位置开始
                    QRect startRect(event->globalPos(), QSize(4, 4));
                    startRect.moveCenter(event->globalPos());
                    // 设置动画
                    m_previewAnimation->stop();
                    m_previewAnimation->setStartValue(startRect);
                    m_previewAnimation->setEndValue(previewRect);
                    // 显示并启动动画
                    //m_previewWidget->stackUnder(window());
                    m_previewWidget->show();
                    m_previewAnimation->start();
                    // 确保主窗口在前
                    window()->raise();
                }
            }
            else if(m_dockingPreview)
            {
                // 离开顶部区域，隐藏预览
                m_dockingPreview = false;
                m_previewAnimation->stop();
                m_previewWidget->hide();
            }


            event->accept();
        }
        else if(!m_mouseRelease)
        {
            // 先还原窗口
            showNormal();
            //window()->showNormal();
            emit sig_max(false);
            m_MaxPBtn->setIcon(m_maxIcon);
            // 计算还原后的窗口位置
            QScreen* windowScreen = window()->windowHandle()->screen();
            int screenWidth = windowScreen->geometry().width();
            // 左侧1/3
            if(m_dragStartPos.x() < screenWidth / 3)
            {
                qDebug() << u8"left";
                window()->move(0, m_dragStartPos.y());
                offset_pos = QPoint(m_dragStartPos.x(), m_dragStartPos.y() + m_marginsWidth);
            }
            else if(m_dragStartPos.x() > screenWidth * 2 / 3) // 右侧1/3
            {
                qDebug() << u8"right";
                int x = screenWidth - window()->width();
                window()->move(x, m_dragStartPos.y());
                offset_pos = QPoint(m_dragStartPos.x() - x, m_dragStartPos.y() + m_marginsWidth);
            }
            else // 中部：x轴鼠标位于窗口正中
            {
                qDebug() << u8"middle";
                int x = m_dragStartPos.x() - window()->width() / 2;
                window()->move(x, m_dragStartPos.y());
                offset_pos = QPoint(m_dragStartPos.x() - x, m_dragStartPos.y() + m_marginsWidth);
            }
            event->accept();
        }
    }
    else event->ignore();
}

void TitleBar::mouseReleaseEvent(QMouseEvent *event)
{
    if(event->button() == Qt::LeftButton)
    {
        if(m_dockingPreview && event->globalPos().y() == window()->windowHandle()->screen()->geometry().top())
        {
            // 停靠在顶部，最大化窗口
            m_dockingPreview = false;
            m_previewWidget->hide();

            //if(!window()->isMaximized())
            if(!m_isMaximized)
            {
                //window()->showMaximized();
                showMaximized();
                m_MaxPBtn->setIcon(m_restoreIcon);
                emit sig_max(true);
            }
        }

        m_mouseRelease = true;
        event->accept();
    }
    else event->ignore();
}

void TitleBar::mouseDoubleClickEvent(QMouseEvent *)
{
    m_MaxPBtn->clicked();
}
