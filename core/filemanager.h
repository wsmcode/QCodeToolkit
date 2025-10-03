#ifndef FILEMANAGER_H
#define FILEMANAGER_H
/*****************************************************
*
* @file     filemanager.h
* @brief    文件管理类（单例）：
*
* @description
* ========================================================================
*           ====== 核心概念说明 ======
*           1. 分类目录：包含隐藏文件(.coderepo)的文件夹，用于组织项目文件
*           2. 项目文件：包含隐藏文件(.coderepo)和配置文件(meta.ctk)的文件夹
*           3. 文件限制：
*               - 同一文件夹不能同时存在普通文件夹和项目文件
*               - 每个项目文件/分类目录必须包含标识文件(.coderepo)
*           ====== 核心功能 ======
*           - 创建分类目录/项目文件
*           - 添加文件/文件夹
*           - 自动重命名处理
*           - 项目结构验证
*           - 递归删除操作
*           ====== 梳理 ======
*           1. 传入的路径一般为相对路径
*           2. 传入的路径有两种：主分类路径，默认路径（没有设置主分类）
*           3. 文件存储后应该发送信号，让数据库处理
*           ====== 使用说明 ======
*           1. 所有路径参数均支持相对路径
*           2. 添加文件时自动处理项目结构创建
*           3. 图片文件仅可作为项目演示图添加（每个项目限1张）
*           ====== 注意 ======
*           当前版本仅支持标准项目结构的文件夹操作
*           非项目结构文件夹需手动转换
*           目前没有针对磁盘文件被手动改变的处理
*
* @author   无声目
* @date     2025/08/15
* @history
*****************************************************/

#include <QObject>
#include "code_types.h"

class FileManager : public QObject
{
    Q_OBJECT
public:
    // 单例模式
    static FileManager *getFileManager()
    {
        static FileManager f;
        return &f;
    }
    // 删除拷贝构造函数和赋值运算符
    FileManager(const FileManager&) = delete;
    FileManager& operator=(const FileManager&) = delete;

    // ======== 核心操作接口 ========
    QString createProject(const QString& destDir);
    QString createCategory(const QString& destDir);
    bool importFile(const QString &sourcePath, const QString &destDir);
    bool importFolder(const QString &sourceDir, const QString &destDir);
    bool removeItem(const QString &path);

    // ======== 验证接口 ========
    bool isRepositoryItem(const QString& destDir);
    bool isProject(const QString& destDir);
    bool isCategory(const QString& destDir);
    bool hasNameRepetition(const QString& name, const QString& destDir);
    bool isCodeFile(const QString& filePath);
    bool isImageFile(const QString& filePath);
    bool hasProjectMarker(const QString& destDir);
    bool hasCategoryMarker(const QString& destDir);

    // ======== 工具接口 ========
    QString autoRename(const QString& name, const QString& path);
    bool renameItem(const QString& newName, const QString& path);
    QString sanitizeFileName(const QString &fileName);

    // ======== 查询接口 ========（用到再写）
    // 查询项目文件信息：文件大小，内有文件数量，
    //QVariantMap getProjectFileInfo(const QString& filePath);
    // 查询分类目录信息：内有部是分类目录还是项目文件，数量，大小
    //QVariantMap getClassifyDirInfo(const QString &folderPath);

    const QString REPO_ID_FILE = ".coderepof";  // 标识当前分类目录下的类型为项目文件
    const QString REPO_ID_DIR = ".coderepod";   // 标识当前分类目录下的类型为分类目录
    const QString REPO_ID = ".coderepo";        // 标识文件名称

signals:
    void fileImported(const QString& resourcePath, const QString& configPath);
    void createMetaCtk(const QString &configPath, const QString &name);

private:
    explicit FileManager(QObject *parent = nullptr);

    bool removeRecursively(const QString &path);    // 递归删除实现
    bool addRepoIdFile(const QString& path);        // 添加隐藏标识文件
    bool addRepoIdDir(const QString& path);         // 添加隐藏标识文件
    bool addRepoId(const QString& path);            // 添加隐藏标识文件

    // 默认命名配置
    QString m_defaultFileName = "untitle";
    QString m_defaultDirName = "分类";
};

#endif // FILEMANAGER_H
