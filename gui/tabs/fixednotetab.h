#ifndef FIXEDNOTETAB_H
#define FIXEDNOTETAB_H

#include "notetab.h"

class ItemCell;
class QHBoxLayout;
class FixedNoteTab : public NoteTab
{
    Q_OBJECT
public:
    explicit FixedNoteTab(const QString& configPath, QWidget *parent = nullptr);

    void load() override;
    void save() override;

    void maximizeImageItem();
    void maximizeCodeItem();
    void restoreNormal();

signals:

private slots:
    void onContentChanged();

private:
    void initUI();
    void updateContent() override;
    void setupImageItem();
    void setupCodeItem();
    void switchToMaximizedView(ItemCell *itemToMaximize);

    enum Mode {
        Normal,
        ImageMaximized,
        CodeMaximized
    };

    Mode m_currentMode = Normal;

    QWidget *m_imageContainer = nullptr;
    QWidget *m_codeContainer = nullptr;
    QVBoxLayout *m_imageVLayout = nullptr;
    QVBoxLayout *m_codeVLayout = nullptr;

    QHBoxLayout *m_imageToolbar = nullptr;
    QToolButton *m_maximizeImageButton = nullptr;

    QToolButton *m_maximizeCodeButton = nullptr;

    ItemCell *m_imageItemCell = nullptr;
    ItemCell *m_codeItemCell = nullptr;

    // 最大化布局的组件
    QWidget *m_maximizedWidget = nullptr;
    QVBoxLayout *m_maximizedVLayout = nullptr;
    QHBoxLayout *m_maximizedToolbar = nullptr;
    QToolButton *m_restoreButton = nullptr;
    ItemCell *m_maximizedItemCell = nullptr;
};

#endif // FIXEDNOTETAB_H
