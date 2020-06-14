#include <stdio.h>
#define __STDC_CONSTANT_MACROS
#define __STDC_FORMAT_MACROS
extern "C" {
	#include "libavcodec/avcodec.h"
	#include "libavformat/avformat.h"
	#include "libavfilter/avfilter.h"
}


int main(int argc, char **argv)
{
	char *info = (char *)malloc(40000);
	memset(info, 0, 40000);
	sprintf(info, "%s\n", avcodec_configuration());
	printf(info);
	return 0;
}

