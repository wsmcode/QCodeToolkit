#include "metactk.h"

#include <QJsonArray>
#include <QJsonObject>
#include <QDebug>
#include <QFile>
#include <QJsonDocument>
#include <QFileInfo>
#include <QDir>

MetaCtk::MetaCtk(const QString &configPath, QObject *parent)
    : QObject(parent),
      m_configPath(configPath)
{
    initDefaults();
}

bool MetaCtk::load(const QString &configPath)
{
    initDefaults();
    if(!configPath.isEmpty())
    {
        m_configPath = configPath;
    }
    qInfo() << "load" << m_configPath;

    // 检查文件是否存在
    if(m_configPath.isEmpty()
            || !QFile::exists(m_configPath)
            || QFileInfo(m_configPath).fileName() != "meta.ctk")
    {
        if(!m_configPath.isEmpty() && QFile::exists(m_configPath + "/meta.ctk"))
        {
            m_configPath += "/meta.ctk";
        }
        else
        {
            qWarning() << "The config file does not exist" << m_configPath;
            return false;
        }
    }

    QFile file(m_configPath);
    if(!file.open(QIODevice::ReadOnly))
    {
        qWarning() << "Failed to open the file" << m_configPath;
        return false;
    }
    QByteArray data = file.readAll();
    file.close();

    // 检查文件是否为空
    if(data.trimmed().isEmpty())
    {
        qInfo() << "Config file is empty, using defaults" << m_configPath;
        return true; // 空文件视为初次加载，使用默认值
    }

    QJsonDocument doc = QJsonDocument::fromJson(data);
    if(doc.isNull() || !doc.isObject())
    {
        qWarning() << "The file data is not a valid JSON object" << m_configPath;
        return false;
    }

    QJsonObject root = doc.object();

    // 读取基本元数据
    QString id = root.value("ID").toString();
    if(!id.isEmpty()) m_id = id;

    QString projectName = root.value("ProjectName").toString();
    if(!projectName.isEmpty()) m_projectName = projectName;

    QString demoImage = root.value("DemoImage").toString();
    if(!demoImage.isEmpty()) m_demoImage = demoImage;

    QString author = root.value("Author").toString();
    if(!author.isEmpty()) m_author = author;

    // 读取时间戳
    QString createdStr = root.value("Created").toString();
    if(!createdStr.isEmpty())
        m_created = QDateTime::fromString(createdStr, "yyyy/MM/dd hh:mm:ss.zzz");

    QString modifiedStr = root.value("Modified").toString();
    if(!modifiedStr.isEmpty())
        m_modified = QDateTime::fromString(modifiedStr, "yyyy/MM/dd hh:mm:ss.zzz");

    // 读取收藏路径
    QJsonArray favoriteArray = root.value("Favorite").toArray();
    for(const auto& pathVal : favoriteArray)
    {
        m_favoritePaths.append(pathVal.toString());
    }

    // 读取笔记内容
    if(root.contains("NoteContent") && root["NoteContent"].isObject())
    {
        m_codeNote = jsonToNote(root["NoteContent"].toObject());
    }

    return true;
}

bool MetaCtk::save(const QString &configPath) const
{
    QString savePath = configPath.isEmpty() ? m_configPath : configPath;
    if(savePath.isEmpty())
    {
        qWarning() << "Save failed. No directory was specified";
        return false;
    }

    QJsonObject root;

    // 写入基本元数据
    root["ID"] = m_id;
    root["ProjectName"] = m_projectName;
    root["DemoImage"] = m_demoImage;
    root["Author"] = m_author;

    // 写入时间戳
    root["Created"] = m_created.toString("yyyy/MM/dd hh:mm:ss.zzz");
    root["Modified"] = currentDateTimeString();

    // 写入收藏路径
    QJsonArray favoriteArray;
    for(const auto& path : m_favoritePaths)
    {
        favoriteArray.append(path);
    }
    root["Favorite"] = favoriteArray;

    // 写入笔记内容
    root["NoteContent"] = noteToJson(m_codeNote);

    // 确保目录存在
    QFileInfo fileInfo(savePath);
    QDir dir = fileInfo.dir();
    if(!dir.exists())
    {
        qWarning() << "The saveDir does not exist" << m_configPath;
        return false;
    }

    // 写入文件
    QFile file(savePath);
    if(!file.open(QIODevice::WriteOnly))
    {
        qWarning() << "Failed to open the file" << m_configPath;
        return false;
    }

    QJsonDocument doc(root);
    file.write(doc.toJson(QJsonDocument::Indented));
    file.close();

    return true;
}

bool MetaCtk::isValid() const
{
    // 验证ID
    if(m_id.isEmpty()) return false;
    return true;
}

QString MetaCtk::configPath() const
{
    return m_configPath;
}

QString MetaCtk::id() const
{
    return m_id;
}

QString MetaCtk::projectName() const
{
    return m_projectName;
}

QString MetaCtk::demoImagePath() const
{
    return m_demoImage;
}

QDateTime MetaCtk::created() const
{
    return m_created;
}

QDateTime MetaCtk::modifed() const
{
    return m_modified;
}

