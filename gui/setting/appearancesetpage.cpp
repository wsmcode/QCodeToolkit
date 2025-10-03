#include "appearancesetpage.h"

#include <QApplication>
#include <QCheckBox>
#include <QDebug>
#include <QFontDialog>
#include <QLabel>
#include <QPushButton>
#include <QRadioButton>
#include <QScrollArea>
#include <QVBoxLayout>
#include <fontcombobox.h>
#include <spinbox.h>
#include <stylemanager.h>

AppearanceSetPage::AppearanceSetPage(QWidget *parent) : SettingsPage(parent)
{
    initUI();
    initConnections();

    m_zoomComboBox->setEnabled(false);
    m_zoomWheelCheckBox->setEnabled(false);
    m_showWordCountCheckBox->setEnabled(false);
}

QString AppearanceSetPage::title() const
{
    return "外观";
}

void AppearanceSetPage::load(const AppSettings &settings)
{
    // 加载主题设置
    m_themeLightRButton->setChecked(settings.appearance.theme == "light");
    m_themeDarkRButton->setChecked(settings.appearance.theme == "dark");

    // 加载字体设置
    m_fontSizeBox->setValue(settings.appearance.fontSize);
    if(m_fontComboBox->findText(settings.appearance.fontFamily) == -1)
    {
        m_fontComboBox->addItem(settings.appearance.fontFamily);
    }
    m_fontComboBox->setCurrentText(settings.appearance.fontFamily);

    // 加载Zoom设置
    m_zoomComboBox->setCurrentText(QString::number(settings.appearance.zoomLevel) + "%");
    m_zoomWheelCheckBox->setChecked(settings.appearance.zoomWithWheel);

    // 加载显示设置
    m_showStatusCheckBox->setChecked(settings.appearance.showStatusBar);
    m_showWordCountCheckBox->setChecked(settings.appearance.showWordCount);
}

void AppearanceSetPage::save(AppSettings &settings)
{
    // 保存主题设置
    settings.appearance.theme = m_themeLightRButton->isChecked() ? "light" :"dark";

    // 保存字体设置
    settings.appearance.fontSize = m_fontSizeBox->value();
    settings.appearance.fontFamily = m_fontComboBox->currentText();

    // 保存Zoom设置
    QString zoomText = m_zoomComboBox->currentText();
    zoomText.chop(1); // 移除%符号
    settings.appearance.zoomLevel = zoomText.toInt();
    settings.appearance.zoomWithWheel = m_zoomWheelCheckBox->isChecked();

    // 保存显示设置
    settings.appearance.showStatusBar = m_showStatusCheckBox->isChecked();
    settings.appearance.showWordCount = m_showWordCountCheckBox->isChecked();
}

