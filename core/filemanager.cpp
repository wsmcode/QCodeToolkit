#include "filemanager.h"

#include <QDir>
#include <QTextStream>
#include <QDebug>
#include <QRegularExpression>

FileManager::FileManager(QObject *parent) : QObject(parent)
{
}

// 添加项目文件（含配置文件和隐藏文件）返回当前文件名，无效返回空
QString FileManager::createProject(const QString &destDir)
{
    QDir dir(destDir);
    if(!dir.exists())
    {
        qWarning() << "The destDir does not exist" << dir;
        return "";
    }
    if(!isCategory(destDir) && !isRepositoryItem(destDir))
    {
        qWarning() << "The destDir is not a category" << destDir;
        return "";
    }
    // == 1. 创建文件夹 ==
    QString fileName = autoRename(m_defaultFileName, destDir);
    QString filePath = dir.filePath(fileName);
    if(!dir.mkdir(fileName))
    {
        qWarning() << "Failed to create the project folder";
        return "";
    }
    // == 2. 添加隐藏文件 ==
    if(!addRepoId(filePath) || !addRepoIdFile(destDir))
    {
        dir.rmdir(fileName);// 回滚
        return "";
    }
    // == 3. 添加配置文件 ==
    QFile file(filePath + "/meta.ctk");
    if(file.open(QIODevice::WriteOnly))
    {
        file.close();
    }
    emit createMetaCtk(filePath + "/meta.ctk", fileName);

    return fileName;
}

// 添加分类空目录(仅含隐藏文件)返回当前文件名，无效返回空
QString FileManager::createCategory(const QString &destDir)
{
    QDir dir(destDir);
    if(!dir.exists())
    {
        qWarning() << "The destDir does not exist" << dir;
        return "";
    }
    if(!isCategory(destDir) && !isRepositoryItem(destDir))
    {
        qWarning() << "The destDir is not a category" << destDir;
        return "";
    }
    // == 1. 创建文件夹 ==
    QString dirName = autoRename(m_defaultDirName, destDir);
    QString dirPath = dir.filePath(dirName);
    if(!dir.mkdir(dirName))
    {
        qWarning() << "Failed to create a category";
        return "";
    }
    // == 2. 添加隐藏文件 ==
    if(!addRepoId(dirPath) || !addRepoIdDir(destDir))
    {
        dir.rmdir(dirName);// 回滚
        return "";
    }
    return dirName;
}

/***************
FileManager::imporFile添加文件处理：
    1. 源文件判断：只支持图片
        1. 图片：依据目标文件
            1. 项目文件：直接加入项目文件
            2. 其他：不理会
        2. 其他：不理会
******************/
bool FileManager::importFile(const QString &sourcePath, const QString &destDir)
{
    // ==== 获取源文件 ====
    QFileInfo srcInfo(sourcePath);
    if(!srcInfo.isFile())
    {
        qWarning() << "The sourcePath is an invalid file" << sourcePath;
        return false;
    }

    // ==== 获取目标文件夹 ====
    QDir dest(destDir);
    if(!dest.exists())
    {
        qWarning() << "The destDir does not exist" << dest;
        return false;
    }

    // ==== 文件类型验证 ====
    bool isImage = isImageFile(sourcePath);
    if(isImage)
    {
        if(isProject(destDir))
        {
            // 加入项目文件中
            emit fileImported(sourcePath, dest.filePath("meta.ctk"));
        }
        else
        {
            qWarning() << "destDir is not a valid project location" << destDir;
            return false;
        }
    }
    else
    {
        qWarning() << "The file format is not supported";
        return false;
    }
    return true;
}

// 添加文件夹/分类目录/项目文件
bool FileManager::importFolder(const QString &sourceDir, const QString &destDir)
{
    // ==== 获取源目录 ====
    QDir srcDir(sourceDir);
    if(!srcDir.exists())
    {
        qWarning() << "The sourceDir does not exist" << srcDir;
        return false;
    }
    // ==== 获取目标目录 ====
    QDir dest(destDir);
    if(!dest.exists())
    {
        qWarning() << "The destDir does not exist" << dest;
        return false;
    }
    // ==== 判断目标目录类型 ====
    // 是项目文件
    if(isProject(destDir))
    {
        // 返回上一级
        dest.cdUp();
    }

    // 如果源目录是文件夹，则根据具体文件生成分类目录并拷贝到目标目录
    if(!(isRepositoryItem(sourceDir) || isCategory(sourceDir)))
    {
        // 将文件夹转为分类目录
        // 需要多重转换和判断，且格式未必匹配
        // 检测代码文件和图片资源以及meta.ctk配置文件
        // 拷贝递归实现逻辑
        // 1. 当前目录下添加隐藏文件
        // 2. 如果为空文件夹：返回
        // 3. 文件夹下既有文件夹也有文件
        //     1. 文件：配置文件，检验配置文件把和其相关的的资源 根据配置文件整合，并更改配置文件内的路径信息
        //             资源文件，如果没有被配置文件带走，整合生成一个项目文件（配置文件同时生成）
        //             其他文件，忽略
        //     2. 文件夹：判断是否全部为文件，
        //              是，如果没有配置文件自动生成配置文件，此文件夹为项目文件
        //              否，继续递归
        //
        // 拷贝结束
        qWarning() << "Not supported in the current version.";
        //emit operationError("FileManager::addFolder: 当前版本只支持添加项目结构文件夹");
        return false;
    }
    // 源目录是分类目录或项目文件，统一处理
    QString folderName = autoRename(srcDir.dirName(), destDir);
    QString destPath = dest.filePath(folderName);
    // 创建目标目录
    if(!dest.mkdir(folderName))
    {
        qWarning() << "Failed to create the directory";
        return false;
    }
    // 复制内容
    bool success = true;
    QFileInfoList entries = srcDir.entryInfoList(QDir::AllEntries | QDir::NoDotAndDotDot | QDir::Hidden);
    for(const QFileInfo &entry : entries)
    {
        QString srcPath = entry.filePath();
        QString newDestPath = destPath + "/" + entry.fileName();

        if(entry.isDir())
        {
            success = success && importFolder(srcPath, destPath);
        }
        else success = success && QFile::copy(srcPath, newDestPath);
    }
    if(!success)
    {
        removeRecursively(destPath);
    }
    return success;
}

