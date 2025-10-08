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
*           - 项目文件
*               1. 文件夹形式：内有meta.ctk文件，用.coderepo判断
*               2. 文件形式：以ctk为后缀（未实现）
*           - 非项目文件识别，且可加载
*           ====== 核心功能 ======
*           - 创建分类目录/项目文件
*           - 添加文件/文件夹
*           - 自动重命名处理
*           - 递归删除操作
*           ====== 梳理 ======
*           1. 传入的路径一般为相对路径
*           2. 传入的路径有两种：主分类路径，默认路径（没有设置主分类）
*           3. 文件存储后应该发送信号，让数据库处理
*           ====== 使用说明 ======
*           1. 所有路径参数均支持相对路径
*           2. 图片文件仅可作为项目演示图添加（每个项目限1张）
*           ====== 注意 ======
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
    bool removeItem(const QString &path);

    // ======== 验证接口 ========
    bool hasNameRepetition(const QString& name, const QString& destDir);
    bool isCodeFile(const QString& filePath);
    bool isImageFile(const QString& filePath);

    // ======== 工具接口 ========
    bool isProject(const QString &path);
    QString autoRename(const QString& name, const QString& path);
    bool renameItem(const QString& newName, const QString& path);
    QString sanitizeFileName(const QString &fileName);

    // ======== 查询接口 ========（用到再写）
    // 查询项目文件信息：文件大小，内有文件数量
    //QVariantMap getProjectFileInfo(const QString& filePath);
    // 查询分类目录信息：内有部是分类目录还是项目文件，数量，大小
    //QVariantMap getClassifyDirInfo(const QString &folderPath);
signals:
    void fileImported(const QString& resourcePath, const QString& configPath);
    void createMetaCtk(const QString &configPath, const QString &name);

private:
    explicit FileManager(QObject *parent = nullptr);

    bool removeRecursively(const QString &path);    // 递归删除实现

    // 默认命名配置
    QString m_defaultFileName = "untitle";
    QString m_defaultDirName = "分类";

    // 项目标识文件
    const QString m_projectIdentifier = ".coderepo";
};

#endif // FILEMANAGER_H
