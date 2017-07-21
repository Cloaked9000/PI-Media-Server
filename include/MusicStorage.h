//
// Created by fred on 06/05/17.
//

#ifndef MEDIASERVER_MUSICSTORAGE_H
#define MEDIASERVER_MUSICSTORAGE_H

#include <string>
#include <vector>

#define MUSIC_DIRECTORY "music/"

class MusicStorage
{
public:
    MusicStorage()= default;
    virtual ~MusicStorage()= default;

    bool initialise();
    std::vector<std::string> list_albums();
    std::vector<std::string> list_album_songs(const std::string &album_name);
private:
};


#endif //MEDIASERVER_MUSICSTORAGE_H
