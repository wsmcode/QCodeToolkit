#include "fixednotetab.h"

#include <QDebug>
#include <QStackedLayout>
#include <QToolButton>
#include <QVBoxLayout>
#include <codeitemcell.h>
#include <imageitemcell.h>

FixedNoteTab::FixedNoteTab(const QString& configPath, QWidget *parent) : NoteTab(configPath, parent)
{
    initUI();
    load();
    qInfo() <<"FixedLayoutCodeEditorTab initialized successfully";
}

void FixedNoteTab::load()
{
    NoteTab::load();
    updateContent();
}

void FixedNoteTab::save()
{
    NoteTab::save();
}

void FixedNoteTab::maximizeImageItem()
{
    if(m_currentMode == ImageMaximized) return;
    switchToMaximizedView(m_imageItemCell);
}

void FixedNoteTab::maximizeCodeItem()
{
    if(m_currentMode == CodeMaximized) return;
    switchToMaximizedView(m_codeItemCell);
}

void FixedNoteTab::restoreNormal()
{
    if(m_currentMode == Normal) return;

    // 保存最大化项的内容
    if(m_maximizedItemCell)
    {
        NoteItem itemCell = m_maximizedItemCell->item();
        if(m_currentMode == ImageMaximized)
        {
            m_imageItemCell->setItem(itemCell);
        }
        else if(m_currentMode == CodeMaximized)
        {
            m_codeItemCell->setItem(itemCell);
        }
    }

    // 切换回正常布局
    m_stackedLayout->setCurrentWidget(m_mainContent);
    m_currentMode = Normal;

    // 清理最大化项目
    if(m_maximizedItemCell)
    {
        m_maximizedVLayout->removeWidget(m_maximizedItemCell);
        delete m_maximizedItemCell;
        m_maximizedItemCell = nullptr;
    }
}

void FixedNoteTab::onContentChanged()
{
    // 更新固定布局的内容到m_codeNote
    m_codeNote.note.clear();

    m_codeNote.note.append(m_imageItemCell->item());
    m_codeNote.note.append(m_codeItemCell->item());

    m_isSaved = false;
    emit savedChanged(false);
}

void FixedNoteTab::initUI()
{
    setupImageItem();
    setupCodeItem();

    m_scrollVLayout->insertWidget(1, m_imageContainer);
    m_scrollVLayout->insertWidget(2, m_codeContainer);

    m_maximizedWidget = new QWidget;
    m_maximizedVLayout = new QVBoxLayout(m_maximizedWidget);
    m_maximizedVLayout->setSpacing(0);
    m_maximizedVLayout->setContentsMargins(0, 0, 0, 0);

    m_stackedLayout->addWidget(m_maximizedWidget);
    m_stackedLayout->setCurrentWidget(m_mainContent);
}

void FixedNoteTab::updateContent()
{
    NoteTab::updateContent();
    // 更新图片项和代码项的内容
    if(m_codeNote.note.size() >= 1 && m_imageItemCell)
    {
        m_imageItemCell->setItem(m_codeNote.note[0]);
    }
    else if(m_imageItemCell)
    {
        // 如果没有图片项数据，创建默认的
        NoteItem imageItem;
        imageItem.type = NType::Image;
        imageItem.content = "";
        imageItem.language = "";
        m_imageItemCell->setItem(imageItem);
    }

    if(m_codeNote.note.size() >= 2 && m_codeItemCell)
    {
        m_codeItemCell->setItem(m_codeNote.note[1]);
    }
    else if(m_codeItemCell)
    {
        // 如果没有代码项数据，创建默认的
        NoteItem codeItem;
        codeItem.type = NType::Code;
        codeItem.content = "";
        codeItem.language = "C++";
        m_codeItemCell->setItem(codeItem);
    }
}

