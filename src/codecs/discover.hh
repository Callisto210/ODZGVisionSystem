#ifndef ODZGVISIONSYSTEM_DISCOVER_HH
#define ODZGVISIONSYSTEM_DISCOVER_HH


#include <string>
#include <vector>
#include <spdlog/spdlog.h>
#include "rapidjson/stringbuffer.h"
#include "rapidjson/document.h"
#include "rapidjson/writer.h"
#include <pistache/http.h>
#include <gst/pbutils/pbutils.h>


struct pads_struct {
    GstDiscoverer *discoverer;
    GMainLoop *loop;
    Pistache::Http::ResponseWriter *response;
    rapidjson::Document *doc;
    rapidjson::Document::AllocatorType *alloc;
    rapidjson::Value *audio;
    rapidjson::Value *video;
    rapidjson::Value *obj;
    int videon;
    int audion;
};
void discover_uri(Pistache::Http::ResponseWriter &resp, std::string source, std::string uri);
static void print_tag_foreach (const GstTagList *tags, const gchar *tag, gpointer data);
static void print_stream_info (GstDiscovererStreamInfo *info, gint depth,pads_struct *data);
static void print_topology (GstDiscovererStreamInfo *info, gint depth,pads_struct *data);
static void on_discovered_cb (GstDiscoverer *discoverer, GstDiscovererInfo *info, GError *err, pads_struct *data);
static void on_finished_cb (GstDiscoverer *discoverer, pads_struct *data);
#endif