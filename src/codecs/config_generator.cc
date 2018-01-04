//
// Created by misiek on 03.11.17.
//

#include "config_generator.hh"
#include <map>
#include "rapidjson/stringbuffer.h"
#include "rapidjson/document.h"
#include "rapidjson/writer.h"
#include "rest/endpoints.hh"
using std::string;
using std::map;
using namespace rapidjson;



void no_more_pads_cb (GstElement *e, Elements *ptr);
gboolean bus_watch_get_stream (GstBus* bus, GstMessage *msg, GstElement *pipeline);

static std::shared_ptr<spdlog::logger> log_config = std::shared_ptr<spdlog::logger>();
static map<string, string> acodec_map = {
        {"opus" , "opusenc"},
        {"aac", "voaacenc"},
        {"lame" ,"lamemp3enc"},
	{"vorbis", "vorbisenc"}
};

static map<string, string> vcodec_map = {
        {"vp8", "vp8enc"},
        {"vp9", "vp9enc"},
        {"h264", "x264enc"},
	{"theora", "theoraenc"}
};

static map <string, string> sink_map = {
        {"file", "filesink"},
	{"udp", "udpsink"},
	{"icecast", "shout2send"}
};

static map <string, string> mux_map = {
	{"mp4", "mp4mux"},
	{"mpegts", "mpegtsmux"},
	{"webm", "webmmux"},
	{"ogg", "oggmux"}
};

static void zero_elements(Elements& e) {
    memset(&e, 0, sizeof(Elements));
}

