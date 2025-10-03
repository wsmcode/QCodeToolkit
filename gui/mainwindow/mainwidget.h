#ifndef MAINWIDGET_H
#define MAINWIDGET_H

#include "framelesswindowbase.h"

class LogViewer;
class MenuBar;
class TreePanel;
class TabWidget;
class SQLDatabase;
class MainWidget : public FrameLessWindowBase
{
    Q_OBJECT

public:
    MainWidget(QWidget *parent = nullptr);
    ~MainWidget();

    // 显示日志查看器
    void showLogViewer();

protected:
    void closeEvent(QCloseEvent *event) override;
private:
    // ==== 界面相关函数 ====
    void initUI(); // 初始化界面UI
    MenuBar *createMenuBar(); // 创建菜单栏
    TreePanel *createTreePanel(); // 创建树面板
    TabWidget *createTabWidget(); // 创建选项卡小部件

    SQLDatabase *sqlDB;
    LogViewer *m_logViewer = nullptr;
    MenuBar *m_menuBar = nullptr; // 菜单栏
    TreePanel *m_treePanel = nullptr; // 树视图
    TabWidget* m_contentTabs = nullptr;// 内容区
    QString m_rootPath;
    bool flag = false;
};
#endif // MAINWIDGET_H
