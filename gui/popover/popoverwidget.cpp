#include "framelesswindowbase.h"
#include "popoverwidget.h"
#include "stylemanager.h"

#include <QListWidget>
#include <QVBoxLayout>
#include <QApplication>
#include <QKeyEvent>
#include <QMouseEvent>
#include <QScreen>
#include <QDebug>
#include <QLabel>
#include <QGraphicsDropShadowEffect>

PopoverWidget::PopoverWidget(QWidget *parent)
    : QFrame(parent, Qt::Tool | Qt::FramelessWindowHint | Qt::NoDropShadowWindowHint)
{
    m_respondedWidget = parent;
    // 阴影效果可以把这一层改成透明窗口
//    QGraphicsDropShadowEffect *m_shadowEffect = new QGraphicsDropShadowEffect(this);
//    m_shadowEffect->setBlurRadius(15);
//    m_shadowEffect->setColor(QColor(63, 63, 63, 90));
//    m_shadowEffect->setOffset(0, 0);
//    setGraphicsEffect(m_shadowEffect);
    setAttribute(Qt::WA_X11NetWmWindowTypeCombo);
    // 设置 QFrame 的边框样式
    setFrameShape(QFrame::Box);
    setFrameShadow(QFrame::Plain);
    setLineWidth(1);
    // 设置样式
    setObjectName("PopoverWidget");
    StyleManager::getStyleManager()->registerWidget(this);
    setMouseTracking(true); // 启用鼠标跟踪
    initUI();


}

void PopoverWidget::addItem(QWidget *itemWidget)
{
    // 确保列表可见
    listWidget->setVisible(true);

    // 创建列表项
    QListWidgetItem *item = new QListWidgetItem(listWidget);
    item->setSizeHint(itemWidget->sizeHint());

    // 设置项控件
    listWidget->setItemWidget(item, itemWidget);
}

void PopoverWidget::addItem(const QString &text)
{
    // 创建文本标签控件
    QLabel *label = new QLabel(text);
    label->setMargin(4);

    // 添加到列表
    addItem(label);
}

void PopoverWidget::addItems(const QStringList &texts)
{
    for(const QString &text : texts)
    {
        addItem(text);
    }
}

void PopoverWidget::setItems(const QStringList &texts)
{
    clearItems();
    addItems(texts);
}

void PopoverWidget::insertItem(int index, QWidget *itemWidget)
{
    // 确保列表可见
    listWidget->setVisible(true);

    // 创建列表项并插入到指定位置
    QListWidgetItem *item = new QListWidgetItem();
    item->setSizeHint(itemWidget->sizeHint());

    listWidget->insertItem(index, item);
    listWidget->setItemWidget(item, itemWidget);
}

void PopoverWidget::insertItem(int index, const QString &text)
{
    // 创建文本标签控件
    QLabel *label = new QLabel(text);
    label->setMargin(4);

    // 插入到列表
    insertItem(index, label);
}

QListWidgetItem *PopoverWidget::item(int row)
{
    return listWidget->item(row);
}

QWidget *PopoverWidget::itemWidget(QListWidgetItem *item)
{
    return listWidget->itemWidget(item);
}

QString PopoverWidget::itemText(int row)
{
    QLabel *label = qobject_cast<QLabel *>(itemWidget(item(row)));
    QString text = "";
    if(label) text = label->text();
    return text;
}

void PopoverWidget::clearItems()
{
    listWidget->clear();
}

void PopoverWidget::showRelativeToWidget(QWidget *widget, PD direction, PP placement)
{
    if(!widget && m_respondedWidget) widget = m_respondedWidget;
    if(!widget && parent()) widget = parentWidget();
    if(!widget) return;

    // 调整大小以适应内容
    adjustSize();

    // 计算位置
    QPoint pos = calculatePosition(widget, direction, placement);
    move(pos);

    // 确保不超出屏幕
    QRect screenGeometry = QApplication::primaryScreen()->availableGeometry();
    QRect popoverRect(pos, size());

    // 水平方向调整
    if(popoverRect.right() > screenGeometry.right())
    {
        int dx = screenGeometry.right() - popoverRect.right();
        move(x() + dx, y());
    }
    else if(popoverRect.left() < screenGeometry.left())
    {
        int dx = screenGeometry.left() - popoverRect.left();
        move(x() + dx, y());
    }

    // 垂直方向调整
    if(popoverRect.bottom() > screenGeometry.bottom())
    {
        int dy = screenGeometry.bottom() - popoverRect.bottom();
        move(x(), y() + dy);
    }
    else if(popoverRect.top() < screenGeometry.top())
    {
        int dy = screenGeometry.top() - popoverRect.top();
        move(x(), y() + dy);
    }

    show();// 显示悬浮层
    setFocus();// 设置焦点
    raise();
}

void PopoverWidget::showSouth(QWidget *widget, PP placement)
{
    if(!widget && m_respondedWidget) widget = m_respondedWidget;
    if(!widget && parent()) widget = parentWidget();
    if(!widget) return;
    showRelativeToWidget(widget, PD::South, placement);
}

void PopoverWidget::showNorth(QWidget *widget, PP placement)
{
    if(!widget && m_respondedWidget) widget = m_respondedWidget;
    if(!widget && parent()) widget = parentWidget();
    if(!widget) return;
    showRelativeToWidget(widget, PD::North, placement);
}

