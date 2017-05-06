//
// Created by fred.nicolson on 05/05/17.
//

#ifndef MEDIASERVER_MUSICPLAYER_H
#define MEDIASERVER_MUSICPLAYER_H
#include <string>
#include <chrono>
#include <thread>
#include <atomic>
#include <mutex>
#include <condition_variable>

extern "C" {
#include <libavformat/avformat.h>
#include <libavutil/samplefmt.h>
#include <ao/ao.h>
}

class MusicPlayer
{
public:
    enum State
    {
        Stopped,
        Playing,
        Paused
    };

    MusicPlayer();
    virtual ~MusicPlayer();

    /*!
     * Loads a soundtrack from a given filepath,
     * should be followed up with a call to 'play'.
     *
     * @param filepath A valid filepath to some media that can be loaded
     * @return True on success, false on failure.
     */
    bool load(const std::string &filepath);

    /*!
     * Unloads the given track
     */
    void unload();

    /*!
     * Starts/resumes playing the track
     */
    void play();

    /*!
     * Pauses the track
     */
    void pause();

    /*!
     * Gets the current playing offset from the track
     *
     * @return The current playing offset as a time point
     */
    std::chrono::time_point<std::chrono::system_clock> get_offset();

    /*!
     * Sets the current playing offset of the track
     *
     * @param offset Where to continue playing from
     */
    void set_offset(std::chrono::time_point<std::chrono::system_clock> offset);

    /*!
     * Sets the playback volume.
     * Should be a value between 0 and 100.
     *
     * @param volume The volume to use
     */
    void set_volume(uint32_t volume);

    /*!
     * Gets the current track volume.
     *
     * @return The track volume
     */
    uint32_t get_volume();

    /*!
     * Gets the track's duration.
     *
     * @return The tracks' duration.
     */
    std::chrono::time_point<std::chrono::system_clock> get_duration();

    /*!
     * Gets the playback state. Paused/playing/stopped etc.
     *
     * @return Current playback state
     */
    State get_state();

private:

    /*!
     * Music is played in another thread, so that calls to
     * play don't block, and the caller can keep going. This
     * occurs in this function.
     */
    void play_thread();

    /*!
     * Updates the internal play state of the player,
     * and notifies the playback thread (to let it pause,
     * resume etc if needed).
     *
     * @param play_state The state to set the play_state to
     */
    void set_state(State play_state);

    //State
    std::unique_ptr<std::thread> thread;
    uint32_t volume;
    std::atomic<State> play_state;
    std::string track_filepath;

    std::condition_variable notifier;
    std::mutex notifier_lock;

    bool is_planar = false;
    int plane_size = 0;
    AVFormatContext *av_container;
    AVCodecContext *av_codec_context;
    AVCodec *av_codec;
    int stream_id;

    int ao_driver;
    ao_device *ao_output;
    ao_sample_format ao_format;
};


#endif //MEDIASERVER_MUSICPLAYER_H
