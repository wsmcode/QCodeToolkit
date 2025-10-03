#ifndef SETTINGS_TYPES_H
#define SETTINGS_TYPES_H

#include <QString>
#include <QDateTime>
#include <QKeySequence>
#include <QMap>

// 片段结构
struct Snippet {
    QString trigger;
    QString content;
    QString triggerType;
    QDateTime created;
    QDateTime modified;

    Snippet() : trigger(""), content(""), triggerType("")
    {
        created = QDateTime::currentDateTime();
        modified = QDateTime::currentDateTime();
    }

    bool isEmpty() const
    {
        return trigger.isEmpty() && content.isEmpty();
    }
};

// 快捷键条目
struct Shortcut {
    QString actionName;
    QKeySequence keySequence;
    QString description;

    bool isEmpty() const
    {
        return actionName.isEmpty() && keySequence.isEmpty();
    }

    bool operator==(const Shortcut &other) const
    {
        return actionName == other.actionName;
    }
};

struct AppearanceSettings {
    QString theme = "light"; // "light" / "dark"
    QString fontFamily = "Microsoft YaHei";
    int fontSize = 9;
    int zoomLevel = 100;
    bool zoomWithWheel = true;
    bool showStatusBar = true;
    bool showWordCount = true;

    bool isEmpty() const
    {
        return theme.isEmpty() && fontFamily.isEmpty() && fontSize == 0;
    }
};
using AppearanceSet = AppearanceSettings;

struct EditorSettings {
    bool scrollFollow;
    QString scrollSpeed;
    bool saveOnClose;
    bool autoSave;

    bool isEmpty() const
    {
        return scrollSpeed.isEmpty();
    }
};
using EditorSet = EditorSettings;

struct CodeSettings {
    QString defaultIndent; // "自动" / "2" / "3" / "4" / "5" / "Tab"
    QString defaultLanguage;
    bool usePairedSymbols;
    bool showLineNumbers;
    QVector<Snippet> snippets;

    bool isEmpty() const
    {
        return defaultIndent.isEmpty() && defaultLanguage.isEmpty() && snippets.isEmpty();
    }
};
using CodeSet = CodeSettings;

struct MenuSettings {
    bool showNewItem;
    bool showCodeCompletionList;
    bool showFilter;
    bool showQuickSettings;

    bool isEmpty() const
    {
        return !(showNewItem || showCodeCompletionList || showFilter || showQuickSettings);
    }
};
using MenuSet = MenuSettings;

struct ExportSettings {
    // 当前版本未实现，预留扩展
    bool isEmpty() const {return true;}
};
using ExportSet = ExportSettings;

struct GeneralSettings {
    QString language;
    bool checkForUpdates;
    QVector<Shortcut> shortcuts;
    bool enableLogging;
    QString logLevel;

    bool isEmpty() const
    {
        return language.isEmpty() && shortcuts.isEmpty();
    }
};
using GeneralSet = GeneralSettings;

struct ApplicationSettings {
    AppearanceSettings appearance;
    EditorSettings editor;
    CodeSettings code;
    MenuSettings menu;
    ExportSettings exportSet;
    GeneralSettings general;
    QDateTime lastModified;

    bool isEmpty() const
    {
        return appearance.isEmpty() && editor.isEmpty() && code.isEmpty() &&
               menu.isEmpty() && exportSet.isEmpty() && general.isEmpty();
    }
};
using AppSettings = ApplicationSettings;

#endif // SETTINGS_TYPES_H
