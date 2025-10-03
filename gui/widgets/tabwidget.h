#ifndef TABWIDGET_H
#define TABWIDGET_H
/*****************************************************
*
* @file     tabwidget.h
* @brief    选项卡小部件类
*           功能
*           1. 管理所有选项卡
*
* @author   无声目
* @date     2025/08/16
* @history
*****************************************************/

#include <QTabWidget>
#include <QMap>

class HomeTab;
class TabWidget : public QTabWidget
{
    Q_OBJECT
public:
    explicit TabWidget(QWidget *parent = nullptr);

    // ======= 添加选项卡 ========
    // 添加demo
    void addDemoLabel();
    // 添加项目编辑页
    void addNoteTab(const QString &path = "");
    void addTagsTab();


signals:
    void openNote(const QString &fullPath);

protected:

private slots:
    void onTabCloseRequested(int index);

private:
    // 添加首页选项卡（有且只有一个）
    void addHomeTab();

    HomeTab* m_homeTab;

    QMap<QString, QWidget*> m_pathToTabMap; // 路径到选项卡的映射

    bool m_isOpenTags = false;
};

#endif // TABWIDGET_H
