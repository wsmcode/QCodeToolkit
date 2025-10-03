#include "menutoolbutton.h"

#include <QDebug>
#include <QIcon>
#include <QMouseEvent>
#include <QPainter>
#include <QPropertyAnimation>
#include <QSvgRenderer>

MenuToolButton::MenuToolButton(QWidget *parent) : QWidget(parent)
{
    init();
}

MenuToolButton::MenuToolButton(const QString &imagePath, QWidget *parent) : QWidget(parent)
{
    setImage(imagePath);
    init();
}

MenuToolButton::MenuToolButton(const QString &imagePath, const QString &toolTip, QWidget *parent)
    : QWidget(parent)
{
    setImage(imagePath);
    setToolTip(toolTip);
    init();
}

void MenuToolButton::setImage(const QString &image)
{
    // 普通图片
//    m_imagePath = image;
//    m_pixmap.load(image);

//    if(!m_pixmap.isNull())
//    {
//        // 计算不同状态下的尺寸
//        calculateSizes();

//        // 设置初始尺寸
//        updateButtonSize();
//    }

//    update();

    // svg图片
    // 清理旧的渲染器
    if(m_svgRenderer)
    {
        delete m_svgRenderer;
        m_svgRenderer = nullptr;
    }

    // 创建新的SVG渲染器
    m_svgRenderer = new QSvgRenderer(image, this);

    if(m_svgRenderer->isValid())
    {
        // 计算不同状态下的尺寸
        calculateSizes();

        // 设置初始尺寸
        updateButtonSize();
    }
    else
    {
        qWarning() << "Failed to load SVG:" << image;
        // 可以在这里添加回退到QPixmap的逻辑
    }
    update();
}

void MenuToolButton::setChecked(bool checked)
{
    if(m_checked != checked)
    {
        m_checked = checked;
        update();
        emit toggled(checked);
    }
}

bool MenuToolButton::isChecked() const
{
    return m_checked;
}

void MenuToolButton::setClickAnimationDuration(int ms)
{
    m_clickAnimationDuration = ms;
    m_clickAnimation->setDuration(ms);
}

void MenuToolButton::setHoverAnimationDuration(int ms)
{
    m_hoverAnimationDuration = ms;
    m_enterAnimation->setDuration(ms);
    m_leaveAnimation->setDuration(ms);
}

void MenuToolButton::setParentOrientation(Qt::Orientation orientation)
{
    if(m_parentOrientation != orientation)
    {
        m_parentOrientation = orientation;
        updateButtonSize(); // 方向改变时更新按钮尺寸
    }
}

Qt::Orientation MenuToolButton::parentOrientation() const
{
    return m_parentOrientation;
}

void MenuToolButton::enterEvent(QEvent *event)
{
    m_isHovered = true;

    // 停止之前的动画
    m_leaveAnimation->stop();

    // 设置进入动画：从当前尺寸过渡到悬停尺寸
    m_enterAnimation->setStartValue(geometry());

    // 根据父控件方向计算悬停时的目标尺寸（正方形）
    int targetSize;
    if(m_parentOrientation == Qt::Horizontal)
    {
        // 水平方向：基于父控件高度计算
        targetSize = parentWidget() ?
                    parentWidget()->height() * m_sizeRatio / 100 * m_hoverScale : m_hoverSize;
    }
    else
    {
        // 垂直方向：基于父控件宽度计算
        targetSize = parentWidget() ?
                    parentWidget()->width() * m_sizeRatio / 100 * m_hoverScale : m_hoverSize;
    }

    QRect targetRect = QRect(x(), y(), targetSize, targetSize);
    m_enterAnimation->setEndValue(targetRect);
    m_enterAnimation->start();

    QWidget::enterEvent(event);
}

void MenuToolButton::leaveEvent(QEvent *event)
{
    m_isHovered = false;
    m_pressed = false;

    // 停止之前的动画
    m_enterAnimation->stop();

    // 设置离开动画
    m_leaveAnimation->setStartValue(geometry());

    // 根据父控件方向计算正常状态的目标尺寸（正方形）
    int targetSize;
    if (m_parentOrientation == Qt::Horizontal) {
        // 水平方向：基于父控件高度计算
        targetSize = parentWidget() ? parentWidget()->height() * m_sizeRatio / 100 : m_normalSize;
    } else {
        // 垂直方向：基于父控件宽度计算
        targetSize = parentWidget() ? parentWidget()->width() * m_sizeRatio / 100 : m_normalSize;
    }

    QRect targetRect = QRect(x(), y(), targetSize, targetSize);
    m_leaveAnimation->setEndValue(targetRect);
    m_leaveAnimation->start();

    QWidget::leaveEvent(event);
}

