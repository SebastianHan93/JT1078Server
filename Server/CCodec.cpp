//
// Created by hc on 2020/5/14.
//

#include <muduo/base/Logging.h>
#include "CCodec.h"
#include "../Codec/G711.h"
#include "../Codec/G726.h"
#include "../Codec/Adpcm.h"


CCodec::CCodec() {
    m_iResult.m_nOutBufLen = 0;
    m_iResult.m_pOutBuf = (char *) new char[2048];
    m_iResult.m_nBufLen = 2048;
    m_iResult.m_eType = AUDIO_CODING_TYPE::eUnSupport;

    m_pCachePcmData = (char *) new char[10240];
    m_nCachePcmDataLen = 0;
    m_nCachePcmDataBufferSize = 10240;
}

bool CCodec::InitAACEncoder(unsigned long nSampleRate, unsigned int nChannels)
{
    m_nInputSamples = 0;
    m_nMaxOutputBytes = 0;
    m_nPCMBitSize = 16;
    m_hEncoder = nullptr;
    m_nChannels = nChannels;//1;
    m_nSampleRate = nSampleRate;//11025;
    m_hEncoder = faacEncOpen(m_nSampleRate,m_nChannels,&m_nInputSamples,&m_nMaxOutputBytes);
    if(!m_hEncoder)
    {
        LOG_ERROR << "CCodec::Pcm2AAC-->faacEncOpen出错";
        return false;
    }
    unsigned long nPCMBufferSize = m_nInputSamples * m_nPCMBitSize / 8;

    m_iAACData.m_nAACOutBufLen = 0;
    m_iAACData.m_pAACBuf = (char *) new char[m_nMaxOutputBytes];
    m_iAACData.m_pPCMBuffer = (char *) new char[nPCMBufferSize];
    m_iAACData.m_nPCMBufferSize = nPCMBufferSize;
    m_iAACData.m_nAACBufSize = m_nMaxOutputBytes;
    m_iAACData.m_eType = AUDIO_CODING_TYPE ::eEncodeError;
    LOG_DEBUG << "m_iAACData.m_nBufLen:"<<m_iAACData.m_nAACBufSize;


    m_pConfiguration = faacEncGetCurrentConfiguration(m_hEncoder);
    m_pConfiguration->aacObjectType = LOW;              //编码类型
    m_pConfiguration->mpegVersion = MPEG4;              //设置版本,录制MP4文件时要用MPEG4
    m_pConfiguration->useTns = 0;                       //瞬时噪声定形(temporal noise shaping，TNS)滤波器
    m_pConfiguration->allowMidside = 1;                 //M/S编码
    m_pConfiguration->shortctl = SHORTCTL_NORMAL;
    m_pConfiguration->outputFormat = 0;                 //录制MP4文件时，要用raw流; 1是ADTS
    m_pConfiguration->inputFormat = FAAC_INPUT_16BIT;   //输入数据类型
    int nRet = faacEncSetConfiguration(m_hEncoder,m_pConfiguration);
    if(!nRet)
    {
        LOG_ERROR << "CCodec::InitAACEncoder-->faacEncSetConfiguration设置失败";
        return false;
    }

    return true;
}

CCodec::~CCodec()
{
    if (m_iResult.m_pOutBuf != nullptr && m_iResult.m_nOutBufLen != 0) {
        delete[]m_iResult.m_pOutBuf;
        m_iResult.m_nOutBufLen = 0;
        m_iResult.m_nBufLen = 0;
    }
    if(m_iAACData.m_pAACBuf != nullptr && m_iAACData.m_nAACBufSize!=0)
    {
        delete[]m_iAACData.m_pAACBuf;
        m_iAACData.m_nAACBufSize = 0;
        m_iAACData.m_nAACOutBufLen = 0;
    }
    if(m_iAACData.m_pPCMBuffer != nullptr && m_iAACData.m_nPCMBufferSize!=0)
    {
        delete[]m_iAACData.m_pPCMBuffer;
        m_iAACData.m_nPCMBufferSize = 0;
    }

    if(m_hEncoder!= nullptr)
    {
        faacEncClose(m_hEncoder);
        m_hEncoder = nullptr;
    }
}

void CCodec::__DecodeAdpcm2Pcm(char *cInBuf, int nInBufLen)
{
    adpcm_state iAdpcmState{0};
    iAdpcmState.valprev = (cInBuf[5] << 8) | (cInBuf[4] & 0xFF);
    iAdpcmState.index = cInBuf[6];
    nInBufLen -= 8;
    cInBuf += 8;
    adpcm_decoder((char *) cInBuf, (short *) m_iResult.m_pOutBuf, nInBufLen * 2, &iAdpcmState);
    m_iResult.m_nOutBufLen = 4 * nInBufLen;
    m_iResult.m_eType = AUDIO_CODING_TYPE::eAdpcm;
}

void CCodec::__DecodeG711A2Pcm(char *cInBuf, int nInBufLen)
{

    int nRet = g711a_decode((short *) m_iResult.m_pOutBuf, (unsigned char *) cInBuf, nInBufLen);
    m_iResult.m_nOutBufLen = nRet;
    m_iResult.m_eType = AUDIO_CODING_TYPE::eG711A;
}

