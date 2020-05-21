//
// Created by hc on 2020/5/14.
//

#ifndef C20_CRTMPSTREAM_H
#define C20_CRTMPSTREAM_H

#include "iostream"
#include "CFFmpeg.h"
typedef struct OutputStream {
    AVStream *st;
    AVCodecContext *enc;

    int64_t next_pts;
    long long samples_count;

    AVFrame *frame;
    AVFrame *tmp_frame;

    struct SwrContext *swr_ctx;
} OutputStream;

class CRtmpStream
{
public:
    CRtmpStream();
    virtual ~CRtmpStream();

    int Init(const char *pFilename);
    int WriteData(AVMediaType datatype, char *data, int datalen);
    bool GetPushState() const;
    std::string GetUrl() const;

    static int GetSpsPpsFromH264(uint8_t* pBuf, int nLen);

private:
    void SetPushState(bool bIsPush);
    int write_frame(AVFormatContext *fmt_ctx, const AVRational *time_base, AVStream *st, AVPacket *pkt);

    int add_stream(OutputStream *ost, AVCodec **codec, enum AVCodecID codec_id);

    int open_audio(AVCodec *codec, OutputStream *ost, AVDictionary *opt_arg);
    int write_audio_frame(OutputStream *ost, char* data, int datalen);

    int open_video(AVCodec *codec, OutputStream *ost, AVDictionary *opt_arg);
    int write_video_frame(OutputStream *ost, char* data, int datalen);

    void close_stream(OutputStream *ost);

    bool isIdrFrame2(uint8_t *buf, int len);
    bool isIdrFrame1(uint8_t *buf, int size);
private:
    OutputStream video_st, audio_st;
    AVOutputFormat *fmt;
    AVFormatContext *oc;
    AVCodec *audio_codec, *video_codec;
    AVDictionary *opt;

    int have_video, have_audio;
    int ptsInc;

    int iRawLineSize;
    int iRawBuffSize;
    uint8_t *pRawBuff;

    int iConvertLineSize;
    int iConvertBuffSize;
    uint8_t *pConvertBuff;

    char pcmencodebuf[4096];
    int pcmencodesize;
    bool m_bIsPushing;
    std::string m_sUrl;
};





#endif //C20_CRTMPSTREAM_H
