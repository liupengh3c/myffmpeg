#include <stdio.h>
#include <string.h>
#include <iostream>
#include <math.h>
#include "dream.h"
#define __STDC_CONSTANT_MACROS
#define __STDC_FORMAT_MACROS
extern "C"
{
    #include "libavformat/avformat.h"
}
int strToInt(char* p)
{
	int length = strlen(p);
	int val = 0;
	for (size_t i = 0; i < length; i++)
	{
		val += (p[i] - 0x30) * pow(10, length - 1 - i);
	}
	return val;
}
int main (int argc, char **argv)
{
    int ret = 0;
    char input[5] = {};
	int number = 0;

    std::string msg = "\n\nAll the funtions are:\n\
	1. print ffmpeg informations.\n\
	2. demux mp4 to h264+aac/dts,you should input the mp4 path.\n\
	3. decode h264 to yuv420p.\n";
	while (true)
	{
		std::cout << msg << std::endl;
		std::cout << "please select the number:";
		std::cin >> input;
		number = strToInt(input);
		switch (number)
		{
            case 1:
            {
                std::cout << avformat_configuration() << std::endl;
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
			default:
				break;
		}
	}

    return  0;
}

