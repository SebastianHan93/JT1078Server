cmake_minimum_required(VERSION 3.15)
project(JT1078Server)

set(CMAKE_CXX_STANDARD 11)

set(INC_DIR /usr/include/ /usr/local/ffmpeg/include)
set(LINK_DIR /usr/lib/lib/ /usr/local/ffmpeg/lib)

include_directories(${INC_DIR})
link_directories(${LINK_DIR})
link_libraries(muduo_net muduo_base pthread avformat avutil avcodec swresample)

add_executable(JT1078Server main.cpp Server/CRealTimeVideoServer.h Server/CRealTimeVideoServer.cpp Server/Log.h Server/Log.cpp Server/JT1078Header.h Server/CDecoder.cpp Server/CDecoder.h Server/CRtmpStream.cpp Server/CRtmpStream.h Server/CFFmpeg.h Server/CCodec.cpp Server/CCodec.h Codec/Adpcm.cpp Codec/G711.cpp Codec/G711.h Codec/Adpcm.h Codec/G726.cpp Codec/G726.h)