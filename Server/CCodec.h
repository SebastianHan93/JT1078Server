//
// Created by hc on 2020/5/14.
//

#ifndef JT1078SERVER_CCODEC_H
#define JT1078SERVER_CCODEC_H
#include <iostream>
#include <memory>
#include "CDecoder.h"

class CCodec
{
public:
    CCodec() = default;
    ~CCodec()= default;

public:
    void DecodeG711A2Pcm(char * cInBuf,int nInBufLen,char * cOutBuf,int *nOutBufLen);
    void DecodeG711U2Pcm(char * cInBuf,int nInBufLen,char * cOutBuf,int *nOutBufLen);
    void DecodeAdpcm2Pcm(char * cInBuf,int nInBufLen,char * cOutBuf,int *nOutBufLen);

};



#endif //JT1078SERVER_CCODEC_H
