#ifndef LOGGER_H
#define LOGGER_H

#pragma execution_character_set("utf-8")
/*******************************************************************************************
 *
 * 模块:  日志模块
 *可以参考: https://blog.csdn.net/bloke_come/article/details/76090845
 * 1.提供分级日志
 *
 * 使用方法:
 *
 *    1. 在main函数中   #include "logger.h"
 *    2. 在main函数中添加   qInstallMessageHandler(outputMessage);
 *    3. 在 .pro 文件中添加以下内容(两个全局宏定义):
 *                DEFINES += LOG_TO_FILE
 *                DEFINES += AM_STR_LOG_FILE_PATH=\\\"am_log.txt\\\"
**********************************************************************************************/
//是否输出日志到文件
#ifdef LOG_TO_FILE

#include <QMutex>
#include <QFile>
#include <QTextStream>
#include <QDateTime>
#include <QDebug>


static QMutex g_LogFileMutex;   //必须加锁, 否则会导致日志输出格式错乱(一行多条日志)

static void OutputMessageHandler(QtMsgType nType, const QMessageLogContext &msgLogContext, const QString &strMsg)
{
    //static QMutex g_LogFileMutex;   //必须加锁, 否则会导致日志输出格式错乱(一行多条日志)
    g_LogFileMutex.lock();
    QString strLevelText;

    switch(nType)
    {
    case QtDebugMsg:
        strLevelText = QString("[Debug]: ");
        break;

    case QtWarningMsg:
        strLevelText = QString("[Warning]: ");
        break;

    case QtCriticalMsg:
        strLevelText = QString("[Critical]: ");
        break;

    case QtFatalMsg:
        strLevelText = QString("[Fatal]: ");
        break;

    default:
        strLevelText = QString("[Debug]: ");
        break;
    }
    QString strContextInfo = QString("File:(%1) Line:(%2)").arg(QString(msgLogContext.file)).arg(msgLogContext.line);
    QString strCurDateTime = QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss ddd");
    QString strCurTime = QString("(%1)").arg(strCurDateTime);
    QString strLogMessage = QString("%1 %2 %3 %4").arg(strLevelText).arg(strContextInfo).arg(strMsg).arg(strCurTime);


    QFile file(AM_STR_LOG_FILE_PATH);
    file.open(QIODevice::WriteOnly | QIODevice::Append);
    QTextStream textStream(&file);
    textStream << strLogMessage << "\r\n";
    file.flush();
    file.close();


    g_LogFileMutex.unlock();
}

#endif

#endif // LOGGER_H
