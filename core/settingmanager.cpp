#include "databasemanager.h"
#include "fontmanager.h"
#include "settingmanager.h"
#include "stylemanager.h"

#include <QApplication>
#include <QDebug>
#include <QDir>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QSettings>
#include <settingdialog.h>

SettingManager::SettingManager(QObject *parent) : QObject(parent)
{
    m_db = DatabaseManager::getDatabaseManager();
    m_debounceTimer.setSingleShot(true);
    m_debounceTimer.setInterval(1000);

    QObject::connect(&m_debounceTimer, &QTimer::timeout, this, &SettingManager::onDebounceTimeout);
}

SettingManager::~SettingManager()
{
    // 保存
    saveAll(m_currentSettings);
    if(m_settings)
    {
        m_settings->sync();
        delete m_settings;
    }
}

void SettingManager::init(const QString &configPath)
{
    if(m_configPath == configPath) return;
    m_configPath = configPath;

    // 确保配置目录存在
    QDir configDir(configPath);
    if(!configDir.exists()) configDir.mkpath(".");

    QString configFile = configDir.filePath("app_settings.ini");
    m_settings = new QSettings(configFile, QSettings::IniFormat);

    // 加载设置
    m_currentSettings = loadAll();
}

void SettingManager::applyCurrentSettings()
{
    // 应用主题
    if(m_currentSettings.appearance.theme == "light")
        StyleManager::getStyleManager()->setTheme(Theme::LightTheme);
    else
        StyleManager::getStyleManager()->setTheme(Theme::DarkTheme);

    // 应用字体设置
    FontManager::getFontManager()->setBaseFont(m_currentSettings.appearance.fontFamily,
                                               m_currentSettings.appearance.fontSize);


    emit statusBarVisibilityChanged(m_currentSettings.appearance.showStatusBar);
}

void SettingManager::showDialog(QWidget *parent)
{
    SettingDialog dialog(parent);
    dialog.load(m_currentSettings);

    if(dialog.exec())
    {
        AppSettings newSettings = dialog.currentSettings();
        saveAll(newSettings);
    }
}

void SettingManager::handleSettingChange(const QString &category, const QString &key, const QVariant &value)
{
    // 更新当前设置
    updateCurrentSettings(category, key, value);

    // 立即应用需要实时生效的设置
    applyImmediateSettings(category, key, value);

    // 启动防抖保存定时器
    m_debounceTimer.start();
}

void SettingManager::updateCurrentSettings(const QString &category, const QString &key, const QVariant &value)
{
    if(category == "appearance")
    {
        if(key == "theme")
        {
            m_currentSettings.appearance.theme = value.toString();
        }
        else if(key == "fontSize")
        {
            m_currentSettings.appearance.fontSize = value.toInt();
        }
        else if(key == "fontFamily")
        {
            m_currentSettings.appearance.fontFamily = value.toString();
        }
        else if(key == "showStatusBar")
        {
            m_currentSettings.appearance.showStatusBar = value.toBool();
        }
    }
}

void SettingManager::applyImmediateSettings(const QString &category, const QString &key, const QVariant &value)
{
    if(category == "appearance" && key == "theme")
    {
        // 立即应用主题变更
        if(value.toString() == "light")
        {
            StyleManager::getStyleManager()->setTheme(Theme::LightTheme);
        }
        else
        {
            StyleManager::getStyleManager()->setTheme(Theme::DarkTheme);
        }
    }

    if(category == "appearance" && (key == "fontSize" || key == "fontFamily"))
    {
        // 立即应用字体变更
        QFont currentFont = FontManager::getFontManager()->baseFont();

        if(key == "fontFamily")
            currentFont.setFamily(value.toString());
        else if(key == "fontSize")
            currentFont.setPointSize(value.toInt());

        FontManager::getFontManager()->setBaseFont(currentFont);
    }

    if(category == "appearance" && key == "showStatusBar")
    {
        emit statusBarVisibilityChanged(value.toBool());
    }
}

