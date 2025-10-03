#include "tagswidget.h"
#include "flowlayout.h"
#include "tagspopover.h"
#include "databasemanager.h"
#include "stylemanager.h"

#include <QToolButton>
#include <QDebug>
#include <QMenu>
#include <QInputDialog>
#include <QMessageBox>

TagsWidget::TagsWidget(QWidget *parent) : QWidget(parent)
{
    initUI();

    QObject::connect(this, &TagsWidget::tagsChanged, [=](){updateTagUsageInfo();});
}

void TagsWidget::setTags(const QMap<QString, QStringList> &tags)
{
    m_tags = tags;
    updateContent();
    updateTagUsageInfo();
}

//void TagsWidget::setNodeId(int id)
//{
//    m_nodeId = id;
//}

QMap<QString, QStringList> TagsWidget::tags() const
{
    return m_tags;
}

void TagsWidget::updateTagUsageInfo()
{
    DatabaseManager * db = DatabaseManager::getDatabaseManager();
    m_tagUsageCount.clear();

    // 获取所有标签组
    QVector<TagGroup> groups = db->allTagGroups();

    for(const TagGroup &group : groups)
    {
        QVector<Tag> tags = db->tagsByGroup(group.id);

        for(const Tag &tag : tags)
        {
            // 获取使用改标签的笔记数量
            QVector<Note> notes = db->notesForTag(tag.id);
            QString key = group.name + "::" + tag.name;
            m_tagUsageCount[key] = notes.size();
        }
    }

    setTagUsageInfo();
}

void TagsWidget::addTagGroup()
{
    QVector<TagGroup> allTagGroups = DatabaseManager::getDatabaseManager()->allTagGroups();
    QStringList allItems;
    for(const auto &tagGroup : allTagGroups)
    {
        allItems.append(tagGroup.name);
    }
    QStringList existingItems = m_tags.keys();

    QString groupName = TagsPopover::getText(this, m_addGroupButton, existingItems, allItems);
    if(groupName.isEmpty()) return;

    // 创建水平布局容器
    QWidget *groupContainer = new QWidget;
    QHBoxLayout *hLayout = new QHBoxLayout(groupContainer);
    hLayout->setContentsMargins(0, 0, 0, 0);

    // 组名按钮（左侧）
    QToolButton *groupButton = createGroupButton(groupName);
    hLayout->addWidget(groupButton, 0, Qt::AlignTop);

    // 创建流式布局容器（右侧）
    QWidget *flowWidget = new QWidget;
    FlowLayout *flowLayout = new FlowLayout(flowWidget);
    flowLayout->setContentsMargins(0, 0, 0, 0);

    // 添加标签按钮
    QToolButton *addTagButton = createAddTagButton(groupName);
    flowLayout->addWidget(addTagButton);

    hLayout->addWidget(flowWidget, 1); // 让流式布局占据剩余空间
    m_mainVLayout->insertWidget(m_mainVLayout->count() - 2, groupContainer);

    // 更新映射
    m_groupLayouts[groupName] = flowLayout;
    m_tags[groupName] = QStringList();
    m_addTagBtn[groupName] = addTagButton;

    updateAddButtonPosition();
    emit tagsChanged(m_tags);
}

void TagsWidget::addTagToGroup(const QString &groupName)
{
    if(!m_groupLayouts.contains(groupName))
    {
        qWarning() << "The groupName does not exist";
        return;
    }
    DatabaseManager *db = DatabaseManager::getDatabaseManager();

    QVector<Tag> allTags = db->tagsByGroup(db->tagGroupByName(groupName).id);
    QStringList allItems;
    for(const auto &tag : allTags)
    {
        allItems.append(tag.name);
    }
    QStringList existingItems = m_tags.value(groupName);

    QString tagName = TagsPopover::getText(this, m_addTagBtn[groupName], existingItems, allItems);

    if(tagName.isEmpty()) return;

    m_tags[groupName].append(tagName);

    // 添加标签UI
    if(m_groupLayouts.contains(groupName))
    {
        qDebug() << "groupName" << groupName << "tag" << tagName;
        QToolButton *tagButton = createTagButton(tagName, groupName);

        // 找到添加按钮之前的最后一个widget
        FlowLayout *flowLayout = m_groupLayouts[groupName];
        flowLayout->removeWidget(m_addTagBtn[groupName]);
        flowLayout->addWidget(tagButton);
        flowLayout->addWidget(m_addTagBtn[groupName]);
    }

    emit tagsChanged(m_tags);
}

void TagsWidget::removeTagGroup(const QString &groupName)
{
    if(!m_tags.contains(groupName))
    {
        qWarning() << "The groupName does not exist: " << groupName;
        return;
    }

    // 移除数据
    m_tags.remove(groupName);

    // 移除UI
    if(m_groupLayouts.contains(groupName))
    {
        FlowLayout *flowLayout = m_groupLayouts[groupName];
        QWidget *groupContainer = flowLayout->parentWidget()->parentWidget(); // 获取最外层的容器

        m_mainVLayout->removeWidget(groupContainer);
        groupContainer->deleteLater();

        m_groupLayouts.remove(groupName);
    }

    updateAddButtonPosition();
    emit tagsChanged(m_tags);
    emit tagGroupRemoved(groupName);
}

