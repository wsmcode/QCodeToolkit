#include "hometab.h"
#include "stylemanager.h"

#include <QFrame>
#include <QLabel>
#include <QListWidget>
#include <QVBoxLayout>
#include <QDebug>
#include <databasemanager.h>
#include <QFile>
#include <projectmanager.h>
#include <QPropertyAnimation>
#include <rotationbutton.h>
#include <imagelabel.h>

HomeTab::HomeTab(QWidget *parent) : QFrame(parent)
{
    setObjectName("HomeTab");
    StyleManager::getStyleManager()->registerWidget(this);
    setFrameShape(QFrame::NoFrame);

    initUI();
    // 连接项目列表变化信号
    connect(ProjectManager::getProjectManager(), &ProjectManager::projectListChanged,
            this, &HomeTab::onProjectListChanged);

    qInfo() << "HomeTab initialized successfully";
}

void HomeTab::onProjectItemClicked(QListWidgetItem *item)
{
    QString projectPath = item->data(Qt::UserRole).toString();
    if(!projectPath.isEmpty()) emit projectItemClicked(projectPath);
}

void HomeTab::onProjectListChanged()
{
    loadRecentProjects();
}

void HomeTab::initUI()
{
    QVBoxLayout* homeLayout = new QVBoxLayout(this);
    homeLayout->setContentsMargins(30, 20, 30, 20);
    homeLayout->setSpacing(20);

    // === 最近项目区 ===
    QWidget* recentGroup = new QWidget;
    recentGroup->setObjectName("recentGroup");

    QVBoxLayout* recentLayout = new QVBoxLayout(recentGroup);
    recentLayout->setContentsMargins(0, 0, 0, 0);

    // 标题和刷新按钮
    QHBoxLayout *headerLayout = new QHBoxLayout();
    headerLayout->setContentsMargins(0, 0, 0, 0);

    QLabel* recentTitle = new QLabel("最近项目");
    recentTitle->setObjectName("recentTitle");

    RotationButton *refreshButton = new RotationButton;
    refreshButton->setObjectName("HomeTabRefreshButton");
    refreshButton->setFixedSize(24, 24);
    // 创建旋转动画
    QPropertyAnimation *rotationAnimation = new QPropertyAnimation(refreshButton, "rotation", this);
    rotationAnimation->setDuration(500);  // 动画持续时间
    rotationAnimation->setStartValue(0);
    rotationAnimation->setEndValue(360);
    rotationAnimation->setEasingCurve(QEasingCurve::OutCubic);  // 缓动曲线

    // 连接按钮点击信号
    QObject::connect(refreshButton, &QPushButton::clicked, [=](){
        // 开始旋转动画
        rotationAnimation->start();
        // 执行刷新操作
        loadRecentProjects();
    });

    headerLayout->addWidget(recentTitle);
    headerLayout->addStretch();
    headerLayout->addWidget(refreshButton);

    // 项目列表
    m_projectList = new QListWidget;
    m_projectList->setObjectName("projectList");
    m_projectList->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    m_projectList->setVerticalScrollMode(QAbstractItemView::ScrollPerPixel);
    m_projectList->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    m_projectList->setFocusPolicy(Qt::NoFocus);

    m_projectList->setResizeMode(QListWidget::Adjust);
    m_projectList->setMovement(QListWidget::Static);

    // 添加示例项目
    recentLayout->addLayout(headerLayout);
    recentLayout->addWidget(m_projectList, 1);
    homeLayout->addWidget(recentGroup, 1);

    QObject::connect(m_projectList, &QListWidget::itemClicked, this, &HomeTab::onProjectItemClicked);

    // 加载最近项目
    loadRecentProjects();
}

