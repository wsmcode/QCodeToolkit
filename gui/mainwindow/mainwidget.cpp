#include "mainwidget.h"
#include "menubar.h"
#include "tabwidget.h"
#include "treepanel.h"
#include "logviewer.h"
#include "sqldatabase.h"
#include "databasemanager.h"
#include "stylemanager.h"

#include <QAction>
#include <QCloseEvent>
#include <QCoreApplication>
#include <QDebug>
#include <QDir>
#include <QPushButton>
#include <QSplitter>
#include <QVBoxLayout>
#include <settingmanager.h>

MainWidget::MainWidget(QWidget *parent)
    : FrameLessWindowBase(parent)
{
    resize(1000, 750);
    setWindowTitle("个人代码库");
    setWindowIcon(":/res/icon/code.svg");

    QObject::connect(StyleManager::getStyleManager(), &StyleManager::themeChanged, [=](StyleTheme theme){
        if(theme == StyleTheme::DarkTheme) setTheme(FTheme::Dark);
        else setTheme(FTheme::Light);
    });

    // 获取存储根路径
    QString exeDir = QCoreApplication::applicationDirPath();
    QString storagePath = QDir(exeDir + "/data/storage/").absolutePath();

    // 优先使用已存在的路径
    if(QDir(storagePath).exists()) m_rootPath = storagePath;
    else
    {
        // 不存在，创建目录
        m_rootPath = storagePath;
        QDir().mkpath(m_rootPath);
    }
    // 初始化数据库
    DatabaseManager::getDatabaseManager()->init(m_rootPath);

    initUI();

    // 加入日志视图（CTRL+L唤起）
    QAction *action = new QAction("logger", this);
    action->setShortcut(QKeySequence(Qt::CTRL + Qt::Key_L));
    // 连接动作的triggered信号到槽函数
    QObject::connect(action, &QAction::triggered, this, &MainWidget::showLogViewer);
    addAction(action);

    SettingManager::getSettingManager()->init(exeDir);
    QObject::connect(getSetButton(), &QPushButton::clicked, [=](){
        SettingManager::getSettingManager()->showDialog(this);
    });
    QObject::connect(SettingManager::getSettingManager(), &SettingManager::statusBarVisibilityChanged,
                     [=](bool visible){
        setStatusBarHide(!visible);
    });
    qInfo() << "MainWidget initialized successfully";
}

MainWidget::~MainWidget()
{
    if(m_logViewer)
    {
        delete m_logViewer;
        m_logViewer = nullptr;
    }
}

void MainWidget::showLogViewer()
{
    // 如果日志查看器不存在，则创建它
    if(!m_logViewer)
    {
        qInfo() << "Create log viewer";
        m_logViewer = new LogViewer(nullptr); // 创建为独立窗口
        QObject::connect(m_logViewer, &LogViewer::sig_close, [=](){
            delete m_logViewer;   // 销毁 LogViewer
            m_logViewer = nullptr;
        });
    }

    // 显示日志查看器
    m_logViewer->show();
    m_logViewer->raise();
    m_logViewer->activateWindow();
}

void MainWidget::closeEvent(QCloseEvent *event)
{
    // 关闭所有子窗口
    if(m_logViewer)
    {
        m_logViewer->close(); // 关闭 LogViewer
        delete m_logViewer;   // 销毁 LogViewer
        m_logViewer = nullptr;
    }

    // 接受关闭事件
    event->accept();
}

void MainWidget::initUI()
{
    QWidget *central = getCentralWidget();

    // 创建菜单栏，布局：放在最顶部
    m_menuBar = createMenuBar();
    // 创建文件树，布局：放在左侧
    m_treePanel = createTreePanel();
    // 创建选项卡小部件
    m_contentTabs = createTabWidget();

    //添加分裂器水平布局
    QSplitter* hSplitter = new QSplitter(Qt::Horizontal);
    // 设置分裂器属性
    hSplitter->setChildrenCollapsible(false);
    hSplitter->setHandleWidth(1);
    hSplitter->setObjectName("hSplitter");
    // 添加控件
    hSplitter->addWidget(m_treePanel);
    hSplitter->addWidget(m_contentTabs);
    // 设置初始分割比例 (1:4)

    //hSplitter->setSizes({width() / 5, width() * 4 / 5} );
    hSplitter->setStretchFactor(1, 1);

    // 添加垂直布局作为主布局
    QVBoxLayout* mainVLayout = new QVBoxLayout(central);
    mainVLayout->setContentsMargins(1, 0, 1, 0);
    mainVLayout->setSpacing(0);
    mainVLayout->addWidget(m_menuBar);
    mainVLayout->addWidget(hSplitter, 1);

    QObject::connect(m_treePanel, &TreePanel::projectItemClicked, [=](const QString &path){
        m_contentTabs->addNoteTab(path);
    });
    QObject::connect(m_contentTabs, &TabWidget::openNote, [=](const QString &path){
        m_contentTabs->addNoteTab(path);
    });

    QObject::connect(m_menuBar, &MenuBar::openNote, [=](const QString &path){
        m_contentTabs->addNoteTab(path);
    });

    QObject::connect(m_menuBar, &MenuBar::openTags, [=](){
        m_contentTabs->addTagsTab();
    });
}

MenuBar *MainWidget::createMenuBar()
{
    MenuBar* menuBar = new MenuBar;

    return menuBar;
}

TreePanel *MainWidget::createTreePanel()
{
    TreePanel *treePanel = new TreePanel;
    treePanel->setupTreeView(m_rootPath);
    return treePanel;
}

TabWidget *MainWidget::createTabWidget()
{
    TabWidget* tabWidget = new TabWidget;
    return tabWidget;
}
