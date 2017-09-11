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
}


#endif //__AVFILTER_HH
