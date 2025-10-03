#include "itemcell.h"

ItemCell::ItemCell(NoteItem item, QWidget *parent)
    : QWidget(parent), m_item(item)
{
}

NoteItem ItemCell::item() const
{
    return m_item;
}

void ItemCell::setItem(const NoteItem &item)
{
    m_item = item;
    updateContent();
}

void ItemCell::setFixed(bool fixed)
{
    m_fixed = fixed;
}
