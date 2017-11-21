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

struct pads_struct {
	Elements *e;
	Http::ResponseWriter *response;
};

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

static map <string, string> source_map = {
        {"file", "filesrc"}
};

static map <string, string> sink_map = {
        {"file", "filesink"},
	{"udp", "udpsink"},
	{"icecast", "shout2send"}
};

static void zero_elements(Elements& e) {
    memset(&e, 0, sizeof(Elements));
}

void configure_pipeline(Elements &e, Http::ResponseWriter &resp, config_struct conf)
{
    if(log_config == nullptr) {
        log_config = spdlog::get("config");
    }
    log_config->debug("Source {}, path: {}, acodec: {}, vcodec: {}, sink: {}",
                      conf.source, conf.path, conf.acodec, conf.vcodec, conf.sink);

    static GstElement* audio_last = nullptr;
    static GstElement* video_last = nullptr;
    static GstElement* optional = nullptr;
    static bool configured = false;

    if(!configured) {
        configured = !configured;
 	memset(&e, 0, sizeof(Elements));
    }

    string acodec_gst = acodec_map[conf.acodec];
    string vcodec_gst = vcodec_map[conf.vcodec];
    string source_gst = source_map[conf.source];
    string sink_gst = sink_map[conf.sink];


    log_config->debug("Elements opts: source: {} acodec: {} vcodec: {} sink: {}",
        source_gst, acodec_gst, vcodec_gst, sink_gst);


    e.pipeline = gst_pipeline_new("pipeline");

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

    e.src = gst_element_factory_make(source_gst.c_str(), "filesource");
    gst_bin_add(GST_BIN(e.pipeline), e.src);
    e.decode = gst_element_factory_make("decodebin", "source");
    gst_bin_add(GST_BIN(e.pipeline), e.decode);
    
    gst_element_link (e.src, e.decode);

    g_object_set (e.src, "location", conf.path.c_str(), nullptr);

#if 0
	/* Get info about streams */
	pads_struct s;
	s.response = new Http::ResponseWriter(resp);
	s.e = &e;

	GstBus *bus = gst_element_get_bus(e.pipeline);
	g_signal_connect(G_OBJECT(e.decode), "no-more-pads", G_CALLBACK(no_more_pads_cb), &s);
	gst_element_set_state(e.pipeline, GST_STATE_PAUSED);
	while (true)
	{
		GstMessage *msg = gst_bus_pop(bus);
		if (msg) 
			if (!bus_watch_get_stream(bus,msg, e.pipeline))
	  			break;
		 else
			break;
	}
#endif


    if (!vcodec_gst.empty()) {
	    if(conf.fps != -1) {
		optional = gst_element_factory_make("videorate", "fps");
		if (optional != NULL) {
			gst_bin_add(GST_BIN(e.pipeline), optional);
			g_object_set(optional, "max-rate", conf.fps, NULL);
			if (gst_element_link(video_last, optional)) {
				video_last = optional;
			}
		}
	    }

	    if(conf.width != -1 && conf.height !=-1) {
		optional = gst_element_factory_make("videoscale", "scale");
		if (optional != NULL) {
			gst_bin_add(GST_BIN(e.pipeline), optional);
			GstCaps * caps = gst_caps_new_simple ("video/x-raw",
			    "width", G_TYPE_INT, conf.width,
			    "height", G_TYPE_INT, conf.height,
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
		if (conf.audio_bitrate != -1) {
		//lamemp3enc takes bitrate in kbit/s
			if (strncmp("lamemp3enc", acodec_gst.c_str(), 10) == 0)
				g_object_set(e.acodec, "bitrate", (conf.audio_bitrate/8)*8, NULL);
			else	
				g_object_set(e.acodec, "bitrate", ((conf.audio_bitrate*1000)/8)*8, NULL);
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
			if (conf.video_bitrate != -1)
				g_object_set(e.vcodec, "target-bitrate", conf.video_bitrate*1000, NULL);
		}	
		if(strncmp("vp9enc", vcodec_gst.c_str(), 6) == 0) {
			g_object_set(e.vcodec, "threads", 6, NULL);
			if (conf.video_bitrate != -1)
				g_object_set(e.vcodec, "target-bitrate", conf.video_bitrate*1000, NULL);
		}
		gst_bin_add(GST_BIN(e.pipeline), e.vcodec);
		gst_element_link_many (video_last, e.vcodec, e.vqueue, NULL);
	    } else {
			log_config->error("Can't find video codec");
		}

    }

    if (!sink_gst.empty()) {
    	e.sink = gst_element_factory_make(sink_gst.c_str(), "sink");
	if (e.sink != nullptr) {
	    if (strncmp("filesink", sink_gst.c_str(), 4) == 0) {
		g_object_set (e.sink, "location", "transcoded.webm", NULL);
	    }
	    if (strncmp("udpsink", sink_gst.c_str(), 3) == 0) {
		g_object_set (e.sink, "host", "127.0.0.1", NULL);
		g_object_set (e.sink, "port", 8080, NULL);
	    }
	    if (strncmp("shout2send", sink_gst.c_str(), 7) == 0) {
		g_object_set (e.sink, "ip", "127.0.0.1", NULL);
		g_object_set (e.sink, "port", 8000, NULL);
		g_object_set (e.sink, "password", "ala123", NULL);
		g_object_set (e.sink, "mount", "/stream.webm", NULL);
	    }
	    gst_bin_add(GST_BIN(e.pipeline), e.sink);
	}
    }
}

gboolean bus_watch_get_stream (GstBus* bus, GstMessage *msg, GstElement *pipeline)
{
  switch (GST_MESSAGE_TYPE (msg)) {
    case GST_MESSAGE_ASYNC_DONE:
      gst_message_unref(msg);
      return FALSE;
    case GST_MESSAGE_EOS:
      gst_element_set_state(GST_ELEMENT(pipeline),GST_STATE_NULL);
      gst_message_unref(msg);
      return FALSE;
    case GST_MESSAGE_TAG: //TODO collect tags
      break;
    default:
      break;
  }

  gst_message_unref(msg);
  return TRUE;
} 

void
no_more_pads_cb (GstElement *e, pads_struct *ptr)
{
	GstIterator *pads_iter;
	Document doc;
	Value array(kArrayType);
	Document::AllocatorType& alloc = doc.GetAllocator();
	GValue item = G_VALUE_INIT;

	doc.SetObject();
	g_print("In no_more_pads cb \n");

	for (pads_iter = gst_element_iterate_src_pads(e);
	     gst_iterator_next (pads_iter, &item) == GST_ITERATOR_OK;
	     g_value_reset(&item))
	{
		GstPad *pad = GST_PAD(g_value_get_object(&item));
		gchar *pad_stream_id = GST_PAD_NAME(pad);
		Value obj(kObjectType);
		Value str(kObjectType);

		str.SetString(pad_stream_id, alloc);
		obj.AddMember("name", str, alloc);//(char *)pad_stream_id);

		array.PushBack(obj, alloc);
		g_free(pad_stream_id);
	}
	g_value_unset(&item);
	gst_iterator_free(pads_iter);

	doc.AddMember("streams", array, alloc);

	StringBuffer strbuf;
	Writer<StringBuffer> writer(strbuf);
	doc.Accept(writer);
	g_print("%s\n", strbuf.GetString());
	ptr->response->send(Http::Code::Ok, strbuf.GetString());
}
