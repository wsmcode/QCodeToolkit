#include "fontcombobox.h"

#include <QApplication>
#include <QDebug>
#include <QFontDatabase>

FontComboBox::FontComboBox(QWidget *parent) : ComboBox(parent)
{
    initTimer();
}

void FontComboBox::showPopup()
{
    if(m_clearTimer.isActive()) m_clearTimer.stop();

    // 如果字体未加载，则加载字体列表
    if(!m_fontsLoaded) loadFonts();

    ComboBox::showPopup();
}

void FontComboBox::hidePopup()
{
    ComboBox::hidePopup();

    // 启动清除定时器
    if(m_fontsLoaded) m_clearTimer.start();
}

void FontComboBox::loadFonts()
{
    if(m_fontsLoaded) return;

    qDebug() << "Loading font list...";

    // 保存当前选中的字体
    QString currentFont = currentText();
    qDebug() << "currentText" << currentFont;

    // 获取系统字体列表
    QFontDatabase fontDatabase;
    QStringList fontFamilies = fontDatabase.families();

    // 添加所有字体到下拉框
    clear();
    addItems(fontFamilies);

    // 恢复选中的字体
    if(!currentFont.isEmpty()) setCurrentText(currentFont);

    m_fontsLoaded = true;
    qDebug() << "Font list loaded, total fonts:" << fontFamilies.size();
}

void FontComboBox::clearFonts()
{
    if(!m_fontsLoaded) return;

    qDebug() << "Clearing font list...";

    // 保存当前选中的字体
    QString currentFont = currentText();
    clear();
    addItem(currentFont);
    setCurrentText(currentFont);

    m_fontsLoaded = false;
    qDebug() << "Font list cleared";
}

void FontComboBox::initTimer()
{
    m_clearTimer.setSingleShot(true);
    m_clearTimer.setInterval(1000); // 1秒后清除
    QObject::connect(&m_clearTimer, &QTimer::timeout, this, &FontComboBox::clearFonts);
}
