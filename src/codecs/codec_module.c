#include <gst/gst.h>
#include <gst/gstbin.h>
#include "jsmn/jsmn.h"
#include <string.h>
#include "codec_module.h"


#define ICECAST
#define WEBM
// #define HLSSINK
// #define MPEGTS



// void pad_added_handler (GstElement *src, GstPad *pad, Elements *data);

int magic(Elements data, e_sink_t sink_type, e_mux_t mux_type) {
	GstBus *bus;
	GstMessage *msg;
	GstStateChangeReturn ret;
	gboolean terminate = FALSE;

	char* mux_str = get_mux_str(mux_type);
	char* sink_str = get_sink_str(sink_type);

    g_printerr("Mux is %s\n", mux_str);
    g_printerr("Sink is %s\n", sink_str);
	data.muxer = gst_element_factory_make(mux_str, "muxer");
	data.sink = gst_element_factory_make (sink_str, "sink");

	free(mux_str);
	free(sink_str);

	if (elements_has_null_field(&data)) {
		g_printerr ("Not all elements could be created.\n");
		return -1;
	}

	/* Build the pipeline. Note that we are NOT linking the source at this
	 * point. We will do it later. */
	gst_bin_add_many (GST_BIN (data.pipeline),
	    data.muxer,
	    data.sink,
	    NULL);
	    
	if (!gst_element_link (data.muxer, data.sink)) {
		g_printerr ("Elements could not be linked.\n");
		gst_object_unref (data.pipeline);
		return -1;
	}

	GstPad *muxer_a_in, *muxer_v_in;
	GstPad *acodec_out, *vcodec_out;

	if (data.acodec != NULL) {
		muxer_a_in = gst_element_get_request_pad(data.muxer, "audio_%u");
		g_print ("Obtained request pad %s for audio branch.\n", gst_pad_get_name (muxer_a_in));

		acodec_out = gst_element_get_static_pad(data.aqueue, "src");
		g_print ("Obtained request pad %s for audio branch.\n", gst_pad_get_name (acodec_out));

		if (gst_pad_link (acodec_out, muxer_a_in) != GST_PAD_LINK_OK) {
			g_printerr ("Links could not be linked.\n");
			gst_object_unref (data.pipeline);
			return -1;
		}
	}

	if (data.vcodec != NULL) {
		muxer_v_in = gst_element_get_request_pad(data.muxer, "video_%u");
		g_print ("Obtained request pad %s for video branch.\n", gst_pad_get_name (muxer_v_in));
		
		vcodec_out = gst_element_get_static_pad(data.vqueue, "src");
		g_print ("Obtained request pad %s for video branch.\n", gst_pad_get_name (vcodec_out));
		
		
		if (gst_pad_link (vcodec_out, muxer_v_in) != GST_PAD_LINK_OK) {
			g_printerr ("Links could not be linked.\n");
			gst_object_unref (data.pipeline);
			return -1;
		}
	}
	
	/* Set the URI to play */
#ifdef TCPSINK
	g_object_set (data.sink, "host", "127.0.0.1", NULL);
	g_object_set (data.sink, "port", 8080, NULL);
#else
#ifdef HLSSINK
	g_object_set (data.sink, "max-files", 5, NULL);
	g_object_set (data.sink, "target-duration", 5, NULL);
	g_object_set (data.sink, "playlist-root", "http://localhost", NULL);
	g_object_set (data.sink, "playlist-location", "/var/www/localhost/htdocs/playlist.m3u8", NULL);
	g_object_set (data.sink, "location", "/var/www/localhost/htdocs/segment%05d.ts", NULL);
#else
#ifdef ICECAST
	g_object_set (data.sink, "ip", "127.0.0.1", NULL);
	g_object_set (data.sink, "port", 8000, NULL);
	g_object_set (data.sink, "password", "ala123", NULL);
	g_object_set (data.sink, "mount", "/stream.webm", NULL);
#else
	g_object_set (data.sink, "location", "transcoded.webm", NULL);
#endif
#endif
#endif

#ifdef MP4MUX
	g_object_set (data.muxer, "fragment-duration", 100, NULL);
#endif



    g_signal_connect (data.decode, "pad-added", G_CALLBACK (pad_added_handler), &data);
    //g_signal_connect (data.decode, "autoplug-continue", G_CALLBACK (autoplug_continue_cb), &data);

	/* Start playing */
	ret = gst_element_set_state (data.pipeline, GST_STATE_PLAYING);
	if (ret == GST_STATE_CHANGE_FAILURE) {
		g_printerr ("Unable to set the pipeline to the playing state.\n");
		gst_object_unref (data.pipeline);
		return -1;
	}

	/* Listen to the bus */
	bus = gst_element_get_bus (data.pipeline);
	do {
		msg = gst_bus_timed_pop_filtered (bus, GST_CLOCK_TIME_NONE,
				GST_MESSAGE_STATE_CHANGED | GST_MESSAGE_ERROR | GST_MESSAGE_EOS);

		/* Parse message */
		if (msg != NULL) {
			GError *err;
			gchar *debug_info;

			switch (GST_MESSAGE_TYPE (msg)) {
				case GST_MESSAGE_ERROR:
					gst_message_parse_error (msg, &err, &debug_info);
					g_printerr ("Error received from element %s: %s\n", GST_OBJECT_NAME (msg->src), err->message);
					g_printerr ("Debugging information: %s\n", debug_info ? debug_info : "none");
					g_clear_error (&err);
					g_free (debug_info);
					terminate = TRUE;
					break;
				case GST_MESSAGE_EOS:
					g_print ("End-Of-Stream reached.\n");
					terminate = TRUE;
					break;
				case GST_MESSAGE_STATE_CHANGED:
					/* We are only interested in state-changed messages from the pipeline */
					if (GST_MESSAGE_SRC (msg) == GST_OBJECT (data.pipeline)) {
						GstState old_state, new_state, pending_state;
						gst_message_parse_state_changed (msg, &old_state, &new_state, &pending_state);
						g_print ("Pipeline state changed from %s to %s:\n",
								gst_element_state_get_name (old_state), gst_element_state_get_name (new_state));
					}
					break;
				default:
					/* We should not reach here */
					g_printerr ("Unexpected message received.\n");
					break;
			}
			gst_message_unref (msg);
		}
	} while (!terminate);

	/* Free resources */
	gst_object_unref (bus);
	gst_element_set_state (data.pipeline, GST_STATE_NULL);
	gst_object_unref (data.pipeline);
	return 0;
}

