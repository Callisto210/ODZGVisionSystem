#ifndef __CODEC_MODULE_H
#define __CODEC_MODULE_H

#include <gst/gst.h>
#include <gst/gstbin.h>
#include <jsmn/jsmn.h>
#include <string.h>
#include "mux.h"
#include "sink.h"

gboolean autoplug_continue_cb(GstBin *bin, GstPad *pad,
                              GstCaps *caps, gpointer user_data);


/* Structure to contain all our information, so we can pass it to callbacks */
typedef struct _elements {
    GstElement *pipeline;
    GstElement *src;
    GstElement *decode;

    GstElement *aconvert;
    GstElement *vconvert;
    GstElement *acodec;
    GstElement *vcodec;
    GstElement *aqueue;
    GstElement *vqueue;

    GstElement* sink;
    GstElement* muxer;
} Elements;


int elements_has_null_field(Elements* elements);

int test_pipeline();

int magic(Elements data, e_sink_t sink_type, e_mux_t mux_type);

void pad_added_handler (GstElement *src, GstPad *new_pad, Elements *data);

void configure_pipeline(const char* json);
#endif
