//
// Created by hc on 2020/5/6.
//

#ifndef C20_CREALTIMEVIDEOSERVER_H
#define C20_CREALTIMEVIDEOSERVER_H

#include "muduo/net/TcpServer.h"
#include "CDecoder.h"
#include "../Dao/CRedisCluster.h"
#include "CURLManager.h"
#include <unordered_set>
#include <boost/circular_buffer.hpp>

typedef struct
{
    std::string m_sMobileNumber;
    std::string m_sAudioCodingMode;
    std::string m_sAudioChanelNum;
    std::string m_sAudioRate;
    std::string m_sAudioRateBit;
    std::string m_sAudioFrameLengh;
    std::string m_sEnableAudioOut;
    std::string m_sCameraCodingMode;
    std::string m_sMaxAudioChanelNum;
    std::string m_sMaxCameraChanelNUm;
    std::string m_sVehiclePlateNo;
    std::string m_sVehiclePlateColor;
    std::string m_sMediaFlag;
    std::string m_sIp;
    std::string m_sPort;
    std::string m_sToken;
    std::string m_sState;
    std::string m_sLiveCount;
    std::string m_sParaKey;
    std::string m_sLiveStatKey;
}REDIS_INFO;

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
    std::string GetServerName() const;
private:
    void __OnConnection(const muduo::net::TcpConnectionPtr& iConn);
    void __OnMessage(const muduo::net::TcpConnectionPtr& iConn,muduo::net::Buffer* pBuf,muduo::Timestamp iTime);
    void __OnTimer();
    void __DumpConnectionBuckets() const ;

private:
    typedef std::weak_ptr<muduo::net::TcpConnection> WEAK_TCP_CONNECTION_PTR;
    typedef std::shared_ptr<CRedisCluster> SHARED_REDIS_CLUSTER_PTR;
    typedef std::shared_ptr<CURLManager> SHARED_URL_MANAGER_PTR;
    struct Entry:public muduo::copyable
    {
        explicit Entry(const WEAK_TCP_CONNECTION_PTR& iWeakConn,CRealTimeVideoServer * pServer);
        ~Entry();

        std::string EscapeURL(const std::string &sURL);
        std::string DeescapeURL(const std::string &sURL);
        char Dec2HexChar(short int n);
        short int HexChar2Dec(char c);
        bool GetInfoFromRedis();
        std::string GetURL();
        bool InitOnRedisState();
        bool CloseOnRedisState();
        WEAK_TCP_CONNECTION_PTR m_iWeakConn;
        CDecoder m_iWeakDecoder;
        REDIS_INFO m_iRedisInfo;
        CRealTimeVideoServer * m_pServer;
        std::string m_sRTMPURL;
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
    SHARED_REDIS_CLUSTER_PTR m_iRedis;
    SHARED_URL_MANAGER_PTR m_iURLManager;
    int m_nRTMPPort;
    std::string m_sRTMPAddr;
};


#endif //C20_CREALTIMEVIDEOSERVER_H
