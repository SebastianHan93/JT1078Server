//
// Created by hc on 2020/5/27.
//
#include <cstring>
#include "Flv.h"

namespace cnvt
{
    Flv::Flv()
    {
        m_pSPS = nullptr;
        m_pPPS = nullptr;
        m_nSPSSize = 0;
        m_nPPSSize = 0;
        m_bWriteAVCSeqHeader = false;
        m_nPrevTagSize = 0;
        m_nStreamID = 0;

        m_pAudioSpecificConfig = nullptr;
        m_nAudioConfigSize = 0;
        m_nAACProfile = 0;
        m_nSampleRateIndex = 0;
        m_nChannelConfig = 0;
        m_bWriteAACSeqHeader = false;

        m_nCachePrevTagSize = 0;
    }

    Flv::~Flv()
    {
        if(m_pSPS!= nullptr)
        {
            delete m_pSPS;
            m_pSPS = nullptr;
        }

        if(m_pPPS!= nullptr)
        {
            delete m_pPPS;
            m_pPPS = nullptr;
        }

        if(m_bHaveVideo)
        {
            __WriteH264EndofSeq();
        }

        if(m_pAudioSpecificConfig!= nullptr)
        {
            delete[] m_pAudioSpecificConfig;
            m_pAudioSpecificConfig = nullptr;
        }
        m_iFileOut.close();
    }

    int Flv::__ConvertH264(char *pNalu, int nNaluSize, unsigned int nTimeStamp)
    {
        m_nVideoTimeStamp = nTimeStamp;
        if(pNalu == nullptr || nNaluSize <=4)
            return 0;

        int nNaluType = pNalu[4] & 0x1f;
        if(m_pSPS == nullptr && nNaluType == 0x07)
        {
            m_pSPS = new unsigned char[nNaluSize];
            m_nSPSSize = nNaluSize;
            memcpy(m_pSPS,pNalu,nNaluSize);
        }
        if(m_pPPS == nullptr && nNaluType == 0x08)
        {
            m_pPPS = new unsigned char[nNaluSize];
            m_nPPSSize = nNaluSize;
            memcpy(m_pPPS,pNalu,nNaluSize);
        }
        if(m_pSPS != nullptr && m_pPPS != nullptr && nNaluType == 0x07 && m_bWriteAVCSeqHeader == 0)
        {
            __WriteH264Header(nTimeStamp);
            m_bWriteAVCSeqHeader = true;
        }
        if(!m_bWriteAVCSeqHeader)
        {
            return 1;
        }
        __WriteH264Frame(pNalu,nNaluSize,nTimeStamp);
        return 1;
    }

    int Flv::__ConvertAAC(char *pAAC, int nAACFrameSize, unsigned int nTimeStamp)
    {
        if(pAAC == nullptr || nAACFrameSize <=7)
            return 0;
        if(m_pAudioSpecificConfig == nullptr)
        {
            m_pAudioSpecificConfig = new unsigned char[2];
            m_nAudioConfigSize = 2;
            unsigned char * p = (unsigned char *)pAAC;
            m_nAACProfile = (p[2]>>6) + 1;
            m_nSampleRateIndex = (p[2] >> 2) & 0x0f;
            m_nChannelConfig = ((p[2] & 0x01) << 2) + (p[3] >> 6);
            m_pAudioSpecificConfig[0] = (m_nAACProfile << 3) + (m_nSampleRateIndex>>1);
            m_pAudioSpecificConfig[1] = ((m_nSampleRateIndex & 0x01) <<7) + (m_nChannelConfig<<3);
        }
        if(m_pAudioSpecificConfig != nullptr & m_bWriteAACSeqHeader == false)
        {
            __WriteAACHeader(nTimeStamp);
            m_bWriteAACSeqHeader = true;
        }
        if(!m_bWriteAACSeqHeader)
            return 1;
        __WriteAACFrame(pAAC,nAACFrameSize,nTimeStamp);
        return 1;

    }

    void Flv::__MakeFlvHeader(FLV_HEADER & iFlvHeader)
    {
        iFlvHeader.m_ui8SignatureF = 'F';
        iFlvHeader.m_ui8SignatureL = 'L';
        iFlvHeader.m_ui8SignatureV = 'V';
        iFlvHeader.m_ui8Version = 1;
        iFlvHeader.m_ub5TypeFlagsReserved1 = 0x0;
        if(m_bHaveAudio)
            iFlvHeader.m_ub1TypeFlagsAudio = 0x1;
        else
            iFlvHeader.m_ub1TypeFlagsAudio = 0x0;
        iFlvHeader.m_ub1TypeFlagsReserved2 = 0x0;
        if(m_bHaveVideo)
            iFlvHeader.m_ub1TypeFlagsVideo = 0x1;
        else
            iFlvHeader.m_ub1TypeFlagsVideo = 0x0;
        iFlvHeader.m_ui32DataOffset = htobe32(9);
    }

