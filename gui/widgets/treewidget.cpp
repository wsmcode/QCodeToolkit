#include "treewidget.h"

#include "projectmanager.h"
#include "databasemanager.h"
#include "stylemanager.h"
#include "fontmanager.h"

#include <QApplication>
#include <QDir>
#include <QJsonObject>
#include <QFileInfo>
#include <QJsonParseError>
#include <QDebug>
#include <QMenu>
#include <QMouseEvent>
#include <QMessageBox>
#include <QFileDialog>
#include <QSettings>
#include <QStandardPaths>
#include <QTimer>

TreeWidget::TreeWidget(QWidget *parent) : QTreeWidget(parent)
{
    m_projectManager = ProjectManager::getProjectManager();

    FontManager::getFontManager()->registerWidget(this);

    // 设置树视图属性
    setHeaderHidden(true);
    setIconSize(QSize(16, 16)); // 设置图标大小
    setAnimated(true); // 启用展开/折叠动画
    setIndentation(10);// 设置子项缩进像素
    setRootIsDecorated(false); // 去掉左边的箭头
    setFrameShape(QFrame::NoFrame); // 去掉自带的边框
    setEditTriggers(QAbstractItemView::NoEditTriggers); // 禁用自动编辑触发
    setSelectionMode(QAbstractItemView::ExtendedSelection);
    setFocusPolicy(Qt::NoFocus);

    // 使用样式表隐藏所有箭头
    setObjectName("TreeWidget");
    StyleManager::getStyleManager()->registerWidget(this);

    // 启用上下文菜单
    setContextMenuPolicy(Qt::CustomContextMenu);
    QObject::connect(this, &TreeWidget::customContextMenuRequested,
                     this, &TreeWidget::onCustomContextMenu);

    // 连接展开和折叠信号
    QObject::connect(this, &QTreeWidget::itemExpanded, this, &TreeWidget::onItemExpanded);

    // 设置默认图标
//    m_folderIcon = QApplication::style()->standardIcon(QStyle::SP_DirIcon);
//    m_fileIcon = QApplication::style()->standardIcon(QStyle::SP_FileIcon);
    setIcons();

    QObject::connect(StyleManager::getStyleManager(), &StyleManager::themeChanged, [=](){setIcons();});
    QObject::connect(this, &QTreeWidget::itemClicked, [=](QTreeWidgetItem *item, int) {
        // 切换展开/折叠状态
        if(!item) return;
        QString type = item->data(0, Qt::UserRole + 1).toString();
        QString path = item->data(0, Qt::UserRole).toString();

        if(type == "FOLDER") item->setExpanded(!item->isExpanded());
        else if(type == "PROJECT_FOLDER")
        {
            emit projectItemClicked(path);
        }

    });
    QObject::connect(this, &TreeWidget::itemChanged, this, &TreeWidget::onItemChanged);
    qInfo() << "TreeWidget initialized successfully";
}

void TreeWidget::setupTreeView(const QString& url)
{
    // 保存当前展开状态
    saveExpandedState();

    clear();
    m_loadedPaths.clear();

    // 将url的相对路径转为绝对路径
    QDir dir(url);
    if(!dir.exists())
    {
        qWarning() << "The path does not exist: " << url;
    }
    m_rootPath  = dir.absolutePath(); // 不包含文件名

    // 确保根目录有标识文件
    if(!m_projectManager->isRepositoryItem(m_rootPath))
    {
        QFile file(m_rootPath + "/" + m_projectManager->repositoryId());
        if(file.open(QIODevice::WriteOnly))
        {
            file.write("Code Repository Root Directory");
            file.close();
        }
    }

    // 加载目录结构
    // 不创建根节点，直接加载根目录内容作为顶级项
    loadDir(m_rootPath, invisibleRootItem(), m_projectManager->rootNodeId());
    if(!m_loadedPaths.contains(m_rootPath)) m_loadedPaths.insert(m_rootPath);

    // 恢复展开状态
    restoreExpandedState();
}