void TagsWidget::removeTagFromGroup(const QString &groupName, const QString &tag)
{
    if(!m_tags.contains(groupName))
    {
        qWarning() << "The groupName does not exist:" << groupName;
        return;
    }

    if(!m_tags[groupName].contains(tag))
    {
        qWarning() << "The tag does not exist in group:" << tag << groupName;
        return;
    }

    // 移除数据
    m_tags[groupName].removeAll(tag);

    // 移除UI
    if(m_groupLayouts.contains(groupName))
    {
        FlowLayout *flowLayout = m_groupLayouts[groupName];

        // 查找并移除标签按钮
        for(int i = 0; i < flowLayout->count(); i++)
        {
            QLayoutItem *item = flowLayout->itemAt(i);
            if(item && item->widget())
            {
                QToolButton *tagButton = qobject_cast<QToolButton*>(item->widget());
                if(tagButton && tagButton->property("tagText").toString() == tag)
                {
                    flowLayout->removeWidget(tagButton);
                    tagButton->deleteLater();
                    break;
                }
            }
        }
    }

    emit tagsChanged(m_tags);
    emit tagRemovedFromGroup(groupName, tag);
}

void TagsWidget::initUI()
{
    m_mainVLayout = new QVBoxLayout(this);
    m_mainVLayout->setContentsMargins(15, 5, 15, 5);
    m_mainVLayout->setSpacing(10);

    // 添加标签组按钮
    m_addGroupButton = new QToolButton(this);
    m_addGroupButton->setText("+");
    m_addGroupButton->setToolTip("添加标签组");
    m_addGroupButton->setFixedSize(fixHeight * 2, fixHeight);
    m_addGroupButton->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);

    m_addGroupButton->setObjectName("TagsWidgetAddGroupButton");
    StyleManager::getStyleManager()->registerWidget(m_addGroupButton);

    QObject::connect(m_addGroupButton, &QToolButton::clicked, this, &TagsWidget::addTagGroup);

    m_mainVLayout->addWidget(m_addGroupButton);
    m_mainVLayout->addStretch(1);
}

void TagsWidget::updateContent()
{
    // 清除现有UI
    QLayoutItem* child;
    while((child = m_mainVLayout->takeAt(0)) != nullptr)
    {
        if(child->widget() && child->widget() != m_addGroupButton)
        {
            child->widget()->setParent(nullptr);
            delete child->widget();
        }
        delete child;
    }

    // 重新添加添加组按钮和stretch
    m_mainVLayout->addWidget(m_addGroupButton);
    m_mainVLayout->addStretch(1);

    // 清空组布局映射
    m_groupLayouts.clear();
    m_addTagBtn.clear();

    // 添加标签组
    for(auto it = m_tags.begin(); it != m_tags.end(); it++)
    {
        const QString &groupName = it.key();
        const QStringList &tagList = it.value();

        // 创建标签组
        // 创建水平布局容器
        QWidget *groupContainer = new QWidget;
        QHBoxLayout *hLayout = new QHBoxLayout(groupContainer);
        hLayout->setContentsMargins(0, 0, 0, 0);

        // 组名按钮（不可点击，用于展示）
        QToolButton *groupButton = createGroupButton(groupName);
        hLayout->addWidget(groupButton, 0, Qt::AlignTop);

        // 流式布局容器
        QWidget *flowWidget = new QWidget;
        FlowLayout *flowLayout = new FlowLayout(flowWidget);
        flowLayout->setContentsMargins(0, 0, 0, 0);

        // 添加现有标签
        for(const QString &tag : tagList)
        {
            QToolButton *tagButton = createTagButton(tag, groupName);
            flowLayout->addWidget(tagButton);
        }

        // 添加标签按钮
        QToolButton *addTagButton = createAddTagButton(groupName);
        flowLayout->addWidget(addTagButton);

        hLayout->addWidget(flowWidget, 1); // 让流式布局占据剩余空间
        m_mainVLayout->insertWidget(m_mainVLayout->count() - 2, groupContainer); // 插入到添加按钮之前

        // 更新映射
        m_groupLayouts[groupName] = flowLayout;
        m_addTagBtn[groupName] = addTagButton;
    }
    updateAddButtonPosition();
}

void TagsWidget::updateAddButtonPosition()
{
    // 确保添加按钮始终在底部
    m_mainVLayout->removeWidget(m_addGroupButton);
    m_mainVLayout->addWidget(m_addGroupButton);
}

