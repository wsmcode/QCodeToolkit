#include "logviewer.h"
#include "stylemanager.h"

#include "logmanager.h"
#include <QFile>
#include <QFileSystemWatcher>
#include <QGridLayout>
#include <QTextEdit>
#include <QTextStream>
#include <QRegularExpression>
#include <QApplication>
#include <QClipboard>
#include <QToolButton>
#include <QDebug>
#include <QCheckBox>
#include <QComboBox>
#include <QLabel>
#include <QFileInfo>

LogViewer::LogViewer(QWidget *parent) : FrameLessWindowBase(parent)
{
    setTitleBarColor("#f5f5f5");
    setWindowTitle("日志查看器");
    setWindowIcon(":/res/icon/log.svg");
    setPinButton(":/res/icon/pin.svg", ":/res/icon/pin-off.svg");
    setPinButtonChecked(true);
    setStatusBarHide(true);
    resize(1200, 400);

    // 初始化UI
    initUI();

    // 类成员初始化
    m_currentSessionId = LogManager::getLogManager()->getCurrentSessionId();// 获取当前会话id
    m_onlyCurrentSession = true;
    m_currentLevelFilter = "all";

    // 加载初始日志内容
    loadLogFile();

    m_logConnection = QObject::connect(LogManager::getLogManager(),
                                       &LogManager::logMessageReceived,
                                       [=](const QString &text){
        m_fullLogContent += text;
        // 检查是否需要过滤此条日志
        bool shouldDisplay = true;
        if(m_currentLevelFilter != "all")
        {
            QRegularExpression levelRegex("\\[([a-z]+)\\]");
            QRegularExpressionMatch match = levelRegex.match(text);
            if(match.hasMatch() && match.captured(1) != m_currentLevelFilter)
            {
                shouldDisplay = false;
            }
        }

        if(shouldDisplay)
        {
            m_formattedContent += formatLogEntry(text) + "<br>";
            m_textEdit->setHtml(m_formattedContent);
            // 滚动到文本底部（显示最新日志）
            QTextCursor cursor = m_textEdit->textCursor();
            cursor.movePosition(QTextCursor::End);
            m_textEdit->setTextCursor(cursor);
        }
    });
    qInfo() << "LogViewer initialized successfully";
}

LogViewer::~LogViewer()
{
    qInfo() << "LogViewer destroyed";
    if(m_logConnection) QObject::disconnect(m_logConnection);
}

void LogViewer::loadLogFile(const QString &path)
{
    QString filePath = path;
    if(filePath.isEmpty())
    {
        filePath = LogManager::getLogManager()->getCurrentLogPath();
    }
    qInfo() << QString("Loading log file: %1").arg(QFileInfo(filePath).absoluteFilePath());

    // 打开文件并读取内容
    QFile file(filePath);
    if(file.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        QTextStream in(&file);
        in.setCodec("UTF-8");
        m_fullLogContent = in.readAll();
        file.close();

        // 过滤并显示日志内容
        filterLogContent();
        qInfo() << "Log file loaded successfully";
    }
    else
    {
        qCritical() << QString("Failed to open log file: %1").arg(filePath);
    }
}

void LogViewer::clearLogDisplay()
{
    qInfo() << "Clearing log display";
    m_textEdit->clear();
    m_formattedContent.clear();
}

void LogViewer::copyLogToClipboard()
{
    qInfo() << "Copying log to clipboard";
    QApplication::clipboard()->setText(m_textEdit->toPlainText());
}

void LogViewer::onOnlyCurrentSessionChanged(int state)
{
    bool checked = (state == Qt::Checked);
    qInfo() << QString("Only current session changed: %1").arg(checked ? "checked" : "unchecked");
    m_onlyCurrentSession = checked;
    filterLogContent();
}

void LogViewer::onLevelFilterChanged(int index)
{
    QString level = m_levelFilterComboBox->itemData(index).toString();
    qInfo() << QString("Log level filter changed to: %1").arg(level);
    m_currentLevelFilter = level;
    filterLogContent();
}

void LogViewer::initUI()
{
    // == 获取中心部件 ==
    QWidget *centralWidget = getCentralWidget();

    // ==== 创建工具栏 ====
    // 创建显示对话复选框
    m_onlyCurrentSessionCheckBox = createCheckBox();
    // 创建日志过滤下拉列表
    QLabel *levelLabel = new QLabel("日志级别:"); // 添加日志级别过滤
    levelLabel->setObjectName("LogLevelLabel");
    StyleManager::getStyleManager()->registerWidget(levelLabel);
    m_levelFilterComboBox = createComboBox();
    // 创建清除按钮
    m_clearBtn = createToolButton();
    m_clearBtn->setText("清除");
    m_clearBtn->setToolTip("清除显示内容");
    connect(m_clearBtn, &QToolButton::clicked, this, &LogViewer::clearLogDisplay);
    // 创建复制按钮
    m_copyBtn = createToolButton();
    m_copyBtn->setText("复制");
    m_copyBtn->setToolTip("复制日志到剪贴板");
    connect(m_copyBtn, &QToolButton::clicked, this, &LogViewer::copyLogToClipboard);
    // 工具栏水平布局
    QHBoxLayout *toolbarLayout = new QHBoxLayout;
    toolbarLayout->setSpacing(5);
    toolbarLayout->setContentsMargins(10, 0, 0, 0);
    toolbarLayout->addWidget(m_onlyCurrentSessionCheckBox);
    toolbarLayout->addWidget(levelLabel);
    toolbarLayout->addWidget(m_levelFilterComboBox);
    toolbarLayout->addStretch();
    toolbarLayout->addWidget(m_clearBtn);
    toolbarLayout->addWidget(m_copyBtn);

    // ==== 创建内容显示区 ====
    m_textEdit = createTextEdit();

    // ==== 创建主布局 ====
    QVBoxLayout *vLayout = new QVBoxLayout(centralWidget);
    vLayout->setSpacing(0);
    vLayout->setContentsMargins(0, 0, 0, 0);
    vLayout->addLayout(toolbarLayout);
    vLayout->addWidget(m_textEdit);
}

