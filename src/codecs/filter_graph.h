/* Filter graph */

#include <libavfilter/avfilter.h>
#include <libavcodec/avcodec.h>

#define MAX_FRAMES_FORM_FILTER 100

typedef struct FilteringContext {
    AVFilterContext *buffersink_ctx;
    AVFilterContext *buffersrc_ctx;
    AVFilterGraph *filter_graph;
} FilteringContext;

int init_filter_graph_video(FilteringContext *, enum AVPixelFormat,
    const char *, AVCodecContext *);
int init_filter_graph_audio(FilteringContext *, AVCodecContext *,
    AVCodecContext *, const char *);

int filter_encode_write_frame(FilteringContext *, AVFrame *,
    AVFrame **, unsigned int);
