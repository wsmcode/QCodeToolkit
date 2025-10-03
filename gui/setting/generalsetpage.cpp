#include "generalsetpage.h"
#include "shortcutsdialog.h"

#include <QCheckBox>
#include <QHeaderView>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QTableWidget>
#include <QVBoxLayout>
#include <combobox.h>
#include <stylemanager.h>

GeneralSetPage::GeneralSetPage(QWidget *parent) : SettingsPage(parent)
{
    initUI();
}

QString GeneralSetPage::title() const
{
    return "通用";
}

void GeneralSetPage::load(const AppSettings &settings)
{
    m_languageComboBox->setCurrentText(settings.general.language);
    m_logEnableCheckBox->setChecked(settings.general.enableLogging);
    m_logLevelComboBox->setCurrentText(settings.general.logLevel);
}

void GeneralSetPage::save(AppSettings &settings)
{
    settings.general.language = m_languageComboBox->currentText();
    settings.general.enableLogging = m_logEnableCheckBox->isChecked();
    settings.general.logLevel = m_logLevelComboBox->currentText();
}

void GeneralSetPage::initUI()
{
    QWidget *mainWidget = new QWidget;
    setWidget(mainWidget);
    mainWidget->setObjectName("SettingScrollWidget");
    StyleManager::getStyleManager()->registerWidget(mainWidget);

    QVBoxLayout *mainVLayout = new QVBoxLayout(mainWidget);
    mainVLayout->setSpacing(10);
    mainVLayout->setContentsMargins(10, 10, 0, 10);

    QLabel *titleLabel = new QLabel("通用");
    titleLabel->setObjectName("SettingTitleLabel");

    // 语言设置部分
    QLabel *languageLabel = new QLabel("语言设置");
    languageLabel->setObjectName("SettingSubtitleLabel");

    QHBoxLayout *languageHLayout = new QHBoxLayout;
    languageHLayout->setSpacing(10);
    languageHLayout->setContentsMargins(0, 0, 0, 0);

    QLabel *languageSelectLabel = new QLabel("界面语言");
    m_languageComboBox = new ComboBox;
    m_languageComboBox->addItems({"简体中文", "English", "日本語", "繁體中文"});
    m_languageComboBox->setCurrentText("简体中文");

    languageHLayout->addWidget(languageSelectLabel);
    languageHLayout->addWidget(m_languageComboBox);
    languageHLayout->addStretch();

    // 更新设置部分
    QLabel *updateLabel = new QLabel("更新设置");
    updateLabel->setObjectName("SettingSubtitleLabel");

    QHBoxLayout *updateHLayout = new QHBoxLayout;
    updateHLayout->setSpacing(10);
    updateHLayout->setContentsMargins(0, 0, 0, 0);

    m_checkUpdateButton = new QPushButton("检查更新");
    QLabel *versionLabel = new QLabel("当前版本: v1.0.0");

    updateHLayout->addWidget(m_checkUpdateButton);
    updateHLayout->addWidget(versionLabel);
    updateHLayout->addStretch();

    // 全局快捷键设置部分
    QLabel *shortcutsLabel = new QLabel("全局快捷键");
    shortcutsLabel->setObjectName("SettingSubtitleLabel");

    // 创建快捷键布局
    QHBoxLayout *shortcutsHLayout = new QHBoxLayout;
    updateHLayout->setSpacing(0);
    updateHLayout->setContentsMargins(0, 0, 0, 0);
    QGridLayout *shortcutsGLayout = new QGridLayout;
    shortcutsGLayout->setSpacing(10);
    shortcutsGLayout->setContentsMargins(0, 0, 0, 0);

    // 快捷键数据
    QStringList actions = {"打开剪切板"};
    QStringList defaultShortcuts = {"Ctrl+Shift+C"};

    for(int i = 0; i < actions.size(); i++)
    {
        QLabel *actionLabel = new QLabel(actions[i]);
        QLineEdit *shortcutEdit = new QLineEdit(defaultShortcuts[i]);
        shortcutEdit->setFixedWidth(200);
        shortcutEdit->setPlaceholderText("输入快捷键");
        QPushButton *resetButton = new QPushButton("恢复默认");
        resetButton->setFixedWidth(100);

        // 连接恢复默认按钮
        QObject::connect(resetButton, &QPushButton::clicked, [=](){
            shortcutEdit->setText(defaultShortcuts[i]);
        });

        shortcutsGLayout->addWidget(actionLabel, i, 0, Qt::AlignRight);
        shortcutsGLayout->addWidget(shortcutEdit, i, 1);
        shortcutsGLayout->addWidget(resetButton, i, 2, Qt::AlignCenter);
    }

    // 内置快捷键按钮
    QPushButton *builtinShortcutsButton = new QPushButton("内置快捷键");
    builtinShortcutsButton->setFixedWidth(100);

    // 连接内置快捷键按钮
    QObject::connect(builtinShortcutsButton, &QPushButton::clicked, [=](){
        ShortcutsDialog *dialog = new ShortcutsDialog(this);
        dialog->show();
    });

    shortcutsGLayout->addWidget(builtinShortcutsButton, actions.size() + 1, 2);

    shortcutsHLayout->addLayout(shortcutsGLayout);
    shortcutsHLayout->addStretch();

    // 开发者设置部分
    QLabel *developerLabel = new QLabel("开发者设置");
    developerLabel->setObjectName("SettingSubtitleLabel");

    // 日志开关
    m_logEnableCheckBox = new QCheckBox("启用日志记录");
    m_logEnableCheckBox->setChecked(true);

    // 日志等级
    QHBoxLayout *logLevelHLayout = new QHBoxLayout;
    logLevelHLayout->setSpacing(10);
    logLevelHLayout->setContentsMargins(0, 0, 0, 0);

    QLabel *logLevelLabel = new QLabel("日志等级");
    m_logLevelComboBox = new ComboBox;
    QStringList logLevels = {"调试", "信息", "警告", "错误", "致命"};
    m_logLevelComboBox->addItems(logLevels);
    m_logLevelComboBox->setCurrentText("信息");

    logLevelHLayout->addWidget(logLevelLabel);
    logLevelHLayout->addWidget(m_logLevelComboBox);
    logLevelHLayout->addStretch();

    mainVLayout->addWidget(titleLabel);
    mainVLayout->addSpacing(20);
    mainVLayout->addWidget(languageLabel);
    mainVLayout->addLayout(languageHLayout);
    mainVLayout->addSpacing(20);
    mainVLayout->addWidget(updateLabel);
    mainVLayout->addLayout(updateHLayout);
    mainVLayout->addSpacing(20);
    mainVLayout->addWidget(shortcutsLabel);
    mainVLayout->addLayout(shortcutsHLayout);
    mainVLayout->addSpacing(20);
    mainVLayout->addWidget(developerLabel);
    mainVLayout->addWidget(m_logEnableCheckBox);
    mainVLayout->addLayout(logLevelHLayout);
    mainVLayout->addStretch();
}

void GeneralSetPage::initConnections()
{
    // 连接信号和槽
    QObject::connect(m_languageComboBox, &ComboBox::currentTextChanged, [=](const QString& text){
        emit settingChanged("general", "language", text);
    });

    QObject::connect(m_checkUpdateButton, &QPushButton::clicked, [=](){
        emit settingChanged("general", "checkForUpdates", true);
    });

    QObject::connect(m_logEnableCheckBox, &QCheckBox::toggled, [=](bool checked){
        emit settingChanged("general", "enableLogging", checked);
    });

    QObject::connect(m_logLevelComboBox, &ComboBox::currentTextChanged, [=](const QString& text){
        emit settingChanged("general", "logLevel", text);
    });
}
