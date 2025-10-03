#ifndef DYNAMICNOTETAB_H
#define DYNAMICNOTETAB_H

#include "notetab.h"

class QHBoxLayout;
class DynamicNoteTab : public NoteTab
{
    Q_OBJECT
public:
    explicit DynamicNoteTab(const QString& configPath, QWidget *parent = nullptr);

    void load() override;
    void save() override;

signals:

private:
    void initUI();
    void updateContent() override;
    void setupToolBar();

    QHBoxLayout *m_addToolBar;
    QToolButton *m_addTextButton = nullptr;
    QToolButton *m_addImageButton = nullptr;
    QToolButton *m_addMarkdownButton = nullptr;
    QToolButton *m_addCodeButton = nullptr;
    const int fixHeight = 30;

    NoteEditor *m_noteWidget;
};

#endif // DYNAMICNOTETAB_H
