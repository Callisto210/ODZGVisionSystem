//
// Created by misiek on 03.11.17.
//

#include "config_generator.hh"
#include <map>
#include "rapidjson/stringbuffer.h"
#include "rapidjson/document.h"
#include "rapidjson/writer.h"
#include <endpoints.hh>

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

/* This function transforms audio configuration into usable part of pipeline */
static void configure_audio(Elements &e, audio_config_struct *conf) {

	string acodec_gst = acodec_map[conf->acodec];
  	GstElement* audio_last = nullptr;

	if (acodec_gst.empty()) {
		log_config->error("Something goes wrong around acodec\n");
		return;
	}

	/* Set convert */ 
	    e.audio[e.n_audio].aconvert = gst_element_factory_make("audioconvert", NULL);
	    e.audio[e.n_audio].aqueue = gst_element_factory_make("queue", NULL);
	    audio_last = e.audio[e.n_audio].aconvert;
	    gst_bin_add_many(GST_BIN(e.pipeline), e.audio[e.n_audio].aconvert, e.audio[e.n_audio].aqueue, NULL);

	/* Create audio encoder element */
	    e.audio[e.n_audio].acodec = gst_element_factory_make(acodec_gst.c_str(), NULL);
	    if (e.audio[e.n_audio].acodec != nullptr) {
		if (conf->audio_bitrate != -1) {
		//lamemp3enc takes bitrate in kbit/s
			if (strncmp("lamemp3enc", acodec_gst.c_str(), 10) == 0)
				g_object_set(e.audio[e.n_audio].acodec, "bitrate", (conf->audio_bitrate/8)*8, NULL);
			else	
				g_object_set(e.audio[e.n_audio].acodec, "bitrate", ((conf->audio_bitrate*1000)/8)*8, NULL);
		}
		gst_bin_add(GST_BIN(e.pipeline), e.audio[e.n_audio].acodec);
		gst_element_link_many (audio_last, e.audio[e.n_audio].acodec, e.audio[e.n_audio].aqueue, NULL);
		} else {
			log_config->error("Can't find audio codec\n");
		}

	e.audio[e.n_audio].ptr = conf;
	e.n_audio++;
}

