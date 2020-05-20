//
// Created by hc on 2020/5/14.
//

#include "CRtmpStream.h"
#include "muduo/base/Logging.h"

CRtmpStream::CRtmpStream()
  :m_bIsPushing(false)
{
    have_video = 0;
    have_audio = 0;
    ptsInc = 0;

    oc = NULL;
    opt = NULL;
    video_st = { 0 };
    audio_st = { 0 };

    iRawLineSize = 0;
    iRawBuffSize = 0;
    pRawBuff = NULL;

    iConvertLineSize = 0;
    iConvertBuffSize = 0;
    pConvertBuff = NULL;

    memset(pcmencodebuf, 0, 4096);
    pcmencodesize = 0;
}

CRtmpStream::~CRtmpStream()
{
    /* Write the stream trailer to an output media file */
    av_write_trailer(oc);

    /* Close each codec. */
    if (have_video)
        close_stream(&video_st);
    if (have_audio)
        close_stream(&audio_st);

    /* Close the output file. */
    if (!(fmt->flags & AVFMT_NOFILE))
        avio_closep(&oc->pb);

    /* free the stream */
    if (oc)
        avformat_free_context(oc);

    /* free the audio frame */
    if (pRawBuff)
        av_free(pRawBuff);
    if (pConvertBuff)
        av_free(pConvertBuff);
    if(m_bIsPushing)
    {
        SetPushState(false);
    }
}

int CRtmpStream::Init(const char *filename)
{
    int ret = 0;
    if(filename == nullptr)
    {
        return -1;
    }
    m_sUrl = filename;

    av_register_all();
    avformat_network_init();

    /* allocate the output media context */
    avformat_alloc_output_context2(&oc, NULL, "flv", filename);
    if (!oc) {
        LOG_ERROR << "CRtmpStream::Init::avformat_alloc_output_context2-->无法从文件扩展名推断输出格式";
        return -1;
    }

    fmt = oc->oformat;

    /* Add the audio and video streams using the default format codecs
    * and initialize the codecs. */
    if (fmt->video_codec != AV_CODEC_ID_NONE) {
        fmt->video_codec = AV_CODEC_ID_H264;
        ret = add_stream(&video_st, &video_codec, fmt->video_codec);
        if (ret < 0) {
            LOG_ERROR << "CRtmpStream::Init::add_stream-->打开视频流出错";
            return -1;
        }
        have_video = 1;
    }
    if (fmt->audio_codec != AV_CODEC_ID_NONE) {
        fmt->audio_codec = AV_CODEC_ID_AAC;
        ret = add_stream(&audio_st, &audio_codec, fmt->audio_codec);
        if (ret < 0) {
            LOG_ERROR << "CRtmpStream::Init::add_stream-->打开音频流出错";
            return -1;
        }
        have_audio = 1;
    }

    /* Now that all the parameters are set, we can open the audio and
    * video codecs and allocate the necessary encode buffers. */
    if (have_video) {
        ret = open_video(video_codec, &video_st, opt);
        if (ret < 0) {
            LOG_ERROR << "CRtmpStream::Init::open_video-->打开video失败";
            return -1;
        }
    }

    if (have_audio) {
        ret = open_audio(audio_codec, &audio_st, opt);
        if (ret < 0) {
            LOG_ERROR << "CRtmpStream::Init::open_audio-->打开audio失败";
            return -1;
        }
    }

    av_dump_format(oc, 0, filename, 1);

    /* open the output file, if needed */
    if (!(fmt->flags & AVFMT_NOFILE)) {
        ret = avio_open(&oc->pb, filename, AVIO_FLAG_WRITE);
        if (ret < 0) {
            printf("Could not open '%s'\n", filename);
            LOG_ERROR << "CRtmpStream::Init::avio_open-->打开URL失败-->URL["<<filename<<"]";
            return -1;
        }
    }

    /* Write the stream header, if any. */
    ret = avformat_write_header(oc, &opt);
    if (ret < 0) {
        LOG_ERROR << "CRtmpStream::Init::avformat_write_header-->打开输出文件发生错误";
        return -1;
    }
    m_bIsPushing = true;
    return 0;
}
/*
* If the data is video, the input is H264;
* If the data is audio, the input is PCM;
*/
int CRtmpStream::WriteData(AVMediaType datatype, char *data, int datalen)
{
    int ret = 0;

    if (AVMEDIA_TYPE_VIDEO == datatype) {
        ret = write_video_frame(&video_st, data, datalen);
    } else if (AVMEDIA_TYPE_AUDIO == datatype) {
        ret = write_audio_frame(&audio_st, data, datalen);
    }

    return ret;
}

