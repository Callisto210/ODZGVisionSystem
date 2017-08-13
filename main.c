#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libswscale/swscale.h>
#include <libavutil/avconfig.h>
#include <libavutil/pixdesc.h>
#include <libavfilter/avfiltergraph.h>

#include "filter_graph.h"

/*
 * AVFormatContext Fields description at https://ffmpeg.org/doxygen/3.2/structAVFormatContext.html
 */

static FilteringContext *filter_ctx;

static int open_input_file(AVFormatContext **pFormatCtx, const char *filename) {
	int ret;
	unsigned int i;
	
	AVStream *pStream;
	AVCodec *pCodec;
	AVCodecContext *pCodec_ctx;
	
	*pFormatCtx = NULL;
	if((ret = avformat_open_input(pFormatCtx, filename, NULL, NULL))!=0) {
		av_log(NULL, AV_LOG_ERROR, "Cannot open input file\n");
		return ret;
	}

	//This function populates pFormatCtx->streams (and pFormatCtx->nb_streams) with the proper information.	
	if((ret = avformat_find_stream_info(*pFormatCtx, NULL)) < 0) {
		av_log(NULL, AV_LOG_ERROR, "Cannot find stream information\n");
		return ret;
	}
		
	// Dump information about file onto standard error
	av_dump_format(*pFormatCtx, 0, filename, 0);
	
	for (i = 0; i < (*pFormatCtx)->nb_streams; i++) {
		pStream = (*pFormatCtx)->streams[i];
		pCodec = avcodec_find_decoder(pStream->codecpar->codec_id);
		pCodec_ctx = avcodec_alloc_context3(pCodec);
		avcodec_parameters_to_context(pCodec_ctx, pStream->codecpar);
		/* Reencode video & audio and remux subtitles etc. */
		if (pCodec_ctx->codec_type == AVMEDIA_TYPE_VIDEO || pCodec_ctx->codec_type == AVMEDIA_TYPE_AUDIO) {
			/* Open decoder */
			ret = avcodec_open2(pCodec_ctx, pCodec, NULL);
			if (ret < 0) {
				av_log(NULL, AV_LOG_ERROR, "Failed to open decoder for stream #%u\n", i);
				avcodec_free_context(&pCodec_ctx);
				return ret;
			}
		}
	}
	
	avcodec_free_context(&pCodec_ctx);
	return 0;
}

static int create_output_context(AVFormatContext **pFormatCtx, const char *filename) {
	*pFormatCtx = NULL;
    avformat_alloc_output_context2(pFormatCtx, NULL, NULL, filename);
    if (!*pFormatCtx) {
        av_log(NULL, AV_LOG_ERROR, "Could not create output context\n");
        return AVERROR_UNKNOWN;
    }
    return 0;
}

/* You need:
 * - context 
 * - codec (to encode) AVCodec
 * - stream parameters AVCodecContext
 */

static int put_stream_into_context(AVFormatContext *pFormatCtx, AVCodec *encoder, AVCodecContext *enc_ctx) {
    int ret;	
	
	AVStream *out_stream = avformat_new_stream(pFormatCtx, NULL);
	if (!out_stream) {
		av_log(NULL, AV_LOG_ERROR, "Failed allocating output stream\n");
		return AVERROR_UNKNOWN;
	}
	
	if (!encoder) {
		av_log(NULL, AV_LOG_FATAL, "No codec passed to put_stream_into_context\n");
		return AVERROR_INVALIDDATA;
	}

	if(!enc_ctx) {
		av_log(NULL, AV_LOG_FATAL, "No codec context passed to put_stream_into_context\n");
		return AVERROR_INVALIDDATA;
	}
	avcodec_parameters_from_context(out_stream->codecpar, enc_ctx);
		
	/* Third parameter can be used to pass settings to encoder */
	ret = avcodec_open2(enc_ctx, encoder, NULL);
	if (ret < 0) {
		av_log(NULL, AV_LOG_ERROR, "Cannot open video encoder for stream\n");
		return ret;
	}

	return 0;
}

/*
 * This function populates codec context with parameters
 * such height, width, aspect, etc.
 * 
 */

