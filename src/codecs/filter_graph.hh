/*
 * ODZVisionSystem
 * Filter Graph code
 * © I@IET AGH 2017
 * Authors: Patryk Duda,
 *          Jan Górski,
 *          Małgorzata Olszewska
 *          Michał Zagórski
 *
 */

#ifndef __FILTER_GRAPH_HH
#define __FILTER_GRAPH_HH
#define MAX_FRAMES_FORM_FILTER 100

#ifdef __cplusplus
extern "C" {
#endif

#include <libavcodec/avcodec.h>
#include <libavutil/pixdesc.h>
#include <libavfilter/avfiltergraph.h>
#include <libavfilter/buffersink.h>
#include <libavfilter/buffersrc.h>
#include <libavutil/opt.h>

#ifdef __cplusplus
};
#endif
//C++ libraries
#include <spdlog/spdlog.h>

class FilteringContext {
public:
    /**
     * Creating basic structure of FilteringContext;
     */
    FilteringContext();

    /**
     * @brief Initializing filter graph for video stream inside Filtering Context
     * @param pixelFormat
     * @param
     * @param AVCodecContext codec context
     * @return 0 or error code otherwise
     */
    int init_filter_graph_video(AVPixelFormat pixel_format, const char *filter_spec, AVCodecContext *codec_ctx);

    int init_filter_graph_audio(AVCodecContext *, AVCodecContext *, const char *);

    int filter_encode_write_frame(AVFrame *, AVFrame **, unsigned int);

    ~FilteringContext() = default;

    AVFilterContext *buffersink_ctx;
    AVFilterContext *buffersrc_ctx;
    AVFilterGraph *filter_graph;
private:
    std::shared_ptr<spdlog::logger> _log;
};

#endif // __FILTER_GRAPH_HH