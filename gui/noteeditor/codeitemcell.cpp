#include "codeitemcell.h"
#include "stylemanager.h"

#include <QDebug>
#include <QFontDatabase>
#include <QLineEdit>
#include <QMenu>
#include <QVBoxLayout>
#include <codeeditor.h>
#include <combobox.h>

CodeItemCell::CodeItemCell(NoteItem item, QWidget *parent)
    : ItemCell(item, parent)
{
    initUI();

    setContextMenuPolicy(Qt::CustomContextMenu);
    QObject::connect(this, &CodeItemCell::customContextMenuRequested,
                     this, &CodeItemCell::onCustomContextMenu);
}

void CodeItemCell::addToolBar(QWidget *button)
{
    m_codeToolBar->addWidget(button);
}

void CodeItemCell::deleteToolBar(QWidget *button)
{
    m_codeToolBar->removeWidget(button);
}

void CodeItemCell::onCustomContextMenu(const QPoint &pos)
{
    QMenu menu;
    // 手动创建标准编辑功能（中文）
    menu.addAction("撤销\tCtrl+Z", m_codeEditor, &CodeEditor::undo, QKeySequence::Undo);
    menu.addAction("重做\tCtrl+Y", m_codeEditor, &CodeEditor::redo, QKeySequence::Redo);
    menu.addSeparator();
    menu.addAction("剪切\tCtrl+X", m_codeEditor, &CodeEditor::cut, QKeySequence::Cut);
    menu.addAction("复制\tCtrl+C", m_codeEditor, &CodeEditor::copy, QKeySequence::Copy);
    menu.addAction("粘贴\tCtrl+V", m_codeEditor, &CodeEditor::paste, QKeySequence::Paste);
    menu.addAction("删除\tDelete", m_codeEditor, &CodeEditor::removeSelectedText, QKeySequence::Delete);
    menu.addSeparator();
    menu.addAction("全选\tCtrl+A", m_codeEditor, &CodeEditor::selectAll, QKeySequence::SelectAll);

    if(!m_fixed)
    {
        menu.addSeparator();
        menu.addAction("移除项", [=](){emit removeRequested();});
    }

    menu.exec(mapToGlobal(pos));
}

void CodeItemCell::initUI()
{
    QHBoxLayout *mainHLayout = new QHBoxLayout(this);
    mainHLayout->setContentsMargins(0, 0, 0, 0);

    QVBoxLayout *contentVLayout = new QVBoxLayout;
    contentVLayout->setContentsMargins(0, 0, 0, 0);

    // 代码编辑器
    m_codeEditor = new CodeEditor;
    StyleManager::getStyleManager()->registerWidget(m_codeEditor);
    m_codeEditor->setLanguage(Language::CPP);
    m_codeEditor->setText(m_item.content);
    m_codeEditor->setMinimumHeight(300);

    QObject::connect(m_codeEditor, &CodeEditor::textChanged, [=](){
        if(isUpdateContent) return;
        m_item.content = m_codeEditor->text();
        emit contentChanged(m_item);
        qDebug() << m_codeEditor->getConfig().backgroundColor.name();
    });

    m_codeToolBar = new QHBoxLayout;
    m_codeToolBar->setContentsMargins(0, 0, 0, 0);

    m_languageBox = new ComboBox;
    m_languageBox->addItems({"C++", "C", "Python", "Java", "JavaScript", "Rust"});
    m_languageBox->setFixedWidth(100);
    QObject::connect(m_languageBox, &ComboBox::currentTextChanged, this, &CodeItemCell::updateLanguage);
    m_codeToolBar->addWidget(m_languageBox);
    m_codeToolBar->addStretch();

    contentVLayout->addLayout(m_codeToolBar);
    contentVLayout->addWidget(m_codeEditor, 1);

    mainHLayout->addLayout(contentVLayout);
}

void CodeItemCell::updateContent()
{
    isUpdateContent = true;
    m_codeEditor->setText(m_item.content);
    isUpdateContent = false;
}

void CodeItemCell::updateLanguage(const QString &language)
{
    m_item.language = language;
    m_codeEditor->setLanguage(language);
    emit contentChanged(m_item);
}
