//
// Created by misiek on 03.11.17.
//

#include "config_generator.hh"
#include <map>

using std::string;
using std::map;
static std::shared_ptr<spdlog::logger> log_config = std::shared_ptr<spdlog::logger>();
static map<string, string> acodec_map = {
        {"opus" , "opusenc"},
        {"aac", "voaacenc"},
        {"lame" ,"lamemp3enc"}
};

static map<string, string> vcodec_map = {
        {"vp8", "vp8enc"},
        {"vp9", "vp9enc"},
        {"h264", "x264enc"}
};

static map <string, string> source_map = {
        {"file", "filesrc"}
};

static void zero_elements(Elements& e) {
    memset(&e, 0, sizeof(Elements));
}

static void empty_pipeline(Elements& data, GstElement*& video_last, GstElement*& audio_last) {
    data.pipeline = gst_pipeline_new("pipeline");
    data.aconvert = gst_element_factory_make("audioconvert", "aconvert");
    data.vconvert = gst_element_factory_make("videoconvert", "vconvert");

    data.aqueue = gst_element_factory_make("queue", "aqueue");
    data.vqueue = gst_element_factory_make("queue", "vqueue");

    video_last = data.vconvert;
    audio_last = data.aconvert;

    gst_bin_add_many(GST_BIN(data.pipeline),
                     data.aconvert,
                     data.aqueue,
                     data.vconvert,
                     data.vqueue,
                     NULL);


}

Elements& configure_pipeline(string source, string path, int fps, string acodec, string vcodec)
{
    if(log_config == nullptr) {
        log_config = spdlog::get("config");
    }
    log_config->debug("Source {}, path: {}, fps: {}, acodec: {}, vcodec: {}",
                      source, path, fps, acodec, vcodec);

    static GstElement* audio_last = nullptr;
    static GstElement* video_last = nullptr;
    static Elements e;
    static bool configured = false;

    if(!configured) {
        configured = !configured;
        zero_elements(e);
    }

    string acodec_gst = acodec_map[acodec];
    string vcodec_gst = vcodec_map[vcodec];
    string source_gst = source_map[source];


    log_config->debug("Elements opts: source: {} acodec: {} vcodec: {}",
        source_gst, acodec_gst, vcodec_gst);
    gst_init(NULL, NULL);

    if(e.pipeline == NULL) {
        empty_pipeline(e, audio_last, video_last);
    }

    e.src = gst_element_factory_make(source_gst.c_str(), "filesource");
    gst_bin_add(GST_BIN(e.pipeline), e.src);
    e.decode = gst_element_factory_make("decodebin", "source");
    gst_bin_add(GST_BIN(e.pipeline), e.decode);
    
    gst_element_link (e.src, e.decode);

    g_signal_connect (e.decode, "pad-added", G_CALLBACK (pad_added_handler), &e);
    //g_signal_connect (e.decode, "autoplug-continue", G_CALLBACK (autoplug_continue_cb), &e);

    g_object_set (e.src, "location", path.c_str(), nullptr);

    e.acodec = gst_element_factory_make(acodec_gst.c_str(), "acodec");
    if (e.acodec != nullptr) {
        gst_bin_add(GST_BIN(e.pipeline), e.acodec);
        gst_element_link_many (audio_last, e.acodec, e.aqueue, NULL);
    } else {
		log_config->error("Can't find audio codec");
	}
    e.vcodec = gst_element_factory_make(vcodec_gst.c_str(), "vcodec");
    if (e.vcodec != nullptr) {
        gst_bin_add(GST_BIN(e.pipeline), e.vcodec);
        gst_element_link_many (video_last, e.vcodec, e.vqueue, NULL);
    } else {
		log_config->error("Can't find video codec");
	}

	return (e);
}



