#include "codesetpage.h"

#include <QCheckBox>
#include <QFontDatabase>
#include <QHeaderView>
#include <QLabel>
#include <QPlainTextEdit>
#include <QPushButton>
#include <QTableWidget>
#include <QVBoxLayout>
#include <QDebug>
#include <combobox.h>
#include <stylemanager.h>
#include <QMessageBox>
#include <QJsonArray>
#include <QJsonObject>
#include <QJsonDocument>

CodeSetPage::CodeSetPage(QWidget *parent) : SettingsPage(parent)
{
    initUI();
    initConnections();
    // emit settingChanged("code", "snippets", doc.toJson(QJsonDocument::Compact));
    // 如果考虑性能影响，改成只发送改变的片段
    m_applySnippetBtn->setEnabled(false);
//    m_addSnippetBtn->setEnabled(false);
//    m_removeSnippetBtn->setEnabled(false);
}

QString CodeSetPage::title() const
{
    return "代码";
}

void CodeSetPage::load(const AppSettings &settings)
{
    m_indentComboBox->setCurrentText(settings.code.defaultIndent);
    m_languageComboBox->setCurrentText(settings.code.defaultLanguage);
    m_pairSymbolsCheckBox->setChecked(settings.code.usePairedSymbols);
    m_lineNumbersCheckBox->setChecked(settings.code.showLineNumbers);

    m_snippets = settings.code.snippets;
}

void CodeSetPage::save(AppSettings &settings)
{
    settings.code.defaultIndent = m_indentComboBox->currentText();
    settings.code.defaultLanguage = m_languageComboBox->currentText();
    settings.code.usePairedSymbols = m_pairSymbolsCheckBox->isChecked();
    settings.code.showLineNumbers = m_lineNumbersCheckBox->isChecked();

    settings.code.snippets = m_snippets;
}

