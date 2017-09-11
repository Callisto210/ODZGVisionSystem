//
// Created by misiek on 11.09.17.
//

#include <stdexcept>
#include "AVPacket.hh"

ffmpeg::AVPacket::AVPacket() {
    this->_av_packet = av_packet_alloc();
    if(_av_packet == nullptr) {
        throw std::runtime_error("AVPacket");
    }
}

ffmpeg::AVPacket::~AVPacket() {
    av_packet_free(&_av_packet);
    if(_av_packet != nullptr) {
        av_log(nullptr, AV_LOG_ERROR, "Cannot set output sample rate\n");
    }
}

ffmpeg::AVPacket &ffmpeg::AVPacket::operator=(ffmpeg::AVPacket const &other) {
    av_copy_packet(this->_av_packet, other.av_packet());
}

::AVPacket *ffmpeg::AVPacket::av_packet() const {
    return this->_av_packet;
}
