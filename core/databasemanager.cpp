#include "databasemanager.h"
#include "sqldatabase.h"

#include <QDebug>
#include <QDir>

DatabaseManager::DatabaseManager(QObject *parent)
    :QObject(parent)
{
    m_db = new SQLDatabase(this);

    QObject::connect(m_db, &SQLDatabase::connectionFail, [=](){
        qCritical("Database connection failure");
    });

    QObject::connect(m_db, &SQLDatabase::connectionSuccess, [=](){
        // 测试用
        qInfo("Database connection success");
    });

    initDatabase();
}

DatabaseManager::~DatabaseManager()
{
    if(m_db)
    {
        m_db->closeDatabase();
        // 不需要手动删除，因为设置了parent(this)会自动删除
    }

    // 然后移除数据库连接
    if(QSqlDatabase::contains(QSqlDatabase::defaultConnection))
    {
        QSqlDatabase::removeDatabase(QSqlDatabase::defaultConnection);
    }
}

void DatabaseManager::init(const QString &rootPath)
{
    m_rootPath = rootPath;
}

bool DatabaseManager::initDatabase()
{
    // 首先确保数据库连接
    if(!m_db->connectToDatabase())
    {
        qCritical() << "Failed to connect to database";
        return false;
    }
    // 创建节点表
    QStringList nodeFields = {
        "id INTEGER PRIMARY KEY AUTOINCREMENT",
        "name TEXT NOT NULL",
        "parent_id INTEGER",
        "type TEXT NOT NULL",
        "created DATETIME DEFAULT CURRENT_TIMESTAMP",
        "modified DATETIME DEFAULT CURRENT_TIMESTAMP"
    };

    if(!m_db->createTable("node", nodeFields))
    {
        qCritical() << "Failed to create node table:" << m_db->lastError();
        return false;
    }
    else qInfo() << "Node table created successfully";

    // 创建笔记表
    QStringList noteFields = {
        "node_id INTEGER PRIMARY KEY",
        "project_name TEXT DEFAULT 'untitle'",
        "image_path TEXT",
        "author TEXT DEFAULT 'Unknown'",
        "uuid TEXT UNIQUE NOT NULL"
    };

    if(!m_db->createTable("note", noteFields))
    {
        qCritical() << "Failed to create note table:" << m_db->lastError();
        return false;
    }
    else qInfo() << "Note table created successfully";

    // 创建标签组表
    QStringList tagGroupFields = {
        "id INTEGER PRIMARY KEY AUTOINCREMENT",
        "name TEXT UNIQUE NOT NULL",
        "color TEXT",
        "created DATETIME DEFAULT CURRENT_TIMESTAMP"
    };

    if(!m_db->createTable("tag_groups", tagGroupFields))
    {
        qCritical() << "Failed to create tag_groups table:" << m_db->lastError();
        return false;
    }
    else qInfo() << "Tag groups table created successfully";

    // 创建标签表
    QStringList tagFields = {
        "id INTEGER PRIMARY KEY AUTOINCREMENT",
        "name TEXT NOT NULL",
        "group_id INTEGER",
        "color TEXT",
        "created DATETIME DEFAULT CURRENT_TIMESTAMP"
    };

    if(!m_db->createTable("tags", tagFields))
    {
        qCritical() << "Failed to create tags table:" << m_db->lastError();
        return false;
    }
    else qInfo() << "Tags table created successfully";

    // 创建笔记标签关联表
    QStringList noteTagFields = {
        "id INTEGER PRIMARY KEY AUTOINCREMENT",
        "note_id INTEGER NOT NULL",
        "tag_id INTEGER NOT NULL",
        "created DATETIME DEFAULT CURRENT_TIMESTAMP"
    };

    if(!m_db->createTable("note_tags", noteTagFields))
    {
        qCritical() << "Failed to create note_tags table:" << m_db->lastError();
        return false;
    }
    else qInfo() << "Note tags table created successfully";

    // 创建设置表
    QStringList settingsFields = {
        "key TEXT PRIMARY KEY",
        "value TEXT NOT NULL",
        "category TEXT NOT NULL",
        "modified DATETIME DEFAULT CURRENT_TIMESTAMP",
        "data_type TEXT DEFAULT 'string'"
    };

    if(!m_db->createTable("settings", settingsFields))
    {
        qCritical() << "Failed to create settings table:" << m_db->lastError();
        return false;
    }
    else qInfo() << "Settings table created successfully";

    // 创建索引
    QStringList indexes = {
        "CREATE INDEX IF NOT EXISTS idx_node_parent_id ON node(parent_id)",
        "CREATE INDEX IF NOT EXISTS idx_node_type ON node(type)",
        "CREATE INDEX IF NOT EXISTS idx_note_uuid ON note(uuid)",
        "CREATE INDEX IF NOT EXISTS idx_tags_group_id ON tags(group_id)",
        "CREATE INDEX IF NOT EXISTS idx_note_tags_note_id ON note_tags(note_id)",
        "CREATE INDEX IF NOT EXISTS idx_note_tags_tag_id ON note_tags(tag_id)",
        "CREATE INDEX IF NOT EXISTS idx_settings_category ON settings(category)",
        "CREATE INDEX IF NOT EXISTS idx_settings_key ON settings(key)"
    };

    for(const QString &indexSql : indexes)
    {
        if(m_db->executeQuery(indexSql).isEmpty() && !m_db->lastError().isEmpty())
        {
            qWarning() << "Failed to create index:" << m_db->lastError();
            // 索引创建失败不是致命错误，继续执行
        }
    }

    qInfo() << "Database initialized successfully";
    return true;
}

