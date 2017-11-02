//
// Created by misiek on 03.11.17.
//

#include <lzma.h>
#include "sink.h"

char *get_sink_str(e_sink_t sink) {
    const size_t sink_str_size = 20;
    char* sink_str = NULL;
    sink_str = (char*) calloc(sink_str_size, sizeof(char));
    switch(sink) {
        case HLS_SINK:
            sink_str = strncpy(sink_str, "hlssink", sink_str_size);
            break;
        case TCP_SINK:
            sink_str = strncpy(sink_str, "tcpsink", sink_str_size);
            break;
        case FILE_SINK:
            sink_str = strncpy(sink_str, "filesink", sink_str_size);
            break;
    }
    return sink_str;
}
