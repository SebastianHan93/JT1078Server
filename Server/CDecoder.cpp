//
// Created by hc on 2020/5/13.
//

#include "CDecoder.h"
#include <muduo/net/Buffer.h>
#include <muduo/base/Timestamp.h>
#include <muduo/base/Logging.h>
#include <iostream>

using namespace muduo;
using namespace muduo::net;

const int CDecoder::sm_nFirstReceiveBytes = 16;

CDecoder::CDecoder()
    : m_sData(),
      m_iRecvLen(sm_nFirstReceiveBytes),
      m_eCurrentStat(eInit),
      m_eSubMarke(eFirst),
      m_eDataType(eVideoI),
      m_eSkip(eSkipNon),
      m_eErr(eNoError),
      m_iHeader(),
//      m_pRtmpStream(new CRtmpStream()),
      m_eCodingType(eH264),
      m_pCodec(new CCodec()),
      m_spRtmp(new CRtmp())
{

}

CDecoder::~CDecoder()
{
//    delete m_pRtmpStream;
    delete m_pCodec;
}

bool CDecoder::Decode(muduo::net::Buffer* pBuf, muduo::Timestamp iReceiveTime)
{
    bool b;
    LOG_DEBUG << "pBuf->readableBytes" <<pBuf->readableBytes() ;
    while(pBuf->readableBytes()>m_iRecvLen)
    {
        if(m_eSkip == eSkipNon || m_eSkip == eSkipPaser16)
        {
            if(m_eCurrentStat == eInit)
            {
                if(!m_sData.empty())
                {
                    m_sData.clear();
                }
            }
            if(pBuf->readableBytes()>m_iRecvLen)
            {
                b = DecodeHeader(pBuf,iReceiveTime);
                if(!b || m_eErr!=eNoError)
                {
                    return false;
                }
            }
        }
        if(m_eSkip == eSkipPaserTheRestData)
        {
//            DumpToHex(m_iHeader);
            if(pBuf->readableBytes()>m_iRecvLen)
            {
                DecodeBody(pBuf,iReceiveTime);
                if(m_eCurrentStat == eCompleted)
                {
                    //doDecode
                    LOG_DEBUG << "收到完整包!";
                    return true;
                }
            }

        }

    }
    return true;
}

void CDecoder::DecodeBody(muduo::net::Buffer* pBuf, muduo::Timestamp iReceiveTime)
{
    assert(m_eErr == eNoError);
    assert(pBuf->readableBytes() >= m_iRecvLen);
    m_sData += pBuf->retrieveAsString(m_iRecvLen);
    if(m_eSubMarke == eAtomic || m_eSubMarke == eLast)
    {
        m_eCurrentStat = eCompleted;
        m_eSkip = eSkipNon;
        m_iRecvLen = sm_nFirstReceiveBytes;
    }
    else
    {
        m_eSkip = eSkipNon;
        m_iRecvLen = sm_nFirstReceiveBytes;
    }
}

bool CDecoder::DecodeHeader(muduo::net::Buffer* pBuf, muduo::Timestamp iReceiveTime)
{
    assert(m_eErr == eNoError);
    assert(pBuf->readableBytes() >= m_iRecvLen);
    bool b;
    b = __ParserHeader(pBuf,iReceiveTime);
    return b;
}

bool CDecoder::__ParserHeader(muduo::net::Buffer* pBuf, muduo::Timestamp iReceiveTime)
{
    assert(m_eErr == eNoError);
    assert(m_eSkip == eSkipNon || m_eSkip == eSkipPaser16);
    bool b;
    if(m_eSkip != eSkipPaser16 )
    {
        b = __Paser16(pBuf,iReceiveTime);
        if(!b)
            return false;
        else
        {
            if(pBuf->readableBytes()>m_iRecvLen)
            {
                b = __PaserTheRestData(pBuf,iReceiveTime);
                if(!b)
                {
                    return false;
                }
                else
                {
                    m_eSkip = eSkipPaserTheRestData;
                    return true;
                }
            }
            else
            {
                m_eSkip = eSkipPaser16;
                return true;
            }

        }
    }
    else
    {
        b = __PaserTheRestData(pBuf,iReceiveTime);
        if(!b)
        {
            return false;
        }
        else
        {
            m_eSkip = eSkipPaserTheRestData;
            return true;
        }
    }


}

