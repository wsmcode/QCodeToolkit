#include "tagonlynotetab.h"

#include <QMessageBox>
#include <databasemanager.h>

TagOnlyNoteTab::TagOnlyNoteTab(QWidget *parent) : NoteTab("", parent)
{
    load();
    qInfo() << "TagLayoutCodeEditorTab initialized successfully";
}

void TagOnlyNoteTab::load()
{
    loadAllTags();
    updateContent();
    m_isSaved = true;
    emit savedChanged(true);
}

void TagOnlyNoteTab::save()
{
    if(m_isSaved) return;
    saveAllTags();

    m_isSaved = true;
    emit savedChanged(true);
    qInfo() << "All tags saved successfully";
}

void TagOnlyNoteTab::onAllTagsChanged(const QMap<QString, QStringList> &tags)
{
    m_codeNote.tags = tags;
    m_isSaved = false;
    emit savedChanged(false);
}

void TagOnlyNoteTab::onTagGroupRemoved(const QString &groupName)
{
    DatabaseManager *db = DatabaseManager::getDatabaseManager();

    // 获取标签组
    TagGroup group = db->tagGroupByName(groupName);
    if(group.isEmpty())
    {
        qWarning() << "Tag group not found:" << groupName;
        return;
    }

    // 获取组内的所有标签
    QVector<Tag> tags = db->tagsByGroup(group.id);

    // 检查组内标签的使用情况
    int totalUsage = 0;
    for(const Tag &tag : tags)
    {
        QVector<Note> notesUsingTag = db->notesForTag(tag.id);
        totalUsage += notesUsingTag.size();
    }

    if(totalUsage > 0)
    {
        // 弹出确认对话框
        QMessageBox::StandardButton reply;
        reply = QMessageBox::question(this,
                                    "确认删除标签组",
                                    QString("标签组 '%1' 中的标签正在被 %2 个笔记使用。\n确定要删除整个标签组吗？")
                                      .arg(groupName).arg(totalUsage),
                                    QMessageBox::Yes | QMessageBox::No);

        if(reply == QMessageBox::No) return;
    }

    // 删除标签组
    if(db->deleteTagGroup(group.id))
    {
        qDebug() << "Tag group" << groupName << "deleted successfully";
        // 从本地数据中移除
        m_codeNote.tags.remove(groupName);
        updateContent();
    }
    else qWarning() << "Failed to delete tag group:" << groupName;
}

void TagOnlyNoteTab::onTagRemovedFromGroup(const QString &groupName, const QString &tagName)
{
    DatabaseManager *db = DatabaseManager::getDatabaseManager();

    // 获取标签组
    TagGroup group = db->tagGroupByName(groupName);
    if(group.isEmpty())
    {
        qWarning() << "Tag group not found:" << groupName;
        return;
    }

    // 获取标签
    Tag tag = db->tagByNameAndGroup(tagName, group.id);
    if(tag.isEmpty())
    {
        qWarning() << "Tag not found:" << tagName << "in group:" << groupName;
        return;
    }

    // 检查标签是否被其他笔记使用
    QVector<Note> notes = db->notesForTag(tag.id);
    int usageCount = notes.size();

    if(usageCount > 0)
    {
        // 弹出确认对话框
        QMessageBox::StandardButton reply;
        reply = QMessageBox::question(this, "确认删除标签",
                                      QString("标签 '%1' 正在被 %2 个笔记使用。\n确定要删除这个标签吗？")
                                      .arg(tagName).arg(usageCount),
                                      QMessageBox::Yes | QMessageBox::No);

        if(reply == QMessageBox::No) return;
    }

    // 删除标签
    if(db->deleteTag(tag.id))
    {
        qDebug() << "Tag" << tagName << "deleted successfully";
        // 从本地数据中移除
        if(m_codeNote.tags.contains(groupName))
        {
            m_codeNote.tags[groupName].removeAll(tagName);
            if(m_codeNote.tags[groupName].isEmpty())
            {
                m_codeNote.tags.remove(groupName);
            }
        }
        updateContent();
    }
    else qWarning() << "Failed to delete tag:" << tagName;
}

