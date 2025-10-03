#ifndef EXPORTSETPAGE_H
#define EXPORTSETPAGE_H

/*****************************************************
*
* @file     exportsetpage.h
* @brief    ExportSetPage类
*
* @description
*           ==== 布局 ====
*           ==== 核心功能 ====
*           5. 导出
*           	1. 当前版本未实现
*           ==== 使用说明 ====
*           ==== 注意 ====
*
* @author   无声目
* @date     2025/09/23
* @history
*****************************************************/
#include "settingspage.h"

class ExportSetPage : public SettingsPage
{
    Q_OBJECT
public:
    explicit ExportSetPage(QWidget *parent = nullptr);

    QString title() const override;

    void load(const AppSettings &) override;
    void save(AppSettings &) override;

signals:

private:
    void initUI() override;

};

#endif // EXPORTSETPAGE_H
