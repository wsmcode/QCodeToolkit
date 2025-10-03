#ifndef SETTINGWIDGET_H
#define SETTINGWIDGET_H

/*****************************************************
*
* @file     settingdialog.h
* @brief    SettingDialog类
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
*           2. 编辑器
*               1. 外滚轮是否跟随内滚轮：复选框控制
*           	2. 滚轮速度：下拉框选择
*           	3. 关闭后直接保存：复选框控制
*           	4. 自动保存：复选框控制（选择后，关闭后直接保存置灰）
*           3. 代码
*           	1. 默认缩进：下拉框选择
*           	2. 默认代码语言：下拉框选择
*           	3. 使用成对符号：复选框控制
*           	4. 代码行号：复选框控制
*           	5. 设置片段
*           4. 菜单栏
*           	1. 菜单栏项目：新项目、代码提示词列表、筛选、快速设置：每个项目都是一个复选框（选择代表在菜单栏显示）
*           5. 导出
*           	1. 当前版本未实现
*           6. 通用
*           	1. 语言：下拉框选择
*           	2. 更新：检查更新按钮
*           	3. 全局快捷键：一个网格布局（第一列是动作：QLabel，第二列是快捷键：QLineEdit，第三列是恢复默认：按钮）
*                   1. 最后一行第一列：按钮（内置快捷键）：弹出一个列表布局，第一列是动作：QLabel，第二列是快捷键：QLabel
*           	4. 开发者设置
*           		1. 日志开关
*           		2. 日志等级
*
*           ==== 使用说明 ====
*           ==== 注意 ====
*
* @author   无声目
* @date     2025/09/20
* @history
*****************************************************/


#include <dialog.h>

#include "settings_types.h"

class QListWidget;
class SettingsPage;
class QStackedWidget;
class SettingDialog : public Dialog
{
    Q_OBJECT
public:
    explicit SettingDialog(QWidget *parent = nullptr);

    void load(const AppSettings &settings);
    AppSettings currentSettings();

signals:
    // 设置变更信号
    void settingsChanged(const AppSettings &newSettings);

private:
    void initUI();
    void initPages();
    void registerPage(SettingsPage *page);

    QListWidget *m_navigationList;
    QStackedWidget *m_stackedSettings;

    // 当前设置
    QHash<QString, SettingsPage*> m_pages;
    AppSettings m_currentSettings;
};

#endif // SETTINGWIDGET_H
