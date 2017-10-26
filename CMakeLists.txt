cmake_minimum_required(VERSION 3.5.1)
project(ODZGVisionSystem)

set(CMAKE_VERBOSE_MAKEFILE on)
set(CMAKE_MODULE_PATH .)
find_package(Threads REQUIRED)
find_package(GTest REQUIRED)
set(CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/CMakeModules )
set(RAPIDJSON_USE_SSE42 ON)
set(RAPIDJSON_INCLUDEDIR /usr/include)


include(ExternalProject)
set(MAKE /usr/bin/make)


find_library(pistache libpistache PATHS lib/pistache/src)
find_library(spdlog libspdlog PATHS lib/spdlog)
include(${CMAKE_ROOT}/Modules/ExternalProject.cmake)
include_directories(include)
include_directories(include/gstreamer-1.0)

find_package(RapidJSON)

add_subdirectory(lib/pistache)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_SOURCE_FILE_EXTENSIONS cc;hh)
#file(GLOB CFILES src/codecs/*c )
# set_source_files_properties(${CFILES} PROPERTIES LANGUAGE C)
#add_library(internal STATIC ${CFILES})

#set_target_properties(internal PROPERTIES LANGUAGE C)
#set(GST NOTFOUND)

find_package(PkgConfig REQUIRED)
#pkg_search_module(GST gstreamer-1.0>=1.0 gstreamer-audio-1.0 gstreamer-video-1.0)


add_executable(Main
        src/rest/endpoints.cc
        src/codecs/main.c
        src/decodebin_cb.c)

#if(GST-NOTFOUND)
ExternalProject_Add(GST
        SOURCE_DIR ${CMAKE_CURRENT_LIST_DIR}/lib/gstreamer
        TMP_DIR ${CMAKE_CURRENT_LIST_DIR}/build/gst
        INSTALL_DIR ${CMAKE_CURRENT_LIST_DIR}/bin/gst
        CONFIGURE_COMMAND cd ${CMAKE_CURRENT_LIST_DIR}/build/gst && ${CMAKE_CURRENT_LIST_DIR}/lib/gstreamer/autogen.sh --noconfigure
        && ${CMAKE_CURRENT_LIST_DIR}/lib/gstreamer/configure
        --srcdir=${CMAKE_CURRENT_LIST_DIR}/lib/gstreamer
        --libdir=${CMAKE_CURRENT_LIST_DIR}/bin/gst
        --bindir=${CMAKE_CURRENT_LIST_DIR}/bin/gst/bin
        --includedir=${CMAKE_CURRENT_LIST_DIR}/include
        --disable-examples --disable-tests --disable-parse --disable-tools
        --prefix=${CMAKE_CURRENT_LIST_DIR}/bin
        --with-PACKAGE=gstreamer-audio-1.0
        --with-PACKAGE=gstreamer-video-1.0
        --with-PACKAGE=gstreamer-1.0
        BUILD_COMMAND ${MAKE} -C ${CMAKE_CURRENT_LIST_DIR}/build/gst -j9 all
        INSTALL_COMMAND ${MAKE} -C ${CMAKE_CURRENT_LIST_DIR}/build/gst -j9 install)

find_library(GST gstreamer libgstreamer PATHS
        ${CMAKE_CURRENT_LIST_DIR}/lib/gstreamer
        ${CMAKE_CURRENT_LIST_DIR}/bin/gst)

message(STATUS "Lib gstreamer path: ${GST}")
#endif()

find_library(JSMN jsmn PATHS lib/jsmn/)
#add_dependencies(Main GST)
add_dependencies(Main pistache)
#add_dependencies(Main internal)


target_link_libraries(Main
        pistache
        pthread
        ${JSMN}
        )