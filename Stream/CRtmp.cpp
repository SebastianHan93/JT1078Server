//
// Created by hc on 2020/5/29.
//

#include "CRtmp.h"
#include "../../../muduo/build/release-install-cpp11/include/muduo/base/Logging.h"

CRtmp::CRtmp()
{
    m_iRtmp = nullptr;
    m_pPacket = nullptr;

    m_pSPS = nullptr;
    m_pPPS = nullptr;


    m_bIsPushing = false;
    m_bNextIsKey = false;
    m_bWriteAVCSeq = false;
    m_bWriteAACSeq = false;
    std::string m_sUrl = "";


    m_nStartTime = 0;
    m_nNowTime = 0;
    m_nPreFrameTime = 0;
    m_nLastTime = 0;


    m_nSPSSize = 0;
    m_nPPSSize = 0;
}

CRtmp::~CRtmp()
{
    LOG_DEBUG << "CRtmp::~CRtmp()";
    if(m_iRtmp!=nullptr)
    {
        RTMP_Close(m_iRtmp);
        RTMP_Free(m_iRtmp);
        m_iRtmp = nullptr;
    }

    if(m_pPacket!=nullptr)
    {
        free(m_pPacket);
    }

    if(m_pSPS!= nullptr)
    {
        delete[] m_pSPS;
        m_pSPS = nullptr;
    }

    if(m_pPPS!= nullptr)
    {
        delete[] m_pPPS;
        m_pPPS = nullptr;
    }
}

bool CRtmp::Init(std::string sUrl)
{
    m_iRtmp = RTMP_Alloc();
    m_iRtmp->Link.timeout = 30;
    RTMP_Init(m_iRtmp);
    if(sUrl.empty())
    {
        LOG_ERROR << "CRtmp::Init-->m_sUrl为空！";
        return false;
    }
    m_sUrl = sUrl;
    if(!RTMP_SetupURL(m_iRtmp, const_cast<char*>(m_sUrl.data())))
    {
        LOG_ERROR << "CRtmp::Init-->RTMP_SetupURL错误！";
        return false;
    }
    RTMP_EnableWrite(m_iRtmp);
    if(!RTMP_Connect(m_iRtmp, nullptr))
    {
        LOG_ERROR << "CRtmp::Init-->RTMP_Connect 错误！";
        return false;
    }

    if(!RTMP_ConnectStream(m_iRtmp,0))
    {
        LOG_ERROR << "CRtmp::Init-->RTMP_ConnectStream 错误！";
        return false;
    }

//    pPacket= nullptr;
//    pPacket = (RTMPPacket*) malloc(sizeof(RTMPPacket));
//    RTMPPacket_Alloc(pPacket,1024*64);
//    RTMPPacket_Reset(pPacket);
//
//    pPacket->m_hasAbsTimestamp = 0;
//    pPacket->m_nChannel = 0x04;
//    pPacket->m_nInfoField2 = m_iRtmp->m_stream_id;
//    m_nStartTime = RTMP_GetTime();
    m_bIsPushing = true;
    LOG_DEBUG << "CRtmp::Init-->初始化完毕";
    //    m_nNowTime = RTMP_GetTime();
//    if(((m_nNowTime - m_nStartTime)<m_nPreFrameTime) && bNextIsKey)
//    {
//        sleep(1);
//        return true;
//    }
    return true;
}

