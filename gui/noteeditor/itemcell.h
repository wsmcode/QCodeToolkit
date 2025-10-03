#ifndef ITEMCELL_H
#define ITEMCELL_H

#include <QWidget>
#include "code_types.h"

class ItemCell : public QWidget
{
    Q_OBJECT
public:
    explicit ItemCell(NoteItem item, QWidget *parent = nullptr);
    virtual ~ItemCell() = default;

    NoteItem item() const;
    virtual void setItem(const NoteItem &item);

    void setFixed(bool fixed);

signals:
    void contentChanged(const NoteItem &item);
    void removeRequested();
//    void moveUpRequested();
//    void moveDownRequested();

protected:
    virtual void initUI() = 0;
    virtual void updateContent() = 0;

    NoteItem m_item;
    bool m_fixed;
};

#endif // ITEMCELL_H