bool CDecoder::__Paser16(muduo::net::Buffer* pBuf, muduo::Timestamp iReceiveTime)
{
    m_iHeader.DWFramHeadMark = pBuf->readInt32();
    if(m_iHeader.DWFramHeadMark!=0x30316364)
    {
        LOG_DEBUG << "m_iHeader.DWFramHeadMark" << m_iHeader.DWFramHeadMark;
        return false;
    }
    int nInt8;
    int nInt16;
    nInt8 = pBuf->readInt8();
    m_iHeader.V2 = (nInt8>>6) & 0x03;
    m_iHeader.P1 = (nInt8>>5) & 0x01;
    m_iHeader.X1 = (nInt8>>4) & 0x01;
    m_iHeader.CC4 = nInt8 & 0x0F;

    nInt8 = pBuf->readInt8();
    m_iHeader.M1 = (nInt8>>7) & 0x01;
    m_iHeader.PT7 = nInt8 & 0x7F;

    __SetAVCodingType();
    if(m_eCodingType == eUnSupport)
    {
        LOG_DEBUG << "在__SetAVCodingType中解析音视频编码错误-->m_iHeader.PT7"<<"["<<m_iHeader.PT7<<"]";
        m_eErr = eCodingTypeError;
        return false;
    }
    m_iHeader.WdPackageSequence = pBuf->readInt16();

    m_iHeader.BCDSIMCardNumber[0] = pBuf->readInt8();
    m_iHeader.BCDSIMCardNumber[1] = pBuf->readInt8();
    m_iHeader.BCDSIMCardNumber[2] = pBuf->readInt8();
    m_iHeader.BCDSIMCardNumber[3] = pBuf->readInt8();
    m_iHeader.BCDSIMCardNumber[4] = pBuf->readInt8();
    m_iHeader.BCDSIMCardNumber[5] = pBuf->readInt8();
    m_iHeader.Bt1LogicChannelNumber = pBuf->readInt8();

    nInt8 = pBuf->readInt8();
    m_iHeader.DataType4 = (nInt8 >> 4) & 0x0F;
    m_iHeader.SubpackageHandleMark4 = nInt8 & 0x0F;
    int nLen = __GetTailLenAndSetDataType();
    if(nLen==-1)
    {
        LOG_DEBUG << "在__GetTailLen中解析数据类型错误-->m_iHeader.DataType4"<<"["<<m_iHeader.DataType4<<"]";
        m_eErr = eTypeError;
        return false;
    }

    __CheckSubMark();
    if(m_eErr == eMarkeError)
    {
        LOG_DEBUG << "在__CheckSubMark中解析分包处理标记错误-->m_iHeader.SubpackageHandleMark4"<<"["<<m_iHeader.SubpackageHandleMark4<<"]";
        return false;
    }
    if(m_eSubMarke == eAtomic || m_eSubMarke==eLast)
    {
        LOG_DEBUG << "eAtomic Or eLast";
    }

    m_iRecvLen = nLen;
    m_eCurrentStat = eReceiving;
    return true;
}

