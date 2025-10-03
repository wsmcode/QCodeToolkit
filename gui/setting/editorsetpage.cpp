#include "editorsetpage.h"

#include <QCheckBox>
#include <QLabel>
#include <QVBoxLayout>
#include <combobox.h>
#include <stylemanager.h>

EditorSetPage::EditorSetPage(QWidget *parent) : SettingsPage(parent)
{
    initUI();
    m_wheelSpeedComboBox->setEnabled(false); // 把QString改成int 同时类中处理m_wheelSpeedComboBox的text转为int
}

QString EditorSetPage::title() const
{
    return "编辑器";
}

void EditorSetPage::load(const AppSettings &settings)
{
    m_outerWheelFollowsInnerCheckBox->setChecked(settings.editor.scrollFollow);
    m_wheelSpeedComboBox->setCurrentText(settings.editor.scrollSpeed);
    m_saveOnCloseCheckBox->setChecked(settings.editor.saveOnClose);
    m_autoSaveCheckBox->setChecked(settings.editor.autoSave);
}

void EditorSetPage::save(AppSettings &settings)
{
    settings.editor.scrollFollow = m_outerWheelFollowsInnerCheckBox->isChecked();
    settings.editor.scrollSpeed = m_wheelSpeedComboBox->currentText();
    settings.editor.saveOnClose = m_saveOnCloseCheckBox->isChecked();
    settings.editor.autoSave = m_autoSaveCheckBox->isChecked();
}

void EditorSetPage::initUI()
{
    QWidget *mainWidget = new QWidget;
    setWidget(mainWidget);
    mainWidget->setObjectName("SettingScrollWidget");
    StyleManager::getStyleManager()->registerWidget(mainWidget);

    QVBoxLayout *mainVLayout = new QVBoxLayout(mainWidget);
    mainVLayout->setSpacing(10);
    mainVLayout->setContentsMargins(10, 10, 0, 10);

    // 创建UI控件
    QLabel *titleLabel = new QLabel("编辑器");
    titleLabel->setObjectName("SettingTitleLabel");

    // 滚轮设置部分
    QLabel *wheelLabel = new QLabel("滚轮设置");
    wheelLabel->setObjectName("SettingSubtitleLabel");
    // 外滚轮是否跟随内滚轮设置部分
    m_outerWheelFollowsInnerCheckBox = new QCheckBox("外滚轮是否跟随内滚轮");
    m_outerWheelFollowsInnerCheckBox->setChecked(true); // 默认选中

    QHBoxLayout *wheelSpeedHLayout = new QHBoxLayout;
    wheelSpeedHLayout->setSpacing(10);
    wheelSpeedHLayout->setContentsMargins(0, 0, 0, 0);

    QLabel *wheelSpeedLabel = new QLabel("滚轮速度");
    m_wheelSpeedComboBox = new ComboBox;
    QStringList speedOptions = {"慢", "中", "快", "自定义"};
    m_wheelSpeedComboBox->addItems(speedOptions);
    m_wheelSpeedComboBox->setCurrentText("中");

    wheelSpeedHLayout->addWidget(wheelSpeedLabel);
    wheelSpeedHLayout->addWidget(m_wheelSpeedComboBox);
    wheelSpeedHLayout->addStretch();

    // 保存设置部分
    QLabel *saveLabel = new QLabel("保存设置");
    saveLabel->setObjectName("SettingSubtitleLabel");

    // 关闭后直接保存设置部分
    m_saveOnCloseCheckBox = new QCheckBox("关闭后直接保存");
    m_saveOnCloseCheckBox->setChecked(false); // 默认不选中
    // 自动保存设置部分
    m_autoSaveCheckBox = new QCheckBox("自动保存");
    m_autoSaveCheckBox->setChecked(false); // 默认不选中

    // 添加到主布局
    mainVLayout->addWidget(titleLabel);
    mainVLayout->addSpacing(20);
    mainVLayout->addWidget(wheelLabel);
    mainVLayout->addWidget(m_outerWheelFollowsInnerCheckBox);
    mainVLayout->addLayout(wheelSpeedHLayout);
    mainVLayout->addSpacing(20);
    mainVLayout->addWidget(saveLabel);
    mainVLayout->addWidget(m_saveOnCloseCheckBox);
    mainVLayout->addWidget(m_autoSaveCheckBox);
    mainVLayout->addStretch();


}

void EditorSetPage::initConnections()
{
    // 连接信号和槽
    QObject::connect(m_outerWheelFollowsInnerCheckBox, &QCheckBox::toggled, [=](bool checked){
        emit settingChanged("editor", "scrollFollow", checked);
    });

    QObject::connect(m_wheelSpeedComboBox, &ComboBox::currentTextChanged, [=](const QString& text){
        emit settingChanged("appearance", "scrollSpeed", text);
    });

    QObject::connect(m_saveOnCloseCheckBox, &QCheckBox::toggled, [=](bool checked){
        emit settingChanged("editor", "saveOnClose", checked);
    });

    QObject::connect(m_autoSaveCheckBox, &QCheckBox::toggled, [=](bool checked){
        emit settingChanged("editor", "autoSave", checked);
    });

    // 自动保存选中时，"关闭后直接保存"置灰
    QObject::connect(m_autoSaveCheckBox, &QCheckBox::stateChanged, [=](int state){
        m_autoSaveCheckBox->setEnabled(state == Qt::Unchecked);
    });
}
