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
    GstElement *playbin;
    GstElement *src;
    GstElement *decode;
    GstElement *vbin;
    GstElement *abin;
    GstElement *aconvert;
    GstElement *vconvert;
    GstElement *acodec;
    GstElement *vcodec;
    GstElement *aqueue;
    GstElement *vqueue;
    GstElement* intervideosink;
    GstElement* interaudiosink;
    GstElement* intervideosrc;
    GstElement* interaudiosrc;
    GstElement* sink;
    GstElement* muxer;
    GMainLoop *main_loop;
    gint n_video;          /* Number of embedded video streams */
    gint n_audio;          /* Number of embedded audio streams */
    gint n_text;           /* Number of embedded subtitle streams */

    gint current_video;    /* Currently playing video stream */
    gint current_audio;    /* Currently playing audio stream */
    gint current_text;     /* Currently playing subtitle stream */
} Elements;

typedef enum {
    GST_PLAY_FLAG_VIDEO         = (1 << 0), /* We want video output */
    GST_PLAY_FLAG_AUDIO         = (1 << 1), /* We want audio output */
    GST_PLAY_FLAG_TEXT          = (1 << 2)  /* We want subtitle output */
} GstPlayFlags;

int elements_has_null_field(Elements* elements);

int magic(Elements data, e_sink_t sink_type, e_mux_t mux_type);

void pad_added_handler (GstElement *src, GstPad *new_pad, Elements *data);

static void analyze_streams (Elements *data);

static gboolean handle_message (GstBus *bus, GstMessage *msg, Elements *data);
static gboolean handle_message2 (GstBus *bus, GstMessage *msg, Elements *data);

void configure_pipeline(const char* json);
#endif
