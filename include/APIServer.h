//
// Created by fred on 06/05/17.
//

#ifndef MEDIASERVER_APISERVER_H
#define MEDIASERVER_APISERVER_H


#include <thread>
#include <atomic>
#include <frnetlib/HttpRequest.h>
#include <frnetlib/HttpResponse.h>
#include <frnetlib/TcpListener.h>
#include "MusicStorage.h"
#include "MusicPlayer.h"
#include "ALSAController.h"


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

    typedef std::function<fr::HttpResponse(fr::HttpRequest &, const std::vector<std::string>&)> UriCallback;
    fr::HttpResponse handler_list_albums(fr::HttpRequest &request, const std::vector<std::string> &args);
    fr::HttpResponse handler_list_album_songs(fr::HttpRequest &request, const std::vector<std::string> &args);
    fr::HttpResponse handler_play_song(fr::HttpRequest &request, const std::vector<std::string> &args);
    fr::HttpResponse handler_resume_song(fr::HttpRequest &request, const std::vector<std::string> &args);
    fr::HttpResponse handler_pause_song(fr::HttpRequest &request, const std::vector<std::string> &args);
    fr::HttpResponse handler_get_playing(fr::HttpRequest &request, const std::vector<std::string> &args);
    fr::HttpResponse handler_skip_next(fr::HttpRequest &request, const std::vector<std::string> &args);
    fr::HttpResponse handler_skip_prior(fr::HttpRequest &request, const std::vector<std::string> &args);
    fr::HttpResponse handler_get_album_art(fr::HttpRequest &request, const std::vector<std::string> &args);
    fr::HttpResponse handler_set_volume(fr::HttpRequest &request, const std::vector<std::string> &args);

    //Required stuff
    std::unique_ptr<std::thread> server_thread;
    std::atomic<bool> running;
    fr::TcpListener listener;
    void register_uri_handler(std::string uri, UriCallback handler);
    std::unordered_map<std::string, UriCallback> uri_handlers;
    std::unordered_map<std::string, UriCallback>::iterator find_handler(const std::string &uri);
    std::vector<std::string> parse_uri_arguments(const std::string &uri_handler, const std::string &uri);

    //Dependencies
    MusicStorage *music_storage;
    MusicPlayer *music_player;
};


#endif //MEDIASERVER_APISERVER_H
