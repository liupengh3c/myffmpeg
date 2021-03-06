/*
	将mp4文件解封装为h264+aac
*/
#define __STDC_CONSTANT_MACROS
#include "iostream"
extern "C"
{
#include "libavformat/avformat.h"

    int demux(const char *filename)
    {
        int ret;
        int video_index= -1, audio_index = -1;

        std::string h264_file = "";
        std::string aac_file = "";

        AVFormatContext* ifmt_ctx = NULL,*video_fmt_ctx=NULL,*audio_fmt_ctx=NULL;
        AVStream* in_stream = NULL,*out_stream = NULL;
        AVPacket avpacket;

        h264_file.append(filename).append(".h264");
        //aac_file.append(filename).append(".aac");

        // 1、打开mp4文件,并读取文件header
        avformat_open_input(&ifmt_ctx, filename, NULL, NULL);

        // 2、读取流信息
        avformat_find_stream_info(ifmt_ctx, NULL);
        av_dump_format(ifmt_ctx, 0, filename, 0);

        // 3、h264文件分配avformatcontext
        ret = avformat_alloc_output_context2(&video_fmt_ctx, NULL, NULL, h264_file.data());

        // 4、aac文件分配avformatcontext
        //ret = avformat_alloc_output_context2(&audio_fmt_ctx, NULL, NULL, aac_file.data());

        // 5、分别为h264、aac文件添加流
        for (size_t i = 0; i < ifmt_ctx->nb_streams; i++)
        {
            in_stream = ifmt_ctx->streams[i];
            if (in_stream->codecpar->codec_type == AVMEDIA_TYPE_VIDEO)
            {
                out_stream = avformat_new_stream(video_fmt_ctx, NULL);
                video_index = i;
            }
            if (in_stream->codecpar->codec_type == AVMEDIA_TYPE_AUDIO)
            {
                if (in_stream->codecpar->codec_id == AV_CODEC_ID_DTS)
                {
                    aac_file.append(filename).append(".dts");
                }
                else
                {
                    aac_file.append(filename).append(".aac");
                }
                avformat_alloc_output_context2(&audio_fmt_ctx, NULL, NULL, aac_file.data());
                out_stream = avformat_new_stream(audio_fmt_ctx, NULL);
                audio_index = i;
            }
            if (out_stream)
            {
                // 拷贝编码参数
                avcodec_parameters_copy(out_stream->codecpar, in_stream->codecpar);
            }
        }

        // 打印输出文件信息
        std::cout << "--------------------------------------------------" << std::endl;
        av_dump_format(video_fmt_ctx, 0, h264_file.data(), 1);
        av_dump_format(audio_fmt_ctx, 0, aac_file.data(), 1);

        // 6、打开视频文件
        ret = avio_open(&video_fmt_ctx->pb, h264_file.data(), AVIO_FLAG_WRITE);
        if (ret < 0)
        {
            std::cout << "create and init avio context for access h264 file error,err=" << ret;
            return -1;
        }

        // 7、打开音频文件
        ret = avio_open(&audio_fmt_ctx->pb, aac_file.data(), AVIO_FLAG_WRITE);
        if (ret < 0)
        {
            std::cout << "create and init avio context for access aac file error,err=" << ret;
            return -2;
        }

        // 8、视频文件写header
        ret = avformat_write_header(video_fmt_ctx, NULL);
        if (ret < 0)
        {
            std::cout << "write h264 file header error,err=" << ret;
            return -3;
        }

        // 9、音频文件写header
        ret = avformat_write_header(audio_fmt_ctx, NULL);
        if (ret < 0)
        {
            std::cout << "write aac file header error,err=" << ret;
            return -4;
        }

        // 10、开始写内容
        while (true)
        {
            ret = av_read_frame(ifmt_ctx, &avpacket);
            if (ret < 0)
            {
                break;
            }
            if (avpacket.stream_index == video_index)
            {
                avpacket.stream_index = 0;
                av_interleaved_write_frame(video_fmt_ctx, &avpacket);
            }
            else if (avpacket.stream_index == audio_index)
            {
                avpacket.stream_index = 0;
                av_interleaved_write_frame(audio_fmt_ctx, &avpacket);
            }
            av_packet_unref(&avpacket);
        }

        // 11、写文件尾
        av_write_trailer(video_fmt_ctx);
        av_write_trailer(audio_fmt_ctx);

        // 12、关闭输入、输出文件,释放资源
        avformat_close_input(&ifmt_ctx);
        avio_close(video_fmt_ctx->pb);
        avio_close(audio_fmt_ctx->pb);

        avformat_free_context(video_fmt_ctx);
        avformat_free_context(audio_fmt_ctx);
        return 0;
    }
}