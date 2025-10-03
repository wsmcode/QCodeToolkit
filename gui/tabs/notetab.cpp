#include "noteeditor.h"
#include "notetab.h"
#include "tagswidget.h"
#include "projectmanager.h"
#include "databasemanager.h"
#include "popoverwidget.h"
#include "stylemanager.h"

#include <QScrollArea>
#include <QScrollBar>
#include <QVBoxLayout>
#include <QResizeEvent>
#include <QDebug>
#include <QAction>
#include <QToolButton>
#include <QStackedLayout>

NoteTab::NoteTab(const QString &configPath, QWidget *parent)
    : QWidget(parent),
      m_configPath(configPath)
{
    m_popover = new PopoverWidget(this);
    m_popover->setFixedWidth(300);
    QObject::connect(m_popover, &PopoverWidget::itemClicked, [=](int index){
        QString text = m_popover->itemText(index);
        qDebug() << "index" << index << "text" << text;
        onPopoverActivated(text);
    });

    if(!m_configPath.isEmpty())
        m_metaCtk = ProjectManager::getProjectManager()->getMetaCtk(m_configPath);

    initNoteTabUI();

    // 创建动作并设置快捷键
    QAction *action = new QAction("save", this);
    action->setShortcut(QKeySequence(Qt::CTRL + Qt::Key_S));
    // 连接动作的triggered信号到槽函数
    QObject::connect(action, &QAction::triggered, this, &NoteTab::save);
    addAction(action);
}

QString NoteTab::configPath() const {return m_configPath;}

bool NoteTab::isSaved() const {return m_isSaved;}

void NoteTab::load()
{
    if(!m_metaCtk->load())
    {
        qWarning() << "metaCtk(configuration file) failed to load" << m_configPath;
        return;
    }
    m_codeNote = m_metaCtk->noteContent();
    m_isSaved = true;
    emit savedChanged(true);
}

void NoteTab::save()
{
    if(m_isSaved) return;
    m_metaCtk->setNoteContent(m_codeNote);
    m_metaCtk->save();

    syncTagsToDatabase();

    m_isSaved = true;
    emit savedChanged(true);
    qInfo() << "saved successfully";
}

void NoteTab::resizeEvent(QResizeEvent *event)
{
    QWidget::resizeEvent(event);

    // 获取可用宽度（父窗口宽度减去滚动条宽度）
    int availableWidth = width() - (m_scrollArea->verticalScrollBar()->isVisible() ?
                                  m_scrollArea->verticalScrollBar()->width() : 0);
    // 计算动态边距 - 使用非线性函数确保在各种窗口大小下都有合适的边距
    int margin = calculateSmartMargin(availableWidth);
    m_scrollVLayout->setContentsMargins(margin, 15, margin, 15);

    m_scrollArea->widget()->setFixedWidth(availableWidth);
}

void NoteTab::updateContent()
{
    m_tagsWidget->setTags(m_codeNote.tags);
}

void NoteTab::onTagClicked(QToolButton *tagButton, const QString &groupName)
{
    DatabaseManager *db = DatabaseManager::getDatabaseManager();

    QString tagName = tagButton->property("tagText").toString();
    Tag tag = db->tagByNameAndGroup(tagName, db->tagGroupByName(groupName).id);
    QVector<Note> notes = db->notesForTag(tag.id);

    m_nodeIdFromSuggestion.clear();
    QStringList suggestions;

    for(const Note &note : notes)
    {
        QString suggestion = QString("%1 %2").arg(note.nodeId).arg(note.projectName); // 显示全名
        suggestions.append(suggestion);
        m_nodeIdFromSuggestion[suggestion] = note.nodeId;
    }
    m_popover->setRespondedWidget(tagButton);
    m_popover->setItems(suggestions);
    m_popover->setFixedHeight(this->height());
    m_popover->showEast(this, PP::Internal);
}

