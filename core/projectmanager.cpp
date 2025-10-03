#include "projectmanager.h"

#include "filemanager.h"
#include "databasemanager.h"
#include <QDebug>
#include <QFileInfo>

ProjectManager::ProjectManager(QObject *parent) : QObject(parent)
{
    m_fileManager = FileManager::getFileManager();
    m_dbManager = DatabaseManager::getDatabaseManager();
    handleConnect();

    // 初始化时创建根节点
    QVector<Node> rootNodes = m_dbManager->nodesByName("ROOT");
    if(rootNodes.isEmpty())
    {
        Node rootNode;
        rootNode.name = "ROOT";
        rootNode.type = NodeType::Catalog;
        rootNode.parentId = -1; // 根节点的父节点ID设为-1
        m_rootNodeId = m_dbManager->addNode(rootNode);
    }
    else m_rootNodeId = rootNodes.first().id;
}

ProjectManager::~ProjectManager()
{
    clearAllCache();
}

void ProjectManager::cleanupCache()
{
    // 如果缓存大小没有超过限制，不需要清理
    if(m_metaCtks.size() <= m_cacheLimit) return;

    // 计算需要清理的数量
    int itemsToRemove = m_metaCtks.size() - m_cacheLimit;

    // 按访问时间排序，找到最不常用的项
    QList<QPair<QDateTime, QString>> sortedItems;
    for(auto it = m_accessTimes.begin(); it != m_accessTimes.end(); it++)
    {
        sortedItems.append(qMakePair(it.value(), it.key()));
    }

    // 按访问时间升序排序（最早的在前）
    std::sort(sortedItems.begin(), sortedItems.end(),
              [](const QPair<QDateTime, QString> &a, const QPair<QDateTime, QString> &b) {
        return a.first < b.first;
    });

    // 移除最不常用的项
    for(int i = 0; i < itemsToRemove && i < sortedItems.size(); i++)
    {
        releaseMetaCtk(sortedItems[i].second);
    }
}

void ProjectManager::updateAccessTime(const QString &configPath)
{
    m_accessTimes[configPath] = QDateTime::currentDateTime();
}

void ProjectManager::handleConnect()
{
    QObject::connect(m_fileManager, &FileManager::fileImported,
                     [=](const QString& resourcePath, const QString& configPath){
        MetaCtk *metaCtk = new MetaCtk(configPath);
        if(metaCtk->load())
        {
            metaCtk->setDemoImagePath(resourcePath);
            metaCtk->save();
        }
        delete metaCtk;
    });
    QObject::connect(m_fileManager, &FileManager::createMetaCtk,
                     [=](const QString &configPath, const QString &name){
        MetaCtk *metaCtk = new MetaCtk(configPath);
        if(metaCtk->load())
        {
            metaCtk->setProjectName(name);
            metaCtk->save();
        }
        delete metaCtk;
    });
}

MetaCtk *ProjectManager::getMetaCtk(const QString& configPath)
{
    if(configPath.isEmpty()) return nullptr;
    // 如果已经缓存，直接返回
    if(m_metaCtks.contains(configPath))
    {
        updateAccessTime(configPath);
        return m_metaCtks[configPath];
    }

    // 检查缓存大小，如果超过限制则清理最不常用的项
    if(m_metaCtks.size() >= m_cacheLimit)
    {
        cleanupCache();
    }

    // 创建新实例并缓存
    MetaCtk* metaCtk = new MetaCtk(configPath);
    m_metaCtks[configPath] = metaCtk;
    updateAccessTime(configPath);

    return metaCtk;
}

void ProjectManager::releaseMetaCtk(const QString &configPath)
{
    if(m_metaCtks.contains(configPath))
    {
        delete m_metaCtks[configPath];
        m_metaCtks.remove(configPath);
        m_accessTimes.remove(configPath);
    }
}