static void populate_codec_context_video(AVCodecContext *enc_ctx,
    AVCodecContext *dec_ctx, enum AVPixelFormat pix_fmt) {
	enc_ctx->height = dec_ctx->height;
	enc_ctx->width = dec_ctx->width;
	enc_ctx->sample_aspect_ratio = dec_ctx->sample_aspect_ratio;
	enc_ctx->pix_fmt = pix_fmt;
	/* video time_base can be set to whatever is handy and supported by encoder */
	//enc_ctx->time_base = dec_ctx->time_base;
	enc_ctx->time_base = (AVRational){1, 30};
	
#ifdef DEBUG
	printf("timebase: %d %d\n", enc_ctx->time_base.num, enc_ctx->time_base.den);
#endif
	
	return;
}

static void populate_codec_context_audio(AVCodecContext *enc_ctx,
    AVCodecContext *dec_ctx, enum AVSampleFormat sample_fmt) {
	enc_ctx->sample_rate = dec_ctx->sample_rate;
	enc_ctx->channel_layout = dec_ctx->channel_layout;
	enc_ctx->channels = av_get_channel_layout_nb_channels(enc_ctx->channel_layout);
	enc_ctx->sample_fmt = sample_fmt;
	enc_ctx->time_base = (AVRational){1, enc_ctx->sample_rate};
	return;
}


/* Helpful when you simply want to copy stream 
 * Adds input stream into output file context
 */
static int remux_stream(AVFormatContext *pFormatCtx, AVStream *in_stream) {
	int ret;
	AVCodecContext *ctx;

	AVStream *out_stream = avformat_new_stream(pFormatCtx, NULL);
	if (!out_stream) {
		av_log(NULL, AV_LOG_ERROR, "Failed allocating output stream\n");
		return AVERROR_UNKNOWN;
	}
	
	ctx = avcodec_alloc_context3(NULL);
	ret = avcodec_parameters_to_context(ctx, in_stream->codecpar);
	if(ret >= 0) {
		ret = avcodec_parameters_from_context(out_stream->codecpar, ctx);
	}
	//ret = avcodec_copy_context(out_stream->codec, in_stream->codec);
	if (ret < 0) {
		av_log(NULL, AV_LOG_ERROR, "Copying stream context failed\n");
		return ret;
	}
	return 0;
}

static int
setup_stream(AVFormatContext *in_ctx, AVFormatContext *out_ctx, int stream)
{
	AVCodec *decoder, *encoder;
	AVCodecContext *dec_ctx, *enc_ctx;
	AVStream *in_stream;
	int ret;
	
	in_stream = in_ctx->streams[stream];
	
	decoder = avcodec_find_decoder(in_stream->codecpar->codec_id);
	dec_ctx = avcodec_alloc_context3(decoder);
	avcodec_parameters_to_context(dec_ctx, in_stream->codecpar);
	
	if(dec_ctx->codec_type == AVMEDIA_TYPE_VIDEO) {
#ifdef DEBUG
		printf("Stream: %d, VIDEO\n", stream);
#endif
		encoder = avcodec_find_encoder(AV_CODEC_ID_VP8);
		enc_ctx = avcodec_alloc_context3(encoder);
		populate_codec_context_video(enc_ctx, dec_ctx, encoder->pix_fmts[0]);
		
		/* Init filter graph for stream */
		ret = init_filter_graph_video(&filter_ctx[stream], encoder->pix_fmts[0],
            "null");
		
	}
	else if (dec_ctx->codec_type == AVMEDIA_TYPE_AUDIO) {
#ifdef DEBUG
		printf("Stream: %d, AUDIO\n", stream);
#endif
		encoder = avcodec_find_encoder(AV_CODEC_ID_MP3);
		enc_ctx = avcodec_alloc_context3(encoder);
		populate_codec_context_audio(enc_ctx, dec_ctx, encoder->sample_fmts[0]);
		
		/*Init filter graph for stream */
		ret = init_filter_graph_audio(&filter_ctx[stream], dec_ctx, enc_ctx,
            "anull");
	}
	else if (dec_ctx->codec_type == AVMEDIA_TYPE_UNKNOWN) {
		av_log(NULL, AV_LOG_FATAL, "Elementary stream is of unknown type, cannot proceed\n");
		return AVERROR_INVALIDDATA;
	}
	
	if (in_ctx->oformat != NULL)
		if (in_ctx->oformat->flags & AVFMT_GLOBALHEADER)
			enc_ctx->flags |= AV_CODEC_FLAG_GLOBAL_HEADER;
	
	put_stream_into_context(out_ctx, encoder, enc_ctx);
	
	return (0);
}

