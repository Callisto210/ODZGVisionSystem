/* Filter graph implementation */


#include "filter_graph.hh"
#include "../ffmpeg/AVFilter.hh"

FilteringContext::FilteringContext() {
    _log = spdlog::get("filter");
}
/* dec_ctx is used only for createing argument string to graph source */
int FilteringContext::init_filter_graph_video(
        AVPixelFormat pix_fmt,
        const char *filter_spec,
        AVCodecContext *dec_ctx)
{
    char args[512];
    int ret = 0;
    AVFilter *buffersrc = nullptr;
    AVFilter *buffersink = nullptr;
    AVFilterContext *buffersrc_ctx = nullptr;
    AVFilterContext *buffersink_ctx = nullptr;
    ffmpeg::AVFilterInOut outputs;
    ffmpeg::AVFilterInOut inputs;
    ffmpeg::AVFilterGraph filter_graph;


	buffersrc = avfilter_get_by_name("buffer");
	buffersink = avfilter_get_by_name("buffersink");
	if (!buffersrc || !buffersink) {
		av_log(nullptr, AV_LOG_ERROR, "filtering source or sink element not found\n");
		ret = AVERROR_UNKNOWN;
		goto end;
	}

	snprintf(args, sizeof(args),
		"video_size=%dx%d:pix_fmt=%d:time_base=%d/%d:pixel_aspect=%d/%d",
		dec_ctx->width, dec_ctx->height, dec_ctx->pix_fmt,
//		dec_ctx->time_base.num, dec_ctx->time_base.den,
		1, 30,
		dec_ctx->sample_aspect_ratio.num,
		dec_ctx->sample_aspect_ratio.den);

		_log->debug("{}: Argument list for video graph filtering source\n{}\n", __func__, args);

	ret = avfilter_graph_create_filter(&buffersrc_ctx, buffersrc, "in",
			args, nullptr, filter_graph.filter());
	if (ret < 0) {
		av_log(nullptr, AV_LOG_ERROR, "Cannot create buffer source\n");
		goto end;
	}


	_log->debug("{}: Buffer source (video filtering graph in) created.", __func__);
	ret = avfilter_graph_create_filter(&buffersink_ctx, buffersink, "out",
                                       nullptr, nullptr, filter_graph.filter());
	if (ret < 0) {
		av_log(nullptr, AV_LOG_ERROR, "Cannot create buffer sink.\n");
		goto end;
	}
	_log->debug("{}: Buffer sink (video filtering graph out) created.", __func__);

	ret = av_opt_set_bin(buffersink_ctx, "pix_fmts", (uint8_t *)&pix_fmt, sizeof(pix_fmt),
	    AV_OPT_SEARCH_CHILDREN);
	if (ret < 0) {
		av_log(nullptr, AV_LOG_ERROR, "Cannot set output pixel format.\n");
		goto end;
	}
	_log->debug("{}: pix_fmt for video filtering graph output - set.", __func__);

    /* Endpoints for the filter graph. */
    outputs.filter()->name       = av_strdup("in");
    outputs.filter()->filter_ctx = buffersrc_ctx;
    outputs.filter()->pad_idx    = 0;
    outputs.filter()->next       = nullptr;
    inputs.filter()->name        = av_strdup("out");
    inputs.filter()->filter_ctx  = buffersink_ctx;
    inputs.filter()->pad_idx     = 0;
    inputs.filter()->next        = nullptr;
    if (!outputs.filter()->name || !inputs.filter()->name) {
        ret = AVERROR(ENOMEM);
        goto end;
    }
    if ((ret = avfilter_graph_parse_ptr(filter_graph.filter(), filter_spec,
                    inputs.ref(), outputs.ref(), nullptr)) < 0)
        goto end;
    filter_graph.config(nullptr);
    /* Fill FilteringContext */
    this->buffersrc_ctx = buffersrc_ctx;
    this->buffersink_ctx = buffersink_ctx;
    this->filter_graph = &filter_graph;
end:
    return ret;
}