bool CRtmp::WriteH264(unsigned char *pData, int nDatalen)
{
    int nOffset = 0;
    int nCount = 0;
    int nNaluSize = 0;

    if(!RTMP_IsConnected(m_iRtmp))
    {
        LOG_ERROR << "CRtmp::RTMP_IsConnected-->错误！";
        return false;
    }

    unsigned char * pNalu = nullptr;
    pNalu = new unsigned char[nDatalen];
    m_nStartTime = __GetTickCount();
    while(1)
    {
        if(GetOneNalu(pData+nOffset,nDatalen-nOffset,pNalu,nNaluSize) == 0)
            break;
        int nNaluType = pNalu[4] & 0x1f;

        if(nNaluType == 0x07)
        {
            if(m_pSPS != nullptr)
            {
                delete[] m_pSPS;
                m_pSPS = nullptr;
            }
            m_pSPS = new unsigned char[nNaluSize];
            m_nSPSSize = nNaluSize;
            memcpy(m_pSPS,pNalu,nNaluSize);
        }
        else if(nNaluType == 0x08)
        {
            if(m_pPPS != nullptr)
            {
                delete[] m_pPPS;
                m_pPPS = nullptr;
            }
            m_pPPS = new unsigned char[nNaluSize];
            m_nPPSSize = nNaluSize;
            memcpy(m_pPPS,pNalu,nNaluSize);
            if(__SendVideoSpsPps() == -1)
            {
                return false;
            }
        }
        else
        {
            if(__SendRtmpH264(pNalu,nNaluSize) == -1)
            {
                return false;
            }
        }
//        if(m_pSPS != nullptr && m_pPPS != nullptr && nNaluType == 0x07 && !m_bWriteAVCSeq)
//        {
//            __SendVideoSpsPps();
//            m_bWriteAVCSeq = true;
//        }

//        if(!m_bWriteAVCSeq)
//        {
//            continue;
//        }
        nOffset += nNaluSize;
        nCount++;
    }

    return true;
}

int CRtmp::__SendVideoSpsPps()
{
    unsigned char * pBody;
    int i;

    m_pPacket = (RTMPPacket *)malloc(RTMP_HEAD_SIZE+1024);
    memset(m_pPacket,0,RTMP_HEAD_SIZE);

    m_pPacket->m_body = (char *)m_pPacket + RTMP_HEAD_SIZE;
    pBody = (unsigned char *)m_pPacket->m_body;

//    memcpy(winsys->pps,buf,len);
//    winsys->pps_len = len;
    i = 0;
    pBody[i++] = 0x17;
    pBody[i++] = 0x00;

    pBody[i++] = 0x00;
    pBody[i++] = 0x00;
    pBody[i++] = 0x00;

    /*AVCDecoderConfigurationRecord*/
    pBody[i++] = 0x01;
    pBody[i++] = m_pSPS[1];
    pBody[i++] = m_pSPS[2];
    pBody[i++] = m_pSPS[3];
    pBody[i++] = 0xff;

    /*sps*/
    pBody[i++]   = 0xe1;
    pBody[i++] = (m_nSPSSize >> 8) & 0xff;
    pBody[i++] = m_nSPSSize & 0xff;
    memcpy(&pBody[i],m_pSPS,m_nSPSSize);
    i +=  m_nSPSSize;

    /*pps*/
    pBody[i++]   = 0x01;
    pBody[i++] = (m_nPPSSize >> 8) & 0xff;
    pBody[i++] = (m_nPPSSize) & 0xff;
    memcpy(&pBody[i],m_pPPS,m_nPPSSize);
    i +=  m_nPPSSize;

    m_pPacket->m_packetType = RTMP_PACKET_TYPE_VIDEO;
    m_pPacket->m_nBodySize = i;
    m_pPacket->m_nChannel = 0x04;
    m_pPacket->m_nTimeStamp = 0;
    m_pPacket->m_hasAbsTimestamp = 0;
    m_pPacket->m_headerType = RTMP_PACKET_SIZE_MEDIUM;
    m_pPacket->m_nInfoField2 = m_iRtmp->m_stream_id;

    /*调用发送接口*/
    if(!RTMP_SendPacket(m_iRtmp,m_pPacket,TRUE))
    {
        free(m_pPacket);
        LOG_ERROR << "CRtmp::__SendRtmpH264-->RTMP_SendPacket 出错！";
        return -1;
    }
    free(m_pPacket);
    m_pPacket = nullptr;
    return 0;
}

