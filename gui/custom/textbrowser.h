#ifndef TEXTBROWSER_H
#define TEXTBROWSER_H

#include <QTextBrowser>

class TextBrowser : public QTextBrowser
{
    Q_OBJECT
public:
    explicit TextBrowser(QWidget *parent = nullptr);

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

#endif // TEXTBROWSER_H
