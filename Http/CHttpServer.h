//
// Created by hc on 2020/5/22.
//

#ifndef JT1078SERVER_CHTTPSERVER_H
#define JT1078SERVER_CHTTPSERVER_H

#include <muduo/net/TcpServer.h>
//using muduo::net::EventLoop;
//using muduo::net::InetAddress;
class CHttpRequest;
class CHttpResponse;

class CHttpServer : muduo::noncopyable
{
public:
    typedef std::function<void (const CHttpRequest&,CHttpResponse*)> HTTP_CALLBACK;
    CHttpServer(muduo::net::EventLoop* pLoop,const muduo::net::InetAddress & rListenAddr,const std::string &name,muduo::net::TcpServer::Option option = muduo::net::TcpServer::kNoReusePort);

public:
    muduo::net::EventLoop * GetLoop() const ;
    void SetHttpCallback(const HTTP_CALLBACK& cb);
    void SetThreadNum(int nNumThreads);
    void Start();

private:
    void __OnConnection(const muduo::net::TcpConnectionPtr& rConn);
    void __OnMessage(const muduo::net::TcpConnectionPtr & rConn,muduo::net::Buffer * pBuf,muduo::Timestamp iReceiveTime);
    void __OnRequest(const muduo::net::TcpConnectionPtr & rConn,const CHttpRequest& rHttpReq);

private:
    muduo::net::TcpServer m_iTcpServer;
    HTTP_CALLBACK m_pfnHttpCallback;

};


#endif //JT1078SERVER_CHTTPSERVER_H
