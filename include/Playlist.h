//
// Created by fred on 07/05/17.
//

#ifndef MEDIASERVER_PLAYLIST_H
#define MEDIASERVER_PLAYLIST_H

#include <string>
#include <vector>
#include <mutex>

class Playlist
{
public:
    typedef std::string Album;
    typedef std::string Track;
    typedef std::vector<std::pair<Album, Track>> PlaylistQueue;
    typedef PlaylistQueue::iterator PlaylistEntry;

    Playlist();

    void shuffle();
    void enqueue(const PlaylistQueue &tracks);
    void clear();
    void enqueue(const std::pair<Album, Track> &track);
    std::pair<Album, Track> skip_next();
    std::pair<Album, Track> get_playing();
    void set_playing(const std::pair<Album, Track> &track);
    bool empty();
    std::pair<Playlist::Album, Playlist::Track> skip_prior();

private:
    PlaylistQueue queue;
    PlaylistEntry current_track;
    std::mutex lock;
};


#endif //MEDIASERVER_PLAYLIST_H
