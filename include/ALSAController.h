//
// Created by fred on 07/05/17.
//

#ifndef MEDIASERVER_ALSACONTROLLER_H
#define MEDIASERVER_ALSACONTROLLER_H

#include <memory>
extern "C" {
#include <alsa/asoundlib.h>
}

class ALSAController
{
public:
    static std::unique_ptr<ALSAController> &get();
    void set_volume(long val);
    long get_volume();
    ~ALSAController();
private:
    ALSAController();

    snd_mixer_t *alsa_mixer;
    snd_mixer_elem_t *mixer_element;
    long min_volume;
    long max_volume;
    float volume_scale;
};


#endif //MEDIASERVER_ALSACONTROLLER_H
