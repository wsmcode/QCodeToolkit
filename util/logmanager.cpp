#include "logmanager.h"

#include <QDir>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/sinks/stdout_sinks.h>
#include <spdlog/sinks/daily_file_sink.h>
#include <QCoreApplication>
#include <QDateTime>
#include <QDebug>
#include <QUuid>

/***********************
 * @brief 初始化日志系统
 *
 * 创建日志目录，设置控制台和文件输出，配置日志格式，安装Qt消息处理函数
 ***********************/
void LogManager::init()
{
    try {
        // 生成唯一会话ID
        m_sessionId = QUuid::createUuid().toString(QUuid::WithoutBraces);

        // 创建日志目录（如果不存在）
        QString exeDir = QCoreApplication::applicationDirPath();
        QString path = exeDir + "/logs";
        QString filePath = path + "/app.log";
        QDir().mkdir(path);

        // 创建控制台输出sink（用于开发时查看日志）
        auto console_sink = std::make_shared<spdlog::sinks::stderr_color_sink_mt>(); // Qt控制台无法输出
//        auto console_sink = std::make_shared<spdlog::sinks::stdout_sink_mt>(); // 应用程序输出窗口的信息会中文乱码
        // 创建每日文件sink（每天生成一个新的日志文件）
        auto file_sink = std::make_shared<spdlog::sinks::daily_file_sink_mt>(filePath.toStdString(), 0, 0);
        m_currentLogPath = QString::fromStdString(file_sink->filename());
        // 创建多个sink的logger（同时输出到控制台和文件）
        m_logger = std::make_shared<spdlog::logger>("logger",
                                                    spdlog::sinks_init_list{console_sink, file_sink});
        // 创建日志格式：[%Y-%m-%d %H:%M:%S.%f] [%P/%t] [%l] [%n]  %v
        auto formatter = std::make_unique<spdlog::pattern_formatter>();
        formatter->set_pattern("[%Y-%m-%d %H:%M:%S.%f] [%P/%t] [%l]  %v");
        m_logger->set_formatter(std::move(formatter)); // 将formatter左值强制转化为右值
        // 设置日志级别（跟踪基本及以上会被记录）
        m_logger->set_level(spdlog::level::trace);
        // 设置刷新级别（INFO及以上级别立即刷新到输出）
        m_logger->flush_on(spdlog::level::info);
        // 注册为全局logger，方便其他地方使用spdlog::info()函数
        spdlog::register_logger(m_logger);
        spdlog::set_default_logger(m_logger);
        // 安装Qt消息处理函数，将qDebug等重定向到spdlog
        qInstallMessageHandler(qtMessageHandler);

        // 记录会话开始
        qInfo() << "==== Session Started ==== Session ID:" << m_sessionId;
    }
    catch(const spdlog::spdlog_ex& ex)
    {
        // 日志初始化失败，使用Qt原生系统记录错误
        qCritical("Log initialization failed: %s", ex.what());
    }
}

/***********************
 * @brief 关闭日志系统
 *
 * 调用spdlog的shutdown函数，确保所有日志消息被刷新
 ***********************/
void LogManager::shutdown()
{
    if(m_logger)
    {
        spdlog::shutdown();
        m_logger.reset();
    }
}

std::shared_ptr<spdlog::logger> LogManager::logger() const
{
    return m_logger;
}

/***********************
 * @brief Qt消息处理函数
 *
 * 将Qt的日志消息转换为spdlog格式，使用string_view避免不必要的字符串拷贝
 *
 * @param type 消息类型
 * @param context 消息上下文
 * @param msg 消息内容
 ***********************/
void LogManager::qtMessageHandler(QtMsgType type, const QMessageLogContext &context, const QString &msg)
{
    // 获取默认日志记录器
    auto logger = spdlog::default_logger();
    if(!logger) return;

    QByteArray utf8Msg = msg.toUtf8();
    std::string message(utf8Msg.constData(), utf8Msg.length());
    std::string file = context.file ? QByteArray(context.file).constData() : "";
    std::string function = context.function ? QByteArray(context.function).constData() : "";
    // 只要文件名
    size_t lastSlash = file.find_last_of("/\\");
    if(lastSlash != std::string::npos)
    {
        file = file.substr(lastSlash + 1);
    }
    std::stringstream text;
    text << " [" << file << ":" << context.line << ", " << function << "] - " << message;
    // 映射Qt日志级别到spdlog级别
    QString level_str;

    // 映射Qt日志级别到spdlog级别
    switch(type)
    {
    case QtDebugMsg:
        level_str = "debug";
        logger->debug(text.str());
        break;
    case QtInfoMsg:
        level_str = "info";
        logger->info(text.str());
        break;
    case QtWarningMsg:
        level_str = "warning";
        logger->warn(text.str());
        break;
    case QtCriticalMsg:
        level_str = "critical";
        logger->critical(text.str());
        break;
    case QtFatalMsg:
        level_str = "error";
        logger->error(text.str());
        break;
    }
    QString formattedMsg = QString("[%1] [%2]  %3")
            .arg(QDateTime::currentDateTime().toString("yyyy/MM/dd hh:mm:ss.zzz"))
            .arg(level_str).arg(QString::fromStdString(text.str()));
    // 发射信号（通过单例获取实例）
    emit LogManager::getLogManager()->logMessageReceived(formattedMsg);
}

QString LogManager::getCurrentLogPath() const
{
    return m_currentLogPath;
}

QString LogManager::getCurrentSessionId() const
{
    return m_sessionId;
}

LogManager::LogManager(QObject *parent) : QObject(parent)
{

}

LogManager::~LogManager()
{
    shutdown();
}
