#ifndef NOTEEDITOR_H
#define NOTEEDITOR_H

/*****************************************************
*
* @file     noteeditor.h
* @brief    NoteEditor类
*
* @description
*           ==== 布局 ====
*           ==== 核心功能 ====
*           ==== 使用说明 ====
*           ==== 注意 ====
*
* @author   无声目
* @date     2025/09/27
* @history
*****************************************************/
#include <QWidget>
#include <code_types.h>

class QVBoxLayout;
class QToolButton;
class ItemCell;
class NoteEditor : public QWidget
{
    Q_OBJECT
public:
    explicit NoteEditor(QWidget *parent = nullptr);

    void setNoteItems(const QList<NoteItem> &items);
    QList<NoteItem> noteItems() const;

signals:
    void noteItemsChanged(const QList<NoteItem> &items);

public slots:
    void addTextItem();
    void addImageItem();
    void addMarkdownItem();
    void addCodeItem();

    void removeItem(ItemCell *itemCell);
//    void moveItemUp(ItemCell *itemCell);
//    void moveItemDown(ItemCell *itemCell);
    void itemContentChanged();

private:
    void initUI();
    void updateContent();
    void clearItems();
    int findItemIndex(ItemCell *itemCell) const;

    QVBoxLayout *m_mainVLayout = nullptr;

    QList<NoteItem> m_NoteItems;
    QList<ItemCell*> m_itemCells;
};

#endif // NOTEEDITOR_H

