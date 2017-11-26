//
// Created by misiek on 03.11.17.
//

#ifndef ODZGVISIONSYSTEM_CONFIG_GENERATOR_HH
#define ODZGVISIONSYSTEM_CONFIG_GENERATOR_HH

#include <string>
#include <vector>
#include <spdlog/spdlog.h>
#ifdef __cplusplus
extern "C" {
#endif

#include "codec_module.h"
#ifdef __cplusplus
};

#endif
using std::string;

#include "rapidjson/stringbuffer.h"
#include "rapidjson/document.h"
#include "rapidjson/writer.h"
#include <pistache/http.h>
#include <gst/pbutils/pbutils.h>
struct config_struct {
	int audio_bitrate;
	int video_bitrate;
	int fps;
	int width;
	int height;
	string source;
	string sink;
	string acodec;
	string vcodec;
	string path;
};


void configure_pipeline(Elements &e, Pistache::Http::ResponseWriter &resp, config_struct conf);
//static void print_tag_foreach (const GstTagList *tags, const gchar *tag, gpointer data);
//static void print_stream_info (GstDiscovererStreamInfo *info, gint depth,pads_struct *data);
//static void print_topology (GstDiscovererStreamInfo *info, gint depth,pads_struct *data);
//static void on_discovered_cb (GstDiscoverer *discoverer, GstDiscovererInfo *info, GError *err, pads_struct *data);
//static void on_finished_cb (GstDiscoverer *discoverer, pads_struct *data);
#endif //ODZGVISIONSYSTEM_CONFIG_GENERATOR_HH
