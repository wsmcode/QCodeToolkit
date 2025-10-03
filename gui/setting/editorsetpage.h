#ifndef EDITORSETPAGE_H
#define EDITORSETPAGE_H

/*****************************************************
*
* @file     editorsetpage.h
* @brief    EditorSetPage类
*
* @description
*           ==== 布局 ====
*           ==== 核心功能 ====
*           2. 编辑器
*               1. 外滚轮是否跟随内滚轮：复选框控制
*           	2. 滚轮速度：下拉框选择
*           	3. 关闭后直接保存：复选框控制
*           	4. 自动保存：复选框控制（选择后，关闭后直接保存置灰）
*           ==== 使用说明 ====
*           ==== 注意 ====
*
* @author   无声目
* @date     2025/09/23
* @history
*****************************************************/
#include "settingspage.h"

class ComboBox;
class QCheckBox;
class EditorSetPage : public SettingsPage
{
    Q_OBJECT
public:
    explicit EditorSetPage(QWidget *parent = nullptr);

    QString title() const override;

    void load(const AppSettings &) override;
    void save(AppSettings &) override;

signals:

private:
    void initUI() override;
    void initConnections();

    QCheckBox *m_outerWheelFollowsInnerCheckBox;
    ComboBox *m_wheelSpeedComboBox;
    QCheckBox *m_saveOnCloseCheckBox;
    QCheckBox *m_autoSaveCheckBox;

};

#endif // EDITORSETPAGE_H
