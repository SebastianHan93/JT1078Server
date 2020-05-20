#include <iostream>
#include "muduo/net/TcpServer.h"
#include "muduo/net/EventLoop.h"
#include "muduo/base/AsyncLogging.h"
#include "muduo/base/Logging.h"
#include "muduo/base/Thread.h"
#include "Server/CRealTimeVideoServer.h"
#include "Server/Log.h"
#include "Server/JT1078Header.h"
#include "Configuration/CConf.h"
#include "Server/Daemon.h"

using namespace muduo;
using namespace muduo::net;


int main() {

    CConf * pConf = CConf::GetInstance();
    if(!pConf->LoadConf("../Server.conf"))
    {
        LOG_ERROR << "打开配置文件失败";
        return -1;
    }
    muduo::Logger::setLogLevel(muduo::Logger::DEBUG);

    const char * pDir = pConf->GetString("DaemonRoot");
    if(pConf->GetIntDefault("Daemon",0) == 1)
    {
        int nRet = Daemon(pDir);
        if(nRet<0)
        {
            LOG_ERROR << "开启守护进程失败";
            return -1;
        }
        if(nRet==1)
        {
            LOG_ERROR << "开启守护进程成功";
            return 0;
        }
    }


//    SetLogging("JT1078Server.log", true,Logger::LogLevel::DEBUG);
//    SetLogging("JT1078Server.log", true, static_cast<muduo::Logger::LogLevel>(pConf->GetIntDefault("LogLevel",1)));

    EventLoop loop;
    int LivePort = pConf->GetIntDefault("LiveListenPort", 20002);;
    InetAddress iLiveListenAddr(LivePort);
    int nIdleSeconds = pConf->GetIntDefault("IdleSeconds", 10);
    CRealTimeVideoServer iVideoServer(&loop, iLiveListenAddr, nIdleSeconds, "live", 4);
    iVideoServer.Start();
    loop.loop();


    return 0;
}



