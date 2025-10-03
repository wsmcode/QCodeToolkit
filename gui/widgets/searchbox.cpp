#include "popoverwidget.h"
#include "searchbox.h"
#include "stylemanager.h"

#include "databasemanager.h"
#include <QSettings>
#include <QDir>
#include <QDebug>
#include <QStringListModel>
#include <QAbstractItemView>
#include <QListWidgetItem>
#include <QKeyEvent>
#include <QLabel>

SearchBox::SearchBox(QWidget *parent)
    : QLineEdit(parent)
{
    // 初始化
    m_popover = new PopoverWidget(this);
    m_popover->setFixedWidth(240);

    // 连接信号槽
    QObject::connect(this, &QLineEdit::textChanged, this, &SearchBox::onTextChanged);
    QObject::connect(m_popover, &PopoverWidget::itemClicked, [=](int index){
        QString text = m_popover->itemText(index);
        qDebug() << "index" << index << "text" << text;

        if(this->text().isEmpty()) setText(text);
        else onPopoverActivated(text);
    });

    // 加载搜索历史
    loadSearchHistory();

    setFixedSize(240, 35);
    setPlaceholderText("搜索代码和分类");
    setClearButtonEnabled(true);
    setObjectName("SearchBox");

    // 在 SearchBox 构造函数或主题切换时
    QPalette palette = this->palette();
    palette.setColor(QPalette::PlaceholderText, QColor(160, 160, 160));
    this->setPalette(palette);

    StyleManager::getStyleManager()->registerWidget(this);
}

SearchBox::~SearchBox()
{
    // 保存搜索历史
    saveSearchHistory();
}

void SearchBox::keyPressEvent(QKeyEvent *event)
{
    if(event->key() == Qt::Key_Return || event->key() == Qt::Key_Enter)
    {
        // 回车键处理
        QString searchText = text().trimmed();
        if(!searchText.isEmpty())
        {
            // 执行搜索
            DatabaseManager *dbManager = DatabaseManager::getDatabaseManager();
            QVector<Note> notes = dbManager->searchNotesByName(searchText);

            if(!notes.isEmpty())
            {
                // 取第一个匹配的笔记
                Note note = notes.first();
                QString fullPath = dbManager->getNodeFullPath(note.nodeId);
                qDebug() << fullPath;
                emit openNote(fullPath);

                // 添加到搜索历史
                if(!m_searchHistory.contains(searchText))
                {
                    m_searchHistory.prepend(searchText);
                    // 保持历史记录数量限制
                    if(m_searchHistory.size() > 10)
                        m_searchHistory.removeLast();
                }
                else // 历史记录包含
                {
                    m_searchHistory.removeAll(searchText);
                    m_searchHistory.prepend(searchText);
                }

                // 清空搜索框
                clear();
                m_popover->hide();
            }
        }
    }
    else QLineEdit::keyPressEvent(event);
}

void SearchBox::mousePressEvent(QMouseEvent *event)
{
    // 点击搜索框时显示PopoverWidget
    if(!m_popover->isVisible())
    {
        if(text().isEmpty())
        {
            m_popover->setItems(m_searchHistory);
        }
        m_popover->showSouth();
    }
    QLineEdit::mousePressEvent(event);
}

void SearchBox::onTextChanged(const QString &text)
{
    if(text.isEmpty())
    {
        m_popover->setItems(m_searchHistory);
    }
    else
    {
        // 根据输入内容更新自动完成建议
        QVector<Note> notes = DatabaseManager::getDatabaseManager()->searchNotesByName(text);

        m_nodeIdFromSuggestion.clear(); // 清空缓存
        QStringList suggestions;

        for(const Note &note : notes)
        {
            QString suggestion = formatSuggestion(note.nodeId, note.projectName);
            suggestions.append(suggestion);
            m_nodeIdFromSuggestion[suggestion] = note.nodeId; // 缓存映射
        }

        m_popover->setItems(suggestions);
    }
    if(!m_popover->isVisible()) m_popover->showSouth();
}

void SearchBox::onPopoverActivated(const QString &text)
{
    if(text.isEmpty()) return;

    // 检查是否是历史记录项
    if(this->text().isEmpty())
    {
        // 历史记录项：设置文本到LineEdit，并调整历史记录
        setText(text);

        // 从历史记录中移除并重新添加到最前面
        m_searchHistory.removeAll(text);
        m_searchHistory.prepend(text);
    }
    else
    {
        // 非历史记录项：执行搜索
        if(!m_nodeIdFromSuggestion.contains(text)) return;
        int nodeId = m_nodeIdFromSuggestion.value(text);
        if(nodeId != -1)
        {
            QString fullPath = DatabaseManager::getDatabaseManager()->getNodeFullPath(nodeId);
            qDebug() << fullPath;
            emit openNote(fullPath);

            // 将当前文本加入历史记录
            QString currentText = this->text();
            if(!currentText.isEmpty() && !m_searchHistory.contains(currentText))
            {
                m_searchHistory.prepend(currentText);
                if(m_searchHistory.size() > 10)
                    m_searchHistory.removeLast();
            }

            // 清空搜索框
            clear();
            m_popover->hide();
        }
    }
}

void SearchBox::loadSearchHistory()
{
    QSettings settings("QCodeToolkit");
    m_searchHistory = settings.value("search/history").toStringList();
}

void SearchBox::saveSearchHistory()
{
    QSettings settings("QCodeToolkit");
    settings.setValue("search/history", m_searchHistory);
}

QString SearchBox::formatSuggestion(int nodeId, const QString &projectName) const
{
    // 计算最大允许的项目名称长度
    QFontMetrics metrics(font());
    int maxWidth = width(); // 项目名称不超过整体宽度

    // 格式化字符串
    QString formattedName = projectName;

    // 如果项目名称太长，则截断并添加省略号
    if(metrics.horizontalAdvance(projectName) > maxWidth)
        formattedName = metrics.elidedText(projectName, Qt::ElideRight, maxWidth);

    return QString("%1 %2").arg(nodeId).arg(formattedName);
}
