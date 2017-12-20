cmake_minimum_required(VERSION 3.5.1)

file(GLOB MAIN codec_modules.cc)
file(GLOB REST rest/*.cc)
file(GLOB CODECS_CXX codecs/*.cc)
file(GLOB CODECS_C codecs/*.c)

message(STATUS "Main files: ${MAIN}")
message(STATUS "Rest files: ${REST}")

include_directories(codecs)
include_directories(rest)
include_directories(${CMAKE_CURRENT_LIST_DIR})
add_executable(Main ${MAIN} ${REST} ${CODECS_C} ${CODECS_CXX} app_config.cc app_config.hh)

target_link_libraries(Main
        ${pistache}
        pthread 
        ${GST_PKG_LDFLAGS}
        ${jsmn}
	gstpbutils-1.0
        )
install(TARGETS Main RUNTIME DESTINATION bin)