int CRtmpStream::write_frame(AVFormatContext *fmt_ctx, const AVRational *time_base, AVStream *st, AVPacket *pkt)
{
    /* rescale output packet timestamp values from codec to stream timebase */
    av_packet_rescale_ts(pkt, *time_base, st->time_base);
    pkt->stream_index = st->index;

    /* Write the compressed frame to the media file. */
    return av_interleaved_write_frame(fmt_ctx, pkt);
}

/* Add an output stream. */
int CRtmpStream::add_stream(OutputStream *ost, AVCodec **codec, enum AVCodecID codec_id)
{
    int i = 0;
    AVCodecContext *c = NULL;
    AVRational a_time_base;
    AVRational v_time_base;

    /* find the encoder */
    *codec = avcodec_find_encoder(codec_id);
    if (!(*codec)) {
        LOG_ERROR << "CRtmpStream::add_stream::avcodec_find_encoder -->寻找编码器错误-->编码器ID["<<avcodec_get_name(codec_id) <<"]";
        return -1;
    }

    ost->st = avformat_new_stream(oc, NULL);
    if (!ost->st) {
        LOG_ERROR << "CRtmpStream::add_stream::avformat_new_stream-->初始化流错误";
        return -1;
    }
    ost->st->id = oc->nb_streams - 1;

    c = avcodec_alloc_context3(*codec);
    if (!c) {
        LOG_ERROR << "CRtmpStream::add_stream::avcodec_alloc_context3-->初始化编码上下文错误";
        return -1;
    }
    ost->enc = c;

    switch ((*codec)->type) {
        case AVMEDIA_TYPE_AUDIO:
            c->codec_id = codec_id;
            c->codec_type = AVMEDIA_TYPE_AUDIO;
            /**
             * 对于浮点格式，其值在[-1.0,1.0]之间，任何在该区间之外的值都超过了最大音量的范围。
                和YUV的图像格式格式，音频的采样格式分为平面（planar）和打包（packed）两种类型，
                在枚举值中上半部分是packed类型，后面（有P后缀的）是planar类型。
             */
            c->sample_fmt = AV_SAMPLE_FMT_FLTP;//音频采样格式
            c->bit_rate = 64000;
            c->sample_rate = 8000;
            c->channel_layout = AV_CH_LAYOUT_MONO;//声道数
            c->channels = av_get_channel_layout_nb_channels(c->channel_layout);
            a_time_base = { 1, c->sample_rate };
            ost->st->time_base = a_time_base;
            break;

        case AVMEDIA_TYPE_VIDEO:
            c->codec_id = codec_id;
            c->codec_type = AVMEDIA_TYPE_VIDEO;
            c->pix_fmt = AV_PIX_FMT_YUV420P;
            c->bit_rate = 400000;
            c->width = 1280;
            c->height = 720;
            v_time_base = { 1, 50 };
            ost->st->time_base = v_time_base;
            c->time_base = ost->st->time_base;
            c->codec_tag = 0;

//            c->rc_buffer_size = 20000;

            break;

        default:
            break;
    }

    /* Some formats want stream headers to be separate. */
    if (oc->oformat->flags & AVFMT_GLOBALHEADER)
        c->flags |= AV_CODEC_FLAG_GLOBAL_HEADER;

    return 0;
}