int DatabaseManager::addNode(const Node &node)
{
//    QStringList fields = {"name", "parent_id", "type"};
//    QStringList values = {
//        node.name,
//        QString::number(node.parentId),
//        (node.type == NodeType::NOTE) ? "note" : "catalog"
//    };

//    return m_db->insertValues("node", fields, values);
    // insertValue函数 两种均可
    QMap<QString, QVariant> data;
    data["name"] = node.name;
    data["parent_id"] = node.parentId;
    data["type"] = (node.type == NodeType::Note) ? "note" : "catalog";

    return m_db->insertValues("node", data);
}

bool DatabaseManager::updateNode(const Node &node)
{
    QString setClause = QString("name='%1', parent_id=%2, type='%3', modified=CURRENT_TIMESTAMP")
            .arg(node.name).arg(node.parentId).arg((node.type == NodeType::Note) ? "note" : "catalog");

    QString whereClause = "id=" + QString::number(node.id);

    return m_db->updateValues("node", setClause, whereClause);
}

bool DatabaseManager::deleteNode(int nodeId)
{
    // 收集所有需要删除的节点ID（包括子节点）
    QVector<int> nodesToDelete;
    collectNodesToDelete(nodeId, nodesToDelete);
    // 开始事务，确保所有操作原子性
    if(!m_db->beginTransaction())
    {
        qCritical() << "Failed to begin transaction for node deletion\n" << m_db->lastError();
        return false;
    }
    bool success = true;

    try
    {
        // 批量删除笔记标签关联、笔记和节点
        for(int id : nodesToDelete)
        {
            // 删除相关的笔记标签关联
            if(!m_db->deleteValues("note_tags", "note_id=" + QString::number(id)))
            {
                success = false;
                break;
            }

            // 删除相关的笔记
            if(!m_db->deleteValues("note", "node_id=" + QString::number(id)))
            {
                success = false;
                break;
            }

            // 删除节点本身
            if(!m_db->deleteValues("node", "id=" + QString::number(id)))
            {
                success = false;
                break;
            }
        }

        if(success)
        {
            if(!m_db->commitTransaction())
            {
                qCritical() << "Failed to commit transaction for node deletion";
                success = false;
            }
        }
        else
        {
            m_db->rollbackTransaction();
        }
    }
    catch (...)
    {
        m_db->rollbackTransaction();
        success = false;
    }

    return success;
}

Node DatabaseManager::node(int nodeId)
{
    QStringList fields = {"id", "name", "parent_id", "type", "created", "modified"};
    QString filter = "id=" + QString::number(nodeId);

    QVector<QVector<QVariant>> results = m_db->selectTable("node", fields, filter);

    if(results.isEmpty()) return Node();

    return nodeFromQueryResult(results.first());
}

