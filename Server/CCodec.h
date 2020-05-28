//
// Created by hc on 2020/5/14.
//

#ifndef JT1078SERVER_CCODEC_H
#define JT1078SERVER_CCODEC_H
#include <iostream>
#include <memory>
#include <faac.h>

class CCodec {
public:
    CCodec();

    ~CCodec();

public:
    enum AUDIO_CODING_TYPE {
        eG711A,
        eG711U,
        eAdpcm,
        eAAC,
        eAgain,
        eUnSupport,
        eEncodeError,
    };
    typedef struct {
        char *m_pOutBuf;
        int m_nBufLen;
        int m_nOutBufLen;
        AUDIO_CODING_TYPE m_eType;
    } DECODE_RESULT;
    typedef struct {
        char * m_pAACBuf;
        char * m_pPCMBuffer;
        unsigned long m_nAACOutBufLen;
        unsigned long m_nPCMBufferSize;
        unsigned long m_nAACBufSize;
        AUDIO_CODING_TYPE m_eType;
    } AAC_DATA;
    DECODE_RESULT &DecodeAudio(char *pInBuf, int nInBufLen, AUDIO_CODING_TYPE eType);
    //unsigned long nSampleRate,unsigned int nPCMBitSize,unsigned int nChannels,
    AAC_DATA & Pcm2AAC(unsigned char* pbPCMBuffer,int nInBufLen);
    bool InitAACEncoder(unsigned long nSampleRate, unsigned int nChannels);

private:
    void __DecodeG711A2Pcm(char *pInBuf, int nInBufLen);

    void __DecodeG711U2Pcm(char *pInBuf, int nInBufLen);

    void __DecodeAdpcm2Pcm(char *pInBuf, int nInBufLen);

private:
    unsigned int m_nChannels;
    unsigned int m_nPCMBitSize;
    unsigned long m_nSampleRate;
    DECODE_RESULT m_iResult;
    AAC_DATA m_iAACData;
    //faac编码器句柄
    faacEncHandle m_hEncoder;
    //faac设置类
    faacEncConfigurationPtr m_pConfiguration;
    int m_nBytesRead;
    char * m_pCachePcmData;
    unsigned long m_nCachePcmDataLen;
    unsigned long m_nCachePcmDataBufferSize;
    unsigned long m_nInputSamples;
    unsigned long m_nMaxOutputBytes;
};

typedef CCodec::DECODE_RESULT DECODE_RESULT;
typedef CCodec::AAC_DATA AAC_DATA;
typedef CCodec::AUDIO_CODING_TYPE AUDIO_CODING_TYPE;
#endif //JT1078SERVER_CCODEC_H
