#include <iostream>
#include <InternetRadio.h>
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

//    InternetRadio radio;
//    radio.play_stream("http://a.files.bbci.co.uk/media/live/manifesto/audio/simulcast/hls/uk/sbr_high/ak/bbc_radio_one.m3u8");
//
//    return 0;

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
        else if(input == "exit")
        {
            return 0;
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