void MenuToolButton::paintEvent(QPaintEvent *)
{
    // svg图片
    if(!m_svgRenderer || !m_svgRenderer->isValid()) return;

    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);
    painter.setRenderHint(QPainter::SmoothPixmapTransform);

    // 设置透明度
    painter.setOpacity(m_opacity);

    // 计算绘制区域（保持宽高比）
    QSize svgSize = m_svgRenderer->defaultSize();
    svgSize.scale(m_currentSize, m_currentSize, Qt::KeepAspectRatio);

    int pixX = (width() - svgSize.width()) / 2;
    int pixY = (height() - svgSize.height()) / 2;
    QRect targetRect(pixX, pixY, svgSize.width(), svgSize.height());

    // 直接渲染SVG，避免中间转换
    m_svgRenderer->render(&painter, targetRect);

    // 普通图片
//    if(m_pixmap.isNull()) return;

//    QPainter painter(this);
//    painter.setRenderHint(QPainter::Antialiasing);
//    painter.setRenderHint(QPainter::SmoothPixmapTransform); // 开启平滑像素图变换

//    // 设置透明度
//    painter.setOpacity(m_opacity);

//    // 计算绘制区域（保持图片宽高比）
//    QRect targetRect;
//    QSize pixmapSize = m_pixmap.size();
//    pixmapSize.scale(m_currentSize, m_currentSize, Qt::KeepAspectRatio);

//    // 计算绘制位置（居中）
//    int pixX = (width() - pixmapSize.width()) / 2;
//    int pixY = (height() - pixmapSize.height()) / 2;
//    targetRect = QRect(pixX, pixY, pixmapSize.width(), pixmapSize.height());

//    // 绘制图片
//    QPixmap scaledPix = m_pixmap.scaled(pixmapSize, Qt::KeepAspectRatio, Qt::SmoothTransformation);
//    painter.drawPixmap(targetRect, scaledPix);

//    // 如果被选中，绘制选中状态指示
//    if(m_checked)
//    {
//        painter.setOpacity(1.0); // 选中框不使用透明度
//        QPen pen(QColor(0, 120, 215), 2); // 蓝色边框，2像素宽
//        painter.setPen(pen);
//        painter.drawRect(rect().adjusted(1, 1, -1, -1));  // 向内调整1像素，避免绘制在边界上
//    }
}

void MenuToolButton::mousePressEvent(QMouseEvent *event)
{
    if(event->button() == Qt::LeftButton)
    {
        m_pressed = true;
        startClickAnimation(); // 开始点击动画
    }
    QWidget::mousePressEvent(event);
}

void MenuToolButton::mouseReleaseEvent(QMouseEvent *event)
{
    if(event->button() == Qt::LeftButton && m_pressed)
    {
        m_pressed = false;
        if(rect().contains(event->pos()))
        {
            setChecked(!m_checked);
            emit clicked();
        }
    }
    QWidget::mouseReleaseEvent(event);
}

void MenuToolButton::resizeEvent(QResizeEvent *event)
{
    // 父控件尺寸变化时，重新计算按钮尺寸
    updateButtonSize();
    QWidget::resizeEvent(event);
}

qreal MenuToolButton::opacity() const
{
    return m_opacity;
}

void MenuToolButton::setOpacity(qreal opacity)
{
    if(qFuzzyCompare(m_opacity, opacity)) return; // 避免重复设置相同的值

    m_opacity = opacity;
    update();
}

void MenuToolButton::onEnterAnimationFinished()
{
    m_currentSize = m_hoverSize;
    update();
}

void MenuToolButton::onLeaveAnimationFinished()
{
    m_currentSize = m_normalSize;
    update();
}

void MenuToolButton::onClickAnimationFinished()
{
    // 点击动画结束后恢复正常透明度
    if(m_clickAnimation->direction() == QPropertyAnimation::Forward)
    {
        m_clickAnimation->setDirection(QPropertyAnimation::Backward); // 反向播放动画
        m_clickAnimation->start();
    }
}

