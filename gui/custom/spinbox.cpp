#include "spinbox.h"
#include "stylemanager.h"

#include <QWheelEvent>

SpinBox::SpinBox(QWidget *parent) : QSpinBox(parent)
{
    setObjectName("SpinBox");
    StyleManager::getStyleManager()->registerWidget(this);
}

void SpinBox::wheelEvent(QWheelEvent *event)
{
    event->ignore();
}