QCheckBox *LogViewer::createCheckBox()
{
    QCheckBox *checkBox = new QCheckBox("只显示当前对话");
    checkBox->setChecked(true);
    checkBox->setObjectName("LogCheckBox");
    StyleManager::getStyleManager()->registerWidget(checkBox);
    connect(checkBox, &QCheckBox::stateChanged, this, &LogViewer::onOnlyCurrentSessionChanged);
    return checkBox;
}

QComboBox *LogViewer::createComboBox()
{
    QComboBox *comboBox = new QComboBox;
    comboBox->setObjectName("LogComboBox");
    StyleManager::getStyleManager()->registerWidget(comboBox);
    comboBox->addItem("所有级别 all", "all");
    comboBox->addItem("跟踪 trace", "trace");
    comboBox->addItem("调试 debug", "debug");
    comboBox->addItem("信息 info", "info");
    comboBox->addItem("警告 warning", "warning");
    comboBox->addItem("错误 error", "error");
    comboBox->addItem("严重 critical", "critical");
    connect(comboBox, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &LogViewer::onLevelFilterChanged);
    return comboBox;
}

QString LogViewer::formatLogEntry(const QString &line)
{
    // 定义不同日志级别的样式
    QHash<QString, QString> levelStyles;
    levelStyles["trace"] = "color: gray;";
    levelStyles["debug"] = "color: green;";
    levelStyles["info"] = "color: blue;";
    levelStyles["warning"] = "color: orange; font-weight: bold;";
    levelStyles["error"] = "color: red; font-weight: bold;";
    levelStyles["critical"] = "color: darkred; font-weight: bold; background-color: #ffeeee;";

    // 默认样式
    QString style = "color: black; margin: 2px 0";

    // 检测日志级别并应用响应样式
    QRegularExpression levelRegex("\\[([a-z]+)\\]");
    QRegularExpressionMatch match = levelRegex.match(line);

    if(match.hasMatch())
    {
        QString level = match.captured(1);
        if(levelStyles.contains(level))
        {
            style = levelStyles[level];
        }
    }
    // 高亮"会话开始"标记
    if(line.contains("==== Session Started ===="))
    {
        style = "color: purple; font-weight: bold; background-color: #f0f0ff; padding: 3px;";
    }

    // 转义HTML特殊字符
    QString escapedLine = line.toHtmlEscaped();

    return QString("<div style='%1'>%2</div>").arg(style, escapedLine);
}

QToolButton *LogViewer::createToolButton()
{
    QToolButton *toolButton = new QToolButton;
    toolButton->setObjectName("LogToolButton");
    StyleManager::getStyleManager()->registerWidget(toolButton);
    toolButton->setFixedSize(60, 30);
    return toolButton;
}

QTextEdit *LogViewer::createTextEdit()
{
    QTextEdit *textEdit = new QTextEdit;
    textEdit->setReadOnly(true);
    textEdit->setObjectName("LogTextEdit");
    StyleManager::getStyleManager()->registerWidget(textEdit);
    return textEdit;
}

void LogViewer::filterLogContent()
{
    QString formattedContent;
    QStringList lines = m_fullLogContent.split("\n");

    // 查找当前会话开始位置
    int currentSessionStart = -1;
    QString sessionStartStr = "==== Session Started ==== Session ID: \"" + m_currentSessionId + "\"";

    int size = lines.size();
    for(int i = 0; i < size; i++)
    {
        if(lines[i].contains(sessionStartStr))
        {
            currentSessionStart = i;
            break;
        }
    }
    // 处理日志行
    m_formattedContent.clear();

    QRegularExpression regex(R"(\\[\\d+/\\d+\\])"); // 匹配 [进程ID/线程ID]

    QRegularExpression levelRegex("\\[([a-z]+)\\]"); // 匹配日志级别
    for(int i = 0; i < lines.size(); i++)
    {
        const QString &line = lines[i];
        if(!line.trimmed().isEmpty())
        {
            // 如果只显示当前会话，则跳过当前会话开始之前的内容
            if(m_onlyCurrentSession && currentSessionStart != -1 && i < currentSessionStart)
            {
                continue;
            }
            if(m_currentLevelFilter != "all")
            {
                QRegularExpressionMatch match = levelRegex.match(line);
                if(match.hasMatch() && match.captured(1) != m_currentLevelFilter)
                {
                    continue;
                }
            }
            QString formattedLine = line;
            if(currentSessionStart >= i)
            {
                formattedLine.remove(regex);
            }
            m_formattedContent += formatLogEntry(formattedLine) + "<br>";
        }
    }
    m_textEdit->setHtml(m_formattedContent);

    // 滚动到文本底部（显示最新日志）
    QTextCursor cursor = m_textEdit->textCursor();
    cursor.movePosition(QTextCursor::End);
    m_textEdit->setTextCursor(cursor);
}