void CodeSetPage::initUI()
{
    QWidget *mainWidget = new QWidget;
    setWidget(mainWidget);
    mainWidget->setObjectName("SettingScrollWidget");
    StyleManager::getStyleManager()->registerWidget(mainWidget);

    QVBoxLayout *mainVLayout = new QVBoxLayout(mainWidget);
    mainVLayout->setSpacing(10);
    mainVLayout->setContentsMargins(10, 10, 0, 10);

    QLabel *titleLabel = new QLabel("代码");
    titleLabel->setObjectName("SettingTitleLabel");

    // 格式设置部分
    QLabel *formatLabel = new QLabel("格式设置");
    formatLabel->setObjectName("SettingSubtitleLabel");

    QHBoxLayout *indentLayout = new QHBoxLayout;
    indentLayout->setSpacing(10);
    indentLayout->setContentsMargins(0, 0, 0, 0);

    QLabel *indentLabel = new QLabel("默认缩进");

    m_indentComboBox = new ComboBox;
    QStringList indentOptions = {"2", "4", "8", "Tab"};
    m_indentComboBox->addItems(indentOptions);
    m_indentComboBox->setCurrentText("4");

    indentLayout->addWidget(indentLabel);
    indentLayout->addWidget(m_indentComboBox);
    indentLayout->addStretch();

    // 默认代码语言设置部分
    QLabel *languageLabel = new QLabel("默认代码语言");

    QHBoxLayout *languageLayout = new QHBoxLayout;
    languageLayout->setSpacing(10);
    languageLayout->setContentsMargins(0, 0, 0, 0);

    m_languageComboBox = new ComboBox;
    QStringList languages = {
        "C++", "Python", "JavaScript", "Java", "C#",
        "PHP", "Swift", "Kotlin", "Go", "Rust",
        "TypeScript", "Ruby", "HTML", "CSS", "SQL",
        "R", "MATLAB", "Shell", "PowerShell", "Dart"
    };
    m_languageComboBox->addItems(languages);
    m_languageComboBox->setCurrentText("C++");

    languageLayout->addWidget(languageLabel);
    languageLayout->addWidget(m_languageComboBox);
    languageLayout->addStretch();

    QLabel *displayAndInputLabel = new QLabel("显示与输入");
    displayAndInputLabel->setObjectName("SettingSubtitleLabel");

    // 使用成对符号设置部分
    m_pairSymbolsCheckBox = new QCheckBox("使用成对符号");
    m_pairSymbolsCheckBox->setChecked(true); // 默认选中

    // 代码行号设置部分
    m_lineNumbersCheckBox = new QCheckBox("显示代码行号");
    m_lineNumbersCheckBox->setChecked(true); // 默认选中

    QLabel *snippetsLabel = new QLabel("设置片段");
    snippetsLabel->setObjectName("SettingSubtitleLabel");
    // 设置片段列表
    m_snippetsTable = new QTableWidget;
    m_snippetsTable->setFixedHeight(250);
    m_snippetsTable->setColumnCount(2);
    m_snippetsTable->setHorizontalHeaderLabels({"触发", "触发种类"});
    m_snippetsTable->verticalHeader()->setVisible(false);
    // 设置表格属性
    m_snippetsTable->setFocusPolicy(Qt::NoFocus); // 移除焦点框
    m_snippetsTable->setShowGrid(false);
    m_snippetsTable->setSelectionBehavior(QAbstractItemView::SelectRows); // 整行选择
    m_snippetsTable->setSelectionMode(QAbstractItemView::SingleSelection); // 单行选择
    m_snippetsTable->setEditTriggers(QAbstractItemView::DoubleClicked); // 双击编辑
    m_snippetsTable->horizontalHeader()->setStretchLastSection(true);
    m_snippetsTable->verticalHeader()->setDefaultSectionSize(15);
    m_snippetsTable->horizontalHeader()->setDefaultAlignment(Qt::AlignVCenter);
    m_snippetsTable->horizontalHeader()->setSelectionMode(QAbstractItemView::NoSelection);
    m_snippetsTable->horizontalHeader()->setHighlightSections(false);
    m_snippetsTable->horizontalHeader()->setSectionsClickable(false);
    m_snippetsTable->horizontalHeader()->setSectionsMovable(false);

    m_addSnippetBtn = new QPushButton("添加");
    m_removeSnippetBtn = new QPushButton("删除");
    QVBoxLayout *tableButtonsVLayout = new QVBoxLayout;
    tableButtonsVLayout->setContentsMargins(0, 5, 0, 0);
    tableButtonsVLayout->addWidget(m_addSnippetBtn);
    tableButtonsVLayout->addSpacing(5);
    tableButtonsVLayout->addWidget(m_removeSnippetBtn);
    tableButtonsVLayout->addStretch();

    QHBoxLayout *snippetTableLayout = new QHBoxLayout;
    snippetTableLayout->setContentsMargins(0, 0, 30, 0);
    snippetTableLayout->setSpacing(5);
    snippetTableLayout->addWidget(m_snippetsTable, 1);
    snippetTableLayout->addLayout(tableButtonsVLayout);

    m_snippetsEdit = new QPlainTextEdit;
    m_snippetsEdit->setFixedHeight(250);

    // 设置等宽字体
    QFont font = QFontDatabase::systemFont(QFontDatabase::FixedFont);
    m_snippetsEdit->setFont(font);

    // 设置水平滚动策略
    m_snippetsEdit->setLineWrapMode(QPlainTextEdit::NoWrap); // 不自动换行
    m_snippetsEdit->setHorizontalScrollBarPolicy(Qt::ScrollBarAsNeeded); // 需要时显示水平滚动条

    // 设置Tab键宽度（4个空格宽度）
    QFontMetrics metrics(font);
    m_snippetsEdit->setTabStopDistance(4 * metrics.horizontalAdvance(' '));

    m_applySnippetBtn = new QPushButton("应用");
    QVBoxLayout *editButtonsVLayout = new QVBoxLayout;
    editButtonsVLayout->setContentsMargins(0, 5, 0, 0);
    editButtonsVLayout->addWidget(m_applySnippetBtn);
    editButtonsVLayout->addStretch();

    QHBoxLayout *m_snippetsEditLayout = new QHBoxLayout;
    m_snippetsEditLayout->setContentsMargins(0, 0, 30, 0);
    m_snippetsEditLayout->setSpacing(5);
    m_snippetsEditLayout->addWidget(m_snippetsEdit, 1);
    m_snippetsEditLayout->addLayout(editButtonsVLayout);

    mainVLayout->addWidget(titleLabel);
    mainVLayout->addSpacing(20);
    mainVLayout->addWidget(formatLabel);
    mainVLayout->addLayout(indentLayout);
    mainVLayout->addLayout(languageLayout);
    mainVLayout->addSpacing(20);
    mainVLayout->addWidget(displayAndInputLabel);
    mainVLayout->addWidget(m_pairSymbolsCheckBox);
    mainVLayout->addWidget(m_lineNumbersCheckBox);
    mainVLayout->addSpacing(20);
    mainVLayout->addWidget(snippetsLabel);
    mainVLayout->addLayout(snippetTableLayout);
    mainVLayout->addLayout(m_snippetsEditLayout);
    mainVLayout->addStretch();
}

