
#include "iostream"
#define SAMPLE_RATE 48000
#define SAMPLE_FMT AV_SAMPLE_FMT_S16
#define CH_LAYOUT AV_CH_LAYOUT_STERO
extern "C"
{
#include "libavcodec/avcodec.h"

    static int encode(AVCodecContext *enc_ctx, AVPacket *pkt, AVFrame *frame, FILE *outfile)
    {
        int ret = 0;
        if (frame)
        {
            std::cout << "send frame " << frame->pts << std::endl;
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
                return -2;
            }
            std::cout << "Write packet " << pkt->pts << " size=" << pkt->size << std::endl;
            fwrite(pkt->data, 1, pkt->size, outfile);
            av_packet_unref(pkt);
        }
        return 0;
    }

    int encode_audio(std::string input_file, std::string output_file)
    {
        int ret = 0;
        uint16_t *samples;
        FILE* f_in = NULL, * f_out = NULL;

        AVCodec* codec = NULL;
        AVCodecContext* enc_ctx = NULL;

        AVFrame* frame = NULL;
        AVPacket* pkt = NULL;

        f_in = fopen(input_file.data(), "rb");
        f_out = fopen(output_file.data(), "wb+");

        frame = av_frame_alloc();
        pkt = av_packet_alloc();

        // 1. find encoder
        codec = avcodec_find_encoder(AV_CODEC_ID_AAC);
        if (!codec)
        {
            std::cout << "find encoder aac error" << std::endl;
            return -1;
        }

        // 2. 分配编码上下文
        enc_ctx = avcodec_alloc_context3(codec);
        if (!enc_ctx)
        {
            std::cout << "alloc context error" << std::endl;
            return -2;
        }

        enc_ctx->bit_rate = 64000;
        enc_ctx->sample_fmt = AV_SAMPLE_FMT_S16;
        enc_ctx->sample_rate = 48000;
        enc_ctx->channel_layout = AV_CH_LAYOUT_STEREO;
        enc_ctx->channels = 2;
        enc_ctx->codec_type = AVMEDIA_TYPE_AUDIO;
        

        // 3. 打开编码器
        ret = avcodec_open2(enc_ctx, codec, NULL);
        if (ret < 0)
        {
            std::cout << "encoder open fail,ret = " << ret << std::endl;
            return -3;
        }

        // 4. 申请空间，用于存储pcm数据
        frame->format = enc_ctx->sample_fmt;
        frame->nb_samples = enc_ctx->frame_size;
        av_frame_get_buffer(frame, 0);

        // 5. 读取文件、编码
        while (!feof(f_in))
        {
            samples = (uint16_t *)frame->data[0];
            ret = fread(samples, 1, enc_ctx->frame_size * av_get_bytes_per_sample(AVSampleFormat(frame->format)), f_in);
            encode(enc_ctx, pkt, frame, f_out);
        }
        
        // 6. flush the encoder
        encode(enc_ctx, pkt, NULL, f_out);

        // 7. 释放资源
        fclose(f_in);
        fclose(f_out);
        av_frame_free(&frame);
        av_packet_free(&pkt);
        avcodec_free_context(&enc_ctx);

        return 0;
    }
}