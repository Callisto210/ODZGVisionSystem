cmake_minimum_required(VERSION 3.5.1)

file(GLOB MAIN test_main.cc)
file(GLOB REST rest/*.cc)

message(STATUS "Main files: ${MAIN}")
message(STATUS "Rest files: ${REST}")


add_executable(Main ${MAIN} test_main.hh main.cc ${REST})

target_link_libraries(Main
        ${pistache}
        pthread 
        ${GST_PKG_LDFLAGS}
        ${jsmn}
        )
install(TARGETS Main RUNTIME DESTINATION bin)