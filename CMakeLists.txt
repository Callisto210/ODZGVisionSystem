cmake_minimum_required(VERSION 3.5.1)
project(odzg)
include(ExternalProject)
set(CMAKE_CXX_COMPILER /usr/bin/c++)

set(CMAKE_MODULE_PATH .)
find_package (Threads)

set(CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/CMakeModules )
set(RAPIDJSON_USE_SSE42 ON)
set(RAPIDJSON_INCLUDEDIR /usr/include)
set(GTEST_LIBRARY /usr/include)
find_library(pistache lib/pistache/src)
find_library(spdlog lib/spdlog)
find_library(ffmpeg lib/ffmpeg)
include(${CMAKE_ROOT}/Modules/ExternalProject.cmake)
include_directories(include)
include_directories(/usr/lib/include)
#set(FFMPEG_LIBRARIES)
find_package(FFmpeg REQUIRED)
find_package(RapidJSON)
#
#set(CMAKE_MODULE_PATH lib/pistache/src)
#find_package(Pistache REQUIRED)
add_subdirectory(lib/pistache)
set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_SOURCE_FILE_EXTENSIONS cc;hh)
#ExternalProject_Add(pistache
#        SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/lib/pistache
#        BINARY_DIR build/pistache
#
#        )

#add_dependencies(libpistache pistache)


add_executable(Main src/rest/endpoints.cc
        src/main.cc
        src/ffmpeg/AVFilter.cc
        src/codecs/codecs.cc
        src/codecs/filter_graph.cc src/ffmpeg/AVCodec.cc src/ffmpeg/AVCodec.hh src/ffmpeg/AVPacket.cc src/ffmpeg/AVPacket.hh src/codecs/main.h)



target_link_libraries(Main avformat
        avutil
        avfilter
        avresample
        postproc
        swresample
        swscale
        pthread
        avdevice
        avcodec)