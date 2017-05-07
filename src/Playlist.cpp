//
// Created by fred on 07/05/17.
//

#include <algorithm>
#include "../include/Playlist.h"
#include "../include/Log.h"

Playlist::Playlist()
: current_track(queue.end())
{

}

void Playlist::shuffle()
{
    std::lock_guard<std::mutex> guard(lock);
    std::random_shuffle(queue.begin(), queue.end()); //Yay STL!
}

void Playlist::enqueue(const PlaylistQueue &tracks)
{
    std::lock_guard<std::mutex> guard(lock);
    queue = tracks;
    if(queue.empty())
        current_track = queue.end();
    else
        current_track = queue.begin();
}

std::pair<Playlist::Album, Playlist::Track> Playlist::skip_next()
{
    std::lock_guard<std::mutex> guard(lock);
    if(queue.empty())
        return {"NULL", "NULL"};

    if(current_track == queue.end() || ++current_track == queue.end())
        current_track = queue.begin();

    return *current_track;
}

std::pair<Playlist::Album, Playlist::Track> Playlist::get_playing()
{
    std::lock_guard<std::mutex> guard(lock);
    if(current_track == queue.end())
        return std::make_pair("No album is selected", "No song is selected");
    return *current_track;
}

void Playlist::clear()
{
    std::lock_guard<std::mutex> guard(lock);
    queue.clear();
    current_track = queue.end();
}

void Playlist::enqueue(const std::pair<Playlist::Album, Playlist::Track> &track)
{
    std::lock_guard<std::mutex> guard(lock);
    frlog << Log::info << "Enqueued: " << track.first << ": " << track.second << Log::end;
    queue.emplace_back(track);
}

void Playlist::set_playing(const std::pair<Playlist::Album, Playlist::Track> &track)
{
    std::lock_guard<std::mutex> guard(lock);
    auto iter = std::find(queue.begin(), queue.end(), track);
    if(iter == queue.end())
    {
        frlog << Log::warn << "Failed to set " << track.first << ": " << track.second << " as playing in queue. Not found" << Log::end;
        return;
    }
    else
    {
        frlog << Log::info << "Set " << track.first << ": " << track.second << " as currently queued" << Log::end;
        current_track = iter;
    }
}

bool Playlist::empty()
{
    std::lock_guard<std::mutex> guard(lock);
    return queue.empty();
}

std::pair<Playlist::Album, Playlist::Track> Playlist::skip_prior()
{
    std::lock_guard<std::mutex> guard(lock);
    if(queue.empty())
        return {"NULL", "NULL"};

    if(current_track == queue.begin())
        current_track = --queue.end();
    else
        current_track--;

    return *current_track;
}
