//
// Created by fred.nicolson on 24/05/17.
//

#include <Log.h>
#include <frnetlib/HttpRequest.h>
#include "M3UStreamDecoder.h"
#define EXTM3U_HEADER "#EXTM3U"

bool M3UStreamDecoder::supports_mime(const std::string &mime)
{
    return (mime == "application/mpegurl" || mime == "application/x-mpegurl" || mime == "audio/mpegurl" || mime == "application/x-mpegurl");
}

bool M3UStreamDecoder::play_stream(fr::Socket &&sock, const fr::HttpResponse &resp)
{
    //Go through it line by line
    std::unordered_map<std::string, std::string> headers;
    std::vector<std::string> page = split_string(resp.get_body());
    for(int a = 0; a < page.size(); a++)
    {
        std::cout << "LINE " << a << ": " << page[a] << std::endl;
    }

    //Ensure that the M3U header is present
    if(page[0] != EXTM3U_HEADER)
    {
        frlog << Log::warn << "Can't play M3U stream. Stream has an invalid header: " << page[0] << Log::end;
        return false;
    }

    //Parse it
    for(size_t a = 1; a < page.size(); a++)
    {
        //Skip if empty line
        if(page[a].empty())
            continue;

        //Skip if comment
        if(page[a].size() >= 2 && page[a][0] == '#' && page[a][0] == '#')
            continue;

        //Parse and store headers
        if(page[a][0] == '#')
        {
            headers.emplace(parse_header(page[a]));
            std::cout << "Header: " << parse_header(page[a]).first << ", " << parse_header(page[a]).second << std::endl;
        }
        else
        {
            //If not a header, download next M3U file
            std::cout << "Non-header: " << page[a] << std::endl;
            fr::URL url(page[a]);
            fr::TcpSocket socket;
            fr::Socket::Status ret = socket.connect(url.get_host(), url.get_port());
            if(ret != fr::Socket::Success)
            {
                frlog << Log::warn << "Failed to connect, to download M3U file from " << page[a] << Log::end;
                return false;
            }

            //Send request for it
            fr::HttpRequest request;
            request.set_uri("/" + url.get_path() + "?" + url.get_query() + "#" + url.get_fragment());
            ret = socket.send(request);
            if(ret != fr::Socket::Success)
            {
                frlog << Log::warn << "Failed to query radio stream. Error code: " << ret << Log::end;
                return false;
            }

            //Receive response
            fr::HttpResponse response;
            ret = socket.receive(response);
            if(ret != fr::Socket::Success)
            {
                frlog << Log::warn << "Failed to query radio stream. Error code: " << ret << Log::end;
                return false;
            }

            M3UStreamDecoder decoder;
            decoder.play_stream(std::move(socket), response);
        }

    }
    return true;
}

std::pair<std::string, std::string> M3UStreamDecoder::parse_header(const std::string &header_line)
{
    auto pos = header_line.find(":");
    if(pos == std::string::npos)
        throw std::runtime_error("M3UStreamDecoder::parse_header() error: Header does not contain a colon. Header: " + header_line);
    return std::make_pair(header_line.substr(0, pos), header_line.substr(pos + 1, header_line.size() - pos - 1));
}
