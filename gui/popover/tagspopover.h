#ifndef TAGSPOPOVER_H
#define TAGSPOPOVER_H

/*****************************************************
*
* @file     tagspopover.h
* @brief    TagsPopover类
*
* @description
*           ==== 布局 ====
*           使用自定义布局
*           ==== 核心功能 ====
*           ==== 使用说明 ====
*           通过静态函数使用浮窗
*           参数：
*           "父控件"QWidget* parent、
*           "需响应控件"QWidget* responded、
*           "一组标签/标签组名"QStringList、
*           "lineEdit的占位符"QString placeholderText
*           ==== 注意 ====
*           此类专供TagsWidget，为“添加标签按钮”和“添加标签组按钮”提供浮窗
*           这些按钮共用一个TagsPopover类（防止TagsPopover新建太多）
*
* @author   无声目
* @date     2025/09/18
* @history
*****************************************************/

#include "popoverwidget.h"

class QLabel;
class QLineEdit;
class QToolButton;
class FlowLayout;
class QVBoxLayout;
class TagsPopover : public PopoverWidget
{
    Q_OBJECT
public:
    explicit TagsPopover(QWidget *parent = nullptr);

    static QString getText(QWidget *parent, QWidget *respondedWidget,
                           const QStringList &existingItems, const QStringList &allItems);

signals:
    void textSelected(const QString &text);

private slots:
    // 处理添加按钮点击
    void onAddButtonClicked();

private:
    void initUI();
    void setupTagsButtons(const QStringList &allItems, const QStringList &existingItems);
    bool isValidInput(const QString &input); // 检查输入是否有效

    QWidget *m_contentWidget;
    QVBoxLayout *m_mainVLayout;
    FlowLayout *m_flowLayout;

    QList<QToolButton*> m_tagsButtons;
    QLineEdit *m_newNameEdit;
    QToolButton *m_addButton;
    QLabel *m_errorLabel;

    QStringList m_allItems;
    QStringList m_existingItems;

    const int m_fixHeight = 24;
};

#endif // TAGSPOPOVER_H
