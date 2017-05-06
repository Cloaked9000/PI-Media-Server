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


class APIServer
{
public:
    APIServer();
    virtual ~APIServer();
    bool initialise();

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

    //Required stuff
    std::unique_ptr<std::thread> server_thread;
    std::atomic<bool> running;
    fr::TcpListener listener;
    std::unordered_map<std::string, std::function<fr::HttpResponse(fr::HttpRequest &)>> uri_handlers;

};


#endif //MEDIASERVER_APISERVER_H
