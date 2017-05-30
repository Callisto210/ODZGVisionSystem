#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libswscale/swscale.h>


/*AVFormatContext Fields description at https://ffmpeg.org/doxygen/3.2/structAVFormatContext.html
 */

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

/* opening output file is something like:
 * - create output context
 * - populate context with streams you want to put into file
 * - of course set stream properties (codec, height, width, aspect)
 * - open file to write
 * - init muxer, and write file header
*/

static int create_output_context(AVFormatContext **pFormatCtx, const char *filename) {
	*pFormatCtx = NULL;
    avformat_alloc_output_context2(pFormatCtx, NULL, NULL, filename);
    if (!*pFormatCtx) {
        av_log(NULL, AV_LOG_ERROR, "Could not create output context\n");
        return AVERROR_UNKNOWN;
    }
    return 0;
}

static int put_stream_into_context(AVFormatContext *pFormatCtx, AVStream *in_stream) {
	AVCodecContext *dec_ctx, *enc_ctx;
    AVCodec *encoder, *decoder;
    int ret;	
	
	AVStream *out_stream = avformat_new_stream(pFormatCtx, NULL);
	if (!out_stream) {
		av_log(NULL, AV_LOG_ERROR, "Failed allocating output stream\n");
		return AVERROR_UNKNOWN;
	}
	decoder = avcodec_find_decoder(in_stream->codecpar->codec_id);
	dec_ctx = avcodec_alloc_context3(decoder);
	avcodec_parameters_to_context(dec_ctx, in_stream->codecpar);
	
	if (dec_ctx->codec_type == AVMEDIA_TYPE_VIDEO || dec_ctx->codec_type == AVMEDIA_TYPE_AUDIO) {
		/* in this example, we choose transcoding to same codec */
		encoder = avcodec_find_encoder(AV_CODEC_ID_VP8);
		enc_ctx = avcodec_alloc_context3(encoder);
		/*avcodec_parameters_to_context(enc_ctx, in_stream->codecpar);*/
		if (!encoder) {
			av_log(NULL, AV_LOG_FATAL, "Necessary encoder not found\n");
			return AVERROR_INVALIDDATA;
		}
		/* In this example, we transcode to same properties (picture size,
		 * sample rate etc.). These properties can be changed for output
		 * streams easily using filters */
		if (dec_ctx->codec_type == AVMEDIA_TYPE_VIDEO) {
			enc_ctx->height = dec_ctx->height;
			enc_ctx->width = dec_ctx->width;
			enc_ctx->sample_aspect_ratio = dec_ctx->sample_aspect_ratio;
			/* take first format from list of supported formats */
			if (encoder->pix_fmts)
				enc_ctx->pix_fmt = encoder->pix_fmts[0];
			else
				enc_ctx->pix_fmt = dec_ctx->pix_fmt;
			/* video time_base can be set to whatever is handy and supported by encoder */
			enc_ctx->time_base = dec_ctx->time_base;
		} else {
			enc_ctx->sample_rate = dec_ctx->sample_rate;
			enc_ctx->channel_layout = dec_ctx->channel_layout;
			enc_ctx->channels = av_get_channel_layout_nb_channels(enc_ctx->channel_layout);
			/* take first format from list of supported formats */
			enc_ctx->sample_fmt = encoder->sample_fmts[0];
			enc_ctx->time_base = (AVRational){1, enc_ctx->sample_rate};
		}
		/* Third parameter can be used to pass settings to encoder */
		ret = avcodec_open2(enc_ctx, encoder, NULL);
		if (ret < 0) {
			av_log(NULL, AV_LOG_ERROR, "Cannot open video encoder for stream\n");
			return ret;
		}
	} else if (dec_ctx->codec_type == AVMEDIA_TYPE_UNKNOWN) {
		av_log(NULL, AV_LOG_FATAL, "Elementary stream is of unknown type, cannot proceed\n");
		return AVERROR_INVALIDDATA;
	}
	if (pFormatCtx->oformat->flags & AVFMT_GLOBALHEADER)
		enc_ctx->flags |= AV_CODEC_FLAG_GLOBAL_HEADER;

	return 0;
}

/* Helpful when you simply want to copy stream 
 * Adds input stream into output file context
 */
static int remux_stream(AVFormatContext *pFormatCtx, AVStream *in_stream) {
	int ret;
	AVCodecParameters parameters;

	AVStream *out_stream = avformat_new_stream(pFormatCtx, NULL);
	if (!out_stream) {
		av_log(NULL, AV_LOG_ERROR, "Failed allocating output stream\n");
		return AVERROR_UNKNOWN;
	}
	ret = avcodec_parameters_from_context(&parameters, in_stream->codec);
	if(ret >= 0) {
		ret = avcodec_parameters_to_context(out_stream->codec, &parameters);
	}
	//ret = avcodec_copy_context(out_stream->codec, in_stream->codec);
	if (ret < 0) {
		av_log(NULL, AV_LOG_ERROR, "Copying stream context failed\n");
		return ret;
	}
	return 0;
}


int main(int argc, char *argv[]) {
	if (argc != 2)
		return 1;
		
	av_register_all();
	
	AVFormatContext *pFormatCtx;
	open_input_file(&pFormatCtx, argv[1]);
}
