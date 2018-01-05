cmake_minimum_required(VERSION 3.5.1)
project(ODZGVisionSystem_libraries)
include(ExternalProject)

set(MAKE /usr/bin/make)
find_package(PkgConfig)

set(CMAKE_VERBOSE_MAKEFILE on)
set(CMAKE_MODULE_PATH .)
find_package(Threads REQUIRED)
find_package(GTest REQUIRED)

set(CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/CMakeModules )
set(RAPIDJSON_USE_SSE42 ON)
set(RAPIDJSON_INCLUDEDIR /usr/include)

if(NOT CORES)
    execute_process(COMMAND getconf _NPROCESSORS_ONLN OUTPUT_VARIABLE CORES)
endif()
#Spdlog
find_path(spdlog spdlog/spdlog.h)
if(NOT spdlog)
    add_subdirectory(spdlog)
else()
    message(STATUS "Found spdlog: ${spdlog}")
endif()

# jsmn json parsing C lib
find_library(JSMN jsmn)
if(NOT JSMN)
    add_library(jsmn STATIC jsmn/jsmn.c)
    include_directories(jsmn)
    install(TARGETS jsmn ARCHIVE DESTINATION lib)
else()
    message(STATUS "Found jsmn: ${JSMN}")
endif()


#pistache
find_library(pistache pistache)
if(NOT pistache)
    add_subdirectory(pistache)
else()
    message(STATUS "Found pistache: ${pistache}")
endif()

#Gstreamer
set(ENV{PKG_CONFIG_PATH} "${CMAKE_BINARY_DIR}/pkgconfig/:$ENV{PKG_CONFIG_PATH}")
message(STATUS "PKG config path: $ENV{PKG_CONFIG_PATH}")

pkg_check_modules(GST_PKG gstreamer-1.0>=1.10)
pkg_check_modules(GST_VIDEO gstreamer-audio-1.0>=1.10 gstreamer-video-1.0>=1.10)
pkg_check_modules(GST_PLUGINS_BAD gstreamer-plugins-bad-1.0>=1.10)
# assume that there is problem with plugins, which can affect all types of plugins (good, bad, ugly)

string(REPLACE "\n" " " CORES ${CORES})
message(STATUS "Cores count: ${CORES}")

if(NOT GST_PKG_FOUND)

    message(WARNING "gstreamer-1.0>=1.12 not found")
    ExternalProject_Add(GST
            SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/gstreamer
            BINARY_DIR ${CMAKE_BINARY_DIR}/build
            INSTALL_DIR ${CMAKE_INSTALL_PREFIX}
            CONFIGURE_COMMAND ${CMAKE_CURRENT_SOURCE_DIR}/gstreamer/autogen.sh --noconfigure &&
            ${CMAKE_CURRENT_SOURCE_DIR}/gstreamer/configure
            --prefix=<INSTALL_DIR>
            --srcdir=${CMAKE_CURRENT_SOURCE_DIR}/gstreamer
            --disable-examples --disable-tests --disable-parse --disable-tools
            --disable-gtk-doc
            BUILD_COMMAND ${MAKE} -j4
            INSTALL_COMMAND ${MAKE} install -j4
            )

    ExternalProject_Add_Step(
            GST pkg
            COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_BINARY_DIR}/lib/pkgconfig/ ${CMAKE_BINARY_DIR}/pkgconfig/
            DEPENDEES install
    )
    message(STATUS "binary dir: ${CMAKE_BINARY_DIR}/lib/pkgconfig/gstreamer-1.0.pc")

endif()

if(NOT GST_VIDEO_FOUND)
    message(WARNING "gstreamer-audio-1.0>=1.12 and gstreamer-video-1.0>=1.12 not found")
    ExternalProject_Add(GST_BASE
            DEPENDS GST
            SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/gst-plugins-base
            BINARY_DIR ${CMAKE_BINARY_DIR}/build
            INSTALL_DIR ${CMAKE_INSTALL_PREFIX}
            CONFIGURE_COMMAND ${CMAKE_CURRENT_SOURCE_DIR}/gst-plugins-base/autogen.sh --noconfigure &&
            ${CMAKE_CURRENT_SOURCE_DIR}/gst-plugins-base/configure
            --srcdir=${CMAKE_CURRENT_SOURCE_DIR}/gst-plugins-base
            --disable-examples --disable-tests
            --with-pkg-config-path=${CMAKE_BINARY_DIR}/pkgconfig
            --disable-gtk-doc
            --prefix=<INSTALL_DIR>
            BUILD_COMMAND ${MAKE} -j4
            INSTALL_COMMAND ${MAKE}  install -j4
            )

    ExternalProject_Add_Step(
            GST_BASE pkg
            COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_BINARY_DIR}/lib/pkgconfig ${CMAKE_BINARY_DIR}/pkgconfig/
            DEPENDEES install
    )
