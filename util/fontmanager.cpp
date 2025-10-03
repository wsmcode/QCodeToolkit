#include "fontmanager.h"

#include <QApplication>
#include <QDebug>
#include <QRegularExpression>
#include <QWidget>

FontManager::FontManager(QObject *parent) : QObject(parent)
{
    m_baseFont = QApplication::font();
}

FontManager::~FontManager()
{

}

void FontManager::setBaseFont(const QFont &font)
{
    if(m_baseFont != font)
    {
        m_baseFont = font;
        updateAllWidgetsFont();
        emit fontChanged(font);
    }
}

void FontManager::setBaseFont(const QString &family, int pointSize)
{
    QFont font = m_baseFont;
    font.setFamily(family);
    font.setPointSize(pointSize);
    setBaseFont(font);
}

QFont FontManager::baseFont() const
{
    return m_baseFont;
}

void FontManager::registerWidget(QWidget *widget)
{
    if(widget && !m_registeredWidgets.contains(widget))
    {
        m_registeredWidgets.insert(widget);
        widget->setFont(m_baseFont);

        // 当控件被销毁时自动取消注册
        connect(widget, &QObject::destroyed, this, [this, widget]() {
            m_registeredWidgets.remove(widget);
        });
    }
}

void FontManager::unregisterWidget(QWidget *widget)
{
    m_registeredWidgets.remove(widget);
}

void FontManager::updateAllWidgetsFont()
{
    for(QWidget *widget : m_registeredWidgets)
    {
        if(widget)
        {
            widget->setFont(m_baseFont);
        }

    }
}
