#include "iostream"

#include "SDL2/SDL.h"
extern "C"
{
#include "libavdevice/avdevice.h"
#include "libavformat/avformat.h"
#include "libavcodec/avcodec.h"
#include "libavutil/time.h"
#include "libswscale/swscale.h"

#include <libavutil/imgutils.h>
    static int encode(AVCodecContext *enc_ctx, AVPacket *pkt, AVFrame *frame, AVFormatContext *fmt)
    {
        int ret = 0;

        ret = avcodec_send_frame(enc_ctx, frame);
        if (ret < 0)
        {
            std::cout << "avcodec_send_frame error,ret=" << ret << std::endl;
            return -1;
        }
        while (ret >= 0)
        {
            ret = avcodec_receive_packet(enc_ctx, pkt);
            if (ret == AVERROR(EAGAIN) || ret == AVERROR_EOF)
            {
                return 0;
            }
            else if (ret < 0)
            {
                std::cout << "avcodec_receive_packet error,ret=" << ret << std::endl;
                return -2;
            }
            pkt->stream_index = 0;
            std::cout << "pkt->pts = " << pkt->pts << std::endl;
            ret = av_interleaved_write_frame(fmt, pkt);
            av_packet_unref(pkt);
            if (ret < 0)
            {
                std::cout << "av_interleaved_write_frame error,ret=" << ret << std::endl;
            }
        }
        return 0;
    }
    int captureFrame()
    {
        int ret = -1;
        int stream_index = -1;
        //输入文件
        AVInputFormat *ifmt = NULL;
        AVFormatContext *infmt_ctx = NULL;
        AVFormatContext *outfmt_ctx = NULL;

        AVPacket packet;
        av_init_packet(&packet);
        packet.data = NULL;
        packet.size = 0;

        unsigned char *src_data[4];
        unsigned char *dst_data[4];
        int src_linesize[4];
        int dst_linesize[4];

        const char *rtmp_addr = "rtmp://106.13.105.231:8144/live/rfBd56ti2SMtYvSgD5xAV0YU99zampta7Z7S575KLkIZ9PYk";

        ifmt = av_find_input_format("video4linux2");
        if (!ifmt)
        {
            std::cout << "find input format error" << std::endl;
            return -1;
        }
        ret = avformat_open_input(&infmt_ctx, "/dev/video0", ifmt, NULL);
        if (ret < 0)
        {
            printf("failed open input file\n");
            return -2;
        }
        if (0 > avformat_find_stream_info(infmt_ctx, NULL))
        {
            printf("failed find stream info\n");
            avformat_close_input(&infmt_ctx);
            return -3;
        }

        stream_index = av_find_best_stream(infmt_ctx, AVMEDIA_TYPE_VIDEO, -1, -1, NULL, 0);
        if (-1 == stream_index)
        {
            printf("failed find stream\n");
            avformat_close_input(&infmt_ctx);
            return -4;
        }
        //av_dump_format(infmt_ctx, 0, "video=USB2.0 PC CAMERA", 1);
        //END输入文件

        //编码器
        AVCodec *encodec = NULL;
        // encodec = avcodec_find_encoder_by_name("libx264");
        encodec = avcodec_find_encoder_by_name("h264_nvenc");
        if (!encodec)
        {
            printf("not find encoder\n");
            avformat_close_input(&infmt_ctx);
            return -5;
        }
        AVCodecContext *encodec_ctx = NULL;
        encodec_ctx = avcodec_alloc_context3(encodec);
        if (!encodec_ctx)
        {
            printf("not alloc context3\n\n");
            avformat_close_input(&infmt_ctx);
            return -6;
        }
        int num; ///< Numerator
        int den; ///< Denominator
        encodec_ctx->bit_rate = 400000;
        encodec_ctx->width = 640;
        encodec_ctx->height = 480;
        encodec_ctx->time_base.num = 1;
        encodec_ctx->time_base.den = 25;
        encodec_ctx->framerate.num = 25;
        encodec_ctx->framerate.den = 1;
        encodec_ctx->gop_size = 10;
        encodec_ctx->max_b_frames = 0;
        encodec_ctx->pix_fmt = AV_PIX_FMT_YUV420P;

        AVDictionary *param = NULL;
        // av_dict_set(&param, "preset", "superfast", 0);
        // av_dict_set(&param, "tune", "zerolatency", 0);
        // av_dict_set(&param, "profile", "main", 0);

        if (0 > avcodec_open2(encodec_ctx, encodec, &param))
        {
            printf("failed open coder\n");
            avformat_close_input(&infmt_ctx);
            return -7;
        }

        //END编码器

        //输出文件

        if (0 > avformat_alloc_output_context2(&outfmt_ctx, nullptr, "flv", rtmp_addr))
        {
            printf("failed alloc output context\n");
            avformat_close_input(&infmt_ctx);
            return -8;
        }

        AVStream *out_stream = avformat_new_stream(outfmt_ctx, encodec_ctx->codec);
        if (!out_stream)
        {
            printf("failed new stream\n");
            avformat_close_input(&infmt_ctx);
            avformat_close_input(&outfmt_ctx);
            return -9;
        }
        avcodec_copy_context(out_stream->codec, encodec_ctx);
        //out_stream->codecpar->codec_tag = 0;
        if (0 > avio_open(&outfmt_ctx->pb, rtmp_addr, AVIO_FLAG_WRITE))
        {
            printf("failed to open outfile\n");
            avformat_close_input(&infmt_ctx);
            avformat_close_input(&outfmt_ctx);
            return -10;
        }

        av_dump_format(outfmt_ctx, 0, rtmp_addr, 1);

        if (0 > avformat_write_header(outfmt_ctx, NULL))
        {
            printf("failed to write header\n");
            avio_close(outfmt_ctx->pb);
            avformat_close_input(&infmt_ctx);
            avformat_close_input(&outfmt_ctx);
            return -11;
        }
        //END输出文件

        struct SwsContext *sws_ctx = sws_getContext(infmt_ctx->streams[stream_index]->codec->width, infmt_ctx->streams[stream_index]->codec->height,
                                                    infmt_ctx->streams[stream_index]->codec->pix_fmt, 640, 480, AV_PIX_FMT_YUV420P,
                                                    SWS_BILINEAR, NULL, NULL, NULL);
        int src_bufsize = av_image_alloc(src_data, src_linesize, infmt_ctx->streams[stream_index]->codec->width, infmt_ctx->streams[stream_index]->codec->height, infmt_ctx->streams[stream_index]->codec->pix_fmt, 16);
        int dst_bufsize = av_image_alloc(dst_data, dst_linesize, 640, 480, AV_PIX_FMT_YUV420P, 1);

        AVFrame *outFrame = av_frame_alloc();
        int picture_size = avpicture_get_size(encodec_ctx->pix_fmt, encodec_ctx->width, encodec_ctx->height);
        unsigned char *picture_buf = (uint8_t *)av_malloc(picture_size);
        avpicture_fill((AVPicture *)outFrame, picture_buf, encodec_ctx->pix_fmt, encodec_ctx->width, encodec_ctx->height);
        outFrame->format = encodec_ctx->pix_fmt;
        outFrame->width = encodec_ctx->width;
        outFrame->height = encodec_ctx->height;

        int y_size = encodec_ctx->width * encodec_ctx->height;
        AVPacket outpkt;
        av_new_packet(&outpkt, picture_size);

        int loop = 0;
        int got_picture = -1;
        int delayedFrame = 0;
        while (1)
        {
            av_read_frame(infmt_ctx, &packet);
            if (packet.stream_index == stream_index)
            {
                memcpy(src_data[0], packet.data, packet.size);
                sws_scale(sws_ctx, src_data, src_linesize, 0, infmt_ctx->streams[stream_index]->codec->height, dst_data, dst_linesize);
                outFrame->data[0] = dst_data[0];
                outFrame->data[1] = dst_data[0] + y_size;
                outFrame->data[2] = dst_data[0] + y_size * 5 / 4;
                outFrame->pts = loop;
                loop++;
                encode(encodec_ctx, &outpkt, outFrame, outfmt_ctx);
            }
        }
        encode(encodec_ctx, &outpkt, NULL, outfmt_ctx);
        av_write_trailer(outfmt_ctx);
        av_free(outFrame);
        av_free(picture_buf);
        avio_close(outfmt_ctx->pb);
        avformat_close_input(&infmt_ctx);
        avformat_close_input(&outfmt_ctx);
        return 0;
    }
    int rtmp_video2()
    {
        avdevice_register_all();
        captureFrame();
        return 0;
    }
}