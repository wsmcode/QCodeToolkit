#ifndef DATABASEMANAGER_H
#define DATABASEMANAGER_H

/*****************************************************
*
* @file     databasemanager.h
* @brief    DatabaseManager类
*
* @description
*           ==== 核心功能 ====
*           - 数据库初始化和表结构管理
*           - 节点(目录/笔记)的CRUD操作
*           - 笔记内容管理
*           - 标签和标签组管理
*           - 笔记标签关联管理
*           - 应用程序设置管理
*           - 事务支持和错误处理
*
*           ==== 使用说明 ====
*           1. 通过单例模式获取实例: DatabaseManager::getDatabaseManager()
*           2. 调用init()方法设置根路径
*           3. 调用initDatabase()初始化数据库表结构
*           4. 使用提供的各种方法进行数据操作
*
*           ==== 注意 ====
*
* @author   无声目
* @date     2025/10/02
* @history
*****************************************************/
#include <QObject>
#include "sql_table_types.h"

class SQLDatabase;
class DatabaseManager : public QObject
{
    Q_OBJECT
public:
    // 单例模式
    static DatabaseManager *getDatabaseManager()
    {
        static DatabaseManager d;
        return &d;
    }
    // 删除拷贝构造函数和赋值运算符
    DatabaseManager(const DatabaseManager&) = delete;
    DatabaseManager& operator=(const DatabaseManager&) = delete;

    void init(const QString &rootPath);

    // 初始化数据库(创建所有表)
    bool initDatabase();

    // 节点操作
    int addNode(const Node &node);
    bool updateNode(const Node &node);
    bool deleteNode(int nodeId);
    Node node(int nodeId);
    QVector<Node> nodesByName(const QString & name);
    QVector<Node> nodesByParent(int parentId);
    QVector<Node> nodesByType(NodeType type);

    // 笔记操作
    bool addNote(const Note &note);
    bool updateNote(const Note &note);
    bool deleteNote(int nodeId);
    Note note(int nodeId);
    Note noteByUuid(const QString &uuid);
    QVector<Note> allNotes();
    QVector<Note> notesByName(const QString &name);
    QVector<Note> searchNotesByName(const QString &name);

    // 标签组操作
    int addTagGroup(const TagGroup &tagGroup);
    bool updateTagGroup(const TagGroup &tagGroup);
    bool deleteTagGroup(int groupId);
    TagGroup tagGroup(int groupId);
    QVector<TagGroup> allTagGroups();
    TagGroup tagGroupByName(const QString &name);

    // 标签操作
    int addTag(const Tag &tag);
    bool updateTag(const Tag &tag);
    bool deleteTag(int tagId);
    Tag tag(int tagId);
    QVector<Tag> tagsByGroup(int groupId);
    Tag tagByNameAndGroup(const QString &name, int groupId);

    // 笔记标签关联操作
    bool addNoteTag(int noteId, int tagId);
    bool removeNoteTag(int noteId, int tagId);
    QVector<Tag> tagsForNote(int noteId);
    QVector<Note> notesForTag(int tagId);

    // 获取最后错误信息
    QString lastError() const;

    QString getNodeFullPath(int nodeId);

    // 设置项操作
    bool addSetting(const Settings &setting);
    bool updateSetting(const Settings &setting);
    bool deleteSetting(const QString &key);
    Settings setting(const QString &key);
    QVector<Settings> settingsByCategory(const QString &category);
    QVector<Settings> allSettings();

signals:

private:
    explicit DatabaseManager(QObject *parent = nullptr);
    ~DatabaseManager();

    // 将查询结果转换为结构体
    Node nodeFromQueryResult(const QVector<QVariant> &result);
    Note noteFromQueryResult(const QVector<QVariant> &result);
    TagGroup tagGroupFromQueryResult(const QVector<QVariant> &result);
    Tag tagFromQueryResult(const QVector<QVariant> &result);
    Settings settingFromQueryResult(const QVector<QVariant> &result);

    // 递归收集所有需要删除的节点ID
    void collectNodesToDelete(int nodeId, QVector<int> &nodesToDelete);

    SQLDatabase *m_db;

    QString m_rootPath;
};

#endif // DATABASEMANAGER_H
