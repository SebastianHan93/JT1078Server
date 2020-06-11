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
        NAL_SLICE = 1,
        NAL_SLICE_DPA = 2,
        NAL_SLICE_DPB = 3,
        NAL_SLICE_DPC = 4,
        NAL_SLICE_IDR = 5,
        NAL_SEI = 6,
        NAL_SPS = 7,
        NAL_PPS = 8,
        NAL_AUD = 9,
        NAL_FILLER = 12,
    }NALU_TYPE;

public:
    bool Init(std::string sUrl);
    bool WriteH264(unsigned char *pData, int nDatalen,uint64_t nTimestamp);
    int GetOneNalu(unsigned char *pBufIn, int nInSize, unsigned char *pNalu, int &nNaluSize);
    int IsVideojjSEI(unsigned char *pNalu, int nNaluSize);
    int GetOneAACFrame(unsigned char *pBufIn, int nInSize, unsigned char *pAACFrame, int &nAACFrameSize);
    bool GetPushState() const;
    std::string GetUrl() const;
private:
    unsigned long __GetTickCount();
    int __SendVideoSpsPps(uint64_t nTimestamp);
    int __SendRtmpH264(unsigned char *pData, int nDatalen,uint64_t nTimestamp);
private:

    RTMP * m_iRtmp;
    RTMPPacket *m_pPacket;

    unsigned char * m_pSPS;
    unsigned char * m_pPPS;

    std::string m_sUrl;

    bool m_bIsPushing;
    bool m_bStartTimestamp;
    bool m_bNextIsKey;
    bool m_bWriteAVCSeq;
    bool m_bWriteAACSeq;


    int m_nSPSSize;
    int m_nPPSSize;

    uint64_t m_nStartTime;
    uint64_t m_nNowTime;
    uint64_t m_nPreFrameTime;
    uint64_t m_nLastTime;
};


#endif //JT1078SERVER_CRTMP_H
