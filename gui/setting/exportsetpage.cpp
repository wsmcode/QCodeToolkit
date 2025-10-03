#include "exportsetpage.h"

#include <QLabel>
#include <QVBoxLayout>
#include <stylemanager.h>

ExportSetPage::ExportSetPage(QWidget *parent) : SettingsPage(parent)
{
    initUI();
}

QString ExportSetPage::title() const
{
    return "导出";
}

void ExportSetPage::load(const AppSettings &)
{

}

void ExportSetPage::save(AppSettings &)
{

}

void ExportSetPage::initUI()
{
    QWidget *mainWidget = new QWidget;
    setWidget(mainWidget);
    mainWidget->setObjectName("SettingScrollWidget");
    StyleManager::getStyleManager()->registerWidget(mainWidget);

    QVBoxLayout *mainVLayout = new QVBoxLayout(mainWidget);
    mainVLayout->setSpacing(10);
    mainVLayout->setContentsMargins(10, 10, 0, 10);

    QLabel *titleLabel = new QLabel("导出");
    titleLabel->setObjectName("SettingTitleLabel");

    // 格式设置部分
    QLabel *label = new QLabel("当前版本未实现");
    label->setObjectName("SettingSubtitleLabel");

    mainVLayout->addWidget(titleLabel);
    mainVLayout->addSpacing(20);
    mainVLayout->addWidget(label);
    mainVLayout->addStretch();
}
