/*
	本函数以解码h264文件为例，解码为yuv420p
	没有使用av_read_frame函数,输入必须是只包含视频编码数据“裸流”（例如h264、HEVC码流文件）
	而不能是包含封装格式的媒体数据（例如AVI、MKV、MP4）。
*/
#define __STDC_CONSTANT_MACROS
#include "iostream"
extern "C"
{
#include "libavcodec/avcodec.h"
#include "libavformat/avformat.h"

#define INBUF_SIZE 4096
    int decode(AVCodecContext* dec_ctx, AVFrame* frame, AVPacket* pkt, FILE *f)
    {
        int ret = 0;
        ret = avcodec_send_packet(dec_ctx, pkt);
        if (ret < 0)
        {
            std::cout << "error sending a packet for decoding." << std::endl;
            return -1;
        }
        while (ret >= 0)
        {
            ret = avcodec_receive_frame(dec_ctx, frame);
            if (ret == AVERROR(EAGAIN) || ret == AVERROR_EOF)
            {
                return 0;
            }
            else if (ret < 0)
            {
                return -2;
            }
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
    int decode_video(std::string input_filename, std::string output_filename)
    {
        int ret = 0;

        AVCodec* codec = NULL;
        AVCodecContext* avcodec_ctx = NULL;
        AVPacket *pkt = NULL;
        AVFrame* frame = NULL;
        // 解析器上下文
        AVCodecParserContext* parser = NULL;

        FILE* f_in = NULL;
        FILE* f_out = NULL;

        uint8_t inbuf[INBUF_SIZE + AV_INPUT_BUFFER_PADDING_SIZE];
        uint8_t* data;
        size_t   data_size;

        // alloc an avpacket && frame
        pkt = av_packet_alloc();
        frame = av_frame_alloc();
        f_out = fopen(output_filename.data(), "wb+");

        // 1、找解码器
        codec = avcodec_find_decoder(AV_CODEC_ID_H264);
        if (!codec)
        {
            std::cout << "codec not found." << std::endl;
            return -1;
        }

        // 2、初始化parser
        parser = av_parser_init(codec->id);
        if (!parser)
        {
            std::cout << "parser not found." << std::endl;
            return -2;
        }

        // 3、分配解码上下文 alloc codec context
        avcodec_ctx = avcodec_alloc_context3(codec);
        if (!avcodec_ctx)
        {
            std::cout << "could not allocate avcodec_ctx." << std::endl;
            return -3;
        }

        // 4、打开解码器
        ret = avcodec_open2(avcodec_ctx, codec, NULL);
        if (ret < 0)
        {
            std::cout << "could not open codec,ret=" << ret << std::endl;
            return -4;
        }

        // 5、打开h264文件
        f_in = fopen(input_filename.data(), "rb");

        // 6、开始解码
        while (!feof(f_in))
        {
            data_size = fread(inbuf, 1, INBUF_SIZE, f_in);
            if (data_size <= 0)
            {
                break;
            }
            data = inbuf;
            while (data_size > 0)
            {
                // 输入必须是只包含视频编码数据“裸流”（例如H.264、HEVC码流文件），而不能是包含封装格式的媒体数据（例如AVI、MKV、MP4）。
                ret = av_parser_parse2(parser, avcodec_ctx, &pkt->data, &pkt->size, data, data_size, AV_NOPTS_VALUE, AV_NOPTS_VALUE, 0);
                std::cout << "ret=" << ret << std::endl;
                data += ret;
                data_size -= ret;
                
                std::cout << "pkt_size=" << pkt->size << std::endl;
                if (pkt->size)
                {
                    decode(avcodec_ctx, frame, pkt, f_out);
                }
            }
        }

        /* flush the decoder */
        decode(avcodec_ctx, frame, pkt, f_out);

        fclose(f_in);
        fclose(f_out);

        av_parser_close(parser);
        avcodec_free_context(&avcodec_ctx);
        av_frame_free(&frame);
        av_packet_free(&pkt);
        return 0;
    }
}