int CRtmp::__SendRtmpH264(unsigned char *pData, int nDatalen)
{
    int nType;
    long nTimeOffset;
    unsigned char * pBody;
    /*start_time为开始直播时的时间戳*/
    nTimeOffset = __GetTickCount() - m_nStartTime;

    /*去掉帧界定符*/
    if (pData[2] == 0x00) { /*00 00 00 01*/
        pData += 4;
        nDatalen -= 4;
    } else if (pData[2] == 0x01){ /*00 00 01*/
        pData += 3;
        nDatalen -= 3;
    }
    nType = pData[0]&0x1f;

    m_pPacket = (RTMPPacket *)malloc(RTMP_HEAD_SIZE+nDatalen+9);
    memset(m_pPacket,0,RTMP_HEAD_SIZE);

    m_pPacket->m_body = (char *)m_pPacket + RTMP_HEAD_SIZE;
    m_pPacket->m_nBodySize = nDatalen + 9;

    /*send video packet*/
    pBody = (unsigned char *)m_pPacket->m_body;
    memset(pBody,0,nDatalen+9);

    /*key frame*/
    pBody[0] = 0x27;
    if (nType == NAL_SLICE_IDR) {
        pBody[0] = 0x17;
    }

    pBody[1] = 0x01;   /*nal unit*/
    pBody[2] = 0x00;
    pBody[3] = 0x00;
    pBody[4] = 0x00;

    pBody[5] = (nDatalen >> 24) & 0xff;
    pBody[6] = (nDatalen >> 16) & 0xff;
    pBody[7] = (nDatalen >>  8) & 0xff;
    pBody[8] = (nDatalen ) & 0xff;

    /*copy data*/
    memcpy(&pBody[9],pData,nDatalen);

    m_pPacket->m_hasAbsTimestamp = 0;
    m_pPacket->m_packetType = RTMP_PACKET_TYPE_VIDEO;
    m_pPacket->m_nInfoField2 = m_iRtmp->m_stream_id;
    m_pPacket->m_nChannel = 0x04;
    m_pPacket->m_headerType = RTMP_PACKET_SIZE_LARGE;
    m_pPacket->m_nTimeStamp = nTimeOffset;

    /*调用发送接口*/
    if(!RTMP_SendPacket(m_iRtmp,m_pPacket,TRUE))
    {
        free(m_pPacket);
        LOG_ERROR << "CRtmp::__SendRtmpH264-->RTMP_SendPacket 出错！";
        return -1;
    }

    free(m_pPacket);
    m_pPacket = nullptr;
    return 0;
}

bool CRtmp::GetPushState() const
{
    return m_bIsPushing;
}

std::string CRtmp::GetUrl() const
{
    return m_sUrl;
}

int CRtmp::GetOneNalu(unsigned char *pBufIn, int nInSize, unsigned char *pNalu, int &nNaluSize)
{
    unsigned char *p = pBufIn;
    int nStartPos = 0, nEndPos = 0;
    while (1)
    {
        if (p[0] == 0 && p[1] == 0 && p[2] == 0 && p[3] == 1)
        {
            nStartPos = p - pBufIn;
            break;
        }
        p++;
        if (p - pBufIn >= nInSize - 4)
            return 0;
    }
    p++;
    while (1)
    {
        if (p[0] == 0 && p[1] == 0 && p[2] == 0 && p[3] == 1)
        {
            nEndPos = p - pBufIn;
            break;
        }
        p++;
        if (p - pBufIn >= nInSize - 4)
        {
            nEndPos = nInSize;
            break;
        }
    }
    nNaluSize = nEndPos - nStartPos;
    memcpy(pNalu, pBufIn + nStartPos, nNaluSize);

    return 1;

}

int CRtmp::IsVideojjSEI(unsigned char *pNalu, int nNaluSize)
{
    unsigned char *p = pNalu;

    if (p[3] != 1 || p[4] != 6 || p[5] != 5)
        return 0;
    p += 6;
    while (*p++==0xff) ;
    const char *szVideojjUUID = "VideojjLeonUUID";
    char *pp = (char *)p;
    for (int i = 0; i < strlen(szVideojjUUID); i++)
    {
        if (pp[i] != szVideojjUUID[i])
            return 0;
    }

    return 1;
}

int CRtmp::GetOneAACFrame(unsigned char *pBufIn, int nInSize, unsigned char *pAACFrame, int &nAACFrameSize)
{
    unsigned char *p = pBufIn;

    if (nInSize <= 7)
        return 0;

    int nFrameSize = ((p[3] & 0x3) << 11) + (p[4] << 3) + (p[5] >> 5);
    if (nInSize < nFrameSize)
        return 0;

    nAACFrameSize = nFrameSize;
    memcpy(pAACFrame, pBufIn, nFrameSize);

    return 1;
}

unsigned long CRtmp::__GetTickCount()
{
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return (ts.tv_sec * 1000 + ts.tv_nsec / 1000000);
}



