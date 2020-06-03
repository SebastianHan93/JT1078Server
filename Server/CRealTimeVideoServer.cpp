//
// Created by hc on 2020/5/6.
//
#include <cassert>
#include <cstdio>
#include "CRealTimeVideoServer.h"
#include <functional>
#include "muduo/base/Logging.h"
#include "muduo/net/EventLoop.h"
#include "muduo/net/EventLoopThreadPool.h"
#include "JT1078Header.h"
#include "../Configuration/CConf.h"

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
     m_nNumThreads(nNumThreads>0?nNumThreads:0),
     m_iRedis(new CRedisCluster()),
     m_iURLManager(new CURLManager())
{
    CConf *pConf = CConf::GetInstance();
    m_iServer.setConnectionCallback(std::bind(&CRealTimeVideoServer::__OnConnection, this, _1));
    m_iServer.setMessageCallback(std::bind(&CRealTimeVideoServer::__OnMessage, this, _1, _2, _3));
    iLoop->runEvery(2.0, std::bind(&CRealTimeVideoServer::__OnTimer, this));
    m_iConnectionBuckets.resize(nIdleSeconds);
    //__DumpConnectionBuckets();
    SetThreadNum(m_nNumThreads);
    if (m_sServerName == "live") {
        m_nRTMPPort = pConf->GetIntDefault("RTMPLivePort", 20002);
    } else {
        m_nRTMPPort = pConf->GetIntDefault("RTMPHistoryPort", 20003);
    }
    m_sRTMPAddr = pConf->GetString("RTMPServer");
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
        ENTRY_PTR entryPtr(new Entry(conn,this));
        m_iConnectionBuckets.back().insert(entryPtr);
//        __DumpConnectionBuckets();
        WEAK_ENTRY_PTR weakEntryPtr(entryPtr);
        conn->setContext(weakEntryPtr);
    }
    else
    {
        assert(!conn->getContext().empty());
        WEAK_ENTRY_PTR weakEntryPtr(boost::any_cast<WEAK_ENTRY_PTR>(conn->getContext()));
        LOG_INFO << "Entry use_count = " << weakEntryPtr.use_count();
        ENTRY_PTR iEntry(weakEntryPtr.lock());
        if(iEntry)
        {
           LOG_INFO << "退出推流" <<iEntry->GetURL();
        }
    }
}

