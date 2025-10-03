#ifndef SQL_TABLE_TYPES_H
#define SQL_TABLE_TYPES_H

#include <QString>
#include <QDateTime>
// 节点类型枚举
enum class NodeType {
    Catalog,
    Note
};

// 节点表
struct Node { // 实表和虚表共用一个数据结构
    int id;
    QString name;
    int parentId;
    NodeType type;
    QDateTime created;
    QDateTime modified;
};

// 笔记表（叶子节点）
struct Note { // 如果感觉与Node之间难以分辨改成：ProjectNote
    int nodeId;
    QString projectName;
    QString imagePath;
    QString author;
    QString uuid;

    Note() : projectName(""), imagePath(""), author(""), uuid(""){}

    bool isEmpty() const
    {
        return projectName.isEmpty() && imagePath.isEmpty() && author.isEmpty() && uuid.isEmpty();
    }
};

// 标签组表
struct TagGroup {
    int id;
    QString name;
    QString color;
    QDateTime created;
    TagGroup() : id(0), name(""), color(""){}

    bool isEmpty() const
    {
        return id == 0 && name.isEmpty() && color.isEmpty();
    }
};

// 标签表
struct Tag {
    int id;
    QString name;
    int groupId;
    QString color;
    QDateTime created;
    Tag() : id(0), name(""), groupId(0), color(""){}

    bool isEmpty() const
    {
        return id == 0 && name.isEmpty() && color.isEmpty() && groupId == 0;
    }

};

// 笔记标签关联表
struct NoteTag {
    int id;
    int noteId;
    int tagId;
    QDateTime created;
};

// 设置项表结构
struct Settings {
    QString key;
    QString value;
    QString category;
    QDateTime modified;
    QString dataType; // "string", "int", "bool", "json"

    Settings() : key(""), value(""), category(""), dataType("string")
    {
        modified = QDateTime::currentDateTime();
    }

    bool isEmpty() const
    {
        return key.isEmpty() && value.isEmpty();
    }
};

#endif // SQL_TABLE_TYPES_H
