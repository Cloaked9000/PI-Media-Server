//
// Created by fred on 06/05/17.
//

#ifndef MEDIASERVER_APISERVER_H
#define MEDIASERVER_APISERVER_H


#include <thread>
#include <atomic>
#include <frnetlib/HttpRequest.h>
#include <frnetlib/HttpResponse.h>
#include <frnetlib/HttpSocket.h>
#include <frnetlib/TcpListener.h>
#include "MusicStorage.h"
#include "MusicPlayer.h"


class APIServer
{
public:
    APIServer();
    virtual ~APIServer();
    bool initialise(MusicStorage &music_storage, MusicPlayer &player);

    /*!
     * Closes the server and cleans up
     */
    void stop();

private:

    /*!
     * Starts the server API server, maintains connections.
     * Should be started in another thread as it wont stop
     * without being signaled.
     *
     */
    void server_loop();

    /*!
     * Constructs a WebRequest object containing a JSON
     * error message.
     *
     * @param type Why the error is occurring
     * @param reason A more detailed explanation as to why
     * @return The constructed WebResponse objects containing the error
     */
    fr::HttpResponse construct_error_response(fr::HttpRequest::RequestStatus type, const std::string &reason);

    fr::HttpResponse handler_list_albums(fr::HttpRequest &request);
    fr::HttpResponse handler_list_album_songs(fr::HttpRequest &request);
    fr::HttpResponse handler_play_song(fr::HttpRequest &request);
    fr::HttpResponse handler_resume_song(fr::HttpRequest &request);
    fr::HttpResponse handler_pause_song(fr::HttpRequest &request);
    fr::HttpResponse handler_get_playing(fr::HttpRequest &request);
    fr::HttpResponse handler_skip_next(fr::HttpRequest &request);
    fr::HttpResponse handler_skip_prior(fr::HttpRequest &request);

    //Required stuff
    std::unique_ptr<std::thread> server_thread;
    std::atomic<bool> running;
    fr::TcpListener listener;
    std::unordered_map<std::string, std::function<fr::HttpResponse(fr::HttpRequest &)>> uri_handlers;

    //Dependencies
    MusicStorage *music_storage;
    MusicPlayer *music_player;
};


#endif //MEDIASERVER_APISERVER_H
