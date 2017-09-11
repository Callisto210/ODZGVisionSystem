//
// Created by misiek on 11.09.17.
//

#ifndef __AVCODEC_HH
#define __AVCODEC_HH
#ifdef __cplusplus
extern "C" {
#endif
#include <libavcodec/avcodec.h>
#ifdef __cplusplus
};
#endif
namespace ffmpeg {

    class AVCodec {
    public:
        AVCodec();

        virtual ~AVCodec() = 0;

    protected:
        ::AVCodec* av_codec{nullptr};
    };


}
#endif //ODZG_AVCODEC_HH