    void Flv::__WriteH264Header(unsigned int nTimeStamp)
    {

    }

    void Flv::__WriteH264Frame(char *pNalu, int nNaluSize, unsigned int nTimeStamp)
    {

    }

    void Flv::__WriteH264EndofSeq()
    {

    }

    void Flv::__MakeAACHeader(unsigned int nTimeStamp)
    {
        m_nPrevTagSize = htobe32(m_nCachePrevTagSize);
        m_iFlvTagHeader.m_ub2Reserved = 0x0;
        m_iFlvTagHeader.m_ub1Filter = 0x0;
        m_iFlvTagHeader.m_ub5TagType = 0x8;
        //AUDIO_TAG_HEADER + m_pAudioSpecificConfig
        u3 datasize_u3(2+2);
        memcpy(m_iFlvTagHeader.m_ui24DataSize,datasize_u3._u, sizeof(m_iFlvTagHeader.m_ui24DataSize));
        u3 ts_u3(nTimeStamp);
        memcpy(m_iFlvTagHeader.m_ui24Timestamp,ts_u3._u, sizeof(m_iFlvTagHeader.m_ui24Timestamp));
        m_iFlvTagHeader.m_ui8TimestampExtended = nTimeStamp >> 24;
        //Always 0.
        memset(m_iFlvTagHeader.m_ui24StreamID,0, sizeof(m_iFlvTagHeader.m_ui24StreamID));
        m_iAudioTagHeader.m_ub4SoundFormat = 0xA;
        //0 = 5.5 kHz
        //1 = 11 kHz
        //2 = 22 kHz
        //3 = 44 kHz
        m_iAudioTagHeader.m_ub2SoundRate = 0x3;
        //0 = 8-bit samples
        //1 = 16-bit samples
        m_iAudioTagHeader.m_ub1SoundSize = 0x1;
        //0 = Mono sound
        //1 = Stereo sound
        m_iAudioTagHeader.m_ub1SoundType = 0x1;
        //0 = AAC sequence header
        //1 = AAC raw
        m_iAudioTagHeader.m_ui8AACPacketType = 0x01;

        //FLV_HEADER + AUDIO_TAG_HEADER + m_pAudioSpecificConfig
        m_nCachePrevTagSize = 11 + 2 + 2;
    }

    void Flv::__WriteAACHeader(unsigned int nTimeStamp)
    {
        __MakeAACHeader(nTimeStamp);
    }

    void Flv::__WriteAACFrame(char *pFrame, int nFrameSize, unsigned int nTimeStamp)
    {

    }

    int Flv::GetOneNalu(unsigned char *pBufIn, int nInSize, unsigned char *pNalu, int &nNaluSize)
    {
        unsigned char *p = pBufIn;
        int nStartPos = 0, nEndPos = 0;
        while (1)
        {
            if (p[0] == 0 && p[1] == 0 && p[2] == 0 && p[3] == 1)
            {
                nStartPos = p - pBufIn;
                break;
            }
            p++;
            if (p - pBufIn >= nInSize - 4)
                return 0;
        }
        p++;
        while (1)
        {
            if (p[0] == 0 && p[1] == 0 && p[2] == 0 && p[3] == 1)
            {
                nEndPos = p - pBufIn;
                break;
            }
            p++;
            if (p - pBufIn >= nInSize - 4)
            {
                nEndPos = nInSize;
                break;
            }
        }
        nNaluSize = nEndPos - nStartPos;
        memcpy(pNalu, pBufIn + nStartPos, nNaluSize);

        return 1;
    }

    int Flv::IsVideojjSEI(unsigned char *pNalu, int nNaluSize)
    {
        unsigned char *p = pNalu;

        if (p[3] != 1 || p[4] != 6 || p[5] != 5)
            return 0;
        p += 6;
        while (*p++==0xff) ;
        const char *szVideojjUUID = "VideojjLeonUUID";
        char *pp = (char *)p;
        for (int i = 0; i < strlen(szVideojjUUID); i++)
        {
            if (pp[i] != szVideojjUUID[i])
                return 0;
        }

        return 1;
    }

    int Flv::GetOneAACFrame(unsigned char *pBufIn, int nInSize, unsigned char *pAACFrame, int &nAACFrameSize)
    {
        unsigned char *p = pBufIn;

        if (nInSize <= 7)
            return 0;

        int nFrameSize = ((p[3] & 0x3) << 11) + (p[4] << 3) + (p[5] >> 5);
        if (nInSize < nFrameSize)
            return 0;

        nAACFrameSize = nFrameSize;
        memcpy(pAACFrame, pBufIn, nFrameSize);

        return 1;
    }

}