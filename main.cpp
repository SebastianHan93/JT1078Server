#include <iostream>
#include "muduo/net/TcpServer.h"
#include "muduo/net/EventLoop.h"
#include "muduo/base/AsyncLogging.h"
#include "muduo/base/Logging.h"
#include "muduo/base/Thread.h"
#include "Server/CRealTimeVideoServer.h"
#include "Server/Log.h"
#include "Server/JT1078Header.h"

using namespace muduo;
using namespace muduo::net;


int main() {
//    SetLogging("C20VideoServer.log", true,Logger::LogLevel::INFO);
    muduo::Logger::setLogLevel(muduo::Logger::DEBUG);
    EventLoop loop;
    InetAddress iListenAddr(2007);
    int nIdleSeconds = 2;
    LOG_INFO << "pid = " << getpid() << ", idle seconds = " << nIdleSeconds;
    CRealTimeVideoServer iVideoServer(&loop,iListenAddr,nIdleSeconds,"RealTimeVideoServer",4);
    iVideoServer.Start();
    loop.loop();
    return 0;
}



