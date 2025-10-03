#ifndef NOTETAB_H
#define NOTETAB_H

/*****************************************************
*
* @file     notetab.h
* @brief    NoteTab类
*
* @description
*           ==== 布局 ====
*           1. 自由布局（FreeLayout）：初始只有一个TagsWidget（不可删除），可以添加单元格
*           2. 固定布局（FixedLayout）：从上至下：TagsWidget、ImageItemCell、CodeItemCell，固定不可变
*           3. 标签布局（TagLayout）：仅标签，无其他布局
*           ==== 核心功能 ====
*           ==== 使用说明 ====
*           ==== 注意 ====
*           自由布局中，单元格删除行会导致布局抖动
*
* @author   无声目
* @date     2025/09/14
* @history
*****************************************************/

#include <QWidget>
#include "code_types.h"

class QScrollArea;
class QVBoxLayout;
class TagsWidget;
class PopoverWidget;
class QToolButton;
class NoteEditor;
class MetaCtk;
class QStackedLayout;

class NoteTab : public QWidget
{
    Q_OBJECT
public:
    explicit NoteTab(const QString& configPath, QWidget *parent = nullptr);
    virtual ~NoteTab() = default;

    QString configPath() const;
    bool isSaved() const;

    virtual void load();
    virtual void save();

signals:
    void savedChanged(bool isSaved);
    void openNote(const QString &fullPath);

protected:
    void resizeEvent(QResizeEvent *event) override;

    virtual void updateContent();
    QToolButton *createToolButton();
    void syncTagsToDatabase();

    QString m_configPath;

    QWidget *m_mainContent;
    QVBoxLayout* m_mainVLayout;
    QScrollArea *m_scrollArea;
    QVBoxLayout* m_scrollVLayout;

    TagsWidget *m_tagsWidget;
    CodeNote m_codeNote;
    MetaCtk *m_metaCtk;
    bool m_isSaved = false;

    PopoverWidget *m_popover;

    // 布局堆栈
    QStackedLayout *m_stackedLayout = nullptr;

protected slots:
    virtual void onTagRemovedFromGroup(const QString &groupName, const QString &tag);
    virtual void onTagGroupRemoved(const QString &groupName);

private slots:
    void onTagClicked(QToolButton *tagButton, const QString &groupName);
    void onPopoverActivated(const QString &text);

private:
    void initNoteTabUI();
    int calculateSmartMargin(int availableWidth);
    double smoothStep(double x);


    int m_minMargin = 20;    // 最小边距
    int m_maxMargin = 150;    // 最大边距
    int m_baseWidth = 800;   // 基准宽度（在此宽度下边距为中间值）
    int m_transitionRange = 400; // 过渡范围

    const int fixHeight = 30;

    QMap<QString, int> m_nodeIdFromSuggestion;
};

#endif // NOTETAB_H
