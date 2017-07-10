//
// Created by fred on 07/05/17.
//

#include <string.h>
#include "../include/ALSAController.h"
#include "../include/Log.h"

ALSAController::ALSAController()
{
    frlog << Log::info << "Initialising ALSA" << Log::end;

    //Initialise volume control
    const char *mixer_element_name;
    snd_mixer_open(&alsa_mixer, 0);
    snd_config_update_free_global();
    snd_mixer_attach(alsa_mixer, "default");
    snd_mixer_selem_register(alsa_mixer, nullptr, nullptr);
    snd_mixer_load(alsa_mixer);

    //Find the mixer element we want
    mixer_element = snd_mixer_first_elem(alsa_mixer);
    while(mixer_element)
    {
        mixer_element_name = snd_mixer_selem_get_name(mixer_element);
        frlog << Log::info << "Mixer name: " << mixer_element_name << Log::end;
        if(strcasecmp(mixer_element_name, "Master") == 0 || strcasecmp(mixer_element_name, "Speaker") == 0)
        {
            snd_mixer_selem_get_playback_volume_range(mixer_element, &min_volume, &max_volume);
            max_volume = max_volume == 0 ? 100 : max_volume;
            volume_scale = (float)max_volume / 100;
            frlog << Log::info << "Max volume: " << max_volume << ". Volume scale: " << volume_scale << Log::end;
            return;
        }
        mixer_element = snd_mixer_elem_next(mixer_element);
    }

    throw std::runtime_error("Failed to initialise ALSA controller");
}

ALSAController::~ALSAController()
{
    snd_mixer_free(alsa_mixer);
}

std::unique_ptr<ALSAController> &ALSAController::get()
{
    static std::unique_ptr<ALSAController> instance(new ALSAController());
    return instance;
}

void ALSAController::set_volume(long val)
{
    frlog << Log::info << "Setting ALSA volume to: " << val * volume_scale << Log::end;
    snd_mixer_selem_set_playback_volume_all(mixer_element, val * volume_scale);
}

float ALSAController::get_volume()
{
    long vol;
    snd_mixer_selem_get_playback_volume(mixer_element,SND_MIXER_SCHN_FRONT_RIGHT, &vol);
    return vol / volume_scale;
}
