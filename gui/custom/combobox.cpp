#include "combobox.h"
#include "stylemanager.h"

#include <QWheelEvent>

ComboBox::ComboBox(QWidget *parent) : QComboBox(parent)
{
    setObjectName("comboBox");
    StyleManager::getStyleManager()->registerWidget(this);
}

void ComboBox::wheelEvent(QWheelEvent *event)
{
    event->ignore();
}