int FilteringContext::init_filter_graph_audio(AVCodecContext *dec_ctx,
        AVCodecContext *enc_ctx, const char *filter_spec)
{
    char args[512];
    int ret = 0;
    AVFilter *buffersrc = nullptr;
    AVFilter *buffersink = nullptr;
    AVFilterContext *buffersrc_ctx = nullptr;
    AVFilterContext *buffersink_ctx = nullptr;
    ffmpeg::AVFilterInOut outputs;
    ffmpeg::AVFilterInOut inputs;
    ffmpeg::AVFilterGraph filter_graph;
	buffersrc = avfilter_get_by_name("abuffer");
	buffersink = avfilter_get_by_name("abuffersink");
	if (!buffersrc || !buffersink) {
		av_log(nullptr, AV_LOG_ERROR, "filtering source or sink element not found\n");
		ret = AVERROR_UNKNOWN;
		goto end;
	}
	if (!dec_ctx->channel_layout)
		dec_ctx->channel_layout = static_cast<uint64_t>(av_get_default_channel_layout(dec_ctx->channels));
			
	snprintf(args, sizeof(args),
		"time_base=%d/%d:sample_rate=%d:sample_fmt=%s:channel_layout=0x%" PRIx64,
		dec_ctx->time_base.num, dec_ctx->time_base.den, dec_ctx->sample_rate,
		av_get_sample_fmt_name(dec_ctx->sample_fmt),
		dec_ctx->channel_layout);

	ret = avfilter_graph_create_filter(&buffersrc_ctx, buffersrc, "in",
			args, nullptr, filter_graph.filter());
	if (ret < 0) {
		av_log(nullptr, AV_LOG_ERROR, "Cannot create audio buffer source\n");
		goto end;
	}
	ret = avfilter_graph_create_filter(&buffersink_ctx, buffersink, "out",
                                       nullptr, nullptr, filter_graph.filter());
	if (ret < 0) {
		av_log(nullptr, AV_LOG_ERROR, "Cannot create audio buffer sink\n");
		goto end;
	}
	ret = av_opt_set_bin(buffersink_ctx, "sample_fmts",
			(uint8_t*)&enc_ctx->sample_fmt, sizeof(enc_ctx->sample_fmt),
			AV_OPT_SEARCH_CHILDREN);
	if (ret < 0) {
		av_log(nullptr, AV_LOG_ERROR, "Cannot set output sample format\n");
		goto end;
	}
	ret = av_opt_set_bin(buffersink_ctx, "channel_layouts",
			(uint8_t*)&enc_ctx->channel_layout,
			sizeof(enc_ctx->channel_layout), AV_OPT_SEARCH_CHILDREN);
	if (ret < 0) {
		av_log(nullptr, AV_LOG_ERROR, "Cannot set output channel layout\n");
		goto end;
	}
	ret = av_opt_set_bin(buffersink_ctx, "sample_rates",
			(uint8_t*)&enc_ctx->sample_rate, sizeof(enc_ctx->sample_rate),
			AV_OPT_SEARCH_CHILDREN);
	if (ret < 0) {
		av_log(nullptr, AV_LOG_ERROR, "Cannot set output sample rate\n");
		goto end;
	}

    /* Endpoints for the filter graph. */
    outputs->name       = av_strdup("in");
    outputs->filter_ctx = buffersrc_ctx;
    outputs->pad_idx    = 0;
    outputs->next       = nullptr;
    inputs->name       = av_strdup("out");
    inputs->filter_ctx = buffersink_ctx;
    inputs->pad_idx    = 0;
    inputs->next       = nullptr;
    if (!outputs->name || !inputs->name) {
        ret = AVERROR(ENOMEM);
        goto end;
    }
    if ((ret = avfilter_graph_parse_ptr(filter_graph.filter(), filter_spec,
                    inputs.ref(), outputs.ref(), nullptr)) < 0)
        goto end;
    filter_graph.config(nullptr);
    /* Fill FilteringContext */
    this->buffersrc_ctx = buffersrc_ctx;
    this->buffersink_ctx = buffersink_ctx;
    this->filter_graph = &filter_graph;
end:
    return ret;
}

/*
 * Please provide me filter context
 * One input frame
 * Array of AVFrame* and its size
 * 
 * This function should return number of frames
 * In most cases it will be 1, but number can be higher
 * or even 0.
 *
 */

int FilteringContext::filter_encode_write_frame(
    AVFrame *frame, AVFrame **filt_frame, unsigned int nframes)
{
    int ret = 0;
    unsigned int i;
    av_log(nullptr, AV_LOG_INFO, "Pushing decoded frame to filters\n");
    /* push the decoded frame into the filtergraph */
    ret = av_buffersrc_add_frame_flags(this->buffersrc_ctx,
            frame, 0);
    if (ret < 0) {
        av_log(nullptr, AV_LOG_ERROR, "Error while feeding the filtergraph\n");
        return ret;
    }
    /* pull filtered frames from the filtergraph */
    for (i = 0; i < nframes; i++) {
        filt_frame[i] = av_frame_alloc();
        if (!filt_frame[i]) {
            ret = AVERROR(ENOMEM);
            break;
        }
        av_log(nullptr, AV_LOG_INFO, "Pulling filtered frame from filters\n");
        ret = av_buffersink_get_frame(this->buffersink_ctx,
                filt_frame[i]);
        if (ret < 0) {
            /* if no more frames for output - returns AVERROR(EAGAIN)
             * if flushed and no more frames for output - returns AVERROR_EOF
             */
            if (ret == AVERROR(EAGAIN) || ret == AVERROR_EOF)
                ret = 0;
            av_frame_free(&filt_frame[i]);
            return (i);
        }
        filt_frame[i]->pict_type = AV_PICTURE_TYPE_NONE;
    }
    return ret;
}