int CRtmpStream::open_audio(AVCodec *codec, OutputStream *ost, AVDictionary *opt_arg)
{
    int ret = 0;
    int nb_samples = 0;
    AVDictionary *opt = NULL;
    AVCodecContext *c = ost->enc;

    /* open it */
    av_dict_copy(&opt, opt_arg, 0);
    ret = avcodec_open2(c, codec, &opt);
    av_dict_free(&opt);
    if (ret < 0) {
        printf("Could not open audio codec\n");
        LOG_ERROR << "CRtmpStream::open_audio::avcodec_open2-->无法找到音频编码器";
        return -1;
    }

    /* copy the stream parameters to the muxer */
    ret = avcodec_parameters_from_context(ost->st->codecpar, c);
    if (ret < 0) {
        LOG_ERROR << "CRtmpStream::open_audio::avcodec_parameters_from_context-->流参数错误";

        return -1;
    }

    nb_samples = c->frame_size;

    /* init and alloc input(pcm) audio frame */
    iRawBuffSize = av_samples_get_buffer_size(&iRawLineSize, c->channels, nb_samples, AV_SAMPLE_FMT_S16, 0);
    pRawBuff = (uint8_t *)av_malloc(iRawBuffSize);

    ost->tmp_frame = av_frame_alloc();
    ost->tmp_frame->nb_samples = nb_samples;
    ost->tmp_frame->format = AV_SAMPLE_FMT_S16;
    ost->tmp_frame->channels = c->channels;

    ret = avcodec_fill_audio_frame(ost->tmp_frame, c->channels, AV_SAMPLE_FMT_S16, (const uint8_t*)pRawBuff, iRawBuffSize, 0);
    if (ret<0)
    {
        LOG_ERROR << "CRtmpStream::open_audio::avcodec_fill_audio_frame-->无法填充音频帧";

        return -1;
    }

    /* init and alloc need resample(aac) audio frame */
    iConvertBuffSize = av_samples_get_buffer_size(&iConvertLineSize, c->channels, nb_samples, c->sample_fmt, 0);
    pConvertBuff = (uint8_t *)av_malloc(iConvertBuffSize);

    ost->frame = av_frame_alloc();
    ost->frame->nb_samples = nb_samples;
    ost->frame->format = c->sample_fmt;
    ost->frame->channels = c->channels;

    ret = avcodec_fill_audio_frame(ost->frame, c->channels, c->sample_fmt, (const uint8_t*)pConvertBuff, iConvertBuffSize, 0);
    if (ret < 0)
    {
        LOG_ERROR << "CRtmpStream::open_audio::avcodec_fill_audio_frame-->无法填充重采样音频帧";
        return -1;
    }

    /* create resampler context */
    ost->swr_ctx = swr_alloc();
    if (!ost->swr_ctx) {
        LOG_ERROR << "CRtmpStream::open_audio::swr_alloc-->无法分配重采样上下文";
        return -1;
    }

    /* set options */
    av_opt_set_int(ost->swr_ctx, "in_channel_count", c->channels, 0);
    av_opt_set_int(ost->swr_ctx, "in_sample_rate", c->sample_rate, 0);
    av_opt_set_sample_fmt(ost->swr_ctx, "in_sample_fmt", AV_SAMPLE_FMT_S16, 0);
    av_opt_set_int(ost->swr_ctx, "out_channel_count", c->channels, 0);
    av_opt_set_int(ost->swr_ctx, "out_sample_rate", c->sample_rate, 0);
    av_opt_set_sample_fmt(ost->swr_ctx, "out_sample_fmt", c->sample_fmt, 0);

    /* initialize the resampling context */
    if ((ret = swr_init(ost->swr_ctx)) < 0) {
        LOG_ERROR << "CRtmpStream::open_audio::swr_init-->初始化重采样上下文错误";
        return -1;
    }
    LOG_DEBUG << "打开音频编码器成功";
    return 0;
}

/*
* encode one audio frame and send it to the muxer
* return 1 when encoding is finished, 0 otherwise
*/
int CRtmpStream::write_audio_frame(OutputStream *ost, char* data, int datalen)
{
    int ret = 0;
    int got_packet = 0;
    int dst_nb_samples = 0;
    AVCodecContext *c = ost->enc;
    AVPacket pkt = { 0 };
    AVFrame *frame = NULL;

    av_init_packet(&pkt);

    frame = ost->tmp_frame;

    /* Compose a frame of data to be resampled */
    memcpy(&pcmencodebuf[pcmencodesize], data, datalen);
    pcmencodesize += datalen;
    if (pcmencodesize >= iRawBuffSize) {
        memcpy(pRawBuff, pcmencodebuf, iRawBuffSize);

        pcmencodesize -= iRawBuffSize;
        memcpy(&pcmencodebuf[0], &pcmencodebuf[iRawBuffSize], pcmencodesize);

        frame->pts = ost->next_pts;
        ost->next_pts += frame->nb_samples;
    } else {
        return 0;
    }

    if (frame) {
        dst_nb_samples = frame->nb_samples;

        ret = swr_convert(ost->swr_ctx, (uint8_t**)ost->frame->data, frame->nb_samples, (const uint8_t**)frame->data, frame->nb_samples);
        if (ret < 0) {
            printf("Error while converting\n");
            LOG_ERROR << "CRtmpStream::write_audio_frame::swr_convert-->编码转换错误";

            return -1;
        }

        frame = ost->frame;
        frame->pts = ost->samples_count;
        ost->samples_count += dst_nb_samples;
    }

    ret = avcodec_encode_audio2(c, &pkt, frame, &got_packet);
    if (ret < 0) {
        LOG_ERROR << "CRtmpStream::write_audio_frame::avcodec_encode_audio2-->编码音频错误";
        return -1;
    }

    if (got_packet) {
        ret = write_frame(oc, &c->time_base, ost->st, &pkt);
        if (ret < 0) {
            LOG_ERROR << "CRtmpStream::write_audio_frame::write_frame-->写入音频错误";
            return -1;
        }
    }

    return 0;
}

