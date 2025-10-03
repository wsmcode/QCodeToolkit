#include "noteeditor.h"

#include "codeitemcell.h"
#include "imageitemcell.h"
#include "markdownitemcell.h"
#include "textitemcell.h"

#include <QVBoxLayout>
#include <QToolButton>

NoteEditor::NoteEditor(QWidget *parent)
    : QWidget(parent)
{
    initUI();
}

void NoteEditor::setNoteItems(const QList<NoteItem> &items)
{
    clearItems();
    m_NoteItems = items;
    updateContent();
}

QList<NoteItem> NoteEditor::noteItems() const
{
    QList<NoteItem> items;
    for(auto widget : m_itemCells)
    {
        items.append(widget->item());
    }
    return items;
}

void NoteEditor::addTextItem()
{
    NoteItem item;
    item.type = NType::Text;
    item.content = "";
    item.language = "";

    TextItemCell *widget = new TextItemCell(item, this);
    QObject::connect(widget, &TextItemCell::removeRequested, [=](){removeItem(widget);});
    QObject::connect(widget, &TextItemCell::contentChanged,
                     this, &NoteEditor::itemContentChanged);

    m_mainVLayout->addWidget(widget);
    m_itemCells.append(widget);

    emit noteItemsChanged(noteItems());
}

void NoteEditor::addImageItem()
{
    NoteItem item;
    item.type = NType::Image;
    item.content = "";
    item.language = "";

    ImageItemCell *widget = new ImageItemCell(item, this);
    QObject::connect(widget, &ImageItemCell::removeRequested, [=](){removeItem(widget);});
    QObject::connect(widget, &ImageItemCell::contentChanged,
                     this, &NoteEditor::itemContentChanged);

    m_mainVLayout->addWidget(widget);
    m_itemCells.append(widget);

    emit noteItemsChanged(noteItems());
}

void NoteEditor::addMarkdownItem()
{
    NoteItem item;
    item.type = NType::Markdown;
    item.content = "";
    item.language = "";

    MarkdownItemCell *widget = new MarkdownItemCell(item, this);
    QObject::connect(widget, &MarkdownItemCell::removeRequested, [=](){removeItem(widget);});
    QObject::connect(widget, &MarkdownItemCell::contentChanged,
                     this, &NoteEditor::itemContentChanged);

    m_mainVLayout->addWidget(widget);
    m_itemCells.append(widget);

    emit noteItemsChanged(noteItems());
}

void NoteEditor::addCodeItem()
{
    NoteItem item;
    item.type = NType::Code;
    item.content = "";
    item.language = "C++";

    CodeItemCell *widget = new CodeItemCell(item, this);
    QObject::connect(widget, &CodeItemCell::removeRequested, [=](){removeItem(widget);});
    QObject::connect(widget, &CodeItemCell::contentChanged,
                     this, &NoteEditor::itemContentChanged);

    m_mainVLayout->addWidget(widget);
    m_itemCells.append(widget);

    emit noteItemsChanged(noteItems());
}

void NoteEditor::removeItem(ItemCell *itemCell)
{
    int index = findItemIndex(itemCell);
    if(index != -1)
    {
        m_mainVLayout->removeWidget(itemCell);
        m_itemCells.removeAt(index);
        delete itemCell;

        emit noteItemsChanged(noteItems());
    }
}

void NoteEditor::itemContentChanged()
{
    emit noteItemsChanged(noteItems());
}

void NoteEditor::initUI()
{
    m_mainVLayout = new QVBoxLayout(this);
    m_mainVLayout->setContentsMargins(0, 0, 0, 0);
    m_mainVLayout->setSpacing(10);
}

void NoteEditor::updateContent()
{
    for(const auto &item : m_NoteItems)
    {
        ItemCell *widget = nullptr;

        switch(item.type)
        {
        case NType::Text:
            widget = new TextItemCell(item, this);
            break;
        case NType::Image:
            widget = new ImageItemCell(item, this);
            break;
        case NType::Markdown:
            widget = new MarkdownItemCell(item, this);
            break;
        case NType::Code:
            widget = new CodeItemCell(item, this);
            break;
        }

        if(widget)
        {
            QObject::connect(widget, &ItemCell::removeRequested, [=](){removeItem(widget);});
            QObject::connect(widget, &ItemCell::contentChanged,
                             this, &NoteEditor::itemContentChanged);
            m_mainVLayout->insertWidget(m_mainVLayout->count() - 1, widget);
            m_itemCells.append(widget);
        }
    }
}

void NoteEditor::clearItems()
{
    for(auto widget : m_itemCells)
    {
        m_mainVLayout->removeWidget(widget);
        delete widget;
    }
    m_itemCells.clear();
}

int NoteEditor::findItemIndex(ItemCell *itemCell) const
{
    return m_itemCells.indexOf(itemCell);
}
