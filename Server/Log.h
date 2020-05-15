//
// Created by hc on 2020/5/7.
//

#ifndef C20_LOG_H
#define C20_LOG_H

#include "muduo/base/Logging.h"

void AsyncOutput(const char* msg, int len);
void SetLogging(const char* argv0,bool async,muduo::Logger::LogLevel level);
void OutputFunc(const char* msg, int len);
void FlushFunc();
void FileFlush();
void FileOutput(const char* msg, int len);

#endif //C20_LOG_H
