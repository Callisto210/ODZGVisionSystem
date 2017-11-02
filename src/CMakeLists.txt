cmake_minimum_required(VERSION 3.5.1)

file(GLOB MAIN codec_module.cc)
file(GLOB REST rest/*.cc)

message(STATUS "Main files: ${MAIN}")
message(STATUS "Rest files: ${REST}")

include_directories(codecs)
add_executable(Main ${MAIN} ${REST} codecs/mux.c codecs/sink.c
        codecs/codec_module.c codecs/decodebin_cb.c)

target_link_libraries(Main
        ${pistache}
        pthread 
        ${GST_PKG_LDFLAGS}
        ${jsmn}
        )
install(TARGETS Main RUNTIME DESTINATION bin)
