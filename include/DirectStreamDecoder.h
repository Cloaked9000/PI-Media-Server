//
// Created by fred.nicolson on 24/05/17.
//

#ifndef MEDIASERVER_DIRECTSTREAMDECODER_H
#define MEDIASERVER_DIRECTSTREAMDECODER_H


#include "StreamDecoderBase.h"

class DirectStreamDecoder : public StreamDecoderBase
{
public:
    /*!
     * Should be used to check if the decoder supports a given stream
     *
     * @param mime The stream mime type
     * @return True if it's supported. False otherwise.
     */
    static bool supports_mime(const std::string &mime);

    /*!
     * Starts the playback of a connected stream
     *
     * @param socket The socket connected to the stream
     * @param response The HTTP query received during connection
     * @return True on success, false on failure
     */
    virtual bool play_stream(fr::Socket &&socket, const fr::HttpResponse &response) override;
};


#endif //MEDIASERVER_DIRECTSTREAMDECODER_H
