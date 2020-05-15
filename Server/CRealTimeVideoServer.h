//
// Created by hc on 2020/5/6.
//

#ifndef C20_CREALTIMEVIDEOSERVER_H
#define C20_CREALTIMEVIDEOSERVER_H

#include "muduo/net/TcpServer.h"
#include "CDecoder.h"
#include <unordered_set>
#include <boost/circular_buffer.hpp>

class CRealTimeVideoServer {
public:
    CRealTimeVideoServer(muduo::net::EventLoop * iLoop,
            const muduo::net::InetAddress& iListenAddr,
            int nIdleSeconds,
            std::string sServerName,
            int nNumThreads);

public:
    void Start();
    void SetThreadNum(int nNumThreads);
    bool WriteDataToStream(CDecoder & iCoder,JT1078_MEDIA_DATA_TYPE eDataType);//,AVMediaType iDataType, char *pData, int nDataLen);

private:
    void __OnConnection(const muduo::net::TcpConnectionPtr& iConn);
    void __OnMessage(const muduo::net::TcpConnectionPtr& iConn,muduo::net::Buffer* pBuf,muduo::Timestamp iTime);
    void __OnTimer();
    void __DumpConnectionBuckets() const ;

private:
    typedef std::weak_ptr<muduo::net::TcpConnection> WEAK_TCP_CONNECTION_PTR;

    struct Entry:public muduo::copyable
    {
        explicit Entry(const WEAK_TCP_CONNECTION_PTR& iWeakConn);
        ~Entry();

        WEAK_TCP_CONNECTION_PTR m_iWeakConn;
        CDecoder m_iWeakDecoder;
    };
    typedef std::shared_ptr<Entry> ENTRY_PTR;
    typedef std::weak_ptr<Entry> WEAK_ENTRY_PTR;
    typedef std::unordered_set<ENTRY_PTR> BUCKET;
    typedef boost::circular_buffer<BUCKET > WEAK_CONNECTION_LIST;
private:
    int m_nNumThreads;
    muduo::net::TcpServer m_iServer;
    WEAK_CONNECTION_LIST m_iConnectionBuckets;
    std::string m_sServerName;

};


#endif //C20_CREALTIMEVIDEOSERVER_H
