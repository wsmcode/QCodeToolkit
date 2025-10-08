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
    // == 1. 创建文件夹 ==
    QString fileName = autoRename(m_defaultFileName, destDir);
    if(fileName.isEmpty())
    {
        qWarning() << "Failed to generate valid file name";
        return "";
    }

    QString filePath = dir.filePath(fileName);
    if(!dir.mkdir(fileName))
    {
        qWarning() << "Failed to create the project folder";
        return "";
    }
    // == 2. 添加配置文件 ==
    QString configPath = filePath + "/meta.ctk";
    QFile file(configPath);
    if(!file.open(QIODevice::WriteOnly))
    {
        qWarning() << "Failed to create config file:" << configPath;
        // 回滚：删除已创建的文件夹
        dir.rmdir(fileName);
        return "";
    }
    // 添加项目标识文件
    QFile identifierFile(filePath + "/" + m_projectIdentifier);
    if(!identifierFile.open(QIODevice::WriteOnly))
    {
        qWarning() << "Failed to create project identifier file:" << filePath + "/" + m_projectIdentifier;
        // 回滚：删除已创建的文件夹和meta.ctk
        QFile::remove(configPath);
        dir.rmdir(fileName);
        return "";
    }
    identifierFile.close();

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
    // == 1. 创建文件夹 ==
    QString dirName = autoRename(m_defaultDirName, destDir);
    QString dirPath = dir.filePath(dirName);
    if(!dir.mkdir(dirName))
    {
        qWarning() << "Failed to create a category";
        return "";
    }
    return dirName;
}

// 删除项目文件
bool FileManager::removeItem(const QString &path)
{
    // 获取父目录路径
    QFileInfo fileInfo(path);
    QString parentDir = fileInfo.path();

    // 删除项目
    return removeRecursively(path);
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

bool FileManager::isProject(const QString &path)
{
    QFileInfo fileInfo(path);

    // 如果是目录，检查是否存在 meta.ctk 文件
    if(fileInfo.isDir())
    {
        bool hasMetaCtk = QFile::exists(path + "/meta.ctk");
        bool hasIdentifier = QFile::exists(path + "/" + m_projectIdentifier);
        return hasMetaCtk && hasIdentifier;
    }
//    // 如果是文件，检查是否以 .ctk 结尾（根据头文件说明）
//    else if(fileInfo.isFile())
//    {
//        return fileInfo.suffix().toLower() == "ctk";
//    }

    return false;
}

// 检测命名重复，自动重命名
QString FileManager::autoRename(const QString &name, const QString &path)
{
    // 进行合法性检查
    QString sanitizedName = sanitizeFileName(name);
    if(sanitizedName.isEmpty()) return "";

    QDir dir(path);
    QString base = sanitizedName;
    QString newName = sanitizedName;
    int count = 0;

    while(dir.exists(newName))
    {
        count++;
        // 目录
        if(QDir(path + "/" + newName).exists())
        {
            newName = QString("%1%2").arg(base).arg(count);
        }
        else // 文件
        {
            QFileInfo info(newName);
            QString fileBase = info.completeBaseName();
            QString suffix = info.suffix();
            newName = QString("%1%2%3").arg(fileBase).arg(count).arg(suffix.isEmpty() ? "" : "." + suffix);
        }
    }

    return newName;
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
        if(!parentDir.rename(fileInfo.fileName(), sanitizedName))
        {
            qWarning() << "Failed to rename project:" << path << "to" << sanitizedName;
            return false;
        }
        emit createMetaCtk(newPath + "/meta.ctk", sanitizedName);

    }
    else
    {
        if(!parentDir.rename(fileInfo.fileName(), sanitizedName))
        {
            qWarning() << "Failed to rename item:" << path << "to" << sanitizedName;
            return false;
        }
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
