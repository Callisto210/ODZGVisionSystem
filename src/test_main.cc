/*
 * =====================================================================================
 *
 *       Filename:  test_main.cc
 *
 *    Description:  
 *
 *        Version:  1.0
 *        Created:  26.10.2017 23:37:10
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  Michal Zagorski, <mzagorsk@student.agh.edu.pl>
 *   Organization:  AGH University of Science and Technology I@IET
 *
 * =====================================================================================
 */
#include <iostream>
#include "test_main.hh"
#include <spdlog/spdlog.h>
#if (__cplusplus)
extern "C" {
#endif
#include <gst/gst.h>
#if (__cplusplus)
}
#endif

using std::string;
//using namespace Net;
namespace spd = spdlog;

class StreamData {

public:
        GstElement* element;

        gint video_streams_count;
        gint audio_streams_count;

        gint subtitle_text_streams_count;

        gint current_video;
        gint current_audio;
        gint current_text;

        GMainLoop *main_loop;
        /*  Methods */

};


enum GstPlayFlags {
    GST_PLAY_FLAG_VIDEO = (1 << 0),
    GST_PLAY_FLAG_AUDIO = (1 << 1),
    GST_PLAY_FLAG_TEXT  = (1 << 2)
};
static gboolean handle_message (GstBus* bus, GstMessage* msg, StreamData* stream_data);
static gboolean handle_keyboard(GIOChannel* source, GIOCondition cond, StreamData* stream_data);
void play()
{

    auto console = spd::stdout_color_mt("gst_bin");

    console->info("Application started... main.");

    StreamData data;
    GstBus* bus;
    GstStateChangeReturn ret;

    gint flags;
    GIOChannel* io_stdin;
try{
    /* Init gstreamer */
    gst_init(0, NULL);

    data.element = gst_element_factory_make("playbin", "playbin");
    
    if (!data.element) {
        g_printerr("Not all elements could be created.\n");
        console->error("Data.elements could not be loaded. Fail.");
        throw string("Create elements");
    }

    g_object_set(data.element, "uri", "https://www.freedesktop.org/software/gstreamer-sdk/data/media/sintel_cropped_multilingual.webm", NULL);


    g_object_get(data.element, "flags",&flags, NULL);
    console->debug("Got flags from stream... {}", flags);
    flags |= GST_PLAY_FLAG_VIDEO | GST_PLAY_FLAG_AUDIO;
    flags &= ~GST_PLAY_FLAG_TEXT;

    g_object_set(data.element, "flags", flags, NULL);


    /*  Set connection speed. This will affect some internal decisions of element */
    g_object_set (data.element, "connection-speed", 56, NULL);

    bus = gst_element_get_bus(data.element);
    gst_bus_add_watch(bus, (GstBusFunc)handle_message, &data);
    

    io_stdin = g_io_channel_unix_new(fileno(stdin));

    g_io_add_watch(io_stdin, G_IO_IN, (GIOFunc)handle_keyboard, &data);


    console->info("Start playing the video");
    ret = gst_element_set_state(data.element, GST_STATE_PLAYING);

    if (ret == GST_STATE_CHANGE_FAILURE) {
            g_printerr("Unable to set the pipeline to the playing state.\n");
            console->error("Set state to playing failed.");
            gst_object_unref(data.element);
            throw string("Playing video");
    }

    data.main_loop = g_main_loop_new(NULL, FALSE);

    g_main_loop_run(data.main_loop);


    } catch(string& err) {
        console->error(err);
    }
    g_main_loop_unref (data.main_loop);
    g_io_channel_unref (io_stdin);
    gst_object_unref (bus);
    gst_element_set_state (data.element, GST_STATE_NULL);
    gst_object_unref (data.element);

//    return 0;
}