/* This function will be called by the pad-added signal */
void pad_added_handler (GstElement *src, GstPad *new_pad, Elements *data) {
	GstPad *sink_pad = NULL;
	GstPadLinkReturn ret;
	GstCaps *new_pad_caps = NULL;
	GstStructure *new_pad_struct = NULL;
	const gchar *new_pad_type = NULL;

	g_print ("Received new pad '%s' from '%s':\n", GST_PAD_NAME (new_pad), GST_ELEMENT_NAME (src));

	/* Check the new pad's type */
	new_pad_caps = gst_pad_query_caps (new_pad, NULL);
	new_pad_struct = gst_caps_get_structure (new_pad_caps, 0);
	new_pad_type = gst_structure_get_name (new_pad_struct);
	g_print("new_pad_type: %s \n", new_pad_type);

	

	if (g_str_has_prefix (new_pad_type, "audio/x-raw")) {
		if (data->aconvert != NULL)
			sink_pad = gst_element_get_static_pad (data->aconvert, "sink");
		else {
			g_print ("Audio not selected for transcoding. Skip \n");
			goto exit;
		}
	}
	else if (g_str_has_prefix (new_pad_type, "video/x-raw")) {
		if (data->vconvert != NULL)
			sink_pad = gst_element_get_static_pad (data->vconvert, "sink");
		else {
			g_print ("Video not selected for transcoding. Skip \n");
			goto exit;
		}
	}

	if (sink_pad == NULL) {
		g_print ("  It has type '%s' which is not raw audio. Ignoring.\n", new_pad_type);
		goto exit;
	}
	
	/* If our converter is already linked, we have nothing to do here */
	if (gst_pad_is_linked (sink_pad)) {
		g_print ("  We are already linked. Ignoring.\n");
		goto exit;
	}

	/* Attempt the link */
	ret = gst_pad_link (new_pad, sink_pad);
	if (GST_PAD_LINK_FAILED (ret)) {
		g_print ("  Type is '%s' but link failed.\n", new_pad_type);
	} else {
		g_print ("  Link succeeded (type '%s').\n", new_pad_type);
	}

exit:
	/* Unreference the new pad's caps, if we got them */
	if (new_pad_caps != NULL)
		gst_caps_unref (new_pad_caps);

	/* Unreference the sink pad */
	if (sink_pad != NULL)
		gst_object_unref (sink_pad);
}

