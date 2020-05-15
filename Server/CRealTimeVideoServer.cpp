//
// Created by hc on 2020/5/6.
//
#include <assert.h>
#include <stdio.h>
#include "CRealTimeVideoServer.h"
#include <functional>
#include "muduo/base/Logging.h"
#include "muduo/net/EventLoop.h"
#include "muduo/net/EventLoopThreadPool.h"
#include "JT1078Header.h"

using namespace muduo;
using namespace muduo::net;
using std::bind;

CRealTimeVideoServer::CRealTimeVideoServer(muduo::net::EventLoop * iLoop,
        const muduo::net::InetAddress& iListenAddr,
        int nIdleSeconds,
        std::string sServerName,
        int nNumThreads)
    :m_iServer(iLoop,iListenAddr,sServerName),
     m_iConnectionBuckets(nIdleSeconds),
     m_sServerName(sServerName),
     m_nNumThreads(nNumThreads>0?nNumThreads:0)
{
    m_iServer.setConnectionCallback(std::bind(&CRealTimeVideoServer::__OnConnection,this,_1));
    m_iServer.setMessageCallback(std::bind(&CRealTimeVideoServer::__OnMessage,this,_1,_2,_3));
    iLoop->runEvery(1.0,std::bind(&CRealTimeVideoServer::__OnTimer,this));
    m_iConnectionBuckets.resize(nIdleSeconds);
    __DumpConnectionBuckets();
    SetThreadNum(m_nNumThreads);
}

void CRealTimeVideoServer::Start()
{
    m_iServer.start();
}

void CRealTimeVideoServer::SetThreadNum(int nNumThreads)
{
    m_iServer.setThreadNum(nNumThreads);
}

void CRealTimeVideoServer::__OnConnection(const muduo::net::TcpConnectionPtr& conn)
{
    LOG_INFO<<"RealTimeVideoServer - " << conn->peerAddress().toIpPort() << " -> "
            << conn->localAddress().toIpPort() << "is" << (conn->connected() ? "UP" : "DOWN");
    if(conn->connected())
    {
        ENTRY_PTR entryPtr(new Entry(conn));
        m_iConnectionBuckets.back().insert(entryPtr);
//        __DumpConnectionBuckets();
        WEAK_ENTRY_PTR weakEntryPtr(entryPtr);
        conn->setContext(weakEntryPtr);
    }
    else
    {
        assert(!conn->getContext().empty());
        WEAK_ENTRY_PTR weakEntryPtr(boost::any_cast<WEAK_ENTRY_PTR>(conn->getContext()));
        LOG_DEBUG << "Entry use_count = " << weakEntryPtr.use_count();
    }
}