int CRtmpStream::open_video(AVCodec *codec, OutputStream *ost, AVDictionary *opt_arg)
{
    int ret = 0;
    AVDictionary *opt = NULL;
    AVCodecContext *c = ost->enc;

    av_dict_copy(&opt, opt_arg, 0);
    /* open the codec */
    ret = avcodec_open2(c, codec, &opt);
    av_dict_free(&opt);
    if (ret < 0) {
        LOG_ERROR << "CRtmpStream::open_video::avcodec_open2-->打开视频编码器错误";
        return -1;
    }

    /* copy the stream parameters to the muxer */
    ret = avcodec_parameters_from_context(ost->st->codecpar, c);
    if (ret < 0) {
        LOG_ERROR << "CRtmpStream::open_video::avcodec_parameters_from_context-->设置视频流信息出错";

        return -1;
    }

    LOG_DEBUG << "打开视频编码器成功";
    return 0;
}

/*
* encode one video frame and send it to the muxer
* return 1 when encoding is finished, 0 otherwise
*/
int CRtmpStream::write_video_frame(OutputStream *ost, char* data, int datalen)
{
    int ret = 0;
    int isI = 0;
    AVCodecContext *c = ost->enc;
    AVPacket pkt = { 0 };

    av_init_packet(&pkt);

    isI = isIdrFrame1((uint8_t*)data, datalen);
    pkt.flags |= isI ? AV_PKT_FLAG_KEY : 0;
    pkt.data = (uint8_t*)data;
    pkt.size = datalen;

    AVRational time_base = { 1, 1000};
    pkt.pts = av_rescale_q((ptsInc++) * 2, time_base, ost->st->time_base);
    pkt.dts = av_rescale_q_rnd(pkt.dts, ost->st->time_base, ost->st->time_base, (AVRounding)(AV_ROUND_NEAR_INF | AV_ROUND_PASS_MINMAX));
    pkt.duration = av_rescale_q(pkt.duration, ost->st->time_base, ost->st->time_base);
    pkt.pos = -1;

    ret = write_frame(oc, &c->time_base, ost->st, &pkt);
    if (ret < 0) {
        LOG_DEBUG << "CRtmpStream::write_video_frame::write_frame-->写入视频帧出错";
        return -1;
    }

    return 0;
}

void CRtmpStream::close_stream(OutputStream *ost)
{
    avcodec_free_context(&ost->enc);
    av_frame_free(&ost->frame);
    av_frame_free(&ost->tmp_frame);
    swr_free(&ost->swr_ctx);
}

bool CRtmpStream::isIdrFrame2(uint8_t *buf, int len)
{
    switch (buf[0] & 0x1f) {
        case 7: // SPS
            return true;
        case 8: // PPS
            return true;
        case 5:
            return true;
        case 1:
            return false;

        default:
            return false;
            break;
    }
    return false;
}

bool CRtmpStream::isIdrFrame1(uint8_t *buf, int size)
{
    int last = 0;
    for (int i = 2; i <= size; ++i) {
        if (i == size) {
            if (last) {
                bool ret = isIdrFrame2(buf + last, i - last);
                if (ret) {
                    return true;
                }
            }
        }
        else if (buf[i - 2] == 0x00 && buf[i - 1] == 0x00 && buf[i] == 0x01) {
            if (last) {
                int size = i - last - 3;
                if (buf[i - 3]) ++size;
                bool ret = isIdrFrame2(buf + last, size);
                if (ret) {
                    return true;
                }
            }
            last = i + 1;
        }
    }
    return false;
}

void CRtmpStream::SetPushState(bool bIsPush)
{
    m_bIsPushing = bIsPush;
}

bool CRtmpStream::GetPushState() const
{
    return m_bIsPushing;
}

std::string CRtmpStream::GetUrl() const
{
    return m_sUrl;
}