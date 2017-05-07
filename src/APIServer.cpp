//
// Created by fred on 06/05/17.
//

#include "../include/APIServer.h"
#include "../include/Log.h"
#include "../include/Filesystem.h"
#include "../include/MusicPlayer.h"
#include <nlohmann/json.hpp>
#define STATUS_SUCCESS "success"
#define STATUS_FAILED "failed"
#define DO_CHECK_AUTH(x) if(!check_auth(request)) return construct_error_response(fr::Http::Unauthorised, "API Key incorrect");
#define API_PORT "9092"

using json = nlohmann::json;

APIServer::APIServer()
: running(false)
{
    uri_handlers["/api/albums/list"] = std::bind(&APIServer::handler_list_albums, this, std::placeholders::_1);
    uri_handlers["/api/albums/list_songs"] = std::bind(&APIServer::handler_list_album_songs, this, std::placeholders::_1);
    uri_handlers["/api/songs/play"] = std::bind(&APIServer::handler_play_song, this, std::placeholders::_1);
    uri_handlers["/api/control/resume"] = std::bind(&APIServer::handler_resume_song, this, std::placeholders::_1);
    uri_handlers["/api/control/pause"] = std::bind(&APIServer::handler_pause_song, this, std::placeholders::_1);
    uri_handlers["/api/info/get_playing"] = std::bind(&APIServer::handler_get_playing, this, std::placeholders::_1);
}

APIServer::~APIServer()
{
    stop();
}

void APIServer::stop()
{
    if(!running)
        return;

    //Stop API thread
    running = false;
    listener.shutdown();
    if(server_thread)
    {
        if(server_thread->joinable())
        {
            server_thread->join();
        }
    }
    server_thread = nullptr;
}


bool APIServer::initialise(MusicStorage &music_storage_, MusicPlayer &player_)
{
    //Bind to port and start listener
    if(listener.listen(API_PORT) != fr::Socket::Success)
    {
        frlog << Log::crit << "Failed to bind API to port: " << API_PORT << Log::end;
        return false;
    }

    //Link dependencies
    music_storage = &music_storage_;
    music_player = &player_;

    //Start client thread
    running = true;
    server_thread = std::unique_ptr<std::thread>(new std::thread(std::bind(&APIServer::server_loop, this)));

    return true;
}

void APIServer::server_loop()
{
    frlog << Log::info << "API client is listening for connections on port " << API_PORT << "." << Log::end;
    while(running)
    {
        //Wait for a connection (it's blocking, don't you worry there)
        fr::HttpSocket<fr::TcpSocket> client;
        if(!listener.accept(client))
            continue;

        //Accept the clients HTTP request
        fr::HttpRequest request;
        if(!client.receive(request))
            continue;

        //Pass it off to the correct handler. First making sure it actually exists.
        auto handler = uri_handlers.find(request.get_uri());
        if(handler == uri_handlers.end())
        {
            //Check if it's a html page
            std::string page;
            if(request.get_uri().size() <= 2)
                request.set_uri("/index.html");

            if(request.get_uri().find("..") == std::string::npos && Filesystem::read_file("html" + request.get_uri(), page))
            {
                frlog << Log::info << client.get_remote_address() << " asked for page: " << request.get_uri() << Log::end;
                fr::HttpResponse response;
                response.set_status(fr::Http::Ok);
                response.set_body(page);
                response.header("content-type") = response.get_mimetype(request.get_uri());
                client.send(response);
            }
            else
            {
                //Else construct a 404 response and send it
                frlog << Log::info << client.get_remote_address() << " tried to access an invalid page: " << request.get_uri() << Log::end;
                fr::HttpResponse response;
                response.set_status(fr::Http::NotFound);
                response.set_body("<h1>404 - Not Found</h1>");
                client.send(response);
            }
        }
        else
        {
            frlog << Log::info << client.get_remote_address() << " sent an API request: " << handler->first << Log::end;
            try
            {
                //Send the client the result of the handler function
                client.send(handler->second(request));
            }
            catch(const std::exception &e)
            {
                frlog << Log::warn << "An error occurred whilst processing an API request for" << request.get_uri()
                      << ": " << e.what() << Log::end;
            }
        }


    }
}

fr::HttpResponse APIServer::construct_error_response(fr::HttpRequest::RequestStatus type, const std::string &reason)
{
    json details;
    fr::HttpResponse response;
    response.header("Content-Type") = "application/json";

    details["status"] = STATUS_FAILED;
    details["reason"] = reason;

    response.set_status(type);
    response.set_body(details.dump());

    return response;
}

fr::HttpResponse APIServer::handler_list_albums(fr::HttpRequest &request)
{
    json details;
    details["albums"] = music_storage->list_albums();
    details["status"] = STATUS_SUCCESS;

    fr::HttpResponse response;
    response.header("Content-Type") = "application/json";
    response.set_body(details.dump());
    return response;
}

fr::HttpResponse APIServer::handler_list_album_songs(fr::HttpRequest &request)
{
    if(!request.header_exists("album_name"))
    {
        return construct_error_response(fr::HttpRequest::RequestStatus::BadRequest, "Missing HEADER parameters");
    }

    json details;
    details["album_songs"] = music_storage->list_album_songs(request.header("album_name"));
    details["status"] = STATUS_SUCCESS;

    fr::HttpResponse response;
    response.header("Content-Type") = "application/json";
    response.set_body(details.dump());
    return response;
}

fr::HttpResponse APIServer::handler_play_song(fr::HttpRequest &request)
{
    if(!request.header_exists("album_name") || !request.header_exists("song_name"))
    {
        return construct_error_response(fr::HttpRequest::RequestStatus::BadRequest, "Missing HEADER parameters");
    }

    if(!music_player->load(MUSIC_DIRECTORY + request.header("album_name") + "/" + request.header("song_name")))
    {
        return construct_error_response(fr::HttpRequest::RequestStatus::BadRequest, "No such song");
    }
    music_player->play();

    json details;
    details["status"] = STATUS_SUCCESS;

    fr::HttpResponse response;
    response.header("Content-Type") = "application/json";
    response.set_body(details.dump());
    return response;
}

fr::HttpResponse APIServer::handler_resume_song(fr::HttpRequest &request)
{
    music_player->play();

    json details;
    details["status"] = STATUS_SUCCESS;

    fr::HttpResponse response;
    response.header("Content-Type") = "application/json";
    response.set_body(details.dump());
    return response;
}

fr::HttpResponse APIServer::handler_pause_song(fr::HttpRequest &request)
{
    music_player->pause();

    json details;
    details["status"] = STATUS_SUCCESS;

    fr::HttpResponse response;
    response.header("Content-Type") = "application/json";
    response.set_body(details.dump());
    return response;
}

fr::HttpResponse APIServer::handler_get_playing(fr::HttpRequest &request)
{
    json details;
    details["status"] = STATUS_SUCCESS;
    details["track_name"] = music_player->get_filepath();
    details["track_state"] = music_player->state_to_string(music_player->get_state());
    details["track_duration"] = std::to_string(music_player->get_duration().count());
    details["track_offset"] = std::to_string(music_player->get_offset().count());
    details["track_volume"] = music_player->get_volume();

    fr::HttpResponse response;
    response.header("Content-Type") = "application/json";
    response.set_body(details.dump());
    return response;
}