// 删除项目文件
bool FileManager::removeItem(const QString &path)
{
    if(!isRepositoryItem(path))
    {
        return false;
    }

    // 获取父目录路径
    QFileInfo fileInfo(path);
    QString parentDir = fileInfo.path();

    // 先删除项目
    bool success = removeRecursively(path);
    if(success)
    {
        // 检查父目录是否为空（除了隐藏标识文件）
        QDir dir(parentDir);
        if(dir.exists())
        {
            QFileInfoList entries = dir.entryInfoList(QDir::AllEntries | QDir::NoDotAndDotDot | QDir::Hidden);

            // 过滤掉隐藏标识文件
            QFileInfoList filteredEntries;
            for(const QFileInfo &entry : entries)
            {
                QString fileName = entry.fileName();
                if(fileName != REPO_ID && fileName != REPO_ID_FILE && fileName != REPO_ID_DIR)
                {
                    filteredEntries.append(entry);
                }
            }

            // 如果除了隐藏标识文件外没有其他文件和文件夹
            if(filteredEntries.isEmpty())
            {
                // 删除标识文件（保留REPO_ID）
                QFile::remove(parentDir + "/" + REPO_ID_FILE);
                QFile::remove(parentDir + "/" + REPO_ID_DIR);
            }
        }
    }

    return success;
}

// 判断是否为代码库项目文件/文件夹
bool FileManager::isRepositoryItem(const QString &destDir)
{
    return QFile::exists(destDir + "/" + REPO_ID);
}

// 判断是否为项目文件
bool FileManager::isProject(const QString &destDir)
{
    return isRepositoryItem(destDir) && QFile::exists(destDir + "/meta.ctk");
}

// 判断是否为分类目录
bool FileManager::isCategory(const QString &destDir)
{
    return isRepositoryItem(destDir) && !isProject(destDir);
}

// 判断是否命名重复
bool FileManager::hasNameRepetition(const QString &name, const QString& destDir)
{
    return QDir(destDir).exists(name);
}

// 判断是否是代码文件
bool FileManager::isCodeFile(const QString &filePath)
{
    // 获取文件后缀（不区分大小写）
    QString suffix = QFileInfo(filePath).suffix().toLower();
    // 常见代码文件扩展名列表
    static const QStringList codeExtensions = {
        "c", "cc", "cpp", "cxx", "c++", "h", "hh", "hpp", "hxx",
        "java", "js", "ts", "py", "php", "rb", "cs", "swift", "go",
        "rs", "kt", "dart", "lua", "sh", "bat", "ps1", "vbs",
        "html", "htm", "css", "scss", "less", "xml", "json", "yaml", "yml",
        "sql", "md", "ini", "cfg", "conf", "asm", "s", "f", "f90", "m"};
    return codeExtensions.contains(suffix);
}

// 判断是否是图片文件
bool FileManager::isImageFile(const QString &filePath)
{
    // 获取文件后缀（不区分大小写）
    QString suffix = QFileInfo(filePath).suffix().toLower();
    // 常见图片文件扩展名列表
    static const QStringList imageExtensions = {
        "png", "jpg", "jpeg", "bmp", "gif", "tif", "tiff",
        "svg", "webp", "ico", "psd", "raw", "heic", "heif",
        "dng", "cr2", "nef", "orf", "sr2", "arw", "ppm", "pgm"
    };

    return imageExtensions.contains(suffix);
}

bool FileManager::hasProjectMarker(const QString &destDir)
{
    return QFile::exists(destDir + "/" + REPO_ID_FILE);
}

bool FileManager::hasCategoryMarker(const QString &destDir)
{
    return QFile::exists(destDir + "/" + REPO_ID_DIR);
}