void AppearanceSetPage::initUI()
{
    QWidget *mainWidget = new QWidget;
    setWidget(mainWidget);
    mainWidget->setObjectName("SettingScrollWidget");
    StyleManager::getStyleManager()->registerWidget(mainWidget);

    QVBoxLayout *mainVLayout = new QVBoxLayout(mainWidget);
    mainVLayout->setSpacing(10);
    mainVLayout->setContentsMargins(10, 10, 0, 10);

    // 创建UI控件
    QLabel *titleLabel = new QLabel("外观");
    titleLabel->setObjectName("SettingTitleLabel");

    // 主题设置部分
    QLabel *themeLabel = new QLabel("设置主题");
    themeLabel->setObjectName("SettingSubtitleLabel");
    QHBoxLayout *themeHLayout = new QHBoxLayout;
    themeHLayout->setSpacing(10);
    themeHLayout->setContentsMargins(0, 0, 0, 0);

    m_themeLightRButton = new QRadioButton("浅色");
    m_themeDarkRButton = new QRadioButton("深色");
    themeHLayout->addWidget(m_themeLightRButton);
    themeHLayout->addWidget(m_themeDarkRButton);
    themeHLayout->addStretch();

    // 字体设置部分
    QLabel *fontLabel = new QLabel("设置字体");
    fontLabel->setObjectName("SettingSubtitleLabel");
    QHBoxLayout *fontHLayout = new QHBoxLayout;
    fontHLayout->setSpacing(5);
    fontHLayout->setContentsMargins(0, 0, 0, 0);

    // 字体大小
    QLabel *fontSizeLabel = new QLabel("字体大小");
    m_fontSizeBox = new SpinBox;
    m_fontSizeBox->setRange(9, 24);
    m_fontSizeBox->setFixedWidth(50);
    QLabel *pxLabel = new QLabel("px");

    // 字体下拉框
    QLabel *fontFamilyLabel = new QLabel("字体");
    m_fontComboBox = new FontComboBox;
    m_fontComboBox->setFixedWidth(200);
    m_fontComboBox->addItem(qApp->font().family());
    m_fontComboBox->setCurrentText(qApp->font().family());

    // 设置字体按钮
    m_fontDialogButton = new QPushButton("设置字体");

    fontHLayout->addWidget(fontSizeLabel);
    fontHLayout->addWidget(m_fontSizeBox);
    fontHLayout->addWidget(pxLabel);
    fontHLayout->addSpacing(10);
    fontHLayout->addWidget(fontFamilyLabel);
    fontHLayout->addWidget(m_fontComboBox);
    fontHLayout->addSpacing(10);
    fontHLayout->addWidget(m_fontDialogButton);
    fontHLayout->addStretch();

    // Zoom设置部分
    QLabel *zoomLabel = new QLabel("Zoom");
    zoomLabel->setObjectName("SettingSubtitleLabel");

    QVBoxLayout *zoomVLayout = new QVBoxLayout;
    zoomVLayout->setSpacing(10);
    zoomVLayout->setContentsMargins(0, 0, 0, 0);

    // 页面缩放
    QHBoxLayout *zoomComboLayout = new QHBoxLayout;
    zoomComboLayout->setSpacing(10);
    zoomComboLayout->setContentsMargins(0, 0, 0, 0);

    QLabel *zoomLevelLabel = new QLabel("页面缩放");
    m_zoomComboBox = new ComboBox;
    // 添加缩放比例选项
    QStringList zoomLevels = {"25%", "50%", "75%", "100%", "125%", "150%",
                              "175%", "200%", "250%", "300%", "400%", "500%"};
    m_zoomComboBox->addItems(zoomLevels);
    m_zoomComboBox->setCurrentText("100%");

    zoomComboLayout->addWidget(zoomLevelLabel);
    zoomComboLayout->addWidget(m_zoomComboBox);
    zoomComboLayout->addStretch();

    // 鼠标滚轮缩放复选框
    m_zoomWheelCheckBox = new QCheckBox("按住Ctrl使用时鼠标滚轮缩放");

    zoomVLayout->addLayout(zoomComboLayout);
    zoomVLayout->addWidget(m_zoomWheelCheckBox);

    // 状态栏设置部分
    QLabel *statusBarLabel = new QLabel("设置状态栏");
    statusBarLabel->setObjectName("SettingSubtitleLabel");

    m_showStatusCheckBox = new QCheckBox("显示状态栏");

    // 字数统计设置部分
    QLabel *wordCountLabel = new QLabel("设置字数统计");
    wordCountLabel->setObjectName("SettingSubtitleLabel");

    m_showWordCountCheckBox = new QCheckBox("显示字数统计");

    // 添加到主布局
    mainVLayout->addWidget(titleLabel);
    mainVLayout->addSpacing(20);
    mainVLayout->addWidget(themeLabel);
    mainVLayout->addLayout(themeHLayout);
    mainVLayout->addSpacing(20);
    mainVLayout->addWidget(fontLabel);
    mainVLayout->addLayout(fontHLayout);
    mainVLayout->addSpacing(20);
    mainVLayout->addWidget(zoomLabel);
    mainVLayout->addLayout(zoomVLayout);
    mainVLayout->addSpacing(20);
    mainVLayout->addWidget(statusBarLabel);
    mainVLayout->addWidget(m_showStatusCheckBox);
    mainVLayout->addSpacing(20);
    mainVLayout->addWidget(wordCountLabel);
    mainVLayout->addWidget(m_showWordCountCheckBox);
    mainVLayout->addStretch();
}

void AppearanceSetPage::initConnections()
{
    QObject::connect(m_themeLightRButton, &QRadioButton::toggled, [=](bool checked){
        if(checked) emit settingChanged("appearance", "theme", "light");
    });
    QObject::connect(m_themeDarkRButton, &QRadioButton::toggled, [=](bool checked){
        if(checked) emit settingChanged("appearance", "theme", "dark");
    });
    // 字体设置连接
    QObject::connect(m_fontSizeBox, QOverload<int>::of(&SpinBox::valueChanged), [=](int value){
        emit settingChanged("appearance", "fontSize", value);
    });

    QObject::connect(m_fontComboBox, &FontComboBox::currentTextChanged, [=](const QString& text){
        emit settingChanged("appearance", "fontFamily", text);
    });

    // Zoom设置连接
    QObject::connect(m_zoomComboBox, &ComboBox::currentTextChanged, [=](const QString& text){
        QString zoomText = text;
        zoomText.chop(1); // 移除%符号
        emit settingChanged("appearance", "zoomLevel", zoomText.toInt());
    });

    QObject::connect(m_zoomWheelCheckBox, &QCheckBox::toggled, [=](bool checked){
        emit settingChanged("appearance", "zoomWithWheel", checked);
    });

    // 显示设置连接
    QObject::connect(m_showStatusCheckBox, &QCheckBox::toggled, [=](bool checked){
        emit settingChanged("appearance", "showStatusBar", checked);
    });

    QObject::connect(m_showWordCountCheckBox, &QCheckBox::toggled, [=](bool checked){
        emit settingChanged("appearance", "showWordCount", checked);
    });

    // 字体对话框按钮点击事件
    QObject::connect(m_fontDialogButton, &QPushButton::clicked, [=](){
        bool ok;
        QFont font = QFontDialog::getFont(&ok,
                                          QFont(m_fontComboBox->currentText(), m_fontSizeBox->value()),
                                          nullptr, "选择字体");
        if(ok)
        {
            m_fontSizeBox->setValue(font.pointSize());
            if(m_fontComboBox->findText(font.family()) == -1)
            {
                m_fontComboBox->addItem(font.family());
            }
            m_fontComboBox->setCurrentText(font.family());
        }
    });
}
