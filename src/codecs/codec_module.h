/*
 * =====================================================================================
 *
 *       Filename:  codec_module.h
 *
 *    Description:  
 *
 *        Version:  1.0
 *        Created:  02.11.2017 23:49:50
 *       Revision:  none
 *       Compiler:  gcc
 *
 *   Organization:  
 *
 * =====================================================================================
 */

#ifndef __CODEC_MODULE_H
#define __CODEC_MODULE_H

#include <gst/gst.h>
#include <gst/gstbin.h>
#include <jsmn/jsmn.h>
#include <string.h>
#include "mux.h"
#include "sink.h"


/* Structure to contain all our information, so we can pass it to callbacks */
typedef struct _elements {
    GstElement *pipeline;
    GstElement *src;
    GstElement *decode;

    GstElement *aconvert;
    GstElement *vconvert;
    GstElement *acodec;
    GstElement *vcodec;
    GstElement *aqueue;
    GstElement *vqueue;

    GstElement* sink;
    GstElement* muxer;
} Elements;


int elements_has_null_field(Elements* elements);

int test_pipeline();

int magic(Elements data, e_sink_t sink_type, e_mux_t mux_type);


#endif
