#include "discover.hh"
#include <endpoints.hh>

using std::string;
using std::map;
using namespace rapidjson;
namespace spd = spdlog;




void discover_uri(Http::ResponseWriter &resp, string uri){

    /* Get info about streams */
    pads_struct data;
    GError *err = NULL;
    memset (&data, 0, sizeof (data));
    data.response = &resp;
    data.discoverer = gst_discoverer_new (5 * GST_SECOND, &err);
    data.doc = new Document;
    data.doc->SetObject();
    data.alloc = &data.doc->GetAllocator();
    Value error;
    if (!data.discoverer) {
        error.SetString(StringRef(g_strconcat("Error creating discoverer instance: ", err->message, NULL)));
        g_print ("Error creating discoverer instance: %s\n", err->message);
        g_clear_error (&err);
        goto exit;
        //return;
    }
    /* Connecting signals */
    g_signal_connect (data.discoverer, "discovered", G_CALLBACK (on_discovered_cb), &data);
    g_signal_connect (data.discoverer, "finished", G_CALLBACK (on_finished_cb), &data);
    gst_discoverer_start (data.discoverer);
    if (!gst_discoverer_discover_uri_async (data.discoverer, uri.c_str())) {
        error.SetString(StringRef("Failed to start discovering URI"));
        g_print ("Failed to start discovering URI '%s'\n", uri.c_str());
        g_object_unref (data.discoverer);
        goto exit;
        //return;
    }
    data.loop = g_main_loop_new (NULL, FALSE);
    /* Starting loop, needed to work properly */
    g_main_loop_run (data.loop);

    /* Stop the discoverer process */
    gst_discoverer_stop (data.discoverer);

    /* Free resources */
    g_object_unref (data.discoverer);
    g_main_loop_unref (data.loop);
    return;

    exit:
    Value errorsmsg;
    errorsmsg.SetString(StringRef("error"));
    data.doc->AddMember(errorsmsg, error, *data.alloc);
    StringBuffer strbuf;
    Writer<StringBuffer> writer(strbuf);
    data.doc->Accept(writer);
    data.response->send(Http::Code::Internal_Server_Error, strbuf.GetString());


}
static void print_tag_foreach (const GstTagList *tags, const gchar *tag, gpointer data) {
    /* Add tags to doc, name taken form tags */
    GValue val = { 0, };
    gchar *str;
    //gint depth = GPOINTER_TO_INT (user_data);

    pads_struct *datas = (pads_struct *) data;
    gst_tag_list_copy_value (&val, tags, tag);

    if (G_VALUE_HOLDS_STRING (&val))
        str = g_value_dup_string (&val);
    else
        str = gst_value_serialize (&val);
    Value name;
    name.SetString(StringRef(g_str_to_ascii(gst_tag_get_nick (tag),NULL)));
    Value msg;
    msg.SetString(StringRef(g_str_to_ascii(str,NULL)));
    datas->obj->AddMember(name,msg,*datas->alloc);

    //g_print ("%*s%s: %s\n", 2 * 2, " ", gst_tag_get_nick (tag), str);
    g_free (str);

    g_value_unset (&val);
}

/* Print information regarding a stream */
static void print_stream_info (GstDiscovererStreamInfo *info, gint depth, pads_struct *data) {
    /* Add stream information to json if it's audio or video */
    gchar *desc = NULL;

    GstCaps *caps;
    const GstTagList *tags;
    Value obj(kObjectType);
    caps = gst_discoverer_stream_info_get_caps (info);

    if (caps) {
        if (gst_caps_is_fixed (caps))
            desc = gst_pb_utils_get_codec_description (caps);
        else
            desc = gst_caps_to_string (caps);
        gst_caps_unref (caps);
    }
    if(g_strcmp0("audio",gst_discoverer_stream_info_get_stream_type_nick (info))==0){
        Value str;
        Value name;
        name.SetString(StringRef("audio"));
        if(desc)
            str.SetString(StringRef(g_str_to_ascii(desc, NULL)));
        else
            str.SetString(StringRef(g_str_to_ascii("", NULL)));
        obj.AddMember(name, str, *data->alloc);
        name.SetString(StringRef("streamid"));
        str.SetString(StringRef(g_str_to_ascii(gst_discoverer_stream_info_get_stream_id(info),NULL)));
        obj.AddMember(name,str,*data->alloc);
        data->obj = &obj;
        data->audion++;
        /* If there are any tags, add them */
        tags = gst_discoverer_stream_info_get_tags (info);
        if (tags) {
            //g_print ("%*sTags:\n", 2 * (depth + 1), " ");
            gst_tag_list_foreach (tags, print_tag_foreach, (gpointer) data);
        }
        data->audio->PushBack(*data->obj,*data->alloc);

    }
    if(g_strcmp0("video",gst_discoverer_stream_info_get_stream_type_nick (info))==0){
        Value str;

        Value name;
        if(desc)
            str.SetString(StringRef(g_str_to_ascii(desc, NULL)));
        else
            str.SetString(StringRef(g_str_to_ascii("", NULL)));

        name.SetString(StringRef("video"));
        obj.AddMember(name, str, *data->alloc);
        name.SetString(StringRef("streamid"));
        str.SetString(StringRef(g_str_to_ascii(gst_discoverer_stream_info_get_stream_id(info),NULL)));
        obj.AddMember(name, str, *data->alloc);
        data->obj = &obj;
        data->videon++;
        tags = gst_discoverer_stream_info_get_tags (info);
        /* If there are any tags, add them */
        if (tags) {
            gst_tag_list_foreach (tags, print_tag_foreach, (gpointer) data);
        }
        data->video->PushBack(*data->obj,*data->alloc);

    }
    //g_print ("%*s%s: %s\n", 2 * depth, " ", gst_discoverer_stream_info_get_stream_type_nick (info), (desc ? desc : ""));

    if (desc) {
        g_free (desc);
        desc = NULL;
    }

}