QString MetaCtk::author() const
{
    return m_author;
}

QStringList MetaCtk::favoritePaths() const
{
    return m_favoritePaths;
}

CodeNote MetaCtk::noteContent() const
{
    return m_codeNote;
}

void MetaCtk::setConfigPath(const QString &configPath)
{
    m_configPath = configPath;
}

void MetaCtk::setProjectName(const QString &projectName)
{
    m_projectName = projectName;
}

void MetaCtk::addFavoritePath(const QString &path)
{
    if(!m_favoritePaths.contains(path))
    {
        m_favoritePaths.append(path);
    }
}

void MetaCtk::setFavoritePaths(const QStringList &pathList)
{
    //if(pathList.isEmpty()) return;
    m_favoritePaths.clear();
    m_favoritePaths = pathList;
}

void MetaCtk::setDemoImagePath(const QString &path)
{
    if(m_demoImage.isEmpty()) m_demoImage = path;
}

void MetaCtk::modifyDemoImagePath(const QString &path)
{
    m_demoImage = path;
}

void MetaCtk::setAuthor(const QString &author)
{
    m_author = author;
}

void MetaCtk::setNoteContent(const CodeNote &codeNote)
{
    m_codeNote = codeNote;
}

void MetaCtk::addTagGroup(const QString &groupName)
{
    if(!m_codeNote.tags.contains(groupName))
    {
        m_codeNote.tags[groupName] = QStringList();
    }
}

void MetaCtk::removeTagGroup(const QString &groupName)
{
    m_codeNote.tags.remove(groupName);
}

void MetaCtk::addTagToGroup(const QString &groupName, const QString &tag)
{
    if(!m_codeNote.tags.contains(groupName))
    {
        qWarning() << "The grounName dose not exist: " << groupName;
    }
    if(!m_codeNote.tags[groupName].contains(tag))
    {
        m_codeNote.tags[groupName].append(tag);
    }
}

void MetaCtk::removeTagFromGroup(const QString &groupName, const QString &tag)
{
    if(m_codeNote.tags.contains(groupName))
    {
        m_codeNote.tags[groupName].removeAll(tag);
    }
}

void MetaCtk::setTags(const QMap<QString, QStringList> &tags)
{
    m_codeNote.tags = tags;
}

QJsonObject MetaCtk::noteToJson(const CodeNote &note) const
{
    QJsonObject obj;

    // 序列化标签组
    QJsonObject tagsObj;
    for(auto it = note.tags.begin(); it != note.tags.end(); it++)
    {
        QJsonArray tagsArray;
        for(const auto& tag : it.value())
        {
            tagsArray.append(tag);
        }
        tagsObj[it.key()] = tagsArray;
    }
    obj["tags"] = tagsObj;

    // 序列化笔记
    QJsonArray noteArray;
    for(const auto& item : note.note)
    {
        QJsonObject itemObj;
        switch (item.type)
        {
        case NType::Text:
            itemObj["type"] = "text"; break;
        case NType::Image:
            itemObj["type"] = "image"; break;
        case NType::Markdown:
            itemObj["type"] = "markdown"; break;
        case NType::Code:
            itemObj["type"] = "code"; break;
        }
        itemObj["content"] = item.content;
        itemObj["language"] = item.language;
        noteArray.append(itemObj);
    }
    obj["note"] = noteArray;

    return obj;
}

CodeNote MetaCtk::jsonToNote(const QJsonObject &json) const
{
    CodeNote note;

    // 反序列化标签组
    if(json.contains("tags") && json["tags"].isObject())
    {
        QJsonObject tagsObj = json["tags"].toObject();
        for(auto it = tagsObj.begin(); it != tagsObj.end(); it++)
        {
            if(it.value().isArray())
            {
                QJsonArray tagArray = it.value().toArray();
                QStringList tagList;
                for(const auto& tagVal : tagArray)
                {
                    tagList.append(tagVal.toString());
                }
                note.tags[it.key()] = tagList;
            }
        }
    }

    // 反序列化笔记
    if(json.contains("note") && json["note"].isArray())
    {
        QJsonArray noteArray = json["note"].toArray();
        for(const auto& itemVal : noteArray)
        {
            QJsonObject itemObj = itemVal.toObject();
            NoteItem item;
            QString typeStr = itemObj["type"].toString();
            if(typeStr == "text") item.type = NType::Text;
            else if(typeStr == "image") item.type = NType::Image;
            else if(typeStr == "markdown") item.type = NType::Markdown;
            else if(typeStr == "code") item.type = NType::Code;

            item.content = itemObj["content"].toString();
            item.language = itemObj["language"].toString();
            note.note.append(item);
        }
    }
    return note;
}

void MetaCtk::initDefaults()
{
    m_projectName = "untitled";
    m_id = generateUniqueId();
    m_favoritePaths = QStringList();

    m_demoImage = "";
    m_created = QDateTime::currentDateTime();
    m_modified = QDateTime::currentDateTime();
    m_author = "Unknow";

    // 清空笔记内容
    m_codeNote.note.clear();
    m_codeNote.tags.clear();
}