void CodeSetPage::initConnections()
{
    // 连接信号和槽
    QObject::connect(m_indentComboBox, &ComboBox::currentTextChanged, [=](const QString& text){
        emit settingChanged("code", "defaultIndent", text);
    });

    QObject::connect(m_languageComboBox, &ComboBox::currentTextChanged, [=](const QString& text){
        emit settingChanged("code", "defaultLanguage", text);
    });

    QObject::connect(m_pairSymbolsCheckBox, &QCheckBox::toggled, [=](bool checked){
        emit settingChanged("code", "usePairedSymbols", checked);
    });

    QObject::connect(m_lineNumbersCheckBox, &QCheckBox::toggled, [=](bool checked){
        emit settingChanged("code", "showLineNumbers", checked);
    });

    // 实现片段管理功能
    QObject::connect(m_applySnippetBtn, &QPushButton::clicked, [=](){
        // 应用片段编辑
        int currentRow = m_snippetsTable->currentRow();
        if(currentRow >= 0 && currentRow < m_snippets.size())
        {
            QString content = m_snippetsEdit->toPlainText();

            // 更新片段数据
            if(m_snippets[currentRow].content == content) return;
            m_snippets[currentRow].content = content;
            m_snippets[currentRow].modified = QDateTime::currentDateTime();

            // 将snippets转换为JSON字符串发送
            QJsonArray snippetsArray;
            for(const Snippet &snippet : m_snippets)
            {
                QJsonObject snippetObj;
                snippetObj["trigger"] = snippet.trigger;
                snippetObj["content"] = snippet.content;
                snippetObj["triggerType"] = snippet.triggerType;
                snippetObj["created"] = snippet.created.toString(Qt::ISODate);
                snippetObj["modified"] = snippet.modified.toString(Qt::ISODate);
                snippetsArray.append(snippetObj);
            }

            QJsonDocument doc(snippetsArray);

            emit settingChanged("code", "snippets", doc.toJson(QJsonDocument::Compact));
        }

    });

    QObject::connect(m_addSnippetBtn, &QPushButton::clicked, [=](){
        // 添加新片段行
        m_snippetsTable->insertRow(0);
        m_snippetsTable->setItem(0, 0, new QTableWidgetItem(""));
        m_snippetsTable->setItem(0, 1, new QTableWidgetItem(""));
        m_snippetsTable->setRowHeight(0, 12); // 设置行高

        m_snippetsTable->selectRow(0);
        m_snippetsTable->editItem(m_snippetsTable->item(0, 0));
    });

    QObject::connect(m_removeSnippetBtn, &QPushButton::clicked, [=](){
        // 删除选中的片段行
        int currentRow = m_snippetsTable->currentRow();
        if(currentRow >= 0 && currentRow < m_snippets.size())
        {
            m_snippetsTable->removeRow(currentRow);
            m_snippets.removeAt(currentRow);
            if(currentRow == m_snippetsTable->currentRow())
            {
                m_snippetsEdit->clear();
            }
        }
    });

    // 片段表格项改变时的验证cellPressed
    QObject::connect(m_snippetsTable, &QTableWidget::cellChanged, [=](int row, int column){
        int rowCount = m_snippetsTable->rowCount();
        if(m_snippets.size()  < rowCount)
        {
            QString triggerStr = "trigger" + QString::number(rowCount);
            Snippet newSnippet;
            newSnippet.trigger = triggerStr;
            m_snippets.prepend(newSnippet); // 使用prepend添加到开头
            m_snippetsTable->item(row, column)->setText(triggerStr);
            return;
        }
        if(m_snippets[row].trigger == m_snippetsTable->item(row, 0)->text())
        {
            return;
        }
        QString trigger = m_snippets[row].trigger;
        qDebug() << "row" << row << "column" << column << m_snippetsTable->item(row, column)->text();
        if(column == 0)
        {
            QString newTrigger = m_snippetsTable->item(row, column)->text();
            if(isValidTrigger(newTrigger))
            {
                m_snippets[row].trigger = newTrigger;
            }
            else
            {
                QMessageBox::warning(this, "错误", "不是一个有效的触发");
                m_snippetsTable->item(row, column)->setText(trigger);
            }
        }
        else if(column == 1)
        {
            QString triggerType = m_snippetsTable->item(row, column)->text();
            m_snippets[row].triggerType = triggerType;
            m_snippets[row].modified = QDateTime::currentDateTime();
        }
    });

    QObject::connect(m_snippetsTable, &QTableWidget::itemSelectionChanged, [=](){

        // 当选择改变时，将选中片段的内容加载到编辑框
        int currentRow = m_snippetsTable->currentRow();
        if(currentRow >= 0 && currentRow < m_snippets.size())
        {
            QString trigger = m_snippetsTable->item(currentRow, 0)->text();
            QString content = m_snippets[currentRow].content;
            m_snippetsEdit->setPlainText(content);
        }
    });
}

// 验证辅助函数
bool CodeSetPage::isValidTrigger(const QString &trigger)
{
    if(trigger.isEmpty())
        return false;

    // 验证规则：只能包含字母、数字、下划线，不能以数字开头
    QRegExp validTriggerRegex("^[a-zA-Z_][a-zA-Z0-9_]*$");
    return validTriggerRegex.exactMatch(trigger);
}
