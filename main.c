#include <gst/gst.h>
#include <gst/gstbin.h>

#define HLSSINK
#define MPEGTS

extern gboolean autoplug_continue_cb(GstBin *, GstPad *,
    GstCaps *, gpointer);

/* Structure to contain all our information, so we can pass it to callbacks */
typedef struct _elements {
	GstElement *pipeline;
	GstElement *filesrc;
	GstElement *decode;
	
	GstElement *aconvert;
	GstElement *vconvert;
	GstElement *acodec;
	GstElement *vcodec;
	GstElement *aqueue;
	GstElement *vqueue;
	GstElement *muxer;
	GstElement *sink;
} Elements;

/* Handler for the pad-added signal */
static void pad_added_handler (GstElement *src, GstPad *pad, Elements *data);

int main(int argc, char *argv[]) {
	Elements data;
	GstBus *bus;
	GstMessage *msg;
	GstStateChangeReturn ret;
	gboolean terminate = FALSE;

	/* Initialize GStreamer */
	gst_init (&argc, &argv);

	/* Create the elements */
	data.filesrc = gst_element_factory_make ("filesrc", "filesource");
	data.decode = gst_element_factory_make ("decodebin", "source");
	data.aconvert = gst_element_factory_make ("audioconvert", "aconvert");
	data.vconvert = gst_element_factory_make ("videoconvert", "vconvert");
	//data.acodec = gst_element_factory_make ("opusenc", "acodec");
	data.acodec = gst_element_factory_make ("voaacenc", "acodec");
	//data.acodec = gst_element_factory_make ("lamemp3enc", "acodec");
	//data.vcodec = gst_element_factory_make ("vp8enc", "vcodec");
	data.vcodec = gst_element_factory_make ("x265enc", "vcodec");
	data.aqueue = gst_element_factory_make ("queue", "aqueue");
	data.vqueue = gst_element_factory_make ("queue", "vqueue");
#ifdef OGGMUX
	data.muxer = gst_element_factory_make("oggmux", "muxer");
#endif
#ifdef MPEGTS
	data.muxer = gst_element_factory_make("mpegtsmux", "muxer");
#endif
#ifdef MP4MUX
	data.muxer = gst_element_factory_make("mp4mux", "muxer");
#endif
#ifdef TCPSINK
	data.sink = gst_element_factory_make ("tcpserversink", "sink");
#else
#ifdef HLSSINK
	data.sink = gst_element_factory_make ("hlssink", "sink");
#else
	data.sink = gst_element_factory_make ("filesink", "sink");
#endif
#endif

	/* Create the empty pipeline */
	data.pipeline = gst_pipeline_new ("pipeline");

	if (!data.pipeline ||
	    !data.filesrc ||
	    !data.decode ||
	    !data.aconvert ||
	    !data.vconvert ||
	    !data.acodec ||
	    !data.vcodec ||
	    !data.aqueue ||
	    !data.vqueue ||
	    !data.muxer ||
	    !data.sink) {
		g_printerr ("Not all elements could be created.\n");
		return -1;
	}

	/* Build the pipeline. Note that we are NOT linking the source at this
	 * point. We will do it later. */
	gst_bin_add_many (GST_BIN (data.pipeline),
	    data.filesrc,
	    data.decode,
	    data.aconvert,
	    data.vconvert,
	    data.acodec,
	    data.vcodec,
	    data.aqueue,
	    data.vqueue,
	    data.muxer,
	    data.sink,
	    NULL);
	    
	if (!gst_element_link (data.filesrc, data.decode) ||
	    !gst_element_link_many (data.aconvert, data.acodec, data.aqueue, NULL) ||
	    !gst_element_link_many (data.vconvert, data.vcodec, data.vqueue, NULL) ||
	    !gst_element_link (data.muxer, data.sink)) {
		g_printerr ("Elements could not be linked.\n");
		gst_object_unref (data.pipeline);
		return -1;
	}

	GstPad *muxer_a_in, *muxer_v_in;
#ifdef MPEGTS
	muxer_a_in = gst_element_get_request_pad(data.muxer, "sink_%d");
	muxer_v_in = gst_element_get_request_pad(data.muxer, "sink_%d");
#else
	muxer_a_in = gst_element_get_request_pad(data.muxer, "audio_%u");
	muxer_v_in = gst_element_get_request_pad(data.muxer, "video_%u");
#endif
	g_print ("Obtained request pad %s for audio branch.\n", gst_pad_get_name (muxer_a_in));
	g_print ("Obtained request pad %s for video branch.\n", gst_pad_get_name (muxer_v_in));
	
	GstPad *acodec_out, *vcodec_out;
	acodec_out = gst_element_get_static_pad(data.aqueue, "src");
	vcodec_out = gst_element_get_static_pad(data.vqueue, "src");
	g_print ("Obtained request pad %s for audio branch.\n", gst_pad_get_name (acodec_out));
	g_print ("Obtained request pad %s for video branch.\n", gst_pad_get_name (vcodec_out));
	
	
	if (gst_pad_link (acodec_out, muxer_a_in) != GST_PAD_LINK_OK ||
        gst_pad_link (vcodec_out, muxer_v_in) != GST_PAD_LINK_OK) {
		g_printerr ("Links could not be linked.\n");
		gst_object_unref (data.pipeline);
		return -1;
	}
	
	/* Set the URI to play */
	g_object_set (data.filesrc, "location", "sample.mp4", NULL);
#ifdef TCPSINK
	g_object_set (data.sink, "host", "127.0.0.1", NULL);
	g_object_set (data.sink, "port", 8080, NULL);
#else
#ifdef HLSSINK
	g_object_set (data.sink, "max-files", "5", NULL);
	g_object_set (data.sink, "playlist-root", "http://localhost", NULL);
	g_object_set (data.sink, "playlist-location", "/var/www/localhost/htdocs", NULL);
	g_object_set (data.sink, "location", "/var/www/localhost/htdocs", NULL);
#else
	g_object_set (data.sink, "location", "transcoded.webm", NULL);
#endif
#endif

#ifdef MP4MUX
	g_object_set (data.muxer, "fragment-duration", 100, NULL);
#endif

	/* Connect to the pad-added signal */
	g_signal_connect (data.decode, "pad-added", G_CALLBACK (pad_added_handler), &data);
	g_signal_connect (data.decode, "autoplug-continue", G_CALLBACK (autoplug_continue_cb), &data);

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
static void pad_added_handler (GstElement *src, GstPad *new_pad, Elements *data) {
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
		sink_pad = gst_element_get_static_pad (data->aconvert, "sink");
	}
	
	if (g_str_has_prefix (new_pad_type, "video/x-raw")) {
		sink_pad = gst_element_get_static_pad (data->vconvert, "sink");
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
	gst_object_unref (sink_pad);
}