QSize TreeWidget::sizeHint() const
{
    return QSize(200, 200);
}

void TreeWidget::onCustomContextMenu(const QPoint &pos)
{
    QTreeWidgetItem* item = itemAt(pos);
    QMenu menu;

    if(item)
    {
        QString path = item->data(0, Qt::UserRole).toString();
        QString type = item->data(0, Qt::UserRole + 1).toString();
        int nodeId = item->data(0, Qt::UserRole + 3).toInt(); // 获取节点ID

//        qDebug() << "type" << type;
//        qDebug() << "path" << path;
        if(type == "FOLDER")
        {
            if(m_projectManager->hasCategoryMarker(path))
            {
                menu.addAction("新建子分类", this, [=](){
                    m_projectManager->createCategory(path, nodeId);
                    setupTreeView(m_rootPath);
                });
            }
            else if(m_projectManager->hasProjectMarker(path))
            {
                menu.addAction("新建项目", this, [=](){
                    m_projectManager->createProject(path, nodeId);
                    setupTreeView(m_rootPath);
                });
            }
            else if(m_projectManager->isRepositoryItem(path))
            {
                menu.addAction("新建子分类", this, [=](){
                    m_projectManager->createCategory(path, nodeId);
                    setupTreeView(m_rootPath);
                });
                menu.addAction("新建项目", this, [=](){
                    m_projectManager->createProject(path, nodeId);
                    setupTreeView(m_rootPath);
                });
            }
        }
        else if(type == "PROJECT_FOLDER")
        {
            // 为项目文件夹添加设置演示图片的菜单项
            menu.addAction("设置演示图片", this, &TreeWidget::onSetDemoImage);
            menu.addSeparator();
        }
        menu.addSeparator();
        menu.addAction("重命名", this, [=](){
            item->setFlags(Qt::ItemIsEnabled | Qt::ItemIsSelectable | Qt::ItemIsEditable);
            editItem(item, 0);
        });
        menu.addAction("删除", this, [=](){
            m_projectManager->removeItem(path, nodeId);
            setupTreeView(m_rootPath);
        });
    }
    else
    {
        menu.addAction("新建子分类", this, [=](){
            m_projectManager->createCategory(m_rootPath, m_projectManager->rootNodeId());
            setupTreeView(m_rootPath);
        });
    }
    // 公共菜单项（无论是否选中项都显示）

    menu.addSeparator();
    menu.addAction("刷新视图", this, [=](){
        setupTreeView(m_rootPath);
    });

    menu.exec(viewport()->mapToGlobal(pos));
}

void TreeWidget::onItemChanged(QTreeWidgetItem *item, int column)
{
    if(column != 0) return;

    if(m_isOnItemChanged) return;
    m_isOnItemChanged = true;

    QString newName = item->text(0);
    QString oldName = item->data(0, Qt::UserRole + 2).toString();
    QString path = item->data(0, Qt::UserRole).toString();
    int nodeId = item->data(0, Qt::UserRole + 3).toInt(); // 获取节点ID

    QString parentPath = QFileInfo(path).path();
    if(!oldName.isEmpty() && newName != oldName)
    {
        QString sanitizedName = m_projectManager->sanitizeFileName(newName);
        if(sanitizedName != newName)
        {
            item->setText(0, sanitizedName);
            newName = sanitizedName;
        }

        if(newName.isEmpty())
        {
            item->setText(0, oldName);
            m_isOnItemChanged = false;
            return;
        }

        newName = m_projectManager->autoRename(newName, parentPath);

        if(newName == oldName)
        {
            m_isOnItemChanged = false;
            return;
        }

        // 计算新的完整路径
        QString newPath = parentPath + "/" + newName;

        // 先更新数据，再执行重命名操作
        item->setData(0, Qt::UserRole, newPath);
        item->setData(0, Qt::UserRole + 2, newName);

        // 文件重命名
        bool renameSuccess = m_projectManager->renameItem(newName, path, nodeId);

        if(!renameSuccess)
        {
            // 如果重命名失败，恢复原来的数据
            item->setText(0, oldName);
            item->setData(0, Qt::UserRole, path);
            item->setData(0, Qt::UserRole + 2, oldName);
            m_isOnItemChanged = false;
            return;
        }
        else
        {
            // 刷新当前项目
            if(QTreeWidgetItem* parent = item->parent()) refreshItem(parent);
            else setupTreeView(m_rootPath);
        }
    }

    m_isOnItemChanged = false;
}

