#ifndef POPOVERWIDGET_H
#define POPOVERWIDGET_H

/*****************************************************
*
* @file     popoverwidget.h
* @brief    PopoverWidget类
*
* @description
*           ==== 布局 ====
*           1. 垂直列表布局（使用内置QListWidget）
*           2. 自定义内容布局（传入任意QWidget作为内容）
*           ==== 核心功能 ====
*           1. 可相对于指定控件弹出悬浮窗口
*           2. 支持多种弹出方向和位置
*           3. 自动处理焦点丢失和边界检测
*           4. 提供添加和管理内容项的相关API（仅listWidget）
*           ==== 使用说明 ====
*           1. 激活控件时调用显示相关接口
*           2. 默认情况下，点击弹窗外部区域及"需响应控件"外部区域将自动隐藏
*           ==== 注意 ====
*           确保设置正确的父控件或响应控件以获得正确的行为
*
* @author   无声目
* @date     2025/09/18
* @history
*****************************************************/
#include <QFrame>

// 悬浮层显示位置
enum class PopoverPlacement {
    External,  // 外部
    Internal   // 内部
};
using PP = PopoverPlacement;

// 悬浮层显示方向
enum class PopoverDirection {
    North,  // 上方
    South,  // 下方
    West,   // 左侧
    East    // 右侧
};
using PD = PopoverDirection;

class QVBoxLayout;
class QListWidget;
class QListWidgetItem;
class PopoverWidget : public QFrame
{
    Q_OBJECT
public:
    explicit PopoverWidget(QWidget *parent = nullptr);

    // 添加项（使用自定义控件）
    void addItem(QWidget *itemWidget);
    // 添加文本项
    void addItem(const QString &text);
    void addItems(const QStringList &texts);
    // 清空并设置文本项
    void setItems(const QStringList &texts);

    // 在指定位置插入项
    void insertItem(int index, QWidget *itemWidget);
    // 在指定位置插入文本项
    void insertItem(int index, const QString &text);

    QListWidgetItem *item(int row);
    QWidget *itemWidget(QListWidgetItem *item);
    QString itemText(int row);

    // 清空所有项
    void clearItems();

    // 显示在指定控件相对位置
    void showRelativeToWidget(QWidget *widget = nullptr, PD direction = PD::South, PP placement = PP::External);
    void showSouth(QWidget *widget = nullptr, PP placement = PP::External); // 显示在指定控件下方
    void showNorth(QWidget *widget = nullptr, PP placement = PP::External); // 显示在指定控件上方
    void showWest(QWidget *widget = nullptr, PP placement = PP::External); // 显示在指定控件左侧
    void showEast(QWidget *widget = nullptr, PP placement = PP::External); // 显示在指定控件右侧

    // 设置内容控件（替代列表）
    void setContentWidget(QWidget *widget);

//    // 设置边距
//    void setMargin(int margin);
//    int margin() const;

    // 设置父控件（将默认显示在此控件下方）
    void setParent(QWidget *widget);
    // 设置响应控件（默认父控件为响应控件）
    void setRespondedWidget(QWidget *widget);

signals:
    void itemClicked(int index);
    void aboutToHide();

protected:
    void keyPressEvent(QKeyEvent *event) override;
    void hideEvent(QHideEvent *event) override;
    bool eventFilter(QObject *obj, QEvent *event) override;

private:
    void initUI();
    // 计算并调整位置，确保不超出屏幕
    QPoint calculatePosition(QWidget *widget, PD direction, PP placement);

    QVBoxLayout *mainVLayout = nullptr;
    QWidget *contentWidget = nullptr;
    QListWidget *listWidget = nullptr;
    QWidget *m_respondedWidget = nullptr;
    int m_margin = 2; // 与指定控件的间距

};

#endif // POPOVERWIDGET_H
