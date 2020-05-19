//
// Created by hc on 2020/5/14.
//

#include <muduo/base/Logging.h>
#include "CCodec.h"
#include "../Codec/G711.h"
#include "../Codec/G726.h"
#include "../Codec/Adpcm.h"

CCodec::CCodec() {
    m_iResult.m_nOutBufLen = 2048;
    m_iResult.m_pOutBuf = (char *) new char[2048];
    m_iResult.m_eType = AUDIO_CODING_TYPE::eUnSupport;
}

CCodec::~CCodec() {
    if (m_iResult.m_pOutBuf != nullptr && m_iResult.m_nOutBufLen != 0) {
        delete[]m_iResult.m_pOutBuf;
        m_iResult.m_nOutBufLen = 0;
    }
}

void CCodec::__DecodeAdpcm2Pcm(char *cInBuf, int nInBufLen) {
    adpcm_state iAdpcmState{0};
    iAdpcmState.valprev = (cInBuf[5] << 8) | (cInBuf[4] & 0xFF);
    iAdpcmState.index = cInBuf[6];
    nInBufLen -= 8;
    cInBuf += 8;
    adpcm_decoder((char *) cInBuf, (short *) m_iResult.m_pOutBuf, nInBufLen * 2, &iAdpcmState);
    m_iResult.m_nOutBufLen = 4 * nInBufLen;
    m_iResult.m_eType = AUDIO_CODING_TYPE::eAdpcm;
}

void CCodec::__DecodeG711A2Pcm(char *cInBuf, int nInBufLen) {
    int nRet = g711a_decode((short *) m_iResult.m_pOutBuf, (unsigned char *) cInBuf, nInBufLen);
    m_iResult.m_nOutBufLen = nRet;
    m_iResult.m_eType = AUDIO_CODING_TYPE::eG711A;
}

void CCodec::__DecodeG711U2Pcm(char *cInBuf, int nInBufLen) {
    int nRet = g711u_decode((short *) m_iResult.m_pOutBuf, (unsigned char *) cInBuf, nInBufLen);
    m_iResult.m_nOutBufLen = nRet;
    m_iResult.m_eType = AUDIO_CODING_TYPE::eG711U;
}

DECODE_RESULT &CCodec::DecodeAudio(char *pInBuf, int nInBufLen, AUDIO_CODING_TYPE eType) {
    switch (eType) {
        case AUDIO_CODING_TYPE::eG711A:
            LOG_INFO << "eG711A";
            __DecodeG711A2Pcm(pInBuf, nInBufLen);
            break;
        case AUDIO_CODING_TYPE::eG711U:
            LOG_INFO << "eG711U";
            __DecodeG711U2Pcm(pInBuf, nInBufLen);
            break;
        case AUDIO_CODING_TYPE::eAdpcm:
            LOG_INFO << "eAdpcm";
            __DecodeAdpcm2Pcm(pInBuf, nInBufLen);
            break;
        default:
            m_iResult.m_eType = AUDIO_CODING_TYPE::eUnSupport;
            break;
    }
    return m_iResult;
}