AppSettings SettingManager::loadAll()
{
    AppSettings settings;

    // 从配置文件加载
    loadFromConfigFile(settings);
    // 从数据库加载
    loadFromDatabase(settings);

    return settings;
}

void SettingManager::saveAll(const AppSettings &settings)
{
    m_currentSettings = settings;

    // 保存到配置文件
    saveToConfigFile(settings);
    // 保存到数据库
    saveToDatabase(settings);

    emit settingsSaved();
}

void SettingManager::saveToConfigFile(const AppSettings &settings)
{
    saveAppearanceSettings(settings.appearance);
    saveEditorSettings(settings.editor);
    saveCodebasicSettings(settings.code);
    saveMenuSettings(settings.menu);
    saveExportSettings(settings.exportSet);
    saveGeneralbasicSettings(settings.general);
}

void SettingManager::loadFromConfigFile(AppSettings &settings)
{
    loadAppearanceSettings(settings.appearance);
    loadEditorSettings(settings.editor);
    loadCodebasicSettings(settings.code);
    loadMenuSettings(settings.menu);
    loadExportSettings(settings.exportSet);
    loadGeneralbasicSettings(settings.general);
}

void SettingManager::saveToDatabase(const AppSettings &settings)
{
    saveCodeSnippetsSettings(settings.code);
    saveGeneralShortcutsSettings(settings.general);
}

void SettingManager::loadFromDatabase(AppSettings &settings)
{
    loadCodeSnippetsSettings(settings.code);
    loadGeneralShortcutsSettings(settings.general);
}

void SettingManager::onDebounceTimeout()
{
    // 防抖保存
    saveToConfigFile(m_currentSettings);
    saveToDatabase(m_currentSettings);
}

void SettingManager::loadAppearanceSettings(AppearanceSet &settings)
{
    // 外观设置
    settings.theme = m_settings->value("appearance/theme", "light").toString();
    settings.fontFamily = m_settings->value("appearance/fontFamily", "Microsoft YaHei").toString();
    settings.fontSize = m_settings->value("appearance/fontSize", 9).toInt();
    settings.zoomLevel = m_settings->value("appearance/zoomLevel", 100).toInt();
    settings.zoomWithWheel = m_settings->value("appearance/zoomWithWheel", true).toBool();
    settings.showStatusBar = m_settings->value("appearance/showStatusBar", true).toBool();
    settings.showWordCount = m_settings->value("appearance/showWordCount", true).toBool();
}

void SettingManager::loadEditorSettings(EditorSet &settings)
{
    // 编辑器设置
    settings.scrollFollow = m_settings->value("editor/scrollFollow", false).toBool();
    settings.scrollSpeed = QString(m_settings->value("editor/scrollSpeed", 1).toInt());
    settings.saveOnClose = m_settings->value("editor/saveOnClose", true).toBool();
    settings.autoSave = m_settings->value("editor/autoSave", false).toBool();
}

void SettingManager::loadCodebasicSettings(CodeSet &settings)
{
    // 代码设置（基础部分）
    settings.defaultIndent = m_settings->value("code/defaultIndent", "4").toString();
    settings.defaultLanguage = m_settings->value("code/defaultLanguage", "text").toString();
    settings.usePairedSymbols = m_settings->value("code/usePairedSymbols", true).toBool();
    settings.showLineNumbers = m_settings->value("code/showLineNumbers", true).toBool();
}

void SettingManager::loadCodeSnippetsSettings(CodeSet &settings)
{
    // 加载代码片段
    Settings snippetSetting = m_db->setting("code_snippets");
    if(!snippetSetting.isEmpty())
    {
        QJsonDocument doc = QJsonDocument::fromJson(snippetSetting.value.toUtf8());
        if(!doc.isNull())
        {
            settings.snippets = jsonToSnippets(doc.object());
        }
    }
}

void SettingManager::loadMenuSettings(MenuSet &settings)
{
    // 菜单设置
    settings.showNewItem = m_settings->value("menu/showNewItem", true).toBool();
    settings.showCodeCompletionList = m_settings->value("menu/showCodeCompletionList", true).toBool();
    settings.showFilter = m_settings->value("menu/showFilter", true).toBool();
    settings.showQuickSettings = m_settings->value("menu/showQuickSettings", true).toBool();
}

