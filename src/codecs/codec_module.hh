#ifndef __CODEC_MODULE_H
#define __CODEC_MODULE_H

extern "C" {
#include <gst/gst.h>
#include <gst/gstbin.h>
#include <jsmn/jsmn.h>
#include <string.h>
#include "mux.h"
#include "sink.h"
}


#include <string>

gboolean autoplug_continue_cb(GstBin *bin, GstPad *pad,
                              GstCaps *caps, gpointer user_data);
using namespace std;

struct config_struct {
	int audio_bitrate;
	int video_bitrate;
	int fps;
	int width;
	int height;
	int port;
	string random;
	string uri;
	string sink;
	string acodec;
	string vcodec;
	string audio_stream;
	string video_stream;
	string host;
	string location;
	string state;
};

/* Structure to contain all our information, so we can pass it to callbacks */
typedef struct _elements {
    GstElement *pipeline;
    GstElement *decode;

    GstElement *aconvert;
    GstElement *vconvert;
    GstElement *acodec;
    GstElement *vcodec;
    GstElement *aqueue;
    GstElement *vqueue;

    GstElement* sink;
    GstElement* muxer;
    void *ptr;
} Elements;


int elements_has_null_field(Elements* elements);

int test_pipeline();

int magic(Elements data, e_mux_t mux_type, config_struct conf);

void pad_added_handler (GstElement *src, GstPad *new_pad, Elements *data);
#endif
