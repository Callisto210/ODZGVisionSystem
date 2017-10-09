cmake_minimum_required(VERSION 3.5.1)
project(odzg)
#include(ExternalProject)

#set(CMAKE_CXX_COMPILER /usr/bin/g++-6)
set(CMAKE_VERBOSE_MAKEFILE on)
set(CMAKE_MODULE_PATH .)
find_package(Threads REQUIRED)
find_package(GTest REQUIRED)
set(CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/CMakeModules )
set(RAPIDJSON_USE_SSE42 ON)
set(RAPIDJSON_INCLUDEDIR /usr/include)



find_library(pistache libpistache PATHS lib/pistache/src)
find_library(spdlog libspdlog PATHS lib/spdlog)
include(${CMAKE_ROOT}/Modules/ExternalProject.cmake)
include_directories(include)


find_package(RapidJSON)

add_subdirectory(lib/pistache)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_SOURCE_FILE_EXTENSIONS cc;hh)
file(GLOB CFILES src/codecs/*c )
# set_source_files_properties(${CFILES} PROPERTIES LANGUAGE C)
add_library(internal STATIC ${CFILES})

set_target_properties(internal PROPERTIES LANGUAGE C)


add_executable(Main src/rest/endpoints.cc
        src/main.cc
        src/codecs/main.c
        src/codecs/filter_graph.c
        src/codecs/log_ffmpeg.c
        src/ffmpeg/AVFilter.cc
        src/codecs/codecs.cc
        src/codecs/filter_graph.cc
        src/ffmpeg/AVCodec.cc
        src/ffmpeg/AVCodec.hh
        src/ffmpeg/AVPacket.cc
        src/ffmpeg/AVPacket.hh
        src/codecs/main.h)


include(ExternalProject)
set(MAKE /usr/bin/make)

set(FF_DIR ${CMAKE_BINARY_DIR}/bin/lib/ffmpeg)

ExternalProject_Add(ffmpeg
        SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/lib/ffmpeg
        CONFIGURE_COMMAND cd ${CMAKE_CURRENT_SOURCE_DIR}/lib/ffmpeg && ./configure
        --disable-programs --enable-gpl
        --libdir=${FF_DIR}
        --shlibdir=${FF_DIR}
        --enable-shared --enable-avresample --enable-libx264 --enable-libx265 &&
        mkdir -p ${FF_DIR}
        # BUILD_COMMAND "/usr/bin/make -C ${CMAKE_CURRENT_SOURCE_DIR}/lib/ffmpeg")
        BUILD_COMMAND ${MAKE} -C ${CMAKE_SOURCE_DIR}/lib/ffmpeg -j9 all
        INSTALL_COMMAND echo "Done")

message(STATUS "FF_DIR is set as: ${FF_DIR}")

#find_library(avformat)
#link_directories(bin/lib/ffmpeg)
#find_package(FFmpeg REQUIRED)
add_dependencies(Main pistache)
add_dependencies(Main internal)
add_dependencies(Main ffmpeg)


link_directories(${FF_DIR})

#find_library(ffmpeg SHARED ffmpeg libffmpeg avutil libavutil avformat libavformat avfilter libavfilter)
#find_library(FORMAT avformat PATHS ${FF_DIR}
#        DOC "Searching AVFORMAT library"
#        NO_DEFAULT_PATH
#        NO_CMAKE_SYSTEM_PATH
#        NO_CMAKE_ENVIRONMENT_PATH
#        NO_CMAKE_PATH
#        )

if(FORMAT)
    message(STATUS "${FORMAT} format")
else()
    set(FORMAT ${FF_DIR}/libavformat.a)
    message(STATUS "Library libavformat: ${FORMAT}")
endif()
#find_library(CODEC avcodec PATHS HINTS ENV ${FF_DIR} NO_DEFAULT_PATH)
if(CODEC)
    message(STATUS "Lib avcodec found: ${CODEC} :: ${CODEC_VERSION}")
endif()
#find_library(FILTER avfilter PATHS ${FF_DIR} NO_DEFAULT_PATH)
if(FILTER)
    message(STATUS "Lib avfilter found: ${FILTER}")
endif()
#find_library(UTIL avutil PATHS ${FF_DIR})
#find_library(POSTPROC postproc PATHS ${FF_DIR})
#find_library(SWSCALE swscale PATHS ${FF_DIR})
#find_library(RESAMPLE avresample  ${FF_DIR} NO_DEFAULT_PATH)
#find_library(DEVICE avdevice  ${FF_DIR} NO_DEFAULT_PATH)

set(DEVICE ${FF_DIR}/libavdevice.so)
set(FORMAT ${FF_DIR}/libavformat.so)
set(CODEC ${FF_DIR}/libavcodec.so)
set(FILTER ${FF_DIR}/libavfilter.so)
set(UTIl ${FF_DIR}/libavutil.so)
set(POSTPROC ${FF_DIR}/libpostproc.so)
set(SWSCALE ${FF_DIR}/libswscale.so)
set(RESAMPLE ${FF_DIR}/libavresample.so)
set(SWRESAMPLE ${FF_DIR}/libswresample.so)
if(DEVICE)
    message(STATUS "Lib avdevice found: ${DEVICE}")
endif()
if(CODEC)
    message(STATUS "Lib avcodec found: ${CODEC}")
endif()

target_link_libraries(Main
        ${FORMAT}
        internal
        ${CODEC}
        ${DEVICE}
        ${FILTER}
        ${RESAMPLE}
        ${UTIL}
        ${POSTPROC}
        ${SWSCALE}
        ${SWRESAMPLE}
#        avformat
#        avfilter
#        avcodec
#        avutil
#        swresample
#        swscale
#        postproc
        pistache
        pthread
        )

install(TARGETS Main internal
        RUNTIME DESTINATION bin
        LIBRARY DESTINATION lib
        ARCHIVE DESTINATION lib)
