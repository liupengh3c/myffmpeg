#include "iostream"
#ifndef DREAM_H
#define DREAM_H
#ifdef __cplusplus
extern "C"
{
#endif
    int demux(const char *filename);
    int decode_video(std::string input_filename, std::string output_filename);
    int decode_video2(std::string input_filename, std::string output_filename);
    int decode_audio(std::string input_filename, std::string output_filename);
    int decode_audio2(std::string input_filename, std::string output_filename);
    int demux_decode(std::string input_file, std::string video_file, std::string audio_file);
    int encode_video(std::string input_file, std::string output_file);
    int encode_video2(std::string input_file, std::string output_file);
    int encode_audio(std::string input_file, std::string output_file);
<<<<<<< HEAD
    int filter_logo(char *in_yuv, char *out_yuv);
=======
    int video(std::string yuv);
    int rtmp_video(std::string flv);
    int encode_rgba(std::string input_file, std::string output_file);
    int get_encode_video(std::string flv);
    int rtmp_video2();
>>>>>>> 05ddaed216f0cb1b0d68c6fc8871687c5bd65194
#ifdef __cplusplus
}
#endif
#endif
