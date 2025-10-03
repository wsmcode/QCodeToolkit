#ifndef TAGONLYNOTETAB_H
#define TAGONLYNOTETAB_H

/*****************************************************
*
* @file     tagonlynotetab.h
* @brief    TagOnlyNoteTab类
*
* @description
*           ==== 布局 ====
*           ==== 核心功能 ====
*           ==== 使用说明 ====
*           ==== 注意 ====
*           无需
*
* @author   无声目
* @date     2025/09/28
* @history
*****************************************************/
#include "notetab.h"

#include <QDebug>

class TagOnlyNoteTab : public NoteTab
{
    Q_OBJECT
public:
    // 单例模式
    static TagOnlyNoteTab *getTagOnlyNoteTab()
    {
        static TagOnlyNoteTab t;
        return &t;
    }
    // 删除拷贝构造函数和赋值运算符
    TagOnlyNoteTab(const TagOnlyNoteTab&) = delete;
    TagOnlyNoteTab& operator=(const TagOnlyNoteTab&) = delete;

    void load() override;
    void save() override;

signals:

private slots:
    void onAllTagsChanged(const QMap<QString, QStringList> &tags);
    void onTagGroupRemoved(const QString &groupName) override;
    void onTagRemovedFromGroup(const QString &groupName, const QString &tagName) override;
private:
    explicit TagOnlyNoteTab(QWidget *parent = nullptr);

    void initUI();
    void loadAllTags();
    void saveAllTags();
};

#endif // TAGONLYNOTETAB_H
