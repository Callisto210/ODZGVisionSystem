#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libswscale/swscale.h>
#include <libavutil/avconfig.h>
#include <libavutil/pixdesc.h>
#include <libavfilter/avfiltergraph.h>

#include "filter_graph.h"
#include "log_common.h"

#define FABULOUS_CURRENT FABULOUS_INFO

#define MAX_FILTERED_FRAMES 100
/*
 * AVFormatContext Fields description at https://ffmpeg.org/doxygen/3.2/structAVFormatContext.html
 */

static FilteringContext *filter_ctx;
static AVCodecContext **codec_ctx;

static int open_input_file(AVFormatContext **pFormatCtx, const char *filename) {
	int ret;
	unsigned int i;
	
	AVStream *pStream;
	AVCodec *pCodec;
	
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
	
	codec_ctx = malloc(sizeof(AVCodecContext *) * (*pFormatCtx)->nb_streams);
	
	for (i = 0; i < (*pFormatCtx)->nb_streams; i++) {
		pStream = (*pFormatCtx)->streams[i];
		pCodec = avcodec_find_decoder(pStream->codecpar->codec_id);
		codec_ctx[i] = avcodec_alloc_context3(pCodec);
		avcodec_parameters_to_context(codec_ctx[i], pStream->codecpar);
		/* Reencode video & audio and remux subtitles etc. */
		if (codec_ctx[i]->codec_type == AVMEDIA_TYPE_VIDEO || codec_ctx[i]->codec_type == AVMEDIA_TYPE_AUDIO) {
			/* Open decoder */
			ret = avcodec_open2(codec_ctx[i], pCodec, NULL);
			NSDI(avcodec_open2, ret);
			if (ret < 0) {
				av_log(NULL, AV_LOG_ERROR, "Failed to open decoder for stream #%u\n", i);
				avcodec_free_context(&codec_ctx[i]);
				return ret;
			}
		}
	}
	
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

static int put_stream_into_context(AVFormatContext *pFormatCtx, AVCodec *encoder, AVCodecContext *enc_ctx,
    AVDictionary **options) {
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
	ret = avcodec_open2(enc_ctx, encoder, options);
	if (ret < 0) {
		av_log(NULL, AV_LOG_ERROR, "Cannot open encoder for stream\n");
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
    AVCodecContext *dec_ctx, enum AVPixelFormat pix_fmt, AVDictionary **d) {
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
    AVCodecContext *dec_ctx, enum AVSampleFormat sample_fmt, AVDictionary **d) {
	enc_ctx->sample_rate = dec_ctx->sample_rate;
	enc_ctx->channel_layout = dec_ctx->channel_layout;
	enc_ctx->channels = av_get_channel_layout_nb_channels(enc_ctx->channel_layout);
	enc_ctx->sample_fmt = sample_fmt;
	enc_ctx->time_base = (AVRational){1, enc_ctx->sample_rate};

	av_dict_set_int(d, "frame_size", 50000, 0);

#ifdef DEBUG
	printf("AUDIO: samplerate %d\n", enc_ctx->sample_rate);
#endif

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
setup_stream(AVFormatContext *in_ctx, AVFormatContext *out_ctx,
    AVCodecContext **enc_ctx, int stream)
{
	AVCodec *decoder, *encoder;
	AVCodecContext *dec_ctx;
	AVStream *in_stream;
	AVDictionary *options = NULL; /* New dictionary */
	AVDictionaryEntry *t = NULL;
	int ret = 0;
	
	in_stream = in_ctx->streams[stream];
	
	decoder = avcodec_find_decoder(in_stream->codecpar->codec_id);
	dec_ctx = avcodec_alloc_context3(decoder);
	avcodec_parameters_to_context(dec_ctx, in_stream->codecpar);
	
	if(dec_ctx->codec_type == AVMEDIA_TYPE_VIDEO) {
#ifdef DEBUG
		printf("Stream: %d, VIDEO\n", stream);
#endif
		encoder = avcodec_find_encoder(AV_CODEC_ID_VP8);
		if (encoder == NULL)
			av_log(NULL, AV_LOG_FATAL, "Can't find video encoder\n");
		*enc_ctx = avcodec_alloc_context3(encoder);
	//	NSDP(*enc_ctx = avcodec_alloc_context3, encoder);
		populate_codec_context_video(*enc_ctx, dec_ctx, encoder->pix_fmts[0], &options);
		
		/* Init filter graph for stream */
		ret = init_filter_graph_video(&filter_ctx[stream], encoder->pix_fmts[0],
            "null", dec_ctx);
		
	}
	else if (dec_ctx->codec_type == AVMEDIA_TYPE_AUDIO) {
#ifdef DEBUG
		printf("Stream: %d, AUDIO\n", stream);
#endif
		encoder = avcodec_find_encoder(AV_CODEC_ID_VORBIS);
		if (encoder == NULL)
			av_log(NULL, AV_LOG_FATAL, "Can't find audio encoder\n");
		*enc_ctx = avcodec_alloc_context3(encoder);
		populate_codec_context_audio(*enc_ctx, dec_ctx, encoder->sample_fmts[0], &options);
		
		/*Init filter graph for stream */
		ret = init_filter_graph_audio(&filter_ctx[stream], dec_ctx, *enc_ctx,
            "anull");
	}
	else if (dec_ctx->codec_type == AVMEDIA_TYPE_UNKNOWN) {
		av_log(NULL, AV_LOG_FATAL, "Elementary stream is of unknown type, cannot proceed\n");
		return AVERROR_INVALIDDATA;
	}
	
	if (in_ctx->oformat != NULL)
		if (in_ctx->oformat->flags & AVFMT_GLOBALHEADER)
			(*enc_ctx)->flags |= AV_CODEC_FLAG_GLOBAL_HEADER;
	
	put_stream_into_context(out_ctx, encoder, *enc_ctx, &options);
	while (t = av_dict_get(options, "", t, AV_DICT_IGNORE_SUFFIX)) {
		av_log(NULL, AV_LOG_INFO, "Option %s not found\n", t->key);
	}
	return (ret);
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
    AVCodecContext *enc_ctx, AVFrame **filt_frame, unsigned int nframes,
    unsigned int stream_index) {
	int ret;
	unsigned int i;
	AVPacket *enc_pkt;

	enc_pkt = av_packet_alloc();
	for (i = 0; i < nframes;) {
		av_log(NULL, AV_LOG_INFO, "Encoding frame\n");

		/* Send frame to encoder */
		if ((ret = avcodec_send_frame(enc_ctx, filt_frame[i])) != 0) {
			av_log(NULL, AV_LOG_ERROR, "Send frame failed with error: %d\n", ret);
			break;
		}

		/* Receive packet from encoder */
		if ((ret = avcodec_receive_packet(enc_ctx, enc_pkt)) != 0) {
			if (ret == AVERROR(EAGAIN)) {
				/* Output not available, send more packets */
				continue;
			}
			av_log(NULL, AV_LOG_ERROR, "Receive packet failed with error: %d\n", ret);
			break;
		}

		av_frame_free(&filt_frame[i++]);

		/* prepare packet for muxing */
		enc_pkt->stream_index = stream_index;
		av_packet_rescale_ts(enc_pkt, enc_ctx->time_base,
		    ofmt_ctx->streams[stream_index]->time_base);
		av_log(NULL, AV_LOG_DEBUG, "Muxing frame\n");
		/* mux encoded frame */
		ret = av_interleaved_write_frame(ofmt_ctx, enc_pkt);

	}

	av_packet_free(&enc_pkt);

	/* free rest of not freed frames */
	for (; i < nframes; i++)
		av_frame_free(&filt_frame[i]);

	return (ret);
}

static int flush_encoder(AVFormatContext *ofmt_ctx,
    AVCodecContext *codec_ctx, unsigned int stream_index)
{
    int ret;
    AVFrame *dummy_frame = NULL;

    if (!(codec_ctx->codec->capabilities &
                AV_CODEC_CAP_DELAY))
        return 0;
    while (1) {
        av_log(NULL, AV_LOG_INFO, "Flushing stream #%u encoder\n", stream_index);
        ret = encode_write_frame(ofmt_ctx, codec_ctx, &dummy_frame, 1,
            stream_index);
        if (ret < 0)
            break;
    }
    return ret;
}

static int filter_write_frame(AVFormatContext *out_ctx,
    AVFrame *frame, AVCodecContext *enc_ctx, unsigned int stream_index)
{
	AVFrame *filtered_frames[MAX_FILTERED_FRAMES];
	int ret;
	
	ret = filter_encode_write_frame(&filter_ctx[stream_index],
	    frame, filtered_frames, MAX_FILTERED_FRAMES);
	if (ret < 0) {
		av_log(NULL, AV_LOG_ERROR,
			"filter_encode_write_frame: returned %d\n", ret);
	}
	
	ret = encode_write_frame(out_ctx, enc_ctx, filtered_frames, ret,
	    stream_index);
	if (ret < 0) {
		av_log(NULL, AV_LOG_ERROR,
			"encode_write_frame: returned %d\n", ret);
	}
	
	return ret;
}

int main(int argc, char *argv[]) {
	if (argc != 2)
		return 1;
		
	const char *filename = "./transcoded.mkv";
	int ret;
	AVCodecContext **enc_codec_ctx;
		
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
	enc_codec_ctx = malloc(sizeof(*enc_codec_ctx) * pFormatCtx->nb_streams);
	
	setup_stream(pFormatCtx, out_ctx, &enc_codec_ctx[0], 0);
	//setup_stream(pFormatCtx, out_ctx, &enc_codec_ctx[1], 1);
	 
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
	
	
	/* main loop */
	AVPacket packet = { .data = NULL, .size = 0 };
	unsigned int i, stream_index;
	enum AVMediaType type;
	AVFrame *frame;
	
	/* read all packets */
    while (1) {
        if ((ret = av_read_frame(pFormatCtx, &packet)) < 0) {
		if (ret == AVERROR_EOF) {
#ifdef DEBUG
			av_log(NULL, AV_LOG_INFO, "End Of File\nᕕ(⌐■_■)ᕗ ♪♬\n");
#endif
			break;
		}
	    av_log(NULL, AV_LOG_ERROR, "av_read_frame: returned %d\n", ret);
            break;
	}
        stream_index = packet.stream_index;
	if (stream_index == 1)
		continue; /* "IF" for debugging purpose */
        type = codec_ctx[stream_index]->codec_type;
        av_log(NULL, AV_LOG_INFO, "Demuxer gave frame of stream_index %u\n",
                stream_index);
        if (filter_ctx[stream_index].filter_graph) {
            av_log(NULL, AV_LOG_INFO, "Going to reencode&filter the frame\n");
            av_packet_rescale_ts(&packet,
                                 pFormatCtx->streams[stream_index]->time_base,
                                 codec_ctx[stream_index]->time_base);
                                 
			/* Send packet to decoder */
			if ((ret = avcodec_send_packet(codec_ctx[stream_index], &packet)) != 0) {
                av_log(NULL, AV_LOG_ERROR, "Send packet failed with error: %d\n", ret);
				return ret;
			}

            frame = av_frame_alloc();
            if (!frame) {
				av_log(NULL, AV_LOG_ERROR, "av_frame_alloc: out of memory\n");
                ret = AVERROR(ENOMEM);
                return (ret);
            }

			/* Receive frame from decoder */
			if ((ret = avcodec_receive_frame(codec_ctx[stream_index], frame)) != 0) {
				av_frame_free(&frame);
				if (ret == AVERROR(EAGAIN)) {
					/* Output not available, send more packets */
					continue;
				}
                av_log(NULL, AV_LOG_ERROR, "Receive frame failed with error: %d\n", ret);
				return ret;
			}
				
            if (ret < 0) {
                av_frame_free(&frame);
                av_log(NULL, AV_LOG_ERROR, "Decoding failed\n");
                break;
            }

			frame->pts = av_frame_get_best_effort_timestamp(frame);
			ret = filter_write_frame(out_ctx,
			    frame, enc_codec_ctx[stream_index], stream_index);	
			av_frame_free(&frame);
			if (ret < 0)
				break;

        } else {
            /* remux this frame without reencoding */
            av_packet_rescale_ts(&packet,
                                 pFormatCtx->streams[stream_index]->time_base,
                                 out_ctx->streams[stream_index]->time_base);
            ret = av_interleaved_write_frame(out_ctx, &packet);
            if (ret < 0)
                break;
        }
        av_packet_unref(&packet);
    } //while

    /* flush filters and encoders */
    for (i = 0; i < pFormatCtx->nb_streams; i++) {
        /* flush filter */
        if (!filter_ctx[i].filter_graph)
            continue;
        ret = filter_write_frame(out_ctx, NULL, enc_codec_ctx[i], i);
        
        /* flush encoder */
        ret = flush_encoder(out_ctx, enc_codec_ctx[i], i);
        if (ret == AVERROR_EOF) {
            av_log(NULL, AV_LOG_ERROR, "Flushing encoder failed\n");
            goto end;
        } else {
		av_log(NULL, AV_LOG_INFO, "Flushed...\n");
	}
    }
    av_write_trailer(out_ctx);
    
#if 0
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
end: 
	return 0;
}
