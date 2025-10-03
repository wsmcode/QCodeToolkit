#ifndef GENERALSETPAGE_H
#define GENERALSETPAGE_H

/*****************************************************
*
* @file     generalsetpage.h
* @brief    GeneralSetPage类
*
* @description
*           ==== 布局 ====
*           ==== 核心功能 ====
*           6. 通用
*           	1. 语言：下拉框选择
*           	2. 更新：检查更新按钮
*           	3. 全局快捷键：一个网格布局（第一列是动作：QLabel，第二列是快捷键：QLineEdit，第三列是恢复默认：按钮）
*                   1. 最后一行第一列：按钮（内置快捷键）：弹出一个列表布局，第一列是动作：QLabel，第二列是快捷键：QLabel
*           	4. 开发者设置
*           		1. 日志开关
*           		2. 日志等级
*           ==== 使用说明 ====
*           ==== 注意 ====
*
* @author   无声目
* @date     2025/09/23
* @history
*****************************************************/
#include "settingspage.h"

class Dialog;
class ComboBox;
class QCheckBox;
class QPushButton;
class GeneralSetPage : public SettingsPage
{
    Q_OBJECT
public:
    explicit GeneralSetPage(QWidget *parent = nullptr);

    QString title() const override;

    void load(const AppSettings &) override;
    void save(AppSettings &) override;

signals:

private:
    void initUI() override;
    void initConnections();

    ComboBox *m_languageComboBox;
    QPushButton *m_checkUpdateButton;
    QCheckBox *m_logEnableCheckBox;
    ComboBox *m_logLevelComboBox;
};

#endif // GENERALSETPAGE_H