QVector<Node> DatabaseManager::nodesByName(const QString &name)
{
    QStringList fields = {"id", "name", "parent_id", "type", "created", "modified"};
    QString filter = "name='" + name + "'";

    QVector<QVector<QVariant>> results = m_db->selectTable("node", fields, filter);

    QVector<Node> nodes;
    for(const QVector<QVariant> &result : results)
    {
        nodes.append(nodeFromQueryResult(result));
    }

    return nodes;
}

QVector<Node> DatabaseManager::nodesByParent(int parentId)
{
    QStringList fields = {"id", "name", "parent_id", "type", "created", "modified"};
    QString filter = "parent_id=" + QString::number(parentId);

    QVector<QVector<QVariant>> results = m_db->selectTable("node", fields, filter);

    QVector<Node> nodes;
    for(const QVector<QVariant> &result : results)
    {
        nodes.append(nodeFromQueryResult(result));
    }

    return nodes;
}

QVector<Node> DatabaseManager::nodesByType(NodeType type)
{
    QStringList fields = {"id", "name", "parent_id", "type", "created", "modified"};
    QString typeStr = (type == NodeType::Note) ? "note" : "catalog";
    QString filter = "type='" + typeStr + "'";

    QVector<QVector<QVariant>> results = m_db->selectTable("node", fields, filter);

    QVector<Node> nodes;
    for(const QVector<QVariant> &result : results)
    {
        nodes.append(nodeFromQueryResult(result));
    }
    return nodes;
}

bool DatabaseManager::addNote(const Note &note)
{
    QStringList fields = {"node_id", "project_name", "image_path", "author", "uuid"};
    QStringList values = {
        QString::number(note.nodeId),
        note.projectName,
        note.imagePath,
        note.author,
        note.uuid
    };

    return m_db->insertValues("note", fields, values);
}

bool DatabaseManager::updateNote(const Note &note)
{
    QString setClause = QString("project_name='%1', image_path='%2', author='%3', uuid='%4'")
            .arg(note.projectName).arg(note.imagePath).arg(note.author).arg(note.uuid);

    QString whereClause = "node_id=" + QString::number(note.nodeId);

    return m_db->updateValues("note", setClause, whereClause);
}

bool DatabaseManager::deleteNote(int nodeId)
{
    // 先删除相关的笔记标签关联
    m_db->deleteValues("note_tags", "note_id=" + QString::number(nodeId));

    // 删除笔记
    return m_db->deleteValues("note", "node_id=" + QString::number(nodeId));
}

Note DatabaseManager::note(int nodeId)
{
    QStringList fields = {"node_id", "project_name", "image_path", "author", "uuid"};
    QString filter = "node_id=" + QString::number(nodeId);

    QVector<QVector<QVariant>> results = m_db->selectTable("note", fields, filter);

    if(results.isEmpty()) return Note();

    return noteFromQueryResult(results.first());
}

Note DatabaseManager::noteByUuid(const QString &uuid)
{
    QStringList fields = {"node_id", "project_name", "image_path", "author", "uuid"};
    QString filter = "uuid='" + uuid + "'";

    QVector<QVector<QVariant>> results = m_db->selectTable("note", fields, filter);

    if(results.isEmpty()) return Note();

    return noteFromQueryResult(results.first());
}

QVector<Note> DatabaseManager::allNotes()
{
    QStringList fields = {"node_id", "project_name", "image_path", "author", "uuid"};

    QVector<QVector<QVariant>> results = m_db->selectTable("note", fields, "");

    QVector<Note> notes;
    for(const QVector<QVariant> &result : results)
    {
        notes.append(noteFromQueryResult(result));
    }
    return notes;
}

QVector<Note> DatabaseManager::notesByName(const QString &name)
{
    QStringList fields = {"node_id", "project_name", "image_path", "author", "uuid"};
    QString filter = "project_name='" + name + "'";

    QVector<QVector<QVariant>> results = m_db->selectTable("note", fields, filter);

    QVector<Note> notes;
    for(const QVector<QVariant> &result : results)
    {
        notes.append(noteFromQueryResult(result));
    }
    return notes;
}

