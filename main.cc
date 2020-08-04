#include <stdio.h>
#include <string.h>
#include <iostream>
#include <math.h>
#include "dream.h"
#define __STDC_CONSTANT_MACROS
#define __STDC_FORMAT_MACROS
extern "C"
{
#include "libavcodec/avcodec.h"
}
int strToInt(char *p)
{
    int length = strlen(p);
    int val = 0;
    for (size_t i = 0; i < length; i++)
    {
        val += (p[i] - 0x30) * pow(10, length - 1 - i);
    }
    return val;
}
int main(int argc, char **argv)
{
    int ret = 0;
    char input[5] = {};
    int number = 0;

    std::string msg = "\n\nAll the funtions are:\n\
        1. print ffmpeg informations.\n\
        2. demux mp4 to h264+aac/dts,you should input the mp4 path.\n\
        3. decode h264 to yuv420p(av_parser_parser2).\n\
        4. decode h264/mp4 to yuv420p(av_read_frame).\n\
        5. decode aac to pcm(av_parser_parser2).\n\
        6. decode aac/mp4 to pcm(av_read_frame).\n\
        7. demux and decode mp4 to pcm + yuv420p.\n\
        8. encode yuv420p to h264(fwrite).\n\
        9. encode yuv420p to h264(av_interleaved_write_frame).\n\
        10. encode pcm to aac.\n\
        11. get webcam video to yuv420p.\n\
        12. get webcam video and push it to rtmp server.\n\
        13. press 'q' for quit applation.\n";
    // av_log_set_level(AV_LOG_TRACE);
    while (true)
    {
        int is_over = 0;
        std::cout << msg << std::endl;
        std::cout << "please select the number:";
        std::cin >> input;
        number = strToInt(input);
        switch (number)
        {
        case 1:
        {
            std::cout << "\n\n"
                      << avcodec_configuration() << std::endl;
            break;
        }
        case 2:
        {
            std::cout << "please input the mp4 file path:";
            std::string path;
            std::cin >> path;
            demux(path.data());
            break;
        }
        case 3:
        {
            std::cout << "please input the h264 file path:";
            std::string h264;
            std::string yuv420;
            std::cin >> h264;

            std::cout << "please input the yuv420p file path:";
            std::cin >> yuv420;
            decode_video(h264, yuv420);
            break;
        }
        case 4:
        {
            std::cout << "please input the media file path:";
            std::string h264;
            std::string yuv420;
            std::cin >> h264;

            std::cout << "please input the yuv420p file path:";
            std::cin >> yuv420;
            decode_video2(h264, yuv420);
            break;
        }
        case 5:
        {
            std::cout << "please input the aac file path:";
            std::string dts;
            std::string pcm;
            std::cin >> dts;

            std::cout << "please input the pcm file path:";
            std::cin >> pcm;
            decode_audio(dts, pcm);
            break;
        }
        case 6:
        {
            std::cout << "please input the media file path:";
            std::string aac;
            std::string pcm;
            std::cin >> aac;

            std::cout << "please input the pcm file path:";
            std::cin >> pcm;
            decode_audio2(aac, pcm);
            break;
        }
        case 7:
        {
            std::string mp4;
            std::string pcm;
            std::string yuv;

            std::cout << "please input the media file path:";
            std::cin >> mp4;

            std::cout << "please input the pcm file path:";
            std::cin >> pcm;

            std::cout << "please input the yuv file path:";
            std::cin >> yuv;
            demux_decode(mp4, yuv, pcm);
            break;
        }
        case 8:
        {
            std::cout << "please input the yuv file path:";
            std::string yuv;
            std::string h264;
            std::cin >> yuv;

            std::cout << "please input the h264 file path:";
            std::cin >> h264;
            encode_video(yuv, h264);
            break;
        }
        case 9:
        {
            std::cout << "please input the yuv file path:";
            std::string yuv;
            std::string h264;
            std::cin >> yuv;

            std::cout << "please input the h264 file path:";
            std::cin >> h264;
            encode_video2(yuv, h264);
            break;
        }
        case 10:
        {
            std::string pcm;
            std::string aac;

            std::cout << "please input the pcm file path:";
            std::cin >> pcm;

            std::cout << "please input the aac file path:";
            std::cin >> aac;
            encode_audio(pcm, aac);
            break;
        }
        case 11:
        {
            std::cout << "please input the yuv file path:";
            std::string yuv;
            std::cin >> yuv;
            video(yuv);
            break;
        }
        case 12:
        {
            std::cout << "please input the flv file path:";
            std::string flv;
            std::cin >> flv;
            rtmp_video(flv);
            break;
        }
        case 13:
        {
            std::cout << "please input the flv file path(local):";
            std::string flv;
            std::cin >> flv;
            rtmp_video2(flv);
            break;
        }
        default:
        {
            is_over = 1;
            break;
        }
        }
        if (is_over)
        {
            break;
        }
    }

    return 0;
}
