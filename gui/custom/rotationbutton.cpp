#include "rotationbutton.h"

#include <QPainter>
#include <QStyleOptionButton>

RotationButton::RotationButton(QWidget *parent) : QPushButton(parent)
{

}

void RotationButton::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event);
    QPainter painter(this);
    painter.setRenderHint(QPainter::SmoothPixmapTransform);

    // 保存原始状态
    painter.save();

    // 平移坐标系到中心
    painter.translate(width() / 2, height() / 2);
    painter.rotate(m_rotation);
    painter.translate(-width() / 2, -height() / 2);

    // 使用样式表绘制按钮
    QStyleOptionButton option;
    initStyleOption(&option);
    style()->drawControl(QStyle::CE_PushButton, &option, &painter, this);

    // 恢复原始状态
    painter.restore();
}

int RotationButton::rotation() const
{
    return m_rotation;
}

void RotationButton::setRotation(int rotation)
{
    m_rotation = rotation;
    update();
}