/* Add information regarding a stream and its substreams, if any */
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
    Value errors;


    data->audio = new Value;
    data->video = new Value;
    data->audio->SetArray();
    data->video->SetArray();

    uri = gst_discoverer_info_get_uri (info);
    result = gst_discoverer_info_get_result (info);
    switch (result) {
        case GST_DISCOVERER_URI_INVALID:
            errors.SetString(StringRef("Invalid URI"));
            g_print ("Invalid URI '%s'\n", uri);
            break;
        case GST_DISCOVERER_ERROR:

            g_print ("Discoverer error: %s\n", err->message);
            errors.SetString(StringRef(g_strconcat("Discoverer error: ", err->message, NULL)));
            break;
        case GST_DISCOVERER_TIMEOUT:
            errors.SetString(StringRef("Timeout"));
            g_print ("Timeout\n");
            break;
        case GST_DISCOVERER_BUSY:
            errors.SetString(StringRef("Busy"));
            g_print ("Busy\n");
            break;
        case GST_DISCOVERER_MISSING_PLUGINS:{
            const GstStructure *s;
            gchar *str;

            s = gst_discoverer_info_get_misc (info);
            str = gst_structure_to_string (s);
            g_print ("Missing plugins: %s\n", str);
            errors.SetString(StringRef(g_strconcat("Missing plugins ", str, NULL)));
            g_free (str);
            break;
        }
        case GST_DISCOVERER_OK:
            g_print ("Discovered '%s'\n", uri);
            break;
    }

    if (result != GST_DISCOVERER_OK) {

        g_printerr ("This URI cannot be played\n");
        Value e;
        e.SetString(StringRef("error"));
        data->doc->AddMember(e, errors, *data->alloc);
        return;
    }

    /* If we got no error, show the retrieved information */

    g_print ("Duration: %" GST_TIME_FORMAT "\n", GST_TIME_ARGS (gst_discoverer_info_get_duration (info)));

//    tags = gst_discoverer_info_get_tags (info);
//    Value tag(kObjectType);
//    if (tags) {
//        //g_print ("Tags:\n");
//        data->obj = &tag;
//        gst_tag_list_foreach (tags, print_tag_foreach, (gpointer) data);
//    }

    //   g_print ("Seekable: %s\n", (gst_discoverer_info_get_seekable (info) ? "yes" : "no"));

    //   g_print ("\n");

    sinfo = gst_discoverer_info_get_stream_info (info);
    if (!sinfo)
        return;

//    g_print ("Stream information:\n");

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
//    Value container ;
//    container.SetString(StringRef("container_tags"),*data->alloc);
    obj.AddMember(audio_n, Value(data->audion), *data->alloc);
    obj.AddMember(video_n, Value(data->videon), *data->alloc);
//    data->doc->AddMember(container, *data->obj, *data->alloc);
    data->doc->AddMember(data_count, obj , *data->alloc);
    data->doc->AddMember(audio, *data->audio, *data->alloc);
    data->doc->AddMember(video, *data->video, *data->alloc);
}

/* This function is called when the discoverer has finished examining
 * all the URIs we provided.*/
static void on_finished_cb (GstDiscoverer *discoverer, pads_struct *data) {
    //  g_print ("Finished discovering\n");
    StringBuffer strbuf;
    Writer<StringBuffer> writer(strbuf);
    data->doc->Accept(writer);
    //g_print("%s\n", strbuf.GetString());
    if(!data->doc->HasMember("error"))
        data->response->send(Http::Code::Ok, strbuf.GetString());
    else
        data->response->send(Http::Code::Bad_Request, strbuf.GetString());
    g_main_loop_quit (data->loop);
}