void TreeWidget::onItemExpanded(QTreeWidgetItem *item)
{
    QString path = item->data(0, Qt::UserRole).toString();
    QString type = item->data(0, Qt::UserRole + 1).toString();
    int nodeId = item->data(0, Qt::UserRole + 3).toInt(); // 获取节点ID

    // 只有目录类型才需要加载子项
    if(type != "FOLDER") return;

    // 如果尚未加载，则加载子项
    if(!m_loadedPaths.contains(path))
    {
        loadDir(path, item, nodeId);
        m_loadedPaths.insert(path);
    }
}

void TreeWidget::refreshItem(QTreeWidgetItem *item)
{
    if(!item || m_isRefreshing) return;
    m_isRefreshing = true;

    QString path = item->data(0, Qt::UserRole).toString();
    int nodeId = item->data(0, Qt::UserRole + 3).toInt();

    // 保存当前展开状态
    bool wasExpanded = item->isExpanded();

    // 移除所有子项
    while(item->childCount() > 0)
    {
        QTreeWidgetItem* child = item->child(0);
        item->removeChild(child);
        delete child;
    }

    // 重新加载子项
    loadDir(path, item, nodeId);

    // 恢复展开状态
    if(wasExpanded) item->setExpanded(true);
    m_isRefreshing = false;
}

void TreeWidget::onSetDemoImage()
{
    QTreeWidgetItem* item = currentItem();
    if(!item)
    {
        QMessageBox::warning(this, "警告", "请先选择一个项目");
        return;
    }

    QString type = item->data(0, Qt::UserRole + 1).toString();
    if(type != "PROJECT_FOLDER")
    {
        QMessageBox::warning(this, "警告", "只能为项目文件夹设置演示图片");
        return;
    }

    QString projectPath = item->data(0, Qt::UserRole).toString();
    QString metaPath = projectPath + "/meta.ctk";

    // 获取应用程序设置
    QSettings settings("QCodeToolkit");

    // 获取上次使用的目录，如果没有则使用系统图片目录
    QString lastDir = settings.value("lastImageDir", "").toString();
    // 确保目录存在
    QDir dir(lastDir);
    if(!dir.exists())
    {
        lastDir = QStandardPaths::writableLocation(QStandardPaths::HomeLocation);
    }

    QString imagePath = QFileDialog::getOpenFileName(this, "选择图片", lastDir,
                                                   "图片文件 (*.png *.jpg *.jpeg "
                                                   "*.bmp *.gif *.svg "
                                                   "*.webp *.ico)");

    if(imagePath.isEmpty()) return; // 用户取消了选择
    // 保存本次使用的目录
    QFileInfo fileInfo(imagePath);
    settings.setValue("lastImageDir", fileInfo.path());

    // 验证图片文件
    QFileInfo imageInfo(imagePath);
    if(!imageInfo.exists() || !imageInfo.isFile())
    {
        QMessageBox::warning(this, "错误", "选择的文件不存在或不是有效的图片文件");
        return;
    }

    // 获取项目配置
    MetaCtk *config = m_projectManager->getMetaCtk(metaPath);
    if(!config)
    {
        QMessageBox::warning(this, "错误", "无法加载项目配置");
        return;
    }

    // 加载配置
    if(!config->load())
    {
        QMessageBox::warning(this, "错误", "无法加载项目配置文件");
        return;
    }

    // 处理图片路径 - 复制到项目目录中
    QString projectDir = QFileInfo(m_rootPath).path();
    QString imagesDir = projectDir + "/images";

    // 创建images目录（如果不存在）
    QDir pDir(projectDir);
    if(!pDir.exists("images"))
    {
        if(!pDir.mkdir("images"))
        {
            QMessageBox::warning(this, "错误", "无法创建图片目录");
            return;
        }
    }

    // 生成新的文件名（避免重名）
    QString newFileName = "demo_" + QDateTime::currentDateTime().toString("yyyyMMdd_hhmmss") + "." + imageInfo.suffix();
    QString newImagePath = imagesDir + "/" + newFileName;

    // 复制图片到项目目录
    if(!QFile::copy(imagePath, newImagePath))
    {
        QMessageBox::warning(this, "错误", "无法复制图片到项目目录");
        return;
    }

    // 更新配置中的演示图片路径
    config->modifyDemoImagePath(newImagePath);

    // 保存配置
    if(!config->save())
    {
        QMessageBox::warning(this, "错误", "无法保存项目配置");
        // 删除已复制的图片
        QFile::remove(newImagePath);
        return;
    }

    // 更新数据库中的图片路径
    int nodeId = item->data(0, Qt::UserRole + 3).toInt();
    if(nodeId > 0)
    {
        Note note = m_projectManager->getDbManager()->note(nodeId);
        if(note.nodeId > 0)
        {
            note.imagePath = newImagePath;
            m_projectManager->getDbManager()->updateNote(note);
        }
    }

    // 更新项目图标
    QIcon newIcon = loadIconFromMetaCtk(projectPath);
    if(!newIcon.isNull())
    {
        item->setIcon(0, newIcon);
    }

    QMessageBox::information(this, "成功", "演示图片设置成功");
}

