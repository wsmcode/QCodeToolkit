#include "menubar.h"
#include "menutoolbutton.h"
#include "searchbox.h"
#include "settingmanager.h"
#include "stylemanager.h"

#include <QGraphicsDropShadowEffect>
#include <QHBoxLayout>
#include <QDebug>

MenuBar::MenuBar(QWidget *parent)
    : QFrame(parent)
{
    setFixedHeight(70);
    setObjectName("MenuBar");
    StyleManager::getStyleManager()->registerWidget(this);
    setFrameShape(QFrame::NoFrame);

    initUI();

    MenuToolButton *tagsButton = addMenu(":/res/icon/tags.svg", "标签一览");
    MenuToolButton *themeButton = addMenu(":/res/icon/theme-light-dark.svg", "主题切换");

    QObject::connect(tagsButton, &MenuToolButton::clicked, [=](){emit openTags();});
    QObject::connect(themeButton, &MenuToolButton::clicked, [=](){
        Theme theme = StyleManager::getStyleManager()->currentTheme();
        QString themeStr = (theme == Theme::LightTheme ? "dark" : "light");
        SettingManager::getSettingManager()->handleSettingChange("appearance", "theme", themeStr);
    });

    qInfo() << "MenuBar initialized successfully";
}

MenuBar::~MenuBar()
{
}

MenuToolButton *MenuBar::addMenu(const QString &imagePath, const QString &toolTip)
{
    MenuToolButton *button =  new MenuToolButton(imagePath, toolTip);
    m_toolBarHLayout->addWidget(button);
    return button;
}

QSize MenuBar::sizeHint() const
{
    return QSize(100, 50);
}

void MenuBar::initUI()
{
    // 阴影效果部件
    QFrame *showWidget = new QFrame;
    showWidget->setObjectName("MenuBarShowWidget");
    StyleManager::getStyleManager()->registerWidget(showWidget);
    QGraphicsDropShadowEffect *shadowEffect = new QGraphicsDropShadowEffect(showWidget);
    shadowEffect->setOffset(0, 0);
    shadowEffect->setColor(QColor(0, 0, 0, 90));// 阴影颜色
    shadowEffect->setBlurRadius(5);// 阴影半径
    showWidget->setGraphicsEffect(shadowEffect); // 这句话必须有

    // 菜单按钮
    QFrame *menuToolBar = new QFrame;
    menuToolBar->setObjectName("MenuToolBar");
    StyleManager::getStyleManager()->registerWidget(menuToolBar);
    m_toolBarHLayout = new QHBoxLayout(menuToolBar);
    m_toolBarHLayout->setSpacing(15);
    m_toolBarHLayout->setContentsMargins(15, 0, 15, 0);

    // 搜索框
    m_searchBox = new SearchBox;
    connect(m_searchBox, &SearchBox::openNote, this, &MenuBar::openNote);

    // 主显示区域布局
    QHBoxLayout *hLayout = new QHBoxLayout(showWidget);
    hLayout->setContentsMargins(5, 5, 5, 5);
    hLayout->addStretch();
    hLayout->addStretch();
    hLayout->addWidget(menuToolBar);
    hLayout->addStretch();
    hLayout->addWidget(m_searchBox);

    // 主布局：留出阴影
    QHBoxLayout *mainHLayout = new QHBoxLayout(this);
    mainHLayout->setContentsMargins(5, 5, 5, 5);
    mainHLayout->addWidget(showWidget);
}