void MenuToolButton::init()
{
    setCursor(Qt::PointingHandCursor);  // 设置手型光标，表示可点击
    setFocusPolicy(Qt::StrongFocus); // 支持键盘焦点

    // 初始化进入动画
    m_enterAnimation = new QPropertyAnimation(this, "geometry", this);
    m_enterAnimation->setEasingCurve(QEasingCurve::OutCubic); // 设置缓动曲线为三次方缓出
    m_enterAnimation->setDuration(m_hoverAnimationDuration);  // 设置动画时长
    QObject::connect(m_enterAnimation, &QPropertyAnimation::finished,
                     this, &MenuToolButton::onEnterAnimationFinished);

    // 初始化离开动画
    m_leaveAnimation = new QPropertyAnimation(this, "geometry", this);
    m_leaveAnimation->setEasingCurve(QEasingCurve::OutCubic); // 设置缓动曲线
    m_leaveAnimation->setDuration(m_hoverAnimationDuration);  // 设置动画时长
    QObject::connect(m_leaveAnimation, &QPropertyAnimation::finished,
                     this, &MenuToolButton::onLeaveAnimationFinished);

    // 初始化点击动画（透明度变化）
    m_clickAnimation = new QPropertyAnimation(this, "opacity", this);
    m_clickAnimation->setEasingCurve(QEasingCurve::InOutQuad); // 设置缓动曲线为二次方缓入缓出
    m_clickAnimation->setDuration(m_clickAnimationDuration);   // 设置动画时长
    QObject::connect(m_clickAnimation, &QPropertyAnimation::finished,
                     this, &MenuToolButton::onClickAnimationFinished);
}

void MenuToolButton::startClickAnimation()
{
    m_clickAnimation->stop();
    m_clickAnimation->setDirection(QPropertyAnimation::Forward); // 正向播放动画
    m_clickAnimation->setStartValue(1.0);
    m_clickAnimation->setEndValue(0.7);
    m_clickAnimation->start();
}

void MenuToolButton::updatePixmapSize()
{
    // 根据悬停状态更新当前图片尺寸
    if(m_isHovered) m_currentSize = m_hoverSize;
    else m_currentSize = m_normalSize;

    update(); // 更新显示
}

void MenuToolButton::updateButtonSize()
{
//    if(!m_pixmap.isNull())
    if(m_svgRenderer && m_svgRenderer->isValid())
    {
        // 计算各种状态下的尺寸
        calculateSizes();

        setFixedSize(m_hoverSize, m_hoverSize); // 设置正方形尺寸

        // 更新当前图片尺寸
        updatePixmapSize();
    }
}

void MenuToolButton::calculateSizes()
{
    // 普通图片
//    if(!m_pixmap.isNull())
//    {
//        // 基于父控件方向
//        if(m_parentOrientation == Qt::Horizontal)
//        {
//            // 水平方向：基于父控件高度计算
//            m_normalSize = parentWidget() ?
//                          parentWidget()->height() * m_sizeRatio / 100 :
//                          qMin(m_pixmap.width(), m_pixmap.height()) * 0.5;
//        }
//        else
//        {
//            // 垂直方向：基于父控件宽度计算
//            m_normalSize = parentWidget() ?
//                          parentWidget()->width() * m_sizeRatio / 100 :
//                          qMin(m_pixmap.width(), m_pixmap.height()) * 0.5;
//        }
//        m_hoverSize = m_normalSize * m_hoverScale; // 悬停时放大

//        // 设置初始尺寸
//        m_currentSize = m_normalSize;
//    }

    // svg图片
    QSize defaultSize = m_svgRenderer->defaultSize();
    if(m_svgRenderer && m_svgRenderer->isValid())
    {
        // 基于父控件方向计算基础尺寸
        if(m_parentOrientation == Qt::Horizontal)
        {
            // 水平方向：基于父控件高度计算
            m_normalSize = parentWidget() ?
                          parentWidget()->height() * m_sizeRatio / 100 :
                          qMin(defaultSize.width(), defaultSize.height()) * 0.5;
        }
        else
        {
            // 垂直方向：基于父控件宽度计算
            m_normalSize = parentWidget() ?
                          parentWidget()->width() * m_sizeRatio / 100 :
                          qMin(defaultSize.width(), defaultSize.height()) * 0.5;
        }

        m_hoverSize = m_normalSize * m_hoverScale; // 悬停时放大

        // 设置初始尺寸
        m_currentSize = m_normalSize;
    }
}
