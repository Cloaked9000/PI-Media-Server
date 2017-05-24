//
// Created by fred.nicolson on 23/05/17.
//

#ifndef MEDIASERVER_INTERNETRADIO_H
#define MEDIASERVER_INTERNETRADIO_H
#include <string>
#include <memory>
#include "StreamDecoderBase.h"

class InternetRadio
{
public:
    virtual ~InternetRadio()=default;

    /*!
     * Attempts to play an internet radio stream.
     * Playback occurs on another thread, and the
     * function will return once playback has started/
     * failed to start.
     *
     * @param stream_url The URL of the radio to play
     * @return True on success, false on failure.
     */
    bool play_stream(const std::string &stream_url);

private:
    std::unique_ptr<StreamDecoderBase> stream_decoder;
};


#endif //MEDIASERVER_INTERNETRADIO_H
