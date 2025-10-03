#ifndef MARKDOWNITEMCELL_H
#define MARKDOWNITEMCELL_H

#include "itemcell.h"

class QToolButton;
class QMarkdownTextEdit;
class MarkdownItemCell : public ItemCell
{
    Q_OBJECT
public:
    explicit MarkdownItemCell(NoteItem item, QWidget *parent = nullptr);

signals:

private slots:
    void onCustomContextMenu(const QPoint &pos);

private:
    void initUI() override;
    void updateContent() override;
    QMarkdownTextEdit *m_markdownEdit;
};

#endif // MARKDOWNITEMCELL_H