void configure_pipeline(Elements &e, config_struct *conf)
{
    if(log_config == nullptr) {
        log_config = spdlog::get("config");
    }
    log_config->debug("uri {}, acodec: {}, vcodec: {}, sink: {}",
                      conf->uri, conf->acodec, conf->vcodec, conf->sink);

    static GstElement* audio_last = nullptr;
    static GstElement* video_last = nullptr;
    static GstElement* optional = nullptr;
    static bool configured = false;

    if(!configured) {
        configured = !configured;
 	memset(&e, 0, sizeof(Elements));
    }

    string acodec_gst = acodec_map[conf->acodec];
    string vcodec_gst = vcodec_map[conf->vcodec];
    string sink_gst = sink_map[conf->sink];
    string mux_gst = mux_map[conf->mux];

    log_config->debug("Elements opts: acodec: {} vcodec: {} sink: {}",
        acodec_gst, vcodec_gst, sink_gst);


    e.pipeline = gst_pipeline_new(conf->random.c_str());

    if (!acodec_gst.empty()) {
	    e.aconvert = gst_element_factory_make("audioconvert", "aconvert");
	    e.aqueue = gst_element_factory_make("queue", "aqueue");
	    audio_last = e.aconvert;
	    gst_bin_add_many(GST_BIN(e.pipeline), e.aconvert, e.aqueue, NULL);
    }
   
    if (!vcodec_gst.empty()) {
	    e.vconvert = gst_element_factory_make("videoconvert", "vconvert");
	    e.vqueue = gst_element_factory_make("queue", "vqueue");
	    video_last = e.vconvert;
	    gst_bin_add_many(GST_BIN(e.pipeline),
			     e.vconvert,
			     e.vqueue,
			     NULL);
    }

    e.decode = gst_element_factory_make("uridecodebin", "source");
    gst_bin_add(GST_BIN(e.pipeline), e.decode);
    
    g_object_set (e.decode, "uri", conf->uri.c_str(), nullptr);



    if (!vcodec_gst.empty()) {
	    if(conf->fps != -1) {
		optional = gst_element_factory_make("videorate", "fps");
		if (optional != NULL) {
			gst_bin_add(GST_BIN(e.pipeline), optional);
			g_object_set(optional, "max-rate", conf->fps, NULL);
			if (gst_element_link(video_last, optional)) {
				video_last = optional;
			}
		}
	    }

	    if(conf->width != -1 && conf->height !=-1) {
		optional = gst_element_factory_make("videoscale", "scale");
		if (optional != NULL) {
			gst_bin_add(GST_BIN(e.pipeline), optional);
			GstCaps * caps = gst_caps_new_simple ("video/x-raw",
			    "width", G_TYPE_INT, conf->width,
			    "height", G_TYPE_INT, conf->height,
			    NULL);
			gst_pad_set_caps(gst_element_get_static_pad (optional, "src"), caps);
			gst_caps_unref(caps);
			if (gst_element_link(video_last, optional)) {
				video_last = optional;
			}
		}
	    }
    }

    if (!acodec_gst.empty()) {
	    e.acodec = gst_element_factory_make(acodec_gst.c_str(), "acodec");
	    if (e.acodec != nullptr) {
		if (conf->audio_bitrate != -1) {
		//lamemp3enc takes bitrate in kbit/s
			if (strncmp("lamemp3enc", acodec_gst.c_str(), 10) == 0)
				g_object_set(e.acodec, "bitrate", (conf->audio_bitrate/8)*8, NULL);
			else	
				g_object_set(e.acodec, "bitrate", ((conf->audio_bitrate*1000)/8)*8, NULL);
		}
		gst_bin_add(GST_BIN(e.pipeline), e.acodec);
		gst_element_link_many (audio_last, e.acodec, e.aqueue, NULL);
	    } else {
			log_config->error("Can't find audio codec");
		}
    }

    if (!vcodec_gst.empty()) {
	    e.vcodec = gst_element_factory_make(vcodec_gst.c_str(), "vcodec");
	    if (e.vcodec != nullptr) {
		if(strncmp("vp8enc", vcodec_gst.c_str(), 6) == 0) {
			g_object_set(e.vcodec, "threads", 6, NULL);
			if (conf->video_bitrate != -1)
				g_object_set(e.vcodec, "target-bitrate", conf->video_bitrate*1000, NULL);
		}	
		if(strncmp("vp9enc", vcodec_gst.c_str(), 6) == 0) {
			g_object_set(e.vcodec, "threads", 6, NULL);
			if (conf->video_bitrate != -1)
				g_object_set(e.vcodec, "target-bitrate", conf->video_bitrate*1000, NULL);
		}
		gst_bin_add(GST_BIN(e.pipeline), e.vcodec);
		gst_element_link_many (video_last, e.vcodec, e.vqueue, NULL);
	    } else {
			log_config->error("Can't find video codec");
		}

    }

    if (!mux_gst.empty()) { 
	e.muxer = gst_element_factory_make(mux_gst.c_str(), "muxer");
    	if (e.muxer != nullptr) {
	    if (strncmp("mp4mux", mux_gst.c_str(), 6) == 0) {
		g_object_set (e.muxer, "fragment-duration", 100, NULL);
	    }
	    gst_bin_add(GST_BIN(e.pipeline), e.muxer);
	} else {
		log_config->error("Can't find muxer");
	}
    }

    if (!sink_gst.empty()) {
    	e.sink = gst_element_factory_make(sink_gst.c_str(), "sink");
	if (e.sink != nullptr) {
	    if (strncmp("filesink", sink_gst.c_str(), 4) == 0) {
	    	if(!conf->location.empty())
			g_object_set (e.sink, "location", conf->location.c_str(), NULL);
	    }
	    if (strncmp("udpsink", sink_gst.c_str(), 3) == 0) {
	    	if(!conf->host.empty())
			g_object_set (e.sink, "host", conf->host.c_str(), NULL);
		if(conf->port != -1)
			g_object_set (e.sink, "port", conf->port, NULL);
	    }
	    if (strncmp("shout2send", sink_gst.c_str(), 7) == 0) {
		g_object_set (e.sink, "ip", "127.0.0.1", NULL);
		g_object_set (e.sink, "port", 8000, NULL);
		g_object_set (e.sink, "password", "ala123", NULL);
		if (!conf->random.empty()) {
			string loc("/");
			loc += conf->random;
			loc += ".webm";
			g_object_set (e.sink, "mount", loc.c_str(), NULL);
		}
	    }
	    gst_bin_add(GST_BIN(e.pipeline), e.sink);
	    gst_element_link(e.muxer, e.sink);
	}
    }
}