void NoteTab::onPopoverActivated(const QString &text)
{
    // 非历史记录项：执行搜索
    if(!m_nodeIdFromSuggestion.contains(text)) return;
    int nodeId = m_nodeIdFromSuggestion.value(text);
    if(nodeId != -1)
    {
        QString fullPath = DatabaseManager::getDatabaseManager()->getNodeFullPath(nodeId);
        qDebug() << fullPath;
        emit openNote(fullPath);

        m_popover->hide();
    }
}

void NoteTab::onTagRemovedFromGroup(const QString &groupName, const QString &tag)
{
    // 获取数据库记录
    DatabaseManager *db = DatabaseManager::getDatabaseManager();
    Note note = db->noteByUuid(m_metaCtk->id());
    if(note.isEmpty())
    {
        qWarning() << "Note not found in database for UUID:" << m_metaCtk->id();
        return;
    }

    // 获取标签组
    TagGroup group = db->tagGroupByName(groupName);
    if(group.isEmpty())
    {
        qWarning() << "Tag group not found:" << groupName;
        return;
    }

    // 获取标签
    Tag tagObj = db->tagByNameAndGroup(tag, group.id);
    if(tagObj.isEmpty())
    {
        qWarning() << "Tag not found:" << tag << "in group:" << groupName;
        return;
    }

    // 移除笔记与标签的关联
    if(!db->removeNoteTag(note.nodeId, tagObj.id))
    {
        qWarning() << "Failed to remove tag association:" << tag << "from note";
    }
    else
    {
        qDebug() << "Tag" << tag << "remove from note successfully";
    }
}

void NoteTab::onTagGroupRemoved(const QString &groupName)
{
    DatabaseManager *db = DatabaseManager::getDatabaseManager();

    // 获取标签组
    TagGroup group = db->tagGroupByName(groupName);
    if(group.isEmpty())
    {
        qWarning() << "Tag group not found:" << groupName;
        return;
    }

    // 获取组内的所有标签
    QVector<Tag> tagsInGroup = db->tagsByGroup(group.id);
    for(const auto &tag : tagsInGroup)
    {
        onTagRemovedFromGroup(groupName, tag.name);
    }
}

void NoteTab::initNoteTabUI()
{
    // 滚动区域：垂直布局
    m_scrollArea = new QScrollArea;
    m_scrollArea->setWidgetResizable(true);
    m_scrollArea->setFrameShape(QFrame::NoFrame);
    m_scrollArea->setObjectName("NoteScrollArea");
    StyleManager::getStyleManager()->registerWidget(m_scrollArea);
    m_scrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    m_scrollArea->verticalScrollBar()->setSingleStep(30); // 增加这个值会使滚动更快
    m_scrollArea->verticalScrollBar()->setPageStep(100);  // 页面步长，通常用于PageUp/PageDown

    // 滚动区域的内部widget
    QWidget *scrollContent = new QWidget;
    scrollContent->setObjectName("NoteScrollContent");
    StyleManager::getStyleManager()->registerWidget(scrollContent);

    m_scrollVLayout = new QVBoxLayout(scrollContent);
    // 获取可用宽度（父窗口宽度减去滚动条宽度）
    int availableWidth = width() - (m_scrollArea->verticalScrollBar()->isVisible() ?
                                  m_scrollArea->verticalScrollBar()->width() : 0);
    m_scrollVLayout->setSpacing(10);
    int margin = calculateSmartMargin(availableWidth);
    m_scrollVLayout->setContentsMargins(margin, 15, margin, 15);

    // tags区
    m_tagsWidget = new TagsWidget;
    QObject::connect(m_tagsWidget, &TagsWidget::tagClicked, this, &NoteTab::onTagClicked);

    m_scrollVLayout->addWidget(m_tagsWidget);
    m_scrollVLayout->addStretch(1);

    m_scrollArea->setWidget(scrollContent);

    // 主布局
    m_mainContent = new QWidget;
    m_mainVLayout = new QVBoxLayout(m_mainContent);
    m_mainVLayout->setSpacing(0);
    m_mainVLayout->setContentsMargins(0, 0, 0, 0);
    m_mainVLayout->addWidget(m_scrollArea, 1);

    m_stackedLayout = new QStackedLayout(this);
    m_stackedLayout->addWidget(m_mainContent);
    m_stackedLayout->setCurrentWidget(m_mainContent);

    QObject::connect(m_tagsWidget, &TagsWidget::tagsChanged, [=](const QMap<QString, QStringList> &tags){
        m_codeNote.tags = tags;
        m_isSaved = false;
        save();
    });
    QObject::connect(m_tagsWidget, &TagsWidget::tagRemovedFromGroup,
                                             this, &NoteTab::onTagRemovedFromGroup);
    QObject::connect(m_tagsWidget, &TagsWidget::tagGroupRemoved,
                                         this, &NoteTab::onTagGroupRemoved);
}

