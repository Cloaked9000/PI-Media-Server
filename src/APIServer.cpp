//
// Created by fred on 06/05/17.
//

#include "../include/APIServer.h"
#include "../include/Log.h"
#include "../include/Filesystem.h"
#include "../include/MusicPlayer.h"
#include "../include/ALSAController.h"
#include <nlohmann/json.hpp>
#define STATUS_SUCCESS "success"
#define STATUS_FAILED "failed"
#define DO_CHECK_AUTH(x) if(!check_auth(request)) return construct_error_response(fr::Http::Unauthorised, "API Key incorrect");
#define API_PORT "9092"

using json = nlohmann::json;

APIServer::APIServer()
: running(false)
{
    register_uri_handler("/api/albums/list", std::bind(&APIServer::handler_list_albums, this, std::placeholders::_1, std::placeholders::_2));
    register_uri_handler("/api/albums/list_songs", std::bind(&APIServer::handler_list_album_songs, this, std::placeholders::_1, std::placeholders::_2));
    register_uri_handler("/api/songs/play", std::bind(&APIServer::handler_play_song, this, std::placeholders::_1, std::placeholders::_2));
    register_uri_handler("/api/control/resume", std::bind(&APIServer::handler_resume_song, this, std::placeholders::_1, std::placeholders::_2));
    register_uri_handler("/api/control/pause", std::bind(&APIServer::handler_pause_song, this, std::placeholders::_1, std::placeholders::_2));
    register_uri_handler("/api/info/get_playing", std::bind(&APIServer::handler_get_playing, this, std::placeholders::_1, std::placeholders::_2));
    register_uri_handler("/api/control/skip_next", std::bind(&APIServer::handler_skip_next, this, std::placeholders::_1, std::placeholders::_2));
    register_uri_handler("/api/control/skip_prior", std::bind(&APIServer::handler_skip_prior, this, std::placeholders::_1, std::placeholders::_2));
    register_uri_handler("/cover_art/?", std::bind(&APIServer::handler_get_album_art, this, std::placeholders::_1, std::placeholders::_2));
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
        auto handler = find_handler(request.get_uri());
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
                client.send(handler->second(request, parse_uri_arguments(handler->first, request.get_uri())));
            }
            catch(const std::exception &e)
            {
                frlog << Log::warn << "An error occurred whilst processing an API request for" << request.get_uri()
                      << ": " << e.what() << Log::end;
            }
        }
        client.close_socket();
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

fr::HttpResponse APIServer::handler_list_albums(fr::HttpRequest &request, const std::vector<std::string> &args)
{
    json details;
    details["albums"] = music_storage->list_albums();
    details["status"] = STATUS_SUCCESS;

    fr::HttpResponse response;
    response.header("Content-Type") = "application/json";
    response.set_body(details.dump());
    return response;
}

fr::HttpResponse APIServer::handler_list_album_songs(fr::HttpRequest &request, const std::vector<std::string> &args)
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

fr::HttpResponse APIServer::handler_play_song(fr::HttpRequest &request, const std::vector<std::string> &args)
{
    if(!request.header_exists("album_name") || !request.header_exists("song_name"))
    {
        return construct_error_response(fr::HttpRequest::RequestStatus::BadRequest, "Missing HEADER parameters");
    }

    //Add album to player's playlist
    music_player->unload();
    music_player->playlist.clear();
    std::vector<std::string> tracks = music_storage->list_album_songs(request.header("album_name"));
    for(const auto &c : tracks)
        music_player->playlist.enqueue(std::make_pair(request.header("album_name"), c));
    music_player->playlist.set_playing(std::make_pair(request.header("album_name"), request.header("song_name")));
    music_player->play();

    while(music_player->get_duration().count() == 1);

    json details;
    details["status"] = STATUS_SUCCESS;

    fr::HttpResponse response;
    response.header("Content-Type") = "application/json";
    response.set_body(details.dump());
    return response;
}

fr::HttpResponse APIServer::handler_resume_song(fr::HttpRequest &request, const std::vector<std::string> &args)
{
    music_player->play();

    json details;
    details["status"] = STATUS_SUCCESS;

    fr::HttpResponse response;
    response.header("Content-Type") = "application/json";
    response.set_body(details.dump());
    return response;
}

