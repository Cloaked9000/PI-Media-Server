//
// Created by fred.nicolson on 23/05/17.
//

#include <frnetlib/TcpSocket.h>
#include <frnetlib/HttpRequest.h>
#include <frnetlib/URL.h>
#include <Log.h>
#include <frnetlib/HttpResponse.h>
#include <M3UStreamDecoder.h>
#include <DirectStreamDecoder.h>
#include "InternetRadio.h"

bool InternetRadio::play_stream(const std::string &stream_url)
{
    frlog << Log::info << "Loading internet radio stream: " << stream_url << Log::end;

    //Parse the URL
    fr::URL url(stream_url);
    if(url.get_scheme() != fr::URL::HTTP)
    {
        frlog << Log::warn << "Can't play stream " << stream_url << ". Scheme not supported" << Log::end;
        return false;
    }

    //Query the URL, find out what type of stream it is
    fr::Socket::Status ret;
    fr::TcpSocket socket;

    //Connect
    ret = socket.connect(url.get_host(), url.get_port());
    if(ret != fr::Socket::Success)
    {
        frlog << Log::warn << "Failed to connect to internet radio stream: " << stream_url << ". Error code: " << ret << Log::end;
        return false;
    }

    //Query
    fr::HttpRequest request;
    request.set_uri("/" + url.get_path() + "?" + url.get_query() + "#" + url.get_fragment());
    ret = socket.send(request);
    if(ret != fr::Socket::Success)
    {
        frlog << Log::warn << "Failed to query radio stream " << stream_url << ". Error code: " << ret << Log::end;
        return false;
    }

    //Receive response
    fr::HttpResponse response;
    ret = socket.receive(response);
    if(ret != fr::Socket::Success)
    {
        frlog << Log::warn << "Failed to query radio stream " << stream_url << ". Error code: " << ret << Log::end;
        return false;
    }

    //Verify that there's data
    if(response.get_body().empty())
    {
        frlog << Log::warn << "Radio stream " << stream_url << " has no body." << Log::end;
        return false;
    }

    //Get mime type
    const std::string &mime = response.header("content-type");
    if(mime.empty())
    {
        frlog << Log::warn << "Radio stream " << stream_url << " does not provide a mime type. Abandoning." << Log::end;
        return false;
    }

    //Check if there's a decoder available which supports the stream
    if(M3UStreamDecoder::supports_mime(mime))
    {
        stream_decoder = std::unique_ptr<M3UStreamDecoder>(new M3UStreamDecoder);
    }
    else if(DirectStreamDecoder::supports_mime(mime))
    {
        stream_decoder = std::unique_ptr<DirectStreamDecoder>(new DirectStreamDecoder);
    }
    else
    {
        frlog << Log::info << "Can't load radio stream. No available decoder for mime type: " << response.header("content-type") << Log::end;
        return false;
    }

    //Initialise decoder
    return stream_decoder->play_stream(std::move(socket), response);
}
