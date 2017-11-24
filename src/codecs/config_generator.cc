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

#if 1
	/* Get info about streams */
	pads_struct data;
    GError *err = NULL;
    //memset (&data, 0, sizeof (data));
    data.videon = 0;
    data.audion = 0;
    g_print ("Discovering '%s'\n", conf.path.c_str());
    data.discoverer = gst_discoverer_new (5 * GST_SECOND, &err);
    if (!data.discoverer) {
        g_print ("Error creating discoverer instance: %s\n", err->message);
        g_clear_error (&err);
        return;
    }
    g_signal_connect (data.discoverer, "discovered", G_CALLBACK (on_discovered_cb), &data);
    g_signal_connect (data.discoverer, "finished", G_CALLBACK (on_finished_cb), &data);
    gst_discoverer_start (data.discoverer);
    if(strncmp("file", conf.source.c_str(),4)==0){
        conf.path.insert(0,"file://");
    }
    if (!gst_discoverer_discover_uri_async (data.discoverer, conf.path.c_str())) {
        g_print ("Failed to start discovering URI '%s'\n", conf.path.c_str());
        g_object_unref (data.discoverer);
        return;
    }
    data.loop = g_main_loop_new (NULL, FALSE);
    g_main_loop_run (data.loop);

    /* Stop the discoverer process */
    gst_discoverer_stop (data.discoverer);

    /* Free resources */
    g_object_unref (data.discoverer);
    g_main_loop_unref (data.loop);
//	s.response = new Http::ResponseWriter(resp);
//	//s.e = &e;
//
//	GstBus *bus = gst_element_get_bus(e.pipeline);
//	g_signal_connect(G_OBJECT(e.decode), "no-more-pads", G_CALLBACK(no_more_pads_cb), &s);
//	gst_element_set_state(e.pipeline, GST_STATE_PAUSED);
//	while (true)
//	{
//		GstMessage *msg = gst_bus_pop(bus);
//		if (msg)
//			if (!bus_watch_get_stream(bus,msg, e.pipeline))
//	  			break;
//		 else
//			break;
//	}
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
        Value str2(kObjectType);
		str.SetString(pad_stream_id, alloc);
        str2.SetString("name",alloc);
		obj.AddMember(str2, str, alloc);//(char *)pad_stream_id);

		array.PushBack(obj, alloc);
		g_free(pad_stream_id);
	}
	g_value_unset(&item);
	gst_iterator_free(pads_iter);

	ptr->doc->AddMember("streams", array, alloc);

	StringBuffer strbuf;
	Writer<StringBuffer> writer(strbuf);
	doc.Accept(writer);
	g_print("%s\n", strbuf.GetString());
	ptr->response->send(Http::Code::Ok, strbuf.GetString());
}
//static void print_tag_foreach (const GstTagList *tags, const gchar *tag, Value *data) {
//    GValue val = { 0, };
//    gchar *str;
//    //gint depth = GPOINTER_TO_INT (user_data);
//
//    gst_tag_list_copy_value (&val, tags, tag);
//
//    if (G_VALUE_HOLDS_STRING (&val))
//        str = g_value_dup_string (&val);
//    else
//        str = gst_value_serialize (&val);
//    if(g_strcmp0("bitrate",gst_discoverer_stream_info_get_stream_type_nick (info))==0)
//    g_print ("%*s%s: %s\n", 2 * 2, " ", gst_tag_get_nick (tag), str);
//    g_free (str);
//
//    g_value_unset (&val);
//}

