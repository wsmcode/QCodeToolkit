#ifndef TREEWIDGET_H
#define TREEWIDGET_H
/*****************************************************
*
* @file     treewidget.h
* @brief    文件树视图类
*
* @description
*           ==== 布局 ====
*           ==== 核心功能 ====
*           ==== 使用说明 ====
*           Qt::UserRole是文件路径
*           Qt::UserRole+1是文件类型
*           Qt::UserRole+2是文件名
*           Qt::UserRole+3是节点ID（sqlite）
*           ==== 注意 ====
*           当前版本，添加分类和项目均会刷新视图（更新建议：使用new/addChild）
*           当前版本已实现懒加载，提高大型目录树的性能
*
* @author   无声目
* @date     2025/08/19
* @history
*****************************************************/

#include <QTreeWidget>

#include <QMap>
#include <QJsonObject>

class ProjectManager;
class TreeWidget : public QTreeWidget
{
    Q_OBJECT
public:
    explicit TreeWidget(QWidget *parent = nullptr);
    // 初始化 / 刷新视图
    void setupTreeView(const QString& url);

protected:
    QSize sizeHint() const override;


signals:
    void projectItemClicked(const QString &);

private slots:
    void onCustomContextMenu(const QPoint& pos);
    void onItemChanged(QTreeWidgetItem *item, int column);
    void onItemExpanded(QTreeWidgetItem *item);
    void refreshItem(QTreeWidgetItem *item);
    void onSetDemoImage();

private:
    // 加载目录结构
    void loadDir(const QString& path, QTreeWidgetItem* parentItem, int parentNodeId = 0);
    // 从meta.ctk文件加载图标
    QIcon loadIconFromMetaCtk(const QString& projectPath);
    // 保存当前展开状态
    void saveExpandedState();
    // 恢复展开状态
    void restoreExpandedState();

    void setIcons();

    // 默认图标
    QIcon m_folderIcon;
    QIcon m_fileIcon;

    QString m_rootPath; // 存储根目录路径

    QSet<QString> m_expandedItems; // 保存展开状态的路径集合
    QSet<QString> m_loadedPaths;   // 记录已加载的路径，避免重复加载

    ProjectManager *m_projectManager;

    bool m_isOnItemChanged = false;
    bool m_isLoadDir = false;
    bool m_isRefreshing = false;
};

#endif // TREEWIDGET_H
