//
// Created by hc on 2020/5/27.
//

#ifndef JT1078SERVER_FLV_H
#define JT1078SERVER_FLV_H


#include <muduo/base/noncopyable.h>
#include <cstdint>
#include <fstream>
namespace cnvt
{
    class u4
    {
    public:
        u4(uint32_t i) { _u[0] = i >> 24; _u[1] = (i >> 16) & 0xff; _u[2] = (i >> 8) & 0xff; _u[3] = i & 0xff; }

    public:
        unsigned char _u[4];
    };
    class u3
    {
    public:
        u3(uint32_t i) { _u[0] = i >> 16; _u[1] = (i >> 8) & 0xff; _u[2] = i & 0xff; }

    public:
        unsigned char _u[3];
    };
    class u2
    {
    public:
        u2(uint32_t i) { _u[0] = i >> 8; _u[1] = i & 0xff; }

    public:
        unsigned char _u[2];
    };

#pragma pack (1)
    typedef struct
    {
        uint8_t     m_ui8SignatureF;
        uint8_t     m_ui8SignatureL;
        uint8_t     m_ui8SignatureV;
        uint8_t     m_ui8Version;
        uint8_t     m_ub5TypeFlagsReserved1:5;
        uint8_t     m_ub1TypeFlagsAudio:1;
        uint8_t     m_ub1TypeFlagsReserved2:1;
        uint8_t     m_ub1TypeFlagsVideo:1;
        uint32_t    m_ui32DataOffset;
    }FLV_HEADER;
#pragma pack()

    typedef struct
    {
        uint8_t m_ub4SoundFormat:4;
        uint8_t m_ub2SoundRate:2;
        uint8_t m_ub1SoundSize:1;
        uint8_t m_ub1SoundType:1;
        uint8_t m_ui8AACPacketType;
    }AUDIO_TAG_HEADER;

    typedef struct
    {

    }VIDEO_TAG_HEADER;


#pragma pack (1)
    typedef struct
    {
        uint8_t     m_ub2Reserved:2;
        uint8_t     m_ub1Filter:1;
        uint8_t     m_ub5TagType:5;
        uint8_t     m_ui24DataSize[3];
        uint8_t     m_ui24Timestamp[3];
        uint8_t     m_ui8TimestampExtended;
        uint8_t     m_ui24StreamID[3];
    }FLV_TAG_HEADER;
#pragma pack()

    class Flv : muduo::noncopyable
    {
    public:
        Flv();
        ~Flv();

    public:

        int GetOneNalu(unsigned char *pBufIn, int nInSize, unsigned char *pNalu, int &nNaluSize);
        int IsVideojjSEI(unsigned char *pNalu, int nNaluSize);

        int GetOneAACFrame(unsigned char *pBufIn, int nInSize, unsigned char *pAACFrame, int &nAACFrameSize);

    private:
        int __ConvertH264(char *pNalu, int nNaluSize, unsigned int nTimeStamp);
        int __ConvertAAC(char *pAAC, int nAACFrameSize, unsigned int nTimeStamp);

        void __MakeFlvHeader(FLV_HEADER & iFlvHeader);
        void __WriteH264Header(unsigned int nTimeStamp);
        void __WriteH264Frame(char *pNalu, int nNaluSize, unsigned int nTimeStamp);
        void __WriteH264EndofSeq();
        void __WriteAACHeader(unsigned int nTimeStamp);
        void __MakeAACHeader(unsigned int nTimeStamp);
        void __WriteAACFrame(char *pFrame, int nFrameSize, unsigned int nTimeStamp);

        void __Write(unsigned char u) { m_iFileOut.write((char *)&u, 1); }
        void __Write(u4 u) { m_iFileOut.write((char *)u._u, 4); }
        void __Write(u3 u) { m_iFileOut.write((char *)u._u, 3); }
        void __Write(u2 u) { m_iFileOut.write((char *)u._u, 2); }

    private:
        bool m_bWriteAVCSeqHeader;
        unsigned char * m_pSPS;
        unsigned char * m_pPPS;
        int m_nSPSSize;
        int m_nPPSSize;
        int m_nStreamID;
        uint32_t m_nPrevTagSize;
        uint32_t m_nCachePrevTagSize;
        unsigned int m_nVideoTimeStamp;

        bool m_bWriteAACSeqHeader;
        unsigned char * m_pAudioSpecificConfig;
        int m_nAudioConfigSize;
        int m_nAACProfile;
        int m_nSampleRateIndex;
        int m_nChannelConfig;


        bool m_bHaveAudio;
        bool m_bHaveVideo;
        std::fstream m_iFileOut;
        FLV_HEADER m_iFlvHeader;
        FLV_TAG_HEADER m_iFlvTagHeader;
        AUDIO_TAG_HEADER m_iAudioTagHeader;
    };
}



#endif //JT1078SERVER_FLV_H