QVector<Note> DatabaseManager::searchNotesByName(const QString &name)
{
    QVector<Note> notes;
    if(name.isEmpty()) return notes;

    QStringList fields = {"node_id", "project_name", "image_path", "author", "uuid"};
    QString filter = "project_name LIKE '%" + name + "%'";

    QVector<QVector<QVariant>> results = m_db->selectTable("note", fields, filter);

    for(const QVector<QVariant> &result : results)
    {
        notes.append(noteFromQueryResult(result));
    }

    return notes;
}

int DatabaseManager::addTagGroup(const TagGroup &tagGroup)
{
    QStringList fields = {"name", "color"};
    QStringList values = {tagGroup.name, tagGroup.color};

    return m_db->insertValues("tag_groups", fields, values);
}

bool DatabaseManager::updateTagGroup(const TagGroup &tagGroup)
{
    QString setClause = QString("name='%1', color='%2'").arg(tagGroup.name, tagGroup.color);
    QString whereClause = "id=" + QString::number(tagGroup.id);

    return m_db->updateValues("tag_groups", setClause, whereClause);
}

bool DatabaseManager::deleteTagGroup(int groupId)
{
    // 开始事务
    if(!m_db->beginTransaction())
    {
        qCritical() << "Failed to begin transaction for tag group deletion";
        return false;
    }
    bool success = true;

    try
    {
        // 获取组内的所有标签
        QVector<Tag> tagsInGroup = tagsByGroup(groupId);

        // 先删除所有标签的笔记关联
        for(const Tag &tag : tagsInGroup)
        {
            // 删除该标签的所有笔记关联
            if(!m_db->deleteValues("note_tags", "tag_id=" + QString::number(tag.id)))
            {
                success = false;
                break;
            }
        }
        if(success)
        {
            // 删除组内的所有标签
            if(!m_db->deleteValues("tags", "group_id=" + QString::number(groupId)))
                success = false;
        }
        if(success)
        {
            // 删除标签组
            if(!m_db->deleteValues("tag_groups", "id=" + QString::number(groupId)))
                success = false;
        }

        if(success)
        {
            if(!m_db->commitTransaction())
            {
                qCritical() << "Failed to commit transaction for tag group deletion";
                success = false;
            }
        }
        else m_db->rollbackTransaction();
    }
    catch (...)
    {
        m_db->rollbackTransaction();
        success = false;
    }

    return success;
}

TagGroup DatabaseManager::tagGroup(int groupId)
{
    QStringList fields = {"id", "name", "color", "created"};
    QString filter = "id=" + QString::number(groupId);

    QVector<QVector<QVariant>> results = m_db->selectTable("tag_groups", fields, filter);

    if(results.isEmpty()) return TagGroup();

    return tagGroupFromQueryResult(results.first());
}

QVector<TagGroup> DatabaseManager::allTagGroups()
{
    QStringList fields = {"id", "name", "color", "created"};

    QVector<QVector<QVariant>> results = m_db->selectTable("tag_groups", fields, "");

    QVector<TagGroup> tagGroups;
    for(const QVector<QVariant> &result : results)
    {
        tagGroups.append(tagGroupFromQueryResult(result));
    }

    return tagGroups;
}

TagGroup DatabaseManager::tagGroupByName(const QString &name)
{
    QStringList fields = {"id", "name", "color", "created"};
    QString filter = "name='" + name + "'";

    QVector<QVector<QVariant>> results = m_db->selectTable("tag_groups", fields, filter);

    if(results.isEmpty()) return TagGroup();

    return tagGroupFromQueryResult(results.first());
}

int DatabaseManager::addTag(const Tag &tag)
{
    QStringList fields = {"name", "group_id", "color"};
    QStringList values = {
        tag.name,
        QString::number(tag.groupId),
        tag.color
    };

    return m_db->insertValues("tags", fields, values);
}

bool DatabaseManager::updateTag(const Tag &tag)
{
    QString setClause = QString("name='%1', group_id=%2, color='%3'")
            .arg(tag.name).arg(tag.groupId).arg(tag.color);

    QString whereClause = "id=" + QString::number(tag.id);

    return m_db->updateValues("tags", setClause, whereClause);
}