static int alloc_filtering_contexts(unsigned int nb_streams)
{
    unsigned int i;
    filter_ctx = av_malloc_array(nb_streams, sizeof(*filter_ctx));
    if (!filter_ctx)
        return AVERROR(ENOMEM);
    for (i = 0; i < nb_streams; i++) {
        filter_ctx[i].buffersrc_ctx  = NULL;
        filter_ctx[i].buffersink_ctx = NULL;
        filter_ctx[i].filter_graph   = NULL;
    }
    return 0;
}

static int encode_write_frame(AVFormatContext *ofmt_ctx,
    AVFrame *filt_frame, unsigned int stream_index, int *got_frame) {
    int ret;
    AVPacket enc_pkt;
    int (*enc_func)(AVCodecContext *, AVPacket *, const AVFrame *, int *) =
        (ofmt_ctx->streams[stream_index]->codec->codec_type ==
         AVMEDIA_TYPE_VIDEO) ? avcodec_encode_video2 : avcodec_encode_audio2;

    av_log(NULL, AV_LOG_INFO, "Encoding frame\n");
    /* encode filtered frame */
    enc_pkt.data = NULL;
    enc_pkt.size = 0;
    av_init_packet(&enc_pkt);
    ret = enc_func(ofmt_ctx->streams[stream_index]->codec, &enc_pkt,
        filt_frame, got_frame);
    av_frame_free(&filt_frame);
    if (ret < 0)
        return ret;
    if (!(*got_frame)) {
		av_log(NULL, AV_LOG_DEBUG, "avcodec_encode got_frame set to 0!\n");
        return AVERROR(EINVAL);
	}
    /* prepare packet for muxing */
    enc_pkt.stream_index = stream_index;
    av_packet_rescale_ts(&enc_pkt,
                         ofmt_ctx->streams[stream_index]->codec->time_base,
                         ofmt_ctx->streams[stream_index]->time_base);
    av_log(NULL, AV_LOG_DEBUG, "Muxing frame\n");
    /* mux encoded frame */
    ret = av_interleaved_write_frame(ofmt_ctx, &enc_pkt);
    return ret;
}

static int flush_encoder(AVFormatContext *ofmt_ctx,
    unsigned int stream_index)
{
    int ret;
    int got_frame;
    if (!(ofmt_ctx->streams[stream_index]->codec->codec->capabilities &
                AV_CODEC_CAP_DELAY))
        return 0;
    while (1) {
        av_log(NULL, AV_LOG_INFO, "Flushing stream #%u encoder\n", stream_index);
        ret = encode_write_frame(ofmt_ctx, NULL, stream_index, &got_frame);
        if (ret < 0)
            break;
        if (!got_frame)
            return 0;
    }
    return ret;
}