void HomeTab::loadRecentProjects()
{
    m_projectList->clear();

    // 从数据库获取所有项目
    DatabaseManager *db = DatabaseManager::getDatabaseManager();
    QVector<Note> allNotes = db->allNotes();

    // 按修改时间排序（最近修改的在前）
    std::sort(allNotes.begin(), allNotes.end(), [](const Note &a, const Note &b){
        Node nodeA = DatabaseManager::getDatabaseManager()->node(a.nodeId);
        Node nodeB = DatabaseManager::getDatabaseManager()->node(b.nodeId);
        return nodeA.modified > nodeB.modified;
    });

    // 只显示最近15个项目
    int count = qMin(15, allNotes.size());
    for(int i = 0; i < count; i++)
    {
        const Note &note = allNotes[i];
        Node node = db->node(note.nodeId);

        // 获取项目完整路径
        QString projectPath = db->getNodeFullPath(note.nodeId);

        // 获取父节点名称
        QString parentName = "根目录";
        if(node.parentId > 0)
        {
            Node parentNode = db->node(node.parentId);
            if(parentNode.id > 0) parentName = parentNode.name;
        }

        addProjectItem(note.projectName, note.imagePath, note.nodeId, projectPath, parentName, node.modified);
    }

    // 如果没有项目，显示提示信息
    if(m_projectList->count() == 0)
    {
        QListWidgetItem *emptyItem = new QListWidgetItem("暂无项目");
        emptyItem->setTextAlignment(Qt::AlignCenter);
        emptyItem->setFlags(Qt::NoItemFlags); // 不可点击
        m_projectList->addItem(emptyItem);
    }
}

void HomeTab::addProjectItem(const QString &projectName, const QString &imagePath,
                             int nodeId, const QString &projectPath,
                             const QString &parentName, const QDateTime &modifiedTime)
{
    QListWidgetItem *item = new QListWidgetItem(m_projectList);
    item->setData(Qt::UserRole, projectPath); // 存储项目路径

    QWidget *widget = new QWidget;
    QHBoxLayout *hLayout = new QHBoxLayout(widget);
    hLayout->setSpacing(12);
    hLayout->setContentsMargins(10, 8, 10, 8);

    // 左侧图片
    ImageLabel *imageLabel = new ImageLabel;
    imageLabel->setObjectName("projectImage");

    QPixmap pixmap;
    if(!imagePath.isEmpty() && QFile::exists(imagePath)) pixmap = QPixmap(imagePath);

    if(!pixmap.isNull())
    {
        // 缩放图片以适应标签大小
        pixmap = pixmap.scaled(40, 40, Qt::KeepAspectRatio, Qt::SmoothTransformation);
        imageLabel->setPixmap(pixmap);
    }

    // 右侧信息区域
    QWidget *infoWidget = new QWidget;
    QVBoxLayout *infoLayout = new QVBoxLayout(infoWidget);
    infoLayout->setContentsMargins(0, 0, 0, 0);
    infoLayout->setSpacing(4);

    // 项目名
    QLabel *nameLabel = new QLabel(projectName);
    nameLabel->setObjectName("projectName");

    // 修改日期
    QString modifiedText = modifiedTime.toString("yyyy-MM-dd");
    QLabel *modifiedLabel = new QLabel(modifiedText);
    modifiedLabel->setObjectName("projectModified");

    // 父节点名称
    QLabel *parentLabel = new QLabel(parentName);
    parentLabel->setObjectName("projectParent");

    // NodeId（右下角，小字）
    QLabel *idLabel = new QLabel(QString("ID: %1").arg(nodeId));
    idLabel->setObjectName("projectId");

    // 底部信息区域（NodeId和间距）
    QWidget *bottomWidget = new QWidget;
    QHBoxLayout *bottomLayout = new QHBoxLayout(bottomWidget);
    bottomLayout->setContentsMargins(0, 0, 0, 0);
    bottomLayout->setSpacing(0);

    bottomLayout->addWidget(modifiedLabel);
    bottomLayout->addStretch();
    bottomLayout->addWidget(parentLabel);
    bottomLayout->addSpacing(5);
    bottomLayout->addWidget(idLabel);

    infoLayout->addWidget(nameLabel);
    infoLayout->addStretch();
    infoLayout->addWidget(bottomWidget);

    hLayout->addWidget(imageLabel);
    hLayout->addWidget(infoWidget, 1);

    widget->setLayout(hLayout);
    item->setSizeHint(widget->sizeHint());

    m_projectList->setItemWidget(item, widget);
}
