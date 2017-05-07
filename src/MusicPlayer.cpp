//
// Created by fred.nicolson on 05/05/17.
//

#include <cstring>
#include <iostream>
#include "../include/MusicPlayer.h"
#include "../include/Log.h"

#define MAX_AUDIO_FRAME_SIZE 192000 //1 second of 48khz 32bit audio

MusicPlayer::MusicPlayer()
: play_state(State::Stopped), av_container(nullptr), av_codec(nullptr), av_codec_context(nullptr), ao_output(nullptr)
{
    //initialize AO lib
    ao_initialize();
    memset(&ao_format, 0, sizeof(ao_format));
    ao_driver = ao_default_driver_id();

    //Initialise ffmpeg
    av_register_all();
}

MusicPlayer::~MusicPlayer()
{
    unload();
    ao_shutdown();
}

bool MusicPlayer::load(const std::string &filepath)
{
    try
    {
        //Unload anything previously loaded
        unload();
        track_filepath = filepath;
        frlog << Log::info << "Loading track: " << filepath << Log::end;

        //Load the file using ffmpeg
        av_container = avformat_alloc_context();
        if(avformat_open_input(&av_container, filepath.c_str(), NULL, NULL) < 0)
        {
            throw std::runtime_error("Could not open file: " + filepath);
        }

        if(avformat_find_stream_info(av_container, NULL) < 0)
        {
            throw std::runtime_error("Could not find file info");
        }

        av_dump_format(av_container, 0, filepath.c_str(), false);
        stream_id = -1;
        for(int i = 0; i < av_container->nb_streams; i++)
        {
            if(av_container->streams[i]->codec->codec_type == AVMEDIA_TYPE_AUDIO)
            {
                stream_id = i;
                break;
            }
        }
        if(stream_id == -1)
        {
            throw std::runtime_error("Could not find audio stream");
        }

        av_codec_context = av_container->streams[stream_id]->codec;
        av_codec = avcodec_find_decoder(av_codec_context->codec_id);

        if(av_codec == NULL)
        {
            throw std::runtime_error("avcodec_find_decoder() failed, cannot find codec.");
        }

        if(avcodec_open2(av_codec_context, av_codec, NULL) < 0)
        {
            throw std::runtime_error("avcodec_open2() failed, cannot open codec.");
        }


        //Setup libao, to play the audio
        AVSampleFormat sfmt = av_codec_context->sample_fmt;
        if(sfmt == AV_SAMPLE_FMT_U8)
        {
            ao_format.bits = 8;
        }
        else if(sfmt == AV_SAMPLE_FMT_S16)
        {
            ao_format.bits = 16;
        }
        else if(sfmt == AV_SAMPLE_FMT_S32)
        {
            ao_format.bits = 32;
        }
        else if(sfmt == AV_SAMPLE_FMT_U8P)
        {
            ao_format.bits = 8;
            is_planar = true;
        }
        else if(sfmt == AV_SAMPLE_FMT_S16P)
        {
            ao_format.bits = 16;
            is_planar = true;
        }
        else if(sfmt == AV_SAMPLE_FMT_S32P)
        {
            ao_format.bits = 32;
            is_planar = true;
        }
        else
        {
            throw std::runtime_error("Unsupported audio type " + std::to_string(sfmt));
        }

        plane_size = ao_format.bits / 8;
        ao_format.channels = av_codec_context->channels;
        ao_format.rate = av_codec_context->sample_rate;
        ao_format.byte_format = AO_FMT_NATIVE;
        ao_format.matrix = 0;

        ao_output = ao_open_live(ao_driver, &ao_format, NULL);
        if(ao_output == NULL)
        {
            throw std::runtime_error("Failed to initialise libao");
        }

        play_state = State::Playing;
        thread = std::unique_ptr<std::thread>(new std::thread(std::bind(&MusicPlayer::play_thread, this)));
    }
    catch(const std::exception &e)
    {
        frlog << Log::crit << "Failed to load " << filepath << ": " << e.what() << Log::end;
        return false;
    }

    return true;
}

