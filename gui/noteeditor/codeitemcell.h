#ifndef CODEITEMCELL_H
#define CODEITEMCELL_H

#include "itemcell.h"

class ComboBox;
class QHBoxLayout;
class CodeEditor;
class CodeItemCell : public ItemCell
{
    Q_OBJECT
public:
    explicit CodeItemCell(NoteItem item, QWidget *parent = nullptr);

    void addToolBar(QWidget *button);
    void deleteToolBar(QWidget *button);

signals:

private slots:
    void updateLanguage(const QString &language);
    void onCustomContextMenu(const QPoint &pos);

private:
    void initUI() override;
    void updateContent() override;

    QHBoxLayout *m_codeToolBar;
    CodeEditor *m_codeEditor = nullptr;
    ComboBox *m_languageBox = nullptr;
    int m_editorIdealHeight = 0;

    bool isUpdateContent = false;
};

#endif // CODEITEMCELL_H
