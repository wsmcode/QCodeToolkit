#include "flowlayout.h"
#include "stylemanager.h"
#include "tagspopover.h"

#include <QEventLoop>
#include <QLabel>
#include <QLineEdit>
#include <QToolButton>
#include <QVBoxLayout>

TagsPopover::TagsPopover(QWidget *parent) : PopoverWidget(parent)
{
    initUI();
}

QString TagsPopover::getText(QWidget *parent, QWidget *respondedWidget,
                             const QStringList &existingItems, const QStringList &allItems)
{
    // 创建TagsPopover实例
    TagsPopover *popover = new TagsPopover(parent);
    popover->setFixedWidth(parent->width() / 2);
    popover->setRespondedWidget(respondedWidget);
    popover->setupTagsButtons(allItems, existingItems);

    // 隐藏时删除
    QObject::connect(popover, &TagsPopover::aboutToHide, [=](){popover->deleteLater();});

    // 显示在南侧
    popover->showSouth(respondedWidget);

    // 创建事件循环并等待用户选择
    QEventLoop loop;
    QString result;
    QObject::connect(popover, &TagsPopover::textSelected, [&](const QString &text){
        result = text;
        loop.quit();
    });

    QObject::connect(popover, &TagsPopover::aboutToHide, [&loop, &result]() {
        if(result.isEmpty()) loop.quit();
    });

    loop.exec();
    return result;
}

void TagsPopover::onAddButtonClicked()
{
    QString text = m_newNameEdit->text().trimmed();
    if(text.isEmpty())
    {
        m_errorLabel->setText("输入不能为空");
        m_errorLabel->setVisible(true);
        return;
    }
    if(!isValidInput(text))
    {
        m_errorLabel->setText("名称已存在");
        m_errorLabel->setVisible(true);
        return;
    }
    emit textSelected(text);
    hide();
}

void TagsPopover::initUI()
{
    m_contentWidget = new QWidget;

    m_mainVLayout = new  QVBoxLayout(m_contentWidget);
    m_mainVLayout->setContentsMargins(5, 5, 5, 5);
    m_mainVLayout->setSpacing(10);

    m_flowLayout = new FlowLayout;

    // 错误提示标签
    m_errorLabel = new QLabel;
    m_errorLabel->setObjectName("TagsPopoverErrorLabel");
    StyleManager::getStyleManager()->registerWidget(m_errorLabel);
    m_errorLabel->setVisible(false);

    QHBoxLayout *hLayout = new QHBoxLayout;
    hLayout->setContentsMargins(0, 0, 0, 0);
    hLayout->setSpacing(5);
    m_newNameEdit = new QLineEdit;
    m_newNameEdit->setPlaceholderText("输入新名称");
    m_newNameEdit->setObjectName("TagsPopoverLineEdit");
    StyleManager::getStyleManager()->registerWidget(m_newNameEdit);

    m_addButton = new QToolButton;
    m_addButton->setFixedSize(60, 24);
    m_addButton->setText("添加");
    m_addButton->setToolTip("添加新的标签/标签组");
    m_addButton->setObjectName("TagsPopoverAddToolButton");
    StyleManager::getStyleManager()->registerWidget(m_addButton);

    hLayout->addWidget(m_newNameEdit, 1);
    hLayout->addWidget(m_addButton);

    m_mainVLayout->addLayout(m_flowLayout, 1);
    m_mainVLayout->addLayout(hLayout);
    m_mainVLayout->addWidget(m_errorLabel);


    setContentWidget(m_contentWidget);

    // 连接信号
    QObject::connect(m_addButton, &QToolButton::clicked, this, &TagsPopover::onAddButtonClicked);
    QObject::connect(m_newNameEdit, &QLineEdit::returnPressed, this, &TagsPopover::onAddButtonClicked);

    QObject::connect(m_newNameEdit, &QLineEdit::textChanged, [=](){
        // 输入变化时隐藏错误提示
        m_errorLabel->setVisible(false);
    });
}

void TagsPopover::setupTagsButtons(const QStringList &allItems, const QStringList &existingItems)
{
    m_allItems = allItems;
    m_existingItems = existingItems;

    // 清空现有按钮
    for(QToolButton *button : m_tagsButtons)
    {
        m_flowLayout->removeWidget(button);
        delete button;
    }
    m_tagsButtons.clear();

    // 找出不在当前note中的标签
    QStringList availableItems;
    for(const QString &item : allItems)
    {
        if(!existingItems.contains(item))
        {
            availableItems.append(item);
        }
    }

    // 创建标签按钮
    for(const QString &tag : availableItems)
    {
        QToolButton *tagButton = new QToolButton;
        tagButton->setText(tag);
        tagButton->setFixedHeight(m_fixHeight);
        tagButton->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);
        tagButton->setObjectName("TagsPopoverTagToolButton");
        StyleManager::getStyleManager()->registerWidget(tagButton);

        QObject::connect(tagButton, &QToolButton::clicked, [=](){
            emit textSelected(tag);
            hide();
        });

        m_flowLayout->addWidget(tagButton);
        m_tagsButtons.append(tagButton);
    }
}

bool TagsPopover::isValidInput(const QString &input)
{
    // 输入有效：不是当前note中已有的标签/标签组
    return !m_existingItems.contains(input);
}