void MusicPlayer::unload()
{
    frlog << Log::info << "Unloading: " << track_filepath << Log::end;
    set_state(State::Stopped);
    if(thread && thread->joinable())
        thread->join();
    if(av_container != nullptr)
        avformat_close_input(&av_container);
    if(av_codec != nullptr)
        avcodec_close(av_codec_context);
    if(ao_output != nullptr)
        ao_close(ao_output);
    av_container = nullptr;
    av_codec = nullptr;
    ao_output = nullptr;
    volume = 100;
    memset(&ao_format, 0, sizeof(ao_format));
}

void MusicPlayer::play()
{
    set_state(State::Playing);
}

void MusicPlayer::pause()
{
    set_state(State::Paused);
}

std::chrono::seconds MusicPlayer::get_offset()
{
    return std::chrono::seconds(1);
}

void MusicPlayer::set_offset(std::chrono::seconds offset)
{

}

void MusicPlayer::set_volume(uint32_t vol)
{
    volume = vol;
}

uint32_t MusicPlayer::get_volume()
{
    return volume;
}

std::chrono::seconds MusicPlayer::get_duration()
{
    if(play_state == State::Stopped)
        return std::chrono::seconds(1);
    return std::chrono::seconds(av_container->duration / AV_TIME_BASE);
}

void MusicPlayer::play_thread()
{

    //Play it
    AVPacket packet;
    av_init_packet(&packet);

    AVFrame *frame = av_frame_alloc();
    int buffer_size = MAX_AUDIO_FRAME_SIZE + FF_INPUT_BUFFER_PADDING_SIZE;;
    uint8_t buffer[buffer_size];
    packet.data = buffer;
    packet.size = buffer_size;

    //Go through the frames, passing them to libao
    int is_frame_over = 0;
    while(av_read_frame(av_container, &packet) >= 0)
    {
        {
            //Acquire lock on notifier
            std::unique_lock<std::mutex> lock(notifier_lock);

            notifier.wait(lock, [&]()
            {
                return play_state != State::Paused;
            });
            if(play_state == State::Stopped)
                break;

        }

        if(packet.stream_index == stream_id)
        {
            avcodec_decode_audio4(av_codec_context, frame, &is_frame_over, &packet);
            if(is_frame_over)
            {
                //If it's planar, convert to non-planar first
                if(is_planar)
                {
                    uint8_t sample_buffer[frame->linesize[0] * ao_format.channels];
                    uint32_t sample_index = 0;
                    for(uint32_t a = 0; a < frame->linesize[0]; a += plane_size)
                    {
                        for(int channel = 0; channel < ao_format.channels; channel++)
                        {
                            memcpy(sample_buffer + sample_index, frame->extended_data[channel] + a, (size_t)plane_size);
                            sample_index += plane_size;
                        }
                    }
                    ao_play(ao_output, (char *)sample_buffer, (uint_32)(frame->linesize[0] * ao_format.channels));
                }
                else
                {
                    ao_play(ao_output, (char *)frame->extended_data[0], (uint_32)frame->linesize[0]);
                }
            }
        }
    }

    //Cleanup
    av_frame_free(&frame);
}

void MusicPlayer::set_state(MusicPlayer::State state)
{
    frlog << Log::info << "Setting state of " << track_filepath << " to: " << state << Log::end;
    play_state = state;
    notifier.notify_all();
}

MusicPlayer::State MusicPlayer::get_state()
{
    return play_state;
}

const std::string &MusicPlayer::get_filepath()
{
    return track_filepath;
}

const std::string MusicPlayer::state_to_string(MusicPlayer::State state)
{
    static std::string state_strings[] = {"stopped", "playing", "paused"};
    return state_strings[state];
}
