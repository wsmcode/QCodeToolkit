#include "dynamicnotetab.h"

#include <QToolButton>
#include <QVBoxLayout>
#include <noteeditor.h>
#include <QDebug>

DynamicNoteTab::DynamicNoteTab(const QString& configPath, QWidget *parent) : NoteTab(configPath, parent)
{
    initUI();
    load();
    qInfo() << "FreeLayoutCodeEditorTab initialized successfully";
}

void DynamicNoteTab::load()
{
    NoteTab::load();
    updateContent();
}

void DynamicNoteTab::save()
{
    NoteTab::save();
}

void DynamicNoteTab::initUI()
{
    // 笔记区
    m_noteWidget = new NoteEditor;
    setupToolBar();

    m_scrollVLayout->insertWidget(1, m_noteWidget);
    m_scrollVLayout->addStretch(1);

    m_mainVLayout->insertLayout(0, m_addToolBar);

    QObject::connect(m_noteWidget, &NoteEditor::noteItemsChanged, [=](const QList<NoteItem> &items){
        m_codeNote.note = items;
        m_isSaved = false;
        emit savedChanged(false);
    });
}

void DynamicNoteTab::updateContent()
{
    NoteTab::updateContent();
    m_noteWidget->setNoteItems(m_codeNote.note);
}

void DynamicNoteTab::setupToolBar()
{
    // 四个添加按钮：纯文本，图片，markdown，代码
    m_addToolBar = new QHBoxLayout;
    m_addToolBar->setContentsMargins(0, 10, 0, 10);
    // 按钮(五个)
    m_addTextButton = createToolButton();
    m_addTextButton->setText("+纯文本");
    QObject::connect(m_addTextButton, &QToolButton::clicked, m_noteWidget, &NoteEditor::addTextItem);

    m_addImageButton = createToolButton();
    m_addImageButton->setText("+图片");
    QObject::connect(m_addImageButton, &QToolButton::clicked, m_noteWidget, &NoteEditor::addImageItem);

    m_addMarkdownButton = createToolButton();
    m_addMarkdownButton->setText("+markdown");
    QObject::connect(m_addMarkdownButton, &QToolButton::clicked, m_noteWidget, &NoteEditor::addMarkdownItem);

    m_addCodeButton = createToolButton();
    m_addCodeButton->setText("+代码");
    QObject::connect(m_addCodeButton, &QToolButton::clicked, m_noteWidget, &NoteEditor::addCodeItem);

    m_addToolBar->addWidget(m_addTextButton);
    m_addToolBar->addWidget(m_addImageButton);
    m_addToolBar->addWidget(m_addMarkdownButton);
    m_addToolBar->addWidget(m_addCodeButton);
}
