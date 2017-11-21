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

struct pads_struct {
	Elements *e;
	Http::ResponseWriter *response;
};

void no_more_pads_cb (GstElement *e, Elements *ptr);
gboolean bus_watch_get_stream (GstBus* bus, GstMessage *msg, GstElement *pipeline);

static std::shared_ptr<spdlog::logger> log_config = std::shared_ptr<spdlog::logger>();
static map<string, string> acodec_map = {
        {"opus" , "opusenc"},
        {"aac", "faac"},
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

static void zero_elements(Elements* e) {
    memset(e, 0, sizeof(Elements));
}


static void basic_pipeline(Elements* data) {
    data->playbin = gst_element_factory_make("playbin", "playbin");
    data->pipeline = gst_pipeline_new("pipeline");
    data->aconvert = gst_element_factory_make("audioconvert", "aconvert");
    data->vconvert = gst_element_factory_make("videoconvert", "vconvert");
    data->aqueue = gst_element_factory_make("queue", "aqueue");
    data->vqueue = gst_element_factory_make("queue", "vqueue");
    data->intervideosink = gst_element_factory_make ("intervideosink", "intervideosink");
    data->interaudiosink = gst_element_factory_make ("interaudiosink", "interaudiosink");
    data->intervideosrc = gst_element_factory_make ("intervideosrc", "intervideosrc");
    data->interaudiosrc = gst_element_factory_make ("interaudiosrc", "interaudiosrc");
    data->vbin = gst_bin_new ("vbin");
    data->abin = gst_bin_new ("abin");
}


void configure_pipeline(Elements& e, string source, string path, int fps, string acodec, string vcodec, Pistache::Http::ResponseWriter &resp)
{

    if(log_config == nullptr) {
        log_config = spdlog::get("config");
    }
    log_config->debug("Source {}, path: {}, fps: {}, acodec: {}, vcodec: {}",
                      source, path, fps, acodec, vcodec);

    static GstElement* audio_last = nullptr;
    static GstElement* video_last = nullptr;
    GstElement * in_a_queue, * in_v_queue, * out_v_queue, * out_a_queue;
    GstPad *pad, *a_pad, *v_pad;
    GstPad *acodec_out, *vcodec_out;
    GstPad *muxer_a_in, *muxer_v_in;
    in_a_queue = gst_element_factory_make("queue", "in_a_queue");
    in_v_queue = gst_element_factory_make("queue", "in_v_queue");
    out_a_queue = gst_element_factory_make("queue", "out_a_queue");
    out_v_queue = gst_element_factory_make("queue", "out_v_queue");
    GstBus *bus;
    GstStateChangeReturn ret;
    static bool configured = false;

    if(!configured) {
        configured = !configured;
        zero_elements(&e);
    }

    string acodec_gst = acodec_map[acodec];
    string vcodec_gst = vcodec_map[vcodec];
    string source_gst = source_map[source];


    log_config->debug("Elements opts: source: {} acodec: {} vcodec: {}",
        source_gst, acodec_gst, vcodec_gst);



    basic_pipeline(&e);
    video_last = e.vconvert;
    audio_last = e.aconvert;
    // Configure interaudiosink
//    g_object_set (G_OBJECT (e.intervideosink), "sync", TRUE, NULL);
//    g_object_set (G_OBJECT (e.interaudiosink), "sync", TRUE, NULL);
//    g_object_set (G_OBJECT (e.intervideosink), "sync", TRUE, NULL);
//    g_object_set (G_OBJECT (e.interaudiosink), "sync", TRUE, NULL);

    g_object_set (G_OBJECT (e.intervideosink), "channel", "channel0", NULL);
    g_object_set (G_OBJECT (e.intervideosrc), "channel", "channel0", NULL);

    g_object_set (G_OBJECT (e.interaudiosink), "channel", "channel1", NULL);
    g_object_set (G_OBJECT (e.interaudiosrc), "channel", "channel1", NULL);

//    g_object_set (G_OBJECT (e.intervideosink), "blocksize", 4096, NULL);
//    g_object_set (G_OBJECT (e.intervideosrc), "blocksize", 4096, NULL);
//
//    g_object_set (G_OBJECT (e.interaudiosink), "blocksize", 4096, NULL);
//    g_object_set (G_OBJECT (e.interaudiosrc), "blocksize", 4096, NULL);

//    g_object_set (G_OBJECT (e.intervideosrc), "num-buffers", 2200, NULL);
//    g_object_set (G_OBJECT (e.interaudiosrc), "num-buffers", 2200, NULL);
//    g_object_set (G_OBJECT (e.intervideosink), "num-buffers", 100, NULL);
//    g_object_set (G_OBJECT (e.interaudiosink), "num-buffers", 100, NULL);
//    gst_element_link(e.intervideosink,e.intervideosrc);
//    gst_element_link(e.interaudiosink,e.interaudiosrc);

    gint flags;
    gst_bin_add_many(GST_BIN(e.pipeline),
                     e.interaudiosrc,
                     in_a_queue,
                     e.aconvert,
                     e.aqueue,
                     e.intervideosrc,
                     in_v_queue,
                     e.vconvert,
                     e.vqueue,
                     NULL);

//    e.src = gst_element_factory_make("intervideosrc", "intervideosrc");
//    gst_bin_add(GST_BIN(e.pipeline), e.src);
//    e.decode = gst_element_factory_make("decodebin", "source");
//    gst_bin_add(GST_BIN(e.pipeline), e.decode);
//    if(strncmp("file",source.c_str(),5)==0){
//        path.insert(0,"file://");
//    }
//    g_object_set(e.playbin, "uri", path.c_str(),NULL);
//
//    g_object_get (e.playbin, "flags", &flags, NULL);
//    flags |= GST_PLAY_FLAG_VIDEO | GST_PLAY_FLAG_AUDIO;
//    flags &= ~GST_PLAY_FLAG_TEXT;
//    g_object_set (e.playbin, "flags", flags, NULL);
    //g_object_set (e.playbin, "connection-speed", 56, NULL);
    GString *pipe_desc;
    GError *error = NULL;
    pipe_desc = g_string_new ("");
    g_string_append (pipe_desc, "playbin uri=file:///home/jgorski/Downloads/matroska_test_w1_1/test2.mkv");
    //g_string_append (pipe_desc, path.c_str());
    log_config->debug(pipe_desc->str);
    e.playbin = (GstElement*) gst_parse_launch(pipe_desc->str, &error);
    if (error) {
        g_print ("pipeline parsing error: %s\n", error->message);
        gst_object_unref (e.playbin);
        g_clear_error (&error);
        return;
    }
    //gst_element_link (e.src, e.decode);
//    gst_bin_add_many(GST_BIN(e.vbin),
//                    out_v_queue,
//                     e.intervideosink,
//                     NULL);
//    gst_bin_add_many(GST_BIN(e.abin),
//                     out_a_queue,
//                     e.interaudiosink,
//                     NULL);
//    gst_element_link_many(out_a_queue, e.interaudiosink, NULL);
//    gst_element_link_many(out_v_queue, e.intervideosink, NULL);
//    pad = gst_element_get_static_pad (out_a_queue, "sink");
//    a_pad = gst_ghost_pad_new ("sink", pad);
//    gst_pad_set_active (a_pad, TRUE);
//    gst_element_add_pad (e.abin, a_pad);
//    gst_object_unref (pad);
//    pad = gst_element_get_static_pad (out_v_queue, "sink");
//    v_pad = gst_ghost_pad_new ("sink", pad);
//    gst_pad_set_active (v_pad, TRUE);
//    gst_element_add_pad (e.vbin, v_pad);
//    gst_object_unref (pad);
    g_object_set (GST_OBJECT (e.playbin), "audio-sink",e.interaudiosink, NULL);
    g_object_set (GST_OBJECT (e.playbin), "video-sink", e.intervideosink, NULL);
    g_object_set (GST_OBJECT (e.playbin), "message-forward",TRUE, NULL);
    g_object_set (GST_OBJECT (e.playbin), "async-handling",TRUE, NULL),
    gst_element_set_state (e.playbin, GST_STATE_READY);
//    g_object_set (e.src, "location", path.c_str(), nullptr);

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
        //gst_bin_add_many(GST_BIN(e.playbin),audio_last, e.acodec, e.interaudiosink, NULL);
        gst_bin_add(GST_BIN(e.pipeline), e.acodec);
        gst_element_link_many (e.interaudiosrc ,in_a_queue, audio_last, e.acodec,e.aqueue, NULL);
//        gst_element_link_many (audio_last, e.acodec,e.aqueue, NULL);
//        muxer_a_in = gst_element_get_static_pad (audio_last, "sink");
//        acodec_out = gst_element_get_static_pad(in_a_queue, "src");
//        if (gst_pad_link (acodec_out, muxer_a_in) != GST_PAD_LINK_OK)  {
//            g_printerr ("Links could not be linked audio.\n");
//            //gst_object_unref (e.pipeline);
//            return;
//        }
    } else {
        log_config->error("Can't find audio codec");
	}
    e.vcodec = gst_element_factory_make(vcodec_gst.c_str(), "vcodec");
    if (e.vcodec != nullptr) {
        if(strncmp("vp8enc", vcodec_gst.c_str(), 6) == 0){
		g_object_set(e.vcodec, "threads", 6, NULL);
		g_object_set(e.vcodec, "target-bitrate", 2000, NULL);}
        gst_bin_add(GST_BIN(e.pipeline), e.vcodec);
        gst_element_link_many (e.intervideosrc,in_v_queue, video_last,e.vcodec,e.vqueue, NULL);
//        gst_element_link_many (e.intervideosrc ,in_v_queue,  NULL);
//        gst_element_link_many (video_last, e.vcodec,e.vqueue, NULL);
//        muxer_v_in = gst_element_get_static_pad (video_last, "sink");
//        vcodec_out = gst_element_get_static_pad(in_v_queue, "src");
//        if (gst_pad_link (vcodec_out, muxer_v_in) != GST_PAD_LINK_OK)  {
//            g_printerr ("Links could not be linked video.\n");
//            //gst_object_unref (e.pipeline);
//            return;
//        }

    } else {
        log_config->error("Can't find video codec");
	}
//    GstStateChangeReturn ret;
//    ret = gst_element_set_state (e.playbin, GST_STATE_READY);
//    if (ret == GST_STATE_CHANGE_FAILURE) {
//        g_printerr ("Unable to set the 2 pipeline to the playing state.\n");
//        gst_object_unref (e.playbin);
//        return;
//    }


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
