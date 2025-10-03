#ifndef MENUBAR_H
#define MENUBAR_H
/*****************************************************
*
* @file     menubar.h
* @brief    菜单栏
*
* @description
*           ==== 布局 ====
*           水平布局：导航、工具、搜索
*           ==== 核心功能 ====
*           ==== 使用说明 ====
*           ==== 注意 ====
*
* @author   无声目
* @date     2025/08/19
* @history
*****************************************************/

#include <QWidget>
#include <QFrame>

class SearchBox;
class QHBoxLayout;
class MenuToolButton;
class DatabaseManager;
class MenuBar : public QFrame
{
    Q_OBJECT
public:
    explicit MenuBar(QWidget *parent = nullptr);
    ~MenuBar();

    // 点击后直接响应而不是出现actions
    MenuToolButton *addMenu(const QString &imagePath, const QString &toolTip = "");

protected:
    QSize sizeHint() const override;

signals:
    void openNote(const QString &fullPath);
    void openTags();

private:
    void initUI();

    SearchBox *m_searchBox;

    QHBoxLayout *m_toolBarHLayout;
};

#endif // MENUBAR_H
