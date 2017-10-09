//
// Created by misiek on 10.09.17.
//

#ifndef __AVFILTER_HH
#define __AVFILTER_HH
#ifdef __cplusplus
extern "C" {
#endif
#include <libavfilter/avfilter.h>
#ifdef __cplusplus
};
#endif


namespace ffmpeg {
    class AVFilterInOut {
    public:
        AVFilterInOut();
        ~AVFilterInOut();

        ::AVFilterInOut *filter() const { return _filter; }

        void set_filter(::AVFilterInOut *_filter);
        ::AVFilterInOut **ref() { return &_filter; }
        ::AVFilterInOut* operator->() { return _filter;}
        ::AVFilterInOut& operator*() { return  *_filter;}

    private:
        ::AVFilterInOut* _filter;

    };


    class AVFilter {
    public:


        explicit AVFilter(const char *name);

        ~AVFilter() = default;
        ::AVFilter* filter() const { return _filter; }


    private:
        AVFilter() : _filter(nullptr){}
        ::AVFilter* _filter;
    };

    class AVFilterGraph {
    public:
        AVFilterGraph();
        ~AVFilterGraph();
        ::AVFilterGraph** ref() { return  &_filter;}
        ::AVFilterGraph* operator->() { return _filter;}
        ::AVFilterGraph& operator*() { return *_filter;}
        ::AVFilterGraph* filter() { return _filter;}
        void config(void* log_ctx);
        AVFilterGraph& operator=(AVFilterGraph& other) = default; //TODO: Implement right way

    private:
        ::AVFilterGraph* _filter;
    };
}


#endif //__AVFILTER_HH
