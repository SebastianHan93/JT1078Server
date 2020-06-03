//
// Created by hc on 2020/5/29.
//

#ifndef JT1078SERVER_CRTMP_H
#define JT1078SERVER_CRTMP_H

#include <librtmp/rtmp.h>
#include <string>

#define RTMP_HEAD_SIZE   (sizeof(RTMPPacket)+RTMP_MAX_HEADER_SIZE)

class CRtmp
{
public:
    CRtmp();
    ~CRtmp();

public:
    typedef enum{
        NAL_SLICE = 0x01,
        NAL_SLICE_DPA = 0x02,
        NAL_SLICE_DPB = 0x03,
        NAL_SLICE_DPC = 0x04,
        NAL_SLICE_IDR = 0x05,
        NAL_SEI = 0x06,
        NAL_SPS = 0x07,
        NAL_PPS = 0x08,
        NAL_AUD = 0x09,
        NAL_FILLER = 0x12,
    }NALU_TYPE;

public:
    bool Init(std::string sUrl);
    bool WriteH264(unsigned char *pData, int nDatalen);
    int GetOneNalu(unsigned char *pBufIn, int nInSize, unsigned char *pNalu, int &nNaluSize);
    int IsVideojjSEI(unsigned char *pNalu, int nNaluSize);
    int GetOneAACFrame(unsigned char *pBufIn, int nInSize, unsigned char *pAACFrame, int &nAACFrameSize);
    bool GetPushState() const;
    std::string GetUrl() const;
private:
    unsigned long __GetTickCount();
    int __SendVideoSpsPps();
    int __SendRtmpH264(unsigned char *pData, int nDatalen);
private:

    RTMP * m_iRtmp;
    RTMPPacket *m_pPacket;

    unsigned char * m_pSPS;
    unsigned char * m_pPPS;

    std::string m_sUrl;

    bool m_bIsPushing;
    bool m_bNextIsKey;
    bool m_bWriteAVCSeq;
    bool m_bWriteAACSeq;


    int m_nSPSSize;
    int m_nPPSSize;

    uint32_t m_nStartTime;
    uint32_t m_nNowTime;
    uint32_t m_nPreFrameTime;
    uint32_t m_nLastTime;
};


#endif //JT1078SERVER_CRTMP_H
