#include "dialog.h"

#include <QDebug>
#include <QKeyEvent>
#include <QPushButton>
#include <stylemanager.h>

Dialog::Dialog(QWidget *parent) : QDialog(parent)
{
    setWindowFlags(Qt::FramelessWindowHint | Qt::Tool);
    setAttribute(Qt::WA_TranslucentBackground);

    initDialogUI();

    m_closePBtn = new QPushButton(this);
    m_closePBtn->setFixedSize(30, 30); // 建议固定大小，方便定位
    m_closePBtn->setObjectName("SettingCloseButton");
    StyleManager::getStyleManager()->registerWidget(m_closePBtn);
    QObject::connect(m_closePBtn, &QPushButton::clicked, this, &Dialog::close);
}

void Dialog::showEvent(QShowEvent *event)
{
    qDebug() << width();
    m_closePBtn->move(width() - m_closePBtn->width() - 15, 10);
    m_closePBtn->raise(); // 确保按钮在最上层
    QWidget::showEvent(event);
}

void Dialog::keyPressEvent(QKeyEvent *event)
{
    // 阻止回车键关闭对话框
    if(event->key() == Qt::Key_Return || event->key() == Qt::Key_Enter)
    {
        event->accept();  // 接受事件但不处理，防止对话框关闭
        return;
    }

    // 其他按键按默认方式处理
    QDialog::keyPressEvent(event);
}

void Dialog::mousePressEvent(QMouseEvent *event)
{
    if(event->button() == Qt::LeftButton)
    {
         bringToFront();

        // 仅当点击标题栏区域（顶部30像素）时才启动拖动
        if(event->pos().y() <= 30)
        {
            m_dragPosition = event->globalPos() - frameGeometry().topLeft();
            m_isDragging = true;
            event->accept();
            return;
        }
    }
    QDialog::mousePressEvent(event);
}

void Dialog::mouseMoveEvent(QMouseEvent *event)
{
    if(m_isDragging && event->buttons() & Qt::LeftButton)
    {
        move(event->globalPos() - m_dragPosition);
        event->accept();
        return;
    }
    QDialog::mouseMoveEvent(event);
}

void Dialog::mouseReleaseEvent(QMouseEvent *event)
{
    if(event->button() == Qt::LeftButton)
    {
        m_isDragging = false;
    }
    QDialog::mouseReleaseEvent(event);
}

void Dialog::initDialogUI()
{
    m_centralWidget = new QWidget(this);
    m_centralWidget->setObjectName("SettingDialog");
    StyleManager::getStyleManager()->registerWidget(m_centralWidget);
}

void Dialog::bringToFront()
{
    // 确保对话框在最上层
    raise();
    activateWindow();
}
