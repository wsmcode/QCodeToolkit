#include "markdownitemcell.h"
#include "stylemanager.h"

#include <QDebug>
#include <QHBoxLayout>
#include <QMenu>
#include <qmarkdowntextedit.h>

MarkdownItemCell::MarkdownItemCell(NoteItem item, QWidget *parent)
    : ItemCell(item, parent)
{
    initUI();

    setContextMenuPolicy(Qt::CustomContextMenu);
    QObject::connect(this, &MarkdownItemCell::customContextMenuRequested,
                     this, &MarkdownItemCell::onCustomContextMenu);
}

void MarkdownItemCell::initUI()
{
    // 主布局
    QHBoxLayout *mainHLayout = new QHBoxLayout(this);
    mainHLayout->setContentsMargins(0, 0, 0, 0);
    mainHLayout->setSpacing(5);

    // 内容区域
    QWidget *contentWidget = new QWidget;
    contentWidget->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
    QVBoxLayout *contentVLayout = new QVBoxLayout(contentWidget);
    contentVLayout->setContentsMargins(0, 0, 0, 0);
    m_markdownEdit = new QMarkdownTextEdit;
    m_markdownEdit->setContextMenuPolicy(Qt::NoContextMenu);
    m_markdownEdit->setFixedHeight(500);
    m_markdownEdit->setPlaceholderText("开始编写 Markdown 文档...\n\n"
                                       "支持的功能：\n"
                                       "• # 标题语法\n"
                                       "• **粗体** 和 *斜体* 文本\n"
                                       "• 列表和表格\n"
                                       "• 代码块高亮\n"
                                       "• 使用 Ctrl+F 进行搜索\n"
                                       "• 使用 Tab 缩进文本块"
                                       "• Shift + Tab - 取消缩进\n"
                                       "• Ctrl + Alt + ↓ - 复制当前行\n"
                                       "• Ctrl + Click - 打开链接");
    QObject::connect(m_markdownEdit, &QMarkdownTextEdit::textChanged, [=](){
        m_item.content = m_markdownEdit->toPlainText();
        emit contentChanged(m_item);
    });

    contentVLayout->addWidget(m_markdownEdit);

    mainHLayout->addWidget(contentWidget);

    updateContent();
}

void MarkdownItemCell::updateContent()
{
    m_markdownEdit->setText(m_item.content);
}

void MarkdownItemCell::onCustomContextMenu(const QPoint &pos)
{
    QMenu menu;
    // 手动创建标准编辑功能（中文）
    menu.addAction("撤销\tCtrl+Z", m_markdownEdit, &QMarkdownTextEdit::undo, QKeySequence::Undo);
    menu.addAction("重做\tCtrl+Y", m_markdownEdit, &QMarkdownTextEdit::redo, QKeySequence::Redo);
    menu.addSeparator();
    menu.addAction("剪切\tCtrl+X", m_markdownEdit, &QMarkdownTextEdit::cut, QKeySequence::Cut);
    menu.addAction("复制\tCtrl+C", m_markdownEdit, &QMarkdownTextEdit::copy, QKeySequence::Copy);
    menu.addAction("粘贴\tCtrl+V", m_markdownEdit, &QMarkdownTextEdit::paste, QKeySequence::Paste);
    menu.addAction("删除\tDelete", m_markdownEdit,
                   [=](){m_markdownEdit->textCursor().removeSelectedText();}, QKeySequence::Delete);
    menu.addSeparator();
    menu.addAction("全选\tCtrl+A", m_markdownEdit, &QMarkdownTextEdit::selectAll, QKeySequence::SelectAll);

    menu.addSeparator();
    menu.addAction("移除项", [=](){emit removeRequested();});

    menu.exec(mapToGlobal(pos));
}
