#include "textedit.h"
#include "stylemanager.h"

TextEdit::TextEdit(QWidget *parent): QTextEdit(parent)
{
    setObjectName("TextEdit");
    StyleManager::getStyleManager()->registerWidget(this);

    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);

    QObject::connect(this, &TextEdit::textChanged, this, &TextEdit::adjustHeight);
}

void TextEdit::setResizable(bool resizable)
{
    m_isResizable = resizable;
    if(m_isResizable) setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    else setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
}

bool TextEdit::isResizeable()
{
    return m_isResizable;
}

void TextEdit::showEvent(QShowEvent *event)
{
    QTextEdit::showEvent(event);
    if(m_isResizable) adjustHeight();
}

void TextEdit::adjustHeight()
{
    if(!m_isResizable) return;
    QFontMetrics fm(font());
    int margins = contentsMargins().top() + contentsMargins().bottom();
    int documentHeight = document()->size().height();

    // 设置新高度
    int newHeight = qMax(fm.height() + margins, documentHeight + margins);
    setFixedHeight(newHeight);
}
