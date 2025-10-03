#ifndef SHORTCUTSDIALOG_H
#define SHORTCUTSDIALOG_H

#include <dialog.h>

class ShortcutsDialog : public Dialog
{
    Q_OBJECT
public:
    explicit ShortcutsDialog(QWidget *parent = nullptr);

signals:

private:
    void initUI();

};

#endif // SHORTCUTSDIALOG_H