fr::HttpResponse APIServer::handler_pause_song(fr::HttpRequest &request, const std::vector<std::string> &args)
{
    music_player->pause();

    json details;
    details["status"] = STATUS_SUCCESS;

    fr::HttpResponse response;
    response.header("Content-Type") = "application/json";
    response.set_body(details.dump());
    return response;
}

fr::HttpResponse APIServer::handler_get_playing(fr::HttpRequest &request, const std::vector<std::string> &args)
{
    json details;
    details["status"] = STATUS_SUCCESS;
    details["album_name"] = music_player->playlist.get_playing().first;
    details["track_name"] = music_player->playlist.get_playing().second;
    details["track_state"] = music_player->state_to_string(music_player->get_state());
    details["track_duration"] = std::to_string(music_player->get_duration().count());
    details["track_offset"] = std::to_string(music_player->get_offset().count());
    details["track_volume"] = ALSAController::get()->get_volume();

    fr::HttpResponse response;
    response.header("Content-Type") = "application/json";
    response.set_body(details.dump());
    return response;
}

fr::HttpResponse APIServer::handler_skip_next(fr::HttpRequest &request, const std::vector<std::string> &args)
{
    music_player->playlist.skip_next();
    music_player->unload();
    music_player->play();

    while(music_player->get_duration().count() == 1);

    json details;
    details["status"] = STATUS_SUCCESS;

    fr::HttpResponse response;
    response.header("Content-Type") = "application/json";
    response.set_body(details.dump());
    return response;
}

fr::HttpResponse APIServer::handler_skip_prior(fr::HttpRequest &request, const std::vector<std::string> &args)
{
    music_player->playlist.skip_prior();
    music_player->unload();
    music_player->play();

    while(music_player->get_duration().count() == 1);

    json details;
    details["status"] = STATUS_SUCCESS;

    fr::HttpResponse response;
    response.header("Content-Type") = "application/json";
    response.set_body(details.dump());
    return response;
}

fr::HttpResponse APIServer::handler_get_album_art(fr::HttpRequest &request, const std::vector<std::string> &args)
{
    //Check we've got all of the arguments
    if(args.size() < 2)
        return construct_error_response(fr::Http::BadRequest, "URI should contain both album name and song name.");

    fr::HttpResponse response;
    response.header("Content-Type") = response.get_mimetype(".jpg");
    response.set_body(music_player->get_album_cover());
    return response;
}

std::unordered_map<std::string, APIServer::UriCallback>::iterator
APIServer::find_handler(const std::string &uri)
{
    auto iter = std::find_if(uri_handlers.begin(), uri_handlers.end(), [&](const auto &elem)
    {
        const std::string &str = elem.first;

        //Handler length couldn't logically be longer than the URI
        if(str.size() > uri.size())
            return false;

        //Logically, they can't have different lengths, and one have a '/' at the end of the URI
        if(uri.size() != str.size() && uri[str.size()] != '/')
            return false;

        //Compare up to the end of URI
        return uri.compare(0, str.size(), str.c_str()) == 0;
    });
    return iter;
}

void APIServer::register_uri_handler(std::string uri, UriCallback handler)
{
    //Safety check
    if(uri.empty())
    {
        frlog << Log::crit << "Attempt to register empty URI handler!" << Log::end;
        return;
    }

    //Remove any /?/?
    auto pos = uri.find("?");
    if(pos != std::string::npos)
    {
        uri.erase(pos, uri.size() - pos);
    }

    //Remove trailing slashes if any
    if(uri.back() == '/')
        uri.erase(uri.size() - 1, 1);

    //Store
    frlog << Log::info << "Registering API handler: " << uri << Log::end;
    uri_handlers[uri] = handler;
}

std::vector<std::string> APIServer::parse_uri_arguments(const std::string &uri_handler, const std::string &uri)
{
    std::vector<std::string> args;
    std::string buffer;
    for(size_t a = uri_handler.size() + 1; a < uri.size(); a++)
    {
        if(uri[a] == '/' && !buffer.empty())
        {
            args.emplace_back(std::move(buffer));
            buffer.clear();
        }
        else
        {
            buffer += uri[a];
        }
    }

    if(!buffer.empty())
    {
        args.emplace_back(std::move(buffer));
        buffer.clear();
    }

    return args;
}