/* Extract some metadata from the streams and print it on the screen */
static void analyze_streams (StreamData *data) {
  gint i;
  GstTagList *tags;
  gchar *str;
  guint rate;

  /* Read some properties */
  g_object_get (data->element, "n-video", &data->video_streams_count, NULL);
  g_object_get (data->element, "n-audio", &data->audio_streams_count, NULL);
  g_object_get (data->element, "n-text", &data->subtitle_text_streams_count, NULL);

  g_print ("%d video stream(s), %d audio stream(s), %d text stream(s)\n",
    data->video_streams_count, data->audio_streams_count, data->subtitle_text_streams_count);

  g_print ("\n");
  for (i = 0; i < data->video_streams_count; i++) {
    tags = NULL;
    /* Retrieve the stream's video tags */
    g_signal_emit_by_name (data->element, "get-video-tags", i, &tags);
    if (tags) {
      g_print ("video stream %d:\n", i);
      gst_tag_list_get_string (tags, GST_TAG_VIDEO_CODEC, &str);
      g_print ("  codec: %s\n", str ? str : "unknown");
      g_free (str);
      gst_tag_list_free (tags);
    }
  }

  g_print ("\n");
  for (i = 0; i < data->audio_streams_count; i++) {
    tags = NULL;
    /* Retrieve the stream's audio tags */
    g_signal_emit_by_name (data->element, "get-audio-tags", i, &tags);
    if (tags) {
      g_print ("audio stream %d:\n", i);
      if (gst_tag_list_get_string (tags, GST_TAG_AUDIO_CODEC, &str)) {
        g_print ("  codec: %s\n", str);
        g_free (str);
      }
      if (gst_tag_list_get_string (tags, GST_TAG_LANGUAGE_CODE, &str)) {
        g_print ("  language: %s\n", str);
        g_free (str);
      }
      if (gst_tag_list_get_uint (tags, GST_TAG_BITRATE, &rate)) {
        g_print ("  bitrate: %d\n", rate);
      }
      gst_tag_list_free (tags);
    }
  }

  g_print ("\n");
  for (i = 0; i < data->subtitle_text_streams_count; i++) {
    tags = NULL;
    /* Retrieve the stream's subtitle tags */
    g_signal_emit_by_name (data->element, "get-text-tags", i, &tags);
    if (tags) {
      g_print ("subtitle stream %d:\n", i);
      if (gst_tag_list_get_string (tags, GST_TAG_LANGUAGE_CODE, &str)) {
        g_print ("  language: %s\n", str);
        g_free (str);
      }
      gst_tag_list_free (tags);
    }
  }

  g_object_get (data->element, "current-video", &data->current_video, NULL);
  g_object_get (data->element, "current-audio", &data->current_audio, NULL);
  g_object_get (data->element, "current-text", &data->current_text, NULL);

  g_print ("\n");
  g_print ("Currently playing video stream %d, audio stream %d and text stream %d\n",
    data->current_video, data->current_audio, data->current_text);
  g_print ("Type any number and hit ENTER to select a different audio stream\n");
}



/* Process messages from GStreamer */
static gboolean handle_message (GstBus *bus, GstMessage *msg, StreamData *data) {
  GError *err;
  gchar *debug_info;

  switch (GST_MESSAGE_TYPE (msg)) {
    case GST_MESSAGE_ERROR:
      gst_message_parse_error (msg, &err, &debug_info);
      g_printerr ("Error received from element %s: %s\n", GST_OBJECT_NAME (msg->src), err->message);
      g_printerr ("Debugging information: %s\n", debug_info ? debug_info : "none");
      g_clear_error (&err);
      g_free (debug_info);
      g_main_loop_quit (data->main_loop);
      break;
    case GST_MESSAGE_EOS:
      g_print ("End-Of-Stream reached.\n");
      g_main_loop_quit (data->main_loop);
      break;
    case GST_MESSAGE_STATE_CHANGED: {
      GstState old_state, new_state, pending_state;
      gst_message_parse_state_changed (msg, &old_state, &new_state, &pending_state);
      if (GST_MESSAGE_SRC (msg) == GST_OBJECT (data->element)) {
        if (new_state == GST_STATE_PLAYING) {
          /* Once we are in the playing state, analyze the streams */
          analyze_streams (data);
        }
      }
    } break;
  }

  /* We want to keep receiving messages */
  return TRUE;
}

/* Process keyboard input */
static gboolean handle_keyboard (GIOChannel *source, GIOCondition cond, StreamData *data) {
  gchar *str = NULL;

  if (g_io_channel_read_line (source, &str, NULL, NULL, NULL) == G_IO_STATUS_NORMAL) {
    int index = g_ascii_strtoull (str, NULL, 0);
    if (index < 0 || index >= data->audio_streams_count) {
      g_printerr ("Index out of bounds\n");
    } else {
      /* If the input was a valid audio stream index, set the current audio stream */
      g_print ("Setting current audio stream to %d\n", index);
      g_object_set (data->element, "current-audio", index, NULL);
    }
  }
  g_free (str);
  return TRUE;
}
