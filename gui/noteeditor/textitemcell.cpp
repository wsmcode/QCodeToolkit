#include "textedit.h"
#include "stylemanager.h"
#include "textitemcell.h"

#include <QHBoxLayout>
#include <QMenu>
#include <QToolButton>

TextItemCell::TextItemCell(NoteItem item, QWidget *parent)
    : ItemCell(item, parent)
{
    initUI();

    setContextMenuPolicy(Qt::CustomContextMenu);
    QObject::connect(this, &TextItemCell::customContextMenuRequested,
                     this, &TextItemCell::onCustomContextMenu);
}

void TextItemCell::initUI()
{
    QHBoxLayout *mainLayout = new QHBoxLayout(this);
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->setSpacing(5);


    // 创建带表格的文本编辑器
    m_editor = new TextEdit;
    m_editor->setResizable(true);
    // 编辑器
    m_editor->setResizable(true);
    m_editor->setPlaceholderText("纯文本");
    m_editor->setPlainText(m_item.content);
    m_editor->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    QObject::connect(m_editor, &TextEdit::textChanged, [=](){
        m_item.content = m_editor->toPlainText();
        emit contentChanged(m_item);
    });

    mainLayout->addWidget(m_editor);
}

void TextItemCell::updateContent()
{
    m_editor->setPlainText(m_item.content);
}

void TextItemCell::onCustomContextMenu(const QPoint &pos)
{
    QMenu menu;
    // 手动创建标准编辑功能（中文）
    menu.addAction("撤销\tCtrl+Z", m_editor, &TextEdit::undo, QKeySequence::Undo);
    menu.addAction("重做\tCtrl+Y", m_editor, &TextEdit::redo, QKeySequence::Redo);
    menu.addSeparator();
    menu.addAction("剪切\tCtrl+X", m_editor, &TextEdit::cut, QKeySequence::Cut);
    menu.addAction("复制\tCtrl+C", m_editor, &TextEdit::copy, QKeySequence::Copy);
    menu.addAction("粘贴\tCtrl+V", m_editor, &TextEdit::paste, QKeySequence::Paste);
    menu.addAction("删除\tDelete", m_editor,
                   [=](){m_editor->textCursor().removeSelectedText();}, QKeySequence::Delete);
    menu.addSeparator();
    menu.addAction("全选\tCtrl+A", m_editor, &TextEdit::selectAll, QKeySequence::SelectAll);

    menu.addSeparator();
    menu.addAction("移除项", [=](){emit removeRequested();});

    menu.exec(mapToGlobal(pos));
}