bool CDecoder::__PaserTheRestData(muduo::net::Buffer* pBuf, muduo::Timestamp iReceiveTime)
{
    assert(pBuf->readableBytes() >= m_iRecvLen);
    if(m_eDataType == eVideoI || m_eDataType == eVideoP || m_eDataType == eVideoB)
    {
        m_iHeader.Bt8timeStamp = pBuf->readInt64();
        m_iHeader.WdLastIFrameInterval = pBuf->readInt16();
        m_iHeader.WdLastFrameInterval = pBuf->readInt16();
    }
    else if (m_eDataType == eAudio)
    {
        m_iHeader.Bt8timeStamp = pBuf->readInt64();
    }
    else
    {
        LOG_INFO << "不支持的数据类型";
        m_eErr = eTypeError;
        return false;
    }
    m_iHeader.WdBodyLen = pBuf->readInt16();
    if(m_iHeader.WdBodyLen > JT1078_MAX_LENGTH)
    {
        LOG_INFO << "在__PaserTheRestData中解析出后续包长度大于JT1078_MAX_LENGTH";
        m_eErr = eLengthError;
        return false;
    }
    m_iRecvLen = m_iHeader.WdBodyLen;
    return true;
}

int CDecoder::__GetTailLenAndSetDataType()
{
    int nRemaining = HEAD_BUFSIZE - sm_nFirstReceiveBytes;
    switch (m_iHeader.DataType4)
    {
        case 0x00://I
            m_eDataType = eVideoI;
            break;
        case 0x01://P
            m_eDataType = eVideoP;
            break;
        case 0x02://B
            m_eDataType = eVideoB;
            break;
        case 0x03://音频
            m_eDataType = eAudio;
            nRemaining = nRemaining - 2 - 2;
            break;
        case 0x04://透传
            m_eDataType = ePassthrough;
            nRemaining = nRemaining - 8 - 2 - 2;
            break;
        default:
            m_eErr = eTypeError;
            return -1;
    }
    return nRemaining;
}

void CDecoder::__CheckSubMark()
{
    switch (m_iHeader.SubpackageHandleMark4)
    {
        case 0x00:
            m_eSubMarke = eAtomic;break;
        case 0x01:
            m_eSubMarke = eFirst;break;
        case 0x02:
            m_eSubMarke = eLast;break;
        case 0x03:
            m_eSubMarke = eIntermediate;break;
        default:
            m_eErr = eMarkeError;break;
    }
}

CDecoder::CURRENT_RECEIVE_STATE CDecoder::GetCurReceiveStat() const
{
    return m_eCurrentStat;
}

CDecoder::SUBCONTRACT_PROCESSING_MARKE CDecoder::GetProcessingMarke() const
{
    return m_eSubMarke;
}

CDecoder::DATA_TYPE CDecoder::GetDataType() const
{
    return m_eDataType;
}

CDecoder::ERRS CDecoder::GetErr() const
{
    return m_eErr;
}

JT_1078_HEADER & CDecoder::GetHeader()
{
    return m_iHeader;
}

std::string & CDecoder::GetData()
{
    return m_sData;
}


void CDecoder::DumpToHex(const std::string & sStr) const
{
    std::cout << sStr << std::hex << std::endl;
}

void CDecoder::DumpToHex(const JT_1078_HEADER &m_iHeader) const
{
    printf("----------------after-----------------\n");
    printf("DWFramHeadMark = %X\n",m_iHeader.DWFramHeadMark);
    printf("V2 = %X\n",m_iHeader.V2);
    printf("P1 = %X\n",m_iHeader.P1);
    printf("X1 = %X\n",m_iHeader.X1);
    printf("CC4 = %X\n",m_iHeader.CC4);
    printf("M1 = %X\n",m_iHeader.M1);
    printf("PT7 = %X\n",m_iHeader.PT7);
    printf("WdPackageSequence = %X\n",m_iHeader.WdPackageSequence);
    printf("BCDSIMCardNumber0 = %X\n",m_iHeader.BCDSIMCardNumber[0]);
    printf("BCDSIMCardNumber1 = %X\n",m_iHeader.BCDSIMCardNumber[1]);
    printf("BCDSIMCardNumber2 = %X\n",m_iHeader.BCDSIMCardNumber[2]);
    printf("BCDSIMCardNumber3 = %X\n",m_iHeader.BCDSIMCardNumber[3]);
    printf("BCDSIMCardNumber4 = %X\n",m_iHeader.BCDSIMCardNumber[4]);
    printf("BCDSIMCardNumber5 = %X\n",m_iHeader.BCDSIMCardNumber[5]);
    printf("Bt1LogicChannelNumber = %X\n",m_iHeader.Bt1LogicChannelNumber);
    printf("DataType4 = %X\n",m_iHeader.DataType4);
    printf("subpackageHandleMark4 = %X\n",m_iHeader.SubpackageHandleMark4);
    printf("Bt8timeStamp = %lX\n",m_iHeader.Bt8timeStamp);
    printf("WdLastIFrameInterval = %X\n",m_iHeader.WdLastIFrameInterval);
    printf("WdLastFrameInterval = %X\n",m_iHeader.WdLastFrameInterval);
    printf("WdBodyLen = %X\n",m_iHeader.WdBodyLen);
    printf("----------------after-----------------\n");
}

