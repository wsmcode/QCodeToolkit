#ifndef SETTINGSPAGE_H
#define SETTINGSPAGE_H

#include <QScrollArea>
#include "settings_types.h"

class SettingsPage : public QScrollArea
{
    Q_OBJECT
public:
    explicit SettingsPage(QWidget *parent = nullptr);

    virtual QString title() const = 0;
    virtual void load(const AppSettings&) = 0;
    virtual void save(AppSettings&) = 0;

signals:
    // 通用设置变更信号
    void settingChanged(const QString& category, const QString& key, const QVariant& value);
    void settingsRequireApply(); // 需要应用设置的信号

    void appearanceSettingsChanged(const AppearanceSet &settings);
    void editorSettingsChanged(const EditorSet &settings);
    void codeSettingsChanged(const CodeSet &settings);
    void menuSettingsChanged(const MenuSet &settings);
    void exportSettingsChanged(const ExportSet &settings);
    void generalSettingsChanged(const GeneralSet &settings);

protected:
    virtual void initUI() = 0;
};

#endif // SETTINGSPAGE_H
