#ifndef PROJECTMANAGER_H
#define PROJECTMANAGER_H

#include <QObject>
#include "metactk.h"
#include "sql_table_types.h"

class FileManager;
class DatabaseManager;
class ProjectManager : public QObject
{
    Q_OBJECT
public:
    // 单例模式
    static ProjectManager* getProjectManager()
    {
        static ProjectManager p;
        return &p;
    }
    // 删除拷贝构造函数和赋值运算符
    ProjectManager(const ProjectManager&) = delete;
    ProjectManager& operator=(const ProjectManager&) = delete;

    void handleConnect();
    // 通用接口
    MetaCtk *getMetaCtk(const QString& configPath);
    void releaseMetaCtk(const QString& configPath);

    // ======== FileManager ========
    // == 核心操作接口 ==
    Node createProject(const QString& destDir, int id = 0);
    Node createCategory(const QString& destDir, int id = 0);
    bool removeItem(const QString &path, int id);

    // == 验证接口 ==
    bool isRepositoryItem(const QString& destDir);
    bool isProject(const QString& destDir);
    bool isCategory(const QString& destDir);
    bool hasNameRepetition(const QString& name, const QString& destDir);
    bool isCodeFile(const QString& filePath);
    bool isImageFile(const QString& filePath);
    bool hasProjectMarker(const QString& destDir);
    bool hasCategoryMarker(const QString& destDir);

    // == 工具接口 ==
    QString autoRename(const QString& name, const QString& path);
    bool renameItem(const QString& newName, const QString& path, int id);
    QString sanitizeFileName(const QString &fileName);

    QString repositoryIdFile() const;
    QString repositoryIdDir() const;
    QString repositoryId() const;

    // ======== DatabaseManager ========
    DatabaseManager *getDbManager() const;
//    Node nodeByParentIdAndName(int parentId, const QString& name);
//    QVector<Node> nodesByName(const QString& name);

    // 根节点
    int rootNodeId() const;
    void setRootNodeId(int id);


    // 清理缓存
    void clearCache(const QString& configPath);
    void clearAllCache();

    // 设置缓存大小限制
    void setCacheLimit(int limit) { m_cacheLimit = limit; }
    int cacheLimit() const { return m_cacheLimit; }

signals:
    void projectListChanged(); // 项目列表变化信号

private:
    explicit ProjectManager(QObject *parent = nullptr);
    ~ProjectManager();

    // 清理最不常用的缓存项
    void cleanupCache();

    // 更新缓存项的访问时间
    void updateAccessTime(const QString& configPath);

    QMap<QString, MetaCtk*> m_metaCtks; // 缓存最近使用的MetaCtk实例
    QMap<QString, QDateTime> m_accessTimes; // 记录每个缓存项的最后访问时间
    int m_cacheLimit = 20; // 缓存大小限制

    int m_rootNodeId = 0;
    FileManager *m_fileManager = nullptr;
    DatabaseManager *m_dbManager = nullptr;
};

#endif // PROJECTMANAGER_H