void CRealTimeVideoServer::__OnMessage(const muduo::net::TcpConnectionPtr& conn,muduo::net::Buffer* pBuf,muduo::Timestamp time)
{

//    muduo::string msg(pBuf->retrieveAllAsString());
//    muduo::string msg(pBuf->retrieveAsString(100));
//    string s;
//    LOG_INFO << s.max_size();
//    LOG_INFO << conn->name() << " echo " << msg.size() << " bytes, "
//             << "data received at " << time.toString();
//    conn->send(msg);
    static int i = 0;
    i++;
    printf("static int i=%d\n",i);
    bool bSuccess;
    assert(!conn->getContext().empty());
    WEAK_ENTRY_PTR weakEntry(boost::any_cast<WEAK_ENTRY_PTR>(conn->getContext()));
    ENTRY_PTR entry(weakEntry.lock());
    if(entry)
    {
        m_iConnectionBuckets.back().insert(entry);
        __DumpConnectionBuckets();
    }
    CDecoder & iCoder = entry->m_iWeakDecoder;
    bSuccess = iCoder.Decode(pBuf,time);
    if (!bSuccess)
    {
        conn->shutdown();
    }
    else
    {
        JT1078_CUR_RECEIVE_STATE eStat = iCoder.GetCurReceiveStat();
        if(eStat == JT1078_CUR_RECEIVE_STATE::eCompleted)
        {
            LOG_DEBUG << "收到完整包";

            JT1078_MEDIA_DATA_TYPE eDataType  = iCoder.GetDataType();
            JT1078_ERRS eErr = iCoder.GetErr();
            JT1078_SUB_MARKE eMarke = iCoder.GetProcessingMarke();
            assert(eErr == JT1078_ERRS::eNoError);
            assert(eDataType != JT1078_MEDIA_DATA_TYPE::ePassthrough);
            assert(eMarke == JT1078_SUB_MARKE::eAtomic || eMarke == JT1078_SUB_MARKE::eLast);
            if(!iCoder.GetPushState())
            {

                string sUrl = "rtmp://192.168.0.141:20002/live/%E4%BA%91L34018.%E9%BB%91%E8%89%B2.4.0.1";
                bSuccess = iCoder.Init(sUrl);
                if(!bSuccess)
                {
                    LOG_ERROR << "初始化Stream失败"<< "--->" << "URL:" << sUrl;
                    conn->forceClose();
                }
            }
            bSuccess = WriteDataToStream(iCoder,eDataType);
            if(!bSuccess)
            {
                LOG_ERROR << "写入" << ((eDataType == JT1078_MEDIA_DATA_TYPE::eAudio)?"音频":"视频") << "失败"
                        << "--->" << "URL:" << iCoder.GetUrl();
                conn->forceClose();
            }
            iCoder.SetCurReceiveStat(JT1078_CUR_RECEIVE_STATE::eInit);
        }
        else
        {
            LOG_DEBUG << "不是完整包";
        }
    }
}

void CRealTimeVideoServer::__OnTimer()
{
    m_iConnectionBuckets.push_back(BUCKET());
//    __DumpConnectionBuckets();
}

void CRealTimeVideoServer::__DumpConnectionBuckets() const
{
    LOG_DEBUG << "size = " << m_iConnectionBuckets.size();
    int idx = 0;
    for (WEAK_CONNECTION_LIST::const_iterator pBucketI = m_iConnectionBuckets.begin();
         pBucketI != m_iConnectionBuckets.end();
         ++pBucketI, ++idx)
    {
        const BUCKET& iBucket = *pBucketI;
        printf("[%d] len = %zd : ", idx, iBucket.size());
        for (const auto& it : iBucket)
        {
            bool connectionDead = it->m_iWeakConn.expired();
            printf("%p(%ld)%s, ", get_pointer(it), it.use_count(),
                   connectionDead ? " DEAD" : "");
        }
//        puts("");
    }
}

CRealTimeVideoServer::Entry::Entry(const WEAK_TCP_CONNECTION_PTR & iWeakConn)
    :m_iWeakConn(iWeakConn)
{

}

CRealTimeVideoServer::Entry::~Entry()
{
    muduo::net::TcpConnectionPtr conn = m_iWeakConn.lock();
    if(conn)
    {
        conn->shutdown();
    }
}

bool CRealTimeVideoServer::WriteDataToStream(CDecoder & iCoder,JT1078_MEDIA_DATA_TYPE eDataType)//,AVMediaType iDataType, char *pData, int nDataLen)
{
    bool b;
    int nRet;
    JT1078_AV_CODING_TYPE eType = iCoder.GetAVCodingType();
    assert(eType!=JT1078_AV_CODING_TYPE::eUnSupport);

    if(eDataType == JT1078_MEDIA_DATA_TYPE::eVideoI ||
       eDataType == JT1078_MEDIA_DATA_TYPE::eVideoP ||
       eDataType == JT1078_MEDIA_DATA_TYPE::eVideoB)
    {
        b = iCoder.WriteData(AVMEDIA_TYPE_VIDEO, const_cast<char *>(iCoder.GetData().data()),iCoder.GetData().size());
    }
    else// if (eDataType == JT1078_MEDIA_DATA_TYPE::eAudio)
    {
        b = iCoder.WriteData(AVMEDIA_TYPE_AUDIO, const_cast<char *>(iCoder.GetData().data()),iCoder.GetData().size());
    }
    iCoder.GetData().clear();
    return b;
}
