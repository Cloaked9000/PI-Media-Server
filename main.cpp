#include "include/MusicPlayer.h"

int main(int argc, char **argv)
{
    if(argc < 1)
        return 1;

    MusicPlayer player;
    player.load(argv[1]);
    player.play();


    return 0;
}