//// 当前为递归，以后优化可以改为懒加载
//void TreeWidget::loadDir(const QString &path, QTreeWidgetItem *parentItem)
//{
//    if(!m_projectManager->isRepositoryItem(path))
//    {
//        qWarning() << "It is not the repository directory: " << path;
//        return;
//    }
//    QDir dir(path);
//    if(!dir.exists()) return;
//    // 获取当前目录配置
//    //QJsonObject localConfig = m_configData.value(path, QJsonObject());
//    // 只获取目录（忽略文件）
//    QFileInfoList entries = dir.entryInfoList(QDir::Dirs | QDir::NoDotAndDotDot,
//                                              QDir::DirsFirst | QDir::Name);

//    for(const QFileInfo & entry : entries)
//    {
//        QString entryName = entry.fileName();
//        QString entryPath = entry.filePath();
//        if(entryPath.isEmpty()) continue;
//        if(path == entryPath)
//        {
//            qWarning() << entry << entryPath;
//            continue;
//        }

//        QTreeWidgetItem* item = new QTreeWidgetItem(parentItem);

//        item->setText(0, entryName);
//        item->setData(0, Qt::UserRole, entryPath);// 存储完整路径
//        item->setData(0, Qt::UserRole + 2, entryName);// 存储文件名

//        // 检查是否为项目文件夹（包含meta.ctk文件）
//        QString metaPath = entryPath + "/meta.ctk";
//        if(QFile::exists(metaPath))
//        {
//            // 作为项目文件（叶子节点）处理
//            item->setIcon(0, m_fileIcon);
//            item->setData(0, Qt::UserRole + 1, "PROJECT_FOLDER");
//            // 从meta.ctk读取自定义图标
//            QIcon customIcon = loadIconFromMetaCtk(entryPath);
//            if(!customIcon.isNull())
//            {
//                item->setIcon(0, customIcon);
//            }
//        }
//        else
//        {