void TagOnlyNoteTab::loadAllTags()
{
    DatabaseManager *db = DatabaseManager::getDatabaseManager();

    // 获取所有标签组
    QVector<TagGroup> groups = db->allTagGroups();
    QMap<QString, QStringList> allTags;

    for(const TagGroup &group : groups)
    {
        // 获取该组的所有标签
        QVector<Tag> tags = db->tagsByGroup(group.id);
        QStringList tagNames;

        for(const Tag &tag : tags)
        {
            tagNames.append(tag.name);
        }

        allTags[group.name] = tagNames;
    }

    m_codeNote.tags = allTags;
}

void TagOnlyNoteTab::saveAllTags()
{
    DatabaseManager *db = DatabaseManager::getDatabaseManager();

    // 获取数据库中的标签和标签组
    QVector<TagGroup> existingGroups = db->allTagGroups();
    QMap<QString, TagGroup> existingGroupsByName;
    for(const TagGroup &group : existingGroups)
    {
        existingGroupsByName[group.name] = group;
    }

    // 获取所有标签
    QMap<QString, Tag> existingTagsByNameAndGroup; // "GroupName::TagName" -> Tag

    for(const TagGroup &group : existingGroups)
    {
        QVector<Tag> tags = db->tagsByGroup(group.id);

        for(const Tag &tag : tags)
        {
            QString key = group.name + "::" + tag.name;
            existingTagsByNameAndGroup[key] = tag;
        }
    }

    // 处理标签组和标签的同步

    // 数据库中添加新的标签和标签组
    for(auto it = m_codeNote.tags.constBegin(); it != m_codeNote.tags.constEnd(); it++)
    {
        const QString &groupName = it.key();
        const QStringList &tags = it.value();

        // 检查标签组是否存在
        TagGroup group = db->tagGroupByName(groupName);
        if(group.isEmpty())
        {
            // 创建新标签组
            group.name = groupName;
            group.id = db->addTagGroup(group);
            if(group.id == 0)
            {
                qWarning() << "Failed to create tag group:" << groupName;
                continue;
            }
            qDebug() << "Created new tag group:" << groupName;
        }
        else existingGroupsByName.remove(groupName);

        // 处理该组内的标签
        for(const QString &tagName : tags)
        {
            Tag tag = db->tagByNameAndGroup(tagName, group.id);
            QString key = groupName + "::" + tagName;
            if(tag.isEmpty())
            {
                // 创建新标签
                tag.name = tagName;
                tag.groupId = group.id;
                tag.id = db->addTag(tag);
                if(tag.id == 0)
                {
                    qWarning() << "Failed to create tag:" << tagName << "in group:" << groupName;
                    continue;
                }
                qDebug() << "Created new tag:" << tagName << "in group:" << groupName;
            }
            else existingTagsByNameAndGroup.remove(key);
        }
    }

    // 数据库中删除标签
    for(const Tag &tag : existingTagsByNameAndGroup)
    {
        // 删除标签
        if(db->deleteTag(tag.id))
        {
            qDebug() << "Deleted tag:" << tag.name << "tag id:" << tag.id;
        }
        else
        {
            qWarning() << "Failed to delete tag:" << tag.name << "tag id:" << tag.id;
        }
    }

    // 删除不再存在的标签组和标签
    for(const TagGroup &group : existingGroupsByName)
    {
        // 删除整个标签组（会自动删除关联的标签）
        if(db->deleteTagGroup(group.id))
        {
            qDebug() << "Deleted tag group:" << group.name;
        }
        else
        {
            qWarning() << "Failed to delete tag group:" << group.name;
        }
        continue;
    }
}
