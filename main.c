#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libswscale/swscale.h>


/*AVFormatContext Fields description at https://ffmpeg.org/doxygen/2.7/structAVFormatContext.html
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
				return ret;
			}
		}
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
