#ifndef HOMETAB_H
#define HOMETAB_H
/*****************************************************
*
* @file     hometab.h
* @brief    选项卡：首页
*
* @description
*           ==== 布局 ====
*           ==== 核心功能 ====
*           ==== 使用说明 ====
*           ==== 注意 ====
*
* @author   无声目
* @date     2025/08/16
* @history
*****************************************************/

#include <QFrame>

class QListWidget;
class QListWidgetItem;
class HomeTab : public QFrame
{
    Q_OBJECT
public:

    explicit HomeTab(QWidget *parent = nullptr);

signals:
    void projectItemClicked(const QString &path);

protected:

private slots:
    void onProjectItemClicked(QListWidgetItem *item);
    void onProjectListChanged();

private:
    void initUI();
    void loadRecentProjects();
    void addProjectItem(const QString &projectName, const QString &imagePath, int nodeId,
                        const QString &projectPath, const QString &parentName, const QDateTime &modifiedTime);

    QListWidget *m_projectList;
};

#endif // HOMETAB_H
