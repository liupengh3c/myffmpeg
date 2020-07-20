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
#ifdef __cplusplus
}
#endif
#endif
