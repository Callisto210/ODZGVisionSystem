//
// Created by misiek on 10.09.17.
//
#include <exception>
#include <stdexcept>
#include "AVFilter.hh"

ffmpeg::AVFilterInOut::AVFilterInOut() {
    _filter = nullptr;
    this->_filter = ::avfilter_inout_alloc();
    if(_filter == nullptr) {
        throw std::runtime_error("AVFilterInOut constructor");
    }
}
ffmpeg::AVFilterInOut::~AVFilterInOut() {
    ::avfilter_inout_free(&_filter);
    _filter = nullptr;
}


void ffmpeg::AVFilterInOut::set_filter(::AVFilterInOut *_filter) {
    this->_filter = _filter;
}

ffmpeg::AVFilter::AVFilter(const char* name) {
    this->_filter = avfilter_get_by_name(name);
}

ffmpeg::AVFilterGraph::AVFilterGraph() {
    this->_filter = avfilter_graph_alloc();
    if(_filter == nullptr) {
        throw std::runtime_error("AVFilterGraph()");
    }
}

ffmpeg::AVFilterGraph::~AVFilterGraph() {
    avfilter_graph_free(&_filter);
    _filter = nullptr;
}

void ffmpeg::AVFilterGraph::config(void *log_ctx) {
    int ret = avfilter_graph_config(_filter, log_ctx);
    if (ret < 0) throw std::runtime_error("AVFilterGraph config error");
}