//            // 目录处理
//            item->setIcon(0, m_folderIcon);
//            item->setData(0, Qt::UserRole + 1, "FOLDER");
//            // 关键：递归加载子目录
//            loadDir(entryPath, item);  // 递归调用
//        }
//    }
//}

// 改为懒加载
void TreeWidget::loadDir(const QString &path, QTreeWidgetItem *parentItem, int parentNodeId)
{
    if(m_isLoadDir) return;
    m_isLoadDir = true;

    if(!m_projectManager->isRepositoryItem(path))
    {
        qWarning() << "It is not the repository directory: " << path;
        return;
    }
    QDir dir(path);
    if(!dir.exists()) return;

    // 只获取目录（忽略文件）
    QFileInfoList entries = dir.entryInfoList(QDir::Dirs | QDir::NoDotAndDotDot,
                                              QDir::DirsFirst | QDir::Name);

    for(const QFileInfo & entry : entries)
    {
        QString entryName = entry.fileName();
        QString entryPath = entry.filePath();

        if(entryPath.isEmpty()) continue;
        if(path == entryPath)
        {
            qWarning() << entry << entryPath;
            continue;
        }

        QTreeWidgetItem* item = new QTreeWidgetItem(parentItem);

        item->setText(0, entryName);
        item->setData(0, Qt::UserRole, entryPath);      // 存储完整路径
        item->setData(0, Qt::UserRole + 2, entryName);  // 存储文件名

        // 检查是否为项目文件夹（包含meta.ctk文件）
        QString metaPath = entryPath + "/meta.ctk";
        if(QFile::exists(metaPath))
        {
            // 作为项目文件（叶子节点）处理
            item->setIcon(0, m_fileIcon);
            item->setData(0, Qt::UserRole + 1, "PROJECT_FOLDER");
            // 从meta.ctk读取自定义图标
            QIcon customIcon = loadIconFromMetaCtk(entryPath);
            if(!customIcon.isNull())
            {
                item->setIcon(0, customIcon);
            }

            // 获取或创建数据库节点
            QVector<Node> nodes = m_projectManager->getDbManager()->nodesByParent(parentNodeId);

            int nodeId = 0;
            bool nodeFound = false;

            for(const Node &node : nodes)
            {
                if(node.name == entryName && node.type == NodeType::Note)
                {
                    nodeId = node.id;
                    nodeFound = true;
                    break;
                }
            }

            if(!nodeFound)
            {
                // 创建新节点
                Node newNode;
                newNode.name = entryName;
                newNode.type = NodeType::Note;
                newNode.parentId = parentNodeId;
                nodeId = m_projectManager->getDbManager()->addNode(newNode);

                // 创建对应的Note
                MetaCtk *metaCtk = m_projectManager->getMetaCtk(metaPath);
                if(metaCtk && metaCtk->load())
                {
                    Note note;
                    note.nodeId = nodeId;
                    note.projectName = entryName;
                    note.imagePath = metaCtk->demoImagePath();
                    note.author = metaCtk->author();
                    note.uuid = metaCtk->id();
                    m_projectManager->getDbManager()->addNote(note);
                }
            }

            item->setData(0, Qt::UserRole + 3, nodeId); // 存储节点ID
        }
        else
        {

            // 目录处理
            item->setIcon(0, m_folderIcon);
            item->setData(0, Qt::UserRole + 1, "FOLDER");

            // 获取数据库节点 - 结合父节点ID和节点名查找
            QVector<Node> nodes = m_projectManager->getDbManager()->nodesByParent(parentNodeId);
            int nodeId = 0;
            bool nodeFound = false;

            for(const Node &node : nodes)
            {
                if(node.name == entryName && node.type == NodeType::Catalog)
                {
                    nodeId = node.id;
                    nodeFound = true;
                    break;
                }
            }

            if(!nodeFound)
            {
                // 创建新节点
                Node newNode;
                newNode.name = entryName;
                newNode.type = NodeType::Catalog;
                newNode.parentId = parentNodeId;
                nodeId = m_projectManager->getDbManager()->addNode(newNode);
            }

            item->setData(0, Qt::UserRole + 3, nodeId); // 存储节点ID
        }
    }
    m_isLoadDir = false;
}