endif()

if(NOT GST_PLUGINS_BAD_FOUND)
    message(STATUS "GStreamer plugins good")
    ExternalProject_Add(GST_GOOD
            SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/gst-plugins-good
            BINARY_DIR ${CMAKE_BINARY_DIR}/build
            INSTALL_DIR ${CMAKE_INSTALL_PREFIX}
            CONFIGURE_COMMAND ${CMAKE_CURRENT_SOURCE_DIR}/gst-plugins-good/autogen.sh --noconfigure &&
            ${CMAKE_CURRENT_SOURCE_DIR}/gst-plugins-good/configure
            --srcdir=${CMAKE_CURRENT_SOURCE_DIR}/gst-plugins-good
            --disable-examples --disable-tests
            --with-pkg-config-path=${CMAKE_BINARY_DIR}/pkgconfig
            --disable-gtk-doc
            --prefix=<INSTALL_DIR>
            BUILD_COMMAND ${MAKE} -j4
            INSTALL_COMMAND ${MAKE}  install -j4
            )

    ExternalProject_Add_Step(
            GST_GOOD pkg
            COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_BINARY_DIR}/lib/pkgconfig ${CMAKE_BINARY_DIR}/pkgconfig/
            DEPENDEES install
    )

    message(STATUS "GStreamer plugins bad")
    ExternalProject_Add(GST_BAD
            SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/gst-plugins-bad
            BINARY_DIR ${CMAKE_BINARY_DIR}/build
            INSTALL_DIR ${CMAKE_INSTALL_PREFIX}
            CONFIGURE_COMMAND ${CMAKE_CURRENT_SOURCE_DIR}/gst-plugins-bad/autogen.sh --noconfigure &&
            ${CMAKE_CURRENT_SOURCE_DIR}/gst-plugins-bad/configure
            --srcdir=${CMAKE_CURRENT_SOURCE_DIR}/gst-plugins-bad
            --disable-examples --disable-tests
            --with-pkg-config-path=${CMAKE_BINARY_DIR}/pkgconfig
            --disable-gtk-doc
            --prefix=<INSTALL_DIR>
            BUILD_COMMAND ${MAKE} -j4
            INSTALL_COMMAND ${MAKE}  install -j4
            )

    ExternalProject_Add_Step(
            GST_BAD pkg
            COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_BINARY_DIR}/lib/pkgconfig ${CMAKE_BINARY_DIR}/pkgconfig/
            DEPENDEES install
    )


    message(STATUS "GStreamer plugins ugly")
    ExternalProject_Add(GST_UGLY
            SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/gst-plugins-ugly
            BINARY_DIR ${CMAKE_BINARY_DIR}/build
            INSTALL_DIR ${CMAKE_INSTALL_PREFIX}
            CONFIGURE_COMMAND ${CMAKE_CURRENT_SOURCE_DIR}/gst-plugins-ugly/autogen.sh --noconfigure &&
            ${CMAKE_CURRENT_SOURCE_DIR}/gst-plugins-ugly/configure
            --srcdir=${CMAKE_CURRENT_SOURCE_DIR}/gst-plugins-ugly
            --disable-examples --disable-tests
            --with-pkg-config-path=${CMAKE_BINARY_DIR}/pkgconfig
            --disable-gtk-doc
            --prefix=<INSTALL_DIR>
            BUILD_COMMAND ${MAKE} -j4
            INSTALL_COMMAND ${MAKE}  install -j4
            )

    ExternalProject_Add_Step(
            GST_UGLY pkg
            COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_BINARY_DIR}/lib/pkgconfig ${CMAKE_BINARY_DIR}/pkgconfig/
            DEPENDEES install
    )
endif()




install(DIRECTORY ${CMAKE_BINARY_DIR}/pkgconfig DESTINATION lib)