QToolButton *NoteTab::createToolButton()
{
    QToolButton *toolButton = new QToolButton;
    toolButton->setFixedSize(fixHeight * 3, fixHeight);
    toolButton->setObjectName("NoteToolButton");
    StyleManager::getStyleManager()->registerWidget(toolButton);
    return toolButton;
}

void NoteTab::syncTagsToDatabase()
{
    // 获取数据库记录
    DatabaseManager *db = DatabaseManager::getDatabaseManager();
    Note note = db->noteByUuid(m_metaCtk->id());
    if(note.isEmpty())
    {
        qWarning() << "Note not found in database for UUID:" << m_metaCtk->id();
        return;
    }

    // 获取数据库中，已存储的，当前笔记的标签
    QVector<Tag> noteTags = db->tagsForNote(note.nodeId);
    QMap<int, Tag> noteIdForTags;
    for(const Tag &noteTag : noteTags)
    {
        noteIdForTags[noteTag.id] = noteTag;
    }

    // 遍历当前笔记中所有的标签
    for(auto it = m_codeNote.tags.constBegin(); it != m_codeNote.tags.constEnd(); it++)
    {
        const QString &groupName = it.key();
        const QStringList &tags = it.value();

        // 在数据库中查找标签组，没有就加入
        TagGroup group = db->tagGroupByName(groupName);
        if(group.isEmpty())
        {
            group.name = groupName;
            group.id = db->addTagGroup(group);
            if(group.id == 0)
            {
                qWarning() << "Failed to create tag group: " << groupName;
                continue;
            }
        }

        // 遍历标签组内的标签
        for(const QString tagName : tags)
        {
            // 在数据库中查找标签，没有就加入
            Tag tag = db->tagByNameAndGroup(tagName, group.id);

            if(tag.isEmpty())
            {
                tag.name = tagName;
                tag.groupId = group.id;
                tag.id = db->addTag(tag);
                if(tag.id == 0)
                {
                    qWarning() << "Failed to create tag:" << tagName << "in group:" << groupName;
                    continue;
                }

            }
            // 检查笔记是否关联此标签
            if(!noteIdForTags.contains(tag.id))
            {
                if(!db->addNoteTag(note.nodeId, tag.id))
                    qWarning() << "Failed to link tag" << tagName << "to note";
            }
        }
    }
    qDebug() << "Tags synchronized to database successfully";
}

// 添加智能边距计算方法
int NoteTab::calculateSmartMargin(int availableWidth)
{
    // 定义过渡函数 - 使用S形曲线实现平滑过渡
    double transitionFactor;

    if(availableWidth <= m_baseWidth - m_transitionRange/2)
    {
        // 小窗口：使用固定最小边距
        return m_minMargin;
    }
    else if(availableWidth >= m_baseWidth + m_transitionRange/2)
    {
        // 大窗口：使用固定最大边距
        return m_maxMargin;
    }
    else
    {
        // 过渡区域：使用S形曲线平滑过渡
        double x = static_cast<double>(availableWidth - (m_baseWidth - m_transitionRange / 2))
                / m_transitionRange;
        transitionFactor = smoothStep(x); // 0到1之间的平滑过渡因子

        // 计算边距
        return m_minMargin + transitionFactor * (m_maxMargin - m_minMargin);
    }
}

// 添加缓动函数
double NoteTab::smoothStep(double x)
{
    // 三次多项式S形曲线：3x² - 2x³
    if(x <= 0) return 0;
    if(x >= 1) return 1;
    return x * x * (3 - 2 * x);
}
