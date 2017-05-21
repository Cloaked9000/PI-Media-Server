//
// Created by fred on 06/05/17.
//

#include <stdexcept>
#include <algorithm>
#include "../include/MusicStorage.h"
#include "../include/Filesystem.h"
#include "../include/Log.h"

MusicStorage::MusicStorage()
{

}

MusicStorage::~MusicStorage()
{

}

bool MusicStorage::initialise()
{
    return true;
}

std::vector<std::string> MusicStorage::list_albums()
{
    std::vector<std::string> files;
    if(!Filesystem::list_files(MUSIC_DIRECTORY, files))
    {
        frlog << Log::crit << "Failed to enumerate music directory: " << MUSIC_DIRECTORY << Log::end;
        throw std::runtime_error("Failed to enumerate music directory: " + std::string(MUSIC_DIRECTORY));
    }
    std::sort(files.begin(), files.end());
    return files;
}

std::vector<std::string> MusicStorage::list_album_songs(const std::string &album_name)
{
    std::vector<std::string> files;
    if(!Filesystem::list_files(MUSIC_DIRECTORY + album_name, files))
    {
        frlog << Log::crit << "Failed to enumerate music directory: " << MUSIC_DIRECTORY + album_name << Log::end;
        throw std::runtime_error("Failed to enumerate music directory: " + std::string(MUSIC_DIRECTORY + album_name));
    }
    std::sort(files.begin(), files.end());
    return files;
}
