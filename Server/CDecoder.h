//
// Created by hc on 2020/5/13.
//

#ifndef C20_CDECODER_H
#define C20_CDECODER_H


#include <muduo/base/copyable.h>
#include <memory>
#include <muduo/net/TcpConnection.h>
#include "JT1078Header.h"
#include "CRtmpStream.h"
#include "CCodec.h"

class CDecoder : muduo::copyable,public std::enable_shared_from_this<CDecoder>
{
public:
    enum CURRENT_RECEIVE_STATE
    {
        eInit,
        eReceiving,
        eCompleted,
    };
    enum SUBCONTRACT_PROCESSING_MARKE
    {
        eAtomic,
        eFirst,
        eLast,
        eIntermediate,
    };
    enum DATA_TYPE
    {
        eVideoI,
        eVideoP,
        eVideoB,
        eAudio,
        ePassthrough,
    };
    enum SKIP
    {
        eSkipPaser16,
        eSkipPaserTheRestData,
        eSkipNon,

    };
    enum ERRS
    {
        eSTATError,
        eMarkeError,
        eTypeError,
        eNoError,
        eLengthError,
        eCodingTypeError,
        eRedisError
    };
    enum AV_CODING_TYPE
    {
        eG711A,
        eG711U,
        eAdpcm,
        eH264,
        eUnSupport,
    };
    CDecoder();
    ~CDecoder();

public:
    bool Decode(muduo::net::Buffer* buf, muduo::Timestamp receiveTime);
    bool DecodeHeader(muduo::net::Buffer* buf, muduo::Timestamp receiveTime);
    void DecodeBody(muduo::net::Buffer* buf, muduo::Timestamp receiveTime);
    CURRENT_RECEIVE_STATE GetCurReceiveStat() const;
    SUBCONTRACT_PROCESSING_MARKE GetProcessingMarke() const;
    DATA_TYPE GetDataType() const;
    ERRS GetErr() const;
    JT_1078_HEADER & GetHeader();
    std::string & GetData();
    void DumpToHex(const std::string & sStr) const;
    void DumpToHex(const JT_1078_HEADER &m_iHeader) const;
    bool Init(std::string& sUrl);
    bool WriteData(AVMediaType iDataType, char *pData, int nDataLen);
    bool GetPushState() const;
    void SetCurReceiveStat(CURRENT_RECEIVE_STATE eState);
    std::string GetUrl() const;
    AV_CODING_TYPE GetAVCodingType() const;

    DECODE_RESULT &DecodeAudio(char *pInBuf, int nInBufLen, AV_CODING_TYPE eType);

public:
    static const int sm_nFirstReceiveBytes;

private:
    bool __ParserHeader(muduo::net::Buffer* buf, muduo::Timestamp receiveTime);
    bool __Paser16(muduo::net::Buffer* buf, muduo::Timestamp receiveTime);
    bool __PaserTheRestData(muduo::net::Buffer* buf, muduo::Timestamp receiveTime);
    int __GetTailLenAndSetDataType();
    void __CheckSubMark();
    void __SetAVCodingType();
private:

    CURRENT_RECEIVE_STATE m_eCurrentStat;
    SUBCONTRACT_PROCESSING_MARKE m_eSubMarke;
    DATA_TYPE m_eDataType;
    SKIP m_eSkip;
    ERRS m_eErr;
    AV_CODING_TYPE m_eCodingType;
    unsigned int  m_iRecvLen;
    JT_1078_HEADER m_iHeader;
    std::string m_sData;
    CRtmpStream *m_pRtmpStream;
    CCodec *m_pCodec;

};
typedef std::shared_ptr<CDecoder> DECODER_PTR;
typedef CDecoder::DATA_TYPE JT1078_MEDIA_DATA_TYPE;
typedef CDecoder::ERRS JT1078_ERRS;
typedef CDecoder::CURRENT_RECEIVE_STATE JT1078_CUR_RECEIVE_STATE;
typedef CDecoder::SUBCONTRACT_PROCESSING_MARKE JT1078_SUB_MARKE;
typedef CDecoder::AV_CODING_TYPE JT1078_AV_CODING_TYPE;
#endif //C20_CDECODER_H
