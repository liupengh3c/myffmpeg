#include "iostream"
extern "C"
{
#include "libavdevice/avdevice.h"
#include "libavformat/avformat.h"
#include "libavcodec/avcodec.h"
#include "libavutil/time.h"

    // 注意，必需定义为static类型，不同的文件中decode重名，会报错
    static int decode(AVCodecContext *ctx, AVPacket *pkt, AVFrame *frame, FILE *f)
    {
        int ret = 0;
        ret = avcodec_send_packet(ctx, pkt);
        if (ret < 0)
        {
            return -1;
        }
        while (ret >= 0)
        {
            ret = avcodec_receive_frame(ctx, frame);
            if (ret == AVERROR(EAGAIN) || ret == AVERROR_EOF)
            {
                return 0;
            }
            else if (ret < 0)
            {
                return -2;
            }
            av_packet_unref(pkt);
            // 写Y分量
            for (size_t i = 0; i < frame->height; i++)
            {
                fwrite(frame->data[0] + frame->linesize[0] * i, 1, frame->width, f);
            }

            // 写U分量
            for (size_t i = 0; i < frame->height / 2; i++)
            {
                fwrite(frame->data[1] + frame->linesize[1] * i, 1, frame->width / 2, f);
            }

            // 写V分量
            for (size_t i = 0; i < frame->height / 2; i++)
            {
                fwrite(frame->data[2] + frame->linesize[2] * i, 1, frame->width / 2, f);
            }
        }
        return 0;
    }
    int video()
    {
        int ret = 0;
        int video_index = -1;
        int64_t start_time = 0;

        AVCodecContext *dec_ctx = NULL;
        AVCodec *decodec = NULL;
        AVFormatContext *fmt_ctx = NULL;
        AVInputFormat *fmt = NULL;
        AVStream *stream = NULL;
        AVFrame *frame = NULL;
        AVPacket *pkt = NULL;

        FILE *f_out = NULL;
        f_out = fopen("video.yuv", "wb+");

        frame = av_frame_alloc();
        pkt = av_packet_alloc();

        // 1. 注册所有输入/输出设备，必须步骤
        avdevice_register_all();

        // 2. find input format
        fmt = av_find_input_format("v4l2");
        if (!fmt)
        {
            std::cout << "find input format error" << std::endl;
            return -1;
        }

        // 3. 打开摄像头
        ret = avformat_open_input(&fmt_ctx, "/dev/video0", fmt, NULL);
        if (ret < 0)
        {
            std::cout << "avformat_open_input error,ret=" << ret << std::endl;
            return -2;
        }

        // 4. 查找流信息
        ret = avformat_find_stream_info(fmt_ctx, NULL);
        if (ret < 0)
        {
            std::cout << "avformat_find_stream_info error,ret=" << ret << std::endl;
            return -3;
        }
        av_dump_format(fmt_ctx, 0, "/dev/video0", 0);

        // 5. 获取视频流索引
        for (size_t i = 0; i < fmt_ctx->nb_streams; i++)
        {
            stream = fmt_ctx->streams[i];
            if (stream->codecpar->codec_type == AVMEDIA_TYPE_VIDEO)
            {
                video_index = i;
                decodec = avcodec_find_decoder(stream->codecpar->codec_id);
                break;
            }
        }

        if (video_index == -1)
        {
            std::cout << "can not find video index" << std::endl;
            return -4;
        }

        // 6. 分配解码器上下文
        dec_ctx = avcodec_alloc_context3(decodec);
        if (!dec_ctx)
        {
            std::cout << "avcodec_alloc_context3 fail" << std::endl;
            return -5;
        }

        // 7. 复制参数到解码器
        ret = avcodec_parameters_to_context(dec_ctx, stream->codecpar);
        if (ret < 0)
        {
            std::cout << "avcodec_parameters_to_context fail,ret=" << ret << std::endl;
            return -6;
        }

        // 8. 打开解码器
        ret = avcodec_open2(dec_ctx, decodec, NULL);
        if (ret < 0)
        {
            std::cout << "avcodec_open2 fail,ret=" << ret << std::endl;
            return -7;
        }
        std::cout << "stream->codecpar->codec_id=" << stream->codecpar->codec_id << std::endl;
        start_time = av_gettime();
        while (true)
        {
            ret = av_read_frame(fmt_ctx, pkt);
            if (ret < 0)
            {
                break;
            }
            ret = decode(dec_ctx, pkt, frame, f_out);
            if (ret < 0)
            {
                break;
            }
            if (av_gettime() - start_time >= 5)
            {
                break;
            }
        }
        // flush the decoder
        decode(dec_ctx, NULL, frame, f_out);
        avformat_close_input(&fmt_ctx);
        avcodec_free_context(&dec_ctx);

        return 0;
    }
}