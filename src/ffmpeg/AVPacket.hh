//
// Created by misiek on 11.09.17.
//

#ifndef __AVPACKET_HH
#define __AVPACKET_HH
#ifdef __cplusplus
extern "C" {
#endif
#include <libavcodec/avcodec.h>
#ifdef __cplusplus
};
#endif

namespace ffmpeg{
    class AVPacket{
    public:
        AVPacket();
        ~AVPacket();
        AVPacket& operator=(AVPacket const& other);
        ::AVPacket* av_packet() const;
        ::AVPacket* operator->() { return _av_packet;}
        ::AVPacket operator*() { return *_av_packet;}
    private:
        ::AVPacket *_av_packet;
    };
}


#endif //ODZG_AVPACKET_HH
