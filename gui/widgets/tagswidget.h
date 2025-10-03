#ifndef TAGSWIDGET_H
#define TAGSWIDGET_H

/*****************************************************
*
* @file     tagswidget.h
* @brief    TagsWidget类
*
* @description
*           ==== 布局 ====
*           默认只有一个加号按钮，点击后添加tagGroup，按钮下移，并在tagGroup右侧添加一个按钮，用于添加tag
*           效果：group1 (按钮+)
*               (按钮+)
*           ==== 核心功能 ====
*           ==== 使用说明 ====
*           ==== 注意 ====
*
* @author   无声目
* @date     2025/09/27
* @history
*****************************************************/

#include <QWidget>
#include <QMap>

class QVBoxLayout;
class FlowLayout;
class QToolButton;
class TagsWidget : public QWidget
{
    Q_OBJECT
public:
    explicit TagsWidget(QWidget *parent = nullptr);

    void setTags(const QMap<QString, QStringList> &tags);
//    void setNodeId(int id);

    QMap<QString, QStringList> tags() const;

    void updateTagUsageInfo();

signals:
    void tagsChanged(const QMap<QString, QStringList> &tags);
    void tagClicked(QToolButton *tag, const QString &groupName);
    void tagRemovedFromGroup(const QString &groupName, const QString &tag);
    void tagGroupRemoved(const QString &groupName);

private slots:
    void addTagGroup();
    void addTagToGroup(const QString &groupName);
    void removeTagGroup(const QString &groupName);
    void removeTagFromGroup(const QString &groupName, const QString &tag);

private:
    void initUI();
    void updateContent();
    void updateAddButtonPosition();
    void setTagUsageInfo();
    QToolButton *createGroupButton(const QString &groupName);
    QToolButton *createTagButton(const QString &tag, const QString &groupName);
    QToolButton *createAddTagButton(const QString &groupName);

    QVBoxLayout *m_mainVLayout = nullptr;
    QToolButton *m_addGroupButton = nullptr;
    QMap<QString, QStringList> m_tags;
    QMap<QString, FlowLayout*> m_groupLayouts;  // 组名到流式布局的映射
    QMap<QString, QToolButton*> m_addTagBtn;
//    int m_nodeId = 0;
    QMap<QString, int> m_tagUsageCount; // "GroupName::TagName" -> 标签关联笔记个数

    const int fixHeight = 26;
};

#endif // TAGSWIDGET_H