void SettingManager::loadExportSettings(ExportSet &settings)
{
    Q_UNUSED(settings)
}

void SettingManager::loadGeneralbasicSettings(GeneralSet &settings)
{
    // 通用设置（基础部分）
    settings.language = m_settings->value("general/language", "zh_CN").toString();
    settings.checkForUpdates = m_settings->value("general/checkForUpdates", true).toBool();
    settings.enableLogging = m_settings->value("general/enableLogging", false).toBool();
    settings.logLevel = m_settings->value("general/logLevel", "info").toString();
}

void SettingManager::loadGeneralShortcutsSettings(GeneralSet &settings)
{
    // 加载快捷键
    Settings shortcutSetting = m_db->setting("general_shortcuts");
    if(!shortcutSetting.isEmpty())
    {
        QJsonDocument doc = QJsonDocument::fromJson(shortcutSetting.value.toUtf8());
        if(!doc.isNull())
        {
            settings.shortcuts = jsonToShortcuts(doc.object());
        }
    }
}

void SettingManager::saveAppearanceSettings(const AppearanceSet &settings)
{
    m_currentSettings.appearance = settings;

    // 外观设置实时保存到配置文件
    m_settings->setValue("appearance/theme", settings.theme);
    m_settings->setValue("appearance/fontFamily", settings.fontFamily);
    m_settings->setValue("appearance/fontSize", settings.fontSize);
    m_settings->setValue("appearance/zoomLevel", settings.zoomLevel);
    m_settings->setValue("appearance/zoomWithWheel", settings.zoomWithWheel);
    m_settings->setValue("appearance/showStatusBar", settings.showStatusBar);
    m_settings->setValue("appearance/showWordCount", settings.showWordCount);

    m_settings->sync();
}

void SettingManager::saveEditorSettings(const EditorSet &settings)
{
    m_currentSettings.editor = settings;

    // 基础设置实时保存到配置文件
    m_settings->setValue("editor/scrollFollow", settings.scrollFollow);
    m_settings->setValue("editor/scrollSpeed", settings.scrollSpeed);
    m_settings->setValue("editor/saveOnClose", settings.saveOnClose);
    m_settings->setValue("editor/autoSave", settings.autoSave);

    m_settings->sync();
}

void SettingManager::saveCodebasicSettings(const CodeSet &settings)
{
    m_currentSettings.code = settings;

    // 基础设置实时保存到配置文件
    m_settings->setValue("code/defaultIndent", settings.defaultIndent);
    m_settings->setValue("code/defaultLanguage", settings.defaultLanguage);
    m_settings->setValue("code/usePairedSymbols", settings.usePairedSymbols);
    m_settings->setValue("code/showLineNumbers", settings.showLineNumbers);

    m_settings->sync();
}

void SettingManager::saveCodeSnippetsSettings(const CodeSet &settings)
{
    // 保存代码片段
    QJsonObject snippetsJson = snippetsToJson(settings.snippets);
    QJsonDocument snippetsDoc(snippetsJson);

    Settings snippetSetting;
    snippetSetting.key = "code_snippets";
    snippetSetting.value = snippetsDoc.toJson(QJsonDocument::Compact);
    snippetSetting.category = "code";
    snippetSetting.dataType = "json";

    if(m_db->setting(snippetSetting.key).isEmpty())
        m_db->addSetting(snippetSetting);
    else
        m_db->updateSetting(snippetSetting);
}

void SettingManager::saveMenuSettings(const MenuSet &settings)
{
    m_currentSettings.menu = settings;

    // 基础设置实时保存到配置文件
    m_settings->setValue("menu/showNewItem", settings.showNewItem);
    m_settings->setValue("menu/showCodeCompletionList", settings.showCodeCompletionList);
    m_settings->setValue("menu/showFilter", settings.showFilter);
    m_settings->setValue("menu/showQuickSettings", settings.showQuickSettings);

    m_settings->sync();
}

