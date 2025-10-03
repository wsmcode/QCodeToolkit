#ifndef TEXTITEMWIDGET_H
#define TEXTITEMWIDGET_H

#include "itemcell.h"

class TextEdit;
class TextItemCell : public ItemCell
{
    Q_OBJECT
public:
    explicit TextItemCell(NoteItem item, QWidget *parent = nullptr);

signals:

private slots:
    void onCustomContextMenu(const QPoint &pos);

private:
    void initUI() override;
    void updateContent() override;

    TextEdit *m_editor;
};

#endif // TEXTITEMWIDGET_H
