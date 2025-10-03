#include "tabwidget.h"
#include "hometab.h"
#include "notetab.h"
#include "stylemanager.h"
#include "fontmanager.h"

#include <QTabBar>
#include <QLabel>
#include <QEvent>
#include <QMouseEvent>
#include <QDebug>
#include <QFileInfo>
#include <QMessageBox>
#include <dynamicnotetab.h>
#include <fixednotetab.h>
#include <tagonlynotetab.h>

TabWidget::TabWidget(QWidget *parent) : QTabWidget(parent)
{
    FontManager::getFontManager()->registerWidget(this);

    setTabsClosable(true);

    setMovable(true);
    //setDocumentMode(true); // 启用后#TabWidget QTabBar会设置标签栏右侧区域，但会有其他效果
    setObjectName("TabWidget");
    StyleManager::getStyleManager()->registerWidget(this);

    //设置最小宽度
    setMinimumWidth(100);
    // 只添加一次
    addHomeTab();

    // 连接首页的项目点击信号
    QObject::connect(m_homeTab, &HomeTab::projectItemClicked, [=](const QString &path){
        // 假设项目文件夹包含 meta.ctk 文件
        QString metaPath = path + "/meta.ctk";
        if (QFile::exists(metaPath)) {
            addNoteTab(path);
        }
    });

    QObject::connect(this, &TabWidget::tabCloseRequested, this, &TabWidget::onTabCloseRequested);
    qInfo() << "TabWidget initialized successfully";
}

void TabWidget::addHomeTab()
{
    m_homeTab = new HomeTab;
    int index = addTab(m_homeTab, "首页");
    tabBar()->setTabButton(0, QTabBar::RightSide, nullptr);
    setCurrentIndex(index);
}

void TabWidget::addDemoLabel()
{
    QLabel* demoLabel = new QLabel("<center><h2>代码详情视图</h2><p>选择左侧代码项查看详情</p></center>");
    demoLabel->setAlignment(Qt::AlignCenter);
    int index = addTab(demoLabel, "样式示例");
    setCurrentIndex(index);
}

void TabWidget::addNoteTab(const QString &path) // 传入路径为配置文件路径，上一级为项目路径，项目名为tab名
{
    // 检查路径是否已打开
    if(!path.isEmpty() && m_pathToTabMap.contains(path))
    {
        // 直接切换到已打开的标签
        setCurrentWidget(m_pathToTabMap.value(path));
        return;
    }

    NoteTab* noteTab = nullptr;
    QString tabName;

    if(path.isEmpty()) tabName = "未命名项目";
    else tabName = QFileInfo(path).fileName();

    if(tabName == "DynamicNoteTab") noteTab = new DynamicNoteTab(path);
    else noteTab = new FixedNoteTab(path);

    if(!path.isEmpty()) // 记录打开的路径和对应的选项卡
        m_pathToTabMap[path] = noteTab;

    int index = addTab(noteTab, tabName);
    setCurrentIndex(index);

    QObject::connect(noteTab, &NoteTab::openNote, [=](const QString &fullPath){
        emit openNote(fullPath);
    });
    // 连接保存状态改变信号
    QObject::connect(noteTab, &NoteTab::savedChanged, [=](bool isSaved){
        int noteIndex = indexOf(noteTab);
        if(noteIndex != -1)
        {
            QString currentText = tabText(noteIndex);
            // 移除现有的星号
            if(currentText.endsWith("*"))
            {
                currentText.chop(1);
            }
            // 如果未保存，添加星号
            if(!isSaved)
            {
                currentText += "*";
            }
            setTabText(noteIndex, currentText);
        }
    });
}

void TabWidget::addTagsTab()
{
    if(m_isOpenTags)
    {
        // 如果已经打开，直接切换到该标签页
        TagOnlyNoteTab *tab = TagOnlyNoteTab::getTagOnlyNoteTab();
        setCurrentWidget(tab);
        return;
    }

    TagOnlyNoteTab *tab = TagOnlyNoteTab::getTagOnlyNoteTab();
    if(indexOf(tab) != -1) return;

    int index = addTab(tab, "全局标签一览");
    setCurrentIndex(index);

    m_isOpenTags = true;
}

void TabWidget::onTabCloseRequested(int index)
{
    // 获取要关闭的标签页的路径
    QWidget* widget = this->widget(index);

    // 检查是否是 TagOnlyNoteTab
    TagOnlyNoteTab *tagTab = qobject_cast<TagOnlyNoteTab*>(widget);
    if(tagTab)
    {
        // 对于 TagOnlyNoteTab，只移除标签页，不删除 widget
        this->removeTab(index);
        m_isOpenTags = false;
        return;
    }

    // 检查是否是首页
    if(widget == m_homeTab)
    {
        // 首页不允许关闭，直接返回
        return;
    }

    NoteTab* note = qobject_cast<NoteTab*>(widget);

    // 如果是代码编辑器标签页且有未保存的更改
    if(note && !note->isSaved())
    {
        QMessageBox::StandardButton reply;
        reply = QMessageBox::question(this, tr("保存更改"),
                                     tr("文件 '%1' 有未保存的更改。是否保存更改？")
                                     .arg(tabText(index).remove('*')),
                                     QMessageBox::Save | QMessageBox::Discard | QMessageBox::Cancel);

        if(reply == QMessageBox::Save) note->save();
        else if(reply == QMessageBox::Cancel) return; // 取消关闭操作
        // 如果选择Discard，继续关闭操作
    }

    if(note && !note->configPath().isEmpty())
    {
        // 从映射中移除路径
        m_pathToTabMap.remove(note->configPath());
    }

    // 移除标签页
    this->removeTab(index);
    // 删除widget对象
    delete note;
}
