#ifndef LOGVIEWER_H
#define LOGVIEWER_H

#include <QWidget>
#include "framelesswindowbase.h"
/*****************************************************
*
* @file     logviewer.h
* @brief    日志查看器
*
* @description
*           ==== 布局 ====
*
*           ==== 核心功能 ====
*           ==== 使用说明 ====
*           ==== 注意 ====
*
* @author   无声目
* @date     2025/08/31
* @history
*****************************************************/

class QTextEdit;
class QFileSystemWatcher;
class QCheckBox;
class QComboBox;
class QToolButton;
class LogViewer : public FrameLessWindowBase
{
    Q_OBJECT
public:
    explicit LogViewer(QWidget *parent = nullptr);
    ~LogViewer();

public slots:
    void loadLogFile(const QString &path = "");
    void clearLogDisplay();
    void copyLogToClipboard();
    void onOnlyCurrentSessionChanged(int state);
    void onLevelFilterChanged(int index);

signals:

private:
    void initUI();
    QCheckBox *createCheckBox();
    QComboBox *createComboBox();
    QToolButton *createToolButton();
    QTextEdit *createTextEdit();
    QString formatLogEntry(const QString &line);
    void filterLogContent();

    QTextEdit *m_textEdit;         // 显示日志内容的文本编辑框

    QToolButton *m_clearBtn;       // 清除显示按钮
    QToolButton *m_copyBtn;        // 复制日志按钮
    QCheckBox *m_onlyCurrentSessionCheckBox;  // 只显示当前会话复选框
    QComboBox *m_levelFilterComboBox; // 日志级别过滤组合框

    QString m_currentSessionId;    // 当前会话ID
    QString m_fullLogContent;      // 存储完整的日志内容
    QString m_formattedContent;    // 存储当前的日志内容
    bool m_onlyCurrentSession; // 是否只显示当前会话
    QString m_currentLevelFilter;  // 当前选择的日志级别过滤

    QMetaObject::Connection m_logConnection; // 添加连接对象
};

#endif // LOGVIEWER_H
