#include "textbrowser.h"
#include "stylemanager.h"


TextBrowser::TextBrowser(QWidget *parent)
    : QTextBrowser(parent)
{
    setObjectName("TextBrowser");
    StyleManager::getStyleManager()->registerWidget(this);

    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);

    QObject::connect(this, &TextBrowser::textChanged, this, &TextBrowser::adjustHeight);
}

void TextBrowser::setResizable(bool resizable)
{
    m_isResizable = resizable;
    if(m_isResizable) setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    else setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
}

bool TextBrowser::isResizeable()
{
    return m_isResizable;
}

void TextBrowser::showEvent(QShowEvent *event)
{
    QTextBrowser::showEvent(event);
    if(m_isResizable) adjustHeight();
}

void TextBrowser::adjustHeight()
{
    if(!m_isResizable) return;
    QFontMetrics fm(font());
    int margins = contentsMargins().top() + contentsMargins().bottom();
    int documentHeight = document()->size().height();

    // 设置新高度
    int newHeight = qMax(fm.height() + margins, documentHeight + margins);
    setFixedHeight(newHeight);
}
