//
// Created by fred.nicolson on 23/05/17.
//

#ifndef MEDIASERVER_STREAMDECODERBASE_H
#define MEDIASERVER_STREAMDECODERBASE_H
#include <string>
#include <frnetlib/URL.h>
#include <frnetlib/HttpResponse.h>

class StreamDecoderBase
{
public:
    virtual ~StreamDecoderBase() = default;

    /*!
     * Starts the playback of a connected stream
     *
     * @param socket The socket connected to the stream
     * @param response The HTTP query received during connection
     * @return True on success, false on failure
     */
    virtual bool play_stream(fr::Socket &&socket, const fr::HttpResponse &response) = 0;

protected:
    std::vector<std::string> split_string(const std::string &input)
    {
        std::vector<std::string> ret;
        size_t index = 0;
        while(true)
        {
            auto pos = input.find("\n", index);
            if(pos == std::string::npos)
            {
                ret.emplace_back(input.substr(index, input.size() - index));
                break;
            }

            std::string line = input.substr(index, pos - index);
            if(!line.empty())
                ret.emplace_back(std::move(line));
            index = pos + 1;
        }
        return ret;
    }

};


#endif //MEDIASERVER_STREAMDECODERBASE_H
