#include "mainwidget.h"

#include "logmanager.h"
#include "settingmanager.h"

#include <QApplication>

/*****************************************************
*
* @file     main.cpp
* @brief    main函数
*
* @description
*           ==== 使用说明 ====
*           ==== 注意 ====
*           1. 代码文本字符集：UTF-8
*           2. Qscitinlla库直接集成到了Qt路径中
*               1. 下载链接：https://www.riverbankcomputing.com/software/qscintilla/download
*               2. 参考的教程：https://blog.csdn.net/qq_43680827/article/details/122611652
*           3. 全局主题样式设置时，**MainWidget类**和**TreeWidget类**都连接了themeChanged信号
*               1. MainWidget连接到无边框父类，设置其主题（所以无边框主题需要到无边框类中修改）
*               2. TreeWidget连接改变item的图片设置（颜色）
*
* @author   无声目
* @date     2025/08/29
* @history
*****************************************************/

int main(int argc, char *argv[])
{
    SetConsoleOutputCP(CP_UTF8);
    SetConsoleCP(CP_UTF8);

    QApplication a(argc, argv);
    a.setFont(QFont("Microsoft YaHei", 9));

    // 初始化日志系统
    LogManager::getLogManager()->init();
    // 记录应用启动信息
    auto logger = LogManager::getLogManager()->logger();
    if(logger)
    {
        qInfo("Application started");
        qInfo("Qt version: %s", qVersion());
        qInfo("Application directory: %s", QCoreApplication::applicationDirPath().toStdString().c_str());
        qInfo("Command line arguments: %d", argc);
        for(int i = 0; i < argc; i++)
        {
            qInfo("  Argument %d: %s", i, argv[i]);
        }
    }
    MainWidget w;
    w.show();
    SettingManager::getSettingManager()->applyCurrentSettings();

    // 进入主事件循环
    int result = a.exec();

    // 记录应用程序退出信息
    if(logger)
    {
        qInfo("Application exited with code: %d", result);
    }
    // 关闭日志系统
    LogManager::getLogManager()->shutdown();

    return result;
}
