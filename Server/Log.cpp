//
// Created by hc on 2020/5/7.
//
#include "Log.h"
#include "muduo/base/AsyncLogging.h"
#include "muduo/base/Logging.h"
#include "muduo/base/Thread.h"
#include "muduo/base/LogFile.h"

using namespace muduo;

int kRollSize = 500*1000*1000;
std::unique_ptr<muduo::AsyncLogging> g_asyncLog;
std::unique_ptr<muduo::LogFile> g_logFile;
FILE* g_file;

void AsyncOutput(const char* msg, int len)
{
    g_asyncLog->append(msg, len);
}

void SetLogging(const char* argv0,bool async,muduo::Logger::LogLevel level)
{
    char name[256];
    strncpy(name, argv0, 256);
//    assert(Logger::LogLevel::TRACE<=nLogLevel && Logger::LogLevel::NUM_LOG_LEVELS>=nLogLevel);
    if(async)
    {
        muduo::Logger::setOutput(AsyncOutput);
        muduo::Logger::setLogLevel(level);

//
        g_asyncLog.reset(new muduo::AsyncLogging(::basename(name), kRollSize,2));
        g_asyncLog->start();

    }
    else
    {
        g_file = ::fopen(::basename(name), "ae");
//        g_logFile.reset(new muduo::LogFile(::basename(name), kRollSize,nLogLevel));
        muduo::Logger::setOutput(FileOutput);
        muduo::Logger::setFlush(FileFlush);
    }
}



void OutputFunc(const char* msg, int len)
{
    g_logFile->append(msg, len);
}

void FlushFunc()
{
    g_logFile->flush();
}

void FileOutput(const char* msg, int len)
{
    if (g_file)
    {
        fwrite(msg, 1, len, g_file);
    }
    else if (g_logFile)
    {
        g_logFile->append(msg, len);
    }
}

void FileFlush()
{
    fflush(g_file);
}
