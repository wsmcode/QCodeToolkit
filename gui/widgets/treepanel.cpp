#include "combobox.h"
#include "treepanel.h"
#include "treewidget.h"

#include <QComboBox>
#include <QVBoxLayout>
#include <QDebug>

TreePanel::TreePanel(QWidget *parent) : QFrame(parent)
{
    initUI();
    QObject::connect(m_treeWidget, &TreeWidget::projectItemClicked, [=](const QString& path){
        emit projectItemClicked(path);
    });

    qInfo() << "TreePanel initialized successfully";
}

void TreePanel::setupTreeView(const QString &url)
{
    m_treeWidget->setupTreeView(url);
}

void TreePanel::initUI()
{
    // 垂直布局：工具区、树视图
    QHBoxLayout *toolHLayout = createToolHLayout();
    m_treeWidget = new TreeWidget;

    QVBoxLayout *vLayout = new QVBoxLayout(this);
    vLayout->setSpacing(0);
    vLayout->setContentsMargins(0, 0, 0, 0);
    vLayout->addLayout(toolHLayout);
    vLayout->addWidget(m_treeWidget);
    vLayout->addSpacing(15);
}

QHBoxLayout *TreePanel::createToolHLayout()
{
    // 下拉框、工具按钮
    ComboBox * comboBox = new ComboBox;
    comboBox->setFixedHeight(24);
    comboBox->addItem("主分类目录");
    comboBox->addItem("自定义目录1");

    QHBoxLayout *hLayout = new QHBoxLayout;
    hLayout->setSpacing(0);
    hLayout->setContentsMargins(0, 0, 0, 0);
    hLayout->addWidget(comboBox);

    return hLayout;
}
