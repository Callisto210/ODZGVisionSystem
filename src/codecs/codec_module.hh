#ifndef __CODEC_MODULE_H
#define __CODEC_MODULE_H

extern "C" {
#include <gst/gst.h>
#include <gst/gstbin.h>
#include "mux.h"
#include "sink.h"
}
#include <jsmn/jsmn.h>
#include <string.h>
#include <string>

gboolean autoplug_continue_cb(GstBin *bin, GstPad *pad,
                              GstCaps *caps, gpointer user_data);
using namespace std;

struct pip_config_struct {
	int x;
	int y;
	int pip_width;
	int pip_height;
	string pip_stream;
};

struct audio_config_struct {
	int audio_bitrate;
	string acodec;
	string audio_stream;
};

struct video_config_struct {
	int video_bitrate;
	int fps;
	int width;
	int height;
	string vcodec;
	string video_stream;
	int n_pip;
	struct pip_config_struct pip[10];
};

struct config_struct {
	int port;
	string date;
	string time;
	string random;
	int n_uri;
	string uri[10];
	string sink;
	string mux;
	string host;
	string location;
	string state;
	int n_audio;
	int n_video;
	struct audio_config_struct audio[10];
	struct video_config_struct video[10];
};

/* Structure to contain all our information, so we can pass it to callbacks */
typedef struct _audio_elements {
    GstElement *aconvert;
    GstElement *acodec;
    GstElement *aqueue;
    audio_config_struct *ptr;
} Audio_Elements;

typedef struct _video_elements {
    GstElement *vconvert;
    GstElement *vcodec;
    GstElement *vqueue;
    video_config_struct *ptr;
} Video_Elements;

typedef struct _elements {
    GstElement *pipeline;
    GstElement *decode[10];
    int n_decode;

    int n_audio;
    int n_video;
    Audio_Elements audio[10];
    Video_Elements video[10];

    GstElement* sink;
    GstElement* muxer;
    void *ptr;
} Elements;


int elements_has_null_field(Elements* elements);

int test_pipeline();

int magic(Elements data, config_struct conf);

void pad_added_handler (GstElement *src, GstPad *new_pad, Elements *data);
#endif
