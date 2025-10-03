#include "menusetpage.h"

#include <QCheckBox>
#include <QLabel>
#include <QVBoxLayout>
#include <QVariant>
#include <stylemanager.h>

MenuSetPage::MenuSetPage(QWidget *parent) : SettingsPage(parent)
{
    initUI();
    m_codeCompletionListCheckBox->setEnabled(false);
    m_filterCheckBox->setEnabled(false);
}

QString MenuSetPage::title() const
{
    return "菜单栏";
}

void MenuSetPage::load(const AppSettings &settings)
{
    m_newProjectCheckBox->setChecked(settings.menu.showNewItem);
    m_codeCompletionListCheckBox->setChecked(settings.menu.showCodeCompletionList);
    m_filterCheckBox->setChecked(settings.menu.showFilter);
    m_quickSettingCheckBox->setChecked(settings.menu.showQuickSettings);
}

void MenuSetPage::save(AppSettings &settings)
{
    settings.menu.showNewItem = m_newProjectCheckBox->isChecked();
    settings.menu.showCodeCompletionList = m_codeCompletionListCheckBox->isChecked();
    settings.menu.showFilter = m_filterCheckBox->isChecked();
    settings.menu.showQuickSettings = m_quickSettingCheckBox->isChecked();
}

void MenuSetPage::initUI()
{
    QWidget *mainWidget = new QWidget;
    setWidget(mainWidget);
    mainWidget->setObjectName("SettingScrollWidget");
    StyleManager::getStyleManager()->registerWidget(mainWidget);

    QVBoxLayout *mainVLayout = new QVBoxLayout(mainWidget);
    mainVLayout->setSpacing(10);
    mainVLayout->setContentsMargins(10, 10, 0, 10);

    QLabel *titleLabel = new QLabel("菜单栏");
    titleLabel->setObjectName("SettingTitleLabel");

    // 格式设置部分
    QLabel *showLabel = new QLabel("显示");
    showLabel->setObjectName("SettingSubtitleLabel");

    // 新项目
    m_newProjectCheckBox = new QCheckBox("新项目");
    m_newProjectCheckBox->setChecked(true); // 默认选中

    // 代码提示词列表
    m_codeCompletionListCheckBox = new QCheckBox("代码提示词列表");
    m_codeCompletionListCheckBox->setChecked(true); // 默认选中

    // 筛选
    m_filterCheckBox = new QCheckBox("筛选");
    m_filterCheckBox->setChecked(true); // 默认选中

    // 快速设置
    m_quickSettingCheckBox = new QCheckBox("快速设置");
    m_quickSettingCheckBox->setChecked(true); // 默认选中

    mainVLayout->addWidget(titleLabel);
    mainVLayout->addSpacing(20);
    mainVLayout->addWidget(showLabel);
    mainVLayout->addWidget(m_newProjectCheckBox);
    mainVLayout->addWidget(m_codeCompletionListCheckBox);
    mainVLayout->addWidget(m_filterCheckBox);
    mainVLayout->addWidget(m_quickSettingCheckBox);
    mainVLayout->addStretch();
}

void MenuSetPage::initConnections()
{
    QObject::connect(m_newProjectCheckBox, &QCheckBox::toggled, [=](bool checked){
        emit settingChanged("menu", "showNewItem", checked);
    });

    QObject::connect(m_codeCompletionListCheckBox, &QCheckBox::toggled, [=](bool checked){
        emit settingChanged("menu", "showCodeCompletionList", checked);
    });

    QObject::connect(m_filterCheckBox, &QCheckBox::toggled, [=](bool checked){
        emit settingChanged("menu", "showFilter", checked);
    });

    QObject::connect(m_quickSettingCheckBox, &QCheckBox::toggled, [=](bool checked){
        emit settingChanged("menu", "showQuickSettings", checked);
    });
}
