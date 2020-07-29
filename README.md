# myffmpeg
ffmpeg应用，系统：Ubuntu18.04
# 使用方法
1. 将代码clone到本地
2. 进入到build目录
3. 运行build.sh 脚本：sh build.sh进行编译
4. 运行，./myffmpeg

# 运行
liupeng@baidu-bj-liupeng:~/ffmpeg/myffmpeg/build$ ./myffmpeg


All the funtions are:  
	1. print ffmpeg informations.  
	2. demux mp4 to h264+aac/dts,you should input the mp4 path.  
	3. decode h264 to yuv420p(av_parser_parser2).  
	4. decode h264/mp4 to yuv420p(av_read_frame).  
	5. decode aac to pcm(av_parser_parser2).  
	6. decode aac/mp4 to pcm(av_read_frame).  
	7. demux and decode mp4 to pcm + yuv420p.  
	8. encode yuv420p to h264(fwrite).  
	9. encode yuv420p to h264(av_interleaved_write_frame).  
	10. encode pcm to aac.  
	11. get webcam video to yuv420p.  
    12. press 'q' for quit applation.  

please select the number:  