// 检测命名重复，自动重命名
QString FileManager::autoRename(const QString &name, const QString &path)
{
    // 进行合法性检查
    QString sanitizedName = sanitizeFileName(name);
    if(sanitizedName.isEmpty()) return "";

    if(isProject(path))
    {
        QFileInfo info(sanitizedName);
        QString base = info.completeBaseName();
        QString suffix = info.suffix();
        QString newName = sanitizedName;
        QDir dir(path);
        int count = 0;

        while(dir.exists(newName))
        {
            newName = QString("%1%2%3")
                    .arg(base).arg(++count).arg(suffix.isEmpty() ? "" : "." + suffix);
        }
        return newName;
    }
    else
    {
        QString base = sanitizedName;
        QString newName = sanitizedName;
        QDir dir(path);
        int count = 0;

        while(dir.exists(newName))
        {
            newName = QString("%1%2").arg(base).arg(++count);
        }
        return newName;
    }
}

bool FileManager::renameItem(const QString &newName, const QString &path)
{
    // 进行合法性检查
    QString sanitizedName = sanitizeFileName(newName);
    if(sanitizedName.isEmpty()) return false;
    // 获取原始路径的 QFileInfo
    QFileInfo fileInfo(path);

    // 检查路径是否存在且是目录
    if (!fileInfo.exists() || !fileInfo.isDir())
    {
        qWarning() << "The path does not exist or is not a directory" + path;
        return false;
    }

    // 获取父目录
    QDir parentDir = fileInfo.dir();

    // 构建新路径
    QString newPath = parentDir.filePath(sanitizedName);

    // 检查新路径是否已存在
    if (QFileInfo::exists(newPath))
    {
        qWarning() << "The destPath already exists" + newPath;
        return false;
    }
    // 重命名项目文件/分类目录
    if(isProject(path))
    {
        parentDir.rename(fileInfo.fileName(), sanitizedName);
        // 对m_metaCtk进行重配置
        QFile file(newPath + "/meta.ctk");
        if(file.open(QIODevice::WriteOnly))
        {
            file.close();
        }
        emit createMetaCtk(newPath + "/meta.ctk", sanitizedName);

    }
    else if(isCategory(path))
    {
        parentDir.rename(fileInfo.fileName(), sanitizedName);
    }
    else
    {
        qWarning() << "It is not a repository item" + path;
        return false;
    }

    return true;

}

QString FileManager::sanitizeFileName(const QString &fileName)
{
    if(fileName.isEmpty()) return "";

    // Windows/Linux 文件命名非法字符
    static QRegularExpression illegalChars(R"([<>:"/\\|?*\x00-\x1F])");
    QString sanitized = fileName;

    // 替换非法字符为下划线
    sanitized.replace(illegalChars, "_");

    // 移除开头和结尾的空格和点
    sanitized = sanitized.trimmed();
    while(sanitized.endsWith('.')) sanitized.chop(1);

    // 确保不以空格开头或结尾
    sanitized = sanitized.trimmed();

    // 检查保留名称（Windows）
    static QStringList reservedNames = {
        "CON", "PRN", "AUX", "NUL",
        "COM1", "COM2", "COM3", "COM4", "COM5", "COM6", "COM7", "COM8", "COM9",
        "LPT1", "LPT2", "LPT3", "LPT4", "LPT5", "LPT6", "LPT7", "LPT8", "LPT9"
    };

    QString upperName = sanitized.toUpper();
    for(const QString &reserved : reservedNames)
    {
        if(upperName == reserved || upperName.startsWith(reserved + "."))
        {
            sanitized = "_" + sanitized;
            break;
        }
    }

    // 限制长度（255字符，但为了安全限制到200）
    if(sanitized.length() > 200) sanitized = sanitized.left(200);

    return sanitized;
}

// 递归删除实现
bool FileManager::removeRecursively(const QString &path)
{
    QDir dir(path);
    if(!dir.exists()) return true;

    bool success = true;
    QFileInfoList entries = dir.entryInfoList(QDir::AllEntries | QDir::NoDotAndDotDot | QDir::Hidden);

    for(const QFileInfo& info : entries)
    {
        if(info.isDir())
        {
            success = success && removeRecursively(info.absoluteFilePath());
        }
        else
        {
            success = success && QFile::remove(info.absoluteFilePath());
        }
    }

    return success && dir.rmdir(path);
}

// 添加隐藏标识文件
bool FileManager::addRepoIdFile(const QString &path)
{
    QFile file(path + "/" + REPO_ID_FILE);
    if(!file.open(QIODevice::WriteOnly))
    {
        qWarning() << "Failed to create the identification file" + file.errorString();
        return false;
    }
    file.close();
    return true;

}

bool FileManager::addRepoIdDir(const QString &path)
{
    QFile file(path + "/" + REPO_ID_DIR);
    if(!file.open(QIODevice::WriteOnly))
    {
        qWarning() << "Failed to create the identification file" + file.errorString();
        return false;
    }
    file.close();
    return true;
}

bool FileManager::addRepoId(const QString &path)
{
    QFile file(path + "/" + REPO_ID);
    if(!file.open(QIODevice::WriteOnly))
    {
        qWarning() << "Failed to create the identification file" + file.errorString();
        return false;
    }
    file.close();
    return true;
}
