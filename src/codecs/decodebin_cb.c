#include <gst/gst.h>

gboolean autoplug_continue_cb(GstBin *bin, GstPad *pad,
    GstCaps *caps, gpointer user_data)
{
	/* Simply print information about streams */
	g_print("Source info:\n%s\n", gst_caps_to_string(caps));
	
	return (TRUE);
	
}