/* Print information regarding a stream */
static void print_stream_info (GstDiscovererStreamInfo *info, gint depth,pads_struct *data) {
    gchar *desc = NULL;
    gchar *streamid = NULL;
    GstCaps *caps;
    const GstTagList *tags;
    Value *push;
    Value obj(kObjectType);
    caps = gst_discoverer_stream_info_get_caps (info);

    Document::AllocatorType& alloc = data->doc->GetAllocator();
    if (caps) {
        if (gst_caps_is_fixed (caps))
            desc = gst_pb_utils_get_codec_description (caps);
        else
            desc = gst_caps_to_string (caps);
        gst_caps_unref (caps);
    }
    if(g_strcmp0("audio",gst_discoverer_stream_info_get_stream_type_nick (info))==0){
        g_print("1\n");
        Value str;
        Value name;
        name.SetString(StringRef("codec"));
        push = data->audio;
        char buffer[40];
        int len = sprintf(buffer, "%s", data->audion); // dynamically created string.
        g_print("2\n");
       // if(desc) {
        if(desc)
            str.SetString(StringRef(g_str_to_ascii(desc, NULL)));
        else
            str.SetString(StringRef(g_str_to_ascii("", NULL)));
        g_print("3\n");
     //   } else{
      //      str.SetString("",alloc);
     //   }

        obj.AddMember(name, str, alloc);
        name.SetString(StringRef("streamid"));
        str.SetString(StringRef(g_str_to_ascii(gst_discoverer_stream_info_get_stream_id(info),NULL)));
        obj.AddMember(name,str,alloc);
        g_print("4\n");

        //data->audio->AddMember(Value(buffer,len, *data->alloc), obj, *data->alloc);
        data->audio->PushBack(obj,*data->alloc);
        data->audion++;
        g_print("5\n");

    }
    if(g_strcmp0("video",gst_discoverer_stream_info_get_stream_type_nick (info))==0){
        char buffer[40];
        int len = sprintf(buffer, "%s", data->videon); // dynamically created string.
        g_print("6\n");
        push = data->video;
        Value str;

        Value name;
     //   if(desc) {
        if(desc)
            str.SetString(StringRef(g_str_to_ascii(desc, NULL)));
        else
            str.SetString(StringRef(g_str_to_ascii("", NULL)));
        g_print("7\n");
        //} else{
       //     str.SetString("",alloc);
     //   }
        name.SetString(StringRef("codec"));
        g_print("8\n");
        obj.AddMember(name, str, alloc);
        g_print("9\n");
        name.SetString(StringRef("streamid"));
        str.SetString(StringRef(g_str_to_ascii(gst_discoverer_stream_info_get_stream_id(info),NULL)));
        obj.AddMember(name, str, alloc);
        //data->video->AddMember(Value(buffer,len, *data->alloc), obj, *data->alloc);
        data->video->PushBack(obj,*data->alloc);
        data->videon++;
        g_print("10\n");

    }
    g_print ("%*s%s: %s\n", 2 * depth, " ", gst_discoverer_stream_info_get_stream_type_nick (info), (desc ? desc : ""));

    if (desc) {
        g_free (desc);
        desc = NULL;
    }
//    tags = gst_discoverer_stream_info_get_tags (info);
//    if (tags) {
//        g_print ("%*sTags:\n", 2 * (depth + 1), " ");
//        gst_tag_list_foreach (tags, print_tag_foreach, &obj);
//    }
}

/* Print information regarding a stream and its substreams, if any */
static void print_topology (GstDiscovererStreamInfo *info, gint depth,pads_struct *data) {
    GstDiscovererStreamInfo *next;

    if (!info)
        return;

    print_stream_info (info, depth,data);

    next = gst_discoverer_stream_info_get_next (info);
    if (next) {
        print_topology (next, depth + 1,data);
        gst_discoverer_stream_info_unref (next);
    } else if (GST_IS_DISCOVERER_CONTAINER_INFO (info)) {
        GList *tmp, *streams;
        //Value container(kObjectType);
        streams = gst_discoverer_container_info_get_streams (GST_DISCOVERER_CONTAINER_INFO (info));
        for (tmp = streams; tmp; tmp = tmp->next) {
            GstDiscovererStreamInfo *tmpinf = (GstDiscovererStreamInfo *) tmp->data;
            print_topology (tmpinf, depth + 1,data);
        }
        gst_discoverer_stream_info_list_free (streams);
    }
}

/* This function is called every time the discoverer has information regarding
 * one of the URIs we provided.*/