bool DatabaseManager::deleteTag(int tagId)
{
    // 先删除相关的笔记标签关联
    m_db->deleteValues("note_tags", "tag_id=" + QString::number(tagId));

    // 删除标签
    return m_db->deleteValues("tags", "id=" + QString::number(tagId));
}

Tag DatabaseManager::tag(int tagId)
{
    QStringList fields = {"id", "name", "group_id", "color", "created"};
    QString filter = "id=" + QString::number(tagId);

    QVector<QVector<QVariant>> results = m_db->selectTable("tags", fields, filter);

    if(results.isEmpty()) return Tag();

    return tagFromQueryResult(results.first());
}

QVector<Tag> DatabaseManager::tagsByGroup(int groupId)
{
    QStringList fields = {"id", "name", "group_id", "color", "created"};
    QString filter = "group_id=" + QString::number(groupId);

    QVector<QVector<QVariant>> results = m_db->selectTable("tags", fields, filter);

    QVector<Tag> tags;
    for(const QVector<QVariant> &result : results)
    {
        tags.append(tagFromQueryResult(result));
    }

    return tags;
}

Tag DatabaseManager::tagByNameAndGroup(const QString &name, int groupId)
{
    QStringList fields = {"id", "name", "group_id", "color", "created"};
    QString filter = "name='" + name + "' AND group_id=" + QString::number(groupId);

    QVector<QVector<QVariant>> results = m_db->selectTable("tags", fields, filter);

    if(results.isEmpty()) return Tag();
    return tagFromQueryResult(results.first());
}

bool DatabaseManager::addNoteTag(int noteId, int tagId)
{
    QStringList fields = {"note_id", "tag_id"};
    QStringList values = {QString::number(noteId), QString::number(tagId)};

    return m_db->insertValues("note_tags", fields, values) > 0;
}

bool DatabaseManager::removeNoteTag(int noteId, int tagId)
{
    return m_db->deleteValues("note_tags",
                             "note_id=" + QString::number(noteId) +
                             " AND tag_id=" + QString::number(tagId));
}

QVector<Tag> DatabaseManager::tagsForNote(int noteId)
{
    QString query = QString(R"(
                            SELECT t.id, t.name, t.group_id, t.color, t.created
                            FROM tags t
                            JOIN note_tags nt ON t.id = nt.tag_id
                            WHERE nt.note_id = %1)").arg(noteId);

    QVector<QVector<QVariant>> results = m_db->executeQuery(query);

    QVector<Tag> tags;
    for(const QVector<QVariant> &result : results)
    {
        tags.append(tagFromQueryResult(result));
    }

    return tags;
}

QVector<Note> DatabaseManager::notesForTag(int tagId)
{
    QString query = QString(R"(
                            SELECT n.node_id, n.project_name, n.image_path, n.author, n.uuid
                            FROM note n
                            JOIN note_tags nt ON n.node_id = nt.note_id
                            WHERE nt.tag_id = %1)").arg(tagId);

    QVector<QVector<QVariant>> results = m_db->executeQuery(query);

    QVector<Note> notes;
    for(const QVector<QVariant> &result : results)
    {
        notes.append(noteFromQueryResult(result));
    }

    return notes;
}

QString DatabaseManager::lastError() const
{
    return m_db->lastError();
}

QString DatabaseManager::getNodeFullPath(int nodeId)
{
    if(nodeId <= 0) return "";

    // 递归获取节点的完整路径
    QStringList pathParts;
    int currentId = nodeId;

    while(currentId > 0)
    {
        Node node = this->node(currentId);
        if(node.id == 0) break; // 无效节点
        if(node.parentId == -1) break;

        pathParts.prepend(node.name);
        currentId = node.parentId;
    }

    if(pathParts.isEmpty()) return "";

    // 构建完整路径
    QDir baseDir(m_rootPath);
    for(const QString &part : pathParts)
    {
        if(!baseDir.cd(part))
        {
            qWarning() << "Failed to navigate to directory:" << part;
            return "";
        }
    }

    return baseDir.absolutePath();
}