void TagsWidget::setTagUsageInfo()
{
    for(auto it = m_groupLayouts.begin(); it != m_groupLayouts.end(); it++)
    {
        const QString &groupName = it.key();
        FlowLayout *flowLayout = it.value();

        // 遍历流式布局
        for(int i = 0; i < flowLayout->count(); i++)
        {
            QLayoutItem *item = flowLayout->itemAt(i);
            if(item && item->widget())
            {
                QToolButton *tagButton = qobject_cast<QToolButton*>(item->widget());
                if(tagButton && tagButton->property("isTag").toBool())
                {
                    QString tagName = tagButton->property("tagText").toString();

                    // 获取使用记数
                    int usageCount = 0;
                    QString key = groupName + "::" + tagName;
                    if(m_tagUsageCount.contains(key))
                    {
                        usageCount = m_tagUsageCount[key];
                    }

                    // 更新按钮文本和tooltip
                    QFontMetrics fm(tagButton->font());
                    QString displayText = tagName;
                    if(usageCount >= 0)
                    {
                        displayText = QString("%1 (%2)").arg(tagName).arg(usageCount);
                    }
                    // 计算文本宽度，限制最大宽度
                    int textWidth = fm.horizontalAdvance(displayText) + 20;
                    tagButton->setFixedHeight(fixHeight);
                    tagButton->setMinimumWidth(fixHeight * 2);
                    tagButton->setMaximumWidth(fixHeight * 5);

                    if(textWidth > fixHeight * 4)
                    {
                        QString elidedText = fm.elidedText(displayText, Qt::ElideRight, fixHeight * 4 - 10);
                        tagButton->setText(elidedText);
                        tagButton->setToolTip(QString("%1\n被 %2 个笔记使用\n右键点击删除")
                                              .arg(tagName).arg(usageCount));
                    }
                    else
                    {
                        tagButton->setText(displayText);
                        tagButton->setToolTip(QString("被 %1 个笔记使用\n右键点击删除").arg(usageCount));
                    }

                }
            }
        }
    }
}

QToolButton *TagsWidget::createGroupButton(const QString &groupName)
{
    QToolButton *groupButton = new QToolButton;
    groupButton->setText(groupName);
//    groupButton->setEnabled(false);
    groupButton->setToolTip("右键重命名或删除");
    groupButton->setFixedHeight(fixHeight);
    groupButton->setMinimumWidth(fixHeight * 2);
    groupButton->setMaximumWidth(fixHeight * 4);
    groupButton->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);

    groupButton->setObjectName("TagsWidgetGroupButton");
    StyleManager::getStyleManager()->registerWidget(groupButton);

    // 添加上下文菜单
    groupButton->setContextMenuPolicy(Qt::CustomContextMenu);
    QObject::connect(groupButton, &QToolButton::customContextMenuRequested, [=](const QPoint &pos){
        QMenu menu;
        menu.addAction("删除", [=](){
            QMessageBox::StandardButton reply;
            reply = QMessageBox::question(this, "确认删除",
                                         QString("确定要删除标签组 '%1' 及其所有标签吗?").arg(groupName),
                                         QMessageBox::Yes|QMessageBox::No);
            if(reply == QMessageBox::Yes) removeTagGroup(groupName);
        });
        menu.exec(groupButton->mapToGlobal(pos));
    });
    return groupButton;
}

QToolButton *TagsWidget::createTagButton(const QString &tag, const QString &groupName)
{
    QToolButton *tagButton = new QToolButton;

    QObject::connect(tagButton, &QToolButton::clicked, this, [=]{
        emit tagClicked(tagButton, groupName);
    });

    tagButton->setProperty("tagText", tag);
    tagButton->setProperty("isTag", true);

    // 计算文本宽度，限制最大宽度
    QFontMetrics fm(tagButton->font());
    int textWidth = fm.horizontalAdvance(tag) + 20; // 加上一些padding
    tagButton->setFixedHeight(fixHeight);
    tagButton->setMinimumWidth(fixHeight * 2);
    tagButton->setMaximumWidth(fixHeight * 5);
    tagButton->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);

    // 如果文本太长，显示省略号并在tooltip中显示完整文本
    if(textWidth > fixHeight * 4)
    {
        QString elidedText = fm.elidedText(tag, Qt::ElideRight, fixHeight * 4 - 10);
        tagButton->setText(elidedText);
        tagButton->setToolTip(tag + "\n右键点击删除");
    }
    else
    {
        tagButton->setText(tag);
        tagButton->setToolTip("右键点击删除");
    }
    tagButton->setObjectName("TagsWidgetTagButton");
    StyleManager::getStyleManager()->registerWidget(tagButton);

    // 添加上下文菜单用于删除
    tagButton->setContextMenuPolicy(Qt::CustomContextMenu);
    QObject::connect(tagButton, &QToolButton::customContextMenuRequested, [=](const QPoint &){
        QMenu menu;
        menu.addAction("删除", [=](){
            removeTagFromGroup(groupName, tag);
        });
        menu.exec(QCursor::pos());
    });
    return tagButton;
}

QToolButton *TagsWidget::createAddTagButton(const QString &groupName)
{
    QToolButton *addTagButton = new QToolButton;
    addTagButton->setText("+");
    addTagButton->setFixedSize(fixHeight * 2, fixHeight);
    addTagButton->setToolTip("添加标签");

    addTagButton->setObjectName("TagsWidgetAddTagButton");
    StyleManager::getStyleManager()->registerWidget(addTagButton);

    QObject::connect(addTagButton, &QToolButton::clicked, [=]() {
        addTagToGroup(groupName);
    });
    return addTagButton;
}
