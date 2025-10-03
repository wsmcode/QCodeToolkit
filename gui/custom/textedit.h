#ifndef TEXTEDIT_H
#define TEXTEDIT_H

#include <QTextEdit>

class TextEdit : public QTextEdit
{
    Q_OBJECT
public:
    explicit TextEdit(QWidget *parent = nullptr);

    void setResizable(bool resizable);
    bool isResizeable();

signals:

protected:
    void showEvent(QShowEvent *event) override;

private slots:
    void adjustHeight();

private:
    bool m_isResizable = false;
};

#endif // TEXTEDIT_H
