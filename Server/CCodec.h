//
// Created by hc on 2020/5/14.
//

#ifndef JT1078SERVER_CCODEC_H
#define JT1078SERVER_CCODEC_H
#include <iostream>
#include <memory>

class CCodec {
public:
    CCodec();

    ~CCodec();

public:
    enum AUDIO_CODING_TYPE {
        eG711A,
        eG711U,
        eAdpcm,
        eUnSupport,
    };
    typedef struct {
        char *m_pOutBuf;
        int m_nOutBufLen;
        AUDIO_CODING_TYPE m_eType;
    } DECODE_RESULT;

    DECODE_RESULT &DecodeAudio(char *pInBuf, int nInBufLen, AUDIO_CODING_TYPE eType);

public:
    DECODE_RESULT m_iResult;

private:
    void __DecodeG711A2Pcm(char *pInBuf, int nInBufLen);

    void __DecodeG711U2Pcm(char *pInBuf, int nInBufLen);

    void __DecodeAdpcm2Pcm(char *pInBuf, int nInBufLen);
};

typedef CCodec::DECODE_RESULT DECODE_RESULT;
typedef CCodec::AUDIO_CODING_TYPE AUDIO_CODING_TYPE;
#endif //JT1078SERVER_CCODEC_H
