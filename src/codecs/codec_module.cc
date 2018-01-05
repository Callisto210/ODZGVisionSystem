extern "C" {
#include <gst/gst.h>
#include <gst/gstbin.h>
}
#include <string.h>
#include "codec_module.hh"

#define WEBM

int magic(Elements data, config_struct conf) {
	GstBus *bus;
	GstMessage *msg;
	GstStateChangeReturn ret;
	gboolean terminate = FALSE;

	data.ptr = &conf;

	if (elements_has_null_field(&data)) {
		g_printerr ("Not all elements could be created.\n");
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
#endif
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
				(GstMessageType)(GST_MESSAGE_STATE_CHANGED | GST_MESSAGE_ERROR | GST_MESSAGE_EOS));

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
	const gchar *streamid = NULL;
	config_struct *conf = (config_struct *)data->ptr;

	g_print ("Received new pad '%s' from '%s':\n", GST_PAD_NAME (new_pad), GST_ELEMENT_NAME (src));

	/* Check the new pad's type */
	new_pad_caps = gst_pad_query_caps (new_pad, NULL);
	new_pad_struct = gst_caps_get_structure (new_pad_caps, 0);
	new_pad_type = gst_structure_get_name (new_pad_struct);
	streamid = gst_pad_get_stream_id(new_pad);
	g_print("new_pad_type: %s \n", new_pad_type);
	g_print("streamid: %s \n", streamid);

	if (g_str_has_prefix (new_pad_type, "audio/x-raw")) {
		if (conf->audio_stream.empty() || conf->audio_stream.compare(streamid) == 0) {
			if (data->aconvert != NULL)
				sink_pad = gst_element_get_static_pad (data->aconvert, "sink");
			else {
				g_print ("Audio not selected for transcoding. Skip \n");
				goto exit;
			}
		}
	}
	else if (g_str_has_prefix (new_pad_type, "video/x-raw")) {
		if (conf->video_stream.empty() || conf->video_stream.compare(streamid) == 0) {
			if (data->vconvert != NULL)
				sink_pad = gst_element_get_static_pad (data->vconvert, "sink");
			else {
				g_print ("Video not selected for transcoding. Skip \n");
				goto exit;
			}
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

int elements_has_null_field(Elements* data)
{
    string reason;

	if(data != NULL)
	if(!data->decode)
		reason = "decode";
	else if(!data->aconvert)
		goto skip_audio;
	else if(!data->aqueue)
		reason = "aqueue";
	else if(!data->acodec)
		reason = "acodec";

	skip_audio:
	if(!data->vcodec)
		goto skip_video;
	else if(!data->vconvert)
		reason = "vconvert";
	else if(!data->vqueue)
		reason = "vqueue";
	
	skip_video:
	if(!data->muxer)
		reason = "muxer";
	else if(!data->sink)
		reason = "sink";

    if(reason.length() > 0) {
        g_print("%s element can't be created\n", reason.c_str());
		return (1);
	}
	else
		return (0);
}
