cmake_minimum_required(VERSION 3.5.1)
project(ODZGVisionSystem)

set(CMAKE_VERBOSE_MAKEFILE off)
set(CMAKE_MODULE_PATH .)
find_package(Threads REQUIRED)
find_package(GTest REQUIRED)

set(CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/CMakeModules )
set(RAPIDJSON_USE_SSE42 ON)
set(RAPIDJSON_INCLUDEDIR /usr/include)


find_package(PkgConfig)
set(PKG_CONFIG_USE_CMAKE_PREFIX_PATH ON)

option(PISTACHE_PATH "")
option(JSMN_PATH "")
option(PKG_PATH "")
if(PKG_PATH)
    set(ENV{PKG_CONFIG_PATH} "${PKG_PATH}/pkgconfig/:$ENV{PKG_CONFIG_PATH}")
endif()
message(STATUS "PKG config path: $ENV{PKG_CONFIG_PATH}")
pkg_check_modules(GST_PKG gstreamer-1.0>=1.10 gstreamer-audio-1.0>=1.10 gstreamer-video-1.0>=1.10 gstreamer-pbutils-1.0>=1.10)

include(CheckIncludeFiles)
include_directories(include)
include_directories(lib)
include_directories(${GST_PKG_INCLUDE_DIRS})

find_library(pistache pistache HINTS lib ${PISTACHE_PATH})
if(pistache)
    message(STATUS "Found: pistache ${pistache}")
endif()

find_library(jsmn jsmn HINTS lib ${JSMN_PATH})
if(jsmn)
    message(STATUS "Found jsmn: ${jsmn}")
endif()

find_library(GST gstreamer-1.0>=1.10 )
find_package(RapidJSON)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_SOURCE_FILE_EXTENSIONS cc;hh)

install(DIRECTORY data/html DESTINATION /var/www/html/odzg)
add_subdirectory(src)

