#ifndef SETTINGMANAGER_H
#define SETTINGMANAGER_H

#include <QObject>
#include <QTimer>
#include <QVariant>
#include <settings_types.h>

class QSettings;
class DatabaseManager;
class SettingManager : public QObject
{
    Q_OBJECT
public:
//    // 存储策略
//    enum StorageType {
//        ConfigFile,     // 配置文件存储（实时性高、简单的设置）
//        Database,       // 数据库存储（复杂、频繁修改的设置）
//        Both            // 双重存储（关键设置）
//    };
//    Q_ENUM(StorageType)

//    // 设置项信息结构体
//    struct SettingInfo {
//        QString key;
//        QVariant defaultValue;
//        QString dataType;
//        StorageType storageType;
//    };
    // 单例模式
    static SettingManager *getSettingManager()
    {
        static SettingManager s;
        return &s;
    }
    // 删除拷贝构造函数和赋值运算符
    SettingManager(const SettingManager&) = delete;
    SettingManager& operator=(const SettingManager&) = delete;

    // 初始化
    void init(const QString &configPath);

    void applyCurrentSettings(); // 应用当前设置
    // 获取当前设置
    AppSettings currentSettings() const;

    void showDialog(QWidget *parent = nullptr);

    void handleSettingChange(const QString& category, const QString& key, const QVariant& value);

    void updateCurrentSettings(const QString& category, const QString& key, const QVariant& value);

    void applyImmediateSettings(const QString& category, const QString& key, const QVariant& value);

    // 加载所有设置
    AppSettings loadAll();
    // 保存所有设置
    void saveAll(const AppSettings &settings);

    // 配置文件操作
    void saveToConfigFile(const AppSettings &settings);
    void loadFromConfigFile(AppSettings &settings);

    // 数据库操作
    void saveToDatabase(const AppSettings &settings);
    void loadFromDatabase(AppSettings &settings);

signals:
    void statusBarVisibilityChanged(bool visible);
    void settingsSaved();

private slots:
    void onDebounceTimeout();

private:
    explicit SettingManager(QObject *parent = nullptr);
    ~SettingManager();

    // 加载特定分类设置
    void loadAppearanceSettings(AppearanceSet &settings);
    void loadEditorSettings(EditorSet &settings);
    void loadCodebasicSettings(CodeSet &settings);
    void loadCodeSnippetsSettings(CodeSet &settings);
    void loadMenuSettings(MenuSet &settings);
    void loadExportSettings(ExportSet &settings);
    void loadGeneralbasicSettings(GeneralSet &settings);
    void loadGeneralShortcutsSettings(GeneralSet &settings);

    // 保存特定分类设置
    void saveAppearanceSettings(const AppearanceSet &settings);
    void saveEditorSettings(const EditorSet &settings);
    void saveCodebasicSettings(const CodeSet &settings);
    void saveCodeSnippetsSettings(const CodeSet &settings);
    void saveMenuSettings(const MenuSet &settings);
    void saveExportSettings(const ExportSet &settings);
    void saveGeneralbasicSettings(const GeneralSet &settings);
    void saveGeneralShortcutsSettings(const GeneralSet &settings);

    // 序列化/反序列化
    QJsonObject snippetsToJson(const QVector<Snippet> &snippets);
    QVector<Snippet> jsonToSnippets(const QJsonObject &json);

    QJsonObject shortcutsToJson(const QVector<Shortcut> &shortcuts);
    QVector<Shortcut> jsonToShortcuts(const QJsonObject &json);

    // 注册的设置项信息
//    QHash<QString, SettingInfo> m_registeredSettings;

    // 防抖定时器
    QTimer m_debounceTimer;

    // 配置文件路径
    QString m_configPath;
    QSettings *m_settings = nullptr;

    // 数据库管理器
    DatabaseManager *m_db = nullptr;

    // 当前设置
    AppSettings m_currentSettings;

};

#endif // SETTINGMANAGER_H
