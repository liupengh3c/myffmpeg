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

    struct SDL_Info
    {
        //SDL---------------------------
        SDL_Window *screen;
        SDL_Renderer *sdlRenderer;
        SDL_Texture *sdlTexture;
        SDL_Rect sdlRect;
    };

    static int encode(AVCodecContext *enc_ctx, AVPacket *pkt, AVFrame *frame, AVFormatContext *fmt, FILE *f)
    {
        int ret = 0;
        if (frame)
        {
            std::cout << "frame->linesize[0] = " << frame->linesize[0] << std::endl;
            std::cout << "frame->linesize[1] = " << frame->linesize[1] << std::endl;
            std::cout << "frame->linesize[2] = " << frame->linesize[2] << std::endl;
        }

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
            fwrite(pkt->data, 1, pkt->size, f);
            pkt->stream_index = 0;
            av_interleaved_write_frame(fmt, pkt);
            av_packet_unref(pkt);
        }
        return 0;
    }
    int rtmp_video(std::string flv)
    {
        int ret = 0;
        int video_index = -1;
        int64_t start_time = 0;
        int dst_width = 640, dst_height = 480;

        AVFormatContext *fmt_ctx = NULL;
        AVInputFormat *fmt = NULL;

        AVFormatContext *ofmt_ctx = NULL;
        AVStream *istream = NULL, *ostream = NULL;

        AVPacket *pkt = NULL;

        FILE *f_out = NULL;

        AVFrame *frame = NULL, *yuv_frame = NULL;
        struct SwsContext *sws_ctx = NULL;
        struct SDL_Info display;

        const char *rtmp_server = "rtmp://106.13.42.53/liupeng/video";
        AVCodecContext *enc_ctx = NULL;
        AVCodec *codec = NULL;

        f_out = fopen(flv.data(), "wb+");
        pkt = av_packet_alloc();
        frame = av_frame_alloc();
        yuv_frame = av_frame_alloc();

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

        // 为输出文件分配avformat_context
        avformat_alloc_output_context2(&ofmt_ctx, NULL, "flv", rtmp_server);
        if (!ofmt_ctx)
        {
            std::cout << "avformat_alloc_output_context2 for output file error,ret=" << std::endl;
            return -4;
        }
        // 打开输出文件
        ret = avio_open(&ofmt_ctx->pb, rtmp_server, AVIO_FLAG_WRITE);
        if (ret < 0)
        {
            std::cout << "open output file error,ret=" << ret << std::endl;
            return -5;
        }

        codec = avcodec_find_encoder_by_name("h264_nvenc");
        if (!codec)
        {
            std::cout << "avcodec_find_encoder_by_name h264_nvenc fail" << std::endl;
            return -7;
        }

        enc_ctx = avcodec_alloc_context3(codec);
        if (!enc_ctx)
        {
            std::cout << "avcodec_alloc_context3 fail" << std::endl;
            return -8;
        }
        enc_ctx->width = dst_width;
        enc_ctx->height = dst_height;
        enc_ctx->pix_fmt = AV_PIX_FMT_YUV420P;
        enc_ctx->framerate = {25, 1};
        enc_ctx->time_base = {1, 25};
        enc_ctx->bit_rate = 4000000;
        // 5. 获取视频流索引
        for (size_t i = 0; i < fmt_ctx->nb_streams; i++)
        {
            istream = fmt_ctx->streams[i];
            ostream = avformat_new_stream(ofmt_ctx, NULL);
            if (istream->codecpar->codec_type == AVMEDIA_TYPE_VIDEO)
            {
                video_index = i;
                avcodec_parameters_from_context(ostream->codecpar, enc_ctx);
                break;
            }
        }

        if (video_index == -1)
        {
            std::cout << "can not find video index" << std::endl;
            return -9;
        }
        std::cout << "-----------------------------------\n"
                  << std::endl;
        av_dump_format(ofmt_ctx, 0, rtmp_server, 1);
        // 写文件头
        ret = avformat_write_header(ofmt_ctx, NULL);
        if (ret < 0)
        {
            std::cout << "avformat_write_header to file error,ret=" << ret << std::endl;
            return -6;
        }
        // 8. 打开解码器
        ret = avcodec_open2(enc_ctx, codec, NULL);
        if (ret < 0)
        {
            std::cout << "avcodec_open2 fail,ret=" << ret << std::endl;
            return -10;
        }

        // 9. 初始化转换器
        sws_ctx = sws_getContext(istream->codecpar->width, istream->codecpar->height, AVPixelFormat(istream->codecpar->format), dst_width, dst_height, AV_PIX_FMT_YUV420P, SWS_BILINEAR, NULL, NULL, NULL);
        if (!sws_ctx)
        {
            std::cout << "sws_getContext fail,ret=" << ret << std::endl;
            return -11;
        }

        yuv_frame->width = dst_width;
        yuv_frame->height = dst_height;
        yuv_frame->format = AV_PIX_FMT_YUV420P;
        av_frame_get_buffer(yuv_frame, 0);

        frame->width = istream->codecpar->width;
        frame->height = istream->codecpar->height;
        frame->format = istream->codecpar->format;
        av_frame_get_buffer(frame, 0);

        // SDL初始化
        SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO);
        display.screen = SDL_CreateWindow("get yuv data from webcam", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
                                          dst_width, dst_height,
                                          SDL_WINDOW_OPENGL);
        if (!display.screen)
        {
            std::cout << "sdl create window fail" << std::endl;
            return -9;
        }

        display.sdlRenderer = SDL_CreateRenderer(display.screen, -1, 0);
        display.sdlTexture = SDL_CreateTexture(display.sdlRenderer, SDL_PIXELFORMAT_IYUV, SDL_TEXTUREACCESS_STREAMING, dst_width, dst_height);

        display.sdlRect.x = 0;
        display.sdlRect.y = 0;
        display.sdlRect.w = dst_width;
        display.sdlRect.h = dst_height;

        AVPacket *opkt = NULL;
        opkt = av_packet_alloc();
        start_time = av_gettime();

        while (true)
        {
            ret = av_read_frame(fmt_ctx, pkt);
            if (ret < 0)
            {
                break;
            }
            memcpy(frame->data[0], pkt->data, pkt->size);
            sws_scale(sws_ctx, frame->data, frame->linesize, 0, frame->height, yuv_frame->data, yuv_frame->linesize);
            encode(enc_ctx, opkt, yuv_frame, ofmt_ctx, f_out);
            // 设置纹理数据
            SDL_UpdateYUVTexture(display.sdlTexture, &display.sdlRect,
                                 yuv_frame->data[0], yuv_frame->linesize[0],
                                 yuv_frame->data[1], yuv_frame->linesize[1],
                                 yuv_frame->data[2], yuv_frame->linesize[2]);
            // SDL_RenderClear(sdlRenderer);
            // 纹理复制给渲染器
            SDL_RenderCopy(display.sdlRenderer, display.sdlTexture, NULL, &display.sdlRect);
            // 显示
            SDL_RenderPresent(display.sdlRenderer);
            if (av_gettime() - start_time >= 30000000)
            {
                break;
            }
            AVRational time_base = istream->time_base;
            AVRational time_base_q = {1, AV_TIME_BASE};
            int64_t pts_time = av_rescale_q(pkt->pts, time_base, time_base_q);
            int64_t now_time = av_gettime() - start_time;
            if (pts_time > now_time)
                av_usleep(pts_time - now_time);

            std::cout << "pts_time=" << pts_time << std::endl;
            std::cout << "pkt->dts=" << pkt->dts << std::endl;
            std::cout << "pkt->pts=" << pkt->pts << std::endl;
            std::cout << "av_gettime() - start_time=" << av_gettime() - start_time << std::endl;
            std::cout << "istream->time_base.den=" << istream->time_base.den << std::endl;
            std::cout << "istream->time_base.num=" << istream->time_base.num << std::endl;
        }

        encode(enc_ctx, opkt, NULL, ofmt_ctx, f_out);
        av_write_trailer(ofmt_ctx);

        sws_freeContext(sws_ctx);

        av_packet_free(&pkt);
        av_frame_free(&frame);
        av_frame_free(&yuv_frame);
        avcodec_free_context(&enc_ctx);
        avformat_close_input(&fmt_ctx);

        avio_close(ofmt_ctx->pb);
        avformat_free_context(ofmt_ctx);
        fclose(f_out);

        SDL_Quit();
        return 0;
    }
}