void CCodec::__DecodeG711U2Pcm(char *cInBuf, int nInBufLen)
{
    int nRet = g711u_decode((short *) m_iResult.m_pOutBuf, (unsigned char *) cInBuf, nInBufLen);
    m_iResult.m_nOutBufLen = nRet;
    m_iResult.m_eType = AUDIO_CODING_TYPE::eG711U;
}

DECODE_RESULT &CCodec::DecodeAudio(char *pInBuf, int nInBufLen, AUDIO_CODING_TYPE eType) {
    switch (eType) {
        case AUDIO_CODING_TYPE::eG711A:
            LOG_DEBUG << "eG711A";
            __DecodeG711A2Pcm(pInBuf, nInBufLen);
            break;
        case AUDIO_CODING_TYPE::eG711U:
            LOG_DEBUG << "eG711U";
            __DecodeG711U2Pcm(pInBuf, nInBufLen);
            break;
        case AUDIO_CODING_TYPE::eAdpcm:
            LOG_DEBUG << "eAdpcm";
            __DecodeAdpcm2Pcm(pInBuf, nInBufLen);
            break;
        case AUDIO_CODING_TYPE::eAAC:
            LOG_DEBUG << "eAAC";
            m_iResult.m_eType = AUDIO_CODING_TYPE::eAAC;
            m_iResult.m_nOutBufLen = nInBufLen;
            memcpy(m_iResult.m_pOutBuf,pInBuf,nInBufLen);
            break;
        default:
            m_iResult.m_eType = AUDIO_CODING_TYPE::eUnSupport;
            break;
    }
    return m_iResult;
}
/**
 *
 * @param nSampleRate :编码采样率
 * @param nPCMBitSize :pcm位深，用于计算一帧pcm大小
 * @param nChannels ：声道数
 */
AAC_DATA & CCodec::Pcm2AAC(unsigned char* pbPCMBuffer,int nInBufLen)
{
    int nRet;

    unsigned long nPcmLen = nInBufLen + m_nCachePcmDataLen;
    char gTempCache[nInBufLen];
    memcpy(gTempCache,pbPCMBuffer,nInBufLen);
    memcpy(m_pCachePcmData+m_nCachePcmDataLen,gTempCache,nInBufLen);
    m_nCachePcmDataLen += nInBufLen;

//    LOG_DEBUG << "2m_sCachePcmData.size():" << m_sCachePcmData.size();
    if(nPcmLen<m_iAACData.m_nPCMBufferSize)
    {
        m_iAACData.m_eType = eAgain;
        assert(m_nCachePcmDataLen<m_iAACData.m_nPCMBufferSize);
        m_iAACData.m_nAACOutBufLen = 0;
        return m_iAACData;
    }
    else if(nPcmLen==m_iAACData.m_nPCMBufferSize)
    {
        assert(m_nCachePcmDataLen==m_iAACData.m_nPCMBufferSize);
        memcpy(m_iAACData.m_pPCMBuffer,m_pCachePcmData,m_iAACData.m_nPCMBufferSize);
        m_nCachePcmDataLen = 0;
    }
    else
    {
        assert(m_nCachePcmDataLen > m_iAACData.m_nPCMBufferSize);
        memcpy(m_iAACData.m_pPCMBuffer,m_pCachePcmData,m_iAACData.m_nPCMBufferSize);
        m_nCachePcmDataLen -=  m_iAACData.m_nPCMBufferSize;
        char gTemp[m_nCachePcmDataLen];
        memcpy(gTemp,m_pCachePcmData+m_iAACData.m_nPCMBufferSize,m_nCachePcmDataLen);
        memcpy(m_pCachePcmData,gTemp,m_nCachePcmDataLen);
    }

    unsigned long nInputSamples = m_iAACData.m_nPCMBufferSize /(m_nPCMBitSize/8);

    LOG_DEBUG << "AAC编码开始";
    nRet = faacEncEncode(m_hEncoder,(int32_t *)m_iAACData.m_pPCMBuffer,nInputSamples,(unsigned char*)m_iAACData.m_pAACBuf,m_nMaxOutputBytes);
    while (nRet == 0)
    {
        LOG_DEBUG << "AAC编码中";
        nRet = faacEncEncode(m_hEncoder,(int32_t *)m_iAACData.m_pPCMBuffer,nInputSamples,(unsigned char*)m_iAACData.m_pAACBuf,m_nMaxOutputBytes);
    }
    if(nRet>0)
    {
        LOG_DEBUG << "AAC编码成功";
        m_iAACData.m_nAACOutBufLen = nRet;
    }
    else
    {
        LOG_DEBUG << "AAC编码失败";
        m_iAACData.m_eType = eEncodeError;
        m_iAACData.m_nAACOutBufLen = 0;
        return m_iAACData;
    }
    m_iAACData.m_eType = eAAC;
    return m_iAACData;
}
