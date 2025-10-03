#ifndef CODESETPAGE_H
#define CODESETPAGE_H

/*****************************************************
*
* @file     codesetpage.h
* @brief    CodeSetPage类
*
* @description
*           ==== 布局 ====
*           ==== 核心功能 ====
*           3. 代码
*           	1. 默认缩进：下拉框选择
*           	2. 默认代码语言：下拉框选择
*           	3. 使用成对符号：复选框控制
*           	4. 代码行号：复选框控制
*           	5. 设置片段
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
class QPushButton;
class QTableWidget;
class QPlainTextEdit;
class CodeSetPage : public SettingsPage
{
    Q_OBJECT
public:
    explicit CodeSetPage(QWidget *parent = nullptr);

    QString title() const override;

    void load(const AppSettings &) override;
    void save(AppSettings &) override;

signals:

private:
    void initUI() override;
    void initConnections();

    bool isValidTrigger(const QString &trigger);

    QVector<Snippet> m_snippets; // 存储片段数据

    QPushButton *m_addSnippetBtn;
    QPushButton *m_removeSnippetBtn;
    QTableWidget *m_snippetsTable;
    QPlainTextEdit *m_snippetsEdit;
    QPushButton *m_applySnippetBtn;

    ComboBox *m_indentComboBox;
    ComboBox *m_languageComboBox;
    QCheckBox *m_pairSymbolsCheckBox;
    QCheckBox *m_lineNumbersCheckBox;
};

#endif // CODESETPAGE_H