// 从meta.ctk文件加载图标
QIcon TreeWidget::loadIconFromMetaCtk(const QString &projectPath)
{
    QString metaPath = projectPath + "/meta.ctk";
    MetaCtk *config = ProjectManager::getProjectManager()->getMetaCtk(metaPath);
    if(!config || !config->load()) return QIcon();

    QString iconPath = config->demoImagePath();

    if(iconPath.isEmpty())
    {
        CodeNote note = config->noteContent();
        for(const NoteItem &item : note.note)
        {
            if(item.type == NType::Image && !item.content.isEmpty())
            {
                iconPath = item.content;
                break;
            }
        }
    }
    if(iconPath.isEmpty()) return QIcon(); // 返回空图标

    // 标准化路径
    iconPath = QDir::cleanPath(iconPath);
    if(!QFile::exists(iconPath))
    {
        qWarning() << "The icon does not exist: " << iconPath;
        return QIcon();
    }

    // 创建并返回图标
    QPixmap pixmap(iconPath);
    if(pixmap.isNull())
    {
        qWarning() << "Failed to load icon: " << iconPath;
        return QIcon();
    }

    // 缩放图标到合适大小
    QSize iconSize = this->iconSize();
    if(iconSize.isValid() && !iconSize.isEmpty())
    {
        pixmap = pixmap.scaled(iconSize, Qt::KeepAspectRatio, Qt::SmoothTransformation);
    }

    return QIcon(iconPath);
}

// 保存当前展开状态
void TreeWidget::saveExpandedState()
{
    m_expandedItems.clear();

    // 递归保存所有展开的项
    std::function<void(QTreeWidgetItem*)> saveExpanded;
    saveExpanded = [&](QTreeWidgetItem* item){
        if(!item) return;

        QString path = item->data(0, Qt::UserRole).toString();
        if(!path.isEmpty() && item->isExpanded())
            m_expandedItems.insert(path);

        for(int i = 0; i < item->childCount(); ++i)
            saveExpanded(item->child(i));
    };

    // 从顶层项开始
    for(int i = 0; i < topLevelItemCount(); ++i)
        saveExpanded(topLevelItem(i));
}

// 恢复展开状态
void TreeWidget::restoreExpandedState()
{
    if(m_expandedItems.isEmpty()) return;

    // 递归恢复展开状态
    std::function<void(QTreeWidgetItem*)> restoreExpanded;
    restoreExpanded = [&](QTreeWidgetItem* item){
        if(!item) return;

        QString path = item->data(0, Qt::UserRole).toString();
        if(!path.isEmpty() && m_expandedItems.contains(path))
        {
            // 如果该项已展开但尚未加载，则加载其子项
            if(!m_loadedPaths.contains(path))
            {
                int nodeId = item->data(0, Qt::UserRole + 3).toInt();
                loadDir(path, item, nodeId);
                m_loadedPaths.insert(path);
            }

            item->setExpanded(true);
        }

        for(int i = 0; i < item->childCount(); ++i)
            restoreExpanded(item->child(i));
    };

    // 从顶层项开始
    for(int i = 0; i < topLevelItemCount(); ++i)
        restoreExpanded(topLevelItem(i));
}

void TreeWidget::setIcons()
{
    m_fileIcon = QIcon(StyleManager::getStyleManager()->currentTheme() == Theme::LightTheme ?
                           ":/res/icon/codenote.svg" : ":/res/icon/codenote-light.svg");
    m_folderIcon = QIcon(StyleManager::getStyleManager()->currentTheme() == Theme::LightTheme ?
                           ":/res/icon/category.svg" : ":/res/icon/category-light.svg");
    setupTreeView(m_rootPath);
}