/* This function transforms video configuration into usable part of pipeline */
static int configure_video(Elements &e, video_config_struct *conf) {

	string vcodec_gst = vcodec_map[conf->vcodec];
	GstElement* video_last = nullptr;
	GstElement* optional = nullptr;
	int place;

	/* Get some place */
	place = e.n_video++;

	/* Set convert */
	    e.video[place].vconvert = gst_element_factory_make("videoconvert", NULL);
	    e.video[place].vqueue = gst_element_factory_make("queue", NULL);
	    video_last = e.video[place].vconvert;
	    gst_bin_add_many(GST_BIN(e.pipeline),
			     e.video[place].vconvert,
			     e.video[place].vqueue,
			     NULL);

	/* Set fps & scale */
	    if(conf->fps != -1) {
		optional = gst_element_factory_make("videorate", NULL);
		if (optional != NULL) {
			gst_bin_add(GST_BIN(e.pipeline), optional);
			g_object_set(optional, "max-rate", conf->fps, NULL);
			if (gst_element_link(video_last, optional)) {
				video_last = optional;
			}
		}
	    }

	    if(conf->width != -1 && conf->height !=-1) {
		optional = gst_element_factory_make("videoscale", NULL);
		if (optional != NULL) {
			gst_bin_add(GST_BIN(e.pipeline), optional);
			if (gst_element_link(video_last, optional)) {
				video_last = optional;
			}
		}
		optional = gst_element_factory_make("capsfilter", NULL);
		if (optional != NULL) {
			gst_bin_add(GST_BIN(e.pipeline), optional);
			GstCaps * caps = gst_caps_new_simple ("video/x-raw",
			    "width", G_TYPE_INT, conf->width,
			    "height", G_TYPE_INT, conf->height,
			    NULL);
			g_object_set(optional, "caps", caps, NULL);
			gst_caps_unref(caps);
			if (gst_element_link(video_last, optional)) {
				video_last = optional;
			}
		}
	    }
	
	/* Handle PIP */
	if (conf->x != -1 && conf->y != -1 &&
	    conf->pip_width != -1 && conf->pip_height != -1 &&
	    !conf->pip_stream.empty()) {
		GstPad *comp_m_in, *comp_p_in;
		GstPad *main_out, *pip_out;
		optional = gst_element_factory_make("compositor", NULL);
		if (optional != NULL) {
			gst_bin_add(GST_BIN(e.pipeline), optional);
			/* Add main image */
			comp_m_in = gst_element_get_request_pad(optional, "sink_%u");
			main_out = gst_element_get_static_pad(video_last, "src");
			if (gst_pad_link (main_out, comp_m_in) != GST_PAD_LINK_OK)
				g_printerr ("PIP, Main image, failed\n");

			/* Add Pip image */
			video_config_struct *pip_conf_str = new video_config_struct;
			pip_conf_str->video_stream = conf->pip_stream;
			pip_conf_str->fps = -1;
			pip_conf_str->width = -1;
			pip_conf_str->height = -1;
			pip_conf_str->x = -1;
			pip_conf_str->y = -1;
			pip_conf_str->pip_width = -1;
			int pip_idx = configure_video(e, pip_conf_str);
			comp_p_in = gst_element_get_request_pad(optional, "sink_%u");
			pip_out = gst_element_get_static_pad(e.video[pip_idx].vconvert, "src");

			/* Set parameters */
			g_object_set(comp_p_in, "xpos", conf->x, NULL);
			g_object_set(comp_p_in, "ypos", conf->y, NULL);
			g_object_set(comp_p_in, "width", conf->pip_width, NULL);
			g_object_set(comp_p_in, "height", conf->pip_height, NULL);
			
			/* And pray it will work */
			if (gst_pad_link (pip_out, comp_p_in) != GST_PAD_LINK_OK)
				g_printerr ("PIP, PIP image, failed\n");

			video_last = optional;
		}
	}

	/* Create encoding element */
	if (!vcodec_gst.empty()) {
	    e.video[place].vcodec = gst_element_factory_make(vcodec_gst.c_str(), NULL);
	    if (e.video[place].vcodec != nullptr) {
		if(strncmp("vp8enc", vcodec_gst.c_str(), 6) == 0) {
			g_object_set(e.video[place].vcodec, "threads", 6, NULL);
			if (conf->video_bitrate != -1)
				g_object_set(e.video[place].vcodec, "target-bitrate", conf->video_bitrate*1000, NULL);
		}	
		if(strncmp("vp9enc", vcodec_gst.c_str(), 6) == 0) {
			g_object_set(e.video[place].vcodec, "threads", 6, NULL);
			if (conf->video_bitrate != -1)
				g_object_set(e.video[place].vcodec, "target-bitrate", conf->video_bitrate*1000, NULL);
		}
		gst_bin_add(GST_BIN(e.pipeline), e.video[place].vcodec);
		gst_element_link_many (video_last, e.video[place].vcodec, e.video[place].vqueue, NULL);
	    } else {
			log_config->error("Can't find video codec");
	    }
	}
	e.video[place].ptr = conf;
	return (place);

}

void configure_pipeline(Elements &e, config_struct *conf)
{
    if(log_config == nullptr) {
        log_config = spdlog::get("config");
    }

    static bool configured = false;

    if(!configured) {
        configured = !configured;
 	memset(&e, 0, sizeof(Elements));
    }

    string sink_gst = sink_map[conf->sink];
    string mux_gst = mux_map[conf->mux];

    GstElementFactory* vaapi_factory;
    vaapi_factory = gst_element_factory_find("bcmdec");
    gst_plugin_feature_set_rank(GST_PLUGIN_FEATURE(vaapi_factory), GST_RANK_NONE);
    
    GstElementFactory* vdp_factory;
    vaapi_factory = gst_element_factory_find("vdpdecoder");
    gst_plugin_feature_set_rank(GST_PLUGIN_FEATURE(vdp_factory), GST_RANK_NONE);

    e.pipeline = gst_pipeline_new(conf->random.c_str());

    GstRegistry* reg = gst_registry_get();

    GstPluginFeature* vaapi_decode = gst_registry_lookup_feature(reg, "bcmdec");
    gst_plugin_feature_set_rank(vaapi_decode, GST_RANK_NONE);
    gst_object_unref(vaapi_decode);

    GstPluginFeature* vdp_decode = gst_registry_lookup_feature(reg, "vdpdecoder");
    gst_plugin_feature_set_rank(vdp_decode, GST_RANK_NONE);
    gst_object_unref(vdp_decode);

    e.decode = gst_element_factory_make("uridecodebin", "source");
    gst_bin_add(GST_BIN(e.pipeline), e.decode);
    
    g_object_set (e.decode, "uri", conf->uri.c_str(), nullptr);

    e.n_audio = 0;
    e.n_video = 0;

    for (int i=0; i < conf->n_audio; i++)
    	configure_audio(e, &conf->audio[i]);
    
    for (int i=0; i < conf->n_video; i++) 
    	configure_video(e, &conf->video[i]);

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
