//
// Created by hc on 2020/5/14.
//

#include "CCodec.h"
#include "../Codec/G711.h"
#include "../Codec/G726.h"
#include "../Codec/Adpcm.h"

void CCodec::DecodeAdpcm2Pcm(char * cInBuf,int nInBufLen,char * cOutBuf,int * nOutBufLen)
{
    adpcm_state iAdpcmState{0};
    iAdpcmState.valprev = (cInBuf[5]<<8) | (cInBuf[4] & 0xFF);
    iAdpcmState.index = cInBuf[6];
    nInBufLen -= 8;
    cInBuf += 8;
    adpcm_decoder((char*)cInBuf,(short*)cOutBuf,nInBufLen*2,&iAdpcmState);
}

void CCodec::DecodeG711A2Pcm(char * cInBuf,int nInBufLen,char * cOutBuf,int *nOutBufLen)
{
    int nRet = g711a_decode((short*)cOutBuf, (unsigned char*)cInBuf ,nInBufLen);
    *nOutBufLen = nRet;
}

void CCodec::DecodeG711U2Pcm(char * cInBuf,int nInBufLen,char * cOutBuf,int *nOutBufLen)
{
    int nRet = g711u_decode((short*)cOutBuf, (unsigned char*)cInBuf ,nInBufLen);
    *nOutBufLen = nRet;
}