void SettingManager::saveExportSettings(const ExportSet &settings)
{
    Q_UNUSED(settings)
}

void SettingManager::saveGeneralbasicSettings(const GeneralSet &settings)
{
    m_currentSettings.general = settings;

    // 基础设置实时保存到配置文件
    m_settings->setValue("general/language", settings.language);
    m_settings->setValue("general/checkForUpdates", settings.checkForUpdates);
    m_settings->setValue("general/enableLogging", settings.enableLogging);
    m_settings->setValue("general/logLevel", settings.logLevel);

    m_settings->sync();
}

void SettingManager::saveGeneralShortcutsSettings(const GeneralSet &settings)
{
    // 保存快捷键
    QJsonObject shortcutsJson = shortcutsToJson(settings.shortcuts);
    QJsonDocument shortcutsDoc(shortcutsJson);

    Settings shortcutSetting;
    shortcutSetting.key = "general_shortcuts";
    shortcutSetting.value = shortcutsDoc.toJson(QJsonDocument::Compact);
    shortcutSetting.category = "general";
    shortcutSetting.dataType = "json";

    if(m_db->setting(shortcutSetting.key).isEmpty())
        m_db->addSetting(shortcutSetting);
    else
        m_db->updateSetting(shortcutSetting);
}

QJsonObject SettingManager::snippetsToJson(const QVector<Snippet> &snippets)
{
    QJsonObject root;
    QJsonArray snippetsArray;

    for(const Snippet &snippet : snippets)
    {
        QJsonObject snippetObj;
        snippetObj["trigger"] = snippet.trigger;
        snippetObj["content"] = snippet.content;
        snippetObj["triggerType"] = snippet.triggerType;
        snippetObj["created"] = snippet.created.toString(Qt::ISODate);
        snippetObj["modified"] = snippet.modified.toString(Qt::ISODate);
        snippetsArray.append(snippetObj);
    }

    root["snippets"] = snippetsArray;
    return root;
}

QVector<Snippet> SettingManager::jsonToSnippets(const QJsonObject &json)
{
    QVector<Snippet> snippets;

    if(json.contains("snippets") && json["snippets"].isArray())
    {
        QJsonArray array = json["snippets"].toArray();
        for(const QJsonValue &value : array)
        {
            QJsonObject obj = value.toObject();
            Snippet snippet;
            snippet.trigger = obj["trigger"].toString();
            snippet.content = obj["content"].toString();
            snippet.triggerType = obj["triggerType"].toString();
            snippet.created = QDateTime::fromString(obj["created"].toString(), Qt::ISODate);
            snippet.modified = QDateTime::fromString(obj["modified"].toString(), Qt::ISODate);
            snippets.append(snippet);
        }
    }

    return snippets;
}

QJsonObject SettingManager::shortcutsToJson(const QVector<Shortcut> &shortcuts)
{
    QJsonObject root;
    QJsonArray shortcutsArray;

    for(const Shortcut &shortcut : shortcuts)
    {
        QJsonObject shortcutObj;
        shortcutObj["actionName"] = shortcut.actionName;
        shortcutObj["keySequence"] = shortcut.keySequence.toString();
        shortcutObj["description"] = shortcut.description;
        shortcutsArray.append(shortcutObj);
    }

    root["shortcuts"] = shortcutsArray;
    return root;
}

QVector<Shortcut> SettingManager::jsonToShortcuts(const QJsonObject &json)
{
    QVector<Shortcut> shortcuts;

    if(json.contains("shortcuts") && json["shortcuts"].isArray())
    {
        QJsonArray array = json["shortcuts"].toArray();
        for(const QJsonValue &value : array)
        {
            QJsonObject obj = value.toObject();
            Shortcut shortcut;
            shortcut.actionName = obj["actionName"].toString();
            shortcut.keySequence = QKeySequence::fromString(obj["keySequence"].toString());
            shortcut.description = obj["description"].toString();
            shortcuts.append(shortcut);
        }
    }

    return shortcuts;
}

