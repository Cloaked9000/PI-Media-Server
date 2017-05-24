//
// Created by fred.nicolson on 24/05/17.
//

#include "DirectStreamDecoder.h"

bool DirectStreamDecoder::supports_mime(const std::string &mime)
{
    return mime == "audio/mpeg" || mime == "audio/mpeg3" || mime == "audio/x-mpeg-3" || mime == "audio/mp4" || mime == "audio/ogg" || mime == "audio/vorbis" || mime == "audio/wav" || mime == "audio/vnd.wav";
}

bool DirectStreamDecoder::play_stream(fr::Socket &&socket, const fr::HttpResponse &response)
{
    throw std::logic_error("DirectStreamDecoder::play_stream() error: Not implemented");
}
