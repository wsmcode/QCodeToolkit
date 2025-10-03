#ifndef MENUSETPAGE_H
#define MENUSETPAGE_H

/*****************************************************
*
* @file     menusetpage.h
* @brief    MenuSetPage类
*
* @description
*           ==== 布局 ====
*           ==== 核心功能 ====
*           4. 菜单栏
*           	1. 菜单栏项目：新项目、代码提示词列表、筛选、快速设置：每个项目都是一个复选框（选择代表在菜单栏显示）
*           ==== 使用说明 ====
*           ==== 注意 ====
*
* @author   无声目
* @date     2025/09/23
* @history
*****************************************************/
#include "settingspage.h"

class QCheckBox;
class MenuSetPage : public SettingsPage
{
    Q_OBJECT
public:
    explicit MenuSetPage(QWidget *parent = nullptr);

    QString title() const override;

    void load(const AppSettings &) override;
    void save(AppSettings &) override;

signals:

private:
    void initUI() override;
    void initConnections();

    QCheckBox *m_newProjectCheckBox;
    QCheckBox *m_codeCompletionListCheckBox;
    QCheckBox *m_filterCheckBox;
    QCheckBox *m_quickSettingCheckBox;
};

#endif // MENUSETPAGE_H
