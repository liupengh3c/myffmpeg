
#include "iostream"
#define SAMPLE_RATE 48000
#define SAMPLE_FMT AV_SAMPLE_FMT_S16
#define CH_LAYOUT AV_CH_LAYOUT_STERO
extern "C"
{
#include "libavcodec/avcodec.h"
#include "libavformat/avformat.h"

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
        FILE *f_in = NULL;
        FILE *f_out = NULL;

        AVCodec *codec = NULL;
        AVCodecContext *enc_ctx = NULL;

        AVFrame *frame = NULL;
        AVPacket *pkt = NULL;

        // 1. find encoder
        codec = avcodec_find_encoder(AV_CODEC_ID_AAC);
        if (!codec)
        {
            std::cout << "find encoder error" << std::endl;
            return -1;
        }
        enc_ctx = avcodec_alloc_context3(codec);
        if (!codec)
        {
            std::cout << "avcodec_alloc_context3 error" << std::endl;
            return -2;
        }
        enc_ctx->sample_rate = SAMPLE_RATE;
    }
}