bool CDecoder::Init(std::string& sUrl)
{
//    int nRet = m_pRtmpStream->Init(sUrl.c_str());
    return m_spRtmp->Init(sUrl.c_str());
}
//
//bool CDecoder::WriteData(AVMediaType iDataType, char *pData, int nDataLen)
//{
//    int nRet = m_pRtmpStream->WriteData(iDataType,pData,nDataLen);
//    return nRet >= 0;
//}

bool CDecoder::WriteH264(unsigned char *pData, int nDatalen,uint64_t nTimestamp)
{
    return m_spRtmp->WriteH264(pData,nDatalen,nTimestamp);
}

bool CDecoder::GetPushState() const
{
//    return m_pRtmpStream->GetPushState();
    return m_spRtmp->GetPushState();
}

void CDecoder::SetCurReceiveStat(CURRENT_RECEIVE_STATE eState)
{
    m_eCurrentStat = eState;
}

std::string CDecoder::GetUrl() const
{
//    return m_pRtmpStream->GetUrl();
    return m_spRtmp->GetUrl();
}

CDecoder::AV_CODING_TYPE CDecoder::GetAVCodingType() const
{
    return m_eCodingType;
}

void CDecoder::__SetAVCodingType()
{
    switch (m_iHeader.PT7)
    {
        case 6:
            m_eCodingType = eG711A;
            break;
        case 7:
            m_eCodingType = eG711U;
            break;
        case 19:
            m_eCodingType = eAAC;
            break;
        case 26:
            m_eCodingType = eAdpcm;
            break;
        case 98:
            m_eCodingType = eH264;
            break;
        default:
            m_eCodingType = eUnSupport;
            break;
    }
}

DECODE_RESULT &CDecoder::DecodeAudio2PCM(char *pInBuf, int nInBufLen, AV_CODING_TYPE eType) {
    AUDIO_CODING_TYPE eAudioType;
    switch (eType) {
        case eG711A:
            eAudioType = AUDIO_CODING_TYPE::eG711A;
            break;
        case eG711U:
            eAudioType = AUDIO_CODING_TYPE::eG711U;
            break;
        case eAdpcm:
            eAudioType = AUDIO_CODING_TYPE::eAdpcm;
            break;
        case eAAC:
            eAudioType = AUDIO_CODING_TYPE::eAAC;
            break;
        default:
            eAudioType = AUDIO_CODING_TYPE::eUnSupport;
            break;
    }
    DECODE_RESULT &iResult = m_pCodec->DecodeAudio(pInBuf, nInBufLen, eAudioType);
    return iResult;
}

AAC_DATA & CDecoder::Pcm2AAC(unsigned char* pbPCMBuffer,int nInBufLen)
{
    return m_pCodec->Pcm2AAC(pbPCMBuffer,nInBufLen);
}

bool CDecoder::InitAACEncoder(unsigned long nSampleRate, unsigned int nChannels)
{
    return m_pCodec->InitAACEncoder(nSampleRate,nChannels);
}