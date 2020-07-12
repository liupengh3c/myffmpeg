# CMake 最低版本号要求
cmake_minimum_required (VERSION 2.8)

# 项目信息
project (myffmpeg)
 
#去哪里找头文件 相当于-I(i的大写字母)参数
include_directories(${PROJECT_SOURCE_DIR}/include)

set(LINK_DIR ${PROJECT_SOURCE_DIR}/lib)
link_directories(${LINK_DIR})

# 查找当前目录下的所有源文件
# 并将名称保存到 DIR_SRCS 变量
aux_source_directory(. DIR_SRCS)

# 添加 lib 子目录
add_subdirectory(dream)

# 指定生成目标
add_executable(myffmpeg ${DIR_SRCS})



target_link_libraries(myffmpeg dream avdevice avfilter postproc avformat avcodec avutil stdc++ rt x264 dl vpx pthread opus mp3lame fdk-aac expat freetype z swresample swscale avutil m bz2 lzma)