Node ProjectManager::createProject(const QString &destDir, int parentId)
{
    // 方案一：通过参数获取父节点id
    // 方案二：通过在DatabaseManager中添加路径解析将destDir解析成父节点id

    // 如果没有指定父节点ID，使用根节点ID
    if(parentId == 0) parentId = m_rootNodeId;

    QString fileName = m_fileManager->createProject(destDir);
    QString path = destDir + "/" + fileName + "/meta.ctk";
    MetaCtk *metaCtk = new MetaCtk(path);
    Node node;
    node.name = fileName;
    node.type = NodeType::Note;
    node.parentId = parentId;
    node.id = m_dbManager->addNode(node);
    node = m_dbManager->node(node.id);

    Note note;
    note.nodeId = node.id;
    note.projectName = fileName;
    note.imagePath = metaCtk->demoImagePath();
    note.author = metaCtk->author();
    note.uuid = metaCtk->id();
    m_dbManager->addNote(note);
    delete metaCtk;

    emit projectListChanged();
    return node;
}

Node ProjectManager::createCategory(const QString &destDir, int parentId)
{
    // 如果没有指定父节点ID，使用根节点ID
    if(parentId == 0) parentId = m_rootNodeId;

    QString fileName = m_fileManager->createCategory(destDir);
    Node node;
    node.name = fileName;
    node.type = NodeType::Catalog;
    node.parentId = parentId;
    node.id = m_dbManager->addNode(node);
    node = m_dbManager->node(node.id);

    return node;
}

bool ProjectManager::removeItem(const QString &path, int id)
{
    if(!m_dbManager->deleteNode(id))
    {
        qWarning() << "Project information deletion failure in the database";
        return false;
    }

    if(!m_fileManager->removeItem(path)) return false;

    emit projectListChanged();
    return true;
}

bool ProjectManager::isRepositoryItem(const QString &destDir)
{
    return m_fileManager->isRepositoryItem(destDir);
}

bool ProjectManager::isProject(const QString &destDir)
{
    return m_fileManager->isProject(destDir);
}

bool ProjectManager::isCategory(const QString &destDir)
{
    return m_fileManager->isCategory(destDir);
}

bool ProjectManager::hasNameRepetition(const QString &name, const QString &destDir)
{
    return m_fileManager->hasNameRepetition(name, destDir);
}

bool ProjectManager::isCodeFile(const QString &filePath)
{
    return m_fileManager->isCodeFile(filePath);
}

bool ProjectManager::isImageFile(const QString &filePath)
{
    return m_fileManager->isImageFile(filePath);
}

bool ProjectManager::hasProjectMarker(const QString &destDir)
{
    return m_fileManager->hasProjectMarker(destDir);
}

bool ProjectManager::hasCategoryMarker(const QString &destDir)
{
    return m_fileManager->hasCategoryMarker(destDir);
}

QString ProjectManager::autoRename(const QString &name, const QString &path)
{
    return m_fileManager->autoRename(name, path);
}

bool ProjectManager::renameItem(const QString &newName, const QString &path, int id)
{
    if(!m_fileManager->renameItem(newName, path)) return false;
    Node node = m_dbManager->node(id);
    node.name = newName;
    m_dbManager->updateNode(node);
    Note note = m_dbManager->note(id);
    qDebug() << "new name" << newName;
    if(!note.isEmpty())
    {
        note.projectName = newName;
        qDebug() << "note projectName" << note.projectName;
        m_dbManager->updateNote(note);
    }

    note = m_dbManager->note(id);
    qDebug() << "note projectName" << note.projectName;

    emit projectListChanged();
    return true;
}

QString ProjectManager::sanitizeFileName(const QString &fileName)
{
    return m_fileManager->sanitizeFileName(fileName);
}

QString ProjectManager::repositoryIdFile() const
{
    return m_fileManager->REPO_ID_FILE;
}

QString ProjectManager::repositoryIdDir() const
{
    return m_fileManager->REPO_ID_DIR;
}

QString ProjectManager::repositoryId() const
{
    return m_fileManager->REPO_ID;
}

DatabaseManager *ProjectManager::getDbManager() const
{
    return m_dbManager;
}

int ProjectManager::rootNodeId() const
{
    return m_rootNodeId;
}

void ProjectManager::setRootNodeId(int id)
{
    m_rootNodeId = id;
}

void ProjectManager::clearCache(const QString &configPath)
{
    releaseMetaCtk(configPath);
}

void ProjectManager::clearAllCache()
{
    for(const QString& key : m_metaCtks.keys())
    {
        releaseMetaCtk(key);
    }
}
