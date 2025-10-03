#ifndef LOGMANAGER_H
#define LOGMANAGER_H

#include <QObject>
#include <QMessageLogContext>
#include <memory>
#include <spdlog/spdlog.h>

/*****************************************************
*
* @file     logmanager.h
* @brief    LogManager 日志管理类 - 单例模式
*
* @description
*           ==== 核心功能 ====
*           1. 实现了基本的日志功能
*           2. 提供了Qt消息到spdlog的桥接
*           ==== 使用说明 ====
*           ==== 注意 ====
*
* @author   无声目
* @date     2025/08/29
* @history
*****************************************************/
class LogManager : public QObject
{
    Q_OBJECT
public:
    // 单例模式
    static LogManager *getLogManager()
    {
        static LogManager log;
        return &log;
    }
    // 删除拷贝构造函数和赋值运算符
    LogManager(const LogManager&) = delete;
    LogManager& operator=(const LogManager&) = delete;

    // 初始化日志系统
    void init(); // 创建日志目录，设置日志格式，注册Qt消息处理函数
    // 关闭日志系统
    void shutdown(); // 清理spdlog资源，确保所有日志消息被刷新
    // 获取默认日志记录器
    std::shared_ptr<spdlog::logger> logger() const;
    // Qt消息处理
    static void qtMessageHandler(QtMsgType type, const QMessageLogContext &context, const QString &msg);

    QString getCurrentLogPath() const;
    QString getCurrentSessionId() const;

signals:
    void logMessageReceived(const QString &message);

private:
    explicit LogManager(QObject *parent = nullptr);
    ~LogManager();

//    ~LogManager() = default;

    // 默认日志记录器
    std::shared_ptr<spdlog::logger> m_logger;

    QString m_currentLogPath;  // 存储当前日志文件路径

    QString m_sessionId;  // 当前会话ID
};

#endif // LOGMANAGER_H