bool DatabaseManager::addSetting(const Settings &setting)
{
    QMap<QString, QVariant> data;
    data["key"] = setting.key;
    data["value"] = setting.value;
    data["category"] = setting.category;
    data["data_type"] = setting.dataType;

    return m_db->insertValues("settings", data) > 0;
}

bool DatabaseManager::updateSetting(const Settings &setting)
{
    QString setClause = QString("value='%1', category='%2', data_type='%3', modified=CURRENT_TIMESTAMP")
            .arg(setting.value, setting.category, setting.dataType);
    QString whereClause = "key='" + setting.key + "'";

    return m_db->updateValues("settings", setClause, whereClause);
}

bool DatabaseManager::deleteSetting(const QString &key)
{
    return m_db->deleteValues("settings", "key='" + key + "'");
}

Settings DatabaseManager::setting(const QString &key)
{
    QStringList fields = {"key", "value", "category", "modified", "data_type"};
    QString filter = "key='" + key + "'";

    QVector<QVector<QVariant>> results = m_db->selectTable("settings", fields, filter);

    if(results.isEmpty()) return Settings();
    return settingFromQueryResult(results.first());
}

QVector<Settings> DatabaseManager::settingsByCategory(const QString &category)
{
    QStringList fields = {"key", "value", "category", "modified", "data_type"};
    QString filter = "category='" + category + "'";

    QVector<QVector<QVariant>> results = m_db->selectTable("settings", fields, filter);

    QVector<Settings> settings;
    for(const QVector<QVariant> &result : results)
    {
        settings.append(settingFromQueryResult(result));
    }
    return settings;
}

QVector<Settings> DatabaseManager::allSettings()
{
    QStringList fields = {"key", "value", "category", "modified", "data_type"};

    QVector<QVector<QVariant>> results = m_db->selectTable("settings", fields, "");

    QVector<Settings> settings;
    for(const QVector<QVariant> &result : results)
    {
        settings.append(settingFromQueryResult(result));
    }
    return settings;
}

Node DatabaseManager::nodeFromQueryResult(const QVector<QVariant> &result)
{
    Node node;
    if(result.size() >= 6)
    {
        node.id = result[0].toInt();
        node.name = result[1].toString();
        node.parentId = result[2].toInt();
        node.type = (result[3].toString() == "note") ? NodeType::Note : NodeType::Catalog;
        node.created = result[4].toDateTime();
        node.modified = result[5].toDateTime();
    }
    return node;
}

Note DatabaseManager::noteFromQueryResult(const QVector<QVariant> &result)
{
    Note note;
    if(result.size() >= 5)
    {
        note.nodeId = result[0].toInt();
        note.projectName = result[1].toString();
        note.imagePath = result[2].toString();
        note.author = result[3].toString();
        note.uuid = result[4].toString();
    }
    return note;
}

TagGroup DatabaseManager::tagGroupFromQueryResult(const QVector<QVariant> &result)
{
    TagGroup tagGroup;
    if(result.size() >= 4)
    {
        tagGroup.id = result[0].toInt();
        tagGroup.name = result[1].toString();
        tagGroup.color = result[2].toString();
        tagGroup.created = result[3].toDateTime();
    }
    return tagGroup;
}

Tag DatabaseManager::tagFromQueryResult(const QVector<QVariant> &result)
{
    Tag tag;
    if(result.size() >= 5)
    {
        tag.id = result[0].toInt();
        tag.name = result[1].toString();
        tag.groupId = result[2].toInt();
        tag.color = result[3].toString();
        tag.created = result[4].toDateTime();
    }
    return tag;
}

Settings DatabaseManager::settingFromQueryResult(const QVector<QVariant> &result)
{
    Settings setting;
    if(result.size() >= 5)
    {
        setting.key = result[0].toString();
        setting.value = result[1].toString();
        setting.category = result[2].toString();
        setting.modified = result[3].toDateTime();
        setting.dataType = result[4].toString();
    }
    return setting;
}

void DatabaseManager::collectNodesToDelete(int nodeId, QVector<int> &nodesToDelete)
{
    nodesToDelete.append(nodeId);

    // 获取所有子节点
    QVector<Node> children = nodesByParent(nodeId);
    for(const Node &child : children)
    {
        collectNodesToDelete(child.id, nodesToDelete);
    }
}
