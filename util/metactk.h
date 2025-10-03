#ifndef METACTK_H
#define METACTK_H
/*****************************************************
*
* @file     metactk.h
* 用id作为文件表示，包括文件夹和文件，在隐藏文件中标识
* @brief    meta.ctk配置文件封装
*
* @description
*           ==== 核心功能 ====
*           - 读写json格式的配置文件(meta.ctk)
*           - 管理代码笔记的元数据
*           - 资源路径解析与验证
*           - 自动生成唯一标识符和时间戳
*           ==== 配置文件结构 ====
*           ID = <唯一标识>
*
*
*           DemoImage = <封面图路径>
*           Created = <创建时间>
*           Modified = <修改时间>
*           Author = <作者>
*           ProjectName = <项目名称>
*           Favorite = <收藏路径列表>
*
*           ==== 使用说明 ====
*           1. 构造函数接收配置文件路径
*           2. 支持读取/保存配置
*
* @author   无声目
* @date     2025/08/15
* @history
*****************************************************/

#include <QDateTime>
#include <QObject>
#include <QUuid>
#include "code_types.h"
#include <QMap>

class MetaCtk : public QObject
{
    Q_OBJECT
public:
    explicit MetaCtk(const QString& configPath = "", QObject *parent = nullptr);

    // ======== 文件操作接口 ========
    bool load(const QString& configPath = "");
    bool save(const QString& configPath = "") const;

    // ======== 配置验证接口 ========
    bool isValid() const;

    // ======== 数据访问接口 ========
    QString configPath() const;
    QString id() const;
    QString projectName() const;
    QString demoImagePath() const;
    QDateTime created() const;
    QDateTime modifed() const;
    QString author() const;
    QStringList favoritePaths() const;
    CodeNote noteContent() const;

    // ======== 数据设置接口 ========
    void setConfigPath(const QString& configPath);
    void setProjectName(const QString& projectName);
    void addFavoritePath(const QString& path);
    void setFavoritePaths(const QStringList& pathList);
    void setDemoImagePath(const QString& path);
    void modifyDemoImagePath(const QString& path);
    void setAuthor(const QString& author);
    void setNoteContent(const CodeNote& codeNote);

    // ======== 标签操作接口 ========
    void addTagGroup(const QString& groupName);
    void removeTagGroup(const QString& groupName);
    void addTagToGroup(const QString& groupName, const QString& tag);
    void removeTagFromGroup(const QString& groupName, const QString& tag);
    void setTags(const QMap<QString, QStringList>& tags);

    // ======== 处理json文件 ========
    QJsonObject noteToJson(const CodeNote& note) const;
    CodeNote jsonToNote(const QJsonObject& json) const;
    bool saveNoteToFile(const CodeNote& note, const QString& filePath) const;

    // 静态工具方法
    static QString generateUniqueId()
    {
        return QUuid::createUuid().toString(QUuid::WithoutBraces);
    }
    static QString currentDateTimeString()
    {
        return QDateTime::currentDateTime().toString("yyyy/MM/dd hh:mm:ss.zzz");
    }

signals:

private:
    void initDefaults(); // 初始化默认值

    QString m_configPath;// 配置文件路径

    // ======== 配置数据 ========
    QString m_projectName; // 项目名
    QString m_id; // 唯一标识符，不分大小写
    QStringList m_favoritePaths; // 收藏目录
    QString m_demoImage;
    QDateTime m_created;
    QDateTime m_modified;
    QString m_author;

    CodeNote m_codeNote;
};

#endif // METACTK_H
