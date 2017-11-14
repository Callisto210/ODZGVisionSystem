#ifndef ODZGVISIONSYSTEM_MUX_H
#define ODZGVISIONSYSTEM_MUX_H

#include <stddef.h>
#include <string.h>
#include <stdlib.h>

typedef enum _e_mux_t {
    MP4_MUX,
    OGG_MUX,
    MPEG_TS_MUX,
    WEBM_MUX
} e_mux_t;

char* get_mux_str(e_mux_t mux);

#endif //ODZGVISIONSYSTEM_MUX_H
