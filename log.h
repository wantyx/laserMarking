#pragma once
#ifndef LOG_H
#define LOG_H


#include <QFile>
#include <QMessageLogger>
#include <qlogging.h>
#include <QFile>
#include <QTextStream>
#include <QDateTime>

extern QMessageLogger* gMLog;
void logSysInit(QString filePath);
void logSysInit();

//打印日志到文件中
//qDebug("This is a debug message");
//qWarning("This is a warning message");
//qCritical("This is a critical message");
//qFatal("This is a fatal message");


#endif // LOG_H