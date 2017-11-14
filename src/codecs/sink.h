//
// Created by misiek on 03.11.17.
//

#ifndef ODZGVISIONSYSTEM_SINK_H
#define ODZGVISIONSYSTEM_SINK_H

#include <stdlib.h>
#include <string.h>

typedef enum _e_sink_t {
    HLS_SINK,
    TCP_SINK,
    FILE_SINK,
    ICECAST
} e_sink_t;

char* get_sink_str(e_sink_t sink);

#endif //ODZGVISIONSYSTEM_SINK_H
