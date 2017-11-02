//
// Created by misiek on 03.11.17.
//

#include "mux.h"

char* get_mux_str(e_mux_t mux) {
    const size_t mux_str_size = 20;
    char* mux_str = NULL;
    mux_str = (char*) calloc(mux_str_size, sizeof(char));
    switch(mux) {
        case MP4_MUX:
            mux_str = strncpy(mux_str, "mp4mux", mux_str_size);
            break;
        case MPEG_TS_MUX:
            mux_str = strncpy(mux_str, "mpegtsmux", mux_str_size);
            break;
        case OGG_MUX:
            mux_str = strncpy(mux_str, "oggmux", mux_str_size);
            break;
    }
    return mux_str;
}