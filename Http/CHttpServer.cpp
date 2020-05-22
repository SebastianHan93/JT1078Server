//
// Created by hc on 2020/5/22.
//

#include "CHttpServer.h"
#include "CHttpRequest.h"
#include "CHttpResponse.h"
#include "CHttpContext.h"
#include "../../../muduo/build/release-install-cpp11/include/muduo/base/Logging.h"

using namespace muduo;
using namespace muduo::net;
using std::placeholders::_1;
using std::placeholders::_2;
using std::placeholders::_3;
void defaultHttpCallback(const CHttpRequest&, CHttpResponse* resp)
{
    resp->SetStatusCode(CHttpResponse::e404NotFound);
    resp->SetStatusMessage("Not Found");
    resp->SetCloseConnection(true);
}

CHttpServer::CHttpServer(muduo::net::EventLoop* pLoop,const muduo::net::InetAddress & rListenAddr,const std::string &rName,muduo::net::TcpServer::Option eOption)
    :m_iTcpServer(pLoop,rListenAddr,rName,eOption),
     m_pfnHttpCallback(defaultHttpCallback)
{
    m_iTcpServer.setConnectionCallback(std::bind(&CHttpServer::__OnConnection,this,_1));
    m_iTcpServer.setMessageCallback(std::bind(&CHttpServer::__OnMessage,this,_1,_2,_3));
}

muduo::net::EventLoop * CHttpServer::GetLoop() const
{
    return m_iTcpServer.getLoop();
}

void CHttpServer::SetHttpCallback(const HTTP_CALLBACK& cb)
{
    m_pfnHttpCallback = cb;
}

void CHttpServer::SetThreadNum(int nNumThreads)
{
    m_iTcpServer.setThreadNum(nNumThreads);
}

void CHttpServer::Start()
{
    LOG_WARN << "HttpServer[" << m_iTcpServer.name()
             << "] 开始监听： " << m_iTcpServer.ipPort();
    m_iTcpServer.start();
}

void CHttpServer::__OnConnection(const muduo::net::TcpConnectionPtr& rConn)
{
    if(rConn->connected())
    {
        rConn->setContext(CHttpContext());
    }
}

void CHttpServer::__OnMessage(const muduo::net::TcpConnectionPtr & rConn,muduo::net::Buffer * pBuf,muduo::Timestamp iReceiveTime)
{
    CHttpContext* pContext = boost::any_cast<CHttpContext>(rConn->getMutableContext());

    if (!pContext->ParseRequest(pBuf, iReceiveTime))
    {
        rConn->send("HTTP/1.1 400 Bad Request\r\n\r\n");
        rConn->shutdown();
    }

    if (pContext->GotAll())
    {
        __OnRequest(rConn, pContext->Request());
        pContext->Reset();
    }
}

void CHttpServer::__OnRequest(const muduo::net::TcpConnectionPtr & rConn,const CHttpRequest& rHttpReq)
{
    const string& rConnection = rHttpReq.GetHeader("Connection");
    bool close = rConnection == "close" ||
                 (rHttpReq.GetVersion() == CHttpRequest::eHttp10 && rConnection != "Keep-Alive");
    CHttpResponse rHttpResponse(close);
    m_pfnHttpCallback(rHttpReq, &rHttpResponse);
    Buffer buf;
    rHttpResponse.AppendToBuffer(&buf);
    rConn->send(&buf);
    if (rHttpResponse.IsCloseConnection())
    {
        rConn->shutdown();
    }
}