void FixedNoteTab::setupImageItem()
{
    // 图片项容器
    m_imageContainer = new QWidget;
    m_imageVLayout = new QVBoxLayout(m_imageContainer);
    m_imageVLayout->setSpacing(0);
    m_imageVLayout->setContentsMargins(0, 0, 0, 0);

    // 图片项工具栏
    m_imageToolbar = new QHBoxLayout;

    m_maximizeImageButton = createToolButton();
    m_maximizeImageButton->setText("最大化");
    QObject::connect(m_maximizeImageButton, &QToolButton::clicked, this, &FixedNoteTab::maximizeImageItem);

    m_imageToolbar->addStretch();
    m_imageToolbar->addWidget(m_maximizeImageButton);

    // 创建图片项
    NoteItem imageItem;
    imageItem.type = NType::Image;
    imageItem.content = "";
    imageItem.language = "";
    m_imageItemCell = new ImageItemCell(imageItem, this);
    m_imageItemCell->setFixed(true);
    QObject::connect(m_imageItemCell, &ImageItemCell::contentChanged, this, &FixedNoteTab::onContentChanged);

    m_imageVLayout->addLayout(m_imageToolbar);
    m_imageVLayout->addWidget(m_imageItemCell);
}

void FixedNoteTab::setupCodeItem()
{
    // 代码项容器
    m_codeContainer = new QWidget;
    m_codeVLayout = new QVBoxLayout(m_codeContainer);
    m_codeVLayout->setSpacing(0);
    m_codeVLayout->setContentsMargins(0, 0, 0, 0);

    // 代码项工具栏
    m_maximizeCodeButton = createToolButton();

    m_maximizeCodeButton->setText("最大化");
    QObject::connect(m_maximizeCodeButton, &QToolButton::clicked, this, &FixedNoteTab::maximizeCodeItem);

    // 创建代码项
    NoteItem codeItem;
    codeItem.type = NType::Code;
    codeItem.content = "";
    codeItem.language = "";
    m_codeItemCell = new CodeItemCell(codeItem, this);
    m_codeItemCell->setFixed(true);

    qobject_cast<CodeItemCell*>(m_codeItemCell)->addToolBar(m_maximizeCodeButton);
    QObject::connect(m_codeItemCell, &CodeItemCell::contentChanged, this, &FixedNoteTab::onContentChanged);

    m_codeVLayout->addWidget(m_codeItemCell);
}

void FixedNoteTab::switchToMaximizedView(ItemCell *itemCell)
{
    if(!itemCell) return;

    if(itemCell == m_imageItemCell) m_currentMode = ImageMaximized;
    else if(itemCell == m_codeItemCell) m_currentMode = CodeMaximized;
    else return;

    // 清理布局
    QLayoutItem *child;
    while((child = m_maximizedVLayout->takeAt(0)) != nullptr)
    {
        if(child->widget()) child->widget()->setParent(nullptr);
        delete child;
    }



    // 恢复按钮
    m_restoreButton = createToolButton();
    m_restoreButton->setText("恢复布局");
    m_restoreButton->setToolTip("恢复到正常布局");
    QObject::connect(m_restoreButton, &QToolButton::clicked, this, &FixedNoteTab::restoreNormal);

    // 创建新的最大化项目（复制内容）
    NoteItem itemData = itemCell->item();
    if(m_currentMode == ImageMaximized) m_maximizedItemCell = new ImageItemCell(itemData, this);
    else m_maximizedItemCell = new CodeItemCell(itemData, this);

    m_maximizedItemCell->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    QObject::connect(m_maximizedItemCell, &ItemCell::contentChanged, this, &FixedNoteTab::onContentChanged);

    if(itemCell == m_imageItemCell)
    {
        m_maximizedToolbar = new QHBoxLayout;
        m_maximizedToolbar->setContentsMargins(10, 5, 10, 5);
        m_maximizedToolbar->addStretch();
        m_maximizedToolbar->addWidget(m_restoreButton);
        m_maximizedVLayout->addLayout(m_maximizedToolbar);
    }
    else
    {
        qobject_cast<CodeItemCell*>(m_maximizedItemCell)->addToolBar(m_restoreButton);
    }

    m_maximizedVLayout->addWidget(m_maximizedItemCell);

    m_maximizedWidget->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

    // 切换到最大化视图
    m_stackedLayout->setCurrentWidget(m_maximizedWidget);
}
