#ifndef TREEPANEL_H
#define TREEPANEL_H
/*****************************************************
*
* @file     treepanel.h
* @brief    树形结构面板类
*
* @description
*           ==== 布局 ====
*           垂直布局：工具栏、树视图
*           ==== 核心功能 ====
*           ==== 使用说明 ====
*           ==== 注意 ====
*
* @author   无声目
* @date     2025/09/06
* @history
*****************************************************/

#include <QFrame>

class TreeWidget;
class QHBoxLayout;
class TreePanel : public QFrame
{
    Q_OBJECT
public:
    explicit TreePanel(QWidget *parent = nullptr);
    // 初始化 / 刷新视图
    void setupTreeView(const QString& url);

signals:
    void projectItemClicked(const QString &);

private:
    void initUI();
    QHBoxLayout *createToolHLayout();

    TreeWidget *m_treeWidget;

};

#endif // TREEPANEL_H
