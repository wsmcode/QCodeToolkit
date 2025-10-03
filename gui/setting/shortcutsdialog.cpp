#include "shortcutsdialog.h"

#include <QDebug>
#include <QHeaderView>
#include <QLabel>
#include <QScrollArea>
#include <QTableWidget>
#include <QVBoxLayout>
#include <stylemanager.h>

ShortcutsDialog::ShortcutsDialog(QWidget *parent) : Dialog(parent)
{
    initUI();
}

void ShortcutsDialog::initUI()
{
    QVBoxLayout *mainVLayout = new QVBoxLayout(m_centralWidget);
    mainVLayout->setSpacing(0);
    mainVLayout->setContentsMargins(1, 30, 1, 20);

    // 创建滚动区域
    QScrollArea *scrollArea = new QScrollArea;
    scrollArea->setWidgetResizable(true);
    scrollArea->setFrameShape(QFrame::NoFrame);

    QWidget *contentWidget = new QWidget;
    contentWidget->setObjectName("SettingScrollWidget");
    StyleManager::getStyleManager()->registerWidget(contentWidget);
    QVBoxLayout *contentVLayout = new QVBoxLayout(contentWidget);
    contentVLayout->setSpacing(5);
    contentVLayout->setContentsMargins(30, 10, 30, 1);

    // 添加内置快捷键数据 - 分类显示
    QList<QPair<QString, QList<QPair<QString, QString> > > > categorizedShortcuts = {
        {"文件操作", {
            {"新建文件", "Ctrl+N"},
            {"打开文件", "Ctrl+O"},
            {"保存文件", "Ctrl+S"},
            {"另存为", "Ctrl+Shift+S"},
            {"关闭文件", "Ctrl+W"},
            {"全部关闭", "Ctrl+Shift+W"}
        }},
        {"编辑操作", {
            {"撤销", "Ctrl+Z"},
            {"重做", "Ctrl+Y"},
            {"剪切", "Ctrl+X"},
            {"复制", "Ctrl+C"},
            {"粘贴", "Ctrl+V"},
            {"全选", "Ctrl+A"},
            {"查找", "Ctrl+F"}, // 需要做一个视图
            {"替换", "Ctrl+H"}
        }},
        {"视图操作", {
            {"放大", "Ctrl++"},
            {"缩小", "Ctrl+-"},
            {"重置缩放", "Ctrl+0"},
            {"切换全屏", "F11"}
        }},
        {"代码操作", {
            {"注释/取消注释", "Ctrl+/"}
        }},
        {"导航操作", {
            {"切换标签页", "Ctrl+Tab"},
            {"下一个标签页", "Ctrl+PageDown"},
            {"上一个标签页", "Ctrl+PageUp"},
            {"聚焦编辑器", "Ctrl+1"}
        }}
    };

    // 创建表格显示快捷键

    // 按分类创建表格
    for(auto category = categorizedShortcuts.begin(); category != categorizedShortcuts.end(); category++)
    {
        QLabel *categoryLabel = new QLabel(category->first);
        categoryLabel->setObjectName("SettingSubtitleLabel");
        contentVLayout->addWidget(categoryLabel);

        // 创建该分类的表格
        QTableWidget *shortcutsTable = new QTableWidget;
        shortcutsTable->setColumnCount(2);
        shortcutsTable->horizontalHeader()->setVisible(false);
        shortcutsTable->verticalHeader()->setVisible(false);
        shortcutsTable->setSelectionBehavior(QAbstractItemView::SelectRows);
        shortcutsTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
        shortcutsTable->setFocusPolicy(Qt::NoFocus);
        shortcutsTable->setShowGrid(false);

        // 设置表格属性
        shortcutsTable->horizontalHeader()->setStretchLastSection(true);

        // 添加数据
        const auto& shortcuts = category->second;
        shortcutsTable->setRowCount(shortcuts.size());

        int row = 0;
        for(auto shortcut = shortcuts.begin(); shortcut != shortcuts.end(); shortcut++)
        {
            shortcutsTable->setItem(row, 0, new QTableWidgetItem(shortcut->first));
            shortcutsTable->setItem(row, 1, new QTableWidgetItem(shortcut->second));
            row++;
        }
        int margin = shortcutsTable->contentsMargins().top() + shortcutsTable->contentsMargins().bottom();
        shortcutsTable->resizeColumnsToContents();
        shortcutsTable->setFixedHeight(shortcutsTable->rowHeight(0) * shortcuts.size() + margin);

        contentVLayout->addWidget(shortcutsTable);
        contentVLayout->addSpacing(10);
    }

    scrollArea->setWidget(contentWidget);
    mainVLayout->addWidget(scrollArea);


    m_centralWidget->setFixedSize(400, 600);

    QWidget *grandParent = parentWidget()->parentWidget()->parentWidget()->parentWidget();
    if(grandParent)
    {
        qDebug() << grandParent;
        int x = grandParent->x() + grandParent->width() / 2 - m_centralWidget->width() / 2;
        int y = grandParent->y() + grandParent->height() / 2 - m_centralWidget->height() / 2;
        qDebug() << x << y;
        move(x, y);
    }
}
