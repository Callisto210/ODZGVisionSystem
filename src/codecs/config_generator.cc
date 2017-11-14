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

void configure_pipeline(Elements &e, string source, string path, int fps, string acodec, string vcodec, Http::ResponseWriter &resp)
{
    if(log_config == nullptr) {
        log_config = spdlog::get("config");
    }
    log_config->debug("Source {}, path: {}, fps: {}, acodec: {}, vcodec: {}",
                      source, path, fps, acodec, vcodec);

    static GstElement* audio_last = nullptr;
    static GstElement* video_last = nullptr;
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


    e.pipeline = gst_pipeline_new("pipeline");
    e.aconvert = gst_element_factory_make("audioconvert", "aconvert");
    e.vconvert = gst_element_factory_make("videoconvert", "vconvert");

    e.aqueue = gst_element_factory_make("queue", "aqueue");
    e.vqueue = gst_element_factory_make("queue", "vqueue");

    video_last = e.vconvert;
    audio_last = e.aconvert;

    gst_bin_add_many(GST_BIN(e.pipeline),
                     e.aconvert,
                     e.aqueue,
                     e.vconvert,
                     e.vqueue,
                     NULL);

    e.src = gst_element_factory_make(source_gst.c_str(), "filesource");
    gst_bin_add(GST_BIN(e.pipeline), e.src);
    e.decode = gst_element_factory_make("decodebin", "source");
    gst_bin_add(GST_BIN(e.pipeline), e.decode);
    
    gst_element_link (e.src, e.decode);

    g_object_set (e.src, "location", path.c_str(), nullptr);

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

	return;
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
