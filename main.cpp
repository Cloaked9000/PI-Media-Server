#include <iostream>
#include "include/MusicPlayer.h"

int main(int argc, char **argv)
{
    if(argc < 1)
        return 1;

    MusicPlayer player;
    player.load(argv[1]);
    player.play();

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
    }

    return 0;
}