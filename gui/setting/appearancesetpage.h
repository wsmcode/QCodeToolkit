#ifndef APPEARANCESETPAGE_H
#define APPEARANCESETPAGE_H

/*****************************************************
*
* @file     appearancesetpage.h
* @brief    AppearanceSetPage类
*
* @description
*           ==== 布局 ====
*           ==== 核心功能 ====
*           1. 外观
*           	1. 主题：Dark/Light
*           	2. 字体和颜色：字体选择框+字号选择+颜色选择按钮
*           	3. Zoom(按住Ctrl时使用鼠标滑轮缩放)
*           	4. 状态栏
*           	5. 字数统计
*           ==== 使用说明 ====
*           ==== 注意 ====
*
* @author   无声目
* @date     2025/09/23
* @history
*****************************************************/

#include "settingspage.h"

class SpinBox;
class ComboBox;
class QCheckBox;
class QPushButton;
class QRadioButton;
class AppearanceSetPage : public SettingsPage
{
    Q_OBJECT
public:
    explicit AppearanceSetPage(QWidget *parent = nullptr);

    QString title() const override;

    void load(const AppSettings &) override;
    void save(AppSettings &) override;

signals:

private slots:
//    void onThemeToggled(bool checked);
//    void onFontDialogClicked();
//    void onZoomLevelChanged(int index);

private:
    void initUI() override;
    void initConnections();

    // 主题设置
    QRadioButton *m_themeLightRButton;
    QRadioButton *m_themeDarkRButton;

    // 字体设置
    SpinBox *m_fontSizeBox;
    ComboBox *m_fontComboBox;

    // Zoom设置
    ComboBox *m_zoomComboBox;
    QCheckBox *m_zoomWheelCheckBox;

    // 显示设置
    QCheckBox *m_showStatusCheckBox;
    QCheckBox *m_showWordCountCheckBox;

    QPushButton *m_fontDialogButton;

    bool m_fontsLoaded = false;
};

#endif // APPEARANCESETPAGE_H
