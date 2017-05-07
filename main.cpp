#include <iostream>
#include "include/Log.h"
#include "include/MusicPlayer.h"
#include "include/Filesystem.h"
#include "include/APIServer.h"
#include "include/ALSAController.h"

frlog_define();

int main(int argc, char **argv)
{
    //Initialise logging
    if(!Filesystem::does_filepath_exist("logs"))
    {
        if(!Filesystem::create_directory("logs"))
        {
            std::cout << "Failed to create 'logs' directory. Exiting." << std::endl;
            return 1;
        }
    }

    if(!Log::init("logs/" + std::to_string(std::time(NULL))))
        return 1;


    //Declare components
    APIServer api;
    MusicStorage music_storage;
    MusicPlayer player;

    //Initialise
    if(!music_storage.initialise() || !api.initialise(music_storage, player))
    {
        frlog << Log::crit << "Failed to initialise dependencies, exiting!" << Log::end;
        return 1;
    }

    if(argc < 1)
        return 1;

    std::string input;
    while(true)
    {
        std::cin >> input;
        if(input == "pause")
        {
            player.pause();
        }
        else if(input == "resume")
        {
            player.play();
        }
        else if(player.get_state() == MusicPlayer::State::Stopped)
        {
            break;
        }
        else
        {
            ALSAController::get()->set_volume(std::stoi(input));
        }
    }


    return 0;
}