int main(int argc, char *argv[]) {
	if (argc != 2)
		return 1;
		
	const char *filename = "./transcoded.mkv";
	int ret;
		
	av_register_all();
	avfilter_register_all();
	
	AVFormatContext *pFormatCtx;
	open_input_file(&pFormatCtx, argv[1]);
	
	/* opening output file is something like:
	 * - create output context
	 * - populate context with streams you want to put into file
	 * - of course set stream properties (codec, height, width, aspect)
	 * - open file to write
	 * - init muxer, and write file header
	*/
	
	AVFormatContext *out_ctx;
	create_output_context(&out_ctx, filename);
	alloc_filtering_contexts(pFormatCtx->nb_streams);
	
	setup_stream(pFormatCtx, out_ctx, 0);
	setup_stream(pFormatCtx, out_ctx, 1);
	 
	// Dump information about file onto standard error
	av_dump_format(out_ctx, 0, filename, 1);
	
	/*Here setting stream ends */
	
	if (!(out_ctx->oformat->flags & AVFMT_NOFILE)) {
        ret = avio_open(&out_ctx->pb, filename, AVIO_FLAG_WRITE);
        if (ret < 0) {
            av_log(NULL, AV_LOG_ERROR, "Could not open output file '%s'", filename);
            return ret;
        }
    }
    /* init muxer, write output file header */
    ret = avformat_write_header(out_ctx, NULL);
    if (ret < 0) {
        av_log(NULL, AV_LOG_ERROR, "Error occurred when opening output file\n");
        return ret;
    }
	
#if 0
	/* main loop */
	
	AVPacket packet = { .data = NULL, .size = 0 };
	int got_frame, i, stream_index;
	
	
	/* read all packets */
    while (1) {
        if ((ret = av_read_frame(pFormatCtx, &packet)) < 0)
            break;
        stream_index = packet.stream_index;
        type = pFormatCtx->streams[packet.stream_index]->codec->codec_type;
        av_log(NULL, AV_LOG_DEBUG, "Demuxer gave frame of stream_index %u\n",
                stream_index);
        if (filter_ctx[stream_index].filter_graph) {
            av_log(NULL, AV_LOG_DEBUG, "Going to reencode&filter the frame\n");
            frame = av_frame_alloc();
            if (!frame) {
                ret = AVERROR(ENOMEM);
                break;
            }
            av_packet_rescale_ts(&packet,
                                 pFormatCtx->streams[stream_index]->time_base,
                                 pFormatCtx->streams[stream_index]->codec->time_base);
            dec_func = (type == AVMEDIA_TYPE_VIDEO) ? avcodec_decode_video2 :
                avcodec_decode_audio4;
            ret = dec_func(pFormatCtx->streams[stream_index]->codec, frame,
                    &got_frame, &packet);
            if (ret < 0) {
                av_frame_free(&frame);
                av_log(NULL, AV_LOG_ERROR, "Decoding failed\n");
                break;
            }
            if (got_frame) {
                frame->pts = av_frame_get_best_effort_timestamp(frame);
                ret = filter_encode_write_frame(frame, stream_index);
                av_frame_free(&frame);
                if (ret < 0)
                    goto end;
            } else {
                av_frame_free(&frame);
            }
        } else {
            /* remux this frame without reencoding */
            av_packet_rescale_ts(&packet,
                                 pFormatCtx->streams[stream_index]->time_base,
                                 out_ctx->streams[stream_index]->time_base);
            ret = av_interleaved_write_frame(out_ctx, &packet);
            if (ret < 0)
                goto end;
        }
        av_packet_unref(&packet);
    }
    /* flush filters and encoders */
    for (i = 0; i < pFormatCtx->nb_streams; i++) {
        /* flush filter */
        if (!filter_ctx[i].filter_graph)
            continue;
        ret = filter_encode_write_frame(NULL, i);
        if (ret < 0) {
            av_log(NULL, AV_LOG_ERROR, "Flushing filter failed\n");
            goto end;
        }
        /* flush encoder */
        ret = flush_encoder(i);
        if (ret < 0) {
            av_log(NULL, AV_LOG_ERROR, "Flushing encoder failed\n");
            goto end;
        }
    }
    av_write_trailer(out_ctx);
end:
    av_packet_unref(&packet);
    av_frame_free(&frame);
    for (i = 0; i < pFormatCtx->nb_streams; i++) {
        avcodec_close(pFormatCtx->streams[i]->codec);
        if (out_ctx && out_ctx->nb_streams > i && out_ctx->streams[i] && out_ctx->streams[i]->codec)
            avcodec_close(out_ctx->streams[i]->codec);
        if (filter_ctx && filter_ctx[i].filter_graph)
            avfilter_graph_free(&filter_ctx[i].filter_graph);
    }
    av_free(filter_ctx);
    avformat_close_input(&pFormatCtx);
    if (out_ctx && !(out_ctx->oformat->flags & AVFMT_NOFILE))
        avio_closep(&out_ctx->pb);
    avformat_free_context(out_ctx);
    if (ret < 0)
        av_log(NULL, AV_LOG_ERROR, "Error occurred: %s\n", av_err2str(ret));
    return ret ? 1 : 0;
    
#endif
	
}