void CRealTimeVideoServer::__OnMessage(const muduo::net::TcpConnectionPtr& conn,muduo::net::Buffer* pBuf,muduo::Timestamp time)
{
    bool bSuccess;
    assert(!conn->getContext().empty());
    WEAK_ENTRY_PTR weakEntry(boost::any_cast<WEAK_ENTRY_PTR>(conn->getContext()));
    ENTRY_PTR iEntry(weakEntry.lock());
    if(iEntry)
    {
        m_iConnectionBuckets.back().insert(iEntry);
        //__DumpConnectionBuckets();
    }
    CDecoder & iCoder = iEntry->m_iWeakDecoder;

    bSuccess = iCoder.Decode(pBuf,time);
    if (!bSuccess) {
        conn->forceClose();
        return;
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
                string sUrl;
                if(iEntry->m_sRTMPURL.empty())
                {
                    bSuccess = iEntry->GetInfoFromRedis();
                    if(!bSuccess) {
                        LOG_INFO << "获取redis数据出错,关闭连接";
                        conn->forceClose();
                        return;
                    }

                    sUrl = iEntry->GetURL();
                }

                bSuccess = iCoder.Init(sUrl);
                if(!bSuccess) {
                    LOG_ERROR << "初始化Stream失败" << "--->" << "URL:" << sUrl;
                    conn->forceClose();
                    return;
                }

                bSuccess = iCoder.InitAACEncoder(11025,1);
                if(!bSuccess) {
                    LOG_ERROR << "初始化AACEncoder失败" << "--->" << "URL:" << sUrl;
                    conn->forceClose();
                    return;
                }

                if(!iEntry->InitOnRedisState())
                {
                    conn->forceClose();
                    return;
                }
            }
            bSuccess = WriteDataToStream(iCoder,eDataType);
            if(!bSuccess) {
                LOG_ERROR << "写入" << ((eDataType == JT1078_MEDIA_DATA_TYPE::eAudio) ? "音频" : "视频") << "失败"
                          << "--->" << "URL:" << iCoder.GetUrl();
                conn->forceClose();
                return;
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
    time_t tNowTime=time(NULL);
    muduo::Timestamp iTimestamp = muduo::Timestamp::now();
    m_iRedis->Hset("streamServerKeepAlive","timeStamp",iTimestamp.toString());
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

CRealTimeVideoServer::Entry::Entry(const WEAK_TCP_CONNECTION_PTR & iWeakConn,CRealTimeVideoServer * iServer)
    :m_iWeakConn(iWeakConn),
     m_pServer(iServer)
{

}

CRealTimeVideoServer::Entry::~Entry()
{
    muduo::net::TcpConnectionPtr conn = m_iWeakConn.lock();
    if(conn)
    {
        conn->shutdown();
    }

    if(!m_iRedisInfo.m_sLiveStatKey.empty() && (m_pServer->GetServerName() == "live"))
    {
        CloseOnRedisState();
    }

}

bool CRealTimeVideoServer::Entry::InitOnRedisState()
{
    if(!m_pServer->m_iRedis->Hset(m_iRedisInfo.m_sLiveStatKey,"state","0"))
    {
        LOG_ERROR << "CRealTimeVideoServer::Entry::InitOnRedisState-->更改redis状态失败-->Key["
                    << m_iRedisInfo.m_sLiveStatKey << "] "
                    <<"filed[" << "state" << "]";
        return false;
    }
    return true;
}

bool CRealTimeVideoServer::Entry::CloseOnRedisState()
{
    if(m_pServer->GetServerName() == "live")
    {
        bool b;
        b = m_pServer->m_iRedis->Hset(m_iRedisInfo.m_sLiveStatKey,"state","-1");
        if(!b)
        {
            LOG_ERROR << "CRealTimeVideoServer::Entry::CloseOnRedisState-->更改redis状态失败-->Key["
                      << m_iRedisInfo.m_sLiveStatKey << "] "
                      <<"filed[" << "state" << "]";
            return false;
        }

        b = m_pServer->m_iRedis->Hset(m_iRedisInfo.m_sLiveStatKey,"liveCount","0");
        if(!b)
        {
            LOG_ERROR << "CRealTimeVideoServer::Entry::CloseOnRedisState-->更改redis状态失败-->Key["
                      << m_iRedisInfo.m_sLiveStatKey << "] "
                      <<"filed[" << "liveCount" << "]";
            return false;
        }
        return true;
    }
    return true;
}

std::string CRealTimeVideoServer::GetServerName() const
{
    return m_sServerName;
}

std::string CRealTimeVideoServer::Entry::GetURL() {
    char gStream[100] = {0};
    char gRtmpUrl[200] = {0};
    JT_1078_HEADER &iHeader = m_iWeakDecoder.GetHeader();
    sprintf(gStream, "%s.%s.%d.%s.%s",
            m_iRedisInfo.m_sVehiclePlateNo.c_str(),
            m_iRedisInfo.m_sVehiclePlateColor.c_str(),
            iHeader.Bt1LogicChannelNumber,
            m_iRedisInfo.m_sMediaFlag.c_str(),
            "1");
    //m_iRedisInfo.m_sToken.c_str());

    string sStream = EscapeURL(gStream);

    sprintf(gRtmpUrl, "rtmp://%s:%d/%s/%s",
            m_pServer->m_sRTMPAddr.data(),
            m_pServer->m_nRTMPPort,
            m_pServer->GetServerName().data(),
            sStream.data()
    );
    LOG_INFO << "sStream-->" << gStream;
    LOG_INFO << "EscapeURL-->"<<gRtmpUrl;
    m_sRTMPURL = gRtmpUrl;
    return gRtmpUrl;
}

bool CRealTimeVideoServer::Entry::GetInfoFromRedis()
{
    JT_1078_HEADER &iHeader = m_iWeakDecoder.GetHeader();
    char gParaKey[50] = {0};
    sprintf(gParaKey, "%s%02X%02X%02X%02X%02X%02X",
            "stream:",
            iHeader.BCDSIMCardNumber[0],
            iHeader.BCDSIMCardNumber[1],
            iHeader.BCDSIMCardNumber[2],
            iHeader.BCDSIMCardNumber[3],
            iHeader.BCDSIMCardNumber[4],
            iHeader.BCDSIMCardNumber[5]);
    LOG_DEBUG << "+++++++" << gParaKey;
    LOG_DEBUG << "+++++++++" << iHeader.Bt1LogicChannelNumber;
    m_iRedisInfo.m_sParaKey = gParaKey;
    m_iRedisInfo.m_sIp = m_pServer->m_iRedis->Hget(m_iRedisInfo.m_sParaKey, "ip");
    m_iRedisInfo.m_sPort = m_pServer->m_iRedis->Hget(m_iRedisInfo.m_sParaKey, "port");
    m_iRedisInfo.m_sToken = m_pServer->m_iRedis->Hget(m_iRedisInfo.m_sParaKey, "token");
    m_iRedisInfo.m_sMobileNumber = m_pServer->m_iRedis->Hget(m_iRedisInfo.m_sParaKey, "mobileNumber");
    m_iRedisInfo.m_sAudioCodingMode = m_pServer->m_iRedis->Hget(m_iRedisInfo.m_sParaKey, "audioCodingMode");
    m_iRedisInfo.m_sAudioChanelNum = m_pServer->m_iRedis->Hget(m_iRedisInfo.m_sParaKey, "audioChanelNum");
    m_iRedisInfo.m_sAudioRate = m_pServer->m_iRedis->Hget(m_iRedisInfo.m_sParaKey, "audioRate");
    m_iRedisInfo.m_sAudioRateBit = m_pServer->m_iRedis->Hget(m_iRedisInfo.m_sParaKey, "audioRateBit");
    m_iRedisInfo.m_sAudioFrameLengh = m_pServer->m_iRedis->Hget(m_iRedisInfo.m_sParaKey, "audioFrameLengh");
    m_iRedisInfo.m_sEnableAudioOut = m_pServer->m_iRedis->Hget(m_iRedisInfo.m_sParaKey,"enableAudioOut");
    m_iRedisInfo.m_sCameraCodingMode = m_pServer->m_iRedis->Hget(m_iRedisInfo.m_sParaKey,"cameraCodingMode");
    m_iRedisInfo.m_sMaxAudioChanelNum = m_pServer->m_iRedis->Hget(m_iRedisInfo.m_sParaKey,"maxAudioChanelNum");
    m_iRedisInfo.m_sMaxCameraChanelNUm = m_pServer->m_iRedis->Hget(m_iRedisInfo.m_sParaKey,"maxCameraChanelNUm");
    m_iRedisInfo.m_sVehiclePlateNo = m_pServer->m_iRedis->Hget(m_iRedisInfo.m_sParaKey,"vehiclePlateNo");
    m_iRedisInfo.m_sVehiclePlateColor = m_pServer->m_iRedis->Hget(m_iRedisInfo.m_sParaKey,"vehiclePlateColor");
    m_iRedisInfo.m_sMediaFlag = m_pServer->m_iRedis->Hget(m_iRedisInfo.m_sParaKey,"mediaFlag");
    if(m_iRedisInfo.m_sVehiclePlateColor.empty() || m_iRedisInfo.m_sVehiclePlateNo.empty() || m_iRedisInfo.m_sMediaFlag.empty())
    {
        LOG_INFO << "从redis获取终端信息失败";
        return false;
    }
    m_iRedisInfo.m_sLiveStatKey = m_iRedisInfo.m_sParaKey+":"+std::to_string(iHeader.Bt1LogicChannelNumber) + ":" + "live";
    LOG_DEBUG << "REDIS LEY -->" <<m_iRedisInfo.m_sLiveStatKey;
    return true;

}

string CRealTimeVideoServer::Entry::EscapeURL(const string &URL)
{
    string result = "";
    for ( unsigned int i=0; i<URL.size(); i++ ) {
        char c = URL[i];
        if (
                ( '0'<=c && c<='9' ) ||
                ( 'a'<=c && c<='z' ) ||
                ( 'A'<=c && c<='Z' ) ||
                c=='/' || c=='.'
                ) {
            result += c;
        } else {
            int j = (short int)c;
            if ( j < 0 ) {
                j += 256;
            }
            int i1, i0;
            i1 = j / 16;
            i0 = j - i1*16;
            result += '%';
            result += Dec2HexChar(i1);
            result += Dec2HexChar(i0);
        }
    }
    return result;
}

string CRealTimeVideoServer::Entry::DeescapeURL(const string &URL) {
    string result = "";
    for ( unsigned int i=0; i<URL.size(); i++ ) {
        char c = URL[i];
        if ( c != '%' ) {
            result += c;
        } else {
            char c1 = URL[++i];
            char c0 = URL[++i];
            int num = 0;
            num += HexChar2Dec(c1) * 16 + HexChar2Dec(c0);
            result += char(num);
        }
    }
    return result;
}

char CRealTimeVideoServer::Entry::Dec2HexChar(short int n)
{
    if ( 0 <= n && n <= 9 ) {
        return char( short('0') + n );
    } else if ( 10 <= n && n <= 15 ) {
        return char( short('A') + n - 10 );
    } else {
        return char(0);
    }
}

short int CRealTimeVideoServer::Entry::HexChar2Dec(char c)
{
    if ( '0'<=c && c<='9' ) {
        return short(c-'0');
    } else if ( 'a'<=c && c<='f' ) {
        return ( short(c-'a') + 10 );
    } else if ( 'A'<=c && c<='F' ) {
        return ( short(c-'A') + 10 );
    } else {
        return -1;
    }
}



bool CRealTimeVideoServer::WriteDataToStream(CDecoder & iCoder,JT1078_MEDIA_DATA_TYPE eDataType)//,AVMediaType iDataType, char *pData, int nDataLen)
{
    bool bSucc = false;
    int nRet;
    JT1078_AV_CODING_TYPE eType = iCoder.GetAVCodingType();
    assert(eType!=JT1078_AV_CODING_TYPE::eUnSupport);

    if(eDataType == JT1078_MEDIA_DATA_TYPE::eVideoI ||
       eDataType == JT1078_MEDIA_DATA_TYPE::eVideoP ||
       eDataType == JT1078_MEDIA_DATA_TYPE::eVideoB)
    {

        bSucc = iCoder.WriteH264((unsigned char *)iCoder.GetData().data(),iCoder.GetData().size());
        if(!bSucc)
            return false;
    }
    else// if (eDataType == JT1078_MEDIA_DATA_TYPE::eAudio)
    {
        DECODE_RESULT &iResult = iCoder.DecodeAudio2PCM(const_cast<char *>(iCoder.GetData().data()),iCoder.GetData().size(), iCoder.GetAVCodingType());
        switch (iResult.m_eType)
        {
            case CCodec::AUDIO_CODING_TYPE::eUnSupport:
                LOG_INFO << "不支持的音频类型!";
                bSucc = false;
                break;
            case CCodec::AUDIO_CODING_TYPE::eEncodeError:
                LOG_INFO << "编码失败!";
                bSucc = false;
                break;
            default:
                bSucc = true;
                break;
        }
        if(iResult.m_eType!=AUDIO_CODING_TYPE::eAAC && bSucc)
        {
//            bSucc = iCoder.WriteData(AVMEDIA_TYPE_AUDIO, iResult.m_pOutBuf, iResult.m_nOutBufLen);
            AAC_DATA & aacData = iCoder.Pcm2AAC((unsigned char*)iResult.m_pOutBuf,iResult.m_nOutBufLen);
            switch (aacData.m_eType)
            {
                case CCodec::AUDIO_CODING_TYPE::eUnSupport:
                    LOG_INFO << "不支持的音频类型!";
                    bSucc = false;
                    break;
                case CCodec::AUDIO_CODING_TYPE::eEncodeError:
                    LOG_INFO << "aac编码失败!";
                    bSucc = false;
                    break;
                case CCodec::AUDIO_CODING_TYPE::eAgain:
                    LOG_DEBUG << "解码数据不够";
                    break;
                default:
                    break;
            }

//            if(bSucc && aacData.m_eType!=CCodec::AUDIO_CODING_TYPE::eAgain)
//            {
//                FILE * fp = fopen("/home/hc/CLionProjects/JT1078Server/pcm2aac.aac","ab+");
//                int tmp = fwrite(aacData.m_pAACBuf,aacData.m_nAACOutBufLen,1,fp);
//                if(tmp<0)
//                {
//                    LOG_INFO<<"tmp<0";
//                    exit(0);
//                }
//                fclose(fp);
//            }
        }


    }
    iCoder.GetData().clear();
    return bSucc;
}


