cmake_minimum_required(VERSION 3.10)
project(JT1078Server)

set(CMAKE_CXX_STANDARD 11)

set(INC_DIR /usr/include/ /usr/local/ffmpeg/include)
set(LINK_DIR /usr/lib/lib/ /usr/local/ffmpeg/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/cmake-build-debug")

include_directories(${INC_DIR})
link_directories(${LINK_DIR})
link_libraries(muduo_net muduo_base pthread avformat avutil avcodec swresample rediscluster)

add_executable(JT1078Server main.cpp Server/CRealTimeVideoServer.h Server/CRealTimeVideoServer.cpp Server/Log.h Server/Log.cpp Server/JT1078Header.h Server/CDecoder.cpp Server/CDecoder.h Server/CFFmpeg.h Server/CCodec.cpp Server/CCodec.h Codec/Adpcm.cpp Codec/G711.cpp Codec/G711.h Codec/Adpcm.h Codec/G726.cpp Codec/G726.h Configuration/CConf.cpp Configuration/CConf.h Server/Daemon.h Server/Daemon.cpp Dao/CRedisCluster.cpp Dao/CRedisCluster.h Server/CURLManager.cpp Server/CURLManager.h Flv/CConverter.cpp Flv/CConverter.h Flv/CSplitter.cpp Flv/CSplitter.h Http/CHttpContext.cpp Http/CHttpContext.h Http/CHttpRequest.cpp Http/CHttpRequest.h Http/CHttpResponse.cpp Http/CHttpResponse.h)