#include "appearancesetpage.h"
#include "codesetpage.h"
#include "editorsetpage.h"
#include "exportsetpage.h"
#include "generalsetpage.h"
#include "menusetpage.h"
#include "settingdialog.h"

#include <QHBoxLayout>
#include <QListWidget>
#include <QPushButton>
#include <QStackedWidget>
#include <QDebug>
#include <stylemanager.h>
#include <settingmanager.h>
#include <QKeyEvent>

SettingDialog::SettingDialog(QWidget *parent) : Dialog(parent)
{

    initUI();
    initPages();
}

void SettingDialog::load(const AppSettings &settings)
{
    m_currentSettings = settings;

    // 为每个页面加载设置
    for(auto *page : m_pages)
    {
        page->load(settings);
    }
}

AppSettings SettingDialog::currentSettings()
{
    AppSettings settings = m_currentSettings;

    // 从每个页面收集当前设置
    for(SettingsPage *page : m_pages)
    {
        page->save(settings);
    }
    return settings;
}

void SettingDialog::initUI()
{
    m_centralWidget->setFixedHeight(600);

    m_navigationList = new QListWidget(this);
    m_navigationList->setFixedWidth(120);
    m_navigationList->setFrameShape(QFrame::NoFrame);
    m_navigationList->setFocusPolicy(Qt::NoFocus);
    m_navigationList->setContentsMargins(0, 0, 0, 0);

    m_stackedSettings = new QStackedWidget;
    m_stackedSettings->setFixedWidth(700);

    QHBoxLayout *mainHLayout = new QHBoxLayout(m_centralWidget);
    mainHLayout->setSpacing(0);
    mainHLayout->setContentsMargins(10, 30, 1, 10);
    mainHLayout->addWidget(m_navigationList);
    mainHLayout->addWidget(m_stackedSettings);

    m_centralWidget->setFixedWidth(m_navigationList->width() + m_stackedSettings->width() +
                  mainHLayout->contentsMargins().left() + mainHLayout->contentsMargins().right());

    // 居中显示
    qDebug() << parentWidget();


    if(parentWidget())
    {
        int x = parentWidget()->x() + parentWidget()->width() / 2 - m_centralWidget->width() / 2;
        int y = parentWidget()->y() + parentWidget()->height() / 2 - m_centralWidget->height() / 2;
        move(x, y);
    }
}

void SettingDialog::initPages()
{
    // 注册所有设置页面
    registerPage(new AppearanceSetPage(this));
    registerPage(new EditorSetPage(this));
    registerPage(new CodeSetPage(this));
    registerPage(new MenuSetPage(this));
    registerPage(new ExportSetPage(this));
    registerPage(new GeneralSetPage(this));

    m_navigationList->setCurrentRow(0);

    // 连接信号
    QObject::connect(m_navigationList, &QListWidget::currentRowChanged, [=](int index){
        m_stackedSettings->setCurrentIndex(index);
    });
}

void SettingDialog::registerPage(SettingsPage *page)
{
    if(!page) return;

    // 添加到分类列表
    m_navigationList->addItem(page->title());

    // 添加到堆栈窗口
    m_stackedSettings->addWidget(page);

    // 存储页面引用
    m_pages[page->title()] = page;

    // 连接设置变更信号
    connect(page, &SettingsPage::settingChanged,[=]
            (const QString& category, const QString& key, const QVariant& value){
        SettingManager::getSettingManager()->handleSettingChange(category, key, value);
    });
}