void configure_pipeline(const char *json)
{
	jsmn_parser parser;
	jsmntok_t tokens[50];
	unsigned int entries;
	int toknum, i=0;
	static Elements data;
	char path[256];
	static GstElement *video_last = NULL;
	static GstElement *audio_last = NULL;
	static int configured = 0;

	if(!configured++)
		for(unsigned int i=0; i<sizeof(Elements)/sizeof(GstElement *); i++)
			((GstElement **)(&data))[i] = (GstElement *)NULL;

	jsmn_init(&parser);

	/* Firstly prepare buffers */
	for (unsigned int i=0; i<sizeof(path); i++)
		path[i] = '\0';

	/* Create the empty pipeline */
	if(data.pipeline == NULL) {
		data.pipeline = gst_pipeline_new ("pipeline");
		data.aconvert = gst_element_factory_make ("audioconvert", "aconvert");
		data.vconvert = gst_element_factory_make ("videoconvert", "vconvert");
		data.aqueue = gst_element_factory_make ("queue", "aqueue");
		data.vqueue = gst_element_factory_make ("queue", "vqueue");
		video_last = data.vconvert;
		audio_last = data.aconvert;
		gst_bin_add_many(GST_BIN(data.pipeline),
		    data.aconvert,
		    data.aqueue,
		    data.vconvert,
		    data.vqueue,
		    NULL);
	}

	if((toknum = jsmn_parse(&parser, json, strlen(json), tokens, 50)) < 0) {
		g_print ("Failed to parse json ;< \n");
		return;
	}
	
	if (tokens[0].type == JSMN_OBJECT) {
		entries = tokens[0].size;
		i=1;
	}

	for(; i<toknum; i+=2) {
		switch (tokens[i].type) {
			case JSMN_STRING:
				if(strncmp(json + tokens[i].start, "source", MIN(6, tokens[i].end - tokens[i].start)) == 0) {
					g_print("Source: ");
					if(strncmp(json + tokens[i+1].start, "file", MIN(4, tokens[i+1].end - tokens[i+1].start)) == 0) {
						g_print("file\n");
						data.src = gst_element_factory_make("filesrc", "filesource");
						gst_bin_add(GST_BIN(data.pipeline), data.src);
					}
					
					data.decode = gst_element_factory_make ("decodebin", "source");
					gst_bin_add(GST_BIN(data.pipeline), data.decode);
					gst_element_link (data.src, data.decode);

					/* Connect to the pad-added signal */
					g_signal_connect (data.decode, "pad-added", G_CALLBACK (pad_added_handler), &data);
					g_signal_connect (data.decode, "autoplug-continue", G_CALLBACK (autoplug_continue_cb), &data);
				}
				else if(strncmp(json + tokens[i].start, "path", MIN(4, tokens[i].end - tokens[i].start)) == 0) {
					strncpy(path, json + tokens[i+1].start, MIN(sizeof(path), tokens[i+1].end - tokens[i+1].start));
					g_print("Path %s\n", path);
					g_object_set (data.src, "location", path, NULL);
				}
				else if(strncmp(json + tokens[i].start, "fps", MIN(3, tokens[i].end - tokens[i].start)) == 0) {
					g_print("Fps\n");
				}
				else if(strncmp(json + tokens[i].start, "acodec", MIN(6, tokens[i].end - tokens[i].start)) == 0) {
					g_print("Audio codec: ");
					if(strncmp(json + tokens[i+1].start, "opus", MIN(4, tokens[i+1].end - tokens[i+1].start)) == 0) {
						g_print("opus\n");
						data.acodec = gst_element_factory_make ("opusenc", "acodec");
					}
					if(strncmp(json + tokens[i+1].start, "aac", MIN(3, tokens[i+1].end - tokens[i+1].start)) == 0) {
						g_print("aac\n");
						data.acodec = gst_element_factory_make ("voaacenc", "acodec");
					}
					if(strncmp(json + tokens[i+1].start, "lame", MIN(4, tokens[i+1].end - tokens[i+1].start)) == 0) {
						g_print("lame\n");
						data.acodec = gst_element_factory_make ("lamemp3enc", "acodec");
					}
					if (data.acodec != NULL) {
						gst_bin_add(GST_BIN(data.pipeline), data.acodec);	
						gst_element_link_many (audio_last, data.acodec, data.aqueue, NULL);
					}
				}
				else if(strncmp(json + tokens[i].start, "vcodec", MIN(6, tokens[i].end - tokens[i].start)) == 0) {
					g_print("Video codec: ");
					if(strncmp(json + tokens[i+1].start, "vp8", MIN(3, tokens[i+1].end - tokens[i+1].start)) == 0) {
						g_print("vp8\n");					
						data.vcodec = gst_element_factory_make ("vp8enc", "vcodec");
					}
					if(strncmp(json + tokens[i+1].start, "h264", MIN(4, tokens[i+1].end - tokens[i+1].start)) == 0) {
						g_print("h264\n");					
						data.vcodec = gst_element_factory_make ("x264enc", "vcodec");
					}
					if (data.vcodec != NULL) {
						gst_bin_add(GST_BIN(data.pipeline), data.vcodec);
						gst_element_link_many (video_last, data.vcodec, data.vqueue, NULL);
					}
				}
			break;
			case JSMN_PRIMITIVE:
				g_print("Found PRIMITIVE start: %d, end: %d\n",
				    tokens[i].start, tokens[i].end);
			break;
			default:
				g_print("Bad entry\n");
		}
	}
	magic(data, HLS_SINK, MPEG_TS_MUX);
	return;
}

static void print_null_element(const char* element) {
    if(element != NULL) {
        g_print("%s element cannot be created.\n", element);
    }
}
int elements_has_null_field(Elements* data)
{
	char *reason = NULL;
    int null_count = 0;
    if(data != NULL) {
    if(!data->src) {
        print_null_element("src");
        null_count++;
    }
    if(!data->decode) {
        print_null_element("decode");
        null_count++;
    }
    if(!data->aconvert)
		goto skip_audio;
    if(!data->aqueue) {
        print_null_element("aqueue");
        null_count++;
    }
    if(!data->acodec) {
        print_null_element("acodec");
        null_count++;
    }
	
	skip_audio:
	if(!data->vcodec)
		goto skip_video;
    if(!data->vconvert) {
        print_null_element("vconvert");
        null_count++;
    if(!data->vqueue) {
        print_null_element("vqueue");
        null_count++;
    }
	
	skip_video:
    if(!data->muxer) {
        print_null_element("muxer");
        null_count++;
    }
    if(!data->sink) {
        print_null_element("sink");
        null_count++;
    }
	if(reason) {
		g_print("%s element can't be created\n", reason);
		return (1);
	}
	else
		return (0);

}