static void on_discovered_cb (GstDiscoverer *discoverer, GstDiscovererInfo *info, GError *err, pads_struct *data) {
    GstDiscovererResult result;
    const gchar *uri;
    const GstTagList *tags;
    GstDiscovererStreamInfo *sinfo;
    data->doc = new Document;
    data->doc->SetObject();
    g_print("HeRe");
    data->alloc = &(*data->doc).GetAllocator();
    g_print("Tutaj");


    data->audio = new Value;
    data->video = new Value;
    data->audio->SetArray();
    data->video->SetArray();
//    Document doc;
//    Document::AllocatorType& alloc = doc.GetAllocator();
//    doc.SetObject();
    uri = gst_discoverer_info_get_uri (info);
    result = gst_discoverer_info_get_result (info);
    switch (result) {
        case GST_DISCOVERER_URI_INVALID:
            g_print ("Invalid URI '%s'\n", uri);
            break;
        case GST_DISCOVERER_ERROR:
            g_print ("Discoverer error: %s\n", err->message);
            break;
        case GST_DISCOVERER_TIMEOUT:
            g_print ("Timeout\n");
            break;
        case GST_DISCOVERER_BUSY:
            g_print ("Busy\n");
            break;
        case GST_DISCOVERER_MISSING_PLUGINS:{
            const GstStructure *s;
            gchar *str;

            s = gst_discoverer_info_get_misc (info);
            str = gst_structure_to_string (s);

            g_print ("Missing plugins: %s\n", str);
            g_free (str);
            break;
        }
        case GST_DISCOVERER_OK:
            g_print ("Discovered '%s'\n", uri);
            break;
    }

    if (result != GST_DISCOVERER_OK) {
        g_printerr ("This URI cannot be played\n");
        return;
    }

    /* If we got no error, show the retrieved information */

    g_print ("\nDuration: %" GST_TIME_FORMAT "\n", GST_TIME_ARGS (gst_discoverer_info_get_duration (info)));

//    tags = gst_discoverer_info_get_tags (info);
//    if (tags) {
//        g_print ("Tags:\n");
//        gst_tag_list_foreach (tags, print_tag_foreach, GINT_TO_POINTER (1));
//    }

    g_print ("Seekable: %s\n", (gst_discoverer_info_get_seekable (info) ? "yes" : "no"));

    g_print ("\n");

    sinfo = gst_discoverer_info_get_stream_info (info);
    if (!sinfo)
        return;

    g_print ("Stream information:\n");

    print_topology (sinfo, 1,data);

    gst_discoverer_stream_info_unref (sinfo);
    Value obj(kObjectType);
    Value audio_n ;
    audio_n.SetString(StringRef("audio_n"),*data->alloc);
    Value video_n ;
    video_n.SetString(StringRef("video_n"),*data->alloc);
    Value data_count ;
    data_count.SetString(StringRef("data_count"),*data->alloc);
    Value audio ;
    audio.SetString(StringRef("audio"),*data->alloc);
    Value video ;
    video.SetString(StringRef("video"),*data->alloc);
    g_print("11\n");
    obj.AddMember(audio_n, Value(data->audion), *data->alloc);
    g_print("12\n");
    obj.AddMember(video_n, Value(data->videon), *data->alloc);
    g_print("13\n");
    data->doc->AddMember(data_count, obj , *data->alloc);
    g_print("14\n");
    data->doc->AddMember(audio, *data->audio, *data->alloc);
    g_print("15\n");
    data->doc->AddMember(video, *data->video, *data->alloc);
    g_print("16\n");

    StringBuffer strbuf;
    Writer<StringBuffer> writer(strbuf);
    data->doc->Accept(writer);
    g_print("%s\n", strbuf.GetString());

    g_print ("\n");
}

/* This function is called when the discoverer has finished examining
 * all the URIs we provided.*/
static void on_finished_cb (GstDiscoverer *discoverer, pads_struct *data) {
    g_print ("Finished discovering\n");

    g_main_loop_quit (data->loop);
}