void PopoverWidget::showWest(QWidget *widget, PP placement)
{
    if(!widget && m_respondedWidget) widget = m_respondedWidget;
    if(!widget && parent()) widget = parentWidget();
    if(!widget) return;
    showRelativeToWidget(widget, PD::West, placement);
}

void PopoverWidget::showEast(QWidget *widget, PP placement)
{
    if(!widget && m_respondedWidget) widget = m_respondedWidget;
    if(!widget && parent()) widget = parentWidget();
    if(!widget) return;
    showRelativeToWidget(widget, PD::East, placement);
}

void PopoverWidget::setContentWidget(QWidget *widget)
{
    // 隐藏列表
    listWidget->setVisible(false);

    // 删除现有内容（如果有）
    if(contentWidget)
    {
        mainVLayout->removeWidget(contentWidget);
        delete contentWidget;
    }

    // 设置新内容
    contentWidget = widget;
    mainVLayout->addWidget(contentWidget);
}

void PopoverWidget::setParent(QWidget *widget)
{
    if(!m_respondedWidget) m_respondedWidget = widget;
    QWidget::setParent(widget);
}

void PopoverWidget::setRespondedWidget(QWidget *widget)
{
    m_respondedWidget = widget;
}

void PopoverWidget::keyPressEvent(QKeyEvent *event)
{
    // ESC键关闭悬浮层
    if(event->key() == Qt::Key_Escape)
    {
        hide();
        event->accept();
        return;
    }

    QWidget::keyPressEvent(event);
}

void PopoverWidget::hideEvent(QHideEvent *event)
{
    emit aboutToHide();
//    qDebug() << event;
    QWidget::hideEvent(event);
}

bool PopoverWidget::eventFilter(QObject *obj, QEvent *event)
{
    if(event->type() == QEvent::MouseButtonPress)
    {
        QMouseEvent *mouseEvent = static_cast<QMouseEvent*>(event);
//        qDebug() << "Mouse press on:" << obj << "at:" << mouseEvent->globalPos();
        // 检查点击是否在PopoverWidget或其父控件区域内
        QPoint globalPos = mouseEvent->globalPos();
        QWidget *responded = m_respondedWidget;

        // 如果点击不在PopoverWidget内，也不在父控件内，则隐藏
        if(!rect().contains(mapFromGlobal(globalPos)) &&
            (!responded || !responded->rect().contains(responded->mapFromGlobal(globalPos))))
        {
            if(isVisible()) hide();
        }
    }
    else if(event->type() == QEvent::WindowActivate ||
             event->type() == QEvent::WindowDeactivate ||
             event->type() == QEvent::ApplicationActivate ||
             event->type() == QEvent::ApplicationDeactivate)
    {
        // 窗口激活状态变化时处理
        if(!qApp->activeWindow() || qApp->applicationState() != Qt::ApplicationActive)
        {
            hide();
        }
    }
    return QWidget::eventFilter(obj, event);
}

void PopoverWidget::initUI()
{
    // 主布局
    mainVLayout = new QVBoxLayout(this);
    mainVLayout->setSpacing(0);
    mainVLayout->setContentsMargins(5, 5, 5, 5);

    // 创建列表控件
    listWidget = new QListWidget(this);
    listWidget->setFrameShape(QFrame::NoFrame);
    listWidget->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    listWidget->setSelectionMode(QAbstractItemView::SingleSelection);
    // 初始时不显示列表
    listWidget->setVisible(false);

    mainVLayout->addWidget(listWidget);

    // 安装事件过滤器以处理焦点丢失
    qApp->installEventFilter(this);

    // 连接列表项点击信号
    QObject::connect(listWidget, &QListWidget::itemClicked, [=](QListWidgetItem *item){
        emit itemClicked(listWidget->row(item));
    });
}

QPoint PopoverWidget::calculatePosition(QWidget *widget, PD direction, PP placement)
{
    QPoint globalPos = widget->mapToGlobal(QPoint(0, 0));
    QSize widgetSize = widget->size();
    QSize popoverSize = size();

    // 根据方向计算位置
    switch(direction)
    {
    case PD::South:
        if(placement == PP::External)
            return QPoint(globalPos.x(), globalPos.y() + widgetSize.height() + m_margin);
        else
            return QPoint(globalPos.x(), globalPos.y() + widgetSize.height() - popoverSize.height() - m_margin);

    case PD::North:
        if(placement == PP::External)
            return QPoint(globalPos.x(), globalPos.y() - popoverSize.height() - m_margin);
        else
            return QPoint(globalPos.x(), globalPos.y() + m_margin);

    case PD::West:
        if(placement == PP::External)
            return QPoint(globalPos.x() - popoverSize.width() - m_margin, globalPos.y());
        else
            return QPoint(globalPos.x() + m_margin, globalPos.y());

    case PD::East:
        if(placement == PP::External)
            return QPoint(globalPos.x() + widgetSize.width() + m_margin, globalPos.y());
        else
            return QPoint(globalPos.x() + widgetSize.width() - popoverSize.width() - m_margin, globalPos.y());

    default:
        return QPoint(globalPos.x(), globalPos.y() + widgetSize.height() + m_margin);
    }
}
